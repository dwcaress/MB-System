/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_route.c	9/25/2003
 *    $Id: mbview_route.c,v 5.6 2005-02-02 08:23:52 caress Exp $
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
 * Revision 5.5  2004/09/16 21:44:38  caress
 * Many changes over the summer.
 *
 * Revision 5.4  2004/07/27 19:50:28  caress
 * Improving route planning capability.
 *
 * Revision 5.3  2004/07/15 19:26:44  caress
 * Improvements to survey planning.
 *
 * Revision 5.2  2004/06/18 04:26:06  caress
 * June 17, 2004 update.
 *
 * Revision 5.1  2004/02/24 22:52:29  caress
 * Added spherical projection to MBview.
 *
 * Revision 5.0  2003/12/02 20:38:34  caress
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

static char rcs_id[]="$Id: mbview_route.c,v 5.6 2005-02-02 08:23:52 caress Exp $";

/*------------------------------------------------------------------------------*/
int mbview_getroutecount(int verbose, int instance,
			int *nroute,
			int *error)
{
	/* local variables */
	char	*function_name = "mbview_getroutecount";
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

	/* get number of routes */
	*nroute = shared.shareddata.nroute;
		
	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nroute:                    %d\n", *nroute);
		fprintf(stderr,"dbg2       error:                     %d\n", *error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n", status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_getroutepointcount(int verbose, int instance,
			int	route,
			int	*npoint,
			int	*nintpoint,
			int *error)
{
	/* local variables */
	char	*function_name = "mbview_getroutepointcount";
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
		fprintf(stderr,"dbg2       route:                     %d\n", route);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* get number of points in specified route */
	*npoint = 0;
	*nintpoint = 0;
	if (route >= 0 && route < shared.shareddata.nroute)
		{
		*npoint = shared.shareddata.routes[route].npoints;
		for (i=0;i<*npoint-1;i++)
			{
			if (shared.shareddata.routes[route].segments[i].nls > 2)
				*nintpoint += shared.shareddata.routes[route].segments[i].nls - 2;
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
int mbview_getrouteinfo(int verbose, int instance,
			int working_route, 
			int *nroutewaypoint, 
			int *nroutpoint, 
			char *routename, 
			int *routecolor, 
			int *routesize, 
			double *routedistancelateral, 
			double *routedistancetopo, 
			int *error)
{
	/* local variables */
	char	*function_name = "mbview_getrouteinfo";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	struct mbview_route_struct *route;
	double	distlateral, distovertopo;
	double	routelon0, routelon1;
	double	routelat0, routelat1;
	double	routetopo0, routetopo1;
	double	routeslope;
	int	i, j;

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
		fprintf(stderr,"dbg2       working_route:             %d\n", working_route);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* check that the route is valid */
	if (working_route < 0 && working_route >= shared.shareddata.nroute)
		{
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
	else
		{	
		/* get basic info */
		route = &(shared.shareddata.routes[working_route]);
		*nroutewaypoint = route->npoints;
		*nroutpoint = 0;
		strcpy(routename, route->name);
		*routecolor = route->color;
		*routesize = route->size;
		*routedistancelateral = 0.0;
		*routedistancetopo = 0.0;

		/* loop over the route segments */
		for (i=0;i<route->npoints-1;i++)
			{
			/* do first point */
			routelon1 = route->points[i].xlon;
			if (routelon1 < -180.0)
				routelon1 += 360.0;
			else if (routelon1 > 180.0)
				routelon1 -= 360.0;
			routelat1 = route->points[i].ylat ;
			routetopo1 = route->points[i].zdata;
			if (*nroutpoint == 0)
				{
				distlateral = 0.0;
				distovertopo = 0.0;
				}
			else
				{
				mbview_projectdistance(instance,
					routelon0, routelat0, routetopo0,
					routelon1, routelat1, routetopo1,
					&distlateral,
					&distovertopo,
					&routeslope);
				}
			(*routedistancelateral) += distlateral;
			(*routedistancetopo) += distovertopo;
			routelon0 = routelon1;
			routelat0 = routelat1;
			routetopo0 = routetopo1;
			(*nroutpoint)++;
			
			/* loop over interior of segment */
			for (j=1;j<route->segments[i].nls-1;j++)
				{
				routelon1 = route->segments[i].lspoints[j].xlon;
				if (routelon1 < -180.0)
					routelon1 += 360.0;
				else if (routelon1 > 180.0)
					routelon1 -= 360.0;
				routelat1 = route->segments[i].lspoints[j].ylat;
				routetopo1 = route->segments[i].lspoints[j].zdata;
				mbview_projectdistance(instance,
					routelon0, routelat0, routetopo0,
					routelon1, routelat1, routetopo1,
					&distlateral,
					&distovertopo,
					&routeslope);
				(*routedistancelateral) += distlateral;
				(*routedistancetopo) += distovertopo;
				routelon0 = routelon1;
				routelat0 = routelat1;
				routetopo0 = routetopo1;
				(*nroutpoint)++;
				}
			}
			
		/* do last point */
		j = route->npoints - 1;
		routelon1 = route->points[j].xlon;
		if (routelon1 < -180.0)
			routelon1 += 360.0;
		else if (routelon1 > 180.0)
			routelon1 -= 360.0;
		routelat1 = route->points[j].ylat ;
		routetopo1 = route->points[j].zdata;
		mbview_projectdistance(instance,
			routelon0, routelat0, routetopo0,
			routelon1, routelat1, routetopo1,
			&distlateral,
			&distovertopo,
			&routeslope);
		(*routedistancelateral) += distlateral;
		(*routedistancetopo) += distovertopo;
		routelon0 = routelon1;
		routelat0 = routelat1;
		routetopo0 = routetopo1;
		(*nroutpoint)++;
		}
		
	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nroutewaypoint:            %d\n", *nroutewaypoint);
		fprintf(stderr,"dbg2       nroutpoint:                %d\n", *nroutpoint);
		fprintf(stderr,"dbg2       routename:                 %d\n", *routename);
		fprintf(stderr,"dbg2       routecolor:                %d\n", *routecolor);
		fprintf(stderr,"dbg2       routesize:                 %d\n", *routesize);
		fprintf(stderr,"dbg2       routedistancelateral:      %f\n", *routedistancelateral);
		fprintf(stderr,"dbg2       routedistancetopo:         %f\n", *routedistancetopo);
		fprintf(stderr,"dbg2       error:                     %d\n", *error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n", status);
		}

	/* return */
	return(status);

}

/*------------------------------------------------------------------------------*/
int mbview_allocroutearrays(int verbose, 
			int	npointtotal,
			double	**routelon,
			double	**routelat,
			int	**waypoint,
			double	**routetopo,
			double	**routebearing,
			double	**distlateral,
			double	**distovertopo,
			double	**slope,
			int *error)
{
	/* local variables */
	char	*function_name = "mbview_allocroutearrays";
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
		fprintf(stderr,"dbg2       routelon:                  %d\n", *routelon);
		fprintf(stderr,"dbg2       routelat:                  %d\n", *routelat);
		if (waypoint != NULL)
		fprintf(stderr,"dbg2       waypoint:                  %d\n", *waypoint);
		if (routetopo != NULL)
		fprintf(stderr,"dbg2       routetopo:                 %d\n", *routetopo);
		if (routebearing != NULL)
		fprintf(stderr,"dbg2       routebearing:              %d\n", *routebearing);
		if (distlateral != NULL)
		fprintf(stderr,"dbg2       distlateral:               %d\n", *distlateral);
		if (distovertopo != NULL)
		fprintf(stderr,"dbg2       distovertopo:              %d\n", *distovertopo);
		if (slope != NULL)
		fprintf(stderr,"dbg2       slope:                     %d\n", *slope);
		}

	/* allocate the arrays using mb_realloc */
	status = mb_realloc(verbose,npointtotal*sizeof(double),routelon,error);
	if (status == MB_SUCCESS)
		status = mb_realloc(verbose,npointtotal*sizeof(double),routelat,error);
	if (status == MB_SUCCESS && waypoint != NULL)
		status = mb_realloc(verbose,npointtotal*sizeof(int),waypoint,error);
	if (status == MB_SUCCESS && routetopo != NULL)
		status = mb_realloc(verbose,npointtotal*sizeof(double),routetopo,error);
	if (status == MB_SUCCESS && routebearing != NULL)
		status = mb_realloc(verbose,npointtotal*sizeof(double),routebearing,error);
	if (status == MB_SUCCESS && distlateral != NULL)
		status = mb_realloc(verbose,npointtotal*sizeof(double),distlateral,error);
	if (status == MB_SUCCESS && distovertopo != NULL)
		status = mb_realloc(verbose,npointtotal*sizeof(double),distovertopo,error);
	if (status == MB_SUCCESS && slope != NULL)
		status = mb_realloc(verbose,npointtotal*sizeof(double),slope,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       routelon:                  %d\n", *routelon);
		fprintf(stderr,"dbg2       routelat:                  %d\n", *routelat);
		if (waypoint != NULL)
		fprintf(stderr,"dbg2       waypoint:                  %d\n", *waypoint);
		if (routetopo != NULL)
		fprintf(stderr,"dbg2       routetopo:                 %d\n", *routetopo);
		if (routebearing != NULL)
		fprintf(stderr,"dbg2       routebearing:              %d\n", *routebearing);
		if (distlateral != NULL)
		fprintf(stderr,"dbg2       distlateral:               %d\n", *distlateral);
		if (distovertopo != NULL)
		fprintf(stderr,"dbg2       distovertopo:              %d\n", *distovertopo);
		if (slope != NULL)
		fprintf(stderr,"dbg2       slope:                     %d\n", *slope);
		fprintf(stderr,"dbg2       error:                     %d\n", *error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n", status);
		}

	/* return */
	return(status);
}


/*------------------------------------------------------------------------------*/
int mbview_freeroutearrays(int verbose,
			double	**routelon,
			double	**routelat,
			int	**waypoint,
			double	**routetopo,
			double	**routebearing,
			double	**distlateral,
			double	**distovertopo,
			double	**slope,
			int *error)
{
	/* local variables */
	char	*function_name = "mbview_freeroutearrays";
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
		fprintf(stderr,"dbg2       routelon:                  %d\n", *routelon);
		fprintf(stderr,"dbg2       routelat:                  %d\n", *routelat);
		if (waypoint != NULL)
		fprintf(stderr,"dbg2       waypoint:                  %d\n", *waypoint);
		if (routetopo != NULL)
		fprintf(stderr,"dbg2       routetopo:                 %d\n", *routetopo);
		if (routebearing != NULL)
		fprintf(stderr,"dbg2       routebearing:              %d\n", *routebearing);
		if (distlateral != NULL)
		fprintf(stderr,"dbg2       distlateral:               %d\n", *distlateral);
		if (distovertopo != NULL)
		fprintf(stderr,"dbg2       distovertopo:              %d\n", *distovertopo);
		if (slope != NULL)
		fprintf(stderr,"dbg2       slope:                     %d\n", *slope);
		}

	/* free the arrays using mb_free */
	status = mb_free(verbose,routelon,error);
	status = mb_free(verbose,routelat,error);
	if (waypoint != NULL)
		status = mb_free(verbose,waypoint,error);
	if (routetopo != NULL)
		status = mb_free(verbose,routetopo,error);
	if (routebearing != NULL)
		status = mb_free(verbose,routebearing,error);
	if (distlateral != NULL)
		status = mb_free(verbose,distlateral,error);
	if (distovertopo != NULL)
		status = mb_free(verbose,distovertopo,error);
	if (slope != NULL)
		status = mb_free(verbose,slope,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       routelon:                  %d\n", *routelon);
		fprintf(stderr,"dbg2       routelat:                  %d\n", *routelat);
		if (waypoint != NULL)
		fprintf(stderr,"dbg2       waypoint:                  %d\n", *waypoint);
		if (routetopo != NULL)
		fprintf(stderr,"dbg2       routetopo:                 %d\n", *routetopo);
		if (routebearing != NULL)
		fprintf(stderr,"dbg2       routebearing:              %d\n", *routebearing);
		if (distlateral != NULL)
		fprintf(stderr,"dbg2       distlateral:               %d\n", *distlateral);
		if (distovertopo != NULL)
		fprintf(stderr,"dbg2       distovertopo:              %d\n", *distovertopo);
		if (slope != NULL)
		fprintf(stderr,"dbg2       slope:                     %d\n", *slope);
		fprintf(stderr,"dbg2       error:                     %d\n", *error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n", status);
		}

	/* return */
	return(status);
}


/*------------------------------------------------------------------------------*/
int mbview_addroute(int verbose, int instance,
			int	npoint,
			double	*routelon,
			double	*routelat,
			int	*waypoint,
			int	routecolor,
			int	routesize,
			mb_path	routename,
			int	*iroute,
			int *error)
{
	/* local variables */
	char	*function_name = "mbview_addroute";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	double	xgrid, ygrid, zdata;
	double	xdisplay, ydisplay, zdisplay;
	int	nfound;
	int	nadded;
	int	i, ii, jj, iii, jjj, kkk;

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
		fprintf(stderr,"dbg2       routelon:                  %d\n", routelon);
		fprintf(stderr,"dbg2       routelat:                  %d\n", routelat);
		fprintf(stderr,"dbg2       waypoint:                  %d\n", waypoint);
		for (i=0;i<npoint;i++)
			{
			fprintf(stderr,"dbg2       point:%d lon:%f lat:%f waypoint:%d\n", 
					i, routelon[i], routelat[i], waypoint[i]);
			}
		fprintf(stderr,"dbg2       routecolor:                %d\n", routecolor);
		fprintf(stderr,"dbg2       routesize:                 %d\n", routesize);
		fprintf(stderr,"dbg2       routename:                 %s\n", routename);
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
	for (i=0;i<npoint;i++)
		{
		/* check waypoint flag correct */
		if (waypoint[i] <= MBV_ROUTE_WAYPOINT_NONE
			|| waypoint[i] > MBV_ROUTE_WAYPOINT_ENDLINE)
			waypoint[i] = MBV_ROUTE_WAYPOINT_SIMPLE;
		
		/* get route positions in grid coordinates */
		status = mbview_projectll2xyzgrid(instance,
				routelon[i], routelat[i], 
				&xgrid, &ygrid, &zdata);

		/* get route positions in display coordinates */
		status = mbview_projectll2display(instance,
				routelon[i], routelat[i], zdata,
				&xdisplay, &ydisplay, &zdisplay);
			
		/* add the route point */
		mbview_route_add(instance, *iroute, i, waypoint[i],
			xgrid, ygrid,
			routelon[i], routelat[i], zdata,
			xdisplay, ydisplay, zdisplay);
		}

	/* set color size and name for new route */
	shared.shareddata.routes[*iroute].color = routecolor;
	shared.shareddata.routes[*iroute].size = routesize;
	strcpy(shared.shareddata.routes[*iroute].name,routename);
	
	/* make routes viewable */
	if (data->route_view_mode != MBV_VIEW_ON)
		{
		data->route_view_mode = MBV_VIEW_ON;
		set_mbview_route_view_mode(instance, MBV_VIEW_ON);
		}
	
	/* update route list */
	mbview_updateroutelist();
		
	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       iroute:                    %d\n",*iroute);
		fprintf(stderr,"dbg2       error:                     %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n",status);
		}

	/* return */
	return(status);
}


/*------------------------------------------------------------------------------*/
int mbview_deleteroute(int verbose, int instance,
			int iroute,
			int *error)
{
	/* local variables */
	char	*function_name = "mbview_deleteroute";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	jpoint;

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
		fprintf(stderr,"dbg2       iroute:                    %d\n", iroute);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* delete the points in the route backwards */
	for (jpoint=shared.shareddata.routes[iroute].npoints-1;jpoint>=0;jpoint--)
		{
		/* delete the route point */
		mbview_route_delete(instance, iroute, jpoint);
		}
		
	/* set pick annotation */
	mbview_pick_text(instance);
	
	/* update route list */
	mbview_updateroutelist();
		
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
int mbview_getroute(int verbose, int instance,
			int	route,
			int	*npointtotal,
			double	*routelon,
			double	*routelat,
			int	*waypoint,
			double	*routetopo,
			double	*routebearing,
			double	*distlateral,
			double	*distovertopo,
			double	*slope,
			int	*routecolor,
			int	*routesize,
			mb_path	routename,
			int *error)
{
	/* local variables */
	char	*function_name = "mbview_getroute";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	double	dx, dy, range, bearing;
	double	xx0, yy0, zz0, xx1, yy1, zz1, xx2, yy2, zz2;
	double	dxx, dyy, dzz;
	double	dll, doo;
	int	i, j;

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
		fprintf(stderr,"dbg2       route:                     %d\n", route);
		fprintf(stderr,"dbg2       npointtotal:               %d\n", npointtotal);
		fprintf(stderr,"dbg2       routelon:                  %d\n", routelon);
		fprintf(stderr,"dbg2       routelat:                  %d\n", routelat);
		fprintf(stderr,"dbg2       waypoint:                  %d\n", waypoint);
		fprintf(stderr,"dbg2       routetopo:                 %d\n", routetopo);
		fprintf(stderr,"dbg2       routebearing:              %d\n", routebearing);
		fprintf(stderr,"dbg2       distlateral:               %d\n", distlateral);
		fprintf(stderr,"dbg2       distovertopo:              %d\n", distovertopo);
		fprintf(stderr,"dbg2       slope:                     %d\n", slope);
		fprintf(stderr,"dbg2       routecolor:                %d\n", routecolor);
		fprintf(stderr,"dbg2       routesize:                 %d\n", routesize);
		fprintf(stderr,"dbg2       routename:                 %d\n", routename);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* zero the points returned */
	*npointtotal = 0;
	
	/* check that the array pointers are not NULL */
	if (routelon == NULL || routelat == NULL || waypoint == NULL || routetopo == NULL 
		|| distlateral == NULL || distovertopo == NULL || slope == NULL)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_DATA_NOT_INSERTED;
		}
	
	/* otherwise go get the route data */
	else
		{	
		/* loop over the route segments */
		for (i=0;i<shared.shareddata.routes[route].npoints-1;i++)
			{
			/* get bearing of segment */
			if (data->display_projection_mode != MBV_PROJECTION_SPHEROID)
				{
				dx = shared.shareddata.routes[route].points[i+1].xdisplay
						- shared.shareddata.routes[route].points[i].xdisplay;
				dy = shared.shareddata.routes[route].points[i+1].ydisplay
						- shared.shareddata.routes[route].points[i].ydisplay;
				range = sqrt(dx * dx + dy * dy) / view->scale ;
				bearing = RTD * atan2(dx, dy);
				}
			else
				{
				mbview_greatcircle_distbearing(instance, 
					shared.shareddata.routes[route].points[i].xlon, 
					shared.shareddata.routes[route].points[i].ylat, 
					shared.shareddata.routes[route].points[i+1].xlon, 
					shared.shareddata.routes[route].points[i+1].ylat, 
					&bearing, &range);
				}
			if (bearing < 0.0)
				bearing += 360.0;

			/* add first point */
			routelon[*npointtotal] = shared.shareddata.routes[route].points[i].xlon;
			if (routelon[*npointtotal] < -180.0)
				routelon[*npointtotal] += 360.0;
			else if (routelon[*npointtotal] > 180.0)
				routelon[*npointtotal] -= 360.0;
			routelat[*npointtotal] = shared.shareddata.routes[route].points[i].ylat ;
			waypoint[*npointtotal] = shared.shareddata.routes[route].waypoint[i];
			routetopo[*npointtotal] = shared.shareddata.routes[route].points[i].zdata;
			routebearing[*npointtotal] = bearing;
			if (*npointtotal == 0)
				{
				distlateral[*npointtotal] = 0.0;
				distovertopo[*npointtotal] = 0.0;
				slope[*npointtotal] = 0.0;
				}
			else
				{
				mbview_projectdistance(instance,
					routelon[*npointtotal-1], routelat[*npointtotal-1], routetopo[*npointtotal-1],
					routelon[*npointtotal], routelat[*npointtotal], routetopo[*npointtotal],
					&distlateral[*npointtotal],
					&distovertopo[*npointtotal],
					&slope[*npointtotal]);
				distlateral[*npointtotal] += distlateral[*npointtotal-1];
				distovertopo[*npointtotal] += distovertopo[*npointtotal-1];
				}
			(*npointtotal)++;
			
			/* loop over interior of segment */
			for (j=1;j<shared.shareddata.routes[route].segments[i].nls-1;j++)
				{
				routelon[*npointtotal] = shared.shareddata.routes[route].segments[i].lspoints[j].xlon;
				if (routelon[*npointtotal] < -180.0)
					routelon[*npointtotal] += 360.0;
				else if (routelon[*npointtotal] > 180.0)
					routelon[*npointtotal] -= 360.0;
				routelat[*npointtotal] = shared.shareddata.routes[route].segments[i].lspoints[j].ylat;
				waypoint[*npointtotal] = MBV_ROUTE_WAYPOINT_NONE;
				routetopo[*npointtotal] = shared.shareddata.routes[route].segments[i].lspoints[j].zdata;
				routebearing[*npointtotal] = bearing;
				mbview_projectdistance(instance,
					routelon[*npointtotal-1], routelat[*npointtotal-1], routetopo[*npointtotal-1],
					routelon[*npointtotal], routelat[*npointtotal], routetopo[*npointtotal],
					&distlateral[*npointtotal],
					&distovertopo[*npointtotal],
					&slope[*npointtotal]);
				distlateral[*npointtotal] += distlateral[*npointtotal-1];
				distovertopo[*npointtotal] += distovertopo[*npointtotal-1];
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
		routelat[*npointtotal] = shared.shareddata.routes[route].points[j].ylat ;
		waypoint[*npointtotal] = shared.shareddata.routes[route].waypoint[j];;
		routetopo[*npointtotal] = shared.shareddata.routes[route].points[j].zdata;
		routebearing[*npointtotal] = bearing;
		mbview_projectdistance(instance,
			routelon[*npointtotal-1], routelat[*npointtotal-1], routetopo[*npointtotal-1],
			routelon[*npointtotal], routelat[*npointtotal], routetopo[*npointtotal],
			&distlateral[*npointtotal],
			&distovertopo[*npointtotal],
			&slope[*npointtotal]);
		distlateral[*npointtotal] += distlateral[*npointtotal-1];
		distovertopo[*npointtotal] += distovertopo[*npointtotal-1];
		(*npointtotal)++;
		
		/* get color size and name */
		*routecolor = shared.shareddata.routes[route].color;
		*routesize = shared.shareddata.routes[route].size;
		strcpy(routename, shared.shareddata.routes[route].name);
		
		/* recalculate slope */
		for (j=0;j<*npointtotal;j++)
			{
			if (j == 0 && *npointtotal == 1)
				{
				slope[j] = 0.0;
				}
			else if (j == 0)
				{
				if (distlateral[j+1] > 0.0)
					slope[j] = (routetopo[j+1] - routetopo[j]) / distlateral[j+1];
				else
					slope[j] = 0.0;
				}
			else if (j == *npointtotal - 1)
				{
				if ((distlateral[j] - distlateral[j-1]) > 0.0)
					slope[j] = (routetopo[j] - routetopo[j-1]) 
							/ (distlateral[j] - distlateral[j-1]);
				else
					slope[j] = 0.0;
				}
			else
				{
				if ((distlateral[j+1] - distlateral[j-1]) > 0.0)
					slope[j] = (routetopo[j+1] - routetopo[j-1]) 
							/ (distlateral[j+1] - distlateral[j-1]);
				else
					slope[j] = 0.0;
				}
			}
		}
		
	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       npointtotal:               %d\n", *npointtotal);
		for (i=0;i<*npointtotal;i++)
			{
			fprintf(stderr,"dbg2       route:%d lon:%f lat:%f waypoint:%d topo:%f bearing:%f dist:%f distbot:%f color:%d size:%d name:%s\n", 
					i, routelon[i], routelat[i], waypoint[i], routetopo[i], 
					routebearing[i], distlateral[i], distovertopo[i], 
					*routecolor, *routesize, routename);
			}
		fprintf(stderr,"dbg2       error:                     %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:                    %d\n",status);
		}

	/* return */
	return(status);

}

/*------------------------------------------------------------------------------*/
int mbview_enableviewroutes(int verbose, int instance,
			int *error)

{
	/* local variables */
	char	*function_name = "mbview_enableviewroutes";
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
        shared.shareddata.route_mode = MBV_ROUTE_VIEW;
		
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
int mbview_enableeditroutes(int verbose, int instance,
			int *error)

{
	/* local variables */
	char	*function_name = "mbview_enableeditroutes";
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
        shared.shareddata.route_mode = MBV_ROUTE_EDIT;
		
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
int mbview_pick_route_select(int instance, int which, int xpixel, int ypixel)
{

	/* local variables */
	char	*function_name = "mbview_pick_route_select";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	found;
	double	xgrid, ygrid;
	double	xlon, ylat, zdata;
	double	xdisplay, ydisplay, zdisplay;
	double	xx, yy, rr, rrmin;
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
		fprintf(stderr,"dbg2       which:            %d\n",which);
		fprintf(stderr,"dbg2       xpixel:           %d\n",xpixel);
		fprintf(stderr,"dbg2       ypixel:           %d\n",ypixel);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* only select route points if enabled and not in move mode */
	if (shared.shareddata.route_mode != MBV_ROUTE_OFF
		&& shared.shareddata.nroute > 0
		&& (which == MBV_PICK_DOWN
			|| shared.shareddata.route_selected == MBV_SELECT_NONE))
		{
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, 
				&found, 
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);

		/* look for nearest route point */
		if (found)
			{
			rrmin = 1000000000.0;
			shared.shareddata.route_selected = MBV_SELECT_NONE;
			shared.shareddata.route_point_selected = MBV_SELECT_NONE;
			for (i=0;i<shared.shareddata.nroute;i++)
			for (j=0;j<shared.shareddata.routes[i].npoints;j++)
				{
				xx = xgrid - shared.shareddata.routes[i].points[j].xgrid[instance];
				yy = ygrid - shared.shareddata.routes[i].points[j].ygrid[instance];
				rr = sqrt(xx * xx + yy * yy);
				if (rr < rrmin)
					{
					rrmin = rr;
					shared.shareddata.route_selected = i;
					shared.shareddata.route_point_selected = j;
					}
				}
			}
		else
			{
			shared.shareddata.route_selected = MBV_SELECT_NONE;
			XBell(view->dpy,100);
			}
		}
	
	/* only move selected route points if enabled */
	else if (shared.shareddata.route_mode != MBV_ROUTE_OFF
		&& shared.shareddata.nroute > 0
		&& (which == MBV_PICK_MOVE
			&& shared.shareddata.route_selected != MBV_SELECT_NONE))
		{
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, 
				&found, 
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);
				
		/* reset selected route position */
		if (found)
			{
			shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].xgrid[instance] = xgrid;
			shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].ygrid[instance] = ygrid;
			shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].xlon = xlon;
			shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].ylat = ylat;
			shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].zdata = zdata;
			shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].xdisplay[instance] = xdisplay;
			shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].ydisplay[instance] = ydisplay;
			shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].zdisplay[instance] = zdisplay;
			mbview_updatepointw(instance, &(shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected]));
			
			/* drape the affected segments */
			if (shared.shareddata.route_point_selected > 0)
				{
				/* drape the segment */
				mbview_drapesegmentw(instance, &(shared.shareddata.routes[shared.shareddata.route_selected].segments[shared.shareddata.route_point_selected-1]));
			
				/* update the segment for all active instances */
				mbview_updatesegmentw(instance, &(shared.shareddata.routes[shared.shareddata.route_selected].segments[shared.shareddata.route_point_selected-1]));
				}
			if (shared.shareddata.route_point_selected < shared.shareddata.routes[shared.shareddata.route_selected].npoints - 1)
				{
				/* drape the segment */
				mbview_drapesegmentw(instance, &(shared.shareddata.routes[shared.shareddata.route_selected].segments[shared.shareddata.route_point_selected]));
			
				/* update the segment for all active instances */
				mbview_updatesegmentw(instance, &(shared.shareddata.routes[shared.shareddata.route_selected].segments[shared.shareddata.route_point_selected]));
				}
			}
		}

	/* else beep */
	else
		{
		shared.shareddata.route_selected = MBV_SELECT_NONE;
		XBell(view->dpy,100);
		}
		
	/* set what kind of pick to annotate */
	if (shared.shareddata.route_selected != MBV_SELECT_NONE)
		{
		data->pickinfo_mode = MBV_PICK_ROUTE;
		}
	else
		{
		data->pickinfo_mode = data->pick_type;
		}
		
	/* set pick annotation */
	mbview_pick_text(instance);
	
	/* update route list */
	mbview_updateroutelist();
	
	/* print route debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Route data altered in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Route values:\n");
		fprintf(stderr,"dbg2       route_view_mode:      %d\n",data->route_view_mode);
		fprintf(stderr,"dbg2       route_mode:           %d\n",shared.shareddata.route_mode);
		fprintf(stderr,"dbg2       nroute:               %d\n",shared.shareddata.nroute);
		fprintf(stderr,"dbg2       nroute_alloc:         %d\n",shared.shareddata.nroute_alloc);
		fprintf(stderr,"dbg2       route_selected:       %d\n",shared.shareddata.route_selected);
		fprintf(stderr,"dbg2       route_point_selected: %d\n",shared.shareddata.route_point_selected);
		for (i=0;i<shared.shareddata.nroute;i++)
			{
			fprintf(stderr,"dbg2       route %d color:         %d\n",i,shared.shareddata.routes[i].color);
			fprintf(stderr,"dbg2       route %d size:          %d\n",i,shared.shareddata.routes[i].size);
			fprintf(stderr,"dbg2       route %d name:          %s\n",i,shared.shareddata.routes[i].name);
			fprintf(stderr,"dbg2       route %d npoints:       %d\n",i,shared.shareddata.routes[i].npoints);
			fprintf(stderr,"dbg2       route %d npoints_alloc: %d\n",i,shared.shareddata.routes[i].npoints_alloc);
			for (j=0;j<shared.shareddata.routes[i].npoints;j++)
				{
				fprintf(stderr,"dbg2       route %d %d waypoint: %d\n",i,j,shared.shareddata.routes[i].waypoint[j]);
				fprintf(stderr,"dbg2       route %d %d xgrid:    %f\n",i,j,shared.shareddata.routes[i].points[j].xgrid);
				fprintf(stderr,"dbg2       route %d %d ygrid:    %f\n",i,j,shared.shareddata.routes[i].points[j].ygrid);
				fprintf(stderr,"dbg2       route %d %d xlon:     %f\n",i,j,shared.shareddata.routes[i].points[j].xlon);
				fprintf(stderr,"dbg2       route %d %d ylat:     %f\n",i,j,shared.shareddata.routes[i].points[j].ylat);
				fprintf(stderr,"dbg2       route %d %d zdata:    %f\n",i,j,shared.shareddata.routes[i].points[j].zdata);
				fprintf(stderr,"dbg2       route %d %d xdisplay: %f\n",i,j,shared.shareddata.routes[i].points[j].xdisplay);
				fprintf(stderr,"dbg2       route %d %d ydisplay: %f\n",i,j,shared.shareddata.routes[i].points[j].ydisplay);
				fprintf(stderr,"dbg2       route %d %d zdisplay: %f\n",i,j,shared.shareddata.routes[i].points[j].zdisplay);
				}
			for (j=0;j<shared.shareddata.routes[i].npoints-1;j++)
				{
				fprintf(stderr,"dbg2       route %d %d nls:          %d\n",i,j,shared.shareddata.routes[i].segments[j].nls);
				fprintf(stderr,"dbg2       route %d %d nls_alloc:    %d\n",i,j,shared.shareddata.routes[i].segments[j].nls_alloc);
				fprintf(stderr,"dbg2       route %d %d endpoints[0]: %d\n",i,j,shared.shareddata.routes[i].segments[j].endpoints[0]);
				fprintf(stderr,"dbg2       route %d %d endpoints[1]: %d\n",i,j,shared.shareddata.routes[i].segments[j].endpoints[1]);
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
int mbview_pick_route_add(int instance, int which, int xpixel, int ypixel)
{

	/* local variables */
	char	*function_name = "mbview_pick_route_add";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	found;
	double	xgrid, ygrid;
	double	xlon, ylat, zdata;
	double	xdisplay, ydisplay, zdisplay;
	double	xx, yy, rr, rrmin;
	int	i, j, inew, jnew;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       which:            %d\n",which);
		fprintf(stderr,"dbg2       xpixel:           %d\n",xpixel);
		fprintf(stderr,"dbg2       ypixel:           %d\n",ypixel);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* only add route points if enabled and not in move mode */
	if (shared.shareddata.route_mode == MBV_ROUTE_EDIT
		&& (which == MBV_PICK_DOWN 
			|| (which == MBV_PICK_MOVE
				&& shared.shareddata.route_selected == MBV_SELECT_NONE)))
		{
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, 
				&found, 
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);

		/* add route if necessary */
		if (found && shared.shareddata.route_selected == MBV_SELECT_NONE)
			{
			/* add route point after currently selected route point if any */
			inew = shared.shareddata.nroute;
			jnew = 0;
			
			/* add the route point */
			mbview_route_add(instance, inew, jnew, MBV_ROUTE_WAYPOINT_SIMPLE,
				xgrid, ygrid,
				xlon, ylat, zdata,
				xdisplay, ydisplay, zdisplay);
			
			/* select the new route */
			shared.shareddata.route_selected = inew;
			shared.shareddata.route_point_selected = jnew;
			}

		/* else just add point to currently selected route and point */
		else if (found && shared.shareddata.route_selected != MBV_SELECT_NONE)
			{
			/* add route point after currently selected route point if any */
			inew = shared.shareddata.route_selected;
			jnew = shared.shareddata.route_point_selected + 1;
			
			/* add the route point */
			mbview_route_add(instance, inew, jnew, MBV_ROUTE_WAYPOINT_SIMPLE,
				xgrid, ygrid,
				xlon, ylat, zdata,
				xdisplay, ydisplay, zdisplay);
			
			
			/* select the new route */
			shared.shareddata.route_selected = inew;
			shared.shareddata.route_point_selected = jnew;
			}

		else
			{
			/* deselect the new route */
			shared.shareddata.route_selected = MBV_SELECT_NONE;
			shared.shareddata.route_point_selected = MBV_SELECT_NONE;
			XBell(view->dpy,100);
			}
		}
	
	/* only move selected routes if enabled */
	else if (shared.shareddata.route_mode != MBV_ROUTE_OFF
		&& shared.shareddata.nroute > 0
		&& (which == MBV_PICK_MOVE
			&& shared.shareddata.route_selected != MBV_SELECT_NONE))
		{
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, 
				&found, 
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);
				
		/* reset selected route position */
		if (found)
			{
			/* move the point */
			shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].xgrid[instance] = xgrid;
			shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].ygrid[instance] = ygrid;
			shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].xlon = xlon;
			shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].ylat = ylat;
			shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].zdata = zdata;
			shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].xdisplay[instance] = xdisplay;
			shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].ydisplay[instance] = ydisplay;
			shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].zdisplay[instance] = zdisplay;
			mbview_updatepointw(instance, &(shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected]));
			
			/* drape the affected segments */
			if (shared.shareddata.route_point_selected > 0)
				{
				/* drape the segment */
				mbview_drapesegmentw(instance, 
					&(shared.shareddata.routes[shared.shareddata.route_selected].segments[shared.shareddata.route_point_selected-1]));
			
				/* update the segment for all active instances */
				mbview_updatesegmentw(instance, &(shared.shareddata.routes[shared.shareddata.route_selected].segments[shared.shareddata.route_point_selected-1]));
				}
			if (shared.shareddata.route_point_selected < shared.shareddata.routes[shared.shareddata.route_selected].npoints - 1)
				{
				/* drape the segment */
				mbview_drapesegmentw(instance, 
					&(shared.shareddata.routes[shared.shareddata.route_selected].segments[shared.shareddata.route_point_selected]));
			
				/* update the segment for all active instances */
				mbview_updatesegmentw(instance, &(shared.shareddata.routes[shared.shareddata.route_selected].segments[shared.shareddata.route_point_selected]));
				}
			}
		}

	/* else beep */
	else
		{
		shared.shareddata.route_selected = MBV_SELECT_NONE;
		shared.shareddata.route_point_selected = MBV_SELECT_NONE;
		XBell(view->dpy,100);
		}
		
	/* set what kind of pick to annotate */
	if (shared.shareddata.route_selected != MBV_SELECT_NONE)
		{
		data->pickinfo_mode = MBV_PICK_ROUTE;
		}
	else
		{
		data->pickinfo_mode = data->pick_type;
		}
		
	/* set pick annotation */
	mbview_pick_text(instance);
	
	/* update route list */
	mbview_updateroutelist();
	
	/* print route debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Route data altered in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Route values:\n");
		fprintf(stderr,"dbg2       route_view_mode:      %d\n",data->route_view_mode);
		fprintf(stderr,"dbg2       route_mode:           %d\n",shared.shareddata.route_mode);
		fprintf(stderr,"dbg2       nroute:               %d\n",shared.shareddata.nroute);
		fprintf(stderr,"dbg2       nroute_alloc:         %d\n",shared.shareddata.nroute_alloc);
		fprintf(stderr,"dbg2       route_selected:       %d\n",shared.shareddata.route_selected);
		fprintf(stderr,"dbg2       route_point_selected: %d\n",shared.shareddata.route_point_selected);
		for (i=0;i<shared.shareddata.nroute;i++)
			{
			fprintf(stderr,"dbg2       route %d color:         %d\n",i,shared.shareddata.routes[i].color);
			fprintf(stderr,"dbg2       route %d size:          %d\n",i,shared.shareddata.routes[i].size);
			fprintf(stderr,"dbg2       route %d name:          %s\n",i,shared.shareddata.routes[i].name);
			fprintf(stderr,"dbg2       route %d npoints:       %d\n",i,shared.shareddata.routes[i].npoints);
			fprintf(stderr,"dbg2       route %d npoints_alloc: %d\n",i,shared.shareddata.routes[i].npoints_alloc);
			for (j=0;j<shared.shareddata.routes[i].npoints;j++)
				{
				fprintf(stderr,"dbg2       route %d %d xgrid:    %f\n",i,j,shared.shareddata.routes[i].points[j].xgrid[instance]);
				fprintf(stderr,"dbg2       route %d %d ygrid:    %f\n",i,j,shared.shareddata.routes[i].points[j].ygrid[instance]);
				fprintf(stderr,"dbg2       route %d %d xlon:     %f\n",i,j,shared.shareddata.routes[i].points[j].xlon);
				fprintf(stderr,"dbg2       route %d %d ylat:     %f\n",i,j,shared.shareddata.routes[i].points[j].ylat);
				fprintf(stderr,"dbg2       route %d %d zdata:    %f\n",i,j,shared.shareddata.routes[i].points[j].zdata);
				fprintf(stderr,"dbg2       route %d %d xdisplay: %f\n",i,j,shared.shareddata.routes[i].points[j].xdisplay[instance]);
				fprintf(stderr,"dbg2       route %d %d ydisplay: %f\n",i,j,shared.shareddata.routes[i].points[j].ydisplay[instance]);
				fprintf(stderr,"dbg2       route %d %d zdisplay: %f\n",i,j,shared.shareddata.routes[i].points[j].zdisplay[instance]);
				}
			for (j=0;j<shared.shareddata.routes[i].npoints-1;j++)
				{
				fprintf(stderr,"dbg2       route %d %d nls:          %d\n",i,j,shared.shareddata.routes[i].segments[j].nls);
				fprintf(stderr,"dbg2       route %d %d nls_alloc:    %d\n",i,j,shared.shareddata.routes[i].segments[j].nls_alloc);
				fprintf(stderr,"dbg2       route %d %d endpoints[0]: %d\n",i,j,shared.shareddata.routes[i].segments[j].endpoints[0]);
				fprintf(stderr,"dbg2       route %d %d endpoints[1]: %d\n",i,j,shared.shareddata.routes[i].segments[j].endpoints[1]);
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
int mbview_pick_route_delete(int instance, int xpixel, int ypixel)
{

	/* local variables */
	char	*function_name = "mbview_pick_route_delete";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	found;
	double	xgrid, ygrid;
	double	xlon, ylat, zdata;
	double	xdisplay, ydisplay, zdisplay;
	double	xx, yy, rr, rrmin;
	int	i, j, idelete, jdelete;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       instance:         %d\n",instance);
		fprintf(stderr,"dbg2       xpixel:           %d\n",xpixel);
		fprintf(stderr,"dbg2       ypixel:           %d\n",ypixel);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* only delete a selected route if enabled */
	if (shared.shareddata.route_mode == MBV_ROUTE_EDIT
		&& shared.shareddata.route_selected != MBV_SELECT_NONE)
		{
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, 
				&found, 
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);

		/* find closest route to pick point */
		if (found)
			{
			rrmin = 1000000000.0;
			idelete = MBV_SELECT_NONE;
			jdelete = MBV_SELECT_NONE;
			for (i=0;i<shared.shareddata.nroute;i++)
				{
				for (j=0;j<shared.shareddata.routes[i].npoints;j++)
					{
					xx = xgrid - shared.shareddata.routes[i].points[j].xgrid[instance];
					yy = ygrid - shared.shareddata.routes[i].points[j].ygrid[instance];
					rr = sqrt(xx * xx + yy * yy);
					if (rr < rrmin)
						{
						rrmin = rr;
						idelete = i;
						jdelete = j;
						}
					}
				}
			}

		/* delete route point if its the same as previously selected */
		if (found && shared.shareddata.route_selected == idelete
			&& shared.shareddata.route_point_selected == jdelete)
			{
			mbview_route_delete(instance, idelete, jdelete);
			}

		/* else beep */
		else
			{
			XBell(view->dpy,100);
			}
		}

	/* else beep */
	else
		{
		XBell(view->dpy,100);
		}
		
	/* set pick annotation */
	mbview_pick_text(instance);
	
	/* update route list */
	mbview_updateroutelist();
	
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
int mbview_route_add(int instance, int inew, int jnew, int waypoint,
				double xgrid, double ygrid,
				double xlon, double ylat, double zdata,
				double xdisplay, double ydisplay, double zdisplay)
{

	/* local variables */
	char	*function_name = "mbview_route_add";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
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
		fprintf(stderr,"dbg2       inew:             %d\n",inew);
		fprintf(stderr,"dbg2       jnew:             %d\n",jnew);
		fprintf(stderr,"dbg2       waypoint:         %d\n",waypoint);
		fprintf(stderr,"dbg2       xgrid:            %f\n",xgrid);
		fprintf(stderr,"dbg2       ygrid:            %f\n",ygrid);
		fprintf(stderr,"dbg2       xlon:             %f\n",xlon);
		fprintf(stderr,"dbg2       ylat:             %f\n",ylat);
		fprintf(stderr,"dbg2       zdata:            %f\n",zdata);
		fprintf(stderr,"dbg2       xdisplay:         %f\n",xdisplay);
		fprintf(stderr,"dbg2       ydisplay:         %f\n",ydisplay);
		fprintf(stderr,"dbg2       zdisplay:         %f\n",zdisplay);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* add route if required */
	if (inew == shared.shareddata.nroute)
		{
		/* allocate memory for a new route if required */
		if (shared.shareddata.nroute_alloc < shared.shareddata.nroute + 1)
			{
			shared.shareddata.nroute_alloc += MBV_ALLOC_NUM;
			status = mb_realloc(mbv_verbose, 
			    		shared.shareddata.nroute_alloc * sizeof(struct mbview_route_struct),
			    		&(shared.shareddata.routes), &error);
			if (status == MB_FAILURE)
				{
				shared.shareddata.nroute_alloc = 0;
				}
			else
				{
				for (i=shared.shareddata.nroute;i<shared.shareddata.nroute_alloc;i++)
					{
					shared.shareddata.routes[i].color = MBV_COLOR_RED;
					shared.shareddata.routes[i].size = 1;
					shared.shareddata.routes[i].name[0] = '\0';
					shared.shareddata.routes[i].npoints = 0;
					shared.shareddata.routes[i].npoints_alloc = MBV_ALLOC_NUM;
					shared.shareddata.routes[i].points = NULL;
					shared.shareddata.routes[i].segments = NULL;
					status = mb_realloc(mbv_verbose, 
			    				shared.shareddata.routes[i].npoints_alloc * sizeof(int),
			    				&(shared.shareddata.routes[i].waypoint), &error);
					status = mb_realloc(mbv_verbose, 
			    				shared.shareddata.routes[i].npoints_alloc * sizeof(struct mbview_pointw_struct),
			    				&(shared.shareddata.routes[i].points), &error);
					status = mb_realloc(mbv_verbose, 
			    				shared.shareddata.routes[i].npoints_alloc * sizeof(struct mbview_linesegmentw_struct),
			    				&(shared.shareddata.routes[i].segments), &error);
					for (j=0;j<shared.shareddata.routes[i].npoints_alloc-1;j++)
						{
						shared.shareddata.routes[i].segments[j].nls = 0;
						shared.shareddata.routes[i].segments[j].nls_alloc = 0;
						shared.shareddata.routes[i].segments[j].lspoints = NULL;
						shared.shareddata.routes[i].segments[j].endpoints[0] = &(shared.shareddata.routes[i].points[j]);
						shared.shareddata.routes[i].segments[j].endpoints[1] = &(shared.shareddata.routes[i].points[j+1]);
						}
					}
				}
			}
			
		/* set nroute */
		shared.shareddata.nroute++;

		/* add the new route */
		shared.shareddata.routes[inew].color = MBV_COLOR_RED;
		shared.shareddata.routes[inew].size = 1;
		sprintf(shared.shareddata.routes[inew].name, "Route:%d", shared.shareddata.nroute);
		}

	/* allocate memory for point if required */
	if (status == MB_SUCCESS
		&& shared.shareddata.routes[inew].npoints_alloc < shared.shareddata.routes[inew].npoints + 1)
		{
		shared.shareddata.routes[inew].npoints_alloc += MBV_ALLOC_NUM;
		status = mb_realloc(mbv_verbose, 
			    	shared.shareddata.routes[inew].npoints_alloc * sizeof(int),
			    	&(shared.shareddata.routes[inew].waypoint), &error);
		status = mb_realloc(mbv_verbose, 
			    	shared.shareddata.routes[inew].npoints_alloc * sizeof(struct mbview_pointw_struct),
			    	&(shared.shareddata.routes[inew].points), &error);
		status = mb_realloc(mbv_verbose, 
			    	shared.shareddata.routes[inew].npoints_alloc * sizeof(struct mbview_linesegmentw_struct),
			    	&(shared.shareddata.routes[inew].segments), &error);
		if (status == MB_FAILURE)
			{
			shared.shareddata.routes[inew].npoints = 0;
			shared.shareddata.routes[inew].npoints_alloc = 0;
			}
		}

	/* add the new route point */
	if (status == MB_SUCCESS)
		{
		/* move points after jnew if necessary */
		for (j=shared.shareddata.routes[inew].npoints;j>jnew;j--)
			{
			shared.shareddata.routes[inew].waypoint[j] = shared.shareddata.routes[inew].waypoint[j-1];
			shared.shareddata.routes[inew].points[j] = shared.shareddata.routes[inew].points[j-1];
			}

		/* move segments after jnew if necessary */
		for (j=shared.shareddata.routes[inew].npoints-1;j>jnew;j--)
			{
			shared.shareddata.routes[inew].segments[j] = shared.shareddata.routes[inew].segments[j-1];
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
		shared.shareddata.routes[inew].segments[jnew].endpoints[0] = &(shared.shareddata.routes[inew].points[jnew]);
		shared.shareddata.routes[inew].segments[jnew].endpoints[1] = &(shared.shareddata.routes[inew].points[jnew+1]);
		if (jnew > 0)
			{
			shared.shareddata.routes[inew].segments[jnew-1].endpoints[0] = &(shared.shareddata.routes[inew].points[jnew-1]);
			shared.shareddata.routes[inew].segments[jnew-1].endpoints[1] = &(shared.shareddata.routes[inew].points[jnew]);
			}

		/* set npoints */
		shared.shareddata.routes[inew].npoints++;

		/* drape the affected segments */
		if (jnew > 0)
			{
			/* drape the segment */
			mbview_drapesegmentw(instance, &(shared.shareddata.routes[inew].segments[jnew-1]));
			
			/* update the segment for all active instances */
			mbview_updatesegmentw(instance, &(shared.shareddata.routes[inew].segments[jnew-1]));
			}
		if (jnew < shared.shareddata.routes[inew].npoints - 1)
			{
			/* drape the segment */
			mbview_drapesegmentw(instance, &(shared.shareddata.routes[inew].segments[jnew]));
			
			/* update the segment for all active instances */
			mbview_updatesegmentw(instance, &(shared.shareddata.routes[inew].segments[jnew]));
			}
	
		/* make routes viewable */
		if (data->route_view_mode != MBV_VIEW_ON)
			{
			data->route_view_mode = MBV_VIEW_ON;
			set_mbview_route_view_mode(instance, MBV_VIEW_ON);
			}
		}

	
	/* print route debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Route data altered in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Route values:\n");
		fprintf(stderr,"dbg2       route_view_mode:      %d\n",data->route_view_mode);
		fprintf(stderr,"dbg2       route_mode:           %d\n",shared.shareddata.route_mode);
		fprintf(stderr,"dbg2       nroute:               %d\n",shared.shareddata.nroute);
		fprintf(stderr,"dbg2       nroute_alloc:         %d\n",shared.shareddata.nroute_alloc);
		fprintf(stderr,"dbg2       route_selected:       %d\n",shared.shareddata.route_selected);
		fprintf(stderr,"dbg2       route_point_selected: %d\n",shared.shareddata.route_point_selected);
		for (i=0;i<shared.shareddata.nroute;i++)
			{
			fprintf(stderr,"dbg2       route %d color:         %d\n",i,shared.shareddata.routes[i].color);
			fprintf(stderr,"dbg2       route %d size:          %d\n",i,shared.shareddata.routes[i].size);
			fprintf(stderr,"dbg2       route %d name:          %s\n",i,shared.shareddata.routes[i].name);
			fprintf(stderr,"dbg2       route %d npoints:       %d\n",i,shared.shareddata.routes[i].npoints);
			fprintf(stderr,"dbg2       route %d npoints_alloc: %d\n",i,shared.shareddata.routes[i].npoints_alloc);
			for (j=0;j<shared.shareddata.routes[i].npoints;j++)
				{
				fprintf(stderr,"dbg2       route %d %d xgrid:    %f\n",i,j,shared.shareddata.routes[i].points[j].xgrid[instance]);
				fprintf(stderr,"dbg2       route %d %d ygrid:    %f\n",i,j,shared.shareddata.routes[i].points[j].ygrid[instance]);
				fprintf(stderr,"dbg2       route %d %d xlon:     %f\n",i,j,shared.shareddata.routes[i].points[j].xlon);
				fprintf(stderr,"dbg2       route %d %d ylat:     %f\n",i,j,shared.shareddata.routes[i].points[j].ylat);
				fprintf(stderr,"dbg2       route %d %d zdata:    %f\n",i,j,shared.shareddata.routes[i].points[j].zdata);
				fprintf(stderr,"dbg2       route %d %d xdisplay: %f\n",i,j,shared.shareddata.routes[i].points[j].xdisplay[instance]);
				fprintf(stderr,"dbg2       route %d %d ydisplay: %f\n",i,j,shared.shareddata.routes[i].points[j].ydisplay[instance]);
				fprintf(stderr,"dbg2       route %d %d zdisplay: %f\n",i,j,shared.shareddata.routes[i].points[j].zdisplay[instance]);
				}
			for (j=0;j<shared.shareddata.routes[i].npoints-1;j++)
				{
				fprintf(stderr,"dbg2       route %d %d nls:          %d\n",i,j,shared.shareddata.routes[i].segments[j].nls);
				fprintf(stderr,"dbg2       route %d %d nls_alloc:    %d\n",i,j,shared.shareddata.routes[i].segments[j].nls_alloc);
				fprintf(stderr,"dbg2       route %d %d endpoints[0]: %f\n",i,j,shared.shareddata.routes[i].segments[j].endpoints[0]);
				fprintf(stderr,"dbg2       route %d %d endpoints[1]: %d\n",i,j,shared.shareddata.routes[i].segments[j].endpoints[1]);
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
int mbview_route_delete(int instance, int iroute, int ipoint)
{

	/* local variables */
	char	*function_name = "mbview_route_delete";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
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
		fprintf(stderr,"dbg2       iroute:          %d\n",iroute);
		fprintf(stderr,"dbg2       ipoint:          %d\n",ipoint);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* delete route point if its valid */
	if (iroute >= 0 && iroute < shared.shareddata.nroute
		&& ipoint >= 0 && ipoint < shared.shareddata.routes[iroute].npoints)
		{
		/* free segment */
		if (shared.shareddata.routes[iroute].npoints > 1)
			{
			if (ipoint == 0)
				{
				mb_free(mbv_verbose,&(shared.shareddata.routes[iroute].segments[ipoint].lspoints),&error);
				shared.shareddata.routes[iroute].segments[ipoint].nls = 0;
				shared.shareddata.routes[iroute].segments[ipoint].nls_alloc = 0;
				}
			else
				{
				mb_free(mbv_verbose,&(shared.shareddata.routes[iroute].segments[ipoint].lspoints),&error);
				shared.shareddata.routes[iroute].segments[ipoint].nls = 0;
				shared.shareddata.routes[iroute].segments[ipoint].nls_alloc = 0;
				}
			}

		/* move route point data if necessary */
		for (j=ipoint;j<shared.shareddata.routes[iroute].npoints-1;j++)
			{
			shared.shareddata.routes[iroute].waypoint[j] = shared.shareddata.routes[iroute].waypoint[j+1];
			shared.shareddata.routes[iroute].points[j] = shared.shareddata.routes[iroute].points[j+1];
			}

		/* move route segment data if necessary */
		for (j=ipoint;j<shared.shareddata.routes[iroute].npoints-2;j++)
			{
			shared.shareddata.routes[iroute].segments[j] = shared.shareddata.routes[iroute].segments[j+1];
			}

		/* decrement npoints */
		shared.shareddata.routes[iroute].npoints--;

		/* if route still has points then reset affected segment endpoints */
		if (shared.shareddata.routes[iroute].npoints > 0)
			{
			for (j=MAX(0,ipoint-1);j<MIN(shared.shareddata.routes[iroute].npoints-1,ipoint+1);j++)
				{
				shared.shareddata.routes[iroute].segments[j].endpoints[0] = &(shared.shareddata.routes[iroute].points[j]);
				shared.shareddata.routes[iroute].segments[j].endpoints[1] = &(shared.shareddata.routes[iroute].points[j+1]);

				/* drape the segment */
				mbview_drapesegmentw(instance, &(shared.shareddata.routes[iroute].segments[j]));
			
				/* update the segment for all active instances */
				mbview_updatesegmentw(instance, &(shared.shareddata.routes[iroute].segments[j]));
				}
			}

		/* if last point deleted then move remaining routes if necessary */
		if (shared.shareddata.routes[iroute].npoints <= 0)
			{
			/* move route data if necessary */
			for (i=iroute;i<shared.shareddata.nroute-1;i++)
				{
				shared.shareddata.routes[i] = shared.shareddata.routes[i+1];
				}

			/* decrement nroute */
			shared.shareddata.nroute--;
			}

		/* no route selection now */
		if (shared.shareddata.route_selected != MBV_SELECT_NONE)
			{
			shared.shareddata.route_selected = MBV_SELECT_NONE;
			shared.shareddata.route_point_selected = MBV_SELECT_NONE;
			data->pickinfo_mode = data->pick_type;
			}
		}

	/* else beep */
	else
		{
		XBell(view->dpy,100);
		}
	
	/* print route debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Route data altered in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Route values:\n");
		fprintf(stderr,"dbg2       route_view_mode:      %d\n",data->route_view_mode);
		fprintf(stderr,"dbg2       route_mode:           %d\n",shared.shareddata.route_mode);
		fprintf(stderr,"dbg2       nroute:               %d\n",shared.shareddata.nroute);
		fprintf(stderr,"dbg2       nroute_alloc:         %d\n",shared.shareddata.nroute_alloc);
		fprintf(stderr,"dbg2       route_selected:       %d\n",shared.shareddata.route_selected);
		fprintf(stderr,"dbg2       route_point_selected: %d\n",shared.shareddata.route_point_selected);
		for (i=0;i<shared.shareddata.nroute;i++)
			{
			fprintf(stderr,"dbg2       route %d color:         %d\n",i,shared.shareddata.routes[i].color);
			fprintf(stderr,"dbg2       route %d size:          %d\n",i,shared.shareddata.routes[i].size);
			fprintf(stderr,"dbg2       route %d name:          %s\n",i,shared.shareddata.routes[i].name);
			fprintf(stderr,"dbg2       route %d npoints:       %d\n",i,shared.shareddata.routes[i].npoints);
			fprintf(stderr,"dbg2       route %d npoints_alloc: %d\n",i,shared.shareddata.routes[i].npoints_alloc);
			for (j=0;j<shared.shareddata.routes[i].npoints;j++)
				{
				fprintf(stderr,"dbg2       route %d %d xgrid:    %f\n",i,j,shared.shareddata.routes[i].points[j].xgrid[instance]);
				fprintf(stderr,"dbg2       route %d %d ygrid:    %f\n",i,j,shared.shareddata.routes[i].points[j].ygrid[instance]);
				fprintf(stderr,"dbg2       route %d %d xlon:     %f\n",i,j,shared.shareddata.routes[i].points[j].xlon);
				fprintf(stderr,"dbg2       route %d %d ylat:     %f\n",i,j,shared.shareddata.routes[i].points[j].ylat);
				fprintf(stderr,"dbg2       route %d %d zdata:    %f\n",i,j,shared.shareddata.routes[i].points[j].zdata);
				fprintf(stderr,"dbg2       route %d %d xdisplay: %f\n",i,j,shared.shareddata.routes[i].points[j].xdisplay[instance]);
				fprintf(stderr,"dbg2       route %d %d ydisplay: %f\n",i,j,shared.shareddata.routes[i].points[j].ydisplay[instance]);
				fprintf(stderr,"dbg2       route %d %d zdisplay: %f\n",i,j,shared.shareddata.routes[i].points[j].zdisplay[instance]);
				}
			for (j=0;j<shared.shareddata.routes[i].npoints-1;j++)
				{
				fprintf(stderr,"dbg2       route %d %d nls:          %d\n",i,j,shared.shareddata.routes[i].segments[j].nls);
				fprintf(stderr,"dbg2       route %d %d nls_alloc:    %d\n",i,j,shared.shareddata.routes[i].segments[j].nls_alloc);
				fprintf(stderr,"dbg2       route %d %d endpoints[0]: %d\n",i,j,shared.shareddata.routes[i].segments[j].endpoints[0]);
				fprintf(stderr,"dbg2       route %d %d endpoints[1]: %d\n",i,j,shared.shareddata.routes[i].segments[j].endpoints[1]);
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
int mbview_drawroute(int instance, int rez)
{
	/* local variables */
	char	*function_name = "mbview_drawroute";
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
	int	iroute, jpoint;
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
		
	/* draw route points in 2D */
	if (shared.shareddata.route_mode != MBV_ROUTE_OFF
		&& data->display_mode == MBV_DISPLAY_2D
		&& data->route_view_mode == MBV_VIEW_ON
		&& shared.shareddata.nroute > 0)
		{
		/* set stride for looping over data */
		stride = 1;
		
		/* loop over the route points */
		for (iroute=0;iroute<shared.shareddata.nroute;iroute++)
			{
			for (jpoint=0;jpoint<shared.shareddata.routes[iroute].npoints;jpoint++)
				{
				/* get grid bounds for plotting */
				ix = (shared.shareddata.routes[iroute].points[jpoint].xgrid[instance] - data->primary_xmin) 
						/ data->primary_dx;
				jy = (shared.shareddata.routes[iroute].points[jpoint].ygrid[instance] - data->primary_ymin) 
						/ data->primary_dy;

				if (ix >= 0 && ix < data->primary_nx
					&& jy >= 0 && jy < data->primary_ny)
					{
					ixsize = MAX(data->viewbounds[1] - data->viewbounds[0] + 1, 
							data->viewbounds[3] - data->viewbounds[2] + 1);
					if (iroute == shared.shareddata.route_selected
						&& jpoint == shared.shareddata.route_point_selected)
						ixsize /= 200;
					else
						ixsize /= 300;
					if (ixsize < 1) ixsize = 1;
					jysize = (int)((ixsize * (1000.0 * data->primary_dy 
								/ data->primary_dx)) / 1000.0);
					if (ixsize < 1) ixsize = 1;
					if (jysize < 1) jysize = 1;
					ixmin = MAX(ix - ixsize, 0);
					ixmax = MIN(ix + ixsize, data->primary_nx - stride);
					jymin = MAX(jy - jysize, 0);
					jymax = MIN(jy + jysize, data->primary_ny - stride);

					/* set the color for this route */
					if (iroute == shared.shareddata.route_selected
						&& jpoint == shared.shareddata.route_point_selected)
						icolor = MBV_COLOR_RED;
					else
						icolor = shared.shareddata.routes[iroute].color;
					glColor3f(colortable_object_red[icolor], 
						colortable_object_green[icolor], 
						colortable_object_blue[icolor]);

					/* draw the route box */
					glLineWidth(2.0);
					glBegin(GL_QUADS);
					k = ixmin * data->primary_ny + jymin;
					glVertex3f(data->primary_x[k],
						   data->primary_y[k],
						   data->primary_z[k] + MBV_OPENGL_3D_LINE_OFFSET);
					k = ixmax * data->primary_ny + jymin;
					glVertex3f(data->primary_x[k],
						   data->primary_y[k],
						   data->primary_z[k] + MBV_OPENGL_3D_LINE_OFFSET);
					k = ixmax * data->primary_ny + jymax;
					glVertex3f(data->primary_x[k],
						   data->primary_y[k],
						   data->primary_z[k] + MBV_OPENGL_3D_LINE_OFFSET);
					k = ixmin * data->primary_ny + jymax;
					glVertex3f(data->primary_x[k],
						   data->primary_y[k],
						   data->primary_z[k] + MBV_OPENGL_3D_LINE_OFFSET);
					glEnd();

					/* draw the boundary */
					icolor = MBV_COLOR_BLACK;
					glColor3f(colortable_object_red[icolor], 
						colortable_object_green[icolor], 
						colortable_object_blue[icolor]);
					glLineWidth(2.0);
					glBegin(GL_LINE_LOOP);
					k = ixmin * data->primary_ny + jymin;
					glVertex3f(data->primary_x[k],
						   data->primary_y[k],
						   data->primary_z[k] + MBV_OPENGL_3D_LINE_OFFSET);
					k = ixmax * data->primary_ny + jymin;
					glVertex3f(data->primary_x[k],
						   data->primary_y[k],
						   data->primary_z[k] + MBV_OPENGL_3D_LINE_OFFSET);
					k = ixmax * data->primary_ny + jymax;
					glVertex3f(data->primary_x[k],
						   data->primary_y[k],
						   data->primary_z[k] + MBV_OPENGL_3D_LINE_OFFSET);
					k = ixmin * data->primary_ny + jymax;
					glVertex3f(data->primary_x[k],
						   data->primary_y[k],
						   data->primary_z[k] + MBV_OPENGL_3D_LINE_OFFSET);
					glEnd();
					}
				}
			}
		}
		
	/* draw route points in 3D */
	else if (shared.shareddata.route_mode != MBV_ROUTE_OFF
		&& data->display_mode == MBV_DISPLAY_3D
		&& data->route_view_mode == MBV_VIEW_ON
		&& shared.shareddata.nroute > 0)
		{
		/* set stride for looping over data */
		stride = 1;
		
		/* loop over the route points */
		for (iroute=0;iroute<shared.shareddata.nroute;iroute++)
			{
			for (jpoint=0;jpoint<shared.shareddata.routes[iroute].npoints;jpoint++)
				{
				/* get grid bounds for plotting */
				ix = (shared.shareddata.routes[iroute].points[jpoint].xgrid[instance] - data->primary_xmin) 
						/ data->primary_dx;
				jy = (shared.shareddata.routes[iroute].points[jpoint].ygrid[instance] - data->primary_ymin) 
						/ data->primary_dy;
				if (ix >= 0 && ix < data->primary_nx
					&& jy >= 0 && jy < data->primary_ny)
					{
					ixsize = MAX(data->viewbounds[1] - data->viewbounds[0] + 1, 
							data->viewbounds[3] - data->viewbounds[2] + 1);
					if (iroute == shared.shareddata.route_selected
						&& jpoint == shared.shareddata.route_point_selected)
						ixsize /= 200;
					else
						ixsize /= 300;
					if (ixsize < 1) ixsize = 1;
					jysize = (int)((ixsize * (1000.0 * data->primary_dy 
								/ data->primary_dx)) / 1000.0);
					if (ixsize < 1) ixsize = 1;
					jysize = (int)((ixsize * (1000.0 * data->primary_dy / data->primary_dx)) / 1000.0);
					if (ixsize < 1) ixsize = 1;
					ixmin = MAX(stride * ((ix - ixsize) / stride), 0);
					ixmax = MIN(stride * ((ix + ixsize) / stride + 1), 
							data->primary_nx - stride);
					jymin = MAX(stride * ((jy - jysize) / stride), 0);
					jymax = MIN(stride * ((jy + jysize) / stride + 1), 
							data->primary_ny - stride);

					/* set the color for this site */
					if (iroute == shared.shareddata.route_selected
						&& jpoint == shared.shareddata.route_point_selected)
						icolor = MBV_COLOR_RED;
					else
						icolor = shared.shareddata.routes[iroute].color;
					glColor3f(colortable_object_red[icolor], 
						colortable_object_green[icolor], 
						colortable_object_blue[icolor]);

					/* draw the data as triangle strips */
					for (i=ixmin;i<ixmax;i+=stride)
						{
						on = MB_NO;
						flip = MB_NO;
						for (j=jymin;j<=jymax;j+=stride)
							{
							k = i * data->primary_ny + j;
							l = (i + stride) * data->primary_ny + j;
							if (flip == MB_NO)
								{
								kk = k;
								ll = l;
								}
							else
								{
								kk = l;
								ll = k;
								}
							if (data->primary_data[kk] != data->primary_nodatavalue)
								{
								if (on == MB_NO)
									{
									glBegin(GL_TRIANGLE_STRIP);
									on = MB_YES;
									if (kk == k)
										flip = MB_NO;
									else
										flip = MB_YES;
									}
								glVertex3f(data->primary_x[kk],
									data->primary_y[kk],
									data->primary_z[kk]+MBV_OPENGL_3D_LINE_OFFSET);
								}
							else
								{
								glEnd();
								on = MB_NO;
								flip = MB_NO;
								}
							if (data->primary_data[ll] != data->primary_nodatavalue)
								{
								if (on == MB_NO)
									{
									glBegin(GL_TRIANGLE_STRIP);
									on = MB_YES;
									if (ll == l)
										flip = MB_NO;
									else
										flip = MB_YES;
									}
								glVertex3f(data->primary_x[ll],
									data->primary_y[ll],
									data->primary_z[ll]+MBV_OPENGL_3D_LINE_OFFSET);
								}
							else
								{
								glEnd();
								on = MB_NO;
								flip = MB_NO;
								}
							}
						if (on == MB_YES)
							{
							glEnd();
							on = MB_NO;
							flip = MB_NO;
							}
						glEnd();
						}

					/* draw the boundary */
					if (iroute == shared.shareddata.route_selected
						&& jpoint == shared.shareddata.route_point_selected)
						icolor = MBV_COLOR_BLACK;
					else
						icolor = shared.shareddata.routes[iroute].color;
					glColor3f(colortable_object_red[icolor], 
						colortable_object_green[icolor], 
						colortable_object_blue[icolor]);
					glLineWidth(2.0);
					glBegin(GL_LINE_LOOP);
					k = ixmin * data->primary_ny + jymin;
					glVertex3f(data->primary_x[k],
						   data->primary_y[k],
						   data->primary_z[k] + MBV_OPENGL_3D_LINE_OFFSET);
					k = ixmax * data->primary_ny + jymin;
					glVertex3f(data->primary_x[k],
						   data->primary_y[k],
						   data->primary_z[k] + MBV_OPENGL_3D_LINE_OFFSET);
					k = ixmax * data->primary_ny + jymax;
					glVertex3f(data->primary_x[k],
						   data->primary_y[k],
						   data->primary_z[k] + MBV_OPENGL_3D_LINE_OFFSET);
					k = ixmin * data->primary_ny + jymax;
					glVertex3f(data->primary_x[k],
						   data->primary_y[k],
						   data->primary_z[k] + MBV_OPENGL_3D_LINE_OFFSET);
					glEnd();
					}
				}
			}
		}
		
		
	/* draw draped route line segments */
	if (shared.shareddata.route_mode != MBV_ROUTE_OFF
		&& data->route_view_mode == MBV_VIEW_ON
		&& shared.shareddata.nroute > 0)
		{
		/* loop over the routes */
		for (iroute=0;iroute<shared.shareddata.nroute;iroute++)
			{
			glColor3f(0.0, 0.0, 0.0);
			glLineWidth((float)(2.0));
			glBegin(GL_LINE_STRIP);
			for (jpoint=0;jpoint<shared.shareddata.routes[iroute].npoints-1;jpoint++)
				{
				/* set size and color */
				if (iroute == shared.shareddata.route_selected
					&& (jpoint == shared.shareddata.route_point_selected
						|| jpoint == shared.shareddata.route_point_selected - 1))
					{
					icolor = MBV_COLOR_RED;
					}
				else
					{
					icolor = shared.shareddata.routes[iroute].color;
					}
				glColor3f(colortable_object_red[icolor], 
					colortable_object_green[icolor], 
					colortable_object_blue[icolor]);
					
				/* draw draped segment */
				for (k=0;k<shared.shareddata.routes[iroute].segments[jpoint].nls;k++)
					{
					/* draw points */
					glVertex3f((float)(shared.shareddata.routes[iroute].segments[jpoint].lspoints[k].xdisplay[instance]), 
							(float)(shared.shareddata.routes[iroute].segments[jpoint].lspoints[k].ydisplay[instance]), 
							(float)(shared.shareddata.routes[iroute].segments[jpoint].lspoints[k].zdisplay[instance]));
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
int mbview_updateroutelist()
{
	/* local variables */
	char	*function_name = "mbview_updateroutelist";
	int	status = MB_SUCCESS;
   	XmString *xstr;
	int	iroute;
	int	jpoint;
	int	nitems;
	int	iitem;
	char	lonstr0[24];
	char	latstr0[24];
	

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		}
		
	/* update route list */
	if (shared.init_routelist == MBV_WINDOW_VISIBLE)
		{
		/* remove all existing items */
		XmListDeleteAllItems(shared.mb3d_routelist.mbview_list_routelist);
		
		if (shared.shareddata.nroute > 0)
			{
			/* get number of items */
			nitems = 0;
			for (iroute=0;iroute<shared.shareddata.nroute;iroute++)
				{
				nitems += 1 + shared.shareddata.routes[iroute].npoints;
				}
			
			/* allocate array of label XmStrings */
			xstr = (XmString *) malloc(nitems * sizeof(XmString));

			/* loop over the routes */
			nitems = 0;
			for (iroute=0;iroute<shared.shareddata.nroute;iroute++)
				{
				/* add list item for each waypoint */
				for (jpoint=0;jpoint<shared.shareddata.routes[iroute].npoints;jpoint++)
					{
					/* add list item for each route */
					mbview_setlonlatstrings(shared.shareddata.routes[iroute].points[jpoint].xlon,
								shared.shareddata.routes[iroute].points[jpoint].ylat,
								lonstr0, latstr0);
					sprintf(value_string,"%3d | %3d | %s | %s | %.3f | %s | %d | %s", 
						iroute, jpoint, lonstr0, latstr0,
						shared.shareddata.routes[iroute].points[jpoint].zdata,
						mbview_colorname[shared.shareddata.routes[iroute].color],
						shared.shareddata.routes[iroute].size,
						shared.shareddata.routes[iroute].name);
      					xstr[nitems] = XmStringCreateLocalized(value_string);
					nitems++;
					}
				}

			/* add list items */
    			XmListAddItems(shared.mb3d_routelist.mbview_list_routelist,
					xstr, nitems, 0);
					
			/* select list item for selected route */
			if (shared.shareddata.route_selected != MBV_SELECT_NONE)
				{
				/* get item number */
				iitem = 1;
				for (iroute=0;iroute<=shared.shareddata.route_selected;iroute++)
					{
					if (iroute < shared.shareddata.route_selected)
						iitem += shared.shareddata.routes[iroute].npoints;
					else if (iroute == shared.shareddata.route_selected)
						iitem += shared.shareddata.route_point_selected;
					}

				XmListSelectPos(shared.mb3d_routelist.mbview_list_routelist,
						iitem,0);
				XmListSetPos(shared.mb3d_routelist.mbview_list_routelist,
					MAX(iitem-5, 1));
				}

			/* deallocate memory no longer needed */
			for (iitem=0;iitem<nitems;iitem++)
				{
    				XmStringFree(xstr[iitem]);
    				}
    			free(xstr);
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

