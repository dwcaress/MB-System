/*--------------------------------------------------------------------
 *    The MB-system:  mbfilter.c   (GMT-module rewrite of mbfilter.cc)
 *
 *    Original mbfilter.c (1995) by D. W. Caress.
 *    Re-converted from current upstream src/utilities/mbfilter.cc back
 *    to C and wrapped as a GMT module entry so it can be invoked from
 *    the GMT API (and therefore from Julia FFI / Matlab MEX via GMT).
 *
 *    Copyright (c) 1995-2026 by
 *    David W. Caress (caress@mbari.org)  — MBARI
 *    Dale N. Chayes  — University of New Hampshire CCOM
 *    Christian dos Santos Ferreira  — MARUM
 *
 *    See README.md for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbfilter applies one or more simple filters (boxcar/gaussian/median
 * mean, inverse gradient, edge / gradient contrast enhancement) to
 * bathymetry, beam amplitude, or sidescan data and writes the result
 * to .ffb/.ffa/.ffs companion files.
 */

#define THIS_MODULE_NAME    "mbfilter"
#define THIS_MODULE_LIB     "mbsystem"
#define THIS_MODULE_PURPOSE "Apply smoothing, hipass or contrast filters to swath sonar data"
#define THIS_MODULE_KEYS    ""
#define THIS_MODULE_NEEDS		""
#define THIS_MODULE_OPTIONS		"->V"

#include "gmt_dev.h"

#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_io.h"
#include "mbsys_ldeoih.h"

/* --- mode enums ------------------------------------------------------ */

typedef enum {
	MBFILTER_BATH = 0,
	MBFILTER_AMP  = 1,
	MBFILTER_SS   = 2
} filter_kind_t;

typedef enum {
	MBFILTER_HIPASS_NONE = 0,
	MBFILTER_HIPASS_MEAN = 1,
	MBFILTER_HIPASS_GAUSSIAN = 2,
	MBFILTER_HIPASS_MEDIAN = 3
} hipass_mode_t;

typedef enum {
	MBFILTER_SMOOTH_NONE = 0,
	MBFILTER_SMOOTH_MEAN = 1,
	MBFILTER_SMOOTH_GAUSSIAN = 2,
	MBFILTER_SMOOTH_MEDIAN = 3,
	MBFILTER_SMOOTH_GRADIENT = 4
} smooth_mode_t;

typedef enum {
	MBFILTER_CONTRAST_NONE = 0,
	MBFILTER_CONTRAST_EDGE = 1,
	MBFILTER_CONTRAST_GRADIENT = 2
} contrast_mode_t;

typedef enum {
	MBFILTER_A_NONE = 0,
	MBFILTER_A_HIPASS_MEAN = 1,
	MBFILTER_A_HIPASS_GAUSSIAN = 2,
	MBFILTER_A_HIPASS_MEDIAN = 3,
	MBFILTER_A_SMOOTH_MEAN = 4,
	MBFILTER_A_SMOOTH_GAUSSIAN = 5,
	MBFILTER_A_SMOOTH_MEDIAN = 6,
	MBFILTER_A_SMOOTH_GRADIENT = 7,
	MBFILTER_A_CONTRAST_EDGE = 8,
	MBFILTER_A_CONTRAST_GRADIENT = 9
} filter_a_mode_t;

#define MBFILTER_BUFFER_DEFAULT 5000
#define MBFILTER_NFILTER_MAX    10

struct mbfilter_ping_struct {
	int    time_i[7];
	double time_d;
	double navlon;
	double navlat;
	double speed;
	double heading;
	double distance;
	double altitude;
	double sensordepth;
	double roll;
	double pitch;
	double heave;
	int    beams_bath;
	int    beams_amp;
	int    pixels_ss;
	char   *beamflag;
	double *bath;
	double *bathacrosstrack;
	double *bathalongtrack;
	double *amp;
	char   *pixelflag;
	double *ss;
	double *ssacrosstrack;
	double *ssalongtrack;
	double *dataprocess;
	double *datasave;
	int    ndatapts;
	double *data_i_ptr;
	double *data_f_ptr;
	char   *flag_ptr;
};

struct mbfilter_filter_struct {
	filter_a_mode_t mode;
	int    xdim;
	int    ldim;
	int    iteration;
	bool   threshold;
	double threshold_lo;
	double threshold_hi;
	double hipass_offset;
};

EXTERN_MSC int GMT_mbfilter(void *API, int mode, void *args);

/* --- Control structure ----------------------------------------------- */

struct MBFILTER_CTRL {
	struct mbf_A { bool active; int datakind; } A;
	struct mbf_B { bool active; int t[7]; } B;
	struct mbf_C { bool active; int mode, xdim, ldim, iter; } C;
	struct mbf_D { bool active; int mode, xdim, ldim, iter; double offset; } D;
	struct mbf_E { bool active; int t[7]; } E;
	struct mbf_F { bool active; int format; } F;
	struct mbf_I { bool active; char *inputfile; } I;
	struct mbf_N { bool active; int n_buffer_max; } N;
	struct mbf_R { bool active; double bounds[4]; } R;
	struct mbf_S { bool active; int mode, xdim, ldim, iter;
	               bool threshold; double threshold_lo, threshold_hi; } S;
	struct mbf_T { bool active; double threshold_lo, threshold_hi; } T;
};

/* --- forward decls --------------------------------------------------- */

static int hipass_mean(int verbose, int n, const double *val, double *wgt, double *hipass);
static int hipass_gaussian(int verbose, int n, double *val, double *wgt, double *dis, double *hipass);
static int hipass_median(int verbose, int n, double *val, double *wgt, double *hipass);
static int smooth_mean(int verbose, int n, double *val, double *wgt, double *smooth);
static int smooth_gaussian(int verbose, int n, double *val, double *wgt, double *dis, double *smooth);
static int smooth_median(int verbose, double original, bool apply_threshold,
                         double threshold_lo, double threshold_hi,
                         int n, double *val, double *wgt, double *smooth);
static int smooth_gradient(int verbose, int n, double *val, double *wgt, double *smooth);
static int contrast_edge(int verbose, int n, double *val, double *grad, double *result);
static int contrast_gradient(int verbose, int n, double *val, double *wgt, double *result);
static int mbcopy_any_to_mbldeoih(int verbose, int system, int kind, int *time_i, double time_d,
                                  double navlon, double navlat, double speed, double heading,
                                  double draft, double altitude, double roll, double pitch, double heave,
                                  double beamwidth_xtrack, double beamwidth_ltrack,
                                  int nbath, int namp, int nss, char *beamflag, double *bath, double *amp,
                                  double *bathacrosstrack, double *bathalongtrack,
                                  double *ss, double *ssacrosstrack, double *ssalongtrack,
                                  char *comment, char *ombio_ptr, char *ostore_ptr, int *error);

