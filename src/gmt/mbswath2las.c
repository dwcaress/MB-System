/*--------------------------------------------------------------------
 *    The MB-system:  mbswath2las.c  11/26/20
 *
 *    Copyright (c) 2020-2025 by
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
 * MBswath2las exports swath bathymetry data from swath files to LAS format files.
 *
 * Author:  D. W. Caress
 * Date:  November 26, 2020
 *
 * GMT-module rewrite of mbswath2las.cc: wrapped as GMT_mbswath2las entry
 * so it can be invoked from the GMT API (Julia FFI / Matlab MEX).
 * Original argv/getopt loop is re-expressed against the GMT_OPTION list;
 * dbg2 stderr prints routed through GMT_Report (GMT_MSG_NORMAL channel).
 */

#define THIS_MODULE_NAME		"mbswath2las"
#define THIS_MODULE_LIB			"mbsystem"
#define THIS_MODULE_PURPOSE		"Export swath bathymetry data from swath files to LAS format files"
#define THIS_MODULE_KEYS		""
#define THIS_MODULE_NEEDS		""
#define THIS_MODULE_OPTIONS		"-:>Vh"

#include "gmt_dev.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"

static const char help_message[] =
    "MBswath2las exports swath bathymetry data from swath files to LAS format files.";
static const char usage_message[] =
    "mbswath2las [--input=input --output=outputfile --verbose --help]";

/*--------------------------------------------------------------------*/
/* --- Control structure ---------------------------------------------- */

struct MBSWATH2LAS_CTRL {
	struct ms2l_B { bool active; int time_i[7]; } B;                 /* survey window start */
	struct ms2l_E { bool active; int time_i[7]; } E;                 /* survey window end */
	struct ms2l_F { bool active; int format; } F;                    /* MB format override */
	struct ms2l_I { bool active; char *inputfile; } I;               /* swath file / datalist */
	struct ms2l_J { bool active; char projection_pars[MB_PATH_MAXLINE]; } J;  /* projection */
	struct ms2l_L { bool active; int lonflip; } L;                   /* lonflip */
	struct ms2l_R { bool active; double bounds[4]; } R;              /* geographic bounds */
	struct ms2l_S { bool active; double speedmin; } S;               /* speed cutoff */
	struct ms2l_T { bool active; double timegap; } T;                /* time gap cutoff */
};

static void *New_mbswath2las_Ctrl(struct GMT_CTRL *GMT) {
	struct MBSWATH2LAS_CTRL *Ctrl = gmt_M_memory(GMT, NULL, 1, struct MBSWATH2LAS_CTRL);
	return Ctrl;
}

static void Free_mbswath2las_Ctrl(struct GMT_CTRL *GMT, struct MBSWATH2LAS_CTRL *Ctrl) {
	if (!Ctrl) return;
	if (Ctrl->I.inputfile) free(Ctrl->I.inputfile);
	gmt_M_free(GMT, Ctrl);
}

