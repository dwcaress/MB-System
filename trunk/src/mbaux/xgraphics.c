/*--------------------------------------------------------------------
 *    The MB-system:	xgraphics.c	8/3/94
 *    $Id: xgraphics.c,v 5.5 2007-10-08 05:48:26 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 1999, 2000 by
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
 * The xgraphics library supports simple 8-bit X Window graphics for
 * interactive graphical tools.  This code is based on an earlier
 * library which explicitely controlled the colormap to allow 
 * double overlays.  This implementation uses only colors defined
 * by the calling program and allows line drawing in two styles:
 * solid and dashed.
 *
 * Author:	D. W. Caress
 * Date:	August 3, 1994
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.4  2006/01/24 19:17:13  caress
 * Version 5.0.8 beta.
 *
 * Revision 5.3  2006/01/11 07:33:01  caress
 * Working towards 5.0.8
 *
 * Revision 5.2  2003/04/17 20:45:10  caress
 * Release 5.0.beta30
 *
 * Revision 5.1  2003/03/10 19:56:16  caress
 * Expanded the libraries ability to deal with various screen types.
 *
 * Revision 5.0  2000/12/01 22:53:59  caress
 * First cut at Version 5.0.
 *
 * Revision 4.6  2000/10/11  00:54:20  caress
 * Converted to ANSI C
 *
 * Revision 4.5  2000/09/30  06:54:58  caress
 * Snapshot for Dale.
 *
 * Revision 4.4  1999/12/29  00:59:34  caress
 * Release 4.6.8
 *
 * Revision 4.3  1998/10/05  17:45:32  caress
 * MB-System version 4.6beta
 *
 * Revision 4.2  1997/04/21  16:56:14  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.2  1997/04/16  21:29:30  caress
 * Complete rewrite without uid file.
 *
 * Revision 4.1  1994/12/28  14:46:05  caress
 * Added support for TrueColor displays as per Peter Lemmond's mods.
 *
 * Revision 4.1  1994/12/28  14:46:05  caress
 * Added support for TrueColor displays as per Peter Lemmond's mods.
 *
 * Revision 4.0  1994/10/21  11:55:41  caress
 * Release V4.0
 *
 *
 */
/*--------------------------------------------------------------------*/

/* standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

/* global defines */
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

int xg_init(Display *display, Window can_xid, 
		int *can_bounds, char *fontname);
void xg_free(int xgid);
void xg_drawpoint(int xgid, int x, int y, unsigned int pixel, int style);
void xg_drawline(int xgid, int x1, int y1, int x2, int y2, 
		unsigned int pixel, int style);
void xg_drawrectangle(int xgid, int x, int y, int width, int height, 
		unsigned int pixel, int style);
void xg_drawtriangle(int xgid, 
		int x1, int y1, int x2, int y2, int x3, int y3, 
		unsigned int pixel, int style);
void xg_fillrectangle(int xgid, int x, int y, int width, int height, 
		unsigned int pixel, int style);
void xg_filltriangle(int xgid, 
		int x1, int y1, int x2, int y2, int x3, int y3, 
		unsigned int pixel, int style);
void xg_drawstring(int xgid, int x, int y, char *string, 
		unsigned int pixel, int style);
void xg_justify(int xgid, char *string, 
		int *width, int *ascent, int *descent);
void xg_setclip(int xgid, int x, int y, int width, int height);

/**********************************************************************
 *	XG_INIT
 *	- initializes plotting variables, the colortable, and the GC
 **********************************************************************/
