/*--------------------------------------------------------------------
 *    The MB-system:	mbpreprocess.c	1/8/2014
 *
 *    Copyright (c) 2014-2019 by
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
 * mbconfig provides command line access to the MB-System version and to the
 * locations of the levitus database and the OTPS tidal correction software.
 *
 * Author:	D. W. Caress
 * Date:	May 5, 2017
 */

#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "levitus.h"
#include "mb_define.h"
#include "mb_format.h"
#include "mb_status.h"

static const char program_name[] = "mbconfig";
static const char help_message[] =
    "mbconfig provides command line access to the MB-System installation location, "
    "the compile and libs flags needed to compile and link programs using MB-System "
    "libraries, and the locations of the levitus database and the OTPS tidal "
    "correction software.\n";
static const char usage_message[] =
    "mbconfig --verbose --help --prefix --cflags --libs "
    "--version --version-id --version-major --version-minor --version-archive";

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	int option_index;
	int errflg = 0;
	int c;
	int mode_set = MB_NO;
	int mode_help = MB_NO;
	int mode_prefix = MB_NO;
	int mode_cflags = MB_NO;
	int mode_libs = MB_NO;
	int mode_version = MB_NO;
	int mode_version_id = MB_NO;
	int mode_version_major = MB_NO;
	int mode_version_minor = MB_NO;
	int mode_version_archive = MB_NO;
	int mode_levitus = MB_NO;
	int mode_otps = MB_NO;

	/* MBIO status variables */
	int status = MB_SUCCESS;
	int verbose = 0;
	int error = MB_ERROR_NO_ERROR;
	mb_path version_string;
	int version_id;
	int version_major;
	int version_minor;
	int version_archive;

	/* command line option definitions */
	/* mbpreprocess
	 * 		--verbose
	 * 		--help
	 *
	 * 		--prefix
	 * 		--cflags
	 * 		--libs
	 *
	 * 		--version
	 * 		--version-id
	 * 		--version-major
	 * 		--version-minor
	 * 		--version-archive
	 *
	 * 		--levitus
	 * 		--otps
	 */
	static struct option options[] = {{"verbose", no_argument, NULL, 0},
	                                  {"help", no_argument, NULL, 0},
	                                  {"prefix", no_argument, NULL, 0},
	                                  {"cflags", no_argument, NULL, 0},
	                                  {"libs", no_argument, NULL, 0},
	                                  {"version", no_argument, NULL, 0},
	                                  {"version-id", no_argument, NULL, 0},
	                                  {"version-major", no_argument, NULL, 0},
	                                  {"version-minor", no_argument, NULL, 0},
	                                  {"version-archive", no_argument, NULL, 0},
	                                  {"levitus", no_argument, NULL, 0},
	                                  {"otps", no_argument, NULL, 0},
	                                  {NULL, 0, NULL, 0}};

	status = mb_version(verbose, version_string, &version_id, &version_major, &version_minor, &version_archive, &error);

	/* process argument list */
	while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1)
		switch (c) {
		/* long options all return c=0 */
		case 0:
			/* verbose */
			if (strcmp("verbose", options[option_index].name) == 0) {
				verbose++;
			}

			/* help */
			else if (strcmp("help", options[option_index].name) == 0) {
				mode_help = MB_YES;
				mode_set = MB_YES;
			}

			/*-------------------------------------------------------*/

			/* prefix */
			else if (strcmp("prefix", options[option_index].name) == 0) {
				mode_prefix = MB_YES;
				mode_set = MB_YES;
			}

			/* cflags */
			else if (strcmp("cflags", options[option_index].name) == 0) {
				mode_cflags = MB_YES;
				mode_set = MB_YES;
			}

			/* libs */
			else if (strcmp("libs", options[option_index].name) == 0) {
				mode_libs = MB_YES;
				mode_set = MB_YES;
			}

			/*-------------------------------------------------------
			 * Various sorts of version statement */

			/* version */
			else if (strcmp("version", options[option_index].name) == 0) {
				mode_version = MB_YES;
				mode_set = MB_YES;
			}

			/* version-id */
			else if (strcmp("version-id", options[option_index].name) == 0) {
				mode_version_id = MB_YES;
				mode_set = MB_YES;
			}

			/* version-major */
			else if (strcmp("version-major", options[option_index].name) == 0) {
				mode_version_major = MB_YES;
				mode_set = MB_YES;
			}

			/* version-minor */
			else if (strcmp("version-minor", options[option_index].name) == 0) {
				mode_version_minor = MB_YES;
				mode_set = MB_YES;
			}

			/* version-archive */
			else if (strcmp("version-archive", options[option_index].name) == 0) {
				mode_version_archive = MB_YES;
				mode_set = MB_YES;
			}

			/*-------------------------------------------------------*/

			/* levitus */
			else if (strcmp("levitus", options[option_index].name) == 0) {
				mode_levitus = MB_YES;
				mode_set = MB_YES;
			}

			/* otps */
			else if (strcmp("otps", options[option_index].name) == 0) {
				mode_otps = MB_YES;
				mode_set = MB_YES;
			}

			/*-------------------------------------------------------*/

			break;
		case '?':
			errflg++;
		}

	/* if error flagged then print it and exit */
	if (errflg) {
		fprintf(stderr, "usage: %s\n", usage_message);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
	}

	/* if no mode specified then just do version */
	if (mode_set == MB_NO)
		mode_version = MB_YES;

	if (verbose == 1 || mode_help == MB_YES) {
		fprintf(stderr, "\n# Program %s\n", program_name);
		fprintf(stderr, "# MB-system Version %s\n", version_string);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
		fprintf(stderr, "dbg2  MB-system Version %s\n", version_string);
		fprintf(stderr, "dbg2  Default MB-System Parameters:\n");
		fprintf(stderr, "dbg2       verbose:                    %d\n", verbose);
		fprintf(stderr, "dbg2       mode_set:                   %d\n", mode_set);
		fprintf(stderr, "dbg2       mode_help:                  %d\n", mode_help);
		fprintf(stderr, "dbg2       mode_prefix:                %d\n", mode_prefix);
		fprintf(stderr, "dbg2       mode_cflags:                %d\n", mode_cflags);
		fprintf(stderr, "dbg2       mode_libs:                  %d\n", mode_libs);
		fprintf(stderr, "dbg2       mode_version:               %d\n", mode_version);
		fprintf(stderr, "dbg2       mode_version_id:            %d\n", mode_version_id);
		fprintf(stderr, "dbg2       mode_version_major:         %d\n", mode_version_major);
		fprintf(stderr, "dbg2       mode_version_minor:         %d\n", mode_version_minor);
		fprintf(stderr, "dbg2       mode_version_archive:       %d\n", mode_version_archive);
		fprintf(stderr, "dbg2       mode_levitus:               %d\n", mode_levitus);
		fprintf(stderr, "dbg2       mode_otps:                  %d\n", mode_otps);
	}

	/* help */
	if (mode_help == MB_YES) {
		fprintf(stderr, "\n%s\n", help_message);
		fprintf(stderr, "\nusage: %s\n", usage_message);
	}

	/* prefix */
	if (mode_prefix == MB_YES) {
		if (verbose > 0)
			fprintf(stdout, "\n# MB-System install prefix:\n");
		fprintf(stdout, "%s\n", MBSYSTEM_INSTALL_PREFIX);
	}

	/* cflags */
	if (mode_cflags == MB_YES) {
		if (verbose > 0)
			fprintf(stdout, "\n# MB-System compile flags:\n");
		fprintf(stdout, "-I%s/include\n", MBSYSTEM_INSTALL_PREFIX);
	}

	/* libs */
	if (mode_libs == MB_YES) {
		if (verbose > 0)
			fprintf(stdout, "\n# MB-System link flags:\n");
#ifdef MBSYSTEM_BUNDLED_PROJ
		fprintf(stdout, "-L%s/libs -lmbaux.la -lmbsapi.la -lmbbsio.la -lmbview.la -lmbgsf.la -lmbxgr.la -lmbio.la -lproj\n",
		        MBSYSTEM_INSTALL_PREFIX);
#else
		fprintf(stdout, "-L%s/libs -lmbaux.la -lmbsapi.la -lmbbsio.la -lmbview.la -lmbgsf.la -lmbxgr.la -lmbio.la\n",
		        MBSYSTEM_INSTALL_PREFIX);
#endif
	}

	/* version */
	if (mode_version == MB_YES) {
		if (verbose > 0)
			fprintf(stdout, "\n# MB-System version:\n");
		fprintf(stdout, "%s\n", version_string);
	}

	/* version-id */
	if (mode_version_id == MB_YES) {
		if (verbose > 0)
			fprintf(stdout, "\n# MB-System version id:\n");
		fprintf(stdout, "%d\n", version_id);
	}

	/* version-major */
	if (mode_version_major == MB_YES) {
		if (verbose > 0)
			fprintf(stdout, "\n# MB-System major version:\n");
		fprintf(stdout, "%d\n", version_major);
	}

	/* version-minor */
	if (mode_version_minor == MB_YES) {
		if (verbose > 0)
			fprintf(stdout, "\n# MB-System minor version:\n");
		fprintf(stdout, "%d\n", version_minor);
	}

	/* version-archive */
	if (mode_version_archive == MB_YES) {
		if (verbose > 0)
			fprintf(stdout, "\n# MB-System archive version:\n");
		fprintf(stdout, "%d\n", version_archive);
	}

	/* version-archive */
	if (mode_levitus == MB_YES) {
		if (verbose > 0)
			fprintf(stdout, "# MB-System Levitus database location:\n");
		fprintf(stdout, "%s\n", levitusfile);
	}

	/* version-archive */
	if (mode_otps == MB_YES) {
		if (verbose > 0)
			fprintf(stdout, "\n# OTPS tide modeling package location:\n");
		fprintf(stdout, "%s\n", otps_location);
	}

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
