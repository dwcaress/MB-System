/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_process.c	9/25/2003
 *    $Id: mbview_process.c,v 5.2 2004-02-24 22:52:29 caress Exp $
 *
 *    Copyright (c) 2003 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *------------------------------------------------------------------------------*/
/*
 *
 * Author:	D. W. Caress
 * Date:	September 25, 2003
 *
 * Note:	This code was broken out of mbview_callbacks.c, which was
 *		begun on October 7, 2002
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.1  2004/01/06 21:11:04  caress
 * Added pick region capability.
 *
 * Revision 5.0  2003/12/02 20:38:34  caress
 * Making version number 5.0
 *
 * Revision 1.3  2003/12/01 20:55:48  caress
 * Changed debug output.
 *
 * Revision 1.2  2003/11/25 01:43:19  caress
 * MBview version generated during EW0310.
 *
 *
 */
/*------------------------------------------------------------------------------*/

/* Standard includes for builtins. */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/* Motif required Headers */
#include <X11/StringDefs.h>
#include <X11/cursorfont.h>
#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/DialogS.h>
#include <Xm/RepType.h>
#include <Xm/MwmUtil.h>
#include <Xm/BulletinB.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>
#include <Xm/PushB.h>
#include <Xm/Separator.h>
#include "MB3DView.h"

/* OpenGL include files */
#include <GL/gl.h>
#include <GL/glu.h>
#include "GL/GLwMDrawA.h" 
#include <GL/glx.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"

/* mbview include */
#include "mbview.h"
#include "mbviewprivate.h"

/*------------------------------------------------------------------------------*/

/* local variables */
Cardinal 	ac;
Arg      	args[256];
char		value_text[MB_PATH_MAXLINE];

static char rcs_id[]="$Id: mbview_process.c,v 5.2 2004-02-24 22:52:29 caress Exp $";

/*------------------------------------------------------------------------------*/
int mbview_projectdata(int instance)
{
	/* local variables */
	char	*function_name = "mbview_projectdata";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	int	proj_status = MB_SUCCESS;
	double	mtodeglon, mtodeglat;
	double	xgrid, ygrid, xlon, ylat, xdisplay, ydisplay, zdisplay;
	double	xlonmin, xlonmax, ylatmin, ylatmax;
	int	derivative_ok;
	double	dx, dy;
	int	i, j, k, k1, k2;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		}
if (mbv_verbose >= 2)
fprintf(stderr,"mbview_projectdata: %d\n", instance);
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* delete old projections if necessary */
	if (view->primary_pj_init == MB_YES
		&& view->primary_pjptr != NULL)
		{
		mb_proj_free(mbv_verbose, &(view->primary_pjptr), &error);
		view->primary_pj_init = MB_NO;
		}
	if (view->secondary_pj_init == MB_YES
		&& view->secondary_pjptr != NULL)
		{
		mb_proj_free(mbv_verbose, &(view->secondary_pjptr), &error);
		view->secondary_pj_init = MB_NO;
		}
	if (view->display_pj_init == MB_YES
		&& view->display_pjptr != NULL)
		{
		mb_proj_free(mbv_verbose, &(view->display_pjptr), &error);
		view->display_pj_init = MB_NO;
		}
	if (data->primary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED)
		data->primary_grid_projection_mode = MBV_PROJECTION_PROJECTED;
	if (data->secondary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED)
		data->secondary_grid_projection_mode = MBV_PROJECTION_PROJECTED;
	if (data->display_projection_mode == MBV_PROJECTION_ALREADYPROJECTED)
		data->display_projection_mode = MBV_PROJECTION_PROJECTED;

	/* check for case where primary grid is already projected but displayed
	   in that same projection 
	   - use same bounds info */
	if (data->primary_grid_projection_mode == MBV_PROJECTION_PROJECTED
		&& data->display_projection_mode == MBV_PROJECTION_PROJECTED
		&& strcmp(data->primary_grid_projection_id,
				data->display_projection_id) == 0)
		{
		/* reset modes */
		data->primary_grid_projection_mode = MBV_PROJECTION_ALREADYPROJECTED;
		data->display_projection_mode = MBV_PROJECTION_ALREADYPROJECTED;
		
		/* get bounds */
		view->xmin = data->primary_xmin;
		view->xmax = data->primary_xmax;
		view->ymin = data->primary_ymin;
		view->ymax = data->primary_ymax;

		/* set projection for getting lon lat */
		proj_status = mb_proj_init(mbv_verbose, 
					data->primary_grid_projection_id,
					&(view->primary_pjptr),
					&error);
		if (proj_status == MB_SUCCESS)
			view->primary_pj_init = MB_YES;
		}
	   
	/* else set up projections as needed */
	else 
		{
		/* first go from grid coordinates to lon lat */
		if (data->primary_grid_projection_mode == MBV_PROJECTION_PROJECTED)
			{
			/* set projection */
			proj_status = mb_proj_init(mbv_verbose, 
						data->primary_grid_projection_id,
						&(view->primary_pjptr),
						&error);
			if (proj_status == MB_SUCCESS)
				view->primary_pj_init = MB_YES;
				
			/* get initial bounds */
			proj_status = mb_proj_inverse(mbv_verbose,
						view->primary_pjptr,
						data->primary_xmin,
						data->primary_ymin,
						&xlonmin, &ylatmin,
						&error);						
			proj_status = mb_proj_inverse(mbv_verbose,
						view->primary_pjptr,
						data->primary_xmax,
						data->primary_ymax,
						&xlonmax, &ylatmax,
						&error);						
			}
		else
			{
			/* already lon lat - just copy initial bounds */
			xlonmin = data->primary_xmin;
			xlonmax = data->primary_xmax;
			ylatmin = data->primary_ymin;
			ylatmax = data->primary_ymax;
			}
			
		/* now go from lon lat to display coordinates */
		if (data->display_projection_mode == MBV_PROJECTION_PROJECTED)
			{
			/* set projection */
			proj_status = mb_proj_init(mbv_verbose, 
						data->display_projection_id,
						&(view->display_pjptr),
						&error);
			if (proj_status == MB_SUCCESS)
				view->display_pj_init = MB_YES;
				
			/* get bounds */
			proj_status = mb_proj_forward(mbv_verbose,
						view->display_pjptr,
						xlonmin, ylatmin,
						&view->xmin, 
						&view->ymin,
						&error);						
			proj_status = mb_proj_forward(mbv_verbose,
						view->display_pjptr,
						xlonmax, ylatmax,
						&view->xmax, 
						&view->ymax,
						&error);						

			/* get origin */
			view->xorigin = 0.5 * (view->xmin + view->xmax);
			view->yorigin = 0.5 * (view->ymin + view->ymax);
			view->zorigin = data->exageration * 0.5 * (data->primary_min + data->primary_max);
			}
		else if (data->display_projection_mode == MBV_PROJECTION_GEOGRAPHIC)
			{
			/* set up geographic pseduo-projection */
			mb_coor_scale(mbv_verbose, 
	 			0.5*(ylatmin + ylatmax),
	 			&(view->mtodeglon), &(view->mtodeglat));

			/* get bounds */
			view->xmin = xlonmin / view->mtodeglon;
			view->xmax = xlonmax / view->mtodeglon;
			view->ymin = ylatmin / view->mtodeglat;
			view->ymax = ylatmax / view->mtodeglat;

			/* get origin */
			view->xorigin = 0.5 * (view->xmin + view->xmax);
			view->yorigin = 0.5 * (view->ymin + view->ymax);
			view->zorigin = data->exageration * 0.5 * (data->primary_min + data->primary_max);
			}
		else if (data->display_projection_mode == MBV_PROJECTION_SPHEROID)
			{
			/* get bounds */
			if (xlonmax - xlonmin >= 180.0
				|| ylatmax - ylatmin >= 90.0)
				{
				/* setup spheroid 3D projection with view towards the center of the grid */
				mbview_spheroid_setup(instance, MB_YES,
							0.5 * (xlonmin + xlonmax),
							0.5 * (ylatmin + ylatmax));

				view->xmin = -MBV_SPHEROID_RADIUS;
				view->xmax = MBV_SPHEROID_RADIUS;
				view->ymin = -MBV_SPHEROID_RADIUS;
				view->ymax = MBV_SPHEROID_RADIUS;

				/* get reference */
				view->sphere_refx = 0.0;
				view->sphere_refy = 0.0;
				view->sphere_refz = 0.0;

				/* get origin */
				view->xorigin = 0.0;
				view->yorigin = 0.0;
				view->zorigin = 0.0;
				}
			else
				{
				/* setup spheroid 3D projection with view towards the center of the grid */
				mbview_spheroid_setup(instance, MB_NO,
							0.5 * (xlonmin + xlonmax),
							0.5 * (ylatmin + ylatmax));

				/* get origin */
				mbview_spheroid_forward(instance,
							0.5 * (xlonmin + xlonmax),
							0.5 * (ylatmin + ylatmax),
							&view->sphere_refx, 
							&view->sphere_refy, 
							&view->sphere_refz);

				mbview_spheroid_forward(instance,
							xlonmin, 
							ylatmin,
							&view->xmin, 
							&view->ymin, 
							&zdisplay);
				mbview_spheroid_forward(instance,
							xlonmax, 
							ylatmax,
							&view->xmax, 
							&view->ymax, 
							&zdisplay);
				view->xmin -= view->sphere_refx;
				view->xmax -= view->sphere_refx;
				view->ymin -= view->sphere_refy;
				view->ymax -= view->sphere_refy;

				/* get origin */
				mbview_spheroid_forward(instance,
							0.5 * (xlonmin + xlonmax),
							0.5 * (ylatmin + ylatmax),
							&view->xorigin, 
							&view->yorigin, 
							&view->zorigin);
				
				view->xorigin -= view->sphere_refx;
				view->yorigin -= view->sphere_refy;
				view->zorigin += 0.5 * (data->primary_min + data->primary_max) 
						- view->sphere_refz;
				}
			}
		}
		
	/* get origin and scaling */
	view->scale = MIN((1.75 * MBV_OPENGL_WIDTH 
					/ (view->xmax - view->xmin)),
				(1.75 * MBV_OPENGL_WIDTH  
					/ view->aspect_ratio
					/ (view->ymax - view->ymin)));
	view->size2d = 1.0;

