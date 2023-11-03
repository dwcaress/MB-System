/*--------------------------------------------------------------------
 *    The MB-system:  mbgrid.cc  5/2/94
 *
 *    Copyright (c) 1993-2023 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/**
  @file
 * mbgrid is a utility used to grid bathymetry, amplitude, or
 * sidescan data contained in a set of swath sonar data files.
 * This program uses one of four algorithms (gaussian weighted mean,
 * median filter, minimum filter, maximum filter) to grid regions
 * covered by swaths and then fills in gaps between
 * the swaths (to the degree specified by the user) using a minimum
 * curvature algorithm.
 *
 * The April 1995 version reinstated the use of the IGPP/SIO zgrid routine
 * for thin plate spline interpolation. The zgrid code has been
 * translated from Fortran to C. The zgrid algorithm is much
 * faster than the Wessel and Smith minimum curvature algorithm
 * from the GMT program surface used in recent versions of mbgrid.
 *
 * The July 2002 version allows the creation of grids using
 * UTM eastings and northings rather than uniformly spaced
 * in longitude and latitude.
 *
 * Author:  D. W. Caress
 * Date:  February 22, 1993
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <getopt.h>
#include <limits>
#include <unistd.h>

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_format.h"
#include "mb_info.h"
#include "mb_io.h"
#include "mb_status.h"

/* gridding algorithms */

typedef enum {
    MBGRID_WEIGHTED_MEAN = 1,
    MBGRID_MEDIAN_FILTER = 2,
    MBGRID_MINIMUM_FILTER = 3,
    MBGRID_MAXIMUM_FILTER = 4,
    MBGRID_WEIGHTED_FOOTPRINT_SLOPE = 5,
    MBGRID_WEIGHTED_FOOTPRINT = 6,
    MBGRID_MINIMUM_WEIGHTED_MEAN = 7,
    MBGRID_MAXIMUM_WEIGHTED_MEAN = 8,
} grid_alg_t;

/* grid format definitions */
typedef enum {
    MBGRID_ASCII = 1,
    MBGRID_OLDGRD = 2,
    MBGRID_CDFGRD = 3,
    MBGRID_ARCASCII = 4,
    MBGRID_GMTGRD = 100,
} grid_type_t;

/* gridded data type */

typedef enum {
    MBGRID_DATA_BATHYMETRY = 1,
    MBGRID_DATA_TOPOGRAPHY = 2,
    MBGRID_DATA_AMPLITUDE = 3,
    MBGRID_DATA_SIDESCAN = 4,
} grid_data_t;

/* flag for no data in grid */;
constexpr int NO_DATA_FLAG = 99999;

/* number of data to be allocated at a time */
constexpr int REALLOC_STEP_SIZE = 25;

/* usage of footprint based weight */
typedef enum {
    MBGRID_USE_NO = 0,
    MBGRID_USE_YES = 1,
    MBGRID_USE_CONDITIONAL = 2,
} grid_use_t;

/* interpolation mode */
typedef enum {
    MBGRID_INTERP_NONE = 0,
    MBGRID_INTERP_GAP = 1,
    MBGRID_INTERP_NEAR = 2,
    MBGRID_INTERP_ALL = 3,
} grid_interp_t;

/* comparison threshold */;
constexpr double MBGRID_TINY = 0.00000001;

/* maximum allowed beam grazing angle */
constexpr double FOOT_THETA_MAX = 85.0;

/* interpolation algorithm
    The code is set to use either of two
    algorithms for 2D thin plate spline
    interpolation. If the USESURFACE preprocessor
    define is defined then
    the code will use the surface algorithm
    from GMT. If not, then the zgrid
    algorithm will be used.
    - The default is to use zgrid - to
    change this uncomment the define below. */
/* #define USESURFACE */

/* output stream for basic stuff (stdout if verbose <= 1,
    stderr if verbose > 1) */
FILE *outfp = stdout;

/* program identifiers */
constexpr char program_name[] = "mbgrid";
constexpr char help_message[] =
    "mbgrid is an utility used to grid bathymetry, amplitude, or\n"
    "sidescan data contained in a set of swath sonar data files.\n"
    "This program uses one of four algorithms (gaussian weighted mean,\n"
    "median filter, minimum filter, maximum filter) to grid regions\n"
    "covered swaths and then fills in gaps between\n"
    "the swaths (to the degree specified by the user) using a minimum\n"
    "curvature algorithm.";
constexpr char usage_message[] =
    "mbgrid   -Ifilelist -Oroot [-Adatatype -Bborder -Cclip[/mode] -Dxdim/ydim\n"
    "          -Edx/dy/units[!]  -Fmode[/threshold] -Ggridkind -Jprojection\n"
    "          -Kbackground -Llonflip -M -N -Ppings -Q  -Rwest/east/south/north\n"
    "          -Rfactor  -Sspeed  -Ttension  -Utime  -V -Wscale -Xextend]";

/*--------------------------------------------------------------------*/
/* approximate error function altered from numerical recipes */
double mbgrid_erf(double x) {
  const double z = fabs(x);
  const double t = 1.0 / (1.0 + 0.5 * z);
  double erfc_d =
      t *
      exp(-z * z - 1.26551223 +
          t * (1.00002368 +
               t * (0.37409196 +
                    t * (0.09678418 +
                         t * (-0.18628806 +
                              t * (0.27886807 + t * (-1.13520398 + t * (1.48851587 + t * (-0.82215223 + t * 0.17087277)))))))));
  erfc_d = x >= 0.0 ? erfc_d : 2.0 - erfc_d;
  const double erf_d = 1.0 - erfc_d;
  return erf_d;
}

/*--------------------------------------------------------------------*/
/*
 * function write_ascii writes output grid to an ascii file
 */
