/*--------------------------------------------------------------------
 *    The MB-system:  mbgrid.c  5/2/94
 *
 *    Copyright (c) 1993-2025 by
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
/*
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
 *
 * GMT module port of utilities/mbgrid.cc — November 2026.
 */

#define THIS_MODULE_NAME    "mbgrid"
#define THIS_MODULE_LIB     "mbsystem"
#define THIS_MODULE_PURPOSE "grid bathymetry, amplitude, or sidescan data of a set of swath sonar data files"
#define THIS_MODULE_KEYS    "ID{,OG}"
#define THIS_MODULE_NEEDS   ""
#define THIS_MODULE_OPTIONS "->V"

#include "gmt_dev.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_format.h"
#include "mb_info.h"
#include "mb_io.h"
#include "mb_status.h"

/* gridding algorithms */
#define MBGRID_WEIGHTED_MEAN              1
#define MBGRID_MEDIAN_FILTER              2
#define MBGRID_MINIMUM_FILTER             3
#define MBGRID_MAXIMUM_FILTER             4
#define MBGRID_WEIGHTED_FOOTPRINT_SLOPE   5
#define MBGRID_WEIGHTED_FOOTPRINT         6
#define MBGRID_MINIMUM_WEIGHTED_MEAN      7
#define MBGRID_MAXIMUM_WEIGHTED_MEAN      8

/* grid format definitions */
#define MBGRID_ASCII    1
#define MBGRID_OLDGRD   2
#define MBGRID_CDFGRD   3
#define MBGRID_ARCASCII 4
#define MBGRID_GMTGRD   100

/* gridded data type */
#define MBGRID_DATA_BATHYMETRY 1
#define MBGRID_DATA_TOPOGRAPHY 2
#define MBGRID_DATA_AMPLITUDE  3
#define MBGRID_DATA_SIDESCAN   4

/* flag for no data in grid */
#define NO_DATA_FLAG 99999

/* number of data to be allocated at a time */
#define REALLOC_STEP_SIZE 25

/* usage of footprint based weight */
#define MBGRID_USE_NO          0
#define MBGRID_USE_YES         1
#define MBGRID_USE_CONDITIONAL 2

/* interpolation mode */
#define MBGRID_INTERP_NONE 0
#define MBGRID_INTERP_GAP  1
#define MBGRID_INTERP_NEAR 2
#define MBGRID_INTERP_ALL  3

/* shift mode */
#define MBGRID_SHIFT_NONE   0
#define MBGRID_SHIFT_BOUNDS 1
#define MBGRID_SHIFT_DATA   2

/* comparison threshold */
#define MBGRID_TINY 0.00000001

/* maximum allowed beam grazing angle */
#define FOOT_THETA_MAX 85.0

/* interpolation algorithm: zgrid (default) vs surface */
/* #define USESURFACE */

/* program identifiers */
static const char help_message[] =
	"mbgrid is an utility used to grid bathymetry, amplitude, or\n"
	"sidescan data contained in a set of swath sonar data files.\n"
	"This program uses one of four algorithms (gaussian weighted mean,\n"
	"median filter, minimum filter, maximum filter) to grid regions\n"
	"covered swaths and then fills in gaps between\n"
	"the swaths (to the degree specified by the user) using a minimum\n"
	"curvature algorithm.";
static const char usage_message[] =
	"mbgrid   -Ifilelist -Oroot [-Adatatype -Bborder -Cclip[/mode] -Dxdim/ydim\n"
	"          -Edx/dy/units[!]  -Fmode[/threshold] -Ggridkind -Jprojection\n"
	"          -Kbackground -Llonflip -M -N -Ppings -Q  -Rwest/east/south/north\n"
	"          -Rfactor  -Sspeed  -Ttension  -Utime  -V -Wscale -Xextend\n"
	"          -Yshift_x/shift_y[/shift_mode]]";

/*--------------------------------------------------------------------*/
/* approximate error function altered from numerical recipes */
static double mbgrid_erf(double x) {
	const double z = fabs(x);
	const double t = 1.0 / (1.0 + 0.5 * z);
	double erfc_d = t * exp(-z * z - 1.26551223 + t * (1.00002368 + t * (0.37409196 + t * (0.09678418 + t * (-0.18628806 +
	                t * (0.27886807 + t * (-1.13520398 + t * (1.48851587 + t * (-0.82215223 + t * 0.17087277)))))))));
	erfc_d = x >= 0.0 ? erfc_d : 2.0 - erfc_d;
	return 1.0 - erfc_d;
}

/*--------------------------------------------------------------------*/
/* write_ascii — write output grid to an ascii file */
static int write_ascii(int verbose, char *outfile, float *grid, int nx, int ny, double xmin, double xmax,
					   double ymin, double ymax, double dx, double dy, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       outfile:    %s\n", outfile);
		fprintf(stderr, "dbg2       grid:       %p\n", (void *)grid);
		fprintf(stderr, "dbg2       nx:         %d\n", nx);
		fprintf(stderr, "dbg2       ny:         %d\n", ny);
		fprintf(stderr, "dbg2       xmin:       %f\n", xmin);
		fprintf(stderr, "dbg2       xmax:       %f\n", xmax);
		fprintf(stderr, "dbg2       ymin:       %f\n", ymin);
		fprintf(stderr, "dbg2       ymax:       %f\n", ymax);
		fprintf(stderr, "dbg2       dx:         %f\n", dx);
		fprintf(stderr, "dbg2       dy:         %f\n", dy);
	}

	int status = MB_SUCCESS;
	FILE *fp = fopen(outfile, "w");
	if (fp == NULL) {
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
	} else {
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
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}
	return status;
}

/*--------------------------------------------------------------------*/
/* write_arcascii — write output grid to an Arc/Info ascii file */
static int write_arcascii(int verbose, char *outfile, float *grid, int nx, int ny, double xmin, double xmax,
						  double ymin, double ymax, double dx, double dy, double nodata, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       outfile:    %s\n", outfile);
		fprintf(stderr, "dbg2       grid:       %p\n", (void *)grid);
		fprintf(stderr, "dbg2       nx:         %d\n", nx);
		fprintf(stderr, "dbg2       ny:         %d\n", ny);
		fprintf(stderr, "dbg2       xmin:       %f\n", xmin);
		fprintf(stderr, "dbg2       xmax:       %f\n", xmax);
		fprintf(stderr, "dbg2       ymin:       %f\n", ymin);
		fprintf(stderr, "dbg2       ymax:       %f\n", ymax);
		fprintf(stderr, "dbg2       dx:         %f\n", dx);
		fprintf(stderr, "dbg2       dy:         %f\n", dy);
		fprintf(stderr, "dbg2       nodata:     %f\n", nodata);
	}

	int status = MB_SUCCESS;
	FILE *fp = fopen(outfile, "w");
	if (fp == NULL) {
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
	} else {
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
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}
	return status;
}

/*--------------------------------------------------------------------*/
/* write_oldgrd — write output grid to a GMT v1 binary grd file */
static int write_oldgrd(int verbose, char *outfile, float *grid, int nx, int ny, double xmin, double xmax,
						double ymin, double ymax, double dx, double dy, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       outfile:    %s\n", outfile);
		fprintf(stderr, "dbg2       grid:       %p\n", (void *)grid);
		fprintf(stderr, "dbg2       nx:         %d\n", nx);
		fprintf(stderr, "dbg2       ny:         %d\n", ny);
		fprintf(stderr, "dbg2       xmin:       %f\n", xmin);
		fprintf(stderr, "dbg2       xmax:       %f\n", xmax);
		fprintf(stderr, "dbg2       ymin:       %f\n", ymin);
		fprintf(stderr, "dbg2       ymax:       %f\n", ymax);
		fprintf(stderr, "dbg2       dx:         %f\n", dx);
		fprintf(stderr, "dbg2       dy:         %f\n", dy);
	}

	int status = MB_SUCCESS;
	FILE *fp = fopen(outfile, "w");
	if (fp == NULL) {
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
	} else {
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
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}
	return status;
}

/*--------------------------------------------------------------------*/
/* mbgrid_weight — integrated weight over a bin given the footprint of a sounding */
static int mbgrid_weight(int verbose, double foot_a, double foot_b, double pcx, double pcy, double dx, double dy,
						 double *px, double *py, double *weight, int *use, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       foot_a:     %f\n", foot_a);
		fprintf(stderr, "dbg2       foot_b:     %f\n", foot_b);
		fprintf(stderr, "dbg2       pcx:        %f\n", pcx);
		fprintf(stderr, "dbg2       pcy:        %f\n", pcy);
		fprintf(stderr, "dbg2       dx:         %f\n", dx);
		fprintf(stderr, "dbg2       dy:         %f\n", dy);
		fprintf(stderr, "dbg2       p1 x:       %f\n", px[0]);
		fprintf(stderr, "dbg2       p1 y:       %f\n", py[0]);
		fprintf(stderr, "dbg2       p2 x:       %f\n", px[1]);
		fprintf(stderr, "dbg2       p2 y:       %f\n", py[1]);
		fprintf(stderr, "dbg2       p3 x:       %f\n", px[2]);
		fprintf(stderr, "dbg2       p3 y:       %f\n", py[2]);
		fprintf(stderr, "dbg2       p4 x:       %f\n", px[3]);
		fprintf(stderr, "dbg2       p4 y:       %f\n", py[3]);
	}

	/* The weighting function is
		w(x, y) = (1 / (PI * a * b)) * exp(-(x**2/a**2 + y**2/b**2))
		in the footprint coordinate system, where the x axis
		is along the horizontal projection of the beam and the
		y axix is perpendicular to that. */

	const double fa = foot_a;
	const double fb = foot_b;
	*weight = 0.25 * (mbgrid_erf((pcx + dx) / fa) - mbgrid_erf((pcx - dx) / fa)) *
			  (mbgrid_erf((pcy + dy) / fb) - mbgrid_erf((pcy - dy) / fb));

	if (*weight > 0.05) {
		*use = MBGRID_USE_YES;
	} else {
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
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2       weight:     %f\n", *weight);
		fprintf(stderr, "dbg2       use:        %d\n", *use);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}
	return status;
}

/*--------------------------------------------------------------------*/
/* Control structure for mbgrid */
struct MBGRID_CTRL {
	struct mbgrid_A { bool active; int datatype; } A;
	struct mbgrid_B { bool active; double border; } B;
	struct mbgrid_C { bool active; int clip; int clipmode; double tension; } C;
	struct mbgrid_D { bool active; int xdim, ydim; } D;
	struct mbgrid_E { bool active; char units[MB_PATH_MAXLINE]; double dx_set, dy_set; bool spacing_priority; bool set_spacing; } E;
	struct mbgrid_F { bool active; int grid_mode; double threshold; } F;
	struct mbgrid_G { bool active; char gridkindstring[MB_PATH_MAXLINE]; int gridkind; } G;
	struct mbgrid_I { bool active; char inputfile[MB_PATH_MAXLINE]; } I;
	struct mbgrid_J { bool active; char projection_pars[MB_PATH_MAXLINE]; } J;
	struct mbgrid_K { bool active; char backgroundfile[MB_PATH_MAXLINE]; int grdrasterid; } K;
	struct mbgrid_L { bool active; int lonflip; } L;
	struct mbgrid_M { bool active; bool more; } M;
	struct mbgrid_N { bool active; } N;
	struct mbgrid_O { bool active; char fileroot[MB_PATH_MAXLINE]; } O;
	struct mbgrid_P { bool active; int pings; } P;
	struct mbgrid_Q { bool active; } Q;
	struct mbgrid_R { bool active; double boundsfactor; double gbnd[4]; bool gbndset; } R;
	struct mbgrid_S { bool active; double speedmin; } S;
	struct mbgrid_T { bool active; double tension; } T;
	struct mbgrid_U { bool active; double timediff; bool check_time; bool first_in_stays; } U;
	struct mbgrid_W { bool active; double scale; } W;
	struct mbgrid_X { bool active; double extend; } X;
	struct mbgrid_Y { bool active; double shift_x, shift_y; int shift_mode; } Y;
};

/*--------------------------------------------------------------------*/
static void *New_mbgrid_Ctrl(struct GMT_CTRL *GMT) {
	struct MBGRID_CTRL *Ctrl = gmt_M_memory(GMT, NULL, 1, struct MBGRID_CTRL);
	Ctrl->A.datatype = MBGRID_DATA_BATHYMETRY;
	Ctrl->C.clipmode = MBGRID_INTERP_NONE;
#ifdef USESURFACE
	Ctrl->C.tension = 0.35;
	Ctrl->T.tension = 0.35;
#else
	Ctrl->C.tension = 0.0;
	Ctrl->T.tension = 0.0;
#endif
	Ctrl->D.xdim = 101;
	Ctrl->D.ydim = 101;
	Ctrl->F.grid_mode = MBGRID_WEIGHTED_MEAN;
	Ctrl->F.threshold = 1.0;
	Ctrl->G.gridkind = MBGRID_GMTGRD;
	strcpy(Ctrl->I.inputfile, "datalist.mb-1");
	Ctrl->P.pings = 1;
	Ctrl->S.speedmin = 0.0;
	Ctrl->U.timediff = 300.0;
	Ctrl->U.first_in_stays = true;
	Ctrl->W.scale = 1.0;
	Ctrl->X.extend = 0.0;
	Ctrl->Y.shift_mode = MBGRID_SHIFT_NONE;
	strcpy(Ctrl->O.fileroot, "grid");
	return Ctrl;
}

static void Free_mbgrid_Ctrl(struct GMT_CTRL *GMT, struct MBGRID_CTRL *Ctrl) {
	if (!Ctrl) return;
	gmt_M_free(GMT, Ctrl);
}

/*--------------------------------------------------------------------*/
static int usage(struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose(API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return GMT_NOERROR;
	GMT_Message(API, GMT_TIME_NONE, "%s\n\n", help_message);
	GMT_Message(API, GMT_TIME_NONE, "usage: %s\n", usage_message);
	if (level == GMT_SYNOPSIS) return EXIT_FAILURE;
	GMT_Message(API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-A<datatype>\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-B<border>\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-C<clip>[/<mode>[/<tension>]]\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-D<xdim>/<ydim>\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-E<dx>/<dy>/<units>[!]\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-F<mode>[/<threshold>]\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-G<gridkind>\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-I<filelist>\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-J<projection>\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-K<backgroundfile>\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-L<lonflip>\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-M (output data density and sigma grids)\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-N (use NaN for no-data flag)\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-O<fileroot>\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-P<pings>\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-Q (output bathymetry in feet)\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-R<west>/<east>/<south>/<north> | -R<factor>\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-S<speedmin>\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-T<tension>\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-U<timediff>\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-W<scale>\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-X<extend>\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-Y<shift_x>/<shift_y>[/<shift_mode>]\n");
	return EXIT_FAILURE;
}

/*--------------------------------------------------------------------*/
static int parse_mbgrid(struct GMT_CTRL *GMT, struct MBGRID_CTRL *Ctrl, struct GMT_OPTION *options) {
	unsigned int n_errors = 0;
	unsigned int n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		int n;
		switch (opt->option) {
			case '<':
				Ctrl->I.active = true;
				strncpy(Ctrl->I.inputfile, opt->arg, MB_PATH_MAXLINE - 1);
				n_files = 1;
				break;

			case 'A':
				n = sscanf(opt->arg, "%d", &Ctrl->A.datatype);
				if (n > 0) Ctrl->A.active = true;
				else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -A option\n"); n_errors++; }
				break;

			case 'B':
				n = sscanf(opt->arg, "%lf", &Ctrl->B.border);
				if (n > 0) Ctrl->B.active = true;
				else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -B option\n"); n_errors++; }
				break;

			case 'C':
				Ctrl->C.active = true;
				n = sscanf(opt->arg, "%d/%d/%lf", &Ctrl->C.clip, &Ctrl->C.clipmode, &Ctrl->C.tension);
				if (n < 1)
					Ctrl->C.clipmode = MBGRID_INTERP_NONE;
				else if (n == 1 && Ctrl->C.clip > 0)
					Ctrl->C.clipmode = MBGRID_INTERP_GAP;
				else if (n == 1)
					Ctrl->C.clipmode = MBGRID_INTERP_NONE;
				else if (Ctrl->C.clip > 0 && Ctrl->C.clipmode < 0)
					Ctrl->C.clipmode = MBGRID_INTERP_GAP;
				else if (Ctrl->C.clipmode >= 3)
					Ctrl->C.clipmode = MBGRID_INTERP_ALL;
				if (n < 3) {
#ifdef USESURFACE
					Ctrl->C.tension = 0.35;
#else
					Ctrl->C.tension = 0.0;
#endif
				}
				break;

			case 'D':
				n = sscanf(opt->arg, "%d/%d", &Ctrl->D.xdim, &Ctrl->D.ydim);
				if (n > 0) Ctrl->D.active = true;
				else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -D option\n"); n_errors++; }
				break;

			case 'E':
				if (opt->arg[strlen(opt->arg) - 1] == '!') {
					Ctrl->E.spacing_priority = true;
					opt->arg[strlen(opt->arg) - 1] = '\0';
				}
				n = sscanf(opt->arg, "%lf/%lf/%s", &Ctrl->E.dx_set, &Ctrl->E.dy_set, Ctrl->E.units);
				if (n > 0) {
					Ctrl->E.active = true;
					if (n > 1) Ctrl->E.set_spacing = true;
					if (n < 3) strcpy(Ctrl->E.units, "meters");
				} else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -E option\n"); n_errors++; }
				break;

			case 'F': {
				int tmp;
				double dvalue;
				n = sscanf(opt->arg, "%d/%lf", &tmp, &dvalue);
				if (n > 0) {
					Ctrl->F.grid_mode = tmp;
					Ctrl->F.active = true;
					if (n == 2) {
						if (Ctrl->F.grid_mode == MBGRID_MINIMUM_FILTER) {
							Ctrl->F.threshold = dvalue;
							Ctrl->F.grid_mode = MBGRID_MINIMUM_WEIGHTED_MEAN;
						} else if (Ctrl->F.grid_mode == MBGRID_MAXIMUM_FILTER) {
							Ctrl->F.threshold = dvalue;
							Ctrl->F.grid_mode = MBGRID_MAXIMUM_WEIGHTED_MEAN;
						} else {
							Ctrl->F.threshold = dvalue;
						}
					}
				} else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -F option\n"); n_errors++; }
				break;
			}

			case 'G':
				if (opt->arg[0] == '=') {
					Ctrl->G.gridkind = MBGRID_GMTGRD;
					strcpy(Ctrl->G.gridkindstring, opt->arg);
					Ctrl->G.active = true;
				} else {
					int tmp;
					int nscan = sscanf(opt->arg, "%d", &tmp);
					if (nscan == 1 && tmp >= 1 && tmp <= 4) {
						Ctrl->G.gridkind = tmp;
						if (Ctrl->G.gridkind == MBGRID_CDFGRD) {
							Ctrl->G.gridkind = MBGRID_GMTGRD;
							Ctrl->G.gridkindstring[0] = '\0';
						}
						Ctrl->G.active = true;
					} else if (opt->arg[0] == 'n' || opt->arg[0] == 'c' || opt->arg[0] == 'b' ||
							   opt->arg[0] == 'r' || opt->arg[0] == 's' || opt->arg[0] == 'a' ||
							   opt->arg[0] == 'e' || opt->arg[0] == 'g') {
						snprintf(Ctrl->G.gridkindstring, sizeof(Ctrl->G.gridkindstring), "=%s", opt->arg);
						Ctrl->G.gridkind = MBGRID_GMTGRD;
						Ctrl->G.active = true;
					} else {
						GMT_Report(API, GMT_MSG_NORMAL, "Invalid gridkind option: -G%s\n", opt->arg);
						n_errors++;
					}
				}
				break;

			case 'I':
				if (!gmt_access(GMT, opt->arg, R_OK)) {
					strncpy(Ctrl->I.inputfile, opt->arg, MB_PATH_MAXLINE - 1);
					Ctrl->I.active = true;
					n_files = 1;
				} else {
					GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -I option: cannot access file %s\n", opt->arg);
					n_errors++;
				}
				break;

			case 'J':
				n = sscanf(opt->arg, "%s", Ctrl->J.projection_pars);
				if (n > 0) Ctrl->J.active = true;
				else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -J option\n"); n_errors++; }
				break;

			case 'K':
				n = sscanf(opt->arg, "%s", Ctrl->K.backgroundfile);
				if (n > 0) {
					Ctrl->K.active = true;
					if ((Ctrl->K.grdrasterid = (int)strtol(Ctrl->K.backgroundfile, NULL, 10)) <= 0)
						Ctrl->K.grdrasterid = -1;
				} else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -K option\n"); n_errors++; }
				break;

			case 'L':
				n = sscanf(opt->arg, "%d", &Ctrl->L.lonflip);
				if (n > 0) Ctrl->L.active = true;
				else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -L option\n"); n_errors++; }
				break;

			case 'M':
				Ctrl->M.active = true;
				Ctrl->M.more = true;
				break;

			case 'N':
				Ctrl->N.active = true;
				break;

			case 'O':
				n = sscanf(opt->arg, "%s", Ctrl->O.fileroot);
				if (n > 0) Ctrl->O.active = true;
				else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -O option\n"); n_errors++; }
				break;

			case 'P':
				n = sscanf(opt->arg, "%d", &Ctrl->P.pings);
				if (n > 0) Ctrl->P.active = true;
				else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -P option\n"); n_errors++; }
				break;

			case 'Q':
				Ctrl->Q.active = true;
				break;

			case 'R':
				Ctrl->R.active = true;
				if (strchr(opt->arg, '/') == NULL) {
					sscanf(opt->arg, "%lf", &Ctrl->R.boundsfactor);
					if (Ctrl->R.boundsfactor <= 1.0) Ctrl->R.boundsfactor = 0.0;
				} else {
					mb_get_bounds(opt->arg, Ctrl->R.gbnd);
					Ctrl->R.gbndset = true;
				}
				break;

			case 'S':
				n = sscanf(opt->arg, "%lf", &Ctrl->S.speedmin);
				if (n > 0) Ctrl->S.active = true;
				else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -S option\n"); n_errors++; }
				break;

			case 'T':
				n = sscanf(opt->arg, "%lf", &Ctrl->T.tension);
				if (n > 0) Ctrl->T.active = true;
				else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -T option\n"); n_errors++; }
				break;

			case 'U':
				n = sscanf(opt->arg, "%lf", &Ctrl->U.timediff);
				if (n > 0) {
					Ctrl->U.active = true;
					Ctrl->U.timediff *= 60;
					Ctrl->U.check_time = true;
					if (Ctrl->U.timediff < 0.0) {
						Ctrl->U.timediff = fabs(Ctrl->U.timediff);
						Ctrl->U.first_in_stays = false;
					} else {
						Ctrl->U.first_in_stays = true;
					}
				} else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -U option\n"); n_errors++; }
				break;

			case 'W':
				n = sscanf(opt->arg, "%lf", &Ctrl->W.scale);
				if (n > 0) Ctrl->W.active = true;
				else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -W option\n"); n_errors++; }
				break;

			case 'X':
				n = sscanf(opt->arg, "%lf", &Ctrl->X.extend);
				if (n > 0) Ctrl->X.active = true;
				else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -X option\n"); n_errors++; }
				break;

			case 'Y':
				n = sscanf(opt->arg, "%lf/%lf/%d", &Ctrl->Y.shift_x, &Ctrl->Y.shift_y, &Ctrl->Y.shift_mode);
				if (n >= 2) {
					Ctrl->Y.active = true;
					if (n == 2) Ctrl->Y.shift_mode = MBGRID_SHIFT_BOUNDS;
				} else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -Y option\n"); n_errors++; }
				break;

			default:
				n_errors += gmt_default_error(GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition(GMT, n_files != 1, "Syntax error: Must specify one input file\n");
	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {gmt_M_free_options(mode); return (code);}
#define Return(code) {Free_mbgrid_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

/*--------------------------------------------------------------------*/
int GMT_mbgrid(void *V_API, int mode, void *args) {
	int error = MB_ERROR_NO_ERROR;

	struct MBGRID_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr(V_API);

	if (API == NULL) return GMT_NOT_A_SESSION;
	if (mode == GMT_MODULE_PURPOSE) return usage(API, GMT_MODULE_PURPOSE);
	options = GMT_Create_Options(API, mode, args);
	if (API->error) return API->error;

	if (!options || options->option == GMT_OPT_USAGE) bailout(usage(API, GMT_USAGE));
	if (options->option == GMT_OPT_SYNOPSIS) bailout(usage(API, GMT_SYNOPSIS));

#if GMT_MAJOR_VERSION >= 6
	if ((GMT = gmt_init_module(API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout(API->error);
#else
	GMT = gmt_begin_module(API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy);
#endif
	if (GMT_Parse_Common(API, THIS_MODULE_OPTIONS, options)) Return(API->error);

	Ctrl = (struct MBGRID_CTRL *)New_mbgrid_Ctrl(GMT);
	if ((error = parse_mbgrid(GMT, Ctrl, options))) Return (error);

	/* MBIO status variables */
	int verbose = GMT->common.V.active ? GMT->current.setting.verbose : 0;
	int status = MB_SUCCESS;
	int memclear_error = MB_ERROR_NO_ERROR;
	char *message = NULL;

	/* MBIO read control parameters from defaults */
	int format;
	int pings;
	int lonflip;
	double bounds[4];
	int btime_i[7];
	int etime_i[7];
	double speedmin;
	double timegap;
	status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

	/* Pull control values from Ctrl */
	int datatype = Ctrl->A.datatype;
	double border = Ctrl->B.border;
	bool setborder = Ctrl->B.active;
	char gridkindstring[MB_PATH_MAXLINE];
	strcpy(gridkindstring, Ctrl->G.gridkindstring);
	int clip = Ctrl->C.clip;
	int xdim = Ctrl->D.xdim;
	int ydim = Ctrl->D.ydim;
	char fileroot[MB_PATH_MAXLINE];
	strcpy(fileroot, Ctrl->O.fileroot);
	char projection_id[MB_PATH_MAXLINE] = "Geographic";
	double gbnd[4];
	gbnd[0] = Ctrl->R.gbnd[0];
	gbnd[1] = Ctrl->R.gbnd[1];
	gbnd[2] = Ctrl->R.gbnd[2];
	gbnd[3] = Ctrl->R.gbnd[3];
	bool gbndset = Ctrl->R.gbndset;
	double scale = Ctrl->W.scale;
	double extend = Ctrl->X.extend;
	int shift_mode = Ctrl->Y.shift_mode;
	double shift_x = Ctrl->Y.shift_x;
	double shift_y = Ctrl->Y.shift_y;
	double shift_lon = 0.0;
	double shift_lat = 0.0;
	bool first_in_stays = Ctrl->U.first_in_stays;
	bool check_time = Ctrl->U.check_time;
	double timediff = Ctrl->U.active ? Ctrl->U.timediff : 300.0;
	double tension = Ctrl->T.active ? Ctrl->T.tension : Ctrl->C.tension;
	double boundsfactor = Ctrl->R.boundsfactor;
	bool bathy_in_feet = Ctrl->Q.active;
	bool more = Ctrl->M.more;
	bool use_NaN = Ctrl->N.active;
	int grdrasterid = Ctrl->K.grdrasterid;
	char projection_pars[MB_PATH_MAXLINE];
	strcpy(projection_pars, Ctrl->J.projection_pars);
	bool projection_pars_f = Ctrl->J.active;
	char filelist[MB_PATH_MAXLINE];
	strcpy(filelist, Ctrl->I.inputfile);
	char backgroundfile[MB_PATH_MAXLINE];
	strcpy(backgroundfile, Ctrl->K.backgroundfile);
	int gridkind = Ctrl->G.gridkind;
	double minormax_weighted_mean_threshold = Ctrl->F.threshold;
	int grid_mode = Ctrl->F.grid_mode;
	bool set_spacing = Ctrl->E.set_spacing;
	char units[MB_PATH_MAXLINE];
	strcpy(units, Ctrl->E.active ? Ctrl->E.units : "");
	double dx_set = Ctrl->E.dx_set;
	double dy_set = Ctrl->E.dy_set;
	bool spacing_priority = Ctrl->E.spacing_priority;
	bool set_dimensions = (Ctrl->D.active && Ctrl->D.xdim > 0 && Ctrl->D.ydim > 0);
	int clipmode = Ctrl->C.clipmode;
	if (Ctrl->L.active) lonflip = Ctrl->L.lonflip;
	if (Ctrl->P.active) pings = Ctrl->P.pings;
	if (Ctrl->S.active) speedmin = Ctrl->S.speedmin;

	if (verbose >= 2) {
		GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  Program <%s>\n", THIS_MODULE_NAME);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  MB-system Version %s\n", MB_VERSION);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  Control Parameters:\n");
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       verbose:              %d\n", verbose);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       pings:                %d\n", pings);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       lonflip:              %d\n", lonflip);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       speedmin:             %f\n", speedmin);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       timegap:              %f\n", timegap);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       file list:            %s\n", filelist);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       output file root:     %s\n", fileroot);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       grid x dimension:     %d\n", xdim);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       grid y dimension:     %d\n", ydim);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       grid bounds[0..3]:    %f %f %f %f\n", gbnd[0], gbnd[1], gbnd[2], gbnd[3]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       boundsfactor:         %f\n", boundsfactor);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       clipmode:             %d\n", clipmode);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       clip:                 %d\n", clip);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       tension:              %f\n", tension);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       grdraster background: %d\n", grdrasterid);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       backgroundfile:       %s\n", backgroundfile);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       more:                 %d\n", more);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       use_NaN:              %d\n", use_NaN);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       grid_mode:            %d\n", grid_mode);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       data type:            %d\n", datatype);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       grid format:          %d\n", gridkind);
		if (gridkind == MBGRID_GMTGRD)
			GMT_Report(API, GMT_MSG_NORMAL, "dbg2       gmt grid format id:   %s\n", gridkindstring);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       scale:                %f\n", scale);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       timediff:             %f\n", timediff);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       setborder:            %d\n", setborder);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       border:               %f\n", border);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       extend:               %f\n", extend);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       shift_mode:           %d\n", shift_mode);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       shift_x:              %f\n", shift_x);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       shift_y:              %f\n", shift_y);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       bathy_in_feet:        %d\n", bathy_in_feet);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       projection_pars:      %s\n", projection_pars);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       projection_pars_f:    %d\n", projection_pars_f);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       projection_id:        %s\n", projection_id);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       minormax_weighted_mean_threshold: %f\n", minormax_weighted_mean_threshold);
	}

	if (verbose == 1) {
		GMT_Report(API, GMT_MSG_NORMAL, "\nProgram %s\n", THIS_MODULE_NAME);
		GMT_Report(API, GMT_MSG_NORMAL, "MB-system Version %s\n", MB_VERSION);
	}

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
	bool file_in_bounds = false;
	void *mbio_ptr = NULL;
	struct mb_io_struct *mb_io_ptr = NULL;
	int topo_type;

	/* mbgrid control variables */
	void *datalist;
	double file_weight;
	int look_processed = MB_DATALIST_LOOK_UNSET;
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
	char ofile[2 * MB_PATH_MAXLINE + 100] = "";
	char dfile[MB_PATH_MAXLINE] = "";
	char plot_cmd[8 * 1024] = "";
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
	char *beamflag = NULL;
	double *bath = NULL;
	double *bathlon = NULL;
	double *bathlat = NULL;
	double *amp = NULL;
	double *ss = NULL;
	double *sslon = NULL;
	double *sslat = NULL;
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
	double *grid = NULL;
	double *norm = NULL;
	double *sigma = NULL;
	double *firsttime = NULL;
	double *gridsmall = NULL;
	double *minormax = NULL;
