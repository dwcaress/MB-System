/*--------------------------------------------------------------------
 *    The MB-system:  mbvq_bridge.h  11/7/2026
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
 * Private interface between the C half of the mbview-qt library
 * (mbvq_core.c, compiled like the mbview core files with
 * MBVIEW_NO_MOTIF) and its Qt half (C++). Only plain C types cross it:
 * the C half owns the mbview structures, the Qt half owns the windows.
 */

#ifndef MBVIEW_QT_MBVQ_BRIDGE_H
#define MBVIEW_QT_MBVQ_BRIDGE_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* values of mbview.h and mbviewprivate.h that the Qt half needs; the C half
   checks that they agree */
#define MBVQ_MAX_WINDOWS 10 /* MBV_MAX_WINDOWS */
#define MBVQ_NO_WINDOW 999  /* MBV_NO_WINDOW */
#define MBVQ_REZ_NONE 0     /* MBV_REZ_NONE */
#define MBVQ_REZ_LOW 1      /* MBV_REZ_LOW */
#define MBVQ_REZ_HIGH 2     /* MBV_REZ_HIGH */
#define MBVQ_REZ_FULL 3     /* MBV_REZ_FULL */

/* labels the core writes through set_mbview_label_multiline_string() */
#define MBVQ_LABEL_PICKINFO 0
#define MBVQ_LABEL_PROFILEINFO 1
#define MBVQ_LABEL_ABOUTVERSION 2
#define MBVQ_LABEL_PROJECTION 3
#define MBVQ_LABEL_OTHER 4

/* ---- implemented by the C half (mbvq_core.c) ---- */

/* the Motif-free tail of mbview_open() */
int mbvq_c_open(int verbose, size_t instance, int *error);
/* the map view of an instance got a new size in physical pixels */
void mbvq_c_resize(size_t instance, int width, int height);
/* draw the map view at resolution rez (MBV_REZ_*), inside a paint */
int mbvq_c_plot(size_t instance, int rez);
/* the resolution drawn last, MBV_REZ_NONE before the first drawing */
int mbvq_c_lastdrawrez(size_t instance);
/* run the background work function once; instance MBV_NO_WINDOW for the timer */
void mbvq_c_work(size_t instance);
/* simple getters for the windows */
int mbvq_c_mouse_mode(size_t instance);
bool mbvq_c_display_3d(size_t instance);
/* select a mouse mode (MBV_MOUSE_*), as the mouse mode buttons do */
void mbvq_c_set_mouse_mode(size_t instance, int mode);
const char *mbvq_c_title(size_t instance);
void mbvq_c_geometry(size_t instance, int *xo, int *yo, int *width, int *height);
/* close an instance, as the Motif Dismiss menu does (mbview_destroy) */
void mbvq_c_destroy(size_t instance);
/* the Clear, Reset and Full buttons */
void mbvq_c_clearpicks(size_t instance);
void mbvq_c_resetview(size_t instance);
void mbvq_c_fullrender(size_t instance);

/* View menu items, in the order of the Motif View menu */
#define MBVQ_VIEW_DISPLAY_2D 0
#define MBVQ_VIEW_DISPLAY_3D 1
#define MBVQ_VIEW_GRID_PRIMARY 2
#define MBVQ_VIEW_GRID_PRIMARYSLOPE 3
#define MBVQ_VIEW_GRID_SECONDARY 4
#define MBVQ_VIEW_HISTOGRAM 5
#define MBVQ_VIEW_SHADE_NONE 6
#define MBVQ_VIEW_SHADE_ILLUMINATION 7
#define MBVQ_VIEW_SHADE_SLOPE 8
#define MBVQ_VIEW_SHADE_OVERLAY 9
#define MBVQ_VIEW_CONTOUR 10
#define MBVQ_VIEW_SITE 11
#define MBVQ_VIEW_ROUTE 12
#define MBVQ_VIEW_NAV 13
#define MBVQ_VIEW_NAVSWATHBOUNDS 14
#define MBVQ_VIEW_NAVDRAPE 15
#define MBVQ_VIEW_VECTOR 16
#define MBVQ_VIEW_POLYLINE 17
#define MBVQ_VIEW_POLYGON 18
#define MBVQ_VIEW_GISPOINT 19
#define MBVQ_VIEW_COLORTABLE_HAXBY 20
#define MBVQ_VIEW_COLORTABLE_BRIGHT 21
#define MBVQ_VIEW_COLORTABLE_MUTED 22
#define MBVQ_VIEW_COLORTABLE_GRAY 23
#define MBVQ_VIEW_COLORTABLE_FLAT 24
#define MBVQ_VIEW_COLORTABLE_SEALEVEL1 25
#define MBVQ_VIEW_COLORTABLE_SEALEVEL2 26
#define MBVQ_VIEW_PROFILE 27
#define MBVQ_VIEW_ITEMS 28

