/*--------------------------------------------------------------------
 *    The MB-system:	mbview_callbacks.c	10/7/2002
 *
 *    Copyright (c) 2002-2023 by
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
 * Date:	October 7, 2002
 */

#ifndef SANS
#define SANS "helvetica"
#endif
#ifndef SERIF
#define SERIF "times"
#endif
#ifndef MONO
#define MONO "courier"
#endif

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
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <Xm/Separator.h>
#include <Xm/FileSB.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
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

/* Set flag to define mbview global variables in this code block */
#define MBVIEWGLOBAL

#include "mbview.h"
#include "mbviewprivate.h"

//#define MBV_DEBUG_GLX 1
//#define MBV_GET_GLX_ERRORS 1

#include "creation-c.h"

Widget BxFindTopShell(Widget);
WidgetList BxWidgetIdsFromNames(Widget, char *, char *);

/*------------------------------------------------------------------------------*/
int mbview_startup(int verbose, Widget parent, XtAppContext app, int *error) {
	mbv_verbose = verbose;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                 %d\n", verbose);
		fprintf(stderr, "dbg2       parent:                  %p\n", parent);
		fprintf(stderr, "dbg2       app:                     %p\n", app);
	}

	/* set parent widget and app context */
	parent_widget = parent;
	app_context = app;

	/* set global work function parameters */
	work_function_enabled = true;
	work_function_set = false;
	timer_timeout_time = 100;
	timer_timeout_count = 10;
	timer_count = 0;

	/* set starting number of windows */
	mbv_ninstance = 0;

	/* initialize shared data */
	mbview_reset_shared(true);

	/* initialize windows */
	for (int i = 0; i < MBV_MAX_WINDOWS; i++) {
		mbview_reset(i);
	}

	/* create and manage site list window */
	shared.init_sitelist = MBV_WINDOW_NULL;
	Arg args[256];
  Cardinal ac = 0;
	XtSetArg(args[ac], XmNtitle, "Site List");
	ac++;
	shared.topLevelShell_sitelist = XtCreatePopupShell("topLevelShell", topLevelShellWidgetClass, parent_widget, args, ac);
	shared.mainWindow_sitelist = XmCreateMainWindow(shared.topLevelShell_sitelist, "mainWindow_sitelist", args, ac);
	XtManageChild(shared.mainWindow_sitelist);
	MB3DSiteListCreate(&(shared.mb3d_sitelist), shared.mainWindow_sitelist, "mbview_sitelist", args, ac);
	ac = 0;
	Boolean argok = False;
	XmString tmp0 = (XmString)BX_CONVERT(shared.mb3d_sitelist.MB3DSiteList, (char *)"Site | Lon | Lat | Depth | Color | Size | Name",
	                            XmRXmString, 0, &argok);
	XtSetArg(args[ac], XmNlabelString, tmp0);
	if (argok)
		ac++;
	XtSetValues(shared.mb3d_sitelist.mbview_sitelist_label, args, ac);
	XtManageChild(shared.mb3d_sitelist.MB3DSiteList);

	/* create and manage route list window */
	shared.init_routelist = MBV_WINDOW_NULL;
	ac = 0;
	XtSetArg(args[ac], XmNtitle, "Route List");
	ac++;
	shared.topLevelShell_routelist = XtCreatePopupShell("topLevelShell", topLevelShellWidgetClass, parent_widget, args, ac);
	shared.mainWindow_routelist = XmCreateMainWindow(shared.topLevelShell_routelist, "mainWindow_routelist", args, ac);
	XtManageChild(shared.mainWindow_routelist);
	MB3DRouteListCreate(&(shared.mb3d_routelist), shared.mainWindow_routelist, "mbview_routelist", args, ac);
	ac = 0;
	tmp0 = (XmString)BX_CONVERT(shared.mb3d_routelist.MB3DRouteList,
	                            (char *)"Route | Waypoint | Lon | Lat | Depth | Distance | DistanceOverTopo | Waypoint Type",
	                            XmRXmString, 0, &argok);
	XtSetArg(args[ac], XmNlabelString, tmp0);
	if (argok)
		ac++;
	XtSetValues(shared.mb3d_routelist.mbview_routelist_label, args, ac);
	XtManageChild(shared.mb3d_routelist.MB3DRouteList);

	/* create and manage navigation list window */
	shared.init_navlist = MBV_WINDOW_NULL;
	ac = 0;
	XtSetArg(args[ac], XmNtitle, "Navigation List");
	ac++;
	shared.topLevelShell_navlist = XtCreatePopupShell("topLevelShell", topLevelShellWidgetClass, parent_widget, args, ac);
	shared.mainWindow_navlist = XmCreateMainWindow(shared.topLevelShell_navlist, "mainWindow_navlist", args, ac);
	XtManageChild(shared.mainWindow_navlist);
	MB3DNavListCreate(&(shared.mb3d_navlist), shared.mainWindow_navlist, "mbview_navlist", args, ac);
	ac = 0;
	tmp0 = (XmString)BX_CONVERT(shared.mb3d_navlist.MB3DNavList, (char *)"Route | Navpoints | Color | Size | Name", XmRXmString,
	                            0, &argok);
	XtSetArg(args[ac], XmNlabelString, tmp0);
	if (argok)
		ac++;
	XtSetValues(shared.mb3d_navlist.mbview_navlist_label, args, ac);
	XtManageChild(shared.mb3d_navlist.MB3DNavList);

	/* set timer function */
	do_mbview_settimer();

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:        %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:       %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_reset_shared(bool mode) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2       mode:                    %d\n", mode);
	}

	if (mode) {
		shared.init_sitelist = MBV_WINDOW_NULL;
		shared.topLevelShell_sitelist = NULL;
		shared.mainWindow_sitelist = NULL;
		shared.init_routelist = MBV_WINDOW_NULL;
		shared.topLevelShell_routelist = NULL;
		shared.mainWindow_routelist = NULL;
		shared.init_navlist = MBV_WINDOW_NULL;
		shared.topLevelShell_navlist = NULL;
		shared.mainWindow_navlist = NULL;
	}

	/* global lon lat print style */
	shared.lonlatstyle = MBV_LONLAT_DEGREESMINUTES;

	/* site data */
	shared.shareddata.site_mode = MBV_SITE_OFF;
	shared.shareddata.nsite = 0;
	shared.shareddata.nsite_alloc = 0;
	shared.shareddata.site_selected = MBV_SELECT_NONE;
	shared.shareddata.sites = NULL;

	/* route data */
	shared.shareddata.route_mode = MBV_ROUTE_OFF;
	shared.shareddata.nroute = 0;
	shared.shareddata.nroute_alloc = 0;
	shared.shareddata.route_selected = MBV_SELECT_NONE;
	shared.shareddata.route_point_selected = MBV_SELECT_NONE;
	shared.shareddata.routes = NULL;

	/* nav data */
	shared.shareddata.nav_mode = MBV_NAV_OFF;
	shared.shareddata.nnav = 0;
	shared.shareddata.nnav_alloc = 0;
	shared.shareddata.nav_selected[0] = MBV_SELECT_NONE;
	shared.shareddata.nav_selected[1] = MBV_SELECT_NONE;
	shared.shareddata.nav_point_selected[0] = MBV_SELECT_NONE;
	shared.shareddata.nav_point_selected[1] = MBV_SELECT_NONE;
	shared.shareddata.nav_selected_mbnavadjust[0] = MBV_SELECT_NONE;
	shared.shareddata.nav_selected_mbnavadjust[1] = MBV_SELECT_NONE;
	shared.shareddata.navs = NULL;

	for (size_t instance = 0; instance < MBV_MAX_WINDOWS; instance++) {
		/* nav pick data */
		shared.shareddata.navpick_type = MBV_PICK_NONE;
		for (int i = 0; i < 2; i++) {
			shared.shareddata.navpick.endpoints[i].xgrid[instance] = 0.0;
			shared.shareddata.navpick.endpoints[i].ygrid[instance] = 0.0;
			shared.shareddata.navpick.endpoints[i].xlon = 0.0;
			shared.shareddata.navpick.endpoints[i].ylat = 0.0;
			shared.shareddata.navpick.endpoints[i].zdata = 0.0;
			shared.shareddata.navpick.endpoints[i].xdisplay[instance] = 0.0;
			shared.shareddata.navpick.endpoints[i].ydisplay[instance] = 0.0;
			shared.shareddata.navpick.endpoints[i].zdisplay[instance] = 0.0;
			shared.shareddata.navpick.segment.endpoints[i] = shared.shareddata.navpick.endpoints[i];
		}
		shared.shareddata.navpick.segment.nls = 0;
		shared.shareddata.navpick.segment.nls_alloc = 0;
		shared.shareddata.navpick.segment.lspoints = NULL;
		for (int i = 0; i < 8; i++) {
			shared.shareddata.navpick.xpoints[i].xgrid[instance] = 0.0;
			shared.shareddata.navpick.xpoints[i].ygrid[instance] = 0.0;
			shared.shareddata.navpick.xpoints[i].xlon = 0.0;
			shared.shareddata.navpick.xpoints[i].ylat = 0.0;
			shared.shareddata.navpick.xpoints[i].zdata = 0.0;
			shared.shareddata.navpick.xpoints[i].xdisplay[instance] = 0.0;
			shared.shareddata.navpick.xpoints[i].ydisplay[instance] = 0.0;
			shared.shareddata.navpick.xpoints[i].zdisplay[instance] = 0.0;
		}
	}
	for (int j = 0; j < 4; j++) {
		shared.shareddata.navpick.xsegments[j].nls = 0;
		shared.shareddata.navpick.xsegments[j].nls_alloc = 0;
		shared.shareddata.navpick.xsegments[j].lspoints = NULL;
		for (int i = 0; i < 2; i++) {
			shared.shareddata.navpick.xsegments[j].endpoints[i] = shared.shareddata.navpick.xpoints[2 * j + i];
		}
	}

	/* vector data */
	shared.shareddata.vector_mode = MBV_VECTOR_OFF;
	shared.shareddata.nvector = 0;
	shared.shareddata.vector_selected = MBV_SELECT_NONE;
	shared.shareddata.vector_point_selected = MBV_SELECT_NONE;
	shared.shareddata.vectors = NULL;

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:       %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_reset(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:                %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	int status = MB_SUCCESS;

	/* initialize windows */
	if (instance != MBV_NO_WINDOW && instance < MBV_MAX_WINDOWS) {
		view->init = MBV_WINDOW_NULL;

		/* initialize function pointers */
		data->mbview_pickonepoint_notify = NULL;
		data->mbview_picktwopoint_notify = NULL;
		data->mbview_pickarea_notify = NULL;
		data->mbview_pickregion_notify = NULL;
		data->mbview_picksite_notify = NULL;
		data->mbview_pickroute_notify = NULL;
		data->mbview_picknav_notify = NULL;
		data->mbview_sensitivity_notify = NULL;
		data->mbview_colorchange_notify = NULL;

		/* initialize data structure */
		data->active = false;

		/* initialize mbview data */
		strcpy(data->title, "MB3DView - MBgrdviz");
		data->xo = 0;
		data->yo = 0;
		data->width = 560;
		data->height = 500;
		data->lorez_dimension = 100;
		data->hirez_dimension = 500;
		data->lorez_navdecimate = 5;
		data->hirez_navdecimate = 1;

		/* mode controls */
		data->display_mode = MBV_DISPLAY_2D;
		data->mouse_mode = MBV_MOUSE_MOVE;
		data->grid_mode = MBV_GRID_VIEW_PRIMARY;
		data->grid_contour_mode = MBV_VIEW_OFF;

		data->primary_histogram = false;
		data->primaryslope_histogram = false;
		data->secondary_histogram = false;

		data->primary_colortable = MBV_COLORTABLE_HAXBY;
		data->primary_colortable_mode = MBV_COLORTABLE_NORMAL;
		data->primary_colortable_min = 0.0;
		data->primary_colortable_max = 0.0;
		data->primary_shade_mode = MBV_SHADE_VIEW_NONE;
		data->slope_colortable = MBV_COLORTABLE_HAXBY;
		data->slope_colortable_mode = MBV_COLORTABLE_REVERSED;
		data->slope_colortable_min = 0.0;
		data->slope_colortable_max = 0.5;
		data->slope_shade_mode = MBV_SHADE_VIEW_NONE;
		data->secondary_colortable = MBV_COLORTABLE_HAXBY;
		data->secondary_colortable_mode = MBV_COLORTABLE_NORMAL;
		data->secondary_colortable_min = 0.0;
		data->secondary_colortable_max = 0.0;
		data->secondary_shade_mode = MBV_SHADE_VIEW_NONE;

		data->exageration = 1.0;
		data->modelelevation3d = 90.0;
		data->modelazimuth3d = 0.0;
		data->viewelevation3d = 90.0;
		data->viewazimuth3d = 0.0;
		data->viewbounds[0] = 0;
		data->viewbounds[1] = 0;
		data->viewbounds[2] = 0;
		data->viewbounds[3] = 0;

		/* shading controls */
		data->illuminate_magnitude = 1.0;
		data->illuminate_elevation = 5.0;
		data->illuminate_azimuth = 90.0;
		data->slope_magnitude = 1.0;
		data->overlay_shade_magnitude = 1.0;
		data->overlay_shade_center = 0.0;
		data->overlay_shade_mode = MBV_COLORTABLE_NORMAL;

		/* contour controls */
		data->contour_interval = 100.0;

		/* profile controls */
		data->profile_exageration = 1.0;
		data->profile_widthfactor = 1;
		data->profile_slopethreshold = 2.00;

		/* projection controls */
		data->primary_grid_projection_mode = MBV_PROJECTION_GEOGRAPHIC;
		strcpy(data->primary_grid_projection_id, "GEOGRAPHIC");
		data->secondary_grid_projection_mode = MBV_PROJECTION_GEOGRAPHIC;
		strcpy(data->secondary_grid_projection_id, "GEOGRAPHIC");
		data->display_projection_mode = MBV_PROJECTION_GEOGRAPHIC;
		strcpy(data->display_projection_id, "GEOGRAPHIC");

		/* grid data */
		data->primary_nodatavalue = MBV_DEFAULT_NODATA;
		data->primary_nxy = 0;
		data->primary_n_columns = 0;
		data->primary_n_rows = 0;
		data->primary_xmin = 0.0;
		data->primary_xmax = 0.0;
		data->primary_ymin = 0.0;
		data->primary_ymax = 0.0;
		data->primary_dx = 0.0;
		data->primary_dy = 0.0;
		data->primary_data = NULL;
		data->primary_x = NULL;
		data->primary_y = NULL;
		data->primary_z = NULL;
		data->primary_dzdx = NULL;
		data->primary_dzdy = NULL;
		data->primary_r = NULL;
		data->primary_g = NULL;
		data->primary_b = NULL;
		data->primary_stat_color = NULL;
		data->primary_stat_z = NULL;
		data->secondary_sameas_primary = false;
		data->secondary_nodatavalue = MBV_DEFAULT_NODATA;
		data->secondary_nxy = 0;
		data->secondary_n_columns = 0;
		data->secondary_n_rows = 0;
		data->secondary_xmin = 0.0;
		data->secondary_xmax = 0.0;
		data->secondary_ymin = 0.0;
		data->secondary_ymax = 0.0;
		data->secondary_dx = 0.0;
		data->secondary_dy = 0.0;
		data->secondary_data = NULL;

		/* pick info flag */
		data->pickinfo_mode = MBV_PICK_NONE;

		/* point and line pick data */
		data->pick_type = MBV_PICK_NONE;
		for (int i = 0; i < 2; i++) {
			data->pick.endpoints[i].xgrid = 0.0;
			data->pick.endpoints[i].ygrid = 0.0;
			data->pick.endpoints[i].xlon = 0.0;
			data->pick.endpoints[i].ylat = 0.0;
			data->pick.endpoints[i].zdata = 0.0;
			data->pick.endpoints[i].xdisplay = 0.0;
			data->pick.endpoints[i].ydisplay = 0.0;
			data->pick.endpoints[i].zdisplay = 0.0;
			data->pick.segment.endpoints[i] = data->pick.endpoints[i];
		}
		data->pick.segment.nls = 0;
		data->pick.segment.nls_alloc = 0;
		data->pick.segment.lspoints = NULL;
		for (int i = 0; i < 8; i++) {
			data->pick.xpoints[i].xgrid = 0.0;
			data->pick.xpoints[i].ygrid = 0.0;
			data->pick.xpoints[i].xlon = 0.0;
			data->pick.xpoints[i].ylat = 0.0;
			data->pick.xpoints[i].zdata = 0.0;
			data->pick.xpoints[i].xdisplay = 0.0;
			data->pick.xpoints[i].ydisplay = 0.0;
			data->pick.xpoints[i].zdisplay = 0.0;
		}
		for (int j = 0; j < 4; j++) {
			data->pick.xsegments[j].nls = 0;
			data->pick.xsegments[j].nls_alloc = 0;
			data->pick.xsegments[j].lspoints = NULL;
			for (int i = 0; i < 2; i++) {
				data->pick.xsegments[j].endpoints[i] = data->pick.xpoints[2 * j + i];
			}
		}

		/* region pick data */
		data->region_type = MBV_REGION_NONE;
		data->region.width = 0.0;
		data->region.height = 0.0;
		for (int i = 0; i < 4; i++) {
			data->region.cornerpoints[i].xgrid = 0.0;
			data->region.cornerpoints[i].ygrid = 0.0;
			data->region.cornerpoints[i].xlon = 0.0;
			data->region.cornerpoints[i].ylat = 0.0;
			data->region.cornerpoints[i].zdata = 0.0;
			data->region.cornerpoints[i].xdisplay = 0.0;
			data->region.cornerpoints[i].ydisplay = 0.0;
			data->region.cornerpoints[i].zdisplay = 0.0;
		}
		int ii;
		for (int i = 0; i < 4; i++) {
			if (i == 0)
				ii = 1;
			else if (i == 1)
				ii = 3;
			else if (i == 2)
				ii = 0;
			else if (i == 3)
				ii = 2;
			data->region.segments[i].endpoints[0] = data->region.cornerpoints[i];
			data->region.segments[i].endpoints[1] = data->region.cornerpoints[ii];
			data->region.segments[i].nls = 0;
			data->region.segments[i].nls_alloc = 0;
			data->region.segments[i].lspoints = NULL;
		}

		/* area pick data */
		data->area_type = MBV_AREA_NONE;
		data->area.width = 0.0;
		data->area.length = 0.0;
		data->area.bearing = 0.0;
		for (int i = 0; i < 2; i++) {
			data->area.endpoints[i].xgrid = 0.0;
			data->area.endpoints[i].ygrid = 0.0;
			data->area.endpoints[i].xlon = 0.0;
			data->area.endpoints[i].ylat = 0.0;
			data->area.endpoints[i].zdata = 0.0;
			data->area.endpoints[i].xdisplay = 0.0;
			data->area.endpoints[i].ydisplay = 0.0;
			data->area.endpoints[i].zdisplay = 0.0;
			data->area.segment.endpoints[i] = data->area.endpoints[i];
		}
		data->area.segment.nls = 0;
		data->area.segment.nls_alloc = 0;
		data->area.segment.lspoints = NULL;
		for (int i = 0; i < 4; i++) {
			data->area.cornerpoints[i].xgrid = 0.0;
			data->area.cornerpoints[i].ygrid = 0.0;
			data->area.cornerpoints[i].xlon = 0.0;
			data->area.cornerpoints[i].ylat = 0.0;
			data->area.cornerpoints[i].zdata = 0.0;
			data->area.cornerpoints[i].xdisplay = 0.0;
			data->area.cornerpoints[i].ydisplay = 0.0;
			data->area.cornerpoints[i].zdisplay = 0.0;
		}
		for (int i = 0; i < 4; i++) {
			ii = i + 1;
			if (ii > 3)
				ii = 0;
			data->area.segments[i].endpoints[0] = data->area.cornerpoints[i];
			data->area.segments[i].endpoints[1] = data->area.cornerpoints[ii];
			data->area.segments[i].nls = 0;
			data->area.segments[i].nls_alloc = 0;
			data->area.segments[i].lspoints = NULL;
		}

		/* site data */
		data->site_view_mode = MBV_VIEW_OFF;

		/* route data */
		data->route_view_mode = MBV_VIEW_OFF;

		/* nav data */
		data->nav_view_mode = MBV_VIEW_OFF;
		data->navdrape_view_mode = MBV_VIEW_OFF;

		/* vector data */
		data->vector_view_mode = MBV_VIEW_OFF;

		/* profile data */
		data->profile_view_mode = MBV_VIEW_OFF;

    /* general use state variables to turn action button sensitivity on and off */
    data->state13 = MBV_VIEW_OFF;
    data->state14 = MBV_VIEW_OFF;
    data->state15 = MBV_VIEW_OFF;
    data->state16 = MBV_VIEW_OFF;
    data->state17 = MBV_VIEW_OFF;
    data->state18 = MBV_VIEW_OFF;
    data->state19 = MBV_VIEW_OFF;
    data->state20 = MBV_VIEW_OFF;
    data->state21 = MBV_VIEW_OFF;
    data->state22 = MBV_VIEW_OFF;
    data->state23 = MBV_VIEW_OFF;
    data->state24 = MBV_VIEW_OFF;
    data->state25 = MBV_VIEW_OFF;
    data->state26 = MBV_VIEW_OFF;
    data->state27 = MBV_VIEW_OFF;
    data->state28 = MBV_VIEW_OFF;
    data->state29 = MBV_VIEW_OFF;
    data->state30 = MBV_VIEW_OFF;
    data->state31 = MBV_VIEW_OFF;

		/* set mbview default values */
		status = mb_mbview_defaults(mbv_verbose, &data->primary_colortable, &data->primary_colortable_mode,
		                            &data->primary_shade_mode, &data->slope_colortable, &data->slope_colortable_mode,
		                            &data->secondary_colortable, &data->secondary_colortable_mode, &data->illuminate_magnitude,
		                            &data->illuminate_elevation, &data->illuminate_azimuth, &data->slope_magnitude);

		/* windows */
		view->topLevelShell = NULL;
		view->mainWindow = NULL;
		view->glwmda = NULL;
		view->prglwmda = NULL;
		view->dpy = NULL;
		view->glx_init = false;
		view->prglx_init = false;
		view->message_on = false;
		view->plot_recursion = 0;
		view->plot_done = false;
		view->plot_interrupt_allowed = true;
		view->naction = 0;
		for (int i = 0; i < MBV_NUM_ACTIONS; i++) {
			view->actionsensitive[i] = 0;
			view->pushButton_action[i] = NULL;
		}

		/* drawing variables */
		view->gl_width = 0;
		view->gl_height = 0;
		view->projected = false;
		view->globalprojected = false;
		view->lastdrawrez = MBV_REZ_NONE;
		view->viewboundscount = MBV_BOUNDSFREQUENCY;
		mbview_zscaleclear(instance);
		mbview_setcolorparms(instance);
		mbview_colorclear(instance);
		view->contourlorez = false;
		view->contourhirez = false;
		view->contourfullrez = false;
		view->primary_histogram_set = false;
		view->primaryslope_histogram_set = false;
		view->secondary_histogram_set = false;

		/* grid display bounds */
		view->xmin = 0.0;
		view->xmax = 0.0;
		view->ymin = 0.0;
		view->ymax = 0.0;
		view->xorigin = 0.0;
		view->yorigin = 0.0;
		view->zorigin = 0.0;
		view->scale = 0.0;

		view->offset2d_x = 0.0;
		view->offset2d_y = 0.0;
		view->offset2d_x_save = 0.0;
		view->offset2d_y_save = 0.0;
		view->size2d = 0.0;
		view->size2d_save = 0.0;
		view->offset3d_x = 0.0;
		view->offset3d_y = 0.0;
		view->offset3d_z = 0.0;
		view->viewoffset3d_z = 0.0;
		view->offset3d_x_save = 0.0;
		view->offset3d_y_save = 0.0;
		view->offset3d_z_save = 0.0;
		view->viewoffset3d_z_save = 0.0;
		view->areaaspect = 0.5;
		view->areaaspect_save = 0.5;
		view->exageration_save = 0.0;
		view->modelelevation3d_save = 0.0;
		view->modelazimuth3d_save = 0.0;
		view->viewelevation3d_save = 0.0;
		view->viewazimuth3d_save = 0.0;
		view->illuminate_magnitude_save = 0.0;
		view->illuminate_elevation_save = 0.0;
		view->illuminate_azimuth_save = 0.0;
		view->slope_magnitude_save = 0.0;
		view->overlay_shade_magnitude_save = 0.0;

		/* set mbio default values */
		int dummy_format;
		int dummy_pings;
		double dummy_bounds[4];
		int dummy_btime_i[7];
		int dummy_etime_i[7];
		double dummy_speedmin;
		status = mb_defaults(mbv_verbose, &dummy_format, &dummy_pings, &(view->lonflip), dummy_bounds, dummy_btime_i,
		                     dummy_etime_i, &dummy_speedmin, &(view->timegap));
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:       %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_init(int verbose, size_t *instance, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
	}

	/* get next instance number */
	*instance = MBV_NO_WINDOW;
	for (size_t i = 0; i < MBV_MAX_WINDOWS; i++) {
		if (*instance == MBV_NO_WINDOW && mbviews[i].init != MBV_WINDOW_VISIBLE)
			*instance = i;
	}

	if (*instance == MBV_NO_WINDOW) {
		fprintf(stderr, "Unable to create mbview - all %d mbview windows already in use.\n", MBV_MAX_WINDOWS);
		return (MB_FAILURE);
	}

	struct mbview_world_struct *view = &(mbviews[*instance]);

	/* copy control structure */
	view->mainWindow = parent_widget;
	mbv_ninstance++;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       instance:                  %zu\n", *instance);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_getdataptr(int verbose, size_t instance, struct mbview_struct **datahandle, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	*datahandle = &(view->data);
	struct mbview_struct *data = &(view->data);

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       datahandle:                %p\n", *datahandle);

		/* widget controls */
		fprintf(stderr, "dbg2       title:                     %s\n", data->title);
		fprintf(stderr, "dbg2       xo:                        %d\n", data->xo);
		fprintf(stderr, "dbg2       yo:                        %d\n", data->yo);
		fprintf(stderr, "dbg2       width:                     %d\n", data->width);
		fprintf(stderr, "dbg2       height:                    %d\n", data->height);
		fprintf(stderr, "dbg2       lorez_dimension:           %d\n", data->lorez_dimension);
		fprintf(stderr, "dbg2       hirez_dimension:           %d\n", data->hirez_dimension);
		fprintf(stderr, "dbg2       lorez_navdecimate:         %d\n", data->lorez_navdecimate);
		fprintf(stderr, "dbg2       hirez_navdecimate:         %d\n", data->hirez_navdecimate);

		/* mode controls */
		fprintf(stderr, "dbg2       display_mode:              %d\n", data->display_mode);
		fprintf(stderr, "dbg2       mouse_mode:                %d\n", data->mouse_mode);
		fprintf(stderr, "dbg2       grid_mode:                 %d\n", data->grid_mode);
		fprintf(stderr, "dbg2       grid_contour_mode:         %d\n", data->grid_contour_mode);

		/* colortable controls */
		fprintf(stderr, "dbg2       primary_colortable:        %d\n", data->primary_colortable);
		fprintf(stderr, "dbg2       primary_colortable_mode:   %d\n", data->primary_colortable_mode);
		fprintf(stderr, "dbg2       primary_colortable_min:    %f\n", data->primary_colortable_min);
		fprintf(stderr, "dbg2       primary_colortable_max:    %f\n", data->primary_colortable_max);
		fprintf(stderr, "dbg2       primary_shade_mode:        %d\n", data->primary_shade_mode);
		fprintf(stderr, "dbg2       slope_colortable:          %d\n", data->slope_colortable);
		fprintf(stderr, "dbg2       slope_colortable_mode:     %d\n", data->slope_colortable_mode);
		fprintf(stderr, "dbg2       slope_colortable_min:      %f\n", data->slope_colortable_min);
		fprintf(stderr, "dbg2       slope_colortable_max:      %f\n", data->slope_colortable_max);
		fprintf(stderr, "dbg2       slope_shade_mode:          %d\n", data->slope_shade_mode);
		fprintf(stderr, "dbg2       secondary_colortable:      %d\n", data->secondary_colortable);
		fprintf(stderr, "dbg2       secondary_colortable_mode: %d\n", data->secondary_colortable_mode);
		fprintf(stderr, "dbg2       secondary_colortable_min:  %f\n", data->secondary_colortable_min);
		fprintf(stderr, "dbg2       secondary_colortable_max:  %f\n", data->secondary_colortable_max);
		fprintf(stderr, "dbg2       secondary_shade_mode:      %d\n", data->secondary_shade_mode);

		/* view controls */
		fprintf(stderr, "dbg2       exageration:               %f\n", data->exageration);
		fprintf(stderr, "dbg2       modelelevation3d:          %f\n", data->modelelevation3d);
		fprintf(stderr, "dbg2       modelazimuth3d:            %f\n", data->modelazimuth3d);
		fprintf(stderr, "dbg2       viewelevation3d:           %f\n", data->viewelevation3d);
		fprintf(stderr, "dbg2       viewazimuth3d:             %f\n", data->viewazimuth3d);

		/* shading controls */
		fprintf(stderr, "dbg2       illuminate_magnitude:      %f\n", data->illuminate_magnitude);
		fprintf(stderr, "dbg2       illuminate_elevation:      %f\n", data->illuminate_elevation);
		fprintf(stderr, "dbg2       illuminate_azimuth:        %f\n", data->illuminate_azimuth);
		fprintf(stderr, "dbg2       slope_magnitude:           %f\n", data->slope_magnitude);

		/* contour controls */
		fprintf(stderr, "dbg2       contour_interval:           %f\n", data->slope_magnitude);

		/* profile controls */
		fprintf(stderr, "dbg2       profile_exageration:        %f\n", data->profile_exageration);
		fprintf(stderr, "dbg2       profile_widthfactor:        %d\n", data->profile_widthfactor);
		fprintf(stderr, "dbg2       profile_slopethreshold:     %f\n", data->profile_slopethreshold);

		/* projection controls */
		fprintf(stderr, "dbg2       primary_grid_projection_mode:   %d\n", data->primary_grid_projection_mode);
		fprintf(stderr, "dbg2       primary_grid_projection_id:     %s\n", data->primary_grid_projection_id);
		fprintf(stderr, "dbg2       secondary_grid_projection_mode: %d\n", data->secondary_grid_projection_mode);
		fprintf(stderr, "dbg2       secondary_grid_projection_id:   %s\n", data->secondary_grid_projection_id);
		fprintf(stderr, "dbg2       display_projection_mode:        %d\n", data->display_projection_mode);
		fprintf(stderr, "dbg2       display_projection_id:          %s\n", data->display_projection_id);

		/* primary grid data */
		fprintf(stderr, "dbg2       primary_nodatavalue:       %f\n", data->primary_nodatavalue);
		fprintf(stderr, "dbg2       primary_nxy:               %d\n", data->primary_nxy);
		fprintf(stderr, "dbg2       primary_n_columns:         %d\n", data->primary_n_columns);
		fprintf(stderr, "dbg2       primary_n_rows:            %d\n", data->primary_n_rows);
		fprintf(stderr, "dbg2       primary_min:               %f\n", data->primary_min);
		fprintf(stderr, "dbg2       primary_max:               %f\n", data->primary_max);
		fprintf(stderr, "dbg2       primary_xmin:              %f\n", data->primary_xmin);
		fprintf(stderr, "dbg2       primary_xmax:              %f\n", data->primary_xmax);
		fprintf(stderr, "dbg2       primary_ymin:              %f\n", data->primary_ymin);
		fprintf(stderr, "dbg2       primary_ymax:              %f\n", data->primary_ymax);
		fprintf(stderr, "dbg2       primary_dx:                %f\n", data->primary_dx);
		fprintf(stderr, "dbg2       primary_dy:                %f\n", data->primary_dy);
		fprintf(stderr, "dbg2       primary_data:              %p\n", data->primary_data);
		fprintf(stderr, "dbg2       primary_x:                 %p\n", data->primary_x);
		fprintf(stderr, "dbg2       primary_y:                 %p\n", data->primary_y);
		fprintf(stderr, "dbg2       primary_z:                 %p\n", data->primary_z);
		fprintf(stderr, "dbg2       primary_dxdz:              %p\n", data->primary_dzdx);
		fprintf(stderr, "dbg2       primary_dydz:              %p\n", data->primary_dzdy);
		fprintf(stderr, "dbg2       primary_r:                 %p\n", data->primary_r);
		fprintf(stderr, "dbg2       primary_g:                 %p\n", data->primary_g);
		fprintf(stderr, "dbg2       primary_b:                 %p\n", data->primary_b);
		fprintf(stderr, "dbg2       primary_stat_color:        %p\n", data->primary_stat_color);
		fprintf(stderr, "dbg2       primary_stat_z:            %p\n", data->primary_stat_z);

		/* secondary grid data */
		fprintf(stderr, "dbg2       secondary_sameas_primary:  %d\n", data->secondary_sameas_primary);
		fprintf(stderr, "dbg2       secondary_nodatavalue:     %f\n", data->secondary_nodatavalue);
		fprintf(stderr, "dbg2       secondary_nxy:             %d\n", data->secondary_nxy);
		fprintf(stderr, "dbg2       secondary_n_columns:       %d\n", data->secondary_n_columns);
		fprintf(stderr, "dbg2       secondary_n_rows:          %d\n", data->secondary_n_rows);
		fprintf(stderr, "dbg2       secondary_xmin:            %f\n", data->secondary_xmin);
		fprintf(stderr, "dbg2       secondary_xmax:            %f\n", data->secondary_xmax);
		fprintf(stderr, "dbg2       secondary_ymin:            %f\n", data->secondary_ymin);
		fprintf(stderr, "dbg2       secondary_ymax:            %f\n", data->secondary_ymax);
		fprintf(stderr, "dbg2       secondary_dx:              %f\n", data->secondary_dx);
		fprintf(stderr, "dbg2       secondary_dy:              %f\n", data->secondary_dy);
		fprintf(stderr, "dbg2       secondary_data:            %p\n", data->secondary_data);

		/* site data */
		fprintf(stderr, "dbg2       site_view_mode:       %d\n", data->site_view_mode);
		fprintf(stderr, "dbg2       site_mode:            %d\n", shared.shareddata.site_mode);
		fprintf(stderr, "dbg2       nsite:                %d\n", shared.shareddata.nsite);
		fprintf(stderr, "dbg2       nsite_alloc:          %d\n", shared.shareddata.nsite_alloc);
		fprintf(stderr, "dbg2       site_selected:        %d\n", shared.shareddata.site_selected);
		for (int i = 0; i < shared.shareddata.nsite; i++) {
			fprintf(stderr, "dbg2       site %d xgrid:       %f\n", i, shared.shareddata.sites[i].point.xgrid[0]);
			fprintf(stderr, "dbg2       site %d ygrid:       %f\n", i, shared.shareddata.sites[i].point.ygrid[0]);
			fprintf(stderr, "dbg2       site %d xlon:        %f\n", i, shared.shareddata.sites[i].point.xlon);
			fprintf(stderr, "dbg2       site %d ylat:        %f\n", i, shared.shareddata.sites[i].point.ylat);
			fprintf(stderr, "dbg2       site %d zdata:       %f\n", i, shared.shareddata.sites[i].point.zdata);
			fprintf(stderr, "dbg2       site %d xdisplay:    %f\n", i, shared.shareddata.sites[i].point.xdisplay[0]);
			fprintf(stderr, "dbg2       site %d ydisplay:    %f\n", i, shared.shareddata.sites[i].point.ydisplay[0]);
			fprintf(stderr, "dbg2       site %d zdisplay:    %f\n", i, shared.shareddata.sites[i].point.zdisplay[0]);
			fprintf(stderr, "dbg2       site %d color:       %d\n", i, shared.shareddata.sites[i].color);
			fprintf(stderr, "dbg2       site %d size:        %d\n", i, shared.shareddata.sites[i].size);
			fprintf(stderr, "dbg2       site %d name:        %s\n", i, shared.shareddata.sites[i].name);
		}

		/* route data */
		fprintf(stderr, "dbg2       route_view_mode:      %d\n", data->route_view_mode);
		fprintf(stderr, "dbg2       route_mode:           %d\n", shared.shareddata.route_mode);
		fprintf(stderr, "dbg2       nroute:               %d\n", shared.shareddata.nroute);
		fprintf(stderr, "dbg2       nroute_alloc:         %d\n", shared.shareddata.nroute_alloc);
		fprintf(stderr, "dbg2       route_selected:       %d\n", shared.shareddata.route_selected);
		fprintf(stderr, "dbg2       route_point_selected: %d\n", shared.shareddata.route_point_selected);
		for (int i = 0; i < shared.shareddata.nroute; i++) {
			fprintf(stderr, "dbg2       route %d color:       %d\n", i, shared.shareddata.routes[i].color);
			fprintf(stderr, "dbg2       route %d size:        %d\n", i, shared.shareddata.routes[i].size);
			fprintf(stderr, "dbg2       route %d name:        %s\n", i, shared.shareddata.routes[i].name);
			int j = 0;
			for (; j < shared.shareddata.routes[i].npoints; j++) {
				fprintf(stderr, "dbg2       route %d %d xgrid:       %f\n", i, j, shared.shareddata.routes[i].points[j].xgrid[0]);
				fprintf(stderr, "dbg2       route %d %d ygrid:       %f\n", i, j, shared.shareddata.routes[i].points[j].ygrid[0]);
				fprintf(stderr, "dbg2       route %d %d xlon:        %f\n", i, j, shared.shareddata.routes[i].points[j].xlon);
				fprintf(stderr, "dbg2       route %d %d ylat:        %f\n", i, j, shared.shareddata.routes[i].points[j].ylat);
				fprintf(stderr, "dbg2       route %d %d zdata:       %f\n", i, j, shared.shareddata.routes[i].points[j].zdata);
				fprintf(stderr, "dbg2       route %d %d xdisplay:    %f\n", i, j,
				        shared.shareddata.routes[i].points[j].xdisplay[0]);
				fprintf(stderr, "dbg2       route %d %d ydisplay:    %f\n", i, j,
				        shared.shareddata.routes[i].points[j].ydisplay[0]);
				fprintf(stderr, "dbg2       route %d %d zdisplay:    %f\n", i, j,
				        shared.shareddata.routes[i].points[j].zdisplay[0]);
				fprintf(stderr, "dbg2       route %d %d distlateral: %f\n", i, j, shared.shareddata.routes[i].distlateral[j]);
				fprintf(stderr, "dbg2       route %d %d distlateral: %f\n", i, j, shared.shareddata.routes[i].disttopo[j]);
				fprintf(stderr, "dbg2       route %d %d distlateral: %f\n", i, j, shared.shareddata.routes[i].distlateral[j]);
				fprintf(stderr, "dbg2       route %d %d distlateral: %f\n", i, j, shared.shareddata.routes[i].disttopo[j]);
				fprintf(stderr, "dbg2       route %d %d distlateral: %f\n", i, j, shared.shareddata.routes[i].distlateral[j]);
			}
			fprintf(stderr, "dbg2       route %d %d distlateral: %f\n", i, j, shared.shareddata.routes[i].disttopo[j]);
		}

		/* nav data */
		fprintf(stderr, "dbg2       nav_view_mode:             %d\n", data->nav_view_mode);
		fprintf(stderr, "dbg2       navdrape_view_mode:        %d\n", data->navdrape_view_mode);
		fprintf(stderr, "dbg2       nav_mode:                  %d\n", shared.shareddata.nav_mode);
		fprintf(stderr, "dbg2       nnav:                      %d\n", shared.shareddata.nnav);
		fprintf(stderr, "dbg2       nnav_alloc:                %d\n", shared.shareddata.nnav_alloc);
		fprintf(stderr, "dbg2       nav_selected:              %p\n", shared.shareddata.nav_selected);
		fprintf(stderr, "dbg2       nav_point_selected:        %p\n", shared.shareddata.nav_point_selected);
		for (int i = 0; i < shared.shareddata.nnav; i++) {
			fprintf(stderr, "dbg2       nav %d color:         %d\n", i, shared.shareddata.navs[i].color);
			fprintf(stderr, "dbg2       nav %d size:          %d\n", i, shared.shareddata.navs[i].size);
			fprintf(stderr, "dbg2       nav %d name:          %s\n", i, shared.shareddata.navs[i].name);
			fprintf(stderr, "dbg2       nav %d swathbounds:   %d\n", i, shared.shareddata.navs[i].swathbounds);
			fprintf(stderr, "dbg2       nav %d shot:          %d\n", i, shared.shareddata.navs[i].shot);
			fprintf(stderr, "dbg2       nav %d cdp:           %d\n", i, shared.shareddata.navs[i].cdp);
			fprintf(stderr, "dbg2       nav %d npoints:       %d\n", i, shared.shareddata.navs[i].npoints);
			fprintf(stderr, "dbg2       nav %d npoints_alloc: %d\n", i, shared.shareddata.navs[i].npoints_alloc);
			fprintf(stderr, "dbg2       nav %d nselected:     %d\n", i, shared.shareddata.navs[i].nselected);
			for (int j = 0; j < shared.shareddata.navs[i].npoints; j++) {
				fprintf(stderr, "dbg2       nav %d %d xgrid:    %f\n", i, j, shared.shareddata.navs[i].navpts[j].point.xgrid[0]);
				fprintf(stderr, "dbg2       nav %d %d ygrid:    %f\n", i, j, shared.shareddata.navs[i].navpts[j].point.ygrid[0]);
				fprintf(stderr, "dbg2       nav %d %d xlon:     %f\n", i, j, shared.shareddata.navs[i].navpts[j].point.xlon);
				fprintf(stderr, "dbg2       nav %d %d ylat:     %f\n", i, j, shared.shareddata.navs[i].navpts[j].point.ylat);
				fprintf(stderr, "dbg2       nav %d %d zdata:    %f\n", i, j, shared.shareddata.navs[i].navpts[j].point.zdata);
				fprintf(stderr, "dbg2       nav %d %d xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.xdisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.ydisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.zdisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d port xgrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.xgrid[0]);
				fprintf(stderr, "dbg2       nav %d %d port ygrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.ygrid[0]);
				fprintf(stderr, "dbg2       nav %d %d port xlon:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.xlon);
				fprintf(stderr, "dbg2       nav %d %d port ylat:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.ylat);
				fprintf(stderr, "dbg2       nav %d %d port zdata:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.zdata);
				fprintf(stderr, "dbg2       nav %d %d port xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.xdisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d port ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.ydisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d port zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.zdisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d cntr xgrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.xgrid[0]);
				fprintf(stderr, "dbg2       nav %d %d cntr ygrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.ygrid[0]);
				fprintf(stderr, "dbg2       nav %d %d cntr xlon:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.xlon);
				fprintf(stderr, "dbg2       nav %d %d cntr ylat:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.ylat);
				fprintf(stderr, "dbg2       nav %d %d cntr zdata:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.zdata);
				fprintf(stderr, "dbg2       nav %d %d cntr xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.xdisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d cntr ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.ydisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d cntr zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.zdisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d stbd xgrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.xgrid[0]);
				fprintf(stderr, "dbg2       nav %d %d stbd ygrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.ygrid[0]);
				fprintf(stderr, "dbg2       nav %d %d stbd xlon:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.xlon);
				fprintf(stderr, "dbg2       nav %d %d stbd ylat:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.ylat);
				fprintf(stderr, "dbg2       nav %d %d stbd zdata:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.zdata);
				fprintf(stderr, "dbg2       nav %d %d stbd xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.xdisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d stbd ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.ydisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d stbd zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.zdisplay[0]);
			}
		}

		/* vector data */
		fprintf(stderr, "dbg2       vector_view_mode:          %d\n", data->vector_view_mode);
		fprintf(stderr, "dbg2       vector_mode:               %d\n", shared.shareddata.vector_mode);
		fprintf(stderr, "dbg2       nvector:                   %d\n", shared.shareddata.nvector);
		fprintf(stderr, "dbg2       nvector_alloc:             %d\n", shared.shareddata.nvector_alloc);
		fprintf(stderr, "dbg2       vector_selected:           %d\n", shared.shareddata.vector_selected);
		fprintf(stderr, "dbg2       vector_point_selected:     %d\n", shared.shareddata.vector_point_selected);
		for (int i = 0; i < shared.shareddata.nvector; i++) {
			fprintf(stderr, "dbg2       vector %d color:         %d\n", i, shared.shareddata.vectors[i].color);
			fprintf(stderr, "dbg2       vector %d size:          %d\n", i, shared.shareddata.vectors[i].size);
			fprintf(stderr, "dbg2       vector %d name:          %s\n", i, shared.shareddata.vectors[i].name);
			fprintf(stderr, "dbg2       vector %d format:        %d\n", i, shared.shareddata.vectors[i].format);
			fprintf(stderr, "dbg2       vector %d npoints:       %d\n", i, shared.shareddata.vectors[i].npoints);
			fprintf(stderr, "dbg2       vector %d npoints_alloc: %d\n", i, shared.shareddata.vectors[i].npoints_alloc);
			fprintf(stderr, "dbg2       vector %d nselected:     %d\n", i, shared.shareddata.vectors[i].nselected);
			for (int j = 0; j < shared.shareddata.navs[i].npoints; j++) {
				fprintf(stderr, "dbg2       vector %d %d xgrid:    %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.xgrid[0]);
				fprintf(stderr, "dbg2       vector %d %d ygrid:    %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.ygrid[0]);
				fprintf(stderr, "dbg2       vector %d %d xlon:     %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.xlon);
				fprintf(stderr, "dbg2       vector %d %d ylat:     %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.ylat);
				fprintf(stderr, "dbg2       vector %d %d zdata:    %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.zdata);
				fprintf(stderr, "dbg2       vector %d %d xdisplay: %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.xdisplay[0]);
				fprintf(stderr, "dbg2       vector %d %d ydisplay: %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.ydisplay[0]);
				fprintf(stderr, "dbg2       vector %d %d zdisplay: %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.zdisplay[0]);
			}
		}

		/* profile data */
		fprintf(stderr, "dbg2       profile_view_mode:         %d\n", data->profile_view_mode);
		fprintf(stderr, "dbg2       source:                    %d\n", data->profile.source);
		fprintf(stderr, "dbg2       source_name:               %s\n", data->profile.source_name);
		fprintf(stderr, "dbg2       length:                    %f\n", data->profile.length);
		fprintf(stderr, "dbg2       zmin:                      %f\n", data->profile.zmin);
		fprintf(stderr, "dbg2       zmax:                      %f\n", data->profile.zmax);
		fprintf(stderr, "dbg2       npoints:                   %d\n", data->profile.npoints);
		fprintf(stderr, "dbg2       npoints_alloc:             %d\n", data->profile.npoints_alloc);
		for (int i = 0; i < data->profile.npoints; i++) {
			fprintf(stderr, "dbg2       profile %d boundary: %d\n", i, data->profile.points[i].boundary);
			fprintf(stderr, "dbg2       profile %d xgrid:    %f\n", i, data->profile.points[i].xgrid);
			fprintf(stderr, "dbg2       profile %d ygrid:    %f\n", i, data->profile.points[i].ygrid);
			fprintf(stderr, "dbg2       profile %d xlon:     %f\n", i, data->profile.points[i].xlon);
			fprintf(stderr, "dbg2       profile %d ylat:     %f\n", i, data->profile.points[i].ylat);
			fprintf(stderr, "dbg2       profile %d zdata:    %f\n", i, data->profile.points[i].zdata);
			fprintf(stderr, "dbg2       profile %d distance: %f\n", i, data->profile.points[i].distance);
			fprintf(stderr, "dbg2       profile %d xdisplay: %f\n", i, data->profile.points[i].xdisplay);
			fprintf(stderr, "dbg2       profile %d ydisplay: %f\n", i, data->profile.points[i].ydisplay);
		}

		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_getsharedptr(int verbose, struct mbview_shareddata_struct **sharedhandle, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
	}

	/* get shared ptr */
	*sharedhandle = &(shared.shareddata);

	int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       sharedhandle:              %p\n", *sharedhandle);

		/* site data */
		fprintf(stderr, "dbg2       site_mode:            %d\n", shared.shareddata.site_mode);
		fprintf(stderr, "dbg2       nsite:                %d\n", shared.shareddata.nsite);
		fprintf(stderr, "dbg2       nsite_alloc:          %d\n", shared.shareddata.nsite_alloc);
		fprintf(stderr, "dbg2       site_selected:        %d\n", shared.shareddata.site_selected);
		for (int i = 0; i < shared.shareddata.nsite; i++) {
			fprintf(stderr, "dbg2       site %d xgrid:       %f\n", i, shared.shareddata.sites[i].point.xgrid[0]);
			fprintf(stderr, "dbg2       site %d ygrid:       %f\n", i, shared.shareddata.sites[i].point.ygrid[0]);
			fprintf(stderr, "dbg2       site %d xlon:        %f\n", i, shared.shareddata.sites[i].point.xlon);
			fprintf(stderr, "dbg2       site %d ylat:        %f\n", i, shared.shareddata.sites[i].point.ylat);
			fprintf(stderr, "dbg2       site %d zdata:       %f\n", i, shared.shareddata.sites[i].point.zdata);
			fprintf(stderr, "dbg2       site %d xdisplay:    %f\n", i, shared.shareddata.sites[i].point.xdisplay[0]);
			fprintf(stderr, "dbg2       site %d ydisplay:    %f\n", i, shared.shareddata.sites[i].point.ydisplay[0]);
			fprintf(stderr, "dbg2       site %d zdisplay:    %f\n", i, shared.shareddata.sites[i].point.zdisplay[0]);
			fprintf(stderr, "dbg2       site %d color:       %d\n", i, shared.shareddata.sites[i].color);
			fprintf(stderr, "dbg2       site %d size:        %d\n", i, shared.shareddata.sites[i].size);
			fprintf(stderr, "dbg2       site %d name:        %s\n", i, shared.shareddata.sites[i].name);
		}

		/* route data */
		fprintf(stderr, "dbg2       route_mode:           %d\n", shared.shareddata.route_mode);
		fprintf(stderr, "dbg2       nroute:               %d\n", shared.shareddata.nroute);
		fprintf(stderr, "dbg2       nroute_alloc:         %d\n", shared.shareddata.nroute_alloc);
		fprintf(stderr, "dbg2       route_selected:       %d\n", shared.shareddata.route_selected);
		fprintf(stderr, "dbg2       route_point_selected: %d\n", shared.shareddata.route_point_selected);
		for (int i = 0; i < shared.shareddata.nroute; i++) {
			fprintf(stderr, "dbg2       route %d color:       %d\n", i, shared.shareddata.routes[i].color);
			fprintf(stderr, "dbg2       route %d size:        %d\n", i, shared.shareddata.routes[i].size);
			fprintf(stderr, "dbg2       route %d name:        %s\n", i, shared.shareddata.routes[i].name);
			for (int j = 0; j < shared.shareddata.routes[i].npoints; j++) {
				fprintf(stderr, "dbg2       route %d %d xgrid:       %f\n", i, j, shared.shareddata.routes[i].points[j].xgrid[0]);
				fprintf(stderr, "dbg2       route %d %d ygrid:       %f\n", i, j, shared.shareddata.routes[i].points[j].ygrid[0]);
				fprintf(stderr, "dbg2       route %d %d xlon:        %f\n", i, j, shared.shareddata.routes[i].points[j].xlon);
				fprintf(stderr, "dbg2       route %d %d ylat:        %f\n", i, j, shared.shareddata.routes[i].points[j].ylat);
				fprintf(stderr, "dbg2       route %d %d zdata:       %f\n", i, j, shared.shareddata.routes[i].points[j].zdata);
				fprintf(stderr, "dbg2       route %d %d xdisplay:    %f\n", i, j,
				        shared.shareddata.routes[i].points[j].xdisplay[0]);
				fprintf(stderr, "dbg2       route %d %d ydisplay:    %f\n", i, j,
				        shared.shareddata.routes[i].points[j].ydisplay[0]);
				fprintf(stderr, "dbg2       route %d %d zdisplay:    %f\n", i, j,
				        shared.shareddata.routes[i].points[j].zdisplay[0]);
				fprintf(stderr, "dbg2       route %d %d distlateral: %f\n", i, j, shared.shareddata.routes[i].distlateral[j]);
				fprintf(stderr, "dbg2       route %d %d distlateral: %f\n", i, j, shared.shareddata.routes[i].disttopo[j]);
			}
		}

		/* nav data */
		fprintf(stderr, "dbg2       nav_mode:                  %d\n", shared.shareddata.nav_mode);
		fprintf(stderr, "dbg2       nnav:                      %d\n", shared.shareddata.nnav);
		fprintf(stderr, "dbg2       nnav_alloc:                %d\n", shared.shareddata.nnav_alloc);
		fprintf(stderr, "dbg2       nav_selected:              %p\n", shared.shareddata.nav_selected);
		fprintf(stderr, "dbg2       nav_point_selected:        %p\n", shared.shareddata.nav_point_selected);
		for (int i = 0; i < shared.shareddata.nnav; i++) {
			fprintf(stderr, "dbg2       nav %d color:         %d\n", i, shared.shareddata.navs[i].color);
			fprintf(stderr, "dbg2       nav %d size:          %d\n", i, shared.shareddata.navs[i].size);
			fprintf(stderr, "dbg2       nav %d name:          %s\n", i, shared.shareddata.navs[i].name);
			fprintf(stderr, "dbg2       nav %d swathbounds:   %d\n", i, shared.shareddata.navs[i].swathbounds);
			fprintf(stderr, "dbg2       nav %d shot:          %d\n", i, shared.shareddata.navs[i].shot);
			fprintf(stderr, "dbg2       nav %d cdp:           %d\n", i, shared.shareddata.navs[i].cdp);
			fprintf(stderr, "dbg2       nav %d npoints:       %d\n", i, shared.shareddata.navs[i].npoints);
			fprintf(stderr, "dbg2       nav %d npoints_alloc: %d\n", i, shared.shareddata.navs[i].npoints_alloc);
			fprintf(stderr, "dbg2       nav %d nselected:     %d\n", i, shared.shareddata.navs[i].nselected);
			for (int j = 0; j < shared.shareddata.navs[i].npoints; j++) {
				fprintf(stderr, "dbg2       nav %d %d draped:        %d\n", i, j, shared.shareddata.navs[i].navpts[j].draped);
				fprintf(stderr, "dbg2       nav %d %d selected:      %d\n", i, j, shared.shareddata.navs[i].navpts[j].selected);
				fprintf(stderr, "dbg2       nav %d %d time_d:        %f\n", i, j, shared.shareddata.navs[i].navpts[j].time_d);
				fprintf(stderr, "dbg2       nav %d %d heading:       %f\n", i, j, shared.shareddata.navs[i].navpts[j].heading);
				fprintf(stderr, "dbg2       nav %d %d speed:         %f\n", i, j, shared.shareddata.navs[i].navpts[j].speed);
				fprintf(stderr, "dbg2       nav %d %d xgrid:         %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.xgrid[0]);
				fprintf(stderr, "dbg2       nav %d %d ygrid:         %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.ygrid[0]);
				fprintf(stderr, "dbg2       nav %d %d xlon:          %f\n", i, j, shared.shareddata.navs[i].navpts[j].point.xlon);
				fprintf(stderr, "dbg2       nav %d %d ylat:          %f\n", i, j, shared.shareddata.navs[i].navpts[j].point.ylat);
				fprintf(stderr, "dbg2       nav %d %d zdata:         %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.zdata);
				fprintf(stderr, "dbg2       nav %d %d xdisplay:      %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.xdisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d ydisplay:      %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.ydisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d zdisplay:      %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.zdisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d port xgrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.xgrid[0]);
				fprintf(stderr, "dbg2       nav %d %d port ygrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.ygrid[0]);
				fprintf(stderr, "dbg2       nav %d %d port xlon:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.xlon);
				fprintf(stderr, "dbg2       nav %d %d port ylat:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.ylat);
				fprintf(stderr, "dbg2       nav %d %d port zdata:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.zdata);
				fprintf(stderr, "dbg2       nav %d %d port xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.xdisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d port ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.ydisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d port zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.zdisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d cntr xgrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.xgrid[0]);
				fprintf(stderr, "dbg2       nav %d %d cntr ygrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.ygrid[0]);
				fprintf(stderr, "dbg2       nav %d %d cntr xlon:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.xlon);
				fprintf(stderr, "dbg2       nav %d %d cntr ylat:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.ylat);
				fprintf(stderr, "dbg2       nav %d %d cntr zdata:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.zdata);
				fprintf(stderr, "dbg2       nav %d %d cntr xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.xdisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d cntr ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.ydisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d cntr zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.zdisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d stbd xgrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.xgrid[0]);
				fprintf(stderr, "dbg2       nav %d %d stbd ygrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.ygrid[0]);
				fprintf(stderr, "dbg2       nav %d %d stbd xlon:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.xlon);
				fprintf(stderr, "dbg2       nav %d %d stbd ylat:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.ylat);
				fprintf(stderr, "dbg2       nav %d %d stbd zdata:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.zdata);
				fprintf(stderr, "dbg2       nav %d %d stbd xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.xdisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d stbd ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.ydisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d stbd zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.zdisplay[0]);
			}
		}

		/* vector data */
		fprintf(stderr, "dbg2       vector_mode:               %d\n", shared.shareddata.vector_mode);
		fprintf(stderr, "dbg2       nvector:                   %d\n", shared.shareddata.nvector);
		fprintf(stderr, "dbg2       nvector_alloc:             %d\n", shared.shareddata.nvector_alloc);
		fprintf(stderr, "dbg2       vector_selected:           %d\n", shared.shareddata.vector_selected);
		fprintf(stderr, "dbg2       vector_point_selected:     %d\n", shared.shareddata.vector_point_selected);
		for (int i = 0; i < shared.shareddata.nvector; i++) {
			fprintf(stderr, "dbg2       vector %d color:         %d\n", i, shared.shareddata.vectors[i].color);
			fprintf(stderr, "dbg2       vector %d size:          %d\n", i, shared.shareddata.vectors[i].size);
			fprintf(stderr, "dbg2       vector %d name:          %s\n", i, shared.shareddata.vectors[i].name);
			fprintf(stderr, "dbg2       vector %d format:        %d\n", i, shared.shareddata.vectors[i].format);
			fprintf(stderr, "dbg2       vector %d npoints:       %d\n", i, shared.shareddata.vectors[i].npoints);
			fprintf(stderr, "dbg2       vector %d npoints_alloc: %d\n", i, shared.shareddata.vectors[i].npoints_alloc);
			fprintf(stderr, "dbg2       vector %d nselected:     %d\n", i, shared.shareddata.vectors[i].nselected);
			for (int j = 0; j < shared.shareddata.navs[i].npoints; j++) {
				fprintf(stderr, "dbg2       vector %d %d xgrid:    %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.xgrid[0]);
				fprintf(stderr, "dbg2       vector %d %d ygrid:    %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.ygrid[0]);
				fprintf(stderr, "dbg2       vector %d %d xlon:     %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.xlon);
				fprintf(stderr, "dbg2       vector %d %d ylat:     %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.ylat);
				fprintf(stderr, "dbg2       vector %d %d zdata:    %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.zdata);
				fprintf(stderr, "dbg2       vector %d %d xdisplay: %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.xdisplay[0]);
				fprintf(stderr, "dbg2       vector %d %d ydisplay: %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.ydisplay[0]);
				fprintf(stderr, "dbg2       vector %d %d zdisplay: %f\n", i, j,
				        shared.shareddata.vectors[i].vectorpts[j].point.zdisplay[0]);
			}
		}

		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_setwindowparms(int verbose, size_t instance, int (*mbview_dismiss_notify)(size_t), char *title, int xo, int yo,
                          int width, int height, int lorez_dimension, int hirez_dimension, int lorez_navdecimate,
                          int hirez_navdecimate, int *error)
{
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       mbview_dismiss_notify:     %p\n", mbview_dismiss_notify);
		fprintf(stderr, "dbg2       title:                     %s\n", title);
		fprintf(stderr, "dbg2       xo:                        %d\n", xo);
		fprintf(stderr, "dbg2       yo:                        %d\n", yo);
		fprintf(stderr, "dbg2       width:                     %d\n", width);
		fprintf(stderr, "dbg2       height:                    %d\n", height);
		fprintf(stderr, "dbg2       lorez_dimension:           %d\n", lorez_dimension);
		fprintf(stderr, "dbg2       hirez_dimension:           %d\n", hirez_dimension);
		fprintf(stderr, "dbg2       lorez_navdecimate:         %d\n", lorez_navdecimate);
		fprintf(stderr, "dbg2       hirez_navdecimate:         %d\n", hirez_navdecimate);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* set values */
	data->mbview_dismiss_notify = mbview_dismiss_notify;
	strcpy(data->title, title);
	data->xo = xo;
	data->yo = yo;
	data->width = width;
	data->height = height;
	data->lorez_dimension = lorez_dimension;
	data->hirez_dimension = hirez_dimension;
	data->lorez_navdecimate = lorez_navdecimate;
	data->hirez_navdecimate = hirez_navdecimate;

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
int mbview_setviewcontrols(int verbose, size_t instance, int display_mode, int mouse_mode, int grid_mode, int primary_histogram,
                           int primaryslope_histogram, int secondary_histogram, int primary_shade_mode, int slope_shade_mode,
                           int secondary_shade_mode, int grid_contour_mode, int site_view_mode, int route_view_mode,
                           int nav_view_mode, int navdrape_view_mode, int vector_view_mode, double exageration,
                           double modelelevation3d, double modelazimuth3d, double viewelevation3d, double viewazimuth3d,
                           double illuminate_magnitude, double illuminate_elevation, double illuminate_azimuth,
                           double slope_magnitude, double overlay_shade_magnitude, double overlay_shade_center,
                           int overlay_shade_mode, double contour_interval, int display_projection_mode,
                           char *display_projection_id, int *error)
{
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       display_mode:              %d\n", display_mode);
		fprintf(stderr, "dbg2       mouse_mode:                %d\n", mouse_mode);
		fprintf(stderr, "dbg2       grid_mode:                 %d\n", grid_mode);
		fprintf(stderr, "dbg2       primary_histogram:         %d\n", primary_histogram);
		fprintf(stderr, "dbg2       primaryslope_histogram:    %d\n", primaryslope_histogram);
		fprintf(stderr, "dbg2       secondary_histogram:       %d\n", secondary_histogram);
		fprintf(stderr, "dbg2       primary_shade_mode:        %d\n", primary_shade_mode);
		fprintf(stderr, "dbg2       slope_shade_mode:          %d\n", slope_shade_mode);
		fprintf(stderr, "dbg2       secondary_shade_mode:      %d\n", secondary_shade_mode);
		fprintf(stderr, "dbg2       grid_contour_mode:         %d\n", grid_contour_mode);
		fprintf(stderr, "dbg2       site_view_mode:            %d\n", site_view_mode);
		fprintf(stderr, "dbg2       route_view_mode:           %d\n", route_view_mode);
		fprintf(stderr, "dbg2       nav_view_mode:             %d\n", nav_view_mode);
		fprintf(stderr, "dbg2       navdrape_view_mode:        %d\n", navdrape_view_mode);
		fprintf(stderr, "dbg2       vector_view_mode:          %d\n", vector_view_mode);
		fprintf(stderr, "dbg2       exageration:               %f\n", exageration);
		fprintf(stderr, "dbg2       modelelevation3d:          %f\n", modelelevation3d);
		fprintf(stderr, "dbg2       modelazimuth3d:            %f\n", modelazimuth3d);
		fprintf(stderr, "dbg2       viewelevation3d:           %f\n", viewelevation3d);
		fprintf(stderr, "dbg2       viewazimuth3d:             %f\n", viewazimuth3d);
		fprintf(stderr, "dbg2       illuminate_magnitude:      %f\n", illuminate_magnitude);
		fprintf(stderr, "dbg2       illuminate_elevation:      %f\n", illuminate_elevation);
		fprintf(stderr, "dbg2       illuminate_azimuth:        %f\n", illuminate_azimuth);
		fprintf(stderr, "dbg2       slope_magnitude:           %f\n", slope_magnitude);
		fprintf(stderr, "dbg2       overlay_shade_magnitude:   %f\n", overlay_shade_magnitude);
		fprintf(stderr, "dbg2       overlay_shade_center:      %f\n", overlay_shade_center);
		fprintf(stderr, "dbg2       overlay_shade_mode:        %d\n", overlay_shade_mode);
		fprintf(stderr, "dbg2       contour_interval:          %f\n", slope_magnitude);
		fprintf(stderr, "dbg2       display_projection_mode:   %d\n", display_projection_mode);
		fprintf(stderr, "dbg2       display_projection_id:     %s\n", display_projection_id);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* set values */
	data->display_mode = display_mode;
	data->mouse_mode = mouse_mode;
	data->grid_mode = grid_mode;
	data->primary_histogram = primary_histogram;
	data->primaryslope_histogram = primaryslope_histogram;
	data->secondary_histogram = secondary_histogram;
	data->primary_shade_mode = primary_shade_mode;
	data->slope_shade_mode = slope_shade_mode;
	data->secondary_shade_mode = secondary_shade_mode;
	data->grid_contour_mode = grid_contour_mode;
	data->site_view_mode = site_view_mode;
	data->route_view_mode = route_view_mode;
	data->nav_view_mode = nav_view_mode;
	data->navdrape_view_mode = navdrape_view_mode;
	data->vector_view_mode = vector_view_mode;
	data->exageration = exageration;
	data->modelelevation3d = modelelevation3d;
	data->modelazimuth3d = modelazimuth3d;
	data->viewelevation3d = viewelevation3d;
	data->viewazimuth3d = viewazimuth3d;
	data->illuminate_magnitude = illuminate_magnitude;
	data->illuminate_elevation = illuminate_elevation;
	data->illuminate_azimuth = illuminate_azimuth;
	data->slope_magnitude = slope_magnitude;
	data->overlay_shade_magnitude = overlay_shade_magnitude;
	data->overlay_shade_center = overlay_shade_center;
	data->overlay_shade_mode = overlay_shade_mode;
	data->contour_interval = contour_interval;
	data->display_projection_mode = display_projection_mode;
	strcpy(data->display_projection_id, display_projection_id);

	/* set widgets */
	if (data->active)
		mbview_set_widgets(verbose, instance, error);

	/* set widget sensitivity */
	if (data->active)
		mbview_update_sensitivity(verbose, instance, error);

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
int mbview_open(int verbose, size_t instance, int *error) {
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       view:                      %p\n", view);
		fprintf(stderr, "dbg2       data:                      %p\n", data);

		/* widget controls */
		fprintf(stderr, "dbg2       title:                     %s\n", data->title);
		fprintf(stderr, "dbg2       xo:                        %d\n", data->xo);
		fprintf(stderr, "dbg2       yo:                        %d\n", data->yo);
		fprintf(stderr, "dbg2       width:                     %d\n", data->width);
		fprintf(stderr, "dbg2       height:                    %d\n", data->height);
		fprintf(stderr, "dbg2       lorez_dimension:           %d\n", data->lorez_dimension);
		fprintf(stderr, "dbg2       hirez_dimension:           %d\n", data->hirez_dimension);
		fprintf(stderr, "dbg2       lorez_navdecimate:         %d\n", data->lorez_navdecimate);
		fprintf(stderr, "dbg2       hirez_navdecimate:         %d\n", data->hirez_navdecimate);

		/* mode controls */
		fprintf(stderr, "dbg2       display_mode:              %d\n", data->display_mode);
		fprintf(stderr, "dbg2       mouse_mode:                %d\n", data->mouse_mode);
		fprintf(stderr, "dbg2       grid_mode:                 %d\n", data->grid_mode);
		fprintf(stderr, "dbg2       grid_contour_mode:         %d\n", data->grid_contour_mode);

		/* colortable controls */
		fprintf(stderr, "dbg2       primary_colortable:        %d\n", data->primary_colortable);
		fprintf(stderr, "dbg2       primary_colortable_mode:   %d\n", data->primary_colortable_mode);
		fprintf(stderr, "dbg2       primary_colortable_min:    %f\n", data->primary_colortable_min);
		fprintf(stderr, "dbg2       primary_colortable_max:    %f\n", data->primary_colortable_max);
		fprintf(stderr, "dbg2       slope_colortable:          %d\n", data->slope_colortable);
		fprintf(stderr, "dbg2       slope_colortable_mode:     %d\n", data->slope_colortable_mode);
		fprintf(stderr, "dbg2       slope_colortable_min:      %f\n", data->slope_colortable_min);
		fprintf(stderr, "dbg2       slope_colortable_max:      %f\n", data->slope_colortable_max);
		fprintf(stderr, "dbg2       slope_shade_mode:          %d\n", data->slope_shade_mode);
		fprintf(stderr, "dbg2       secondary_colortable:      %d\n", data->secondary_colortable);
		fprintf(stderr, "dbg2       secondary_colortable_mode: %d\n", data->secondary_colortable_mode);
		fprintf(stderr, "dbg2       secondary_colortable_min:  %f\n", data->secondary_colortable_min);
		fprintf(stderr, "dbg2       secondary_colortable_max:  %f\n", data->secondary_colortable_max);
		fprintf(stderr, "dbg2       secondary_shade_mode:      %d\n", data->secondary_shade_mode);

		/* view controls */
		fprintf(stderr, "dbg2       exageration:               %f\n", data->exageration);
		fprintf(stderr, "dbg2       modelelevation3d:          %f\n", data->modelelevation3d);
		fprintf(stderr, "dbg2       modelazimuth3d:            %f\n", data->modelazimuth3d);
		fprintf(stderr, "dbg2       viewelevation3d:           %f\n", data->viewelevation3d);
		fprintf(stderr, "dbg2       viewazimuth3d:             %f\n", data->viewazimuth3d);

		/* shading controls */
		fprintf(stderr, "dbg2       illuminate_magnitude:      %f\n", data->illuminate_magnitude);
		fprintf(stderr, "dbg2       illuminate_elevation:      %f\n", data->illuminate_elevation);
		fprintf(stderr, "dbg2       illuminate_azimuth:        %f\n", data->illuminate_azimuth);
		fprintf(stderr, "dbg2       slope_magnitude:           %f\n", data->slope_magnitude);

		/* contour controls */
		fprintf(stderr, "dbg2       contour_interval:           %f\n", data->slope_magnitude);

		/* profile controls */
		fprintf(stderr, "dbg2       profile_exageration:        %f\n", data->profile_exageration);
		fprintf(stderr, "dbg2       profile_widthfactor:        %d\n", data->profile_widthfactor);
		fprintf(stderr, "dbg2       profile_slopethreshold:     %f\n", data->profile_slopethreshold);

		/* projection controls */
		fprintf(stderr, "dbg2       primary_grid_projection_mode:   %d\n", data->primary_grid_projection_mode);
		fprintf(stderr, "dbg2       primary_grid_projection_id:     %s\n", data->primary_grid_projection_id);
		fprintf(stderr, "dbg2       secondary_grid_projection_mode: %d\n", data->secondary_grid_projection_mode);
		fprintf(stderr, "dbg2       secondary_grid_projection_id:   %s\n", data->secondary_grid_projection_id);
		fprintf(stderr, "dbg2       display_projection_mode:        %d\n", data->display_projection_mode);
		fprintf(stderr, "dbg2       display_projection_id:          %s\n", data->display_projection_id);

		/* primary grid data */
		fprintf(stderr, "dbg2       primary_nodatavalue:       %f\n", data->primary_nodatavalue);
		fprintf(stderr, "dbg2       primary_nxy:               %d\n", data->primary_nxy);
		fprintf(stderr, "dbg2       primary_n_columns:         %d\n", data->primary_n_columns);
		fprintf(stderr, "dbg2       primary_n_rows:            %d\n", data->primary_n_rows);
		fprintf(stderr, "dbg2       primary_min:               %f\n", data->primary_min);
		fprintf(stderr, "dbg2       primary_max:               %f\n", data->primary_max);
		fprintf(stderr, "dbg2       primary_xmin:              %f\n", data->primary_xmin);
		fprintf(stderr, "dbg2       primary_xmax:              %f\n", data->primary_xmax);
		fprintf(stderr, "dbg2       primary_ymin:              %f\n", data->primary_ymin);
		fprintf(stderr, "dbg2       primary_ymax:              %f\n", data->primary_ymax);
		fprintf(stderr, "dbg2       primary_dx:                %f\n", data->primary_dx);
		fprintf(stderr, "dbg2       primary_dy:                %f\n", data->primary_dy);
		fprintf(stderr, "dbg2       primary_data:              %p\n", data->primary_data);
		fprintf(stderr, "dbg2       primary_x:                 %p\n", data->primary_x);
		fprintf(stderr, "dbg2       primary_y:                 %p\n", data->primary_y);
		fprintf(stderr, "dbg2       primary_z:                 %p\n", data->primary_z);
		fprintf(stderr, "dbg2       primary_dxdz:              %p\n", data->primary_dzdx);
		fprintf(stderr, "dbg2       primary_dydz:              %p\n", data->primary_dzdy);
		fprintf(stderr, "dbg2       primary_r:                 %p\n", data->primary_r);
		fprintf(stderr, "dbg2       primary_g:                 %p\n", data->primary_g);
		fprintf(stderr, "dbg2       primary_b:                 %p\n", data->primary_b);
		fprintf(stderr, "dbg2       primary_stat_color:        %p\n", data->primary_stat_color);
		fprintf(stderr, "dbg2       primary_stat_z:            %p\n", data->primary_stat_z);

		/* secondary grid data */
		fprintf(stderr, "dbg2       secondary_sameas_primary:  %d\n", data->secondary_sameas_primary);
		fprintf(stderr, "dbg2       secondary_nodatavalue:     %f\n", data->secondary_nodatavalue);
		fprintf(stderr, "dbg2       secondary_nxy:             %d\n", data->secondary_nxy);
		fprintf(stderr, "dbg2       secondary_n_columns:       %d\n", data->secondary_n_columns);
		fprintf(stderr, "dbg2       secondary_n_rows:          %d\n", data->secondary_n_rows);
		fprintf(stderr, "dbg2       secondary_xmin:            %f\n", data->secondary_xmin);
		fprintf(stderr, "dbg2       secondary_xmax:            %f\n", data->secondary_xmax);
		fprintf(stderr, "dbg2       secondary_ymin:            %f\n", data->secondary_ymin);
		fprintf(stderr, "dbg2       secondary_ymax:            %f\n", data->secondary_ymax);
		fprintf(stderr, "dbg2       secondary_dx:              %f\n", data->secondary_dx);
		fprintf(stderr, "dbg2       secondary_dy:              %f\n", data->secondary_dy);
		fprintf(stderr, "dbg2       secondary_data:            %p\n", data->secondary_data);

		/* site data */
		fprintf(stderr, "dbg2       site_view_mode:       %d\n", data->site_view_mode);
		fprintf(stderr, "dbg2       site_mode:            %d\n", shared.shareddata.site_mode);
		fprintf(stderr, "dbg2       nsite:                %d\n", shared.shareddata.nsite);
		fprintf(stderr, "dbg2       nsite_alloc:          %d\n", shared.shareddata.nsite_alloc);
		fprintf(stderr, "dbg2       site_selected:        %d\n", shared.shareddata.site_selected);
		for (int i = 0; i < shared.shareddata.nsite; i++) {
			fprintf(stderr, "dbg2       site %d xgrid:       %f\n", i, shared.shareddata.sites[i].point.xgrid[0]);
			fprintf(stderr, "dbg2       site %d ygrid:       %f\n", i, shared.shareddata.sites[i].point.ygrid[0]);
			fprintf(stderr, "dbg2       site %d xlon:        %f\n", i, shared.shareddata.sites[i].point.xlon);
			fprintf(stderr, "dbg2       site %d ylat:        %f\n", i, shared.shareddata.sites[i].point.ylat);
			fprintf(stderr, "dbg2       site %d zdata:       %f\n", i, shared.shareddata.sites[i].point.zdata);
			fprintf(stderr, "dbg2       site %d xdisplay:    %f\n", i, shared.shareddata.sites[i].point.xdisplay[0]);
			fprintf(stderr, "dbg2       site %d ydisplay:    %f\n", i, shared.shareddata.sites[i].point.ydisplay[0]);
			fprintf(stderr, "dbg2       site %d zdisplay:    %f\n", i, shared.shareddata.sites[i].point.zdisplay[0]);
			fprintf(stderr, "dbg2       site %d color:       %d\n", i, shared.shareddata.sites[i].color);
			fprintf(stderr, "dbg2       site %d size:        %d\n", i, shared.shareddata.sites[i].size);
			fprintf(stderr, "dbg2       site %d name:        %s\n", i, shared.shareddata.sites[i].name);
		}

		/* route data */
		fprintf(stderr, "dbg2       route_view_mode:      %d\n", data->route_view_mode);
		fprintf(stderr, "dbg2       route_mode:           %d\n", shared.shareddata.route_mode);
		fprintf(stderr, "dbg2       nroute:               %d\n", shared.shareddata.nroute);
		fprintf(stderr, "dbg2       nroute_alloc:         %d\n", shared.shareddata.nroute_alloc);
		fprintf(stderr, "dbg2       route_selected:       %d\n", shared.shareddata.route_selected);
		fprintf(stderr, "dbg2       route_point_selected: %d\n", shared.shareddata.route_point_selected);
		for (int i = 0; i < shared.shareddata.nroute; i++) {
			fprintf(stderr, "dbg2       route %d color:       %d\n", i, shared.shareddata.routes[i].color);
			fprintf(stderr, "dbg2       route %d size:        %d\n", i, shared.shareddata.routes[i].size);
			fprintf(stderr, "dbg2       route %d name:        %s\n", i, shared.shareddata.routes[i].name);
			fprintf(stderr, "dbg2       route %d npoints:     %d\n", i, shared.shareddata.routes[i].npoints);
			fprintf(stderr, "dbg2       route %d npoints_alloc: %d\n", i, shared.shareddata.routes[i].npoints_alloc);
			for (int j = 0; j < shared.shareddata.routes[i].npoints; j++) {
				fprintf(stderr, "dbg2       route %d %d xgrid:       %f\n", i, j, shared.shareddata.routes[i].points[j].xgrid[0]);
				fprintf(stderr, "dbg2       route %d %d ygrid:       %f\n", i, j, shared.shareddata.routes[i].points[j].ygrid[0]);
				fprintf(stderr, "dbg2       route %d %d xlon:        %f\n", i, j, shared.shareddata.routes[i].points[j].xlon);
				fprintf(stderr, "dbg2       route %d %d ylat:        %f\n", i, j, shared.shareddata.routes[i].points[j].ylat);
				fprintf(stderr, "dbg2       route %d %d zdata:       %f\n", i, j, shared.shareddata.routes[i].points[j].zdata);
				fprintf(stderr, "dbg2       route %d %d xdisplay:    %f\n", i, j,
				        shared.shareddata.routes[i].points[j].xdisplay[0]);
				fprintf(stderr, "dbg2       route %d %d ydisplay:    %f\n", i, j,
				        shared.shareddata.routes[i].points[j].ydisplay[0]);
				fprintf(stderr, "dbg2       route %d %d zdisplay:    %f\n", i, j,
				        shared.shareddata.routes[i].points[j].zdisplay[0]);
				fprintf(stderr, "dbg2       route %d %d distlateral: %f\n", i, j, shared.shareddata.routes[i].distlateral[j]);
				fprintf(stderr, "dbg2       route %d %d distlateral: %f\n", i, j, shared.shareddata.routes[i].disttopo[j]);
			}
		}

		/* nav data */
		fprintf(stderr, "dbg2       nav_view_mode:         %d\n", data->nav_view_mode);
		fprintf(stderr, "dbg2       navdrape_view_mode:    %d\n", data->navdrape_view_mode);
		fprintf(stderr, "dbg2       vector_view_mode:      %d\n", data->vector_view_mode);
		fprintf(stderr, "dbg2       nav_mode:              %d\n", shared.shareddata.nav_mode);
		fprintf(stderr, "dbg2       nnav:                  %d\n", shared.shareddata.nnav);
		fprintf(stderr, "dbg2       nnav_alloc:            %d\n", shared.shareddata.nnav_alloc);
		fprintf(stderr, "dbg2       nav_selected:          %p\n", shared.shareddata.nav_selected);
		fprintf(stderr, "dbg2       nav_point_selected:    %p\n", shared.shareddata.nav_point_selected);
		for (int i = 0; i < shared.shareddata.nnav; i++) {
			fprintf(stderr, "dbg2       nav %d color:         %d\n", i, shared.shareddata.navs[i].color);
			fprintf(stderr, "dbg2       nav %d size:          %d\n", i, shared.shareddata.navs[i].size);
			fprintf(stderr, "dbg2       nav %d name:          %s\n", i, shared.shareddata.navs[i].name);
			fprintf(stderr, "dbg2       nav %d swathbounds:   %d\n", i, shared.shareddata.navs[i].swathbounds);
			fprintf(stderr, "dbg2       nav %d shot:          %d\n", i, shared.shareddata.navs[i].shot);
			fprintf(stderr, "dbg2       nav %d cdp:           %d\n", i, shared.shareddata.navs[i].cdp);
			fprintf(stderr, "dbg2       nav %d npoints:       %d\n", i, shared.shareddata.navs[i].npoints);
			fprintf(stderr, "dbg2       nav %d npoints_alloc: %d\n", i, shared.shareddata.navs[i].npoints_alloc);
			fprintf(stderr, "dbg2       nav %d nselected:     %d\n", i, shared.shareddata.navs[i].nselected);
			for (int j = 0; j < shared.shareddata.navs[i].npoints; j++) {
				fprintf(stderr, "dbg2       nav %d %d draped:   %d\n", i, j, shared.shareddata.navs[i].navpts[j].draped);
				fprintf(stderr, "dbg2       nav %d %d selected: %d\n", i, j, shared.shareddata.navs[i].navpts[j].selected);
				fprintf(stderr, "dbg2       nav %d %d time_d:   %f\n", i, j, shared.shareddata.navs[i].navpts[j].time_d);
				fprintf(stderr, "dbg2       nav %d %d heading:  %f\n", i, j, shared.shareddata.navs[i].navpts[j].heading);
				fprintf(stderr, "dbg2       nav %d %d speed:    %f\n", i, j, shared.shareddata.navs[i].navpts[j].speed);
				fprintf(stderr, "dbg2       nav %d %d shot:     %d\n", i, j, shared.shareddata.navs[i].navpts[j].shot);
				fprintf(stderr, "dbg2       nav %d %d cdp:      %d\n", i, j, shared.shareddata.navs[i].navpts[j].cdp);
				fprintf(stderr, "dbg2       nav %d %d xgrid:    %f\n", i, j, shared.shareddata.navs[i].navpts[j].point.xgrid[0]);
				fprintf(stderr, "dbg2       nav %d %d ygrid:    %f\n", i, j, shared.shareddata.navs[i].navpts[j].point.ygrid[0]);
				fprintf(stderr, "dbg2       nav %d %d xlon:     %f\n", i, j, shared.shareddata.navs[i].navpts[j].point.xlon);
				fprintf(stderr, "dbg2       nav %d %d ylat:     %f\n", i, j, shared.shareddata.navs[i].navpts[j].point.ylat);
				fprintf(stderr, "dbg2       nav %d %d zdata:    %f\n", i, j, shared.shareddata.navs[i].navpts[j].point.zdata);
				fprintf(stderr, "dbg2       nav %d %d xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.xdisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.ydisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].point.zdisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d port xgrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.xgrid[0]);
				fprintf(stderr, "dbg2       nav %d %d port ygrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.ygrid[0]);
				fprintf(stderr, "dbg2       nav %d %d port xlon:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.xlon);
				fprintf(stderr, "dbg2       nav %d %d port ylat:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.ylat);
				fprintf(stderr, "dbg2       nav %d %d port zdata:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.zdata);
				fprintf(stderr, "dbg2       nav %d %d port xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.xdisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d port ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.ydisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d port zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointport.zdisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d cntr xgrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.xgrid[0]);
				fprintf(stderr, "dbg2       nav %d %d cntr ygrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.ygrid[0]);
				fprintf(stderr, "dbg2       nav %d %d cntr xlon:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.xlon);
				fprintf(stderr, "dbg2       nav %d %d cntr ylat:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.ylat);
				fprintf(stderr, "dbg2       nav %d %d cntr zdata:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.zdata);
				fprintf(stderr, "dbg2       nav %d %d cntr xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.xdisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d cntr ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.ydisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d cntr zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointcntr.zdisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d stbd xgrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.xgrid[0]);
				fprintf(stderr, "dbg2       nav %d %d stbd ygrid:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.ygrid[0]);
				fprintf(stderr, "dbg2       nav %d %d stbd xlon:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.xlon);
				fprintf(stderr, "dbg2       nav %d %d stbd ylat:     %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.ylat);
				fprintf(stderr, "dbg2       nav %d %d stbd zdata:    %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.zdata);
				fprintf(stderr, "dbg2       nav %d %d stbd xdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.xdisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d stbd ydisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.ydisplay[0]);
				fprintf(stderr, "dbg2       nav %d %d stbd zdisplay: %f\n", i, j,
				        shared.shareddata.navs[i].navpts[j].pointstbd.zdisplay[0]);
			}
		}

		/* profile data */
		fprintf(stderr, "dbg2       profile_view_mode:         %d\n", data->profile_view_mode);
		fprintf(stderr, "dbg2       source:                    %d\n", data->profile.source);
		fprintf(stderr, "dbg2       source_name:               %s\n", data->profile.source_name);
		fprintf(stderr, "dbg2       length:                    %f\n", data->profile.length);
		fprintf(stderr, "dbg2       zmin:                      %f\n", data->profile.zmin);
		fprintf(stderr, "dbg2       zmax:                      %f\n", data->profile.zmax);
		fprintf(stderr, "dbg2       npoints:                   %d\n", data->profile.npoints);
		fprintf(stderr, "dbg2       npoints_alloc:             %d\n", data->profile.npoints_alloc);
		for (int i = 0; i < data->profile.npoints; i++) {
			fprintf(stderr, "dbg2       profile %d boundary: %d\n", i, data->profile.points[i].boundary);
			fprintf(stderr, "dbg2       profile %d xgrid:    %f\n", i, data->profile.points[i].xgrid);
			fprintf(stderr, "dbg2       profile %d ygrid:    %f\n", i, data->profile.points[i].ygrid);
			fprintf(stderr, "dbg2       profile %d xlon:     %f\n", i, data->profile.points[i].xlon);
			fprintf(stderr, "dbg2       profile %d ylat:     %f\n", i, data->profile.points[i].ylat);
			fprintf(stderr, "dbg2       profile %d zdata:    %f\n", i, data->profile.points[i].zdata);
			fprintf(stderr, "dbg2       profile %d distance: %f\n", i, data->profile.points[i].distance);
			fprintf(stderr, "dbg2       profile %d xdisplay: %f\n", i, data->profile.points[i].xdisplay);
			fprintf(stderr, "dbg2       profile %d ydisplay: %f\n", i, data->profile.points[i].ydisplay);
		}
	}

	/* set active */
	data->active = true;

	/* if not yet created then create the MB3DView class in
	    a topLevelShell as a child of Widget parent */
	if (view->init != MBV_WINDOW_VISIBLE) {
		Arg args[256];
    Cardinal ac = 0;
		XtSetArg(args[ac], XmNtitle, data->title);
		ac++;
		XtSetArg(args[ac], XmNwidth, data->width + LEFT_WIDTH);
		ac++;
		XtSetArg(args[ac], XmNheight, data->height + LEFT_HEIGHT);
		ac++;
		view->topLevelShell = XtCreatePopupShell("topLevelShell", topLevelShellWidgetClass, parent_widget, args, ac);
		view->mainWindow = XmCreateMainWindow(view->topLevelShell, "mainWindow_mbview", args, ac);
		XtManageChild(view->mainWindow);
		MB3DViewCreate(&(view->mb3dview), view->mainWindow, "mbview_mbgrdviz", args, ac);

		ac = 0;
		XtSetArg(args[ac], XmNx, data->xo);
		ac++;
		XtSetArg(args[ac], XmNy, data->yo);
		ac++;
		XtSetArg(args[ac], XmNwidth, data->width + LEFT_WIDTH);
		ac++;
		XtSetArg(args[ac], XmNheight, data->height + LEFT_HEIGHT);
		ac++;
		XtSetValues(view->mb3dview.MB3DView, args, ac);
		XtManageChild(view->mb3dview.MB3DView);
		XtPopup(XtParent(view->mainWindow), XtGrabNone);

		/* get resize events - add event handlers */
		XtAddEventHandler(view->topLevelShell, StructureNotifyMask, False, mbview_resize, (XtPointer)instance);
		XtAddEventHandler(view->mb3dview.mbview_form_profile, StructureNotifyMask, False, do_mbview_profile_resize,
		                  (XtPointer)instance);

		/* intitialize OpenGL graphics */
		ac = 0;
		XtSetArg(args[ac], mbGLwNrgba, TRUE);
		ac++;
		XtSetArg(args[ac], mbGLwNdepthSize, 1);
		ac++;
		XtSetArg(args[ac], mbGLwNdoublebuffer, True);
		ac++;
		XtSetArg(args[ac], mbGLwNallocateBackground, TRUE);
		ac++;
		XtSetArg(args[ac], XmNwidth, data->width);
		ac++;
		XtSetArg(args[ac], XmNheight, data->height);
		ac++;
		view->dpy = (Display *)XtDisplay(view->mb3dview.MB3DView);
		view->glwmda = mbGLwCreateMDrawingArea(view->mb3dview.mbview_drawingArea_mbview, "glwidget", args, ac);
		/* view->glwmda = XtCreateWidget("glwidget", mbGLwDrawingAreaWidgetClass, view->mb3dview.mbview_drawingArea_mbview, args,
		 * ac);*/
		XtManageChild(view->glwmda);
		XtAddCallback(view->glwmda, "exposeCallback", &(do_mbview_glwda_expose), (XtPointer)NULL);
		XtAddCallback(view->glwmda, "resizeCallback", &(do_mbview_glwda_resize), (XtPointer)NULL);
		XtAddCallback(view->glwmda, "inputCallback", &(do_mbview_glwda_input), (XtPointer)NULL);
		XSelectInput(view->dpy, XtWindow(view->glwmda),
		             (ButtonPressMask | ButtonReleaseMask | ButtonMotionMask | KeyPressMask | KeyReleaseMask | ExposureMask));

		/* generate cursors for later use */
		view->xid = XtWindow(view->mb3dview.mbview_drawingArea_mbview);
		{
			XColor XColorBlack;
			XColor XColorWhite;
			XColor XColorRed;
			XColor XColorGreen;
			XColor XColorBlue;
			XColor XColorCoral;
			XColor exact;

			XAllocNamedColor(view->dpy, DefaultColormap(view->dpy, XDefaultScreen(view->dpy)), "red", &XColorRed, &exact);
			XAllocNamedColor(view->dpy, DefaultColormap(view->dpy, XDefaultScreen(view->dpy)), "green", &XColorGreen, &exact);
			XAllocNamedColor(view->dpy, DefaultColormap(view->dpy, XDefaultScreen(view->dpy)), "blue", &XColorBlue, &exact);
			XAllocNamedColor(view->dpy, DefaultColormap(view->dpy, XDefaultScreen(view->dpy)), "black", &XColorBlack, &exact);
			XAllocNamedColor(view->dpy, DefaultColormap(view->dpy, XDefaultScreen(view->dpy)), "white", &XColorWhite, &exact);
			XAllocNamedColor(view->dpy, DefaultColormap(view->dpy, XDefaultScreen(view->dpy)), "coral", &XColorCoral, &exact);
			view->TargetBlackCursor = XCreateFontCursor(view->dpy, XC_target);
			view->TargetGreenCursor = XCreateFontCursor(view->dpy, XC_target);
			view->TargetRedCursor = XCreateFontCursor(view->dpy, XC_target);
			view->FleurBlackCursor = XCreateFontCursor(view->dpy, XC_fleur);
			view->FleurRedCursor = XCreateFontCursor(view->dpy, XC_fleur);
			view->SizingBlackCursor = XCreateFontCursor(view->dpy, XC_sizing);
			view->SizingRedCursor = XCreateFontCursor(view->dpy, XC_sizing);
			view->BoatBlackCursor = XCreateFontCursor(view->dpy, XC_boat);
			view->BoatRedCursor = XCreateFontCursor(view->dpy, XC_boat);
			view->WatchBlackCursor = XCreateFontCursor(view->dpy, XC_watch);
			view->WatchRedCursor = XCreateFontCursor(view->dpy, XC_watch);
			XRecolorCursor(view->dpy, view->TargetRedCursor, &XColorRed, &XColorCoral);
			XRecolorCursor(view->dpy, view->TargetGreenCursor, &XColorGreen, &XColorCoral);
			XRecolorCursor(view->dpy, view->FleurRedCursor, &XColorRed, &XColorCoral);
			XRecolorCursor(view->dpy, view->SizingRedCursor, &XColorRed, &XColorCoral);
			XRecolorCursor(view->dpy, view->BoatRedCursor, &XColorRed, &XColorCoral);
			XRecolorCursor(view->dpy, view->WatchRedCursor, &XColorRed, &XColorCoral);
			XDefineCursor(view->dpy, view->xid, view->TargetBlackCursor);
		}

		/* set instance into XmNuserData resources */
		ac = 0;
		XtSetArg(args[ac], XmNuserData, (XtPointer)instance);
		ac++;
		XtSetValues(view->topLevelShell, args, ac);
		XtSetValues(view->mainWindow, args, ac);
		XtSetValues(view->mb3dview.MB3DView, args, ac);
		XtSetValues(view->mb3dview.mbview_pushButton_clearpicks, args, ac);
		XtSetValues(view->mb3dview.mbview_pushButton_reset, args, ac);
		XtSetValues(view->mb3dview.mbview_radioBox_mouse, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_rmove, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_rrotate, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_rviewpoint, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_rshade, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_rarea, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_rsite, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_rroute, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_rnav, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_rnavfile, args, ac);
		XtSetValues(view->mb3dview.mbview_label_status, args, ac);
		XtSetValues(view->mb3dview.mbview_pushButton_fullrez, args, ac);
		XtSetValues(view->mb3dview.mbview_label_pickinfo, args, ac);
		XtSetValues(view->mb3dview.mbview_menuBar_mbview, args, ac);
		XtSetValues(view->mb3dview.mbview_cascadeButton_view, args, ac);
		XtSetValues(view->mb3dview.mbview_pulldownMenu_view, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_display_2D, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_display_3D, args, ac);
		XtSetValues(view->mb3dview.mbview_separator10, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_data_primary, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_data_primaryslope, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_data_secondary, args, ac);
		XtSetValues(view->mb3dview.mbview_separator, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_histogram, args, ac);
		XtSetValues(view->mb3dview.mbview_separator21, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_overlay_none, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_overlay_illumination, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_overlay_slope, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_overlay_secondary, args, ac);
		XtSetValues(view->mb3dview.mbview_separator1, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_overlay_contour, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_site, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_route, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_nav, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_navdrape, args, ac);
		XtSetValues(view->mb3dview.mbview_separator8, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_colortable_haxby, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_colortable_bright, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_colortable_muted, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_colortable_gray, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_colortable_flat, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_colortable_sealevel1, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_colortable_sealevel2, args, ac);
		XtSetValues(view->mb3dview.separator1, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_profile, args, ac);
		XtSetValues(view->mb3dview.mbview_cascadeButton_controls, args, ac);
		XtSetValues(view->mb3dview.mbview_pulldownMenu_controls, args, ac);
		XtSetValues(view->mb3dview.mbview_pushButton_colorbounds, args, ac);
		XtSetValues(view->mb3dview.mbview_pushButton_2dview, args, ac);
		XtSetValues(view->mb3dview.mbview_pushButton_3dview, args, ac);
		XtSetValues(view->mb3dview.mbview_pushButton_shadeparms, args, ac);
		XtSetValues(view->mb3dview.mbview_pushButton_resolution, args, ac);
		XtSetValues(view->mb3dview.mbview_pushButton_projections, args, ac);
		XtSetValues(view->mb3dview.mbview_pushButton_sitelist, args, ac);
		XtSetValues(view->mb3dview.mbview_pushButton_routelist, args, ac);
		XtSetValues(view->mb3dview.mbview_pushButton_navlist, args, ac);
		XtSetValues(view->mb3dview.mbview_cascadeButton_mouse, args, ac);
		XtSetValues(view->mb3dview.mbview_pulldownMenu_mouse, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_move, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_rotate, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_viewpoint, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_shade, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_area, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_site, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_route, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_nav, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_navfile, args, ac);
		XtSetValues(view->mb3dview.mbview_cascadeButton_action, args, ac);
		XtSetValues(view->mb3dview.mbview_pulldownMenu_action, args, ac);
		XtSetValues(view->mb3dview.mbview_pushButton_help_about, args, ac);
		XtSetValues(view->mb3dview.mbview_cascadeButton_dismiss, args, ac);
		XtSetValues(view->mb3dview.mbview_pulldownMenu_dismiss, args, ac);
		XtSetValues(view->mb3dview.mbview_pushButton_dismiss, args, ac);
		XtSetValues(view->mb3dview.mbview_label_mouse, args, ac);
		XtSetValues(view->mb3dview.mbview_drawingArea_mbview, args, ac);
		XtSetValues(view->mb3dview.mbview_dialogShell_colorbounds, args, ac);
		XtSetValues(view->mb3dview.mbview_bulletinBoard_colorbounds, args, ac);
		XtSetValues(view->mb3dview.mbview_separator5, args, ac);
		XtSetValues(view->mb3dview.mbview_radioBox_overlaymode, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_overlay_ctoh, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_overlay_htoc, args, ac);
		XtSetValues(view->mb3dview.mbview_textField_overlaymax, args, ac);
		XtSetValues(view->mb3dview.mbview_label_overlaymax, args, ac);
		XtSetValues(view->mb3dview.mbview_textField_overlaymin, args, ac);
		XtSetValues(view->mb3dview.mbview_label_overlaymin, args, ac);
		XtSetValues(view->mb3dview.mbview_label_overlaybounds, args, ac);
		XtSetValues(view->mb3dview.mbview_separator3, args, ac);
		XtSetValues(view->mb3dview.mbview_radioBox_slopemode, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_slope_ctoh, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_slope_htoc, args, ac);
		XtSetValues(view->mb3dview.mbview_textField_slopemax, args, ac);
		XtSetValues(view->mb3dview.mbview_label_slopemax, args, ac);
		XtSetValues(view->mb3dview.mbview_textField_slopemin, args, ac);
		XtSetValues(view->mb3dview.mbview_label_slopemin, args, ac);
		XtSetValues(view->mb3dview.mbview_label_slopebounds, args, ac);
		XtSetValues(view->mb3dview.mbview_radioBox_colormode, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_data_ctoh, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_data_htoc, args, ac);
		XtSetValues(view->mb3dview.mbview_textField_datamax, args, ac);
		XtSetValues(view->mb3dview.mbview_textField_datamin, args, ac);
		XtSetValues(view->mb3dview.mbview_label_colormax, args, ac);
		XtSetValues(view->mb3dview.mbview_label_colormin, args, ac);
		XtSetValues(view->mb3dview.mbview_label_colorbounds, args, ac);
		XtSetValues(view->mb3dview.mbview_separator2, args, ac);
		XtSetValues(view->mb3dview.mbview_pushButton_colorbounds_apply, args, ac);
		XtSetValues(view->mb3dview.mbview_label_contour, args, ac);
		XtSetValues(view->mb3dview.mbview_textField_contours, args, ac);
		XtSetValues(view->mb3dview.mbview_pushButton_colorbounds_dismiss, args, ac);
		XtSetValues(view->mb3dview.mbview_dialogShell_resolution, args, ac);
		XtSetValues(view->mb3dview.mbview_bulletinBoard_resolution, args, ac);
		XtSetValues(view->mb3dview.mbview_scale_navmediumresolution, args, ac);
		XtSetValues(view->mb3dview.mbview_scale_navlowresolution, args, ac);
		XtSetValues(view->mb3dview.separator, args, ac);
		XtSetValues(view->mb3dview.mbview_label_navrenderdecimation, args, ac);
		XtSetValues(view->mb3dview.mbview_label_gridrenderres, args, ac);
		XtSetValues(view->mb3dview.mbview_scale_mediumresolution, args, ac);
		XtSetValues(view->mb3dview.mbview_scale_lowresolution, args, ac);
		XtSetValues(view->mb3dview.mbview_pushButton_resolution_dismiss, args, ac);
		XtSetValues(view->mb3dview.mbview_dialogShell_message, args, ac);
		XtSetValues(view->mb3dview.mbview_bulletinBoard_message, args, ac);
		XtSetValues(view->mb3dview.mbview_label_message, args, ac);
		XtSetValues(view->mb3dview.mbview_label_thanks, args, ac);
		XtSetValues(view->mb3dview.mbview_dialogShell_about, args, ac);
		XtSetValues(view->mb3dview.mbview_bulletinBoard_about, args, ac);
		XtSetValues(view->mb3dview.mbview_label_about_version, args, ac);
		XtSetValues(view->mb3dview.mbview_label_about_authors, args, ac);
		XtSetValues(view->mb3dview.mbview_label_about_MBARI, args, ac);
		XtSetValues(view->mb3dview.mbview_label_about_LDEO, args, ac);
		XtSetValues(view->mb3dview.mbview_separator6, args, ac);
		XtSetValues(view->mb3dview.mbview_label_about_mbsystem, args, ac);
		XtSetValues(view->mb3dview.mbview_separator7, args, ac);
		XtSetValues(view->mb3dview.mbview_label_about_title, args, ac);
		XtSetValues(view->mb3dview.mbview_pushButton_about_dismiss, args, ac);
		XtSetValues(view->mb3dview.mbview_dialogShell_shadeparms, args, ac);
		XtSetValues(view->mb3dview.mbview_bulletinBoard_shadeparms, args, ac);
		XtSetValues(view->mb3dview.mbview_separator13, args, ac);
		XtSetValues(view->mb3dview.mbview_textField_overlay_center, args, ac);
		XtSetValues(view->mb3dview.mbview_label_overlay_center, args, ac);
		XtSetValues(view->mb3dview.mbview_label_overlayshade, args, ac);
		XtSetValues(view->mb3dview.mbview_radioBox_overlay_shade, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_overlay_shade_ctoh, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_overlay_shade_htoc, args, ac);
		XtSetValues(view->mb3dview.mbview_textField_overlay_amp, args, ac);
		XtSetValues(view->mb3dview.mbview_label_overlay_amp, args, ac);
		XtSetValues(view->mb3dview.mbview_separator15, args, ac);
		XtSetValues(view->mb3dview.mbview_textField_slope_amp, args, ac);
		XtSetValues(view->mb3dview.mbview_label_slope_amp, args, ac);
		XtSetValues(view->mb3dview.mbview_label_slopeshade, args, ac);
		XtSetValues(view->mb3dview.mbview_textField_illum_azi, args, ac);
		XtSetValues(view->mb3dview.mbview_textField_illum_amp, args, ac);
		XtSetValues(view->mb3dview.mbview_label_illum_azi, args, ac);
		XtSetValues(view->mb3dview.mbview_label_illum_amp, args, ac);
		XtSetValues(view->mb3dview.mbview_label_illumination, args, ac);
		XtSetValues(view->mb3dview.mbview_separator16, args, ac);
		XtSetValues(view->mb3dview.mbview_pushButton_shadeparms_apply, args, ac);
		XtSetValues(view->mb3dview.mbview_label_illum_elev, args, ac);
		XtSetValues(view->mb3dview.mbview_textField_illum_elev, args, ac);
		XtSetValues(view->mb3dview.mbview_pushButton_shadeparms_dismiss2, args, ac);
		XtSetValues(view->mb3dview.mbview_dialogShell_3dparms, args, ac);
		XtSetValues(view->mb3dview.mbview_bulletinBoard_3dparms, args, ac);
		XtSetValues(view->mb3dview.mbview_textField_model_3dzoom, args, ac);
		XtSetValues(view->mb3dview.mbview_label_model_3dzoom, args, ac);
		XtSetValues(view->mb3dview.mbview_separator11, args, ac);
		XtSetValues(view->mb3dview.mbview_textField_view_3dzoom, args, ac);
		XtSetValues(view->mb3dview.mbview_label_view_3dzoom, args, ac);
		XtSetValues(view->mb3dview.mbview_textField_view_3doffsety, args, ac);
		XtSetValues(view->mb3dview.mbview_label_view_3doffsety, args, ac);
		XtSetValues(view->mb3dview.mbview_separator20, args, ac);
		XtSetValues(view->mb3dview.mbview_textField_view_3doffsetx, args, ac);
		XtSetValues(view->mb3dview.mbview_label_view_3doffsetx, args, ac);
		XtSetValues(view->mb3dview.mbview_label_view_offset, args, ac);
		XtSetValues(view->mb3dview.mbview_textField_view_elevation, args, ac);
		XtSetValues(view->mb3dview.mbview_label_view_elevation, args, ac);
		XtSetValues(view->mb3dview.mbview_separator4, args, ac);
		XtSetValues(view->mb3dview.mbview_textField_view_azimuth, args, ac);
		XtSetValues(view->mb3dview.mbview_label_view_azimuth, args, ac);
		XtSetValues(view->mb3dview.mbview_label_view, args, ac);
		XtSetValues(view->mb3dview.mbview_textField_model_elevation, args, ac);
		XtSetValues(view->mb3dview.mbview_textField_model_azimuth, args, ac);
		XtSetValues(view->mb3dview.mbview_label_model_elevation, args, ac);
		XtSetValues(view->mb3dview.mbview_label_model_azimuth, args, ac);
		XtSetValues(view->mb3dview.mbview_label_model, args, ac);
		XtSetValues(view->mb3dview.mbview_separator9, args, ac);
		XtSetValues(view->mb3dview.mbview_pushButton_view_3d_apply, args, ac);
		XtSetValues(view->mb3dview.mbview_label_exager, args, ac);
		XtSetValues(view->mb3dview.mbview_textField_exageration, args, ac);
		XtSetValues(view->mb3dview.mbview_pushButton_view_3d_dismiss, args, ac);
		XtSetValues(view->mb3dview.mbview_dialogShell_2dparms, args, ac);
		XtSetValues(view->mb3dview.mbview_bulletinBoard_2dparms, args, ac);
		XtSetValues(view->mb3dview.mbview_textField_view_2dzoom, args, ac);
		XtSetValues(view->mb3dview.mbview_label_view_2dzoom, args, ac);
		XtSetValues(view->mb3dview.mbview_textField_view_2doffsety, args, ac);
		XtSetValues(view->mb3dview.mbview_label_view_2doffsety, args, ac);
		XtSetValues(view->mb3dview.mbview_separator14, args, ac);
		XtSetValues(view->mb3dview.mbview_textField_view_2doffsetx, args, ac);
		XtSetValues(view->mb3dview.mbview_label_view_2doffsetx, args, ac);
		XtSetValues(view->mb3dview.mbview_label_2d_offset, args, ac);
		XtSetValues(view->mb3dview.mbview_pushButton_view_2d_apply, args, ac);
		XtSetValues(view->mb3dview.mbview_pushButton_view_2d_dismiss, args, ac);
		XtSetValues(view->mb3dview.mbview_dialogShell_projection, args, ac);
		XtSetValues(view->mb3dview.mbview_bulletinBoard_projection, args, ac);
		XtSetValues(view->mb3dview.mbview_label_displayprojection, args, ac);
		XtSetValues(view->mb3dview.mbview_radioBox_projection, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_geographic, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_utm, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_spheroid, args, ac);
		XtSetValues(view->mb3dview.mbview_label_projection, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_annotation_degreesminutes, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_annotation_degreesdecimal, args, ac);
		XtSetValues(view->mb3dview.mbview_pushButton_projection_dismiss, args, ac);
		XtSetValues(view->mb3dview.mbview_dialogShell_profile, args, ac);
		XtSetValues(view->mb3dview.mbview_form_profile, args, ac);
		XtSetValues(view->mb3dview.mbview_scale_profile_width, args, ac);
		XtSetValues(view->mb3dview.mbview_scale_profile_slope, args, ac);
		XtSetValues(view->mb3dview.mbview_scrolledWindow_profile, args, ac);
		XtSetValues(view->mb3dview.mbview_drawingArea_profile, args, ac);
		XtSetValues(view->mb3dview.mbview_profile_label_info, args, ac);
		XtSetValues(view->mb3dview.mbview_scale_profile_exager, args, ac);
		XtSetValues(view->mb3dview.mbview_profile_pushButton_dismiss, args, ac);
		XtSetValues(view->glwmda, args, ac);
		/* set the initialization flag */
		view->init = MBV_WINDOW_VISIBLE;
	}

	/* make sure some key parameters are set */
	view->projected = false;
	view->globalprojected = false;
	view->lastdrawrez = MBV_REZ_NONE;
	view->viewboundscount = MBV_BOUNDSFREQUENCY;
	mbview_zscaleclear(instance);
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);
	view->contourlorez = false;
	view->contourhirez = false;
	view->contourfullrez = false;
	view->primary_histogram_set = false;
	view->primaryslope_histogram_set = false;
	view->secondary_histogram_set = false;
	if (data->primary_colortable_max <= data->primary_colortable_min) {
		data->primary_colortable_min = data->primary_min - 0.01 * (data->primary_max - data->primary_min);
		data->primary_colortable_max = data->primary_max + 0.01 * (data->primary_max - data->primary_min);
		data->contour_interval = pow(10.0, floor(log10(data->primary_max - data->primary_min)) - 1.0);
	}

	/* set about version label */
  mb_path value_text;
	sprintf(value_text, "::#TimesMedium14:t\"MB-System Release %s\"#TimesMedium14\"%s\"", MB_VERSION, MB_VERSION_DATE);
	set_mbview_label_multiline_string(view->mb3dview.mbview_label_about_version, value_text);

	/* set widgets */
	mbview_set_widgets(verbose, instance, error);

	/* set widget sensitivity */
	mbview_update_sensitivity(verbose, instance, error);
	//fprintf(stderr,"Calling mbview_action_sensitivityall()\n");
	mbview_action_sensitivityall();
	//fprintf(stderr,"Done with mbview_action_sensitivityall()\n");

	/* create glx context */
	//fprintf(stderr,"Calling mbview_reset_glx(%zu)\n",instance);
	mbview_reset_glx(instance);
	//fprintf(stderr,"Done with mbview_reset_glx()\n");

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:        %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:       %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_update(int verbose, size_t instance, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* make sure some key parameters are set */
	/*view->projected = false;*/
	/*view->globalprojected = false;*/
	view->lastdrawrez = MBV_REZ_NONE;
	view->viewboundscount = MBV_BOUNDSFREQUENCY;
	mbview_zscaleclear(instance);
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);
	view->contourlorez = false;
	view->contourhirez = false;
	view->contourfullrez = false;
	view->primary_histogram_set = false;
	view->primaryslope_histogram_set = false;
	view->secondary_histogram_set = false;
	if (data->primary_nxy > 0 && data->primary_colortable_max <= data->primary_colortable_min) {
		data->primary_colortable_min = data->primary_min - 0.01 * (data->primary_max - data->primary_min);
		data->primary_colortable_max = data->primary_max + 0.01 * (data->primary_max - data->primary_min);
		data->contour_interval = pow(10.0, floor(log10(data->primary_max - data->primary_min)) - 1.0);
	}
	if (data->secondary_nxy > 0 && data->secondary_colortable_max <= data->secondary_colortable_min) {
		data->secondary_colortable_min = data->secondary_min - 0.01 * (data->secondary_max - data->secondary_min);
		data->secondary_colortable_max = data->secondary_max + 0.01 * (data->secondary_max - data->secondary_min);
		data->overlay_shade_center = 0.5 * (data->secondary_max + data->secondary_min);
	}

	/* set widgets */
	if (data->active)
		mbview_set_widgets(verbose, instance, error);

	/* set widget sensitivity */
	if (data->active)
		mbview_update_sensitivity(verbose, instance, error);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from mbview_update\n");
	mbview_plotlowhigh(instance);
	mbview_plotlowhighall(instance);

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:        %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:       %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_update_sensitivity(int verbose, size_t instance, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* set widget sensitivity */

	Arg args[256];
  Cardinal ac = 0;
	if (data->display_projection_mode == MBV_PROJECTION_SPHEROID) {
		XtSetArg(args[ac], XmNsensitive, False);
		ac++;
		data->display_mode = MBV_DISPLAY_3D;
	}
	else {
		XtSetArg(args[ac], XmNsensitive, True);
		ac++;
	}
	XtSetValues(view->mb3dview.mbview_toggleButton_display_2D, args, ac);

	ac = 0;
	if (data->primary_nxy <= 0 || data->primary_data == NULL) {
		XtSetArg(args[ac], XmNsensitive, False);
		ac++;
	}
	else {
		XtSetArg(args[ac], XmNsensitive, True);
		ac++;
	}
	XtSetValues(view->mb3dview.mbview_toggleButton_data_primary, args, ac);
	XtSetValues(view->mb3dview.mbview_toggleButton_data_primaryslope, args, ac);
	XtSetValues(view->mb3dview.mbview_toggleButton_histogram, args, ac);
	XtSetValues(view->mb3dview.mbview_toggleButton_overlay_none, args, ac);
	XtSetValues(view->mb3dview.mbview_toggleButton_overlay_illumination, args, ac);
	XtSetValues(view->mb3dview.mbview_toggleButton_overlay_slope, args, ac);
	XtSetValues(view->mb3dview.mbview_toggleButton_overlay_contour, args, ac);
	XtSetValues(view->mb3dview.mbview_toggleButton_profile, args, ac);
	XtSetValues(view->mb3dview.mbview_radioBox_slopemode, args, ac);
	XtSetValues(view->mb3dview.mbview_toggleButton_slope_ctoh, args, ac);
	XtSetValues(view->mb3dview.mbview_toggleButton_slope_htoc, args, ac);
	XtSetValues(view->mb3dview.mbview_textField_slopemax, args, ac);
	XtSetValues(view->mb3dview.mbview_textField_slopemin, args, ac);
	XtSetValues(view->mb3dview.mbview_radioBox_colormode, args, ac);
	XtSetValues(view->mb3dview.mbview_toggleButton_data_ctoh, args, ac);
	XtSetValues(view->mb3dview.mbview_toggleButton_data_htoc, args, ac);
	XtSetValues(view->mb3dview.mbview_textField_datamax, args, ac);
	XtSetValues(view->mb3dview.mbview_textField_datamin, args, ac);
	XtSetValues(view->mb3dview.mbview_textField_contours, args, ac);
	XtSetValues(view->mb3dview.mbview_textField_slope_amp, args, ac);
	XtSetValues(view->mb3dview.mbview_textField_illum_azi, args, ac);
	XtSetValues(view->mb3dview.mbview_textField_illum_amp, args, ac);
	XtSetValues(view->mb3dview.mbview_textField_illum_elev, args, ac);
	XtSetValues(view->mb3dview.mbview_pushButton_shadeparms_apply, args, ac);
	XtSetValues(view->mb3dview.mbview_textField_model_azimuth, args, ac);
	XtSetValues(view->mb3dview.mbview_textField_model_elevation, args, ac);
	XtSetValues(view->mb3dview.mbview_textField_model_3dzoom, args, ac);
	XtSetValues(view->mb3dview.mbview_label_model_3dzoom, args, ac);
	XtSetValues(view->mb3dview.mbview_textField_exageration, args, ac);
	XtSetValues(view->mb3dview.mbview_textField_view_azimuth, args, ac);
	XtSetValues(view->mb3dview.mbview_textField_view_elevation, args, ac);
	XtSetValues(view->mb3dview.mbview_textField_view_3doffsetx, args, ac);
	XtSetValues(view->mb3dview.mbview_textField_view_3doffsety, args, ac);
	XtSetValues(view->mb3dview.mbview_textField_view_3dzoom, args, ac);
	XtSetValues(view->mb3dview.mbview_pushButton_view_3d_apply, args, ac);
	XtSetValues(view->mb3dview.mbview_textField_view_2doffsetx, args, ac);
	XtSetValues(view->mb3dview.mbview_textField_view_2doffsety, args, ac);
	XtSetValues(view->mb3dview.mbview_textField_view_2dzoom, args, ac);
	XtSetValues(view->mb3dview.mbview_pushButton_view_2d_apply, args, ac);

	ac = 0;
	if (data->secondary_nxy <= 0 || data->secondary_data == NULL) {
		XtSetArg(args[ac], XmNsensitive, False);
		ac++;
	}
	else {
		XtSetArg(args[ac], XmNsensitive, True);
		ac++;
	}
	XtSetValues(view->mb3dview.mbview_toggleButton_data_secondary, args, ac);
	XtSetValues(view->mb3dview.mbview_toggleButton_overlay_secondary, args, ac);
	XtSetValues(view->mb3dview.mbview_radioBox_overlaymode, args, ac);
	XtSetValues(view->mb3dview.mbview_toggleButton_overlay_ctoh, args, ac);
	XtSetValues(view->mb3dview.mbview_toggleButton_overlay_htoc, args, ac);
	XtSetValues(view->mb3dview.mbview_textField_overlaymax, args, ac);
	XtSetValues(view->mb3dview.mbview_textField_overlaymin, args, ac);
	XtSetValues(view->mb3dview.mbview_radioBox_overlay_shade, args, ac);
	XtSetValues(view->mb3dview.mbview_toggleButton_overlay_shade_ctoh, args, ac);
	XtSetValues(view->mb3dview.mbview_toggleButton_overlay_shade_htoc, args, ac);
	XtSetValues(view->mb3dview.mbview_textField_overlay_amp, args, ac);
	XtSetValues(view->mb3dview.mbview_textField_overlay_center, args, ac);

	ac = 0;
	if (shared.shareddata.site_mode == MBV_SITE_OFF) {
		XtSetArg(args[ac], XmNsensitive, False);
		ac++;
	}
	else {
		XtSetArg(args[ac], XmNsensitive, True);
		ac++;
	}
	XtSetValues(view->mb3dview.mbview_toggleButton_site, args, ac);
	XtSetValues(view->mb3dview.mbview_toggleButton_mode_site, args, ac);
	XtSetValues(view->mb3dview.mbview_toggleButton_mode_rsite, args, ac);
	if (shared.shareddata.site_mode != MBV_SITE_EDIT) {
		set_mbview_label_string(view->mb3dview.mbview_toggleButton_mode_site, "Pick Sites");
		set_mbview_label_string(view->mb3dview.mbview_toggleButton_mode_rsite, "Pick Sites");
	}
	else {
		set_mbview_label_string(view->mb3dview.mbview_toggleButton_mode_site, "Edit Sites");
		set_mbview_label_string(view->mb3dview.mbview_toggleButton_mode_rsite, "Edit Sites");
	}

	ac = 0;
	if (shared.shareddata.route_mode == MBV_ROUTE_OFF) {
		XtSetArg(args[ac], XmNsensitive, False);
		ac++;
	}
	else {
		XtSetArg(args[ac], XmNsensitive, True);
		ac++;
	}
	XtSetValues(view->mb3dview.mbview_toggleButton_route, args, ac);
	XtSetValues(view->mb3dview.mbview_toggleButton_mode_route, args, ac);
	XtSetValues(view->mb3dview.mbview_toggleButton_mode_rroute, args, ac);
	if (shared.shareddata.route_mode == MBV_ROUTE_EDIT) {
		set_mbview_label_string(view->mb3dview.mbview_toggleButton_mode_route, "Edit Routes");
		set_mbview_label_string(view->mb3dview.mbview_toggleButton_mode_rroute, "Edit Routes");
	}
	else if (shared.shareddata.route_mode == MBV_ROUTE_NAVADJUST) {
		set_mbview_label_string(view->mb3dview.mbview_toggleButton_mode_route, "Pick Ties");
		set_mbview_label_string(view->mb3dview.mbview_toggleButton_mode_rroute, "Pick Ties");
	}
	else {
		set_mbview_label_string(view->mb3dview.mbview_toggleButton_mode_route, "Edit Routes");
		set_mbview_label_string(view->mb3dview.mbview_toggleButton_mode_rroute, "Edit Routes");
	}

	ac = 0;
	if (shared.shareddata.nav_mode == MBV_NAV_OFF) {
		XtSetArg(args[ac], XmNsensitive, False);
		ac++;
		XtSetValues(view->mb3dview.mbview_toggleButton_nav, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_navdrape, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_nav, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_rnav, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_navfile, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_rnavfile, args, ac);
		set_mbview_label_string(view->mb3dview.mbview_toggleButton_mode_nav, "Pick Nav");
		set_mbview_label_string(view->mb3dview.mbview_toggleButton_mode_rnav, "Pick Nav");
		set_mbview_label_string(view->mb3dview.mbview_toggleButton_mode_navfile, "Pick Nav File");
		set_mbview_label_string(view->mb3dview.mbview_toggleButton_mode_rnavfile, "Pick Nav File");
	}
	else if (shared.shareddata.nav_mode == MBV_NAV_VIEW) {
		XtSetArg(args[ac], XmNsensitive, True);
		ac++;
		XtSetValues(view->mb3dview.mbview_toggleButton_nav, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_navdrape, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_nav, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_rnav, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_navfile, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_rnavfile, args, ac);
		set_mbview_label_string(view->mb3dview.mbview_toggleButton_mode_nav, "Pick Nav");
		set_mbview_label_string(view->mb3dview.mbview_toggleButton_mode_rnav, "Pick Nav");
		set_mbview_label_string(view->mb3dview.mbview_toggleButton_mode_navfile, "Pick Nav File");
		set_mbview_label_string(view->mb3dview.mbview_toggleButton_mode_rnavfile, "Pick Nav File");
	}
	else // if (shared.shareddata.nav_mode == MBV_NAV_MBNAVADJUST)
	{
		XtSetArg(args[ac], XmNsensitive, True);
		ac++;
		XtSetValues(view->mb3dview.mbview_toggleButton_nav, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_navdrape, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_navfile, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_rnavfile, args, ac);
		XtSetArg(args[ac], XmNsensitive, False);
		ac++;
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_nav, args, ac);
		XtSetValues(view->mb3dview.mbview_toggleButton_mode_rnav, args, ac);
		set_mbview_label_string(view->mb3dview.mbview_toggleButton_mode_nav, "Pick Nav");
		set_mbview_label_string(view->mb3dview.mbview_toggleButton_mode_rnav, "Pick Nav");
		set_mbview_label_string(view->mb3dview.mbview_toggleButton_mode_navfile, "Pick Nav Section");
		set_mbview_label_string(view->mb3dview.mbview_toggleButton_mode_rnavfile, "Pick Nav Section");
	}

	ac = 0;
	if (shared.shareddata.vector_mode == MBV_VECTOR_OFF) {
		XtSetArg(args[ac], XmNsensitive, False);
		ac++;
	}
	else {
		XtSetArg(args[ac], XmNsensitive, True);
		ac++;
	}
	XtSetValues(view->mb3dview.mbview_toggleButton_vector, args, ac);

	/* now set action buttons according to current pick states */
	mbview_action_sensitivity(instance);

	/* reset sensitivity in parent program */
	/* fprintf(stderr,"could call mbview_sensitivity_notify:%d\n",data->mbview_sensitivity_notify); */
	if (data->mbview_sensitivity_notify != NULL)
		(data->mbview_sensitivity_notify)();

	int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:        %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:       %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_action_sensitivityall() {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       mbv_verbose:               %d\n", mbv_verbose);
	}

	/* set action sensitivity on all active instances */
	for (size_t instance = 0; instance < MBV_MAX_WINDOWS; instance++) {
			struct mbview_world_struct *view = &(mbviews[instance]);
		struct mbview_struct *data = &(view->data);

		/* if instance active reset action sensitivity */
		if (data->active)
			mbview_action_sensitivity(instance);
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:       %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_action_sensitivity(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       mbv_verbose:               %d\n", mbv_verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	/* check if all available instances are active */
	bool mbview_allactive = true;
	for (int i = 0; i < MBV_MAX_WINDOWS; i++) {
		if (!mbviews[i].data.active)
			mbview_allactive = false;
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* now set action buttons according to current pick states */
	for (int i = 0; i < view->naction; i++) {
		if (view->pushButton_action[i] != NULL) {
			Arg args[256];
      Cardinal ac = 0;
			XtSetArg(args[ac], XmNsensitive, False);
			if (view->actionsensitive[i] == MBV_PICKMASK_NONE) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			else if (view->actionsensitive[i] & MBV_PICKMASK_ONEPOINT && data->pick_type == MBV_PICK_ONEPOINT) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			else if (view->actionsensitive[i] & MBV_PICKMASK_TWOPOINT && data->pick_type == MBV_PICK_TWOPOINT) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			else if (view->actionsensitive[i] & MBV_PICKMASK_AREA && data->area_type == MBV_AREA_QUAD) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			else if (view->actionsensitive[i] & MBV_PICKMASK_REGION && data->region_type == MBV_REGION_QUAD) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			else if (view->actionsensitive[i] & MBV_PICKMASK_SITE && shared.shareddata.site_selected >= 0) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			else if (view->actionsensitive[i] & MBV_PICKMASK_ROUTE && shared.shareddata.route_selected >= 0) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			else if (view->actionsensitive[i] & MBV_PICKMASK_NAVONEPOINT && shared.shareddata.navpick_type == MBV_PICK_ONEPOINT) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			else if (view->actionsensitive[i] & MBV_PICKMASK_NAVTWOPOINT && shared.shareddata.navpick_type == MBV_PICK_TWOPOINT) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			else if (view->actionsensitive[i] & MBV_PICKMASK_NAVANY) {
				for (int j = 0; j < shared.shareddata.nnav; j++) {
					if (shared.shareddata.navs[j].nselected > 0)
						XtSetArg(args[ac], XmNsensitive, True);
				}
			}
			else if (view->actionsensitive[i] & MBV_EXISTMASK_SITE && shared.shareddata.nsite > 0) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			else if (view->actionsensitive[i] & MBV_EXISTMASK_ROUTE && shared.shareddata.nroute > 0) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			if (view->actionsensitive[i] & MBV_PICKMASK_NEWINSTANCE && mbview_allactive) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			if (view->actionsensitive[i] & MBV_STATEMASK_13 && data->state13) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			if (view->actionsensitive[i] & MBV_STATEMASK_14 && data->state14) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			if (view->actionsensitive[i] & MBV_STATEMASK_15 && data->state15) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			if (view->actionsensitive[i] & MBV_STATEMASK_16 && data->state16) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			if (view->actionsensitive[i] & MBV_STATEMASK_17 && data->state17) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			if (view->actionsensitive[i] & MBV_STATEMASK_18 && data->state18) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			if (view->actionsensitive[i] & MBV_STATEMASK_19 && data->state19) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			if (view->actionsensitive[i] & MBV_STATEMASK_20 && data->state20) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			if (view->actionsensitive[i] & MBV_STATEMASK_21 && data->state21) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			if (view->actionsensitive[i] & MBV_STATEMASK_22 && data->state22) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			if (view->actionsensitive[i] & MBV_STATEMASK_23 && data->state23) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			if (view->actionsensitive[i] & MBV_STATEMASK_24 && data->state24) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			if (view->actionsensitive[i] & MBV_STATEMASK_25 && data->state25) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			if (view->actionsensitive[i] & MBV_STATEMASK_26 && data->state26) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			if (view->actionsensitive[i] & MBV_STATEMASK_27 && data->state27) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			if (view->actionsensitive[i] & MBV_STATEMASK_28 && data->state28) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			if (view->actionsensitive[i] & MBV_STATEMASK_29 && data->state29) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			if (view->actionsensitive[i] & MBV_STATEMASK_30 && data->state30) {
				XtSetArg(args[ac], XmNsensitive, True);
			}
			if (view->actionsensitive[i] & MBV_STATEMASK_31 && data->state31) {
				XtSetArg(args[ac], XmNsensitive, True);
			}

			ac++;
			XtSetValues(view->pushButton_action[i], args, ac);
		}
	}

	/* reset sensitivity in parent program */
	/* fprintf(stderr,"could call mbview_sensitivity_notify:%d\n",data->mbview_sensitivity_notify); */
	if (data->mbview_sensitivity_notify != NULL)
		(data->mbview_sensitivity_notify)();

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:       %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_set_widgets(int verbose, size_t instance, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       instance:        %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* check for spheroid projection */
	if (data->display_projection_mode == MBV_PROJECTION_SPHEROID) {
		data->display_mode = MBV_DISPLAY_3D;
	}

	/* set widgets */
	set_mbview_display_mode(instance, data->display_mode);
	set_mbview_mouse_mode(instance, data->mouse_mode);
	set_mbview_grid_mode(instance, data->grid_mode);
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY) {
		set_mbview_histogram_mode(instance, data->primary_histogram);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE) {
		set_mbview_histogram_mode(instance, data->primaryslope_histogram);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY) {
		set_mbview_histogram_mode(instance, data->secondary_histogram);
	}
	set_mbview_contour_mode(instance, data->grid_contour_mode);
	set_mbview_site_view_mode(instance, data->site_view_mode);
	set_mbview_route_view_mode(instance, data->route_view_mode);
	set_mbview_nav_view_mode(instance, data->nav_view_mode);
	set_mbview_navdrape_view_mode(instance, data->navdrape_view_mode);
	set_mbview_vector_view_mode(instance, data->vector_view_mode);
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY) {
		set_mbview_colortable(instance, data->primary_colortable);
		set_mbview_colortable_mode(instance, data->primary_colortable_mode);
		set_mbview_shade_mode(instance, data->primary_shade_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE) {
		set_mbview_colortable(instance, data->slope_colortable);
		set_mbview_colortable_mode(instance, data->slope_colortable_mode);
		set_mbview_shade_mode(instance, data->slope_shade_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY) {
		set_mbview_colortable(instance, data->secondary_colortable);
		set_mbview_colortable_mode(instance, data->secondary_colortable_mode);
		set_mbview_shade_mode(instance, data->secondary_shade_mode);
	}

	/* reset if mouse radiobox controls are visible or not */
	if (data->height > MBV_WINDOW_HEIGHT_THRESHOLD) {
		XtManageChild(view->mb3dview.mbview_radioBox_mouse);
	}
	else {
		XtUnmanageChild(view->mb3dview.mbview_radioBox_mouse);
	}

	/* set pick annotation */
	mbview_pick_text(instance);

	/* set projection label */
	do_mbview_set_projection_label(instance);

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:        %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_addaction(int verbose, size_t instance, void(mbview_action_notify)(Widget, XtPointer, XtPointer), char *label,
                     int sensitive, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:              %d\n", verbose);
		fprintf(stderr, "dbg2       instance:             %zu\n", instance);
		fprintf(stderr, "dbg2       mbview_action_notify: %p\n", mbview_action_notify);
		fprintf(stderr, "dbg2       label:                %s\n", label);
		fprintf(stderr, "dbg2       sensitive:            %d\n", sensitive);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);

	/* add pushbutton to action menu */
	Arg args[256];
  Cardinal ac = 0;
	Boolean argok = False;
	XmString tmp0 = (XmString)BX_CONVERT(view->mb3dview.mbview_pulldownMenu_action, label, XmRXmString, 0, &argok);
	XtSetArg(args[ac], XmNlabelString, tmp0);
	if (argok)
		ac++;
	XtSetArg(args[ac], XmNfontList,
	         BX_CONVERT(view->mb3dview.mbview_pulldownMenu_action, "-*-" SANS "-bold-r-*-*-*-140-75-75-*-*-iso8859-1",
	                    XmRFontList, 0, &argok));
	if (argok)
		ac++;
	XtSetArg(args[ac], XmNuserData, (XtPointer)instance);
	ac++;
	view->pushButton_action[view->naction] =
	    (Widget)XmCreatePushButton(view->mb3dview.mbview_pulldownMenu_action, label, args, ac);
	view->actionsensitive[view->naction] = sensitive;
	XmStringFree((XmString)tmp0);
	XtManageChild(view->pushButton_action[view->naction]);
	XtAddCallback(view->pushButton_action[view->naction], XmNactivateCallback, mbview_action_notify, (XtPointer)instance);
	view->naction++;

	/* now set action buttons according to current pick states */
	mbview_action_sensitivity(instance);

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:        %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_setstate(int verbose, size_t instance, int mask, int value, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:              %d\n", verbose);
		fprintf(stderr, "dbg2       instance:             %zu\n", instance);
		fprintf(stderr, "dbg2       mask:                 %d\n", mask);
		fprintf(stderr, "dbg2       value:                %d\n", value);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

  /* set state value */
  if (mask & MBV_STATEMASK_13)
    data->state13 = value;
  if (mask & MBV_STATEMASK_14)
    data->state14 = value;
  if (mask & MBV_STATEMASK_15)
    data->state15 = value;
  if (mask & MBV_STATEMASK_16)
    data->state16 = value;
  if (mask & MBV_STATEMASK_17)
    data->state17 = value;
  if (mask & MBV_STATEMASK_18)
    data->state18 = value;
  if (mask & MBV_STATEMASK_19)
    data->state19 = value;
  if (mask & MBV_STATEMASK_20)
    data->state20 = value;
  if (mask & MBV_STATEMASK_21)
    data->state21 = value;
  if (mask & MBV_STATEMASK_22)
    data->state22 = value;
  if (mask & MBV_STATEMASK_23)
    data->state23 = value;
  if (mask & MBV_STATEMASK_24)
    data->state24 = value;
  if (mask & MBV_STATEMASK_25)
    data->state25 = value;
  if (mask & MBV_STATEMASK_26)
    data->state26 = value;
  if (mask & MBV_STATEMASK_27)
    data->state27 = value;
  if (mask & MBV_STATEMASK_28)
    data->state28 = value;
  if (mask & MBV_STATEMASK_29)
    data->state29 = value;
  if (mask & MBV_STATEMASK_30)
    data->state30 = value;
  if (mask & MBV_STATEMASK_31)
    data->state31 = value;

	/* now set action buttons according to current pick states */
	mbview_action_sensitivity(instance);

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:        %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_addpicknotify(int verbose, size_t instance, int picktype, void(mbview_pick_notify)(size_t), int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:              %d\n", verbose);
		fprintf(stderr, "dbg2       instance:             %zu\n", instance);
		fprintf(stderr, "dbg2       picktype:             %d\n", picktype);
		fprintf(stderr, "dbg2       mbview_pick_notify:   %p\n", mbview_pick_notify);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* set the pick notify function */
	if (picktype == MBV_PICK_ONEPOINT) {
		data->mbview_pickonepoint_notify = mbview_pick_notify;
	}
	else if (picktype == MBV_PICK_TWOPOINT) {
		data->mbview_picktwopoint_notify = mbview_pick_notify;
	}
	else if (picktype == MBV_PICK_AREA) {
		data->mbview_pickarea_notify = mbview_pick_notify;
	}
	else if (picktype == MBV_PICK_REGION) {
		data->mbview_pickregion_notify = mbview_pick_notify;
	}
	else if (picktype == MBV_PICK_SITE) {
		data->mbview_picksite_notify = mbview_pick_notify;
	}
	else if (picktype == MBV_PICK_ROUTE) {
		data->mbview_pickroute_notify = mbview_pick_notify;
	}
	else if (picktype == MBV_PICK_NAV) {
		data->mbview_picknav_notify = mbview_pick_notify;
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:        %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_setsensitivitynotify(int verbose, size_t instance, void(mbview_sensitivity_notify)(), int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       mbview_sensitivity_notify: %p\n", mbview_sensitivity_notify);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* set the pick notify function */
	data->mbview_sensitivity_notify = mbview_sensitivity_notify;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:        %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
int mbview_setcolorchangenotify(int verbose, size_t instance, void(mbview_colorchange_notify)(size_t), int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       mbview_colorchange_notify: %p\n", mbview_colorchange_notify);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* set the pick notify function */
	data->mbview_colorchange_notify = mbview_colorchange_notify;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:        %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
void mbview_resize(Widget w, XtPointer client_data, XEvent *event, Boolean *unused) {
	(void)w;  // Unused parameter
	(void)unused;  // Unused parameter

	XConfigureEvent *cevent = (XConfigureEvent *)event;

	/* do this only if a resize event happens */
	if (cevent->type == ConfigureNotify) {
		const size_t instance = (size_t)client_data;
		struct mbview_world_struct *view = &(mbviews[instance]);
		struct mbview_struct *data = &(view->data);

		Dimension width;
		Dimension height;
		/* get new shell size */
		XtVaGetValues(view->topLevelShell, XmNwidth, &width, XmNheight, &height, NULL);

		/* do this only if the shell was REALLY resized and not just moved */
		if (data->width != width - LEFT_WIDTH || data->height != height - LEFT_HEIGHT) {
			/*fprintf(stderr,"mbview_resize: %d %d instance: %d\n",
			width, height, instance);*/

			/* set drawing area size */
			data->width = width - LEFT_WIDTH;
			data->height = height - LEFT_HEIGHT;
			Arg args[256];
      Cardinal ac = 0;
			XtSetArg(args[ac], XmNwidth, data->width);
			ac++;
			XtSetArg(args[ac], XmNheight, data->height);
			ac++;
			XtSetValues(view->mb3dview.mbview_drawingArea_mbview, args, ac);
			/*fprintf(stderr,"mbviews[%d].mb3dview.mbview_drawingArea_mbview: %d %d\n",
			instance, data->width, data->height);*/

			/* set glwda size */
			ac = 0;
			XtSetArg(args[ac], XmNwidth, data->width);
			ac++;
			XtSetArg(args[ac], XmNheight, data->height);
			ac++;
			XtSetValues(view->glwmda, args, ac);
			/*fprintf(stderr,"mbviews[%d].glwmda: %d %d\n\n",
			instance, data->width, data->height);*/

			/* update the gl drawing context */
			mbview_reset_glx(instance);

			/* reset if mouse radiobox controls are visible or not */
			if (data->height > MBV_WINDOW_HEIGHT_THRESHOLD) {
				XtManageChild(view->mb3dview.mbview_radioBox_mouse);
			}
			else {
				XtUnmanageChild(view->mb3dview.mbview_radioBox_mouse);
			}

			/* draw */
			if (mbv_verbose >= 2)
				fprintf(stderr, "Calling mbview_plotlowhigh from mbview_resize\n");
			mbview_plotlowhigh(instance);
		}
	}
}
/*------------------------------------------------------------------------------*/
void do_mbview_projection_popup(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	fprintf(stderr, "do_mbview_projection_popup: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	XtManageChild(view->mb3dview.mbview_bulletinBoard_projection);

	/* set values of widgets */
	if (data->display_projection_mode == MBV_PROJECTION_GEOGRAPHIC) {
		XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_geographic, TRUE, FALSE);
		XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_utm, FALSE, FALSE);
		XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_spheroid, FALSE, FALSE);
	}
	else if (data->display_projection_mode == MBV_PROJECTION_PROJECTED ||
	         data->display_projection_mode == MBV_PROJECTION_ALREADYPROJECTED) {
		XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_geographic, FALSE, FALSE);
		XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_utm, TRUE, FALSE);
		XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_spheroid, FALSE, FALSE);
	}
	else if (data->display_projection_mode == MBV_PROJECTION_SPHEROID) {
		XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_spheroid, TRUE, FALSE);
		XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_utm, FALSE, FALSE);
		XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_spheroid, FALSE, FALSE);
	}
	if (shared.lonlatstyle == MBV_LONLAT_DEGREESMINUTES) {
		XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_annotation_degreesminutes, TRUE, FALSE);
		XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_annotation_degreesdecimal, FALSE, FALSE);
	}
	else {
		XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_annotation_degreesminutes, FALSE, FALSE);
		XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_annotation_degreesdecimal, TRUE, FALSE);
	}

	/* set label */
	do_mbview_set_projection_label(instance);
}

/*------------------------------------------------------------------------------*/
void do_mbview_set_projection_label(size_t instance) {
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* set projection label */
  mb_path value_text;
	sprintf(value_text, ":::t\"Primary Grid Projection:\"");
	if (data->primary_grid_projection_mode == MBV_PROJECTION_GEOGRAPHIC) {
		strcat(value_text, ":t\"  Geographic\"");
	}
	else if (data->primary_grid_projection_mode == MBV_PROJECTION_PROJECTED ||
	         data->primary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED) {
		int projectionid;
		char tmptext[MB_PATH_MAXLINE*2];
		if (sscanf(data->primary_grid_projection_id, "epsg%d", &projectionid) == 1 && projectionid == 32661) {
			sprintf(tmptext, ":t\"  Projected: %s\":t\"    North Polar Steographic\"", data->secondary_grid_projection_id);
			strcat(value_text, tmptext);
		}
		else if (sscanf(data->primary_grid_projection_id, "EPSG:%d", &projectionid) == 1 && projectionid == 32661) {
			sprintf(tmptext, ":t\"  Projected: %s\":t\"    North Polar Steographic\"", data->secondary_grid_projection_id);
			strcat(value_text, tmptext);
		}
		else if (sscanf(data->primary_grid_projection_id, "epsg%d", &projectionid) == 1 && projectionid == 32761) {
			sprintf(tmptext, ":t\"  Projected: %s\":t\"    South Polar Steographic\"", data->secondary_grid_projection_id);
			strcat(value_text, tmptext);
		}
		else if (sscanf(data->primary_grid_projection_id, "EPSG:%d", &projectionid) == 1 && projectionid == 32761) {
			sprintf(tmptext, ":t\"  Projected: %s\":t\"    South Polar Steographic\"", data->secondary_grid_projection_id);
			strcat(value_text, tmptext);
		}
		else if (sscanf(data->primary_grid_projection_id, "epsg%d", &projectionid) == 1 && projectionid >= 32600 &&
		         projectionid < 32700) {
			sprintf(tmptext, ":t\"  Projected: %s\":t\"    UTM Zone %d N\"", data->primary_grid_projection_id,
			        projectionid - 32600);
			strcat(value_text, tmptext);
		}
		else if (sscanf(data->primary_grid_projection_id, "EPSG:%d", &projectionid) == 1 && projectionid >= 32600 &&
		         projectionid < 32700) {
			sprintf(tmptext, ":t\"  Projected: %s\":t\"    UTM Zone %d N\"", data->primary_grid_projection_id,
			        projectionid - 32600);
			strcat(value_text, tmptext);
		}
		else if (sscanf(data->primary_grid_projection_id, "epsg%d", &projectionid) == 1 && projectionid >= 32700 &&
		         projectionid < 32800) {
			sprintf(tmptext, ":t\"  Projected: %s\":t\"    UTM Zone %d S\"", data->primary_grid_projection_id,
			        projectionid - 32700);
			strcat(value_text, tmptext);
		}
		else if (sscanf(data->primary_grid_projection_id, "EPSG:%d", &projectionid) == 1 && projectionid >= 32700 &&
		         projectionid < 32800) {
			sprintf(tmptext, ":t\"  Projected: %s\":t\"    UTM Zone %d S\"", data->primary_grid_projection_id,
			        projectionid - 32700);
			strcat(value_text, tmptext);
		}
		else {
			sprintf(tmptext, ":t\"  Projected: %s\"", data->primary_grid_projection_id);
			strcat(value_text, tmptext);
		}
	}
	else if (data->primary_grid_projection_mode == MBV_PROJECTION_SPHEROID) {
		strcat(value_text, ":t\"  Spheroid\"");
	}

	if (data->secondary_nxy > 0) {
		strcat(value_text, ":t\"Secondary Grid Projection:\"");
		if (data->secondary_grid_projection_mode == MBV_PROJECTION_GEOGRAPHIC) {
			strcat(value_text, ":t\"  Geographic\"");
		}
		else if (data->secondary_grid_projection_mode == MBV_PROJECTION_PROJECTED ||
		         data->secondary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED) {
			int projectionid;
			char tmptext[MB_PATH_MAXLINE*2];
			if (sscanf(data->secondary_grid_projection_id, "epsg%d", &projectionid) == 1 && projectionid == 32661) {
				sprintf(tmptext, ":t\"  Projected: %s\":t\"    North Polar Steographic\"", data->secondary_grid_projection_id);
				strcat(value_text, tmptext);
			}
			else if (sscanf(data->secondary_grid_projection_id, "EPSG:%d", &projectionid) == 1 && projectionid == 32661) {
				sprintf(tmptext, ":t\"  Projected: %s\":t\"    North Polar Steographic\"", data->secondary_grid_projection_id);
				strcat(value_text, tmptext);
			}
			else if (sscanf(data->secondary_grid_projection_id, "epsg%d", &projectionid) == 1 && projectionid == 32761) {
				sprintf(tmptext, ":t\"  Projected: %s\":t\"    South Polar Steographic\"", data->secondary_grid_projection_id);
				strcat(value_text, tmptext);
			}
			else if (sscanf(data->secondary_grid_projection_id, "EPSG:%d", &projectionid) == 1 && projectionid == 32761) {
				sprintf(tmptext, ":t\"  Projected: %s\":t\"    South Polar Steographic\"", data->secondary_grid_projection_id);
				strcat(value_text, tmptext);
			}
			else if (sscanf(data->secondary_grid_projection_id, "epsg%d", &projectionid) == 1 && projectionid >= 32600 &&
			         projectionid < 32700) {
				sprintf(tmptext, ":t\"  Projected: %s\":t\"    UTM Zone %d N\"", data->secondary_grid_projection_id,
				        projectionid - 32600);
				strcat(value_text, tmptext);
			}
			else if (sscanf(data->secondary_grid_projection_id, "EPSG:%d", &projectionid) == 1 && projectionid >= 32600 &&
			         projectionid < 32700) {
				sprintf(tmptext, ":t\"  Projected: %s\":t\"    UTM Zone %d N\"", data->secondary_grid_projection_id,
				        projectionid - 32600);
				strcat(value_text, tmptext);
			}
			else if (sscanf(data->secondary_grid_projection_id, "epsg%d", &projectionid) == 1 && projectionid >= 32700 &&
			         projectionid < 32800) {
				sprintf(tmptext, ":t\"  Projected: %s\":t\"    UTM Zone %d S\"", data->secondary_grid_projection_id,
				        projectionid - 32700);
				strcat(value_text, tmptext);
			}
			else if (sscanf(data->secondary_grid_projection_id, "EPSG:%d", &projectionid) == 1 && projectionid >= 32700 &&
			         projectionid < 32800) {
				sprintf(tmptext, ":t\"  Projected: %s\":t\"    UTM Zone %d S\"", data->secondary_grid_projection_id,
				        projectionid - 32700);
				strcat(value_text, tmptext);
			}
			else {
				sprintf(tmptext, ":t\"  Projected: %s\"", data->secondary_grid_projection_id);
				strcat(value_text, tmptext);
			}
		}
		else if (data->secondary_grid_projection_mode == MBV_PROJECTION_SPHEROID) {
			strcat(value_text, ":t\"  Spheroid\"");
		}
	}

	strcat(value_text, ":t\"Display Grid Projection:\"");
	if (data->display_projection_mode == MBV_PROJECTION_GEOGRAPHIC) {
		strcat(value_text, ":t\"  Geographic\"");
	}
	else if (data->display_projection_mode == MBV_PROJECTION_PROJECTED ||
	         data->display_projection_mode == MBV_PROJECTION_ALREADYPROJECTED) {
		int projectionid;
		char tmptext[MB_PATH_MAXLINE*2];
		if (sscanf(data->display_projection_id, "epsg%d", &projectionid) == 1 && projectionid == 32661) {
			sprintf(tmptext, ":t\"  Projected: %s\":t\"    North Polar Steographic\"", data->secondary_grid_projection_id);
			strcat(value_text, tmptext);
		}
		else if (sscanf(data->display_projection_id, "EPSG:%d", &projectionid) == 1 && projectionid == 32661) {
			sprintf(tmptext, ":t\"  Projected: %s\":t\"    North Polar Steographic\"", data->secondary_grid_projection_id);
			strcat(value_text, tmptext);
		}
		else if (sscanf(data->display_projection_id, "epsg%d", &projectionid) == 1 && projectionid == 32761) {
			sprintf(tmptext, ":t\"  Projected: %s\":t\"    South Polar Steographic\"", data->secondary_grid_projection_id);
			strcat(value_text, tmptext);
		}
		else if (sscanf(data->display_projection_id, "EPSG:%d", &projectionid) == 1 && projectionid == 32761) {
			sprintf(tmptext, ":t\"  Projected: %s\":t\"    South Polar Steographic\"", data->secondary_grid_projection_id);
			strcat(value_text, tmptext);
		}
		else if (sscanf(data->display_projection_id, "epsg%d", &projectionid) == 1 && projectionid >= 32600 &&
		         projectionid < 32700) {
			sprintf(tmptext, ":t\"  Projected: %s\":t\"    UTM Zone %d N", data->display_projection_id, projectionid - 32600);
			strcat(value_text, tmptext);
		}
		else if (sscanf(data->display_projection_id, "EPSG:%d", &projectionid) == 1 && projectionid >= 32600 &&
		         projectionid < 32700) {
			sprintf(tmptext, ":t\"  Projected: %s\":t\"    UTM Zone %d N", data->display_projection_id, projectionid - 32600);
			strcat(value_text, tmptext);
		}
		else if (sscanf(data->display_projection_id, "epsg%d", &projectionid) == 1 && projectionid >= 32700 &&
		         projectionid < 32800) {
			sprintf(tmptext, ":t\"  Projected: %s\":t\"    UTM Zone %d S\"", data->display_projection_id, projectionid - 32700);
			strcat(value_text, tmptext);
		}
		else if (sscanf(data->display_projection_id, "EPSG:%d", &projectionid) == 1 && projectionid >= 32700 &&
		         projectionid < 32800) {
			sprintf(tmptext, ":t\"  Projected: %s\":t\"    UTM Zone %d S\"", data->display_projection_id, projectionid - 32700);
			strcat(value_text, tmptext);
		}
		else {
			sprintf(tmptext, ":t\"  Projected: %s\"", data->display_projection_id);
			strcat(value_text, tmptext);
		}
	}
	else if (data->display_projection_mode == MBV_PROJECTION_SPHEROID) {
		strcat(value_text, ":t\"  Spheroid\"");
	}
	set_mbview_label_multiline_string(view->mb3dview.mbview_label_projection, value_text);
}

/*------------------------------------------------------------------------------*/
void do_mbview_projection_popdown(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	if (mbv_verbose >= 0)
		fprintf(stderr, "do_mbview_projection_popdown: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);

	XtUnmanageChild(view->mb3dview.mbview_bulletinBoard_projection);
}

/*------------------------------------------------------------------------------*/
void do_mbview_display_spheroid(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	fprintf(stderr, "do_mbview_display_spheroid: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* reproject as spheroid if the togglebutton has been set */
	if (XmToggleButtonGetState(view->mb3dview.mbview_toggleButton_spheroid)) {
		data->display_projection_mode = MBV_PROJECTION_SPHEROID;
		view->plot_done = false;
		view->projected = false;
		view->globalprojected = false;
		view->viewboundscount = MBV_BOUNDSFREQUENCY;

		/* set label */
		do_mbview_set_projection_label(instance);

		/* clear zscale for grid */
		mbview_zscaleclear(instance);

		/* project and scale data other than the grid */
		mbview_zscale(instance);

		/* draw */
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_display_spheroid\n");
		mbview_plotlowhigh(instance);
	}
}
/*------------------------------------------------------------------------------*/

void do_mbview_display_geographic(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	fprintf(stderr, "do_mbview_display_geographic: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* reproject as geographic if the togglebutton has been set */
	if (XmToggleButtonGetState(view->mb3dview.mbview_toggleButton_geographic)) {
		data->display_projection_mode = MBV_PROJECTION_GEOGRAPHIC;
		view->plot_done = false;
		view->projected = false;
		view->globalprojected = false;
		view->viewboundscount = MBV_BOUNDSFREQUENCY;

		/* set label */
		do_mbview_set_projection_label(instance);

		/* clear zscale for grid */
		mbview_zscaleclear(instance);

		/* project and scale data other than the grid */
		mbview_zscale(instance);

		/* draw */
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_display_geographic\n");
		mbview_plotlowhigh(instance);
	}
}

/*------------------------------------------------------------------------------*/
void do_mbview_display_utm(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
	size_t instance;
	int projectionid, utmzone;
	double reference_lon;
	double reference_lat;

	Arg args[256];
  Cardinal ac = 0;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	fprintf(stderr, "do_mbview_display_utm: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* reproject as utm if the togglebutton has been set */
	if (XmToggleButtonGetState(view->mb3dview.mbview_toggleButton_utm)) {
		data->display_projection_mode = MBV_PROJECTION_PROJECTED;
		view->plot_done = false;
		view->projected = false;
		view->globalprojected = false;
		view->viewboundscount = MBV_BOUNDSFREQUENCY;

		mbview_projectgrid2ll(instance, 0.5 * (data->primary_xmin + data->primary_xmax),
		                      0.5 * (data->primary_ymin + data->primary_ymax), &reference_lon, &reference_lat);
		if (reference_lon > 180.0)
			reference_lon -= 360.0;
		utmzone = (int)(((reference_lon + 183.0) / 6.0) + 0.5);
		if (reference_lat >= 0.0)
			projectionid = 32600 + utmzone;
		else
			projectionid = 32700 + utmzone;
		sprintf(data->display_projection_id, "EPSG:%d", projectionid);

		/* set label */
		do_mbview_set_projection_label(instance);

		/* clear zscale for grid */
		mbview_zscaleclear(instance);

		/* project and scale data other than the grid */
		mbview_zscale(instance);

		/* draw */
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_display_utm\n");
		mbview_plotlowhigh(instance);
	}
}

/*------------------------------------------------------------------------------*/
void do_mbview_annotation_degreesminutes(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
	size_t instance;

	Arg args[256];
  Cardinal ac = 0;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	fprintf(stderr, "do_mbview_annotation_degreesminutes: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);

	/* use degrees + minutes for pick annotation */
	if (XmToggleButtonGetState(view->mb3dview.mbview_toggleButton_annotation_degreesminutes)) {
		shared.lonlatstyle = MBV_LONLAT_DEGREESMINUTES;
		mbview_pick_text(instance);
	}
}

/*------------------------------------------------------------------------------*/
void do_mbview_annotation_degreesdecimal(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
	size_t instance;

	Arg args[256];
  Cardinal ac = 0;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	fprintf(stderr, "do_mbview_annotation_degreesdecimal: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);

	/* use decimal degrees for pick annotation */
	if (XmToggleButtonGetState(view->mb3dview.mbview_toggleButton_annotation_degreesdecimal)) {
		shared.lonlatstyle = MBV_LONLAT_DEGREESDECIMAL;
		mbview_pick_text(instance);
	}
}

/*------------------------------------------------------------------------------*/

void do_mbview_glwda_expose(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
	// mbGLwDrawingAreaCallbackStruct *acs = (mbGLwDrawingAreaCallbackStruct *)call_data;
	size_t instance;

	Arg args[256];
  Cardinal ac = 0;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_glwda_expose\n");
	mbview_plotlowhigh(instance);
}
/*------------------------------------------------------------------------------*/

void do_mbview_glwda_resize(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
	// mbGLwDrawingAreaCallbackStruct *acs = (mbGLwDrawingAreaCallbackStruct *)call_data;
	size_t instance;

	Arg args[256];
  Cardinal ac = 0;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);
}
/*------------------------------------------------------------------------------*/

void do_mbview_glwda_input(Widget w, XtPointer client_data, XtPointer call_data) {
	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	bool replotall = false;
	bool replotprofile = false;

	/* get event */
	mbGLwDrawingAreaCallbackStruct *acs = (mbGLwDrawingAreaCallbackStruct *)call_data;
	XEvent *event = acs->event;

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_glwda_input: %d %d  instance:%zu type:%d\n", acs->width, acs->height, instance,
		        event->xany.type);


	/* If there is input in the drawing area and the drawing area is still initialized */
	if (acs->reason == XmCR_INPUT && view->init != MBV_WINDOW_NULL) {

		/* Check for mouse pressed. */
		if (event->xany.type == ButtonPress) {
			/*fprintf(stderr, "event->xany.type == ButtonPress  %d %d  mouse mode:%d\n",
			event->xbutton.x,event->xbutton.y, data->mouse_mode);*/
			/* save location */
			view->button_down_x = event->xbutton.x;
			view->button_down_y = event->xbutton.y;

			/* If left mouse button is pushed */
			if (event->xbutton.button == 1) {
				/* set button1down flag */
				view->button1down = true;

				/* handle move */
				if (data->mouse_mode == MBV_MOUSE_MOVE || data->mouse_mode == MBV_MOUSE_ROTATE ||
				    data->mouse_mode == MBV_MOUSE_SHADE || data->mouse_mode == MBV_MOUSE_VIEWPOINT ||
				    data->mouse_mode == MBV_MOUSE_NAV || data->mouse_mode == MBV_MOUSE_NAVFILE) {
					/* set cursor for pick */
					XDefineCursor(view->dpy, view->xid, view->TargetRedCursor);

					/* process pick */
					mbview_pick(instance, MBV_PICK_DOWN, view->button_down_x, data->height - view->button_down_y);

					/* set replotprofile */
					replotprofile = true;

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle region picking */
				else if (data->mouse_mode == MBV_MOUSE_AREA) {
					/* set cursor for area */
					XDefineCursor(view->dpy, view->xid, view->TargetRedCursor);

					/* process area */
					mbview_region(instance, MBV_REGION_DOWN, view->button_down_x, data->height - view->button_down_y);

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle editing sites */
				else if (data->mouse_mode == MBV_MOUSE_SITE) {
					/* set cursor for site */
					XDefineCursor(view->dpy, view->xid, view->TargetRedCursor);

					/* process site select */
					mbview_pick_site_select(instance, MBV_PICK_DOWN, view->button_down_x, data->height - view->button_down_y);

					/* set replotall */
					replotall = true;

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle editing routes */
				else if (data->mouse_mode == MBV_MOUSE_ROUTE) {
					/* set cursor for route */
					XDefineCursor(view->dpy, view->xid, view->TargetRedCursor);

					/* process route select */
					mbview_pick_route_select(mbv_verbose, instance, MBV_PICK_DOWN, view->button_down_x,
					                         data->height - view->button_down_y);

					/* set replotall */
					replotall = true;

					/* set replotprofile */
					replotprofile = true;

					/* replot */
					mbview_plotlow(instance);
				}

			} /* end of left button events */

			/* If middle mouse button is pushed */
			else if (event->xbutton.button == 2) {
				/* set button2down flag */
				view->button2down = true;

				/* handle move */
				if (data->mouse_mode == MBV_MOUSE_MOVE) {
					/* set cursor for move */
					XDefineCursor(view->dpy, view->xid, view->FleurBlackCursor);

					/* save starting position offsets */
					if (data->display_mode == MBV_DISPLAY_2D) {
						view->offset2d_x_save = view->offset2d_x;
						view->offset2d_y_save = view->offset2d_y;
					}
					else {
						view->offset3d_x_save = view->offset3d_x;
						view->offset3d_y_save = view->offset3d_y;
					}
				}

				/* handle 3D rotate */
				else if (data->mouse_mode == MBV_MOUSE_ROTATE) {
					/* set cursor for rotate */
					XDefineCursor(view->dpy, view->xid, view->FleurBlackCursor);

					view->modelazimuth3d_save = data->modelazimuth3d;
					view->modelelevation3d_save = data->modelelevation3d;
				}

				/* handle shading */
				else if (data->mouse_mode == MBV_MOUSE_SHADE) {
					/* get shade mode */
					int shade_mode;
					if (data->grid_mode == MBV_GRID_VIEW_PRIMARY)
						shade_mode = data->primary_shade_mode;
					else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE)
						shade_mode = data->slope_shade_mode;
					else /* if (data->grid_mode == MBV_GRID_VIEW_SECONDARY) */
						shade_mode = data->secondary_shade_mode;

					/* handle shading by illumination */
					if (shade_mode == MBV_SHADE_VIEW_ILLUMINATION) {
						/* set cursor for shade */
						XDefineCursor(view->dpy, view->xid, view->FleurBlackCursor);

						view->illuminate_azimuth_save = data->illuminate_azimuth;
						view->illuminate_elevation_save = data->illuminate_elevation;
					}

					/* handle shading by overlay */
					else if (shade_mode == MBV_SHADE_VIEW_SLOPE) {
						/* set cursor for shade */
						XDefineCursor(view->dpy, view->xid, view->FleurBlackCursor);
					}

					/* handle shading by overlay */
					else if (shade_mode == MBV_SHADE_VIEW_OVERLAY) {
						/* set cursor for shade */
						XDefineCursor(view->dpy, view->xid, view->FleurBlackCursor);
					}
				}

				/* handle viewpoint rotation */
				else if (data->mouse_mode == MBV_MOUSE_VIEWPOINT) {
					/* set cursor for rotate */
					XDefineCursor(view->dpy, view->xid, view->FleurBlackCursor);

					view->viewazimuth3d_save = data->viewazimuth3d;
					view->viewelevation3d_save = data->viewelevation3d;
				}

				/* handle area picking */
				else if (data->mouse_mode == MBV_MOUSE_AREA) {
					/* set cursor for area */
					XDefineCursor(view->dpy, view->xid, view->TargetRedCursor);

					/* process area */
					mbview_area(instance, MBV_AREALENGTH_DOWN, view->button_down_x, data->height - view->button_down_y);

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle editing sites */
				else if (data->mouse_mode == MBV_MOUSE_SITE) {
					/* set cursor for site */
					XDefineCursor(view->dpy, view->xid, view->TargetRedCursor);

					/* process site select */
					mbview_pick_site_add(instance, MBV_PICK_DOWN, view->button_down_x, data->height - view->button_down_y);

					/* set replotall */
					replotall = true;

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle editing routes */
				else if (data->mouse_mode == MBV_MOUSE_ROUTE) {
					/* set cursor for route */
					XDefineCursor(view->dpy, view->xid, view->TargetRedCursor);

					/* process route select */
					mbview_pick_route_add(mbv_verbose, instance, MBV_PICK_DOWN, view->button_down_x,
					                      data->height - view->button_down_y);

					/* set replotall */
					replotall = true;

					/* set replotprofile */
					replotprofile = true;

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle selecting navigation */
				else if (data->mouse_mode == MBV_MOUSE_NAV || data->mouse_mode == MBV_MOUSE_NAVFILE) {
					/* set cursor for nav */
					XDefineCursor(view->dpy, view->xid, view->TargetRedCursor);

					/* process nav select */
					mbview_pick_nav_select(instance, true, MBV_PICK_DOWN, view->button_down_x,
					                       data->height - view->button_down_y);

					/* set replotall */
					replotall = true;

					/* replot */
					mbview_plotlow(instance);
				}

			} /* end of middle button events */

			/* If right mouse button is pushed */
			else if (event->xbutton.button == 3) {
				/* set button3down flag */
				view->button3down = true;

				/* change the map scaling */
				if (data->mouse_mode == MBV_MOUSE_MOVE) {
					/* set cursor for scaling */
					XDefineCursor(view->dpy, view->xid, view->SizingBlackCursor);

					/* save starting size */
					if (data->display_mode == MBV_DISPLAY_2D) {
						view->size2d_save = view->size2d;
					}
					else {
						view->offset3d_z_save = view->offset3d_z;
					}
				}

				/* handle vertical exaggerate */
				else if (data->mouse_mode == MBV_MOUSE_ROTATE) {
					/* set cursor for exaggerate */
					XDefineCursor(view->dpy, view->xid, view->FleurBlackCursor);

					view->exageration_save = data->exageration;
				}

				/* handle shading */
				else if (data->mouse_mode == MBV_MOUSE_SHADE) {
					/* get shade mode */
					int shade_mode;
					if (data->grid_mode == MBV_GRID_VIEW_PRIMARY)
						shade_mode = data->primary_shade_mode;
					else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE)
						shade_mode = data->slope_shade_mode;
					else /* if (data->grid_mode == MBV_GRID_VIEW_SECONDARY) */
						shade_mode = data->secondary_shade_mode;

					/* handle shading by illumination */
					if (shade_mode == MBV_SHADE_VIEW_ILLUMINATION) {
						/* set cursor for shading */
						XDefineCursor(view->dpy, view->xid, view->FleurBlackCursor);

						view->illuminate_magnitude_save = data->illuminate_magnitude;
					}

					/* handle shading by slope */
					else if (shade_mode == MBV_SHADE_VIEW_SLOPE) {
						/* set cursor for shading */
						XDefineCursor(view->dpy, view->xid, view->FleurBlackCursor);

						view->slope_magnitude_save = data->slope_magnitude;
					}

					/* handle shading by overlay */
					else if (shade_mode == MBV_SHADE_VIEW_OVERLAY) {
						/* set cursor for shading */
						XDefineCursor(view->dpy, view->xid, view->FleurBlackCursor);

						view->overlay_shade_magnitude_save = data->overlay_shade_magnitude;
					}
				}

				/* handle viewpoint rotation */
				else if (data->mouse_mode == MBV_MOUSE_VIEWPOINT) {
					/* set cursor for scaling */
					XDefineCursor(view->dpy, view->xid, view->SizingBlackCursor);

					/* save starting size */
					if (data->display_mode == MBV_DISPLAY_2D) {
						view->size2d_save = view->size2d;
					}
					else {
						view->viewoffset3d_z_save = view->viewoffset3d_z;
					}
				}

				/* handle area picking */
				else if (data->mouse_mode == MBV_MOUSE_AREA) {
					/* set cursor for area width */
					XDefineCursor(view->dpy, view->xid, view->SizingBlackCursor);

					/* process area */
					view->areaaspect_save = view->areaaspect;
				}

				/* handle editing sites */
				else if (data->mouse_mode == MBV_MOUSE_SITE) {
					/* set cursor for area */
					XDefineCursor(view->dpy, view->xid, view->TargetRedCursor);

					/* process site select */
					mbview_pick_site_delete(instance, view->button_down_x, data->height - view->button_down_y);

					/* set replotall */
					replotall = true;

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle editing routes */
				else if (data->mouse_mode == MBV_MOUSE_ROUTE) {
					/* set cursor for area */
					XDefineCursor(view->dpy, view->xid, view->TargetRedCursor);

					/* process route select */
					mbview_pick_route_delete(mbv_verbose, instance, view->button_down_x, data->height - view->button_down_y);

					/* set replotall */
					replotall = true;

					/* set replotprofile */
					replotprofile = true;

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle deselecting navigation */
				else if (data->mouse_mode == MBV_MOUSE_NAV || data->mouse_mode == MBV_MOUSE_NAVFILE) {
					/* set cursor for area */
					XDefineCursor(view->dpy, view->xid, view->TargetRedCursor);

					/* process nav deselect */
					mbview_pick_nav_select(instance, false, MBV_PICK_DOWN, view->button_down_x,
					                       data->height - view->button_down_y);

					/* set replotall */
					replotall = true;

					/* replot */
					mbview_plotlow(instance);
				}

			} /* end of right button events */

			/* if needed replot all active instances */
			if (replotall) {
				mbview_plotlowall(instance);
			}

		} /* end of button press events */

		/* Check for mouse motion while pressed. */
		if (event->xany.type == MotionNotify) {
			/*fprintf(stderr, "event->xany.type == MotionNotify  %d %d  mouse mode:%d\n",
			event->xbutton.x,event->xbutton.y, data->mouse_mode);*/

			/* prohibit event interruption of plotting */
			view->plot_interrupt_allowed = false;

			/* save location */
			view->button_move_x = event->xmotion.x;
			view->button_move_y = event->xmotion.y;

			/* If left mouse button is dragged */
			if (view->button1down) {

				/* set cursor for drag */
				XDefineCursor(view->dpy, view->xid, view->FleurRedCursor);

				/* handle move */
				if (data->mouse_mode == MBV_MOUSE_MOVE || data->mouse_mode == MBV_MOUSE_ROTATE ||
				    data->mouse_mode == MBV_MOUSE_SHADE || data->mouse_mode == MBV_MOUSE_VIEWPOINT ||
				    data->mouse_mode == MBV_MOUSE_NAV || data->mouse_mode == MBV_MOUSE_NAVFILE) {
					/* process pick */
					mbview_pick(instance, MBV_PICK_MOVE, view->button_move_x, data->height - view->button_move_y);

					/* set replotprofile */
					replotprofile = true;

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle region picking */
				else if (data->mouse_mode == MBV_MOUSE_AREA) {
					/* set cursor for region */
					XDefineCursor(view->dpy, view->xid, view->TargetRedCursor);

					/* process region */
					mbview_region(instance, MBV_REGION_MOVE, view->button_move_x, data->height - view->button_move_y);

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle editing sites */
				else if (data->mouse_mode == MBV_MOUSE_SITE) {
					/* set cursor for site */
					XDefineCursor(view->dpy, view->xid, view->TargetRedCursor);

					/* process site select */
					mbview_pick_site_select(instance, MBV_PICK_MOVE, view->button_move_x, data->height - view->button_move_y);

					/* set replotall */
					replotall = true;

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle editing routes */
				else if (data->mouse_mode == MBV_MOUSE_ROUTE) {
					/* set cursor for route */
					XDefineCursor(view->dpy, view->xid, view->TargetRedCursor);

					/* process route select */
					mbview_pick_route_select(mbv_verbose, instance, MBV_PICK_MOVE, view->button_move_x,
					                         data->height - view->button_move_y);

					/* set replotall */
					replotall = true;

					/* set replotprofile */
					replotprofile = true;

					/* replot */
					mbview_plotlow(instance);
				}

			} /* end of left button events */

			/* If middle mouse button is dragged */
			else if (view->button2down) {
				/* handle move */
				if (data->mouse_mode == MBV_MOUSE_MOVE) {
					/* set cursor for move */
					XDefineCursor(view->dpy, view->xid, view->FleurRedCursor);

					/* move map */
					if (data->display_mode == MBV_DISPLAY_2D) {
						view->offset2d_x = view->offset2d_x_save +
						                   (view->button_move_x - view->button_down_x) * (view->right - view->left) / data->width;
						view->offset2d_y = view->offset2d_y_save - (view->button_move_y - view->button_down_y) *
						                                               (view->top - view->bottom) / data->height;
						if (XtIsManaged(view->mb3dview.mbview_textField_view_2doffsetx)) {
              mb_path value_text;
							sprintf(value_text, "%g", view->offset2d_x);
							XmTextFieldSetString(view->mb3dview.mbview_textField_view_2doffsetx, value_text);
							sprintf(value_text, "%g", view->offset2d_y);
							XmTextFieldSetString(view->mb3dview.mbview_textField_view_2doffsety, value_text);
						}
					}
					else {
						view->offset3d_x = view->offset3d_x_save + (view->button_move_x - view->button_down_x) *
						                                               MBV_OPENGL_WIDTH * MBV_OPENGL_WIDTH / data->width;
						view->offset3d_y = view->offset3d_y_save - (view->button_move_y - view->button_down_y) *
						                                               view->aspect_ratio * view->aspect_ratio *
						                                               MBV_OPENGL_WIDTH * MBV_OPENGL_WIDTH / data->height;
						if (XtIsManaged(view->mb3dview.mbview_textField_view_3doffsetx)) {
              mb_path value_text;
							sprintf(value_text, "%g", view->offset3d_x);
							XmTextFieldSetString(view->mb3dview.mbview_textField_view_3doffsetx, value_text);
							sprintf(value_text, "%g", view->offset3d_y);
							XmTextFieldSetString(view->mb3dview.mbview_textField_view_3doffsety, value_text);
						}
					}

					/* set flag to reset view bounds */
					view->viewboundscount++;

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle 3D rotate */
				else if (data->mouse_mode == MBV_MOUSE_ROTATE) {
					/* set cursor for rotate */
					XDefineCursor(view->dpy, view->xid, view->FleurRedCursor);

					/* rotate viewpoint of 3D map */
					data->modelazimuth3d = view->modelazimuth3d_save +
					                       180.0 * ((double)(view->button_move_x - view->button_down_x)) / ((double)data->width);
					data->modelelevation3d =
					    view->modelelevation3d_save +
					    180.0 * ((double)(view->button_move_y - view->button_down_y)) / ((double)data->height);
					if (XtIsManaged(view->mb3dview.mbview_textField_model_azimuth)) {
            mb_path value_text;
						sprintf(value_text, "%g", data->modelazimuth3d);
						XmTextFieldSetString(view->mb3dview.mbview_textField_model_azimuth, value_text);
						sprintf(value_text, "%g", data->modelelevation3d);
						XmTextFieldSetString(view->mb3dview.mbview_textField_model_elevation, value_text);
					}

					/* set flag to reset view bounds */
					view->viewboundscount++;

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle shading */
				else if (data->mouse_mode == MBV_MOUSE_SHADE) {
					/* get shade mode */
					int shade_mode = data->primary_shade_mode;
					if (data->grid_mode == MBV_GRID_VIEW_PRIMARY)
						shade_mode = data->primary_shade_mode;
					else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE)
						shade_mode = data->slope_shade_mode;
					else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY)
						shade_mode = data->secondary_shade_mode;

					/* handle shading by illumination */
					if (shade_mode == MBV_SHADE_VIEW_ILLUMINATION) {
						/* set cursor for shade */
						XDefineCursor(view->dpy, view->xid, view->FleurBlackCursor);

						/* rotate illumination */
						data->illuminate_azimuth =
						    view->illuminate_azimuth_save +
						    180.0 * ((double)(view->button_move_x - view->button_down_x)) / ((double)data->width);
						data->illuminate_elevation =
						    view->illuminate_elevation_save +
						    180.0 * ((double)(view->button_move_y - view->button_down_y)) / ((double)data->height);
						if (XtIsManaged(view->mb3dview.mbview_textField_illum_azi)) {
              mb_path value_text;
							sprintf(value_text, "%g", data->illuminate_azimuth);
							XmTextFieldSetString(view->mb3dview.mbview_textField_illum_azi, value_text);
							sprintf(value_text, "%g", data->illuminate_elevation);
							XmTextFieldSetString(view->mb3dview.mbview_textField_illum_elev, value_text);
						}

						/* clear color status array */
						mbview_setcolorparms(instance);
						mbview_colorclear(instance);

						/* replot */
						mbview_plotlow(instance);
					}
				}

				/* handle viewpoint rotation */
				else if (data->mouse_mode == MBV_MOUSE_VIEWPOINT) {
					/* set cursor for rotate */
					XDefineCursor(view->dpy, view->xid, view->FleurRedCursor);

					/* rotate viewpoint of 3D map */
					data->viewazimuth3d = view->viewazimuth3d_save +
					                      180.0 * ((double)(view->button_move_x - view->button_down_x)) / ((double)data->width);
					data->viewelevation3d =
					    view->viewelevation3d_save +
					    180.0 * ((double)(view->button_move_y - view->button_down_y)) / ((double)data->height);
					if (XtIsManaged(view->mb3dview.mbview_textField_view_azimuth)) {
            mb_path value_text;
						sprintf(value_text, "%g", data->viewazimuth3d);
						XmTextFieldSetString(view->mb3dview.mbview_textField_view_azimuth, value_text);
						sprintf(value_text, "%g", data->viewelevation3d);
						XmTextFieldSetString(view->mb3dview.mbview_textField_view_elevation, value_text);
					}

					/* set flag to reset view bounds */
					view->viewboundscount++;

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle area picking */
				else if (data->mouse_mode == MBV_MOUSE_AREA) {
					/* set cursor for area */
					XDefineCursor(view->dpy, view->xid, view->TargetRedCursor);

					/* process area */
					mbview_area(instance, MBV_AREALENGTH_MOVE, view->button_move_x, data->height - view->button_move_y);

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle editing sites */
				else if (data->mouse_mode == MBV_MOUSE_SITE) {
					/* set cursor for site */
					XDefineCursor(view->dpy, view->xid, view->TargetRedCursor);

					/* process site select */
					mbview_pick_site_add(instance, MBV_PICK_MOVE, view->button_move_x, data->height - view->button_move_y);

					/* set replotall */
					replotall = true;

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle editing routes */
				else if (data->mouse_mode == MBV_MOUSE_ROUTE) {
					/* set cursor for route */
					XDefineCursor(view->dpy, view->xid, view->TargetRedCursor);

					/* process route select */
					mbview_pick_route_add(mbv_verbose, instance, MBV_PICK_MOVE, view->button_move_x,
					                      data->height - view->button_move_y);

					/* set replotall */
					replotall = true;

					/* set replotprofile */
					replotprofile = true;

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle selecting navigation */
				else if (data->mouse_mode == MBV_MOUSE_NAV || data->mouse_mode == MBV_MOUSE_NAVFILE) {
					/* set cursor for nav */
					XDefineCursor(view->dpy, view->xid, view->TargetRedCursor);

					/* process nav select */
					mbview_pick_nav_select(instance, true, MBV_PICK_MOVE, view->button_move_x,
					                       data->height - view->button_move_y);

					/* set replotall */
					replotall = true;

					/* replot */
					mbview_plotlow(instance);
				}

			} /* end of middle button events */

			/* If right mouse button is dragged */
			else if (view->button3down) {

				/* change the map scaling */
				if (data->mouse_mode == MBV_MOUSE_MOVE) {
					/* set cursor for scaling */
					XDefineCursor(view->dpy, view->xid, view->SizingBlackCursor);

					/* rescale 2D map */
					if (data->display_mode == MBV_DISPLAY_2D) {
						view->size2d = view->size2d_save *
						               exp(((double)(view->button_down_y - view->button_move_y)) / ((double)data->height));
						if (XtIsManaged(view->mb3dview.mbview_textField_view_2dzoom)) {
              mb_path value_text;
							sprintf(value_text, "%g", view->size2d);
							XmTextFieldSetString(view->mb3dview.mbview_textField_view_2dzoom, value_text);
						}
					}

					/* rescale 3D map */
					else {
						view->offset3d_z = view->offset3d_z_save +
						                   2.0 * (((double)(view->button_down_y - view->button_move_y)) / ((double)data->height));
						if (XtIsManaged(view->mb3dview.mbview_textField_model_3dzoom)) {
              mb_path value_text;
							sprintf(value_text, "%g", view->offset3d_z);
							XmTextFieldSetString(view->mb3dview.mbview_textField_model_3dzoom, value_text);
						}
					}

					/* set flag to reset view bounds */
					view->viewboundscount++;

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle vertical exaggerate */
				else if (data->mouse_mode == MBV_MOUSE_ROTATE) {
					/* set cursor for exaggerate */
					XDefineCursor(view->dpy, view->xid, view->FleurRedCursor);

					/* change vertical exageration of 3D map */
					data->exageration = view->exageration_save *
					                    exp(((double)(view->button_down_y - view->button_move_y)) / ((double)data->height));
					if (data->display_projection_mode != MBV_PROJECTION_SPHEROID) {
						view->zorigin = data->exageration * 0.5 * (data->primary_min + data->primary_max);
					}
					if (XtIsManaged(view->mb3dview.mbview_textField_exageration)) {
            mb_path value_text;
						sprintf(value_text, "%g", data->exageration);
						XmTextFieldSetString(view->mb3dview.mbview_textField_exageration, value_text);
					}

					/* reset flags */
					mbview_zscaleclear(instance);
					view->contourlorez = false;
					view->contourhirez = false;
					view->contourfullrez = false;

					/* rescale data other than the grid */
					mbview_zscale(instance);

					/* set flag to reset view bounds */
					view->viewboundscount++;

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle shading */
				else if (data->mouse_mode == MBV_MOUSE_SHADE) {
					/* get shade mode */
					int shade_mode = data->primary_shade_mode;
					if (data->grid_mode == MBV_GRID_VIEW_PRIMARY)
						shade_mode = data->primary_shade_mode;
					else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE)
						shade_mode = data->slope_shade_mode;
					else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY)
						shade_mode = data->secondary_shade_mode;

					/* handle shading by illumination */
					if (shade_mode == MBV_SHADE_VIEW_ILLUMINATION) {
						/* set cursor for shading */
						XDefineCursor(view->dpy, view->xid, view->FleurBlackCursor);

						/* change magnitude of illumination */
						data->illuminate_magnitude =
						    view->illuminate_magnitude_save *
						    exp(((double)(view->button_down_y - view->button_move_y)) / ((double)data->height));
						if (XtIsManaged(view->mb3dview.mbview_textField_illum_amp)) {
              mb_path value_text;
							sprintf(value_text, "%g", data->illuminate_magnitude);
							XmTextFieldSetString(view->mb3dview.mbview_textField_illum_amp, value_text);
						}

						/* clear color status array */
						mbview_setcolorparms(instance);
						mbview_colorclear(instance);

						/* replot */
						mbview_plotlow(instance);
					}

					/* handle shading by slope */
					else if (shade_mode == MBV_SHADE_VIEW_SLOPE) {
						/* set cursor for shading */
						XDefineCursor(view->dpy, view->xid, view->FleurBlackCursor);

						/* change magnitude of slope shading */
						data->slope_magnitude =
						    view->slope_magnitude_save *
						    exp(((double)(view->button_down_y - view->button_move_y)) / ((double)data->height));
						if (XtIsManaged(view->mb3dview.mbview_textField_slope_amp)) {
              mb_path value_text;
							sprintf(value_text, "%g", data->slope_magnitude);
							XmTextFieldSetString(view->mb3dview.mbview_textField_slope_amp, value_text);
						}

						/* clear color status array */
						mbview_setcolorparms(instance);
						mbview_colorclear(instance);

						/* replot */
						mbview_plotlow(instance);
					}

					/* handle shading by overlay */
					else if (shade_mode == MBV_SHADE_VIEW_OVERLAY) {
						/* set cursor for shading */
						XDefineCursor(view->dpy, view->xid, view->FleurBlackCursor);

						/* change magnitude of overlay shading */
						data->overlay_shade_magnitude =
						    view->overlay_shade_magnitude_save *
						    exp(((double)(view->button_down_y - view->button_move_y)) / ((double)data->height));
						if (XtIsManaged(view->mb3dview.mbview_textField_overlay_amp)) {
              mb_path value_text;
							sprintf(value_text, "%g", data->overlay_shade_magnitude);
							XmTextFieldSetString(view->mb3dview.mbview_textField_overlay_amp, value_text);
						}

						/* clear color status array */
						mbview_setcolorparms(instance);
						mbview_colorclear(instance);

						/* replot */
						mbview_plotlow(instance);
					}
				}

				/* handle viewpoint rotation */
				else if (data->mouse_mode == MBV_MOUSE_VIEWPOINT) {
					/* set cursor for scaling */
					XDefineCursor(view->dpy, view->xid, view->SizingBlackCursor);

					/* rescale 2D map */
					if (data->display_mode == MBV_DISPLAY_2D) {
						view->size2d = view->size2d_save *
						               exp(((double)(view->button_down_y - view->button_move_y)) / ((double)data->height));
						if (XtIsManaged(view->mb3dview.mbview_textField_view_2dzoom)) {
              mb_path value_text;
							sprintf(value_text, "%g", view->size2d);
							XmTextFieldSetString(view->mb3dview.mbview_textField_view_2dzoom, value_text);
						}
					}

					/* rescale 3D map */
					else {
						view->viewoffset3d_z =
						    view->viewoffset3d_z_save +
						    2.0 * (((double)(view->button_down_y - view->button_move_y)) / ((double)data->height));
						if (XtIsManaged(view->mb3dview.mbview_textField_view_3dzoom)) {
							mb_path value_text;
							sprintf(value_text, "%g", view->viewoffset3d_z);
							XmTextFieldSetString(view->mb3dview.mbview_textField_view_3dzoom, value_text);
						}
					}

					/* set flag to reset view bounds */
					view->viewboundscount++;

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle area picking */
				else if (data->mouse_mode == MBV_MOUSE_AREA) {
					/* set cursor for area */
					XDefineCursor(view->dpy, view->xid, view->SizingBlackCursor);

					/* process area */
					view->areaaspect = view->areaaspect_save *
					                   exp(((double)(view->button_down_y - view->button_move_y)) / ((double)data->height));

					/* process area */
					mbview_area(instance, MBV_AREAASPECT_CHANGE, view->button_move_x, data->height - view->button_move_y);

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle editing sites */
				else if (data->mouse_mode == MBV_MOUSE_SITE) {
					/* set cursor for sites */
					XDefineCursor(view->dpy, view->xid, view->TargetRedCursor);

					/* set replotall */
					replotall = true;

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle editing routes */
				else if (data->mouse_mode == MBV_MOUSE_ROUTE) {
					/* set cursor for routes */
					XDefineCursor(view->dpy, view->xid, view->TargetRedCursor);

					/* set replotall */
					replotall = true;

					/* set replotprofile */
					replotprofile = true;

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle deselecting navigation */
				else if (data->mouse_mode == MBV_MOUSE_NAV || data->mouse_mode == MBV_MOUSE_NAVFILE) {
					/* set cursor for nav */
					XDefineCursor(view->dpy, view->xid, view->TargetRedCursor);

					/* process nav deselect */
					mbview_pick_nav_select(instance, false, MBV_PICK_MOVE, view->button_move_x,
					                       data->height - view->button_move_y);

					/* set replotall */
					replotall = true;

					/* replot */
					mbview_plotlow(instance);
				}

			} /* end of right button events */

			/* if needed replot all active instances */
			if (replotall) {
				mbview_plotlowall(instance);
			}

		} /* end of motion notify events */

		/* Check for mouse released. */
		if (event->xany.type == ButtonRelease) {
			/*fprintf(stderr, "event->xany.type == ButtonRelease  %d %d  mouse mode:%d\n",
			event->xbutton.x,event->xbutton.y, data->mouse_mode);*/

			/* save location */
			view->button_up_x = event->xbutton.x;
			view->button_up_y = event->xbutton.y;

			/* If left mouse button is released */
			if (view->button1down) {

				/* handle move */
				if ((data->mouse_mode == MBV_MOUSE_MOVE || data->mouse_mode == MBV_MOUSE_ROTATE ||
				     data->mouse_mode == MBV_MOUSE_SHADE || data->mouse_mode == MBV_MOUSE_VIEWPOINT) &&
				    ((view->button_down_x != view->button_up_x) || (view->button_down_y != view->button_up_y))) {
					/* set cursor for pick */
					XDefineCursor(view->dpy, view->xid, view->TargetRedCursor);

					/* process pick */
					mbview_pick(instance, MBV_PICK_UP, view->button_up_x, data->height - view->button_up_y);

					/* set replotprofile */
					replotprofile = true;

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle area picking */
				else if ((data->mouse_mode == MBV_MOUSE_AREA &&
				          (view->button_down_x != view->button_up_x || view->button_down_y != view->button_up_y))) {
					/* process area */
					mbview_region(instance, MBV_REGION_UP, view->button_up_x, data->height - view->button_up_y);

					/* set replotall */
					replotall = true;
				}

				/* handle editing sites */
				else if (data->mouse_mode == MBV_MOUSE_SITE) {
					/* set replotall */
					replotall = true;
				}

				/* handle editing routes */
				else if (data->mouse_mode == MBV_MOUSE_ROUTE) {
					/* set replotall */
					replotall = true;

					/* set replotprofile */
					replotprofile = true;
				}
			}

			/* If middle mouse button is released */
			else if (view->button2down) {
				/* handle move */
				if (data->mouse_mode == MBV_MOUSE_MOVE) {
					/* set flag to reset view bounds */
					view->viewboundscount = MBV_BOUNDSFREQUENCY;
				}

				/* handle 3D rotate */
				else if (data->mouse_mode == MBV_MOUSE_ROTATE) {
					/* set flag to reset view bounds */
					view->viewboundscount = MBV_BOUNDSFREQUENCY;
				}

				/* handle viewpoint rotation */
				else if (data->mouse_mode == MBV_MOUSE_VIEWPOINT) {
					/* set flag to reset view bounds */
					view->viewboundscount = MBV_BOUNDSFREQUENCY;
				}

				/* handle area picking */
				else if (data->mouse_mode == MBV_MOUSE_AREA &&
				         (view->button_down_x != view->button_up_x || view->button_down_y != view->button_up_y)) {
					/* process area */
					mbview_area(instance, MBV_AREALENGTH_UP, view->button_up_x, data->height - view->button_up_y);

					/* set replotall */
					replotall = true;
				}

				/* handle editing sites */
				else if (data->mouse_mode == MBV_MOUSE_SITE) {
					/* set replotall */
					replotall = true;
				}

				/* handle editing routes */
				else if (data->mouse_mode == MBV_MOUSE_ROUTE) {
					/* set replotall */
					replotall = true;

					/* set replotprofile */
					replotprofile = true;
				}

				/* handle selecting navigation */
				else if (data->mouse_mode == MBV_MOUSE_NAV || data->mouse_mode == MBV_MOUSE_NAVFILE) {
					/* process nav select */
					mbview_pick_nav_select(instance, true, MBV_PICK_UP, view->button_move_x,
					                       data->height - view->button_move_y);

					/* set replotall */
					replotall = true;

					/* set replotprofile */
					replotprofile = true;

					/* replot */
					mbview_plotlow(instance);
				}
			} /* end of middle button events */

			/* If right mouse button is released */
			else if (view->button3down) {
				/* change the map scaling */
				if (data->mouse_mode == MBV_MOUSE_MOVE) {
					/* set flag to reset view bounds */
					view->viewboundscount = MBV_BOUNDSFREQUENCY;
				}

				/* handle vertical exaggerate */
				else if (data->mouse_mode == MBV_MOUSE_ROTATE) {
					/* set flag to reset view bounds */
					view->viewboundscount = MBV_BOUNDSFREQUENCY;
				}

				/* handle vertical exaggerate */
				else if (data->mouse_mode == MBV_MOUSE_VIEWPOINT) {
					/* set flag to reset view bounds */
					view->viewboundscount = MBV_BOUNDSFREQUENCY;
				}

				/* handle area picking */
				else if (data->mouse_mode == MBV_MOUSE_AREA &&
				         (view->button_down_x != view->button_up_x || view->button_down_y != view->button_up_y)) {
					/* process area */
					mbview_area(instance, MBV_AREAASPECT_UP, view->button_up_x, data->height - view->button_up_y);

					/* set replotall */
					replotall = true;
				}

				/* handle editing sites */
				else if (data->mouse_mode == MBV_MOUSE_SITE) {
					/* set replotall */
					replotall = true;
				}

				/* handle editing routes */
				else if (data->mouse_mode == MBV_MOUSE_ROUTE) {
					/* set replotall */
					replotall = true;

					/* set replotprofile */
					replotprofile = true;
				}

				/* handle deselecting navigation */
				if (data->mouse_mode == MBV_MOUSE_NAV || data->mouse_mode == MBV_MOUSE_NAVFILE) {
					/* process nav deselect */
					mbview_pick_nav_select(instance, false, MBV_PICK_UP, view->button_move_x, data->height - view->button_move_y);

					/* set replotall */
					replotall = true;

					/* set replotprofile */
					replotprofile = true;

					/* replot */
					mbview_plotlow(instance);
				}
			} /* end of right button events */

			/* unset all buttondown flags */
			view->button1down = false;
			view->button2down = false;
			view->button3down = false;

			/* replot in high rez if last draw was low rez */
			if (view->lastdrawrez == MBV_REZ_LOW) {
				/* replot the current instance */
				mbview_plothigh(instance);
			}

			/* if needed replot all active instances */
			if (replotall) {
				mbview_plothighall(instance);
			}

			/* reset cursor */
			XDefineCursor(view->dpy, view->xid, view->TargetBlackCursor);

			/* allow event interruption of plotting */
			view->plot_interrupt_allowed = true;
		} /* end of button release events */

		/* Deal with KeyPress events */
		if (event->xany.type == KeyPress) {
			/* fprintf(stderr,"KeyPress event\n"); */
			/* Get key pressed - buffer[0] */
			// int actual =
			KeySym keysym;
			char buffer[1];
			XLookupString((XKeyEvent *)event, buffer, 1, &keysym, NULL);

			/* process events */
			switch (buffer[0]) {
			case 'R':
			case 'r':
				do_mbview_reset_view(w, client_data, call_data);
				break;
			default:
				break;
			} /* end of key switch */

		} /* end of key press events */

		/* if needed replot profile */
		if (replotprofile) {
			/* extract profile if pick is right type */
			if (data->pickinfo_mode == MBV_PICK_TWOPOINT)
				mbview_extract_pick_profile(instance);
			else if (data->pickinfo_mode == MBV_PICK_ROUTE)
				mbview_extract_route_profile(instance);
			else if (data->pickinfo_mode == MBV_PICK_NAV)
				mbview_extract_nav_profile(instance);

			/* now replot profile */
			mbview_plotprofile(instance);
		}

		/* update action buttons according to pick state */
		/* fprintf(stderr,"About to call mbview_action_sensitivity %zu\n",instance); */
		mbview_action_sensitivity(instance);

	} /* end of inputs from window */
}

/*------------------------------------------------------------------------------*/

void do_mbview_dismiss(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_dismiss: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* destroy the widgets for this instance  */
	if (data->active) {
		int error = MB_ERROR_NO_ERROR;
		mbview_destroy(mbv_verbose, instance, true, &error);
	}
}

/*------------------------------------------------------------------------------*/

void do_mbview_goaway(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* destroy the widgets for this instance  */
	if (data->active) {
		int error = MB_ERROR_NO_ERROR;
		mbview_destroy(mbv_verbose, instance, false, &error);
	}
}
/*------------------------------------------------------------------------------*/
int mbview_destroy(int verbose, size_t instance, bool destroywidgets, int *error) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       instance:        %zu\n", instance);
		fprintf(stderr, "dbg2       destroywidgets:  %d\n", destroywidgets);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	int status = MB_SUCCESS;

	/* handle destruction if not already handled */
	if (data->active) {
		/* destroy the widgets */
		if (destroywidgets) {
			/* delete old glx_context if it exists */
			if (view->prglx_init) {
/* make correct window current for OpenGL */
#ifdef MBV_DEBUG_GLX
				fprintf(stderr, "%s:%d:%s instance:%zu glXMakeCurrent(%p,%lu,%p)\n", __FILE__, __LINE__, __func__, instance,
				        view->dpy, XtWindow(view->prglwmda), view->prglx_context);
#endif
				//glXMakeCurrent(view->dpy, XtWindow(view->prglwmda), view->prglx_context);

#ifdef MBV_GET_GLX_ERRORS
				mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif

#ifdef MBV_DEBUG_GLX
				fprintf(stderr, "%s:%d:%s instance:%zu glXDestroyContext(%p,%p)\n", __FILE__, __LINE__, __func__, instance,
				        view->dpy, view->prglx_context);
#endif
				glXDestroyContext(view->dpy, view->prglx_context);
				view->prglx_init = false;

#ifdef MBV_GET_GLX_ERRORS
				mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif
			}

			/* delete old glx_context if it exists */
			if (view->glx_init) {
/* make correct window current for OpenGL */
#ifdef MBV_DEBUG_GLX
				fprintf(stderr, "%s:%d:%s instance:%zu glXMakeCurrent(%p,%lu,%p)\n", __FILE__, __LINE__, __func__, instance,
				        XtDisplay(view->glwmda), XtWindow(view->glwmda), view->glx_context);
#endif
				//glXMakeCurrent(XtDisplay(view->glwmda), XtWindow(view->glwmda), view->glx_context);

#ifdef MBV_GET_GLX_ERRORS
				mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif

#ifdef MBV_DEBUG_GLX
				fprintf(stderr, "%s:%d:%s instance:%zu glXDestroyContext(%p,%p)\n", __FILE__, __LINE__, __func__, instance,
				        view->dpy, view->glx_context);
#endif
				glXDestroyContext(view->dpy, view->glx_context);
				view->glx_init = false;

#ifdef MBV_GET_GLX_ERRORS
				mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif
			}

/* destroy the topLevelShell and all its children */
#ifdef MBV_DEBUG_GLX
			fprintf(stderr, "%s:%d:%s instance:%zu XtDestroyWidget(%p)\n", __FILE__, __LINE__, __func__, instance,
			        view->topLevelShell);
#endif
			XtDestroyWidget(view->topLevelShell);

#ifdef MBV_GET_GLX_ERRORS
			mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif
		}

		data->active = false;
		view->init = MBV_WINDOW_NULL;
		mbv_ninstance--;

		/* deallocate memory */
		if (status == MB_SUCCESS && data->primary_data != NULL)
			status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&data->primary_data, error);
		if (status == MB_SUCCESS && data->primary_x != NULL)
			status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&data->primary_x, error);
		if (status == MB_SUCCESS && data->primary_y != NULL)
			status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&data->primary_y, error);
		if (status == MB_SUCCESS && data->primary_z != NULL)
			status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&data->primary_z, error);
		if (status == MB_SUCCESS && data->primary_dzdx != NULL)
			status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&data->primary_dzdx, error);
		if (status == MB_SUCCESS && data->primary_dzdy != NULL)
			status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&data->primary_dzdy, error);
		if (status == MB_SUCCESS && data->primary_r != NULL)
			status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&data->primary_r, error);
		if (status == MB_SUCCESS && data->primary_g != NULL)
			status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&data->primary_g, error);
		if (status == MB_SUCCESS && data->primary_b != NULL)
			status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&data->primary_b, error);
		if (status == MB_SUCCESS && data->primary_stat_color != NULL)
			status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&data->primary_stat_color, error);
		if (status == MB_SUCCESS && data->primary_stat_z != NULL)
			status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&data->primary_stat_z, error);
		if (status == MB_SUCCESS && data->secondary_data != NULL)
			status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&data->secondary_data, error);
		if (status == MB_SUCCESS && data->pick.segment.nls_alloc != 0 && data->pick.segment.lspoints != NULL) {
			status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&data->pick.segment.lspoints, error);
			data->pick.segment.nls_alloc = 0;
		}
		for (int i = 0; i < 4; i++) {
			if (status == MB_SUCCESS && data->pick.xsegments[i].nls_alloc != 0 && data->pick.xsegments[i].lspoints != NULL) {
				status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&data->pick.xsegments[i].lspoints, error);
				data->pick.xsegments[i].nls_alloc = 0;
			}
		}
		if (status == MB_SUCCESS && data->area.segment.nls_alloc != 0 && data->area.segment.lspoints != NULL) {
			status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&data->area.segment.lspoints, error);
			data->area.segment.nls_alloc = 0;
		}
		for (int i = 0; i < 4; i++) {
			if (status == MB_SUCCESS && data->area.segments[i].nls_alloc != 0 && data->area.segments[i].lspoints != NULL) {
				status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&data->area.segments[i].lspoints, error);
				data->area.segments[i].nls_alloc = 0;
			}
		}
		for (int i = 0; i < 4; i++) {
			if (status == MB_SUCCESS && data->region.segments[i].nls_alloc != 0 && data->region.segments[i].lspoints != NULL) {
				status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&data->region.segments[i].lspoints, error);
				data->region.segments[i].nls_alloc = 0;
			}
		}
		if (data->profile.npoints_alloc > 0) {
			status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&data->profile.points, error);
			data->profile.npoints_alloc = 0;
		}

		/* deallocate shared data if no more active instances */
		if (mbv_ninstance <= 0) {
			shared.init_sitelist = MBV_WINDOW_NULL;
			XmListDeleteAllItems(shared.mb3d_sitelist.mbview_list_sitelist);
			XtPopdown(XtParent(shared.mainWindow_sitelist));

			shared.init_routelist = MBV_WINDOW_NULL;
			XmListDeleteAllItems(shared.mb3d_routelist.mbview_list_routelist);
			XtPopdown(XtParent(shared.mainWindow_routelist));

			shared.init_navlist = MBV_WINDOW_NULL;
			XmListDeleteAllItems(shared.mb3d_navlist.mbview_list_navlist);
			XtPopdown(XtParent(shared.mainWindow_navlist));

			if (status == MB_SUCCESS && shared.shareddata.navpick.segment.nls_alloc != 0 &&
			    shared.shareddata.navpick.segment.lspoints != NULL) {
				status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&shared.shareddata.navpick.segment.lspoints, error);
				shared.shareddata.navpick.segment.nls_alloc = 0;
			}
			for (int i = 0; i < 4; i++) {
				if (status == MB_SUCCESS && shared.shareddata.navpick.xsegments[i].lspoints != 0 &&
				    shared.shareddata.navpick.xsegments[i].lspoints != NULL)
					status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&shared.shareddata.navpick.xsegments[i].lspoints,
					                  error);
			}
			if (status == MB_SUCCESS && shared.shareddata.nsite_alloc != 0 && shared.shareddata.sites != NULL) {
				status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(shared.shareddata.sites), error);
				shared.shareddata.nsite_alloc = 0;
				shared.shareddata.sites = NULL;
			}
			if (status == MB_SUCCESS && shared.shareddata.nroute_alloc != 0 && shared.shareddata.routes != NULL) {
				for (int i = 0; i < shared.shareddata.nroute_alloc; i++) {
					for (int j = 0; j < shared.shareddata.routes[i].npoints_alloc; j++) {
						if (shared.shareddata.routes[i].segments[j].nls_alloc != 0 &&
						    shared.shareddata.routes[i].segments[j].lspoints != NULL) {
							status = mb_freed(mbv_verbose, __FILE__, __LINE__,
							                  (void **)&shared.shareddata.routes[i].segments[j].lspoints, error);
							shared.shareddata.routes[i].segments[j].nls_alloc = 0;
						}
					}
					status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(shared.shareddata.routes[i].waypoint), error);
					status =
					    mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(shared.shareddata.routes[i].distlateral), error);
					status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(shared.shareddata.routes[i].disttopo), error);
					status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(shared.shareddata.routes[i].points), error);
					status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(shared.shareddata.routes[i].segments), error);
				}
				status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(shared.shareddata.routes), error);
				shared.shareddata.nroute_alloc = 0;
				shared.shareddata.routes = NULL;
			}
			if (status == MB_SUCCESS && shared.shareddata.nnav_alloc != 0 && shared.shareddata.navs != NULL) {
				status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(shared.shareddata.navs), error);
				shared.shareddata.nnav_alloc = 0;
				shared.shareddata.navs = NULL;
			}
		}

		if (status != MB_SUCCESS) {
			fprintf(stderr, "\nUnable to free memory\n");
			fprintf(stderr, "\nProgram terminated in function <%s>\n", __func__);
			exit(0);
		}

		/* if no more active instances reset shared data */
		if (mbv_ninstance <= 0)
			mbview_reset_shared(false);

		/* initialize view for next use */
		mbview_reset(instance);

		/* reset action button sensitivity for all instances */
		mbview_action_sensitivityall();

		/* let the calling program know */
		(data->mbview_dismiss_notify)(instance);
	}

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:        %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/

int mbview_quit(int verbose, int *error) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
	}

	/* Destroy the list windows */
	XtUnmanageChild(shared.mb3d_sitelist.MB3DSiteList);
	XtUnmanageChild(shared.mb3d_routelist.MB3DRouteList);
	XtUnmanageChild(shared.mb3d_navlist.MB3DNavList);

	/* loope over all possible instances and dismiss anything that's up */
	for (int i = 0; i < MBV_MAX_WINDOWS; i++) {
		if (mbviews[i].init)
			mbview_destroy(verbose, i, true, error);
	}

	const int status = MB_SUCCESS;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:        %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/

void do_mbview_display_2D(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get mode value */
	data->display_mode = MBV_DISPLAY_2D;

	/* set togglebuttons */
	set_mbview_display_mode(instance, data->display_mode);
	set_mbview_mouse_mode(instance, data->mouse_mode);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_display_2d instance:%zu mode:%d\n", instance, data->display_mode);

	/* set contour flags */
	view->contourlorez = false;
	view->contourhirez = false;
	view->contourfullrez = false;

	/* set flag to reset view bounds */
	view->viewboundscount = MBV_BOUNDSFREQUENCY;

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_display_2D\n");
	mbview_plotlowhigh(instance);
}
/*------------------------------------------------------------------------------*/

void do_mbview_display_3D(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get mode value */
	data->display_mode = MBV_DISPLAY_3D;

	/* set togglebuttons */
	set_mbview_display_mode(instance, data->display_mode);
	set_mbview_mouse_mode(instance, data->mouse_mode);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_display_3d instance:%zu mode:%d\n", instance, data->display_mode);

	/* set contour flags */
	view->contourlorez = false;
	view->contourhirez = false;
	view->contourfullrez = false;

	/* set flag to reset view bounds */
	view->viewboundscount = MBV_BOUNDSFREQUENCY;

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_display_3D\n");
	mbview_plotlowhigh(instance);
}
/*------------------------------------------------------------------------------*/

void do_mbview_data_primary(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get mode value */
	data->grid_mode = MBV_GRID_VIEW_PRIMARY;

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_data_primary instance:%zu mode:%d\n", instance, data->grid_mode);

	/* set togglebuttons */
	set_mbview_grid_mode(instance, data->grid_mode);
	set_mbview_histogram_mode(instance, data->primary_histogram);
	set_mbview_colortable(instance, data->primary_colortable);
	set_mbview_colortable_mode(instance, data->primary_colortable_mode);
	set_mbview_shade_mode(instance, data->primary_shade_mode);

	/* clear color status array */
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_data_primary\n");
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);
}

/*------------------------------------------------------------------------------*/
void do_mbview_data_primaryslope(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get mode value */
	data->grid_mode = MBV_GRID_VIEW_PRIMARYSLOPE;

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_data_primaryslope instance:%zu mode:%d\n", instance, data->grid_mode);

	/* set togglebuttons */
	set_mbview_grid_mode(instance, data->grid_mode);
	set_mbview_histogram_mode(instance, data->primaryslope_histogram);
	set_mbview_colortable(instance, data->slope_colortable);
	set_mbview_colortable_mode(instance, data->slope_colortable_mode);
	set_mbview_shade_mode(instance, data->slope_shade_mode);

	/* clear color status array */
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_data_primaryslope\n");
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);
}

/*------------------------------------------------------------------------------*/
void do_mbview_data_secondary(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get mode value */
	data->grid_mode = MBV_GRID_VIEW_SECONDARY;

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_data_secondary instance:%zu mode:%d\n", instance, data->grid_mode);

	/* set togglebuttons */
	set_mbview_grid_mode(instance, data->grid_mode);
	set_mbview_histogram_mode(instance, data->secondary_histogram);
	set_mbview_colortable(instance, data->secondary_colortable);
	set_mbview_colortable_mode(instance, data->secondary_colortable_mode);
	set_mbview_shade_mode(instance, data->secondary_shade_mode);

	/* clear color status array */
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_data_secondary\n");
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);
}

/*------------------------------------------------------------------------------*/
void do_mbview_histogram(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);
	/*fprintf(stderr,"do_mbview_histogram instance:%zu\n", instance);*/

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get histogram value */
	const Boolean value = XmToggleButtonGetState(w);
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY) {
		data->primary_histogram = value;
	}
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE) {
		data->primaryslope_histogram = value;
	}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY) {
		data->secondary_histogram = value;
	}

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_histogram instance:%zu mode:%d\n", instance, data->grid_mode);

	/* clear color status array */
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_histogram\n");
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);
}

/*------------------------------------------------------------------------------*/
void do_mbview_overlay_none(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_overlay_none instance:%zu\n", instance);

	/* set mode value */
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY) {
		data->primary_shade_mode = MBV_SHADE_VIEW_NONE;
		set_mbview_shade_mode(instance, data->primary_shade_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE) {
		data->slope_shade_mode = MBV_SHADE_VIEW_NONE;
		set_mbview_shade_mode(instance, data->slope_shade_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY) {
		data->secondary_shade_mode = MBV_SHADE_VIEW_NONE;
		set_mbview_shade_mode(instance, data->secondary_shade_mode);
	}

	/* clear color status array */
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_overlay_none\n");
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);
}

/*------------------------------------------------------------------------------*/
void do_mbview_overlay_slope(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_overlay_slope instance:%zu\n", instance);

	/* set mode value */
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY) {
		data->primary_shade_mode = MBV_SHADE_VIEW_SLOPE;
		set_mbview_shade_mode(instance, data->primary_shade_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE) {
		data->slope_shade_mode = MBV_SHADE_VIEW_SLOPE;
		set_mbview_shade_mode(instance, data->slope_shade_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY) {
		data->secondary_shade_mode = MBV_SHADE_VIEW_SLOPE;
		set_mbview_shade_mode(instance, data->secondary_shade_mode);
	}

	/* clear color status array */
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_overlay_slope\n");
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);

}

/*------------------------------------------------------------------------------*/
void do_mbview_overlay_illumination(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_overlay_illumination instance:%zu\n", instance);

	/* set mode value */
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY) {
		data->primary_shade_mode = MBV_SHADE_VIEW_ILLUMINATION;
		set_mbview_shade_mode(instance, data->primary_shade_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE) {
		data->slope_shade_mode = MBV_SHADE_VIEW_ILLUMINATION;
		set_mbview_shade_mode(instance, data->slope_shade_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY) {
		data->secondary_shade_mode = MBV_SHADE_VIEW_ILLUMINATION;
		set_mbview_shade_mode(instance, data->secondary_shade_mode);
	}

	/* clear color status array */
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_overlay_illumination\n");
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);
}

/*------------------------------------------------------------------------------*/
void do_mbview_overlay_secondary(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_overlay_secondary instance:%zu\n", instance);

	/* set mode value */
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY) {
		data->primary_shade_mode = MBV_SHADE_VIEW_OVERLAY;
		set_mbview_shade_mode(instance, data->primary_shade_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE) {
		data->slope_shade_mode = MBV_SHADE_VIEW_OVERLAY;
		set_mbview_shade_mode(instance, data->slope_shade_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY) {
		data->secondary_shade_mode = MBV_SHADE_VIEW_OVERLAY;
		set_mbview_shade_mode(instance, data->secondary_shade_mode);
	}

	/* clear color status array */
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_overlay_secondary\n");
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);
}

/*------------------------------------------------------------------------------*/
void do_mbview_overlay_contour(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	const Boolean value = XmToggleButtonGetState(w);
	data->grid_contour_mode = value ? MBV_VIEW_ON : MBV_VIEW_OFF;

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_overlay_contour instance:%zu mode:%d\n", instance, data->grid_contour_mode);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_overlay_contour\n");
	mbview_plotlowhigh(instance);
}
/*------------------------------------------------------------------------------*/
void do_mbview_site(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get mode value */
	const Boolean value = XmToggleButtonGetState(w);
	if (value) {
		data->site_view_mode = MBV_VIEW_ON;
	} else {
		data->site_view_mode = MBV_VIEW_OFF;

		/* if site view is off, site edit must be off */
		if (data->mouse_mode == MBV_MOUSE_SITE) {
			data->mouse_mode = MBV_MOUSE_MOVE;
			set_mbview_mouse_mode(instance, data->mouse_mode);

			/* make sure sites aren't selected if edit modes off */
			shared.shareddata.site_selected = MBV_SELECT_NONE;
			data->pickinfo_mode = data->pick_type;

			/* set pick annotation */
			mbview_pick_text(instance);
		}
	}

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_site instance:%zu mode:%d\n", instance, data->site_view_mode);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_site\n");
	mbview_plotlowhigh(instance);
}
/*------------------------------------------------------------------------------*/

void do_mbview_route(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get mode value */
	const Boolean value = XmToggleButtonGetState(w);
	if (value) {
		data->route_view_mode = MBV_VIEW_ON;
	} else {
		data->route_view_mode = MBV_VIEW_OFF;

		/* if route view is off, route edit must be off */
		if (data->mouse_mode == MBV_MOUSE_ROUTE) {
			data->mouse_mode = MBV_MOUSE_MOVE;
			set_mbview_mouse_mode(instance, data->mouse_mode);

			/* make sure routes aren't selected if edit modes off */
			shared.shareddata.route_selected = MBV_SELECT_NONE;
			shared.shareddata.route_point_selected = MBV_SELECT_NONE;
			data->pickinfo_mode = data->pick_type;

			/* set pick annotation */
			mbview_pick_text(instance);
		}
	}

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_route instance:%zu mode:%d\n", instance, data->route_view_mode);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_route\n");
	mbview_plotlowhigh(instance);
}

/*------------------------------------------------------------------------------*/

void do_mbview_nav(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get mode value */
	Boolean value = XmToggleButtonGetState(w);
	if (value) {
		data->nav_view_mode = MBV_VIEW_ON;
	} else {
		data->nav_view_mode = MBV_VIEW_OFF;
		if (data->navdrape_view_mode == MBV_VIEW_OFF &&
		    (data->mouse_mode == MBV_MOUSE_NAV || data->mouse_mode == MBV_MOUSE_NAVFILE)) {
			data->mouse_mode = MBV_MOUSE_MOVE;
			set_mbview_mouse_mode(instance, data->mouse_mode);
		}
	}

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_nav instance:%zu mode:%d\n", instance, data->nav_view_mode);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_nav\n");
	mbview_plotlowhigh(instance);
}

/*------------------------------------------------------------------------------*/

void do_mbview_navdrape(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get mode value */
	const Boolean value = XmToggleButtonGetState(w);
	if (value) {
		data->navdrape_view_mode = MBV_VIEW_ON;
	} else {
		data->navdrape_view_mode = MBV_VIEW_OFF;
		if (data->nav_view_mode == MBV_VIEW_OFF && (data->mouse_mode == MBV_MOUSE_NAV || data->mouse_mode == MBV_MOUSE_NAVFILE)) {
			data->mouse_mode = MBV_MOUSE_MOVE;
			set_mbview_mouse_mode(instance, data->mouse_mode);
		}
	}

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_navdrape instance:%zu mode:%d\n", instance, data->navdrape_view_mode);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_navdrape\n");
	mbview_plotlowhigh(instance);
}
/*------------------------------------------------------------------------------*/

void do_mbview_vector(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get mode value */
	const Boolean value = XmToggleButtonGetState(w);
	if (value) {
		data->vector_view_mode = MBV_VIEW_ON;
	} else {
		data->vector_view_mode = MBV_VIEW_OFF;
		if (data->nav_view_mode == MBV_VIEW_OFF && (data->mouse_mode == MBV_MOUSE_NAV || data->mouse_mode == MBV_MOUSE_NAVFILE)) {
			data->mouse_mode = MBV_MOUSE_MOVE;
			set_mbview_mouse_mode(instance, data->mouse_mode);
		}
	}

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_vector instance:%zu mode:%d\n", instance, data->vector_view_mode);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_vector\n");
	mbview_plotlowhigh(instance);
}

/*------------------------------------------------------------------------------*/

void do_mbview_colortable_haxby(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get mode value */
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY) {
		data->primary_colortable = MBV_COLORTABLE_HAXBY;
		set_mbview_colortable(instance, data->primary_colortable);
		set_mbview_colortable_mode(instance, data->primary_colortable_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE) {
		data->slope_colortable = MBV_COLORTABLE_HAXBY;
		set_mbview_colortable(instance, data->slope_colortable);
		set_mbview_colortable_mode(instance, data->slope_colortable_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY) {
		data->secondary_colortable = MBV_COLORTABLE_HAXBY;
		set_mbview_colortable(instance, data->secondary_colortable);
		set_mbview_colortable_mode(instance, data->secondary_colortable_mode);
	}

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_colortable_haxby instance:%zu\n", instance);

	/* clear color status array */
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_colortable_haxby\n");
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);
}
/*------------------------------------------------------------------------------*/

void do_mbview_colortable_bright(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get mode value */
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY) {
		data->primary_colortable = MBV_COLORTABLE_BRIGHT;
		set_mbview_colortable(instance, data->primary_colortable);
		set_mbview_colortable_mode(instance, data->primary_colortable_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE) {
		data->slope_colortable = MBV_COLORTABLE_BRIGHT;
		set_mbview_colortable(instance, data->slope_colortable);
		set_mbview_colortable_mode(instance, data->slope_colortable_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY) {
		data->secondary_colortable = MBV_COLORTABLE_BRIGHT;
		set_mbview_colortable(instance, data->secondary_colortable);
		set_mbview_colortable_mode(instance, data->secondary_colortable_mode);
	}

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_colortable_bright instance:%zu\n", instance);

	/* clear color status array */
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_colortable_bright\n");
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);
}
/*------------------------------------------------------------------------------*/

void do_mbview_colortable_muted(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get mode value */
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY) {
		data->primary_colortable = MBV_COLORTABLE_MUTED;
		set_mbview_colortable(instance, data->primary_colortable);
		set_mbview_colortable_mode(instance, data->primary_colortable_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE) {
		data->slope_colortable = MBV_COLORTABLE_MUTED;
		set_mbview_colortable(instance, data->slope_colortable);
		set_mbview_colortable_mode(instance, data->slope_colortable_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY) {
		data->secondary_colortable = MBV_COLORTABLE_MUTED;
		set_mbview_colortable(instance, data->secondary_colortable);
		set_mbview_colortable_mode(instance, data->secondary_colortable_mode);
	}

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_colortable_muted instance:%zu\n", instance);

	/* clear color status array */
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_colortable_muted\n");
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);
}
/*------------------------------------------------------------------------------*/

void do_mbview_colortable_gray(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get mode value */
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY) {
		data->primary_colortable = MBV_COLORTABLE_GRAY;
		set_mbview_colortable(instance, data->primary_colortable);
		set_mbview_colortable_mode(instance, data->primary_colortable_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE) {
		data->slope_colortable = MBV_COLORTABLE_GRAY;
		set_mbview_colortable(instance, data->slope_colortable);
		set_mbview_colortable_mode(instance, data->slope_colortable_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY) {
		data->secondary_colortable = MBV_COLORTABLE_GRAY;
		set_mbview_colortable(instance, data->secondary_colortable);
		set_mbview_colortable_mode(instance, data->secondary_colortable_mode);
	}

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_colortable_gray instance:%zu\n", instance);

	/* clear color status array */
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_colortable_gray\n");
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);
}
/*------------------------------------------------------------------------------*/

void do_mbview_colortable_flat(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get mode value */
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY) {
		data->primary_colortable = MBV_COLORTABLE_FLAT;
		set_mbview_colortable(instance, data->primary_colortable);
		set_mbview_colortable_mode(instance, data->primary_colortable_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE) {
		data->slope_colortable = MBV_COLORTABLE_FLAT;
		set_mbview_colortable(instance, data->slope_colortable);
		set_mbview_colortable_mode(instance, data->slope_colortable_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY) {
		data->secondary_colortable = MBV_COLORTABLE_FLAT;
		set_mbview_colortable(instance, data->secondary_colortable);
		set_mbview_colortable_mode(instance, data->secondary_colortable_mode);
	}

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_colortable_flat instance:%zu\n", instance);

	/* clear color status array */
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_colortable_flat\n");
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);
}

/*------------------------------------------------------------------------------*/

void do_mbview_colortable_sealevel1(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get mode value */
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY) {
		data->primary_colortable = MBV_COLORTABLE_SEALEVEL1;
		set_mbview_colortable(instance, data->primary_colortable);
		set_mbview_colortable_mode(instance, data->primary_colortable_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE) {
		data->slope_colortable = MBV_COLORTABLE_SEALEVEL1;
		set_mbview_colortable(instance, data->slope_colortable);
		set_mbview_colortable_mode(instance, data->slope_colortable_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY) {
		data->secondary_colortable = MBV_COLORTABLE_SEALEVEL1;
		set_mbview_colortable(instance, data->secondary_colortable);
		set_mbview_colortable_mode(instance, data->secondary_colortable_mode);
	}

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_colortable_sealevel1 instance:%zu\n", instance);

	/* clear color status array */
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_colortable_sealevel1\n");
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);
}
/*------------------------------------------------------------------------------*/

void do_mbview_colortable_sealevel2(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get mode value */
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY) {
		data->primary_colortable = MBV_COLORTABLE_SEALEVEL2;
		set_mbview_colortable(instance, data->primary_colortable);
		set_mbview_colortable_mode(instance, data->primary_colortable_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE) {
		data->slope_colortable = MBV_COLORTABLE_SEALEVEL2;
		set_mbview_colortable(instance, data->slope_colortable);
		set_mbview_colortable_mode(instance, data->slope_colortable_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY) {
		data->secondary_colortable = MBV_COLORTABLE_SEALEVEL2;
		set_mbview_colortable(instance, data->secondary_colortable);
		set_mbview_colortable_mode(instance, data->secondary_colortable_mode);
	}

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_colortable_sealevel2 instance:%zu\n", instance);

	/* clear color status array */
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_colortable_sealevel2\n");
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);
}

/*------------------------------------------------------------------------------*/
void do_mbview_mouse_rmode(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_mouse_rmode: \n");

	XmToggleButtonCallbackStruct *acs = (XmToggleButtonCallbackStruct *)call_data;


	/* do nothing unless calling widget is set */
	if (acs->event != NULL && acs->set > 0) {
		Arg args[256];
    Cardinal ac = 0;
		size_t instance;
		XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
		ac++;
		XtGetValues(w, args, ac);

		struct mbview_world_struct *view = &(mbviews[instance]);
		struct mbview_struct *data = &(view->data);

		/* get mode value */
		MB3DViewData *mb3dviewptr = &(view->mb3dview);

		bool replot = false;

		if (w == (mb3dviewptr->mbview_toggleButton_mode_rmove))
			data->mouse_mode = MBV_MOUSE_MOVE;
		else if (w == (mb3dviewptr->mbview_toggleButton_mode_rrotate))
			data->mouse_mode = MBV_MOUSE_ROTATE;
		else if (w == (mb3dviewptr->mbview_toggleButton_mode_rshade))
			data->mouse_mode = MBV_MOUSE_SHADE;
		else if (w == (mb3dviewptr->mbview_toggleButton_mode_rviewpoint))
			data->mouse_mode = MBV_MOUSE_VIEWPOINT;
		else if (w == (mb3dviewptr->mbview_toggleButton_mode_rarea))
			data->mouse_mode = MBV_MOUSE_AREA;
		else if (w == (mb3dviewptr->mbview_toggleButton_mode_rsite)) {
			data->mouse_mode = MBV_MOUSE_SITE;
			data->site_view_mode = MBV_VIEW_ON;
			set_mbview_site_view_mode(instance, data->site_view_mode);
			replot = true;
		}
		else if (w == (mb3dviewptr->mbview_toggleButton_mode_rroute)) {
			data->mouse_mode = MBV_MOUSE_ROUTE;
			data->route_view_mode = MBV_VIEW_ON;
			set_mbview_route_view_mode(instance, data->route_view_mode);
			replot = true;
		}
		else if (w == (mb3dviewptr->mbview_toggleButton_mode_rnav)) {
			data->mouse_mode = MBV_MOUSE_NAV;
			if (data->display_mode == MBV_DISPLAY_3D) {
				data->navdrape_view_mode = MBV_VIEW_ON;
				set_mbview_navdrape_view_mode(instance, data->navdrape_view_mode);
				replot = true;
			}
			else {
				data->nav_view_mode = MBV_VIEW_ON;
				set_mbview_nav_view_mode(instance, data->nav_view_mode);
				replot = true;
			}
		}
		else if (w == (mb3dviewptr->mbview_toggleButton_mode_rnavfile)) {
			data->mouse_mode = MBV_MOUSE_NAVFILE;
			if (data->display_mode == MBV_DISPLAY_3D) {
				data->navdrape_view_mode = MBV_VIEW_ON;
				set_mbview_navdrape_view_mode(instance, data->navdrape_view_mode);
				replot = true;
			}
			else {
				data->nav_view_mode = MBV_VIEW_ON;
				set_mbview_nav_view_mode(instance, data->nav_view_mode);
				replot = true;
			}
		}

		/* make sure sites or routes aren't selected if edit modes off */
		if (data->mouse_mode != MBV_MOUSE_SITE && shared.shareddata.site_selected != MBV_SELECT_NONE) {
			shared.shareddata.site_selected = MBV_SELECT_NONE;
			data->pickinfo_mode = data->pick_type;
			replot = true;
		}
		if (data->mouse_mode != MBV_MOUSE_ROUTE && shared.shareddata.route_selected != MBV_SELECT_NONE &&
		    shared.shareddata.route_mode != MBV_ROUTE_NAVADJUST) {
			shared.shareddata.route_selected = MBV_SELECT_NONE;
			shared.shareddata.route_point_selected = MBV_SELECT_NONE;
			data->pickinfo_mode = data->pick_type;
			replot = true;
		}

		/* set mouse togglebuttons */
		set_mbview_mouse_mode(instance, data->mouse_mode);

		/* replot if necessary */
		if (replot) {
			/* set pick annotation */
			mbview_pick_text(instance);

			/* draw */
			if (mbv_verbose >= 2)
				fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_mouse_rmode\n");
			mbview_plotlowhigh(instance);
		}
	}
}

/*------------------------------------------------------------------------------*/
void do_mbview_mouse_mode(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter
	// XmToggleButtonCallbackStruct *acs = (XmToggleButtonCallbackStruct *)call_data;

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_mouse_mode: \n");

	// do nothing unless calling widget is set
	// if (acs->event != NULL && acs->set > 0)
	{
		Arg args[256];
    Cardinal ac = 0;
		size_t instance;
		XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
		ac++;
		XtGetValues(w, args, ac);

		struct mbview_world_struct *view = &(mbviews[instance]);
		struct mbview_struct *data = &(view->data);

		/* get mode value */
		MB3DViewData *mb3dviewptr = &(view->mb3dview);

		int replot = false;

		if (w == (mb3dviewptr->mbview_toggleButton_mode_move))
			data->mouse_mode = MBV_MOUSE_MOVE;
		else if (w == (mb3dviewptr->mbview_toggleButton_mode_rotate))
			data->mouse_mode = MBV_MOUSE_ROTATE;
		else if (w == (mb3dviewptr->mbview_toggleButton_mode_shade))
			data->mouse_mode = MBV_MOUSE_SHADE;
		else if (w == (mb3dviewptr->mbview_toggleButton_mode_viewpoint))
			data->mouse_mode = MBV_MOUSE_VIEWPOINT;
		else if (w == (mb3dviewptr->mbview_toggleButton_mode_area))
			data->mouse_mode = MBV_MOUSE_AREA;
		else if (w == (mb3dviewptr->mbview_toggleButton_mode_site)) {
			data->mouse_mode = MBV_MOUSE_SITE;
			data->site_view_mode = MBV_VIEW_ON;
			set_mbview_site_view_mode(instance, data->site_view_mode);
		}
		else if (w == (mb3dviewptr->mbview_toggleButton_mode_route)) {
			data->mouse_mode = MBV_MOUSE_ROUTE;
			data->route_view_mode = MBV_VIEW_ON;
			set_mbview_route_view_mode(instance, data->route_view_mode);
		}
		else if (w == (mb3dviewptr->mbview_toggleButton_mode_nav)) {
			data->mouse_mode = MBV_MOUSE_NAV;
			if (data->display_mode == MBV_DISPLAY_3D) {
				data->navdrape_view_mode = MBV_VIEW_ON;
				set_mbview_navdrape_view_mode(instance, data->nav_view_mode);
			}
			else {
				data->nav_view_mode = MBV_VIEW_ON;
				set_mbview_nav_view_mode(instance, data->nav_view_mode);
			}
		}
		else if (w == (mb3dviewptr->mbview_toggleButton_mode_navfile)) {
			data->mouse_mode = MBV_MOUSE_NAVFILE;
			if (data->display_mode == MBV_DISPLAY_3D) {
				data->navdrape_view_mode = MBV_VIEW_ON;
				set_mbview_navdrape_view_mode(instance, data->nav_view_mode);
			}
			else {
				data->nav_view_mode = MBV_VIEW_ON;
				set_mbview_nav_view_mode(instance, data->nav_view_mode);
			}
		}

		/* make sure sites or routes aren't selected if edit modes off */
		if (data->mouse_mode != MBV_MOUSE_SITE && shared.shareddata.site_selected != MBV_SELECT_NONE) {
			shared.shareddata.site_selected = MBV_SELECT_NONE;
			data->pickinfo_mode = data->pick_type;
			replot = true;
		}
		if (data->mouse_mode != MBV_MOUSE_ROUTE && shared.shareddata.route_selected != MBV_SELECT_NONE &&
		    shared.shareddata.route_mode != MBV_ROUTE_NAVADJUST) {
			shared.shareddata.route_selected = MBV_SELECT_NONE;
			shared.shareddata.route_point_selected = MBV_SELECT_NONE;
			data->pickinfo_mode = data->pick_type;
			replot = true;
		}

		/* set mouse togglebuttons */
		set_mbview_mouse_mode(instance, data->mouse_mode);

		/* replot if necessary */
		if (replot) {
			/* set pick annotation */
			mbview_pick_text(instance);

			/* draw */
			if (mbv_verbose >= 2)
				fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_mouse_mode\n");
			mbview_plotlowhigh(instance);
		}
	}
}

/*------------------------------------------------------------------------------*/
void set_mbview_mouse_mode(size_t instance, int mode) {
	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_mouse_mode: instance:%zu mode:%d\n", instance, mode);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	MB3DViewData *mb3dviewptr = &(view->mb3dview);

	/*set mode */
	data->mouse_mode = mode;
	if (data->display_mode == MBV_DISPLAY_2D && (data->mouse_mode == MBV_MOUSE_ROTATE || data->mouse_mode == MBV_MOUSE_VIEWPOINT))
		data->mouse_mode = MBV_MOUSE_MOVE;

	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_move, False, False);
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_rotate, False, False);
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_shade, False, False);
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_viewpoint, False, False);
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_area, False, False);
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_site, False, False);
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_route, False, False);
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_nav, False, False);
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_navfile, False, False);

	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_rmove, False, False);
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_rrotate, False, False);
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_rshade, False, False);
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_rviewpoint, False, False);
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_rarea, False, False);
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_rsite, False, False);
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_rroute, False, False);
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_rnav, False, False);
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_rnavfile, False, False);

	if (data->mouse_mode == MBV_MOUSE_MOVE) {
		XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_move, True, False);
		XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_rmove, True, False);
	}
	else if (data->mouse_mode == MBV_MOUSE_ROTATE) {
		XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_rotate, True, False);
		XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_rrotate, True, False);
	}
	else if (data->mouse_mode == MBV_MOUSE_SHADE) {
		XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_shade, True, False);
		XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_rshade, True, False);
	}
	else if (data->mouse_mode == MBV_MOUSE_VIEWPOINT) {
		XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_viewpoint, True, False);
		XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_rviewpoint, True, False);
	}
	else if (data->mouse_mode == MBV_MOUSE_AREA) {
		XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_area, True, False);
		XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_rarea, True, False);
	}
	else if (data->mouse_mode == MBV_MOUSE_SITE) {
		XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_site, True, False);
		XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_rsite, True, False);
	}
	else if (data->mouse_mode == MBV_MOUSE_ROUTE) {
		XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_route, True, False);
		XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_rroute, True, False);
	}
	else if (data->mouse_mode == MBV_MOUSE_NAV) {
		XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_nav, True, False);
		XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_rnav, True, False);
	}
	else if (data->mouse_mode == MBV_MOUSE_NAVFILE) {
		XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_navfile, True, False);
		XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_mode_rnavfile, True, False);
	}

	/* set widget sensitivity and visibility */
	if (data->display_mode == MBV_DISPLAY_2D) {
		XtVaSetValues(mb3dviewptr->mbview_toggleButton_mode_rotate, XmNsensitive, False, NULL);
		XtVaSetValues(mb3dviewptr->mbview_toggleButton_mode_viewpoint, XmNsensitive, False, NULL);
		XtVaSetValues(mb3dviewptr->mbview_toggleButton_mode_rrotate, XmNsensitive, False, NULL);
		XtVaSetValues(mb3dviewptr->mbview_toggleButton_mode_rviewpoint, XmNsensitive, False, NULL);
	}
	else {
		XtVaSetValues(mb3dviewptr->mbview_toggleButton_mode_rotate, XmNsensitive, True, NULL);
		XtVaSetValues(mb3dviewptr->mbview_toggleButton_mode_viewpoint, XmNsensitive, True, NULL);
		XtVaSetValues(mb3dviewptr->mbview_toggleButton_mode_rrotate, XmNsensitive, True, NULL);
		XtVaSetValues(mb3dviewptr->mbview_toggleButton_mode_rviewpoint, XmNsensitive, True, NULL);
	}

	/* set label */
	mb_path value_text;
	if (data->mouse_mode == MBV_MOUSE_MOVE)
    sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Pick\":t\"M: Pan\":t\"R: Zoom\"");
	else if (data->mouse_mode == MBV_MOUSE_ROTATE)
		sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Pick\":t\"M: Rotate\":t\"R:Exageration\"");
	else if (data->mouse_mode == MBV_MOUSE_SHADE)
		sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Pick\":t\"M: Light Source\":t\"R: Shade Magnitude\"");
	else if (data->mouse_mode == MBV_MOUSE_VIEWPOINT)
		sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Pick\":t\"M: View Rotate\":t\"R: Exageration\"");
	else if (data->mouse_mode == MBV_MOUSE_AREA)
		sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Drag Region\":t\"M: Drag Area\":t\"R: Area Width\"");
	else if (data->mouse_mode == MBV_MOUSE_SITE)
		sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Select Site\":t\"M: Add Site\":t\"R: Delete Site\"");
	else if (data->mouse_mode == MBV_MOUSE_ROUTE)
		sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Select Route\":t\"M: Add Route\":t\"R: Delete Route\"");
	else if (data->mouse_mode == MBV_MOUSE_NAV)
		sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Pick\":t\"M: Select Nav\":t\"R: Deselect Nav\"");
	else if (data->mouse_mode == MBV_MOUSE_NAVFILE)
		sprintf(value_text, ":::t\"Mouse Mode:\":t\"L: Pick\":t\"M: Select Nav File\":t\"R: Deselect Nav File\"");
	set_mbview_label_multiline_string(view->mb3dview.mbview_label_mouse, value_text);
}

/*------------------------------------------------------------------------------*/
void set_mbview_grid_mode(size_t instance, int mode) {
	if (mbv_verbose >= 2)
		fprintf(stderr, "set_mbview_grid_mode: instance:%zu mode:%d\n", instance, mode);

	struct mbview_world_struct *view = &(mbviews[instance]);

	MB3DViewData *mb3dviewptr = &(view->mb3dview);

	const Boolean value1 = mode == MBV_GRID_VIEW_PRIMARY;
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_data_primary, value1, False);

	const Boolean value2 = mode == MBV_GRID_VIEW_PRIMARYSLOPE;
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_data_primaryslope, value2, False);

	const Boolean value3 = mode == MBV_GRID_VIEW_SECONDARY;
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_data_secondary, value3, False);
}