#ifdef USESURFACE
	float *bxdata = NULL;
	float *bydata = NULL;
	float *bzdata = NULL;
	float *sxdata = NULL;
	float *sydata = NULL;
	float *szdata = NULL;
#else
	float *bdata = NULL;
	float *sdata = NULL;
	float *work1 = NULL;
	int *work2 = NULL;
	bool *work3 = NULL;
#endif
	double bdata_origin_x, bdata_origin_y;
	float *output = NULL;
	float *sgrid = NULL;
	int *cnt = NULL;
	int *num = NULL;
	double **data = NULL;
	double *value = NULL;
	int ndata, ndatafile, nbackground = 0;
	double zmin, zmax, zclip;
	int nmax;
	double smin, smax;
	int nbinset, nbinzero, nbinspline, nbinbackground;

	/* projected grid parameters */
	void *pjptr = NULL;
	double deglontokm, deglattokm;
	double mtodeglon, mtodeglat;

	/* output char strings */
	char xlabel[MB_PATH_MAXLINE + 20] = "";
	char ylabel[MB_PATH_MAXLINE + 20] = "";
	char zlabel[MB_PATH_MAXLINE + 20] = "";
	char title[MB_PATH_MAXLINE] = "";
	char nlabel[MB_PATH_MAXLINE] = "";
	char sdlabel[MB_PATH_MAXLINE] = "";

	/* other variables */
	FILE *dfp = NULL;
	FILE *rfp = NULL;
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
	int use_weight;

	int gxdim = 0;
	int gydim = 0;

	bool time_ok = true;
	bool region_ok = false;
	bool footprint_ok = false;

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
		GMT_Report(API, GMT_MSG_NORMAL, "\nGrid bounds not properly specified:\n\t%f %f %f %f\n", gbnd[0], gbnd[1], gbnd[2], gbnd[3]);
		GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", THIS_MODULE_NAME);
		Return(MB_ERROR_BAD_PARAMETER);
	}

	/* footprint option only for bathymetry */
	if ((grid_mode == MBGRID_WEIGHTED_FOOTPRINT_SLOPE || grid_mode == MBGRID_WEIGHTED_FOOTPRINT) &&
		(datatype != MBGRID_DATA_TOPOGRAPHY && datatype != MBGRID_DATA_BATHYMETRY)) {
		grid_mode = MBGRID_WEIGHTED_MEAN;
	}

	/* more option not available with minimum or maximum filter algorithms */
	if (more && (grid_mode == MBGRID_MINIMUM_FILTER || grid_mode == MBGRID_MAXIMUM_FILTER))
		more = false;

	/* NaN cannot be used for ASCII grids */
	if (use_NaN && (gridkind == MBGRID_ASCII || gridkind == MBGRID_ARCASCII))
		use_NaN = false;

	/* define NaN in case it's needed */
	if (use_NaN) {
		outclipvalue = (float)NAN;
	}

	bool use_projection = false;

	/* deal with projected gridding */
	if (projection_pars_f) {

		/* check for UTM with undefined zone */
		if (strcmp(projection_pars, "UTM") == 0 || strcmp(projection_pars, "U") == 0 ||
			strcmp(projection_pars, "utm") == 0 || strcmp(projection_pars, "u") == 0) {
			double reference_lon = 0.5 * (gbnd[0] + gbnd[1]);
			if (reference_lon < 180.0) reference_lon += 360.0;
			if (reference_lon >= 180.0) reference_lon -= 360.0;
			const int utm_zone = (int)(((reference_lon + 183.0) / 6.0) + 0.5);
			double reference_lat = 0.5 * (gbnd[2] + gbnd[3]);
			if (reference_lat >= 0.0)
				snprintf(projection_id, sizeof(projection_id), "UTM%2.2dN", utm_zone);
			else
				snprintf(projection_id, sizeof(projection_id), "UTM%2.2dS", utm_zone);
		} else if (strncmp(projection_pars, "LTM", 3) == 0 || strncmp(projection_pars, "ltm", 3) == 0 ||
				   strcmp(projection_pars, "L") == 0 || strcmp(projection_pars, "l") == 0) {
			double reference_lon;
			double reference_lat;
			if (sscanf(projection_pars, "LTM%lf/%lf", &reference_lon, &reference_lat) == 2 ||
				sscanf(projection_pars, "ltm%lf/%lf", &reference_lon, &reference_lat) == 2) {
				strncpy(projection_id, projection_pars, sizeof(projection_id));
			} else {
				reference_lon = 0.5 * (gbnd[0] + gbnd[1]);
				reference_lat = 0.5 * (gbnd[2] + gbnd[3]);
				snprintf(projection_id, sizeof(projection_id), "LTM%.5f/%.5f", reference_lon, reference_lat);
			}
		} else
			strcpy(projection_id, projection_pars);

		use_projection = true;
		const int proj_status = mb_proj_init(verbose, projection_id, &pjptr, &error);

		if (proj_status != MB_SUCCESS) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nOutput projection %s not found in database\n", projection_id);
			GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", THIS_MODULE_NAME);
			mb_memory_clear(verbose, &memclear_error);
			Return(MB_ERROR_BAD_PARAMETER);
		}

		/* translate lon lat bounds from UTM if required */
		if (gbnd[0] < -360.0 || gbnd[0] > 360.0 || gbnd[1] < -360.0 || gbnd[1] > 360.0 ||
			gbnd[2] < -90.0 || gbnd[2] > 90.0 || gbnd[3] < -90.0 || gbnd[3] > 90.0) {
			xx = gbnd[0]; yy = gbnd[2];
			mb_proj_inverse(verbose, pjptr, xx, yy, &xlon, &ylat, &error);
			mb_apply_lonflip(verbose, lonflip, &xlon);
			obnd[0] = xlon; obnd[1] = xlon; obnd[2] = ylat; obnd[3] = ylat;
			xx = gbnd[1]; yy = gbnd[2];
			mb_proj_inverse(verbose, pjptr, xx, yy, &xlon, &ylat, &error);
			mb_apply_lonflip(verbose, lonflip, &xlon);
			obnd[0] = MIN(obnd[0], xlon); obnd[1] = MAX(obnd[1], xlon);
			obnd[2] = MIN(obnd[2], ylat); obnd[3] = MAX(obnd[3], ylat);
			xx = gbnd[0]; yy = gbnd[3];
			mb_proj_inverse(verbose, pjptr, xx, yy, &xlon, &ylat, &error);
			mb_apply_lonflip(verbose, lonflip, &xlon);
			obnd[0] = MIN(obnd[0], xlon); obnd[1] = MAX(obnd[1], xlon);
			obnd[2] = MIN(obnd[2], ylat); obnd[3] = MAX(obnd[3], ylat);
			xx = gbnd[1]; yy = gbnd[3];
			mb_proj_inverse(verbose, pjptr, xx, yy, &xlon, &ylat, &error);
			mb_apply_lonflip(verbose, lonflip, &xlon);
			obnd[0] = MIN(obnd[0], xlon); obnd[1] = MAX(obnd[1], xlon);
			obnd[2] = MIN(obnd[2], ylat); obnd[3] = MAX(obnd[3], ylat);
		} else {
			obnd[0] = gbnd[0]; obnd[1] = gbnd[1]; obnd[2] = gbnd[2]; obnd[3] = gbnd[3];
			xlon = obnd[0]; ylat = obnd[2];
			mb_proj_forward(verbose, pjptr, xlon, ylat, &xx, &yy, &error);
			gbnd[0] = xx; gbnd[1] = xx; gbnd[2] = yy; gbnd[3] = yy;
			xlon = obnd[1]; ylat = obnd[2];
			mb_proj_forward(verbose, pjptr, xlon, ylat, &xx, &yy, &error);
			gbnd[0] = MIN(gbnd[0], xx); gbnd[1] = MAX(gbnd[1], xx);
			gbnd[2] = MIN(gbnd[2], yy); gbnd[3] = MAX(gbnd[3], yy);
			xlon = obnd[0]; ylat = obnd[3];
			mb_proj_forward(verbose, pjptr, xlon, ylat, &xx, &yy, &error);
			gbnd[0] = MIN(gbnd[0], xx); gbnd[1] = MAX(gbnd[1], xx);
			gbnd[2] = MIN(gbnd[2], yy); gbnd[3] = MAX(gbnd[3], yy);
			xlon = obnd[1]; ylat = obnd[3];
			mb_proj_forward(verbose, pjptr, xlon, ylat, &xx, &yy, &error);
			gbnd[0] = MIN(gbnd[0], xx); gbnd[1] = MAX(gbnd[1], xx);
			gbnd[2] = MIN(gbnd[2], yy); gbnd[3] = MAX(gbnd[3], yy);
		}

		mb_coor_scale(2, 0.5 * (obnd[2] + obnd[3]), &mtodeglon, &mtodeglat);
		deglontokm = 0.001 / mtodeglon;
		deglattokm = 0.001 / mtodeglat;
		if (shift_mode != MBGRID_SHIFT_NONE) {
			shift_lon = shift_x * mtodeglon;
			shift_lat = shift_y * mtodeglat;
		}

		if (set_spacing) {
			xdim = (int)lrint((gbnd[1] - gbnd[0]) / dx_set + 1);
			if (dy_set <= 0.0) dy_set = dx_set;
			ydim = (int)lrint((gbnd[3] - gbnd[2]) / dy_set + 1);
			if (spacing_priority) {
				gbnd[1] = gbnd[0] + dx_set * (xdim - 1);
				gbnd[3] = gbnd[2] + dy_set * (ydim - 1);
			}
			if (units[0] == 'M' || units[0] == 'm') strcpy(units, "meters");
			else if (units[0] == 'K' || units[0] == 'k') strcpy(units, "km");
			else if (units[0] == 'F' || units[0] == 'f') strcpy(units, "feet");
			else if (strncmp(units, "arcmin", 6) == 0) { dx_set /= 60.0; dy_set /= 60.0; strcpy(units, "degrees"); }
			else if (strncmp(units, "arcsec", 6) == 0) { dx_set /= 3600.0; dy_set /= 3600.0; strcpy(units, "degrees"); }
			else strcpy(units, "unknown");
		}
	} else {
		mb_coor_scale(verbose, 0.5 * (gbnd[2] + gbnd[3]), &mtodeglon, &mtodeglat);
		deglontokm = 0.001 / mtodeglon;
		deglattokm = 0.001 / mtodeglat;
		if (shift_mode != MBGRID_SHIFT_NONE) {
			shift_lon = shift_x * mtodeglon;
			shift_lat = shift_y * mtodeglat;
		}

		if (set_spacing && (units[0] == 'M' || units[0] == 'm')) {
			xdim = (int)lrint((gbnd[1] - gbnd[0]) / (mtodeglon * dx_set) + 1);
			if (dy_set <= 0.0) dy_set = mtodeglon * dx_set / mtodeglat;
			ydim = (int)lrint((gbnd[3] - gbnd[2]) / (mtodeglat * dy_set) + 1);
			if (spacing_priority) {
				gbnd[1] = gbnd[0] + mtodeglon * dx_set * (xdim - 1);
				gbnd[3] = gbnd[2] + mtodeglat * dy_set * (ydim - 1);
			}
			strcpy(units, "meters");
		} else if (set_spacing && (units[0] == 'K' || units[0] == 'k')) {
			xdim = (int)lrint((gbnd[1] - gbnd[0]) * deglontokm / dx_set + 1);
			if (dy_set <= 0.0) dy_set = deglattokm * dx_set / deglontokm;
			ydim = (int)lrint((gbnd[3] - gbnd[2]) * deglattokm / dy_set + 1);
			if (spacing_priority) {
				gbnd[1] = gbnd[0] + dx_set * (xdim - 1) / deglontokm;
				gbnd[3] = gbnd[2] + dy_set * (ydim - 1) / deglattokm;
			}
			strcpy(units, "km");
		} else if (set_spacing && (units[0] == 'F' || units[0] == 'f')) {
			xdim = (int)lrint((gbnd[1] - gbnd[0]) / (mtodeglon * 0.3048 * dx_set) + 1);
			if (dy_set <= 0.0) dy_set = mtodeglon * dx_set / mtodeglat;
			ydim = (int)lrint((gbnd[3] - gbnd[2]) / (mtodeglat * 0.3048 * dy_set) + 1);
			if (spacing_priority) {
				gbnd[1] = gbnd[0] + mtodeglon * 0.3048 * dx_set * (xdim - 1);
				gbnd[3] = gbnd[2] + mtodeglat * 0.3048 * dy_set * (ydim - 1);
			}
			strcpy(units, "feet");
		} else if (set_spacing) {
			if (strncmp(units, "arcmin", 6) == 0) { dx_set /= 60.0; dy_set /= 60.0; strcpy(units, "degrees"); }
			else if (strncmp(units, "arcsec", 6) == 0) { dx_set /= 3600.0; dy_set /= 3600.0; strcpy(units, "degrees"); }
			else strcpy(units, "degrees");
			xdim = (int)lrint((gbnd[1] - gbnd[0]) / dx_set + 1);
			if (dy_set <= 0.0) dy_set = dx_set;
			ydim = (int)lrint((gbnd[3] - gbnd[2]) / dy_set + 1);
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
	offx = 0; offy = 0;
	if (extend > 0.0) { offx = (int)(extend * xdim); offy = (int)(extend * ydim); }
	xtradim = (int)scale + 2;
	gxdim = xdim + 2 * offx;
	gydim = ydim + 2 * offy;
	wbnd[0] = gbnd[0] - offx * dx;
	wbnd[1] = gbnd[1] + offx * dx;
	wbnd[2] = gbnd[2] - offy * dy;
	wbnd[3] = gbnd[3] + offy * dy;
	if (datatype == MBGRID_DATA_TOPOGRAPHY) topofactor = -1.0; else topofactor = 1.0;
	if (bathy_in_feet && (datatype == MBGRID_DATA_TOPOGRAPHY || datatype == MBGRID_DATA_BATHYMETRY))
		topofactor /= 0.3048;

	if (gridkind == MBGRID_ARCASCII && fabs(dx - dy) > MBGRID_TINY) {
		GMT_Report(API, GMT_MSG_NORMAL, "\nArc Ascii grid output (-G4) requires square cells, but grid intervals dx:%f dy:%f differ...\n", dx, dy);
		GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", THIS_MODULE_NAME);
		Return(MB_ERROR_BAD_PARAMETER);
	}

	/* get data input bounds in lon lat */
	if (!use_projection) {
		bounds[0] = wbnd[0]; bounds[1] = wbnd[1]; bounds[2] = wbnd[2]; bounds[3] = wbnd[3];
	} else {
		xx = wbnd[0] - 0.05 * (wbnd[1] - wbnd[0]); yy = wbnd[2] - 0.05 * (wbnd[3] - wbnd[2]);
		mb_proj_inverse(verbose, pjptr, xx, yy, &xlon, &ylat, &error);
		mb_apply_lonflip(verbose, lonflip, &xlon);
		bounds[0] = xlon; bounds[1] = xlon; bounds[2] = ylat; bounds[3] = ylat;
		xx = wbnd[1] + 0.05 * (wbnd[1] - wbnd[0]); yy = wbnd[2] - 0.05 * (wbnd[3] - wbnd[2]);
		mb_proj_inverse(verbose, pjptr, xx, yy, &xlon, &ylat, &error);
		mb_apply_lonflip(verbose, lonflip, &xlon);
		bounds[0] = MIN(bounds[0], xlon); bounds[1] = MAX(bounds[1], xlon);
		bounds[2] = MIN(bounds[2], ylat); bounds[3] = MAX(bounds[3], ylat);
		xx = wbnd[0] - 0.05 * (wbnd[1] - wbnd[0]); yy = wbnd[3] + 0.05 * (wbnd[3] - wbnd[2]);
		mb_proj_inverse(verbose, pjptr, xx, yy, &xlon, &ylat, &error);
		mb_apply_lonflip(verbose, lonflip, &xlon);
		bounds[0] = MIN(bounds[0], xlon); bounds[1] = MAX(bounds[1], xlon);
		bounds[2] = MIN(bounds[2], ylat); bounds[3] = MAX(bounds[3], ylat);
		xx = wbnd[1] + 0.05 * (wbnd[1] - wbnd[0]); yy = wbnd[3] + 0.05 * (wbnd[3] - wbnd[2]);
		mb_proj_inverse(verbose, pjptr, xx, yy, &xlon, &ylat, &error);
		mb_apply_lonflip(verbose, lonflip, &xlon);
		bounds[0] = MIN(bounds[0], xlon); bounds[1] = MAX(bounds[1], xlon);
		bounds[2] = MIN(bounds[2], ylat); bounds[3] = MAX(bounds[3], ylat);
	}

	/* extend the bounds slightly */
	xx = MIN(0.05 * (bounds[1] - bounds[0]), 0.1);
	yy = MIN(0.05 * (bounds[3] - bounds[2]), 0.1);
	bounds[0] -= xx; bounds[1] += xx; bounds[2] -= yy; bounds[3] += yy;

	/* figure out lonflip for data bounds */
	if (bounds[0] < -180.0) lonflip = -1;
	else if (bounds[1] > 180.0) lonflip = 1;
	else if (lonflip == -1 && bounds[1] > 0.0) lonflip = 0;
	else if (lonflip == 1 && bounds[0] < 0.0) lonflip = 0;

	/* check interpolation parameters */
	if ((clipmode == MBGRID_INTERP_GAP || clipmode == MBGRID_INTERP_NEAR) && clip > xdim && clip > ydim)
		clipmode = MBGRID_INTERP_ALL;
	if (clipmode == MBGRID_INTERP_ALL)
		clip = MAX(xdim, ydim);

	bdata_origin_x = 0.5 * (wbnd[0] + wbnd[1]);
	bdata_origin_y = 0.5 * (wbnd[2] + wbnd[3]);

	/* set plot label strings */
	if (use_projection) {
		snprintf(xlabel, sizeof(xlabel), "Easting (%s)", units);
		snprintf(ylabel, sizeof(ylabel), "Northing (%s)", units);
	} else {
		strcpy(xlabel, "Longitude");
		strcpy(ylabel, "Latitude");
	}
	if (datatype == MBGRID_DATA_BATHYMETRY) {
		strcpy(zlabel, bathy_in_feet ? "Depth (ft)" : "Depth (m)");
		strcpy(nlabel, "Number of Depth Data Points");
		strcpy(sdlabel, bathy_in_feet ? "Depth Standard Deviation (ft)" : "Depth Standard Deviation (m)");
		strcpy(title, "Bathymetry Grid");
	} else if (datatype == MBGRID_DATA_TOPOGRAPHY) {
		strcpy(zlabel, bathy_in_feet ? "Topography (ft)" : "Topography (m)");
		strcpy(nlabel, "Number of Topography Data Points");
		strcpy(sdlabel, bathy_in_feet ? "Topography Standard Deviation (ft)" : "Topography Standard Deviation (m)");
		strcpy(title, "Topography Grid");
	} else if (datatype == MBGRID_DATA_AMPLITUDE) {
		strcpy(zlabel, "Amplitude");
		strcpy(nlabel, "Number of Amplitude Data Points");
		strcpy(sdlabel, "Amplitude Standard Deviation (m)");
		strcpy(title, "Amplitude Grid");
	} else if (datatype == MBGRID_DATA_SIDESCAN) {
		strcpy(zlabel, "Sidescan");
		strcpy(nlabel, "Number of Sidescan Data Points");
		strcpy(sdlabel, "Sidescan Standard Deviation (m)");
		strcpy(title, "Sidescan Grid");
	}

	/* output info */
	if (verbose >= 0) {
		GMT_Report(API, GMT_MSG_NORMAL, "\nMBGRID Parameters:\n");
		GMT_Report(API, GMT_MSG_NORMAL, "List of input files: %s\n", filelist);
		GMT_Report(API, GMT_MSG_NORMAL, "Output fileroot:     %s\n", fileroot);
		GMT_Report(API, GMT_MSG_NORMAL, "Input Data Type:     ");
		if (datatype == MBGRID_DATA_BATHYMETRY) {
			GMT_Report(API, GMT_MSG_NORMAL, "Bathymetry\n");
			if (bathy_in_feet) GMT_Report(API, GMT_MSG_NORMAL, "Bathymetry gridded in feet\n");
		} else if (datatype == MBGRID_DATA_TOPOGRAPHY) {
			GMT_Report(API, GMT_MSG_NORMAL, "Topography\n");
			if (bathy_in_feet) GMT_Report(API, GMT_MSG_NORMAL, "Topography gridded in feet\n");
		} else if (datatype == MBGRID_DATA_AMPLITUDE) GMT_Report(API, GMT_MSG_NORMAL, "Amplitude\n");
		else if (datatype == MBGRID_DATA_SIDESCAN) GMT_Report(API, GMT_MSG_NORMAL, "Sidescan\n");
		else GMT_Report(API, GMT_MSG_NORMAL, "Unknown?\n");
		GMT_Report(API, GMT_MSG_NORMAL, "Gridding algorithm:  ");
		if (grid_mode == MBGRID_MEDIAN_FILTER) GMT_Report(API, GMT_MSG_NORMAL, "Median Filter\n");
		else if (grid_mode == MBGRID_MINIMUM_FILTER) GMT_Report(API, GMT_MSG_NORMAL, "Minimum Filter\n");
		else if (grid_mode == MBGRID_MAXIMUM_FILTER) GMT_Report(API,GMT_MSG_NORMAL, "Maximum Filter\n");
		else if (grid_mode == MBGRID_WEIGHTED_FOOTPRINT_SLOPE) GMT_Report(API, GMT_MSG_NORMAL, "Footprint-Slope Weighted Mean\n");
		else if (grid_mode == MBGRID_WEIGHTED_FOOTPRINT) GMT_Report(API, GMT_MSG_NORMAL, "Footprint Weighted Mean\n");
		else if (grid_mode == MBGRID_MINIMUM_WEIGHTED_MEAN) GMT_Report(API, GMT_MSG_NORMAL, "Minimum Gaussian Weighted Mean\n");
		else if (grid_mode == MBGRID_MAXIMUM_WEIGHTED_MEAN) GMT_Report(API, GMT_MSG_NORMAL, "Maximum Gaussian Weighted Mean\n");
		else GMT_Report(API, GMT_MSG_NORMAL, "Gaussian Weighted Mean\n");
		GMT_Report(API, GMT_MSG_NORMAL, "Grid projection: %s\n", projection_id);
		if (use_projection) GMT_Report(API, GMT_MSG_NORMAL, "Projection ID: %s\n", projection_id);
		GMT_Report(API, GMT_MSG_NORMAL, "Grid dimensions: %d %d\n", xdim, ydim);
		GMT_Report(API, GMT_MSG_NORMAL, "Grid bounds:\n");
		if (use_projection) {
			GMT_Report(API, GMT_MSG_NORMAL, "  Eastings:  %9.4f %9.4f\n", gbnd[0], gbnd[1]);
			GMT_Report(API, GMT_MSG_NORMAL, "  Northings: %9.4f %9.4f\n", gbnd[2], gbnd[3]);
			GMT_Report(API, GMT_MSG_NORMAL, "  Longitude: %9.4f %9.4f\n", obnd[0], obnd[1]);
			GMT_Report(API, GMT_MSG_NORMAL, "  Latitude:  %9.4f %9.4f\n", obnd[2], obnd[3]);
		} else {
			GMT_Report(API, GMT_MSG_NORMAL, "  Longitude: %9.4f %9.4f\n", gbnd[0], gbnd[1]);
			GMT_Report(API, GMT_MSG_NORMAL, "  Latitude:  %9.4f %9.4f\n", gbnd[2], gbnd[3]);
		}
		if (boundsfactor > 1.0)
			GMT_Report(API, GMT_MSG_NORMAL, "  Grid bounds correspond to %f times actual data coverage\n", boundsfactor);
		GMT_Report(API, GMT_MSG_NORMAL, "Working grid dimensions: %d %d\n", gxdim, gydim);
		if (use_projection) {
			GMT_Report(API, GMT_MSG_NORMAL, "Working Grid bounds:\n");
			GMT_Report(API, GMT_MSG_NORMAL, "  Eastings:  %9.4f %9.4f\n", wbnd[0], wbnd[1]);
			GMT_Report(API, GMT_MSG_NORMAL, "  Northings: %9.4f %9.4f\n", wbnd[2], wbnd[3]);
			GMT_Report(API, GMT_MSG_NORMAL, "Easting interval:  %f %s\n", dx, units);
			GMT_Report(API, GMT_MSG_NORMAL, "Northing interval: %f %s\n", dy, units);
			if (set_spacing) {
				GMT_Report(API, GMT_MSG_NORMAL, "Specified Easting interval:  %f %s\n", dx_set, units);
				GMT_Report(API, GMT_MSG_NORMAL, "Specified Northing interval: %f %s\n", dy_set, units);
			}
		} else {
			GMT_Report(API, GMT_MSG_NORMAL, "Working Grid bounds:\n");
			GMT_Report(API, GMT_MSG_NORMAL, "  Longitude: %9.4f %9.4f\n", wbnd[0], wbnd[1]);
			GMT_Report(API, GMT_MSG_NORMAL, "  Latitude:  %9.4f %9.4f\n", wbnd[2], wbnd[3]);
			GMT_Report(API, GMT_MSG_NORMAL, "Longitude interval: %f degrees or %f m\n", dx, 1000 * dx * deglontokm);
			GMT_Report(API, GMT_MSG_NORMAL, "Latitude interval:  %f degrees or %f m\n", dy, 1000 * dy * deglattokm);
			if (set_spacing) {
				GMT_Report(API, GMT_MSG_NORMAL, "Specified Longitude interval: %f %s\n", dx_set, units);
				GMT_Report(API, GMT_MSG_NORMAL, "Specified Latitude interval:  %f %s\n", dy_set, units);
			}
		}
		if (shift_mode == MBGRID_SHIFT_BOUNDS && use_projection) {
			GMT_Report(API, GMT_MSG_NORMAL, "Grid shift (applied to the bounds of output grids):\n");
			GMT_Report(API, GMT_MSG_NORMAL, "  East shift:   %9.4f m\n", shift_x);
			GMT_Report(API, GMT_MSG_NORMAL, "  North shift:  %9.4f m\n", shift_y);
		} else if (shift_mode == MBGRID_SHIFT_BOUNDS) {
			GMT_Report(API, GMT_MSG_NORMAL, "Grid shift (applied to the bounds of output grids):\n");
			GMT_Report(API, GMT_MSG_NORMAL, "  Longitude interval: %g degrees or %f m\n", shift_lon, shift_x);
			GMT_Report(API, GMT_MSG_NORMAL, "  Latitude interval:  %g degrees or %f m\n", shift_lat, shift_y);
		} else if (shift_mode == MBGRID_SHIFT_DATA) {
			GMT_Report(API, GMT_MSG_NORMAL, "Data shift (applied to the position of input data):\n");
			GMT_Report(API, GMT_MSG_NORMAL, "  Longitude interval: %g degrees or %f m\n", shift_lon, shift_x);
			GMT_Report(API, GMT_MSG_NORMAL, "  Latitude interval:  %g degrees or %f m\n", shift_lat, shift_y);
		}
		GMT_Report(API, GMT_MSG_NORMAL, "Input data bounds:\n");
		GMT_Report(API, GMT_MSG_NORMAL, "  Longitude: %9.4f %9.4f\n", bounds[0], bounds[1]);
		GMT_Report(API, GMT_MSG_NORMAL, "  Latitude:  %9.4f %9.4f\n", bounds[2], bounds[3]);
		if (grid_mode == MBGRID_WEIGHTED_MEAN)
			GMT_Report(API, GMT_MSG_NORMAL, "Gaussian filter 1/e length: %f grid intervals\n", scale);
		if (grid_mode == MBGRID_WEIGHTED_FOOTPRINT_SLOPE || grid_mode == MBGRID_WEIGHTED_FOOTPRINT)
			GMT_Report(API, GMT_MSG_NORMAL, "Footprint 1/e distance: %f times footprint\n", scale);
		if (grid_mode == MBGRID_MINIMUM_WEIGHTED_MEAN)
			GMT_Report(API, GMT_MSG_NORMAL, "Minimum filter threshold for Minimum Weighted Mean: %f\n", minormax_weighted_mean_threshold);
		if (check_time && !first_in_stays) GMT_Report(API, GMT_MSG_NORMAL, "Swath overlap handling:       Last data used\n");
		if (check_time && first_in_stays) GMT_Report(API, GMT_MSG_NORMAL, "Swath overlap handling:       First data used\n");
		if (check_time) GMT_Report(API, GMT_MSG_NORMAL, "Swath overlap time threshold: %f minutes\n", timediff / 60.);
		if (clipmode == MBGRID_INTERP_NONE) GMT_Report(API, GMT_MSG_NORMAL, "Spline interpolation not applied\n");
		else if (clipmode == MBGRID_INTERP_GAP) {
			GMT_Report(API, GMT_MSG_NORMAL, "Spline interpolation applied to fill data gaps\n");
			GMT_Report(API, GMT_MSG_NORMAL, "Spline interpolation clipping dimension: %d\n", clip);
			GMT_Report(API, GMT_MSG_NORMAL, "Spline tension (range 0.0 to infinity): %f\n", tension);
		} else if (clipmode == MBGRID_INTERP_NEAR) {
			GMT_Report(API, GMT_MSG_NORMAL, "Spline interpolation applied near data\n");
			GMT_Report(API, GMT_MSG_NORMAL, "Spline interpolation clipping dimension: %d\n", clip);
			GMT_Report(API, GMT_MSG_NORMAL, "Spline tension (range 0.0 to infinity): %f\n", tension);
		} else if (clipmode == MBGRID_INTERP_ALL) {
			GMT_Report(API, GMT_MSG_NORMAL, "Spline interpolation applied to fill entire grid\n");
			GMT_Report(API, GMT_MSG_NORMAL, "Spline tension (range 0.0 to infinity): %f\n", tension);
		}
		if (grdrasterid == 0) GMT_Report(API, GMT_MSG_NORMAL, "Background not applied\n");
		else if (grdrasterid < 0) GMT_Report(API, GMT_MSG_NORMAL, "Background obtained using grd2xyz from GMT grid file: %s\n", backgroundfile);
		else GMT_Report(API, GMT_MSG_NORMAL, "Background obtained using grdraster from dataset: %d\n", grdrasterid);
		if (gridkind == MBGRID_ASCII) GMT_Report(API, GMT_MSG_NORMAL, "Grid format %d:  ascii table\n", gridkind);
		else if (gridkind == MBGRID_CDFGRD) GMT_Report(API, GMT_MSG_NORMAL, "Grid format %d:  GMT version 2 grd (netCDF)\n", gridkind);
		else if (gridkind == MBGRID_OLDGRD) GMT_Report(API, GMT_MSG_NORMAL, "Grid format %d:  GMT version 1 grd (binary)\n", gridkind);
		else if (gridkind == MBGRID_ARCASCII) GMT_Report(API, GMT_MSG_NORMAL, "Grid format %d:  Arc/Info ascii table\n", gridkind);
		else if (gridkind == MBGRID_GMTGRD) {
			GMT_Report(API, GMT_MSG_NORMAL, "Grid format %d:  GMT grid\n", gridkind);
			if (strlen(gridkindstring) > 0) GMT_Report(API, GMT_MSG_NORMAL, "GMT Grid ID:     %s\n", gridkindstring);
		}
		if (use_NaN) GMT_Report(API, GMT_MSG_NORMAL, "NaN values used to flag regions with no data\n");
		else GMT_Report(API, GMT_MSG_NORMAL, "Real value of %f used to flag regions with no data\n", outclipvalue);
		if (more) GMT_Report(API, GMT_MSG_NORMAL, "Data density and sigma grids also created\n");
		GMT_Report(API, GMT_MSG_NORMAL, "MBIO parameters:\n");
		GMT_Report(API, GMT_MSG_NORMAL, "  Ping averaging:       %d\n", pings);
		GMT_Report(API, GMT_MSG_NORMAL, "  Longitude flipping:   %d\n", lonflip);
		GMT_Report(API, GMT_MSG_NORMAL, "  Speed minimum:      %4.1f km/hr\n", speedmin);
	}
	if (verbose > 0) GMT_Report(API, GMT_MSG_NORMAL, "\n");

	/* if grdrasterid set extract background data and interpolate later onto internal grid */
	if (grdrasterid != 0) {
		if (grdrasterid > 0)
			GMT_Report(API, GMT_MSG_NORMAL, "\nExtracting background from grdraster dataset %d...\n", grdrasterid);
		else
			GMT_Report(API, GMT_MSG_NORMAL, "\nExtracting background from grid file %s...\n", backgroundfile);

		int nbackground_alloc = 2 * gxdim * gydim;

#ifdef USESURFACE
		status = mb_mallocd(verbose, __FILE__, __LINE__, nbackground_alloc * sizeof(float), (void **)&bxdata, &error);
		if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, nbackground_alloc * sizeof(float), (void **)&bydata, &error);
		if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, nbackground_alloc * sizeof(float), (void **)&bzdata, &error);
		if (error != MB_ERROR_NO_ERROR) {
			mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating background data array:\n%s\n", message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", THIS_MODULE_NAME);
			mb_memory_clear(verbose, &memclear_error);
			Return(MB_ERROR_MEMORY_FAIL);
		}
		memset((char *)bxdata, 0, nbackground_alloc * sizeof(float));
		memset((char *)bydata, 0, nbackground_alloc * sizeof(float));
		memset((char *)bzdata, 0, nbackground_alloc * sizeof(float));
