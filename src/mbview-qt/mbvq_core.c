/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_core.c  11/7/2026
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
 * C half of the mbview-qt library: the mbview functions that the Motif
 * library implements in mbview_callbacks.c, mbv_ui_motif.c,
 * mbview_gislist.c and mb3dsoundings_callbacks.c, written without Motif.
 * Compiled like the mbview core files, with MBVIEW_NO_MOTIF. Everything
 * the windows have to show goes through mbvq_bridge.h to the Qt half.
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_status.h"

/* the private headers keep a Windows OpenGL context type (HGLRC) */
#ifdef _WIN32
#include <windows.h>
#endif

#include "mbv_xtypes.h"
#include "MB3DView.h"
#include "MB3DSiteList.h"
#include "MB3DRouteList.h"
#include "MB3DNavList.h"
#include "Mb3dsdg.h"
#include "mb3dsoundingsprivate.h"
#include "mbview.h"
#include "mbviewprivate.h"
#include "mbv_render.h"
#include "mbv_ui.h"

#include "mbvq_bridge.h"

/* the Qt half uses the MBVQ_ copies of these values (compile-time checks) */
typedef char mbvq_check_max_windows[(MBVQ_MAX_WINDOWS == MBV_MAX_WINDOWS) ? 1 : -1];
typedef char mbvq_check_no_window[(MBVQ_NO_WINDOW == MBV_NO_WINDOW) ? 1 : -1];
typedef char mbvq_check_rez[(MBVQ_REZ_NONE == MBV_REZ_NONE && MBVQ_REZ_LOW == MBV_REZ_LOW && MBVQ_REZ_HIGH == MBV_REZ_HIGH &&
                             MBVQ_REZ_FULL == MBV_REZ_FULL)
                                ? 1
                                : -1];

/* Distinct stand-ins for the label widgets the core writes to, so that
   set_mbview_label_multiline_string() can tell them apart. They are
   never dereferenced. */
static char mbvq_label_token[MBV_MAX_WINDOWS][MBVQ_LABEL_OTHER];

/*------------------------------------------------------------------------------*/
int mbview_startup(int verbose, Widget parent, XtAppContext app, int *error) {
	mbv_verbose = verbose;

	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                 %d\n", verbose);
		fprintf(stderr, "dbg2       parent:                  %p\n", (void *)parent);
		fprintf(stderr, "dbg2       app:                     %p\n", (void *)app);
	}

	/* no Motif parent widget or application context */
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

	/* the renderer is installed by the Qt half (mbvq::set_backend) */

	/* initialize shared data */
	mbview_reset_shared(true);

	/* initialize windows */
	for (int i = 0; i < MBV_MAX_WINDOWS; i++) {
		mbview_reset(i);
	}

	/* the site, route, navigation and GIS list windows come later */
	shared.init_sitelist = MBV_WINDOW_NULL;
	shared.init_routelist = MBV_WINDOW_NULL;
	shared.init_navlist = MBV_WINDOW_NULL;

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
int mbview_open(int verbose, size_t instance, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	const int status = mbvq_c_open(verbose, instance, error);

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
int mbvq_c_open(int verbose, size_t instance, int *error) {
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* set active */
	data->active = true;

	/* label stand-ins, then the window */
	view->mb3dview.mbview_label_pickinfo = (Widget)&mbvq_label_token[instance][MBVQ_LABEL_PICKINFO];
	view->mb3dview.mbview_profile_label_info = (Widget)&mbvq_label_token[instance][MBVQ_LABEL_PROFILEINFO];
	view->mb3dview.mbview_label_about_version = (Widget)&mbvq_label_token[instance][MBVQ_LABEL_ABOUTVERSION];
	if (view->init != MBV_WINDOW_VISIBLE) {
		mbvq_qt_open_window(instance);
		view->init = MBV_WINDOW_VISIBLE;
	}

	/* make sure some key parameters are set, as mbview_open() does */
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
		if (data->primary_max > data->primary_min)
			data->contour_interval = pow(10.0, floor(log10(data->primary_max - data->primary_min)) - 1.0);
	}

	/* set about version label */
	mb_path value_text;
	snprintf(value_text, sizeof(value_text), "::#TimesMedium14:t\"MB-System Release %s\"#TimesMedium14\"%s\"", MB_VERSION,
	         MB_VERSION_DATE);
	set_mbview_label_multiline_string(view->mb3dview.mbview_label_about_version, value_text);

	/* set widgets */
	mbview_set_widgets(verbose, instance, error);

	/* set widget sensitivity */
	mbview_update_sensitivity(verbose, instance, error);
	mbview_action_sensitivityall();

	/* The OpenGL context belongs to the Qt map view, which keeps it across
	   resizes; mbview_reset_glx() is not needed. The core draws only while
	   glx_init is set, which the null backend leaves false. */
	view->glx_init = mbvq_qt_drawable();
	view->aspect_ratio = ((float)data->width) / ((float)data->height);

	return (MB_SUCCESS);
}

