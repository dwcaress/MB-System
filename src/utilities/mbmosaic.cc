/*--------------------------------------------------------------------
 *    The MB-system:	mbmosaic.c	2/10/97
 *
 *    Copyright (c) 1997-2023 by
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
 * mbmosaic is an utility used to mosaic amplitude or sidescan
 * data contained in a set of swath mapping sonar data files.
 * This program mosaics the data using a prioritization scheme
 * tied to the apparent grazing angle and look azimuth for the
 * pixels/beams. The grazing
 * angle is calculated as arctan(xtrack / depth) where the
 * acrosstrack distance xtrack is positive to starboard.
 *
 * Author:	D. W. Caress
 * Date:	February 10, 1997
 */

#include <algorithm>
#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <limits>

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_format.h"
#include "mb_info.h"
#include "mb_io.h"
#include "mb_process.h"
#include "mb_status.h"

/* gridding algorithms */
typedef enum {
    MBMOSAIC_SINGLE_BEST = 1,
    MBMOSAIC_AVERAGE = 2,
} grid_mode_t;

/* grid format definitions */
typedef enum {
    MBMOSAIC_ASCII = 1,
    MBMOSAIC_OLDGRD = 2,
    MBMOSAIC_CDFGRD = 3,
    MBMOSAIC_ARCASCII = 4,
    MBMOSAIC_GMTGRD = 100,
} grid_type_t;

/* gridded data type */
typedef enum {
    MBMOSAIC_DATA_AMPLITUDE = 3,
    MBMOSAIC_DATA_SIDESCAN = 4,
    MBMOSAIC_DATA_FLAT_GRAZING = 5,
    MBMOSAIC_DATA_GRAZING = 6,
    MBMOSAIC_DATA_SLOPE = 7,
} datatype_t ;

/* prioritization mode */
// TODO(schwehr): DANGER!  This appears to be a bitmask, not an enum.
typedef enum {
    MBMOSAIC_PRIORITY_NONE = 0,
    MBMOSAIC_PRIORITY_ANGLE = 1,
    MBMOSAIC_PRIORITY_AZIMUTH = 2,
    MBMOSAIC_PRIORITY_HEADING = 4,
} priority_t;

typedef enum {
    MBMOSAIC_PRIORITYTABLE_FILE = 0,
    MBMOSAIC_PRIORITYTABLE_60DEGREESUP = 1,
    MBMOSAIC_PRIORITYTABLE_67DEGREESUP = 2,
    MBMOSAIC_PRIORITYTABLE_75DEGREESUP = 3,
    MBMOSAIC_PRIORITYTABLE_85DEGREESUP = 4,
    MBMOSAIC_PRIORITYTABLE_60DEGREESDN = 5,
    MBMOSAIC_PRIORITYTABLE_67DEGREESDN = 6,
    MBMOSAIC_PRIORITYTABLE_75DEGREESDN = 7,
    MBMOSAIC_PRIORITYTABLE_85DEGREESDN = 8,
} priority_table_t;

constexpr int n_priority_angle_60degreesup = 3;
double priority_angle_60degreesup_angle[] = {-60, 0, 60};
double priority_angle_60degreesup_priority[] = {1.0, 0.0, 1.0};
constexpr int n_priority_angle_67degreesup = 3;
double priority_angle_67degreesup_angle[] = {-67, 0, 67};
double priority_angle_67degreesup_priority[] = {1.0, 0.0, 1.0};
constexpr int n_priority_angle_75degreesup = 3;
double priority_angle_75degreesup_angle[] = {-75, 0, 75};
double priority_angle_75degreesup_priority[] = {1.0, 0.0, 1.0};
constexpr int n_priority_angle_85degreesup = 3;
double priority_angle_85degreesup_angle[] = {-85, 0, 85};
double priority_angle_85degreesup_priority[] = {1.0, 0.0, 1.0};
constexpr int n_priority_angle_60degreesdn = 3;
double priority_angle_60degreesdn_angle[] = {-60, 0, 60};
double priority_angle_60degreesdn_priority[] = {0.0, 1.0, 0.0};
constexpr int n_priority_angle_67degreesdn = 3;
double priority_angle_67degreesdn_angle[] = {-67, 0, 67};
double priority_angle_67degreesdn_priority[] = {0.0, 1.0, 0.0};
constexpr int n_priority_angle_75degreesdn = 3;
double priority_angle_75degreesdn_angle[] = {-75, 0, 75};
double priority_angle_75degreesdn_priority[] = {0.0, 1.0, 0.0};
constexpr int n_priority_angle_85degreesdn = 3;
double priority_angle_85degreesdn_angle[] = {-85, 0, 85};
double priority_angle_85degreesdn_priority[] = {0.0, 1.0, 0.0};

constexpr int MB7K2SS_NUM_ANGLES = 171;
constexpr double MB7K2SS_ANGLE_MAX = 85.0;

/* flag for no data in grid */;
constexpr int NO_DATA_FLAG = 99999;

/* interpolation mode */;
constexpr int MBMOSAIC_INTERP_NONE = 0;
constexpr int MBMOSAIC_INTERP_GAP = 1;
constexpr int MBMOSAIC_INTERP_NEAR = 2;
constexpr int MBMOSAIC_INTERP_ALL = 3;

constexpr int MBMOSAIC_FOOTPRINT_REAL = 0;
constexpr int MBMOSAIC_FOOTPRINT_SPACING = 1;

struct footprint {
	double x[4];
	double y[4];
};

constexpr char program_name[] = "mbmosaic";
constexpr char help_message[] =
    "mbmosaic is an utility used to mosaic amplitude or\n"
    "sidescan data contained in a set of swath sonar data files.\n"
    "This program uses one of four algorithms (gaussian weighted mean,\n"
    "median filter, minimum filter, maximum filter) to grid regions\n"
    "covered by multibeam swaths and then fills in gaps between\n"
    "the swaths (to the degree specified by the user) using a minimum\n"
    "curvature algorithm.";
constexpr char usage_message[] =
    "mbmosaic -Ifilelist -Oroot\n"
    "    [-Rwest/east/south/north -Rfactor -Adatatype\n"
    "    -Bborder -Cclip/mode/tension -Dxdim/ydim -Edx/dy/units\n"
    "    -Fpriority_range -Ggridkind -H -Jprojection -Llonflip -M -N -Ppings\n"
    "    -Sspeed -Ttopogrid -Ubearing/factor[/mode] -V -Wscale -Xextend\n"
    "    -Ypriority_source -Zbathdef]";

/*--------------------------------------------------------------------*/
/*
 * function write_ascii writes output grid to an ascii file
 */