#else
		status = mb_mallocd(verbose, __FILE__, __LINE__, 3 * nbackground_alloc * sizeof(float), (void **)&bdata, &error);
		if (error != MB_ERROR_NO_ERROR) {
			mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating background interpolation work arrays:\n%s\n", message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", THIS_MODULE_NAME);
			mb_memory_clear(verbose, &memclear_error);
			Return(MB_ERROR_MEMORY_FAIL);
		}
		memset((char *)bdata, 0, 3 * nbackground_alloc * sizeof(float));
#endif

		const int pid = (int)time(NULL);

		if (grdrasterid > 0) {
			snprintf(backgroundfile, sizeof(backgroundfile), "tmpgrdraster%d.grd", pid);
			snprintf(plot_cmd, sizeof(plot_cmd), "grdraster %d -R%f/%f/%f/%f -G%s", grdrasterid, bounds[0], bounds[1], bounds[2], bounds[3], backgroundfile);
			GMT_Report(API, GMT_MSG_NORMAL, "Executing: %s\n", plot_cmd);
			const int fork_status = system(plot_cmd);
			if (fork_status != 0) {
				GMT_Report(API, GMT_MSG_NORMAL, "\nExecution of command:\n\t%s\nby system() call failed....\nProgram <%s> Terminated\n", plot_cmd, THIS_MODULE_NAME);
				mb_memory_clear(verbose, &memclear_error);
				Return(MB_ERROR_BAD_PARAMETER);
			}
		}

		snprintf(plot_cmd, sizeof(plot_cmd), "gmt grdinfo %s", backgroundfile);
		char backgroundfileuse[MB_PATH_MAXLINE] = "";
		strcpy(backgroundfileuse, backgroundfile);
		if ((rfp = popen(plot_cmd, "r")) != NULL) {
			char plot_stdout[MB_COMMENT_MAXLINE];
			if (fgets(plot_stdout, MB_COMMENT_MAXLINE, rfp) == NULL) { /* ignore */ }
			if (fgets(plot_stdout, MB_COMMENT_MAXLINE, rfp) == NULL) { /* ignore */ }
			if (fgets(plot_stdout, MB_COMMENT_MAXLINE, rfp) == NULL) { /* ignore */ }
			if (fgets(plot_stdout, MB_COMMENT_MAXLINE, rfp) == NULL) { /* ignore */ }
			pclose(rfp);
			if (strncmp(plot_stdout, "Pixel node registration used", 28) == 0) {
				snprintf(backgroundfileuse, sizeof(backgroundfileuse), "tmpgrdsampleT%d.grd", pid);
				snprintf(plot_cmd, sizeof(plot_cmd), "grdsample %s -G%s -T", backgroundfile, backgroundfileuse);
				GMT_Report(API, GMT_MSG_NORMAL, "Executing: %s\n", plot_cmd);
				const int fork_status = system(plot_cmd);
				if (fork_status != 0) {
					GMT_Report(API, GMT_MSG_NORMAL, "\nExecution of command:\n\t%s\nby system() call failed....\nProgram <%s> Terminated\n", plot_cmd, THIS_MODULE_NAME);
					mb_memory_clear(verbose, &memclear_error);
					Return(MB_ERROR_BAD_PARAMETER);
				}
			}
		} else {
			GMT_Report(API, GMT_MSG_NORMAL, "\nBackground data not extracted as per -K option\n");
			if (grdrasterid > 0) {
				GMT_Report(API, GMT_MSG_NORMAL, "The program grdraster may not have been found\n");
				GMT_Report(API, GMT_MSG_NORMAL, "or the specified background dataset %d may not exist.\n", grdrasterid);
			} else {
				GMT_Report(API, GMT_MSG_NORMAL, "The specified background dataset %s may not exist.\n", backgroundfile);
			}
			GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", THIS_MODULE_NAME);
			error = MB_ERROR_BAD_PARAMETER;
			mb_memory_clear(verbose, &memclear_error);
			Return(MB_ERROR_BAD_PARAMETER);
		}

		if (use_projection)
			snprintf(plot_cmd, sizeof(plot_cmd), "gmt grdsample %s -Gtmpgrdsample%d.grd -R%.12f/%.12f/%.12f/%.12f -I%.12f/%.12f", backgroundfileuse, pid, bounds[0], bounds[1], bounds[2], bounds[3], dx * mtodeglon, dy * mtodeglat);
		else
			snprintf(plot_cmd, sizeof(plot_cmd), "gmt grdsample %s -Gtmpgrdsample%d.grd -R%.12f/%.12f/%.12f/%.12f -I%.12f/%.12f", backgroundfileuse, pid, bounds[0], bounds[1], bounds[2], bounds[3], dx, dy);
		GMT_Report(API, GMT_MSG_NORMAL, "Executing: %s\n", plot_cmd);
		int fork_status = system(plot_cmd);
		if (fork_status != 0) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nExecution of command:\n\t%s\nby system() call failed....\nProgram <%s> Terminated\n", plot_cmd, THIS_MODULE_NAME);
			error = MB_ERROR_BAD_PARAMETER;
			mb_memory_clear(verbose, &memclear_error);
			Return(MB_ERROR_BAD_PARAMETER);
		}

		if (use_projection)
			snprintf(plot_cmd, sizeof(plot_cmd), "gmt grd2xyz tmpgrdsample%d.grd -s -bo | gmt blockmean -bi -bo -C -R%f/%f/%f/%f -I%.12f/%.12f", pid, bounds[0], bounds[1], bounds[2], bounds[3], dx * mtodeglon, dy * mtodeglat);
		else
			snprintf(plot_cmd, sizeof(plot_cmd), "gmt grd2xyz tmpgrdsample%d.grd -s -bo | gmt blockmean -bi -bo -C -R%f/%f/%f/%f -I%.12f/%.12f", pid, bounds[0], bounds[1], bounds[2], bounds[3], dx, dy);
		GMT_Report(API, GMT_MSG_NORMAL, "Executing: %s\n", plot_cmd);
		if ((rfp = popen(plot_cmd, "r")) != NULL) {
			nbackground = 0;
			while (fread(&tlon, sizeof(double), 1, rfp) == 1) {
				if (fread(&tlat, sizeof(double), 1, rfp) != 1) break;
				if (fread(&tvalue, sizeof(double), 1, rfp) != 1) break;
				if (lonflip == -1 && tlon > 0.0) tlon -= 360.0;
				else if (lonflip == 0 && tlon < -180.0) tlon += 360.0;
				else if (lonflip == 0 && tlon > 180.0) tlon -= 360.0;
				else if (lonflip == 1 && tlon < 0.0) tlon += 360.0;
				if (use_projection) mb_proj_forward(verbose, pjptr, tlon, tlat, &tlon, &tlat, &error);
#ifdef USESURFACE
				if (nbackground >= nbackground_alloc) {
					nbackground_alloc += 10000;
					status = mb_reallocd(verbose, __FILE__, __LINE__, nbackground_alloc * sizeof(float), (void **)&bxdata, &error);
					if (status == MB_SUCCESS) status = mb_reallocd(verbose, __FILE__, __LINE__, nbackground_alloc * sizeof(float), (void **)&bydata, &error);
					if (status == MB_SUCCESS) status = mb_reallocd(verbose, __FILE__, __LINE__, nbackground_alloc * sizeof(float), (void **)&bzdata, &error);
					if (error != MB_ERROR_NO_ERROR) {
						mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
						GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error reallocating background data array:\n%s\n", message);
						mb_memory_clear(verbose, &memclear_error);
						Return(MB_ERROR_MEMORY_FAIL);
					}
				}
				bxdata[nbackground] = (float)(tlon - bdata_origin_x);
				bydata[nbackground] = (float)(tlat - bdata_origin_y);
				bzdata[nbackground] = (float)tvalue;
#else
				if (nbackground >= nbackground_alloc) {
					nbackground_alloc += 10000;
					status = mb_reallocd(verbose, __FILE__, __LINE__, 3 * nbackground_alloc * sizeof(float), (void **)&bdata, &error);
					if (error != MB_ERROR_NO_ERROR) {
						mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
						GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating background interpolation work arrays:\n%s\n", message);
						mb_memory_clear(verbose, &memclear_error);
						Return(MB_ERROR_MEMORY_FAIL);
					}
				}
				bdata[nbackground * 3] = (float)(tlon - bdata_origin_x);
				bdata[nbackground * 3 + 1] = (float)(tlat - bdata_origin_y);
				bdata[nbackground * 3 + 2] = (float)tvalue;
#endif
				nbackground++;
			}
			pclose(rfp);
		} else {
			GMT_Report(API, GMT_MSG_NORMAL, "\nBackground data not extracted as per -K option\n");
			GMT_Report(API, GMT_MSG_NORMAL, "The program grdraster may not have been found\n");
			GMT_Report(API, GMT_MSG_NORMAL, "or the specified background dataset %d may not exist.\n", grdrasterid);
			error = MB_ERROR_BAD_PARAMETER;
			mb_memory_clear(verbose, &memclear_error);
			Return(MB_ERROR_BAD_PARAMETER);
		}

		snprintf(plot_cmd, sizeof(plot_cmd), "rm tmpgrd*%d.grd", pid);
		GMT_Report(API, GMT_MSG_NORMAL, "Executing: %s\n", plot_cmd);
		fork_status = system(plot_cmd);
		if (fork_status != 0) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nExecution of command:\n\t%s\nby system() call failed....\nProgram <%s> Terminated\n", plot_cmd, THIS_MODULE_NAME);
			error = MB_ERROR_BAD_PARAMETER;
			mb_memory_clear(verbose, &memclear_error);
			Return(MB_ERROR_BAD_PARAMETER);
		}
	}

	/* allocate memory for grid arrays */
	status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(double), (void **)&grid, &error);
	if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(double), (void **)&sigma, &error);
	if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(double), (void **)&firsttime, &error);
	if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(int), (void **)&cnt, &error);
	if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(int), (void **)&num, &error);
	if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, xdim * ydim * sizeof(float), (void **)&output, &error);

	if (error != MB_ERROR_NO_ERROR) {
		mb_error(verbose, error, &message);
		GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
		GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", THIS_MODULE_NAME);
		mb_memory_clear(verbose, &memclear_error);
		Return(error);
	}

	/* open datalist file for list of all files that contribute to the grid */
	strcpy(dfile, fileroot);
	strcat(dfile, ".mb-1");
	if ((dfp = fopen(dfile, "w")) == NULL) {
		error = MB_ERROR_OPEN_FAIL;
		GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open datalist file: %s\n", dfile);
	}

	nbinset = 0;
	nbinzero = 0;
	nbinspline = 0;
	nbinbackground = 0;

