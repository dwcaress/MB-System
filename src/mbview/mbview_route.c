/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_route.c	9/25/2003
 *    $Id: mbview_route.c,v 5.0 2003-12-02 20:38:34 caress Exp $
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

static char rcs_id[]="$Id: mbview_route.c,v 5.0 2003-12-02 20:38:34 caress Exp $";

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

fprintf(stderr,"Called mbview_getroutecount:%d\n",instance);

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
	*nroute = data->nroute;
		
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

fprintf(stderr,"Called mbview_getroutepointcount:%d\n",instance);

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
	if (route >= 0 && route < data->nroute)
		{
		*npoint = data->routes[route].npoints;
		for (i=0;i<*npoint-1;i++)
			{
			if (data->routes[route].segments[i].nls > 2)
				*nintpoint += data->routes[route].segments[i].nls - 2;
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
int mbview_allocroutearrays(int verbose, 
			int	npointtotal,
			double	**routelon,
			double	**routelat,
			int	**waypoint,
			double	**routetopo,
			double	**distlateral,
			double	**distovertopo,
			double	**slope,
			int *error)
{
	/* local variables */
	char	*function_name = "mbview_allocroutearrays";
	int	status = MB_SUCCESS;

fprintf(stderr,"Called mbview_allocroutearrays: npointtotal:%d\n", npointtotal);

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
			double	**distlateral,
			double	**distovertopo,
			double	**slope,
			int *error)
{
	/* local variables */
	char	*function_name = "mbview_freeroutearrays";
	int	status = MB_SUCCESS;

fprintf(stderr,"Called mbview_freeroutearrays:\n");

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
			int	routecolor,
			int	routesize,
			mb_path	routename,
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
	int	iroute;
	int	i, ii, jj, iii, jjj, kkk;

fprintf(stderr,"Called mbview_addroute:%d\n",instance);

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
		for (i=0;i<npoint;i++)
			{
			fprintf(stderr,"dbg2       point:%d lon:%f lat:%f\n", 
					i, routelon[i], routelat[i]);
			}
		fprintf(stderr,"dbg2       routecolor:                %d\n", routecolor);
		fprintf(stderr,"dbg2       routesize:                 %d\n", routesize);
		fprintf(stderr,"dbg2       routename:                 %s\n", routename);
		}

	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* make sure no route is selected */
	data->route_selected = MBV_SELECT_NONE;
	data->route_point_selected = MBV_SELECT_NONE;
	
	/* set route id so that new route is created */
	iroute = data->nroute;
	
	/* loop over the points in the new route */
	for (i=0;i<npoint;i++)
		{
		/* get route positions in grid and display coordinates */
		status = mbview_projectfromlonlat(instance,
				routelon[i], routelat[i], 
				&xgrid, &ygrid,
				&xdisplay, &ydisplay);
				
		/* get topo from primary grid */
		nfound = 0;
		zdata = 0.0;
		ii = (int)((xgrid - data->primary_xmin) / data->primary_dx);
		jj = (int)((ygrid - data->primary_ymin) / data->primary_dy);
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
		    
		/* get zdisplay */
		zdisplay = view->zscale * (zdata - view->zorigin);
			
		/* add the route point */
		mbview_route_add(instance, iroute, i,
			xgrid, ygrid,
			routelon[i], routelat[i], zdata,
			xdisplay, ydisplay, zdisplay);
		}

	/* set color size and name for new route */
	data->routes[iroute].color = routecolor;
	data->routes[iroute].size = routesize;
	strcpy(data->routes[iroute].name,routename);
	
	/* make routes viewable */
	data->route_view_mode = MBV_VIEW_ON;
		
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
	double	xx0, yy0, zz0, xx1, yy1, zz1, xx2, yy2, zz2;
	double	dxx, dyy, dzz;
	double	dll, doo;
	int	i, j;

fprintf(stderr,"Called mbview_getroute:%d\n",instance);

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
		for (i=0;i<data->routes[route].npoints-1;i++)
			{
			/* add first point */
			routelon[*npointtotal] = data->routes[route].points[i].xlon;
			routelat[*npointtotal] = data->routes[route].points[i].ylat ;
			waypoint[*npointtotal] = MB_YES;
			routetopo[*npointtotal] = data->routes[route].points[i].zdata;
			(*npointtotal)++;
			
			/* loop over interior of segment */
			for (j=1;j<data->routes[route].segments[i].nls-1;j++)
				{
				routelon[*npointtotal] = data->routes[route].segments[i].lspoints[j].xlon;
				routelat[*npointtotal] = data->routes[route].segments[i].lspoints[j].ylat;
				waypoint[*npointtotal] = MB_NO;
				routetopo[*npointtotal] = data->routes[route].segments[i].lspoints[j].zdata;
				(*npointtotal)++;
				}
			}
			
		/* add last point */
		j = data->routes[route].npoints - 1;
		routelon[*npointtotal] = data->routes[route].points[j].xlon;
		routelat[*npointtotal] = data->routes[route].points[j].ylat ;
		waypoint[*npointtotal] = MB_YES;
		routetopo[*npointtotal] = data->routes[route].points[j].zdata;
		(*npointtotal)++;
		
		/* get color size and name */
		*routecolor = data->routes[route].color;
		*routesize = data->routes[route].size;
		strcpy(routename, data->routes[route].name);
		
		/* calculate distance and slope */
		mbview_projectll2meters(instance,
			routelon[0], routelat[0],
			&xx1, &yy1);
		zz1 = routetopo[0];
		distlateral[0] = 0.0;
		distovertopo[0] = 0.0;
		for (j=0;j<*npointtotal;j++)
			{
			if (j < *npointtotal - 1)
				{
				mbview_projectll2meters(instance,
					routelon[j+1], routelat[j+1],
					&xx2, &yy2);
				zz2 = routetopo[j+1];
				}
			if (j == 0 && *npointtotal == 1)
				{
				dxx = 0.0;
				dyy = 0.0;
				dzz = 0.0;
				}
			else if (j == 0)
				{
				dxx = xx2 - xx1;
				dyy = yy2 - yy1;
				dzz = zz2 - zz1;
				}
			else if (j == *npointtotal - 1)
				{
				dxx = xx1 - xx0;
				dyy = yy1 - yy0;
				dzz = zz1 - zz0;
				}
			else
				{
				dxx = xx2 - xx0;
				dyy = yy2 - yy0;
				dzz = zz2 - zz0;
				}
			dll = sqrt(dxx * dxx + dyy * dyy);
			doo = sqrt(dxx * dxx + dyy * dyy + dzz * dzz);
			if (j > 0)
				{
				distlateral[j] += dll;
				distovertopo[j] += doo;
				}
			if (dll > 0.0)
				slope[j] = dzz / dll;
			else
				slope[j] = 0.0;
			xx0 = xx1;
			yy0 = yy1;
			zz0 = zz1;
			xx1 = xx2;
			yy1 = yy2;
			zz1 = zz2;
			if (j < *npointtotal - 1)
				{
				distlateral[j+1] = distlateral[j];
				distovertopo[j+1] = distovertopo[j];
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
			fprintf(stderr,"dbg2       route:%d lon:%f lat:%f interp:%d topo:%f color:%d size:%d name:%s\n", 
					i, routelon[i], routelat[i], waypoint[i], routetopo[i], 
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

fprintf(stderr,"Called mbview_enableviewroutes:%d\n",instance);

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
        data->route_mode = MBV_ROUTE_VIEW;
		
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

fprintf(stderr,"Called mbview_enableeditroutes:%d\n",instance);

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
        data->route_mode = MBV_ROUTE_EDIT;
		
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
	if (data->route_mode != MBV_ROUTE_OFF
		&& data->nroute > 0
		&& (which == MBV_PICK_DOWN
			|| data->route_selected == MBV_SELECT_NONE))
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
			data->route_selected = MBV_SELECT_NONE;
			data->route_point_selected = MBV_SELECT_NONE;
			for (i=0;i<data->nroute;i++)
			for (j=0;j<data->routes[i].npoints;j++)
				{
				xx = xgrid - data->routes[i].points[j].xgrid;
				yy = ygrid - data->routes[i].points[j].ygrid;
				rr = sqrt(xx * xx + yy * yy);
				if (rr < rrmin)
					{
					rrmin = rr;
					data->route_selected = i;
					data->route_point_selected = j;
					}
				}
			}
		else
			{
			data->route_selected = MBV_SELECT_NONE;
			XBell(view->dpy,100);
			}
		}
	
	/* only move selected route points if enabled */
	else if (data->route_mode != MBV_ROUTE_OFF
		&& data->nroute > 0
		&& (which == MBV_PICK_MOVE
			&& data->route_selected != MBV_SELECT_NONE))
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
			data->routes[data->route_selected].points[data->route_point_selected].xgrid = xgrid;
			data->routes[data->route_selected].points[data->route_point_selected].ygrid = ygrid;
			data->routes[data->route_selected].points[data->route_point_selected].xlon = xlon;
			data->routes[data->route_selected].points[data->route_point_selected].ylat = ylat;
			data->routes[data->route_selected].points[data->route_point_selected].zdata = zdata;
			data->routes[data->route_selected].points[data->route_point_selected].xdisplay = xdisplay;
			data->routes[data->route_selected].points[data->route_point_selected].ydisplay = ydisplay;
			data->routes[data->route_selected].points[data->route_point_selected].zdisplay = zdisplay;
			
			/* drape the affected segments */
			if (data->route_point_selected > 0)
				{
				/* drape the segment */
				mbview_drapesegment(instance, &(data->routes[data->route_selected].segments[data->route_point_selected-1]));
				}
			if (data->route_point_selected < data->routes[data->route_selected].npoints - 1)
				{
				/* drape the segment */
				mbview_drapesegment(instance, &(data->routes[data->route_selected].segments[data->route_point_selected]));
				}
			}
		}

	/* else beep */
	else
		{
		data->route_selected = MBV_SELECT_NONE;
		XBell(view->dpy,100);
		}
		
	/* set what kind of pick to annotate */
	if (data->route_selected != MBV_SELECT_NONE)
		{
		data->pickinfo_mode = MBV_PICK_ROUTE;
		}
	else
		{
		data->pickinfo_mode = data->pick_type;
		}
		
	/* set pick annotation */
	mbview_pick_text(instance);
	
	/* print route debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Route data altered in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Route values:\n");
		fprintf(stderr,"dbg2       route_view_mode:      %d\n",data->route_view_mode);
		fprintf(stderr,"dbg2       route_mode:           %d\n",data->route_mode);
		fprintf(stderr,"dbg2       nroute:               %d\n",data->nroute);
		fprintf(stderr,"dbg2       nroute_alloc:         %d\n",data->nroute_alloc);
		fprintf(stderr,"dbg2       route_selected:       %d\n",data->route_selected);
		fprintf(stderr,"dbg2       route_point_selected: %d\n",data->route_point_selected);
		for (i=0;i<data->nroute;i++)
			{
			fprintf(stderr,"dbg2       route %d color:         %d\n",i,data->routes[i].color);
			fprintf(stderr,"dbg2       route %d size:          %d\n",i,data->routes[i].size);
			fprintf(stderr,"dbg2       route %d name:          %s\n",i,data->routes[i].name);
			fprintf(stderr,"dbg2       route %d npoints:       %d\n",i,data->routes[i].npoints);
			fprintf(stderr,"dbg2       route %d npoints_alloc: %d\n",i,data->routes[i].npoints_alloc);
			for (j=0;j<data->routes[i].npoints;j++)
				{
				fprintf(stderr,"dbg2       route %d %d xgrid:    %f\n",i,j,data->routes[i].points[j].xgrid);
				fprintf(stderr,"dbg2       route %d %d ygrid:    %f\n",i,j,data->routes[i].points[j].ygrid);
				fprintf(stderr,"dbg2       route %d %d xlon:     %f\n",i,j,data->routes[i].points[j].xlon);
				fprintf(stderr,"dbg2       route %d %d ylat:     %f\n",i,j,data->routes[i].points[j].ylat);
				fprintf(stderr,"dbg2       route %d %d zdata:    %f\n",i,j,data->routes[i].points[j].zdata);
				fprintf(stderr,"dbg2       route %d %d xdisplay: %f\n",i,j,data->routes[i].points[j].xdisplay);
				fprintf(stderr,"dbg2       route %d %d ydisplay: %f\n",i,j,data->routes[i].points[j].ydisplay);
				fprintf(stderr,"dbg2       route %d %d zdisplay: %f\n",i,j,data->routes[i].points[j].zdisplay);
				}
			for (j=0;j<data->routes[i].npoints-1;j++)
				{
				fprintf(stderr,"dbg2       route %d %d nls:          %d\n",i,j,data->routes[i].segments[j].nls);
				fprintf(stderr,"dbg2       route %d %d nls_alloc:    %d\n",i,j,data->routes[i].segments[j].nls_alloc);
				fprintf(stderr,"dbg2       route %d %d endpoints[0]: %d\n",i,j,data->routes[i].segments[j].endpoints[0]);
				fprintf(stderr,"dbg2       route %d %d endpoints[1]: %d\n",i,j,data->routes[i].segments[j].endpoints[1]);
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
	if (data->route_mode == MBV_ROUTE_EDIT
		&& (which == MBV_PICK_DOWN 
			|| (which == MBV_PICK_MOVE
				&& data->route_selected == MBV_SELECT_NONE)))
		{
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, 
				&found, 
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);

		/* add route if necessary */
		if (found && data->route_selected == MBV_SELECT_NONE)
			{
			/* add route point after currently selected route point if any */
			inew = data->nroute;
			jnew = 0;
			
			/* add the route point */
			mbview_route_add(instance, inew, jnew,
				xgrid, ygrid,
				xlon, ylat, zdata,
				xdisplay, ydisplay, zdisplay);
			
			/* select the new route */
			data->route_selected = inew;
			data->route_point_selected = jnew;
			}

		/* else just add point to currently selected route and point */
		else if (found && data->route_selected != MBV_SELECT_NONE)
			{
			/* add route point after currently selected route point if any */
			inew = data->route_selected;
			jnew = data->route_point_selected + 1;
			
			/* add the route point */
			mbview_route_add(instance, inew, jnew,
				xgrid, ygrid,
				xlon, ylat, zdata,
				xdisplay, ydisplay, zdisplay);
			
			
			/* select the new route */
			data->route_selected = inew;
			data->route_point_selected = jnew;
			}

		else
			{
			/* deselect the new route */
			data->route_selected = MBV_SELECT_NONE;
			data->route_point_selected = MBV_SELECT_NONE;
			XBell(view->dpy,100);
			}
		}
	
	/* only move selected routes if enabled */
	else if (data->route_mode != MBV_ROUTE_OFF
		&& data->nroute > 0
		&& (which == MBV_PICK_MOVE
			&& data->route_selected != MBV_SELECT_NONE))
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
			data->routes[data->route_selected].points[data->route_point_selected].xgrid = xgrid;
			data->routes[data->route_selected].points[data->route_point_selected].ygrid = ygrid;
			data->routes[data->route_selected].points[data->route_point_selected].xlon = xlon;
			data->routes[data->route_selected].points[data->route_point_selected].ylat = ylat;
			data->routes[data->route_selected].points[data->route_point_selected].zdata = zdata;
			data->routes[data->route_selected].points[data->route_point_selected].xdisplay = xdisplay;
			data->routes[data->route_selected].points[data->route_point_selected].ydisplay = ydisplay;
			data->routes[data->route_selected].points[data->route_point_selected].zdisplay = zdisplay;
			
			/* drape the affected segments */
			if (data->route_point_selected > 0)
				{
				/* drape the segment */
				mbview_drapesegment(instance, 
					&(data->routes[data->route_selected].segments[data->route_point_selected-1]));
				}
			if (data->route_point_selected < data->routes[data->route_selected].npoints - 1)
				{
				/* drape the segment */
				mbview_drapesegment(instance, 
					&(data->routes[data->route_selected].segments[data->route_point_selected]));
				}
			}
		}

	/* else beep */
	else
		{
		data->route_selected = MBV_SELECT_NONE;
		data->route_point_selected = MBV_SELECT_NONE;
		XBell(view->dpy,100);
		}
		
	/* set what kind of pick to annotate */
	if (data->route_selected != MBV_SELECT_NONE)
		{
		data->pickinfo_mode = MBV_PICK_ROUTE;
		}
	else
		{
		data->pickinfo_mode = data->pick_type;
		}
		
	/* set pick annotation */
	mbview_pick_text(instance);
	
	/* print route debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Route data altered in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Route values:\n");
		fprintf(stderr,"dbg2       route_view_mode:      %d\n",data->route_view_mode);
		fprintf(stderr,"dbg2       route_mode:           %d\n",data->route_mode);
		fprintf(stderr,"dbg2       nroute:               %d\n",data->nroute);
		fprintf(stderr,"dbg2       nroute_alloc:         %d\n",data->nroute_alloc);
		fprintf(stderr,"dbg2       route_selected:       %d\n",data->route_selected);
		fprintf(stderr,"dbg2       route_point_selected: %d\n",data->route_point_selected);
		for (i=0;i<data->nroute;i++)
			{
			fprintf(stderr,"dbg2       route %d color:         %d\n",i,data->routes[i].color);
			fprintf(stderr,"dbg2       route %d size:          %d\n",i,data->routes[i].size);
			fprintf(stderr,"dbg2       route %d name:          %s\n",i,data->routes[i].name);
			fprintf(stderr,"dbg2       route %d npoints:       %d\n",i,data->routes[i].npoints);
			fprintf(stderr,"dbg2       route %d npoints_alloc: %d\n",i,data->routes[i].npoints_alloc);
			for (j=0;j<data->routes[i].npoints;j++)
				{
				fprintf(stderr,"dbg2       route %d %d xgrid:    %f\n",i,j,data->routes[i].points[j].xgrid);
				fprintf(stderr,"dbg2       route %d %d ygrid:    %f\n",i,j,data->routes[i].points[j].ygrid);
				fprintf(stderr,"dbg2       route %d %d xlon:     %f\n",i,j,data->routes[i].points[j].xlon);
				fprintf(stderr,"dbg2       route %d %d ylat:     %f\n",i,j,data->routes[i].points[j].ylat);
				fprintf(stderr,"dbg2       route %d %d zdata:    %f\n",i,j,data->routes[i].points[j].zdata);
				fprintf(stderr,"dbg2       route %d %d xdisplay: %f\n",i,j,data->routes[i].points[j].xdisplay);
				fprintf(stderr,"dbg2       route %d %d ydisplay: %f\n",i,j,data->routes[i].points[j].ydisplay);
				fprintf(stderr,"dbg2       route %d %d zdisplay: %f\n",i,j,data->routes[i].points[j].zdisplay);
				}
			for (j=0;j<data->routes[i].npoints-1;j++)
				{
				fprintf(stderr,"dbg2       route %d %d nls:          %d\n",i,j,data->routes[i].segments[j].nls);
				fprintf(stderr,"dbg2       route %d %d nls_alloc:    %d\n",i,j,data->routes[i].segments[j].nls_alloc);
				fprintf(stderr,"dbg2       route %d %d endpoints[0]: %d\n",i,j,data->routes[i].segments[j].endpoints[0]);
				fprintf(stderr,"dbg2       route %d %d endpoints[1]: %d\n",i,j,data->routes[i].segments[j].endpoints[1]);
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
	if (data->route_mode == MBV_ROUTE_EDIT
		&& data->route_selected != MBV_SELECT_NONE)
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
			for (i=0;i<data->nroute;i++)
				{
				for (j=0;j<data->routes[i].npoints;j++)
					{
					xx = xgrid - data->routes[i].points[j].xgrid;
					yy = ygrid - data->routes[i].points[j].ygrid;
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
		if (found && data->route_selected == idelete
			&& data->route_point_selected == jdelete)
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
int mbview_route_add(int instance, int inew, int jnew, 
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
	if (inew == data->nroute)
		{
		/* allocate memory for a new route if required */
		if (data->nroute_alloc < data->nroute + 1)
			{
			data->nroute_alloc += MBV_ALLOC_NUM;
			status = mb_realloc(mbv_verbose, 
			    		data->nroute_alloc * sizeof(struct mbview_route_struct),
			    		&(data->routes), &error);
			if (status == MB_FAILURE)
				{
				data->nroute_alloc = 0;
				}
			else
				{
				for (i=data->nroute;i<data->nroute_alloc;i++)
					{
					data->routes[i].color = MBV_COLOR_RED;
					data->routes[i].size = 1;
					data->routes[i].name[0] = '\0';
					data->routes[i].npoints = 0;
					data->routes[i].npoints_alloc = MBV_ALLOC_NUM;
					data->routes[i].points = NULL;
					data->routes[i].segments = NULL;
					status = mb_realloc(mbv_verbose, 
			    				data->routes[i].npoints_alloc * sizeof(struct mbview_route_struct),
			    				&(data->routes[i].points), &error);
					status = mb_realloc(mbv_verbose, 
			    				data->routes[i].npoints_alloc * sizeof(struct mbview_linesegment_struct),
			    				&(data->routes[i].segments), &error);
					for (j=0;j<data->routes[i].npoints_alloc-1;j++)
						{
						data->routes[i].segments[j].nls = 0;
						data->routes[i].segments[j].nls_alloc = 0;
						data->routes[i].segments[j].lspoints = NULL;
						data->routes[i].segments[j].endpoints[0] = &(data->routes[i].points[j]);
						data->routes[i].segments[j].endpoints[1] = &(data->routes[i].points[j+1]);
						}
					}
				}
			}
			
		/* set nroute */
		data->nroute++;

		/* add the new route */
		data->routes[inew].color = MBV_COLOR_RED;
		data->routes[inew].size = 1;
		sprintf(data->routes[inew].name, "Route:%d", data->nroute);
		}

	/* allocate memory for point if required */
	if (status == MB_SUCCESS
		&& data->routes[inew].npoints_alloc < data->routes[inew].npoints + 1)
		{
		data->routes[inew].npoints_alloc += MBV_ALLOC_NUM;
		status = mb_realloc(mbv_verbose, 
			    	data->routes[inew].npoints_alloc * sizeof(struct mbview_point_struct),
			    	&(data->routes[inew].points), &error);
		status = mb_realloc(mbv_verbose, 
			    	data->routes[inew].npoints_alloc * sizeof(struct mbview_linesegment_struct),
			    	&(data->routes[inew].segments), &error);
		if (status == MB_FAILURE)
			{
			data->routes[inew].npoints = 0;
			data->routes[inew].npoints_alloc = 0;
			}
		}

	/* add the new route point */
	if (status == MB_SUCCESS)
		{
		/* move points after jnew if necessary */
		for (j=data->routes[inew].npoints;j>jnew;j--)
			{
			data->routes[inew].points[j].xgrid = data->routes[inew].points[j-1].xgrid;
			data->routes[inew].points[j].ygrid = data->routes[inew].points[j-1].ygrid;
			data->routes[inew].points[j].xlon = data->routes[inew].points[j-1].xlon;
			data->routes[inew].points[j].ylat = data->routes[inew].points[j-1].ylat;
			data->routes[inew].points[j].zdata = data->routes[inew].points[j-1].zdata;
			data->routes[inew].points[j].xdisplay = data->routes[inew].points[j-1].xdisplay;
			data->routes[inew].points[j].ydisplay = data->routes[inew].points[j-1].ydisplay;
			data->routes[inew].points[j].zdisplay = data->routes[inew].points[j-1].zdisplay;
			}

		/* move segments after jnew if necessary */
		for (j=data->routes[inew].npoints-1;j>jnew;j--)
			{
			data->routes[inew].segments[j].nls = data->routes[inew].segments[j-1].nls;
			data->routes[inew].segments[j].nls_alloc = data->routes[inew].segments[j-1].nls_alloc;
			data->routes[inew].segments[j].lspoints = data->routes[inew].segments[j-1].lspoints;
			data->routes[inew].segments[j].endpoints[0] = &(data->routes[inew].points[j]);
			data->routes[inew].segments[j].endpoints[1] = &(data->routes[inew].points[j+1]);
			}
		
		/* add the new point */
		data->routes[inew].points[jnew].xgrid = xgrid;
		data->routes[inew].points[jnew].ygrid = ygrid;
		data->routes[inew].points[jnew].xlon = xlon;
		data->routes[inew].points[jnew].ylat = ylat;
		data->routes[inew].points[jnew].zdata = zdata;
		data->routes[inew].points[jnew].xdisplay = xdisplay;
		data->routes[inew].points[jnew].ydisplay = ydisplay;
		data->routes[inew].points[jnew].zdisplay = zdisplay;
		data->routes[inew].segments[jnew].nls = 0;
		data->routes[inew].segments[jnew].nls_alloc = 0;
		data->routes[inew].segments[jnew].lspoints = NULL;
		data->routes[inew].segments[jnew].endpoints[0] = &(data->routes[inew].points[jnew]);
		data->routes[inew].segments[jnew].endpoints[1] = &(data->routes[inew].points[jnew+1]);
		if (jnew > 0)
			{
			data->routes[inew].segments[jnew-1].endpoints[0] = &(data->routes[inew].points[jnew-1]);
			data->routes[inew].segments[jnew-1].endpoints[1] = &(data->routes[inew].points[jnew]);
			}

		/* set npoints */
		data->routes[inew].npoints++;

		/* drape the affected segments */
		if (jnew > 0)
			{
			/* drape the segment */
			mbview_drapesegment(instance, &(data->routes[inew].segments[jnew-1]));
			}
		if (jnew < data->routes[inew].npoints - 1)
			{
			/* drape the segment */
			mbview_drapesegment(instance, &(data->routes[inew].segments[jnew]));
			}
		}

	
	/* print route debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Route data altered in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Route values:\n");
		fprintf(stderr,"dbg2       route_view_mode:      %d\n",data->route_view_mode);
		fprintf(stderr,"dbg2       route_mode:           %d\n",data->route_mode);
		fprintf(stderr,"dbg2       nroute:               %d\n",data->nroute);
		fprintf(stderr,"dbg2       nroute_alloc:         %d\n",data->nroute_alloc);
		fprintf(stderr,"dbg2       route_selected:       %d\n",data->route_selected);
		fprintf(stderr,"dbg2       route_point_selected: %d\n",data->route_point_selected);
		for (i=0;i<data->nroute;i++)
			{
			fprintf(stderr,"dbg2       route %d color:         %d\n",i,data->routes[i].color);
			fprintf(stderr,"dbg2       route %d size:          %d\n",i,data->routes[i].size);
			fprintf(stderr,"dbg2       route %d name:          %s\n",i,data->routes[i].name);
			fprintf(stderr,"dbg2       route %d npoints:       %d\n",i,data->routes[i].npoints);
			fprintf(stderr,"dbg2       route %d npoints_alloc: %d\n",i,data->routes[i].npoints_alloc);
			for (j=0;j<data->routes[i].npoints;j++)
				{
				fprintf(stderr,"dbg2       route %d %d xgrid:    %f\n",i,j,data->routes[i].points[j].xgrid);
				fprintf(stderr,"dbg2       route %d %d ygrid:    %f\n",i,j,data->routes[i].points[j].ygrid);
				fprintf(stderr,"dbg2       route %d %d xlon:     %f\n",i,j,data->routes[i].points[j].xlon);
				fprintf(stderr,"dbg2       route %d %d ylat:     %f\n",i,j,data->routes[i].points[j].ylat);
				fprintf(stderr,"dbg2       route %d %d zdata:    %f\n",i,j,data->routes[i].points[j].zdata);
				fprintf(stderr,"dbg2       route %d %d xdisplay: %f\n",i,j,data->routes[i].points[j].xdisplay);
				fprintf(stderr,"dbg2       route %d %d ydisplay: %f\n",i,j,data->routes[i].points[j].ydisplay);
				fprintf(stderr,"dbg2       route %d %d zdisplay: %f\n",i,j,data->routes[i].points[j].zdisplay);
				}
			for (j=0;j<data->routes[i].npoints-1;j++)
				{
				fprintf(stderr,"dbg2       route %d %d nls:          %d\n",i,j,data->routes[i].segments[j].nls);
				fprintf(stderr,"dbg2       route %d %d nls_alloc:    %d\n",i,j,data->routes[i].segments[j].nls_alloc);
				fprintf(stderr,"dbg2       route %d %d endpoints[0]: %f\n",i,j,data->routes[i].segments[j].endpoints[0]);
				fprintf(stderr,"dbg2       route %d %d endpoints[1]: %d\n",i,j,data->routes[i].segments[j].endpoints[1]);
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
int mbview_route_delete(int instance, int idelete, int jdelete)
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
		fprintf(stderr,"dbg2       idelete:          %d\n",idelete);
		fprintf(stderr,"dbg2       jdelete:          %d\n",jdelete);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);

	/* delete route point if its valid */
	if (idelete >= 0 && idelete < data->nroute
		&& jdelete >= 0 && jdelete < data->routes[idelete].npoints)
		{
		/* free segment */
		if (data->routes[idelete].npoints > 1)
			{
			if (jdelete == 0)
				{
				mb_free(mbv_verbose,&(data->routes[idelete].segments[jdelete].lspoints),&error);
				data->routes[idelete].segments[jdelete].nls = 0;
				data->routes[idelete].segments[jdelete].nls_alloc = 0;
				}
			else
				{
				mb_free(mbv_verbose,&(data->routes[idelete].segments[jdelete].lspoints),&error);
				data->routes[idelete].segments[jdelete].nls = 0;
				data->routes[idelete].segments[jdelete].nls_alloc = 0;
				}
			}

		/* move route point data if necessary */
		for (j=jdelete;j<data->routes[idelete].npoints-1;j++)
			{
			data->routes[idelete].points[j].xgrid = data->routes[idelete].points[j+1].xgrid;
			data->routes[idelete].points[j].ygrid = data->routes[idelete].points[j+1].ygrid;
			data->routes[idelete].points[j].xlon = data->routes[idelete].points[j+1].xlon;
			data->routes[idelete].points[j].ylat = data->routes[idelete].points[j+1].ylat;
			data->routes[idelete].points[j].zdata = data->routes[idelete].points[j+1].zdata;
			data->routes[idelete].points[j].xdisplay = data->routes[idelete].points[j+1].xdisplay;
			data->routes[idelete].points[j].ydisplay = data->routes[idelete].points[j+1].ydisplay;
			data->routes[idelete].points[j].zdisplay = data->routes[idelete].points[j+1].zdisplay;
			}

		/* move route segment data if necessary */
		for (j=jdelete;j<data->routes[idelete].npoints-2;j++)
			{
			data->routes[idelete].segments[j] = data->routes[idelete].segments[j+1];
			}

		/* decrement npoints */
		data->routes[idelete].npoints--;

		/* if route still has points then reset affected segment endpoints */
		if (data->routes[idelete].npoints > 0)
			{
			for (j=MAX(0,jdelete-1);j<MIN(data->routes[idelete].npoints-1,jdelete+1);j++)
				{
				data->routes[idelete].segments[j].endpoints[0] = &(data->routes[idelete].points[j]);
				data->routes[idelete].segments[j].endpoints[1] = &(data->routes[idelete].points[j+1]);
				mbview_drapesegment(instance, &(data->routes[idelete].segments[j]));
				}
			}

		/* if last point deleted then move remaining routes if necessary */
		if (data->routes[idelete].npoints <= 0)
			{
			/* move route data if necessary */
			for (i=idelete;i<data->nroute-1;i++)
				{
				data->routes[i] = data->routes[i+1];
				}

			/* decrement nroute */
			data->nroute--;
			}

		/* no route selection now */
		if (data->route_selected != MBV_SELECT_NONE)
			{
			data->route_selected = MBV_SELECT_NONE;
			data->route_point_selected = MBV_SELECT_NONE;
			data->pickinfo_mode = data->pick_type;
			}
		}

	/* else beep */
	else
		{
		XBell(view->dpy,100);
		}
		
	/* set pick annotation */
	mbview_pick_text(instance);
	
	/* print route debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Route data altered in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg2  Route values:\n");
		fprintf(stderr,"dbg2       route_view_mode:      %d\n",data->route_view_mode);
		fprintf(stderr,"dbg2       route_mode:           %d\n",data->route_mode);
		fprintf(stderr,"dbg2       nroute:               %d\n",data->nroute);
		fprintf(stderr,"dbg2       nroute_alloc:         %d\n",data->nroute_alloc);
		fprintf(stderr,"dbg2       route_selected:       %d\n",data->route_selected);
		fprintf(stderr,"dbg2       route_point_selected: %d\n",data->route_point_selected);
		for (i=0;i<data->nroute;i++)
			{
			fprintf(stderr,"dbg2       route %d color:         %d\n",i,data->routes[i].color);
			fprintf(stderr,"dbg2       route %d size:          %d\n",i,data->routes[i].size);
			fprintf(stderr,"dbg2       route %d name:          %s\n",i,data->routes[i].name);
			fprintf(stderr,"dbg2       route %d npoints:       %d\n",i,data->routes[i].npoints);
			fprintf(stderr,"dbg2       route %d npoints_alloc: %d\n",i,data->routes[i].npoints_alloc);
			for (j=0;j<data->routes[i].npoints;j++)
				{
				fprintf(stderr,"dbg2       route %d %d xgrid:    %f\n",i,j,data->routes[i].points[j].xgrid);
				fprintf(stderr,"dbg2       route %d %d ygrid:    %f\n",i,j,data->routes[i].points[j].ygrid);
				fprintf(stderr,"dbg2       route %d %d xlon:     %f\n",i,j,data->routes[i].points[j].xlon);
				fprintf(stderr,"dbg2       route %d %d ylat:     %f\n",i,j,data->routes[i].points[j].ylat);
				fprintf(stderr,"dbg2       route %d %d zdata:    %f\n",i,j,data->routes[i].points[j].zdata);
				fprintf(stderr,"dbg2       route %d %d xdisplay: %f\n",i,j,data->routes[i].points[j].xdisplay);
				fprintf(stderr,"dbg2       route %d %d ydisplay: %f\n",i,j,data->routes[i].points[j].ydisplay);
				fprintf(stderr,"dbg2       route %d %d zdisplay: %f\n",i,j,data->routes[i].points[j].zdisplay);
				}
			for (j=0;j<data->routes[i].npoints-1;j++)
				{
				fprintf(stderr,"dbg2       route %d %d nls:          %d\n",i,j,data->routes[i].segments[j].nls);
				fprintf(stderr,"dbg2       route %d %d nls_alloc:    %d\n",i,j,data->routes[i].segments[j].nls_alloc);
				fprintf(stderr,"dbg2       route %d %d endpoints[0]: %d\n",i,j,data->routes[i].segments[j].endpoints[0]);
				fprintf(stderr,"dbg2       route %d %d endpoints[1]: %d\n",i,j,data->routes[i].segments[j].endpoints[1]);
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
	if (data->route_mode != MBV_SITE_OFF
		&& data->display_mode == MBV_DISPLAY_2D
		&& data->route_view_mode == MBV_VIEW_ON
		&& data->nroute > 0)
		{
		/* set stride for looping over data */
		stride = 1;
		
		/* loop over the route points */
		for (iroute=0;iroute<data->nroute;iroute++)
			{
			for (jpoint=0;jpoint<data->routes[iroute].npoints;jpoint++)
				{
				/* get grid bounds for plotting */
				ix = (data->routes[iroute].points[jpoint].xgrid - data->primary_xmin) 
						/ data->primary_dx;
				jy = (data->routes[iroute].points[jpoint].ygrid - data->primary_ymin) 
						/ data->primary_dy;
				if (ix >= 0 && ix < data->primary_nx
					&& jy >= 0 && jy < data->primary_ny)
					{
					if (iroute == data->route_selected
						&& jpoint == data->route_point_selected)
						ixsize = MAX(data->primary_nx, data->primary_ny) / 200;
					else
						ixsize = MAX(data->primary_nx, data->primary_ny) / 300;
					if (ixsize < 1) ixsize = 1;
					jysize = (int)((ixsize * (1000.0 * data->primary_dy 
								/ data->primary_dx)) / 1000.0);
					if (ixsize < 1) ixsize = 1;
					ixmin = MAX(ix - ixsize, 0);
					ixmax = MIN(ix + ixsize, data->primary_nx - stride);
					jymin = MAX(jy - jysize, 0);
					jymax = MIN(jy + jysize, data->primary_ny - stride);

					/* set the color for this site */
					if (iroute == data->route_selected
						&& jpoint == data->route_point_selected)
						icolor = MBV_COLOR_RED;
					else
						icolor = MBV_COLOR_BLUE;
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
					glColor3f(colortable_object_red[MBV_COLOR_BLACK], 
						colortable_object_green[MBV_COLOR_BLACK], 
						colortable_object_blue[MBV_COLOR_BLACK]);
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
	else if (data->route_mode != MBV_SITE_OFF
		&& data->display_mode == MBV_DISPLAY_3D
		&& data->route_view_mode == MBV_VIEW_ON
		&& data->nroute > 0)
		{
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
		
		/* loop over the route points */
		for (iroute=0;iroute<data->nroute;iroute++)
			{
			for (jpoint=0;jpoint<data->routes[iroute].npoints;jpoint++)
				{
				/* get grid bounds for plotting */
				ix = (data->routes[iroute].points[jpoint].xgrid - data->primary_xmin) 
						/ data->primary_dx;
				jy = (data->routes[iroute].points[jpoint].ygrid - data->primary_ymin) 
						/ data->primary_dy;
				if (ix >= 0 && ix < data->primary_nx
					&& jy >= 0 && jy < data->primary_ny)
					{
					if (iroute == data->route_selected
						&& jpoint == data->route_point_selected)
						ixsize = MAX(data->primary_nx, data->primary_ny) / 200;
					else
						ixsize = MAX(data->primary_nx, data->primary_ny) / 300;
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
					if (iroute == data->route_selected
						&& jpoint == data->route_point_selected)
						icolor = MBV_COLOR_RED;
					else
						icolor = MBV_COLOR_BLUE;
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
					glColor3f(colortable_object_red[MBV_COLOR_BLACK], 
						colortable_object_green[MBV_COLOR_BLACK], 
						colortable_object_blue[MBV_COLOR_BLACK]);
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
	if (data->route_mode != MBV_SITE_OFF
		&& data->route_view_mode == MBV_VIEW_ON
		&& data->nroute > 0)
		{
		/* loop over the routes */
		for (iroute=0;iroute<data->nroute;iroute++)
			{
			glColor3f(0.0, 0.0, 0.0);
			glLineWidth((float)(2.0));
			glBegin(GL_LINE_STRIP);
			for (jpoint=0;jpoint<data->routes[iroute].npoints-1;jpoint++)
				{
				/* set size and color */
				if (iroute == data->route_selected
					&& (jpoint == data->route_point_selected
						|| jpoint == data->route_point_selected - 1))
					{
					glColor3f(colortable_object_red[MBV_COLOR_RED], 
						colortable_object_green[MBV_COLOR_RED], 
						colortable_object_blue[MBV_COLOR_RED]);
					}
				else
					{
					glColor3f(colortable_object_red[MBV_COLOR_BLACK], 
						colortable_object_green[MBV_COLOR_BLACK], 
						colortable_object_blue[MBV_COLOR_BLACK]);
					}
					
				/* draw draped segment */
				for (k=0;k<data->routes[iroute].segments[jpoint].nls;k++)
					{
					/* draw points */
					glVertex3f((float)(data->routes[iroute].segments[jpoint].lspoints[k].xdisplay), 
							(float)(data->routes[iroute].segments[jpoint].lspoints[k].ydisplay), 
							(float)(data->routes[iroute].segments[jpoint].lspoints[k].zdisplay));
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