/*------------------------------------------------------------------------------*/
void mbvq_c_resize(size_t instance, int width, int height) {
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* what mbview_reset_glx() sets for a new size; the display lists stay */
	data->width = width;
	data->height = height > 0 ? height : 1;
	view->aspect_ratio = ((float)data->width) / ((float)data->height);
	view->lastdrawrez = MBV_REZ_NONE;
}

/*------------------------------------------------------------------------------*/
int mbvq_c_plot(size_t instance, int rez) {
	if (rez == MBV_REZ_LOW)
		return (mbview_plotlow(instance));
	if (rez == MBV_REZ_HIGH)
		return (mbview_plothigh(instance));
	return (mbview_plotfull(instance));
}

/*------------------------------------------------------------------------------*/
int mbvq_c_lastdrawrez(size_t instance) { return (mbviews[instance].lastdrawrez); }

void mbvq_c_work(size_t instance) { do_mbview_workfunction((XtPointer)instance); }

int mbvq_c_mouse_mode(size_t instance) { return (mbviews[instance].data.mouse_mode); }

bool mbvq_c_display_3d(size_t instance) { return (mbviews[instance].data.display_mode == MBV_DISPLAY_3D); }

void mbvq_c_set_mouse_mode(size_t instance, int mode) { mbview_setmousemode(instance, mode); }

const char *mbvq_c_title(size_t instance) { return (mbviews[instance].data.title); }

void mbvq_c_geometry(size_t instance, int *xo, int *yo, int *width, int *height) {
	const struct mbview_struct *data = &(mbviews[instance].data);
	*xo = data->xo;
	*yo = data->yo;
	*width = data->width;
	*height = data->height;
}

void mbvq_c_destroy(size_t instance) {
	/* as do_mbview_dismiss() */
	const struct mbview_struct *data = &(mbviews[instance].data);
	if (data->active) {
		int error = MB_ERROR_NO_ERROR;
		mbview_destroy(mbv_verbose, instance, true, &error);
	}
}

void mbvq_c_clearpicks(size_t instance) { mbview_clearpicks(instance); }

void mbvq_c_resetview(size_t instance) { mbview_resetview(instance); }

void mbvq_c_fullrender(size_t instance) {
	/* The background work draws at full resolution about a second after the
	   last change, so the button often has nothing to do; say so */
	if (mbviews[instance].lastdrawrez == MBV_REZ_FULL)
		mbvq_qt_status(instance, "Already at full resolution");
	else
		mbview_fullrender(instance);
}

/*------------------------------------------------------------------------------*/
/* What the window shows. The checked states come from the data; the enabled
   states follow mbview_update_sensitivity() and set_mbview_mouse_mode() of the
   Motif user interface. */