/* -------------------------------------------------------------------------- */
	/***** do weighted footprint slope gridding *****/
	if (grid_mode == MBGRID_WEIGHTED_FOOTPRINT_SLOPE) {
		const double sdx = 2.0 * dx;
		const double sdy = 2.0 * dy;
		int sxdim = gxdim / 2;
		int sydim = gydim / 2;
		int sclip = MAX(gxdim, gydim);

		mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(double), (void **)&norm, &error);
		mb_mallocd(verbose, __FILE__, __LINE__, sxdim * sydim * sizeof(double), (void **)&gridsmall, &error);

		if (error != MB_ERROR_NO_ERROR) {
			mb_error(verbose, error, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
			mb_memory_clear(verbose, &memclear_error);
			Return(error);
		}

		for (int i = 0; i < sxdim; i++)
			for (int j = 0; j < sydim; j++) {
				kgrid = i * sydim + j;
				gridsmall[kgrid] = 0.0;
				cnt[kgrid] = 0;
			}

		GMT_Report(API, GMT_MSG_NORMAL, "\nDoing first pass to generate low resolution slope grid...\n");
		ndata = 0;
		if (mb_datalist_open(verbose, &datalist, filelist, look_processed, &error) != MB_SUCCESS) {
			error = MB_ERROR_OPEN_FAIL;
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open data list file: %s\n", filelist);
			mb_memory_clear(verbose, &memclear_error);
			Return(MB_ERROR_OPEN_FAIL);
		}
		while (mb_datalist_read3(verbose, datalist, &pstatus, path, ppath, &astatus, apath, dpath, &format, &file_weight, &error) == MB_SUCCESS) {
			ndatafile = 0;
			if (format > 0 && path[0] != '#') {
				if (pstatus == MB_PROCESSED_USE) strcpy(file, ppath); else strcpy(file, path);
				rformat = format;
				strcpy(rfile, file);
				status = mb_check_info(verbose, rfile, lonflip, bounds, &file_in_bounds, &error);
				if (status == MB_FAILURE) { file_in_bounds = true; status = MB_SUCCESS; error = MB_ERROR_NO_ERROR; }

				bool first = true;
				double dmin = 0.0, dmax = 0.0;
				if (file_in_bounds) {
					if (datatype == MBGRID_DATA_TOPOGRAPHY || datatype == MBGRID_DATA_BATHYMETRY)
						mb_get_fbt(verbose, rfile, &rformat, &error);

					if (mb_read_init_altnav(verbose, rfile, rformat, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, astatus, apath, &mbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
						mb_error(verbose, error, &message);
						GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error returned from function <mb_read_init_altnav>:\n%s\n", message);
						GMT_Report(API, GMT_MSG_NORMAL, "\nMultibeam File <%s> not initialized for reading\n", rfile);
						mb_memory_clear(verbose, &memclear_error);
						Return(error);
					}

					mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
					status = mb_sonartype(verbose, mbio_ptr, mb_io_ptr->store_data, &topo_type, &error);

					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlon, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlat, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslon, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslat, &error);

					if (error != MB_ERROR_NO_ERROR) {
						mb_error(verbose, error, &message);
						GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
						mb_memory_clear(verbose, &memclear_error);
						Return(error);
					}

					while (error <= MB_ERROR_NO_ERROR) {
						status = mb_read(verbose, mbio_ptr, &kind, &rpings, time_i, &time_d, &navlon, &navlat, &speed, &heading, &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp, bathlon, bathlat, ss, sslon, sslat, comment, &error);
						if (error == MB_ERROR_TIME_GAP) { error = MB_ERROR_NO_ERROR; status = MB_SUCCESS; }

						if (verbose >= 2) {
							GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  Ping read in program <%s>\n", THIS_MODULE_NAME);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       kind:           %d\n", kind);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       beams_bath:     %d\n", beams_bath);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       beams_amp:      %d\n", beams_amp);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       pixels_ss:      %d\n", pixels_ss);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       error:          %d\n", error);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       status:         %d\n", status);
						}

						if (shift_mode == MBGRID_SHIFT_DATA) {
							navlon += shift_lon; navlat += shift_lat;
							for (ib = 0; ib < beams_bath; ib++)
								if (mb_beam_ok(beamflag[ib])) { bathlon[ib] += shift_lon; bathlat[ib] += shift_lat; }
						}

						if ((datatype == MBGRID_DATA_BATHYMETRY || datatype == MBGRID_DATA_TOPOGRAPHY) && error == MB_ERROR_NO_ERROR) {
							if (use_projection) {
								mb_proj_forward(verbose, pjptr, navlon, navlat, &navlon, &navlat, &error);
								for (ib = 0; ib < beams_bath; ib++)
									if (mb_beam_ok(beamflag[ib]))
										mb_proj_forward(verbose, pjptr, bathlon[ib], bathlat[ib], &bathlon[ib], &bathlat[ib], &error);
							}

							for (ib = 0; ib < beams_bath; ib++)
								if (mb_beam_ok(beamflag[ib])) {
									ix = (int)((bathlon[ib] - wbnd[0] + dx) / sdx);
									iy = (int)((bathlat[ib] - wbnd[2] + dy) / sdy);
									if (ix >= 0 && ix < sxdim && iy >= 0 && iy < sydim) {
										kgrid = ix * sydim + iy;
										gridsmall[kgrid] += topofactor * bath[ib];
										cnt[kgrid]++;
										ndata++; ndatafile++;
										if (first) { first = false; dmin = topofactor * bath[ib]; dmax = topofactor * bath[ib]; }
										else { dmin = MIN(topofactor * bath[ib], dmin); dmax = MAX(topofactor * bath[ib], dmax); }
									}
								}
						}
					}
					mb_close(verbose, &mbio_ptr, &error);
					status = MB_SUCCESS; error = MB_ERROR_NO_ERROR;
				}
				if (verbose > 0 || file_in_bounds) {
					if (astatus == MB_ALTNAV_USE)
						GMT_Report(API, GMT_MSG_NORMAL, "%d data points processed in %s (minmax: %f %f) using nav from %s\n", ndatafile, rfile, dmin, dmax, apath);
					else
						GMT_Report(API, GMT_MSG_NORMAL, "%d data points processed in %s (minmax: %f %f)\n", ndatafile, rfile, dmin, dmax);
				}
				if (ndatafile > 0 && dfp != NULL) {
					if (pstatus == MB_PROCESSED_USE && astatus == MB_ALTNAV_USE)
						GMT_Report(API, GMT_MSG_NORMAL, "A:%s %d %f %s\n", path, format, file_weight, apath);
					else if (pstatus == MB_PROCESSED_USE)
						GMT_Report(API, GMT_MSG_NORMAL, "P:%s %d %f\n", path, format, file_weight);
					else
						GMT_Report(API, GMT_MSG_NORMAL, "R:%s %d %f\n", path, format, file_weight);
					fflush(dfp);
				}
			}
		}
		if (datalist != NULL) mb_datalist_close(verbose, &datalist, &error);
		GMT_Report(API, GMT_MSG_NORMAL, "\n%d total data points processed\n", ndata);

		if (dfp != NULL) { fclose(dfp); dfp = NULL; }

		if (verbose >= 1) GMT_Report(API, GMT_MSG_NORMAL, "\nMaking low resolution slope grid...\n");
		ndata = 8;
		for (int i = 0; i < sxdim; i++)
			for (int j = 0; j < sydim; j++) {
				kgrid = i * sydim + j;
				if (cnt[kgrid] > 0) { gridsmall[kgrid] /= (double)cnt[kgrid]; ndata++; }
			}

#ifdef USESURFACE
		status = mb_mallocd(verbose, __FILE__, __LINE__, ndata * sizeof(float), (void **)&sxdata, &error);
		if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, ndata * sizeof(float), (void **)&sydata, &error);
		if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, ndata * sizeof(float), (void **)&szdata, &error);
		if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, sxdim * sydim * sizeof(float), (void **)&sgrid, &error);
		if (error != MB_ERROR_NO_ERROR) {
			mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating interpolation work arrays:\n%s\n", message);
			mb_memory_clear(verbose, &memclear_error);
			Return(error);
		}
		memset((char *)sgrid, 0, sxdim * sydim * sizeof(float));
		memset((char *)sxdata, 0, ndata * sizeof(float));
		memset((char *)sydata, 0, ndata * sizeof(float));
		memset((char *)szdata, 0, ndata * sizeof(float));

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
		GMT_Report(API, GMT_MSG_NORMAL, "\nDoing Surface spline interpolation with %d data points...\n", ndata);
		mb_surface(verbose, ndata, sxdata, sydata, szdata, (wbnd[0] - bdata_origin_x), (wbnd[1] - bdata_origin_x), (wbnd[2] - bdata_origin_y), (wbnd[3] - bdata_origin_y), sdx, sdy, tension, sgrid);
#else
		status = mb_mallocd(verbose, __FILE__, __LINE__, 3 * ndata * sizeof(float), (void **)&sdata, &error);
		if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, sxdim * sydim * sizeof(float), (void **)&sgrid, &error);
		if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, ndata * sizeof(float), (void **)&work1, &error);
		if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, ndata * sizeof(int), (void **)&work2, &error);
		if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, (sxdim + sydim) * sizeof(bool), (void **)&work3, &error);
		if (error != MB_ERROR_NO_ERROR) {
			mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating interpolation work arrays:\n%s\n", message);
			mb_memory_clear(verbose, &memclear_error);
			Return(error);
		}
		memset((char *)sgrid, 0, sxdim * sydim * sizeof(float));
		memset((char *)sdata, 0, 3 * ndata * sizeof(float));
		memset((char *)work1, 0, ndata * sizeof(float));
		memset((char *)work2, 0, ndata * sizeof(int));
		memset((char *)work3, 0, (sxdim + sydim) * sizeof(bool));

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

		{
			float cay = (float)tension;
			float xmin = (float)(wbnd[0] - 0.5 * sdx - bdata_origin_x);
			float ymin = (float)(wbnd[2] - 0.5 * sdy - bdata_origin_y);
			float ddx = (float)sdx;
			float ddy = (float)sdy;
			GMT_Report(API, GMT_MSG_NORMAL, "\nDoing Zgrid spline interpolation with %d data points...\n", ndata);
			mb_zgrid2(sgrid, &sxdim, &sydim, &xmin, &ymin, &ddx, &ddy, sdata, &ndata, work1, work2, work3, &cay, &sclip);
		}
#endif

		for (int i = 0; i < sxdim; i++)
			for (int j = 0; j < sydim; j++) {
				kgrid = i * sydim + j;
#ifdef USESURFACE
				kint = i + (sydim - j - 1) * sxdim;
#else
				kint = i + j * sxdim;
#endif
				if (cnt[kgrid] == 0) gridsmall[kgrid] = sgrid[kint];
			}

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

		/* second pass footprint gridding using slope estimates */
		for (int i = 0; i < gxdim; i++)
			for (int j = 0; j < gydim; j++) {
				kgrid = i * gydim + j;
				grid[kgrid] = 0.0; norm[kgrid] = 0.0; sigma[kgrid] = 0.0; firsttime[kgrid] = 0.0; num[kgrid] = 0; cnt[kgrid] = 0;
			}

		GMT_Report(API, GMT_MSG_NORMAL, "\nDoing second pass to generate final grid...\n");
		ndata = 0;
		if (mb_datalist_open(verbose, &datalist, dfile, look_processed, &error) != MB_SUCCESS) {
			error = MB_ERROR_OPEN_FAIL;
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open data list file: %s\n", filelist);
			mb_memory_clear(verbose, &memclear_error);
			Return(error);
		}
		while (mb_datalist_read3(verbose, datalist, &pstatus, path, ppath, &astatus, apath, dpath, &format, &file_weight, &error) == MB_SUCCESS) {
			ndatafile = 0;
			if (format > 0 && path[0] != '#') {
				if (pstatus == MB_PROCESSED_USE) strcpy(file, ppath); else strcpy(file, path);
				rformat = format;
				strcpy(rfile, file);
				status = mb_check_info(verbose, rfile, lonflip, bounds, &file_in_bounds, &error);
				if (status == MB_FAILURE) { file_in_bounds = true; status = MB_SUCCESS; error = MB_ERROR_NO_ERROR; }

				bool first = true;
				double dmin = 0.0, dmax = 0.0;
				if (file_in_bounds) {
					if (datatype == MBGRID_DATA_TOPOGRAPHY || datatype == MBGRID_DATA_BATHYMETRY)
						mb_get_fbt(verbose, rfile, &rformat, &error);
					if (mb_read_init_altnav(verbose, rfile, rformat, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, astatus, apath, &mbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
						mb_error(verbose, error, &message);
						GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error returned from function <mb_read_init_altnav>:\n%s\n", message);
						mb_memory_clear(verbose, &memclear_error);
						Return(error);
					}
					mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
					status = mb_sonartype(verbose, mbio_ptr, mb_io_ptr->store_data, &topo_type, &error);

					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlon, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlat, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslon, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslat, &error);

					if (error != MB_ERROR_NO_ERROR) {
						mb_error(verbose, error, &message);
						GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
						mb_memory_clear(verbose, &memclear_error);
						Return(error);
					}

					while (error <= MB_ERROR_NO_ERROR) {
						status = mb_read(verbose, mbio_ptr, &kind, &rpings, time_i, &time_d, &navlon, &navlat, &speed, &heading, &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp, bathlon, bathlat, ss, sslon, sslat, comment, &error);
						if (error == MB_ERROR_TIME_GAP) { error = MB_ERROR_NO_ERROR; status = MB_SUCCESS; }

						if (verbose >= 2) {
							GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  Ping read in program <%s>\n", THIS_MODULE_NAME);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       kind:           %d\n", kind);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       beams_bath:     %d\n", beams_bath);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       beams_amp:      %d\n", beams_amp);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       pixels_ss:      %d\n", pixels_ss);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       error:          %d\n", error);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       status:         %d\n", status);
						}

						if (shift_mode == MBGRID_SHIFT_DATA) {
							navlon += shift_lon; navlat += shift_lat;
							for (ib = 0; ib < beams_bath; ib++)
								if (mb_beam_ok(beamflag[ib])) { bathlon[ib] += shift_lon; bathlat[ib] += shift_lat; }
						}

						if ((datatype == MBGRID_DATA_BATHYMETRY || datatype == MBGRID_DATA_TOPOGRAPHY) && error == MB_ERROR_NO_ERROR) {

							if (topo_type == MB_TOPOGRAPHY_TYPE_UNKNOWN) {
								status = mb_sonartype(verbose, mbio_ptr, mb_io_ptr->store_data, &topo_type, &error);
								if (topo_type == MB_TOPOGRAPHY_TYPE_UNKNOWN && mb_io_ptr->beamwidth_xtrack > 0.0 && mb_io_ptr->beamwidth_ltrack > 0.0)
									topo_type = MB_TOPOGRAPHY_TYPE_MULTIBEAM;
							}

							if (use_projection) {
								mb_proj_forward(verbose, pjptr, navlon, navlat, &navlon, &navlat, &error);
								for (ib = 0; ib < beams_bath; ib++)
									if (mb_beam_ok(beamflag[ib]))
										mb_proj_forward(verbose, pjptr, bathlon[ib], bathlat[ib], &bathlon[ib], &bathlat[ib], &error);
							}

							for (ib = 0; ib < beams_bath; ib++)
								if (mb_beam_ok(beamflag[ib])) {
									ix = (int)((bathlon[ib] - wbnd[0] + 0.5 * dx) / dx);
									iy = (int)((bathlat[ib] - wbnd[2] + 0.5 * dy) / dy);

									if (topo_type != MB_TOPOGRAPHY_TYPE_MULTIBEAM) {
										if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
											kgrid = ix * gydim + iy;
											norm[kgrid] += file_weight;
											grid[kgrid] += file_weight * topofactor * bath[ib];
											sigma[kgrid] += file_weight * topofactor * topofactor * bath[ib] * bath[ib];
											num[kgrid]++; cnt[kgrid]++; ndata++; ndatafile++;
											if (first) { first = false; dmin = topofactor * bath[ib]; dmax = topofactor * bath[ib]; }
											else { dmin = MIN(topofactor * bath[ib], dmin); dmax = MAX(topofactor * bath[ib], dmax); }
										}
									} else {
										int isx = (int)((bathlon[ib] - wbnd[0] + 0.5 * sdx) / sdx);
										int isy = (int)((bathlat[ib] - wbnd[2] + 0.5 * sdy) / sdy);
										isx = MIN(MAX(isx, 0), sxdim - 1);
										isy = MIN(MAX(isy, 0), sydim - 1);
										double dzdx, dzdy;
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

										region_ok = (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim);
										time_ok = true;
										if (region_ok && check_time) {
											kgrid = ix * gydim + iy;
											if (firsttime[kgrid] <= 0.0) { firsttime[kgrid] = time_d; time_ok = true; }
											else if (fabs(time_d - firsttime[kgrid]) > timediff) {
												if (first_in_stays) time_ok = false;
												else {
													time_ok = true;
													firsttime[kgrid] = time_d;
													ndata -= cnt[kgrid]; ndatafile -= cnt[kgrid];
													norm[kgrid] = 0.0; grid[kgrid] = 0.0; sigma[kgrid] = 0.0;
													num[kgrid] = 0; cnt[kgrid] = 0;
												}
											}
										}

										if (region_ok && time_ok) {
											if (use_projection) {
												foot_dx = bathlon[ib] - navlon;
												foot_dy = bathlat[ib] - navlat;
											} else {
												foot_dx = (bathlon[ib] - navlon) / mtodeglon;
												foot_dy = (bathlat[ib] - navlat) / mtodeglat;
											}
											foot_lateral = sqrt(foot_dx * foot_dx + foot_dy * foot_dy);
											if (foot_lateral > 0.0) { foot_dxn = foot_dx / foot_lateral; foot_dyn = foot_dy / foot_lateral; }
											else { foot_dxn = 1.0; foot_dyn = 0.0; }
											beam_altitude = bath[ib] - sensordepth;
											foot_range = sqrt(foot_lateral * foot_lateral + beam_altitude * beam_altitude);
											foot_theta = RTD * atan2(foot_lateral, beam_altitude);
											if (foot_range > 0.0 && foot_theta < FOOT_THETA_MAX) {
												footprint_ok = true;
												foot_dtheta = 0.5 * scale * mb_io_ptr->beamwidth_xtrack;
												foot_dphi = 0.5 * scale * mb_io_ptr->beamwidth_ltrack;
												if (foot_dtheta <= 0.0) foot_dtheta = 1.0;
												if (foot_dphi <= 0.0) foot_dphi = 1.0;
												foot_hwidth = (bath[ib] - sensordepth) * tan(DTR * (foot_theta + foot_dtheta)) - foot_lateral;
												foot_hlength = foot_range * tan(DTR * foot_dphi);
											} else {
												footprint_ok = false;
											}
										}

										if (time_ok && region_ok && footprint_ok) {
											if (use_projection) {
												foot_wix = (int)fabs(foot_hwidth * cos(DTR * foot_theta) / dx);
												foot_wiy = (int)fabs(foot_hwidth * sin(DTR * foot_theta) / dx);
												foot_lix = (int)fabs(foot_hlength * sin(DTR * foot_theta) / dy);
												foot_liy = (int)fabs(foot_hlength * cos(DTR * foot_theta) / dy);
											} else {
												foot_wix = (int)fabs(foot_hwidth * cos(DTR * foot_theta) * mtodeglon / dx);
												foot_wiy = (int)fabs(foot_hwidth * sin(DTR * foot_theta) * mtodeglon / dx);
												foot_lix = (int)fabs(foot_hlength * sin(DTR * foot_theta) * mtodeglat / dy);
												foot_liy = (int)fabs(foot_hlength * cos(DTR * foot_theta) * mtodeglat / dy);
											}
											foot_dix = 2 * MAX(foot_wix, foot_lix);
											foot_diy = 2 * MAX(foot_wiy, foot_liy);
											ix1 = MAX(ix - foot_dix, 0);
											ix2 = MIN(ix + foot_dix, gxdim - 1);
											iy1 = MAX(iy - foot_diy, 0);
											iy2 = MIN(iy + foot_diy, gydim - 1);

											for (int ii = ix1; ii <= ix2; ii++)
												for (int jj = iy1; jj <= iy2; jj++) {
													kgrid = ii * gydim + jj;
													xx = (wbnd[0] + ii * dx + 0.5 * dx - bathlon[ib]);
													yy = (wbnd[2] + jj * dy + 0.5 * dy - bathlat[ib]);
													sbath = topofactor * bath[ib] + dzdx * xx + dzdy * yy;
													if (use_projection) { xx0 = xx; yy0 = yy; bdx = 0.5 * dx; bdy = 0.5 * dy; }
													else { xx0 = xx / mtodeglon; yy0 = yy / mtodeglat; bdx = 0.5 * dx / mtodeglon; bdy = 0.5 * dy / mtodeglat; }
													xx1 = xx0 - bdx; xx2 = xx0 + bdx; yy1 = yy0 - bdy; yy2 = yy0 + bdy;

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

													mbgrid_weight(verbose, foot_hwidth, foot_hlength, prx[0], pry[0], bdx, bdy, &prx[1], &pry[1], &weight, &use_weight, &error);

													if (use_weight != MBGRID_USE_NO && weight > 0.000001) {
														weight *= file_weight;
														norm[kgrid] += weight;
														grid[kgrid] += weight * sbath;
														sigma[kgrid] += weight * sbath * sbath;
														if (use_weight == MBGRID_USE_YES) {
															num[kgrid]++;
															if (ii == ix && jj == iy) cnt[kgrid]++;
														}
													}
												}
											ndata++; ndatafile++;
											if (first) { first = false; dmin = topofactor * bath[ib]; dmax = topofactor * bath[ib]; }
											else { dmin = MIN(topofactor * bath[ib], dmin); dmax = MAX(topofactor * bath[ib], dmax); }
										} else if (time_ok && region_ok) {
											kgrid = ix * gydim + iy;
											norm[kgrid] += file_weight;
											grid[kgrid] += file_weight * topofactor * bath[ib];
											sigma[kgrid] += file_weight * topofactor * topofactor * bath[ib] * bath[ib];
											num[kgrid]++; cnt[kgrid]++; ndata++; ndatafile++;
											if (first) { first = false; dmin = topofactor * bath[ib]; dmax = topofactor * bath[ib]; }
											else { dmin = MIN(topofactor * bath[ib], dmin); dmax = MAX(topofactor * bath[ib], dmax); }
										}
									}
								}
						}
					}
					mb_close(verbose, &mbio_ptr, &error);
					status = MB_SUCCESS; error = MB_ERROR_NO_ERROR;
				}
				if (verbose > 0 || file_in_bounds)
					GMT_Report(API, GMT_MSG_NORMAL, "%d data points processed in %s (minmax: %f %f)\n", ndatafile, rfile, dmin, dmax);
			}
		}
		if (datalist != NULL) mb_datalist_close(verbose, &datalist, &error);
		GMT_Report(API, GMT_MSG_NORMAL, "\n%d total data points processed\n", ndata);

		if (verbose >= 1) GMT_Report(API, GMT_MSG_NORMAL, "\nMaking raw grid...\n");
		for (int i = 0; i < gxdim; i++)
			for (int j = 0; j < gydim; j++) {
				kgrid = i * gydim + j;
				if (num[kgrid] > 0) {
					grid[kgrid] /= norm[kgrid];
					factor = sigma[kgrid] / norm[kgrid] - grid[kgrid] * grid[kgrid];
					sigma[kgrid] = sqrt(fabs(factor));
					nbinset++;
				} else { grid[kgrid] = clipvalue; sigma[kgrid] = 0.0; }
			}
	}
