/*--------------------------------------------------------------------
 *    The MB-system:	mb_truecont.c	4/21/94
 *    $Id: mb_truecont.c,v 5.4 2005-11-04 22:49:51 caress Exp $
 *
 *    Copyright (c) 1994, 2000 by
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
 * mb_truecontour.c contours a block of multibeam bathymetry data,
 * dealing correctly with beams in arbitrary locations by forming
 * a delauney triangle network and then contouring that network.
 *
 * Author:	D. W. Caress
 * Date:	April, 1994
 *
 */

/* standard global include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_contour.h"
#include "../../include/mb_define.h"

/* global defines */
#define IUP 3
#define IDN 2
#define IOR -3
#define EPS 0.0001

/*--------------------------------------------------------------------------*/
/* 	function mb_contour_init initializes the memory required to
	contour multibeam bathymetry data. 
	if mbio_ptr is null, the arrays are allocated using mb_malloc. If
	mbio_ptr is a valid mbio structure, then the arrays tied to
	beams_bath will be registered using mb_register_array */
int mb_contour_init(
		int	verbose, 
		struct swath **data,
		int	npings_max,
		int	beams_bath,
		int	contour_algorithm,
		int	plot_contours,
		int	plot_triangles,
		int	plot_track,
		int	plot_name,
		double	contour_int,
		double	color_int,
		double	tick_int,
		double	label_int,
		double	tick_len,
		double	label_hgt,
		double	label_spacing,
		int	ncolor,
		int	nlevel,
		double	*level_list,
		int	*label_list,
		int	*tick_list,
		double	time_tick_int,
		double	time_annot_int,
		double	date_annot_int,
		double	time_tick_len,
		double	name_hgt,
		int	*error)
{
  	static char rcs_id[]="$Id: mb_truecont.c,v 5.4 2005-11-04 22:49:51 caress Exp $";
	char	*function_name = "mb_contour_init";
	int	status = MB_SUCCESS;
	struct swath *dataptr;
	struct ping *ping;
	int	ntri_max;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		fprintf(stderr,"dbg2       data:             %d\n",data);
		fprintf(stderr,"dbg2       npings_max:       %d\n",npings_max);
		fprintf(stderr,"dbg2       beams_bath:       %d\n",beams_bath);
		fprintf(stderr,"dbg2       contour algorithm:%d\n",contour_algorithm);
		fprintf(stderr,"dbg2       plot contours:    %d\n",plot_contours);
		fprintf(stderr,"dbg2       plot triangles:   %d\n",plot_triangles);
		fprintf(stderr,"dbg2       plot track:       %d\n",plot_track);
		fprintf(stderr,"dbg2       plot name:        %d\n",plot_name);
		fprintf(stderr,"dbg2       contour interval: %f\n",contour_int);
		fprintf(stderr,"dbg2       color interval:   %f\n",color_int);
		fprintf(stderr,"dbg2       tick interval:    %f\n",tick_int);
		fprintf(stderr,"dbg2       label interval:   %f\n",label_int);
		fprintf(stderr,"dbg2       tick length:      %f\n",tick_len);
		fprintf(stderr,"dbg2       label height:     %f\n",label_hgt);
		fprintf(stderr,"dbg2       label spacing:    %f\n",label_spacing);
		fprintf(stderr,"dbg2       number of colors: %d\n",ncolor);
		fprintf(stderr,"dbg2       number of levels: %d\n",nlevel);
		for (i=0;i<nlevel;i++)
			fprintf(stderr,"dbg2       level %d: %f %d %d\n",
				i,level_list[i],label_list[i],tick_list[i]);
		fprintf(stderr,"dbg2       time tick int:    %f\n",time_tick_int);
		fprintf(stderr,"dbg2       time interval:    %f\n",time_annot_int);
		fprintf(stderr,"dbg2       date interval:    %f\n",date_annot_int);
		fprintf(stderr,"dbg2       time tick length: %f\n",time_tick_len);
		fprintf(stderr,"dbg2       name height:      %f\n\n",name_hgt);
		}

	/* allocate memory for swath structure */
	status = mb_malloc(verbose,sizeof(struct swath),data,error);

	/* set variables and allocate memory for bathymetry data */
	dataptr = *data;
	dataptr->npings = 0;
	dataptr->npings_max = npings_max;
	dataptr->beams_bath = beams_bath;
	status = mb_malloc(verbose,npings_max*sizeof(struct ping),
				&(dataptr->pings),error);
	for (i=0;i<npings_max;i++)
		{
		ping = &dataptr->pings[i];
		ping->beams_bath = 0;
		ping->beams_bath_alloc = beams_bath;
		ping->beamflag = NULL;
		ping->bath = NULL;
		ping->bathlon = NULL;
		ping->bathlat = NULL;
		status = mb_malloc(verbose,beams_bath*sizeof(char),
				&(ping->beamflag),error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
				&(ping->bath),error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
				&(ping->bathlon),error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
				&(ping->bathlat),error);
		if (contour_algorithm == MB_CONTOUR_TRIANGLES)
			{
			ping->bflag[0] = NULL;
			ping->bflag[1] = NULL;
			}
		else
			{
			ping->bflag[0] = NULL;
			ping->bflag[1] = NULL;
			status = mb_malloc(verbose,beams_bath*sizeof(int),
				&(ping->bflag[0]),error);
			status = mb_malloc(verbose,beams_bath*sizeof(int),
				&(ping->bflag[1]),error);
			}
		}

	/* set controls on what gets plotted */
	dataptr->contour_algorithm = contour_algorithm;
	dataptr->plot_contours = plot_contours;
	dataptr->plot_triangles = plot_triangles;
	dataptr->plot_track = plot_track;
	dataptr->plot_name = plot_name;

	/* set variables and allocate memory for contour controls */
	dataptr->contour_int = contour_int;
	dataptr->color_int = color_int;
	dataptr->tick_int = tick_int;
	dataptr->label_int = label_int;
	dataptr->tick_len = tick_len;
	dataptr->label_hgt = label_hgt;
	if (label_spacing > 0.0)
	    dataptr->label_spacing = label_spacing;
	else
	    dataptr->label_spacing = label_hgt;
	dataptr->ncolor = ncolor;
	dataptr->nlevel = nlevel;
	dataptr->nlevelset = MB_NO;
	dataptr->level_list = NULL;
	dataptr->label_list = NULL;
	dataptr->tick_list = NULL;
	dataptr->color_list = NULL;
	if (nlevel > 0)
		{
		dataptr->nlevelset = MB_YES;
		status = mb_malloc(verbose,nlevel*sizeof(double),
				&(dataptr->level_list),error);
		status = mb_malloc(verbose,nlevel*sizeof(int),
				&(dataptr->label_list),error);
		status = mb_malloc(verbose,nlevel*sizeof(int),
				&(dataptr->tick_list),error);
		status = mb_malloc(verbose,nlevel*sizeof(int),
				&(dataptr->color_list),error);
		for (i=0;i<nlevel;i++)
			{
			dataptr->level_list[i] = level_list[i];
			dataptr->label_list[i] = label_list[i];
			dataptr->tick_list[i] = tick_list[i];
			dataptr->color_list[i] = i;
			}
		}

	/* set variables and allocate memory for track controls */
	dataptr->time_tick_int = time_tick_int;
	dataptr->time_annot_int = time_annot_int;
	dataptr->date_annot_int = date_annot_int;
	dataptr->time_tick_len = time_tick_len;
	dataptr->name_hgt = name_hgt;

	/* set variables and allocate memory for triangle network */
	dataptr->npts = 0;
	dataptr->x = NULL;
	dataptr->y = NULL;
	dataptr->z = NULL;
	dataptr->edge = NULL;
	dataptr->pingid = NULL;
	dataptr->ntri = 0;
	for (i=0;i<3;i++)
		{
		dataptr->iv[i] = NULL;
		dataptr->ct[i] = NULL;
		dataptr->cs[i] = NULL;
		dataptr->ed[i] = NULL;
		dataptr->flag[i] = NULL;
		}
	dataptr->v1 = NULL;
	dataptr->v2 = NULL;
	dataptr->v3 = NULL;
	dataptr->istack = NULL;
	dataptr->kv1 = NULL;
	dataptr->kv2 = NULL;
	if (contour_algorithm == MB_CONTOUR_TRIANGLES)
	  {
	  dataptr->npts = 0;
	  dataptr->npts_alloc = npings_max*beams_bath+3;
	  status = mb_malloc(verbose,dataptr->npts_alloc*sizeof(double),
			&(dataptr->x),error);
	  status = mb_malloc(verbose,dataptr->npts_alloc*sizeof(double),
			&(dataptr->y),error);
	  status = mb_malloc(verbose,dataptr->npts_alloc*sizeof(double),
			&(dataptr->z),error);
	  status = mb_malloc(verbose,dataptr->npts_alloc*sizeof(int),
			&(dataptr->edge),error);
	  status = mb_malloc(verbose,dataptr->npts_alloc*sizeof(int),
			&(dataptr->pingid),error);
	  ntri_max = 3*npings_max*beams_bath + 1;
	  dataptr->ntri = 0;
	  dataptr->ntri_alloc = ntri_max;
	  for (i=0;i<3;i++)
		{
		status = mb_malloc(verbose,ntri_max*sizeof(int),
				&(dataptr->iv[i]),error);
		status = mb_malloc(verbose,ntri_max*sizeof(int),
				&(dataptr->ct[i]),error);
		status = mb_malloc(verbose,ntri_max*sizeof(int),
				&(dataptr->cs[i]),error);
		status = mb_malloc(verbose,ntri_max*sizeof(int),
				&(dataptr->ed[i]),error);
		status = mb_malloc(verbose,ntri_max*sizeof(int),
				&(dataptr->flag[i]),error);
		}
	  status = mb_malloc(verbose,ntri_max*sizeof(double),
			&(dataptr->v1),error);
	  status = mb_malloc(verbose,ntri_max*sizeof(double),
			&(dataptr->v2),error);
	  status = mb_malloc(verbose,ntri_max*sizeof(double),
			&(dataptr->v3),error);
	  status = mb_malloc(verbose,ntri_max*sizeof(int),
			&(dataptr->istack),error);
	  status = mb_malloc(verbose,3*ntri_max*sizeof(int),
			&(dataptr->kv1),error);
	  status = mb_malloc(verbose,3*ntri_max*sizeof(int),
			&(dataptr->kv2),error);
	  }

	/* allocate memory for contour positions */
	dataptr->nsave = 0;
	dataptr->xsave = NULL;
	dataptr->ysave = NULL;
	dataptr->isave = NULL;
	dataptr->jsave = NULL;
	if (contour_algorithm == MB_CONTOUR_TRIANGLES)
		{
		status = mb_malloc(verbose,(4*ntri_max+1)*sizeof(double),
				&(dataptr->xsave),error);
		status = mb_malloc(verbose,(4*ntri_max+1)*sizeof(double),
				&(dataptr->ysave),error);
		}
	else
		{
		dataptr->npts = 0;
		dataptr->npts_alloc = npings_max * beams_bath;
		status = mb_malloc(verbose,dataptr->npts_alloc*sizeof(double),
				&(dataptr->xsave),error);
		status = mb_malloc(verbose,dataptr->npts_alloc*sizeof(double),
				&(dataptr->ysave),error);
		status = mb_malloc(verbose,dataptr->npts_alloc*sizeof(int),
				&(dataptr->isave),error);
		status = mb_malloc(verbose,dataptr->npts_alloc*sizeof(int),
				&(dataptr->jsave),error);
		}

	/* allocate memory for contour labels */
	dataptr->nlabel = 0;
	dataptr->xlabel = NULL;
	dataptr->ylabel = NULL;
	dataptr->angle = NULL;
	dataptr->justify = NULL;
	status = mb_malloc(verbose,(5*npings_max)*sizeof(double),
				&(dataptr->xlabel),error);
	status = mb_malloc(verbose,(5*npings_max)*sizeof(double),
				&(dataptr->ylabel),error);
	status = mb_malloc(verbose,(5*npings_max)*sizeof(double),
				&(dataptr->angle),error);
	status = mb_malloc(verbose,(5*npings_max)*sizeof(int),
				&(dataptr->justify),error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	return(MB_SUCCESS);
}

