/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 2009-2015 by P. Wessel and R. Scharroo
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation; version 3 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *--------------------------------------------------------------------*/
/*
 * This include file must be included by all programs using pslib.a
 *
 * Authors:	Paul Wessel, Dept. of Geology and Geophysics, SOEST, U Hawaii
 *			   pwessel@hawaii.edu
 *		Remko Scharroo, Altimetrics
 *			   remko@altimetrics.com
 * Version:	5.0 [64-bit enabled API edition]
 * Date:	15-OCT-2009
 */

#ifndef _PSLIB_H
#define _PSLIB_H

#ifdef __cplusplus
extern "C" {
#endif

/* CMake definitions: This must be first! */
//#include "gmt_config.h"

/* Declaration modifiers for DLL support (MSC et al) */
#include "declspec.h"

#include <stdio.h>

/* Number of PostScript points in one inch */

#define PSL_POINTS_PER_INCH	72.0
#define PSL_DOTS_PER_INCH	1200.0	/* Effective dots per inch resolution */
#define PSL_ALL_CLIP		INT_MAX	/* Terminates all clipping */

/* PSL codes for geometric symbols as expected by PSL_plotsymbol */

#define PSL_STAR		((int)'a')
#define PSL_CIRCLE		((int)'c')
#define PSL_DIAMOND		((int)'d')
#define PSL_ELLIPSE		((int)'e')
#define PSL_HEXAGON		((int)'h')
#define PSL_OCTAGON		((int)'g')
#define PSL_INVTRIANGLE		((int)'i')
#define PSL_ROTRECT		((int)'j')
#define PSL_MARC		((int)'m')
#define PSL_PENTAGON		((int)'n')
#define PSL_DOT			((int)'p')
#define PSL_RECT		((int)'r')
#define PSL_RNDRECT		((int)'R')
#define PSL_SQUARE		((int)'s')
#define PSL_TRIANGLE		((int)'t')
#define PSL_VECTOR		((int)'v')
#define PSL_WEDGE		((int)'w')
#define PSL_CROSS		((int)'x')
#define PSL_YDASH		((int)'y')
#define PSL_PLUS		((int)'+')
#define PSL_XDASH		((int)'-')

/* PSL codes for vector attributes - mirroring similar codes and macros in GMT */

/* Note: If changes are made to PSL_enum_vecattr you must also change gmt_plot.h: GMT_enum_vecattr */
enum PSL_enum_vecattr {PSL_VEC_LEFT = 1,	/* Only draw left half of vector head */
	PSL_VEC_RIGHT		= 2,		/* Only draw right half of vector head */
	PSL_VEC_BEGIN		= 4,		/* Place vector head at beginning of vector */
	PSL_VEC_END		= 8,		/* Place vector head at end of vector */
	PSL_VEC_JUST_B		= 0,		/* Align vector beginning at (x,y) */
	PSL_VEC_JUST_C		= 16,		/* Align vector center at (x,y) */
	PSL_VEC_JUST_E		= 32,		/* Align vector end at (x,y) */
	PSL_VEC_JUST_S		= 64,		/* Align vector center at (x,y) */
	PSL_VEC_ANGLES		= 128,		/* Got start/stop angles instead of az, length */
	PSL_VEC_POLE		= 256,		/* Got pole of small/great circle */
	PSL_VEC_OUTLINE		= 512,		/* Draw vector head outline using default pen */
	PSL_VEC_OUTLINE2	= 1024,		/* Draw vector head outline using supplied v_pen */
	PSL_VEC_FILL		= 2048,		/* Fill vector head using default fill */
	PSL_VEC_FILL2		= 4096,		/* Fill vector head using supplied v_fill) */
	PSL_VEC_MARC90		= 8192};	/* Matharc only: if angles subtend 90, draw straight angle symbol */

#define PSL_vec_justify(status) ((status>>4)&3)			/* Return justification as 0-3 */
#define PSL_vec_head(status) ((status>>2)&3)			/* Return head selection as 0-3 */
#define PSL_vec_side(status) ((status&3) ? 2*(status&3)-3 : 0)	/* Return side selection as 0,-1,+1 */

/* PSL codes for arguments of PSL_beginplot and other routines */

enum PSL_enum_const {PSL_CM	= 0,
	PSL_INCH		= 1,
	PSL_METER		= 2,
	PSL_PT			= 3,
	PSL_FINALIZE		= 1,
	PSL_OVERLAY		= 1,
	PSL_INIT		= 0,
	PSL_LANDSCAPE		= 0,
	PSL_PORTRAIT		= 1,
	PSL_ASCII85		= 0,
	PSL_HEX			= 1,
	PSL_NONE		= 0,
	PSL_RLE			= 1,
	PSL_LZW			= 2,
	PSL_DEFLATE		= 3,
	PSL_NO			= 0,
	PSL_YES			= 1,
	PSL_FWD			= 0,
	PSL_INV			= 1,
	PSL_OUTLINE		= 1,
	PSL_MAX_EPS_FONTS	= 6,
	PSL_MAX_DIMS		= 8,		/* Max number of dim arguments to PSL_plot_symbol */
	PSL_N_PATTERNS		= 91,		/* Current number of predefined patterns + 1, # 91 is user-supplied */
	PSL_BUFSIZ		= 4096U};	/* To match GMT_BUFSIZ and be consistent across all platforms */

/* PSL codes for pen movements (used by PSL_plotpoint, PSL_plotline, PSL_plotarc) */

enum PSL_enum_move {PSL_DRAW	= 0,
	PSL_MOVE		= 1,
	PSL_STROKE		= 2,
	PSL_REL			= 4,
	PSL_CLOSE		= 8};

/* PSL codes for text and paragraph justification */

enum PSL_enum_just {PSL_BL	= 1,
	PSL_BC			= 2,
	PSL_BR			= 3,
	PSL_ML			= 5,
	PSL_MC			= 6,
	PSL_MR			= 7,
	PSL_TL			= 9,
	PSL_TC			= 10,
	PSL_TR			= 11,
	PSL_JUST		= 4};

/* PSL code for rectangle shapes */

enum PSL_enum_rect {PSL_RECT_STRAIGHT	= 0,
	PSL_RECT_ROUNDED,
	PSL_RECT_CONVEX,
	PSL_RECT_CONCAVE};

/* PSL codes for line settings */

enum PSL_enum_line {PSL_BUTT_CAP	= 0,
	PSL_ROUND_CAP			= 1,
	PSL_SQUARE_CAP			= 2,
	PSL_MITER_JOIN			= 0,
	PSL_ROUND_JOIN			= 1,
	PSL_BEVEL_JOIN			= 2,
	PSL_MITER_DEFAULT		= 35};

/* Verbosity levels */

enum PSL_enum_verbose {PSL_MSG_QUIET = 0,	/* No messages whatsoever */
	PSL_MSG_FATAL,		/* Fatal errors */
	PSL_MSG_TICTOC,		/* To print a tic-toc elapsed time message */
	PSL_MSG_COMPAT,		/* Compatibility warnings */
	PSL_MSG_NORMAL,		/* Warnings level -V */
	PSL_MSG_VERBOSE,	/* Longer verbose, -Vl in some programs */
	PSL_MSG_DEBUG};		/* Debug messages for developers mostly */

/* Color spaces */

enum PSL_enum_color {PSL_RGB = 0,
	PSL_CMYK,
	PSL_HSV,
	PSL_GRAY};

/* Color types */

enum PSL_enum_fill {PSL_IS_STROKE = 0,
	PSL_IS_FILL,
	PSL_IS_FONT};

/* PSL error codes */

enum PSL_enum_err {PSL_BAD_VALUE = -99,	/* Bad value */
	PSL_BAD_JUST,		/* Bad text or paragraph justification */
	PSL_READ_FAILURE,	/* Less than 3 points */
	PSL_NO_POLYGON,		/* Less than 3 points */
	PSL_BAD_TEXT,		/* Text is too long */
	PSL_BAD_WIDTH,		/* Negative line width */
	PSL_NO_PATH,		/* Less than 2 points given as path */
	PSL_BAD_SYMBOL,		/* Unknown symbol type */
	PSL_BAD_SIZE,		/* Size is negative */
	PSL_BAD_RANGE,		/* Range defined by min/max exceeds limit */
	PSL_BAD_FLAG,		/* A flag is outside required range */
	PSL_NO_SESSION,		/* No active session */
	PSL_NO_ERROR = 0};	/* No errors, all is OK */

/*--------------------------------------------------------------------
 *			PSL PARAMETERS DEFINITIONS
 *--------------------------------------------------------------------*/

struct PSL_CTRL {
	struct INIT {	/* Parameters set by user via PSL_beginplot() */
		FILE *err;			/* Error stream (NULL means stderr)		*/
		char *encoding;			/* The encoding name. e.g. ISO-8859-1		*/
		char *session;			/* The session name (NULL)			*/
		int unit;			/* 0 = cm, 1 = inch, 2 = meter			*/
		int copies;		/* Number of copies for this plot		*/
		double page_rgb[4];		/* RGB color for background paper [white]	*/
		double page_size[2];		/* Width and height of paper used in points	*/
		double magnify[2];		/* Global scale values [1/1]			*/
	} init;
	struct CURRENT {	/* Variables and settings that changes via PSL_* calls */
		char string[PSL_BUFSIZ];	/* Last text string plotted			*/
		char style[512];		/* Current setdash pattern			*/
		char bw_format[8];		/* Format used for grayshade value		*/
		char rgb_format[64];		/* Same, for RGB color triplets			*/
		char hsv_format[64];		/* Same, for HSV color triplets	(HSB in PS)	*/
		char cmyk_format[64];		/* Same, for CMYK color quadruples		*/
		char transparency_mode[16];	/* PDF transparency mode			*/
		double linewidth;		/* Current pen thickness			*/
		double rgb[3][4];		/* Current stroke, fill, and fs fill rgb	*/
		double offset;			/* Current setdash offset			*/
		double fontsize;		/* Current font size				*/
		int nclip;			/* Clip depth 					*/
		int font_no;		/* Current font number				*/
		int outline;		/* Current outline				*/
	} current;
	struct INTERNAL {	/* Variables used internally only */
		char *SHAREDIR;			/* Pointer to path of directory with pslib subdirectory */
		char *USERDIR;			/* Pointer to path of directory with user definitions (~/.gmt) */
		char *user_image[PSL_N_PATTERNS];	/* Name of user patterns		*/
		char origin[2];			/* 'r', 'a', 'f', 'c' depending on reference for new origin x and y coordinate */
		double offset[2];		/* Origin offset [1/1]				*/
		double p_width;			/* Paper width in points, set in PSL_beginplot();	*/
		double p_height;		/* Paper height in points, set in PSL_beginplot();	*/
		double dpu;			/* PS dots per unit.  Must be set through PSL_beginplot();		*/
		double dpp;			/* PS dots per point.  Must be set through PSL_beginplot();		*/
		double x2ix;			/* Scales user x to PS dots			*/
		double y2iy;			/* Scales user y to PS dots			*/
		double p2u;			/* Scales dimensions in points (e.g., fonts, linewidths) to user units (e.g. inch)		*/
		double axis_limit[4];		/* The current xmin, xmax, ymin, ymax settings for axes */
		double axis_pos[2];		/* Lower left placement for axes		*/
		double axis_dim[2];		/* Lengths of axes 				*/
		int verbose;		/* Verbosity level (0-4): see PSL_MSG_*	*/
		int comments;		/* true for writing comments to output, false strips all comments */
		int overlay;		/* true if overlay (-O)				*/
		int landscape;		/* true = Landscape, false = Portrait		*/
		int text_init;		/* true after PSL_text.ps has been loaded	*/
		int image_format;		/* 0 writes images in ascii, 2 uses binary	*/
		int N_FONTS;		/* Total no of fonts;  To add more, modify the file CUSTOM_font_info.d */
		int compress;		/* Compresses images with (1) RLE or (2) LZW (3) DEFLATE or (0) None */
		unsigned deflate_level; /* Compression level for DEFLATE (1-9, default 0) */
		int color_mode;		/* 0 = rgb, 1 = cmyk, 2 = hsv (only 1-2 for images)	*/
		int line_cap;		/* 0, 1, or 2 for butt, round, or square [butt]	*/
		int line_join;		/* 0, 1, or 2 for miter, arc, or bevel [miter]	*/
		int miter_limit;		/* Acute angle threshold 0-180; 0 means PS default [0] */
		int ix, iy;		/* Absolute coordinates of last point		*/
		int length;		/* Image row output byte counter		*/
		int n_userimages;		/* Number of specified custom patterns		*/
		int x0, y0;		/* x,y PS offsets				*/
		FILE *fp;			/* PS output file pointer. NULL = stdout	*/
		struct PSL_FONT {
			double height;		/* Height of A for unit fontsize */
			char *name;		/* Name of this font */
			int encoded;	/* true if we never should reencode this font (e.g. symbols) */
						/* This is also changed to true after we do reencode a font */
		} *font;	/* Pointer to array of font structures 		*/
		struct PSL_PATTERN {
			int nx, ny;
			int status, depth, dpi;
			double f_rgb[4], b_rgb[4];
		} pattern[PSL_N_PATTERNS*2];
	} internal;
};

struct imageinfo {
	int magic;		/* magic number */
	int width;		/* width (pixels) of image */
	int height;		/* height (pixels) of image */
	int depth;		/* depth (1, 8, or 24 bits) of pixel; 0 for EPS */
	int length;		/* length (bytes) of image */
	int type;		/* type of file; see RT_* below */
	int maptype;	/* type of colormap; see RMT_* below */
	int maplength;	/* length (bytes) of following map */
	int xorigin;	/* x-coordinate of origin (EPS only) */
	int yorigin;	/* y-coordinate of origin (EPS only) */
	/* color map follows for maplength bytes, followed by image */
};

#define	RAS_MAGIC	0x59a66a95	/* Magic number for Sun rasterfile */
#define EPS_MAGIC	0x25215053	/* Magic number for EPS file */
#define RT_OLD		0		/* Old-style, unencoded Sun rasterfile */
#define RT_STANDARD	1		/* Standard, unencoded Sun rasterfile */
#define RT_BYTE_ENCODED	2		/* Run-length-encoded Sun rasterfile */
#define RT_FORMAT_RGB	3		/* [X]RGB instead of [X]BGR Sun rasterfile */
#define RT_EPS		4		/* Encapsulated PostScript format */
#define RMT_NONE	0		/* maplength is expected to be 0 */
#define RMT_EQUAL_RGB	1		/* red[maplength/3], green[], blue[] follow */

/* Public functions */

EXTERN_MSC struct PSL_CTRL *New_PSL_Ctrl (char *session);
EXTERN_MSC int PSL_beginaxes (struct PSL_CTRL *PSL, double llx, double lly, double width, double height, double x0, double y0, double x1, double y1);
EXTERN_MSC int PSL_beginclipping (struct PSL_CTRL *PSL, double *x, double *y, int n, double rgb[], int flag);
EXTERN_MSC int PSL_beginlayer (struct PSL_CTRL *PSL, int layer);
EXTERN_MSC int PSL_beginplot (struct PSL_CTRL *PSL, FILE *fp, int orientation, int overlay, int color_mode, char origin[], double offset[], double page_size[], char *title, int font_no[]);
EXTERN_MSC int PSL_beginsession (struct PSL_CTRL *PSL, unsigned int search, char *sharedir, char *userdir);
EXTERN_MSC int PSL_endaxes (struct PSL_CTRL *PSL);
EXTERN_MSC int PSL_endclipping (struct PSL_CTRL *PSL, int mode);
EXTERN_MSC int PSL_endlayer (struct PSL_CTRL *PSL);
EXTERN_MSC int PSL_endplot (struct PSL_CTRL *PSL, int lastpage);
EXTERN_MSC int PSL_endsession (struct PSL_CTRL *PSL);
EXTERN_MSC int PSL_plotarc (struct PSL_CTRL *PSL, double x, double y, double radius, double az1, double az2, int type);
EXTERN_MSC int PSL_plotaxis (struct PSL_CTRL *PSL, double annotation_int, char *label, double annotfontsize, int side);
EXTERN_MSC int PSL_plotbitimage (struct PSL_CTRL *PSL, double x, double y, double xsize, double ysize, int justify, unsigned char *buffer, int nx, int ny, double f_rgb[], double b_rgb[]);
EXTERN_MSC int PSL_plotcolorimage (struct PSL_CTRL *PSL, double x, double y, double xsize, double ysize, int justify, unsigned char *buffer, int nx, int ny, int nbits);
EXTERN_MSC int PSL_plotepsimage (struct PSL_CTRL *PSL, double x, double y, double xsize, double ysize, int justify, unsigned char *buffer, int size, int nx, int ny, int ox, int oy);
EXTERN_MSC int PSL_plotline (struct PSL_CTRL *PSL, double *x, double *y, int n, int type);
EXTERN_MSC int PSL_plotparagraph (struct PSL_CTRL *PSL, double x, double y, double fontsize, char *paragraph, double angle, int justify);
EXTERN_MSC int PSL_plotparagraphbox (struct PSL_CTRL *PSL, double x, double y, double fontsize, char *paragraph, double angle, int justify, double offset[], int mode);
EXTERN_MSC int PSL_plotpoint (struct PSL_CTRL *PSL, double x, double y, int pen);
EXTERN_MSC int PSL_plotbox (struct PSL_CTRL *PSL, double x0, double y0, double x1, double y1);
EXTERN_MSC int PSL_plotpolygon (struct PSL_CTRL *PSL, double *x, double *y, int n);
EXTERN_MSC int PSL_plotsegment (struct PSL_CTRL *PSL, double x0, double y0, double x1, double y1);
EXTERN_MSC int PSL_plotsymbol (struct PSL_CTRL *PSL, double x, double y, double param[], int symbol);
EXTERN_MSC int PSL_plottext (struct PSL_CTRL *PSL, double x, double y, double fontsize, char *text, double angle, int justify, int mode);
EXTERN_MSC int PSL_plottextbox (struct PSL_CTRL *PSL, double x, double y, double fontsize, char *text, double angle, int justify, double offset[], int mode);
EXTERN_MSC int PSL_plottextclip (struct PSL_CTRL *PSL, double x[], double y[], int m, double fontsize, char *label[], double angle[], int justify, double offset[], int mode);
EXTERN_MSC int PSL_plottextpath (struct PSL_CTRL *PSL, double x[], double y[], int n, int node[], double fontsize, char *label[], int m, double angle[], int justify, double offset[], int mode);
EXTERN_MSC int PSL_loadimage (struct PSL_CTRL *PSL, char *file, struct imageinfo *header, unsigned char **image);
EXTERN_MSC int PSL_setcolor (struct PSL_CTRL *PSL, double rgb[], int mode);
EXTERN_MSC int PSL_setdefaults (struct PSL_CTRL *PSL, double xyscales[], double page_rgb[], char *encoding);
EXTERN_MSC int PSL_setdash (struct PSL_CTRL *PSL, char *pattern, double offset);
EXTERN_MSC int PSL_setfill (struct PSL_CTRL *PSL, double rgb[], int outline);
EXTERN_MSC int PSL_setfont (struct PSL_CTRL *PSL, int font_no);
EXTERN_MSC int PSL_setformat (struct PSL_CTRL *PSL, int n_decimals);
EXTERN_MSC int PSL_setlinecap (struct PSL_CTRL *PSL, int cap);
EXTERN_MSC int PSL_setlinejoin (struct PSL_CTRL *PSL, int join);
EXTERN_MSC int PSL_setlinewidth (struct PSL_CTRL *PSL, double linewidth);
EXTERN_MSC int PSL_setmiterlimit (struct PSL_CTRL *PSL, int limit);
EXTERN_MSC int PSL_setorigin (struct PSL_CTRL *PSL, double x, double y, double angle, int mode);
EXTERN_MSC int PSL_setparagraph (struct PSL_CTRL *PSL, double line_space, double par_width, int par_just);
EXTERN_MSC int PSL_setpattern (struct PSL_CTRL *PSL, int image_no, char *imagefile, int image_dpi, double f_rgb[], double b_rgb[]);
EXTERN_MSC int PSL_settransparencymode (struct PSL_CTRL *PSL, const char *mode);
EXTERN_MSC int PSL_definteger (struct PSL_CTRL *PSL, const char *param, int value);
EXTERN_MSC int PSL_defpen (struct PSL_CTRL *PSL, const char *param, double width, char *style, double offset, double rgb[]);
EXTERN_MSC int PSL_defpoints (struct PSL_CTRL *PSL, const char *param, double fontsize);
EXTERN_MSC int PSL_defcolor (struct PSL_CTRL *PSL, const char *param, double rgb[]);
EXTERN_MSC int PSL_deftextdim (struct PSL_CTRL *PSL, const char *dim, double fontsize, char *text);
EXTERN_MSC int PSL_defunits (struct PSL_CTRL *PSL, const char *param, double value);
EXTERN_MSC unsigned char *psl_gray_encode (struct PSL_CTRL *PSL, int *nbytes, unsigned char *input);

/* Other deep level routines that could be useful */
EXTERN_MSC int psl_ix (struct PSL_CTRL *PSL, double value);
EXTERN_MSC int psl_iy (struct PSL_CTRL *PSL, double value);
EXTERN_MSC int psl_iz (struct PSL_CTRL *PSL, double value);
EXTERN_MSC int psl_ip (struct PSL_CTRL *PSL, double value);

/* Used indirectly by macro PSL_free and FORTRAN wrapper PSL_free_ . */
EXTERN_MSC int PSL_free_nonmacro (void *addr);

/* Definition for printing a message. When DEBUG is on, also print source file and line number.
 * Use this for various progress statements, debugging to see certain variables, and even fatal
 * error messages. */
/* For FORTRAN there is PSL_command_ that only accepts one text argument */
EXTERN_MSC int PSL_command (struct PSL_CTRL *C, const char *format, ...);
EXTERN_MSC int PSL_comment (struct PSL_CTRL *C, const char *format, ...);
EXTERN_MSC int PSL_initerr (struct PSL_CTRL *C, const char *format, ...);
EXTERN_MSC int PSL_message (struct PSL_CTRL *C, int level, const char *format, ...);
EXTERN_MSC FILE *PSL_fopen (char *file, char *mode);

#define PSL_free(ptr) (PSL_free_nonmacro(ptr),(ptr)=NULL) /* Cleanly set the freed pointer to NULL */

#ifdef __cplusplus
}
#endif

#endif /* _PSLIB_H */
