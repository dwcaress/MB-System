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
	view->mb3dview.mbview_label_projection = (Widget)&mbvq_label_token[instance][MBVQ_LABEL_PROJECTION];
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
	c[MBVQ_VIEW_PROFILE] = data->profile_view_mode == MBV_VIEW_ON;

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
	e[MBVQ_VIEW_PROFILE] = primary;

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
/* Controls dialogs: the values they show, as the Motif popup functions fill them */
void mbvq_c_controls(size_t instance, struct mbvq_controls *controls) {
	const struct mbview_world_struct *view = &(mbviews[instance]);
	const struct mbview_struct *data = &(view->data);
	memset(controls, 0, sizeof(*controls));
	double *f = controls->field;
	f[MBVQ_F_DATAMIN] = data->primary_colortable_min;
	f[MBVQ_F_DATAMAX] = data->primary_colortable_max;
	f[MBVQ_F_CONTOURS] = data->contour_interval;
	f[MBVQ_F_SLOPEMIN] = data->slope_colortable_min;
	f[MBVQ_F_SLOPEMAX] = data->slope_colortable_max;
	f[MBVQ_F_OVERLAYMIN] = data->secondary_colortable_min;
	f[MBVQ_F_OVERLAYMAX] = data->secondary_colortable_max;
	f[MBVQ_F_ILLUM_AMP] = data->illuminate_magnitude;
	f[MBVQ_F_ILLUM_AZI] = data->illuminate_azimuth;
	f[MBVQ_F_ILLUM_ELEV] = data->illuminate_elevation;
	f[MBVQ_F_SLOPE_AMP] = data->slope_magnitude;
	f[MBVQ_F_OVERLAY_AMP] = data->overlay_shade_magnitude;
	f[MBVQ_F_OVERLAY_CENTER] = data->overlay_shade_center;
	f[MBVQ_F_MODEL_AZI] = data->modelazimuth3d;
	f[MBVQ_F_MODEL_ELEV] = data->modelelevation3d;
	f[MBVQ_F_VIEW_AZI] = data->viewazimuth3d;
	f[MBVQ_F_VIEW_ELEV] = data->viewelevation3d;
	f[MBVQ_F_EXAGGERATION] = data->exaggeration;
	f[MBVQ_F_OFFSET3DX] = view->offset3d_x;
	f[MBVQ_F_OFFSET3DY] = view->offset3d_y;
	f[MBVQ_F_MODEL_3DZOOM] = view->offset3d_z;
	f[MBVQ_F_VIEW_3DZOOM] = view->viewoffset3d_z;
	f[MBVQ_F_OFFSET2DX] = view->offset2d_x;
	f[MBVQ_F_OFFSET2DY] = view->offset2d_y;
	f[MBVQ_F_ZOOM2D] = view->size2d;
	controls->toggle[MBVQ_T_DATA_HTOC] = data->primary_colortable_mode == MBV_COLORTABLE_REVERSED;
	controls->toggle[MBVQ_T_SLOPE_HTOC] = data->slope_colortable_mode == MBV_COLORTABLE_REVERSED;
	controls->toggle[MBVQ_T_OVERLAY_HTOC] = data->secondary_colortable_mode == MBV_COLORTABLE_REVERSED;
	controls->toggle[MBVQ_T_OVERLAY_SHADE_HTOC] = data->overlay_shade_mode == MBV_COLORTABLE_REVERSED;
	controls->lorez_dimension = data->lorez_dimension;
	controls->hirez_dimension = data->hirez_dimension;
	controls->lorez_navdecimate = data->lorez_navdecimate;
	controls->hirez_navdecimate = data->hirez_navdecimate;
	/* PROJECTED and ALREADYPROJECTED show as UTM, as in the Motif popup */
	controls->display_projection = -1;
	if (data->display_projection_mode == MBV_PROJECTION_GEOGRAPHIC)
		controls->display_projection = 0;
	else if (data->display_projection_mode == MBV_PROJECTION_PROJECTED ||
	         data->display_projection_mode == MBV_PROJECTION_ALREADYPROJECTED)
		controls->display_projection = 1;
	else if (data->display_projection_mode == MBV_PROJECTION_SPHEROID)
		controls->display_projection = 2;
	controls->lonlatstyle = shared.lonlatstyle == MBV_LONLAT_DEGREESMINUTES ? 1 : 0;
	controls->primary = data->primary_nxy > 0 && data->primary_data != NULL;
	controls->secondary = data->secondary_nxy > 0 && data->secondary_data != NULL;
}