/* MEDIAN_SENTINEL */
/* -------------------------------------------------------------------------- */
	/***** do weighted footprint gridding *****/
	else if (grid_mode == MBGRID_WEIGHTED_FOOTPRINT) {

		status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(double), (void **)&norm, &error);

		for (int i = 0; i < gxdim; i++)
			for (int j = 0; j < gydim; j++) {
				kgrid = i * gydim + j;
				grid[kgrid] = 0.0; norm[kgrid] = 0.0; sigma[kgrid] = 0.0; firsttime[kgrid] = 0.0; num[kgrid] = 0; cnt[kgrid] = 0;
			}

		GMT_Report(API, GMT_MSG_NORMAL, "\nDoing single pass to generate grid...\n");
		ndata = 0;
		if (mb_datalist_open(verbose, &datalist, filelist, look_processed, &error) != MB_SUCCESS) {
			error = MB_ERROR_OPEN_FAIL;
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open data list file: %s\n", filelist);
			mb_memory_clear(verbose, &memclear_error);
			Return(error);
		}
		while (mb_datalist_read3(verbose, datalist, &pstatus, path, ppath, &astatus, apath, dpath, &format, &file_weight, &error) == MB_SUCCESS) {
			ndatafile = 0;
			if (format > 0 && path[0] != '#') {
				if (pstatus == MB_PROCESSED_USE) strcpy(file, ppath); else strcpy(file, path);
				rformat = format;
				strcpy(rfile, file);
				status = mb_check_info(verbose, rfile, lonflip, bounds, &file_in_bounds, &error);
				if (status == MB_FAILURE) { file_in_bounds = true; status = MB_SUCCESS; error = MB_ERROR_NO_ERROR; }

				bool first = true;
				double dmin = 0.0, dmax = 0.0;
				if (file_in_bounds) {
					if (datatype == MBGRID_DATA_TOPOGRAPHY || datatype == MBGRID_DATA_BATHYMETRY)
						mb_get_fbt(verbose, rfile, &rformat, &error);
					if (mb_read_init_altnav(verbose, rfile, rformat, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, astatus, apath, &mbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
						mb_error(verbose, error, &message);
						GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error returned from function <mb_read_init_altnav>:\n%s\n", message);
						mb_memory_clear(verbose, &memclear_error);
						Return(error);
					}
					mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
					status = mb_sonartype(verbose, mbio_ptr, mb_io_ptr->store_data, &topo_type, &error);

					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlon, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlat, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslon, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslat, &error);

					if (error != MB_ERROR_NO_ERROR) {
						mb_error(verbose, error, &message);
						GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
						mb_memory_clear(verbose, &memclear_error);
						Return(error);
					}

					while (error <= MB_ERROR_NO_ERROR) {
						status = mb_read(verbose, mbio_ptr, &kind, &rpings, time_i, &time_d, &navlon, &navlat, &speed, &heading, &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp, bathlon, bathlat, ss, sslon, sslat, comment, &error);
						if (error == MB_ERROR_TIME_GAP) { error = MB_ERROR_NO_ERROR; status = MB_SUCCESS; }

						if (verbose >= 2) {
							GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  Ping read in program <%s>\n", THIS_MODULE_NAME);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       kind:           %d\n", kind);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       beams_bath:     %d\n", beams_bath);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       beams_amp:      %d\n", beams_amp);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       pixels_ss:      %d\n", pixels_ss);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       error:          %d\n", error);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       status:         %d\n", status);
						}

						if (shift_mode == MBGRID_SHIFT_DATA) {
							navlon += shift_lon; navlat += shift_lat;
							for (ib = 0; ib < beams_bath; ib++)
								if (mb_beam_ok(beamflag[ib])) { bathlon[ib] += shift_lon; bathlat[ib] += shift_lat; }
						}

						if ((datatype == MBGRID_DATA_BATHYMETRY || datatype == MBGRID_DATA_TOPOGRAPHY) && error == MB_ERROR_NO_ERROR) {

							if (topo_type == MB_TOPOGRAPHY_TYPE_UNKNOWN) {
								status = mb_sonartype(verbose, mbio_ptr, mb_io_ptr->store_data, &topo_type, &error);
								if (topo_type == MB_TOPOGRAPHY_TYPE_UNKNOWN && mb_io_ptr->beamwidth_xtrack > 0.0 && mb_io_ptr->beamwidth_ltrack > 0.0)
									topo_type = MB_TOPOGRAPHY_TYPE_MULTIBEAM;
							}

							if (use_projection) {
								mb_proj_forward(verbose, pjptr, navlon, navlat, &navlon, &navlat, &error);
								for (ib = 0; ib < beams_bath; ib++)
									if (mb_beam_ok(beamflag[ib]))
										mb_proj_forward(verbose, pjptr, bathlon[ib], bathlat[ib], &bathlon[ib], &bathlat[ib], &error);
							}

							for (ib = 0; ib < beams_bath; ib++)
								if (mb_beam_ok(beamflag[ib])) {
									ix = (int)((bathlon[ib] - wbnd[0] + 0.5 * dx) / dx);
									iy = (int)((bathlat[ib] - wbnd[2] + 0.5 * dy) / dy);

									time_ok = true;
									if (check_time) {
										if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
											kgrid = ix * gydim + iy;
											if (firsttime[kgrid] <= 0.0) { firsttime[kgrid] = time_d; time_ok = true; }
											else if (fabs(time_d - firsttime[kgrid]) > timediff) {
												if (first_in_stays) time_ok = false;
												else {
													time_ok = true;
													firsttime[kgrid] = time_d;
													ndata -= cnt[kgrid]; ndatafile -= cnt[kgrid];
													norm[kgrid] = 0.0; grid[kgrid] = 0.0; sigma[kgrid] = 0.0; num[kgrid] = 0; cnt[kgrid] = 0;
												}
											} else time_ok = true;
										}
									}

									if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim && time_ok) {
										if (topo_type != MB_TOPOGRAPHY_TYPE_MULTIBEAM) {
											kgrid = ix * gydim + iy;
											norm[kgrid] += file_weight;
											grid[kgrid] += file_weight * topofactor * bath[ib];
											sigma[kgrid] += file_weight * topofactor * topofactor * bath[ib] * bath[ib];
											num[kgrid]++; cnt[kgrid]++; ndata++; ndatafile++;
											if (first) { first = false; dmin = topofactor * bath[ib]; dmax = topofactor * bath[ib]; }
											else { dmin = MIN(topofactor * bath[ib], dmin); dmax = MAX(topofactor * bath[ib], dmax); }
										} else {
											if (use_projection) { foot_dx = bathlon[ib] - navlon; foot_dy = bathlat[ib] - navlat; }
											else { foot_dx = (bathlon[ib] - navlon) / mtodeglon; foot_dy = (bathlat[ib] - navlat) / mtodeglat; }
											foot_lateral = sqrt(foot_dx * foot_dx + foot_dy * foot_dy);
											if (foot_lateral > 0.0) { foot_dxn = foot_dx / foot_lateral; foot_dyn = foot_dy / foot_lateral; }
											else { foot_dxn = 1.0; foot_dyn = 0.0; }
											foot_range = sqrt(foot_lateral * foot_lateral + altitude * altitude);
											if (foot_range > 0.0) {
												foot_theta = RTD * atan2(foot_lateral, (bath[ib] - sensordepth));
												foot_dtheta = 0.5 * scale * mb_io_ptr->beamwidth_xtrack;
												foot_dphi = 0.5 * scale * mb_io_ptr->beamwidth_ltrack;
												if (foot_dtheta <= 0.0) foot_dtheta = 1.0;
												if (foot_dphi <= 0.0) foot_dphi = 1.0;
												foot_hwidth = (bath[ib] - sensordepth) * tan(DTR * (foot_theta + foot_dtheta)) - foot_lateral;
												foot_hlength = foot_range * tan(DTR * foot_dphi);

												if (use_projection) {
													foot_wix = (int)fabs(foot_hwidth * cos(DTR * foot_theta) / dx);
													foot_wiy = (int)fabs(foot_hwidth * sin(DTR * foot_theta) / dx);
													foot_lix = (int)fabs(foot_hlength * sin(DTR * foot_theta) / dy);
													foot_liy = (int)fabs(foot_hlength * cos(DTR * foot_theta) / dy);
												} else {
													foot_wix = (int)fabs(foot_hwidth * cos(DTR * foot_theta) * mtodeglon / dx);
													foot_wiy = (int)fabs(foot_hwidth * sin(DTR * foot_theta) * mtodeglon / dx);
													foot_lix = (int)fabs(foot_hlength * sin(DTR * foot_theta) * mtodeglat / dy);
													foot_liy = (int)fabs(foot_hlength * cos(DTR * foot_theta) * mtodeglat / dy);
												}
												foot_dix = 2 * MAX(foot_wix, foot_lix);
												foot_diy = 2 * MAX(foot_wiy, foot_liy);
												ix1 = MAX(ix - foot_dix, 0);
												ix2 = MIN(ix + foot_dix, gxdim - 1);
												iy1 = MAX(iy - foot_diy, 0);
												iy2 = MIN(iy + foot_diy, gydim - 1);

												for (int ii = ix1; ii <= ix2; ii++)
													for (int jj = iy1; jj <= iy2; jj++) {
														kgrid = ii * gydim + jj;
														xx = (wbnd[0] + ii * dx + 0.5 * dx - bathlon[ib]);
														yy = (wbnd[2] + jj * dy + 0.5 * dy - bathlat[ib]);
														sbath = topofactor * bath[ib];
														if (use_projection) { xx0 = xx; yy0 = yy; bdx = 0.5 * dx; bdy = 0.5 * dy; }
														else { xx0 = xx / mtodeglon; yy0 = yy / mtodeglat; bdx = 0.5 * dx / mtodeglon; bdy = 0.5 * dy / mtodeglat; }
														xx1 = xx0 - bdx; xx2 = xx0 + bdx; yy1 = yy0 - bdy; yy2 = yy0 + bdy;
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
														mbgrid_weight(verbose, foot_hwidth, foot_hlength, prx[0], pry[0], bdx, bdy, &prx[1], &pry[1], &weight, &use_weight, &error);
														if (use_weight != MBGRID_USE_NO && weight > 0.000001) {
															weight *= file_weight;
															norm[kgrid] += weight;
															grid[kgrid] += weight * sbath;
															sigma[kgrid] += weight * sbath * sbath;
															if (use_weight == MBGRID_USE_YES) {
																num[kgrid]++;
																if (ii == ix && jj == iy) cnt[kgrid]++;
															}
														}
													}
												ndata++; ndatafile++;
												if (first) { first = false; dmin = topofactor * bath[ib]; dmax = topofactor * bath[ib]; }
												else { dmin = MIN(topofactor * bath[ib], dmin); dmax = MAX(topofactor * bath[ib], dmax); }
											} else if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
												kgrid = ix * gydim + iy;
												norm[kgrid] += file_weight;
												grid[kgrid] += file_weight * topofactor * bath[ib];
												sigma[kgrid] += file_weight * topofactor * topofactor * bath[ib] * bath[ib];
												num[kgrid]++; cnt[kgrid]++; ndata++; ndatafile++;
												if (first) { first = false; dmin = topofactor * bath[ib]; dmax = topofactor * bath[ib]; }
												else { dmin = MIN(topofactor * bath[ib], dmin); dmax = MAX(topofactor * bath[ib], dmax); }
											}
										}
									}
								}
						}
					}
					mb_close(verbose, &mbio_ptr, &error);
					status = MB_SUCCESS; error = MB_ERROR_NO_ERROR;
				}
				if (verbose > 0 || file_in_bounds) {
					if (astatus == MB_ALTNAV_USE)
						GMT_Report(API, GMT_MSG_NORMAL, "%d data points processed in %s (minmax: %f %f) using nav from %s\n", ndatafile, rfile, dmin, dmax, apath);
					else
						GMT_Report(API, GMT_MSG_NORMAL, "%d data points processed in %s (minmax: %f %f)\n", ndatafile, rfile, dmin, dmax);
				}
				if (ndatafile > 0 && dfp != NULL) {
					if (pstatus == MB_PROCESSED_USE && astatus == MB_ALTNAV_USE)
						GMT_Report(API, GMT_MSG_NORMAL, "A:%s %d %f %s\n", path, format, file_weight, apath);
					else if (pstatus == MB_PROCESSED_USE)
						GMT_Report(API, GMT_MSG_NORMAL, "P:%s %d %f\n", path, format, file_weight);
					else
						GMT_Report(API, GMT_MSG_NORMAL, "R:%s %d %f\n", path, format, file_weight);
					fflush(dfp);
				}
			}
		}
		if (datalist != NULL) mb_datalist_close(verbose, &datalist, &error);
		GMT_Report(API, GMT_MSG_NORMAL, "\n%d total data points processed\n", ndata);

		if (dfp != NULL) { fclose(dfp); dfp = NULL; }

		if (verbose >= 1) GMT_Report(API, GMT_MSG_NORMAL, "\nMaking raw grid...\n");
		for (int i = 0; i < gxdim; i++)
			for (int j = 0; j < gydim; j++) {
				kgrid = i * gydim + j;
				if (num[kgrid] > 0) {
					grid[kgrid] /= norm[kgrid];
					factor = sigma[kgrid] / norm[kgrid] - grid[kgrid] * grid[kgrid];
					sigma[kgrid] = sqrt(fabs(factor));
					nbinset++;
				} else { grid[kgrid] = clipvalue; sigma[kgrid] = 0.0; }
			}
	}

/* -------------------------------------------------------------------------- */
	/***** else do median filtering gridding *****/
	else if (grid_mode == MBGRID_MEDIAN_FILTER) {

		status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(double *), (void **)&data, &error);
		if (error != MB_ERROR_NO_ERROR) {
			mb_error(verbose, error, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
			mb_memory_clear(verbose, &memclear_error);
			Return(error);
		}

		for (int i = 0; i < gxdim; i++)
			for (int j = 0; j < gydim; j++) {
				kgrid = i * gydim + j;
				grid[kgrid] = 0.0; sigma[kgrid] = 0.0; firsttime[kgrid] = 0.0; cnt[kgrid] = 0; num[kgrid] = 0;
				data[kgrid] = NULL;
			}

		ndata = 0;
		if (mb_datalist_open(verbose, &datalist, filelist, look_processed, &error) != MB_SUCCESS) {
			error = MB_ERROR_OPEN_FAIL;
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open data list file: %s\n", filelist);
			mb_memory_clear(verbose, &memclear_error);
			Return(error);
		}
		while (mb_datalist_read3(verbose, datalist, &pstatus, path, ppath, &astatus, apath, dpath, &format, &file_weight, &error) == MB_SUCCESS) {
			ndatafile = 0;
			if (format > 0 && path[0] != '#') {
				if (pstatus == MB_PROCESSED_USE) strcpy(file, ppath); else strcpy(file, path);
				rformat = format;
				strcpy(rfile, file);
				status = mb_check_info(verbose, file, lonflip, bounds, &file_in_bounds, &error);
				if (status == MB_FAILURE) { file_in_bounds = true; status = MB_SUCCESS; error = MB_ERROR_NO_ERROR; }

				bool first = true;
				double dmin = 0.0, dmax = 0.0;
				if (file_in_bounds) {
					if (datatype == MBGRID_DATA_TOPOGRAPHY || datatype == MBGRID_DATA_BATHYMETRY)
						mb_get_fbt(verbose, rfile, &rformat, &error);
					if (mb_read_init_altnav(verbose, rfile, rformat, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, astatus, apath, &mbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
						mb_error(verbose, error, &message);
						GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error returned from function <mb_read_init_altnav>:\n%s\n", message);
						mb_memory_clear(verbose, &memclear_error);
						Return(error);
					}

					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlon, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlat, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslon, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslat, &error);

					if (error != MB_ERROR_NO_ERROR) {
						mb_error(verbose, error, &message);
						GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
						mb_memory_clear(verbose, &memclear_error);
						Return(error);
					}

					while (error <= MB_ERROR_NO_ERROR) {
						status = mb_read(verbose, mbio_ptr, &kind, &rpings, time_i, &time_d, &navlon, &navlat, &speed, &heading, &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp, bathlon, bathlat, ss, sslon, sslat, comment, &error);
						if (error == MB_ERROR_TIME_GAP) { error = MB_ERROR_NO_ERROR; status = MB_SUCCESS; }

						if (verbose >= 2) {
							GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  Ping read in program <%s>\n", THIS_MODULE_NAME);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       kind:           %d\n", kind);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       beams_bath:     %d\n", beams_bath);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       beams_amp:      %d\n", beams_amp);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       pixels_ss:      %d\n", pixels_ss);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       error:          %d\n", error);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       status:         %d\n", status);
						}

						if (shift_mode == MBGRID_SHIFT_DATA) {
							navlon += shift_lon; navlat += shift_lat;
							for (ib = 0; ib < beams_bath; ib++)
								if (mb_beam_ok(beamflag[ib])) { bathlon[ib] += shift_lon; bathlat[ib] += shift_lat; }
							for (ib = 0; ib < pixels_ss; ib++)
								if (ss[ib] > MB_SIDESCAN_NULL) { sslon[ib] += shift_lon; sslat[ib] += shift_lat; }
						}

						if ((datatype == MBGRID_DATA_BATHYMETRY || datatype == MBGRID_DATA_TOPOGRAPHY) && error == MB_ERROR_NO_ERROR) {
							if (use_projection) {
								for (ib = 0; ib < beams_bath; ib++)
									if (mb_beam_ok(beamflag[ib]))
										mb_proj_forward(verbose, pjptr, bathlon[ib], bathlat[ib], &bathlon[ib], &bathlat[ib], &error);
							}
							for (ib = 0; ib < beams_bath; ib++)
								if (mb_beam_ok(beamflag[ib])) {
									ix = (int)((bathlon[ib] - wbnd[0] + 0.5 * dx) / dx);
									iy = (int)((bathlat[ib] - wbnd[2] + 0.5 * dy) / dy);
									if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
										kgrid = ix * gydim + iy;
										time_ok = true;
										if (check_time) time_ok = true;
										else {
											if (firsttime[kgrid] <= 0.0) { firsttime[kgrid] = time_d; time_ok = true; }
											else if (fabs(time_d - firsttime[kgrid]) > timediff) {
												if (first_in_stays) time_ok = false;
												else { time_ok = true; firsttime[kgrid] = time_d; ndata -= cnt[kgrid]; ndatafile -= cnt[kgrid]; cnt[kgrid] = 0; }
											} else time_ok = true;
										}
										if (time_ok && cnt[kgrid] >= num[kgrid]) {
											num[kgrid] += REALLOC_STEP_SIZE;
											if ((data[kgrid] = (double *)realloc(data[kgrid], num[kgrid] * sizeof(double))) == NULL) {
												error = MB_ERROR_MEMORY_FAIL;
												mb_error(verbose, error, &message);
												GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
												mb_memory_clear(verbose, &memclear_error);
												Return(error);
											}
										}
										if (time_ok) {
											value = data[kgrid];
											value[cnt[kgrid]] = topofactor * bath[ib];
											cnt[kgrid]++; ndata++; ndatafile++;
											if (first) { first = false; dmin = topofactor * bath[ib]; dmax = topofactor * bath[ib]; }
											else { dmin = MIN(topofactor * bath[ib], dmin); dmax = MAX(topofactor * bath[ib], dmax); }
										}
									}
								}
						} else if (datatype == MBGRID_DATA_AMPLITUDE && error == MB_ERROR_NO_ERROR) {
							if (use_projection) {
								for (ib = 0; ib < beams_amp; ib++)
									if (mb_beam_ok(beamflag[ib]))
										mb_proj_forward(verbose, pjptr, bathlon[ib], bathlat[ib], &bathlon[ib], &bathlat[ib], &error);
							}
							for (ib = 0; ib < beams_bath; ib++)
								if (mb_beam_ok(beamflag[ib])) {
									ix = (int)((bathlon[ib] - wbnd[0] + 0.5 * dx) / dx);
									iy = (int)((bathlat[ib] - wbnd[2] + 0.5 * dy) / dy);
									if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
										kgrid = ix * gydim + iy;
										time_ok = true;
										if (!check_time) time_ok = true;
										else {
											if (firsttime[kgrid] <= 0.0) { firsttime[kgrid] = time_d; time_ok = true; }
											else if (fabs(time_d - firsttime[kgrid]) > timediff) {
												if (first_in_stays) time_ok = false;
												else { time_ok = true; firsttime[kgrid] = time_d; ndata -= cnt[kgrid]; ndatafile -= cnt[kgrid]; cnt[kgrid] = 0; }
											} else time_ok = true;
										}
										if (time_ok && cnt[kgrid] >= num[kgrid]) {
											num[kgrid] += REALLOC_STEP_SIZE;
											if ((data[kgrid] = (double *)realloc(data[kgrid], num[kgrid] * sizeof(double))) == NULL) {
												error = MB_ERROR_MEMORY_FAIL;
												mb_error(verbose, error, &message);
												GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
												mb_memory_clear(verbose, &memclear_error);
												Return(error);
											}
										}
										if (time_ok) {
											value = data[kgrid];
											value[cnt[kgrid]] = amp[ib];
											cnt[kgrid]++; ndata++; ndatafile++;
											if (first) { first = false; dmin = amp[ib]; dmax = amp[ib]; }
											else { dmin = MIN(amp[ib], dmin); dmax = MAX(amp[ib], dmax); }
										}
									}
								}
						} else if (datatype == MBGRID_DATA_SIDESCAN && error == MB_ERROR_NO_ERROR) {
							if (use_projection) {
								for (ib = 0; ib < pixels_ss; ib++)
									if (ss[ib] > MB_SIDESCAN_NULL)
										mb_proj_forward(verbose, pjptr, sslon[ib], sslat[ib], &sslon[ib], &sslat[ib], &error);
							}
							for (ib = 0; ib < pixels_ss; ib++)
								if (ss[ib] > MB_SIDESCAN_NULL) {
									ix = (int)((sslon[ib] - wbnd[0] + 0.5 * dx) / dx);
									iy = (int)((sslat[ib] - wbnd[2] + 0.5 * dy) / dy);
									if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
										kgrid = ix * gydim + iy;
										time_ok = true;
										if (!check_time) time_ok = true;
										else {
											if (firsttime[kgrid] <= 0.0) { firsttime[kgrid] = time_d; time_ok = true; }
											else if (fabs(time_d - firsttime[kgrid]) > timediff) {
												if (first_in_stays) time_ok = false;
												else { time_ok = true; firsttime[kgrid] = time_d; ndata -= cnt[kgrid]; ndatafile -= cnt[kgrid]; cnt[kgrid] = 0; }
											} else time_ok = true;
										}
										if (time_ok && cnt[kgrid] >= num[kgrid]) {
											num[kgrid] += REALLOC_STEP_SIZE;
											if ((data[kgrid] = (double *)realloc(data[kgrid], num[kgrid] * sizeof(double))) == NULL) {
												error = MB_ERROR_MEMORY_FAIL;
												mb_error(verbose, error, &message);
												GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
												mb_memory_clear(verbose, &memclear_error);
												Return(error);
											}
										}
										if (time_ok) {
											value = data[kgrid];
											value[cnt[kgrid]] = ss[ib];
											cnt[kgrid]++; ndata++; ndatafile++;
											if (first) { first = false; dmin = ss[ib]; dmax = ss[ib]; }
											else { dmin = MIN(ss[ib], dmin); dmax = MAX(ss[ib], dmax); }
										}
									}
								}
						}
					}
					mb_close(verbose, &mbio_ptr, &error);
					status = MB_SUCCESS; error = MB_ERROR_NO_ERROR;
				}
				if (verbose > 0 || file_in_bounds) {
					if (astatus == MB_ALTNAV_USE)
						GMT_Report(API, GMT_MSG_NORMAL, "%d data points processed in %s (minmax: %f %f) using nav from %s\n", ndatafile, rfile, dmin, dmax, apath);
					else
						GMT_Report(API, GMT_MSG_NORMAL, "%d data points processed in %s (minmax: %f %f)\n", ndatafile, rfile, dmin, dmax);
				}
				if (ndatafile > 0 && dfp != NULL) {
					if (pstatus == MB_PROCESSED_USE && astatus == MB_ALTNAV_USE)
						GMT_Report(API, GMT_MSG_NORMAL, "A:%s %d %f %s\n", path, format, file_weight, apath);
					else if (pstatus == MB_PROCESSED_USE)
						GMT_Report(API, GMT_MSG_NORMAL, "P:%s %d %f\n", path, format, file_weight);
					else
						GMT_Report(API, GMT_MSG_NORMAL, "R:%s %d %f\n", path, format, file_weight);
					fflush(dfp);
				}
			}
			else if (format == 0 && path[0] != '#') {
				if ((rfp = fopen(path, "r")) == NULL) {
					error = MB_ERROR_OPEN_FAIL;
					GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open lon,lat,value triples data path: %s\n", path);
					mb_memory_clear(verbose, &memclear_error);
					Return(error);
				}
				bool first = true;
				double dmin = 0.0, dmax = 0.0;
				while (fscanf(rfp, "%lf %lf %lf", &tlon, &tlat, &tvalue) != EOF) {
					if (shift_mode == MBGRID_SHIFT_DATA) { tlon += shift_lon; tlat += shift_lat; }
					if (use_projection) mb_proj_forward(verbose, pjptr, tlon, tlat, &tlon, &tlat, &error);
					ix = (int)((tlon - wbnd[0] + 0.5 * dx) / dx);
					iy = (int)((tlat - wbnd[2] + 0.5 * dy) / dy);
					if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
						kgrid = ix * gydim + iy;
						if (!check_time) time_ok = true;
						else time_ok = (firsttime[kgrid] <= 0.0);
						if (time_ok && cnt[kgrid] >= num[kgrid]) {
							num[kgrid] += REALLOC_STEP_SIZE;
							if ((data[kgrid] = (double *)realloc(data[kgrid], num[kgrid] * sizeof(double))) == NULL) {
								error = MB_ERROR_MEMORY_FAIL;
								mb_error(verbose, error, &message);
								GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
								mb_memory_clear(verbose, &memclear_error);
								Return(error);
							}
						}
						if (time_ok) {
							value = data[kgrid];
							value[cnt[kgrid]] = topofactor * tvalue;
							cnt[kgrid]++; ndata++; ndatafile++;
							if (first) { first = false; dmin = topofactor * tvalue; dmax = topofactor * tvalue; }
							else { dmin = MIN(topofactor * tvalue, dmin); dmax = MAX(topofactor * tvalue, dmax); }
						}
					}
				}
				fclose(rfp);
				status = MB_SUCCESS; error = MB_ERROR_NO_ERROR;
				if (verbose > 0 || file_in_bounds) {
					if (astatus == MB_ALTNAV_USE)
						GMT_Report(API, GMT_MSG_NORMAL, "%d data points processed in %s (minmax: %f %f) using nav from %s\n", ndatafile, rfile, dmin, dmax, apath);
					else
						GMT_Report(API, GMT_MSG_NORMAL, "%d data points processed in %s (minmax: %f %f)\n", ndatafile, rfile, dmin, dmax);
				}
				if (ndatafile > 0 && dfp != NULL) {
					if (pstatus == MB_PROCESSED_USE && astatus == MB_ALTNAV_USE)
						GMT_Report(API, GMT_MSG_NORMAL, "A:%s %d %f %s\n", path, format, file_weight, apath);
					else if (pstatus == MB_PROCESSED_USE)
						GMT_Report(API, GMT_MSG_NORMAL, "P:%s %d %f\n", path, format, file_weight);
					else
						GMT_Report(API, GMT_MSG_NORMAL, "R:%s %d %f\n", path, format, file_weight);
					fflush(dfp);
				}
			}
		}
		if (datalist != NULL) mb_datalist_close(verbose, &datalist, &error);
		GMT_Report(API, GMT_MSG_NORMAL, "\n%d total data points processed\n", ndata);

		if (dfp != NULL) { fclose(dfp); dfp = NULL; }

		if (verbose >= 1) GMT_Report(API, GMT_MSG_NORMAL, "\nMaking raw grid...\n");
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
					if (cnt[kgrid] > 1) sigma[kgrid] = sqrt(sigma[kgrid] / (cnt[kgrid] - 1));
					else sigma[kgrid] = 0.0;
					nbinset++;
				} else grid[kgrid] = clipvalue;
			}

		for (int i = 0; i < gxdim; i++)
			for (int j = 0; j < gydim; j++) {
				kgrid = i * gydim + j;
				if (cnt[kgrid] > 0) free(data[kgrid]);
			}
	}
