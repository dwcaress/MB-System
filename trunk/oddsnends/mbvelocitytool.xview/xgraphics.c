/**********************************************************************
 *	XGRAPHICS
 * $Id: xgraphics.c,v 4.2 1994-04-12 01:13:24 caress Exp $
 *	- subroutines for handling 8-bit color in Xwindows applications.
 *	- version 1.0 September 1992
 *	David W. Caress
 *	Lamont-Doherty Geological Observatory
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.0  1994/03/05  23:50:33  caress
 * First cut at version 4.0
 *
 * Revision 4.0  1994/02/27  00:18:47  caress
 * First cut at new version.
 *
 * Revision 4.0  1994/02/27  00:18:47  caress
 * First cut at new version.
 *
 * Revision 1.1  1993/08/16  23:53:16  caress
 * Initial revision
 *
 * Revision 3.1  1993/05/14  23:34:00  sohara
 * fixed rcs_id message
 *
 * Revision 3.0  1993/04/22  18:50:26  dale
 * Initial RCS version.
 *
 **********************************************************************/

/* standard includes */
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

/* global defines */
#define	CLEAR_ALL	0
#define	OVERLAY1_CLEAR	64
#define OVERLAY1_DRAW	65
#define OVERLAY1_DASH	66
#define OVERLAY2_CLEAR	128
#define OVERLAY2_DRAW	129
#define OVERLAY2_DASH	130

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
	Colormap	cmap;		/* Colormap used by drawing window */
	Colormap	cmap_default;	/* Default colormap inherited by 
						window */
	XColor		color[256];	/* Array of color values put into 
						cmap */
	XColor		color_default[256]; /* Array of color values gotten 
						from cmap_default */
	unsigned long	pixel_start;	/* Offset of user specified colors 
						in cmap */
	unsigned long	bg_pixel;	/* background color */
	unsigned long	fg_pixel;	/* foreground color */
	GC		gc[6];		/* Xwindows graphics context */
	XGCValues	gc_val;		/* Values used to define GC */
	XFontStruct	*font_info;	/* XFontStruct pointer */
	};

/**********************************************************************
 *	XG_INIT
 *	- initializes plotting variables, the colortable, and the GC
 **********************************************************************/