/* Apply buttons, as the Motif callbacks: the texts go to the core unchanged */
void mbvq_c_apply(size_t instance, int dialog, const char *const *text, const int *toggle) {
	if (dialog == MBVQ_DIALOG_COLORBOUNDS) {
		struct mbview_colorbounds_values values;
		snprintf(values.datamin, sizeof(values.datamin), "%s", text[MBVQ_F_DATAMIN]);
		snprintf(values.datamax, sizeof(values.datamax), "%s", text[MBVQ_F_DATAMAX]);
		snprintf(values.contours, sizeof(values.contours), "%s", text[MBVQ_F_CONTOURS]);
		values.data_htoc = toggle[MBVQ_T_DATA_HTOC];
		snprintf(values.slopemin, sizeof(values.slopemin), "%s", text[MBVQ_F_SLOPEMIN]);
		snprintf(values.slopemax, sizeof(values.slopemax), "%s", text[MBVQ_F_SLOPEMAX]);
		values.slope_htoc = toggle[MBVQ_T_SLOPE_HTOC];
		snprintf(values.overlaymin, sizeof(values.overlaymin), "%s", text[MBVQ_F_OVERLAYMIN]);
		snprintf(values.overlaymax, sizeof(values.overlaymax), "%s", text[MBVQ_F_OVERLAYMAX]);
		values.overlay_htoc = toggle[MBVQ_T_OVERLAY_HTOC];
		mbview_colorboundsapply(instance, &values);
	}
	else if (dialog == MBVQ_DIALOG_SHADE) {
		struct mbview_shadeparms_values values;
		snprintf(values.illum_amp, sizeof(values.illum_amp), "%s", text[MBVQ_F_ILLUM_AMP]);
		snprintf(values.illum_azi, sizeof(values.illum_azi), "%s", text[MBVQ_F_ILLUM_AZI]);
		snprintf(values.illum_elev, sizeof(values.illum_elev), "%s", text[MBVQ_F_ILLUM_ELEV]);
		snprintf(values.slope_amp, sizeof(values.slope_amp), "%s", text[MBVQ_F_SLOPE_AMP]);
		snprintf(values.overlay_amp, sizeof(values.overlay_amp), "%s", text[MBVQ_F_OVERLAY_AMP]);
		snprintf(values.overlay_center, sizeof(values.overlay_center), "%s", text[MBVQ_F_OVERLAY_CENTER]);
		values.overlay_shade_htoc = toggle[MBVQ_T_OVERLAY_SHADE_HTOC];
		mbview_shadeparmsapply(instance, &values);
	}
	else if (dialog == MBVQ_DIALOG_VIEW3D) {
		struct mbview_3dparms_values values;
		snprintf(values.model_azimuth, sizeof(values.model_azimuth), "%s", text[MBVQ_F_MODEL_AZI]);
		snprintf(values.model_elevation, sizeof(values.model_elevation), "%s", text[MBVQ_F_MODEL_ELEV]);
		snprintf(values.view_azimuth, sizeof(values.view_azimuth), "%s", text[MBVQ_F_VIEW_AZI]);
		snprintf(values.view_elevation, sizeof(values.view_elevation), "%s", text[MBVQ_F_VIEW_ELEV]);
		snprintf(values.exaggeration, sizeof(values.exaggeration), "%s", text[MBVQ_F_EXAGGERATION]);
		snprintf(values.view_3doffsetx, sizeof(values.view_3doffsetx), "%s", text[MBVQ_F_OFFSET3DX]);
		snprintf(values.view_3doffsety, sizeof(values.view_3doffsety), "%s", text[MBVQ_F_OFFSET3DY]);
		snprintf(values.model_3dzoom, sizeof(values.model_3dzoom), "%s", text[MBVQ_F_MODEL_3DZOOM]);
		snprintf(values.view_3dzoom, sizeof(values.view_3dzoom), "%s", text[MBVQ_F_VIEW_3DZOOM]);
		mbview_3dparmsapply(instance, &values);
	}
	else if (dialog == MBVQ_DIALOG_VIEW2D) {
		struct mbview_2dparms_values values;
		snprintf(values.view_2doffsetx, sizeof(values.view_2doffsetx), "%s", text[MBVQ_F_OFFSET2DX]);
		snprintf(values.view_2doffsety, sizeof(values.view_2doffsety), "%s", text[MBVQ_F_OFFSET2DY]);
		snprintf(values.view_2dzoom, sizeof(values.view_2dzoom), "%s", text[MBVQ_F_ZOOM2D]);
		mbview_2dparmsapply(instance, &values);
	}
}