/*--------------------------------------------------------------------------*/
/* 	function mb_contour_deall deallocates the memory required to
	contour multibeam bathymetry data. */
int mb_contour_deall(
		int	verbose, 
		struct swath *data, 
		int	*error)
{
  	static char rcs_id[]="$Id: mb_truecont.c,v 5.4 2005-11-04 22:49:51 caress Exp $";
	char	*function_name = "mb_contour_deall";
	int	status = MB_SUCCESS;
	struct ping *ping;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		fprintf(stderr,"dbg2       data:             %d\n",data);
		}

	/* deallocate memory for bathymetry data */
	for (i=0;i<data->npings_max;i++)
		{
		ping = &data->pings[i];
		status = mb_free(verbose,&ping->beamflag,error);
		status = mb_free(verbose,&ping->bath,error);
		status = mb_free(verbose,&ping->bathlon,error);
		status = mb_free(verbose,&ping->bathlat,error);
		status = mb_free(verbose,&ping->bflag[0],error);
		status = mb_free(verbose,&ping->bflag[1],error);
		}
	status = mb_free(verbose,&data->pings,error);

	/* deallocate memory for contour controls */
	if (data->nlevel > 0)
		{
		status = mb_free(verbose,&data->level_list,error);
		status = mb_free(verbose,&data->label_list,error);
		status = mb_free(verbose,&data->tick_list,error);
		status = mb_free(verbose,&data->color_list,error);
		}

	/* deallocate memory for triangle network */
	status = mb_free(verbose,&data->x,error);
	status = mb_free(verbose,&data->y,error);
	status = mb_free(verbose,&data->z,error);
	status = mb_free(verbose,&data->edge,error);
	status = mb_free(verbose,&data->pingid,error);
	for (i=0;i<3;i++)
		{
		status = mb_free(verbose,&data->iv[i],error);
		status = mb_free(verbose,&data->ct[i],error);
		status = mb_free(verbose,&data->cs[i],error);
		status = mb_free(verbose,&data->ed[i],error);
		status = mb_free(verbose,&data->flag[i],error);
		}
	status = mb_free(verbose,&data->v1,error);
	status = mb_free(verbose,&data->v2,error);
	status = mb_free(verbose,&data->v3,error);
	status = mb_free(verbose,&data->istack,error);
	status = mb_free(verbose,&data->kv1,error);
	status = mb_free(verbose,&data->kv2,error);

	/* deallocate memory for contour positions */
	status = mb_free(verbose,&data->xsave,error);
	status = mb_free(verbose,&data->ysave,error);
	status = mb_free(verbose,&data->isave,error);
	status = mb_free(verbose,&data->jsave,error);

	/* deallocate memory for contour labels */
	status = mb_free(verbose,&data->xlabel,error);
	status = mb_free(verbose,&data->ylabel,error);
	status = mb_free(verbose,&data->angle,error);
	status = mb_free(verbose,&data->justify,error);

	/* deallocate memory for swath structure */
	status = mb_free(verbose,&data,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	return(MB_SUCCESS);
}
/*--------------------------------------------------------------------------*/
/* 	function mb_contour calls the appropriate contouring routine. */
int mb_contour(
		int	verbose, 
		struct swath *data, 
		int	*error)
{
  	static char rcs_id[]="";
	char	*function_name = "mb_contour";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		fprintf(stderr,"dbg2       data:             %d\n",data);
		fprintf(stderr,"dbg2       data->contour_alg:%d\n",data->contour_algorithm);
		}

	/* call the appropriate contouring routine */
	if (data->contour_algorithm == MB_CONTOUR_TRIANGLES)
		status = mb_tcontour(verbose,data,error);
	else
		status = mb_ocontour(verbose,data,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------------*/
/* 	function mb_tcontour contours multibeam data. */
int mb_tcontour(
		int	verbose, 
		struct swath *data, 
		int	*error)
{
  	static char rcs_id[]="$Id: mb_truecont.c,v 5.4 2005-11-04 22:49:51 caress Exp $";
	char	*function_name = "mb_tcontour";
	int	status = MB_SUCCESS;
	struct ping *ping;
	int	npt_cnt;
	int	ntri_cnt;
	int	extreme_start;
	int	left, right;
	double	bathmin, bathmax, xmin, xmax, ymin, ymax;
	int	nci, ncf;
	double	eps;
	int	ival;
	double	value;
	int	tick, label;
	int	itri, iside1, iside2, closed;
	int	itristart, isidestart, itriend, isideend;
	double	x, y;
	double	magdis;
	int	hand;
	int	tick_last;
	int	i, j, k, jj;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		fprintf(stderr,"dbg2       data:             %d\n",data);
		fprintf(stderr,"dbg2       data->contour_algorithm: %d\n",data->contour_algorithm);
		fprintf(stderr,"dbg2       data->plot_contours:     %d\n",data->plot_contours);
		fprintf(stderr,"dbg2       data->plot_triangles:    %d\n",data->plot_triangles);
		fprintf(stderr,"dbg2       data->plot_track:        %d\n",data->plot_track);
		fprintf(stderr,"dbg2       data->plot_name:         %d\n",data->plot_name);
		fprintf(stderr,"dbg2       data->contour_int:       %f\n",data->contour_int);
		fprintf(stderr,"dbg2       data->color_int:         %f\n",data->color_int);
		fprintf(stderr,"dbg2       data->tick_int:          %f\n",data->tick_int);
		fprintf(stderr,"dbg2       data->label_int:         %f\n",data->label_int);
		fprintf(stderr,"dbg2       data->tick_len:          %f\n",data->tick_len);
		fprintf(stderr,"dbg2       data->label_hgt:         %f\n",data->label_hgt);
		fprintf(stderr,"dbg2       data->label_spacing:     %f\n",data->label_spacing);
		fprintf(stderr,"dbg2       data->ncolor:            %d\n",data->ncolor);
		fprintf(stderr,"dbg2       data->nlevel:            %d\n",data->nlevel);
		fprintf(stderr,"dbg2       data->nlevelset:         %d\n",data->nlevelset);
		if (data->nlevelset == MB_YES)
		for (i=0;i<data->nlevel;i++)
			{
			fprintf(stderr,"dbg2          level[%3d]:  %f %d %d %d\n",
				i,data->level_list[i],data->label_list[i],data->tick_list[i],data->color_list[i]);
			}
		fprintf(stderr,"dbg2       data->npings:     %d\n",data->npings);
		fprintf(stderr,"dbg2       data->npings_max: %d\n",data->npings_max);
		fprintf(stderr,"dbg2       data->beams_bath: %d\n",data->beams_bath);
		for (i=0;i<data->npings;i++)
			{
			fprintf(stderr,"dbg2          ping[%4d]: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d:%6.6d %f %f %f %f %d\n",
				i,data->pings[i].time_i[0],data->pings[i].time_i[1],data->pings[i].time_i[2],
				data->pings[i].time_i[3],data->pings[i].time_i[4],data->pings[i].time_i[5],data->pings[i].time_i[6],
				data->pings[i].time_d,data->pings[i].navlon,data->pings[i].navlat,data->pings[i].heading,
				data->pings[i].beams_bath);
			for (j=0;j<data->pings[i].beams_bath;j++)
				{
				if (mb_beam_ok(data->pings[i].beamflag[j]))
				fprintf(stderr,"dbg2          beam[%4d:%3d]:  %2d %f %f %f\n",
					i,j,data->pings[i].beamflag[j],data->pings[i].bath[j],
					data->pings[i].bathlon[j],data->pings[i].bathlat[j]);
				}
			}
		}
		
	/* count number of points and verify that enough memory is allocated */
	npt_cnt = 0;
	for (i=0;i<data->npings;i++)
		{
		ping = &data->pings[i];
		for (j=0;j<ping->beams_bath;j++)
			{
			if (mb_beam_ok(ping->beamflag[j]))
				npt_cnt++;
			}
		}
	ntri_cnt = 3 * npt_cnt + 1;
	if (npt_cnt > data->npts_alloc)
		{
		data->npts_alloc = npt_cnt;
		status = mb_realloc(verbose,data->npts_alloc*sizeof(double),
			&(data->x),error);
		status = mb_realloc(verbose,data->npts_alloc*sizeof(double),
			&(data->y),error);
		status = mb_realloc(verbose,data->npts_alloc*sizeof(double),
			&(data->z),error);
		status = mb_realloc(verbose,data->npts_alloc*sizeof(int),
			&(data->edge),error);
		status = mb_realloc(verbose,data->npts_alloc*sizeof(int),
			&(data->pingid),error);
		}
	if (ntri_cnt > data->ntri_alloc)
		{
		data->ntri_alloc = ntri_cnt;
		for (i=0;i<3;i++)
			{
			status = mb_realloc(verbose,ntri_cnt*sizeof(int),
				 	&(data->iv[i]),error);
			status = mb_realloc(verbose,ntri_cnt*sizeof(int),
				 	&(data->ct[i]),error);
			status = mb_realloc(verbose,ntri_cnt*sizeof(int),
				 	&(data->cs[i]),error);
			status = mb_realloc(verbose,ntri_cnt*sizeof(int),
				 	&(data->ed[i]),error);
			status = mb_realloc(verbose,ntri_cnt*sizeof(int),
					&(data->flag[i]),error);
			}
		status = mb_realloc(verbose,ntri_cnt*sizeof(double),
					&(data->v1),error);
		status = mb_realloc(verbose,ntri_cnt*sizeof(double),
		 			&(data->v2),error);
		status = mb_realloc(verbose,ntri_cnt*sizeof(double),
		 			&(data->v3),error);
		status = mb_realloc(verbose,ntri_cnt*sizeof(int),
		 			&(data->istack),error);
		status = mb_realloc(verbose,3*ntri_cnt*sizeof(int),
		 			&(data->kv1),error);
		status = mb_realloc(verbose,3*ntri_cnt*sizeof(int),
		 			&(data->kv2),error);
		status = mb_realloc(verbose,(4*ntri_cnt+1)*sizeof(double),
				&(data->xsave),error);
		status = mb_realloc(verbose,(4*ntri_cnt+1)*sizeof(double),
				&(data->ysave),error);
		}

	/* put good bathymetry data into x arrays */
	data->npts = 0;
	extreme_start = MB_NO;
	for (i=0;i<data->npings;i++)
	  {
	  ping = &data->pings[i];

	  /* find edges of ping */
	  left = ping->beams_bath/2;
	  right = left;
	  for (j=0;j<ping->beams_bath;j++)
		{
		if (mb_beam_ok(ping->beamflag[j]) && j < left) left = j;
		if (mb_beam_ok(ping->beamflag[j]) && j > right) right = j;
		}

	  /* deal with data */
	  for (j=0;j<ping->beams_bath;j++)
		{
		if (extreme_start == MB_NO && mb_beam_ok(ping->beamflag[j]))
			{
			bathmin = ping->bath[j];
			bathmax = ping->bath[j];
			xmin = ping->bathlon[j];
			xmax = xmin;
			ymin = ping->bathlat[j];
			ymax = ymin;
			extreme_start = MB_YES;
			}
		if (mb_beam_ok(ping->beamflag[j]))
			{
			data->x[data->npts] = ping->bathlon[j];
			data->y[data->npts] = ping->bathlat[j];
			data->z[data->npts] = ping->bath[j];
			if (j == right)
				data->edge[data->npts] = 1;
			else if (j == left)
				data->edge[data->npts] = -1;
			else
				data->edge[data->npts] = 0;
			data->pingid[data->npts] = i;
			bathmin = MIN(bathmin, data->z[data->npts]);
			bathmax = MAX(bathmax, data->z[data->npts]);
			xmin = MIN(xmin, data->x[data->npts]);
			xmax = MAX(xmax, data->x[data->npts]);
			ymin = MIN(ymin, data->y[data->npts]);
			ymax = MAX(ymax, data->y[data->npts]);
			data->npts++;
			}
		}
	  }

	/* delete duplicate points */
	for (i=0;i<data->npts;i++)
		for (j=data->npts-1;j>i;j--)
			{
			if (data->x[i] == data->x[j] 
				&& data->y[i] == data->y[j])
				{
				data->z[i] = 0.5*(data->z[i] + data->z[j]);
				for (jj=j;jj<data->npts-1;jj++)
					{
					data->x[jj] = data->x[jj+1];
					data->y[jj] = data->y[jj+1];
					data->z[jj] = data->z[jj+1];
					data->edge[jj] = data->edge[jj+1];
					data->pingid[jj] = data->pingid[jj+1];
					}
				data->npts--;
				}
			}

	/* print debug statements */
	if (verbose >= 4)
		{
		fprintf(stderr,"\ndbg4  Data points:\n");
		fprintf(stderr,"dbg4       npts:             %d\n",data->npts);
		}
	if (verbose >= 5)
		{
		for (i=0;i<data->npts;i++)
			fprintf(stderr,"dbg2       %4d %4d  %f %f %f\n",
				i,data->pingid[i],
				data->x[i],data->y[i],data->z[i]);
		}

	/* if no depth variation dont bother */
	if ((bathmax - bathmin) < EPS) return(status);

	/* get number of contour intervals */
	if (data->nlevelset == MB_NO)
		{
		if (data->nlevel > 0)
			{
			mb_free(verbose,&data->level_list,error);
			mb_free(verbose,&data->color_list,error);
			mb_free(verbose,&data->label_list,error);
			mb_free(verbose,&data->tick_list,error);
			}
		nci = bathmin/data->contour_int + 1;
		ncf = bathmax/data->contour_int + 1;
		data->nlevel = ncf - nci;
		status = mb_malloc(verbose,data->nlevel*sizeof(double),
				&(data->level_list),error);
		status = mb_malloc(verbose,data->nlevel*sizeof(int),
				&(data->color_list),error);
		status = mb_malloc(verbose,data->nlevel*sizeof(int),
				&(data->label_list),error);
		status = mb_malloc(verbose,data->nlevel*sizeof(int),
				&(data->tick_list),error);
		if (*error != MB_ERROR_NO_ERROR) return(status);
		for (i=0;i<data->nlevel;i++)
			{
			k = nci + i;
			data->level_list[i] = k*data->contour_int;
			data->color_list[i] = 
				(int)(data->level_list[i]/data->color_int)
					%data->ncolor;
			if (data->tick_int <= 0.0)
				data->tick_list[i] = 0;
			else if (((int)data->level_list[i])
					%((int)data->tick_int) == 0) 
				data->tick_list[i] = 1;
			else
				data->tick_list[i] = 0;
			if (data->label_int <= 0.0)
				data->label_list[i] = 0;
			else if (((int)data->level_list[i])
					%((int)data->label_int) == 0) 
				data->label_list[i] = 1;
			else
				data->label_list[i] = 0;
			}
		}

	/* print debug statements */
	if (verbose >= 4)
		{
		fprintf(stderr,"\ndbg4  Data points:\n");
		fprintf(stderr,"dbg4       nlevel:           %d\n",data->nlevel);
		fprintf(stderr,"dbg4       i level color tick label:\n");
		for (i=0;i<data->nlevel;i++)
			fprintf(stderr,"dbg4       %d %f %d %d %d\n",
				i,data->level_list[i],data->color_list[i],
				data->tick_list[i],data->label_list[i]);
		}

	/* make sure that no depths are exact contour values */
	eps = EPS*(bathmax - bathmin);
	for (k=0;k<data->nlevel;k++)
		{
		for (i=0;i<data->npts;i++)
			{
			if (fabs(data->z[i] - data->level_list[k]) < eps)
				data->z[i] = data->level_list[k] + eps;
			}
		}

	/* get triangle network */
	status = mb_delaun(verbose,data->npts,data->x,data->y,data->edge,
			&data->ntri,data->iv[0],data->iv[1],data->iv[2],
			data->ct[0],data->ct[1],data->ct[2],
			data->cs[0],data->cs[1],data->cs[2],
			data->v1,data->v2,data->v3,
			data->istack,data->kv1,data->kv2,error);
	if (verbose > 1)
		fprintf(stderr,"\n");
	if (verbose > 0)
		fprintf(stderr,"Obtained %d triangles from %d points in %d pings...\n",
		data->ntri,data->npts,data->npings);

	/* figure out which triangle sides are on the swath edge */
	for (itri=0;itri<data->ntri;itri++)
	  {
	  for (j=0;j<3;j++)
		{
		jj = j + 1;
		if (jj > 2) jj = 0;
		if (data->edge[data->iv[j][itri]] == -1
			&& data->edge[data->iv[jj][itri]] == -1)
			data->ed[j][itri] = -1;
		else if (data->edge[data->iv[j][itri]] == 1
			&& data->edge[data->iv[jj][itri]] == 1)
			data->ed[j][itri] = 1;
		else
			data->ed[j][itri] = 0;
		}
	  }

	/* plot the triangles if desired */
	if (data->plot_triangles)
	  {
	  newpen(0);
	  for (itri=0;itri<data->ntri;itri++)
		{
		plot(data->x[data->iv[0][itri]],
			data->y[data->iv[0][itri]],IUP);
		plot(data->x[data->iv[1][itri]],
			data->y[data->iv[1][itri]],IDN);
		plot(data->x[data->iv[2][itri]],
			data->y[data->iv[2][itri]],IDN);
		plot(data->x[data->iv[0][itri]],
			data->y[data->iv[0][itri]],IDN);
		}
	  }

	/* loop over all of the contour values */
	data->nsave = 0;
	data->nlabel = 0;
	if (status == MB_SUCCESS && data->plot_contours == MB_YES)
	for (ival=0;ival<data->nlevel;ival++)
		{
		value = data->level_list[ival];
		newpen(data->color_list[ival]);
		tick = data->tick_list[ival];
		label = data->label_list[ival];

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  About to contour level in function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4       value:         %f\n",value);
			fprintf(stderr,"dbg4       tick:          %d\n",tick);
			fprintf(stderr,"dbg4       label:         %d\n",label);
			}

		/* flag all triangle sides crossed by the current contour */
		for (itri=0;itri<data->ntri;itri++)
			{
			for (j=0;j<3;j++)
				{
				jj = j + 1;
				if (jj == 3) jj = 0;
				if ((data->z[data->iv[j][itri]] > value 
					&& data->z[data->iv[jj][itri]] < value)
					|| (data->z[data->iv[jj][itri]] > value 
					&& data->z[data->iv[j][itri]] < value))
					data->flag[j][itri] = 1;
				else
					data->flag[j][itri] = 0;
				}
			}

		/* do the contouring */
		data->nsave = 0;
		while (get_start_tri(data,&itri,&iside1,&iside2,&closed))
			{
			/* if not closed remove flags */
			data->flag[iside1][itri] = -1;
			data->flag[iside2][itri] = -1;

			/* get position of start of contour */
			get_pos_tri(data,eps,itri,iside1,value,
				&data->xsave[data->nsave],
				&data->ysave[data->nsave]);
			data->nsave++;
			get_pos_tri(data,eps,itri,iside2,value,
				&data->xsave[data->nsave],
				&data->ysave[data->nsave]);
			data->nsave++;
			itristart = itri;
			isidestart = iside1;
			itriend = itri;
			isideend = iside2;

			/* set tick flag */
			tick_last = MB_NO;

			/* look for next segment */
			while(get_next_tri(data,&itri,&iside1,&iside2,&closed,
				&itristart,&isidestart))
				{
				/* get position */
				get_pos_tri(data,eps,itri,iside2,value,&x,&y);

				/* deal with tick as needed */
				if (tick && tick_last == MB_NO)
					{
					if (data->z[data->iv[iside1][itri]] > 
						data->z[data->iv[iside2][itri]])
						hand = -1;
					else
						hand = 1;
					data->xsave[data->nsave] =  
					  0.5*(x + data->xsave[data->nsave-1]);
					data->ysave[data->nsave] =  
					  0.5*(y + data->ysave[data->nsave-1]);
					magdis = sqrt(pow((x - 
					  data->xsave[data->nsave-1]),2.0)
					  + pow((y - data->ysave[data->nsave-1]),2.0));
					data->xsave[data->nsave+1] = 
					  data->xsave[data->nsave] 
					  - hand*data->tick_len*
					  (y - data->ysave[data->nsave-1])
					  /magdis;
					data->ysave[data->nsave+1] = 
					  data->ysave[data->nsave] 
					  + hand*data->tick_len*
					  (x - data->xsave[data->nsave-1])
					  /magdis;
					data->xsave[data->nsave+2] = 
						data->xsave[data->nsave];
					data->ysave[data->nsave+2] = 
						data->ysave[data->nsave];
					data->xsave[data->nsave+3] = x;
					data->ysave[data->nsave+3] = y;
					data->flag[iside1][itri] = -1;
					data->flag[iside2][itri] = -1;
					data->nsave = data->nsave + 4;
					tick_last = MB_YES;
					}
				else
					{
					data->xsave[data->nsave] = x;
					data->ysave[data->nsave] = y;
					data->flag[iside1][itri] = -1;
					data->flag[iside2][itri] = -1;
					data->nsave++;
					tick_last = MB_NO;
					}

				/* set latest point */
				itriend = itri;
				isideend = iside2;
				}

			/* set label if needed */
			if (label && !closed 
				&& data->ed[isidestart][itristart] != 0)
				{
				data->xlabel[data->nlabel] = data->xsave[0];
				data->ylabel[data->nlabel] = data->ysave[0];
				get_azimuth_tri(data,itristart,isidestart,
					&data->angle[data->nlabel]);
				if (data->ed[isidestart][itristart] == -1)
					data->justify[data->nlabel] = 1;
				else
					data->justify[data->nlabel] = 0;
				if (check_label(data,data->nlabel))
					data->nlabel++;
				}
			if (label && !closed 
				&& data->ed[isideend][itriend] != 0)
				{
				data->xlabel[data->nlabel] 
					= data->xsave[data->nsave-1];
				data->ylabel[data->nlabel] 
					= data->ysave[data->nsave-1];
				get_azimuth_tri(data,itriend,isideend,
					&data->angle[data->nlabel]);
				if (data->ed[isideend][itriend] == -1)
					data->justify[data->nlabel] = 1;
				else
					data->justify[data->nlabel] = 0;
				if (check_label(data,data->nlabel))
					data->nlabel++;
				}

			/* dump the contour */
			dump_contour(data,value);
			}

		/* done with contouring this level */
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	return(status);
}

