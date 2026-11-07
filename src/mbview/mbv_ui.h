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
 * mbview_polyline.c, mbview_profile.c) make no X11 or Motif calls. When
 * the user interface has to react, they call the functions below. The
 * Motif library implements them in mbv_ui_motif.c; another toolkit
 * implements the same functions.
 *
 * The core also calls these user interface functions, declared in
 * mbviewprivate.h. Another toolkit has to provide them too:
 *   mbview_callbacks.c: do_mbview_status, do_mbview_xevents,
 *     mbview_update_sensitivity, set_mbview_label_multiline_string,
 *     set_mbview_route_view_mode, set_mbview_polyline_view_mode,
 *     set_mbview_polygon_view_mode, set_mbview_gispoint_view_mode
 *   mbview_gislist.c: mbview_gis_routelistbutton
 *   mbv_ui_motif.c: mbview_updatenavlist, mbview_updateroutelist,
 *     mbview_updatesitelist
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

#ifdef __cplusplus
}
#endif

#endif /* MBVIEW_MBV_UI_H */
