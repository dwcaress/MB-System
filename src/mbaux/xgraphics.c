/*--------------------------------------------------------------------
 *    The MB-system:	xgraphics.c	8/3/94
 *    $Id: xgraphics.c,v 4.0 1994-10-21 11:55:41 caress Exp $
 *
 *    Copyright (c) 1993, 1994 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
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
 *
 */
/*--------------------------------------------------------------------*/

/* standard includes */
#include <stdio.h>
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
	unsigned long	bg_pixel;	/* background color */
	unsigned long	fg_pixel;	/* foreground color */
	GC		gc_solid;	/* Xwindows graphics context */
	GC		gc_dash;	/* Xwindows graphics context */
	XFontStruct	*font_info;	/* XFontStruct pointer */
	};

/**********************************************************************
 *	XG_INIT
 *	- initializes plotting variables, the colortable, and the GC
 **********************************************************************/
int xg_init(display,can_xid,can_bounds,fontname)
Display	*display;
Window	can_xid;
int	*can_bounds;
char	*fontname;
{
static char rcs_id[]="$Id: xgraphics.c,v 4.0 1994-10-21 11:55:41 caress Exp $";
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
	else
		{
		fprintf(stderr, "Error: Could not Match an eight bit or one bit plane\n");
		exit(-1);
		}

	/* set foreground and background colors */
	if (graphic->display_type == StaticGray 
		|| graphic->display_type == PseudoColor)
		{
		graphic->bg_pixel = WhitePixel(graphic->dpy, 
			DefaultScreen(graphic->dpy));
		graphic->fg_pixel = BlackPixel(graphic->dpy, 
			DefaultScreen(graphic->dpy));
		}
	else
		{
		fprintf(stderr, "Error: Could not Match an eight bit or one bit plane\n");
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
void xg_free(xgid)
int	xgid;
{
	struct xg_graphic *graphic;

	graphic = (struct xg_graphic *) xgid;
	free(graphic);
}
/**********************************************************************
 *	XG_DRAWPOINT
 *	- draws a pixel
 **********************************************************************/
void xg_drawpoint(xgid, x, y, pixel, style)
int	xgid;
int	x, y;
unsigned long	pixel;
int	style;
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
void xg_drawline(xgid, x1, y1, x2, y2, pixel, style)
int	xgid;
int	x1, y1, x2, y2;
unsigned long	pixel;
int	style;
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
void xg_drawrectangle(xgid, x, y, width, height, pixel, style)
int	xgid;
int	x, y, width, height;
unsigned long	pixel;
int	style;
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
void xg_drawtriangle(xgid, x1, y1, x2, y2, x3, y3, pixel, style)
int	xgid;
int	x1, y1, x2, y2, x3, y3;
unsigned long	pixel;
int	style;
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
void xg_fillrectangle(xgid, x, y, width, height, pixel, style)
int	xgid;
int	x, y, width, height;
unsigned long	pixel;
int	style;
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
void xg_filltriangle(xgid, x1, y1, x2, y2, x3, y3, pixel, style)
int	xgid;
int	x1, y1, x2, y2, x3, y3;
unsigned long	pixel;
int	style;
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
void xg_drawstring(xgid, x, y, string, pixel, style)
int	xgid;
int	x, y;
char	*string;
unsigned long	pixel;
int	style;
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
void xg_justify(xgid, string, width, ascent, descent)
int	xgid;
char	*string;
int	*width;
int	*ascent;
int	*descent;
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
void xg_setclip(xgid, x, y, width, height)
int	xgid;
int	x, y, width, height;
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