/*------------------------------------------------------------------------------*/
void set_mbview_histogram_mode(size_t instance, bool mode) {
	if (mbv_verbose >= 2)
		fprintf(stderr, "set_mbview_histogram_mode: instance:%zu mode:%d\n", instance, mode);

	struct mbview_world_struct *view = &(mbviews[instance]);

	MB3DViewData *mb3dviewptr = &(view->mb3dview);

	const Boolean value = mode;
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_histogram, value, False);
}

/*------------------------------------------------------------------------------*/
void set_mbview_shade_mode(size_t instance, int mode) {
	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_shade_mode: instance:%zu mode:%d\n", instance, mode);

	struct mbview_world_struct *view = &(mbviews[instance]);

	MB3DViewData *mb3dviewptr = &(view->mb3dview);

	const Boolean value1 = mode == MBV_SHADE_VIEW_NONE;
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_overlay_none, value1, False);

	const Boolean value2 = mode == MBV_SHADE_VIEW_ILLUMINATION;
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_overlay_illumination, value2, False);

	const Boolean value3 = mode == MBV_SHADE_VIEW_SLOPE;
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_overlay_slope, value3, False);

	const Boolean value4 = mode == MBV_SHADE_VIEW_OVERLAY;
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_overlay_secondary, value4, False);
}