void mbvq_c_state(size_t instance, struct mbvq_state *state) {
	const struct mbview_struct *data = &(mbviews[instance].data);
	const struct mbview_shareddata_struct *sd = &(shared.shareddata);
	memset(state, 0, sizeof(*state));

	/* settings of the grid shown */
	bool histogram = data->primary_histogram;
	int shade_mode = data->primary_shade_mode;
	int colortable = data->primary_colortable;
	if (data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE) {
		histogram = data->primaryslope_histogram;
		shade_mode = data->slope_shade_mode;
		colortable = data->slope_colortable;
	}
	else if (data->grid_mode == MBV_GRID_VIEW_SECONDARY) {
		histogram = data->secondary_histogram;
		shade_mode = data->secondary_shade_mode;
		colortable = data->secondary_colortable;
	}

	bool *c = state->view_checked;
	c[MBVQ_VIEW_DISPLAY_2D] = data->display_mode == MBV_DISPLAY_2D;
	c[MBVQ_VIEW_DISPLAY_3D] = data->display_mode == MBV_DISPLAY_3D;
	c[MBVQ_VIEW_GRID_PRIMARY] = data->grid_mode == MBV_GRID_VIEW_PRIMARY;
	c[MBVQ_VIEW_GRID_PRIMARYSLOPE] = data->grid_mode == MBV_GRID_VIEW_PRIMARYSLOPE;
	c[MBVQ_VIEW_GRID_SECONDARY] = data->grid_mode == MBV_GRID_VIEW_SECONDARY;
	c[MBVQ_VIEW_HISTOGRAM] = histogram;
	c[MBVQ_VIEW_SHADE_NONE] = shade_mode == MBV_SHADE_VIEW_NONE;
	c[MBVQ_VIEW_SHADE_ILLUMINATION] = shade_mode == MBV_SHADE_VIEW_ILLUMINATION;
	c[MBVQ_VIEW_SHADE_SLOPE] = shade_mode == MBV_SHADE_VIEW_SLOPE;
	c[MBVQ_VIEW_SHADE_OVERLAY] = shade_mode == MBV_SHADE_VIEW_OVERLAY;
	c[MBVQ_VIEW_CONTOUR] = data->grid_contour_mode == MBV_VIEW_ON;
	c[MBVQ_VIEW_SITE] = data->site_view_mode == MBV_VIEW_ON;
	c[MBVQ_VIEW_ROUTE] = data->route_view_mode == MBV_VIEW_ON;
	c[MBVQ_VIEW_NAV] = data->nav_view_mode == MBV_VIEW_ON;
	c[MBVQ_VIEW_NAVSWATHBOUNDS] = data->navswathbounds_view_mode == MBV_VIEW_ON;
	c[MBVQ_VIEW_NAVDRAPE] = data->navdrape_view_mode == MBV_VIEW_ON;
	c[MBVQ_VIEW_VECTOR] = data->vector_view_mode == MBV_VIEW_ON;
	c[MBVQ_VIEW_POLYLINE] = data->polyline_view_mode == MBV_VIEW_ON;
	c[MBVQ_VIEW_POLYGON] = data->polygon_view_mode == MBV_VIEW_ON;
	c[MBVQ_VIEW_GISPOINT] = data->gispoint_view_mode == MBV_VIEW_ON;
	c[MBVQ_VIEW_COLORTABLE_HAXBY] = colortable == MBV_COLORTABLE_HAXBY;
	c[MBVQ_VIEW_COLORTABLE_BRIGHT] = colortable == MBV_COLORTABLE_BRIGHT;
	c[MBVQ_VIEW_COLORTABLE_MUTED] = colortable == MBV_COLORTABLE_MUTED;
	c[MBVQ_VIEW_COLORTABLE_GRAY] = colortable == MBV_COLORTABLE_GRAY;
	c[MBVQ_VIEW_COLORTABLE_FLAT] = colortable == MBV_COLORTABLE_FLAT;
	c[MBVQ_VIEW_COLORTABLE_SEALEVEL1] = colortable == MBV_COLORTABLE_SEALEVEL1;
	c[MBVQ_VIEW_COLORTABLE_SEALEVEL2] = colortable == MBV_COLORTABLE_SEALEVEL2;
	c[MBVQ_VIEW_PROFILE] = false;

	const bool primary = data->primary_nxy > 0 && data->primary_data != NULL;
	const bool secondary = data->secondary_nxy > 0 && data->secondary_data != NULL;
	bool *e = state->view_enabled;
	for (int i = 0; i < MBVQ_VIEW_ITEMS; i++)
		e[i] = true;
	e[MBVQ_VIEW_DISPLAY_2D] = data->display_projection_mode != MBV_PROJECTION_SPHEROID;
	e[MBVQ_VIEW_GRID_PRIMARY] = primary;
	e[MBVQ_VIEW_GRID_PRIMARYSLOPE] = primary;
	e[MBVQ_VIEW_HISTOGRAM] = primary;
	e[MBVQ_VIEW_SHADE_NONE] = primary;
	e[MBVQ_VIEW_SHADE_ILLUMINATION] = primary;
	e[MBVQ_VIEW_SHADE_SLOPE] = primary;
	e[MBVQ_VIEW_CONTOUR] = primary;
	e[MBVQ_VIEW_GRID_SECONDARY] = secondary;
	e[MBVQ_VIEW_SHADE_OVERLAY] = secondary;
	e[MBVQ_VIEW_SITE] = sd->site_mode != MBV_SITE_OFF;
	e[MBVQ_VIEW_ROUTE] = sd->route_mode != MBV_ROUTE_OFF;
	e[MBVQ_VIEW_NAV] = sd->nav_mode != MBV_NAV_OFF;
	e[MBVQ_VIEW_NAVSWATHBOUNDS] = sd->nav_mode != MBV_NAV_OFF;
	e[MBVQ_VIEW_NAVDRAPE] = sd->nav_mode != MBV_NAV_OFF;
	e[MBVQ_VIEW_VECTOR] = sd->vector_mode != MBV_VECTOR_OFF;
	e[MBVQ_VIEW_POLYLINE] = sd->polyline_mode != MBV_POLYLINE_OFF;
	e[MBVQ_VIEW_POLYGON] = sd->polygon_mode != MBV_POLYGON_OFF;
	e[MBVQ_VIEW_GISPOINT] = sd->gispoint_mode != MBV_GISPOINT_OFF;
	/* the profile window comes in step 3e */
	e[MBVQ_VIEW_PROFILE] = false;

	state->mouse_mode = data->mouse_mode;
	const bool display_3d = data->display_mode == MBV_DISPLAY_3D;
	for (int i = 0; i < MBVQ_MOUSE_MODES; i++)
		state->mouse_enabled[i] = true;
	state->mouse_enabled[MBV_MOUSE_ROTATE] = display_3d;
	state->mouse_enabled[MBV_MOUSE_VIEWPOINT] = display_3d;
	state->mouse_enabled[MBV_MOUSE_SITE] = sd->site_mode != MBV_SITE_OFF;
	state->mouse_enabled[MBV_MOUSE_ROUTE] = sd->route_mode != MBV_ROUTE_OFF;
	state->mouse_enabled[MBV_MOUSE_NAV] = sd->nav_mode == MBV_NAV_VIEW;
	state->mouse_enabled[MBV_MOUSE_NAVFILE] = sd->nav_mode != MBV_NAV_OFF;
	state->mouse_label[MBV_MOUSE_MOVE] = "Pan & Zoom";
	state->mouse_label[MBV_MOUSE_ROTATE] = "Rotate Model";
	state->mouse_label[MBV_MOUSE_SHADE] = "Shading";
	state->mouse_label[MBV_MOUSE_VIEWPOINT] = "Rotate View";
	state->mouse_label[MBV_MOUSE_AREA] = "Pick Area";
	state->mouse_label[MBV_MOUSE_SITE] = sd->site_mode != MBV_SITE_EDIT ? "Pick Sites" : "Edit Sites";
	state->mouse_label[MBV_MOUSE_ROUTE] = sd->route_mode == MBV_ROUTE_NAVADJUST ? "Pick Ties" : "Edit Routes";
	state->mouse_label[MBV_MOUSE_NAV] = "Pick Nav";
	state->mouse_label[MBV_MOUSE_NAVFILE] = sd->nav_mode == MBV_NAV_MBNAVADJUST ? "Pick Nav Section" : "Pick Nav File";
}