int xg_init(display,can_xid,can_bounds,colors,ncolors,fontname)
Display	*display;
Window	can_xid;
int	*can_bounds;
int	*colors;
int	ncolors;
char	*fontname;
{
static char rcs_id[]="$Id: xgraphics.c,v 4.2 1994-04-12 01:13:24 caress Exp $";
	/* local variables */
	struct xg_graphic *graphic;
	void xg_setclip();
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
	graphic->display_depth = DisplayPlanes(graphic->dpy, DefaultScreen(graphic->dpy));
	if (graphic->display_depth == 1 )
		{
		if (XMatchVisualInfo(graphic->dpy,DefaultScreen(graphic->dpy),1,StaticGray,&(graphic->visinfo)) == 0)
			{
			fprintf(stderr,"Error: Could not Match an 1 bit GrayScale plane\n");
			exit(-1);
			}
		graphic->display_type = StaticGray;
		graphic->visual = graphic->visinfo.visual;  
		}
	else if (graphic->display_depth == 8)
		{
		if (XMatchVisualInfo(graphic->dpy, DefaultScreen(graphic->dpy),8,PseudoColor,&(graphic->visinfo)) == 0)
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

	/* set colormap */
	graphic->cmap_default = DefaultColormap(graphic->dpy, DefaultScreen(graphic->dpy));
	if (graphic->display_type == StaticGray)
		{
		/* just use the default colortable */
		graphic->cmap = graphic->cmap_default;
		graphic->bg_pixel = WhitePixel(graphic->dpy, DefaultScreen(graphic->dpy));
		graphic->fg_pixel = BlackPixel(graphic->dpy, DefaultScreen(graphic->dpy));
		}
	else if (graphic->display_type == PseudoColor)
		{
		/* get default colortable values */
		for (i=0; i<256; i++) 
			graphic->color_default[i].pixel = i;
		XQueryColors(graphic->dpy, graphic->cmap_default, graphic->color_default, 256);

		/* allocate new colormap */
		graphic->cmap = XCreateColormap(graphic->dpy,DefaultRootWindow(graphic->dpy),
					graphic->visual,AllocAll);

		/* copy values from default colortable into unused
			space in new colortable */
		if (ncolors > 64)
			ncolors = 64;
		else if (ncolors < 0)
			ncolors = 0;
		graphic->pixel_start = 64 - ncolors;
		for (i=0; i<graphic->pixel_start; i++)
			{
			graphic->color[i] = graphic->color_default[i];
			XStoreColor(graphic->dpy, graphic->cmap, &(graphic->color_default[i]));
			}

		/* now fill up new colortable with up to 64 specified values */
		for (i=graphic->pixel_start;i<64;i++)
			{
			j = i - graphic->pixel_start;
			graphic->color[i].pixel = i;
			graphic->color[i].red =   (unsigned short) (256*(colors[j*3]+1) - 1);
			graphic->color[i].green = (unsigned short) (256*(colors[j*3+1]+1) - 1);
			graphic->color[i].blue =  (unsigned short) (256*(colors[j*3+2]+1) - 1);
			graphic->color[i].flags = DoRed | DoGreen | DoBlue;
			XStoreColor(graphic->dpy, graphic->cmap, &(graphic->color[i]));
			}

		/* now fill up rest of new colortable with 
			black for overlay 1 and black for overlay 2*/
		for (i=64;i<128;i++)
			{
			graphic->color[i].pixel = i;
			graphic->color[i].red =   (unsigned short) 0;
			graphic->color[i].green = (unsigned short) 0;
			graphic->color[i].blue =  (unsigned short) 0;
			graphic->color[i].flags = DoRed | DoGreen | DoBlue;
			XStoreColor(graphic->dpy, graphic->cmap, 
				&(graphic->color[i]));
			}
		for (i=128;i<256;i++)
			{
			graphic->color[i].pixel = i;
			graphic->color[i].red =   (unsigned short) 0;
			graphic->color[i].green = (unsigned short) 0;
			graphic->color[i].blue =  (unsigned short) 0;
			graphic->color[i].flags = DoRed | DoGreen | DoBlue;
			XStoreColor(graphic->dpy, graphic->cmap, 
				&(graphic->color[i]));
			}

		/* set window to use new colortable */
		XSetWindowColormap(graphic->dpy,graphic->xid,graphic->cmap);

		/* set background and foreground to first two values
			specified for new colortable */
		graphic->bg_pixel = (unsigned long) graphic->pixel_start;
		graphic->fg_pixel = (unsigned long) graphic->pixel_start + 1;
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
	graphic->gc_val.foreground = graphic->fg_pixel;
	graphic->gc_val.background = graphic->bg_pixel;
	graphic->gc_val.font = graphic->font_info->fid;

	/* set plane mask to modify all 8 planes */
	graphic->gc_val.plane_mask = AllPlanes;
	graphic->gc_val.line_style = LineSolid;
	graphic->gc[0] = XCreateGC(graphic->dpy, graphic->xid, 
		(GCForeground | GCBackground | GCFont 
		| GCPlaneMask | GCLineStyle),
		&(graphic->gc_val));

	/* set plane mask to modify lower 6 planes */
	graphic->gc_val.plane_mask = (unsigned long) 63;
	graphic->gc_val.line_style = LineSolid;
	graphic->gc[1] = XCreateGC(graphic->dpy, graphic->xid, 
		(GCForeground | GCBackground | GCFont 
		| GCPlaneMask | GCLineStyle),
		&(graphic->gc_val));

	/* set plane mask to modify plane 7 */
	graphic->gc_val.plane_mask = (unsigned long) 1<<6;
	graphic->gc_val.line_style = LineSolid;
	graphic->gc[2] = XCreateGC(graphic->dpy, graphic->xid, 
		(GCForeground | GCBackground | GCFont 
		| GCPlaneMask | GCLineStyle),
		&(graphic->gc_val));

	/* set plane mask to modify plane 7 with dash lines */
	graphic->gc_val.plane_mask = (unsigned long) 1<<6;
	graphic->gc_val.line_style = LineOnOffDash;
	graphic->gc[3] = XCreateGC(graphic->dpy, graphic->xid, 
		(GCForeground | GCBackground | GCFont 
		| GCPlaneMask | GCLineStyle),
		&(graphic->gc_val));

	/* set plane mask to modify plane 8 */
	graphic->gc_val.plane_mask = (unsigned long) 1<<7;
	graphic->gc_val.line_style = LineSolid;
	graphic->gc[4] = XCreateGC(graphic->dpy, graphic->xid, 
		(GCForeground | GCBackground | GCFont 
		| GCPlaneMask | GCLineStyle),
		&(graphic->gc_val));

	/* set plane mask to modify plane 8 with dashed lines */
	graphic->gc_val.plane_mask = (unsigned long) 1<<7;
	graphic->gc_val.line_style = LineOnOffDash;
	graphic->gc[5] = XCreateGC(graphic->dpy, graphic->xid, 
		(GCForeground | GCBackground | GCFont 
		| GCPlaneMask | GCLineStyle),
		&(graphic->gc_val));

	/* set clipping mask to whole canvas for all gc's */
	(void) xg_setclip(graphic,graphic->bounds[0],graphic->bounds[2],
		graphic->bounds[1]-graphic->bounds[0],
		graphic->bounds[3]-graphic->bounds[2]);

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
 *	XG_SETWINCOLORMAP
 *	- sets colormap for specified window (using XID number)
 **********************************************************************/
void xg_setwincolormap(xgid, win_xid)
int	xgid;
Window	win_xid;
{
	struct xg_graphic *graphic;

	graphic = (struct xg_graphic *) xgid;
	XSetWindowColormap(graphic->dpy,win_xid,graphic->cmap);
}
/**********************************************************************
 *	XG_GETPIXELVALUE
 *	- gets pixel value from integer color number
 **********************************************************************/
void xg_getpixelvalue(xgid, color, pixel_value, gc_id)
int	xgid;
int	color;
unsigned long *pixel_value;
int	*gc_id;
{
	struct xg_graphic *graphic;

	graphic = (struct xg_graphic *) xgid;
	if (graphic->display_type == PseudoColor)
		if (color < 0)
			{
			*pixel_value = (unsigned long) (-color - 1);
			*gc_id = 0;
			}
		else if (color == CLEAR_ALL)
			{
			*pixel_value = (unsigned long) 0;
			*gc_id = 0;
			}
		else if (color == OVERLAY1_CLEAR)
			{
			*pixel_value = (unsigned long) 0;
			*gc_id = 2;
			}
		else if (color == OVERLAY1_DRAW)
			{
			*pixel_value = (unsigned long) 1<<6;
			*gc_id = 2;
			}
		else if (color == OVERLAY1_DASH)
			{
			*pixel_value = (unsigned long) 1<<6;
			*gc_id = 3;
			}
		else if (color == OVERLAY2_CLEAR)
			{
			*pixel_value = (unsigned long) 0;
			*gc_id = 4;
			}
		else if (color == OVERLAY2_DRAW)
			{
			*pixel_value = (unsigned long) 1<<7;
			*gc_id = 4;
			}
		else if (color == OVERLAY2_DASH)
			{
			*pixel_value = (unsigned long) 1<<7;
			*gc_id = 5;
			}
		else
			{
			*pixel_value = graphic->pixel_start + (unsigned long) color;
			*gc_id = 1;
			}
	else
		{
		if (color > 0)
			*pixel_value = graphic->fg_pixel;
		else
			*pixel_value = graphic->bg_pixel;
		*gc_id = 0;
		}
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
	XRectangle rectangle;
	int	i;

	/* set up rectangle */
	rectangle.x = x;
	rectangle.y = y;
	rectangle.width = width;
	rectangle.height = height;

	/* set clip rectangle */
	graphic = (struct xg_graphic *) xgid;
	for (i=0;i<6;i++)
		XSetClipRectangles(graphic->dpy, graphic->gc[i], 0, 0, 
			rectangle, 1, Unsorted);
}
/**********************************************************************
 *	XG_DRAWPOINT
 *	- draws a pixel
 **********************************************************************/
void xg_drawpoint(xgid, x, y, color)
int	xgid;
int	x, y;
int	color;
{
	unsigned long pixel_value;
	int gc_id;
	struct xg_graphic *graphic;

	graphic = (struct xg_graphic *) xgid;
	xg_getpixelvalue(xgid,color,&pixel_value,&gc_id);
	XSetForeground(graphic->dpy, graphic->gc[gc_id], pixel_value);
	XDrawPoint(graphic->dpy, graphic->xid, graphic->gc[gc_id], x, y);
}
/**********************************************************************
 *	XG_DRAWLINE
 *	- draws a line
 **********************************************************************/
void xg_drawline(xgid, x1, y1, x2, y2, color)
int	xgid;
int	x1, y1, x2, y2;
int	color;
{
	unsigned long pixel_value;
	int gc_id;
	struct xg_graphic *graphic;

	graphic = (struct xg_graphic *) xgid;
	xg_getpixelvalue(xgid,color,&pixel_value,&gc_id);
	XSetForeground(graphic->dpy, graphic->gc[gc_id], pixel_value);
	XDrawLine(graphic->dpy, graphic->xid, graphic->gc[gc_id], x1, y1, x2, y2);
}
/**********************************************************************
 *	XG_DRAWRECTANGLE
 *	- draws a rectangle outline
 **********************************************************************/
void xg_drawrectangle(xgid, x, y, width, height, color)
int	xgid;
int	x, y, width, height;
int	color;
{
	unsigned long pixel_value;
	int gc_id;
	struct xg_graphic *graphic;

	graphic = (struct xg_graphic *) xgid;
	xg_getpixelvalue(xgid,color,&pixel_value,&gc_id);
	XSetForeground(graphic->dpy, graphic->gc[gc_id], pixel_value);
	XDrawRectangle(graphic->dpy, graphic->xid, graphic->gc[gc_id], x, y, width, height);
}
/**********************************************************************
 *	XG_DRAWTRIANGLE
 *	- draws a triangle outline
 **********************************************************************/
void xg_drawtriangle(xgid, x1, y1, x2, y2, x3, y3, color)
int	xgid;
int	x1, y1, x2, y2, x3, y3;
int	color;
{
	XSegment segments[3];
	int nsegments = 3;
	unsigned long pixel_value;
	int gc_id;
	struct xg_graphic *graphic;

	graphic = (struct xg_graphic *) xgid;
	xg_getpixelvalue(xgid,color,&pixel_value,&gc_id);
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
	XSetForeground(graphic->dpy, graphic->gc[gc_id], pixel_value);
	XDrawSegments(graphic->dpy, graphic->xid, graphic->gc[gc_id], segments, nsegments);
}
/**********************************************************************
 *	XG_FILLRECTANGLE
 *	- fills a rectangle
 **********************************************************************/
void xg_fillrectangle(xgid, x, y, width, height, color)
int	xgid;
int	x, y, width, height;
int	color;
{
	unsigned long pixel_value;
	int gc_id;
	struct xg_graphic *graphic;

	graphic = (struct xg_graphic *) xgid;
	xg_getpixelvalue(xgid,color,&pixel_value,&gc_id);
	XSetForeground(graphic->dpy, graphic->gc[gc_id], pixel_value);
	XFillRectangle(graphic->dpy, graphic->xid, graphic->gc[gc_id], x, y, width, height);
}
/**********************************************************************
 *	XG_FILLTRIANGLE
 *	- fills a triangle
 **********************************************************************/
void xg_filltriangle(xgid, x1, y1, x2, y2, x3, y3, color)
int	xgid;
int	x1, y1, x2, y2, x3, y3;
int	color;
{
	XPoint points[3];
	int npoints = 3;
	int shape = Convex;
	int mode = CoordModeOrigin;
	unsigned long pixel_value;
	int gc_id;
	struct xg_graphic *graphic;

	graphic = (struct xg_graphic *) xgid;
	xg_getpixelvalue(xgid,color,&pixel_value,&gc_id);
	points[0].x = (short) x1;
	points[0].y = (short) y1;
	points[1].x = (short) x2;
	points[1].y = (short) y2;
	points[2].x = (short) x3;
	points[2].y = (short) y3;
	XSetForeground(graphic->dpy, graphic->gc[gc_id], pixel_value);
	XFillPolygon(graphic->dpy, graphic->xid, graphic->gc[gc_id], points, npoints, shape, mode);
}
/**********************************************************************
 *	XG_DRAWSTRING
 *	- draws a string
 **********************************************************************/
void xg_drawstring(xgid, x, y, string, color)
int	xgid;
int	x, y;
char	*string;
int	color;
{
	unsigned long pixel_value;
	int gc_id;
	struct xg_graphic *graphic;
	int	string_length;

	graphic = (struct xg_graphic *) xgid;
	xg_getpixelvalue(xgid,color,&pixel_value,&gc_id);
	XSetForeground(graphic->dpy, graphic->gc[gc_id], pixel_value);
	string_length = strlen(string);
	XDrawString(graphic->dpy, graphic->xid, graphic->gc[gc_id], 
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
	int gc_id;
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
