/*------------------------------------------------------------------------------
 *    The MB-system:	mbview_pick.c	9/29/2003
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
 * Date:	September 29, 2003
 *
 * Note:	This code was broken out of mbview_callbacks.c
 */

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_status.h"
#include "mb_define.h"

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

/*------------------------------------------------------------------------------*/
int mbview_clearpicks(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	bool replotinstance = false;
	if (data->pick_type != MBV_PICK_NONE) {
		data->pick_type = MBV_PICK_NONE;
		replotinstance = true;
	}
	if (data->region_type != MBV_REGION_NONE) {
		data->region_type = MBV_REGION_NONE;
		replotinstance = true;
	}
	if (data->area_type != MBV_AREA_NONE) {
		data->area_type = MBV_AREA_NONE;
		replotinstance = true;
	}

	/* clear local profile */
	if (data->profile.npoints > 0) {
		data->profile.npoints = 0;
		data->profile.source = MBV_PROFILE_NONE;
		if (data->profile_view_mode == MBV_VIEW_ON)
			mbview_plotprofile(instance);
	}

	bool replotall = false;

	/* clear shared picks */
	if (shared.shareddata.navpick_type != MBV_PICK_NONE) {
		shared.shareddata.navpick_type = MBV_PICK_NONE;
		shared.shareddata.nav_selected[0] = MBV_SELECT_NONE;
		shared.shareddata.nav_selected[1] = MBV_SELECT_NONE;
		replotall = true;

		/* loop over the navs resetting selected points */
		for (int inav = 0; inav < shared.shareddata.nnav; inav++) {
			shared.shareddata.navs[inav].nselected = 0;
			for (int jpoint = 0; jpoint < shared.shareddata.navs[inav].npoints; jpoint++) {
				/* set size and color */
				if (shared.shareddata.navs[inav].navpts[jpoint].selected) {
					shared.shareddata.navs[inav].navpts[jpoint].selected = false;
					replotall = true;
				}
			}
		}
	}
	if (shared.shareddata.site_selected != MBV_SELECT_NONE) {
		shared.shareddata.site_selected = MBV_SELECT_NONE;
		replotall = true;
	}
	if (shared.shareddata.route_selected != MBV_SELECT_NONE) {
		shared.shareddata.route_selected = MBV_SELECT_NONE;
		shared.shareddata.route_point_selected = MBV_SELECT_NONE;
		replotall = true;
	}

	/* set widget sensitivity */
	if (data->active) {
		int error = MB_ERROR_NO_ERROR;
		mbview_update_sensitivity(mbv_verbose, instance, &error);
	}

	/* set pick annotation */
	mbview_pick_text(instance);

	/* update nav, site, and route lists */
	mbview_updatenavlist();
	mbview_updatesitelist();
	mbview_updateroutelist();

	/* draw */
	if (replotinstance || replotall) {
		if (mbv_verbose >= 2)
			fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_clearpicks\n");
		mbview_plotlowhigh(instance);
	}

	/* if needed replot all active instances */
	if (replotall) {
		mbview_plothighall(instance);
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
int mbview_clearnavpicks(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	bool replotinstance = false;
	if (data->pick_type == MBV_PICK_NAV) {
		data->pick_type = MBV_PICK_NONE;
		replotinstance = true;
	}

	bool replotall = false;

	/* clear shared nav picks */
	if (shared.shareddata.navpick_type != MBV_PICK_NONE) {
		shared.shareddata.navpick_type = MBV_PICK_NONE;
		shared.shareddata.nav_selected[0] = MBV_SELECT_NONE;
		shared.shareddata.nav_selected[1] = MBV_SELECT_NONE;
		replotall = true;

		/* loop over the navs resetting selected points */
		for (int inav = 0; inav < shared.shareddata.nnav; inav++) {
			shared.shareddata.navs[inav].nselected = 0;
			for (int jpoint = 0; jpoint < shared.shareddata.navs[inav].npoints; jpoint++) {
				/* set size and color */
				if (shared.shareddata.navs[inav].navpts[jpoint].selected) {
					shared.shareddata.navs[inav].navpts[jpoint].selected = false;
					replotall = true;
				}
			}
		}
	}

	/* set widget sensitivity */
	if (data->active && (replotinstance || replotall)) {
		int error = MB_ERROR_NO_ERROR;
		mbview_update_sensitivity(mbv_verbose, instance, &error);

		/* set pick annotation */
		mbview_pick_text(instance);

		/* update nav list */
		mbview_updatenavlist();
	}

	/* draw */
	if (replotinstance || replotall) {
		if (mbv_verbose >= 2)
			fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_clearpicks\n");
		mbview_plotlowhigh(instance);
	}

	/* if needed replot all active instances */
	if (replotall) {
		mbview_plothighall(instance);
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
int mbview_pick(size_t instance, int which, int xpixel, int ypixel) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       which:            %d\n", which);
		fprintf(stderr, "dbg2       xpixel:           %d\n", xpixel);
		fprintf(stderr, "dbg2       ypixel:           %d\n", ypixel);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	bool found;
	double xgrid, ygrid;
	double xlon, ylat, zdata;
	double xdisplay, ydisplay, zdisplay;

	/* look for point */
	mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay, &zdisplay);

	/* use any good pick */
	if (found) {
		if ((which == MBV_PICK_DOWN) || (which == MBV_PICK_MOVE && data->pick_type == MBV_PICK_NONE)) {
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
		else if (which == MBV_PICK_MOVE) {
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
		if (data->display_projection_mode != MBV_PROJECTION_SPHEROID) {
			const double dx = data->pick.endpoints[1].xdisplay - data->pick.endpoints[0].xdisplay;
			const double dy = data->pick.endpoints[1].ydisplay - data->pick.endpoints[0].ydisplay;
			data->pick.range = sqrt(dx * dx + dy * dy) / view->scale;
			data->pick.bearing = RTD * atan2(dx, dy);
		}
		else {
			mbview_greatcircle_distbearing(instance, data->pick.endpoints[0].xlon, data->pick.endpoints[0].ylat,
			                               data->pick.endpoints[1].xlon, data->pick.endpoints[1].ylat, &(data->pick.bearing),
			                               &(data->pick.range));
		}
		if (data->pick.bearing < 0.0)
			data->pick.bearing += 360.0;

		/* generate 3D drape of pick marks if either 3D display
		    or the pick move is final */
		if (data->pick_type != MBV_PICK_NONE && (data->display_mode == MBV_DISPLAY_3D || which == MBV_PICK_UP)) {
			mbview_picksize(instance);
		}

		/* if a two point pick has been made generate 3D drape
		    if either 3D display, the pick move is final
		    or the profile display is on */
		if (data->pick_type == MBV_PICK_TWOPOINT &&
		    (data->display_mode == MBV_DISPLAY_3D || data->profile_view_mode == MBV_VIEW_ON || which == MBV_PICK_UP)) {
			data->pick.segment.endpoints[0] = data->pick.endpoints[0];
			data->pick.segment.endpoints[1] = data->pick.endpoints[1];
			mbview_drapesegment(instance, &(data->pick.segment));
		}
	} else {
		if (which == MBV_PICK_DOWN) {
			data->pickinfo_mode = MBV_PICK_NONE;
			data->pick_type = MBV_PICK_NONE;
			XBell(view->dpy, 100);
		}
		else if (which == MBV_PICK_MOVE) {
			XBell(view->dpy, 100);
		}
	}

	/* set pick annotation */
	mbview_pick_text(instance);

	/* call pick notify if defined */
	if (which == MBV_PICK_UP && data->pick_type == MBV_PICK_ONEPOINT && data->mbview_pickonepoint_notify != NULL) {
		(data->mbview_pickonepoint_notify)(instance);
	}
	else if (which == MBV_PICK_UP && data->pick_type == MBV_PICK_TWOPOINT && data->mbview_picktwopoint_notify != NULL) {
		(data->mbview_picktwopoint_notify)(instance);
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
int mbview_extract_pick_profile(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	int status = MB_SUCCESS;

	/* if a two point pick has been made and the profile display
	    is on or the pick is final, insert the draped
	    segment into the profile data */
	if (data->pick_type == MBV_PICK_TWOPOINT) {
		data->profile.source = MBV_PROFILE_TWOPOINT;
		strcpy(data->profile.source_name, "Two point pick");
		data->profile.length = data->pick.range;
		const int npoints = MAX(2, data->pick.segment.nls);
		if (data->profile.npoints_alloc < npoints) {
			int error = MB_ERROR_NO_ERROR;
			status = mbview_allocprofilepoints(mbv_verbose, npoints, &(data->profile.points), &error);
			if (status == MB_SUCCESS) {
				data->profile.npoints_alloc = npoints;
			}
			else {
				data->profile.npoints_alloc = 0;
			}
		}
		if (npoints > 2 && data->profile.npoints_alloc >= npoints) {
			/* get the profile data */
			for (int i = 0; i < npoints; i++) {
				data->profile.points[i].boundary = false;
				data->profile.points[i].xgrid = data->pick.segment.lspoints[i].xgrid;
				data->profile.points[i].ygrid = data->pick.segment.lspoints[i].ygrid;
				data->profile.points[i].xlon = data->pick.segment.lspoints[i].xlon;
				data->profile.points[i].ylat = data->pick.segment.lspoints[i].ylat;
				data->profile.points[i].zdata = data->pick.segment.lspoints[i].zdata;
				data->profile.points[i].xdisplay = data->pick.segment.lspoints[i].xdisplay;
				data->profile.points[i].ydisplay = data->pick.segment.lspoints[i].ydisplay;
				if (i == 0) {
					data->profile.zmin = data->profile.points[i].zdata;
					data->profile.zmax = data->profile.points[i].zdata;
					data->profile.points[i].distance = 0.0;
					data->profile.points[i].distovertopo = 0.0;
				}
				else {
					data->profile.zmin = MIN(data->profile.zmin, data->profile.points[i].zdata);
					data->profile.zmax = MAX(data->profile.zmax, data->profile.points[i].zdata);
					if (data->display_projection_mode != MBV_PROJECTION_SPHEROID) {
						const double dx = data->profile.points[i].xdisplay - data->profile.points[i - 1].xdisplay;
						const double dy = data->profile.points[i].ydisplay - data->profile.points[i - 1].ydisplay;
						data->profile.points[i].distance =
						    sqrt(dx * dx + dy * dy) / view->scale + data->profile.points[i - 1].distance;
					}
					else {
						mbview_greatcircle_dist(instance, data->profile.points[0].xlon, data->profile.points[0].ylat,
						                        data->profile.points[i].xlon, data->profile.points[i].ylat,
						                        &(data->profile.points[i].distance));
					}
					const double dy = (data->profile.points[i].zdata - data->profile.points[i - 1].zdata);
					const double dx = (data->profile.points[i].distance - data->profile.points[i - 1].distance);
					data->profile.points[i].distovertopo = data->profile.points[i - 1].distovertopo + sqrt(dy * dy + dx * dx);
					if (dx > 0.0)
						data->profile.points[i].slope = fabs(dy / dx);
					else
						data->profile.points[i].slope = 0.0;
				}
				data->profile.points[i].bearing = data->pick.bearing;
				if (i > 1) {
					const double dy = (data->profile.points[i].zdata - data->profile.points[i - 2].zdata);
					const double dx = (data->profile.points[i].distance - data->profile.points[i - 2].distance);
					if (dx > 0.0)
						data->profile.points[i - 1].slope = fabs(dy / dx);
					else
						data->profile.points[i - 1].slope = 0.0;
				}
				data->profile.points[i].navzdata = 0.0;
				data->profile.points[i].navtime_d = 0.0;
			}
			data->profile.points[0].boundary = true;
			data->profile.points[npoints - 1].boundary = true;
			data->profile.npoints = npoints;
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
int mbview_picksize(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	bool found;

	double xlength = 0.0;

	/* resize and redrape pick marks if required */
	if (data->pickinfo_mode == MBV_PICK_ONEPOINT || data->pickinfo_mode == MBV_PICK_TWOPOINT) {
		/* set size of 'X' marks in gl units for 3D case */
		const double scalefactor = MIN(((double)(data->viewbounds[1] - data->viewbounds[0])) / ((double)data->primary_n_columns),
		                  ((double)(data->viewbounds[3] - data->viewbounds[2])) / ((double)data->primary_n_rows));
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
		for (int i = 0; i < 4; i++) {
			mbview_projectinverse(instance, true, data->pick.xpoints[i].xdisplay, data->pick.xpoints[i].ydisplay,
			                      data->pick.xpoints[i].zdisplay, &data->pick.xpoints[i].xlon, &data->pick.xpoints[i].ylat,
			                      &data->pick.xpoints[i].xgrid, &data->pick.xpoints[i].ygrid);
			mbview_getzdata(instance, data->pick.xpoints[i].xgrid, data->pick.xpoints[i].ygrid, &found,
			                &data->pick.xpoints[i].zdata);
			if (!found)
				data->pick.xpoints[i].zdata = data->pick.endpoints[0].zdata;
			mbview_projectforward(instance, true, data->pick.xpoints[i].xgrid, data->pick.xpoints[i].ygrid,
			                      data->pick.xpoints[i].zdata, &(data->pick.xpoints[i].xlon), &(data->pick.xpoints[i].ylat),
			                      &(data->pick.xpoints[i].xdisplay), &(data->pick.xpoints[i].ydisplay),
			                      &(data->pick.xpoints[i].zdisplay));
		}

		/* drape the x marker line segments */
		for (int i = 0; i < 2; i++) {
			data->pick.xsegments[i].endpoints[0] = data->pick.xpoints[2 * i];
			data->pick.xsegments[i].endpoints[1] = data->pick.xpoints[2 * i + 1];
			mbview_drapesegment(instance, &(data->pick.xsegments[i]));
		}
	}
	if (data->pickinfo_mode == MBV_PICK_TWOPOINT) {
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
		for (int i = 0; i < 4; i++) {
			mbview_projectinverse(instance, true, data->pick.xpoints[i + 4].xdisplay, data->pick.xpoints[i + 4].ydisplay,
			                      data->pick.xpoints[i + 4].zdisplay, &data->pick.xpoints[i + 4].xlon,
			                      &data->pick.xpoints[i + 4].ylat, &data->pick.xpoints[i + 4].xgrid,
			                      &data->pick.xpoints[i + 4].ygrid);
			mbview_getzdata(instance, data->pick.xpoints[i + 4].xgrid, data->pick.xpoints[i + 4].ygrid, &found,
			                &data->pick.xpoints[i + 4].zdata);
			if (!found)
				data->pick.xpoints[i + 4].zdata = data->pick.endpoints[1].zdata;
			mbview_projectforward(instance, true, data->pick.xpoints[i + 4].xgrid, data->pick.xpoints[i + 4].ygrid,
			                      data->pick.xpoints[i + 4].zdata, &(data->pick.xpoints[i + 4].xlon),
			                      &(data->pick.xpoints[i + 4].ylat), &(data->pick.xpoints[i + 4].xdisplay),
			                      &(data->pick.xpoints[i + 4].ydisplay), &(data->pick.xpoints[i + 4].zdisplay));
		}

		/* drape the x marker line segments */
		for (int i = 0; i < 2; i++) {
			data->pick.xsegments[i + 2].endpoints[0] = data->pick.xpoints[2 * i + 4];
			data->pick.xsegments[i + 2].endpoints[1] = data->pick.xpoints[2 * i + 5];
			mbview_drapesegment(instance, &(data->pick.xsegments[i + 2]));
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
int mbview_pick_text(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);
	// fprintf(stderr,"mbview_pick_text: instance:%zu pickinfo_mode:%d\n",instance,data->pickinfo_mode);

  char value_text[3*MB_PATH_MAXLINE];
  char value_list[5*MB_PATH_MAXLINE];

	int time_i[7];
	char londstr0[24], londstr1[24], lonmstr0[24], lonmstr1[24];
	char latdstr0[24], latdstr1[24], latmstr0[24], latmstr1[24];
	char date0[24], date1[24];
	char shot0[48], shot1[48];
	double lonmin, lonmax, latmin, latmax;

	/* update pick info */
	if (data->pickinfo_mode == MBV_PICK_ONEPOINT) {
		mbview_setlonlatstrings(data->pick.endpoints[0].xlon, data->pick.endpoints[0].ylat, londstr0, latdstr0, lonmstr0,
		                        latmstr0);
		if (shared.lonlatstyle == MBV_LONLAT_DEGREESDECIMAL)
			sprintf(value_text, ":::t\"Pick Info:\":t\" Lon: %s\":t\" Lat: %s\":t\" Depth: %.3f m\"", londstr0, latdstr0,
			        data->pick.endpoints[0].zdata);
		else
			sprintf(value_text, ":::t\"Pick Info:\":t\" Lon: %s\":t\" Lat: %s\":t\" Depth: %.3f m\"", lonmstr0, latmstr0,
			        data->pick.endpoints[0].zdata);
		sprintf(value_list,
		        "Pick Info: Lon: %s Lat: %s Depth: %.3f m\n"
		        "           Lon: %s Lat: %s Depth: %.3f m\n",
		        londstr0, latdstr0, data->pick.endpoints[0].zdata, lonmstr0, latmstr0, data->pick.endpoints[0].zdata);
	}
	else if (data->pickinfo_mode == MBV_PICK_TWOPOINT) {
		mbview_setlonlatstrings(data->pick.endpoints[0].xlon, data->pick.endpoints[0].ylat, londstr0, latdstr0, lonmstr0,
		                        latmstr0);
		mbview_setlonlatstrings(data->pick.endpoints[1].xlon, data->pick.endpoints[1].ylat, londstr1, latdstr1, lonmstr1,
		                        latmstr1);
		if (shared.lonlatstyle == MBV_LONLAT_DEGREESDECIMAL) {
			sprintf(value_text,
			        ":::t\"Pick Info:\":t\" Lon 1: %s\":t\" Lat 1: %s\":t\" Depth 1: %.3f m\":t\" Lon 2: %s\":t\" Lat 2: "
			        "%s\":t\" Depth 2: %.3f m\":t\" Bearing: %.1f deg\":t\" Distance: %.3f m\"",
			        londstr0, latdstr0, data->pick.endpoints[0].zdata, londstr1, latdstr1, data->pick.endpoints[1].zdata,
			        data->pick.bearing, data->pick.range);
		}
		else {
			sprintf(value_text,
			        ":::t\"Pick Info:\":t\" Lon 1: %s\":t\" Lat 1: %s\":t\" Depth 1: %.3f m\":t\" Lon 2: %s\":t\" Lat 2: "
			        "%s\":t\" Depth 2: %.3f m\":t\" Bearing: %.1f deg\":t\" Distance: %.3f m\"",
			        lonmstr0, latmstr0, data->pick.endpoints[0].zdata, lonmstr1, latmstr1, data->pick.endpoints[1].zdata,
			        data->pick.bearing, data->pick.range);
		}
		sprintf(value_list,
		        "Pick Info: Lon 1: %s Lat 1: %s Depth 1: %.3f m Lon 2: %s Lat 2: %s Depth 2: %.3f m Bearing: %.1f deg Distance: "
		        "%.3f m\n"
		        "           Lon 1: %s Lat 1: %s Depth 1: %.3f m Lon 2: %s Lat 2: %s Depth 2: %.3f m Bearing: %.1f deg Distance: "
		        "%.3f m\n",
		        londstr0, latdstr0, data->pick.endpoints[0].zdata, londstr1, latdstr1, data->pick.endpoints[1].zdata,
		        data->pick.bearing, data->pick.range, lonmstr0, latmstr0, data->pick.endpoints[0].zdata, lonmstr1, latmstr1,
		        data->pick.endpoints[1].zdata, data->pick.bearing, data->pick.range);
	}
	else if (data->pickinfo_mode == MBV_PICK_AREA) {
		sprintf(value_text, ":::t\"Area Info:\":t\" Length: %.3f m\":t\" Width: %.3f m\":t\" Bearing: %.1f deg\"",
		        data->area.length, data->area.width, data->area.bearing);
		sprintf(value_list, "Area Info: Length: %.3f m Width: %.3f m Bearing: %.1f deg\n", data->area.length, data->area.width,
		        data->area.bearing);
	}
	else if (data->pickinfo_mode == MBV_PICK_REGION) {
		lonmin = data->region.cornerpoints[0].xlon;
		lonmax = data->region.cornerpoints[0].xlon;
		latmin = data->region.cornerpoints[0].ylat;
		latmax = data->region.cornerpoints[0].ylat;
		for (int i = 1; i < 4; i++) {
			lonmin = MIN(lonmin, data->region.cornerpoints[i].xlon);
			lonmax = MAX(lonmax, data->region.cornerpoints[i].xlon);
			latmin = MIN(latmin, data->region.cornerpoints[i].ylat);
			latmax = MAX(latmax, data->region.cornerpoints[i].ylat);
		}
		if (view->lonflip < 0) {
			if (lonmin > 0.)
				lonmin = lonmin - 360.;
			else if (lonmin < -360.)
				lonmin = lonmin + 360.;
			if (lonmax > 0.)
				lonmax = lonmax - 360.;
			else if (lonmax < -360.)
				lonmax = lonmax + 360.;
		}
		else if (view->lonflip == 0) {
			if (lonmin > 180.)
				lonmin = lonmin - 360.;
			else if (lonmin < -180.)
				lonmin = lonmin + 360.;
			if (lonmax > 180.)
				lonmax = lonmax - 360.;
			else if (lonmax < -180.)
				lonmax = lonmax + 360.;
		}
		else {
			if (lonmin > 360.)
				lonmin = lonmin - 360.;
			else if (lonmin < 0.)
				lonmin = lonmin + 360.;
			if (lonmax > 360.)
				lonmax = lonmax - 360.;
			else if (lonmax < 0.)
				lonmax = lonmax + 360.;
		}
		mbview_setlonlatstrings(lonmin, latmin, londstr0, latdstr0, lonmstr0, latmstr0);
		mbview_setlonlatstrings(lonmax, latmax, londstr1, latdstr1, lonmstr1, latmstr1);
		if (shared.lonlatstyle == MBV_LONLAT_DEGREESDECIMAL)
			sprintf(
			    value_text,
			    ":::t\"Region Info:\":t\" W: %s\":t\" E: %s\":t\" S: %s\":t\" N: %s\":t\" Width: %.3f m\":t\" Height: %.3f m\"",
			    londstr0, londstr1, latdstr0, latdstr1, data->region.width, data->region.height);
		else
			sprintf(
			    value_text,
			    ":::t\"Region Info:\":t\" W: %s\":t\" E: %s\":t\" S: %s\":t\" N: %s\":t\" Width: %.3f m\":t\" Height: %.3f m\"",
			    lonmstr0, lonmstr1, latmstr0, latmstr1, data->region.width, data->region.height);
		sprintf(value_list,
		        "Region Info: Bounds: %s/%s/%s/%s\n"
		        "             Bounds: %s/%s/%s/%s\n"
		        "             Width: %.3f m Height: %.3f m\n",
		        londstr0, londstr1, latdstr0, latdstr1, lonmstr0, lonmstr1, latmstr0, latmstr1, data->region.width,
		        data->region.height);
	}
	else if (data->pickinfo_mode == MBV_PICK_SITE && shared.shareddata.site_selected != MBV_SELECT_NONE) {
		mbview_setlonlatstrings(shared.shareddata.sites[shared.shareddata.site_selected].point.xlon,
		                        shared.shareddata.sites[shared.shareddata.site_selected].point.ylat, londstr0, latdstr0, lonmstr0,
		                        latmstr0);
		if (shared.lonlatstyle == MBV_LONLAT_DEGREESDECIMAL)
			sprintf(value_text,
			        ":::t\"Site %d Pick Info:\":t\" Lon: %s\":t\" Lat: %s\":t\" Depth: %.3f m\":t\" Color: %d\":t\" Size: "
			        "%d\":t\" Name: %s\"",
			        shared.shareddata.site_selected, londstr0, latdstr0,
			        shared.shareddata.sites[shared.shareddata.site_selected].point.zdata,
			        shared.shareddata.sites[shared.shareddata.site_selected].color,
			        shared.shareddata.sites[shared.shareddata.site_selected].size,
			        shared.shareddata.sites[shared.shareddata.site_selected].name);
		else
			sprintf(value_text,
			        ":::t\"Site %d Pick Info:\":t\" Lon: %s\":t\" Lat: %s\":t\" Depth: %.3f m\":t\" Color: %d\":t\" Size: "
			        "%d\":t\" Name: %s\"",
			        shared.shareddata.site_selected, lonmstr0, latmstr0,
			        shared.shareddata.sites[shared.shareddata.site_selected].point.zdata,
			        shared.shareddata.sites[shared.shareddata.site_selected].color,
			        shared.shareddata.sites[shared.shareddata.site_selected].size,
			        shared.shareddata.sites[shared.shareddata.site_selected].name);
		sprintf(value_list,
		        "Site %3d Pick Info: Lon: %s Lat: %s Depth: %.3f m Color: %d Size: %d Name: %s\n"
		        "                    Lon: %s Lat: %s Depth: %.3f m Color: %d Size: %d Name: %s\n",
		        shared.shareddata.site_selected, londstr0, latdstr0,
		        shared.shareddata.sites[shared.shareddata.site_selected].point.zdata,
		        shared.shareddata.sites[shared.shareddata.site_selected].color,
		        shared.shareddata.sites[shared.shareddata.site_selected].size,
		        shared.shareddata.sites[shared.shareddata.site_selected].name, lonmstr0, latmstr0,
		        shared.shareddata.sites[shared.shareddata.site_selected].point.zdata,
		        shared.shareddata.sites[shared.shareddata.site_selected].color,
		        shared.shareddata.sites[shared.shareddata.site_selected].size,
		        shared.shareddata.sites[shared.shareddata.site_selected].name);
	}
	else if (data->pickinfo_mode == MBV_PICK_ROUTE && shared.shareddata.route_selected != MBV_SELECT_NONE &&
	         shared.shareddata.route_point_selected == MBV_SELECT_ALL) {
		sprintf(value_text, ":::t\"Route %d Pick Info:\":t\" Points: %d\":t\" Length: %.3f m\":t\" LOB: %.3f m\":t\" Name: %s\"",
		        shared.shareddata.route_selected, shared.shareddata.routes[shared.shareddata.route_selected].npoints,
		        shared.shareddata.routes[shared.shareddata.route_selected].distancelateral,
		        shared.shareddata.routes[shared.shareddata.route_selected].distancetopo,
		        shared.shareddata.routes[shared.shareddata.route_selected].name);
		sprintf(value_list, "Route %d Pick Info: Points: %d Length: %.3f m LOB: %.3f m Name: %s\n",
		        shared.shareddata.route_selected, shared.shareddata.routes[shared.shareddata.route_selected].npoints,
		        shared.shareddata.routes[shared.shareddata.route_selected].distancelateral,
		        shared.shareddata.routes[shared.shareddata.route_selected].distancetopo,
		        shared.shareddata.routes[shared.shareddata.route_selected].name);
	}
	else if (data->pickinfo_mode == MBV_PICK_ROUTE && shared.shareddata.route_selected != MBV_SELECT_NONE &&
	         shared.shareddata.route_point_selected != MBV_SELECT_NONE) {
		mbview_setlonlatstrings(
		    shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].xlon,
		    shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].ylat,
		    londstr0, latdstr0, lonmstr0, latmstr0);
		if (shared.lonlatstyle == MBV_LONLAT_DEGREESDECIMAL)
			sprintf(
			    value_text,
			    ":::t\"Route %d Pick Info:\":t\" Point: %d\":t\" Lon: %s\":t\" Lat: %s\":t\" Depth: %.3f m\":t\" Length: %.3f "
			    "m\":t\" LOB: %.3f m\":t\" Name: %s\"",
			    shared.shareddata.route_selected, shared.shareddata.route_point_selected, londstr0, latdstr0,
			    shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].zdata,
			    shared.shareddata.routes[shared.shareddata.route_selected].distlateral[shared.shareddata.route_point_selected],
			    shared.shareddata.routes[shared.shareddata.route_selected].disttopo[shared.shareddata.route_point_selected],
			    shared.shareddata.routes[shared.shareddata.route_selected].name);
		else
			sprintf(
			    value_text,
			    ":::t\"Route %d Pick Info:\":t\" Point: %d\":t\" Lon: %s\":t\" Lat: %s\":t\" Depth: %.3f m\":t\" Length: %.3f "
			    "m\":t\" LOB: %.3f m\":t\" Name: %s\"",
			    shared.shareddata.route_selected, shared.shareddata.route_point_selected, lonmstr0, latmstr0,
			    shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].zdata,
			    shared.shareddata.routes[shared.shareddata.route_selected].distlateral[shared.shareddata.route_point_selected],
			    shared.shareddata.routes[shared.shareddata.route_selected].disttopo[shared.shareddata.route_point_selected],
			    shared.shareddata.routes[shared.shareddata.route_selected].name);
		sprintf(value_list,
		        "Route %3d Pick Info: Point: %d Lon: %s Lat: %s Depth: %.3f m Length: %.3f m LOB: %.3f m Name: %s\n"
		        "                     Point: %d Lon: %s Lat: %s Depth: %.3f m Length: %.3f m LOB: %.3f m Name: %s\n",
		        shared.shareddata.route_selected, shared.shareddata.route_point_selected, londstr0, latdstr0,
		        shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].zdata,
		        shared.shareddata.routes[shared.shareddata.route_selected].distlateral[shared.shareddata.route_point_selected],
		        shared.shareddata.routes[shared.shareddata.route_selected].disttopo[shared.shareddata.route_point_selected],
		        shared.shareddata.routes[shared.shareddata.route_selected].name, shared.shareddata.route_point_selected, lonmstr0,
		        latmstr0,
		        shared.shareddata.routes[shared.shareddata.route_selected].points[shared.shareddata.route_point_selected].zdata,
		        shared.shareddata.routes[shared.shareddata.route_selected].distlateral[shared.shareddata.route_point_selected],
		        shared.shareddata.routes[shared.shareddata.route_selected].disttopo[shared.shareddata.route_point_selected],
		        shared.shareddata.routes[shared.shareddata.route_selected].name);
	}
	else if (data->pickinfo_mode == MBV_PICK_NAV && shared.shareddata.navpick_type == MBV_PICK_ONEPOINT &&
	         shared.shareddata.nav_selected[0] != MBV_SELECT_NONE) {
		mb_get_date(
		    mbv_verbose,
		    shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].time_d,
		    time_i);
		sprintf(date0, "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%3.3d", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4],
		        time_i[5], (time_i[6] / 1000));
		mbview_setlonlatstrings(
		    shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].point.xlon,
		    shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].point.ylat,
		    londstr0, latdstr0, lonmstr0, latmstr0);
		sprintf(shot0, "#:%d:%d/%d",
		        shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].line,
		        shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].shot,
		        shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].cdp);
		if (shared.lonlatstyle == MBV_LONLAT_DEGREESDECIMAL)
			sprintf(
			    value_text,
			    ":::t\"Navigation Pick Info:\":t\" %s\":t\" %s\":t\" Lon: %s\":t\" Lat: %s\":t\" Vertical: %.3f m\":t\" Heading: "
			    "%.1f deg\":t\" Speed: %.1f km/hr\":t\" %s\"",
			    shared.shareddata.navs[shared.shareddata.nav_selected[0]].name, date0, londstr0, latdstr0,
			    shared.shareddata.navs[shared.shareddata.nav_selected[0]]
			        .navpts[shared.shareddata.nav_point_selected[0]]
			        .point.zdata,
			    shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].heading,
			    shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].speed,
			    shot0);
		else
			sprintf(
			    value_text,
			    ":::t\"Navigation Pick Info:\":t\" %s\":t\" %s\":t\" Lon: %s\":t\" Lat: %s\":t\" Vertical: %.3f m\":t\" Heading: "
			    "%.1f deg\":t\" Speed: %.1f km/hr\":t\" %s\"",
			    shared.shareddata.navs[shared.shareddata.nav_selected[0]].name, date0, lonmstr0, latmstr0,
			    shared.shareddata.navs[shared.shareddata.nav_selected[0]]
			        .navpts[shared.shareddata.nav_point_selected[0]]
			        .point.zdata,
			    shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].heading,
			    shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].speed,
			    shot0);
		sprintf(
		    value_list,
		    "Navigation Pick Info: %s %s Lon: %s Lat: %s Vehicle Depth: %.3f m Heading: %.1f deg Speed: %.1f km/hr %s\n"
		    "                      %s %s Lon: %s Lat: %s Vehicle Depth: %.3f m Heading: %.1f deg Speed: %.1f km/hr %s\n",
		    shared.shareddata.navs[shared.shareddata.nav_selected[0]].name, date0, londstr0, latdstr0,
		    shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].point.zdata,
		    shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].heading,
		    shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].speed,
		    shot0, shared.shareddata.navs[shared.shareddata.nav_selected[0]].name, date0, lonmstr0, latmstr0,
		    shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].point.zdata,
		    shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].heading,
		    shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].speed,
		    shot0);
	}
	else if (data->pickinfo_mode == MBV_PICK_NAV && shared.shareddata.navpick_type == MBV_PICK_TWOPOINT &&
	         shared.shareddata.nav_selected[0] != MBV_SELECT_NONE && shared.shareddata.nav_selected[1] != MBV_SELECT_NONE) {
		mb_get_date(
		    mbv_verbose,
		    shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].time_d,
		    time_i);
		sprintf(date0, "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%3.3d", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4],
		        time_i[5], (time_i[6] / 1000));
		mbview_setlonlatstrings(
		    shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].point.xlon,
		    shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].point.ylat,
		    londstr0, latdstr0, lonmstr0, latmstr0);
		sprintf(shot0, "#:%d:%d/%d",
		        shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].line,
		        shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].shot,
		        shared.shareddata.navs[shared.shareddata.nav_selected[0]].navpts[shared.shareddata.nav_point_selected[0]].cdp);
		mb_get_date(
		    mbv_verbose,
		    shared.shareddata.navs[shared.shareddata.nav_selected[1]].navpts[shared.shareddata.nav_point_selected[1]].time_d,
		    time_i);
		sprintf(date1, "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%3.3d", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4],
		        time_i[5], (time_i[6] / 1000));
		mbview_setlonlatstrings(
		    shared.shareddata.navs[shared.shareddata.nav_selected[1]].navpts[shared.shareddata.nav_point_selected[1]].point.xlon,
		    shared.shareddata.navs[shared.shareddata.nav_selected[1]].navpts[shared.shareddata.nav_point_selected[1]].point.ylat,
		    londstr1, latdstr1, lonmstr1, latmstr1);
		sprintf(shot1, "#:%d:%d/%d",
		        shared.shareddata.navs[shared.shareddata.nav_selected[1]].navpts[shared.shareddata.nav_point_selected[1]].line,
		        shared.shareddata.navs[shared.shareddata.nav_selected[1]].navpts[shared.shareddata.nav_point_selected[1]].shot,
		        shared.shareddata.navs[shared.shareddata.nav_selected[1]].navpts[shared.shareddata.nav_point_selected[1]].cdp);
		if (shared.lonlatstyle == MBV_LONLAT_DEGREESDECIMAL)
			sprintf(value_text,
			        ":::t\"Navigation Picks Info:\":t\" %s\":t\" %s\":t\" Lon: %s\":t\" Lat: %s\":t\" %s\":t\" %s\":t\" %s\":t\" "
			        "Lon: %s\":t\" Lat: %s\":t\" %s\"",
			        shared.shareddata.navs[shared.shareddata.nav_selected[0]].name, date0, londstr0, latdstr0, shot0,
			        shared.shareddata.navs[shared.shareddata.nav_selected[1]].name, date1, londstr1, latdstr1, shot1);
		else
			sprintf(value_text,
			        ":::t\"Navigation Picks Info:\":t\" %s\":t\" %s\":t\" Lon: %s\":t\" Lat: %s\":t\" %s\":t\" %s\":t\" %s\":t\" "
			        "Lon: %s\":t\" Lat: %s\":t\" %s\"",
			        shared.shareddata.navs[shared.shareddata.nav_selected[0]].name, date0, lonmstr0, latmstr0, shot0,
			        shared.shareddata.navs[shared.shareddata.nav_selected[1]].name, date1, lonmstr1, latmstr1, shot1);
		sprintf(value_list,
		        "Navigation Picks Info: %s %s Lon: %s Lat: %s %s %s %s Lon: %s Lat: %s %s\n"
		        "                       %s %s Lon: %s Lat: %s %s %s %s Lon: %s Lat: %s %s\n",
		        shared.shareddata.navs[shared.shareddata.nav_selected[0]].name, date0, londstr0, latdstr0, shot0,
		        shared.shareddata.navs[shared.shareddata.nav_selected[1]].name, date1, londstr1, latdstr1, shot1,
		        shared.shareddata.navs[shared.shareddata.nav_selected[0]].name, date0, lonmstr0, latmstr0, shot0,
		        shared.shareddata.navs[shared.shareddata.nav_selected[1]].name, date1, lonmstr1, latmstr1, shot1);
	}
	else
	        {
	        sprintf(value_text, ":::t\"Pick Info:\":t\"No Pick\"");
	        sprintf(value_list, "Pick Info: No Pick\n");
          data->pickinfo_mode = MBV_PICK_NONE;
	        }
	set_mbview_label_multiline_string(view->mb3dview.mbview_label_pickinfo, value_text);
  if (data->pickinfo_mode != MBV_PICK_NONE)
    fprintf(stderr, "%s", value_list);

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_setlonlatstrings(double lon, double lat, char *londstring, char *latdstring, char *lonmstring, char *latmstring) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       lon:              %f\n", lon);
		fprintf(stderr, "dbg2       lat:              %f\n", lat);
	}

	if (lon > 180.0) {
		lon -= 360.0;
	}
	if (lon < -180.0) {
		lon += 360.0;
	}

	/* decimal degrees (style == MBV_LONLAT_DEGREESDECIMAL) */
	sprintf(londstring, "%.7f", lon);
	sprintf(latdstring, "%.7f", lat);

	/* degrees + minutes (style == MBV_LONLAT_DEGREESMINUTES) */
	const int lon_degree = (int)fabs(lon);
	const double lon_minute = 60.0 * (fabs(lon) - lon_degree);
  char EorW;
  EorW = lon >= 0 ? 'E' : 'W';
	sprintf(lonmstring, "%3d %10.6f %c", lon_degree, lon_minute, EorW);

	const int degree = (int)fabs(lat);
	const double minute = 60.0 * (fabs(lat) - degree);
	if (lat < 0.0)
		sprintf(latmstring, "%3d %10.6f S", degree, minute);
	else
		sprintf(latmstring, "%3d %10.6f N", degree, minute);

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       londstring:      %s\n", londstring);
		fprintf(stderr, "dbg2       latdstring:      %s\n", latdstring);
		fprintf(stderr, "dbg2       lonmstring:      %s\n", lonmstring);
		fprintf(stderr, "dbg2       latmstring:      %s\n", latmstring);
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_region(size_t instance, int which, int xpixel, int ypixel) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       which:            %d\n", which);
		fprintf(stderr, "dbg2       xpixel:           %d\n", xpixel);
		fprintf(stderr, "dbg2       ypixel:           %d\n", ypixel);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	bool found;
	double xgrid, ygrid;
	double xlon, ylat, zdata;
	double xdisplay, ydisplay, zdisplay;
	double bearing;

	/* check to see if pick is at existing corner points */
	if (which == MBV_REGION_DOWN && data->region_type == MBV_REGION_QUAD) {
		/* look for match to existing corner points in neighborhood of pick */

		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay, &zdisplay);

		const double dx = 0.10 * (data->region.cornerpoints[3].xdisplay - data->region.cornerpoints[0].xdisplay);
		const double dy = 0.10 * (data->region.cornerpoints[3].ydisplay - data->region.cornerpoints[0].ydisplay);
		const double dd = MAX(dx, dy);

		bool match = false;
		bool match0 = false;
		bool match1 = false;
		bool match2 = false;
		bool match3 = false;

		if (found) {
			if (fabs(xdisplay - data->region.cornerpoints[0].xdisplay) < dd &&
			    fabs(ydisplay - data->region.cornerpoints[0].ydisplay) < dd) {
				match = true;
				match0 = true;
			}
			else if (fabs(xdisplay - data->region.cornerpoints[1].xdisplay) < dd &&
			         fabs(ydisplay - data->region.cornerpoints[1].ydisplay) < dd) {
				match = true;
				match1 = true;
			}
			else if (fabs(xdisplay - data->region.cornerpoints[2].xdisplay) < dd &&
			         fabs(ydisplay - data->region.cornerpoints[2].ydisplay) < dd) {
				match = true;
				match2 = true;
			}
			else if (fabs(xdisplay - data->region.cornerpoints[3].xdisplay) < dd &&
			         fabs(ydisplay - data->region.cornerpoints[3].ydisplay) < dd) {
				match = true;
				match3 = true;
			}
		}

		/* if no match then start new region */
		if (!match) {
			/* look for point */
			mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay,
			                 &zdisplay);

			/* use any good point */
			if (found) {
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
		else if (match0) {
			/* look for point */
			mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay,
			                 &zdisplay);

			/* use any good point */
			if (found) {
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
		else if (match1) {
			/* look for point */
			mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay,
			                 &zdisplay);

			/* use any good point */
			if (found) {
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
		else if (match2) {
			/* look for point */
			mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay,
			                 &zdisplay);

			/* use any good point */
			if (found) {
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
		else if (match3) {
			/* look for point */
			mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay,
			                 &zdisplay);

			/* use any good point */
			if (found) {
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
	else if ((which == MBV_REGION_DOWN || which == MBV_REGION_MOVE) && data->region_type == MBV_REGION_NONE) {
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay, &zdisplay);
		/*fprintf(stderr,"NEW REGION: corner0: xgrid:%f ygrid:%f xlon:%f ylat:%f zdata:%f display: %f %f %f\n",
		xgrid,ygrid,xlon,ylat,zdata,xdisplay,ydisplay,zdisplay);*/

		/* use any good point */
		if (found) {
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
	else if (which == MBV_REGION_MOVE && data->region_pickcorner == MBV_REGION_PICKCORNER0) {
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay, &zdisplay);
		/*fprintf(stderr,"CHANGE REGION: corner0: xgrid:%f ygrid:%f xlon:%f ylat:%f zdata:%f display: %f %f %f\n",
		xgrid,ygrid,xlon,ylat,zdata,xdisplay,ydisplay,zdisplay);*/

		/* ignore an identical pair of points */
		if (found && data->region.cornerpoints[3].xgrid == xgrid && data->region.cornerpoints[3].ygrid == ygrid) {
			data->region_type = MBV_REGION_ONEPOINT;
			XBell(view->dpy, 100);
		}

		/* use any good pair of points */
		else if (found) {
			/* set the second endpoint */
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
		} else /* if (!found) */ {
			/* ignore a bad point */
			XBell(view->dpy, 100);
		}
	}

	/* deal with definition or change of cornerpoint 1 */
	else if (which == MBV_REGION_MOVE && data->region_pickcorner == MBV_REGION_PICKCORNER1) {
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay, &zdisplay);
		/*fprintf(stderr,"CHANGE REGION: corner1: xgrid:%f ygrid:%f xlon:%f ylat:%f zdata:%f display: %f %f %f\n",
		xgrid,ygrid,xlon,ylat,zdata,xdisplay,ydisplay,zdisplay);*/

		/* ignore an identical pair of points */
		if (found && data->region.cornerpoints[2].xgrid == xgrid && data->region.cornerpoints[2].ygrid == ygrid) {
			data->region_type = MBV_REGION_ONEPOINT;
			XBell(view->dpy, 100);
		}

		/* use any good pair of points */
		else if (found) {
			/* set the second endpoint */
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
		} else /* if (!found) */ {
			/* ignore a bad point */
			XBell(view->dpy, 100);
		}
	}

	/* deal with definition or change of cornerpoint 2 */
	else if (which == MBV_REGION_MOVE && data->region_pickcorner == MBV_REGION_PICKCORNER2) {
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay, &zdisplay);
		/*fprintf(stderr,"CHANGE REGION: corner2: xgrid:%f ygrid:%f xlon:%f ylat:%f zdata:%f display: %f %f %f\n",
		xgrid,ygrid,xlon,ylat,zdata,xdisplay,ydisplay,zdisplay);*/

		/* ignore an identical pair of points */
		if (found && data->region.cornerpoints[1].xgrid == xgrid && data->region.cornerpoints[1].ygrid == ygrid) {
			data->region_type = MBV_REGION_ONEPOINT;
			XBell(view->dpy, 100);
		}

		/* use any good pair of points */
		else if (found) {
			/* set the second endpoint */
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
		} else /* if (!found) */ {
			/* ignore a bad point */
			XBell(view->dpy, 100);
		}
	}

	/* deal with definition or change of cornerpoint 3 */
	else if (which == MBV_REGION_MOVE && data->region_pickcorner == MBV_REGION_PICKCORNER3) {
		/* look for point */
		mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay, &zdisplay);
		/*fprintf(stderr,"CHANGE REGION: corner3: xgrid:%f ygrid:%f xlon:%f ylat:%f zdata:%f display: %f %f %f\n",
		xgrid,ygrid,xlon,ylat,zdata,xdisplay,ydisplay,zdisplay);*/

		/* ignore an identical pair of points */
		if (found && data->region.cornerpoints[0].xgrid == xgrid && data->region.cornerpoints[0].ygrid == ygrid) {
			data->region_type = MBV_REGION_ONEPOINT;
			XBell(view->dpy, 100);
		}

		/* use any good pair of points */
		else if (found) {
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
		} else /* if (!found) */ {
			/* ignore a bad point */
			XBell(view->dpy, 100);
		}
	}

	bool ok;

	/* recalculate any good quad region */
	if (data->region_type == MBV_REGION_QUAD && which != MBV_REGION_UP) {
		/* if needed define corners 1 and 2 in grid coordinates */
		if (data->region_pickcorner == MBV_REGION_PICKCORNER0 || data->region_pickcorner == MBV_REGION_PICKCORNER3) {
			data->region.cornerpoints[1].xgrid = data->region.cornerpoints[0].xgrid;
			data->region.cornerpoints[1].ygrid = data->region.cornerpoints[3].ygrid;
			mbview_getzdata(instance, data->region.cornerpoints[1].xgrid, data->region.cornerpoints[1].ygrid, &ok,
			                &(data->region.cornerpoints[1].zdata));
			if (!ok)
				data->region.cornerpoints[1].zdata =
				    0.5 * (data->region.cornerpoints[0].zdata + data->region.cornerpoints[3].zdata);
			mbview_projectforward(instance, true, data->region.cornerpoints[1].xgrid, data->region.cornerpoints[1].ygrid,
			                      data->region.cornerpoints[1].zdata, &(data->region.cornerpoints[1].xlon),
			                      &(data->region.cornerpoints[1].ylat), &(data->region.cornerpoints[1].xdisplay),
			                      &(data->region.cornerpoints[1].ydisplay), &(data->region.cornerpoints[1].zdisplay));

			data->region.cornerpoints[2].xgrid = data->region.cornerpoints[3].xgrid;
			data->region.cornerpoints[2].ygrid = data->region.cornerpoints[0].ygrid;
			mbview_getzdata(instance, data->region.cornerpoints[2].xgrid, data->region.cornerpoints[2].ygrid, &ok,
			                &(data->region.cornerpoints[2].zdata));
			if (!ok)
				data->region.cornerpoints[2].zdata =
				    0.5 * (data->region.cornerpoints[0].zdata + data->region.cornerpoints[3].zdata);
			mbview_projectforward(instance, true, data->region.cornerpoints[2].xgrid, data->region.cornerpoints[2].ygrid,
			                      data->region.cornerpoints[2].zdata, &(data->region.cornerpoints[2].xlon),
			                      &(data->region.cornerpoints[2].ylat), &(data->region.cornerpoints[2].xdisplay),
			                      &(data->region.cornerpoints[2].ydisplay), &(data->region.cornerpoints[2].zdisplay));
		}

		/* if needed define corners 0 and 3 in grid coordinates */
		if (data->region_pickcorner == MBV_REGION_PICKCORNER1 || data->region_pickcorner == MBV_REGION_PICKCORNER2) {
			data->region.cornerpoints[0].xgrid = data->region.cornerpoints[2].xgrid;
			data->region.cornerpoints[0].ygrid = data->region.cornerpoints[1].ygrid;
			mbview_getzdata(instance, data->region.cornerpoints[0].xgrid, data->region.cornerpoints[0].ygrid, &ok,
			                &(data->region.cornerpoints[0].zdata));
			if (!ok)
				data->region.cornerpoints[0].zdata =
				    0.5 * (data->region.cornerpoints[1].zdata + data->region.cornerpoints[2].zdata);
			mbview_projectforward(instance, true, data->region.cornerpoints[0].xgrid, data->region.cornerpoints[0].ygrid,
			                      data->region.cornerpoints[0].zdata, &(data->region.cornerpoints[0].xlon),
			                      &(data->region.cornerpoints[0].ylat), &(data->region.cornerpoints[0].xdisplay),
			                      &(data->region.cornerpoints[0].ydisplay), &(data->region.cornerpoints[0].zdisplay));

			data->region.cornerpoints[3].xgrid = data->region.cornerpoints[1].xgrid;
			data->region.cornerpoints[3].ygrid = data->region.cornerpoints[2].ygrid;
			mbview_getzdata(instance, data->region.cornerpoints[3].xgrid, data->region.cornerpoints[3].ygrid, &ok,
			                &(data->region.cornerpoints[3].zdata));
			if (!ok)
				data->region.cornerpoints[3].zdata =
				    0.5 * (data->region.cornerpoints[1].zdata + data->region.cornerpoints[2].zdata);
			mbview_projectforward(instance, true, data->region.cornerpoints[3].xgrid, data->region.cornerpoints[3].ygrid,
			                      data->region.cornerpoints[3].zdata, &(data->region.cornerpoints[3].xlon),
			                      &(data->region.cornerpoints[3].ylat), &(data->region.cornerpoints[3].xdisplay),
			                      &(data->region.cornerpoints[3].ydisplay), &(data->region.cornerpoints[3].zdisplay));
		}

		/* calculate width and length */
		if (data->display_projection_mode != MBV_PROJECTION_SPHEROID) {
			data->region.width =
			    fabs(data->region.cornerpoints[3].xdisplay - data->region.cornerpoints[0].xdisplay) / view->scale;
			data->region.height =
			    fabs(data->region.cornerpoints[3].ydisplay - data->region.cornerpoints[0].ydisplay) / view->scale;
		}
		else {
			mbview_greatcircle_distbearing(instance, data->region.cornerpoints[0].xlon, data->region.cornerpoints[0].ylat,
			                               data->region.cornerpoints[2].xlon, data->region.cornerpoints[2].ylat, &bearing,
			                               &data->region.width);
			mbview_greatcircle_distbearing(instance, data->region.cornerpoints[0].xlon, data->region.cornerpoints[0].ylat,
			                               data->region.cornerpoints[1].xlon, data->region.cornerpoints[1].ylat, &bearing,
			                               &data->region.height);
		}

		/* reset segment endpoints */
		for (int i = 0; i < 4; i++) {
			int k;
			if (i == 0)
				k = 1;
			else if (i == 1)
				k = 3;
			else if (i == 2)
				k = 0;
			else if (i == 3)
				k = 2;
			else {
				// Should never get here.
				k = 0;
				assert(false);
			}

			data->region.segments[i].endpoints[0] = data->region.cornerpoints[i];
			data->region.segments[i].endpoints[1] = data->region.cornerpoints[k];
		}

		/* set pick info */
		data->pickinfo_mode = MBV_PICK_REGION;

		/* set pick annotation */
		mbview_pick_text(instance);
	}

	/* now set and drape the segments
	    if either 3D display
	    or the pick move is final  */
	if (data->region_type == MBV_REGION_QUAD && (data->display_mode == MBV_DISPLAY_3D || which == MBV_REGION_UP)) {
		for (int i = 0; i < 4; i++) {
			/* drape the segment */
			mbview_drapesegment(instance, &(data->region.segments[i]));
		}
	}

	/* call pick notify if defined */
	if (which == MBV_REGION_UP && data->region_type == MBV_REGION_QUAD && data->mbview_pickregion_notify != NULL) {
		(data->mbview_pickregion_notify)(instance);
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
int mbview_area(size_t instance, int which, int xpixel, int ypixel) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       which:            %d\n", which);
		fprintf(stderr, "dbg2       xpixel:           %d\n", xpixel);
		fprintf(stderr, "dbg2       ypixel:           %d\n", ypixel);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* check to see if pick is at existing end points */
	if (which == MBV_AREALENGTH_DOWN && data->area_type == MBV_AREA_QUAD) {
		/* look for match to existing endpoints in neighborhood of pick */
		bool match = false;
		bool match0 = false;
		bool match1 = false;

		/* look for point */
	  bool found = false;
  	double xgrid, ygrid;
  	double xlon, ylat, zdata;
  	double xdisplay, ydisplay, zdisplay;
		mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay, &zdisplay);

		const double dx = 0.10 * (data->area.endpoints[1].xdisplay - data->area.endpoints[0].xdisplay);
		const double dy = 0.10 * (data->area.endpoints[1].ydisplay - data->area.endpoints[0].ydisplay);
		const double dd = MAX(dx, dy);
		if (found) {
			if (fabs(xdisplay - data->area.endpoints[0].xdisplay) < dd &&
			    fabs(ydisplay - data->area.endpoints[0].ydisplay) < dd) {
				match = true;
				match0 = true;
			}
			else if (fabs(xdisplay - data->area.endpoints[1].xdisplay) < dd &&
			         fabs(ydisplay - data->area.endpoints[1].ydisplay) < dd) {
				match = true;
				match1 = true;
			}
		}

		/* if no match then start new area */
		if (!match) {
			/* look for point */
    	bool found = false;
    	double xgrid, ygrid;
    	double xlon, ylat, zdata;
    	double xdisplay, ydisplay, zdisplay;
			mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay,
			                 &zdisplay);

			/* use any good point */
			if (found) {
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
		else if (match0) {
			/* look for point */
    	bool found = false;
    	double xgrid, ygrid;
    	double xlon, ylat, zdata;
    	double xdisplay, ydisplay, zdisplay;
			mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay,
			                 &zdisplay);

			/* use any good point */
			if (found) {
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
		else if (match1) {
			/* look for point */
    	bool found = false;
    	double xgrid, ygrid;
    	double xlon, ylat, zdata;
    	double xdisplay, ydisplay, zdisplay;
			mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay,
			                 &zdisplay);

			/* use any good point */
			if (found) {
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
	else if ((which == MBV_AREALENGTH_DOWN || which == MBV_AREALENGTH_MOVE) && data->area_type == MBV_AREA_NONE) {
		/* look for point */
  	bool found = false;
  	double xgrid, ygrid;
  	double xlon, ylat, zdata;
  	double xdisplay, ydisplay, zdisplay;
		mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay, &zdisplay);

		/* use any good point */
		if (found) {
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
	else if (which == MBV_AREALENGTH_MOVE && data->area_pickendpoint == MBV_AREA_PICKENDPOINT0) {
		/* look for point */
  	bool found = false;
  	double xgrid, ygrid;
  	double xlon, ylat, zdata;
  	double xdisplay, ydisplay, zdisplay;
		mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay, &zdisplay);

		/* ignore an identical pair of points */
		if (found && data->area.endpoints[1].xgrid == xgrid && data->area.endpoints[1].ygrid == ygrid) {
			data->area_type = MBV_AREA_ONEPOINT;
			data->area_pickendpoint = MBV_AREA_PICKENDPOINT0;
			XBell(view->dpy, 100);
		}

		/* use any good pair of points */
		else if (found) {
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
		} else /* if (!found) */ {
			/* ignore a bad point */
			XBell(view->dpy, 100);
		}
	}

	/* deal with definition or change of second endpoint */
	else if (which == MBV_AREALENGTH_MOVE && data->area_pickendpoint == MBV_AREA_PICKENDPOINT1) {
		/* look for point */
  	bool found = false;
  	double xgrid, ygrid;
  	double xlon, ylat, zdata;
  	double xdisplay, ydisplay, zdisplay;
		mbview_findpoint(instance, xpixel, ypixel, &found, &xgrid, &ygrid, &xlon, &ylat, &zdata, &xdisplay, &ydisplay, &zdisplay);

		/* ignore an identical pair of points */
		if (found && data->area.endpoints[0].xgrid == xgrid && data->area.endpoints[0].ygrid == ygrid) {
			data->area_type = MBV_AREA_ONEPOINT;
			data->area_pickendpoint = MBV_AREA_PICKENDPOINT1;
			XBell(view->dpy, 100);
		}

		/* use any good pair of points */
		else if (found) {
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
		} else /* if (!found) */ {
			/* ignore a bad point */
			XBell(view->dpy, 100);
		}
	}

	int status = MB_SUCCESS;

	/* recalculate any good quad area whether defined this time or previously
	    this catches which == MBV_AREAASPECT_CHANGE calls */
	if (data->area_type == MBV_AREA_QUAD && which != MBV_AREALENGTH_UP && which != MBV_AREAASPECT_UP) {
		/* deal with non-spheroid case */
		if (data->display_projection_mode != MBV_PROJECTION_SPHEROID) {
			/* now define the quad corners in display coordinates */
			const double dx = data->area.endpoints[1].xdisplay - data->area.endpoints[0].xdisplay;
			const double dy = data->area.endpoints[1].ydisplay - data->area.endpoints[0].ydisplay;
			const double dxuse = 0.5 * view->areaaspect * dy;
			const double dyuse = 0.5 * view->areaaspect * dx;

			data->area.cornerpoints[0].xdisplay = data->area.endpoints[0].xdisplay - dxuse;
			data->area.cornerpoints[0].ydisplay = data->area.endpoints[0].ydisplay + dyuse;
			data->area.cornerpoints[1].xdisplay = data->area.endpoints[0].xdisplay + dxuse;
			data->area.cornerpoints[1].ydisplay = data->area.endpoints[0].ydisplay - dyuse;
			data->area.cornerpoints[2].xdisplay = data->area.endpoints[1].xdisplay + dxuse;
			data->area.cornerpoints[2].ydisplay = data->area.endpoints[1].ydisplay - dyuse;
			data->area.cornerpoints[3].xdisplay = data->area.endpoints[1].xdisplay - dxuse;
			data->area.cornerpoints[3].ydisplay = data->area.endpoints[1].ydisplay + dyuse;

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

			/* reset segment endpoints */
			for (int i = 0; i < 2; i++) {
				data->area.segment.endpoints[i] = data->area.endpoints[i];
			}
			for (int i = 0; i < 4; i++) {
				int k = i + 1;
				if (k > 3)
					k = 0;
				data->area.segments[i].endpoints[0] = data->area.cornerpoints[i];
				data->area.segments[i].endpoints[1] = data->area.cornerpoints[k];
			}

			/* now project the segment endpoints */
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 2; j++) {
					mbview_projectinverse(
					    instance, true, data->area.segments[i].endpoints[j].xdisplay,
					    data->area.segments[i].endpoints[j].ydisplay, data->area.segments[i].endpoints[j].zdisplay,
					    &(data->area.segments[i].endpoints[j].xlon), &(data->area.segments[i].endpoints[j].ylat),
					    &(data->area.segments[i].endpoints[j].xgrid), &(data->area.segments[i].endpoints[j].ygrid));
					bool ok;
					mbview_getzdata(instance, data->area.segments[i].endpoints[j].xgrid,
					                data->area.segments[i].endpoints[j].ygrid, &ok, &(data->area.segments[i].endpoints[j].zdata));
					if (!ok && ((i == 0) || (i == 1 && j == 0) || (i == 3 && j == 1)))
						data->area.segments[i].endpoints[j].zdata = data->area.endpoints[0].zdata;
					else if (!ok)
						data->area.segments[i].endpoints[j].zdata = data->area.endpoints[1].zdata;
					mbview_projectll2display(
					    instance, data->area.segments[i].endpoints[j].xlon, data->area.segments[i].endpoints[j].ylat,
					    data->area.segments[i].endpoints[j].zdata, &data->area.segments[i].endpoints[j].xdisplay,
					    &data->area.segments[i].endpoints[j].ydisplay, &data->area.segments[i].endpoints[j].zdisplay);
				}
			}
		}

		/* else deal with spheroid case */
		else {
			/* now get length and bearing of center line */
			mbview_greatcircle_distbearing(instance, data->area.endpoints[0].xlon, data->area.endpoints[0].ylat,
			                               data->area.endpoints[1].xlon, data->area.endpoints[1].ylat, &data->area.bearing,
			                               &data->area.length);
			data->area.width = view->areaaspect * data->area.length;

			/* the corners of the area are defined by great
			    circle arcs perpendicular to the center line */

			double bearing = data->area.bearing - 90.0;
			if (bearing < 0.0)
				bearing += 360.0;
			if (bearing > 360.0)
				bearing -= 360.0;
			mbview_greatcircle_endposition(instance, data->area.endpoints[0].xlon, data->area.endpoints[0].ylat, bearing,
			                               (0.5 * data->area.width), &(data->area.cornerpoints[0].xlon),
			                               &(data->area.cornerpoints[0].ylat)),
			    status = mbview_projectll2xyzgrid(instance, data->area.cornerpoints[0].xlon, data->area.cornerpoints[0].ylat,
			                                      &(data->area.cornerpoints[0].xgrid), &(data->area.cornerpoints[0].ygrid),
			                                      &(data->area.cornerpoints[0].zdata));
			status = mbview_projectll2display(instance, data->area.cornerpoints[0].xlon, data->area.cornerpoints[0].ylat,
			                                  data->area.cornerpoints[0].zdata, &data->area.cornerpoints[0].xdisplay,
			                                  &data->area.cornerpoints[0].ydisplay, &data->area.cornerpoints[0].zdisplay);

			bearing = data->area.bearing + 90.0;
			if (bearing < 0.0)
				bearing += 360.0;
			if (bearing > 360.0)
				bearing -= 360.0;
			mbview_greatcircle_endposition(instance, data->area.endpoints[0].xlon, data->area.endpoints[0].ylat, bearing,
			                               (0.5 * data->area.width), &(data->area.cornerpoints[1].xlon),
			                               &(data->area.cornerpoints[1].ylat)),
			    status = mbview_projectll2xyzgrid(instance, data->area.cornerpoints[1].xlon, data->area.cornerpoints[1].ylat,
			                                      &(data->area.cornerpoints[1].xgrid), &(data->area.cornerpoints[1].ygrid),
			                                      &(data->area.cornerpoints[1].zdata));
			status = mbview_projectll2display(instance, data->area.cornerpoints[1].xlon, data->area.cornerpoints[1].ylat,
			                                  data->area.cornerpoints[1].zdata, &data->area.cornerpoints[1].xdisplay,
			                                  &data->area.cornerpoints[1].ydisplay, &data->area.cornerpoints[1].zdisplay);

			bearing = data->area.bearing + 90.0;
			if (bearing < 0.0)
				bearing += 360.0;
			if (bearing > 360.0)
				bearing -= 360.0;
			mbview_greatcircle_endposition(instance, data->area.endpoints[1].xlon, data->area.endpoints[1].ylat, bearing,
			                               (0.5 * data->area.width), &(data->area.cornerpoints[2].xlon),
			                               &(data->area.cornerpoints[2].ylat)),
			    status = mbview_projectll2xyzgrid(instance, data->area.cornerpoints[2].xlon, data->area.cornerpoints[2].ylat,
			                                      &(data->area.cornerpoints[2].xgrid), &(data->area.cornerpoints[2].ygrid),
			                                      &(data->area.cornerpoints[2].zdata));
			status = mbview_projectll2display(instance, data->area.cornerpoints[2].xlon, data->area.cornerpoints[2].ylat,
			                                  data->area.cornerpoints[2].zdata, &data->area.cornerpoints[2].xdisplay,
			                                  &data->area.cornerpoints[2].ydisplay, &data->area.cornerpoints[2].zdisplay);

			bearing = data->area.bearing - 90.0;
			if (bearing < 0.0)
				bearing += 360.0;
			if (bearing > 360.0)
				bearing -= 360.0;
			mbview_greatcircle_endposition(instance, data->area.endpoints[1].xlon, data->area.endpoints[1].ylat, bearing,
			                               (0.5 * data->area.width), &(data->area.cornerpoints[3].xlon),
			                               &(data->area.cornerpoints[3].ylat)),
			    status = mbview_projectll2xyzgrid(instance, data->area.cornerpoints[3].xlon, data->area.cornerpoints[3].ylat,
			                                      &(data->area.cornerpoints[3].xgrid), &(data->area.cornerpoints[3].ygrid),
			                                      &(data->area.cornerpoints[3].zdata));
			status = mbview_projectll2display(instance, data->area.cornerpoints[3].xlon, data->area.cornerpoints[3].ylat,
			                                  data->area.cornerpoints[3].zdata, &data->area.cornerpoints[3].xdisplay,
			                                  &data->area.cornerpoints[3].ydisplay, &data->area.cornerpoints[3].zdisplay);

			/* set pick info */
			data->pickinfo_mode = MBV_PICK_AREA;

			/* reset segment endpoints */
			for (int i = 0; i < 2; i++) {
				data->area.segment.endpoints[i] = data->area.endpoints[i];
			}
			for (int i = 0; i < 4; i++) {
				int k = i + 1;
				if (k > 3)
					k = 0;
				data->area.segments[i].endpoints[0] = data->area.cornerpoints[i];
				data->area.segments[i].endpoints[1] = data->area.cornerpoints[k];
			}

			/* now project the segment endpoints */
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 2; j++) {
					bool ok;
					mbview_getzdata(instance, data->area.segments[i].endpoints[j].xgrid,
					                data->area.segments[i].endpoints[j].ygrid, &ok, &(data->area.segments[i].endpoints[j].zdata));
					if (!ok && ((i == 0) || (i == 1 && j == 0) || (i == 3 && j == 1)))
						data->area.segments[i].endpoints[j].zdata = data->area.endpoints[0].zdata;
					else if (!ok)
						data->area.segments[i].endpoints[j].zdata = data->area.endpoints[1].zdata;
					mbview_projectll2display(
					    instance, data->area.segments[i].endpoints[j].xlon, data->area.segments[i].endpoints[j].ylat,
					    data->area.segments[i].endpoints[j].zdata, &data->area.segments[i].endpoints[j].xdisplay,
					    &data->area.segments[i].endpoints[j].ydisplay, &data->area.segments[i].endpoints[j].zdisplay);
				}
			}
		}

		/* set pick annotation */
		mbview_pick_text(instance);
	}

	/* now set and drape the segments
	    if either 3D display
	    or the pick move is final  */
	if (data->area_type == MBV_AREA_QUAD &&
	    (data->display_mode == MBV_DISPLAY_3D || which == MBV_AREALENGTH_UP || which == MBV_AREAASPECT_UP)) {
		mbview_drapesegment(instance, &(data->area.segment));
		for (int i = 0; i < 4; i++) {
			/* drape the segment */
			mbview_drapesegment(instance, &(data->area.segments[i]));
		}
	}

	/* call pick notify if defined */
	if ((which == MBV_AREALENGTH_UP || which == MBV_AREAASPECT_UP) && data->area_type == MBV_AREA_QUAD &&
	    data->mbview_pickarea_notify != NULL) {
		(data->mbview_pickarea_notify)(instance);
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_drawpick(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* draw current pick */
	if (data->pick_type != MBV_PICK_NONE) {
		/* set size of X mark for 2D case */
		double xlength = 0.0;
		if (data->display_mode == MBV_DISPLAY_2D)
			xlength = 0.05 / view->size2d;

		/* set color and linewidth */
		glColor3f(1.0, 0.0, 0.0);
		glLineWidth(3.0);

		/* plot first pick point */
		if (data->display_mode == MBV_DISPLAY_3D && data->pick.xsegments[0].nls > 0 && data->pick.xsegments[1].nls > 0) {
			glBegin(GL_LINE_STRIP);
			for (int i = 0; i < data->pick.xsegments[0].nls; i++) {
				glVertex3f((float)(data->pick.xsegments[0].lspoints[i].xdisplay),
				           (float)(data->pick.xsegments[0].lspoints[i].ydisplay),
				           (float)(data->pick.xsegments[0].lspoints[i].zdisplay));
			}
			glEnd();
			glBegin(GL_LINE_STRIP);
			for (int i = 0; i < data->pick.xsegments[1].nls; i++) {
				glVertex3f((float)(data->pick.xsegments[1].lspoints[i].xdisplay),
				           (float)(data->pick.xsegments[1].lspoints[i].ydisplay),
				           (float)(data->pick.xsegments[1].lspoints[i].zdisplay));
			}
			glEnd();
		}
		else if (data->display_mode == MBV_DISPLAY_3D) {
			glBegin(GL_LINES);
			for (int i = 0; i < 4; i++) {
				glVertex3f((float)(data->pick.xpoints[i].xdisplay), (float)(data->pick.xpoints[i].ydisplay),
				           (float)(data->pick.xpoints[i].zdisplay));
			}
			glEnd();
		}
		else {
			glBegin(GL_LINES);
			glVertex3f((float)(data->pick.endpoints[0].xdisplay - xlength), (float)(data->pick.endpoints[0].ydisplay - xlength),
			           (float)(data->pick.endpoints[0].zdisplay));
			glVertex3f((float)(data->pick.endpoints[0].xdisplay + xlength), (float)(data->pick.endpoints[0].ydisplay + xlength),
			           (float)(data->pick.endpoints[0].zdisplay));
			glVertex3f((float)(data->pick.endpoints[0].xdisplay + xlength), (float)(data->pick.endpoints[0].ydisplay - xlength),
			           (float)(data->pick.endpoints[0].zdisplay));
			glVertex3f((float)(data->pick.endpoints[0].xdisplay - xlength), (float)(data->pick.endpoints[0].ydisplay + xlength),
			           (float)(data->pick.endpoints[0].zdisplay));
			glEnd();
		}

		if (data->pick_type == MBV_PICK_TWOPOINT) {
			/* plot second pick point */
			if (data->display_mode == MBV_DISPLAY_3D && data->pick.xsegments[2].nls > 0 && data->pick.xsegments[3].nls > 0) {
				glBegin(GL_LINE_STRIP);
				for (int i = 0; i < data->pick.xsegments[2].nls; i++) {
					glVertex3f((float)(data->pick.xsegments[2].lspoints[i].xdisplay),
					           (float)(data->pick.xsegments[2].lspoints[i].ydisplay),
					           (float)(data->pick.xsegments[2].lspoints[i].zdisplay));
				}
				glEnd();
				glBegin(GL_LINE_STRIP);
				for (int i = 0; i < data->pick.xsegments[3].nls; i++) {
					glVertex3f((float)(data->pick.xsegments[3].lspoints[i].xdisplay),
					           (float)(data->pick.xsegments[3].lspoints[i].ydisplay),
					           (float)(data->pick.xsegments[3].lspoints[i].zdisplay));
				}
				glEnd();
			}
			else if (data->display_mode == MBV_DISPLAY_3D) {
				glBegin(GL_LINES);
				for (int i = 4; i < 8; i++) {
					glVertex3f((float)(data->pick.xpoints[i].xdisplay), (float)(data->pick.xpoints[i].ydisplay),
					           (float)(data->pick.xpoints[i].zdisplay));
				}
				glEnd();
			}
			else {
				glBegin(GL_LINES);
				glVertex3f((float)(data->pick.endpoints[1].xdisplay - xlength),
				           (float)(data->pick.endpoints[1].ydisplay - xlength), (float)(data->pick.endpoints[1].zdisplay));
				glVertex3f((float)(data->pick.endpoints[1].xdisplay + xlength),
				           (float)(data->pick.endpoints[1].ydisplay + xlength), (float)(data->pick.endpoints[1].zdisplay));
				glVertex3f((float)(data->pick.endpoints[1].xdisplay + xlength),
				           (float)(data->pick.endpoints[1].ydisplay - xlength), (float)(data->pick.endpoints[1].zdisplay));
				glVertex3f((float)(data->pick.endpoints[1].xdisplay - xlength),
				           (float)(data->pick.endpoints[1].ydisplay + xlength), (float)(data->pick.endpoints[1].zdisplay));
				glEnd();
			}

			/* plot line segment between pick points */
			if (data->display_mode == MBV_DISPLAY_3D && data->pick.segment.nls > 0) {
				glBegin(GL_LINE_STRIP);
				for (int i = 0; i < data->pick.segment.nls; i++) {
					glVertex3f((float)(data->pick.segment.lspoints[i].xdisplay), (float)(data->pick.segment.lspoints[i].ydisplay),
					           (float)(data->pick.segment.lspoints[i].zdisplay));
				}
				glEnd();
			}
			else {
				glBegin(GL_LINES);
				glVertex3f((float)(data->pick.endpoints[0].xdisplay), (float)(data->pick.endpoints[0].ydisplay),
				           (float)(data->pick.endpoints[0].zdisplay));
				glVertex3f((float)(data->pick.endpoints[1].xdisplay), (float)(data->pick.endpoints[1].ydisplay),
				           (float)(data->pick.endpoints[1].zdisplay));
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
int mbview_drawregion(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* draw current area */
	if (data->region_type == MBV_REGION_QUAD) {
		/* set color and linewidth */
		glColor3f(colortable_object_red[MBV_COLOR_YELLOW], colortable_object_green[MBV_COLOR_YELLOW],
		          colortable_object_blue[MBV_COLOR_YELLOW]);
		glLineWidth(3.0);

		/* plot quad segments */
		for (int i = 0; i < 4; i++) {
			if (data->display_mode == MBV_DISPLAY_3D && data->region.segments[i].nls > 2) {
				glBegin(GL_LINE_STRIP);
				for (int j = 0; j < data->region.segments[i].nls - 1; j++) {
					glVertex3f((float)(data->region.segments[i].lspoints[j].xdisplay),
					           (float)(data->region.segments[i].lspoints[j].ydisplay),
					           (float)(data->region.segments[i].lspoints[j].zdisplay));
				}
				glEnd();
			}
			else {
				glBegin(GL_LINES);
				glVertex3f((float)(data->region.segments[i].endpoints[0].xdisplay),
				           (float)(data->region.segments[i].endpoints[0].ydisplay),
				           (float)(data->region.segments[i].endpoints[0].zdisplay));
				glVertex3f((float)(data->region.segments[i].endpoints[1].xdisplay),
				           (float)(data->region.segments[i].endpoints[1].ydisplay),
				           (float)(data->region.segments[i].endpoints[1].zdisplay));
				glEnd();
#ifdef MBV_GETERRORS
				mbview_glerrorcheck(instance, 1, __func__);
#endif
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
int mbview_drawarea(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* draw current area */
	if (data->area_type == MBV_AREA_QUAD) {
		/* set color and linewidth */
		glColor3f(colortable_object_red[MBV_COLOR_YELLOW], colortable_object_green[MBV_COLOR_YELLOW],
		          colortable_object_blue[MBV_COLOR_YELLOW]);
		glLineWidth(3.0);

		/* plot center segment */
		if (data->display_mode == MBV_DISPLAY_3D && data->area.segment.nls > 2) {
			glBegin(GL_LINE_STRIP);
			for (int j = 0; j < data->area.segment.nls; j++) {
				glVertex3f((float)(data->area.segment.lspoints[j].xdisplay), (float)(data->area.segment.lspoints[j].ydisplay),
				           (float)(data->area.segment.lspoints[j].zdisplay));
			}
			glEnd();
#ifdef MBV_GETERRORS
			mbview_glerrorcheck(instance, 1, __func__);
#endif
		}
		else {
			glBegin(GL_LINES);
			glVertex3f((float)(data->area.segment.endpoints[0].xdisplay), (float)(data->area.segment.endpoints[0].ydisplay),
			           (float)(data->area.segment.endpoints[0].zdisplay));
			glVertex3f((float)(data->area.segment.endpoints[1].xdisplay), (float)(data->area.segment.endpoints[1].ydisplay),
			           (float)(data->area.segment.endpoints[1].zdisplay));
			glEnd();
#ifdef MBV_GETERRORS
			mbview_glerrorcheck(instance, 1, __func__);
#endif
		}

		/* plot quad segments */
		for (int i = 0; i < 4; i++) {
			if (data->display_mode == MBV_DISPLAY_3D && data->area.segments[i].nls > 2) {
				glBegin(GL_LINE_STRIP);
				for (int j = 0; j < data->area.segments[i].nls - 1; j++) {
					glVertex3f((float)(data->area.segments[i].lspoints[j].xdisplay),
					           (float)(data->area.segments[i].lspoints[j].ydisplay),
					           (float)(data->area.segments[i].lspoints[j].zdisplay));
				}
				glEnd();
#ifdef MBV_GETERRORS
				mbview_glerrorcheck(instance, 1, __func__);
#endif
			}
			else {
				glBegin(GL_LINES);
				glVertex3f((float)(data->area.segments[i].endpoints[0].xdisplay),
				           (float)(data->area.segments[i].endpoints[0].ydisplay),
				           (float)(data->area.segments[i].endpoints[0].zdisplay));
				glVertex3f((float)(data->area.segments[i].endpoints[1].xdisplay),
				           (float)(data->area.segments[i].endpoints[1].ydisplay),
				           (float)(data->area.segments[i].endpoints[1].zdisplay));
				glEnd();
#ifdef MBV_GETERRORS
				mbview_glerrorcheck(instance, 1, __func__);
#endif
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
