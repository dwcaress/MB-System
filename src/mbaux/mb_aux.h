/*--------------------------------------------------------------------
 *    The MB-system:  mb_aux.h  5/16/94
 *
 *    Copyright (c); 1993-2019 by
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
 * Author:  D. W. Caress
 * Date:  May 15, 1994
 *
 * Name change:  mb_countour.h changed to mb_aux.h
 * Date:  October 13, 2009
 *
 *
 */

#ifndef MB_AUX_H_
#define MB_AUX_H_

/* Avoid conflict with GDAL */
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_URL
#undef PACKAGE_VERSION
#include "mb_io.h"

/* contour algorithm defines */
#define MB_CONTOUR_OLD 0
#define MB_CONTOUR_TRIANGLES 1

/* swath bathymetry data structure */
struct ping {
  int time_i[7];
  double time_d;
  double navlon;
  double navlat;
  double heading;
  double sensordepth;
  unsigned int pingnumber;
  int beams_bath;
  int beams_bath_alloc;
  char *beamflag;
  double *bath;
  double *bathlon;
  double *bathlat;
  int *bflag[2];
};

/* structure including swath bathymetry data and control parameters
    for swath contouring and ship track plotting */
struct swath {
  /* raw swath data */
  int npings;
  int npings_max;
  int beams_bath;
  struct ping *pings;

  /* what is plotted */
  int contour_algorithm;
  int plot_contours;
  int plot_triangles;
  int plot_track;
  int plot_name;
  int plot_pingnumber;

  /* contour control parameters */
  double contour_int;
  double color_int;
  double tick_int;
  double label_int;
  double tick_len;
  double label_hgt;
  double label_spacing;
  int ncolor;
  int nlevel;
  int nlevelset;
  double *level_list;
  int *label_list;
  int *tick_list;
  int *color_list;

  /* track control parameters */
  double time_tick_int;
  double time_annot_int;
  double date_annot_int;
  double time_tick_len;
  double name_hgt;

  /* pingnumber control parameters */
  int pingnumber_tick_int;
  int pingnumber_annot_int;
  double pingnumber_tick_len;

  /* triangle network */
  int npts;
  int npts_alloc;
  int *edge;
  int *pingid;
  int *beamid;
  double *x;
  double *y;
  double *z;
  int ntri;
  int ntri_alloc;
  int *iv[3];
  int *ct[3];
  int *cs[3];
  int *ed[3];
  double bath_min;
  double bath_max;
  double triangle_scale;

  /* triangle side flags */
  int *flag[3];

  /* mb_delaun work arrays */
  int ndelaun_alloc;
  double *v1;
  double *v2;
  double *v3;
  int *istack;
  int *kv1;
  int *kv2;

  /* contour arrays */
  int nsave;
  int nsave_alloc;
  double *xsave;
  double *ysave;
  int *isave;
  int *jsave;

  /* contour label arrays */
  int nlabel;
  double *xlabel;
  double *ylabel;
  double *angle;
  int *justify;

  /* function pointers for plot functions */
  void (*contour_plot)(double x, double y, int ipen);
  void (*contour_newpen)(int ipen);
  void (*contour_setline)(int linewidth);
  void (*contour_justify_string)(double height, char *string, double *s);
  void (*contour_plot_string)(double x, double y, double hgt, double angle, char *label);
};

/* topography grid structure for mb_intersectgrid() */
struct mb_topogrid_struct {
  mb_path file;
  int projection_mode;
  mb_path projection_id;
  float nodatavalue;
  int nxy;
  int n_columns;
  int n_rows;
  double min;
  double max;
  double xmin;
  double xmax;
  double ymin;
  double ymax;
  double dx;
  double dy;
  float *data;
};

