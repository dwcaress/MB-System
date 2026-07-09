/*--------------------------------------------------------------------
 *    The MB-system:	mbnavlist.c	2/1/93
 *
 *    Copyright (c) 1993-2025 by
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
 * mbnavlist prints the specified contents of navigation records
 * in a swath sonar data file to stdout. The form of the
 * output is quite flexible; mbnavlist is tailored to produce
 * ascii files in spreadsheet style with data columns separated by tabs.
 *
 * Author:	D. W. Caress
 * Date:	November 11, 1999
 *
 * GMT-module rewrite of mbnavlist.cc: wrapped as GMT_mbnavlist entry
 * so it can be invoked from the GMT API (Julia FFI / Matlab MEX).
 */

#define THIS_MODULE_NAME	"mbnavlist"
#define THIS_MODULE_LIB     "mbsystem"
#define THIS_MODULE_PURPOSE		"List contents of navigation records in a swath sonar data file"
#define THIS_MODULE_KEYS		"<D{,>D}"
#define THIS_MODULE_NEEDS		""
#define THIS_MODULE_OPTIONS		"-:>RVhi"

#include "gmt_dev.h"

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"

#define MAX_OPTIONS 100

typedef enum {
	MBNAVLIST_SEGMENT_MODE_NONE      = 0,
	MBNAVLIST_SEGMENT_MODE_TAG       = 1,
	MBNAVLIST_SEGMENT_MODE_SWATHFILE = 2,
	MBNAVLIST_SEGMENT_MODE_DATALIST  = 3
} segment_mode_t;

EXTERN_MSC int GMT_mbnavlist(void *API, int mode, void *args);

/* --- helper ---------------------------------------------------------- */

static int printsimplevalue(int verbose, double value, int width, int precision, bool ascii,
                            bool *invert, bool *flipsign, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       value:           %f\n", value);
		fprintf(stderr, "dbg2       width:           %d\n", width);
		fprintf(stderr, "dbg2       precision:       %d\n", precision);
		fprintf(stderr, "dbg2       ascii:           %d\n", ascii);
		fprintf(stderr, "dbg2       invert:          %d\n", *invert);
		fprintf(stderr, "dbg2       flipsign:        %d\n", *flipsign);
	}

	char format[24];
	format[0] = '%';
	if (*invert)
		strcpy(format, "%g");
	else if (width > 0)
		snprintf(&format[1], 23, "%d.%df", width, precision);
	else
		snprintf(&format[1], 23, ".%df", precision);

	if (*invert) {
		*invert = false;
		if (value != 0.0)
			value = 1.0 / value;
	}

	if (*flipsign) {
		*flipsign = false;
		value = -value;
	}

	if (ascii)
		printf(format, value);
	else
		fwrite(&value, sizeof(double), 1, stdout);

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       invert:          %d\n", *invert);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return status;
}

/* --- Control structure ---------------------------------------------- */

struct MBNAVLIST_CTRL {
	struct mnl_A { bool active; } A;                                   /* ascii=false (binary) */
	struct mnl_B { bool active; int t[7]; } B;
	struct mnl_D { bool active; int decimate; } D;
	struct mnl_E { bool active; int t[7]; } E;
	struct mnl_F { bool active; int format; } F;
	struct mnl_G { bool active; char delim[MB_PATH_MAXLINE]; } G;
	struct mnl_I { bool active; char *inputfile; } I;
	struct mnl_J { bool active; char proj[MB_PATH_MAXLINE]; } J;
	struct mnl_K { bool active; int data_kind; } K;
	struct mnl_L { bool active; int lonflip; } L;
	struct mnl_N { bool active; int aux_nav_channel; } N;
	struct mnl_O { bool active; char list[MAX_OPTIONS]; int n_list; bool use_projection; } O;
	struct mnl_R { bool active; double bounds[4]; } R;
	struct mnl_S { bool active; double speedmin; } S;
	struct mnl_T { bool active; double timegap; } T;
	struct mnl_Z { bool active; segment_mode_t mode; char tag[MB_PATH_MAXLINE]; } Z;
};