/*------------------------------------------------------------------------------*/
/* a View menu item, as the Motif View menu callbacks do */
void mbvq_c_view(size_t instance, int item, bool value) {
	switch (item) {
	case MBVQ_VIEW_DISPLAY_2D:
		mbview_setdisplaymode(instance, MBV_DISPLAY_2D);
		break;
	case MBVQ_VIEW_DISPLAY_3D:
		mbview_setdisplaymode(instance, MBV_DISPLAY_3D);
		break;
	case MBVQ_VIEW_GRID_PRIMARY:
		mbview_setgridmode(instance, MBV_GRID_VIEW_PRIMARY);
		break;
	case MBVQ_VIEW_GRID_PRIMARYSLOPE:
		mbview_setgridmode(instance, MBV_GRID_VIEW_PRIMARYSLOPE);
		break;
	case MBVQ_VIEW_GRID_SECONDARY:
		mbview_setgridmode(instance, MBV_GRID_VIEW_SECONDARY);
		break;
	case MBVQ_VIEW_HISTOGRAM:
		mbview_sethistogram(instance, value);
		break;
	case MBVQ_VIEW_SHADE_NONE:
		mbview_setshademode(instance, MBV_SHADE_VIEW_NONE);
		break;
	case MBVQ_VIEW_SHADE_ILLUMINATION:
		mbview_setshademode(instance, MBV_SHADE_VIEW_ILLUMINATION);
		break;
	case MBVQ_VIEW_SHADE_SLOPE:
		mbview_setshademode(instance, MBV_SHADE_VIEW_SLOPE);
		break;
	case MBVQ_VIEW_SHADE_OVERLAY:
		mbview_setshademode(instance, MBV_SHADE_VIEW_OVERLAY);
		break;
	case MBVQ_VIEW_CONTOUR:
		mbview_setviewtoggle(instance, MBV_VIEWTOGGLE_CONTOUR, value);
		break;
	case MBVQ_VIEW_SITE:
		mbview_setviewtoggle(instance, MBV_VIEWTOGGLE_SITE, value);
		break;
	case MBVQ_VIEW_ROUTE:
		mbview_setviewtoggle(instance, MBV_VIEWTOGGLE_ROUTE, value);
		break;
	case MBVQ_VIEW_NAV:
		mbview_setviewtoggle(instance, MBV_VIEWTOGGLE_NAV, value);
		break;
	case MBVQ_VIEW_NAVSWATHBOUNDS:
		mbview_setviewtoggle(instance, MBV_VIEWTOGGLE_NAVSWATHBOUNDS, value);
		break;
	case MBVQ_VIEW_NAVDRAPE:
		mbview_setviewtoggle(instance, MBV_VIEWTOGGLE_NAVDRAPE, value);
		break;
	case MBVQ_VIEW_VECTOR:
		mbview_setviewtoggle(instance, MBV_VIEWTOGGLE_VECTOR, value);
		break;
	case MBVQ_VIEW_POLYLINE:
		mbview_setviewtoggle(instance, MBV_VIEWTOGGLE_POLYLINE, value);
		break;
	case MBVQ_VIEW_POLYGON:
		mbview_setviewtoggle(instance, MBV_VIEWTOGGLE_POLYGON, value);
		break;
	case MBVQ_VIEW_GISPOINT:
		mbview_setviewtoggle(instance, MBV_VIEWTOGGLE_GISPOINT, value);
		break;
	case MBVQ_VIEW_COLORTABLE_HAXBY:
		mbview_setcolortable(instance, MBV_COLORTABLE_HAXBY);
		break;
	case MBVQ_VIEW_COLORTABLE_BRIGHT:
		mbview_setcolortable(instance, MBV_COLORTABLE_BRIGHT);
		break;
	case MBVQ_VIEW_COLORTABLE_MUTED:
		mbview_setcolortable(instance, MBV_COLORTABLE_MUTED);
		break;
	case MBVQ_VIEW_COLORTABLE_GRAY:
		mbview_setcolortable(instance, MBV_COLORTABLE_GRAY);
		break;
	case MBVQ_VIEW_COLORTABLE_FLAT:
		mbview_setcolortable(instance, MBV_COLORTABLE_FLAT);
		break;
	case MBVQ_VIEW_COLORTABLE_SEALEVEL1:
		mbview_setcolortable(instance, MBV_COLORTABLE_SEALEVEL1);
		break;
	case MBVQ_VIEW_COLORTABLE_SEALEVEL2:
		mbview_setcolortable(instance, MBV_COLORTABLE_SEALEVEL2);
		break;
	default:
		break;
	}
	/* the Motif toggle buttons change their own state; here the window shows
	   the state of the core after every choice */
	mbvq_qt_modes_changed(instance);
}