void mbvq_c_setresolution(size_t instance, int lorez_dimension, int hirez_dimension, int lorez_navdecimate,
                          int hirez_navdecimate) {
	mbview_setresolution(instance, lorez_dimension, hirez_dimension, lorez_navdecimate, hirez_navdecimate);
}

void mbvq_c_projection_label(size_t instance) { do_mbview_set_projection_label(instance); }

void mbvq_c_setprojection(size_t instance, int projection) {
	if (projection == 0)
		mbview_setdisplayprojection(instance, MBV_PROJECTION_GEOGRAPHIC);
	else if (projection == 1)
		mbview_setdisplayprojection(instance, MBV_PROJECTION_PROJECTED);
	else if (projection == 2)
		mbview_setdisplayprojection(instance, MBV_PROJECTION_SPHEROID);
	/* the window shows the state of the core again (menus) */
	mbvq_qt_modes_changed(instance);
}

void mbvq_c_setlonlatstyle(size_t instance, int style) {
	mbview_setlonlatstyle(instance, style == 1 ? MBV_LONLAT_DEGREESMINUTES : MBV_LONLAT_DEGREESDECIMAL);
}

/*------------------------------------------------------------------------------*/
/* Profile window, as do_mbview_view_profile, do_mbview_profile_dismiss and the
   profile sliders do without the Motif widgets */
void mbvq_c_profile_show(size_t instance, bool on) {
	struct mbview_struct *data = &(mbviews[instance].data);
	if (on && data->profile_view_mode == MBV_VIEW_OFF) {
		data->profile_view_mode = MBV_VIEW_ON;
		mbview_plotprofile(instance);
	}
	else if (!on && data->profile_view_mode == MBV_VIEW_ON) {
		mbview_destroy_prglx(instance);
		data->profile_view_mode = MBV_VIEW_OFF;
	}
	mbvq_qt_modes_changed(instance);
}

bool mbvq_c_profile_shown(size_t instance) { return (mbviews[instance].data.profile_view_mode == MBV_VIEW_ON); }

void mbvq_c_profile_resize(size_t instance, int width, int height) {
	struct mbview_world_struct *view = &(mbviews[instance]);
	struct mbview_struct *data = &(view->data);

	/* what mbview_reset_prglx() sets */
	data->prwidth = width > 0 ? width : 1;
	data->prheight = height > 0 ? height : 1;
	view->praspect_ratio = ((float)data->prheight) / ((float)data->prwidth);
}

void mbvq_c_plotprofile(size_t instance) { mbview_plotprofile(instance); }

void mbvq_c_profile_control(size_t instance, int control, int value) {
	int c = MBV_PROFILE_CONTROL_WIDTH;
	if (control == 1)
		c = MBV_PROFILE_CONTROL_SLOPE;
	else if (control == 2)
		c = MBV_PROFILE_CONTROL_EXAGGERATION;
	mbview_setprofilecontrol(instance, c, value);
}

void mbvq_c_profile_controls(size_t instance, int *width, int *slope, int *exaggeration) {
	const struct mbview_struct *data = &(mbviews[instance].data);
	*width = data->profile_widthfactor;
	*slope = (int)lround(100.0 * data->profile_slopethreshold);
	*exaggeration = (int)lround(10.0 * data->profile_exaggeration);
}

/*------------------------------------------------------------------------------*/
/* List windows: the rows and actions of the core; the windows are shared by
   all instances, as in Motif */
