/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_nav.c	10/28/2003
 *    $Id: mbview_nav.c,v 1.2 2003-11-25 01:43:19 caress Exp $
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
 * Date:	October 28, 2003
 *
 * $Log: not supported by cvs2svn $
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

static char rcs_id[]="$Id: mbview_nav.c,v 1.2 2003-11-25 01:43:19 caress Exp $";

/*------------------------------------------------------------------------------*/
int mbview_getnavcount(int verbose, int instance,
			int *nnav,
			int *error)
{
	/* local variables */
	char	*function_name = "mbview_getnavcount";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

fprintf(stderr,"Called mbview_getnavcount:%d\n",instance);

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr,"dbg2       instance:                  %d\n", instance);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* get number of navs */
	*nnav = data->nnav;
		
	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nnav:                      %d\n", *nnav);
		fprintf(stderr,"dbg2       error:                     %d\n", *error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n", status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_getnavpointcount(int verbose, int instance,
			int	nav,
			int	*npoint,
			int	*nintpoint,
			int *error)
{
	/* local variables */
	char	*function_name = "mbview_getnavpointcount";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	i;

fprintf(stderr,"Called mbview_getnavpointcount:%d\n",instance);

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr,"dbg2       instance:                  %d\n", instance);
		fprintf(stderr,"dbg2       nav:                     %d\n", nav);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* get number of points in specified nav */
	*npoint = 0;
	*nintpoint = 0;
	if (nav >= 0 && nav < data->nnav)
		{
		*npoint = data->navs[nav].npoints;
		for (i=0;i<*npoint-1;i++)
			{
			if (data->navs[nav].segments[i].nls > 2)
				*nintpoint += data->navs[nav].segments[i].nls - 2;
			}
		}
		
	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       npoint:                    %d\n", *npoint);
		fprintf(stderr,"dbg2       nintpoint:                 %d\n", *nintpoint);
		fprintf(stderr,"dbg2       error:                     %d\n", *error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n", status);
		}

	/* return */
	return(status);

}

/*------------------------------------------------------------------------------*/
int mbview_allocnavarrays(int verbose, 
			int	npointtotal,
			double	**time_d,
			double	**navlon,
			double	**navlat,
			double	**navz,
			double	**heading,
			double	**speed,
			double	**navportlon,
			double	**navportlat,
			double	**navstbdlon,
			double	**navstbdlat,
			int	**cdp,
			int	**shot,
			int 	*error)
{
	/* local variables */
	char	*function_name = "mbview_allocnavarrays";
	int	status = MB_SUCCESS;

fprintf(stderr,"Called mbview_allocnavarrays: npointtotal:%d\n", npointtotal);

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr,"dbg2       npointtotal:               %d\n", npointtotal);
		fprintf(stderr,"dbg2       time_d:                    %d\n", *time_d);
		fprintf(stderr,"dbg2       navlon:                    %d\n", *navlon);
		fprintf(stderr,"dbg2       navlat:                    %d\n", *navlat);
		fprintf(stderr,"dbg2       navz:                      %d\n", *navz);
		fprintf(stderr,"dbg2       heading:                   %f\n", *heading);
		fprintf(stderr,"dbg2       speed:                     %f\n", *speed);
		if (navportlon != NULL)
		fprintf(stderr,"dbg2       navportlon:                %d\n", *navportlon);
		if (navportlat != NULL)
		fprintf(stderr,"dbg2       navportlat:                %d\n", *navportlat);
		if (navstbdlon != NULL)
		fprintf(stderr,"dbg2       navstbdlon:                %d\n", *navstbdlon);
		if (navstbdlat != NULL)
		fprintf(stderr,"dbg2       navstbdlat:                %d\n", *navstbdlat);
		if (cdp != NULL)
		fprintf(stderr,"dbg2       cdp:                       %d\n", *cdp);
		if (shot != NULL)
		fprintf(stderr,"dbg2       shot:                      %d\n", *shot);
		}

	/* allocate the arrays using mb_realloc */
	status = mb_realloc(verbose,npointtotal*sizeof(double),time_d,error);
	if (status == MB_SUCCESS)
		status = mb_realloc(verbose,npointtotal*sizeof(double),navlon,error);
	if (status == MB_SUCCESS)
		status = mb_realloc(verbose,npointtotal*sizeof(double),navlat,error);
	if (status == MB_SUCCESS)
		status = mb_realloc(verbose,npointtotal*sizeof(double),navz,error);
	if (status == MB_SUCCESS)
		status = mb_realloc(verbose,npointtotal*sizeof(double),heading,error);
	if (status == MB_SUCCESS)
		status = mb_realloc(verbose,npointtotal*sizeof(double),speed,error);
	if (status == MB_SUCCESS && navportlon != NULL)
		status = mb_realloc(verbose,npointtotal*sizeof(double),navportlon,error);
	if (status == MB_SUCCESS && navportlat != NULL)
		status = mb_realloc(verbose,npointtotal*sizeof(double),navportlat,error);
	if (status == MB_SUCCESS && navstbdlon != NULL)
		status = mb_realloc(verbose,npointtotal*sizeof(double),navstbdlon,error);
	if (status == MB_SUCCESS && navstbdlat != NULL)
		status = mb_realloc(verbose,npointtotal*sizeof(double),navstbdlat,error);
	if (status == MB_SUCCESS && cdp != NULL)
		status = mb_realloc(verbose,npointtotal*sizeof(int),cdp,error);
	if (status == MB_SUCCESS && shot != NULL)
		status = mb_realloc(verbose,npointtotal*sizeof(int),shot,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       time_d:                    %d\n", *time_d);
		fprintf(stderr,"dbg2       navlon:                    %d\n", *navlon);
		fprintf(stderr,"dbg2       navlat:                    %d\n", *navlat);
		fprintf(stderr,"dbg2       navz:                      %d\n", *navz);
		fprintf(stderr,"dbg2       heading:                   %d\n", *heading);
		fprintf(stderr,"dbg2       speed:                     %d\n", *speed);
		if (navportlon != NULL)
		fprintf(stderr,"dbg2       navportlon:                %d\n", *navportlon);
		if (navportlat != NULL)
		fprintf(stderr,"dbg2       navportlat:                %d\n", *navportlat);
		if (navstbdlon != NULL)
		fprintf(stderr,"dbg2       navstbdlon:                %d\n", *navstbdlon);
		if (navstbdlat != NULL)
		fprintf(stderr,"dbg2       navstbdlat:                %d\n", *navstbdlat);
		if (cdp != NULL)
		fprintf(stderr,"dbg2       cdp:                       %d\n", *cdp);
		if (shot != NULL)
		fprintf(stderr,"dbg2       shot:                      %d\n", *shot);
		fprintf(stderr,"dbg2       error:                     %d\n", *error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n", status);
		}

	/* return */
	return(status);
}