/*------------------------------------------------------------------------------*/
/* Action menu: the program's callbacks, called as Motif calls them, with the
   instance as client data */
static void (*mbvq_action_notify[MBV_MAX_WINDOWS][MBV_NUM_ACTIONS])(Widget, XtPointer, XtPointer);
static char mbvq_action_token[MBV_MAX_WINDOWS][MBV_NUM_ACTIONS];

int mbview_addaction(int verbose, size_t instance, void(mbview_action_notify)(Widget, XtPointer, XtPointer), char *label,
                     int sensitive, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
		fprintf(stderr, "dbg2       label:                     %s\n", label);
		fprintf(stderr, "dbg2       sensitive:                 %d\n", sensitive);
	}

	struct mbview_world_struct *view = &(mbviews[instance]);

	/* the Motif version has no limit check; the arrays hold MBV_NUM_ACTIONS */
	int status = MB_SUCCESS;
	if (view->naction >= MBV_NUM_ACTIONS) {
		fprintf(stderr, "mbview_addaction: no room for action \"%s\"\n", label);
		*error = MB_ERROR_BAD_PARAMETER;
		status = MB_FAILURE;
	}
	else {
		view->pushButton_action[view->naction] = (Widget)&mbvq_action_token[instance][view->naction];
		view->actionsensitive[view->naction] = sensitive;
		mbvq_action_notify[instance][view->naction] = mbview_action_notify;
		mbvq_qt_add_action(instance, view->naction, label);
		view->naction++;

		/* now set action buttons according to current pick states */
		mbview_action_sensitivity(instance);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:        %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:       %d\n", status);
	}

	return (status);
}

