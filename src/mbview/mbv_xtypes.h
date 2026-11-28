/*--------------------------------------------------------------------
 *    The MB-system:  mbv_xtypes.h  11/7/2026
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
 * Stand-in X11, Xt, Motif and GLX types for building the mbview core
 * without Motif (MBVIEW_NO_MOTIF, used by the Qt6 programs).
 *
 * The mbview structures keep their Motif fields (widgets, cursors, GLX
 * contexts) so that the Motif and Qt builds share one set of headers.
 * Without Motif those fields are only stored, never used, so opaque
 * stand-ins are enough. The Motif build never includes this file.
 */

#ifndef MBVIEW_MBV_XTYPES_H
#define MBVIEW_MBV_XTYPES_H

#ifndef MBVIEW_NO_MOTIF
#error "mbv_xtypes.h is only for builds without Motif (MBVIEW_NO_MOTIF)"
#endif

/* X11. Display uses the Xlib structure name, because Qt on Linux (with its
   X11 support) declares "typedef struct _XDisplay Display;" in
   qguiapplication_platform.h; the same declaration twice is allowed in C++. */
typedef struct _XDisplay Display;
typedef unsigned long Window;
typedef unsigned long Cursor;
typedef struct mbv_x_visualinfo XVisualInfo;
typedef union mbv_x_event XEvent;

/* Xt */
typedef struct mbv_xt_widget *Widget;
typedef struct mbv_xt_appcontext *XtAppContext;
typedef void *XtPointer;
typedef char *String;
typedef char Boolean;
typedef unsigned int Cardinal;
typedef unsigned short Dimension;
typedef struct {
	String name;
	long value;
} Arg, *ArgList;

/* GLX */
typedef struct mbv_glx_context *GLXContext;

#endif /* MBVIEW_MBV_XTYPES_H */
