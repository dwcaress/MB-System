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
    "complete list of images and camera settings. The results are dumped to stdout.\n"
    "The --files option (the default) outputs one image path per line.\n"
    "The --settings option outputs \"path time_d gain exposure\" per image.\n"
    "The --imagelist option outputs the full imagelist row format:\n"
    "  left-image-path right-image-path left-time_d right-time_d "
    "left-gain right-gain left-exposure right-exposure\n"
    "with a missing side represented by \"NULL\". With --imagelist in use,\n"
    "--parameters outputs processing parameter lines in the imagelist file\n"
    "form (e.g. \"#PARAMETER --calibration-file=20250325_LASSCameraCalibration.yml\").\n"
    "The --start and --end options restrict the image listings output to\n"
    "those with timestamps within the specified time period; the default\n"
    "is to output all image listings.";
constexpr char usage_message[] =
    "mbimagelist [--input=file --files --settings --imagelist --parameters\n"
    "\t--start=yyyy/mm/dd/hh/mm/ss.ssssss --end=yyyy/mm/dd/hh/mm/ss.ssssss --verbose --help]";
    
#define MBIMAGELIST_FILECHOICE_ALL 0
#define MBIMAGELIST_FILECHOICE_LEFT 1
#define MBIMAGELIST_FILECHOICE_RIGHT 2
#define MBIMAGELIST_FILECHOICE_SINGLE 3

/* processing parameter options taking a file path argument - these are
   the same options recognized by mbphotomosaic.cc when it reads
   imagelist #PARAMETER entries (e.g. src/photo/mbphotomosaic.cc:3956) */
static const char *mbimagelist_pathopts[] = {
    "--correction-file=",
    "--platform-file=",
    "--calibration-file=",
    "--navigation-file=",
    "--tide-file=",
    "--image-quality-file=",
    nullptr};

/*--------------------------------------------------------------------*/
/* If the processing parameter string in parameter is a recognized
   file-path option, and directory is not empty, rewrite the file path
   value to be prefixed by directory - the directory of the imagelist
   file in which the #PARAMETER entry was found. This follows the same
   convention used by mbphotomosaic.cc to resolve file paths given in
   #PARAMETER entries relative to the imagelist file's location rather
   than the current working directory. An already-absolute file path
   (starting with '/') is left unmodified. */
