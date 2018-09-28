/*--------------------------------------------------------------------
 *    The MB-system:	mbvoxelclean.c	8/27/2018
 *    $Id:  $
 *
 *    Copyright (c) 2018-2018 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbvoxelclean identifies and flags artifacts in swath bathymetry data
 * using a 3D density filter. The notion applied is that true targets
 * (e.g. the seafloor) result in dense regions of soundings while sparse soundings
 * in the water column or the subsurface are erroneous and can be flagged as bad.
 * This technique is more appropriate for lidar data than multibeam sonar data.
 * The resulting sounding edit events are output to edit save files which can be 
 * applied to the data by the program mbprocess. These are the same edit save
 * files created and/or modified by mbclean and mbedit.
 * The input data are one swath file or a datalist referencing multiple
 * swath files. Each file is read and processed separately.
 * The rectangular prism including all of the flagged and unflagged soundings
 * is divided into 3D voxels of the specified size. All of the soundings are
 * read into memory and associated with one of the voxels. Once all of
 * data are read, a density filter is applied such that containing more than a
 * specified threshold of soundings are considered to be occupied by a valid target and
 * voxels containing less than the threshold are considered to be empty.
 * The user may specify one or both of the following actions:
 *   1) Previously unflagged soundings in an empty voxel are flagged as bad.
 *   2) Previously flagged soundings in a full voxel are unflagged.
 * This program will also apply specified range minimum and maximum filters.
 * If a sounding's flag status is changed, that flagging action is output
 * to the edit save file of the swath file containing that sounding. This
 * program will create edit save files if necessary, or append to those that
 * already exist.
 *
 * Author:	D. W. Caress
 * Date:	August 3, 2018
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_io.h"
#include "mb_swap.h"
#include "mb_process.h"
#include "mb_info.h"

static char version_id[] = "$Id:  $";

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	char program_name[] = "mbvoxelclean";
	char help_message[] = "mbvoxelclean parses recursive datalist files and outputs the\ncomplete list of data files and formats. "
	                      "\nThe results are dumped to stdout.";
	char usage_message[] = "mbvoxelclean \n\t[\n\t--verbose\n\t--help\n"
                            "\t--input=datalist\n\t--format=value\n\t--voxel-size=xysize[/zsize]\n"
                            "\toccupy-threshold\n\t--flag-empty\n\t--ignore-empty\n\t--unflag-occupied\n"
                            "\t--ignore-occupied\n\t--range-minimum=value\n\t--range-maximum=value]";
	extern char *optarg;
	int option_index;
	int errflg = 0;
	int c;
	int help = 0;
	int flag = 0;

	/* MBIO status variables */
	int status;
	int verbose = 0;
	int error = MB_ERROR_NO_ERROR;
	char *message = NULL;

	/* MBIO read control parameters */
	void *mbio_ptr = NULL;
	void *store_ptr = NULL;
	int kind;
	int read_datalist = MB_NO;
	char read_file[MB_PATH_MAXLINE];
	char swathfile[MB_PATH_MAXLINE];
	char swathfileread[MB_PATH_MAXLINE];
	char dfile[MB_PATH_MAXLINE];
	void *datalist;
	int look_processed = MB_DATALIST_LOOK_NO;
	int read_data;
	double file_weight;
	int format;
	int formatread;
	int variable_beams;
	int traveltime;
	int beam_flagging;
	int pings;
	int lonflip;
	double bounds[4];
	int btime_i[7];
	int etime_i[7];
	double btime_d;
	double etime_d;
	double speedmin;
	double timegap;
	struct mb_info_struct mb_info;

	/* command line option definitions */
	/* mbvoxelclean 
     *     [
     *     --verbose
     *     --help
     *     --input=datalist
     *     --format=value
     *     --voxel-size=xysize[/zsize]
     *     --occupy-threshold
     *     --flag-empty
     *     --ignore-empty
     *     --unflag-occupied
     *     --ignore-occupied
     *     --range-minimum=value
     *     --range-maximum=value
     *     ]
     */
	static struct option options[] = {{"verbose", no_argument, NULL, 0},
	                                  {"help", no_argument, NULL, 0},
	                                  {"input", required_argument, NULL, 0},
	                                  {"format", required_argument, NULL, 0},
	                                  {"voxel-size", required_argument, NULL, 0},
	                                  {"occupy-threshold", required_argument, NULL, 0},
	                                  {"flag-empty", no_argument, NULL, 0},
	                                  {"ignore-empty", no_argument, NULL, 0},
	                                  {"unflag-occupied", no_argument, NULL, 0},
	                                  {"ignore-occupied", no_argument, NULL, 0},
	                                  {"range-minimum", required_argument, NULL, 0},
	                                  {"range-maximum", required_argument, NULL, 0},
	                                  {NULL, 0, NULL, 0}};

    /* other mbvoxelclean control parameters */
    double voxel_size_xy = 0.05;
    double voxel_size_z = 0.05;
    int occupy_threshold = 5;
    int flag_empty = MB_YES;
    int ignore-empty = MB_NO;
    int unflag_occupied = MB_YES;
    int ignore_occupied = MB_NO;
    int apply_range_minimum = MB_NO;
    double range_minimum = 0.0;
    int apply_range_maximum = MB_NO;
    double range_maximum = 0.0;

	/* process argument list */
	while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1) {
		switch (c) {
		/* long options */
		case 0:
			/* verbose */
			if (strcmp("verbose", options[option_index].name) == 0) {
				verbose++;
			}

			/* help */
			else if (strcmp("help", options[option_index].name) == 0) {
				help = MB_YES;
			}

			/* input */
			else if (strcmp("input", options[option_index].name) == 0) {
				sscanf(optarg, "%s", read_file);
				flag++;
			}

			/* format */
			else if (strcmp("format", options[option_index].name) == 0) {
				sscanf(optarg, "%d", &format);
				flag++;
			}

			/* voxel-size */
			else if (strcmp("voxel-size", options[option_index].name) == 0) {
				sscanf(optarg, "%lf", &voxel_size);
				flag++;
			}

			/* occupy-threshold */
			else if (strcmp("occupy-threshold", options[option_index].name) == 0) {
				sscanf(optarg, "%lf", &voxel_size);
				flag++;
			}

			/* flag-empty */
			else if (strcmp("flag-empty", options[option_index].name) == 0) {
				empty_mode = MBVC_EMPTY_FLAG;
				flag++;
			}

			/* ignore-empty */
			else if (strcmp("ignore-empty", options[option_index].name) == 0) {
				empty_mode = MBVC_EMPTY_IGNORE;
				flag++;
			}

			/* unflag-occupied files */
			else if (strcmp("unflag-occupied", options[option_index].name) == 0) {
				occupied_mode = MBVC_OCCUPIED_UNFLAG;
				flag++;
			}

			/* ignore-occupied */
			else if (strcmp("ignore-occupied", options[option_index].name) == 0) {
				occupied_mode = MBVC_OCCUPIED_IGNORE;
				flag++;
			}

			/* range-minimum  */
			else if (strcmp("range-minimum", options[option_index].name) == 0) {
				range_minimum_apply = MB_YES;
				sscanf(optarg, "%lf", &range_minimum);
				flag++;
			}

			/* range-maximum */
			else if (strcmp("range-maximum", options[option_index].name) == 0) {
				range_maximum_apply = MB_YES;
				sscanf(optarg, "%lf", &range_maximum);
			}
            
            break;
            
		case '?':
			errflg++;
		}
    }


	/* if error flagged then print it and exit */
	if (errflg) {
		fprintf(output, "usage: %s\n", usage_message);
		fprintf(output, "\nProgram <%s> Terminated\n", program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
	}

	/* print starting message */
	if (verbose == 1 || help) {
		fprintf(output, "\nProgram %s\n", program_name);
		fprintf(output, "Version %s\n", rcs_id);
		fprintf(output, "MB-system Version %s\n", MB_VERSION);
	}

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(output, "\ndbg2  Program <%s>\n", program_name);
		fprintf(output, "dbg2  Version %s\n", rcs_id);
		fprintf(output, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(output, "dbg2  Control Parameters:\n");
		fprintf(output, "dbg2       verbose:             %d\n", verbose);
		fprintf(output, "dbg2       help:                %d\n", help);
		fprintf(output, "dbg2       file:                %s\n", read_file);
		fprintf(output, "dbg2       format:              %d\n", format);
		fprintf(output, "dbg2       look_processed:      %d\n", look_processed);
		fprintf(output, "dbg2       copyfiles:           %d\n", copyfiles);
		fprintf(output, "dbg2       reportdatalists:     %d\n", reportdatalists);
		fprintf(output, "dbg2       make_inf:            %d\n", make_inf);
		fprintf(output, "dbg2       force_update:        %d\n", force_update);
		fprintf(output, "dbg2       status_report:       %d\n", status_report);
		fprintf(output, "dbg2       problem_report:      %d\n", problem_report);
		fprintf(output, "dbg2       make_datalistp:      %d\n", make_datalistp);
		fprintf(output, "dbg2       remove_locks:        %d\n", remove_locks);
		fprintf(output, "dbg2       pings:               %d\n", pings);
		fprintf(output, "dbg2       lonflip:             %d\n", lonflip);
		fprintf(output, "dbg2       bounds[0]:           %f\n", bounds[0]);
		fprintf(output, "dbg2       bounds[1]:           %f\n", bounds[1]);
		fprintf(output, "dbg2       bounds[2]:           %f\n", bounds[2]);
		fprintf(output, "dbg2       bounds[3]:           %f\n", bounds[3]);
		fprintf(output, "dbg2       btime_i[0]:          %d\n", btime_i[0]);
		fprintf(output, "dbg2       btime_i[1]:          %d\n", btime_i[1]);
		fprintf(output, "dbg2       btime_i[2]:          %d\n", btime_i[2]);
		fprintf(output, "dbg2       btime_i[3]:          %d\n", btime_i[3]);
		fprintf(output, "dbg2       btime_i[4]:          %d\n", btime_i[4]);
		fprintf(output, "dbg2       btime_i[5]:          %d\n", btime_i[5]);
		fprintf(output, "dbg2       btime_i[6]:          %d\n", btime_i[6]);
		fprintf(output, "dbg2       etime_i[0]:          %d\n", etime_i[0]);
		fprintf(output, "dbg2       etime_i[1]:          %d\n", etime_i[1]);
		fprintf(output, "dbg2       etime_i[2]:          %d\n", etime_i[2]);
		fprintf(output, "dbg2       etime_i[3]:          %d\n", etime_i[3]);
		fprintf(output, "dbg2       etime_i[4]:          %d\n", etime_i[4]);
		fprintf(output, "dbg2       etime_i[5]:          %d\n", etime_i[5]);
		fprintf(output, "dbg2       etime_i[6]:          %d\n", etime_i[6]);
		fprintf(output, "dbg2       speedmin:            %f\n", speedmin);
		fprintf(output, "dbg2       timegap:             %f\n", timegap);
	}

	/* if help desired then print it and exit */
	if (help) {
		fprintf(output, "\n%s\n", help_message);
		fprintf(output, "\nusage: %s\n", usage_message);
		exit(error);
	}

