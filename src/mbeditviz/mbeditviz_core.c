/*--------------------------------------------------------------------
 *    The MB-system:  mbeditviz_core.c  11/7/2026
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
 * The toolkit-free functions of mbeditviz, moved unchanged from
 * mbeditviz_callbacks.c and shared by mbeditviz and mbeditviz-qt: viewing,
 * regridding and updating the grid, opening data, quitting, and the
 * notifications from the mbview window that open the 3D soundings view.
 * The only changes: the bell is mbeditviz_ui_bell() instead of XBell(), and
 * the View Selected work takes the selected list positions as arguments.
 */

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_status.h"
#include "mbsys_singlebeam.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1900)
#include <windows.h>
#endif

#ifndef MBVIEW_NO_MOTIF
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#else
#include "mbv_xtypes.h"
#endif
#include "mbview.h"
#include "mbeditviz.h"

/* a scratch buffer for messages (as in mbeditviz_callbacks.c) */
static char value_text[MB_PATH_MAXLINE];

/* the names of the gridding algorithms (MBEV_GRID_ALGORITHM_*) */
static char *mbev_grid_algorithm_label[] = {"Simple Mean", "Footprint", "Shoal Bias"};

/*------------------------------------------------------------------------------*/
void do_mbeditviz_quit(Widget w, XtPointer client_data, XtPointer call_data) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_quit\n");
#endif
  do_mbeditviz_message_on("Shutting down...");

  /* destroy any mbview window */
  if (mbev_grid.status == MBEV_GRID_VIEWED) {
    /* destroy any mb3dsoundings window */
    mbev_status = mb3dsoundings_end(mbev_verbose, &mbev_error);
    mbeditviz_mb3dsoundings_dismiss();

    mbev_status = mbview_destroy(mbev_verbose, 0, true, &mbev_error);
    mbev_grid.status = MBEV_GRID_NOTVIEWED;
  }

  /* destroy the grid */
  if (mbev_grid.status != MBEV_GRID_NONE)
    mbeditviz_destroy_grid();

  /* loop over all files to be sure all files are unloaded */
  for (int ifile = 0; ifile < mbev_num_files; ifile++) {
    struct mbev_file_struct *file = &(mbev_files[ifile]);
    if (file->load_status) {
      // Unload file, asserting unlock
      mbeditviz_unload_file(ifile, true);
    }
  }

  /* reset the gui */
  do_mbeditviz_update_gui();
  do_mbeditviz_message_off();

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_quit status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/
void do_mbeditviz_viewall(Widget w, XtPointer client_data, XtPointer call_data) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_viewall\n");
#endif

  /* destroy any mbview window */
  if (mbev_grid.status == MBEV_GRID_VIEWED) {
    /* destroy any mb3dsoundings window */
    mbev_status = mb3dsoundings_end(mbev_verbose, &mbev_error);
    mbeditviz_mb3dsoundings_dismiss();

    mbev_status = mbview_destroy(mbev_verbose, 0, true, &mbev_error);
    mbev_grid.status = MBEV_GRID_NOTVIEWED;
#ifdef MBEDITVIZ_GUI_DEBUG
    fprintf(stderr, "do_mbeditviz_viewall destroyed previous windows\n");
#endif
  }

  /* destroy old grid */
  if (mbev_grid.status != MBEV_GRID_NONE) {
    mbeditviz_destroy_grid();
#ifdef MBEDITVIZ_GUI_DEBUG
    fprintf(stderr, "do_mbeditviz_viewall destroyed old grid\n");
#endif
  }


/* loop over all files to be sure all files are loaded */
#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_viewall loading files...\n");
#endif
  do_mbeditviz_message_on("Loading files...");
  int loadcount = 0;
  for (int ifile = 0; ifile < mbev_num_files; ifile++) {
#ifdef MBEDITVIZ_GUI_DEBUG
    fprintf(stderr, "do_mbeditviz_viewall loading file %d\n", ifile);
#endif
    struct mbev_file_struct *file = &(mbev_files[ifile]);
    if (!file->load_status) {
      snprintf(value_text, sizeof(value_text), "Loading file %d of %d...", ifile + 1, mbev_num_files);
      do_mbeditviz_message_on(value_text);
#ifdef MBEDITVIZ_GUI_DEBUG
      fprintf(stderr, "do_mbeditviz_viewall loading file %d of %d...\n", ifile + 1, mbev_num_files);
#endif
      // Load file, asserting lock
      mbeditviz_load_file(ifile, true);
    }
    loadcount++;
#ifdef MBEDITVIZ_GUI_DEBUG
    fprintf(stderr, "do_mbeditviz_viewall mbev_status:%d loadcount:%d\n", mbev_status, loadcount);
#endif
  }
  do_mbeditviz_message_off();

/* put up dialog on grid parameters */
#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_viewall mbev_status:%d loadcount:%d\n", mbev_status, loadcount);
#endif
  if (mbev_status == MB_SUCCESS && loadcount > 0) {
#ifdef MBEDITVIZ_GUI_DEBUG
    fprintf(stderr, "do_mbeditviz_viewall calling do_mbeditviz_gridparameters\n");
#endif
    do_mbeditviz_gridparameters(w, client_data, call_data);
  }
  else {
    mbeditviz_ui_bell();
  }

/* reset the gui */
#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_viewall calling do_mbeditviz_update_gui\n");
#endif
  do_mbeditviz_update_gui();

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_viewall status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/
void do_mbeditviz_regrid(Widget w, XtPointer client_data, XtPointer call_data) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_regrid\n");
#endif

  /* destroy any mbview window */
  if (mbev_grid.status == MBEV_GRID_VIEWED) {
    /* destroy any mb3dsoundings window */
    mbev_status = mb3dsoundings_end(mbev_verbose, &mbev_error);
    mbeditviz_mb3dsoundings_dismiss();

    mbev_status = mbview_destroy(mbev_verbose, 0, true, &mbev_error);
    mbev_grid.status = MBEV_GRID_NOTVIEWED;
  }

  /* destroy old grid */
  if (mbev_grid.status != MBEV_GRID_NONE)
    mbeditviz_destroy_grid();

  /* loop over all files to be count loaded files */
  int loadcount = 0;
  for (int ifile = 0; ifile < mbev_num_files; ifile++) {
    struct mbev_file_struct *file = &(mbev_files[ifile]);
    if (file->load_status)
      loadcount++;
  }