int xg_init(Display *display, Window can_xid, 
		int *can_bounds, char *fontname)
{
static char rcs_id[]="$Id: xgraphics.c,v 5.5 2007-10-08 05:48:26 caress Exp $";
	/* local variables */
	struct xg_graphic *graphic;
	XGCValues gc_val;
	int	i, j;

	/* allocate memory for xg_graphic structure */
	if ((graphic = (struct xg_graphic *) 
		calloc(1,sizeof(struct xg_graphic))) == NULL)
		exit(1);

	/* copy input variables to global variables */
	graphic->dpy = display;
	graphic->xid = can_xid;
	for (i=0;i<4;i++)
		graphic->bounds[i] = can_bounds[i];

	/* check for the type of display and set the display_type */
	graphic->display_depth = DisplayPlanes(graphic->dpy, 
			DefaultScreen(graphic->dpy));
/*fprintf(stderr,"graphic->display_depth:%d Default Visual:%d\n", 
graphic->display_depth,
DefaultVisual(graphic->dpy, DefaultScreen(graphic->dpy)));*/
	if (graphic->display_depth == 1 )
		{
		if (XMatchVisualInfo(graphic->dpy,DefaultScreen(graphic->dpy),
			1,StaticGray,&(graphic->visinfo)) == 0)
			{
			fprintf(stderr,"Error: Could not Match an 1 bit GrayScale plane\n");
			exit(-1);
			}
		graphic->display_type = StaticGray;
		graphic->visual = graphic->visinfo.visual;  
		}
	else if (graphic->display_depth == 8)
		{
		if (XMatchVisualInfo(graphic->dpy, DefaultScreen(graphic->dpy),
			8,PseudoColor,&(graphic->visinfo)) == 0)
			{
			fprintf(stderr,"Error: Could not Match an 8 bit Pseudo-Color plane\n");
			exit(-1);
			}
		graphic->display_type = PseudoColor;
		graphic->visual = graphic->visinfo.visual;  
		}
	else if (graphic->display_depth == 16)
		{
		if (XMatchVisualInfo(graphic->dpy, DefaultScreen(graphic->dpy),
			16,TrueColor,&(graphic->visinfo)) != 0)
			{
			graphic->display_type = TrueColor;
			graphic->visual = graphic->visinfo.visual;  
			}
		else if (XMatchVisualInfo(graphic->dpy, DefaultScreen(graphic->dpy),
			16,PseudoColor,&(graphic->visinfo)) != 0)
			{
			graphic->display_type = PseudoColor;
			graphic->visual = graphic->visinfo.visual;  
			}
		else
			{
			fprintf(stderr,"Error: Could not Match a 16 bit TrueColor or Pseudocolor plane\n");
			exit(-1);
			}
		}
	else if (graphic->display_depth == 24)
		{
		if (XMatchVisualInfo(graphic->dpy, DefaultScreen(graphic->dpy),
			24,TrueColor,&(graphic->visinfo)) == 0)
			{
			fprintf(stderr,"Error: Could not Match a 24 bit TrueColor plane\n");
			exit(-1);
			}
		graphic->display_type = TrueColor;
		graphic->visual = graphic->visinfo.visual;  
		}
	else
		{
		graphic->visual = DefaultVisual(graphic->dpy, DefaultScreen(graphic->dpy));
		graphic->display_type = 0;
		}

	/* set foreground and background colors */
	if (graphic->display_type == StaticGray 
		|| graphic->display_type == PseudoColor
	        || graphic->display_type == TrueColor)
		{
		graphic->bg_pixel = WhitePixel(graphic->dpy, 
			DefaultScreen(graphic->dpy));
		graphic->fg_pixel = BlackPixel(graphic->dpy, 
			DefaultScreen(graphic->dpy));
		}
	else
		{
		fprintf(stderr, "Error: Could not Match a one, eight, or twentyfour bit plane\n");
		exit(-1);
		}

	/* load font */
	if ((graphic->font_info = XLoadQueryFont(graphic->dpy, fontname)) 
		== NULL)
		{
		printf("X Error: Cannot load font: %s\n",fontname);
		exit(-1);
		}

	/* set up graphics context */
	gc_val.foreground = graphic->fg_pixel;
	gc_val.background = graphic->bg_pixel;
	gc_val.font = graphic->font_info->fid;

	/* set gc with solid lines */
	gc_val.plane_mask = AllPlanes;
	gc_val.line_style = LineSolid;
	graphic->gc_solid = XCreateGC(graphic->dpy, graphic->xid, 
		(GCForeground | GCBackground | GCFont 
		| GCPlaneMask | GCLineStyle),
		&(gc_val));

	/* set gc with dash lines */
	gc_val.line_style = LineOnOffDash;
	graphic->gc_dash = XCreateGC(graphic->dpy, graphic->xid, 
		(GCForeground | GCBackground | GCFont 
		| GCPlaneMask | GCLineStyle),
		&(gc_val));

	/* return pointer to xg_graphic structure */
	return((int) graphic);
}
/**********************************************************************
 *	XG_FREE
 *	- deallocates xg_graphic structure when no longer needed
 **********************************************************************/
void xg_free(int xgid)
{
	struct xg_graphic *graphic;

	graphic = (struct xg_graphic *) xgid;
	free(graphic);
}
/**********************************************************************
 *	XG_DRAWPOINT
 *	- draws a pixel
 **********************************************************************/
