/*--------------------------------------------------------------------
 *    The MB-system:	mb_aux.h	10/13/2009
 *    $Id$
 *
 *    Copyright (c) 2009-2013 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mb_xgraphics.h defines data structures used by mb_xgraphics.c.
 *
 * Author:	D. W. Caress
 * Date:	October 13, 2009
 *
 * $Log: $
 *
 */

/* xgraphics defines */
#define	XG_SOLIDLINE	0
#define	XG_DASHLINE	1

/* xg_graphic structure */
struct xg_graphic
	{
	Display		*dpy;		/* Xwindows display/screen number */
	Window		xid;		/* Xwindows window id for drawable */
	int		bounds[4];	/* Drawable borders */
	int		display_type;	/* Monochrome or 8-bit color */
	int		display_depth;	/* 1-bit or 8-bit */
	Visual		*visual;	/* We get it but we don't use it */
	XVisualInfo	visinfo;	/* We get it but we don't use it */
	unsigned int	bg_pixel;	/* background color */
	unsigned int	fg_pixel;	/* foreground color */
	GC		gc_solid;	/* Xwindows graphics context */
	GC		gc_dash;	/* Xwindows graphics context */
	XFontStruct	*font_info;	/* XFontStruct pointer */
	};

/* xgraphics function prototypes */
void xg_init(Display *display, Window can_xid,
		int *can_bounds, char *fontname, void **xgid);
void xg_free(void *xgid);
void xg_drawpoint(void *xgid, int x, int y, unsigned int pixel, int style);
void xg_drawline(void *xgid, int x1, int y1, int x2, int y2,
		unsigned int pixel, int style);
void xg_drawrectangle(void *xgid, int x, int y, int width, int height,
		unsigned int pixel, int style);
void xg_drawtriangle(void *xgid,
		int x1, int y1, int x2, int y2, int x3, int y3,
		unsigned int pixel, int style);
void xg_fillrectangle(void *xgid, int x, int y, int width, int height,
		unsigned int pixel, int style);
void xg_filltriangle(void *xgid,
		int x1, int y1, int x2, int y2, int x3, int y3,
		unsigned int pixel, int style);
void xg_drawstring(void *xgid, int x, int y, char *string,
		unsigned int pixel, int style);
void xg_justify(void *xgid, char *string,
		int *width, int *ascent, int *descent);
void xg_setclip(void *xgid, int x, int y, int width, int height);