int write_ascii(int verbose, char *outfile, float *grid, int nx, int ny, double xmin, double xmax, double ymin, double ymax,
                double dx, double dy, int *error) {
  if (verbose >= 2) {
    fprintf(outfp, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(outfp, "dbg2  Input arguments:\n");
    fprintf(outfp, "dbg2       verbose:    %d\n", verbose);
    fprintf(outfp, "dbg2       outfile:    %s\n", outfile);
    fprintf(outfp, "dbg2       grid:       %p\n", (void *)grid);
    fprintf(outfp, "dbg2       nx:         %d\n", nx);
    fprintf(outfp, "dbg2       ny:         %d\n", ny);
    fprintf(outfp, "dbg2       xmin:       %f\n", xmin);
    fprintf(outfp, "dbg2       xmax:       %f\n", xmax);
    fprintf(outfp, "dbg2       ymin:       %f\n", ymin);
    fprintf(outfp, "dbg2       ymax:       %f\n", ymax);
    fprintf(outfp, "dbg2       dx:         %f\n", dx);
    fprintf(outfp, "dbg2       dy:         %f\n", dy);
  }

  int status = MB_SUCCESS;

  /* open the file */
  FILE *fp = fopen(outfile, "w");
  if (fp == nullptr) {
    *error = MB_ERROR_OPEN_FAIL;
    status = MB_FAILURE;
  } else {
    /* output grid */
    fprintf(fp, "grid created by program MBGRID\n");
    char user[256], host[256], date[32];
    status = mb_user_host_date(verbose, user, host, date, error);
    fprintf(fp, "program run by %s on %s at %s\n", user, host, date);
    fprintf(fp, "%d %d\n%f %f %f %f\n", nx, ny, xmin, xmax, ymin, ymax);
    for (int i = 0; i < nx * ny; i++) {
      fprintf(fp, "%13.5g ", grid[i]);
      if ((i + 1) % 6 == 0)
        fprintf(fp, "\n");
    }
    if ((nx * ny) % 6 != 0)
      fprintf(fp, "\n");
    fclose(fp);
  }

  if (verbose >= 2) {
    fprintf(outfp, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(outfp, "dbg2  Return values:\n");
    fprintf(outfp, "dbg2       error:      %d\n", *error);
    fprintf(outfp, "dbg2  Return status:\n");
    fprintf(outfp, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
/*
 * function write_arcascii writes output grid to an Arc/Info ascii file
 */
int write_arcascii(int verbose, char *outfile, float *grid, int nx, int ny, double xmin, double xmax, double ymin, double ymax,
                   double dx, double dy, double nodata, int *error) {
  if (verbose >= 2) {
    fprintf(outfp, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(outfp, "dbg2  Input arguments:\n");
    fprintf(outfp, "dbg2       verbose:    %d\n", verbose);
    fprintf(outfp, "dbg2       outfile:    %s\n", outfile);
    fprintf(outfp, "dbg2       grid:       %p\n", (void *)grid);
    fprintf(outfp, "dbg2       nx:         %d\n", nx);
    fprintf(outfp, "dbg2       ny:         %d\n", ny);
    fprintf(outfp, "dbg2       xmin:       %f\n", xmin);
    fprintf(outfp, "dbg2       xmax:       %f\n", xmax);
    fprintf(outfp, "dbg2       ymin:       %f\n", ymin);
    fprintf(outfp, "dbg2       ymax:       %f\n", ymax);
    fprintf(outfp, "dbg2       dx:         %f\n", dx);
    fprintf(outfp, "dbg2       dy:         %f\n", dy);
    fprintf(outfp, "dbg2       nodata:     %f\n", nodata);
  }

  int status = MB_SUCCESS;

  /* open the file */
  FILE *fp = fopen(outfile, "w");
  if (fp == nullptr) {
    *error = MB_ERROR_OPEN_FAIL;
    status = MB_FAILURE;
  } else {
    /* output grid */
    fprintf(fp, "ncols %d\n", nx);
    fprintf(fp, "nrows %d\n", ny);
    fprintf(fp, "xllcorner %.10g\n", xmin - 0.5 * dx);
    fprintf(fp, "yllcorner %.10g\n", ymin - 0.5 * dy);
    fprintf(fp, "cellsize %.10g\n", dx);
    fprintf(fp, "nodata_value -99999\n");
    for (int j = 0; j < ny; j++) {
      for (int i = 0; i < nx; i++) {
        const int k = i * ny + (ny - 1 - j);
        if (grid[k] == nodata)
          fprintf(fp, "-99999 ");
        else
          fprintf(fp, "%f ", grid[k]);
      }
      fprintf(fp, "\n");
    }
    fclose(fp);
  }

  if (verbose >= 2) {
    fprintf(outfp, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(outfp, "dbg2  Return values:\n");
    fprintf(outfp, "dbg2       error:      %d\n", *error);
    fprintf(outfp, "dbg2  Return status:\n");
    fprintf(outfp, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
/*
 * function write_oldgrd writes output grid to a
 * GMT version 1 binary grd file
 */
int write_oldgrd(int verbose, char *outfile, float *grid, int nx, int ny, double xmin, double xmax, double ymin, double ymax,
                 double dx, double dy, int *error) {
  if (verbose >= 2) {
    fprintf(outfp, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(outfp, "dbg2  Input arguments:\n");
    fprintf(outfp, "dbg2       verbose:    %d\n", verbose);
    fprintf(outfp, "dbg2       outfile:    %s\n", outfile);
    fprintf(outfp, "dbg2       grid:       %p\n", (void *)grid);
    fprintf(outfp, "dbg2       nx:         %d\n", nx);
    fprintf(outfp, "dbg2       ny:         %d\n", ny);
    fprintf(outfp, "dbg2       xmin:       %f\n", xmin);
    fprintf(outfp, "dbg2       xmax:       %f\n", xmax);
    fprintf(outfp, "dbg2       ymin:       %f\n", ymin);
    fprintf(outfp, "dbg2       ymax:       %f\n", ymax);
    fprintf(outfp, "dbg2       dx:         %f\n", dx);
    fprintf(outfp, "dbg2       dy:         %f\n", dy);
  }

  int status = MB_SUCCESS;

  /* open the file */
  FILE *fp = fopen(outfile, "w");
  if (fp == nullptr) {
    *error = MB_ERROR_OPEN_FAIL;
    status = MB_FAILURE;
  } else {
    /* output grid */
    fwrite((char *)&nx, 1, 4, fp);
    fwrite((char *)&ny, 1, 4, fp);
    fwrite((char *)&xmin, 1, 8, fp);
    fwrite((char *)&xmax, 1, 8, fp);
    fwrite((char *)&ymin, 1, 8, fp);
    fwrite((char *)&ymax, 1, 8, fp);
    fwrite((char *)&dx, 1, 8, fp);
    fwrite((char *)&dy, 1, 8, fp);
    fwrite((char *)grid, nx * ny, 4, fp);
    fclose(fp);
  }

  if (verbose >= 2) {
    fprintf(outfp, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(outfp, "dbg2  Return values:\n");
    fprintf(outfp, "dbg2       error:      %d\n", *error);
    fprintf(outfp, "dbg2  Return status:\n");
    fprintf(outfp, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
/*
 * function mbgrid_weight calculates the integrated weight over a bin
 * given the footprint of a sounding
 */
int mbgrid_weight(int verbose, double foot_a, double foot_b, double pcx, double pcy, double dx, double dy, double *px, double *py,
                  double *weight, grid_use_t *use, int *error) {
  if (verbose >= 2) {
    fprintf(outfp, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(outfp, "dbg2  Input arguments:\n");
    fprintf(outfp, "dbg2       verbose:    %d\n", verbose);
    fprintf(outfp, "dbg2       foot_a:     %f\n", foot_a);
    fprintf(outfp, "dbg2       foot_b:     %f\n", foot_b);
    fprintf(outfp, "dbg2       pcx:        %f\n", pcx);
    fprintf(outfp, "dbg2       pcy:        %f\n", pcy);
    fprintf(outfp, "dbg2       dx:         %f\n", dx);
    fprintf(outfp, "dbg2       dy:         %f\n", dy);
    fprintf(outfp, "dbg2       p1 x:       %f\n", px[0]);
    fprintf(outfp, "dbg2       p1 y:       %f\n", py[0]);
    fprintf(outfp, "dbg2       p2 x:       %f\n", px[1]);
    fprintf(outfp, "dbg2       p2 y:       %f\n", py[1]);
    fprintf(outfp, "dbg2       p3 x:       %f\n", px[2]);
    fprintf(outfp, "dbg2       p3 y:       %f\n", py[2]);
    fprintf(outfp, "dbg2       p4 x:       %f\n", px[3]);
    fprintf(outfp, "dbg2       p4 y:       %f\n", py[3]);
  }

  /* The weighting function is
      w(x, y) = (1 / (PI * a * b)) * exp(-(x**2/a**2 + y**2/b**2))
      in the footprint coordinate system, where the x axis
      is along the horizontal projection of the beam and the
      y axix is perpendicular to that. The integral of the
      weighting function over an simple rectangle defined
      by corners (x1, y1), (x2, y1), (x1, y2), (x2, y2) is
          x2 y2
      W = I  I { w(x, y) } dx dy
          x1 y1

        = 1 / 4 * ( erfc(x1/a) - erfc(x2/a)) * ( erfc(y1/a) - erfc(y2/a))
      where erfc(u) is the complementary error function.
      Each bin is represented as a simple integral in geographic
      coordinates, but is rotated in the footprint coordinate system.
      I can't figure out how to evaluate this integral over a
      rotated rectangle,  and so I am crudely and incorrectly
      approximating the integrated weight value by evaluating it over
      the same sized rectangle centered at the same location.
      Maybe someday I'll figure out how to do it correctly.
      DWC 11/18/99 */

  /* get integrated weight */
  const double fa = foot_a;
  const double fb = foot_b;
  *weight = 0.25 * (mbgrid_erf((pcx + dx) / fa) - mbgrid_erf((pcx - dx) / fa)) *
            (mbgrid_erf((pcy + dy) / fb) - mbgrid_erf((pcy - dy) / fb));

  /* use if weight large or any ratio <= 1 */
  if (*weight > 0.05) {
    *use = MBGRID_USE_YES;
  } else {
    /* check ratio of each corner footprint 1/e distance */
    *use = MBGRID_USE_NO;
    for (int i = 0; i < 4; i++) {
      const double ang = RTD * atan2(py[i], px[i]);
      const double xe = foot_a * cos(DTR * ang);
      const double ye = foot_b * sin(DTR * ang);
      const double ratio = sqrt((px[i] * px[i] + py[i] * py[i]) / (xe * xe + ye * ye));
      if (ratio <= 1.0)
        *use = MBGRID_USE_YES;
      else if (ratio <= 2.0)
        *use = MBGRID_USE_CONDITIONAL;
    }
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(outfp, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(outfp, "dbg2  Return values:\n");
    fprintf(outfp, "dbg2       error:      %d\n", *error);
    fprintf(outfp, "dbg2       weight:     %f\n", *weight);
    fprintf(outfp, "dbg2       use:        %d\n", *use);
    fprintf(outfp, "dbg2  Return status:\n");
    fprintf(outfp, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
  int verbose = 0;
  int format;
  int pings;
  int lonflip;
  double bounds[4];
  int btime_i[7];
  int etime_i[7];
  double speedmin;
  double timegap;
  int status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

  grid_data_t datatype = MBGRID_DATA_BATHYMETRY;
  double border = 0.0;
  bool setborder = false;
  char gridkindstring[MB_PATH_MAXLINE] = "";
  int clip = 0;
  int xdim = 101;
  int ydim = 101;
  char fileroot[MB_PATH_MAXLINE] = "grid";
  char projection_id[MB_PATH_MAXLINE] = "Geographic";
  double gbnd[4] = {0.0, 0.0, 0.0, 0.0};
  bool gbndset = false;
  double scale = 1.0;
  double extend = 0.0;
  double shift_x = 0.0;
  double shift_y = 0.0;
  bool shift = false;
  bool first_in_stays = true;
  bool check_time = false;
  double timediff = 300.0;
#ifdef USESURFACE
  double tension = 0.35;
#else
  double tension = 0.0;
#endif

  double boundsfactor = 0.0;
  bool bathy_in_feet = false;
  bool more = false;
  bool use_NaN = false;
  int grdrasterid = 0;
  char projection_pars[MB_PATH_MAXLINE] = "";
  bool projection_pars_f = false;
  char filelist[MB_PATH_MAXLINE] = "datalist.mb-1";
  char backgroundfile[MB_PATH_MAXLINE] = "";
  grid_type_t gridkind = MBGRID_GMTGRD;
  double minormax_weighted_mean_threshold = 1.0;
  grid_alg_t grid_mode = MBGRID_WEIGHTED_MEAN;
  bool set_spacing = false;
  char units[MB_PATH_MAXLINE] = "";
  double dx_set = 0.0;
  double dy_set = 0.0;
  bool spacing_priority = false;
  bool set_dimensions = false;
  grid_interp_t clipmode = MBGRID_INTERP_NONE;

  {
    bool errflg = false;
    int c;
    bool help = false;
    while ((c = getopt(argc, argv, "A:a:B:b:C:c:D:d:E:e:F:f:G:g:HhI:i:J:j:K:k:L:l:MmNnO:o:P:p:QqR:r:S:s:T:t:U:u:VvW:w:X:x:Y:y:")) !=
           -1)
    {
      switch (c) {
      case 'A':
      case 'a':
      {
        int tmp;
        sscanf(optarg, "%d", &tmp);
        datatype = (grid_data_t)tmp;
        break;
      }
      case 'B':
      case 'b':
        sscanf(optarg, "%lf", &border);
        setborder = true;
        break;
      case 'C':
      case 'c':
      {
        const int n = sscanf(optarg, "%d/%d", &clip, (int *)&clipmode);
        if (n < 1)
          clipmode = MBGRID_INTERP_NONE;
        else if (n == 1 && clip > 0)
          clipmode = MBGRID_INTERP_GAP;
        else if (n == 1)
          clipmode = MBGRID_INTERP_NONE;
        else if (clip > 0 && clipmode < 0)
          clipmode = MBGRID_INTERP_GAP;
        else if (clipmode >= 3)
          clipmode = MBGRID_INTERP_ALL;
        break;
      }
      case 'D':
      case 'd':
      {
        const int n = sscanf(optarg, "%d/%d", &xdim, &ydim);
        if (n == 2)
          set_dimensions = true;
        break;
      }
      case 'E':
      case 'e':
      {
        if (optarg[strlen(optarg) - 1] == '!') {
          spacing_priority = true;
          optarg[strlen(optarg) - 1] = '\0';
        }
        const int n = sscanf(optarg, "%lf/%lf/%1023s", &dx_set, &dy_set, units);
        if (n > 1)
          set_spacing = true;
        if (n < 3)
          strcpy(units, "meters");
        break;
      }
      case 'F':
      case 'f':
      {
        int tmp;
        double dvalue;
        const int n = sscanf(optarg, "%d/%lf", &tmp, &dvalue);
        grid_mode = (grid_alg_t)tmp;
        if (n == 2) {
          if (grid_mode == MBGRID_MINIMUM_FILTER) {
            minormax_weighted_mean_threshold = dvalue;
            grid_mode = MBGRID_MINIMUM_WEIGHTED_MEAN;
          } else if (grid_mode == MBGRID_MAXIMUM_FILTER) {
            minormax_weighted_mean_threshold = dvalue;
            grid_mode = MBGRID_MAXIMUM_WEIGHTED_MEAN;
          } else {
            minormax_weighted_mean_threshold = dvalue;
          }
        }
        break;
      }
      case 'G':
      case 'g':
        if (optarg[0] == '=') {
          gridkind = MBGRID_GMTGRD;
          strcpy(gridkindstring, optarg);
        }
        else {
          int tmp;
          int nscan = sscanf(optarg, "%d", &tmp);
          // Range check
          if (nscan == 1 && tmp >= 1 && tmp <= 4) {
            gridkind = (grid_type_t)tmp;
            if (gridkind == MBGRID_CDFGRD) {
              gridkind = MBGRID_GMTGRD;
              gridkindstring[0] = '\0';
            }
          } else if (optarg[0] == 'n' || optarg[0] == 'c' || optarg[0] == 'b'
                    || optarg[0] == 'r' || optarg[0] == 's' || optarg[0] == 'a'
                    || optarg[0] == 'e' || optarg[0] == 'g'){
            snprintf(gridkindstring, sizeof(gridkindstring), "=%s", optarg);
            gridkind = MBGRID_GMTGRD;
          } else {
            fprintf(stdout, "Invalid gridkind option: -G%s\n\n", optarg);
            errflg = true;
          }
        }
        break;
      case 'H':
      case 'h':
        help = true;
        break;
      case 'I':
      case 'i':
        sscanf(optarg, "%1023s", filelist);
        break;
      case 'J':
      case 'j':
        sscanf(optarg, "%1023s", projection_pars);
        projection_pars_f = true;
        break;
      case 'K':
      case 'k':
        sscanf(optarg, "%1023s", backgroundfile);
        if ((grdrasterid = (int)strtol(backgroundfile, NULL, 10)) <= 0)
          grdrasterid = -1;
        break;
      case 'L':
      case 'l':
        sscanf(optarg, "%d", &lonflip);
        break;
      case 'M':
      case 'm':
        more = true;
        break;
      case 'N':
      case 'n':
        use_NaN = true;
        break;
      case 'O':
      case 'o':
        sscanf(optarg, "%1023s", fileroot);
        break;
      case 'P':
      case 'p':
        sscanf(optarg, "%d", &pings);
        break;
      case 'Q':
      case 'q':
        bathy_in_feet = true;
        break;
      case 'R':
      case 'r':
        if (strchr(optarg, '/') == nullptr) {
          sscanf(optarg, "%lf", &boundsfactor);
          if (boundsfactor <= 1.0)
            boundsfactor = 0.0;
        }
        else {
          mb_get_bounds(optarg, gbnd);
          gbndset = true;
        }
        break;
      case 'S':
      case 's':
        sscanf(optarg, "%lf", &speedmin);
        break;
      case 'T':
      case 't':
        sscanf(optarg, "%lf", &tension);
        break;
      case 'U':
      case 'u':
        sscanf(optarg, "%lf", &timediff);
        timediff = 60 * timediff;
        check_time = true;
        if (timediff < 0.0) {
          timediff = fabs(timediff);
          first_in_stays = false;
        }
        break;
      case 'V':
      case 'v':
        verbose++;
        if (verbose >= 2)
          outfp = stderr;
        break;
      case 'W':
      case 'w':
        sscanf(optarg, "%lf", &scale);
        break;
      case 'X':
      case 'x':
        sscanf(optarg, "%lf", &extend);
        break;
      case 'Y':
      case 'y':
        if (int n = sscanf(optarg, "%lf/%lf", &shift_x, &shift_y) == 2) {
          shift = true;
        }
        break;
      case '?':
        errflg = true;
      }
    }

    if (errflg) {
      fprintf(outfp, "usage: %s\n", usage_message);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_BAD_USAGE);
    }

    if (verbose == 1 || help) {
      fprintf(outfp, "\nProgram %s\n", program_name);
      fprintf(outfp, "MB-system Version %s\n", MB_VERSION);
    }

    if (verbose >= 2) {
      fprintf(outfp, "\ndbg2  Program <%s>\n", program_name);
      fprintf(outfp, "dbg2  MB-system Version %s\n", MB_VERSION);
      fprintf(outfp, "dbg2  Control Parameters:\n");
      fprintf(outfp, "dbg2       verbose:              %d\n", verbose);
      fprintf(outfp, "dbg2       help:                 %d\n", help);
      fprintf(outfp, "dbg2       pings:                %d\n", pings);
      fprintf(outfp, "dbg2       lonflip:              %d\n", lonflip);
      fprintf(outfp, "dbg2       btime_i[0]:           %d\n", btime_i[0]);
      fprintf(outfp, "dbg2       btime_i[1]:           %d\n", btime_i[1]);
      fprintf(outfp, "dbg2       btime_i[2]:           %d\n", btime_i[2]);
      fprintf(outfp, "dbg2       btime_i[3]:           %d\n", btime_i[3]);
      fprintf(outfp, "dbg2       btime_i[4]:           %d\n", btime_i[4]);
      fprintf(outfp, "dbg2       btime_i[5]:           %d\n", btime_i[5]);
      fprintf(outfp, "dbg2       btime_i[6]:           %d\n", btime_i[6]);
      fprintf(outfp, "dbg2       etime_i[0]:           %d\n", etime_i[0]);
      fprintf(outfp, "dbg2       etime_i[1]:           %d\n", etime_i[1]);
      fprintf(outfp, "dbg2       etime_i[2]:           %d\n", etime_i[2]);
      fprintf(outfp, "dbg2       etime_i[3]:           %d\n", etime_i[3]);
      fprintf(outfp, "dbg2       etime_i[4]:           %d\n", etime_i[4]);
      fprintf(outfp, "dbg2       etime_i[5]:           %d\n", etime_i[5]);
      fprintf(outfp, "dbg2       etime_i[6]:           %d\n", etime_i[6]);
      fprintf(outfp, "dbg2       speedmin:             %f\n", speedmin);
      fprintf(outfp, "dbg2       timegap:              %f\n", timegap);
      fprintf(outfp, "dbg2       file list:            %s\n", filelist);
      fprintf(outfp, "dbg2       output file root:     %s\n", fileroot);
      fprintf(outfp, "dbg2       grid x dimension:     %d\n", xdim);
      fprintf(outfp, "dbg2       grid y dimension:     %d\n", ydim);
      fprintf(outfp, "dbg2       grid bounds[0]:       %f\n", gbnd[0]);
      fprintf(outfp, "dbg2       grid bounds[1]:       %f\n", gbnd[1]);
      fprintf(outfp, "dbg2       grid bounds[2]:       %f\n", gbnd[2]);
      fprintf(outfp, "dbg2       grid bounds[3]:       %f\n", gbnd[3]);
      fprintf(outfp, "dbg2       boundsfactor:         %f\n", boundsfactor);
      fprintf(outfp, "dbg2       clipmode:             %d\n", clipmode);
      fprintf(outfp, "dbg2       clip:                 %d\n", clip);
      fprintf(outfp, "dbg2       tension:              %f\n", tension);
      fprintf(outfp, "dbg2       grdraster background: %d\n", grdrasterid);
      fprintf(outfp, "dbg2       backgroundfile:       %s\n", backgroundfile);
      fprintf(outfp, "dbg2       more:                 %d\n", more);
      fprintf(outfp, "dbg2       use_NaN:              %d\n", use_NaN);
      fprintf(outfp, "dbg2       grid_mode:            %d\n", grid_mode);
      fprintf(outfp, "dbg2       data type:            %d\n", datatype);
      fprintf(outfp, "dbg2       grid format:          %d\n", gridkind);
      if (gridkind == MBGRID_GMTGRD)
        fprintf(outfp, "dbg2       gmt grid format id:   %s\n", gridkindstring);
      fprintf(outfp, "dbg2       scale:                %f\n", scale);
      fprintf(outfp, "dbg2       timediff:             %f\n", timediff);
      fprintf(outfp, "dbg2       setborder:            %d\n", setborder);
      fprintf(outfp, "dbg2       border:               %f\n", border);
      fprintf(outfp, "dbg2       extend:               %f\n", extend);
      fprintf(outfp, "dbg2       shift:                %d\n", shift);
      fprintf(outfp, "dbg2       shift_x:              %f\n", shift_x);
      fprintf(outfp, "dbg2       shift_y:              %f\n", shift_y);
      fprintf(outfp, "dbg2       bathy_in_feet:        %d\n", bathy_in_feet);
      fprintf(outfp, "dbg2       projection_pars:      %s\n", projection_pars);
      fprintf(outfp, "dbg2       proj flag 1:          %d\n", projection_pars_f);
      fprintf(outfp, "dbg2       projection_id:        %s\n", projection_id);
      // fprintf(outfp, "dbg2       utm_zone:             %d\n", utm_zone);
      fprintf(outfp, "dbg2       minormax_weighted_mean_threshold: %f\n", minormax_weighted_mean_threshold);

    }

    if (help) {
      fprintf(outfp, "\n%s\n", help_message);
      fprintf(outfp, "\nusage: %s\n", usage_message);
      exit(MB_ERROR_NO_ERROR);
    }
  }

  int error = MB_ERROR_NO_ERROR;
  int memclear_error = MB_ERROR_NO_ERROR;

  /* if bounds not set get bounds of input data */
  if (!gbndset || (!set_spacing && !set_dimensions)) {
    struct mb_info_struct mb_info;
    int formatread = -1;
    status = mb_get_info_datalist(verbose, filelist, &formatread, &mb_info, lonflip, &error);

    if (!gbndset) {
      gbnd[0] = mb_info.lon_min;
      gbnd[1] = mb_info.lon_max;
      gbnd[2] = mb_info.lat_min;
      gbnd[3] = mb_info.lat_max;
      // gbndset = true;
    }

    if (!set_spacing && !set_dimensions) {
      dx_set = 0.02 * mb_info.altitude_max;
      dy_set = 0.02 * mb_info.altitude_max;
      set_spacing = true;
      strcpy(units, "meters");
    }
  }

  double btime_d;
  double etime_d;
  int beams_bath;
  int beams_amp;
  int pixels_ss;
  char file[MB_PATH_MAXLINE] = "";
  void *mbio_ptr = nullptr;
  struct mb_io_struct *mb_io_ptr = nullptr;
  int topo_type;

  /* mbgrid control variables */
  void *datalist;
  double file_weight;
  double dx = 0.0;
  double dy = 0.0;
  double clipvalue = NO_DATA_FLAG;
  float outclipvalue = NO_DATA_FLAG;
  int rformat;
  int pstatus;
  int astatus = MB_ALTNAV_NONE;
  char path[MB_PATH_MAXLINE] = "";
  char ppath[MB_PATH_MAXLINE] = "";
  char apath[MB_PATH_MAXLINE] = "";
  char dpath[MB_PATH_MAXLINE] = "";
  char rfile[MB_PATH_MAXLINE] = "";
  char ofile[2*MB_PATH_MAXLINE+100] = "";
  char dfile[MB_PATH_MAXLINE] = "";
  char plot_cmd[(8*1024)] = "";
  int plot_status;

  /* mbio read values */
  int rpings;
  int kind;
  int time_i[7];
  double time_d;
  double navlon;
  double navlat;
  double speed;
  double heading;
  double distance;
  double altitude;
  double sensordepth;
  char *beamflag = nullptr;
  double *bath = nullptr;
  double *bathlon = nullptr;
  double *bathlat = nullptr;
  double *amp = nullptr;
  double *ss = nullptr;
  double *sslon = nullptr;
  double *sslat = nullptr;
  char comment[MB_COMMENT_MAXLINE];

  /* lon,lat,value triples variables */
  double tlon;
  double tlat;
  double tvalue;

  /* grid variables */
  double wbnd[4], obnd[4];
  double xlon, ylat, xx, yy;
  double factor, weight, topofactor;
  int offx, offy, xtradim;
  double *grid = nullptr;
  double *norm = nullptr;
  double *sigma = nullptr;
  double *firsttime = nullptr;
  double *gridsmall = nullptr;
  double *minormax = nullptr;
#ifdef USESURFACE
  float *bxdata = nullptr;
  float *bydata = nullptr;
  float *bzdata = nullptr;
  float *sxdata = nullptr;
  float *sydata = nullptr;
  float *szdata = nullptr;
#else
  float *bdata = nullptr;
  float *sdata = nullptr;
  float *work1 = nullptr;
  int *work2 = nullptr;
  bool *work3 = nullptr;
#endif
  double bdata_origin_x, bdata_origin_y;
  float *output = nullptr;
  float *sgrid = nullptr;
  int *cnt = nullptr;
  int *num = nullptr;
  double **data;
  double *value = nullptr;
  int ndata, ndatafile, nbackground;
  double zmin, zmax, zclip;
  int nmax;
  double smin, smax;
  int nbinset, nbinzero, nbinspline, nbinbackground;

  /* projected grid parameters */
  void *pjptr;
  double deglontokm, deglattokm;
  double mtodeglon, mtodeglat;

  /* output char strings */
  char xlabel[MB_PATH_MAXLINE+20] = "";
  char ylabel[MB_PATH_MAXLINE+20] = "";
  char zlabel[MB_PATH_MAXLINE+20] = "";
  char title[MB_PATH_MAXLINE] = "";
  char nlabel[MB_PATH_MAXLINE] = "";
  char sdlabel[MB_PATH_MAXLINE] = "";

  /* other variables */
  FILE *dfp = nullptr;
  FILE *rfp = nullptr;
  int kgrid, kout, kint, ib, ix, iy;
  int ix1, ix2, iy1, iy2;

  double foot_dx, foot_dy, foot_dxn, foot_dyn;
  double foot_lateral, beam_altitude, foot_range, foot_theta;
  double foot_dtheta, foot_dphi;
  double foot_hwidth, foot_hlength;
  int foot_wix, foot_wiy, foot_lix, foot_liy, foot_dix, foot_diy;
  double sbath;
  double xx0, yy0, bdx, bdy, xx1, xx2, yy1, yy2;
  double prx[5], pry[5];
  grid_use_t use_weight;

  int gxdim = 0;
  int gydim = 0;

  /* if requested expand the grid bounds */
  if (boundsfactor > 1.0) {
    xx1 = 0.5 * (boundsfactor - 1.0) * (gbnd[1] - gbnd[0]);
    yy1 = 0.5 * (boundsfactor - 1.0) * (gbnd[3] - gbnd[2]);
    gbnd[0] -= xx1;
    gbnd[1] += xx1;
    gbnd[2] -= yy1;
    gbnd[3] += yy1;
  }

  /* if bounds not specified then quit */
  if (gbnd[0] >= gbnd[1] || gbnd[2] >= gbnd[3]) {
    fprintf(outfp, "\nGrid bounds not properly specified:\n\t%f %f %f %f\n", gbnd[0], gbnd[1], gbnd[2], gbnd[3]);
    fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
    exit(MB_ERROR_BAD_PARAMETER);
  }

  /* footprint option only for bathymetry */
  if ((grid_mode == MBGRID_WEIGHTED_FOOTPRINT_SLOPE || grid_mode == MBGRID_WEIGHTED_FOOTPRINT) &&
      (datatype != MBGRID_DATA_TOPOGRAPHY && datatype != MBGRID_DATA_BATHYMETRY)) {
    grid_mode = MBGRID_WEIGHTED_MEAN;
  }

  /* more option not available with minimum
      or maximum filter algorithms */
  if (more && (grid_mode == MBGRID_MINIMUM_FILTER || grid_mode == MBGRID_MAXIMUM_FILTER))
    more = false;

  /* NaN cannot be used for ASCII grids */
  if (use_NaN && (gridkind == MBGRID_ASCII || gridkind == MBGRID_ARCASCII))
    use_NaN = false;

  /* define NaN in case it's needed */
  if (use_NaN) {
    outclipvalue = std::numeric_limits<float>::quiet_NaN();
  }

  bool use_projection = false;

  /* deal with projected gridding */
  if (projection_pars_f) {
    /* check for UTM with undefined zone */
    if (strcmp(projection_pars, "UTM") == 0 || strcmp(projection_pars, "U") == 0 || strcmp(projection_pars, "utm") == 0 ||
        strcmp(projection_pars, "u") == 0) {
      double reference_lon = 0.5 * (gbnd[0] + gbnd[1]);
      if (reference_lon < 180.0)
        reference_lon += 360.0;
      if (reference_lon >= 180.0)
        reference_lon -= 360.0;
      const int utm_zone = (int)(((reference_lon + 183.0) / 6.0) + 0.5);
      double reference_lat = 0.5 * (gbnd[2] + gbnd[3]);
      if (reference_lat >= 0.0)
        snprintf(projection_id, sizeof(projection_id), "UTM%2.2dN", utm_zone);
      else
        snprintf(projection_id, sizeof(projection_id), "UTM%2.2dS", utm_zone);
    }
    else
      strcpy(projection_id, projection_pars);

    /* set projection flag */
    use_projection = true;
    const int proj_status = mb_proj_init(verbose, projection_id, &(pjptr), &error);

    /* if projection not successfully initialized then quit */
    if (proj_status != MB_SUCCESS) {
      fprintf(outfp, "\nOutput projection %s not found in database\n", projection_id);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      mb_memory_clear(verbose, &memclear_error);
      exit(MB_ERROR_BAD_PARAMETER);
    }

    /* translate lon lat bounds from UTM if required */
    if (gbnd[0] < -360.0 || gbnd[0] > 360.0 || gbnd[1] < -360.0 || gbnd[1] > 360.0 || gbnd[2] < -90.0 || gbnd[2] > 90.0 ||
        gbnd[3] < -90.0 || gbnd[3] > 90.0) {
      /* first point */
      xx = gbnd[0];
      yy = gbnd[2];
      mb_proj_inverse(verbose, pjptr, xx, yy, &xlon, &ylat, &error);
      mb_apply_lonflip(verbose, lonflip, &xlon);
      obnd[0] = xlon;
      obnd[1] = xlon;
      obnd[2] = ylat;
      obnd[3] = ylat;

      /* second point */
      xx = gbnd[1];
      yy = gbnd[2];
      mb_proj_inverse(verbose, pjptr, xx, yy, &xlon, &ylat, &error);
      mb_apply_lonflip(verbose, lonflip, &xlon);
      obnd[0] = std::min(obnd[0], xlon);
      obnd[1] = std::max(obnd[1], xlon);
      obnd[2] = std::min(obnd[2], ylat);
      obnd[3] = std::max(obnd[3], ylat);

      /* third point */
      xx = gbnd[0];
      yy = gbnd[3];
      mb_proj_inverse(verbose, pjptr, xx, yy, &xlon, &ylat, &error);
      mb_apply_lonflip(verbose, lonflip, &xlon);
      obnd[0] = std::min(obnd[0], xlon);
      obnd[1] = std::max(obnd[1], xlon);
      obnd[2] = std::min(obnd[2], ylat);
      obnd[3] = std::max(obnd[3], ylat);

      /* fourth point */
      xx = gbnd[1];
      yy = gbnd[3];
      mb_proj_inverse(verbose, pjptr, xx, yy, &xlon, &ylat, &error);
      mb_apply_lonflip(verbose, lonflip, &xlon);
      obnd[0] = std::min(obnd[0], xlon);
      obnd[1] = std::max(obnd[1], xlon);
      obnd[2] = std::min(obnd[2], ylat);
      obnd[3] = std::max(obnd[3], ylat);
    }

    /* else translate bounds to UTM */
    else {
      /* copy gbnd to obnd */
      obnd[0] = gbnd[0];
      obnd[1] = gbnd[1];
      obnd[2] = gbnd[2];
      obnd[3] = gbnd[3];

      /* first point */
      xlon = obnd[0];
      ylat = obnd[2];
      mb_proj_forward(verbose, pjptr, xlon, ylat, &xx, &yy, &error);
      gbnd[0] = xx;
      gbnd[1] = xx;
      gbnd[2] = yy;
      gbnd[3] = yy;

      /* second point */
      xlon = obnd[1];
      ylat = obnd[2];
      mb_proj_forward(verbose, pjptr, xlon, ylat, &xx, &yy, &error);
      gbnd[0] = std::min(gbnd[0], xx);
      gbnd[1] = std::max(gbnd[1], xx);
      gbnd[2] = std::min(gbnd[2], yy);
      gbnd[3] = std::max(gbnd[3], yy);

      /* third point */
      xlon = obnd[0];
      ylat = obnd[3];
      mb_proj_forward(verbose, pjptr, xlon, ylat, &xx, &yy, &error);
      gbnd[0] = std::min(gbnd[0], xx);
      gbnd[1] = std::max(gbnd[1], xx);
      gbnd[2] = std::min(gbnd[2], yy);
      gbnd[3] = std::max(gbnd[3], yy);

      /* fourth point */
      xlon = obnd[1];
      ylat = obnd[3];
      mb_proj_forward(verbose, pjptr, xlon, ylat, &xx, &yy, &error);
      gbnd[0] = std::min(gbnd[0], xx);
      gbnd[1] = std::max(gbnd[1], xx);
      gbnd[2] = std::min(gbnd[2], yy);
      gbnd[3] = std::max(gbnd[3], yy);
    }

    /* get local scaling of lon lat */
    mb_coor_scale(verbose, 0.5 * (obnd[2] + obnd[3]), &mtodeglon, &mtodeglat);
    deglontokm = 0.001 / mtodeglon;
    deglattokm = 0.001 / mtodeglat;

    /* calculate grid properties */
    if (set_spacing) {
      xdim = lrint((gbnd[1] - gbnd[0]) / dx_set + 1);
      if (dy_set <= 0.0)
        dy_set = dx_set;
      ydim = lrint((gbnd[3] - gbnd[2]) / dy_set + 1);
      if (spacing_priority) {
        gbnd[1] = gbnd[0] + dx_set * (xdim - 1);
        gbnd[3] = gbnd[2] + dy_set * (ydim - 1);
      }
      if (units[0] == 'M' || units[0] == 'm')
        strcpy(units, "meters");
      else if (units[0] == 'K' || units[0] == 'k')
        strcpy(units, "km");
      else if (units[0] == 'F' || units[0] == 'f')
        strcpy(units, "feet");
      else if (strncmp(units, "arcmin", 6) == 0) {
        dx_set = dx_set / 60.0;
        dy_set = dy_set / 60.0;
        strcpy(units, "degrees");
      }
      else if (strncmp(units, "arcsec", 6) == 0) {
        dx_set = dx_set / 3600.0;
        dy_set = dy_set / 3600.0;
        strcpy(units, "degrees");
      }
      else
        strcpy(units, "unknown");
    }
  }

  /* deal with no projection */
  else {
    /* get local scaling of lon lat */
    mb_coor_scale(verbose, 0.5 * (gbnd[2] + gbnd[3]), &mtodeglon, &mtodeglat);
    deglontokm = 0.001 / mtodeglon;
    deglattokm = 0.001 / mtodeglat;

    /* calculate grid properties */
    if (set_spacing && (units[0] == 'M' || units[0] == 'm')) {
      xdim = lrint((gbnd[1] - gbnd[0]) / (mtodeglon * dx_set) + 1);
      if (dy_set <= 0.0)
        dy_set = mtodeglon * dx_set / mtodeglat;
      ydim = lrint((gbnd[3] - gbnd[2]) / (mtodeglat * dy_set) + 1);
      if (spacing_priority) {
        gbnd[1] = gbnd[0] + mtodeglon * dx_set * (xdim - 1);
        gbnd[3] = gbnd[2] + mtodeglat * dy_set * (ydim - 1);
      }
      strcpy(units, "meters");
    }
    else if (set_spacing && (units[0] == 'K' || units[0] == 'k')) {
      xdim = lrint((gbnd[1] - gbnd[0]) * deglontokm / dx_set + 1);
      if (dy_set <= 0.0)
        dy_set = deglattokm * dx_set / deglontokm;
      ydim = lrint((gbnd[3] - gbnd[2]) * deglattokm / dy_set + 1);
      if (spacing_priority) {
        gbnd[1] = gbnd[0] + dx_set * (xdim - 1) / deglontokm;
        gbnd[3] = gbnd[2] + dy_set * (ydim - 1) / deglattokm;
      }
      strcpy(units, "km");
    }
    else if (set_spacing && (units[0] == 'F' || units[0] == 'f')) {
      xdim = lrint((gbnd[1] - gbnd[0]) / (mtodeglon * 0.3048 * dx_set) + 1);
      if (dy_set <= 0.0)
        dy_set = mtodeglon * dx_set / mtodeglat;
      ydim = lrint((gbnd[3] - gbnd[2]) / (mtodeglat * 0.3048 * dy_set) + 1);
      if (spacing_priority) {
        gbnd[1] = gbnd[0] + mtodeglon * 0.3048 * dx_set * (xdim - 1);
        gbnd[3] = gbnd[2] + mtodeglat * 0.3048 * dy_set * (ydim - 1);
      }
      strcpy(units, "feet");
    }
    else if (set_spacing) {
      if (strncmp(units, "arcmin", 6) == 0) {
        dx_set = dx_set / 60.0;
        dy_set = dy_set / 60.0;
        strcpy(units, "degrees");
      }
      else if (strncmp(units, "arcsec", 6) == 0) {
        dx_set = dx_set / 3600.0;
        dy_set = dy_set / 3600.0;
        strcpy(units, "degrees");
      }
      else
        strcpy(units, "degrees");
      xdim = lrint((gbnd[1] - gbnd[0]) / dx_set + 1);
      if (dy_set <= 0.0)
        dy_set = dx_set;
      ydim = lrint((gbnd[3] - gbnd[2]) / dy_set + 1);
      if (spacing_priority) {
        gbnd[1] = gbnd[0] + dx_set * (xdim - 1);
        gbnd[3] = gbnd[2] + dy_set * (ydim - 1);
      }
    }
  }

  /* calculate other grid properties */
  dx = (gbnd[1] - gbnd[0]) / (xdim - 1);
  dy = (gbnd[3] - gbnd[2]) / (ydim - 1);
  factor = 4.0 / (scale * scale * dx * dy);
  offx = 0;
  offy = 0;
  if (extend > 0.0) {
    offx = (int)(extend * xdim);
    offy = (int)(extend * ydim);
  }
  xtradim = scale + 2;
  gxdim = xdim + 2 * offx;
  gydim = ydim + 2 * offy;
  wbnd[0] = gbnd[0] - offx * dx;
  wbnd[1] = gbnd[1] + offx * dx;
  wbnd[2] = gbnd[2] - offy * dy;
  wbnd[3] = gbnd[3] + offy * dy;
  if (datatype == MBGRID_DATA_TOPOGRAPHY)
    topofactor = -1.0;
  else
    topofactor = 1.0;
  if (bathy_in_feet && (datatype == MBGRID_DATA_TOPOGRAPHY || datatype == MBGRID_DATA_BATHYMETRY))
    topofactor = topofactor / 0.3048;

  /* check that dx == dy for Arc ascii grid output */
  if (gridkind == MBGRID_ARCASCII && fabs(dx - dy) > MBGRID_TINY) {
    fprintf(outfp, "\nArc Ascii grid output (-G4) requires square cells, but grid intervals dx:%f dy:%f differ...\n", dx, dy);
    fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
    exit(MB_ERROR_BAD_PARAMETER);
  }

  /* get data input bounds in lon lat */
  if (!use_projection) {
    bounds[0] = wbnd[0];
    bounds[1] = wbnd[1];
    bounds[2] = wbnd[2];
    bounds[3] = wbnd[3];
  }
  /* get min max of lon lat for data input from projected bounds */
  else {
    /* do first point */
    xx = wbnd[0] - 0.05 * (wbnd[1] - wbnd[0]);
    yy = wbnd[2] - 0.05 * (wbnd[3] - wbnd[2]);
    mb_proj_inverse(verbose, pjptr, xx, yy, &xlon, &ylat, &error);
    mb_apply_lonflip(verbose, lonflip, &xlon);
    bounds[0] = xlon;
    bounds[1] = xlon;
    bounds[2] = ylat;
    bounds[3] = ylat;

    /* do second point */
    xx = wbnd[1] + 0.05 * (wbnd[1] - wbnd[0]);
    yy = wbnd[2] - 0.05 * (wbnd[3] - wbnd[2]);
    mb_proj_inverse(verbose, pjptr, xx, yy, &xlon, &ylat, &error);
    mb_apply_lonflip(verbose, lonflip, &xlon);
    bounds[0] = std::min(bounds[0], xlon);
    bounds[1] = std::max(bounds[1], xlon);
    bounds[2] = std::min(bounds[2], ylat);
    bounds[3] = std::max(bounds[3], ylat);

    /* do third point */
    xx = wbnd[0] - 0.05 * (wbnd[1] - wbnd[0]);
    yy = wbnd[3] + 0.05 * (wbnd[3] - wbnd[2]);
    mb_proj_inverse(verbose, pjptr, xx, yy, &xlon, &ylat, &error);
    mb_apply_lonflip(verbose, lonflip, &xlon);
    bounds[0] = std::min(bounds[0], xlon);
    bounds[1] = std::max(bounds[1], xlon);
    bounds[2] = std::min(bounds[2], ylat);
    bounds[3] = std::max(bounds[3], ylat);

    /* do fourth point */
    xx = wbnd[1] + 0.05 * (wbnd[1] - wbnd[0]);
    yy = wbnd[3] + 0.05 * (wbnd[3] - wbnd[2]);
    mb_proj_inverse(verbose, pjptr, xx, yy, &xlon, &ylat, &error);
    mb_apply_lonflip(verbose, lonflip, &xlon);
    bounds[0] = std::min(bounds[0], xlon);
    bounds[1] = std::max(bounds[1], xlon);
    bounds[2] = std::min(bounds[2], ylat);
    bounds[3] = std::max(bounds[3], ylat);
  }

  /* extend the bounds slightly to be sure no data gets missed */
  xx = std::min(0.05 * (bounds[1] - bounds[0]), 0.1);
  yy = std::min(0.05 * (bounds[3] - bounds[2]), 0.1);
  bounds[0] = bounds[0] - xx;
  bounds[1] = bounds[1] + xx;
  bounds[2] = bounds[2] - yy;
  bounds[3] = bounds[3] + yy;

  /* figure out lonflip for data bounds */
  if (bounds[0] < -180.0)
    lonflip = -1;
  else if (bounds[1] > 180.0)
    lonflip = 1;
  else if (lonflip == -1 && bounds[1] > 0.0)
    lonflip = 0;
  else if (lonflip == 1 && bounds[0] < 0.0)
    lonflip = 0;

  /* check interpolation parameters */
  if ((clipmode == MBGRID_INTERP_GAP || clipmode == MBGRID_INTERP_NEAR) && clip > xdim && clip > ydim)
    clipmode = MBGRID_INTERP_ALL;
  if (clipmode == MBGRID_INTERP_ALL)
    clip = std::max(xdim, ydim);

  /* set origin used to reduce data value size before conversion from
   * double to float when calling the interpolation routines */
  bdata_origin_x = 0.5 * (wbnd[0] + wbnd[1]);
  bdata_origin_y = 0.5 * (wbnd[2] + wbnd[3]);

  /* set plot label strings */
  if (use_projection) {
    snprintf(xlabel, sizeof(xlabel), "Easting (%s)", units);
    snprintf(ylabel, sizeof(ylabel), "Northing (%s)", units);
  }
  else {
    strcpy(xlabel, "Longitude");
    strcpy(ylabel, "Latitude");
  }
  if (datatype == MBGRID_DATA_BATHYMETRY) {
    if (bathy_in_feet)
      strcpy(zlabel, "Depth (ft)");
    else
      strcpy(zlabel, "Depth (m)");
    strcpy(nlabel, "Number of Depth Data Points");
    if (bathy_in_feet)
      strcpy(sdlabel, "Depth Standard Deviation (ft)");
    else
      strcpy(sdlabel, "Depth Standard Deviation (m)");
    strcpy(title, "Bathymetry Grid");
  }
  else if (datatype == MBGRID_DATA_TOPOGRAPHY) {
    if (bathy_in_feet)
      strcpy(zlabel, "Topography (ft)");
    else
      strcpy(zlabel, "Topography (m)");
    strcpy(nlabel, "Number of Topography Data Points");
    if (bathy_in_feet)
      strcpy(sdlabel, "Topography Standard Deviation (ft)");
    else
      strcpy(sdlabel, "Topography Standard Deviation (m)");
    strcpy(title, "Topography Grid");
  }
  else if (datatype == MBGRID_DATA_AMPLITUDE) {
    strcpy(zlabel, "Amplitude");
    strcpy(nlabel, "Number of Amplitude Data Points");
    strcpy(sdlabel, "Amplitude Standard Deviation (m)");
    strcpy(title, "Amplitude Grid");
  }
  else if (datatype == MBGRID_DATA_SIDESCAN) {
    strcpy(zlabel, "Sidescan");
    strcpy(nlabel, "Number of Sidescan Data Points");
    strcpy(sdlabel, "Sidescan Standard Deviation (m)");
    strcpy(title, "Sidescan Grid");
  }

  /* output info */
  if (verbose >= 0) {
    fprintf(outfp, "\nMBGRID Parameters:\n");
    fprintf(outfp, "List of input files: %s\n", filelist);
    fprintf(outfp, "Output fileroot:     %s\n", fileroot);
    fprintf(outfp, "Input Data Type:     ");
    if (datatype == MBGRID_DATA_BATHYMETRY) {
      fprintf(outfp, "Bathymetry\n");
      if (bathy_in_feet)
        fprintf(outfp, "Bathymetry gridded in feet\n");
    }
    else if (datatype == MBGRID_DATA_TOPOGRAPHY) {
      fprintf(outfp, "Topography\n");
      if (bathy_in_feet)
        fprintf(outfp, "Topography gridded in feet\n");
    }
    else if (datatype == MBGRID_DATA_AMPLITUDE)
      fprintf(outfp, "Amplitude\n");
    else if (datatype == MBGRID_DATA_SIDESCAN)
      fprintf(outfp, "Sidescan\n");
    else
      fprintf(outfp, "Unknown?\n");
    fprintf(outfp, "Gridding algorithm:  ");
    if (grid_mode == MBGRID_MEDIAN_FILTER)
      fprintf(outfp, "Median Filter\n");
    else if (grid_mode == MBGRID_MINIMUM_FILTER)
      fprintf(outfp, "Minimum Filter\n");
    else if (grid_mode == MBGRID_MAXIMUM_FILTER)
      fprintf(outfp, "Maximum Filter\n");
    else if (grid_mode == MBGRID_WEIGHTED_FOOTPRINT_SLOPE)
      fprintf(outfp, "Footprint-Slope Weighted Mean\n");
    else if (grid_mode == MBGRID_WEIGHTED_FOOTPRINT)
      fprintf(outfp, "Footprint Weighted Mean\n");
    else if (grid_mode == MBGRID_MINIMUM_WEIGHTED_MEAN)
      fprintf(outfp, "Minimum Gaussian Weighted Mean\n");
    else if (grid_mode == MBGRID_MAXIMUM_WEIGHTED_MEAN)
      fprintf(outfp, "Maximum Gaussian Weighted Mean\n");
    else
      fprintf(outfp, "Gaussian Weighted Mean\n");
    fprintf(outfp, "Grid projection: %s\n", projection_id);
    if (use_projection) {
      fprintf(outfp, "Projection ID: %s\n", projection_id);
    }
    fprintf(outfp, "Grid dimensions: %d %d\n", xdim, ydim);
    fprintf(outfp, "Grid bounds:\n");
    if (use_projection) {
      fprintf(outfp, "  Eastings:  %9.4f %9.4f\n", gbnd[0], gbnd[1]);
      fprintf(outfp, "  Northings: %9.4f %9.4f\n", gbnd[2], gbnd[3]);
      fprintf(outfp, "  Longitude: %9.4f %9.4f\n", obnd[0], obnd[1]);
      fprintf(outfp, "  Latitude:  %9.4f %9.4f\n", obnd[2], obnd[3]);
    }
    else {
      fprintf(outfp, "  Longitude: %9.4f %9.4f\n", gbnd[0], gbnd[1]);
      fprintf(outfp, "  Latitude:  %9.4f %9.4f\n", gbnd[2], gbnd[3]);
    }
    if (boundsfactor > 1.0)
      fprintf(outfp, "  Grid bounds correspond to %f times actual data coverage\n", boundsfactor);
    fprintf(outfp, "Working grid dimensions: %d %d\n", gxdim, gydim);
    if (use_projection) {
      fprintf(outfp, "Working Grid bounds:\n");
      fprintf(outfp, "  Eastings:  %9.4f %9.4f\n", wbnd[0], wbnd[1]);
      fprintf(outfp, "  Northings: %9.4f %9.4f\n", wbnd[2], wbnd[3]);
      fprintf(outfp, "Easting interval:  %f %s\n", dx, units);
      fprintf(outfp, "Northing interval: %f %s\n", dy, units);
      if (set_spacing) {
        fprintf(outfp, "Specified Easting interval:  %f %s\n", dx_set, units);
        fprintf(outfp, "Specified Northing interval: %f %s\n", dy_set, units);
      }
    }
    else {
      fprintf(outfp, "Working Grid bounds:\n");
      fprintf(outfp, "  Longitude: %9.4f %9.4f\n", wbnd[0], wbnd[1]);
      fprintf(outfp, "  Latitude:  %9.4f %9.4f\n", wbnd[2], wbnd[3]);
      fprintf(outfp, "Longitude interval: %f degrees or %f m\n", dx, 1000 * dx * deglontokm);
      fprintf(outfp, "Latitude interval:  %f degrees or %f m\n", dy, 1000 * dy * deglattokm);
      if (set_spacing) {
        fprintf(outfp, "Specified Longitude interval: %f %s\n", dx_set, units);
        fprintf(outfp, "Specified Latitude interval:  %f %s\n", dy_set, units);
      }
    }
    if (shift && use_projection) {
      fprintf(outfp, "Grid shift (applied to the bounds of output grids):\n");
      fprintf(outfp, "  East shift:   %9.4f m\n", shift_x);
      fprintf(outfp, "  North shift:  %9.4f m\n", shift_y);
    }
    else if (shift) {
      fprintf(outfp, "Grid shift (applied to the bounds of output grids):\n");
      fprintf(outfp, "  Longitude interval: %f degrees or %f m\n", shift_x * mtodeglon, shift_x);
      fprintf(outfp, "  Latitude interval:  %f degrees or %f m\n", shift_y * mtodeglat, shift_y);
    }
    fprintf(outfp, "Input data bounds:\n");
    fprintf(outfp, "  Longitude: %9.4f %9.4f\n", bounds[0], bounds[1]);
    fprintf(outfp, "  Latitude:  %9.4f %9.4f\n", bounds[2], bounds[3]);
    if (grid_mode == MBGRID_WEIGHTED_MEAN)
      fprintf(outfp, "Gaussian filter 1/e length: %f grid intervals\n", scale);
    if (grid_mode == MBGRID_WEIGHTED_FOOTPRINT_SLOPE || grid_mode == MBGRID_WEIGHTED_FOOTPRINT)
      fprintf(outfp, "Footprint 1/e distance: %f times footprint\n", scale);
    if (grid_mode == MBGRID_MINIMUM_WEIGHTED_MEAN)
      fprintf(outfp, "Minimum filter threshold for Minimum Weighted Mean: %f\n", minormax_weighted_mean_threshold);
    if (check_time && !first_in_stays)
      fprintf(outfp, "Swath overlap handling:       Last data used\n");
    if (check_time && first_in_stays)
      fprintf(outfp, "Swath overlap handling:       First data used\n");
    if (check_time)
      fprintf(outfp, "Swath overlap time threshold: %f minutes\n", timediff / 60.);
    if (clipmode == MBGRID_INTERP_NONE)
      fprintf(outfp, "Spline interpolation not applied\n");
    else if (clipmode == MBGRID_INTERP_GAP) {
      fprintf(outfp, "Spline interpolation applied to fill data gaps\n");
      fprintf(outfp, "Spline interpolation clipping dimension: %d\n", clip);
      fprintf(outfp, "Spline tension (range 0.0 to infinity): %f\n", tension);
    }
    else if (clipmode == MBGRID_INTERP_NEAR) {
      fprintf(outfp, "Spline interpolation applied near data\n");
      fprintf(outfp, "Spline interpolation clipping dimension: %d\n", clip);
      fprintf(outfp, "Spline tension (range 0.0 to infinity): %f\n", tension);
    }
    else if (clipmode == MBGRID_INTERP_ALL) {
      fprintf(outfp, "Spline interpolation applied to fill entire grid\n");
      fprintf(outfp, "Spline tension (range 0.0 to infinity): %f\n", tension);
    }
    if (grdrasterid == 0)
      fprintf(outfp, "Background not applied\n");
    else if (grdrasterid < 0)
      fprintf(outfp, "Background obtained using grd2xyz from GMT grid file: %s\n", backgroundfile);
    else
      fprintf(outfp, "Background obtained using grdraster from dataset: %d\n", grdrasterid);
    if (gridkind == MBGRID_ASCII)
      fprintf(outfp, "Grid format %d:  ascii table\n", gridkind);
    else if (gridkind == MBGRID_CDFGRD)
      fprintf(outfp, "Grid format %d:  GMT version 2 grd (netCDF)\n", gridkind);
    else if (gridkind == MBGRID_OLDGRD)
      fprintf(outfp, "Grid format %d:  GMT version 1 grd (binary)\n", gridkind);
    else if (gridkind == MBGRID_ARCASCII)
      fprintf(outfp, "Grid format %d:  Arc/Info ascii table\n", gridkind);
    else if (gridkind == MBGRID_GMTGRD) {
      fprintf(outfp, "Grid format %d:  GMT grid\n", gridkind);
      if (strlen(gridkindstring) > 0)
        fprintf(outfp, "GMT Grid ID:     %s\n", gridkindstring);
    }
    if (use_NaN)
      fprintf(outfp, "NaN values used to flag regions with no data\n");
    else
      fprintf(outfp, "Real value of %f used to flag regions with no data\n", outclipvalue);
    if (more)
      fprintf(outfp, "Data density and sigma grids also created\n");
    fprintf(outfp, "MBIO parameters:\n");
    fprintf(outfp, "  Ping averaging:       %d\n", pings);
    fprintf(outfp, "  Longitude flipping:   %d\n", lonflip);
    fprintf(outfp, "  Speed minimum:      %4.1f km/hr\n", speedmin);
  }
  if (verbose > 0)
    fprintf(outfp, "\n");

  /* if grdrasterid set extract background data
      and interpolate it later onto internal grid */
  if (grdrasterid != 0) {
    if (grdrasterid > 0)
      fprintf(outfp, "\nExtracting background from grdraster dataset %d...\n", grdrasterid);
    else
      fprintf(outfp, "\nExtracting background from grid file %s...\n", backgroundfile);

    /* guess about twice the data actually expected */
    int nbackground_alloc = 2 * gxdim * gydim;

/* allocate and initialize background data arrays */
#ifdef USESURFACE
    status = mb_mallocd(verbose, __FILE__, __LINE__, nbackground_alloc * sizeof(float), (void **)&bxdata, &error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, nbackground_alloc * sizeof(float), (void **)&bydata, &error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, nbackground_alloc * sizeof(float), (void **)&bzdata, &error);
    if (error != MB_ERROR_NO_ERROR) {
      char *message = nullptr;
      mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
      fprintf(outfp, "\nMBIO Error allocating background data array:\n%s\n", message);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      mb_memory_clear(verbose, &memclear_error);
      exit(MB_ERROR_MEMORY_FAIL);
    }
    memset((char *)bxdata, 0, nbackground_alloc * sizeof(float));
    memset((char *)bydata, 0, nbackground_alloc * sizeof(float));
    memset((char *)bzdata, 0, nbackground_alloc * sizeof(float));
#else
    status = mb_mallocd(verbose, __FILE__, __LINE__, 3 * nbackground_alloc * sizeof(float), (void **)&bdata, &error);
    if (error != MB_ERROR_NO_ERROR) {
      char *message = nullptr;
      mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
      fprintf(outfp, "\nMBIO Error allocating background interpolation work arrays:\n%s\n", message);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      mb_memory_clear(verbose, &memclear_error);
      exit(MB_ERROR_MEMORY_FAIL);
    }
    memset((char *)bdata, 0, 3 * nbackground_alloc * sizeof(float));
#endif

    const int pid = getpid();

    /* get initial grid using grdraster */
    if (grdrasterid > 0) {
      snprintf(backgroundfile, sizeof(backgroundfile), "tmpgrdraster%d.grd", pid);
      snprintf(plot_cmd, sizeof(plot_cmd), "grdraster %d -R%f/%f/%f/%f -G%s", grdrasterid, bounds[0], bounds[1], bounds[2], bounds[3],
              backgroundfile);
      fprintf(stderr, "Executing: %s\n", plot_cmd);
      const int fork_status = system(plot_cmd);
      if (fork_status != 0) {
        fprintf(outfp, "\nExecution of command:\n\t%s\nby system() call failed....\nProgram <%s> Terminated\n", plot_cmd,
                program_name);
        mb_memory_clear(verbose, &memclear_error);
        exit(MB_ERROR_BAD_PARAMETER);
      }
    }

    /* if needed translate grid to normal registration */
    snprintf(plot_cmd, sizeof(plot_cmd), "gmt grdinfo %s", backgroundfile);
    char backgroundfileuse[MB_PATH_MAXLINE] = "";
    strcpy(backgroundfileuse, backgroundfile);
    if ((rfp = popen(plot_cmd, "r")) != nullptr) {
      /* parse the grdinfo results */
      char plot_stdout[MB_COMMENT_MAXLINE];
      /* char *bufptr = */ fgets(plot_stdout, MB_COMMENT_MAXLINE, rfp);
      /* bufptr = */ fgets(plot_stdout, MB_COMMENT_MAXLINE, rfp);
      /* bufptr = */ fgets(plot_stdout, MB_COMMENT_MAXLINE, rfp);
      /* bufptr = */ fgets(plot_stdout, MB_COMMENT_MAXLINE, rfp);
      pclose(rfp);
      if (strncmp(plot_stdout, "Pixel node registration used", 28) == 0) {
        snprintf(backgroundfileuse, sizeof(backgroundfileuse), "tmpgrdsampleT%d.grd", pid);
        snprintf(plot_cmd, sizeof(plot_cmd), "grdsample %s -G%s -T", backgroundfile, backgroundfileuse);
        fprintf(stderr, "Executing: %s\n", plot_cmd);
        const int fork_status = system(plot_cmd);
        if (fork_status != 0) {
          fprintf(outfp, "\nExecution of command:\n\t%s\nby system() call failed....\nProgram <%s> Terminated\n",
                  plot_cmd, program_name);
          mb_memory_clear(verbose, &memclear_error);
          exit(MB_ERROR_BAD_PARAMETER);
        }
      }
    }
    else {
      fprintf(outfp, "\nBackground data not extracted as per -K option\n");
      if (grdrasterid > 0) {
        fprintf(outfp, "The program grdraster may not have been found\n");
        fprintf(outfp, "or the specified background dataset %d may not exist.\n", grdrasterid);
      }
      else {
        fprintf(outfp, "The specified background dataset %s may not exist.\n", backgroundfile);
      }
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      error = MB_ERROR_BAD_PARAMETER;
      mb_memory_clear(verbose, &memclear_error);
      exit(MB_ERROR_BAD_PARAMETER);
    }

    /* resample extracted grid to have similar resolution as working grid */
    if (use_projection)
      snprintf(plot_cmd, sizeof(plot_cmd), "gmt grdsample %s -Gtmpgrdsample%d.grd -R%.12f/%.12f/%.12f/%.12f -I%.12f/%.12f", backgroundfileuse,
              pid, bounds[0], bounds[1], bounds[2], bounds[3], dx * mtodeglon, dy * mtodeglat);
    else
      snprintf(plot_cmd, sizeof(plot_cmd), "gmt grdsample %s -Gtmpgrdsample%d.grd -R%.12f/%.12f/%.12f/%.12f -I%.12f/%.12f", backgroundfileuse,
              pid, bounds[0], bounds[1], bounds[2], bounds[3], dx, dy);
    fprintf(stderr, "Executing: %s\n", plot_cmd);
    int fork_status = system(plot_cmd);
    if (fork_status != 0) {
      fprintf(outfp, "\nExecution of command:\n\t%s\nby system() call failed....\nProgram <%s> Terminated\n", plot_cmd,
              program_name);
      error = MB_ERROR_BAD_PARAMETER;
      mb_memory_clear(verbose, &memclear_error);
      exit(MB_ERROR_BAD_PARAMETER);
    }

    /* extract points with preprocessing if that will help */
    if (use_projection) {
      snprintf(plot_cmd, sizeof(plot_cmd), "gmt grd2xyz tmpgrdsample%d.grd -s -bo | gmt blockmean -bi -bo -C -R%f/%f/%f/%f -I%.12f/%.12f", pid,
              bounds[0], bounds[1], bounds[2], bounds[3], dx * mtodeglon, dy * mtodeglat);
    }
    else {
      snprintf(plot_cmd, sizeof(plot_cmd), "gmt grd2xyz tmpgrdsample%d.grd -s -bo | gmt blockmean -bi -bo -C -R%f/%f/%f/%f -I%.12f/%.12f", pid,
              bounds[0], bounds[1], bounds[2], bounds[3], dx, dy);
    }
    fprintf(stderr, "Executing: %s\n", plot_cmd);
    if ((rfp = popen(plot_cmd, "r")) != nullptr) {
      /* loop over reading */
      nbackground = 0;
      while (fread(&tlon, sizeof(double), 1, rfp) == 1) {
        /* size_t freadsize = */ fread(&tlat, sizeof(double), 1, rfp);
        /* freadsize = */ fread(&tvalue, sizeof(double), 1, rfp);
        if (lonflip == -1 && tlon > 0.0)
          tlon -= 360.0;
        else if (lonflip == 0 && tlon < -180.0)
          tlon += 360.0;
        else if (lonflip == 0 && tlon > 180.0)
          tlon -= 360.0;
        else if (lonflip == 1 && tlon < 0.0)
          tlon += 360.0;
        if (use_projection)
          mb_proj_forward(verbose, pjptr, tlon, tlat, &tlon, &tlat, &error);
#ifdef USESURFACE
        if (nbackground >= nbackground_alloc) {
          nbackground_alloc += 10000;
          status =
              mb_reallocd(verbose, __FILE__, __LINE__, nbackground_alloc * sizeof(float), (void **)&bxdata, &error);
          if (status == MB_SUCCESS)
            status =
                mb_reallocd(verbose, __FILE__, __LINE__, nbackground_alloc * sizeof(float), (void **)&bydata, &error);
          if (status == MB_SUCCESS)
            status =
                mb_reallocd(verbose, __FILE__, __LINE__, nbackground_alloc * sizeof(float), (void **)&bzdata, &error);
          if (error != MB_ERROR_NO_ERROR) {
            char *message = nullptr;
            mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
            fprintf(outfp, "\nMBIO Error reallocating background data array:\n%s\n", message);
            fprintf(outfp, "\nProgram <%s> Terminated at line %d in source file %s\n", program_name, __LINE__,
                    __FILE__);
            mb_memory_clear(verbose, &memclear_error);
            exit(MB_ERROR_MEMORY_FAIL);
          }
        }
        bxdata[nbackground] = (float)(tlon - bdata_origin_x);
        bydata[nbackground] = (float)(tlat - bdata_origin_y);
        bzdata[nbackground] = (float)tvalue;
#else
        if (nbackground >= nbackground_alloc) {
          nbackground_alloc += 10000;
          status =
              mb_reallocd(verbose, __FILE__, __LINE__, 3 * nbackground_alloc * sizeof(float), (void **)&bdata, &error);
          if (error != MB_ERROR_NO_ERROR) {
            char *message = nullptr;
            mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
            fprintf(outfp, "\nMBIO Error allocating background interpolation work arrays:\n%s\n", message);
            fprintf(outfp, "\nProgram <%s> Terminated at line %d in source file %s\n", program_name, __LINE__,
                    __FILE__);
            mb_memory_clear(verbose, &memclear_error);
            exit(MB_ERROR_MEMORY_FAIL);
          }
        }
        bdata[nbackground * 3] = (float)(tlon - bdata_origin_x);
        bdata[nbackground * 3 + 1] = (float)(tlat - bdata_origin_y);
        bdata[nbackground * 3 + 2] = (float)tvalue;
#endif
        nbackground++;
      }
      pclose(rfp);
    }
    else {
      fprintf(outfp, "\nBackground data not extracted as per -K option\n");
      fprintf(outfp, "The program grdraster may not have been found\n");
      fprintf(outfp, "or the specified background dataset %d may not exist.\n", grdrasterid);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      error = MB_ERROR_BAD_PARAMETER;
      mb_memory_clear(verbose, &memclear_error);
      exit(MB_ERROR_BAD_PARAMETER);
    }

    /* delete any temporary files */
    snprintf(plot_cmd, sizeof(plot_cmd), "rm tmpgrd*%d.grd", pid);
    fprintf(stderr, "Executing: %s\n", plot_cmd);
    fork_status = system(plot_cmd);
    if (fork_status != 0) {
      fprintf(outfp, "\nExecution of command:\n\t%s\nby system() call failed....\nProgram <%s> Terminated\n", plot_cmd,
              program_name);
      error = MB_ERROR_BAD_PARAMETER;
      mb_memory_clear(verbose, &memclear_error);
      exit(MB_ERROR_BAD_PARAMETER);
    }
  }

  /* allocate memory for grid arrays */
  status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(double), (void **)&grid, &error);
  if (status == MB_SUCCESS)
    status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(double), (void **)&sigma, &error);
  if (status == MB_SUCCESS)
    status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(double), (void **)&firsttime, &error);
  if (status == MB_SUCCESS)
    status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(int), (void **)&cnt, &error);
  if (status == MB_SUCCESS)
    status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(int), (void **)&num, &error);
  if (status == MB_SUCCESS)
    status = mb_mallocd(verbose, __FILE__, __LINE__, xdim * ydim * sizeof(float), (void **)&output, &error);

  /* if error initializing memory then quit */
  if (error != MB_ERROR_NO_ERROR) {
    char *message = nullptr;
    mb_error(verbose, error, &message);
    fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
    fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
    mb_memory_clear(verbose, &memclear_error);
    exit(error);
  }

  /* open datalist file for list of all files that contribute to the grid */
  strcpy(dfile, fileroot);
  strcat(dfile, ".mb-1");
  if ((dfp = fopen(dfile, "w")) == nullptr) {
    error = MB_ERROR_OPEN_FAIL;
    fprintf(outfp, "\nUnable to open datalist file: %s\n", dfile);
  }

/* -------------------------------------------------------------------------- */
  bool file_in_bounds = false;
  bool time_ok;  // TODO(schwehr): Probably can localize many variables.
  bool region_ok;
  bool footprint_ok;

  /***** do weighted footprint slope gridding *****/
  if (grid_mode == MBGRID_WEIGHTED_FOOTPRINT_SLOPE) {
    /* set up parameters for first cut low resolution slope grid */
    // sbnd[4]; for (int i = 0; i < 4; i++) sbnd[i] = wbnd[i];
    const double sdx = 2.0 * dx;
    const double sdy = 2.0 * dy;
    int sxdim = gxdim / 2;
    int sydim = gydim / 2;
    int sclip = std::max(gxdim, gydim);

    /* allocate memory for additional arrays */
    /* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(double), (void **)&norm, &error);
    /* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, sxdim * sydim * sizeof(double), (void **)&gridsmall, &error);

    /* if error initializing memory then quit */
    if (error != MB_ERROR_NO_ERROR) {
      char *message = nullptr;
      mb_error(verbose, error, &message);
      fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      mb_memory_clear(verbose, &memclear_error);
      exit(error);
    }

    /* do first pass using simple mean to get low-resolution quick bathymetry to provide bottom slope
        estimates for footprint gridding */

    /* initialize arrays */
    for (int i = 0; i < sxdim; i++)
      for (int j = 0; j < sydim; j++) {
        kgrid = i * sydim + j;
        gridsmall[kgrid] = 0.0;
        cnt[kgrid] = 0;
      }

    /* read in data */
    fprintf(outfp, "\nDoing first pass to generate low resolution slope grid...\n");
    ndata = 0;
    const int look_processed = MB_DATALIST_LOOK_UNSET;
    if (mb_datalist_open(verbose, &datalist, filelist, look_processed, &error) != MB_SUCCESS) {
      error = MB_ERROR_OPEN_FAIL;
      fprintf(outfp, "\nUnable to open data list file: %s\n", filelist);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      mb_memory_clear(verbose, &memclear_error);
      exit(MB_ERROR_OPEN_FAIL);
    }
    while (mb_datalist_read3(verbose, datalist, &pstatus, path, ppath, &astatus, apath, dpath, &format, &file_weight, &error) ==
           MB_SUCCESS) {
      ndatafile = 0;

      /* if format > 0 then input is swath sonar file */
      if (format > 0 && path[0] != '#') {
        /* apply pstatus */
        if (pstatus == MB_PROCESSED_USE)
          strcpy(file, ppath);
        else
          strcpy(file, path);

        /* check for mbinfo file - get file bounds if possible */
        rformat = format;
        strcpy(rfile, file);
        status = mb_check_info(verbose, rfile, lonflip, bounds, &file_in_bounds, &error);
        if (status == MB_FAILURE) {
          file_in_bounds = true;
          status = MB_SUCCESS;
          error = MB_ERROR_NO_ERROR;
        }

        /* initialize the swath sonar file */
        bool first = true;
        double dmin = 0.0;
        double dmax = 0.0;
        if (file_in_bounds) {
          /* check for "fast bathymetry" or "fbt" file */
          if (datatype == MBGRID_DATA_TOPOGRAPHY || datatype == MBGRID_DATA_BATHYMETRY) {
            mb_get_fbt(verbose, rfile, &rformat, &error);
          }

          /* call mb_read_init_altnav() */
          if (mb_read_init_altnav(verbose, rfile, rformat, pings, lonflip, bounds, btime_i, etime_i, speedmin,
                                     timegap, astatus, apath, &mbio_ptr, &btime_d, &etime_d, 
                                     &beams_bath, &beams_amp, &pixels_ss,
                                     &error) != MB_SUCCESS) {
            char *message = nullptr;
            mb_error(verbose, error, &message);
            fprintf(outfp, "\nMBIO Error returned from function <mb_read_init_altnav>:\n%s\n", message);
            fprintf(outfp, "\nMultibeam File <%s> not initialized for reading\n", rfile);
            fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
            mb_memory_clear(verbose, &memclear_error);
            exit(error);
          }

          /* get mb_io_ptr */
          mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

          /* get topography type */
          status = mb_sonartype(verbose, mbio_ptr, mb_io_ptr->store_data, &topo_type, &error);

          /* allocate memory for reading data arrays */
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlon,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlat,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslon, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslat, &error);

          /* if error initializing memory then quit */
          if (error != MB_ERROR_NO_ERROR) {
            char *message = nullptr;
            mb_error(verbose, error, &message);
            fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
            fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
            mb_memory_clear(verbose, &memclear_error);
            exit(error);
          }

          /* loop over reading */
          while (error <= MB_ERROR_NO_ERROR) {
            status = mb_read(verbose, mbio_ptr, &kind, &rpings, time_i, &time_d, &navlon, &navlat, &speed, &heading,
                             &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath,
                             amp, bathlon, bathlat, ss, sslon, sslat, comment, &error);

            /* time gaps are not a problem here */
            if (error == MB_ERROR_TIME_GAP) {
              error = MB_ERROR_NO_ERROR;
              status = MB_SUCCESS;
            }

            if (verbose >= 2) {
              fprintf(outfp, "\ndbg2  Ping read in program <%s>\n", program_name);
              fprintf(outfp, "dbg2       kind:           %d\n", kind);
              fprintf(outfp, "dbg2       beams_bath:     %d\n", beams_bath);
              fprintf(outfp, "dbg2       beams_amp:      %d\n", beams_amp);
              fprintf(outfp, "dbg2       pixels_ss:      %d\n", pixels_ss);
              fprintf(outfp, "dbg2       error:          %d\n", error);
              fprintf(outfp, "dbg2       status:         %d\n", status);
            }

            if ((datatype == MBGRID_DATA_BATHYMETRY || datatype == MBGRID_DATA_TOPOGRAPHY) &&
                error == MB_ERROR_NO_ERROR) {

              /* reproject beam positions if necessary */
              if (use_projection) {
                mb_proj_forward(verbose, pjptr, navlon, navlat, &navlon, &navlat, &error);
                for (ib = 0; ib < beams_bath; ib++)
                  if (mb_beam_ok(beamflag[ib]))
                    mb_proj_forward(verbose, pjptr, bathlon[ib], bathlat[ib], &bathlon[ib], &bathlat[ib],
                                    &error);
              }

              /* deal with data */
              for (ib = 0; ib < beams_bath; ib++)
                if (mb_beam_ok(beamflag[ib])) {
                  /* get position in grid */
                  ix = (bathlon[ib] - wbnd[0] + dx) / sdx;
                  iy = (bathlat[ib] - wbnd[2] + dy) / sdy;

                  /* process if in region of interest */
                  if (ix >= 0 && ix < sxdim
                      && iy >= 0 && iy < sydim) {
                    kgrid = ix * sydim + iy;
                    gridsmall[kgrid] += topofactor * bath[ib];
                    cnt[kgrid]++;
                    ndata++;
                    ndatafile++;
                    if (first) {
                      first = false;
                      dmin = topofactor * bath[ib];
                      dmax = topofactor * bath[ib];
                    } else {
                      dmin = std::min(topofactor * bath[ib], dmin);
                      dmax = std::max(topofactor * bath[ib], dmax);
                    }
                  }
                }
            }
          }
          mb_close(verbose, &mbio_ptr, &error);
          status = MB_SUCCESS;
          error = MB_ERROR_NO_ERROR;
        }
        if (verbose >= 2)
          fprintf(outfp, "\n");
        if (verbose > 0)
          fprintf(outfp, "%d data points processed in %s (minmax: %f %f)\n", ndatafile, rfile, dmin, dmax);
        else if (file_in_bounds)
          fprintf(outfp, "%d data points processed in %s\n", ndatafile, rfile);

        /* add to datalist if data actually contributed */
        if (ndatafile > 0 && dfp != nullptr) {
          if (pstatus == MB_PROCESSED_USE && astatus == MB_ALTNAV_USE)
            fprintf(dfp, "A:%s %d %f %s\n", path, format, file_weight, apath);
          else if (pstatus == MB_PROCESSED_USE)
            fprintf(dfp, "P:%s %d %f\n", path, format, file_weight);
          else
            fprintf(dfp, "R:%s %d %f\n", path, format, file_weight);
          fflush(dfp);
        }
      } /* end if (format > 0) */
    }
    if (datalist != nullptr)
      mb_datalist_close(verbose, &datalist, &error);
    if (verbose > 0)
      fprintf(outfp, "\n%d total data points processed\n", ndata);

    /* close datalist if necessary */
    if (dfp != nullptr) {
      fclose(dfp);
      dfp = nullptr;
    }

    /* now loop over all points in the low resolution grid */
    if (verbose >= 1)
      fprintf(outfp, "\nMaking low resolution slope grid...\n");
    ndata = 8;
    for (int i = 0; i < sxdim; i++)
      for (int j = 0; j < sydim; j++) {
        kgrid = i * sydim + j;
        if (cnt[kgrid] > 0) {
          gridsmall[kgrid] = gridsmall[kgrid] / ((double)cnt[kgrid]);
          ndata++;
        }
      }

/* now fill in the low resolution grid with interpolation */
#ifdef USESURFACE
    /* allocate and initialize sgrid */
    status = mb_mallocd(verbose, __FILE__, __LINE__, ndata * sizeof(float), (void **)&sxdata, &error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, ndata * sizeof(float), (void **)&sydata, &error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, ndata * sizeof(float), (void **)&szdata, &error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, sxdim * sydim * sizeof(float), (void **)&sgrid, &error);
    if (error != MB_ERROR_NO_ERROR) {
      char *message = nullptr;
      mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
      fprintf(outfp, "\nMBIO Error allocating interpolation work arrays:\n%s\n", message);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      mb_memory_clear(verbose, &memclear_error);
      exit(error);
    }
    memset((char *)sgrid, 0, sxdim * sydim * sizeof(float));
    memset((char *)sxdata, 0, ndata * sizeof(float));
    memset((char *)sydata, 0, ndata * sizeof(float));
    memset((char *)szdata, 0, ndata * sizeof(float));

    /* get points from grid */
    /* simultaneously find the depth values nearest to the grid corners and edge midpoints */
    ndata = 0;
    for (int i = 0; i < sxdim; i++)
      for (int j = 0; j < sydim; j++) {
        kgrid = i * sydim + j;
        if (cnt[kgrid] > 0) {
          sxdata[ndata] = (float)(wbnd[0] + sdx * i - bdata_origin_x);
          sydata[ndata] = (float)(wbnd[2] + sdy * j - bdata_origin_y);
          szdata[ndata] = (float)gridsmall[kgrid];
          ndata++;
        }
      }

    /* do the interpolation */
    fprintf(outfp, "\nDoing Surface spline interpolation with %d data points...\n", ndata);
    mb_surface(verbose, ndata, sxdata, sydata, szdata, (wbnd[0] - bdata_origin_x), (wbnd[1] - bdata_origin_x),
               (wbnd[2] - bdata_origin_y), (wbnd[3] - bdata_origin_y), sdx, sdy, tension, sgrid);
#else
    /* allocate and initialize sgrid */
    status = mb_mallocd(verbose, __FILE__, __LINE__, 3 * ndata * sizeof(float), (void **)&sdata, &error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, sxdim * sydim * sizeof(float), (void **)&sgrid, &error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, ndata * sizeof(float), (void **)&work1, &error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, ndata * sizeof(int), (void **)&work2, &error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, (sxdim + sydim) * sizeof(bool), (void **)&work3, &error);
    if (error != MB_ERROR_NO_ERROR) {
      char *message = nullptr;
      mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
      fprintf(outfp, "\nMBIO Error allocating interpolation work arrays:\n%s\n", message);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      mb_memory_clear(verbose, &memclear_error);
      exit(error);
    }
    memset((char *)sgrid, 0, sxdim * sydim * sizeof(float));
    memset((char *)sdata, 0, 3 * ndata * sizeof(float));
    memset((char *)work1, 0, ndata * sizeof(float));
    memset((char *)work2, 0, ndata * sizeof(int));
    memset((char *)work3, 0, (sxdim + sydim) * sizeof(bool));

    /* get points from grid */
    /* simultaneously find the depth values nearest to the grid corners and edge midpoints */
    ndata = 0;
    for (int i = 0; i < sxdim; i++)
      for (int j = 0; j < sydim; j++) {
        kgrid = i * sydim + j;
        if (cnt[kgrid] > 0) {
          sdata[ndata++] = (float)(wbnd[0] + sdx * i - bdata_origin_x);
          sdata[ndata++] = (float)(wbnd[2] + sdy * j - bdata_origin_y);
          sdata[ndata++] = (float)gridsmall[kgrid];
        }
      }
    ndata = ndata / 3;

    /* do the interpolation */
    float cay = (float)tension;
    float xmin = (float)(wbnd[0] - 0.5 * sdx - bdata_origin_x);
    float ymin = (float)(wbnd[2] - 0.5 * sdy - bdata_origin_y);
    float ddx = (float)sdx;
    float ddy = (float)sdy;
    fprintf(outfp, "\nDoing Zgrid spline interpolation with %d data points...\n", ndata);
    mb_zgrid2(sgrid, &sxdim, &sydim, &xmin, &ymin, &ddx, &ddy, sdata, &ndata, work1, work2, work3, &cay, &sclip);
#endif

    // float zflag = 5.0e34f;
    for (int i = 0; i < sxdim; i++)
      for (int j = 0; j < sydim; j++) {
        kgrid = i * sydim + j;
#ifdef USESURFACE
        kint = i + (sydim - j - 1) * sxdim;
#else
        kint = i + j * sxdim;
#endif
        if (cnt[kgrid] == 0) {
          gridsmall[kgrid] = sgrid[kint];
        }
      }

/* deallocate the interpolation arrays */
#ifdef USESURFACE
    mb_freed(verbose, __FILE__, __LINE__, (void **)&sxdata, &error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&sydata, &error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&szdata, &error);
#else
    mb_freed(verbose, __FILE__, __LINE__, (void **)&sdata, &error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&work1, &error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&work2, &error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&work3, &error);
#endif
    mb_freed(verbose, __FILE__, __LINE__, (void **)&sgrid, &error);

    /* do second pass footprint gridding using slope estimates from first pass interpolated grid */

    /* initialize arrays */
    for (int i = 0; i < gxdim; i++)
      for (int j = 0; j < gydim; j++) {
        kgrid = i * gydim + j;
        grid[kgrid] = 0.0;
        norm[kgrid] = 0.0;
        sigma[kgrid] = 0.0;
        firsttime[kgrid] = 0.0;
        num[kgrid] = 0;
        cnt[kgrid] = 0;
      }

    /* read in data */
    fprintf(outfp, "\nDoing second pass to generate final grid...\n");
    ndata = 0;
    if (mb_datalist_open(verbose, &datalist, dfile, look_processed, &error) != MB_SUCCESS) {
      error = MB_ERROR_OPEN_FAIL;
      fprintf(outfp, "\nUnable to open data list file: %s\n", filelist);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      mb_memory_clear(verbose, &memclear_error);
      exit(error);
    }
    while (mb_datalist_read3(verbose, datalist, &pstatus, path, ppath, &astatus, apath, dpath, &format, &file_weight, &error) ==
           MB_SUCCESS) {
      ndatafile = 0;

      /* if format > 0 then input is swath sonar file */
      if (format > 0 && path[0] != '#') {
        /* apply pstatus */
        if (pstatus == MB_PROCESSED_USE)
          strcpy(file, ppath);
        else
          strcpy(file, path);

        /* check for mbinfo file - get file bounds if possible */
        rformat = format;
        strcpy(rfile, file);
        status = mb_check_info(verbose, rfile, lonflip, bounds, &file_in_bounds, &error);
        if (status == MB_FAILURE) {
          file_in_bounds = true;
          status = MB_SUCCESS;
          error = MB_ERROR_NO_ERROR;
        }

        /* initialize the swath sonar file */
        bool first = true;
        double dmin = 0.0;
        double dmax = 0.0;
        if (file_in_bounds) {
          /* check for "fast bathymetry" or "fbt" file */
          if (datatype == MBGRID_DATA_TOPOGRAPHY || datatype == MBGRID_DATA_BATHYMETRY) {
            mb_get_fbt(verbose, rfile, &rformat, &error);
          }

          /* call mb_read_init_altnav() */
          if (mb_read_init_altnav(verbose, rfile, rformat, pings, lonflip, bounds, btime_i, etime_i, speedmin,
                                     timegap, astatus, apath, &mbio_ptr, &btime_d, &etime_d, 
                                     &beams_bath, &beams_amp, &pixels_ss,
                                     &error) != MB_SUCCESS) {
            char *message = nullptr;
            mb_error(verbose, error, &message);
            fprintf(outfp, "\nMBIO Error returned from function <mb_read_init_altnav>:\n%s\n", message);
            fprintf(outfp, "\nMultibeam File <%s> not initialized for reading\n", rfile);
            fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
            mb_memory_clear(verbose, &memclear_error);
            exit(error);
          }

          /* get mb_io_ptr */
          mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

          /* get topography type */
          status = mb_sonartype(verbose, mbio_ptr, mb_io_ptr->store_data, &topo_type, &error);

          /* allocate memory for reading data arrays */
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlon,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlat,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslon, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslat, &error);

          /* if error initializing memory then quit */
          if (error != MB_ERROR_NO_ERROR) {
            char *message = nullptr;
            mb_error(verbose, error, &message);
            fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
            fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
            mb_memory_clear(verbose, &memclear_error);
            exit(error);
          }

          /* loop over reading */
          while (error <= MB_ERROR_NO_ERROR) {
            status = mb_read(verbose, mbio_ptr, &kind, &rpings, time_i, &time_d, &navlon, &navlat, &speed, &heading,
                             &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath,
                             amp, bathlon, bathlat, ss, sslon, sslat, comment, &error);

            /* time gaps are not a problem here */
            if (error == MB_ERROR_TIME_GAP) {
              error = MB_ERROR_NO_ERROR;
              status = MB_SUCCESS;
            }

            if (verbose >= 2) {
              fprintf(outfp, "\ndbg2  Ping read in program <%s>\n", program_name);
              fprintf(outfp, "dbg2       kind:           %d\n", kind);
              fprintf(outfp, "dbg2       beams_bath:     %d\n", beams_bath);
              fprintf(outfp, "dbg2       beams_amp:      %d\n", beams_amp);
              fprintf(outfp, "dbg2       pixels_ss:      %d\n", pixels_ss);
              fprintf(outfp, "dbg2       error:          %d\n", error);
              fprintf(outfp, "dbg2       status:         %d\n", status);
            }

            if ((datatype == MBGRID_DATA_BATHYMETRY || datatype == MBGRID_DATA_TOPOGRAPHY) &&
                error == MB_ERROR_NO_ERROR) {

                            /* if needed try again to get topography type */
                            if (topo_type == MB_TOPOGRAPHY_TYPE_UNKNOWN) {
                                status = mb_sonartype(verbose, mbio_ptr, mb_io_ptr->store_data, &topo_type, &error);
                                if (topo_type == MB_TOPOGRAPHY_TYPE_UNKNOWN
                                    && mb_io_ptr->beamwidth_xtrack > 0.0 && mb_io_ptr->beamwidth_ltrack > 0.0) {
                                    topo_type = MB_TOPOGRAPHY_TYPE_MULTIBEAM;
                                }
                            }

              /* reproject beam positions if necessary */
              if (use_projection) {
                mb_proj_forward(verbose, pjptr, navlon, navlat, &navlon, &navlat, &error);
                for (ib = 0; ib < beams_bath; ib++)
                  if (mb_beam_ok(beamflag[ib]))
                    mb_proj_forward(verbose, pjptr, bathlon[ib], bathlat[ib], &bathlon[ib], &bathlat[ib],
                                    &error);
              }

              /* deal with data */
              for (ib = 0; ib < beams_bath; ib++)
                if (mb_beam_ok(beamflag[ib])) {
                  /* get position in grid */
                  ix = (bathlon[ib] - wbnd[0] + 0.5 * dx) / dx;
                  iy = (bathlat[ib] - wbnd[2] + 0.5 * dy) / dy;

                  /* deal with point data without footprint */
                  if (topo_type != MB_TOPOGRAPHY_TYPE_MULTIBEAM) {
                    if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
                      kgrid = ix * gydim + iy;
                      norm[kgrid] = norm[kgrid] + file_weight;
                      grid[kgrid] = grid[kgrid] + file_weight * topofactor * bath[ib];
                      sigma[kgrid] =
                          sigma[kgrid] + file_weight * topofactor * topofactor * bath[ib] * bath[ib];
                      num[kgrid]++;
                      cnt[kgrid]++;
                      ndata++;
                      ndatafile++;
                      if (first) {
                        first = false;
                        dmin = topofactor * bath[ib];
                        dmax = topofactor * bath[ib];
                      } else {
                        dmin = std::min(topofactor * bath[ib], dmin);
                        dmax = std::max(topofactor * bath[ib], dmax);
                      }
                    }
                  }

                  /* else deal with multibeam data that have beam footprints */
                  else {
                    /* get slope from low resolution grid */
                    int isx = (bathlon[ib] - wbnd[0] + 0.5 * sdx) / sdx;
                    int isy = (bathlat[ib] - wbnd[2] + 0.5 * sdy) / sdy;
                    isx = std::min(std::max(isx, 0), sxdim - 1);
                    isy = std::min(std::max(isy, 0), sydim - 1);
                    double dzdx;
                    double dzdy;
                    if (isx == 0) {
                      const int k1 = isx * sydim + isy;
                      const int k2 = (isx + 1) * sydim + isy;
                      dzdx = (gridsmall[k2] - gridsmall[k1]) / sdx;
                    } else if (isx == sxdim - 1) {
                      const int k1 = (isx - 1) * sydim + isy;
                      const int k2 = isx * sydim + isy;
                      dzdx = (gridsmall[k2] - gridsmall[k1]) / sdx;
                    } else {
                      const int k1 = (isx - 1) * sydim + isy;
                      const int k2 = (isx + 1) * sydim + isy;
                      dzdx = (gridsmall[k2] - gridsmall[k1]) / (2.0 * sdx);
                    }
                    if (isy == 0) {
                      const int k1 = isx * sydim + isy;
                      const int k2 = isx * sydim + (isy + 1);
                      dzdy = (gridsmall[k2] - gridsmall[k1]) / sdy;
                    } else if (isy == sydim - 1) {
                      const int k1 = isx * sydim + (isy - 1);
                      const int k2 = isx * sydim + isy;
                      dzdy = (gridsmall[k2] - gridsmall[k1]) / sdy;
                    } else {
                      const int k1 = isx * sydim + (isy - 1);
                      const int k2 = isx * sydim + (isy + 1);
                      dzdy = (gridsmall[k2] - gridsmall[k1]) / (2.0 * sdy);
                    }

                    /* check if in region of interest */
                    if (ix >= 0 && ix < gxdim
                      && iy >= 0 && iy < gydim) {
                      region_ok = true;
                    } else {
                      region_ok = false;
                    }

                    /* check if within allowed time */
                    time_ok = true;
                    if (region_ok && check_time) {
                      /* if in region of interest
                         check if time is ok */
                      kgrid = ix * gydim + iy;
                      if (firsttime[kgrid] <= 0.0) {
                        firsttime[kgrid] = time_d;
                        time_ok = true;
                      } else if (fabs(time_d - firsttime[kgrid]) > timediff) {
                        if (first_in_stays)
                          time_ok = false;
                        else {
                          time_ok = true;
                          firsttime[kgrid] = time_d;
                          ndata = ndata - cnt[kgrid];
                          ndatafile = ndatafile - cnt[kgrid];
                          norm[kgrid] = 0.0;
                          grid[kgrid] = 0.0;
                          sigma[kgrid] = 0.0;
                          num[kgrid] = 0;
                          cnt[kgrid] = 0;
                        }
                      }
                    }

                    /* calculate footprint if in region of interest */
                    if (region_ok && time_ok) {
                      /* calculate footprint  */
                      if (use_projection) {
                        foot_dx = (bathlon[ib] - navlon);
                        foot_dy = (bathlat[ib] - navlat);
                      }
                      else {
                        foot_dx = (bathlon[ib] - navlon) / mtodeglon;
                        foot_dy = (bathlat[ib] - navlat) / mtodeglat;
                      }
                      foot_lateral = sqrt(foot_dx * foot_dx + foot_dy * foot_dy);
                      if (foot_lateral > 0.0) {
                        foot_dxn = foot_dx / foot_lateral;
                        foot_dyn = foot_dy / foot_lateral;
                      }
                      else {
                        foot_dxn = 1.0;
                        foot_dyn = 0.0;
                      }
                      beam_altitude = bath[ib] - sensordepth;
                      foot_range = sqrt(foot_lateral * foot_lateral + beam_altitude * beam_altitude);
                      foot_theta = RTD * atan2(foot_lateral, beam_altitude);
                      if (foot_range > 0.0 && foot_theta < FOOT_THETA_MAX) {
                        footprint_ok = true;
                        foot_dtheta = 0.5 * scale * mb_io_ptr->beamwidth_xtrack;
                        foot_dphi = 0.5 * scale * mb_io_ptr->beamwidth_ltrack;
                        if (foot_dtheta <= 0.0)
                          foot_dtheta = 1.0;
                        if (foot_dphi <= 0.0)
                          foot_dphi = 1.0;
                        foot_hwidth = (bath[ib] - sensordepth) * tan(DTR * (foot_theta + foot_dtheta)) -
                                      foot_lateral;
                        foot_hlength = foot_range * tan(DTR * foot_dphi);
                      } else {
                        footprint_ok = false;
                      }
                    }

                    if (time_ok && region_ok && footprint_ok) {
                      /* get range of bins around footprint to examine */
                      if (use_projection) {
                        foot_wix = fabs(foot_hwidth * cos(DTR * foot_theta) / dx);
                        foot_wiy = fabs(foot_hwidth * sin(DTR * foot_theta) / dx);
                        foot_lix = fabs(foot_hlength * sin(DTR * foot_theta) / dy);
                        foot_liy = fabs(foot_hlength * cos(DTR * foot_theta) / dy);
                      }
                      else {
                        foot_wix = fabs(foot_hwidth * cos(DTR * foot_theta) * mtodeglon / dx);
                        foot_wiy = fabs(foot_hwidth * sin(DTR * foot_theta) * mtodeglon / dx);
                        foot_lix = fabs(foot_hlength * sin(DTR * foot_theta) * mtodeglat / dy);
                        foot_liy = fabs(foot_hlength * cos(DTR * foot_theta) * mtodeglat / dy);
                      }
                      foot_dix = 2 * std::max(foot_wix, foot_lix);
                      foot_diy = 2 * std::max(foot_wiy, foot_liy);
                      ix1 = std::max(ix - foot_dix, 0);
                      ix2 = std::min(ix + foot_dix, gxdim - 1);
                      iy1 = std::max(iy - foot_diy, 0);
                      iy2 = std::min(iy + foot_diy, gydim - 1);

                      /* loop over neighborhood of bins */
                      for (int ii = ix1; ii <= ix2; ii++)
                        for (int jj = iy1; jj <= iy2; jj++) {
                          /* find center of bin in lon lat degrees from sounding center */
                          kgrid = ii * gydim + jj;
                          xx = (wbnd[0] + ii * dx + 0.5 * dx - bathlon[ib]);
                          yy = (wbnd[2] + jj * dy + 0.5 * dy - bathlat[ib]);

                          /* get depth or topo value at this point using slope estimate */
                          sbath = topofactor * bath[ib] + dzdx * xx + dzdy * yy;

                          /* get center and corners of bin in meters from sounding center */
                          if (use_projection) {
                            xx0 = xx;
                            yy0 = yy;
                            bdx = 0.5 * dx;
                            bdy = 0.5 * dy;
                          }
                          else {
                            xx0 = xx / mtodeglon;
                            yy0 = yy / mtodeglat;
                            bdx = 0.5 * dx / mtodeglon;
                            bdy = 0.5 * dy / mtodeglat;
                          }
                          xx1 = xx0 - bdx;
                          xx2 = xx0 + bdx;
                          yy1 = yy0 - bdy;
                          yy2 = yy0 + bdy;

                          /* rotate center and corners of bin to footprint coordinates */
                          prx[0] = xx0 * foot_dxn + yy0 * foot_dyn;
                          pry[0] = -xx0 * foot_dyn + yy0 * foot_dxn;
                          prx[1] = xx1 * foot_dxn + yy1 * foot_dyn;
                          pry[1] = -xx1 * foot_dyn + yy1 * foot_dxn;
                          prx[2] = xx2 * foot_dxn + yy1 * foot_dyn;
                          pry[2] = -xx2 * foot_dyn + yy1 * foot_dxn;
                          prx[3] = xx1 * foot_dxn + yy2 * foot_dyn;
                          pry[3] = -xx1 * foot_dyn + yy2 * foot_dxn;
                          prx[4] = xx2 * foot_dxn + yy2 * foot_dyn;
                          pry[4] = -xx2 * foot_dyn + yy2 * foot_dxn;

                          /* get weight integrated over bin */
                          mbgrid_weight(verbose, foot_hwidth, foot_hlength, prx[0], pry[0], bdx,
                                        bdy, &prx[1], &pry[1], &weight, &use_weight, &error);

                          if (use_weight != MBGRID_USE_NO && weight > 0.000001) {
                            weight *= file_weight;
                            norm[kgrid] = norm[kgrid] + weight;
                            grid[kgrid] = grid[kgrid] + weight * sbath;
                            sigma[kgrid] = sigma[kgrid] + weight * sbath * sbath;
                            if (use_weight == MBGRID_USE_YES) {
                              num[kgrid]++;
                              if (ii == ix && jj == iy)
                                cnt[kgrid]++;
                            }
                          }
                        }
                      ndata++;
                      ndatafile++;
                      if (first) {
                        first = false;
                        dmin = topofactor * bath[ib];
                        dmax = topofactor * bath[ib];
                      } else {
                        dmin = std::min(topofactor * bath[ib], dmin);
                        dmax = std::max(topofactor * bath[ib], dmax);
                      }
                    }

                      /* else for xyz data without footprint */
                    else if (time_ok && region_ok) {
                      kgrid = ix * gydim + iy;
                      norm[kgrid] = norm[kgrid] + file_weight;
                      grid[kgrid] = grid[kgrid] + file_weight * topofactor * bath[ib];
                      sigma[kgrid] =
                          sigma[kgrid] + file_weight * topofactor * topofactor * bath[ib] * bath[ib];
                      num[kgrid]++;
                      cnt[kgrid]++;
                      ndata++;
                      ndatafile++;
                      if (first) {
                        first = false;
                        dmin = topofactor * bath[ib];
                        dmax = topofactor * bath[ib];
                      } else {
                        dmin = std::min(topofactor * bath[ib], dmin);
                        dmax = std::max(topofactor * bath[ib], dmax);
                      }
                    }
                  }
                }
              }
            }
          mb_close(verbose, &mbio_ptr, &error);
          status = MB_SUCCESS;
          error = MB_ERROR_NO_ERROR;
        }
        if (verbose >= 2)
          fprintf(outfp, "\n");
        if (verbose > 0)
          fprintf(outfp, "%d data points processed in %s (minmax: %f %f)\n", ndatafile, rfile, dmin, dmax);
        else if (file_in_bounds)
          fprintf(outfp, "%d data points processed in %s\n", ndatafile, rfile);
      } /* end if (format > 0) */
    }
    if (datalist != nullptr)
      mb_datalist_close(verbose, &datalist, &error);
    if (verbose > 0)
      fprintf(outfp, "\n%d total data points processed\n", ndata);

    /* now loop over all points in the output grid */
    if (verbose >= 1)
      fprintf(outfp, "\nMaking raw grid...\n");
    nbinset = 0;
    nbinzero = 0;
    nbinspline = 0;
    nbinbackground = 0;
    for (int i = 0; i < gxdim; i++)
      for (int j = 0; j < gydim; j++) {
        kgrid = i * gydim + j;
        if (num[kgrid] > 0) {
          grid[kgrid] = grid[kgrid] / norm[kgrid];
          factor = sigma[kgrid] / norm[kgrid] - grid[kgrid] * grid[kgrid];
          sigma[kgrid] = sqrt(fabs(factor));
          nbinset++;
        }
        else {
          grid[kgrid] = clipvalue;
          sigma[kgrid] = 0.0;
        }
      }

    /***** end of weighted footprint slope gridding *****/
  }

/* -------------------------------------------------------------------------- */
  /***** do weighted footprint gridding *****/
  else if (grid_mode == MBGRID_WEIGHTED_FOOTPRINT) {

    /* allocate memory for additional arrays */
    status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(double), (void **)&norm, &error);

    /* initialize arrays */
    for (int i = 0; i < gxdim; i++)
      for (int j = 0; j < gydim; j++) {
        kgrid = i * gydim + j;
        grid[kgrid] = 0.0;
        norm[kgrid] = 0.0;
        sigma[kgrid] = 0.0;
        firsttime[kgrid] = 0.0;
        num[kgrid] = 0;
        cnt[kgrid] = 0;
      }

    /* read in data */
    fprintf(outfp, "\nDoing single pass to generate grid...\n");
    ndata = 0;
    const int look_processed = MB_DATALIST_LOOK_UNSET;
    if (mb_datalist_open(verbose, &datalist, filelist, look_processed, &error) != MB_SUCCESS) {
      error = MB_ERROR_OPEN_FAIL;
      fprintf(outfp, "\nUnable to open data list file: %s\n", filelist);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      mb_memory_clear(verbose, &memclear_error);
      exit(error);
    }
    while (mb_datalist_read3(verbose, datalist, &pstatus, path, ppath, &astatus, apath, dpath, &format, &file_weight, &error) ==
           MB_SUCCESS) {
      ndatafile = 0;

      /* if format > 0 then input is swath sonar file */
      if (format > 0 && path[0] != '#') {
        /* apply pstatus */
        if (pstatus == MB_PROCESSED_USE)
          strcpy(file, ppath);
        else
          strcpy(file, path);

        /* check for mbinfo file - get file bounds if possible */
        rformat = format;
        strcpy(rfile, file);
        status = mb_check_info(verbose, rfile, lonflip, bounds, &file_in_bounds, &error);
        if (status == MB_FAILURE) {
          file_in_bounds = true;
          status = MB_SUCCESS;
          error = MB_ERROR_NO_ERROR;
        }

        /* initialize the swath sonar file */
        bool first = true;
        double dmin = 0.0;
        double dmax = 0.0;
        if (file_in_bounds) {
          /* check for "fast bathymetry" or "fbt" file */
          if (datatype == MBGRID_DATA_TOPOGRAPHY || datatype == MBGRID_DATA_BATHYMETRY) {
            mb_get_fbt(verbose, rfile, &rformat, &error);
          }

          /* call mb_read_init_altnav() */
          if (mb_read_init_altnav(verbose, rfile, rformat, pings, lonflip, bounds, btime_i, etime_i, speedmin,
                                     timegap, astatus, apath, &mbio_ptr, &btime_d, &etime_d, 
                                     &beams_bath, &beams_amp, &pixels_ss,
                                     &error) != MB_SUCCESS) {
            char *message = nullptr;
            mb_error(verbose, error, &message);
            fprintf(outfp, "\nMBIO Error returned from function <mb_read_init_altnav>:\n%s\n", message);
            fprintf(outfp, "\nMultibeam File <%s> not initialized for reading\n", rfile);
            fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
            mb_memory_clear(verbose, &memclear_error);
            exit(error);
          }

          /* get mb_io_ptr */
          mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

          /* get topography type */
          status = mb_sonartype(verbose, mbio_ptr, mb_io_ptr->store_data, &topo_type, &error);

          /* allocate memory for reading data arrays */
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlon,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlat,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslon, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslat, &error);

          /* if error initializing memory then quit */
          if (error != MB_ERROR_NO_ERROR) {
            char *message = nullptr;
            mb_error(verbose, error, &message);
            fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
            fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
            mb_memory_clear(verbose, &memclear_error);
            exit(error);
          }

          /* loop over reading */
          while (error <= MB_ERROR_NO_ERROR) {
            status = mb_read(verbose, mbio_ptr, &kind, &rpings, time_i, &time_d, &navlon, &navlat, &speed, &heading,
                             &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath,
                             amp, bathlon, bathlat, ss, sslon, sslat, comment, &error);

            /* time gaps are not a problem here */
            if (error == MB_ERROR_TIME_GAP) {
              error = MB_ERROR_NO_ERROR;
              status = MB_SUCCESS;
            }

            if (verbose >= 2) {
              fprintf(outfp, "\ndbg2  Ping read in program <%s>\n", program_name);
              fprintf(outfp, "dbg2       kind:           %d\n", kind);
              fprintf(outfp, "dbg2       beams_bath:     %d\n", beams_bath);
              fprintf(outfp, "dbg2       beams_amp:      %d\n", beams_amp);
              fprintf(outfp, "dbg2       pixels_ss:      %d\n", pixels_ss);
              fprintf(outfp, "dbg2       error:          %d\n", error);
              fprintf(outfp, "dbg2       status:         %d\n", status);
            }

            if ((datatype == MBGRID_DATA_BATHYMETRY || datatype == MBGRID_DATA_TOPOGRAPHY) &&
                error == MB_ERROR_NO_ERROR) {

                            /* if needed try again to get topography type */
                            if (topo_type == MB_TOPOGRAPHY_TYPE_UNKNOWN) {
                                status = mb_sonartype(verbose, mbio_ptr, mb_io_ptr->store_data, &topo_type, &error);
                                if (topo_type == MB_TOPOGRAPHY_TYPE_UNKNOWN
                                    && mb_io_ptr->beamwidth_xtrack > 0.0 && mb_io_ptr->beamwidth_ltrack > 0.0) {
                                    topo_type = MB_TOPOGRAPHY_TYPE_MULTIBEAM;
                                }
                            }

              /* reproject beam positions if necessary */
              if (use_projection) {
                mb_proj_forward(verbose, pjptr, navlon, navlat, &navlon, &navlat, &error);
                for (ib = 0; ib < beams_bath; ib++)
                  if (mb_beam_ok(beamflag[ib]))
                    mb_proj_forward(verbose, pjptr, bathlon[ib], bathlat[ib], &bathlon[ib], &bathlat[ib],
                                    &error);
              }

              /* deal with data */
              for (ib = 0; ib < beams_bath; ib++)
                if (mb_beam_ok(beamflag[ib])) {
                  /* get position in grid */
                  ix = (bathlon[ib] - wbnd[0] + 0.5 * dx) / dx;
                  iy = (bathlat[ib] - wbnd[2] + 0.5 * dy) / dy;

                  /* check if within allowed time */
                  if (check_time) {
                    /* if in region of interest
                       check if time is ok */
                    if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
                      kgrid = ix * gydim + iy;
                      if (firsttime[kgrid] <= 0.0) {
                        firsttime[kgrid] = time_d;
                        time_ok = true;
                      }
                      else if (fabs(time_d - firsttime[kgrid]) > timediff) {
                        if (first_in_stays)
                          time_ok = false;
                        else {
                          time_ok = true;
                          firsttime[kgrid] = time_d;
                          ndata = ndata - cnt[kgrid];
                          ndatafile = ndatafile - cnt[kgrid];
                          norm[kgrid] = 0.0;
                          grid[kgrid] = 0.0;
                          sigma[kgrid] = 0.0;
                          num[kgrid] = 0;
                          cnt[kgrid] = 0;
                        }
                      }
                      else
                        time_ok = true;
                    }
                    else
                      time_ok = true;
                  }
                  else
                    time_ok = true;

                  /* process if in region of interest */
                  if (ix >= 0 && ix < gxdim
                      && iy >= 0 && iy < gydim && time_ok) {
                    /* deal with point data without footprint */
                    if (topo_type != MB_TOPOGRAPHY_TYPE_MULTIBEAM) {
                      kgrid = ix * gydim + iy;
                      norm[kgrid] = norm[kgrid] + file_weight;
                      grid[kgrid] = grid[kgrid] + file_weight * topofactor * bath[ib];
                      sigma[kgrid] =
                          sigma[kgrid] + file_weight * topofactor * topofactor * bath[ib] * bath[ib];
                      num[kgrid]++;
                      cnt[kgrid]++;
                      ndata++;
                      ndatafile++;
                      if (first) {
                        first = false;
                        dmin = topofactor * bath[ib];
                        dmax = topofactor * bath[ib];
                      } else {
                        dmin = std::min(topofactor * bath[ib], dmin);
                        dmax = std::max(topofactor * bath[ib], dmax);
                      }
                    }

                    /* else deal with multibeam data that have beam footprints */
                    else {
                      /* calculate footprint - this is a kluge assuming
                         sonar at surface - also assumes lon lat grid
                         - to be generalized in later version
                         DWC 11/16/99 */
                      /* calculate footprint - now uses sonar altitude
                         - still assumes lon lat grid
                         - to be generalized in later version
                         DWC 1/29/2001 */
                      /* now handles projected grids
                         DWC 3/5/2003 */
                      if (use_projection) {
                        foot_dx = (bathlon[ib] - navlon);
                        foot_dy = (bathlat[ib] - navlat);
                      }
                      else {
                        foot_dx = (bathlon[ib] - navlon) / mtodeglon;
                        foot_dy = (bathlat[ib] - navlat) / mtodeglat;
                      }
                      foot_lateral = sqrt(foot_dx * foot_dx + foot_dy * foot_dy);
                      if (foot_lateral > 0.0) {
                        foot_dxn = foot_dx / foot_lateral;
                        foot_dyn = foot_dy / foot_lateral;
                      }
                      else {
                        foot_dxn = 1.0;
                        foot_dyn = 0.0;
                      }
                      foot_range = sqrt(foot_lateral * foot_lateral + altitude * altitude);
                      if (foot_range > 0.0) {
                        foot_theta = RTD * atan2(foot_lateral, (bath[ib] - sensordepth));
                        foot_dtheta = 0.5 * scale * mb_io_ptr->beamwidth_xtrack;
                        foot_dphi = 0.5 * scale * mb_io_ptr->beamwidth_ltrack;
                        if (foot_dtheta <= 0.0)
                          foot_dtheta = 1.0;
                        if (foot_dphi <= 0.0)
                          foot_dphi = 1.0;
                        foot_hwidth = (bath[ib] - sensordepth) * tan(DTR * (foot_theta + foot_dtheta)) -
                                      foot_lateral;
                        foot_hlength = foot_range * tan(DTR * foot_dphi);

                        /* get range of bins around footprint to examine */
                        if (use_projection) {
                          foot_wix = fabs(foot_hwidth * cos(DTR * foot_theta) / dx);
                          foot_wiy = fabs(foot_hwidth * sin(DTR * foot_theta) / dx);
                          foot_lix = fabs(foot_hlength * sin(DTR * foot_theta) / dy);
                          foot_liy = fabs(foot_hlength * cos(DTR * foot_theta) / dy);
                        }
                        else {
                          foot_wix = fabs(foot_hwidth * cos(DTR * foot_theta) * mtodeglon / dx);
                          foot_wiy = fabs(foot_hwidth * sin(DTR * foot_theta) * mtodeglon / dx);
                          foot_lix = fabs(foot_hlength * sin(DTR * foot_theta) * mtodeglat / dy);
                          foot_liy = fabs(foot_hlength * cos(DTR * foot_theta) * mtodeglat / dy);
                        }
                        foot_dix = 2 * std::max(foot_wix, foot_lix);
                        foot_diy = 2 * std::max(foot_wiy, foot_liy);
                        ix1 = std::max(ix - foot_dix, 0);
                        ix2 = std::min(ix + foot_dix, gxdim - 1);
                        iy1 = std::max(iy - foot_diy, 0);
                        iy2 = std::min(iy + foot_diy, gydim - 1);

                        /* loop over neighborhood of bins */
                        for (int ii = ix1; ii <= ix2; ii++)
                          for (int jj = iy1; jj <= iy2; jj++) {
                            /* find center of bin in lon lat degrees from sounding center */
                            kgrid = ii * gydim + jj;
                            xx = (wbnd[0] + ii * dx + 0.5 * dx - bathlon[ib]);
                            yy = (wbnd[2] + jj * dy + 0.5 * dy - bathlat[ib]);

                            /* get depth or topo value at this point */
                            sbath = topofactor * bath[ib];

                            /* get center and corners of bin in meters from sounding center */
                            if (use_projection) {
                              xx0 = xx;
                              yy0 = yy;
                              bdx = 0.5 * dx;
                              bdy = 0.5 * dy;
                            }
                            else {
                              xx0 = xx / mtodeglon;
                              yy0 = yy / mtodeglat;
                              bdx = 0.5 * dx / mtodeglon;
                              bdy = 0.5 * dy / mtodeglat;
                            }
                            xx1 = xx0 - bdx;
                            xx2 = xx0 + bdx;
                            yy1 = yy0 - bdy;
                            yy2 = yy0 + bdy;

                            /* rotate center and corners of bin to footprint coordinates */
                            prx[0] = xx0 * foot_dxn + yy0 * foot_dyn;
                            pry[0] = -xx0 * foot_dyn + yy0 * foot_dxn;
                            prx[1] = xx1 * foot_dxn + yy1 * foot_dyn;
                            pry[1] = -xx1 * foot_dyn + yy1 * foot_dxn;
                            prx[2] = xx2 * foot_dxn + yy1 * foot_dyn;
                            pry[2] = -xx2 * foot_dyn + yy1 * foot_dxn;
                            prx[3] = xx1 * foot_dxn + yy2 * foot_dyn;
                            pry[3] = -xx1 * foot_dyn + yy2 * foot_dxn;
                            prx[4] = xx2 * foot_dxn + yy2 * foot_dyn;
                            pry[4] = -xx2 * foot_dyn + yy2 * foot_dxn;

                            /* get weight integrated over bin */
                            mbgrid_weight(verbose, foot_hwidth, foot_hlength, prx[0], pry[0], bdx,
                                          bdy, &prx[1], &pry[1], &weight, &use_weight, &error);

                            if (use_weight != MBGRID_USE_NO && weight > 0.000001) {
                              weight *= file_weight;
                              norm[kgrid] = norm[kgrid] + weight;
                              grid[kgrid] = grid[kgrid] + weight * sbath;
                              sigma[kgrid] = sigma[kgrid] + weight * sbath * sbath;
                              if (use_weight == MBGRID_USE_YES) {
                                num[kgrid]++;
                                if (ii == ix && jj == iy)
                                  cnt[kgrid]++;
                              }
                            }
                          }
                        ndata++;
                        ndatafile++;
                        if (first) {
                          first = false;
                          dmin = topofactor * bath[ib];
                          dmax = topofactor * bath[ib];
                        } else {
                          dmin = std::min(topofactor * bath[ib], dmin);
                          dmax = std::max(topofactor * bath[ib], dmax);
                        }
                      }

                      /* else for xyz data without footprint */
                      else if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
                        kgrid = ix * gydim + iy;
                        norm[kgrid] = norm[kgrid] + file_weight;
                        grid[kgrid] = grid[kgrid] + file_weight * topofactor * bath[ib];
                        sigma[kgrid] =
                            sigma[kgrid] + file_weight * topofactor * topofactor * bath[ib] * bath[ib];
                        num[kgrid]++;
                        cnt[kgrid]++;
                        ndata++;
                        ndatafile++;
                        if (first) {
                          first = false;
                          dmin = topofactor * bath[ib];
                          dmax = topofactor * bath[ib];
                        } else {
                          dmin = std::min(topofactor * bath[ib], dmin);
                          dmax = std::max(topofactor * bath[ib], dmax);
                        }
                      }
                    }
                  }
                }
            }
          }
          mb_close(verbose, &mbio_ptr, &error);
          status = MB_SUCCESS;
          error = MB_ERROR_NO_ERROR;
        }
        if (verbose >= 2)
          fprintf(outfp, "\n");
        if (verbose > 0)
          fprintf(outfp, "%d data points processed in %s (minmax: %f %f)\n", ndatafile, rfile, dmin, dmax);
        else if (file_in_bounds)
          fprintf(outfp, "%d data points processed in %s\n", ndatafile, rfile);

        /* add to datalist if data actually contributed */
        if (ndatafile > 0 && dfp != nullptr) {
          if (pstatus == MB_PROCESSED_USE && astatus == MB_ALTNAV_USE)
            fprintf(dfp, "A:%s %d %f %s\n", path, format, file_weight, apath);
          else if (pstatus == MB_PROCESSED_USE)
            fprintf(dfp, "P:%s %d %f\n", path, format, file_weight);
          else
            fprintf(dfp, "R:%s %d %f\n", path, format, file_weight);
          fflush(dfp);
        }
      } /* end if (format > 0) */
    }
    if (datalist != nullptr)
      mb_datalist_close(verbose, &datalist, &error);
    if (verbose > 0)
      fprintf(outfp, "\n%d total data points processed\n", ndata);

    /* close datalist if necessary */
    if (dfp != nullptr) {
      fclose(dfp);
      dfp = nullptr;
    }

    /* now loop over all points in the output grid */
    if (verbose >= 1)
      fprintf(outfp, "\nMaking raw grid...\n");
    nbinset = 0;
    nbinzero = 0;
    nbinspline = 0;
    nbinbackground = 0;
    for (int i = 0; i < gxdim; i++)
      for (int j = 0; j < gydim; j++) {
        kgrid = i * gydim + j;
        if (num[kgrid] > 0) {
          grid[kgrid] = grid[kgrid] / norm[kgrid];
          factor = sigma[kgrid] / norm[kgrid] - grid[kgrid] * grid[kgrid];
          sigma[kgrid] = sqrt(fabs(factor));
          nbinset++;
        }
        else {
          grid[kgrid] = clipvalue;
          sigma[kgrid] = 0.0;
        }
      }

    /***** end of weighted footprint gridding *****/
  }
/* -------------------------------------------------------------------------- */
  /***** else do median filtering gridding *****/
  else if (grid_mode == MBGRID_MEDIAN_FILTER) {

    /* allocate memory for additional arrays */
    status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(double *), (void **)&data, &error);

    /* if error initializing memory then quit */
    if (error != MB_ERROR_NO_ERROR) {
      char *message = nullptr;
      mb_error(verbose, error, &message);
      fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      mb_memory_clear(verbose, &memclear_error);
      exit(error);
    }

    /* initialize arrays */
    for (int i = 0; i < gxdim; i++)
      for (int j = 0; j < gydim; j++) {
        kgrid = i * gydim + j;
        grid[kgrid] = 0.0;
        sigma[kgrid] = 0.0;
        firsttime[kgrid] = 0.0;
        cnt[kgrid] = 0;
        num[kgrid] = 0;
        data[kgrid] = nullptr;
      }

    /* read in data */
    ndata = 0;
    const int look_processed = MB_DATALIST_LOOK_UNSET;
    if (mb_datalist_open(verbose, &datalist, filelist, look_processed, &error) != MB_SUCCESS) {
      error = MB_ERROR_OPEN_FAIL;
      fprintf(outfp, "\nUnable to open data list file: %s\n", filelist);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      mb_memory_clear(verbose, &memclear_error);
      exit(error);
    }
    while (mb_datalist_read3(verbose, datalist, &pstatus, path, ppath, &astatus, apath, dpath, &format, &file_weight, &error) ==
           MB_SUCCESS) {
      ndatafile = 0;

      /* if format > 0 then input is swath sonar file */
      if (format > 0 && path[0] != '#') {
        /* apply pstatus */
        if (pstatus == MB_PROCESSED_USE)
          strcpy(file, ppath);
        else
          strcpy(file, path);

        /* check for mbinfo file - get file bounds if possible */
        rformat = format;
        strcpy(rfile, file);
        status = mb_check_info(verbose, file, lonflip, bounds, &file_in_bounds, &error);
        if (status == MB_FAILURE) {
          file_in_bounds = true;
          status = MB_SUCCESS;
          error = MB_ERROR_NO_ERROR;
        }

        /* initialize the swath sonar file */
        bool first = true;
        double dmin = 0.0;
        double dmax = 0.0;
        if (file_in_bounds) {
          /* check for "fast bathymetry" or "fbt" file */
          if (datatype == MBGRID_DATA_TOPOGRAPHY || datatype == MBGRID_DATA_BATHYMETRY) {
            mb_get_fbt(verbose, rfile, &rformat, &error);
          }

          /* call mb_read_init_altnav() */
          if (mb_read_init_altnav(verbose, rfile, rformat, pings, lonflip, bounds, btime_i, etime_i, speedmin,
                                     timegap, astatus, apath, &mbio_ptr, &btime_d, &etime_d, 
                                     &beams_bath, &beams_amp, &pixels_ss,
                                     &error) != MB_SUCCESS) {
            char *message = nullptr;
            mb_error(verbose, error, &message);
            fprintf(outfp, "\nMBIO Error returned from function <mb_read_init_altnav>:\n%s\n", message);
            fprintf(outfp, "\nMultibeam File <%s> not initialized for reading\n", rfile);
            fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
            mb_memory_clear(verbose, &memclear_error);
            exit(error);
          }

          /* allocate memory for reading data arrays */
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlon,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlat,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslon, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslat, &error);

          /* if error initializing memory then quit */
          if (error != MB_ERROR_NO_ERROR) {
            char *message = nullptr;
            mb_error(verbose, error, &message);
            fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
            fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
            mb_memory_clear(verbose, &memclear_error);
            exit(error);
          }

          /* loop over reading */
          while (error <= MB_ERROR_NO_ERROR) {
            status = mb_read(verbose, mbio_ptr, &kind, &rpings, time_i, &time_d, &navlon, &navlat, &speed, &heading,
                             &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath,
                             amp, bathlon, bathlat, ss, sslon, sslat, comment, &error);

            /* time gaps are not a problem here */
            if (error == MB_ERROR_TIME_GAP) {
              error = MB_ERROR_NO_ERROR;
              status = MB_SUCCESS;
            }

            if (verbose >= 2) {
              fprintf(outfp, "\ndbg2  Ping read in program <%s>\n", program_name);
              fprintf(outfp, "dbg2       kind:           %d\n", kind);
              fprintf(outfp, "dbg2       beams_bath:     %d\n", beams_bath);
              fprintf(outfp, "dbg2       beams_amp:      %d\n", beams_amp);
              fprintf(outfp, "dbg2       pixels_ss:      %d\n", pixels_ss);
              fprintf(outfp, "dbg2       error:          %d\n", error);
              fprintf(outfp, "dbg2       status:         %d\n", status);
            }

            if ((datatype == MBGRID_DATA_BATHYMETRY || datatype == MBGRID_DATA_TOPOGRAPHY) &&
                error == MB_ERROR_NO_ERROR) {

              /* reproject beam positions if necessary */
              if (use_projection) {
                for (ib = 0; ib < beams_bath; ib++)
                  if (mb_beam_ok(beamflag[ib]))
                    mb_proj_forward(verbose, pjptr, bathlon[ib], bathlat[ib], &bathlon[ib], &bathlat[ib],
                                    &error);
              }

              /* deal with data */
              for (ib = 0; ib < beams_bath; ib++)
                if (mb_beam_ok(beamflag[ib])) {
                  ix = (bathlon[ib] - wbnd[0] + 0.5 * dx) / dx;
                  iy = (bathlat[ib] - wbnd[2] + 0.5 * dy) / dy;
                  if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
                    /* check if within allowed time */
                    kgrid = ix * gydim + iy;
                    if (check_time)
                      time_ok = true;
                    else {
                      if (firsttime[kgrid] <= 0.0) {
                        firsttime[kgrid] = time_d;
                        time_ok = true;
                      }
                      else if (fabs(time_d - firsttime[kgrid]) > timediff) {
                        if (first_in_stays)
                          time_ok = false;
                        else {
                          time_ok = true;
                          firsttime[kgrid] = time_d;
                          ndata = ndata - cnt[kgrid];
                          ndatafile = ndatafile - cnt[kgrid];
                          cnt[kgrid] = 0;
                        }
                      }
                      else
                        time_ok = true;
                    }

                    /* make sure there is space for the data */
                    if (time_ok && cnt[kgrid] >= num[kgrid]) {
                      num[kgrid] += REALLOC_STEP_SIZE;
                      if ((data[kgrid] = (double *)realloc(data[kgrid], num[kgrid] * sizeof(double))) ==
                          nullptr) {
                        error = MB_ERROR_MEMORY_FAIL;
                        char *message = nullptr;
                        mb_error(verbose, error, &message);
                        fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
                        fprintf(outfp, "The weighted mean algorithm uses much less\n");
                        fprintf(outfp, "memory than the median filter algorithm.\n");
                        fprintf(outfp, "You could also try using ping averaging to\n");
                        fprintf(outfp, "reduce the number of data points to be gridded.\n");
                        fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
                        mb_memory_clear(verbose, &memclear_error);
                        exit(error);
                      }
                    }

                    /* process it */
                    if (time_ok) {
                      value = data[kgrid];
                      value[cnt[kgrid]] = topofactor * bath[ib];
                      cnt[kgrid]++;
                      ndata++;
                      ndatafile++;
                      if (first) {
                        first = false;
                        dmin = topofactor * bath[ib];
                        dmax = topofactor * bath[ib];
                      } else {
                        dmin = std::min(topofactor * bath[ib], dmin);
                        dmax = std::max(topofactor * bath[ib], dmax);
                      }
                    }
                  }
                }
            }
            else if (datatype == MBGRID_DATA_AMPLITUDE && error == MB_ERROR_NO_ERROR) {

              /* reproject beam positions if necessary */
              if (use_projection) {
                for (ib = 0; ib < beams_amp; ib++)
                  if (mb_beam_ok(beamflag[ib]))
                    mb_proj_forward(verbose, pjptr, bathlon[ib], bathlat[ib], &bathlon[ib], &bathlat[ib],
                                    &error);
              }

              /* deal with data */
              for (ib = 0; ib < beams_bath; ib++)
                if (mb_beam_ok(beamflag[ib])) {
                  ix = (bathlon[ib] - wbnd[0] + 0.5 * dx) / dx;
                  iy = (bathlat[ib] - wbnd[2] + 0.5 * dy) / dy;
                  if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
                    /* check if within allowed time */
                    kgrid = ix * gydim + iy;
                    if (!check_time)
                      time_ok = true;
                    else {
                      if (firsttime[kgrid] <= 0.0) {
                        firsttime[kgrid] = time_d;
                        time_ok = true;
                      }
                      else if (fabs(time_d - firsttime[kgrid]) > timediff) {
                        if (first_in_stays)
                          time_ok = false;
                        else {
                          time_ok = true;
                          firsttime[kgrid] = time_d;
                          ndata = ndata - cnt[kgrid];
                          ndatafile = ndatafile - cnt[kgrid];
                          cnt[kgrid] = 0;
                        }
                      }
                      else
                        time_ok = true;
                    }

                    /* make sure there is space for the data */
                    if (time_ok && cnt[kgrid] >= num[kgrid]) {
                      num[kgrid] += REALLOC_STEP_SIZE;
                      if ((data[kgrid] = (double *)realloc(data[kgrid], num[kgrid] * sizeof(double))) ==
                          nullptr) {
                        error = MB_ERROR_MEMORY_FAIL;
                        char *message = nullptr;
                        mb_error(verbose, error, &message);
                        fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
                        fprintf(outfp, "The weighted mean algorithm uses much less\n");
                        fprintf(outfp, "memory than the median filter algorithm.\n");
                        fprintf(outfp, "You could also try using ping averaging to\n");
                        fprintf(outfp, "reduce the number of data points to be gridded.\n");
                        fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
                        mb_memory_clear(verbose, &memclear_error);
                        exit(error);
                      }
                    }

                    /* process it */
                    if (time_ok) {
                      value = data[kgrid];
                      value[cnt[kgrid]] = amp[ib];
                      cnt[kgrid]++;
                      ndata++;
                      ndatafile++;
                      if (first) {
                        first = false;
                        dmin = amp[ib];
                        dmax = amp[ib];
                      } else {
                        dmin = std::min(amp[ib], dmin);
                        dmax = std::max(amp[ib], dmax);
                      }
                    }
                  }
                }
            }
            else if (datatype == MBGRID_DATA_SIDESCAN && error == MB_ERROR_NO_ERROR) {

              /* reproject pixel positions if necessary */
              if (use_projection) {
                for (ib = 0; ib < pixels_ss; ib++)
                  if (ss[ib] > MB_SIDESCAN_NULL)
                    mb_proj_forward(verbose, pjptr, sslon[ib], sslat[ib], &sslon[ib], &sslat[ib], &error);
              }

              /* deal with data */
              for (ib = 0; ib < pixels_ss; ib++)
                if (ss[ib] > MB_SIDESCAN_NULL) {
                  ix = (sslon[ib] - wbnd[0] + 0.5 * dx) / dx;
                  iy = (sslat[ib] - wbnd[2] + 0.5 * dy) / dy;
                  if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
                    /* check if within allowed time */
                    kgrid = ix * gydim + iy;
                    if (!check_time)
                      time_ok = true;
                    else {
                      if (firsttime[kgrid] <= 0.0) {
                        firsttime[kgrid] = time_d;
                        time_ok = true;
                      }
                      else if (fabs(time_d - firsttime[kgrid]) > timediff) {
                        if (first_in_stays)
                          time_ok = false;
                        else {
                          time_ok = true;
                          firsttime[kgrid] = time_d;
                          ndata = ndata - cnt[kgrid];
                          ndatafile = ndatafile - cnt[kgrid];
                          cnt[kgrid] = 0;
                        }
                      }
                      else
                        time_ok = true;
                    }

                    /* make sure there is space for the data */
                    if (time_ok && cnt[kgrid] >= num[kgrid]) {
                      num[kgrid] += REALLOC_STEP_SIZE;
                      if ((data[kgrid] = (double *)realloc(data[kgrid], num[kgrid] * sizeof(double))) ==
                          nullptr) {
                        error = MB_ERROR_MEMORY_FAIL;
                        char *message = nullptr;
                        mb_error(verbose, error, &message);
                        fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
                        fprintf(outfp, "The weighted mean algorithm uses much less\n");
                        fprintf(outfp, "memory than the median filter algorithm.\n");
                        fprintf(outfp, "You could also try using ping averaging to\n");
                        fprintf(outfp, "reduce the number of data points to be gridded.\n");
                        fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
                        mb_memory_clear(verbose, &memclear_error);
                        exit(error);
                      }
                    }

                    /* process it */
                    if (time_ok) {
                      value = data[kgrid];
                      value[cnt[kgrid]] = ss[ib];
                      cnt[kgrid]++;
                      ndata++;
                      ndatafile++;
                      if (first) {
                        first = false;
                        dmin = ss[ib];
                        dmax = ss[ib];
                      } else {
                        dmin = std::min(ss[ib], dmin);
                        dmax = std::max(ss[ib], dmax);
                      }
                    }
                  }
                }
            }
          }
          mb_close(verbose, &mbio_ptr, &error);
          status = MB_SUCCESS;
          error = MB_ERROR_NO_ERROR;
        }
        if (verbose >= 2)
          fprintf(outfp, "\n");
        if (verbose > 0)
          fprintf(outfp, "%d data points processed in %s (minmax: %f %f)\n", ndatafile, rfile, dmin, dmax);
        else if (file_in_bounds)
          fprintf(outfp, "%d data points processed in %s\n", ndatafile, rfile);

        /* add to datalist if data actually contributed */
        if (ndatafile > 0 && dfp != nullptr) {
          if (pstatus == MB_PROCESSED_USE && astatus == MB_ALTNAV_USE)
            fprintf(dfp, "A:%s %d %f %s\n", path, format, file_weight, apath);
          else if (pstatus == MB_PROCESSED_USE)
            fprintf(dfp, "P:%s %d %f\n", path, format, file_weight);
          else
            fprintf(dfp, "R:%s %d %f\n", path, format, file_weight);
          fflush(dfp);
        }
      } /* end if (format > 0) */

      /* if format == 0 then input is lon,lat,values triples file */
      else if (format == 0 && path[0] != '#') {

        /* open data file */
        if ((rfp = fopen(path, "r")) == nullptr) {
          error = MB_ERROR_OPEN_FAIL;
          fprintf(outfp, "\nUnable to open lon,lat,value triples data path: %s\n", path);
          fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
          mb_memory_clear(verbose, &memclear_error);
          exit(error);
        }

        /* loop over reading */
        bool first = true;
        double dmin = 0.0;
        double dmax = 0.0;
        while (fscanf(rfp, "%lf %lf %lf", &tlon, &tlat, &tvalue) != EOF) {
          /* reproject data positions if necessary */
          if (use_projection)
            mb_proj_forward(verbose, pjptr, tlon, tlat, &tlon, &tlat, &error);

          /* get position in grid */
          ix = (tlon - wbnd[0] + 0.5 * dx) / dx;
          iy = (tlat - wbnd[2] + 0.5 * dy) / dy;
          if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
            /* check if overwriting */
            kgrid = ix * gydim + iy;
            if (!check_time)
              time_ok = true;
            else {
              if (firsttime[kgrid] > 0.0)
                time_ok = false;
              else
                time_ok = true;
            }

            /* make sure there is space for the data */
            if (time_ok && cnt[kgrid] >= num[kgrid]) {
              num[kgrid] += REALLOC_STEP_SIZE;
              if ((data[kgrid] = (double *)realloc(data[kgrid], num[kgrid] * sizeof(double))) == nullptr) {
                error = MB_ERROR_MEMORY_FAIL;
                char *message = nullptr;
                mb_error(verbose, error, &message);
                fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
                fprintf(outfp, "The weighted mean algorithm uses much less\n");
                fprintf(outfp, "memory than the median filter algorithm.\n");
                fprintf(outfp, "You could also try using ping averaging to\n");
                fprintf(outfp, "reduce the number of data points to be gridded.\n");
                fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
                mb_memory_clear(verbose, &memclear_error);
                exit(error);
              }
            }

            /* process it */
            if (time_ok) {
              value = data[kgrid];
              value[cnt[kgrid]] = topofactor * tvalue;
              cnt[kgrid]++;
              ndata++;
              ndatafile++;
              if (first) {
                first = false;
                dmin = topofactor * tvalue;
                dmax = topofactor * tvalue;
              } else {
                dmin = std::min(topofactor * tvalue, dmin);
                dmax = std::max(topofactor * tvalue, dmax);
              }
            }
          }
        }
        fclose(rfp);
        status = MB_SUCCESS;
        error = MB_ERROR_NO_ERROR;
        if (verbose >= 2)
          fprintf(outfp, "\n");
        if (verbose > 0)
          fprintf(outfp, "%d data points processed in %s (minmax: %f %f)\n", ndatafile, file, dmin, dmax);
        else if (ndatafile > 0)
          fprintf(outfp, "%d data points processed in %s\n", ndatafile, file);

        /* add to datalist if data actually contributed */
        if (ndatafile > 0 && dfp != nullptr) {
          if (pstatus == MB_PROCESSED_USE && astatus == MB_ALTNAV_USE)
            fprintf(dfp, "A:%s %d %f %s\n", path, format, file_weight, apath);
          else if (pstatus == MB_PROCESSED_USE)
            fprintf(dfp, "P:%s %d %f\n", path, format, file_weight);
          else
            fprintf(dfp, "R:%s %d %f\n", path, format, file_weight);
          fflush(dfp);
        }
      } /* end if (format == 0) */
    }
    if (datalist != nullptr)
      mb_datalist_close(verbose, &datalist, &error);
    if (verbose > 0)
      fprintf(outfp, "\n%d total data points processed\n", ndata);

    /* close datalist if necessary */
    if (dfp != nullptr) {
      fclose(dfp);
      dfp = nullptr;
    }

    /* now loop over all points in the output grid */
    if (verbose >= 1)
      fprintf(outfp, "\nMaking raw grid...\n");
    nbinset = 0;
    nbinzero = 0;
    nbinspline = 0;
    nbinbackground = 0;
    for (int i = 0; i < gxdim; i++)
      for (int j = 0; j < gydim; j++) {
        kgrid = i * gydim + j;
        if (cnt[kgrid] > 0) {
          value = data[kgrid];
          qsort((char *)value, cnt[kgrid], sizeof(double), mb_double_compare);
          grid[kgrid] = value[cnt[kgrid] / 2];
          sigma[kgrid] = 0.0;
          for (int k = 0; k < cnt[kgrid]; k++)
            sigma[kgrid] += (value[k] - grid[kgrid]) * (value[k] - grid[kgrid]);
          if (cnt[kgrid] > 1)
            sigma[kgrid] = sqrt(sigma[kgrid] / (cnt[kgrid] - 1));
          else
            sigma[kgrid] = 0.0;
          nbinset++;
        }
        else
          grid[kgrid] = clipvalue;
      }

    /* now deallocate space for the data */
    for (int i = 0; i < gxdim; i++)
      for (int j = 0; j < gydim; j++) {
        kgrid = i * gydim + j;
        if (cnt[kgrid] > 0)
          free(data[kgrid]);
      }

    /***** end of median filter gridding *****/
  }
/* -------------------------------------------------------------------------- */

  /***** do weighted mean or min/max gridding *****/
  else if (grid_mode == MBGRID_WEIGHTED_MEAN
            || grid_mode == MBGRID_MINIMUM_FILTER
            || grid_mode == MBGRID_MAXIMUM_FILTER) {

    /* allocate memory for additional arrays */
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(double), (void **)&norm, &error);

    /* if error initializing memory then quit */
    if (error != MB_ERROR_NO_ERROR) {
      char *message = nullptr;
      mb_error(verbose, error, &message);
      fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      mb_memory_clear(verbose, &memclear_error);
      exit(error);
    }

    /* initialize arrays */
    for (int i = 0; i < gxdim; i++)
      for (int j = 0; j < gydim; j++) {
        kgrid = i * gydim + j;
        grid[kgrid] = 0.0;
        norm[kgrid] = 0.0;
        sigma[kgrid] = 0.0;
        firsttime[kgrid] = 0.0;
        num[kgrid] = 0;
        cnt[kgrid] = 0;
      }

    /* read in data */
    ndata = 0;
    const int look_processed = MB_DATALIST_LOOK_UNSET;
    if (mb_datalist_open(verbose, &datalist, filelist, look_processed, &error) != MB_SUCCESS) {
      error = MB_ERROR_OPEN_FAIL;
      fprintf(outfp, "\nUnable to open data list file: %s\n", filelist);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      mb_memory_clear(verbose, &memclear_error);
      exit(error);
    }
    while (mb_datalist_read3(verbose, datalist, &pstatus, path, ppath, &astatus, apath, dpath, &format, &file_weight, &error) ==
           MB_SUCCESS) {
      ndatafile = 0;

      /* if format > 0 then input is swath sonar file */
      if (format > 0 && path[0] != '#') {
        /* apply pstatus */
        if (pstatus == MB_PROCESSED_USE)
          strcpy(file, ppath);
        else
          strcpy(file, path);

        /* check for mbinfo file - get file bounds if possible */
        rformat = format;
        strcpy(rfile, file);
        status = mb_check_info(verbose, rfile, lonflip, bounds, &file_in_bounds, &error);
        if (status == MB_FAILURE) {
          file_in_bounds = true;
          status = MB_SUCCESS;
          error = MB_ERROR_NO_ERROR;
        }

        /* initialize the swath sonar file */
        bool first = true;
        double dmin = 0.0;
        double dmax = 0.0;
        if (file_in_bounds) {
          /* check for "fast bathymetry" or "fbt" file */
          if (datatype == MBGRID_DATA_TOPOGRAPHY || datatype == MBGRID_DATA_BATHYMETRY) {
            mb_get_fbt(verbose, rfile, &rformat, &error);
          }

          /* call mb_read_init_altnav() */
          if (mb_read_init_altnav(verbose, rfile, rformat, pings, lonflip, bounds, btime_i, etime_i, speedmin,
                                     timegap, astatus, apath, &mbio_ptr, &btime_d, &etime_d, 
                                     &beams_bath, &beams_amp, &pixels_ss,
                                     &error) != MB_SUCCESS) {
            char *message = nullptr;
            mb_error(verbose, error, &message);
            fprintf(outfp, "\nMBIO Error returned from function <mb_read_init_altnav>:\n%s\n", message);
            fprintf(outfp, "\nMultibeam File <%s> not initialized for reading\n", rfile);
            fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
            mb_memory_clear(verbose, &memclear_error);
            exit(error);
          }

          /* allocate memory for reading data arrays */
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlon,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlat,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslon, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslat, &error);

          /* if error initializing memory then quit */
          if (error != MB_ERROR_NO_ERROR) {
            char *message = nullptr;
            mb_error(verbose, error, &message);
            fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
            fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
            mb_memory_clear(verbose, &memclear_error);
            exit(error);
          }

          /* loop over reading */
          while (error <= MB_ERROR_NO_ERROR) {
            status = mb_read(verbose, mbio_ptr, &kind, &rpings, time_i, &time_d, &navlon, &navlat, &speed, &heading,
                             &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath,
                             amp, bathlon, bathlat, ss, sslon, sslat, comment, &error);

            /* time gaps are not a problem here */
            if (error == MB_ERROR_TIME_GAP) {
              error = MB_ERROR_NO_ERROR;
              status = MB_SUCCESS;
            }

            if (verbose >= 2) {
              fprintf(outfp, "\ndbg2  Ping read in program <%s>\n", program_name);
              fprintf(outfp, "dbg2       kind:           %d\n", kind);
              fprintf(outfp, "dbg2       beams_bath:     %d\n", beams_bath);
              fprintf(outfp, "dbg2       beams_amp:      %d\n", beams_amp);
              fprintf(outfp, "dbg2       pixels_ss:      %d\n", pixels_ss);
              fprintf(outfp, "dbg2       error:          %d\n", error);
              fprintf(outfp, "dbg2       status:         %d\n", status);
            }

            if ((datatype == MBGRID_DATA_BATHYMETRY || datatype == MBGRID_DATA_TOPOGRAPHY) &&
                error == MB_ERROR_NO_ERROR) {

              /* reproject beam positions if necessary */
              if (use_projection) {
                for (ib = 0; ib < beams_bath; ib++)
                  if (mb_beam_ok(beamflag[ib]))
                    mb_proj_forward(verbose, pjptr, bathlon[ib], bathlat[ib], &bathlon[ib], &bathlat[ib],
                                    &error);
              }

              /* deal with data */
              for (ib = 0; ib < beams_bath; ib++)
                if (mb_beam_ok(beamflag[ib])) {
                  /* get position in grid */
                  ix = (bathlon[ib] - wbnd[0] + 0.5 * dx) / dx;
                  iy = (bathlat[ib] - wbnd[2] + 0.5 * dy) / dy;
                  /* if (ib==beams_bath/2)fprintf(outfp, "ib:%d ix:%d iy:%d   bath: lon:%.10f lat:%.10f bath:%f
                  dx:%.10f dy:%.10f  origin: lon:%.10f lat:%.10f\n", ib, ix, iy, bathlon[ib], bathlat[ib],
                  bath[ib], dx, dy, wbnd[0], wbnd[1]); */

                  /* check if within allowed time */
                  if (check_time) {
                    /* if in region of interest
                       check if time is ok */
                    if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
                      kgrid = ix * gydim + iy;
                      if (firsttime[kgrid] <= 0.0) {
                        firsttime[kgrid] = time_d;
                        time_ok = true;
                      }
                      else if (fabs(time_d - firsttime[kgrid]) > timediff) {
                        if (first_in_stays)
                          time_ok = false;
                        else {
                          time_ok = true;
                          firsttime[kgrid] = time_d;
                          ndata = ndata - cnt[kgrid];
                          ndatafile = ndatafile - cnt[kgrid];
                          norm[kgrid] = 0.0;
                          grid[kgrid] = 0.0;
                          sigma[kgrid] = 0.0;
                          num[kgrid] = 0;
                          cnt[kgrid] = 0;
                        }
                      }
                      else
                        time_ok = true;
                    }
                    else
                      time_ok = true;
                  }
                  else
                    time_ok = true;

                  /* process if in region of interest */
                  if (grid_mode == MBGRID_WEIGHTED_MEAN
                      && ix >= 0 && ix < gxdim
                      && iy >= 0 && iy < gydim && time_ok) {
                    ix1 = std::max(ix - xtradim, 0);
                    ix2 = std::min(ix + xtradim, gxdim - 1);
                    iy1 = std::max(iy - xtradim, 0);
                    iy2 = std::min(iy + xtradim, gydim - 1);
                    for (int ii = ix1; ii <= ix2; ii++)
                      for (int jj = iy1; jj <= iy2; jj++) {
                        kgrid = ii * gydim + jj;
                        xx = wbnd[0] + ii * dx - bathlon[ib];
                        yy = wbnd[2] + jj * dy - bathlat[ib];
                        weight = file_weight * exp(-(xx * xx + yy * yy) * factor);
                        norm[kgrid] = norm[kgrid] + weight;
                        grid[kgrid] = grid[kgrid] + weight * topofactor * bath[ib];
                        sigma[kgrid] =
                            sigma[kgrid] + weight * topofactor * topofactor * bath[ib] * bath[ib];
                        num[kgrid]++;
                        if (ii == ix && jj == iy)
                          cnt[kgrid]++;
                      }
                    ndata++;
                    ndatafile++;
                    if (first) {
                      first = false;
                      dmin = topofactor * bath[ib];
                      dmax = topofactor * bath[ib];
                    } else {
                      dmin = std::min(topofactor * bath[ib], dmin);
                      dmax = std::max(topofactor * bath[ib], dmax);
                    }
                  }
                  else if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim && time_ok) {
                    kgrid = ix * gydim + iy;
                    if ((num[kgrid] > 0 && grid_mode == MBGRID_MINIMUM_FILTER &&
                         grid[kgrid] > topofactor * bath[ib]) ||
                        (num[kgrid] > 0 && grid_mode == MBGRID_MAXIMUM_FILTER &&
                         grid[kgrid] < topofactor * bath[ib]) ||
                        num[kgrid] <= 0) {
                      norm[kgrid] = 1.0;
                      grid[kgrid] = topofactor * bath[ib];
                      sigma[kgrid] = topofactor * topofactor * bath[ib] * bath[ib];
                      num[kgrid] = 1;
                      cnt[kgrid] = 1;
                    }
                    ndata++;
                    ndatafile++;
                    if (first) {
                      first = false;
                      dmin = topofactor * bath[ib];
                      dmax = topofactor * bath[ib];
                    } else {
                      dmin = std::min(topofactor * bath[ib], dmin);
                      dmax = std::max(topofactor * bath[ib], dmax);
                    }
                  }
                }
            }
            else if (datatype == MBGRID_DATA_AMPLITUDE && error == MB_ERROR_NO_ERROR) {

              /* reproject beam positions if necessary */
              if (use_projection) {
                for (ib = 0; ib < beams_amp; ib++)
                  if (mb_beam_ok(beamflag[ib]))
                    mb_proj_forward(verbose, pjptr, bathlon[ib], bathlat[ib], &bathlon[ib], &bathlat[ib],
                                    &error);
              }

              /* deal with data */
              for (ib = 0; ib < beams_amp; ib++)
                if (mb_beam_ok(beamflag[ib])) {
                  /* get position in grid */
                  ix = (bathlon[ib] - wbnd[0] + 0.5 * dx) / dx;
                  iy = (bathlat[ib] - wbnd[2] + 0.5 * dy) / dy;

                  /* check if within allowed time */
                  if (check_time) {
                    /* if in region of interest
                       check if time is ok */
                    if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
                      kgrid = ix * gydim + iy;
                      if (firsttime[kgrid] <= 0.0) {
                        firsttime[kgrid] = time_d;
                        time_ok = true;
                      }
                      else if (fabs(time_d - firsttime[kgrid]) > timediff) {
                        if (first_in_stays)
                          time_ok = false;
                        else {
                          time_ok = true;
                          firsttime[kgrid] = time_d;
                          ndata = ndata - cnt[kgrid];
                          ndatafile = ndatafile - cnt[kgrid];
                          norm[kgrid] = 0.0;
                          grid[kgrid] = 0.0;
                          sigma[kgrid] = 0.0;
                          num[kgrid] = 0;
                          cnt[kgrid] = 0;
                        }
                      }
                      else
                        time_ok = true;
                    }
                    else
                      time_ok = true;
                  }
                  else
                    time_ok = true;

                  /* process if in region of interest */
                  if (grid_mode == MBGRID_WEIGHTED_MEAN
                      && ix >= 0 && ix < gxdim
                      && iy >= 0 && iy < gydim && time_ok) {
                    ix1 = std::max(ix - xtradim, 0);
                    ix2 = std::min(ix + xtradim, gxdim - 1);
                    iy1 = std::max(iy - xtradim, 0);
                    iy2 = std::min(iy + xtradim, gydim - 1);
                    for (int ii = ix1; ii <= ix2; ii++)
                      for (int jj = iy1; jj <= iy2; jj++) {
                        kgrid = ii * gydim + jj;
                        xx = wbnd[0] + ii * dx - bathlon[ib];
                        yy = wbnd[2] + jj * dy - bathlat[ib];
                        weight = file_weight * exp(-(xx * xx + yy * yy) * factor);
                        norm[kgrid] = norm[kgrid] + weight;
                        grid[kgrid] = grid[kgrid] + weight * amp[ib];
                        sigma[kgrid] = sigma[kgrid] + weight * amp[ib] * amp[ib];
                        num[kgrid]++;
                        if (ii == ix && jj == iy)
                          cnt[kgrid]++;
                      }
                    ndata++;
                    ndatafile++;
                    if (first) {
                      first = false;
                      dmin = topofactor * bath[ib];
                      dmax = topofactor * bath[ib];
                    } else {
                      dmin = std::min(topofactor * bath[ib], dmin);
                      dmax = std::max(topofactor * bath[ib], dmax);
                    }
                  }
                  else if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim && time_ok) {
                    kgrid = ix * gydim + iy;
                    if ((num[kgrid] > 0 && grid_mode == MBGRID_MINIMUM_FILTER && grid[kgrid] > amp[ib]) ||
                        (num[kgrid] > 0 && grid_mode == MBGRID_MAXIMUM_FILTER && grid[kgrid] < amp[ib]) ||
                        num[kgrid] <= 0) {
                      norm[kgrid] = 1.0;
                      grid[kgrid] = amp[ib];
                      sigma[kgrid] = amp[ib] * amp[ib];
                      num[kgrid] = 1;
                      cnt[kgrid] = 1;
                    }
                    ndata++;
                    ndatafile++;
                    if (first) {
                      first = false;
                      dmin = amp[ib];
                      dmax = amp[ib];
                    } else {
                      dmin = std::min(amp[ib], dmin);
                      dmax = std::max(amp[ib], dmax);
                    }
                  }
                }
            }
            else if (datatype == MBGRID_DATA_SIDESCAN && error == MB_ERROR_NO_ERROR) {

              /* reproject pixel positions if necessary */
              if (use_projection) {
                for (ib = 0; ib < pixels_ss; ib++)
                  if (ss[ib] > MB_SIDESCAN_NULL)
                    mb_proj_forward(verbose, pjptr, sslon[ib], sslat[ib], &sslon[ib], &sslat[ib], &error);
              }

              /* deal with data */
              for (ib = 0; ib < pixels_ss; ib++)
                if (ss[ib] > MB_SIDESCAN_NULL) {
                  /* get position in grid */
                  ix = (sslon[ib] - wbnd[0] + 0.5 * dx) / dx;
                  iy = (sslat[ib] - wbnd[2] + 0.5 * dy) / dy;

                  /* check if within allowed time */
                  if (check_time) {
                    /* if in region of interest
                       check if time is ok */
                    if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
                      kgrid = ix * gydim + iy;
                      if (firsttime[kgrid] <= 0.0) {
                        firsttime[kgrid] = time_d;
                        time_ok = true;
                      }
                      else if (fabs(time_d - firsttime[kgrid]) > timediff) {
                        if (first_in_stays)
                          time_ok = false;
                        else {
                          time_ok = true;
                          firsttime[kgrid] = time_d;
                          ndata = ndata - cnt[kgrid];
                          ndatafile = ndatafile - cnt[kgrid];
                          norm[kgrid] = 0.0;
                          grid[kgrid] = 0.0;
                          sigma[kgrid] = 0.0;
                          num[kgrid] = 0;
                          cnt[kgrid] = 0;
                        }
                      }
                      else
                        time_ok = true;
                    }
                    else
                      time_ok = true;
                  }
                  else
                    time_ok = true;

                  /* process if in region of interest */
                  if (grid_mode == MBGRID_WEIGHTED_MEAN
                      && ix >= 0 && ix < gxdim
                      && iy >= 0 && iy < gydim && time_ok) {
                    ix1 = std::max(ix - xtradim, 0);
                    ix2 = std::min(ix + xtradim, gxdim - 1);
                    iy1 = std::max(iy - xtradim, 0);
                    iy2 = std::min(iy + xtradim, gydim - 1);
                    for (int ii = ix1; ii <= ix2; ii++)
                      for (int jj = iy1; jj <= iy2; jj++) {
                        kgrid = ii * gydim + jj;
                        xx = wbnd[0] + ii * dx - sslon[ib];
                        yy = wbnd[2] + jj * dy - sslat[ib];
                        weight = file_weight * exp(-(xx * xx + yy * yy) * factor);
                        norm[kgrid] = norm[kgrid] + weight;
                        grid[kgrid] = grid[kgrid] + weight * ss[ib];
                        sigma[kgrid] = sigma[kgrid] + weight * ss[ib] * ss[ib];
                        num[kgrid]++;
                        if (ii == ix && jj == iy)
                          cnt[kgrid]++;
                      }
                    ndata++;
                    ndatafile++;
                  }
                  else if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim && time_ok) {
                    kgrid = ix * gydim + iy;
                    if ((num[kgrid] > 0 && grid_mode == MBGRID_MINIMUM_FILTER && grid[kgrid] > ss[ib]) ||
                        (num[kgrid] > 0 && grid_mode == MBGRID_MAXIMUM_FILTER && grid[kgrid] < ss[ib]) ||
                        num[kgrid] <= 0) {
                      norm[kgrid] = 1.0;
                      grid[kgrid] = ss[ib];
                      sigma[kgrid] = ss[ib] * ss[ib];
                      num[kgrid] = 1;
                      cnt[kgrid] = 1;
                    }
                    ndata++;
                    ndatafile++;
                  }
                }
            }
          }
          mb_close(verbose, &mbio_ptr, &error);
          status = MB_SUCCESS;
          error = MB_ERROR_NO_ERROR;
        }
        if (verbose >= 2)
          fprintf(outfp, "\n");
        if (verbose > 0)
          fprintf(outfp, "%d data points processed in %s (minmax: %f %f)\n", ndatafile, rfile, dmin, dmax);
        else if (file_in_bounds)
          fprintf(outfp, "%d data points processed in %s\n", ndatafile, rfile);

        /* add to datalist if data actually contributed */
        if (ndatafile > 0 && dfp != nullptr) {
          if (pstatus == MB_PROCESSED_USE && astatus == MB_ALTNAV_USE)
            fprintf(dfp, "A:%s %d %f %s\n", path, format, file_weight, apath);
          else if (pstatus == MB_PROCESSED_USE)
            fprintf(dfp, "P:%s %d %f\n", path, format, file_weight);
          else
            fprintf(dfp, "R:%s %d %f\n", path, format, file_weight);
          fflush(dfp);
        }
      } /* end if (format > 0) */

      /* if format == 0 then input is lon,lat,values triples file */
      else if (format == 0 && path[0] != '#') {
        /* open data file */
        if ((rfp = fopen(path, "r")) == nullptr) {
          error = MB_ERROR_OPEN_FAIL;
          fprintf(outfp, "\nUnable to open lon,lat,value triples data file1: %s\n", path);
          fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
          mb_memory_clear(verbose, &memclear_error);
          exit(error);
        }

        /* loop over reading */
        bool first = true;
        double dmin = 0.0;
        double dmax = 0.0;
        while (fscanf(rfp, "%lf %lf %lf", &tlon, &tlat, &tvalue) != EOF) {
          /* reproject data positions if necessary */
          if (use_projection)
            mb_proj_forward(verbose, pjptr, tlon, tlat, &tlon, &tlat, &error);

          /* get position in grid */
          ix = (tlon - wbnd[0] + 0.5 * dx) / dx;
          iy = (tlat - wbnd[2] + 0.5 * dy) / dy;

          /* check if overwriting */
          if (check_time) {
            /* if in region of interest
               check if overwriting */
            if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
              kgrid = ix * gydim + iy;
              if (firsttime[kgrid] > 0.0)
                time_ok = false;
              else
                time_ok = true;
            }
            else
              time_ok = true;
          }
          else
            time_ok = true;

          /* process the data */
          if (grid_mode == MBGRID_WEIGHTED_MEAN && ix >= -xtradim && ix < gxdim + xtradim && iy >= -xtradim &&
              iy < gydim + xtradim && time_ok) {
            ix1 = std::max(ix - xtradim, 0);
            ix2 = std::min(ix + xtradim, gxdim - 1);
            iy1 = std::max(iy - xtradim, 0);
            iy2 = std::min(iy + xtradim, gydim - 1);
            for (int ii = ix1; ii <= ix2; ii++)
              for (int jj = iy1; jj <= iy2; jj++) {
                kgrid = ii * gydim + jj;
                xx = wbnd[0] + ii * dx - tlon;
                yy = wbnd[2] + jj * dy - tlat;
                weight = file_weight * exp(-(xx * xx + yy * yy) * factor);
                norm[kgrid] = norm[kgrid] + weight;
                grid[kgrid] = grid[kgrid] + weight * topofactor * tvalue;
                sigma[kgrid] = sigma[kgrid] + weight * topofactor * topofactor * tvalue * tvalue;
                num[kgrid]++;
                if (ii == ix && jj == iy)
                  cnt[kgrid]++;
              }
            ndata++;
            ndatafile++;
          }
          else if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim && time_ok) {
            kgrid = ix * gydim + iy;
            if ((num[kgrid] > 0 && grid_mode == MBGRID_MINIMUM_FILTER && grid[kgrid] > topofactor * tvalue) ||
                (num[kgrid] > 0 && grid_mode == MBGRID_MAXIMUM_FILTER && grid[kgrid] < topofactor * tvalue) ||
                num[kgrid] <= 0) {
              norm[kgrid] = 1.0;
              grid[kgrid] = topofactor * tvalue;
              sigma[kgrid] = topofactor * topofactor * tvalue * tvalue;
              num[kgrid] = 1;
              cnt[kgrid] = 1;
            }
            ndata++;
            ndatafile++;
          }
        }
        fclose(rfp);
        status = MB_SUCCESS;
        error = MB_ERROR_NO_ERROR;
        if (verbose >= 2)
          fprintf(outfp, "\n");
        if (verbose > 0)
          fprintf(outfp, "%d data points processed in %s (minmax: %f %f)\n", ndatafile, file, dmin, dmax);
        else if (ndatafile > 0)
          fprintf(outfp, "%d data points processed in %s\n", ndatafile, file);

        /* add to datalist if data actually contributed */
        if (ndatafile > 0 && dfp != nullptr) {
          if (pstatus == MB_PROCESSED_USE && astatus == MB_ALTNAV_USE)
            fprintf(dfp, "A:%s %d %f %s\n", path, format, file_weight, apath);
          else if (pstatus == MB_PROCESSED_USE)
            fprintf(dfp, "P:%s %d %f\n", path, format, file_weight);
          else
            fprintf(dfp, "R:%s %d %f\n", path, format, file_weight);
          fflush(dfp);
        }
      } /* end if (format == 0) */
    }
    if (datalist != nullptr)
      mb_datalist_close(verbose, &datalist, &error);
    if (verbose > 0)
      fprintf(outfp, "\n%d total data points processed\n", ndata);

    /* close datalist if necessary */
    if (dfp != nullptr) {
      fclose(dfp);
      dfp = nullptr;
    }

    /* now loop over all points in the output grid */
    if (verbose >= 1)
      fprintf(outfp, "\nMaking raw grid...\n");
    nbinset = 0;
    nbinzero = 0;
    nbinspline = 0;
    nbinbackground = 0;
    for (int i = 0; i < gxdim; i++)
      for (int j = 0; j < gydim; j++) {
        kgrid = i * gydim + j;
        if (cnt[kgrid] > 0) {
          grid[kgrid] = grid[kgrid] / norm[kgrid];
          factor = sigma[kgrid] / norm[kgrid] - grid[kgrid] * grid[kgrid];
          sigma[kgrid] = sqrt(fabs(factor));
          nbinset++;
        }
        else {
          grid[kgrid] = clipvalue;
          sigma[kgrid] = 0.0;
        }
      }

    /***** end of weighted mean gridding *****/
  }