/* --- ctor / dtor / usage --------------------------------------------- */

static void *New_mbfilter_Ctrl(struct GMT_CTRL *GMT) {
	struct MBFILTER_CTRL *Ctrl = gmt_M_memory(GMT, NULL, 1, struct MBFILTER_CTRL);
	Ctrl->A.datakind     = MBFILTER_SS;
	Ctrl->N.n_buffer_max = MBFILTER_BUFFER_DEFAULT;
	Ctrl->D.offset       = 1000.0;
	return Ctrl;
}

static void Free_mbfilter_Ctrl(struct GMT_CTRL *GMT, struct MBFILTER_CTRL *Ctrl) {
	if (!Ctrl) return;
	if (Ctrl->I.inputfile) free(Ctrl->I.inputfile);
	gmt_M_free(GMT, Ctrl);
}

static int usage(struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose(API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return GMT_NOERROR;

	GMT_Message(API, GMT_TIME_NONE,
	    "usage: mbfilter [-Akind -Byr/mo/da/hr/mn/sc\n"
	    "\t-Cmode/xdim/ldim/iter -Dmode/xdim/ldim/iter/offset\n"
	    "\t-Eyr/mo/da/hr/mn/sc -Fformat -Iinfile -Nbuffer\n"
	    "\t-Rwest/east/south/north -Smode/xdim/ldim/iter\n"
	    "\t-Tthreshold_lo/threshold_hi -V -H]\n\n");
	if (level == GMT_SYNOPSIS) return GMT_PARSE_ERROR;
	GMT_Message(API, GMT_TIME_NONE,
	    "\t<inputfile> is an MB-System datalist or single swath file.\n\n");
	return GMT_PARSE_ERROR;
}

/* --- parse ----------------------------------------------------------- */

static int parse(struct GMT_CTRL *GMT, struct MBFILTER_CTRL *Ctrl, struct GMT_OPTION *options) {
	unsigned int n_errors = 0, n_files = 0;
	int n;
	struct GMT_OPTION *opt;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {
		case '<':
			Ctrl->I.active = true;
			if (gmt_check_filearg(GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) {
				Ctrl->I.inputfile = strdup(opt->arg);
				n_files = 1;
			} else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error: only one input file is allowed.\n"); n_errors++; }
			break;

		case 'A': {
			int tmp = MBFILTER_SS;
			sscanf(opt->arg, "%d", &tmp);
			if (tmp != MBFILTER_SS && tmp != MBFILTER_AMP) tmp = MBFILTER_SS;
			Ctrl->A.datakind = tmp;
			Ctrl->A.active = true;
			break;
		}

		case 'B':
			Ctrl->B.t[6] = 0;
			n = sscanf(opt->arg, "%d/%d/%d/%d/%d/%d",
			           &Ctrl->B.t[0], &Ctrl->B.t[1], &Ctrl->B.t[2],
			           &Ctrl->B.t[3], &Ctrl->B.t[4], &Ctrl->B.t[5]);
			if (n == 6) Ctrl->B.active = true;
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -B option\n"); n_errors++; }
			break;

		case 'C':
			n = sscanf(opt->arg, "%d/%d/%d/%d",
			           &Ctrl->C.mode, &Ctrl->C.xdim, &Ctrl->C.ldim, &Ctrl->C.iter);
			if (n >= 3) { Ctrl->C.active = true; if (n < 4) Ctrl->C.iter = 1; }
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -C option\n"); n_errors++; }
			break;

		case 'D':
			n = sscanf(opt->arg, "%d/%d/%d/%d/%lf",
			           &Ctrl->D.mode, &Ctrl->D.xdim, &Ctrl->D.ldim, &Ctrl->D.iter, &Ctrl->D.offset);
			if (n >= 3) {
				Ctrl->D.active = true;
				if (n < 4) Ctrl->D.iter = 1;
				if (n < 5) Ctrl->D.offset = 1000.0;
			}
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -D option\n"); n_errors++; }
			break;

		case 'E':
			Ctrl->E.t[6] = 0;
			n = sscanf(opt->arg, "%d/%d/%d/%d/%d/%d", &Ctrl->E.t[0], &Ctrl->E.t[1], &Ctrl->E.t[2],
			           &Ctrl->E.t[3], &Ctrl->E.t[4], &Ctrl->E.t[5]);
			if (n == 6) Ctrl->E.active = true;
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -E option\n"); n_errors++; }
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
			n = sscanf(opt->arg, "%d", &Ctrl->N.n_buffer_max);
			if (n > 0) {
				Ctrl->N.active = true;
				if (Ctrl->N.n_buffer_max > MBFILTER_BUFFER_DEFAULT || Ctrl->N.n_buffer_max < 10)
					Ctrl->N.n_buffer_max = MBFILTER_BUFFER_DEFAULT;
			}
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -N option\n"); n_errors++; }
			break;

		case 'R':
			mb_get_bounds(opt->arg, Ctrl->R.bounds);
			Ctrl->R.active = true;
			break;

		case 'S':
			n = sscanf(opt->arg, "%d/%d/%d/%d/%lf/%lf", &Ctrl->S.mode, &Ctrl->S.xdim, &Ctrl->S.ldim, &Ctrl->S.iter,
			           &Ctrl->S.threshold_lo, &Ctrl->S.threshold_hi);
			if (n >= 3) {
				Ctrl->S.active = true;
				if (n < 4) Ctrl->S.iter = 1;
				if (n >= 6) Ctrl->S.threshold = true;
			}
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -S option\n"); n_errors++; }
			break;

		case 'T':
			n = sscanf(opt->arg, "%lf/%lf", &Ctrl->T.threshold_lo, &Ctrl->T.threshold_hi);
			if (n == 2) Ctrl->T.active = true;
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -T option\n"); n_errors++; }
			break;

		default:
			n_errors += gmt_default_error(GMT, opt->option);
			break;
		}
	}

	n_errors += gmt_M_check_condition(GMT, n_files != 1, "Syntax error: Must specify one input file\n");

	return n_errors ? GMT_PARSE_ERROR : GMT_OK;
}

#define bailout(code)  { gmt_M_free_options(mode); return (code); }
#define Return(code)   { Free_mbfilter_Ctrl(GMT, Ctrl); gmt_end_module(GMT, GMT_cpy); bailout(code); }

/* ====================================================================
 * GMT_mbfilter — GMT module entry point.
 * ==================================================================== */

int GMT_mbfilter(void *V_API, int mode, void *args) {
	int error = MB_ERROR_NO_ERROR;

	struct MBFILTER_CTRL *Ctrl = NULL;
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
	if ((GMT = gmt_init_module(API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout(API->error);
#else
	GMT = gmt_begin_module(API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy);
#endif
	if (GMT_Parse_Common(API, THIS_MODULE_OPTIONS, options)) Return(API->error);

	Ctrl = New_mbfilter_Ctrl(GMT);
	if ((error = parse(GMT, Ctrl, options)) != 0) Return (error);

	int    verbose = GMT->common.V.active;
	int    format, pings, lonflip;
	double bounds[4];
	int    btime_i[7], etime_i[7];
	double speedmin, timegap;
	int    status = mb_defaults(verbose, &format, &pings, &lonflip, bounds,
	                            btime_i, etime_i, &speedmin, &timegap);

	pings = 1;
	bounds[0] = -360.; bounds[1] = 360.; bounds[2] = -90.; bounds[3] = 90.;
	btime_i[0]=1962; btime_i[1]=2; btime_i[2]=21; btime_i[3]=10; btime_i[4]=30; btime_i[5]=0; btime_i[6]=0;
	etime_i[0]=2062; etime_i[1]=2; etime_i[2]=21; etime_i[3]=10; etime_i[4]=30; etime_i[5]=0; etime_i[6]=0;
	speedmin = 0.0;
	timegap = 1000000000.0;

	if (Ctrl->F.active) format = Ctrl->F.format;
	if (Ctrl->B.active) for (int i = 0; i < 7; i++) btime_i[i] = Ctrl->B.t[i];
	if (Ctrl->E.active) for (int i = 0; i < 7; i++) etime_i[i] = Ctrl->E.t[i];
	if (Ctrl->R.active) for (int i = 0; i < 4; i++) bounds[i] = Ctrl->R.bounds[i];

	mb_path read_file;
	strcpy(read_file, Ctrl->I.active ? Ctrl->I.inputfile : "datalist.mb-1");

	filter_kind_t datakind = (filter_kind_t)Ctrl->A.datakind;
	if (datakind != MBFILTER_BATH && datakind != MBFILTER_AMP) datakind = MBFILTER_SS;

	int n_buffer_max = Ctrl->N.n_buffer_max;
	if (n_buffer_max > MBFILTER_BUFFER_DEFAULT || n_buffer_max < 10) n_buffer_max = MBFILTER_BUFFER_DEFAULT;

	bool   apply_threshold_global = Ctrl->T.active;
	double threshold_lo_global    = Ctrl->T.threshold_lo;
	double threshold_hi_global    = Ctrl->T.threshold_hi;

	/* aggregate filters in the order they were requested (C, D, S like upstream) */
	int num_filters = 0;
	struct mbfilter_filter_struct filters[MBFILTER_NFILTER_MAX];
	hipass_mode_t   hipass_mode   = MBFILTER_HIPASS_NONE;
	smooth_mode_t   smooth_mode   = MBFILTER_SMOOTH_NONE;
	contrast_mode_t contrast_mode_v = MBFILTER_CONTRAST_NONE;
	int hipass_xdim = 10, hipass_ldim = 3, hipass_iter = 1;
	int smooth_xdim = 3,  smooth_ldim = 3, smooth_iter = 1;
	int contrast_xdim = 5, contrast_ldim = 5, contrast_iter = 1;
	double hipass_offset = 1000.0;

	if (Ctrl->C.active && num_filters < MBFILTER_NFILTER_MAX) {
		contrast_mode_v = (contrast_mode_t)Ctrl->C.mode;
		contrast_xdim = Ctrl->C.xdim;
		contrast_ldim = Ctrl->C.ldim;
		contrast_iter = Ctrl->C.iter;
		filters[num_filters].mode = (filter_a_mode_t)(Ctrl->C.mode + 7);
		filters[num_filters].xdim = Ctrl->C.xdim;
		filters[num_filters].ldim = Ctrl->C.ldim;
		filters[num_filters].iteration = Ctrl->C.iter;
		filters[num_filters].threshold = false;
		filters[num_filters].hipass_offset = 0.0;
		num_filters++;
	}
	if (Ctrl->D.active && num_filters < MBFILTER_NFILTER_MAX) {
		hipass_mode = (hipass_mode_t)Ctrl->D.mode;
		hipass_xdim = Ctrl->D.xdim;
		hipass_ldim = Ctrl->D.ldim;
		hipass_iter = Ctrl->D.iter;
		hipass_offset = Ctrl->D.offset;
		filters[num_filters].mode = (filter_a_mode_t)(Ctrl->D.mode + 0);
		filters[num_filters].xdim = Ctrl->D.xdim;
		filters[num_filters].ldim = Ctrl->D.ldim;
		filters[num_filters].iteration = Ctrl->D.iter;
		filters[num_filters].threshold = false;
		filters[num_filters].hipass_offset = Ctrl->D.offset;
		num_filters++;
	}
	if (Ctrl->S.active && num_filters < MBFILTER_NFILTER_MAX) {
		smooth_mode = (smooth_mode_t)Ctrl->S.mode;
		smooth_xdim = Ctrl->S.xdim;
		smooth_ldim = Ctrl->S.ldim;
		smooth_iter = Ctrl->S.iter;
		filters[num_filters].mode = (filter_a_mode_t)(Ctrl->S.mode + 3);
		filters[num_filters].xdim = Ctrl->S.xdim;
		filters[num_filters].ldim = Ctrl->S.ldim;
		filters[num_filters].iteration = Ctrl->S.iter;
		filters[num_filters].hipass_offset = 0.0;
		if (Ctrl->S.threshold) {
			filters[num_filters].threshold = true;
			filters[num_filters].threshold_lo = Ctrl->S.threshold_lo;
			filters[num_filters].threshold_hi = Ctrl->S.threshold_hi;
		} else if (apply_threshold_global) {
			filters[num_filters].threshold = true;
			filters[num_filters].threshold_lo = threshold_lo_global;
			filters[num_filters].threshold_hi = threshold_hi_global;
		} else {
			filters[num_filters].threshold = false;
		}
		num_filters++;
	}

	if (format == 0) mb_get_format(verbose, read_file, NULL, &format, &error);

	if (verbose > 0) {
		if (datakind == MBFILTER_BATH) GMT_Report(API, GMT_MSG_NORMAL, "\nProcessing bathymetry data...\n");
		else if (datakind == MBFILTER_AMP) GMT_Report(API, GMT_MSG_NORMAL, "\nProcessing beam amplitude data...\n");
		else GMT_Report(API, GMT_MSG_NORMAL, "\nProcessing sidescan data...\n");
		GMT_Report(API, GMT_MSG_NORMAL, "Number of filters to be applied: %d\n", num_filters);
	}

	const bool read_datalist = (format < 0);
	bool   read_data;
	void  *datalist = NULL;
	char   file[MB_PATH_MAXLINE];
	char   dfile[MB_PATH_MAXLINE];
	double file_weight;

	if (read_datalist) {
		const int look_processed = MB_DATALIST_LOOK_UNSET;
		if (mb_datalist_open(verbose, &datalist, read_file, look_processed, &error) != MB_SUCCESS) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open data list file: %s\n", read_file);
			Return(MB_ERROR_OPEN_FAIL);
		}
		read_data = (mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error) == MB_SUCCESS);
	} else {
		strcpy(file, read_file);
		read_data = true;
	}

	int    system;
	double btime_d, etime_d;
	int    beams_bath, beams_amp, pixels_ss;
	int    obeams_bath, obeams_amp, opixels_ss;
	void  *imbio_ptr = NULL, *ombio_ptr = NULL;
	void  *store_ptr;
	int    kind;
	char   comment[MB_COMMENT_MAXLINE];
	int    nreadtot = 0, nwritetot = 0;

	struct mbfilter_ping_struct *ping = NULL;
	double *weights = NULL, *values = NULL, *distances = NULL;
	char   ofile[MB_PATH_MAXLINE + 10];

	/* heap-alloc ping buffer (could be 5000 entries × ~150 B = ~750 KB) */
	ping = (struct mbfilter_ping_struct *)calloc((size_t)n_buffer_max, sizeof(*ping));
	if (!ping) {
		GMT_Report(API, GMT_MSG_NORMAL, "\nFailed to allocate ping buffer (%d entries)\n", n_buffer_max);
		Return(MB_ERROR_MEMORY_FAIL);
	}

	while (read_data) {
		mb_format_system(verbose, &format, &system, &error);
		mb_format_dimensions(verbose, &format, &beams_bath, &beams_amp, &pixels_ss, &error);

		if (mb_read_init(verbose, file, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap,
		                 &imbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
			char *message;
			mb_error(verbose, error, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nmb_read_init failed: %s\nFile: %s\n", message, file);
			free(ping);
			Return(error);
		}
		struct mb_io_struct *imb_io_ptr = (struct mb_io_struct *)imbio_ptr;

		if (datakind == MBFILTER_BATH)      snprintf(ofile, sizeof(ofile), "%s.ffb", file);
		else if (datakind == MBFILTER_AMP)  snprintf(ofile, sizeof(ofile), "%s.ffa", file);
		else                                 snprintf(ofile, sizeof(ofile), "%s.ffs", file);

		if (mb_write_init(verbose, ofile, 71, &ombio_ptr, &obeams_bath, &obeams_amp, &opixels_ss, &error) != MB_SUCCESS) {
			char *message;
			mb_error(verbose, error, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nmb_write_init failed: %s\nFile: %s\n", message, ofile);
			free(ping);
			Return(error);
		}
		struct mb_io_struct *omb_io_ptr = (struct mb_io_struct *)ombio_ptr;

		for (int i = 0; i < n_buffer_max; i++) {
			ping[i].beamflag = NULL; ping[i].bath = NULL; ping[i].amp = NULL;
			ping[i].bathacrosstrack = NULL; ping[i].bathalongtrack = NULL;
			ping[i].pixelflag = NULL; ping[i].ss = NULL;
			ping[i].ssacrosstrack = NULL; ping[i].ssalongtrack = NULL;
			ping[i].dataprocess = NULL; ping[i].datasave = NULL;
			ping[i].data_i_ptr = NULL; ping[i].data_f_ptr = NULL; ping[i].flag_ptr = NULL;

			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&ping[i].beamflag, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&ping[i].bath, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&ping[i].amp, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&ping[i].bathacrosstrack, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&ping[i].bathalongtrack, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(char), (void **)&ping[i].pixelflag, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ping[i].ss, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ping[i].ssacrosstrack, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ping[i].ssalongtrack, &error);

			if (datakind == MBFILTER_BATH) {
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&ping[i].dataprocess, &error);
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&ping[i].datasave, &error);
			} else if (datakind == MBFILTER_AMP) {
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&ping[i].dataprocess, &error);
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&ping[i].datasave, &error);
			} else {
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ping[i].dataprocess, &error);
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ping[i].datasave, &error);
			}
		}

		int nhold_ping = 1, nweightmax = 1;
		for (int i = 0; i < num_filters; i++) {
			if (filters[i].ldim > nhold_ping) nhold_ping = filters[i].ldim;
			const int wm = filters[i].xdim * filters[i].ldim;
			if (wm > nweightmax) nweightmax = wm;
		}

		if (error == MB_ERROR_NO_ERROR)
			mb_mallocd(verbose, __FILE__, __LINE__, nweightmax * sizeof(double), (void **)&weights,   &error);
		if (error == MB_ERROR_NO_ERROR)
			mb_mallocd(verbose, __FILE__, __LINE__, nweightmax * sizeof(double), (void **)&values,    &error);
		if (error == MB_ERROR_NO_ERROR)
			mb_mallocd(verbose, __FILE__, __LINE__, nweightmax * sizeof(double), (void **)&distances, &error);
		if (error != MB_ERROR_NO_ERROR) {
			char *message;
			mb_error(verbose, error, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nFailed allocating filter scratch: %s\n", message);
			free(ping);
			Return(error);
		}

		kind = MB_DATA_COMMENT;
		snprintf(comment, sizeof(comment), "Data filtered by program %s", THIS_MODULE_NAME);
		status = mb_put_comment(verbose, ombio_ptr, comment, &error);
		snprintf(comment, sizeof(comment), "MB-system Version %s", MB_VERSION);
		status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		{
			char user[256], host[256], date[32];
			mb_user_host_date(verbose, user, host, date, &error);
			snprintf(comment, sizeof(comment), "Run by user <%s> on cpu <%s> at <%s>", user, host, date);
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		}
		if (datakind == MBFILTER_BATH)      snprintf(comment, sizeof(comment), "Processing bathymetry data...");
		else if (datakind == MBFILTER_AMP)  snprintf(comment, sizeof(comment), "Processing beam amplitude data...");
		else                                 snprintf(comment, sizeof(comment), "Processing sidescan data...");
		status &= mb_put_comment(verbose, ombio_ptr, comment, &error);

		if (hipass_mode != MBFILTER_HIPASS_NONE) {
			if      (hipass_mode == MBFILTER_HIPASS_MEAN)     snprintf(comment, sizeof(comment), "applying mean subtraction filter for hipass");
			else if (hipass_mode == MBFILTER_HIPASS_GAUSSIAN) snprintf(comment, sizeof(comment), "applying gaussian mean subtraction filter for hipass");
			else                                              snprintf(comment, sizeof(comment), "applying median subtraction filter for hipass");
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
			snprintf(comment, sizeof(comment), "  filter acrosstrack dimension: %d", hipass_xdim); status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
			snprintf(comment, sizeof(comment), "  filter alongtrack dimension:  %d", hipass_ldim); status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
			snprintf(comment, sizeof(comment), "  filter iterations:            %d", hipass_iter); status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
			snprintf(comment, sizeof(comment), "  filter offset:                %f", hipass_offset); status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		}
		if (smooth_mode != MBFILTER_SMOOTH_NONE) {
			if      (smooth_mode == MBFILTER_SMOOTH_MEAN)     snprintf(comment, sizeof(comment), "applying mean filter for smoothing");
			else if (smooth_mode == MBFILTER_SMOOTH_GAUSSIAN) snprintf(comment, sizeof(comment), "applying gaussian mean filter for smoothing");
			else if (smooth_mode == MBFILTER_SMOOTH_MEDIAN)   snprintf(comment, sizeof(comment), "applying median filter for smoothing");
			else                                              snprintf(comment, sizeof(comment), "applying inverse gradient filter for smoothing");
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
			if (smooth_mode == MBFILTER_SMOOTH_MEDIAN && apply_threshold_global) {
				snprintf(comment, sizeof(comment), "  filter low ratio threshold:   %f", threshold_lo_global);  status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
				snprintf(comment, sizeof(comment), "  filter high ratio threshold:  %f", threshold_hi_global);  status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
			}
			snprintf(comment, sizeof(comment), "  filter acrosstrack dimension: %d", smooth_xdim); status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
			snprintf(comment, sizeof(comment), "  filter alongtrack dimension:  %d", smooth_ldim); status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
			snprintf(comment, sizeof(comment), "  filter iterations:            %d", smooth_iter); status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		}
		if (contrast_mode_v != MBFILTER_CONTRAST_NONE) {
			if (contrast_mode_v == MBFILTER_CONTRAST_EDGE)
				snprintf(comment, sizeof(comment), "applying edge detection filter for contrast enhancement");
			else
				snprintf(comment, sizeof(comment), "applying gradient subtraction filter for contrast enhancement");
			status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
			snprintf(comment, sizeof(comment), "  filter acrosstrack dimension: %d", contrast_xdim); status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
			snprintf(comment, sizeof(comment), "  filter alongtrack dimension:  %d", contrast_ldim); status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
			snprintf(comment, sizeof(comment), "  filter iterations:            %d", contrast_iter); status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		}
		snprintf(comment, sizeof(comment), "Control Parameters:");  status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		snprintf(comment, sizeof(comment), "  MBIO data format:   %d", format);  status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		snprintf(comment, sizeof(comment), "  Input file:         %s", file);    status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		snprintf(comment, sizeof(comment), "  Output file:        %s", ofile);   status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		snprintf(comment, sizeof(comment), "  Longitude flip:     %d", lonflip); status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		snprintf(comment, sizeof(comment), "  Data kind:          %d", datakind); status &= mb_put_comment(verbose, ombio_ptr, comment, &error);
		snprintf(comment, sizeof(comment), " ");                                 status &= mb_put_comment(verbose, ombio_ptr, comment, &error);

		bool first = true;
		int  ndata = 0;
		int  nhold = 0;
		int  nread = 0;
		int  nwrite = 0;
		bool done = false;
		if (status != MB_SUCCESS) done = true;

		while (!done) {
			error = MB_ERROR_NO_ERROR;
			int nload = 0, nunload = 0;
			while (status == MB_SUCCESS && ndata < n_buffer_max) {
				status = mb_get_all(verbose, imbio_ptr, &store_ptr, &kind, ping[ndata].time_i, &ping[ndata].time_d,
				                    &ping[ndata].navlon, &ping[ndata].navlat, &ping[ndata].speed, &ping[ndata].heading,
				                    &ping[ndata].distance, &ping[ndata].altitude, &ping[ndata].sensordepth,
				                    &ping[ndata].beams_bath, &ping[ndata].beams_amp, &ping[ndata].pixels_ss, ping[ndata].beamflag,
				                    ping[ndata].bath, ping[ndata].amp, ping[ndata].bathacrosstrack, ping[ndata].bathalongtrack,
				                    ping[ndata].ss, ping[ndata].ssacrosstrack, ping[ndata].ssalongtrack, comment, &error);
				if (status == MB_SUCCESS && kind == MB_DATA_DATA) {
					if (datakind == MBFILTER_SS) {
						for (int i = 0; i < ping[ndata].pixels_ss; i++)
							ping[ndata].pixelflag[i] = (ping[ndata].ss[i] > MB_SIDESCAN_NULL) ? MB_FLAG_NONE : MB_FLAG_NULL;
					}
					status = mb_extract_nav(verbose, imbio_ptr, store_ptr, &kind, ping[ndata].time_i, &ping[ndata].time_d,
					                        &ping[ndata].navlon, &ping[ndata].navlat, &ping[ndata].speed, &ping[ndata].heading,
					                        &ping[ndata].sensordepth, &ping[ndata].roll, &ping[ndata].pitch, &ping[ndata].heave,
					                        &error);
					status &= mb_extract_altitude(verbose, imbio_ptr, store_ptr, &kind, &ping[ndata].sensordepth,
					                              &ping[ndata].altitude, &error);
				}
				if (status == MB_SUCCESS && kind == MB_DATA_DATA) {
					ndata++; nread++; nreadtot++; nload++;
				}
				if (status == MB_FAILURE && error < 0) {
					status = MB_SUCCESS; error = MB_ERROR_NO_ERROR;
				}
			}
			if (status == MB_FAILURE && error > 0) {
				status = MB_SUCCESS; error = MB_ERROR_NO_ERROR; done = true;
			}

			if (verbose >= 1) {
				GMT_Report(API, GMT_MSG_NORMAL, "%d records loaded into buffer\n", nload);
				GMT_Report(API, GMT_MSG_NORMAL, "%d records held in buffer\n", ndata);
			}

			int jbeg;
			if (first) { jbeg = 0; first = false; }
			else       { jbeg = (nhold / 2 + 1 < ndata) ? (nhold / 2 + 1) : ndata; }

			if      (done)                 nhold = 0;
			else if (ndata > nhold_ping)   nhold = nhold_ping;
			else                            nhold = 0;

			int jend = done ? (ndata - 1) : (ndata - 1 - nhold / 2);
			if (jend < jbeg) jend = jbeg;

			for (int ifilter = 0; ifilter < num_filters; ifilter++) {
				int iteration = 0;
				const int ndx = filters[ifilter].xdim / 2;
				const int ndl = filters[ifilter].ldim / 2;

				while (iteration < filters[ifilter].iteration) {
					for (int j = 0; j < ndata; j++) {
						if (datakind == MBFILTER_BATH) {
							ping[j].ndatapts   = ping[j].beams_bath;
							ping[j].data_i_ptr = ping[j].bath;
							ping[j].flag_ptr   = ping[j].beamflag;
						} else if (datakind == MBFILTER_AMP) {
							ping[j].ndatapts   = ping[j].beams_amp;
							ping[j].data_i_ptr = ping[j].amp;
							ping[j].flag_ptr   = ping[j].beamflag;
						} else {
							ping[j].ndatapts   = ping[j].pixels_ss;
							ping[j].data_i_ptr = ping[j].ss;
							ping[j].flag_ptr   = ping[j].pixelflag;
						}
						ping[j].data_f_ptr = ping[j].dataprocess;
					}

					for (int j = 0; j < ndata; j++) {
						int ja = j - ndl, jb = j + ndl;
						if (ja < 0)       ja = 0;
						if (jb >= ndata)  jb = ndata - 1;

						double *dataptr0 = ping[j].data_i_ptr;
						char   *flagptr0 = ping[j].flag_ptr;
						const int ndatapts = ping[j].ndatapts;

						for (int i = 0; i < ndatapts; i++) {
							int ia = i - ndx, ib = i + ndx;
							if (ia < 0)         ia = 0;
							if (ib >= ndatapts) ib = ndatapts - 1;
							int nweight = 0;

							if (mb_beam_ok(flagptr0[i])) {
								nweight = 1;
								values[0] = dataptr0[i];
								distances[0] = 0.0;

								for (int jj = ja; jj <= jb; jj++) {
									for (int ii = ia; ii <= ib; ii++) {
										if (ii < ping[jj].ndatapts) {
											double *dataptr1 = ping[jj].data_i_ptr;
											char   *flagptr1 = ping[jj].flag_ptr;
											if ((jj != j || ii != i) && mb_beam_ok(flagptr1[ii])) {
												values[nweight] = dataptr1[ii];
												double ddis = 0.0;
												if (ndx > 0) {
													double di = ((double)(ii - i)) / ((double)ndx);
													ddis += di * di;
												}
												if (ndl > 0) {
													double dj = ((double)(jj - j)) / ((double)ndl);
													ddis += dj * dj;
												}
												distances[nweight] = sqrt(ddis);
												nweight++;
											}
										}
									}
								}
							}

							if (nweight > 0) {
								switch (filters[ifilter].mode) {
								case MBFILTER_A_HIPASS_MEAN:
									hipass_mean(verbose, nweight, values, weights, &ping[j].data_f_ptr[i]); break;
								case MBFILTER_A_HIPASS_GAUSSIAN:
									hipass_gaussian(verbose, nweight, values, weights, distances, &ping[j].data_f_ptr[i]); break;
								case MBFILTER_A_HIPASS_MEDIAN:
									hipass_median(verbose, nweight, values, weights, &ping[j].data_f_ptr[i]); break;
								case MBFILTER_A_SMOOTH_MEAN:
									smooth_mean(verbose, nweight, values, weights, &ping[j].data_f_ptr[i]); break;
								case MBFILTER_A_SMOOTH_GAUSSIAN:
									smooth_gaussian(verbose, nweight, values, weights, distances, &ping[j].data_f_ptr[i]); break;
								case MBFILTER_A_SMOOTH_MEDIAN:
									smooth_median(verbose, dataptr0[i], filters[ifilter].threshold,
									              filters[ifilter].threshold_lo, filters[ifilter].threshold_hi,
									              nweight, values, weights, &ping[j].data_f_ptr[i]); break;
								case MBFILTER_A_SMOOTH_GRADIENT:
									smooth_gradient(verbose, nweight, values, weights, &ping[j].data_f_ptr[i]); break;
								case MBFILTER_A_CONTRAST_EDGE:
									contrast_edge(verbose, nweight, values, weights, &ping[j].data_f_ptr[i]); break;
								case MBFILTER_A_CONTRAST_GRADIENT:
									contrast_gradient(verbose, nweight, values, weights, &ping[j].data_f_ptr[i]); break;
								default: break;
								}
							} else {
								ping[j].data_f_ptr[i] = MB_SIDESCAN_NULL;
							}
						}
					}

					if (iteration == filters[ifilter].iteration - 1) {
						for (int j = 0; j < ndata; j++)
							for (int i = 0; i < ping[j].ndatapts; i++)
								ping[j].data_i_ptr[i] = ping[j].data_f_ptr[i] + filters[ifilter].hipass_offset;
					} else {
						for (int j = 0; j < ndata; j++)
							for (int i = 0; i < ping[j].ndatapts; i++)
								ping[j].data_i_ptr[i] = ping[j].data_f_ptr[i];
					}

					if (ndata > 0 && iteration == filters[ifilter].iteration - 1) {
						for (int j = jbeg; j <= jend; j++)
							for (int i = 0; i < ping[j].ndatapts; i++)
								ping[j].datasave[i] = ping[j].data_i_ptr[i];
					}

					iteration++;
				}
			}

			if (ndata > 0) {
				for (int j = jbeg; j <= jend; j++) {
					if (datakind == MBFILTER_BATH) {
						status &= mbcopy_any_to_mbldeoih(
						    verbose, system, MB_DATA_DATA, ping[j].time_i, ping[j].time_d, ping[j].navlon, ping[j].navlat,
						    ping[j].speed, ping[j].heading, ping[j].sensordepth, ping[j].altitude, ping[j].roll, ping[j].pitch,
						    ping[j].heave, imb_io_ptr->beamwidth_xtrack, imb_io_ptr->beamwidth_ltrack, ping[j].beams_bath, 0, 0,
						    ping[j].beamflag, ping[j].datasave, ping[j].amp, ping[j].bathacrosstrack, ping[j].bathalongtrack,
						    ping[j].ss, ping[j].ssacrosstrack, ping[j].ssalongtrack, comment,
						    (char *)ombio_ptr, (char *)omb_io_ptr->store_data, &error);
					} else if (datakind == MBFILTER_AMP) {
						status &= mbcopy_any_to_mbldeoih(
						    verbose, system, MB_DATA_DATA, ping[j].time_i, ping[j].time_d, ping[j].navlon, ping[j].navlat,
						    ping[j].speed, ping[j].heading, ping[j].sensordepth, ping[j].altitude, ping[j].roll, ping[j].pitch,
						    ping[j].heave, imb_io_ptr->beamwidth_xtrack, imb_io_ptr->beamwidth_ltrack, ping[j].beams_bath,
						    ping[j].beams_amp, 0, ping[j].beamflag, ping[j].bath, ping[j].datasave, ping[j].bathacrosstrack,
						    ping[j].bathalongtrack, ping[j].ss, ping[j].ssacrosstrack, ping[j].ssalongtrack, comment,
						    (char *)ombio_ptr, (char *)omb_io_ptr->store_data, &error);
					} else {
						status &= mbcopy_any_to_mbldeoih(
						    verbose, system, MB_DATA_DATA, ping[j].time_i, ping[j].time_d, ping[j].navlon, ping[j].navlat,
						    ping[j].speed, ping[j].heading, ping[j].sensordepth, ping[j].altitude, ping[j].roll, ping[j].pitch,
						    ping[j].heave, imb_io_ptr->beamwidth_xtrack, imb_io_ptr->beamwidth_ltrack, ping[j].beams_bath, 0,
						    ping[j].pixels_ss, ping[j].beamflag, ping[j].bath, ping[j].amp, ping[j].bathacrosstrack,
						    ping[j].bathalongtrack, ping[j].datasave, ping[j].ssacrosstrack, ping[j].ssalongtrack, comment,
						    (char *)ombio_ptr, (char *)omb_io_ptr->store_data, &error);
					}

					status &= mb_write_ping(verbose, ombio_ptr, omb_io_ptr->store_data, &error);
					if (status == MB_SUCCESS) {
						nunload++; nwrite++; nwritetot++;
					}
				}
			}

			if (ndata > nhold) {
				for (int j = 0; j < nhold; j++) {
					const int jj = ndata - nhold + j;
					for (int i = 0; i < 7; i++) ping[j].time_i[i] = ping[jj].time_i[i];
					ping[j].time_d = ping[jj].time_d;
					ping[j].navlon = ping[jj].navlon;
					ping[j].navlat = ping[jj].navlat;
					ping[j].speed = ping[jj].speed;
					ping[j].heading = ping[jj].heading;
					ping[j].distance = ping[jj].distance;
					ping[j].altitude = ping[jj].altitude;
					ping[j].sensordepth = ping[jj].sensordepth;
					ping[j].roll = ping[jj].roll;
					ping[j].pitch = ping[jj].pitch;
					ping[j].heave = ping[jj].heave;
					ping[j].beams_bath = ping[jj].beams_bath;
					ping[j].beams_amp = ping[jj].beams_amp;
					ping[j].pixels_ss = ping[jj].pixels_ss;
					for (int i = 0; i < ping[j].beams_bath; i++) {
						ping[j].beamflag[i] = ping[jj].beamflag[i];
						ping[j].bath[i] = ping[jj].bath[i];
						ping[j].bathacrosstrack[i] = ping[jj].bathacrosstrack[i];
						ping[j].bathalongtrack[i] = ping[jj].bathalongtrack[i];
					}
					for (int i = 0; i < ping[j].beams_amp; i++) ping[j].amp[i] = ping[jj].amp[i];
					for (int i = 0; i < ping[j].pixels_ss; i++) {
						ping[j].pixelflag[i] = ping[jj].pixelflag[i];
						ping[j].ss[i] = ping[jj].ss[i];
						ping[j].ssacrosstrack[i] = ping[jj].ssacrosstrack[i];
						ping[j].ssalongtrack[i] = ping[jj].ssalongtrack[i];
					}
					for (int i = 0; i < ping[jj].ndatapts; i++)
						ping[j].datasave[i] = ping[jj].datasave[i];
				}
				ndata = nhold;
			}

			if (verbose >= 1) {
				GMT_Report(API, GMT_MSG_NORMAL, "\n%d records written from buffer\n", nunload);
				GMT_Report(API, GMT_MSG_NORMAL, "%d records saved in buffer\n", ndata);
			}
		}

		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;
		status = mb_close(verbose, &imbio_ptr, &error);
		status = mb_close(verbose, &ombio_ptr, &error);

		if (verbose >= 1) {
			GMT_Report(API, GMT_MSG_NORMAL, "%d data records read from:  %s\n", nread, file);
			GMT_Report(API, GMT_MSG_NORMAL, "%d data records written to: %s\n", nwrite, ofile);
		}

		mb_freed(verbose, __FILE__, __LINE__, (void **)&weights,   &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&values,    &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&distances, &error);

		if (read_datalist) {
			read_data = (mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error) == MB_SUCCESS);
		} else {
			read_data = false;
		}
	}

	if (read_datalist) mb_datalist_close(verbose, &datalist, &error);

	free(ping);

	if (verbose >= 1) {
		GMT_Report(API, GMT_MSG_NORMAL, "%d total data records read\n", nreadtot);
		GMT_Report(API, GMT_MSG_NORMAL, "%d total data records written\n", nwritetot);
	}

	if (mb_memory_list(verbose, &error) == MB_FAILURE)
		GMT_Report(API, GMT_MSG_NORMAL, "Program %s completed but leaked memory\n", THIS_MODULE_NAME);

	Return(GMT_NOERROR);
}

