/*--------------------------------------------------------------------
 *    The MB-system:	mb_contour.h	5/16/94
 *    $Id: mb_contour.h,v 4.0 1994-05-16 22:12:46 caress Exp $
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
 * mb_contour.h defines data structures used by swath contouring
 * functions and programs.  The source files mb_contour.c, mb_track.c,
 * and mbcontour.c all depend on this include file. 
 *
 * Author:	D. W. Caress
 * Date:	May 15, 1994
 *
 * $Log: not supported by cvs2svn $
 *
 */


/* swath bathymetry data structure */
struct	ping
	{
	int	time_i[6];
	double	time_d;
	double	navlon;
	double	navlat;
	double	heading;
	double	*bath;
	double	*bathlon;
	double	*bathlat;
	};

/* structure including swath bathymetry data and control parameters
	for swath contouring and ship track plotting */
struct swath
	{
	/* raw swath data */
	int	npings;
	int	npings_max;
	int	beams_bath;
	struct ping *pings;

	/* what is plotted */
	int	plot_contours;
	int	plot_triangles;
	int	plot_track;

	/* contour control parameters */
	double	contour_int;
	double	color_int;
	double	tick_int;
	double	label_int;
	double	tick_len;
	double	label_hgt;
	int	ncolor;
	int	nlevel;
	int	nlevelset;
	double	*level_list;
	int	*label_list;
	int	*tick_list;
	int	*color_list;

	/* track control parameters */
	double	time_tick_int;
	double	time_annot_int;
	double	date_annot_int;
	double	time_tick_len;

	/* triangle network */
	int	npts;
	double	*x;
	double	*y;
	double	*z;
	int	*edge;
	int	*pingid;
	int	ntri;
	int	*iv[3];
	int	*ct[3];
	int	*cs[3];
	int	*ed[3];

	/* mb_delaun work arrays */
	double	*v1;
	double	*v2;
	double	*v3;
	int	*istack;
	int	*kv1;
	int	*kv2;

	/* triangle side flags */
	int	*flag[3];

	/* contour arrays */
	int	nsave;
	double	*xsave;
	double	*ysave;

	/* contour label arrays */
	int	nlabel;
	double	*xlabel;
	double	*ylabel;
	double	*angle;
	int	*justify;

	};