/* -------------------------------------------------------------------------- */
	/***** do weighted mean or min/max gridding *****/
	else if (grid_mode == MBGRID_WEIGHTED_MEAN || grid_mode == MBGRID_MINIMUM_FILTER || grid_mode == MBGRID_MAXIMUM_FILTER) {

		if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(double), (void **)&norm, &error);

		if (error != MB_ERROR_NO_ERROR) {
			mb_error(verbose, error, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
			mb_memory_clear(verbose, &memclear_error);
			Return(error);
		}

		for (int i = 0; i < gxdim; i++)
			for (int j = 0; j < gydim; j++) {
				kgrid = i * gydim + j;
				grid[kgrid] = 0.0; norm[kgrid] = 0.0; sigma[kgrid] = 0.0; firsttime[kgrid] = 0.0; num[kgrid] = 0; cnt[kgrid] = 0;
			}

		ndata = 0;
		if (mb_datalist_open(verbose, &datalist, filelist, look_processed, &error) != MB_SUCCESS) {
			error = MB_ERROR_OPEN_FAIL;
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open data list file: %s\n", filelist);
			mb_memory_clear(verbose, &memclear_error);
			Return(error);
		}
		while (mb_datalist_read3(verbose, datalist, &pstatus, path, ppath, &astatus, apath, dpath, &format, &file_weight, &error) == MB_SUCCESS) {
			ndatafile = 0;
			if (format > 0 && path[0] != '#') {
				if (pstatus == MB_PROCESSED_USE) strcpy(file, ppath); else strcpy(file, path);
				rformat = format;
				strcpy(rfile, file);
				status = mb_check_info(verbose, rfile, lonflip, bounds, &file_in_bounds, &error);
				if (status == MB_FAILURE) { file_in_bounds = true; status = MB_SUCCESS; error = MB_ERROR_NO_ERROR; }

				bool first = true;
				double dmin = 0.0, dmax = 0.0;
				if (file_in_bounds) {
					if (datatype == MBGRID_DATA_TOPOGRAPHY || datatype == MBGRID_DATA_BATHYMETRY)
						mb_get_fbt(verbose, rfile, &rformat, &error);
					if (mb_read_init_altnav(verbose, rfile, rformat, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, astatus, apath, &mbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
						mb_error(verbose, error, &message);
						GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error returned from function <mb_read_init_altnav>:\n%s\n", message);
						mb_memory_clear(verbose, &memclear_error);
						Return(error);
					}

					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlon, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlat, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslon, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslat, &error);

					if (error != MB_ERROR_NO_ERROR) {
						mb_error(verbose, error, &message);
						GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
						mb_memory_clear(verbose, &memclear_error);
						Return(error);
					}

					while (error <= MB_ERROR_NO_ERROR) {
						status = mb_read(verbose, mbio_ptr, &kind, &rpings, time_i, &time_d, &navlon, &navlat, &speed, &heading, &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp, bathlon, bathlat, ss, sslon, sslat, comment, &error);
						if (error == MB_ERROR_TIME_GAP) { error = MB_ERROR_NO_ERROR; status = MB_SUCCESS; }

						if (verbose >= 2) {
							GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  Ping read in program <%s>\n", THIS_MODULE_NAME);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       kind:           %d\n", kind);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       beams_bath:     %d\n", beams_bath);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       beams_amp:      %d\n", beams_amp);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       pixels_ss:      %d\n", pixels_ss);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       error:          %d\n", error);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       status:         %d\n", status);
						}

						if (shift_mode == MBGRID_SHIFT_DATA) {
							navlon += shift_lon; navlat += shift_lat;
							for (ib = 0; ib < beams_bath; ib++)
								if (mb_beam_ok(beamflag[ib])) { bathlon[ib] += shift_lon; bathlat[ib] += shift_lat; }
							for (ib = 0; ib < pixels_ss; ib++)
								if (ss[ib] > MB_SIDESCAN_NULL) { sslon[ib] += shift_lon; sslat[ib] += shift_lat; }
						}

						if ((datatype == MBGRID_DATA_BATHYMETRY || datatype == MBGRID_DATA_TOPOGRAPHY) && error == MB_ERROR_NO_ERROR) {
							if (use_projection) {
								for (ib = 0; ib < beams_bath; ib++)
									if (mb_beam_ok(beamflag[ib]))
										mb_proj_forward(verbose, pjptr, bathlon[ib], bathlat[ib], &bathlon[ib], &bathlat[ib], &error);
							}
							for (ib = 0; ib < beams_bath; ib++)
								if (mb_beam_ok(beamflag[ib])) {
									ix = (int)((bathlon[ib] - wbnd[0] + 0.5 * dx) / dx);
									iy = (int)((bathlat[ib] - wbnd[2] + 0.5 * dy) / dy);

									time_ok = true;
									if (check_time) {
										if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
											kgrid = ix * gydim + iy;
											if (firsttime[kgrid] <= 0.0) { firsttime[kgrid] = time_d; time_ok = true; }
											else if (fabs(time_d - firsttime[kgrid]) > timediff) {
												if (first_in_stays) time_ok = false;
												else {
													time_ok = true;
													firsttime[kgrid] = time_d;
													ndata -= cnt[kgrid]; ndatafile -= cnt[kgrid];
													norm[kgrid] = 0.0; grid[kgrid] = 0.0; sigma[kgrid] = 0.0; num[kgrid] = 0; cnt[kgrid] = 0;
												}
											}
										}
									}

									if (grid_mode == MBGRID_WEIGHTED_MEAN && ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim && time_ok) {
										ix1 = MAX(ix - xtradim, 0);
										ix2 = MIN(ix + xtradim, gxdim - 1);
										iy1 = MAX(iy - xtradim, 0);
										iy2 = MIN(iy + xtradim, gydim - 1);
										for (int ii = ix1; ii <= ix2; ii++)
											for (int jj = iy1; jj <= iy2; jj++) {
												kgrid = ii * gydim + jj;
												xx = wbnd[0] + ii * dx - bathlon[ib];
												yy = wbnd[2] + jj * dy - bathlat[ib];
												weight = file_weight * exp(-(xx * xx + yy * yy) * factor);
												norm[kgrid] += weight;
												grid[kgrid] += weight * topofactor * bath[ib];
												sigma[kgrid] += weight * topofactor * topofactor * bath[ib] * bath[ib];
												num[kgrid]++;
												if (ii == ix && jj == iy) cnt[kgrid]++;
											}
										ndata++; ndatafile++;
										if (first) { first = false; dmin = topofactor * bath[ib]; dmax = topofactor * bath[ib]; }
										else { dmin = MIN(topofactor * bath[ib], dmin); dmax = MAX(topofactor * bath[ib], dmax); }
									} else if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim && time_ok) {
										kgrid = ix * gydim + iy;
										if ((num[kgrid] > 0 && grid_mode == MBGRID_MINIMUM_FILTER && grid[kgrid] > topofactor * bath[ib]) ||
											(num[kgrid] > 0 && grid_mode == MBGRID_MAXIMUM_FILTER && grid[kgrid] < topofactor * bath[ib]) ||
											num[kgrid] <= 0) {
											norm[kgrid] = 1.0;
											grid[kgrid] = topofactor * bath[ib];
											sigma[kgrid] = topofactor * topofactor * bath[ib] * bath[ib];
											num[kgrid] = 1; cnt[kgrid] = 1;
										}
										ndata++; ndatafile++;
										if (first) { first = false; dmin = topofactor * bath[ib]; dmax = topofactor * bath[ib]; }
										else { dmin = MIN(topofactor * bath[ib], dmin); dmax = MAX(topofactor * bath[ib], dmax); }
									}
								}
						} else if (datatype == MBGRID_DATA_AMPLITUDE && error == MB_ERROR_NO_ERROR) {
							if (use_projection) {
								for (ib = 0; ib < beams_amp; ib++)
									if (mb_beam_ok(beamflag[ib]))
										mb_proj_forward(verbose, pjptr, bathlon[ib], bathlat[ib], &bathlon[ib], &bathlat[ib], &error);
							}
							for (ib = 0; ib < beams_amp; ib++)
								if (mb_beam_ok(beamflag[ib])) {
									ix = (int)((bathlon[ib] - wbnd[0] + 0.5 * dx) / dx);
									iy = (int)((bathlat[ib] - wbnd[2] + 0.5 * dy) / dy);

									time_ok = true;
									if (check_time) {
										if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
											kgrid = ix * gydim + iy;
											if (firsttime[kgrid] <= 0.0) { firsttime[kgrid] = time_d; time_ok = true; }
											else if (fabs(time_d - firsttime[kgrid]) > timediff) {
												if (first_in_stays) time_ok = false;
												else {
													time_ok = true; firsttime[kgrid] = time_d;
													ndata -= cnt[kgrid]; ndatafile -= cnt[kgrid];
													norm[kgrid] = 0.0; grid[kgrid] = 0.0; sigma[kgrid] = 0.0; num[kgrid] = 0; cnt[kgrid] = 0;
												}
											}
										}
									}

									if (grid_mode == MBGRID_WEIGHTED_MEAN && ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim && time_ok) {
										ix1 = MAX(ix - xtradim, 0);
										ix2 = MIN(ix + xtradim, gxdim - 1);
										iy1 = MAX(iy - xtradim, 0);
										iy2 = MIN(iy + xtradim, gydim - 1);
										for (int ii = ix1; ii <= ix2; ii++)
											for (int jj = iy1; jj <= iy2; jj++) {
												kgrid = ii * gydim + jj;
												xx = wbnd[0] + ii * dx - bathlon[ib];
												yy = wbnd[2] + jj * dy - bathlat[ib];
												weight = file_weight * exp(-(xx * xx + yy * yy) * factor);
												norm[kgrid] += weight;
												grid[kgrid] += weight * amp[ib];
												sigma[kgrid] += weight * amp[ib] * amp[ib];
												num[kgrid]++;
												if (ii == ix && jj == iy) cnt[kgrid]++;
											}
										ndata++; ndatafile++;
										if (first) { first = false; dmin = amp[ib]; dmax = amp[ib]; }
										else { dmin = MIN(amp[ib], dmin); dmax = MAX(amp[ib], dmax); }
									} else if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim && time_ok) {
										kgrid = ix * gydim + iy;
										if ((num[kgrid] > 0 && grid_mode == MBGRID_MINIMUM_FILTER && grid[kgrid] > amp[ib]) ||
											(num[kgrid] > 0 && grid_mode == MBGRID_MAXIMUM_FILTER && grid[kgrid] < amp[ib]) ||
											num[kgrid] <= 0) {
											norm[kgrid] = 1.0;
											grid[kgrid] = amp[ib];
											sigma[kgrid] = amp[ib] * amp[ib];
											num[kgrid] = 1; cnt[kgrid] = 1;
										}
										ndata++; ndatafile++;
										if (first) { first = false; dmin = amp[ib]; dmax = amp[ib]; }
										else { dmin = MIN(amp[ib], dmin); dmax = MAX(amp[ib], dmax); }
									}
								}
						} else if (datatype == MBGRID_DATA_SIDESCAN && error == MB_ERROR_NO_ERROR) {
							if (use_projection) {
								for (ib = 0; ib < pixels_ss; ib++)
									if (ss[ib] > MB_SIDESCAN_NULL)
										mb_proj_forward(verbose, pjptr, sslon[ib], sslat[ib], &sslon[ib], &sslat[ib], &error);
							}
							for (ib = 0; ib < pixels_ss; ib++)
								if (ss[ib] > MB_SIDESCAN_NULL) {
									ix = (int)((sslon[ib] - wbnd[0] + 0.5 * dx) / dx);
									iy = (int)((sslat[ib] - wbnd[2] + 0.5 * dy) / dy);

									time_ok = true;
									if (check_time) {
										if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
											kgrid = ix * gydim + iy;
											if (firsttime[kgrid] <= 0.0) { firsttime[kgrid] = time_d; time_ok = true; }
											else if (fabs(time_d - firsttime[kgrid]) > timediff) {
												if (first_in_stays) time_ok = false;
												else {
													time_ok = true; firsttime[kgrid] = time_d;
													ndata -= cnt[kgrid]; ndatafile -= cnt[kgrid];
													norm[kgrid] = 0.0; grid[kgrid] = 0.0; sigma[kgrid] = 0.0; num[kgrid] = 0; cnt[kgrid] = 0;
												}
											}
										}
									}

									if (grid_mode == MBGRID_WEIGHTED_MEAN && ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim && time_ok) {
										ix1 = MAX(ix - xtradim, 0);
										ix2 = MIN(ix + xtradim, gxdim - 1);
										iy1 = MAX(iy - xtradim, 0);
										iy2 = MIN(iy + xtradim, gydim - 1);
										for (int ii = ix1; ii <= ix2; ii++)
											for (int jj = iy1; jj <= iy2; jj++) {
												kgrid = ii * gydim + jj;
												xx = wbnd[0] + ii * dx - sslon[ib];
												yy = wbnd[2] + jj * dy - sslat[ib];
												weight = file_weight * exp(-(xx * xx + yy * yy) * factor);
												norm[kgrid] += weight;
												grid[kgrid] += weight * ss[ib];
												sigma[kgrid] += weight * ss[ib] * ss[ib];
												num[kgrid]++;
												if (ii == ix && jj == iy) cnt[kgrid]++;
											}
										ndata++; ndatafile++;
									} else if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim && time_ok) {
										kgrid = ix * gydim + iy;
										if ((num[kgrid] > 0 && grid_mode == MBGRID_MINIMUM_FILTER && grid[kgrid] > ss[ib]) ||
											(num[kgrid] > 0 && grid_mode == MBGRID_MAXIMUM_FILTER && grid[kgrid] < ss[ib]) ||
											num[kgrid] <= 0) {
											norm[kgrid] = 1.0;
											grid[kgrid] = ss[ib];
											sigma[kgrid] = ss[ib] * ss[ib];
											num[kgrid] = 1; cnt[kgrid] = 1;
										}
										ndata++; ndatafile++;
									}
								}
						}
					}
					mb_close(verbose, &mbio_ptr, &error);
					status = MB_SUCCESS; error = MB_ERROR_NO_ERROR;
				}
				if (verbose > 0 || file_in_bounds) {
					if (astatus == MB_ALTNAV_USE)
						GMT_Report(API, GMT_MSG_NORMAL, "%d data points processed in %s (minmax: %f %f) using nav from %s\n", ndatafile, rfile, dmin, dmax, apath);
					else
						GMT_Report(API, GMT_MSG_NORMAL, "%d data points processed in %s (minmax: %f %f)\n", ndatafile, rfile, dmin, dmax);
				}
				if (ndatafile > 0 && dfp != NULL) {
					if (pstatus == MB_PROCESSED_USE && astatus == MB_ALTNAV_USE)
						GMT_Report(API, GMT_MSG_NORMAL, "A:%s %d %f %s\n", path, format, file_weight, apath);
					else if (pstatus == MB_PROCESSED_USE)
						GMT_Report(API, GMT_MSG_NORMAL, "P:%s %d %f\n", path, format, file_weight);
					else
						GMT_Report(API, GMT_MSG_NORMAL, "R:%s %d %f\n", path, format, file_weight);
					fflush(dfp);
				}
			}
			else if (format == 0 && path[0] != '#') {
				if ((rfp = fopen(path, "r")) == NULL) {
					error = MB_ERROR_OPEN_FAIL;
					GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open lon,lat,value triples data file1: %s\n", path);
					mb_memory_clear(verbose, &memclear_error);
					Return(error);
				}
				bool first = true;
				double dmin = 0.0, dmax = 0.0;
				while (fscanf(rfp, "%lf %lf %lf", &tlon, &tlat, &tvalue) != EOF) {
					if (shift_mode == MBGRID_SHIFT_DATA) { tlon += shift_lon; tlat += shift_lat; }
					if (use_projection) mb_proj_forward(verbose, pjptr, tlon, tlat, &tlon, &tlat, &error);

					ix = (int)((tlon - wbnd[0] + 0.5 * dx) / dx);
					iy = (int)((tlat - wbnd[2] + 0.5 * dy) / dy);

					if (check_time) {
						if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
							kgrid = ix * gydim + iy;
							time_ok = (firsttime[kgrid] <= 0.0);
						} else time_ok = true;
					} else time_ok = true;

					if (grid_mode == MBGRID_WEIGHTED_MEAN && ix >= -xtradim && ix < gxdim + xtradim && iy >= -xtradim && iy < gydim + xtradim && time_ok) {
						ix1 = MAX(ix - xtradim, 0);
						ix2 = MIN(ix + xtradim, gxdim - 1);
						iy1 = MAX(iy - xtradim, 0);
						iy2 = MIN(iy + xtradim, gydim - 1);
						for (int ii = ix1; ii <= ix2; ii++)
							for (int jj = iy1; jj <= iy2; jj++) {
								kgrid = ii * gydim + jj;
								xx = wbnd[0] + ii * dx - tlon;
								yy = wbnd[2] + jj * dy - tlat;
								weight = file_weight * exp(-(xx * xx + yy * yy) * factor);
								norm[kgrid] += weight;
								grid[kgrid] += weight * topofactor * tvalue;
								sigma[kgrid] += weight * topofactor * topofactor * tvalue * tvalue;
								num[kgrid]++;
								if (ii == ix && jj == iy) cnt[kgrid]++;
							}
						ndata++; ndatafile++;
					} else if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim && time_ok) {
						kgrid = ix * gydim + iy;
						if ((num[kgrid] > 0 && grid_mode == MBGRID_MINIMUM_FILTER && grid[kgrid] > topofactor * tvalue) ||
							(num[kgrid] > 0 && grid_mode == MBGRID_MAXIMUM_FILTER && grid[kgrid] < topofactor * tvalue) ||
							num[kgrid] <= 0) {
							norm[kgrid] = 1.0;
							grid[kgrid] = topofactor * tvalue;
							sigma[kgrid] = topofactor * topofactor * tvalue * tvalue;
							num[kgrid] = 1; cnt[kgrid] = 1;
						}
						ndata++; ndatafile++;
					}
					(void)dmin; (void)dmax; (void)first;
				}
				fclose(rfp);
				status = MB_SUCCESS; error = MB_ERROR_NO_ERROR;
				if (verbose > 0 || file_in_bounds) {
					if (astatus == MB_ALTNAV_USE)
						GMT_Report(API, GMT_MSG_NORMAL, "%d data points processed in %s (minmax: %f %f) using nav from %s\n", ndatafile, rfile, dmin, dmax, apath);
					else
						GMT_Report(API, GMT_MSG_NORMAL, "%d data points processed in %s (minmax: %f %f)\n", ndatafile, rfile, dmin, dmax);
				}
				if (ndatafile > 0 && dfp != NULL) {
					if (pstatus == MB_PROCESSED_USE && astatus == MB_ALTNAV_USE)
						GMT_Report(API, GMT_MSG_NORMAL, "A:%s %d %f %s\n", path, format, file_weight, apath);
					else if (pstatus == MB_PROCESSED_USE)
						GMT_Report(API, GMT_MSG_NORMAL, "P:%s %d %f\n", path, format, file_weight);
					else
						GMT_Report(API, GMT_MSG_NORMAL, "R:%s %d %f\n", path, format, file_weight);
					fflush(dfp);
				}
			}
		}
		if (datalist != NULL) mb_datalist_close(verbose, &datalist, &error);
		GMT_Report(API, GMT_MSG_NORMAL, "\n%d total data points processed\n", ndata);

		if (dfp != NULL) { fclose(dfp); dfp = NULL; }

		if (verbose >= 1) GMT_Report(API, GMT_MSG_NORMAL, "\nMaking raw grid...\n");
		for (int i = 0; i < gxdim; i++)
			for (int j = 0; j < gydim; j++) {
				kgrid = i * gydim + j;
				if (cnt[kgrid] > 0) {
					grid[kgrid] /= norm[kgrid];
					factor = sigma[kgrid] / norm[kgrid] - grid[kgrid] * grid[kgrid];
					sigma[kgrid] = sqrt(fabs(factor));
					nbinset++;
				} else { grid[kgrid] = clipvalue; sigma[kgrid] = 0.0; }
			}
	}
