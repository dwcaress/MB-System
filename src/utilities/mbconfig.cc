/*--------------------------------------------------------------------
 *    The MB-system:	mbpreprocess.c	1/8/2014
 *
 *    Copyright (c) 2014-2023 by
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
 * mbconfig provides command line access to the MB-System version and to the
 * locations of the levitus database and the OTPS tidal correction software.
 *
 * Author:	D. W. Caress
 * Date:	May 5, 2017
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// CMake build system
#ifdef CMAKE_BUILD_SYSTEM
const char *levitusfile = "$(levitusDir)/LevitusAnnual82.dat";
const char *otps_location = "$(otpsDir)";

// Autotools build system
#else
#include "levitus.h"
#endif

#include "mb_define.h"
#include "mb_format.h"
#include "mb_status.h"

constexpr char program_name[] = "mbconfig";
constexpr char help_message[] =
    "mbconfig provides command line access to the MB-System installation location, "
    "the compile and libs flags needed to compile and link programs using MB-System "
    "libraries, and the locations of the levitus database and the OTPS tidal "
    "correction software.\n";
constexpr char usage_message[] =
    "mbconfig --verbose --help --prefix --cflags --libs "
    "--version --version-id --version-major --version-minor --version-archive";

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	bool mode_set = false;
	bool mode_help = false;
	bool mode_prefix = false;
	bool mode_cflags = false;
	bool mode_libs = false;
	bool mode_version = false;
	bool mode_version_id = false;
	bool mode_version_major = false;
	bool mode_version_minor = false;
	bool mode_version_archive = false;
	bool mode_levitus = false;
	bool mode_otps = false;

	int verbose = 0;

	/* process argument list */
	{
		const struct option options[] =
			{{"verbose", no_argument, nullptr, 0},
	                {"help", no_argument, nullptr, 0},
	                {"prefix", no_argument, nullptr, 0},
	                {"cflags", no_argument, nullptr, 0},
	                {"libs", no_argument, nullptr, 0},
	                {"version", no_argument, nullptr, 0},
	                {"version-id", no_argument, nullptr, 0},
	                {"version-major", no_argument, nullptr, 0},
	                {"version-minor", no_argument, nullptr, 0},
	                {"version-archive", no_argument, nullptr, 0},
	                {"levitus", no_argument, nullptr, 0},
	                {"otps", no_argument, nullptr, 0},
	                {nullptr, 0, nullptr, 0}};

		bool errflg = false;
		int c;
		int option_index;
		while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1)
			switch (c) {
			/* long options all return c=0 */
			case 0:
				if (strcmp("verbose", options[option_index].name) == 0) {
					verbose++;
				}
				else if (strcmp("help", options[option_index].name) == 0) {
					mode_help = true;
					mode_set = true;
				}
				else if (strcmp("prefix", options[option_index].name) == 0) {
					mode_prefix = true;
					mode_set = true;
				}
				else if (strcmp("cflags", options[option_index].name) == 0) {
					mode_cflags = true;
					mode_set = true;
				}
				else if (strcmp("libs", options[option_index].name) == 0) {
					mode_libs = true;
					mode_set = true;
				}
				else if (strcmp("version", options[option_index].name) == 0) {
					mode_version = true;
					mode_set = true;
				}
				else if (strcmp("version-id", options[option_index].name) == 0) {
					mode_version_id = true;
					mode_set = true;
				}
				else if (strcmp("version-major", options[option_index].name) == 0) {
					mode_version_major = true;
					mode_set = true;
				}
				else if (strcmp("version-minor", options[option_index].name) == 0) {
					mode_version_minor = true;
					mode_set = true;
				}
				else if (strcmp("version-archive", options[option_index].name) == 0) {
					mode_version_archive = true;
					mode_set = true;
				}
				else if (strcmp("levitus", options[option_index].name) == 0) {
					mode_levitus = true;
					mode_set = true;
				}
				else if (strcmp("otps", options[option_index].name) == 0) {
					mode_otps = true;
					mode_set = true;
				}
				break;
			case '?':
				errflg = true;
			}

		if (errflg) {
			fprintf(stderr, "usage: %s\n", usage_message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_BAD_USAGE);
		}
	}

	/* if no mode specified then just do version */
	if (!mode_set)
		mode_version = true;

	mb_path version_string;
	int version_id;
	int version_major;
	int version_minor;
	int version_archive;
	int error = MB_ERROR_NO_ERROR;

	int status = mb_version(verbose, version_string, &version_id, &version_major, &version_minor, &version_archive, &error);

	if (verbose == 1 || mode_help) {
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

	if (mode_help) {
		fprintf(stderr, "\n%s\n", help_message);
		fprintf(stderr, "\nusage: %s\n", usage_message);
	}

	if (mode_prefix) {
		if (verbose > 0)
			fprintf(stdout, "\n# MB-System install prefix:\n");
		fprintf(stdout, "%s\n", MBSYSTEM_INSTALL_PREFIX);
	}

	if (mode_cflags) {
		if (verbose > 0)
			fprintf(stdout, "\n# MB-System compile flags:\n");
		fprintf(stdout, "-I%s/include\n", MBSYSTEM_INSTALL_PREFIX);
	}

	if (mode_libs) {
		if (verbose > 0)
			fprintf(stdout, "\n# MB-System link flags:\n");
		fprintf(stdout, "-L%s/lib -lmbaux -lmbsapi -lmbbsio -lmbview -lmbgsf -lmbxgr -lmbio\n",
		        MBSYSTEM_INSTALL_PREFIX);
	}

	if (mode_version) {
		if (verbose > 0)
			fprintf(stdout, "\n# MB-System version:\n");
		fprintf(stdout, "%s\n", version_string);
	}

	if (mode_version_id) {
		if (verbose > 0)
			fprintf(stdout, "\n# MB-System version id:\n");
		fprintf(stdout, "%d\n", version_id);
	}

	if (mode_version_major) {
		if (verbose > 0)
			fprintf(stdout, "\n# MB-System major version:\n");
		fprintf(stdout, "%d\n", version_major);
	}

	if (mode_version_minor) {
		if (verbose > 0)
			fprintf(stdout, "\n# MB-System minor version:\n");
		fprintf(stdout, "%d\n", version_minor);
	}

	if (mode_version_archive) {
		if (verbose > 0)
			fprintf(stdout, "\n# MB-System archive version:\n");
		fprintf(stdout, "%d\n", version_archive);
	}

	if (mode_levitus) {
		if (verbose > 0)
			fprintf(stdout, "# MB-System Levitus database location:\n");
		fprintf(stdout, "%s\n", levitusfile);
	}

	if (mode_otps) {
		if (verbose > 0)
			fprintf(stdout, "\n# OTPS tide modeling package location:\n");
		fprintf(stdout, "%s\n", otps_location);
	}

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