/*--------------------------------------------------------------------------*/
/* 	function get_start_tri finds next contour starting point. */
int get_start_tri(
		struct swath *data, 
		int	*itri, 
		int	*iside1, 
		int	*iside2, 
		int	*closed)
{
	int	isave;
	int	i, j, jj;

	/* search triangles */
	*closed = MB_NO;
	for (i=0;i<data->ntri;i++)
		for (j=0;j<3;j++)
			{
			if (data->flag[j][i] > 0)
				{
				/* find two flagged sides */
				*itri = i;
				*iside1 = j;
				*iside2 = -1;
				for (jj=0;jj<3;jj++)
					if (jj != j && data->flag[jj][i] > 0)
						*iside2 = jj;
		if (*iside2 == -1)
			{
			fprintf(stderr,"no flagged side in get_start_tri???\n");
			fprintf(stderr,"noflag: itri:%d flags: %d %d %d\n",
				*itri,data->flag[0][*itri],
				data->flag[1][*itri],data->flag[2][*itri]);
			}

				/* check if contour continues on both sides */
				if (data->ct[*iside1][i] > -1
					&& data->ct[*iside2][i] > -1)
					*closed = MB_YES;

				/* else make sure contour starts at dead end */
				else if (data->ct[*iside1][i] > -1)
					{
					isave = *iside1;
					*iside1 = *iside2;
					*iside2 = isave;
					*closed = MB_NO;
					}
				else
					*closed = MB_NO;
				return(MB_YES);
				}
			}

	/* nothing found */
	return(MB_NO);
}
/*--------------------------------------------------------------------------*/
/* 	function get_next_tri finds next contour component if it exists */
int get_next_tri(
		struct swath *data, 
		int	*itri, 
		int	*iside1, 
		int	*iside2, 
		int	*closed, 
		int	*itristart, 
		int	*isidestart)
{
	double	xs, ys;
	int	itrisave, isidesave;
	int	i, j;

	/* check if contour ends where it began */
	if (*closed && data->ct[*iside2][*itri] == *itristart 
		&& data->cs[*iside2][*itri] == *isidestart)
		return(MB_NO);

	/* check if current triangle side connects to another */
	else if (data->ct[*iside2][*itri] > -1)
		{
		*iside1 = data->cs[*iside2][*itri];
		*itri = data->ct[*iside2][*itri];
		*iside2 = -1;
		for (j=0;j<3;j++)
			if (j != *iside1 && data->flag[j][*itri] != 0)
				*iside2 = j;
		if (*iside2 == -1)
			{
			fprintf(stderr,"no flagged side in get_next_tri???\n");
			fprintf(stderr,"noflag: itri:%d flags: %d %d %d\n",
				*itri,data->flag[0][*itri],
				data->flag[1][*itri],data->flag[2][*itri]);
			return(MB_NO);
			}
		return(MB_YES);
		}

	/* else if contour ends but closed set true then 
		turn contour around and continue in other direction */
	else if (*closed)
		{
		for (i=0;i<data->nsave/2;i++)
			{
			xs = data->xsave[i];
			ys = data->ysave[i];
			data->xsave[i] = data->xsave[data->nsave-i-1];
			data->ysave[i] = data->ysave[data->nsave-i-1];
			data->xsave[data->nsave-i-1] = xs;
			data->ysave[data->nsave-i-1] = ys;
			}
		*closed = MB_NO;
		data->nsave--;
		itrisave = *itristart;
		isidesave = *isidestart;
		*itristart = *itri;
		*isidestart = *iside2;
		*itri = itrisave;
		*iside2 = isidesave;
		*iside1 = -1;
		for (j=0;j<3;j++)
			if (j != *iside2 && data->flag[j][*itri] != 0)
				*iside1 = j;

		/* if next side not found end contour */
		if (*iside1 == -1)
			return(MB_NO);

		/* else keep going */
		return(MB_YES);
		}

	/* else contour ends and is not closed */
	else
		return(MB_NO);
}
/*--------------------------------------------------------------------------*/
/* 	function get_pos_tri finds position of contour crossing point */
int get_pos_tri(
		struct swath *data, 
		double	eps, 
		int	itri, 
		int	iside, 
		double	value, 
		double	*x, 
		double	*y)
{
	double	factor;
	int	v1, v2, pt1, pt2;

	v1 = iside;
	v2 = iside + 1;
	if (v2 == 3) v2 = 0;
	pt1 = data->iv[v1][itri];
	pt2 = data->iv[v2][itri];

	if (fabs(data->z[pt2] - data->z[pt1]) > eps)
		factor = (value - data->z[pt1])/(data->z[pt2] - data->z[pt1]);
	else
		factor = 0.5;
	*x = data->x[pt1] + factor*(data->x[pt2] - data->x[pt1]);
	*y = data->y[pt1] + factor*(data->y[pt2] - data->y[pt1]);

	return(MB_YES);
	
}
/*--------------------------------------------------------------------------*/
/* 	function get_azimuth_tri gets azimuth across track for a label */
int get_azimuth_tri(
		struct swath *data, 
		int	itri, 
		int	iside, 
		double	*angle)
{
	double	heading;

	*angle = -data->pings[data->pingid[data->iv[iside][itri]]].heading;
	if (*angle > 180.0)
		*angle = *angle - 360.0;
	if (*angle < -180.0)
		*angle = *angle + 360.0;

	return(MB_YES);
}
/*--------------------------------------------------------------------------*/
/* 	function check_label checks if new label will overwrite any recent
 *	labels. */