/* -------------------------------------------------------------------------- */
	/***** do minimum weighted mean or maximum weighted mean gridding *****/
	else if (grid_mode == MBGRID_MINIMUM_WEIGHTED_MEAN || grid_mode == MBGRID_MAXIMUM_WEIGHTED_MEAN) {

		if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(double), (void **)&norm, &error);
		if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(double), (void **)&minormax, &error);

		if (error != MB_ERROR_NO_ERROR) {
			mb_error(verbose, error, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
			mb_memory_clear(verbose, &memclear_error);
			Return(error);
		}

		for (int i = 0; i < gxdim; i++)
			for (int j = 0; j < gydim; j++) {
				kgrid = i * gydim + j;
				grid[kgrid] = 0.0; norm[kgrid] = 0.0; sigma[kgrid] = 0.0; minormax[kgrid] = 0.0;
				firsttime[kgrid] = 0.0; num[kgrid] = 0; cnt[kgrid] = 0;
			}

		/* First pass: find min/max in each cell */
		ndata = 0;
		if (mb_datalist_open(verbose, &datalist, filelist, look_processed, &error) != MB_SUCCESS) {
			error = MB_ERROR_OPEN_FAIL;
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open data list file: %s\n", filelist);
			mb_memory_clear(verbose, &memclear_error);
			Return(error);
		}
		while (mb_datalist_read3(verbose, datalist, &pstatus, path, ppath, &astatus, apath, dpath, &format, &file_weight, &error) == MB_SUCCESS) {
			ndatafile = 0;
			if (format > 0 && path[0] != '#') {
				if (pstatus == MB_PROCESSED_USE) strcpy(file, ppath); else strcpy(file, path);
				rformat = format;
				strcpy(rfile, file);
				status = mb_check_info(verbose, rfile, lonflip, bounds, &file_in_bounds, &error);
				if (status == MB_FAILURE) { file_in_bounds = true; status = MB_SUCCESS; error = MB_ERROR_NO_ERROR; }

				bool first = true;
				double dmin = 0.0, dmax = 0.0;
				if (file_in_bounds) {
					if (datatype == MBGRID_DATA_TOPOGRAPHY || datatype == MBGRID_DATA_BATHYMETRY)
						mb_get_fbt(verbose, rfile, &rformat, &error);
					if (mb_read_init_altnav(verbose, rfile, rformat, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, astatus, apath, &mbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
						mb_error(verbose, error, &message);
						GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error returned from function <mb_read_init_altnav>:\n%s\n", message);
						mb_memory_clear(verbose, &memclear_error);
						Return(error);
					}
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlon, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlat, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslon, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslat, &error);

					if (error != MB_ERROR_NO_ERROR) {
						mb_error(verbose, error, &message);
						GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
						mb_memory_clear(verbose, &memclear_error);
						Return(error);
					}

					while (error <= MB_ERROR_NO_ERROR) {
						status = mb_read(verbose, mbio_ptr, &kind, &rpings, time_i, &time_d, &navlon, &navlat, &speed, &heading, &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp, bathlon, bathlat, ss, sslon, sslat, comment, &error);
						if (error == MB_ERROR_TIME_GAP) { error = MB_ERROR_NO_ERROR; status = MB_SUCCESS; }

						if (verbose >= 2) {
							GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  Ping read in program <%s>\n", THIS_MODULE_NAME);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       kind:           %d\n", kind);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       beams_bath:     %d\n", beams_bath);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       beams_amp:      %d\n", beams_amp);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       pixels_ss:      %d\n", pixels_ss);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       error:          %d\n", error);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       status:         %d\n", status);
						}

						if (shift_mode == MBGRID_SHIFT_DATA) {
							navlon += shift_lon; navlat += shift_lat;
							for (ib = 0; ib < beams_bath; ib++)
								if (mb_beam_ok(beamflag[ib])) { bathlon[ib] += shift_lon; bathlat[ib] += shift_lat; }
							for (ib = 0; ib < pixels_ss; ib++)
								if (ss[ib] > MB_SIDESCAN_NULL) { sslon[ib] += shift_lon; sslat[ib] += shift_lat; }
						}

						if ((datatype == MBGRID_DATA_BATHYMETRY || datatype == MBGRID_DATA_TOPOGRAPHY) && error == MB_ERROR_NO_ERROR) {
							if (use_projection)
								for (ib = 0; ib < beams_bath; ib++)
									if (mb_beam_ok(beamflag[ib]))
										mb_proj_forward(verbose, pjptr, bathlon[ib], bathlat[ib], &bathlon[ib], &bathlat[ib], &error);
							for (ib = 0; ib < beams_bath; ib++)
								if (mb_beam_ok(beamflag[ib])) {
									ix = (int)((bathlon[ib] - wbnd[0] + 0.5 * dx) / dx);
									iy = (int)((bathlat[ib] - wbnd[2] + 0.5 * dy) / dy);
									time_ok = true;
									if (check_time) {
										if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
											kgrid = ix * gydim + iy;
											if (firsttime[kgrid] <= 0.0) { firsttime[kgrid] = time_d; time_ok = true; }
											else if (fabs(time_d - firsttime[kgrid]) > timediff) {
												if (first_in_stays) time_ok = false;
												else {
													time_ok = true; firsttime[kgrid] = time_d;
													ndata -= cnt[kgrid]; ndatafile -= cnt[kgrid];
													norm[kgrid] = 0.0; grid[kgrid] = 0.0; sigma[kgrid] = 0.0; num[kgrid] = 0; cnt[kgrid] = 0;
												}
											}
										}
									}
									if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim && time_ok) {
										kgrid = ix * gydim + iy;
										if (cnt[kgrid] <= 0 ||
											(grid_mode == MBGRID_MINIMUM_WEIGHTED_MEAN && minormax[kgrid] > topofactor * bath[ib]) ||
											(grid_mode == MBGRID_MAXIMUM_WEIGHTED_MEAN && minormax[kgrid] < topofactor * bath[ib])) {
											minormax[kgrid] = topofactor * bath[ib];
											cnt[kgrid]++;
										}
										ndata++; ndatafile++;
										if (first) { first = false; dmin = topofactor * bath[ib]; dmax = topofactor * bath[ib]; }
										else { dmin = MIN(topofactor * bath[ib], dmin); dmax = MAX(topofactor * bath[ib], dmax); }
									}
								}
						} else if (datatype == MBGRID_DATA_AMPLITUDE && error == MB_ERROR_NO_ERROR) {
							if (use_projection)
								for (ib = 0; ib < beams_amp; ib++)
									if (mb_beam_ok(beamflag[ib]))
										mb_proj_forward(verbose, pjptr, bathlon[ib], bathlat[ib], &bathlon[ib], &bathlat[ib], &error);
							for (ib = 0; ib < beams_amp; ib++)
								if (mb_beam_ok(beamflag[ib])) {
									ix = (int)((bathlon[ib] - wbnd[0] + 0.5 * dx) / dx);
									iy = (int)((bathlat[ib] - wbnd[2] + 0.5 * dy) / dy);
									time_ok = true;
									if (check_time) {
										if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
											kgrid = ix * gydim + iy;
											if (firsttime[kgrid] <= 0.0) { firsttime[kgrid] = time_d; time_ok = true; }
											else if (fabs(time_d - firsttime[kgrid]) > timediff) {
												if (first_in_stays) time_ok = false;
												else {
													time_ok = true; firsttime[kgrid] = time_d;
													ndata -= cnt[kgrid]; ndatafile -= cnt[kgrid];
													norm[kgrid] = 0.0; grid[kgrid] = 0.0; sigma[kgrid] = 0.0; num[kgrid] = 0; cnt[kgrid] = 0;
												}
											}
										}
									}
									if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim && time_ok) {
										kgrid = ix * gydim + iy;
										if (cnt[kgrid] <= 0 ||
											(grid_mode == MBGRID_MINIMUM_WEIGHTED_MEAN && minormax[kgrid] > amp[ib]) ||
											(grid_mode == MBGRID_MAXIMUM_WEIGHTED_MEAN && minormax[kgrid] < amp[ib])) {
											minormax[kgrid] = amp[ib]; cnt[kgrid]++;
										}
										ndata++; ndatafile++;
										if (first) { first = false; dmin = amp[ib]; dmax = amp[ib]; }
										else { dmin = MIN(amp[ib], dmin); dmax = MAX(amp[ib], dmax); }
									}
								}
						} else if (datatype == MBGRID_DATA_SIDESCAN && error == MB_ERROR_NO_ERROR) {
							if (use_projection)
								for (ib = 0; ib < pixels_ss; ib++)
									if (ss[ib] > MB_SIDESCAN_NULL)
										mb_proj_forward(verbose, pjptr, sslon[ib], sslat[ib], &sslon[ib], &sslat[ib], &error);
							for (ib = 0; ib < pixels_ss; ib++)
								if (ss[ib] > MB_SIDESCAN_NULL) {
									ix = (int)((sslon[ib] - wbnd[0] + 0.5 * dx) / dx);
									iy = (int)((sslat[ib] - wbnd[2] + 0.5 * dy) / dy);
									time_ok = true;
									if (check_time) {
										if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
											kgrid = ix * gydim + iy;
											if (firsttime[kgrid] <= 0.0) { firsttime[kgrid] = time_d; time_ok = true; }
											else if (fabs(time_d - firsttime[kgrid]) > timediff) {
												if (first_in_stays) time_ok = false;
												else {
													time_ok = true; firsttime[kgrid] = time_d;
													ndata -= cnt[kgrid]; ndatafile -= cnt[kgrid];
													norm[kgrid] = 0.0; grid[kgrid] = 0.0; sigma[kgrid] = 0.0; num[kgrid] = 0; cnt[kgrid] = 0;
												}
											}
										}
									}
									if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim && time_ok) {
										kgrid = ix * gydim + iy;
										if (cnt[kgrid] <= 0 ||
											(grid_mode == MBGRID_MINIMUM_WEIGHTED_MEAN && minormax[kgrid] > ss[ib]) ||
											(grid_mode == MBGRID_MAXIMUM_WEIGHTED_MEAN && minormax[kgrid] < ss[ib])) {
											minormax[kgrid] = ss[ib]; cnt[kgrid]++;
										}
										ndata++; ndatafile++;
										if (first) { first = false; dmin = ss[ib]; dmax = ss[ib]; }
										else { dmin = MIN(ss[ib], dmin); dmax = MAX(ss[ib], dmax); }
									}
								}
						}
					}
					mb_close(verbose, &mbio_ptr, &error);
					status = MB_SUCCESS; error = MB_ERROR_NO_ERROR;
				}
				if (verbose > 0 || file_in_bounds) {
					if (astatus == MB_ALTNAV_USE)
						GMT_Report(API, GMT_MSG_NORMAL, "%d data points processed in %s (minmax: %f %f) using nav from %s\n", ndatafile, rfile, dmin, dmax, apath);
					else
						GMT_Report(API, GMT_MSG_NORMAL, "%d data points processed in %s (minmax: %f %f)\n", ndatafile, rfile, dmin, dmax);
				}
				if (ndatafile > 0 && dfp != NULL) {
					if (pstatus == MB_PROCESSED_USE && astatus == MB_ALTNAV_USE)
						GMT_Report(API, GMT_MSG_NORMAL, "A:%s %d %f %s\n", path, format, file_weight, apath);
					else if (pstatus == MB_PROCESSED_USE)
						GMT_Report(API, GMT_MSG_NORMAL, "P:%s %d %f\n", path, format, file_weight);
					else
						GMT_Report(API, GMT_MSG_NORMAL, "R:%s %d %f\n", path, format, file_weight);
					fflush(dfp);
				}
			}
		}
		if (datalist != NULL) mb_datalist_close(verbose, &datalist, &error);
		GMT_Report(API, GMT_MSG_NORMAL, "\n%d total data points processed\n", ndata);
		if (dfp != NULL) { fclose(dfp); dfp = NULL; }

		/* Second pass: accumulate values within threshold */
		for (int i = 0; i < gxdim; i++)
			for (int j = 0; j < gydim; j++) { kgrid = i * gydim + j; cnt[kgrid] = 0; }

		GMT_Report(API, GMT_MSG_NORMAL, "\nDoing second pass to generate final grid...\n");
		ndata = 0;
		if (mb_datalist_open(verbose, &datalist, dfile, look_processed, &error) != MB_SUCCESS) {
			error = MB_ERROR_OPEN_FAIL;
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open data list file: %s\n", filelist);
			mb_memory_clear(verbose, &memclear_error);
			Return(error);
		}
		while (mb_datalist_read3(verbose, datalist, &pstatus, path, ppath, &astatus, apath, dpath, &format, &file_weight, &error) == MB_SUCCESS) {
			ndatafile = 0;
			if (format > 0 && path[0] != '#') {
				if (pstatus == MB_PROCESSED_USE) strcpy(file, ppath); else strcpy(file, path);
				rformat = format;
				strcpy(rfile, file);
				status = mb_check_info(verbose, rfile, lonflip, bounds, &file_in_bounds, &error);
				if (status == MB_FAILURE) { file_in_bounds = true; status = MB_SUCCESS; error = MB_ERROR_NO_ERROR; }

				bool first = true;
				double dmin = 0.0, dmax = 0.0;
				if (file_in_bounds) {
					if (datatype == MBGRID_DATA_TOPOGRAPHY || datatype == MBGRID_DATA_BATHYMETRY)
						mb_get_fbt(verbose, rfile, &rformat, &error);
					if (mb_read_init_altnav(verbose, rfile, rformat, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, astatus, apath, &mbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
						mb_error(verbose, error, &message);
						GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error returned from function <mb_read_init_altnav>:\n%s\n", message);
						mb_memory_clear(verbose, &memclear_error);
						Return(error);
					}
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlon, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlat, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslon, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslat, &error);

					if (error != MB_ERROR_NO_ERROR) {
						mb_error(verbose, error, &message);
						GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
						mb_memory_clear(verbose, &memclear_error);
						Return(error);
					}

					while (error <= MB_ERROR_NO_ERROR) {
						status = mb_read(verbose, mbio_ptr, &kind, &rpings, time_i, &time_d, &navlon, &navlat, &speed, &heading, &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp, bathlon, bathlat, ss, sslon, sslat, comment, &error);
						if (error == MB_ERROR_TIME_GAP) { error = MB_ERROR_NO_ERROR; status = MB_SUCCESS; }

						if (verbose >= 2) {
							GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  Ping read in program <%s>\n", THIS_MODULE_NAME);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       kind:           %d\n", kind);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       beams_bath:     %d\n", beams_bath);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       beams_amp:      %d\n", beams_amp);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       pixels_ss:      %d\n", pixels_ss);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       error:          %d\n", error);
							GMT_Report(API, GMT_MSG_NORMAL, "dbg2       status:         %d\n", status);
						}

						if (shift_mode == MBGRID_SHIFT_DATA) {
							navlon += shift_lon; navlat += shift_lat;
							for (ib = 0; ib < beams_bath; ib++)
								if (mb_beam_ok(beamflag[ib])) { bathlon[ib] += shift_lon; bathlat[ib] += shift_lat; }
							for (ib = 0; ib < pixels_ss; ib++)
								if (ss[ib] > MB_SIDESCAN_NULL) { sslon[ib] += shift_lon; sslat[ib] += shift_lat; }
						}

						if ((datatype == MBGRID_DATA_BATHYMETRY || datatype == MBGRID_DATA_TOPOGRAPHY) && error == MB_ERROR_NO_ERROR) {
							if (use_projection)
								for (ib = 0; ib < beams_bath; ib++)
									if (mb_beam_ok(beamflag[ib]))
										mb_proj_forward(verbose, pjptr, bathlon[ib], bathlat[ib], &bathlon[ib], &bathlat[ib], &error);
							for (ib = 0; ib < beams_bath; ib++)
								if (mb_beam_ok(beamflag[ib])) {
									ix = (int)((bathlon[ib] - wbnd[0] + 0.5 * dx) / dx);
									iy = (int)((bathlat[ib] - wbnd[2] + 0.5 * dy) / dy);
									time_ok = true;
									if (check_time) {
										if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
											kgrid = ix * gydim + iy;
											if (firsttime[kgrid] <= 0.0) { firsttime[kgrid] = time_d; time_ok = true; }
											else if (fabs(time_d - firsttime[kgrid]) > timediff) {
												if (first_in_stays) time_ok = false;
												else { time_ok = true; firsttime[kgrid] = time_d; ndata -= cnt[kgrid]; ndatafile -= cnt[kgrid]; norm[kgrid] = 0.0; grid[kgrid] = 0.0; sigma[kgrid] = 0.0; num[kgrid] = 0; cnt[kgrid] = 0; }
											}
										}
									}
									if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim && time_ok) {
										kgrid = ix * gydim + iy;
										if (fabs(minormax[kgrid] - topofactor * bath[ib]) < minormax_weighted_mean_threshold) {
											ix1 = MAX(ix - xtradim, 0); ix2 = MIN(ix + xtradim, gxdim - 1);
											iy1 = MAX(iy - xtradim, 0); iy2 = MIN(iy + xtradim, gydim - 1);
											for (int ii = ix1; ii <= ix2; ii++)
												for (int jj = iy1; jj <= iy2; jj++) {
													kgrid = ii * gydim + jj;
													xx = wbnd[0] + ii * dx - bathlon[ib];
													yy = wbnd[2] + jj * dy - bathlat[ib];
													weight = file_weight * exp(-(xx * xx + yy * yy) * factor);
													norm[kgrid] += weight;
													grid[kgrid] += weight * topofactor * bath[ib];
													sigma[kgrid] += weight * topofactor * topofactor * bath[ib] * bath[ib];
													num[kgrid]++;
													if (ii == ix && jj == iy) cnt[kgrid]++;
												}
											ndata++; ndatafile++;
											if (first) { first = false; dmin = topofactor * bath[ib]; dmax = topofactor * bath[ib]; }
											else { dmin = MIN(topofactor * bath[ib], dmin); dmax = MAX(topofactor * bath[ib], dmax); }
										}
									}
								}
						} else if (datatype == MBGRID_DATA_AMPLITUDE && error == MB_ERROR_NO_ERROR) {
							if (use_projection)
								for (ib = 0; ib < beams_amp; ib++)
									if (mb_beam_ok(beamflag[ib]))
										mb_proj_forward(verbose, pjptr, bathlon[ib], bathlat[ib], &bathlon[ib], &bathlat[ib], &error);
							for (ib = 0; ib < beams_amp; ib++)
								if (mb_beam_ok(beamflag[ib])) {
									ix = (int)((bathlon[ib] - wbnd[0] + 0.5 * dx) / dx);
									iy = (int)((bathlat[ib] - wbnd[2] + 0.5 * dy) / dy);
									time_ok = true;
									if (check_time) {
										if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
											kgrid = ix * gydim + iy;
											if (firsttime[kgrid] <= 0.0) { firsttime[kgrid] = time_d; time_ok = true; }
											else if (fabs(time_d - firsttime[kgrid]) > timediff) {
												if (first_in_stays) time_ok = false;
												else { time_ok = true; firsttime[kgrid] = time_d; ndata -= cnt[kgrid]; ndatafile -= cnt[kgrid]; norm[kgrid] = 0.0; grid[kgrid] = 0.0; sigma[kgrid] = 0.0; num[kgrid] = 0; cnt[kgrid] = 0; }
											}
										}
									}
									if (grid_mode == MBGRID_WEIGHTED_MEAN && ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim && time_ok) {
										kgrid = ix * gydim + iy;
										if (fabs(minormax[kgrid] - amp[ib]) < minormax_weighted_mean_threshold) {
											ix1 = MAX(ix - xtradim, 0); ix2 = MIN(ix + xtradim, gxdim - 1);
											iy1 = MAX(iy - xtradim, 0); iy2 = MIN(iy + xtradim, gydim - 1);
											for (int ii = ix1; ii <= ix2; ii++)
												for (int jj = iy1; jj <= iy2; jj++) {
													kgrid = ii * gydim + jj;
													xx = wbnd[0] + ii * dx - bathlon[ib];
													yy = wbnd[2] + jj * dy - bathlat[ib];
													weight = file_weight * exp(-(xx * xx + yy * yy) * factor);
													norm[kgrid] += weight;
													grid[kgrid] += weight * amp[ib];
													sigma[kgrid] += weight * amp[ib] * amp[ib];
													num[kgrid]++;
													if (ii == ix && jj == iy) cnt[kgrid]++;
												}
											ndata++; ndatafile++;
											if (first) { first = false; dmin = amp[ib]; dmax = amp[ib]; }
											else { dmin = MIN(amp[ib], dmin); dmax = MAX(amp[ib], dmax); }
										}
									}
								}
						} else if (datatype == MBGRID_DATA_SIDESCAN && error == MB_ERROR_NO_ERROR) {
							if (use_projection)
								for (ib = 0; ib < pixels_ss; ib++)
									if (ss[ib] > MB_SIDESCAN_NULL)
										mb_proj_forward(verbose, pjptr, sslon[ib], sslat[ib], &sslon[ib], &sslat[ib], &error);
							for (ib = 0; ib < pixels_ss; ib++)
								if (ss[ib] > MB_SIDESCAN_NULL) {
									ix = (int)((sslon[ib] - wbnd[0] + 0.5 * dx) / dx);
									iy = (int)((sslat[ib] - wbnd[2] + 0.5 * dy) / dy);
									time_ok = true;
									if (check_time) {
										if (ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim) {
											kgrid = ix * gydim + iy;
											if (firsttime[kgrid] <= 0.0) { firsttime[kgrid] = time_d; time_ok = true; }
											else if (fabs(time_d - firsttime[kgrid]) > timediff) {
												if (first_in_stays) time_ok = false;
												else { time_ok = true; firsttime[kgrid] = time_d; ndata -= cnt[kgrid]; ndatafile -= cnt[kgrid]; norm[kgrid] = 0.0; grid[kgrid] = 0.0; sigma[kgrid] = 0.0; num[kgrid] = 0; cnt[kgrid] = 0; }
											}
										}
									}
									if (grid_mode == MBGRID_WEIGHTED_MEAN && ix >= 0 && ix < gxdim && iy >= 0 && iy < gydim && time_ok) {
										kgrid = ix * gydim + iy;
										if (fabs(minormax[kgrid] - ss[ib]) < minormax_weighted_mean_threshold) {
											ix1 = MAX(ix - xtradim, 0); ix2 = MIN(ix + xtradim, gxdim - 1);
											iy1 = MAX(iy - xtradim, 0); iy2 = MIN(iy + xtradim, gydim - 1);
											for (int ii = ix1; ii <= ix2; ii++)
												for (int jj = iy1; jj <= iy2; jj++) {
													kgrid = ii * gydim + jj;
													xx = wbnd[0] + ii * dx - bathlon[ib];
													yy = wbnd[2] + jj * dy - bathlat[ib];
													weight = file_weight * exp(-(xx * xx + yy * yy) * factor);
													norm[kgrid] += weight;
													grid[kgrid] += weight * ss[ib];
													sigma[kgrid] += weight * ss[ib] * ss[ib];
													num[kgrid]++;
													if (ii == ix && jj == iy) cnt[kgrid]++;
												}
											ndata++; ndatafile++;
											if (first) { first = false; dmin = ss[ib]; dmax = ss[ib]; }
											else { dmin = MIN(ss[ib], dmin); dmax = MAX(ss[ib], dmax); }
										}
									}
								}
						}
					}
					mb_close(verbose, &mbio_ptr, &error);
					status = MB_SUCCESS; error = MB_ERROR_NO_ERROR;
				}
				if (verbose > 0 || file_in_bounds) {
					if (astatus == MB_ALTNAV_USE)
						GMT_Report(API, GMT_MSG_NORMAL, "%d data points processed in %s (minmax: %f %f) using nav from %s\n", ndatafile, rfile, dmin, dmax, apath);
					else
						GMT_Report(API, GMT_MSG_NORMAL, "%d data points processed in %s (minmax: %f %f)\n", ndatafile, rfile, dmin, dmax);
				}
				if (ndatafile > 0 && dfp != NULL) {
					if (pstatus == MB_PROCESSED_USE && astatus == MB_ALTNAV_USE)
						GMT_Report(API, GMT_MSG_NORMAL, "A:%s %d %f %s\n", path, format, file_weight, apath);
					else if (pstatus == MB_PROCESSED_USE)
						GMT_Report(API, GMT_MSG_NORMAL, "P:%s %d %f\n", path, format, file_weight);
					else
						GMT_Report(API, GMT_MSG_NORMAL, "R:%s %d %f\n", path, format, file_weight);
					fflush(dfp);
				}
			}
		}
		if (datalist != NULL) mb_datalist_close(verbose, &datalist, &error);
		GMT_Report(API, GMT_MSG_NORMAL, "\n%d total data points processed\n", ndata);
		if (dfp != NULL) { fclose(dfp); dfp = NULL; }

		if (verbose >= 1) GMT_Report(API, GMT_MSG_NORMAL, "\nMaking raw grid...\n");
		for (int i = 0; i < gxdim; i++)
			for (int j = 0; j < gydim; j++) {
				kgrid = i * gydim + j;
				if (cnt[kgrid] > 0) {
					grid[kgrid] /= norm[kgrid];
					factor = sigma[kgrid] / norm[kgrid] - grid[kgrid] * grid[kgrid];
					sigma[kgrid] = sqrt(fabs(factor));
					nbinset++;
				} else { grid[kgrid] = clipvalue; sigma[kgrid] = 0.0; }
			}
	}