/* -------------------------------------------------------------------------- */
  /***** do minimum weighted mean or maximum weighted mean gridding *****/
  /* two pass algorithm - the first pass finds the minimum or maximum value in each cell,
     the second pass accumulates values within the specified threshold of the
     minimum or maximum and then calculates the weighted mean from those */
  else if (grid_mode == MBGRID_MINIMUM_WEIGHTED_MEAN
            || grid_mode == MBGRID_MAXIMUM_WEIGHTED_MEAN) {

    /* allocate memory for additional arrays */
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(double), (void **)&norm, &error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(double), (void **)&minormax, &error);

    /* if error initializing memory then quit */
    if (error != MB_ERROR_NO_ERROR) {
      char *message = nullptr;
      mb_error(verbose, error, &message);
      fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      mb_memory_clear(verbose, &memclear_error);
      exit(error);
    }

    /* initialize arrays */
    for (int i = 0; i < gxdim; i++)
      for (int j = 0; j < gydim; j++) {
        kgrid = i * gydim + j;
        grid[kgrid] = 0.0;
        norm[kgrid] = 0.0;
        sigma[kgrid] = 0.0;
        minormax[kgrid] = 0.0;
        firsttime[kgrid] = 0.0;
        num[kgrid] = 0;
        cnt[kgrid] = 0;
      }

    /* read in data */
    ndata = 0;
    const int look_processed = MB_DATALIST_LOOK_UNSET;
    if (mb_datalist_open(verbose, &datalist, filelist, look_processed, &error) != MB_SUCCESS) {
      error = MB_ERROR_OPEN_FAIL;
      fprintf(outfp, "\nUnable to open data list file: %s\n", filelist);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      mb_memory_clear(verbose, &memclear_error);
      exit(error);
    }
    while (mb_datalist_read3(verbose, datalist, &pstatus, path, ppath, &astatus, apath, dpath, &format, &file_weight, &error) ==
           MB_SUCCESS) {
      ndatafile = 0;

      /* if format > 0 then input is swath sonar file */
      if (format > 0 && path[0] != '#') {
        /* apply pstatus */
        if (pstatus == MB_PROCESSED_USE)
          strcpy(file, ppath);
        else
          strcpy(file, path);

        /* check for mbinfo file - get file bounds if possible */
        rformat = format;
        strcpy(rfile, file);
        status = mb_check_info(verbose, rfile, lonflip, bounds, &file_in_bounds, &error);
        if (status == MB_FAILURE) {
          file_in_bounds = true;
          status = MB_SUCCESS;
          error = MB_ERROR_NO_ERROR;
        }

        /* initialize the swath sonar file */
        bool first = true;
        double dmin = 0.0;
        double dmax = 0.0;
        if (file_in_bounds) {
          /* check for "fast bathymetry" or "fbt" file */
          if (datatype == MBGRID_DATA_TOPOGRAPHY || datatype == MBGRID_DATA_BATHYMETRY) {
            mb_get_fbt(verbose, rfile, &rformat, &error);
          }

          /* call mb_read_init_altnav() */
          if (mb_read_init_altnav(verbose, rfile, rformat, pings, lonflip, bounds, btime_i, etime_i, speedmin,
                                     timegap, astatus, apath, &mbio_ptr, &btime_d, &etime_d, 
                                     &beams_bath, &beams_amp, &pixels_ss,
                                     &error) != MB_SUCCESS) {
            char *message = nullptr;
            mb_error(verbose, error, &message);
            fprintf(outfp, "\nMBIO Error returned from function <mb_read_init_altnav>:\n%s\n", message);
            fprintf(outfp, "\nMultibeam File <%s> not initialized for reading\n", rfile);
            fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
            mb_memory_clear(verbose, &memclear_error);
            exit(error);
          }

          /* allocate memory for reading data arrays */
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlon,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlat,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslon, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslat, &error);

          /* if error initializing memory then quit */
          if (error != MB_ERROR_NO_ERROR) {
            char *message = nullptr;
            mb_error(verbose, error, &message);
            fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
            fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
            mb_memory_clear(verbose, &memclear_error);
            exit(error);
          }

          /* loop over reading */
          while (error <= MB_ERROR_NO_ERROR) {
            status = mb_read(verbose, mbio_ptr, &kind, &rpings, time_i, &time_d, &navlon, &navlat, &speed, &heading,
                             &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath,
                             amp, bathlon, bathlat, ss, sslon, sslat, comment, &error);

            /* time gaps are not a problem here */
            if (error == MB_ERROR_TIME_GAP) {
              error = MB_ERROR_NO_ERROR;
              status = MB_SUCCESS;
            }

            if (verbose >= 2) {
              fprintf(outfp, "\ndbg2  Ping read in program <%s>\n", program_name);
              fprintf(outfp, "dbg2       kind:           %d\n", kind);
              fprintf(outfp, "dbg2       beams_bath:     %d\n", beams_bath);
              fprintf(outfp, "dbg2       beams_amp:      %d\n", beams_amp);
              fprintf(outfp, "dbg2       pixels_ss:      %d\n", pixels_ss);
              fprintf(outfp, "dbg2       error:          %d\n", error);
              fprintf(outfp, "dbg2       status:         %d\n", status);
            }

            if ((datatype == MBGRID_DATA_BATHYMETRY || datatype == MBGRID_DATA_TOPOGRAPHY) &&
                error == MB_ERROR_NO_ERROR) {

              /* reproject beam positions if necessary */
              if (use_projection) {
                for (ib = 0; ib < beams_bath; ib++)
                  if (mb_beam_ok(beamflag[ib]))
                    mb_proj_forward(verbose, pjptr, bathlon[ib], bathlat[ib], &bathlon[ib], &bathlat[ib],
                                    &error);
              }

              /* deal with data */
              for (ib = 0; ib < beams_bath; ib++)
                if (mb_beam_ok(beamflag[ib])) {
                  /* get position in grid */
                  ix = (bathlon[ib] - wbnd[0] + 0.5 * dx) / dx;
                  iy = (bathlat[ib] - wbnd[2] + 0.5 * dy) / dy;
                  /* if (ib==beams_bath/2)fprintf(outfp, "ib:%d ix:%d iy:%d   bath: lon:%.10f lat:%.10f bath:%f
                  dx:%.10f dy:%.10f  origin: lon:%.10f lat:%.10f\n", ib, ix, iy, bathlon[ib], bathlat[ib],
                  bath[ib], dx, dy, wbnd[0], wbnd[1]); */

                  /* check if within allowed time */
                  if (check_time) {
                    /* if in region of interest
                       check if time is ok */
                    if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
                      kgrid = ix * gydim + iy;
                      if (firsttime[kgrid] <= 0.0) {
                        firsttime[kgrid] = time_d;
                        time_ok = true;
                      }
                      else if (fabs(time_d - firsttime[kgrid]) > timediff) {
                        if (first_in_stays)
                          time_ok = false;
                        else {
                          time_ok = true;
                          firsttime[kgrid] = time_d;
                          ndata = ndata - cnt[kgrid];
                          ndatafile = ndatafile - cnt[kgrid];
                          norm[kgrid] = 0.0;
                          grid[kgrid] = 0.0;
                          sigma[kgrid] = 0.0;
                          num[kgrid] = 0;
                          cnt[kgrid] = 0;
                        }
                      }
                      else
                        time_ok = true;
                    }
                    else
                      time_ok = true;
                  }
                  else
                    time_ok = true;

                  /* process if in region of interest */
                  if (ix >= 0 && ix < gxdim
                      && iy >= 0 && iy < gydim && time_ok) {
                    kgrid = ix * gydim + iy;
                    if (cnt[kgrid] <= 0
                          || (grid_mode == MBGRID_MINIMUM_WEIGHTED_MEAN &&
                                minormax[kgrid] > topofactor * bath[ib])
                          || (grid_mode == MBGRID_MAXIMUM_WEIGHTED_MEAN &&
                                minormax[kgrid] < topofactor * bath[ib])) {
                      minormax[kgrid] = topofactor * bath[ib];
                      cnt[kgrid]++;
                    }
                    ndata++;
                    ndatafile++;
                    if (first) {
                      first = false;
                      dmin = topofactor * bath[ib];
                      dmax = topofactor * bath[ib];
                    } else {
                      dmin = std::min(topofactor * bath[ib], dmin);
                      dmax = std::max(topofactor * bath[ib], dmax);
                    }
                  }
                }
            }
            else if (datatype == MBGRID_DATA_AMPLITUDE && error == MB_ERROR_NO_ERROR) {

              /* reproject beam positions if necessary */
              if (use_projection) {
                for (ib = 0; ib < beams_amp; ib++)
                  if (mb_beam_ok(beamflag[ib]))
                    mb_proj_forward(verbose, pjptr, bathlon[ib], bathlat[ib], &bathlon[ib], &bathlat[ib],
                                    &error);
              }

              /* deal with data */
              for (ib = 0; ib < beams_amp; ib++)
                if (mb_beam_ok(beamflag[ib])) {
                  /* get position in grid */
                  ix = (bathlon[ib] - wbnd[0] + 0.5 * dx) / dx;
                  iy = (bathlat[ib] - wbnd[2] + 0.5 * dy) / dy;

                  /* check if within allowed time */
                  if (check_time) {
                    /* if in region of interest
                       check if time is ok */
                    if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
                      kgrid = ix * gydim + iy;
                      if (firsttime[kgrid] <= 0.0) {
                        firsttime[kgrid] = time_d;
                        time_ok = true;
                      }
                      else if (fabs(time_d - firsttime[kgrid]) > timediff) {
                        if (first_in_stays)
                          time_ok = false;
                        else {
                          time_ok = true;
                          firsttime[kgrid] = time_d;
                          ndata = ndata - cnt[kgrid];
                          ndatafile = ndatafile - cnt[kgrid];
                          norm[kgrid] = 0.0;
                          grid[kgrid] = 0.0;
                          sigma[kgrid] = 0.0;
                          num[kgrid] = 0;
                          cnt[kgrid] = 0;
                        }
                      }
                      else
                        time_ok = true;
                    }
                    else
                      time_ok = true;
                  }
                  else
                    time_ok = true;

                  /* process if in region of interest */
                  if (ix >= 0 && ix < gxdim
                      && iy >= 0 && iy < gydim && time_ok) {
                    kgrid = ix * gydim + iy;
                    if (cnt[kgrid] <= 0
                          || (grid_mode == MBGRID_MINIMUM_WEIGHTED_MEAN &&
                                minormax[kgrid] > amp[ib])
                          || (grid_mode == MBGRID_MAXIMUM_WEIGHTED_MEAN &&
                                minormax[kgrid] < amp[ib])) {
                      minormax[kgrid] = amp[ib];
                      cnt[kgrid]++;
                    }
                    ndata++;
                    ndatafile++;
                    if (first) {
                      first = false;
                      dmin = amp[ib];
                      dmax = amp[ib];
                    } else {
                      dmin = std::min(amp[ib], dmin);
                      dmax = std::max(amp[ib], dmax);
                    }
                  }
                }
            }

            else if (datatype == MBGRID_DATA_SIDESCAN && error == MB_ERROR_NO_ERROR) {

              /* reproject pixel positions if necessary */
              if (use_projection) {
                for (ib = 0; ib < pixels_ss; ib++)
                  if (ss[ib] > MB_SIDESCAN_NULL)
                    mb_proj_forward(verbose, pjptr, sslon[ib], sslat[ib], &sslon[ib], &sslat[ib], &error);
              }

              /* deal with data */
              for (ib = 0; ib < pixels_ss; ib++)
                if (ss[ib] > MB_SIDESCAN_NULL) {
                  /* get position in grid */
                  ix = (sslon[ib] - wbnd[0] + 0.5 * dx) / dx;
                  iy = (sslat[ib] - wbnd[2] + 0.5 * dy) / dy;

                  /* check if within allowed time */
                  if (check_time) {
                    /* if in region of interest
                       check if time is ok */
                    if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
                      kgrid = ix * gydim + iy;
                      if (firsttime[kgrid] <= 0.0) {
                        firsttime[kgrid] = time_d;
                        time_ok = true;
                      }
                      else if (fabs(time_d - firsttime[kgrid]) > timediff) {
                        if (first_in_stays)
                          time_ok = false;
                        else {
                          time_ok = true;
                          firsttime[kgrid] = time_d;
                          ndata = ndata - cnt[kgrid];
                          ndatafile = ndatafile - cnt[kgrid];
                          norm[kgrid] = 0.0;
                          grid[kgrid] = 0.0;
                          sigma[kgrid] = 0.0;
                          num[kgrid] = 0;
                          cnt[kgrid] = 0;
                        }
                      }
                      else
                        time_ok = true;
                    }
                    else
                      time_ok = true;
                  }
                  else
                    time_ok = true;

                  /* process if in region of interest */
                  if (ix >= 0 && ix < gxdim
                      && iy >= 0 && iy < gydim && time_ok) {
                    kgrid = ix * gydim + iy;
                    if (cnt[kgrid] <= 0
                          || (grid_mode == MBGRID_MINIMUM_WEIGHTED_MEAN &&
                                minormax[kgrid] > ss[ib])
                          || (grid_mode == MBGRID_MAXIMUM_WEIGHTED_MEAN &&
                                minormax[kgrid] < ss[ib])) {
                      minormax[kgrid] = ss[ib];
                      cnt[kgrid]++;
                    }
                    ndata++;
                    ndatafile++;
                    if (first) {
                      first = false;
                      dmin = topofactor * bath[ib];
                      dmax = topofactor * bath[ib];
                    } else {
                      dmin = std::min(topofactor * bath[ib], dmin);
                      dmax = std::max(topofactor * bath[ib], dmax);
                    }
                  }
                }
            }
          }
          mb_close(verbose, &mbio_ptr, &error);
          status = MB_SUCCESS;
          error = MB_ERROR_NO_ERROR;
        }
        if (verbose >= 2)
          fprintf(outfp, "\n");
        if (verbose > 0)
          fprintf(outfp, "%d data points processed in %s (minmax: %f %f)\n", ndatafile, rfile, dmin, dmax);
        else if (file_in_bounds)
          fprintf(outfp, "%d data points processed in %s\n", ndatafile, rfile);

        /* add to datalist if data actually contributed */
        if (ndatafile > 0 && dfp != nullptr) {
          if (pstatus == MB_PROCESSED_USE && astatus == MB_ALTNAV_USE)
            fprintf(dfp, "A:%s %d %f %s\n", path, format, file_weight, apath);
          else if (pstatus == MB_PROCESSED_USE)
            fprintf(dfp, "P:%s %d %f\n", path, format, file_weight);
          else
            fprintf(dfp, "R:%s %d %f\n", path, format, file_weight);
          fflush(dfp);
        }
      } /* end if (format > 0) */

    }
    if (datalist != nullptr)
      mb_datalist_close(verbose, &datalist, &error);
    if (verbose > 0)
      fprintf(outfp, "\n%d total data points processed\n", ndata);

    /* close datalist if necessary */
    if (dfp != nullptr) {
      fclose(dfp);
      dfp = nullptr;
    }

    /* now read the data again, using only the data within the threshold of
       the minimum or maximum values */

    /* reinitialize cnt array */
    for (int i = 0; i < gxdim; i++)
      for (int j = 0; j < gydim; j++) {
        kgrid = i * gydim + j;
        cnt[kgrid] = 0;
      }

    /* read in data */
    fprintf(outfp, "\nDoing second pass to generate final grid...\n");
    ndata = 0;
    if (mb_datalist_open(verbose, &datalist, dfile, look_processed, &error) != MB_SUCCESS) {
      error = MB_ERROR_OPEN_FAIL;
      fprintf(outfp, "\nUnable to open data list file: %s\n", filelist);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      mb_memory_clear(verbose, &memclear_error);
      exit(error);
    }
    while (mb_datalist_read3(verbose, datalist, &pstatus, path, ppath, &astatus, apath, dpath, &format, &file_weight, &error) ==
           MB_SUCCESS) {
      ndatafile = 0;

      /* if format > 0 then input is swath sonar file */
      if (format > 0 && path[0] != '#') {
        /* apply pstatus */
        if (pstatus == MB_PROCESSED_USE)
          strcpy(file, ppath);
        else
          strcpy(file, path);

        /* check for mbinfo file - get file bounds if possible */
        rformat = format;
        strcpy(rfile, file);
        status = mb_check_info(verbose, rfile, lonflip, bounds, &file_in_bounds, &error);
        if (status == MB_FAILURE) {
          file_in_bounds = true;
          status = MB_SUCCESS;
          error = MB_ERROR_NO_ERROR;
        }

        /* initialize the swath sonar file */
        bool first = true;
        double dmin = 0.0;
        double dmax = 0.0;
        if (file_in_bounds) {
          /* check for "fast bathymetry" or "fbt" file */
          if (datatype == MBGRID_DATA_TOPOGRAPHY || datatype == MBGRID_DATA_BATHYMETRY) {
            mb_get_fbt(verbose, rfile, &rformat, &error);
          }

          /* call mb_read_init_altnav() */
          if (mb_read_init_altnav(verbose, rfile, rformat, pings, lonflip, bounds, btime_i, etime_i, speedmin,
                                     timegap, astatus, apath, &mbio_ptr, &btime_d, &etime_d, 
                                     &beams_bath, &beams_amp, &pixels_ss,
                                     &error) != MB_SUCCESS) {
            char *message = nullptr;
            mb_error(verbose, error, &message);
            fprintf(outfp, "\nMBIO Error returned from function <mb_read_init_altnav>:\n%s\n", message);
            fprintf(outfp, "\nMultibeam File <%s> not initialized for reading\n", rfile);
            fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
            mb_memory_clear(verbose, &memclear_error);
            exit(error);
          }

          /* allocate memory for reading data arrays */
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlon,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlat,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslon, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslat, &error);

          /* if error initializing memory then quit */
          if (error != MB_ERROR_NO_ERROR) {
            char *message = nullptr;
            mb_error(verbose, error, &message);
            fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
            fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
            mb_memory_clear(verbose, &memclear_error);
            exit(error);
          }

          /* loop over reading */
          while (error <= MB_ERROR_NO_ERROR) {
            status = mb_read(verbose, mbio_ptr, &kind, &rpings, time_i, &time_d, &navlon, &navlat, &speed, &heading,
                             &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath,
                             amp, bathlon, bathlat, ss, sslon, sslat, comment, &error);

            /* time gaps are not a problem here */
            if (error == MB_ERROR_TIME_GAP) {
              error = MB_ERROR_NO_ERROR;
              status = MB_SUCCESS;
            }

            if (verbose >= 2) {
              fprintf(outfp, "\ndbg2  Ping read in program <%s>\n", program_name);
              fprintf(outfp, "dbg2       kind:           %d\n", kind);
              fprintf(outfp, "dbg2       beams_bath:     %d\n", beams_bath);
              fprintf(outfp, "dbg2       beams_amp:      %d\n", beams_amp);
              fprintf(outfp, "dbg2       pixels_ss:      %d\n", pixels_ss);
              fprintf(outfp, "dbg2       error:          %d\n", error);
              fprintf(outfp, "dbg2       status:         %d\n", status);
            }

            if ((datatype == MBGRID_DATA_BATHYMETRY || datatype == MBGRID_DATA_TOPOGRAPHY) &&
                error == MB_ERROR_NO_ERROR) {

              /* reproject beam positions if necessary */
              if (use_projection) {
                for (ib = 0; ib < beams_bath; ib++)
                  if (mb_beam_ok(beamflag[ib]))
                    mb_proj_forward(verbose, pjptr, bathlon[ib], bathlat[ib], &bathlon[ib], &bathlat[ib],
                                    &error);
              }

              /* deal with data */
              for (ib = 0; ib < beams_bath; ib++)
                if (mb_beam_ok(beamflag[ib])) {
                  /* get position in grid */
                  ix = (bathlon[ib] - wbnd[0] + 0.5 * dx) / dx;
                  iy = (bathlat[ib] - wbnd[2] + 0.5 * dy) / dy;
                  /* if (ib==beams_bath/2)fprintf(outfp, "ib:%d ix:%d iy:%d   bath: lon:%.10f lat:%.10f bath:%f
                  dx:%.10f dy:%.10f  origin: lon:%.10f lat:%.10f\n", ib, ix, iy, bathlon[ib], bathlat[ib],
                  bath[ib], dx, dy, wbnd[0], wbnd[1]); */

                  /* check if within allowed time */
                  if (check_time) {
                    /* if in region of interest
                       check if time is ok */
                    if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
                      kgrid = ix * gydim + iy;
                      if (firsttime[kgrid] <= 0.0) {
                        firsttime[kgrid] = time_d;
                        time_ok = true;
                      }
                      else if (fabs(time_d - firsttime[kgrid]) > timediff) {
                        if (first_in_stays)
                          time_ok = false;
                        else {
                          time_ok = true;
                          firsttime[kgrid] = time_d;
                          ndata = ndata - cnt[kgrid];
                          ndatafile = ndatafile - cnt[kgrid];
                          norm[kgrid] = 0.0;
                          grid[kgrid] = 0.0;
                          sigma[kgrid] = 0.0;
                          num[kgrid] = 0;
                          cnt[kgrid] = 0;
                        }
                      }
                      else
                        time_ok = true;
                    }
                    else
                      time_ok = true;
                  }
                  else
                    time_ok = true;

                  /* process if in region of interest */
                  if (ix >= 0 && ix < gxdim
                      && iy >= 0 && iy < gydim && time_ok) {
                    kgrid = ix * gydim + iy;
                    if (fabs(minormax[kgrid] - topofactor * bath[ib]) < minormax_weighted_mean_threshold) {
                      ix1 = std::max(ix - xtradim, 0);
                      ix2 = std::min(ix + xtradim, gxdim - 1);
                      iy1 = std::max(iy - xtradim, 0);
                      iy2 = std::min(iy + xtradim, gydim - 1);
                      for (int ii = ix1; ii <= ix2; ii++) {
                        for (int jj = iy1; jj <= iy2; jj++) {
                          kgrid = ii * gydim + jj;
                          xx = wbnd[0] + ii * dx - bathlon[ib];
                          yy = wbnd[2] + jj * dy - bathlat[ib];
                          weight = file_weight * exp(-(xx * xx + yy * yy) * factor);
                          norm[kgrid] = norm[kgrid] + weight;
                          grid[kgrid] = grid[kgrid] + weight * topofactor * bath[ib];
                          sigma[kgrid] =
                              sigma[kgrid] + weight * topofactor * topofactor * bath[ib] * bath[ib];
                          num[kgrid]++;
                          if (ii == ix && jj == iy)
                            cnt[kgrid]++;
                        }
                      }
                      ndata++;
                      ndatafile++;
                      if (first) {
                        first = false;
                        dmin = topofactor * bath[ib];
                        dmax = topofactor * bath[ib];
                      } else {
                        dmin = std::min(topofactor * bath[ib], dmin);
                        dmax = std::max(topofactor * bath[ib], dmax);
                      }
                    }
                  }
                }
            }
            else if (datatype == MBGRID_DATA_AMPLITUDE && error == MB_ERROR_NO_ERROR) {

              /* reproject beam positions if necessary */
              if (use_projection) {
                for (ib = 0; ib < beams_amp; ib++)
                  if (mb_beam_ok(beamflag[ib]))
                    mb_proj_forward(verbose, pjptr, bathlon[ib], bathlat[ib], &bathlon[ib], &bathlat[ib],
                                    &error);
              }

              /* deal with data */
              for (ib = 0; ib < beams_amp; ib++)
                if (mb_beam_ok(beamflag[ib])) {
                  /* get position in grid */
                  ix = (bathlon[ib] - wbnd[0] + 0.5 * dx) / dx;
                  iy = (bathlat[ib] - wbnd[2] + 0.5 * dy) / dy;

                  /* check if within allowed time */
                  if (check_time) {
                    /* if in region of interest
                       check if time is ok */
                    if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
                      kgrid = ix * gydim + iy;
                      if (firsttime[kgrid] <= 0.0) {
                        firsttime[kgrid] = time_d;
                        time_ok = true;
                      }
                      else if (fabs(time_d - firsttime[kgrid]) > timediff) {
                        if (first_in_stays)
                          time_ok = false;
                        else {
                          time_ok = true;
                          firsttime[kgrid] = time_d;
                          ndata = ndata - cnt[kgrid];
                          ndatafile = ndatafile - cnt[kgrid];
                          norm[kgrid] = 0.0;
                          grid[kgrid] = 0.0;
                          sigma[kgrid] = 0.0;
                          num[kgrid] = 0;
                          cnt[kgrid] = 0;
                        }
                      }
                      else
                        time_ok = true;
                    }
                    else
                      time_ok = true;
                  }
                  else
                    time_ok = true;

                  /* process if in region of interest */
                  if (grid_mode == MBGRID_WEIGHTED_MEAN
                      && ix >= 0 && ix < gxdim
                      && iy >= 0 && iy < gydim && time_ok) {
                    kgrid = ix * gydim + iy;
                    if (fabs(minormax[kgrid] - amp[ib]) < minormax_weighted_mean_threshold) {
                      ix1 = std::max(ix - xtradim, 0);
                      ix2 = std::min(ix + xtradim, gxdim - 1);
                      iy1 = std::max(iy - xtradim, 0);
                      iy2 = std::min(iy + xtradim, gydim - 1);
                      for (int ii = ix1; ii <= ix2; ii++)
                        for (int jj = iy1; jj <= iy2; jj++) {
                          kgrid = ii * gydim + jj;
                          xx = wbnd[0] + ii * dx - bathlon[ib];
                          yy = wbnd[2] + jj * dy - bathlat[ib];
                          weight = file_weight * exp(-(xx * xx + yy * yy) * factor);
                          norm[kgrid] = norm[kgrid] + weight;
                          grid[kgrid] = grid[kgrid] + weight * amp[ib];
                          sigma[kgrid] = sigma[kgrid] + weight * amp[ib] * amp[ib];
                          num[kgrid]++;
                          if (ii == ix && jj == iy)
                            cnt[kgrid]++;
                        }
                      ndata++;
                      ndatafile++;
                      if (first) {
                        first = false;
                        dmin = amp[ib];
                        dmax = amp[ib];
                      } else {
                        dmin = std::min(amp[ib], dmin);
                        dmax = std::max(amp[ib], dmax);
                      }
                    }
                  }
                }
            }

            else if (datatype == MBGRID_DATA_SIDESCAN && error == MB_ERROR_NO_ERROR) {

              /* reproject pixel positions if necessary */
              if (use_projection) {
                for (ib = 0; ib < pixels_ss; ib++)
                  if (ss[ib] > MB_SIDESCAN_NULL)
                    mb_proj_forward(verbose, pjptr, sslon[ib], sslat[ib], &sslon[ib], &sslat[ib], &error);
              }

              /* deal with data */
              for (ib = 0; ib < pixels_ss; ib++)
                if (ss[ib] > MB_SIDESCAN_NULL) {
                  /* get position in grid */
                  ix = (sslon[ib] - wbnd[0] + 0.5 * dx) / dx;
                  iy = (sslat[ib] - wbnd[2] + 0.5 * dy) / dy;

                  /* check if within allowed time */
                  if (check_time) {
                    /* if in region of interest
                       check if time is ok */
                    if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
                      kgrid = ix * gydim + iy;
                      if (firsttime[kgrid] <= 0.0) {
                        firsttime[kgrid] = time_d;
                        time_ok = true;
                      }
                      else if (fabs(time_d - firsttime[kgrid]) > timediff) {
                        if (first_in_stays)
                          time_ok = false;
                        else {
                          time_ok = true;
                          firsttime[kgrid] = time_d;
                          ndata = ndata - cnt[kgrid];
                          ndatafile = ndatafile - cnt[kgrid];
                          norm[kgrid] = 0.0;
                          grid[kgrid] = 0.0;
                          sigma[kgrid] = 0.0;
                          num[kgrid] = 0;
                          cnt[kgrid] = 0;
                        }
                      }
                      else
                        time_ok = true;
                    }
                    else
                      time_ok = true;
                  }
                  else
                    time_ok = true;

                  /* process if in region of interest */
                  if (grid_mode == MBGRID_WEIGHTED_MEAN
                      && ix >= 0 && ix < gxdim
                      && iy >= 0 && iy < gydim && time_ok) {
                    kgrid = ix * gydim + iy;
                    if (fabs(minormax[kgrid] - ss[ib]) < minormax_weighted_mean_threshold) {
                      ix1 = std::max(ix - xtradim, 0);
                      ix2 = std::min(ix + xtradim, gxdim - 1);
                      iy1 = std::max(iy - xtradim, 0);
                      iy2 = std::min(iy + xtradim, gydim - 1);
                      for (int ii = ix1; ii <= ix2; ii++)
                        for (int jj = iy1; jj <= iy2; jj++) {
                          kgrid = ii * gydim + jj;
                          xx = wbnd[0] + ii * dx - bathlon[ib];
                          yy = wbnd[2] + jj * dy - bathlat[ib];
                          weight = file_weight * exp(-(xx * xx + yy * yy) * factor);
                          norm[kgrid] = norm[kgrid] + weight;
                          grid[kgrid] = grid[kgrid] + weight * ss[ib];
                          sigma[kgrid] = sigma[kgrid] + weight * ss[ib] * ss[ib];
                          num[kgrid]++;
                          if (ii == ix && jj == iy)
                            cnt[kgrid]++;
                        }
                      ndata++;
                      ndatafile++;
                      if (first) {
                        first = false;
                        dmin = topofactor * bath[ib];
                        dmax = topofactor * bath[ib];
                      } else {
                        dmin = std::min(topofactor * bath[ib], dmin);
                        dmax = std::max(topofactor * bath[ib], dmax);
                      }
                    }
                  }
                }
            }
          }
          mb_close(verbose, &mbio_ptr, &error);
          status = MB_SUCCESS;
          error = MB_ERROR_NO_ERROR;
        }
        if (verbose >= 2)
          fprintf(outfp, "\n");
        if (verbose > 0)
          fprintf(outfp, "%d data points processed in %s (minmax: %f %f)\n", ndatafile, rfile, dmin, dmax);
        else if (file_in_bounds)
          fprintf(outfp, "%d data points processed in %s\n", ndatafile, rfile);

        /* add to datalist if data actually contributed */
        if (ndatafile > 0 && dfp != nullptr) {
          if (pstatus == MB_PROCESSED_USE && astatus == MB_ALTNAV_USE)
            fprintf(dfp, "A:%s %d %f %s\n", path, format, file_weight, apath);
          else if (pstatus == MB_PROCESSED_USE)
            fprintf(dfp, "P:%s %d %f\n", path, format, file_weight);
          else
            fprintf(dfp, "R:%s %d %f\n", path, format, file_weight);
          fflush(dfp);
        }
      } /* end if (format > 0) */

    }
    if (datalist != nullptr)
      mb_datalist_close(verbose, &datalist, &error);
    if (verbose > 0)
      fprintf(outfp, "\n%d total data points processed\n", ndata);

    /* close datalist if necessary */
    if (dfp != nullptr) {
      fclose(dfp);
      dfp = nullptr;
    }

    /* now loop over all points in the output grid */
    if (verbose >= 1)
      fprintf(outfp, "\nMaking raw grid...\n");
    nbinset = 0;
    nbinzero = 0;
    nbinspline = 0;
    nbinbackground = 0;
    for (int i = 0; i < gxdim; i++)
      for (int j = 0; j < gydim; j++) {
        kgrid = i * gydim + j;
        if (cnt[kgrid] > 0) {
          grid[kgrid] = grid[kgrid] / norm[kgrid];
          factor = sigma[kgrid] / norm[kgrid] - grid[kgrid] * grid[kgrid];
          sigma[kgrid] = sqrt(fabs(factor));
          nbinset++;
        }
        else {
          grid[kgrid] = clipvalue;
          sigma[kgrid] = 0.0;
        }
      }

    /***** end of weighted mean gridding *****/
  }

