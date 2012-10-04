/*--------------------------------------------------------------------
 *    The MB-system:	mb_aux.h	5/16/94
 *    $Id$
 *
 *    Copyright (c); 1993-2012 by
 *    David W. Caress (caress@mbari.org);
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu);
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mb_aux.h defines data structures used by swath contouring
 * functions and programs.  The source files mb_contour.c, mb_track.c,
 * and mbcontour.c all depend on this include file.
 *
 * Author:	D. W. Caress
 * Date:	May 15, 1994
 *
 * Name change:	mb_countour.h changed to mb_aux.h
 * Date:	October 13, 2009
 *
 * $Log: mb_contour.h,v $
 * Revision 5.7  2008/07/10 06:43:40  caress
 * Preparing for 5.1.1beta20
 *
 * Revision 5.6  2006/11/10 22:36:04  caress
 * Working towards release 5.1.0
 *
 * Revision 5.5  2005/11/04 22:49:51  caress
 * Programs changed to register arrays through mb_register_array(); rather than allocating the memory directly with mb_realloc(); or mb_malloc();.
 *
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
	int	pingnumber;
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
	int	plot_pingnumber;

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

	/* pingnumber control parameters */
	int	pingnumber_tick_int;
	int	pingnumber_annot_int;
	double	pingnumber_tick_len;

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
		int	plot_pingnumber,
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
		int	pingnumber_tick_int,
		int	pingnumber_annot_int,
		double	pingnumber_tick_len,
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
void mb_trackpingnumber(int verbose, struct swath *data, int *error);
void mb_trackname(int verbose, int perpendicular, struct swath *data, char *file, int *error);

/* pslibface function prototypes */
int plot_init(	int	verbose,
		int	argc,
		char	**argv,
		double	*bounds_use,
		double	*scale,
		double	*inch2lon,
		int	*error);
int plot_end(int verbose, int *error);
int plot_exit(int argc, char **argv);
void set_colors(int ncol, int *rd, int *gn, int *bl);
void plot(double x, double y, int ipen);
void setline(int linewidth);
void newpen(int ipen);
void justify_string(double height, char *string, double *s);
void plot_string(double x, double y, double hgt, double angle, char *label);

/* mb_surface function prototypes */
int mb_surface(int verbose, int ndat, float *xdat, float *ydat, float *zdat,
		double xxmin, double xxmax, double yymin, double yymax, double xxinc, double yyinc,
		double ttension, float *sgrid);
int mb_zgrid(float *z, int *nx, int *ny,
		float *x1, float *y1, float *dx, float *dy, float *xyz,
		int *n, float *zpij, int *knxt, int *imnew,
		float *cay, int *nrng);
int mb_zgrid2(float *z, int *nx, int *ny,
		float *x1, float *y1, float *dx, float *dy, float *xyz,
		int *n, float *zpij, int *knxt, int *imnew,
		float *cay, int *nrng);

/* mb_truecont function prototypes */
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
		int	plot_pingnumber,
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
		int	pingnumber_tick_int,
		int	pingnumber_annot_int,
		double	pingnumber_tick_len,
		int	*error);
int mb_contour_deall(
		int	verbose,
		struct swath *data,
		int	*error);
int mb_contour(
		int	verbose,
		struct swath *data,
		int	*error);
int mb_tcontour(
		int	verbose,
		struct swath *data,
		int	*error);

/* mb_delaun function prototypes */
int mb_delaun(
	int	verbose,
	int	npts,
	double	*p1,
	double	*p2,
	int	*ed,
	int	*ntri,
	int	*iv1,
	int	*iv2,
	int	*iv3,
	int	*ct1,
	int	*ct2,
	int	*ct3,
	int	*cs1,
	int	*cs2,
	int	*cs3,
	double	*v1,
	double	*v2,
	double	*v3,
	int	*istack,
	int	*kv1,
	int	*kv2,
	int	*error);

/* mb_readgrd function prototypes */
int mb_readgrd(int verbose, char *grdfile,
			int	*grid_projection_mode,
			char	*grid_projection_id,
			float	*nodatavalue,
			int	*nxy,
			int	*nx,
			int	*ny,
			double	*min,
			double	*max,
			double	*xmin,
			double	*xmax,
			double	*ymin,
			double	*ymax,
			double	*dx,
			double	*dy,
			float	**data,
			float	**data_dzdx,
			float	**data_dzdy,
			int	*error);

/* mb_spline function prototypes */
int mb_spline_init(int verbose, double *x, double *y,
	int n, double yp1, double ypn, double *y2, int *error);
int mb_spline_interp(int verbose, double *xa, double *ya, double *y2a,
	int n, double x, double *y, int *i, int *error);
int mb_linear_interp(int verbose, double *xa, double *ya,
		int n, double x, double *y, int *i, int *error);
int mb_linear_interp_degrees(int verbose, double *xa, double *ya,
		int n, double x, double *y, int *i, int *error);

/* mb_cheb function prototypes */
void lsqup(
    double  *a,
    int	    *ia,
    int	    *nia,
    int	    nnz,
    int	    nc,
    int	    nr,
    double  *x,
    double  *dx,
    double  *d,
    int	    nfix,
    int	    *ifix,
    double  *fix,
    int	    ncycle,
    double  *sigma);
void chebyu(
    double  *sigma,
    int	    ncycle,
    double  shi,
    double  slo,
    double  *work);
void splits(
    double  *x,
    double  *t,
    int	    n);
double errlim(
	double	*sigma,
	int	ncycle,
	double	shi,
	double	slo);
double errrat(
	double	x1,
	double	x2,
	double	*sigma,
	int	ncycle);
void lspeig(
	double	*a,
	int	*ia,
	int	*nia,
	int	nnz,
	int	nc,
	int	nr,
	int	ncyc,
	int	*nsig,
	double	*x,
	double	*dx,
	double	*sigma,
	double	*w,
	double	*smax,
	double	*err,
	double	*sup);