int check_label(struct swath *data, 
		int	nlab)
{
#define	MAXHIS 30
	static double xlabel_his[MAXHIS], ylabel_his[MAXHIS];
	static int nlabel_his = 0;
	double	rad_label_his;
	int	good, ilab, i;
	double	dx, dy, rr;

	good = 1;
	ilab = 0;
	rad_label_his = data->label_spacing;
	while (good && ilab < nlabel_his)
		{
		dx = xlabel_his[ilab] - data->xlabel[nlab];
		dy = ylabel_his[ilab] - data->ylabel[nlab];
		rr = sqrt(dx*dx + dy*dy);
		if (rr < rad_label_his) good = 0;
		ilab++;
		}
	ilab--;
	if (good)
		{
		nlabel_his++;
		if (nlabel_his >= MAXHIS) 
			nlabel_his = MAXHIS - 1;
		for (i=nlabel_his;i>0;i--)
			{
			xlabel_his[i] = xlabel_his[i-1];
			ylabel_his[i] = ylabel_his[i-1];
			}
		xlabel_his[0] = data->xlabel[nlab];
		ylabel_his[0] = data->ylabel[nlab];
		}
	return(good);
}
/*--------------------------------------------------------------------------*/
/* 	function dump_contour dumps the contour stored in xsave and ysave
 *	to the plotting routines */