/* -------------------------------------------------------------------------- */

  /* if clip set do smooth interpolation */
  if (clipmode != MBGRID_INTERP_NONE && clip > 0 && nbinset > 0) {
    /* set up data vector */
    if (setborder)
      ndata = 2 * gxdim + 2 * gydim - 2;
    else
      ndata = 8;
    for (int i = 0; i < gxdim; i++)
      for (int j = 0; j < gydim; j++) {
        kgrid = i * gydim + j;
        if (grid[kgrid] < clipvalue)
          ndata++;
      }

#ifdef USESURFACE
    /* allocate and initialize sgrid */
    status = mb_mallocd(verbose, __FILE__, __LINE__, ndata * sizeof(float), (void **)&sxdata, &error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, ndata * sizeof(float), (void **)&sydata, &error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, ndata * sizeof(float), (void **)&szdata, &error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(float), (void **)&sgrid, &error);
    if (error != MB_ERROR_NO_ERROR) {
      char *message = nullptr;
      mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
      fprintf(outfp, "\nMBIO Error allocating interpolation work arrays:\n%s\n", message);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      mb_memory_clear(verbose, &memclear_error);
      exit(error);
    }
    memset((char *)sgrid, 0, gxdim * gydim * sizeof(float));
    memset((char *)sxdata, 0, ndata * sizeof(float));
    memset((char *)sydata, 0, ndata * sizeof(float));
    memset((char *)szdata, 0, ndata * sizeof(float));

    /* get points from grid */
    /* simultaneously find the depth values nearest to the grid corners and edge midpoints */
    ndata = 0;
    for (int i = 0; i < gxdim; i++)
      for (int j = 0; j < gydim; j++) {
        kgrid = i * gydim + j;
        if (grid[kgrid] < clipvalue) {
          sxdata[ndata] = (float)(wbnd[0] + dx * i - bdata_origin_x);
          sydata[ndata] = (float)(wbnd[2] + dy * j - bdata_origin_y);
          szdata[ndata] = (float)grid[kgrid];
          ndata++;
        }
      }

    /* if desired set border */
    if (setborder) {
      for (int i = 0; i < gxdim; i++) {
        j = 0;
        kgrid = i * gydim + j;
        if (grid[kgrid] >= clipvalue) {
          sxdata[ndata] = (float)(wbnd[0] + dx * i - bdata_origin_x);
          sydata[ndata] = (float)(wbnd[2] + dy * j - bdata_origin_y);
          szdata[ndata] = (float)border;
          ndata++;
        }
        j = gydim - 1;
        kgrid = i * gydim + j;
        if (grid[kgrid] >= clipvalue) {
          sxdata[ndata] = (float)(wbnd[0] + dx * i - bdata_origin_x);
          sydata[ndata] = (float)(wbnd[2] + dy * j - bdata_origin_y);
          szdata[ndata] = (float)border;
          ndata++;
        }
      }
      for (int j = 1; j < gydim - 1; j++) {
        i = 0;
        kgrid = i * gydim + j;
        if (grid[kgrid] >= clipvalue) {
          sxdata[ndata] = (float)(wbnd[0] + dx * i - bdata_origin_x);
          sydata[ndata] = (float)(wbnd[2] + dy * j - bdata_origin_y);
          szdata[ndata] = (float)border;
          ndata++;
        }
        i = gxdim - 1;
        kgrid = i * gydim + j;
        if (grid[kgrid] >= clipvalue) {
          sxdata[ndata] = (float)(wbnd[0] + dx * i - bdata_origin_x);
          sydata[ndata] = (float)(wbnd[2] + dy * j - bdata_origin_y);
          szdata[ndata] = (float)border;
          ndata++;
        }
      }
    }

    /* do the interpolation */
    fprintf(outfp, "\nDoing Surface spline interpolation with %d data points...\n", ndata);
    mb_surface(verbose, ndata, sxdata, sydata, szdata, (float)(gbnd[0] - bdata_origin_x), (float)(gbnd[1] - bdata_origin_x),
               (float)(gbnd[2] - bdata_origin_y), (float)(gbnd[3] - bdata_origin_y), dx, dy, tension, sgrid);
#else
    /* allocate and initialize sgrid */
    status = mb_mallocd(verbose, __FILE__, __LINE__, 3 * ndata * sizeof(float), (void **)&sdata, &error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(float), (void **)&sgrid, &error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, ndata * sizeof(float), (void **)&work1, &error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, ndata * sizeof(int), (void **)&work2, &error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, (gxdim + gydim) * sizeof(bool), (void **)&work3, &error);
    if (error != MB_ERROR_NO_ERROR) {
      char *message = nullptr;
      mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
      fprintf(outfp, "\nMBIO Error allocating interpolation work arrays:\n%s\n", message);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      mb_memory_clear(verbose, &memclear_error);
      exit(error);
    }
    memset((char *)sgrid, 0, gxdim * gydim * sizeof(float));
    memset((char *)sdata, 0, 3 * ndata * sizeof(float));
    memset((char *)work1, 0, ndata * sizeof(float));
    memset((char *)work2, 0, ndata * sizeof(int));
    memset((char *)work3, 0, (gxdim + gydim) * sizeof(bool));

    /* get points from grid */
    /* simultaneously find the depth values nearest to the grid corners and edge midpoints */
    ndata = 0;
    for (int i = 0; i < gxdim; i++)
      for (int j = 0; j < gydim; j++) {
        kgrid = i * gydim + j;
        if (grid[kgrid] < clipvalue) {
          sdata[ndata++] = (float)(wbnd[0] + dx * i - bdata_origin_x);
          sdata[ndata++] = (float)(wbnd[2] + dy * j - bdata_origin_y);
          sdata[ndata++] = (float)grid[kgrid];
        }
      }

    /* if desired set border */
    if (setborder) {
      for (int i = 0; i < gxdim; i++) {
        int j = 0;
        kgrid = i * gydim + j;
        if (grid[kgrid] >= clipvalue) {
          sdata[ndata++] = (float)(wbnd[0] + dx * i - bdata_origin_x);
          sdata[ndata++] = (float)(wbnd[2] + dy * j - bdata_origin_y);
          sdata[ndata++] = (float)border;
        }
        j = gydim - 1;
        kgrid = i * gydim + j;
        if (grid[kgrid] >= clipvalue) {
          sdata[ndata++] = (float)(wbnd[0] + dx * i - bdata_origin_x);
          sdata[ndata++] = (float)(wbnd[2] + dy * j - bdata_origin_y);
          sdata[ndata++] = (float)border;
        }
      }
      for (int j = 1; j < gydim - 1; j++) {
        int i = 0;
        kgrid = i * gydim + j;
        if (grid[kgrid] >= clipvalue) {
          sdata[ndata++] = (float)(wbnd[0] + dx * i - bdata_origin_x);
          sdata[ndata++] = (float)(wbnd[2] + dy * j - bdata_origin_y);
          sdata[ndata++] = (float)border;
        }
        i = gxdim - 1;
        kgrid = i * gydim + j;
        if (grid[kgrid] >= clipvalue) {
          sdata[ndata++] = (float)(wbnd[0] + dx * i - bdata_origin_x);
          sdata[ndata++] = (float)(wbnd[2] + dy * j - bdata_origin_y);
          sdata[ndata++] = (float)border;
        }
      }
    }
    ndata = ndata / 3;

    /* do the interpolation */
    float cay = (float)tension;
    float xmin = (float)(wbnd[0] - 0.5 * dx - bdata_origin_x);
    float ymin = (float)(wbnd[2] - 0.5 * dy - bdata_origin_y);
    float ddx = (float)dx;
    float ddy = (float)dy;
    fprintf(outfp, "\nDoing Zgrid spline interpolation with %d data points...\n", ndata);
    /*for (i=0;i<ndata/3;i++)
    {
    if (sdata[3*i+2]>2000.0)
    fprintf(stderr,"%d %f\n",i,sdata[3*i+2]);
    }*/
    if (clipmode == MBGRID_INTERP_ALL)
      clip = std::max(gxdim, gydim);
    mb_zgrid(sgrid, &gxdim, &gydim, &xmin, &ymin, &ddx, &ddy, sdata, &ndata, work1, work2, work3, &cay, &clip);
#endif

    if (clipmode == MBGRID_INTERP_GAP)
      fprintf(outfp, "Applying spline interpolation to fill gaps of %d cells or less...\n", clip);
    else if (clipmode == MBGRID_INTERP_NEAR)
      fprintf(outfp, "Applying spline interpolation to fill %d cells from data...\n", clip);
    else if (clipmode == MBGRID_INTERP_ALL)
      fprintf(outfp, "Applying spline interpolation to fill all undefined cells in the grid...\n");

    /* allocate interpolation mask */
    bool *smask = NULL;
    if (mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(bool), (void **)&smask, &error) != MB_SUCCESS) {
      char *message = nullptr;
      mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
      fprintf(outfp, "\nMBIO Error allocating interpolation work arrays:\n%s\n", message);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      mb_memory_clear(verbose, &memclear_error);
      exit(error);
    }
    memset((char *)smask, 0, (gxdim + gydim) * sizeof(bool));

    /* translate the interpolation into the grid array
        filling only data gaps */
    const float zflag = 5.0e34f;
    if (clipmode == MBGRID_INTERP_GAP) {
      for (int i = 0; i < gxdim; i++)
        for (int j = 0; j < gydim; j++) {
          kgrid = i * gydim + j;
#ifdef USESURFACE
          kint = i + (gydim - j - 1) * gxdim;
#else
          kint = i + j * gxdim;
#endif
          smask[kgrid] = false;
          if (grid[kgrid] >= clipvalue && sgrid[kint] < zflag) {
            /* initialize direction mask of search */
            int dmask[9];
            for (int ii = 0; ii < 9; ii++)
              dmask[ii] = false;

            /* loop over rings around point, starting close */
            for (int ir = 0; ir <= clip && smask[kgrid] == false; ir++) {
              /* set bounds of search */
              const int i1 = std::max(0, i - ir);
              const int i2 = std::min(gxdim - 1, i + ir);
              const int j1 = std::max(0, j - ir);
              const int j2 = std::min(gydim - 1, j + ir);

              int jj = j1;
              for (int ii = i1; ii <= i2 && smask[kgrid] == false; ii++) {
                if (grid[ii * gydim + jj] < clipvalue) {
                  const double r = sqrt((double)((ii - i) * (ii - i) + (jj - j) * (jj - j)));
                  const int iii = rint((ii - i) / r + 1);
                  const int jjj = rint((jj - j) / r + 1);
                  const int kkk = iii * 3 + jjj;
                  dmask[kkk] = true;
                  if ((dmask[0] && dmask[8]) || (dmask[3] && dmask[5]) || (dmask[6] && dmask[2]) ||
                      (dmask[1] && dmask[7]))
                    smask[kgrid] = true;
                }
              }

              jj = j2;
              for (int ii = i1; ii <= i2 && smask[kgrid] == false; ii++) {
                if (grid[ii * gydim + jj] < clipvalue) {
                  const double r = sqrt((double)((ii - i) * (ii - i) + (jj - j) * (jj - j)));
                  const int iii = rint((ii - i) / r + 1);
                  const int jjj = rint((jj - j) / r + 1);
                  const int kkk = iii * 3 + jjj;
                  dmask[kkk] = true;
                  if ((dmask[0] && dmask[8]) || (dmask[3] && dmask[5]) || (dmask[6] && dmask[2]) ||
                      (dmask[1] && dmask[7]))
                    smask[kgrid] = true;
                }
              }

              int ii = i1;
              for (jj = j1; jj <= j2 && smask[kgrid] == false; jj++) {
                if (grid[ii * gydim + jj] < clipvalue) {
                  const double r = sqrt((double)((ii - i) * (ii - i) + (jj - j) * (jj - j)));
                  const int iii = rint((ii - i) / r + 1);
                  const int jjj = rint((jj - j) / r + 1);
                  const int kkk = iii * 3 + jjj;
                  dmask[kkk] = true;
                  if ((dmask[0] && dmask[8]) || (dmask[3] && dmask[5]) || (dmask[6] && dmask[2]) ||
                      (dmask[1] && dmask[7]))
                    smask[kgrid] = true;
                }
              }

              ii = i2;
              for (jj = j1; jj <= j2 && smask[kgrid] == false; jj++) {
                if (grid[ii * gydim + jj] < clipvalue) {
                  const double r = sqrt((double)((ii - i) * (ii - i) + (jj - j) * (jj - j)));
                  const int iii = rint((ii - i) / r + 1);
                  const int jjj = rint((jj - j) / r + 1);
                  const int kkk = iii * 3 + jjj;
                  dmask[kkk] = true;
                  if ((dmask[0] && dmask[8]) || (dmask[3] && dmask[5]) || (dmask[6] && dmask[2]) ||
                      (dmask[1] && dmask[7]))
                    smask[kgrid] = true;
                }
              }
            }
          }
        }
      for (int i = 0; i < gxdim; i++)
        for (int j = 0; j < gydim; j++) {
          kgrid = i * gydim + j;
#ifdef USESURFACE
          kint = i + (gydim - j - 1) * gxdim;
#else
          kint = i + j * gxdim;
#endif
          if (smask[kgrid] == true) {
            grid[kgrid] = sgrid[kint];
            nbinspline++;
          }
        }
    }

    /* translate the interpolation into the grid array
        filling by proximity */
    else if (clipmode == MBGRID_INTERP_NEAR) {
      for (int i = 0; i < gxdim; i++)
        for (int j = 0; j < gydim; j++) {
          kgrid = i * gydim + j;
#ifdef USESURFACE
          kint = i + (gydim - j - 1) * gxdim;
#else
          kint = i + j * gxdim;
#endif

          smask[kgrid] = false;
          if (grid[kgrid] >= clipvalue && sgrid[kint] < zflag) {
            /* loop over rings around point, starting close */
            for (int ir = 0; ir <= clip && smask[kgrid] == false; ir++) {
              /* set bounds of search */
              const int i1 = std::max(0, i - ir);
              const int i2 = std::min(gxdim - 1, i + ir);
              const int j1 = std::max(0, j - ir);
              const int j2 = std::min(gydim - 1, j + ir);

              int jj = j1;
              for (int ii = i1; ii <= i2 && smask[kgrid] == false; ii++) {
                if (grid[ii * gydim + jj] < clipvalue) {
                  smask[kgrid] = true;
                }
              }

              jj = j2;
              for (int ii = i1; ii <= i2 && smask[kgrid] == false; ii++) {
                if (grid[ii * gydim + jj] < clipvalue) {
                  smask[kgrid] = true;
                }
              }

              int ii = i1;
              for (jj = j1; jj <= j2 && smask[kgrid] == false; jj++) {
                if (grid[ii * gydim + jj] < clipvalue) {
                  smask[kgrid] = true;
                }
              }

              ii = i2;
              for (jj = j1; jj <= j2 && smask[kgrid] == false; jj++) {
                if (grid[ii * gydim + jj] < clipvalue) {
                  smask[kgrid] = true;
                }
              }
            }
          }
        }
      for (int i = 0; i < gxdim; i++)
        for (int j = 0; j < gydim; j++) {
          kgrid = i * gydim + j;
#ifdef USESURFACE
          kint = i + (gydim - j - 1) * gxdim;
#else
          kint = i + j * gxdim;
#endif
          if (smask[kgrid] == true) {
            grid[kgrid] = sgrid[kint];
            nbinspline++;
          }
        }
    }

    /* translate the interpolation into the grid array
        filling all empty bins */
    else {
      for (int i = 0; i < gxdim; i++)
        for (int j = 0; j < gydim; j++) {
          kgrid = i * gydim + j;
#ifdef USESURFACE
          kint = i + (gydim - j - 1) * gxdim;
#else
          kint = i + j * gxdim;
#endif
          if (grid[kgrid] >= clipvalue && sgrid[kint] < zflag) {
            grid[kgrid] = sgrid[kint];
            nbinspline++;
          }
        }
    }