/* put up dialog on grid parameters */
#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, " mbev_status:%d loadcount:%d\n", mbev_status, loadcount);
#endif
  if (mbev_status == MB_SUCCESS && loadcount > 0) {
    do_mbeditviz_gridparameters(w, client_data, call_data);
  }
  else {
    mbeditviz_ui_bell();
  }

  /* reset the gui */
  do_mbeditviz_update_gui();

  /* reset status */
  mbev_status = MB_SUCCESS;
  mbev_error = MB_ERROR_NO_ERROR;

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_regrid status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/
void do_mbeditviz_updategrid(Widget w, XtPointer client_data, XtPointer call_data) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_updategrid\n");
#endif
  do_mbeditviz_mbview_dismiss_notify(0);

  /* loop over all files to be sure all files are loaded */
  int loadcount = 0;
  for (int ifile = 0; ifile < mbev_num_files; ifile++) {
    struct mbev_file_struct *file = &(mbev_files[ifile]);
    if (file->load_status)
      loadcount++;
  }

  /* make the grid and display grid */
  if (mbev_status == MB_SUCCESS && loadcount > 0) {
    /* make the grid */
    do_mbeditviz_message_on("Making grid...");
    mbev_status = mbeditviz_setup_grid();
    mbeditviz_project_soundings();
    mbev_status = mbeditviz_make_grid();

    /* display grid */
    do_mbeditviz_viewgrid();

    do_mbeditviz_message_off();
  }
  else {
    do_mbeditviz_message_off();
    mbeditviz_ui_bell();
  }

  /* reset the gui */
  do_mbeditviz_update_gui();

  /* reset status */
  mbev_status = MB_SUCCESS;
  mbev_error = MB_ERROR_NO_ERROR;

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_updategrid status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/
void do_mbeditviz_viewgrid() {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_viewgrid\n");
#endif

  struct mbev_file_struct *file;
  struct mbev_ping_struct *ping;
  char mbv_title[MB_PATH_MAXLINE];
  int mbv_xo;
  int mbv_yo;
  int mbv_width;
  int mbv_height;
  int mbv_lorez_dimension;
  int mbv_hirez_dimension;
  int mbv_lorez_navdecimate;
  int mbv_hirez_navdecimate;
  int mbv_display_mode;
  int mbv_mouse_mode;
  int mbv_grid_mode;
  int mbv_primary_histogram;
  int mbv_primaryslope_histogram;
  int mbv_secondary_histogram;
  int mbv_primary_shade_mode;
  int mbv_slope_shade_mode;
  int mbv_secondary_shade_mode;
  int mbv_grid_contour_mode;
  int mbv_site_view_mode;
  int mbv_route_view_mode;
  int mbv_nav_view_mode;
  int mbv_navswathbounds_view_mode;
  int mbv_navdrape_view_mode;
  int mbv_vector_view_mode;
  int mbv_primary_colortable;
  int mbv_primary_colortable_mode;
  double mbv_primary_colortable_min;
  double mbv_primary_colortable_max;
  int mbv_slope_colortable;
  int mbv_slope_colortable_mode;
  double mbv_slope_colortable_min;
  double mbv_slope_colortable_max;
  int mbv_secondary_colortable;
  int mbv_secondary_colortable_mode;
  double mbv_secondary_colortable_min;
  double mbv_secondary_colortable_max;
  double mbv_exaggeration;
  double mbv_modelelevation3d;
  double mbv_modelazimuth3d;
  double mbv_viewelevation3d;
  double mbv_viewazimuth3d;
  double mbv_illuminate_magnitude;
  double mbv_illuminate_elevation;
  double mbv_illuminate_azimuth;
  double mbv_slope_magnitude;
  double mbv_overlay_shade_magnitude;
  double mbv_overlay_shade_center;
  int mbv_overlay_shade_mode;
  double mbv_contour_interval;
  double *navtime_d = NULL;
  double *navlon = NULL;
  double *navlat = NULL;
  double *navz = NULL;
  double *navheading = NULL;
  double *navspeed = NULL;
  double *navportlon = NULL;
  double *navportlat = NULL;
  double *navstbdlon = NULL;
  double *navstbdlat = NULL;
  unsigned int *navline = NULL;
  unsigned int *navshot = NULL;
  unsigned int *navcdp = NULL;
  int ifile, iping;

  /* display grid */
  if (mbev_status == MB_SUCCESS && mbev_grid.status == MBEV_GRID_NOTVIEWED) {
    /* set parameters */
    snprintf(mbv_title, sizeof(mbv_title), "MBeditviz Survey Viewer");
    mbv_xo = 200;
    mbv_yo = 200;
    mbv_width = 560;
    mbv_height = 500;
    mbv_lorez_dimension = 100;
    mbv_hirez_dimension = 500;
    mbv_lorez_navdecimate = 5;
    mbv_hirez_navdecimate = 1;
    mbv_display_mode = MBV_DISPLAY_2D;
    mbv_mouse_mode = MBV_MOUSE_MOVE;
    mbv_grid_mode = MBV_GRID_VIEW_PRIMARY;
    mbv_primary_histogram = false;
    mbv_primaryslope_histogram = false;
    mbv_secondary_histogram = false;
    mbv_primary_shade_mode = MBV_SHADE_VIEW_SLOPE;
    mbv_slope_shade_mode = MBV_SHADE_VIEW_NONE;
    mbv_secondary_shade_mode = MBV_SHADE_VIEW_NONE;
    mbv_grid_contour_mode = MBV_VIEW_OFF;
    mbv_site_view_mode = MBV_VIEW_OFF;
    mbv_route_view_mode = MBV_VIEW_OFF;
    mbv_nav_view_mode = MBV_VIEW_OFF;
    mbv_navswathbounds_view_mode = MBV_VIEW_OFF;
    mbv_navdrape_view_mode = MBV_VIEW_OFF;
    mbv_vector_view_mode = MBV_VIEW_OFF;
    mbv_primary_colortable = MBV_COLORTABLE_HAXBY;
    mbv_primary_colortable_mode = MBV_COLORTABLE_NORMAL;
    mbv_primary_colortable_min = mbev_grid.min;
    mbv_primary_colortable_max = mbev_grid.max;
    mbv_slope_colortable = MBV_COLORTABLE_HAXBY;
    mbv_slope_colortable_mode = MBV_COLORTABLE_REVERSED;
    mbv_slope_colortable_min = 0.0;
    mbv_slope_colortable_max = 0.5;
    mbv_secondary_colortable = MBV_COLORTABLE_HAXBY;
    mbv_secondary_colortable_mode = MBV_COLORTABLE_NORMAL;
    mbv_secondary_colortable_min = mbev_grid.smin;
    mbv_secondary_colortable_max = mbev_grid.smax;
    mbv_exaggeration = 1.0;
    mbv_modelelevation3d = 90.0;
    mbv_modelazimuth3d = 0.0;
    mbv_viewelevation3d = 90.0;
    mbv_viewazimuth3d = 0.0;
    mbv_illuminate_magnitude = 1.0;
    mbv_illuminate_elevation = 5.0;
    mbv_illuminate_azimuth = 90.0;
    mbv_slope_magnitude = 1.0;
    mbv_overlay_shade_magnitude = 1.0;
    mbv_overlay_shade_center = 0.0;
    mbv_overlay_shade_mode = MBV_COLORTABLE_NORMAL;
    mbv_contour_interval = pow(10.0, floor(log10(mbev_grid.max - mbev_grid.min)) - 1.0);

    /* initialize mbview window */
    mbev_status = mbview_init(mbev_verbose, &mbev_instance, &mbev_error);

    /* set basic mbview window parameters */
    mbev_status = mbview_setwindowparms(mbev_verbose, mbev_instance, &do_mbeditviz_mbview_dismiss_notify, mbv_title, mbv_xo,
                                        mbv_yo, mbv_width, mbv_height, mbv_lorez_dimension, mbv_hirez_dimension,
                                        mbv_lorez_navdecimate, mbv_hirez_navdecimate, &mbev_error);

    /* set basic mbview view controls */
    if (mbev_status == MB_SUCCESS)
      mbev_status = mbview_setviewcontrols(
          mbev_verbose, mbev_instance, mbv_display_mode, mbv_mouse_mode, mbv_grid_mode, mbv_primary_histogram,
          mbv_primaryslope_histogram, mbv_secondary_histogram, mbv_primary_shade_mode, mbv_slope_shade_mode,
          mbv_secondary_shade_mode, mbv_grid_contour_mode, mbv_site_view_mode, mbv_route_view_mode, 
          mbv_nav_view_mode, mbv_navswathbounds_view_mode, mbv_navdrape_view_mode, mbv_vector_view_mode, 
          mbv_exaggeration, mbv_modelelevation3d, mbv_modelazimuth3d,
          mbv_viewelevation3d, mbv_viewazimuth3d, mbv_illuminate_magnitude, mbv_illuminate_elevation,
          mbv_illuminate_azimuth, mbv_slope_magnitude, mbv_overlay_shade_magnitude, mbv_overlay_shade_center,
          mbv_overlay_shade_mode, mbv_contour_interval, MBV_PROJECTION_PROJECTED, mbev_grid.projection_id, &mbev_error);

    /* set primary grid data */
    if (mbev_status == MB_SUCCESS)
      mbev_status = mbview_setprimarygrid(mbev_verbose, mbev_instance, MBV_PROJECTION_PROJECTED, mbev_grid.projection_id,
                                          mbev_grid.nodatavalue, mbev_grid.n_columns, mbev_grid.n_rows, mbev_grid.min, mbev_grid.max,
                                          mbev_grid.boundsutm[0], mbev_grid.boundsutm[1], mbev_grid.boundsutm[2],
                                          mbev_grid.boundsutm[3], mbev_grid.dx, mbev_grid.dy, mbev_grid.val, &mbev_error);

    /* set more mbview control values */
    if (mbev_status == MB_SUCCESS)
      mbev_status =
          mbview_setprimarycolortable(mbev_verbose, mbev_instance, mbv_primary_colortable, mbv_primary_colortable_mode,
                                      mbv_primary_colortable_min, mbv_primary_colortable_max, &mbev_error);
    if (mbev_status == MB_SUCCESS)
      mbev_status = mbview_setslopecolortable(mbev_verbose, mbev_instance, mbv_slope_colortable, mbv_slope_colortable_mode,
                                              mbv_slope_colortable_min, mbv_slope_colortable_max, &mbev_error);

    /* open up mbview window */
    mbev_status = mbview_open(mbev_verbose, mbev_instance, &mbev_error);

    /* set grid status */
    if (mbev_status == MB_SUCCESS)
      mbev_grid.status = MBEV_GRID_VIEWED;

    /* set more mbview control values */
    if (mbev_status == MB_SUCCESS)
      mbev_status = mbview_setsecondarygrid(
          mbev_verbose, mbev_instance, MBV_PROJECTION_PROJECTED, mbev_grid.projection_id, mbev_grid.nodatavalue,
          mbev_grid.n_columns, mbev_grid.n_rows, mbev_grid.smin, mbev_grid.smax, mbev_grid.boundsutm[0], mbev_grid.boundsutm[1],
          mbev_grid.boundsutm[2], mbev_grid.boundsutm[3], mbev_grid.dx, mbev_grid.dy, mbev_grid.sgm, &mbev_error);
    if (mbev_status == MB_SUCCESS)
      mbev_status = mbview_setsecondarycolortable(mbev_verbose, mbev_instance, mbv_secondary_colortable,
                                                  mbv_secondary_colortable_mode, mbv_secondary_colortable_min,
                                                  mbv_secondary_colortable_max, mbv_overlay_shade_magnitude,
                                                  mbv_overlay_shade_center, mbv_overlay_shade_mode, &mbev_error);
    mbev_status = mbview_setsecondaryname(mbev_verbose, mbev_instance, "Standard Deviation", &mbev_error);

    /* update widgets */
    if (mbev_status == MB_SUCCESS)
      mbev_status = mbview_update(mbev_verbose, mbev_instance, &mbev_error);

    /* add navigation to view */
    for (ifile = 0; ifile < mbev_num_files; ifile++) {
      file = &(mbev_files[ifile]);
      if (file->load_status && file->num_pings > 0) {
        /* set message */
        snprintf(value_text, sizeof(value_text), "Loading nav %d of %d...", ifile + 1, mbev_num_files);
        do_mbeditviz_message_on(value_text);

        /* reset status for this file - mbev_status is a global gating every
            allocation below, and must not carry a failure over from a
            previous file in this loop or every subsequent file would
            silently skip having its navigation added to the view */
        mbev_status = MB_SUCCESS;

        /* allocate arrays */
        if ((navtime_d = (double *)malloc(file->num_pings * sizeof(double))) == NULL)
          mbev_status = MB_FAILURE;
        if (mbev_status == MB_SUCCESS)
          if ((navlon = malloc(file->num_pings * sizeof(double))) == NULL)
            mbev_status = MB_FAILURE;
        if (mbev_status == MB_SUCCESS)
          if ((navlat = malloc(file->num_pings * sizeof(double))) == NULL)
            mbev_status = MB_FAILURE;
        if (mbev_status == MB_SUCCESS)
          if ((navz = malloc(file->num_pings * sizeof(double))) == NULL)
            mbev_status = MB_FAILURE;
        if (mbev_status == MB_SUCCESS)
          if ((navheading = malloc(file->num_pings * sizeof(double))) == NULL)
            mbev_status = MB_FAILURE;
        if (mbev_status == MB_SUCCESS)
          if ((navspeed = malloc(file->num_pings * sizeof(double))) == NULL)
            mbev_status = MB_FAILURE;
        if (mbev_status == MB_SUCCESS)
          if ((navportlon = malloc(file->num_pings * sizeof(double))) == NULL)
            mbev_status = MB_FAILURE;
        if (mbev_status == MB_SUCCESS)
          if ((navportlat = malloc(file->num_pings * sizeof(double))) == NULL)
            mbev_status = MB_FAILURE;
        if (mbev_status == MB_SUCCESS)
          if ((navstbdlon = malloc(file->num_pings * sizeof(double))) == NULL)
            mbev_status = MB_FAILURE;
        if (mbev_status == MB_SUCCESS)
          if ((navstbdlat = malloc(file->num_pings * sizeof(double))) == NULL)
            mbev_status = MB_FAILURE;
        if (mbev_status == MB_SUCCESS)
          if ((navline = malloc(file->num_pings * sizeof(int))) == NULL)
            mbev_status = MB_FAILURE;
        if (mbev_status == MB_SUCCESS)
          if ((navshot = malloc(file->num_pings * sizeof(int))) == NULL)
            mbev_status = MB_FAILURE;
        if (mbev_status == MB_SUCCESS)
          if ((navcdp = malloc(file->num_pings * sizeof(int))) == NULL)
            mbev_status = MB_FAILURE;

        /* copy nav data */
        if (mbev_status == MB_SUCCESS)
          for (iping = 0; iping < file->num_pings; iping++) {
            ping = (struct mbev_ping_struct *)&(file->pings[iping]);
            navtime_d[iping] = ping->time_d;
            navlon[iping] = ping->navlon;
            navlat[iping] = ping->navlat;
            navz[iping] = -ping->sensordepth;
            navheading[iping] = ping->heading;
            navspeed[iping] = ping->speed;
            navportlon[iping] = ping->portlon;
            navportlat[iping] = ping->portlat;
            navstbdlon[iping] = ping->stbdlon;
            navstbdlat[iping] = ping->stbdlat;
            navline[iping] = 0;
            navshot[iping] = iping;
            navcdp[iping] = 0;
          }

        /* add nav data to mbview */
        if (mbev_status == MB_SUCCESS)
          mbev_status = mbview_addnav(mbev_verbose, mbev_instance, file->num_pings, navtime_d, navlon, navlat, navz,
                                      navheading, navspeed, navportlon, navportlat, navstbdlon, navstbdlat, navline,
                                      navshot, navcdp, MBV_COLOR_BLACK, 2, file->name, MB_PROCESSED_NONE, file->path,
                                      file->path, file->format, true, false, false, false, 1, &mbev_error);

        /* deallocate memory used for data arrays */
        free(navtime_d);
        navtime_d = NULL;
        free(navlon);
        navlon = NULL;
        free(navlat);
        navlat = NULL;
        free(navz);
        navz = NULL;
        free(navheading);
        navheading = NULL;
        free(navspeed);
        navspeed = NULL;
        free(navportlon);
        navportlon = NULL;
        free(navportlat);
        navportlat = NULL;
        free(navstbdlon);
        navstbdlon = NULL;
        free(navstbdlat);
        navstbdlat = NULL;
        free(navline);
        navline = NULL;
        free(navshot);
        navshot = NULL;
        free(navcdp);
        navcdp = NULL;
      }
    }
    mbview_enableviewnavs(mbev_verbose, mbev_instance, &mbev_error);
    mbev_status = mbview_update(mbev_verbose, mbev_instance, &mbev_error);
    do_mbeditviz_message_off();

    /* add pick notify functions */
    mbview_addpicknotify(mbev_verbose, 0, MBV_PICK_ONEPOINT, do_mbeditviz_pickonepoint_notify, &mbev_error);
    mbview_addpicknotify(mbev_verbose, 0, MBV_PICK_TWOPOINT, do_mbeditviz_picktwopoint_notify, &mbev_error);
    mbview_addpicknotify(mbev_verbose, 0, MBV_PICK_AREA, do_mbeditviz_pickarea_notify, &mbev_error);
    mbview_addpicknotify(mbev_verbose, 0, MBV_PICK_REGION, do_mbeditviz_pickregion_notify, &mbev_error);
    mbview_addpicknotify(mbev_verbose, 0, MBV_PICK_SITE, do_mbeditviz_picksite_notify, &mbev_error);
    mbview_addpicknotify(mbev_verbose, 0, MBV_PICK_ROUTE, do_mbeditviz_pickroute_notify, &mbev_error);
    mbview_addpicknotify(mbev_verbose, 0, MBV_PICK_NAV, do_mbeditviz_picknav_notify, &mbev_error);

    /* add action button */
    mbview_addaction(mbev_verbose, mbev_instance, do_mbeditviz_regrid_notify, "Update Bathymetry Grid", MBV_PICKMASK_NONE,
                     &mbev_error);

    /* if some soundings flagged as secondary have been loaded, add actions to enable/disable using them */
    if (mbev_num_soundings_secondary > 0) {

      /* add action button */
      mbview_addaction(mbev_verbose, mbev_instance, do_mbeditviz_enablesecondarypicks_notify, "Enable Secondary Picks", MBV_STATEMASK_20,
                       &mbev_error);
      mbview_setstate(mbev_verbose, mbev_instance, MBV_STATEMASK_20, 1, &mbev_error);

      /* add action button */
      mbview_addaction(mbev_verbose, mbev_instance, do_mbeditviz_disablesecondarypicks_notify, "Disable Secondary Picks", MBV_STATEMASK_21,
                       &mbev_error);
      mbview_setstate(mbev_verbose, mbev_instance, MBV_STATEMASK_21, 0, &mbev_error);
    }

    /* add colorchange notify function */
    mbview_setcolorchangenotify(mbev_verbose, mbev_instance, do_mbeditviz_colorchange_notify, &mbev_error);
  }

  /* reset the gui */
  do_mbeditviz_update_gui();

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_viewgrid status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/
int do_mbeditviz_mbview_dismiss_notify(size_t instance) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:   %zu\n", instance);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_mbview_dismiss_notify status:%d\n", mbev_status);
#endif

