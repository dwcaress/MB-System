/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_nav.c	10/28/2003
 *
 *    Copyright (c) 2003-2023 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * Author:	D. W. Caress
 * Date:	October 28, 2003
 */

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_status.h"

/* Need to include windows.h BEFORE the the Xm stuff otherwise VC14+ barf with conflicts */
#if defined(_MSC_VER) && (_MSC_VER >= 1800)
#	ifndef WIN32
#		define WIN32
#	endif
#	include <WinSock2.h>
#include <windows.h>
#endif

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
#include <Xm/List.h>
#include "MB3DView.h"
#include "MB3DSiteList.h"
#include "MB3DRouteList.h"
#include "MB3DNavList.h"

#include <GL/gl.h>
#include <GL/glu.h>
#ifndef WIN32
#include <GL/glx.h>
#endif
#include "mb_glwdrawa.h"

#include "mbview.h"
#include "mbviewprivate.h"

static char value_string[2*MB_PATH_MAXLINE];

/*------------------------------------------------------------------------------*/
int mbview_getnavcount(int verbose, size_t instance, int *nnav, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	/* get number of navs */
	*nnav = shared.shareddata.nnav;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nnav:                      %d\n", *nnav);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_getnavpointcount(int verbose, size_t instance, int nav, int *npoint, int *nintpoint, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       nav:                     %d\n", nav);
	}

	/* get number of points in specified nav */
	*npoint = 0;
	*nintpoint = 0;
	if (nav >= 0 && nav < shared.shareddata.nnav) {
		*npoint = shared.shareddata.navs[nav].npoints;
		for (int i = 0; i < *npoint - 1; i++) {
			if (shared.shareddata.navs[nav].segments[i].nls > 2)
				*nintpoint += shared.shareddata.navs[nav].segments[i].nls - 2;
		}
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       npoint:                    %d\n", *npoint);
		fprintf(stderr, "dbg2       nintpoint:                 %d\n", *nintpoint);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_allocnavarrays(int verbose, int npointtotal, double **time_d, double **navlon, double **navlat, double **navz,
                          double **heading, double **speed, double **navportlon, double **navportlat, double **navstbdlon,
                          double **navstbdlat, int **line, int **shot, int **cdp, int *error) {
	fprintf(stderr, "mbview_allocnavarrays: %d points\n", npointtotal);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       npointtotal:               %d\n", npointtotal);
		fprintf(stderr, "dbg2       time_d:                    %p\n", *time_d);
		fprintf(stderr, "dbg2       navlon:                    %p\n", *navlon);
		fprintf(stderr, "dbg2       navlat:                    %p\n", *navlat);
		fprintf(stderr, "dbg2       navz:                      %p\n", *navz);
		fprintf(stderr, "dbg2       heading:                   %p\n", *heading);
		fprintf(stderr, "dbg2       speed:                     %p\n", *speed);
		if (navportlon != NULL)
			fprintf(stderr, "dbg2       navportlon:                %p\n", *navportlon);
		if (navportlat != NULL)
			fprintf(stderr, "dbg2       navportlat:                %p\n", *navportlat);
		if (navstbdlon != NULL)
			fprintf(stderr, "dbg2       navstbdlon:                %p\n", *navstbdlon);
		if (navstbdlat != NULL)
			fprintf(stderr, "dbg2       navstbdlat:                %p\n", *navstbdlat);
		if (line != NULL)
			fprintf(stderr, "dbg2       line:                      %p\n", *line);
		if (shot != NULL)
			fprintf(stderr, "dbg2       shot:                      %p\n", *shot);
		if (cdp != NULL)
			fprintf(stderr, "dbg2       cdp:                       %p\n", *cdp);
	}

	/* allocate the arrays using mb_reallocd */
	int status = mb_reallocd(verbose, __FILE__, __LINE__, npointtotal * sizeof(double), (void **)time_d, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npointtotal * sizeof(double), (void **)navlon, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npointtotal * sizeof(double), (void **)navlat, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npointtotal * sizeof(double), (void **)navz, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npointtotal * sizeof(double), (void **)heading, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npointtotal * sizeof(double), (void **)speed, error);
	if (status == MB_SUCCESS && navportlon != NULL)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npointtotal * sizeof(double), (void **)navportlon, error);
	if (status == MB_SUCCESS && navportlat != NULL)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npointtotal * sizeof(double), (void **)navportlat, error);
	if (status == MB_SUCCESS && navstbdlon != NULL)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npointtotal * sizeof(double), (void **)navstbdlon, error);
	if (status == MB_SUCCESS && navstbdlat != NULL)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npointtotal * sizeof(double), (void **)navstbdlat, error);
	if (status == MB_SUCCESS && line != NULL)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npointtotal * sizeof(int), (void **)line, error);
	if (status == MB_SUCCESS && shot != NULL)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npointtotal * sizeof(int), (void **)shot, error);
	if (status == MB_SUCCESS && cdp != NULL)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npointtotal * sizeof(int), (void **)cdp, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       time_d:                    %p\n", *time_d);
		fprintf(stderr, "dbg2       navlon:                    %p\n", *navlon);
		fprintf(stderr, "dbg2       navlat:                    %p\n", *navlat);
		fprintf(stderr, "dbg2       navz:                      %p\n", *navz);
		fprintf(stderr, "dbg2       heading:                   %p\n", *heading);
		fprintf(stderr, "dbg2       speed:                     %p\n", *speed);
		if (navportlon != NULL)
			fprintf(stderr, "dbg2       navportlon:                %p\n", *navportlon);
		if (navportlat != NULL)
			fprintf(stderr, "dbg2       navportlat:                %p\n", *navportlat);
		if (navstbdlon != NULL)
			fprintf(stderr, "dbg2       navstbdlon:                %p\n", *navstbdlon);
		if (navstbdlat != NULL)
			fprintf(stderr, "dbg2       navstbdlat:                %p\n", *navstbdlat);
		if (line != NULL)
			fprintf(stderr, "dbg2       line:                      %p\n", *line);
		if (shot != NULL)
			fprintf(stderr, "dbg2       shot:                      %p\n", *shot);
		if (cdp != NULL)
			fprintf(stderr, "dbg2       cdp:                       %p\n", *cdp);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_freenavarrays(int verbose, double **time_d, double **navlon, double **navlat, double **navz, double **heading,
                         double **speed, double **navportlon, double **navportlat, double **navstbdlon, double **navstbdlat,
                         int **line, int **shot, int **cdp, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       time_d:                    %p\n", *time_d);
		fprintf(stderr, "dbg2       navlon:                    %p\n", *navlon);
		fprintf(stderr, "dbg2       navlat:                    %p\n", *navlat);
		fprintf(stderr, "dbg2       navz:                      %p\n", *navz);
		fprintf(stderr, "dbg2       heading:                   %p\n", *heading);
		fprintf(stderr, "dbg2       speed:                     %p\n", *speed);
		if (navportlon != NULL)
			fprintf(stderr, "dbg2       navportlon:                %p\n", *navportlon);
		if (navportlat != NULL)
			fprintf(stderr, "dbg2       navportlat:                %p\n", *navportlat);
		if (navstbdlon != NULL)
			fprintf(stderr, "dbg2       navstbdlon:                %p\n", *navstbdlon);
		if (navstbdlat != NULL)
			fprintf(stderr, "dbg2       navstbdlat:                %p\n", *navstbdlat);
		if (line != NULL)
			fprintf(stderr, "dbg2       line:                      %p\n", *line);
		if (shot != NULL)
			fprintf(stderr, "dbg2       shot:                      %p\n", *shot);
		if (cdp != NULL)
			fprintf(stderr, "dbg2       cdp:                       %p\n", *cdp);
	}

	/* free the arrays using mb_freed */
	int status = mb_freed(verbose, __FILE__, __LINE__, (void **)time_d, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)navlon, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)navlat, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)navz, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)heading, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)speed, error);
	if (navportlon != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)navportlon, error);
	if (navportlat != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)navportlat, error);
	if (navstbdlon != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)navstbdlon, error);
	if (navstbdlat != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)navstbdlat, error);
	if (line != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)line, error);
	if (shot != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)shot, error);
	if (cdp != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)cdp, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       time_d:                    %p\n", *time_d);
		fprintf(stderr, "dbg2       navlon:                    %p\n", *navlon);
		fprintf(stderr, "dbg2       navlat:                    %p\n", *navlat);
		fprintf(stderr, "dbg2       navz:                      %p\n", *navz);
		fprintf(stderr, "dbg2       heading:                   %p\n", *heading);
		fprintf(stderr, "dbg2       speed:                     %p\n", *speed);
		if (navportlon != NULL)
			fprintf(stderr, "dbg2       navportlon:                %p\n", *navportlon);
		if (navportlat != NULL)
			fprintf(stderr, "dbg2       navportlat:                %p\n", *navportlat);
		if (navstbdlon != NULL)
			fprintf(stderr, "dbg2       navstbdlon:                %p\n", *navstbdlon);
		if (navstbdlat != NULL)
			fprintf(stderr, "dbg2       navstbdlat:                %p\n", *navstbdlat);
		if (line != NULL)
			fprintf(stderr, "dbg2       line:                      %p\n", *line);
		if (shot != NULL)
			fprintf(stderr, "dbg2       shot:                      %p\n", *shot);
		if (cdp != NULL)
			fprintf(stderr, "dbg2       cdp:                       %p\n", *cdp);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_addnav(int verbose, size_t instance, int npoint, double *time_d, double *navlon, double *navlat, double *navz,
                  double *heading, double *speed, double *navportlon, double *navportlat, double *navstbdlon, double *navstbdlat,
                  unsigned int *line, unsigned int *shot, unsigned int *cdp, int navcolor, int navsize, mb_path navname, int navpathstatus,
                  mb_path navpathraw, mb_path navpathprocessed, int navformat, bool navswathbounds, bool navline, bool navshot,
                  bool navcdp, int decimation, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       npoint:                    %d\n", npoint);
		for (int i = 0; i < npoint; i++) {
			fprintf(stderr, "dbg2       point:%d time_d:%f lon:%f lat:%f z:%f heading:%f zpeed:%f\n", i, time_d[i], navlon[i],
			        navlat[i], navz[i], heading[i], speed[i]);
		}
		if (navswathbounds)
			for (int i = 0; i < npoint; i++) {
				fprintf(stderr, "dbg2       point:%d port: lon:%f lat:%f  stbd: lon:%f lat:%f\n", i, navportlon[i], navportlat[i],
				        navstbdlon[i], navstbdlat[i]);
			}
		if (navline)
			for (int i = 0; i < npoint; i++) {
				fprintf(stderr, "dbg2       point:%d line:%d\n", i, line[i]);
			}
		if (navshot)
			for (int i = 0; i < npoint; i++) {
				fprintf(stderr, "dbg2       point:%d shot:%d\n", i, shot[i]);
			}
		if (navcdp)
			for (int i = 0; i < npoint; i++) {
				fprintf(stderr, "dbg2       point:%d cdp: %d\n", i, cdp[i]);
			}
		fprintf(stderr, "dbg2       navcolor:                  %d\n", navcolor);
		fprintf(stderr, "dbg2       navsize:                   %d\n", navsize);
		fprintf(stderr, "dbg2       navname:                   %s\n", navname);
		fprintf(stderr, "dbg2       navpathstatus:             %d\n", navpathstatus);
		fprintf(stderr, "dbg2       navpathraw:                %s\n", navpathraw);
		fprintf(stderr, "dbg2       navpathprocessed:          %s\n", navpathprocessed);
		fprintf(stderr, "dbg2       navformat:                 %d\n", navformat);
		fprintf(stderr, "dbg2       navswathbounds:            %d\n", navswathbounds);
		fprintf(stderr, "dbg2       navline:                   %d\n", navline);
		fprintf(stderr, "dbg2       navshot:                   %d\n", navshot);
		fprintf(stderr, "dbg2       navcdp:                    %d\n", navcdp);
		fprintf(stderr, "dbg2       decimation:                %d\n", decimation);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* make sure no nav is selected */
	shared.shareddata.nav_selected[0] = MBV_SELECT_NONE;
	shared.shareddata.nav_selected[1] = MBV_SELECT_NONE;
	shared.shareddata.nav_point_selected[0] = MBV_SELECT_NONE;
	shared.shareddata.nav_point_selected[1] = MBV_SELECT_NONE;
	shared.shareddata.nav_selected_mbnavadjust[0] = MBV_SELECT_NONE;
	shared.shareddata.nav_selected_mbnavadjust[1] = MBV_SELECT_NONE;

	/* set nav id so that new nav is created */
	int inav = shared.shareddata.nnav;

	int status = MB_SUCCESS;

	/* allocate memory for a new nav if required */
	if (shared.shareddata.nnav_alloc < shared.shareddata.nnav + 1) {
		shared.shareddata.nnav_alloc = shared.shareddata.nnav + 1;
		status = mb_reallocd(mbv_verbose, __FILE__, __LINE__, shared.shareddata.nnav_alloc * sizeof(struct mbview_nav_struct),
		                     (void **)&(shared.shareddata.navs), error);
		if (status == MB_FAILURE) {
			shared.shareddata.nnav_alloc = 0;
		}
		else {
			for (int i = shared.shareddata.nnav; i < shared.shareddata.nnav_alloc; i++) {
				shared.shareddata.navs[i].active = false;
				shared.shareddata.navs[i].color = MBV_COLOR_RED;
				shared.shareddata.navs[i].size = 4;
				shared.shareddata.navs[i].name[0] = '\0';
				shared.shareddata.navs[i].pathstatus = MB_PROCESSED_NONE;
				shared.shareddata.navs[i].pathraw[0] = '\0';
				shared.shareddata.navs[i].pathprocessed[0] = '\0';
				shared.shareddata.navs[i].format = 0;
				shared.shareddata.navs[i].swathbounds = false;
				shared.shareddata.navs[i].line = false;
				shared.shareddata.navs[i].shot = false;
				shared.shareddata.navs[i].cdp = false;
				shared.shareddata.navs[i].decimation = false;
				shared.shareddata.navs[i].npoints = 0;
				shared.shareddata.navs[i].npoints_alloc = 0;
				shared.shareddata.navs[i].nselected = 0;
				shared.shareddata.navs[i].navpts = NULL;
				shared.shareddata.navs[i].segments = NULL;
			}
		}
	}

	/* allocate memory to for nav arrays */
	if (shared.shareddata.navs[inav].npoints_alloc < npoint) {
		shared.shareddata.navs[inav].npoints_alloc = npoint;
		status = mb_reallocd(mbv_verbose, __FILE__, __LINE__,
		                     shared.shareddata.navs[inav].npoints_alloc * sizeof(struct mbview_navpointw_struct),
		                     (void **)&(shared.shareddata.navs[inav].navpts), error);
		status = mb_reallocd(mbv_verbose, __FILE__, __LINE__,
		                     shared.shareddata.navs[inav].npoints_alloc * sizeof(struct mbview_linesegmentw_struct),
		                     (void **)&(shared.shareddata.navs[inav].segments), error);
    memset((void *)shared.shareddata.navs[inav].segments, 0,
            (size_t)shared.shareddata.navs[inav].npoints_alloc * sizeof(struct mbview_linesegmentw_struct));
	}

	/* add the new nav */
	if (status == MB_SUCCESS) {
		/* set nnav */
		shared.shareddata.nnav++;

		/* set color size and name for new nav */
		shared.shareddata.navs[inav].active = true;
		shared.shareddata.navs[inav].color = navcolor;
		shared.shareddata.navs[inav].size = navsize;
		strcpy(shared.shareddata.navs[inav].name, navname);
		shared.shareddata.navs[inav].pathstatus = navpathstatus;
		strcpy(shared.shareddata.navs[inav].pathraw, navpathraw);
		strcpy(shared.shareddata.navs[inav].pathprocessed, navpathprocessed);
		shared.shareddata.navs[inav].format = navformat;
		shared.shareddata.navs[inav].swathbounds = navswathbounds;
		shared.shareddata.navs[inav].line = navline;
		shared.shareddata.navs[inav].shot = navshot;
		shared.shareddata.navs[inav].cdp = navcdp;
		shared.shareddata.navs[inav].decimation = decimation;

		/* loop over the points in the new nav */
		shared.shareddata.navs[inav].npoints = npoint;
		for (int i = 0; i < npoint; i++) {
			/* set status values */
			shared.shareddata.navs[inav].navpts[i].draped = false;
			shared.shareddata.navs[inav].navpts[i].selected = false;

			/* set time and shot info */
			shared.shareddata.navs[inav].navpts[i].time_d = time_d[i];
			shared.shareddata.navs[inav].navpts[i].heading = heading[i];
			shared.shareddata.navs[inav].navpts[i].speed = speed[i];
			if (shared.shareddata.navs[inav].line)
				shared.shareddata.navs[inav].navpts[i].line = line[i];
			if (shared.shareddata.navs[inav].shot)
				shared.shareddata.navs[inav].navpts[i].shot = shot[i];
			if (shared.shareddata.navs[inav].cdp)
				shared.shareddata.navs[inav].navpts[i].cdp = cdp[i];

			/* ************************************************* */
			/* get nav positions in grid and display coordinates */
			shared.shareddata.navs[inav].navpts[i].point.xlon = navlon[i];
			shared.shareddata.navs[inav].navpts[i].point.ylat = navlat[i];
			shared.shareddata.navs[inav].navpts[i].point.zdata = navz[i];
			status = mbview_projectfromlonlat(instance, shared.shareddata.navs[inav].navpts[i].point.xlon,
			                                  shared.shareddata.navs[inav].navpts[i].point.ylat,
			                                  shared.shareddata.navs[inav].navpts[i].point.zdata,
			                                  &(shared.shareddata.navs[inav].navpts[i].point.xgrid[instance]),
			                                  &(shared.shareddata.navs[inav].navpts[i].point.ygrid[instance]),
			                                  &(shared.shareddata.navs[inav].navpts[i].point.xdisplay[instance]),
			                                  &(shared.shareddata.navs[inav].navpts[i].point.ydisplay[instance]),
			                                  &(shared.shareddata.navs[inav].navpts[i].point.zdisplay[instance]));
			mbview_updatepointw(instance, &(shared.shareddata.navs[inav].navpts[i].point));

			/* fprintf(stderr,"Depth: llz:%.10f %.10f %.10f   grid:%.10f %.10f   dpy:%.10f %.10f %.10f\n",
			shared.shareddata.navs[inav].navpts[i].point.xlon,
			shared.shareddata.navs[inav].navpts[i].point.ylat,
			shared.shareddata.navs[inav].navpts[i].point.zdata,
			shared.shareddata.navs[inav].navpts[i].point.xgrid[instance],
			shared.shareddata.navs[inav].navpts[i].point.ygrid[instance],
			shared.shareddata.navs[inav].navpts[i].point.xdisplay[instance],
			shared.shareddata.navs[inav].navpts[i].point.ydisplay[instance],
			shared.shareddata.navs[inav].navpts[i].point.zdisplay[instance]); */

			/* ************************************************* */
			/* get center on-bottom nav positions in grid coordinates */
			shared.shareddata.navs[inav].navpts[i].pointcntr.xlon = navlon[i];
			shared.shareddata.navs[inav].navpts[i].pointcntr.ylat = navlat[i];
			status = mbview_projectll2xyzgrid(instance, shared.shareddata.navs[inav].navpts[i].pointcntr.xlon,
			                                  shared.shareddata.navs[inav].navpts[i].pointcntr.ylat,
			                                  &(shared.shareddata.navs[inav].navpts[i].pointcntr.xgrid[instance]),
			                                  &(shared.shareddata.navs[inav].navpts[i].pointcntr.ygrid[instance]),
			                                  &(shared.shareddata.navs[inav].navpts[i].pointcntr.zdata));

			/* get center on-bottom nav positions in display coordinates */
			status = mbview_projectll2display(instance, shared.shareddata.navs[inav].navpts[i].pointcntr.xlon,
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
			status = mbview_projectll2xyzgrid(instance, shared.shareddata.navs[inav].navpts[i].pointport.xlon,
			                                  shared.shareddata.navs[inav].navpts[i].pointport.ylat,
			                                  &(shared.shareddata.navs[inav].navpts[i].pointport.xgrid[instance]),
			                                  &(shared.shareddata.navs[inav].navpts[i].pointport.ygrid[instance]),
			                                  &(shared.shareddata.navs[inav].navpts[i].pointport.zdata));

			/* get port on-bottom nav positions in display coordinates */
			status = mbview_projectll2display(instance, shared.shareddata.navs[inav].navpts[i].pointport.xlon,
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
			status = mbview_projectll2xyzgrid(instance, shared.shareddata.navs[inav].navpts[i].pointstbd.xlon,
			                                  shared.shareddata.navs[inav].navpts[i].pointstbd.ylat,
			                                  &(shared.shareddata.navs[inav].navpts[i].pointstbd.xgrid[instance]),
			                                  &(shared.shareddata.navs[inav].navpts[i].pointstbd.ygrid[instance]),
			                                  &(shared.shareddata.navs[inav].navpts[i].pointstbd.zdata));

			/* get starboard on-bottom nav positions in display coordinates */
			status = mbview_projectll2display(instance, shared.shareddata.navs[inav].navpts[i].pointstbd.xlon,
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
		for (int i = 0; i < shared.shareddata.navs[inav].npoints - 1; i++) {
			shared.shareddata.navs[inav].segments[i].endpoints[0] = shared.shareddata.navs[inav].navpts[i].pointcntr;
			shared.shareddata.navs[inav].segments[i].endpoints[1] = shared.shareddata.navs[inav].navpts[i + 1].pointcntr;

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
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  Nav data altered in function <%s>\n", __func__);
		fprintf(stderr, "dbg2  Nav values:\n");
		fprintf(stderr, "dbg2       nav_mode:           %d\n", shared.shareddata.nav_mode);
		fprintf(stderr, "dbg2       nav_view_mode:      %d\n", data->nav_view_mode);
		fprintf(stderr, "dbg2       navdrape_view_mode: %d\n", data->navdrape_view_mode);
		fprintf(stderr, "dbg2       nnav:               %d\n", shared.shareddata.nnav);
		fprintf(stderr, "dbg2       nnav_alloc:         %d\n", shared.shareddata.nnav_alloc);
		fprintf(stderr, "dbg2       nav_selected[0]:    %d\n", shared.shareddata.nav_selected[0]);
		fprintf(stderr, "dbg2       nav_selected[1]:    %d\n", shared.shareddata.nav_selected[1]);
		fprintf(stderr, "dbg2       nav_point_selected: %p\n", shared.shareddata.nav_point_selected);
		for (int i = 0; i < shared.shareddata.nnav; i++) {
			fprintf(stderr, "dbg2       nav %d active:        %d\n", i, shared.shareddata.navs[i].active);
			fprintf(stderr, "dbg2       nav %d color:         %d\n", i, shared.shareddata.navs[i].color);
			fprintf(stderr, "dbg2       nav %d size:          %d\n", i, shared.shareddata.navs[i].size);
			fprintf(stderr, "dbg2       nav %d name:          %s\n", i, shared.shareddata.navs[i].name);
			fprintf(stderr, "dbg2       nav %d pathstatus:    %d\n", i, shared.shareddata.navs[i].pathstatus);
			fprintf(stderr, "dbg2       nav %d pathraw:       %s\n", i, shared.shareddata.navs[i].pathraw);
			fprintf(stderr, "dbg2       nav %d pathprocessed: %s\n", i, shared.shareddata.navs[i].pathprocessed);
			fprintf(stderr, "dbg2       nav %d swathbounds:   %d\n", i, shared.shareddata.navs[i].swathbounds);
			fprintf(stderr, "dbg2       nav %d line:          %d\n", i, shared.shareddata.navs[i].line);
			fprintf(stderr, "dbg2       nav %d shot:          %d\n", i, shared.shareddata.navs[i].shot);
			fprintf(stderr, "dbg2       nav %d cdp:           %d\n", i, shared.shareddata.navs[i].cdp);
			fprintf(stderr, "dbg2       nav %d decimation:    %d\n", i, shared.shareddata.navs[i].decimation);
			fprintf(stderr, "dbg2       nav %d npoints:       %d\n", i, shared.shareddata.navs[i].npoints);
			fprintf(stderr, "dbg2       nav %d npoints_alloc: %d\n", i, shared.shareddata.navs[i].npoints_alloc);
			fprintf(stderr, "dbg2       nav %d nselected:     %d\n", i, shared.shareddata.navs[i].nselected);
			for (int j = 0; j < shared.shareddata.navs[i].npoints; j++) {
				fprintf(stderr, "dbg2       nav %d %d draped:   %d\n", i, j, shared.shareddata.navs[i].navpts[j].draped);
				fprintf(stderr, "dbg2       nav %d %d selected: %d\n", i, j, shared.shareddata.navs[i].navpts[j].selected);
				fprintf(stderr, "dbg2       nav %d %d time_d:   %f\n", i, j, shared.shareddata.navs[i].navpts[j].time_d);
				fprintf(stderr, "dbg2       nav %d %d heading:  %f\n", i, j, shared.shareddata.navs[i].navpts[j].heading);
				fprintf(stderr, "dbg2       nav %d %d speed:    %f\n", i, j, shared.shareddata.navs[i].navpts[j].speed);
				fprintf(stderr, "dbg2       nav %d %d line:     %d\n", i, j, shared.shareddata.navs[i].navpts[j].line);
				fprintf(stderr, "dbg2       nav %d %d shot:     %d\n", i, j, shared.shareddata.navs[i].navpts[j].shot);
				fprintf(stderr, "dbg2       nav %d %d cdp:      %d\n", i, j, shared.shareddata.navs[i].navpts[j].cdp);

				fprintf(stderr, "dbg2       nav %d %d xgrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.xgrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d ygrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.ygrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d xlon:     %f\n", i, j, shared.shareddata.navs[i].navpts[j].point.xlon);
				fprintf(stderr, "dbg2       nav %d %d ylat:     %f\n", i, j, shared.shareddata.navs[i].navpts[j].point.ylat);
				fprintf(stderr, "dbg2       nav %d %d zdata:    %f\n", i, j, shared.shareddata.navs[i].navpts[j].point.zdata);
				fprintf(stderr, "dbg2       nav %d %d xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.xdisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.ydisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.zdisplay[instance]);

				fprintf(stderr, "dbg2       nav %d %d stbd xgrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.xgrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d stbd ygrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.ygrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d stbd xlon:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.xlon);
				fprintf(stderr, "dbg2       nav %d %d stbd ylat:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.ylat);
				fprintf(stderr, "dbg2       nav %d %d stbd zdata:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.zdata);
				fprintf(stderr, "dbg2       nav %d %d stbd xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.xdisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d stbd ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.ydisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d stbd zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.zdisplay[instance]);

				fprintf(stderr, "dbg2       nav %d %d cntr xgrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.xgrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d cntr ygrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.ygrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d cntr xlon:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.xlon);
				fprintf(stderr, "dbg2       nav %d %d cntr ylat:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.ylat);
				fprintf(stderr, "dbg2       nav %d %d cntr zdata:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.zdata);
				fprintf(stderr, "dbg2       nav %d %d cntr xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.xdisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d cntr ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.ydisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d cntr zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.zdisplay[instance]);

				fprintf(stderr, "dbg2       nav %d %d port xgrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.xgrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d port ygrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.ygrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d port xlon:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.xlon);
				fprintf(stderr, "dbg2       nav %d %d port ylat:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.ylat);
				fprintf(stderr, "dbg2       nav %d %d port zdata:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.zdata);
				fprintf(stderr, "dbg2       nav %d %d port xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.xdisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d port ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.ydisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d port zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.zdisplay[instance]);
			}
			for (int j = 0; j < shared.shareddata.navs[i].npoints - 1; j++) {
				fprintf(stderr, "dbg2       nav %d %d nls:          %d\n", i, j, shared.shareddata.navs[i].segments[j].nls);
				fprintf(stderr, "dbg2       nav %d %d nls_alloc:    %d\n", i, j, shared.shareddata.navs[i].segments[j].nls_alloc);
				fprintf(stderr, "dbg2       nav %d %d endpoints[0]: %p\n", i, j,
				        &shared.shareddata.navs[i].segments[j].endpoints[0]);
				fprintf(stderr, "dbg2       nav %d %d endpoints[1]: %p\n", i, j,
				        &shared.shareddata.navs[i].segments[j].endpoints[1]);
			}
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_enableviewnavs(int verbose, size_t instance, int *error)
{
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	/* set values */
	shared.shareddata.nav_mode = MBV_NAV_VIEW;

	/* set widget sensitivity on all active instances */
	for (instance = 0; instance < MBV_MAX_WINDOWS; instance++) {
		struct mbview_world_struct *view = &(mbviews[instance]);
		struct mbview_struct *data = &(view->data);

		/* if instance active reset action sensitivity */
		if (data->active)
			mbview_update_sensitivity(verbose, instance, error);
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_enableadjustnavs(int verbose, size_t instance, int *error)
{
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	/* set values */
	shared.shareddata.nav_mode = MBV_NAV_MBNAVADJUST;

	/* set widget sensitivity on all active instances */
	for (instance = 0; instance < MBV_MAX_WINDOWS; instance++) {
			struct mbview_world_struct *view = &(mbviews[instance]);
		struct mbview_struct *data = &(view->data);

		/* if instance active reset action sensitivity */
		if (data->active)
			mbview_update_sensitivity(verbose, instance, error);
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_pick_nav_select(size_t instance, int select, int which, int xpixel, int ypixel) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       select:           %d\n", select);
		fprintf(stderr, "dbg2       which:            %d\n", which);
		fprintf(stderr, "dbg2       xpixel:           %d\n", xpixel);
		fprintf(stderr, "dbg2       ypixel:           %d\n", ypixel);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* only work if there is nav */
	if (shared.shareddata.nav_mode != MBV_NAV_OFF && shared.shareddata.nnav > 0) {
		/* deal with MBV_NAV_VIEW mode */
		if (shared.shareddata.nav_mode == MBV_NAV_VIEW) {
			/* select first pick - usually this is an MBV_PICK_DOWN event */
			if (which == MBV_PICK_DOWN || shared.shareddata.nav_selected[0] == MBV_SELECT_NONE) {
				/* look for point */
				bool found;
				double xgrid;
				double ygrid;
				double xlon;
				double ylat;
				double zdata;
				double xdisplay;
				double ydisplay;
				double zdisplay;
				mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay,
				                 &zdisplay);

				/* look for nearest nav point */
				if (found) {
					double rrmin = 1000000000.0;
					shared.shareddata.nav_selected[0] = MBV_SELECT_NONE;
					shared.shareddata.nav_point_selected[0] = MBV_SELECT_NONE;
					shared.shareddata.nav_selected[1] = MBV_SELECT_NONE;
					shared.shareddata.nav_point_selected[1] = MBV_SELECT_NONE;

					for (int i = 0; i < shared.shareddata.nnav; i++) {
            if (shared.shareddata.navs[i].active) {
  						for (int j = 0; j < shared.shareddata.navs[i].npoints; j++) {
  							const double xx = xgrid - shared.shareddata.navs[i].navpts[j].point.xgrid[instance];
  							const double yy = ygrid - shared.shareddata.navs[i].navpts[j].point.ygrid[instance];
  							const double rr = sqrt(xx * xx + yy * yy);
  							if (rr < rrmin) {
  								rrmin = rr;
  								shared.shareddata.nav_selected[0] = i;
  								shared.shareddata.nav_point_selected[0] = j;
  							}
  						}
            }
					}

					/* set pick location */
					data->pickinfo_mode = MBV_PICK_NAV;
					shared.shareddata.navpick_type = MBV_PICK_ONEPOINT;
					shared.shareddata.navpick.endpoints[0].xgrid[instance] =
					    shared.shareddata.navs[shared.shareddata.nav_selected[0]]
					        .navpts[shared.shareddata.nav_point_selected[0]]
					        .point.xgrid[instance];
					shared.shareddata.navpick.endpoints[0].ygrid[instance] =
					    shared.shareddata.navs[shared.shareddata.nav_selected[0]]
					        .navpts[shared.shareddata.nav_point_selected[0]]
					        .point.ygrid[instance];
					shared.shareddata.navpick.endpoints[0].xlon = shared.shareddata.navs[shared.shareddata.nav_selected[0]]
					                                                  .navpts[shared.shareddata.nav_point_selected[0]]
					                                                  .point.xlon;
					shared.shareddata.navpick.endpoints[0].ylat = shared.shareddata.navs[shared.shareddata.nav_selected[0]]
					                                                  .navpts[shared.shareddata.nav_point_selected[0]]
					                                                  .point.ylat;
					shared.shareddata.navpick.endpoints[0].zdata = shared.shareddata.navs[shared.shareddata.nav_selected[0]]
					                                                   .navpts[shared.shareddata.nav_point_selected[0]]
					                                                   .point.zdata;
					shared.shareddata.navpick.endpoints[0].xdisplay[instance] =
					    shared.shareddata.navs[shared.shareddata.nav_selected[0]]
					        .navpts[shared.shareddata.nav_point_selected[0]]
					        .point.xdisplay[instance];
					shared.shareddata.navpick.endpoints[0].ydisplay[instance] =
					    shared.shareddata.navs[shared.shareddata.nav_selected[0]]
					        .navpts[shared.shareddata.nav_point_selected[0]]
					        .point.ydisplay[instance];
					shared.shareddata.navpick.endpoints[0].zdisplay[instance] =
					    shared.shareddata.navs[shared.shareddata.nav_selected[0]]
					        .navpts[shared.shareddata.nav_point_selected[0]]
					        .point.zdisplay[instance];

					/* get pick positions for all active instances */
					mbview_updatepointw(instance, &(shared.shareddata.navpick.endpoints[0]));

					/* generate 3D drape of pick marks  */
					mbview_navpicksize(instance);
				}
				else {
					/* unselect nav pick */
					data->pickinfo_mode = data->pick_type;
					shared.shareddata.navpick_type = MBV_PICK_NONE;
					shared.shareddata.nav_selected[0] = MBV_SELECT_NONE;
					shared.shareddata.nav_point_selected[0] = MBV_SELECT_NONE;
					XBell(view->dpy, 100);
				}
			}

			/* select second point if MBV_PICK_MOVE event */
			else if (which == MBV_PICK_MOVE) {
				/* look for point */
				bool found;
				double xgrid;
				double ygrid;
				double xlon;
				double ylat;
				double zdata;
				double xdisplay;
				double ydisplay;
				double zdisplay;
				mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay,
				                 &zdisplay);

				/* look for nearest nav point */
				if (found) {
					double rrmin = 1000000000.0;
					shared.shareddata.nav_selected[1] = MBV_SELECT_NONE;
					shared.shareddata.nav_point_selected[1] = MBV_SELECT_NONE;

					for (int i = 0; i < shared.shareddata.nnav; i++) {
            if (shared.shareddata.navs[i].active) {
  						for (int j = 0; j < shared.shareddata.navs[i].npoints; j++) {
  							const double xx = xgrid - shared.shareddata.navs[i].navpts[j].point.xgrid[instance];
  							const double yy = ygrid - shared.shareddata.navs[i].navpts[j].point.ygrid[instance];
  							const double rr = sqrt(xx * xx + yy * yy);
  							if (rr < rrmin) {
  								rrmin = rr;
  								shared.shareddata.nav_selected[1] = i;
  								shared.shareddata.nav_point_selected[1] = j;
  							}
  						}
            }
					}

					/* set pick location */
					data->pickinfo_mode = MBV_PICK_NAV;
					shared.shareddata.navpick_type = MBV_PICK_TWOPOINT;
					shared.shareddata.navpick.endpoints[1].xgrid[instance] =
					    shared.shareddata.navs[shared.shareddata.nav_selected[1]]
					        .navpts[shared.shareddata.nav_point_selected[1]]
					        .point.xgrid[instance];
					shared.shareddata.navpick.endpoints[1].ygrid[instance] =
					    shared.shareddata.navs[shared.shareddata.nav_selected[1]]
					        .navpts[shared.shareddata.nav_point_selected[1]]
					        .point.ygrid[instance];
					shared.shareddata.navpick.endpoints[1].xlon = shared.shareddata.navs[shared.shareddata.nav_selected[1]]
					                                                  .navpts[shared.shareddata.nav_point_selected[1]]
					                                                  .point.xlon;
					shared.shareddata.navpick.endpoints[1].ylat = shared.shareddata.navs[shared.shareddata.nav_selected[1]]
					                                                  .navpts[shared.shareddata.nav_point_selected[1]]
					                                                  .point.ylat;
					shared.shareddata.navpick.endpoints[1].zdata = shared.shareddata.navs[shared.shareddata.nav_selected[1]]
					                                                   .navpts[shared.shareddata.nav_point_selected[1]]
					                                                   .point.zdata;
					shared.shareddata.navpick.endpoints[1].xdisplay[instance] =
					    shared.shareddata.navs[shared.shareddata.nav_selected[1]]
					        .navpts[shared.shareddata.nav_point_selected[1]]
					        .point.xdisplay[instance];
					shared.shareddata.navpick.endpoints[1].ydisplay[instance] =
					    shared.shareddata.navs[shared.shareddata.nav_selected[1]]
					        .navpts[shared.shareddata.nav_point_selected[1]]
					        .point.ydisplay[instance];
					shared.shareddata.navpick.endpoints[1].zdisplay[instance] =
					    shared.shareddata.navs[shared.shareddata.nav_selected[1]]
					        .navpts[shared.shareddata.nav_point_selected[1]]
					        .point.zdisplay[instance];

					/* get pick positions for all active instances */
					mbview_updatepointw(instance, &(shared.shareddata.navpick.endpoints[1]));

					/* generate 3D drape of pick marks */
					mbview_navpicksize(instance);
				}
			}

			/* deal with MBV_PICK_UP event */
			else if (which == MBV_PICK_UP) {
				/* if data->mouse_mode == MBV_MOUSE_NAV only select or deselect
				    range of nav points if enabled and two different points selected */
				if (data->mouse_mode == MBV_MOUSE_NAV) {
					/* select range of nav if two different points have been selected */
					if (shared.shareddata.nav_selected[0] != MBV_SELECT_NONE &&
					    shared.shareddata.nav_selected[1] != MBV_SELECT_NONE) {
						/* get order of selected nav points */
						const int inav0 = MIN(shared.shareddata.nav_selected[0], shared.shareddata.nav_selected[1]);
						const int inav1 = MAX(shared.shareddata.nav_selected[0], shared.shareddata.nav_selected[1]);
						int jpt0;
						int jpt1;
						if (inav0 == inav1) {
							jpt0 = MIN(shared.shareddata.nav_point_selected[0], shared.shareddata.nav_point_selected[1]);
							jpt1 = MAX(shared.shareddata.nav_point_selected[0], shared.shareddata.nav_point_selected[1]);
						}
						else if (shared.shareddata.nav_selected[0] <= shared.shareddata.nav_selected[1]) {
							jpt0 = shared.shareddata.nav_point_selected[0];
							jpt1 = shared.shareddata.nav_point_selected[1];
						}
						else {
							jpt0 = shared.shareddata.nav_point_selected[1];
							jpt1 = shared.shareddata.nav_point_selected[0];
						}

						/* loop over the affected nav */
						for (int inav = inav0; inav <= inav1; inav++) {
							int jj0;
							if (inav == inav0)
								jj0 = MIN(jpt0, shared.shareddata.navs[inav].npoints - 1);
							else
								jj0 = 0;
							int jj1;
							if (inav == inav1)
								jj1 = MAX(jpt1, 0);
							else
								jj1 = shared.shareddata.navs[inav].npoints;
							for (int jpt = jj0; jpt <= jj1; jpt++) {
								shared.shareddata.navs[inav].navpts[jpt].selected = select;
							}
							shared.shareddata.navs[inav].nselected = 0;
							for (int jpt = 0; jpt < shared.shareddata.navs[inav].npoints; jpt++) {
								if (shared.shareddata.navs[inav].navpts[jpt].selected)
									shared.shareddata.navs[inav].nselected++;
							}
						}
					}

					/* else select single nav point */
					else if (shared.shareddata.nav_selected[0] != MBV_SELECT_NONE) {
						int inav = shared.shareddata.nav_selected[0];
						int jpt = shared.shareddata.nav_point_selected[0];
						shared.shareddata.navs[inav].navpts[jpt].selected = select;
						shared.shareddata.navs[inav].nselected = 0;
						for (jpt = 0; jpt < shared.shareddata.navs[inav].npoints; jpt++) {
							if (shared.shareddata.navs[inav].navpts[jpt].selected)
								shared.shareddata.navs[inav].nselected++;
						}
					}
				}

				/* if data->mouse_mode == MBV_MOUSE_NAVFILE & view mode select
				    or deselect all affected files */
				else if (data->mouse_mode == MBV_MOUSE_NAVFILE) {
					/* select range of nav files if one or two different points have been selected */
					if (shared.shareddata.nav_selected[0] != MBV_SELECT_NONE) {
						/* get order of selected nav points */
						int inav0;
						int inav1;
						if (shared.shareddata.nav_selected[1] != MBV_SELECT_NONE) {
							inav0 = MIN(shared.shareddata.nav_selected[0], shared.shareddata.nav_selected[1]);
							inav1 = MAX(shared.shareddata.nav_selected[0], shared.shareddata.nav_selected[1]);
						}
						else {
							inav0 = shared.shareddata.nav_selected[0];
							inav1 = shared.shareddata.nav_selected[0];
						}

						/* loop over the affected nav */
						for (int inav = inav0; inav <= inav1; inav++) {
							for (int jpt = 0; jpt < shared.shareddata.navs[inav].npoints; jpt++) {
								shared.shareddata.navs[inav].navpts[jpt].selected = select;
							}
							shared.shareddata.navs[inav].nselected = 0;
							for (int jpt = 0; jpt < shared.shareddata.navs[inav].npoints; jpt++) {
								if (shared.shareddata.navs[inav].navpts[jpt].selected)
									shared.shareddata.navs[inav].nselected++;
							}
						}
					}
				}

				/* call pick notify if defined */
				if (data->mbview_picknav_notify != NULL && shared.shareddata.nav_selected[0] != MBV_SELECT_NONE) {
					(data->mbview_picknav_notify)(instance);
				}
			}
		}

		/* deal with MBV_NAV_NAVADJUST mode (note: data->mouse_mode will be MBV_MOUSE_NAVFILE only) */
		else if (shared.shareddata.nav_mode == MBV_NAV_MBNAVADJUST) {
			/* select first pick - usually this is an MBV_PICK_DOWN event */
			if (which == MBV_PICK_DOWN || shared.shareddata.nav_selected[0] == MBV_SELECT_NONE) {
				/* delete all previous standard nav selections */
				shared.shareddata.nav_selected[0] = MBV_SELECT_NONE;
				shared.shareddata.nav_point_selected[0] = MBV_SELECT_NONE;
				shared.shareddata.nav_selected[1] = MBV_SELECT_NONE;
				shared.shareddata.nav_point_selected[1] = MBV_SELECT_NONE;
				for (int i = 0; i < shared.shareddata.nnav; i++) {
					for (int j = 0; j < shared.shareddata.navs[i].npoints; j++) {
						shared.shareddata.navs[i].navpts[j].selected = false;
					}
				}

				/* look for point */
				bool found;
				double xgrid;
				double ygrid;
				double xlon;
				double ylat;
				double zdata;
				double xdisplay;
				double ydisplay;
				double zdisplay;
				mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay,
				                 &zdisplay);

				/* look for nearest nav point */
				if (found) {
					double rrmin = 1000000000.0;
					for (int i = 0; i < shared.shareddata.nnav; i++) {
            if (shared.shareddata.navs[i].active) {
  						for (int j = 0; j < shared.shareddata.navs[i].npoints; j++) {
  							const double xx = xgrid - shared.shareddata.navs[i].navpts[j].point.xgrid[instance];
  							const double yy = ygrid - shared.shareddata.navs[i].navpts[j].point.ygrid[instance];
  							const double rr = sqrt(xx * xx + yy * yy);
  							if (rr < rrmin) {
  								rrmin = rr;
  								shared.shareddata.nav_selected[0] = i;
  								shared.shareddata.nav_point_selected[0] = j;
  							}
  						}
            }
					}

					/* set pick location */
					data->pickinfo_mode = MBV_PICK_NAV;
					shared.shareddata.navpick_type = MBV_PICK_ONEPOINT;
					shared.shareddata.navpick.endpoints[0].xgrid[instance] =
					    shared.shareddata.navs[shared.shareddata.nav_selected[0]]
					        .navpts[shared.shareddata.nav_point_selected[0]]
					        .point.xgrid[instance];
					shared.shareddata.navpick.endpoints[0].ygrid[instance] =
					    shared.shareddata.navs[shared.shareddata.nav_selected[0]]
					        .navpts[shared.shareddata.nav_point_selected[0]]
					        .point.ygrid[instance];
					shared.shareddata.navpick.endpoints[0].xlon = shared.shareddata.navs[shared.shareddata.nav_selected[0]]
					                                                  .navpts[shared.shareddata.nav_point_selected[0]]
					                                                  .point.xlon;
					shared.shareddata.navpick.endpoints[0].ylat = shared.shareddata.navs[shared.shareddata.nav_selected[0]]
					                                                  .navpts[shared.shareddata.nav_point_selected[0]]
					                                                  .point.ylat;
					shared.shareddata.navpick.endpoints[0].zdata = shared.shareddata.navs[shared.shareddata.nav_selected[0]]
					                                                   .navpts[shared.shareddata.nav_point_selected[0]]
					                                                   .point.zdata;
					shared.shareddata.navpick.endpoints[0].xdisplay[instance] =
					    shared.shareddata.navs[shared.shareddata.nav_selected[0]]
					        .navpts[shared.shareddata.nav_point_selected[0]]
					        .point.xdisplay[instance];
					shared.shareddata.navpick.endpoints[0].ydisplay[instance] =
					    shared.shareddata.navs[shared.shareddata.nav_selected[0]]
					        .navpts[shared.shareddata.nav_point_selected[0]]
					        .point.ydisplay[instance];
					shared.shareddata.navpick.endpoints[0].zdisplay[instance] =
					    shared.shareddata.navs[shared.shareddata.nav_selected[0]]
					        .navpts[shared.shareddata.nav_point_selected[0]]
					        .point.zdisplay[instance];

					/* get pick positions for all active instances */
					mbview_updatepointw(instance, &(shared.shareddata.navpick.endpoints[0]));

					/* generate 3D drape of pick marks  */
					mbview_navpicksize(instance);
				}
				else {
					/* unselect nav pick */
					data->pickinfo_mode = data->pick_type;
					shared.shareddata.navpick_type = MBV_PICK_NONE;
					shared.shareddata.nav_selected[0] = MBV_SELECT_NONE;
					shared.shareddata.nav_point_selected[0] = MBV_SELECT_NONE;
					XBell(view->dpy, 100);
				}
			}

			/* select second point if MBV_PICK_MOVE event */
			else if (which == MBV_PICK_MOVE) {
				/* look for point */
				bool found;
				double xgrid;
				double ygrid;
				double xlon;
				double ylat;
				double zdata;
				double xdisplay;
				double ydisplay;
				double zdisplay;
				mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay,
				                 &zdisplay);

				/* look for nearest nav point */
				if (found) {
					double rrmin = 1000000000.0;
					shared.shareddata.nav_selected[1] = MBV_SELECT_NONE;
					shared.shareddata.nav_point_selected[1] = MBV_SELECT_NONE;

					for (int i = 0; i < shared.shareddata.nnav; i++) {
            if (shared.shareddata.navs[i].active) {
  						for (int j = 0; j < shared.shareddata.navs[i].npoints; j++) {
  							const double xx = xgrid - shared.shareddata.navs[i].navpts[j].point.xgrid[instance];
  							const double yy = ygrid - shared.shareddata.navs[i].navpts[j].point.ygrid[instance];
  							const double rr = sqrt(xx * xx + yy * yy);
  							if (rr < rrmin) {
  								rrmin = rr;
  								shared.shareddata.nav_selected[1] = i;
  								shared.shareddata.nav_point_selected[1] = j;
  							}
  						}
            }
					}

					/* set pick location */
					data->pickinfo_mode = MBV_PICK_NAV;
					shared.shareddata.navpick_type = MBV_PICK_TWOPOINT;
					shared.shareddata.navpick.endpoints[1].xgrid[instance] =
					    shared.shareddata.navs[shared.shareddata.nav_selected[1]]
					        .navpts[shared.shareddata.nav_point_selected[1]]
					        .point.xgrid[instance];
					shared.shareddata.navpick.endpoints[1].ygrid[instance] =
					    shared.shareddata.navs[shared.shareddata.nav_selected[1]]
					        .navpts[shared.shareddata.nav_point_selected[1]]
					        .point.ygrid[instance];
					shared.shareddata.navpick.endpoints[1].xlon = shared.shareddata.navs[shared.shareddata.nav_selected[1]]
					                                                  .navpts[shared.shareddata.nav_point_selected[1]]
					                                                  .point.xlon;
					shared.shareddata.navpick.endpoints[1].ylat = shared.shareddata.navs[shared.shareddata.nav_selected[1]]
					                                                  .navpts[shared.shareddata.nav_point_selected[1]]
					                                                  .point.ylat;
					shared.shareddata.navpick.endpoints[1].zdata = shared.shareddata.navs[shared.shareddata.nav_selected[1]]
					                                                   .navpts[shared.shareddata.nav_point_selected[1]]
					                                                   .point.zdata;
					shared.shareddata.navpick.endpoints[1].xdisplay[instance] =
					    shared.shareddata.navs[shared.shareddata.nav_selected[1]]
					        .navpts[shared.shareddata.nav_point_selected[1]]
					        .point.xdisplay[instance];
					shared.shareddata.navpick.endpoints[1].ydisplay[instance] =
					    shared.shareddata.navs[shared.shareddata.nav_selected[1]]
					        .navpts[shared.shareddata.nav_point_selected[1]]
					        .point.ydisplay[instance];
					shared.shareddata.navpick.endpoints[1].zdisplay[instance] =
					    shared.shareddata.navs[shared.shareddata.nav_selected[1]]
					        .navpts[shared.shareddata.nav_point_selected[1]]
					        .point.zdisplay[instance];

					/* get pick positions for all active instances */
					mbview_updatepointw(instance, &(shared.shareddata.navpick.endpoints[1]));

					/* generate 3D drape of pick marks */
					mbview_navpicksize(instance);
				}
			}

			/* deal with MBV_PICK_UP event */
			else if (which == MBV_PICK_UP) {
				/* select range of nav files if one or two different points have been selected */
				if (shared.shareddata.nav_selected[0] != MBV_SELECT_NONE) {
					/* two different files picked - take both */
					if (shared.shareddata.nav_selected[1] != MBV_SELECT_NONE &&
					    shared.shareddata.nav_selected[0] != shared.shareddata.nav_selected[1]) {
						shared.shareddata.nav_selected_mbnavadjust[0] =
						    MIN(shared.shareddata.nav_selected[0], shared.shareddata.nav_selected[1]);
						shared.shareddata.nav_selected_mbnavadjust[1] =
						    MAX(shared.shareddata.nav_selected[0], shared.shareddata.nav_selected[1]);
					}

					/* else one file picked and at least one valid pick already to be kept */
					else if (shared.shareddata.nav_selected_mbnavadjust[0] != MBV_SELECT_NONE) {
						shared.shareddata.nav_selected_mbnavadjust[1] = shared.shareddata.nav_selected_mbnavadjust[0];
						shared.shareddata.nav_selected_mbnavadjust[0] = shared.shareddata.nav_selected[0];
					}

					/* else one file picked and no previous pick */
					else {
						shared.shareddata.nav_selected_mbnavadjust[0] = shared.shareddata.nav_selected[0];
						shared.shareddata.nav_selected_mbnavadjust[1] = MBV_SELECT_NONE;
					}

					/* clear all previous selection */
					for (int i = 0; i < shared.shareddata.nnav; i++) {
						for (int j = 0; j < shared.shareddata.navs[i].npoints; j++) {
							shared.shareddata.navs[i].navpts[j].selected = false;
						}
					}

					/* select the nav from the selected files */
					if (shared.shareddata.nav_selected_mbnavadjust[0] != MBV_SELECT_NONE) {
						shared.shareddata.nav_selected[0] = shared.shareddata.nav_selected_mbnavadjust[0];
						shared.shareddata.nav_point_selected[0] = 0;

						int inav = shared.shareddata.nav_selected_mbnavadjust[0];
						for (int jpt = 0; jpt < shared.shareddata.navs[inav].npoints; jpt++) {
							shared.shareddata.navs[inav].navpts[jpt].selected = select;
						}
						shared.shareddata.navs[inav].nselected = shared.shareddata.navs[inav].npoints;

						/* set pick location */
						data->pickinfo_mode = MBV_PICK_NAV;
						shared.shareddata.navpick_type = MBV_PICK_ONEPOINT;
						shared.shareddata.navpick.endpoints[0].xgrid[instance] =
						    shared.shareddata.navs[shared.shareddata.nav_selected[0]]
						        .navpts[shared.shareddata.nav_point_selected[0]]
						        .point.xgrid[instance];
						shared.shareddata.navpick.endpoints[0].ygrid[instance] =
						    shared.shareddata.navs[shared.shareddata.nav_selected[0]]
						        .navpts[shared.shareddata.nav_point_selected[0]]
						        .point.ygrid[instance];
						shared.shareddata.navpick.endpoints[0].xlon = shared.shareddata.navs[shared.shareddata.nav_selected[0]]
						                                                  .navpts[shared.shareddata.nav_point_selected[0]]
						                                                  .point.xlon;
						shared.shareddata.navpick.endpoints[0].ylat = shared.shareddata.navs[shared.shareddata.nav_selected[0]]
						                                                  .navpts[shared.shareddata.nav_point_selected[0]]
						                                                  .point.ylat;
						shared.shareddata.navpick.endpoints[0].zdata = shared.shareddata.navs[shared.shareddata.nav_selected[0]]
						                                                   .navpts[shared.shareddata.nav_point_selected[0]]
						                                                   .point.zdata;
						shared.shareddata.navpick.endpoints[0].xdisplay[instance] =
						    shared.shareddata.navs[shared.shareddata.nav_selected[0]]
						        .navpts[shared.shareddata.nav_point_selected[0]]
						        .point.xdisplay[instance];
						shared.shareddata.navpick.endpoints[0].ydisplay[instance] =
						    shared.shareddata.navs[shared.shareddata.nav_selected[0]]
						        .navpts[shared.shareddata.nav_point_selected[0]]
						        .point.ydisplay[instance];
						shared.shareddata.navpick.endpoints[0].zdisplay[instance] =
						    shared.shareddata.navs[shared.shareddata.nav_selected[0]]
						        .navpts[shared.shareddata.nav_point_selected[0]]
						        .point.zdisplay[instance];

						/* get pick positions for all active instances */
						mbview_updatepointw(instance, &(shared.shareddata.navpick.endpoints[0]));
					}
					if (shared.shareddata.nav_selected_mbnavadjust[1] != MBV_SELECT_NONE) {
						shared.shareddata.nav_selected[1] = shared.shareddata.nav_selected_mbnavadjust[1];
						shared.shareddata.nav_point_selected[1] = 0;

						int inav = shared.shareddata.nav_selected_mbnavadjust[1];
						for (int jpt = 0; jpt < shared.shareddata.navs[inav].npoints; jpt++) {
							shared.shareddata.navs[inav].navpts[jpt].selected = select;
						}
						shared.shareddata.navs[inav].nselected = shared.shareddata.navs[inav].npoints;

						/* set pick location */
						data->pickinfo_mode = MBV_PICK_NAV;
						shared.shareddata.navpick_type = MBV_PICK_TWOPOINT;
						shared.shareddata.navpick.endpoints[1].xgrid[instance] =
						    shared.shareddata.navs[shared.shareddata.nav_selected[1]]
						        .navpts[shared.shareddata.nav_point_selected[1]]
						        .point.xgrid[instance];
						shared.shareddata.navpick.endpoints[1].ygrid[instance] =
						    shared.shareddata.navs[shared.shareddata.nav_selected[1]]
						        .navpts[shared.shareddata.nav_point_selected[1]]
						        .point.ygrid[instance];
						shared.shareddata.navpick.endpoints[1].xlon = shared.shareddata.navs[shared.shareddata.nav_selected[1]]
						                                                  .navpts[shared.shareddata.nav_point_selected[1]]
						                                                  .point.xlon;
						shared.shareddata.navpick.endpoints[1].ylat = shared.shareddata.navs[shared.shareddata.nav_selected[1]]
						                                                  .navpts[shared.shareddata.nav_point_selected[1]]
						                                                  .point.ylat;
						shared.shareddata.navpick.endpoints[1].zdata = shared.shareddata.navs[shared.shareddata.nav_selected[1]]
						                                                   .navpts[shared.shareddata.nav_point_selected[1]]
						                                                   .point.zdata;
						shared.shareddata.navpick.endpoints[1].xdisplay[instance] =
						    shared.shareddata.navs[shared.shareddata.nav_selected[1]]
						        .navpts[shared.shareddata.nav_point_selected[1]]
						        .point.xdisplay[instance];
						shared.shareddata.navpick.endpoints[1].ydisplay[instance] =
						    shared.shareddata.navs[shared.shareddata.nav_selected[1]]
						        .navpts[shared.shareddata.nav_point_selected[1]]
						        .point.ydisplay[instance];
						shared.shareddata.navpick.endpoints[1].zdisplay[instance] =
						    shared.shareddata.navs[shared.shareddata.nav_selected[1]]
						        .navpts[shared.shareddata.nav_point_selected[1]]
						        .point.zdisplay[instance];

						/* get pick positions for all active instances */
						mbview_updatepointw(instance, &(shared.shareddata.navpick.endpoints[1]));
					}

					/* generate 3D drape of pick marks */
					if (shared.shareddata.nav_selected_mbnavadjust[0] != MBV_SELECT_NONE) {
						mbview_navpicksize(instance);
					}
				}

				/* call pick notify if defined */
				if (data->mbview_picknav_notify != NULL && (shared.shareddata.nav_selected_mbnavadjust[0] != MBV_SELECT_NONE ||
				                                            shared.shareddata.nav_selected_mbnavadjust[1] != MBV_SELECT_NONE)) {
					(data->mbview_picknav_notify)(instance);
				}
			}
		}
	}

	/* else beep */
	else {
		shared.shareddata.nav_selected[0] = MBV_SELECT_NONE;
		shared.shareddata.nav_point_selected[0] = MBV_SELECT_NONE;
		shared.shareddata.nav_selected[1] = MBV_SELECT_NONE;
		shared.shareddata.nav_point_selected[1] = MBV_SELECT_NONE;
		shared.shareddata.nav_selected_mbnavadjust[0] = MBV_SELECT_NONE;
		shared.shareddata.nav_selected_mbnavadjust[1] = MBV_SELECT_NONE;
		XBell(view->dpy, 100);
		for (int i = 0; i < shared.shareddata.nnav; i++) {
			for (int j = 0; j < shared.shareddata.navs[i].npoints; j++) {
				shared.shareddata.navs[i].navpts[j].selected = false;
			}
		}
	}

	/* set what kind of pick to annotate */
	if (shared.shareddata.nav_selected[0] != MBV_SELECT_NONE) {
		data->pickinfo_mode = MBV_PICK_NAV;
	}
	else {
		data->pickinfo_mode = data->pick_type;
	}

	/* update nav data list */
	mbview_updatenavlist();

	/* set pick annotation */
	mbview_pick_text(instance);

	/* print nav debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  Nav data altered in function <%s>\n", __func__);
		fprintf(stderr, "dbg2  Nav values:\n");
		fprintf(stderr, "dbg2       nav_mode:              %d\n", shared.shareddata.nav_mode);
		fprintf(stderr, "dbg2       nav_view_mode:         %d\n", data->nav_view_mode);
		fprintf(stderr, "dbg2       navdrape_view_mode:    %d\n", data->navdrape_view_mode);
		fprintf(stderr, "dbg2       nnav:                  %d\n", shared.shareddata.nnav);
		fprintf(stderr, "dbg2       nnav_alloc:            %d\n", shared.shareddata.nnav_alloc);
		fprintf(stderr, "dbg2       nav_selected[0]:       %d\n", shared.shareddata.nav_selected[0]);
		fprintf(stderr, "dbg2       nav_point_selected[0]: %d\n", shared.shareddata.nav_point_selected[0]);
		fprintf(stderr, "dbg2       nav_selected[1]:       %d\n", shared.shareddata.nav_selected[1]);
		fprintf(stderr, "dbg2       nav_point_selected[1]: %d\n", shared.shareddata.nav_point_selected[1]);
		for (int i = 0; i < shared.shareddata.nnav; i++) {
			fprintf(stderr, "dbg2       nav %d active:        %d\n", i, shared.shareddata.navs[i].active);
			fprintf(stderr, "dbg2       nav %d color:         %d\n", i, shared.shareddata.navs[i].color);
			fprintf(stderr, "dbg2       nav %d size:          %d\n", i, shared.shareddata.navs[i].size);
			fprintf(stderr, "dbg2       nav %d name:          %s\n", i, shared.shareddata.navs[i].name);
			fprintf(stderr, "dbg2       nav %d swathbounds:   %d\n", i, shared.shareddata.navs[i].swathbounds);
			fprintf(stderr, "dbg2       nav %d line:          %d\n", i, shared.shareddata.navs[i].line);
			fprintf(stderr, "dbg2       nav %d shot:          %d\n", i, shared.shareddata.navs[i].shot);
			fprintf(stderr, "dbg2       nav %d cdp:           %d\n", i, shared.shareddata.navs[i].cdp);
			fprintf(stderr, "dbg2       nav %d decimation:    %d\n", i, shared.shareddata.navs[i].decimation);
			fprintf(stderr, "dbg2       nav %d npoints:       %d\n", i, shared.shareddata.navs[i].npoints);
			fprintf(stderr, "dbg2       nav %d npoints_alloc: %d\n", i, shared.shareddata.navs[i].npoints_alloc);
			fprintf(stderr, "dbg2       nav %d nselected:     %d\n", i, shared.shareddata.navs[i].nselected);
			for (int j = 0; j < shared.shareddata.navs[i].npoints; j++) {
				fprintf(stderr, "dbg2       nav %d %d draped:   %d\n", i, j, shared.shareddata.navs[i].navpts[j].draped);
				fprintf(stderr, "dbg2       nav %d %d selected: %d\n", i, j, shared.shareddata.navs[i].navpts[j].selected);
				fprintf(stderr, "dbg2       nav %d %d time_d:   %f\n", i, j, shared.shareddata.navs[i].navpts[j].time_d);
				fprintf(stderr, "dbg2       nav %d %d heading:  %f\n", i, j, shared.shareddata.navs[i].navpts[j].heading);
				fprintf(stderr, "dbg2       nav %d %d speed:    %f\n", i, j, shared.shareddata.navs[i].navpts[j].speed);
				fprintf(stderr, "dbg2       nav %d %d line:     %d\n", i, j, shared.shareddata.navs[i].navpts[j].line);
				fprintf(stderr, "dbg2       nav %d %d shot:     %d\n", i, j, shared.shareddata.navs[i].navpts[j].shot);
				fprintf(stderr, "dbg2       nav %d %d cdp:      %d\n", i, j, shared.shareddata.navs[i].navpts[j].cdp);

				fprintf(stderr, "dbg2       nav %d %d xgrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.xgrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d ygrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.ygrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d xlon:     %f\n", i, j, shared.shareddata.navs[i].navpts[j].point.xlon);
				fprintf(stderr, "dbg2       nav %d %d ylat:     %f\n", i, j, shared.shareddata.navs[i].navpts[j].point.ylat);
				fprintf(stderr, "dbg2       nav %d %d zdata:    %f\n", i, j, shared.shareddata.navs[i].navpts[j].point.zdata);
				fprintf(stderr, "dbg2       nav %d %d xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.xdisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.ydisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.zdisplay[instance]);

				fprintf(stderr, "dbg2       nav %d %d stbd xgrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.xgrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d stbd ygrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.ygrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d stbd xlon:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.xlon);
				fprintf(stderr, "dbg2       nav %d %d stbd ylat:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.ylat);
				fprintf(stderr, "dbg2       nav %d %d stbd zdata:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.zdata);
				fprintf(stderr, "dbg2       nav %d %d stbd xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.xdisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d stbd ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.ydisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d stbd zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.zdisplay[instance]);

				fprintf(stderr, "dbg2       nav %d %d cntr xgrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.xgrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d cntr ygrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.ygrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d cntr xlon:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.xlon);
				fprintf(stderr, "dbg2       nav %d %d cntr ylat:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.ylat);
				fprintf(stderr, "dbg2       nav %d %d cntr zdata:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.zdata);
				fprintf(stderr, "dbg2       nav %d %d cntr xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.xdisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d cntr ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.ydisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d cntr zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.zdisplay[instance]);

				fprintf(stderr, "dbg2       nav %d %d port xgrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.xgrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d port ygrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.ygrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d port xlon:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.xlon);
				fprintf(stderr, "dbg2       nav %d %d port ylat:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.ylat);
				fprintf(stderr, "dbg2       nav %d %d port zdata:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.zdata);
				fprintf(stderr, "dbg2       nav %d %d port xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.xdisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d port ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.ydisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d port zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.zdisplay[instance]);
			}
			for (int j = 0; j < shared.shareddata.navs[i].npoints - 1; j++) {
				fprintf(stderr, "dbg2       nav %d %d nls:          %d\n", i, j, shared.shareddata.navs[i].segments[j].nls);
				fprintf(stderr, "dbg2       nav %d %d nls_alloc:    %d\n", i, j, shared.shareddata.navs[i].segments[j].nls_alloc);
				fprintf(stderr, "dbg2       nav %d %d endpoints[0]: %p\n", i, j,
				        &shared.shareddata.navs[i].segments[j].endpoints[0]);
				fprintf(stderr, "dbg2       nav %d %d endpoints[1]: %p\n", i, j,
				        &shared.shareddata.navs[i].segments[j].endpoints[1]);
			}
		}
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_extract_nav_profile(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	int status = MB_SUCCESS;

	/* if any nav exists, check if any are selected then extract the profile */
	if (shared.shareddata.nnav > 0) {
		data->profile.source = MBV_PROFILE_NAV;
		strcpy(data->profile.source_name, "Navigation");
		data->profile.length = 0.0;

		/* make sure enough memory is allocated for the profile */
		int nprpoints = 0;
		for (int i = 0; i < shared.shareddata.nnav; i++) {
			for (int j = 0; j < shared.shareddata.navs[i].npoints; j++) {
				if (shared.shareddata.navs[i].navpts[j].selected) {
					nprpoints++;
				}
			}
		}
		if (data->profile.npoints_alloc < nprpoints) {
			int error = MB_ERROR_NO_ERROR;
			status = mbview_allocprofilepoints(mbv_verbose, nprpoints, &(data->profile.points), &error);
			if (status == MB_SUCCESS) {
				data->profile.npoints_alloc = nprpoints;
			}
			else {
				data->profile.npoints_alloc = 0;
			}
		}

		/* extract the profile */
		if (nprpoints > 2 && data->profile.npoints_alloc >= nprpoints) {
			data->profile.npoints = 0;
			int lasti = 0;
			int lastj = 0;
			for (int i = 0; i < shared.shareddata.nnav; i++) {
				int firstj = -1;
				for (int j = 0; j < shared.shareddata.navs[i].npoints; j++) {
					if (shared.shareddata.navs[i].navpts[j].selected) {
						data->profile.points[data->profile.npoints].boundary = true;
						if (data->profile.npoints > 0 && i == lasti && j > 1 && lastj == j - 1 && j > 0 && firstj != j - 1)
							data->profile.points[data->profile.npoints - 1].boundary = false;
						lasti = i;
						lastj = j;
						if (firstj == -1)
							firstj = j;

						data->profile.points[data->profile.npoints].xgrid =
						    shared.shareddata.navs[i].navpts[j].pointcntr.xgrid[instance];
						data->profile.points[data->profile.npoints].ygrid =
						    shared.shareddata.navs[i].navpts[j].pointcntr.ygrid[instance];
						data->profile.points[data->profile.npoints].xlon = shared.shareddata.navs[i].navpts[j].pointcntr.xlon;
						data->profile.points[data->profile.npoints].ylat = shared.shareddata.navs[i].navpts[j].pointcntr.ylat;
						data->profile.points[data->profile.npoints].zdata = shared.shareddata.navs[i].navpts[j].pointcntr.zdata;
						data->profile.points[data->profile.npoints].xdisplay =
						    shared.shareddata.navs[i].navpts[j].pointcntr.xdisplay[instance];
						data->profile.points[data->profile.npoints].ydisplay =
						    shared.shareddata.navs[i].navpts[j].pointcntr.ydisplay[instance];
						if (data->profile.npoints == 0) {
							data->profile.zmin = data->profile.points[data->profile.npoints].zdata;
							data->profile.zmax = data->profile.points[data->profile.npoints].zdata;
							data->profile.points[data->profile.npoints].distance = 0.0;
							data->profile.points[data->profile.npoints].distovertopo = 0.0;
							data->profile.points[data->profile.npoints].bearing = 0.0;
						}
						else {
							data->profile.zmin = MIN(data->profile.zmin, data->profile.points[data->profile.npoints].zdata);
							data->profile.zmax = MAX(data->profile.zmax, data->profile.points[data->profile.npoints].zdata);
							if (data->display_projection_mode != MBV_PROJECTION_SPHEROID) {
								const double dx = data->profile.points[data->profile.npoints].xdisplay -
								     data->profile.points[data->profile.npoints - 1].xdisplay;
								const double dy = data->profile.points[data->profile.npoints].ydisplay -
								     data->profile.points[data->profile.npoints - 1].ydisplay;
								data->profile.points[data->profile.npoints].distance =
								    sqrt(dx * dx + dy * dy) / view->scale +
								    data->profile.points[data->profile.npoints - 1].distance;
								data->profile.points[data->profile.npoints].bearing = RTD * atan2(dx, dy);
							}
							else {
								mbview_greatcircle_distbearing(instance, data->profile.points[data->profile.npoints - 1].xlon,
								                               data->profile.points[data->profile.npoints - 1].ylat,
								                               data->profile.points[data->profile.npoints].xlon,
								                               data->profile.points[data->profile.npoints].ylat,
								                               &(data->profile.points[data->profile.npoints].bearing),
								                               &(data->profile.points[data->profile.npoints].distance));
								mbview_greatcircle_dist(instance, data->profile.points[0].xlon, data->profile.points[0].ylat,
								                        data->profile.points[data->profile.npoints].xlon,
								                        data->profile.points[data->profile.npoints].ylat,
								                        &(data->profile.points[data->profile.npoints].distance));
							}
							const double dy = (data->profile.points[data->profile.npoints].zdata -
							      data->profile.points[data->profile.npoints - 1].zdata);
							const double dx = (data->profile.points[data->profile.npoints].distance -
							      data->profile.points[data->profile.npoints - 1].distance);
							data->profile.points[data->profile.npoints].distovertopo =
							    data->profile.points[data->profile.npoints - 1].distovertopo + sqrt(dy * dy + dx * dx);
							if (dx > 0.0)
								data->profile.points[data->profile.npoints].slope = fabs(dy / dx);
							else
								data->profile.points[data->profile.npoints].slope = 0.0;
						}
						if (data->profile.points[data->profile.npoints].bearing < 0.0)
							data->profile.points[data->profile.npoints].bearing += 360.0;
						if (data->profile.npoints == 1)
							data->profile.points[0].bearing = data->profile.points[data->profile.npoints].bearing;
						if (data->profile.npoints > 1) {
							const double dy = (data->profile.points[data->profile.npoints].zdata -
							      data->profile.points[data->profile.npoints - 2].zdata);
							const double dx = (data->profile.points[data->profile.npoints].distance -
							      data->profile.points[data->profile.npoints - 2].distance);
							if (dx > 0.0)
								data->profile.points[data->profile.npoints - 1].slope = fabs(dy / dx);
							else
								data->profile.points[data->profile.npoints - 1].slope = 0.0;
						}
						data->profile.points[data->profile.npoints].navzdata = shared.shareddata.navs[i].navpts[j].point.zdata;
						;
						data->profile.points[data->profile.npoints].navtime_d = shared.shareddata.navs[i].navpts[j].time_d;
						data->profile.npoints++;
					}
					else {
						firstj = -1;
					}
				}
			}
			data->profile.length = data->profile.points[data->profile.npoints - 1].distance;

			/* calculate slope */
			for (int i = 0; i < data->profile.npoints; i++) {
				double dx;
				double dy;
				if (i == 0) {
					dy = (data->profile.points[i + 1].zdata - data->profile.points[i].zdata);
					dx = (data->profile.points[i + 1].distance - data->profile.points[i].distance);
				}
				else if (i == data->profile.npoints - 1) {
					dy = (data->profile.points[i].zdata - data->profile.points[i - 1].zdata);
					dx = (data->profile.points[i].distance - data->profile.points[i - 1].distance);
				}
				else {
					dy = (data->profile.points[i + 1].zdata - data->profile.points[i - 1].zdata);
					dx = (data->profile.points[i + 1].distance - data->profile.points[i - 1].distance);
				}
				if (dx > 0.0)
					data->profile.points[i].slope = fabs(dy / dx);
				else
					data->profile.points[i].slope = 0.0;
			}
		}
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_nav_delete(size_t instance, int inav) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       inav:            %d\n", inav);
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	// struct mbview_world_struct *view = &(mbviews[instance]);
	// struct mbview_struct *data = &(view->data);

	int status = MB_SUCCESS;

	/* delete nav if its the same as previously selected */
	if (inav >= 0 && inav < shared.shareddata.nnav) {
		/* free memory for deleted nav */
		int error = MB_ERROR_NO_ERROR;
		mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(shared.shareddata.navs[inav].navpts), &error);
		mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(shared.shareddata.navs[inav].segments), &error);

		/* move nav data if necessary */
		for (int i = inav; i < shared.shareddata.nnav - 1; i++) {
			shared.shareddata.navs[i] = shared.shareddata.navs[i + 1];
		}

		/* reset last nav */
		shared.shareddata.navs[shared.shareddata.nnav - 1].active = false;
		shared.shareddata.navs[shared.shareddata.nnav - 1].color = MBV_COLOR_RED;
		shared.shareddata.navs[shared.shareddata.nnav - 1].size = 4;
		shared.shareddata.navs[shared.shareddata.nnav - 1].name[0] = '\0';
		shared.shareddata.navs[shared.shareddata.nnav - 1].pathstatus = MB_PROCESSED_NONE;
		shared.shareddata.navs[shared.shareddata.nnav - 1].pathraw[0] = '\0';
		shared.shareddata.navs[shared.shareddata.nnav - 1].pathprocessed[0] = '\0';
		shared.shareddata.navs[shared.shareddata.nnav - 1].format = 0;
		shared.shareddata.navs[shared.shareddata.nnav - 1].swathbounds = false;
		shared.shareddata.navs[shared.shareddata.nnav - 1].line = false;
		shared.shareddata.navs[shared.shareddata.nnav - 1].shot = false;
		shared.shareddata.navs[shared.shareddata.nnav - 1].cdp = false;
		shared.shareddata.navs[shared.shareddata.nnav - 1].decimation = 1;
		shared.shareddata.navs[shared.shareddata.nnav - 1].npoints = 0;
		shared.shareddata.navs[shared.shareddata.nnav - 1].npoints_alloc = 0;
		shared.shareddata.navs[shared.shareddata.nnav - 1].navpts = NULL;
		shared.shareddata.navs[shared.shareddata.nnav - 1].segments = NULL;

		/* set nnav */
		shared.shareddata.nnav--;

		/* no selection */
		shared.shareddata.navpick_type = MBV_PICK_NONE;
		shared.shareddata.nav_selected[0] = MBV_SELECT_NONE;
		shared.shareddata.nav_selected[1] = MBV_SELECT_NONE;
		shared.shareddata.nav_point_selected[0] = MBV_SELECT_NONE;
		shared.shareddata.nav_point_selected[1] = MBV_SELECT_NONE;
		shared.shareddata.nav_selected_mbnavadjust[0] = MBV_SELECT_NONE;
		shared.shareddata.nav_selected_mbnavadjust[1] = MBV_SELECT_NONE;
	}
	else {
		status = MB_FAILURE;
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_navpicksize(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	double xlength;

	/* resize and redrape navpick marks if required */
	if (shared.shareddata.navpick_type != MBV_PICK_NONE) {
		/* set size of 'V' marks in gl units for 3D case */
		xlength = 0.05;
		const double headingx = sin(
		    shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].heading *
		    DTR);
		const double headingy = cos(
		    shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].heading *
		    DTR);

		/* set navpick location V marker */
		shared.shareddata.navpick.xpoints[0].xdisplay[instance] =
		    shared.shareddata.navpick.endpoints[0].xdisplay[instance] + xlength * (headingy - headingx);
		shared.shareddata.navpick.xpoints[0].ydisplay[instance] =
		    shared.shareddata.navpick.endpoints[0].ydisplay[instance] - xlength * (headingx + headingy);
		shared.shareddata.navpick.xpoints[1].xdisplay[instance] = shared.shareddata.navpick.endpoints[0].xdisplay[instance];
		shared.shareddata.navpick.xpoints[1].ydisplay[instance] = shared.shareddata.navpick.endpoints[0].ydisplay[instance];
		shared.shareddata.navpick.xpoints[2].xdisplay[instance] = shared.shareddata.navpick.endpoints[0].xdisplay[instance];
		shared.shareddata.navpick.xpoints[2].ydisplay[instance] = shared.shareddata.navpick.endpoints[0].ydisplay[instance];
		shared.shareddata.navpick.xpoints[3].xdisplay[instance] =
		    shared.shareddata.navpick.endpoints[0].xdisplay[instance] - xlength * (headingx + headingy);
		shared.shareddata.navpick.xpoints[3].ydisplay[instance] =
		    shared.shareddata.navpick.endpoints[0].ydisplay[instance] + xlength * (headingx - headingy);
		for (int i = 0; i < 4; i++) {
			mbview_projectinverse(
			    instance, true, shared.shareddata.navpick.xpoints[i].xdisplay[instance],
			    shared.shareddata.navpick.xpoints[i].ydisplay[instance], shared.shareddata.navpick.xpoints[i].zdisplay[instance],
			    &shared.shareddata.navpick.xpoints[i].xlon, &shared.shareddata.navpick.xpoints[i].ylat,
			    &shared.shareddata.navpick.xpoints[i].xgrid[instance], &shared.shareddata.navpick.xpoints[i].ygrid[instance]);
			bool found;
			mbview_getzdata(instance, shared.shareddata.navpick.xpoints[i].xgrid[instance],
			                shared.shareddata.navpick.xpoints[i].ygrid[instance], &found,
			                &shared.shareddata.navpick.xpoints[i].zdata);
			if (!found)
				shared.shareddata.navpick.xpoints[i].zdata = shared.shareddata.navpick.endpoints[0].zdata;
			mbview_projectll2display(instance, shared.shareddata.navpick.xpoints[i].xlon,
			                         shared.shareddata.navpick.xpoints[i].ylat, shared.shareddata.navpick.xpoints[i].zdata,
			                         &shared.shareddata.navpick.xpoints[i].xdisplay[instance],
			                         &shared.shareddata.navpick.xpoints[i].ydisplay[instance],
			                         &shared.shareddata.navpick.xpoints[i].zdisplay[instance]);
			mbview_updatepointw(instance, &(shared.shareddata.navpick.xpoints[i]));
		}

		/* drape the V marker line segments */
		for (int j = 0; j < 2; j++) {
			mbview_drapesegmentw(instance, &(shared.shareddata.navpick.xsegments[j]));
			mbview_updatesegmentw(instance, &(shared.shareddata.navpick.xsegments[j]));
		}
	}
	if (shared.shareddata.navpick_type == MBV_PICK_TWOPOINT) {
		const double headingx = sin(
		    shared.shareddata.navs[shared.shareddata.nav_selected[1]].navpts[shared.shareddata.nav_point_selected[1]].heading *
		    DTR);
		const double headingy = cos(
		    shared.shareddata.navs[shared.shareddata.nav_selected[1]].navpts[shared.shareddata.nav_point_selected[1]].heading *
		    DTR);

		/* set navpick location V marker */
		shared.shareddata.navpick.xpoints[4].xdisplay[instance] =
		    shared.shareddata.navpick.endpoints[1].xdisplay[instance] + xlength * (headingy - headingx);
		shared.shareddata.navpick.xpoints[4].ydisplay[instance] =
		    shared.shareddata.navpick.endpoints[1].ydisplay[instance] - xlength * (headingx + headingy);
		shared.shareddata.navpick.xpoints[5].xdisplay[instance] = shared.shareddata.navpick.endpoints[1].xdisplay[instance];
		shared.shareddata.navpick.xpoints[5].ydisplay[instance] = shared.shareddata.navpick.endpoints[1].ydisplay[instance];
		shared.shareddata.navpick.xpoints[6].xdisplay[instance] = shared.shareddata.navpick.endpoints[1].xdisplay[instance];
		shared.shareddata.navpick.xpoints[6].ydisplay[instance] = shared.shareddata.navpick.endpoints[1].ydisplay[instance];
		shared.shareddata.navpick.xpoints[7].xdisplay[instance] =
		    shared.shareddata.navpick.endpoints[1].xdisplay[instance] - xlength * (headingx + headingy);
		shared.shareddata.navpick.xpoints[7].ydisplay[instance] =
		    shared.shareddata.navpick.endpoints[1].ydisplay[instance] + xlength * (headingx - headingy);
		for (int i = 4; i < 8; i++) {
			mbview_projectinverse(
			    instance, true, shared.shareddata.navpick.xpoints[i].xdisplay[instance],
			    shared.shareddata.navpick.xpoints[i].ydisplay[instance], shared.shareddata.navpick.xpoints[i].zdisplay[instance],
			    &shared.shareddata.navpick.xpoints[i].xlon, &shared.shareddata.navpick.xpoints[i].ylat,
			    &shared.shareddata.navpick.xpoints[i].xgrid[instance], &shared.shareddata.navpick.xpoints[i].ygrid[instance]);
			bool found;
			mbview_getzdata(instance, shared.shareddata.navpick.xpoints[i].xgrid[instance],
			                shared.shareddata.navpick.xpoints[i].ygrid[instance], &found,
			                &shared.shareddata.navpick.xpoints[i].zdata);
			if (!found)
				shared.shareddata.navpick.xpoints[i].zdata = shared.shareddata.navpick.endpoints[1].zdata;
			shared.shareddata.navpick.xpoints[i].zdisplay[instance] =
			    view->scale * (data->exageration * shared.shareddata.navpick.xpoints[i].zdata - view->zorigin);
			mbview_projectll2display(instance, shared.shareddata.navpick.xpoints[i].xlon,
			                         shared.shareddata.navpick.xpoints[i].ylat, shared.shareddata.navpick.xpoints[i].zdata,
			                         &shared.shareddata.navpick.xpoints[i].xdisplay[instance],
			                         &shared.shareddata.navpick.xpoints[i].ydisplay[instance],
			                         &shared.shareddata.navpick.xpoints[i].zdisplay[instance]);
			mbview_updatepointw(instance, &(shared.shareddata.navpick.xpoints[i]));
		}

		/* drape the V marker line segments */
		for (int j = 2; j < 4; j++) {
			mbview_drapesegmentw(instance, &(shared.shareddata.navpick.xsegments[j]));
			mbview_updatesegmentw(instance, &(shared.shareddata.navpick.xsegments[j]));
		}
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_drawnavpick(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* draw current navpick */
	if (shared.shareddata.navpick_type != MBV_PICK_NONE &&
	    (data->nav_view_mode == MBV_VIEW_ON || data->navdrape_view_mode == MBV_VIEW_ON)) {
	// float zdisplay;
		/* set size of X mark for 2D case */
		// if (data->display_mode == MBV_DISPLAY_2D)
		// 	xlength = 0.05 / view->size2d;

		/* set color and linewidth */
		glColor3f(1.0, 0.0, 0.0);
		glLineWidth(3.0);

		/* plot first navpick point draped */
		if (data->display_mode == MBV_DISPLAY_3D && shared.shareddata.navpick.xsegments[0].nls > 0 &&
		    shared.shareddata.navpick.xsegments[1].nls > 0) {
			glBegin(GL_LINE_STRIP);
			for (int i = 0; i < shared.shareddata.navpick.xsegments[0].nls; i++) {
				glVertex3f((float)(shared.shareddata.navpick.xsegments[0].lspoints[i].xdisplay[instance]),
				           (float)(shared.shareddata.navpick.xsegments[0].lspoints[i].ydisplay[instance]),
				           (float)(shared.shareddata.navpick.xsegments[0].lspoints[i].zdisplay[instance]));
			}
			glEnd();
			glBegin(GL_LINE_STRIP);
			for (int i = 0; i < shared.shareddata.navpick.xsegments[1].nls; i++) {
				glVertex3f((float)(shared.shareddata.navpick.xsegments[1].lspoints[i].xdisplay[instance]),
				           (float)(shared.shareddata.navpick.xsegments[1].lspoints[i].ydisplay[instance]),
				           (float)(shared.shareddata.navpick.xsegments[1].lspoints[i].zdisplay[instance]));
			}
			glEnd();
		}
		else if (data->display_mode == MBV_DISPLAY_3D) {
			glBegin(GL_LINES);
			for (int i = 0; i < 4; i++) {
				glVertex3f((float)(shared.shareddata.navpick.xpoints[i].xdisplay[instance]),
				           (float)(shared.shareddata.navpick.xpoints[i].ydisplay[instance]),
				           (float)(shared.shareddata.navpick.xpoints[i].zdisplay[instance]));
			}
			glEnd();
		}
		else {
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
		if (data->display_mode == MBV_DISPLAY_3D && data->nav_view_mode == MBV_VIEW_ON) {
			int inav = shared.shareddata.nav_selected[0];
			int jpt = shared.shareddata.nav_point_selected[0];
			const double zdisplay = shared.shareddata.navs[inav].navpts[jpt].point.zdisplay[instance];
			glColor3f(1.0, 0.0, 0.0);
			glBegin(GL_LINES);
			for (int i = 0; i < 4; i++) {
				glVertex3f((float)(shared.shareddata.navpick.xpoints[i].xdisplay[instance]),
				           (float)(shared.shareddata.navpick.xpoints[i].ydisplay[instance]), zdisplay);
			}
			glEnd();
		}

		if (shared.shareddata.navpick_type == MBV_PICK_TWOPOINT) {
			/* plot second navpick point draped */
			if (data->display_mode == MBV_DISPLAY_3D && shared.shareddata.navpick.xsegments[2].nls > 0 &&
			    shared.shareddata.navpick.xsegments[3].nls > 0) {
				glBegin(GL_LINE_STRIP);
				for (int i = 0; i < shared.shareddata.navpick.xsegments[2].nls; i++) {
					glVertex3f((float)(shared.shareddata.navpick.xsegments[2].lspoints[i].xdisplay[instance]),
					           (float)(shared.shareddata.navpick.xsegments[2].lspoints[i].ydisplay[instance]),
					           (float)(shared.shareddata.navpick.xsegments[2].lspoints[i].zdisplay[instance]));
				}
				glEnd();
				glBegin(GL_LINE_STRIP);
				for (int i = 0; i < shared.shareddata.navpick.xsegments[3].nls; i++) {
					glVertex3f((float)(shared.shareddata.navpick.xsegments[3].lspoints[i].xdisplay[instance]),
					           (float)(shared.shareddata.navpick.xsegments[3].lspoints[i].ydisplay[instance]),
					           (float)(shared.shareddata.navpick.xsegments[3].lspoints[i].zdisplay[instance]));
				}
				glEnd();
			}
			else if (data->display_mode == MBV_DISPLAY_3D) {
				glBegin(GL_LINES);
				for (int i = 4; i < 8; i++) {
					glVertex3f((float)(shared.shareddata.navpick.xpoints[i].xdisplay[instance]),
					           (float)(shared.shareddata.navpick.xpoints[i].ydisplay[instance]),
					           (float)(shared.shareddata.navpick.xpoints[i].zdisplay[instance]));
				}
				glEnd();
			}
			else {
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
			if (data->display_mode == MBV_DISPLAY_3D && data->nav_view_mode == MBV_VIEW_ON) {
				const int inav = shared.shareddata.nav_selected[1];
				const int jpt = shared.shareddata.nav_point_selected[1];
				const double zdisplay = shared.shareddata.navs[inav].navpts[jpt].point.zdisplay[instance];
				glBegin(GL_LINES);
				for (int i = 4; i < 8; i++) {
					glVertex3f((float)(shared.shareddata.navpick.xpoints[i].xdisplay[instance]),
					           (float)(shared.shareddata.navpick.xpoints[i].ydisplay[instance]), zdisplay);
				}
				glEnd();
			}
		}
	}
#ifdef MBV_GETERRORS
	mbview_glerrorcheck(instance, 1, __func__);
#endif

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_drawnav(size_t instance, int rez) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       rez:              %d\n", rez);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* set decimation */
	int stride;
	if (rez == MBV_REZ_FULL)
		stride = 1;
	else if (rez == MBV_REZ_HIGH)
		stride = data->hirez_navdecimate;
	else
		stride = data->lorez_navdecimate;

	/* draw navigation */
	if (shared.shareddata.nav_mode != MBV_NAV_OFF && data->nav_view_mode == MBV_VIEW_ON && shared.shareddata.nnav > 0) {
		/* loop over the navs plotting xyz navigation */
		for (int inav = 0; inav < shared.shareddata.nnav; inav++) {
      if (shared.shareddata.navs[inav].active) {
  			int icolor = shared.shareddata.navs[inav].color;
  			glLineWidth((float)(shared.shareddata.navs[inav].size));
  			glBegin(GL_LINE_STRIP);
  			for (int jpoint = 0; jpoint < shared.shareddata.navs[inav].npoints; jpoint += stride) {
  				/* set size and color */
  				if (shared.shareddata.navs[inav].navpts[jpoint].selected ||
  				    (jpoint < shared.shareddata.navs[inav].npoints - 1 &&
  				     shared.shareddata.navs[inav].navpts[jpoint + 1].selected)) {
  					glColor3f(colortable_object_red[MBV_COLOR_RED], colortable_object_green[MBV_COLOR_RED],
  					          colortable_object_blue[MBV_COLOR_RED]);
  				}
  				else {
  					glColor3f(colortable_object_red[icolor], colortable_object_green[icolor], colortable_object_blue[icolor]);
  				}

  				/* draw points */
  				glVertex3f((float)(shared.shareddata.navs[inav].navpts[jpoint].point.xdisplay[instance]),
  				           (float)(shared.shareddata.navs[inav].navpts[jpoint].point.ydisplay[instance]),
  				           (float)(shared.shareddata.navs[inav].navpts[jpoint].point.zdisplay[instance]));
  			}
  			glEnd();
  		}
    }
	}

	int status = MB_SUCCESS;

	/* draw draped navigation */
	if (shared.shareddata.nav_mode != MBV_NAV_OFF && data->navdrape_view_mode == MBV_VIEW_ON && shared.shareddata.nnav > 0) {
		/* loop over the navs plotting draped navigation */
		for (int inav = 0; inav < shared.shareddata.nnav; inav++) {
      if (shared.shareddata.navs[inav].active) {
  			int icolor = shared.shareddata.navs[inav].color;
  			glLineWidth((float)(shared.shareddata.navs[inav].size));
  			glBegin(GL_LINE_STRIP);
  			for (int jpoint = 0; jpoint < shared.shareddata.navs[inav].npoints - stride; jpoint += stride) {
  				/* set size and color */
  				if (shared.shareddata.navs[inav].navpts[jpoint].selected ||
  				    shared.shareddata.navs[inav].navpts[jpoint + stride].selected) {
  					glColor3f(colortable_object_red[MBV_COLOR_RED], colortable_object_green[MBV_COLOR_RED],
  					          colortable_object_blue[MBV_COLOR_RED]);
  				}
  				else {
  					glColor3f(colortable_object_red[icolor], colortable_object_green[icolor], colortable_object_blue[icolor]);
  				}

  				/*fprintf(stderr,"inav:%d npoints:%d jpoint:%d nls:%d\n",
  				inav, shared.shareddata.navs[inav].npoints, jpoint, shared.shareddata.navs[inav].segments[jpoint].nls);*/
  				/* draw draped segment if stride == 1 */
  				if (stride == 1)
  					for (int k = 0; k < shared.shareddata.navs[inav].segments[jpoint].nls; k++) {
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
  				else if (shared.shareddata.navs[inav].segments[jpoint].nls > 0) {
  					/* draw points */
  					glVertex3f((float)(shared.shareddata.navs[inav].segments[jpoint].lspoints[0].xdisplay[instance]),
  					           (float)(shared.shareddata.navs[inav].segments[jpoint].lspoints[0].ydisplay[instance]),
  					           (float)(shared.shareddata.navs[inav].segments[jpoint].lspoints[0].zdisplay[instance]));
  				}
  			}
  			glEnd();
  		}
    }
	}

	/* draw swathbounds */
	if (shared.shareddata.nav_mode != MBV_NAV_OFF &&
	    (data->nav_view_mode == MBV_VIEW_ON || data->navdrape_view_mode == MBV_VIEW_ON) && shared.shareddata.nnav > 0) {
		/* initialize on the fly draping segment */
	  struct mbview_linesegmentw_struct segment;
		segment.nls = 0;
		segment.nls_alloc = 0;
		segment.lspoints = NULL;

		/* loop over the navs plotting swathbounds */
		for (int inav = 0; inav < shared.shareddata.nnav; inav++) {
      if (shared.shareddata.navs[inav].active) {
  			const double timegapuse = 60.0 * shared.shareddata.navs[inav].decimation * view->timegap;
  			if (shared.shareddata.navs[inav].swathbounds && shared.shareddata.navs[inav].nselected > 0) {
  				glColor3f(colortable_object_red[MBV_COLOR_YELLOW], colortable_object_green[MBV_COLOR_YELLOW],
  				          colortable_object_blue[MBV_COLOR_YELLOW]);
  				glLineWidth((float)(shared.shareddata.navs[inav].size));

  				/* draw port side of swath */
  				bool swathbounds_on = false;
  				for (int jpoint = 0; jpoint < shared.shareddata.navs[inav].npoints; jpoint++) {
  					/* draw from center at start of selected data */
  					if (!swathbounds_on && shared.shareddata.navs[inav].navpts[jpoint].selected) {
  						swathbounds_on = true;
  						glBegin(GL_LINE_STRIP);

  						if (data->display_mode == MBV_DISPLAY_3D && stride == 1) {
  							/* drape segment on the fly */
  							segment.endpoints[0] = shared.shareddata.navs[inav].navpts[jpoint].pointcntr;
  							segment.endpoints[1] = shared.shareddata.navs[inav].navpts[jpoint].pointport;
  							mbview_drapesegmentw(instance, &(segment));

  							/* draw the segment */
  							for (int i = 0; i < segment.nls; i++) {
  								glVertex3f((float)(segment.lspoints[i].xdisplay[instance]),
  								           (float)(segment.lspoints[i].ydisplay[instance]),
  								           (float)(segment.lspoints[i].zdisplay[instance]));
  							}
  						}
  						else {
  							glVertex3f((float)(shared.shareddata.navs[inav].navpts[jpoint].pointcntr.xdisplay[instance]),
  							           (float)(shared.shareddata.navs[inav].navpts[jpoint].pointcntr.ydisplay[instance]),
  							           (float)(shared.shareddata.navs[inav].navpts[jpoint].pointcntr.zdisplay[instance]));
  						}
  					}

  					/* draw during selected data */
  					if (shared.shareddata.navs[inav].navpts[jpoint].selected) {
  						glVertex3f((float)(shared.shareddata.navs[inav].navpts[jpoint].pointport.xdisplay[instance]),
  						           (float)(shared.shareddata.navs[inav].navpts[jpoint].pointport.ydisplay[instance]),
  						           (float)(shared.shareddata.navs[inav].navpts[jpoint].pointport.zdisplay[instance]));
  					}

  					/* draw to center at end of selected data */
  					if (swathbounds_on &&
  					    (!shared.shareddata.navs[inav].navpts[jpoint].selected ||
  					     jpoint >= shared.shareddata.navs[inav].npoints - 1 ||
  					     (jpoint > 0 && (shared.shareddata.navs[inav].navpts[jpoint].time_d -
  					                     shared.shareddata.navs[inav].navpts[jpoint - 1].time_d) > timegapuse))) {
  						if (data->display_mode == MBV_DISPLAY_3D && stride == 1) {
  							/* drape segment on the fly */
  							segment.endpoints[0] = shared.shareddata.navs[inav].navpts[jpoint].pointport;
  							segment.endpoints[1] = shared.shareddata.navs[inav].navpts[jpoint].pointcntr;
  							mbview_drapesegmentw(instance, &(segment));

  							/* draw the segment */
  							for (int i = 0; i < segment.nls; i++) {
  								glVertex3f((float)(segment.lspoints[i].xdisplay[instance]),
  								           (float)(segment.lspoints[i].ydisplay[instance]),
  								           (float)(segment.lspoints[i].zdisplay[instance]));
  							}
  						}
  						else {
  							glVertex3f((float)(shared.shareddata.navs[inav].navpts[jpoint].pointcntr.xdisplay[instance]),
  							           (float)(shared.shareddata.navs[inav].navpts[jpoint].pointcntr.ydisplay[instance]),
  							           (float)(shared.shareddata.navs[inav].navpts[jpoint].pointcntr.zdisplay[instance]));
  						}

  						swathbounds_on = false;
  						glEnd();
  					}
  				}

  				/* draw starboard side of swath */
  				swathbounds_on = false;
  				for (int jpoint = 0; jpoint < shared.shareddata.navs[inav].npoints; jpoint++) {
  					/* draw from center at start of selected data */
  					if (!swathbounds_on && shared.shareddata.navs[inav].navpts[jpoint].selected) {
  						swathbounds_on = true;
  						glBegin(GL_LINE_STRIP);

  						if (data->display_mode == MBV_DISPLAY_3D && stride == 1) {
  							/* drape segment on the fly */
  							segment.endpoints[0] = shared.shareddata.navs[inav].navpts[jpoint].pointcntr;
  							segment.endpoints[1] = shared.shareddata.navs[inav].navpts[jpoint].pointstbd;
  							mbview_drapesegmentw(instance, &(segment));

  							/* draw the segment */
  							for (int i = 0; i < segment.nls; i++) {
  								glVertex3f((float)(segment.lspoints[i].xdisplay[instance]),
  								           (float)(segment.lspoints[i].ydisplay[instance]),
  								           (float)(segment.lspoints[i].zdisplay[instance]));
  							}
  						}
  						else {
  							glVertex3f((float)(shared.shareddata.navs[inav].navpts[jpoint].pointcntr.xdisplay[instance]),
  							           (float)(shared.shareddata.navs[inav].navpts[jpoint].pointcntr.ydisplay[instance]),
  							           (float)(shared.shareddata.navs[inav].navpts[jpoint].pointcntr.zdisplay[instance]));
  						}
  					}

  					/* draw during selected data */
  					if (shared.shareddata.navs[inav].navpts[jpoint].selected) {
  						glVertex3f((float)(shared.shareddata.navs[inav].navpts[jpoint].pointstbd.xdisplay[instance]),
  						           (float)(shared.shareddata.navs[inav].navpts[jpoint].pointstbd.ydisplay[instance]),
  						           (float)(shared.shareddata.navs[inav].navpts[jpoint].pointstbd.zdisplay[instance]));
  					}

  					/* draw to center at end of selected data */
  					if (swathbounds_on &&
  					    (!shared.shareddata.navs[inav].navpts[jpoint].selected ||
  					     jpoint >= shared.shareddata.navs[inav].npoints - 1 ||
  					     (jpoint > 0 && (shared.shareddata.navs[inav].navpts[jpoint].time_d -
  					                     shared.shareddata.navs[inav].navpts[jpoint - 1].time_d) > timegapuse))) {
  						if (data->display_mode == MBV_DISPLAY_3D && stride == 1) {
  							/* drape segment on the fly */
  							segment.endpoints[0] = shared.shareddata.navs[inav].navpts[jpoint].pointstbd;
  							segment.endpoints[1] = shared.shareddata.navs[inav].navpts[jpoint].pointcntr;
  							mbview_drapesegmentw(instance, &(segment));

  							/* draw the segment */
  							for (int i = 0; i < segment.nls; i++) {
  								glVertex3f((float)(segment.lspoints[i].xdisplay[instance]),
  								           (float)(segment.lspoints[i].ydisplay[instance]),
  								           (float)(segment.lspoints[i].zdisplay[instance]));
  							}
  						}
  						else {
  							glVertex3f((float)(shared.shareddata.navs[inav].navpts[jpoint].pointcntr.xdisplay[instance]),
  							           (float)(shared.shareddata.navs[inav].navpts[jpoint].pointcntr.ydisplay[instance]),
  							           (float)(shared.shareddata.navs[inav].navpts[jpoint].pointcntr.zdisplay[instance]));
  						}

  						swathbounds_on = false;
  						glEnd();
  					}
  				}
  			}
  		}
    }

		/* deallocate on the fly draping segment */
		if (segment.nls_alloc > 0 && segment.lspoints != NULL) {
      int error = MB_ERROR_NO_ERROR;
			status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&segment.lspoints, &error);
			segment.nls_alloc = 0;
		}
	}
#ifdef MBV_GETERRORS
	mbview_glerrorcheck(instance, 1, __func__);
#endif

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_updatenavlist() {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
	}

	/* update nav list */
	if (shared.init_navlist == MBV_WINDOW_VISIBLE) {
		/* remove all existing items */
		XmListDeleteAllItems(shared.mb3d_navlist.mbview_list_navlist);

		if (shared.shareddata.nnav > 0) {
			/* get number of items */
			int nitems = 0;
			for (int inav = 0; inav < shared.shareddata.nnav; inav++) {
        if (shared.shareddata.navs[inav].active) {
				  nitems += 1;
        }
			}

      if (nitems > 0) {

  			/* allocate array of label XmStrings */
  			XmString *xstr = (XmString *)malloc(nitems * sizeof(XmString));

  			/* loop over the navs */
  			nitems = 0;
  			for (int inav = 0; inav < shared.shareddata.nnav; inav++) {
          if (shared.shareddata.navs[inav].active) {
    				/* add list item for each nav */
    				sprintf(value_string, "%3d | %3d | %s | %d | %s", inav, shared.shareddata.navs[inav].npoints,
    				        mbview_colorname[shared.shareddata.navs[inav].color], shared.shareddata.navs[inav].size,
    				        shared.shareddata.navs[inav].name);
    				xstr[nitems] = XmStringCreateLocalized(value_string);
    				nitems++;
          }
  			}

  			/* add list items */
  			XmListAddItems(shared.mb3d_navlist.mbview_list_navlist, xstr, nitems, 0);

  			/* deallocate memory no longer needed */
  			for (int iitem = 0; iitem < nitems; iitem++) {
  				XmStringFree(xstr[iitem]);
  			}
  			free(xstr);

  			/* check for completely selected navs */
  			int inavselect = MBV_SELECT_NONE;
  			for (int inav = 0; inav < shared.shareddata.nnav; inav++) {
          if (shared.shareddata.navs[inav].active) {
    				if (inavselect == MBV_SELECT_NONE && shared.shareddata.navs[inav].npoints > 1 &&
    				    shared.shareddata.navs[inav].nselected == shared.shareddata.navs[inav].npoints) {
    					inavselect = inav;
    				}
          }
  			}

  			/* select first item with fully selected nav */
  			if (inavselect != MBV_SELECT_NONE) {
  				const int iitem = inavselect + 1;
  				XmListSelectPos(shared.mb3d_navlist.mbview_list_navlist, iitem, 0);
  				XmListSetPos(shared.mb3d_navlist.mbview_list_navlist, MAX(iitem - 5, 1));
  			}

      }
		}
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/

int mbview_picknavbyname(int verbose, size_t instance, char *name, int *error) {
	(void)verbose;  // Unused parameter
	(void)error;  // Unused parameter

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       name:             %s\n", name);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	// fprintf(stderr,"mbview_picknavbyname:%s\n",name);

	/* find and select the navigation associated with name */
	if (shared.shareddata.nav_mode != MBV_NAV_OFF && shared.shareddata.nnav > 0) {
		bool found = false;
		for (int inav = 0; inav < shared.shareddata.nnav && !found; inav++) {
			if (strcmp(shared.shareddata.navs[inav].name, name) == 0
          && shared.shareddata.navs[inav].active) {
				found = true;
				shared.shareddata.navpick_type = MBV_PICK_TWOPOINT;
				shared.shareddata.nav_selected[0] = inav;
				shared.shareddata.nav_point_selected[0] = 0;
				shared.shareddata.nav_selected[1] = inav;
				shared.shareddata.nav_point_selected[1] = shared.shareddata.navs[inav].npoints - 1;
				shared.shareddata.navs[inav].nselected = shared.shareddata.navs[inav].npoints;
				shared.shareddata.nav_selected_mbnavadjust[0] = MBV_SELECT_NONE;
				shared.shareddata.nav_selected_mbnavadjust[1] = MBV_SELECT_NONE;
				for (int jpt = 0; jpt < shared.shareddata.navs[inav].npoints; jpt++) {
					shared.shareddata.navs[inav].navpts[jpt].selected = true;
				}
			}
		}
	}

	/* else beep */
	else {
		shared.shareddata.navpick_type = MBV_PICK_NONE;
		shared.shareddata.nav_selected[0] = MBV_SELECT_NONE;
		shared.shareddata.nav_selected[1] = MBV_SELECT_NONE;
		shared.shareddata.nav_point_selected[0] = MBV_SELECT_NONE;
		shared.shareddata.nav_point_selected[1] = MBV_SELECT_NONE;
		shared.shareddata.nav_selected_mbnavadjust[0] = MBV_SELECT_NONE;
		shared.shareddata.nav_selected_mbnavadjust[1] = MBV_SELECT_NONE;
		XBell(view->dpy, 100);
		for (int i = 0; i < shared.shareddata.nnav; i++) {
			for (int j = 0; j < shared.shareddata.navs[i].npoints; j++) {
				shared.shareddata.navs[i].navpts[j].selected = false;
			}
		}
	}

	/* set what kind of pick to annotate */
	if (shared.shareddata.nav_selected[0] != MBV_SELECT_NONE) {
		data->pickinfo_mode = MBV_PICK_NAV;
	}
	else {
		data->pickinfo_mode = data->pick_type;
	}

	/* update nav data list */
	mbview_updatenavlist();

	/* print nav debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  Nav data altered in function <%s>\n", __func__);
		fprintf(stderr, "dbg2  Nav values:\n");
		fprintf(stderr, "dbg2       nav_mode:              %d\n", shared.shareddata.nav_mode);
		fprintf(stderr, "dbg2       nav_view_mode:         %d\n", data->nav_view_mode);
		fprintf(stderr, "dbg2       navdrape_view_mode:    %d\n", data->navdrape_view_mode);
		fprintf(stderr, "dbg2       nnav:                  %d\n", shared.shareddata.nnav);
		fprintf(stderr, "dbg2       nnav_alloc:            %d\n", shared.shareddata.nnav_alloc);
		fprintf(stderr, "dbg2       nav_selected[0]:       %d\n", shared.shareddata.nav_selected[0]);
		fprintf(stderr, "dbg2       nav_point_selected[0]: %d\n", shared.shareddata.nav_point_selected[0]);
		fprintf(stderr, "dbg2       nav_selected[1]:       %d\n", shared.shareddata.nav_selected[1]);
		fprintf(stderr, "dbg2       nav_point_selected[1]: %d\n", shared.shareddata.nav_point_selected[1]);
		for (int i = 0; i < shared.shareddata.nnav; i++) {
			fprintf(stderr, "dbg2       nav %d active:        %d\n", i, shared.shareddata.navs[i].active);
			fprintf(stderr, "dbg2       nav %d color:         %d\n", i, shared.shareddata.navs[i].color);
			fprintf(stderr, "dbg2       nav %d size:          %d\n", i, shared.shareddata.navs[i].size);
			fprintf(stderr, "dbg2       nav %d name:          %s\n", i, shared.shareddata.navs[i].name);
			fprintf(stderr, "dbg2       nav %d swathbounds:   %d\n", i, shared.shareddata.navs[i].swathbounds);
			fprintf(stderr, "dbg2       nav %d line:          %d\n", i, shared.shareddata.navs[i].line);
			fprintf(stderr, "dbg2       nav %d shot:          %d\n", i, shared.shareddata.navs[i].shot);
			fprintf(stderr, "dbg2       nav %d cdp:           %d\n", i, shared.shareddata.navs[i].cdp);
			fprintf(stderr, "dbg2       nav %d decimation:    %d\n", i, shared.shareddata.navs[i].decimation);
			fprintf(stderr, "dbg2       nav %d npoints:       %d\n", i, shared.shareddata.navs[i].npoints);
			fprintf(stderr, "dbg2       nav %d npoints_alloc: %d\n", i, shared.shareddata.navs[i].npoints_alloc);
			fprintf(stderr, "dbg2       nav %d nselected:     %d\n", i, shared.shareddata.navs[i].nselected);
			for (int j = 0; j < shared.shareddata.navs[i].npoints; j++) {
				fprintf(stderr, "dbg2       nav %d %d draped:   %d\n", i, j, shared.shareddata.navs[i].navpts[j].draped);
				fprintf(stderr, "dbg2       nav %d %d selected: %d\n", i, j, shared.shareddata.navs[i].navpts[j].selected);
				fprintf(stderr, "dbg2       nav %d %d time_d:   %f\n", i, j, shared.shareddata.navs[i].navpts[j].time_d);
				fprintf(stderr, "dbg2       nav %d %d heading:  %f\n", i, j, shared.shareddata.navs[i].navpts[j].heading);
				fprintf(stderr, "dbg2       nav %d %d speed:    %f\n", i, j, shared.shareddata.navs[i].navpts[j].speed);
				fprintf(stderr, "dbg2       nav %d %d line:     %d\n", i, j, shared.shareddata.navs[i].navpts[j].line);
				fprintf(stderr, "dbg2       nav %d %d shot:     %d\n", i, j, shared.shareddata.navs[i].navpts[j].shot);
				fprintf(stderr, "dbg2       nav %d %d cdp:      %d\n", i, j, shared.shareddata.navs[i].navpts[j].cdp);

				fprintf(stderr, "dbg2       nav %d %d xgrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.xgrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d ygrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.ygrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d xlon:     %f\n", i, j, shared.shareddata.navs[i].navpts[j].point.xlon);
				fprintf(stderr, "dbg2       nav %d %d ylat:     %f\n", i, j, shared.shareddata.navs[i].navpts[j].point.ylat);
				fprintf(stderr, "dbg2       nav %d %d zdata:    %f\n", i, j, shared.shareddata.navs[i].navpts[j].point.zdata);
				fprintf(stderr, "dbg2       nav %d %d xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.xdisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.ydisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.zdisplay[instance]);

				fprintf(stderr, "dbg2       nav %d %d stbd xgrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.xgrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d stbd ygrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.ygrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d stbd xlon:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.xlon);
				fprintf(stderr, "dbg2       nav %d %d stbd ylat:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.ylat);
				fprintf(stderr, "dbg2       nav %d %d stbd zdata:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.zdata);
				fprintf(stderr, "dbg2       nav %d %d stbd xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.xdisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d stbd ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.ydisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d stbd zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.zdisplay[instance]);

				fprintf(stderr, "dbg2       nav %d %d cntr xgrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.xgrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d cntr ygrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.ygrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d cntr xlon:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.xlon);
				fprintf(stderr, "dbg2       nav %d %d cntr ylat:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.ylat);
				fprintf(stderr, "dbg2       nav %d %d cntr zdata:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.zdata);
				fprintf(stderr, "dbg2       nav %d %d cntr xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.xdisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d cntr ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.ydisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d cntr zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.zdisplay[instance]);

				fprintf(stderr, "dbg2       nav %d %d port xgrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.xgrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d port ygrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.ygrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d port xlon:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.xlon);
				fprintf(stderr, "dbg2       nav %d %d port ylat:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.ylat);
				fprintf(stderr, "dbg2       nav %d %d port zdata:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.zdata);
				fprintf(stderr, "dbg2       nav %d %d port xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.xdisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d port ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.ydisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d port zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.zdisplay[instance]);
			}
			for (int j = 0; j < shared.shareddata.navs[i].npoints - 1; j++) {
				fprintf(stderr, "dbg2       nav %d %d nls:          %d\n", i, j, shared.shareddata.navs[i].segments[j].nls);
				fprintf(stderr, "dbg2       nav %d %d nls_alloc:    %d\n", i, j, shared.shareddata.navs[i].segments[j].nls_alloc);
				fprintf(stderr, "dbg2       nav %d %d endpoints[0]: %p\n", i, j,
				        &shared.shareddata.navs[i].segments[j].endpoints[0]);
				fprintf(stderr, "dbg2       nav %d %d endpoints[1]: %p\n", i, j,
				        &shared.shareddata.navs[i].segments[j].endpoints[1]);
			}
		}
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/

int mbview_setnavactivebyname(int verbose, size_t instance, char *name, bool active, bool updatelist, int *error) {
	(void)verbose;  // Unused parameter
	(void)error;  // Unused parameter

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       name:             %s\n", name);
		fprintf(stderr, "dbg2       active:           %d\n", active);
		fprintf(stderr, "dbg2       updatelist:       %d\n", updatelist);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* find and select the navigation associated with name */
	if (shared.shareddata.nav_mode != MBV_NAV_OFF && shared.shareddata.nnav > 0) {
		bool found = false;
		for (int inav = 0; inav < shared.shareddata.nnav && !found; inav++) {
			if (strcmp(shared.shareddata.navs[inav].name, name) == 0) {
				found = true;
				shared.shareddata.navs[inav].active = active;
			}
		}
	}

	/* update nav data list */
	if (updatelist) {
    mbview_updatenavlist();
  }

	/* print nav debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  Nav data altered in function <%s>\n", __func__);
		fprintf(stderr, "dbg2  Nav values:\n");
		fprintf(stderr, "dbg2       nav_mode:              %d\n", shared.shareddata.nav_mode);
		fprintf(stderr, "dbg2       nav_view_mode:         %d\n", data->nav_view_mode);
		fprintf(stderr, "dbg2       navdrape_view_mode:    %d\n", data->navdrape_view_mode);
		fprintf(stderr, "dbg2       nnav:                  %d\n", shared.shareddata.nnav);
		fprintf(stderr, "dbg2       nnav_alloc:            %d\n", shared.shareddata.nnav_alloc);
		fprintf(stderr, "dbg2       nav_selected[0]:       %d\n", shared.shareddata.nav_selected[0]);
		fprintf(stderr, "dbg2       nav_point_selected[0]: %d\n", shared.shareddata.nav_point_selected[0]);
		fprintf(stderr, "dbg2       nav_selected[1]:       %d\n", shared.shareddata.nav_selected[1]);
		fprintf(stderr, "dbg2       nav_point_selected[1]: %d\n", shared.shareddata.nav_point_selected[1]);
		for (int i = 0; i < shared.shareddata.nnav; i++) {
			fprintf(stderr, "dbg2       nav %d active:        %d\n", i, shared.shareddata.navs[i].active);
			fprintf(stderr, "dbg2       nav %d color:         %d\n", i, shared.shareddata.navs[i].color);
			fprintf(stderr, "dbg2       nav %d size:          %d\n", i, shared.shareddata.navs[i].size);
			fprintf(stderr, "dbg2       nav %d name:          %s\n", i, shared.shareddata.navs[i].name);
			fprintf(stderr, "dbg2       nav %d swathbounds:   %d\n", i, shared.shareddata.navs[i].swathbounds);
			fprintf(stderr, "dbg2       nav %d line:          %d\n", i, shared.shareddata.navs[i].line);
			fprintf(stderr, "dbg2       nav %d shot:          %d\n", i, shared.shareddata.navs[i].shot);
			fprintf(stderr, "dbg2       nav %d cdp:           %d\n", i, shared.shareddata.navs[i].cdp);
			fprintf(stderr, "dbg2       nav %d decimation:    %d\n", i, shared.shareddata.navs[i].decimation);
			fprintf(stderr, "dbg2       nav %d npoints:       %d\n", i, shared.shareddata.navs[i].npoints);
			fprintf(stderr, "dbg2       nav %d npoints_alloc: %d\n", i, shared.shareddata.navs[i].npoints_alloc);
			fprintf(stderr, "dbg2       nav %d nselected:     %d\n", i, shared.shareddata.navs[i].nselected);
			for (int j = 0; j < shared.shareddata.navs[i].npoints; j++) {
				fprintf(stderr, "dbg2       nav %d %d draped:   %d\n", i, j, shared.shareddata.navs[i].navpts[j].draped);
				fprintf(stderr, "dbg2       nav %d %d selected: %d\n", i, j, shared.shareddata.navs[i].navpts[j].selected);
				fprintf(stderr, "dbg2       nav %d %d time_d:   %f\n", i, j, shared.shareddata.navs[i].navpts[j].time_d);
				fprintf(stderr, "dbg2       nav %d %d heading:  %f\n", i, j, shared.shareddata.navs[i].navpts[j].heading);
				fprintf(stderr, "dbg2       nav %d %d speed:    %f\n", i, j, shared.shareddata.navs[i].navpts[j].speed);
				fprintf(stderr, "dbg2       nav %d %d line:     %d\n", i, j, shared.shareddata.navs[i].navpts[j].line);
				fprintf(stderr, "dbg2       nav %d %d shot:     %d\n", i, j, shared.shareddata.navs[i].navpts[j].shot);
				fprintf(stderr, "dbg2       nav %d %d cdp:      %d\n", i, j, shared.shareddata.navs[i].navpts[j].cdp);

				fprintf(stderr, "dbg2       nav %d %d xgrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.xgrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d ygrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.ygrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d xlon:     %f\n", i, j, shared.shareddata.navs[i].navpts[j].point.xlon);
				fprintf(stderr, "dbg2       nav %d %d ylat:     %f\n", i, j, shared.shareddata.navs[i].navpts[j].point.ylat);
				fprintf(stderr, "dbg2       nav %d %d zdata:    %f\n", i, j, shared.shareddata.navs[i].navpts[j].point.zdata);
				fprintf(stderr, "dbg2       nav %d %d xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.xdisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.ydisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.zdisplay[instance]);

				fprintf(stderr, "dbg2       nav %d %d stbd xgrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.xgrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d stbd ygrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.ygrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d stbd xlon:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.xlon);
				fprintf(stderr, "dbg2       nav %d %d stbd ylat:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.ylat);
				fprintf(stderr, "dbg2       nav %d %d stbd zdata:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.zdata);
				fprintf(stderr, "dbg2       nav %d %d stbd xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.xdisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d stbd ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.ydisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d stbd zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.zdisplay[instance]);

				fprintf(stderr, "dbg2       nav %d %d cntr xgrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.xgrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d cntr ygrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.ygrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d cntr xlon:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.xlon);
				fprintf(stderr, "dbg2       nav %d %d cntr ylat:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.ylat);
				fprintf(stderr, "dbg2       nav %d %d cntr zdata:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.zdata);
				fprintf(stderr, "dbg2       nav %d %d cntr xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.xdisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d cntr ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.ydisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d cntr zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.zdisplay[instance]);

				fprintf(stderr, "dbg2       nav %d %d port xgrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.xgrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d port ygrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.ygrid[instance]);
				fprintf(stderr, "dbg2       nav %d %d port xlon:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.xlon);
				fprintf(stderr, "dbg2       nav %d %d port ylat:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.ylat);
				fprintf(stderr, "dbg2       nav %d %d port zdata:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.zdata);
				fprintf(stderr, "dbg2       nav %d %d port xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.xdisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d port ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.ydisplay[instance]);
				fprintf(stderr, "dbg2       nav %d %d port zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.zdisplay[instance]);
			}
			for (int j = 0; j < shared.shareddata.navs[i].npoints - 1; j++) {
				fprintf(stderr, "dbg2       nav %d %d nls:          %d\n", i, j, shared.shareddata.navs[i].segments[j].nls);
				fprintf(stderr, "dbg2       nav %d %d nls_alloc:    %d\n", i, j, shared.shareddata.navs[i].segments[j].nls_alloc);
				fprintf(stderr, "dbg2       nav %d %d endpoints[0]: %p\n", i, j,
				        &shared.shareddata.navs[i].segments[j].endpoints[0]);
				fprintf(stderr, "dbg2       nav %d %d endpoints[1]: %p\n", i, j,
				        &shared.shareddata.navs[i].segments[j].endpoints[1]);
			}
		}
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