/*------------------------------------------------------------------------------*/
void set_mbview_contour_mode(size_t instance, int mode) {
	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_contour_mode: instance:%zu mode:%d\n", instance, mode);

	struct mbview_world_struct *view = &(mbviews[instance]);

	MB3DViewData *mb3dviewptr = &(view->mb3dview);
	const Boolean value = mode == MBV_VIEW_ON;
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_overlay_contour, value, False);
}

/*------------------------------------------------------------------------------*/
void set_mbview_site_view_mode(size_t instance, int mode) {
	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_site_view_mode: instance:%zu mode:%d\n", instance, mode);

	struct mbview_world_struct *view = &(mbviews[instance]);

	MB3DViewData *mb3dviewptr = &(view->mb3dview);
	const Boolean value = mode == MBV_VIEW_ON;
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_site, value, False);
}

/*------------------------------------------------------------------------------*/
void set_mbview_route_view_mode(size_t instance, int mode) {
	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_route_view_mode: instance:%zu mode:%d\n", instance, mode);

	struct mbview_world_struct *view = &(mbviews[instance]);

	MB3DViewData *mb3dviewptr = &(view->mb3dview);
	const Boolean value = mode == MBV_VIEW_ON;
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_route, value, False);
}

/*------------------------------------------------------------------------------*/
void set_mbview_nav_view_mode(size_t instance, int mode) {
	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_nav_view_mode: instance:%zu mode:%d\n", instance, mode);

	struct mbview_world_struct *view = &(mbviews[instance]);

	MB3DViewData *mb3dviewptr = &(view->mb3dview);
	Boolean value = mode == MBV_VIEW_ON;
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_nav, value, False);
}

