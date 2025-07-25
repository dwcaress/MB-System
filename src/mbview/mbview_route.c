/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_route.c	9/25/2003
 *
 *    Copyright (c) 2003-2025 by
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
 *
 * Author:	D. W. Caress
 * Date:	September 25, 2003
 *
 * Note:	This code was broken out of mbview_callbacks.c, which was
 *		begun on October 7, 2002
 *
 *
 */
/*------------------------------------------------------------------------------*/

/* Standard includes for builtins. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/* Need to include windows.h BEFORE the the Xm stuff otherwise VC14+ barf with conflicts */
#if defined(_MSC_VER) && (_MSC_VER >= 1800)
#	ifndef WIN32
#		define WIN32
#	endif
#	include <WinSock2.h>
#include <windows.h>
#endif

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
#include <Xm/List.h>
#include "MB3DView.h"
#include "MB3DSiteList.h"
#include "MB3DRouteList.h"
#include "MB3DNavList.h"

/* OpenGL include files */

#include <GL/gl.h>
#include <GL/glu.h>
#ifndef WIN32
#include <GL/glx.h>
#endif
#include "mb_glwdrawa.h"

/* MBIO include files */
#include "mb_status.h"
#include "mb_define.h"

/* mbview include */
#include "mbview.h"
#include "mbviewprivate.h"

/*------------------------------------------------------------------------------*/

/* local variables */
static char value_string[2*MB_PATH_MAXLINE];


