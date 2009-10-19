/*--------------------------------------------------------------------
 *    The MB-system:	mb_rt.c	11/14/94
 *    $Id: mb_rt.c,v 5.2 2008/07/10 06:43:40 caress Exp $
 *
 *    Copyright (c) 1994-2009 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mb_rt.c traces a ray through a gradient velocity structure. The
 * ray's starting position and takeoff angle are provided, as is
 * the velocity structure. The ray is traced until it either exits
 * the model or exhausts the specified travel time.
 *
 * Author:	D. W. Caress
 * Date:	November 14, 1994
 *
 * $Log: mb_rt.c,v $
 * Revision 5.2  2008/07/10 06:43:40  caress
 * Preparing for 5.1.1beta20
 *
 * Revision 5.1  2007/10/08 06:10:15  caress
 * Added function prototypes.
 *
 * Revision 5.0  2000/12/01 22:53:59  caress
 * First cut at Version 5.0.
 *
 * Revision 4.10  2000/10/11  00:54:20  caress
 * Converted to ANSI C
 *
 * Revision 4.9  2000/09/30  06:54:58  caress
 * Snapshot for Dale.
 *
 * Revision 4.8  1999/02/04  23:54:34  caress
 * MB-System version 4.6beta7
 *
 * Revision 4.7  1998/10/05  19:18:30  caress
 * MB-System version 4.6beta
 *
 * Revision 4.6  1997/09/12  18:51:10  caress
 * Moved mb_rt.c to own directory.
 *
 *
 */

/* standard global include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"

/* raytracing defines */
#define	MB_RT_GRADIENT_TOLERANCE    0.00001
#define	MB_RT_LAYER_HOMOGENEOUS	    0
#define	MB_RT_LAYER_GRADIENT	    1
#define	MB_RT_ERROR 0
#define	MB_RT_DOWN 1
#define	MB_RT_UP 2
#define	MB_RT_DOWN_TURN 3
#define	MB_RT_UP_TURN 4
#define	MB_RT_OUT_BOTTOM 5
#define	MB_RT_OUT_TOP 6
#define	MB_RT_NUMBER_SEGMENTS 5
#define	MB_SSV_NO_USE	    0
#define	MB_SSV_CORRECT	    1
#define	MB_SSV_INCORRECT    2

/* velocity model structure */
struct	velocity_model
	{
	int	number_node;
	double	*depth;
	double	*velocity;
	int	number_layer;
	int	*layer_mode;
	double	*layer_gradient;
	double	*layer_depth_center;
	double	*layer_depth_top;
	double	*layer_depth_bottom;
	double	*layer_vel_top;
	double	*layer_vel_bottom;
	};
	
/* global raytrace values */
static struct velocity_model *model;
static int	ray_status;
static int	done;
static int	outofbounds;
static int	layer;
static int	turned;
static int	number_plot_max;
static int	number_plot;
static int	sign_x;
static double	xx;
static double	zz;
static double	xf;
static double	zf;
static double	tt;
static double	dt;
static double	tt_left;
static double	vv_source;
static double	pp;
static double	xc;
static double	zc;
static double	radius;
static double	*xx_plot;
static double	*zz_plot;

int mb_rt_init(int verbose, int number_node, 
		double *depth, double *velocity, 
		void **modelptr, int *error);
int mb_rt_deall(int verbose, void **modelptr, int *error);
int mb_rt(int verbose, void *modelptr, 
	double source_depth, double source_angle, double end_time, 
	int ssv_mode, double surface_vel, double null_angle, 
	int nplot_max, int *nplot, double *xplot, double *zplot, 
	double *x, double *z, double *travel_time, int *ray_stat, int *error);
int mb_rt_circular(int verbose, int *error);
int mb_rt_quad1(int verbose, int *error);
int mb_rt_quad2(int verbose, int *error);
int mb_rt_quad3(int verbose, int *error);
int mb_rt_quad4(int verbose, int *error);
int mb_rt_get_depth(int verbose, double beta, int dir_sign, int turn_sign, 
		double *depth, int *error);
