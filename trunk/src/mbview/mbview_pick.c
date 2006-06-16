/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_pick.c	9/29/2003
 *    $Id: mbview_pick.c,v 5.12 2006-06-16 19:30:58 caress Exp $
 *
 *    Copyright (c) 2003, 2006 by
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
 * Revision 5.11  2006/04/26 22:06:39  caress
 * Improved profile view feature and enabled export of profile data.
 *
 * Revision 5.10  2006/04/11 19:17:04  caress
 * Added a profile capability.
 *
 * Revision 5.9  2006/01/24 19:21:32  caress
 * Version 5.0.8 beta.
 *
 * Revision 5.8  2005/11/05 01:11:47  caress
 * Much work over the past two months.
 *
 * Revision 5.7  2005/02/18 07:32:55  caress
 * Fixed nav display and button sensitivity.
 *
 * Revision 5.6  2005/02/08 22:37:42  caress
 * Heading towards 5.0.6 release.
 *
 * Revision 5.4  2004/07/27 19:50:28  caress
 * Improving route planning capability.
 *
 * Revision 5.3  2004/06/18 04:26:06  caress
 * June 17, 2004 update.
 *
 * Revision 5.2  2004/02/24 22:52:27  caress
 * Added spherical projection to MBview.
 *
 * Revision 5.1  2004/01/06 21:11:03  caress
 * Added pick region capability.
 *
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
static char		value_list[MB_PATH_MAXLINE];

static char rcs_id[]="$Id: mbview_pick.c,v 5.12 2006-06-16 19:30:58 caress Exp $";
	