/* deallocate the interpolation arrays */
#ifdef USESURFACE
    mb_freed(verbose, __FILE__, __LINE__, (void **)&sxdata, &error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&sydata, &error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&szdata, &error);
#else
    mb_freed(verbose, __FILE__, __LINE__, (void **)&sdata, &error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&work1, &error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&work2, &error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&work3, &error);
#endif
    mb_freed(verbose, __FILE__, __LINE__, (void **)&smask, &error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&sgrid, &error);
  }
/* -------------------------------------------------------------------------- */

  /* if grdrasterid set and background data previously read in
      then interpolate it onto internal grid */
  if (grdrasterid != 0 && nbackground > 0) {

/* allocate and initialize grid and work arrays */
#ifdef USESURFACE
    status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(float), (void **)&sgrid, &error);
    if (error != MB_ERROR_NO_ERROR) {
      char *message = nullptr;
      mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
      fprintf(outfp, "\nMBIO Error allocating background data array:\n%s\n", message);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      mb_memory_clear(verbose, &memclear_error);
      exit(error);
    }
    memset((char *)sgrid, 0, gxdim * gydim * sizeof(float));
#else
    status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(float), (void **)&sgrid, &error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, nbackground * sizeof(float), (void **)&work1, &error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, nbackground * sizeof(int), (void **)&work2, &error);
    if (status == MB_SUCCESS)
      status = mb_mallocd(verbose, __FILE__, __LINE__, (gxdim + gydim) * sizeof(int), (void **)&work3, &error);
    if (error != MB_ERROR_NO_ERROR) {
      char *message = nullptr;
      mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
      fprintf(outfp, "\nMBIO Error allocating background interpolation work arrays:\n%s\n", message);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      mb_memory_clear(verbose, &memclear_error);
      exit(error);
    }
    memset((char *)sgrid, 0, gxdim * gydim * sizeof(float));
    memset((char *)work1, 0, nbackground * sizeof(float));
    memset((char *)work2, 0, nbackground * sizeof(int));
    memset((char *)work3, 0, (gxdim + gydim) * sizeof(int));