void mbimagelist_resolve_parameter_path(const char *directory, mb_path parameter) {
	if (strlen(directory) == 0)
		return;

	for (int i = 0; mbimagelist_pathopts[i] != nullptr; i++) {
		const char *opt = mbimagelist_pathopts[i];
		const size_t optlen = strlen(opt);
		if (strncmp(parameter, opt, optlen) == 0) {
			mb_path tmp;
			if (sscanf(parameter + optlen, "%s", tmp) == 1 && tmp[0] != '/') {
				mb_path newparameter;
				snprintf(newparameter, sizeof(newparameter), "%s%s/%s", opt, directory, tmp);
				strcpy(parameter, newparameter);
			}
			return;
		}
	}
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
	
	bool absolutepaths = false;
	bool copyfiles = false;
	mb_path copydirectory = "";
	bool files = false;
	mb_path read_file = "imagelist.mb-2";
	bool parameters = false;
	bool settings = false;
	bool output_imagelist = false;
	int imagechoice = MBIMAGELIST_FILECHOICE_ALL;
	bool use_start_time = false;
	bool use_end_time = false;
	double start_time_d = 0.0;
	double end_time_d = 0.0;
	
	FILE *output = stdout;

	{
		int option_index;

		const struct option options[] = {
	            	{"absolutepaths", no_argument, nullptr, 0},
	        		{"copy", required_argument, nullptr, 0},
	            	{"copyhere", no_argument, nullptr, 0},
	            	{"end", required_argument, nullptr, 0},
	            	{"file", no_argument, nullptr, 0},
	            	{"files", no_argument, nullptr, 0},
	            	{"help", no_argument, nullptr, 0},
	            	{"imagelist", no_argument, nullptr, 0},
	        		{"input", required_argument, nullptr, 0},
					{"left", no_argument, nullptr, 0},
					{"parameter", no_argument, nullptr, 0},
					{"parameters", no_argument, nullptr, 0},
					{"right", no_argument, nullptr, 0},
					{"setting", no_argument, nullptr, 0},
					{"settings", no_argument, nullptr, 0},
					{"single", no_argument, nullptr, 0},
					{"start", required_argument, nullptr, 0},
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
				else if (strcmp("end", options[option_index].name) == 0) {
					int time_i[7] = {0, 0, 0, 0, 0, 0, 0};
					double seconds = 0.0;
					sscanf(optarg, "%d/%d/%d/%d/%d/%lf", &time_i[0], &time_i[1], &time_i[2],
									&time_i[3], &time_i[4], &seconds);
					time_i[5] = (int)seconds;
					time_i[6] = (int)(1000000 * (seconds - time_i[5]));
					mb_get_time(verbose, time_i, &end_time_d);
					use_end_time = true;
				}
				else if (strcmp("file", options[option_index].name) == 0 || strcmp("files", options[option_index].name) == 0) {
					files = true;
				}
				else if (strcmp("help", options[option_index].name) == 0) {
					help = true;
				}
				else if (strcmp("imagelist", options[option_index].name) == 0) {
					output_imagelist = true;
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
				else if (strcmp("start", options[option_index].name) == 0) {
					int time_i[7] = {0, 0, 0, 0, 0, 0, 0};
					double seconds = 0.0;
					sscanf(optarg, "%d/%d/%d/%d/%d/%lf", &time_i[0], &time_i[1], &time_i[2],
									&time_i[3], &time_i[4], &seconds);
					time_i[5] = (int)seconds;
					time_i[6] = (int)(1000000 * (seconds - time_i[5]));
					mb_get_time(verbose, time_i, &start_time_d);
					use_start_time = true;
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
		
		if (!files && !parameters && !settings && !output_imagelist)
			files = true;
		if (output_imagelist)
			files = settings = false;
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
			fprintf(output, "dbg2       output_imagelist:    %d\n", output_imagelist);
			fprintf(output, "dbg2       use_start_time:      %d\n", use_start_time);
			fprintf(output, "dbg2       start_time_d:        %f\n", start_time_d);
			fprintf(output, "dbg2       use_end_time:        %d\n", use_end_time);
			fprintf(output, "dbg2       end_time_d:          %f\n", end_time_d);
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
	bool rectified = false;
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

  while ((status = mb_imagelist_read(verbose, imagelist, &imageStatus, &rectified, 
                                imageLeftFile, imageRightFile, dpath,
                                &left_time_d, &right_time_d,
                                &left_gain, &right_gain,
                                &left_exposure, &right_exposure, &error)) == MB_SUCCESS) {
        if (imageStatus == MB_IMAGESTATUS_PARAMETER) {
        	if (parameters) {
        		mbimagelist_resolve_parameter_path(imageRightFile, imageLeftFile);
        		if (output_imagelist)
        			fprintf(output, "#PARAMETER %s\n", imageLeftFile);
        		else
            		fprintf(output, "  ->Processing parameter: %s\n",imageLeftFile);
        	}
        }
        else if (imageStatus != MB_IMAGESTATUS_NONE) {
			const bool has_left = (imageStatus == MB_IMAGESTATUS_STEREO
						|| imageStatus == MB_IMAGESTATUS_LEFT
						|| imageStatus == MB_IMAGESTATUS_SINGLE);
			const bool has_right = (imageStatus == MB_IMAGESTATUS_STEREO
						|| imageStatus == MB_IMAGESTATUS_RIGHT);

			/* skip entries not matching the requested --left/--right/--single selection */
			if ((imagechoice == MBIMAGELIST_FILECHOICE_LEFT
						&& imageStatus != MB_IMAGESTATUS_STEREO && imageStatus != MB_IMAGESTATUS_LEFT)
					|| (imagechoice == MBIMAGELIST_FILECHOICE_RIGHT
						&& imageStatus != MB_IMAGESTATUS_STEREO && imageStatus != MB_IMAGESTATUS_RIGHT)
					|| (imagechoice == MBIMAGELIST_FILECHOICE_SINGLE
						&& imageStatus != MB_IMAGESTATUS_SINGLE)) {
				continue;
			}
			/* suppress the non-selected side of a stereo pair when --left or --right is in effect */
			const bool emit_left = has_left && (imagechoice != MBIMAGELIST_FILECHOICE_RIGHT);
			const bool emit_right = has_right && (imagechoice != MBIMAGELIST_FILECHOICE_LEFT);

			const double record_time_d = has_left ? left_time_d : right_time_d;
			if ((use_start_time && record_time_d < start_time_d)
						|| (use_end_time && record_time_d > end_time_d)) {
				continue;
			}

			if (absolutepaths) {
				if (emit_left)
					mb_get_absolute_path(verbose, imageLeftFile, pwd, &error);
				if (emit_right)
					mb_get_absolute_path(verbose, imageRightFile, pwd, &error);
			}

			if (output_imagelist) {
				fprintf(output, "%s %s %.6f %.6f %f %f %f %f\n",
								emit_left ? imageLeftFile : "NULL",
								emit_right ? imageRightFile : "NULL",
								emit_left ? left_time_d : 0.0,
								emit_right ? right_time_d : 0.0,
								emit_left ? left_gain : 0.0,
								emit_right ? right_gain : 0.0,
								emit_left ? left_exposure : 0.0,
								emit_right ? right_exposure : 0.0);
			}
			else {
				if (emit_left) {
					if (settings) {
						fprintf(output, "%s %.6f %f %f\n",
										imageLeftFile, left_time_d, left_gain, left_exposure);
					}
					else {
						fprintf(output, "%s\n", imageLeftFile);
					}
				}
				if (emit_right) {
					if (settings) {
						fprintf(output, "%s %.6f %f %f\n",
										imageRightFile, right_time_d, right_gain, right_exposure);
					}
					else {
						fprintf(output, "%s\n", imageRightFile);
					}
				}
			}

			if (emit_left) {
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
			if (emit_right) {
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
