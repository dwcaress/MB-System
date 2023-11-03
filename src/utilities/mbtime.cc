/*--------------------------------------------------------------------
 *    The MB-system:	mbtime.c	6/5/2008
 *
 *    Copyright (c) 2008-2023 by
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
 * MBTIME converts time values between epoch seconds (seconds since
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
    MBTIME_INPUT_EPOCH = 0,
    MBTIME_INPUT_CALENDAR  = 1
} time_mode_t;

constexpr char program_name[] = "MBTIME";
constexpr char help_message[] =
    "MBTIME converts time values between epoch seconds (seconds since\n"
    "1970/01/01 00:00:00.000000) and calendar time (e.g. 2008/006/05/17/24/32/0).\n"
    "The input time is set using the command line arguments -Mtime_d for\n"
    "epoch seconds and -Tyear/month/day/hour/minute/second/microsecond for\n"
    "calendar time. The output time (in the form not specified as input) is\n"
    "written to stdout.";
constexpr char usage_message[] =
    "mbtime [-Mtime_d -Tyear/month/day/hour/minute/second -V -H]";

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	int verbose = 0;
	time_mode_t mode = MBTIME_INPUT_EPOCH;
	int time_i[7];
	double time_d = 0.0;
	double seconds;

	/* process argument list */
	{
		bool errflg = false;
		int c;
		bool help = false;
		while ((c = getopt(argc, argv, "VvHhM:m:T:t:")) != -1)
			switch (c) {
			case 'H':
			case 'h':
				help = true;
				break;
			case 'M':
			case 'm':
				sscanf(optarg, "%lf", &time_d);
				mode = MBTIME_INPUT_EPOCH;
				break;
			case 'T':
			case 't':
				sscanf(optarg, "%d/%d/%d/%d/%d/%lf", &time_i[0], &time_i[1], &time_i[2], &time_i[3], &time_i[4], &seconds);
				time_i[5] = (int)seconds;
				time_i[6] = (int)(1000000 * (seconds - time_i[5]));
				mode = MBTIME_INPUT_CALENDAR;
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
			fprintf(stderr, "dbg2       mode:       %d\n", mode);
			fprintf(stderr, "dbg2       time_i[0]:  %d\n", time_i[0]);
			fprintf(stderr, "dbg2       time_i[1]:  %d\n", time_i[1]);
			fprintf(stderr, "dbg2       time_i[2]:  %d\n", time_i[2]);
			fprintf(stderr, "dbg2       time_i[3]:  %d\n", time_i[3]);
			fprintf(stderr, "dbg2       time_i[4]:  %d\n", time_i[4]);
			fprintf(stderr, "dbg2       time_i[5]:  %d\n", time_i[5]);
			fprintf(stderr, "dbg2       time_i[6]:  %d\n", time_i[6]);
			fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
		}

		if (help) {
			fprintf(stderr, "\n%s\n", help_message);
			fprintf(stderr, "\nusage: %s\n", usage_message);
			exit(MB_ERROR_NO_ERROR);
		}

	}

	/* convert to calendar time and output */
	if (mode == MBTIME_INPUT_EPOCH) {
		mb_get_date(verbose, time_d, time_i);
		fprintf(stdout, "%4.4d/%2.2d/%2.2d/%2.2d/%2.2d/%2.2d.%6.6d\n",
			time_i[0], time_i[1], time_i[2], time_i[3], time_i[4],
		        time_i[5], time_i[6]);
	} else {
		/* convert to epoch time and output */
		mb_get_time(verbose, time_i, &time_d);
		fprintf(stdout, "%f\n", time_d);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s> completed\n", program_name);
		fprintf(stderr, "dbg2  Ending status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", MB_SUCCESS);
	}

	exit(MB_ERROR_NO_ERROR);
}
/*--------------------------------------------------------------------*/
