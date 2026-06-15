/*--------------------------------------------------------------------
 *    The MB-system:	mbset.c
 *
 *    Copyright (c) 2000-2025 by
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

#define THIS_MODULE_NAME	"mbset"
#define THIS_MODULE_LIB     "mbsystem"
#define THIS_MODULE_PURPOSE	"Set values in an mbprocess parameter file"
#define THIS_MODULE_KEYS	""
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "->V"

#include <sys/types.h>
#include <sys/stat.h>

/* GMT header file */
#include "gmt_dev.h"

/* mbio include files */
#include "mb_format.h"
#include "mb_status.h"
#include "mb_define.h"
#include "mb_process.h"

/* Control structure for mbset */
static struct MBSET_CTRL {
	int     read_datalist;
	void	*datalist;

	struct mbset_E {
		bool active;
	} E;
	struct mbset_F {
		bool active;
		int format;
	} F;
	struct mbset_I {
		bool active;
		char *file;
	} I;
	struct mbset_L {
		bool active;
	} L;
	struct mbset_N {
		bool active;
	} N;
	struct mbset_P {
		bool   active;
		int    pargc;
		char **pargv;
	} P;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {
	struct MBSET_CTRL *Ctrl;

	Ctrl = gmt_M_memory (GMT, NULL, 1, struct MBSET_CTRL);

	Ctrl->I.file = NULL;
	Ctrl->read_datalist = MB_NO;
	Ctrl->datalist = NULL;
	Ctrl->F.format = 0;
	Ctrl->P.pargc = 0;
	Ctrl->P.pargv = NULL;
	Ctrl->E.active = false;
	Ctrl->F.active = false;
	Ctrl->I.active = false;
	Ctrl->L.active = false;
	Ctrl->N.active = false;
	Ctrl->P.active = false;

	return (Ctrl);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct MBSET_CTRL *Ctrl) {
	if (!Ctrl) return;
	if (Ctrl->I.file) free (Ctrl->I.file);
	if (Ctrl->P.pargv) {
		for (int i = 0; i < Ctrl->P.pargc; i++) {
			free(Ctrl->P.pargv[i]);
		}
		free(Ctrl->P.pargv);
	}
	gmt_M_free (GMT, Ctrl);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message(API, GMT_TIME_NONE, "usage: mbset -Iinfile -PPARAMETER:value [-E -L -N -V]\n");
	GMT_Message(API, GMT_TIME_NONE, "MB-System Version %s\n", MB_VERSION);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	return (EXIT_FAILURE);
}

static int parse (struct GMT_CTRL *GMT, struct MBSET_CTRL *Ctrl, struct GMT_OPTION *options) {
	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;
	int i, n;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {
			case '<':
				Ctrl->I.active = true;
				if (gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) {
					Ctrl->I.file = strdup (opt->arg);
					n_files = 1;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: only one input file is allowed.\n");
					n_errors++;
				}
				break;

			case 'E':
				Ctrl->E.active = true;
				break;
			case 'F':
				n = sscanf(opt->arg, "%d", &(Ctrl->F.format));
				if (n == 1)
					Ctrl->F.active = true;
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -F option: \n");
					n_errors++;
				}
				break;
			case 'I':
				Ctrl->I.active = true;
				if (!gmt_access(GMT, opt->arg, R_OK)) {
					Ctrl->I.file = strdup(opt->arg);
					n_files = 1;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -I: Requires a valid file\n");
					n_errors++;
				}
				break;
			case 'L':
				Ctrl->L.active = true;
				break;
			case 'N':
				Ctrl->N.active = true;
				break;
			case 'P':
				Ctrl->P.active = true;
				if (strlen(opt->arg) == 0) {
					GMT_Report (API, GMT_MSG_NORMAL, "Error -p option: Don't invent, number of pings must be >= 0\n");
					n_errors++;
					break;
				}
				/* Replace first '=' before ':' with ':' */
				for (i = 0; i < (int)strlen(opt->arg); i++) {
					if (opt->arg[i] == ':')
						break;
					else if (opt->arg[i] == '=') {
						opt->arg[i] = ':';
						break;
					}
				}
				/* store the parameter argument */
				Ctrl->P.pargv = (char **) realloc(Ctrl->P.pargv, (Ctrl->P.pargc + 1) * sizeof(char *));
				Ctrl->P.pargv[Ctrl->P.pargc] = (char *)malloc(strlen(opt->arg)+1);
				strcpy(Ctrl->P.pargv[Ctrl->P.pargc], opt->arg);
				Ctrl->P.pargc++;
				break;
			default:
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, n_files != 1, "Syntax error: Must specify one input file(s)\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.active && !Ctrl->I.file,
								   "Syntax error -I option: Must specify input file\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_mbset (void *V_API, int mode, void *args) {

	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);
	struct MBSET_CTRL *Ctrl = NULL;

	char **pargv;

	int status = MB_SUCCESS;
	int verbose = 0;
	int error = MB_ERROR_NO_ERROR;

	struct mb_process_struct process;

	int is_explicit = MB_NO;
	int read_data = MB_NO;
	int look_processed = MB_DATALIST_LOOK_NO;
	double file_weight;
	int lookforfiles = MB_NO;
	int removembnavadjust = MB_NO;
	struct stat file_status;
	int fstat;
	char mbp_ifile[MBP_FILENAMESIZE] = {""};
	char mbp_dfile[MBP_FILENAMESIZE] = {""};
	int mbp_format;
	int write_parameter_file = MB_NO;
	int i, nscan;

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);

	if (!options || options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));
	if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));

#if GMT_MAJOR_VERSION >= 6
	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error);
#else
	GMT = gmt_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy);