/*------------------------------------------------------------------------------*/
int mbview_freenavarrays(int verbose,
			double	**time_d,
			double	**navlon,
			double	**navlat,
			double	**navz,
			double	**heading,
			double	**speed,
			double	**navportlon,
			double	**navportlat,
			double	**navstbdlon,
			double	**navstbdlat,
			int	**cdp,
			int	**shot,
			int *error)
{
	/* local variables */
	char	*function_name = "mbview_freenavarrays";
	int	status = MB_SUCCESS;

fprintf(stderr,"Called mbview_freenavarrays:\n");

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr,"dbg2       time_d:                    %d\n", *time_d);
		fprintf(stderr,"dbg2       navlon:                    %d\n", *navlon);
		fprintf(stderr,"dbg2       navlat:                    %d\n", *navlat);
		fprintf(stderr,"dbg2       navz:                      %d\n", *navz);
		fprintf(stderr,"dbg2       heading:                   %d\n", *heading);
		fprintf(stderr,"dbg2       speed:                     %d\n", *speed);
		if (navportlon != NULL)
		fprintf(stderr,"dbg2       navportlon:                %d\n", *navportlon);
		if (navportlat != NULL)
		fprintf(stderr,"dbg2       navportlat:                %d\n", *navportlat);
		if (navstbdlon != NULL)
		fprintf(stderr,"dbg2       navstbdlon:                %d\n", *navstbdlon);
		if (navstbdlat != NULL)
		fprintf(stderr,"dbg2       navstbdlat:                %d\n", *navstbdlat);
		if (cdp != NULL)
		fprintf(stderr,"dbg2       cdp:                       %d\n", *cdp);
		if (shot != NULL)
		fprintf(stderr,"dbg2       shot:                      %d\n", *shot);
		}

	/* free the arrays using mb_free */
	status = mb_free(verbose,time_d,error);
	status = mb_free(verbose,navlon,error);
	status = mb_free(verbose,navlat,error);
	status = mb_free(verbose,navz,error);
	status = mb_free(verbose,heading,error);
	status = mb_free(verbose,speed,error);
	if (navportlon != NULL)
		status = mb_free(verbose,navportlon,error);
	if (navportlat != NULL)
		status = mb_free(verbose,navportlat,error);
	if (navstbdlon != NULL)
		status = mb_free(verbose,navstbdlon,error);
	if (navstbdlat != NULL)
		status = mb_free(verbose,navstbdlat,error);
	if (cdp != NULL)
		status = mb_free(verbose,cdp,error);
	if (shot != NULL)
		status = mb_free(verbose,shot,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       time_d:                    %d\n", *time_d);
		fprintf(stderr,"dbg2       navlon:                    %d\n", *navlon);
		fprintf(stderr,"dbg2       navlat:                    %d\n", *navlat);
		fprintf(stderr,"dbg2       navz:                      %d\n", *navz);
		fprintf(stderr,"dbg2       heading:                   %d\n", *heading);
		fprintf(stderr,"dbg2       speed:                     %d\n", *speed);
		if (navportlon != NULL)
		fprintf(stderr,"dbg2       navportlon:                %d\n", *navportlon);
		if (navportlat != NULL)
		fprintf(stderr,"dbg2       navportlat:                %d\n", *navportlat);
		if (navstbdlon != NULL)
		fprintf(stderr,"dbg2       navstbdlon:                %d\n", *navstbdlon);
		if (navstbdlat != NULL)
		fprintf(stderr,"dbg2       navstbdlat:                %d\n", *navstbdlat);
		if (cdp != NULL)
		fprintf(stderr,"dbg2       cdp:                       %d\n", *cdp);
		if (shot != NULL)
		fprintf(stderr,"dbg2       shot:                      %d\n", *shot);
		fprintf(stderr,"dbg2       error:                     %d\n", *error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n", status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_addnav(int verbose, int instance,
			int	npoint,
			double	*time_d,
			double	*navlon,
			double	*navlat,
			double	*navz,
			double	*heading,
			double	*speed,
			double	*navportlon,
			double	*navportlat,
			double	*navstbdlon,
			double	*navstbdlat,
			int	*cdp,
			int	*shot,
			int	navcolor,
			int	navsize,
			mb_path	navname,
			int	navswathbounds,
			int	navshot,
			int	navcdp,
			int *error)
{
	/* local variables */
	char	*function_name = "mbview_addnav";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	nfound;
	int	nadded;
	int	inav;
	double	zdata;
	int	i, j, ii, jj, iii, jjj, kkk;

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr,"dbg2       instance:                  %d\n", instance);
		fprintf(stderr,"dbg2       npoint:                    %d\n", npoint);
		fprintf(stderr,"dbg2       navlon:                    %d\n", navlon);
		fprintf(stderr,"dbg2       navlat:                    %d\n", navlat);
		fprintf(stderr,"dbg2       navz:                      %d\n", navz);
		for (i=0;i<npoint;i++)
			{
			fprintf(stderr,"dbg2       point:%d time_d:%f lon:%f lat:%f z:%f heading:%f zpeed:%f\n", 
					i, time_d[i], navlon[i], navlat[i], navz[i],
					heading[i], speed[i]);
			}
		if (navswathbounds == MB_YES)
		for (i=0;i<npoint;i++)
			{
			fprintf(stderr,"dbg2       point:%d port: lon:%f lat:%f  stbd: lon:%f lat:%f\n", 
					i, navportlon[i], navportlat[i], navstbdlon[i], navstbdlat[i]);
			}
		if (navcdp == MB_YES)
		for (i=0;i<npoint;i++)
			{
			fprintf(stderr,"dbg2       point:%d cdp:%d\n", i, cdp[i]);
			}
		if (navshot == MB_YES)
		for (i=0;i<npoint;i++)
			{
			fprintf(stderr,"dbg2       point:%d shot:%d\n", i, shot[i]);
			}
		fprintf(stderr,"dbg2       navcolor:                  %d\n", navcolor);
		fprintf(stderr,"dbg2       navsize:                   %d\n", navsize);
		fprintf(stderr,"dbg2       navname:                   %s\n", navname);
		fprintf(stderr,"dbg2       navswathbounds:            %d\n", navswathbounds);
		fprintf(stderr,"dbg2       navshot:                   %d\n", navshot);
		fprintf(stderr,"dbg2       navcdp:                    %d\n", navcdp);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* make sure no nav is selected */
	data->nav_selected[0] = MBV_SELECT_NONE;
	data->nav_selected[1] = MBV_SELECT_NONE;
	
	/* set nav id so that new nav is created */
	inav = data->nnav;
	
	/* allocate memory for a new nav if required */
	if (data->nnav_alloc < data->nnav + 1)
		{
		data->nnav_alloc = data->nnav + 1;
		status = mb_realloc(mbv_verbose, 
			    	data->nnav_alloc * sizeof(struct mbview_nav_struct),
			    	&(data->navs), error);
		if (status == MB_FAILURE)
			{
			data->nnav_alloc = 0;
			}
		else
			{
			for (i=data->nnav;i<data->nnav_alloc;i++)
				{
				data->navs[i].color = MBV_COLOR_RED;
				data->navs[i].size = 4;
				data->navs[i].name[0] = '\0';
				data->navs[i].swathbounds = MB_NO;
				data->navs[i].shot = MB_NO;
				data->navs[i].cdp = MB_NO;
				data->navs[i].npoints = 0;
				data->navs[i].npoints_alloc = 0;
				data->navs[i].navpts = NULL;
				data->navs[i].segments = NULL;
				}
			}
		}
		
	/* allocate memory to for nav arrays */
	if (data->navs[inav].npoints_alloc < npoint)
		{
		data->navs[inav].npoints_alloc = npoint;
		status = mb_realloc(mbv_verbose, 
			    	data->navs[inav].npoints_alloc * sizeof(struct mbview_nav_struct),
			    	&(data->navs[inav].navpts), error);
		status = mb_realloc(mbv_verbose, 
			    	data->navs[inav].npoints_alloc * sizeof(struct mbview_linesegment_struct),
			    	&(data->navs[inav].segments), error);
		for (j=0;j<data->navs[inav].npoints_alloc-1;j++)
			{
			data->navs[inav].segments[j].nls = 0;
			data->navs[inav].segments[j].nls_alloc = 0;
			data->navs[inav].segments[j].lspoints = NULL;
			data->navs[inav].segments[j].endpoints[0] = &(data->navs[inav].navpts[j].pointcntr);
			data->navs[inav].segments[j].endpoints[1] = &(data->navs[inav].navpts[j+1].pointcntr);
			}
		}
		
	/* add the new nav */
	if (status == MB_SUCCESS)
		{
		/* set nnav */
		data->nnav++;

		/* set color size and name for new nav */
		data->navs[inav].color = navcolor;
		data->navs[inav].size = navsize;
		strcpy(data->navs[inav].name,navname);
		data->navs[inav].swathbounds = navswathbounds;
		data->navs[inav].shot = navshot;
		data->navs[inav].cdp = navcdp;

		/* loop over the points in the new nav */
		data->navs[inav].npoints = npoint;
		for (i=0;i<npoint;i++)
			{
			/* set status values */
			data->navs[inav].navpts[i].draped = MB_NO;
			data->navs[inav].navpts[i].selected = MB_NO;
			
			/* set time and shot info */
			data->navs[inav].navpts[i].time_d = time_d[i];
			data->navs[inav].navpts[i].heading = heading[i];
			data->navs[inav].navpts[i].speed = speed[i];
			if (data->navs[inav].shot == MB_YES)
				data->navs[inav].navpts[i].shot = shot[i];
			if (data->navs[inav].cdp == MB_YES)
				data->navs[inav].navpts[i].cdp = cdp[i];
			
			/* get nav positions in grid and display coordinates */
			data->navs[inav].navpts[i].point.xlon = navlon[i];
			data->navs[inav].navpts[i].point.ylat = navlat[i];
			status = mbview_projectfromlonlat(instance,
					data->navs[inav].navpts[i].point.xlon, 
					data->navs[inav].navpts[i].point.ylat, 
					&data->navs[inav].navpts[i].point.xgrid, 
					&data->navs[inav].navpts[i].point.ygrid,
					&data->navs[inav].navpts[i].point.xdisplay, 
					&data->navs[inav].navpts[i].point.ydisplay);
			data->navs[inav].navpts[i].point.zdata = navz[i];
			data->navs[inav].navpts[i].point.zdisplay 
				= view->zscale * (navz[i] - view->zorigin);

			/* get center on-bottom nav positions in grid and display coordinates */
			data->navs[inav].navpts[i].pointcntr.xlon = navlon[i];
			data->navs[inav].navpts[i].pointcntr.ylat = navlat[i];
			status = mbview_projectfromlonlat(instance,
					data->navs[inav].navpts[i].pointcntr.xlon, 
					data->navs[inav].navpts[i].pointcntr.ylat, 
					&data->navs[inav].navpts[i].pointcntr.xgrid, 
					&data->navs[inav].navpts[i].pointcntr.ygrid,
					&data->navs[inav].navpts[i].pointcntr.xdisplay, 
					&data->navs[inav].navpts[i].pointcntr.ydisplay);

			/* get topo from primary grid */
			nfound = 0;
			zdata = 0.0;
			ii = (int)((data->navs[inav].navpts[i].pointcntr.xgrid 
					- data->primary_xmin) / data->primary_dx);
			jj = (int)((data->navs[inav].navpts[i].pointcntr.ygrid 
					- data->primary_ymin) / data->primary_dy);
			if (ii >= 0 && ii < data->primary_nx - 1
			    && jj >= 0 && jj < data->primary_ny - 1)
				{
				for (iii=ii;iii<=ii+1;iii++)
				for (jjj=jj;jjj<=jj+1;jjj++)
				    {
				    kkk = iii * data->primary_ny + jjj;
				    if (data->primary_data[kkk] != data->primary_nodatavalue)
					{
					nfound++;
					zdata += data->primary_data[kkk];
					}
				    }
				}
			if (nfound > 0)
				{
				zdata /= (double)nfound;
				data->navs[inav].navpts[i].draped = MB_NO;
				}
			else
				zdata = 0.0;
			data->navs[inav].navpts[i].pointcntr.zdata = zdata;
			data->navs[inav].navpts[i].pointcntr.zdisplay 
				= view->zscale * (data->navs[inav].navpts[i].pointcntr.zdata - view->zorigin);

			/* get port swathbound nav positions in grid and display coordinates */
			data->navs[inav].navpts[i].pointport.xlon = navportlon[i];
			data->navs[inav].navpts[i].pointport.ylat = navportlat[i];
			status = mbview_projectfromlonlat(instance,
					data->navs[inav].navpts[i].pointport.xlon, 
					data->navs[inav].navpts[i].pointport.ylat, 
					&data->navs[inav].navpts[i].pointport.xgrid, 
					&data->navs[inav].navpts[i].pointport.ygrid,
					&data->navs[inav].navpts[i].pointport.xdisplay, 
					&data->navs[inav].navpts[i].pointport.ydisplay);

			/* get topo from primary grid */
			nfound = 0;
			zdata = 0.0;
			ii = (int)((data->navs[inav].navpts[i].pointport.xgrid 
					- data->primary_xmin) / data->primary_dx);
			jj = (int)((data->navs[inav].navpts[i].pointport.ygrid 
					- data->primary_ymin) / data->primary_dy);
			if (ii >= 0 && ii < data->primary_nx - 1
			    && jj >= 0 && jj < data->primary_ny - 1)
				{
				for (iii=ii;iii<=ii+1;iii++)
				for (jjj=jj;jjj<=jj+1;jjj++)
				    {
				    kkk = iii * data->primary_ny + jjj;
				    if (data->primary_data[kkk] != data->primary_nodatavalue)
					{
					nfound++;
					zdata += data->primary_data[kkk];
					}
				    }
				}
			if (nfound > 0)
				zdata /= (double)nfound;
			else
				zdata = data->navs[inav].navpts[i].pointcntr.zdata;
			data->navs[inav].navpts[i].pointport.zdata = zdata;
			data->navs[inav].navpts[i].pointport.zdisplay 
				= view->zscale * (data->navs[inav].navpts[i].pointport.zdata - view->zorigin);

			/* get starboard swathbound nav positions in grid and display coordinates */
			data->navs[inav].navpts[i].pointstbd.xlon = navstbdlon[i];
			data->navs[inav].navpts[i].pointstbd.ylat = navstbdlat[i];
			status = mbview_projectfromlonlat(instance,
					data->navs[inav].navpts[i].pointstbd.xlon, 
					data->navs[inav].navpts[i].pointstbd.ylat, 
					&data->navs[inav].navpts[i].pointstbd.xgrid, 
					&data->navs[inav].navpts[i].pointstbd.ygrid,
					&data->navs[inav].navpts[i].pointstbd.xdisplay, 
					&data->navs[inav].navpts[i].pointstbd.ydisplay);

			/* get topo from primary grid */
			nfound = 0;
			zdata = 0.0;
			ii = (int)((data->navs[inav].navpts[i].pointstbd.xgrid 
					- data->primary_xmin) / data->primary_dx);
			jj = (int)((data->navs[inav].navpts[i].pointstbd.ygrid 
					- data->primary_ymin) / data->primary_dy);
			if (ii >= 0 && ii < data->primary_nx - 1
			    && jj >= 0 && jj < data->primary_ny - 1)
				{
				for (iii=ii;iii<=ii+1;iii++)
				for (jjj=jj;jjj<=jj+1;jjj++)
				    {
				    kkk = iii * data->primary_ny + jjj;
				    if (data->primary_data[kkk] != data->primary_nodatavalue)
					{
					nfound++;
					zdata += data->primary_data[kkk];
					}
				    }
				}
			if (nfound > 0)
				zdata /= (double)nfound;
			else
				zdata = data->navs[inav].navpts[i].pointcntr.zdata;
			data->navs[inav].navpts[i].pointstbd.zdata = zdata;
			data->navs[inav].navpts[i].pointstbd.zdisplay 
				= view->zscale * (data->navs[inav].navpts[i].pointstbd.zdata - view->zorigin);
			}
			
		/* drape the segments */
		for (i=0;i<data->navs[inav].npoints-1;i++)
			{
			/* drape the segment */
			mbview_drapesegment(instance, &(data->navs[inav].segments[i]));
			}


		/* make navs viewable */
		data->nav_view_mode = MBV_VIEW_ON;
		
fprintf(stderr,"Added %d nav from %s\n", npoint, navname);
		}
	
	/* print nav debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Nav data altered in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Nav values:\n");
		fprintf(stderr,"dbg2       nav_mode:           %d\n",data->nav_mode);
		fprintf(stderr,"dbg2       nav_view_mode:      %d\n",data->nav_view_mode);
		fprintf(stderr,"dbg2       navdrape_view_mode: %d\n",data->navdrape_view_mode);
		fprintf(stderr,"dbg2       nnav:               %d\n",data->nnav);
		fprintf(stderr,"dbg2       nnav_alloc:         %d\n",data->nnav_alloc);
		fprintf(stderr,"dbg2       nav_selected[0]:    %d\n",data->nav_selected[0]);
		fprintf(stderr,"dbg2       nav_selected[1]:    %d\n",data->nav_selected[1]);
		fprintf(stderr,"dbg2       nav_point_selected: %d\n",data->nav_point_selected);
		for (i=0;i<data->nnav;i++)
			{
			fprintf(stderr,"dbg2       nav %d color:         %d\n",i,data->navs[i].color);
			fprintf(stderr,"dbg2       nav %d size:          %d\n",i,data->navs[i].size);
			fprintf(stderr,"dbg2       nav %d name:          %s\n",i,data->navs[i].name);
			fprintf(stderr,"dbg2       nav %d swathbounds:   %d\n",i,data->navs[i].swathbounds);
			fprintf(stderr,"dbg2       nav %d shot:          %d\n",i,data->navs[i].shot);
			fprintf(stderr,"dbg2       nav %d cdp:           %d\n",i,data->navs[i].cdp);
			fprintf(stderr,"dbg2       nav %d npoints:       %d\n",i,data->navs[i].npoints);
			fprintf(stderr,"dbg2       nav %d npoints_alloc: %d\n",i,data->navs[i].npoints_alloc);
			fprintf(stderr,"dbg2       nav %d nselected:     %d\n",i,data->navs[i].nselected);
			for (j=0;j<data->navs[i].npoints;j++)
				{
				fprintf(stderr,"dbg2       nav %d %d draped:   %d\n",i,j,data->navs[i].navpts[j].draped);
				fprintf(stderr,"dbg2       nav %d %d selected: %d\n",i,j,data->navs[i].navpts[j].selected);
				fprintf(stderr,"dbg2       nav %d %d time_d:   %f\n",i,j,data->navs[i].navpts[j].time_d);
				fprintf(stderr,"dbg2       nav %d %d heading:  %f\n",i,j,data->navs[i].navpts[j].heading);
				fprintf(stderr,"dbg2       nav %d %d speed:    %f\n",i,j,data->navs[i].navpts[j].speed);
				fprintf(stderr,"dbg2       nav %d %d shot:     %d\n",i,j,data->navs[i].navpts[j].shot);
				fprintf(stderr,"dbg2       nav %d %d cdp:      %d\n",i,j,data->navs[i].navpts[j].cdp);

				fprintf(stderr,"dbg2       nav %d %d xgrid:    %f\n",i,j,data->navs[i].navpts[j].point.xgrid);
				fprintf(stderr,"dbg2       nav %d %d ygrid:    %f\n",i,j,data->navs[i].navpts[j].point.ygrid);
				fprintf(stderr,"dbg2       nav %d %d xlon:     %f\n",i,j,data->navs[i].navpts[j].point.xlon);
				fprintf(stderr,"dbg2       nav %d %d ylat:     %f\n",i,j,data->navs[i].navpts[j].point.ylat);
				fprintf(stderr,"dbg2       nav %d %d zdata:    %f\n",i,j,data->navs[i].navpts[j].point.zdata);
				fprintf(stderr,"dbg2       nav %d %d xdisplay: %f\n",i,j,data->navs[i].navpts[j].point.xdisplay);
				fprintf(stderr,"dbg2       nav %d %d ydisplay: %f\n",i,j,data->navs[i].navpts[j].point.ydisplay);
				fprintf(stderr,"dbg2       nav %d %d zdisplay: %f\n",i,j,data->navs[i].navpts[j].point.zdisplay);

				fprintf(stderr,"dbg2       nav %d %d stbd xgrid:    %f\n",i,j,data->navs[i].navpts[j].pointport.xgrid);
				fprintf(stderr,"dbg2       nav %d %d stbd ygrid:    %f\n",i,j,data->navs[i].navpts[j].pointport.ygrid);
				fprintf(stderr,"dbg2       nav %d %d stbd xlon:     %f\n",i,j,data->navs[i].navpts[j].pointport.xlon);
				fprintf(stderr,"dbg2       nav %d %d stbd ylat:     %f\n",i,j,data->navs[i].navpts[j].pointport.ylat);
				fprintf(stderr,"dbg2       nav %d %d stbd zdata:    %f\n",i,j,data->navs[i].navpts[j].pointport.zdata);
				fprintf(stderr,"dbg2       nav %d %d stbd xdisplay: %f\n",i,j,data->navs[i].navpts[j].pointport.xdisplay);
				fprintf(stderr,"dbg2       nav %d %d stbd ydisplay: %f\n",i,j,data->navs[i].navpts[j].pointport.ydisplay);
				fprintf(stderr,"dbg2       nav %d %d stbd zdisplay: %f\n",i,j,data->navs[i].navpts[j].pointport.zdisplay);

				fprintf(stderr,"dbg2       nav %d %d cntr xgrid:    %f\n",i,j,data->navs[i].navpts[j].pointcntr.xgrid);
				fprintf(stderr,"dbg2       nav %d %d cntr ygrid:    %f\n",i,j,data->navs[i].navpts[j].pointcntr.ygrid);
				fprintf(stderr,"dbg2       nav %d %d cntr xlon:     %f\n",i,j,data->navs[i].navpts[j].pointcntr.xlon);
				fprintf(stderr,"dbg2       nav %d %d cntr ylat:     %f\n",i,j,data->navs[i].navpts[j].pointcntr.ylat);
				fprintf(stderr,"dbg2       nav %d %d cntr zdata:    %f\n",i,j,data->navs[i].navpts[j].pointcntr.zdata);
				fprintf(stderr,"dbg2       nav %d %d cntr xdisplay: %f\n",i,j,data->navs[i].navpts[j].pointcntr.xdisplay);
				fprintf(stderr,"dbg2       nav %d %d cntr ydisplay: %f\n",i,j,data->navs[i].navpts[j].pointcntr.ydisplay);
				fprintf(stderr,"dbg2       nav %d %d cntr zdisplay: %f\n",i,j,data->navs[i].navpts[j].pointcntr.zdisplay);

				fprintf(stderr,"dbg2       nav %d %d port xgrid:    %f\n",i,j,data->navs[i].navpts[j].pointstbd.xgrid);
				fprintf(stderr,"dbg2       nav %d %d port ygrid:    %f\n",i,j,data->navs[i].navpts[j].pointstbd.ygrid);
				fprintf(stderr,"dbg2       nav %d %d port xlon:     %f\n",i,j,data->navs[i].navpts[j].pointstbd.xlon);
				fprintf(stderr,"dbg2       nav %d %d port ylat:     %f\n",i,j,data->navs[i].navpts[j].pointstbd.ylat);
				fprintf(stderr,"dbg2       nav %d %d port zdata:    %f\n",i,j,data->navs[i].navpts[j].pointstbd.zdata);
				fprintf(stderr,"dbg2       nav %d %d port xdisplay: %f\n",i,j,data->navs[i].navpts[j].pointstbd.xdisplay);
				fprintf(stderr,"dbg2       nav %d %d port ydisplay: %f\n",i,j,data->navs[i].navpts[j].pointstbd.ydisplay);
				fprintf(stderr,"dbg2       nav %d %d port zdisplay: %f\n",i,j,data->navs[i].navpts[j].pointstbd.zdisplay);
				}
			for (j=0;j<data->navs[i].npoints-1;j++)
				{
				fprintf(stderr,"dbg2       nav %d %d nls:          %d\n",i,j,data->navs[i].segments[j].nls);
				fprintf(stderr,"dbg2       nav %d %d nls_alloc:    %d\n",i,j,data->navs[i].segments[j].nls_alloc);
				fprintf(stderr,"dbg2       nav %d %d endpoints[0]: %d\n",i,j,data->navs[i].segments[j].endpoints[0]);
				fprintf(stderr,"dbg2       nav %d %d endpoints[1]: %d\n",i,j,data->navs[i].segments[j].endpoints[1]);
				}
			}
		}
		
	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:                     %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n",status);
		}

	/* return */
	return(status);
}