/* --------------------------------------------------------------------
 * filter helpers (ported verbatim from mbfilter.cc, C cleanup only)
 * -------------------------------------------------------------------- */

static int hipass_mean(int verbose, int n, const double *val, double *wgt, double *hipass) {
	(void)verbose; (void)wgt;
	*hipass = 0.0;
	int nn = 0;
	for (int i = 0; i < n; i++) { *hipass += val[i]; nn++; }
	if (nn > 0) *hipass = val[0] - *hipass / nn;
	return MB_SUCCESS;
}

static int hipass_gaussian(int verbose, int n, double *val, double *wgt, double *dis, double *hipass) {
	(void)verbose;
	*hipass = 0.0;
	double wgtsum = 0.0;
	for (int i = 0; i < n; i++) { wgt[i] = exp(-dis[i] * dis[i]); wgtsum += wgt[i]; }
	if (wgtsum > 0.0) {
		*hipass = 0.0;
		for (int i = 0; i < n; i++) *hipass += wgt[i] * val[i];
		*hipass = val[0] - *hipass / wgtsum;
	}
	return MB_SUCCESS;
}

static int hipass_median(int verbose, int n, double *val, double *wgt, double *hipass) {
	(void)verbose; (void)wgt;
	*hipass = 0.0;
	if (n > 0) {
		qsort((void *)val, n, sizeof(double), mb_double_compare);
		*hipass = val[0] - val[n / 2];
	}
	return MB_SUCCESS;
}