#ifdef __cplusplus
extern "C" {
#endif

/* mb_contour and mb_track function prototypes */
int mb_triangulate(int verbose, struct swath *data, int *error);
int mb_contour_init(int verbose, struct swath **data, int npings_max, int beams_bath, int contour_algorithm, int plot_contours,
                    int plot_triangles, int plot_track, int plot_name, int plot_pingnumber, double contour_int, double color_int,
                    double tick_int, double label_int, double tick_len, double label_hgt, double label_spacing, int ncolor,
                    int nlevel, double *level_list, int *label_list, int *tick_list, double time_tick_int, double time_annot_int,
                    double date_annot_int, double time_tick_len, double name_hgt, int pingnumber_tick_int,
                    int pingnumber_annot_int, double pingnumber_tick_len, void (*contour_plot)(double, double, int),
                    void (*contour_newpen)(int), void (*contour_setline)(int),
                    void (*contour_justify_string)(double, char *, double *),
                    void (*contour_plot_string)(double, double, double, double, char *), int *error);
int mb_contour_deall(int verbose, struct swath *data, int *error);
int mb_contour(int verbose, struct swath *data, int *error);
void mb_track(int verbose, struct swath *data, int *error);
void mb_trackpingnumber(int verbose, struct swath *data, int *error);
void mb_trackname(int verbose, int perpendicular, struct swath *data, char *file, int *error);

/* pslibface function prototypes */
int mb_plot_init(int verbose, int argc, char **argv, double *bounds_use, double *scale, double *inch2lon, int *error);
int mb_plot_end(int verbose, int *error);
int mb_plot_exit(int argc, char **argv);
void mb_set_colors(int ncol, int *rd, int *gn, int *bl);
void mb_plot(double x, double y, int ipen);
void mb_setline(int linewidth);
void mb_newpen(int ipen);
void mb_justify_string(double height, char *string, double *s);
void mb_plot_string(double x, double y, double hgt, double angle, char *label);

/* mb_surface function prototypes */
int mb_surface(int verbose, int ndat, float *xdat, float *ydat, float *zdat, double xxmin, double xxmax, double yymin,
               double yymax, double xxinc, double yyinc, double ttension, float *sgrid);
int mb_zgrid(float *z, int *n_columns, int *n_rows, float *x1, float *y1, float *dx, float *dy, float *xyz, int *n, float *zpij, int *knxt,
             int *imnew, float *cay, int *nrng);
int mb_zgrid2(float *z, int *n_columns, int *n_rows, float *x1, float *y1, float *dx, float *dy, float *xyz, int *n, float *zpij, int *knxt,
              int *imnew, float *cay, int *nrng);

/* mb_delaun function prototypes */
int mb_delaun(int verbose, int npts, double *p1, double *p2, int *ed, int *ntri, int *iv1, int *iv2, int *iv3, int *ct1, int *ct2,
              int *ct3, int *cs1, int *cs2, int *cs3, double *v1, double *v2, double *v3, int *istack, int *kv1, int *kv2,
              int *error);

/* mb_readwritegrd function prototypes */
int mb_read_gmt_grd(int verbose, char *grdfile, int *grid_projection_mode, char *grid_projection_id, float *nodatavalue, int *nxy,
                    int *n_columns, int *n_rows, double *min, double *max, double *xmin, double *xmax, double *ymin, double *ymax,
                    double *dx, double *dy, float **data, float **data_dzdx, float **data_dzdy, int *error);
int mb_write_gmt_grd(int verbose, char *grdfile, float *grid, float nodatavalue, int n_columns, int n_rows, double xmin, double xmax,
                     double ymin, double ymax, double zmin, double zmax, double dx, double dy, char *xlab, char *ylab, char *zlab,
                     char *titl, char *projection, int argc, char **argv, int *error);

/* mb_cheb function prototypes */
void lsqup(const double *a, const int *ia, const int *nia, int nnz, int nc, int nr, double *x, double *dx, const double *d, int nfix, const int *ifix,
           const double *fix, int ncycle, const double *sigma);
void chebyu(double *sigma, int ncycle, double shi, double slo, double *work);
void splits(double *x, double *t, int n);
double errlim(double *sigma, int ncycle, double shi, double slo);
double errrat(double x1, double x2, double *sigma, int ncycle);
void lspeig(const double *a, const int *ia, const int *nia, int nnz, int nc, int nr, int ncyc, int *nsig, double *x, double *dx, double *sigma,
            double *w, double *smax, double *err, double *sup);

/* mb_topogrid function prototypes */
int mb_topogrid_init(int verbose, mb_path topogridfile, int *lonflip, void **topogrid_ptr, int *error);
int mb_topogrid_deall(int verbose, void **topogrid_ptr, int *error);
int mb_topogrid_bounds(int verbose, void *topogrid_ptr, double bounds[4], int *error);
int mb_topogrid_topo(int verbose, void *topogrid_ptr, double navlon, double navlat, double *topo, int *error);
int mb_topogrid_intersect(int verbose, void *topogrid_ptr, double navlon, double navlat, double altitude, double sonardepth,
                          double mtodeglon, double mtodeglat, double vx, double vy, double vz, double *lon, double *lat,
                          double *topo, double *range, int *error);
int mb_topogrid_getangletable(int verbose, void *topogrid_ptr, int nangle, double angle_min, double angle_max, double navlon,
                              double navlat, double heading, double altitude, double sonardepth, double pitch,
                              double *table_angle, double *table_xtrack, double *table_ltrack, double *table_altitude,
                              double *table_range, int *error);

/* lsqr.h */
/*!
   \file
   Header file for ISO C version of LSQR.
*/
void mb_aprod(int mode, int m, int n, double x[], double y[], void *UsrWrk);

void mblsqr_lsqr(int m, int n, void (*aprod)(int mode, int m, int n, double x[], double y[], void *UsrWrk), double damp,
                 void *UsrWrk,
                 double u[],  // len = m
                 double v[],  // len = n
                 double w[],  // len = n
                 double x[],  // len = n
                 double se[], // len = *
                 double atol, double btol, double conlim, int itnlim, FILE *nout,
                 // The remaining variables are output only.
                 int *istop_out, int *itn_out, double *anorm_out, double *acond_out, double *rnorm_out, double *arnorm_out,
                 double *xnorm_out);

#define ZERO 0.0
#define ONE 1.0

/* cblas.h
   $Revision: 273 $ $Date: 2006-09-04 15:59:04 -0700 (Mon, 04 Sep 2006) $

   ----------------------------------------------------------------------
   This file is part of BCLS (Bound-Constrained Least Squares).

   Copyright (C) 2006 Michael P. Friedlander, Department of Computer
   Science, University of British Columbia, Canada. All rights
   reserved. E-mail: <mpf@cs.ubc.ca>.

   BCLS is free software; you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   BCLS is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General
   Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with BCLS; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
   USA
   ----------------------------------------------------------------------
*/
/*!
   \file
   CBLAS library header file.
*/

#define MBCBLAS_OFFSET(N, incX) ((incX) > 0 ? 0 : ((N)-1) * (-(incX)))

enum MBCBLAS_ORDER { MBCblasRowMajor = 101, MBCblasColMajor = 102 };
enum MBCBLAS_TRANSPOSE { MBCblasNoTrans = 111, MBCblasTrans = 112, MBCblasConjTrans = 113 };

void mbcblas_daxpy(const int N, const double alpha, const double *X, const int incX, double *Y, const int incY);

void mbcblas_dcopy(const int N, const double *X, const int incX, double *Y, const int incY);

double mbcblas_ddot(const int N, const double *X, const int incX, const double *Y, const int incY);

double mbcblas_dnrm2(const int N, const double *X, const int incX);

void mbcblas_dscal(const int N, const double alpha, double *X, const int incX);

void mbcblas_dgemv(const enum MBCBLAS_ORDER order, const enum MBCBLAS_TRANSPOSE TransA, const int M, const int N,
                   const double alpha, const double *A, const int lda, const double *X, const int incX, const double beta,
                   double *Y, const int incY);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  /* MB_AUX_H_ */
