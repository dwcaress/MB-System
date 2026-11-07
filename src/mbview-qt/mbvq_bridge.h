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
#define MBVQ_LABEL_OTHER 3

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
/* the dismiss callback of the program, as the Motif dismiss button calls it */
void mbvq_c_dismiss(size_t instance);

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
/* the mouse mode or a view toggle of an instance changed */
void mbvq_qt_modes_changed(size_t instance);
/* the sensitivity of menu items and actions may have changed */
void mbvq_qt_sensitivity_changed(size_t instance);
/* the site, route, navigation or GIS lists changed */
void mbvq_qt_lists_changed(void);
void mbvq_qt_bell(void);
void mbvq_qt_cursor(size_t instance, int cursor);
/* view values changed by dragging (MBV_VALUES_*) */
void mbvq_qt_values_changed(size_t instance, int values);
/* run mbvq_c_work(instance) after delay_ms milliseconds */
void mbvq_qt_schedule_work(size_t instance, int delay_ms);

#ifdef __cplusplus
}
#endif

#endif /* MBVIEW_QT_MBVQ_BRIDGE_H */
