/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_pick.c	9/29/2003
 *    $Id: mbview_pick.c,v 5.1 2004-01-06 21:11:03 caress Exp $
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
 * Date:	September 29, 2003
 *
 * Note:	This code was broken out of mbview_callbacks.c, which was
 *		begun on October 7, 2002
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.0  2003/12/02 20:38:31  caress
 * Making version number 5.0
 *
 * Revision 1.2  2003/11/25 01:43:18  caress
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

static char rcs_id[]="$Id: mbview_pick.c,v 5.1 2004-01-06 21:11:03 caress Exp $";
	

/*------------------------------------------------------------------------------*/
int mbview_pick(int instance, int which, int xpixel, int ypixel)
{

	/* local variables */
	char	*function_name = "mbview_pick";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	found;
	double	xgrid, ygrid;
	double	xlon, ylat, zdata;
	double	xdisplay, ydisplay, zdisplay;
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
		fprintf(stderr,"dbg2       which:            %d\n",which);
		fprintf(stderr,"dbg2       xpixel:           %d\n",xpixel);
		fprintf(stderr,"dbg2       ypixel:           %d\n",ypixel);
		}
		
	/* get view */
	view = &(mbviews[instance]);
	data = &(view->data);
	
	/* look for point */
	mbview_findpoint(instance, xpixel, ypixel, 
			&found, 
			&xgrid, &ygrid,
			&xlon, &ylat, &zdata,
			&xdisplay, &ydisplay, &zdisplay);
		
	/* use any good pick */
	if (found == MB_YES)
		{
		if ((which == MBV_PICK_DOWN)
			|| (which == MBV_PICK_MOVE 
				&& data->pick_type == MBV_PICK_NONE))
			{
			/* set pick location */
			data->pickinfo_mode = MBV_PICK_ONEPOINT;
			data->pick_type = MBV_PICK_ONEPOINT;
			data->pick.endpoints[0].xgrid = xgrid;
			data->pick.endpoints[0].ygrid = ygrid;
			data->pick.endpoints[0].xlon = xlon;
			data->pick.endpoints[0].ylat = ylat;
			data->pick.endpoints[0].zdata = zdata;
			data->pick.endpoints[0].xdisplay = xdisplay;
			data->pick.endpoints[0].ydisplay = ydisplay;
			data->pick.endpoints[0].zdisplay = zdisplay;
			}
		else if (which == MBV_PICK_MOVE)
			{
			/* set pick location */
			data->pickinfo_mode = MBV_PICK_TWOPOINT;
			data->pick_type = MBV_PICK_TWOPOINT;
			data->pick.endpoints[1].xgrid = xgrid;
			data->pick.endpoints[1].ygrid = ygrid;
			data->pick.endpoints[1].xlon = xlon;
			data->pick.endpoints[1].ylat = ylat;
			data->pick.endpoints[1].zdata = zdata;
			data->pick.endpoints[1].xdisplay = xdisplay;
			data->pick.endpoints[1].ydisplay = ydisplay;
			data->pick.endpoints[1].zdisplay = zdisplay;
			}
			
		/* generate 3D drape of pick marks if either 3D display 
			or the pick move is final */
		if (data->pick_type != MBV_PICK_NONE
			&& (data->display_mode == MBV_DISPLAY_3D 
				|| which == MBV_PICK_UP))
			{
			mbview_picksize(instance);
			}
			
		/* if a two point pick has been made generate 3D drape 
			if either 3D display 
			or the pick move is final */
		if (data->pick_type == MBV_PICK_TWOPOINT
			&& (data->display_mode == MBV_DISPLAY_3D 
				|| which == MBV_PICK_UP))
			{
			mbview_drapesegment(instance, &(data->pick.segment));
			}
		}
	else
		{
		if (which == MBV_PICK_DOWN)
			{
			data->pickinfo_mode = MBV_PICK_NONE;
			data->pick_type = MBV_PICK_NONE;
			XBell(view->dpy,100);
			}
		else if (which == MBV_PICK_MOVE)
			{
			XBell(view->dpy,100);
			}
		}
		
	/* set pick annotation */
	mbview_pick_text(instance);
	
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
int mbview_picksize(int instance)
{

	/* local variables */
	char	*function_name = "mbview_picksize";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	double	xlength;
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

	/* resize and redrape pick marks if required */
	if (data->pickinfo_mode == MBV_PICK_ONEPOINT
		|| data->pickinfo_mode == MBV_PICK_TWOPOINT)
		{
		/* set size of 'X' marks in gl units for 3D case */
		xlength = 0.05;

		/* set pick location x marker */
		data->pick.xpoints[0].xdisplay = data->pick.endpoints[0].xdisplay - xlength;
		data->pick.xpoints[0].ydisplay = data->pick.endpoints[0].ydisplay - xlength;
		data->pick.xpoints[1].xdisplay = data->pick.endpoints[0].xdisplay + xlength;
		data->pick.xpoints[1].ydisplay = data->pick.endpoints[0].ydisplay + xlength;
		data->pick.xpoints[2].xdisplay = data->pick.endpoints[0].xdisplay - xlength;
		data->pick.xpoints[2].ydisplay = data->pick.endpoints[0].ydisplay + xlength;
		data->pick.xpoints[3].xdisplay = data->pick.endpoints[0].xdisplay + xlength;
		data->pick.xpoints[3].ydisplay = data->pick.endpoints[0].ydisplay - xlength;
		for (i=0;i<4;i++)
			{
			mbview_projectinverse(instance, MB_YES,
				data->pick.xpoints[i].xdisplay, 
				data->pick.xpoints[i].ydisplay,
				&data->pick.xpoints[i].xlon, 
				&data->pick.xpoints[i].ylat, 
				&data->pick.xpoints[i].xgrid, 
				&data->pick.xpoints[i].ygrid);
			mbview_getzdata(instance, 
				data->pick.xpoints[i].xgrid, 
				data->pick.xpoints[i].ygrid,
				&found, &data->pick.xpoints[i].zdata);
			if (found == MB_NO)
				data->pick.xpoints[i].zdata 
					= data->pick.endpoints[0].zdata;
			data->pick.xpoints[i].zdisplay = view->zscale 
				* (data->pick.xpoints[i].zdata - view->zorigin);
			}

		/* drape the x marker line segments */
		for (i=0;i<2;i++)
			{
			mbview_drapesegment(instance, &(data->pick.xsegments[i]));
			}
		}
	if (data->pickinfo_mode == MBV_PICK_TWOPOINT)
		{
		/* set pick location x marker */
		data->pick.xpoints[4].xdisplay = data->pick.endpoints[1].xdisplay - xlength;
		data->pick.xpoints[4].ydisplay = data->pick.endpoints[1].ydisplay - xlength;
		data->pick.xpoints[5].xdisplay = data->pick.endpoints[1].xdisplay + xlength;
		data->pick.xpoints[5].ydisplay = data->pick.endpoints[1].ydisplay + xlength;
		data->pick.xpoints[6].xdisplay = data->pick.endpoints[1].xdisplay - xlength;
		data->pick.xpoints[6].ydisplay = data->pick.endpoints[1].ydisplay + xlength;
		data->pick.xpoints[7].xdisplay = data->pick.endpoints[1].xdisplay + xlength;
		data->pick.xpoints[7].ydisplay = data->pick.endpoints[1].ydisplay - xlength;
		for (i=0;i<4;i++)
			{
			mbview_projectinverse(instance, MB_YES,
				data->pick.xpoints[i+4].xdisplay, 
				data->pick.xpoints[i+4].ydisplay,
				&data->pick.xpoints[i+4].xlon, 
				&data->pick.xpoints[i+4].ylat, 
				&data->pick.xpoints[i+4].xgrid, 
				&data->pick.xpoints[i+4].ygrid);
			mbview_getzdata(instance, 
				data->pick.xpoints[i+4].xgrid, 
				data->pick.xpoints[i+4].ygrid,
				&found, &data->pick.xpoints[i+4].zdata);
			if (found == MB_NO)
				data->pick.xpoints[i+4].zdata 
					= data->pick.endpoints[1].zdata;
			data->pick.xpoints[i].zdisplay = view->zscale 
				* (data->pick.xpoints[i+4].zdata - view->zorigin);
			}

		/* drape the x marker line segments */
		for (i=0;i<2;i++)
			{
			mbview_drapesegment(instance, &(data->pick.xsegments[i+2]));
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
int mbview_pick_text(int instance)
{

	/* local variables */
	char	*function_name = "mbview_pick_text";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	double	dx, dy, range, bearing;
	int	time_i[7];
	char	lonstr0[24], lonstr1[24];
	char	latstr0[24], latstr1[24];
	char	date0[24], date1[24];

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

	/* update pick info */
	if (data->pickinfo_mode == MBV_PICK_ONEPOINT)
		{
		mbview_setlonlatstrings(data->pick.endpoints[0].xlon, data->pick.endpoints[0].ylat, 
					lonstr0, latstr0);
		sprintf(value_text,":::t\"Pick Info:\":t\" Lon: %s\":t\" Lat: %s\":t\" Depth: %.3f m\"", 
			lonstr0, latstr0, data->pick.endpoints[0].zdata);
		}
	else if (data->pickinfo_mode == MBV_PICK_TWOPOINT)
		{
		mbview_setlonlatstrings(data->pick.endpoints[0].xlon, data->pick.endpoints[0].ylat, 
					lonstr0, latstr0);
		mbview_setlonlatstrings(data->pick.endpoints[1].xlon, data->pick.endpoints[1].ylat, 
					lonstr1, latstr1);
		dx = data->pick.endpoints[1].xdisplay
				- data->pick.endpoints[0].xdisplay;
		dy = data->pick.endpoints[1].ydisplay
				- data->pick.endpoints[0].ydisplay;
		range = sqrt(dx * dx + dy * dy) / view->scale ;
		bearing = RTD * atan2(dx, dy);
		sprintf(value_text,
		":::t\"Pick Info:\":t\" Lon 1: %s\":t\" Lat 1: %s\":t\" Depth 1: %.3f m\":t\" Lon 2: %s\":t\" Lat 2: %s\":t\" Depth 2: %.3f m\":t\" Bearing: %.1f deg\":t\" Distance: %.3f m\"", 
			lonstr0, latstr0,
			data->pick.endpoints[0].zdata,
			lonstr1, latstr1,
			data->pick.endpoints[1].zdata,
			bearing, range);
		}
	else if (data->pickinfo_mode == MBV_PICK_AREA)
		{
		sprintf(value_text,
		":::t\"Area Info:\":t\" Length: %.3f m\":t\" Width: %.3f m\":t\" Bearing: %.1f deg\"", 
			data->area.length,
			data->area.width,
			data->area.bearing);
		}
	else if (data->pickinfo_mode == MBV_PICK_REGION)
		{
		mbview_setlonlatstrings(data->region.cornerpoints[0].xlon, data->region.cornerpoints[0].ylat, 
					lonstr0, latstr0);
		mbview_setlonlatstrings(data->region.cornerpoints[3].xlon, data->region.cornerpoints[3].ylat, 
					lonstr1, latstr1);
		sprintf(value_text,
		":::t\"Region Info:\":t\" West: %s\":t\" North: %s\":t\" East: %s\":t\" South: %s\":t\" Width: %,3f m\":t\" Height: %.3f m\"", 
			lonstr0, latstr0, lonstr1, latstr1,
			data->region.width,
			data->region.height);
		}
	else if (data->pickinfo_mode == MBV_PICK_SITE)
		{
		mbview_setlonlatstrings(data->sites[data->site_selected].point.xlon, 
					data->sites[data->site_selected].point.ylat, 
					lonstr0, latstr0);
		sprintf(value_text,":::t\"Site %d Pick Info:\":t\" Lon: %s\":t\" Lat: %s\":t\" Depth: %.3f m\":t\" Color: %d\":t\" Size: %d\":t\" Name: %s\"", 
			data->site_selected, lonstr0, latstr0,
			data->sites[data->site_selected].point.zdata,
			data->sites[data->site_selected].color,
			data->sites[data->site_selected].size,
			data->sites[data->site_selected].name);
		}
	else if (data->pickinfo_mode == MBV_PICK_ROUTE)
		{
		mbview_setlonlatstrings(data->routes[data->route_selected].points[data->route_point_selected].xlon, 
					data->routes[data->route_selected].points[data->route_point_selected].ylat, 
					lonstr0, latstr0);
		sprintf(value_text,":::t\"Route %d Pick Info:\":t\" Point: %d\":t\" Lon: %s\":t\" Lat: %s\":t\" Depth: %.3f m\":t\" Color: %d\":t\" Size: %d\":t\" Name: %s\"", 
			data->route_selected,data->route_point_selected, 
			lonstr0, latstr0,
			data->routes[data->route_selected].points[data->route_point_selected].zdata,
			data->routes[data->route_selected].color,
			data->routes[data->route_selected].size,
			data->routes[data->route_selected].name);
		}
	else if (data->pickinfo_mode == MBV_PICK_NAV
		&& data->navpick_type == MBV_PICK_ONEPOINT)
		{
		mb_get_date(mbv_verbose,
				data->navs[data->nav_selected[0]].navpts[data->nav_point_selected[0]].time_d,
				time_i);
		sprintf(date0, "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%3.3d",
			time_i[0], time_i[1], time_i[2], 
			time_i[3], time_i[4], time_i[5], 
			(time_i[6] / 1000));
		mbview_setlonlatstrings(data->navs[data->nav_selected[0]].navpts[data->nav_point_selected[0]].point.xlon, 
					data->navs[data->nav_selected[0]].navpts[data->nav_point_selected[0]].point.ylat, 
					lonstr0, latstr0);
		sprintf(value_text,":::t\"Navigation Pick Info:\":t\" %s\":t\" %s\":t\" Lon: %s\":t\" Lat: %s\":t\" Vehicle Depth: %.3f m\":t\" Heading: %.1f deg\":t\" Speed: %.1f km/hr\"", 
			data->navs[data->nav_selected[0]].name, 
			date0, lonstr0, latstr0,
			data->navs[data->nav_selected[0]].navpts[data->nav_point_selected[0]].point.zdata,
			data->navs[data->nav_selected[0]].navpts[data->nav_point_selected[0]].heading,
			data->navs[data->nav_selected[0]].navpts[data->nav_point_selected[0]].speed);
		}
	else if (data->pickinfo_mode == MBV_PICK_NAV
		&& data->navpick_type == MBV_PICK_TWOPOINT)
		{
		mb_get_date(mbv_verbose,
				data->navs[data->nav_selected[0]].navpts[data->nav_point_selected[0]].time_d,
				time_i);
		sprintf(date0, "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%3.3d",
			time_i[0], time_i[1], time_i[2], 
			time_i[3], time_i[4], time_i[5], 
			(time_i[6] / 1000));
		mbview_setlonlatstrings(data->navs[data->nav_selected[0]].navpts[data->nav_point_selected[0]].point.xlon, 
					data->navs[data->nav_selected[0]].navpts[data->nav_point_selected[0]].point.ylat, 
					lonstr0, latstr0);
		mb_get_date(mbv_verbose,
				data->navs[data->nav_selected[1]].navpts[data->nav_point_selected[1]].time_d,
				time_i);
		sprintf(date1, "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%3.3d",
			time_i[0], time_i[1], time_i[2], 
			time_i[3], time_i[4], time_i[5], 
			(time_i[6] / 1000));
		mbview_setlonlatstrings(data->navs[data->nav_selected[1]].navpts[data->nav_point_selected[1]].point.xlon, 
					data->navs[data->nav_selected[1]].navpts[data->nav_point_selected[1]].point.ylat, 
					lonstr1, latstr1);
		sprintf(value_text,":::t\"Navigation Picks Info:\":t\" %s\":t\" %s\":t\" Lon: %s\":t\" Lat: %s\":t\" %s\":t\" %s\":t\" Lon: %s\":t\" Lat: %s\"", 
			data->navs[data->nav_selected[0]].name, date0, lonstr0, latstr0,
			data->navs[data->nav_selected[1]].name, date1, lonstr1, latstr1);
		}
	else
		{
		sprintf(value_text, ":::t\"Pick Info:\":t\"No Pick\"");
		}
	set_mbview_label_multiline_string(view->mb3dview.mbview_label_pickinfo, value_text);
	
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
int mbview_setlonlatstrings(double lon, double lat, char *lonstring, char *latstring)
{
	/* local variables */
	char	*function_name = "mbview_setlonlatstrings";
	int	status = MB_SUCCESS;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       lon:              %f\n",lon);
		fprintf(stderr,"dbg2       lat:              %f\n",lat);
		}
		
	/* set the strings */
	if (lon < 0.0)
		sprintf(lonstring, "%9.5f W", fabs(lon));
	else
		sprintf(lonstring, "%9.5f E", fabs(lon));
	if (lat < 0.0)
		sprintf(latstring, "%8.5f S", fabs(lat));
	else
		sprintf(latstring, "%8.5f N", fabs(lat));
	
	/* print output debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       lonstring:       %s\n",lonstring);
		fprintf(stderr,"dbg2       latstring:       %s\n",latstring);
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return */
	return(status);
}

/*------------------------------------------------------------------------------*/
int mbview_region(int instance, int which, int xpixel, int ypixel)
{

	/* local variables */
	char	*function_name = "mbview_region";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	found;
	double	xgrid, ygrid;
	double	xlon, ylat, zdata;
	double	xdisplay, ydisplay, zdisplay;
	double	dx, dy, dxuse, dyuse;
	int	ok;
	int	i, j, k;

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

	/* deal with start of new area */
	if (which == MBV_REGION_DOWN
		|| (which == MBV_REGION_MOVE)
			&& data->region_type == MBV_REGION_NONE)
		{
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, 
				&found, 
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);
				
		/* use any good point */
		if (found == MB_YES)
			{
			/* set the first endpoint */
			data->region_type = MBV_REGION_ONEPOINT;
			data->region.cornerpoints[0].xgrid = xgrid;
			data->region.cornerpoints[0].ygrid = ygrid;
			data->region.cornerpoints[0].xlon = xlon;
			data->region.cornerpoints[0].ylat = ylat;
			data->region.cornerpoints[0].zdata = zdata;
			data->region.cornerpoints[0].xdisplay = xdisplay;
			data->region.cornerpoints[0].ydisplay = ydisplay;
			data->region.cornerpoints[0].zdisplay = zdisplay;
			}
		}

	/* deal with definition or change of second endpoint */
	else if (which == MBV_REGION_MOVE)
		{
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, 
				&found, 
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);
				
		/* ignore an identical pair of points */
		if (found == MB_YES
			&& data->region.cornerpoints[0].xgrid
				== xgrid
			&& data->region.cornerpoints[0].ygrid
				== ygrid)
			{
			data->region_type = MBV_REGION_ONEPOINT;
			XBell(view->dpy,100);
			}
			
		/* use any good pair of points */
		else if (found == MB_YES)
			{
			/* set the second endpoint */
			data->region_type = MBV_REGION_QUAD;
			data->region.cornerpoints[3].xgrid = xgrid;
			data->region.cornerpoints[3].ygrid = ygrid;
			data->region.cornerpoints[3].xlon = xlon;
			data->region.cornerpoints[3].ylat = ylat;
			data->region.cornerpoints[3].zdata = zdata;
			data->region.cornerpoints[3].xdisplay = xdisplay;
			data->region.cornerpoints[3].ydisplay = ydisplay;
			data->region.cornerpoints[3].zdisplay = zdisplay;
			
			}
				
		/* ignore a bad point */
		else if (found == MB_NO)
			{
			XBell(view->dpy,100);
			}
		}

	/* recalculate any good quad region */
	if (data->region_type == MBV_REGION_QUAD
		&& which != MBV_REGION_UP)
		{
			
		/* now define the quad corners in grid coordinates */
		data->region.cornerpoints[1].xgrid
			= data->region.cornerpoints[0].xgrid;
		data->region.cornerpoints[1].ygrid 
			= data->region.cornerpoints[3].ygrid;
		mbview_projectforward(instance, MB_YES,
				data->region.cornerpoints[1].xgrid, 
				data->region.cornerpoints[1].ygrid,
				&(data->region.cornerpoints[1].xlon), 
				&(data->region.cornerpoints[1].ylat),
				&(data->region.cornerpoints[1].xdisplay), 
				&(data->region.cornerpoints[1].ydisplay));
		mbview_getzdata(instance, 
				data->region.cornerpoints[1].xgrid, 
				data->region.cornerpoints[1].ygrid, 
				&ok,
				&(data->region.cornerpoints[1].zdata));
		if (ok == MB_NO)
			data->region.cornerpoints[1].zdata
				= 0.5 * (data->region.cornerpoints[0].zdata
						+ data->region.cornerpoints[3].zdata);
		data->region.cornerpoints[1].zdisplay 
			= view->zscale 
				* (data->region.cornerpoints[1].zdata 
					- view->zorigin);

		data->region.cornerpoints[2].xgrid
			= data->region.cornerpoints[3].xgrid;
		data->region.cornerpoints[2].ygrid 
			= data->region.cornerpoints[0].ygrid;
		mbview_projectforward(instance, MB_YES,
				data->region.cornerpoints[2].xgrid, 
				data->region.cornerpoints[2].ygrid,
				&(data->region.cornerpoints[2].xlon), 
				&(data->region.cornerpoints[2].ylat),
				&(data->region.cornerpoints[2].xdisplay), 
				&(data->region.cornerpoints[2].ydisplay));
		mbview_projectforward(instance, MB_YES,
				data->region.cornerpoints[2].xgrid, 
				data->region.cornerpoints[2].ygrid,
				&(data->region.cornerpoints[2].xlon), 
				&(data->region.cornerpoints[2].ylat),
				&(data->region.cornerpoints[2].xdisplay), 
				&(data->region.cornerpoints[2].ydisplay));
		mbview_getzdata(instance, 
				data->region.cornerpoints[2].xgrid, 
				data->region.cornerpoints[2].ygrid, 
				&ok,
				&(data->region.cornerpoints[2].zdata));
		if (ok == MB_NO)
			data->region.cornerpoints[2].zdata
				= 0.5 * (data->region.cornerpoints[0].zdata
						+ data->region.cornerpoints[3].zdata);
		data->region.cornerpoints[2].zdisplay 
			= view->zscale 
				* (data->region.cornerpoints[2].zdata 
					- view->zorigin);
				
		/* calculate width and length */
		data->region.width = fabs(data->region.cornerpoints[3].xdisplay
						- data->region.cornerpoints[0].xdisplay) / view->scale;
		data->region.height = fabs(data->region.cornerpoints[3].ydisplay
						- data->region.cornerpoints[0].ydisplay) / view->scale;
		
		/* set pick info */
		data->pickinfo_mode = MBV_PICK_REGION;
		
		/* set pick annotation */
		mbview_pick_text(instance);
		}

	/* now set and drape the segments 
		if either 3D display 
		or the pick move is final  */
	if (data->region_type == MBV_REGION_QUAD
		&& (data->display_mode == MBV_DISPLAY_3D 
			|| which == MBV_REGION_UP))
		{
		for (i=0;i<4;i++)
			{
			/* drape the segment */
			mbview_drapesegment(instance, &(data->region.segments[i]));
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
int mbview_area(int instance, int which, int xpixel, int ypixel)
{

	/* local variables */
	char	*function_name = "mbview_area";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	found;
	double	xgrid, ygrid;
	double	xlon, ylat, zdata;
	double	xdisplay, ydisplay, zdisplay;
	double	dx, dy, dxuse, dyuse;
	int	ok;
	int	i, j, k;

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

	/* deal with start of new area */
	if (which == MBV_AREALENGTH_DOWN
		|| (which == MBV_AREALENGTH_MOVE)
			&& data->area_type == MBV_AREA_NONE)
		{
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, 
				&found, 
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);
				
		/* use any good point */
		if (found == MB_YES)
			{
			/* set the first endpoint */
			data->area_type = MBV_AREA_ONEPOINT;
			data->area.endpoints[0].xgrid = xgrid;
			data->area.endpoints[0].ygrid = ygrid;
			data->area.endpoints[0].xlon = xlon;
			data->area.endpoints[0].ylat = ylat;
			data->area.endpoints[0].zdata = zdata;
			data->area.endpoints[0].xdisplay = xdisplay;
			data->area.endpoints[0].ydisplay = ydisplay;
			data->area.endpoints[0].zdisplay = zdisplay;
			}
		}

	/* deal with definition or change of second endpoint */
	else if (which == MBV_AREALENGTH_MOVE)
		{
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, 
				&found, 
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);
				
		/* ignore an identical pair of points */
		if (found == MB_YES
			&& data->area.endpoints[0].xgrid == xgrid
			&& data->area.endpoints[0].ygrid == ygrid)
			{
			data->area_type = MBV_AREA_ONEPOINT;
			XBell(view->dpy,100);
			}
			
		/* use any good pair of points */
		else if (found == MB_YES)
			{
			/* set the second endpoint */
			data->area_type = MBV_AREA_QUAD;
			data->area.endpoints[1].xgrid = xgrid;
			data->area.endpoints[1].ygrid = ygrid;
			data->area.endpoints[1].xlon = xlon;
			data->area.endpoints[1].ylat = ylat;
			data->area.endpoints[1].zdata = zdata;
			data->area.endpoints[1].xdisplay = xdisplay;
			data->area.endpoints[1].ydisplay = ydisplay;
			data->area.endpoints[1].zdisplay = zdisplay;
			
			}
				
		/* ignore a bad point */
		else if (found == MB_NO)
			{
			XBell(view->dpy,100);
			}
		}

	/* recalculate any good quad area whether defined this time or previously
		this catches which == MBV_AREAASPECT_CHANGE calls */
	if (data->area_type == MBV_AREA_QUAD
		&& which != MBV_AREALENGTH_UP)
		{
			
		/* now define the quad corners in display coordinates */
		dx = data->area.endpoints[1].xdisplay 
			- data->area.endpoints[0].xdisplay;
		dy = data->area.endpoints[1].ydisplay 
			- data->area.endpoints[0].ydisplay;
		dxuse = 0.5 * view->areaaspect * dy;
		dyuse = 0.5 * view->areaaspect * dx;
		
		data->area.cornerpoints[0].xdisplay
			= data->area.endpoints[0].xdisplay
				- dxuse;
		data->area.cornerpoints[0].ydisplay 
			= data->area.endpoints[0].ydisplay
				+ dyuse;
		data->area.cornerpoints[1].xdisplay
			= data->area.endpoints[0].xdisplay
				+ dxuse;
		data->area.cornerpoints[1].ydisplay 
			= data->area.endpoints[0].ydisplay
				- dyuse;
		data->area.cornerpoints[2].xdisplay
			= data->area.endpoints[1].xdisplay
				+ dxuse;
		data->area.cornerpoints[2].ydisplay 
			= data->area.endpoints[1].ydisplay
				- dyuse;
		data->area.cornerpoints[3].xdisplay
			= data->area.endpoints[1].xdisplay
				- dxuse;
		data->area.cornerpoints[3].ydisplay 
			= data->area.endpoints[1].ydisplay
				+ dyuse;
				
		/* calculate width and length */
		data->area.length = sqrt(dx * dx + dy * dy) / view->scale;
		data->area.width = view->areaaspect * data->area.length;
		data->area.bearing = RTD * atan2(dx, dy);
		
		/* set pick info */
		data->pickinfo_mode = MBV_PICK_AREA;

		/* now project the segment endpoints */
		for (i=0;i<4;i++)
			{
			for (j=0;j<2;j++)
				{
				mbview_projectinverse(instance, MB_YES,
						data->area.segments[i].endpoints[j]->xdisplay, 
						data->area.segments[i].endpoints[j]->ydisplay, 
						&(data->area.segments[i].endpoints[j]->xlon), 
						&(data->area.segments[i].endpoints[j]->ylat),
						&(data->area.segments[i].endpoints[j]->xgrid), 
						&(data->area.segments[i].endpoints[j]->ygrid));
				mbview_getzdata(instance, 
						data->area.segments[i].endpoints[j]->xgrid, 
						data->area.segments[i].endpoints[j]->ygrid, 
						&ok,
						&(data->area.segments[i].endpoints[j]->zdata));
				if (ok == MB_NO &&
					(   (i == 0)
					 || (i == 1 && j == 0)
					 || (i == 3 && j == 1)))
					data->area.segments[i].endpoints[j]->zdata
						= data->area.endpoints[0].zdata;
				else if (ok == MB_NO)
					data->area.segments[i].endpoints[j]->zdata
						= data->area.endpoints[1].zdata;
				data->area.segments[i].endpoints[j]->zdisplay 
					= view->zscale 
						* (data->area.segments[i].endpoints[j]->zdata 
							- view->zorigin);
				}
			}
		
		/* set pick annotation */
		mbview_pick_text(instance);
		}

	/* now set and drape the segments 
		if either 3D display 
		or the pick move is final  */
	if (data->area_type == MBV_AREA_QUAD
		&& (data->display_mode == MBV_DISPLAY_3D 
			|| which == MBV_AREALENGTH_UP))
		{
		mbview_drapesegment(instance, &(data->area.segment));
		for (i=0;i<4;i++)
			{
			/* drape the segment */
			mbview_drapesegment(instance, &(data->area.segments[i]));
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
int mbview_drawpick(int instance)
{
	/* local variables */
	char	*function_name = "mbview_drawpick";
	int	status = MB_SUCCESS;
	int	i, j;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	double	xlength;

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
		
	/* draw current pick */
	if (data->pick_type != MBV_PICK_NONE)
		{
		/* set size of X mark for 2D case */
		if (data->display_mode == MBV_DISPLAY_2D)
			xlength = 0.05 / view->size2d;

		/* set color and linewidth */
		glColor3f(1.0, 0.0, 0.0);
		glLineWidth(3.0);
		glBegin(GL_LINE);
		
		/* plot first pick point */
		if (data->display_mode == MBV_DISPLAY_3D 
			&& data->pick.xsegments[0].nls > 0 
			&& data->pick.xsegments[1].nls > 0)
			{
			glBegin(GL_LINE_STRIP);
			for (i=0;i<data->pick.xsegments[0].nls;i++)
				{
				glVertex3f((float)(data->pick.xsegments[0].lspoints[i].xdisplay), 
						(float)(data->pick.xsegments[0].lspoints[i].ydisplay), 
						(float)(data->pick.xsegments[0].lspoints[i].zdisplay));
				}
			glEnd();
			glBegin(GL_LINE_STRIP);
			for (i=0;i<data->pick.xsegments[1].nls;i++)
				{
				glVertex3f((float)(data->pick.xsegments[1].lspoints[i].xdisplay), 
						(float)(data->pick.xsegments[1].lspoints[i].ydisplay), 
						(float)(data->pick.xsegments[1].lspoints[i].zdisplay));
				}
			glEnd();
			}
		else if (data->display_mode == MBV_DISPLAY_3D)
			{
			glBegin(GL_LINES);
			for (i=0;i<4;i++)
				{
				glVertex3f((float)(data->pick.xpoints[i].xdisplay), 
					(float)(data->pick.xpoints[i].ydisplay), 
					(float)(data->pick.xpoints[i].zdisplay));
				}
			glEnd();
			}
		else
			{
			glBegin(GL_LINES);
			glVertex3f((float)(data->pick.endpoints[0].xdisplay - xlength), 
				(float)(data->pick.endpoints[0].ydisplay - xlength), 
				(float)(data->pick.endpoints[0].zdisplay));
			glVertex3f((float)(data->pick.endpoints[0].xdisplay + xlength), 
				(float)(data->pick.endpoints[0].ydisplay + xlength), 
				(float)(data->pick.endpoints[0].zdisplay));
			glVertex3f((float)(data->pick.endpoints[0].xdisplay + xlength), 
				(float)(data->pick.endpoints[0].ydisplay - xlength), 
				(float)(data->pick.endpoints[0].zdisplay));
			glVertex3f((float)(data->pick.endpoints[0].xdisplay - xlength), 
				(float)(data->pick.endpoints[0].ydisplay + xlength), 
				(float)(data->pick.endpoints[0].zdisplay));
			glEnd();
			}
		
		if (data->pick_type == MBV_PICK_TWOPOINT)
			{
			/* plot second pick point */
			if (data->display_mode == MBV_DISPLAY_3D 
				&& data->pick.xsegments[2].nls > 0 
				&& data->pick.xsegments[3].nls > 0)
				{
				glBegin(GL_LINE_STRIP);
				for (i=0;i<data->pick.xsegments[2].nls;i++)
					{
					glVertex3f((float)(data->pick.xsegments[2].lspoints[i].xdisplay), 
							(float)(data->pick.xsegments[2].lspoints[i].ydisplay), 
							(float)(data->pick.xsegments[2].lspoints[i].zdisplay));
					}
				glEnd();
				glBegin(GL_LINE_STRIP);
				for (i=0;i<data->pick.xsegments[3].nls;i++)
					{
					glVertex3f((float)(data->pick.xsegments[3].lspoints[i].xdisplay), 
							(float)(data->pick.xsegments[3].lspoints[i].ydisplay), 
							(float)(data->pick.xsegments[3].lspoints[i].zdisplay));
					}
				glEnd();
				}
			else if (data->display_mode == MBV_DISPLAY_3D)
				{
				glBegin(GL_LINES);
				for (i=4;i<8;i++)
					{
					glVertex3f((float)(data->pick.xpoints[i].xdisplay), 
						(float)(data->pick.xpoints[i].ydisplay), 
						(float)(data->pick.xpoints[i].zdisplay));
					}
				glEnd();
				}
			else
				{
				glBegin(GL_LINES);
				glVertex3f((float)(data->pick.endpoints[1].xdisplay - xlength), 
					(float)(data->pick.endpoints[1].ydisplay - xlength), 
					(float)(data->pick.endpoints[1].zdisplay));
				glVertex3f((float)(data->pick.endpoints[1].xdisplay + xlength), 
					(float)(data->pick.endpoints[1].ydisplay + xlength), 
					(float)(data->pick.endpoints[1].zdisplay));
				glVertex3f((float)(data->pick.endpoints[1].xdisplay + xlength), 
					(float)(data->pick.endpoints[1].ydisplay - xlength), 
					(float)(data->pick.endpoints[1].zdisplay));
				glVertex3f((float)(data->pick.endpoints[1].xdisplay - xlength), 
					(float)(data->pick.endpoints[1].ydisplay + xlength), 
					(float)(data->pick.endpoints[1].zdisplay));
				glEnd();
				}

			/* plot line segment between pick points */
			if (data->display_mode == MBV_DISPLAY_3D 
				&& data->pick.segment.nls > 0)
				{
				glBegin(GL_LINE_STRIP);
				for (i=0;i<data->pick.segment.nls;i++)
					{
					glVertex3f((float)(data->pick.segment.lspoints[i].xdisplay), 
							(float)(data->pick.segment.lspoints[i].ydisplay), 
							(float)(data->pick.segment.lspoints[i].zdisplay));
					}
				glEnd();
				}
			else
				{
				glBegin(GL_LINES);
				glVertex3f((float)(data->pick.endpoints[0].xdisplay), 
						(float)(data->pick.endpoints[0].ydisplay), 
						(float)(data->pick.endpoints[0].zdisplay));
				glVertex3f((float)(data->pick.endpoints[1].xdisplay), 
						(float)(data->pick.endpoints[1].ydisplay), 
						(float)(data->pick.endpoints[1].zdisplay));
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
int mbview_drawregion(int instance)
{
	/* local variables */
	char	*function_name = "mbview_drawregion";
	int	status = MB_SUCCESS;
	int	i, j;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

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
		
	/* draw current area */
	if (data->region_type == MBV_REGION_QUAD)
		{
		/* set color and linewidth */
		glColor3f(colortable_object_red[MBV_COLOR_YELLOW], 
				colortable_object_green[MBV_COLOR_YELLOW], 
				colortable_object_blue[MBV_COLOR_YELLOW]);
		glLineWidth(3.0);
				
		/* plot quad segments */
		for (i=0;i<4;i++)
			{
			if (data->display_mode == MBV_DISPLAY_3D 
				&& data->region.segments[i].nls > 2)
				{
fprintf(stderr,"PLOT segment %d: nls:%d\n", i, data->region.segments[i].nls);
				glBegin(GL_LINE_STRIP);
				for (j=0;j<data->region.segments[i].nls-1;j++)
					{
					glVertex3f((float)(data->region.segments[i].lspoints[j].xdisplay), 
							(float)(data->region.segments[i].lspoints[j].ydisplay), 
							(float)(data->region.segments[i].lspoints[j].zdisplay));
					}
				glEnd();
				}
			else
				{
fprintf(stderr,"PLOT segment %d: ignore nls:%d\n", i, data->region.segments[i].nls);
				glBegin(GL_LINES);
				glVertex3f((float)(data->region.segments[i].endpoints[0]->xdisplay), 
						(float)(data->region.segments[i].endpoints[0]->ydisplay), 
						(float)(data->region.segments[i].endpoints[0]->zdisplay));
				glVertex3f((float)(data->region.segments[i].endpoints[1]->xdisplay), 
						(float)(data->region.segments[i].endpoints[1]->ydisplay), 
						(float)(data->region.segments[i].endpoints[1]->zdisplay));
				glEnd();
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
int mbview_drawarea(int instance)
{
	/* local variables */
	char	*function_name = "mbview_drawarea";
	int	status = MB_SUCCESS;
	int	i, j;
	struct mbview_world_struct *view;
	struct mbview_struct *data;

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
		
	/* draw current area */
	if (data->area_type == MBV_AREA_QUAD)
		{
		/* set color and linewidth */
		glColor3f(colortable_object_red[MBV_COLOR_YELLOW], 
				colortable_object_green[MBV_COLOR_YELLOW], 
				colortable_object_blue[MBV_COLOR_YELLOW]);
		glLineWidth(3.0);
				
		/* plot center segment */
		if (data->display_mode == MBV_DISPLAY_3D 
				&& data->area.segment.nls > 2)
			{
			glBegin(GL_LINE_STRIP);
			for (j=0;j<data->area.segment.nls;j++)
				{
				glVertex3f((float)(data->area.segment.lspoints[j].xdisplay), 
						(float)(data->area.segment.lspoints[j].ydisplay), 
						(float)(data->area.segment.lspoints[j].zdisplay));
				}
			glEnd();
			}
		else
			{
			glBegin(GL_LINES);
			glVertex3f((float)(data->area.segment.endpoints[0]->xdisplay), 
					(float)(data->area.segment.endpoints[0]->ydisplay), 
					(float)(data->area.segment.endpoints[0]->zdisplay));
			glVertex3f((float)(data->area.segment.endpoints[1]->xdisplay), 
					(float)(data->area.segment.endpoints[1]->ydisplay), 
					(float)(data->area.segment.endpoints[1]->zdisplay));
			glEnd();
			}
				
		/* plot quad segments */
		for (i=0;i<4;i++)
			{
			if (data->display_mode == MBV_DISPLAY_3D 
				&& data->area.segments[i].nls > 2)
				{
				glBegin(GL_LINE_STRIP);
				for (j=0;j<data->area.segments[i].nls-1;j++)
					{
					glVertex3f((float)(data->area.segments[i].lspoints[j].xdisplay), 
							(float)(data->area.segments[i].lspoints[j].ydisplay), 
							(float)(data->area.segments[i].lspoints[j].zdisplay));
					}
				glEnd();
				}
			else
				{
				glBegin(GL_LINES);
				glVertex3f((float)(data->area.segments[i].endpoints[0]->xdisplay), 
						(float)(data->area.segments[i].endpoints[0]->ydisplay), 
						(float)(data->area.segments[i].endpoints[0]->zdisplay));
				glVertex3f((float)(data->area.segments[i].endpoints[1]->xdisplay), 
						(float)(data->area.segments[i].endpoints[1]->ydisplay), 
						(float)(data->area.segments[i].endpoints[1]->zdisplay));
				glEnd();
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