/*------------------------------------------------------------------------------*/
void set_mbview_navdrape_view_mode(size_t instance, int mode) {
	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_nav_view_mode: instance:%zu mode:%d\n", instance, mode);

	struct mbview_world_struct *view = &(mbviews[instance]);

	MB3DViewData *mb3dviewptr = &(view->mb3dview);
	Boolean value = mode == MBV_VIEW_ON;
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_navdrape, value, False);
}

/*------------------------------------------------------------------------------*/
void set_mbview_vector_view_mode(size_t instance, int mode) {
	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_vector_view_mode: instance:%zu mode:%d\n", instance, mode);

	struct mbview_world_struct *view = &(mbviews[instance]);

	MB3DViewData *mb3dviewptr = &(view->mb3dview);
	const Boolean value = mode == MBV_VIEW_ON;
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_vector, value, False);
}

/*------------------------------------------------------------------------------*/
void set_mbview_display_mode(size_t instance, int mode) {
	if (mbv_verbose >= 2)
		fprintf(stderr, "set_mbview_display_mode: instance:%zu mode:%d\n", instance, mode);

	struct mbview_world_struct *view = &(mbviews[instance]);

	MB3DViewData *mb3dviewptr = &(view->mb3dview);

	const Boolean value1 = mode == MBV_DISPLAY_2D;
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_display_2D, value1, False);

	const Boolean value2 = mode == MBV_DISPLAY_3D;
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_display_3D, value2, False);
}
/*------------------------------------------------------------------------------*/
void set_mbview_colortable(size_t instance, int mode) {
	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_colortable: instance:%zu mode:%d\n", instance, mode);

	struct mbview_world_struct *view = &(mbviews[instance]);

	MB3DViewData *mb3dviewptr = &(view->mb3dview);

	const Boolean value1 = mode == MBV_COLORTABLE_HAXBY;
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_colortable_haxby, value1, False);

	const Boolean value2 = mode == MBV_COLORTABLE_BRIGHT;
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_colortable_bright, value2, False);

	const Boolean value3 = mode == MBV_COLORTABLE_MUTED;
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_colortable_muted, value3, False);

	const Boolean value4 = mode == MBV_COLORTABLE_GRAY;
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_colortable_gray, value4, False);

	const Boolean value5 = mode == MBV_COLORTABLE_FLAT;
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_colortable_flat, value5, False);

	const Boolean value6 = mode == MBV_COLORTABLE_SEALEVEL1;
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_colortable_sealevel1, value6, False);

	const Boolean value7 = mode == MBV_COLORTABLE_SEALEVEL2;
	XmToggleButtonSetState(mb3dviewptr->mbview_toggleButton_colortable_sealevel2, value7, False);
}

