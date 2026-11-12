/*--------------------------------------------------------------------
 *    The MB-system:  mbnaq_noview.c  9/11/2026
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
 * The mbview functions that mbnavadjust_prog.c calls, for a build without
 * the map view. The map view is added when mbview-qt is available; until
 * then the visualization action is not offered, so these do nothing.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include <X11/Intrinsic.h>

#include "mb_define.h"
#include "mb_status.h"
#include "mbview.h"

int mbview_addaction(int verbose, size_t instance, void(mbview_action_notify)(Widget, XtPointer,
    XtPointer), char *label, int sensitive, int *error) {
  (void)verbose;
  return (MB_SUCCESS);
}

int mbview_addnav(int verbose, size_t instance, int npoint, double *time_d, double *navlon,
    double *navlat, double *navz, double *heading, double *speed, double *navportlon,
    double *navportlat, double *navstbdlon, double *navstbdlat, unsigned int *line,
    unsigned int *shot, unsigned int *cdp, int navcolor, int navsize, mb_path navname,
    int navpathstatus, mb_path navpathraw, mb_path navpathprocessed, int navformat,
    bool navswathbounds, bool navline, bool navshot, bool navcdp, int decimation, int *error) {
  (void)verbose;
  return (MB_SUCCESS);
}

int mbview_addpicknotify(int verbose, size_t instance, int picktype,
    void(mbview_pick_notify)(size_t), int *error) {
  (void)verbose;
  return (MB_SUCCESS);
}

int mbview_addroute(int verbose, size_t instance, int npoint, double *routelon,
    double *routelat, int *waypoint, int routecolor, int routesize, int routeeditmode,
    mb_path routename, int *iroute, int *error) {
  (void)verbose;
  return (MB_SUCCESS);
}

int mbview_deleteallroutes(int verbose, size_t instance, int *error) {
  (void)verbose;
  return (MB_SUCCESS);
}

int mbview_destroy(int verbose, size_t instance, bool destroywidgets, int *error) {
  (void)verbose;
  return (MB_SUCCESS);
}

int mbview_enableadjustnavs(int verbose, size_t instance, int *error) {
  (void)verbose;
  return (MB_SUCCESS);
}

int mbview_enableviewties(int verbose, size_t instance, int *error) {
  (void)verbose;
  return (MB_SUCCESS);
}

int mbview_init(int verbose, size_t *instance, int *error) {
  (void)verbose;
  return (MB_FAILURE);
}

int mbview_open(int verbose, size_t instance, int *error) {
  (void)verbose;
  return (MB_FAILURE);
}

int mbview_setprimarycolortable(int verbose, size_t instance, int primary_colortable,
    int primary_colortable_mode, double primary_colortable_min, double primary_colortable_max,
    int *error) {
  (void)verbose;
  return (MB_SUCCESS);
}

int mbview_setprimarygrid(int verbose, size_t instance, int primary_grid_projection_mode,
    char *primary_grid_projection_id, float primary_nodatavalue, int primary_n_columns,
    int primary_n_rows, double primary_min, double primary_max, double primary_xmin,
    double primary_xmax, double primary_ymin, double primary_ymax, double primary_dx,
    double primary_dy, float *primary_data, int *error) {
  (void)verbose;
  return (MB_SUCCESS);
}

int mbview_setsensitivitynotify(int verbose, size_t instance, void(mbview_sensitivity_notify)(),
    int *error) {
  (void)verbose;
  return (MB_SUCCESS);
}

int mbview_setslopecolortable(int verbose, size_t instance, int slope_colortable,
    int slope_colortable_mode, double slope_colortable_min, double slope_colortable_max, int *error) {
  (void)verbose;
  return (MB_SUCCESS);
}

int mbview_setviewcontrols(int verbose, size_t instance, int display_mode, int mouse_mode,
    int grid_mode, int primary_histogram, int primaryslope_histogram, int secondary_histogram,
    int primary_shade_mode, int slope_shade_mode, int secondary_shade_mode, int grid_contour_mode,
    int site_view_mode, int route_view_mode, int nav_view_mode, int navswathbounds_view_mode,
    int navdrape_view_mode, int vector_view_mode, double exaggeration, double modelelevation3d,
    double modelazimuth3d, double viewelevation3d, double viewazimuth3d,
    double illuminate_magnitude, double illuminate_elevation, double illuminate_azimuth,
    double slope_magnitude, double overlay_shade_magnitude, double overlay_shade_center,
    int overlay_shade_mode, double contour_interval, int display_projection_mode,
    char *display_projection_id, int *error) {
  (void)verbose;
  return (MB_SUCCESS);
}

int mbview_setwindowparms(int verbose, size_t instance, int (*mbview_dismiss_notify)(size_t),
    char *title, int xo, int yo, int width, int height, int lorez_dimension, int hirez_dimension,
    int lorez_navdecimate, int hirez_navdecimate, int *error) {
  (void)verbose;
  return (MB_SUCCESS);
}

int mbview_update(int verbose, size_t instance, int *error) {
  (void)verbose;
  return (MB_SUCCESS);
}

