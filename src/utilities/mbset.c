/*--------------------------------------------------------------------
 *    The MB-system:	mbset.c	1/4/2000
 *
 *    Copyright (c) 2000-2019 by
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
 * MBset is a tool for setting values in an mbprocess parameter file.
 * MBprocess is a tool for processing swath sonar bathymetry data
 * which performs a number of functions, including:
 *   - merging navigation
 *   - recalculating bathymetry from travel time and angle data
 *     by raytracing through a layered water sound velocity model.
 *   - applying changes to ship draft, roll bias and pitch bias
 *   - applying bathymetry edits from edit save files.
 * The parameters controlling mbprocess are included in an ascii
 * parameter file. The parameter file syntax is documented by
 * comments in the source file mbsystem/src/mbio/mb_process.h
 * and the manual pages for mbprocess and mbset.
 *
 * Author:	D. W. Caress
 * Date:	January 4, 2000
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_process.h"
#include "mb_status.h"
#include "mb_swap.h"

static const char program_name[] = "mbset";
static const char help_message[] =
    "MBset is a tool for setting values in an mbprocess parameter file.\n"
    "MBprocess is a tool for processing swath sonar bathymetry data\n"
    "which performs a number of functions, including:\n"
    "  - merging navigation\n"
    "  - recalculating bathymetry from travel time and angle data\n"
    "    by raytracing through a layered water sound velocity model.\n"
    "  - applying changes to ship draft, roll bias and pitch bias\n"
    "  - applying bathymetry edits from  edit save files.\n"
    "The parameters controlling mbprocess are included in an ascii\n"
    "parameter file. The parameter file syntax is documented by\n"
    "the manual pages for mbprocess and mbset. \n\n";
static const char usage_message[] = "mbset -Iinfile -PPARAMETER:value [-E -L -N -V -H]";

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	int errflg = 0;
	int c;
	int help = 0;
	int flag = 0;
	int pargc = 0;
	char **pargv = NULL;

	/* MBIO status variables */
	int status = MB_SUCCESS;
	int verbose = 0;
	int error = MB_ERROR_NO_ERROR;

	/* parameter controls */
	struct mb_process_struct process;

	/* processing variables */
	int explicit = MB_NO;
	int read_datalist = MB_NO;
	int read_data = MB_NO;
	char read_file[MBP_FILENAMESIZE];
	void *datalist;
	int look_processed = MB_DATALIST_LOOK_NO;
	double file_weight;
	int lookforfiles = MB_NO;
	int removembnavadjust = MB_NO;
	struct stat file_status;
	int fstat;
	int format = 0;
	char mbp_ifile[MBP_FILENAMESIZE];
	char mbp_dfile[MBP_FILENAMESIZE];
	int mbp_format;
	int write_parameter_file = MB_NO;
	int nscan;
	int i;

	/* set default input and output */
	strcpy(mbp_ifile, "\0");
	strcpy(read_file, "datalist.mb-1");

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhEeF:f:I:i:LlNnP:p:")) != -1)
		switch (c) {
		case 'H':
		case 'h':
			help++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case 'E':
		case 'e':
			explicit = MB_YES;
			flag++;
			break;
		case 'F':
		case 'f':
			sscanf(optarg, "%d", &format);
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf(optarg, "%s", read_file);
			flag++;
			break;
		case 'L':
		case 'l':
			lookforfiles++;
			flag++;
			break;
		case 'N':
		case 'n':
			removembnavadjust++;
			flag++;
			break;
		case 'P':
		case 'p':
			if (strlen(optarg) > 1) {
				/* Replace first '=' before ':' with ':'  */
				for (i = 0; i < strlen(optarg); i++) {
					if (optarg[i] == ':') {
						break;
					}
					else if (optarg[i] == '=') {
						optarg[i] = ':';
						break;
					}
				}

				/* store the parameter argument */
				pargv = (char **)realloc(pargv, (pargc + 1) * sizeof(char *));
				pargv[pargc] = (char *)malloc(strlen(optarg) + 1);
				strcpy(pargv[pargc], optarg);
				pargc++;
			}
			flag++;
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

	if (verbose == 1 || help) {
		fprintf(stderr, "\nProgram %s\n", program_name);
		fprintf(stderr, "MB-System Version %s\n", MB_VERSION);
	}

	/* if help desired then print it and exit */
	if (help) {
		fprintf(stderr, "\n%s\n", help_message);
		fprintf(stderr, "\nusage: %s\n", usage_message);
		exit(error);
	}

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose, read_file, NULL, &format, &error);

	/* determine whether to read one file or a list of files */
	if (format < 0)
		read_datalist = MB_YES;

	/* open file list */
	if (read_datalist == MB_YES) {
		if ((status = mb_datalist_open(verbose, &datalist, read_file, look_processed, &error)) != MB_SUCCESS) {
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr, "\nUnable to open data list file: %s\n", read_file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}
		if ((status = mb_datalist_read(verbose, datalist, mbp_ifile, mbp_dfile, &mbp_format, &file_weight, &error)) == MB_SUCCESS)
			read_data = MB_YES;
		else
			read_data = MB_NO;
	}
	/* else copy single filename to be read */
	else {
		strcpy(mbp_ifile, read_file);
		mbp_format = format;
		read_data = MB_YES;
	}

	/* loop over all files to be read */
	while (read_data == MB_YES) {

		/* load parameters */
		status = mb_pr_readpar(verbose, mbp_ifile, lookforfiles, &process, &error);
		process.mbp_ifile_specified = MB_YES;
		write_parameter_file = MB_NO;

		if (process.mbp_format_specified == MB_NO) {
			process.mbp_format = mbp_format;
			process.mbp_format_specified = MB_YES;
		}
		if (process.mbp_ofile_specified == MB_NO) {
			process.mbp_ofile_specified = MB_YES;
			mb_pr_default_output(verbose, &process, &error);
		}

		/* apply request to remove all reference to mbnavadjust
		    - this includes deleting any adjusted navigation
		    file referenced by the process.mbp_navadjfile */
		if (removembnavadjust == MB_YES) {
			/* delete the navadjust file if it exists */
			if (strlen(process.mbp_navadjfile) > 0 && (fstat = stat(process.mbp_navadjfile, &file_status)) == 0 &&
			    (file_status.st_mode & S_IFMT) != S_IFDIR) {
				remove(process.mbp_navadjfile);
				fprintf(stderr, "Removed navigation adjustment file %s for %s\n", process.mbp_navadjfile, mbp_ifile);
			}

			/* set NAVADJFILE and NAVADJMODE to nothing and off */
			if (strlen(process.mbp_navadjfile) > 0 || process.mbp_navadj_mode != MBP_NAVADJ_OFF) {
				fprintf(stderr, "Turned off navigation adjustment for %s\n", mbp_ifile);
				write_parameter_file = MB_YES;
			}

			process.mbp_navadjfile[0] = '\0';
			process.mbp_navadj_mode = MBP_NAVADJ_OFF;
		}

		/* if any parameters are being set, write a parameter file */
		if (pargc > 0)
			write_parameter_file = MB_YES;

		/* process parameter list */
		for (i = 0; i < pargc; i++) {
			/* general parameters */
			if (strncmp(pargv[i], "OUTFILE", 7) == 0) {
				nscan = sscanf(pargv[i], "OUTFILE:%s", process.mbp_ofile);
				if (nscan == 1)
					process.mbp_ofile_specified = MB_YES;
				else {
					process.mbp_ofile_specified = MB_NO;
					process.mbp_ofile[0] = '\0';
				}
			}
			else if (strncmp(pargv[i], "FORMAT", 6) == 0) {
				sscanf(pargv[i], "FORMAT:%d", &process.mbp_format);
				process.mbp_format_specified = MB_YES;
			}

			/* navigation merging */
			else if (strncmp(pargv[i], "NAVMODE", 7) == 0) {
				sscanf(pargv[i], "NAVMODE:%d", &process.mbp_nav_mode);
				if (explicit == MB_NO && process.mbp_nav_mode == MBP_NAV_OFF) {
					process.mbp_navfile[0] = '\0';
					process.mbp_nav_heading = MBP_NAV_OFF;
					process.mbp_nav_speed = MBP_NAV_OFF;
					process.mbp_nav_draft = MBP_NAV_OFF;
					process.mbp_nav_attitude = MBP_NAV_OFF;
				}
			}
			else if (strncmp(pargv[i], "NAVFILE", 7) == 0) {
				sscanf(pargv[i], "NAVFILE:%s", process.mbp_navfile);
				if (explicit == MB_NO) {
					process.mbp_nav_mode = MBP_NAV_ON;
					process.mbp_nav_heading = MBP_NAV_ON;
					process.mbp_nav_speed = MBP_NAV_ON;
					process.mbp_nav_draft = MBP_NAV_ON;
					process.mbp_nav_attitude = MBP_NAV_ON;
				}
			}
			else if (strncmp(pargv[i], "NAVFORMAT", 9) == 0) {
				sscanf(pargv[i], "NAVFORMAT:%d", &process.mbp_nav_format);
			}
			else if (strncmp(pargv[i], "NAVHEADING", 10) == 0) {
				sscanf(pargv[i], "NAVHEADING:%d", &process.mbp_nav_heading);
			}
			else if (strncmp(pargv[i], "NAVSPEED", 8) == 0) {
				sscanf(pargv[i], "NAVSPEED:%d", &process.mbp_nav_speed);
			}
			else if (strncmp(pargv[i], "NAVDRAFT", 8) == 0) {
				sscanf(pargv[i], "NAVDRAFT:%d", &process.mbp_nav_draft);
			}
			else if (strncmp(pargv[i], "NAVATTITUDE", 11) == 0) {
				sscanf(pargv[i], "NAVATTITUDE:%d", &process.mbp_nav_attitude);
			}
			else if (strncmp(pargv[i], "NAVINTERP", 9) == 0) {
				sscanf(pargv[i], "NAVINTERP:%d", &process.mbp_nav_algorithm);
			}
			else if (strncmp(pargv[i], "NAVTIMESHIFT", 12) == 0) {
				sscanf(pargv[i], "NAVTIMESHIFT:%lf", &process.mbp_nav_timeshift);
			}

			/* navigation offsets and shifts */
			else if (strncmp(pargv[i], "NAVOFFSETX", 10) == 0) {
				sscanf(pargv[i], "NAVOFFSETX:%lf", &process.mbp_nav_offsetx);
				process.mbp_nav_shift = MBP_NAV_ON;
			}
			else if (strncmp(pargv[i], "NAVOFFSETY", 10) == 0) {
				sscanf(pargv[i], "NAVOFFSETY:%lf", &process.mbp_nav_offsety);
				process.mbp_nav_shift = MBP_NAV_ON;
			}
			else if (strncmp(pargv[i], "NAVOFFSETZ", 10) == 0) {
				sscanf(pargv[i], "NAVOFFSETZ:%lf", &process.mbp_nav_offsetz);
				process.mbp_nav_shift = MBP_NAV_ON;
			}
			else if (strncmp(pargv[i], "NAVSHIFTLON", 11) == 0) {
				sscanf(pargv[i], "NAVSHIFTLON:%lf", &process.mbp_nav_shiftlon);
				process.mbp_nav_shift = MBP_NAV_ON;
			}
			else if (strncmp(pargv[i], "NAVSHIFTLAT", 11) == 0) {
				sscanf(pargv[i], "NAVSHIFTLAT:%lf", &process.mbp_nav_shiftlat);
				process.mbp_nav_shift = MBP_NAV_ON;
			}
			else if (strncmp(pargv[i], "NAVSHIFTX", 9) == 0) {
				sscanf(pargv[i], "NAVSHIFTX:%lf", &process.mbp_nav_shiftx);
				process.mbp_nav_shift = MBP_NAV_ON;
			}
			else if (strncmp(pargv[i], "NAVSHIFTY", 9) == 0) {
				sscanf(pargv[i], "NAVSHIFTY:%lf", &process.mbp_nav_shifty);
				process.mbp_nav_shift = MBP_NAV_ON;
			}
			else if (strncmp(pargv[i], "NAVSHIFT", 8) == 0) {
				sscanf(pargv[i], "NAVSHIFT:%d", &process.mbp_nav_shift);
			}

			/* adjusted navigation merging */
			else if (strncmp(pargv[i], "NAVADJMODE", 10) == 0) {
				sscanf(pargv[i], "NAVADJMODE:%d", &process.mbp_navadj_mode);
				if (explicit == MB_NO && process.mbp_navadj_mode == MBP_NAVADJ_OFF) {
					process.mbp_navadjfile[0] = '\0';
				}
			}
			else if (strncmp(pargv[i], "NAVADJFILE", 10) == 0) {
				sscanf(pargv[i], "NAVADJFILE:%s", process.mbp_navadjfile);
				if (explicit == MB_NO) {
					process.mbp_navadj_mode = MBP_NAVADJ_LLZ;
				}
			}
			else if (strncmp(pargv[i], "NAVADJINTERP", 12) == 0) {
				sscanf(pargv[i], "NAVADJINTERP:%d", &process.mbp_navadj_algorithm);
			}

			/* attitude merging */
			else if (strncmp(pargv[i], "ATTITUDEMODE", 12) == 0) {
				sscanf(pargv[i], "ATTITUDEMODE:%d", &process.mbp_attitude_mode);
				if (explicit == MB_NO && process.mbp_attitude_mode == MBP_ATTITUDE_OFF) {
					process.mbp_attitudefile[0] = '\0';
				}
			}
			else if (strncmp(pargv[i], "ATTITUDEFILE", 12) == 0) {
				sscanf(pargv[i], "ATTITUDEFILE:%s", process.mbp_attitudefile);
				if (explicit == MB_NO) {
					process.mbp_attitude_mode = MBP_ATTITUDE_ON;
				}
			}
			else if (strncmp(pargv[i], "ATTITUDEFORMAT", 14) == 0) {
				sscanf(pargv[i], "ATTITUDEFORMAT:%d", &process.mbp_attitude_format);
			}

			/* sonardepth merging */
			else if (strncmp(pargv[i], "SONARDEPTHMODE", 14) == 0) {
				sscanf(pargv[i], "SONARDEPTHMODE:%d", &process.mbp_sonardepth_mode);
				if (explicit == MB_NO && process.mbp_sonardepth_mode == MBP_SONARDEPTH_OFF) {
					process.mbp_sonardepthfile[0] = '\0';
				}
			}
			else if (strncmp(pargv[i], "SONARDEPTHFILE", 14) == 0) {
				sscanf(pargv[i], "SONARDEPTHFILE:%s", process.mbp_sonardepthfile);
				if (explicit == MB_NO) {
					process.mbp_sonardepth_mode = MBP_SONARDEPTH_ON;
				}
			}
			else if (strncmp(pargv[i], "SONARDEPTHFORMAT", 16) == 0) {
				sscanf(pargv[i], "SONARDEPTHFORMAT:%d", &process.mbp_sonardepth_format);
			}

			/* data cutting */
			else if (strncmp(pargv[i], "DATACUTCLEAR", 12) == 0) {
				process.mbp_cut_num = 0;
			}
			else if (strncmp(pargv[i], "DATACUT", 7) == 0) {
				if (process.mbp_cut_num < MBP_CUT_NUM_MAX) {
					sscanf(pargv[i], "DATACUT:%d:%d:%lf:%lf", &process.mbp_cut_kind[process.mbp_cut_num],
					       &process.mbp_cut_mode[process.mbp_cut_num], &process.mbp_cut_min[process.mbp_cut_num],
					       &process.mbp_cut_max[process.mbp_cut_num]);
					process.mbp_cut_num++;
				}
			}
			else if (strncmp(pargv[i], "BATHCUTNUMBER", 13) == 0) {
				if (process.mbp_cut_num < MBP_CUT_NUM_MAX) {
					sscanf(pargv[i], "BATHCUTNUMBER:%lf:%lf", &process.mbp_cut_min[process.mbp_cut_num],
					       &process.mbp_cut_max[process.mbp_cut_num]);
					process.mbp_cut_kind[process.mbp_cut_num] = MBP_CUT_DATA_BATH;
					process.mbp_cut_mode[process.mbp_cut_num] = MBP_CUT_MODE_NUMBER;
					process.mbp_cut_num++;
				}
			}
			else if (strncmp(pargv[i], "BATHCUTDISTANCE", 15) == 0) {
				if (process.mbp_cut_num < MBP_CUT_NUM_MAX) {
					sscanf(pargv[i], "BATHCUTDISTANCE:%lf:%lf", &process.mbp_cut_min[process.mbp_cut_num],
					       &process.mbp_cut_max[process.mbp_cut_num]);
					process.mbp_cut_kind[process.mbp_cut_num] = MBP_CUT_DATA_BATH;
					process.mbp_cut_mode[process.mbp_cut_num] = MBP_CUT_MODE_DISTANCE;
					process.mbp_cut_num++;
				}
			}
			else if (strncmp(pargv[i], "BATHCUTSPEED", 12) == 0) {
				if (process.mbp_cut_num < MBP_CUT_NUM_MAX) {
					sscanf(pargv[i], "BATHCUTSPEED:%lf:%lf", &process.mbp_cut_min[process.mbp_cut_num],
					       &process.mbp_cut_max[process.mbp_cut_num]);
					process.mbp_cut_kind[process.mbp_cut_num] = MBP_CUT_DATA_BATH;
					process.mbp_cut_mode[process.mbp_cut_num] = MBP_CUT_MODE_SPEED;
					process.mbp_cut_num++;
				}
			}
			else if (strncmp(pargv[i], "AMPCUTNUMBER", 12) == 0) {
				if (process.mbp_cut_num < MBP_CUT_NUM_MAX) {
					sscanf(pargv[i], "AMPCUTNUMBER:%lf:%lf", &process.mbp_cut_min[process.mbp_cut_num],
					       &process.mbp_cut_max[process.mbp_cut_num]);
					process.mbp_cut_kind[process.mbp_cut_num] = MBP_CUT_DATA_AMP;
					process.mbp_cut_mode[process.mbp_cut_num] = MBP_CUT_MODE_NUMBER;
					process.mbp_cut_num++;
				}
			}
			else if (strncmp(pargv[i], "AMPCUTDISTANCE", 14) == 0) {
				if (process.mbp_cut_num < MBP_CUT_NUM_MAX) {
					sscanf(pargv[i], "AMPCUTDISTANCE:%lf:%lf", &process.mbp_cut_min[process.mbp_cut_num],
					       &process.mbp_cut_max[process.mbp_cut_num]);
					process.mbp_cut_kind[process.mbp_cut_num] = MBP_CUT_DATA_AMP;
					process.mbp_cut_mode[process.mbp_cut_num] = MBP_CUT_MODE_DISTANCE;
					process.mbp_cut_num++;
				}
			}
			else if (strncmp(pargv[i], "AMPCUTSPEED", 11) == 0) {
				if (process.mbp_cut_num < MBP_CUT_NUM_MAX) {
					sscanf(pargv[i], "AMPCUTSPEED:%lf:%lf", &process.mbp_cut_min[process.mbp_cut_num],
					       &process.mbp_cut_max[process.mbp_cut_num]);
					process.mbp_cut_kind[process.mbp_cut_num] = MBP_CUT_DATA_AMP;
					process.mbp_cut_mode[process.mbp_cut_num] = MBP_CUT_MODE_SPEED;
					process.mbp_cut_num++;
				}
			}
			else if (strncmp(pargv[i], "SSCUTNUMBER", 11) == 0) {
				if (process.mbp_cut_num < MBP_CUT_NUM_MAX) {
					sscanf(pargv[i], "SSCUTNUMBER:%lf:%lf", &process.mbp_cut_min[process.mbp_cut_num],
					       &process.mbp_cut_max[process.mbp_cut_num]);
					process.mbp_cut_kind[process.mbp_cut_num] = MBP_CUT_DATA_SS;
					process.mbp_cut_mode[process.mbp_cut_num] = MBP_CUT_MODE_NUMBER;
					process.mbp_cut_num++;
				}
			}
			else if (strncmp(pargv[i], "SSCUTDISTANCE", 13) == 0) {
				if (process.mbp_cut_num < MBP_CUT_NUM_MAX) {
					sscanf(pargv[i], "SSCUTDISTANCE:%lf:%lf", &process.mbp_cut_min[process.mbp_cut_num],
					       &process.mbp_cut_max[process.mbp_cut_num]);
					process.mbp_cut_kind[process.mbp_cut_num] = MBP_CUT_DATA_SS;
					process.mbp_cut_mode[process.mbp_cut_num] = MBP_CUT_MODE_DISTANCE;
					process.mbp_cut_num++;
				}
			}
			else if (strncmp(pargv[i], "SSCUTSPEED", 10) == 0) {
				if (process.mbp_cut_num < MBP_CUT_NUM_MAX) {
					sscanf(pargv[i], "SSCUTSPEED:%lf:%lf", &process.mbp_cut_min[process.mbp_cut_num],
					       &process.mbp_cut_max[process.mbp_cut_num]);
					process.mbp_cut_kind[process.mbp_cut_num] = MBP_CUT_DATA_SS;
					process.mbp_cut_mode[process.mbp_cut_num] = MBP_CUT_MODE_SPEED;
					process.mbp_cut_num++;
				}
			}

			/* bathymetry editing */
			else if (strncmp(pargv[i], "EDITSAVEMODE", 12) == 0) {
				sscanf(pargv[i], "EDITSAVEMODE:%d", &process.mbp_edit_mode);
				if (explicit == MB_NO && process.mbp_edit_mode == MBP_EDIT_OFF) {
					process.mbp_editfile[0] = '\0';
				}
			}
			else if (strncmp(pargv[i], "EDITSAVEFILE", 12) == 0) {
				sscanf(pargv[i], "EDITSAVEFILE:%s", process.mbp_editfile);
				if (explicit == MB_NO) {
					process.mbp_edit_mode = MBP_EDIT_ON;
				}
			}

			/* bathymetry recalculation */
			else if (strncmp(pargv[i], "RAYTRACE", 8) == 0) {
				sscanf(pargv[i], "RAYTRACE:%d", &process.mbp_svp_mode);
			}
			else if (strncmp(pargv[i], "SVPMODE", 7) == 0) {
				sscanf(pargv[i], "SVPMODE:%d", &process.mbp_svp_mode);
				if (explicit == MB_NO && process.mbp_svp_mode == MBP_SVP_OFF) {
					process.mbp_svpfile[0] = '\0';
				}
			}
			else if (strncmp(pargv[i], "SVPFILE", 7) == 0) {
				sscanf(pargv[i], "SVPFILE:%s", process.mbp_svpfile);
				if (explicit == MB_NO) {
					process.mbp_svp_mode = MBP_SVP_ON;
				}
			}
			else if (strncmp(pargv[i], "SSVMODE", 7) == 0) {
				sscanf(pargv[i], "SSVMODE:%d", &process.mbp_ssv_mode);
			}
			else if (strncmp(pargv[i], "SSV", 3) == 0) {
				sscanf(pargv[i], "SSV:%lf", &process.mbp_ssv);
			}
			else if (strncmp(pargv[i], "TTMODE", 6) == 0) {
				sscanf(pargv[i], "TTMODE:%d", &process.mbp_tt_mode);
			}
			else if (strncmp(pargv[i], "TTMULTIPLY", 10) == 0) {
				sscanf(pargv[i], "TTMULTIPLY:%lf", &process.mbp_tt_mult);
			}
			else if (strncmp(pargv[i], "CORRECTED", 9) == 0) {
				sscanf(pargv[i], "CORRECTED:%d", &process.mbp_corrected);
			}
			else if (strncmp(pargv[i], "ANGLEMODE", 9) == 0) {
				sscanf(pargv[i], "ANGLEMODE:%d", &process.mbp_angle_mode);
			}
			else if (strncmp(pargv[i], "SOUNDSPEEDREF", 13) == 0) {
				sscanf(pargv[i], "SOUNDSPEEDREF:%d", &process.mbp_corrected);
			}

			/* static beam bathymetry correction */
			else if (strncmp(pargv[i], "STATICMODE", 10) == 0) {
				sscanf(pargv[i], "STATICMODE:%d", &process.mbp_static_mode);
				if (explicit == MB_NO && process.mbp_static_mode == MBP_STATIC_OFF) {
					process.mbp_staticfile[0] = '\0';
				}
			}
			else if (strncmp(pargv[i], "STATICFILE", 10) == 0) {
				sscanf(pargv[i], "STATICFILE:%s", process.mbp_staticfile);
				if (explicit == MB_NO) {
					process.mbp_static_mode = MBP_SVP_ON;
				}
			}

			/* draft correction */
			else if (strncmp(pargv[i], "DRAFTMODE", 9) == 0) {
				sscanf(pargv[i], "DRAFTMODE:%d", &process.mbp_draft_mode);
			}
			else if (strncmp(pargv[i], "DRAFTOFFSET", 11) == 0) {
				sscanf(pargv[i], "DRAFTOFFSET:%lf", &process.mbp_draft_offset);
				if (explicit == MB_NO && process.mbp_draft_mode == MBP_DRAFT_MULTIPLY) {
					process.mbp_draft_mode = MBP_DRAFT_MULTIPLYOFFSET;
				}
				else if (explicit == MB_NO && process.mbp_draft_mode == MBP_DRAFT_OFF) {
					process.mbp_draft_mode = MBP_DRAFT_OFFSET;
				}
			}
			else if (strncmp(pargv[i], "DRAFTMULTIPLY", 13) == 0) {
				sscanf(pargv[i], "DRAFTMULTIPLY:%lf", &process.mbp_draft_mult);
				if (explicit == MB_NO && process.mbp_draft_mode == MBP_DRAFT_OFFSET) {
					process.mbp_draft_mode = MBP_DRAFT_MULTIPLYOFFSET;
				}
				else if (explicit == MB_NO && process.mbp_draft_mode == MBP_DRAFT_OFF) {
					process.mbp_draft_mode = MBP_DRAFT_MULTIPLY;
				}
			}
			else if (strncmp(pargv[i], "DRAFT", 5) == 0) {
				sscanf(pargv[i], "DRAFT:%lf", &process.mbp_draft);
				if (explicit == MB_NO) {
					process.mbp_draft_mode = MBP_DRAFT_SET;
				}
			}

			/* heave correction */
			else if (strncmp(pargv[i], "HEAVEMODE", 9) == 0) {
				sscanf(pargv[i], "HEAVEMODE:%d", &process.mbp_heave_mode);
			}
			else if (strncmp(pargv[i], "HEAVEOFFSET", 11) == 0) {
				sscanf(pargv[i], "HEAVEOFFSET:%lf", &process.mbp_heave);
				if (explicit == MB_NO && process.mbp_heave_mode == MBP_HEAVE_MULTIPLY) {
					process.mbp_heave_mode = MBP_HEAVE_MULTIPLYOFFSET;
				}
				else if (explicit == MB_NO && process.mbp_heave_mode == MBP_HEAVE_OFF) {
					process.mbp_heave_mode = MBP_HEAVE_OFFSET;
				}
			}
			else if (strncmp(pargv[i], "HEAVEMULTIPLY", 13) == 0) {
				sscanf(pargv[i], "HEAVEMULTIPLY:%lf", &process.mbp_heave_mult);
				if (explicit == MB_NO && process.mbp_heave_mode == MBP_HEAVE_OFFSET) {
					process.mbp_heave_mode = MBP_HEAVE_MULTIPLYOFFSET;
				}
				else if (explicit == MB_NO && process.mbp_heave_mode == MBP_HEADING_OFF) {
					process.mbp_heave_mode = MBP_HEAVE_MULTIPLY;
				}
			}

			/* lever correction */
			else if (strncmp(pargv[i], "LEVERMODE", 9) == 0) {
				sscanf(pargv[i], "LEVERMODE:%d", &process.mbp_lever_mode);
			}
			else if (strncmp(pargv[i], "VRUOFFSETX", 10) == 0) {
				sscanf(pargv[i], "VRUOFFSETX:%lf", &process.mbp_vru_offsetx);
				if (explicit == MB_NO) {
					process.mbp_lever_mode = MBP_LEVER_ON;
				}
			}
			else if (strncmp(pargv[i], "VRUOFFSETY", 10) == 0) {
				sscanf(pargv[i], "VRUOFFSETY:%lf", &process.mbp_vru_offsety);
				if (explicit == MB_NO) {
					process.mbp_lever_mode = MBP_LEVER_ON;
				}
			}
			else if (strncmp(pargv[i], "VRUOFFSETZ", 10) == 0) {
				sscanf(pargv[i], "VRUOFFSETZ:%lf", &process.mbp_vru_offsetz);
				if (explicit == MB_NO) {
					process.mbp_lever_mode = MBP_LEVER_ON;
				}
			}
			else if (strncmp(pargv[i], "SONAROFFSETX", 12) == 0) {
				sscanf(pargv[i], "SONAROFFSETX:%lf", &process.mbp_sonar_offsetx);
				if (explicit == MB_NO) {
					process.mbp_lever_mode = MBP_LEVER_ON;
				}
			}
			else if (strncmp(pargv[i], "SONAROFFSETY", 12) == 0) {
				sscanf(pargv[i], "SONAROFFSETY:%lf", &process.mbp_sonar_offsety);
				if (explicit == MB_NO) {
					process.mbp_lever_mode = MBP_LEVER_ON;
				}
			}
			else if (strncmp(pargv[i], "SONAROFFSETZ", 12) == 0) {
				sscanf(pargv[i], "SONAROFFSETZ:%lf", &process.mbp_sonar_offsetz);
				if (explicit == MB_NO) {
					process.mbp_lever_mode = MBP_LEVER_ON;
				}
			}

			/* roll correction */
			else if (strncmp(pargv[i], "ROLLBIASMODE", 12) == 0) {
				sscanf(pargv[i], "ROLLBIASMODE:%d", &process.mbp_rollbias_mode);
			}
			else if (strncmp(pargv[i], "ROLLBIASPORT", 12) == 0) {
				sscanf(pargv[i], "ROLLBIASPORT:%lf", &process.mbp_rollbias_port);
				if (explicit == MB_NO) {
					process.mbp_rollbias_mode = MBP_ROLLBIAS_DOUBLE;
				}
			}
			else if (strncmp(pargv[i], "ROLLBIASSTBD", 12) == 0) {
				sscanf(pargv[i], "ROLLBIASSTBD:%lf", &process.mbp_rollbias_stbd);
				if (explicit == MB_NO) {
					process.mbp_rollbias_mode = MBP_ROLLBIAS_DOUBLE;
				}
			}
			else if (strncmp(pargv[i], "ROLLBIAS", 8) == 0) {
				sscanf(pargv[i], "ROLLBIAS:%lf", &process.mbp_rollbias);
				if (explicit == MB_NO) {
					process.mbp_rollbias_mode = MBP_ROLLBIAS_SINGLE;
				}
			}

			/* pitch correction */
			else if (strncmp(pargv[i], "PITCHBIASMODE", 13) == 0) {
				sscanf(pargv[i], "PITCHBIASMODE:%d", &process.mbp_pitchbias_mode);
			}
			else if (strncmp(pargv[i], "PITCHBIAS", 9) == 0) {
				sscanf(pargv[i], "PITCHBIAS:%lf", &process.mbp_pitchbias);
				if (explicit == MB_NO) {
					process.mbp_pitchbias_mode = MBP_PITCHBIAS_ON;
				}
			}

			/* heading correction */
			else if (strncmp(pargv[i], "HEADINGMODE", 11) == 0) {
				sscanf(pargv[i], "HEADINGMODE:%d", &process.mbp_heading_mode);
			}
			else if (strncmp(pargv[i], "HEADINGOFFSET", 13) == 0) {
				sscanf(pargv[i], "HEADINGOFFSET:%lf", &process.mbp_headingbias);
				if (explicit == MB_NO && process.mbp_heading_mode == MBP_HEADING_CALC) {
					process.mbp_heading_mode = MBP_HEADING_CALCOFFSET;
				}
				else if (explicit == MB_NO && process.mbp_heading_mode == MBP_HEADING_OFF) {
					process.mbp_heading_mode = MBP_HEADING_OFFSET;
				}
			}

			/* tide correction */
			else if (strncmp(pargv[i], "TIDEMODE", 8) == 0) {
				sscanf(pargv[i], "TIDEMODE:%d", &process.mbp_tide_mode);
				if (explicit == MB_NO && process.mbp_tide_mode == MBP_TIDE_OFF) {
					process.mbp_tidefile[0] = '\0';
				}
			}
			else if (strncmp(pargv[i], "TIDEFILE", 8) == 0) {
				sscanf(pargv[i], "TIDEFILE:%s", process.mbp_tidefile);
				if (explicit == MB_NO) {
					process.mbp_tide_mode = MBP_TIDE_ON;
				}
			}
			else if (strncmp(pargv[i], "TIDEFORMAT", 10) == 0) {
				sscanf(pargv[i], "TIDEFORMAT:%d", &process.mbp_tide_format);
			}

			/* amplitude correction */
			else if (strncmp(pargv[i], "AMPCORRMODE", 11) == 0) {
				sscanf(pargv[i], "AMPCORRMODE:%d", &process.mbp_ampcorr_mode);
				if (explicit == MB_NO && process.mbp_ampcorr_mode == MBP_AMPCORR_OFF) {
					process.mbp_ampcorrfile[0] = '\0';
				}
			}
			else if (strncmp(pargv[i], "AMPCORRFILE", 11) == 0) {
				sscanf(pargv[i], "AMPCORRFILE:%s", process.mbp_ampcorrfile);
				if (explicit == MB_NO) {
					process.mbp_ampcorr_mode = MBP_AMPCORR_ON;
				}
			}
			else if (strncmp(pargv[i], "AMPCORRTYPE", 11) == 0) {
				sscanf(pargv[i], "AMPCORRTYPE:%d", &process.mbp_ampcorr_type);
			}
			else if (strncmp(pargv[i], "AMPCORRSYMMETRY", 15) == 0) {
				sscanf(pargv[i], "AMPCORRSYMMETRY:%d", &process.mbp_ampcorr_symmetry);
			}
			else if (strncmp(pargv[i], "AMPCORRANGLE", 12) == 0) {
				sscanf(pargv[i], "AMPCORRANGLE:%lf", &process.mbp_ampcorr_angle);
			}
			else if (strncmp(pargv[i], "AMPCORRSLOPE", 12) == 0) {
				sscanf(pargv[i], "AMPCORRSLOPE:%d", &process.mbp_ampcorr_slope);
			}
			else if (strncmp(pargv[i], "AMPSSCORRTOPOFILE", 17) == 0) {
				sscanf(pargv[i], "AMPSSCORRTOPOFILE:%s", process.mbp_ampsscorr_topofile);
			}

			/* sidescan correction */
			else if (strncmp(pargv[i], "SSCORRMODE", 10) == 0) {
				sscanf(pargv[i], "SSCORRMODE:%d", &process.mbp_sscorr_mode);
				if (explicit == MB_NO && process.mbp_sscorr_mode == MBP_SSCORR_OFF) {
					process.mbp_sscorrfile[0] = '\0';
				}
			}
			else if (strncmp(pargv[i], "SSCORRFILE", 10) == 0) {
				sscanf(pargv[i], "SSCORRFILE:%s", process.mbp_sscorrfile);
				if (explicit == MB_NO) {
					process.mbp_sscorr_mode = MBP_SSCORR_ON;
				}
			}
			else if (strncmp(pargv[i], "SSCORRTYPE", 10) == 0) {
				sscanf(pargv[i], "SSCORRTYPE:%d", &process.mbp_sscorr_type);
			}
			else if (strncmp(pargv[i], "SSCORRSYMMETRY", 14) == 0) {
				sscanf(pargv[i], "SSCORRSYMMETRY:%d", &process.mbp_sscorr_symmetry);
			}
			else if (strncmp(pargv[i], "SSCORRANGLE", 11) == 0) {
				sscanf(pargv[i], "SSCORRANGLE:%lf", &process.mbp_sscorr_angle);
			}
			else if (strncmp(pargv[i], "SSCORRSLOPE", 11) == 0) {
				sscanf(pargv[i], "SSCORRSLOPE:%d", &process.mbp_sscorr_slope);
			}
			else if (strncmp(pargv[i], "AMPSSCORRTOPOFILE", 17) == 0) {
				sscanf(pargv[i], "AMPSSCORRTOPOFILE:%s", process.mbp_ampsscorr_topofile);
			}

			/* sidescan recalculation */
			else if (strncmp(pargv[i], "SSRECALCMODE", 12) == 0) {
				sscanf(pargv[i], "SSRECALCMODE:%d", &process.mbp_ssrecalc_mode);
			}
			else if (strncmp(pargv[i], "SSPIXELSIZE", 11) == 0) {
				sscanf(pargv[i], "SSPIXELSIZE:%lf", &process.mbp_ssrecalc_pixelsize);
			}
			else if (strncmp(pargv[i], "SSSWATHWIDTH", 11) == 0) {
				sscanf(pargv[i], "SSSWATHWIDTH:%lf", &process.mbp_ssrecalc_swathwidth);
			}
			else if (strncmp(pargv[i], "SSINTERPOLATE", 11) == 0) {
				sscanf(pargv[i], "SSINTERPOLATE:%d", &process.mbp_ssrecalc_interpolate);
			}

			/* metadata insertion */
			else if (strncmp(pargv[i], "METAVESSEL:", 11) == 0) {
				strcpy(process.mbp_meta_vessel, &(pargv[i][11]));
			}
			else if (strncmp(pargv[i], "METAINSTITUTION:", 16) == 0) {
				strcpy(process.mbp_meta_institution, &(pargv[i][16]));
			}
			else if (strncmp(pargv[i], "METAPLATFORM:", 13) == 0) {
				strcpy(process.mbp_meta_platform, &(pargv[i][13]));
			}
			else if (strncmp(pargv[i], "METASONARVERSION:", 17) == 0) {
				strcpy(process.mbp_meta_sonarversion, &(pargv[i][17]));
			}
			else if (strncmp(pargv[i], "METASONAR:", 10) == 0) {
				strcpy(process.mbp_meta_sonar, &(pargv[i][10]));
			}
			else if (strncmp(pargv[i], "METACRUISEID:", 13) == 0) {
				strcpy(process.mbp_meta_cruiseid, &(pargv[i][13]));
			}
			else if (strncmp(pargv[i], "METACRUISENAME:", 15) == 0) {
				strcpy(process.mbp_meta_cruisename, &(pargv[i][15]));
			}
			else if (strncmp(pargv[i], "METAPIINSTITUTION:", 18) == 0) {
				strcpy(process.mbp_meta_piinstitution, &(pargv[i][18]));
			}
			else if (strncmp(pargv[i], "METACLIENT:", 11) == 0) {
				strcpy(process.mbp_meta_client, &(pargv[i][11]));
			}
			else if (strncmp(pargv[i], "METASVCORRECTED:", 16) == 0) {
				sscanf(pargv[i], "METASVCORRECTED:%d", &(process.mbp_meta_svcorrected));
			}
			else if (strncmp(pargv[i], "METATIDECORRECTED:", 18) == 0) {
				sscanf(pargv[i], "METATIDECORRECTED:%d", &(process.mbp_meta_tidecorrected));
			}
			else if (strncmp(pargv[i], "METABATHEDITMANUAL:", 19) == 0) {
				sscanf(pargv[i], "METABATHEDITMANUAL:%d", &(process.mbp_meta_batheditmanual));
			}
			else if (strncmp(pargv[i], "METABATHEDITAUTO:", 17) == 0) {
				sscanf(pargv[i], "METABATHEDITAUTO:%d", &(process.mbp_meta_batheditauto));
			}
			else if (strncmp(pargv[i], "METAROLLBIAS:", 13) == 0) {
				sscanf(pargv[i], "METAROLLBIAS:%lf", &(process.mbp_meta_rollbias));
			}
			else if (strncmp(pargv[i], "METAPITCHBIAS:", 14) == 0) {
				sscanf(pargv[i], "METAPITCHBIAS:%lf", &(process.mbp_meta_pitchbias));
			}
			else if (strncmp(pargv[i], "METAPI:", 7) == 0) {
				strcpy(process.mbp_meta_pi, &(pargv[i][7]));
			}
			else if (strncmp(pargv[i], "METAHEADINGBIAS:", 16) == 0) {
				sscanf(pargv[i], "METAHEADINGBIAS:%lf", &(process.mbp_meta_headingbias));
			}
			else if (strncmp(pargv[i], "METADRAFT:", 10) == 0) {
				sscanf(pargv[i], "METADRAFT:%lf", &(process.mbp_meta_draft));
			}

			/* processing kluges */
			else if (strncmp(pargv[i], "KLUGE001:", 8) == 0) {
				sscanf(pargv[i], "KLUGE001:%d", &(process.mbp_kluge001));
			}
			else if (strncmp(pargv[i], "KLUGE002:", 8) == 0) {
				sscanf(pargv[i], "KLUGE002:%d", &(process.mbp_kluge002));
			}
			else if (strncmp(pargv[i], "KLUGE003:", 8) == 0) {
				sscanf(pargv[i], "KLUGE003:%d", &(process.mbp_kluge003));
			}
			else if (strncmp(pargv[i], "KLUGE004:", 8) == 0) {
				sscanf(pargv[i], "KLUGE004:%d", &(process.mbp_kluge004));
			}
			else if (strncmp(pargv[i], "KLUGE005:", 8) == 0) {
				sscanf(pargv[i], "KLUGE005:%d", &(process.mbp_kluge005));
			}
			else if (strncmp(pargv[i], "KLUGE006:", 8) == 0) {
				sscanf(pargv[i], "KLUGE006:%d", &(process.mbp_kluge006));
			}
			else if (strncmp(pargv[i], "KLUGE007:", 8) == 0) {
				sscanf(pargv[i], "KLUGE007:%d", &(process.mbp_kluge007));
			}
			else if (strncmp(pargv[i], "KLUGE008:", 8) == 0) {
				sscanf(pargv[i], "KLUGE008:%d", &(process.mbp_kluge008));
			}
			else if (strncmp(pargv[i], "KLUGE009:", 8) == 0) {
				sscanf(pargv[i], "KLUGE009:%d", &(process.mbp_kluge009));
			}
			else if (strncmp(pargv[i], "KLUGE010:", 8) == 0) {
				sscanf(pargv[i], "KLUGE010:%d", &(process.mbp_kluge010));
			}

			/* unrecognized command */
			else {
				fprintf(stderr, "\nUnrecognized %s command: %s\n", program_name, pargv[i]);
			}
		}

		/* figure out data format or output filename if required */
		if (process.mbp_format_specified == MB_NO || process.mbp_ofile_specified == MB_NO) {
			mb_pr_default_output(verbose, &process, &error);
		}

		/* update bathymetry recalculation mode */
		mb_pr_bathmode(verbose, &process, &error);

		/* if any of the important modes has been turned on, write the parameter file */
		if (process.mbp_nav_mode != MBP_NAV_OFF || process.mbp_navadj_mode != MBP_NAVADJ_OFF || process.mbp_attitude_mode != 0 ||
		    process.mbp_sonardepth_mode != 0 || process.mbp_cut_num != 0 || process.mbp_edit_mode != MBP_EDIT_OFF ||
		    process.mbp_bathrecalc_mode != MBP_BATHRECALC_OFF || process.mbp_draft_mode != MBP_DRAFT_OFF ||
		    process.mbp_heave_mode != MBP_HEAVE_OFF || process.mbp_lever_mode != MBP_LEVER_OFF ||
		    process.mbp_rollbias_mode != MBP_ROLLBIAS_OFF || process.mbp_pitchbias_mode != MBP_PITCHBIAS_OFF ||
		    process.mbp_heading_mode != MBP_HEADING_OFF || process.mbp_tide_mode != MBP_TIDE_OFF ||
		    process.mbp_ampcorr_mode != MBP_AMPCORR_OFF || process.mbp_sscorr_mode != MBP_SSCORR_OFF ||
		    process.mbp_ssrecalc_mode != MBP_SSRECALC_OFF || process.mbp_meta_vessel[0] != '\0' ||
		    process.mbp_meta_institution[0] != '\0' || process.mbp_meta_platform[0] != '\0' ||
		    process.mbp_meta_sonar[0] != '\0' || process.mbp_meta_sonarversion[0] != '\0' ||
		    process.mbp_meta_cruiseid[0] != '\0' || process.mbp_meta_cruisename[0] != '\0' || process.mbp_meta_pi[0] != '\0' ||
		    process.mbp_meta_piinstitution[0] != '\0' || process.mbp_meta_client[0] != '\0' ||
		    process.mbp_meta_svcorrected != MBP_CORRECTION_UNKNOWN || process.mbp_meta_tidecorrected != MBP_CORRECTION_UNKNOWN ||
		    process.mbp_meta_batheditmanual != MBP_CORRECTION_UNKNOWN ||
		    process.mbp_meta_batheditauto != MBP_CORRECTION_UNKNOWN || process.mbp_meta_rollbias != MBP_METANOVALUE + 1. ||
		    process.mbp_meta_pitchbias != MBP_METANOVALUE + 1. || process.mbp_meta_headingbias != MBP_METANOVALUE + 1. ||
		    process.mbp_meta_draft != MBP_METANOVALUE + 1. || process.mbp_kluge001 != MB_NO || process.mbp_kluge002 != MB_NO ||
		    process.mbp_kluge003 != MB_NO || process.mbp_kluge004 != MB_NO || process.mbp_kluge005 != MB_NO ||
		    process.mbp_kluge006 != MB_NO || process.mbp_kluge007 != MB_NO || process.mbp_kluge008 != MB_NO ||
		    process.mbp_kluge009 != MB_NO || process.mbp_kluge010 != MB_NO) {
			write_parameter_file = MB_YES;
		}

		if (verbose >= 2) {
			fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
			fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
			fprintf(stderr, "\ndbg2  MB-System Control Parameters:\n");
			fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
			fprintf(stderr, "dbg2       help:            %d\n", help);
			fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		}

		if (verbose == 1) {
			fprintf(stderr, "\nProgram <%s>\n", program_name);
			fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
			fprintf(stderr, "\nOutput MBprocess Parameters:\n");
			fprintf(stderr, "\nInput and Output Files:\n");
			if (process.mbp_format_specified == MB_YES)
				fprintf(stderr, "  Format:                        %d\n", process.mbp_format);
			if (process.mbp_ifile_specified == MB_YES)
				fprintf(stderr, "  Input file:                    %s\n", process.mbp_ifile);
			if (process.mbp_ofile_specified == MB_YES)
				fprintf(stderr, "  Output file:                   %s\n", process.mbp_ofile);

			fprintf(stderr, "\nNavigation Merging:\n");
			if (process.mbp_nav_mode == MBP_NAV_ON) {
				fprintf(stderr, "  Navigation merged from navigation file.\n");
				fprintf(stderr, "  Navigation file:               %s\n", process.mbp_navfile);
				fprintf(stderr, "  Navigation format:             %d\n", process.mbp_nav_format);
				if (process.mbp_nav_heading == MBP_NAV_ON)
					fprintf(stderr, "  Heading merged from navigation file.\n");
				else
					fprintf(stderr, "  Heading not merged from navigation file.\n");
				if (process.mbp_nav_speed == MBP_NAV_ON)
					fprintf(stderr, "  Speed merged from navigation file.\n");
				else
					fprintf(stderr, "  Speed not merged from navigation file.\n");
				if (process.mbp_nav_draft == MBP_NAV_ON)
					fprintf(stderr, "  Draft merged from navigation file.\n");
				else
					fprintf(stderr, "  Draft not merged from navigation file.\n");
				if (process.mbp_nav_attitude == MBP_NAV_ON)
					fprintf(stderr, "  Roll, pitch, and heave merged from navigation file.\n");
				else
					fprintf(stderr, "  Roll, pitch, and heave not merged from navigation file.\n");
				if (process.mbp_nav_algorithm == MBP_NAV_LINEAR)
					fprintf(stderr, "  Navigation algorithm:          linear interpolation\n");
				else if (process.mbp_nav_algorithm == MBP_NAV_SPLINE)
					fprintf(stderr, "  Navigation algorithm:          spline interpolation\n");
				fprintf(stderr, "  Navigation time shift:         %f\n", process.mbp_nav_timeshift);
			}
			else
				fprintf(stderr, "  Navigation not merged from navigation file.\n");

			fprintf(stderr, "\nNavigation Offsets and Shifts:\n");
			if (process.mbp_nav_shift == MBP_NAV_ON) {
				fprintf(stderr, "  Navigation positions shifted.\n");
				fprintf(stderr, "  Navigation offset x:                  %f\n", process.mbp_nav_offsetx);
				fprintf(stderr, "  Navigation offset y:                  %f\n", process.mbp_nav_offsety);
				fprintf(stderr, "  Navigation offset z:                  %f\n", process.mbp_nav_offsetz);
				fprintf(stderr, "  Navigation longitude shift (degrees): %f\n", process.mbp_nav_shiftlon);
				fprintf(stderr, "  Navigation latitude shift (degrees):  %f\n", process.mbp_nav_shiftlat);
				fprintf(stderr, "  Navigation longitude shift (meters):  %f\n", process.mbp_nav_shiftx);
				fprintf(stderr, "  Navigation latitude shift (meters):   %f\n", process.mbp_nav_shifty);
			}
			else
				fprintf(stderr, "  Navigation positions not shifted.\n");

			fprintf(stderr, "\nAdjusted Navigation Merging:\n");
			if (process.mbp_navadj_mode == MBP_NAVADJ_LLZ) {
				fprintf(stderr, "  Navigation merged from adjusted navigation file.\n");
				fprintf(stderr, "  Adjusted navigation file:      %s\n", process.mbp_navadjfile);
				if (process.mbp_navadj_algorithm == MBP_NAV_LINEAR)
					fprintf(stderr, "  Adjusted navigation algorithm: linear interpolation\n");
				else if (process.mbp_navadj_algorithm == MBP_NAV_SPLINE)
					fprintf(stderr, "  Adjusted navigation algorithm: spline interpolation\n");
			}
			else
				fprintf(stderr, "  Navigation not merged from adjusted navigation file.\n");

			fprintf(stderr, "\nAttitude Merging:\n");
			if (process.mbp_attitude_mode == MBP_NAV_ON) {
				fprintf(stderr, "  Attitude merged from attitude file.\n");
				fprintf(stderr, "  Attitude file:                 %s\n", process.mbp_attitudefile);
				fprintf(stderr, "  Attitude format:               %d\n", process.mbp_attitude_format);
			}
			else
				fprintf(stderr, "  Attitude not merged from attitude file.\n");

			fprintf(stderr, "\nData Cutting:\n");
			if (process.mbp_cut_num > 0)
				fprintf(stderr, "  Data cutting enabled (%d commands).\n", process.mbp_cut_num);
			else
				fprintf(stderr, "  Data cutting disabled.\n");
			for (i = 0; i < process.mbp_cut_num; i++) {
				if (process.mbp_cut_kind[i] == MBP_CUT_DATA_BATH)
					fprintf(stderr, "  Cut[%d]: bathymetry", i);
				else if (process.mbp_cut_kind[i] == MBP_CUT_DATA_AMP)
					fprintf(stderr, "  Cut[%d]: amplitude ", i);
				else if (process.mbp_cut_kind[i] == MBP_CUT_DATA_SS)
					fprintf(stderr, "  Cut[%d]: sidescan  ", i);
				if (process.mbp_cut_mode[i] == MBP_CUT_MODE_NUMBER)
					fprintf(stderr, "  number   ");
				else if (process.mbp_cut_kind[i] == MBP_CUT_MODE_DISTANCE)
					fprintf(stderr, "  distance ");
				fprintf(stderr, "  %f %f\n", process.mbp_cut_min[i], process.mbp_cut_max[i]);
			}

			fprintf(stderr, "\nBathymetry Editing:\n");
			if (process.mbp_edit_mode == MBP_EDIT_ON)
				fprintf(stderr, "  Bathymetry edits applied from file.\n");
			else
				fprintf(stderr, "  Bathymetry edits not applied from file.\n");
			fprintf(stderr, "  Bathymetry edit file:          %s\n", process.mbp_editfile);

			fprintf(stderr, "\nBathymetry Recalculation:\n");
			if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_OFF)
				fprintf(stderr, "  Bathymetry not recalculated.\n");
			else if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_RAYTRACE)
				fprintf(stderr, "  Bathymetry recalculated by raytracing.\n");
			else if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_ROTATE)
				fprintf(stderr, "  Bathymetry recalculated by rigid rotation.\n");
			else if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_OFFSET)
				fprintf(stderr, "  Bathymetry recalculated by transducer depth shift.\n");
			fprintf(stderr, "  SVP file:                      %s\n", process.mbp_svpfile);
			if (process.mbp_ssv_mode == MBP_SSV_OFF)
				fprintf(stderr, "  SSV not modified.\n");
			else if (process.mbp_ssv_mode == MBP_SSV_OFFSET)
				fprintf(stderr, "  SSV offset by constant.\n");
			else
				fprintf(stderr, "  SSV set to constant.\n");
			fprintf(stderr, "  SSV offset/constant:           %f m/s\n", process.mbp_ssv);
			fprintf(stderr, "  Travel time mode:              %d\n", process.mbp_tt_mode);
			fprintf(stderr, "  Travel time multiplier:        %f\n", process.mbp_tt_mult);
			fprintf(stderr, "  Raytrace angle mode:           %d\n", process.mbp_angle_mode);

			fprintf(stderr, "\nBathymetry Water Sound Speed Reference:\n");
			if (process.mbp_corrected == MB_YES)
				fprintf(stderr, "  Output bathymetry reference:   CORRECTED\n");
			else if (process.mbp_corrected == MB_NO)
				fprintf(stderr, "  Bathymetry reference:          UNCORRECTED\n");
			if (process.mbp_svp_mode == MBP_SVP_SOUNDSPEEDREF) {
				if (process.mbp_corrected == MB_YES)
					fprintf(stderr, "  Depths modified from uncorrected to corrected\n");
				else
					fprintf(stderr, "  Depths modified from corrected to uncorrected\n");
			}
			else if (process.mbp_svp_mode == MBP_SVP_ON) {
				if (process.mbp_corrected == MB_YES)
					fprintf(stderr, "  Depths recalculated as corrected\n");
				else
					fprintf(stderr, "  Depths recalculated as uncorrected\n");
			}
			else {
				fprintf(stderr, "  Depths unmodified with respect to water sound speed reference\n");
			}

			fprintf(stderr, "\nDraft Correction:\n");
			if (process.mbp_draft_mode == MBP_DRAFT_OFF)
				fprintf(stderr, "  Draft not modified.\n");
			else if (process.mbp_draft_mode == MBP_DRAFT_SET)
				fprintf(stderr, "  Draft set to constant.\n");
			else if (process.mbp_draft_mode == MBP_DRAFT_OFFSET)
				fprintf(stderr, "  Draft offset by constant.\n");
			else if (process.mbp_draft_mode == MBP_DRAFT_MULTIPLY)
				fprintf(stderr, "  Draft multiplied by constant.\n");
			else if (process.mbp_draft_mode == MBP_DRAFT_MULTIPLYOFFSET)
				fprintf(stderr, "  Draft multiplied and offset by constants.\n");
			fprintf(stderr, "  Draft constant:                %f m\n", process.mbp_draft);
			fprintf(stderr, "  Draft offset:                  %f m\n", process.mbp_draft_offset);
			fprintf(stderr, "  Draft multiplier:              %f m\n", process.mbp_draft_mult);

			fprintf(stderr, "\nHeave Correction:\n");
			if (process.mbp_heave_mode == MBP_HEAVE_OFF)
				fprintf(stderr, "  Heave not modified.\n");
			else if (process.mbp_heave_mode == MBP_HEAVE_OFFSET)
				fprintf(stderr, "  Heave offset by constant.\n");
			else if (process.mbp_heave_mode == MBP_HEAVE_MULTIPLY)
				fprintf(stderr, "  Heave multiplied by constant.\n");
			else if (process.mbp_heave_mode == MBP_HEAVE_MULTIPLYOFFSET)
				fprintf(stderr, "  Heave multiplied and offset by constants.\n");
			fprintf(stderr, "  Heave offset:                  %f m\n", process.mbp_heave);
			fprintf(stderr, "  Heave multiplier:              %f m\n", process.mbp_heave_mult);

			fprintf(stderr, "\nLever Correction:\n");
			if (process.mbp_lever_mode == MBP_LEVER_OFF)
				fprintf(stderr, "  Lever calculation off.\n");
			else {
				fprintf(stderr, "  Lever calculation used to calculate heave correction.\n");
				fprintf(stderr, "  Heave offset:                  %f m\n", process.mbp_heave);
				fprintf(stderr, "  VRU offset x:                  %f m\n", process.mbp_vru_offsetx);
				fprintf(stderr, "  VRU offset y:                  %f m\n", process.mbp_vru_offsety);
				fprintf(stderr, "  VRU offset z:                  %f m\n", process.mbp_vru_offsetz);
				fprintf(stderr, "  Sonar offset x:                %f m\n", process.mbp_sonar_offsetx);
				fprintf(stderr, "  Sonar offset y:                %f m\n", process.mbp_sonar_offsety);
				fprintf(stderr, "  Sonar offset z:                %f m\n", process.mbp_sonar_offsetz);
			}

			fprintf(stderr, "\nTide Correction:\n");
			if (process.mbp_tide_mode == MBP_TIDE_OFF)
				fprintf(stderr, "  Tide calculation off.\n");
			else {
				fprintf(stderr, "  Tide correction applied to bathymetry.\n");
				fprintf(stderr, "  Tide file:                     %s\n", process.mbp_tidefile);
				fprintf(stderr, "  Tide format:                   %d\n", process.mbp_tide_format);
			}

			fprintf(stderr, "\nRoll Correction:\n");
			if (process.mbp_rollbias_mode == MBP_ROLLBIAS_OFF)
				fprintf(stderr, "  Roll not modified.\n");
			else if (process.mbp_rollbias_mode == MBP_ROLLBIAS_SINGLE)
				fprintf(stderr, "  Roll offset by bias.\n");
			else if (process.mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE)
				fprintf(stderr, "  Roll offset by separate port and starboard biases.\n");
			fprintf(stderr, "  Roll bias:                     %f deg\n", process.mbp_rollbias);
			fprintf(stderr, "  Port roll bias:                %f deg\n", process.mbp_rollbias_port);
			fprintf(stderr, "  Starboard roll bias:           %f deg\n", process.mbp_rollbias_stbd);

			fprintf(stderr, "\nPitch Correction:\n");
			if (process.mbp_pitchbias_mode == MBP_PITCHBIAS_OFF)
				fprintf(stderr, "  Pitch not modified.\n");
			else
				fprintf(stderr, "  Pitch offset by bias.\n");
			fprintf(stderr, "  Pitch bias:                    %f deg\n", process.mbp_pitchbias);

			fprintf(stderr, "\nHeading Correction:\n");
			if (process.mbp_heading_mode == MBP_HEADING_OFF)
				fprintf(stderr, "  Heading not modified.\n");
			else if (process.mbp_heading_mode == MBP_HEADING_CALC)
				fprintf(stderr, "  Heading replaced by course-made-good.\n");
			else if (process.mbp_heading_mode == MBP_HEADING_OFFSET)
				fprintf(stderr, "  Heading offset by bias.\n");
			else if (process.mbp_heading_mode == MBP_HEADING_CALCOFFSET)
				fprintf(stderr, "  Heading replaced by course-made-good and then offset by bias.\n");
			fprintf(stderr, "  Heading offset:                %f deg\n", process.mbp_headingbias);

			fprintf(stderr, "\nAmplitude Corrections:\n");
			if (process.mbp_ampcorr_mode == MBP_SSCORR_ON) {
				fprintf(stderr, "  Amplitude vs grazing angle corrections applied to amplitudes.\n");
				fprintf(stderr, "  Amplitude correction file:      %s m\n", process.mbp_ampcorrfile);
				if (process.mbp_ampcorr_type == MBP_AMPCORR_SUBTRACTION)
					fprintf(stderr, "  Amplitude correction by subtraction (dB scale)\n");
				else
					fprintf(stderr, "  Amplitude correction by division (linear scale)\n");
				if (process.mbp_ampcorr_symmetry == MBP_AMPCORR_SYMMETRIC)
					fprintf(stderr, "  AVGA tables forced to be symmetric\n");
				else
					fprintf(stderr, "  AVGA tables allowed to be asymmetric\n");
				fprintf(stderr, "  Reference grazing angle:       %f deg\n", process.mbp_ampcorr_angle);
				if (process.mbp_ampcorr_slope == MBP_AMPCORR_IGNORESLOPE)
					fprintf(stderr, "  Amplitude correction ignores seafloor slope\n");
				else if (process.mbp_ampcorr_slope == MBP_AMPCORR_USESLOPE)
					fprintf(stderr, "  Amplitude correction uses seafloor slope\n");
				else {
					fprintf(stderr, "  Amplitude correction uses topography grid for slope\n");
					fprintf(stderr, "  Topography grid file:      %s m\n", process.mbp_ampsscorr_topofile);
				}
			}
			else
				fprintf(stderr, "  Amplitude correction off.\n");

			fprintf(stderr, "\nSidescan Corrections:\n");
			if (process.mbp_sscorr_mode == MBP_SSCORR_ON) {
				fprintf(stderr, "  Amplitude vs grazing angle corrections applied to sidescan.\n");
				fprintf(stderr, "  Sidescan correction file:      %s m\n", process.mbp_sscorrfile);
				if (process.mbp_sscorr_type == MBP_SSCORR_SUBTRACTION)
					fprintf(stderr, "  Sidescan correction by subtraction (dB scale)\n");
				else
					fprintf(stderr, "  Sidescan correction by division (linear scale)\n");
				if (process.mbp_sscorr_symmetry == MBP_SSCORR_SYMMETRIC)
					fprintf(stderr, "  AVGA tables forced to be symmetric\n");
				else
					fprintf(stderr, "  AVGA tables allowed to be asymmetric\n");
				fprintf(stderr, "  Reference grazing angle:       %f deg\n", process.mbp_sscorr_angle);
				if (process.mbp_sscorr_slope == MBP_SSCORR_IGNORESLOPE)
					fprintf(stderr, "  Sidescan correction ignores seafloor slope\n");
				else if (process.mbp_ampcorr_slope == MBP_SSCORR_USESLOPE)
					fprintf(stderr, "  Sidescan correction uses seafloor slope\n");
				else {
					fprintf(stderr, "  Sidescan correction uses topography grid for slope\n");
					fprintf(stderr, "  Topography grid file:      %s m\n", process.mbp_ampsscorr_topofile);
				}
			}
			else
				fprintf(stderr, "  Sidescan correction off.\n");

			fprintf(stderr, "\nSidescan Recalculation:\n");
			if (process.mbp_ssrecalc_mode == MBP_SSRECALC_ON)
				fprintf(stderr, "  Sidescan recalculated.\n");
			else
				fprintf(stderr, "  Sidescan not recalculated.\n");
			fprintf(stderr, "  Sidescan pixel size:           %f\n", process.mbp_ssrecalc_pixelsize);
			fprintf(stderr, "  Sidescan swath width:          %f\n", process.mbp_ssrecalc_swathwidth);
			fprintf(stderr, "  Sidescan interpolation:        %d\n", process.mbp_ssrecalc_interpolate);

			fprintf(stderr, "\nMetadata Insertion:\n");
			fprintf(stderr, "  Metadata vessel:               %s\n", process.mbp_meta_vessel);
			fprintf(stderr, "  Metadata institution:          %s\n", process.mbp_meta_institution);
			fprintf(stderr, "  Metadata platform:             %s\n", process.mbp_meta_platform);
			fprintf(stderr, "  Metadata sonar:                %s\n", process.mbp_meta_sonar);
			fprintf(stderr, "  Metadata sonarversion:         %s\n", process.mbp_meta_sonarversion);
			fprintf(stderr, "  Metadata cruiseid:             %s\n", process.mbp_meta_cruiseid);
			fprintf(stderr, "  Metadata cruisename:           %s\n", process.mbp_meta_cruisename);
			fprintf(stderr, "  Metadata pi:                   %s\n", process.mbp_meta_pi);
			fprintf(stderr, "  Metadata piinstitution:        %s\n", process.mbp_meta_piinstitution);
			fprintf(stderr, "  Metadata client:               %s\n", process.mbp_meta_client);
			fprintf(stderr, "  Metadata svcorrected:          %d\n", process.mbp_meta_svcorrected);
			fprintf(stderr, "  Metadata tidecorrected         %d\n", process.mbp_meta_tidecorrected);
			fprintf(stderr, "  Metadata batheditmanual        %d\n", process.mbp_meta_batheditmanual);
			fprintf(stderr, "  Metadata batheditauto:         %d\n", process.mbp_meta_batheditauto);
			fprintf(stderr, "  Metadata rollbias:             %f\n", process.mbp_meta_rollbias);
			fprintf(stderr, "  Metadata pitchbias:            %f\n", process.mbp_meta_pitchbias);
			fprintf(stderr, "  Metadata headingbias:          %f\n", process.mbp_meta_headingbias);
			fprintf(stderr, "  Metadata draft:                %f\n", process.mbp_meta_draft);

			fprintf(stderr, "\nProcessing Kluges:\n");
			fprintf(stderr, "  Kluge001:                      %d\n", process.mbp_kluge001);
			fprintf(stderr, "  Kluge002:                      %d\n", process.mbp_kluge002);
			fprintf(stderr, "  Kluge003:                      %d\n", process.mbp_kluge003);
			fprintf(stderr, "  Kluge004:                      %d\n", process.mbp_kluge004);
			fprintf(stderr, "  Kluge005:                      %d\n", process.mbp_kluge005);
			fprintf(stderr, "  Kluge006:                      %d\n", process.mbp_kluge006);
			fprintf(stderr, "  Kluge007:                      %d\n", process.mbp_kluge007);
			fprintf(stderr, "  Kluge008:                      %d\n", process.mbp_kluge008);
			fprintf(stderr, "  Kluge009:                      %d\n", process.mbp_kluge009);
			fprintf(stderr, "  Kluge010:                      %d\n", process.mbp_kluge010);
		}

		/* write parameters */
		if (write_parameter_file == MB_YES)
			status = mb_pr_writepar(verbose, mbp_ifile, &process, &error);

		/* output results */
		if (status == MB_SUCCESS) {
			fprintf(stderr, "Success updating parameter file for %s...\n", mbp_ifile);
		}
		else {
			fprintf(stderr, "Failure to update parameter file for %s!!!\n", mbp_ifile);
		}

		/* figure out whether and what to read next */
		if (read_datalist == MB_YES) {
			if ((status = mb_datalist_read(verbose, datalist, mbp_ifile, mbp_dfile, &format, &file_weight, &error)) == MB_SUCCESS)
				read_data = MB_YES;
			else
				read_data = MB_NO;
		}
		else {
			read_data = MB_NO;
		}

	} /* end loop over datalist */

	if (read_datalist == MB_YES)
		mb_datalist_close(verbose, &datalist, &error);

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose, &error);

	exit(error);
}
/*--------------------------------------------------------------------*/