void xg_drawpoint(int xgid, int x, int y, unsigned int pixel, int style)
{
	struct xg_graphic *graphic;
	GC *gc;

	graphic = (struct xg_graphic *) xgid;
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
void xg_drawline(int xgid, int x1, int y1, int x2, int y2, 
		unsigned int pixel, int style)
{
	struct xg_graphic *graphic;
	GC *gc;

	graphic = (struct xg_graphic *) xgid;
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
void xg_drawrectangle(int xgid, int x, int y, int width, int height, 
		unsigned int pixel, int style)
{
	struct xg_graphic *graphic;
	GC *gc;

	graphic = (struct xg_graphic *) xgid;
	if (style == XG_SOLIDLINE)
		gc = &graphic->gc_solid;
	else
		gc = &graphic->gc_dash;
	XSetForeground(graphic->dpy, *gc, pixel);
	XDrawRectangle(graphic->dpy, graphic->xid, *gc, 
		x, y, width, height);
}
/**********************************************************************
 *	XG_DRAWTRIANGLE
 *	- draws a triangle outline
 **********************************************************************/
void xg_drawtriangle(int xgid, 
		int x1, int y1, int x2, int y2, int x3, int y3, 
		unsigned int pixel, int style)
{
	struct xg_graphic *graphic;
	GC *gc;
	XSegment segments[3];
	int nsegments = 3;

	graphic = (struct xg_graphic *) xgid;
	if (style == XG_SOLIDLINE)
		gc = &graphic->gc_solid;
	else
		gc = &graphic->gc_dash;
	segments[0].x1 = (short) x1;
	segments[0].y1 = (short) y1;
	segments[0].x2 = (short) x2;
	segments[0].y2 = (short) y2;
	segments[1].x1 = (short) x2;
	segments[1].y1 = (short) y2;
	segments[1].x2 = (short) x3;
	segments[1].y2 = (short) y3;
	segments[2].x1 = (short) x3;
	segments[2].y1 = (short) y3;
	segments[2].x2 = (short) x1;
	segments[2].y2 = (short) y1;
	XSetForeground(graphic->dpy, *gc, pixel);
	XDrawSegments(graphic->dpy, graphic->xid, *gc, 
		segments, nsegments);
}
/**********************************************************************
 *	XG_FILLRECTANGLE
 *	- fills a rectangle
 **********************************************************************/
void xg_fillrectangle(int xgid, int x, int y, int width, int height, 
		unsigned int pixel, int style)
{
	struct xg_graphic *graphic;
	GC *gc;

	graphic = (struct xg_graphic *) xgid;
	if (style == XG_SOLIDLINE)
		gc = &graphic->gc_solid;
	else
		gc = &graphic->gc_dash;
	XSetForeground(graphic->dpy, *gc, pixel);
	XFillRectangle(graphic->dpy, graphic->xid, *gc, 
		x, y, width, height);
}
/**********************************************************************
 *	XG_FILLTRIANGLE
 *	- fills a triangle
 **********************************************************************/
void xg_filltriangle(int xgid, 
		int x1, int y1, int x2, int y2, int x3, int y3, 
		unsigned int pixel, int style)
{
	struct xg_graphic *graphic;
	GC *gc;
	XPoint points[3];
	int npoints = 3;
	int shape = Convex;
	int mode = CoordModeOrigin;

	graphic = (struct xg_graphic *) xgid;
	if (style == XG_SOLIDLINE)
		gc = &graphic->gc_solid;
	else
		gc = &graphic->gc_dash;
	points[0].x = (short) x1;
	points[0].y = (short) y1;
	points[1].x = (short) x2;
	points[1].y = (short) y2;
	points[2].x = (short) x3;
	points[2].y = (short) y3;
	XSetForeground(graphic->dpy, *gc, pixel);
	XFillPolygon(graphic->dpy, graphic->xid, *gc, 
		points, npoints, shape, mode);
}
/**********************************************************************
 *	XG_DRAWSTRING
 *	- draws a string
 **********************************************************************/
void xg_drawstring(int xgid, int x, int y, char *string, 
		unsigned int pixel, int style)
{
	struct xg_graphic *graphic;
	GC *gc;
	int	string_length;

	graphic = (struct xg_graphic *) xgid;
	if (style == XG_SOLIDLINE)
		gc = &graphic->gc_solid;
	else
		gc = &graphic->gc_dash;
	XSetForeground(graphic->dpy, *gc, pixel);
	string_length = strlen(string);
	XDrawString(graphic->dpy, graphic->xid, *gc, 
		x, y, string, string_length);
}
/**********************************************************************
 *	XG_JUSTIFY
 *	- figures out the dimensions of a string when drawn
 **********************************************************************/
void xg_justify(int xgid, char *string, 
		int *width, int *ascent, int *descent)
{
	struct xg_graphic *graphic;
	int	string_length;
	XCharStruct	string_info;
	int	direction;
	int	lascent;
	int	ldescent;

	graphic = (struct xg_graphic *) xgid;
	string_length = strlen(string);
	XTextExtents(graphic->font_info, string, string_length, 
		&direction, &lascent, &ldescent, &string_info);
	*width = string_info.width;
	*ascent = string_info.ascent;
	*descent = string_info.descent;
}
/**********************************************************************
 *	XG_SETCLIP
 *	- sets clipping mask for all gc's
 **********************************************************************/
void xg_setclip(int xgid, int x, int y, int width, int height)
{
	struct xg_graphic *graphic;
	XRectangle rectangle[1];
	int	i;

	/* set up rectangle */
	rectangle[0].x = x;
	rectangle[0].y = y;
	rectangle[0].width = width;
	rectangle[0].height = height;

	/* set clip rectangle */
	graphic = (struct xg_graphic *) xgid;
	XSetClipRectangles(graphic->dpy, graphic->gc_solid, 0, 0, 
		rectangle, 1, Unsorted);
	XSetClipRectangles(graphic->dpy, graphic->gc_dash, 0, 0, 
		rectangle, 1, Unsorted);
}
