/*--------------------------------------------------------------------
 *    The MB-system:	mb_truecont.c	4/21/94
 *    $Id: mb_truecont.c,v 4.1 1994-07-29 19:04:31 caress Exp $
 *
 *    Copyright (c) 1994 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
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
#include <strings.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_contour.h"

/* global array dimensions etc */
#define PI 3.1415926
#define DTR PI/180.
#define RTD 180./PI
#define IUP 3
#define IDN 2
#define IOR -3
#define EPS 0.0001
#define	min(A, B)	((A) < (B) ? (A) : (B))
#define	max(A, B)	((A) > (B) ? (A) : (B))

/*--------------------------------------------------------------------------*/
/* 	function mb_contour_init initializes the memory required to
	contour multibeam bathymetry data. */
int mb_contour_init(verbose,data,npings_max,beams_bath,
			plot_contours,plot_triangles,plot_track,
			contour_int,color_int,tick_int,label_int,
			tick_len,label_hgt,ncolor,
			nlevel,level_list,label_list,tick_list,
			time_tick_int,time_annot_int,
			date_annot_int,time_tick_len,
			error)
int	verbose;
struct swath **data;
int	npings_max;
int	beams_bath;
int	plot_contours;
int	plot_triangles;
int	plot_track;
double	contour_int;
double	color_int;
double	tick_int;
double	label_int;
double	tick_len;
double	label_hgt;
int	ncolor;
int	nlevel;
double	*level_list;
int	*label_list;
int	*tick_list;
double	time_tick_int;
double	time_annot_int;
double	date_annot_int;
double	time_tick_len;
int	*error;
{
  	static char rcs_id[]="$Id: mb_truecont.c,v 4.1 1994-07-29 19:04:31 caress Exp $";
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
		fprintf(stderr,"dbg2       plot contours:    %d\n",
			plot_contours);
		fprintf(stderr,"dbg2       plot triangles:   %d\n",
			plot_triangles);
		fprintf(stderr,"dbg2       plot track:       %d\n",
			plot_track);
		fprintf(stderr,"dbg2       contour interval: %f\n",contour_int);		fprintf(stderr,"dbg2       color interval:   %f\n",color_int);
		fprintf(stderr,"dbg2       tick interval:    %f\n",tick_int);
		fprintf(stderr,"dbg2       label interval:   %f\n",label_int);
		fprintf(stderr,"dbg2       tick length:      %f\n",tick_len);
		fprintf(stderr,"dbg2       label height:     %f\n",label_hgt);
		fprintf(stderr,"dbg2       number of colors: %d\n",ncolor);
		fprintf(stderr,"dbg2       number of levels: %d\n",nlevel);
		for (i=0;i<nlevel;i++)
			fprintf(stderr,"dbg2       level %d: %f %d %d\n",
				i,level_list[i],label_list[i],tick_list[i]);
		fprintf(stderr,"time tick interval:          %f\n",
			time_tick_int);
		fprintf(stderr,"time interval:               %f\n",
			time_annot_int);
		fprintf(stderr,"date interval:               %f\n",
			date_annot_int);
		fprintf(stderr,"time tick length:            %f\n\n",
			time_tick_len);
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
		status = mb_malloc(verbose,beams_bath*sizeof(double),
				&(ping->bath),error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
				&(ping->bathlon),error);
		status = mb_malloc(verbose,beams_bath*sizeof(double),
				&(ping->bathlat),error);
		}

	/* set controls on what gets plotted */
	dataptr->plot_contours = plot_contours;
	dataptr->plot_triangles = plot_triangles;
	dataptr->plot_track = plot_track;

	/* set variables and allocate memory for contour controls */
	dataptr->contour_int = contour_int;
	dataptr->color_int = color_int;
	dataptr->tick_int = tick_int;
	dataptr->label_int = label_int;
	dataptr->tick_len = tick_len;
	dataptr->label_hgt = label_hgt;
	dataptr->ncolor = ncolor;
	dataptr->nlevel = nlevel;
	dataptr->nlevelset = MB_NO;
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
	else
		{
		dataptr->level_list = NULL;
		dataptr->label_list = NULL;
		dataptr->tick_list = NULL;
		dataptr->color_list = NULL;
		}

	/* set variables and allocate memory for track controls */
	dataptr->time_tick_int = time_tick_int;
	dataptr->time_annot_int = time_annot_int;
	dataptr->date_annot_int = date_annot_int;
	dataptr->time_tick_len = time_tick_len;

	/* set variables and allocate memory for triangle network */
	dataptr->npts = 0;
	status = mb_malloc(verbose,npings_max*beams_bath*sizeof(double),
			&(dataptr->x),error);
	status = mb_malloc(verbose,npings_max*beams_bath*sizeof(double),
			&(dataptr->y),error);
	status = mb_malloc(verbose,npings_max*beams_bath*sizeof(double),
			&(dataptr->z),error);
	status = mb_malloc(verbose,npings_max*beams_bath*sizeof(int),
			&(dataptr->edge),error);
	status = mb_malloc(verbose,npings_max*beams_bath*sizeof(int),
			&(dataptr->pingid),error);
	dataptr->ntri = 0;
	ntri_max = 2*npings_max*beams_bath + 1;
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

	/* allocate memory for contour positions */
	dataptr->nsave = 0;
	status = mb_malloc(verbose,(4*ntri_max+1)*sizeof(double),
				&(dataptr->xsave),error);
	status = mb_malloc(verbose,(4*ntri_max+1)*sizeof(double),
				&(dataptr->ysave),error);

	/* allocate memory for contour labels */
	dataptr->nlabel = 0;
	status = mb_malloc(verbose,(2*npings_max)*sizeof(double),
				&(dataptr->xlabel),error);
	status = mb_malloc(verbose,(2*npings_max)*sizeof(double),
				&(dataptr->ylabel),error);
	status = mb_malloc(verbose,(2*npings_max)*sizeof(double),
				&(dataptr->angle),error);
	status = mb_malloc(verbose,(2*npings_max)*sizeof(int),
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

	return;
}