static int smooth_mean(int verbose, int n, double *val, double *wgt, double *smooth) {
	(void)verbose; (void)wgt;
	*smooth = 0.0;
	int nn = 0;
	for (int i = 0; i < n; i++) { *smooth += val[i]; nn++; }
	if (nn > 0) *smooth /= nn;
	return MB_SUCCESS;
}

static int smooth_gaussian(int verbose, int n, double *val, double *wgt, double *dis, double *smooth) {
	(void)verbose;
	*smooth = 0.0;
	double wgtsum = 0.0;
	for (int i = 0; i < n; i++) { wgt[i] = exp(-dis[i] * dis[i]); wgtsum += wgt[i]; }
	if (wgtsum > 0.0) {
		*smooth = 0.0;
		for (int i = 0; i < n; i++) *smooth += wgt[i] * val[i];
		*smooth /= wgtsum;
	}
	return MB_SUCCESS;
}

static int smooth_median(int verbose, double original, bool apply_threshold,
                         double threshold_lo, double threshold_hi,
                         int n, double *val, double *wgt, double *smooth) {
	(void)verbose; (void)wgt;
	*smooth = 0.0;
	if (n > 0) {
		qsort((void *)val, n, sizeof(double), mb_double_compare);
		*smooth = val[n / 2];
	}
	if (apply_threshold && *smooth != 0.0) {
		const double ratio = original / (*smooth);
		if (ratio < threshold_hi && ratio > threshold_lo) *smooth = original;
	}
	return MB_SUCCESS;
}