fprintf(stderr,"Projections:\n");
fprintf(stderr,"  Grid: mode:%d id:%s\n", 
data->primary_grid_projection_mode, data->primary_grid_projection_id);
fprintf(stderr,"  Display: mode:%d id:%s\n", 
data->display_projection_mode, data->display_projection_id);
fprintf(stderr,"  Display min max: %f %f %f %f\n",
view->xmin, view->xmax, view->ymin, view->ymax);
fprintf(stderr,"  Display origin: %f %f %f\n", view->xorigin, view->yorigin, view->zorigin);
fprintf(stderr,"  Display scale: %f\n", view->scale);

	/* set x and y arrays */
	for (i=0;i<data->primary_nx;i++)
	{
	for (j=0;j<data->primary_ny;j++)
		{
		/* get raw values in grid */
		k = i * data->primary_ny + j;
		xgrid = data->primary_xmin + i * data->primary_dx;
		ygrid = data->primary_ymin + j * data->primary_dy;
		
		/* reproject positions into display coordinates */
		mbview_projectforward(instance, MB_NO,
					xgrid, ygrid, data->primary_data[k],
					&xlon, &ylat,
					&xdisplay, &ydisplay, &zdisplay);
					
		/* insert into plotting arrays */
		data->primary_x[k] = (float)xdisplay;
		data->primary_y[k] = (float)ydisplay;
		data->primary_z[k] = (float)zdisplay;
		}

	/* set projection for secondary grid if needed */
	if (data->secondary_nxy > 0
		&& data->secondary_grid_projection_mode == MBV_PROJECTION_PROJECTED)
		{
		/* set projection for getting lon lat */
		proj_status = mb_proj_init(mbv_verbose, 
					data->secondary_grid_projection_id,
					&(view->secondary_pjptr),
					&error);
		if (proj_status == MB_SUCCESS)
			view->secondary_pj_init = MB_YES;
		}
		
	/* check for pending event */
	if (view->plot_done == MB_NO 
		&& view->plot_interrupt_allowed == MB_YES 
		&& i % MBV_EVENTCHECKCOARSENESS == 0)
		do_mbview_xevents();
		
	/* dump out of loop if plotting already done at a higher recursion */
	if (view->plot_done == MB_YES)
		i = data->primary_nx;
	}

	/* calculate derivatives of primary data */
	for (i=0;i<data->primary_nx;i++)
	{
	for (j=0;j<data->primary_ny;j++)
		{
		/* figure if x derivative can be calculated */
		derivative_ok = MB_NO;
		if (i == 0)
		    {
		    k1 = i * data->primary_ny + j;
		    k2 = (i + 1) * data->primary_ny + j;
		    if (data->primary_data[k1] != data->primary_nodatavalue
		    	&& data->primary_data[k2] != data->primary_nodatavalue)
			derivative_ok = MB_YES;
		    }
		else if (i == data->primary_nx - 1)
		    {
		    k1 = (i - 1) * data->primary_ny + j;
		    k2 = i * data->primary_ny + j;
		    if (data->primary_data[k1] != data->primary_nodatavalue
		    	&& data->primary_data[k2] != data->primary_nodatavalue)
			derivative_ok = MB_YES;
		    }
		else
		    {
		    k1 = (i - 1) * data->primary_ny + j;
		    k = i * data->primary_ny + j;
		    k2 = (i + 1) * data->primary_ny + j;
		    if (data->primary_data[k1] != data->primary_nodatavalue
		    	&& data->primary_data[k2] != data->primary_nodatavalue)
			derivative_ok = MB_YES;
		    else if (data->primary_data[k1] != data->primary_nodatavalue
		    	&& data->primary_data[k] != data->primary_nodatavalue)
			{
			derivative_ok = MB_YES;
			k2 = k;
			}
		    else if (data->primary_data[k] != data->primary_nodatavalue
		    	&& data->primary_data[k2] != data->primary_nodatavalue)
			{
			derivative_ok = MB_YES;
			k1 = k;
			}
		    }
		
		/* calculate x derivative */
		if (derivative_ok == MB_YES)
			{
			dx = (data->primary_x[k2] 
				- data->primary_x[k1]);
			if (dx != 0.0)
			data->primary_dzdx[k] 
				= view->scale * 
					(data->primary_data[k2] 
						- data->primary_data[k1])
					/ dx;
			else
			data->primary_dzdx[k] = 0.0;
			}
		else
			data->primary_dzdx[k] = 0.0;
		
		/* figure if y derivative can be calculated */
		derivative_ok = MB_NO;
		if (j == 0)
		    {
		    k1 = i * data->primary_ny + j;
		    k2 = i * data->primary_ny + (j + 1);
		    if (data->primary_data[k1] != data->primary_nodatavalue
		    	&& data->primary_data[k2] != data->primary_nodatavalue)
			derivative_ok = MB_YES;
		    }
		else if (i == data->primary_ny - 1)
		    {
		    k1 = i * data->primary_ny + (j - 1);
		    k2 = i * data->primary_ny + j;
		    if (data->primary_data[k1] != data->primary_nodatavalue
		    	&& data->primary_data[k2] != data->primary_nodatavalue)
			derivative_ok = MB_YES;
		    }
		else
		    {
		    k1 = i * data->primary_ny + (j - 1);
		    k = i * data->primary_ny + j;
		    k2 = i * data->primary_ny + (j + 1);
		    if (data->primary_data[k1] != data->primary_nodatavalue
		    	&& data->primary_data[k2] != data->primary_nodatavalue)
			derivative_ok = MB_YES;
		    else if (data->primary_data[k1] != data->primary_nodatavalue
		    	&& data->primary_data[k] != data->primary_nodatavalue)
			{
			derivative_ok = MB_YES;
			k2 = k;
			}
		    else if (data->primary_data[k] != data->primary_nodatavalue
		    	&& data->primary_data[k2] != data->primary_nodatavalue)
			{
			derivative_ok = MB_YES;
			k1 = k;
			}
		    }
		
		/* calculate y derivative */
		if (derivative_ok == MB_YES)
			{
			dy = (data->primary_y[k2] 
				- data->primary_y[k1]);
			if (dy != 0.0)
			data->primary_dzdy[k] 
				= view->scale * 
					(data->primary_data[k2] 
						- data->primary_data[k1])
					/ (data->primary_y[k2] 
						- data->primary_y[k1]);
			else
			data->primary_dzdy[k] = 0.0;
			}
		else
			data->primary_dzdy[k] = 0.0;
		}
		
	/* check for pending event */
	if (view->plot_done == MB_NO 
		&& view->plot_interrupt_allowed == MB_YES 
		&& i % MBV_EVENTCHECKCOARSENESS == 0)
		do_mbview_xevents();
		
	/* dump out of loop if plotting already done at a higher recursion */
	if (view->plot_done == MB_YES)
		i = data->primary_nx;
	}
		
	/* clear zscale for grid */
	mbview_zscaleclear(instance);
		
	/* project and scale data other than the grid */
	mbview_zscale(instance);
	
	/* set projected flag only if plotting not done */
	if (view->plot_done == MB_NO)
		{
		view->projected = MB_YES;
		}

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return(status);
}
/*------------------------------------------------------------------------------*/
int mbview_zscalegridpoint(int instance, int k)
{
	/* local variables */
	char	*function_name = "mbview_zscalegridpoint";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	double	xgrid, ygrid;
	double	xlon, ylat;
	double	xdisplay, ydisplay, zdisplay;
	int	i, j, l;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       view:             %d\n",view);
		fprintf(stderr,"dbg2       data:             %d\n",data);
		fprintf(stderr,"dbg2       k:                %d\n",k);
		}
if (mbv_verbose >= 2)
fprintf(stderr,"mbview_zscalegridpoint: %d %d\n", i, j);
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* scale z value */
	if (data->display_projection_mode == MBV_PROJECTION_PROJECTED
		|| data->display_projection_mode == MBV_PROJECTION_ALREADYPROJECTED
		|| data->display_projection_mode == MBV_PROJECTION_GEOGRAPHIC)
		{
		/* scale z value alone */
		data->primary_z[k] 
			= (float)(view->scale 
				* (data->exageration * data->primary_data[k] - view->zorigin));
		}
	else if (data->display_projection_mode == MBV_PROJECTION_SPHEROID)
		{
		/* must reproject everything in this case */
		i = k / data->primary_ny;
		j = k % data->primary_ny;
		xgrid = data->primary_xmin + i * data->primary_dx;
		ygrid = data->primary_ymin + j * data->primary_dy;
		
		/* reproject positions into display coordinates */
		mbview_projectforward(instance, MB_NO,
					xgrid, ygrid, data->primary_data[k],
					&xlon, &ylat,
					&xdisplay, &ydisplay, &zdisplay);
					
		/* insert into plotting arrays */
		data->primary_x[k] = (float)xdisplay;
		data->primary_y[k] = (float)ydisplay;
		data->primary_z[k] = (float)zdisplay;
		}

	/* set zscale status bit */
	data->primary_stat_z[k/8] 
		= data->primary_stat_z[k/8] | statmask[k%8];

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_zscalepoint(int instance, int global, double offset_factor, 
			struct mbview_point_struct *point)
{
	/* local variables */
	char	*function_name = "mbview_zscale";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	int	i, j, k, l;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       global:           %d\n",global);
		fprintf(stderr,"dbg2       offset_factor:    %f\n",offset_factor);
		}
if (mbv_verbose >= 2)
fprintf(stderr,"mbview_zscalepoint: %d\n", instance);
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* scale z value */
	if (data->display_projection_mode != MBV_PROJECTION_SPHEROID)
		{
		/* scale z value alone */
		point->zdisplay = view->scale * (data->exageration * point->zdata - view->zorigin)
					+ offset_factor;
		}
	else
		{
		/* reproject positions into display coordinates */
		mbview_projectforward(instance, MB_NO,
					point->xgrid, point->ygrid, point->zdata,
					&point->xlon, &point->ylat,
					&point->xdisplay, &point->ydisplay, &point->zdisplay);
					
		if (global == MB_NO)
			{
			point->zdisplay += offset_factor;
			}
		else
			{
			point->xdisplay += point->xdisplay * offset_factor;
			point->ydisplay += point->ydisplay * offset_factor;
			point->zdisplay += point->zdisplay * offset_factor;
			}
		}

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return(status);
}
/*------------------------------------------------------------------------------*/
int mbview_zscale(int instance)
{
	/* local variables */
	char	*function_name = "mbview_zscale";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	int	i, j, k, l;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	global;
	double	offset_factor;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		}