/*------------------------------------------------------------------------------*/
void set_mbview_colortable_mode(size_t instance, int mode) {
	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_colortable_mode: instance:%zu mode:%d\n", instance, mode);
}

/*------------------------------------------------------------------------------*/

void do_mbview_aboutpopdown(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_aboutpopdown: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);

	XtUnmanageChild(view->mb3dview.mbview_bulletinBoard_about);
}
/*------------------------------------------------------------------------------*/

void do_mbview_aboutpopup(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_aboutpopup: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);

	XtManageChild(view->mb3dview.mbview_bulletinBoard_about);
}

/*------------------------------------------------------------------------------*/

void do_mbview_colorboundspopup(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_colorboundspopup: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	XtManageChild(view->mb3dview.mbview_bulletinBoard_colorbounds);

	mb_path value_text;

	/* set values of widgets */
	sprintf(value_text, "%g", data->primary_colortable_min);
	XmTextFieldSetString(view->mb3dview.mbview_textField_datamin, value_text);
	sprintf(value_text, "%g", data->primary_colortable_max);
	XmTextFieldSetString(view->mb3dview.mbview_textField_datamax, value_text);
	sprintf(value_text, "%g", data->contour_interval);
	XmTextFieldSetString(view->mb3dview.mbview_textField_contours, value_text);
	if (data->primary_colortable_mode == MBV_COLORTABLE_NORMAL) {
		XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_data_ctoh, TRUE, TRUE);
	}
	else {
		XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_data_htoc, TRUE, TRUE);
	}
	sprintf(value_text, "%g", data->slope_colortable_min);
	XmTextFieldSetString(view->mb3dview.mbview_textField_slopemin, value_text);
	sprintf(value_text, "%g", data->slope_colortable_max);
	XmTextFieldSetString(view->mb3dview.mbview_textField_slopemax, value_text);
	if (data->slope_colortable_mode == MBV_COLORTABLE_NORMAL) {
		XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_slope_ctoh, TRUE, TRUE);
	}
	else {
		XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_slope_htoc, TRUE, TRUE);
	}
	sprintf(value_text, "%g", data->secondary_colortable_min);
	XmTextFieldSetString(view->mb3dview.mbview_textField_overlaymin, value_text);
	sprintf(value_text, "%g", data->secondary_colortable_max);
	XmTextFieldSetString(view->mb3dview.mbview_textField_overlaymax, value_text);
	if (data->secondary_colortable_mode == MBV_COLORTABLE_NORMAL) {
		XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_overlay_ctoh, TRUE, TRUE);
	}
	else {
		XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_overlay_htoc, TRUE, TRUE);
	}
}
/*------------------------------------------------------------------------------*/

