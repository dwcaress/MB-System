/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_route.c	9/25/2003
 *    $Id: mbview_route.c,v 5.12 2006-04-11 19:17:04 caress Exp $
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
 * Revision 5.11  2006/01/24 19:21:32  caress
 * Version 5.0.8 beta.
 *
 * Revision 5.10  2005/11/05 01:11:47  caress
 * Much work over the past two months.
 *
 * Revision 5.9  2005/04/07 04:16:31  caress
 * Fixed a route handling problem.
 *
 * Revision 5.8  2005/03/25 04:46:15  caress
 * Fixed MBgrdviz crashes related to route data by fixing problem with allocation and deallocation of route arrays in the mbview library.
 *
 * Revision 5.7  2005/02/08 22:37:43  caress
 * Heading towards 5.0.6 release.
 *
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
#include <stdlib.h>
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
static Cardinal 	ac;
static Arg      	args[256];
static char		value_text[MB_PATH_MAXLINE];
static char		value_string[MB_PATH_MAXLINE];

static char rcs_id[]="$Id: mbview_route.c,v 5.12 2006-04-11 19:17:04 caress Exp $";

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
	double	xx1, yy1, xx2, yy2;
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
				xx1 = shared.shareddata.routes[route].points[i].xdisplay[instance];
				yy1 = shared.shareddata.routes[route].points[i].ydisplay[instance];
				xx2 = shared.shareddata.routes[route].points[i+1].xdisplay[instance];
				yy2 = shared.shareddata.routes[route].points[i+1].ydisplay[instance];
				dx = (double)(shared.shareddata.routes[route].points[i+1].xdisplay[instance]
						- shared.shareddata.routes[route].points[i].xdisplay[instance]);
				dy = (double)(shared.shareddata.routes[route].points[i+1].ydisplay[instance]
						- shared.shareddata.routes[route].points[i].ydisplay[instance]);
				dx = xx2 - xx1;
				dy = yy2 - yy1;
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
	int	error = MB_ERROR_NO_ERROR;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	found;
	double	xgrid, ygrid;
	double	xlon, ylat, zdata;
	double	xdisplay, ydisplay, zdisplay;
	double	xx, yy, rr, rrmin;
	double	dx, dy;
	int	iroute;
	int	jpoint;
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
			iroute = shared.shareddata.route_selected;
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
			if (jpoint > 0)
				{
				/* drape the segment */
				mbview_drapesegmentw(instance, &(shared.shareddata.routes[iroute].segments[jpoint-1]));
			
				/* update the segment for all active instances */
				mbview_updatesegmentw(instance, &(shared.shareddata.routes[iroute].segments[jpoint-1]));
				}
			if (jpoint < shared.shareddata.routes[iroute].npoints - 1)
				{
				/* drape the segment */
				mbview_drapesegmentw(instance, &(shared.shareddata.routes[iroute].segments[jpoint]));
			
				/* update the segment for all active instances */
				mbview_updatesegmentw(instance, &(shared.shareddata.routes[iroute].segments[jpoint]));
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
int mbview_extract_route_profile(int instance)
{

	/* local variables */
	char	*function_name = "mbview_extract_route_profile";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	iroute, jpoint, jstart;
	int	nprpoints;
	double	dx, dy;
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
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
		
	/* if a route is selected, extract the profile */
	if (shared.shareddata.route_selected != MBV_SELECT_NONE
		&& shared.shareddata.routes[shared.shareddata.route_selected].npoints > 1)
		{
		data->profile.source = MBV_PROFILE_ROUTE;
		strcpy(data->profile.source_name, "Route");
		data->profile.length = 0.0;
		iroute = shared.shareddata.route_selected;
		
		/* make sure enough memory is allocated for the profile */
		nprpoints = 0;
		for (i=0;i<shared.shareddata.routes[iroute].npoints-1;i++)
			{
			nprpoints += shared.shareddata.routes[iroute].segments[i].nls;
			}
		if (data->profile.npoints_alloc < nprpoints)
			{
			status = mbview_allocprofilearrays(mbv_verbose, 
					nprpoints, &(data->profile.points), &error);
			if (status == MB_SUCCESS)
				{
				data->profile.npoints_alloc = nprpoints;
				}
			else
				{
				data->profile.npoints_alloc = 0;
				}
			}
			
		/* extract the profile */
		if (nprpoints > 2 && data->profile.npoints_alloc >= nprpoints)
			{
			data->profile.npoints = 0;
			for (i=0;i<shared.shareddata.routes[iroute].npoints-1;i++)
				{
				if (i == 0)
					jstart = 0;
				else
					jstart = 1;
				for (j=jstart;j<shared.shareddata.routes[iroute].segments[i].nls;j++)
					{
					if (j == 0 || j == shared.shareddata.routes[iroute].segments[i].nls - 1)
						data->profile.points[data->profile.npoints].boundary = MB_YES;
					else
						data->profile.points[data->profile.npoints].boundary = MB_NO;
					data->profile.points[data->profile.npoints].xgrid = shared.shareddata.routes[iroute].segments[i].lspoints[j].xgrid[instance];
					data->profile.points[data->profile.npoints].ygrid = shared.shareddata.routes[iroute].segments[i].lspoints[j].ygrid[instance];
					data->profile.points[data->profile.npoints].xlon = shared.shareddata.routes[iroute].segments[i].lspoints[j].xlon;
					data->profile.points[data->profile.npoints].ylat = shared.shareddata.routes[iroute].segments[i].lspoints[j].ylat;
					data->profile.points[data->profile.npoints].zdata = shared.shareddata.routes[iroute].segments[i].lspoints[j].zdata;
					data->profile.points[data->profile.npoints].xdisplay = shared.shareddata.routes[iroute].segments[i].lspoints[j].xdisplay[instance];
					data->profile.points[data->profile.npoints].ydisplay = shared.shareddata.routes[iroute].segments[i].lspoints[j].ydisplay[instance];
					if (data->profile.npoints == 0)
						{
						data->profile.zmin = data->profile.points[data->profile.npoints].zdata;
						data->profile.zmax = data->profile.points[data->profile.npoints].zdata;
						data->profile.points[data->profile.npoints].distance = 0.0;
						}
					else
						{
						data->profile.zmin = MIN(data->profile.zmin, data->profile.points[data->profile.npoints].zdata);
						data->profile.zmax = MAX(data->profile.zmax, data->profile.points[data->profile.npoints].zdata);
						if (data->display_projection_mode != MBV_PROJECTION_SPHEROID)
							{
							dx = data->profile.points[data->profile.npoints].xdisplay
									- data->profile.points[data->profile.npoints-1].xdisplay;
							dy = data->profile.points[data->profile.npoints].ydisplay
									- data->profile.points[data->profile.npoints-1].ydisplay;
							data->profile.points[data->profile.npoints].distance = sqrt(dx * dx + dy * dy) / view->scale 
								+ data->profile.points[data->profile.npoints-1].distance;
							}
						else
							{
							mbview_greatcircle_dist(instance, 
								data->profile.points[0].xlon, 
								data->profile.points[0].ylat, 
								data->profile.points[data->profile.npoints].xlon, 
								data->profile.points[data->profile.npoints].ylat, 
								&(data->profile.points[data->profile.npoints].distance));
							}
						}
					data->profile.points[data->profile.npoints].navzdata = 0.0;
					data->profile.points[data->profile.npoints].navtime_d = 0.0;
					data->profile.npoints++;
					}
				}
			data->profile.length = data->profile.points[data->profile.npoints-1].distance;
			
			/* calculate slope */
			for (i=0;i<data->profile.npoints;i++)
				{
				if (i == 0)
					{
					dy = (data->profile.points[i+1].zdata
						- data->profile.points[i].zdata);
					dx = (data->profile.points[i+1].distance
						- data->profile.points[i].distance);
					}
				else if (i == data->profile.npoints - 1)
					{
					dy = (data->profile.points[i].zdata
						- data->profile.points[i-1].zdata);
					dx = (data->profile.points[i].distance
						- data->profile.points[i-1].distance);
					}
				else
					{
					dy = (data->profile.points[i+1].zdata
						- data->profile.points[i-1].zdata);
					dx = (data->profile.points[i+1].distance
						- data->profile.points[i-1].distance);
					}
				if (dx > 0.0)
					data->profile.points[i].slope = fabs(dy / dx);
				else
					data->profile.points[i].slope = 0.0;
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
					shared.shareddata.routes[i].waypoint = NULL;
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
	GLUquadricObj *globj;
	double	routesizesmall, routesizelarge;
	double	rr, xx, yy;
	int	iroute, jpoint;
	int	icolor;
	int	k, k0, k1;

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
	
	/* Generate GL lists to be plotted */
	if (shared.shareddata.route_mode != MBV_ROUTE_OFF
		&& data->route_view_mode == MBV_VIEW_ON
		&& shared.shareddata.nroute > 0)
		{
		/* get size according to viewbounds */
		k0 = data->viewbounds[0] * data->primary_ny + data->viewbounds[2];
		k1 = data->viewbounds[1] * data->primary_ny + data->viewbounds[3];
		xx = data->primary_x[k1] - data->primary_x[k0];
		yy = data->primary_y[k1] - data->primary_y[k0];
		routesizesmall = 0.004 * sqrt(xx * xx + yy * yy);
		routesizelarge = 1.4 * routesizesmall;
		
		/* Use disks for 2D plotting */
		if (data->display_mode == MBV_DISPLAY_2D)
			{
			/* make list for small route */
	    		glNewList((GLuint)MBV_GLLIST_ROUTESMALL, GL_COMPILE);
			globj = gluNewQuadric();
			gluDisk(globj, 0.0, routesizesmall, 4, 1);
			gluDeleteQuadric(globj);
			icolor = MBV_COLOR_BLACK;
			glColor3f(colortable_object_red[icolor], 
				colortable_object_green[icolor], 
				colortable_object_blue[icolor]);
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
			glColor3f(colortable_object_red[icolor], 
				colortable_object_green[icolor], 
				colortable_object_blue[icolor]);
			globj = gluNewQuadric();
			gluDisk(globj, 0.8 * routesizelarge, routesizelarge, 10, 1);
			gluDeleteQuadric(globj);
			glEndList();
			}
		
		/* Use spheres for 3D plotting */
		else if (data->display_mode == MBV_DISPLAY_3D)
			{
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
		for (iroute=0;iroute<shared.shareddata.nroute;iroute++)
			{
			for (jpoint=0;jpoint<shared.shareddata.routes[iroute].npoints;jpoint++)
				{

				/* set the color for this route */
				if (iroute == shared.shareddata.route_selected
					&& jpoint == shared.shareddata.route_point_selected)
					icolor = MBV_COLOR_RED;
				else
					icolor = shared.shareddata.routes[iroute].color;
				glColor3f(colortable_object_red[icolor], 
					colortable_object_green[icolor], 
					colortable_object_blue[icolor]);

				/* draw the route point as a disk or sphere using GLUT */

				glTranslatef(shared.shareddata.routes[iroute].points[jpoint].xdisplay[instance],
						shared.shareddata.routes[iroute].points[jpoint].ydisplay[instance],
						shared.shareddata.routes[iroute].points[jpoint].zdisplay[instance]);
				if (iroute == shared.shareddata.route_selected
					&& jpoint == shared.shareddata.route_point_selected)
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
					mbview_setlonlatstrings(shared.lonlatstyle, 
								shared.shareddata.routes[iroute].points[jpoint].xlon,
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