/* destroy any mb3dsoundings window */
#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "1 do_mbeditviz_mbview_dismiss_notify status:%d\n", mbev_status);
#endif
  mbev_status = mb3dsoundings_end(mbev_verbose, &mbev_error);
#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "2 do_mbeditviz_mbview_dismiss_notify status:%d\n", mbev_status);
#endif
  mbeditviz_mb3dsoundings_dismiss();
#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "3 do_mbeditviz_mbview_dismiss_notify status:%d\n", mbev_status);
#endif

/* destroy the grid */
#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "mbev_grid.status:%d\n", mbev_grid.status);
#endif
  if (mbev_grid.status != MBEV_GRID_NONE)
    mbeditviz_destroy_grid();
#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "4 do_mbeditviz_mbview_dismiss_notify status:%d\n", mbev_status);
#endif

  /* reset the gui */
  do_mbeditviz_update_gui();

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_mbview_dismiss_notify status:%d\n", mbev_status);
#endif

  return (mbev_status);
}
/*------------------------------------------------------------------------------*/
int do_mbeditviz_opendata(char *input_file_ptr, int format) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       input_file_ptr:    %s\n", input_file_ptr);
    fprintf(stderr, "dbg2       format:            %d\n", format);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_opendata:%s %d\n", input_file_ptr, format);