#endif

    /* do the interpolation */
    fprintf(outfp, "\nDoing spline interpolation with %d background points...\n", nbackground);
#ifdef USESURFACE
    mb_surface(verbose, nbackground, bxdata, bydata, bzdata, (float)(wbnd[0] - bdata_origin_x),
               (float)(wbnd[1] - bdata_origin_x), (float)(wbnd[2] - bdata_origin_y), (float)(wbnd[3] - bdata_origin_y), dx,
               dy, tension, sgrid);
#else
    float cay = (float)tension;
    float xmin = (float)(wbnd[0] - 0.5 * dx - bdata_origin_x);
    float ymin = (float)(wbnd[2] - 0.5 * dy - bdata_origin_y);
    float ddx = (float)dx;
    float ddy = (float)dy;
    clip = std::max(gxdim, gydim);
    fprintf(outfp, "\nDoing Zgrid spline interpolation with %d background points...\n", nbackground);
    mb_zgrid(sgrid, &gxdim, &gydim, &xmin, &ymin, &ddx, &ddy, bdata, &nbackground, work1, work2, work3, &cay, &clip);
#endif

    /* translate the interpolation into the grid array
        - interpolate only to fill a data gap */
    const float zflag = 5.0e34f;
    for (int i = 0; i < gxdim; i++)
      for (int j = 0; j < gydim; j++) {
        kgrid = i * gydim + j;
#ifdef USESURFACE
        kint = i + (gydim - j - 1) * gxdim;
#else
        kint = i + j * gxdim;
#endif
        if (grid[kgrid] >= clipvalue && sgrid[kint] < zflag) {
          grid[kgrid] = sgrid[kint];
          nbinbackground++;
        }
      }