static void *New_mbnavlist_Ctrl(struct GMT_CTRL *GMT) {
	struct MBNAVLIST_CTRL *Ctrl = gmt_M_memory(GMT, NULL, 1, struct MBNAVLIST_CTRL);
	strcpy(Ctrl->G.delim, "\t");
	Ctrl->D.decimate = 1;
	Ctrl->K.data_kind = -1;
	Ctrl->N.aux_nav_channel = -1;
	strcpy(Ctrl->O.list, "tMXYHs");
	Ctrl->O.n_list = 6;
	return Ctrl;
}

static void Free_mbnavlist_Ctrl(struct GMT_CTRL *GMT, struct MBNAVLIST_CTRL *Ctrl) {
	if (!Ctrl) return;
	if (Ctrl->I.inputfile) free(Ctrl->I.inputfile);
	gmt_M_free(GMT, Ctrl);
}

static int usage(struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose(API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return GMT_NOERROR;
	GMT_Message(API, GMT_TIME_NONE,
	    "usage: mbnavlist [-A -Byr/mo/da/hr/mn/sc -Ddecimate -Eyr/mo/da/hr/mn/sc\n"
	    "\t-Fformat -Gdelim -Ifile -Jproj -Kkind -Llonflip -Nnavchan\n"
	    "\t-Ooptions -Rw/e/s/n -Sspeed -Ttimegap -Zseg -V -H]\n\n");
	if (level == GMT_SYNOPSIS) return GMT_PARSE_ERROR;
	return GMT_PARSE_ERROR;
}

static int parse(struct GMT_CTRL *GMT, struct MBNAVLIST_CTRL *Ctrl, struct GMT_OPTION *options) {
	unsigned int n_errors = 0, n_files = 0;
	int n;
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
		case 'A': Ctrl->A.active = true; break;
		case 'B':
			Ctrl->B.t[6] = 0;
			n = sscanf(opt->arg, "%d/%d/%d/%d/%d/%d", &Ctrl->B.t[0], &Ctrl->B.t[1], &Ctrl->B.t[2],
			           &Ctrl->B.t[3], &Ctrl->B.t[4], &Ctrl->B.t[5]);
			if (n == 6) Ctrl->B.active = true; else n_errors++;
			break;
		case 'D':
			if (sscanf(opt->arg, "%d", &Ctrl->D.decimate) > 0) Ctrl->D.active = true; else n_errors++;
			break;
		case 'E':
			Ctrl->E.t[6] = 0;
			n = sscanf(opt->arg, "%d/%d/%d/%d/%d/%d", &Ctrl->E.t[0], &Ctrl->E.t[1], &Ctrl->E.t[2],
			           &Ctrl->E.t[3], &Ctrl->E.t[4], &Ctrl->E.t[5]);
			if (n == 6) Ctrl->E.active = true; else n_errors++;
			break;
		case 'F':
			if (sscanf(opt->arg, "%d", &Ctrl->F.format) > 0) Ctrl->F.active = true; else n_errors++;
			break;
		case 'G':
			sscanf(opt->arg, "%1023s", Ctrl->G.delim);
			Ctrl->G.active = true;
			break;
		case 'I':
			if (!gmt_access(GMT, opt->arg, R_OK)) {
				Ctrl->I.inputfile = strdup(opt->arg); Ctrl->I.active = true; n_files = 1;
			} else n_errors++;
			break;
		case 'J':
			sscanf(opt->arg, "%1023s", Ctrl->J.proj);
			Ctrl->J.active = true;
			break;
		case 'K':
			if (sscanf(opt->arg, "%d", &Ctrl->K.data_kind) > 0) Ctrl->K.active = true; else n_errors++;
			break;
		case 'L':
			if (sscanf(opt->arg, "%d", &Ctrl->L.lonflip) > 0) Ctrl->L.active = true; else n_errors++;
			break;
		case 'N':
			if (sscanf(opt->arg, "%d", &Ctrl->N.aux_nav_channel) > 0) Ctrl->N.active = true; else n_errors++;
			break;
		case 'O':
			if (strlen(opt->arg) > 0) {
				int len = (int)strlen(opt->arg);
				if (len > MAX_OPTIONS) len = MAX_OPTIONS;
				Ctrl->O.n_list = len;
				for (int j = 0; j < len; j++) {
					Ctrl->O.list[j] = opt->arg[j];
					if (Ctrl->O.list[j] == '^') Ctrl->O.use_projection = true;
				}
			}
			Ctrl->O.active = true;
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
		case 'Z':
			sscanf(opt->arg, "%1023s", Ctrl->Z.tag);
			if      (strcmp(Ctrl->Z.tag, "swathfile") == 0) Ctrl->Z.mode = MBNAVLIST_SEGMENT_MODE_SWATHFILE;
			else if (strcmp(Ctrl->Z.tag, "datalist") == 0)  Ctrl->Z.mode = MBNAVLIST_SEGMENT_MODE_DATALIST;
			else                                            Ctrl->Z.mode = MBNAVLIST_SEGMENT_MODE_TAG;
			Ctrl->Z.active = true;
			break;
		default:
			n_errors += gmt_default_error(GMT, opt->option);
			break;
		}
	}

	return n_errors ? GMT_PARSE_ERROR : GMT_OK;
}