int mbview_addactionseparator(int verbose, size_t instance, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       instance:                  %zu\n", instance);
	}

	/* a separator has no sensitivity, so it does not use an action slot */
	mbvq_qt_add_action_separator(instance);

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

void mbvq_c_action(size_t instance, int index) {
	struct mbview_world_struct *view = &(mbviews[instance]);
	if (index < 0 || index >= view->naction || mbvq_action_notify[instance][index] == NULL)
		return;
	(mbvq_action_notify[instance][index])(view->pushButton_action[index], (XtPointer)instance, NULL);
}

/*------------------------------------------------------------------------------*/
int mbview_quit(int verbose, int *error) {
	if (mbv_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
	}

	/* loop over all possible instances and dismiss anything that's up */
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
		fprintf(stderr, "dbg2       status:       %d\n", status);
	}

	return (status);
}

/*------------------------------------------------------------------------------*/
/* User interface functions the core calls (see mbv_ui.h)                       */
/*------------------------------------------------------------------------------*/

int do_mbview_status(char *message, size_t instance) {
	struct mbview_world_struct *view = &(mbviews[instance]);

	view->message_on = true;

	mbvq_qt_status(instance, message);

	return (1);
}

void set_mbview_label_string(Widget w, String str) {
	(void)w;
	(void)str;
}