static int usage(struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose(API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return GMT_NOERROR;
	GMT_Message(API, GMT_TIME_NONE, "\n%s\n", help_message);
	GMT_Message(API, GMT_TIME_NONE, "\nusage: %s\n", usage_message);
	if (level == GMT_SYNOPSIS) return GMT_PARSE_ERROR;
	return GMT_PARSE_ERROR;
}

static int parse(struct GMT_CTRL *GMT, struct MBSWATH2LAS_CTRL *Ctrl, struct GMT_OPTION *options) {
	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {
		case '<':
			Ctrl->I.active = true;
			if (gmt_check_filearg(GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) {
				Ctrl->I.inputfile = strdup(opt->arg); n_files = 1;
			} else n_errors++;
			break;
		case 'B':
			sscanf(opt->arg, "%d/%d/%d/%d/%d/%d",
			       &Ctrl->B.time_i[0], &Ctrl->B.time_i[1], &Ctrl->B.time_i[2],
			       &Ctrl->B.time_i[3], &Ctrl->B.time_i[4], &Ctrl->B.time_i[5]);
			Ctrl->B.time_i[6] = 0;
			Ctrl->B.active = true;
			break;
		case 'E':
			sscanf(opt->arg, "%d/%d/%d/%d/%d/%d",
			       &Ctrl->E.time_i[0], &Ctrl->E.time_i[1], &Ctrl->E.time_i[2],
			       &Ctrl->E.time_i[3], &Ctrl->E.time_i[4], &Ctrl->E.time_i[5]);
			Ctrl->E.time_i[6] = 0;
			Ctrl->E.active = true;
			break;
		case 'F':
			if (sscanf(opt->arg, "%d", &Ctrl->F.format) > 0) Ctrl->F.active = true; else n_errors++;
			break;
		case 'I':
			if (!gmt_access(GMT, opt->arg, R_OK)) {
				Ctrl->I.inputfile = strdup(opt->arg); Ctrl->I.active = true; n_files = 1;
			} else n_errors++;
			break;
		case 'J':
			sscanf(opt->arg, "%1023s", Ctrl->J.projection_pars);
			Ctrl->J.active = true;
			break;
		case 'L':
			if (sscanf(opt->arg, "%d", &Ctrl->L.lonflip) > 0) Ctrl->L.active = true; else n_errors++;
			break;
		case 'R':
			mb_get_bounds(opt->arg, Ctrl->R.bounds);
			Ctrl->R.active = true;
			break;
		case 'S':
			if (sscanf(opt->arg, "%lf", &Ctrl->S.speedmin) > 0) Ctrl->S.active = true; else n_errors++;
			break;
		case 'T':
			if (sscanf(opt->arg, "%lf", &Ctrl->T.timegap) > 0) Ctrl->T.active = true; else n_errors++;
			break;
		default:
			n_errors += gmt_default_error(GMT, opt->option);
			break;
		}
	}

	(void)n_files;
	return n_errors ? GMT_PARSE_ERROR : GMT_OK;
}

#define bailout(code)  { gmt_M_free_options(mode); return code; }
#define Return(code)   { Free_mbswath2las_Ctrl(GMT, Ctrl); gmt_end_module(GMT, GMT_cpy); bailout(code); }

/*--------------------------------------------------------------------*/
int GMT_mbswath2las(void *V_API, int mode, void *args) {
	int error = MB_ERROR_NO_ERROR;

	struct MBSWATH2LAS_CTRL *Ctrl = NULL;
	struct GMT_CTRL       *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION     *options = NULL;
	struct GMTAPI_CTRL    *API = gmt_get_api_ptr(V_API);

	if (API == NULL) return GMT_NOT_A_SESSION;
	if (mode == GMT_MODULE_PURPOSE) return usage(API, GMT_MODULE_PURPOSE);
	options = GMT_Create_Options(API, mode, args);
	if (API->error) return API->error;
	if (!options || options->option == GMT_OPT_USAGE)    bailout(usage(API, GMT_USAGE));
	if (options->option == GMT_OPT_SYNOPSIS)             bailout(usage(API, GMT_SYNOPSIS));

#if GMT_MAJOR_VERSION >= 6
	if ((GMT = gmt_init_module(API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout(API->error);
#else
	GMT = gmt_begin_module(API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy);
#endif
	if (GMT_Parse_Common(API, THIS_MODULE_OPTIONS, options)) Return(API->error);

	Ctrl = New_mbswath2las_Ctrl(GMT);
	if ((error = parse(GMT, Ctrl, options)) != 0) Return (error);

	int verbose = GMT->common.V.active;
	int format;
	int pings;
	int pings_read;
	int lonflip;
	double bounds[4];
	int btime_i[7];
	int etime_i[7];
	double speedmin;
	double timegap;
	int status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

	bool input_datalist = true;
	char read_file[MB_PATH_MAXLINE] = "datalist.mb-1";
	char projection_pars[MB_PATH_MAXLINE] = "";
	bool use_projection = false;

	/* swathbounds variables */
	int beam_port = 0;
	int beam_stbd = 0;
	int pixel_port = 0;
	int pixel_stbd = 0;

	/* projected coordinate system */
	char projection_id[MB_PATH_MAXLINE] = "";
	int proj_status;
	void *pjptr = NULL;
	double reference_lon, reference_lat;
	int utm_zone;
	double naveasting, navnorthing, deasting, dnorthing;
	double headingx, headingy, mtodeglon, mtodeglat;

	/* process argument list */
	/* Apply parsed Ctrl values onto the mb_defaults-initialised locals
	   (replaces the original getopt() switch on B/E/F/I/J/L/R/S/T/V/H). */
	if (Ctrl->B.active) memcpy(btime_i, Ctrl->B.time_i, sizeof(btime_i));
	if (Ctrl->E.active) memcpy(etime_i, Ctrl->E.time_i, sizeof(etime_i));
	if (Ctrl->F.active) format = Ctrl->F.format;
	if (Ctrl->I.active && Ctrl->I.inputfile) {
		strncpy(read_file, Ctrl->I.inputfile, sizeof(read_file) - 1);
		read_file[sizeof(read_file) - 1] = '\0';
	}
	if (Ctrl->J.active) { strcpy(projection_pars, Ctrl->J.projection_pars); use_projection = true; }
	if (Ctrl->L.active) lonflip = Ctrl->L.lonflip;
	if (Ctrl->R.active) memcpy(bounds, Ctrl->R.bounds, sizeof(bounds));
	if (Ctrl->S.active) speedmin = Ctrl->S.speedmin;
	if (Ctrl->T.active) timegap = Ctrl->T.timegap;

	const bool help = false;   /* GMT API handles -H / --help via usage() above */

	if (verbose == 1 || help) {
		GMT_Report(API, GMT_MSG_NORMAL, "\nProgram %s\n", THIS_MODULE_NAME);
		GMT_Report(API, GMT_MSG_NORMAL, "MB-system Version %s\n", MB_VERSION);
	}

	if (verbose >= 2) {
		GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  Program <%s>\n", THIS_MODULE_NAME);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  MB-system Version %s\n", MB_VERSION);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  Control Parameters:\n");
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       verbose:        %d\n", verbose);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       help:           %d\n", help);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       format:         %d\n", format);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       lonflip:        %d\n", lonflip);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       bounds[0]:      %f\n", bounds[0]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       bounds[1]:      %f\n", bounds[1]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       bounds[2]:      %f\n", bounds[2]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       bounds[3]:      %f\n", bounds[3]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       btime_i[0]:     %d\n", btime_i[0]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       btime_i[1]:     %d\n", btime_i[1]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       btime_i[2]:     %d\n", btime_i[2]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       btime_i[3]:     %d\n", btime_i[3]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       btime_i[4]:     %d\n", btime_i[4]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       btime_i[5]:     %d\n", btime_i[5]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       btime_i[6]:     %d\n", btime_i[6]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       etime_i[0]:     %d\n", etime_i[0]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       etime_i[1]:     %d\n", etime_i[1]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       etime_i[2]:     %d\n", etime_i[2]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       etime_i[3]:     %d\n", etime_i[3]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       etime_i[4]:     %d\n", etime_i[4]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       etime_i[5]:     %d\n", etime_i[5]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       etime_i[6]:     %d\n", etime_i[6]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       speedmin:       %f\n", speedmin);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       timegap:        %f\n", timegap);
	}

	if (format == 0)
		mb_get_format(verbose, read_file, NULL, &format, &error);

	/* determine whether to read one file or a list of files */
	const bool read_datalist = format < 0;
	bool read_data;
	void *datalist = NULL;
	char file[MB_PATH_MAXLINE] = "";
	char dfile[MB_PATH_MAXLINE] = "";
	double file_weight;

	/* open file list */
	if (read_datalist) {
		const int look_processed = MB_DATALIST_LOOK_UNSET;
		if (mb_datalist_open(verbose, &datalist, read_file, look_processed, &error) != MB_SUCCESS) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open data list file: %s\n", read_file);
			GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", THIS_MODULE_NAME);
			Return(MB_ERROR_OPEN_FAIL);
		}
		read_data = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error) == MB_SUCCESS;
	} else {
		// else copy single filename to be read
		strcpy(file, read_file);
		read_data = true;
	}

	double btime_d;
	double etime_d;
	int beams_bath;
	int beams_amp;
	int pixels_ss;

	/* MBIO read values */
	void *mbio_ptr = NULL;
	void *store_ptr = NULL;
	int kind;
	int time_i[7];
	double time_d;
	double navlon;
	double navlat;
	double speed;
	double heading;
	double distance;
	double altitude;
	double sensordepth;
	double draft;
	double roll;
	double pitch;
	double heave;
	char *beamflag = NULL;
	double *bath = NULL;
	double *bathacrosstrack = NULL;
	double *bathalongtrack = NULL;
	int *detect = NULL;
	double *amp = NULL;
	double *ss = NULL;
	double *ssacrosstrack = NULL;
	double *ssalongtrack = NULL;
	char comment[MB_COMMENT_MAXLINE];
	int icomment = 0;

	/* loop over all files to be read */
	while (read_data) {

		/* initialize reading the swath file */
		if (mb_read_init(verbose, file, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &mbio_ptr,
		                           &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
			char *message;
			mb_error(verbose, error, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nMultibeam File <%s> not initialized for reading\n", file);
			GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", THIS_MODULE_NAME);
			if (read_datalist) mb_datalist_close(verbose, &datalist, &error);
			Return(error);
		}

		/* allocate memory for data arrays */
		if (error == MB_ERROR_NO_ERROR)
			/* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status &= */
			    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status &= */
			    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, &error);

		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR) {
			char *message;
			mb_error(verbose, error, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", THIS_MODULE_NAME);
			mb_close(verbose, &mbio_ptr, &error);
			if (read_datalist) mb_datalist_close(verbose, &datalist, &error);
			Return(error);
		}

		/* read and print data */
		int nread = 0;
		bool first = true;
		while (error <= MB_ERROR_NO_ERROR) {
			/* reset error */
			error = MB_ERROR_NO_ERROR;

			/* read next data record */
			status = mb_get_all(verbose, mbio_ptr, &store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
			                    &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
			                    bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

			/* time gaps are not a problem here */
			if (error == MB_ERROR_TIME_GAP) {
				error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
			}

			/* if survey data extract nav */
			if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA)
				status = mb_extract_nav(verbose, mbio_ptr, store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed,
				                        &heading, &draft, &roll, &pitch, &heave, &error);

			/* make sure non survey data records are ignored */
			if (error == MB_ERROR_NO_ERROR && kind != MB_DATA_DATA)
				error = MB_ERROR_OTHER;

			/* get projected navigation if needed */
			if (error <= MB_ERROR_NO_ERROR && kind == MB_DATA_DATA && use_projection) {
				/* set up projection if this is the first data */
				if (pjptr == NULL) {
					/* Default projection is UTM */
					if (strlen(projection_pars) == 0)
						strcpy(projection_pars, "U");

					/* check for UTM with undefined zone */
					if (strcmp(projection_pars, "UTM") == 0 || strcmp(projection_pars, "U") == 0 ||
					    strcmp(projection_pars, "utm") == 0 || strcmp(projection_pars, "u") == 0) {
						reference_lon = navlon;
						if (reference_lon < 180.0)
							reference_lon += 360.0;
						if (reference_lon >= 180.0)
							reference_lon -= 360.0;
						utm_zone = (int)(((reference_lon + 183.0) / 6.0) + 0.5);
						reference_lat = navlat;
						if (reference_lat >= 0.0)
							snprintf(projection_id, sizeof(projection_id), "UTM%2.2dN", utm_zone);
						else
							snprintf(projection_id, sizeof(projection_id), "UTM%2.2dS", utm_zone);
					}
					else
						strcpy(projection_id, projection_pars);

					/* set projection flag */
					proj_status = mb_proj_init(verbose, projection_id, &(pjptr), &error);

					/* if projection not successfully initialized then quit */
					if (proj_status != MB_SUCCESS) {
						GMT_Report(API, GMT_MSG_NORMAL, "\nOutput projection %s not found in database\n", projection_id);
						GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", THIS_MODULE_NAME);
						error = MB_ERROR_BAD_PARAMETER;
						mb_memory_clear(verbose, &error);
						mb_close(verbose, &mbio_ptr, &error);
						if (read_datalist) mb_datalist_close(verbose, &datalist, &error);
						Return(MB_ERROR_BAD_PARAMETER);
					}
				}

				/* get projected navigation */
				mb_proj_forward(verbose, pjptr, navlon, navlat, &naveasting, &navnorthing, &error);
			}

			if (verbose >= 2) {
				GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  Ping read in program <%s>\n", THIS_MODULE_NAME);
				GMT_Report(API, GMT_MSG_NORMAL, "dbg2       kind:           %d\n", kind);
				GMT_Report(API, GMT_MSG_NORMAL, "dbg2       error:          %d\n", error);
				GMT_Report(API, GMT_MSG_NORMAL, "dbg2       status:         %d\n", status);
			}

			if (verbose >= 1 && kind == MB_DATA_COMMENT) {
				if (icomment == 0) {
					fprintf(stderr, "\nComments:\n");
					icomment++;
				}
				fprintf(stderr, "%s\n", comment);
			}

			/* get factors for lon lat calculations */
			if (error == MB_ERROR_NO_ERROR) {
				mb_coor_scale(verbose, navlat, &mtodeglon, &mtodeglat);
				headingx = sin(DTR * heading);
				headingy = cos(DTR * heading);
			}

			/* now loop over beams */
			if (error == MB_ERROR_NO_ERROR)
				for (int j = 0; j <= beams_bath; j++) {
				}
		}

		/* close the swath file */
		status &= mb_close(verbose, &mbio_ptr, &error);

		/* figure out whether and what to read next */
		if (read_datalist) {
			read_data = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error) == MB_SUCCESS;
		} else {
			read_data = false;
		}

		/* end loop over files in list */
	}
	if (read_datalist)
		mb_datalist_close(verbose, &datalist, &error);

	if (verbose >= 4)
		status &= mb_memory_list(verbose, &error);

	if (verbose >= 2) {
		GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  Program <%s> completed\n", THIS_MODULE_NAME);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  Ending status:\n");
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       status:  %d\n", status);
	}

	Return(error);
}
/*--------------------------------------------------------------------*/
