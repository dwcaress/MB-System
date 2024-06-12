/*--------------------------------------------------------------------
 *    The MB-system:	mb_aux.h	10/13/2009
 *
 *    Copyright (c) 2009-2024 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mb_xgraphics.h defines data structures used by mb_xgraphics.c.
 *
 * Author:	D. W. Caress
 * Date:	October 13, 2009
 */

#ifndef MB_XGRAPHICS_H_
#define MB_XGRAPHICS_H_

#include <stdbool.h>
#include "mb_color.h"

/* xgraphics defines */
#define XG_SOLIDLINE 0
#define XG_DASHLINE 1

/* xg_graphic structure */
struct xg_graphic {
	Display *dpy;           /* Xwindows display/screen number */
	Window xid;             /* Xwindows window id for drawable */
	int bounds[4];          /* Drawable borders */
	int display_type;       /* Monochrome or 8-bit color */
	int display_depth;      /* 1-bit or 8-bit */
	Visual *visual;         /* We get it but we don't use it */
	XVisualInfo visinfo;    /* We get it but we don't use it */
	unsigned int bg_pixel;  /* background color */
	unsigned int fg_pixel;  /* foreground color */
	GC gc_solid;            /* Xwindows graphics context */
	GC gc_dash;             /* Xwindows graphics context */
	XFontStruct *font_info; /* XFontStruct pointer */
};


/* xgraphics function prototypes */
void xg_init(Display *display, Window can_xid, int *can_bounds, char *fontname, void **xgid);
void xg_free(void *xgid);
void xg_drawpoint(void *xgid, int x, int y, unsigned int pixel, int style);
void xg_drawline(void *xgid, int x1, int y1, int x2, int y2, unsigned int pixel, int style);
void xg_drawrectangle(void *xgid, int x, int y, int width, int height, unsigned int pixel, int style);
void xg_drawtriangle(void *xgid, int x1, int y1, int x2, int y2, int x3, int y3, unsigned int pixel, int style);
void xg_fillrectangle(void *xgid, int x, int y, int width, int height, unsigned int pixel, int style);
void xg_filltriangle(void *xgid, int x1, int y1, int x2, int y2, int x3, int y3, unsigned int pixel, int style);
void xg_drawstring(void *xgid, int x, int y, char *string, unsigned int pixel, int style);
void xg_justify(void *xgid, char *string, int *width, int *ascent, int *descent);
void xg_setclip(void *xgid, int x, int y, int width, int height);


/// Allocate standard drawing colors for mb-system C/X11 apps
/// Returns false on error, else true, with colors and nColors set
/// @param[in] display  XWindows Display
/// @param[in] colormap XWindows ColorMap
/// @param[out] colors Allocated colors
/// @param[out] nColors Number of allocated colors
/// @return false if error looking up or allocating a color, else true
bool setDrawingColors(Display *display, Colormap *colormap,
		       XColor **colors, int *nColors);


#endif  /* MB_XGRAPHICS_H_ */