#ifdef USESURFACE
    mb_freed(verbose, __FILE__, __LINE__, (void **)&bxdata, &error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&bydata, &error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&bzdata, &error);
#else
    mb_freed(verbose, __FILE__, __LINE__, (void **)&bdata, &error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&work1, &error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&work2, &error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&work3, &error);
#endif
    mb_freed(verbose, __FILE__, __LINE__, (void **)&sgrid, &error);
  }
/* -------------------------------------------------------------------------- */

  /* get min max of data */
  zclip = clipvalue;
  zmin = zclip;
  zmax = zclip;
  for (int i = 0; i < gxdim; i++)
    for (int j = 0; j < gydim; j++) {
      kgrid = i * gydim + j;
      if (zmin == zclip && grid[kgrid] < zclip)
        zmin = grid[kgrid];
      if (zmax == zclip && grid[kgrid] < zclip)
        zmax = grid[kgrid];
      if (grid[kgrid] < zmin && grid[kgrid] < zclip)
        zmin = grid[kgrid];
      if (grid[kgrid] > zmax && grid[kgrid] < zclip)
        zmax = grid[kgrid];
    }
  if (zmin == zclip)
    zmin = 0.0;
  if (zmax == zclip)
    zmax = 0.0;

  /* get min max of data distribution */
  nmax = 0;
  for (int i = 0; i < gxdim; i++)
    for (int j = 0; j < gydim; j++) {
      kgrid = i * gydim + j;
      if (cnt[kgrid] > nmax)
        nmax = cnt[kgrid];
    }

  /* get min max of standard deviation */
  smin = 0.0;
  smax = 0.0;
  for (int i = 0; i < gxdim; i++)
    for (int j = 0; j < gydim; j++) {
      kgrid = i * gydim + j;
      if (smin == 0.0 && cnt[kgrid] > 0)
        smin = sigma[kgrid];
      if (smax == 0.0 && cnt[kgrid] > 0)
        smax = sigma[kgrid];
      if (sigma[kgrid] < smin && cnt[kgrid] > 0)
        smin = sigma[kgrid];
      if (sigma[kgrid] > smax && cnt[kgrid] > 0)
        smax = sigma[kgrid];
    }
  nbinzero = gxdim * gydim - nbinset - nbinspline - nbinbackground;
  fprintf(outfp, "\nTotal number of bins:            %d\n", gxdim * gydim);
  fprintf(outfp, "Bins set using data:             %d\n", nbinset);
  fprintf(outfp, "Bins set using interpolation:    %d\n", nbinspline);
  fprintf(outfp, "Bins set using background:       %d\n", nbinbackground);
  fprintf(outfp, "Bins not set:                    %d\n", nbinzero);
  fprintf(outfp, "Maximum number of data in a bin: %d\n", nmax);
  fprintf(outfp, "Minimum value: %10.2f   Maximum value: %10.2f\n", zmin, zmax);
  fprintf(outfp, "Minimum sigma: %10.5f   Maximum sigma: %10.5f\n", smin, smax);

  /* Apply shift to the output grid bounds if specified */
  if (shift && use_projection) {
    gbnd[0] += shift_x;
    gbnd[1] += shift_x;
    gbnd[2] += shift_y;
    gbnd[3] += shift_y;
  }
  else if (shift) {
    gbnd[0] += shift_x * mtodeglon;
    gbnd[1] += shift_x * mtodeglon;
    gbnd[2] += shift_y * mtodeglat;
    gbnd[3] += shift_y * mtodeglat;
  }

  /* write first output file */
  if (verbose > 0)
    fprintf(outfp, "\nOutputting results...\n");
  for (int i = 0; i < xdim; i++)
    for (int j = 0; j < ydim; j++) {
      kgrid = (i + offx) * gydim + (j + offy);
      kout = i * ydim + j;
      output[kout] = (float)grid[kgrid];
      if (gridkind != MBGRID_ASCII && gridkind != MBGRID_ARCASCII && grid[kgrid] >= clipvalue) {
        output[kout] = outclipvalue;
      }
    }
  if (gridkind == MBGRID_ASCII) {
    strcpy(ofile, fileroot);
    strcat(ofile, ".asc");
    status = write_ascii(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, &error);
  }
  else if (gridkind == MBGRID_ARCASCII) {
    strcpy(ofile, fileroot);
    strcat(ofile, ".asc");
    status =
        write_arcascii(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, outclipvalue, &error);
  }
  else if (gridkind == MBGRID_OLDGRD) {
    strcpy(ofile, fileroot);
    strcat(ofile, ".grd1");
    status = write_oldgrd(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, &error);
  }
  else if (gridkind == MBGRID_CDFGRD) {
    strcpy(ofile, fileroot);
    strcat(ofile, ".grd");
    status = mb_write_gmt_grd(verbose, ofile, output, outclipvalue, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], zmin,
                              zmax, dx, dy, xlabel, ylabel, zlabel, title, projection_id, argc, argv, &error);
  }
  else if (gridkind == MBGRID_GMTGRD) {
    strcpy(ofile, fileroot);
    strcat(ofile, ".grd");
    snprintf(ofile, sizeof(ofile), "%s.grd%s", fileroot, gridkindstring);
    status = mb_write_gmt_grd(verbose, ofile, output, outclipvalue, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], zmin,
                              zmax, dx, dy, xlabel, ylabel, zlabel, title, projection_id, argc, argv, &error);
  }
  if (status != MB_SUCCESS) {
    char *message = nullptr;
    mb_error(verbose, error, &message);
    fprintf(outfp, "\nError writing output file: %s\n%s\n", ofile, message);
    fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
    mb_memory_clear(verbose, &memclear_error);
    exit(error);
  }

  /* write second output file */
  if (more) {
    for (int i = 0; i < xdim; i++)
      for (int j = 0; j < ydim; j++) {
        kgrid = (i + offx) * gydim + (j + offy);
        kout = i * ydim + j;
        output[kout] = (float)cnt[kgrid];
        if (output[kout] < 0.0)
          output[kout] = 0.0;
        if (gridkind != MBGRID_ASCII && gridkind != MBGRID_ARCASCII && cnt[kgrid] <= 0)
          output[kout] = outclipvalue;
      }
    if (gridkind == MBGRID_ASCII) {
      strcpy(ofile, fileroot);
      strcat(ofile, "_num.asc");
      status = write_ascii(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, &error);
    }
    else if (gridkind == MBGRID_ARCASCII) {
      strcpy(ofile, fileroot);
      strcat(ofile, "_num.asc");
      status = write_arcascii(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, outclipvalue,
                              &error);
    }
    else if (gridkind == MBGRID_OLDGRD) {
      strcpy(ofile, fileroot);
      strcat(ofile, "_num.grd1");
      status = write_oldgrd(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, &error);
    }
    else if (gridkind == MBGRID_CDFGRD) {
      strcpy(ofile, fileroot);
      strcat(ofile, "_num.grd");
      status = mb_write_gmt_grd(verbose, ofile, output, outclipvalue, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], zmin,
                                zmax, dx, dy, xlabel, ylabel, zlabel, title, projection_id, argc, argv, &error);
    }
    else if (gridkind == MBGRID_GMTGRD) {
      snprintf(ofile, sizeof(ofile), "%s_num.grd%s", fileroot, gridkindstring);
      status = mb_write_gmt_grd(verbose, ofile, output, outclipvalue, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], zmin,
                                zmax, dx, dy, xlabel, ylabel, zlabel, title, projection_id, argc, argv, &error);
    }
    if (status != MB_SUCCESS) {
      char *message = nullptr;
      mb_error(verbose, error, &message);
      fprintf(outfp, "\nError writing output file: %s\n%s\n", ofile, message);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      mb_memory_clear(verbose, &memclear_error);
      exit(error);
    }

    /* write third output file */
    for (int i = 0; i < xdim; i++)
      for (int j = 0; j < ydim; j++) {
        kgrid = (i + offx) * gydim + (j + offy);
        kout = i * ydim + j;
        output[kout] = (float)sigma[kgrid];
        if (output[kout] < 0.0)
          output[kout] = 0.0;
        if (gridkind != MBGRID_ASCII && gridkind != MBGRID_ARCASCII && cnt[kgrid] <= 0)
          output[kout] = outclipvalue;
      }
    if (gridkind == MBGRID_ASCII) {
      strcpy(ofile, fileroot);
      strcat(ofile, "_sd.asc");
      status = write_ascii(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, &error);
    }
    else if (gridkind == MBGRID_ARCASCII) {
      strcpy(ofile, fileroot);
      strcat(ofile, "_sd.asc");
      status = write_arcascii(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, outclipvalue,
                              &error);
    }
    else if (gridkind == MBGRID_OLDGRD) {
      strcpy(ofile, fileroot);
      strcat(ofile, "_sd.grd1");
      status = write_oldgrd(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, &error);
    }
    else if (gridkind == MBGRID_CDFGRD) {
      strcpy(ofile, fileroot);
      strcat(ofile, "_sd.grd");
      status = mb_write_gmt_grd(verbose, ofile, output, outclipvalue, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], zmin,
                                zmax, dx, dy, xlabel, ylabel, zlabel, title, projection_id, argc, argv, &error);
    }
    else if (gridkind == MBGRID_GMTGRD) {
      snprintf(ofile, sizeof(ofile), "%s_sd.grd%s", fileroot, gridkindstring);
      status = mb_write_gmt_grd(verbose, ofile, output, outclipvalue, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], zmin,
                                zmax, dx, dy, xlabel, ylabel, zlabel, title, projection_id, argc, argv, &error);
    }
    if (status != MB_SUCCESS) {
      char *message = nullptr;
      mb_error(verbose, error, &message);
      fprintf(outfp, "\nError writing output file: %s\n%s\n", ofile, message);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      mb_memory_clear(verbose, &memclear_error);
      exit(error);
    }
  }

  /* deallocate arrays */
  mb_freed(verbose, __FILE__, __LINE__, (void **)&grid, &error);
  mb_freed(verbose, __FILE__, __LINE__, (void **)&norm, &error);
  mb_freed(verbose, __FILE__, __LINE__, (void **)&num, &error);
  mb_freed(verbose, __FILE__, __LINE__, (void **)&cnt, &error);
  mb_freed(verbose, __FILE__, __LINE__, (void **)&sigma, &error);
  mb_freed(verbose, __FILE__, __LINE__, (void **)&firsttime, &error);
  mb_freed(verbose, __FILE__, __LINE__, (void **)&output, &error);
  mb_freed(verbose, __FILE__, __LINE__, (void **)&minormax, &error);

  /* deallocate projection */
  if (use_projection)
    /* proj_status = */ mb_proj_free(verbose, &(pjptr), &error);

  /* run mbm_grdplot */
  if (gridkind == MBGRID_GMTGRD) {
    /* execute mbm_grdplot */
    strcpy(ofile, fileroot);
    strcat(ofile, ".grd");
    if (datatype == MBGRID_DATA_BATHYMETRY) {
      snprintf(plot_cmd, sizeof(plot_cmd), "mbm_grdplot -I%s%s -G1 -C -D -V -L\"File %s - %s:%s\"", ofile, gridkindstring, ofile, title, zlabel);
    } else if (datatype == MBGRID_DATA_TOPOGRAPHY) {
      snprintf(plot_cmd, sizeof(plot_cmd), "mbm_grdplot -I%s%s -G1 -C -V -L\"File %s - %s:%s\"", ofile, gridkindstring, ofile, title, zlabel);
    } else { // if (datatype == MBGRID_DATA_AMPLITUDE || datatype == MBGRID_DATA_SIDESCAN) {
      snprintf(plot_cmd, sizeof(plot_cmd), "mbm_grdplot -I%s%s -G1 -W1/4 -S -D -V -L\"File %s - %s:%s\"", ofile, gridkindstring, ofile, title, zlabel);
    }
    if (verbose) {
      fprintf(outfp, "\nexecuting mbm_grdplot...\n%s\n", plot_cmd);
    }
    plot_status = system(plot_cmd);
    if (plot_status == -1) {
      fprintf(outfp, "\nError executing mbm_grdplot on output file %s\n", ofile);
    }
  }
  if (more && gridkind == MBGRID_GMTGRD) {
    /* execute mbm_grdplot */
    strcpy(ofile, fileroot);
    strcat(ofile, "_num.grd");
    snprintf(plot_cmd, sizeof(plot_cmd), "mbm_grdplot -I%s%s -G1 -W1/2 -V -L\"File %s - %s:%s\"", ofile, gridkindstring, ofile, title, nlabel);
    if (verbose) {
      fprintf(outfp, "\nexecuting mbm_grdplot...\n%s\n", plot_cmd);
    }
    plot_status = system(plot_cmd);
    if (plot_status == -1) {
      fprintf(outfp, "\nError executing mbm_grdplot on output file grd_%s\n", fileroot);
    }

    /* execute mbm_grdplot */
    strcpy(ofile, fileroot);
    strcat(ofile, "_sd.grd");
    snprintf(plot_cmd, sizeof(plot_cmd), "mbm_grdplot -I%s%s -G1 -W1/2 -V -L\"File %s - %s:%s\"", ofile, gridkindstring, ofile, title, sdlabel);
    if (verbose) {
      fprintf(outfp, "\nexecuting mbm_grdplot...\n%s\n", plot_cmd);
    }
    plot_status = system(plot_cmd);
    if (plot_status == -1) {
      fprintf(outfp, "\nError executing mbm_grdplot on output file grd_%s\n", fileroot);
    }
  }

  if (verbose > 0)
    fprintf(outfp, "\nDone.\n\n");

  /* check memory */
  if (verbose >= 4)
    status = mb_memory_list(verbose, &error);

  if (verbose >= 2) {
    fprintf(outfp, "\ndbg2  Program <%s> completed\n", program_name);
    fprintf(outfp, "dbg2  Ending status:\n");
    fprintf(outfp, "dbg2       status:  %d\n", status);
  }

  exit(error);
}
/*--------------------------------------------------------------------*/