/* mouse modes, indexed by MBV_MOUSE_* (0 to 8) */
#define MBVQ_MOUSE_MODES 9

/* what the window shows: the state of the menu items and mouse modes */
struct mbvq_state {
	bool view_checked[MBVQ_VIEW_ITEMS];
	bool view_enabled[MBVQ_VIEW_ITEMS];
	int mouse_mode;
	bool mouse_enabled[MBVQ_MOUSE_MODES];
	const char *mouse_label[MBVQ_MOUSE_MODES];
};
void mbvq_c_state(size_t instance, struct mbvq_state *state);
/* a View menu item was chosen; value is the new state of a toggle */
void mbvq_c_view(size_t instance, int item, bool value);

/* Action menu */
void mbvq_c_action(size_t instance, int index);

/* Profile window */
/* show or dismiss the profile, as the View > Profile toggle and the profile
   Dismiss button (profile_view_mode) */
void mbvq_c_profile_show(size_t instance, bool on);
bool mbvq_c_profile_shown(size_t instance);
/* the size of the profile drawing in physical pixels (prwidth, prheight) */
void mbvq_c_profile_resize(size_t instance, int width, int height);
/* draw the profile, inside a paint */
void mbvq_c_plotprofile(size_t instance);
/* a profile slider: 0 width multiple (1-5), 1 slope threshold in hundredths
   (1-200), 2 vertical exaggeration in tenths (0-100) */
void mbvq_c_profile_control(size_t instance, int control, int value);
void mbvq_c_profile_controls(size_t instance, int *width, int *slope, int *exaggeration);

/* List windows (shared by all instances) */
#define MBVQ_LIST_SITE 0
#define MBVQ_LIST_ROUTE 1
#define MBVQ_LIST_NAV 2
#define MBVQ_LIST_POLYLINE 3
#define MBVQ_LIST_POLYGON 4
#define MBVQ_LIST_GISPOINT 5
#define MBVQ_LISTS 6

/* actions on the selected rows */
#define MBVQ_LIST_SELECT 0      /* rows were selected */
#define MBVQ_LIST_DELETE 1      /* Delete Selected */
#define MBVQ_LIST_SHOWHIDE 2    /* GIS: Show/Hide Selected */
#define MBVQ_LIST_TOROUTES 3    /* polylines: Convert Selected to Routes */
#define MBVQ_LIST_TOPOLYLINES 4 /* routes: Convert Selected to Polylines */

/* the rows of a list: an opaque handle, freed with mbvq_c_rows_free */
void *mbvq_c_rows(int list);
int mbvq_c_rows_count(const void *rows);
const char *mbvq_c_rows_text(const void *rows, int i);
/* the list position (1 for the first row) to select and show, 0 for none */
int mbvq_c_rows_selected(const void *rows);
void mbvq_c_rows_free(void *rows);
/* an action on the selected positions (1 for the first row); for
   MBVQ_LIST_SELECT of a GIS list, returns the position to select again
   (0 for none); the caller then redraws with mbvq_c_list_redraw() */