/*------------------------------------------------------------------------------*/
int mbview_pick(int instance, int which, int xpixel, int ypixel)
{

	/* local variables */
	char	*function_name = "mbview_pick";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	int	found;
	double	xgrid, ygrid;
	double	xlon, ylat, zdata;
	double	xdisplay, ydisplay, zdisplay;
	double	dx, dy;
	int	npoints;
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
			
		/* calculate range and bearing */
		if (data->display_projection_mode != MBV_PROJECTION_SPHEROID)
			{
			dx = data->pick.endpoints[1].xdisplay
					- data->pick.endpoints[0].xdisplay;
			dy = data->pick.endpoints[1].ydisplay
					- data->pick.endpoints[0].ydisplay;
			data->pick.range = sqrt(dx * dx + dy * dy) / view->scale ;
			data->pick.bearing = RTD * atan2(dx, dy);
			}
		else
			{
			mbview_greatcircle_distbearing(instance, 
				data->pick.endpoints[0].xlon, 
				data->pick.endpoints[0].ylat, 
				data->pick.endpoints[1].xlon, 
				data->pick.endpoints[1].ylat, 
				&(data->pick.bearing), &(data->pick.range));
			}
		if (data->pick.bearing < 0.0)
			data->pick.bearing += 360.0;
			
		/* generate 3D drape of pick marks if either 3D display 
			or the pick move is final */
		if (data->pick_type != MBV_PICK_NONE
			&& (data->display_mode == MBV_DISPLAY_3D 
				|| which == MBV_PICK_UP))
			{
			mbview_picksize(instance);
			}
			
		/* if a two point pick has been made generate 3D drape 
			if either 3D display, the pick move is final 
			or the profile display is on */
		if (data->pick_type == MBV_PICK_TWOPOINT
			&& (data->display_mode == MBV_DISPLAY_3D 
				|| data->profile_view_mode == MBV_VIEW_ON
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
int mbview_extract_pick_profile(int instance)
{

	/* local variables */
	char	*function_name = "mbview_extract_pick_profile";
	int	status = MB_SUCCESS;
	int	error = MB_ERROR_NO_ERROR;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	double	dx, dy;
	int	npoints;
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
			
	/* if a two point pick has been made and the profile display
		is on or the pick is final, insert the draped
		segment into the profile data */
	if (data->pick_type == MBV_PICK_TWOPOINT)
		{
		data->profile.source = MBV_PROFILE_TWOPOINT;
		strcpy(data->profile.source_name, "Two point pick");
		data->profile.length = data->pick.range;
		npoints = MAX(2, data->pick.segment.nls);
		if (data->profile.npoints_alloc < npoints)
			{
			status = mbview_allocprofilepoints(mbv_verbose, 
					npoints, &(data->profile.points), &error);
			if (status == MB_SUCCESS)
				{
				data->profile.npoints_alloc = npoints;
				}
			else
				{
				data->profile.npoints_alloc = 0;
				}
			}
		if (npoints > 2 && data->profile.npoints_alloc >= npoints)
			{
			/* get the profile data */
			for (i=0;i<npoints;i++)
				{
				data->profile.points[i].boundary = MB_NO;
				data->profile.points[i].xgrid = data->pick.segment.lspoints[i].xgrid;
				data->profile.points[i].ygrid = data->pick.segment.lspoints[i].ygrid;
				data->profile.points[i].xlon = data->pick.segment.lspoints[i].xlon;
				data->profile.points[i].ylat = data->pick.segment.lspoints[i].ylat;
				data->profile.points[i].zdata = data->pick.segment.lspoints[i].zdata;
				data->profile.points[i].xdisplay = data->pick.segment.lspoints[i].xdisplay;
				data->profile.points[i].ydisplay = data->pick.segment.lspoints[i].ydisplay;
				if (i == 0)
					{
					data->profile.zmin = data->profile.points[i].zdata;
					data->profile.zmax = data->profile.points[i].zdata;
					data->profile.points[i].distance = 0.0;
					data->profile.points[i].distovertopo = 0.0;
					}
				else
					{
					data->profile.zmin = MIN(data->profile.zmin, data->profile.points[i].zdata);
					data->profile.zmax = MAX(data->profile.zmax, data->profile.points[i].zdata);
					if (data->display_projection_mode != MBV_PROJECTION_SPHEROID)
						{
						dx = data->profile.points[i].xdisplay
								- data->profile.points[i-1].xdisplay;
						dy = data->profile.points[i].ydisplay
								- data->profile.points[i-1].ydisplay;
						data->profile.points[i].distance = sqrt(dx * dx + dy * dy) / view->scale 
							+ data->profile.points[i-1].distance;
						}
					else
						{
						mbview_greatcircle_dist(instance, 
							data->profile.points[0].xlon, 
							data->profile.points[0].ylat, 
							data->profile.points[i].xlon, 
							data->profile.points[i].ylat, 
							&(data->profile.points[i].distance));
						}
					dy = (data->profile.points[i].zdata
						- data->profile.points[i-1].zdata);
					dx = (data->profile.points[i].distance
						- data->profile.points[i-1].distance);
					data->profile.points[i].distovertopo = data->profile.points[i-1].distovertopo
										+ sqrt(dy * dy + dx * dx);
					if (dx > 0.0)
						data->profile.points[i].slope = fabs(dy / dx);
					else
						data->profile.points[i].slope = 0.0;
					}
				data->profile.points[i].bearing = data->pick.bearing;
				if (i > 1)
					{
					dy = (data->profile.points[i].zdata
						- data->profile.points[i-2].zdata);
					dx = (data->profile.points[i].distance
						- data->profile.points[i-2].distance);
					if (dx > 0.0)
						data->profile.points[i-1].slope = fabs(dy / dx);
					else
						data->profile.points[i-1].slope = 0.0;
					}
				data->profile.points[i].navzdata = 0.0;
				data->profile.points[i].navtime_d = 0.0;
				}
			data->profile.points[0].boundary = MB_YES;
			data->profile.points[npoints-1].boundary = MB_YES;
			data->profile.npoints = npoints;
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
int mbview_picksize(int instance)
{

	/* local variables */
	char	*function_name = "mbview_picksize";
	int	status = MB_SUCCESS;
	struct mbview_world_struct *view;
	struct mbview_struct *data;
	double	scalefactor;
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
		scalefactor = MIN( ((double)(data->viewbounds[1] - data->viewbounds[0])) 
					/ ((double)data->primary_nx), 
				   ((double)(data->viewbounds[3] - data->viewbounds[2])) 
					/ ((double)data->primary_ny) );
		xlength = 0.05 * scalefactor;

		/* set pick location x marker */
		data->pick.xpoints[0].xdisplay = data->pick.endpoints[0].xdisplay - xlength;
		data->pick.xpoints[0].ydisplay = data->pick.endpoints[0].ydisplay - xlength;
		data->pick.xpoints[0].zdisplay = data->pick.endpoints[0].zdisplay;
		data->pick.xpoints[1].xdisplay = data->pick.endpoints[0].xdisplay + xlength;
		data->pick.xpoints[1].ydisplay = data->pick.endpoints[0].ydisplay + xlength;
		data->pick.xpoints[1].zdisplay = data->pick.endpoints[0].zdisplay;
		data->pick.xpoints[2].xdisplay = data->pick.endpoints[0].xdisplay - xlength;
		data->pick.xpoints[2].ydisplay = data->pick.endpoints[0].ydisplay + xlength;
		data->pick.xpoints[2].zdisplay = data->pick.endpoints[0].zdisplay;
		data->pick.xpoints[3].xdisplay = data->pick.endpoints[0].xdisplay + xlength;
		data->pick.xpoints[3].ydisplay = data->pick.endpoints[0].ydisplay - xlength;
		data->pick.xpoints[3].zdisplay = data->pick.endpoints[0].zdisplay;
		for (i=0;i<4;i++)
			{
			mbview_projectinverse(instance, MB_YES,
				data->pick.xpoints[i].xdisplay, 
				data->pick.xpoints[i].ydisplay,
				data->pick.xpoints[i].zdisplay,
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
			mbview_projectforward(instance, MB_YES,
				data->pick.xpoints[i].xgrid, 
				data->pick.xpoints[i].ygrid, 
				data->pick.xpoints[i].zdata,
				&(data->pick.xpoints[i].xlon), 
				&(data->pick.xpoints[i].ylat),
				&(data->pick.xpoints[i].xdisplay), 
				&(data->pick.xpoints[i].ydisplay), 
				&(data->pick.xpoints[i].zdisplay));
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
		data->pick.xpoints[4].zdisplay = data->pick.endpoints[1].zdisplay;
		data->pick.xpoints[5].xdisplay = data->pick.endpoints[1].xdisplay + xlength;
		data->pick.xpoints[5].ydisplay = data->pick.endpoints[1].ydisplay + xlength;
		data->pick.xpoints[5].zdisplay = data->pick.endpoints[1].zdisplay;
		data->pick.xpoints[6].xdisplay = data->pick.endpoints[1].xdisplay - xlength;
		data->pick.xpoints[6].ydisplay = data->pick.endpoints[1].ydisplay + xlength;
		data->pick.xpoints[6].zdisplay = data->pick.endpoints[1].zdisplay;
		data->pick.xpoints[7].xdisplay = data->pick.endpoints[1].xdisplay + xlength;
		data->pick.xpoints[7].ydisplay = data->pick.endpoints[1].ydisplay - xlength;
		data->pick.xpoints[7].zdisplay = data->pick.endpoints[1].zdisplay;
		for (i=0;i<4;i++)
			{
			mbview_projectinverse(instance, MB_YES,
				data->pick.xpoints[i+4].xdisplay, 
				data->pick.xpoints[i+4].ydisplay,
				data->pick.xpoints[i+4].zdisplay,
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
			mbview_projectforward(instance, MB_YES,
				data->pick.xpoints[i+4].xgrid, 
				data->pick.xpoints[i+4].ygrid, 
				data->pick.xpoints[i+4].zdata,
				&(data->pick.xpoints[i+4].xlon), 
				&(data->pick.xpoints[i+4].ylat),
				&(data->pick.xpoints[i+4].xdisplay), 
				&(data->pick.xpoints[i+4].ydisplay), 
				&(data->pick.xpoints[i+4].zdisplay));
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
		mbview_setlonlatstrings(shared.lonlatstyle, 
					data->pick.endpoints[0].xlon, data->pick.endpoints[0].ylat, 
					lonstr0, latstr0);
		sprintf(value_text,":::t\"Pick Info:\":t\" Lon: %s\":t\" Lat: %s\":t\" Depth: %.3f m\"", 
			lonstr0, latstr0, data->pick.endpoints[0].zdata);
		sprintf(value_list,"Pick Info: Lon: %s Lat: %s Depth: %.3f m", 
			lonstr0, latstr0, data->pick.endpoints[0].zdata);
		}
	else if (data->pickinfo_mode == MBV_PICK_TWOPOINT)
		{
		mbview_setlonlatstrings(shared.lonlatstyle, 
					data->pick.endpoints[0].xlon, data->pick.endpoints[0].ylat, 
					lonstr0, latstr0);
		mbview_setlonlatstrings(shared.lonlatstyle, 
					data->pick.endpoints[1].xlon, data->pick.endpoints[1].ylat, 
					lonstr1, latstr1);
		sprintf(value_text,
		":::t\"Pick Info:\":t\" Lon 1: %s\":t\" Lat 1: %s\":t\" Depth 1: %.3f m\":t\" Lon 2: %s\":t\" Lat 2: %s\":t\" Depth 2: %.3f m\":t\" Bearing: %.1f deg\":t\" Distance: %.3f m\"", 
			lonstr0, latstr0,
			data->pick.endpoints[0].zdata,
			lonstr1, latstr1,
			data->pick.endpoints[1].zdata,
			data->pick.bearing, data->pick.range);
		sprintf(value_list,
		"Pick Info: Lon 1: %s Lat 1: %s Depth 1: %.3f m Lon 2: %s Lat 2: %s Depth 2: %.3f m Bearing: %.1f deg Distance: %.3f m", 
			lonstr0, latstr0,
			data->pick.endpoints[0].zdata,
			lonstr1, latstr1,
			data->pick.endpoints[1].zdata,
			data->pick.bearing, data->pick.range);
		}
	else if (data->pickinfo_mode == MBV_PICK_AREA)
		{
		sprintf(value_text,
		":::t\"Area Info:\":t\" Length: %.3f m\":t\" Width: %.3f m\":t\" Bearing: %.1f deg\"", 
			data->area.length,
			data->area.width,
			data->area.bearing);
		sprintf(value_list,
		"Area Info: Length: %.3f m Width: %.3f m Bearing: %.1f deg", 
			data->area.length,
			data->area.width,
			data->area.bearing);
		}
	else if (data->pickinfo_mode == MBV_PICK_REGION)
		{
		mbview_setlonlatstrings(shared.lonlatstyle, 
					data->region.cornerpoints[0].xlon, data->region.cornerpoints[0].ylat, 
					lonstr0, latstr0);
		mbview_setlonlatstrings(shared.lonlatstyle, 
					data->region.cornerpoints[3].xlon, data->region.cornerpoints[3].ylat, 
					lonstr1, latstr1);
		sprintf(value_text,
		":::t\"Region Info:\":t\" West: %s\":t\" North: %s\":t\" East: %s\":t\" South: %s\":t\" Width: %.3f m\":t\" Height: %.3f m\"", 
			lonstr0, latstr0, lonstr1, latstr1,
			data->region.width,
			data->region.height);
		sprintf(value_list,
		"Region Info: West: %s North: %s East: %s South: %s Width: %.3f m Height: %.3f m", 
			lonstr0, latstr0, lonstr1, latstr1,
			data->region.width,
			data->region.height);
		}
	else if (data->pickinfo_mode == MBV_PICK_SITE
		&& shared.shareddata.site_selected != MBV_SELECT_NONE)
		{
		mbview_setlonlatstrings(shared.lonlatstyle, 
					shared.shareddata.sites[shared.shareddata.site_selected].point.xlon, 
					shared.shareddata.sites[shared.shareddata.site_selected].point.ylat, 
					lonstr0, latstr0);
		sprintf(value_text,":::t\"Site %d Pick Info:\":t\" Lon: %s\":t\" Lat: %s\":t\" Depth: %.3f m\":t\" Color: %d\":t\" Size: %d\":t\" Name: %s\"", 
			shared.shareddata.site_selected, lonstr0, latstr0,
			shared.shareddata.sites[shared.shareddata.site_selected].point.zdata,
			shared.shareddata.sites[shared.shareddata.site_selected].color,
			shared.shareddata.sites[shared.shareddata.site_selected].size,
			shared.shareddata.sites[shared.shareddata.site_selected].name);
		sprintf(value_list,"Site %d Pick Info: Lon: %s Lat: %s Depth: %.3f m Color: %d Size: %d Name: %s", 
			shared.shareddata.site_selected, lonstr0, latstr0,
			shared.shareddata.sites[shared.shareddata.site_selected].point.zdata,
			shared.shareddata.sites[shared.shareddata.site_selected].color,
			shared.shareddata.sites[shared.shareddata.site_selected].size,
			shared.shareddata.sites[shared.shareddata.site_selected].name);
		}
	else if (data->pickinfo_mode == MBV_PICK_ROUTE
		&& shared.shareddata.route_selected != MBV_SELECT_NONE
		&& shared.shareddata.route_point_selected != MBV_SELECT_NONE)
		{
		mbview_setlonlatstrings(shared.lonlatstyle, 
					shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].xlon, 
					shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].ylat, 
					lonstr0, latstr0);
		sprintf(value_text,":::t\"Route %d Pick Info:\":t\" Point: %d\":t\" Lon: %s\":t\" Lat: %s\":t\" Depth: %.3f m\":t\" Color: %d\":t\" Size: %d\":t\" Name: %s\"", 
			shared.shareddata.route_selected,shared.shareddata.route_point_selected, 
			lonstr0, latstr0,
			shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].zdata,
			shared.shareddata.routes[shared.shareddata.route_selected].color,
			shared.shareddata.routes[shared.shareddata.route_selected].size,
			shared.shareddata.routes[shared.shareddata.route_selected].name);
		sprintf(value_list,"Route %d Pick Info: Point: %d Lon: %s Lat: %s Depth: %.3f m Color: %d Size: %d Name: %s", 
			shared.shareddata.route_selected,shared.shareddata.route_point_selected, 
			lonstr0, latstr0,
			shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].zdata,
			shared.shareddata.routes[shared.shareddata.route_selected].color,
			shared.shareddata.routes[shared.shareddata.route_selected].size,
			shared.shareddata.routes[shared.shareddata.route_selected].name);
		}
	else if (data->pickinfo_mode == MBV_PICK_NAV
		&& shared.shareddata.navpick_type == MBV_PICK_ONEPOINT
		&& shared.shareddata.nav_selected[0] != MBV_SELECT_NONE)
		{
		mb_get_date(mbv_verbose,
				shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].time_d,
				time_i);
		sprintf(date0, "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%3.3d",
			time_i[0], time_i[1], time_i[2], 
			time_i[3], time_i[4], time_i[5], 
			(time_i[6] / 1000));
		mbview_setlonlatstrings(shared.lonlatstyle, 
					shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].point.xlon, 
					shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].point.ylat, 
					lonstr0, latstr0);
		sprintf(value_text,":::t\"Navigation Pick Info:\":t\" %s\":t\" %s\":t\" Lon: %s\":t\" Lat: %s\":t\" Vehicle Depth: %.3f m\":t\" Heading: %.1f deg\":t\" Speed: %.1f km/hr\"", 
			shared.shareddata.navs[shared.shareddata.nav_selected[0]].name, 
			date0, lonstr0, latstr0,
			shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].point.zdata,
			shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].heading,
			shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].speed);
		sprintf(value_list,"Navigation Pick Info: %s %s Lon: %s Lat: %s Vehicle Depth: %.3f m Heading: %.1f deg Speed: %.1f km/hr", 
			shared.shareddata.navs[shared.shareddata.nav_selected[0]].name, 
			date0, lonstr0, latstr0,
			shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].point.zdata,
			shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].heading,
			shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].speed);
		}
	else if (data->pickinfo_mode == MBV_PICK_NAV
		&& shared.shareddata.navpick_type == MBV_PICK_TWOPOINT
		&& shared.shareddata.nav_selected[0] != MBV_SELECT_NONE
		&& shared.shareddata.nav_selected[1] != MBV_SELECT_NONE)
		{
		mb_get_date(mbv_verbose,
				shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].time_d,
				time_i);
		sprintf(date0, "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%3.3d",
			time_i[0], time_i[1], time_i[2], 
			time_i[3], time_i[4], time_i[5], 
			(time_i[6] / 1000));
		mbview_setlonlatstrings(shared.lonlatstyle, 
					shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].point.xlon, 
					shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].point.ylat, 
					lonstr0, latstr0);
		mb_get_date(mbv_verbose,
				shared.shareddata.navs[shared.shareddata.nav_selected[1]].navpts[shared.shareddata.nav_point_selected[1]].time_d,
				time_i);
		sprintf(date1, "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%3.3d",
			time_i[0], time_i[1], time_i[2], 
			time_i[3], time_i[4], time_i[5], 
			(time_i[6] / 1000));
		mbview_setlonlatstrings(shared.lonlatstyle, 
					shared.shareddata.navs[shared.shareddata.nav_selected[1]].navpts[shared.shareddata.nav_point_selected[1]].point.xlon, 
					shared.shareddata.navs[shared.shareddata.nav_selected[1]].navpts[shared.shareddata.nav_point_selected[1]].point.ylat, 
					lonstr1, latstr1);
		sprintf(value_text,":::t\"Navigation Picks Info:\":t\" %s\":t\" %s\":t\" Lon: %s\":t\" Lat: %s\":t\" %s\":t\" %s\":t\" Lon: %s\":t\" Lat: %s\"", 
			shared.shareddata.navs[shared.shareddata.nav_selected[0]].name, date0, lonstr0, latstr0,
			shared.shareddata.navs[shared.shareddata.nav_selected[1]].name, date1, lonstr1, latstr1);
		sprintf(value_list,"Navigation Picks Info: %s %s Lon: %s Lat: %s %s %s Lon: %s Lat: %s", 
			shared.shareddata.navs[shared.shareddata.nav_selected[0]].name, date0, lonstr0, latstr0,
			shared.shareddata.navs[shared.shareddata.nav_selected[1]].name, date1, lonstr1, latstr1);
		}