/*--------------------------------------------------------------------------*/
/* 	function mb_contour_deall deallocates the memory required to
	contour multibeam bathymetry data. */
int mb_contour_deall(verbose,data,error)
int	verbose;
struct swath *data;
int	*error;
{
  	static char rcs_id[]="$Id: mb_truecont.c,v 4.1 1994-07-29 19:04:31 caress Exp $";
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
		status = mb_free(verbose,ping->bath,error);
		status = mb_free(verbose,ping->bathlon,error);
		status = mb_free(verbose,ping->bathlat,error);
		}
	status = mb_free(verbose,data->pings,error);

	/* deallocate memory for contour controls */
	if (data->nlevel > 0)
		{
		status = mb_free(verbose,data->level_list,error);
		status = mb_free(verbose,data->label_list,error);
		status = mb_free(verbose,data->tick_list,error);
		status = mb_free(verbose,data->color_list,error);
		}

	/* deallocate memory for triangle network */
	status = mb_free(verbose,data->x,error);
	status = mb_free(verbose,data->y,error);
	status = mb_free(verbose,data->z,error);
	status = mb_free(verbose,data->edge,error);
	status = mb_free(verbose,data->pingid,error);
	for (i=0;i<3;i++)
		{
		status = mb_free(verbose,data->iv[i],error);
		status = mb_free(verbose,data->ct[i],error);
		status = mb_free(verbose,data->cs[i],error);
		status = mb_free(verbose,data->ed[i],error);
		status = mb_free(verbose,data->flag[i],error);
		}
	status = mb_free(verbose,data->v1,error);
	status = mb_free(verbose,data->v2,error);
	status = mb_free(verbose,data->v3,error);
	status = mb_free(verbose,data->istack,error);
	status = mb_free(verbose,data->kv1,error);
	status = mb_free(verbose,data->kv2,error);

	/* deallocate memory for contour positions */
	status = mb_free(verbose,data->xsave,error);
	status = mb_free(verbose,data->ysave,error);

	/* deallocate memory for contour labels */
	status = mb_free(verbose,data->xlabel,error);
	status = mb_free(verbose,data->ylabel,error);
	status = mb_free(verbose,data->angle,error);
	status = mb_free(verbose,data->justify,error);

	/* deallocate memory for swath structure */
	status = mb_free(verbose,data,error);

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

	return;
}