#define bailout(code)  { gmt_M_free_options(mode); return code; }
#define Return(code)   { Free_mbnavlist_Ctrl(GMT, Ctrl); gmt_end_module(GMT, GMT_cpy); bailout(code); }

/*--------------------------------------------------------------------*/
int GMT_mbnavlist(void *V_API, int mode, void *args) {
	int error = MB_ERROR_NO_ERROR;

	struct MBNAVLIST_CTRL *Ctrl = NULL;
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

	Ctrl = New_mbnavlist_Ctrl(GMT);
	if ((error = parse(GMT, Ctrl, options)) != 0) Return (error);

	int verbose = GMT->common.V.active;
	int format, pings, lonflip;
	double bounds[4];
	int btime_i[7], etime_i[7];
	double speedmin, timegap;
	int status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

	if (Ctrl->F.active) format = Ctrl->F.format;
	if (Ctrl->L.active) lonflip = Ctrl->L.lonflip;
	if (Ctrl->S.active) speedmin = Ctrl->S.speedmin;
	if (Ctrl->T.active) timegap = Ctrl->T.timegap;
	if (Ctrl->R.active) for (int i = 0; i < 4; i++) bounds[i] = Ctrl->R.bounds[i];
	if (Ctrl->B.active) for (int i = 0; i < 7; i++) btime_i[i] = Ctrl->B.t[i];
	if (Ctrl->E.active) for (int i = 0; i < 7; i++) etime_i[i] = Ctrl->E.t[i];

	char read_file[MB_PATH_MAXLINE];
	strcpy(read_file, Ctrl->I.active ? Ctrl->I.inputfile : "datalist.mb-1");

	int  decimate = Ctrl->D.decimate;
	int  data_kind = Ctrl->K.data_kind;
	int  aux_nav_channel = Ctrl->N.aux_nav_channel;
	bool ascii = !Ctrl->A.active;
	bool segment = Ctrl->Z.active;
	char segment_tag[MB_PATH_MAXLINE];
	strcpy(segment_tag, Ctrl->Z.tag);
	segment_mode_t segment_mode = Ctrl->Z.active ? Ctrl->Z.mode : MBNAVLIST_SEGMENT_MODE_NONE;
	bool use_projection = Ctrl->J.active || Ctrl->O.use_projection;
	char projection_pars[MB_PATH_MAXLINE];
	strcpy(projection_pars, Ctrl->J.active ? Ctrl->J.proj : "");
	char *delimiter = Ctrl->G.delim;

	char list[MAX_OPTIONS];
	memcpy(list, Ctrl->O.list, MAX_OPTIONS);
	int n_list = Ctrl->O.n_list;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s>\n", THIS_MODULE_NAME);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Control Parameters:\n");
		fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
		fprintf(stderr, "dbg2       format:         %d\n", format);
		fprintf(stderr, "dbg2       pings:          %d\n", pings);
		fprintf(stderr, "dbg2       lonflip:        %d\n", lonflip);
		fprintf(stderr, "dbg2       decimate:       %d\n", decimate);
		fprintf(stderr, "dbg2       bounds[0]:      %f\n", bounds[0]);
		fprintf(stderr, "dbg2       bounds[1]:      %f\n", bounds[1]);
		fprintf(stderr, "dbg2       bounds[2]:      %f\n", bounds[2]);
		fprintf(stderr, "dbg2       bounds[3]:      %f\n", bounds[3]);
		fprintf(stderr, "dbg2       btime_i[0]:     %d\n", btime_i[0]);
		fprintf(stderr, "dbg2       btime_i[1]:     %d\n", btime_i[1]);
		fprintf(stderr, "dbg2       btime_i[2]:     %d\n", btime_i[2]);
		fprintf(stderr, "dbg2       btime_i[3]:     %d\n", btime_i[3]);
		fprintf(stderr, "dbg2       btime_i[4]:     %d\n", btime_i[4]);
		fprintf(stderr, "dbg2       btime_i[5]:     %d\n", btime_i[5]);
		fprintf(stderr, "dbg2       btime_i[6]:     %d\n", btime_i[6]);
		fprintf(stderr, "dbg2       etime_i[0]:     %d\n", etime_i[0]);
		fprintf(stderr, "dbg2       etime_i[1]:     %d\n", etime_i[1]);
		fprintf(stderr, "dbg2       etime_i[2]:     %d\n", etime_i[2]);
		fprintf(stderr, "dbg2       etime_i[3]:     %d\n", etime_i[3]);
		fprintf(stderr, "dbg2       etime_i[4]:     %d\n", etime_i[4]);
		fprintf(stderr, "dbg2       etime_i[5]:     %d\n", etime_i[5]);
		fprintf(stderr, "dbg2       etime_i[6]:     %d\n", etime_i[6]);
		fprintf(stderr, "dbg2       speedmin:       %f\n", speedmin);
		fprintf(stderr, "dbg2       timegap:        %f\n", timegap);
		fprintf(stderr, "dbg2       aux_nav_channel:%d\n", aux_nav_channel);
		fprintf(stderr, "dbg2       data_kind:      %d\n", data_kind);
		fprintf(stderr, "dbg2       ascii:          %d\n", ascii);
		fprintf(stderr, "dbg2       segment:        %d\n", segment);
		fprintf(stderr, "dbg2       segment_mode:   %d\n", segment_mode);
		fprintf(stderr, "dbg2       segment_tag:    %s\n", segment_tag);
		fprintf(stderr, "dbg2       delimiter:      %s\n", delimiter);
		fprintf(stderr, "dbg2       use_projection: %d\n", use_projection);
		fprintf(stderr, "dbg2       projection_pars:%s\n", projection_pars);
		fprintf(stderr, "dbg2       n_list:         %d\n", n_list);
		for (int i = 0; i < n_list; i++)
			fprintf(stderr, "dbg2         list[%d]:      %c\n", i, list[i]);
	}

	if (format == 0)
		mb_get_format(verbose, read_file, NULL, &format, &error);

	const bool read_datalist = format < 0;
	bool read_data;
	char file[MB_PATH_MAXLINE];
	void *datalist = NULL;
	double file_weight;
	char dfile[MB_PATH_MAXLINE];

	if (read_datalist) {
		const int look_processed = MB_DATALIST_LOOK_UNSET;
		if (mb_datalist_open(verbose, &datalist, read_file, look_processed, &error) != MB_SUCCESS) {
			GMT_Report(API, GMT_MSG_NORMAL, "Unable to open data list file: %s\n", read_file);
			GMT_Report(API, GMT_MSG_NORMAL, "Program <%s> Terminated\n", THIS_MODULE_NAME);
			Return(MB_ERROR_OPEN_FAIL);
		}
		read_data = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error) == MB_SUCCESS;
	} else {
		strcpy(file, read_file);
		read_data = true;
	}

	bool invert_next_value = false;
	bool signflip_next_value = false;

	double btime_d, etime_d;
	int beams_bath, beams_amp, pixels_ss;

	int platform_source, nav_source, heading_source, sensordepth_source, attitude_source, svp_source;

	int time_j[5];
	bool projectednav_next_value = false;

	void *store_ptr;
	int time_i[7];
	double draft, roll, pitch, heave;
	char *beamflag = NULL;
	double *bath = NULL;
	double *bathacrosstrack = NULL;
	double *bathalongtrack = NULL;
	double *amp = NULL;
	double *ss = NULL;
	double *ssacrosstrack = NULL;
	double *ssalongtrack = NULL;
	char comment[MB_COMMENT_MAXLINE];
	int atime_i[7 * MB_ASYNCH_SAVE_MAX];
	double atime_d[MB_ASYNCH_SAVE_MAX];
	double anavlon[MB_ASYNCH_SAVE_MAX];
	double anavlat[MB_ASYNCH_SAVE_MAX];
	double aspeed[MB_ASYNCH_SAVE_MAX];
	double aheading[MB_ASYNCH_SAVE_MAX];
	double adraft[MB_ASYNCH_SAVE_MAX];
	double aroll[MB_ASYNCH_SAVE_MAX];
	double apitch[MB_ASYNCH_SAVE_MAX];
	double aheave[MB_ASYNCH_SAVE_MAX];

	bool first_m = true;
	double time_d_ref = 0.0;
	bool first_u = true;
	time_t time_u;
	time_t time_u_ref = 0;
	double seconds;

	double dlon, dlat, minutes;
	int degrees;
	double mtodeglon, mtodeglat;
	double course;
	double time_d_old = 0.0;
	double time_interval;
	double speed_made_good;
	double navlon_old = 0.0;
	double navlat_old = 0.0;

	int proj_status;
	void *pjptr = NULL;
	double reference_lon, reference_lat;
	double naveasting = 0.0, navnorthing = 0.0;
	double deasting;

	while (read_data) {
		if ((status = mb_format_source(verbose, &format, &platform_source, &nav_source, &sensordepth_source, &heading_source,
		                               &attitude_source, &svp_source, &error)) == MB_FAILURE) {
			char *message;
			mb_error(verbose, error, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "MBIO Error returned from function <mb_format_source>:\n%s\n", message);
			GMT_Report(API, GMT_MSG_NORMAL, "Program <%s> Terminated\n", THIS_MODULE_NAME);
			Return(error);
		}

		if (aux_nav_channel > 0) {
			if (aux_nav_channel == 1)
				nav_source = MB_DATA_NAV1;
			else if (aux_nav_channel == 2)
				nav_source = MB_DATA_NAV2;
			else if (aux_nav_channel == 3)
				nav_source = MB_DATA_NAV3;
		}

		void *mbio_ptr = NULL;
		if (mb_read_init(verbose, file, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &mbio_ptr,
		                 &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
			char *message;
			mb_error(verbose, error, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "MBIO Error returned from function <mb_read_init>:\n%s\n", message);
			GMT_Report(API, GMT_MSG_NORMAL, "Multibeam File <%s> not initialized for reading\n", file);
			GMT_Report(API, GMT_MSG_NORMAL, "Program <%s> Terminated\n", THIS_MODULE_NAME);
			Return(error);
		}

		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, &error);

		if (error != MB_ERROR_NO_ERROR) {
			char *message;
			mb_error(verbose, error, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "MBIO Error allocating data arrays:\n%s\n", message);
			GMT_Report(API, GMT_MSG_NORMAL, "Program <%s> Terminated\n", THIS_MODULE_NAME);
			Return(error);
		}

		if (segment && ascii) {
			if (segment_mode == MBNAVLIST_SEGMENT_MODE_TAG)
				printf("%s\n", segment_tag);
			else if (segment_mode == MBNAVLIST_SEGMENT_MODE_SWATHFILE)
				printf("# %s\n", file);
			else if (segment_mode == MBNAVLIST_SEGMENT_MODE_DATALIST)
				printf("# %s\n", dfile);
		}

		double distance_total = 0.0;
		int nread = 0;
		int nnav = 0;
		bool first = true;
		double course_old = 0.0;
		double speed_made_good_old = 0.0;
		while (error <= MB_ERROR_NO_ERROR) {
			int kind;
			double navlon, navlat, time_d, speed, heading, distance, altitude, sensordepth;
			status = mb_get_all(verbose, mbio_ptr, &store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
			                    &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
			                    bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

			if (error == MB_ERROR_TIME_GAP) {
				error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
			}

			if (error <= MB_ERROR_NO_ERROR) {
				if (data_kind > 0) {
					if (kind == data_kind) {
						error = MB_ERROR_NO_ERROR;
						status = MB_SUCCESS;
					} else {
						error = MB_ERROR_IGNORE;
						status = MB_FAILURE;
					}
				} else {
					if (kind == nav_source) {
						error = MB_ERROR_NO_ERROR;
						status = MB_SUCCESS;
					} else {
						error = MB_ERROR_IGNORE;
						status = MB_FAILURE;
					}
				}
			}

			int n = 0;
			if (error == MB_ERROR_NO_ERROR) {
				status = mb_extract_nnav(verbose, mbio_ptr, store_ptr, MB_ASYNCH_SAVE_MAX, &kind, &n, atime_i, atime_d, anavlon,
				                         anavlat, aspeed, aheading, adraft, aroll, apitch, aheave, &error);
			}

			if (error == MB_ERROR_NO_ERROR)
				nread++;

			if (verbose >= 2) {
				fprintf(stderr, "\ndbg2  Data read in program <%s>\n", THIS_MODULE_NAME);
				fprintf(stderr, "dbg2       kind:           %d\n", kind);
				fprintf(stderr, "dbg2       error:          %d\n", error);
				fprintf(stderr, "dbg2       status:         %d\n", status);
				fprintf(stderr, "dbg2       n:              %d\n", n);
			}

			if (error == MB_ERROR_NO_ERROR && n > 0) {
				for (int inav = 0; inav < n; inav++) {
					for (int j = 0; j < 7; j++)
						time_i[j] = atime_i[inav * 7 + j];
					time_d = atime_d[inav];
					navlon = anavlon[inav];
					navlat = anavlat[inav];
					speed = aspeed[inav];
					heading = aheading[inav];
					draft = adraft[inav];
					roll = aroll[inav];
					pitch = apitch[inav];
					heave = aheave[inav];
					sensordepth = draft - heave;

					mb_coor_scale(verbose, navlat, &mtodeglon, &mtodeglat);
					if (first) {
						time_interval = 0.0;
						course = heading;
						speed_made_good = 0.0;
						distance = 0.0;
					} else {
						time_interval = time_d - time_d_old;
						const double dx = (navlon - navlon_old) / mtodeglon;
						const double dy = (navlat - navlat_old) / mtodeglat;
						distance = sqrt(dx * dx + dy * dy);
						if (distance > 0.0)
							course = RTD * atan2(dx / distance, dy / distance);
						else
							course = course_old;
						if (course < 0.0)
							course = course + 360.0;
						if (time_interval > 0.0)
							speed_made_good = 3.6 * distance / time_interval;
						else
							speed_made_good = speed_made_good_old;
					}
					distance_total += 0.001 * distance;

					if (use_projection) {
						if (pjptr == NULL) {
							if (strlen(projection_pars) == 0)
								strcpy(projection_pars, "U");

							char projection_id[MB_PATH_MAXLINE];
							if (strcmp(projection_pars, "UTM") == 0 || strcmp(projection_pars, "U") == 0 ||
							    strcmp(projection_pars, "utm") == 0 || strcmp(projection_pars, "u") == 0) {
								reference_lon = navlon;
								if (reference_lon < 180.0)
									reference_lon += 360.0;
								if (reference_lon >= 180.0)
									reference_lon -= 360.0;
								const int utm_zone = (int)(((reference_lon + 183.0) / 6.0) + 0.5);
								reference_lat = navlat;
								if (reference_lat >= 0.0)
									snprintf(projection_id, sizeof(projection_id), "UTM%2.2dN", utm_zone);
								else
									snprintf(projection_id, sizeof(projection_id), "UTM%2.2dS", utm_zone);
							} else
								strcpy(projection_id, projection_pars);

							proj_status = mb_proj_init(verbose, projection_id, &pjptr, &error);

							if (proj_status != MB_SUCCESS) {
								GMT_Report(API, GMT_MSG_NORMAL, "Output projection %s not found in database\n", projection_id);
								GMT_Report(API, GMT_MSG_NORMAL, "Program <%s> Terminated\n", THIS_MODULE_NAME);
								mb_memory_clear(verbose, &error);
								Return(MB_ERROR_BAD_PARAMETER);
							}
						}

						mb_proj_forward(verbose, pjptr, navlon, navlat, &naveasting, &navnorthing, &error);
					}

					navlon_old = navlon;
					navlat_old = navlat;
					course_old = course;
					speed_made_good_old = speed_made_good;
					time_d_old = time_d;

					if (nnav % decimate == 0)
						for (int i = 0; i < n_list; i++) {
							switch (list[i]) {
							case '/':
								invert_next_value = true;
								break;
							case '-':
								signflip_next_value = true;
								break;
							case 'c':
								printsimplevalue(verbose, sensordepth, 0, 4, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'H':
								printsimplevalue(verbose, heading, 7, 3, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'h':
								printsimplevalue(verbose, course, 7, 3, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'J':
								mb_get_jtime(verbose, time_i, time_j);
								seconds = time_i[5] + 0.000001 * time_i[6];
								if (ascii) {
									printf("%.4d %.3d %.2d %.2d %9.6f", time_j[0], time_j[1], time_i[3], time_i[4], seconds);
								} else {
									double b = time_j[0];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_j[1];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_i[3];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_i[4];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_i[5];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_i[6];
									fwrite(&b, sizeof(double), 1, stdout);
								}
								break;
							case 'j':
								mb_get_jtime(verbose, time_i, time_j);
								seconds = time_i[5] + 0.000001 * time_i[6];
								if (ascii) {
									printf("%.4d %.3d %.4d %9.6f", time_j[0], time_j[1], time_j[2], seconds);
								} else {
									double b = time_j[0];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_j[1];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_j[2];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_j[3];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_j[4];
									fwrite(&b, sizeof(double), 1, stdout);
								}
								break;
							case 'L':
								printsimplevalue(verbose, distance_total, 8, 4, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'l':
								printsimplevalue(verbose, 1000.0 * distance_total, 8, 4, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'M':
								printsimplevalue(verbose, time_d, 0, 6, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'm': {
								if (first_m) {
									time_d_ref = time_d;
									first_m = false;
								}
								double b = time_d - time_d_ref;
								printsimplevalue(verbose, b, 0, 6, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							}
							case 'P':
								printsimplevalue(verbose, pitch, 6, 3, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'p':
								printsimplevalue(verbose, draft, 7, 4, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'R':
								printsimplevalue(verbose, roll, 6, 3, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'r':
								printsimplevalue(verbose, heave, 7, 4, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'S':
								printsimplevalue(verbose, speed, 6, 3, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 's':
								printsimplevalue(verbose, speed_made_good, 6, 3, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'T':
								seconds = time_i[5] + 1e-6 * time_i[6];
								if (ascii) {
									printf("%.4d/%.2d/%.2d/%.2d/%.2d/%09.6f", time_i[0], time_i[1], time_i[2], time_i[3],
									       time_i[4], seconds);
								} else {
									double b = time_i[0];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_i[1];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_i[2];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_i[3];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_i[4];
									fwrite(&b, sizeof(double), 1, stdout);
									b = seconds;
									fwrite(&b, sizeof(double), 1, stdout);
								}
								break;
							case 't':
								seconds = time_i[5] + 1e-6 * time_i[6];
								if (ascii) {
									printf("%.4d %.2d %.2d %.2d %.2d %09.6f", time_i[0], time_i[1], time_i[2], time_i[3],
									       time_i[4], seconds);
								} else {
									double b = time_i[0];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_i[1];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_i[2];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_i[3];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_i[4];
									fwrite(&b, sizeof(double), 1, stdout);
									b = seconds;
									fwrite(&b, sizeof(double), 1, stdout);
								}
								break;
							case 'U':
								time_u = (time_t)time_d;
								if (ascii) {
									printf("%lld", (long long)time_u);
								} else {
									double b = (double)time_u;
									fwrite(&b, sizeof(double), 1, stdout);
								}
								break;
							case 'u':
								time_u = (time_t)time_d;
								if (first_u) {
									time_u_ref = time_u;
									first_u = false;
								}
								if (ascii) {
									printf("%lld", (long long)(time_u - time_u_ref));
								} else {
									double b = (double)(time_u - time_u_ref);
									fwrite(&b, sizeof(double), 1, stdout);
								}
								break;
							case 'V':
							case 'v':
								if (ascii) {
									if (fabs(time_interval) > 100.0)
										printf("%g", time_interval);
									else
										printf("%7.3f", time_interval);
								} else {
									fwrite(&time_interval, sizeof(double), 1, stdout);
								}
								break;
							case 'X':
								if (!projectednav_next_value) {
									dlon = navlon;
									printsimplevalue(verbose, dlon, 15, 10, ascii, &invert_next_value, &signflip_next_value, &error);
								} else {
									deasting = naveasting;
									printsimplevalue(verbose, deasting, 15, 3, ascii, &invert_next_value, &signflip_next_value, &error);
								}
								projectednav_next_value = false;
								break;
							case 'x': {
								dlon = navlon;
								char hemi;
								if (dlon < 0.0) {
									hemi = 'W';
									dlon = -dlon;
								} else
									hemi = 'E';
								degrees = (int)dlon;
								minutes = 60.0 * (dlon - degrees);
								if (ascii) {
									printf("%3d %11.8f%c", degrees, minutes, hemi);
								} else {
									double b = degrees;
									if (hemi == 'W')
										b = -b;
									fwrite(&b, sizeof(double), 1, stdout);
									b = minutes;
									fwrite(&b, sizeof(double), 1, stdout);
								}
								break;
							}
							case 'Y':
								if (!projectednav_next_value) {
									dlat = navlat;
									printsimplevalue(verbose, dlat, 15, 10, ascii, &invert_next_value, &signflip_next_value, &error);
								} else {
									const double dnorthing = navnorthing;
									printsimplevalue(verbose, dnorthing, 15, 3, ascii, &invert_next_value, &signflip_next_value, &error);
								}
								projectednav_next_value = false;
								break;
							case 'y': {
								dlat = navlat;
								char hemi;
								if (dlat < 0.0) {
									hemi = 'S';
									dlat = -dlat;
								} else
									hemi = 'N';
								degrees = (int)dlat;
								minutes = 60.0 * (dlat - degrees);
								if (ascii) {
									printf("%3d %11.8f%c", degrees, minutes, hemi);
								} else {
									double b = degrees;
									if (hemi == 'S')
										b = -b;
									fwrite(&b, sizeof(double), 1, stdout);
									b = minutes;
									fwrite(&b, sizeof(double), 1, stdout);
								}
								break;
							}
							default:
								if (ascii)
									printf("<Invalid Option: %c>", list[i]);
								break;
							}
							if (ascii) {
								if (i < (n_list - 1))
									printf("%s", delimiter);
								else
									printf("\n");
							}
						}
					nnav++;
					first = false;
				}
			}
		}

		status &= mb_close(verbose, &mbio_ptr, &error);

		if (read_datalist)
			read_data = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error) == MB_SUCCESS;
		else
			read_data = false;
	}

	if (read_datalist)
		mb_datalist_close(verbose, &datalist, &error);

	if (use_projection && pjptr != NULL)
		mb_proj_free(verbose, &pjptr, &error);

	if (verbose >= 4)
		status = mb_memory_list(verbose, &error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s> completed\n", THIS_MODULE_NAME);
		fprintf(stderr, "dbg2  Ending status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	Return(error);
}
/*--------------------------------------------------------------------*/