#endif
  do_mbeditviz_message_on("Reading data list...");

  mbeditviz_open_data(input_file_ptr, format);

  do_mbeditviz_message_off();

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_opendata status:%d\n", mbev_status);
#endif

  return (mbev_status);
}
/*------------------------------------------------------------------------------*/
void do_mbeditviz_pickonepoint_notify(size_t instance) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:    %zu\n", instance);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_pickonepoint_notify:%zu\n", instance);
  fprintf(stderr, "return do_mbeditviz_pickonepoint_notify status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/
void do_mbeditviz_picktwopoint_notify(size_t instance) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:    %zu\n", instance);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_picktwopoint_notify:%zu\n", instance);
  fprintf(stderr, "return do_mbeditviz_picktwopoint_notify status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/
void do_mbeditviz_pickarea_notify(size_t instance) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:    %zu\n", instance);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_pickarea_notify:%zu\n", instance);
#endif

  mbeditviz_selectarea(instance);
  mbev_status = mb3dsoundings_open(mbev_verbose, &mbev_selected, &mbev_error);
  if (mbev_status == MB_SUCCESS)
    mbev_selected.displayed = true;
  mbev_status = mb3dsoundings_set_dismiss_notify(mbev_verbose, &mbeditviz_mb3dsoundings_dismiss, &mbev_error);
  mbev_status = mb3dsoundings_set_edit_notify(mbev_verbose, &mbeditviz_mb3dsoundings_edit, &mbev_error);
  mbev_status = mb3dsoundings_set_info_notify(mbev_verbose, &mbeditviz_mb3dsoundings_info, &mbev_error);
  mbev_status = mb3dsoundings_set_bias_notify(mbev_verbose, &mbeditviz_mb3dsoundings_bias, &mbev_error);
  mbev_status = mb3dsoundings_set_biasapply_notify(mbev_verbose, &mbeditviz_mb3dsoundings_biasapply, &mbev_error);
  mbev_status = mb3dsoundings_set_flagsparsevoxels_notify(mbev_verbose, &mbeditviz_mb3dsoundings_flagsparsevoxels, &mbev_error);
  mbev_status = mb3dsoundings_set_colorsoundings_notify(mbev_verbose, &mbeditviz_mb3dsoundings_colorsoundings, &mbev_error);
  mbev_status = mb3dsoundings_set_optimizebiasvalues_notify(mbev_verbose, &mbeditviz_mb3dsoundings_optimizebiasvalues, &mbev_error);

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_picktwopoint_notify status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/
void do_mbeditviz_pickregion_notify(size_t instance) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:    %zu\n", instance);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_pickregion_notify:%zu\n", instance);
#endif

  mbeditviz_selectregion(instance);
  mbev_status = mb3dsoundings_open(mbev_verbose, &mbev_selected, &mbev_error);
  if (mbev_status == MB_SUCCESS)
    mbev_selected.displayed = true;
  mbev_status = mb3dsoundings_set_dismiss_notify(mbev_verbose, &mbeditviz_mb3dsoundings_dismiss, &mbev_error);
  mbev_status = mb3dsoundings_set_edit_notify(mbev_verbose, &mbeditviz_mb3dsoundings_edit, &mbev_error);
  mbev_status = mb3dsoundings_set_info_notify(mbev_verbose, &mbeditviz_mb3dsoundings_info, &mbev_error);
  mbev_status = mb3dsoundings_set_bias_notify(mbev_verbose, &mbeditviz_mb3dsoundings_bias, &mbev_error);
  mbev_status = mb3dsoundings_set_biasapply_notify(mbev_verbose, &mbeditviz_mb3dsoundings_biasapply, &mbev_error);
  mbev_status = mb3dsoundings_set_flagsparsevoxels_notify(mbev_verbose, &mbeditviz_mb3dsoundings_flagsparsevoxels, &mbev_error);
  mbev_status = mb3dsoundings_set_colorsoundings_notify(mbev_verbose, &mbeditviz_mb3dsoundings_colorsoundings, &mbev_error);
  mbev_status = mb3dsoundings_set_optimizebiasvalues_notify(mbev_verbose, &mbeditviz_mb3dsoundings_optimizebiasvalues, &mbev_error);

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_pickregion_notify status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/
void do_mbeditviz_picksite_notify(size_t instance) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:    %zu\n", instance);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_picksite_notify:%zu\n", instance);
  fprintf(stderr, "return do_mbeditviz_picksite_notify status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/
void do_mbeditviz_pickroute_notify(size_t instance) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:    %zu\n", instance);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_pickroute_notify:%zu\n", instance);
  fprintf(stderr, "return do_mbeditviz_pickroute_notify status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/
void do_mbeditviz_picknav_notify(size_t instance) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:    %zu\n", instance);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_picknav_notify:%zu\n", instance);
#endif

  mbeditviz_selectnav(instance);

  /* open ping editor */
  /* BxManageCB(parent, (XtPointer)"form_pingedit", NULL); */

  /* open mb3dsoundings 3d editor */
  mbev_status = mb3dsoundings_open(mbev_verbose, &mbev_selected, &mbev_error);
  if (mbev_status == MB_SUCCESS)
    mbev_selected.displayed = true;
  mbev_status = mb3dsoundings_set_dismiss_notify(mbev_verbose, &mbeditviz_mb3dsoundings_dismiss, &mbev_error);
  mbev_status = mb3dsoundings_set_edit_notify(mbev_verbose, &mbeditviz_mb3dsoundings_edit, &mbev_error);
  mbev_status = mb3dsoundings_set_info_notify(mbev_verbose, &mbeditviz_mb3dsoundings_info, &mbev_error);
  mbev_status = mb3dsoundings_set_bias_notify(mbev_verbose, &mbeditviz_mb3dsoundings_bias, &mbev_error);
  mbev_status = mb3dsoundings_set_biasapply_notify(mbev_verbose, &mbeditviz_mb3dsoundings_biasapply, &mbev_error);
  mbev_status = mb3dsoundings_set_flagsparsevoxels_notify(mbev_verbose, &mbeditviz_mb3dsoundings_flagsparsevoxels, &mbev_error);
  mbev_status = mb3dsoundings_set_colorsoundings_notify(mbev_verbose, &mbeditviz_mb3dsoundings_colorsoundings, &mbev_error);
  mbev_status = mb3dsoundings_set_optimizebiasvalues_notify(mbev_verbose, &mbeditviz_mb3dsoundings_optimizebiasvalues, &mbev_error);

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_picknav_notify status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/
void do_mbeditviz_regrid_notify(Widget w, XtPointer client_data, XtPointer call_data) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_regrid_notify\n");
#endif

  double rollbias;
  double pitchbias;
  double headingbias;
  double timelag;
  double snell;

  /* get current bias parameters */
  mb3dsoundings_get_bias_values(mbev_verbose, &rollbias, &pitchbias, &headingbias, &timelag, &snell, &mbev_error);

  /* regrid the bathymetry */
  mbeditviz_mb3dsoundings_biasapply(rollbias, pitchbias, headingbias, timelag, snell);

  /* reset the gui */
  do_mbeditviz_update_gui();

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_regrid_notify status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/
void do_mbeditviz_enablesecondarypicks_notify(Widget w, XtPointer client_data, XtPointer call_data) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_enablesecondarypicks_notify\n");
#endif

  mbview_setstate(mbev_verbose, mbev_instance, MBV_STATEMASK_20, 0, &mbev_error);
  mbview_setstate(mbev_verbose, mbev_instance, MBV_STATEMASK_21, 1, &mbev_error);

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_enablesecondarypicks_notify status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/
void do_mbeditviz_disablesecondarypicks_notify(Widget w, XtPointer client_data, XtPointer call_data) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:           %p\n", w);
    fprintf(stderr, "dbg2       client_data: %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:   %p\n", call_data);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_disablesecondarypicks_notify\n");
