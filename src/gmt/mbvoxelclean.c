/*--------------------------------------------------------------------
 *    The MB-system:  mbvoxelclean.c  8/27/2018
 *
 *    Copyright (c) 2018-2025 by
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
 * mbvoxelclean identifies and flags artifacts in swath bathymetry data
 * using a 3D density filter. The notion applied is that true targets
 * (e.g. the seafloor) result in dense regions of soundings while sparse soundings
 * in the water column or the subsurface are erroneous and can be flagged as bad.
 * This technique is more appropriate for lidar data than multibeam sonar data.
 * The resulting sounding edit events are output to edit save files which can be
 * applied to the data by the program mbprocess. These are the same edit save
 * files created and/or modified by mbvoxelclean and mbedit.
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
 * Author:  D. W. Caress
 * Date:  August 3, 2018
 *
 * GMT module wrapper: re-converted from src/utilities/mbvoxelclean.cc to C
 * and wrapped as a GMT module entry so it can be invoked from the GMT API
 * (and therefore from Julia FFI / Matlab MEX via GMT).
 */

#define THIS_MODULE_NAME	"mbvoxelclean"
#define THIS_MODULE_LIB     "mbsystem"
#define THIS_MODULE_PURPOSE		"Identify and flag artifacts in swath bathymetry using a 3D voxel density filter"
#define THIS_MODULE_KEYS		""
#define THIS_MODULE_NEEDS		""
#define THIS_MODULE_OPTIONS		"->V"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "gmt_dev.h"

#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_info.h"
#include "mb_io.h"
#include "mb_swap.h"
#include "mb_process.h"

/* empty/occupied voxel action modes */
typedef enum {
	MBVC_EMPTY_IGNORE = 0,
	MBVC_EMPTY_FLAG = 1
} empty_mode_t;

typedef enum {
	MBVC_OCCUPIED_IGNORE = 0,
	MBVC_OCCUPIED_UNFLAG = 1
} occupied_mode_t;

/* ping structure */
struct mbvoxelclean_ping_struct {
	int    time_i[7];
	double time_d;
	int    multiplicity;
	double navlon;
	double navlat;
	double heading;
	double sensordepth;
	int    beams_bath;
	int    beams_bath_alloc;
	char   *beamflag;
	char   *beamflagorg;
	double *bathacrosstrack;
	double *bathz;
	double *bathx;
	double *bathy;
	double *bathr;
};

EXTERN_MSC int GMT_mbvoxelclean(void *API, int mode, void *args);

/* --- Control structure -----------------------------------------------
 * Original mbvoxelclean.cc uses long-options (--voxel-size etc).
 * GMT modules use single-character options; map as follows:
 *   -I file                input datalist or single swath file
 *   -F format              MB-System format id
 *   -Sxy[/z]               voxel size in metres
 *   -Tn                    occupy threshold (sounding count)
 *   -C                     count flagged soundings as well as good
 *   -E0|1                  empty-voxel action (0=ignore, 1=flag, default 1)
 *   -O0|1                  occupied-voxel action (0=ignore, 1=unflag, default 0)
 *   -Nn                    neighborhood radius (voxels)
 *   -Avalue                range minimum
 *   -Bvalue                range maximum
 *   -Xvalue                acrosstrack minimum
 *   -Yvalue                acrosstrack maximum
 *   -Pvalue                amplitude minimum
 *   -Qvalue                amplitude maximum
 */
struct MBVOXELCLEAN_CTRL {
	struct mbv_A { bool active; double range_minimum; } A;
	struct mbv_B { bool active; double range_maximum; } B;
	struct mbv_C { bool active; } C;
	struct mbv_E { bool active; empty_mode_t mode; } E;
	struct mbv_F { bool active; int format; } F;
	struct mbv_I { bool active; char *inputfile; } I;
	struct mbv_N { bool active; int neighborhood; } N;
	struct mbv_O { bool active; occupied_mode_t mode; } O;
	struct mbv_P { bool active; double amplitude_minimum; } P;
	struct mbv_Q { bool active; double amplitude_maximum; } Q;
	struct mbv_S { bool active; double xy; double z; } S;
	struct mbv_T { bool active; int threshold; } T;
	struct mbv_X { bool active; double acrosstrack_minimum; } X;
	struct mbv_Y { bool active; double acrosstrack_maximum; } Y;
};

static void *New_mbvoxelclean_Ctrl(struct GMT_CTRL *GMT) {
	struct MBVOXELCLEAN_CTRL *Ctrl = gmt_M_memory(GMT, NULL, 1, struct MBVOXELCLEAN_CTRL);
	Ctrl->S.xy = 0.05;
	Ctrl->S.z = 0.05;
	Ctrl->T.threshold = 5;
	Ctrl->E.mode = MBVC_EMPTY_FLAG;
	Ctrl->O.mode = MBVC_OCCUPIED_IGNORE;
	return Ctrl;
}

static void Free_mbvoxelclean_Ctrl(struct GMT_CTRL *GMT, struct MBVOXELCLEAN_CTRL *Ctrl) {
	if (!Ctrl) return;
	if (Ctrl->I.inputfile) free(Ctrl->I.inputfile);
	gmt_M_free(GMT, Ctrl);
}

