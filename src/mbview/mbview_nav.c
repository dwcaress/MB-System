/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_nav.c	10/28/2003
 *    $Id: mbview_nav.c,v 5.7 2005-02-18 07:32:56 caress Exp $
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
 * Revision 5.6  2005/02/17 07:35:08  caress
 * Moving towards 5.0.6 release.
 *
 * Revision 5.5  2005/02/08 22:37:41  caress
 * Heading towards 5.0.6 release.
 *
 * Revision 5.3  2004/12/02 06:36:31  caress
 * Fixes while supporting Reson 7k data.
 *
 * Revision 5.2  2004/09/16 21:44:40  caress
 * Many changes over the summer.
 *
 * Revision 5.1  2004/02/24 22:52:28  caress
 * Added spherical projection to MBview.
 *
 * Revision 5.0  2003/12/02 20:38:33  caress
 * Making version number 5.0
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
#include "MB3DSiteList.h"
#include "MB3DRouteList.h"
#include "MB3DNavList.h"

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
char		value_string[MB_PATH_MAXLINE];

static char rcs_id[]="$Id: mbview_nav.c,v 5.7 2005-02-18 07:32:56 caress Exp $";

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
	*nnav = shared.shareddata.nnav;
		
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
	if (nav >= 0 && nav < shared.shareddata.nnav)
		{
		*npoint = shared.shareddata.navs[nav].npoints;
		for (i=0;i<*npoint-1;i++)
			{
			if (shared.shareddata.navs[nav].segments[i].nls > 2)
				*nintpoint += shared.shareddata.navs[nav].segments[i].nls - 2;
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
	int	nadded;
	int	inav;
	int	i, j;
	int	ii,jj;

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
	shared.shareddata.nav_selected[0] = MBV_SELECT_NONE;
	shared.shareddata.nav_selected[1] = MBV_SELECT_NONE;
	
	/* set nav id so that new nav is created */
	inav = shared.shareddata.nnav;
	
	/* allocate memory for a new nav if required */
	if (shared.shareddata.nnav_alloc < shared.shareddata.nnav + 1)
		{
		shared.shareddata.nnav_alloc = shared.shareddata.nnav + 1;
		status = mb_realloc(mbv_verbose, 
			    	shared.shareddata.nnav_alloc * sizeof(struct mbview_nav_struct),
			    	&(shared.shareddata.navs), error);
		if (status == MB_FAILURE)
			{
			shared.shareddata.nnav_alloc = 0;
			}
		else
			{
			for (i=shared.shareddata.nnav;i<shared.shareddata.nnav_alloc;i++)
				{
				shared.shareddata.navs[i].color = MBV_COLOR_RED;
				shared.shareddata.navs[i].size = 4;
				shared.shareddata.navs[i].name[0] = '\0';
				shared.shareddata.navs[i].swathbounds = MB_NO;
				shared.shareddata.navs[i].shot = MB_NO;
				shared.shareddata.navs[i].cdp = MB_NO;
				shared.shareddata.navs[i].npoints = 0;
				shared.shareddata.navs[i].npoints_alloc = 0;
				shared.shareddata.navs[i].navpts = NULL;
				shared.shareddata.navs[i].segments = NULL;
				}
			}
		}
		
	/* allocate memory to for nav arrays */
	if (shared.shareddata.navs[inav].npoints_alloc < npoint)
		{
		shared.shareddata.navs[inav].npoints_alloc = npoint;
		status = mb_realloc(mbv_verbose, 
			    	shared.shareddata.navs[inav].npoints_alloc * sizeof(struct mbview_navpointw_struct),
			    	&(shared.shareddata.navs[inav].navpts), error);
		status = mb_realloc(mbv_verbose, 
			    	shared.shareddata.navs[inav].npoints_alloc * sizeof(struct mbview_linesegmentw_struct),
			    	&(shared.shareddata.navs[inav].segments), error);
		for (j=0;j<shared.shareddata.navs[inav].npoints_alloc-1;j++)
			{
			shared.shareddata.navs[inav].segments[j].nls = 0;
			shared.shareddata.navs[inav].segments[j].nls_alloc = 0;
			shared.shareddata.navs[inav].segments[j].lspoints = NULL;
			shared.shareddata.navs[inav].segments[j].endpoints[0] = &(shared.shareddata.navs[inav].navpts[j].pointcntr);
			shared.shareddata.navs[inav].segments[j].endpoints[1] = &(shared.shareddata.navs[inav].navpts[j+1].pointcntr);
			}
		}
		
	/* add the new nav */
	if (status == MB_SUCCESS)
		{
		/* set nnav */
		shared.shareddata.nnav++;

		/* set color size and name for new nav */
		shared.shareddata.navs[inav].color = navcolor;
		shared.shareddata.navs[inav].size = navsize;
		strcpy(shared.shareddata.navs[inav].name,navname);
		shared.shareddata.navs[inav].swathbounds = navswathbounds;
		shared.shareddata.navs[inav].shot = navshot;
		shared.shareddata.navs[inav].cdp = navcdp;

		/* loop over the points in the new nav */
		shared.shareddata.navs[inav].npoints = npoint;
		for (i=0;i<npoint;i++)
			{
			/* set status values */
			shared.shareddata.navs[inav].navpts[i].draped = MB_NO;
			shared.shareddata.navs[inav].navpts[i].selected = MB_NO;
			
			/* set time and shot info */
			shared.shareddata.navs[inav].navpts[i].time_d = time_d[i];
			shared.shareddata.navs[inav].navpts[i].heading = heading[i];
			shared.shareddata.navs[inav].navpts[i].speed = speed[i];
			if (shared.shareddata.navs[inav].shot == MB_YES)
				shared.shareddata.navs[inav].navpts[i].shot = shot[i];
			if (shared.shareddata.navs[inav].cdp == MB_YES)
				shared.shareddata.navs[inav].navpts[i].cdp = cdp[i];
			
			/* ************************************************* */
			/* get nav positions in grid and display coordinates */
			shared.shareddata.navs[inav].navpts[i].point.xlon = navlon[i];
			shared.shareddata.navs[inav].navpts[i].point.ylat = navlat[i];
			shared.shareddata.navs[inav].navpts[i].point.zdata = navz[i];
			status = mbview_projectfromlonlat(instance,
					shared.shareddata.navs[inav].navpts[i].point.xlon, 
					shared.shareddata.navs[inav].navpts[i].point.ylat, 
					shared.shareddata.navs[inav].navpts[i].point.zdata, 
					&(shared.shareddata.navs[inav].navpts[i].point.xgrid[instance]), 
					&(shared.shareddata.navs[inav].navpts[i].point.ygrid[instance]),
					&(shared.shareddata.navs[inav].navpts[i].point.xdisplay[instance]), 
					&(shared.shareddata.navs[inav].navpts[i].point.ydisplay[instance]), 
					&(shared.shareddata.navs[inav].navpts[i].point.zdisplay[instance]));
			mbview_updatepointw(instance, &(shared.shareddata.navs[inav].navpts[i].point));

/*fprintf(stderr,"Depth: llz:%f %f %f   grid:%f %f   dpy:%f %f %f\n",
shared.shareddata.navs[inav].navpts[i].point.xlon, 
shared.shareddata.navs[inav].navpts[i].point.ylat, 
shared.shareddata.navs[inav].navpts[i].point.zdata, 
shared.shareddata.navs[inav].navpts[i].point.xgrid[instance], 
shared.shareddata.navs[inav].navpts[i].point.ygrid[instance],
shared.shareddata.navs[inav].navpts[i].point.xdisplay[instance], 
shared.shareddata.navs[inav].navpts[i].point.ydisplay[instance], 
shared.shareddata.navs[inav].navpts[i].point.zdisplay[instance]);*/

			/* ************************************************* */
			/* get center on-bottom nav positions in grid coordinates */
			shared.shareddata.navs[inav].navpts[i].pointcntr.xlon = navlon[i];
			shared.shareddata.navs[inav].navpts[i].pointcntr.ylat = navlat[i];
			status = mbview_projectll2xyzgrid(instance,
					shared.shareddata.navs[inav].navpts[i].pointcntr.xlon, 
					shared.shareddata.navs[inav].navpts[i].pointcntr.ylat, 
					&(shared.shareddata.navs[inav].navpts[i].pointcntr.xgrid[instance]), 
					&(shared.shareddata.navs[inav].navpts[i].pointcntr.ygrid[instance]), 
					&(shared.shareddata.navs[inav].navpts[i].pointcntr.zdata));

			/* get center on-bottom nav positions in display coordinates */
			status = mbview_projectll2display(instance,
					shared.shareddata.navs[inav].navpts[i].pointcntr.xlon, 
					shared.shareddata.navs[inav].navpts[i].pointcntr.ylat, 
					shared.shareddata.navs[inav].navpts[i].pointcntr.zdata, 
					&(shared.shareddata.navs[inav].navpts[i].pointcntr.xdisplay[instance]), 
					&(shared.shareddata.navs[inav].navpts[i].pointcntr.ydisplay[instance]), 
					&(shared.shareddata.navs[inav].navpts[i].pointcntr.zdisplay[instance]));

			/* get center on-bottom nav positions for all active instances */
			mbview_updatepointw(instance, &(shared.shareddata.navs[inav].navpts[i].pointcntr));

			/* ************************************************* */
			/* get port swathbound nav positions in grid and display coordinates */
			shared.shareddata.navs[inav].navpts[i].pointport.xlon = navportlon[i];
			shared.shareddata.navs[inav].navpts[i].pointport.ylat = navportlat[i];
			status = mbview_projectll2xyzgrid(instance,
					shared.shareddata.navs[inav].navpts[i].pointport.xlon, 
					shared.shareddata.navs[inav].navpts[i].pointport.ylat, 
					&(shared.shareddata.navs[inav].navpts[i].pointport.xgrid[instance]), 
					&(shared.shareddata.navs[inav].navpts[i].pointport.ygrid[instance]), 
					&(shared.shareddata.navs[inav].navpts[i].pointport.zdata));

			/* get port on-bottom nav positions in display coordinates */
			status = mbview_projectll2display(instance,
					shared.shareddata.navs[inav].navpts[i].pointport.xlon, 
					shared.shareddata.navs[inav].navpts[i].pointport.ylat, 
					shared.shareddata.navs[inav].navpts[i].pointport.zdata, 
					&(shared.shareddata.navs[inav].navpts[i].pointport.xdisplay[instance]), 
					&(shared.shareddata.navs[inav].navpts[i].pointport.ydisplay[instance]), 
					&(shared.shareddata.navs[inav].navpts[i].pointport.zdisplay[instance]));

			/* get port on-bottom nav positions for all active instances */
			mbview_updatepointw(instance, &(shared.shareddata.navs[inav].navpts[i].pointport));

			/* ************************************************* */
			/* get starboard swathbound nav positions in grid coordinates */
			shared.shareddata.navs[inav].navpts[i].pointstbd.xlon = navstbdlon[i];
			shared.shareddata.navs[inav].navpts[i].pointstbd.ylat = navstbdlat[i];
			status = mbview_projectll2xyzgrid(instance,
					shared.shareddata.navs[inav].navpts[i].pointstbd.xlon, 
					shared.shareddata.navs[inav].navpts[i].pointstbd.ylat, 
					&(shared.shareddata.navs[inav].navpts[i].pointstbd.xgrid[instance]), 
					&(shared.shareddata.navs[inav].navpts[i].pointstbd.ygrid[instance]), 
					&(shared.shareddata.navs[inav].navpts[i].pointstbd.zdata));

			/* get starboard on-bottom nav positions in display coordinates */
			status = mbview_projectll2display(instance,
					shared.shareddata.navs[inav].navpts[i].pointstbd.xlon, 
					shared.shareddata.navs[inav].navpts[i].pointstbd.ylat, 
					shared.shareddata.navs[inav].navpts[i].pointstbd.zdata, 
					&(shared.shareddata.navs[inav].navpts[i].pointstbd.xdisplay[instance]), 
					&(shared.shareddata.navs[inav].navpts[i].pointstbd.ydisplay[instance]), 
					&(shared.shareddata.navs[inav].navpts[i].pointstbd.zdisplay[instance]));

			/* get starboard on-bottom nav positions for all active instances */
			mbview_updatepointw(instance, &(shared.shareddata.navs[inav].navpts[i].pointstbd));

			/* ************************************************* */
			}
			
		/* drape the segments */
		for (i=0;i<shared.shareddata.navs[inav].npoints-1;i++)
			{
			/* drape the segment */
			mbview_drapesegmentw(instance, &(shared.shareddata.navs[inav].segments[i]));
			
			/* update the segment for all active instances */
			mbview_updatesegmentw(instance, &(shared.shareddata.navs[inav].segments[i]));
			}


		/* make navs viewable */
		data->nav_view_mode = MBV_VIEW_ON;

		/* update nav data list */
		mbview_updatenavlist();	
		
		}
	
	/* print nav debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Nav data altered in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Nav values:\n");
		fprintf(stderr,"dbg2       nav_mode:           %d\n",shared.shareddata.nav_mode);
		fprintf(stderr,"dbg2       nav_view_mode:      %d\n",data->nav_view_mode);
		fprintf(stderr,"dbg2       navdrape_view_mode: %d\n",data->navdrape_view_mode);
		fprintf(stderr,"dbg2       nnav:               %d\n",shared.shareddata.nnav);
		fprintf(stderr,"dbg2       nnav_alloc:         %d\n",shared.shareddata.nnav_alloc);
		fprintf(stderr,"dbg2       nav_selected[0]:    %d\n",shared.shareddata.nav_selected[0]);
		fprintf(stderr,"dbg2       nav_selected[1]:    %d\n",shared.shareddata.nav_selected[1]);
		fprintf(stderr,"dbg2       nav_point_selected: %d\n",shared.shareddata.nav_point_selected);
		for (i=0;i<shared.shareddata.nnav;i++)
			{
			fprintf(stderr,"dbg2       nav %d color:         %d\n",i,shared.shareddata.navs[i].color);
			fprintf(stderr,"dbg2       nav %d size:          %d\n",i,shared.shareddata.navs[i].size);
			fprintf(stderr,"dbg2       nav %d name:          %s\n",i,shared.shareddata.navs[i].name);
			fprintf(stderr,"dbg2       nav %d swathbounds:   %d\n",i,shared.shareddata.navs[i].swathbounds);
			fprintf(stderr,"dbg2       nav %d shot:          %d\n",i,shared.shareddata.navs[i].shot);
			fprintf(stderr,"dbg2       nav %d cdp:           %d\n",i,shared.shareddata.navs[i].cdp);
			fprintf(stderr,"dbg2       nav %d npoints:       %d\n",i,shared.shareddata.navs[i].npoints);
			fprintf(stderr,"dbg2       nav %d npoints_alloc: %d\n",i,shared.shareddata.navs[i].npoints_alloc);
			fprintf(stderr,"dbg2       nav %d nselected:     %d\n",i,shared.shareddata.navs[i].nselected);
			for (j=0;j<shared.shareddata.navs[i].npoints;j++)
				{
				fprintf(stderr,"dbg2       nav %d %d draped:   %d\n",i,j,shared.shareddata.navs[i].navpts[j].draped);
				fprintf(stderr,"dbg2       nav %d %d selected: %d\n",i,j,shared.shareddata.navs[i].navpts[j].selected);
				fprintf(stderr,"dbg2       nav %d %d time_d:   %f\n",i,j,shared.shareddata.navs[i].navpts[j].time_d);
				fprintf(stderr,"dbg2       nav %d %d heading:  %f\n",i,j,shared.shareddata.navs[i].navpts[j].heading);
				fprintf(stderr,"dbg2       nav %d %d speed:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].speed);
				fprintf(stderr,"dbg2       nav %d %d shot:     %d\n",i,j,shared.shareddata.navs[i].navpts[j].shot);
				fprintf(stderr,"dbg2       nav %d %d cdp:      %d\n",i,j,shared.shareddata.navs[i].navpts[j].cdp);

				fprintf(stderr,"dbg2       nav %d %d xgrid:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].point.xgrid[instance]);
				fprintf(stderr,"dbg2       nav %d %d ygrid:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].point.ygrid[instance]);
				fprintf(stderr,"dbg2       nav %d %d xlon:     %f\n",i,j,shared.shareddata.navs[i].navpts[j].point.xlon);
				fprintf(stderr,"dbg2       nav %d %d ylat:     %f\n",i,j,shared.shareddata.navs[i].navpts[j].point.ylat);
				fprintf(stderr,"dbg2       nav %d %d zdata:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].point.zdata);
				fprintf(stderr,"dbg2       nav %d %d xdisplay: %f\n",i,j,shared.shareddata.navs[i].navpts[j].point.xdisplay[instance]);
				fprintf(stderr,"dbg2       nav %d %d ydisplay: %f\n",i,j,shared.shareddata.navs[i].navpts[j].point.ydisplay[instance]);
				fprintf(stderr,"dbg2       nav %d %d zdisplay: %f\n",i,j,shared.shareddata.navs[i].navpts[j].point.zdisplay[instance]);

				fprintf(stderr,"dbg2       nav %d %d stbd xgrid:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointport.xgrid[instance]);
				fprintf(stderr,"dbg2       nav %d %d stbd ygrid:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointport.ygrid[instance]);
				fprintf(stderr,"dbg2       nav %d %d stbd xlon:     %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointport.xlon);
				fprintf(stderr,"dbg2       nav %d %d stbd ylat:     %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointport.ylat);
				fprintf(stderr,"dbg2       nav %d %d stbd zdata:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointport.zdata);
				fprintf(stderr,"dbg2       nav %d %d stbd xdisplay: %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointport.xdisplay[instance]);
				fprintf(stderr,"dbg2       nav %d %d stbd ydisplay: %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointport.ydisplay[instance]);
				fprintf(stderr,"dbg2       nav %d %d stbd zdisplay: %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointport.zdisplay[instance]);

				fprintf(stderr,"dbg2       nav %d %d cntr xgrid:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointcntr.xgrid[instance]);
				fprintf(stderr,"dbg2       nav %d %d cntr ygrid:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointcntr.ygrid[instance]);
				fprintf(stderr,"dbg2       nav %d %d cntr xlon:     %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointcntr.xlon);
				fprintf(stderr,"dbg2       nav %d %d cntr ylat:     %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointcntr.ylat);
				fprintf(stderr,"dbg2       nav %d %d cntr zdata:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointcntr.zdata);
				fprintf(stderr,"dbg2       nav %d %d cntr xdisplay: %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointcntr.xdisplay[instance]);
				fprintf(stderr,"dbg2       nav %d %d cntr ydisplay: %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointcntr.ydisplay[instance]);
				fprintf(stderr,"dbg2       nav %d %d cntr zdisplay: %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointcntr.zdisplay[instance]);

				fprintf(stderr,"dbg2       nav %d %d port xgrid:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointstbd.xgrid[instance]);
				fprintf(stderr,"dbg2       nav %d %d port ygrid:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointstbd.ygrid[instance]);
				fprintf(stderr,"dbg2       nav %d %d port xlon:     %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointstbd.xlon);
				fprintf(stderr,"dbg2       nav %d %d port ylat:     %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointstbd.ylat);
				fprintf(stderr,"dbg2       nav %d %d port zdata:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointstbd.zdata);
				fprintf(stderr,"dbg2       nav %d %d port xdisplay: %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointstbd.xdisplay[instance]);
				fprintf(stderr,"dbg2       nav %d %d port ydisplay: %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointstbd.ydisplay[instance]);
				fprintf(stderr,"dbg2       nav %d %d port zdisplay: %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointstbd.zdisplay[instance]);
				}
			for (j=0;j<shared.shareddata.navs[i].npoints-1;j++)
				{
				fprintf(stderr,"dbg2       nav %d %d nls:          %d\n",i,j,shared.shareddata.navs[i].segments[j].nls);
				fprintf(stderr,"dbg2       nav %d %d nls_alloc:    %d\n",i,j,shared.shareddata.navs[i].segments[j].nls_alloc);
				fprintf(stderr,"dbg2       nav %d %d endpoints[0]: %d\n",i,j,shared.shareddata.navs[i].segments[j].endpoints[0]);
				fprintf(stderr,"dbg2       nav %d %d endpoints[1]: %d\n",i,j,shared.shareddata.navs[i].segments[j].endpoints[1]);
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
        shared.shareddata.nav_mode = MBV_NAV_VIEW;
		
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
	int	ii,jj;

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
	if (shared.shareddata.nav_mode != MBV_NAV_OFF
		&& shared.shareddata.nnav > 0
		&& (which == MBV_PICK_DOWN
			|| shared.shareddata.nav_selected[0] == MBV_SELECT_NONE))
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
			shared.shareddata.nav_selected[0] = MBV_SELECT_NONE;
			shared.shareddata.nav_point_selected[0] = MBV_SELECT_NONE;
			shared.shareddata.nav_selected[1] = MBV_SELECT_NONE;
			shared.shareddata.nav_point_selected[1] = MBV_SELECT_NONE;

			for (i=0;i<shared.shareddata.nnav;i++)
				{
				for (j=0;j<shared.shareddata.navs[i].npoints;j++)
					{
					xx = xgrid - shared.shareddata.navs[i].navpts[j].point.xgrid[instance];
					yy = ygrid - shared.shareddata.navs[i].navpts[j].point.ygrid[instance];
					rr = sqrt(xx * xx + yy * yy);
					if (rr < rrmin)
						{
						rrmin = rr;
						shared.shareddata.nav_selected[0] = i;
						shared.shareddata.nav_point_selected[0] = j;
						}
					}
				}
				
			/* set pick location */
			data->pickinfo_mode = MBV_PICK_NAV;
			shared.shareddata.navpick_type = MBV_PICK_ONEPOINT;
			shared.shareddata.navpick.endpoints[0].xgrid[instance] 
				= shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].point.xgrid[instance];
			shared.shareddata.navpick.endpoints[0].ygrid[instance] 
				= shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].point.ygrid[instance];
			shared.shareddata.navpick.endpoints[0].xlon 
				= shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].point.xlon;
			shared.shareddata.navpick.endpoints[0].ylat 
				= shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].point.ylat;
			shared.shareddata.navpick.endpoints[0].zdata 
				= shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].point.zdata;
			shared.shareddata.navpick.endpoints[0].xdisplay[instance] 
				= shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].point.xdisplay[instance];
			shared.shareddata.navpick.endpoints[0].ydisplay[instance] 
				= shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].point.ydisplay[instance];
			shared.shareddata.navpick.endpoints[0].zdisplay[instance] 
				= shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].point.zdisplay[instance];

			/* get pick positions for all active instances */
			mbview_updatepointw(instance, &(shared.shareddata.navpick.endpoints[0]));
			
			/* generate 3D drape of pick marks  */
			mbview_navpicksize(instance);
			}
		else
			{
			/* unselect nav pick */
			data->pickinfo_mode = data->pick_type;
			shared.shareddata.navpick_type = MBV_PICK_NONE;
			shared.shareddata.nav_selected[0] = MBV_SELECT_NONE;
			shared.shareddata.nav_point_selected[0] = MBV_SELECT_NONE;
			XBell(view->dpy,100);
			}
		}
	
	/* only select two nav points if enabled */
	else if (shared.shareddata.nav_mode != MBV_NAV_OFF
		&& shared.shareddata.nnav > 0
		&& (which == MBV_PICK_MOVE
			&& shared.shareddata.nav_selected[0] != MBV_SELECT_NONE))
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
			shared.shareddata.nav_selected[1] = MBV_SELECT_NONE;
			shared.shareddata.nav_point_selected[1] = MBV_SELECT_NONE;

			for (i=0;i<shared.shareddata.nnav;i++)
				{
				for (j=0;j<shared.shareddata.navs[i].npoints;j++)
					{
					xx = xgrid - shared.shareddata.navs[i].navpts[j].point.xgrid[instance];
					yy = ygrid - shared.shareddata.navs[i].navpts[j].point.ygrid[instance];
					rr = sqrt(xx * xx + yy * yy);
					if (rr < rrmin)
						{
						rrmin = rr;
						shared.shareddata.nav_selected[1] = i;
						shared.shareddata.nav_point_selected[1] = j;
						}
					}
				}
				
			/* set pick location */
			data->pickinfo_mode = MBV_PICK_NAV;
			shared.shareddata.navpick_type = MBV_PICK_TWOPOINT;
			shared.shareddata.navpick.endpoints[1].xgrid[instance] 
				= shared.shareddata.navs[shared.shareddata.nav_selected[1]].navpts[shared.shareddata.nav_point_selected[1]].point.xgrid[instance];
			shared.shareddata.navpick.endpoints[1].ygrid[instance] 
				= shared.shareddata.navs[shared.shareddata.nav_selected[1]].navpts[shared.shareddata.nav_point_selected[1]].point.ygrid[instance];
			shared.shareddata.navpick.endpoints[1].xlon 
				= shared.shareddata.navs[shared.shareddata.nav_selected[1]].navpts[shared.shareddata.nav_point_selected[1]].point.xlon;
			shared.shareddata.navpick.endpoints[1].ylat 
				= shared.shareddata.navs[shared.shareddata.nav_selected[1]].navpts[shared.shareddata.nav_point_selected[1]].point.ylat;
			shared.shareddata.navpick.endpoints[1].zdata 
				= shared.shareddata.navs[shared.shareddata.nav_selected[1]].navpts[shared.shareddata.nav_point_selected[1]].point.zdata;
			shared.shareddata.navpick.endpoints[1].xdisplay[instance] 
				= shared.shareddata.navs[shared.shareddata.nav_selected[1]].navpts[shared.shareddata.nav_point_selected[1]].point.xdisplay[instance];
			shared.shareddata.navpick.endpoints[1].ydisplay[instance] 
				= shared.shareddata.navs[shared.shareddata.nav_selected[1]].navpts[shared.shareddata.nav_point_selected[1]].point.ydisplay[instance];
			shared.shareddata.navpick.endpoints[1].zdisplay[instance] 
				= shared.shareddata.navs[shared.shareddata.nav_selected[1]].navpts[shared.shareddata.nav_point_selected[1]].point.zdisplay[instance];

			/* get pick positions for all active instances */
			mbview_updatepointw(instance, &(shared.shareddata.navpick.endpoints[1]));
			
			/* generate 3D drape of pick marks */
			mbview_navpicksize(instance);
			}
		}
	
	/* only select or deselect range of nav points if enabled and two different points selected */
	else if (shared.shareddata.nav_mode != MBV_NAV_OFF
		&& shared.shareddata.nnav > 0
		&& which == MBV_PICK_UP)
		{
		/* only actually select range of nav if two different points have been selected */
		if (shared.shareddata.nav_selected[0] != MBV_SELECT_NONE
			&& shared.shareddata.nav_selected[1] != MBV_SELECT_NONE
			&& !(shared.shareddata.nav_selected[0] == shared.shareddata.nav_selected[1]
				&& shared.shareddata.nav_point_selected[0] == shared.shareddata.nav_point_selected[1]))
			{
			/* get order of selected nav points */
			inav0 = MIN(shared.shareddata.nav_selected[0], shared.shareddata.nav_selected[1]);
			inav1 = MAX(shared.shareddata.nav_selected[0], shared.shareddata.nav_selected[1]);
			if (inav0 == inav1)
				{
				jpt0 = MIN(shared.shareddata.nav_point_selected[0], shared.shareddata.nav_point_selected[1]);
				jpt1 = MAX(shared.shareddata.nav_point_selected[0], shared.shareddata.nav_point_selected[1]);
				}
			else if (shared.shareddata.nav_selected[0] <= shared.shareddata.nav_selected[1])
				{
				jpt0 = shared.shareddata.nav_point_selected[0];
				jpt1 = shared.shareddata.nav_point_selected[1];
				}
			else
				{
				jpt0 = shared.shareddata.nav_point_selected[1];
				jpt1 = shared.shareddata.nav_point_selected[0];
				}

			/* loop over the affected nav */
			for (inav=inav0;inav<=inav1;inav++)
				{
				if (inav == inav0)
					jj0 = MIN(jpt0, shared.shareddata.navs[inav].npoints - 1);
				else
					jj0 = 0;
				if (inav == inav1)
					jj1 = MAX(jpt1, 0);
				else
					jj1 = shared.shareddata.navs[inav].npoints;
				for (jpt=jj0;jpt<=jj1;jpt++)
					{
					shared.shareddata.navs[inav].navpts[jpt].selected = select;
					}
				shared.shareddata.navs[inav].nselected = 0;
				for (jpt=0;jpt<shared.shareddata.navs[inav].npoints;jpt++)
					{
					if (shared.shareddata.navs[inav].navpts[jpt].selected == MB_YES)
						shared.shareddata.navs[inav].nselected++;
					}
				}
			}
		}

	/* else beep */
	else
		{
		shared.shareddata.nav_selected[0] = MBV_SELECT_NONE;
		shared.shareddata.nav_point_selected[0] = MBV_SELECT_NONE;
		shared.shareddata.nav_selected[1] = MBV_SELECT_NONE;
		shared.shareddata.nav_point_selected[1] = MBV_SELECT_NONE;
		XBell(view->dpy,100);
		for (i=0;i<shared.shareddata.nnav;i++)
			{
			for (j=0;j<shared.shareddata.navs[i].npoints;j++)
				{
				shared.shareddata.navs[i].navpts[j].selected = MB_NO;
				}
			}
		}
		
	/* set what kind of pick to annotate */
	if (shared.shareddata.nav_selected[0] != MBV_SELECT_NONE)
		{
		data->pickinfo_mode = MBV_PICK_NAV;
		}
	else
		{
		data->pickinfo_mode = data->pick_type;
		}

	/* update nav data list */
	mbview_updatenavlist();	
	
	/* set pick annotation */
	mbview_pick_text(instance);
	
	/* print nav debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Nav data altered in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Nav values:\n");
		fprintf(stderr,"dbg2       nav_mode:              %d\n",shared.shareddata.nav_mode);
		fprintf(stderr,"dbg2       nav_view_mode:         %d\n",data->nav_view_mode);
		fprintf(stderr,"dbg2       navdrape_view_mode:    %d\n",data->navdrape_view_mode);
		fprintf(stderr,"dbg2       nnav:                  %d\n",shared.shareddata.nnav);
		fprintf(stderr,"dbg2       nnav_alloc:            %d\n",shared.shareddata.nnav_alloc);
		fprintf(stderr,"dbg2       nav_selected[0]:       %d\n",shared.shareddata.nav_selected[0]);
		fprintf(stderr,"dbg2       nav_point_selected[0]: %d\n",shared.shareddata.nav_point_selected[0]);
		fprintf(stderr,"dbg2       nav_selected[1]:       %d\n",shared.shareddata.nav_selected[1]);
		fprintf(stderr,"dbg2       nav_point_selected[1]: %d\n",shared.shareddata.nav_point_selected[1]);
		for (i=0;i<shared.shareddata.nnav;i++)
			{
			fprintf(stderr,"dbg2       nav %d color:         %d\n",i,shared.shareddata.navs[i].color);
			fprintf(stderr,"dbg2       nav %d size:          %d\n",i,shared.shareddata.navs[i].size);
			fprintf(stderr,"dbg2       nav %d name:          %s\n",i,shared.shareddata.navs[i].name);
			fprintf(stderr,"dbg2       nav %d swathbounds:   %d\n",i,shared.shareddata.navs[i].swathbounds);
			fprintf(stderr,"dbg2       nav %d shot:          %d\n",i,shared.shareddata.navs[i].shot);
			fprintf(stderr,"dbg2       nav %d cdp:           %d\n",i,shared.shareddata.navs[i].cdp);
			fprintf(stderr,"dbg2       nav %d npoints:       %d\n",i,shared.shareddata.navs[i].npoints);
			fprintf(stderr,"dbg2       nav %d npoints_alloc: %d\n",i,shared.shareddata.navs[i].npoints_alloc);
			fprintf(stderr,"dbg2       nav %d nselected:     %d\n",i,shared.shareddata.navs[i].nselected);
			for (j=0;j<shared.shareddata.navs[i].npoints;j++)
				{
				fprintf(stderr,"dbg2       nav %d %d draped:   %d\n",i,j,shared.shareddata.navs[i].navpts[j].draped);
				fprintf(stderr,"dbg2       nav %d %d selected: %d\n",i,j,shared.shareddata.navs[i].navpts[j].selected);
				fprintf(stderr,"dbg2       nav %d %d time_d:   %f\n",i,j,shared.shareddata.navs[i].navpts[j].time_d);
				fprintf(stderr,"dbg2       nav %d %d heading:  %f\n",i,j,shared.shareddata.navs[i].navpts[j].heading);
				fprintf(stderr,"dbg2       nav %d %d speed:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].speed);
				fprintf(stderr,"dbg2       nav %d %d shot:     %d\n",i,j,shared.shareddata.navs[i].navpts[j].shot);
				fprintf(stderr,"dbg2       nav %d %d cdp:      %d\n",i,j,shared.shareddata.navs[i].navpts[j].cdp);

				fprintf(stderr,"dbg2       nav %d %d xgrid:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].point.xgrid[instance]);
				fprintf(stderr,"dbg2       nav %d %d ygrid:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].point.ygrid[instance]);
				fprintf(stderr,"dbg2       nav %d %d xlon:     %f\n",i,j,shared.shareddata.navs[i].navpts[j].point.xlon);
				fprintf(stderr,"dbg2       nav %d %d ylat:     %f\n",i,j,shared.shareddata.navs[i].navpts[j].point.ylat);
				fprintf(stderr,"dbg2       nav %d %d zdata:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].point.zdata);
				fprintf(stderr,"dbg2       nav %d %d xdisplay: %f\n",i,j,shared.shareddata.navs[i].navpts[j].point.xdisplay[instance]);
				fprintf(stderr,"dbg2       nav %d %d ydisplay: %f\n",i,j,shared.shareddata.navs[i].navpts[j].point.ydisplay[instance]);
				fprintf(stderr,"dbg2       nav %d %d zdisplay: %f\n",i,j,shared.shareddata.navs[i].navpts[j].point.zdisplay[instance]);

				fprintf(stderr,"dbg2       nav %d %d stbd xgrid:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointport.xgrid);
				fprintf(stderr,"dbg2       nav %d %d stbd ygrid:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointport.ygrid);
				fprintf(stderr,"dbg2       nav %d %d stbd xlon:     %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointport.xlon);
				fprintf(stderr,"dbg2       nav %d %d stbd ylat:     %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointport.ylat);
				fprintf(stderr,"dbg2       nav %d %d stbd zdata:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointport.zdata);
				fprintf(stderr,"dbg2       nav %d %d stbd xdisplay: %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointport.xdisplay);
				fprintf(stderr,"dbg2       nav %d %d stbd ydisplay: %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointport.ydisplay);
				fprintf(stderr,"dbg2       nav %d %d stbd zdisplay: %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointport.zdisplay);

				fprintf(stderr,"dbg2       nav %d %d cntr xgrid:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointcntr.xgrid);
				fprintf(stderr,"dbg2       nav %d %d cntr ygrid:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointcntr.ygrid);
				fprintf(stderr,"dbg2       nav %d %d cntr xlon:     %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointcntr.xlon);
				fprintf(stderr,"dbg2       nav %d %d cntr ylat:     %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointcntr.ylat);
				fprintf(stderr,"dbg2       nav %d %d cntr zdata:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointcntr.zdata);
				fprintf(stderr,"dbg2       nav %d %d cntr xdisplay: %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointcntr.xdisplay);
				fprintf(stderr,"dbg2       nav %d %d cntr ydisplay: %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointcntr.ydisplay);
				fprintf(stderr,"dbg2       nav %d %d cntr zdisplay: %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointcntr.zdisplay);

				fprintf(stderr,"dbg2       nav %d %d port xgrid:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointstbd.xgrid);
				fprintf(stderr,"dbg2       nav %d %d port ygrid:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointstbd.ygrid);
				fprintf(stderr,"dbg2       nav %d %d port xlon:     %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointstbd.xlon);
				fprintf(stderr,"dbg2       nav %d %d port ylat:     %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointstbd.ylat);
				fprintf(stderr,"dbg2       nav %d %d port zdata:    %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointstbd.zdata);
				fprintf(stderr,"dbg2       nav %d %d port xdisplay: %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointstbd.xdisplay);
				fprintf(stderr,"dbg2       nav %d %d port ydisplay: %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointstbd.ydisplay);
				fprintf(stderr,"dbg2       nav %d %d port zdisplay: %f\n",i,j,shared.shareddata.navs[i].navpts[j].pointstbd.zdisplay);
				}
			for (j=0;j<shared.shareddata.navs[i].npoints-1;j++)
				{
				fprintf(stderr,"dbg2       nav %d %d nls:          %d\n",i,j,shared.shareddata.navs[i].segments[j].nls);
				fprintf(stderr,"dbg2       nav %d %d nls_alloc:    %d\n",i,j,shared.shareddata.navs[i].segments[j].nls_alloc);
				fprintf(stderr,"dbg2       nav %d %d endpoints[0]: %d\n",i,j,shared.shareddata.navs[i].segments[j].endpoints[0]);
				fprintf(stderr,"dbg2       nav %d %d endpoints[1]: %d\n",i,j,shared.shareddata.navs[i].segments[j].endpoints[1]);
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
int mbview_nav_delete(int instance, int inav)
{

	/* local variables */
	char	*function_name = "mbview_nav_delete";
	int	error = MB_ERROR_NO_ERROR;
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	i;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       inav:            %d\n",inav);
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* delete nav if its the same as previously selected */
	if (inav >= 0 && inav < shared.shareddata.nnav)
		{
		/* free memory for deleted nav */
		mb_free(mbv_verbose, 
			    	&(shared.shareddata.navs[inav].navpts), &error);
		mb_free(mbv_verbose, 
			    	&(shared.shareddata.navs[inav].segments), &error);

		/* move nav data if necessary */
		for (i=inav;i<shared.shareddata.nnav-1;i++)
			{
			shared.shareddata.navs[i] = shared.shareddata.navs[i+1];
			}
			
		/* rest last nav */
		shared.shareddata.navs[shared.shareddata.nnav-1].color = MBV_COLOR_RED;
		shared.shareddata.navs[shared.shareddata.nnav-1].size = 4;
		shared.shareddata.navs[shared.shareddata.nnav-1].name[0] = '\0';
		shared.shareddata.navs[shared.shareddata.nnav-1].swathbounds = MB_NO;
		shared.shareddata.navs[shared.shareddata.nnav-1].shot = MB_NO;
		shared.shareddata.navs[shared.shareddata.nnav-1].cdp = MB_NO;
		shared.shareddata.navs[shared.shareddata.nnav-1].npoints = 0;
		shared.shareddata.navs[shared.shareddata.nnav-1].npoints_alloc = 0;
		shared.shareddata.navs[shared.shareddata.nnav-1].navpts = NULL;
		shared.shareddata.navs[shared.shareddata.nnav-1].segments = NULL;

		/* set nnav */
		shared.shareddata.nnav--;

		/* no selection */
		shared.shareddata.navpick_type != MBV_PICK_NONE;
		shared.shareddata.nav_selected[0] = MBV_SELECT_NONE;
		shared.shareddata.nav_selected[1] = MBV_SELECT_NONE;
		}
	else
		{
		status = MB_FAILURE;
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
	int	i, j;
	int	ii,jj;

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
	if (shared.shareddata.navpick_type != MBV_PICK_NONE)
		{
		/* set size of 'V' marks in gl units for 3D case */
		xlength = 0.05;
		headingx = sin(shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].heading * DTR);
		headingy = cos(shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].heading * DTR);

		/* set navpick location V marker */
		shared.shareddata.navpick.xpoints[0].xdisplay[instance] = shared.shareddata.navpick.endpoints[0].xdisplay[instance] 
							+ xlength * (headingy - headingx);
		shared.shareddata.navpick.xpoints[0].ydisplay[instance] = shared.shareddata.navpick.endpoints[0].ydisplay[instance] 
							- xlength * (headingx + headingy);
		shared.shareddata.navpick.xpoints[1].xdisplay[instance] = shared.shareddata.navpick.endpoints[0].xdisplay[instance];
		shared.shareddata.navpick.xpoints[1].ydisplay[instance] = shared.shareddata.navpick.endpoints[0].ydisplay[instance];
		shared.shareddata.navpick.xpoints[2].xdisplay[instance] = shared.shareddata.navpick.endpoints[0].xdisplay[instance];
		shared.shareddata.navpick.xpoints[2].ydisplay[instance] = shared.shareddata.navpick.endpoints[0].ydisplay[instance];
		shared.shareddata.navpick.xpoints[3].xdisplay[instance] = shared.shareddata.navpick.endpoints[0].xdisplay[instance] 
							- xlength * (headingx + headingy);
		shared.shareddata.navpick.xpoints[3].ydisplay[instance] = shared.shareddata.navpick.endpoints[0].ydisplay[instance] 
							+ xlength * (headingx - headingy);
		for (i=0;i<4;i++)
			{
			mbview_projectinverse(instance, MB_YES,
				shared.shareddata.navpick.xpoints[i].xdisplay[instance], 
				shared.shareddata.navpick.xpoints[i].ydisplay[instance],
				shared.shareddata.navpick.xpoints[i].zdisplay[instance],
				&shared.shareddata.navpick.xpoints[i].xlon, 
				&shared.shareddata.navpick.xpoints[i].ylat, 
				&shared.shareddata.navpick.xpoints[i].xgrid[instance], 
				&shared.shareddata.navpick.xpoints[i].ygrid[instance]);
			mbview_getzdata(instance, 
				shared.shareddata.navpick.xpoints[i].xgrid[instance], 
				shared.shareddata.navpick.xpoints[i].ygrid[instance],
				&found, &shared.shareddata.navpick.xpoints[i].zdata);
			if (found == MB_NO)
				shared.shareddata.navpick.xpoints[i].zdata 
					= shared.shareddata.navpick.endpoints[0].zdata;
			mbview_projectll2display(instance,
				shared.shareddata.navpick.xpoints[i].xlon, 
				shared.shareddata.navpick.xpoints[i].ylat, 
				shared.shareddata.navpick.xpoints[i].zdata ,
				&shared.shareddata.navpick.xpoints[i].xdisplay[instance], 
				&shared.shareddata.navpick.xpoints[i].ydisplay[instance],
				&shared.shareddata.navpick.xpoints[i].zdisplay[instance]);
			mbview_updatepointw(instance, &(shared.shareddata.navpick.xpoints[i]));
			}

		/* drape the V marker line segments */
		for (j=0;j<2;j++)
			{
			mbview_drapesegmentw(instance, &(shared.shareddata.navpick.xsegments[j]));
			mbview_updatesegmentw(instance, &(shared.shareddata.navpick.xsegments[j]));
			}
		}
	if (shared.shareddata.navpick_type == MBV_PICK_TWOPOINT)
		{
		headingx = sin(shared.shareddata.navs[shared.shareddata.nav_selected[1]].navpts[shared.shareddata.nav_point_selected[1]].heading * DTR);
		headingy = cos(shared.shareddata.navs[shared.shareddata.nav_selected[1]].navpts[shared.shareddata.nav_point_selected[1]].heading * DTR);

		/* set navpick location V marker */
		shared.shareddata.navpick.xpoints[4].xdisplay[instance] = shared.shareddata.navpick.endpoints[1].xdisplay[instance] 
							+ xlength * (headingy - headingx);
		shared.shareddata.navpick.xpoints[4].ydisplay[instance] = shared.shareddata.navpick.endpoints[1].ydisplay[instance] 
							- xlength * (headingx + headingy);
		shared.shareddata.navpick.xpoints[5].xdisplay[instance] = shared.shareddata.navpick.endpoints[1].xdisplay[instance];
		shared.shareddata.navpick.xpoints[5].ydisplay[instance] = shared.shareddata.navpick.endpoints[1].ydisplay[instance];
		shared.shareddata.navpick.xpoints[6].xdisplay[instance] = shared.shareddata.navpick.endpoints[1].xdisplay[instance];
		shared.shareddata.navpick.xpoints[6].ydisplay[instance] = shared.shareddata.navpick.endpoints[1].ydisplay[instance];
		shared.shareddata.navpick.xpoints[7].xdisplay[instance] = shared.shareddata.navpick.endpoints[1].xdisplay[instance] 
							- xlength * (headingx + headingy);
		shared.shareddata.navpick.xpoints[7].ydisplay[instance] = shared.shareddata.navpick.endpoints[1].ydisplay[instance] 
							+ xlength * (headingx - headingy);
		for (i=4;i<8;i++)
			{
			mbview_projectinverse(instance, MB_YES,
				shared.shareddata.navpick.xpoints[i].xdisplay[instance], 
				shared.shareddata.navpick.xpoints[i].ydisplay[instance],
				shared.shareddata.navpick.xpoints[i].zdisplay[instance],
				&shared.shareddata.navpick.xpoints[i].xlon, 
				&shared.shareddata.navpick.xpoints[i].ylat, 
				&shared.shareddata.navpick.xpoints[i].xgrid[instance], 
				&shared.shareddata.navpick.xpoints[i].ygrid[instance]);
			mbview_getzdata(instance, 
				shared.shareddata.navpick.xpoints[i].xgrid[instance], 
				shared.shareddata.navpick.xpoints[i].ygrid[instance],
				&found, &shared.shareddata.navpick.xpoints[i].zdata);
			if (found == MB_NO)
				shared.shareddata.navpick.xpoints[i].zdata 
					= shared.shareddata.navpick.endpoints[1].zdata;
			shared.shareddata.navpick.xpoints[i].zdisplay[instance] = view->scale 
				* (data->exageration * shared.shareddata.navpick.xpoints[i].zdata - view->zorigin);
			mbview_projectll2display(instance,
				shared.shareddata.navpick.xpoints[i].xlon, 
				shared.shareddata.navpick.xpoints[i].ylat, 
				shared.shareddata.navpick.xpoints[i].zdata ,
				&shared.shareddata.navpick.xpoints[i].xdisplay[instance], 
				&shared.shareddata.navpick.xpoints[i].ydisplay[instance],
				&shared.shareddata.navpick.xpoints[i].zdisplay[instance]);
			mbview_updatepointw(instance, &(shared.shareddata.navpick.xpoints[i]));
			}

		/* drape the V marker line segments */
		for (j=2;j<4;j++)
			{
			mbview_drapesegmentw(instance, &(shared.shareddata.navpick.xsegments[j]));
			mbview_updatesegmentw(instance, &(shared.shareddata.navpick.xsegments[j]));
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
	if (shared.shareddata.navpick_type != MBV_PICK_NONE
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
			&& shared.shareddata.navpick.xsegments[0].nls > 0 
			&& shared.shareddata.navpick.xsegments[1].nls > 0)
			{
			glBegin(GL_LINE_STRIP);
			for (i=0;i<shared.shareddata.navpick.xsegments[0].nls;i++)
				{
				glVertex3f((float)(shared.shareddata.navpick.xsegments[0].lspoints[i].xdisplay[instance]), 
						(float)(shared.shareddata.navpick.xsegments[0].lspoints[i].ydisplay[instance]), 
						(float)(shared.shareddata.navpick.xsegments[0].lspoints[i].zdisplay[instance]));
				}
			glEnd();
			glBegin(GL_LINE_STRIP);
			for (i=0;i<shared.shareddata.navpick.xsegments[1].nls;i++)
				{
				glVertex3f((float)(shared.shareddata.navpick.xsegments[1].lspoints[i].xdisplay[instance]), 
						(float)(shared.shareddata.navpick.xsegments[1].lspoints[i].ydisplay[instance]), 
						(float)(shared.shareddata.navpick.xsegments[1].lspoints[i].zdisplay[instance]));
				}
			glEnd();
			}
		else if (data->display_mode == MBV_DISPLAY_3D)
			{
			glBegin(GL_LINES);
			for (i=0;i<4;i++)
				{
				glVertex3f((float)(shared.shareddata.navpick.xpoints[i].xdisplay[instance]), 
					(float)(shared.shareddata.navpick.xpoints[i].ydisplay[instance]), 
					(float)(shared.shareddata.navpick.xpoints[i].zdisplay[instance]));
				}
			glEnd();
			}
		else
			{
			glBegin(GL_LINES);
			glVertex3f((float)(shared.shareddata.navpick.xpoints[0].xdisplay[instance]), 
				(float)(shared.shareddata.navpick.xpoints[0].ydisplay[instance]), 
				(float)(shared.shareddata.navpick.xpoints[0].zdisplay[instance]));
			glVertex3f((float)(shared.shareddata.navpick.xpoints[1].xdisplay[instance]), 
				(float)(shared.shareddata.navpick.xpoints[1].ydisplay[instance]), 
				(float)(shared.shareddata.navpick.xpoints[1].zdisplay[instance]));
			glVertex3f((float)(shared.shareddata.navpick.xpoints[2].xdisplay[instance]), 
				(float)(shared.shareddata.navpick.xpoints[2].ydisplay[instance]), 
				(float)(shared.shareddata.navpick.xpoints[2].zdisplay[instance]));
			glVertex3f((float)(shared.shareddata.navpick.xpoints[3].xdisplay[instance]), 
				(float)(shared.shareddata.navpick.xpoints[3].ydisplay[instance]), 
				(float)(shared.shareddata.navpick.xpoints[3].zdisplay[instance]));
			glEnd();
			}
			
		/* draw first navpick point undraped */
		if (data->display_mode == MBV_DISPLAY_3D
			&& data->nav_view_mode == MBV_VIEW_ON)
			{
			inav = shared.shareddata.nav_selected[0];
			jpt = shared.shareddata.nav_point_selected[0];
			zdisplay = shared.shareddata.navs[inav].navpts[jpt].point.zdisplay[instance];
			glColor3f(1.0, 0.0, 0.0);
			glBegin(GL_LINES);
			for (i=0;i<4;i++)
				{
				glVertex3f((float)(shared.shareddata.navpick.xpoints[i].xdisplay[instance]), 
					(float)(shared.shareddata.navpick.xpoints[i].ydisplay[instance]), 
					zdisplay);
				}
			glEnd();
			}
		
		if (shared.shareddata.navpick_type == MBV_PICK_TWOPOINT)
			{
			/* plot second navpick point draped */
			if (data->display_mode == MBV_DISPLAY_3D 
				&& shared.shareddata.navpick.xsegments[2].nls > 0 
				&& shared.shareddata.navpick.xsegments[3].nls > 0)
				{
				glBegin(GL_LINE_STRIP);
				for (i=0;i<shared.shareddata.navpick.xsegments[2].nls;i++)
					{
					glVertex3f((float)(shared.shareddata.navpick.xsegments[2].lspoints[i].xdisplay[instance]), 
							(float)(shared.shareddata.navpick.xsegments[2].lspoints[i].ydisplay[instance]), 
							(float)(shared.shareddata.navpick.xsegments[2].lspoints[i].zdisplay[instance]));
					}
				glEnd();
				glBegin(GL_LINE_STRIP);
				for (i=0;i<shared.shareddata.navpick.xsegments[3].nls;i++)
					{
					glVertex3f((float)(shared.shareddata.navpick.xsegments[3].lspoints[i].xdisplay[instance]), 
							(float)(shared.shareddata.navpick.xsegments[3].lspoints[i].ydisplay[instance]), 
							(float)(shared.shareddata.navpick.xsegments[3].lspoints[i].zdisplay[instance]));
					}
				glEnd();
				}
			else if (data->display_mode == MBV_DISPLAY_3D)
				{
				glBegin(GL_LINES);
				for (i=4;i<8;i++)
					{
					glVertex3f((float)(shared.shareddata.navpick.xpoints[i].xdisplay[instance]), 
						(float)(shared.shareddata.navpick.xpoints[i].ydisplay[instance]), 
						(float)(shared.shareddata.navpick.xpoints[i].zdisplay[instance]));
					}
				glEnd();
				}
			else
				{
				glBegin(GL_LINES);
				glVertex3f((float)(shared.shareddata.navpick.xpoints[4].xdisplay[instance]), 
					(float)(shared.shareddata.navpick.xpoints[4].ydisplay[instance]), 
					(float)(shared.shareddata.navpick.xpoints[4].zdisplay[instance]));
				glVertex3f((float)(shared.shareddata.navpick.xpoints[5].xdisplay[instance]), 
					(float)(shared.shareddata.navpick.xpoints[5].ydisplay[instance]), 
					(float)(shared.shareddata.navpick.xpoints[5].zdisplay[instance]));
				glVertex3f((float)(shared.shareddata.navpick.xpoints[6].xdisplay[instance]), 
					(float)(shared.shareddata.navpick.xpoints[6].ydisplay[instance]), 
					(float)(shared.shareddata.navpick.xpoints[6].zdisplay[instance]));
				glVertex3f((float)(shared.shareddata.navpick.xpoints[7].xdisplay[instance]), 
					(float)(shared.shareddata.navpick.xpoints[7].ydisplay[instance]), 
					(float)(shared.shareddata.navpick.xpoints[7].zdisplay[instance]));
				glEnd();
				}
			
			/* draw second navpick point undraped */
			if (data->display_mode == MBV_DISPLAY_3D
				&& data->nav_view_mode == MBV_VIEW_ON)
				{
				inav = shared.shareddata.nav_selected[1];
				jpt = shared.shareddata.nav_point_selected[1];
				zdisplay = shared.shareddata.navs[inav].navpts[jpt].point.zdisplay[instance];
				glBegin(GL_LINES);
				for (i=4;i<8;i++)
					{
					glVertex3f((float)(shared.shareddata.navpick.xpoints[i].xdisplay[instance]), 
						(float)(shared.shareddata.navpick.xpoints[i].ydisplay[instance]), 
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
	struct mbview_linesegmentw_struct segment;
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
	
	/* set decimation */
	if (rez == MBV_REZ_FULL)
	    stride = 1;
	else if (rez == MBV_REZ_HIGH)
	    stride = data->hirez_navdecimate;
	else
	    stride = data->lorez_navdecimate;
		
	/* draw navigation */
	if (shared.shareddata.nav_mode != MBV_NAV_OFF
		&& data->nav_view_mode == MBV_VIEW_ON
		&& shared.shareddata.nnav > 0)
		{
		/* loop over the navs plotting xyz navigation */
		for (inav=0;inav<shared.shareddata.nnav;inav++)
			{
			icolor = shared.shareddata.navs[inav].color;
			glLineWidth((float)(shared.shareddata.navs[inav].size));
			glBegin(GL_LINE_STRIP);
			for (jpoint=0;jpoint<shared.shareddata.navs[inav].npoints;jpoint+=stride)
				{
				/* set size and color */
				if (shared.shareddata.navs[inav].navpts[jpoint].selected == MB_YES
					|| shared.shareddata.navs[inav].navpts[jpoint+1].selected == MB_YES)
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
				glVertex3f((float)(shared.shareddata.navs[inav].navpts[jpoint].point.xdisplay[instance]), 
						(float)(shared.shareddata.navs[inav].navpts[jpoint].point.ydisplay[instance]), 
						(float)(shared.shareddata.navs[inav].navpts[jpoint].point.zdisplay[instance]));
				}
			glEnd();
			}
		}
		
	/* draw draped navigation */
	if (shared.shareddata.nav_mode != MBV_NAV_OFF
		&& data->navdrape_view_mode == MBV_VIEW_ON
		&& shared.shareddata.nnav > 0)
		{
		/* loop over the navs plotting draped navigation */
		for (inav=0;inav<shared.shareddata.nnav;inav++)
			{
			icolor = shared.shareddata.navs[inav].color;
			glLineWidth((float)(shared.shareddata.navs[inav].size));
			glBegin(GL_LINE_STRIP);
			for (jpoint=0;jpoint<shared.shareddata.navs[inav].npoints-stride;jpoint+=stride)
				{
				/* set size and color */
				if (shared.shareddata.navs[inav].navpts[jpoint].selected == MB_YES
					|| shared.shareddata.navs[inav].navpts[jpoint+stride].selected == MB_YES)
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
					
/*fprintf(stderr,"inav:%d npoints:%d jpoint:%d nls:%d\n", 
inav, shared.shareddata.navs[inav].npoints, jpoint, shared.shareddata.navs[inav].segments[jpoint].nls);*/
				/* draw draped segment if stride == 1 */
				if (stride == 1)
				for (k=0;k<shared.shareddata.navs[inav].segments[jpoint].nls;k++)
					{
					/* draw points */
					glVertex3f((float)(shared.shareddata.navs[inav].segments[jpoint].lspoints[k].xdisplay[instance]), 
							(float)(shared.shareddata.navs[inav].segments[jpoint].lspoints[k].ydisplay[instance]), 
							(float)(shared.shareddata.navs[inav].segments[jpoint].lspoints[k].zdisplay[instance]));
/*fprintf(stderr,"inav:%d jpoint:%d k:%d zdata:%f zdisplay:%f\n",
inav,jpoint,k,
shared.shareddata.navs[inav].segments[jpoint].lspoints[k].zdata,
shared.shareddata.navs[inav].segments[jpoint].lspoints[k].zdisplay[instance]);*/

					}
					
				/* else draw decimated draped nav */
				else if (shared.shareddata.navs[inav].segments[jpoint].nls > 0)
					{
					/* draw points */
					glVertex3f((float)(shared.shareddata.navs[inav].segments[jpoint].lspoints[0].xdisplay[instance]), 
							(float)(shared.shareddata.navs[inav].segments[jpoint].lspoints[0].ydisplay[instance]), 
							(float)(shared.shareddata.navs[inav].segments[jpoint].lspoints[0].zdisplay[instance]));
					}
				}
			glEnd();
			}
		}
		
	/* draw swathbounds */
	if (shared.shareddata.nav_mode != MBV_NAV_OFF
		&& (data->nav_view_mode == MBV_VIEW_ON
			|| data->navdrape_view_mode == MBV_VIEW_ON)
		&& shared.shareddata.nnav > 0)
		{
		/* initialize on the fly draping segment */
		segment.nls = 0;
		segment.nls_alloc = 0;
		segment.lspoints = NULL;
		
		/* loop over the navs plotting swathbounds */
		timegapuse = 60.0 * view->timegap;
		for (inav=0;inav<shared.shareddata.nnav;inav++)
			{
			if (shared.shareddata.navs[inav].swathbounds == MB_YES
				&& shared.shareddata.navs[inav].nselected > 0)
				{
				glColor3f(colortable_object_red[MBV_COLOR_YELLOW], 
					colortable_object_green[MBV_COLOR_YELLOW], 
					colortable_object_blue[MBV_COLOR_YELLOW]);
				glLineWidth((float)(shared.shareddata.navs[inav].size));

				/* draw port side of swath */
				swathbounds_on = MB_NO;
				for (jpoint=0;jpoint<shared.shareddata.navs[inav].npoints;jpoint++)
					{
					/* draw from center at start of selected data */
					if (swathbounds_on == MB_NO
						&& shared.shareddata.navs[inav].navpts[jpoint].selected == MB_YES)
						{
						swathbounds_on = MB_YES;
						glBegin(GL_LINE_STRIP);
						
						if (data->display_mode == MBV_DISPLAY_3D && stride == 1)
							{
							/* drape segment on the fly */
							segment.endpoints[0] = &(shared.shareddata.navs[inav].navpts[jpoint].pointcntr);
							segment.endpoints[1] = &(shared.shareddata.navs[inav].navpts[jpoint].pointport);
							mbview_drapesegmentw(instance, &(segment));

							/* draw the segment */
							for (i=0;i<segment.nls;i++)
								{
								glVertex3f((float)(segment.lspoints[i].xdisplay[instance]), 
										(float)(segment.lspoints[i].ydisplay[instance]), 
										(float)(segment.lspoints[i].zdisplay[instance]));
								}
							}
						else
							{
							glVertex3f((float)(shared.shareddata.navs[inav].navpts[jpoint].pointcntr.xdisplay[instance]), 
									(float)(shared.shareddata.navs[inav].navpts[jpoint].pointcntr.ydisplay[instance]), 
									(float)(shared.shareddata.navs[inav].navpts[jpoint].pointcntr.zdisplay[instance]));
							}
						}

					/* draw during selected data */
					if (shared.shareddata.navs[inav].navpts[jpoint].selected == MB_YES)
						{
						glVertex3f((float)(shared.shareddata.navs[inav].navpts[jpoint].pointport.xdisplay[instance]), 
							(float)(shared.shareddata.navs[inav].navpts[jpoint].pointport.ydisplay[instance]), 
							(float)(shared.shareddata.navs[inav].navpts[jpoint].pointport.zdisplay[instance]));
						}

					/* draw to center at end of selected data */
					if (swathbounds_on == MB_YES
						&& (shared.shareddata.navs[inav].navpts[jpoint].selected == MB_NO
							|| jpoint >= shared.shareddata.navs[inav].npoints - 1
							|| (jpoint > 0 
								&& (shared.shareddata.navs[inav].navpts[jpoint].time_d
									- shared.shareddata.navs[inav].navpts[jpoint-1].time_d)
									> timegapuse)))
						{						
						if (data->display_mode == MBV_DISPLAY_3D && stride == 1)
							{
							/* drape segment on the fly */
							segment.endpoints[0] = &(shared.shareddata.navs[inav].navpts[jpoint].pointport);
							segment.endpoints[1] = &(shared.shareddata.navs[inav].navpts[jpoint].pointcntr);
							mbview_drapesegmentw(instance, &(segment));

							/* draw the segment */
							for (i=0;i<segment.nls;i++)
								{
								glVertex3f((float)(segment.lspoints[i].xdisplay[instance]), 
										(float)(segment.lspoints[i].ydisplay[instance]), 
										(float)(segment.lspoints[i].zdisplay[instance]));
								}
							}
						else
							{
							glVertex3f((float)(shared.shareddata.navs[inav].navpts[jpoint].pointcntr.xdisplay[instance]), 
									(float)(shared.shareddata.navs[inav].navpts[jpoint].pointcntr.ydisplay[instance]), 
									(float)(shared.shareddata.navs[inav].navpts[jpoint].pointcntr.zdisplay[instance]));
							}

						swathbounds_on = MB_NO;
						glEnd();
						}
					}

				/* draw starboard side of swath */
				swathbounds_on = MB_NO;
				for (jpoint=0;jpoint<shared.shareddata.navs[inav].npoints;jpoint++)
					{
					/* draw from center at start of selected data */
					if (swathbounds_on == MB_NO
						&& shared.shareddata.navs[inav].navpts[jpoint].selected == MB_YES)
						{
						swathbounds_on = MB_YES;
						glBegin(GL_LINE_STRIP);
						
						if (data->display_mode == MBV_DISPLAY_3D && stride == 1)
							{
							/* drape segment on the fly */
							segment.endpoints[0] = &(shared.shareddata.navs[inav].navpts[jpoint].pointcntr);
							segment.endpoints[1] = &(shared.shareddata.navs[inav].navpts[jpoint].pointstbd);
							mbview_drapesegmentw(instance, &(segment));

							/* draw the segment */
							for (i=0;i<segment.nls;i++)
								{
								glVertex3f((float)(segment.lspoints[i].xdisplay[instance]), 
										(float)(segment.lspoints[i].ydisplay[instance]), 
										(float)(segment.lspoints[i].zdisplay[instance]));
								}
							}
						else
							{
							glVertex3f((float)(shared.shareddata.navs[inav].navpts[jpoint].pointcntr.xdisplay[instance]), 
									(float)(shared.shareddata.navs[inav].navpts[jpoint].pointcntr.ydisplay[instance]), 
									(float)(shared.shareddata.navs[inav].navpts[jpoint].pointcntr.zdisplay[instance]));
							}
						}

					/* draw during selected data */
					if (shared.shareddata.navs[inav].navpts[jpoint].selected == MB_YES)
						{
						glVertex3f((float)(shared.shareddata.navs[inav].navpts[jpoint].pointstbd.xdisplay[instance]), 
							(float)(shared.shareddata.navs[inav].navpts[jpoint].pointstbd.ydisplay[instance]), 
							(float)(shared.shareddata.navs[inav].navpts[jpoint].pointstbd.zdisplay[instance]));
						}

					/* draw to center at end of selected data */
					if (swathbounds_on == MB_YES
						&& (shared.shareddata.navs[inav].navpts[jpoint].selected == MB_NO
							|| jpoint >= shared.shareddata.navs[inav].npoints - 1
							|| (jpoint > 0 
								&& (shared.shareddata.navs[inav].navpts[jpoint].time_d
									- shared.shareddata.navs[inav].navpts[jpoint-1].time_d)
									> timegapuse)))
						{						
						if (data->display_mode == MBV_DISPLAY_3D && stride == 1)
							{
							/* drape segment on the fly */
							segment.endpoints[0] = &(shared.shareddata.navs[inav].navpts[jpoint].pointstbd);
							segment.endpoints[1] = &(shared.shareddata.navs[inav].navpts[jpoint].pointcntr);
							mbview_drapesegmentw(instance, &(segment));

							/* draw the segment */
							for (i=0;i<segment.nls;i++)
								{
								glVertex3f((float)(segment.lspoints[i].xdisplay[instance]), 
										(float)(segment.lspoints[i].ydisplay[instance]), 
										(float)(segment.lspoints[i].zdisplay[instance]));
								}
							}
						else
							{
							glVertex3f((float)(shared.shareddata.navs[inav].navpts[jpoint].pointcntr.xdisplay[instance]), 
									(float)(shared.shareddata.navs[inav].navpts[jpoint].pointcntr.ydisplay[instance]), 
									(float)(shared.shareddata.navs[inav].navpts[jpoint].pointcntr.zdisplay[instance]));
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
int mbview_updatenavlist()
{
	/* local variables */
	char	*function_name = "mbview_updatenavlist";
	int	status = MB_SUCCESS;
   	XmString *xstr;
	int	inav;
	int	jpoint;
	int	nitems;
	int	iitem;
	char	lonstr0[24];
	char	latstr0[24];
	int	inavselect;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		}
		
	/* update nav list */
	if (shared.init_navlist == MBV_WINDOW_VISIBLE)
		{
		/* remove all existing items */
		XmListDeleteAllItems(shared.mb3d_navlist.mbview_list_navlist);
		
		if (shared.shareddata.nnav > 0)
			{
			/* get number of items */
			nitems = 0;
			for (inav=0;inav<shared.shareddata.nnav;inav++)
				{
				nitems += 1 + shared.shareddata.navs[inav].npoints;
				}
			
			/* allocate array of label XmStrings */
			xstr = (XmString *) malloc(nitems * sizeof(XmString));

			/* loop over the navs */
			nitems = 0;
			for (inav=0;inav<shared.shareddata.nnav;inav++)
				{
				/* add list item for each nav */
				sprintf(value_string,"%3d | %3d | %s | %d | %s", 
					inav, shared.shareddata.navs[inav].npoints,
					mbview_colorname[shared.shareddata.navs[inav].color],
					shared.shareddata.navs[inav].size,
					shared.shareddata.navs[inav].name);
    				xstr[nitems] = XmStringCreateLocalized(value_string);
				nitems++;
				}

			/* add list items */
    			XmListAddItems(shared.mb3d_navlist.mbview_list_navlist,
					xstr, nitems, 0);				

			/* deallocate memory no longer needed */
			for (iitem=0;iitem<nitems;iitem++)
				{
    				XmStringFree(xstr[iitem]);
    				}
    			free(xstr);
			
			/* check for completely selected navs */
			inavselect = MBV_SELECT_NONE;
			for (inav=0;inav<shared.shareddata.nnav;inav++)
				{
				if (inavselect == MBV_SELECT_NONE
					&& shared.shareddata.navs[inav].npoints > 1
					&& shared.shareddata.navs[inav].nselected 
						== shared.shareddata.navs[inav].npoints)
					{
					inavselect = inav;
					}
				}

			/* select first item with fully selected nav */
			if (inavselect != MBV_SELECT_NONE)
				{
				iitem = inavselect + 1;
				XmListSelectPos(shared.mb3d_navlist.mbview_list_navlist,
						iitem,0);
				XmListSetPos(shared.mb3d_navlist.mbview_list_navlist,
					MAX(iitem-5, 1));
				}
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

