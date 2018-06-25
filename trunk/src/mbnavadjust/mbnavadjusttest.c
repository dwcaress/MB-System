/*--------------------------------------------------------------------
 *    The MB-system:	mbnavadjusttest.c	4/14/2014
 *    $Id$
 *
 *    Copyright (c) 2018-2018 by
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
 * Mbnavadjustmerge merges two existing mbnavadjust projects. The result
 * can be to add one project to another or to create a new, third project
 * combining the two source projects.
 *
 * Author:	D. W. Caress
 * Date:	June 18, 2018
 *
 *
 */

/* source file version string */
static char version_id[] = "$Id$";

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

/* MBIO include files */
#include "mb_status.h"
#include "mb_define.h"
#include "mb_process.h"
#include "mb_aux.h"
#include "mbnavadjust_io.h"

/* local defines */
#define MBNAVADJUSTTEST_ALGORITHM_CROSS_CORRELATION 0
#define MBNAVADJUSTTEST_ALGORITHM_ICP 1

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	char program_name[] = "mbnavadjusttest";
	char help_message[] = "mbnavadjusttest loads a mbnavadjust project and tests matching features in a specified crossing.\n";
	char usage_message[] = "mbnavadjusttest --input=project_path \n"
	                       "\t[--input=project_path --output=project_path\n"
	                       "\t--crossing=file1:section1/file2:section2\n"
	                       "\t--algorithm-cross-correlation\n"
	                       "\t--algorithm-icp\n"
	                       "\t--verbose --help]\n";
	extern char *optarg;
	int option_index;
	int errflg = 0;
	int c;
	int help = 0;

	/* MBIO status variables */
	int status = MB_SUCCESS;
	int verbose = 0;
	int error = MB_ERROR_NO_ERROR;
    
    /* mbnavadjust variables */
    mb_path project_path;
    int project_set = MB_NO;
    struct mbna_project project;
    struct mbna_crossing *crossing;
    struct mbna_file *file1, *file2;
    struct mbna_section *section1, *section2;
    int icrossing, crossing_id;
    int ifile1, ifile2, isection1, isection2;
    int crossing_set = MB_NO;
    int algorithm = MBNAVADJUSTTEST_ALGORITHM_CROSS_CORRELATION;

    /* swath data storage */
    struct swathraw *swathraw1 = NULL;
    struct swathraw *swathraw2 = NULL;
    struct swath *swath1 = NULL;
    struct swath *swath2 = NULL;
    struct ping *ping = NULL;

	/* command line option definitions */
	/* mbnavadjustmerge --verbose
	 * 		--help
	 * 		--input=project_path
	 * 		--crossing=file1:section1/file2:section2
	 * 		--algorithm-cross-correlation
	 * 		--algorithm-icp
	 */
	static struct option options[] = {{"verbose", no_argument, NULL, 0},
	                                  {"help", no_argument, NULL, 0},
	                                  {"input", required_argument, NULL, 0},
	                                  {"crossing", required_argument, NULL, 0},
	                                  {"algorithm-cross-correlation", no_argument, NULL, 0},
	                                  {"algorithm-icp", no_argument, NULL, 0},
	                                  {NULL, 0, NULL, 0}};
    
    int nscan;
    int found = MB_NO;

	memset(project_path, 0, sizeof(mb_path));

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
				help = MB_YES;
			}

			/*-------------------------------------------------------
			 * Define input project */

			/* input */
			else if (strcmp("input", options[option_index].name) == 0) {
				if (project_set == MB_NO) {
					strcpy(project_path, optarg);
					project_set = MB_YES;
				}
				else {
					fprintf(stderr, "Input project already set:\n\t%s\nProject %s ignored...\n\n", project_path, optarg);
				}
			}

			/*-------------------------------------------------------
			 * crossing to be tested
			    --crossing=file1:section1/file2:section2 */
			else if (strcmp("crossing", options[option_index].name) == 0) {
				if ((nscan = sscanf(optarg, "%d:%d/%d:%d", &ifile1, &isection1, &ifile2, &isection2)) == 4) {
                    crossing_set++;
				}
				else {
					fprintf(stderr, "Failure to parse --crossing=%s\n\tmod command ignored\n\n", optarg);
				}
			}


			/*-------------------------------------------------------
			 * matching features using 2D cross correlation
			    --algorithm-cross-correlation */
			else if (strcmp("algorithm-cross-correlation", options[option_index].name) == 0) {
				algorithm = MBNAVADJUSTTEST_ALGORITHM_CROSS_CORRELATION;
			}

			/* matching features using ICP
			    --algorithm-icp */
			else if (strcmp("algorithm-icp", options[option_index].name) == 0) {
				algorithm = MBNAVADJUSTTEST_ALGORITHM_ICP;
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

	/* print starting message */
	if (verbose == 1 || help) {
		fprintf(stderr, "\nProgram %s\n", program_name);
		fprintf(stderr, "Source File Version %s\n", version_id);
		fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
	}

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
		fprintf(stderr, "dbg2  Version %s\n", version_id);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Control Parameters:\n");
		fprintf(stderr, "dbg2       verbose:                    %d\n", verbose);
		fprintf(stderr, "dbg2       help:                       %d\n", help);
		fprintf(stderr, "dbg2       project_set:                %d\n", project_set);
		fprintf(stderr, "dbg2       project_path:               %s\n", project_path);
		fprintf(stderr, "dbg2       ifile1:                     %d\n", ifile1);
		fprintf(stderr, "dbg2       isection1:                  %d\n", isection1);
		fprintf(stderr, "dbg2       ifile2:                     %d\n", ifile2);
		fprintf(stderr, "dbg2       isection2:                  %d\n", isection2);
		fprintf(stderr, "dbg2       algorithm:                  %d\n", algorithm);
	}

	/* if help desired then print it and exit */
	if (help) {
		fprintf(stderr, "\n%s\n", help_message);
		fprintf(stderr, "\nusage: %s\n", usage_message);
		exit(error);
	}

	/* initialize the project structure */
	memset(&project, sizeof(struct mbna_project), 0);

    /* read the input project */
    status = mbnavadjust_read_project(verbose, project_path, &project, &error);
    if (status == MB_SUCCESS) {
        fprintf(stderr, "\nInput project loaded:\n\t%s\n", project_path);
        fprintf(stderr, "\t%d files\n\t%d crossings\n\t%d ties\n", project.num_files, project.num_crossings,
                project.num_ties);
    }
    else {
        fprintf(stderr, "Load failure for input project:\n\t%s\n",
                project_path);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        error = MB_ERROR_BAD_USAGE;
        exit(error);
    }
    
    /* check if the crossing exists */
    found = MB_NO;
    for (icrossing = 0; icrossing < project.num_crossings && found == MB_NO; icrossing++) {
		/* get structure */
		crossing = &(project.crossings[icrossing]);
        
        if (crossing->file_id_1 == ifile1 && crossing->section_1 == isection1
            && crossing->file_id_2 == ifile2 && crossing->section_2 == isection2) {
            found = MB_YES;
            crossing_id = icrossing;
        }
    }
    
    /* load the crossings */
    if (found == MB_YES) {
		file1 = (struct mbna_file *)&project.files[crossing->file_id_1];
		file2 = (struct mbna_file *)&project.files[crossing->file_id_2];
		section1 = (struct mbna_section *)&file1->sections[crossing->section_1];
		section2 = (struct mbna_section *)&file2->sections[crossing->section_2];

		/* load sections */
		fprintf(stderr, "Loading section 1 of crossing %d...", crossing_id);
		status = mbnavadjust_section_load(verbose, &project, crossing->file_id_1, crossing->section_1,
                                          (void **)&swathraw1, (void **)&swath1, section1->num_pings, &error);
		fprintf(stderr, "Loading section 2 of crossing %d...", crossing_id);
		status = mbnavadjust_section_load(verbose, &project, crossing->file_id_2, crossing->section_2,
                                          (void **)&swathraw2, (void **)&swath2, section2->num_pings, &error);

		/* get lon lat positions for soundings */
		fprintf(stderr, "Transforming section 1 of crossing %d...", crossing_id);
		status = mbnavadjust_section_translate(verbose, &project, crossing->file_id_1, swathraw1, swath1, 0.0, &error);
		fprintf(stderr, "Transforming section 2 of crossing %d...", crossing_id);
		status = mbnavadjust_section_translate(verbose, &project, crossing->file_id_2, swathraw2, swath2, 0.0, &error);
        
    }
}