static int usage(struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose(API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return GMT_NOERROR;
	GMT_Message(API, GMT_TIME_NONE,
	    "usage: mbvoxelclean -Iinfile [-Fformat -Sxy[/z] -Tthresh -C -E0|1 -O0|1 -Nn\n"
	    "\t-Arange_min -Brange_max -Xacrosstrack_min -Yacrosstrack_max\n"
	    "\t-Pamp_min -Qamp_max -V -H]\n\n");
	if (level == GMT_SYNOPSIS) return GMT_PARSE_ERROR;
	GMT_Message(API, GMT_TIME_NONE,
	    "\t<inputfile> is an MB-System datalist or single swath file.\n\n");
	return GMT_PARSE_ERROR;
}

static int parse(struct GMT_CTRL *GMT, struct MBVOXELCLEAN_CTRL *Ctrl, struct GMT_OPTION *options) {
	unsigned int n_errors = 0, n_files = 0;
	int n, tmp;
	double d1, d2;
	struct GMT_OPTION *opt;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {
		case '<':
			Ctrl->I.active = true;
			if (gmt_check_filearg(GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) {
				Ctrl->I.inputfile = strdup(opt->arg);
				n_files = 1;
			} else {
				GMT_Report(API, GMT_MSG_NORMAL, "Syntax error: only one input file is allowed.\n");
				n_errors++;
			}
			break;
		case 'A':
			n = sscanf(opt->arg, "%lf", &d1);
			if (n > 0) { Ctrl->A.range_minimum = d1; Ctrl->A.active = true; }
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -A option\n"); n_errors++; }
			break;
		case 'B':
			n = sscanf(opt->arg, "%lf", &d1);
			if (n > 0) { Ctrl->B.range_maximum = d1; Ctrl->B.active = true; }
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -B option\n"); n_errors++; }
			break;
		case 'C':
			Ctrl->C.active = true;
			break;
		case 'E':
			n = sscanf(opt->arg, "%d", &tmp);
			if (n > 0) {
				Ctrl->E.mode = (tmp != 0) ? MBVC_EMPTY_FLAG : MBVC_EMPTY_IGNORE;
				Ctrl->E.active = true;
			} else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -E option\n"); n_errors++; }
			break;
		case 'F':
			n = sscanf(opt->arg, "%d", &Ctrl->F.format);
			if (n > 0) Ctrl->F.active = true;
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -F option\n"); n_errors++; }
			break;
		case 'I':
			if (!gmt_access(GMT, opt->arg, R_OK)) {
				Ctrl->I.inputfile = strdup(opt->arg);
				Ctrl->I.active = true;
				n_files = 1;
			} else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -I option (file not found)\n"); n_errors++; }
			break;
		case 'N':
			n = sscanf(opt->arg, "%d", &Ctrl->N.neighborhood);
			if (n > 0) Ctrl->N.active = true;
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -N option\n"); n_errors++; }
			break;
		case 'O':
			n = sscanf(opt->arg, "%d", &tmp);
			if (n > 0) {
				Ctrl->O.mode = (tmp != 0) ? MBVC_OCCUPIED_UNFLAG : MBVC_OCCUPIED_IGNORE;
				Ctrl->O.active = true;
			} else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -O option\n"); n_errors++; }
			break;
		case 'P':
			n = sscanf(opt->arg, "%lf", &d1);
			if (n > 0) { Ctrl->P.amplitude_minimum = d1; Ctrl->P.active = true; }
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -P option\n"); n_errors++; }
			break;
		case 'Q':
			n = sscanf(opt->arg, "%lf", &d1);
			if (n > 0) { Ctrl->Q.amplitude_maximum = d1; Ctrl->Q.active = true; }
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -Q option\n"); n_errors++; }
			break;
		case 'S': {
			d1 = Ctrl->S.xy;
			d2 = Ctrl->S.z;
			n = sscanf(opt->arg, "%lf/%lf", &d1, &d2);
			if (n > 0) {
				Ctrl->S.xy = d1;
				Ctrl->S.z = (n > 1) ? d2 : d1;
				Ctrl->S.active = true;
			} else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -S option\n"); n_errors++; }
			break;
		}
		case 'T':
			n = sscanf(opt->arg, "%d", &Ctrl->T.threshold);
			if (n > 0) Ctrl->T.active = true;
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -T option\n"); n_errors++; }
			break;
		case 'X':
			n = sscanf(opt->arg, "%lf", &d1);
			if (n > 0) { Ctrl->X.acrosstrack_minimum = d1; Ctrl->X.active = true; }
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -X option\n"); n_errors++; }
			break;
		case 'Y':
			n = sscanf(opt->arg, "%lf", &d1);
			if (n > 0) { Ctrl->Y.acrosstrack_maximum = d1; Ctrl->Y.active = true; }
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -Y option\n"); n_errors++; }
			break;
		default:
			n_errors += gmt_default_error(GMT, opt->option);
			break;
		}
	}

	n_errors += gmt_M_check_condition(GMT, n_files != 1,
	                                  "Syntax error: Must specify one input file\n");
	return n_errors ? GMT_PARSE_ERROR : GMT_OK;
}

#define bailout(code)  { gmt_M_free_options(mode); return (code); }
#define Return(code)   { Free_mbvoxelclean_Ctrl(GMT, Ctrl); gmt_end_module(GMT, GMT_cpy); bailout(code); }

/* ====================================================================
 * GMT_mbvoxelclean — GMT module entry point.
 * ==================================================================== */

int GMT_mbvoxelclean(void *V_API, int mode, void *args) {
	int  error = MB_ERROR_NO_ERROR;

	struct MBVOXELCLEAN_CTRL *Ctrl = NULL;
	struct GMT_CTRL    *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION  *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr(V_API);

	if (API == NULL) return GMT_NOT_A_SESSION;
	if (mode == GMT_MODULE_PURPOSE) return usage(API, GMT_MODULE_PURPOSE);
	options = GMT_Create_Options(API, mode, args);
	if (API->error) return API->error;
	if (!options || options->option == GMT_OPT_USAGE)    bailout(usage(API, GMT_USAGE));
	if (options->option == GMT_OPT_SYNOPSIS)             bailout(usage(API, GMT_SYNOPSIS));

#if GMT_MAJOR_VERSION >= 6
	if ((GMT = gmt_init_module(API, THIS_MODULE_LIB, THIS_MODULE_NAME,
	        THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL)
		bailout(API->error);
#else
	GMT = gmt_begin_module(API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy);
#endif
	if (GMT_Parse_Common(API, THIS_MODULE_OPTIONS, options)) Return(API->error);

	Ctrl = New_mbvoxelclean_Ctrl(GMT);
	if ((error = parse(GMT, Ctrl, options)) != 0) Return (error);

	int    verbose = GMT->common.V.active;
	int    format, defaultpings, lonflip;
	double bounds[4];
	int    btime_i[7], etime_i[7];
	double speedmin, timegap;
	int    status = mb_defaults(verbose, &format, &defaultpings, &lonflip, bounds,
	                            btime_i, etime_i, &speedmin, &timegap);

	/* reset defaults but keep format/lonflip; honor user -F */
	if (Ctrl->F.active) format = Ctrl->F.format;
	defaultpings = 1;
	bounds[0] = -360.; bounds[1] = 360.; bounds[2] = -90.; bounds[3] = 90.;
	btime_i[0]=1962; btime_i[1]=2; btime_i[2]=21; btime_i[3]=10; btime_i[4]=30; btime_i[5]=0; btime_i[6]=0;
	etime_i[0]=2062; etime_i[1]=2; etime_i[2]=21; etime_i[3]=10; etime_i[4]=30; etime_i[5]=0; etime_i[6]=0;
	speedmin = 0.0;
	timegap = 1000000000.0;

	char read_file[MB_PATH_MAXLINE];
	strcpy(read_file, Ctrl->I.active ? Ctrl->I.inputfile : "datalist.mb-1");

	double       voxel_size_xy = Ctrl->S.xy;
	double       voxel_size_z = Ctrl->S.z;
	int          occupy_threshold = Ctrl->T.threshold;
	bool         count_flagged = Ctrl->C.active;
	empty_mode_t empty_mode = Ctrl->E.mode;
	occupied_mode_t occupied_mode = Ctrl->O.mode;
	int          neighborhood = Ctrl->N.active ? Ctrl->N.neighborhood : 0;

	bool   apply_range_minimum = Ctrl->A.active;
	double range_minimum = Ctrl->A.range_minimum;
	bool   apply_range_maximum = Ctrl->B.active;
	double range_maximum = Ctrl->B.range_maximum;
	bool   apply_acrosstrack_minimum = Ctrl->X.active;
	double acrosstrack_minimum = Ctrl->X.acrosstrack_minimum;
	bool   apply_acrosstrack_maximum = Ctrl->Y.active;
	double acrosstrack_maximum = Ctrl->Y.acrosstrack_maximum;
	bool   apply_amplitude_minimum = Ctrl->P.active;
	double amplitude_minimum = Ctrl->P.amplitude_minimum;
	bool   apply_amplitude_maximum = Ctrl->Q.active;
	double amplitude_maximum = Ctrl->Q.amplitude_maximum;

	FILE *outfp = (verbose <= 1) ? stdout : stderr;

	if (verbose == 1) {
		fprintf(outfp, "\nProgram %s\n", THIS_MODULE_NAME);
		fprintf(outfp, "MB-system Version %s\n", MB_VERSION);
	}

	if (verbose >= 2) {
		fprintf(outfp, "\ndbg2  Program <%s>\n", THIS_MODULE_NAME);
		fprintf(outfp, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(outfp, "dbg2  Control Parameters:\n");
		fprintf(outfp, "dbg2       verbose:                     %d\n", verbose);
		fprintf(outfp, "dbg2       defaultpings:                %d\n", defaultpings);
		fprintf(outfp, "dbg2       lonflip:                     %d\n", lonflip);
		fprintf(outfp, "dbg2       btime_i[0]:                  %d\n", btime_i[0]);
		fprintf(outfp, "dbg2       btime_i[1]:                  %d\n", btime_i[1]);
		fprintf(outfp, "dbg2       btime_i[2]:                  %d\n", btime_i[2]);
		fprintf(outfp, "dbg2       btime_i[3]:                  %d\n", btime_i[3]);
		fprintf(outfp, "dbg2       btime_i[4]:                  %d\n", btime_i[4]);
		fprintf(outfp, "dbg2       btime_i[5]:                  %d\n", btime_i[5]);
		fprintf(outfp, "dbg2       btime_i[6]:                  %d\n", btime_i[6]);
		fprintf(outfp, "dbg2       etime_i[0]:                  %d\n", etime_i[0]);
		fprintf(outfp, "dbg2       etime_i[1]:                  %d\n", etime_i[1]);
		fprintf(outfp, "dbg2       etime_i[2]:                  %d\n", etime_i[2]);
		fprintf(outfp, "dbg2       etime_i[3]:                  %d\n", etime_i[3]);
		fprintf(outfp, "dbg2       etime_i[4]:                  %d\n", etime_i[4]);
		fprintf(outfp, "dbg2       etime_i[5]:                  %d\n", etime_i[5]);
		fprintf(outfp, "dbg2       etime_i[6]:                  %d\n", etime_i[6]);
		fprintf(outfp, "dbg2       speedmin:                    %f\n", speedmin);
		fprintf(outfp, "dbg2       timegap:                     %f\n", timegap);
		fprintf(outfp, "dbg2       read_file:                   %s\n", read_file);
		fprintf(outfp, "dbg2       format:                      %d\n", format);
		fprintf(outfp, "dbg2       voxel_size_xy:               %f\n", voxel_size_xy);
		fprintf(outfp, "dbg2       voxel_size_z:                %f\n", voxel_size_z);
		fprintf(outfp, "dbg2       occupy_threshold:            %d\n", occupy_threshold);
		fprintf(outfp, "dbg2       empty_mode:                  %d\n", empty_mode);
		fprintf(outfp, "dbg2       occupied_mode:               %d\n", occupied_mode);
		fprintf(outfp, "dbg2       neighborhood:                %d\n", neighborhood);
		fprintf(outfp, "dbg2       apply_range_minimum:         %d\n", apply_range_minimum);
		fprintf(outfp, "dbg2       range_minimum:               %f\n", range_minimum);
		fprintf(outfp, "dbg2       apply_range_maximum:         %d\n", apply_range_maximum);
		fprintf(outfp, "dbg2       range_maximum:               %f\n", range_maximum);
		fprintf(outfp, "dbg2       apply_acrosstrack_minimum:   %d\n", apply_acrosstrack_minimum);
		fprintf(outfp, "dbg2       acrosstrack_minimum:         %f\n", acrosstrack_minimum);
		fprintf(outfp, "dbg2       apply_acrosstrack_maximum:   %d\n", apply_acrosstrack_maximum);
		fprintf(outfp, "dbg2       acrosstrack_maximum:         %f\n", acrosstrack_maximum);
		fprintf(outfp, "dbg2       apply_amplitude_minimum:     %d\n", apply_amplitude_minimum);
		fprintf(outfp, "dbg2       amplitude_minimum:           %f\n", amplitude_minimum);
		fprintf(outfp, "dbg2       apply_amplitude_maximum:     %d\n", apply_amplitude_maximum);
		fprintf(outfp, "dbg2       amplitude_maximum:           %f\n", amplitude_maximum);
	}

	bool uselockfiles = true;
	mb_uselockfiles(verbose, &uselockfiles);

	if (format == 0)
		mb_get_format(verbose, read_file, NULL, &format, &error);

	const bool read_datalist = (format < 0);
	bool   read_data = false;
	void  *datalist = NULL;
	char   swathfile[MB_PATH_MAXLINE];
	char   swathfileread[MB_PATH_MAXLINE];
	char   dfile[MB_PATH_MAXLINE];
	double file_weight;

	if (read_datalist) {
		const int look_processed = MB_DATALIST_LOOK_NO;
		if (mb_datalist_open(verbose, &datalist, read_file, look_processed, &error) != MB_SUCCESS) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open data list file: %s\n", read_file);
			GMT_Report(API, GMT_MSG_NORMAL, "Program <%s> Terminated\n", THIS_MODULE_NAME);
			Return(MB_ERROR_OPEN_FAIL);
		}
		read_data = (mb_datalist_read(verbose, datalist, swathfile, dfile, &format, &file_weight, &error) == MB_SUCCESS);
	} else {
		strcpy(swathfile, read_file);
		read_data = true;
	}

	int    kind = MB_DATA_NONE;
	bool   variable_beams;
	bool   traveltime;
	double btime_d, etime_d;

	int    sensorhead = 0;
	int    sensorhead_error = MB_ERROR_NO_ERROR;
	int    pingsread = 0;
	int    time_i[7];
	double time_d = 0.0;
	double navlon = 0.0, navlat = 0.0;
	double speed = 0.0, heading = 0.0;
	double distance = 0.0, altitude = 0.0, sensordepth = 0.0;
	int    beams_bath = 0, beams_amp = 0, pixels_ss = 0;
	char   *beamflag = NULL, *beamflagorg = NULL;
	double *bath = NULL, *bathacrosstrack = NULL, *bathalongtrack = NULL;
	double *amp = NULL;
	double *ss = NULL, *ssacrosstrack = NULL, *ssalongtrack = NULL;
	char   comment[MB_COMMENT_MAXLINE];

	struct mbvoxelclean_ping_struct *pings = NULL;
	unsigned char *voxel_count = NULL;

	char esffile[MB_PATH_MAXLINE];
	struct mb_esf_struct esf;
	memset((void *)&esf, 0, sizeof(esf));

	int n_files_tot = 0;
	int n_pings_tot = 0;
	int n_beams_tot = 0;
	int n_beamflag_null_tot = 0;
	int n_beamflag_good_tot = 0;
	int n_beamflag_flag_tot = 0;
	int n_esf_flag_tot = 0;
	int n_esf_unflag_tot = 0;
	int n_density_flag_tot = 0;
	int n_density_unflag_tot = 0;
	int n_minrange_flag_tot = 0;
	int n_maxrange_flag_tot = 0;
	int n_minacrosstrack_flag_tot = 0;
	int n_maxacrosstrack_flag_tot = 0;
	int n_minamplitude_flag_tot = 0;
	int n_maxamplitude_flag_tot = 0;

	bool esffile_open = false;
	bool locked = false;
	int  n_voxel_alloc = 0;
	int  npings_alloc = 0;

	/* loop over all files to be read */
	while (read_data) {
		bool oktoprocess = true;
		bool beam_flagging;

		if ((status = mb_format_flags(verbose, &format, &variable_beams, &traveltime, &beam_flagging, &error)) != MB_SUCCESS) {
			char *message = NULL;
			mb_error(verbose, error, &message);
			GMT_Report(API, GMT_MSG_NORMAL,
			    "\nMBIO Error returned from function <mb_format_flags> regarding input format %d:\n%s\n",
			    format, message);
			GMT_Report(API, GMT_MSG_NORMAL, "File <%s> skipped by program <%s>\n", swathfile, THIS_MODULE_NAME);
			oktoprocess = false;
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
		}

		if (!beam_flagging) {
			GMT_Report(API, GMT_MSG_NORMAL,
			    "\nWarning: MBIO format %d does not allow flagging of bad bathymetry data.\n", format);
			GMT_Report(API, GMT_MSG_NORMAL,
			    "When mbprocess applies edits to file <%s> the soundings will be nulled rather than flagged.\n",
			    swathfile);
		}

		char lock_date[25] = "";
		if (uselockfiles) {
			status = mb_pr_lockswathfile(verbose, swathfile, MBP_LOCK_EDITBATHY, THIS_MODULE_NAME, &error);
		} else {
			int lock_purpose = MBP_LOCK_NONE;
			mb_path lock_program = "";
			mb_path lock_user = "";
			mb_path lock_cpu = "";
			mb_pr_lockinfo(verbose, swathfile, &locked, &lock_purpose, lock_program, lock_user, lock_cpu, lock_date, &error);
			if (error == MB_ERROR_FILE_LOCKED) {
				GMT_Report(API, GMT_MSG_NORMAL, "\nFile %s locked but lock ignored\n", swathfile);
				GMT_Report(API, GMT_MSG_NORMAL, "File locked by <%s> running <%s> on cpu <%s> at <%s>\n",
				    lock_user, lock_program, lock_cpu, lock_date);
				error = MB_ERROR_NO_ERROR;
			}
		}

		if (status == MB_FAILURE) {
			if (error == MB_ERROR_FILE_LOCKED) {
				int lock_purpose = MBP_LOCK_NONE;
				mb_path lock_program = "";
				mb_path lock_user = "";
				mb_path lock_cpu = "";
				mb_pr_lockinfo(verbose, swathfile, &locked, &lock_purpose, lock_program, lock_user, lock_cpu, lock_date, &error);
				GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open input file: %s\n", swathfile);
				GMT_Report(API, GMT_MSG_NORMAL, "File locked by <%s> running <%s> on cpu <%s> at <%s>\n",
				    lock_user, lock_program, lock_cpu, lock_date);
			} else if (error == MB_ERROR_OPEN_FAIL) {
				GMT_Report(API, GMT_MSG_NORMAL, "Unable to create lock file for intended input file <%s>\n", swathfile);
				GMT_Report(API, GMT_MSG_NORMAL, "-Likely permissions issue\n");
			}
			oktoprocess = false;
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
		}

		if (oktoprocess) {
			int formatread = format;
			struct mb_info_struct mb_info;
			status = mb_get_info_datalist(verbose, swathfile, &formatread, &mb_info, lonflip, &error);

			if (npings_alloc <= mb_info.nrecords) {
				status &= mb_reallocd(verbose, __FILE__, __LINE__,
				    mb_info.nrecords * sizeof(struct mbvoxelclean_ping_struct),
				    (void **)&pings, &error);
				if (error != MB_ERROR_NO_ERROR) {
					char *message = NULL;
					mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
					GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating pings array:\n%s\n", message);
					mb_memory_clear(verbose, &error);
					Return(error);
				}
				memset((void *)&pings[npings_alloc], 0,
				    (mb_info.nrecords - npings_alloc) * sizeof(struct mbvoxelclean_ping_struct));
				npings_alloc = mb_info.nrecords;
			}
			for (int i = 0; i < mb_info.nrecords; i++) {
				if (pings[i].beams_bath_alloc < mb_info.nbeams_bath) {
					if (error == MB_ERROR_NO_ERROR)
						status &= mb_reallocd(verbose, __FILE__, __LINE__,
						    mb_info.nbeams_bath * sizeof(char),
						    (void **)&pings[i].beamflag, &error);
					if (error == MB_ERROR_NO_ERROR)
						status &= mb_reallocd(verbose, __FILE__, __LINE__,
						    mb_info.nbeams_bath * sizeof(char),
						    (void **)&pings[i].beamflagorg, &error);
					if (error == MB_ERROR_NO_ERROR)
						status &= mb_reallocd(verbose, __FILE__, __LINE__,
						    mb_info.nbeams_bath * sizeof(double),
						    (void **)&pings[i].bathacrosstrack, &error);
					if (error == MB_ERROR_NO_ERROR)
						status &= mb_reallocd(verbose, __FILE__, __LINE__,
						    mb_info.nbeams_bath * sizeof(double),
						    (void **)&pings[i].bathz, &error);
					if (error == MB_ERROR_NO_ERROR)
						status &= mb_reallocd(verbose, __FILE__, __LINE__,
						    mb_info.nbeams_bath * sizeof(double),
						    (void **)&pings[i].bathx, &error);
					if (error == MB_ERROR_NO_ERROR)
						status &= mb_reallocd(verbose, __FILE__, __LINE__,
						    mb_info.nbeams_bath * sizeof(double),
						    (void **)&pings[i].bathy, &error);
					if (error == MB_ERROR_NO_ERROR)
						status &= mb_reallocd(verbose, __FILE__, __LINE__,
						    mb_info.nbeams_bath * sizeof(double),
						    (void **)&pings[i].bathr, &error);
					if (error != MB_ERROR_NO_ERROR) {
						char *message = NULL;
						mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
						GMT_Report(API, GMT_MSG_NORMAL,
						    "\nMBIO Error allocating data arrays within the ping structure:\n%s\n", message);
						mb_memory_clear(verbose, &error);
						Return(error);
					}
					pings[i].beams_bath_alloc = mb_info.nbeams_bath;
				}
			}

			/* local cartesian coords keyed off datalist info */
			double mtodeglon, mtodeglat;
			mb_coor_scale(verbose, mb_info.lat_start, &mtodeglon, &mtodeglat);
			const double headingx = sin(mb_info.heading_start * DTR);
			const double headingy = cos(mb_info.heading_start * DTR);

			strcpy(swathfileread, swathfile);
			formatread = format;
			mb_get_fbt(verbose, swathfileread, &formatread, &error);

			if (verbose > 0) {
				fprintf(stderr, "---------------------------------\n");
				fprintf(stderr, "Processing %s...\n\tActually reading %s...\n", swathfile, swathfileread);
			}

			void *mbio_ptr = NULL;
			if (mb_read_init(verbose, swathfileread, formatread, defaultpings, lonflip, bounds, btime_i, etime_i,
			        speedmin, timegap, &mbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss,
			        &error) != MB_SUCCESS) {
				char *message = NULL;
				mb_error(verbose, error, &message);
				GMT_Report(API, GMT_MSG_NORMAL,
				    "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
				GMT_Report(API, GMT_MSG_NORMAL,
				    "Multibeam File <%s> not initialized for reading\n", swathfile);
				Return(error);
			}

			int n_pings = 0;
			int n_beams = 0;
			int n_beamflag_null = 0;
			int n_beamflag_good = 0;
			int n_beamflag_flag = 0;
			int n_esf_flag = 0;
			int n_esf_unflag = 0;
			int n_density_flag = 0;
			int n_density_unflag = 0;
			int n_minrange_flag = 0;
			int n_maxrange_flag = 0;
			int n_minacrosstrack_flag = 0;
			int n_maxacrosstrack_flag = 0;
			int n_minamplitude_flag = 0;
			int n_maxamplitude_flag = 0;

			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char),
				    (void **)&beamflag, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char),
				    (void **)&beamflagorg, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
				    (void **)&bath, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
				    (void **)&bathacrosstrack, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
				    (void **)&bathalongtrack, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double),
				    (void **)&amp, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
				    (void **)&ss, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
				    (void **)&ssacrosstrack, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
				    (void **)&ssalongtrack, &error);

			if (error != MB_ERROR_NO_ERROR) {
				char *message = NULL;
				mb_error(verbose, error, &message);
				GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
				Return(error);
			}

			void *store_ptr = NULL;
			double x_min = 0, x_max = 0, y_min = 0, y_max = 0, z_min = 0, z_max = 0;

			/* old edit save file */
			if (status == MB_SUCCESS) {
				fprintf(stderr, "\tOpening edit save file...\n");
				status = mb_esf_load(verbose, THIS_MODULE_NAME, swathfile, true, true, esffile, &esf, &error);
				if (status == MB_SUCCESS && esf.esffp != NULL)
					esffile_open = true;
				if (status == MB_FAILURE && error == MB_ERROR_OPEN_FAIL) {
					esffile_open = false;
					GMT_Report(API, GMT_MSG_NORMAL, "Unable to open new edit save file %s\n", esf.esffile);
				} else if (status == MB_FAILURE && error == MB_ERROR_MEMORY_FAIL) {
					esffile_open = false;
					GMT_Report(API, GMT_MSG_NORMAL,
					    "Unable to allocate memory for edits in esf file %s\n", esf.esffile);
				}
				if (esf.nedit > 0) {
					fprintf(stderr, "%d old edits sorted...\n", esf.nedit);
				}
			}

			bool done = false;
			bool first = true;
			while (!done) {
				if (verbose > 1)
					fprintf(stderr, "\n");

				error = MB_ERROR_NO_ERROR;
				status = mb_get(verbose, mbio_ptr, &kind, &pingsread, time_i, &time_d, &navlon,
				    &navlat, &speed, &heading, &distance, &altitude, &sensordepth,
				    &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
				    bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment,
				    &error);
				if (verbose >= 2) {
					fprintf(stderr, "\ndbg2  current data status:\n");
					fprintf(stderr, "dbg2    kind:     %d\n", kind);
					fprintf(stderr, "dbg2    status:   %d\n", status);
				}
				if (status == MB_SUCCESS && kind == MB_DATA_DATA) {
					if (beams_bath > pings[n_pings].beams_bath_alloc) {
						if (error == MB_ERROR_NO_ERROR)
							status &= mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(char),
							    (void **)&pings[n_pings].beamflag, &error);
						if (error == MB_ERROR_NO_ERROR)
							status &= mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(char),
							    (void **)&pings[n_pings].beamflagorg, &error);
						if (error == MB_ERROR_NO_ERROR)
							status &= mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double),
							    (void **)&pings[n_pings].bathacrosstrack, &error);
						if (error == MB_ERROR_NO_ERROR)
							status &= mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double),
							    (void **)&pings[n_pings].bathz, &error);
						if (error == MB_ERROR_NO_ERROR)
							status &= mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double),
							    (void **)&pings[n_pings].bathx, &error);
						if (error == MB_ERROR_NO_ERROR)
							status &= mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double),
							    (void **)&pings[n_pings].bathy, &error);
						if (error == MB_ERROR_NO_ERROR)
							status &= mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double),
							    (void **)&pings[n_pings].bathr, &error);
						if (error != MB_ERROR_NO_ERROR) {
							char *message = NULL;
							mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
							GMT_Report(API, GMT_MSG_NORMAL,
							    "\nMBIO Error allocating data arrays within the ping structure:\n%s\n", message);
							mb_memory_clear(verbose, &error);
							Return(error);
						}
						pings[n_pings].beams_bath_alloc = beams_bath;
					}

					/* ping multiplicity */
					status = mb_get_store(verbose, mbio_ptr, &store_ptr, &error);
					const int sensorhead_status = mb_sensorhead(verbose, mbio_ptr, store_ptr,
					    &sensorhead, &sensorhead_error);
					if (sensorhead_status == MB_SUCCESS) {
						pings[n_pings].multiplicity = sensorhead;
					} else if (n_pings > 0 &&
					           fabs(pings[n_pings].time_d - pings[n_pings - 1].time_d) < MB_ESF_MAXTIMEDIFF) {
						pings[n_pings].multiplicity = pings[n_pings - 1].multiplicity + 1;
					} else {
						pings[n_pings].multiplicity = 0;
					}

					pings[n_pings].time_d = time_d;
					pings[n_pings].navlon = navlon;
					pings[n_pings].navlat = navlat;
					pings[n_pings].heading = heading;
					pings[n_pings].sensordepth = sensordepth;
					pings[n_pings].beams_bath = beams_bath;
					const double sensorx = (navlon - mb_info.lon_start) / mtodeglon;
					const double sensory = (navlat - mb_info.lat_start) / mtodeglat;
					const double sensorz = -sensordepth;
					for (int j = 0; j < beams_bath; j++) {
						pings[n_pings].beamflag[j] = beamflag[j];
						pings[n_pings].beamflagorg[j] = beamflag[j];
						if (!mb_beam_check_flag_null(beamflag[j])) {
							pings[n_pings].bathacrosstrack[j] = bathacrosstrack[j];
							pings[n_pings].bathx[j] = (navlon - mb_info.lon_start) / mtodeglon +
							    headingy * bathacrosstrack[j] + headingx * bathalongtrack[j];
							pings[n_pings].bathy[j] = (navlat - mb_info.lat_start) / mtodeglat -
							    headingx * bathacrosstrack[j] + headingy * bathalongtrack[j];
							pings[n_pings].bathz[j] = -bath[j];
							const double dx = pings[n_pings].bathx[j] - sensorx;
							const double dy = pings[n_pings].bathy[j] - sensory;
							const double dz = pings[n_pings].bathz[j] - sensorz;
							pings[n_pings].bathr[j] = sqrt(dx*dx + dy*dy + dz*dz);
							if (first) {
								x_min = pings[n_pings].bathx[j];
								x_max = pings[n_pings].bathx[j];
								y_min = pings[n_pings].bathy[j];
								y_max = pings[n_pings].bathy[j];
								z_min = pings[n_pings].bathz[j];
								z_max = pings[n_pings].bathz[j];
								first = false;
							} else {
								if (pings[n_pings].bathx[j] < x_min) x_min = pings[n_pings].bathx[j];
								if (pings[n_pings].bathx[j] > x_max) x_max = pings[n_pings].bathx[j];
								if (pings[n_pings].bathy[j] < y_min) y_min = pings[n_pings].bathy[j];
								if (pings[n_pings].bathy[j] > y_max) y_max = pings[n_pings].bathy[j];
								if (pings[n_pings].bathz[j] < z_min) z_min = pings[n_pings].bathz[j];
								if (pings[n_pings].bathz[j] > z_max) z_max = pings[n_pings].bathz[j];
							}

							/* amplitude filter — applied where amp values still available */
							if (apply_amplitude_minimum || apply_amplitude_maximum) {
								if (mb_beam_ok(pings[n_pings].beamflag[j])) {
									if (apply_amplitude_minimum && amp[j] < amplitude_minimum) {
										pings[n_pings].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
										const int action = MBP_EDIT_FILTER;
										mb_ess_save(verbose, &esf, pings[n_pings].time_d,
										    j + pings[n_pings].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
										    action, &error);
										n_minamplitude_flag++;
									}
									if (apply_amplitude_maximum && amp[j] > amplitude_maximum) {
										pings[n_pings].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
										const int action = MBP_EDIT_FILTER;
										mb_ess_save(verbose, &esf, pings[n_pings].time_d,
										    j + pings[n_pings].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
										    action, &error);
										n_maxamplitude_flag++;
									}
								}
							}
						} else {
							pings[n_pings].bathacrosstrack[j] = 0.0;
							pings[n_pings].bathx[j] = 0.0;
							pings[n_pings].bathy[j] = 0.0;
							pings[n_pings].bathz[j] = 0.0;
							pings[n_pings].bathr[j] = 0.0;
						}
					}

					if (verbose >= 2) {
						fprintf(stderr, "\ndbg2  beam locations (ping:beam xxx.xxx yyy.yyy zzz.zzz)\n");
						for (int j = 0; j < pings[n_pings].beams_bath; j++) {
							fprintf(stderr, "dbg2    %d:%3.3d %10.3f %10.3f %10.3f\n",
							    n_pings, j, pings[n_pings].bathx[j],
							    pings[n_pings].bathy[j], pings[n_pings].bathz[j]);
						}

						fprintf(stderr, "\ndbg2  current voxel bounds:\n");
						fprintf(stderr, "dbg2    x_min: %10.3f m\n", x_min);
						fprintf(stderr, "dbg2    x_max: %10.3f m\n", x_max);
						fprintf(stderr, "dbg2    y_min: %10.3f m\n", y_min);
						fprintf(stderr, "dbg2    y_max: %10.3f m\n", y_max);
						fprintf(stderr, "dbg2    z_min: %10.3f m\n", z_min);
						fprintf(stderr, "dbg2    z_max: %10.3f m\n", z_max);
					}

					/* counters */
					for (int j = 0; j < pings[n_pings].beams_bath; j++) {
						if (mb_beam_ok(pings[n_pings].beamflag[j]))
							n_beamflag_good++;
						else if (pings[n_pings].beamflag[j] == MB_FLAG_NULL)
							n_beamflag_null++;
						else
							n_beamflag_flag++;
					}

					/* apply saved edits */
					status &= mb_esf_apply(verbose, &esf, pings[n_pings].time_d, pings[n_pings].multiplicity,
					    pings[n_pings].beams_bath, pings[n_pings].beamflag, &error);

					for (int j = 0; j < pings[n_pings].beams_bath; j++) {
						if (pings[n_pings].beamflag[j] != pings[n_pings].beamflagorg[j]) {
							if (mb_beam_ok(pings[n_pings].beamflag[j]))
								n_esf_unflag++;
							else
								n_esf_flag++;
						}
					}
					n_beams += pings[n_pings].beams_bath;
					n_pings++;
				} else if (error > MB_ERROR_NO_ERROR) {
					done = true;
				}
			}

			status = mb_close(verbose, &mbio_ptr, &error);

			/* acrosstrack filter (pre-density) */
			if (apply_acrosstrack_minimum || apply_acrosstrack_maximum) {
				for (int i = 0; i < n_pings; i++) {
					for (int j = 0; j < pings[i].beams_bath; j++) {
						if (!mb_beam_check_flag_null(pings[i].beamflag[j])) {
							if (apply_acrosstrack_minimum
							    && mb_beam_ok(pings[i].beamflag[j])
							    && pings[i].bathacrosstrack[j] < acrosstrack_minimum) {
								pings[i].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								const int action = MBP_EDIT_FILTER;
								mb_ess_save(verbose, &esf, pings[i].time_d,
								    j + pings[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
								    action, &error);
								n_minacrosstrack_flag++;
							} else if (apply_acrosstrack_maximum
							    && mb_beam_ok(pings[i].beamflag[j])
							    && pings[i].bathacrosstrack[j] > acrosstrack_maximum) {
								pings[i].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								const int action = MBP_EDIT_FILTER;
								mb_ess_save(verbose, &esf, pings[i].time_d,
								    j + pings[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
								    action, &error);
								n_maxacrosstrack_flag++;
							}
						}
					}
				}
			}

			/* range filter (pre-density) */
			if (apply_range_minimum || apply_range_maximum) {
				for (int i = 0; i < n_pings; i++) {
					for (int j = 0; j < pings[i].beams_bath; j++) {
						if (!mb_beam_check_flag_null(pings[i].beamflag[j])) {
							if (apply_range_minimum
							    && mb_beam_ok(pings[i].beamflag[j])
							    && pings[i].bathr[j] < range_minimum) {
								pings[i].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								const int action = MBP_EDIT_FILTER;
								mb_ess_save(verbose, &esf, pings[i].time_d,
								    j + pings[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
								    action, &error);
								n_minrange_flag++;
							} else if (apply_range_maximum
							    && mb_beam_ok(pings[i].beamflag[j])
							    && pings[i].bathr[j] > range_maximum) {
								pings[i].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								const int action = MBP_EDIT_FILTER;
								mb_ess_save(verbose, &esf, pings[i].time_d,
								    j + pings[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
								    action, &error);
								n_maxrange_flag++;
							}
						}
					}
				}
			}

			/* allocate voxel count grid — unsigned char so cap is 255 */
			const int n_voxel_x = (int)((x_max - x_min) / voxel_size_xy) + 3;
			x_min = x_min - 0.5 * voxel_size_xy;
			x_max = x_min + n_voxel_x * voxel_size_xy;
			const int n_voxel_y = (int)((x_max - y_min) / voxel_size_xy) + 3;
			y_min = y_min - 0.5 * voxel_size_xy;
			y_max = y_min + n_voxel_y * voxel_size_xy;
			const int n_voxel_z = (int)((z_max - z_min) / voxel_size_z) + 3;
			z_min = z_min - 0.5 * voxel_size_z;
			z_max = z_min + n_voxel_z * voxel_size_z;
			int n_voxel = n_voxel_x * n_voxel_y * n_voxel_z;
			if (verbose >= 2) {
				fprintf(stderr, "\ndbg2  final voxel bounds:\n");
				fprintf(stderr, "dbg2    x_min:      %10.3f m\n", x_min);
				fprintf(stderr, "dbg2    x_max:      %10.3f m\n", x_max);
				fprintf(stderr, "dbg2    y_min:      %10.3f m\n", y_min);
				fprintf(stderr, "dbg2    y_max:      %10.3f m\n", y_max);
				fprintf(stderr, "dbg2    z_min:      %10.3f m\n", z_min);
				fprintf(stderr, "dbg2    z_max:      %10.3f m\n", z_max);
				fprintf(stderr, "dbg2    n_voxel_x:  %d\n", n_voxel_x);
				fprintf(stderr, "dbg2    n_voxel_y:  %d\n", n_voxel_y);
				fprintf(stderr, "dbg2    n_voxel_z:  %d\n", n_voxel_z);
				fprintf(stderr, "dbg2    n_voxel:    %d\n", n_voxel);
			}

			if (n_voxel_alloc < n_voxel) {
				status &= mb_reallocd(verbose, __FILE__, __LINE__, (size_t)n_voxel,
				    (void **)&voxel_count, &error);
				if (error != MB_ERROR_NO_ERROR) {
					char *message = NULL;
					mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
					GMT_Report(API, GMT_MSG_NORMAL,
					    "\nMBIO Error allocating voxel counting arrays:\n%s\n", message);
					mb_memory_clear(verbose, &error);
					Return(error);
				}
				memset((void *)voxel_count, 0, (size_t)n_voxel);
				n_voxel_alloc = n_voxel;
			}

			/* count soundings per voxel */
			for (int i = 0; i < n_pings; i++) {
				for (int j = 0; j < pings[i].beams_bath; j++) {
					if (!mb_beam_check_flag_null(pings[i].beamflag[j])) {
						const int ix = (int)((pings[i].bathx[j] - x_min) / voxel_size_xy);
						const int iy = (int)((pings[i].bathy[j] - y_min) / voxel_size_xy);
						const int iz = (int)((pings[i].bathz[j] - z_min) / voxel_size_z);
						const int kk = (ix * n_voxel_y + iy) * n_voxel_z + iz;
						if ((mb_beam_ok(pings[i].beamflag[j]) || count_flagged) && voxel_count[kk] < 254) {
							voxel_count[kk]++;
						}
					}
				}
			}

			/* neighborhood expansion of occupied region */
			if (neighborhood > 0) {
				for (int ix = 0; ix < n_voxel_x; ix++) {
					for (int iy = 0; iy < n_voxel_y; iy++) {
						for (int iz = 0; iz < n_voxel_z; iz++) {
							const int kk = (ix * n_voxel_y + iy) * n_voxel_z + iz;
							if (voxel_count[kk] >= occupy_threshold && voxel_count[kk] < 255) {
								const int iix_lo = (ix - neighborhood > 0) ? ix - neighborhood : 0;
								const int iix_hi = (ix + neighborhood + 1 < n_voxel_x) ? ix + neighborhood + 1 : n_voxel_x;
								const int iiy_lo = (iy - neighborhood > 0) ? iy - neighborhood : 0;
								const int iiy_hi = (iy + neighborhood + 1 < n_voxel_y) ? iy + neighborhood + 1 : n_voxel_y;
								const int iiz_lo = (iz - neighborhood > 0) ? iz - neighborhood : 0;
								const int iiz_hi = (iz + neighborhood + 1 < n_voxel_z) ? iz + neighborhood + 1 : n_voxel_z;
								for (int iix = iix_lo; iix < iix_hi; iix++) {
									for (int iiy = iiy_lo; iiy < iiy_hi; iiy++) {
										for (int iiz = iiz_lo; iiz < iiz_hi; iiz++) {
											const int kkk = (iix * n_voxel_y + iiy) * n_voxel_z + iiz;
											if (voxel_count[kkk] < occupy_threshold) {
												voxel_count[kkk] = 255;
											}
										}
									}
								}
							}
						}
					}
				}
			}

			/* threshold → binary mask */
			for (int kk = 0; kk < n_voxel; kk++) {
				if (voxel_count[kk] >= occupy_threshold) {
					voxel_count[kk] = 1;
				} else {
					voxel_count[kk] = 0;
				}
			}

			/* density filter */
			if (occupied_mode == MBVC_OCCUPIED_UNFLAG || empty_mode == MBVC_EMPTY_FLAG) {
				for (int i = 0; i < n_pings; i++) {
					for (int j = 0; j < pings[i].beams_bath; j++) {
						if (!mb_beam_check_flag_null(pings[i].beamflag[j])) {
							const int ix = (int)((pings[i].bathx[j] - x_min) / voxel_size_xy);
							const int iy = (int)((pings[i].bathy[j] - y_min) / voxel_size_xy);
							const int iz = (int)((pings[i].bathz[j] - z_min) / voxel_size_z);
							const int kk = (ix * n_voxel_y + iy) * n_voxel_z + iz;
							if (occupied_mode == MBVC_OCCUPIED_UNFLAG
							    && voxel_count[kk]
							    && !mb_beam_ok(pings[i].beamflag[j])) {
								pings[i].beamflag[j] = MB_FLAG_NONE;
								const int action = MBP_EDIT_UNFLAG;
								mb_ess_save(verbose, &esf, pings[i].time_d,
								    j + pings[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
								    action, &error);
								n_density_unflag++;
							}
							if (empty_mode == MBVC_EMPTY_FLAG
							    && !voxel_count[kk]
							    && mb_beam_ok(pings[i].beamflag[j])) {
								pings[i].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								const int action = MBP_EDIT_FILTER;
								mb_ess_save(verbose, &esf, pings[i].time_d,
								    j + pings[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
								    action, &error);
								n_density_flag++;
							}
						}
					}
				}
			}

			/* acrosstrack filter (post-density — covers density-unflagged) */
			if (apply_acrosstrack_minimum || apply_acrosstrack_maximum) {
				for (int i = 0; i < n_pings; i++) {
					for (int j = 0; j < pings[i].beams_bath; j++) {
						if (!mb_beam_check_flag_null(pings[i].beamflag[j])) {
							if (apply_acrosstrack_minimum
							    && mb_beam_ok(pings[i].beamflag[j])
							    && pings[i].bathacrosstrack[j] < acrosstrack_minimum) {
								pings[i].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								const int action = MBP_EDIT_FILTER;
								mb_ess_save(verbose, &esf, pings[i].time_d,
								    j + pings[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
								    action, &error);
								n_minacrosstrack_flag++;
							} else if (apply_acrosstrack_maximum
							    && mb_beam_ok(pings[i].beamflag[j])
							    && pings[i].bathacrosstrack[j] > acrosstrack_maximum) {
								pings[i].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								const int action = MBP_EDIT_FILTER;
								mb_ess_save(verbose, &esf, pings[i].time_d,
								    j + pings[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
								    action, &error);
								n_maxacrosstrack_flag++;
							}
						}
					}
				}
			}

			/* range filter (post-density — covers density-unflagged) */
			if (apply_range_minimum || apply_range_maximum) {
				for (int i = 0; i < n_pings; i++) {
					for (int j = 0; j < pings[i].beams_bath; j++) {
						if (!mb_beam_check_flag_null(pings[i].beamflag[j])) {
							if (apply_range_minimum
							    && mb_beam_ok(pings[i].beamflag[j])
							    && pings[i].bathr[j] < range_minimum) {
								pings[i].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								const int action = MBP_EDIT_FILTER;
								mb_ess_save(verbose, &esf, pings[i].time_d,
								    j + pings[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
								    action, &error);
								n_minrange_flag++;
							} else if (apply_range_maximum
							    && mb_beam_ok(pings[i].beamflag[j])
							    && pings[i].bathr[j] > range_maximum) {
								pings[i].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								const int action = MBP_EDIT_FILTER;
								mb_ess_save(verbose, &esf, pings[i].time_d,
								    j + pings[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
								    action, &error);
								n_maxrange_flag++;
							}
						}
					}
				}
			}

			/* write final edits for changed beamflags */
			for (int i = 0; i < n_pings; i++) {
				for (int j = 0; j < pings[i].beams_bath; j++) {
					if (pings[i].beamflag[j] != pings[i].beamflagorg[j]) {
						int action = MBP_EDIT_ZERO;
						if (mb_beam_ok(pings[i].beamflag[j])) {
							action = MBP_EDIT_UNFLAG;
						} else if (mb_beam_check_flag_filter2(pings[i].beamflag[j])) {
							action = MBP_EDIT_FILTER;
						} else if (mb_beam_check_flag_filter(pings[i].beamflag[j])) {
							action = MBP_EDIT_FILTER;
						} else if (pings[i].beamflag[j] != MB_FLAG_NULL) {
							action = MBP_EDIT_FLAG;
						} else {
							action = MBP_EDIT_ZERO;
						}
						mb_esf_save(verbose, &esf, pings[i].time_d,
						    j + pings[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, action, &error);
					}
				}
			}

			status = mb_esf_close(verbose, &esf, &error);

			if (esffile_open) {
				status = mb_pr_update_format(verbose, swathfile, true, format, &error);
				status = mb_pr_update_edit(verbose, swathfile, MBP_EDIT_ON, esffile, &error);
			}

			if (uselockfiles)
				status = mb_pr_unlockswathfile(verbose, swathfile, MBP_LOCK_EDITBATHY, THIS_MODULE_NAME, &error);

			/* check memory */
			if (verbose >= 4)
				status = mb_memory_list(verbose, &error);

			n_files_tot++;
			n_pings_tot += n_pings;
			n_beams_tot += n_beams;
			n_beamflag_null_tot += n_beamflag_null;
			n_beamflag_good_tot += n_beamflag_good;
			n_beamflag_flag_tot += n_beamflag_flag;
			n_esf_flag_tot += n_esf_flag;
			n_esf_unflag_tot += n_esf_unflag;
			n_density_flag_tot += n_density_flag;
			n_density_unflag_tot += n_density_unflag;
			n_minrange_flag_tot += n_minrange_flag;
			n_maxrange_flag_tot += n_maxrange_flag;
			n_minacrosstrack_flag_tot += n_minacrosstrack_flag;
			n_maxacrosstrack_flag_tot += n_maxacrosstrack_flag;
			n_minamplitude_flag_tot += n_minamplitude_flag;
			n_maxamplitude_flag_tot += n_maxamplitude_flag;

			if (verbose >= 1) {
				fprintf(stderr, "%7d survey data records processed\n", n_pings);
				fprintf(stderr, "%7d soundings processed\n", n_beams);
				fprintf(stderr, "%7d beams good originally\n", n_beamflag_good);
				fprintf(stderr, "%7d beams flagged originally\n", n_beamflag_flag);
				fprintf(stderr, "%7d beams null originally\n", n_beamflag_null);
				if (esf.nedit > 0) {
					fprintf(stderr, "%7d beams flagged in old esf file\n", n_esf_flag);
					fprintf(stderr, "%7d beams unflagged in old esf file\n", n_esf_unflag);
				}
				fprintf(stderr, "%7d beams flagged by density filter\n", n_density_flag);
				fprintf(stderr, "%7d beams unflagged by density filter\n", n_density_unflag);
				fprintf(stderr, "%7d beams flagged by minimum range filter\n", n_minrange_flag);
				fprintf(stderr, "%7d beams flagged by maximum range filter\n", n_maxrange_flag);
				fprintf(stderr, "%7d beams flagged by minimum acrosstrack filter\n", n_minacrosstrack_flag);
				fprintf(stderr, "%7d beams flagged by maximum acrosstrack filter\n", n_maxacrosstrack_flag);
				fprintf(stderr, "%7d beams flagged by minimum amplitude filter\n", n_minamplitude_flag);
				fprintf(stderr, "%7d beams flagged by maximum amplitude filter\n", n_maxamplitude_flag);
			}
		}

		if (read_datalist) {
			read_data = (mb_datalist_read(verbose, datalist, swathfile, dfile, &format, &file_weight, &error) == MB_SUCCESS);
		} else {
			read_data = false;
		}
	}
	if (read_datalist)
		mb_datalist_close(verbose, &datalist, &error);

	if (verbose > 0) {
		fprintf(stderr, "\n---------------------------------\n");
		fprintf(stderr, "MBvoxelclean Processing Totals:\n");
		fprintf(stderr, "---------------------------------\n");
		fprintf(stderr, "%d total swath data files processed\n", n_files_tot);
		fprintf(stderr, "%d total survey data records processed\n", n_pings_tot);
		fprintf(stderr, "%d total soundings processed\n", n_beams_tot);
		fprintf(stderr, "%d total beams good originally\n", n_beamflag_good_tot);
		fprintf(stderr, "%d total beams flagged originally\n", n_beamflag_flag_tot);
		fprintf(stderr, "%d total beams null originally\n", n_beamflag_null_tot);
		fprintf(stderr, "%d total beams flagged in old esf file\n", n_esf_flag_tot);
		fprintf(stderr, "%d total beams unflagged in old esf file\n", n_esf_unflag_tot);
		fprintf(stderr, "%d total beams flagged by density filter\n", n_density_flag_tot);
		fprintf(stderr, "%d total beams unflagged by density filter\n", n_density_unflag_tot);
		fprintf(stderr, "%d total beams flagged by minimum range filter\n", n_minrange_flag_tot);
		fprintf(stderr, "%d total beams flagged by maximum range filter\n", n_maxrange_flag_tot);
		fprintf(stderr, "%d total beams flagged by minimum acrosstrack filter\n", n_minacrosstrack_flag_tot);
		fprintf(stderr, "%d total beams flagged by maximum acrosstrack filter\n", n_maxacrosstrack_flag_tot);
		fprintf(stderr, "%d total beams flagged by minimum amplitude filter\n", n_minamplitude_flag_tot);
		fprintf(stderr, "%d total beams flagged by maximum amplitude filter\n", n_maxamplitude_flag_tot);
	}

	for (int i = 0; i < npings_alloc; i++) {
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&pings[i].beamflag, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&pings[i].beamflagorg, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&pings[i].bathacrosstrack, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&pings[i].bathz, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&pings[i].bathx, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&pings[i].bathy, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&pings[i].bathr, &error);
		pings[i].beams_bath_alloc = 0;
	}
	if (pings)        status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&pings, &error);
	if (voxel_count)  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&voxel_count, &error);

	if ((status = mb_memory_list(verbose, &error)) == MB_FAILURE) {
		fprintf(stderr,
		    "Program %s completed but failed to deallocate all allocated memory - memory leak!\n", THIS_MODULE_NAME);
	}

	(void)beamflagorg;
	(void)pingsread;
	Return(error);
}
/*--------------------------------------------------------------------*/
