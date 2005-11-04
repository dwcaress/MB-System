/*--------------------------------------------------------------------
 *    The MB-system:	mb_contour.h	5/16/94
 *    $Id: mb_contour.h,v 5.5 2005-11-04 22:49:51 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000, 2003, 2005 by
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
 * mb_contour.h defines data structures used by swath contouring
 * functions and programs.  The source files mb_contour.c, mb_track.c,
 * and mbcontour.c all depend on this include file. 
 *
 * Author:	D. W. Caress
 * Date:	May 15, 1994
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.4  2005/03/25 04:10:51  caress
 * Control over the filename annotation orientation has been added and the orientation itself has been fixed.
 *
 * Revision 5.3  2004/12/18 01:32:50  caress
 * Added filename annotation.
 *
 * Revision 5.2  2003/04/17 20:47:57  caress
 * Release 5.0.beta30
 *
 * Revision 5.1  2001/03/22 21:06:19  caress
 * Trying to make release 5.0.beta0
 *
 * Revision 5.0  2000/12/01  22:53:59  caress
 * First cut at Version 5.0.
 *
 * Revision 4.3  2000/09/30  06:54:58  caress
 * Snapshot for Dale.
 *
 * Revision 4.2  1998/10/04  04:18:07  caress
 * MB-System version 4.6beta
 *
 * Revision 4.1  1994/10/21  11:34:20  caress
 * Release V4.0
 *
 * Revision 4.0  1994/05/16  22:12:46  caress
 * First cut at new contouring scheme.
 *
 *
 */

/* contour algorithm defines */
#define	MB_CONTOUR_OLD	0
#define	MB_CONTOUR_TRIANGLES	1

/* swath bathymetry data structure */
struct	ping
	{
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	heading;
	int	beams_bath;
	int	beams_bath_alloc;
	char	*beamflag;
	double	*bath;
	double	*bathlon;
	double	*bathlat;
	int	*bflag[2];
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
	int	contour_algorithm;
	int	plot_contours;
	int	plot_triangles;
	int	plot_track;
	int	plot_name;

	/* contour control parameters */
	double	contour_int;
	double	color_int;
	double	tick_int;
	double	label_int;
	double	tick_len;
	double	label_hgt;
	double	label_spacing;
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
	double	name_hgt;

	/* triangle network */
	int	npts;
	int	npts_alloc;
	double	*x;
	double	*y;
	double	*z;
	int	*edge;
	int	*pingid;
	int	ntri;
	int	ntri_alloc;
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
	int	*isave;
	int	*jsave;

	/* contour label arrays */
	int	nlabel;
	double	*xlabel;
	double	*ylabel;
	double	*angle;
	int	*justify;

	};
	
/* mb_contour function prototypes */
int mb_contour_init(
		int	verbose, 
		struct swath **data,
		int	npings_max,
		int	beams_bath,
		int	contour_algorithm,
		int	plot_contours,
		int	plot_triangles,
		int	plot_track,
		int	plot_name,
		double	contour_int,
		double	color_int,
		double	tick_int,
		double	label_int,
		double	tick_len,
		double	label_hgt,
		double	label_spacing,
		int	ncolor,
		int	nlevel,
		double	*level_list,
		int	*label_list,
		int	*tick_list,
		double	time_tick_int,
		double	time_annot_int,
		double	date_annot_int,
		double	time_tick_len,
		double	name_hgt,
		int	*error);
int mb_contour_deall(
		int	verbose, 
		struct swath *data, 
		int	*error);
int mb_contour(
		int	verbose, 
		struct swath *data, 
		int	*error);
void mb_track(int verbose, struct swath *data, int *error);
void mb_trackname(int verbose, int perpendicular, struct swath *data, char *file, int *error);