static int mbvq_gistype(int list) {
	if (list == MBVQ_LIST_POLYLINE)
		return (MBV_GISLAYER_POLYLINE);
	if (list == MBVQ_LIST_POLYGON)
		return (MBV_GISLAYER_POLYGON);
	if (list == MBVQ_LIST_GISPOINT)
		return (MBV_GISLAYER_GISPOINT);
	return (-1);
}

void *mbvq_c_rows(int list) {
	struct mbview_listrows *rows = (struct mbview_listrows *)calloc(1, sizeof(struct mbview_listrows));
	if (rows == NULL)
		return (NULL);
	int status = MB_SUCCESS;
	if (list == MBVQ_LIST_SITE)
		status = mbview_sitelist_rows(rows);
	else if (list == MBVQ_LIST_ROUTE)
		status = mbview_routelist_rows(rows);
	else if (list == MBVQ_LIST_NAV)
		status = mbview_navlist_rows(rows);
	else if (mbvq_gistype(list) >= 0)
		status = mbview_gislist_rows(mbvq_gistype(list), rows);
	if (status != MB_SUCCESS)
		mbview_listrows_free(rows);
	return (rows);
}

int mbvq_c_rows_count(const void *rows) { return (rows != NULL ? ((const struct mbview_listrows *)rows)->nrows : 0); }

const char *mbvq_c_rows_text(const void *rows, int i) {
	const struct mbview_listrows *r = (const struct mbview_listrows *)rows;
	return (r != NULL && i >= 0 && i < r->nrows ? r->row[i] : "");
}

int mbvq_c_rows_selected(const void *rows) { return (rows != NULL ? ((const struct mbview_listrows *)rows)->selected : 0); }

void mbvq_c_rows_free(void *rows) {
	if (rows != NULL) {
		mbview_listrows_free((struct mbview_listrows *)rows);
		free(rows);
	}
}

int mbvq_c_list_action(int list, int action, int position_count, const int *position_list) {
	/* the core functions take the positions as the Motif callbacks read them */
	int *positions = (int *)position_list;
	const int gistype = mbvq_gistype(list);
	if (action == MBVQ_LIST_SELECT) {
		if (list == MBVQ_LIST_SITE)
			mbview_sitelist_select(position_count, positions);
		else if (list == MBVQ_LIST_ROUTE)
			mbview_routelist_select(position_count, positions);
		else if (list == MBVQ_LIST_NAV)
			mbview_navlist_select(position_count, positions);
		else if (gistype >= 0)
			return (mbview_gislist_select(gistype, position_count, position_list));
	}
	else if (action == MBVQ_LIST_DELETE) {
		if (list == MBVQ_LIST_SITE)
			mbview_sitelist_delete(position_count, positions);
		else if (list == MBVQ_LIST_ROUTE)
			mbview_routelist_delete(position_count, positions);
		else if (list == MBVQ_LIST_NAV)
			mbview_navlist_delete(position_count, positions);
		else if (gistype >= 0)
			mbview_gislist_delete(gistype, position_count, position_list);
	}
	else if (action == MBVQ_LIST_SHOWHIDE && gistype >= 0 && position_count > 0) {
		mbview_gislist_showhide(gistype, position_count, position_list);
	}
	else if (action == MBVQ_LIST_TOROUTES && gistype >= 0) {
		mbview_gislist_toroutes(gistype, position_count, position_list);
	}
	else if (action == MBVQ_LIST_TOPOLYLINES && list == MBVQ_LIST_ROUTE && position_count > 0) {
		mbview_routelist_topolylines(position_count, position_list);
	}
	return (0);
}

void mbvq_c_list_redraw(void) { mbview_gislist_redraw(); }

void mbvq_c_list_shown(int list, bool shown) {
	const int init = shown ? MBV_WINDOW_VISIBLE : MBV_WINDOW_NULL;
	if (list == MBVQ_LIST_SITE)
		shared.init_sitelist = init;
	else if (list == MBVQ_LIST_ROUTE)
		shared.init_routelist = init;
	else if (list == MBVQ_LIST_NAV)
		shared.init_navlist = init;
	else if (mbvq_gistype(list) >= 0)
		shared.init_gislist[mbvq_gistype(list)] = init;
}