int dump_contour(struct swath *data, double value)
{
	int	i;
	char	label[25];
	int	len;
	double	x, y, dx, dy, s[4];
	double	mtodeglon, mtodeglat;

	/* plot the contours */
	if (data->nsave < 2) return(MB_NO);
	plot(data->xsave[0],data->ysave[0],IUP);
	for (i=1;i<data->nsave;i++)
		plot(data->xsave[i],data->ysave[i],IDN);
	data->nsave = 0;

	/* plot the labels */
	sprintf(label,"  %d",(int)value);
	len = strlen(label);
	for (i=0;i<data->nlabel;i++)
		{
		if (data->justify[i] == 1)
			{
			mb_coor_scale(0,data->ylabel[i],&mtodeglon,&mtodeglat);
			justify_string(data->label_hgt,label,s);
			dx = 1.5*s[2]*cos(DTR*data->angle[i]);
			dy = 1.5*mtodeglat/mtodeglon*s[2]
				*sin(DTR*data->angle[i]);
			x = data->xlabel[i] - dx;
			y = data->ylabel[i] - dy;
			plot_string(x,y,data->label_hgt,data->angle[i],label);
			}
		else
			{
			plot_string(data->xlabel[i],data->ylabel[i],
				data->label_hgt,data->angle[i],label);
			}
		}
	data->nlabel = 0;


	return(MB_YES);
}
/*--------------------------------------------------------------------------*/
/* 	function mb_ocontour contours multibeam data. */
int mb_ocontour(int verbose, struct swath *data, int *error)
{
  	static char rcs_id[]="$Id: mb_truecont.c,v 5.4 2005-11-04 22:49:51 caress Exp $";
	char	*function_name = "mb_ocontour";
	int	status = MB_SUCCESS;
	struct ping *ping;
	int	npt_cnt;
	int	extreme_start;
	int	beams_bath_use;
	int	left, right;
	char	*beamflag1, *beamflag2;
	double	*bath1, *bath2;
	double	bathmin, bathmax;
	int	ibeg, jbeg, kbeg, dbeg;
	int	ni, nj, nk, nd;
	int	nci, ncf;
	double	eps;
	int	ival;
	double	value;
	int	tick, label;
	int	itri, iside1, iside2, closed;
	int	itristart, isidestart, itriend, isideend;
	double	x, y;
	double	magdis;
	int	hand;
	int	tick_last;
	int	i, j, k, d, jj;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		fprintf(stderr,"dbg2       data:             %d\n",data);
		fprintf(stderr,"dbg2       data->contour_algorithm: %d\n",data->contour_algorithm);
		fprintf(stderr,"dbg2       data->plot_contours:     %d\n",data->plot_contours);
		fprintf(stderr,"dbg2       data->plot_triangles:    %d\n",data->plot_triangles);
		fprintf(stderr,"dbg2       data->plot_track:        %d\n",data->plot_track);
		fprintf(stderr,"dbg2       data->plot_name:         %d\n",data->plot_name);
		fprintf(stderr,"dbg2       data->contour_int:       %f\n",data->contour_int);
		fprintf(stderr,"dbg2       data->color_int:         %f\n",data->color_int);
		fprintf(stderr,"dbg2       data->tick_int:          %f\n",data->tick_int);
		fprintf(stderr,"dbg2       data->label_int:         %f\n",data->label_int);
		fprintf(stderr,"dbg2       data->tick_len:          %f\n",data->tick_len);
		fprintf(stderr,"dbg2       data->label_hgt:         %f\n",data->label_hgt);
		fprintf(stderr,"dbg2       data->label_spacing:     %f\n",data->label_spacing);
		fprintf(stderr,"dbg2       data->ncolor:            %d\n",data->ncolor);
		fprintf(stderr,"dbg2       data->nlevel:            %d\n",data->nlevel);
		fprintf(stderr,"dbg2       data->nlevelset:         %d\n",data->nlevelset);
		if (data->nlevelset == MB_YES)
		for (i=0;i<data->nlevel;i++)
			{
			fprintf(stderr,"dbg2          level[%3d]:  %f %d %d %d\n",
				i,data->level_list[i],data->label_list[i],data->tick_list[i],data->color_list[i]);
			}
		fprintf(stderr,"dbg2       data->npings:     %d\n",data->npings);
		fprintf(stderr,"dbg2       data->npings_max: %d\n",data->npings_max);
		fprintf(stderr,"dbg2       data->beams_bath: %d\n",data->beams_bath);
		for (i=0;i<data->npings;i++)
			{
			fprintf(stderr,"dbg2          ping[%4d]: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d:%6.6d %f %f %f %f %d\n",
				i,data->pings[i].time_i[0],data->pings[i].time_i[1],data->pings[i].time_i[2],
				data->pings[i].time_i[3],data->pings[i].time_i[4],data->pings[i].time_i[5],data->pings[i].time_i[6],
				data->pings[i].time_d,data->pings[i].navlon,data->pings[i].navlat,data->pings[i].heading,
				data->pings[i].beams_bath);
			for (j=0;j<data->pings[i].beams_bath;j++)
				{
				if (mb_beam_ok(data->pings[i].beamflag[j]))
				fprintf(stderr,"dbg2          beam[%4d:%3d]:  %2d %f %f %f\n",
					i,j,data->pings[i].beamflag[j],data->pings[i].bath[j],
					data->pings[i].bathlon[j],data->pings[i].bathlat[j]);
				}
			}
		}
		
	/* count number of points and verify that enough memory is allocated */
	npt_cnt = 0;
	for (i=0;i<data->npings;i++)
		{
		ping = &data->pings[i];
		for (j=0;j<ping->beams_bath;j++)
			{
			if (mb_beam_ok(ping->beamflag[j]))
				npt_cnt++;
			}
		}
	if (npt_cnt > data->npts_alloc)
		{
		data->npts_alloc = npt_cnt;
		status = mb_realloc(verbose,data->npts_alloc*sizeof(double),
				&(data->xsave),error);
		status = mb_realloc(verbose,data->npts_alloc*sizeof(double),
				&(data->ysave),error);
		status = mb_realloc(verbose,data->npts_alloc*sizeof(int),
				&(data->isave),error);
		status = mb_realloc(verbose,data->npts_alloc*sizeof(int),
				&(data->jsave),error);
		}

	/* zero flags */
	for (i=0;i<data->npings;i++)
		{
		ping = &data->pings[i];
		for (j=0;j<ping->beams_bath;j++)
			{
			ping->bflag[0][j] = 0;
			ping->bflag[1][j] = 0;
			}
		}

	/* get min max of bathymetry */
	extreme_start = MB_NO;
	for (i=0;i<data->npings;i++)
	  {
	  ping = &data->pings[i];
	  for (j=0;j<ping->beams_bath;j++)
		{
		if (extreme_start == MB_NO && mb_beam_ok(ping->beamflag[j]))
			{
			bathmin = ping->bath[j];
			bathmax = ping->bath[j];
			extreme_start = MB_YES;
			}
		if (mb_beam_ok(ping->beamflag[j]))
			{
			bathmin = MIN(bathmin, ping->bath[j]);
			bathmax = MAX(bathmax, ping->bath[j]);
			}
		}
	  }

	/* if no depth variation dont bother */
	if ((bathmax - bathmin) < EPS) return(status);

	/* get number of contour intervals */
	if (data->nlevelset == MB_NO)
		{
		if (data->nlevel > 0)
			{
			mb_free(verbose,&data->level_list,error);
			mb_free(verbose,&data->color_list,error);
			mb_free(verbose,&data->label_list,error);
			mb_free(verbose,&data->tick_list,error);
			}
		nci = bathmin/data->contour_int + 1;
		ncf = bathmax/data->contour_int + 1;
		data->nlevel = ncf - nci;
		status = mb_malloc(verbose,data->nlevel*sizeof(double),
				&(data->level_list),error);
		status = mb_malloc(verbose,data->nlevel*sizeof(int),
				&(data->color_list),error);
		status = mb_malloc(verbose,data->nlevel*sizeof(int),
				&(data->label_list),error);
		status = mb_malloc(verbose,data->nlevel*sizeof(int),
				&(data->tick_list),error);
		if (*error != MB_ERROR_NO_ERROR) return(status);
		for (i=0;i<data->nlevel;i++)
			{
			k = nci + i;
			data->level_list[i] = k*data->contour_int;
			data->color_list[i] = 
				(int)(data->level_list[i]/data->color_int)
					%data->ncolor;
			if (data->tick_int <= 0.0)
				data->tick_list[i] = 0;
			else if (((int)data->level_list[i])
					%((int)data->tick_int) == 0) 
				data->tick_list[i] = 1;
			else
				data->tick_list[i] = 0;
			if (data->label_int <= 0.0)
				data->label_list[i] = 0;
			else if (((int)data->level_list[i])
					%((int)data->label_int) == 0) 
				data->label_list[i] = 1;
			else
				data->label_list[i] = 0;
			}
		}

	/* print debug statements */
	if (verbose >= 4)
		{
		fprintf(stderr,"\ndbg4  Data points:\n");
		fprintf(stderr,"dbg4       nlevel:           %d\n",data->nlevel);
		fprintf(stderr,"dbg4       i level color tick label:\n");
		for (i=0;i<data->nlevel;i++)
			fprintf(stderr,"dbg4       %d %f %d %d %d\n",
				i,data->level_list[i],data->color_list[i],
				data->tick_list[i],data->label_list[i]);
		}

	/* make sure that no depths are exact contour values */
	eps = EPS*(bathmax - bathmin);
	for (k=0;k<data->nlevel;k++)
		{
		for (i=0;i<data->npts;i++)
			{
			if (fabs(data->z[i] - data->level_list[k]) < eps)
				data->z[i] = data->level_list[k] + eps;
			}
		}

	/* loop over all of the contour values */
	data->nsave = 0;
	data->nlabel = 0;
	if (status == MB_SUCCESS && data->plot_contours == MB_YES)
	for (ival=0;ival<data->nlevel;ival++)
		{
		value = data->level_list[ival];
		newpen(data->color_list[ival]);
		tick = data->tick_list[ival];
		label = data->label_list[ival];

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  About to contour level in function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4       value:         %f\n",value);
			fprintf(stderr,"dbg4       tick:          %d\n",tick);
			fprintf(stderr,"dbg4       label:         %d\n",label);
			}

		/* flag all grid sides crossed by the current contour */
		for (i=0;i<data->npings;i++)
			{
			beamflag1 = data->pings[i].beamflag;
			bath1 = data->pings[i].bath;
			beams_bath_use = data->pings[i].beams_bath;
			if (i<data->npings-1) 
			    {
			    beamflag2 = data->pings[i+1].beamflag;
			    bath2 = data->pings[i+1].bath;
			    beams_bath_use = MIN(beams_bath_use, data->pings[i+1].beams_bath);
			    }
			for (j=0;j<beams_bath_use;j++)
				{
				/* check for across track intersection */
				if (j < beams_bath_use-1)
					if ((mb_beam_ok(beamflag1[j]) && mb_beam_ok(beamflag1[j+1])) 
					&& ((bath1[j]<value && bath1[j+1]>value)
					|| (bath1[j]>value && bath1[j+1]<value)))
						data->pings[i].bflag[0][j] = 1;

				/* check for along track intersection */
				if (i < data->npings-1)
					if ((mb_beam_ok(beamflag1[j]) && mb_beam_ok(beamflag2[j])) 
					&& ((bath1[j]<value && bath2[j]>value) 
					|| (bath1[j]>value && bath2[j]<value)))
						data->pings[i].bflag[1][j] = 1;
				}
			}

		/* loop until all flagged points have been unflagged */
		while (get_start_old(data,&k, &i, &j, &d, &closed))
			{
			/* if not closed remove from flag list */
			if (closed == 0) data->pings[i].bflag[k][j] = 0;

			/* get position and handedness */
			get_pos_old(data,eps,&x,&y,k,i,j,value);
			data->xsave[0] = x;
			data->ysave[0] = y;
			data->isave[0] = i;
			data->jsave[0] = j;
			data->nsave = 1;
			data->nlabel = 0;
			ibeg = i;
			jbeg = j;
			kbeg = k;
			dbeg = d;

			/* set tick flag */
			tick_last = MB_NO;

			/* look for next component */
			while (get_next_old(data,&nk,&ni,&nj,&nd,k,i,j,d,
				kbeg,ibeg,jbeg,dbeg,&closed))
				{
				/* get position */
				get_pos_old(data,eps,&x,&y,nk,ni,nj,value);
				get_hand_old(data,&hand,k,i,j,d);
				if (tick && tick_last == MB_NO)
					{
					data->xsave[data->nsave] = 
						0.5*(x + 
						data->xsave[data->nsave-1]);
					data->ysave[data->nsave] = 
						0.5*(y + 
						data->ysave[data->nsave-1]);
					magdis = sqrt(pow((x 
						- data->xsave[data->nsave-1]),2.0) 
						+ pow((y 
						- data->ysave[data->nsave-1]),2.0));
					if (magdis > 0.0)
					    {
					    data->xsave[data->nsave+1] = 
						    data->xsave[data->nsave] 
						    - hand*data->tick_len*(y 
						    - data->ysave[data->nsave-1])/magdis;
					    data->ysave[data->nsave+1] = 
						    data->ysave[data->nsave] 
						    + hand*data->tick_len*(x 
						    - data->xsave[data->nsave-1])/magdis;
					    }
					else
					    {
					    data->xsave[data->nsave+1] = 
						    data->xsave[data->nsave];
					    data->ysave[data->nsave+1] = 
						    data->ysave[data->nsave];
					    }
					data->xsave[data->nsave+2] = 
						data->xsave[data->nsave];
					data->ysave[data->nsave+2] = 
						data->ysave[data->nsave];
					data->xsave[data->nsave+3] = x;
					data->ysave[data->nsave+3] = y;
					data->isave[data->nsave] = ni;
					data->jsave[data->nsave] = nj;
					data->isave[data->nsave+1] = ni;
					data->jsave[data->nsave+1] = nj;
					data->isave[data->nsave+2] = ni;
					data->jsave[data->nsave+2] = nj;
					data->isave[data->nsave+3] = ni;
					data->jsave[data->nsave+3] = nj;
					data->nsave = data->nsave + 4;
					tick_last = MB_YES;
					}
				else
					{
					data->xsave[data->nsave] = x;
					data->ysave[data->nsave] = y;
					data->isave[data->nsave] = ni;
					data->jsave[data->nsave] = nj;
					data->nsave++;
					tick_last = MB_NO;
					}
				i = ni;
				j = nj;
				k = nk;
				d = nd;
				data->pings[i].bflag[k][j] = 0;
				}

			/* clean up if not a full contour */
			if (data->nsave < 2)
				{
				data->nsave = 0;
				data->pings[i].bflag[k][j] = 0;
				}

			/* set labels if needed */
			if (data->nsave > 0 && label && !closed)
				{
				/* check beginning of contour */
				left = data->pings[data->isave[0]].beams_bath/2;
				right = data->pings[data->isave[0]].beams_bath/2;
				for (jj=0;jj<data->beams_bath;jj++)
					{
					if (mb_beam_ok(data->pings[data->isave[0]].beamflag[jj]))
						{
						if (jj < left) left = jj;
						if (jj > right) right = jj;
						}
					}
				if (data->jsave[0] == left 
					|| data->jsave[0] == left+1)
					{
					data->xlabel[data->nlabel] = data->xsave[0];
					data->ylabel[data->nlabel] = data->ysave[0];
					get_azimuth_old(data,data->isave[0],
						&data->angle[data->nlabel]);
					data->justify[data->nlabel] = 1;
					if (check_label(data,data->nlabel)) 
						data->nlabel++;
					}
				else if (data->jsave[0] == right 
					|| data->jsave[0] == right-1)
					{
					data->xlabel[data->nlabel] = data->xsave[0];
					data->ylabel[data->nlabel] = data->ysave[0];
					get_azimuth_old(data,data->isave[0],
						&data->angle[data->nlabel]);
					data->justify[data->nlabel] = 0;
					if (check_label(data,data->nlabel)) 
						data->nlabel++;
					}

				/* check end of contour */
				left = data->pings[data->isave[data->nsave-1]].beams_bath/2;
				right = data->pings[data->isave[data->nsave-1]].beams_bath/2;
				for (jj=0;jj<data->pings[data->isave[data->nsave-1]].beams_bath;jj++)
					{
					if (mb_beam_ok(data->pings[data->isave[data->nsave-1]].beamflag[jj]))
						{
						if (jj < left) left = jj;
						if (jj > right) right = jj;
						}
					}
				if ((data->nlabel == 0 || data->nsave > 10) 
					&& (data->jsave[data->nsave-1] == left 
						|| data->jsave[data->nsave-1] 
							== left+1))
					{
					data->xlabel[data->nlabel] 
						= data->xsave[data->nsave-1];
					data->ylabel[data->nlabel] 
						= data->ysave[data->nsave-1];
					get_azimuth_old(data,data->isave[data->nsave-1],
						&data->angle[data->nlabel]);
					data->justify[data->nlabel] = 1;
					if (check_label(data,data->nlabel)) 
						data->nlabel++;
					}
				else if ((data->nlabel == 0 || data->nsave > 10) 
					&& (data->jsave[data->nsave-1] == right 
						|| data->jsave[data->nsave-1] 
							== right-1))
					{
					data->xlabel[data->nlabel] 
						= data->xsave[data->nsave-1];
					data->ylabel[data->nlabel] 
						= data->ysave[data->nsave-1];
					get_azimuth_old(data,
						data->isave[data->nsave-1],
						&data->angle[data->nlabel]);
					data->justify[data->nlabel] = 0;
					if (check_label(data,data->nlabel)) 
						data->nlabel++;
					}
				}

			/* dump the contour */
			dump_contour(data,value);
			}

		/* done with contouring this level */
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	return(MB_SUCCESS);
}
/*--------------------------------------------------------------------------*/
/* 	function get_start_old finds next contour starting point.
 *	the borders are searched first and then the interior */
