/*--------------------------------------------------------------------
 *    The MB-system:	xgraphics.c	8/3/94
 *
 *    Copyright (c) 1993-2023 by
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
 * The xgraphics library supports simple 8-bit X Window graphics for
 * interactive graphical tools.  This code is based on an earlier
 * library which explicitly controlled the colormap to allow
 * double overlays.  This implementation uses only colors defined
 * by the calling program and allows line drawing in two styles:
 * solid and dashed.
 *
 * Author:	D. W. Caress
 * Date:	August 3, 1994
 *
 *
 */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "mb_xgraphics.h"

/**********************************************************************
 *	XG_INIT
 *	- initializes plotting variables, the colortable, and the GC
 **********************************************************************/
void xg_init(Display *display, Window can_xid, int *can_bounds, char *fontname, void **xgid) {
	/* allocate memory for xg_graphic structure */
	struct xg_graphic *graphic = (struct xg_graphic *)calloc(1, sizeof(struct xg_graphic));
	if (graphic == NULL)
		exit(1);

	/* copy input variables to global variables */
	graphic->dpy = display;
	graphic->xid = can_xid;
	for (int i = 0; i < 4; i++)
		graphic->bounds[i] = can_bounds[i];

	/* check for the type of display and set the display_type */
	graphic->display_depth = DisplayPlanes(graphic->dpy, DefaultScreen(graphic->dpy));
	/*fprintf(stderr,"graphic->display_depth:%d Default Visual:%d\n",
	graphic->display_depth,
	DefaultVisual(graphic->dpy, DefaultScreen(graphic->dpy)));*/
	if (graphic->display_depth == 1) {
		if (XMatchVisualInfo(graphic->dpy, DefaultScreen(graphic->dpy), 1, StaticGray, &(graphic->visinfo)) == 0) {
			fprintf(stderr, "Error: Could not Match an 1 bit GrayScale plane\n");
			exit(-1);
		}
		graphic->display_type = StaticGray;
		graphic->visual = graphic->visinfo.visual;
	}
	else if (graphic->display_depth == 8) {
		if (XMatchVisualInfo(graphic->dpy, DefaultScreen(graphic->dpy), 8, PseudoColor, &(graphic->visinfo)) == 0) {
			fprintf(stderr, "Error: Could not Match an 8 bit Pseudo-Color plane\n");
			exit(-1);
		}
		graphic->display_type = PseudoColor;
		graphic->visual = graphic->visinfo.visual;
	}
	else if (graphic->display_depth == 16) {
		if (XMatchVisualInfo(graphic->dpy, DefaultScreen(graphic->dpy), 16, TrueColor, &(graphic->visinfo)) != 0) {
			graphic->display_type = TrueColor;
			graphic->visual = graphic->visinfo.visual;
		}
		else if (XMatchVisualInfo(graphic->dpy, DefaultScreen(graphic->dpy), 16, PseudoColor, &(graphic->visinfo)) != 0) {
			graphic->display_type = PseudoColor;
			graphic->visual = graphic->visinfo.visual;
		}
		else {
			fprintf(stderr, "Error: Could not Match a 16 bit TrueColor or Pseudocolor plane\n");
			exit(-1);
		}
	}
	else if (graphic->display_depth == 24) {
		if (XMatchVisualInfo(graphic->dpy, DefaultScreen(graphic->dpy), 24, TrueColor, &(graphic->visinfo)) == 0) {
			fprintf(stderr, "Error: Could not Match a 24 bit TrueColor plane\n");
			exit(-1);
		}
		graphic->display_type = TrueColor;
		graphic->visual = graphic->visinfo.visual;
	}
	else {
		graphic->visual = DefaultVisual(graphic->dpy, DefaultScreen(graphic->dpy));
		graphic->display_type = 0;
	}

	/* set foreground and background colors */
	if (graphic->display_type == StaticGray || graphic->display_type == PseudoColor || graphic->display_type == TrueColor) {
		graphic->bg_pixel = WhitePixel(graphic->dpy, DefaultScreen(graphic->dpy));
		graphic->fg_pixel = BlackPixel(graphic->dpy, DefaultScreen(graphic->dpy));
	}
	else {
		fprintf(stderr, "Error: Could not Match a one, eight, or twentyfour bit plane\n");
		exit(-1);
	}

	/* load font */
	if ((graphic->font_info = XLoadQueryFont(graphic->dpy, fontname)) == NULL) {
		fprintf(stderr, "\nFailure to load font using XLoadQueryFont: %s\n", fontname);
		fprintf(stderr, "\tSource file: %s\n\tSource line: %d", __FILE__, __LINE__);
		fprintf(stderr, "Program Terminated\n");
		exit(-1);
	}

	/* set up graphics context */
	XGCValues gc_val;
	gc_val.foreground = graphic->fg_pixel;
	gc_val.background = graphic->bg_pixel;
	gc_val.font = graphic->font_info->fid;

	/* set gc with solid lines */
	gc_val.plane_mask = AllPlanes;
	gc_val.line_style = LineSolid;
	graphic->gc_solid =
	    XCreateGC(graphic->dpy, graphic->xid, (GCForeground | GCBackground | GCFont | GCPlaneMask | GCLineStyle), &(gc_val));

	/* set gc with dash lines */
	gc_val.line_style = LineOnOffDash;
	graphic->gc_dash =
	    XCreateGC(graphic->dpy, graphic->xid, (GCForeground | GCBackground | GCFont | GCPlaneMask | GCLineStyle), &(gc_val));

	/* return pointer to xg_graphic structure */
	*xgid = (void *)graphic;
}
/**********************************************************************
 *	XG_FREE
 *	- deallocates xg_graphic structure when no longer needed
 **********************************************************************/