if (mbv_verbose >= 2)
fprintf(stderr,"mbview_zscale: %d\n", instance);
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* check if the contour offset needs to be applied in a global spherical direction or just up */
	if (data->display_projection_mode == MBV_PROJECTION_SPHEROID
		&& view->sphere_refx == 0.0
		&& view->sphere_refy == 0.0
		&& view->sphere_refz == 0.0)
		{
		global = MB_YES;
		offset_factor = 10.0 * MBV_OPENGL_3D_CONTOUR_OFFSET / (view->scale * MBV_SPHEROID_RADIUS);
		}
	else
		{
		global = MB_NO;
		offset_factor = MBV_OPENGL_3D_CONTOUR_OFFSET;
		}

	/* handle picks */
	if (data->pick_type != MBV_PICK_NONE)
		{
		mbview_zscalepoint(instance, global, offset_factor, &(data->pick.endpoints[0]));
		for (i=0;i<4;i++)
			{
			mbview_zscalepoint(instance, global, offset_factor, &(data->pick.xpoints[i]));
			}
		for (i=0;i<2;i++)
			{
			for (j=0;j<data->pick.xsegments[i].nls;j++)
				{
				mbview_zscalepoint(instance, global, offset_factor, &(data->pick.xsegments[i].lspoints[j]));
				}
			}
		}
	if (data->pick_type == MBV_PICK_TWOPOINT)
		{
		mbview_zscalepoint(instance, global, offset_factor, &(data->pick.endpoints[1]));
		for (i=4;i<8;i++)
			{
			mbview_zscalepoint(instance, global, offset_factor, &(data->pick.xpoints[i]));
			}
		for (i=2;i<4;i++)
			{
			for (j=0;j<data->pick.xsegments[i].nls;j++)
				{
				mbview_zscalepoint(instance, global, offset_factor, &(data->pick.xsegments[i].lspoints[j]));
				}
			}
		if (data->pick.segment.nls > 0)
			{
			for (j=0;j<data->pick.segment.nls;j++)
				{
				mbview_zscalepoint(instance, global, offset_factor, &(data->pick.segment.lspoints[j]));
				}
			}
		}
		
	/* handle area */
	if (data->area_type == MBV_AREA_QUAD)
		{
		for (i=0;i<2;i++)
			{
			mbview_zscalepoint(instance, global, offset_factor, &(data->area.endpoints[i]));
			}
		for (j=0;j<data->area.segment.nls;j++)
			{
			mbview_zscalepoint(instance, global, offset_factor, &(data->area.segment.lspoints[j]));
			}
		for (i=0;i<4;i++)
			{
			for (j=0;j<2;j++)
				{
				mbview_zscalepoint(instance, global, offset_factor, data->area.segments[i].endpoints[j]);
				}
			for (j=0;j<data->area.segments[i].nls;j++)
				{
				mbview_zscalepoint(instance, global, offset_factor, &(data->area.segments[i].lspoints[j]));
				}
			}
		}
		
	/* handle region */
	if (data->region_type == MBV_REGION_QUAD)
		{
		for (i=0;i<4;i++)
			{
			mbview_zscalepoint(instance, global, offset_factor, &(data->region.cornerpoints[i]));
			for (j=0;j<data->region.segments[i].nls;j++)
				{
				mbview_zscalepoint(instance, global, offset_factor, &(data->region.segments[i].lspoints[j]));
				}
			}
		}
	
	/* handle navpicks */
	if (data->navpick_type != MBV_PICK_NONE)
		{
		mbview_zscalepoint(instance, global, offset_factor, &(data->navpick.endpoints[0]));
		for (i=0;i<4;i++)
			{
			mbview_zscalepoint(instance, global, offset_factor, &(data->navpick.xpoints[i]));
			}
		for(i=0;i<2;i++)
			{
			if (data->navpick.xsegments[i].nls > 0)
				{
				for (j=0;j<data->navpick.xsegments[i].nls;j++)
					{
					mbview_zscalepoint(instance, global, offset_factor, 
								&(data->navpick.xsegments[i].lspoints[j]));
					}
				}
			}
		}
	if (data->navpick_type == MBV_PICK_TWOPOINT)
		{
		mbview_zscalepoint(instance, global, offset_factor, &(data->navpick.endpoints[1]));
		for (i=4;i<8;i++)
			{
			mbview_zscalepoint(instance, global, offset_factor, &(data->navpick.xpoints[i]));
			}
		for(i=2;i<4;i++)
			{
			if (data->navpick.xsegments[i].nls > 0)
				{
				for (j=0;j<data->navpick.xsegments[i].nls;j++)
					{
					mbview_zscalepoint(instance, global, offset_factor, 
								&(data->navpick.xsegments[i].lspoints[j]));
					}
				}
			}
		}
		
	/* handle sites */
	if (data->nsite > 0)
		{
		for (i=0;i<data->nsite;i++)
			{
			mbview_zscalepoint(instance, global, offset_factor, &(data->sites[i].point));
			}
		}
		
	/* handle routes */
	if (data->nroute > 0)
		{
		for (i=0;i<data->nroute;i++)
		    {
		    for (j=0;j<data->routes[i].npoints;j++)
			{
			mbview_zscalepoint(instance, global, offset_factor, &(data->routes[i].points[j]));
			}
		    for (j=0;j<data->routes[i].npoints-1;j++)
			{
			for (k=0;k<data->routes[i].segments[j].nls;k++)
				{
				mbview_zscalepoint(instance, global, offset_factor, &(data->routes[i].segments[j].lspoints[k]));
				}
			}
		    }
		}
		
	/* handle nav */
	if (data->nnav > 0)
		{
		for (i=0;i<data->nnav;i++)
		    {
		    for (j=0;j<data->navs[i].npoints;j++)
			{
			mbview_zscalepoint(instance, global, offset_factor, &(data->navs[i].navpts[j].point));
			mbview_zscalepoint(instance, global, offset_factor, &(data->navs[i].navpts[j].pointport));
			mbview_zscalepoint(instance, global, offset_factor, &(data->navs[i].navpts[j].pointcntr));
			mbview_zscalepoint(instance, global, offset_factor, &(data->navs[i].navpts[j].pointstbd));
			}
		    for (j=0;j<data->navs[i].npoints-1;j++)
			{
			for (k=0;k<data->navs[i].segments[j].nls;k++)
				{
				mbview_zscalepoint(instance, global, offset_factor, &(data->navs[i].segments[j].lspoints[k]));
				}
			}
		    }
		}
	
	/* set rez flags only if plotting not done */
	if (view->plot_done == MB_NO)
		{
        	view->contourlorez = MB_NO;
        	view->contourhirez = MB_NO;
        	view->contourfullrez = MB_NO;
		}

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_projectforward(int instance, int needlonlat,
				double xgrid, double ygrid, double zdata,
				double *xlon, double *ylat, 
				double *xdisplay, double *ydisplay, double *zdisplay)
{
	/* local variables */
	char	*function_name = "mbview_projectforward";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	int	proj_status = MB_SUCCESS;
	double	xx, yy, zz;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       needlonlat:       %d\n",needlonlat);
		fprintf(stderr,"dbg2       xgrid:            %f\n",xgrid);
		fprintf(stderr,"dbg2       ygrid:            %f\n",ygrid);
		fprintf(stderr,"dbg2       zdata:            %f\n",zdata);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
		
	/* get positions into geographic coordinates if necessary */
	if (needlonlat == MB_YES
		|| data->primary_grid_projection_mode != MBV_PROJECTION_ALREADYPROJECTED)
		{
		status = mbview_projectgrid2ll(instance,
						xgrid, ygrid,
						xlon, ylat);
		}

	/* get positions in the display projection */
	if (data->primary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED)
		{
		xx = xgrid;
		yy = ygrid;
		zz = data->exageration * zdata;
		*xdisplay = view->scale * (xx - view->xorigin);
		*ydisplay = view->scale * (yy - view->yorigin);
		*zdisplay = view->scale * (zz - view->zorigin);
		}
	else
		{
		status = mbview_projectll2display(instance,
				*xlon, *ylat, zdata,
				xdisplay, ydisplay, zdisplay);
		}

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       xlon:        %f\n",*xlon);
		fprintf(stderr,"dbg2       ylat:        %f\n",*ylat);
		fprintf(stderr,"dbg2       xdisplay:    %f\n",*xdisplay);
		fprintf(stderr,"dbg2       ydisplay:    %f\n",*ydisplay);
		fprintf(stderr,"dbg2       zdisplay:    %f\n",*zdisplay);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_projectinverse(int instance, int needlonlat,
				double xdisplay, double ydisplay,double zdisplay,
				double *xlon, double *ylat,
				double *xgrid, double *ygrid)
{
	/* local variables */
	char	*function_name = "mbview_projectinverse";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	int	proj_status = MB_SUCCESS;
	double	xx, yy, zz;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       needlonlat:       %d\n",needlonlat);
		fprintf(stderr,"dbg2       xdisplay:         %f\n",xdisplay);
		fprintf(stderr,"dbg2       ydisplay:         %f\n",ydisplay);
		fprintf(stderr,"dbg2       zdisplay:         %f\n",zdisplay);
		}
	
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* get positions in geographic coordinates */
	if (needlonlat == MB_YES
		|| data->primary_grid_projection_mode != MBV_PROJECTION_ALREADYPROJECTED)
		{
		status = mbview_projectdisplay2ll(instance,
						xdisplay, ydisplay, zdisplay,
						xlon, ylat);
		}

	/* get positions into grid coordinates */
	if (data->primary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED)
		{
		xx = xdisplay / view->scale + view->xorigin;
		yy = ydisplay / view->scale + view->yorigin;
		*xgrid = xx;
		*ygrid = yy;
		}
	else
		{
		status = mbview_projectll2xygrid(instance,
						*xlon,  *ylat,
						xgrid, ygrid);
		}

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       xlon:         %f\n",*xlon);
		fprintf(stderr,"dbg2       ylat:         %f\n",*ylat);
		fprintf(stderr,"dbg2       xgrid:        %f\n",*xgrid);
		fprintf(stderr,"dbg2       ygrid:        %f\n",*ygrid);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:       %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_projectfromlonlat(int instance,
				double xlon, double ylat, double zdata,
				double *xgrid, double *ygrid,
				double *xdisplay, double *ydisplay, double *zdisplay)
{
	/* local variables */
	char	*function_name = "mbview_projectfromlonlat";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       xlon:             %f\n",xlon);
		fprintf(stderr,"dbg2       ylat:             %f\n",ylat);
		fprintf(stderr,"dbg2       zdata:            %f\n",zdata);
		}
	
	/* get positions into grid coordinates */
	status = mbview_projectll2xygrid(instance,
					xlon, ylat,
			 		xgrid, ygrid);

	/* get positions in the display projection */
	status = mbview_projectll2display(instance,
				xlon, ylat, zdata,
				xdisplay, ydisplay, zdisplay);

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       xgrid:       %f\n",*xgrid);
		fprintf(stderr,"dbg2       ygrid:       %f\n",*ygrid);
		fprintf(stderr,"dbg2       xdisplay:    %f\n",*xdisplay);
		fprintf(stderr,"dbg2       ydisplay:    %f\n",*ydisplay);
		fprintf(stderr,"dbg2       zdisplay:    %f\n",*zdisplay);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}