int mbvq_c_list_action(int list, int action, int position_count, const int *position_list);
void mbvq_c_list_redraw(void);
/* the list window was shown or dismissed, as the Motif popup and popdown */
void mbvq_c_list_shown(int list, bool shown);
/* window titles and column headers */
const char *mbvq_c_list_title(int list);
const char *mbvq_c_list_header(int list);
/* the route list offers Convert Selected to Polylines */
bool mbvq_c_routes_to_polylines(void);

/* Controls dialogs */
#define MBVQ_DIALOG_COLORBOUNDS 0
#define MBVQ_DIALOG_VIEW2D 1
#define MBVQ_DIALOG_VIEW3D 2
#define MBVQ_DIALOG_SHADE 3
#define MBVQ_DIALOG_RESOLUTION 4
#define MBVQ_DIALOG_PROJECTION 5
#define MBVQ_DIALOGS 6

/* the text fields of the dialogs, shown with "%g" as in Motif */
#define MBVQ_F_DATAMIN 0
#define MBVQ_F_DATAMAX 1
#define MBVQ_F_CONTOURS 2
#define MBVQ_F_SLOPEMIN 3
#define MBVQ_F_SLOPEMAX 4
#define MBVQ_F_OVERLAYMIN 5
#define MBVQ_F_OVERLAYMAX 6
#define MBVQ_F_ILLUM_AMP 7
#define MBVQ_F_ILLUM_AZI 8
#define MBVQ_F_ILLUM_ELEV 9
#define MBVQ_F_SLOPE_AMP 10
#define MBVQ_F_OVERLAY_AMP 11
#define MBVQ_F_OVERLAY_CENTER 12
#define MBVQ_F_MODEL_AZI 13
#define MBVQ_F_MODEL_ELEV 14
#define MBVQ_F_VIEW_AZI 15
#define MBVQ_F_VIEW_ELEV 16
#define MBVQ_F_EXAGGERATION 17
#define MBVQ_F_OFFSET3DX 18
#define MBVQ_F_OFFSET3DY 19
#define MBVQ_F_MODEL_3DZOOM 20
#define MBVQ_F_VIEW_3DZOOM 21
#define MBVQ_F_OFFSET2DX 22
#define MBVQ_F_OFFSET2DY 23
#define MBVQ_F_ZOOM2D 24
#define MBVQ_FIELDS 25

/* the toggles: 1 for Hot-to-Cold */
#define MBVQ_T_DATA_HTOC 0
#define MBVQ_T_SLOPE_HTOC 1
#define MBVQ_T_OVERLAY_HTOC 2
#define MBVQ_T_OVERLAY_SHADE_HTOC 3
#define MBVQ_TOGGLES 4

struct mbvq_controls {
	double field[MBVQ_FIELDS];
	int toggle[MBVQ_TOGGLES];
	int lorez_dimension, hirez_dimension, lorez_navdecimate, hirez_navdecimate;
	int display_projection;      /* 0 geographic, 1 UTM (any projected), 2 spheroid */
	int lonlatstyle;             /* 0 decimal degrees, 1 degrees and minutes */
	bool primary, secondary;     /* grids present */
};
void mbvq_c_controls(size_t instance, struct mbvq_controls *controls);
/* Apply of the colour bounds, shading, 3D and 2D dialogs: the texts of all
   MBVQ_FIELDS fields (only those of the dialog are used) and the toggles */
void mbvq_c_apply(size_t instance, int dialog, const char *const *text, const int *toggle);
void mbvq_c_setresolution(size_t instance, int lorez_dimension, int hirez_dimension, int lorez_navdecimate,
                          int hirez_navdecimate);
/* build the projection label (MBVQ_LABEL_PROJECTION), as the Motif popup does */
void mbvq_c_projection_label(size_t instance);
/* projection: 0 geographic, 1 UTM, 2 spheroid */
void mbvq_c_setprojection(size_t instance, int projection);
/* annotation: 0 decimal degrees, 1 degrees and minutes */
void mbvq_c_setlonlatstyle(size_t instance, int style);

/* ---- 3D soundings window (mb3dsoundings) ---- */