static int smooth_gradient(int verbose, int n, double *val, double *wgt, double *smooth) {
	(void)verbose;
	*smooth = 0.0;
	double wgtsum = 0.0;
	int nn = 0;
	wgt[0] = 0.5;
	for (int i = 1; i < n; i++) {
		double diff = fabs(val[i] - val[0]);
		if (diff < 0.01) diff = 0.01;
		wgt[i] = 1.0 / diff;
		wgtsum += wgt[i];
		nn++;
	}
	if (nn > 0) {
		*smooth = wgt[0] * val[0];
		for (int i = 1; i < n; i++) *smooth += 0.5 * wgt[i] * val[i] / wgtsum;
	}
	return MB_SUCCESS;
}

static int contrast_edge(int verbose, int n, double *val, double *grad, double *result) {
	(void)verbose;
	double gradsum = 0.0, edge = 0.0;
	for (int i = 0; i < n; i++) {
		grad[i] = 0.0;
		for (int ii = 0; ii < n; ii++) {
			if (val[ii] > 0.0 && i != ii)
				grad[i] += (val[ii] - val[i]) * (val[ii] - val[i]);
		}
		gradsum += grad[i];
		edge += val[i] * grad[i];
	}
	if (gradsum > 0.0) edge /= gradsum;
	const double denom = fabs(val[0] + edge);
	const double contrast = (denom > 0.0) ? pow(fabs(val[0] - edge) / denom, 0.75) : 0.0;
	if (val[0] >= edge) *result = edge * (1.0 + contrast) / ((1.0 - contrast) != 0.0 ? (1.0 - contrast) : 1.0);
	else                *result = edge * (1.0 - contrast) / ((1.0 + contrast) != 0.0 ? (1.0 + contrast) : 1.0);
	return MB_SUCCESS;
}

