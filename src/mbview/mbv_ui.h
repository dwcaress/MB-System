/*--------------------------------------------------------------------
 *    The MB-system:  mbv_ui.h  11/7/2026
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
 * User interface functions that the mbview core calls.
 *
 * The core files (mbview_plot.c, mbview_process.c, mbview_primary.c,
 * mbview_secondary.c, mbview_pick.c, mbview_nav.c, mbview_route.c,
 * mbview_site.c, mbview_vector.c, mbview_gispoint.c, mbview_polygon.c,
 * mbview_polyline.c, mbview_profile.c, mbv_core.c, mbv_soundings_core.c,
 * mbv_globals.c, mbv_render.c) make no X11 or Motif calls. When the user
 * interface has to react, they call the functions below. The Motif
 * library implements them in mbv_ui_motif.c and
 * mb3dsoundings_callbacks.c; another toolkit implements the same
 * functions.
 *
 * The core also calls these user interface functions, declared in
 * mbviewprivate.h or mb3dsoundingsprivate.h. Another toolkit has to
 * provide them too:
 *   mbview_callbacks.c: do_mbview_status, do_mbview_xevents,
 *     do_mbview_settimer, do_mbview_setbackgroundwork,
 *     mbview_action_sensitivity, mbview_update_sensitivity,
 *     mbview_set_widgets, set_mbview_label_multiline_string,
 *     set_mbview_mouse_mode, set_mbview_display_mode,
 *     set_mbview_grid_mode, set_mbview_histogram_mode,
 *     set_mbview_shade_mode, set_mbview_colortable,
 *     set_mbview_colortable_mode, set_mbview_site_view_mode,
 *     set_mbview_route_view_mode, set_mbview_nav_view_mode,
 *     set_mbview_navdrape_view_mode, set_mbview_polyline_view_mode,
 *     set_mbview_polygon_view_mode, set_mbview_gispoint_view_mode,
 *     do_mbview_2dparmstext, do_mbview_3dparmstext
 *   mbview_gislist.c: mbview_gis_routelistbutton, mbview_gislist_reset
 *   mbv_ui_motif.c: mbview_updatenavlist, mbview_updateroutelist,
 *     mbview_updatesitelist
 *   mb3dsoundings_callbacks.c: mb3dsoundings_updatecursor,
 *     mb3dsoundings_updatemodetoggles, mb3dsoundings_updatestatus
 */

#ifndef MBVIEW_MBV_UI_H
#define MBVIEW_MBV_UI_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* signal a refused or invalid action to the user */
void mbv_ui_bell(size_t instance);

/* the secondary colour table or overlay shading of an instance changed */
void mbv_ui_secondary_colortable_changed(size_t instance);

/* the secondary grid of an instance got a new name */
void mbv_ui_secondary_name_changed(size_t instance, char *name);

/* mouse cursors */
#define MBV_CURSOR_TARGET_BLACK 0
#define MBV_CURSOR_TARGET_GREEN 1
#define MBV_CURSOR_TARGET_RED 2
#define MBV_CURSOR_TARGET_BLUE 3
#define MBV_CURSOR_FLEUR_BLACK 4
#define MBV_CURSOR_FLEUR_RED 5
#define MBV_CURSOR_SIZING_BLACK 6
#define MBV_CURSOR_SIZING_RED 7
#define MBV_CURSOR_EXCHANGE_BLACK 8
#define MBV_CURSOR_EXCHANGE_GREEN 9
#define MBV_CURSOR_EXCHANGE_RED 10

/* set the mouse cursor of the map view of an instance */
void mbv_ui_cursor(size_t instance, int cursor);

/* set the mouse cursor of the 3D soundings view */
void mbv_ui_soundings_cursor(int cursor);

/* signal a refused or invalid action in the 3D soundings view */
void mbv_ui_soundings_bell(void);

/* view values that mouse dragging changes, shown in the view control dialogs */
#define MBV_VALUES_VIEW_2DOFFSET 0      /* view->offset2d_x, offset2d_y */
#define MBV_VALUES_VIEW_3DOFFSET 1      /* view->offset3d_x, offset3d_y */
#define MBV_VALUES_MODEL_ORIENTATION 2  /* data->modelazimuth3d, modelelevation3d */
#define MBV_VALUES_ILLUMINATION 3       /* data->illuminate_azimuth, illuminate_elevation */
#define MBV_VALUES_VIEW_ORIENTATION 4   /* data->viewazimuth3d, viewelevation3d */
#define MBV_VALUES_VIEW_2DZOOM 5        /* view->size2d */
#define MBV_VALUES_MODEL_3DZOOM 6       /* view->offset3d_z */
#define MBV_VALUES_EXAGGERATION 7       /* data->exaggeration */
#define MBV_VALUES_ILLUMINATION_AMP 8   /* data->illuminate_magnitude */
#define MBV_VALUES_SLOPE_AMP 9          /* data->slope_magnitude */
#define MBV_VALUES_OVERLAY_AMP 10       /* data->overlay_shade_magnitude */
#define MBV_VALUES_VIEW_3DZOOM 11       /* view->viewoffset3d_z */

/* view values of an instance changed while dragging */
void mbv_ui_view_values_changed(size_t instance, int values);

/* the user asked to reset the view of an instance (key 'r') */
void mbv_ui_reset_view(size_t instance);

/* destroy the window of an instance (mbview_destroy) */
void mbv_ui_destroy_window(size_t instance);

/* clear and close the site, route, navigation and GIS list windows when the
   last instance is destroyed (mbview_destroy) */
void mbv_ui_close_lists(void);

#ifdef __cplusplus
}
#endif

#endif /* MBVIEW_MBV_UI_H */