/*--------------------------------------------------------------------------*/
/* 	function mb_contour contours multibeam data. */
int mb_contour(verbose,data,error)
int	verbose;
struct swath *data;
int	*error;
{
  	static char rcs_id[]="$Id: mb_truecont.c,v 4.1 1994-07-29 19:04:31 caress Exp $";
	char	*function_name = "mb_contour";
	int	status = MB_SUCCESS;
	struct ping *ping;
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
	int	i, j, k, jj;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:          %d\n",verbose);
		fprintf(stderr,"dbg2       data:             %d\n",data);
		}

	/* put good bathymetry data into x arrays */
	data->npts = 0;
	extreme_start = MB_NO;
	for (i=0;i<data->npings;i++)
	  {
	  ping = &data->pings[i];

	  /* find edges of ping */
	  left = data->beams_bath/2;
	  right = left;
	  for (j=0;j<data->beams_bath;j++)
		{
		if (ping->bath[j] > 0 && j < left) left = j;
		if (ping->bath[j] > 0 && j > right) right = j;
		}

	  /* deal with data */
	  for (j=0;j<data->beams_bath;j++)
		{
		if (extreme_start == MB_NO && ping->bath[j] > 0)
			{
			bathmin = ping->bath[j];
			bathmax = ping->bath[j];
			xmin = ping->bathlon[j];
			xmax = xmin;
			ymin = ping->bathlat[j];
			ymax = ymin;
			extreme_start = MB_YES;
			}
		if (ping->bath[j] > 0)
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
			bathmin = min(bathmin, data->z[data->npts]);
			bathmax = max(bathmax, data->z[data->npts]);
			xmin = min(xmin, data->x[data->npts]);
			xmax = max(xmax, data->x[data->npts]);
			ymin = min(ymin, data->y[data->npts]);
			ymax = max(ymax, data->y[data->npts]);
			data->npts++;
			}
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
			mb_free(verbose,data->level_list,error);
			mb_free(verbose,data->color_list,error);
			mb_free(verbose,data->label_list,error);
			mb_free(verbose,data->tick_list,error);
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
		while (get_start(data,&itri,&iside1,&iside2,&closed))
			{
			/* if not closed remove flags */
			data->flag[iside1][itri] = -1;
			data->flag[iside2][itri] = -1;

			/* get position of start of contour */
			get_pos(data,itri,iside1,value,
				&data->xsave[data->nsave],
				&data->ysave[data->nsave]);
			data->nsave++;
			get_pos(data,itri,iside2,value,
				&data->xsave[data->nsave],
				&data->ysave[data->nsave]);
			data->nsave++;
			itristart = itri;
			isidestart = iside1;
			itriend = itri;
			isideend = iside2;

			/* look for next segment */
			while(get_next(data,&itri,&iside1,&iside2,&closed,
				&itristart,&isidestart))
				{
				/* get position */
				get_pos(data,itri,iside2,value,&x,&y);

				/* deal with tick as needed */
				if (tick)
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
					}
				else
					{
					data->xsave[data->nsave] = x;
					data->ysave[data->nsave] = y;
					data->flag[iside1][itri] = -1;
					data->flag[iside2][itri] = -1;
					data->nsave++;
					}

				/* set latest point */
				itriend = itri;
				isideend = iside2;
				}

			/* set label if needed */
			if (!closed && data->ed[isidestart][itristart] != 0)
				{
				data->xlabel[data->nlabel] = data->xsave[0];
				data->ylabel[data->nlabel] = data->ysave[0];
				get_azimuth(data,itristart,isidestart,
					&data->angle[data->nlabel]);
				if (data->ed[isidestart][itristart] == -1)
					data->justify[data->nlabel] = 1;
				else
					data->justify[data->nlabel] = 0;
				if (check_label(data,data->nlabel))
					data->nlabel++;
				}
			if (!closed && data->ed[isideend][itriend] != 0)
				{
				data->xlabel[data->nlabel] 
					= data->xsave[data->nsave-1];
				data->ylabel[data->nlabel] 
					= data->ysave[data->nsave-1];
				get_azimuth(data,itriend,isideend,
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
/* 	function get_start finds next contour starting point. */
int get_start(data,itri,iside1,iside2,closed)
struct swath *data;
int	*itri;
int	*iside1;
int	*iside2;
int	*closed;
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
			fprintf(stderr,"no flagged side in get_start???\n");
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
/* 	function get_next finds next contour component if it exists */
int get_next(data,itri,iside1,iside2,closed,itristart,isidestart)
struct swath *data;
int	*itri;
int	*iside1;
int	*iside2;
int	*closed;
int	*itristart;
int	*isidestart;
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
			fprintf(stderr,"no flagged side in get_next???\n");
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
/* 	function get_pos finds position of contour crossing point */
int get_pos(data,itri,iside,value,x,y)
struct swath *data;
int	itri;
int	iside;
double	value;
double	*x;
double	*y;
{
	double	factor;
	int	v1, v2, pt1, pt2;

	v1 = iside;
	v2 = iside + 1;
	if (v2 == 3) v2 = 0;
	pt1 = data->iv[v1][itri];
	pt2 = data->iv[v2][itri];

	if (data->z[pt2] == data->z[pt1])
		{
		fprintf(stderr,"\nOH NO! Points equal!\n");
		fprintf(stderr,"itri:%d iside:%d value:%f\n",itri,iside,value);
		fprintf(stderr,"v1:%d v2:%d  pt1:%d pt2:%d  z1:%f z2:%f\n",
			v1,v2,pt1,pt2,data->z[pt1],data->z[pt2]);
		*x = data->x[pt1];
		*y = data->y[pt1];
		exit(0);
		}

	factor = (value - data->z[pt1])/(data->z[pt2] - data->z[pt1]);
	*x = data->x[pt1] + factor*(data->x[pt2] - data->x[pt1]);
	*y = data->y[pt1] + factor*(data->y[pt2] - data->y[pt1]);

	return(MB_YES);
	
}
/*--------------------------------------------------------------------------*/
/* 	function get_azimuth gets azimuth across track for a label */
int get_azimuth(data,itri,iside,angle)
struct swath *data;
int	itri;
int	iside;
double	*angle;
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
int check_label(data,nlab)
struct swath *data;
int	nlab;
{
#define	MAXHIS 30
	static double xlabel_his[MAXHIS], ylabel_his[MAXHIS];
	static int nlabel_his = 0;
	double	rad_label_his;
	int	good, ilab, i;
	double	dx, dy, rr;

	good = 1;
	ilab = 0;
	rad_label_his = data->label_hgt;
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
int dump_contour(data,value)
struct swath *data;
double	value;
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