#endif

  mbview_setstate(mbev_verbose, mbev_instance, MBV_STATEMASK_20, 1, &mbev_error);
  mbview_setstate(mbev_verbose, mbev_instance, MBV_STATEMASK_21, 0, &mbev_error);

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_disablesecondarypicks_notify status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/
void do_mbeditviz_colorchange_notify(size_t instance) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:    %zu\n", instance);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_colorchange_notify\n");
#endif

  /* recolor any selected soundings and then replot */
  if (mbev_selected.displayed && mbev_selected.num_soundings > 0) {
  	for (int isounding = 0; isounding < mbev_selected.num_soundings; isounding++) {
  		struct mb3dsoundings_sounding_struct *sounding = &mbev_selected.soundings[isounding];
  		if (mb_beam_ok(sounding->beamflag)) {
        mbview_colorvalue_instance(instance, sounding->z, &(sounding->r), &(sounding->g), &(sounding->b));
      }
    }
    mbev_status = mb3dsoundings_plot(mbev_verbose, &mbev_error);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_colorchange_notify status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/
/* View Selected, first part: destroy the views and the old grid */
void do_mbeditviz_viewselected_prepare(void) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "do_mbeditviz_viewselected\n");
#endif

  /* destroy any mbview window */
  if (mbev_grid.status == MBEV_GRID_VIEWED) {
    /* destroy any mb3dsoundings window */
    mbev_status = mb3dsoundings_end(mbev_verbose, &mbev_error);
    mbeditviz_mb3dsoundings_dismiss();

    mbev_status = mbview_destroy(mbev_verbose, 0, true, &mbev_error);
    mbev_grid.status = MBEV_GRID_NOTVIEWED;
  }

  /* destroy old grid */
  if (mbev_grid.status != MBEV_GRID_NONE)
    mbeditviz_destroy_grid();
}
/*------------------------------------------------------------------------------*/
/* View Selected, second part: load and view the files at the list positions
   position_list (1-based, as Motif list positions); w, client_data and
   call_data of the button go on to the grid parameters dialog */