static int contrast_gradient(int verbose, int n, double *val, double *wgt, double *result) {
	(void)verbose; (void)wgt;
	double gradient = 0.0;
	for (int i = 1; i < n; i++)
		gradient += (val[i] - val[0]) * (val[i] - val[0]);
	gradient = sqrt(gradient);
	*result = val[0] - 2 * gradient;
	return MB_SUCCESS;
}

static int mbcopy_any_to_mbldeoih(int verbose, int system, int kind, int *time_i, double time_d,
                                  double navlon, double navlat, double speed, double heading,
                                  double draft, double altitude, double roll, double pitch, double heave,
                                  double beamwidth_xtrack, double beamwidth_ltrack,
                                  int nbath, int namp, int nss, char *beamflag, double *bath, double *amp,
                                  double *bathacrosstrack, double *bathalongtrack,
                                  double *ss, double *ssacrosstrack, double *ssalongtrack,
                                  char *comment, char *ombio_ptr, char *ostore_ptr, int *error) {
	struct mbsys_ldeoih_struct *ostore = (struct mbsys_ldeoih_struct *)ostore_ptr;
	int status = MB_SUCCESS;
	(void)verbose;

	if (ostore != NULL) {
		ostore->beam_xwidth = beamwidth_xtrack;
		ostore->beam_lwidth = beamwidth_ltrack;
		ostore->ss_type = (system == MB_SYS_SB2100) ? MB_SIDESCAN_LINEAR : MB_SIDESCAN_LOGARITHMIC;
		ostore->kind = kind;

		if (kind == MB_DATA_DATA) {
			mb_insert_altitude(verbose, ombio_ptr, (void *)ostore, draft, altitude, error);
			mb_insert_nav(verbose, ombio_ptr, (void *)ostore, time_i, time_d, navlon, navlat,
			              speed, heading, draft, roll, pitch, heave, error);
		}
		status = mb_insert(verbose, ombio_ptr, (void *)ostore, kind, time_i, time_d, navlon, navlat,
		                   speed, heading, nbath, namp, nss, beamflag, bath, amp,
		                   bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack,
		                   comment, error);
	}
	return status;
}
/* end mbfilter.c */