/*	else
		{
		sprintf(value_text, ":::t\"Pick Info:\":t\"No Pick\"");
		sprintf(value_list, "Pick Info: No Pick\n");
		}*/
	set_mbview_label_multiline_string(view->mb3dview.mbview_label_pickinfo, value_text);
	fprintf(stderr,"%s\n", value_list);
	
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
int mbview_setlonlatstrings(int style, double lon, double lat, char *lonstring, char *latstring)
{
	/* local variables */
	char	*function_name = "mbview_setlonlatstrings";
	int	status = MB_SUCCESS;
	int	degree;
	double	minute;

	/* print starting debug statements */
	if (mbv_verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       style:            %d\n",style);
		fprintf(stderr,"dbg2       lon:              %f\n",lon);
		fprintf(stderr,"dbg2       lat:              %f\n",lat);
		}
		
	/* set the strings */
	if (lon > 180.0)
		{
		lon -= 360.0;
		}
	if (lon < -180.0)
		{
		lon += 360.0;
		}
	
	if (style == MBV_LONLAT_DECIMAL)
		{
		if (lon < 0.0)
			sprintf(lonstring, "%9.5f W", fabs(lon));
		else
			sprintf(lonstring, "%9.5f E", fabs(lon));
		if (lat < 0.0)
			sprintf(latstring, "%8.5f S", fabs(lat));
		else
			sprintf(latstring, "%8.5f N", fabs(lat));
		}
	else
		{
		degree = (int)fabs(lon);
		minute = 60.0 * (fabs(lon) - (double)degree);
		if (lon < 0.0)
			sprintf(lonstring, "%3d W %6.3f", degree, minute);
		else
			sprintf(lonstring, "%3d E %6.3f", degree, minute);
		degree = (int)fabs(lat);
		minute = 60.0 * (fabs(lat) - (double)degree);
		if (lat < 0.0)
			sprintf(latstring, "%3d S %6.3f", degree, minute);
		else
			sprintf(latstring, "%3d N %6.3f", degree, minute);
		}
	
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
	double	dd;
	int	ok;
	double	bearing;
	int	match, match0, match1, match2, match3;
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

	/* check to see if pick is at existing corner points */
	if (which == MBV_REGION_DOWN
		&& data->region_type == MBV_REGION_QUAD)
		{
		/* look for match to existing corner points in neighborhood of pick */
		match = MB_NO;
		match0 = MB_NO;
		match1 = MB_NO;
		match2 = MB_NO;
		match3 = MB_NO;

		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, 
				&found, 
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);

		dx = 0.10 * (data->region.cornerpoints[3].xdisplay - data->region.cornerpoints[0].xdisplay);
		dy = 0.10 * (data->region.cornerpoints[3].ydisplay - data->region.cornerpoints[0].ydisplay);
		dd = MAX(dx, dy);
		if (found == MB_YES)
			{
			if (fabs(xdisplay - data->region.cornerpoints[0].xdisplay) < dd
				&& fabs(ydisplay - data->region.cornerpoints[0].ydisplay) < dd)
				{
				match = MB_YES;
				match0 = MB_YES;
				}
			else if (fabs(xdisplay - data->region.cornerpoints[1].xdisplay) < dd
				&& fabs(ydisplay - data->region.cornerpoints[1].ydisplay) < dd)
				{
				match = MB_YES;
				match1 = MB_YES;
				}
			else if (fabs(xdisplay - data->region.cornerpoints[2].xdisplay) < dd
				&& fabs(ydisplay - data->region.cornerpoints[2].ydisplay) < dd)
				{
				match = MB_YES;
				match2 = MB_YES;
				}
			else if (fabs(xdisplay - data->region.cornerpoints[3].xdisplay) < dd
				&& fabs(ydisplay - data->region.cornerpoints[3].ydisplay) < dd)
				{
				match = MB_YES;
				match3 = MB_YES;
				}
			}
			
		/* if no match then start new region */
		if (match == MB_NO)
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
				data->region_pickcorner = MBV_REGION_PICKCORNER3;
				data->region.cornerpoints[0].xgrid = xgrid;
				data->region.cornerpoints[0].ygrid = ygrid;
				data->region.cornerpoints[0].xlon = xlon;
				data->region.cornerpoints[0].ylat = ylat;
				data->region.cornerpoints[0].zdata = zdata;
				data->region.cornerpoints[0].xdisplay = xdisplay;
				data->region.cornerpoints[0].ydisplay = ydisplay;
				data->region.cornerpoints[0].zdisplay = zdisplay;
/*fprintf(stderr,"PICK NEW REGION: corner0: xgrid:%f ygrid:%f xlon:%f ylat:%f zdata:%f display: %f %f %f\n",
xgrid,ygrid,xlon,ylat,zdata,xdisplay,ydisplay,zdisplay);*/
				}
			}
			
		/* else if match 0 then reset corner point 0 */
		else if (match0 == MB_YES)
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
				/* set corner point 0 */
				data->region_type = MBV_REGION_QUAD;
				data->region_pickcorner = MBV_REGION_PICKCORNER0;
				data->region.cornerpoints[0].xgrid = xgrid;
				data->region.cornerpoints[0].ygrid = ygrid;
				data->region.cornerpoints[0].xlon = xlon;
				data->region.cornerpoints[0].ylat = ylat;
				data->region.cornerpoints[0].zdata = zdata;
				data->region.cornerpoints[0].xdisplay = xdisplay;
				data->region.cornerpoints[0].ydisplay = ydisplay;
				data->region.cornerpoints[0].zdisplay = zdisplay;