int mb_rt_plot_circular(int verbose, int *error);
int mb_rt_line(int verbose, int *error);
int mb_rt_vertical(int verbose, int *error);

static char rcs_id[]="$Id: mb_rt.c,v 5.2 2008/07/10 06:43:40 caress Exp $";

/*--------------------------------------------------------------------------*/
int mb_rt_init(int verbose, int number_node, 
		double *depth, double *velocity, 
		void **modelptr, int *error)
{
	char	*function_name = "mb_rt_init";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBA function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		fprintf(stderr,"dbg2       number_node:      %d\n",number_node);
		for (i=0;i<number_node;i++)
			{
			fprintf(stderr,"dbg2       depth: %f  velocity:%f\n",
				depth[i], velocity[i]);
			}
		fprintf(stderr,"dbg2       modelptr:         %ld\n",(long)modelptr);
		}

	/* allocate memory for model structure */
	status = mb_mallocd(verbose,__FILE__,__LINE__,sizeof(struct velocity_model),(void **)modelptr,error);

	/* set variables and allocate memory for velocity model */
	model = (struct velocity_model *) *modelptr;
	model->number_node = number_node;
	status = mb_mallocd(verbose,__FILE__,__LINE__,number_node*sizeof(double),
				(void **)&(model->depth),error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,number_node*sizeof(double),
				(void **)&(model->velocity),error);
	model->number_layer = number_node - 1;
	status = mb_mallocd(verbose,__FILE__,__LINE__,model->number_layer*sizeof(int),
				(void **)&(model->layer_mode),error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,model->number_layer*sizeof(double),
				(void **)&(model->layer_gradient),error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,model->number_layer*sizeof(double),
				(void **)&(model->layer_depth_center),error);
	if (status == MB_SUCCESS)
		{
		model->layer_depth_top = &model->depth[0];
		model->layer_depth_bottom = &model->depth[1];
		model->layer_vel_top = &model->velocity[0];
		model->layer_vel_bottom = &model->velocity[1];
		}

	/* put model into structure */
	for (i=0;i<number_node;i++)
		{
		model->depth[i] = depth[i];
		model->velocity[i] = velocity[i];
		}
	for (i=0;i<model->number_layer;i++)
		{
		model->layer_gradient[i] = 
			(model->layer_vel_bottom[i] - model->layer_vel_top[i])/
			(model->layer_depth_bottom[i] - model->layer_depth_top[i]);
		if (fabs(model->layer_gradient[i]) > MB_RT_GRADIENT_TOLERANCE)
			{
			model->layer_mode[i] = MB_RT_LAYER_GRADIENT;
			model->layer_depth_center[i] = 
				model->layer_depth_top[i] - 
				model->layer_vel_top[i]/
				model->layer_gradient[i];
			}
		else
			{
			model->layer_mode[i] = MB_RT_LAYER_HOMOGENEOUS;
			model->layer_depth_center[i] = 0.0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       modelptr:   %ld\n",(long)*modelptr);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------------*/
int mb_rt_deall(int verbose, void **modelptr, int *error)
{
	char	*function_name = "mb_rt";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBA function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		fprintf(stderr,"dbg2       modelptr:         %ld\n",(long)modelptr);
		}

	/* deallocate memory for velocity model */
	model = (struct velocity_model *) *modelptr;
	status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(model->depth),error);
	status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(model->velocity),error);
	status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(model->layer_mode),error);
	status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(model->layer_gradient),error);
	status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(model->layer_depth_center),error);
	status = mb_freed(verbose,__FILE__, __LINE__, (void **)modelptr,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------------*/
int mb_rt(int verbose, void *modelptr, 
	double source_depth, double source_angle, double end_time, 
	int ssv_mode, double surface_vel, double null_angle, 
	int nplot_max, int *nplot, double *xplot, double *zplot, 
	double *x, double *z, double *travel_time, int *ray_stat, int *error)
{
	char	*function_name = "mb_rt";
	int	status = MB_SUCCESS;
	double	diff_angle;
	double	vel_ratio;
	int	i;

	/* get pointer to velocity model */
	model = (struct velocity_model *) modelptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBA function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		fprintf(stderr,"dbg2       modelptr:         %ld\n",(long)modelptr);
		fprintf(stderr,"dbg2       number_node:      %d\n",model->number_node);
		fprintf(stderr,"dbg2       layer depth velocity:\n");
		for (i=0;i<model->number_node;i++)
			{
			fprintf(stderr,"dbg2       %d %f %f\n",
				i, model->depth[i], model->velocity[i]);
			}
		fprintf(stderr,"dbg2       number_layer:     %d\n",model->number_layer);
		fprintf(stderr,"dbg2       layer top bottom veltop velbot  mode grad zc\n");
		for (i=0;i<model->number_layer;i++)
			{
			fprintf(stderr,"dbg2       %d  %f %f  %f %f  %d %f %f\n",
				i, model->layer_depth_top[i], 
				model->layer_depth_bottom[i],
				model->layer_vel_top[i], 
				model->layer_vel_bottom[i],
				model->layer_mode[i],
				model->layer_gradient[i],
				model->layer_depth_center[i]);
			}
		fprintf(stderr,"dbg2       source_depth:     %f\n",source_depth);
		fprintf(stderr,"dbg2       source_angle:     %f\n",source_angle);
		fprintf(stderr,"dbg2       end_time:         %f\n",end_time);
		fprintf(stderr,"dbg2       ssv_mode:         %d\n",ssv_mode);
		fprintf(stderr,"dbg2       surface_vel:      %f\n",surface_vel);
		fprintf(stderr,"dbg2       null_angle:       %f\n",null_angle);
		fprintf(stderr,"dbg2       nplot_max:        %d\n",nplot_max);
		}

	/* prepare the ray */
	layer = -1;
	for (i=0;i<model->number_layer;i++)
		{
		if (source_depth >= model->layer_depth_top[i] 
			&& source_depth <= model->layer_depth_bottom[i])
			layer = i;
		}
	if (verbose > 0 && layer == -1)
		{
		fprintf(stderr,"\nError in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"Ray source depth not within model!!\n");
		fprintf(stderr,"Raytracing terminated with error!!\n");
		}
	if (layer == -1)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_PARAMETER;
		return(status);
		}
	vv_source = model->layer_vel_top[layer] 
		+ model->layer_gradient[layer]*(source_depth - model->layer_depth_top[layer]);

	/* reset takeoff angle because of surface sound velocity change:
	    ssv_mode == MB_SSV_NO_USE:
	      Do nothing to angles before raytracing.
	    ssv_mode == MB_SSV_CORRECT:
	      Adjust the angle assuming the original SSV was correct. 
	      This means use a horizontal layer assumption and Snell's
	      law to adjust angle as ray goes from original SSV
	      to the velocity in the SVP at the initial depth. The
	      null angle is ignored.
	    ssv_mode == MB_SSV_INCORRECT:
	      Adjust the angle assuming the original SSV was incorrect. 
	      This means use Snell's law law to adjust angle in a
	      rotated frame of reference (rotated by null angle) as 
	      ray goes from original SSV to the velocity in the 
	      SVP at the initial depth. This insures that the geometry 
	      of the receiving transducer array is properly handled.
	 */
	if (ssv_mode == MB_SSV_CORRECT && surface_vel > 0.0)
		{
		pp = sin(DTR*source_angle)/surface_vel;
		vel_ratio = MIN(1.0, pp * vv_source);
		source_angle = asin(vel_ratio) * RTD;
		}
	else if (ssv_mode == MB_SSV_INCORRECT && surface_vel > 0.0)
		{
		diff_angle = source_angle - null_angle;
		pp = sin(DTR*diff_angle)/surface_vel;
		vel_ratio = MIN(1.0, pp * vv_source);
		diff_angle = asin(vel_ratio) * RTD;
		source_angle = null_angle + diff_angle;
		}

	/* now initialize ray */
	if (source_angle > 0.0)
		sign_x = 1;
	else
		sign_x = -1;
	source_angle = fabs(source_angle);
	pp = sin(DTR*source_angle)/vv_source;
	if (source_angle < 90.0)
		{
		turned = MB_NO;
		ray_status = MB_RT_DOWN;
		}
	else
		{
		turned = MB_YES;
		ray_status = MB_RT_UP;
		}
	xx = 0.0;
	zz = source_depth;
	tt = 0.0;
	tt_left = end_time;
	outofbounds = MB_NO;
	done = MB_NO;

	/* set up raypath plotting */
	number_plot_max = nplot_max;
	number_plot = 0;
	if (number_plot_max > 0)
		{
		xx_plot = xplot;
		zz_plot = zplot;
		xx_plot[0] = xx;
		zz_plot[0] = zz;
		number_plot++;
		}

	/* print debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  About to trace ray in MB_RT function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2       xx:               %f\n",xx);
		fprintf(stderr,"dbg2       zz:               %f\n",zz);
		fprintf(stderr,"dbg2       layer:            %d\n",layer);
		fprintf(stderr,"dbg2       layer_mode:       %d\n",
			model->layer_mode[layer]);
		fprintf(stderr,"dbg2       vv_source:        %f\n",vv_source);
		fprintf(stderr,"dbg2       pp:               %f\n",pp);
		fprintf(stderr,"dbg2       tt_left:          %f\n",tt_left);
		}

	/* trace the ray */
	while (!done && !outofbounds)
		{
		/* trace ray through current layer */
		if (model->layer_mode[layer] == MB_RT_LAYER_GRADIENT 
			&& pp > 0.0)
			status = mb_rt_circular(verbose, error);
		else if (model->layer_mode[layer] == MB_RT_LAYER_GRADIENT)
			status = mb_rt_vertical(verbose, error);
		else
			status = mb_rt_line(verbose, error);

		/* update ray */
		tt = tt + dt;
		if (layer < 0)
			{
			outofbounds = MB_YES;
			ray_status = MB_RT_OUT_TOP;
			}
		if (layer >= model->number_layer)
			{
			outofbounds = MB_YES;
			ray_status = MB_RT_OUT_BOTTOM;
			}
		if (tt_left <= 0.0)
			done = MB_YES;

		/* print debug statements */
		if (verbose >= 2)
			{
			fprintf(stderr,"\ndbg2  Done with ray iteration in MB_RT function <%s>\n",
				function_name);
			fprintf(stderr,"dbg2       xx:               %f\n",xx);
			fprintf(stderr,"dbg2       zz:               %f\n",zz);
			fprintf(stderr,"dbg2       xf:               %f\n",xf);
			fprintf(stderr,"dbg2       zf:               %f\n",zf);
			fprintf(stderr,"dbg2       layer:            %d\n",layer);
			fprintf(stderr,"dbg2       layer_mode:       %d\n",
				model->layer_mode[layer]);
			fprintf(stderr,"dbg2       tt:               %f\n",tt);
			fprintf(stderr,"dbg2       dt:               %f\n",dt);
			fprintf(stderr,"dbg2       tt_left:          %f\n",tt_left);
			}

		/* reset position */
		xx = xf;
		zz = zf;
		}

	/* report results */
	*x = xx;
	*z = zz;
	*travel_time = tt;
	*ray_stat = ray_status;
	if (number_plot_max > 0)
		*nplot = number_plot;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		if (nplot_max > 0)
		    fprintf(stderr,"dbg2       nplot:      %d\n",*nplot);
		fprintf(stderr,"dbg2       x:          %f\n",*x);
		fprintf(stderr,"dbg2       z:          %f\n",*z);
		fprintf(stderr,"dbg2       travel_time:%f\n",*travel_time);
		fprintf(stderr,"dbg2       raystat:    %d\n",*ray_stat);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------------*/
int mb_rt_circular(int verbose, int *error)
{
	char	*function_name = "mb_rt_circular";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBA function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		}

	/* decide which case to use */
	if (turned == MB_NO && model->layer_gradient[layer] > 0.0)
		status = mb_rt_quad1(verbose, error);
	else if (turned == MB_NO)
		status = mb_rt_quad3(verbose, error);	
	else if (turned == MB_YES && model->layer_gradient[layer] > 0.0)
		status = mb_rt_quad2(verbose, error);	
	else if (turned == MB_YES)
		status = mb_rt_quad4(verbose, error);

	/* put points in plotting arrays */
	if (number_plot_max > 0)
		status = mb_rt_plot_circular(verbose, error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------------*/
int mb_rt_quad1(int verbose, int *error)
{
	char	*function_name = "mb_rt_quad1";
	int	status = MB_SUCCESS;
	double	vi;
	double	ip;
	double	ipvi;
	double	beta;
	double	ivf;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBA function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		}

	/* find circular path */
	radius = fabs(1.0 / (pp * model->layer_gradient[layer]));
	zc = model->layer_depth_center[layer];
	xc = xx + sqrt(radius * radius - (zz - zc) * (zz - zc));
	vi = model->layer_vel_top[layer] 
		+ (zz - model->layer_depth_top[layer]) 
		* model->layer_gradient[layer];
	ip = 1.0 / pp;
	ipvi = ip/vi;
	beta = log(ipvi + sqrt(ipvi * ipvi - 1.0));

	/* Check if ray turns in layer */
	if (zc + radius < model->layer_depth_bottom[layer])
		{
		/* ray can turn in this layer */
		dt = fabs(beta / model->layer_gradient[layer]);

		/* raypath ends before turning */
		if (dt >= tt_left)
			{
			mb_rt_get_depth(verbose, beta, -1, 1, &zf, error);
			xf = xc - sqrt(radius * radius 
				- (zf - zc) * (zf - zc));
			dt = tt_left;
			tt_left = 0.0;
			}

		/* raypath turns */
		else
			{
			ivf = 1.0 / model->layer_vel_top[layer];
			dt = fabs((log(ip * ivf + 
				ip * sqrt(ivf * ivf - pp * pp)) + beta)
				/ model->layer_gradient[layer]);

			/* ray turns and exits layer before 
				exhausting tt_left */
			if (dt <= tt_left)
				{
				turned = MB_YES;
				ray_status = MB_RT_UP_TURN;
				zf = model->layer_depth_top[layer];
				xf = xc + sqrt(radius * radius 
					- (zf - zc) * (zf - zc));
				tt_left = tt_left - dt;
				layer--;
				}
			/* ray turns and exhausts tt_left 
				before exiting layer */
			else if (dt > tt_left)
				{
				turned = MB_YES;
				ray_status = MB_RT_UP_TURN;
				mb_rt_get_depth(verbose, beta, 1, -1, &zf, error);
				xf = xc + sqrt(radius * radius 
					- (zf - zc) * (zf - zc));
				dt = tt_left;
				tt_left = 0.0;
				}
			}
		}
	else
		{
		/* ray cannot turn in this layer */
		ivf = 1.0 / model->layer_vel_bottom[layer];
		dt = fabs((log(ip * ivf + 
			ip * sqrt(ivf * ivf - pp * pp)) - beta)
			/ model->layer_gradient[layer]);


		/* ray exits layer before exhausting tt_left */
		if (dt <= tt_left)
			{
			zf = model->layer_depth_bottom[layer];
			xf = xc - sqrt(radius * radius 
				- (zf - zc) * (zf - zc));
			tt_left = tt_left - dt;
			layer++;
			}
		/* ray exhausts tt_left before exiting layer */
		else if (dt > tt_left)
			{
			turned = MB_YES;
			ray_status = MB_RT_UP_TURN;
			mb_rt_get_depth(verbose, beta, -1, 1, &zf, error);
			xf = xc - sqrt(radius * radius 
				- (zf - zc) * (zf - zc));
			dt = tt_left;
			tt_left = 0.0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------------*/
int mb_rt_quad2(int verbose, int *error)
{
	char	*function_name = "mb_rt_quad2";
	int	status = MB_SUCCESS;
	double	vi;
	double	ip;
	double	ipvi;
	double	beta;
	double	ivf;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBA function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		}

	/* find circular path */
	radius = fabs(1.0 / (pp * model->layer_gradient[layer]));
	zc = model->layer_depth_center[layer];
	xc = xx - sqrt(radius * radius - (zz - zc) * (zz - zc));
	vi = model->layer_vel_top[layer] 
		+ (zz - model->layer_depth_top[layer]) 
		* model->layer_gradient[layer];
	ip = 1.0 / pp;
	ipvi = ip/vi;
	beta = log(ipvi + sqrt(ipvi * ipvi - 1.0));

	/* Check if ray ends in layer */
	ivf = 1.0 / model->layer_vel_top[layer];
	dt = fabs((log(ip * ivf + 
		ip * sqrt(ivf * ivf - pp * pp)) - beta)
		/ model->layer_gradient[layer]);

	/* ray exits layer before exhausting tt_left */
	if (dt <= tt_left)
		{
		zf = model->layer_depth_top[layer];
		xf = xc + sqrt(radius * radius 
			- (zf - zc) * (zf - zc));
		tt_left = tt_left - dt;
		layer--;
		}
	/* ray exhausts tt_left before exiting layer */
	else if (dt > tt_left)
		{
		mb_rt_get_depth(verbose, beta, 1, 1, &zf, error);
		xf = xc + sqrt(radius * radius 
			- (zf - zc) * (zf - zc));
		dt = tt_left;
		tt_left = 0.0;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------------*/
int mb_rt_quad3(int verbose, int *error)
{
	char	*function_name = "mb_rt_quad3";
	int	status = MB_SUCCESS;
	double	vi;
	double	ip;
	double	ipvi;
	double	beta;
	double	ivf;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBA function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		}

	/* find circular path */
	radius = fabs(1.0 / (pp * model->layer_gradient[layer]));
	zc = model->layer_depth_center[layer];
	xc = xx - sqrt(radius * radius - (zz - zc) * (zz - zc));
	vi = model->layer_vel_top[layer] 
		+ (zz - model->layer_depth_top[layer]) 
		* model->layer_gradient[layer];
	ip = 1.0 / pp;
	ipvi = ip/vi;
	beta = log(ipvi + sqrt(ipvi * ipvi - 1.0));

	/* Check if ray ends in layer */
	ivf = 1.0 / model->layer_vel_bottom[layer];
	dt = fabs((log(ip * ivf + 
		ip * sqrt(ivf * ivf - pp * pp)) - beta)
		/ model->layer_gradient[layer]);

	/* ray exits layer before exhausting tt_left */
	if (dt <= tt_left)
		{
		zf = model->layer_depth_bottom[layer];
		xf = xc + sqrt(radius * radius 
			- (zf - zc) * (zf - zc));
		tt_left = tt_left - dt;
		layer++;
		}
	/* ray exhausts tt_left before exiting layer */
	else if (dt > tt_left)
		{
		mb_rt_get_depth(verbose, beta, 1, 1, &zf, error);
		xf = xc + sqrt(radius * radius 
			- (zf - zc) * (zf - zc));
		dt = tt_left;
		tt_left = 0.0;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------------*/
int mb_rt_quad4(int verbose, int *error)
{
	char	*function_name = "mb_rt_quad4";
	int	status = MB_SUCCESS;
	double	vi;
	double	ip;
	double	ipvi;
	double	beta;
	double	ivf;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBA function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		}

	/* find circular path */
	radius = fabs(1.0 / (pp * model->layer_gradient[layer]));
	zc = model->layer_depth_center[layer];
	xc = xx + sqrt(radius * radius - (zz - zc) * (zz - zc));
	vi = model->layer_vel_top[layer] 
		+ (zz - model->layer_depth_top[layer]) 
		* model->layer_gradient[layer];
	ip = 1.0 / pp;
	ipvi = ip/vi;
	beta = log(ipvi + sqrt(ipvi * ipvi - 1.0));

	/* Check if ray turns in layer */
	if (zc - radius > model->layer_depth_top[layer])
		{
		/* ray can turn in this layer */
		dt = fabs(beta / model->layer_gradient[layer]);

		/* raypath ends before turning */
		if (dt >= tt_left)
			{
			mb_rt_get_depth(verbose, beta, -1, 1, &zf, error);
			xf = xc - sqrt(radius * radius 
				- (zf - zc) * (zf - zc));
			dt = tt_left;
			tt_left = 0.0;
			}

		/* raypath turns */
		else
			{
			ivf = 1.0 / model->layer_vel_bottom[layer];
			dt = fabs((log(ip * ivf + 
				ip * sqrt(ivf * ivf - pp * pp)) + beta)
				/ model->layer_gradient[layer]);

			/* ray turns and exits layer before 
				exhausting tt_left */
			if (dt <= tt_left)
				{
				turned = MB_NO;
				ray_status = MB_RT_DOWN_TURN;
				zf = model->layer_depth_bottom[layer];
				xf = xc + sqrt(radius * radius 
					- (zf - zc) * (zf - zc));
				tt_left = tt_left - dt;
				layer++;
				}
			/* ray turns and exhausts tt_left 
				before exiting layer */
			else if (dt > tt_left)
				{
				turned = MB_NO;
				ray_status = MB_RT_DOWN_TURN;
				mb_rt_get_depth(verbose, beta, 1, -1, &zf, error);
				xf = xc + sqrt(radius * radius 
					- (zf - zc) * (zf - zc));
				dt = tt_left;
				tt_left = 0.0;
				}
			}
		}
	else
		{
		/* ray cannot turn in this layer */
		ivf = 1.0 / model->layer_vel_top[layer];
		dt = fabs((log(ip * ivf + 
			ip * sqrt(ivf * ivf - pp * pp)) - beta)
			/ model->layer_gradient[layer]);


		/* ray exits layer before exhausting tt_left */
		if (dt <= tt_left)
			{
			zf = model->layer_depth_top[layer];
			xf = xc - sqrt(radius * radius 
				- (zf - zc) * (zf - zc));
			tt_left = tt_left - dt;
			layer--;
			}
		/* ray exhausts tt_left before exiting layer */
		else if (dt > tt_left)
			{
			turned = MB_YES;
			ray_status = MB_RT_UP_TURN;
			mb_rt_get_depth(verbose, beta, -1, 1, &zf, error);
			xf = xc - sqrt(radius * radius 
				- (zf - zc) * (zf - zc));
			dt = tt_left;
			tt_left = 0.0;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------------*/
int mb_rt_get_depth(int verbose, double beta, int dir_sign, int turn_sign, 
		double *depth, int *error)
{
	char	*function_name = "mb_rt_get_depth";
	int	status = MB_SUCCESS;
	double	alpha;
	double	velf;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBA function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		fprintf(stderr,"dbg2       beta:             %f\n",beta);
		fprintf(stderr,"dbg2       dir_sign:         %d\n",dir_sign);
		fprintf(stderr,"dbg2       turn_sign:        %d\n",turn_sign);
		}

	/* find depth */
	alpha = pp * exp(dir_sign * tt_left
		* fabs(model->layer_gradient[layer]) 
		+ turn_sign*beta);
	velf = 2 * alpha / (alpha * alpha + pp * pp);
	*depth = model->layer_depth_top[layer] 
		+ (velf - model->layer_vel_top[layer]) 
		/ model->layer_gradient[layer];

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       depth:      %f\n",*depth);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------------*/
int mb_rt_plot_circular(int verbose, int *error)
{
	char	*function_name = "mb_rt_plot_circular";
	int	status = MB_SUCCESS;
	double	ai;
	double	af;
	double	dang;
	double	angle;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBA function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		}

	/* get angle range */
	ai = atan2((xx - xc), (zz - zc));
	af = atan2((xf - xc), (zf - zc));
	dang = (af - ai)/MB_RT_NUMBER_SEGMENTS;

	/* add points to plotting arrays */
	for (i=0;i<MB_RT_NUMBER_SEGMENTS;i++)
		{
		angle = ai + (i + 1) * dang;
		if (number_plot < number_plot_max)
			{
			xx_plot[number_plot] = sign_x * (xc + radius * sin(angle));
			zz_plot[number_plot] = zc + radius * cos(angle);
			number_plot++;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------------*/
int mb_rt_line(int verbose, int *error)
{
	char	*function_name = "mb_rt_line";
	int	status = MB_SUCCESS;
	double	theta;
	double	xvel;
	double	zvel;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBA function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		}

	/* find linear path */
	theta = asin(pp * model->layer_vel_top[layer]);
	if (turned == MB_NO)
		{
		zf = model->layer_depth_bottom[layer];
		}
	else
		{
		theta = theta + M_PI;
		zf = model->layer_depth_top[layer];
		}
	xvel = model->layer_vel_top[layer] * sin(theta);
	zvel = model->layer_vel_top[layer] * cos(theta);
	if (zvel != 0.0)
		dt = (zf - zz) / zvel;
	else
		dt = 100 * tt_left;

	/* ray exhausts tt_left before exiting layer */
	if (dt >= tt_left)
		{
		xf = xx + xvel * tt_left;
		zf = zz + zvel * tt_left;
		dt = tt_left;
		tt_left = 0.0;
		}

	/* ray exits layer before exhausting tt_left */
	else
		{
		xf = xx + xvel*dt;
		zf = zz + zvel*dt;
		tt_left = tt_left - dt;
		if (turned == MB_YES)
			layer--;
		else
			layer++;
		}

	/* put points in plotting arrays */
	if (number_plot_max > 0 && number_plot < number_plot_max)
		{
		xx_plot[number_plot] = sign_x * xf;
		zz_plot[number_plot] = zf;
		number_plot++;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------------*/
int mb_rt_vertical(int verbose, int *error)
{
	char	*function_name = "mb_rt_vertical";
	int	status = MB_SUCCESS;
	double	vi;
	double	vf;
	double	vfvi;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBBA function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		}

	/* find linear path */
	vi = model->layer_vel_top[layer] 
		+ (zz - model->layer_depth_top[layer]) 
		* model->layer_gradient[layer];
	if (turned == MB_NO)
		{
		zf = model->layer_depth_bottom[layer];
		vf = model->layer_vel_bottom[layer];
		}
	else
		{
		zf = model->layer_depth_top[layer];
		vf = model->layer_vel_top[layer];
		}
	dt = fabs(log(vf / vi) / model->layer_gradient[layer]);

	/* ray exhausts tt_left before exiting layer */
	if (dt >= tt_left)
		{
		xf = xx;
		vfvi = exp(tt_left * model->layer_gradient[layer]);
		if (turned == MB_NO)
			vf = vi * vfvi;
		else if (turned == MB_YES)
			vf = vi / vfvi;
		zf = (vf - model->layer_vel_top[layer])
			/ model->layer_gradient[layer]
			+ model->layer_depth_top[layer];
		dt = tt_left;
		tt_left = 0.0;
		}

	/* ray exits layer before exhausting tt_left */
	else
		{
		xf = xx;
		tt_left = tt_left - dt;
		if (turned == MB_YES)
			layer--;
		else
			layer++;
		}

	/* put points in plotting arrays */
	if (number_plot_max > 0 && number_plot < number_plot_max)
		{
		xx_plot[number_plot] = sign_x * xf;
		zz_plot[number_plot] = zf;
		number_plot++;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	return(status);
}
/*--------------------------------------------------------------------------*/
