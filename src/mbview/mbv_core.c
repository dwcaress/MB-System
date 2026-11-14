/*--------------------------------------------------------------------
 *    The MB-system:  mbv_core.c  11/7/2026
 *
 *    Copyright (c) 2026 by
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * Core functions of the mbview map view that use no X11 or Motif: start-up
 * and reset of the views, view parameters, notify functions, and the mouse
 * and key handling (mbview_input). They were moved unchanged from
 * mbview_callbacks.c, which keeps the Motif user interface.
 */

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_status.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1800)
#	ifndef WIN32
#		define WIN32
#	endif
#	include <WinSock2.h>
#include <windows.h>
#endif

#ifndef MBVIEW_NO_MOTIF
#include <X11/StringDefs.h>
#include <X11/cursorfont.h>
#include <Xm/Xm.h>
#else
#include "mbv_xtypes.h"
#endif
#include "MB3DView.h"
#include "MB3DSiteList.h"
#include "MB3DRouteList.h"
#include "MB3DNavList.h"

#include "mbv_gl.h"

#include "mbview.h"
#include "mbviewprivate.h"
#include "mbv_render.h"
#include "mbv_input.h"
#include "mbv_ui.h"

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
	mbview_gislist_reset(mode);

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
	shared.shareddata.nvector_alloc = 0;
	shared.shareddata.vector_selected = MBV_SELECT_NONE;
	shared.shareddata.vector_point_selected = MBV_SELECT_NONE;
	shared.shareddata.vectors = NULL;

	/* GIS layer, polyline, polygon, and point data */
	shared.shareddata.ngislayer = 0;
	shared.shareddata.ngislayer_alloc = 0;
	shared.shareddata.gislayers = NULL;
	shared.shareddata.polyline_mode = MBV_POLYLINE_OFF;
	shared.shareddata.npolyline = 0;
	shared.shareddata.npolyline_alloc = 0;
	shared.shareddata.polyline_selected = MBV_SELECT_NONE;
	shared.shareddata.polylines = NULL;
	shared.shareddata.polygon_mode = MBV_POLYGON_OFF;
	shared.shareddata.npolygon = 0;
	shared.shareddata.npolygon_alloc = 0;
	shared.shareddata.polygon_selected = MBV_SELECT_NONE;
	shared.shareddata.polygons = NULL;
	shared.shareddata.gispoint_mode = MBV_GISPOINT_OFF;
	shared.shareddata.ngispoint = 0;
	shared.shareddata.ngispoint_alloc = 0;
	shared.shareddata.gispoint_selected = MBV_SELECT_NONE;
	shared.shareddata.gispoints = NULL;

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

		data->exaggeration = 1.0;
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
		data->profile_exaggeration = 1.0;
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
		data->navswathbounds_view_mode = MBV_VIEW_OFF;
		data->navdrape_view_mode = MBV_VIEW_OFF;

		/* vector data */
		data->vector_view_mode = MBV_VIEW_OFF;

		/* GIS polyline, polygon, and point data */
		data->polyline_view_mode = MBV_VIEW_OFF;
		data->polygon_view_mode = MBV_VIEW_OFF;
		data->gispoint_view_mode = MBV_VIEW_OFF;

		/* profile data */
		data->profile_view_mode = MBV_VIEW_OFF;

    /* general use state variables to turn action button sensitivity on and off */
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

		/* the view has no grid now: drop the blocks of the old one and raise
		   every mark. The counters are not reset, so a user that kept the
		   values of a previous view of this instance cannot match them
		   (mbv_marks.h). */
		mbv_marks_free(mbv_verbose, &view->marks);
		mbview_contour_free_segments(instance);

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
		view->exaggeration_save = 0.0;
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
		fprintf(stderr, "dbg2       exaggeration:               %f\n", data->exaggeration);
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
		fprintf(stderr, "dbg2       profile_exaggeration:        %f\n", data->profile_exaggeration);
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
		fprintf(stderr, "dbg2       navswathbounds_view_mode:  %d\n", data->navswathbounds_view_mode);
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
		fprintf(stderr, "dbg2       polyline_view_mode:        %d\n", data->polyline_view_mode);
		fprintf(stderr, "dbg2       polygon_view_mode:         %d\n", data->polygon_view_mode);
		fprintf(stderr, "dbg2       gispoint_view_mode:        %d\n", data->gispoint_view_mode);
		fprintf(stderr, "dbg2       ngislayer:                 %d\n", shared.shareddata.ngislayer);
		fprintf(stderr, "dbg2       polyline_mode:             %d\n", shared.shareddata.polyline_mode);
		fprintf(stderr, "dbg2       npolyline:                 %d\n", shared.shareddata.npolyline);
		fprintf(stderr, "dbg2       polygon_mode:              %d\n", shared.shareddata.polygon_mode);
		fprintf(stderr, "dbg2       npolygon:                  %d\n", shared.shareddata.npolygon);
		fprintf(stderr, "dbg2       gispoint_mode:             %d\n", shared.shareddata.gispoint_mode);
		fprintf(stderr, "dbg2       ngispoint:                 %d\n", shared.shareddata.ngispoint);
		for (int i = 0; i < shared.shareddata.nvector; i++) {
			fprintf(stderr, "dbg2       vector %d color:         %d\n", i, shared.shareddata.vectors[i].color);
			fprintf(stderr, "dbg2       vector %d size:          %d\n", i, shared.shareddata.vectors[i].size);
			fprintf(stderr, "dbg2       vector %d name:          %s\n", i, shared.shareddata.vectors[i].name);
			fprintf(stderr, "dbg2       vector %d format:        %d\n", i, shared.shareddata.vectors[i].format);
			fprintf(stderr, "dbg2       vector %d npoints:       %d\n", i, shared.shareddata.vectors[i].npoints);
			fprintf(stderr, "dbg2       vector %d npoints_alloc: %d\n", i, shared.shareddata.vectors[i].npoints_alloc);
			fprintf(stderr, "dbg2       vector %d nselected:     %d\n", i, shared.shareddata.vectors[i].nselected);
			for (int j = 0; j < shared.shareddata.vectors[i].npoints; j++) {
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
/*------------------------------------------------------------------------------*/
/* What changed in a view's terrain, for a user that keeps its own copy of the
   node arrays or caches picks (src/mbview/mbv_marks.h). Reading changes
   nothing, so several users can follow one view. */
int mbview_getmarks(int verbose, size_t instance, uint64_t *position, uint64_t *color, uint64_t *mask, uint64_t *drawn,
                    int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	*position = mbv_marks_getposition(&view->marks);
	*color = mbv_marks_getcolor(&view->marks);
	*mask = mbv_marks_getmask(&view->marks);
	*drawn = mbv_marks_getdrawn(&view->marks);
	*error = MB_ERROR_NO_ERROR;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       position:         %llu\n", (unsigned long long)*position);
		fprintf(stderr, "dbg2       color:            %llu\n", (unsigned long long)*color);
		fprintf(stderr, "dbg2       mask:             %llu\n", (unsigned long long)*mask);
		fprintf(stderr, "dbg2       drawn:            %llu\n", (unsigned long long)*drawn);
		fprintf(stderr, "dbg2       error:            %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:           %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_getnodeversion(int verbose, size_t instance, int i0, int i1, int j0, int j1, uint64_t *version, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       i0:               %d\n", i0);
		fprintf(stderr, "dbg2       i1:               %d\n", i1);
		fprintf(stderr, "dbg2       j0:               %d\n", j0);
		fprintf(stderr, "dbg2       j1:               %d\n", j1);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	*version = mbv_marks_nodesversion(&view->marks, i0, i1, j0, j1);
	*error = MB_ERROR_NO_ERROR;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       version:          %llu\n", (unsigned long long)*version);
		fprintf(stderr, "dbg2       error:            %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:           %d\n", status);
	}

	return (status);
}
/*------------------------------------------------------------------------------*/
int mbview_getmarkblocks(int verbose, size_t instance, int *block_nodes, int *nblock_i, int *nblock_j, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	mbv_marks_blocks(&view->marks, block_nodes, nblock_i, nblock_j);
	*error = MB_ERROR_NO_ERROR;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       block_nodes:      %d\n", *block_nodes);
		fprintf(stderr, "dbg2       nblock_i:         %d\n", *nblock_i);
		fprintf(stderr, "dbg2       nblock_j:         %d\n", *nblock_j);
		fprintf(stderr, "dbg2       error:            %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:           %d\n", status);
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
		fprintf(stderr, "dbg2       ngislayer:                 %d\n", shared.shareddata.ngislayer);
		fprintf(stderr, "dbg2       polyline_mode:             %d\n", shared.shareddata.polyline_mode);
		fprintf(stderr, "dbg2       npolyline:                 %d\n", shared.shareddata.npolyline);
		fprintf(stderr, "dbg2       polygon_mode:              %d\n", shared.shareddata.polygon_mode);
		fprintf(stderr, "dbg2       npolygon:                  %d\n", shared.shareddata.npolygon);
		fprintf(stderr, "dbg2       gispoint_mode:             %d\n", shared.shareddata.gispoint_mode);
		fprintf(stderr, "dbg2       ngispoint:                 %d\n", shared.shareddata.ngispoint);
		for (int i = 0; i < shared.shareddata.nvector; i++) {
			fprintf(stderr, "dbg2       vector %d color:         %d\n", i, shared.shareddata.vectors[i].color);
			fprintf(stderr, "dbg2       vector %d size:          %d\n", i, shared.shareddata.vectors[i].size);
			fprintf(stderr, "dbg2       vector %d name:          %s\n", i, shared.shareddata.vectors[i].name);
			fprintf(stderr, "dbg2       vector %d format:        %d\n", i, shared.shareddata.vectors[i].format);
			fprintf(stderr, "dbg2       vector %d npoints:       %d\n", i, shared.shareddata.vectors[i].npoints);
			fprintf(stderr, "dbg2       vector %d npoints_alloc: %d\n", i, shared.shareddata.vectors[i].npoints_alloc);
			fprintf(stderr, "dbg2       vector %d nselected:     %d\n", i, shared.shareddata.vectors[i].nselected);
			for (int j = 0; j < shared.shareddata.vectors[i].npoints; j++) {
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

	/* the window size and the resolution dimensions decide the node range and
	   the stride a renderer should draw; the data does not change here
	   (mbv_marks.h) */
	mbv_marks_drawn(&view->marks);

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
                           int nav_view_mode, int navswathbounds_view_mode, int navdrape_view_mode, int vector_view_mode, double exaggeration,
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
		fprintf(stderr, "dbg2       nav_view_mode:             %d\n", nav_view_mode);
		fprintf(stderr, "dbg2       navswathbounds_view_mode:  %d\n", navswathbounds_view_mode);
		fprintf(stderr, "dbg2       vector_view_mode:          %d\n", vector_view_mode);
		fprintf(stderr, "dbg2       exaggeration:               %f\n", exaggeration);
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
	data->navswathbounds_view_mode = navswathbounds_view_mode;
	data->navdrape_view_mode = navdrape_view_mode;
	data->vector_view_mode = vector_view_mode;
	data->exaggeration = exaggeration;

	/* the vertical exaggeration scales every node's position, and the view
	   controls also carry the grid mode and the shading (mbv_marks.h) */
	mbv_marks_everything(&view->marks);
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
		if (data->primary_max > data->primary_min)
			data->contour_interval = pow(10.0, floor(log10(data->primary_max - data->primary_min)) - 1.0);
	}
	if (data->secondary_nxy > 0 && data->secondary_colortable_max <= data->secondary_colortable_min) {
		data->secondary_colortable_min = data->secondary_min - 0.01 * (data->secondary_max - data->secondary_min);
		data->secondary_colortable_max = data->secondary_max + 0.01 * (data->secondary_max - data->secondary_min);
		data->overlay_shade_center = 0.5 * (data->secondary_max + data->secondary_min);
	}

	/* the colour bounds above are written after the mbview_colorclear of this
	   function, so the colours of every node depend on them: raise the colour
	   mark here, after them, not with the clear (mbv_marks.h) */
	mbv_marks_color(&view->marks);

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
void mbview_input(size_t instance, const struct mbv_input_event *event) {
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	bool replotall = false;
	bool replotprofile = false;

	/* If the drawing area is still initialized */
	if (view->init != MBV_WINDOW_NULL) {

		/* Check for mouse pressed. */
		if (event->type == MBV_INPUT_PRESS) {
			/*fprintf(stderr, "event->type == MBV_INPUT_PRESS  %d %d  mouse mode:%d\n",
			event->x,event->y, data->mouse_mode);*/
			/* save location */
			view->button_down_x = event->x;
			view->button_down_y = event->y;

			/* If left mouse button is pushed */
			if (event->button == 1) {
				/* set button1down flag */
				view->button1down = true;

				/* handle move */
				if (data->mouse_mode == MBV_MOUSE_MOVE || data->mouse_mode == MBV_MOUSE_ROTATE ||
				    data->mouse_mode == MBV_MOUSE_SHADE || data->mouse_mode == MBV_MOUSE_VIEWPOINT ||
				    data->mouse_mode == MBV_MOUSE_NAV || data->mouse_mode == MBV_MOUSE_NAVFILE) {
					/* set cursor for pick */
					mbv_ui_cursor(instance, MBV_CURSOR_TARGET_RED);

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
					mbv_ui_cursor(instance, MBV_CURSOR_TARGET_RED);

					/* process area */
					mbview_region(instance, MBV_REGION_DOWN, view->button_down_x, data->height - view->button_down_y);

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle editing sites */
				else if (data->mouse_mode == MBV_MOUSE_SITE) {
					/* set cursor for site */
					mbv_ui_cursor(instance, MBV_CURSOR_TARGET_RED);

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
					mbv_ui_cursor(instance, MBV_CURSOR_TARGET_RED);

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
			else if (event->button == 2) {
				/* set button2down flag */
				view->button2down = true;

				/* handle move */
				if (data->mouse_mode == MBV_MOUSE_MOVE) {
					/* set cursor for move */
					mbv_ui_cursor(instance, MBV_CURSOR_FLEUR_BLACK);

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
					mbv_ui_cursor(instance, MBV_CURSOR_FLEUR_BLACK);

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
						mbv_ui_cursor(instance, MBV_CURSOR_FLEUR_BLACK);

						view->illuminate_azimuth_save = data->illuminate_azimuth;
						view->illuminate_elevation_save = data->illuminate_elevation;
					}

					/* handle shading by overlay */
					else if (shade_mode == MBV_SHADE_VIEW_SLOPE) {
						/* set cursor for shade */
						mbv_ui_cursor(instance, MBV_CURSOR_FLEUR_BLACK);
					}

					/* handle shading by overlay */
					else if (shade_mode == MBV_SHADE_VIEW_OVERLAY) {
						/* set cursor for shade */
						mbv_ui_cursor(instance, MBV_CURSOR_FLEUR_BLACK);
					}
				}

				/* handle viewpoint rotation */
				else if (data->mouse_mode == MBV_MOUSE_VIEWPOINT) {
					/* set cursor for rotate */
					mbv_ui_cursor(instance, MBV_CURSOR_FLEUR_BLACK);

					view->viewazimuth3d_save = data->viewazimuth3d;
					view->viewelevation3d_save = data->viewelevation3d;
				}

				/* handle area picking */
				else if (data->mouse_mode == MBV_MOUSE_AREA) {
					/* set cursor for area */
					mbv_ui_cursor(instance, MBV_CURSOR_TARGET_RED);

					/* process area */
					mbview_area(instance, MBV_AREALENGTH_DOWN, view->button_down_x, data->height - view->button_down_y);

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle editing sites */
				else if (data->mouse_mode == MBV_MOUSE_SITE) {
					/* set cursor for site */
					mbv_ui_cursor(instance, MBV_CURSOR_TARGET_RED);

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
					mbv_ui_cursor(instance, MBV_CURSOR_TARGET_RED);

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
					mbv_ui_cursor(instance, MBV_CURSOR_TARGET_RED);

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
			else if (event->button == 3) {
				/* set button3down flag */
				view->button3down = true;

				/* change the map scaling */
				if (data->mouse_mode == MBV_MOUSE_MOVE) {
					/* set cursor for scaling */
					mbv_ui_cursor(instance, MBV_CURSOR_SIZING_BLACK);

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
					mbv_ui_cursor(instance, MBV_CURSOR_FLEUR_BLACK);

					view->exaggeration_save = data->exaggeration;
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
						mbv_ui_cursor(instance, MBV_CURSOR_FLEUR_BLACK);

						view->illuminate_magnitude_save = data->illuminate_magnitude;
					}

					/* handle shading by slope */
					else if (shade_mode == MBV_SHADE_VIEW_SLOPE) {
						/* set cursor for shading */
						mbv_ui_cursor(instance, MBV_CURSOR_FLEUR_BLACK);

						view->slope_magnitude_save = data->slope_magnitude;
					}

					/* handle shading by overlay */
					else if (shade_mode == MBV_SHADE_VIEW_OVERLAY) {
						/* set cursor for shading */
						mbv_ui_cursor(instance, MBV_CURSOR_FLEUR_BLACK);

						view->overlay_shade_magnitude_save = data->overlay_shade_magnitude;
					}
				}

				/* handle viewpoint rotation */
				else if (data->mouse_mode == MBV_MOUSE_VIEWPOINT) {
					/* set cursor for scaling */
					mbv_ui_cursor(instance, MBV_CURSOR_SIZING_BLACK);

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
					mbv_ui_cursor(instance, MBV_CURSOR_SIZING_BLACK);

					/* process area */
					view->areaaspect_save = view->areaaspect;
				}

				/* handle editing sites */
				else if (data->mouse_mode == MBV_MOUSE_SITE) {
					/* set cursor for area */
					mbv_ui_cursor(instance, MBV_CURSOR_TARGET_RED);

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
					mbv_ui_cursor(instance, MBV_CURSOR_TARGET_RED);

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
					mbv_ui_cursor(instance, MBV_CURSOR_TARGET_RED);

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
		if (event->type == MBV_INPUT_MOTION) {
			/*fprintf(stderr, "event->type == MBV_INPUT_MOTION  %d %d  mouse mode:%d\n",
			event->x,event->y, data->mouse_mode);*/

			/* prohibit event interruption of plotting */
			view->plot_interrupt_allowed = false;

			/* save location */
			view->button_move_x = event->x;
			view->button_move_y = event->y;

			/* If left mouse button is dragged */
			if (view->button1down) {

				/* set cursor for drag */
				mbv_ui_cursor(instance, MBV_CURSOR_FLEUR_RED);

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
					mbv_ui_cursor(instance, MBV_CURSOR_TARGET_RED);

					/* process region */
					mbview_region(instance, MBV_REGION_MOVE, view->button_move_x, data->height - view->button_move_y);

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle editing sites */
				else if (data->mouse_mode == MBV_MOUSE_SITE) {
					/* set cursor for site */
					mbv_ui_cursor(instance, MBV_CURSOR_TARGET_RED);

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
					mbv_ui_cursor(instance, MBV_CURSOR_TARGET_RED);

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
					mbv_ui_cursor(instance, MBV_CURSOR_FLEUR_RED);

					/* move map */
					if (data->display_mode == MBV_DISPLAY_2D) {
						view->offset2d_x = view->offset2d_x_save +
						                   (view->button_move_x - view->button_down_x) * (view->right - view->left) / data->width;
						view->offset2d_y = view->offset2d_y_save - (view->button_move_y - view->button_down_y) *
						                                               (view->top - view->bottom) / data->height;
						mbv_ui_view_values_changed(instance, MBV_VALUES_VIEW_2DOFFSET);
					}
					else {
						view->offset3d_x = view->offset3d_x_save + (view->button_move_x - view->button_down_x) *
						                                               MBV_OPENGL_WIDTH * MBV_OPENGL_WIDTH / data->width;
						view->offset3d_y = view->offset3d_y_save - (view->button_move_y - view->button_down_y) *
						                                               view->aspect_ratio * view->aspect_ratio *
						                                               MBV_OPENGL_WIDTH * MBV_OPENGL_WIDTH / data->height;
						mbv_ui_view_values_changed(instance, MBV_VALUES_VIEW_3DOFFSET);
					}

					/* set flag to reset view bounds */
					view->viewboundscount++;

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle 3D rotate */
				else if (data->mouse_mode == MBV_MOUSE_ROTATE) {
					/* set cursor for rotate */
					mbv_ui_cursor(instance, MBV_CURSOR_FLEUR_RED);

					/* rotate viewpoint of 3D map */
					data->modelazimuth3d = view->modelazimuth3d_save +
					                       180.0 * ((double)(view->button_move_x - view->button_down_x)) / ((double)data->width);
					data->modelelevation3d =
					    view->modelelevation3d_save +
					    180.0 * ((double)(view->button_move_y - view->button_down_y)) / ((double)data->height);
					mbv_ui_view_values_changed(instance, MBV_VALUES_MODEL_ORIENTATION);

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
						mbv_ui_cursor(instance, MBV_CURSOR_FLEUR_BLACK);

						/* rotate illumination */
						data->illuminate_azimuth =
						    view->illuminate_azimuth_save +
						    180.0 * ((double)(view->button_move_x - view->button_down_x)) / ((double)data->width);
						data->illuminate_elevation =
						    view->illuminate_elevation_save +
						    180.0 * ((double)(view->button_move_y - view->button_down_y)) / ((double)data->height);
						mbv_ui_view_values_changed(instance, MBV_VALUES_ILLUMINATION);

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
					mbv_ui_cursor(instance, MBV_CURSOR_FLEUR_RED);

					/* rotate viewpoint of 3D map */
					data->viewazimuth3d = view->viewazimuth3d_save +
					                      180.0 * ((double)(view->button_move_x - view->button_down_x)) / ((double)data->width);
					data->viewelevation3d =
					    view->viewelevation3d_save +
					    180.0 * ((double)(view->button_move_y - view->button_down_y)) / ((double)data->height);
					mbv_ui_view_values_changed(instance, MBV_VALUES_VIEW_ORIENTATION);

					/* set flag to reset view bounds */
					view->viewboundscount++;

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle area picking */
				else if (data->mouse_mode == MBV_MOUSE_AREA) {
					/* set cursor for area */
					mbv_ui_cursor(instance, MBV_CURSOR_TARGET_RED);

					/* process area */
					mbview_area(instance, MBV_AREALENGTH_MOVE, view->button_move_x, data->height - view->button_move_y);

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle editing sites */
				else if (data->mouse_mode == MBV_MOUSE_SITE) {
					/* set cursor for site */
					mbv_ui_cursor(instance, MBV_CURSOR_TARGET_RED);

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
					mbv_ui_cursor(instance, MBV_CURSOR_TARGET_RED);

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
					mbv_ui_cursor(instance, MBV_CURSOR_TARGET_RED);

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
					mbv_ui_cursor(instance, MBV_CURSOR_SIZING_BLACK);

					/* rescale 2D map */
					if (data->display_mode == MBV_DISPLAY_2D) {
						view->size2d = view->size2d_save *
						               exp(((double)(view->button_down_y - view->button_move_y)) / ((double)data->height));
						mbv_ui_view_values_changed(instance, MBV_VALUES_VIEW_2DZOOM);
					}

					/* rescale 3D map */
					else {
						view->offset3d_z = view->offset3d_z_save +
						                   2.0 * (((double)(view->button_down_y - view->button_move_y)) / ((double)data->height));
						mbv_ui_view_values_changed(instance, MBV_VALUES_MODEL_3DZOOM);
					}

					/* set flag to reset view bounds */
					view->viewboundscount++;

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle vertical exaggerate */
				else if (data->mouse_mode == MBV_MOUSE_ROTATE) {
					/* set cursor for exaggerate */
					mbv_ui_cursor(instance, MBV_CURSOR_FLEUR_RED);

					/* change vertical exaggeration of 3D map */
					data->exaggeration = view->exaggeration_save *
					                    exp(((double)(view->button_down_y - view->button_move_y)) / ((double)data->height));
					if (data->display_projection_mode != MBV_PROJECTION_SPHEROID) {
						view->zorigin = data->exaggeration * 0.5 * (data->primary_min + data->primary_max);
					}
					mbv_ui_view_values_changed(instance, MBV_VALUES_EXAGGERATION);

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
						mbv_ui_cursor(instance, MBV_CURSOR_FLEUR_BLACK);

						/* change magnitude of illumination */
						data->illuminate_magnitude =
						    view->illuminate_magnitude_save *
						    exp(((double)(view->button_down_y - view->button_move_y)) / ((double)data->height));
						mbv_ui_view_values_changed(instance, MBV_VALUES_ILLUMINATION_AMP);

						/* clear color status array */
						mbview_setcolorparms(instance);
						mbview_colorclear(instance);

						/* replot */
						mbview_plotlow(instance);
					}

					/* handle shading by slope */
					else if (shade_mode == MBV_SHADE_VIEW_SLOPE) {
						/* set cursor for shading */
						mbv_ui_cursor(instance, MBV_CURSOR_FLEUR_BLACK);

						/* change magnitude of slope shading */
						data->slope_magnitude =
						    view->slope_magnitude_save *
						    exp(((double)(view->button_down_y - view->button_move_y)) / ((double)data->height));
						mbv_ui_view_values_changed(instance, MBV_VALUES_SLOPE_AMP);

						/* clear color status array */
						mbview_setcolorparms(instance);
						mbview_colorclear(instance);

						/* replot */
						mbview_plotlow(instance);
					}

					/* handle shading by overlay */
					else if (shade_mode == MBV_SHADE_VIEW_OVERLAY) {
						/* set cursor for shading */
						mbv_ui_cursor(instance, MBV_CURSOR_FLEUR_BLACK);

						/* change magnitude of overlay shading */
						data->overlay_shade_magnitude =
						    view->overlay_shade_magnitude_save *
						    exp(((double)(view->button_down_y - view->button_move_y)) / ((double)data->height));
						mbv_ui_view_values_changed(instance, MBV_VALUES_OVERLAY_AMP);

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
					mbv_ui_cursor(instance, MBV_CURSOR_SIZING_BLACK);

					/* rescale 2D map */
					if (data->display_mode == MBV_DISPLAY_2D) {
						view->size2d = view->size2d_save *
						               exp(((double)(view->button_down_y - view->button_move_y)) / ((double)data->height));
						mbv_ui_view_values_changed(instance, MBV_VALUES_VIEW_2DZOOM);
					}

					/* rescale 3D map */
					else {
						view->viewoffset3d_z =
						    view->viewoffset3d_z_save +
						    2.0 * (((double)(view->button_down_y - view->button_move_y)) / ((double)data->height));
						mbv_ui_view_values_changed(instance, MBV_VALUES_VIEW_3DZOOM);
					}

					/* set flag to reset view bounds */
					view->viewboundscount++;

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle area picking */
				else if (data->mouse_mode == MBV_MOUSE_AREA) {
					/* set cursor for area */
					mbv_ui_cursor(instance, MBV_CURSOR_SIZING_BLACK);

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
					mbv_ui_cursor(instance, MBV_CURSOR_TARGET_RED);

					/* set replotall */
					replotall = true;

					/* replot */
					mbview_plotlow(instance);
				}

				/* handle editing routes */
				else if (data->mouse_mode == MBV_MOUSE_ROUTE) {
					/* set cursor for routes */
					mbv_ui_cursor(instance, MBV_CURSOR_TARGET_RED);

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
					mbv_ui_cursor(instance, MBV_CURSOR_TARGET_RED);

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
		if (event->type == MBV_INPUT_RELEASE) {
			/*fprintf(stderr, "event->type == MBV_INPUT_RELEASE  %d %d  mouse mode:%d\n",
			event->x,event->y, data->mouse_mode);*/

			/* save location */
			view->button_up_x = event->x;
			view->button_up_y = event->y;

			/* If left mouse button is released */
			if (view->button1down) {

				/* handle move */
				if ((data->mouse_mode == MBV_MOUSE_MOVE || data->mouse_mode == MBV_MOUSE_ROTATE ||
				     data->mouse_mode == MBV_MOUSE_SHADE || data->mouse_mode == MBV_MOUSE_VIEWPOINT) &&
				    ((view->button_down_x != view->button_up_x) || (view->button_down_y != view->button_up_y))) {
					/* set cursor for pick */
					mbv_ui_cursor(instance, MBV_CURSOR_TARGET_RED);

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
			mbv_ui_cursor(instance, MBV_CURSOR_TARGET_BLACK);

			/* allow event interruption of plotting */
			view->plot_interrupt_allowed = true;
		} /* end of button release events */

		/* Deal with KeyPress events */
		if (event->type == MBV_INPUT_KEYPRESS) {
			/* fprintf(stderr,"KeyPress event\n"); */
			/* Get key pressed */

			/* process events */
			switch (event->key) {
			case 'R':
			case 'r':
				mbv_ui_reset_view(instance);
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
/* Change the mouse mode of an instance (MBV_MOUSE_*): switch on the overlay
   that the mode edits, clear site and route selections that the new mode cannot
   edit, update the mouse mode controls and redraw if needed. An unknown mode
   leaves the mouse mode unchanged. Used by the Motif mouse mode radio buttons
   (do_mbview_mouse_rmode) and by other toolkits. */
int mbview_setmousemode(size_t instance, int mode) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       mode:             %d\n", mode);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	bool replot = false;

	if (mode == MBV_MOUSE_MOVE)
		data->mouse_mode = MBV_MOUSE_MOVE;
	else if (mode == MBV_MOUSE_ROTATE)
		data->mouse_mode = MBV_MOUSE_ROTATE;
	else if (mode == MBV_MOUSE_SHADE)
		data->mouse_mode = MBV_MOUSE_SHADE;
	else if (mode == MBV_MOUSE_VIEWPOINT)
		data->mouse_mode = MBV_MOUSE_VIEWPOINT;
	else if (mode == MBV_MOUSE_AREA)
		data->mouse_mode = MBV_MOUSE_AREA;
	else if (mode == MBV_MOUSE_SITE) {
		data->mouse_mode = MBV_MOUSE_SITE;
		data->site_view_mode = MBV_VIEW_ON;
		set_mbview_site_view_mode(instance, data->site_view_mode);
		replot = true;
	}
	else if (mode == MBV_MOUSE_ROUTE) {
		data->mouse_mode = MBV_MOUSE_ROUTE;
		data->route_view_mode = MBV_VIEW_ON;
		set_mbview_route_view_mode(instance, data->route_view_mode);
		replot = true;
	}
	else if (mode == MBV_MOUSE_NAV) {
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
	else if (mode == MBV_MOUSE_NAVFILE) {
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
			fprintf(stderr, "Calling mbview_plotlowhigh from mbview_setmousemode\n");
		mbview_plotlowhigh(instance);
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
					const int i = k / data->primary_n_rows;
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
int mbview_setdisplaymode(size_t instance, int mode) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       mode:             %d\n", mode);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get mode value */
	data->display_mode = mode;

	/* the node range and the stride a renderer should draw change, the data
	   does not (mbv_marks.h) */
	mbv_marks_drawn(&view->marks);

	/* set togglebuttons */
	set_mbview_display_mode(instance, data->display_mode);
	set_mbview_mouse_mode(instance, data->mouse_mode);

	if (mbv_verbose >= 2)
		fprintf(stderr, "%s instance:%zu mode:%d\n", __func__, instance, data->display_mode);

	/* set contour flags */
	view->contourlorez = false;
	view->contourhirez = false;
	view->contourfullrez = false;

	/* set flag to reset view bounds */
	view->viewboundscount = MBV_BOUNDSFREQUENCY;

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from %s\n", __func__);
	mbview_plotlowhigh(instance);

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
int mbview_setgridmode(size_t instance, int grid_mode) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       grid_mode:        %d\n", grid_mode);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get mode value */
	data->grid_mode = grid_mode;

	/* the colours of every node come from the grid this mode selects, and in
	   secondary grid mode the secondary no-data pattern decides which triangles
	   are drawn (mbv_marks.h) */
	mbv_marks_color(&view->marks);
	mbv_marks_mask(&view->marks);

	if (mbv_verbose >= 2)
		fprintf(stderr, "%s instance:%zu mode:%d\n", __func__, instance, data->grid_mode);

	/* set togglebuttons */
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY) {
		set_mbview_grid_mode(instance, data->grid_mode);
		set_mbview_histogram_mode(instance, data->primary_histogram);
		set_mbview_colortable(instance, data->primary_colortable);
		set_mbview_colortable_mode(instance, data->primary_colortable_mode);
		set_mbview_shade_mode(instance, data->primary_shade_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE) {
		set_mbview_grid_mode(instance, data->grid_mode);
		set_mbview_histogram_mode(instance, data->primaryslope_histogram);
		set_mbview_colortable(instance, data->slope_colortable);
		set_mbview_colortable_mode(instance, data->slope_colortable_mode);
		set_mbview_shade_mode(instance, data->slope_shade_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY) {
		set_mbview_grid_mode(instance, data->grid_mode);
		set_mbview_histogram_mode(instance, data->secondary_histogram);
		set_mbview_colortable(instance, data->secondary_colortable);
		set_mbview_colortable_mode(instance, data->secondary_colortable_mode);
		set_mbview_shade_mode(instance, data->secondary_shade_mode);
	}

	/* clear color status array */
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from %s\n", __func__);
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
int mbview_sethistogram(size_t instance, bool value) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       value:            %d\n", value);
	}

	/*fprintf(stderr,"%s instance:%zu\n", __func__, instance);*/

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get histogram value */
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
		fprintf(stderr, "%s instance:%zu mode:%d\n", __func__, instance, data->grid_mode);

	/* clear color status array */
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from %s\n", __func__);
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
int mbview_setshademode(size_t instance, int shade_mode) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       shade_mode:       %d\n", shade_mode);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	if (mbv_verbose >= 2)
		fprintf(stderr, "%s instance:%zu\n", __func__, instance);

	/* set mode value */
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY) {
		data->primary_shade_mode = shade_mode;
		set_mbview_shade_mode(instance, data->primary_shade_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE) {
		data->slope_shade_mode = shade_mode;
		set_mbview_shade_mode(instance, data->slope_shade_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY) {
		data->secondary_shade_mode = shade_mode;
		set_mbview_shade_mode(instance, data->secondary_shade_mode);
	}

	/* clear color status array */
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from %s\n", __func__);
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
int mbview_setcolortable(size_t instance, int colortable) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       colortable:       %d\n", colortable);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get mode value */
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARY) {
		data->primary_colortable = colortable;
		set_mbview_colortable(instance, data->primary_colortable);
		set_mbview_colortable_mode(instance, data->primary_colortable_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE) {
		data->slope_colortable = colortable;
		set_mbview_colortable(instance, data->slope_colortable);
		set_mbview_colortable_mode(instance, data->slope_colortable_mode);
	}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY) {
		data->secondary_colortable = colortable;
		set_mbview_colortable(instance, data->secondary_colortable);
		set_mbview_colortable_mode(instance, data->secondary_colortable_mode);
	}

	if (mbv_verbose >= 2)
		fprintf(stderr, "%s instance:%zu\n", __func__, instance);

	/* clear color status array */
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from %s\n", __func__);
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
int mbview_setviewtoggle(size_t instance, int toggle, bool value) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       toggle:           %d\n", toggle);
		fprintf(stderr, "dbg2       value:            %d\n", value);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);
	(void)view;  // Unused by some toggles

	switch (toggle) {
	case MBV_VIEWTOGGLE_CONTOUR: {
		data->grid_contour_mode = value ? MBV_VIEW_ON : MBV_VIEW_OFF;

		if (mbv_verbose >= 2)
			fprintf(stderr, "%s instance:%zu mode:%d\n", __func__, instance, data->grid_contour_mode);

		/* draw */
		if (mbv_verbose >= 2)
			fprintf(stderr, "Calling mbview_plotlowhigh from %s\n", __func__);
		mbview_plotlowhigh(instance);
		break;
	}
	case MBV_VIEWTOGGLE_SITE: {
		/* get mode value */
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
			fprintf(stderr, "%s instance:%zu mode:%d\n", __func__, instance, data->site_view_mode);

		/* draw */
		if (mbv_verbose >= 2)
			fprintf(stderr, "Calling mbview_plotlowhigh from %s\n", __func__);
		mbview_plotlowhigh(instance);
		break;
	}
	case MBV_VIEWTOGGLE_ROUTE: {
		/* get mode value */
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
			fprintf(stderr, "%s instance:%zu mode:%d\n", __func__, instance, data->route_view_mode);

		/* draw */
		if (mbv_verbose >= 2)
			fprintf(stderr, "Calling mbview_plotlowhigh from %s\n", __func__);
		mbview_plotlowhigh(instance);
		break;
	}
	case MBV_VIEWTOGGLE_NAV: {
		/* get mode value */
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
			fprintf(stderr, "%s instance:%zu mode:%d\n", __func__, instance, data->nav_view_mode);

		/* draw */
		if (mbv_verbose >= 2)
			fprintf(stderr, "Calling mbview_plotlowhigh from %s\n", __func__);
		mbview_plotlowhigh(instance);
		break;
	}
	case MBV_VIEWTOGGLE_NAVSWATHBOUNDS: {
		/* get mode value */
		if (value) {
			data->navswathbounds_view_mode = MBV_VIEW_ON;
		} else {
			data->navswathbounds_view_mode = MBV_VIEW_OFF;
		}

		if (mbv_verbose >= 2)
			fprintf(stderr, "%s instance:%zu mode:%d\n", __func__, instance, data->navswathbounds_view_mode);

		/* draw */
		if (mbv_verbose >= 2)
			fprintf(stderr, "Calling mbview_plotlowhigh from %s\n", __func__);
		mbview_plotlowhigh(instance);
		break;
	}
	case MBV_VIEWTOGGLE_NAVDRAPE: {
		/* get mode value */
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
			fprintf(stderr, "%s instance:%zu mode:%d\n", __func__, instance, data->navdrape_view_mode);

		/* draw */
		if (mbv_verbose >= 2)
			fprintf(stderr, "Calling mbview_plotlowhigh from %s\n", __func__);
		mbview_plotlowhigh(instance);
		break;
	}
	case MBV_VIEWTOGGLE_VECTOR: {
		/* get mode value */
		if (value) {
			data->vector_view_mode = MBV_VIEW_ON;
		} else {
			data->vector_view_mode = MBV_VIEW_OFF;
			if (data->mouse_mode == MBV_MOUSE_VECTOR) {
				data->mouse_mode = MBV_MOUSE_MOVE;
				set_mbview_mouse_mode(instance, data->mouse_mode);
			}
		}

		if (mbv_verbose >= 2)
			fprintf(stderr, "%s instance:%zu mode:%d\n", __func__, instance, data->vector_view_mode);

		/* draw */
		if (mbv_verbose >= 2)
			fprintf(stderr, "Calling mbview_plotlowhigh from %s\n", __func__);
		mbview_plotlowhigh(instance);
		break;
	}
	case MBV_VIEWTOGGLE_POLYLINE: {
		/* get mode value */
		data->polyline_view_mode = value ? MBV_VIEW_ON : MBV_VIEW_OFF;

		if (mbv_verbose >= 2)
			fprintf(stderr, "%s instance:%zu mode:%d\n", __func__, instance, data->polyline_view_mode);

		/* draw */
		mbview_plotlowhigh(instance);
		break;
	}
	case MBV_VIEWTOGGLE_POLYGON: {
		/* get mode value */
		data->polygon_view_mode = value ? MBV_VIEW_ON : MBV_VIEW_OFF;

		if (mbv_verbose >= 2)
			fprintf(stderr, "%s instance:%zu mode:%d\n", __func__, instance, data->polygon_view_mode);

		/* draw */
		mbview_plotlowhigh(instance);
		break;
	}
	case MBV_VIEWTOGGLE_GISPOINT: {
		/* get mode value */
		data->gispoint_view_mode = value ? MBV_VIEW_ON : MBV_VIEW_OFF;

		if (mbv_verbose >= 2)
			fprintf(stderr, "%s instance:%zu mode:%d\n", __func__, instance, data->gispoint_view_mode);

		/* draw */
		mbview_plotlowhigh(instance);
		break;
	}
	default:
		break;
	}

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
int mbview_resetview(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);
	if (mbv_verbose >= 2)
		fprintf(stderr, "%s\n", __func__);

	/* reset the the view to defaults */
	view->offset2d_x = 0.0;
	view->offset2d_y = 0.0;
	view->size2d = 0.0;
	view->offset3d_x = 0.0;
	view->offset3d_y = 0.0;
	view->offset3d_z = 0.0;
	view->viewoffset3d_z = 0.0;
	data->exaggeration = 1.0;
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
		fprintf(stderr, "Calling mbview_plotlowhigh from %s\n", __func__);
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
int mbview_fullrender(size_t instance) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	if (mbv_verbose >= 2)
		fprintf(stderr, "%s\n", __func__);

	/* replot in full rez if last draw was lower rez */
	if (view->lastdrawrez != MBV_REZ_FULL) {
		if (mbv_verbose >= 2)
			fprintf(stderr, "Calling mbview_plotfull from %s:\n", __func__);
		mbview_plotfull(instance);
	}

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
bool mbview_actionsensitive(size_t instance, int iaction) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       iaction:          %d\n", iaction);
	}

	/* check if all available instances are active */
	bool mbview_allactive = true;
	for (int i = 0; i < MBV_MAX_WINDOWS; i++) {
		if (!mbviews[i].data.active)
			mbview_allactive = false;
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* an action is available according to the current pick states */
	bool sensitive = false;
	if (view->actionsensitive[iaction] == MBV_PICKMASK_NONE) {
		sensitive = true;
	}
	else if (view->actionsensitive[iaction] & MBV_PICKMASK_ONEPOINT && data->pick_type == MBV_PICK_ONEPOINT) {
		sensitive = true;
	}
	else if (view->actionsensitive[iaction] & MBV_PICKMASK_TWOPOINT && data->pick_type == MBV_PICK_TWOPOINT) {
		sensitive = true;
	}
	else if (view->actionsensitive[iaction] & MBV_PICKMASK_AREA && data->area_type == MBV_AREA_QUAD) {
		sensitive = true;
	}
	else if (view->actionsensitive[iaction] & MBV_PICKMASK_REGION && data->region_type == MBV_REGION_QUAD) {
		sensitive = true;
	}
	else if (view->actionsensitive[iaction] & MBV_PICKMASK_SITE && shared.shareddata.site_selected >= 0) {
		sensitive = true;
	}
	else if (view->actionsensitive[iaction] & MBV_PICKMASK_ROUTE && shared.shareddata.route_selected >= 0) {
		sensitive = true;
	}
	else if (view->actionsensitive[iaction] & MBV_PICKMASK_NAVONEPOINT && shared.shareddata.navpick_type == MBV_PICK_ONEPOINT) {
		sensitive = true;
	}
	else if (view->actionsensitive[iaction] & MBV_PICKMASK_NAVTWOPOINT && shared.shareddata.navpick_type == MBV_PICK_TWOPOINT) {
		sensitive = true;
	}
	else if (view->actionsensitive[iaction] & MBV_PICKMASK_NAVANY) {
		for (int j = 0; j < shared.shareddata.nnav; j++) {
			if (shared.shareddata.navs[j].nselected > 0)
				sensitive = true;
		}
	}
	else if (view->actionsensitive[iaction] & MBV_EXISTMASK_SITE && shared.shareddata.nsite > 0) {
		sensitive = true;
	}
	else if (view->actionsensitive[iaction] & MBV_EXISTMASK_ROUTE && shared.shareddata.nroute > 0) {
		sensitive = true;
	}
	else if (view->actionsensitive[iaction] & MBV_EXISTMASK_POLYLINE && shared.shareddata.npolyline > 0) {
		sensitive = true;
	}
	else if (view->actionsensitive[iaction] & MBV_EXISTMASK_POLYGON && shared.shareddata.npolygon > 0) {
		sensitive = true;
	}
	else if (view->actionsensitive[iaction] & MBV_EXISTMASK_GISPOINT && shared.shareddata.ngispoint > 0) {
		sensitive = true;
	}
	if (view->actionsensitive[iaction] & MBV_STATEMASK_16 && data->state16) {
		sensitive = true;
	}
	if (view->actionsensitive[iaction] & MBV_STATEMASK_17 && data->state17) {
		sensitive = true;
	}
	if (view->actionsensitive[iaction] & MBV_STATEMASK_18 && data->state18) {
		sensitive = true;
	}
	if (view->actionsensitive[iaction] & MBV_STATEMASK_19 && data->state19) {
		sensitive = true;
	}
	if (view->actionsensitive[iaction] & MBV_STATEMASK_20 && data->state20) {
		sensitive = true;
	}
	if (view->actionsensitive[iaction] & MBV_STATEMASK_21 && data->state21) {
		sensitive = true;
	}
	if (view->actionsensitive[iaction] & MBV_STATEMASK_22 && data->state22) {
		sensitive = true;
	}
	if (view->actionsensitive[iaction] & MBV_STATEMASK_23 && data->state23) {
		sensitive = true;
	}
	if (view->actionsensitive[iaction] & MBV_STATEMASK_24 && data->state24) {
		sensitive = true;
	}
	if (view->actionsensitive[iaction] & MBV_STATEMASK_25 && data->state25) {
		sensitive = true;
	}
	if (view->actionsensitive[iaction] & MBV_STATEMASK_26 && data->state26) {
		sensitive = true;
	}
	if (view->actionsensitive[iaction] & MBV_STATEMASK_27 && data->state27) {
		sensitive = true;
	}
	if (view->actionsensitive[iaction] & MBV_STATEMASK_28 && data->state28) {
		sensitive = true;
	}
	if (view->actionsensitive[iaction] & MBV_STATEMASK_29 && data->state29) {
		sensitive = true;
	}
	if (view->actionsensitive[iaction] & MBV_STATEMASK_30 && data->state30) {
		sensitive = true;
	}
	if (view->actionsensitive[iaction] & MBV_STATEMASK_31 && data->state31) {
		sensitive = true;
	}

	/* actions that open a new window need a free window */
	if (view->actionsensitive[iaction] & MBV_PICKMASK_NEWINSTANCE && mbview_allactive) {
		sensitive = false;
	}

	return (sensitive);
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
				mbv_render_context_destroy(MBV_RENDER_PROFILE, instance);
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
				mbv_render_context_destroy(MBV_RENDER_MAP, instance);
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
			mbv_ui_destroy_window(instance);

#ifdef MBV_GET_GLX_ERRORS
			mbview_glerrorcheck(instance, __FILE__, __LINE__, __func__);
#endif
		}

		data->active = false;
		view->init = MBV_WINDOW_NULL;
		mbv_ninstance--;

		/* the node arrays are about to be freed: raise every mark first, so a
		   user still holding this instance stops reading them (mbv_marks.h) */
		mbv_marks_free(mbv_verbose, &view->marks);
		mbview_contour_free_segments(instance);

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
			shared.init_routelist = MBV_WINDOW_NULL;
			shared.init_navlist = MBV_WINDOW_NULL;
			mbv_ui_close_lists();

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
				for (int i = 0; i < shared.shareddata.nnav_alloc; i++) {
					if (shared.shareddata.navs[i].segments != NULL) {
						for (int j = 0; j < shared.shareddata.navs[i].npoints_alloc; j++) {
							if (shared.shareddata.navs[i].segments[j].lspoints != NULL)
								mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(shared.shareddata.navs[i].segments[j].lspoints),
								         error);
						}
						mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(shared.shareddata.navs[i].segments), error);
					}
					if (shared.shareddata.navs[i].navpts != NULL)
						mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(shared.shareddata.navs[i].navpts), error);
				}
				status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(shared.shareddata.navs), error);
				shared.shareddata.nnav_alloc = 0;
				shared.shareddata.navs = NULL;
			}
			if (status == MB_SUCCESS && shared.shareddata.nvector_alloc != 0 && shared.shareddata.vectors != NULL) {
				for (int i = 0; i < shared.shareddata.nvector_alloc; i++) {
					if (shared.shareddata.vectors[i].segments != NULL) {
						for (int j = 0; j < shared.shareddata.vectors[i].npoints_alloc; j++) {
							if (shared.shareddata.vectors[i].segments[j].lspoints != NULL)
								mb_freed(mbv_verbose, __FILE__, __LINE__,
								         (void **)&(shared.shareddata.vectors[i].segments[j].lspoints), error);
						}
						mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(shared.shareddata.vectors[i].segments), error);
					}
					if (shared.shareddata.vectors[i].vectorpts != NULL)
						mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(shared.shareddata.vectors[i].vectorpts), error);
				}
				status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(shared.shareddata.vectors), error);
				shared.shareddata.nvector = 0;
				shared.shareddata.nvector_alloc = 0;
				shared.shareddata.vectors = NULL;
			}
			if (status == MB_SUCCESS && shared.shareddata.polylines != NULL) {
				status = mbview_deleteallpolylines(mbv_verbose, instance, error);
				mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(shared.shareddata.polylines), error);
				shared.shareddata.npolyline_alloc = 0;
			}
			if (status == MB_SUCCESS && shared.shareddata.polygons != NULL) {
				status = mbview_deleteallpolygons(mbv_verbose, instance, error);
				mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(shared.shareddata.polygons), error);
				shared.shareddata.npolygon_alloc = 0;
			}
			if (status == MB_SUCCESS && shared.shareddata.gispoints != NULL) {
				status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(shared.shareddata.gispoints), error);
				shared.shareddata.ngispoint = 0;
				shared.shareddata.ngispoint_alloc = 0;
			}
			if (status == MB_SUCCESS && shared.shareddata.gislayers != NULL) {
				status = mb_freed(mbv_verbose, __FILE__, __LINE__, (void **)&(shared.shareddata.gislayers), error);
				shared.shareddata.ngislayer = 0;
				shared.shareddata.ngislayer_alloc = 0;
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
int mbview_colorboundsapply(size_t instance, const struct mbview_colorbounds_values *values) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}


	if (mbv_verbose >= 2)
		fprintf(stderr, "%s: instance:%zu\n", __func__, instance);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);


	/* get values of widgets */

	bool change = false;

	double dvalue;
	if (sscanf(values->datamin, "%lf", &dvalue) == 1 && dvalue != data->primary_colortable_min) {
		data->primary_colortable_min = dvalue;
		view->primary_histogram_set = false;
		if (data->grid_mode == MBV_GRID_VIEW_PRIMARY)
			change = true;
	}

	if (sscanf(values->datamax, "%lf", &dvalue) == 1 && dvalue != data->primary_colortable_max) {
		data->primary_colortable_max = dvalue;
		view->primary_histogram_set = false;
		if (data->grid_mode == MBV_GRID_VIEW_PRIMARY)
			change = true;
	}

	if (sscanf(values->contours, "%lf", &dvalue) == 1 && dvalue > 0.0 && dvalue != data->contour_interval) {
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

	int ivalue = values->data_htoc;
	if (ivalue != data->primary_colortable_mode) {
		data->primary_colortable_mode = ivalue;
		if (data->grid_mode == MBV_GRID_VIEW_PRIMARY)
			change = true;
	}

	if (sscanf(values->slopemin, "%lf", &dvalue) == 1 && dvalue != data->slope_colortable_min) {
		data->slope_colortable_min = dvalue;
		view->primaryslope_histogram_set = false;
		if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE)
			change = true;
	}

	if (sscanf(values->slopemax, "%lf", &dvalue) == 1 && dvalue != data->slope_colortable_max) {
		data->slope_colortable_max = dvalue;
		view->primaryslope_histogram_set = false;
		if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE)
			change = true;
	}

	ivalue = values->slope_htoc;
	if (ivalue != data->slope_colortable_mode) {
		data->slope_colortable_mode = ivalue;
		if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE)
			change = true;
	}

	if (sscanf(values->overlaymin, "%lf", &dvalue) == 1 && dvalue != data->secondary_colortable_min) {
		data->secondary_colortable_min = dvalue;
		view->secondary_histogram_set = false;
		if (data->grid_mode == MBV_GRID_VIEW_SECONDARY)
			change = true;
	}

	if (sscanf(values->overlaymax, "%lf", &dvalue) == 1 && dvalue != data->secondary_colortable_max) {
		data->secondary_colortable_max = dvalue;
		view->secondary_histogram_set = false;
		if (data->grid_mode == MBV_GRID_VIEW_SECONDARY)
			change = true;
	}

	ivalue = values->overlay_htoc;
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
		fprintf(stderr, "Calling mbview_plotlowhigh from %s\n", __func__);
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
int mbview_shadeparmsapply(size_t instance, const struct mbview_shadeparms_values *values) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}


	if (mbv_verbose >= 2)
		fprintf(stderr, "%s: instance:%zu\n", __func__, instance);

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

	double dvalue;
	if (sscanf(values->illum_amp, "%lf", &dvalue) == 1 && dvalue != data->illuminate_magnitude) {
		data->illuminate_magnitude = dvalue;
		if (shade_mode == MBV_SHADE_VIEW_ILLUMINATION)
			change = true;
	}

	if (sscanf(values->illum_azi, "%lf", &dvalue) == 1 && dvalue != data->illuminate_azimuth) {
		data->illuminate_azimuth = dvalue;
		if (shade_mode == MBV_SHADE_VIEW_ILLUMINATION)
			change = true;
	}

	if (sscanf(values->illum_elev, "%lf", &dvalue) == 1 && dvalue != data->illuminate_elevation) {
		data->illuminate_elevation = dvalue;
		if (shade_mode == MBV_SHADE_VIEW_ILLUMINATION)
			change = true;
	}

	if (sscanf(values->slope_amp, "%lf", &dvalue) == 1 && dvalue != data->slope_magnitude) {
		data->slope_magnitude = dvalue;
		if (shade_mode == MBV_SHADE_VIEW_SLOPE)
			change = true;
	}

	if (sscanf(values->overlay_amp, "%lf", &dvalue) == 1 && dvalue != data->overlay_shade_magnitude) {
		data->overlay_shade_magnitude = dvalue;
		if (shade_mode == MBV_SHADE_VIEW_OVERLAY)
			change = true;
	}

	if (sscanf(values->overlay_center, "%lf", &dvalue) == 1 && dvalue != data->overlay_shade_center) {
		data->overlay_shade_center = dvalue;
		if (shade_mode == MBV_SHADE_VIEW_OVERLAY)
			change = true;
	}

	int ivalue = values->overlay_shade_htoc;
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
		fprintf(stderr, "Calling mbview_plotlowhigh from %s\n", __func__);
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
int mbview_3dparmsapply(size_t instance, const struct mbview_3dparms_values *values) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}


	if (mbv_verbose >= 2)
		fprintf(stderr, "%s: instance:%zu\n", __func__, instance);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get values of widgets */

	bool change = false;

	double dvalue;
	if (sscanf(values->model_azimuth, "%lf", &dvalue) == 1 && dvalue != data->modelazimuth3d) {
		data->modelazimuth3d = dvalue;
		change = true;
	}

	if (sscanf(values->model_elevation, "%lf", &dvalue) == 1 && dvalue != data->modelelevation3d) {
		data->modelelevation3d = dvalue;
		change = true;
	}

	if (sscanf(values->view_azimuth, "%lf", &dvalue) == 1 && dvalue != data->viewazimuth3d) {
		data->viewazimuth3d = dvalue;
		change = true;
	}

	if (sscanf(values->view_elevation, "%lf", &dvalue) == 1 && dvalue != data->viewelevation3d) {
		data->viewelevation3d = dvalue;
		change = true;
	}

	if (sscanf(values->exaggeration, "%lf", &dvalue) == 1 && dvalue != data->exaggeration) {
		data->exaggeration = dvalue;
		if (data->display_projection_mode != MBV_PROJECTION_SPHEROID) {
			view->zorigin = data->exaggeration * 0.5 * (data->primary_min + data->primary_max);
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

	if (sscanf(values->view_3doffsetx, "%lf", &dvalue) == 1 && dvalue != view->offset3d_x) {
		view->offset3d_x = dvalue;
		change = true;
	}

	if (sscanf(values->view_3doffsety, "%lf", &dvalue) == 1 && dvalue != view->offset3d_y) {
		view->offset3d_y = dvalue;
		change = true;
	}

	if (sscanf(values->model_3dzoom, "%lf", &dvalue) == 1 && dvalue != view->offset3d_z) {
		view->offset3d_z = dvalue;
		change = true;
	}

	if (sscanf(values->view_3dzoom, "%lf", &dvalue) == 1 && dvalue != view->viewoffset3d_z) {
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
		fprintf(stderr, "Calling mbview_plotlowhigh from %s\n", __func__);
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
int mbview_2dparmsapply(size_t instance, const struct mbview_2dparms_values *values) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
	}


	if (mbv_verbose >= 2)
		fprintf(stderr, "%s: instance:%zu\n", __func__, instance);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* get values of widgets */

	bool change = false;

	double dvalue;
	if (sscanf(values->view_2doffsetx, "%lf", &dvalue) == 1 && dvalue != view->offset2d_x) {
		view->offset2d_x = dvalue;
		change = true;
	}

	if (sscanf(values->view_2doffsety, "%lf", &dvalue) == 1 && dvalue != view->offset2d_y) {
		view->offset2d_y = dvalue;
		change = true;
	}

	if (sscanf(values->view_2dzoom, "%lf", &dvalue) == 1 && dvalue != view->size2d) {
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
		fprintf(stderr, "Calling mbview_plotlowhigh from %s\n", __func__);
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
int mbview_setresolution(size_t instance, int lorez_dimension, int hirez_dimension, int lorez_navdecimate, int hirez_navdecimate) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       lorez_dimension:  %d\n", lorez_dimension);
		fprintf(stderr, "dbg2       hirez_dimension:  %d\n", hirez_dimension);
		fprintf(stderr, "dbg2       lorez_navdecimate: %d\n", lorez_navdecimate);
		fprintf(stderr, "dbg2       hirez_navdecimate: %d\n", hirez_navdecimate);
	}


	if (mbv_verbose >= 2)
		fprintf(stderr, "%s: instance:%zu\n", __func__, instance);

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);


	/* make dimensions even multiples of 10 */
	if (lorez_dimension > hirez_dimension)
		hirez_dimension = lorez_dimension;
	data->lorez_dimension = 25 * ((int)((lorez_dimension + 12.5) / 25));
	data->hirez_dimension = 25 * ((int)((hirez_dimension + 12.5) / 25));

	/* show the rounded dimensions */
	mbv_ui_view_values_changed(instance, MBV_VALUES_RESOLUTION);

	/* set nav decimation */
	data->lorez_navdecimate = lorez_navdecimate;
	data->hirez_navdecimate = hirez_navdecimate;

	/* the stride a renderer should draw comes from these dimensions
	   (mbv_marks.h); the clears below raise the position and colour marks */
	mbv_marks_drawn(&view->marks);

	/* reset status flags and arrays */
	view->lastdrawrez = MBV_REZ_NONE;
	mbview_setcolorparms(instance);
	mbview_colorclear(instance);
	mbview_zscaleclear(instance);
	view->contourlorez = false;
	view->contourhirez = false;
	view->contourfullrez = false;

	if (mbv_verbose >= 2)
		fprintf(stderr, "%s instance:%zu resolutions: %d %d decimations: %d %d\n", __func__, instance,
		        data->lorez_dimension, data->hirez_dimension, data->lorez_navdecimate, data->hirez_navdecimate);

	/* draw */
	if (mbv_verbose >= 2)
		fprintf(stderr, "Calling mbview_plotlowhigh from %s\n", __func__);
	mbview_plotlowhigh(instance);

	/* notify parent program of color change */
	if (data->mbview_colorchange_notify != NULL)
		(data->mbview_colorchange_notify)(instance);

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
int mbview_setdisplayprojection(size_t instance, int projection) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       projection:       %d\n", projection);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);
	int projectionid, utmzone;
	double reference_lon;
	double reference_lat;

	data->display_projection_mode = projection;
	view->plot_done = false;
	view->projected = false;
	view->globalprojected = false;

	/* every node will be projected again into new display coordinates
	   (mbv_marks.h) */
	mbv_marks_everything(&view->marks);
	view->viewboundscount = MBV_BOUNDSFREQUENCY;

	/* UTM: the zone of the grid centre */
	if (projection == MBV_PROJECTION_PROJECTED) {
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
	}

	/* set label */
	do_mbview_set_projection_label(instance);

	/* clear zscale for grid */
	mbview_zscaleclear(instance);

	/* project and scale data other than the grid */
	mbview_zscale(instance);

	/* draw */
	fprintf(stderr, "Calling mbview_plotlowhigh from %s\n", __func__);
	mbview_plotlowhigh(instance);

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
int mbview_setlonlatstyle(size_t instance, int style) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       style:            %d\n", style);
	}

	/* lon/lat style of the pick annotation */
	shared.lonlatstyle = style;
	mbview_pick_text(instance);

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
void do_mbview_set_projection_label(size_t instance) {
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* set projection label */
  char value_text[4 * MB_PATH_MAXLINE];
	snprintf(value_text, sizeof(value_text), ":::t\"Primary Grid Projection:\"");
	if (data->primary_grid_projection_mode == MBV_PROJECTION_GEOGRAPHIC) {
		strncat(value_text, ":t\"  Geographic\"", sizeof(value_text) - strlen(value_text) - 1);
	}
	else if (data->primary_grid_projection_mode == MBV_PROJECTION_PROJECTED ||
	         data->primary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED) {
		int projectionid;
		char tmptext[MB_PATH_MAXLINE*2];
		if (sscanf(data->primary_grid_projection_id, "epsg%d", &projectionid) == 1 && projectionid == 32661) {
			snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\":t\"    North Polar Steographic\"", data->primary_grid_projection_id);
			strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
		}
		else if (sscanf(data->primary_grid_projection_id, "EPSG:%d", &projectionid) == 1 && projectionid == 32661) {
			snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\":t\"    North Polar Steographic\"", data->primary_grid_projection_id);
			strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
		}
		else if (sscanf(data->primary_grid_projection_id, "epsg%d", &projectionid) == 1 && projectionid == 32761) {
			snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\":t\"    South Polar Steographic\"", data->primary_grid_projection_id);
			strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
		}
		else if (sscanf(data->primary_grid_projection_id, "EPSG:%d", &projectionid) == 1 && projectionid == 32761) {
			snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\":t\"    South Polar Steographic\"", data->primary_grid_projection_id);
			strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
		}
		else if (sscanf(data->primary_grid_projection_id, "epsg%d", &projectionid) == 1 && projectionid >= 32600 &&
		         projectionid < 32700) {
			snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\":t\"    UTM Zone %d N\"", data->primary_grid_projection_id,
			        projectionid - 32600);
			strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
		}
		else if (sscanf(data->primary_grid_projection_id, "EPSG:%d", &projectionid) == 1 && projectionid >= 32600 &&
		         projectionid < 32700) {
			snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\":t\"    UTM Zone %d N\"", data->primary_grid_projection_id,
			        projectionid - 32600);
			strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
		}
		else if (sscanf(data->primary_grid_projection_id, "epsg%d", &projectionid) == 1 && projectionid >= 32700 &&
		         projectionid < 32800) {
			snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\":t\"    UTM Zone %d S\"", data->primary_grid_projection_id,
			        projectionid - 32700);
			strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
		}
		else if (sscanf(data->primary_grid_projection_id, "EPSG:%d", &projectionid) == 1 && projectionid >= 32700 &&
		         projectionid < 32800) {
			snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\":t\"    UTM Zone %d S\"", data->primary_grid_projection_id,
			        projectionid - 32700);
			strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
		}
		else {
			snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\"", data->primary_grid_projection_id);
			strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
		}
	}
	else if (data->primary_grid_projection_mode == MBV_PROJECTION_SPHEROID) {
		strncat(value_text, ":t\"  Spheroid\"", sizeof(value_text) - strlen(value_text) - 1);
	}

	if (data->secondary_nxy > 0) {
		strncat(value_text, ":t\"Secondary Grid Projection:\"", sizeof(value_text) - strlen(value_text) - 1);
		if (data->secondary_grid_projection_mode == MBV_PROJECTION_GEOGRAPHIC) {
			strncat(value_text, ":t\"  Geographic\"", sizeof(value_text) - strlen(value_text) - 1);
		}
		else if (data->secondary_grid_projection_mode == MBV_PROJECTION_PROJECTED ||
		         data->secondary_grid_projection_mode == MBV_PROJECTION_ALREADYPROJECTED) {
			int projectionid;
			char tmptext[MB_PATH_MAXLINE*2];
			if (sscanf(data->secondary_grid_projection_id, "epsg%d", &projectionid) == 1 && projectionid == 32661) {
				snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\":t\"    North Polar Steographic\"", data->secondary_grid_projection_id);
				strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
			}
			else if (sscanf(data->secondary_grid_projection_id, "EPSG:%d", &projectionid) == 1 && projectionid == 32661) {
				snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\":t\"    North Polar Steographic\"", data->secondary_grid_projection_id);
				strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
			}
			else if (sscanf(data->secondary_grid_projection_id, "epsg%d", &projectionid) == 1 && projectionid == 32761) {
				snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\":t\"    South Polar Steographic\"", data->secondary_grid_projection_id);
				strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
			}
			else if (sscanf(data->secondary_grid_projection_id, "EPSG:%d", &projectionid) == 1 && projectionid == 32761) {
				snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\":t\"    South Polar Steographic\"", data->secondary_grid_projection_id);
				strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
			}
			else if (sscanf(data->secondary_grid_projection_id, "epsg%d", &projectionid) == 1 && projectionid >= 32600 &&
			         projectionid < 32700) {
				snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\":t\"    UTM Zone %d N\"", data->secondary_grid_projection_id,
				        projectionid - 32600);
				strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
			}
			else if (sscanf(data->secondary_grid_projection_id, "EPSG:%d", &projectionid) == 1 && projectionid >= 32600 &&
			         projectionid < 32700) {
				snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\":t\"    UTM Zone %d N\"", data->secondary_grid_projection_id,
				        projectionid - 32600);
				strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
			}
			else if (sscanf(data->secondary_grid_projection_id, "epsg%d", &projectionid) == 1 && projectionid >= 32700 &&
			         projectionid < 32800) {
				snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\":t\"    UTM Zone %d S\"", data->secondary_grid_projection_id,
				        projectionid - 32700);
				strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
			}
			else if (sscanf(data->secondary_grid_projection_id, "EPSG:%d", &projectionid) == 1 && projectionid >= 32700 &&
			         projectionid < 32800) {
				snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\":t\"    UTM Zone %d S\"", data->secondary_grid_projection_id,
				        projectionid - 32700);
				strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
			}
			else {
				snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\"", data->secondary_grid_projection_id);
				strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
			}
		}
		else if (data->secondary_grid_projection_mode == MBV_PROJECTION_SPHEROID) {
			strncat(value_text, ":t\"  Spheroid\"", sizeof(value_text) - strlen(value_text) - 1);
		}
	}

	strncat(value_text, ":t\"Display Grid Projection:\"", sizeof(value_text) - strlen(value_text) - 1);
	if (data->display_projection_mode == MBV_PROJECTION_GEOGRAPHIC) {
		strncat(value_text, ":t\"  Geographic\"", sizeof(value_text) - strlen(value_text) - 1);
	}
	else if (data->display_projection_mode == MBV_PROJECTION_PROJECTED ||
	         data->display_projection_mode == MBV_PROJECTION_ALREADYPROJECTED) {
		int projectionid;
		double lon_origin;
		double lat_origin;
		char tmptext[MB_PATH_MAXLINE*2];
		if (sscanf(data->display_projection_id, "LTM%lf/%lf", &lon_origin, &lat_origin) == 2) {
			snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\":t\"    Local Transverse Mercator with origin %.5f %.5f\"", 
								data->display_projection_id, lon_origin, lat_origin);
			strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
		}
		else if (sscanf(data->display_projection_id, "epsg%d", &projectionid) == 1 && projectionid == 32661) {
			snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\":t\"    North Polar Steographic\"", data->display_projection_id);
			strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
		}
		else if (sscanf(data->display_projection_id, "EPSG:%d", &projectionid) == 1 && projectionid == 32661) {
			snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\":t\"    North Polar Steographic\"", data->display_projection_id);
			strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
		}
		else if (sscanf(data->display_projection_id, "epsg%d", &projectionid) == 1 && projectionid == 32761) {
			snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\":t\"    South Polar Steographic\"", data->display_projection_id);
			strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
		}
		else if (sscanf(data->display_projection_id, "EPSG:%d", &projectionid) == 1 && projectionid == 32761) {
			snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\":t\"    South Polar Steographic\"", data->display_projection_id);
			strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
		}
		else if (sscanf(data->display_projection_id, "epsg%d", &projectionid) == 1 && projectionid >= 32600 &&
		         projectionid < 32700) {
			snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\":t\"    UTM Zone %d N\"", data->display_projection_id, projectionid - 32600);
			strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
		}
		else if (sscanf(data->display_projection_id, "EPSG:%d", &projectionid) == 1 && projectionid >= 32600 &&
		         projectionid < 32700) {
			snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\":t\"    UTM Zone %d N\"", data->display_projection_id, projectionid - 32600);
			strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
		}
		else if (sscanf(data->display_projection_id, "epsg%d", &projectionid) == 1 && projectionid >= 32700 &&
		         projectionid < 32800) {
			snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\":t\"    UTM Zone %d S\"", data->display_projection_id, projectionid - 32700);
			strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
		}
		else if (sscanf(data->display_projection_id, "EPSG:%d", &projectionid) == 1 && projectionid >= 32700 &&
		         projectionid < 32800) {
			snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\":t\"    UTM Zone %d S\"", data->display_projection_id, projectionid - 32700);
			strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
		}
		else {
			snprintf(tmptext, sizeof(tmptext), ":t\"  Projected: %s\"", data->display_projection_id);
			strncat(value_text, tmptext, sizeof(value_text) - strlen(value_text) - 1);
		}
	}
	else if (data->display_projection_mode == MBV_PROJECTION_SPHEROID) {
		strncat(value_text, ":t\"  Spheroid\"", sizeof(value_text) - strlen(value_text) - 1);
	}
	set_mbview_label_multiline_string(view->mb3dview.mbview_label_projection, value_text);
}

/*------------------------------------------------------------------------------*/
/* a profile control (MBV_PROFILE_CONTROL_*) with the value of its slider, as
    do_mbview_profile_width, do_mbview_profile_slope and do_mbview_profile_exagger
    set it; the caller sizes and draws the profile again */
int mbview_setprofilecontrol(size_t instance, int control, int value) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       instance:         %zu\n", instance);
		fprintf(stderr, "dbg2       control:          %d\n", control);
		fprintf(stderr, "dbg2       value:            %d\n", value);
	}

	struct mbview_struct *data = &(mbviews[instance].data);

	switch (control) {
	case MBV_PROFILE_CONTROL_WIDTH: {
		const int profile_widthfactor = value;
		data->profile_widthfactor = profile_widthfactor;
		break;
	}
	case MBV_PROFILE_CONTROL_SLOPE: {
		const int profile_slopethreshold = value;
		data->profile_slopethreshold = 0.01 * profile_slopethreshold;
		break;
	}
	case MBV_PROFILE_CONTROL_EXAGGERATION: {
		const int profile_exagger = value;
		data->profile_exaggeration = 0.1 * profile_exagger;
		break;
	}
	default:
		break;
	}

	return (MB_SUCCESS);
}