/* -------------------------------------------------------------------------- */
	/* if clip set do smooth interpolation */
	if (clipmode != MBGRID_INTERP_NONE && clip > 0 && nbinset > 0) {
		if (setborder) ndata = 2 * gxdim + 2 * gydim - 2;
		else ndata = 8;
		for (int i = 0; i < gxdim; i++)
			for (int j = 0; j < gydim; j++) {
				kgrid = i * gydim + j;
				if (grid[kgrid] < clipvalue) ndata++;
			}

#ifdef USESURFACE
		status = mb_mallocd(verbose, __FILE__, __LINE__, ndata * sizeof(float), (void **)&sxdata, &error);
		if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, ndata * sizeof(float), (void **)&sydata, &error);
		if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, ndata * sizeof(float), (void **)&szdata, &error);
		if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(float), (void **)&sgrid, &error);
		if (error != MB_ERROR_NO_ERROR) {
			mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating interpolation work arrays:\n%s\n", message);
			mb_memory_clear(verbose, &memclear_error);
			Return(error);
		}
		memset((char *)sgrid, 0, gxdim * gydim * sizeof(float));
		memset((char *)sxdata, 0, ndata * sizeof(float));
		memset((char *)sydata, 0, ndata * sizeof(float));
		memset((char *)szdata, 0, ndata * sizeof(float));

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

		if (setborder) {
			for (int i = 0; i < gxdim; i++) {
				int j = 0;
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
				int i = 0;
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

		GMT_Report(API, GMT_MSG_NORMAL, "\nDoing Surface spline interpolation with %d data points...\n", ndata);
		mb_surface(verbose, ndata, sxdata, sydata, szdata, (float)(gbnd[0] - bdata_origin_x), (float)(gbnd[1] - bdata_origin_x), (float)(gbnd[2] - bdata_origin_y), (float)(gbnd[3] - bdata_origin_y), dx, dy, tension, sgrid);
#else
		status = mb_mallocd(verbose, __FILE__, __LINE__, 3 * ndata * sizeof(float), (void **)&sdata, &error);
		if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(float), (void **)&sgrid, &error);
		if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, ndata * sizeof(float), (void **)&work1, &error);
		if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, ndata * sizeof(int), (void **)&work2, &error);
		if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, (gxdim + gydim) * sizeof(bool), (void **)&work3, &error);
		if (error != MB_ERROR_NO_ERROR) {
			mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating interpolation work arrays:\n%s\n", message);
			mb_memory_clear(verbose, &memclear_error);
			Return(error);
		}
		memset((char *)sgrid, 0, gxdim * gydim * sizeof(float));
		memset((char *)sdata, 0, 3 * ndata * sizeof(float));
		memset((char *)work1, 0, ndata * sizeof(float));
		memset((char *)work2, 0, ndata * sizeof(int));
		memset((char *)work3, 0, (gxdim + gydim) * sizeof(bool));

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

		{
			float cay = (float)tension;
			float xmin = (float)(wbnd[0] - 0.5 * dx - bdata_origin_x);
			float ymin = (float)(wbnd[2] - 0.5 * dy - bdata_origin_y);
			float ddx = (float)dx;
			float ddy = (float)dy;
			GMT_Report(API, GMT_MSG_NORMAL, "\nDoing Zgrid spline interpolation with %d data points...\n", ndata);
			if (clipmode == MBGRID_INTERP_ALL) clip = MAX(gxdim, gydim);
			mb_zgrid(sgrid, &gxdim, &gydim, &xmin, &ymin, &ddx, &ddy, sdata, &ndata, work1, work2, work3, &cay, &clip);
		}
#endif

		if (clipmode == MBGRID_INTERP_GAP)
			GMT_Report(API, GMT_MSG_NORMAL, "Applying spline interpolation to fill gaps of %d cells or less...\n", clip);
		else if (clipmode == MBGRID_INTERP_NEAR)
			GMT_Report(API, GMT_MSG_NORMAL, "Applying spline interpolation to fill %d cells from data...\n", clip);
		else if (clipmode == MBGRID_INTERP_ALL)
			GMT_Report(API, GMT_MSG_NORMAL, "Applying spline interpolation to fill all undefined cells in the grid...\n");

		bool *smask = NULL;
		if (mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(bool), (void **)&smask, &error) != MB_SUCCESS) {
			mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating interpolation work arrays:\n%s\n", message);
			mb_memory_clear(verbose, &memclear_error);
			Return(error);
		}
		memset((char *)smask, 0, (gxdim + gydim) * sizeof(bool));

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
						int dmask[9];
						for (int ii = 0; ii < 9; ii++) dmask[ii] = false;

						for (int ir = 0; ir <= clip && smask[kgrid] == false; ir++) {
							const int i1 = MAX(0, i - ir);
							const int i2 = MIN(gxdim - 1, i + ir);
							const int j1 = MAX(0, j - ir);
							const int j2 = MIN(gydim - 1, j + ir);

							int jj = j1;
							for (int ii = i1; ii <= i2 && smask[kgrid] == false; ii++) {
								if (grid[ii * gydim + jj] < clipvalue) {
									const double r = sqrt((double)((ii - i) * (ii - i) + (jj - j) * (jj - j)));
									const int iii = (int)rint((ii - i) / r + 1);
									const int jjj = (int)rint((jj - j) / r + 1);
									const int kkk = iii * 3 + jjj;
									dmask[kkk] = true;
									if ((dmask[0] && dmask[8]) || (dmask[3] && dmask[5]) || (dmask[6] && dmask[2]) || (dmask[1] && dmask[7]))
										smask[kgrid] = true;
								}
							}

							jj = j2;
							for (int ii = i1; ii <= i2 && smask[kgrid] == false; ii++) {
								if (grid[ii * gydim + jj] < clipvalue) {
									const double r = sqrt((double)((ii - i) * (ii - i) + (jj - j) * (jj - j)));
									const int iii = (int)rint((ii - i) / r + 1);
									const int jjj = (int)rint((jj - j) / r + 1);
									const int kkk = iii * 3 + jjj;
									dmask[kkk] = true;
									if ((dmask[0] && dmask[8]) || (dmask[3] && dmask[5]) || (dmask[6] && dmask[2]) || (dmask[1] && dmask[7]))
										smask[kgrid] = true;
								}
							}

							int ii_l = i1;
							for (jj = j1; jj <= j2 && smask[kgrid] == false; jj++) {
								if (grid[ii_l * gydim + jj] < clipvalue) {
									const double r = sqrt((double)((ii_l - i) * (ii_l - i) + (jj - j) * (jj - j)));
									const int iii = (int)rint((ii_l - i) / r + 1);
									const int jjj = (int)rint((jj - j) / r + 1);
									const int kkk = iii * 3 + jjj;
									dmask[kkk] = true;
									if ((dmask[0] && dmask[8]) || (dmask[3] && dmask[5]) || (dmask[6] && dmask[2]) || (dmask[1] && dmask[7]))
										smask[kgrid] = true;
								}
							}

							ii_l = i2;
							for (jj = j1; jj <= j2 && smask[kgrid] == false; jj++) {
								if (grid[ii_l * gydim + jj] < clipvalue) {
									const double r = sqrt((double)((ii_l - i) * (ii_l - i) + (jj - j) * (jj - j)));
									const int iii = (int)rint((ii_l - i) / r + 1);
									const int jjj = (int)rint((jj - j) / r + 1);
									const int kkk = iii * 3 + jjj;
									dmask[kkk] = true;
									if ((dmask[0] && dmask[8]) || (dmask[3] && dmask[5]) || (dmask[6] && dmask[2]) || (dmask[1] && dmask[7]))
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
		} else if (clipmode == MBGRID_INTERP_NEAR) {
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
						for (int ir = 0; ir <= clip && smask[kgrid] == false; ir++) {
							const int i1 = MAX(0, i - ir);
							const int i2 = MIN(gxdim - 1, i + ir);
							const int j1 = MAX(0, j - ir);
							const int j2 = MIN(gydim - 1, j + ir);
							int jj = j1;
							for (int ii = i1; ii <= i2 && smask[kgrid] == false; ii++)
								if (grid[ii * gydim + jj] < clipvalue) smask[kgrid] = true;
							jj = j2;
							for (int ii = i1; ii <= i2 && smask[kgrid] == false; ii++)
								if (grid[ii * gydim + jj] < clipvalue) smask[kgrid] = true;
							int ii_l = i1;
							for (jj = j1; jj <= j2 && smask[kgrid] == false; jj++)
								if (grid[ii_l * gydim + jj] < clipvalue) smask[kgrid] = true;
							ii_l = i2;
							for (jj = j1; jj <= j2 && smask[kgrid] == false; jj++)
								if (grid[ii_l * gydim + jj] < clipvalue) smask[kgrid] = true;
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
					if (smask[kgrid] == true) { grid[kgrid] = sgrid[kint]; nbinspline++; }
				}
		} else {
			for (int i = 0; i < gxdim; i++)
				for (int j = 0; j < gydim; j++) {
					kgrid = i * gydim + j;
#ifdef USESURFACE
					kint = i + (gydim - j - 1) * gxdim;
#else
					kint = i + j * gxdim;
#endif
					if (grid[kgrid] >= clipvalue && sgrid[kint] < zflag) { grid[kgrid] = sgrid[kint]; nbinspline++; }
				}
		}

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
	/* if grdrasterid set and background data previously read in, interpolate onto internal grid */
	if (grdrasterid != 0 && nbackground > 0) {

#ifdef USESURFACE
		status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(float), (void **)&sgrid, &error);
		if (error != MB_ERROR_NO_ERROR) {
			mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating background data array:\n%s\n", message);
			mb_memory_clear(verbose, &memclear_error);
			Return(error);
		}
		memset((char *)sgrid, 0, gxdim * gydim * sizeof(float));
#else
		status = mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(float), (void **)&sgrid, &error);
		if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, nbackground * sizeof(float), (void **)&work1, &error);
		if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, nbackground * sizeof(int), (void **)&work2, &error);
		if (status == MB_SUCCESS) status = mb_mallocd(verbose, __FILE__, __LINE__, (gxdim + gydim) * sizeof(int), (void **)&work3, &error);
		if (error != MB_ERROR_NO_ERROR) {
			mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating background interpolation work arrays:\n%s\n", message);
			mb_memory_clear(verbose, &memclear_error);
			Return(error);
		}
		memset((char *)sgrid, 0, gxdim * gydim * sizeof(float));
		memset((char *)work1, 0, nbackground * sizeof(float));
		memset((char *)work2, 0, nbackground * sizeof(int));
		memset((char *)work3, 0, (gxdim + gydim) * sizeof(int));
#endif

		GMT_Report(API, GMT_MSG_NORMAL, "\nDoing spline interpolation with %d background points...\n", nbackground);
#ifdef USESURFACE
		mb_surface(verbose, nbackground, bxdata, bydata, bzdata, (float)(wbnd[0] - bdata_origin_x), (float)(wbnd[1] - bdata_origin_x), (float)(wbnd[2] - bdata_origin_y), (float)(wbnd[3] - bdata_origin_y), dx, dy, tension, sgrid);
#else
		{
			float cay = (float)tension;
			float xmin = (float)(wbnd[0] - 0.5 * dx - bdata_origin_x);
			float ymin = (float)(wbnd[2] - 0.5 * dy - bdata_origin_y);
			float ddx = (float)dx;
			float ddy = (float)dy;
			clip = MAX(gxdim, gydim);
			GMT_Report(API, GMT_MSG_NORMAL, "\nDoing Zgrid spline interpolation with %d background points...\n", nbackground);
			mb_zgrid(sgrid, &gxdim, &gydim, &xmin, &ymin, &ddx, &ddy, bdata, &nbackground, work1, work2, work3, &cay, &clip);
		}
#endif

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
			if (zmin == zclip && grid[kgrid] < zclip) zmin = grid[kgrid];
			if (zmax == zclip && grid[kgrid] < zclip) zmax = grid[kgrid];
			if (grid[kgrid] < zmin && grid[kgrid] < zclip) zmin = grid[kgrid];
			if (grid[kgrid] > zmax && grid[kgrid] < zclip) zmax = grid[kgrid];
		}
	if (zmin == zclip) zmin = 0.0;
	if (zmax == zclip) zmax = 0.0;

	nmax = 0;
	for (int i = 0; i < gxdim; i++)
		for (int j = 0; j < gydim; j++) {
			kgrid = i * gydim + j;
			if (cnt[kgrid] > nmax) nmax = cnt[kgrid];
		}

	smin = 0.0;
	smax = 0.0;
	for (int i = 0; i < gxdim; i++)
		for (int j = 0; j < gydim; j++) {
			kgrid = i * gydim + j;
			if (smin == 0.0 && cnt[kgrid] > 0) smin = sigma[kgrid];
			if (smax == 0.0 && cnt[kgrid] > 0) smax = sigma[kgrid];
			if (sigma[kgrid] < smin && cnt[kgrid] > 0) smin = sigma[kgrid];
			if (sigma[kgrid] > smax && cnt[kgrid] > 0) smax = sigma[kgrid];
		}
	nbinzero = gxdim * gydim - nbinset - nbinspline - nbinbackground;
	GMT_Report(API, GMT_MSG_NORMAL, "\nTotal number of bins:            %d\n", gxdim * gydim);
	GMT_Report(API, GMT_MSG_NORMAL, "Bins set using data:             %d\n", nbinset);
	GMT_Report(API, GMT_MSG_NORMAL, "Bins set using interpolation:    %d\n", nbinspline);
	GMT_Report(API, GMT_MSG_NORMAL, "Bins set using background:       %d\n", nbinbackground);
	GMT_Report(API, GMT_MSG_NORMAL, "Bins not set:                    %d\n", nbinzero);
	GMT_Report(API, GMT_MSG_NORMAL, "Maximum number of data in a bin: %d\n", nmax);
	GMT_Report(API, GMT_MSG_NORMAL, "Minimum value: %10.2f   Maximum value: %10.2f\n", zmin, zmax);
	GMT_Report(API, GMT_MSG_NORMAL, "Minimum sigma: %10.5f   Maximum sigma: %10.5f\n", smin, smax);

	/* Apply shift to the output grid bounds if specified */
	if (shift_mode == MBGRID_SHIFT_BOUNDS && use_projection) {
		gbnd[0] += shift_x; gbnd[1] += shift_x;
		gbnd[2] += shift_y; gbnd[3] += shift_y;
	} else if (shift_mode == MBGRID_SHIFT_BOUNDS) {
		gbnd[0] += shift_x * mtodeglon; gbnd[1] += shift_x * mtodeglon;
		gbnd[2] += shift_y * mtodeglat; gbnd[3] += shift_y * mtodeglat;
	}

	/* write first output file */
	if (verbose > 0) GMT_Report(API, GMT_MSG_NORMAL, "\nOutputting results...\n");
	for (int i = 0; i < xdim; i++)
		for (int j = 0; j < ydim; j++) {
			kgrid = (i + offx) * gydim + (j + offy);
			kout = i * ydim + j;
			output[kout] = (float)grid[kgrid];
			if (gridkind != MBGRID_ASCII && gridkind != MBGRID_ARCASCII && grid[kgrid] >= clipvalue)
				output[kout] = outclipvalue;
		}

	if (gridkind == MBGRID_ASCII) {
		strcpy(ofile, fileroot); strcat(ofile, ".asc");
		status = write_ascii(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, &error);
	} else if (gridkind == MBGRID_ARCASCII) {
		strcpy(ofile, fileroot); strcat(ofile, ".asc");
		status = write_arcascii(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, outclipvalue, &error);
	} else if (gridkind == MBGRID_OLDGRD) {
		strcpy(ofile, fileroot); strcat(ofile, ".grd1");
		status = write_oldgrd(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, &error);
	} else if (gridkind == MBGRID_CDFGRD) {
		strcpy(ofile, fileroot); strcat(ofile, ".grd");
		status = mb_write_gmt_grd(verbose, ofile, output, outclipvalue, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], zmin, zmax, dx, dy, xlabel, ylabel, zlabel, title, projection_id, 0, NULL, &error);
	} else if (gridkind == MBGRID_GMTGRD) {
		snprintf(ofile, sizeof(ofile), "%s.grd%s", fileroot, gridkindstring);
		status = mb_write_gmt_grd(verbose, ofile, output, outclipvalue, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], zmin, zmax, dx, dy, xlabel, ylabel, zlabel, title, projection_id, 0, NULL, &error);
	}

	if (status != MB_SUCCESS) {
		mb_error(verbose, error, &message);
		GMT_Report(API, GMT_MSG_NORMAL, "\nError writing output file: %s\n%s\n", ofile, message);
		mb_memory_clear(verbose, &memclear_error);
		Return(error);
	}

	/* write density + sigma grids if requested */
	if (more) {
		for (int i = 0; i < xdim; i++)
			for (int j = 0; j < ydim; j++) {
				kgrid = (i + offx) * gydim + (j + offy);
				kout = i * ydim + j;
				output[kout] = (float)cnt[kgrid];
				if (output[kout] < 0.0) output[kout] = 0.0;
				if (gridkind != MBGRID_ASCII && gridkind != MBGRID_ARCASCII && cnt[kgrid] <= 0) output[kout] = outclipvalue;
			}
		if (gridkind == MBGRID_ASCII) {
			strcpy(ofile, fileroot); strcat(ofile, "_num.asc");
			status = write_ascii(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, &error);
		} else if (gridkind == MBGRID_ARCASCII) {
			strcpy(ofile, fileroot); strcat(ofile, "_num.asc");
			status = write_arcascii(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, outclipvalue, &error);
		} else if (gridkind == MBGRID_OLDGRD) {
			strcpy(ofile, fileroot); strcat(ofile, "_num.grd1");
			status = write_oldgrd(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, &error);
		} else if (gridkind == MBGRID_CDFGRD) {
			strcpy(ofile, fileroot); strcat(ofile, "_num.grd");
			status = mb_write_gmt_grd(verbose, ofile, output, outclipvalue, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], zmin, zmax, dx, dy, xlabel, ylabel, zlabel, title, projection_id, 0, NULL, &error);
		} else if (gridkind == MBGRID_GMTGRD) {
			snprintf(ofile, sizeof(ofile), "%s_num.grd%s", fileroot, gridkindstring);
			status = mb_write_gmt_grd(verbose, ofile, output, outclipvalue, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], zmin, zmax, dx, dy, xlabel, ylabel, zlabel, title, projection_id, 0, NULL, &error);
		}
		if (status != MB_SUCCESS) {
			mb_error(verbose, error, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nError writing output file: %s\n%s\n", ofile, message);
			mb_memory_clear(verbose, &memclear_error);
			Return(error);
		}

		for (int i = 0; i < xdim; i++)
			for (int j = 0; j < ydim; j++) {
				kgrid = (i + offx) * gydim + (j + offy);
				kout = i * ydim + j;
				output[kout] = (float)sigma[kgrid];
				if (output[kout] < 0.0) output[kout] = 0.0;
				if (gridkind != MBGRID_ASCII && gridkind != MBGRID_ARCASCII && cnt[kgrid] <= 0) output[kout] = outclipvalue;
			}
		if (gridkind == MBGRID_ASCII) {
			strcpy(ofile, fileroot); strcat(ofile, "_sd.asc");
			status = write_ascii(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, &error);
		} else if (gridkind == MBGRID_ARCASCII) {
			strcpy(ofile, fileroot); strcat(ofile, "_sd.asc");
			status = write_arcascii(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, outclipvalue, &error);
		} else if (gridkind == MBGRID_OLDGRD) {
			strcpy(ofile, fileroot); strcat(ofile, "_sd.grd1");
			status = write_oldgrd(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, &error);
		} else if (gridkind == MBGRID_CDFGRD) {
			strcpy(ofile, fileroot); strcat(ofile, "_sd.grd");
			status = mb_write_gmt_grd(verbose, ofile, output, outclipvalue, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], zmin, zmax, dx, dy, xlabel, ylabel, zlabel, title, projection_id, 0, NULL, &error);
		} else if (gridkind == MBGRID_GMTGRD) {
			snprintf(ofile, sizeof(ofile), "%s_sd.grd%s", fileroot, gridkindstring);
			status = mb_write_gmt_grd(verbose, ofile, output, outclipvalue, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], zmin, zmax, dx, dy, xlabel, ylabel, zlabel, title, projection_id, 0, NULL, &error);
		}
		if (status != MB_SUCCESS) {
			mb_error(verbose, error, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nError writing output file: %s\n%s\n", ofile, message);
			mb_memory_clear(verbose, &memclear_error);
			Return(error);
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
	if (minormax) mb_freed(verbose, __FILE__, __LINE__, (void **)&minormax, &error);

	if (use_projection) mb_proj_free(verbose, &pjptr, &error);

	/* run mbm_grdplot */
	if (gridkind == MBGRID_GMTGRD) {
		strcpy(ofile, fileroot); strcat(ofile, ".grd");
		if (datatype == MBGRID_DATA_BATHYMETRY) {
			snprintf(plot_cmd, sizeof(plot_cmd), "mbm_grdplot -I%s%s -G1 -C -D -V -L\"File %s - %s:%s\"", ofile, gridkindstring, ofile, title, zlabel);
		} else if (datatype == MBGRID_DATA_TOPOGRAPHY) {
			snprintf(plot_cmd, sizeof(plot_cmd), "mbm_grdplot -I%s%s -G1 -C -V -L\"File %s - %s:%s\"", ofile, gridkindstring, ofile, title, zlabel);
		} else {
			snprintf(plot_cmd, sizeof(plot_cmd), "mbm_grdplot -I%s%s -G1 -W1/4 -S -D -V -L\"File %s - %s:%s\"", ofile, gridkindstring, ofile, title, zlabel);
		}
		if (verbose) GMT_Report(API, GMT_MSG_NORMAL, "\nexecuting mbm_grdplot...\n%s\n", plot_cmd);
		plot_status = system(plot_cmd);
		if (plot_status == -1)
			GMT_Report(API, GMT_MSG_NORMAL, "\nError executing mbm_grdplot on output file %s\n", ofile);
	}
	if (more && gridkind == MBGRID_GMTGRD) {
		strcpy(ofile, fileroot); strcat(ofile, "_num.grd");
		snprintf(plot_cmd, sizeof(plot_cmd), "mbm_grdplot -I%s%s -G1 -W1/2 -V -L\"File %s - %s:%s\"", ofile, gridkindstring, ofile, title, nlabel);
		if (verbose) GMT_Report(API, GMT_MSG_NORMAL, "\nexecuting mbm_grdplot...\n%s\n", plot_cmd);
		plot_status = system(plot_cmd);
		if (plot_status == -1)
			GMT_Report(API, GMT_MSG_NORMAL, "\nError executing mbm_grdplot on output file grd_%s\n", fileroot);

		strcpy(ofile, fileroot); strcat(ofile, "_sd.grd");
		snprintf(plot_cmd, sizeof(plot_cmd), "mbm_grdplot -I%s%s -G1 -W1/2 -V -L\"File %s - %s:%s\"", ofile, gridkindstring, ofile, title, sdlabel);
		if (verbose) GMT_Report(API, GMT_MSG_NORMAL, "\nexecuting mbm_grdplot...\n%s\n", plot_cmd);
		plot_status = system(plot_cmd);
		if (plot_status == -1)
			GMT_Report(API, GMT_MSG_NORMAL, "\nError executing mbm_grdplot on output file grd_%s\n", fileroot);
	}

	if (verbose > 0) GMT_Report(API, GMT_MSG_NORMAL, "\nDone.\n\n");

	if (verbose >= 4)
		status = mb_memory_list(verbose, &error);

	if (verbose >= 2) {
		GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  Program <%s> completed\n", THIS_MODULE_NAME);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  Ending status:\n");
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       status:  %d\n", status);
	}

	Return(GMT_NOERROR);
}
/*--------------------------------------------------------------------*/