/*------------------------------------------------------------------------------*/
int mbview_enableviewnavs(int verbose, int instance,
			int *error)

{
	/* local variables */
	char	*function_name = "mbview_enableviewnavs";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

fprintf(stderr,"Called mbview_enableviewnavs:%d\n",instance);

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr,"dbg2       instance:                  %d\n", instance);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* set values */
        data->nav_mode = MBV_NAV_VIEW;
		
	/* set widget sensitivity */
	if (data->active == MB_YES)
		mbview_update_sensitivity(verbose, instance, error);
		
	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:                     %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n",status);
		}

	/* return */
	return(status);
}


/*------------------------------------------------------------------------------*/
int mbview_pick_nav_select(int instance, int select, int which, int xpixel, int ypixel)
{

	/* local variables */
	char	*function_name = "mbview_pick_nav_select";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	found;
	double	xgrid, ygrid;
	double	xlon, ylat, zdata;
	double	xdisplay, ydisplay, zdisplay;
	double	xx, yy, rr, rrmin;
	int	inav0, inav1, jpt0, jpt1;
	int	jj0, jj1;
	int	inav, jpt;
	int	i, j;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       select:           %d\n",select);
		fprintf(stderr,"dbg2       which:            %d\n",which);
		fprintf(stderr,"dbg2       xpixel:           %d\n",xpixel);
		fprintf(stderr,"dbg2       ypixel:           %d\n",ypixel);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* only select one nav point if enabled and not in move mode */
	if (data->nav_mode != MBV_NAV_OFF
		&& data->nnav > 0
		&& (which == MBV_PICK_DOWN
			|| data->nav_selected[0] == MBV_SELECT_NONE))
		{
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, 
				&found, 
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);

		/* look for nearest nav point */
		if (found)
			{
			rrmin = 1000000000.0;
			data->nav_selected[0] = MBV_SELECT_NONE;
			data->nav_point_selected[0] = MBV_SELECT_NONE;
			data->nav_selected[1] = MBV_SELECT_NONE;
			data->nav_point_selected[1] = MBV_SELECT_NONE;

			for (i=0;i<data->nnav;i++)
				{
				for (j=0;j<data->navs[i].npoints;j++)
					{
					xx = xgrid - data->navs[i].navpts[j].point.xgrid;
					yy = ygrid - data->navs[i].navpts[j].point.ygrid;
					rr = sqrt(xx * xx + yy * yy);
					if (rr < rrmin)
						{
						rrmin = rr;
						data->nav_selected[0] = i;
						data->nav_point_selected[0] = j;
						}
					}
				}
				
			/* set pick location */
			data->pickinfo_mode = MBV_PICK_NAV;
			data->navpick_type = MBV_PICK_ONEPOINT;
			data->navpick.endpoints[0].xgrid 
				= data->navs[data->nav_selected[0]].navpts[data->nav_point_selected[0]].point.xgrid;
			data->navpick.endpoints[0].ygrid 
				= data->navs[data->nav_selected[0]].navpts[data->nav_point_selected[0]].point.ygrid;
			data->navpick.endpoints[0].xlon 
				= data->navs[data->nav_selected[0]].navpts[data->nav_point_selected[0]].point.xlon;
			data->navpick.endpoints[0].ylat 
				= data->navs[data->nav_selected[0]].navpts[data->nav_point_selected[0]].point.ylat;
			data->navpick.endpoints[0].zdata 
				= data->navs[data->nav_selected[0]].navpts[data->nav_point_selected[0]].point.zdata;
			data->navpick.endpoints[0].xdisplay 
				= data->navs[data->nav_selected[0]].navpts[data->nav_point_selected[0]].point.xdisplay;
			data->navpick.endpoints[0].ydisplay 
				= data->navs[data->nav_selected[0]].navpts[data->nav_point_selected[0]].point.ydisplay;
			data->navpick.endpoints[0].zdisplay 
				= data->navs[data->nav_selected[0]].navpts[data->nav_point_selected[0]].point.zdisplay;
			
			/* generate 3D drape of pick marks  */
			mbview_navpicksize(instance);
			}
		else
			{
			/* unselect nav pick */
			data->pickinfo_mode = data->pick_type;
			data->navpick_type = MBV_PICK_NONE;
			data->nav_selected[0] = MBV_SELECT_NONE;
			data->nav_point_selected[0] = MBV_SELECT_NONE;
			XBell(view->dpy,100);
			}
		}
	
	/* only select two nav points if enabled */
	else if (data->nav_mode != MBV_NAV_OFF
		&& data->nnav > 0
		&& (which == MBV_PICK_MOVE
			&& data->nav_selected[0] != MBV_SELECT_NONE))
		{
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, 
				&found, 
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);
				
		/* look for nearest nav point */
		if (found)
			{
			rrmin = 1000000000.0;
			data->nav_selected[1] = MBV_SELECT_NONE;
			data->nav_point_selected[1] = MBV_SELECT_NONE;

			for (i=0;i<data->nnav;i++)
				{
				for (j=0;j<data->navs[i].npoints;j++)
					{
					xx = xgrid - data->navs[i].navpts[j].point.xgrid;
					yy = ygrid - data->navs[i].navpts[j].point.ygrid;
					rr = sqrt(xx * xx + yy * yy);
					if (rr < rrmin)
						{
						rrmin = rr;
						data->nav_selected[1] = i;
						data->nav_point_selected[1] = j;
						}
					}
				}
				
			/* set pick location */
			data->pickinfo_mode = MBV_PICK_NAV;
			data->navpick_type = MBV_PICK_TWOPOINT;
			data->navpick.endpoints[1].xgrid 
				= data->navs[data->nav_selected[1]].navpts[data->nav_point_selected[1]].point.xgrid;
			data->navpick.endpoints[1].ygrid 
				= data->navs[data->nav_selected[1]].navpts[data->nav_point_selected[1]].point.ygrid;
			data->navpick.endpoints[1].xlon 
				= data->navs[data->nav_selected[1]].navpts[data->nav_point_selected[1]].point.xlon;
			data->navpick.endpoints[1].ylat 
				= data->navs[data->nav_selected[1]].navpts[data->nav_point_selected[1]].point.ylat;
			data->navpick.endpoints[1].zdata 
				= data->navs[data->nav_selected[1]].navpts[data->nav_point_selected[1]].point.zdata;
			data->navpick.endpoints[1].xdisplay 
				= data->navs[data->nav_selected[1]].navpts[data->nav_point_selected[1]].point.xdisplay;
			data->navpick.endpoints[1].ydisplay 
				= data->navs[data->nav_selected[1]].navpts[data->nav_point_selected[1]].point.ydisplay;
			data->navpick.endpoints[1].zdisplay 
				= data->navs[data->nav_selected[1]].navpts[data->nav_point_selected[1]].point.zdisplay;
			
			/* generate 3D drape of pick marks */
			mbview_navpicksize(instance);
			}
		}
	
	/* only select or deselect range of nav points if enabled and two different points selected */
	else if (data->nav_mode != MBV_NAV_OFF
		&& data->nnav > 0
		&& which == MBV_PICK_UP)
		{
		/* only actually select range of nav if two different points have been selected */
		if (data->nav_selected[0] != MBV_SELECT_NONE
			&& data->nav_selected[1] != MBV_SELECT_NONE
			&& !(data->nav_selected[0] == data->nav_selected[1]
				&& data->nav_point_selected[0] == data->nav_point_selected[1]))
			{
			/* get order of selected nav points */
			inav0 = MIN(data->nav_selected[0], data->nav_selected[1]);
			inav1 = MAX(data->nav_selected[0], data->nav_selected[1]);
			if (inav0 == inav1)
				{
				jpt0 = MIN(data->nav_point_selected[0], data->nav_point_selected[1]);
				jpt1 = MAX(data->nav_point_selected[0], data->nav_point_selected[1]);
				}
			else if (data->nav_selected[0] <= data->nav_selected[1])
				{
				jpt0 = data->nav_point_selected[0];
				jpt1 = data->nav_point_selected[1];
				}
			else
				{
				jpt0 = data->nav_point_selected[1];
				jpt1 = data->nav_point_selected[0];
				}
			/* loop over the affected nav */
			for (inav=inav0;inav<=inav1;inav++)
				{
				if (inav == inav0)
					jj0 = MIN(jpt0, data->navs[inav].npoints - 1);
				else
					jj0 = 0;
				if (inav == inav1)
					jj1 = MAX(jpt1, 0);
				else
					jj1 = data->navs[inav].npoints;
				for (jpt=jj0;jpt<=jj1;jpt++)
					{
					data->navs[inav].navpts[jpt].selected = select;
					}
				data->navs[inav].nselected = 0;
				for (jpt=0;jpt<data->navs[inav].npoints;jpt++)
					{
					if (data->navs[inav].navpts[jpt].selected == MB_YES)
						data->navs[inav].nselected++;
					}
				}
			}
		}

	/* else beep */
	else
		{
		data->nav_selected[0] = MBV_SELECT_NONE;
		data->nav_point_selected[0] = MBV_SELECT_NONE;
		data->nav_selected[1] = MBV_SELECT_NONE;
		data->nav_point_selected[1] = MBV_SELECT_NONE;
		XBell(view->dpy,100);
		for (i=0;i<data->nnav;i++)
			{
			for (j=0;j<data->navs[i].npoints;j++)
				{
				data->navs[i].navpts[j].selected = MB_NO;
				}
			}
		}
		
	/* set what kind of pick to annotate */
	if (data->nav_selected[0] != MBV_SELECT_NONE)
		{
		data->pickinfo_mode = MBV_PICK_NAV;
		}
	else
		{
		data->pickinfo_mode = data->pick_type;
		}
		
	/* set pick annotation */
	mbview_pick_text(instance);
	
	/* print nav debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Nav data altered in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Nav values:\n");
		fprintf(stderr,"dbg2       nav_mode:              %d\n",data->nav_mode);
		fprintf(stderr,"dbg2       nav_view_mode:         %d\n",data->nav_view_mode);
		fprintf(stderr,"dbg2       navdrape_view_mode:    %d\n",data->navdrape_view_mode);
		fprintf(stderr,"dbg2       nnav:                  %d\n",data->nnav);
		fprintf(stderr,"dbg2       nnav_alloc:            %d\n",data->nnav_alloc);
		fprintf(stderr,"dbg2       nav_selected[0]:       %d\n",data->nav_selected[0]);
		fprintf(stderr,"dbg2       nav_point_selected[0]: %d\n",data->nav_point_selected[0]);
		fprintf(stderr,"dbg2       nav_selected[1]:       %d\n",data->nav_selected[1]);
		fprintf(stderr,"dbg2       nav_point_selected[1]: %d\n",data->nav_point_selected[1]);
		for (i=0;i<data->nnav;i++)
			{
			fprintf(stderr,"dbg2       nav %d color:         %d\n",i,data->navs[i].color);
			fprintf(stderr,"dbg2       nav %d size:          %d\n",i,data->navs[i].size);
			fprintf(stderr,"dbg2       nav %d name:          %s\n",i,data->navs[i].name);
			fprintf(stderr,"dbg2       nav %d swathbounds:   %d\n",i,data->navs[i].swathbounds);
			fprintf(stderr,"dbg2       nav %d shot:          %d\n",i,data->navs[i].shot);
			fprintf(stderr,"dbg2       nav %d cdp:           %d\n",i,data->navs[i].cdp);
			fprintf(stderr,"dbg2       nav %d npoints:       %d\n",i,data->navs[i].npoints);
			fprintf(stderr,"dbg2       nav %d npoints_alloc: %d\n",i,data->navs[i].npoints_alloc);
			fprintf(stderr,"dbg2       nav %d nselected:     %d\n",i,data->navs[i].nselected);
			for (j=0;j<data->navs[i].npoints;j++)
				{
				fprintf(stderr,"dbg2       nav %d %d draped:   %d\n",i,j,data->navs[i].navpts[j].draped);
				fprintf(stderr,"dbg2       nav %d %d selected: %d\n",i,j,data->navs[i].navpts[j].selected);
				fprintf(stderr,"dbg2       nav %d %d time_d:   %f\n",i,j,data->navs[i].navpts[j].time_d);
				fprintf(stderr,"dbg2       nav %d %d heading:  %f\n",i,j,data->navs[i].navpts[j].heading);
				fprintf(stderr,"dbg2       nav %d %d speed:    %f\n",i,j,data->navs[i].navpts[j].speed);
				fprintf(stderr,"dbg2       nav %d %d shot:     %d\n",i,j,data->navs[i].navpts[j].shot);
				fprintf(stderr,"dbg2       nav %d %d cdp:      %d\n",i,j,data->navs[i].navpts[j].cdp);

				fprintf(stderr,"dbg2       nav %d %d xgrid:    %f\n",i,j,data->navs[i].navpts[j].point.xgrid);
				fprintf(stderr,"dbg2       nav %d %d ygrid:    %f\n",i,j,data->navs[i].navpts[j].point.ygrid);
				fprintf(stderr,"dbg2       nav %d %d xlon:     %f\n",i,j,data->navs[i].navpts[j].point.xlon);
				fprintf(stderr,"dbg2       nav %d %d ylat:     %f\n",i,j,data->navs[i].navpts[j].point.ylat);
				fprintf(stderr,"dbg2       nav %d %d zdata:    %f\n",i,j,data->navs[i].navpts[j].point.zdata);
				fprintf(stderr,"dbg2       nav %d %d xdisplay: %f\n",i,j,data->navs[i].navpts[j].point.xdisplay);
				fprintf(stderr,"dbg2       nav %d %d ydisplay: %f\n",i,j,data->navs[i].navpts[j].point.ydisplay);
				fprintf(stderr,"dbg2       nav %d %d zdisplay: %f\n",i,j,data->navs[i].navpts[j].point.zdisplay);

				fprintf(stderr,"dbg2       nav %d %d stbd xgrid:    %f\n",i,j,data->navs[i].navpts[j].pointport.xgrid);
				fprintf(stderr,"dbg2       nav %d %d stbd ygrid:    %f\n",i,j,data->navs[i].navpts[j].pointport.ygrid);
				fprintf(stderr,"dbg2       nav %d %d stbd xlon:     %f\n",i,j,data->navs[i].navpts[j].pointport.xlon);
				fprintf(stderr,"dbg2       nav %d %d stbd ylat:     %f\n",i,j,data->navs[i].navpts[j].pointport.ylat);
				fprintf(stderr,"dbg2       nav %d %d stbd zdata:    %f\n",i,j,data->navs[i].navpts[j].pointport.zdata);
				fprintf(stderr,"dbg2       nav %d %d stbd xdisplay: %f\n",i,j,data->navs[i].navpts[j].pointport.xdisplay);
				fprintf(stderr,"dbg2       nav %d %d stbd ydisplay: %f\n",i,j,data->navs[i].navpts[j].pointport.ydisplay);
				fprintf(stderr,"dbg2       nav %d %d stbd zdisplay: %f\n",i,j,data->navs[i].navpts[j].pointport.zdisplay);

				fprintf(stderr,"dbg2       nav %d %d cntr xgrid:    %f\n",i,j,data->navs[i].navpts[j].pointcntr.xgrid);
				fprintf(stderr,"dbg2       nav %d %d cntr ygrid:    %f\n",i,j,data->navs[i].navpts[j].pointcntr.ygrid);
				fprintf(stderr,"dbg2       nav %d %d cntr xlon:     %f\n",i,j,data->navs[i].navpts[j].pointcntr.xlon);
				fprintf(stderr,"dbg2       nav %d %d cntr ylat:     %f\n",i,j,data->navs[i].navpts[j].pointcntr.ylat);
				fprintf(stderr,"dbg2       nav %d %d cntr zdata:    %f\n",i,j,data->navs[i].navpts[j].pointcntr.zdata);
				fprintf(stderr,"dbg2       nav %d %d cntr xdisplay: %f\n",i,j,data->navs[i].navpts[j].pointcntr.xdisplay);
				fprintf(stderr,"dbg2       nav %d %d cntr ydisplay: %f\n",i,j,data->navs[i].navpts[j].pointcntr.ydisplay);
				fprintf(stderr,"dbg2       nav %d %d cntr zdisplay: %f\n",i,j,data->navs[i].navpts[j].pointcntr.zdisplay);

				fprintf(stderr,"dbg2       nav %d %d port xgrid:    %f\n",i,j,data->navs[i].navpts[j].pointstbd.xgrid);
				fprintf(stderr,"dbg2       nav %d %d port ygrid:    %f\n",i,j,data->navs[i].navpts[j].pointstbd.ygrid);
				fprintf(stderr,"dbg2       nav %d %d port xlon:     %f\n",i,j,data->navs[i].navpts[j].pointstbd.xlon);
				fprintf(stderr,"dbg2       nav %d %d port ylat:     %f\n",i,j,data->navs[i].navpts[j].pointstbd.ylat);
				fprintf(stderr,"dbg2       nav %d %d port zdata:    %f\n",i,j,data->navs[i].navpts[j].pointstbd.zdata);
				fprintf(stderr,"dbg2       nav %d %d port xdisplay: %f\n",i,j,data->navs[i].navpts[j].pointstbd.xdisplay);
				fprintf(stderr,"dbg2       nav %d %d port ydisplay: %f\n",i,j,data->navs[i].navpts[j].pointstbd.ydisplay);
				fprintf(stderr,"dbg2       nav %d %d port zdisplay: %f\n",i,j,data->navs[i].navpts[j].pointstbd.zdisplay);
				}
			for (j=0;j<data->navs[i].npoints-1;j++)
				{
				fprintf(stderr,"dbg2       nav %d %d nls:          %d\n",i,j,data->navs[i].segments[j].nls);
				fprintf(stderr,"dbg2       nav %d %d nls_alloc:    %d\n",i,j,data->navs[i].segments[j].nls_alloc);
				fprintf(stderr,"dbg2       nav %d %d endpoints[0]: %d\n",i,j,data->navs[i].segments[j].endpoints[0]);
				fprintf(stderr,"dbg2       nav %d %d endpoints[1]: %d\n",i,j,data->navs[i].segments[j].endpoints[1]);
				}
			}
		}
	
	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_navpicksize(int instance)
{

	/* local variables */
	char	*function_name = "mbview_navpicksize";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	double	xlength;
	double	headingx, headingy;
	int	found;
	int	i;

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

	/* resize and redrape navpick marks if required */
	if (data->navpick_type != MBV_PICK_NONE)
		{
		/* set size of 'V' marks in gl units for 3D case */
		xlength = 0.05;
		headingx = sin(data->navs[data->nav_selected[0]].navpts[data->nav_point_selected[0]].heading * DTR);
		headingy = cos(data->navs[data->nav_selected[0]].navpts[data->nav_point_selected[0]].heading * DTR);

		/* set navpick location V marker */
		data->navpick.xpoints[0].xdisplay = data->navpick.endpoints[0].xdisplay 
							+ xlength * (headingy - headingx);
		data->navpick.xpoints[0].ydisplay = data->navpick.endpoints[0].ydisplay 
							- xlength * (headingx + headingy);
		data->navpick.xpoints[1].xdisplay = data->navpick.endpoints[0].xdisplay;
		data->navpick.xpoints[1].ydisplay = data->navpick.endpoints[0].ydisplay;
		data->navpick.xpoints[2].xdisplay = data->navpick.endpoints[0].xdisplay;
		data->navpick.xpoints[2].ydisplay = data->navpick.endpoints[0].ydisplay;
		data->navpick.xpoints[3].xdisplay = data->navpick.endpoints[0].xdisplay 
							- xlength * (headingx + headingy);
		data->navpick.xpoints[3].ydisplay = data->navpick.endpoints[0].ydisplay 
							+ xlength * (headingx - headingy);
		for (i=0;i<4;i++)
			{
			mbview_projectinverse(instance, MB_YES,
				data->navpick.xpoints[i].xdisplay, 
				data->navpick.xpoints[i].ydisplay,
				&data->navpick.xpoints[i].xlon, 
				&data->navpick.xpoints[i].ylat, 
				&data->navpick.xpoints[i].xgrid, 
				&data->navpick.xpoints[i].ygrid);
			mbview_getzdata(instance, 
				data->navpick.xpoints[i].xgrid, 
				data->navpick.xpoints[i].ygrid,
				&found, &data->navpick.xpoints[i].zdata);
			if (found == MB_NO)
				data->navpick.xpoints[i].zdata 
					= data->navpick.endpoints[0].zdata;
			data->navpick.xpoints[i].zdisplay = view->zscale 
				* (data->navpick.xpoints[i].zdata - view->zorigin);
			}

		/* drape the V marker line segments */
		for (i=0;i<2;i++)
			{
			mbview_drapesegment(instance, &(data->navpick.xsegments[i]));
			}
		}
	if (data->navpick_type == MBV_PICK_TWOPOINT)
		{
		headingx = sin(data->navs[data->nav_selected[0]].navpts[data->nav_point_selected[0]].heading * DTR);
		headingy = cos(data->navs[data->nav_selected[0]].navpts[data->nav_point_selected[0]].heading * DTR);

		/* set navpick location V marker */
		data->navpick.xpoints[4].xdisplay = data->navpick.endpoints[1].xdisplay 
							+ xlength * (headingy - headingx);
		data->navpick.xpoints[4].ydisplay = data->navpick.endpoints[1].ydisplay 
							- xlength * (headingx + headingy);
		data->navpick.xpoints[5].xdisplay = data->navpick.endpoints[1].xdisplay;
		data->navpick.xpoints[5].ydisplay = data->navpick.endpoints[1].ydisplay;
		data->navpick.xpoints[6].xdisplay = data->navpick.endpoints[1].xdisplay;
		data->navpick.xpoints[6].ydisplay = data->navpick.endpoints[1].ydisplay;
		data->navpick.xpoints[7].xdisplay = data->navpick.endpoints[1].xdisplay 
							- xlength * (headingx + headingy);
		data->navpick.xpoints[7].ydisplay = data->navpick.endpoints[1].ydisplay 
							+ xlength * (headingx - headingy);
		for (i=0;i<4;i++)
			{
			mbview_projectinverse(instance, MB_YES,
				data->navpick.xpoints[i+4].xdisplay, 
				data->navpick.xpoints[i+4].ydisplay,
				&data->navpick.xpoints[i+4].xlon, 
				&data->navpick.xpoints[i+4].ylat, 
				&data->navpick.xpoints[i+4].xgrid, 
				&data->navpick.xpoints[i+4].ygrid);
			mbview_getzdata(instance, 
				data->navpick.xpoints[i+4].xgrid, 
				data->navpick.xpoints[i+4].ygrid,
				&found, &data->navpick.xpoints[i+4].zdata);
			if (found == MB_NO)
				data->navpick.xpoints[i+4].zdata 
					= data->navpick.endpoints[1].zdata;
			data->navpick.xpoints[i].zdisplay = view->zscale 
				* (data->navpick.xpoints[i+4].zdata - view->zorigin);
			}

		/* drape the V marker line segments */
		for (i=0;i<2;i++)
			{
			mbview_drapesegment(instance, &(data->navpick.xsegments[i+2]));
			}
		}
	
	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return */
	return(status);
}
/*------------------------------------------------------------------------------*/
int mbview_drawnavpick(int instance)
{
	/* local variables */
	char	*function_name = "mbview_drawnavpick";
	int	status = MB_SUCCESS;
	int	i, j;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	double	xlength;
	float	zdisplay;
	int	inav, jpt;

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
		
	/* draw current navpick */
	if (data->navpick_type != MBV_PICK_NONE
		&& (data->nav_view_mode == MBV_VIEW_ON
			|| data->navdrape_view_mode == MBV_VIEW_ON))
		{
		/* set size of X mark for 2D case */
		if (data->display_mode == MBV_DISPLAY_2D)
			xlength = 0.05 / view->size2d;

		/* set color and linewidth */
		glColor3f(1.0, 0.0, 0.0);
		glLineWidth(3.0);
		glBegin(GL_LINE);
		
		/* plot first navpick point draped */
		if (data->display_mode == MBV_DISPLAY_3D 
			&& data->navpick.xsegments[0].nls > 0 
			&& data->navpick.xsegments[1].nls > 0)
			{
			glBegin(GL_LINE_STRIP);
			for (i=0;i<data->navpick.xsegments[0].nls;i++)
				{
				glVertex3f((float)(data->navpick.xsegments[0].lspoints[i].xdisplay), 
						(float)(data->navpick.xsegments[0].lspoints[i].ydisplay), 
						(float)(data->navpick.xsegments[0].lspoints[i].zdisplay));
				}
			glEnd();
			glBegin(GL_LINE_STRIP);
			for (i=0;i<data->navpick.xsegments[1].nls;i++)
				{
				glVertex3f((float)(data->navpick.xsegments[1].lspoints[i].xdisplay), 
						(float)(data->navpick.xsegments[1].lspoints[i].ydisplay), 
						(float)(data->navpick.xsegments[1].lspoints[i].zdisplay));
				}
			glEnd();
			}
		else if (data->display_mode == MBV_DISPLAY_3D)
			{
			glBegin(GL_LINES);
			for (i=0;i<4;i++)
				{
				glVertex3f((float)(data->navpick.xpoints[i].xdisplay), 
					(float)(data->navpick.xpoints[i].ydisplay), 
					(float)(data->navpick.xpoints[i].zdisplay));
				}
			glEnd();
			}
		else
			{
			glBegin(GL_LINES);
			glVertex3f((float)(data->navpick.xpoints[0].xdisplay), 
				(float)(data->navpick.xpoints[0].ydisplay), 
				(float)(data->navpick.xpoints[0].zdisplay));
			glVertex3f((float)(data->navpick.xpoints[1].xdisplay), 
				(float)(data->navpick.xpoints[1].ydisplay), 
				(float)(data->navpick.xpoints[1].zdisplay));
			glVertex3f((float)(data->navpick.xpoints[2].xdisplay), 
				(float)(data->navpick.xpoints[2].ydisplay), 
				(float)(data->navpick.xpoints[2].zdisplay));
			glVertex3f((float)(data->navpick.xpoints[3].xdisplay), 
				(float)(data->navpick.xpoints[3].ydisplay), 
				(float)(data->navpick.xpoints[3].zdisplay));
			glEnd();
			}
			
		/* draw first navpick point undraped */
		if (data->display_mode == MBV_DISPLAY_3D
			&& data->nav_view_mode == MBV_VIEW_ON)
			{
			inav = data->nav_selected[0];
			jpt = data->nav_point_selected[0];
			zdisplay = data->navs[inav].navpts[jpt].point.zdisplay;
			glColor3f(0.0, 0.0, 1.0);
			glBegin(GL_LINES);
			for (i=0;i<4;i++)
				{
				glVertex3f((float)(data->navpick.xpoints[i].xdisplay), 
					(float)(data->navpick.xpoints[i].ydisplay), 
					zdisplay);
				}
			glEnd();
			glColor3f(1.0, 0.0, 0.0);
			}
		
		if (data->navpick_type == MBV_PICK_TWOPOINT)
			{
			/* plot second navpick point draped */
			if (data->display_mode == MBV_DISPLAY_3D 
				&& data->navpick.xsegments[2].nls > 0 
				&& data->navpick.xsegments[3].nls > 0)
				{
				glBegin(GL_LINE_STRIP);
				for (i=0;i<data->navpick.xsegments[2].nls;i++)
					{
					glVertex3f((float)(data->navpick.xsegments[2].lspoints[i].xdisplay), 
							(float)(data->navpick.xsegments[2].lspoints[i].ydisplay), 
							(float)(data->navpick.xsegments[2].lspoints[i].zdisplay));
					}
				glEnd();
				glBegin(GL_LINE_STRIP);
				for (i=0;i<data->navpick.xsegments[3].nls;i++)
					{
					glVertex3f((float)(data->navpick.xsegments[3].lspoints[i].xdisplay), 
							(float)(data->navpick.xsegments[3].lspoints[i].ydisplay), 
							(float)(data->navpick.xsegments[3].lspoints[i].zdisplay));
					}
				glEnd();
				}
			else if (data->display_mode == MBV_DISPLAY_3D)
				{
				glBegin(GL_LINES);
				for (i=4;i<8;i++)
					{
					glVertex3f((float)(data->navpick.xpoints[i].xdisplay), 
						(float)(data->navpick.xpoints[i].ydisplay), 
						(float)(data->navpick.xpoints[i].zdisplay));
					}
				glEnd();
				}
			else
				{
				glBegin(GL_LINES);
				glVertex3f((float)(data->navpick.xpoints[4].xdisplay), 
					(float)(data->navpick.xpoints[4].ydisplay), 
					(float)(data->navpick.xpoints[4].zdisplay));
				glVertex3f((float)(data->navpick.xpoints[5].xdisplay), 
					(float)(data->navpick.xpoints[5].ydisplay), 
					(float)(data->navpick.xpoints[5].zdisplay));
				glVertex3f((float)(data->navpick.xpoints[6].xdisplay), 
					(float)(data->navpick.xpoints[6].ydisplay), 
					(float)(data->navpick.xpoints[6].zdisplay));
				glVertex3f((float)(data->navpick.xpoints[7].xdisplay), 
					(float)(data->navpick.xpoints[7].ydisplay), 
					(float)(data->navpick.xpoints[7].zdisplay));
				glEnd();
				}
			
			/* draw second navpick point undraped */
			if (data->display_mode == MBV_DISPLAY_3D
				&& data->nav_view_mode == MBV_VIEW_ON)
				{
				inav = data->nav_selected[1];
				jpt = data->nav_point_selected[1];
				zdisplay = data->navs[inav].navpts[jpt].point.zdisplay;
				glBegin(GL_LINES);
				for (i=4;i<8;i++)
					{
					glVertex3f((float)(data->navpick.xpoints[i].xdisplay), 
						(float)(data->navpick.xpoints[i].ydisplay), 
						zdisplay);
					}
				glEnd();
				}
			}
		glEnd();
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
int mbview_drawnav(int instance, int rez)
{
	/* local variables */
	char	*function_name = "mbview_drawnav";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	on, flip;
	int	stride;
	int	ixmin, ixmax, jymin, jymax;
	int	ix, jy;
	int	ixsize, jysize;
	int	isite;
	int	icolor;
	int	inav, jpoint;
	int	swathbounds_on;
	double	timegapuse;
	struct mbview_linesegment_struct segment;
	int	i, j, k, l, m, n, kk, ll;

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
		
	/* draw navigation */
	if (data->nav_mode != MBV_NAV_OFF
		&& data->nav_view_mode == MBV_VIEW_ON
		&& data->nnav > 0)
		{
		/* loop over the navs plotting xyz navigation */
		for (inav=0;inav<data->nnav;inav++)
			{
			icolor = data->navs[inav].color;
			glLineWidth((float)(data->navs[inav].size));
			glBegin(GL_LINE_STRIP);
			for (jpoint=0;jpoint<data->navs[inav].npoints;jpoint++)
				{
				/* set size and color */
				if (data->navs[inav].navpts[jpoint].selected == MB_YES
					|| data->navs[inav].navpts[jpoint+1].selected == MB_YES)
					{
					glColor3f(colortable_object_red[MBV_COLOR_RED], 
						colortable_object_green[MBV_COLOR_RED], 
						colortable_object_blue[MBV_COLOR_RED]);
					}
				else
					{
					glColor3f(colortable_object_red[icolor], 
						colortable_object_green[icolor], 
						colortable_object_blue[icolor]);
					}
					
				/* draw points */
				glVertex3f((float)(data->navs[inav].navpts[jpoint].point.xdisplay), 
						(float)(data->navs[inav].navpts[jpoint].point.ydisplay), 
						(float)(data->navs[inav].navpts[jpoint].point.zdisplay));
				}
			glEnd();
			}
		}
		
	/* draw navigation */
	if (data->nav_mode != MBV_NAV_OFF
		&& data->navdrape_view_mode == MBV_VIEW_ON
		&& data->nnav > 0)
		{
		/* loop over the navs plotting draped navigation */
		for (inav=0;inav<data->nnav;inav++)
			{
			icolor = data->navs[inav].color;
			glLineWidth((float)(data->navs[inav].size));
			glBegin(GL_LINE_STRIP);
			for (jpoint=0;jpoint<data->navs[inav].npoints-1;jpoint++)
				{
				/* set size and color */
				if (data->navs[inav].navpts[jpoint].selected == MB_YES
					|| data->navs[inav].navpts[jpoint+1].selected == MB_YES)
					{
					glColor3f(colortable_object_red[MBV_COLOR_RED], 
						colortable_object_green[MBV_COLOR_RED], 
						colortable_object_blue[MBV_COLOR_RED]);
					}
				else
					{
					glColor3f(colortable_object_red[icolor], 
						colortable_object_green[icolor], 
						colortable_object_blue[icolor]);
					}
					
				/* draw draped segment */
/*fprintf(stderr,"inav:%d jpoint:%d nls:%d\n", inav, jpoint, data->navs[inav].segments[jpoint].nls);*/
				for (k=0;k<data->navs[inav].segments[jpoint].nls;k++)
					{
					/* draw points */
					glVertex3f((float)(data->navs[inav].segments[jpoint].lspoints[k].xdisplay), 
							(float)(data->navs[inav].segments[jpoint].lspoints[k].ydisplay), 
							(float)(data->navs[inav].segments[jpoint].lspoints[k].zdisplay));
/*fprintf(stderr,"inav:%d jpoint:%d k:%d zdata:%f zdisplay:%f\n",
inav,jpoint,k,
data->navs[inav].segments[jpoint].lspoints[k].zdata,
data->navs[inav].segments[jpoint].lspoints[k].zdisplay);*/

					}
				}
			glEnd();
			}
		}
		
	/* draw swathbounds */
	if (data->nav_mode != MBV_NAV_OFF
		&& (data->nav_view_mode == MBV_VIEW_ON
			|| data->navdrape_view_mode == MBV_VIEW_ON)
		&& data->nnav > 0)
		{
		/* initialize on the fly draping segment */
		segment.nls = 0;
		segment.nls_alloc = 0;
		segment.lspoints = NULL;
		
		/* loop over the navs plotting swathbounds */
		timegapuse = 60.0 * view->timegap;
		for (inav=0;inav<data->nnav;inav++)
			{
			if (data->navs[inav].swathbounds == MB_YES
				&& data->navs[inav].nselected > 0)
				{
				glColor3f(colortable_object_red[MBV_COLOR_YELLOW], 
					colortable_object_green[MBV_COLOR_YELLOW], 
					colortable_object_blue[MBV_COLOR_YELLOW]);
				glLineWidth((float)(data->navs[inav].size));

				/* draw port side of swath */
				swathbounds_on = MB_NO;
				for (jpoint=0;jpoint<data->navs[inav].npoints;jpoint++)
					{
					/* draw from center at start of selected data */
					if (swathbounds_on == MB_NO
						&& data->navs[inav].navpts[jpoint].selected == MB_YES)
						{
						swathbounds_on = MB_YES;
						glBegin(GL_LINE_STRIP);
						
						if (data->display_mode == MBV_DISPLAY_3D)
							{
							/* drape segment on the fly */
							segment.endpoints[0] = &(data->navs[inav].navpts[jpoint].pointcntr);
							segment.endpoints[1] = &(data->navs[inav].navpts[jpoint].pointport);
							mbview_drapesegment(instance, &(segment));

							/* draw the segment */
							for (i=0;i<segment.nls;i++)
								{
								glVertex3f((float)(segment.lspoints[i].xdisplay), 
										(float)(segment.lspoints[i].ydisplay), 
										(float)(segment.lspoints[i].zdisplay));
								}
							}
						else
							{
							glVertex3f((float)(data->navs[inav].navpts[jpoint].pointcntr.xdisplay), 
									(float)(data->navs[inav].navpts[jpoint].pointcntr.ydisplay), 
									(float)(data->navs[inav].navpts[jpoint].pointcntr.zdisplay));
							}
						}

					/* draw during selected data */
					if (data->navs[inav].navpts[jpoint].selected == MB_YES)
						{
						glVertex3f((float)(data->navs[inav].navpts[jpoint].pointport.xdisplay), 
							(float)(data->navs[inav].navpts[jpoint].pointport.ydisplay), 
							(float)(data->navs[inav].navpts[jpoint].pointport.zdisplay));
						}

					/* draw to center at end of selected data */
					if (swathbounds_on == MB_YES
						&& (data->navs[inav].navpts[jpoint].selected == MB_NO
							|| jpoint >= data->navs[inav].npoints - 1
							|| (jpoint > 0 
								&& (data->navs[inav].navpts[jpoint].time_d
									- data->navs[inav].navpts[jpoint-1].time_d)
									> timegapuse)))
						{						
						if (data->display_mode == MBV_DISPLAY_3D)
							{
							/* drape segment on the fly */
							segment.endpoints[0] = &(data->navs[inav].navpts[jpoint].pointport);
							segment.endpoints[1] = &(data->navs[inav].navpts[jpoint].pointcntr);
							mbview_drapesegment(instance, &(segment));

							/* draw the segment */
							for (i=0;i<segment.nls;i++)
								{
								glVertex3f((float)(segment.lspoints[i].xdisplay), 
										(float)(segment.lspoints[i].ydisplay), 
										(float)(segment.lspoints[i].zdisplay));
								}
							}
						else
							{
							glVertex3f((float)(data->navs[inav].navpts[jpoint].pointcntr.xdisplay), 
									(float)(data->navs[inav].navpts[jpoint].pointcntr.ydisplay), 
									(float)(data->navs[inav].navpts[jpoint].pointcntr.zdisplay));
							}

						swathbounds_on = MB_NO;
						glEnd();
						}
					}

				/* draw starboard side of swath */
				swathbounds_on = MB_NO;
				for (jpoint=0;jpoint<data->navs[inav].npoints;jpoint++)
					{
					/* draw from center at start of selected data */
					if (swathbounds_on == MB_NO
						&& data->navs[inav].navpts[jpoint].selected == MB_YES)
						{
						swathbounds_on = MB_YES;
						glBegin(GL_LINE_STRIP);
						
						if (data->display_mode == MBV_DISPLAY_3D)
							{
							/* drape segment on the fly */
							segment.endpoints[0] = &(data->navs[inav].navpts[jpoint].pointcntr);
							segment.endpoints[1] = &(data->navs[inav].navpts[jpoint].pointstbd);
							mbview_drapesegment(instance, &(segment));

							/* draw the segment */
							for (i=0;i<segment.nls;i++)
								{
								glVertex3f((float)(segment.lspoints[i].xdisplay), 
										(float)(segment.lspoints[i].ydisplay), 
										(float)(segment.lspoints[i].zdisplay));
								}
							}
						else
							{
							glVertex3f((float)(data->navs[inav].navpts[jpoint].pointcntr.xdisplay), 
									(float)(data->navs[inav].navpts[jpoint].pointcntr.ydisplay), 
									(float)(data->navs[inav].navpts[jpoint].pointcntr.zdisplay));
							}
						}

					/* draw during selected data */
					if (data->navs[inav].navpts[jpoint].selected == MB_YES)
						{
						glVertex3f((float)(data->navs[inav].navpts[jpoint].pointstbd.xdisplay), 
							(float)(data->navs[inav].navpts[jpoint].pointstbd.ydisplay), 
							(float)(data->navs[inav].navpts[jpoint].pointstbd.zdisplay));
						}

					/* draw to center at end of selected data */
					if (swathbounds_on == MB_YES
						&& (data->navs[inav].navpts[jpoint].selected == MB_NO
							|| jpoint >= data->navs[inav].npoints - 1
							|| (jpoint > 0 
								&& (data->navs[inav].navpts[jpoint].time_d
									- data->navs[inav].navpts[jpoint-1].time_d)
									> timegapuse)))
						{						
						if (data->display_mode == MBV_DISPLAY_3D)
							{
							/* drape segment on the fly */
							segment.endpoints[0] = &(data->navs[inav].navpts[jpoint].pointstbd);
							segment.endpoints[1] = &(data->navs[inav].navpts[jpoint].pointcntr);
							mbview_drapesegment(instance, &(segment));

							/* draw the segment */
							for (i=0;i<segment.nls;i++)
								{
								glVertex3f((float)(segment.lspoints[i].xdisplay), 
										(float)(segment.lspoints[i].ydisplay), 
										(float)(segment.lspoints[i].zdisplay));
								}
							}
						else
							{
							glVertex3f((float)(data->navs[inav].navpts[jpoint].pointcntr.xdisplay), 
									(float)(data->navs[inav].navpts[jpoint].pointcntr.ydisplay), 
									(float)(data->navs[inav].navpts[jpoint].pointcntr.zdisplay));
							}

						swathbounds_on = MB_NO;
						glEnd();
						}
					}
				}
			}

		/* deallocate on the fly draping segment */
		if (segment.nls_alloc > 0 
			&& segment.lspoints != NULL)
			{
			status = mb_free(mbv_verbose, &segment.lspoints, &error);
			segment.nls_alloc = 0;
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