int write_ascii(int verbose, char *outfile, float *grid, int nx, int ny, double xmin, double xmax, double ymin, double ymax,
                double dx, double dy, int *error) {
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
	if (fp == nullptr) {
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
	} else {
		fprintf(fp, "grid created by program mbmosaic\n");
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

	return (status);
}
/*--------------------------------------------------------------------*/
/*
 * function write_arcascii writes output grid to an Arc/Info ascii file
 */
int write_arcascii(int verbose, char *outfile, float *grid, int nx, int ny, double xmin, double xmax, double ymin, double ymax,
                   double dx, double dy, double nodata, int *error) {
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
	if (fp == nullptr) {
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
	}

	/* output grid */
	else {
		fprintf(fp, "ncols %d\n", nx);
		fprintf(fp, "nrows %d\n", ny);
		fprintf(fp, "xllcorner %.10g\n", xmin);
		fprintf(fp, "yllcorner %.10g\n", ymin);
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
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int double_compare(double *a, double *b) {
	return *a > *b ? 1 : -1;
}
/*--------------------------------------------------------------------*/
int mbmosaic_get_footprint(int verbose, int mode, double beamwidth_xtrack, double beamwidth_ltrack, double altitude,
                           double acrosstrack, double alongtrack, double acrosstrack_spacing, struct footprint *footprint,
                           int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBmosaic function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:             %d\n", verbose);
		fprintf(stderr, "dbg2       mode:                %d\n", mode);
		fprintf(stderr, "dbg2       beamwidth_xtrack:    %f\n", beamwidth_xtrack);
		fprintf(stderr, "dbg2       beamwidth_ltrack:    %f\n", beamwidth_ltrack);
		fprintf(stderr, "dbg2       altitude:            %f\n", altitude);
		fprintf(stderr, "dbg2       acrosstrack:         %f\n", acrosstrack);
		fprintf(stderr, "dbg2       alongtrack:          %f\n", alongtrack);
		fprintf(stderr, "dbg2       acrosstrack_spacing: %f\n", acrosstrack_spacing);
	}

	/* calculate footprint location in sonar coordinates */
	const double r = sqrt(altitude * altitude + acrosstrack * acrosstrack + alongtrack * alongtrack);
	double theta;
	double phi;
	double thetap;
	double phip;
	mb_xyz_to_takeoff(verbose, acrosstrack, alongtrack, altitude, &theta, &phi, error);

	phip = phi - 0.5 * beamwidth_ltrack;
	thetap = theta - 0.5 * beamwidth_xtrack;
	if (mode == MBMOSAIC_FOOTPRINT_REAL)
		footprint->x[0] = r * sin(DTR * thetap) * cos(DTR * phip);
	else
		footprint->x[0] = acrosstrack - 0.5 * acrosstrack_spacing;
	footprint->y[0] = r * sin(DTR * thetap) * sin(DTR * phip);

	phip = phi - 0.5 * beamwidth_ltrack;
	thetap = theta + 0.5 * beamwidth_xtrack;
	if (mode == MBMOSAIC_FOOTPRINT_REAL)
		footprint->x[1] = r * sin(DTR * thetap) * cos(DTR * phip);
	else
		footprint->x[1] = acrosstrack + 0.5 * acrosstrack_spacing;
	footprint->y[1] = r * sin(DTR * thetap) * sin(DTR * phip);

	phip = phi + 0.5 * beamwidth_ltrack;
	thetap = theta + 0.5 * beamwidth_xtrack;
	if (mode == MBMOSAIC_FOOTPRINT_REAL)
		footprint->x[2] = r * sin(DTR * thetap) * cos(DTR * phip);
	else
		footprint->x[2] = acrosstrack + 0.5 * acrosstrack_spacing;
	footprint->y[2] = r * sin(DTR * thetap) * sin(DTR * phip);

	phip = phi + 0.5 * beamwidth_ltrack;
	thetap = theta - 0.5 * beamwidth_xtrack;
	if (mode == MBMOSAIC_FOOTPRINT_REAL)
		footprint->x[3] = r * sin(DTR * thetap) * cos(DTR * phip);
	else
		footprint->x[3] = acrosstrack - 0.5 * acrosstrack_spacing;
	footprint->y[3] = r * sin(DTR * thetap) * sin(DTR * phip);

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBmosaic function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		for (int i = 0; i < 4; i++)
			fprintf(stderr, "dbg2       footprint: x[%d]:%f y[%d]:%f\n", i, footprint->x[i], i, footprint->y[i]);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbmosaic_get_beamangles(int verbose, double sensordepth, int beams_bath, char *beamflag, double *bath, double *bathacrosstrack,
                            double *bathalongtrack, double *gangles, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBmosaic function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       sensordepth:      %f\n", sensordepth);
		fprintf(stderr, "dbg2       beams_bath:      %d\n", beams_bath);
		fprintf(stderr, "dbg2       bathymetry:\n");
		for (int i = 0; i < beams_bath; i++)
			fprintf(stderr, "dbg2         beam:%d  flag:%d  bath:%f %f %f\n", i, beamflag[i], bath[i], bathacrosstrack[i],
			        bathalongtrack[i]);
	}

	/* loop over all beams, calculate grazing angles for valid beams */
	for (int i = 0; i < beams_bath; i++) {
		if (mb_beam_ok(beamflag[i])) {
			gangles[i] = RTD * atan(bathacrosstrack[i] / (bath[i] - sensordepth));
		}
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBmosaic function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       beams_bath:      %d\n", beams_bath);
		fprintf(stderr, "dbg2       bathymetry:\n");
		for (int i = 0; i < beams_bath; i++)
			fprintf(stderr, "dbg2         beam:%d  flag:%d  bath:%f %f %f  angle:%f\n", i, beamflag[i], bath[i],
			        bathacrosstrack[i], bathalongtrack[i], gangles[i]);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbmosaic_get_beampriorities(int verbose, int priority_mode, int n_priority_angle, double *priority_angle_angle,
                                double *priority_angle_priority, double priority_azimuth, double priority_azimuth_factor,
                                double priority_heading, double priority_heading_factor, double heading, int beams_bath,
                                char *beamflag, double *gangles, double *priorities, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBmosaic function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       priority_mode:             %d\n", priority_mode);
		fprintf(stderr, "dbg2       n_priority_angle:          %d\n", n_priority_angle);
		fprintf(stderr, "dbg2       priority angle table:\n");
		for (int i = 0; i < n_priority_angle; i++)
			fprintf(stderr, "dbg2         %d  angle:%f  priority:%f\n", i, priority_angle_angle[i], priority_angle_priority[i]);
		fprintf(stderr, "dbg2       priority_azimuth:          %f\n", priority_azimuth);
		fprintf(stderr, "dbg2       priority_azimuth_factor:   %f\n", priority_azimuth_factor);
		fprintf(stderr, "dbg2       heading:         %f\n", heading);
		fprintf(stderr, "dbg2       beams_bath:      %d\n", beams_bath);
		fprintf(stderr, "dbg2       bathymetry grazing angles:\n");
		for (int i = 0; i < beams_bath; i++)
			fprintf(stderr, "dbg2         beam:%d  flag:%d angle:%f\n", i, beamflag[i], gangles[i]);
	}

	/* initialize priority array */
	for (int i = 0; i < beams_bath; i++) {
		if (mb_beam_ok(beamflag[i])) {
			priorities[i] = 1.0;
		}
		else {
			priorities[i] = 0.0;
		}
	}

	/* get grazing angle priorities */
	if (priority_mode & MBMOSAIC_PRIORITY_ANGLE) {
		/* loop over data getting angle based priorities */
		for (int i = 0; i < beams_bath; i++) {
			if (mb_beam_ok(beamflag[i])) {
				/* priority zero if outside the range of the priority-angle table */
				if (gangles[i] < priority_angle_angle[0] || gangles[i] > priority_angle_angle[n_priority_angle - 1]) {
					priorities[i] = 0.0;
				}

				/* priority set using the priority-angle table */
				else {
					for (int j = 0; j < n_priority_angle - 1; j++) {
						if (gangles[i] >= priority_angle_angle[j] && gangles[i] < priority_angle_angle[j + 1]) {
							priorities[i] *=
							    (priority_angle_priority[j] + (priority_angle_priority[j + 1] - priority_angle_priority[j]) *
							                                      (gangles[i] - priority_angle_angle[j]) /
							                                      (priority_angle_angle[j + 1] - priority_angle_angle[j]));
						}
					}
				}
			}
		}
	}

	double heading_difference;
	double weight_heading;
	/* get look azimuth priorities */

	if (priority_mode & MBMOSAIC_PRIORITY_AZIMUTH) {
		/* get priorities for starboard and port sides of ping */
		double azi_starboard = heading - 90.0 - priority_azimuth;
		if (azi_starboard > 180.0)
			azi_starboard -= 360.0 * ((int)((azi_starboard + 180.0) / 360.0));
		else if (azi_starboard < -180.0)
			azi_starboard += 360.0 * ((int)((-azi_starboard + 180.0) / 360.0));
		double weight_starboard;
		if (priority_azimuth_factor * azi_starboard <= -90.0 || priority_azimuth_factor * azi_starboard >= 90.0)
			weight_starboard = 0.0;
		else
			weight_starboard = std::max(cos(DTR * priority_azimuth_factor * azi_starboard), 0.0);

		double azi_port = heading + 90.0 - priority_azimuth;
		if (azi_port > 180.0)
			azi_port -= 360.0 * ((int)((azi_port + 180.0) / 360.0));
		else if (azi_port < -180.0)
			azi_port += 360.0 * ((int)((-azi_port + 180.0) / 360.0));
		double weight_port;
		if (priority_azimuth_factor * azi_port <= -90.0 || priority_azimuth_factor * azi_port >= 90.0)
			weight_port = 0.0;
		else
			weight_port = std::max(cos(DTR * priority_azimuth_factor * azi_port), 0.0);

		/* apply the look azimuth priorities */
		for (int i = 0; i < beams_bath; i++) {
			if (mb_beam_ok(beamflag[i])) {
				if (gangles[i] < 0.0)
					priorities[i] *= weight_starboard;
				else
					priorities[i] *= weight_port;
			}
		}
	}

	/* get heading priorities */
	if (priority_mode & MBMOSAIC_PRIORITY_HEADING) {
		/* get priorities for ping */
		heading_difference = heading - priority_heading;
		if (heading_difference > 180.0)
			heading_difference -= 360.0;
		else if (heading_difference < -180.0)
			heading_difference += 360.0;
		if (priority_heading_factor * heading_difference <= -90.0 || priority_heading_factor * heading_difference >= 90.0)
			weight_heading = 0.0;
		else
			weight_heading = std::max(cos(DTR * priority_heading_factor * heading_difference), 0.0);

		/* apply the heading priorities */
		for (int i = 0; i < beams_bath; i++) {
			if (mb_beam_ok(beamflag[i])) {
				priorities[i] *= weight_heading;
			}
		}
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBmosaic function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       beams_bath:      %d\n", beams_bath);
		fprintf(stderr, "dbg2       bathymetry grazing angles and priorities:\n");
		for (int i = 0; i < beams_bath; i++)
			fprintf(stderr, "dbg2         beam:%d  flag:%d angle:%f  priority:%f\n", i, beamflag[i], gangles[i], priorities[i]);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbmosaic_get_beamslopes(int verbose, int beams_bath, char *beamflag, double *bath, double *bathacrosstrack, double *slopes,
                            int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBmosaic function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       beams_bath:      %d\n", beams_bath);
		fprintf(stderr, "dbg2       bathymetry:\n");
		for (int i = 0; i < beams_bath; i++)
			fprintf(stderr, "dbg2         beam:%d  flag:%d  bath:%f %f\n", i, beamflag[i], bath[i], bathacrosstrack[i]);
	}

	/* get grazing angle priorities */
	/* loop over data getting angle based priorities */
	for (int i = 0; i < beams_bath; i++) {
		if (mb_beam_ok(beamflag[i])) {
			/* find previous good beam */
			bool found_pre = false;
			int i0;
			if (i > 0) {
				for (int j = i - 1; j >= 0 && !found_pre; j--) {
					if (mb_beam_ok(beamflag[j])) {
						found_pre = true;
						i0 = j;
					}
				}
			}

			/* find post good beam */
			bool found_post = false;
			int i1;
			if (i < beams_bath - 1) {
				for (int j = i + 1; j < beams_bath && !found_post; j++) {
					if (mb_beam_ok(beamflag[j])) {
						found_post = true;
						i1 = j;
					}
				}
			}

			/* calculate slope */
			if (found_pre && found_post) {
				if (bathacrosstrack[i1] != bathacrosstrack[i0])
					slopes[i] = -(bath[i1] - bath[i0]) / (bathacrosstrack[i1] - bathacrosstrack[i0]);
				else
					slopes[i] = 0.0;
			}
			else if (found_pre) {
				if (bathacrosstrack[i] != bathacrosstrack[i0])
					slopes[i] = -(bath[i] - bath[i0]) / (bathacrosstrack[i] - bathacrosstrack[i0]);
				else
					slopes[i] = 0.0;
			}
			else if (found_post) {
				if (bathacrosstrack[i1] != bathacrosstrack[i])
					slopes[i] = -(bath[i1] - bath[i]) / (bathacrosstrack[i1] - bathacrosstrack[i]);
				else
					slopes[i] = 0.0;
			}
			else {
				slopes[i] = 0.0;
			}
		}
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBmosaic function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       beams_bath:      %d\n", beams_bath);
		fprintf(stderr, "dbg2       bathymetry:\n");
		for (int i = 0; i < beams_bath; i++)
			fprintf(stderr, "dbg2         beam:%d  flag:%d  bath:%f %f  slope:%f\n", i, beamflag[i], bath[i], bathacrosstrack[i],
			        slopes[i]);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbmosaic_bath_getangletable(int verbose, double sensordepth, int beams_bath, char *beamflag, double *bath,
                                double *bathacrosstrack, double *bathalongtrack, double angle_min, double angle_max, int nangle,
                                double *table_angle, double *table_xtrack, double *table_ltrack, double *table_altitude,
                                double *table_range, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBmosaic function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       sensordepth:      %f\n", sensordepth);
		fprintf(stderr, "dbg2       beams_bath:      %d\n", beams_bath);
		fprintf(stderr, "dbg2       bathymetry:\n");
		for (int i = 0; i < beams_bath; i++)
			fprintf(stderr, "dbg2         beam:%d  flag:%d  bath:%f %f\n", i, beamflag[i], bath[i], bathacrosstrack[i]);
		fprintf(stderr, "dbg2       angle_min:       %f\n", angle_min);
		fprintf(stderr, "dbg2       angle_max:       %f\n", angle_max);
		fprintf(stderr, "dbg2       nangle:          %d\n", nangle);
	}

	double angle1;
	double factor;
	int jstart;
	int jnext;

	/* loop over the angles and figure out the other table values from the bathymetry */
	const double dangle = (angle_max - angle_min) / (nangle - 1);
	jstart = 0;
	*error = MB_ERROR_NO_ERROR;
	int status = MB_SUCCESS;
	for (int i = 0; i < nangle; i++) {
		/* get angles in takeoff coordinates */
		table_angle[i] = angle_min + dangle * i;
		table_xtrack[i] = 0.0;
		table_ltrack[i] = 0.0;
		table_range[i] = 0.0;

		/* estimate the table values for this angle from the bathymetry */
		bool found = false;
		for (int j = jstart; j < beams_bath - 1 && !found; j++) {
			/* check if this beam is valid */
			if (mb_beam_ok(beamflag[j])) {
				/* look for the next valid beam */
				bool foundnext = false;
				jnext = j;
				for (int jj = j + 1; jj < beams_bath && !foundnext; jj++) {
					if (mb_beam_ok(beamflag[jj])) {
						jnext = jj;
						foundnext = true;
					}
				}

				/* get the angle for beam j */
				const double angle0 = RTD * atan(bathacrosstrack[j] / (bath[j] - sensordepth));
				if (foundnext)
					angle1 = RTD * atan(bathacrosstrack[jnext] / (bath[jnext] - sensordepth));

				/* deal with angle to port of swath edge */
				if (table_angle[i] <= angle0) {
					table_altitude[i] = bath[j] - sensordepth;
					table_xtrack[i] = table_altitude[i] * tan(DTR * table_angle[i]);
					table_ltrack[i] = bathalongtrack[j];
					table_range[i] = sqrt(table_altitude[i] * table_altitude[i] + table_xtrack[i] * table_xtrack[i] +
					                      table_ltrack[i] * table_ltrack[i]);
					found = true;
					jstart = j;
				}

				/* deal with angle to starboard of swath edge */
				else if (!foundnext) {
					table_altitude[i] = bath[j] - sensordepth;
					table_xtrack[i] = table_altitude[i] * tan(DTR * table_angle[i]);
					table_ltrack[i] = bathalongtrack[j];
					table_range[i] = sqrt(table_altitude[i] * table_altitude[i] + table_xtrack[i] * table_xtrack[i] +
					                      table_ltrack[i] * table_ltrack[i]);
					found = true;
					jstart = j;
				}

				/* deal with angle to starboard of swath edge */
				else if (foundnext && table_angle[i] > angle1) {
					if (jnext == beams_bath - 1) {
						table_altitude[i] = bath[j] - sensordepth;
						table_xtrack[i] = table_altitude[i] * tan(DTR * table_angle[i]);
						table_ltrack[i] = bathalongtrack[j];
						table_range[i] = sqrt(table_altitude[i] * table_altitude[i] + table_xtrack[i] * table_xtrack[i] +
						                      table_ltrack[i] * table_ltrack[i]);
						found = true;
					}
					jstart = j;
				}

				/* deal with angle between the two valid beams */
				else if (foundnext && table_angle[i] >= angle0 && table_angle[i] <= angle1) {
					factor = (table_angle[i] - angle0) / (angle1 - angle0);
					table_altitude[i] = (bath[j] - sensordepth) + factor * (bath[jnext] - bath[j]);
					table_xtrack[i] = table_altitude[i] * tan(DTR * table_angle[i]);
					table_ltrack[i] = bathalongtrack[j] + factor * (bathalongtrack[jnext] - bathalongtrack[j]);
					table_range[i] = sqrt(table_altitude[i] * table_altitude[i] + table_xtrack[i] * table_xtrack[i] +
					                      table_ltrack[i] * table_ltrack[i]);
					found = true;
					jstart = j;
				}

				/* else skip */
			}
		}

		/* set error if necessary */
		if (!found) {
			status = MB_FAILURE;
			*error = MB_ERROR_NOT_ENOUGH_DATA;
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBmosaic function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nangle:          %d\n", nangle);
		fprintf(stderr, "dbg2       tables:\n");
		for (int i = 0; i < nangle; i++)
			fprintf(stderr, "dbg2         %d angle:%f  xtrack:%f ltrack:%f altitude:%f range:%f\n", i, table_angle[i],
			        table_xtrack[i], table_ltrack[i], table_altitude[i], table_range[i]);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbmosaic_flatbottom_getangletable(int verbose, double altitude, double angle_min, double angle_max, int nangle,
                                      double *table_angle, double *table_xtrack, double *table_ltrack, double *table_altitude,
                                      double *table_range, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBmosaic function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       altitude:        %f\n", altitude);
		fprintf(stderr, "dbg2       angle_min:       %f\n", angle_min);
		fprintf(stderr, "dbg2       angle_max:       %f\n", angle_max);
		fprintf(stderr, "dbg2       nangle:          %d\n", nangle);
	}

	/* loop over the angles and figure out the other table values from the bathymetry */
	const double dangle = (angle_max - angle_min) / (nangle - 1);
	*error = MB_ERROR_NO_ERROR;
	for (int i = 0; i < nangle; i++) {
		/* get angles in takeoff coordinates */
		table_angle[i] = angle_min + dangle * i;
		table_xtrack[i] = altitude * tan(DTR * table_angle[i]);
		table_ltrack[i] = 0.0;
		table_range[i] = sqrt(altitude * altitude + table_xtrack[i] * table_xtrack[i]);
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBmosaic function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nangle:          %d\n", nangle);
		fprintf(stderr, "dbg2       tables:\n");
		for (int i = 0; i < nangle; i++)
			fprintf(stderr, "dbg2         %d angle:%f  xtrack:%f ltrack:%f altitude:%f range:%f\n", i, table_angle[i],
			        table_xtrack[i], table_ltrack[i], table_altitude[i], table_range[i]);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbmosaic_get_ssangles(int verbose, int nangle, double *table_angle, double *table_xtrack, double *table_ltrack,
                          double *table_altitude, double *table_range, int pixels_ss, double *ss, double *ssacrosstrack,
                          double *gangles, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBmosaic function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       nangle:          %d\n", nangle);
		fprintf(stderr, "dbg2       tables:\n");
		for (int i = 0; i < nangle; i++)
			fprintf(stderr, "dbg2         %d angle:%f  xtrack:%f ltrack:%f altitude:%f range:%f\n", i, table_angle[i],
			        table_xtrack[i], table_ltrack[i], table_altitude[i], table_range[i]);
		fprintf(stderr, "dbg2       pixels_ss:       %d\n", pixels_ss);
		fprintf(stderr, "dbg2       sidescan:\n");
		for (int i = 0; i < pixels_ss; i++)
			fprintf(stderr, "dbg2         pixel:%d  ss:%f %f\n", i, ss[i], ssacrosstrack[i]);
	}

	/* loop over the sidescan interpolating angles from the table on the basis of ssacrosstrack */
	int jstart = 0;
	for (int i = 0; i < pixels_ss; i++) {
		/* get angles only for valid sidescan */
		if (ss[i] > MB_SIDESCAN_NULL) {
			bool found = false;
			for (int j = jstart; j < nangle - 1 && !found; j++) {
				if (ssacrosstrack[i] < table_xtrack[j]) {
					gangles[i] = table_angle[j];
					found = true;
				}
				else if (ssacrosstrack[i] >= table_xtrack[j] && ssacrosstrack[i] <= table_xtrack[j + 1]) {
					gangles[i] = table_angle[j] + (table_angle[j + 1] - table_angle[j]) * (ssacrosstrack[i] - table_xtrack[j]) /
					                                  (table_xtrack[j + 1] - table_xtrack[j]);
					found = true;
					jstart = j;
				}
				else if (ssacrosstrack[i] >= table_xtrack[j + 1] && j == nangle - 2) {
					gangles[i] = table_angle[j + 1];
					found = true;
				}
			}
		}

		/* zero angles for invalid sidescan */
		else {
			gangles[i] = 0.0;
		}
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBmosaic function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       pixels_ss:       %d\n", pixels_ss);
		fprintf(stderr, "dbg2       sidescan grazing angles:\n");
		for (int i = 0; i < pixels_ss; i++)
			fprintf(stderr, "dbg2         pixel:%d  ss:%f %f angle:%f\n", i, ss[i], ssacrosstrack[i], gangles[i]);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbmosaic_get_sspriorities(int verbose, int priority_mode, int n_priority_angle, double *priority_angle_angle,
                              double *priority_angle_priority, double priority_azimuth, double priority_azimuth_factor,
                              double priority_heading, double priority_heading_factor, double heading, int pixels_ss, double *ss,
                              double *gangles, double *priorities, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBmosaic function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       priority_mode:             %d\n", priority_mode);
		fprintf(stderr, "dbg2       n_priority_angle:          %d\n", n_priority_angle);
		fprintf(stderr, "dbg2       priority angle table:\n");
		for (int i = 0; i < n_priority_angle; i++)
			fprintf(stderr, "dbg2         %d  angle:%f  priority:%f\n", i, priority_angle_angle[i], priority_angle_priority[i]);
		fprintf(stderr, "dbg2       priority_azimuth:          %f\n", priority_azimuth);
		fprintf(stderr, "dbg2       priority_azimuth_factor:   %f\n", priority_azimuth_factor);
		fprintf(stderr, "dbg2       heading:         %f\n", heading);
		fprintf(stderr, "dbg2       pixels_ss:       %d\n", pixels_ss);
		fprintf(stderr, "dbg2       sidescan grazing angles:\n");
		for (int i = 0; i < pixels_ss; i++)
			fprintf(stderr, "dbg2         pixel:%d  ss:%f angle:%f\n", i, ss[i], gangles[i]);
	}

	/* initialize priority array */
	for (int i = 0; i < pixels_ss; i++) {
		if (ss[i] > MB_SIDESCAN_NULL) {
			priorities[i] = 1.0;
		}
		else {
			priorities[i] = 0.0;
		}
	}

	double heading_difference, weight_heading;

	/* get grazing angle priorities */
	if (priority_mode & MBMOSAIC_PRIORITY_ANGLE) {
		/* loop over data getting angle based priorities */
		for (int i = 0; i < pixels_ss; i++) {
			if (ss[i] > MB_SIDESCAN_NULL) {
				/* priority zero if outside the range of the priority-angle table */
				if (gangles[i] < priority_angle_angle[0] || gangles[i] > priority_angle_angle[n_priority_angle - 1]) {
					priorities[i] = 0.0;
				}

				/* priority set using the priority-angle table */
				else {
					for (int j = 0; j < n_priority_angle - 1; j++) {
						if (gangles[i] >= priority_angle_angle[j] && gangles[i] < priority_angle_angle[j + 1]) {
							priorities[i] *=
							    (priority_angle_priority[j] + (priority_angle_priority[j + 1] - priority_angle_priority[j]) *
							                                      (gangles[i] - priority_angle_angle[j]) /
							                                      (priority_angle_angle[j + 1] - priority_angle_angle[j]));
						}
					}
				}
			}
		}
	}

	/* get look azimuth priorities */
	if (priority_mode & MBMOSAIC_PRIORITY_AZIMUTH) {
		/* get priorities for starboard and port sides of ping */
		double azi_starboard = heading - 90.0 - priority_azimuth;
		if (azi_starboard > 180.0)
			azi_starboard -= 360.0 * ((int)((azi_starboard + 180.0) / 360.0));
		else if (azi_starboard < -180.0)
			azi_starboard += 360.0 * ((int)((-azi_starboard + 180.0) / 360.0));
		double weight_starboard;
		if (priority_azimuth_factor * azi_starboard <= -90.0 || priority_azimuth_factor * azi_starboard >= 90.0)
			weight_starboard = 0.0;
		else
			weight_starboard = std::max(cos(DTR * priority_azimuth_factor * azi_starboard), 0.0);

		double azi_port = heading + 90.0 - priority_azimuth;
		if (azi_port > 180.0)
			azi_port -= 360.0 * ((int)((azi_port + 180.0) / 360.0));
		else if (azi_port < -180.0)
			azi_port += 360.0 * ((int)((-azi_port + 180.0) / 360.0));
		double weight_port;
		if (priority_azimuth_factor * azi_port <= -90.0 || priority_azimuth_factor * azi_port >= 90.0)
			weight_port = 0.0;
		else
			weight_port = std::max(cos(DTR * priority_azimuth_factor * azi_port), 0.0);

		/* apply the look azimuth priorities */
		for (int i = 0; i < pixels_ss; i++) {
			if (ss[i] > MB_SIDESCAN_NULL) {
				if (gangles[i] < 0.0)
					priorities[i] *= weight_starboard;
				else
					priorities[i] *= weight_port;
			}
		}
	}

	/* get heading priorities */
	if (priority_mode & MBMOSAIC_PRIORITY_HEADING) {
		/* get priorities for ping */
		heading_difference = heading - priority_heading;
		if (heading_difference > 180.0)
			heading_difference -= 360.0;
		else if (heading_difference < -180.0)
			heading_difference += 360.0;
		if (priority_heading_factor * heading_difference <= -90.0 || priority_heading_factor * heading_difference >= 90.0)
			weight_heading = 0.0;
		else
			weight_heading = std::max(cos(DTR * priority_heading_factor * heading_difference), 0.0);

		/* apply the look azimuth priorities */
		for (int i = 0; i < pixels_ss; i++) {
			if (ss[i] > MB_SIDESCAN_NULL) {
				priorities[i] *= weight_heading;
			}
		}
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBmosaic function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       pixels_ss:       %d\n", pixels_ss);
		fprintf(stderr, "dbg2       sidescan grazing angles and priorities:\n");
		for (int i = 0; i < pixels_ss; i++)
			fprintf(stderr, "dbg2         pixel:%d  angle:%f  priority:%f\n", i, gangles[i], priorities[i]);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
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

	datatype_t datatype = MBMOSAIC_DATA_SIDESCAN;
	bool usefiltered = false;
	double border = 0.0;
	int clip = 0;
	int clipmode = MBMOSAIC_INTERP_NONE;
	double tension = 0.0;
	int xdim = 101;
	int ydim = 101;
	bool spacing_priority = false;
	bool set_dimensions = false;
	double dx_set = 0.0;
	double dy_set = 0.0;
	mb_path units = "";
	bool set_spacing = false;
	double priority_range = 0.0;
	int weight_priorities = 0;
	grid_mode_t grid_mode = MBMOSAIC_SINGLE_BEST;
	char gridkindstring[MB_PATH_MAXLINE] = "";
	grid_type_t gridkind = MBMOSAIC_GMTGRD;
	bool more = false;
	mb_path filelist = "datalist.mb-1";
	bool projection_pars_f = false;
	mb_path projection_pars = "";
	bool use_NaN = false;
	mb_path fileroot = "grid";
	double boundsfactor = 0.0;
	double gbnd[4] = {0.0, 0.0, 0.0, 0.0};
	bool gbndset = false;
	mb_path topogridfile = "";
	bool usetopogrid = false;
	double priority_heading = 0.0;
	double priority_heading_factor = 1.0;
	int k_mode;  // For option "u"
	priority_t priority_mode = MBMOSAIC_PRIORITY_NONE;
	double priority_azimuth = 0.0;
	double priority_azimuth_factor = 1.0;
	double scale = 1.0;
	double extend = 0.0;
	priority_table_t priority_source = MBMOSAIC_PRIORITYTABLE_FILE;
	char pfile[MB_PATH_MAXLINE] = "";
	int n_priority_angle = 0;
	double *priority_angle_angle = nullptr;
	double *priority_angle_priority = nullptr;
	double altitude_default = 1000.0;
	/* output stream for basic stuff (stdout if verbose <= 1,
	    stderr if verbose > 1) */
	FILE *outfp = nullptr;

	{
		bool errflg = false;
		bool help = false;
		int c;
		while ((c = getopt(argc, argv, "A:a:B:b:C:c:D:d:E:e:F:f:G:g:HhI:i:J:j:L:l:MmNnO:o:P:p:R:r:S:s:T:t:U:u:VvW:w:X:x:Y:y:Z:z:")) !=
		       -1)
		{
			switch (c) {
			case 'A':
			case 'a':
			{
				int tmp;
				sscanf(optarg, "%d", &tmp);
				datatype = (datatype_t)tmp;
				if (optarg[1] == 'f' || optarg[1] == 'F')
					usefiltered = true;
				break;
			}
			case 'B':
			case 'b':
				sscanf(optarg, "%lf", &border);
				break;
			case 'C':
			case 'c':
			{
				const int n = sscanf(optarg, "%d/%d/%lf", &clip, &clipmode, &tension);
				if (n < 1)
					clipmode = MBMOSAIC_INTERP_NONE;
				else if (n == 1 && clip > 0)
					clipmode = MBMOSAIC_INTERP_GAP;
				else if (n == 1)
					clipmode = MBMOSAIC_INTERP_NONE;
				else if (clip > 0 && clipmode < 0)
					clipmode = MBMOSAIC_INTERP_GAP;
				else if (clipmode >= 3)
					clipmode = MBMOSAIC_INTERP_ALL;
				if (n < 3) {
					tension = 0.0;
				}
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
				sscanf(optarg, "%lf/%d", &priority_range, &weight_priorities);
				grid_mode = MBMOSAIC_AVERAGE;
				break;
      case 'G':
      case 'g':
        if (optarg[0] == '=') {
          gridkind = MBMOSAIC_GMTGRD;
          strcpy(gridkindstring, optarg);
        }
        else {
          int tmp;
          int nscan = sscanf(optarg, "%d", &tmp);
          // Range check
          if (nscan == 1 && tmp >= 1 && tmp <= 4) {
            gridkind = (grid_type_t)tmp;
            if (gridkind == MBMOSAIC_CDFGRD) {
              gridkind = MBMOSAIC_GMTGRD;
              gridkindstring[0] = '\0';
            }
          } else if (optarg[0] == 'n' || optarg[0] == 'c' || optarg[0] == 'b'
                    || optarg[0] == 'r' || optarg[0] == 's' || optarg[0] == 'a'
                    || optarg[0] == 'e' || optarg[0] == 'g'){
            snprintf(gridkindstring, sizeof(gridkindstring), "=%s", optarg);
            gridkind = MBMOSAIC_GMTGRD;
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
				sscanf(optarg, "%1023s", topogridfile);
				usetopogrid = true;
				break;
			case 'U':
			case 'u':
			{
				double t1;  // bearing
				double t2;  // factor
				const int n = sscanf(optarg, "%lf/%lf/%d", &t1, &t2, &k_mode);
				if (n == 3 && k_mode == 1) {
					priority_heading = t1;
					priority_heading_factor = t2;
					if ((priority_mode & MBMOSAIC_PRIORITY_HEADING) == 0) {
						priority_mode = static_cast<priority_t>(
							static_cast<int>(priority_mode) +
							static_cast<int>(MBMOSAIC_PRIORITY_HEADING));

					}
				}
				else if (n >= 2) {
					priority_azimuth = t1;
					priority_azimuth_factor = t2;
					if ((priority_mode & MBMOSAIC_PRIORITY_AZIMUTH) == 0) {
						priority_mode = static_cast<priority_t>(
							static_cast<int>(priority_mode) +
							static_cast<int>(MBMOSAIC_PRIORITY_AZIMUTH));
					}
				}
				else if (n >= 1) {
					priority_azimuth = t1;
					priority_azimuth_factor = 1.0;
					if ((priority_mode & MBMOSAIC_PRIORITY_AZIMUTH) == 0) {
						priority_mode = static_cast<priority_t>(
							static_cast<int>(priority_mode) +
							static_cast<int>(MBMOSAIC_PRIORITY_AZIMUTH));
					}
				}
				break;
			}
			case 'V':
			case 'v':
				verbose++;
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
			{
				int tmp = MBMOSAIC_PRIORITYTABLE_FILE;
				int n = sscanf(optarg, "%d", &tmp);
        if (n == 1) {
          if (tmp > MBMOSAIC_PRIORITYTABLE_FILE && tmp <= MBMOSAIC_PRIORITYTABLE_85DEGREESDN)
				      priority_source = (priority_table_t)tmp;
          else {
            fprintf(outfp, "\nInvalid argument to -Ypriority_source option: %s\n", optarg);
            errflg = true;
          }
        }
				if (priority_source == MBMOSAIC_PRIORITYTABLE_FILE) {
					sscanf(optarg, "%1023s", pfile);
				}
				else if (priority_source == MBMOSAIC_PRIORITYTABLE_60DEGREESUP) {
					n_priority_angle = n_priority_angle_60degreesup;
					priority_angle_angle = priority_angle_60degreesup_angle;
					priority_angle_priority = priority_angle_60degreesup_priority;
				}
				else if (priority_source == MBMOSAIC_PRIORITYTABLE_67DEGREESUP) {
					n_priority_angle = n_priority_angle_67degreesup;
					priority_angle_angle = priority_angle_67degreesup_angle;
					priority_angle_priority = priority_angle_67degreesup_priority;
				}
				else if (priority_source == MBMOSAIC_PRIORITYTABLE_75DEGREESUP) {
					n_priority_angle = n_priority_angle_75degreesup;
					priority_angle_angle = priority_angle_75degreesup_angle;
					priority_angle_priority = priority_angle_75degreesup_priority;
				}
				else if (priority_source == MBMOSAIC_PRIORITYTABLE_85DEGREESUP) {
					n_priority_angle = n_priority_angle_85degreesup;
					priority_angle_angle = priority_angle_85degreesup_angle;
					priority_angle_priority = priority_angle_85degreesup_priority;
				}
				else if (priority_source == MBMOSAIC_PRIORITYTABLE_60DEGREESDN) {
					n_priority_angle = n_priority_angle_60degreesdn;
					priority_angle_angle = priority_angle_60degreesdn_angle;
					priority_angle_priority = priority_angle_60degreesdn_priority;
				}
				else if (priority_source == MBMOSAIC_PRIORITYTABLE_67DEGREESDN) {
					n_priority_angle = n_priority_angle_67degreesdn;
					priority_angle_angle = priority_angle_67degreesdn_angle;
					priority_angle_priority = priority_angle_67degreesdn_priority;
				}
				else if (priority_source == MBMOSAIC_PRIORITYTABLE_75DEGREESDN) {
					n_priority_angle = n_priority_angle_75degreesdn;
					priority_angle_angle = priority_angle_75degreesdn_angle;
					priority_angle_priority = priority_angle_75degreesdn_priority;
				}
				else if (priority_source == MBMOSAIC_PRIORITYTABLE_85DEGREESDN) {
					n_priority_angle = n_priority_angle_85degreesdn;
					priority_angle_angle = priority_angle_85degreesdn_angle;
					priority_angle_priority = priority_angle_85degreesdn_priority;
				}
				if ((priority_mode & MBMOSAIC_PRIORITY_ANGLE) == 0) {
					priority_mode = static_cast<priority_t>(
						static_cast<int>(priority_mode) +
						static_cast<int>(MBMOSAIC_PRIORITY_ANGLE));
				}
				break;
			}
			case 'Z':
			case 'z':
				sscanf(optarg, "%lf", &altitude_default);
				break;
			case '?':
				errflg = true;
			}
		}

		if (verbose >= 2)
			outfp = stderr;
		else
			outfp = stdout;

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
			fprintf(outfp, "dbg2       more:                 %d\n", more);
			fprintf(outfp, "dbg2       use_NaN:              %d\n", use_NaN);
			fprintf(outfp, "dbg2       data type:            %d\n", datatype);
			fprintf(outfp, "dbg2       usefiltered:          %d\n", usefiltered);
			fprintf(outfp, "dbg2       grid format:          %d\n", gridkind);
			if (gridkind == MBMOSAIC_GMTGRD)
				fprintf(outfp, "dbg2       gmt grid format id:   %s\n", gridkindstring);
			fprintf(outfp, "dbg2       scale:                %f\n", scale);
			fprintf(outfp, "dbg2       border:               %f\n", border);
			fprintf(outfp, "dbg2       extend:               %f\n", extend);
			fprintf(outfp, "dbg2       tension:              %f\n", tension);
			fprintf(outfp, "dbg2       grid_mode:            %d\n", grid_mode);
			fprintf(outfp, "dbg2       priority_mode:        %d\n", priority_mode);
			fprintf(outfp, "dbg2       priority_range:       %f\n", priority_range);
			fprintf(outfp, "dbg2       weight_priorities:    %d\n", weight_priorities);
			fprintf(outfp, "dbg2       priority_source:      %d\n", priority_source);
			fprintf(outfp, "dbg2       pfile:                %s\n", pfile);
			fprintf(outfp, "dbg2       priority_azimuth:     %f\n", priority_azimuth);
			fprintf(outfp, "dbg2       priority_azimuth_fac: %f\n", priority_azimuth_factor);
			fprintf(outfp, "dbg2       altitude_default:     %f\n", altitude_default);
			fprintf(outfp, "dbg2       projection_pars:      %s\n", projection_pars);
			fprintf(outfp, "dbg2       proj flag 1:          %d\n", projection_pars_f);
			fprintf(stderr, "dbg2      usetopogrid:          %d\n", usetopogrid);
			fprintf(stderr, "dbg2      topogridfile:         %s\n", topogridfile);
		}

		if (help) {
			fprintf(outfp, "\n%s\n", help_message);
			fprintf(outfp, "\nusage: %s\n", usage_message);
			exit(MB_ERROR_NO_ERROR);
		}
	}

	int error = MB_ERROR_NO_ERROR;

	/* if bounds not set get bounds of input data */
	if (!gbndset) {
		int formatread = -1;
		struct mb_info_struct mb_info;
		status = mb_get_info_datalist(verbose, filelist, &formatread, &mb_info, lonflip, &error);

		gbnd[0] = mb_info.lon_min;
		gbnd[1] = mb_info.lon_max;
		gbnd[2] = mb_info.lat_min;
		gbnd[3] = mb_info.lat_max;
		gbndset = true;

		if (!set_spacing && !set_dimensions) {
			dx_set = 0.02 * mb_info.altitude_max;
			dy_set = 0.02 * mb_info.altitude_max;
			set_spacing = true;
			strcpy(units, "meters");
		}
	}

	/* if requested expand the grid bounds */
	if (boundsfactor > 1.0) {
		const double xx1 = 0.5 * (boundsfactor - 1.0) * (gbnd[1] - gbnd[0]);
		const double yy1 = 0.5 * (boundsfactor - 1.0) * (gbnd[3] - gbnd[2]);
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

	/* use bathymetry/amplitude beams for types other than sidescan */
	bool use_beams = false;
	if (datatype == MBMOSAIC_DATA_SIDESCAN)
		use_beams = false;
	else
		use_beams = true;

	/* use bathymetry slope for slope and slope corrected grazing angle */
	const bool use_slope = datatype == MBMOSAIC_DATA_GRAZING || datatype == MBMOSAIC_DATA_SLOPE;

	/* more option not available with single best algorithm */
	if (more && grid_mode == MBMOSAIC_SINGLE_BEST)
		more = false;

	/* NaN cannot be used for ASCII grids */
	float NaN;
	if (use_NaN && (gridkind == MBMOSAIC_ASCII || gridkind == MBMOSAIC_ARCASCII))
		use_NaN = false;

	float outclipvalue = NO_DATA_FLAG;
	/* define NaN in case it's needed */
	if (use_NaN) {
		outclipvalue = std::numeric_limits<float>::quiet_NaN();
	}

	double reference_lon;
	double reference_lat;
	mb_path projection_id = "Geographic";
	bool use_projection = false;
	void *pjptr = nullptr;
	double obnd[4];
	double mtodeglon = 0.0;
	double mtodeglat = 0.0;
	double deglontokm;
	double deglattokm;

	/* deal with projected gridding */
	if (projection_pars_f) {
		/* check for UTM with undefined zone */
		if (strcmp(projection_pars, "UTM") == 0 || strcmp(projection_pars, "U") == 0 || strcmp(projection_pars, "utm") == 0 ||
		    strcmp(projection_pars, "u") == 0) {
			reference_lon = 0.5 * (gbnd[0] + gbnd[1]);
			if (reference_lon < 180.0)
				reference_lon += 360.0;
			if (reference_lon >= 180.0)
				reference_lon -= 360.0;
			const int utm_zone = (int)(((reference_lon + 183.0) / 6.0) + 0.5);
			reference_lat = 0.5 * (gbnd[2] + gbnd[3]);
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
			error = MB_ERROR_BAD_PARAMETER;
			mb_memory_clear(verbose, &error);
			exit(error);
		}

		/* translate lon lat bounds from UTM if required */
		if (gbnd[0] < -360.0 || gbnd[0] > 360.0 || gbnd[1] < -360.0 || gbnd[1] > 360.0 || gbnd[2] < -90.0 || gbnd[2] > 90.0 ||
		    gbnd[3] < -90.0 || gbnd[3] > 90.0) {
			/* first point */
			double xx = gbnd[0];
			double yy = gbnd[2];
			double xlon;
			double ylat;
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
			double xlon = obnd[0];
			double ylat = obnd[2];
			double xx;
			double yy;
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

		fprintf(stderr, " Projected coordinates on: proj_status:%d  projection:%s\n", proj_status, projection_id);
		fprintf(stderr, " Lon Lat Bounds: %f %f %f %f\n", obnd[0], obnd[1], obnd[2], obnd[3]);
		fprintf(stderr, " XY Bounds: %f %f %f %f\n", gbnd[0], gbnd[1], gbnd[2], gbnd[3]);
	}

	/* deal with no projection */
	else {

		/* calculate grid properties */
		mb_coor_scale(verbose, 0.5 * (gbnd[2] + gbnd[3]), &mtodeglon, &mtodeglat);
		deglontokm = 0.001 / mtodeglon;
		deglattokm = 0.001 / mtodeglat;
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
	const double dx = (gbnd[1] - gbnd[0]) / (xdim - 1);
	const double dy = (gbnd[3] - gbnd[2]) / (ydim - 1);
	const double gaussian_factor = 4.0 / (scale * scale * dx * dy);
	int offx = 0;
	int offy = 0;
	if (extend > 0.0) {
		offx = (int)(extend * xdim);
		offy = (int)(extend * ydim);
	}
	int gxdim = xdim + 2 * offx;
	int gydim = ydim + 2 * offy;
	const double wbnd[4] = {
		gbnd[0] - offx * dx,
		gbnd[1] + offx * dx,
		gbnd[2] - offy * dy,
		gbnd[3] + offy * dy,
	};

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
		double xx = wbnd[0] - (wbnd[1] - wbnd[0]);
		double yy = wbnd[2] - (wbnd[3] - wbnd[2]);
		double xlon;
		double ylat;
		mb_proj_inverse(verbose, pjptr, xx, yy, &xlon, &ylat, &error);
		mb_apply_lonflip(verbose, lonflip, &xlon);
		bounds[0] = xlon;
		bounds[1] = xlon;
		bounds[2] = ylat;
		bounds[3] = ylat;

		/* do second point */
		xx = wbnd[0] + (wbnd[1] - wbnd[0]);
		yy = wbnd[2] - (wbnd[3] - wbnd[2]);
		mb_proj_inverse(verbose, pjptr, xx, yy, &xlon, &ylat, &error);
		mb_apply_lonflip(verbose, lonflip, &xlon);
		bounds[0] = std::min(bounds[0], xlon);
		bounds[1] = std::max(bounds[1], xlon);
		bounds[2] = std::min(bounds[2], ylat);
		bounds[3] = std::max(bounds[3], ylat);

		/* do third point */
		xx = wbnd[0] - (wbnd[1] - wbnd[0]);
		yy = wbnd[2] + (wbnd[3] - wbnd[2]);
		mb_proj_inverse(verbose, pjptr, xx, yy, &xlon, &ylat, &error);
		mb_apply_lonflip(verbose, lonflip, &xlon);
		bounds[0] = std::min(bounds[0], xlon);
		bounds[1] = std::max(bounds[1], xlon);
		bounds[2] = std::min(bounds[2], ylat);
		bounds[3] = std::max(bounds[3], ylat);

		/* do fourth point */
		xx = wbnd[0] + (wbnd[1] - wbnd[0]);
		yy = wbnd[2] + (wbnd[3] - wbnd[2]);
		mb_proj_inverse(verbose, pjptr, xx, yy, &xlon, &ylat, &error);
		mb_apply_lonflip(verbose, lonflip, &xlon);
		bounds[0] = std::min(bounds[0], xlon);
		bounds[1] = std::max(bounds[1], xlon);
		bounds[2] = std::min(bounds[2], ylat);
		bounds[3] = std::max(bounds[3], ylat);
	}

	/* extend the bounds slightly to be sure no data gets missed */
	double xx = std::min(0.05 * (bounds[1] - bounds[0]), 0.1);
	double yy = std::min(0.05 * (bounds[3] - bounds[2]), 0.1);
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
	if ((clipmode == MBMOSAIC_INTERP_GAP || clipmode == MBMOSAIC_INTERP_NEAR) && clip > xdim && clip > ydim)
		clipmode = MBMOSAIC_INTERP_ALL;
	if (clipmode == MBMOSAIC_INTERP_ALL)
		clip = std::max(xdim, ydim);

	/* set origin used to reduce data value size before conversion from
	 * double to float when calling the interpolation routines */
	const double bdata_origin_x = 0.5 * (wbnd[0] + wbnd[1]);
	const double bdata_origin_y = 0.5 * (wbnd[2] + wbnd[3]);

	/* if specified get static angle priorities */
	if (priority_source == MBMOSAIC_PRIORITYTABLE_FILE && (priority_mode & MBMOSAIC_PRIORITY_ANGLE)) {
		/* count priorities */
		FILE *fp = fopen(pfile, "r");
		if (fp == nullptr) {
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr, "\nUnable to Open Angle Weights File <%s> for reading\n", pfile);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			mb_memory_clear(verbose, &error);
			exit(MB_ERROR_OPEN_FAIL);
		}
		n_priority_angle = 0;
		mb_path buffer = "";
		while ((/* result = */ fgets(buffer, MB_PATH_MAXLINE, fp)) == buffer) {
			if (buffer[0] != '#') {
				n_priority_angle++;
			}
		}
		fclose(fp);

		/* allocate memory */
		if (error == MB_ERROR_NO_ERROR)
			status = mb_mallocd(verbose, __FILE__, __LINE__, n_priority_angle * sizeof(double), (void **)&priority_angle_angle,
			                    &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_mallocd(verbose, __FILE__, __LINE__, n_priority_angle * sizeof(double), (void **)&priority_angle_priority,
			                    &error);
		if (error != MB_ERROR_NO_ERROR) {
			char *message = nullptr;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
		}

		/* read in angle priorities */
		fp = fopen(pfile, "r");
		if (fp == nullptr) {
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr, "\nUnable to Open Angle Weights File <%s> for reading\n", pfile);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			mb_memory_clear(verbose, &error);
			exit(MB_ERROR_OPEN_FAIL);
		}
		n_priority_angle = 0;
		while ((/* result = */ fgets(buffer, MB_PATH_MAXLINE, fp)) == buffer) {
			if (buffer[0] != '#') {
				int n = sscanf(buffer, "%lf %lf", &priority_angle_angle[n_priority_angle], &priority_angle_priority[n_priority_angle]);
        if (n == 2)
				  n_priority_angle++;
			}
		}
		fclose(fp);
	}

	void *topogrid_ptr = nullptr;

	/* read topography grid if 3D bottom correction specified */
	if (usetopogrid) {
		status = mb_topogrid_init(verbose, topogridfile, &lonflip, &topogrid_ptr, &error);
		if (error != MB_ERROR_NO_ERROR) {
			char *message = nullptr;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error loading topography grid: %s\n%s\n", topogridfile, message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
		}
	}

	/* output info */
	if (verbose >= 0) {
		fprintf(outfp, "\nMBMOSAIC Parameters:\n");
		fprintf(outfp, "List of input files: %s\n", filelist);
		fprintf(outfp, "Output fileroot:     %s\n", fileroot);
		fprintf(outfp, "Input Data Type:     ");
		if (datatype == MBMOSAIC_DATA_AMPLITUDE && !usefiltered)
			fprintf(outfp, "Amplitude (unfiltered)\n");
		else if (datatype == MBMOSAIC_DATA_AMPLITUDE && usefiltered)
			fprintf(outfp, "Amplitude (filtered)\n");
		else if (datatype == MBMOSAIC_DATA_SIDESCAN && !usefiltered)
			fprintf(outfp, "Sidescan (unfiltered)\n");
		else if (datatype == MBMOSAIC_DATA_SIDESCAN && usefiltered)
			fprintf(outfp, "Sidescan (filtered)\n");
		else if (datatype == MBMOSAIC_DATA_FLAT_GRAZING)
			fprintf(outfp, "Flat bottom grazing angle\n");
		else if (datatype == MBMOSAIC_DATA_GRAZING)
			fprintf(outfp, "Grazing angle\n");
		else if (datatype == MBMOSAIC_DATA_SLOPE)
			fprintf(outfp, "Bottom slope\n");
		else
			fprintf(outfp, "Unknown?\n");
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
		fprintf(outfp, "Input data bounds:\n");
		fprintf(outfp, "  Longitude: %9.4f %9.4f\n", bounds[0], bounds[1]);
		fprintf(outfp, "  Latitude:  %9.4f %9.4f\n", bounds[2], bounds[3]);
		fprintf(outfp, "Mosaicing algorithm:  \n");
		if (grid_mode == MBMOSAIC_SINGLE_BEST)
			fprintf(outfp, "  Single highest weighted pixel\n");
		else if (grid_mode == MBMOSAIC_AVERAGE) {
			fprintf(outfp, "  Average of highest weighted pixels\n");
			fprintf(outfp, "  Pixel weighting range: %f\n", priority_range);
		}
		if (priority_mode == MBMOSAIC_PRIORITY_NONE)
			fprintf(outfp, "  All pixels weighted evenly\n");
		if (priority_mode & MBMOSAIC_PRIORITY_ANGLE) {
			fprintf(outfp, "  Pixels prioritized by flat bottom grazing angle\n");
			if (usetopogrid)
				fprintf(outfp, "  Pixel depths calculated from topography grid: %s\n", topogridfile);
			else
				fprintf(outfp, "  Pixel depths calculated from topoography in the swath file\n");
			if (priority_source == MBMOSAIC_PRIORITYTABLE_FILE)
				fprintf(outfp, "  Pixel prioritization file: %s\n", pfile);
			else if (priority_source == MBMOSAIC_PRIORITYTABLE_60DEGREESUP)
				fprintf(outfp, "  Pixel prioritization model: default 120 degree swath increasing out\n");
			else if (priority_source == MBMOSAIC_PRIORITYTABLE_67DEGREESUP)
				fprintf(outfp, "  Pixel prioritization model: default 134 degree swath increasing out\n");
			else if (priority_source == MBMOSAIC_PRIORITYTABLE_75DEGREESUP)
				fprintf(outfp, "  Pixel prioritization model: default 150 degree swath increasing out\n");
			else if (priority_source == MBMOSAIC_PRIORITYTABLE_85DEGREESUP)
				fprintf(outfp, "  Pixel prioritization model: default 170 degree swath increasing out\n");
			else if (priority_source == MBMOSAIC_PRIORITYTABLE_60DEGREESDN)
				fprintf(outfp, "  Pixel prioritization model: default 120 degree swath decreasing out\n");
			else if (priority_source == MBMOSAIC_PRIORITYTABLE_67DEGREESDN)
				fprintf(outfp, "  Pixel prioritization model: default 134 degree swath decreasing out\n");
			else if (priority_source == MBMOSAIC_PRIORITYTABLE_75DEGREESDN)
				fprintf(outfp, "  Pixel prioritization model: default 150 degree swath decreasing out\n");
			else if (priority_source == MBMOSAIC_PRIORITYTABLE_85DEGREESDN)
				fprintf(outfp, "  Pixel prioritization model: default 170 degree swath decreasing out\n");
			fprintf(outfp, "  Grazing angle priorities:\n");
			for (int i = 0; i < n_priority_angle; i++) {
				fprintf(outfp, "    %3d  %10.3f  %10.3f\n", i, priority_angle_angle[i], priority_angle_priority[i]);
			}
		}
		if (priority_mode & MBMOSAIC_PRIORITY_AZIMUTH) {
			fprintf(outfp, "  Pixels weighted by look azimuth\n");
			fprintf(outfp, "  Preferred look azimuth: %f\n", priority_azimuth);
			fprintf(outfp, "  Look azimuth factor:    %f\n", priority_azimuth_factor);
		}
		if (priority_mode & MBMOSAIC_PRIORITY_HEADING) {
			fprintf(outfp, "  Pixels weighted by platform heading\n");
			fprintf(outfp, "  Preferred heading:      %f\n", priority_heading);
			fprintf(outfp, "  Heading factor:         %f\n", priority_heading_factor);
		}
		fprintf(outfp, "  Gaussian filter 1/e length: %f grid intervals\n", scale);
		if (clipmode == MBMOSAIC_INTERP_NONE)
			fprintf(outfp, "Spline interpolation not applied\n");
		else if (clipmode == MBMOSAIC_INTERP_GAP) {
			fprintf(outfp, "Spline interpolation applied to fill data gaps\n");
			fprintf(outfp, "Spline interpolation clipping dimension: %d\n", clip);
			fprintf(outfp, "Spline tension (range 0.0 to infinity): %f\n", tension);
		}
		else if (clipmode == MBMOSAIC_INTERP_NEAR) {
			fprintf(outfp, "Spline interpolation applied near data\n");
			fprintf(outfp, "Spline interpolation clipping dimension: %d\n", clip);
			fprintf(outfp, "Spline tension (range 0.0 to infinity): %f\n", tension);
		}
		else if (clipmode == MBMOSAIC_INTERP_ALL) {
			fprintf(outfp, "Spline interpolation applied to fill entire grid\n");
			fprintf(outfp, "Spline tension (range 0.0 to infinity): %f\n", tension);
		}
		if (gridkind == MBMOSAIC_ASCII)
			fprintf(outfp, "Grid format %d:  ascii table\n", gridkind);
		else if (gridkind == MBMOSAIC_CDFGRD)
			fprintf(outfp, "Grid format %d:  GMT version 2 grd (netCDF)\n", gridkind);
		else if (gridkind == MBMOSAIC_OLDGRD)
			fprintf(outfp, "Grid format %d:  GMT version 1 grd (binary)\n", gridkind);
		else if (gridkind == MBMOSAIC_ARCASCII)
			fprintf(outfp, "Grid format %d:  Arc/Info ascii table\n", gridkind);
		else if (gridkind == MBMOSAIC_GMTGRD) {
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

	/* allocate memory for arrays */
	double *grid = nullptr;
	status &= mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(double), (void **)&grid, &error);
	double *norm = nullptr;
	status &= mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(double), (void **)&norm, &error);
	double *maxpriority = nullptr;
	status &= mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(double), (void **)&maxpriority, &error);
	int *cnt = nullptr;
	status &= mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(int), (void **)&cnt, &error);
	int *num = nullptr;
	if (clip != 0)
		status &= mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(int), (void **)&num, &error);
	double *sigma = nullptr;
	status &= mb_mallocd(verbose, __FILE__, __LINE__, gxdim * gydim * sizeof(double), (void **)&sigma, &error);
	float *output = nullptr;
	status &= mb_mallocd(verbose, __FILE__, __LINE__, xdim * ydim * sizeof(float), (void **)&output, &error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR) {
		char *message = nullptr;
		mb_error(verbose, error, &message);
		fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
		fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
		mb_memory_clear(verbose, &error);
		exit(error);
	}

	/* initialize arrays */
	for (int i = 0; i < gxdim; i++)
		for (int j = 0; j < gydim; j++) {
			int kgrid = i * gydim + j;
			grid[kgrid] = 0.0;
			norm[kgrid] = 0.0;
			cnt[kgrid] = 0;
			sigma[kgrid] = 0.0;
			maxpriority[kgrid] = 0.0;
		}

	/* open datalist file for list of all files that contribute to the grid */
	mb_path dfile = "";
	strcpy(dfile, fileroot);
	strcat(dfile, ".mb-1");
	FILE *dfp = fopen(dfile, "w");
	if (dfp == nullptr) {
		error = MB_ERROR_OPEN_FAIL;
		fprintf(outfp, "\nUnable to open datalist file: %s\n", dfile);
	}

	mb_path file = "";
	void *mbio_ptr = nullptr;
	double btime_d;
	double etime_d;
	int beams_bath;
	int beams_amp;
	int pixels_ss;
	struct mb_io_struct *mb_io_ptr = nullptr;
	void *store_ptr = nullptr;
	char *beamflag = nullptr;
	double *bath = nullptr;
	double *amp = nullptr;
	double *bathacrosstrack = nullptr;
	double *bathalongtrack = nullptr;
	double *bathlon = nullptr;
	double *bathlat = nullptr;
	double *ss = nullptr;
	double *ssacrosstrack = nullptr;
	double *ssalongtrack = nullptr;
	double *sslon = nullptr;
	double *sslat = nullptr;
	double *gangles = nullptr;
	double *slopes = nullptr;
	double *priorities = nullptr;
	struct footprint *footprints = nullptr;
	void *work1 = nullptr;
	void *work2 = nullptr;
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
	char comment[MB_COMMENT_MAXLINE];
	double draft;
	double roll;
	double pitch;
	double heave;
	double headingx = 0.0;
	double headingy = 0.0;
	double beamwidth_xtrack;
	double beamwidth_ltrack;

	/* bottom layout parameters */
	int nangle = MB7K2SS_NUM_ANGLES;
	double angle_min = -MB7K2SS_ANGLE_MAX;
	double angle_max = MB7K2SS_ANGLE_MAX;
	double table_angle[MB7K2SS_NUM_ANGLES];
	double table_xtrack[MB7K2SS_NUM_ANGLES];
	double table_ltrack[MB7K2SS_NUM_ANGLES];
	double table_altitude[MB7K2SS_NUM_ANGLES];
	double table_range[MB7K2SS_NUM_ANGLES];

	bool file_in_bounds = false;

	/***** do first pass gridding *****/
	if (grid_mode == MBMOSAIC_SINGLE_BEST || priority_mode != MBMOSAIC_PRIORITY_NONE) {

		/* read in data */
		void *datalist = nullptr;
		int ndata = 0;
		const int look_processed = MB_DATALIST_LOOK_UNSET;
		if (mb_datalist_open(verbose, &datalist, filelist, look_processed, &error) != MB_SUCCESS) {
			fprintf(outfp, "\nUnable to open data list file: %s\n", filelist);
			fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
			mb_memory_clear(verbose, &error);
			exit(MB_ERROR_OPEN_FAIL);
		}
		int pstatus;
  		int astatus = MB_ALTNAV_NONE;
		mb_path path = "";
		mb_path ppath = "";
		mb_path apath = "";
		mb_path dpath = "";
		double file_weight = 1.0;
		while (mb_datalist_read3(verbose, datalist, &pstatus, path, ppath, 
									&astatus, apath, dpath, &format, &file_weight, &error) ==
		       MB_SUCCESS) {
			int ndatafile = 0;

			/* if format > 0 then input is multibeam file */
			if (format > 0) {
				/* apply pstatus */
				if (pstatus == MB_PROCESSED_USE)
					strcpy(file, ppath);
				else
					strcpy(file, path);

				/* check for mbinfo file - get file bounds if possible */
				status = mb_check_info(verbose, file, lonflip, bounds, &file_in_bounds, &error);
				if (status == MB_FAILURE) {
					file_in_bounds = true;
					status = MB_SUCCESS;
					error = MB_ERROR_NO_ERROR;
				}

				/* initialize the multibeam file */
				if (file_in_bounds) {
					/* check for filtered amplitude or sidescan file */
					if (usefiltered && datatype == MBMOSAIC_DATA_AMPLITUDE) {
						if ((status = mb_get_ffa(verbose, file, &format, &error)) != MB_SUCCESS) {
							char *message = nullptr;
							mb_error(verbose, error, &message);
							fprintf(stderr, "\nMBIO Error returned from function <mb_get_ffa>:\n%s\n", message);
							fprintf(stderr, "Requested filtered amplitude file missing\n");
							fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", file);
							fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
							exit(error);
						}
					}
					else if (usefiltered && datatype == MBMOSAIC_DATA_SIDESCAN) {
						if ((status = mb_get_ffs(verbose, file, &format, &error)) != MB_SUCCESS) {
							char *message = nullptr;
							mb_error(verbose, error, &message);
							fprintf(stderr, "\nMBIO Error returned from function <mb_get_ffs>:\n%s\n", message);
							fprintf(stderr, "Requested filtered sidescan file missing\n");
							fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", file);
							fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
							exit(error);
						}
					}

					/* open the file */
					if (mb_read_init_altnav(verbose, file, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap,
					                           astatus, apath, &mbio_ptr, &btime_d, &etime_d, 
					                           &beams_bath, &beams_amp, &pixels_ss, &error) !=
					    MB_SUCCESS) {
						char *message = nullptr;
						mb_error(verbose, error, &message);
						fprintf(outfp, "\nMBIO Error returned from function <mb_read_init_altnav>:\n%s\n", message);
						fprintf(outfp, "\nMultibeam File <%s> not initialized for reading\n", file);
						fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
						mb_memory_clear(verbose, &error);
						exit(error);
					}

					/* get pointers to data storage */
					mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
					store_ptr = mb_io_ptr->store_data;

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
						status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
						                           (void **)&bathacrosstrack, &error);
					if (error == MB_ERROR_NO_ERROR)
						status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
						                           (void **)&bathalongtrack, &error);
					if (error == MB_ERROR_NO_ERROR)
						status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlon,
						                           &error);
					if (error == MB_ERROR_NO_ERROR)
						status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlat,
						                           &error);
					if (error == MB_ERROR_NO_ERROR)
						status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
					if (error == MB_ERROR_NO_ERROR)
						status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
						                           (void **)&ssacrosstrack, &error);
					if (error == MB_ERROR_NO_ERROR)
						status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
						                           (void **)&ssalongtrack, &error);
					if (error == MB_ERROR_NO_ERROR)
						status =
						    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslon, &error);
					if (error == MB_ERROR_NO_ERROR)
						status =
						    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslat, &error);
					if (datatype != MBMOSAIC_DATA_SIDESCAN) {
						if (error == MB_ERROR_NO_ERROR)
							status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double),
							                           (void **)&gangles, &error);
						if (error == MB_ERROR_NO_ERROR)
							status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&slopes,
							                           &error);
						if (error == MB_ERROR_NO_ERROR)
							status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double),
							                           (void **)&priorities, &error);
						if (error == MB_ERROR_NO_ERROR)
							status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(struct footprint),
							                           (void **)&footprints, &error);
					}
					else {
						if (error == MB_ERROR_NO_ERROR)
							status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&gangles,
							                           &error);
						if (error == MB_ERROR_NO_ERROR)
							status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
							                           (void **)&priorities, &error);
						if (error == MB_ERROR_NO_ERROR)
							status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(struct footprint),
							                           (void **)&footprints, &error);
					}
					if (error == MB_ERROR_NO_ERROR)
						status =
						    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&work1, &error);
					if (error == MB_ERROR_NO_ERROR)
						status =
						    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&work2, &error);

					/* if error initializing memory then quit */
					if (error != MB_ERROR_NO_ERROR) {
						char *message = nullptr;
						mb_error(verbose, error, &message);
						fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
						fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
						mb_memory_clear(verbose, &error);
						exit(error);
					}

					/* loop over reading */
					while (error <= MB_ERROR_NO_ERROR) {
						status =
						    mb_get_all(verbose, mbio_ptr, &store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
						               &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath,
						               amp, bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

						/* time gaps are not a problem here */
						if (error == MB_ERROR_TIME_GAP) {
							error = MB_ERROR_NO_ERROR;
							status = MB_SUCCESS;
						}

						if (verbose >= 2) {
							fprintf(stderr, "\ndbg2  Ping read in program <%s>\n", program_name);
							fprintf(stderr, "dbg2       kind:           %d\n", kind);
							fprintf(stderr, "dbg2       beams_bath:     %d\n", beams_bath);
							fprintf(stderr, "dbg2       beams_amp:      %d\n", beams_amp);
							fprintf(stderr, "dbg2       pixels_ss:      %d\n", pixels_ss);
							fprintf(stderr, "dbg2       error:          %d\n", error);
							fprintf(stderr, "dbg2       status:         %d\n", status);
						}

						if (status == MB_SUCCESS && kind == MB_DATA_DATA) {
							/* get attitude using mb_extract_nav(), but do not overwrite the navigation that 
								may derive from an alternative navigation source */
							double tnavlon, tnavlat, tspeed, theading;
							status = mb_extract_nav(verbose, mbio_ptr, store_ptr, &kind, time_i, &time_d, &tnavlon, &tnavlat,
							                        &tspeed, &theading, &draft, &roll, &pitch, &heave, &error);

							/* get factors for lon lat calculations */
							if (error == MB_ERROR_NO_ERROR) {
								mb_coor_scale(verbose, navlat, &mtodeglon, &mtodeglat);
								headingx = sin(DTR * heading);
								headingy = cos(DTR * heading);
							}

							/* get beam widths */
							if (error == MB_ERROR_NO_ERROR) {
								status = mb_beamwidths(verbose, mbio_ptr, &beamwidth_xtrack, &beamwidth_ltrack, &error);
							}

							/* mosaic beam based data (amplitude, grazing angle, slope) */
							if (use_beams && error == MB_ERROR_NO_ERROR) {
								/* translate beam locations to lon/lat */
								for (int ib = 0; ib < beams_amp; ib++) {
									if (mb_beam_ok(beamflag[ib])) {
										/* handle regular beams */
										bathlon[ib] = navlon + headingy * mtodeglon * bathacrosstrack[ib] +
										              headingx * mtodeglon * bathalongtrack[ib];
										bathlat[ib] = navlat - headingx * mtodeglat * bathacrosstrack[ib] +
										              headingy * mtodeglat * bathalongtrack[ib];

										/* get footprints */
										mbmosaic_get_footprint(verbose, MBMOSAIC_FOOTPRINT_REAL, beamwidth_xtrack,
										                       beamwidth_ltrack, (bath[ib] - sensordepth), bathacrosstrack[ib],
										                       bathalongtrack[ib], 0.0, &footprints[ib], &error);
										for (int j = 0; j < 4; j++) {
											xx = navlon + headingy * mtodeglon * footprints[ib].x[j] +
											     headingx * mtodeglon * footprints[ib].y[j];
											yy = navlat - headingx * mtodeglat * footprints[ib].x[j] +
											     headingy * mtodeglat * footprints[ib].y[j];
											footprints[ib].x[j] = xx;
											footprints[ib].y[j] = yy;
										}
									}
								}

								/* get beam angles */
								mbmosaic_get_beamangles(verbose, sensordepth, beams_bath, beamflag, bath, bathacrosstrack,
								                        bathalongtrack, gangles, &error);

								/* get priorities */
								mbmosaic_get_beampriorities(verbose, priority_mode, n_priority_angle, priority_angle_angle,
								                            priority_angle_priority, priority_azimuth, priority_azimuth_factor,
								                            priority_heading, priority_heading_factor, heading, beams_bath,
								                            beamflag, gangles, priorities, &error);

								/* get bathymetry slopes if needed */
								if (use_slope)
									mbmosaic_get_beamslopes(verbose, beams_bath, beamflag, bath, bathacrosstrack, slopes, &error);

								/* reproject beam positions if necessary */
								if (use_projection) {
									for (int ib = 0; ib < beams_amp; ib++)
										if (mb_beam_ok(beamflag[ib])) {
											mb_proj_forward(verbose, pjptr, bathlon[ib], bathlat[ib], &bathlon[ib], &bathlat[ib],
											                &error);
											for (int j = 0; j < 4; j++) {
												mb_proj_forward(verbose, pjptr, footprints[ib].x[j], footprints[ib].y[j],
												                &footprints[ib].x[j], &footprints[ib].y[j], &error);
											}
										}
								}

								/* deal with data */
								for (int ib = 0; ib < beams_amp; ib++)
									if (mb_beam_ok(beamflag[ib])) {
										int ixx[4];
										int iyy[4];
										/* get position in grid */
										for (int j = 0; j < 4; j++) {
											ixx[j] = (footprints[ib].x[j] - wbnd[0] + 0.5 * dx) / dx;
											iyy[j] = (footprints[ib].y[j] - wbnd[2] + 0.5 * dy) / dy;
										}
										int ix1 = ixx[0];
										int iy1 = iyy[0];
										int ix2 = ixx[0];
										int iy2 = iyy[0];
										for (int j = 1; j < 4; j++) {
											ix1 = std::min(ix1, ixx[j]);
											iy1 = std::min(iy1, iyy[j]);
											ix2 = std::max(ix2, ixx[j]);
											iy2 = std::max(iy2, iyy[j]);
										}
										ix1 = std::max(ix1, 0);
										ix2 = std::min(ix2, gxdim - 1);
										iy1 = std::max(iy1, 0);
										iy2 = std::min(iy2, gydim - 1);

										/* process if in region of interest */
										for (int ii = ix1; ii <= ix2; ii++)
											for (int jj = iy1; jj <= iy2; jj++) {
												/* set grid if highest weight */
												const int kgrid = ii * gydim + jj;
												xx = dx * ii + wbnd[0];
												yy = dy * jj + wbnd[2];
												const int inside = mb_pr_point_in_quad(verbose, xx, yy, footprints[ib].x, footprints[ib].y,
												                             &error);
												if (inside && priorities[ib] > maxpriority[kgrid]) {
													if (datatype == MBMOSAIC_DATA_AMPLITUDE)
														grid[kgrid] = amp[ib];
													else if (datatype == MBMOSAIC_DATA_FLAT_GRAZING) {
														if (gangles[ib] > 0)
															grid[kgrid] = gangles[ib];
														else
															grid[kgrid] = -gangles[ib];
													}
													else if (datatype == MBMOSAIC_DATA_GRAZING) {
														double slope = slopes[ib] + gangles[ib];
														if (slope < 0)
															slope = -slope;
														grid[kgrid] = slope;
													}
													else if (datatype == MBMOSAIC_DATA_SLOPE) {
														double slope = slopes[ib];
														if (slope < 0)
															slope = -slope;
														grid[kgrid] = slope;
													}

													cnt[kgrid] = 1;
													maxpriority[kgrid] = priorities[ib];
												}
											}
										ndata++;
										ndatafile++;
									}
							}

							/* mosaic sidescan */
							else if (datatype == MBMOSAIC_DATA_SIDESCAN && error == MB_ERROR_NO_ERROR) {
								/* get spacing */
								double xsmin = 0.0;
								double xsmax = 0.0;
								int ismin = pixels_ss / 2;
								int ismax = pixels_ss / 2;
								for (int ib = 0; ib < pixels_ss; ib++) {
									if (ss[ib] > MB_SIDESCAN_NULL) {
										if (ssacrosstrack[ib] < xsmin) {
											xsmin = ssacrosstrack[ib];
											ismin = ib;
										}
										if (ssacrosstrack[ib] > xsmax) {
											xsmax = ssacrosstrack[ib];
											ismax = ib;
										}
									}
								}
								int footprint_mode;
								double acrosstrackspacing;
								if (ismax > ismin) {
									footprint_mode = MBMOSAIC_FOOTPRINT_SPACING;
									acrosstrackspacing = (xsmax - xsmin) / (ismax - ismin);
								}
								else {
									footprint_mode = MBMOSAIC_FOOTPRINT_REAL;
									acrosstrackspacing = 0.0;
								}

								/* translate pixel locations to lon/lat */
								for (int ib = 0; ib < pixels_ss; ib++) {
									if (ss[ib] > MB_SIDESCAN_NULL) {
										sslon[ib] = navlon + headingy * mtodeglon * ssacrosstrack[ib] +
										            headingx * mtodeglon * ssalongtrack[ib];
										sslat[ib] = navlat - headingx * mtodeglat * ssacrosstrack[ib] +
										            headingy * mtodeglat * ssalongtrack[ib];

										/* get footprints */
										mbmosaic_get_footprint(verbose, footprint_mode, beamwidth_xtrack, beamwidth_ltrack,
										                       altitude, ssacrosstrack[ib], ssalongtrack[ib], acrosstrackspacing,
										                       &footprints[ib], &error);
										for (int j = 0; j < 4; j++) {
											xx = navlon + headingy * mtodeglon * footprints[ib].x[j] +
											     headingx * mtodeglon * footprints[ib].y[j];
											yy = navlat - headingx * mtodeglat * footprints[ib].x[j] +
											     headingy * mtodeglat * footprints[ib].y[j];
											footprints[ib].x[j] = xx;
											footprints[ib].y[j] = yy;
										}
									}
								}

								/* get angle vs acrosstrack distance table using topographic grid */
								int table_error = MB_ERROR_NO_ERROR;
								int table_status = MB_SUCCESS;
								if (usetopogrid) {
									table_status = mb_topogrid_getangletable(verbose, topogrid_ptr, nangle, angle_min, angle_max,
									                                         navlon, navlat, heading, altitude, sensordepth, pitch,
									                                         table_angle, table_xtrack, table_ltrack,
									                                         table_altitude, table_range, &table_error);
									if (table_status == MB_FAILURE) {
										char *message = nullptr;
										mb_error(verbose, table_error, &message);
										fprintf(outfp, "\nMBIO Error extracting topography from grid for sidescan:\n%s\n",
										        message);
										fprintf(outfp, "\nNonfatal error in program <%s>\n", program_name);
										fprintf(outfp,
										        "Requested angle-distance table extends beyond the bounds of the topography grid "
										        "<%s>\n",
										        topogridfile);
										fprintf(outfp,
										        "used for grazing angle calculation - flat bottom calculation used in places.\n");
										table_status = MB_SUCCESS;
										table_error = MB_ERROR_NO_ERROR;
									}
								}

								/* get angle vs acrosstrack distance table using bathymetry from the swath file with sidescan */
								else {
									table_status = mbmosaic_bath_getangletable(
									    verbose, sensordepth, beams_bath, beamflag, bath, bathacrosstrack, bathalongtrack,
									    angle_min, angle_max, nangle, table_angle, table_xtrack, table_ltrack, table_altitude,
									    table_range, &table_error);
								}

								/* if need be, calculate angles using flat bottom layout and nadir altitude */
								if (table_status == MB_FAILURE) {
									if (altitude <= 0.0)
										altitude = altitude_default;
									table_status = mbmosaic_flatbottom_getangletable(
									    verbose, altitude, angle_min, angle_max, nangle, table_angle, table_xtrack, table_ltrack,
									    table_altitude, table_range, &table_error);
								}

								/* get angles for each pixel */
								mbmosaic_get_ssangles(verbose, nangle, table_angle, table_xtrack, table_ltrack, table_altitude,
								                      table_range, pixels_ss, ss, ssacrosstrack, gangles, &error);

								/* get priorities for each pixel */
								mbmosaic_get_sspriorities(verbose, priority_mode, n_priority_angle, priority_angle_angle,
								                          priority_angle_priority, priority_azimuth, priority_azimuth_factor,
								                          priority_heading, priority_heading_factor, heading, pixels_ss, ss,
								                          gangles, priorities, &error);

								/* reproject pixel positions if necessary */
								if (use_projection) {
									for (int ib = 0; ib < pixels_ss; ib++)
										if (ss[ib] > MB_SIDESCAN_NULL) {
											mb_proj_forward(verbose, pjptr, sslon[ib], sslat[ib], &sslon[ib], &sslat[ib], &error);
											for (int j = 0; j < 4; j++) {
												mb_proj_forward(verbose, pjptr, footprints[ib].x[j], footprints[ib].y[j],
												                &footprints[ib].x[j], &footprints[ib].y[j], &error);
											}
										}
								}

								/* deal with data */
								for (int ib = 0; ib < pixels_ss; ib++)
									if (ss[ib] > MB_SIDESCAN_NULL) {
										int ixx[4];
										int iyy[4];
										/* get position in grid */
										for (int j = 0; j < 4; j++) {
											ixx[j] = (footprints[ib].x[j] - wbnd[0] + 0.5 * dx) / dx;
											iyy[j] = (footprints[ib].y[j] - wbnd[2] + 0.5 * dy) / dy;
										}
										int ix1 = ixx[0];
										int iy1 = iyy[0];
										int ix2 = ixx[0];
										int iy2 = iyy[0];
										for (int j = 1; j < 4; j++) {
											ix1 = std::min(ix1, ixx[j]);
											iy1 = std::min(iy1, iyy[j]);
											ix2 = std::max(ix2, ixx[j]);
											iy2 = std::max(iy2, iyy[j]);
										}
										ix1 = std::max(ix1, 0);
										ix2 = std::min(ix2, gxdim - 1);
										iy1 = std::max(iy1, 0);
										iy2 = std::min(iy2, gydim - 1);

										/* process if in region of interest */
										for (int ii = ix1; ii <= ix2; ii++)
											for (int jj = iy1; jj <= iy2; jj++) {
												/* set grid if highest weight */
												const int kgrid = ii * gydim + jj;
												xx = dx * ii + wbnd[0];
												yy = dy * jj + wbnd[2];
												const int inside = mb_pr_point_in_quad(verbose, xx, yy, footprints[ib].x, footprints[ib].y,
												                             &error);
												if (inside && priorities[ib] > maxpriority[kgrid]) {
													grid[kgrid] = ss[ib];
													cnt[kgrid] = 1;
													maxpriority[kgrid] = priorities[ib];
												}
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
				if (verbose > 0 || file_in_bounds)
					fprintf(outfp, "%u data points processed in %s\n", ndatafile, file);

				/* add to datalist if data actually contributed */
				if (grid_mode != MBMOSAIC_AVERAGE && ndatafile > 0 && dfp != nullptr) {
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
			fprintf(outfp, "\n%u total data points processed in highest weight pass\n", ndata);
		if (verbose > 0 && grid_mode == MBMOSAIC_AVERAGE)
			fprintf(outfp, "\n");
	}
	/***** end of first pass gridding *****/

	double clipvalue = NO_DATA_FLAG;
	char ofile[2*MB_PATH_MAXLINE+100] = "";
	char plot_cmd[4*MB_COMMENT_MAXLINE] = "";
	int plot_status;


	/* grid variables */
	float *sdata = nullptr;
	float *sgrid = nullptr;
	double sxmin, symin;
	float xmin, ymin, ddx, ddy, zflag, cay;
	void *work3 = nullptr;
	double zmin, zmax, zclip;
	int nmax;
	double smin, smax;
	int nbinset, nbinzero, nbinspline;

	/* output char strings */
	char xlabel[1050] = "";
	char ylabel[1050] = "";
	char zlabel[1050] = "";
	mb_path title = "";
	mb_path nlabel = "";
	mb_path sdlabel = "";

	/* other variables */
	double norm_weight;
	// int ir;
	double r;
	int dmask[9];
	// int kint;
	// int ix1, ix2, iy1, iy2;

	/***** do second pass gridding *****/
	if (grid_mode == MBMOSAIC_AVERAGE) {
		/* initialize arrays */
		for (int i = 0; i < gxdim; i++)
			for (int j = 0; j < gydim; j++) {
				const int kgrid = i * gydim + j;
				grid[kgrid] = 0.0;
				cnt[kgrid] = 0;
				sigma[kgrid] = 0.0;
			}

		/* read in data */
		int ndata = 0;
		void *datalist = nullptr;
		const int look_processed = MB_DATALIST_LOOK_UNSET;
		if (mb_datalist_open(verbose, &datalist, filelist, look_processed, &error) != MB_SUCCESS) {
			error = MB_ERROR_OPEN_FAIL;
			fprintf(outfp, "\nUnable to open data list file: %s\n", filelist);
			fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
			mb_memory_clear(verbose, &error);
			exit(MB_ERROR_OPEN_FAIL);
		}
		int pstatus;
		int astatus;
		mb_path path = "";
		mb_path ppath = "";
		mb_path apath = "";
		mb_path dpath = "";
		double file_weight = 1.0;
		while (mb_datalist_read3(verbose, datalist, &pstatus, path, ppath, 
									&astatus, apath, dpath, &format, &file_weight, &error) ==
		       MB_SUCCESS) {
			int ndatafile = 0;

			/* if format > 0 then input is multibeam file */
			if (format > 0 && file[0] != '#') {
				/* apply pstatus */
				if (pstatus == MB_PROCESSED_USE)
					strcpy(file, ppath);
				else
					strcpy(file, path);

				/* check for mbinfo file - get file bounds if possible */
				status = mb_check_info(verbose, file, lonflip, bounds, &file_in_bounds, &error);
				if (status == MB_FAILURE) {
					file_in_bounds = true;
					status = MB_SUCCESS;
					error = MB_ERROR_NO_ERROR;
				}

				/* initialize the multibeam file */
				if (file_in_bounds) {
					/* check for filtered amplitude or sidescan file */
					if (usefiltered && datatype == MBMOSAIC_DATA_AMPLITUDE) {
						if ((status = mb_get_ffa(verbose, file, &format, &error)) != MB_SUCCESS) {
							char *message = nullptr;
							mb_error(verbose, error, &message);
							fprintf(stderr, "\nMBIO Error returned from function <mb_get_ffa>:\n%s\n", message);
							fprintf(stderr, "Requested filtered amplitude file missing\n");
							fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", file);
							fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
							exit(error);
						}
					}
					else if (usefiltered && datatype == MBMOSAIC_DATA_SIDESCAN) {
						if ((status = mb_get_ffs(verbose, file, &format, &error)) != MB_SUCCESS) {
							char *message = nullptr;
							mb_error(verbose, error, &message);
							fprintf(stderr, "\nMBIO Error returned from function <mb_get_ffa>:\n%s\n", message);
							fprintf(stderr, "Requested filtered sidescan file missing\n");
							fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", file);
							fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
							exit(error);
						}
					}

					/* open the file */
					if (mb_read_init_altnav(verbose, file, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap,
					                           astatus, apath, &mbio_ptr, &btime_d, &etime_d, 
					                           &beams_bath, &beams_amp, &pixels_ss, &error) !=
					    MB_SUCCESS) {
						char *message = nullptr;
						mb_error(verbose, error, &message);
						fprintf(outfp, "\nMBIO Error returned from function <mb_read_init_altnav>:\n%s\n", message);
						fprintf(outfp, "\nMultibeam File <%s> not initialized for reading\n", file);
						fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
						mb_memory_clear(verbose, &error);
						exit(error);
					}

					/* get pointers to data storage */
					mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
					store_ptr = mb_io_ptr->store_data;

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
						status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
						                           (void **)&bathacrosstrack, &error);
					if (error == MB_ERROR_NO_ERROR)
						status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
						                           (void **)&bathalongtrack, &error);
					if (error == MB_ERROR_NO_ERROR)
						status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlon,
						                           &error);
					if (error == MB_ERROR_NO_ERROR)
						status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlat,
						                           &error);
					if (error == MB_ERROR_NO_ERROR)
						status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
					if (error == MB_ERROR_NO_ERROR)
						status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
						                           (void **)&ssacrosstrack, &error);
					if (error == MB_ERROR_NO_ERROR)
						status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
						                           (void **)&ssalongtrack, &error);
					if (error == MB_ERROR_NO_ERROR)
						status =
						    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslon, &error);
					if (error == MB_ERROR_NO_ERROR)
						status =
						    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslat, &error);
					if (datatype != MBMOSAIC_DATA_SIDESCAN) {
						if (error == MB_ERROR_NO_ERROR)
							status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double),
							                           (void **)&gangles, &error);
						if (error == MB_ERROR_NO_ERROR)
							status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double),
							                           (void **)&priorities, &error);
						if (error == MB_ERROR_NO_ERROR)
							status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(struct footprint),
							                           (void **)&footprints, &error);
					}
					else {
						if (error == MB_ERROR_NO_ERROR)
							status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&gangles,
							                           &error);
						if (error == MB_ERROR_NO_ERROR)
							status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
							                           (void **)&priorities, &error);
						if (error == MB_ERROR_NO_ERROR)
							status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(struct footprint),
							                           (void **)&footprints, &error);
					}
					if (error == MB_ERROR_NO_ERROR)
						status =
						    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&work1, &error);
					if (error == MB_ERROR_NO_ERROR)
						status =
						    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&work2, &error);

					/* if error initializing memory then quit */
					if (error != MB_ERROR_NO_ERROR) {
						char *message = nullptr;
						mb_error(verbose, error, &message);
						fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
						fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
						mb_memory_clear(verbose, &error);
						exit(error);
					}

					/* loop over reading */
					while (error <= MB_ERROR_NO_ERROR) {
						status =
						    mb_get_all(verbose, mbio_ptr, &store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
						               &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath,
						               amp, bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

						/* time gaps are not a problem here */
						if (error == MB_ERROR_TIME_GAP) {
							error = MB_ERROR_NO_ERROR;
							status = MB_SUCCESS;
						}

						if (verbose >= 2) {
							fprintf(stderr, "\ndbg2  Ping read in program <%s>\n", program_name);
							fprintf(stderr, "dbg2       kind:           %d\n", kind);
							fprintf(stderr, "dbg2       beams_bath:     %d\n", beams_bath);
							fprintf(stderr, "dbg2       beams_amp:      %d\n", beams_amp);
							fprintf(stderr, "dbg2       pixels_ss:      %d\n", pixels_ss);
							fprintf(stderr, "dbg2       error:          %d\n", error);
							fprintf(stderr, "dbg2       status:         %d\n", status);
						}

						if (status == MB_SUCCESS && kind == MB_DATA_DATA) {
							/* get attitude using mb_extract_nav(), but do not overwrite the navigation that 
								may derive from an alternative navigation source */
							double tnavlon, tnavlat, tspeed, theading;
							status = mb_extract_nav(verbose, mbio_ptr, store_ptr, &kind, time_i, &time_d, &tnavlon, &tnavlat,
							                        &tspeed, &theading, &draft, &roll, &pitch, &heave, &error);

							/* get factors for lon lat calculations */
							if (error == MB_ERROR_NO_ERROR) {
								mb_coor_scale(verbose, navlat, &mtodeglon, &mtodeglat);
								headingx = sin(DTR * heading);
								headingy = cos(DTR * heading);
							}

							/* get beam widths */
							if (error == MB_ERROR_NO_ERROR) {
								status = mb_beamwidths(verbose, mbio_ptr, &beamwidth_xtrack, &beamwidth_ltrack, &error);
							}

							/* mosaic beam based data (amplitude, grazing angle, slope) */
							if (use_beams && error == MB_ERROR_NO_ERROR) {

								/* translate beam locations to lon/lat */
								for (int ib = 0; ib < beams_amp; ib++) {
									if (mb_beam_ok(beamflag[ib])) {
										bathlon[ib] = navlon + headingy * mtodeglon * bathacrosstrack[ib] +
										              headingx * mtodeglon * bathalongtrack[ib];
										bathlat[ib] = navlat - headingx * mtodeglat * bathacrosstrack[ib] +
										              headingy * mtodeglat * bathalongtrack[ib];

										/* get footprints */
										mbmosaic_get_footprint(verbose, MBMOSAIC_FOOTPRINT_REAL, beamwidth_xtrack,
										                       beamwidth_ltrack, (bath[ib] - sensordepth), bathacrosstrack[ib],
										                       bathalongtrack[ib], 0.0, &footprints[ib], &error);
										for (int j = 0; j < 4; j++) {
											xx = navlon + headingy * mtodeglon * footprints[ib].x[j] +
											     headingx * mtodeglon * footprints[ib].y[j];
											yy = navlat - headingx * mtodeglat * footprints[ib].x[j] +
											     headingy * mtodeglat * footprints[ib].y[j];
											footprints[ib].x[j] = xx;
											footprints[ib].y[j] = yy;
										}
									}
								}

								/* get beam angles */
								mbmosaic_get_beamangles(verbose, sensordepth, beams_bath, beamflag, bath, bathacrosstrack,
								                        bathalongtrack, gangles, &error);

								/* get priorities */
								mbmosaic_get_beampriorities(verbose, priority_mode, n_priority_angle, priority_angle_angle,
								                            priority_angle_priority, priority_azimuth, priority_azimuth_factor,
								                            priority_heading, priority_heading_factor, heading, beams_bath,
								                            beamflag, gangles, priorities, &error);

								/* get bathymetry slopes if needed */
								if (use_slope)
									mbmosaic_get_beamslopes(verbose, beams_bath, beamflag, bath, bathacrosstrack, slopes, &error);

								/* reproject beam positions if necessary */
								if (use_projection) {
									for (int ib = 0; ib < beams_amp; ib++)
										if (mb_beam_ok(beamflag[ib])) {
											mb_proj_forward(verbose, pjptr, bathlon[ib], bathlat[ib], &bathlon[ib], &bathlat[ib],
											                &error);
											for (int j = 0; j < 4; j++) {
												mb_proj_forward(verbose, pjptr, footprints[ib].x[j], footprints[ib].y[j],
												                &footprints[ib].x[j], &footprints[ib].y[j], &error);
											}
										}
								}

								/* deal with data */
								for (int ib = 0; ib < beams_amp; ib++)
									if (mb_beam_ok(beamflag[ib])) {
										int ixx[4];
										int iyy[4];
										/* get position in grid */
										for (int j = 0; j < 4; j++) {
											ixx[j] = (footprints[ib].x[j] - wbnd[0] + 0.5 * dx) / dx;
											iyy[j] = (footprints[ib].y[j] - wbnd[2] + 0.5 * dy) / dy;
										}
										int ix1 = ixx[0];
										int iy1 = iyy[0];
										int ix2 = ixx[0];
										int iy2 = iyy[0];
										for (int j = 1; j < 4; j++) {
											ix1 = std::min(ix1, ixx[j]);
											iy1 = std::min(iy1, iyy[j]);
											ix2 = std::max(ix2, ixx[j]);
											iy2 = std::max(iy2, iyy[j]);
										}
										ix1 = std::max(ix1, 0);
										ix2 = std::min(ix2, gxdim - 1);
										iy1 = std::max(iy1, 0);
										iy2 = std::min(iy2, gydim - 1);

										/* process if in region of interest */
										for (int ii = ix1; ii <= ix2; ii++)
											for (int jj = iy1; jj <= iy2; jj++) {
												/* add to cell if weight high enough */
												const int kgrid = ii * gydim + jj;
												xx = dx * ii + wbnd[0];
												yy = dy * jj + wbnd[2];
												const int inside = mb_pr_point_in_quad(verbose, xx, yy, footprints[ib].x, footprints[ib].y,
												                             &error);
												if (inside && priorities[ib] > 0.0 &&
												    priorities[ib] >= maxpriority[kgrid] - priority_range) {
													xx = wbnd[0] + ii * dx - bathlon[ib];
													yy = wbnd[2] + jj * dy - bathlat[ib];
													norm_weight = file_weight * exp(-(xx * xx + yy * yy) * gaussian_factor);
													if (weight_priorities == 1)
														norm_weight *= priorities[ib];
													else if (weight_priorities == 2)
														norm_weight *= priorities[ib] * priorities[ib];
													norm[kgrid] += norm_weight;
													if (datatype == MBMOSAIC_DATA_AMPLITUDE) {
														grid[kgrid] += norm_weight * amp[ib];
														sigma[kgrid] += norm_weight * amp[ib] * amp[ib];
													}
													else if (datatype == MBMOSAIC_DATA_FLAT_GRAZING) {
														if (gangles[ib] > 0)
															grid[kgrid] += norm_weight * gangles[ib];
														else
															grid[kgrid] -= norm_weight * gangles[ib];
														sigma[kgrid] += norm_weight * gangles[ib] * gangles[ib];
													}
													else if (datatype == MBMOSAIC_DATA_GRAZING) {
														double slope = slopes[ib] + gangles[ib];
														if (slope < 0)
															slope = -slope;
														grid[kgrid] += norm_weight * slope;
														sigma[kgrid] += norm_weight * slope * slope;
													}
													else if (datatype == MBMOSAIC_DATA_SLOPE) {
														double slope = slopes[ib];
														if (slope < 0)
															slope = -slope;
														grid[kgrid] += norm_weight * slope;
														sigma[kgrid] += norm_weight * slope * slope;
													}
													cnt[kgrid]++;
												}
											}
										ndata++;
										ndatafile++;
									}
							}

							/* mosaic sidescan */
							else if (datatype == MBMOSAIC_DATA_SIDESCAN && error == MB_ERROR_NO_ERROR) {
								/* get spacing */
								double xsmin = 0.0;
								double xsmax = 0.0;
								int ismin = pixels_ss / 2;
								int ismax = pixels_ss / 2;
								for (int ib = 0; ib < pixels_ss; ib++) {
									if (ss[ib] > MB_SIDESCAN_NULL) {
										if (ssacrosstrack[ib] < xsmin) {
											xsmin = ssacrosstrack[ib];
											ismin = ib;
										}
										if (ssacrosstrack[ib] > xsmax) {
											xsmax = ssacrosstrack[ib];
											ismax = ib;
										}
									}
								}
								int footprint_mode;
								double acrosstrackspacing;
								if (ismax > ismin) {
									footprint_mode = MBMOSAIC_FOOTPRINT_SPACING;
									acrosstrackspacing = (xsmax - xsmin) / (ismax - ismin);
								}
								else {
									footprint_mode = MBMOSAIC_FOOTPRINT_REAL;
									acrosstrackspacing = 0.0;
								}

								/* translate pixel locations to lon/lat */
								for (int ib = 0; ib < pixels_ss; ib++) {
									if (ss[ib] > MB_SIDESCAN_NULL) {
										sslon[ib] = navlon + headingy * mtodeglon * ssacrosstrack[ib] +
										            headingx * mtodeglon * ssalongtrack[ib];
										sslat[ib] = navlat - headingx * mtodeglat * ssacrosstrack[ib] +
										            headingy * mtodeglat * ssalongtrack[ib];

										/* get footprints */
										mbmosaic_get_footprint(verbose, footprint_mode, beamwidth_xtrack, beamwidth_ltrack,
										                       altitude, ssacrosstrack[ib], ssalongtrack[ib], acrosstrackspacing,
										                       &footprints[ib], &error);
										for (int j = 0; j < 4; j++) {
											xx = navlon + headingy * mtodeglon * footprints[ib].x[j] +
											     headingx * mtodeglon * footprints[ib].y[j];
											yy = navlat - headingx * mtodeglat * footprints[ib].x[j] +
											     headingy * mtodeglat * footprints[ib].y[j];
											footprints[ib].x[j] = xx;
											footprints[ib].y[j] = yy;
										}
									}
								}

								/* get angle vs acrosstrack distance table using topographic grid */
								int table_error = MB_ERROR_NO_ERROR;
								int table_status = MB_SUCCESS;
								if (usetopogrid) {
									table_status = mb_topogrid_getangletable(verbose, topogrid_ptr, nangle, angle_min, angle_max,
									                                         navlon, navlat, heading, altitude, sensordepth, pitch,
									                                         table_angle, table_xtrack, table_ltrack,
									                                         table_altitude, table_range, &table_error);
									if (table_status == MB_FAILURE) {
										char *message = nullptr;
										mb_error(verbose, table_error, &message);
										fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
										fprintf(outfp, "\nNonfatal error in program <%s>\n", program_name);
										fprintf(outfp, "Sidescan data extends beyond the bounds of the topography grid <%s>\n",
										        topogridfile);
										fprintf(outfp, "used for grazing angle calculation - the mosaic may be truncated.\n");
										table_status = MB_SUCCESS;
										table_error = MB_ERROR_NO_ERROR;
									}
								}

								/* get angle vs acrosstrack distance table using bathymetry from the swath file with sidescan */
								else {
									table_status = mbmosaic_bath_getangletable(
									    verbose, sensordepth, beams_bath, beamflag, bath, bathacrosstrack, bathalongtrack,
									    angle_min, angle_max, nangle, table_angle, table_xtrack, table_ltrack, table_altitude,
									    table_range, &table_error);
								}

								/* if need be, calculate angles using flat bottom layout and nadir altitude */
								if (table_status == MB_FAILURE) {
									if (altitude <= 0.0)
										altitude = altitude_default;
									table_status = mbmosaic_flatbottom_getangletable(
									    verbose, altitude, angle_min, angle_max, nangle, table_angle, table_xtrack, table_ltrack,
									    table_altitude, table_range, &table_error);
								}

								/* get angles for each pixel */
								mbmosaic_get_ssangles(verbose, nangle, table_angle, table_xtrack, table_ltrack, table_altitude,
								                      table_range, pixels_ss, ss, ssacrosstrack, gangles, &error);

								/* get priorities for each pixel */
								mbmosaic_get_sspriorities(verbose, priority_mode, n_priority_angle, priority_angle_angle,
								                          priority_angle_priority, priority_azimuth, priority_azimuth_factor,
								                          priority_heading, priority_heading_factor, heading, pixels_ss, ss,
								                          gangles, priorities, &error);

								/* reproject pixel positions if necessary */
								if (use_projection) {
									for (int ib = 0; ib < pixels_ss; ib++)
										if (ss[ib] > MB_SIDESCAN_NULL) {
											mb_proj_forward(verbose, pjptr, sslon[ib], sslat[ib], &sslon[ib], &sslat[ib], &error);
											for (int j = 0; j < 4; j++) {
												mb_proj_forward(verbose, pjptr, footprints[ib].x[j], footprints[ib].y[j],
												                &footprints[ib].x[j], &footprints[ib].y[j], &error);
											}
										}
								}

								/* deal with data */
								for (int ib = 0; ib < pixels_ss; ib++)
									if (ss[ib] > MB_SIDESCAN_NULL) {
										int ixx[4];
										int iyy[4];
										/* get position in grid */
										for (int j = 0; j < 4; j++) {
											ixx[j] = (footprints[ib].x[j] - wbnd[0] + 0.5 * dx) / dx;
											iyy[j] = (footprints[ib].y[j] - wbnd[2] + 0.5 * dy) / dy;
										}
										int ix1 = ixx[0];
										int iy1 = iyy[0];
										int ix2 = ixx[0];
										int iy2 = iyy[0];
										for (int j = 1; j < 4; j++) {
											ix1 = std::min(ix1, ixx[j]);
											iy1 = std::min(iy1, iyy[j]);
											ix2 = std::max(ix2, ixx[j]);
											iy2 = std::max(iy2, iyy[j]);
										}
										ix1 = std::max(ix1, 0);
										ix2 = std::min(ix2, gxdim - 1);
										iy1 = std::max(iy1, 0);
										iy2 = std::min(iy2, gydim - 1);

										/* process if in region of interest */
										for (int ii = ix1; ii <= ix2; ii++)
											for (int jj = iy1; jj <= iy2; jj++) {
												/* set grid if highest weight */
												const int kgrid = ii * gydim + jj;
												xx = dx * ii + wbnd[0];
												yy = dy * jj + wbnd[2];
												const int inside = mb_pr_point_in_quad(verbose, xx, yy, footprints[ib].x, footprints[ib].y,
												                             &error);
												if (inside && priorities[ib] > 0.0 &&
												    priorities[ib] >= maxpriority[kgrid] - priority_range) {
													xx = wbnd[0] + ii * dx - sslon[ib];
													yy = wbnd[2] + jj * dy - sslat[ib];
													norm_weight = file_weight * exp(-(xx * xx + yy * yy) * gaussian_factor);
													if (weight_priorities == 1)
														norm_weight *= priorities[ib];
													else if (weight_priorities == 2)
														norm_weight *= priorities[ib] * priorities[ib];
													grid[kgrid] += norm_weight * ss[ib];
													norm[kgrid] += norm_weight;
													sigma[kgrid] += norm_weight * ss[ib] * ss[ib];
													cnt[kgrid]++;
												}
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
				if (verbose > 0 || file_in_bounds)
					fprintf(outfp, "%u data points processed in %s\n", ndatafile, file);

				/* add to datalist if data actually contributed */
				if (ndatafile > 0 && dfp != nullptr) {
					if (pstatus == MB_PROCESSED_USE && astatus == MB_ALTNAV_USE)
			          	fprintf(dfp, "A:%s %d %f %s\n", path, format, file_weight, apath);
			        else if (pstatus == MB_PROCESSED_USE)
			          	fprintf(dfp, "P:%s %d %f\n", path, format, file_weight);
			        else
			          	fprintf(dfp, "R:%s %d %f\n", path, format, file_weight);
					fprintf(dfp, "%s %d %f\n", path, format, file_weight);
					fflush(dfp);
				}
			} /* end if (format > 0) */
		}
		if (datalist != nullptr)
			mb_datalist_close(verbose, &datalist, &error);
		if (verbose > 0)
			fprintf(outfp, "\n%u total data points processed in averaging pass\n", ndata);
	}
	/***** end of second pass gridding *****/

	/* close datalist if necessary */
	if (dfp != nullptr)
		fclose(dfp);

	/* deallocate topography grid array if necessary */
	if (usetopogrid)
		status = mb_topogrid_deall(verbose, &topogrid_ptr, &error);

	/* now loop over all points in the output grid */
	if (verbose >= 1)
		fprintf(outfp, "\nMaking raw grid...\n");
	nbinset = 0;
	nbinzero = 0;
	nbinspline = 0;

	/* deal with single best mode */
	if (grid_mode == MBMOSAIC_SINGLE_BEST) {
		for (int i = 0; i < gxdim; i++)
			for (int j = 0; j < gydim; j++) {
				const int kgrid = i * gydim + j;
				if (cnt[kgrid] > 0) {
					nbinset++;
				}
				else {
					grid[kgrid] = clipvalue;
				}
			}
	}
	else if (grid_mode == MBMOSAIC_AVERAGE) {
		for (int i = 0; i < gxdim; i++)
			for (int j = 0; j < gydim; j++) {
				const int kgrid = i * gydim + j;
				if (cnt[kgrid] > 0) {
					nbinset++;
					grid[kgrid] = grid[kgrid] / norm[kgrid];
					sigma[kgrid] = sqrt(fabs(sigma[kgrid] / norm[kgrid] - grid[kgrid] * grid[kgrid]));
				}
				else {
					grid[kgrid] = clipvalue;
				}
			}
	}

	/* if clip set do smooth interpolation */
	if (clipmode != MBMOSAIC_INTERP_NONE && clip > 0 && nbinset > 0) {
		/* set up data vector */
		int ndata = 0;
		if (border > 0.0)
			ndata = 2 * gxdim + 2 * gydim - 2;
		for (int i = 0; i < gxdim; i++)
			for (int j = 0; j < gydim; j++) {
				const int kgrid = i * gydim + j;
				if (grid[kgrid] < clipvalue)
					ndata++;
			}

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
			mb_memory_clear(verbose, &error);
			exit(MB_ERROR_MEMORY_FAIL);
		}
		memset((char *)sgrid, 0, gxdim * gydim * sizeof(float));

		/* get points from grid */
		sxmin = gbnd[0] - offx * dx;
		symin = gbnd[2] - offy * dy;
		ndata = 0;
		for (int i = 0; i < gxdim; i++)
			for (int j = 0; j < gydim; j++) {
				const int kgrid = i * gydim + j;
				if (grid[kgrid] < clipvalue) {
					sdata[ndata++] = (float)(sxmin + dx * i - bdata_origin_x);
					sdata[ndata++] = (float)(symin + dy * j - bdata_origin_y);
					sdata[ndata++] = (float)grid[kgrid];
				}
			}
		/* if desired set border */
		if (border > 0.0) {
			for (int i = 0; i < gxdim; i++) {
				int j = 0;
				int kgrid = i * gydim + j;
				if (grid[kgrid] == clipvalue) {
					sdata[ndata++] = (float)(sxmin + dx * i - bdata_origin_x);
					sdata[ndata++] = (float)(symin + dy * j - bdata_origin_y);
					sdata[ndata++] = (float)border;
				}
				j = gydim - 1;
				kgrid = i * gydim + j;
				if (grid[kgrid] == clipvalue) {
					sdata[ndata++] = (float)(sxmin + dx * i - bdata_origin_x);
					sdata[ndata++] = (float)(symin + dy * j - bdata_origin_y);
					sdata[ndata++] = (float)border;
				}
			}
			for (int j = 1; j < gydim - 1; j++) {
				int i = 0;
				int kgrid = i * gydim + j;
				if (grid[kgrid] == clipvalue) {
					sdata[ndata++] = (float)(sxmin + dx * i - bdata_origin_x);
					sdata[ndata++] = (float)(symin + dy * j - bdata_origin_y);
					sdata[ndata++] = (float)border;
				}
				i = gxdim - 1;
				kgrid = i * gydim + j;
				if (grid[kgrid] == clipvalue) {
					sdata[ndata++] = (float)(sxmin + dx * i - bdata_origin_x);
					sdata[ndata++] = (float)(symin + dy * j - bdata_origin_y);
					sdata[ndata++] = (float)border;
				}
			}
		}
		ndata = ndata / 3;

		/* do the interpolation */
		if (verbose > 0)
			fprintf(outfp, "\nDoing spline interpolation with %u data points...\n", ndata);
		cay = (float)tension;
		xmin = (float)(sxmin - 0.5 * dx - bdata_origin_x);
		ymin = (float)(symin - 0.5 * dy - bdata_origin_y);
		ddx = (float)dx;
		ddy = (float)dy;
		if (clipmode == MBMOSAIC_INTERP_ALL)
			clip = std::max(gxdim, gydim);
		mb_zgrid2(sgrid, &gxdim, &gydim, &xmin, &ymin, &ddx, &ddy, sdata, &ndata,
                      static_cast<float *>(work1), static_cast<int *>(work2),
                      static_cast<bool *>(work3), &cay, &clip);

		if (clipmode == MBMOSAIC_INTERP_GAP)
			fprintf(outfp, "Applying spline interpolation to fill gaps of %d cells or less...\n", clip);
		else if (clipmode == MBMOSAIC_INTERP_NEAR)
			fprintf(outfp, "Applying spline interpolation to fill %d cells from data...\n", clip);
		else if (clipmode == MBMOSAIC_INTERP_ALL)
			fprintf(outfp, "Applying spline interpolation to fill all undefined cells in the grid...\n");

		/* translate the interpolation into the grid array
		    filling only data gaps */
		zflag = 5.0e34;
		if (clipmode == MBMOSAIC_INTERP_GAP) {
			for (int i = 0; i < gxdim; i++)
				for (int j = 0; j < gydim; j++) {
					const int kgrid = i * gydim + j;
#ifdef USESURFACE
					const int kint = i + (gydim - j - 1) * gxdim;
#else
					const int kint = i + j * gxdim;
#endif
					num[kgrid] = false;
					if (grid[kgrid] >= clipvalue && sgrid[kint] < zflag) {
						/* initialize direction mask of search */
						for (int ii = 0; ii < 9; ii++)
							dmask[ii] = false;

						/* loop over rings around point, starting close */
						for (int ir = 0; ir <= clip && num[kgrid] == false; ir++) {
							/* set bounds of search */
							int i1 = std::max(0, i - ir);
							int i2 = std::min(gxdim - 1, i + ir);
							int j1 = std::max(0, j - ir);
							int j2 = std::min(gydim - 1, j + ir);

							int jj = j1;
							for (int ii = i1; ii <= i2 && num[kgrid] == false; ii++) {
								if (grid[ii * gydim + jj] < clipvalue) {
									r = sqrt((double)((ii - i) * (ii - i) + (jj - j) * (jj - j)));
									const int iii = rint((ii - i) / r) + 1;
									const int jjj = rint((jj - j) / r) + 1;
									k_mode = iii * 3 + jjj;
									dmask[k_mode] = true;
									if ((dmask[0] && dmask[8]) || (dmask[3] && dmask[5]) || (dmask[6] && dmask[2]) ||
									    (dmask[1] && dmask[7]))
										num[kgrid] = true;
								}
							}

							jj = j2;
							for (int ii = i1; ii <= i2 && num[kgrid] == false; ii++) {
								if (grid[ii * gydim + jj] < clipvalue) {
									r = sqrt((double)((ii - i) * (ii - i) + (jj - j) * (jj - j)));
									const int iii = rint((ii - i) / r) + 1;
									const int jjj = rint((jj - j) / r) + 1;
									k_mode = iii * 3 + jjj;
									dmask[k_mode] = true;
									if ((dmask[0] && dmask[8]) || (dmask[3] && dmask[5]) || (dmask[6] && dmask[2]) ||
									    (dmask[1] && dmask[7]))
										num[kgrid] = true;
								}
							}

							int ii = i1;
							for (jj = j1; jj <= j2 && num[kgrid] == false; jj++) {
								if (grid[ii * gydim + jj] < clipvalue) {
									r = sqrt((double)((ii - i) * (ii - i) + (jj - j) * (jj - j)));
									const int iii = rint((ii - i) / r) + 1;
									const int jjj = rint((jj - j) / r) + 1;
									k_mode = iii * 3 + jjj;
									dmask[k_mode] = true;
									if ((dmask[0] && dmask[8]) || (dmask[3] && dmask[5]) || (dmask[6] && dmask[2]) ||
									    (dmask[1] && dmask[7]))
										num[kgrid] = true;
								}
							}

							ii = i2;
							for (jj = j1; jj <= j2 && num[kgrid] == false; jj++) {
								if (grid[ii * gydim + jj] < clipvalue) {
									r = sqrt((double)((ii - i) * (ii - i) + (jj - j) * (jj - j)));
									const int iii = rint((ii - i) / r) + 1;
									const int jjj = rint((jj - j) / r) + 1;
									k_mode = iii * 3 + jjj;
									dmask[k_mode] = true;
									if ((dmask[0] && dmask[8]) || (dmask[3] && dmask[5]) || (dmask[6] && dmask[2]) ||
									    (dmask[1] && dmask[7]))
										num[kgrid] = true;
								}
							}
						}
					}
				}
			for (int i = 0; i < gxdim; i++)
				for (int j = 0; j < gydim; j++) {
					const int kgrid = i * gydim + j;
#ifdef USESURFACE
					const int kint = i + (gydim - j - 1) * gxdim;
#else
					const int kint = i + j * gxdim;
#endif
					if (num[kgrid] == true) {
						grid[kgrid] = sgrid[kint];
						nbinspline++;
					}
				}
		}

		/* translate the interpolation into the grid array
		    filling by proximity */
		else if (clipmode == MBMOSAIC_INTERP_NEAR) {
			for (int i = 0; i < gxdim; i++)
				for (int j = 0; j < gydim; j++) {
					const int kgrid = i * gydim + j;
#ifdef USESURFACE
					const int kint = i + (gydim - j - 1) * gxdim;
#else
					const int kint = i + j * gxdim;
#endif

					num[kgrid] = false;
					if (grid[kgrid] >= clipvalue && sgrid[kint] < zflag) {
						/* loop over rings around point, starting close */
						for (int ir = 0; ir <= clip && num[kgrid] == false; ir++) {
							/* set bounds of search */
							int i1 = std::max(0, i - ir);
							int i2 = std::min(gxdim - 1, i + ir);
							int j1 = std::max(0, j - ir);
							int j2 = std::min(gydim - 1, j + ir);

							int jj = j1;
							for (int ii = i1; ii <= i2 && num[kgrid] == false; ii++) {
								if (grid[ii * gydim + jj] < clipvalue) {
									num[kgrid] = true;
								}
							}

							jj = j2;
							for (int ii = i1; ii <= i2 && num[kgrid] == false; ii++) {
								if (grid[ii * gydim + jj] < clipvalue) {
									num[kgrid] = true;
								}
							}

							int ii = i1;
							for (jj = j1; jj <= j2 && num[kgrid] == false; jj++) {
								if (grid[ii * gydim + jj] < clipvalue) {
									num[kgrid] = true;
								}
							}

							ii = i2;
							for (jj = j1; jj <= j2 && num[kgrid] == false; jj++) {
								if (grid[ii * gydim + jj] < clipvalue) {
									num[kgrid] = true;
								}
							}
						}
					}
				}
			for (int i = 0; i < gxdim; i++)
				for (int j = 0; j < gydim; j++) {
					const int kgrid = i * gydim + j;
#ifdef USESURFACE
					const int kint = i + (gydim - j - 1) * gxdim;
#else
					const int kint = i + j * gxdim;
#endif
					if (num[kgrid] == true) {
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
					const int kgrid = i * gydim + j;
#ifdef USESURFACE
					const int kint = i + (gydim - j - 1) * gxdim;
#else
					const int kint = i + j * gxdim;
#endif
					if (grid[kgrid] >= clipvalue && sgrid[kint] < zflag) {
						grid[kgrid] = sgrid[kint];
						nbinspline++;
					}
				}
		}

		/* deallocate the interpolation arrays */
		for (int i = 0; i < gxdim; i++)
			for (int j = 0; j < gydim; j++) {
				const int kgrid = i * gydim + j;
				const int kint = i + j * gxdim;
				if (num[kgrid] == true) {
					grid[kgrid] = sgrid[kint];
					nbinspline++;
				}
			}
		mb_freed(verbose, __FILE__, __LINE__, (void **)&sdata, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&sgrid, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&work1, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&work2, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&work3, &error);
	}

	/* get min max of data */
	zclip = clipvalue;
	zmin = zclip;
	zmax = zclip;
	for (int i = 0; i < gxdim; i++)
		for (int j = 0; j < gydim; j++) {
			const int kgrid = i * gydim + j;

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
			const int kgrid = i * gydim + j;

			if (cnt[kgrid] > nmax)
				nmax = cnt[kgrid];
		}

	/* get min max of standard deviation */
	smin = 0.0;
	smax = 0.0;
	for (int i = 0; i < gxdim; i++)
		for (int j = 0; j < gydim; j++) {
			const int kgrid = i * gydim + j;

			if (smin == 0.0 && cnt[kgrid] > 1)
				smin = sigma[kgrid];
			if (smax == 0.0 && cnt[kgrid] > 1)
				smax = sigma[kgrid];
			if (sigma[kgrid] < smin && cnt[kgrid] > 1)
				smin = sigma[kgrid];
			if (sigma[kgrid] > smax && cnt[kgrid] > 1)
				smax = sigma[kgrid];
		}
	nbinzero = gxdim * gydim - nbinset - nbinspline;
	fprintf(outfp, "\nTotal number of bins:            %d\n", gxdim * gydim);
	fprintf(outfp, "Bins set using data:             %d\n", nbinset);
	fprintf(outfp, "Bins set using interpolation:    %d\n", nbinspline);
	fprintf(outfp, "Bins not set:                    %d\n", nbinzero);
	fprintf(outfp, "Maximum number of data in a bin: %d\n", nmax);
	fprintf(outfp, "Minimum value: %10.2f   Maximum value: %10.2f\n", zmin, zmax);
	fprintf(outfp, "Minimum sigma: %10.5f   Maximum sigma: %10.5f\n", smin, smax);

	/* set plot label strings */
	if (use_projection) {
		snprintf(xlabel, sizeof(xlabel), "Easting (%s)", units);
		snprintf(ylabel, sizeof(ylabel), "Northing (%s)", units);
	}
	else {
		strcpy(xlabel, "Longitude");
		strcpy(ylabel, "Latitude");
	}
	if (datatype == MBMOSAIC_DATA_AMPLITUDE) {
		strcpy(zlabel, "Amplitude");
		strcpy(nlabel, "Number of Amplitude Data Points");
		strcpy(sdlabel, "Amplitude Standard Deviation (m)");
		strcpy(title, "Amplitude Grid");
	}
	else if (datatype == MBMOSAIC_DATA_SIDESCAN) {
		strcpy(zlabel, "Sidescan");
		strcpy(nlabel, "Number of Sidescan Data Points");
		strcpy(sdlabel, "Sidescan Standard Deviation (m)");
		strcpy(title, "Sidescan Grid");
	}
	else if (datatype == MBMOSAIC_DATA_FLAT_GRAZING) {
		strcpy(zlabel, "Degrees");
		strcpy(nlabel, "Number of Bottom Data Points");
		strcpy(sdlabel, "Grazing angle Standard Deviation (m)");
		strcpy(title, "Flat bottom grazing angle Grid");
	}
	else if (datatype == MBMOSAIC_DATA_GRAZING) {
		strcpy(zlabel, "Degrees");
		strcpy(nlabel, "Number of Bottom Data Points");
		strcpy(sdlabel, "Grazing angle Standard Deviation (m)");
		strcpy(title, "Grazing Angle Grid");
	}
	else if (datatype == MBMOSAIC_DATA_SLOPE) {
		strcpy(zlabel, "Degrees");
		strcpy(nlabel, "Number of Slope Data Points");
		strcpy(sdlabel, "Slope Standard Deviation (m)");
		strcpy(title, "Slope Grid");
	}

	/* write first output file */
	if (verbose > 0)
		fprintf(outfp, "\nOutputting results...\n");
	for (int i = 0; i < xdim; i++)
		for (int j = 0; j < ydim; j++) {
			const int kgrid = (i + offx) * gydim + (j + offy);
			const int kout = i * ydim + j;
			output[kout] = (float)grid[kgrid];
			if (gridkind != MBMOSAIC_ASCII && gridkind != MBMOSAIC_ARCASCII && grid[kgrid] == clipvalue) {
				output[kout] = outclipvalue;
			}
		}
	if (gridkind == MBMOSAIC_ASCII) {
		strcpy(ofile, fileroot);
		strcat(ofile, ".asc");
		status = write_ascii(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, &error);
	}
	else if (gridkind == MBMOSAIC_ARCASCII) {
		strcpy(ofile, fileroot);
		strcat(ofile, ".asc");
		status =
		    write_arcascii(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, clipvalue, &error);
	}
	else if (gridkind == MBMOSAIC_OLDGRD) {
		strcpy(ofile, fileroot);
		strcat(ofile, ".grd1");
		status = write_oldgrd(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, &error);
	}
	else if (gridkind == MBMOSAIC_CDFGRD) {
		strcpy(ofile, fileroot);
		strcat(ofile, ".grd");
		status = mb_write_gmt_grd(verbose, ofile, output, outclipvalue, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], zmin,
		                          zmax, dx, dy, xlabel, ylabel, zlabel, title, projection_id, argc, argv, &error);
	}
	else if (gridkind == MBMOSAIC_GMTGRD) {
		snprintf(ofile, sizeof(ofile), "%s.grd%s", fileroot, gridkindstring);
		status = mb_write_gmt_grd(verbose, ofile, output, outclipvalue, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], zmin,
		                          zmax, dx, dy, xlabel, ylabel, zlabel, title, projection_id, argc, argv, &error);
	}
	if (status != MB_SUCCESS) {
		char *message = nullptr;
		mb_error(verbose, error, &message);
		fprintf(stderr, "\nError writing output file: %s\n%s\n", ofile, message);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		mb_memory_clear(verbose, &error);
		exit(error);
	}

	/* write second output file */
	if (more) {
		for (int i = 0; i < xdim; i++)
			for (int j = 0; j < ydim; j++) {
				const int kgrid = (i + offx) * gydim + (j + offy);
				const int kout = i * ydim + j;
				output[kout] = (float)cnt[kgrid];
				if (output[kout] < 0.0)
					output[kout] = 0.0;
				if (gridkind != MBMOSAIC_ASCII && gridkind != MBMOSAIC_ARCASCII && cnt[kgrid] <= 0)
					output[kout] = outclipvalue;
			}
		if (gridkind == MBMOSAIC_ASCII) {
			strcpy(ofile, fileroot);
			strcat(ofile, "_num.asc");
			status = write_ascii(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, &error);
		}
		else if (gridkind == MBMOSAIC_ARCASCII) {
			strcpy(ofile, fileroot);
			strcat(ofile, ".asc");
			status =
			    write_arcascii(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, clipvalue, &error);
		}
		else if (gridkind == MBMOSAIC_OLDGRD) {
			strcpy(ofile, fileroot);
			strcat(ofile, "_num.grd1");
			status = write_oldgrd(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, &error);
		}
		else if (gridkind == MBMOSAIC_CDFGRD) {
			strcpy(ofile, fileroot);
			strcat(ofile, "_num.grd");
			status = mb_write_gmt_grd(verbose, ofile, output, outclipvalue, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], zmin,
			                          zmax, dx, dy, xlabel, ylabel, zlabel, title, projection_id, argc, argv, &error);
		}
		else if (gridkind == MBMOSAIC_GMTGRD) {
			snprintf(ofile, sizeof(ofile), "%s_num.grd%s", fileroot, gridkindstring);
			status = mb_write_gmt_grd(verbose, ofile, output, outclipvalue, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], zmin,
			                          zmax, dx, dy, xlabel, ylabel, zlabel, title, projection_id, argc, argv, &error);
		}
		if (status != MB_SUCCESS) {
			char *message = nullptr;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nError writing output file: %s\n%s\n", ofile, message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
		}

		/* write third output file */
		for (int i = 0; i < xdim; i++)
			for (int j = 0; j < ydim; j++) {
				const int kgrid = (i + offx) * gydim + (j + offy);
				const int kout = i * ydim + j;
				output[kout] = (float)sigma[kgrid];
				if (output[kout] < 0.0)
					output[kout] = 0.0;
				if (gridkind != MBMOSAIC_ASCII && gridkind != MBMOSAIC_ARCASCII && cnt[kgrid] <= 0)
					output[kout] = outclipvalue;
			}
		if (gridkind == MBMOSAIC_ASCII) {
			strcpy(ofile, fileroot);
			strcat(ofile, "_sd.asc");
			status = write_ascii(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, &error);
		}
		else if (gridkind == MBMOSAIC_ARCASCII) {
			strcpy(ofile, fileroot);
			strcat(ofile, ".asc");
			status =
			    write_arcascii(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, clipvalue, &error);
		}
		else if (gridkind == MBMOSAIC_OLDGRD) {
			strcpy(ofile, fileroot);
			strcat(ofile, "_sd.grd1");
			status = write_oldgrd(verbose, ofile, output, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], dx, dy, &error);
		}
		else if (gridkind == MBMOSAIC_CDFGRD) {
			strcpy(ofile, fileroot);
			strcat(ofile, "_sd.grd");
			status = mb_write_gmt_grd(verbose, ofile, output, outclipvalue, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], zmin,
			                          zmax, dx, dy, xlabel, ylabel, zlabel, title, projection_id, argc, argv, &error);
		}
		else if (gridkind == MBMOSAIC_GMTGRD) {
			snprintf(ofile, sizeof(ofile), "%s_sd.grd%s", fileroot, gridkindstring);
			status = mb_write_gmt_grd(verbose, ofile, output, outclipvalue, xdim, ydim, gbnd[0], gbnd[1], gbnd[2], gbnd[3], zmin,
			                          zmax, dx, dy, xlabel, ylabel, zlabel, title, projection_id, argc, argv, &error);
		}
		if (status != MB_SUCCESS) {
			char *message = nullptr;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nError writing output file: %s\n%s\n", ofile, message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
		}
	}

	mb_freed(verbose, __FILE__, __LINE__, (void **)&grid, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&norm, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&maxpriority, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&cnt, &error);
	if (clip != 0)
		mb_freed(verbose, __FILE__, __LINE__, (void **)&num, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&sigma, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&output, &error);
	if (priority_source == MBMOSAIC_PRIORITYTABLE_FILE && n_priority_angle > 0) {
		mb_freed(verbose, __FILE__, __LINE__, (void **)&priority_angle_angle, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&priority_angle_priority, &error);
	}

	if (use_projection) {
		/* proj_status = */ mb_proj_free(verbose, &(pjptr), &error);
	}

	/* run mbm_grdplot */
	if (gridkind == MBMOSAIC_GMTGRD) {
		/* execute mbm_grdplot */
		strcpy(ofile, fileroot);
		strcat(ofile, ".grd");
		if (datatype == MBMOSAIC_DATA_AMPLITUDE) {
			snprintf(plot_cmd, sizeof(plot_cmd), "mbm_grdplot -I%s%s -G1 -W1/4 -S -D -V -L\"File %s - %s:%s\"", ofile, gridkindstring, ofile, title,
			        zlabel);
		}
		else {
			snprintf(plot_cmd, sizeof(plot_cmd), "mbm_grdplot -I%s%s -G1 -W1/4 -S -D -V -L\"File %s - %s:%s\"", ofile, gridkindstring, ofile, title,
			        zlabel);
		}
		if (verbose) {
			fprintf(stderr, "\nexecuting mbm_grdplot...\n%s\n", plot_cmd);
		}
		plot_status = system(plot_cmd);
		if (plot_status == -1) {
			fprintf(stderr, "\nError executing mbm_grdplot on output file %s\n", ofile);
		}
	}
	if (more && gridkind == MBMOSAIC_GMTGRD) {
		/* execute mbm_grdplot */
		strcpy(ofile, fileroot);
		strcat(ofile, "_num.grd");
		snprintf(plot_cmd, sizeof(plot_cmd), "mbm_grdplot -I%s%s -G1 -W1/2 -V -L\"File %s - %s:%s\"", ofile, gridkindstring, ofile, title, nlabel);
		if (verbose) {
			fprintf(stderr, "\nexecuting mbm_grdplot...\n%s\n", plot_cmd);
		}
		plot_status = system(plot_cmd);
		if (plot_status == -1) {
			fprintf(stderr, "\nError executing mbm_grdplot on output file grd_%s\n", fileroot);
		}

		/* execute mbm_grdplot */
		strcpy(ofile, fileroot);
		strcat(ofile, "_sd.grd");
		snprintf(plot_cmd, sizeof(plot_cmd), "mbm_grdplot -I%s%s -G1 -W1/2 -V -L\"File %s - %s:%s\"", ofile, gridkindstring, ofile, title, sdlabel);
		if (verbose) {
			fprintf(stderr, "\nexecuting mbm_grdplot...\n%s\n", plot_cmd);
		}
		plot_status = system(plot_cmd);
		if (plot_status == -1) {
			fprintf(stderr, "\nError executing mbm_grdplot on output file grd_%s\n", fileroot);
		}
	}

	if (verbose > 0)
		fprintf(outfp, "\nDone.\n\n");

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose, &error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s> completed\n", program_name);
		fprintf(stderr, "dbg2  Ending status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	exit(error);
}
/*--------------------------------------------------------------------*/