void do_mbview_colorboundspopdown(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_colorboundspopdown: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);

	XtUnmanageChild(view->mb3dview.mbview_bulletinBoard_colorbounds);
}
/*------------------------------------------------------------------------------*/

void do_mbview_colorboundsapply(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_colorboundsapply: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	mb_path value_text;

	/* get values of widgets */

	bool change = false;

	get_mbview_text_string(view->mb3dview.mbview_textField_datamin, value_text);
	double dvalue;
	sscanf(value_text, "%lf", &dvalue);
	if (dvalue != data->primary_colortable_min) {
		data->primary_colortable_min = dvalue;
		if (data->grid_mode == MBV_GRID_VIEW_PRIMARY)
			change = true;
	}

	get_mbview_text_string(view->mb3dview.mbview_textField_datamax, value_text);
	sscanf(value_text, "%lf", &dvalue);
	if (dvalue != data->primary_colortable_max) {
		data->primary_colortable_max = dvalue;
		if (data->grid_mode == MBV_GRID_VIEW_PRIMARY)
			change = true;
	}

	get_mbview_text_string(view->mb3dview.mbview_textField_contours, value_text);
	sscanf(value_text, "%lf", &dvalue);
	if (dvalue != data->contour_interval) {
		data->contour_interval = dvalue;
		view->contourlorez = false;
		view->contourhirez = false;
		view->contourfullrez = false;
		view->primary_histogram_set = false;
		view->primaryslope_histogram_set = false;
		view->secondary_histogram_set = false;
		if (data->grid_contour_mode == MBV_VIEW_ON)
			change = true;
	}

	int ivalue = XmToggleButtonGetState(view->mb3dview.mbview_toggleButton_data_htoc);
	if (ivalue != data->primary_colortable_mode) {
		data->primary_colortable_mode = ivalue;
		if (data->grid_mode == MBV_GRID_VIEW_PRIMARY)
			change = true;
	}

	get_mbview_text_string(view->mb3dview.mbview_textField_slopemin, value_text);
	sscanf(value_text, "%lf", &dvalue);
	if (dvalue != data->slope_colortable_min) {
		data->slope_colortable_min = dvalue;
		if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE)
			change = true;
	}

	get_mbview_text_string(view->mb3dview.mbview_textField_slopemax, value_text);
	sscanf(value_text, "%lf", &dvalue);
	if (dvalue != data->slope_colortable_max) {
		data->slope_colortable_max = dvalue;
		if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE)
			change = true;
	}

	ivalue = XmToggleButtonGetState(view->mb3dview.mbview_toggleButton_slope_htoc);
	if (ivalue != data->slope_colortable_mode) {
		data->slope_colortable_mode = ivalue;
		if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE)
			change = true;
	}

	get_mbview_text_string(view->mb3dview.mbview_textField_overlaymin, value_text);
	sscanf(value_text, "%lf", &dvalue);
	if (dvalue != data->secondary_colortable_min) {
		data->secondary_colortable_min = dvalue;
		if (data->grid_mode == MBV_GRID_VIEW_SECONDARY)
			change = true;
	}

	get_mbview_text_string(view->mb3dview.mbview_textField_overlaymax, value_text);
	sscanf(value_text, "%lf", &dvalue);
	if (dvalue != data->secondary_colortable_max) {
		data->secondary_colortable_max = dvalue;
		if (data->grid_mode == MBV_GRID_VIEW_SECONDARY)
			change = true;
	}

	ivalue = XmToggleButtonGetState(view->mb3dview.mbview_toggleButton_overlay_htoc);
	if (ivalue != data->secondary_colortable_mode) {
		data->secondary_colortable_mode = ivalue;
		if (data->grid_mode == MBV_GRID_VIEW_SECONDARY)
			change = true;
	}

	/* clear color status array */
	if (change) {
		view->lastdrawrez = MBV_REZ_NONE;
		mbview_setcolorparms(instance);
		mbview_colorclear(instance);
	}

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_colorboundsapply\n");
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);
}
/*------------------------------------------------------------------------------*/