/*fprintf(stderr,"PICK EXISTING REGION: corner0: xgrid:%f ygrid:%f xlon:%f ylat:%f zdata:%f display: %f %f %f\n",
xgrid,ygrid,xlon,ylat,zdata,xdisplay,ydisplay,zdisplay);*/
				}
			}
			
		/* else if match 1 then reset corner point 1 */
		else if (match1 == MB_YES)
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
				/* set corner point 1 */
				data->region_type = MBV_REGION_QUAD;
				data->region_pickcorner = MBV_REGION_PICKCORNER1;
				data->region.cornerpoints[1].xgrid = xgrid;
				data->region.cornerpoints[1].ygrid = ygrid;
				data->region.cornerpoints[1].xlon = xlon;
				data->region.cornerpoints[1].ylat = ylat;
				data->region.cornerpoints[1].zdata = zdata;
				data->region.cornerpoints[1].xdisplay = xdisplay;
				data->region.cornerpoints[1].ydisplay = ydisplay;
				data->region.cornerpoints[1].zdisplay = zdisplay;
/*fprintf(stderr,"PICK EXISTING REGION: corner1: xgrid:%f ygrid:%f xlon:%f ylat:%f zdata:%f display: %f %f %f\n",
xgrid,ygrid,xlon,ylat,zdata,xdisplay,ydisplay,zdisplay);*/
				}
			}
			
		/* else if match 2 then reset corner point 2 */
		else if (match2 == MB_YES)
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
				/* set corner point 2 */
				data->region_type = MBV_REGION_QUAD;
				data->region_pickcorner = MBV_REGION_PICKCORNER2;
				data->region.cornerpoints[2].xgrid = xgrid;
				data->region.cornerpoints[2].ygrid = ygrid;
				data->region.cornerpoints[2].xlon = xlon;
				data->region.cornerpoints[2].ylat = ylat;
				data->region.cornerpoints[2].zdata = zdata;
				data->region.cornerpoints[2].xdisplay = xdisplay;
				data->region.cornerpoints[2].ydisplay = ydisplay;
				data->region.cornerpoints[2].zdisplay = zdisplay;