void xg_free(void *xgid) {
	struct xg_graphic *graphic = (struct xg_graphic *)xgid;
	free(graphic);
}
/**********************************************************************
 *	XG_DRAWPOINT
 *	- draws a pixel
 **********************************************************************/
void xg_drawpoint(void *xgid, int x, int y, unsigned int pixel, int style) {
	struct xg_graphic *graphic = (struct xg_graphic *)xgid;
	GC *gc;
	if (style == XG_SOLIDLINE)
		gc = &graphic->gc_solid;
	else
		gc = &graphic->gc_dash;
	XSetForeground(graphic->dpy, *gc, pixel);
	XDrawPoint(graphic->dpy, graphic->xid, *gc, x, y);
}
/**********************************************************************
 *	XG_DRAWLINE
 *	- draws a line
 **********************************************************************/
void xg_drawline(void *xgid, int x1, int y1, int x2, int y2, unsigned int pixel, int style) {
	struct xg_graphic *graphic = (struct xg_graphic *)xgid;
	GC *gc;
	if (style == XG_SOLIDLINE)
		gc = &graphic->gc_solid;
	else
		gc = &graphic->gc_dash;
	XSetForeground(graphic->dpy, *gc, pixel);
	XDrawLine(graphic->dpy, graphic->xid, *gc, x1, y1, x2, y2);
}
/**********************************************************************
 *	XG_DRAWRECTANGLE
 *	- draws a rectangle outline
 **********************************************************************/
void xg_drawrectangle(void *xgid, int x, int y, int width, int height, unsigned int pixel, int style) {
	struct xg_graphic *graphic = (struct xg_graphic *)xgid;
	GC *gc;
	if (style == XG_SOLIDLINE)
		gc = &graphic->gc_solid;
	else
		gc = &graphic->gc_dash;
	XSetForeground(graphic->dpy, *gc, pixel);
	XDrawRectangle(graphic->dpy, graphic->xid, *gc, x, y, width, height);
}
/**********************************************************************
 *	XG_DRAWTRIANGLE
 *	- draws a triangle outline
 **********************************************************************/