void do_mbeditviz_viewselected_files(Widget w, XtPointer client_data, XtPointer call_data, int position_count,
                                     int *position_list) {
  if (mbev_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       w:              %p\n", w);
    fprintf(stderr, "dbg2       client_data:    %p\n", client_data);
    fprintf(stderr, "dbg2       call_data:      %p\n", call_data);
    fprintf(stderr, "dbg2       position_count: %d\n", position_count);
  }
#ifdef MBEDITVIZ_GUI_DEBUG
  {
    fprintf(stderr, "position_count:%d\n", position_count);
    for (int i = 0; i < position_count; i++)
      fprintf(stderr, "  %d %d\n", i, position_list[i]);
  }
#endif


  /* loop over all files to be sure selected files are loaded */
  do_mbeditviz_message_on("Loading files...");
  int loadcount = 0;
  for (int ifile = 0; ifile < mbev_num_files; ifile++) {
    /* find out if file is in selected list */
    bool selected = false;
    for (int i = 0; i < position_count; i++) {
      if (ifile == position_list[i] - 1)
        selected = true;
    }

    /* load unloaded selected files, unload loaded unselected files */
    struct mbev_file_struct *file = &(mbev_files[ifile]);
    if (selected && !file->load_status) {
      loadcount++;
      snprintf(value_text, sizeof(value_text), "Loading file %d of %d...", loadcount, position_count);
      do_mbeditviz_message_on(value_text);
      // Load file, asserting lock
      mbeditviz_load_file(ifile, true);
    }
    else if (selected && file->load_status) {
      loadcount++;
    }
    else if (!selected && file->load_status) {
      // Unload file, asserting unlock
      mbeditviz_unload_file(ifile, true);
    }
  }
  do_mbeditviz_message_off();

/* put up dialog on grid parameters */
#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, " mbev_status:%d loadcount:%d\n", mbev_status, loadcount);
#endif
  if (mbev_status == MB_SUCCESS && loadcount > 0) {
    do_mbeditviz_gridparameters(w, client_data, call_data);
  }
  else {
    mbeditviz_ui_bell();
  }

  /* reset the gui */
  do_mbeditviz_update_gui();

  /* reset status */
  mbev_status = MB_SUCCESS;
  mbev_error = MB_ERROR_NO_ERROR;

#ifdef MBEDITVIZ_GUI_DEBUG
  fprintf(stderr, "return do_mbeditviz_viewselected status:%d\n", mbev_status);
#endif
}
/*------------------------------------------------------------------------------*/
/* count the loaded files, pings and soundings (mbev_num_*_loaded) and write
   the status text (a label string in Motif "BX" form) into string; the status
   part of do_mbeditviz_update_gui */
