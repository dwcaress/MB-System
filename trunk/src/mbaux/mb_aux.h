/*--------------------------------------------------------------------
 *    The MB-system:	mb_aux.h	5/16/94
 *    $Id$
 *
 *    Copyright (c); 1993-2014 by
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

	/* function pointers for plot functions */
        void (*contour_plot)(double x, double y, int ipen);
        void (*contour_newpen)(int ipen);
        void (*contour_setline)(int linewidth);
        void (*contour_justify_string)(double height, char *string, double *s);
        void (*contour_plot_string)(double x, double y, double hgt, double angle, char *label);

	};

/* topography grid structure for mb_intersectgrid() */
struct mb_topogrid_struct
	{
	mb_path	file;
	int	projection_mode;
	mb_path	projection_id;
	float	nodatavalue;
	int	nxy;
	int	nx;
	int	ny;
	double	min;
	double	max;
	double	xmin;
	double	xmax;
	double	ymin;
	double	ymax;
	double	dx;
	double	dy;
	float	*data;
	};

/* mb_contour and mb_track function prototypes */
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
		void   (*contour_plot)(double, double, int),
		void   (*contour_newpen)(int),
		void   (*contour_setline)(int),
		void   (*contour_justify_string)(double, char *, double *),
		void   (*contour_plot_string)(double, double, double, double, char *),
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
int mb_plot_init(	int	verbose,
		int	argc,
		char	**argv,
		double	*bounds_use,
		double	*scale,
		double	*inch2lon,
		int	*error);
int mb_plot_end(int verbose, int *error);
int mb_plot_exit(int argc, char **argv);
void mb_set_colors(int ncol, int *rd, int *gn, int *bl);
void mb_plot(double x, double y, int ipen);
void mb_setline(int linewidth);
void mb_newpen(int ipen);
void mb_justify_string(double height, char *string, double *s);
void mb_plot_string(double x, double y, double hgt, double angle, char *label);

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

/* mb_readwritegrd function prototypes */
int mb_read_gmt_grd(int verbose, char *grdfile,
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
int mb_write_gmt_grd(int verbose,
			char *grdfile,
			float *grid,
			float nodatavalue,
			int nx,
			int ny,
			double xmin,
			double xmax,
			double ymin,
			double ymax,
			double zmin,
			double zmax,
			double dx,
			double dy,
			char *xlab,
			char *ylab,
			char *zlab,
			char *titl,
			char *projection,
			int argc,
			char **argv,
			int *error);

/* mb_spline function prototypes */
int mb_spline_init(int verbose, double *x, double *y,
	int n, double yp1, double ypn, double *y2, int *error);
int mb_spline_interp(int verbose, double *xa, double *ya, double *y2a,
	int n, double x, double *y, int *i, int *error);
int mb_linear_interp(int verbose, double *xa, double *ya,
		int n, double x, double *y, int *i, int *error);
int mb_linear_interp_longitude(int verbose, double *xa, double *ya,
		int n, double x, double *y, int *i, int *error);
int mb_linear_interp_latitude(int verbose, double *xa, double *ya,
		int n, double x, double *y, int *i, int *error);
int mb_linear_interp_heading(int verbose, double *xa, double *ya,
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

/* mb_topogrid function prototypes */
int mb_topogrid_init(int verbose, mb_path topogridfile, int *lonflip,
			void **topogrid_ptr, int *error);
int mb_topogrid_deall(int verbose, void **topogrid_ptr, int *error);
int mb_topogrid_topo(int verbose, void *topogrid_ptr,
			double navlon, double navlat,
			double *topo, int *error);
int mb_topogrid_intersect(int verbose, void *topogrid_ptr,
                        double navlon, double navlat,
			double altitude, double sonardepth,
			double mtodeglon, double mtodeglat,
			double vx, double vy, double vz,
			double *lon, double *lat, double *topo, double *range,
                        int *error);
int mb_topogrid_getangletable(int verbose, void *topogrid_ptr,
                        int nangle, double angle_min, double angle_max,
			double navlon, double navlat, double heading,
			double altitude, double sonardepth, double pitch,
			double *table_angle, double *table_xtrack,
			double *table_ltrack, double *table_altitude,
			double *table_range, int *error);