void set_mbview_label_multiline_string(Widget w, String str) {
	for (size_t i = 0; i < MBV_MAX_WINDOWS; i++) {
		for (int label = 0; label < MBVQ_LABEL_OTHER; label++) {
			if (w == (Widget)&mbvq_label_token[i][label]) {
				mbvq_qt_label(i, label, str);
				return;
			}
		}
	}
}

/* Qt never processes events while the core draws: a nested paint would
   draw in the wrong context */
void do_mbview_xevents(void) {}

int do_mbview_setbackgroundwork(size_t instance) {
	/* set work function if none set */
	if (!work_function_set) {
		mbvq_qt_schedule_work(instance, 0);
		work_function_set = true;
	}
	else {
		fprintf(stderr, "do_mbview_setbackgroundwork: FUNCTION ALREADY SET for instance:%zu!!\n", instance);
	}
	return (MB_SUCCESS);
}

int do_mbview_settimer(void) {
	/* set timer function if none set */
	if (!work_function_set) {
		mbvq_qt_schedule_work(MBV_NO_WINDOW, timer_timeout_time);
		work_function_set = true;
	}
	else {
		fprintf(stderr, "do_mbview_settimer: FUNCTION ALREADY SET!!\n");
	}
	return (MB_SUCCESS);
}

int mbview_update_sensitivity(int verbose, size_t instance, int *error) {
	(void)verbose;
	(void)error;
	struct mbview_struct *data = &(mbviews[instance].data);

	/* a spheroid projection has only the 3D display, as in the Motif version */
	if (data->display_projection_mode == MBV_PROJECTION_SPHEROID)
		data->display_mode = MBV_DISPLAY_3D;

	/* menus and mouse modes */
	mbvq_qt_sensitivity_changed(instance);

	/* now set action buttons according to current pick states */
	mbview_action_sensitivity(instance);

	/* reset sensitivity in parent program */
	if (data->mbview_sensitivity_notify != NULL)
		(data->mbview_sensitivity_notify)();

	return (MB_SUCCESS);
}

int mbview_action_sensitivity(size_t instance) {
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* now set action buttons according to current pick states */
	for (int i = 0; i < view->naction; i++)
		mbvq_qt_action_enabled(instance, i, mbview_actionsensitive(instance, i));

	/* reset sensitivity in parent program */
	if (data->mbview_sensitivity_notify != NULL)
		(data->mbview_sensitivity_notify)();

	return (MB_SUCCESS);
}

int mbview_set_widgets(int verbose, size_t instance, int *error) {
	(void)verbose;
	(void)error;
	struct mbview_struct *data = &(mbviews[instance].data);

	/* check for spheroid projection, as the Motif version does */
	if (data->display_projection_mode == MBV_PROJECTION_SPHEROID) {
		data->display_mode = MBV_DISPLAY_3D;
	}

	/* the mouse mode follows the same rule as in the Motif version */
	set_mbview_mouse_mode(instance, data->mouse_mode);
	return (MB_SUCCESS);
}

void set_mbview_mouse_mode(size_t instance, int mode) {
	if (mbv_verbose >= 2)
		fprintf(stderr, "do_mbview_mouse_mode: instance:%zu mode:%d\n", instance, mode);

	struct mbview_struct *data = &(mbviews[instance].data);

	/* set mode; rotations need the 3D display */
	data->mouse_mode = mode;
	if (data->display_mode == MBV_DISPLAY_2D && (data->mouse_mode == MBV_MOUSE_ROTATE || data->mouse_mode == MBV_MOUSE_VIEWPOINT))
		data->mouse_mode = MBV_MOUSE_MOVE;

	mbvq_qt_modes_changed(instance);
}