#endif
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);

	Ctrl = (struct MBSET_CTRL *) New_Ctrl (GMT);
	if ((error = parse (GMT, Ctrl, options))) Return (error);

	verbose = GMT->common.V.active;

	if (Ctrl->E.active) is_explicit = MB_YES;
	if (Ctrl->L.active) lookforfiles++;
	if (Ctrl->N.active) removembnavadjust++;
	pargv = Ctrl->P.pargv;

	if (Ctrl->F.format == 0)
		mb_get_format(verbose, Ctrl->I.file, NULL, &Ctrl->F.format, &error);

	if (Ctrl->F.format < 0)
		Ctrl->read_datalist = MB_YES;

	if (Ctrl->read_datalist == MB_YES) {
		if ((status = mb_datalist_open(verbose, &Ctrl->datalist, Ctrl->I.file, look_processed, &error)) != MB_SUCCESS) {
			error = MB_ERROR_OPEN_FAIL;
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to open data list file: %s\n", Ctrl->I.file);
			GMT_Report (API, GMT_MSG_NORMAL, "Program <%s> Terminated\n", THIS_MODULE_NAME);
			Return(error);
		}
		if ((status = mb_datalist_read(verbose, Ctrl->datalist, mbp_ifile, mbp_dfile, &mbp_format, &file_weight, &error)) == MB_SUCCESS)
			read_data = MB_YES;
		else
			read_data = MB_NO;
	}
	else {
		strcpy(mbp_ifile, Ctrl->I.file);
		mbp_format = Ctrl->F.format;
		read_data = MB_YES;
	}

	while (read_data == MB_YES) {
		struct mb_process_struct process;
		struct mb_process_struct process_org;
		char mbp_pfile[MBP_FILENAMESIZE];
		bool existing_parameter_file = false;

		/* load parameters */
		status = mb_pr_readpar(verbose, mbp_ifile, false, &process_org, &error);
		if (lookforfiles) {
			status = mb_pr_readpar(verbose, mbp_ifile, true, &process, &error);
		} else {
			process = process_org;
		}
		process_org.mbp_ifile_specified = MB_YES;
		process.mbp_ifile_specified = MB_YES;
		write_parameter_file = MB_NO;

		strncpy(mbp_pfile, mbp_ifile, MBP_FILENAMESIZE - 4);
		strcat(mbp_pfile, ".par");
		if (stat(mbp_pfile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
			existing_parameter_file = true;
		}

		if (process.mbp_format_specified == MB_NO) {
			process.mbp_format = mbp_format;
			process.mbp_format_specified = MB_YES;
		}
		if (process.mbp_ofile_specified == MB_NO) {
			process.mbp_ofile_specified = MB_YES;
			mb_pr_default_output(verbose, &process, &error);
		}

		if (removembnavadjust == MB_YES) {
			if (strlen(process.mbp_navadjfile) > 0 && (fstat = stat(process.mbp_navadjfile, &file_status)) == 0
												&& (file_status.st_mode & S_IFMT) != S_IFDIR) {
				remove(process.mbp_navadjfile);
				GMT_Report(API, GMT_MSG_NORMAL, "Removed navigation adjustment file %s for %s\n", process.mbp_navadjfile, mbp_ifile);
			}

			if (strlen(process.mbp_navadjfile) > 0 || process.mbp_navadj_mode != MBP_NAVADJ_OFF) {
				GMT_Report(API, GMT_MSG_NORMAL, "Turned off navigation adjustment for %s\n", mbp_ifile);
				write_parameter_file = MB_YES;
			}

			process.mbp_navadjfile[0] = '\0';
			process.mbp_navadj_mode = MBP_NAVADJ_OFF;
		}

		if (Ctrl->P.pargc > 0)
			write_parameter_file = MB_YES;

		for (i = 0; i < Ctrl->P.pargc; i++) {
			if (strncmp(pargv[i], "OUTFILE", 7) == 0) {
				nscan = sscanf(pargv[i], "OUTFILE:%1023s", process.mbp_ofile);
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
			else if (strncmp(pargv[i], "NAVMODE", 7) == 0) {
				sscanf(pargv[i], "NAVMODE:%d", &process.mbp_nav_mode);
				if (is_explicit == MB_NO && process.mbp_nav_mode == MBP_NAV_OFF) {
					process.mbp_navfile[0] = '\0';
					process.mbp_nav_heading = MBP_NAV_OFF;
					process.mbp_nav_speed = MBP_NAV_OFF;
					process.mbp_nav_draft = MBP_NAV_OFF;
					process.mbp_nav_attitude = MBP_NAV_OFF;
				}
			}
			else if (strncmp(pargv[i], "NAVFILE", 7) == 0) {
				sscanf(pargv[i], "NAVFILE:%1023s", process.mbp_navfile);
				if (is_explicit == MB_NO) {
					process.mbp_nav_mode = MBP_NAV_ON;
					process.mbp_nav_heading = MBP_NAV_ON;
					process.mbp_nav_speed = MBP_NAV_ON;
					process.mbp_nav_draft = MBP_NAV_ON;
					process.mbp_nav_attitude = MBP_NAV_ON;
				}
			}
			else if (strncmp(pargv[i], "NAVFORMAT", 9) == 0)
				sscanf(pargv[i], "NAVFORMAT:%d", &process.mbp_nav_format);
			else if (strncmp(pargv[i], "NAVHEADING", 10) == 0)
				sscanf(pargv[i], "NAVHEADING:%d", &process.mbp_nav_heading);
			else if (strncmp(pargv[i], "NAVSPEED", 8) == 0)
				sscanf(pargv[i], "NAVSPEED:%d", &process.mbp_nav_speed);
			else if (strncmp(pargv[i], "NAVDRAFT", 8) == 0)
				sscanf(pargv[i], "NAVDRAFT:%d", &process.mbp_nav_draft);
			else if (strncmp(pargv[i], "NAVATTITUDE", 11) == 0)
				sscanf(pargv[i], "NAVATTITUDE:%d", &process.mbp_nav_attitude);
			else if (strncmp(pargv[i], "NAVINTERP", 9) == 0)
				sscanf(pargv[i], "NAVINTERP:%d", &process.mbp_nav_algorithm);
			else if (strncmp(pargv[i], "NAVTIMESHIFT", 12) == 0)
				sscanf(pargv[i], "NAVTIMESHIFT:%lf", &process.mbp_nav_timeshift);
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
			else if (strncmp(pargv[i], "NAVSHIFT", 8) == 0)
				sscanf(pargv[i], "NAVSHIFT:%d", &process.mbp_nav_shift);
			else if (strncmp(pargv[i], "NAVADJMODE", 10) == 0) {
				sscanf(pargv[i], "NAVADJMODE:%d", &process.mbp_navadj_mode);
				if (is_explicit == MB_NO && process.mbp_navadj_mode == MBP_NAVADJ_OFF)
					process.mbp_navadjfile[0] = '\0';
			}
			else if (strncmp(pargv[i], "NAVADJFILE", 10) == 0) {
				sscanf(pargv[i], "NAVADJFILE:%1023s", process.mbp_navadjfile);
				if (is_explicit == MB_NO)
					process.mbp_navadj_mode = MBP_NAVADJ_LLZ;
			}
			else if (strncmp(pargv[i], "NAVADJINTERP", 12) == 0)
				sscanf(pargv[i], "NAVADJINTERP:%d", &process.mbp_navadj_algorithm);
			else if (strncmp(pargv[i], "ATTITUDEMODE", 12) == 0) {
				sscanf(pargv[i], "ATTITUDEMODE:%d", &process.mbp_attitude_mode);
				if (is_explicit == MB_NO && process.mbp_attitude_mode == MBP_ATTITUDE_OFF)
					process.mbp_attitudefile[0] = '\0';
			}
			else if (strncmp(pargv[i], "ATTITUDEFILE", 12) == 0) {
				sscanf(pargv[i], "ATTITUDEFILE:%1023s", process.mbp_attitudefile);
				if (is_explicit == MB_NO)
					process.mbp_attitude_mode = MBP_ATTITUDE_ON;
			}
			else if (strncmp(pargv[i], "ATTITUDEFORMAT", 14) == 0)
				sscanf(pargv[i], "ATTITUDEFORMAT:%d", &process.mbp_attitude_format);
			else if (strncmp(pargv[i], "SENSORDEPTHMODE", 15) == 0) {
				sscanf(pargv[i], "SENSORDEPTHMODE:%d", &process.mbp_sensordepth_mode);
				if (is_explicit == MB_NO && process.mbp_sensordepth_mode == MBP_SENSORDEPTH_OFF)
					process.mbp_sensordepthfile[0] = '\0';
			}
			else if (strncmp(pargv[i], "SONARDEPTHMODE", 14) == 0) {
				sscanf(pargv[i], "SONARDEPTHMODE:%d", &process.mbp_sensordepth_mode);
				if (is_explicit == MB_NO && process.mbp_sensordepth_mode == MBP_SENSORDEPTH_OFF)
					process.mbp_sensordepthfile[0] = '\0';
			}
			else if (strncmp(pargv[i], "SENSORDEPTHFILE", 15) == 0) {
				sscanf(pargv[i], "SENSORDEPTHFILE:%1023s", process.mbp_sensordepthfile);
				if (is_explicit == MB_NO)
					process.mbp_sensordepth_mode = MBP_SENSORDEPTH_ON;
			}
			else if (strncmp(pargv[i], "SONARDEPTHFILE", 14) == 0) {
				sscanf(pargv[i], "SONARDEPTHFILE:%1023s", process.mbp_sensordepthfile);
				if (is_explicit == MB_NO)
					process.mbp_sensordepth_mode = MBP_SENSORDEPTH_ON;
			}
			else if (strncmp(pargv[i], "SENSORDEPTHFORMAT", 17) == 0)
				sscanf(pargv[i], "SENSORDEPTHFORMAT:%d", &process.mbp_sensordepth_format);
			else if (strncmp(pargv[i], "SONARDEPTHFORMAT", 16) == 0)
				sscanf(pargv[i], "SONARDEPTHFORMAT:%d", &process.mbp_sensordepth_format);
			else if (strncmp(pargv[i], "DATACUTCLEAR", 12) == 0)
				process.mbp_cut_num = 0;
			else if (strncmp(pargv[i], "DATACUT", 7) == 0) {
				if (process.mbp_cut_num < MBP_CUT_NUM_MAX) {
					sscanf(pargv[i], "DATACUT:%d:%d:%lf:%lf", &process.mbp_cut_kind[process.mbp_cut_num],
																&process.mbp_cut_mode[process.mbp_cut_num],
																&process.mbp_cut_min[process.mbp_cut_num],
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
					sscanf(pargv[i], "BATHCUTSPEED:%lf:%lf", &process.mbp_cut_min[process.mbp_cut_num], &process.mbp_cut_max[process.mbp_cut_num]);
					process.mbp_cut_kind[process.mbp_cut_num] = MBP_CUT_DATA_BATH;
					process.mbp_cut_mode[process.mbp_cut_num] = MBP_CUT_MODE_SPEED;
					process.mbp_cut_num++;
				}
			}
			else if (strncmp(pargv[i], "AMPCUTNUMBER", 12) == 0) {
				if (process.mbp_cut_num < MBP_CUT_NUM_MAX) {
					sscanf(pargv[i], "AMPCUTNUMBER:%lf:%lf", &process.mbp_cut_min[process.mbp_cut_num], &process.mbp_cut_max[process.mbp_cut_num]);
					process.mbp_cut_kind[process.mbp_cut_num] = MBP_CUT_DATA_AMP;
					process.mbp_cut_mode[process.mbp_cut_num] = MBP_CUT_MODE_NUMBER;
					process.mbp_cut_num++;
				}
			}
			else if (strncmp(pargv[i], "AMPCUTDISTANCE", 14) == 0) {
				if (process.mbp_cut_num < MBP_CUT_NUM_MAX) {
					sscanf(pargv[i], "AMPCUTDISTANCE:%lf:%lf", &process.mbp_cut_min[process.mbp_cut_num], &process.mbp_cut_max[process.mbp_cut_num]);
					process.mbp_cut_kind[process.mbp_cut_num] = MBP_CUT_DATA_AMP;
					process.mbp_cut_mode[process.mbp_cut_num] = MBP_CUT_MODE_DISTANCE;
					process.mbp_cut_num++;
				}
			}
			else if (strncmp(pargv[i], "AMPCUTSPEED", 11) == 0) {
				if (process.mbp_cut_num < MBP_CUT_NUM_MAX) {
					sscanf(pargv[i], "AMPCUTSPEED:%lf:%lf", &process.mbp_cut_min[process.mbp_cut_num], &process.mbp_cut_max[process.mbp_cut_num]);
					process.mbp_cut_kind[process.mbp_cut_num] = MBP_CUT_DATA_AMP;
					process.mbp_cut_mode[process.mbp_cut_num] = MBP_CUT_MODE_SPEED;
					process.mbp_cut_num++;
				}
			}
			else if (strncmp(pargv[i], "SSCUTNUMBER", 11) == 0) {
				if (process.mbp_cut_num < MBP_CUT_NUM_MAX) {
					sscanf(pargv[i], "SSCUTNUMBER:%lf:%lf", &process.mbp_cut_min[process.mbp_cut_num], &process.mbp_cut_max[process.mbp_cut_num]);
					process.mbp_cut_kind[process.mbp_cut_num] = MBP_CUT_DATA_SS;
					process.mbp_cut_mode[process.mbp_cut_num] = MBP_CUT_MODE_NUMBER;
					process.mbp_cut_num++;
				}
			}
			else if (strncmp(pargv[i], "SSCUTDISTANCE", 13) == 0) {
				if (process.mbp_cut_num < MBP_CUT_NUM_MAX) {
					sscanf(pargv[i], "SSCUTDISTANCE:%lf:%lf", &process.mbp_cut_min[process.mbp_cut_num], &process.mbp_cut_max[process.mbp_cut_num]);
					process.mbp_cut_kind[process.mbp_cut_num] = MBP_CUT_DATA_SS;
					process.mbp_cut_mode[process.mbp_cut_num] = MBP_CUT_MODE_DISTANCE;
					process.mbp_cut_num++;
				}
			}
			else if (strncmp(pargv[i], "SSCUTSPEED", 10) == 0) {
				if (process.mbp_cut_num < MBP_CUT_NUM_MAX) {
					sscanf(pargv[i], "SSCUTSPEED:%lf:%lf", &process.mbp_cut_min[process.mbp_cut_num], &process.mbp_cut_max[process.mbp_cut_num]);
					process.mbp_cut_kind[process.mbp_cut_num] = MBP_CUT_DATA_SS;
					process.mbp_cut_mode[process.mbp_cut_num] = MBP_CUT_MODE_SPEED;
					process.mbp_cut_num++;
				}
			}
			else if (strncmp(pargv[i], "EDITSAVEMODE", 12) == 0) {
				sscanf(pargv[i], "EDITSAVEMODE:%d", &process.mbp_edit_mode);
				if (is_explicit == MB_NO && process.mbp_edit_mode == MBP_EDIT_OFF)
					process.mbp_editfile[0] = '\0';
			}
			else if (strncmp(pargv[i], "EDITSAVEFILE", 12) == 0) {
				sscanf(pargv[i], "EDITSAVEFILE:%1023s", process.mbp_editfile);
				if (is_explicit == MB_NO)
					process.mbp_edit_mode = MBP_EDIT_ON;
			}
			else if (strncmp(pargv[i], "RAYTRACE", 8) == 0)
				sscanf(pargv[i], "RAYTRACE:%d", &process.mbp_svp_mode);
			else if (strncmp(pargv[i], "SVPMODE", 7) == 0) {
				sscanf(pargv[i], "SVPMODE:%d", &process.mbp_svp_mode);
				if (is_explicit == MB_NO && process.mbp_svp_mode == MBP_SVP_OFF)
					process.mbp_svpfile[0] = '\0';
			}
			else if (strncmp(pargv[i], "SVPFILE", 7) == 0) {
				sscanf(pargv[i], "SVPFILE:%1023s", process.mbp_svpfile);
				if (is_explicit == MB_NO)
					process.mbp_svp_mode = MBP_SVP_ON;
			}
			else if (strncmp(pargv[i], "SSVMODE", 7) == 0)
				sscanf(pargv[i], "SSVMODE:%d", &process.mbp_ssv_mode);
			else if (strncmp(pargv[i], "SSV", 3) == 0)
				sscanf(pargv[i], "SSV:%lf", &process.mbp_ssv);
			else if (strncmp(pargv[i], "TTMODE", 6) == 0)
				sscanf(pargv[i], "TTMODE:%d", &process.mbp_tt_mode);
			else if (strncmp(pargv[i], "TTMULTIPLY", 10) == 0)
				sscanf(pargv[i], "TTMULTIPLY:%lf", &process.mbp_tt_mult);
			else if (strncmp(pargv[i], "CORRECTED", 9) == 0)
				sscanf(pargv[i], "CORRECTED:%d", &process.mbp_corrected);
			else if (strncmp(pargv[i], "ANGLEMODE", 9) == 0)
				sscanf(pargv[i], "ANGLEMODE:%d", &process.mbp_angle_mode);
			else if (strncmp(pargv[i], "SOUNDSPEEDREF", 13) == 0)
				sscanf(pargv[i], "SOUNDSPEEDREF:%d", &process.mbp_corrected);
			else if (strncmp(pargv[i], "STATICMODE", 10) == 0) {
				sscanf(pargv[i], "STATICMODE:%d", &process.mbp_static_mode);
				if (is_explicit == MB_NO && process.mbp_static_mode == MBP_STATIC_OFF)
					process.mbp_staticfile[0] = '\0';
			}
			else if (strncmp(pargv[i], "STATICFILE", 10) == 0) {
				sscanf(pargv[i], "STATICFILE:%1023s", process.mbp_staticfile);
				if (is_explicit == MB_NO)
					process.mbp_static_mode = MBP_STATIC_BEAM_ON;
			}
			else if (strncmp(pargv[i], "DRAFTMODE", 9) == 0)
				sscanf(pargv[i], "DRAFTMODE:%d", &process.mbp_draft_mode);
			else if (strncmp(pargv[i], "DRAFTOFFSET", 11) == 0) {
				sscanf(pargv[i], "DRAFTOFFSET:%lf", &process.mbp_draft_offset);
				if (is_explicit == MB_NO && process.mbp_draft_mode == MBP_DRAFT_MULTIPLY)
					process.mbp_draft_mode = MBP_DRAFT_MULTIPLYOFFSET;
				else if (is_explicit == MB_NO && process.mbp_draft_mode == MBP_DRAFT_OFF)
					process.mbp_draft_mode = MBP_DRAFT_OFFSET;
			}
			else if (strncmp(pargv[i], "DRAFTMULTIPLY", 13) == 0) {
				sscanf(pargv[i], "DRAFTMULTIPLY:%lf", &process.mbp_draft_mult);
				if (is_explicit == MB_NO && process.mbp_draft_mode == MBP_DRAFT_OFFSET)
					process.mbp_draft_mode = MBP_DRAFT_MULTIPLYOFFSET;
				else if (is_explicit == MB_NO && process.mbp_draft_mode == MBP_DRAFT_OFF)
					process.mbp_draft_mode = MBP_DRAFT_MULTIPLY;
			}
			else if (strncmp(pargv[i], "DRAFT", 5) == 0) {
				sscanf(pargv[i], "DRAFT:%lf", &process.mbp_draft);
				if (is_explicit == MB_NO)
					process.mbp_draft_mode = MBP_DRAFT_SET;
			}
			else if (strncmp(pargv[i], "HEAVEMODE", 9) == 0)
				sscanf(pargv[i], "HEAVEMODE:%d", &process.mbp_heave_mode);
			else if (strncmp(pargv[i], "HEAVEOFFSET", 11) == 0) {
				sscanf(pargv[i], "HEAVEOFFSET:%lf", &process.mbp_heave);
				if (is_explicit == MB_NO && process.mbp_heave_mode == MBP_HEAVE_MULTIPLY)
					process.mbp_heave_mode = MBP_HEAVE_MULTIPLYOFFSET;
				else if (is_explicit == MB_NO && process.mbp_heave_mode == MBP_HEAVE_OFF)
					process.mbp_heave_mode = MBP_HEAVE_OFFSET;
			}
			else if (strncmp(pargv[i], "HEAVEMULTIPLY", 13) == 0) {
				sscanf(pargv[i], "HEAVEMULTIPLY:%lf", &process.mbp_heave_mult);
				if (is_explicit == MB_NO && process.mbp_heave_mode == MBP_HEAVE_OFFSET)
					process.mbp_heave_mode = MBP_HEAVE_MULTIPLYOFFSET;
				else if (is_explicit == MB_NO && process.mbp_heave_mode == MBP_HEADING_OFF)
					process.mbp_heave_mode = MBP_HEAVE_MULTIPLY;
			}
			else if (strncmp(pargv[i], "LEVERMODE", 9) == 0)
				sscanf(pargv[i], "LEVERMODE:%d", &process.mbp_lever_mode);
			else if (strncmp(pargv[i], "VRUOFFSETX", 10) == 0) {
				sscanf(pargv[i], "VRUOFFSETX:%lf", &process.mbp_vru_offsetx);
				if (is_explicit == MB_NO)
					process.mbp_lever_mode = MBP_LEVER_ON;
			}
			else if (strncmp(pargv[i], "VRUOFFSETY", 10) == 0) {
				sscanf(pargv[i], "VRUOFFSETY:%lf", &process.mbp_vru_offsety);
				if (is_explicit == MB_NO)
					process.mbp_lever_mode = MBP_LEVER_ON;
			}
			else if (strncmp(pargv[i], "VRUOFFSETZ", 10) == 0) {
				sscanf(pargv[i], "VRUOFFSETZ:%lf", &process.mbp_vru_offsetz);
				if (is_explicit == MB_NO)
					process.mbp_lever_mode = MBP_LEVER_ON;
			}
			else if (strncmp(pargv[i], "SONAROFFSETX", 12) == 0) {
				sscanf(pargv[i], "SONAROFFSETX:%lf", &process.mbp_sonar_offsetx);
				if (is_explicit == MB_NO)
					process.mbp_lever_mode = MBP_LEVER_ON;
			}
			else if (strncmp(pargv[i], "SONAROFFSETY", 12) == 0) {
				sscanf(pargv[i], "SONAROFFSETY:%lf", &process.mbp_sonar_offsety);
				if (is_explicit == MB_NO)
					process.mbp_lever_mode = MBP_LEVER_ON;
			}
			else if (strncmp(pargv[i], "SONAROFFSETZ", 12) == 0) {
				sscanf(pargv[i], "SONAROFFSETZ:%lf", &process.mbp_sonar_offsetz);
				if (is_explicit == MB_NO)
					process.mbp_lever_mode = MBP_LEVER_ON;
			}
			else if (strncmp(pargv[i], "ROLLBIASMODE", 12) == 0)
				sscanf(pargv[i], "ROLLBIASMODE:%d", &process.mbp_rollbias_mode);
			else if (strncmp(pargv[i], "ROLLBIASPORT", 12) == 0) {
				sscanf(pargv[i], "ROLLBIASPORT:%lf", &process.mbp_rollbias_port);
				if (is_explicit == MB_NO)
					process.mbp_rollbias_mode = MBP_ROLLBIAS_DOUBLE;
			}
			else if (strncmp(pargv[i], "ROLLBIASSTBD", 12) == 0) {
				sscanf(pargv[i], "ROLLBIASSTBD:%lf", &process.mbp_rollbias_stbd);
				if (is_explicit == MB_NO)
					process.mbp_rollbias_mode = MBP_ROLLBIAS_DOUBLE;
			}
			else if (strncmp(pargv[i], "ROLLBIAS", 8) == 0) {
				sscanf(pargv[i], "ROLLBIAS:%lf", &process.mbp_rollbias);
				if (is_explicit == MB_NO)
					process.mbp_rollbias_mode = MBP_ROLLBIAS_SINGLE;
			}
			else if (strncmp(pargv[i], "PITCHBIASMODE", 13) == 0)
				sscanf(pargv[i], "PITCHBIASMODE:%d", &process.mbp_pitchbias_mode);
			else if (strncmp(pargv[i], "PITCHBIAS", 9) == 0) {
				sscanf(pargv[i], "PITCHBIAS:%lf", &process.mbp_pitchbias);
				if (is_explicit == MB_NO)
					process.mbp_pitchbias_mode = MBP_PITCHBIAS_ON;
			}
			else if (strncmp(pargv[i], "HEADINGMODE", 11) == 0)
				sscanf(pargv[i], "HEADINGMODE:%d", &process.mbp_heading_mode);
			else if (strncmp(pargv[i], "HEADINGOFFSET", 13) == 0) {
				sscanf(pargv[i], "HEADINGOFFSET:%lf", &process.mbp_headingbias);
				if (is_explicit == MB_NO && process.mbp_heading_mode == MBP_HEADING_CALC)
					process.mbp_heading_mode = MBP_HEADING_CALCOFFSET;
				else if (is_explicit == MB_NO && process.mbp_heading_mode == MBP_HEADING_OFF)
					process.mbp_heading_mode = MBP_HEADING_OFFSET;
			}
			else if (strncmp(pargv[i], "TIDEMODE", 8) == 0) {
				sscanf(pargv[i], "TIDEMODE:%d", &process.mbp_tide_mode);
				if (is_explicit == MB_NO && process.mbp_tide_mode == MBP_TIDE_OFF)
					process.mbp_tidefile[0] = '\0';
			}
			else if (strncmp(pargv[i], "TIDEFILE", 8) == 0) {
				sscanf(pargv[i], "TIDEFILE:%1023s", process.mbp_tidefile);
				if (is_explicit == MB_NO)
					process.mbp_tide_mode = MBP_TIDE_ON;
			}
			else if (strncmp(pargv[i], "TIDEFORMAT", 10) == 0)
				sscanf(pargv[i], "TIDEFORMAT:%d", &process.mbp_tide_format);
			else if (strncmp(pargv[i], "AMPCORRMODE", 11) == 0) {
				sscanf(pargv[i], "AMPCORRMODE:%d", &process.mbp_ampcorr_mode);
				if (is_explicit == MB_NO && process.mbp_ampcorr_mode == MBP_AMPCORR_OFF)
					process.mbp_ampcorrfile[0] = '\0';
			}
			else if (strncmp(pargv[i], "AMPCORRFILE", 11) == 0) {
				sscanf(pargv[i], "AMPCORRFILE:%1023s", process.mbp_ampcorrfile);
				if (is_explicit == MB_NO)
					process.mbp_ampcorr_mode = MBP_AMPCORR_ON;
			}
			else if (strncmp(pargv[i], "AMPCORRTYPE", 11) == 0)
				sscanf(pargv[i], "AMPCORRTYPE:%d", &process.mbp_ampcorr_type);
			else if (strncmp(pargv[i], "AMPCORRSYMMETRY", 15) == 0)
				sscanf(pargv[i], "AMPCORRSYMMETRY:%d", &process.mbp_ampcorr_symmetry);
			else if (strncmp(pargv[i], "AMPCORRANGLE", 12) == 0)
				sscanf(pargv[i], "AMPCORRANGLE:%lf", &process.mbp_ampcorr_angle);
			else if (strncmp(pargv[i], "AMPCORRSLOPE", 12) == 0)
				sscanf(pargv[i], "AMPCORRSLOPE:%d", &process.mbp_ampcorr_slope);
			else if (strncmp(pargv[i], "AMPSSCORRTOPOFILE", 17) == 0)
				sscanf(pargv[i], "AMPSSCORRTOPOFILE:%1023s", process.mbp_ampsscorr_topofile);
			else if (strncmp(pargv[i], "SSCORRMODE", 10) == 0) {
				sscanf(pargv[i], "SSCORRMODE:%d", &process.mbp_sscorr_mode);
				if (is_explicit == MB_NO && process.mbp_sscorr_mode == MBP_SSCORR_OFF)
					process.mbp_sscorrfile[0] = '\0';
			}
			else if (strncmp(pargv[i], "SSCORRFILE", 10) == 0) {
				sscanf(pargv[i], "SSCORRFILE:%1023s", process.mbp_sscorrfile);
				if (is_explicit == MB_NO)
					process.mbp_sscorr_mode = MBP_SSCORR_ON;
			}
			else if (strncmp(pargv[i], "SSCORRTYPE", 10) == 0)
				sscanf(pargv[i], "SSCORRTYPE:%d", &process.mbp_sscorr_type);
			else if (strncmp(pargv[i], "SSCORRSYMMETRY", 14) == 0)
				sscanf(pargv[i], "SSCORRSYMMETRY:%d", &process.mbp_sscorr_symmetry);
			else if (strncmp(pargv[i], "SSCORRANGLE", 11) == 0)
				sscanf(pargv[i], "SSCORRANGLE:%lf", &process.mbp_sscorr_angle);
			else if (strncmp(pargv[i], "SSCORRSLOPE", 11) == 0)
				sscanf(pargv[i], "SSCORRSLOPE:%d", &process.mbp_sscorr_slope);
			else if (strncmp(pargv[i], "AMPSSCORRTOPOFILE", 17) == 0)
				sscanf(pargv[i], "AMPSSCORRTOPOFILE:%1023s", process.mbp_ampsscorr_topofile);
			else if (strncmp(pargv[i], "SSRECALCMODE", 12) == 0)
				sscanf(pargv[i], "SSRECALCMODE:%d", &process.mbp_ssrecalc_mode);
			else if (strncmp(pargv[i], "SSPIXELSIZE", 11) == 0)
				sscanf(pargv[i], "SSPIXELSIZE:%lf", &process.mbp_ssrecalc_pixelsize);
			else if (strncmp(pargv[i], "SSSWATHWIDTH", 11) == 0)
				sscanf(pargv[i], "SSSWATHWIDTH:%lf", &process.mbp_ssrecalc_swathwidth);
			else if (strncmp(pargv[i], "SSINTERPOLATE", 11) == 0)
				sscanf(pargv[i], "SSINTERPOLATE:%d", &process.mbp_ssrecalc_interpolate);
			else if (strncmp(pargv[i], "METAVESSEL:", 11) == 0)
				strcpy(process.mbp_meta_vessel, &(pargv[i][11]));
			else if (strncmp(pargv[i], "METAINSTITUTION:", 16) == 0)
				strcpy(process.mbp_meta_institution, &(pargv[i][16]));
			else if (strncmp(pargv[i], "METAPLATFORM:", 13) == 0)
				strcpy(process.mbp_meta_platform, &(pargv[i][13]));
			else if (strncmp(pargv[i], "METASONARVERSION:", 17) == 0)
				strcpy(process.mbp_meta_sonarversion, &(pargv[i][17]));
			else if (strncmp(pargv[i], "METASONAR:", 10) == 0)
				strcpy(process.mbp_meta_sonar, &(pargv[i][10]));
			else if (strncmp(pargv[i], "METACRUISEID:", 13) == 0)
				strcpy(process.mbp_meta_cruiseid, &(pargv[i][13]));
			else if (strncmp(pargv[i], "METACRUISENAME:", 15) == 0)
				strcpy(process.mbp_meta_cruisename, &(pargv[i][15]));
			else if (strncmp(pargv[i], "METAPIINSTITUTION:", 18) == 0)
				strcpy(process.mbp_meta_piinstitution, &(pargv[i][18]));
			else if (strncmp(pargv[i], "METACLIENT:", 11) == 0)
				strcpy(process.mbp_meta_client, &(pargv[i][11]));
			else if (strncmp(pargv[i], "METASVCORRECTED:", 16) == 0)
				sscanf(pargv[i], "METASVCORRECTED:%d", &(process.mbp_meta_svcorrected));
			else if (strncmp(pargv[i], "METATIDECORRECTED:", 18) == 0)
				sscanf(pargv[i], "METATIDECORRECTED:%d", &(process.mbp_meta_tidecorrected));
			else if (strncmp(pargv[i], "METABATHEDITMANUAL:", 19) == 0)
				sscanf(pargv[i], "METABATHEDITMANUAL:%d", &(process.mbp_meta_batheditmanual));
			else if (strncmp(pargv[i], "METABATHEDITAUTO:", 17) == 0)
				sscanf(pargv[i], "METABATHEDITAUTO:%d", &(process.mbp_meta_batheditauto));
			else if (strncmp(pargv[i], "METAROLLBIAS:", 13) == 0)
				sscanf(pargv[i], "METAROLLBIAS:%lf", &(process.mbp_meta_rollbias));
			else if (strncmp(pargv[i], "METAPITCHBIAS:", 14) == 0)
				sscanf(pargv[i], "METAPITCHBIAS:%lf", &(process.mbp_meta_pitchbias));
			else if (strncmp(pargv[i], "METAPI:", 7) == 0)
				strcpy(process.mbp_meta_pi, &(pargv[i][7]));
			else if (strncmp(pargv[i], "METAHEADINGBIAS:", 16) == 0)
				sscanf(pargv[i], "METAHEADINGBIAS:%lf", &(process.mbp_meta_headingbias));
			else if (strncmp(pargv[i], "METADRAFT:", 10) == 0)
				sscanf(pargv[i], "METADRAFT:%lf", &(process.mbp_meta_draft));
			else if (strncmp(pargv[i], "KLUGE001:", 8) == 0)
				sscanf(pargv[i], "KLUGE001:%d", &(process.mbp_kluge001));
			else if (strncmp(pargv[i], "KLUGE002:", 8) == 0)
				sscanf(pargv[i], "KLUGE002:%d", &(process.mbp_kluge002));
			else if (strncmp(pargv[i], "KLUGE003:", 8) == 0)
				sscanf(pargv[i], "KLUGE003:%d", &(process.mbp_kluge003));
			else if (strncmp(pargv[i], "KLUGE004:", 8) == 0)
				sscanf(pargv[i], "KLUGE004:%d", &(process.mbp_kluge004));
			else if (strncmp(pargv[i], "KLUGE005:", 8) == 0)
				sscanf(pargv[i], "KLUGE005:%d", &(process.mbp_kluge005));
			else if (strncmp(pargv[i], "KLUGE006:", 8) == 0)
				sscanf(pargv[i], "KLUGE006:%d", &(process.mbp_kluge006));
			else if (strncmp(pargv[i], "KLUGE007:", 8) == 0)
				sscanf(pargv[i], "KLUGE007:%d", &(process.mbp_kluge007));
			else if (strncmp(pargv[i], "KLUGE008:", 8) == 0)
				sscanf(pargv[i], "KLUGE008:%d", &(process.mbp_kluge008));
			else if (strncmp(pargv[i], "KLUGE009:", 8) == 0)
				sscanf(pargv[i], "KLUGE009:%d", &(process.mbp_kluge009));
			else if (strncmp(pargv[i], "KLUGE010:", 8) == 0)
				sscanf(pargv[i], "KLUGE010:%d", &(process.mbp_kluge010));
			else
				GMT_Report(API, GMT_MSG_NORMAL, "Unrecognized %s command: %s\n", THIS_MODULE_NAME, pargv[i]);
		}

		if (process.mbp_format_specified == MB_NO || process.mbp_ofile_specified == MB_NO)
			mb_pr_default_output(verbose, &process, &error);

		mb_pr_bathmode(verbose, &process, &error);

		if (verbose >= 2) {
			GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  Program <%s>\n", THIS_MODULE_NAME);
			GMT_Report(API, GMT_MSG_NORMAL, "dbg2  MB-system Version %s\n", MB_VERSION);
			GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  MB-System Control Parameters:\n");
			GMT_Report(API, GMT_MSG_NORMAL, "dbg2       verbose:         %d\n", verbose);
		}

		if (verbose == 1) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s>\n", THIS_MODULE_NAME);
			GMT_Report(API, GMT_MSG_NORMAL, "MB-system Version %s\n", MB_VERSION);
			GMT_Report(API, GMT_MSG_NORMAL, "\nOutput MBprocess Parameters:\n");
			GMT_Report(API, GMT_MSG_NORMAL, "\nInput and Output Files:\n");
			if (process.mbp_format_specified)
				GMT_Report(API, GMT_MSG_NORMAL, "  Format:                        %d\n", process.mbp_format);
			if (process.mbp_ifile_specified)
				GMT_Report(API, GMT_MSG_NORMAL, "  Input file:                    %s\n", process.mbp_ifile);
			if (process.mbp_ofile_specified)
				GMT_Report(API, GMT_MSG_NORMAL, "  Output file:                   %s\n", process.mbp_ofile);

			GMT_Report(API, GMT_MSG_NORMAL, "\nNavigation Merging:\n");
			if (process.mbp_nav_mode == MBP_NAV_ON) {
				GMT_Report(API, GMT_MSG_NORMAL, "  Navigation merged from navigation file.\n");
				GMT_Report(API, GMT_MSG_NORMAL, "  Navigation file:               %s\n", process.mbp_navfile);
				GMT_Report(API, GMT_MSG_NORMAL, "  Navigation format:             %d\n", process.mbp_nav_format);
				if (process.mbp_nav_heading == MBP_NAV_ON)
					GMT_Report(API, GMT_MSG_NORMAL, "  Heading merged from navigation file.\n");
				else
					GMT_Report(API, GMT_MSG_NORMAL, "  Heading not merged from navigation file.\n");
				if (process.mbp_nav_speed == MBP_NAV_ON)
					GMT_Report(API, GMT_MSG_NORMAL, "  Speed merged from navigation file.\n");
				else
					GMT_Report(API, GMT_MSG_NORMAL, "  Speed not merged from navigation file.\n");
				if (process.mbp_nav_draft == MBP_NAV_ON)
					GMT_Report(API, GMT_MSG_NORMAL, "  Draft merged from navigation file.\n");
				else
					GMT_Report(API, GMT_MSG_NORMAL, "  Draft not merged from navigation file.\n");
				if (process.mbp_nav_attitude == MBP_NAV_ON)
					GMT_Report(API, GMT_MSG_NORMAL, "  Roll, pitch, and heave merged from navigation file.\n");
				else
					GMT_Report(API, GMT_MSG_NORMAL, "  Roll, pitch, and heave not merged from navigation file.\n");
				if (process.mbp_nav_algorithm == MBP_NAV_LINEAR)
					GMT_Report(API, GMT_MSG_NORMAL, "  Navigation algorithm:          linear interpolation\n");
				else if (process.mbp_nav_algorithm == MBP_NAV_SPLINE)
					GMT_Report(API, GMT_MSG_NORMAL, "  Navigation algorithm:          spline interpolation\n");
				GMT_Report(API, GMT_MSG_NORMAL, "  Navigation time shift:         %f\n", process.mbp_nav_timeshift);
			}
			else
				GMT_Report(API, GMT_MSG_NORMAL, "  Navigation not merged from navigation file.\n");

			GMT_Report(API, GMT_MSG_NORMAL, "\nNavigation Offsets and Shifts:\n");
			if (process.mbp_nav_shift == MBP_NAV_ON) {
				GMT_Report(API, GMT_MSG_NORMAL, "  Navigation positions shifted.\n");
				GMT_Report(API, GMT_MSG_NORMAL, "  Navigation offset x:                  %f\n", process.mbp_nav_offsetx);
				GMT_Report(API, GMT_MSG_NORMAL, "  Navigation offset y:                  %f\n", process.mbp_nav_offsety);
				GMT_Report(API, GMT_MSG_NORMAL, "  Navigation offset z:                  %f\n", process.mbp_nav_offsetz);
				GMT_Report(API, GMT_MSG_NORMAL, "  Navigation longitude shift (degrees): %f\n", process.mbp_nav_shiftlon);
				GMT_Report(API, GMT_MSG_NORMAL, "  Navigation latitude shift (degrees):  %f\n", process.mbp_nav_shiftlat);
				GMT_Report(API, GMT_MSG_NORMAL, "  Navigation longitude shift (meters):  %f\n", process.mbp_nav_shiftx);
				GMT_Report(API, GMT_MSG_NORMAL, "  Navigation latitude shift (meters):   %f\n", process.mbp_nav_shifty);
			}
			else
				GMT_Report(API, GMT_MSG_NORMAL, "  Navigation positions not shifted.\n");

			GMT_Report(API, GMT_MSG_NORMAL, "\nAdjusted Navigation Merging:\n");
			if (process.mbp_navadj_mode == MBP_NAVADJ_LLZ) {
				GMT_Report(API, GMT_MSG_NORMAL, "  Navigation merged from adjusted navigation file.\n");
				GMT_Report(API, GMT_MSG_NORMAL, "  Adjusted navigation file:      %s\n", process.mbp_navadjfile);
				if (process.mbp_navadj_algorithm == MBP_NAV_LINEAR)
					GMT_Report(API, GMT_MSG_NORMAL, "  Adjusted navigation algorithm: linear interpolation\n");
				else if (process.mbp_navadj_algorithm == MBP_NAV_SPLINE)
					GMT_Report(API, GMT_MSG_NORMAL, "  Adjusted navigation algorithm: spline interpolation\n");
			}
			else
				GMT_Report(API, GMT_MSG_NORMAL, "  Navigation not merged from adjusted navigation file.\n");

			GMT_Report(API, GMT_MSG_NORMAL, "\nAttitude Merging:\n");
			if (process.mbp_attitude_mode == MBP_NAV_ON) {
				GMT_Report(API, GMT_MSG_NORMAL, "  Attitude merged from attitude file.\n");
				GMT_Report(API, GMT_MSG_NORMAL, "  Attitude file:                 %s\n", process.mbp_attitudefile);
				GMT_Report(API, GMT_MSG_NORMAL, "  Attitude format:               %d\n", process.mbp_attitude_format);
			}
			else
				GMT_Report(API, GMT_MSG_NORMAL, "  Attitude not merged from attitude file.\n");

			GMT_Report(API, GMT_MSG_NORMAL, "\nData Cutting:\n");
			if (process.mbp_cut_num > 0)
				GMT_Report(API, GMT_MSG_NORMAL, "  Data cutting enabled (%d commands).\n", process.mbp_cut_num);
			else
				GMT_Report(API, GMT_MSG_NORMAL, "  Data cutting disabled.\n");
			for (int j = 0; j < process.mbp_cut_num; j++) {
				if (process.mbp_cut_kind[j] == MBP_CUT_DATA_BATH)
					GMT_Report(API, GMT_MSG_NORMAL, "  Cut[%d]: bathymetry", j);
				else if (process.mbp_cut_kind[j] == MBP_CUT_DATA_AMP)
					GMT_Report(API, GMT_MSG_NORMAL, "  Cut[%d]: amplitude ", j);
				else if (process.mbp_cut_kind[j] == MBP_CUT_DATA_SS)
					GMT_Report(API, GMT_MSG_NORMAL, "  Cut[%d]: sidescan  ", j);
				if (process.mbp_cut_mode[j] == MBP_CUT_MODE_NUMBER)
					GMT_Report(API, GMT_MSG_NORMAL, "  number   ");
				else if (process.mbp_cut_kind[j] == MBP_CUT_MODE_DISTANCE)
					GMT_Report(API, GMT_MSG_NORMAL, "  distance ");
				GMT_Report(API, GMT_MSG_NORMAL, "  %f %f\n", process.mbp_cut_min[j], process.mbp_cut_max[j]);
			}

			GMT_Report(API, GMT_MSG_NORMAL, "\nBathymetry Editing:\n");
			if (process.mbp_edit_mode == MBP_EDIT_ON)
				GMT_Report(API, GMT_MSG_NORMAL, "  Bathymetry edits applied from file.\n");
			else
				GMT_Report(API, GMT_MSG_NORMAL, "  Bathymetry edits not applied from file.\n");
			GMT_Report(API, GMT_MSG_NORMAL, "  Bathymetry edit file:          %s\n", process.mbp_editfile);

			GMT_Report(API, GMT_MSG_NORMAL, "\nBathymetry Recalculation:\n");
			if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_OFF)
				GMT_Report(API, GMT_MSG_NORMAL, "  Bathymetry not recalculated.\n");
			else if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_RAYTRACE)
				GMT_Report(API, GMT_MSG_NORMAL, "  Bathymetry recalculated by raytracing.\n");
			else if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_ROTATE)
				GMT_Report(API, GMT_MSG_NORMAL, "  Bathymetry recalculated by rigid rotation.\n");
			else if (process.mbp_bathrecalc_mode == MBP_BATHRECALC_OFFSET)
				GMT_Report(API, GMT_MSG_NORMAL, "  Bathymetry recalculated by transducer depth shift.\n");
			GMT_Report(API, GMT_MSG_NORMAL, "  SVP file:                      %s\n", process.mbp_svpfile);
			if (process.mbp_ssv_mode == MBP_SSV_OFF)
				GMT_Report(API, GMT_MSG_NORMAL, "  SSV not modified.\n");
			else if (process.mbp_ssv_mode == MBP_SSV_OFFSET)
				GMT_Report(API, GMT_MSG_NORMAL, "  SSV offset by constant.\n");
			else
				GMT_Report(API, GMT_MSG_NORMAL, "  SSV set to constant.\n");
			GMT_Report(API, GMT_MSG_NORMAL, "  SSV offset/constant:           %f m/s\n", process.mbp_ssv);
			GMT_Report(API, GMT_MSG_NORMAL, "  Travel time mode:              %d\n", process.mbp_tt_mode);
			GMT_Report(API, GMT_MSG_NORMAL, "  Travel time multiplier:        %f\n", process.mbp_tt_mult);
			GMT_Report(API, GMT_MSG_NORMAL, "  Raytrace angle mode:           %d\n", process.mbp_angle_mode);

			GMT_Report(API, GMT_MSG_NORMAL, "\nBathymetry Water Sound Speed Reference:\n");
			if (process.mbp_corrected == MB_YES)
				GMT_Report(API, GMT_MSG_NORMAL, "  Output bathymetry reference:   CORRECTED\n");
			else if (process.mbp_corrected == MB_NO)
				GMT_Report(API, GMT_MSG_NORMAL, "  Bathymetry reference:          UNCORRECTED\n");
			if (process.mbp_svp_mode == MBP_SVP_SOUNDSPEEDREF) {
				if (process.mbp_corrected == MB_YES)
					GMT_Report(API, GMT_MSG_NORMAL, "  Depths modified from uncorrected to corrected\n");
				else
					GMT_Report(API, GMT_MSG_NORMAL, "  Depths modified from corrected to uncorrected\n");
			}
			else if (process.mbp_svp_mode == MBP_SVP_ON) {
				if (process.mbp_corrected == MB_YES)
					GMT_Report(API, GMT_MSG_NORMAL, "  Depths recalculated as corrected\n");
				else
					GMT_Report(API, GMT_MSG_NORMAL, "  Depths recalculated as uncorrected\n");
			}
			else
				GMT_Report(API, GMT_MSG_NORMAL, "  Depths unmodified with respect to water sound speed reference\n");

			GMT_Report(API, GMT_MSG_NORMAL, "\nDraft Correction:\n");
			if (process.mbp_draft_mode == MBP_DRAFT_OFF)
				GMT_Report(API, GMT_MSG_NORMAL, "  Draft not modified.\n");
			else if (process.mbp_draft_mode == MBP_DRAFT_SET)
				GMT_Report(API, GMT_MSG_NORMAL, "  Draft set to constant.\n");
			else if (process.mbp_draft_mode == MBP_DRAFT_OFFSET)
				GMT_Report(API, GMT_MSG_NORMAL, "  Draft offset by constant.\n");
			else if (process.mbp_draft_mode == MBP_DRAFT_MULTIPLY)
				GMT_Report(API, GMT_MSG_NORMAL, "  Draft multiplied by constant.\n");
			else if (process.mbp_draft_mode == MBP_DRAFT_MULTIPLYOFFSET)
				GMT_Report(API, GMT_MSG_NORMAL, "  Draft multiplied and offset by constants.\n");
			GMT_Report(API, GMT_MSG_NORMAL, "  Draft constant:                %f m\n", process.mbp_draft);
			GMT_Report(API, GMT_MSG_NORMAL, "  Draft offset:                  %f m\n", process.mbp_draft_offset);
			GMT_Report(API, GMT_MSG_NORMAL, "  Draft multiplier:              %f m\n", process.mbp_draft_mult);

			GMT_Report(API, GMT_MSG_NORMAL, "\nHeave Correction:\n");
			if (process.mbp_heave_mode == MBP_HEAVE_OFF)
				GMT_Report(API, GMT_MSG_NORMAL, "  Heave not modified.\n");
			else if (process.mbp_heave_mode == MBP_HEAVE_OFFSET)
				GMT_Report(API, GMT_MSG_NORMAL, "  Heave offset by constant.\n");
			else if (process.mbp_heave_mode == MBP_HEAVE_MULTIPLY)
				GMT_Report(API, GMT_MSG_NORMAL, "  Heave multiplied by constant.\n");
			else if (process.mbp_heave_mode == MBP_HEAVE_MULTIPLYOFFSET)
				GMT_Report(API, GMT_MSG_NORMAL, "  Heave multiplied and offset by constants.\n");
			GMT_Report(API, GMT_MSG_NORMAL, "  Heave offset:                  %f m\n", process.mbp_heave);
			GMT_Report(API, GMT_MSG_NORMAL, "  Heave multiplier:              %f m\n", process.mbp_heave_mult);

			GMT_Report(API, GMT_MSG_NORMAL, "\nLever Correction:\n");
			if (process.mbp_lever_mode == MBP_LEVER_OFF)
				GMT_Report(API, GMT_MSG_NORMAL, "  Lever calculation off.\n");
			else {
				GMT_Report(API, GMT_MSG_NORMAL, "  Lever calculation used to calculate heave correction.\n");
				GMT_Report(API, GMT_MSG_NORMAL, "  Heave offset:                  %f m\n", process.mbp_heave);
				GMT_Report(API, GMT_MSG_NORMAL, "  VRU offset x:                  %f m\n", process.mbp_vru_offsetx);
				GMT_Report(API, GMT_MSG_NORMAL, "  VRU offset y:                  %f m\n", process.mbp_vru_offsety);
				GMT_Report(API, GMT_MSG_NORMAL, "  VRU offset z:                  %f m\n", process.mbp_vru_offsetz);
				GMT_Report(API, GMT_MSG_NORMAL, "  Sonar offset x:                %f m\n", process.mbp_sonar_offsetx);
				GMT_Report(API, GMT_MSG_NORMAL, "  Sonar offset y:                %f m\n", process.mbp_sonar_offsety);
				GMT_Report(API, GMT_MSG_NORMAL, "  Sonar offset z:                %f m\n", process.mbp_sonar_offsetz);
			}

			GMT_Report(API, GMT_MSG_NORMAL, "\nTide Correction:\n");
			if (process.mbp_tide_mode == MBP_TIDE_OFF)
				GMT_Report(API, GMT_MSG_NORMAL, "  Tide calculation off.\n");
			else {
				GMT_Report(API, GMT_MSG_NORMAL, "  Tide correction applied to bathymetry.\n");
				GMT_Report(API, GMT_MSG_NORMAL, "  Tide file:                     %s\n", process.mbp_tidefile);
				GMT_Report(API, GMT_MSG_NORMAL, "  Tide format:                   %d\n", process.mbp_tide_format);
			}

			GMT_Report(API, GMT_MSG_NORMAL, "\nRoll Correction:\n");
			if (process.mbp_rollbias_mode == MBP_ROLLBIAS_OFF)
				GMT_Report(API, GMT_MSG_NORMAL, "  Roll not modified.\n");
			else if (process.mbp_rollbias_mode == MBP_ROLLBIAS_SINGLE)
				GMT_Report(API, GMT_MSG_NORMAL, "  Roll offset by bias.\n");
			else if (process.mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE)
				GMT_Report(API, GMT_MSG_NORMAL, "  Roll offset by separate port and starboard biases.\n");
			GMT_Report(API, GMT_MSG_NORMAL, "  Roll bias:                     %f deg\n", process.mbp_rollbias);
			GMT_Report(API, GMT_MSG_NORMAL, "  Port roll bias:                %f deg\n", process.mbp_rollbias_port);
			GMT_Report(API, GMT_MSG_NORMAL, "  Starboard roll bias:           %f deg\n", process.mbp_rollbias_stbd);

			GMT_Report(API, GMT_MSG_NORMAL, "\nPitch Correction:\n");
			if (process.mbp_pitchbias_mode == MBP_PITCHBIAS_OFF)
				GMT_Report(API, GMT_MSG_NORMAL, "  Pitch not modified.\n");
			else
				GMT_Report(API, GMT_MSG_NORMAL, "  Pitch offset by bias.\n");
			GMT_Report(API, GMT_MSG_NORMAL, "  Pitch bias:                    %f deg\n", process.mbp_pitchbias);

			GMT_Report(API, GMT_MSG_NORMAL, "\nHeading Correction:\n");
			if (process.mbp_heading_mode == MBP_HEADING_OFF)
				GMT_Report(API, GMT_MSG_NORMAL, "  Heading not modified.\n");
			else if (process.mbp_heading_mode == MBP_HEADING_CALC)
				GMT_Report(API, GMT_MSG_NORMAL, "  Heading replaced by course-made-good.\n");
			else if (process.mbp_heading_mode == MBP_HEADING_OFFSET)
				GMT_Report(API, GMT_MSG_NORMAL, "  Heading offset by bias.\n");
			else if (process.mbp_heading_mode == MBP_HEADING_CALCOFFSET)
				GMT_Report(API, GMT_MSG_NORMAL, "  Heading replaced by course-made-good and then offset by bias.\n");
			GMT_Report(API, GMT_MSG_NORMAL, "  Heading offset:                %f deg\n", process.mbp_headingbias);

			GMT_Report(API, GMT_MSG_NORMAL, "\nAmplitude Corrections:\n");
			if (process.mbp_ampcorr_mode == MBP_SSCORR_ON) {
				GMT_Report(API, GMT_MSG_NORMAL, "  Amplitude vs grazing angle corrections applied to amplitudes.\n");
				GMT_Report(API, GMT_MSG_NORMAL, "  Amplitude correction file:      %s m\n", process.mbp_ampcorrfile);
				if (process.mbp_ampcorr_type == MBP_AMPCORR_SUBTRACTION)
					GMT_Report(API, GMT_MSG_NORMAL, "  Amplitude correction by subtraction (dB scale)\n");
				else
					GMT_Report(API, GMT_MSG_NORMAL, "  Amplitude correction by division (linear scale)\n");
				if (process.mbp_ampcorr_symmetry == MBP_AMPCORR_SYMMETRIC)
					GMT_Report(API, GMT_MSG_NORMAL, "  AVGA tables forced to be symmetric\n");
				else
					GMT_Report(API, GMT_MSG_NORMAL, "  AVGA tables allowed to be asymmetric\n");
				GMT_Report(API, GMT_MSG_NORMAL, "  Reference grazing angle:       %f deg\n", process.mbp_ampcorr_angle);
				if (process.mbp_ampcorr_slope == MBP_AMPCORR_IGNORESLOPE)
					GMT_Report(API, GMT_MSG_NORMAL, "  Amplitude correction ignores seafloor slope\n");
				else if (process.mbp_ampcorr_slope == MBP_AMPCORR_USESLOPE)
					GMT_Report(API, GMT_MSG_NORMAL, "  Amplitude correction uses seafloor slope\n");
				else {
					GMT_Report(API, GMT_MSG_NORMAL, "  Amplitude correction uses topography grid for slope\n");
					GMT_Report(API, GMT_MSG_NORMAL, "  Topography grid file:      %s m\n", process.mbp_ampsscorr_topofile);
				}
			}
			else
				GMT_Report(API, GMT_MSG_NORMAL, "  Amplitude correction off.\n");

			GMT_Report(API, GMT_MSG_NORMAL, "\nSidescan Corrections:\n");
			if (process.mbp_sscorr_mode == MBP_SSCORR_ON) {
				GMT_Report(API, GMT_MSG_NORMAL, "  Amplitude vs grazing angle corrections applied to sidescan.\n");
				GMT_Report(API, GMT_MSG_NORMAL, "  Sidescan correction file:      %s m\n", process.mbp_sscorrfile);
				if (process.mbp_sscorr_type == MBP_SSCORR_SUBTRACTION)
					GMT_Report(API, GMT_MSG_NORMAL, "  Sidescan correction by subtraction (dB scale)\n");
				else
					GMT_Report(API, GMT_MSG_NORMAL, "  Sidescan correction by division (linear scale)\n");
				if (process.mbp_sscorr_symmetry == MBP_SSCORR_SYMMETRIC)
					GMT_Report(API, GMT_MSG_NORMAL, "  AVGA tables forced to be symmetric\n");
				else
					GMT_Report(API, GMT_MSG_NORMAL, "  AVGA tables allowed to be asymmetric\n");
				GMT_Report(API, GMT_MSG_NORMAL, "  Reference grazing angle:       %f deg\n", process.mbp_sscorr_angle);
				if (process.mbp_sscorr_slope == MBP_SSCORR_IGNORESLOPE)
					GMT_Report(API, GMT_MSG_NORMAL, "  Sidescan correction ignores seafloor slope\n");
				else if (process.mbp_ampcorr_slope == MBP_SSCORR_USESLOPE)
					GMT_Report(API, GMT_MSG_NORMAL, "  Sidescan correction uses seafloor slope\n");
				else {
					GMT_Report(API, GMT_MSG_NORMAL, "  Sidescan correction uses topography grid for slope\n");
					GMT_Report(API, GMT_MSG_NORMAL, "  Topography grid file:      %s m\n", process.mbp_ampsscorr_topofile);
				}
			}
			else
				GMT_Report(API, GMT_MSG_NORMAL, "  Sidescan correction off.\n");

			GMT_Report(API, GMT_MSG_NORMAL, "\nSidescan Recalculation:\n");
			if (process.mbp_ssrecalc_mode == MBP_SSRECALC_ON)
				GMT_Report(API, GMT_MSG_NORMAL, "  Sidescan recalculated.\n");
			else
				GMT_Report(API, GMT_MSG_NORMAL, "  Sidescan not recalculated.\n");
			GMT_Report(API, GMT_MSG_NORMAL, "  Sidescan pixel size:           %f\n", process.mbp_ssrecalc_pixelsize);
			GMT_Report(API, GMT_MSG_NORMAL, "  Sidescan swath width:          %f\n", process.mbp_ssrecalc_swathwidth);
			GMT_Report(API, GMT_MSG_NORMAL, "  Sidescan interpolation:        %d\n", process.mbp_ssrecalc_interpolate);

			GMT_Report(API, GMT_MSG_NORMAL, "\nMetadata Insertion:\n");
			GMT_Report(API, GMT_MSG_NORMAL, "  Metadata vessel:               %s\n", process.mbp_meta_vessel);
			GMT_Report(API, GMT_MSG_NORMAL, "  Metadata institution:          %s\n", process.mbp_meta_institution);
			GMT_Report(API, GMT_MSG_NORMAL, "  Metadata platform:             %s\n", process.mbp_meta_platform);
			GMT_Report(API, GMT_MSG_NORMAL, "  Metadata sonar:                %s\n", process.mbp_meta_sonar);
			GMT_Report(API, GMT_MSG_NORMAL, "  Metadata sonarversion:         %s\n", process.mbp_meta_sonarversion);
			GMT_Report(API, GMT_MSG_NORMAL, "  Metadata cruiseid:             %s\n", process.mbp_meta_cruiseid);
			GMT_Report(API, GMT_MSG_NORMAL, "  Metadata cruisename:           %s\n", process.mbp_meta_cruisename);
			GMT_Report(API, GMT_MSG_NORMAL, "  Metadata pi:                   %s\n", process.mbp_meta_pi);
			GMT_Report(API, GMT_MSG_NORMAL, "  Metadata piinstitution:        %s\n", process.mbp_meta_piinstitution);
			GMT_Report(API, GMT_MSG_NORMAL, "  Metadata client:               %s\n", process.mbp_meta_client);
			GMT_Report(API, GMT_MSG_NORMAL, "  Metadata svcorrected:          %d\n", process.mbp_meta_svcorrected);
			GMT_Report(API, GMT_MSG_NORMAL, "  Metadata tidecorrected         %d\n", process.mbp_meta_tidecorrected);
			GMT_Report(API, GMT_MSG_NORMAL, "  Metadata batheditmanual        %d\n", process.mbp_meta_batheditmanual);
			GMT_Report(API, GMT_MSG_NORMAL, "  Metadata batheditauto:         %d\n", process.mbp_meta_batheditauto);
			GMT_Report(API, GMT_MSG_NORMAL, "  Metadata rollbias:             %f\n", process.mbp_meta_rollbias);
			GMT_Report(API, GMT_MSG_NORMAL, "  Metadata pitchbias:            %f\n", process.mbp_meta_pitchbias);
			GMT_Report(API, GMT_MSG_NORMAL, "  Metadata headingbias:          %f\n", process.mbp_meta_headingbias);
			GMT_Report(API, GMT_MSG_NORMAL, "  Metadata draft:                %f\n", process.mbp_meta_draft);

			GMT_Report(API, GMT_MSG_NORMAL, "\nProcessing Kluges:\n");
			GMT_Report(API, GMT_MSG_NORMAL, "  Kluge001:                      %d\n", process.mbp_kluge001);
			GMT_Report(API, GMT_MSG_NORMAL, "  Kluge002:                      %d\n", process.mbp_kluge002);
			GMT_Report(API, GMT_MSG_NORMAL, "  Kluge003:                      %d\n", process.mbp_kluge003);
			GMT_Report(API, GMT_MSG_NORMAL, "  Kluge004:                      %d\n", process.mbp_kluge004);
			GMT_Report(API, GMT_MSG_NORMAL, "  Kluge005:                      %d\n", process.mbp_kluge005);
			GMT_Report(API, GMT_MSG_NORMAL, "  Kluge006:                      %d\n", process.mbp_kluge006);
			GMT_Report(API, GMT_MSG_NORMAL, "  Kluge007:                      %d\n", process.mbp_kluge007);
			GMT_Report(API, GMT_MSG_NORMAL, "  Kluge008:                      %d\n", process.mbp_kluge008);
			GMT_Report(API, GMT_MSG_NORMAL, "  Kluge009:                      %d\n", process.mbp_kluge009);
			GMT_Report(API, GMT_MSG_NORMAL, "  Kluge010:                      %d\n", process.mbp_kluge010);
		}

		/* if the process structure has changed at all, write a new parameter file */
		{
			int num_difference = 0;
			mb_pr_compare(verbose, &process, &process_org, &num_difference, &error);
			if (num_difference > 0)
				write_parameter_file = MB_YES;
		}

		if (write_parameter_file == MB_YES) {
			status = mb_pr_writepar(verbose, mbp_ifile, &process, &error);

			if (status == MB_SUCCESS) {
				if (existing_parameter_file)
					GMT_Report(API, GMT_MSG_NORMAL, "%s: parameter file exists    - updated\n", mbp_ifile);
				else
					GMT_Report(API, GMT_MSG_NORMAL, "%s: no parameter file exists - created\n", mbp_ifile);
			} else {
				if (existing_parameter_file)
					GMT_Report(API, GMT_MSG_NORMAL, "%s: parameter file exists    - ** failed to update **\n", mbp_ifile);
				else
					GMT_Report(API, GMT_MSG_NORMAL, "%s: no parameter file exists - ** failed to create **\n", mbp_ifile);
			}
		} else {
			if (existing_parameter_file)
				GMT_Report(API, GMT_MSG_NORMAL, "File %s: parameter file exists    - not changed\n", mbp_ifile);
			else
				GMT_Report(API, GMT_MSG_NORMAL, "File %s: no parameter file exists - not created\n", mbp_ifile);
		}

		if (Ctrl->read_datalist == MB_YES) {
			if ((status = mb_datalist_read(verbose, Ctrl->datalist, mbp_ifile, mbp_dfile, &Ctrl->F.format, &file_weight, &error)) == MB_SUCCESS)
				read_data = MB_YES;
			else
				read_data = MB_NO;
		}
		else
			read_data = MB_NO;
	}

	if (Ctrl->read_datalist == MB_YES)
		mb_datalist_close(verbose, &Ctrl->datalist, &error);

	if (verbose >= 4)
		status &= mb_memory_list(verbose, &error);

	if (status == MB_FAILURE)
		GMT_Report(API, GMT_MSG_NORMAL, "WARNING: status is MB_FAILURE\n");

	Return (error);
}