/* edit modes (MBS_EDIT_*) and mouse modes (MBS_MOUSE_*) */
#define MBVQ_SDG_EDIT_TOGGLE 0
#define MBVQ_SDG_EDIT_PICK 1
#define MBVQ_SDG_EDIT_ERASE 2
#define MBVQ_SDG_EDIT_RESTORE 3
#define MBVQ_SDG_EDIT_GRAB 4
#define MBVQ_SDG_EDIT_INFO 5
#define MBVQ_SDG_EDIT_MODES 6
#define MBVQ_SDG_MOUSE_ROTATE 0
#define MBVQ_SDG_MOUSE_PANZOOM 1

/* the size of the soundings drawing in physical pixels (gl_width, gl_height) */
void mbvq_c_soundings_resize(int width, int height);
/* draw the soundings, inside a paint */
void mbvq_c_soundings_plot(void);
/* the edit mode radio buttons, the mouse mode buttons and menu, Reset View */
void mbvq_c_soundings_edit_mode(int mode);
void mbvq_c_soundings_mouse_mode(int mode);
void mbvq_c_soundings_resetview(void);
/* the window was dismissed: hide it and tell the calling program */
void mbvq_c_soundings_dismiss(void);
void mbvq_c_soundings_modes(int *edit_mode, int *mouse_mode);

/* ---- implemented by the Qt half ---- */

/* true if the backend draws (the legacy OpenGL backend), false for the null backend */
bool mbvq_qt_drawable(void);
/* create and show the window of an instance (from mbview_open) */
void mbvq_qt_open_window(size_t instance);
/* close the window of an instance without notifying the program */
void mbvq_qt_close_window(size_t instance);
/* status line, message_on is handled by the C half */
void mbvq_qt_status(size_t instance, const char *message);
/* a label in Motif "BX" multiline form (:::t"line":t"line"...) */
void mbvq_qt_label(size_t instance, int label, const char *bxstring);
/* a message while the program works (do_mbview_message_on/off) */
void mbvq_qt_message(size_t instance, const char *message, bool on);
/* the mouse mode or a view toggle of an instance changed */
void mbvq_qt_modes_changed(size_t instance);
/* the sensitivity of menu items and actions may have changed */
void mbvq_qt_sensitivity_changed(size_t instance);
/* the rows of a list changed (MBVQ_LIST_*) */
void mbvq_qt_list_changed(int list);
/* all list windows close (the last instance closed) */
void mbvq_qt_lists_closed(void);
void mbvq_qt_bell(void);
void mbvq_qt_cursor(size_t instance, int cursor);
/* view values changed by dragging or rounding (MBV_VALUES_*), or all values
   of the 2D or 3D view dialog (MBVQ_VALUES_ALL_2D, MBVQ_VALUES_ALL_3D) */
#define MBVQ_VALUES_ALL_2D 100
#define MBVQ_VALUES_ALL_3D 101
void mbvq_qt_values_changed(size_t instance, int values);
/* Action menu: an item (index in the order added) or a separator was added */
void mbvq_qt_add_action(size_t instance, int index, const char *label);
void mbvq_qt_add_action_separator(size_t instance);
void mbvq_qt_action_enabled(size_t instance, int index, bool enabled);
/* run mbvq_c_work(instance) after delay_ms milliseconds */
void mbvq_qt_schedule_work(size_t instance, int delay_ms);
/* the soundings window: create (hidden), show, hide, destroy */
void mbvq_qt_soundings_create(void);
void mbvq_qt_soundings_show(void);
void mbvq_qt_soundings_hide(void);
void mbvq_qt_soundings_destroy(void);
/* the window shows the state of the core again (modes, toggles) */
void mbvq_qt_soundings_refresh(void);
/* the mouse mode label (Motif "BX" form) and the status line; the bias
   sliders are hidden while the status shows sounding information */
void mbvq_qt_soundings_mousemode(const char *bxstring);
void mbvq_qt_soundings_status(const char *text, bool sliders);
void mbvq_qt_soundings_cursor(int cursor);

#ifdef __cplusplus
}
#endif

#endif /* MBVIEW_QT_MBVQ_BRIDGE_H */