void do_mbview_shadeparmspopup(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	if (mbv_verbose >= 0)
		fprintf(stderr, "do_mbview_shadeparmspopup: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	XtManageChild(view->mb3dview.mbview_bulletinBoard_shadeparms);

	mb_path value_text;

	/* set values of widgets */
	sprintf(value_text, "%g", data->illuminate_magnitude);
	XmTextFieldSetString(view->mb3dview.mbview_textField_illum_amp, value_text);
	sprintf(value_text, "%g", data->illuminate_azimuth);
	XmTextFieldSetString(view->mb3dview.mbview_textField_illum_azi, value_text);
	sprintf(value_text, "%g", data->illuminate_elevation);
	XmTextFieldSetString(view->mb3dview.mbview_textField_illum_elev, value_text);
	sprintf(value_text, "%g", data->slope_magnitude);
	XmTextFieldSetString(view->mb3dview.mbview_textField_slope_amp, value_text);
	sprintf(value_text, "%g", data->overlay_shade_magnitude);
	XmTextFieldSetString(view->mb3dview.mbview_textField_overlay_amp, value_text);
	sprintf(value_text, "%g", data->overlay_shade_center);
	XmTextFieldSetString(view->mb3dview.mbview_textField_overlay_center, value_text);
	if (data->overlay_shade_mode == MBV_COLORTABLE_NORMAL) {
		XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_overlay_shade_ctoh, TRUE, TRUE);
	}
	else {
		XmToggleButtonSetState(view->mb3dview.mbview_toggleButton_overlay_shade_htoc, TRUE, TRUE);
	}
}
/*------------------------------------------------------------------------------*/

void do_mbview_shadeparmspopdown(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_shadeparmspopdown: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);

	XtUnmanageChild(view->mb3dview.mbview_bulletinBoard_shadeparms);
}
/*------------------------------------------------------------------------------*/

void do_mbview_shadeparmsapply(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_shadeparmsapply: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	int shade_mode;
	/* check current shading mode */
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY)
		shade_mode = data->primary_shade_mode;
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE)
		shade_mode = data->slope_shade_mode;
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY)
		shade_mode = data->secondary_shade_mode;

	/* get values of widgets */

	/* handle illumination */
	bool change = false;

	mb_path value_text;
	get_mbview_text_string(view->mb3dview.mbview_textField_illum_amp, value_text);
	double dvalue;
	sscanf(value_text, "%lf", &dvalue);
	if (dvalue != data->illuminate_magnitude) {
		data->illuminate_magnitude = dvalue;
		if (shade_mode == MBV_SHADE_VIEW_ILLUMINATION)
			change = true;
	}

	get_mbview_text_string(view->mb3dview.mbview_textField_illum_azi, value_text);
	sscanf(value_text, "%lf", &dvalue);
	if (dvalue != data->illuminate_azimuth) {
		data->illuminate_azimuth = dvalue;
		if (shade_mode == MBV_SHADE_VIEW_ILLUMINATION)
			change = true;
	}

	get_mbview_text_string(view->mb3dview.mbview_textField_illum_elev, value_text);
	sscanf(value_text, "%lf", &dvalue);
	if (dvalue != data->illuminate_elevation) {
		data->illuminate_elevation = dvalue;
		if (shade_mode == MBV_SHADE_VIEW_ILLUMINATION)
			change = true;
	}

	get_mbview_text_string(view->mb3dview.mbview_textField_slope_amp, value_text);
	sscanf(value_text, "%lf", &dvalue);
	if (dvalue != data->slope_magnitude) {
		data->slope_magnitude = dvalue;
		if (shade_mode == MBV_SHADE_VIEW_SLOPE)
			change = true;
	}

	get_mbview_text_string(view->mb3dview.mbview_textField_overlay_amp, value_text);
	sscanf(value_text, "%lf", &dvalue);
	if (dvalue != data->overlay_shade_magnitude) {
		data->overlay_shade_magnitude = dvalue;
		if (shade_mode == MBV_SHADE_VIEW_OVERLAY)
			change = true;
	}

	get_mbview_text_string(view->mb3dview.mbview_textField_overlay_center, value_text);
	sscanf(value_text, "%lf", &dvalue);
	if (dvalue != data->overlay_shade_center) {
		data->overlay_shade_center = dvalue;
		if (shade_mode == MBV_SHADE_VIEW_OVERLAY)
			change = true;
	}

	int ivalue = XmToggleButtonGetState(view->mb3dview.mbview_toggleButton_overlay_shade_ctoh);
	if (ivalue != data->overlay_shade_mode) {
		data->overlay_shade_mode = ivalue;
		if (shade_mode == MBV_SHADE_VIEW_OVERLAY)
			change = true;
	}

	/* clear color status array */
	if (change) {
		view->lastdrawrez = MBV_REZ_NONE;
		mbview_setcolorparms(instance);
		mbview_colorclear(instance);
	}

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_shadeparmsapply\n");
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);
}
/*------------------------------------------------------------------------------*/

int do_mbview_3dparmstext(size_t instance) {
	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_3dparmstext: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	mb_path value_text;

	/* set values of widgets */
	sprintf(value_text, "%g", data->modelazimuth3d);
	XmTextFieldSetString(view->mb3dview.mbview_textField_model_azimuth, value_text);
	sprintf(value_text, "%g", data->modelelevation3d);
	XmTextFieldSetString(view->mb3dview.mbview_textField_model_elevation, value_text);
	sprintf(value_text, "%g", data->viewazimuth3d);
	XmTextFieldSetString(view->mb3dview.mbview_textField_view_azimuth, value_text);
	sprintf(value_text, "%g", data->viewelevation3d);
	XmTextFieldSetString(view->mb3dview.mbview_textField_view_elevation, value_text);
	sprintf(value_text, "%g", data->exageration);
	XmTextFieldSetString(view->mb3dview.mbview_textField_exageration, value_text);
	sprintf(value_text, "%g", view->offset3d_x);
	XmTextFieldSetString(view->mb3dview.mbview_textField_view_3doffsetx, value_text);
	sprintf(value_text, "%g", view->offset3d_y);
	XmTextFieldSetString(view->mb3dview.mbview_textField_view_3doffsety, value_text);
	sprintf(value_text, "%g", view->offset3d_z);
	XmTextFieldSetString(view->mb3dview.mbview_textField_model_3dzoom, value_text);
	sprintf(value_text, "%g", view->viewoffset3d_z);
	XmTextFieldSetString(view->mb3dview.mbview_textField_view_3dzoom, value_text);

	return (0);
}
/*------------------------------------------------------------------------------*/

void do_mbview_3dparmspopup(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_3dparmspopup: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);

	XtManageChild(view->mb3dview.mbview_bulletinBoard_3dparms);

	/* set values of widgets */
	do_mbview_3dparmstext(instance);
}
/*------------------------------------------------------------------------------*/

void do_mbview_3dparmspopdown(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_3dparmspopdown: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);

	XtUnmanageChild(view->mb3dview.mbview_bulletinBoard_3dparms);
}
/*------------------------------------------------------------------------------*/

void do_mbview_3dparmsapply(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_3dparmsapply: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get values of widgets */

	bool change = false;
	mb_path value_text;

	get_mbview_text_string(view->mb3dview.mbview_textField_model_azimuth, value_text);
	double dvalue;
	sscanf(value_text, "%lf", &dvalue);
	if (dvalue != data->modelazimuth3d) {
		data->modelazimuth3d = dvalue;
		change = true;
	}

	get_mbview_text_string(view->mb3dview.mbview_textField_model_elevation, value_text);
	sscanf(value_text, "%lf", &dvalue);
	if (dvalue != data->modelelevation3d) {
		data->modelelevation3d = dvalue;
		change = true;
	}

	get_mbview_text_string(view->mb3dview.mbview_textField_view_azimuth, value_text);
	sscanf(value_text, "%lf", &dvalue);
	if (dvalue != data->viewazimuth3d) {
		data->viewazimuth3d = dvalue;
		change = true;
	}

	get_mbview_text_string(view->mb3dview.mbview_textField_view_elevation, value_text);
	sscanf(value_text, "%lf", &dvalue);
	if (dvalue != data->viewelevation3d) {
		data->viewelevation3d = dvalue;
		change = true;
	}

	get_mbview_text_string(view->mb3dview.mbview_textField_exageration, value_text);
	sscanf(value_text, "%lf", &dvalue);
	if (dvalue != data->exageration) {
		data->exageration = dvalue;
		if (data->display_projection_mode != MBV_PROJECTION_SPHEROID) {
			view->zorigin = data->exageration * 0.5 * (data->primary_min + data->primary_max);
		}
		change = true;

		mbview_zscaleclear(instance);
		view->contourlorez = false;
		view->contourhirez = false;
		view->contourfullrez = false;

		/* rescale data other than the grid */
		mbview_zscale(instance);

		/* set flag to reset view bounds */
		view->viewboundscount++;
	}

	get_mbview_text_string(view->mb3dview.mbview_textField_view_3doffsetx, value_text);
	sscanf(value_text, "%lf", &dvalue);
	if (dvalue != view->offset3d_x) {
		view->offset3d_x = dvalue;
		change = true;
	}

	get_mbview_text_string(view->mb3dview.mbview_textField_view_3doffsety, value_text);
	sscanf(value_text, "%lf", &dvalue);
	if (dvalue != view->offset3d_y) {
		view->offset3d_y = dvalue;
		change = true;
	}

	get_mbview_text_string(view->mb3dview.mbview_textField_model_3dzoom, value_text);
	sscanf(value_text, "%lf", &dvalue);
	if (dvalue != view->offset3d_z) {
		view->offset3d_z = dvalue;
		change = true;
	}

	get_mbview_text_string(view->mb3dview.mbview_textField_view_3dzoom, value_text);
	sscanf(value_text, "%lf", &dvalue);
	if (dvalue != view->viewoffset3d_z) {
		view->viewoffset3d_z = dvalue;
		change = true;
	}

	/* clear color status array */
	if (change && data->display_mode == MBV_DISPLAY_3D) {
		view->lastdrawrez = MBV_REZ_NONE;
		mbview_setcolorparms(instance);
		mbview_colorclear(instance);
	}

	/* set flag to reset view bounds */
	view->viewboundscount = MBV_BOUNDSFREQUENCY;

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_3dparmsapply\n");
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);
}
/*------------------------------------------------------------------------------*/

int do_mbview_2dparmstext(size_t instance) {
	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_2dparmstext: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);

	mb_path value_text;

	/* set values of widgets */
	sprintf(value_text, "%g", view->offset2d_x);
	XmTextFieldSetString(view->mb3dview.mbview_textField_view_2doffsetx, value_text);
	sprintf(value_text, "%g", view->offset2d_y);
	XmTextFieldSetString(view->mb3dview.mbview_textField_view_2doffsety, value_text);
	sprintf(value_text, "%g", view->size2d);
	XmTextFieldSetString(view->mb3dview.mbview_textField_view_2dzoom, value_text);

	return (0);
}
/*------------------------------------------------------------------------------*/

void do_mbview_2dparmspopup(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_2dparmspopup: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);

	XtManageChild(view->mb3dview.mbview_bulletinBoard_2dparms);

	/* set values of widgets */
	do_mbview_2dparmstext(instance);
}
/*------------------------------------------------------------------------------*/

void do_mbview_2dparmspopdown(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_2dparmspopdown: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);

	XtUnmanageChild(view->mb3dview.mbview_bulletinBoard_2dparms);
}
/*------------------------------------------------------------------------------*/

void do_mbview_2dparmsapply(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_2dparmsapply: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get values of widgets */

	bool change = false;
	mb_path value_text;

	get_mbview_text_string(view->mb3dview.mbview_textField_view_2doffsetx, value_text);
	double dvalue;
	sscanf(value_text, "%lf", &dvalue);
	if (dvalue != view->offset2d_x) {
		view->offset2d_x = dvalue;
		change = true;
	}

	get_mbview_text_string(view->mb3dview.mbview_textField_view_2doffsety, value_text);
	sscanf(value_text, "%lf", &dvalue);
	if (dvalue != view->offset2d_y) {
		view->offset2d_y = dvalue;
		change = true;
	}

	get_mbview_text_string(view->mb3dview.mbview_textField_view_2dzoom, value_text);
	sscanf(value_text, "%lf", &dvalue);
	if (dvalue != view->size2d) {
		view->size2d = dvalue;
		change = true;
	}

	/* clear color status array */
	if (change && data->display_mode == MBV_DISPLAY_2D) {
		view->lastdrawrez = MBV_REZ_NONE;
		mbview_setcolorparms(instance);
		mbview_colorclear(instance);
	}

	/* set flag to reset view bounds */
	view->viewboundscount = MBV_BOUNDSFREQUENCY;

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_2dparmsapply\n");
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);
}
/*------------------------------------------------------------------------------*/

void do_mbview_resolutionpopup(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_resolutionpopup: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	XtManageChild(view->mb3dview.mbview_bulletinBoard_resolution);

	/* set values of resolution sliders */
	XtVaSetValues(view->mb3dview.mbview_scale_lowresolution, XmNvalue, data->lorez_dimension, NULL);
	XtVaSetValues(view->mb3dview.mbview_scale_mediumresolution, XmNvalue, data->hirez_dimension, NULL);
	XtVaSetValues(view->mb3dview.mbview_scale_navlowresolution, XmNvalue, data->lorez_navdecimate, NULL);
	XtVaSetValues(view->mb3dview.mbview_scale_navmediumresolution, XmNvalue, data->hirez_navdecimate, NULL);
}
/*------------------------------------------------------------------------------*/

void do_mbview_resolutionpopdown(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_resolutionpopdown: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);

	XtUnmanageChild(view->mb3dview.mbview_bulletinBoard_resolution);
}
/*------------------------------------------------------------------------------*/
void do_mbview_resolutionchange(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_resolutionchange: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get values of resolution sliders */
	int lorez_dimension;
	int hirez_dimension;
	XtVaGetValues(view->mb3dview.mbview_scale_lowresolution, XmNvalue, &lorez_dimension, NULL);
	XtVaGetValues(view->mb3dview.mbview_scale_mediumresolution, XmNvalue, &hirez_dimension, NULL);
	int lorez_navdecimate;
	int hirez_navdecimate;
	XtVaGetValues(view->mb3dview.mbview_scale_navlowresolution, XmNvalue, &lorez_navdecimate, NULL);
	XtVaGetValues(view->mb3dview.mbview_scale_navmediumresolution, XmNvalue, &hirez_navdecimate, NULL);

	/* make dimensions even multiples of 10 */
	if (lorez_dimension > hirez_dimension)
		hirez_dimension = lorez_dimension;
	data->lorez_dimension = 25 * ((int)((lorez_dimension + 12.5) / 25));
	data->hirez_dimension = 25 * ((int)((hirez_dimension + 12.5) / 25));

	/* set values of resolution sliders */
	XtVaSetValues(view->mb3dview.mbview_scale_lowresolution, XmNvalue, data->lorez_dimension, NULL);
	XtVaSetValues(view->mb3dview.mbview_scale_mediumresolution, XmNvalue, data->hirez_dimension, NULL);

	/* set nav decimation */
	data->lorez_navdecimate = lorez_navdecimate;
	data->hirez_navdecimate = hirez_navdecimate;

	/* reset status flags and arrays */
	view->lastdrawrez = MBV_REZ_NONE;
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);
	mbview_zscaleclear(instance);
	view->contourlorez = false;
	view->contourhirez = false;
	view->contourfullrez = false;

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_resolutionchange instance:%zu resolutions: %d %d decimations: %d %d\n", instance,
		        data->lorez_dimension, data->hirez_dimension, data->lorez_navdecimate, data->hirez_navdecimate);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_resolutionchange\n");
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);
}

/*------------------------------------------------------------------------------*/