const char *mbvq_c_list_title(int list) {
	if (list == MBVQ_LIST_SITE)
		return ("Site List");
	if (list == MBVQ_LIST_ROUTE)
		return ("Route List");
	if (list == MBVQ_LIST_NAV)
		return ("Navigation List");
	return (mbview_gislist_title(mbvq_gistype(list)));
}

/* the column headers of mbview_startup() and mbview_gislist_create() */
const char *mbvq_c_list_header(int list) {
	if (list == MBVQ_LIST_SITE)
		return ("Site | Lon | Lat | Depth | Color | Size | Name");
	if (list == MBVQ_LIST_ROUTE)
		return ("Route | Waypoint | Lon | Lat | Depth | Distance | DistanceOverTopo | Waypoint Type");
	if (list == MBVQ_LIST_NAV)
		return ("Route | Navpoints | Color | Size | Name");
	return (mbview_gislist_header(mbvq_gistype(list)));
}

bool mbvq_c_routes_to_polylines(void) { return (shared.shareddata.polyline_mode != MBV_POLYLINE_OFF); }

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

int do_mbview_message_on(char *message, size_t instance) {
	struct mbview_world_struct *view = &(mbviews[instance]);

	view->message_on = true;

	mbvq_qt_message(instance, message, true);

	return (1);
}