/*------------------------------------------------------------------------------*/
int mbview_getroutecount(int verbose, size_t instance, int *nroute, int *error) {
	/* local variables */
	int status = MB_SUCCESS;

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	/* get view */
	// struct mbview_world_struct *view = &(mbviews[instance]);
	// struct mbview_struct *data = &(view->data);

	/* get number of routes */
	*nroute = shared.shareddata.nroute;

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nroute:                    %d\n", *nroute);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	/* return */
	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_getroutepointcount(int verbose, size_t instance, int route, int *npoint, int *nintpoint, int *error) {
	/* local variables */
	int status = MB_SUCCESS;
	int i;

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       route:                     %d\n", route);
	}

	/* get view */
	// struct mbview_world_struct *view = &(mbviews[instance]);
	// struct mbview_struct *data = &(view->data);

	/* get number of points in specified route */
	*npoint = 0;
	*nintpoint = 0;
	if (route >= 0 && route < shared.shareddata.nroute) {
		*npoint = shared.shareddata.routes[route].npoints;
		for (i = 0; i < *npoint - 1; i++) {
			if (shared.shareddata.routes[route].segments[i].nls > 2)
				*nintpoint += shared.shareddata.routes[route].segments[i].nls - 2;
		}
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       npoint:                    %d\n", *npoint);
		fprintf(stderr, "dbg2       nintpoint:                 %d\n", *nintpoint);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	/* return */
	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_getrouteselected(int verbose, size_t instance, int route, bool *selected, int *error) {
	/* local variables */
	int status = MB_SUCCESS;

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       route:                     %d\n", route);
	}

	/* get view */
	// struct mbview_world_struct *view = &(mbviews[instance]);
	// struct mbview_struct *data = &(view->data);

	/* check if the specified route is currently selected in totality */
	if (route == shared.shareddata.route_selected && shared.shareddata.route_point_selected == MBV_SELECT_ALL)
		*selected = true;
	else
		*selected = false;

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       selected:                  %d\n", *selected);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	/* return */
	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_getrouteinfo(int verbose, size_t instance, int working_route, int *nroutewaypoint, int *nroutpoint, char *routename,
                        int *routecolor, int *routesize, double *routedistancelateral, double *routedistancetopo, int *error) {
	/* local variables */
	int status = MB_SUCCESS;
	struct mbview_route_struct *route;

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       working_route:             %d\n", working_route);
	}

	/* get view */
	// struct mbview_world_struct *view = &(mbviews[instance]);
	// struct mbview_struct *data = &(view->data);

	/* check that the route is valid */
	if (working_route < 0 && working_route >= shared.shareddata.nroute) {
		*nroutewaypoint = 0;
		*nroutpoint = 0;
		routename[0] = '\0';
		*routecolor = 0;
		*routesize = 0;
		*routedistancelateral = 0.0;
		*routedistancetopo = 0.0;
		status = MB_FAILURE;
		*error = MB_ERROR_DATA_NOT_INSERTED;
	}

	/* otherwise go get the route data */
	else {
		/* get basic info */
		route = &(shared.shareddata.routes[working_route]);
		*nroutewaypoint = route->npoints;
		*nroutpoint = route->nroutepoint;
		strcpy(routename, route->name);
		*routecolor = route->color;
		*routesize = route->size;
		*routedistancelateral = route->distancelateral;
		*routedistancetopo = route->distancetopo;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nroutewaypoint:            %d\n", *nroutewaypoint);
		fprintf(stderr, "dbg2       nroutpoint:                %d\n", *nroutpoint);
		fprintf(stderr, "dbg2       routename:                 %d\n", *routename);
		fprintf(stderr, "dbg2       routecolor:                %d\n", *routecolor);
		fprintf(stderr, "dbg2       routesize:                 %d\n", *routesize);
		fprintf(stderr, "dbg2       routedistancelateral:      %f\n", *routedistancelateral);
		fprintf(stderr, "dbg2       routedistancetopo:         %f\n", *routedistancetopo);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	/* return */
	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_allocroutearrays(int verbose, int npointtotal, double **routelon, double **routelat, int **waypoint,
                            double **routetopo, double **routebearing, double **distlateral, double **distovertopo,
                            double **slope, int *error) {
	/* local variables */
	int status = MB_SUCCESS;

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       npointtotal:               %d\n", npointtotal);
		fprintf(stderr, "dbg2       routelon:                  %p\n", *routelon);
		fprintf(stderr, "dbg2       routelat:                  %p\n", *routelat);
		if (waypoint != NULL)
			fprintf(stderr, "dbg2       waypoint:                  %p\n", *waypoint);
		if (routetopo != NULL)
			fprintf(stderr, "dbg2       routetopo:                 %p\n", *routetopo);
		if (routebearing != NULL)
			fprintf(stderr, "dbg2       routebearing:              %p\n", *routebearing);
		if (distlateral != NULL)
			fprintf(stderr, "dbg2       distlateral:               %p\n", *distlateral);
		if (distovertopo != NULL)
			fprintf(stderr, "dbg2       distovertopo:              %p\n", *distovertopo);
		if (slope != NULL)
			fprintf(stderr, "dbg2       slope:                     %p\n", *slope);
	}

	/* allocate the arrays using mb_reallocd */
	status = mb_reallocd(verbose, __FILE__, __LINE__, npointtotal * sizeof(double), (void **)routelon, error);
	if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npointtotal * sizeof(double), (void **)routelat, error);
	if (status == MB_SUCCESS && waypoint != NULL)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npointtotal * sizeof(int), (void **)waypoint, error);
	if (status == MB_SUCCESS && routetopo != NULL)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npointtotal * sizeof(double), (void **)routetopo, error);
	if (status == MB_SUCCESS && routebearing != NULL)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npointtotal * sizeof(double), (void **)routebearing, error);
	if (status == MB_SUCCESS && distlateral != NULL)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npointtotal * sizeof(double), (void **)distlateral, error);
	if (status == MB_SUCCESS && distovertopo != NULL)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npointtotal * sizeof(double), (void **)distovertopo, error);
	if (status == MB_SUCCESS && slope != NULL)
		status = mb_reallocd(verbose, __FILE__, __LINE__, npointtotal * sizeof(double), (void **)slope, error);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       routelon:                  %p\n", *routelon);
		fprintf(stderr, "dbg2       routelat:                  %p\n", *routelat);
		if (waypoint != NULL)
			fprintf(stderr, "dbg2       waypoint:                  %p\n", *waypoint);
		if (routetopo != NULL)
			fprintf(stderr, "dbg2       routetopo:                 %p\n", *routetopo);
		if (routebearing != NULL)
			fprintf(stderr, "dbg2       routebearing:              %p\n", *routebearing);
		if (distlateral != NULL)
			fprintf(stderr, "dbg2       distlateral:               %p\n", *distlateral);
		if (distovertopo != NULL)
			fprintf(stderr, "dbg2       distovertopo:              %p\n", *distovertopo);
		if (slope != NULL)
			fprintf(stderr, "dbg2       slope:                     %p\n", *slope);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	/* return */
	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_freeroutearrays(int verbose, double **routelon, double **routelat, int **waypoint, double **routetopo,
                           double **routebearing, double **distlateral, double **distovertopo, double **slope, int *error) {
	/* local variables */
	int status = MB_SUCCESS;

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       routelon:                  %p\n", *routelon);
		fprintf(stderr, "dbg2       routelat:                  %p\n", *routelat);
		if (waypoint != NULL)
			fprintf(stderr, "dbg2       waypoint:                  %p\n", *waypoint);
		if (routetopo != NULL)
			fprintf(stderr, "dbg2       routetopo:                 %p\n", *routetopo);
		if (routebearing != NULL)
			fprintf(stderr, "dbg2       routebearing:              %p\n", *routebearing);
		if (distlateral != NULL)
			fprintf(stderr, "dbg2       distlateral:               %p\n", *distlateral);
		if (distovertopo != NULL)
			fprintf(stderr, "dbg2       distovertopo:              %p\n", *distovertopo);
		if (slope != NULL)
			fprintf(stderr, "dbg2       slope:                     %p\n", *slope);
	}

	/* free the arrays using mb_freed */
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)routelon, error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)routelat, error);
	if (waypoint != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)waypoint, error);
	if (routetopo != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)routetopo, error);
	if (routebearing != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)routebearing, error);
	if (distlateral != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)distlateral, error);
	if (distovertopo != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)distovertopo, error);
	if (slope != NULL)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)slope, error);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       routelon:                  %p\n", *routelon);
		fprintf(stderr, "dbg2       routelat:                  %p\n", *routelat);
		if (waypoint != NULL)
			fprintf(stderr, "dbg2       waypoint:                  %p\n", *waypoint);
		if (routetopo != NULL)
			fprintf(stderr, "dbg2       routetopo:                 %p\n", *routetopo);
		if (routebearing != NULL)
			fprintf(stderr, "dbg2       routebearing:              %p\n", *routebearing);
		if (distlateral != NULL)
			fprintf(stderr, "dbg2       distlateral:               %p\n", *distlateral);
		if (distovertopo != NULL)
			fprintf(stderr, "dbg2       distovertopo:              %p\n", *distovertopo);
		if (slope != NULL)
			fprintf(stderr, "dbg2       slope:                     %p\n", *slope);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	/* return */
	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_addroute(int verbose, size_t instance, int npoint, double *routelon, double *routelat, int *waypoint, int routecolor,
                    int routesize, int routeeditmode, mb_path routename, int *iroute, int *error) {
	/* local variables */
	int status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	double xgrid, ygrid, zdata;
	double xdisplay, ydisplay, zdisplay;
	int i;

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       npoint:                    %d\n", npoint);
		fprintf(stderr, "dbg2       routelon:                  %p\n", routelon);
		fprintf(stderr, "dbg2       routelat:                  %p\n", routelat);
		fprintf(stderr, "dbg2       waypoint:                  %p\n", waypoint);
		for (i = 0; i < npoint; i++) {
			fprintf(stderr, "dbg2       point:%d lon:%f lat:%f waypoint:%d\n", i, routelon[i], routelat[i], waypoint[i]);
		}
		fprintf(stderr, "dbg2       routecolor:                %d\n", routecolor);
		fprintf(stderr, "dbg2       routesize:                 %d\n", routesize);
		fprintf(stderr, "dbg2       routeeditmode:             %d\n", routeeditmode);
		fprintf(stderr, "dbg2       routename:                 %s\n", routename);
	}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* make sure no route is selected */
	shared.shareddata.route_selected = MBV_SELECT_NONE;
	shared.shareddata.route_point_selected = MBV_SELECT_NONE;

	/* set route id so that new route is created */
	*iroute = shared.shareddata.nroute;

	/* loop over the points in the new route */
	for (i = 0; i < npoint; i++) {
		/* check waypoint flag correct */
		if (waypoint[i] <= MBV_ROUTE_WAYPOINT_NONE || waypoint[i] > MBV_ROUTE_WAYPOINT_ENDLINE5)
			waypoint[i] = MBV_ROUTE_WAYPOINT_SIMPLE;

		/* get route positions in grid coordinates */
		status = mbview_projectll2xyzgrid(instance, routelon[i], routelat[i], &xgrid, &ygrid, &zdata);

		/* get route positions in display coordinates */
		status = mbview_projectll2display(instance, routelon[i], routelat[i], zdata, &xdisplay, &ydisplay, &zdisplay);
		
		if (isnan(xdisplay)) {
			mbv_verbose = 5;
			status = mbview_projectll2display(instance, routelon[i], routelat[i], zdata, &xdisplay, &ydisplay, &zdisplay);
			mbv_verbose = 0;
		}

		/* check for reasonable coordinates */
		if (fabs(xdisplay) < 1000.0 && fabs(ydisplay) < 1000.0 && fabs(zdisplay) < 1000.0) {

			/* add the route point */
			mbview_route_add(mbv_verbose, instance, *iroute, i, waypoint[i], xgrid, ygrid, routelon[i], routelat[i], zdata,
			                 xdisplay, ydisplay, zdisplay);
		}

		/* report failure due to unreasonable coordinates */
		else {
			fprintf(stderr,
			        "Failed to add route point at position lon:%f lat:%f due to display coordinate projection (%f %f %f) far "
			        "outside view...\n",
			        routelon[i], routelat[i], xdisplay, ydisplay, zdisplay);
			XBell(view->dpy, 100);
		}
	}

	/* set color size and name for new route */
	shared.shareddata.routes[*iroute].color = routecolor;
	shared.shareddata.routes[*iroute].size = routesize;
	shared.shareddata.routes[*iroute].editmode = routeeditmode;
	strcpy(shared.shareddata.routes[*iroute].name, routename);

	/* set distance values */
	mbview_route_setdistance(instance, *iroute);

	/* make routes viewable */
	if (data->route_view_mode != MBV_VIEW_ON) {
		data->route_view_mode = MBV_VIEW_ON;
		set_mbview_route_view_mode(instance, MBV_VIEW_ON);
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       iroute:                    %d\n", *iroute);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	/* return */
	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_deleteroute(int verbose, size_t instance, int iroute, int *error) {
	/* local variables */
	int status = MB_SUCCESS;
	int jpoint;

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       iroute:                    %d\n", iroute);
	}

	/* get view */
	// struct mbview_world_struct *view = &(mbviews[instance]);
	// struct mbview_struct *data = &(view->data);

	/* delete the points in the route backwards */
	for (jpoint = shared.shareddata.routes[iroute].npoints - 1; jpoint >= 0; jpoint--) {
		/* delete the route point */
		mbview_route_delete(instance, iroute, jpoint);
	}

	/* set pick annotation */
	mbview_pick_text(instance);

	/* update route list */
	mbview_updateroutelist();

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	/* return */
	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_deleteallroutes(int verbose, size_t instance, int *error) {
	/* local variables */
	int status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	struct mbview_route_struct *route;
	struct mbview_linesegmentw_struct *segment;
	int i, j;

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	for (i = 0; i < shared.shareddata.nroute_alloc; i++) {
		route = &shared.shareddata.routes[i];
		if (route->npoints_alloc > 0) {
			for (j = 0; j < route->npoints_alloc; j++) {
				segment = &route->segments[j];
				if (segment->nls_alloc > 0 && segment->lspoints != NULL) {
					status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(segment->lspoints), error);
					segment->nls_alloc = 0;
					segment->nls = 0;
				}
			}
			status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&route->waypoint, error);
			status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&route->distlateral, error);
			status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&route->disttopo, error);
			status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&route->points, error);
			status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&route->segments, error);
		}
		route->npoints = 0;
		route->npoints_alloc = 0;
		route->nroutepoint = 0;
		route->waypoint = NULL;
		route->distlateral = NULL;
		route->disttopo = NULL;
		route->points = NULL;
		route->segments = NULL;
	}
	if (shared.shareddata.nroute_alloc > 0 && shared.shareddata.routes != NULL) {
		status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&shared.shareddata.routes, error);
	}
	shared.shareddata.nroute = 0;
	shared.shareddata.nroute_alloc = 0;
	shared.shareddata.route_selected = MBV_SELECT_NONE;
	shared.shareddata.route_point_selected = MBV_SELECT_NONE;
	shared.shareddata.routes = NULL;

	/* set pick annotation */
	mbview_pick_text(instance);

	/* update route list */
	mbview_updateroutelist();

	/* print route debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  Route data altered in function <%s>\n", __func__);
		fprintf(stderr, "dbg2  Route values:\n");
		fprintf(stderr, "dbg2       route_view_mode:      %d\n", data->route_view_mode);
		fprintf(stderr, "dbg2       route_mode:           %d\n", shared.shareddata.route_mode);
		fprintf(stderr, "dbg2       nroute:               %d\n", shared.shareddata.nroute);
		fprintf(stderr, "dbg2       nroute_alloc:         %d\n", shared.shareddata.nroute_alloc);
		fprintf(stderr, "dbg2       route_selected:       %d\n", shared.shareddata.route_selected);
		fprintf(stderr, "dbg2       route_point_selected: %d\n", shared.shareddata.route_point_selected);
		for (i = 0; i < shared.shareddata.nroute; i++) {
			fprintf(stderr, "dbg2       route %d active:        %d\n", i, shared.shareddata.routes[i].active);
			fprintf(stderr, "dbg2       route %d color:         %d\n", i, shared.shareddata.routes[i].color);
			fprintf(stderr, "dbg2       route %d size:          %d\n", i, shared.shareddata.routes[i].size);
			fprintf(stderr, "dbg2       route %d name:          %s\n", i, shared.shareddata.routes[i].name);
			fprintf(stderr, "dbg2       route %d npoints:       %d\n", i, shared.shareddata.routes[i].npoints);
			fprintf(stderr, "dbg2       route %d npoints_alloc: %d\n", i, shared.shareddata.routes[i].npoints_alloc);
			for (j = 0; j < shared.shareddata.routes[i].npoints; j++) {
				fprintf(stderr, "dbg2       route %d %d xgrid:    %f\n", i, j,
				        shared.shareddata.routes[i].points[j].xgrid[instance]);
				fprintf(stderr, "dbg2       route %d %d ygrid:    %f\n", i, j,
				        shared.shareddata.routes[i].points[j].ygrid[instance]);
				fprintf(stderr, "dbg2       route %d %d xlon:     %f\n", i, j, shared.shareddata.routes[i].points[j].xlon);
				fprintf(stderr, "dbg2       route %d %d ylat:     %f\n", i, j, shared.shareddata.routes[i].points[j].ylat);
				fprintf(stderr, "dbg2       route %d %d zdata:    %f\n", i, j, shared.shareddata.routes[i].points[j].zdata);
				fprintf(stderr, "dbg2       route %d %d xdisplay: %f\n", i, j,
				        shared.shareddata.routes[i].points[j].xdisplay[instance]);
				fprintf(stderr, "dbg2       route %d %d ydisplay: %f\n", i, j,
				        shared.shareddata.routes[i].points[j].ydisplay[instance]);
				fprintf(stderr, "dbg2       route %d %d zdisplay: %f\n", i, j,
				        shared.shareddata.routes[i].points[j].zdisplay[instance]);
			}
			for (j = 0; j < shared.shareddata.routes[i].npoints - 1; j++) {
				fprintf(stderr, "dbg2       route %d %d nls:          %d\n", i, j, shared.shareddata.routes[i].segments[j].nls);
				fprintf(stderr, "dbg2       route %d %d nls_alloc:    %d\n", i, j,
				        shared.shareddata.routes[i].segments[j].nls_alloc);
				fprintf(stderr, "dbg2       route %d %d endpoints[0]: %p\n", i, j,
				        &shared.shareddata.routes[i].segments[j].endpoints[0]);
				fprintf(stderr, "dbg2       route %d %d endpoints[1]: %p\n", i, j,
				        &shared.shareddata.routes[i].segments[j].endpoints[1]);
			}
		}
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	/* return */
	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_getroute(int verbose, size_t instance, int route, int *npointtotal, double *routelon, double *routelat, int *waypoint,
                    double *routetopo, double *routebearing, double *distlateral, double *distovertopo, double *slope,
                    int *routecolor, int *routesize, int *routeeditmode, mb_path routename, int *error) {
	/* local variables */
	int status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	double dx, dy, range, bearing;
	double xx1, yy1, xx2, yy2;
	int i, j;

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       route:                     %d\n", route);
		fprintf(stderr, "dbg2       npointtotal:               %p\n", npointtotal);
		fprintf(stderr, "dbg2       routelon:                  %p\n", routelon);
		fprintf(stderr, "dbg2       routelat:                  %p\n", routelat);
		fprintf(stderr, "dbg2       waypoint:                  %p\n", waypoint);
		fprintf(stderr, "dbg2       routetopo:                 %p\n", routetopo);
		fprintf(stderr, "dbg2       routebearing:              %p\n", routebearing);
		fprintf(stderr, "dbg2       distlateral:               %p\n", distlateral);
		fprintf(stderr, "dbg2       distovertopo:              %p\n", distovertopo);
		fprintf(stderr, "dbg2       slope:                     %p\n", slope);
		fprintf(stderr, "dbg2       routecolor:                %p\n", routecolor);
		fprintf(stderr, "dbg2       routesize:                 %p\n", routesize);
		fprintf(stderr, "dbg2       routeeditmode:             %p\n", routeeditmode);
		fprintf(stderr, "dbg2       routename:                 %p\n", routename);
	}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* zero the points returned */
	*npointtotal = 0;

	/* check that the array pointers are not NULL */
	if (routelon == NULL || routelat == NULL || waypoint == NULL || routetopo == NULL || distlateral == NULL ||
	    distovertopo == NULL || slope == NULL) {
		status = MB_FAILURE;
		*error = MB_ERROR_DATA_NOT_INSERTED;
	}

	/* otherwise go get the route data */
	else {
		/* loop over the route segments */
		for (i = 0; i < shared.shareddata.routes[route].npoints - 1; i++) {
			/* get bearing of segment */
			if (data->display_projection_mode != MBV_PROJECTION_SPHEROID) {
				xx1 = shared.shareddata.routes[route].points[i].xdisplay[instance];
				yy1 = shared.shareddata.routes[route].points[i].ydisplay[instance];
				xx2 = shared.shareddata.routes[route].points[i + 1].xdisplay[instance];
				yy2 = shared.shareddata.routes[route].points[i + 1].ydisplay[instance];
				dx = (double)(shared.shareddata.routes[route].points[i + 1].xdisplay[instance] -
				              shared.shareddata.routes[route].points[i].xdisplay[instance]);
				dy = (double)(shared.shareddata.routes[route].points[i + 1].ydisplay[instance] -
				              shared.shareddata.routes[route].points[i].ydisplay[instance]);
				dx = xx2 - xx1;
				dy = yy2 - yy1;
				range = sqrt(dx * dx + dy * dy) / view->scale;
				bearing = RTD * atan2(dx, dy);
			}
			else {
				mbview_greatcircle_distbearing(instance, shared.shareddata.routes[route].points[i].xlon,
				                               shared.shareddata.routes[route].points[i].ylat,
				                               shared.shareddata.routes[route].points[i + 1].xlon,
				                               shared.shareddata.routes[route].points[i + 1].ylat, &bearing, &range);
			}
			if (bearing < 0.0)
				bearing += 360.0;

			/* add first point */
			routelon[*npointtotal] = shared.shareddata.routes[route].points[i].xlon;
			if (routelon[*npointtotal] < -180.0)
				routelon[*npointtotal] += 360.0;
			else if (routelon[*npointtotal] > 180.0)
				routelon[*npointtotal] -= 360.0;
			routelat[*npointtotal] = shared.shareddata.routes[route].points[i].ylat;
			waypoint[*npointtotal] = shared.shareddata.routes[route].waypoint[i];
			routetopo[*npointtotal] = shared.shareddata.routes[route].points[i].zdata;
			routebearing[*npointtotal] = bearing;
			if (*npointtotal == 0) {
				distlateral[*npointtotal] = 0.0;
				distovertopo[*npointtotal] = 0.0;
				slope[*npointtotal] = 0.0;
			}
			else {
				mbview_projectdistance(instance, routelon[*npointtotal - 1], routelat[*npointtotal - 1],
				                       routetopo[*npointtotal - 1], routelon[*npointtotal], routelat[*npointtotal],
				                       routetopo[*npointtotal], &distlateral[*npointtotal], &distovertopo[*npointtotal],
				                       &slope[*npointtotal]);
				distlateral[*npointtotal] += distlateral[*npointtotal - 1];
				distovertopo[*npointtotal] += distovertopo[*npointtotal - 1];
			}
			(*npointtotal)++;

			/* loop over interior of segment */
			for (j = 1; j < shared.shareddata.routes[route].segments[i].nls - 1; j++) {
				routelon[*npointtotal] = shared.shareddata.routes[route].segments[i].lspoints[j].xlon;
				if (routelon[*npointtotal] < -180.0)
					routelon[*npointtotal] += 360.0;
				else if (routelon[*npointtotal] > 180.0)
					routelon[*npointtotal] -= 360.0;
				routelat[*npointtotal] = shared.shareddata.routes[route].segments[i].lspoints[j].ylat;
				waypoint[*npointtotal] = MBV_ROUTE_WAYPOINT_NONE;
				routetopo[*npointtotal] = shared.shareddata.routes[route].segments[i].lspoints[j].zdata;
				routebearing[*npointtotal] = bearing;
				mbview_projectdistance(instance, routelon[*npointtotal - 1], routelat[*npointtotal - 1],
				                       routetopo[*npointtotal - 1], routelon[*npointtotal], routelat[*npointtotal],
				                       routetopo[*npointtotal], &distlateral[*npointtotal], &distovertopo[*npointtotal],
				                       &slope[*npointtotal]);
				distlateral[*npointtotal] += distlateral[*npointtotal - 1];
				distovertopo[*npointtotal] += distovertopo[*npointtotal - 1];
				(*npointtotal)++;
			}
		}

		/* add last point */
		j = shared.shareddata.routes[route].npoints - 1;
		routelon[*npointtotal] = shared.shareddata.routes[route].points[j].xlon;
		if (routelon[*npointtotal] < -180.0)
			routelon[*npointtotal] += 360.0;
		else if (routelon[*npointtotal] > 180.0)
			routelon[*npointtotal] -= 360.0;
		routelat[*npointtotal] = shared.shareddata.routes[route].points[j].ylat;
		waypoint[*npointtotal] = shared.shareddata.routes[route].waypoint[j];
		;
		routetopo[*npointtotal] = shared.shareddata.routes[route].points[j].zdata;
		routebearing[*npointtotal] = bearing;
		mbview_projectdistance(instance, routelon[*npointtotal - 1], routelat[*npointtotal - 1], routetopo[*npointtotal - 1],
		                       routelon[*npointtotal], routelat[*npointtotal], routetopo[*npointtotal],
		                       &distlateral[*npointtotal], &distovertopo[*npointtotal], &slope[*npointtotal]);
		distlateral[*npointtotal] += distlateral[*npointtotal - 1];
		distovertopo[*npointtotal] += distovertopo[*npointtotal - 1];
		(*npointtotal)++;

		/* get color size and name */
		*routecolor = shared.shareddata.routes[route].color;
		*routesize = shared.shareddata.routes[route].size;
		*routeeditmode = shared.shareddata.routes[route].editmode;
		strcpy(routename, shared.shareddata.routes[route].name);

		/* recalculate slope */
		for (j = 0; j < *npointtotal; j++) {
			if (j == 0 && *npointtotal == 1) {
				slope[j] = 0.0;
			}
			else if (j == 0) {
				if (distlateral[j + 1] > 0.0)
					slope[j] = (routetopo[j + 1] - routetopo[j]) / distlateral[j + 1];
				else
					slope[j] = 0.0;
			}
			else if (j == *npointtotal - 1) {
				if ((distlateral[j] - distlateral[j - 1]) > 0.0)
					slope[j] = (routetopo[j] - routetopo[j - 1]) / (distlateral[j] - distlateral[j - 1]);
				else
					slope[j] = 0.0;
			}
			else {
				if ((distlateral[j + 1] - distlateral[j - 1]) > 0.0)
					slope[j] = (routetopo[j + 1] - routetopo[j - 1]) / (distlateral[j + 1] - distlateral[j - 1]);
				else
					slope[j] = 0.0;
			}
		}
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       npointtotal:               %d\n", *npointtotal);
		fprintf(stderr, "dbg2       routecolor:                %d\n", *routecolor);
		fprintf(stderr, "dbg2       routesize:                 %d\n", *routesize);
		fprintf(stderr, "dbg2       routeeditmode:             %d\n", *routeeditmode);
		fprintf(stderr, "dbg2       routename:                 %s\n", routename);
		for (i = 0; i < *npointtotal; i++) {
			fprintf(
			    stderr,
			    "dbg2       route:%d lon:%f lat:%f waypoint:%d topo:%f bearing:%f dist:%f distbot:%f color:%d size:%d name:%s\n",
			    i, routelon[i], routelat[i], waypoint[i], routetopo[i], routebearing[i], distlateral[i], distovertopo[i],
			    *routecolor, *routesize, routename);
		}
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	/* return */
	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_enableviewroutes(int verbose, size_t instance, int *error)

{
	/* local variables */
	int status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	/* set values */
	shared.shareddata.route_mode = MBV_ROUTE_VIEW;

	/* set widget sensitivity on all active instances */
	for (instance = 0; instance < MBV_MAX_WINDOWS; instance++) {
		/* get view */
		view = &(mbviews[instance]);
		data = &(view->data);

		/* if instance active reset action sensitivity */
		if (data->active)
			mbview_update_sensitivity(verbose, instance, error);
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	/* return */
	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_enableeditroutes(int verbose, size_t instance, int *error)

{
	/* local variables */
	int status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* set values */
	shared.shareddata.route_mode = MBV_ROUTE_EDIT;

	/* set widget sensitivity */
	if (data->active)
		mbview_update_sensitivity(verbose, instance, error);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	/* return */
	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_enableviewties(int verbose, size_t instance, int *error)

{
	/* local variables */
	int status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* set values */
	shared.shareddata.route_mode = MBV_ROUTE_NAVADJUST;

	/* set widget sensitivity */
	if (data->active)
		mbview_update_sensitivity(verbose, instance, error);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	/* return */
	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_pick_routebyname(int verbose, size_t instance, char *name, int *error) {

	/* local variables */
	int status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int i;

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       name:             %s\n", name);
	}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	// fprintf(stderr,"start mbview_pick_routebyname: name:%s route_mode:%d route_selected:%d %d\n",
	// name, shared.shareddata.route_mode, shared.shareddata.route_selected,
	// shared.shareddata.route_point_selected);

	/* only select route points if enabled and not in move mode */
	if (shared.shareddata.route_mode != MBV_ROUTE_OFF && shared.shareddata.nroute > 0) {
		// bool found = false;
		shared.shareddata.route_selected = MBV_SELECT_NONE;
		shared.shareddata.route_point_selected = MBV_SELECT_NONE;
		for (i = 0; i < shared.shareddata.nroute; i++) {
			if (strcmp(name, shared.shareddata.routes[i].name) == 0
          && shared.shareddata.routes[i].active) {
				// found = true;
				shared.shareddata.route_selected = i;
				shared.shareddata.route_point_selected = MBV_SELECT_ALL;
			}
		}
	}

	/* else beep */
	else {
		shared.shareddata.route_selected = MBV_SELECT_NONE;
	}
	// fprintf(stderr,"done  mbview_pick_routebyname: name:%s route_mode:%d route_selected:%d %d\n",
	// name, shared.shareddata.route_mode, shared.shareddata.route_selected,
	// shared.shareddata.route_point_selected);

	/* print route debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Route data altered in function <%s>\n", __func__);
		fprintf(stderr, "dbg2  Route values:\n");
		fprintf(stderr, "dbg2       route_view_mode:      %d\n", data->route_view_mode);
		fprintf(stderr, "dbg2       route_mode:           %d\n", shared.shareddata.route_mode);
		fprintf(stderr, "dbg2       nroute:               %d\n", shared.shareddata.nroute);
		fprintf(stderr, "dbg2       nroute_alloc:         %d\n", shared.shareddata.nroute_alloc);
		fprintf(stderr, "dbg2       route_selected:       %d\n", shared.shareddata.route_selected);
		fprintf(stderr, "dbg2       route_point_selected: %d\n", shared.shareddata.route_point_selected);
	}

	/* print output debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	/* return */
	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_pick_route_select(int verbose, size_t instance, int which, int xpixel, int ypixel) {

	/* local variables */
	int status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	bool found;
	double xgrid, ygrid;
	double xlon, ylat, zdata;
	double xdisplay, ydisplay, zdisplay;
	double xx, yy, rr, rrmin;
	int iroute;
	int jpoint;
	int i, j;

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       which:            %d\n", which);
		fprintf(stderr, "dbg2       xpixel:           %d\n", xpixel);
		fprintf(stderr, "dbg2       ypixel:           %d\n", ypixel);
	}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	// fprintf(stderr,"mbview_pick_route_select: which:%d route_mode:%d route_selected:%d\n",
	// which, shared.shareddata.route_mode, shared.shareddata.route_selected);
	/* only select route points if enabled and not in move mode */
	if (shared.shareddata.route_mode != MBV_ROUTE_OFF && shared.shareddata.nroute > 0 &&
	    (which == MBV_PICK_DOWN || shared.shareddata.route_selected == MBV_SELECT_NONE)) {
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay, &zdisplay);

		/* look for nearest route point */
		if (found) {
			rrmin = 1000000000.0;
			shared.shareddata.route_selected = MBV_SELECT_NONE;
			shared.shareddata.route_point_selected = MBV_SELECT_NONE;
			for (i = 0; i < shared.shareddata.nroute; i++) {
        if (shared.shareddata.routes[i].active) {
  				for (j = 0; j < shared.shareddata.routes[i].npoints; j++) {
  					xx = xgrid - shared.shareddata.routes[i].points[j].xgrid[instance];
  					yy = ygrid - shared.shareddata.routes[i].points[j].ygrid[instance];
  					rr = sqrt(xx * xx + yy * yy);
  					if (rr < rrmin) {
  						rrmin = rr;
  						shared.shareddata.route_selected = i;
  						shared.shareddata.route_point_selected = j;
  					}
  				}
        }
      }
		}
		else {
			shared.shareddata.route_selected = MBV_SELECT_NONE;
			XBell(view->dpy, 100);
		}
		if (shared.shareddata.route_mode == MBV_ROUTE_NAVADJUST && shared.shareddata.route_selected != MBV_SELECT_NONE)
			shared.shareddata.route_point_selected = MBV_SELECT_ALL;
	}

	/* only move selected route points if enabled */
	else if (shared.shareddata.route_mode != MBV_ROUTE_OFF && shared.shareddata.nroute > 0 &&
	         (which == MBV_PICK_MOVE && shared.shareddata.route_selected != MBV_SELECT_NONE)) {
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay, &zdisplay);

		/* reset selected route position */
		iroute = shared.shareddata.route_selected;
		if (found && shared.shareddata.routes[iroute].editmode) {
			jpoint = shared.shareddata.route_point_selected;
			shared.shareddata.routes[iroute].points[jpoint].xgrid[instance] = xgrid;
			shared.shareddata.routes[iroute].points[jpoint].ygrid[instance] = ygrid;
			shared.shareddata.routes[iroute].points[jpoint].xlon = xlon;
			shared.shareddata.routes[iroute].points[jpoint].ylat = ylat;
			shared.shareddata.routes[iroute].points[jpoint].zdata = zdata;
			shared.shareddata.routes[iroute].points[jpoint].xdisplay[instance] = xdisplay;
			shared.shareddata.routes[iroute].points[jpoint].ydisplay[instance] = ydisplay;
			shared.shareddata.routes[iroute].points[jpoint].zdisplay[instance] = zdisplay;
			mbview_updatepointw(instance, &(shared.shareddata.routes[iroute].points[jpoint]));

			/* drape the affected segments */
			if (jpoint > 0) {
				/* drape the segment */
				shared.shareddata.routes[iroute].segments[jpoint - 1].endpoints[1] =
				    shared.shareddata.routes[iroute].points[jpoint];
				mbview_drapesegmentw(instance, &(shared.shareddata.routes[iroute].segments[jpoint - 1]));

				/* update the segment for all active instances */
				mbview_updatesegmentw(instance, &(shared.shareddata.routes[iroute].segments[jpoint - 1]));
			}
			if (jpoint < shared.shareddata.routes[iroute].npoints - 1) {
				/* drape the segment */
				shared.shareddata.routes[iroute].segments[jpoint].endpoints[0] = shared.shareddata.routes[iroute].points[jpoint];
				mbview_drapesegmentw(instance, &(shared.shareddata.routes[iroute].segments[jpoint]));

				/* update the segment for all active instances */
				mbview_updatesegmentw(instance, &(shared.shareddata.routes[iroute].segments[jpoint]));
			}

			/* set distance values */
			mbview_route_setdistance(instance, iroute);
		}

		/* else beep */
		else {
			shared.shareddata.route_selected = MBV_SELECT_NONE;
			XBell(view->dpy, 100);
		}
	}

	/* else beep */
	else {
		shared.shareddata.route_selected = MBV_SELECT_NONE;
		XBell(view->dpy, 100);
	}

	/* call pick notify if defined */
	if (shared.shareddata.route_selected != MBV_SELECT_NONE && data->mbview_pickroute_notify != NULL) {
		(data->mbview_pickroute_notify)(instance);
	}

	/* set what kind of pick to annotate */
	if (shared.shareddata.route_selected != MBV_SELECT_NONE) {
		data->pickinfo_mode = MBV_PICK_ROUTE;
	}
	else {
		data->pickinfo_mode = data->pick_type;
	}

	/* set pick annotation */
	mbview_pick_text(instance);

	/* update route list */
	mbview_updateroutelist();
	// fprintf(stderr,"Done with mbview_pick_route_select:selected:%d %d\n-----\n",
	// shared.shareddata.route_selected,shared.shareddata.route_point_selected);

	/* print route debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Route data altered in function <%s>\n", __func__);
		fprintf(stderr, "dbg2  Route values:\n");
		fprintf(stderr, "dbg2       route_view_mode:      %d\n", data->route_view_mode);
		fprintf(stderr, "dbg2       route_mode:           %d\n", shared.shareddata.route_mode);
		fprintf(stderr, "dbg2       nroute:               %d\n", shared.shareddata.nroute);
		fprintf(stderr, "dbg2       nroute_alloc:         %d\n", shared.shareddata.nroute_alloc);
		fprintf(stderr, "dbg2       route_selected:       %d\n", shared.shareddata.route_selected);
		fprintf(stderr, "dbg2       route_point_selected: %d\n", shared.shareddata.route_point_selected);
		for (i = 0; i < shared.shareddata.nroute; i++) {
			fprintf(stderr, "dbg2       route %d active:        %d\n", i, shared.shareddata.routes[i].active);
			fprintf(stderr, "dbg2       route %d color:         %d\n", i, shared.shareddata.routes[i].color);
			fprintf(stderr, "dbg2       route %d size:          %d\n", i, shared.shareddata.routes[i].size);
			fprintf(stderr, "dbg2       route %d name:          %s\n", i, shared.shareddata.routes[i].name);
			fprintf(stderr, "dbg2       route %d npoints:       %d\n", i, shared.shareddata.routes[i].npoints);
			fprintf(stderr, "dbg2       route %d npoints_alloc: %d\n", i, shared.shareddata.routes[i].npoints_alloc);
			for (j = 0; j < shared.shareddata.routes[i].npoints; j++) {
				fprintf(stderr, "dbg2       route %d %d waypoint: %d\n", i, j, shared.shareddata.routes[i].waypoint[j]);
				fprintf(stderr, "dbg2       route %d %d xgrid:    %f\n", i, j,
				        shared.shareddata.routes[i].points[j].xgrid[instance]);
				fprintf(stderr, "dbg2       route %d %d ygrid:    %f\n", i, j,
				        shared.shareddata.routes[i].points[j].ygrid[instance]);
				fprintf(stderr, "dbg2       route %d %d xlon:     %f\n", i, j, shared.shareddata.routes[i].points[j].xlon);
				fprintf(stderr, "dbg2       route %d %d ylat:     %f\n", i, j, shared.shareddata.routes[i].points[j].ylat);
				fprintf(stderr, "dbg2       route %d %d zdata:    %f\n", i, j, shared.shareddata.routes[i].points[j].zdata);
				fprintf(stderr, "dbg2       route %d %d xdisplay: %f\n", i, j,
				        shared.shareddata.routes[i].points[j].xdisplay[instance]);
				fprintf(stderr, "dbg2       route %d %d ydisplay: %f\n", i, j,
				        shared.shareddata.routes[i].points[j].ydisplay[instance]);
				fprintf(stderr, "dbg2       route %d %d zdisplay: %f\n", i, j,
				        shared.shareddata.routes[i].points[j].zdisplay[instance]);
			}
			for (j = 0; j < shared.shareddata.routes[i].npoints - 1; j++) {
				fprintf(stderr, "dbg2       route %d %d nls:          %d\n", i, j, shared.shareddata.routes[i].segments[j].nls);
				fprintf(stderr, "dbg2       route %d %d nls_alloc:    %d\n", i, j,
				        shared.shareddata.routes[i].segments[j].nls_alloc);
				fprintf(stderr, "dbg2       route %d %d endpoints[0]: %p\n", i, j,
				        &shared.shareddata.routes[i].segments[j].endpoints[0]);
				fprintf(stderr, "dbg2       route %d %d endpoints[1]: %p\n", i, j,
				        &shared.shareddata.routes[i].segments[j].endpoints[1]);
			}
		}
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	/* return */
	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_extract_route_profile(size_t instance) {

	/* local variables */
	int status = MB_SUCCESS;
	int error = MB_ERROR_NO_ERROR;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int iroute, jstart;
	int nprpoints;
	double dx, dy;
	int i, j;

	/* print starting debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* if a route is selected, extract the profile */
	if (shared.shareddata.route_selected != MBV_SELECT_NONE &&
	    shared.shareddata.routes[shared.shareddata.route_selected].npoints > 1) {
		data->profile.source = MBV_PROFILE_ROUTE;
		strcpy(data->profile.source_name, "Route");
		data->profile.length = 0.0;
		iroute = shared.shareddata.route_selected;

		/* make sure enough memory is allocated for the profile */
		nprpoints = 0;
		for (i = 0; i < shared.shareddata.routes[iroute].npoints - 1; i++) {
			nprpoints += shared.shareddata.routes[iroute].segments[i].nls;
		}
		if (data->profile.npoints_alloc < nprpoints) {
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
			for (i = 0; i < shared.shareddata.routes[iroute].npoints - 1; i++) {
				if (i == 0)
					jstart = 0;
				else
					jstart = 1;
				for (j = jstart; j < shared.shareddata.routes[iroute].segments[i].nls; j++) {
					if (j == 0 || j == shared.shareddata.routes[iroute].segments[i].nls - 1)
						data->profile.points[data->profile.npoints].boundary = true;
					else
						data->profile.points[data->profile.npoints].boundary = false;
					data->profile.points[data->profile.npoints].xgrid =
					    shared.shareddata.routes[iroute].segments[i].lspoints[j].xgrid[instance];
					data->profile.points[data->profile.npoints].ygrid =
					    shared.shareddata.routes[iroute].segments[i].lspoints[j].ygrid[instance];
					data->profile.points[data->profile.npoints].xlon =
					    shared.shareddata.routes[iroute].segments[i].lspoints[j].xlon;
					data->profile.points[data->profile.npoints].ylat =
					    shared.shareddata.routes[iroute].segments[i].lspoints[j].ylat;
					data->profile.points[data->profile.npoints].zdata =
					    shared.shareddata.routes[iroute].segments[i].lspoints[j].zdata;
					data->profile.points[data->profile.npoints].xdisplay =
					    shared.shareddata.routes[iroute].segments[i].lspoints[j].xdisplay[instance];
					data->profile.points[data->profile.npoints].ydisplay =
					    shared.shareddata.routes[iroute].segments[i].lspoints[j].ydisplay[instance];
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
							dx = data->profile.points[data->profile.npoints].xdisplay -
							     data->profile.points[data->profile.npoints - 1].xdisplay;
							dy = data->profile.points[data->profile.npoints].ydisplay -
							     data->profile.points[data->profile.npoints - 1].ydisplay;
							data->profile.points[data->profile.npoints].distance =
							    sqrt(dx * dx + dy * dy) / view->scale + data->profile.points[data->profile.npoints - 1].distance;
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
						dy = (data->profile.points[data->profile.npoints].zdata -
						      data->profile.points[data->profile.npoints - 1].zdata);
						dx = (data->profile.points[data->profile.npoints].distance -
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
						dy = (data->profile.points[data->profile.npoints].zdata -
						      data->profile.points[data->profile.npoints - 2].zdata);
						dx = (data->profile.points[data->profile.npoints].distance -
						      data->profile.points[data->profile.npoints - 2].distance);
						if (dx > 0.0)
							data->profile.points[data->profile.npoints - 1].slope = fabs(dy / dx);
						else
							data->profile.points[data->profile.npoints - 1].slope = 0.0;
					}
					data->profile.points[data->profile.npoints].navzdata = 0.0;
					data->profile.points[data->profile.npoints].navtime_d = 0.0;
					data->profile.npoints++;
				}
			}
			data->profile.length = data->profile.points[data->profile.npoints - 1].distance;
		}
	}

	/* print output debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	/* return */
	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_pick_route_add(int verbose, size_t instance, int which, int xpixel, int ypixel) {

	/* local variables */
	int status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	bool found;
	double xgrid, ygrid;
	double xlon, ylat, zdata;
	double xdisplay, ydisplay, zdisplay;
	int i, j, inew, jnew;

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       which:            %d\n", which);
		fprintf(stderr, "dbg2       xpixel:           %d\n", xpixel);
		fprintf(stderr, "dbg2       ypixel:           %d\n", ypixel);
	}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* only add route points if enabled and not in move mode */
	if (shared.shareddata.route_mode == MBV_ROUTE_EDIT &&
	    (which == MBV_PICK_DOWN || (which == MBV_PICK_MOVE && shared.shareddata.route_selected == MBV_SELECT_NONE))) {
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay, &zdisplay);

		/* add route if necessary */
		if (found && shared.shareddata.route_selected == MBV_SELECT_NONE) {
			/* add route point after currently selected route point if any */
			inew = shared.shareddata.nroute;
			jnew = 0;

			/* add the route point */
			mbview_route_add(mbv_verbose, instance, inew, jnew, MBV_ROUTE_WAYPOINT_SIMPLE, xgrid, ygrid, xlon, ylat, zdata,
			                 xdisplay, ydisplay, zdisplay);

			/* select the new route */
			shared.shareddata.route_selected = inew;
			shared.shareddata.route_point_selected = jnew;
		}

		/* else just add point to currently selected route and point */
		else if (found && shared.shareddata.route_selected != MBV_SELECT_NONE &&
		         shared.shareddata.routes[shared.shareddata.route_selected].editmode) {
			/* add route point after currently selected route point if any */
			inew = shared.shareddata.route_selected;
			jnew = shared.shareddata.route_point_selected + 1;

			/* add the route point */
			mbview_route_add(mbv_verbose, instance, inew, jnew, MBV_ROUTE_WAYPOINT_SIMPLE, xgrid, ygrid, xlon, ylat, zdata,
			                 xdisplay, ydisplay, zdisplay);

			/* select the new route */
			shared.shareddata.route_selected = inew;
			shared.shareddata.route_point_selected = jnew;
		}

		/* if selected route not editable then complain */
		else if (found) {
			XBell(view->dpy, 100);
		}

		/* if not found then deselect the new route */
		else {
			shared.shareddata.route_selected = MBV_SELECT_NONE;
			shared.shareddata.route_point_selected = MBV_SELECT_NONE;
			XBell(view->dpy, 100);
		}
	}

	/* only move selected routes if enabled */
	else if (shared.shareddata.route_mode == MBV_ROUTE_EDIT && shared.shareddata.nroute > 0 &&
	         (which == MBV_PICK_MOVE && shared.shareddata.route_selected != MBV_SELECT_NONE)) {
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay, &zdisplay);

		/* reset selected route position */
		if (found) {
			/* move the point */
			shared.shareddata.routes[shared.shareddata.route_selected]
			    .points[shared.shareddata.route_point_selected]
			    .xgrid[instance] = xgrid;
			shared.shareddata.routes[shared.shareddata.route_selected]
			    .points[shared.shareddata.route_point_selected]
			    .ygrid[instance] = ygrid;
			shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].xlon = xlon;
			shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].ylat = ylat;
			shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].zdata =
			    zdata;
			shared.shareddata.routes[shared.shareddata.route_selected]
			    .points[shared.shareddata.route_point_selected]
			    .xdisplay[instance] = xdisplay;
			shared.shareddata.routes[shared.shareddata.route_selected]
			    .points[shared.shareddata.route_point_selected]
			    .ydisplay[instance] = ydisplay;
			shared.shareddata.routes[shared.shareddata.route_selected]
			    .points[shared.shareddata.route_point_selected]
			    .zdisplay[instance] = zdisplay;
			mbview_updatepointw(
			    instance,
			    &(shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected]));

			/* drape the affected segments */
			if (shared.shareddata.route_point_selected > 0) {
				/* drape the segment */
				shared.shareddata.routes[shared.shareddata.route_selected]
				    .segments[shared.shareddata.route_point_selected - 1]
				    .endpoints[1] =
				    shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected];
				mbview_drapesegmentw(instance, &(shared.shareddata.routes[shared.shareddata.route_selected]
				                                     .segments[shared.shareddata.route_point_selected - 1]));

				/* update the segment for all active instances */
				mbview_updatesegmentw(instance, &(shared.shareddata.routes[shared.shareddata.route_selected]
				                                      .segments[shared.shareddata.route_point_selected - 1]));
			}
			if (shared.shareddata.route_point_selected < shared.shareddata.routes[shared.shareddata.route_selected].npoints - 1) {
				/* drape the segment */
				shared.shareddata.routes[shared.shareddata.route_selected]
				    .segments[shared.shareddata.route_point_selected]
				    .endpoints[0] =
				    shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected];
				mbview_drapesegmentw(instance, &(shared.shareddata.routes[shared.shareddata.route_selected]
				                                     .segments[shared.shareddata.route_point_selected]));

				/* update the segment for all active instances */
				mbview_updatesegmentw(instance, &(shared.shareddata.routes[shared.shareddata.route_selected]
				                                      .segments[shared.shareddata.route_point_selected]));
			}

			/* set distance values */
			mbview_route_setdistance(instance, shared.shareddata.route_selected);
		}
	}

	/* else beep */
	else {
		shared.shareddata.route_selected = MBV_SELECT_NONE;
		shared.shareddata.route_point_selected = MBV_SELECT_NONE;
		XBell(view->dpy, 100);
	}

	/* set what kind of pick to annotate */
	if (shared.shareddata.route_selected != MBV_SELECT_NONE) {
		data->pickinfo_mode = MBV_PICK_ROUTE;
	}
	else {
		data->pickinfo_mode = data->pick_type;
	}

	/* set pick annotation */
	mbview_pick_text(instance);

	/* update route list */
	mbview_updateroutelist();

	/* print route debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Route data altered in function <%s>\n", __func__);
		fprintf(stderr, "dbg2  Route values:\n");
		fprintf(stderr, "dbg2       route_view_mode:      %d\n", data->route_view_mode);
		fprintf(stderr, "dbg2       route_mode:           %d\n", shared.shareddata.route_mode);
		fprintf(stderr, "dbg2       nroute:               %d\n", shared.shareddata.nroute);
		fprintf(stderr, "dbg2       nroute_alloc:         %d\n", shared.shareddata.nroute_alloc);
		fprintf(stderr, "dbg2       route_selected:       %d\n", shared.shareddata.route_selected);
		fprintf(stderr, "dbg2       route_point_selected: %d\n", shared.shareddata.route_point_selected);
		for (i = 0; i < shared.shareddata.nroute; i++) {
			fprintf(stderr, "dbg2       route %d active:        %d\n", i, shared.shareddata.routes[i].active);
			fprintf(stderr, "dbg2       route %d color:         %d\n", i, shared.shareddata.routes[i].color);
			fprintf(stderr, "dbg2       route %d size:          %d\n", i, shared.shareddata.routes[i].size);
			fprintf(stderr, "dbg2       route %d name:          %s\n", i, shared.shareddata.routes[i].name);
			fprintf(stderr, "dbg2       route %d npoints:       %d\n", i, shared.shareddata.routes[i].npoints);
			fprintf(stderr, "dbg2       route %d npoints_alloc: %d\n", i, shared.shareddata.routes[i].npoints_alloc);
			for (j = 0; j < shared.shareddata.routes[i].npoints; j++) {
				fprintf(stderr, "dbg2       route %d %d xgrid:    %f\n", i, j,
				        shared.shareddata.routes[i].points[j].xgrid[instance]);
				fprintf(stderr, "dbg2       route %d %d ygrid:    %f\n", i, j,
				        shared.shareddata.routes[i].points[j].ygrid[instance]);
				fprintf(stderr, "dbg2       route %d %d xlon:     %f\n", i, j, shared.shareddata.routes[i].points[j].xlon);
				fprintf(stderr, "dbg2       route %d %d ylat:     %f\n", i, j, shared.shareddata.routes[i].points[j].ylat);
				fprintf(stderr, "dbg2       route %d %d zdata:    %f\n", i, j, shared.shareddata.routes[i].points[j].zdata);
				fprintf(stderr, "dbg2       route %d %d xdisplay: %f\n", i, j,
				        shared.shareddata.routes[i].points[j].xdisplay[instance]);
				fprintf(stderr, "dbg2       route %d %d ydisplay: %f\n", i, j,
				        shared.shareddata.routes[i].points[j].ydisplay[instance]);
				fprintf(stderr, "dbg2       route %d %d zdisplay: %f\n", i, j,
				        shared.shareddata.routes[i].points[j].zdisplay[instance]);
			}
			for (j = 0; j < shared.shareddata.routes[i].npoints - 1; j++) {
				fprintf(stderr, "dbg2       route %d %d nls:          %d\n", i, j, shared.shareddata.routes[i].segments[j].nls);
				fprintf(stderr, "dbg2       route %d %d nls_alloc:    %d\n", i, j,
				        shared.shareddata.routes[i].segments[j].nls_alloc);
				fprintf(stderr, "dbg2       route %d %d endpoints[0]: %p\n", i, j,
				        &shared.shareddata.routes[i].segments[j].endpoints[0]);
				fprintf(stderr, "dbg2       route %d %d endpoints[1]: %p\n", i, j,
				        &shared.shareddata.routes[i].segments[j].endpoints[1]);
			}
		}
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	/* return */
	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_pick_route_delete(int verbose, size_t instance, int xpixel, int ypixel) {

	/* local variables */
	int status = MB_SUCCESS;
	bool found;
	double xgrid, ygrid;
	double xlon, ylat, zdata;
	double xdisplay, ydisplay, zdisplay;
	double xx, yy, rr, rrmin;
	int i, j, idelete, jdelete;

	/* print starting debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       xpixel:           %d\n", xpixel);
		fprintf(stderr, "dbg2       ypixel:           %d\n", ypixel);
	}

	/* get view */
	struct mbview_world_struct *view = &(mbviews[instance]);
	// struct mbview_struct *data = &(view->data);

	/* only delete a selected route if enabled */
	if (shared.shareddata.route_mode == MBV_ROUTE_EDIT && shared.shareddata.route_selected != MBV_SELECT_NONE) {
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay, &zdisplay);

		/* find closest route to pick point */
		if (found) {
			rrmin = 1000000000.0;
			idelete = MBV_SELECT_NONE;
			jdelete = MBV_SELECT_NONE;
			for (i = 0; i < shared.shareddata.nroute; i++) {
        if (shared.shareddata.routes[i].active) {
  				for (j = 0; j < shared.shareddata.routes[i].npoints; j++) {
  					xx = xgrid - shared.shareddata.routes[i].points[j].xgrid[instance];
  					yy = ygrid - shared.shareddata.routes[i].points[j].ygrid[instance];
  					rr = sqrt(xx * xx + yy * yy);
  					if (rr < rrmin) {
  						rrmin = rr;
  						idelete = i;
  						jdelete = j;
  					}
  				}
        }
			}
		}

		/* delete route point if its the same as previously selected */
		if (found && shared.shareddata.route_selected == idelete && shared.shareddata.route_point_selected == jdelete) {
			mbview_route_delete(instance, idelete, jdelete);
		}

		/* else beep */
		else {
			XBell(view->dpy, 100);
		}
	}

	/* else beep */
	else {
		XBell(view->dpy, 100);
	}

	/* set pick annotation */
	mbview_pick_text(instance);

	/* update route list */
	mbview_updateroutelist();

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	/* return */
	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_route_add(int verbose, size_t instance, int inew, int jnew, int waypoint, double xgrid, double ygrid, double xlon,
                     double ylat, double zdata, double xdisplay, double ydisplay, double zdisplay) {

	/* local variables */
	int status = MB_SUCCESS;
	int error = MB_ERROR_NO_ERROR;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	struct mbview_linesegmentw_struct *seg;
	int npoints, npoints_alloc, npoints_diff;
	int i, j, k;

	/* print starting debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       inew:             %d\n", inew);
		fprintf(stderr, "dbg2       jnew:             %d\n", jnew);
		fprintf(stderr, "dbg2       waypoint:         %d\n", waypoint);
		fprintf(stderr, "dbg2       xgrid:            %f\n", xgrid);
		fprintf(stderr, "dbg2       ygrid:            %f\n", ygrid);
		fprintf(stderr, "dbg2       xlon:             %f\n", xlon);
		fprintf(stderr, "dbg2       ylat:             %f\n", ylat);
		fprintf(stderr, "dbg2       zdata:            %f\n", zdata);
		fprintf(stderr, "dbg2       xdisplay:         %f\n", xdisplay);
		fprintf(stderr, "dbg2       ydisplay:         %f\n", ydisplay);
		fprintf(stderr, "dbg2       zdisplay:         %f\n", zdisplay);
	}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* add route if required */
	if (inew == shared.shareddata.nroute) {
		/* allocate memory for a new route if required */
		if (shared.shareddata.nroute_alloc < shared.shareddata.nroute + 1) {
			shared.shareddata.nroute_alloc += MBV_ALLOC_NUM;
			status =
			    mb_reallocd(mbv_verbose, __FILE__, __LINE__, shared.shareddata.nroute_alloc * sizeof(struct mbview_route_struct),
			                (void **)&(shared.shareddata.routes), &error);
			if (status == MB_FAILURE) {
				shared.shareddata.nroute_alloc = 0;
			}
			else {
				memset((void *)&shared.shareddata.routes[shared.shareddata.nroute], 0,
				       MBV_ALLOC_NUM * sizeof(struct mbview_route_struct));
				for (i = shared.shareddata.nroute; i < shared.shareddata.nroute_alloc; i++) {
					shared.shareddata.routes[i].color = false;
					shared.shareddata.routes[i].color = MBV_COLOR_RED;
					shared.shareddata.routes[i].size = 1;
					shared.shareddata.routes[i].editmode = true;
					shared.shareddata.routes[i].name[0] = '\0';
					shared.shareddata.routes[i].npoints = 0;
					shared.shareddata.routes[i].npoints_alloc = MBV_ALLOC_NUM;
					shared.shareddata.routes[i].waypoint = NULL;
					shared.shareddata.routes[i].distlateral = NULL;
					shared.shareddata.routes[i].disttopo = NULL;
					shared.shareddata.routes[i].points = NULL;
					shared.shareddata.routes[i].segments = NULL;
					status = mb_reallocd(mbv_verbose, __FILE__, __LINE__, shared.shareddata.routes[i].npoints_alloc * sizeof(int),
					                     (void **)&(shared.shareddata.routes[i].waypoint), &error);
					status = mb_reallocd(mbv_verbose, __FILE__, __LINE__, shared.shareddata.routes[i].npoints_alloc * sizeof(double),
					                (void **)&(shared.shareddata.routes[i].distlateral), &error);
					status = mb_reallocd(mbv_verbose, __FILE__, __LINE__, shared.shareddata.routes[i].npoints_alloc * sizeof(double),
					                (void **)&(shared.shareddata.routes[i].disttopo), &error);
					status = mb_reallocd(mbv_verbose, __FILE__, __LINE__,
					                     shared.shareddata.routes[i].npoints_alloc * sizeof(struct mbview_pointw_struct),
					                     (void **)&(shared.shareddata.routes[i].points), &error);
					status = mb_reallocd(mbv_verbose, __FILE__, __LINE__,
					                     shared.shareddata.routes[i].npoints_alloc * sizeof(struct mbview_linesegmentw_struct),
					                     (void **)&(shared.shareddata.routes[i].segments), &error);
					memset((void *)shared.shareddata.routes[i].waypoint, 0,
					       shared.shareddata.routes[i].npoints_alloc * sizeof(int));
					memset((void *)shared.shareddata.routes[i].distlateral, 0,
					       shared.shareddata.routes[i].npoints_alloc * sizeof(double));
					memset((void *)shared.shareddata.routes[i].disttopo, 0,
					       shared.shareddata.routes[i].npoints_alloc * sizeof(double));
					memset((void *)shared.shareddata.routes[i].points, 0,
					       shared.shareddata.routes[i].npoints_alloc * sizeof(struct mbview_pointw_struct));
					memset((void *)shared.shareddata.routes[i].segments, 0,
					       shared.shareddata.routes[i].npoints_alloc * sizeof(struct mbview_linesegmentw_struct));
				}
			}
		}

		/* set nroute */
		shared.shareddata.nroute++;

		/* add the new route */
		shared.shareddata.routes[inew].active = true;
		shared.shareddata.routes[inew].color = MBV_COLOR_BLACK;
		shared.shareddata.routes[inew].size = 1;
		shared.shareddata.routes[inew].editmode = true;
		sprintf(shared.shareddata.routes[inew].name, "Route:%d", shared.shareddata.nroute);
	}

	/* allocate memory for point if required */
	if (status == MB_SUCCESS && shared.shareddata.routes[inew].npoints_alloc < shared.shareddata.routes[inew].npoints + 1) {
		npoints = shared.shareddata.routes[inew].npoints;
		if (shared.shareddata.routes[inew].npoints_alloc == 0)
			npoints_alloc = 2;
		else if (shared.shareddata.routes[inew].npoints_alloc < MBV_ALLOC_NUM)
			npoints_alloc = MBV_ALLOC_NUM;
		else
			npoints_alloc = shared.shareddata.routes[inew].npoints_alloc + MBV_ALLOC_NUM;
		status = mb_reallocd(mbv_verbose, __FILE__, __LINE__, npoints_alloc * sizeof(int),
		                     (void **)&(shared.shareddata.routes[inew].waypoint), &error);
		status = mb_reallocd(mbv_verbose, __FILE__, __LINE__, npoints_alloc * sizeof(double),
		                     (void **)&(shared.shareddata.routes[inew].distlateral), &error);
		status = mb_reallocd(mbv_verbose, __FILE__, __LINE__, npoints_alloc * sizeof(double),
		                     (void **)&(shared.shareddata.routes[inew].disttopo), &error);
		status = mb_reallocd(mbv_verbose, __FILE__, __LINE__, npoints_alloc * sizeof(struct mbview_pointw_struct),
		                     (void **)&(shared.shareddata.routes[inew].points), &error);
		status = mb_reallocd(mbv_verbose, __FILE__, __LINE__, npoints_alloc * sizeof(struct mbview_linesegmentw_struct),
		                     (void **)&(shared.shareddata.routes[inew].segments), &error);
		npoints_diff = npoints_alloc - npoints;
		memset((void *)&shared.shareddata.routes[inew].waypoint[npoints], 0, npoints_diff * sizeof(int));
		memset((void *)&shared.shareddata.routes[inew].distlateral[npoints], 0, npoints_diff * sizeof(double));
		memset((void *)&shared.shareddata.routes[inew].disttopo[npoints], 0, npoints_diff * sizeof(double));
		memset((void *)&shared.shareddata.routes[inew].points[npoints], 0, npoints_diff * sizeof(struct mbview_pointw_struct));
		memset((void *)&shared.shareddata.routes[inew].segments[npoints], 0,
		       npoints_diff * sizeof(struct mbview_linesegmentw_struct));
		if (status == MB_SUCCESS) {
			shared.shareddata.routes[inew].npoints_alloc = npoints_alloc;
		}
		else {
			shared.shareddata.routes[inew].npoints = 0;
			shared.shareddata.routes[inew].npoints_alloc = 0;
		}
	}

	/* add the new route point */
	// fprintf(stderr,"mbview_route_add: inew:%d jnew:%d waypoint:%d
	// editmode:%d\n",inew,jnew,waypoint,shared.shareddata.routes[inew].editmode);
	if (status == MB_SUCCESS) {
		/* move points after jnew if necessary */
		for (j = shared.shareddata.routes[inew].npoints; j > jnew; j--) {
			shared.shareddata.routes[inew].waypoint[j] = shared.shareddata.routes[inew].waypoint[j - 1];
			shared.shareddata.routes[inew].points[j] = shared.shareddata.routes[inew].points[j - 1];
		}

		/* move segments after jnew if necessary */
		for (j = shared.shareddata.routes[inew].npoints - 1; j > jnew; j--) {
			shared.shareddata.routes[inew].segments[j] = shared.shareddata.routes[inew].segments[j - 1];
			shared.shareddata.routes[inew].segments[j].endpoints[0] = shared.shareddata.routes[inew].points[j];
			shared.shareddata.routes[inew].segments[j].endpoints[1] = shared.shareddata.routes[inew].points[j + 1];
		}

		/* add the new point */
		shared.shareddata.routes[inew].waypoint[jnew] = waypoint;
		shared.shareddata.routes[inew].points[jnew].xgrid[instance] = xgrid;
		shared.shareddata.routes[inew].points[jnew].ygrid[instance] = ygrid;
		shared.shareddata.routes[inew].points[jnew].xlon = xlon;
		shared.shareddata.routes[inew].points[jnew].ylat = ylat;
		shared.shareddata.routes[inew].points[jnew].zdata = zdata;
		shared.shareddata.routes[inew].points[jnew].xdisplay[instance] = xdisplay;
		shared.shareddata.routes[inew].points[jnew].ydisplay[instance] = ydisplay;
		shared.shareddata.routes[inew].points[jnew].zdisplay[instance] = zdisplay;
		mbview_updatepointw(instance, &(shared.shareddata.routes[inew].points[jnew]));

		/* initialize the new segment */
		shared.shareddata.routes[inew].segments[jnew].nls = 0;
		shared.shareddata.routes[inew].segments[jnew].nls_alloc = 0;
		shared.shareddata.routes[inew].segments[jnew].lspoints = NULL;
		shared.shareddata.routes[inew].segments[jnew].endpoints[0] = shared.shareddata.routes[inew].points[jnew];
		shared.shareddata.routes[inew].segments[jnew].endpoints[1] = shared.shareddata.routes[inew].points[jnew + 1];
		if (jnew > 0) {
			shared.shareddata.routes[inew].segments[jnew - 1].endpoints[0] = shared.shareddata.routes[inew].points[jnew - 1];
			shared.shareddata.routes[inew].segments[jnew - 1].endpoints[1] = shared.shareddata.routes[inew].points[jnew];
		}

		/* set npoints */
		shared.shareddata.routes[inew].npoints++;

		/* reset affected segment endpoints */
		if (shared.shareddata.routes[inew].npoints > 0) {
			for (j = MAX(0, jnew - 1); j < MIN(shared.shareddata.routes[inew].npoints - 1, jnew + 1); j++) {
				shared.shareddata.routes[inew].segments[j].endpoints[0] = shared.shareddata.routes[inew].points[j];
				shared.shareddata.routes[inew].segments[j].endpoints[1] = shared.shareddata.routes[inew].points[j + 1];

				/* drape the segment */
				mbview_drapesegmentw(instance, &(shared.shareddata.routes[inew].segments[j]));

				/* update the segment for all active instances */
				mbview_updatesegmentw(instance, &(shared.shareddata.routes[inew].segments[j]));
			}
		}

		/* set or reset distance values */
		mbview_route_setdistance(instance, inew);

		/* make routes viewable */
		if (data->route_view_mode != MBV_VIEW_ON) {
			data->route_view_mode = MBV_VIEW_ON;
			set_mbview_route_view_mode(instance, MBV_VIEW_ON);
		}
	}

	/* else beep */
	else {
		XBell(view->dpy, 100);
	}

	/* print route debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  Route data altered in function <%s>\n", __func__);
		fprintf(stderr, "dbg2  Route values:\n");
		fprintf(stderr, "dbg2       route_view_mode:      %d\n", data->route_view_mode);
		fprintf(stderr, "dbg2       route_mode:           %d\n", shared.shareddata.route_mode);
		fprintf(stderr, "dbg2       nroute:               %d\n", shared.shareddata.nroute);
		fprintf(stderr, "dbg2       nroute_alloc:         %d\n", shared.shareddata.nroute_alloc);
		fprintf(stderr, "dbg2       route_selected:       %d\n", shared.shareddata.route_selected);
		fprintf(stderr, "dbg2       route_point_selected: %d\n", shared.shareddata.route_point_selected);
		for (i = 0; i < shared.shareddata.nroute; i++) {
			fprintf(stderr, "dbg2       route %d active:        %d\n", i, shared.shareddata.routes[i].active);
			fprintf(stderr, "dbg2       route %d color:         %d\n", i, shared.shareddata.routes[i].color);
			fprintf(stderr, "dbg2       route %d size:          %d\n", i, shared.shareddata.routes[i].size);
			fprintf(stderr, "dbg2       route %d name:          %s\n", i, shared.shareddata.routes[i].name);
			fprintf(stderr, "dbg2       route %d npoints:       %d\n", i, shared.shareddata.routes[i].npoints);
			fprintf(stderr, "dbg2       route %d npoints_alloc: %d\n", i, shared.shareddata.routes[i].npoints_alloc);
			fprintf(stderr, "dbg2       route points: iroute jpoint xgrid[instance] ygrid[instance] xlon ylat zdata "
			                "xdisplay[instance] ydisplay[instance] zdisplay[instance]\n");
			for (j = 0; j < shared.shareddata.routes[i].npoints; j++) {
				fprintf(stderr, "dbg2       %d %d %f %f %f %f %f %f %f %f\n", i, j,
				        shared.shareddata.routes[i].points[j].xgrid[instance],
				        shared.shareddata.routes[i].points[j].ygrid[instance], shared.shareddata.routes[i].points[j].xlon,
				        shared.shareddata.routes[i].points[j].ylat, shared.shareddata.routes[i].points[j].zdata,
				        shared.shareddata.routes[i].points[j].xdisplay[instance],
				        shared.shareddata.routes[i].points[j].ydisplay[instance],
				        shared.shareddata.routes[i].points[j].zdisplay[instance]);
			}
			for (j = 0; j < shared.shareddata.routes[i].npoints - 1; j++) {
				fprintf(stderr, "dbg2       route %d %d nls:          %d\n", i, j, shared.shareddata.routes[i].segments[j].nls);
				fprintf(stderr, "dbg2       route %d %d nls_alloc:    %d\n", i, j,
				        shared.shareddata.routes[i].segments[j].nls_alloc);
				fprintf(stderr, "dbg2       route %d %d endpoints[0]: %f %f %f %f %f %f %f %f\n", i, j,
				        shared.shareddata.routes[i].segments[j].endpoints[0].xgrid[instance],
				        shared.shareddata.routes[i].segments[j].endpoints[0].ygrid[instance],
				        shared.shareddata.routes[i].segments[j].endpoints[0].xlon,
				        shared.shareddata.routes[i].segments[j].endpoints[0].ylat,
				        shared.shareddata.routes[i].segments[j].endpoints[0].zdata,
				        shared.shareddata.routes[i].segments[j].endpoints[0].xdisplay[instance],
				        shared.shareddata.routes[i].segments[j].endpoints[0].ydisplay[instance],
				        shared.shareddata.routes[i].segments[j].endpoints[0].zdisplay[instance]);
				fprintf(stderr, "dbg2       route %d %d endpoints[1]: %f %f %f %f %f %f %f %f\n", i, j,
				        shared.shareddata.routes[i].segments[j].endpoints[1].xgrid[instance],
				        shared.shareddata.routes[i].segments[j].endpoints[1].ygrid[instance],
				        shared.shareddata.routes[i].segments[j].endpoints[1].xlon,
				        shared.shareddata.routes[i].segments[j].endpoints[1].ylat,
				        shared.shareddata.routes[i].segments[j].endpoints[1].zdata,
				        shared.shareddata.routes[i].segments[j].endpoints[1].xdisplay[instance],
				        shared.shareddata.routes[i].segments[j].endpoints[1].ydisplay[instance],
				        shared.shareddata.routes[i].segments[j].endpoints[1].zdisplay[instance]);
				fprintf(stderr, "dbg2       segment points: kpoint xgrid[instance] ygrid[instance] xlon ylat zdata "
				                "xdisplay[instance] ydisplay[instance] zdisplay[instance]\n");
				seg = (struct mbview_linesegmentw_struct *)&(shared.shareddata.routes[inew].segments[j]);
				for (k = 0; k < seg->nls; k++) {
					fprintf(stderr, "dbg2         %d %f %f %f  %f %f  %f %f %f\n", k, seg->lspoints[k].xgrid[instance],
					        seg->lspoints[k].ygrid[instance], seg->lspoints[k].zdata, seg->lspoints[k].xlon,
					        seg->lspoints[k].ylat, seg->lspoints[k].xdisplay[instance], seg->lspoints[k].ydisplay[instance],
					        seg->lspoints[k].zdisplay[instance]);
				}
			}
		}
	}

	/* print output debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	/* return */
	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_route_delete(size_t instance, int iroute, int ipoint) {

	/* local variables */
	int status = MB_SUCCESS;
	int error = MB_ERROR_NO_ERROR;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int idelete;
	int i, j;

	/* print starting debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       iroute:          %d\n", iroute);
		fprintf(stderr, "dbg2       ipoint:          %d\n", ipoint);
	}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	fprintf(stderr, "mbview_route_delete: iroute:%d ipoint:%d editmode:%d\n", iroute, ipoint,
	        shared.shareddata.routes[iroute].editmode);
	/* delete route point if its valid */
	if (iroute >= 0 && iroute < shared.shareddata.nroute && ipoint >= 0 && ipoint < shared.shareddata.routes[iroute].npoints &&
	    shared.shareddata.routes[iroute].editmode) {
		/* free segment immediately after deleted point if in the middle of the
		    route or before if it is at the end */
		if (shared.shareddata.routes[iroute].npoints > 1) {
			if (ipoint < shared.shareddata.routes[iroute].npoints - 1) {
				idelete = ipoint;
			}
			else {
				idelete = ipoint - 1;
			}
			if (shared.shareddata.routes[iroute].segments[idelete].nls_alloc > 0 &&
			    shared.shareddata.routes[iroute].segments[idelete].lspoints != NULL) {
				mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(shared.shareddata.routes[iroute].segments[idelete].lspoints),
				         &error);
				shared.shareddata.routes[iroute].segments[idelete].nls = 0;
				shared.shareddata.routes[iroute].segments[idelete].nls_alloc = 0;
			}
		}

		/* move route point data if necessary */
		for (j = ipoint; j < shared.shareddata.routes[iroute].npoints - 1; j++) {
			shared.shareddata.routes[iroute].waypoint[j] = shared.shareddata.routes[iroute].waypoint[j + 1];
			shared.shareddata.routes[iroute].points[j] = shared.shareddata.routes[iroute].points[j + 1];
		}

		/* move route segment data if necessary */
		for (j = ipoint; j < shared.shareddata.routes[iroute].npoints - 2; j++) {
			shared.shareddata.routes[iroute].segments[j] = shared.shareddata.routes[iroute].segments[j + 1];
		}
		j = shared.shareddata.routes[iroute].npoints - 2;
		if (j >= 0) {
			shared.shareddata.routes[iroute].segments[j].nls = 0;
			shared.shareddata.routes[iroute].segments[j].nls_alloc = 0;
			shared.shareddata.routes[iroute].segments[j].lspoints = NULL;
		}

		/* decrement npoints */
		shared.shareddata.routes[iroute].npoints--;

		/* if route still has points then reset affected segment endpoints */
		if (shared.shareddata.routes[iroute].npoints > 0) {
			for (j = MAX(0, ipoint - 1); j < shared.shareddata.routes[iroute].npoints - 1; j++) {
				shared.shareddata.routes[iroute].segments[j].endpoints[0] = shared.shareddata.routes[iroute].points[j];
				shared.shareddata.routes[iroute].segments[j].endpoints[1] = shared.shareddata.routes[iroute].points[j + 1];

				/* drape the segment */
				mbview_drapesegmentw(instance, &(shared.shareddata.routes[iroute].segments[j]));

				/* update the segment for all active instances */
				mbview_updatesegmentw(instance, &(shared.shareddata.routes[iroute].segments[j]));
			}
		}

		/* if route still has points then reset distance values */
		if (shared.shareddata.routes[iroute].npoints > 0) {
			mbview_route_setdistance(instance, iroute);
		}

		/* if last point deleted then move remaining routes if necessary */
		if (shared.shareddata.routes[iroute].npoints <= 0) {
			/* move route data if necessary */
			for (i = iroute; i < shared.shareddata.nroute - 1; i++) {
				shared.shareddata.routes[i] = shared.shareddata.routes[i + 1];
			}

			/* decrement nroute */
			shared.shareddata.nroute--;
		}

		/* no route selection now */
		if (shared.shareddata.route_selected != MBV_SELECT_NONE) {
			shared.shareddata.route_selected = MBV_SELECT_NONE;
			shared.shareddata.route_point_selected = MBV_SELECT_NONE;
			data->pickinfo_mode = data->pick_type;
		}
	}

	/* else beep */
	else {
		XBell(view->dpy, 100);
	}

	/* print route debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  Route data altered in function <%s>\n", __func__);
		fprintf(stderr, "dbg2  Route values:\n");
		fprintf(stderr, "dbg2       route_view_mode:      %d\n", data->route_view_mode);
		fprintf(stderr, "dbg2       route_mode:           %d\n", shared.shareddata.route_mode);
		fprintf(stderr, "dbg2       nroute:               %d\n", shared.shareddata.nroute);
		fprintf(stderr, "dbg2       nroute_alloc:         %d\n", shared.shareddata.nroute_alloc);
		fprintf(stderr, "dbg2       route_selected:       %d\n", shared.shareddata.route_selected);
		fprintf(stderr, "dbg2       route_point_selected: %d\n", shared.shareddata.route_point_selected);
		for (i = 0; i < shared.shareddata.nroute; i++) {
			fprintf(stderr, "dbg2       route %d active:        %d\n", i, shared.shareddata.routes[i].active);
			fprintf(stderr, "dbg2       route %d color:         %d\n", i, shared.shareddata.routes[i].color);
			fprintf(stderr, "dbg2       route %d size:          %d\n", i, shared.shareddata.routes[i].size);
			fprintf(stderr, "dbg2       route %d name:          %s\n", i, shared.shareddata.routes[i].name);
			fprintf(stderr, "dbg2       route %d npoints:       %d\n", i, shared.shareddata.routes[i].npoints);
			fprintf(stderr, "dbg2       route %d npoints_alloc: %d\n", i, shared.shareddata.routes[i].npoints_alloc);
			for (j = 0; j < shared.shareddata.routes[i].npoints; j++) {
				fprintf(stderr, "dbg2       route %d %d xgrid:    %f\n", i, j,
				        shared.shareddata.routes[i].points[j].xgrid[instance]);
				fprintf(stderr, "dbg2       route %d %d ygrid:    %f\n", i, j,
				        shared.shareddata.routes[i].points[j].ygrid[instance]);
				fprintf(stderr, "dbg2       route %d %d xlon:     %f\n", i, j, shared.shareddata.routes[i].points[j].xlon);
				fprintf(stderr, "dbg2       route %d %d ylat:     %f\n", i, j, shared.shareddata.routes[i].points[j].ylat);
				fprintf(stderr, "dbg2       route %d %d zdata:    %f\n", i, j, shared.shareddata.routes[i].points[j].zdata);
				fprintf(stderr, "dbg2       route %d %d xdisplay: %f\n", i, j,
				        shared.shareddata.routes[i].points[j].xdisplay[instance]);
				fprintf(stderr, "dbg2       route %d %d ydisplay: %f\n", i, j,
				        shared.shareddata.routes[i].points[j].ydisplay[instance]);
				fprintf(stderr, "dbg2       route %d %d zdisplay: %f\n", i, j,
				        shared.shareddata.routes[i].points[j].zdisplay[instance]);
			}
			for (j = 0; j < shared.shareddata.routes[i].npoints - 1; j++) {
				fprintf(stderr, "dbg2       route %d %d nls:          %d\n", i, j, shared.shareddata.routes[i].segments[j].nls);
				fprintf(stderr, "dbg2       route %d %d nls_alloc:    %d\n", i, j,
				        shared.shareddata.routes[i].segments[j].nls_alloc);
				fprintf(stderr, "dbg2       route %d %d endpoints[0]: %p\n", i, j,
				        &shared.shareddata.routes[i].segments[j].endpoints[0]);
				fprintf(stderr, "dbg2       route %d %d endpoints[1]: %p\n", i, j,
				        &shared.shareddata.routes[i].segments[j].endpoints[1]);
			}
		}
	}

	/* print output debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	/* return */
	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_route_setdistance(size_t instance, int working_route) {
	/* local variables */
	int status = MB_SUCCESS;
	struct mbview_route_struct *route = NULL;
	bool valid_route = false;
	double distlateral, distovertopo;
	double routelon0, routelon1;
	double routelat0, routelat1;
	double routetopo0, routetopo1;
	double routeslope;
	int i, j;

	/* print starting debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", mbv_verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       working_route:             %d\n", working_route);
	}

	/* get view */
	// struct mbview_world_struct *view = &(mbviews[instance]);
	// struct mbview_struct *data = &(view->data);

	/* check that the route is valid */
	if (working_route >= 0 && working_route < shared.shareddata.nroute
      && shared.shareddata.routes[working_route].npoints > 0 && shared.shareddata.routes[working_route].active) {
		/* get route pointer */
		route = &(shared.shareddata.routes[working_route]);
		valid_route = true;

		/* loop over the route segments */
		route->distancelateral = 0.0;
		route->distancetopo = 0.0;
		route->nroutepoint = 0;
		for (i = 0; i < route->npoints - 1; i++) {
			/* do first point */
			routelon1 = route->points[i].xlon;
			if (routelon1 < -180.0)
				routelon1 += 360.0;
			else if (routelon1 > 180.0)
				routelon1 -= 360.0;
			routelat1 = route->points[i].ylat;
			routetopo1 = route->points[i].zdata;
			if (route->nroutepoint == 0) {
				distlateral = 0.0;
				distovertopo = 0.0;
			}
			else {
				mbview_projectdistance(instance, routelon0, routelat0, routetopo0, routelon1, routelat1, routetopo1, &distlateral,
				                       &distovertopo, &routeslope);
			}
			route->distancelateral += distlateral;
			route->distancetopo += distovertopo;
			routelon0 = routelon1;
			routelat0 = routelat1;
			routetopo0 = routetopo1;
			route->nroutepoint++;

			/* set distances for route waypoint */
			route->distlateral[i] = route->distancelateral;
			route->disttopo[i] = route->distancetopo;

			/* loop over interior of segment */
			for (j = 1; j < route->segments[i].nls - 1; j++) {
				routelon1 = route->segments[i].lspoints[j].xlon;
				if (routelon1 < -180.0)
					routelon1 += 360.0;
				else if (routelon1 > 180.0)
					routelon1 -= 360.0;
				routelat1 = route->segments[i].lspoints[j].ylat;
				routetopo1 = route->segments[i].lspoints[j].zdata;
				mbview_projectdistance(instance, routelon0, routelat0, routetopo0, routelon1, routelat1, routetopo1, &distlateral,
				                       &distovertopo, &routeslope);
				route->distancelateral += distlateral;
				route->distancetopo += distovertopo;
				routelon0 = routelon1;
				routelat0 = routelat1;
				routetopo0 = routetopo1;
				route->nroutepoint++;
			}
		}

		/* do last point */
		j = route->npoints - 1;
		routelon1 = route->points[j].xlon;
		if (routelon1 < -180.0)
			routelon1 += 360.0;
		else if (routelon1 > 180.0)
			routelon1 -= 360.0;
		routelat1 = route->points[j].ylat;
		routetopo1 = route->points[j].zdata;
		if (j > 0) {
		  mbview_projectdistance(instance, routelon0, routelat0, routetopo0, routelon1, routelat1, routetopo1, &distlateral,
		                       &distovertopo, &routeslope);
		  route->distancelateral += distlateral;
		  route->distancetopo += distovertopo;
		}
		route->nroutepoint++;

		/* set distances for route waypoint */
		route->distlateral[j] = route->distancelateral;
		route->disttopo[j] = route->distancetopo;
	}

	/* print output debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		if (valid_route) {
			route = &(shared.shareddata.routes[working_route]);
			fprintf(stderr, "dbg2       routedistancelateral:      %f\n", route->distancelateral);
			fprintf(stderr, "dbg2       routedistancetopo:         %f\n", route->distancetopo);
		}
		else {
			route = &(shared.shareddata.routes[working_route]);
			fprintf(stderr, "dbg2       invalid working route:     %d\n", working_route);
		}
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	/* return */
	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_drawroute(size_t instance, int rez) {
	/* local variables */
	int status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	GLUquadricObj *globj;
	double routesizesmall, routesizelarge;
	double xx, yy;
	int iroute, jpoint;
	int icolor;
	int k, k0, k1;

	/* print starting debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       rez:              %d\n", rez);
	}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* Generate GL lists to be plotted */
	if (shared.shareddata.route_mode != MBV_ROUTE_OFF && data->route_view_mode == MBV_VIEW_ON && shared.shareddata.nroute > 0) {
		/* get size according to viewbounds */
		k0 = data->viewbounds[0] * data->primary_n_rows + data->viewbounds[2];
		k1 = data->viewbounds[1] * data->primary_n_rows + data->viewbounds[3];
		xx = data->primary_x[k1] - data->primary_x[k0];
		yy = data->primary_y[k1] - data->primary_y[k0];
		routesizesmall = 0.004 * sqrt(xx * xx + yy * yy);
		routesizelarge = 1.4 * routesizesmall;

		/* Use disks for 2D plotting */
		if (data->display_mode == MBV_DISPLAY_2D) {
			/* make list for small route */
			glNewList((GLuint)MBV_GLLIST_ROUTESMALL, GL_COMPILE);
			globj = gluNewQuadric();
			gluDisk(globj, 0.0, routesizesmall, 4, 1);
			gluDeleteQuadric(globj);
			icolor = MBV_COLOR_BLACK;
			glColor3f(colortable_object_red[icolor], colortable_object_green[icolor], colortable_object_blue[icolor]);
			globj = gluNewQuadric();
			gluDisk(globj, 0.8 * routesizesmall, routesizesmall, 10, 1);
			gluDeleteQuadric(globj);
			glEndList();

			/* make list for large route */
			glNewList((GLuint)MBV_GLLIST_ROUTELARGE, GL_COMPILE);
			globj = gluNewQuadric();
			gluDisk(globj, 0.0, routesizelarge, 4, 1);
			gluDeleteQuadric(globj);
			icolor = MBV_COLOR_BLACK;
			glColor3f(colortable_object_red[icolor], colortable_object_green[icolor], colortable_object_blue[icolor]);
			globj = gluNewQuadric();
			gluDisk(globj, 0.8 * routesizelarge, routesizelarge, 10, 1);
			gluDeleteQuadric(globj);
			glEndList();
		}

		/* Use spheres for 3D plotting */
		else if (data->display_mode == MBV_DISPLAY_3D) {
			/* make list for small route */
			glNewList((GLuint)MBV_GLLIST_ROUTESMALL, GL_COMPILE);
			globj = gluNewQuadric();
			gluSphere(globj, routesizesmall, 4, 3);
			gluDeleteQuadric(globj);
			glEndList();

			/* make list for large route */
			glNewList((GLuint)MBV_GLLIST_ROUTELARGE, GL_COMPILE);
			globj = gluNewQuadric();
			gluSphere(globj, routesizelarge, 4, 3);
			gluDeleteQuadric(globj);
			glEndList();
		}

		/* loop over the route points */
		for (iroute = 0; iroute < shared.shareddata.nroute; iroute++) {
      if (shared.shareddata.routes[iroute].active) {
  			for (jpoint = 0; jpoint < shared.shareddata.routes[iroute].npoints; jpoint++) {

  				/* set the color for this route */
  				if (iroute == shared.shareddata.route_selected && (jpoint == shared.shareddata.route_point_selected ||
  				                                                   shared.shareddata.route_point_selected == MBV_SELECT_ALL))
  					icolor = MBV_COLOR_RED;
  				else
  					icolor = shared.shareddata.routes[iroute].color;
  				glColor3f(colortable_object_red[icolor], colortable_object_green[icolor], colortable_object_blue[icolor]);

  				/* draw the route point as a disk or sphere using GLUT */

  				glTranslatef(shared.shareddata.routes[iroute].points[jpoint].xdisplay[instance],
  				             shared.shareddata.routes[iroute].points[jpoint].ydisplay[instance],
  				             shared.shareddata.routes[iroute].points[jpoint].zdisplay[instance]);
  				if (iroute == shared.shareddata.route_selected && (jpoint == shared.shareddata.route_point_selected ||
  				                                                   shared.shareddata.route_point_selected == MBV_SELECT_ALL))
  					glCallList((GLuint)MBV_GLLIST_ROUTELARGE);
  				else
  					glCallList((GLuint)MBV_GLLIST_ROUTESMALL);
  				glTranslatef(-shared.shareddata.routes[iroute].points[jpoint].xdisplay[instance],
  				             -shared.shareddata.routes[iroute].points[jpoint].ydisplay[instance],
  				             -shared.shareddata.routes[iroute].points[jpoint].zdisplay[instance]);
  			}

  			/* draw draped route line segments */
  			glColor3f(0.0, 0.0, 0.0);
  			glLineWidth((float)(2.0));
  			glBegin(GL_LINE_STRIP);
  			for (jpoint = 0; jpoint < shared.shareddata.routes[iroute].npoints - 1; jpoint++) {
  				/* set size and color */
  				if (iroute == shared.shareddata.route_selected &&
  				    (jpoint == shared.shareddata.route_point_selected || jpoint == shared.shareddata.route_point_selected - 1 ||
  				     shared.shareddata.route_point_selected == MBV_SELECT_ALL)) {
  					icolor = MBV_COLOR_RED;
  				}
  				else {
  					icolor = shared.shareddata.routes[iroute].color;
  				}
  				glColor3f(colortable_object_red[icolor], colortable_object_green[icolor], colortable_object_blue[icolor]);

  				/* draw draped segment */
  				for (k = 0; k < shared.shareddata.routes[iroute].segments[jpoint].nls; k++) {
  					/* draw points */
  					glVertex3f((float)(shared.shareddata.routes[iroute].segments[jpoint].lspoints[k].xdisplay[instance]),
  					           (float)(shared.shareddata.routes[iroute].segments[jpoint].lspoints[k].ydisplay[instance]),
  					           (float)(shared.shareddata.routes[iroute].segments[jpoint].lspoints[k].zdisplay[instance]));
  				}
  			}
  			glEnd();
  		}
    }
	}
#ifdef MBV_GETERRORS
	mbview_glerrorcheck(instance, 1, __func__);
#endif

	/* print output debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	/* return */
	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_updateroutelist() {
	/* local variables */
	int status = MB_SUCCESS;
	XmString *xstr;
	int jpoint;
	int nitems;
	int iitem;
	char londstr0[24], lonmstr0[24];
	char latdstr0[24], latmstr0[24];
	char waypointstr[10];
	int iroute, jwaypoint;

	/* print starting debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
	}

	/* update route list */
	if (shared.init_routelist == MBV_WINDOW_VISIBLE) {
		/* remove all existing items */
		XmListDeleteAllItems(shared.mb3d_routelist.mbview_list_routelist);

		if (shared.shareddata.nroute > 0) {
			/* get number of items */
			nitems = 0;
			for (iroute = 0; iroute < shared.shareddata.nroute; iroute++) {
        if (shared.shareddata.routes[iroute].active) {
  				nitems += 1 + shared.shareddata.routes[iroute].npoints;
        }
			}

      if (nitems > 0) {
  			/* allocate array of label XmStrings */
  			xstr = (XmString *)malloc(nitems * sizeof(XmString));

  			/* loop over the routes */
  			nitems = 0;
  			for (iroute = 0; iroute < shared.shareddata.nroute; iroute++) {
          if (shared.shareddata.routes[iroute].active) {
    				if (shared.shareddata.routes[iroute].editmode)
    					sprintf(value_string, "Editable Route %3d | Waypoints:%3d | Length:%.2f %.2f m | %s | Name: %s", iroute,
    					        shared.shareddata.routes[iroute].npoints, shared.shareddata.routes[iroute].distancelateral,
    					        shared.shareddata.routes[iroute].distancetopo,
    					        mbview_colorname[shared.shareddata.routes[iroute].color], shared.shareddata.routes[iroute].name);
    				else
    					sprintf(value_string, "Static Route %3d | Waypoints:%3d | Length:%.2f %.2f m | %s | Name: %s", iroute,
    					        shared.shareddata.routes[iroute].npoints, shared.shareddata.routes[iroute].distancelateral,
    					        shared.shareddata.routes[iroute].distancetopo,
    					        mbview_colorname[shared.shareddata.routes[iroute].color], shared.shareddata.routes[iroute].name);
    				xstr[nitems] = XmStringCreateLocalized(value_string);
    				nitems++;

    				/* add list item for each waypoint */
    				for (jpoint = 0; jpoint < shared.shareddata.routes[iroute].npoints; jpoint++) {
    					/* add list item for each route */
    					mbview_setlonlatstrings(shared.shareddata.routes[iroute].points[jpoint].xlon,
    					                        shared.shareddata.routes[iroute].points[jpoint].ylat, londstr0, latdstr0, lonmstr0,
    					                        latmstr0);

    					if (shared.shareddata.routes[iroute].waypoint[jpoint] == MBV_ROUTE_WAYPOINT_SIMPLE)
    						strcpy(waypointstr, "---------");
    					else if (shared.shareddata.routes[iroute].waypoint[jpoint] == MBV_ROUTE_WAYPOINT_TRANSIT)
    						strcpy(waypointstr, "-TRANSIT-");
    					else if (shared.shareddata.routes[iroute].waypoint[jpoint] == MBV_ROUTE_WAYPOINT_STARTLINE)
    						strcpy(waypointstr, "--START--");
    					else if (shared.shareddata.routes[iroute].waypoint[jpoint] == MBV_ROUTE_WAYPOINT_ENDLINE)
    						strcpy(waypointstr, "---END---");
    					else if (shared.shareddata.routes[iroute].waypoint[jpoint] == MBV_ROUTE_WAYPOINT_STARTLINE2)
    						strcpy(waypointstr, "--START2-");
    					else if (shared.shareddata.routes[iroute].waypoint[jpoint] == MBV_ROUTE_WAYPOINT_ENDLINE2)
    						strcpy(waypointstr, "---END2--");
    					else if (shared.shareddata.routes[iroute].waypoint[jpoint] == MBV_ROUTE_WAYPOINT_STARTLINE3)
    						strcpy(waypointstr, "--START3-");
    					else if (shared.shareddata.routes[iroute].waypoint[jpoint] == MBV_ROUTE_WAYPOINT_ENDLINE3)
    						strcpy(waypointstr, "---END3--");
    					else if (shared.shareddata.routes[iroute].waypoint[jpoint] == MBV_ROUTE_WAYPOINT_STARTLINE4)
    						strcpy(waypointstr, "--START4-");
    					else if (shared.shareddata.routes[iroute].waypoint[jpoint] == MBV_ROUTE_WAYPOINT_ENDLINE4)
    						strcpy(waypointstr, "---END4--");
    					else if (shared.shareddata.routes[iroute].waypoint[jpoint] == MBV_ROUTE_WAYPOINT_STARTLINE5)
    						strcpy(waypointstr, "--START5-");
    					else if (shared.shareddata.routes[iroute].waypoint[jpoint] == MBV_ROUTE_WAYPOINT_ENDLINE5)
    						strcpy(waypointstr, "---END5--");
    					else
    						strcpy(waypointstr, "-------");
    					if (shared.lonlatstyle == MBV_LONLAT_DEGREESDECIMAL)
    						sprintf(value_string, "%3d | %3d | %s | %s | %.2f | %.2f | %.2f | %s", iroute, jpoint, londstr0, latdstr0,
    						        shared.shareddata.routes[iroute].points[jpoint].zdata,
    						        shared.shareddata.routes[iroute].distlateral[jpoint],
    						        shared.shareddata.routes[iroute].disttopo[jpoint], waypointstr);
    					else
    						sprintf(value_string, "%3d | %3d | %s | %s | %.2f | %.2f | %.2f | %s", iroute, jpoint, lonmstr0, latmstr0,
    						        shared.shareddata.routes[iroute].points[jpoint].zdata,
    						        shared.shareddata.routes[iroute].distlateral[jpoint],
    						        shared.shareddata.routes[iroute].disttopo[jpoint], waypointstr);
    					xstr[nitems] = XmStringCreateLocalized(value_string);
    					nitems++;
    				}
          }
  			}

  			/* add list items */
  			XmListAddItems(shared.mb3d_routelist.mbview_list_routelist, xstr, nitems, 0);

  			/* select list item for selected route */
  			if (shared.shareddata.route_selected != MBV_SELECT_NONE) {
          if (shared.shareddata.routes[iroute].active) {
    				/* get item number */
    				iitem = 0;
    				for (iroute = 0; iroute < shared.shareddata.nroute; iroute++) {
    					iitem++;
    					if (iroute == shared.shareddata.route_selected && shared.shareddata.route_point_selected == MBV_SELECT_ALL) {
    						XmListSelectPos(shared.mb3d_routelist.mbview_list_routelist, iitem, 0);
    						XmListSetPos(shared.mb3d_routelist.mbview_list_routelist, MAX(iitem - 5, 1));
    					}
    					for (jwaypoint = 0; jwaypoint < shared.shareddata.routes[iroute].npoints; jwaypoint++) {
    						iitem++;
    						if (iroute == shared.shareddata.route_selected && shared.shareddata.route_point_selected == jwaypoint) {
    							XmListSelectPos(shared.mb3d_routelist.mbview_list_routelist, iitem, 0);
    							XmListSetPos(shared.mb3d_routelist.mbview_list_routelist, MAX(iitem - 5, 1));
    						}
    					}
    				}
    			}
        }

  			/* deallocate memory no longer needed */
  			for (iitem = 0; iitem < nitems; iitem++) {
  				XmStringFree(xstr[iitem]);
  			}
  			free(xstr);
      }
		}
	}

	/* print output debug statements */
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	/* return */
	return (status);
}

/*------------------------------------------------------------------------------*/