void xg_drawtriangle(void *xgid, int x1, int y1, int x2, int y2, int x3, int y3, unsigned int pixel, int style) {
	struct xg_graphic *graphic = (struct xg_graphic *)xgid;
	GC *gc;
	if (style == XG_SOLIDLINE)
		gc = &graphic->gc_solid;
	else
		gc = &graphic->gc_dash;
	XSegment segments[3];
	segments[0].x1 = (short)x1;
	segments[0].y1 = (short)y1;
	segments[0].x2 = (short)x2;
	segments[0].y2 = (short)y2;
	segments[1].x1 = (short)x2;
	segments[1].y1 = (short)y2;
	segments[1].x2 = (short)x3;
	segments[1].y2 = (short)y3;
	segments[2].x1 = (short)x3;
	segments[2].y1 = (short)y3;
	segments[2].x2 = (short)x1;
	segments[2].y2 = (short)y1;
	XSetForeground(graphic->dpy, *gc, pixel);
	const int nsegments = 3;
	XDrawSegments(graphic->dpy, graphic->xid, *gc, segments, nsegments);
}
/**********************************************************************
 *	XG_FILLRECTANGLE
 *	- fills a rectangle
 **********************************************************************/
void xg_fillrectangle(void *xgid, int x, int y, int width, int height, unsigned int pixel, int style) {
	struct xg_graphic *graphic = (struct xg_graphic *)xgid;
	GC *gc;
	if (style == XG_SOLIDLINE)
		gc = &graphic->gc_solid;
	else
		gc = &graphic->gc_dash;
	XSetForeground(graphic->dpy, *gc, pixel);
	XFillRectangle(graphic->dpy, graphic->xid, *gc, x, y, width, height);
}
/**********************************************************************
 *	XG_FILLTRIANGLE
 *	- fills a triangle
 **********************************************************************/
void xg_filltriangle(void *xgid, int x1, int y1, int x2, int y2, int x3, int y3, unsigned int pixel, int style) {
	struct xg_graphic *graphic = (struct xg_graphic *)xgid;
	GC *gc;
	if (style == XG_SOLIDLINE)
		gc = &graphic->gc_solid;
	else
		gc = &graphic->gc_dash;
	XPoint points[3];
	points[0].x = (short)x1;
	points[0].y = (short)y1;
	points[1].x = (short)x2;
	points[1].y = (short)y2;
	points[2].x = (short)x3;
	points[2].y = (short)y3;
	XSetForeground(graphic->dpy, *gc, pixel);
	const int npoints = 3;
	const int shape = Convex;
	const int mode = CoordModeOrigin;
	XFillPolygon(graphic->dpy, graphic->xid, *gc, points, npoints, shape, mode);
}
/**********************************************************************
 *	XG_DRAWSTRING
 *	- draws a string
 **********************************************************************/
void xg_drawstring(void *xgid, int x, int y, char *string, unsigned int pixel, int style) {

	struct xg_graphic *graphic = (struct xg_graphic *)xgid;
	GC *gc;
	if (style == XG_SOLIDLINE)
		gc = &graphic->gc_solid;
	else
		gc = &graphic->gc_dash;
	XSetForeground(graphic->dpy, *gc, pixel);
	const int string_length = strlen(string);
	XDrawString(graphic->dpy, graphic->xid, *gc, x, y, string, string_length);
}
/**********************************************************************
 *	XG_JUSTIFY
 *	- figures out the dimensions of a string when drawn
 **********************************************************************/
void xg_justify(void *xgid, char *string, int *width, int *ascent, int *descent) {

	struct xg_graphic *graphic = (struct xg_graphic *)xgid;
	int direction;
	int lascent;
	int ldescent;
	const int string_length = strlen(string);
	XCharStruct string_info;
	XTextExtents(graphic->font_info, string, string_length, &direction, &lascent, &ldescent, &string_info);
	*width = string_info.width;
	*ascent = string_info.ascent;
	*descent = string_info.descent;
}
/**********************************************************************
 *	XG_SETCLIP
 *	- sets clipping mask for all gc's
 **********************************************************************/
void xg_setclip(void *xgid, int x, int y, int width, int height) {
	/* set up rectangle */
	XRectangle rectangle[1] = {{x, y, width, height}};

	/* set clip rectangle */
	struct xg_graphic *graphic = (struct xg_graphic *)xgid;
	XSetClipRectangles(graphic->dpy, graphic->gc_solid, 0, 0, rectangle, 1, Unsorted);
	XSetClipRectangles(graphic->dpy, graphic->gc_dash, 0, 0, rectangle, 1, Unsorted);
}
