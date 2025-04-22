/*--------------------------------------------------------------------
 *    The MB-system:	mbimagelist.c	10/10/2001
 *
 *    Copyright (c) 2025-2025 by
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
 * MBimagelist parses recursive imagelist files and outputs the
 * complete list of data files and formats.
 * The results are dumped to stdout.
 *
 * Author:	D. W. Caress
 * Date:	March 15, 2025
 */

#include <assert.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_process.h"
#include "mb_status.h"

constexpr char program_name[] = "mbimagelist";
constexpr char help_message[] =
    "mbimagelist parses recursive imagelist files and outputs the\n"
    "complete list of images and camera settings. The results are dumped to stdout.";
constexpr char usage_message[] =
    "mbimagelist [--input=file --parameters --settings --verbose --help]";
    
#define MBIMAGELIST_FILECHOICE_ALL 0
#define MBIMAGELIST_FILECHOICE_LEFT 1
#define MBIMAGELIST_FILECHOICE_RIGHT 2
#define MBIMAGELIST_FILECHOICE_SINGLE 3

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
	
	bool absolutepaths = false;
	bool copyfiles = false;
	mb_path copydirectory = "";
	bool files = false;
	mb_path read_file = "imagelist.mb-2";
	bool parameters = false;
	bool settings = false;
	bool imagechoice = MBIMAGELIST_FILECHOICE_ALL; 
	
	FILE *output = stdout;

	{
		int option_index;

		const struct option options[] = {
	            	{"absolutepaths", no_argument, nullptr, 0},
	        		{"copy", required_argument, nullptr, 0},
	            	{"copyhere", no_argument, nullptr, 0},
	            	{"file", no_argument, nullptr, 0},
	            	{"files", no_argument, nullptr, 0},
	            	{"help", no_argument, nullptr, 0},
	        		{"input", required_argument, nullptr, 0},
					{"left", no_argument, nullptr, 0},
					{"parameter", no_argument, nullptr, 0},
					{"parameters", no_argument, nullptr, 0},
					{"right", no_argument, nullptr, 0},
					{"setting", no_argument, nullptr, 0},
					{"settings", no_argument, nullptr, 0},
					{"single", no_argument, nullptr, 0},
					{"verbose", no_argument, nullptr, 0},
	                {nullptr, 0, nullptr, 0}};

		bool errflg = false;
		int c;
		bool help = false;
		while ((c = getopt_long(argc, argv, "AaC:c:FfHhI:i:LlPpRrSsVv", options, &option_index)) != -1)
		{
			switch (c) {
			/* long options */
			case 0:
				if (strcmp("absolute", options[option_index].name) == 0) {
					absolutepaths = true;
				}
				else if (strcmp("copy", options[option_index].name) == 0) {
					sscanf(optarg, "%1023s", copydirectory);
					if (strlen(copydirectory) > 0)
						copyfiles = true;
				}
				else if (strcmp("copyhere", options[option_index].name) == 0) {
					char *t = getcwd(copydirectory, sizeof(mb_path));
					if (t != nullptr)
						copyfiles = true;
				}
				else if (strcmp("file", options[option_index].name) == 0 || strcmp("files", options[option_index].name) == 0) {
					files = true;
				}
				else if (strcmp("help", options[option_index].name) == 0) {
					help = true;
				}
				else if (strcmp("input", options[option_index].name) == 0) {
					sscanf(optarg, "%1023s", read_file);
				}
				else if (strcmp("left", options[option_index].name) == 0 ) {
					imagechoice = MBIMAGELIST_FILECHOICE_LEFT;
				}
				else if (strcmp("parameter", options[option_index].name) == 0 || strcmp("parameters", options[option_index].name) == 0) {
					parameters = true;
				}
				else if (strcmp("right", options[option_index].name) == 0 ) {
					imagechoice = MBIMAGELIST_FILECHOICE_RIGHT;
				}
				else if (strcmp("setting", options[option_index].name) == 0 || strcmp("settings", options[option_index].name) == 0) {
					settings = true;
				}
				else if (strcmp("single", options[option_index].name) == 0 ) {
					imagechoice = MBIMAGELIST_FILECHOICE_SINGLE;
				}
				else if (strcmp("verbose", options[option_index].name) == 0) {
					verbose++;
				}

				break;

			/* short options (deprecated) */
			case 'A':
			case 'a':
				absolutepaths = true;
				break;
			case 'C':
			case 'c':
				sscanf(optarg, "%1023s", copydirectory);
				if (strlen(copydirectory) > 0)
					copyfiles = true;
				break;
			case 'F':
			case 'f':
				files = true;
				break;
			case 'H':
			case 'h':
				help = true;
				break;
			case 'I':
			case 'i':
				sscanf(optarg, "%1023s", read_file);
				break;
			case 'L':
			case 'l':
				imagechoice = MBIMAGELIST_FILECHOICE_LEFT;
				break;
			case 'P':
			case 'p':
				parameters = true;
				break;
			case 'R':
			case 'r':
				imagechoice = MBIMAGELIST_FILECHOICE_RIGHT;
				break;
			case 'S':
			case 's':
				settings = true;
				break;
			case 'V':
			case 'v':
				verbose++;
				break;
			case '?':
				errflg = true;
			}
		}
		
		if (!files && !parameters && !settings)
			files = true;
		else if (files && settings)
			files = false;

		if (verbose <= 1)
			output = stdout;
		else
			output = stderr;

		if (errflg) {
			fprintf(output, "usage: %s\n", usage_message);
			fprintf(output, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_BAD_USAGE);
		}

		if (verbose == 1 || help) {
			fprintf(output, "\nProgram %s\n", program_name);
			fprintf(output, "MB-system Version %s\n", MB_VERSION);
		}

		if (verbose >= 2) {
			fprintf(output, "\ndbg2  Program <%s>\n", program_name);
			fprintf(output, "dbg2  MB-system Version %s\n", MB_VERSION);
			fprintf(output, "dbg2  Control Parameters:\n");
			fprintf(output, "dbg2       verbose:             %d\n", verbose);
			fprintf(output, "dbg2       help:                %d\n", help);
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
			fprintf(output, "dbg2       absolutepaths:       %d\n", absolutepaths);
			fprintf(output, "dbg2       copyfiles:           %d\n", copyfiles);
			fprintf(output, "dbg2       copydirectory:       %s\n", copydirectory);
			fprintf(output, "dbg2       files:               %d\n", files);
			fprintf(output, "dbg2       read_file:           %s\n", read_file);
			fprintf(output, "dbg2       imagechoice:         %d\n", imagechoice);
			fprintf(output, "dbg2       parameters:          %d\n", parameters);
			fprintf(output, "dbg2       settings:            %d\n", settings);
		}

		if (help) {
			fprintf(output, "\n%s\n", help_message);
			fprintf(output, "\nusage: %s\n", usage_message);
			exit(MB_ERROR_NO_ERROR);
		}
	}

	int error = MB_ERROR_NO_ERROR;
	void *imagelist;
    int imageStatus = MB_IMAGESTATUS_NONE;
    mb_path imageLeftFile = "";
    mb_path imageRightFile = "";
    mb_path dpath = "";
	mb_path pwd = "";
	if (absolutepaths)
    	assert(getcwd(pwd, MB_PATH_MAXLINE) != NULL);
    double left_time_d;
    double right_time_d;
    double left_gain;
    double right_gain;
    double left_exposure;
    double right_exposure;
    
    int num_left_images = 0;
    int num_right_images = 0;
    int num_single_images = 0;
    int num_total_images = 0;

	if (mb_imagelist_open(verbose, &imagelist, read_file, &error) != MB_SUCCESS) {
		fprintf(stderr, "\nUnable to open imagelist file: %s\n", read_file);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(MB_ERROR_OPEN_FAIL);
	}

    while ((status = mb_imagelist_read(verbose, imagelist, &imageStatus,
                                imageLeftFile, imageRightFile, dpath,
                                &left_time_d, &right_time_d,
                                &left_gain, &right_gain,
                                &left_exposure, &right_exposure, &error)) == MB_SUCCESS) {
        if (imageStatus == MB_IMAGESTATUS_PARAMETER) {
        	if (parameters)
            	fprintf(output, "  ->Processing parameter: %s\n",imageLeftFile);
        }
        else if (imageStatus != MB_IMAGESTATUS_NONE) {
			if (imageStatus == MB_IMAGESTATUS_STEREO 
						|| imageStatus == MB_IMAGESTATUS_LEFT 
						|| imageStatus == MB_IMAGESTATUS_SINGLE) {
				if (absolutepaths) {
					mb_get_absolute_path(verbose, imageLeftFile, pwd, &error);
				}
				if (settings) {
					fprintf(output, "%s %.6f %f %f\n", 
									imageLeftFile, left_time_d, left_gain, left_exposure);
				}
				else {
					fprintf(output, "%s\n", imageLeftFile);
				}
				if (imageStatus == MB_IMAGESTATUS_SINGLE)
					num_single_images++;
				else
					num_left_images++;
				if (copyfiles) {
					mb_path command;
					snprintf(command, sizeof(command), "cp %s %s", imageLeftFile, copydirectory);
					int shellstatus = system(command);
					if (shellstatus == 0)
						fprintf(output, "Executed: %s\n", command);
				}
			}
			if (imageStatus == MB_IMAGESTATUS_STEREO 
						|| imageStatus == MB_IMAGESTATUS_RIGHT) {
				if (absolutepaths) {
					mb_get_absolute_path(verbose, imageRightFile, pwd, &error);
				}
				if (settings) {
					fprintf(output, "%s %.6f %f %f\n", 
									imageRightFile, right_time_d, right_gain, right_exposure);
				}
				else {
					fprintf(output, "%s\n", imageRightFile);
				}
				num_right_images++;
				if (copyfiles) {
					mb_path command;
					snprintf(command, sizeof(command), "cp %s %s", imageRightFile, copydirectory);
					int shellstatus = system(command);
					if (shellstatus == 0)
						fprintf(output, "Executed: %s\n", command);
				}
			}
		}
    }
	mb_imagelist_close(verbose, &imagelist, &error);
	num_total_images = num_left_images + num_right_images + num_single_images;

	/* set program status */
	// status = MB_SUCCESS;

	/* output counts */
	if (verbose > 0) {
		fprintf(output, "\nNumbers of images:\n");
		fprintf(output, "    %d left images\n", num_left_images);
		fprintf(output, "    %d right images\n", num_right_images);
		fprintf(output, "    %d single images\n", num_single_images);
		fprintf(output, "    %d total images\n", num_total_images);
	}

	/* check memory */
	if ((status = mb_memory_list(verbose, &error)) == MB_FAILURE) {
	  fprintf(stderr, "Program %s completed but failed to deallocate all allocated memory - the code has a memory leak somewhere!\n", program_name);
	}

	exit(error);
}
/*--------------------------------------------------------------------*/