void do_mbview_sitelistpopup(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_sitelistpopup: \n");

	XtPopup(XtParent(shared.mainWindow_sitelist), XtGrabNone);
	shared.init_sitelist = MBV_WINDOW_VISIBLE;
	mbview_updatesitelist();
}
/*------------------------------------------------------------------------------*/

void do_mbview_routelistpopup(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_routelistpopup:\n");

	XtPopup(XtParent(shared.mainWindow_routelist), XtGrabNone);
	shared.init_routelist = MBV_WINDOW_VISIBLE;
	mbview_updateroutelist();
}
/*------------------------------------------------------------------------------*/

void do_mbview_navlistpopup(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_navlistpopup:\n");

	XtPopup(XtParent(shared.mainWindow_navlist), XtGrabNone);
	shared.init_navlist = MBV_WINDOW_VISIBLE;
	mbview_updatenavlist();
}
/*------------------------------------------------------------------------------*/

void do_mbview_sitelistselect(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_sitelistselect:\n");

	/* get position of selected list item */
	Arg args[256];
  Cardinal ac = 0;
	int position_count = 0;
	XtSetArg(args[ac], XmNselectedPositionCount, (XtPointer)&position_count);
	ac++;
	int *position_list = NULL;
	XtSetArg(args[ac], XmNselectedPositions, (XtPointer)&position_list);
	ac++;
	XtGetValues(w, args, ac);

	/* save last site selection if any */
	int site_selected_old = shared.shareddata.site_selected;

	/* find selected site point if any */
	shared.shareddata.site_selected = MBV_SELECT_NONE;
	if (position_count == 1) {
		shared.shareddata.site_selected = position_list[0] - 1;
	}

	/* change site color if clicked more than once */
	if (site_selected_old == shared.shareddata.site_selected) {
		const int isite = shared.shareddata.site_selected;

		/* increment color */
		shared.shareddata.sites[isite].color++;
		if (shared.shareddata.sites[isite].color == MBV_COLOR_RED)
			shared.shareddata.sites[isite].color++;
		if (shared.shareddata.sites[isite].color > MBV_COLOR_PURPLE)
			shared.shareddata.sites[isite].color = MBV_COLOR_BLACK;

		/* update site list */
		mbview_updatesitelist();
	}

	/* redraw valid instances */
	size_t instance = MBV_NO_WINDOW;
	for (size_t i = 0; i < MBV_MAX_WINDOWS; i++) {
		if (mbviews[i].data.active) {
			/* set instance to first good instance */
			if (instance == MBV_NO_WINDOW)
				instance = i;

			/* set pick annotation */
			mbviews[i].data.pickinfo_mode = MBV_PICK_SITE;
			mbview_pick_text(i);
			/* draw */
			if (mbv_verbose >= 2)
				fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_sitelistselect instance:%zu\n", instance);
			mbview_plotlowhigh(i);
			mbview_plotlowhighall(i);
		}
	}
}
/*------------------------------------------------------------------------------*/

void do_mbview_routelistselect(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_routelistselect:\n");

	/* get position of selected list item */
	Arg args[256];
  Cardinal ac = 0;
	int position_count = 0;
	XtSetArg(args[ac], XmNselectedPositionCount, (XtPointer)&position_count);
	ac++;
	int *position_list = NULL;
	XtSetArg(args[ac], XmNselectedPositions, (XtPointer)&position_list);
	ac++;
	XtGetValues(shared.mb3d_routelist.mbview_list_routelist, args, ac);

	/* save last route selection if any */
	const int route_selected_old = shared.shareddata.route_selected;
	const int route_point_selected_old = shared.shareddata.route_point_selected;

	/* find selected route point if any */
	shared.shareddata.route_selected = MBV_SELECT_NONE;
	shared.shareddata.route_point_selected = MBV_SELECT_NONE;
	if (position_count == 1) {
		const int iposition = position_list[0] - 1;
		int iroutepos = 0;
		for (int iroute = 0; iroute < shared.shareddata.nroute; iroute++) {
			if (iroutepos == iposition) {
				shared.shareddata.route_selected = iroute;
				shared.shareddata.route_point_selected = MBV_SELECT_ALL;
			}
			else if (iroutepos < iposition && iroutepos + shared.shareddata.routes[iroute].npoints >= iposition) {
				shared.shareddata.route_selected = iroute;
				shared.shareddata.route_point_selected = iposition - iroutepos - 1;
			}
			iroutepos += shared.shareddata.routes[iroute].npoints + 1;
		}

		/* change route color if clicked more than once */
		if (route_selected_old == shared.shareddata.route_selected && route_point_selected_old == MBV_SELECT_ALL &&
		    shared.shareddata.route_point_selected == MBV_SELECT_ALL) {
			const int iroute = shared.shareddata.route_selected;

			/* increment color */
			shared.shareddata.routes[iroute].color++;
			if (shared.shareddata.routes[iroute].color == MBV_COLOR_RED)
				shared.shareddata.routes[iroute].color++;
			if (shared.shareddata.routes[iroute].color > MBV_COLOR_PURPLE)
				shared.shareddata.routes[iroute].color = MBV_COLOR_BLACK;

			/* update route list */
			mbview_updateroutelist();
		}

		/* change waypoint type if waypoint clicked more than once */
		if (route_selected_old == shared.shareddata.route_selected &&
		    route_point_selected_old == shared.shareddata.route_point_selected) {
			const int iroute = shared.shareddata.route_selected;
			const int iwaypoint = shared.shareddata.route_point_selected;

			/* increment waypoint type */
			shared.shareddata.routes[iroute].waypoint[iwaypoint]++;
			if (shared.shareddata.routes[iroute].waypoint[iwaypoint] < MBV_ROUTE_WAYPOINT_SIMPLE ||
			    shared.shareddata.routes[iroute].waypoint[iwaypoint] > MBV_ROUTE_WAYPOINT_ENDLINE5)
				shared.shareddata.routes[iroute].waypoint[iwaypoint] = MBV_ROUTE_WAYPOINT_SIMPLE;

			/* update route list */
			mbview_updateroutelist();
		}
	}

	/* redraw valid instances */
	size_t instance = MBV_NO_WINDOW;
	for (size_t i = 0; i < MBV_MAX_WINDOWS; i++) {
		if (mbviews[i].data.active) {
			/* set instance to first good instance */
			if (instance == MBV_NO_WINDOW)
				instance = i;

			/* set pick annotation */
			mbviews[i].data.pickinfo_mode = MBV_PICK_ROUTE;
			mbview_pick_text(i);

			/* draw */
			if (mbv_verbose >= 2)
				fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_routelistselect\n");
			mbview_plotlowhigh(i);
			mbview_plotlowhighall(i);
		}
	}
}
/*------------------------------------------------------------------------------*/

void do_mbview_navlistselect(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_navlistselect:\n");

	/* get position of selected list item */
	Arg args[256];
  Cardinal ac = 0;
	int position_count = 0;
	XtSetArg(args[ac], XmNselectedPositionCount, (XtPointer)&position_count);
	ac++;
	int *position_list = NULL;
	XtSetArg(args[ac], XmNselectedPositions, (XtPointer)&position_list);
	ac++;
	XtGetValues(shared.mb3d_navlist.mbview_list_navlist, args, ac);

	/* first deselect all navigation */
	shared.shareddata.navpick_type = MBV_PICK_NONE;
	shared.shareddata.nav_selected[0] = MBV_SELECT_NONE;
	shared.shareddata.nav_point_selected[0] = MBV_SELECT_NONE;
	shared.shareddata.nav_selected[1] = MBV_SELECT_NONE;
	shared.shareddata.nav_point_selected[1] = MBV_SELECT_NONE;
	for (int inav = 0; inav < shared.shareddata.nnav; inav++) {
		shared.shareddata.navs[inav].nselected = 0;
		for (int jpt = 0; jpt < shared.shareddata.navs[inav].npoints; jpt++) {
			shared.shareddata.navs[inav].navpts[jpt].selected = false;
		}
	}

	/* now select all nav points in selected files */
	for (int j = 0; j < position_count; j++) {
		int inav = position_list[j] - 1;
		if (shared.shareddata.navs[inav].npoints > 0) {

			/* Select all nav points in inav */
			for (int jpt = 0; jpt < shared.shareddata.navs[inav].npoints; jpt++) {
				shared.shareddata.navs[inav].navpts[jpt].selected = true;
				shared.shareddata.navs[inav].nselected++;
			}

			/* pick first and last navigation points */
			if (j == 0) {
				shared.shareddata.navpick_type = MBV_PICK_ONEPOINT;
				shared.shareddata.nav_selected[0] = inav;
				shared.shareddata.nav_point_selected[0] = 0;
				shared.shareddata.navpick.endpoints[0] =
				    shared.shareddata.navs[inav].navpts[shared.shareddata.nav_point_selected[0]].point;
			}
			if (j == position_count - 1) {
				shared.shareddata.navpick_type = MBV_PICK_TWOPOINT;
				shared.shareddata.nav_selected[1] = inav;
				shared.shareddata.nav_point_selected[1] = shared.shareddata.navs[inav].npoints - 1;
				shared.shareddata.navpick.endpoints[1] =
				    shared.shareddata.navs[inav].navpts[shared.shareddata.nav_point_selected[1]].point;
			}
		}
	}

	/* only call it a two point pick if the two points are different */
	if (shared.shareddata.navpick_type == MBV_PICK_TWOPOINT &&
	    shared.shareddata.nav_selected[0] == shared.shareddata.nav_selected[1] &&
	    shared.shareddata.nav_point_selected[0] == shared.shareddata.nav_point_selected[1])
		shared.shareddata.navpick_type = MBV_PICK_ONEPOINT;

	/* redraw valid instances */
	size_t instance = MBV_NO_WINDOW;
	for (size_t i = 0; i < MBV_MAX_WINDOWS; i++) {
		if (mbviews[i].data.active) {
			/* set instance to first good instance */
			if (instance == MBV_NO_WINDOW)
				instance = i;

			/* generate 3D drape of pick marks */
			mbview_navpicksize(i);

			/* set pick annotation */
			mbviews[i].data.pickinfo_mode = MBV_PICK_NAV;
			mbview_pick_text(i);

			/* draw */
			if (mbv_verbose >= 2)
				fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_navlistselect: instance:%zu\n", i);
			mbview_plotlowhigh(i);
			mbview_plotlowhighall(i);

			/* extract profile if pick is right type */
			if (mbviews[i].data.pickinfo_mode == MBV_PICK_NAV)
				mbview_extract_nav_profile(i);

			/* now replot profile */
			mbview_plotprofile(i);
		}
	}

	/* call picknav notify if defined */
	if (instance != MBV_NO_WINDOW
    && mbviews[instance].data.mbview_picknav_notify != NULL
    && shared.shareddata.navpick_type != MBV_PICK_NONE) {
		(mbviews[instance].data.mbview_picknav_notify)(instance);
	}

	/* set widget sensitivity */
	mbview_action_sensitivityall();
}

/*------------------------------------------------------------------------------*/
void do_mbview_sitelist_delete(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_sitelist_delete:\n");

	/* get position of selected list item */
	Arg args[256];
  Cardinal ac = 0;
	int position_count = 0;
	XtSetArg(args[ac], XmNselectedPositionCount, (XtPointer)&position_count);
	ac++;
	int *position_list = NULL;
	XtSetArg(args[ac], XmNselectedPositions, (XtPointer)&position_list);
	ac++;
	XtGetValues(shared.mb3d_sitelist.mbview_list_sitelist, args, ac);

	/* deselect any selected site */
	shared.shareddata.site_selected = MBV_SELECT_NONE;

	/* get first valid instance */
	size_t instance = MBV_NO_WINDOW;
	for (int i = 0; i < MBV_MAX_WINDOWS; i++) {
		if (instance == MBV_NO_WINDOW && mbviews[i].data.active)
			instance = i;
	}

	/* delete selected site points in reverse order if any */
	for (int i = position_count - 1; i >= 0; i--) {
		const int isite = position_list[i] - 1;
		mbview_site_delete(instance, isite);
	}

	/* reset pick annotation */
	if (position_count > 0) {
		for (int i = 0; i < MBV_MAX_WINDOWS; i++) {
			if (mbviews[i].data.active) {
				/* set pick annotation */
				if (mbviews[i].data.pickinfo_mode == MBV_PICK_SITE)
					mbviews[i].data.pickinfo_mode = MBV_PICK_NONE;
				mbview_pick_text(i);

				/* draw */
				if (mbv_verbose >= 2)
					fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_sitelist_delete\n");
				mbview_plotlowhigh(i);
				mbview_plotlowhighall(i);
			}
		}

		/* update site list */
		mbview_updatesitelist();
	}

	/* set widget sensitivity */
	mbview_action_sensitivityall();
}

/*------------------------------------------------------------------------------*/
void do_mbview_routelist_delete(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_routelist_delete:\n");

	/* get position of selected list item */
	Arg args[256];
  Cardinal ac = 0;
	int position_count = 0;
	XtSetArg(args[ac], XmNselectedPositionCount, (XtPointer)&position_count);
	ac++;
	int *position_list = NULL;
	XtSetArg(args[ac], XmNselectedPositions, (XtPointer)&position_list);
	ac++;
	XtGetValues(shared.mb3d_routelist.mbview_list_routelist, args, ac);

	/* deselect any selected route */
	shared.shareddata.route_selected = MBV_SELECT_NONE;

	/* get first valid instance */
	size_t instance = MBV_NO_WINDOW;
	for (int i = 0; i < MBV_MAX_WINDOWS; i++) {
		if (instance == MBV_NO_WINDOW && mbviews[i].data.active)
			instance = i;
	}

	/* figure out which routes and waypoints are selected,
	    and flag them for deletion by settting waypoint
	    values to MBV_ROUTE_WAYPOINT_DELETEFLAG */
	for (int i = 0; i < position_count; i++) {
		int iposition = 0;
		bool done = false;
		for (int iroute = 0; iroute < shared.shareddata.nroute && !done; iroute++) {
			iposition++;

			/* delete entire route */
			if (iposition == position_list[i]) {
				for (int jwaypoint = 0; jwaypoint < shared.shareddata.routes[iroute].npoints; jwaypoint++) {
					shared.shareddata.routes[iroute].waypoint[jwaypoint] = MBV_ROUTE_WAYPOINT_DELETEFLAG;
				}
				done = true;
			} else {
				/* else check waypoints */
				for (int jwaypoint = 0; jwaypoint < shared.shareddata.routes[iroute].npoints && !done; jwaypoint++) {
					iposition++;
					if (iposition == position_list[i]) {
						shared.shareddata.routes[iroute].waypoint[jwaypoint] = MBV_ROUTE_WAYPOINT_DELETEFLAG;
						done = true;
					}
				}
			}
		}
	}

	/* now loop over all route waypoints backwards, deleting any that have been flagged */
	for (int iroute = shared.shareddata.nroute - 1; iroute >= 0; iroute--) {
		for (int jwaypoint = shared.shareddata.routes[iroute].npoints - 1; jwaypoint >= 0; jwaypoint--) {
			if (shared.shareddata.routes[iroute].waypoint[jwaypoint] == MBV_ROUTE_WAYPOINT_DELETEFLAG)
				mbview_route_delete(instance, iroute, jwaypoint);
		}
	}

	/* reset pick annotation */
	if (position_count > 0) {
		for (int i = 0; i < MBV_MAX_WINDOWS; i++) {
			if (mbviews[i].data.active) {
				/* set pick annotation */
				if (mbviews[i].data.pickinfo_mode == MBV_PICK_ROUTE)
					mbviews[i].data.pickinfo_mode = MBV_PICK_NONE;
				mbview_pick_text(i);

				/* draw */
				if (mbv_verbose >= 2)
					fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_routelist_delete\n");
				mbview_plotlowhigh(i);
				mbview_plotlowhighall(i);
			}
		}

		/* update route list */
		mbview_updateroutelist();
	}

	/* set widget sensitivity */
	mbview_action_sensitivityall();
}
/*------------------------------------------------------------------------------*/
void do_mbview_navlist_delete(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_navlist_delete:\n");

	/* get position of selected list item */
	Arg args[256];
  Cardinal ac = 0;
	int position_count = 0;
	XtSetArg(args[ac], XmNselectedPositionCount, (XtPointer)&position_count);
	ac++;
	int *position_list = NULL;
	XtSetArg(args[ac], XmNselectedPositions, (XtPointer)&position_list);
	ac++;
	XtGetValues(shared.mb3d_navlist.mbview_list_navlist, args, ac);

	/* deselect any selected nav */
	shared.shareddata.navpick_type = MBV_PICK_NONE;
	shared.shareddata.nav_selected[0] = MBV_SELECT_NONE;
	shared.shareddata.nav_selected[1] = MBV_SELECT_NONE;
	shared.shareddata.nav_point_selected[0] = MBV_SELECT_NONE;
	shared.shareddata.nav_point_selected[1] = MBV_SELECT_NONE;
	shared.shareddata.nav_selected_mbnavadjust[0] = MBV_SELECT_NONE;
	shared.shareddata.nav_selected_mbnavadjust[1] = MBV_SELECT_NONE;

	/* get first valid instance */
	size_t instance = MBV_NO_WINDOW;
	for (int i = 0; i < MBV_MAX_WINDOWS; i++) {
		if (instance == MBV_NO_WINDOW && mbviews[i].data.active)
			instance = i;
	}

	/* delete selected nav points in reverse order if any */
	for (int i = position_count - 1; i >= 0; i--) {
		const int inav = position_list[i] - 1;
		mbview_nav_delete(instance, inav);
	}

	/* reset pick annotation */
	if (position_count > 0) {
		for (int i = 0; i < MBV_MAX_WINDOWS; i++) {
			if (mbviews[i].data.active) {
				/* set pick annotation */
				if (mbviews[i].data.pickinfo_mode == MBV_PICK_NAV)
					mbviews[i].data.pickinfo_mode = MBV_PICK_NONE;
				mbview_pick_text(i);

				/* draw */
				if (mbv_verbose >= 2)
					fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_navlist_delete\n");
				mbview_plotlowhigh(instance);
				mbview_plotlowhighall(instance);

				/* extract profile if pick is right type */
				if (mbviews[i].data.pickinfo_mode == MBV_PICK_NAV)
					mbview_extract_nav_profile(i);

				/* now replot profile */
				mbview_plotprofile(i);
			}
		}

		/* update nav list */
		mbview_updatenavlist();
	}

	/* set widget sensitivity */
	mbview_action_sensitivityall();
}

/*------------------------------------------------------------------------------*/

void do_mbview_sitelist_popdown(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	if (mbv_verbose >= 0)
		fprintf(stderr, "do_mbview_sitelist_popdown:\n");

	shared.init_sitelist = MBV_WINDOW_NULL;
	XmListDeleteAllItems(shared.mb3d_sitelist.mbview_list_sitelist);
	XtPopdown(XtParent(shared.mainWindow_sitelist));
}
/*------------------------------------------------------------------------------*/

void do_mbview_routelist_popdown(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	if (mbv_verbose >= 0)
		fprintf(stderr, "do_mbview_routelist_popdown:\n");

	shared.init_routelist = MBV_WINDOW_NULL;
	XmListDeleteAllItems(shared.mb3d_routelist.mbview_list_routelist);
	XtPopdown(XtParent(shared.mainWindow_routelist));
}
/*------------------------------------------------------------------------------*/

void do_mbview_navlist_popdown(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)w;  // Unused parameter
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	if (mbv_verbose >= 0)
		fprintf(stderr, "do_mbview_navlist_popdown:\n");

	shared.init_navlist = MBV_WINDOW_NULL;
	XmListDeleteAllItems(shared.mb3d_navlist.mbview_list_navlist);
	XtPopdown(XtParent(shared.mainWindow_navlist));
}
/*------------------------------------------------------------------------------*/
void do_mbview_full_render(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_full_render\n");

	/* replot in full rez if last draw was lower rez */
	if (view->lastdrawrez != MBV_REZ_FULL) {
		if (mbv_verbose >= 2)
			fprintf(stderr, "Calling mbview_plotfull from do_mbview_full_render:\n");
		mbview_plotfull(instance);
	}
}

/*------------------------------------------------------------------------------*/
void do_mbview_reset_view(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);
	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_reset_view\n");

	/* reset the the view to defaults */
	view->offset2d_x = 0.0;
	view->offset2d_y = 0.0;
	view->size2d = 0.0;
	view->offset3d_x = 0.0;
	view->offset3d_y = 0.0;
	view->offset3d_z = 0.0;
	view->viewoffset3d_z = 0.0;
	data->exageration = 1.0;
	data->modelelevation3d = 90.0;
	data->modelazimuth3d = 0.0;
	data->viewelevation3d = 90.0;
	data->viewazimuth3d = 0.0;
	view->size2d = 1.0;

	/* reset dialog widgets */
	do_mbview_3dparmstext(instance);
	do_mbview_2dparmstext(instance);

	/* rescale grid */
	mbview_zscaleclear(instance);

	/* rescale data other than the grid */
	mbview_zscale(instance);

	/* clear color status array */
	if (data->display_mode == MBV_DISPLAY_3D) {
		view->lastdrawrez = MBV_REZ_NONE;
		mbview_setcolorparms(instance);
		mbview_colorclear(instance);
	}

	/* set flag to reset view bounds */
	view->viewboundscount = MBV_BOUNDSFREQUENCY;

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from do_mbview_reset_view\n");
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);
}

/*------------------------------------------------------------------------------*/

void do_mbview_clearpicks(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_clearpicks: instance:%zu\n", instance);

	mbview_clearpicks(instance);
}
/*------------------------------------------------------------------------------*/

void do_mbview_profile_dismiss(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	if (mbv_verbose >= 0)
		fprintf(stderr, "do_mbview_profile_dismiss: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	if (data->profile_view_mode == MBV_VIEW_ON) {
		/* destroy opengl context */
		mbview_destroy_prglx(instance);

		/* turn off profile viewing */
		XtUnmanageChild(view->mb3dview.mbview_form_profile);
		data->profile_view_mode = MBV_VIEW_OFF;
	}

	/* reset the togglebutton */
	ac = 0;
	if (data->profile_view_mode == MBV_VIEW_ON) {
		XtSetArg(args[ac], XmNset, XmSET);
		ac++;
	} else {
		XtSetArg(args[ac], XmNset, XmUNSET);
		ac++;
	}
	XtSetValues(view->mb3dview.mbview_toggleButton_profile, args, ac);
}
/*------------------------------------------------------------------------------*/

void do_mbview_view_profile(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	(void)call_data;  // Unused parameter

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_view_profile: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	if (data->profile_view_mode == MBV_VIEW_OFF) {

		XtManageChild(view->mb3dview.mbview_form_profile);
		data->profile_view_mode = MBV_VIEW_ON;

		/* intitialize OpenGL graphics */
		ac = 0;
		XtSetArg(args[ac], mbGLwNrgba, TRUE);
		ac++;
		XtSetArg(args[ac], mbGLwNdepthSize, 1);
		ac++;
		XtSetArg(args[ac], mbGLwNdoublebuffer, True);
		ac++;
		XtSetArg(args[ac], mbGLwNallocateBackground, FALSE);
		ac++;
		XtSetArg(args[ac], XmNwidth, data->prwidth);
		ac++;
		XtSetArg(args[ac], XmNheight, data->prheight);
		ac++;
		view->dpy = (Display *)XtDisplay(view->mb3dview.MB3DView);
		view->prglwmda = mbGLwCreateMDrawingArea(view->mb3dview.mbview_drawingArea_profile, "glwidget", args, ac);
		/* view->prglwmda = XtCreateWidget("glwidget", mbGLwDrawingAreaWidgetClass, view->mb3dview.mbview_drawingArea_profile,
		 * args, ac);*/
		XtManageChild(view->prglwmda);
		XSelectInput(view->dpy, XtWindow(view->prglwmda),
		             (ButtonPressMask | ButtonReleaseMask | ButtonMotionMask | KeyPressMask | KeyReleaseMask | ExposureMask));

		/* initialize the opengl widget */
		mbview_reset_prglx(instance);

		/* draw the profile */
		mbview_plotprofile(instance);
	}

	/* reset the togglebutton */
	ac = 0;
	if (data->profile_view_mode == MBV_VIEW_ON) {
		XtSetArg(args[ac], XmNset, XmSET);
		ac++;
	} else {
		XtSetArg(args[ac], XmNset, XmUNSET);
		ac++;
	}
	XtSetValues(view->mb3dview.mbview_toggleButton_profile, args, ac);
}
/*------------------------------------------------------------------------------*/

void do_mbview_profile_resize(Widget w, XtPointer client_data, XEvent *event, Boolean *unused) {
	(void)w;  // Unused parameter
	(void)unused;  // Unused parameter
	const size_t instance = (size_t)client_data;

	if (mbv_verbose >= 0)
		fprintf(stderr, "do_mbview_profile_resize: instance:%zu\n", instance);

	XConfigureEvent *cevent = (XConfigureEvent *)event;

	/* do this only if a resize event happens */
	if (cevent->type == ConfigureNotify) {
			struct mbview_world_struct *view = &(mbviews[instance]);

		/* get new shell size */
		Dimension width;
		Dimension height;
		XtVaGetValues(view->mb3dview.mbview_scrolledWindow_profile, XmNwidth, &width, XmNheight, &height, NULL);
		fprintf(stderr, "view->mbview_scrolledWindow_profile: width:%d height:%d\n", width, height);

		/* reinitialize the opengl widget */
		mbview_reset_prglx(instance);

		/* draw the profile */
		mbview_plotprofile(instance);
	}
}
/*------------------------------------------------------------------------------*/

void do_mbview_profile_exager(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter

	XmScaleCallbackStruct *acs = (XmScaleCallbackStruct *)call_data;

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_profile_exager: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	const int profile_exager = acs->value;
	data->profile_exageration = 0.1 * profile_exager;

	/* reinitialize the opengl widget */
	mbview_reset_prglx(instance);

	/* draw the profile */
	mbview_plotprofile(instance);
}
/*------------------------------------------------------------------------------*/

void do_mbview_profile_width(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	XmScaleCallbackStruct *acs = (XmScaleCallbackStruct *)call_data;

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_profile_width: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	const int profile_widthfactor = acs->value;
	data->profile_widthfactor = profile_widthfactor;

	/* reinitialize the opengl widget */
	mbview_reset_prglx(instance);

	/* draw the profile */
	mbview_plotprofile(instance);
}
/*------------------------------------------------------------------------------*/

void do_mbview_profile_slope(Widget w, XtPointer client_data, XtPointer call_data) {
	(void)client_data;  // Unused parameter
	XmScaleCallbackStruct *acs = (XmScaleCallbackStruct *)call_data;

	Arg args[256];
  Cardinal ac = 0;
	size_t instance;
	XtSetArg(args[ac], XmNuserData, (XtPointer)&instance);
	ac++;
	XtGetValues(w, args, ac);

	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_profile_slope: instance:%zu\n", instance);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	const int profile_slopethreshold = acs->value;
	data->profile_slopethreshold = 0.01 * profile_slopethreshold;

	/* reinitialize the opengl widget */
	mbview_reset_prglx(instance);

	/* draw the profile */
	mbview_plotprofile(instance);
}
/*------------------------------------------------------------------------------*/
/* MBview status and message functions */
/*------------------------------------------------------------------------------*/

int do_mbview_status(char *message, size_t instance) {
	struct mbview_world_struct *view = &(mbviews[instance]);

	view->message_on = true;

	set_mbview_label_string(view->mb3dview.mbview_label_status, message);

	return (1);
}

/*------------------------------------------------------------------------------*/

int do_mbview_message_on(char *message, size_t instance) {
	struct mbview_world_struct *view = &(mbviews[instance]);

	view->message_on = true;

	set_mbview_label_string(view->mb3dview.mbview_label_message, message);
	XtManageChild(view->mb3dview.mbview_bulletinBoard_message);

	/* force the label to be visible */
	Widget diashell;
	for (diashell = view->mb3dview.mbview_label_message; !XtIsShell(diashell); diashell = XtParent(diashell))
		;
	Widget topshell;
	for (topshell = diashell; !XtIsTopLevelShell(topshell); topshell = XtParent(topshell))
		;
	if (XtIsRealized(diashell) && XtIsRealized(topshell)) {
		Window diawindow = XtWindow(diashell);
		Window topwindow = XtWindow(topshell);
		XEvent event;
		XWindowAttributes xwa;

		/* wait for the dialog to be mapped */
		while (XGetWindowAttributes(view->dpy, diawindow, &xwa) && xwa.map_state != IsViewable) {
			if (XGetWindowAttributes(view->dpy, topwindow, &xwa) && xwa.map_state != IsViewable)
				break;

			XtAppNextEvent(app_context, &event);
			XtDispatchEvent(&event);
		}
	}

	XmUpdateDisplay(topshell);

	return (1);
}

/*------------------------------------------------------------------------------*/

int do_mbview_message_off(size_t instance) {
	struct mbview_world_struct *view = &(mbviews[instance]);

	XtUnmanageChild(view->mb3dview.mbview_bulletinBoard_message);
	XSync(XtDisplay(view->mb3dview.mbview_bulletinBoard_message), 0);
	XmUpdateDisplay(view->mainWindow);

	return (1);
}

/*------------------------------------------------------------------------------*/
/* Change label string cleanly, no memory leak */
/*------------------------------------------------------------------------------*/

void set_mbview_label_string(Widget w, String str) {
	XmString xstr = XmStringCreateLocalized(str);
	if (xstr != NULL)
		XtVaSetValues(w, XmNlabelString, xstr, NULL);
	else
		XtWarning("Failed to update labelString");

	XmStringFree(xstr);
}
/*------------------------------------------------------------------------------*/
/* Change multiline label string cleanly, no memory leak */
/*------------------------------------------------------------------------------*/

void set_mbview_label_multiline_string(Widget w, String str) {
	Boolean argok;
	XmString xstr = (XtPointer)BX_CONVERT(w, str, XmRXmString, 0, &argok);
	if (xstr != NULL && argok)
		XtVaSetValues(w, XmNlabelString, xstr, NULL);
	else
		XtWarning("Failed to update labelString");

	XmStringFree(xstr);
}
/*------------------------------------------------------------------------------*/
/* Get text item string cleanly, no memory leak */
/*------------------------------------------------------------------------------*/

void get_mbview_text_string(Widget w, String str) {
	char *str_tmp = (char *)XmTextGetString(w);
	strcpy(str, str_tmp);
	XtFree(str_tmp);
}

/*------------------------------------------------------------------------------*/
/* Deal with pending X events */
/*------------------------------------------------------------------------------*/

void do_mbview_xevents() {
	if (XtAppPending(app_context)) {
		XEvent event;
		XtAppNextEvent(app_context, &event);
		XtDispatchEvent(&event);
	}
}

/*------------------------------------------------------------------------------*/
/* Add work procedure */
/*------------------------------------------------------------------------------*/

int do_mbview_setbackgroundwork(size_t instance) {
	int status = MB_SUCCESS;

	/* set work function if none set for this instance */
	if (!work_function_set) {
		const int id = XtAppAddWorkProc(app_context, (XtWorkProc)do_mbview_workfunction, (XtPointer)instance);
		if (id > 0)
			work_function_set = true;
		else
			status = MB_FAILURE;
		/*fprintf(stderr,"do_mbview_setbackgroundwork: instance:%zu id:%d\n",
		instance, id);*/
	} else {
		fprintf(stderr, "do_mbview_setbackgroundwork: FUNCTION ALREADY SET for instance:%zu!!\n", instance);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/

int do_mbview_settimer() {
	int status = MB_SUCCESS;

	/* set timer function if none set for this instance */
	if (!work_function_set) {
		const int id = XtAppAddTimeOut(app_context, (unsigned long)timer_timeout_time, (XtTimerCallbackProc)do_mbview_workfunction,
		                     (XtPointer)-1);
		if (id > 0)
			work_function_set = true;
		else
			status = MB_FAILURE;
		/*fprintf(stderr,"do_mbview_settimer: \n");*/
	} else {
		fprintf(stderr, "do_mbview_settimer: FUNCTION ALREADY SET!!\n");
	}

	return (status);
}

/*------------------------------------------------------------------------------*/

int do_mbview_workfunction(XtPointer client_data) {
	/* set starting values */
	size_t instance = (size_t)client_data;
	bool plotting = false;
	int mode = MBV_BACKGROUND_NONE;

	/*fprintf(stderr,"\ndo_mbview_workfunction called: instance:%zu timer_count:%d\n", instance, timer_count);*/

	/* first make sure no plotting is active */
	struct mbview_struct *data = NULL;
	for (int i = 0; i < MBV_MAX_WINDOWS && !plotting; i++) {
		struct mbview_world_struct *view = &(mbviews[i]);
		data = &(view->data);

		/* check it if nothing already found */
		if (data->primary_nxy > 0 &&
		    (view->plot_recursion > 0 || !view->plot_interrupt_allowed || view->button1down ||
		     view->button2down || view->button3down)) {
			plotting = true;
		}
	}

	bool found = false;

	/* first see if possible to work with instance value */
	if (!plotting && instance != MBV_NO_WINDOW && instance < MBV_MAX_WINDOWS && data->primary_nxy > 0) {
		struct mbview_world_struct *view = &(mbviews[instance]);
		data = &(view->data);

		if (view->zscaledonecount < data->primary_nxy - 1) {
			/* set found */
			found = true;
			mode = MBV_BACKGROUND_ZSCALE;
		}

		/* then work on color */
		else if (view->colordonecount < data->primary_nxy - 1) {
			/* set found */
			found = true;
			mode = MBV_BACKGROUND_COLOR;
		}

		/* finally do the full rez plot */
		else if (view->lastdrawrez != MBV_REZ_FULL && timer_count > timer_timeout_count) {
			/* set found */
			found = true;
			mode = MBV_BACKGROUND_FULLPLOT;
		}
	}

	/* if not found check all possible instances */
	if (!plotting && !found) {
		for (int i = 0; i < MBV_MAX_WINDOWS; i++) {
			struct mbview_world_struct *view = &(mbviews[i]);
			data = &(view->data);

			/* check it if nothing already found */
			if (!found && data->primary_nxy > 0) {
				if (view->zscaledonecount < data->primary_nxy - 1) {
					/* set found */
					found = true;
					mode = MBV_BACKGROUND_ZSCALE;
					instance = i;
				}

				/* then work on color */
				else if (view->colordonecount < data->primary_nxy - 1) {
					/* set found */
					found = true;
					mode = MBV_BACKGROUND_COLOR;
					instance = i;
				}

				/* finally do the full rez plot */
				else if (view->lastdrawrez != MBV_REZ_FULL && timer_count > timer_timeout_count) {
					/* set found */
					found = true;
					mode = MBV_BACKGROUND_FULLPLOT;
					instance = i;
				}
			}
		}
	}
	/*fprintf(stderr, "do_mbview_workfunction: plotting:%d found:%d instance:%zu mode:%d\n",
	plotting,found,instance,mode);*/

	/* do the work if instance found */
	if (!plotting && found) {
		struct mbview_world_struct *view = &(mbviews[instance]);
		data = &(view->data);

		/* first work on zscale */
		if (mode == MBV_BACKGROUND_ZSCALE) {
			/*fprintf(stderr,"do_mbview_workfunction: recalculating zscale in background %d of %d...\n",
			view->zscaledonecount,data->primary_nxy);*/
			/* recalculate zscale for MBV_NUMBACKGROUNDCALC cells */
			int ncalc = 0;
			for (int k = view->zscaledonecount; k < data->primary_nxy && ncalc < MBV_NUMBACKGROUNDCALC; k++) {
				if (!(data->primary_stat_z[k / 8] & statmask[k % 8])) {
					mbview_zscalegridpoint(instance, k);
					ncalc++;
				}
				view->zscaledonecount = k;
			}
		}

		/* then work on color */
		else if (mode == MBV_BACKGROUND_COLOR) {
			/* fprintf(stderr,"do_mbview_workfunction: recalculating color in background %d of %d...\n",
			view->colordonecount,data->primary_nxy);*/

			/* use histogram equalization if needed */
			float *histogram = NULL;
			if (data->grid_mode == MBV_GRID_VIEW_PRIMARY && data->primary_histogram) {
				histogram = view->primary_histogram;
			}
			else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE && data->primaryslope_histogram) {
				histogram = view->primaryslope_histogram;
			}
			else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY && data->secondary_histogram) {
				histogram = view->secondary_histogram;
			}

			/* recalculate color for MBV_NUMBACKGROUNDCALC cells */
			int ncalc = 0;
			for (int k = view->colordonecount; k < data->primary_nxy && ncalc < MBV_NUMBACKGROUNDCALC; k++) {
				if (!(data->primary_stat_color[k / 8] & statmask[k % 8])) {
					const int i = k / data->primary_n_columns;
					const int j = k % data->primary_n_rows;
					mbview_colorpoint(view, data, histogram, i, j, k);
					ncalc++;
				}
				view->colordonecount = k;
			}
		}

		/* finally do the full rez plot */
		else if (mode == MBV_BACKGROUND_FULLPLOT) {
			/*fprintf(stderr,"do_mbview_workfunction: plotting instance %d full resolution on timeout...\n", instance);*/
			/* do full rez plot */
			mbview_plotfull(instance);
		}
	}

	/* reset the work function as either background or timed */
	work_function_set = false;
	if (found) {
		do_mbview_setbackgroundwork(instance);
		timer_count = 0;
	}
	else {
		do_mbview_settimer();
		if (plotting)
			timer_count = 0;
		else
			timer_count++;
	}

	return (MB_SUCCESS);
}
/*------------------------------------------------------------------------------*/
