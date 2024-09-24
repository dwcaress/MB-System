/*--------------------------------------------------------------------
 *    The MB-system:	mbmapscale.c	6/5/2008
 *
 *    Copyright (c) 2008-2024 by
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
 * mbmapscale converts time values between epoch seconds (seconds since
 * 1970/01/01 00:00:00.000000) and calendar time (e.g. 2008/006/05/17/24/32/0).
 * The input time is set using the command line arguments -Mtime_d for
 * epoch seconds and -Tyear/month/day/hour/minute/second/microsecond for
 * calendar time. The output time (in the form not specified as input) is
 * written to stdout.
 *
 * Author:	D. W. Caress
 * Date:	June 5, 2008
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_status.h"

typedef enum {
    MBMAPSCALE_MODE_WGS72 = 0,
    MBMAPSCALE_MODE_ALVINXY  = 1
} time_mode_t;

constexpr char program_name[] = "mbmapscale";
constexpr char help_message[] =
    "mbmapscale outputs the scaling between geographic coordinates (longitude and latitude)"
    "and local meters east and north at a user defined latitude. The map scale is\n"
    "written to stdout in the form of meters per degree longitude and latitude.";
constexpr char usage_message[] =
    "mbmapscale [-Llatitude -A -V -H]";

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	int verbose = 0;
	int mode = MBMAPSCALE_MODE_WGS72;
	double latitude;
	int status = MB_SUCCESS;

	/* process argument list */
	{
		bool errflg = false;
		int c;
		bool help = false;
		while ((c = getopt(argc, argv, "VvHhAaL:l:")) != -1)
			switch (c) {
			case 'H':
			case 'h':
				help = true;
				break;
			case 'A':
			case 'a':
				mode = MBMAPSCALE_MODE_ALVINXY;
				break;
			case 'L':
			case 'l':
				sscanf(optarg, "%lf", &latitude);
				break;
			case 'V':
			case 'v':
				verbose++;
				break;
			case '?':
				errflg = true;
			}

		if (errflg) {
			fprintf(stderr, "usage: %s\n", usage_message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_BAD_USAGE);
		}

		if (verbose == 1 || help) {
			fprintf(stdout, "\nProgram %s\n", program_name);
			fprintf(stdout, "MB-system Version %s\n", MB_VERSION);
		}

		if (verbose >= 2) {
			fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
			fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
			fprintf(stderr, "dbg2  Control Parameters:\n");
			fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
			fprintf(stderr, "dbg2       help:       %d\n", help);
			fprintf(stderr, "dbg2       latitude:   %f\n", latitude);
			fprintf(stderr, "dbg2       mode:       %d\n", mode);
		}

		if (help) {
			fprintf(stderr, "\n%s\n", help_message);
			fprintf(stderr, "\nusage: %s\n", usage_message);
			exit(MB_ERROR_NO_ERROR);
		}

	}

	/* calculate mtodeglon and mtodeglat */
	double mtodeglon, mtodeglat;
	if (mode == MBMAPSCALE_MODE_WGS72) {
		status = mb_coor_scale(verbose, latitude, &mtodeglon, &mtodeglat);
	}
	else {
		status = mb_alvinxy_scale(verbose, latitude, &mtodeglon, &mtodeglat);
	}
	fprintf(stdout, "\nLocal scaling between degrees longitude and latitude and meters east and north:\n");
	if (mode == MBMAPSCALE_MODE_WGS72) {
		fprintf(stdout, "\tUsing WGS72 ellipsoid\n");
	}
	else {
		fprintf(stdout, "\tUsing 1866 Clark Spheroid as per AlvinXY coordinates\n");
	}
	fprintf(stdout, "\tMeters per degree longitude: %.3f\n", 1.0/mtodeglon);
	fprintf(stdout, "\tMeters per degree latitude:  %.3f\n", 1.0/mtodeglat);
	fprintf(stdout, "\tMeters to degree longitude:  %.9f\n", mtodeglon);
	fprintf(stdout, "\tMeters to degree latitude:   %.9f\n", mtodeglat);
	
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s> completed\n", program_name);
		fprintf(stderr, "dbg2  Ending status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", MB_SUCCESS);
	}

	exit(MB_ERROR_NO_ERROR);
}
/*--------------------------------------------------------------------*/