int get_start_old(struct swath *data, 
		int *k, int *i, int *j, int *d, int *closed)
{
	int	nn, ii, jj;

	/* search edges */
	*closed = 0;

	/* search bottom (i = 0) */
	for (jj=0;jj<data->pings[0].beams_bath-1;jj++)
		if (data->pings[0].bflag[0][jj])
			{
			*k = 0;
			*i = 0;
			*j = jj;
			*d = 0;
			return(1);
			}

	/* search top (i = npings-1) */
	for (jj=0;jj<data->pings[data->npings-1].beams_bath-1;jj++)
		if (data->pings[data->npings-1].bflag[0][jj])
			{
			*k = 0;
			*i = data->npings - 1;
			*j = jj;
			*d = 1;
			return(1);
			}

	/* search left (j = 0) */
	for (ii=0;ii<data->npings-1;ii++)
		if (data->pings[ii].bflag[1][0])
			{
			*k = 1;
			*i = ii;
			*j = 0;
			*d = 0;
			return(1);
			}

	/* search right (j = beams_bath-1) */
	for (ii=0;ii<data->npings-1;ii++)
		if (data->pings[ii].bflag[1][data->pings[ii].beams_bath-1])
			{
			*k = 1;
			*i = ii;
			*j = data->pings[ii].beams_bath - 1;
			*d = 1;
			return(1);
			}

	/* search interior */
	*closed = 1;
	for (ii=0;ii<data->npings-1;ii++)
		for (jj=0;jj<data->pings[ii].beams_bath-1;jj++)
			{
			if (data->pings[ii].bflag[0][jj])
				{
				*k = 0;
				*i = ii;
				*j = jj;
				*d = 0;
				return(1);
				}
			if (data->pings[ii].bflag[1][jj])
				{
				*k = 1;
				*i = ii;
				*j = jj;
				*d = 0;
				return(1);
				}
			}

	/* nothing found */
	return(0);
}
/*--------------------------------------------------------------------------*/
/* 	function get_next_old finds next contour component if it exists */
int get_next_old(struct swath *data, int *nk, int *ni, int *nj, int *nd,
		int k, int i, int j, int d, 
		int kbeg, int ibeg, int jbeg, int dbeg, int *closed)
{
	static int ioff[3][2][2] =
		{
		{{0,-1},{1,0}},
		{{1,-1},{0,0}},
		{{0,-1},{0,1}}
		};
	static int joff[3][2][2] =
		{
		{{0,1},{0,-1}},
		{{0,0},{1,-1}},
		{{1,0},{0,-1}}
		};
	static int koff[3][2][2] =
		{
		{{1,1},{0,0}},
		{{0,0},{1,1}},
		{{1,1},{0,0}}
		};
	static int doff[3][2][2] =
		{
		{{1,0},{0,1}},
		{{0,1},{0,1}},
		{{0,1},{1,0}}
		};
	int	ii, edge;
	int	kt[3], it[3], jt[3], dt[3], ifedge[3];
	double	xs, ys, x0, y0, x2, y2, dist0, dist2;

	/* there are three possible edges for the contour to go to */
	/* (left = 0, across = 1, right = 2) */
	/* find out which edges have unflagged crossing points */
	for (edge=0;edge<3;edge++)
		{
		kt[edge] = koff[edge][k][d];
		it[edge] = i + ioff[edge][k][d];
		jt[edge] = j + joff[edge][k][d];
		dt[edge] = doff[edge][k][d];
		if (it[edge] < 0 || it[edge] >= data->npings 
			|| jt[edge] < 0 || jt[edge] >= data->pings[i].beams_bath)
			ifedge[edge] = 0;
		else
			ifedge[edge] = 
				data->pings[it[edge]].bflag[kt[edge]][jt[edge]];
		}

	/* if the across edge exists, use it */
	if (ifedge[1])
		{
		*nk = kt[1];
		*ni = it[1];
		*nj = jt[1];
		*nd = dt[1];
		return(1);
		}

	/* else if edge 0 exists, use it */
	else if (ifedge[0])
		{
		*nk = kt[0];
		*ni = it[0];
		*nj = jt[0];
		*nd = dt[0];
		return(1);
		}

	/* else if edge 2 exists, use it */
	else if (ifedge[2])
		{
		*nk = kt[2];
		*ni = it[2];
		*nj = jt[2];
		*nd = dt[2];
		return(1);
		}

	/* if no edge is found and contour is closed and closes then */
	/* contour ends */
	else if (*closed && kbeg == k && ibeg == i && jbeg == j)
		return(0);

	/* if no edge is found and contour is closed but doesnt close then */
	/* reverse order of points and start over */
	else if (*closed)
		{
		for (ii=0;ii<data->nsave/2;ii++)
			{
			xs = data->xsave[ii];
			ys = data->ysave[ii];
			data->xsave[ii] = data->xsave[data->nsave-ii-1];
			data->ysave[ii] = data->ysave[data->nsave-ii-1];
			data->xsave[data->nsave-ii-1] = xs;
			data->ysave[data->nsave-ii-1] = ys;
			}
		*closed = 0;
		*nk = kbeg;
		*ni = ibeg;
		*nj = jbeg;
		if (dbeg) *nd = 0;
		else *nd = 1;
		data->nsave--;
		return(1);
		}

	/* else if no edge is found and contour is not closed */
	/* then contour ends */
	else
		return(0);
}
/*--------------------------------------------------------------------------*/
/* 	function get_pos_old finds position of contour crossing point */
int get_pos_old(struct swath *data, double eps, double *x, double *y,
		int k, int i, int j, double value)
{
	int	i2, j2;
	double	x1, y1, x2, y2, v1, v2, factor;

	/* get grid positions and values */
	x1 = data->pings[i].bathlon[j];
	y1 = data->pings[i].bathlat[j];
	v1 = data->pings[i].bath[j];
	if (k == 0)
		{
		x2 = data->pings[i].bathlon[j+1];
		y2 = data->pings[i].bathlat[j+1];
		v2 = data->pings[i].bath[j+1];
		}
	else
		{
		x2 = data->pings[i+1].bathlon[j];
		y2 = data->pings[i+1].bathlat[j];
		v2 = data->pings[i+1].bath[j];
		}

	/* interpolate the position */
	if (fabs(v2 - v1) > eps)
		factor = (value - v1)/(v2 - v1);
	else
		factor = 0.5;
	if (factor < 0.0) factor = 0.0;
	if (factor > 1.0) factor = 1.0;
	*x = factor*(x2 - x1) + x1;
	*y = factor*(y2 - y1) + y1;

	return(MB_YES);
}
/*--------------------------------------------------------------------------*/
/* 	function get_hand_old finds handedness of contour */
int get_hand_old(struct swath *data, int *hand, 
		int k, int i, int j, int d)
{
	if (k == 0 && d == 0)
		{
		if (data->pings[i].bath[j] > data->pings[i].bath[j+1])
			*hand = 1;
		else
			*hand = -1;
		}
	else if (k == 0 && d == 1)
		{
		if (data->pings[i].bath[j] > data->pings[i].bath[j+1])
			*hand = -1;
		else
			*hand = 1;
		}
	else if (k == 1 && d == 0)
		{
		if (data->pings[i].bath[j] > data->pings[i+1].bath[j])
			*hand = -1;
		else
			*hand = 1;
		}
	else if (k == 1 && d == 1)
		{
		if (data->pings[i].bath[j] > data->pings[i+1].bath[j])
			*hand = 1;
		else
			*hand = -1;
		}
	return(MB_YES);
}

/*--------------------------------------------------------------------------*/
/* 	function get_azimuth_old gets azimuth across shiptrack at ping iping */
int get_azimuth_old(struct swath *data, int iping, double *angle)
{
	double	heading;

	*angle = -data->pings[iping].heading;
	if (*angle > 180.0)
		*angle = *angle - 360.0;
	if (*angle < -180.0)
		*angle = *angle + 360.0;

	return(MB_YES);
}
/*--------------------------------------------------------------------------*/