/* The toggles and radio items show the state of data fields the core already set */
void set_mbview_display_mode(size_t instance, int mode) {
	(void)mode;
	mbvq_qt_modes_changed(instance);
}
void set_mbview_grid_mode(size_t instance, int mode) {
	(void)mode;
	mbvq_qt_modes_changed(instance);
}
void set_mbview_histogram_mode(size_t instance, bool mode) {
	(void)mode;
	mbvq_qt_modes_changed(instance);
}
void set_mbview_shade_mode(size_t instance, int mode) {
	(void)mode;
	mbvq_qt_modes_changed(instance);
}
void set_mbview_colortable(size_t instance, int mode) {
	(void)mode;
	mbvq_qt_modes_changed(instance);
}
void set_mbview_colortable_mode(size_t instance, int mode) {
	(void)mode;
	mbvq_qt_modes_changed(instance);
}

/* the 2D and 3D view parameter dialogs come in step 3c */
int do_mbview_2dparmstext(size_t instance) {
	mbvq_qt_values_changed(instance, MBV_VALUES_VIEW_2DOFFSET);
	return (MB_SUCCESS);
}
int do_mbview_3dparmstext(size_t instance) {
	mbvq_qt_values_changed(instance, MBV_VALUES_VIEW_3DOFFSET);
	return (MB_SUCCESS);
}

void set_mbview_site_view_mode(size_t instance, int mode) {
	(void)mode;
	mbvq_qt_modes_changed(instance);
}
void set_mbview_route_view_mode(size_t instance, int mode) {
	(void)mode;
	mbvq_qt_modes_changed(instance);
}
void set_mbview_nav_view_mode(size_t instance, int mode) {
	(void)mode;
	mbvq_qt_modes_changed(instance);
}
void set_mbview_navdrape_view_mode(size_t instance, int mode) {
	(void)mode;
	mbvq_qt_modes_changed(instance);
}
void set_mbview_polyline_view_mode(size_t instance, int mode) {
	(void)mode;
	mbvq_qt_modes_changed(instance);
}
void set_mbview_polygon_view_mode(size_t instance, int mode) {
	(void)mode;
	mbvq_qt_modes_changed(instance);
}
void set_mbview_gispoint_view_mode(size_t instance, int mode) {
	(void)mode;
	mbvq_qt_modes_changed(instance);
}

/* list windows */
void mbview_gis_routelistbutton(void) { mbvq_qt_lists_changed(); }
void mbview_gislist_reset(bool mode) {
	(void)mode;
	mbvq_qt_lists_changed();
}
int mbview_updatenavlist(void) {
	mbvq_qt_lists_changed();
	return (MB_SUCCESS);
}
int mbview_updateroutelist(void) {
	mbvq_qt_lists_changed();
	return (MB_SUCCESS);
}
int mbview_updatesitelist(void) {
	mbvq_qt_lists_changed();
	return (MB_SUCCESS);
}

/* hooks of mbv_ui.h */
void mbv_ui_bell(size_t instance) {
	(void)instance;
	mbvq_qt_bell();
}
void mbv_ui_secondary_colortable_changed(size_t instance) { mbvq_qt_modes_changed(instance); }
void mbv_ui_secondary_name_changed(size_t instance, char *name) {
	(void)name;
	mbvq_qt_modes_changed(instance);
}
void mbv_ui_cursor(size_t instance, int cursor) { mbvq_qt_cursor(instance, cursor); }
void mbv_ui_view_values_changed(size_t instance, int values) { mbvq_qt_values_changed(instance, values); }
void mbv_ui_reset_view(size_t instance) { mbview_resetview(instance); }
void mbv_ui_destroy_window(size_t instance) { mbvq_qt_close_window(instance); }
void mbv_ui_close_lists(void) { mbvq_qt_lists_changed(); }

/* 3D soundings view: Phase 5 */
void mbv_ui_soundings_cursor(int cursor) { (void)cursor; }
void mbv_ui_soundings_bell(void) { mbvq_qt_bell(); }
int mb3dsoundings_updatecursor(void) { return (MB_SUCCESS); }
int mb3dsoundings_updatemodetoggles(void) { return (MB_SUCCESS); }
int mb3dsoundings_updatestatus(void) { return (MB_SUCCESS); }