void mbeditviz_status_text(char *string, size_t size) {
  /* set status text */
  mbev_num_files_loaded = 0;
  mbev_num_pings_loaded = 0;
  mbev_num_soundings_loaded = 0;
  mbev_num_soundings_secondary = 0;
  for (int i = 0; i < mbev_num_files; i++) {
    struct mbev_file_struct *file = &(mbev_files[i]);
    if (file->load_status) {
      mbev_num_files_loaded++;
      mbev_num_pings_loaded += file->num_pings;
      for (int j = 0; j < file->num_pings; j++) {
        struct mbev_ping_struct *ping = &(file->pings[j]);
        for (int k = 0; k < ping->beams_bath; k++) {
          if (!mb_beam_check_flag_unusable(ping->beamflag[k]))
            mbev_num_soundings_loaded++;
          if (mb_beam_check_flag_multipick(ping->beamflag[k]))
            mbev_num_soundings_secondary++;
        }
      }
    }
  }

  if (mbev_grid.status == MBEV_GRID_NONE)
    snprintf(string, size, ":::t\"Available Files: %d\":t\"Loaded Files: %d\":t\"Grid Not Generated\"", mbev_num_files,
            mbev_num_files_loaded);
  else
    snprintf(string, size,
            ":::t\"Available Files: %d\":t\"Loaded Files: %d\":t\"Grid:\":t\"  Lon: %f %f\":t\"  Lat: %f %f\":t\"  Cell "
            "Size: %f m\":t\"  Algorithm: %d\":t\"  Interpolation: %d\":t\"  Dimensions: %d %d\"",
            mbev_num_files, mbev_num_files_loaded, mbev_grid.bounds[0], mbev_grid.bounds[1], mbev_grid.bounds[2],
            mbev_grid.bounds[3], mbev_grid.dx, mbev_grid_algorithm, mbev_grid_interpolation,
            mbev_grid.n_columns, mbev_grid.n_rows);
}
/*------------------------------------------------------------------------------*/
/* check the load, lock and edit save file state of every file; true if the
   file list has to be rebuilt (the checks of do_mbeditviz_update_filelist) */