/*fprintf(stderr,"PICK EXISTING REGION: corner2: xgrid:%f ygrid:%f xlon:%f ylat:%f zdata:%f display: %f %f %f\n",
xgrid,ygrid,xlon,ylat,zdata,xdisplay,ydisplay,zdisplay);*/
				}
			}
			
		/* else if match 3 then reset corner point 3 */
		else if (match3 == MB_YES)
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
				/* set corner point 3 */
				data->region_type = MBV_REGION_QUAD;
				data->region_pickcorner = MBV_REGION_PICKCORNER3;
				data->region.cornerpoints[3].xgrid = xgrid;
				data->region.cornerpoints[3].ygrid = ygrid;
				data->region.cornerpoints[3].xlon = xlon;
				data->region.cornerpoints[3].ylat = ylat;
				data->region.cornerpoints[3].zdata = zdata;
				data->region.cornerpoints[3].xdisplay = xdisplay;
				data->region.cornerpoints[3].ydisplay = ydisplay;
				data->region.cornerpoints[3].zdisplay = zdisplay;
/*fprintf(stderr,"PICK EXISTING REGION: corner3: xgrid:%f ygrid:%f xlon:%f ylat:%f zdata:%f display: %f %f %f\n",
xgrid,ygrid,xlon,ylat,zdata,xdisplay,ydisplay,zdisplay);*/
				}
			}
		}

	/* deal with start of new region */
	else if ((which == MBV_REGION_DOWN
			|| which == MBV_REGION_MOVE)
			&& data->region_type == MBV_REGION_NONE)
		{
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, 
				&found, 
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);
/*fprintf(stderr,"NEW REGION: corner0: xgrid:%f ygrid:%f xlon:%f ylat:%f zdata:%f display: %f %f %f\n",
xgrid,ygrid,xlon,ylat,zdata,xdisplay,ydisplay,zdisplay);*/
				
		/* use any good point */
		if (found == MB_YES)
			{
			/* set the first endpoint */
			data->region_type = MBV_REGION_ONEPOINT;
			data->region_pickcorner = MBV_REGION_PICKCORNER3;
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

	/* deal with definition or change of cornerpoint 0 */
	else if (which == MBV_REGION_MOVE
		&& data->region_pickcorner == MBV_REGION_PICKCORNER0)
		{
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, 
				&found, 
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);
/*fprintf(stderr,"CHANGE REGION: corner0: xgrid:%f ygrid:%f xlon:%f ylat:%f zdata:%f display: %f %f %f\n",
xgrid,ygrid,xlon,ylat,zdata,xdisplay,ydisplay,zdisplay);*/
				
		/* ignore an identical pair of points */
		if (found == MB_YES
			&& data->region.cornerpoints[3].xgrid
				== xgrid
			&& data->region.cornerpoints[3].ygrid
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
			data->region_pickcorner == MBV_REGION_PICKCORNER0;
			data->region.cornerpoints[0].xgrid = xgrid;
			data->region.cornerpoints[0].ygrid = ygrid;
			data->region.cornerpoints[0].xlon = xlon;
			data->region.cornerpoints[0].ylat = ylat;
			data->region.cornerpoints[0].zdata = zdata;
			data->region.cornerpoints[0].xdisplay = xdisplay;
			data->region.cornerpoints[0].ydisplay = ydisplay;
			data->region.cornerpoints[0].zdisplay = zdisplay;
			}
				
		/* ignore a bad point */
		else if (found == MB_NO)
			{
			XBell(view->dpy,100);
			}
		}

	/* deal with definition or change of cornerpoint 1 */
	else if (which == MBV_REGION_MOVE
		&& data->region_pickcorner == MBV_REGION_PICKCORNER1)
		{
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, 
				&found, 
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);
/*fprintf(stderr,"CHANGE REGION: corner1: xgrid:%f ygrid:%f xlon:%f ylat:%f zdata:%f display: %f %f %f\n",
xgrid,ygrid,xlon,ylat,zdata,xdisplay,ydisplay,zdisplay);*/
				
		/* ignore an identical pair of points */
		if (found == MB_YES
			&& data->region.cornerpoints[2].xgrid
				== xgrid
			&& data->region.cornerpoints[2].ygrid
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
			data->region_pickcorner == MBV_REGION_PICKCORNER1;
			data->region.cornerpoints[1].xgrid = xgrid;
			data->region.cornerpoints[1].ygrid = ygrid;
			data->region.cornerpoints[1].xlon = xlon;
			data->region.cornerpoints[1].ylat = ylat;
			data->region.cornerpoints[1].zdata = zdata;
			data->region.cornerpoints[1].xdisplay = xdisplay;
			data->region.cornerpoints[1].ydisplay = ydisplay;
			data->region.cornerpoints[1].zdisplay = zdisplay;
			}
				
		/* ignore a bad point */
		else if (found == MB_NO)
			{
			XBell(view->dpy,100);
			}
		}

	/* deal with definition or change of cornerpoint 2 */
	else if (which == MBV_REGION_MOVE
		&& data->region_pickcorner == MBV_REGION_PICKCORNER2)
		{
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, 
				&found, 
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);
/*fprintf(stderr,"CHANGE REGION: corner2: xgrid:%f ygrid:%f xlon:%f ylat:%f zdata:%f display: %f %f %f\n",
xgrid,ygrid,xlon,ylat,zdata,xdisplay,ydisplay,zdisplay);*/
				
		/* ignore an identical pair of points */
		if (found == MB_YES
			&& data->region.cornerpoints[1].xgrid
				== xgrid
			&& data->region.cornerpoints[1].ygrid
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
			data->region_pickcorner == MBV_REGION_PICKCORNER2;
			data->region.cornerpoints[2].xgrid = xgrid;
			data->region.cornerpoints[2].ygrid = ygrid;
			data->region.cornerpoints[2].xlon = xlon;
			data->region.cornerpoints[2].ylat = ylat;
			data->region.cornerpoints[2].zdata = zdata;
			data->region.cornerpoints[2].xdisplay = xdisplay;
			data->region.cornerpoints[2].ydisplay = ydisplay;
			data->region.cornerpoints[2].zdisplay = zdisplay;
			}
				
		/* ignore a bad point */
		else if (found == MB_NO)
			{
			XBell(view->dpy,100);
			}
		}

	/* deal with definition or change of cornerpoint 3 */
	else if (which == MBV_REGION_MOVE
		&& data->region_pickcorner == MBV_REGION_PICKCORNER3)
		{
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, 
				&found, 
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);
/*fprintf(stderr,"CHANGE REGION: corner3: xgrid:%f ygrid:%f xlon:%f ylat:%f zdata:%f display: %f %f %f\n",
xgrid,ygrid,xlon,ylat,zdata,xdisplay,ydisplay,zdisplay);*/
				
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
			/* set corner point 3 */
			data->region_type = MBV_REGION_QUAD;
			data->region_pickcorner == MBV_REGION_PICKCORNER3;
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
		/* if needed define corners 1 and 2 in grid coordinates */
		if (data->region_pickcorner == MBV_REGION_PICKCORNER0
			|| data->region_pickcorner == MBV_REGION_PICKCORNER3)
			{
			data->region.cornerpoints[1].xgrid
				= data->region.cornerpoints[0].xgrid;
			data->region.cornerpoints[1].ygrid 
				= data->region.cornerpoints[3].ygrid;
			mbview_getzdata(instance, 
					data->region.cornerpoints[1].xgrid, 
					data->region.cornerpoints[1].ygrid, 
					&ok,
					&(data->region.cornerpoints[1].zdata));
			if (ok == MB_NO)
				data->region.cornerpoints[1].zdata
					= 0.5 * (data->region.cornerpoints[0].zdata
							+ data->region.cornerpoints[3].zdata);
			mbview_projectforward(instance, MB_YES,
					data->region.cornerpoints[1].xgrid, 
					data->region.cornerpoints[1].ygrid, 
					data->region.cornerpoints[1].zdata,
					&(data->region.cornerpoints[1].xlon), 
					&(data->region.cornerpoints[1].ylat),
					&(data->region.cornerpoints[1].xdisplay), 
					&(data->region.cornerpoints[1].ydisplay), 
					&(data->region.cornerpoints[1].zdisplay));

			data->region.cornerpoints[2].xgrid
				= data->region.cornerpoints[3].xgrid;
			data->region.cornerpoints[2].ygrid 
				= data->region.cornerpoints[0].ygrid;
			mbview_getzdata(instance, 
					data->region.cornerpoints[2].xgrid, 
					data->region.cornerpoints[2].ygrid, 
					&ok,
					&(data->region.cornerpoints[2].zdata));
			if (ok == MB_NO)
				data->region.cornerpoints[2].zdata
					= 0.5 * (data->region.cornerpoints[0].zdata
							+ data->region.cornerpoints[3].zdata);
			mbview_projectforward(instance, MB_YES,
					data->region.cornerpoints[2].xgrid, 
					data->region.cornerpoints[2].ygrid, 
					data->region.cornerpoints[2].zdata,
					&(data->region.cornerpoints[2].xlon), 
					&(data->region.cornerpoints[2].ylat),
					&(data->region.cornerpoints[2].xdisplay), 
					&(data->region.cornerpoints[2].ydisplay), 
					&(data->region.cornerpoints[2].zdisplay));
			}
				
		/* if needed define corners 0 and 3 in grid coordinates */
		if (data->region_pickcorner == MBV_REGION_PICKCORNER1
			|| data->region_pickcorner == MBV_REGION_PICKCORNER2)
			{
			data->region.cornerpoints[0].xgrid
				= data->region.cornerpoints[2].xgrid;
			data->region.cornerpoints[0].ygrid 
				= data->region.cornerpoints[1].ygrid;
			mbview_getzdata(instance, 
					data->region.cornerpoints[0].xgrid, 
					data->region.cornerpoints[0].ygrid, 
					&ok,
					&(data->region.cornerpoints[0].zdata));
			if (ok == MB_NO)
				data->region.cornerpoints[0].zdata
					= 0.5 * (data->region.cornerpoints[1].zdata
							+ data->region.cornerpoints[2].zdata);
			mbview_projectforward(instance, MB_YES,
					data->region.cornerpoints[0].xgrid, 
					data->region.cornerpoints[0].ygrid, 
					data->region.cornerpoints[0].zdata,
					&(data->region.cornerpoints[0].xlon), 
					&(data->region.cornerpoints[0].ylat),
					&(data->region.cornerpoints[0].xdisplay), 
					&(data->region.cornerpoints[0].ydisplay), 
					&(data->region.cornerpoints[0].zdisplay));

			data->region.cornerpoints[3].xgrid
				= data->region.cornerpoints[1].xgrid;
			data->region.cornerpoints[3].ygrid 
				= data->region.cornerpoints[2].ygrid;
			mbview_getzdata(instance, 
					data->region.cornerpoints[3].xgrid, 
					data->region.cornerpoints[3].ygrid, 
					&ok,
					&(data->region.cornerpoints[3].zdata));
			if (ok == MB_NO)
				data->region.cornerpoints[3].zdata
					= 0.5 * (data->region.cornerpoints[1].zdata
							+ data->region.cornerpoints[2].zdata);
			mbview_projectforward(instance, MB_YES,
					data->region.cornerpoints[3].xgrid, 
					data->region.cornerpoints[3].ygrid, 
					data->region.cornerpoints[3].zdata,
					&(data->region.cornerpoints[3].xlon), 
					&(data->region.cornerpoints[3].ylat),
					&(data->region.cornerpoints[3].xdisplay), 
					&(data->region.cornerpoints[3].ydisplay), 
					&(data->region.cornerpoints[3].zdisplay));
			}
				
		/* calculate width and length */
		if (data->display_projection_mode != MBV_PROJECTION_SPHEROID)
			{
			data->region.width = fabs(data->region.cornerpoints[3].xdisplay
						- data->region.cornerpoints[0].xdisplay) / view->scale;
			data->region.height = fabs(data->region.cornerpoints[3].ydisplay
						- data->region.cornerpoints[0].ydisplay) / view->scale;
			}
		else
			{
			mbview_greatcircle_distbearing(instance, 
				data->region.cornerpoints[0].xlon, 
				data->region.cornerpoints[0].ylat, 
				data->region.cornerpoints[2].xlon, 
				data->region.cornerpoints[2].ylat, 
				&bearing, &data->region.width);
			mbview_greatcircle_distbearing(instance, 
				data->region.cornerpoints[0].xlon, 
				data->region.cornerpoints[0].ylat, 
				data->region.cornerpoints[1].xlon, 
				data->region.cornerpoints[1].ylat, 
				&bearing, &data->region.height);
			}
			
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
	double	dx, dy, dxuse, dyuse, bearing;
	double	dd;
	int	ok;
	int	match, match0, match1;
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

	/* check to see if pick is at existing end points */
	if (which == MBV_AREALENGTH_DOWN
		&& data->area_type == MBV_AREA_QUAD)
		{
		/* look for match to existing endpoints in neighborhood of pick */
		match = MB_NO;
		match0 = MB_NO;
		match1 = MB_NO;

		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, 
				&found, 
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);

		dx = 0.10 * (data->area.endpoints[1].xdisplay - data->area.endpoints[0].xdisplay);
		dy = 0.10 * (data->area.endpoints[1].ydisplay - data->area.endpoints[0].ydisplay);
		dd = MAX(dx, dy);
		if (found == MB_YES)
			{
			if (fabs(xdisplay - data->area.endpoints[0].xdisplay) < dd
				&& fabs(ydisplay - data->area.endpoints[0].ydisplay) < dd)
				{
				match = MB_YES;
				match0 = MB_YES;
				}
			else if (fabs(xdisplay - data->area.endpoints[1].xdisplay) < dd
				&& fabs(ydisplay - data->area.endpoints[1].ydisplay) < dd)
				{
				match = MB_YES;
				match1 = MB_YES;
				}
			}
			
		/* if no match then start new area */
		if (match == MB_NO)
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
				data->area_pickendpoint = MBV_AREA_PICKENDPOINT1;
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
			
		/* else if match 0 then reset endpoint 0 */
		else if (match0 == MB_YES)
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
				data->area_type = MBV_AREA_QUAD;
				data->area_pickendpoint = MBV_AREA_PICKENDPOINT0;
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
			
		/* else if match 1 then reset endpoint 1 */
		else if (match1 == MB_YES)
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
				data->area_type = MBV_AREA_QUAD;
				data->area_pickendpoint = MBV_AREA_PICKENDPOINT1;
				data->area.endpoints[1].xgrid = xgrid;
				data->area.endpoints[1].ygrid = ygrid;
				data->area.endpoints[1].xlon = xlon;
				data->area.endpoints[1].ylat = ylat;
				data->area.endpoints[1].zdata = zdata;
				data->area.endpoints[1].xdisplay = xdisplay;
				data->area.endpoints[1].ydisplay = ydisplay;
				data->area.endpoints[1].zdisplay = zdisplay;
				}
			}
		}

	/* deal with start of new area */
	else if ((which == MBV_AREALENGTH_DOWN
			|| which == MBV_AREALENGTH_MOVE)
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
			data->area_pickendpoint = MBV_AREA_PICKENDPOINT1;
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

	/* deal with definition or change of first endpoint */
	else if (which == MBV_AREALENGTH_MOVE
		&& data->area_pickendpoint == MBV_AREA_PICKENDPOINT0)
		{
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, 
				&found, 
				&xgrid, &ygrid,
				&xlon, &ylat, &zdata,
				&xdisplay, &ydisplay, &zdisplay);
				
		/* ignore an identical pair of points */
		if (found == MB_YES
			&& data->area.endpoints[1].xgrid == xgrid
			&& data->area.endpoints[1].ygrid == ygrid)
			{
			data->area_type = MBV_AREA_ONEPOINT;
			data->area_pickendpoint = MBV_AREA_PICKENDPOINT0;
			XBell(view->dpy,100);
			}
			
		/* use any good pair of points */
		else if (found == MB_YES)
			{
			/* set the second endpoint */
			data->area_type = MBV_AREA_QUAD;
			data->area_pickendpoint = MBV_AREA_PICKENDPOINT0;
			data->area.endpoints[0].xgrid = xgrid;
			data->area.endpoints[0].ygrid = ygrid;
			data->area.endpoints[0].xlon = xlon;
			data->area.endpoints[0].ylat = ylat;
			data->area.endpoints[0].zdata = zdata;
			data->area.endpoints[0].xdisplay = xdisplay;
			data->area.endpoints[0].ydisplay = ydisplay;
			data->area.endpoints[0].zdisplay = zdisplay;
			
			}
				
		/* ignore a bad point */
		else if (found == MB_NO)
			{
			XBell(view->dpy,100);
			}
		}


	/* deal with definition or change of second endpoint */
	else if (which == MBV_AREALENGTH_MOVE
		&& data->area_pickendpoint == MBV_AREA_PICKENDPOINT1)
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
			data->area_pickendpoint = MBV_AREA_PICKENDPOINT1;
			XBell(view->dpy,100);
			}
			
		/* use any good pair of points */
		else if (found == MB_YES)
			{
			/* set the second endpoint */
			data->area_type = MBV_AREA_QUAD;
			data->area_pickendpoint = MBV_AREA_PICKENDPOINT1;
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
		/* deal with non-spheroid case */
		if (data->display_projection_mode != MBV_PROJECTION_SPHEROID)
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
			if (data->area.bearing < 0.0)
				data->area.bearing += 360.0;
			if (data->area.bearing > 360.0)
				data->area.bearing -= 360.0;

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
							data->area.segments[i].endpoints[j]->zdisplay, 
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
					mbview_projectll2display(instance,
						data->area.segments[i].endpoints[j]->xlon, 
						data->area.segments[i].endpoints[j]->ylat, 
						data->area.segments[i].endpoints[j]->zdata ,
						&data->area.segments[i].endpoints[j]->xdisplay, 
						&data->area.segments[i].endpoints[j]->ydisplay,
						&data->area.segments[i].endpoints[j]->zdisplay);
					}
				}
			}

		/* else deal with spheroid case */
		else
			{			
			/* now get length and bearing of center line */
			mbview_greatcircle_distbearing(instance, 
				data->area.endpoints[0].xlon, 
				data->area.endpoints[0].ylat, 
				data->area.endpoints[1].xlon, 
				data->area.endpoints[1].ylat, 
				&data->area.bearing, &data->area.length);
			data->area.width = view->areaaspect * data->area.length;
				
			/* the corners of the area are defined by great
				circle arcs perpendicular to the center line */

			bearing = data->area.bearing - 90.0;
			if (bearing < 0.0)
				bearing += 360.0;
			if (bearing > 360.0)
				bearing -= 360.0;
			mbview_greatcircle_endposition(instance,
							data->area.endpoints[0].xlon, 
							data->area.endpoints[0].ylat, 
							bearing, 
							(0.5 * data->area.width),
							&(data->area.cornerpoints[0].xlon), 
							&(data->area.cornerpoints[0].ylat)), 
			status = mbview_projectll2xyzgrid(instance,
							data->area.cornerpoints[0].xlon, 
							data->area.cornerpoints[0].ylat, 
							&(data->area.cornerpoints[0].xgrid), 
							&(data->area.cornerpoints[0].ygrid), 
							&(data->area.cornerpoints[0].zdata)); 

			bearing = data->area.bearing + 90.0;
			if (bearing < 0.0)
				bearing += 360.0;
			if (bearing > 360.0)
				bearing -= 360.0;
			mbview_greatcircle_endposition(instance,
							data->area.endpoints[0].xlon, 
							data->area.endpoints[0].ylat, 
							bearing, 
							(0.5 * data->area.width),
							&(data->area.cornerpoints[1].xlon), 
							&(data->area.cornerpoints[1].ylat)), 
			status = mbview_projectll2xyzgrid(instance,
							data->area.cornerpoints[1].xlon, 
							data->area.cornerpoints[1].ylat, 
							&(data->area.cornerpoints[1].xgrid), 
							&(data->area.cornerpoints[1].ygrid), 
							&(data->area.cornerpoints[1].zdata)); 

			bearing = data->area.bearing + 90.0;
			if (bearing < 0.0)
				bearing += 360.0;
			if (bearing > 360.0)
				bearing -= 360.0;
			mbview_greatcircle_endposition(instance,
							data->area.endpoints[1].xlon, 
							data->area.endpoints[1].ylat, 
							bearing, 
							(0.5 * data->area.width),
							&(data->area.cornerpoints[2].xlon), 
							&(data->area.cornerpoints[2].ylat)), 
			status = mbview_projectll2xyzgrid(instance,
							data->area.cornerpoints[2].xlon, 
							data->area.cornerpoints[2].ylat, 
							&(data->area.cornerpoints[2].xgrid), 
							&(data->area.cornerpoints[2].ygrid), 
							&(data->area.cornerpoints[2].zdata)); 

			bearing = data->area.bearing - 90.0;
			if (bearing < 0.0)
				bearing += 360.0;
			if (bearing > 360.0)
				bearing -= 360.0;
			mbview_greatcircle_endposition(instance,
							data->area.endpoints[1].xlon, 
							data->area.endpoints[1].ylat, 
							bearing, 
							(0.5 * data->area.width),
							&(data->area.cornerpoints[3].xlon), 
							&(data->area.cornerpoints[3].ylat)), 
			status = mbview_projectll2xyzgrid(instance,
							data->area.cornerpoints[3].xlon, 
							data->area.cornerpoints[3].ylat, 
							&(data->area.cornerpoints[3].xgrid), 
							&(data->area.cornerpoints[3].ygrid), 
							&(data->area.cornerpoints[3].zdata)); 

			/* set pick info */
			data->pickinfo_mode = MBV_PICK_AREA;

			/* now project the segment endpoints */
			for (i=0;i<4;i++)
				{
				for (j=0;j<2;j++)
					{
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
					mbview_projectll2display(instance,
						data->area.segments[i].endpoints[j]->xlon, 
						data->area.segments[i].endpoints[j]->ylat, 
						data->area.segments[i].endpoints[j]->zdata ,
						&data->area.segments[i].endpoints[j]->xdisplay, 
						&data->area.segments[i].endpoints[j]->ydisplay,
						&data->area.segments[i].endpoints[j]->zdisplay);
					}
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
		}
#ifdef MBV_GETERRORS
mbview_glerrorcheck(instance, 1, function_name);
#endif

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
				glBegin(GL_LINES);
				glVertex3f((float)(data->region.segments[i].endpoints[0]->xdisplay), 
						(float)(data->region.segments[i].endpoints[0]->ydisplay), 
						(float)(data->region.segments[i].endpoints[0]->zdisplay));
				glVertex3f((float)(data->region.segments[i].endpoints[1]->xdisplay), 
						(float)(data->region.segments[i].endpoints[1]->ydisplay), 
						(float)(data->region.segments[i].endpoints[1]->zdisplay));
				glEnd();
#ifdef MBV_GETERRORS
mbview_glerrorcheck(instance, 1, function_name);
#endif
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
#ifdef MBV_GETERRORS
mbview_glerrorcheck(instance, 1, function_name);
#endif
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
#ifdef MBV_GETERRORS
mbview_glerrorcheck(instance, 1, function_name);
#endif
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
#ifdef MBV_GETERRORS
mbview_glerrorcheck(instance, 1, function_name);
#endif
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
#ifdef MBV_GETERRORS
mbview_glerrorcheck(instance, 1, function_name);
#endif
				}
			}
		}
#ifdef MBV_GETERRORS
mbview_glerrorcheck(instance, 1, function_name);
#endif

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
