/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_nav.c	10/28/2003
 *    $Id: mbview_nav.c,v 1.1 2003-11-07 00:39:15 caress Exp $
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

static char rcs_id[]="$Id: mbview_nav.c,v 1.1 2003-11-07 00:39:15 caress Exp $";

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
			double	**navportlon,
			double	**navportlat,
			double	**navstbdlon,
			double	**navstbdlat,
			int	**cdp,
			int	**shot,
			int *error)
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
	if (status == MB_SUCCESS && navz != NULL)
		status = mb_realloc(verbose,npointtotal*sizeof(double),navz,error);
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

fprintf(stderr,"Called mbview_addnav:%d %d\n",instance,npoint);

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
			fprintf(stderr,"dbg2       point:%d time_d:%f lon:%f lat:%f z:%f\n", 
					i, time_d[i], navlon[i], navlat[i], navz[i]);
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
	data->nav_selected = MBV_SELECT_NONE;
	
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
				data->navs[i].size = 2;
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
		data->navs[inav].cdp = navcdp;
		data->navs[inav].shot = navshot;

		/* loop over the points in the new nav */
		data->navs[inav].npoints = npoint;
		for (i=0;i<npoint;i++)
			{
			/* set time and shot info */
			data->navs[inav].navpts[i].time_d = time_d[i];
			data->navs[inav].navpts[i].shot = shot[i];
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
				zdata = 0.0;
			data->navs[inav].navpts[i].pointport.zdata = zdata;
			data->navs[inav].navpts[i].pointport.zdisplay 
				= view->zscale * (data->navs[inav].navpts[i].pointport.zdata - view->zorigin);

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
				zdata /= (double)nfound;
			else
				zdata = 0.0;
			data->navs[inav].navpts[i].pointcntr.zdata = zdata;
			data->navs[inav].navpts[i].pointcntr.zdisplay 
				= view->zscale * (data->navs[inav].navpts[i].pointcntr.zdata - view->zorigin);

			/* get starboard swathbound nav positions in grid and display coordinates */
			data->navs[inav].navpts[i].pointstbd.xlon = navportlon[i];
			data->navs[inav].navpts[i].pointstbd.ylat = navportlat[i];
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
				data->navs[inav].navpts[i].pointstbd.zdata /= (double)nfound;
			else
				data->navs[inav].navpts[i].pointstbd.zdata = 0.0;
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
int mbview_drawnav(int instance, int rez)
{
	/* local variables */
	char	*function_name = "mbview_drawnav";
	int	status = MB_SUCCESS;
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
	if (data->nav_mode != MBV_SITE_OFF
		&& data->nav_view_mode == MBV_VIEW_ON
		&& data->nnav > 0)
		{
		/* loop over the navs */
		if (data->nnav == -41)
		for (inav=0;inav<data->nnav;inav++)
			{
			icolor = data->navs[inav].color;
			glLineWidth((float)(data->navs[inav].size));
			glBegin(GL_LINE_STRIP);
			for (jpoint=0;jpoint<data->navs[inav].npoints;jpoint++)
				{
				/* set size and color */
				if (inav == data->nav_selected)
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

		/* loop over the navs */
		for (inav=0;inav<data->nnav;inav++)
			{
			icolor = data->navs[inav].color;
			glLineWidth((float)(data->navs[inav].size));
			glBegin(GL_LINE_STRIP);
			for (jpoint=0;jpoint<data->navs[inav].npoints-1;jpoint++)
				{
				/* set size and color */
				if (inav == data->nav_selected)
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

