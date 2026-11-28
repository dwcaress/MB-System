/*--------------------------------------------------------------------
 *    The MB-system:  mbv_input.h  11/7/2026
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
 * Toolkit-neutral mouse and keyboard input for the mbview drawing areas.
 *
 * The map view (mbview_input) and the 3D soundings view
 * (mb3dsoundings_input) handle all mouse modes and keys. A toolkit only
 * translates its own events into struct mbv_input_event and passes every
 * event on, whatever its type. The Motif callbacks do_mbview_glwda_input()
 * and do_mb3dsdg_glwda_input() translate X events with
 * mbv_input_from_xevent().
 */

#ifndef MBVIEW_MBV_INPUT_H
#define MBVIEW_MBV_INPUT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* event types */
#define MBV_INPUT_OTHER 0      /* any other event */
#define MBV_INPUT_PRESS 1      /* mouse button pressed */
#define MBV_INPUT_MOTION 2     /* mouse moved with a button down */
#define MBV_INPUT_RELEASE 3    /* mouse button released */
#define MBV_INPUT_KEYPRESS 4   /* key pressed */
#define MBV_INPUT_KEYRELEASE 5 /* key released */
#define MBV_INPUT_EXPOSE 6     /* part of the drawing area needs redrawing */

struct mbv_input_event {
	int type;   /* MBV_INPUT_* */
	int button; /* press and release: 1 left, 2 middle, 3 right */
	int x;      /* pixels from the left edge of the drawing area */
	int y;      /* pixels from the top edge of the drawing area */
	char key;   /* key events: the character of the key, 0 if it has none */
};

/* handle an event in the map view of an mbview instance */
void mbview_input(size_t instance, const struct mbv_input_event *event);

/* handle an event in the 3D soundings view */
void mb3dsoundings_input(const struct mbv_input_event *event);

#ifndef MBVIEW_NO_MOTIF
/* translate an X event (Motif programs; mbv_ui_motif.c) */
void mbv_input_from_xevent(XEvent *xevent, struct mbv_input_event *event);
#endif

#ifdef __cplusplus
}
#endif

#endif /* MBVIEW_MBV_INPUT_H */