int do_mbview_message_off(size_t instance) {
	mbvq_qt_message(instance, "", false);

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

/* the 2D and 3D view parameter dialogs show the values again (after Reset) */
int do_mbview_2dparmstext(size_t instance) {
	mbvq_qt_values_changed(instance, MBVQ_VALUES_ALL_2D);
	return (MB_SUCCESS);
}
int do_mbview_3dparmstext(size_t instance) {
	mbvq_qt_values_changed(instance, MBVQ_VALUES_ALL_3D);
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
void mbview_gis_routelistbutton(void) { mbvq_qt_list_changed(MBVQ_LIST_ROUTE); }
int mbview_updategislist(int type) {
	if (type == MBV_GISLAYER_POLYLINE)
		mbvq_qt_list_changed(MBVQ_LIST_POLYLINE);
	else if (type == MBV_GISLAYER_POLYGON)
		mbvq_qt_list_changed(MBVQ_LIST_POLYGON);
	else if (type == MBV_GISLAYER_GISPOINT)
		mbvq_qt_list_changed(MBVQ_LIST_GISPOINT);
	return (MB_SUCCESS);
}
int mbview_updatenavlist(void) {
	mbvq_qt_list_changed(MBVQ_LIST_NAV);
	return (MB_SUCCESS);
}
int mbview_updateroutelist(void) {
	mbvq_qt_list_changed(MBVQ_LIST_ROUTE);
	return (MB_SUCCESS);
}
int mbview_updatesitelist(void) {
	mbvq_qt_list_changed(MBVQ_LIST_SITE);
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
void mbv_ui_close_lists(void) { mbvq_qt_lists_closed(); }

/*------------------------------------------------------------------------------*/
/* The 3D soundings window: the Qt versions of the window functions of
   mb3dsoundings_callbacks.c (startup, open, end and the user interface
   functions). The logic is the shared code of mbv_soundings_core.c. */

typedef char mbvq_check_sdg_edit[(MBVQ_SDG_EDIT_TOGGLE == MBS_EDIT_TOGGLE && MBVQ_SDG_EDIT_PICK == MBS_EDIT_PICK &&
                                  MBVQ_SDG_EDIT_ERASE == MBS_EDIT_ERASE && MBVQ_SDG_EDIT_RESTORE == MBS_EDIT_RESTORE &&
                                  MBVQ_SDG_EDIT_GRAB == MBS_EDIT_GRAB && MBVQ_SDG_EDIT_INFO == MBS_EDIT_INFO)
                                     ? 1
                                     : -1];
typedef char mbvq_check_sdg_mouse[(MBVQ_SDG_MOUSE_ROTATE == MBS_MOUSE_ROTATE && MBVQ_SDG_MOUSE_PANZOOM == MBS_MOUSE_PANZOOM)
                                      ? 1
                                      : -1];

/* the status line text (mb3dsoundings_updatestatus) */
static mb_path mbvq_soundings_text;

int mb3dsoundings_startup(int verbose, Widget parent, XtAppContext app, int *error) {
	/* set local verbosity */
	mbs_verbose = verbose;

	if (mbs_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
	}

	/* set parent widget and app context (unused by Qt) */
	mbs_parent_widget = parent;
	mbs_app_context = app;
	mbs_work_function_set = false;
	mbs_timer_count = 0;

	/* initialize window */
	mb3dsoundings_reset();

	*error = mbs_error;

	if (mbs_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
	}

	return (mbs_status);
}

int mb3dsoundings_open(int verbose, struct mb3dsoundings_struct *soundingdata, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
	}

	/* show the statistics and set the data */
	mb3dsoundings_setdata(verbose, soundingdata, error);

	/* create the window if needed, then show it */
	if (mb3dsoundings.init == MBS_WINDOW_NULL) {
		mbvq_qt_soundings_create();
		mb3dsoundings.init = MBS_WINDOW_HIDDEN;
	}
	if (mb3dsoundings.init == MBS_WINDOW_HIDDEN) {
		mbvq_qt_soundings_show();
		mb3dsoundings.init = MBS_WINDOW_VISIBLE;
	}

	/* update gui widgets */
	mb3dsoundings_updategui();

	/* the drawing view sets gl_width and gl_height when it is sized */
	mb3dsoundings.glx_init = true;

	/* recalculate vertical scaling */
	mb3dsoundings_setzscale(verbose, error);

	/* replot the data */
	mb3dsoundings_plot(verbose, error);

	*error = mbs_error;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
	}

	return (mbs_status);
}

int mb3dsoundings_end(int verbose, int *error) {
	/* set local verbosity */
	mbs_verbose = verbose;

	if (mbs_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
	}

	/* handle destruction if not already handled */
	if (mb3dsoundings.init != MBS_WINDOW_NULL) {
		mbvq_qt_soundings_destroy();
		mb3dsoundings.glx_init = false;
		mb3dsoundings.init = MBS_WINDOW_NULL;
	}

	/* reinitialize parameters, but keep the patch test bias values */
	mb3dsoundings_reset_keepbias();

	/* set flags */
	mbs_status = MB_SUCCESS;
	mbs_error = MB_ERROR_NO_ERROR;

	*error = mbs_error;

	if (mbs_verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
	}

	return (mbs_status);
}

/* the cursor of each edit mode, as the Motif mb3dsoundings_updatecursor() */
int mb3dsoundings_updatecursor(void) {
	int cursor = MBV_CURSOR_TARGET_RED;
	if (mb3dsoundings.edit_mode == MBS_EDIT_ERASE)
		cursor = MBV_CURSOR_EXCHANGE_RED;
	else if (mb3dsoundings.edit_mode == MBS_EDIT_RESTORE)
		cursor = MBV_CURSOR_EXCHANGE_GREEN;
	else if (mb3dsoundings.edit_mode == MBS_EDIT_INFO)
		cursor = MBV_CURSOR_TARGET_BLUE;
	mbvq_qt_soundings_cursor(cursor);
	return (mbs_status);
}

/* the edit mode radio buttons and the mouse mode label, as the Motif
   mb3dsoundings_updatemodetoggles() (whose label always names Pan and Zoom) */
int mb3dsoundings_updatemodetoggles(void) {
	mbvq_qt_soundings_refresh();
	const char *mode = "Toggle";
	if (mb3dsoundings.edit_mode == MBS_EDIT_PICK)
		mode = "Pick";
	else if (mb3dsoundings.edit_mode == MBS_EDIT_ERASE)
		mode = "Erase";
	else if (mb3dsoundings.edit_mode == MBS_EDIT_RESTORE)
		mode = "Restore";
	else if (mb3dsoundings.edit_mode == MBS_EDIT_GRAB)
		mode = "Grab";
	else if (mb3dsoundings.edit_mode == MBS_EDIT_INFO)
		mode = "Info";
	snprintf(mbvq_soundings_text, sizeof(mbvq_soundings_text),
	         ":::t\"Mouse Mode:\":t\"L: Edit (%s)\":t\"M: Pan\"\"R: Zoom\"", mode);
	mbvq_qt_soundings_mousemode(mbvq_soundings_text);
	return (mbs_status);
}

/* the status line, as the Motif mb3dsoundings_updatestatus() */
int mb3dsoundings_updatestatus(void) {
	struct mb3dsoundings_struct *soundingdata = (struct mb3dsoundings_struct *)mb3dsoundings.soundingdata;
	if (soundingdata == NULL)
		return (mbs_status);

	/* if in info mode and sounding picked print info as status */
	if (mb3dsoundings.edit_mode == MBS_EDIT_INFO && mb3dsoundings.last_sounding_defined) {
		struct mb3dsoundings_sounding_struct *sounding =
		    (struct mb3dsoundings_sounding_struct *)&(soundingdata->soundings[mb3dsoundings.last_sounding_edited]);
		(mb3dsoundings.mb3dsoundings_info_notify)(sounding->ifile, sounding->iping, sounding->ibeam, mbvq_soundings_text);
		fprintf(stderr, "\n%s\n", mbvq_soundings_text);
		mbvq_qt_soundings_status(mbvq_soundings_text, false);
	}

	/* else set standard status label */
	else {
		snprintf(mbvq_soundings_text, sizeof(mbvq_soundings_text),
		         "Azi:%.2f | Elev: %.2f | exagger:%.2f | Tot:%d Good:%d Flagged:%d", mb3dsoundings.azimuth,
		         mb3dsoundings.elevation, mb3dsoundings.exaggeration, soundingdata->num_soundings,
		         soundingdata->num_soundings_unflagged, soundingdata->num_soundings_flagged);
		mbvq_qt_soundings_status(mbvq_soundings_text, true);
	}

	return (mbs_status);
}

/* the controls show the state of the core (the bias sliders come with the
   soundings controls) */
int mb3dsoundings_updategui(void) {
	mbvq_qt_soundings_refresh();
	return (mbs_status);
}

void mbv_ui_soundings_mousemode(char *text) { mbvq_qt_soundings_mousemode(text); }
void mbv_ui_soundings_cursor(int cursor) { mbvq_qt_soundings_cursor(cursor); }
void mbv_ui_soundings_bell(void) { mbvq_qt_bell(); }

void mbvq_c_soundings_resize(int width, int height) {
	mb3dsoundings.gl_width = width;
	mb3dsoundings.gl_height = height;
	if (height > 0)
		mb3dsoundings.aspect_ratio = ((float)width) / ((float)height);
}

void mbvq_c_soundings_plot(void) {
	if (mb3dsoundings.soundingdata != NULL)
		mb3dsoundings_plot(mbs_verbose, &mbs_error);
}

void mbvq_c_soundings_edit_mode(int mode) {
	if (mode == MBS_EDIT_TOGGLE)
		do_mb3dsdg_mouse_toggle(NULL, NULL, NULL);
	else if (mode == MBS_EDIT_PICK)
		do_mb3dsdg_mouse_pick(NULL, NULL, NULL);
	else if (mode == MBS_EDIT_ERASE)
		do_mb3dsdg_mouse_erase(NULL, NULL, NULL);
	else if (mode == MBS_EDIT_RESTORE)
		do_mb3dsdg_mouse_restore(NULL, NULL, NULL);
	else if (mode == MBS_EDIT_GRAB)
		do_mb3dsdg_mouse_grab(NULL, NULL, NULL);
	else if (mode == MBS_EDIT_INFO)
		do_mb3dsdg_mouse_info(NULL, NULL, NULL);
}

void mbvq_c_soundings_mouse_mode(int mode) {
	mb3dsoundings_setmousemode(mode);
	mbvq_qt_soundings_refresh();
}

void mbvq_c_soundings_resetview(void) { do_mb3dsdg_resetview(NULL, NULL, NULL); }

void mbvq_c_soundings_dismiss(void) {
	mbvq_qt_soundings_hide();
	mb3dsoundings_dismissed();
}

void mbvq_c_soundings_modes(int *edit_mode, int *mouse_mode) {
	*edit_mode = mb3dsoundings.edit_mode;
	*mouse_mode = mb3dsoundings.mouse_mode;
}