bool mbeditviz_filelist_check(void) {
  struct mbev_file_struct *file;

  /* esf file checking variables */
  bool esf_exists = false;
  struct stat file_status;
  char save_file[MB_PATH_MAXLINE+40];

  /* check to see if anything has changed */
  bool update_filelist = false;

  /* check for change in load status, lock status, or esf status */
  for (int i = 0; i < mbev_num_files; i++) {
    file = &(mbev_files[i]);

    /* check load status */
    if (file->load_status != file->load_status_shown) {
      file->load_status_shown = file->load_status;
      update_filelist = true;
    }

		/* swath file locking variables */
		int lock_error = MB_ERROR_NO_ERROR;
		bool locked = false;
		int lock_purpose;
		mb_path lock_program;
		mb_path lock_cpu;
		mb_path lock_user;
		char lock_date[25];

    /* check for locks */
    // int lock_status =
    mb_pr_lockinfo(mbev_verbose, mbev_files[i].path, &locked, &lock_purpose, lock_program, lock_user, lock_cpu,
                   lock_date, &lock_error);
    if (locked != file->locked) {
      file->locked = locked;
      update_filelist = true;
    }

    /* check for edit save file */
    snprintf(save_file, sizeof(save_file), "%s.esf", mbev_files[i].path);
    const int fstat = stat(save_file, &file_status);
    if (fstat == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR)
      esf_exists = true;
    else
      esf_exists = false;
    if (esf_exists != file->esf_exists) {
      file->esf_exists = esf_exists;
      update_filelist = true;
    }
  }

  return (update_filelist);
}
/*------------------------------------------------------------------------------*/
/* the list label of file i (lock or load state, edit save file, async
   data, name, format), as do_mbeditviz_update_filelist shows it */
void mbeditviz_filelist_item(int i, char *string, size_t size) {
  struct mbev_file_struct *file = &(mbev_files[i]);
  char athchar;
  char atschar;
  char atachar;
  char *lockstrptr;
  char *esfstrptr;
  const char *lockedstr = "<Locked>";
  const char *unlockedstr = "        ";
  const char *loadedstr = "<loaded>";
  const char *esfyesstr = "<esf>";
  const char *esfnostr = "     ";

  /* set label strings */
  if (file->load_status)
    lockstrptr = (char *) loadedstr;
  else if (file->locked)
    lockstrptr = (char *) lockedstr;
  else
    lockstrptr = (char *) unlockedstr;
  if (file->esf_exists)
    esfstrptr = (char *) esfyesstr;
  else
    esfstrptr = (char *) esfnostr;
  if (file->n_async_heading > 0)
    athchar = 'H';
  else
    athchar = ' ';
  if (file->n_async_sensordepth > 0)
    atschar = 'S';
  else
    atschar = ' ';
  if (file->n_async_attitude > 0)
    atachar = 'A';
  else
    atachar = ' ';
  snprintf(string, size, "%s %s %c%c%c %s %d", lockstrptr, esfstrptr, athchar, atschar, atachar, mbev_files[i].name,
          mbev_files[i].format);
}
/*------------------------------------------------------------------------------*/
/* the grid bounds and suggested grid parameters text of the grid parameters
   dialog (a label string in Motif "BX" form) */
void mbeditviz_gridparameters_text(char *string, size_t size) {
  double xx = (mbev_grid_boundsutm[1] - mbev_grid_boundsutm[0]);
  double yy = (mbev_grid_boundsutm[3] - mbev_grid_boundsutm[2]);

  snprintf(string, size, ":::t\"Grid Bounds:\""
          ":t\"    Longitude: %10.5f %10.5f  | %6.3f km  | %9.3f m\""
          ":t\"    Latitude: %9.5f %9.5f | %6.3f km  | %9.3f m\""
          ":t\"Suggested Grid Parameters:\""
          ":t\"    Cell Size: %.2f m\""
          ":t\"    Dimensions: %d %d\"",
          mbev_grid_bounds[0], mbev_grid_bounds[1], 0.001 * xx, xx,
          mbev_grid_bounds[2], mbev_grid_bounds[3], 0.001 * yy, yy,
          mbev_grid_cellsize, mbev_grid_n_columns, mbev_grid_n_rows);
}
/*------------------------------------------------------------------------------*/
/* the selected grid parameters text of the grid parameters dialog */
void mbeditviz_gridparameters_implied(char *string, size_t size) {
  // char string2[MB_PATH_MAXLINE];
  snprintf(string, size, ":::t\"Selected Grid Parameters:\""
                  ":t\"    Cell Size: %.2f m\""
                  ":t\"    Dimensions: %d %d\""
                  ":t\"    Algorithm: %s\""
                  ":t\"    Interpolation: %d cell gaps\"",
                  mbev_grid_cellsize,
                  mbev_grid_n_columns, mbev_grid_n_rows,
                  mbev_grid_algorithm_label[mbev_grid_algorithm],
                  mbev_grid_interpolation);
}
/*------------------------------------------------------------------------------*/
/* the grid dimensions for the cell size mbev_grid_cellsize */
void mbeditviz_set_grid_dimensions(void) {
  /* get updated grid dimensions */
  mbev_grid_n_columns = (mbev_grid_boundsutm[1] - mbev_grid_boundsutm[0]) / mbev_grid_cellsize + 1;
  mbev_grid_n_rows = (mbev_grid_boundsutm[3] - mbev_grid_boundsutm[2]) / mbev_grid_cellsize + 1;
  if (mbev_verbose > 0) {
    fprintf(stderr, "Grid bounds: %f %f %f %f    %f %f %f %f\n",
            mbev_grid_bounds[0], mbev_grid_bounds[1],
            mbev_grid_bounds[2], mbev_grid_bounds[3],
            mbev_grid_boundsutm[0], mbev_grid_boundsutm[1],
            mbev_grid_boundsutm[2],  mbev_grid_boundsutm[3]);
    fprintf(stderr, "cell size:%f dimensions: %d %d\n", mbev_grid_cellsize, mbev_grid_n_columns, mbev_grid_n_rows);
  }
}