/*------------------------------------------------------------------------------*/
int mbview_projectgrid2ll(int instance,
				double xgrid, double ygrid,
				double *xlon, double *ylat)
{
	/* local variables */
	char	*function_name = "mbview_projectgrid2ll";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	int	proj_status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	double	xx, yy;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       xgrid:            %f\n",xgrid);
		fprintf(stderr,"dbg2       ygrid:            %f\n",ygrid);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* get positions into geographic coordinates */
	if (data->primary_grid_projection_mode == MBV_PROJECTION_PROJECTED
		|| data->primary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED)
		{
		mb_proj_inverse(mbv_verbose, view->primary_pjptr,
					xgrid, ygrid, xlon, ylat, &error);
		}
	else if (data->primary_grid_projection_mode == MBV_PROJECTION_GEOGRAPHIC)
		{
		*xlon = xgrid;
		*ylat = ygrid;
		}

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       xlon:             %f\n",*xlon);
		fprintf(stderr,"dbg2       ylat:             %f\n",*ylat);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_projectll2xygrid(int instance,
				double xlon, double ylat,
				double *xgrid, double *ygrid)
{
	/* local variables */
	char	*function_name = "mbview_projectll2xygrid";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	int	proj_status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	double	xx, yy;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       xlon:             %f\n",xlon);
		fprintf(stderr,"dbg2       ylat:             %f\n",ylat);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* get positions into grid coordinates */
	if (data->primary_grid_projection_mode == MBV_PROJECTION_PROJECTED
		|| data->primary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED)
		{
		mb_proj_forward(mbv_verbose, view->primary_pjptr,
				xlon, ylat, xgrid, ygrid, &error);
		}
	else
		{
		*xgrid = xlon;
		*ygrid = ylat;
		}

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       xgrid:       %f\n",*xgrid);
		fprintf(stderr,"dbg2       ygrid:       %f\n",*ygrid);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_projectll2xyzgrid(int instance,
				double xlon, double ylat,
				double *xgrid, double *ygrid, double *zdata)
{
	/* local variables */
	char	*function_name = "mbview_projectll2xyzgrid";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	int	proj_status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	double	xx, yy;
	int	nfound;
	int	i, j, k, ii, jj;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       xlon:             %f\n",xlon);
		fprintf(stderr,"dbg2       ylat:             %f\n",ylat);
		fprintf(stderr,"dbg2       zdata:            %f\n",zdata);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* get positions into grid coordinates */
	if (data->primary_grid_projection_mode == MBV_PROJECTION_PROJECTED
		|| data->primary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED)
		{
		mb_proj_forward(mbv_verbose, view->primary_pjptr,
				xlon, ylat, xgrid, ygrid, &error);
		}
	else
		{
		*xgrid = xlon;
		*ygrid = ylat;
		}
		
	/* now get zdata  from primary grid */
	nfound = 0;
	*zdata = 0.0;
	i = (int)((*xgrid - data->primary_xmin) / data->primary_dx);
	j = (int)((*ygrid - data->primary_ymin) / data->primary_dy);
	if (i >= 0 && i < data->primary_nx - 1
	    && j >= 0 && j < data->primary_ny - 1)
		{
		for (ii=i;ii<=i+1;ii++)
		for (jj=j;jj<=j+1;jj++)
		    {
		    k = ii * data->primary_ny + jj;
		    if (data->primary_data[k] != data->primary_nodatavalue)
			{
			nfound++;
			*zdata += data->primary_data[k];
			}
		    }
		}
	if (nfound > 0)
		{
		*zdata /= (double)nfound;
		status = MB_SUCCESS;
		}
	else
		{
		*zdata = 0.0;
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       xgrid:       %f\n",*xgrid);
		fprintf(stderr,"dbg2       ygrid:       %f\n",*ygrid);
		fprintf(stderr,"dbg2       zdata:       %f\n",*zdata);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_projectll2display(int instance,
				double xlon, double ylat, double zdata,
				double *xdisplay, double *ydisplay, double *zdisplay)
{
	/* local variables */
	char	*function_name = "mbview_projectll2display";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	int	proj_status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	double	xx, yy, zz;
	double	effective_topography;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       xlon:             %f\n",xlon);
		fprintf(stderr,"dbg2       ylat:             %f\n",ylat);
		fprintf(stderr,"dbg2       zdata:            %f\n",zdata);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* get positions in the display projection */
	if (data->display_projection_mode == MBV_PROJECTION_PROJECTED
		|| data->display_projection_mode == MBV_PROJECTION_ALREADYPROJECTED)
		{
		mb_proj_forward(mbv_verbose, view->display_pjptr,
				xlon, 
				ylat, 
				&xx,
				&yy,
				&error);
		zz = data->exageration * zdata;
		}
	else if (data->display_projection_mode == MBV_PROJECTION_GEOGRAPHIC)
		{
		xx = xlon / view->mtodeglon;
		yy = ylat / view->mtodeglat;
		zz = data->exageration * zdata;
		}
	else if (data->display_projection_mode == MBV_PROJECTION_SPHEROID)
		{
		mbview_spheroid_forward(instance, xlon, ylat, &xx, &yy, &zz);
		effective_topography = data->exageration * (zdata - 0.5 * (data->primary_min + data->primary_max))
					+ 0.5 * (data->primary_min + data->primary_max);
/*fprintf(stderr,"pos: %f %f %f   raw: %f %f %f  topo:%f ",
xlon, ylat, zdata, xx, yy, zz, effective_topography);*/

		xx += (effective_topography * xx / MBV_SPHEROID_RADIUS) - view->sphere_refx;
		yy += (effective_topography * yy / MBV_SPHEROID_RADIUS) - view->sphere_refy;
		zz += (effective_topography * zz / MBV_SPHEROID_RADIUS) - view->sphere_refz;
/*fprintf(stderr,"unscaled: %f %f %f",
xx, yy, zz);*/
		}

	/* get final positions in display coordinates */
	*xdisplay = view->scale * (xx - view->xorigin);
	*ydisplay = view->scale * (yy - view->yorigin);
	*zdisplay = view->scale * (zz - view->zorigin);
/*fprintf(stderr,"   scale:%f   scaled: %f %f %f\n",
view->scale, *xdisplay, *ydisplay, *zdisplay);*/

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       xdisplay:    %f\n",*xdisplay);
		fprintf(stderr,"dbg2       ydisplay:    %f\n",*ydisplay);
		fprintf(stderr,"dbg2       zdisplay:    %f\n",*zdisplay);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_projectdisplay2ll(int instance,
				double xdisplay, double ydisplay, double zdisplay,
				double *xlon, double *ylat)
{
	/* local variables */
	char	*function_name = "mbview_projectdisplay2ll";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	int	proj_status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	double	xx, yy, zz;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       xdisplay:         %f\n",xdisplay);
		fprintf(stderr,"dbg2       ydisplay:         %f\n",ydisplay);
		fprintf(stderr,"dbg2       zdisplay:         %f\n",zdisplay);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* get positions in display projection */
	xx = xdisplay / view->scale + view->xorigin;
	yy = ydisplay / view->scale + view->yorigin;
	zz = zdisplay / view->scale + view->zorigin;

	/* get positions in geographic coordinates */
	if (data->display_projection_mode == MBV_PROJECTION_PROJECTED
		|| data->display_projection_mode == MBV_PROJECTION_ALREADYPROJECTED)
		{
		mb_proj_inverse(mbv_verbose, view->display_pjptr,
				xx, yy, xlon, ylat, &error);
		}
	else if (data->display_projection_mode == MBV_PROJECTION_GEOGRAPHIC)
		{
		*xlon = xx * view->mtodeglon;
		*ylat = yy * view->mtodeglat;
		}
	else if (data->display_projection_mode == MBV_PROJECTION_SPHEROID)
		{
		xx += view->sphere_refx;
		yy += view->sphere_refy;
		zz += view->sphere_refz;
		mbview_spheroid_inverse(instance, xx, yy, zz, xlon, ylat);
		}

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       xlon:             %f\n",*xlon);
		fprintf(stderr,"dbg2       ylat:             %f\n",*ylat);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*------------------------------------------------------------------------------*/
int mbview_projectdistance(int instance,
				double xlon1, double ylat1, double zdata1,
				double xlon2, double ylat2, double zdata2,
				double *distancelateral, 
				double *distanceoverground,
				double *slope)
{
	/* local variables */
	char	*function_name = "mbview_projectdistance";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	int	proj_status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	double	xx1, yy1, zz1;
	double	xx2, yy2, zz2;
	double	dx, dy, dz;
	double	bearing;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       xlon1:            %f\n",xlon1);
		fprintf(stderr,"dbg2       ylat1:            %f\n",ylat1);
		fprintf(stderr,"dbg2       zdata1:           %f\n",zdata1);
		fprintf(stderr,"dbg2       xlon2:            %f\n",xlon2);
		fprintf(stderr,"dbg2       ylat2:            %f\n",ylat2);
		fprintf(stderr,"dbg2       zdata2:           %f\n",zdata2);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* get positions in display projection without scaling or exageration */
	if (data->display_projection_mode == MBV_PROJECTION_PROJECTED
		|| data->display_projection_mode == MBV_PROJECTION_ALREADYPROJECTED)
		{
		/* point 1 */
		mb_proj_forward(mbv_verbose, view->display_pjptr,
				xlon1, 
				ylat1, 
				&xx1,
				&yy1,
				&error);
		zz1 = zdata1;

		/* point 2 */
		mb_proj_forward(mbv_verbose, view->display_pjptr,
				xlon2, 
				ylat2, 
				&xx2,
				&yy2,
				&error);
		zz2 = zdata2;

		/* distance and slope */
		dx = xx2 - xx1;
		dy = yy2 - yy1;
		dz = zz2 - zz1;
		*distancelateral = sqrt(dx * dx + dy * dy);
		*distanceoverground = sqrt(dx * dx + dy * dy + dz * dz);
		if (*distancelateral > 0.0)
			*slope = dz / (*distancelateral);
		else
			*slope = 0.0;
		}
	else if (data->display_projection_mode == MBV_PROJECTION_GEOGRAPHIC)
		{
		/* point 1 */
		xx1 = xlon1 / view->mtodeglon;
		yy1 = ylat1 / view->mtodeglat;
		zz1 = zdata1;

		/* point 2 */
		xx2 = xlon2 / view->mtodeglon;
		yy2 = ylat2 / view->mtodeglat;
		zz2 = zdata2;

		/* distance and slope */
		dx = xx2 - xx1;
		dy = yy2 - yy1;
		dz = zz2 - zz1;
		*distancelateral = sqrt(dx * dx + dy * dy);
		*distanceoverground = sqrt(dx * dx + dy * dy + dz * dz);
		if (*distancelateral > 0.0)
			*slope = dz / (*distancelateral);
		else
			*slope = 0.0;
		}
	else if (data->display_projection_mode == MBV_PROJECTION_SPHEROID)
		{
		/* point 1 */
		mbview_spheroid_forward(instance, xlon1, ylat1, &xx1, &yy1, &zz1);

		/* point 2 */
		mbview_spheroid_forward(instance, xlon2, ylat2, &xx2, &yy2, &zz2);

		/* lateral distance */
		mbview_greatcircle_distbearing(instance, 
			xlon1, ylat1, xlon2, ylat2,
			&bearing, distancelateral);

		/* distance over ground */
		xx1 += zdata1 * xx1 / MBV_SPHEROID_RADIUS;
		yy1 += zdata1 * yy1 / MBV_SPHEROID_RADIUS;
		zz1 += zdata1 * zz1 / MBV_SPHEROID_RADIUS;
		xx2 += zdata2 * xx2 / MBV_SPHEROID_RADIUS;
		yy2 += zdata2 * yy2 / MBV_SPHEROID_RADIUS;
		zz2 += zdata2 * zz2 / MBV_SPHEROID_RADIUS;
		dx = xx2 - xx1;
		dy = yy2 - yy1;
		dz = zz2 - zz1;
		*distanceoverground = sqrt(dx * dx + dy * dy + dz * dz);

		/* slope */
		if (*distancelateral > 0.0)
			*slope = (zdata2 - zdata1) / (*distancelateral);
		else
			*slope = 0.0;
		}

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       distancelateral:     %f\n",*distancelateral);
		fprintf(stderr,"dbg2       distanceoverground:  %f\n",*distanceoverground);
		fprintf(stderr,"dbg2       slope:               %f\n",*slope);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*------------------------------------------------------------------------------*/
int mbview_spheroid_setup(int instance, int earthcentered, double xlon, double ylat)
{
	/* local variables */
	char	*function_name = "mbview_spheroid_setup";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	double	phi, theta, psi;
	double	xx, yy, zz;
	int	i,j;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       earthcentered:    %d\n",earthcentered);
		fprintf(stderr,"dbg2       xlon:             %f\n",xlon);
		fprintf(stderr,"dbg2       ylat:             %f\n",ylat);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* The initial spherical coordinate system is defined as:
			x = r * cos(longitude) * cos(latitude)
			y = r * sin(longitude) * cos(latitude)
			z = r * sin(latitude)
	   which is equivalent to:
			x = r * cos(longitude) * sin(colatitude)
			y = r * sin(longitude) * sin(colatitude)
			z = r * cos(colatitude)
	   where:
	   		colatitude = PI/2 - latitude
	
	   Euler's rotation theorem proves than any general rotation may be
	   described by three successive rotations about the axes. One convention
	   is to use first a rotation about the z-axis (angle phi), then a
	   rotation about the x'-axis (angle theta), and finally a rotation
	   about the z''-axis (angle psi).
	    
	   The euler rotation matrix becomes: 
	   		|	a11	a12	a13	|
	   		|	a21	a22	a23	|
	   		|	a31	a32	a33	|
	   where:
			a11 = cos(phi) * cos(psi) - sin(phi) * cos(theta) * sin(psi)
			a12 = sin(phi) * cos(psi) + cos(phi) * cos(theta) * sin(psi)
			a13 = sin(theta) * sin (psi)
			a21 = -cos(phi) * sin(psi) - sin(phi) * cos(theta) * cos(psi)
			a22 = -sin(phi) * sin(psi) + cos(phi) * cos(theta) * cos(psi)
			a23 = sin(theta) * cos(psi)
			a31 = sin(phi) * sin(theta)
			a32 = -cos(phi) * sin(theta)
			a33 = cos(theta)
			
	   We wish to rotate the coordinate system so that the reference position
	   defined by xlon and ylat are located on the positive z-axis. The forward
	   rotation is accomplished using:
	   		phi = -PI/2 + xlon
			theta = -ycolat = ylat - PI/2
			psi = PI
	   The reverse rotation is accomplished using:
	   		phi = -PI
			theta = ycolat = PI/2 - ylat
			psi = xlon - PI/2
			
	  The relevant equations derived in part from:
		http://mathworld.wolfram.com/EulerAngles.html
	  which were viewed on January 19, 2004
		*/
	
	/* create forward rotation matrix */
	phi = DTR * xlon - 0.5 * M_PI;
	theta = DTR * ylat - 0.5 * M_PI;
	psi = M_PI;
	view->sphere_eulerforward[0] = cos(phi) * cos(psi) - sin(phi) * cos(theta) * sin(psi);
	view->sphere_eulerforward[1] = sin(phi) * cos(psi) + cos(phi) * cos(theta) * sin(psi);
	view->sphere_eulerforward[2] = sin(theta) * sin (psi);
	view->sphere_eulerforward[3] = -cos(phi) * sin(psi) - sin(phi) * cos(theta) * cos(psi);
	view->sphere_eulerforward[4] = -sin(phi) * sin(psi) + cos(phi) * cos(theta) * cos(psi);
	view->sphere_eulerforward[5] = sin(theta) * cos(psi);
	view->sphere_eulerforward[6] = sin(phi) * sin(theta);
	view->sphere_eulerforward[7] = -cos(phi) * sin(theta);
	view->sphere_eulerforward[8] = cos(theta);
	
	/* create reverse rotation matrix */
	phi = -M_PI;
	theta = 0.5 * M_PI - DTR * ylat;
	psi = 0.5 * M_PI - DTR * xlon;
	view->sphere_eulerreverse[0] = cos(phi) * cos(psi) - sin(phi) * cos(theta) * sin(psi);
	view->sphere_eulerreverse[1] = sin(phi) * cos(psi) + cos(phi) * cos(theta) * sin(psi);
	view->sphere_eulerreverse[2] = sin(theta) * sin (psi);
	view->sphere_eulerreverse[3] = -cos(phi) * sin(psi) - sin(phi) * cos(theta) * cos(psi);
	view->sphere_eulerreverse[4] = -sin(phi) * sin(psi) + cos(phi) * cos(theta) * cos(psi);
	view->sphere_eulerreverse[5] = sin(theta) * cos(psi);
	view->sphere_eulerreverse[6] = sin(phi) * sin(theta);
	view->sphere_eulerreverse[7] = -cos(phi) * sin(theta);
	view->sphere_eulerreverse[8] = cos(theta);
	
	/* now get reference location in rotated coordinates */
	view->sphere_reflon = xlon;
	view->sphere_reflat = ylat;
	view->sphere_refx = 0.0;
	view->sphere_refy = 0.0;
	view->sphere_refz = 0.0;
	if (earthcentered == MB_NO)
		{
		mbview_spheroid_forward(instance, xlon, ylat, 
				&view->sphere_refx,&view->sphere_refy, &view->sphere_refz);
		}

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Internal results:\n");
		fprintf(stderr,"dbg2       view->sphere_reflon:      %f\n",view->sphere_reflon);
		fprintf(stderr,"dbg2       view->sphere_reflat:      %f\n",view->sphere_reflat);
		fprintf(stderr,"dbg2       view->sphere_refx:        %f\n",view->sphere_refx);
		fprintf(stderr,"dbg2       view->sphere_refy:        %f\n",view->sphere_refy);
		fprintf(stderr,"dbg2       view->sphere_refz:        %f\n",view->sphere_refz);
		fprintf(stderr,"dbg2       view->sphere_eulerforward:\n");
		for (j=0;j<3;j++)
			{
			fprintf(stderr,"dbg2                         %f %f %f\n",
					 view->sphere_eulerforward[0 + 3 * j],
					 view->sphere_eulerforward[1 + 3 * j],
					 view->sphere_eulerforward[2 + 3 * j]);
			}
		fprintf(stderr,"dbg2       view->sphere_eulerreverse:\n");
		for (j=0;j<3;j++)
			{
			fprintf(stderr,"dbg2                         %f %f %f\n",
					 view->sphere_eulerreverse[0 + 3 * j],
					 view->sphere_eulerreverse[1 + 3 * j],
					 view->sphere_eulerreverse[2 + 3 * j]);
			}
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*------------------------------------------------------------------------------*/
int mbview_spheroid_forward(int instance, double xlon, double ylat,
			double *xx, double *yy, double *zz)
{
	/* local variables */
	char	*function_name = "mbview_spheroid_forward";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	double	sinlon, coslon, sinlat, coslat;
	double	posu[3], posr[3];
	int	i,j;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       xlon:             %f\n",xlon);
		fprintf(stderr,"dbg2       ylat:             %f\n",ylat);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* get position in initial cartesian coordinates */
	sinlon = sin(DTR * xlon);
	coslon = cos(DTR * xlon);
	sinlat = sin(DTR * ylat);
	coslat = cos(DTR * ylat);
	posu[0] = MBV_SPHEROID_RADIUS * coslon * coslat;
	posu[1] = MBV_SPHEROID_RADIUS * sinlon * coslat;
	posu[2] = MBV_SPHEROID_RADIUS * sinlat;
	
	/* apply rotation to coordinates with the reference location
		at the center of the view, on the positive z-axis. */
	for(i=0;i<3;i++)
		posr[i] = 0.0;
	for (j=0;j<3;j++)
		{
		for (i=0;i<3;i++)
			{
			posr[j] += posu[i] * view->sphere_eulerforward[i + 3 * j];
			}
		}

	/* make relative to reference location */
	*xx = posr[0];
	*yy = posr[1];
	*zz = posr[2];

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       posu[0]:     %f\n",posu[0]);
		fprintf(stderr,"dbg2       posu[1]:     %f\n",posu[1]);
		fprintf(stderr,"dbg2       posu[2]:     %f\n",posu[2]);
		fprintf(stderr,"dbg2       posr[0]:     %f\n",posr[0]);
		fprintf(stderr,"dbg2       posr[1]:     %f\n",posr[1]);
		fprintf(stderr,"dbg2       posr[2]:     %f\n",posr[2]);
		fprintf(stderr,"dbg2       xx:          %f\n",*xx);
		fprintf(stderr,"dbg2       yy:          %f\n",*yy);
		fprintf(stderr,"dbg2       zz:          %f\n",*zz);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_spheroid_inverse(int instance, double xx, double yy, double zz, 
			double *xlon, double *ylat)
{
	/* local variables */
	char	*function_name = "mbview_spheroid_inverse";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	double	posu[3], posr[3];
	int	i,j;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       xx:               %f\n",xx);
		fprintf(stderr,"dbg2       yy:               %f\n",yy);
		fprintf(stderr,"dbg2       zz:               %f\n",zz);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* get position in cartesian spheroid coordinates */
	posr[0] = xx;
	posr[1] = yy;
	posr[2] = zz;
	
	/* unrotate position */
	for(i=0;i<3;i++)
		posu[i] = 0.0;
	for (j=0;j<3;j++)
		{
		for (i=0;i<3;i++)
			{
			posu[j] += posr[i] * view->sphere_eulerreverse[i + 3 * j];
			}
		}
	
	/* get longitude and latitude */
	*xlon = RTD * atan2(posu[1], posu[0]);
	*ylat = 90.0 - RTD * (atan2(sqrt(posu[0] * posu[0] + posu[1] * posu[1]), posu[2]));

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       posr[0]:     %f\n",posr[0]);
		fprintf(stderr,"dbg2       posr[1]:     %f\n",posr[1]);
		fprintf(stderr,"dbg2       posr[2]:     %f\n",posr[2]);
		fprintf(stderr,"dbg2       posu[0]:     %f\n",posu[0]);
		fprintf(stderr,"dbg2       posu[1]:     %f\n",posu[1]);
		fprintf(stderr,"dbg2       posu[2]:     %f\n",posu[2]);
		fprintf(stderr,"dbg2       xlon:        %f\n",*xlon);
		fprintf(stderr,"dbg2       ylat:        %f\n",*ylat);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_greatcircle_distbearing(int instance, 
			double lon1, double lat1, double lon2,  double lat2,
			double *bearing, double *distance)
{
	/* local variables */
	char	*function_name = "mbview_greatcircle_distbearing";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	double	rlon1, rlat1, rlon2, rlat2, rbearing;
	double	t1, t2, t3, dd;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       lon1:             %f\n",lon1);
		fprintf(stderr,"dbg2       lat1:             %f\n",lat1);
		fprintf(stderr,"dbg2       lon2:             %f\n",lon2);
		fprintf(stderr,"dbg2       lat2:             %f\n",lat2);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* note: these equations derive in part from source code read at:
		http://simgear.org/doxygen/polar3d_8hxx-source.html
		on 17 February 2004 by D.W. Caress
		The source code found at this location is licensed under the LGPL */

	/* get great circle distance */
	rlon1 = DTR * lon1;
	rlat1 = DTR * lat1;
	rlon2 = DTR * lon2;
	rlat2 = DTR * lat2;
	t1 = sin(0.5 * (rlon1 - rlon2));
	t2 = sin(0.5 * (rlat1 - rlat2));
	dd = 2.0 * asin(sqrt(t2 * t2 + cos(rlat1) * cos(rlat2) * t1 * t1));
	*distance = MBV_SPHEROID_RADIUS * dd;
	
	/* get great circle bearing */
	
	/* first check if at poles */
	if (fabs(1.0 - sin(rlat1)) < 0.000001)
		{
		/* at north pole therefore heading south */
		if (lat1 > 0.0)
			{
			*bearing = 180.0;
			}
			
		/* at south pole therefore heading north */
		else
			{
			*bearing = 0.0;
			}
		}
		
	/* handle position away from poles */
	else
		{
		t3 = (sin(rlat2) - sin(rlat1) * cos(dd)) 
				/ (sin(dd) * cos(rlat1));
		rbearing = acos( MAX(MIN(t3, 1.0), -1.0));
		if (t1 <= 0.0)
			{
			*bearing = RTD * rbearing;
			}
		else
			{
			*bearing = 360.0 - RTD * rbearing;
			}
		if (*bearing < 0.0)
			*bearing += 360.0;
		}

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       t3:          %f\n",t3);
		fprintf(stderr,"dbg2       bearing:     %f\n",*bearing);
		fprintf(stderr,"dbg2       distance:    %f\n",*distance);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*------------------------------------------------------------------------------*/
int mbview_greatcircle_endposition(int instance, 
			double lon1, double lat1, double bearing, double distance,
			double *lon2, double *lat2)
{
	/* local variables */
	char	*function_name = "mbview_greatcircle_endposition";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	double	rd, rbearing, rlon1, rlat1, rlon2, rlat2;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       lon1:             %f\n",lon1);
		fprintf(stderr,"dbg2       lat1:             %f\n",lat1);
		fprintf(stderr,"dbg2       bearing:          %f\n",bearing);
		fprintf(stderr,"dbg2       distance:         %f\n",distance);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* note: these equations derive in part from source code read at:
		http://simgear.org/doxygen/polar3d_8hxx-source.html
		on 17 February 2004 by D.W. Caress
		The source code found at this location is licensed under the LGPL */
		
	/* scale angles to radians */
	rd = distance / MBV_SPHEROID_RADIUS;
	rbearing = DTR * (360.0 - bearing);
	rlon1 = DTR * lon1;
	rlat1 = DTR * lat1;

	/* calculate latitude */
	rlat2 = asin(sin(rlat1) * cos(rd) + cos(rlat1) * sin(rd) * cos(rbearing));
	*lat2 = RTD * rlat2;

	/* calculate longitude */
	if (cos(rlat2) < 0.000001)
		{
		*lon2 = lon1;
		}
	else
		{
		*lon2 = RTD * (fmod(rlon1 - asin( sin(rbearing) * sin(rd) / 
                                   cos(rlat2) ) + M_PI, 2.0 * M_PI) - M_PI);
		}

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       lon2:             %f\n",*lon2);
		fprintf(stderr,"dbg2       lat2:             %f\n",*lat2);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}
/*------------------------------------------------------------------------------*/
int mbview_colorclear(int instance)
{
	/* local variables */
	char	*function_name = "mbview_colorclear";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		}
		
if (mbv_verbose >= 2)
fprintf(stderr,"mbview_colorclear: %d\n", instance);
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* set status bit arrays */
	view->colordonecount = 0;
	if (data->primary_stat_color != NULL)
		memset(data->primary_stat_color, 0, (data->primary_nxy / 8) + 1);

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_zscaleclear(int instance)
{
	/* local variables */
	char	*function_name = "mbview_zscaleclear";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		}
		
if (mbv_verbose >= 2)
fprintf(stderr,"mbview_zscaleclear: %d\n", instance);
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* set status bit arrays */
	view->zscaledonecount = 0;
	if (data->primary_stat_z != NULL)
		memset(data->primary_stat_z, 0, (data->primary_nxy / 8) + 1);

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:      %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_setcolorparms(int instance)
{
	/* local variables */
	char	*function_name = "mbview_setcolorparms";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* get min max values for coloring */
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY)
		{
		view->colortable = data->primary_colortable;
		view->colortable_mode = data->primary_colortable_mode;
		view->min = data->primary_colortable_min;
		view->max = data->primary_colortable_max;
		}
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE)
		{
		view->colortable = data->slope_colortable;
		view->colortable_mode = data->slope_colortable_mode;
		view->min = data->slope_colortable_min;
		view->max = data->slope_colortable_max;
		}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY)
		{
		view->colortable = data->secondary_colortable;
		view->colortable_mode = data->secondary_colortable_mode;
		view->min = data->secondary_colortable_min;
		view->max = data->secondary_colortable_max;
		}
		
	/* get illumination vector if necessary */
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY)
	    view->shade_mode = data->primary_shade_mode;
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE)
	    view->shade_mode = data->slope_shade_mode;
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY)
	    view->shade_mode = data->secondary_shade_mode;
	view->illum_x = 0.0;
	view->illum_y = 0.0;
	view->illum_z = 0.0;
	view->mag2 = 0.0;
	if (view->shade_mode == MBV_SHADE_VIEW_ILLUMINATION)
		{
        	view->illum_x = sin(DTR * data->illuminate_azimuth) 
				* cos(DTR * data->illuminate_elevation);
       	 	view->illum_y = cos(DTR * data->illuminate_azimuth) 
				* cos(DTR * data->illuminate_elevation);
        	view->illum_z = sin(DTR * data->illuminate_elevation);
		view->mag2 = data->illuminate_magnitude
			* data->illuminate_magnitude;
/*fprintf(stderr,"ILLUMRAW: %f %f %f\n",
data->illuminate_azimuth, data->illuminate_elevation, data->illuminate_magnitude);
fprintf(stderr,"ILLUMLGT: %f %f %f %f\n",view->illum_x, view->illum_y, view->illum_z, view->mag2);*/
		}
		
	/* get sign of overlay shading if necessary */
	view->sign = 1.0;
	if (view->shade_mode == MBV_SHADE_VIEW_OVERLAY)
		{
		if (data->overlay_shade_mode == MBV_COLORTABLE_NORMAL)
			view->sign = 1.0;
		else
			view->sign = -1.0;
		}
	
	/* get colortable */
	if (view->colortable == MBV_COLORTABLE_HAXBY)
		{
		view->colortable_red = colortable_haxby_red;
		view->colortable_blue = colortable_haxby_blue;
		view->colortable_green = colortable_haxby_green;
		}
	else if (view->colortable == MBV_COLORTABLE_BRIGHT)
		{
		view->colortable_red = colortable_bright_red;
		view->colortable_blue = colortable_bright_blue;
		view->colortable_green = colortable_bright_green;
		}
	else if (view->colortable == MBV_COLORTABLE_MUTED)
		{
		view->colortable_red = colortable_muted_red;
		view->colortable_blue = colortable_muted_blue;
		view->colortable_green = colortable_muted_green;
		}
	else if (view->colortable == MBV_COLORTABLE_GRAY)
		{
		view->colortable_red = colortable_gray_red;
		view->colortable_blue = colortable_gray_blue;
		view->colortable_green = colortable_gray_green;
		}
	else if (view->colortable == MBV_COLORTABLE_FLAT)
		{
		view->colortable_red = colortable_flat_red;
		view->colortable_blue = colortable_flat_blue;
		view->colortable_green = colortable_flat_green;
		}
	else if (view->colortable == MBV_COLORTABLE_SEALEVEL)
		{
		view->colortable_red = colortable_haxby_red;
		view->colortable_blue = colortable_haxby_blue;
		view->colortable_green = colortable_haxby_green;
		}

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return(status);
}
		
/*------------------------------------------------------------------------------*/
int mbview_colorpoint(
	struct mbview_world_struct *view,
	struct mbview_struct *data,
	int i, int j, int k)
{
	/* local variables */
	char	*function_name = "mbview_colorpoint";
	int	status = MB_SUCCESS;
	double	value, svalue, factor, dd;
	double	xlon, ylat;
	double	intensity;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       view:             %d\n",view);
		fprintf(stderr,"dbg2       data:             %d\n",data);
		fprintf(stderr,"dbg2       i:                %d\n",i);
		fprintf(stderr,"dbg2       j:                %d\n",j);
		fprintf(stderr,"dbg2       k:                %d\n",k);
		}
		
if (mbv_verbose >= 2)
fprintf(stderr,"mbview_colorpoint: %d %d %d\n", i, j, k);
		
	/* get values for coloring */
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY)
		{
		value = data->primary_data[k];
		}
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE)
		{
		value = sqrt(data->primary_dzdx[k] 
					* data->primary_dzdx[k]
				+ data->primary_dzdy[k] 
					* data->primary_dzdy[k]);
		}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY)
		{
		if (data->secondary_sameas_primary == MB_YES)
			value = data->secondary_data[k];
		else
			mbview_getsecondaryvalue(view, data, i, j, &value);
		}

	/* get color */
	if (view->colortable != MBV_COLORTABLE_SEALEVEL)
	    mbview_getcolor(value, view->min, view->max, view->colortable_mode, 
			view->colortable_red,
			view->colortable_green,
			view->colortable_blue,
			&data->primary_r[k],
			&data->primary_g[k],
			&data->primary_b[k]);

	else
	    {
	    if (value > 0.0)
		{
		if (view->colortable_mode == MBV_COLORTABLE_NORMAL)
		    {
		    mbview_getcolor(value, 0.0, view->max, view->colortable_mode, 
			colortable_abovesealevel_red,
			colortable_abovesealevel_green,
			colortable_abovesealevel_blue,
			&data->primary_r[k],
			&data->primary_g[k],
			&data->primary_b[k]);
		    }
		else
		    {
		    mbview_getcolor(value, -view->max / 11.0, view->max, view->colortable_mode, 
			colortable_haxby_red,
			colortable_haxby_green,
			colortable_haxby_blue,
			&data->primary_r[k],
			&data->primary_g[k],
			&data->primary_b[k]);
		    }
		}
	    else
		{
		if (view->colortable_mode == MBV_COLORTABLE_REVERSED)
		    {
		    mbview_getcolor(value, view->min, 0.0, view->colortable_mode, 
			colortable_abovesealevel_red,
			colortable_abovesealevel_green,
			colortable_abovesealevel_blue,
			&data->primary_r[k],
			&data->primary_g[k],
			&data->primary_b[k]);
		    }
		else
		    {
		    view->colortable_red = colortable_haxby_red;
		    view->colortable_green = colortable_haxby_green;
		    view->colortable_blue = colortable_haxby_blue;
		    mbview_getcolor(value, view->min, -view->min / 11.0, view->colortable_mode, 
			colortable_haxby_red,
			colortable_haxby_green,
			colortable_haxby_blue,
			&data->primary_r[k],
			&data->primary_g[k],
			&data->primary_b[k]);
		    }
		}
	    }

	/* get values for shading */
	if (view->shade_mode != MBV_SHADE_VIEW_NONE)
	    {
	    if (view->shade_mode == MBV_SHADE_VIEW_ILLUMINATION)
		{
		dd = sqrt(view->mag2 * data->primary_dzdx[k] 
				* data->primary_dzdx[k]
			+ view->mag2 * data->primary_dzdy[k] 
				* data->primary_dzdy[k]
				+ 1.0);
		intensity = data->illuminate_magnitude 
			    * view->illum_x * data->primary_dzdx[k] / dd
			+ data->illuminate_magnitude 
			    * view->illum_y * data->primary_dzdy[k] / dd
			+ view->illum_z / dd
			- 0.5;
/*if (j==25)
fprintf(stderr,"intensity:%f  dzdx:%f  dzdy:%f\n",
intensity,data->primary_dzdx[k], data->primary_dzdy[k]);
*/

		mbview_applyshade(intensity,
			&data->primary_r[k],
			&data->primary_g[k],
			&data->primary_b[k]);
		}
	    else if (view->shade_mode == MBV_SHADE_VIEW_SLOPE)
		{
		intensity = -data->slope_magnitude
			* sqrt(data->primary_dzdx[k] 
					* data->primary_dzdx[k]
				+ data->primary_dzdy[k] 
					* data->primary_dzdy[k]);
		intensity = MAX(intensity, -1.0);
		mbview_applyshade(intensity,
			&data->primary_r[k],
			&data->primary_g[k],
			&data->primary_b[k]);
		}
	    else if (view->shade_mode == MBV_SHADE_VIEW_OVERLAY)
		{
		if (data->secondary_sameas_primary == MB_YES)
			svalue = data->secondary_data[k];
		else
			mbview_getsecondaryvalue(view, data, i, j, &svalue);
		if (svalue != data->secondary_nodatavalue)
			{
			intensity = view->sign * data->overlay_shade_magnitude 
				* (svalue - data->overlay_shade_center)
				/ (data->secondary_max - data->secondary_min);
			mbview_applyshade(intensity,
				&data->primary_r[k],
				&data->primary_g[k],
				&data->primary_b[k]);
			}
		}
	    }

	/* set color status bit */
	data->primary_stat_color[k/8] 
		= data->primary_stat_color[k/8] | statmask[k%8];

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return(status);
}
/*------------------------------------------------------------------------------*/
int mbview_colordata(int instance, int rez)
{
	/* local variables */
	char	*function_name = "mbview_colordata";
	int	status = MB_SUCCESS;
	int	stride;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	i, j, k, ii;
	double	value, svalue, factor, dd;
	double	xlon, ylat;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       rez:              %d\n",rez);
		}
		
if (mbv_verbose >= 2)
fprintf(stderr,"mbview_colordata: %d %d\n", instance, rez);
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* set stride for looping over data */
	if (rez == MBV_REZ_FULL)
	    stride = 1;
	else if (rez == MBV_REZ_HIGH)
	    stride = MAX((int)ceil(((double)data->primary_nx) 
				/ ((double)data->hirez_dimension)), 
			(int)ceil(((double)data->primary_ny) 
				/ ((double)data->hirez_dimension)));
	else
	    stride = MAX((int)ceil(((double)data->primary_nx) 
				/ ((double)data->lorez_dimension)), 
			(int)ceil(((double)data->primary_ny) 
				/ ((double)data->lorez_dimension)));
		
	/* color the data */
	for (i=0;i<data->primary_nx;i+=stride)
		{
		for (j=0;j<data->primary_ny;j+=stride)
			{
			k = i * data->primary_ny + j;

			if (data->primary_data[k] != data->primary_nodatavalue
				&& !(data->primary_stat_color[k/8] & statmask[k%8]))
				{
				mbview_colorpoint(view, data, i, j, k);
				}
			}

		/* check for pending event */
		if (view->plot_done == MB_NO 
			&& view->plot_interrupt_allowed == MB_YES 
			&& i % MBV_EVENTCHECKCOARSENESS == 0)
			do_mbview_xevents();

		/* dump out of loop if plotting already done at a higher recursion */
		if (view->plot_done == MB_YES)
			i = data->primary_nx;
		}

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_getsecondaryvalue(struct mbview_world_struct *view,
				struct mbview_struct *data,
				int i, int j, 
				double *secondary_value)
{
	/* local variables */
	char	*function_name = "mbview_getsecondaryvalue";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	double	xlon, ylat;
	double	xgrid, ygrid;
	double	xsgrid, ysgrid;
	int	ii, jj, kk;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       view:             %d\n",view);
		fprintf(stderr,"dbg2       data:             %d\n",data);
		fprintf(stderr,"dbg2       i:                %d\n",i);
		fprintf(stderr,"dbg2       j:                %d\n",j);
		}
	
	/* get position in primary grid */
	xgrid = data->primary_xmin + i * data->primary_dx;
	ygrid = data->primary_ymin + j * data->primary_dy;
		
	/* get lon and lat of desired position */
	if (data->primary_grid_projection_mode == MBV_PROJECTION_PROJECTED
		|| data->primary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED)
		{
		mb_proj_inverse(mbv_verbose, view->primary_pjptr,
				xgrid, ygrid, &xlon, &ylat, &error);
		}
	else
		{
		xlon = xgrid;
		ylat = ygrid;
		}

	/* get position in secondary grid coordinates */
	if (data->secondary_grid_projection_mode == MBV_PROJECTION_PROJECTED
		|| data->secondary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED)
		{
		mb_proj_forward(mbv_verbose, view->secondary_pjptr,
				xlon, ylat, &xsgrid, &ysgrid, &error);
		}
	else
		{
		xsgrid = xlon;
		ysgrid = ylat;
		}

	/* get rounded location in secondary grid */
	ii = (xsgrid - data->secondary_xmin) / data->secondary_dx;
	jj = (ysgrid - data->secondary_ymin) / data->secondary_dy;
	
	/* answer only defined within grid bounds */
	if (ii < 0 || ii >= data->secondary_nx
		|| jj < 0 || jj > data->secondary_ny)
		{
		*secondary_value = data->secondary_nodatavalue;
		}
	else
		{
		kk = ii * data->secondary_ny + jj;
		*secondary_value = data->secondary_data[kk];
		}

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       secondary_value:  %f\n",*secondary_value);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_getcolor(double value, double min, double max,
			int colortablemode, 
			float *colortable_red,
			float *colortable_green,
			float *colortable_blue,
			float *red, float *green, float *blue)
{
	/* local variables */
	char	*function_name = "mbview_getcolor";
	int	status = MB_SUCCESS;
	int	ii;
	double	ff, factor;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       value:            %f\n",value);
		fprintf(stderr,"dbg2       min:              %f\n",min);
		fprintf(stderr,"dbg2       max:              %f\n",max);
		fprintf(stderr,"dbg2       colortablemode:   %d\n",colortablemode);
		fprintf(stderr,"dbg2       colortable_red:   %d\n",colortable_red);
		fprintf(stderr,"dbg2       colortable_green: %d\n",colortable_green);
		fprintf(stderr,"dbg2       colortable_blue:  %d\n",colortable_blue);
		}
		
	/* get color */
	if (colortablemode == MBV_COLORTABLE_NORMAL)
		factor = (max - value) / (max - min);
	else
		factor = (value -  min) / (max - min);
	factor = MAX(factor, 0.000001);
	factor = MIN(factor, 0.999999);
	ii = (int) (factor * (MBV_NUM_COLORS - 1));
	ff = factor * (MBV_NUM_COLORS - 1) - ii;
	*red = colortable_red[ii]
		+ ff * (colortable_red[ii+1] - colortable_red[ii]);
	*green = colortable_green[ii]
		+ ff * (colortable_green[ii+1] - colortable_green[ii]);
	*blue = colortable_blue[ii]
		+ ff * (colortable_blue[ii+1] - colortable_blue[ii]);

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       red:     %f\n",*red);
		fprintf(stderr,"dbg2       green:   %f\n",*green);
		fprintf(stderr,"dbg2       blue:    %f\n",*blue);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_applyshade(double intensity, float *r, float *g, float *b)
{
	/* note - this correction algorithm is taken from the GMT Technical
	   Reference and Cookbook by Wessel and Smith - you can find it in
	   Appendix I: Color Space - The final frontier */
	   
	/* local variables */
	char	*function_name = "mbview_applyshade";
	int	status = MB_SUCCESS;
	double	h, s, v;
	double	vmax, vmin, dv, idv;
	double	rmod, gmod, bmod;
	double	f, p, q, t;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       intensity:           %f\n",intensity);
		}
	
	/* change the initial rgb into hsv space */
	vmax = MAX (MAX (*r, *g), *b);
	vmin = MIN (MIN (*r, *g), *b);
	dv = vmax - vmin;
	v = vmax;
	if (vmax == 0.0)
		s = 0.0;
	else
		s = dv / vmax;
	h = 0.0;
	if (s > 0.0)
		{
		idv = 1.0 / dv;
		rmod = (vmax - *r) * idv;
		gmod = (vmax - *g) * idv;
		bmod = (vmax - *b) * idv;
		if (*r == vmax)
			h = bmod - gmod;
		else if (*g == vmax)
			h = 2.0 + rmod - bmod;
		else
			h = 4.0 + gmod - rmod;
		h *= 60.0;
		if (h < 0.0) 
			h += 360.0;
		}

	/* apply the shade to the color */
	if (intensity > 0) 
		{
		if (s != 0.0) 
			s = (1.0 - intensity) * s 
				+ intensity * 0.1;
		v = (1.0 - intensity) * v 
			+ intensity;
		}
	else 
		{
		if (s != 0.0) 
			s = (1.0 + intensity) * s 
				- intensity;
		v = (1.0 + intensity) * v 
			- intensity * 0.3;
		}
	if (v < 0.0) 
		v = 0.0;
	if (s < 0.0) 
		s = 0.0;
	if (v > 1.0) 
		v = 1.0;
	if (s > 1.0) 
		s = 1.0;
	
	/* change the corrected hsv values back into rgb */
	if (s == 0.0)
		{
		*r = v;
		*g = v;
		*b = v;
		}
	else
		{
		while (h >= 360.0) 
			h -= 360.0;
		h /= 60.0;
		f = h - ((int)h);
		p = v * (1.0 - s);
		q = v * (1.0 - (s * f));
		t = v * (1.0 - (s * (1.0 - f)));
		switch (((int)h)) 
			{
			case 0:
				*r = v;	
				*g = t;	
				*b = p;
				break;
			case 1:
				*r = q;	
				*g = v;	
				*b = p;
				break;
			case 2:
				*r = p;	
				*g = v;	
				*b = t;
				break;
			case 3:
				*r = p;	
				*g = q;	
				*b = v;
				break;
			case 4:
				*r = t;	
				*g = p;	
				*b = v;
				break;
			case 5:
				*r = v;	
				*g = p;	
				*b = q;
				break;
			}
		}

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       red:     %d\n",*r);
		fprintf(stderr,"dbg2       green:   %d\n",*g);
		fprintf(stderr,"dbg2       blue:    %d\n",*b);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return(status);
}
/*------------------------------------------------------------------------------*/
int mbview_contour(int instance, int rez)
{
	/* local variables */
	char	*function_name = "mbview_contour";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	i, j, k, l, kk;
	int	stride;
	int	vertex[4];
	int	triangleA, triangleB;
	int	nlevel, level_min, level_max;
	int	nvertex, nside;
	float	level_value, datamin, datamax;
	float	factor;
	float	xx[2], yy[2], zz[2];
	int	global;
	double	contour_offset_factor;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       rez:              %d\n",rez);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* set stride for looping over data */
	if (rez == MBV_REZ_FULL)
	    	stride = 1;
	else if (rez == MBV_REZ_HIGH)
	    	stride = MAX((int)ceil(((double)data->primary_nx) 
				/ ((double)data->hirez_dimension)), 
			(int)ceil(((double)data->primary_ny) 
				/ ((double)data->hirez_dimension)));
	else
	    	stride = MAX((int)ceil(((double)data->primary_nx) 
				/ ((double)data->lorez_dimension)), 
			(int)ceil(((double)data->primary_ny) 
				/ ((double)data->lorez_dimension)));

	/* start openGL list */
	if (rez == MBV_REZ_FULL)
		{
	    	glNewList((GLuint)(3*instance+3), GL_COMPILE);
		}
	else if (rez == MBV_REZ_HIGH)
		{
	    	glNewList((GLuint)(3*instance+2), GL_COMPILE);
		}
	else
		{
	    	glNewList((GLuint)(3*instance+1), GL_COMPILE);
		}
	glColor3f(0.0, 0.0, 0.0);
	glLineWidth(1.0);
	glBegin(GL_LINES);
	
if (mbv_verbose >= 2)
fprintf(stderr,"mbview_contour: instance:%d rez:%d stride:%d contour interval:%f\n",
instance, rez, stride, data->contour_interval);

	/* check if the contour offset needs to be applied in a global spherical direction or just up */
	if (data->display_projection_mode == MBV_PROJECTION_SPHEROID
		&& view->sphere_refx == 0.0
		&& view->sphere_refy == 0.0
		&& view->sphere_refz == 0.0)
		{
		global = MB_YES;
		contour_offset_factor = MBV_OPENGL_3D_CONTOUR_OFFSET / (view->scale * MBV_SPHEROID_RADIUS);
		}
	else
		{
		global = MB_NO;
		contour_offset_factor = MBV_OPENGL_3D_CONTOUR_OFFSET;
		}

	/* construct the contour segments in each triangle */
	for (i=0;i<data->primary_nx-stride;i+=stride)
	{
	for (j=0;j<data->primary_ny-stride;j+=stride)
		{
		/* get vertex id's */
		vertex[0] = i * data->primary_ny + j;
		vertex[1] = (i + stride) * data->primary_ny + j;
	        vertex[2] = i * data->primary_ny + j + stride;
		vertex[3] = (i + stride) * data->primary_ny + j + stride;
		
		/* check if either triangle can be contoured */
		triangleA = MB_NO;
		triangleB = MB_NO;
		if (data->primary_data[vertex[0]] != data->primary_nodatavalue
			&& data->primary_data[vertex[1]] != data->primary_nodatavalue
			&& data->primary_data[vertex[2]] != data->primary_nodatavalue)
			triangleA = MB_YES;
		if (data->primary_data[vertex[1]] != data->primary_nodatavalue
			&& data->primary_data[vertex[3]] != data->primary_nodatavalue
			&& data->primary_data[vertex[2]] != data->primary_nodatavalue)
			triangleB = MB_YES;
			
		/* if at least one triangle is valid, contour it */
		if (triangleA == MB_YES || triangleB == MB_YES)
			{
			/* get min max values and number of contours */
			nvertex = 0;
			datamin = 0.0;
			datamax = 0.0;
			for (kk=0;kk<4;kk++)
				{
				k = vertex[kk];
				if (data->primary_data[k] != data->primary_nodatavalue) 
					{
					if (!(data->primary_stat_z[k/8] & statmask[k%8]))
						mbview_zscalegridpoint(instance,k);
					if (nvertex == 0)
						{
						datamin = data->primary_data[k];
						datamax = data->primary_data[k];
						}
					else
						{
						datamin = MIN(datamin, data->primary_data[k]);
						datamax = MAX(datamax, data->primary_data[k]);
						}
					nvertex++;
					}
				}

			/* get start, end, and number of contour levels in contour_interval units */
			level_min = (int)ceil(datamin / data->contour_interval);
			level_max = (int)floor(datamax / data->contour_interval);
			nlevel = level_max - level_min + 1;
				
			/* now if contours are needed loop over the contour levels */
			if (nlevel > 0)
				{
				for (l=level_min;l<=level_max;l++)
					{
					level_value = l * data->contour_interval;
					
					/* deal with triangle A - vertexes 0, 1, and 2 */
					if (triangleA == MB_YES)
					{
					nside = 0;
					if ((data->primary_data[vertex[0]] > level_value
							&& data->primary_data[vertex[1]] < level_value)
						|| (data->primary_data[vertex[0]] < level_value
							&& data->primary_data[vertex[1]] > level_value))
						{
						factor = (level_value - data->primary_data[vertex[0]])
										/ (data->primary_data[vertex[1]]
											- data->primary_data[vertex[0]]);
						xx[nside] = data->primary_x[vertex[0]]
									+ factor * (data->primary_x[vertex[1]]
											- data->primary_x[vertex[0]]);
						yy[nside] = data->primary_y[vertex[0]]
									+ factor * (data->primary_y[vertex[1]]
											- data->primary_y[vertex[0]]);
						zz[nside] = data->primary_z[vertex[0]]
									+ factor * (data->primary_z[vertex[1]]
											- data->primary_z[vertex[0]]);
						nside++;
						}
					if ((data->primary_data[vertex[1]] > level_value
							&& data->primary_data[vertex[2]] < level_value)
						|| (data->primary_data[vertex[1]] < level_value
							&& data->primary_data[vertex[2]] > level_value))
						{
						factor = (level_value - data->primary_data[vertex[1]])
										/ (data->primary_data[vertex[2]]
											- data->primary_data[vertex[1]]);
						xx[nside] = data->primary_x[vertex[1]]
									+ factor * (data->primary_x[vertex[2]]
											- data->primary_x[vertex[1]]);
						yy[nside] = data->primary_y[vertex[1]]
									+ factor * (data->primary_y[vertex[2]]
											- data->primary_y[vertex[1]]);
						zz[nside] = data->primary_z[vertex[1]]
									+ factor * (data->primary_z[vertex[2]]
											- data->primary_z[vertex[1]]);
						nside++;
						}
					if (nside < 2 &&
						((data->primary_data[vertex[2]] > level_value
							&& data->primary_data[vertex[0]] < level_value)
						|| (data->primary_data[vertex[2]] < level_value
							&& data->primary_data[vertex[0]] > level_value)))
						{
						factor = (level_value - data->primary_data[vertex[2]])
										/ (data->primary_data[vertex[0]]
											- data->primary_data[vertex[2]]);
						xx[nside] = data->primary_x[vertex[2]]
									+ factor * (data->primary_x[vertex[0]]
											- data->primary_x[vertex[2]]);
						yy[nside] = data->primary_y[vertex[2]]
									+ factor * (data->primary_y[vertex[0]]
											- data->primary_y[vertex[2]]);
						zz[nside] = data->primary_z[vertex[2]]
									+ factor * (data->primary_z[vertex[0]]
											- data->primary_z[vertex[2]]);
						nside++;
						}
					if (nside == 2)
						{
						if (data->display_projection_mode != MBV_PROJECTION_SPHEROID)
							{
							zz[0] += contour_offset_factor;
							zz[1] += contour_offset_factor;
							}
						else if (global == MB_YES)
							{
							xx[0] += xx[0] * contour_offset_factor;
							yy[0] += yy[0] * contour_offset_factor;
							zz[0] += zz[0] * contour_offset_factor;
							xx[1] += xx[1] * contour_offset_factor;
							yy[1] += yy[1] * contour_offset_factor;
							zz[1] += zz[1] * contour_offset_factor;
							}
						else
							{
							zz[0] += contour_offset_factor;
							zz[1] += contour_offset_factor;
							}
						glVertex3f(xx[0], yy[0], zz[0]);
						glVertex3f(xx[1], yy[1], zz[1]);
						}
					}
					
					/* deal with triangle B - vertexes 1, 3, and 2 */
					if (triangleB == MB_YES)
					{
					nside = 0;
					if ((data->primary_data[vertex[1]] > level_value
							&& data->primary_data[vertex[3]] < level_value)
						|| (data->primary_data[vertex[1]] < level_value
							&& data->primary_data[vertex[3]] > level_value))
						{
						factor = (level_value - data->primary_data[vertex[1]])
										/ (data->primary_data[vertex[3]]
											- data->primary_data[vertex[1]]);
						xx[nside] = data->primary_x[vertex[1]]
									+ factor * (data->primary_x[vertex[3]]
											- data->primary_x[vertex[1]]);
						yy[nside] = data->primary_y[vertex[1]]
									+ factor * (data->primary_y[vertex[3]]
											- data->primary_y[vertex[1]]);
						zz[nside] = data->primary_z[vertex[1]]
									+ factor * (data->primary_z[vertex[3]]
											- data->primary_z[vertex[1]]);
						nside++;
						}
					if ((data->primary_data[vertex[3]] > level_value
							&& data->primary_data[vertex[2]] < level_value)
						|| (data->primary_data[vertex[3]] < level_value
							&& data->primary_data[vertex[2]] > level_value))
						{
						factor = (level_value - data->primary_data[vertex[3]])
										/ (data->primary_data[vertex[2]]
											- data->primary_data[vertex[3]]);
						xx[nside] = data->primary_x[vertex[3]]
									+ factor * (data->primary_x[vertex[2]]
											- data->primary_x[vertex[3]]);
						yy[nside] = data->primary_y[vertex[3]]
									+ factor * (data->primary_y[vertex[2]]
											- data->primary_y[vertex[3]]);
						zz[nside] = data->primary_z[vertex[3]]
									+ factor * (data->primary_z[vertex[2]]
											- data->primary_z[vertex[3]]);
						nside++;
						}
					if (nside < 2 &&
						((data->primary_data[vertex[2]] > level_value
							&& data->primary_data[vertex[1]] < level_value)
						|| (data->primary_data[vertex[2]] < level_value
							&& data->primary_data[vertex[1]] > level_value)))
						{
						factor = (level_value - data->primary_data[vertex[2]])
										/ (data->primary_data[vertex[1]]
											- data->primary_data[vertex[2]]);
						xx[nside] = data->primary_x[vertex[2]]
									+ factor * (data->primary_x[vertex[1]]
											- data->primary_x[vertex[2]]);
						yy[nside] = data->primary_y[vertex[2]]
									+ factor * (data->primary_y[vertex[1]]
											- data->primary_y[vertex[2]]);
						zz[nside] = data->primary_z[vertex[2]]
									+ factor * (data->primary_z[vertex[1]]
											- data->primary_z[vertex[2]]);
						nside++;
						}
					if (nside == 2)
						{
						if (data->display_projection_mode != MBV_PROJECTION_SPHEROID)
							{
							zz[0] += contour_offset_factor;
							zz[1] += contour_offset_factor;
							}
						else if (global == MB_YES)
							{
							xx[0] += xx[0] * contour_offset_factor;
							yy[0] += yy[0] * contour_offset_factor;
							zz[0] += zz[0] * contour_offset_factor;
							xx[1] += xx[1] * contour_offset_factor;
							yy[1] += yy[1] * contour_offset_factor;
							zz[1] += zz[1] * contour_offset_factor;
							}
						else
							{
							zz[0] += contour_offset_factor;
							zz[1] += contour_offset_factor;
							}
						glVertex3f(xx[0], yy[0], zz[0]);
						glVertex3f(xx[1], yy[1], zz[1]);
						}
					}
					
					}
				}
			}
		
		}
		
	/* check for pending event */
	if (view->plot_done == MB_NO 
		&& view->plot_interrupt_allowed == MB_YES 
		&& i % MBV_EVENTCHECKCOARSENESS == 0)
		do_mbview_xevents();
		
	/* dump out of loop if plotting already done at a higher recursion */
	if (view->plot_done == MB_YES)
		i = data->primary_nx;
	}
	
	/* end openGL list */
	glEnd();
	glEndList();

	/* set rez flag only if plotting not done */
	if (view->plot_done == MB_NO)
		{
		if (rez == MBV_REZ_FULL)
			{
			view->contourfullrez = MB_YES;
			}
		else if (rez == MBV_REZ_HIGH)
			{
			view->contourhirez = MB_YES;
			}
		else
			{
			view->contourlorez = MB_YES;
			}
		}

	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return */
	return(status);
}
/*------------------------------------------------------------------------------*/
int mbview_getzdata(int instance, 
			double xgrid, double ygrid,
			int *found, double *zdata)
{

	/* local variables */
	char	*function_name = "mbview_getzdata";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	nsum;
	double	zdatasum;
	int	i,j, k, l, m, n;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       xgrid:            %f\n",xgrid);
		fprintf(stderr,"dbg2       ygrid:            %f\n",ygrid);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* get location in grid */
	i = (int)((xgrid - data->primary_xmin) / data->primary_dx);
	j = (int)((ygrid - data->primary_ymin) / data->primary_dy);
	
	/* fail if outside grid */
	if (i < 0 || i >= data->primary_nx - 1
		|| j < 0 || j >= data->primary_ny - 1)
		{
		*found = MB_NO;
		*zdata = 0.0;
		}
		
	/* check all four points and average the good ones */
	else
		{
		k = i * data->primary_ny + j;
		l = (i + 1) * data->primary_ny + j;
		m = i * data->primary_ny + j + 1;
		n = (i + 1) * data->primary_ny + j + 1;
		nsum = 0;
		zdatasum = 0.0;
		if (data->primary_data[k] != data->primary_nodatavalue)
			{
			zdatasum += data->primary_data[k];
			nsum++;
			}
		if (data->primary_data[l] != data->primary_nodatavalue)
			{
			zdatasum += data->primary_data[l];
			nsum++;
			}
		if (data->primary_data[m] != data->primary_nodatavalue)
			{
			zdatasum += data->primary_data[m];
			nsum++;
			}
		if (data->primary_data[n] != data->primary_nodatavalue)
			{
			zdatasum += data->primary_data[n];
			nsum++;
			}
		if (nsum > 0)
			{
			*zdata = zdatasum / nsum;
			*found = MB_YES;
			}
		else
			{
			*zdata = 0.0;
			*found = MB_NO;
			}
		}
	
	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       found:           %d\n",*found);
		fprintf(stderr,"dbg2       zdata:           %f\n",*zdata);
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return */
	return(status);
}


/*------------------------------------------------------------------------------*/
