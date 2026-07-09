/*--------------------------------------------------------------------
 *    The MB-system:  mbclean.c   (GMT-module rewrite of mbclean.cc)
 *
 *    Original mbclean.c (1993-2015) by D. W. Caress and D. N. Chayes.
 *    GMT-plugin form (mbclean_j.c) by J. Luis 2015.
 *    Re-converted from current upstream src/utilities/mbclean.cc back to
 *    C and wrapped as a GMT module entry so it can be invoked from the
 *    GMT API (and therefore from Julia FFI / Matlab MEX via GMT).
 *
 *    Copyright (c) 1993-2026 by
 *    David W. Caress (caress@mbari.org)  — MBARI
 *    Dale N. Chayes  — University of New Hampshire CCOM
 *    Christian dos Santos Ferreira  — MARUM
 *
 *    See README.md for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbclean identifies and flags artifacts in swath sonar bathymetry data.
 *
 * Flagging algorithm order:
 *   1. Flag specified number of outer beams (-X).
 *   2. Flag beams outside acceptable depth range (-B).
 *   3. Flag beams outside fractional acceptable depth range (-G).
 *   4. Flag beams outside deviation from local median depth (-A).
 *   5. Flag beams associated with excessive slopes (-C, default).
 *   6. Zap "rails" (-Q).
 *   7. Flag pings with too few good soundings (-U).
 *   Plus: -E long acrosstrack, -K min range, -N ping-deviation tolerance,
 *         -P speed range, -R heading rate, -S spike, -T edit-timestamp fix,
 *         -W position bounds, -Y angle/distance flag/unflag, -Z zero-position.
 */

#define THIS_MODULE_NAME    "mbclean"
#define THIS_MODULE_LIB	    "mbsystem"
#define THIS_MODULE_PURPOSE "Identifies and flags artifacts in swath sonar bathymetry data"
#define THIS_MODULE_KEYS    ""
#define THIS_MODULE_NEEDS		""
#define THIS_MODULE_OPTIONS		"-:>Vh"

#include "gmt_dev.h"

#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_io.h"
#include "mb_swap.h"
#include "mb_process.h"

/* --- local defines / enums ------------------------------------------- */

#define MBCLEAN_FLAG_ONE   1
#define MBCLEAN_FLAG_BOTH  2

/* Y-option sub-modes (extended from original -Y two-arg form). */
#define MBCLEAN_Y_MODE_DISTANCE_FLAG   1
#define MBCLEAN_Y_MODE_DISTANCE_UNFLAG 2
#define MBCLEAN_Y_MODE_ANGLE_FLAG      3
#define MBCLEAN_Y_MODE_ANGLE_UNFLAG    4

#define MBCLEAN_NOACTION 0

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

/* --- structs --------------------------------------------------------- */

struct mbclean_ping_struct {
	int    time_i[7];
	double time_d;
	int    multiplicity;
	double navlon;
	double navlat;
	double speed;
	double heading;
	int    beams_bath;
	char  *beamflag;
	char  *beamflagorg;
	double *bath;
	double *bathacrosstrack;
	double *bathalongtrack;
	double *bathx;
	double *bathy;
};

struct bad_struct {
	bool   flag;
	int    ping;
	int    beam;
	double bath;
};

EXTERN_MSC int GMT_mbclean(void *API, int mode, void *args);

/* --- Control structure ----------------------------------------------- */

struct MBCLEAN_CTRL {

	struct mbcln_A { bool active; double deviation_max; } A;
	struct mbcln_B { bool active; double depth_low, depth_high; } B;
	struct mbcln_C { bool active; int slope_form; double slopemax; } C;
	struct mbcln_D { bool active; double distancemin, distancemax; } D;
	struct mbcln_E { bool active; double max_acrosstrack; } E;
	struct mbcln_F { bool active; int format; } F;
	struct mbcln_G { bool active; double fraction_low, fraction_high; } G;
	struct mbcln_I { bool active; char *inputfile; } I;
	struct mbcln_K { bool active; double range_min; } K;
	struct mbcln_L { bool active; int lonflip; } L;
	struct mbcln_M { bool active; int mode; } M;
	struct mbcln_N { bool active; double ping_deviation_tolerance; } N;
	struct mbcln_P { bool active; double speed_low, speed_high; } P;
	struct mbcln_Q { bool active; double backup_dist; } Q;
	struct mbcln_R { bool active; double max_heading_rate; } R;
	struct mbcln_S { bool active; int spike_mode, slope_form; double spikemax; } S;
	struct mbcln_T { bool active; double tolerance; } T;
	struct mbcln_U { bool active; int num_good_min; } U;
	struct mbcln_W { bool active; double west, east, south, north; } W;
	struct mbcln_X { bool active; int zap_beams_left, zap_beams_right; } X;
	struct mbcln_Y {
		bool   active;
		int    y_mode;
		double flag_distance_left, flag_distance_right;
		double unflag_distance_left, unflag_distance_right;
		double flag_angle_left, flag_angle_right;
		double unflag_angle_left, unflag_angle_right;
		bool   flag_distance, unflag_distance, flag_angle, unflag_angle;
	} Y;
	struct mbcln_Z { bool active; } Z;

	/* Aggregated "did the user request this check?" flags, set during parse. */
	struct mbcln_tranf {
		bool check_deviation;
		bool check_range;
		bool check_slope;
		bool zap_long_across;
		bool check_fraction;
		bool check_range_min;
		bool check_speed_good;
		bool zap_rails;
		bool zap_max_heading_rate;
		bool check_spike;
		bool fix_edit_timestamps;
		bool check_position_bounds;
		bool zap_beams;
		bool check_zero_position;
		bool check_num_good_min;
		bool check_ping_deviation;
	} transf;
};

/* --- forward decls --------------------------------------------------- */

static int mbclean_save_edit(int verbose, FILE *sofp, double time_d,
							 int beam, int action, int *error);

/* --- ctor / dtor / usage --------------------------------------------- */

static void *New_mbclean_Ctrl(struct GMT_CTRL *GMT) {
	struct MBCLEAN_CTRL *Ctrl = gmt_M_memory(GMT, NULL, 1, struct MBCLEAN_CTRL);

	/* Defaults */
	Ctrl->C.slopemax        = 1.0;
	Ctrl->D.distancemin     = 0.01;
	Ctrl->D.distancemax     = 0.25;
	Ctrl->E.max_acrosstrack = 120.0;
	Ctrl->F.format          = 0;
	Ctrl->M.mode            = MBCLEAN_FLAG_ONE;
	Ctrl->N.ping_deviation_tolerance = 1.0;
	Ctrl->S.spikemax        = 1.0;
	Ctrl->S.spike_mode      = 1;

	return Ctrl;
}

static void Free_mbclean_Ctrl(struct GMT_CTRL *GMT, struct MBCLEAN_CTRL *Ctrl) {
	if (!Ctrl) return;
	if (Ctrl->I.inputfile) free(Ctrl->I.inputfile);
	gmt_M_free(GMT, Ctrl);
}

static int usage(struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose(API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return GMT_NOERROR;

	GMT_Message(API, GMT_TIME_NONE,
		"usage: mbclean -I<inputfile> [-Adev_max] [-Blow/high] [-Cslope/unit]\n"
		"\t[-Dmin/max] [-E<max_acrosstrack>] [-Fformat] [-Gfrac_low/frac_high]\n"
		"\t[-K<min_range>] [-Llonflip] [-Mmode] [-N<ping_dev_tol>]\n"
		"\t[-Pmin_speed/max_speed] [-Q[<backup>]] [-R<max_heading_rate>]\n"
		"\t[-Sspike/mode/form] [-Ttolerance] [-Unum_good_min]\n"
		"\t[-Wwest/east/south/north] [-Xleft[/right]] [-Yleft/right[/mode]] [-Z]\n"
		"\t[-V -H]\n\n");
	if (level == GMT_SYNOPSIS) return GMT_PARSE_ERROR;

	GMT_Message(API, GMT_TIME_NONE,
		"\t<inputfile> is an MB-System datalist referencing the swath data.\n\n");
	return GMT_PARSE_ERROR;
}

/* --- parse ----------------------------------------------------------- */

static int parse(struct GMT_CTRL *GMT, struct MBCLEAN_CTRL *Ctrl, struct GMT_OPTION *options) {
	unsigned int n_errors = 0, n_files = 0;
	int    n;
	struct GMT_OPTION *opt;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

		case '<':   /* Input file */
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
			n = sscanf(opt->arg, "%lf", &Ctrl->A.deviation_max);
			if (n > 0) { Ctrl->A.active = true; Ctrl->transf.check_deviation = true; }
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -A option\n"); n_errors++; }
			break;

		case 'B':
			n = sscanf(opt->arg, "%lf/%lf", &Ctrl->B.depth_low, &Ctrl->B.depth_high);
			if (n > 1) { Ctrl->B.active = true; Ctrl->transf.check_range = true; }
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -B option\n"); n_errors++; }
			break;

		case 'C':
			Ctrl->C.slope_form = 0;
			n = sscanf(opt->arg, "%lf/%d", &Ctrl->C.slopemax, &Ctrl->C.slope_form);
			if (n > 0) {
				Ctrl->C.active = true;
				Ctrl->transf.check_slope = true;
				if (Ctrl->C.slope_form == 1)      Ctrl->C.slopemax = tan(Ctrl->C.slopemax);
				else if (Ctrl->C.slope_form == 2) Ctrl->C.slopemax = tan(DTR * Ctrl->C.slopemax);
			} else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -C option\n"); n_errors++; }
			break;

		case 'D':
			n = sscanf(opt->arg, "%lf/%lf", &Ctrl->D.distancemin, &Ctrl->D.distancemax);
			if (n > 1) Ctrl->D.active = true;
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -D option\n"); n_errors++; }
			break;

		case 'E':
			n = sscanf(opt->arg, "%lf", &Ctrl->E.max_acrosstrack);
			if (n > 0) { Ctrl->E.active = true; Ctrl->transf.zap_long_across = true; }
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -E option\n"); n_errors++; }
			break;

		case 'F':
			n = sscanf(opt->arg, "%d", &Ctrl->F.format);
			if (n > 0) Ctrl->F.active = true;
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -F option\n"); n_errors++; }
			break;

		case 'G':
			n = sscanf(opt->arg, "%lf/%lf", &Ctrl->G.fraction_low, &Ctrl->G.fraction_high);
			if (n > 1) { Ctrl->G.active = true; Ctrl->transf.check_fraction = true; }
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -G option\n"); n_errors++; }
			break;

		case 'I':
			if (!gmt_access(GMT, opt->arg, R_OK)) {
				Ctrl->I.inputfile = strdup(opt->arg);
				Ctrl->I.active = true;
				n_files = 1;
			} else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -I option (file not found)\n"); n_errors++; }
			break;

		case 'K':
			n = sscanf(opt->arg, "%lf", &Ctrl->K.range_min);
			if (n > 0) { Ctrl->K.active = true; Ctrl->transf.check_range_min = true; }
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -K option\n"); n_errors++; }
			break;

		case 'L':
			n = sscanf(opt->arg, "%d", &Ctrl->L.lonflip);
			if (n > 0) Ctrl->L.active = true;
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -L option\n"); n_errors++; }
			break;

		case 'M':
			n = sscanf(opt->arg, "%d", &Ctrl->M.mode);
			if (n > 0) Ctrl->M.active = true;
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -M option\n"); n_errors++; }
			break;

		case 'N':
			n = sscanf(opt->arg, "%lf", &Ctrl->N.ping_deviation_tolerance);
			if (n > 0) { Ctrl->N.active = true; Ctrl->transf.check_ping_deviation = true; }
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -N option\n"); n_errors++; }
			break;

		case 'P':
			n = sscanf(opt->arg, "%lf/%lf", &Ctrl->P.speed_low, &Ctrl->P.speed_high);
			if (n > 0) { Ctrl->P.active = true; Ctrl->transf.check_speed_good = true; }
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -P option\n"); n_errors++; }
			break;

		case 'Q':
			Ctrl->Q.backup_dist = 0.0;
			sscanf(opt->arg, "%lf", &Ctrl->Q.backup_dist);
			Ctrl->Q.active = true;
			Ctrl->transf.zap_rails = true;
			break;

		case 'R':
			n = sscanf(opt->arg, "%lf", &Ctrl->R.max_heading_rate);
			if (n > 0) { Ctrl->R.active = true; Ctrl->transf.zap_max_heading_rate = true; }
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -R option\n"); n_errors++; }
			break;

		case 'S':
			Ctrl->S.slope_form = 0;
			n = sscanf(opt->arg, "%lf/%d/%d",
					   &Ctrl->S.spikemax, &Ctrl->S.spike_mode, &Ctrl->S.slope_form);
			if (n > 1) {
				Ctrl->S.active = true;
				Ctrl->transf.check_spike = true;
				if (Ctrl->S.slope_form == 1)      Ctrl->S.spikemax = tan(Ctrl->S.spikemax);
				else if (Ctrl->S.slope_form == 2) Ctrl->S.spikemax = tan(DTR * Ctrl->S.spikemax);
			} else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -S option\n"); n_errors++; }
			break;

		case 'T':
			sscanf(opt->arg, "%lf", &Ctrl->T.tolerance);
			Ctrl->T.active = true;
			Ctrl->transf.fix_edit_timestamps = true;
			break;

		case 'U':
			n = sscanf(opt->arg, "%d", &Ctrl->U.num_good_min);
			if (n > 0) { Ctrl->U.active = true; Ctrl->transf.check_num_good_min = true; }
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -U option\n"); n_errors++; }
			break;

		case 'W':
			n = sscanf(opt->arg, "%lf/%lf/%lf/%lf",
					   &Ctrl->W.west, &Ctrl->W.east, &Ctrl->W.south, &Ctrl->W.north);
			if (n == 4) { Ctrl->W.active = true; Ctrl->transf.check_position_bounds = true; }
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -W option\n"); n_errors++; }
			break;

		case 'X': {
			int left = 0, right = 0;
			n = sscanf(opt->arg, "%d/%d", &left, &right);
			if (n == 1) right = left;
			Ctrl->X.zap_beams_left  = left;
			Ctrl->X.zap_beams_right = right;
			Ctrl->X.active = true;
			Ctrl->transf.zap_beams = true;
			break;
		}

		case 'Y': {
			double left = 0.0, right = 0.0;
			int    y_mode_tmp = 0;
			n = sscanf(opt->arg, "%lf/%lf/%d", &left, &right, &y_mode_tmp);
			Ctrl->Y.active = true;
			if (n == 1) {
				Ctrl->Y.flag_distance_left  = -fabs(left);
				Ctrl->Y.flag_distance_right =  fabs(left);
				Ctrl->Y.flag_distance = true;
			} else if (n == 2) {
				Ctrl->Y.flag_distance_left  = left;
				Ctrl->Y.flag_distance_right = right;
				Ctrl->Y.flag_distance = true;
			} else if (n == 3) {
				if (y_mode_tmp < 1 || y_mode_tmp > 4)
					y_mode_tmp = MBCLEAN_Y_MODE_DISTANCE_FLAG;
				Ctrl->Y.y_mode = y_mode_tmp;
				if (y_mode_tmp == MBCLEAN_Y_MODE_DISTANCE_FLAG) {
					Ctrl->Y.flag_distance_left  = left;
					Ctrl->Y.flag_distance_right = right;
					Ctrl->Y.flag_distance = true;
				} else if (y_mode_tmp == MBCLEAN_Y_MODE_DISTANCE_UNFLAG) {
					Ctrl->Y.unflag_distance_left  = left;
					Ctrl->Y.unflag_distance_right = right;
					Ctrl->Y.unflag_distance = true;
				} else if (y_mode_tmp == MBCLEAN_Y_MODE_ANGLE_FLAG) {
					Ctrl->Y.flag_angle_left  = left;
					Ctrl->Y.flag_angle_right = right;
					Ctrl->Y.flag_angle = true;
				} else { /* MBCLEAN_Y_MODE_ANGLE_UNFLAG */
					Ctrl->Y.unflag_angle_left  = left;
					Ctrl->Y.unflag_angle_right = right;
					Ctrl->Y.unflag_angle = true;
				}
			} else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -Y option\n"); n_errors++; }
			break;
		}

		case 'Z':
			Ctrl->Z.active = true;
			Ctrl->transf.check_zero_position = true;
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
#define Return(code)   { Free_mbclean_Ctrl(GMT, Ctrl); gmt_end_module(GMT, GMT_cpy); bailout(code); }

/* ====================================================================
 * GMT_mbclean  — GMT module entry point.
 * ==================================================================== */

int GMT_mbclean(void *V_API, int mode, void *args) {
	int    error   = MB_ERROR_NO_ERROR;

	struct MBCLEAN_CTRL *Ctrl = NULL;
	struct GMT_CTRL     *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION   *options = NULL;
	struct GMTAPI_CTRL  *API = gmt_get_api_ptr(V_API);

	/* ------- standard GMT module preamble -------------------------- */
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

	Ctrl = New_mbclean_Ctrl(GMT);
	if ((error = parse(GMT, Ctrl, options)) != 0) Return (error);

	/* ------- pull parsed control values into locals ---------------- */
	bool   check_deviation       = Ctrl->transf.check_deviation;
	bool   check_range           = Ctrl->transf.check_range;
	bool   check_slope           = Ctrl->transf.check_slope;
	bool   zap_long_across       = Ctrl->transf.zap_long_across;
	bool   check_fraction        = Ctrl->transf.check_fraction;
	bool   check_range_min       = Ctrl->transf.check_range_min;
	bool   check_speed_good      = Ctrl->transf.check_speed_good;
	bool   zap_rails             = Ctrl->transf.zap_rails;
	bool   zap_max_heading_rate  = Ctrl->transf.zap_max_heading_rate;
	bool   check_spike           = Ctrl->transf.check_spike;
	bool   fix_edit_timestamps   = Ctrl->transf.fix_edit_timestamps;
	bool   check_position_bounds = Ctrl->transf.check_position_bounds;
	bool   zap_beams             = Ctrl->transf.zap_beams;
	bool   check_zero_position   = Ctrl->transf.check_zero_position;
	bool   check_num_good_min    = Ctrl->transf.check_num_good_min;
	bool   check_ping_deviation  = Ctrl->transf.check_ping_deviation;

	bool   flag_distance         = Ctrl->Y.flag_distance;
	bool   unflag_distance       = Ctrl->Y.unflag_distance;
	bool   flag_angle            = Ctrl->Y.flag_angle;
	bool   unflag_angle          = Ctrl->Y.unflag_angle;

	int    verbose = GMT->common.V.active;
	int    status;

	/* ------- MBIO defaults + reset ---------------------------------- */
	int    format, pings, lonflip;
	double bounds[4];
	int    btime_i[7], etime_i[7];
	double speedmin, timegap;
	int    uselockfiles_i;
	bool   uselockfiles;

	status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);
	mb_uselockfiles(verbose, &uselockfiles);
	uselockfiles_i = uselockfiles ? 1 : 0;
	(void)uselockfiles_i;
	if (Ctrl->L.active) lonflip = Ctrl->L.lonflip;
	if (Ctrl->F.active) format  = Ctrl->F.format;

	pings = 1;
	bounds[0] = -360.;  bounds[1] = 360.;  bounds[2] = -90.;  bounds[3] = 90.;
	btime_i[0]=1962; btime_i[1]=2; btime_i[2]=21; btime_i[3]=10; btime_i[4]=30; btime_i[5]=0; btime_i[6]=0;
	etime_i[0]=2062; etime_i[1]=2; etime_i[2]=21; etime_i[3]=10; etime_i[4]=30; etime_i[5]=0; etime_i[6]=0;
	speedmin = 0.0;
	timegap  = 1000000000.0;

	/* ------- copy Ctrl values into named locals (algorithm style) -- */
	double deviation_max         = Ctrl->A.deviation_max;
	double depth_low             = Ctrl->B.depth_low;
	double depth_high            = Ctrl->B.depth_high;
	double slopemax              = Ctrl->C.slopemax;
	double distancemin           = Ctrl->D.distancemin;
	double distancemax           = Ctrl->D.distancemax;
	double max_acrosstrack       = Ctrl->E.max_acrosstrack;
	double fraction_low          = Ctrl->G.fraction_low;
	double fraction_high         = Ctrl->G.fraction_high;
	double range_min             = Ctrl->K.range_min;
	int    clean_mode            = Ctrl->M.mode;
	double ping_deviation_tol    = Ctrl->N.ping_deviation_tolerance;
	double speed_low             = Ctrl->P.speed_low;
	double speed_high            = Ctrl->P.speed_high;
	double backup_dist           = Ctrl->Q.backup_dist;
	double max_heading_rate      = Ctrl->R.max_heading_rate;
	double spikemax              = Ctrl->S.spikemax;
	int    spike_mode            = Ctrl->S.spike_mode;
	double tolerance             = Ctrl->T.tolerance;
	int    num_good_min          = Ctrl->U.num_good_min;
	double west                  = Ctrl->W.west;
	double east                  = Ctrl->W.east;
	double south                 = Ctrl->W.south;
	double north                 = Ctrl->W.north;
	int    zap_beams_left        = Ctrl->X.zap_beams_left;
	int    zap_beams_right       = Ctrl->X.zap_beams_right;
	double flag_distance_left    = Ctrl->Y.flag_distance_left;
	double flag_distance_right   = Ctrl->Y.flag_distance_right;
	double unflag_distance_left  = Ctrl->Y.unflag_distance_left;
	double unflag_distance_right = Ctrl->Y.unflag_distance_right;
	double flag_angle_left       = Ctrl->Y.flag_angle_left;
	double flag_angle_right      = Ctrl->Y.flag_angle_right;
	double unflag_angle_left     = Ctrl->Y.unflag_angle_left;
	double unflag_angle_right    = Ctrl->Y.unflag_angle_right;

	mb_path read_file;
	strcpy(read_file, Ctrl->I.active ? Ctrl->I.inputfile : "datalist.mb-1");

	/* If nothing else asked, default to slope checking. */
	if (!check_slope && !zap_beams && !flag_distance && !unflag_distance &&
		!zap_rails && !check_spike && !check_range && !check_fraction &&
		!check_speed_good && !check_deviation && !check_num_good_min &&
		!check_position_bounds && !check_zero_position && !fix_edit_timestamps &&
		!zap_max_heading_rate)
		check_slope = true;

	GMT_Report(API, GMT_MSG_VERBOSE,
			   "Processing input files (MB-System Version %s)\n", MB_VERSION);

	if (format == 0) mb_get_format(verbose, read_file, NULL, &format, &error);

	/* ------- open datalist (or single file) ------------------------ */
	bool   read_datalist = (format < 0);
	bool   read_data;
	void  *datalist = NULL;
	char   swathfile[MB_PATH_MAXLINE];
	char   dfile[MB_PATH_MAXLINE];
	double file_weight;
	int    look_processed = MB_DATALIST_LOOK_UNSET;

	if (read_datalist) {
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

	/* ------- shared state across files ----------------------------- */
	double btime_d, etime_d;
	bool   locked = false;
	int    lock_purpose = MBP_LOCK_NONE;
	mb_path lock_program, lock_cpu, lock_user;
	char    lock_date[25];

	char    swathfileread[MB_PATH_MAXLINE];
	int     formatread;
	bool    variable_beams, traveltime;
	double  distance, altitude, sensordepth;
	int     beams_bath, beams_amp, pixels_ss;

	void   *mbio_ptr = NULL;
	void   *store_ptr = NULL;
	int     kind;
	struct mbclean_ping_struct ping[3];
	int     pingsread;
	char    comment[MB_COMMENT_MAXLINE];

	/* totals (across files) */
	int nfiletot=0, ndatatot=0, ndepthrangetot=0, nminrangetot=0, nfractiontot=0;
	int ndeviationtot=0, nouterbeamstot=0, nouterdistancetot=0, ninnerdistancetot=0;
	int nouterangletot=0, ninnerangletot=0, nrailtot=0, nlong_acrosstot=0;
	int nmintot=0, nbadtot=0, nspiketot=0, npingdeviationtot=0;
	int nflagtot=0, nunflagtot=0, nflagesftot=0, nunflagesftot=0, nzeroesftot=0;
	int nmax_heading_ratetot=0;

	char  esffile[MB_PATH_MAXLINE];
	struct mb_esf_struct esf;
	int   sensorhead = 0, sensorhead_error = MB_ERROR_NO_ERROR;
	bool  beam_flagging;
	double mtodeglon, mtodeglat;
	int    nlist;
	double median = 0.0;

	/* ================================================================
	 * Per-file loop
	 * ============================================================== */
	while (read_data) {
		bool oktoprocess = true;

		if ((status = mb_format_flags(verbose, &format, &variable_beams, &traveltime,
									  &beam_flagging, &error)) != MB_SUCCESS) {
			char *message = NULL;
			mb_error(verbose, error, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "MBIO error from mb_format_flags (format %d): %s\n", format, message);
			GMT_Report(API, GMT_MSG_NORMAL, "File <%s> skipped by program <%s>\n", swathfile, THIS_MODULE_NAME);
			oktoprocess = false;
			status = MB_SUCCESS; error = MB_ERROR_NO_ERROR;
		}

		if (!beam_flagging)
			GMT_Report(API, GMT_MSG_NORMAL, "Warning: MBIO format %d does not allow flagging; mbprocess will null soundings.\n", format);

		/* lock handling */
		if (uselockfiles)
			status = mb_pr_lockswathfile(verbose, swathfile, MBP_LOCK_EDITBATHY, THIS_MODULE_NAME, &error);
		else {
			mb_pr_lockinfo(verbose, swathfile, &locked, &lock_purpose,
						   lock_program, lock_user, lock_cpu, lock_date, &error);
			if (error == MB_ERROR_FILE_LOCKED) {
				GMT_Report(API, GMT_MSG_NORMAL, "File %s locked but lock ignored\n", swathfile);
				error = MB_ERROR_NO_ERROR;
			}
		}
		if (status == MB_FAILURE) {
			if (error == MB_ERROR_FILE_LOCKED) {
				mb_pr_lockinfo(verbose, swathfile, &locked, &lock_purpose,
							   lock_program, lock_user, lock_cpu, lock_date, &error);
				GMT_Report(API, GMT_MSG_NORMAL,
						   "Unable to open %s — locked by <%s> running <%s> on <%s> at <%s>\n",
						   swathfile, lock_user, lock_program, lock_cpu, lock_date);
			} else if (error == MB_ERROR_OPEN_FAIL) {
				GMT_Report(API, GMT_MSG_NORMAL, "Unable to create lock for %s (permissions?)\n", swathfile);
			}
			oktoprocess = false;
			status = MB_SUCCESS; error = MB_ERROR_NO_ERROR;
		}

		if (oktoprocess) {
			/* fast bathymetry / fbt redirection */
			strcpy(swathfileread, swathfile);
			formatread = format;
			mb_get_fbt(verbose, swathfileread, &formatread, &error);

			if (mb_read_init(verbose, swathfileread, formatread, pings, lonflip, bounds,
							 btime_i, etime_i, speedmin, timegap, &mbio_ptr,
							 &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss,
							 &error) != MB_SUCCESS) {
				char *message = NULL;
				mb_error(verbose, error, &message);
				GMT_Report(API, GMT_MSG_NORMAL, "mb_read_init failed: %s\n", message);
				GMT_Report(API, GMT_MSG_NORMAL, "Multibeam File <%s> not initialized\n", swathfile);
				Return(error);
			}

			/* per-file counters */
			int ndata=0, ndepthrange=0, nminrange=0, nfraction=0, nspeed=0;
			int nzeropos=0, nrangepos=0, ndeviation=0, nouterbeams=0;
			int nouterdistance=0, ninnerdistance=0, nouterangle=0, ninnerangle=0;
			int nrail=0, nlong_across=0, nmin=0, nbad=0, nspike=0;
			int nmax_heading_rate=0, npingdeviation=0;
			int nflag=0, nunflag=0, nflagesf=0, nunflagesf=0, nzeroesf=0;

			GMT_Report(API, GMT_MSG_VERBOSE, "\nProcessing %s\n", swathfileread);

			/* allocate per-ping arrays via mb_register_array */
			int i;
			for (i = 0; i < 3; i++) {
				ping[i].beamflag = NULL; ping[i].beamflagorg = NULL;
				ping[i].bath = NULL; ping[i].bathacrosstrack = NULL;
				ping[i].bathalongtrack = NULL; ping[i].bathx = NULL; ping[i].bathy = NULL;
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
											   sizeof(char), (void **)&ping[i].beamflag, &error);
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
											   sizeof(char), (void **)&ping[i].beamflagorg, &error);
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
											   sizeof(double), (void **)&ping[i].bath, &error);
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
											   sizeof(double), (void **)&ping[i].bathacrosstrack, &error);
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
											   sizeof(double), (void **)&ping[i].bathalongtrack, &error);
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
											   sizeof(double), (void **)&ping[i].bathx, &error);
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
											   sizeof(double), (void **)&ping[i].bathy, &error);
			}
			double *amp = NULL, *ss = NULL, *ssacrosstrack = NULL, *ssalongtrack = NULL;
			double *list = NULL;
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE,
										   sizeof(double), (void **)&amp, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
										   sizeof(double), (void **)&ss, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
										   sizeof(double), (void **)&ssacrosstrack, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
										   sizeof(double), (void **)&ssalongtrack, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
										   4 * sizeof(double), (void **)&list, &error);

			if (error != MB_ERROR_NO_ERROR) {
				char *message = NULL;
				mb_error(verbose, error, &message);
				GMT_Report(API, GMT_MSG_NORMAL, "MBIO error allocating data arrays: %s\n", message);
				Return(error);
			}

			/* edit save file */
			bool esffile_open = false;
			if (status == MB_SUCCESS) {
				GMT_Report(API, GMT_MSG_VERBOSE, "Sorting old edits...\n");
				status = mb_esf_load(verbose, THIS_MODULE_NAME, swathfile, true, true,
									 esffile, &esf, &error);
				if (status == MB_SUCCESS && esf.esffp != NULL) esffile_open = true;
				if (status == MB_FAILURE && error == MB_ERROR_OPEN_FAIL) {
					esffile_open = false;
					GMT_Report(API, GMT_MSG_NORMAL, "Unable to open new ESF %s\n", esf.esffile);
				} else if (status == MB_FAILURE && error == MB_ERROR_MEMORY_FAIL) {
					esffile_open = false;
					GMT_Report(API, GMT_MSG_NORMAL, "Unable to allocate memory for ESF\n");
				}
				GMT_Report(API, GMT_MSG_VERBOSE, "%d old edits sorted...\n", esf.nedit);
			}

			int nrec = 0;
			int action;
			double dev, ping_deviation;
			int center;
			double lowdist, highdist;
			int num_good;
			int j, k;

			/* ============================================================
			 * Per-ping loop
			 * ============================================================ */
			bool done = false;
			while (!done) {
				error = MB_ERROR_NO_ERROR;
				status = mb_get(verbose, mbio_ptr, &kind, &pingsread,
								ping[nrec].time_i, &ping[nrec].time_d,
								&ping[nrec].navlon, &ping[nrec].navlat,
								&ping[nrec].speed, &ping[nrec].heading,
								&distance, &altitude, &sensordepth,
								&ping[nrec].beams_bath, &beams_amp, &pixels_ss,
								ping[nrec].beamflag, ping[nrec].bath, amp,
								ping[nrec].bathacrosstrack, ping[nrec].bathalongtrack,
								ss, ssacrosstrack, ssalongtrack, comment, &error);

				if (status == MB_SUCCESS && kind == MB_DATA_DATA) {
					/* ping multiplicity */
					status = mb_get_store(verbose, mbio_ptr, &store_ptr, &error);
					const int sh_status = mb_sensorhead(verbose, mbio_ptr, store_ptr,
														&sensorhead, &sensorhead_error);
					if (sh_status == MB_SUCCESS)
						ping[nrec].multiplicity = sensorhead;
					else if (nrec > 0 && fabs(ping[nrec].time_d - ping[nrec-1].time_d) < MB_ESF_MAXTIMEDIFF)
						ping[nrec].multiplicity = ping[nrec-1].multiplicity + 1;
					else
						ping[nrec].multiplicity = 0;

					/* save original beamflags */
					for (i = 0; i < ping[nrec].beams_bath; i++)
						ping[nrec].beamflagorg[i] = ping[nrec].beamflag[i];

					/* local coordinates */
					mb_coor_scale(verbose, ping[nrec].navlat, &mtodeglon, &mtodeglat);
					const double headingx = sin(ping[nrec].heading * DTR);
					const double headingy = cos(ping[nrec].heading * DTR);
					for (j = 0; j <= nrec; j++) {
						for (i = 0; i < ping[j].beams_bath; i++) {
							ping[j].bathx[i] = (ping[j].navlon - ping[0].navlon) / mtodeglon
											 + headingy * ping[j].bathacrosstrack[i]
											 + headingx * ping[j].bathalongtrack[i];
							ping[j].bathy[i] = (ping[j].navlat - ping[0].navlat) / mtodeglat
											 - headingx * ping[j].bathacrosstrack[i]
											 + headingy * ping[j].bathalongtrack[i];
						}
					}

					if (fix_edit_timestamps)
						mb_esf_fixtimestamps(verbose, &esf, ping[nrec].time_d, tolerance, &error);

					mb_esf_apply(verbose, &esf, ping[nrec].time_d, ping[nrec].multiplicity,
								 ping[nrec].beams_bath, ping[nrec].beamflag, &error);

					for (i = 0; i < ping[nrec].beams_bath; i++) {
						if (ping[nrec].beamflag[i] != ping[nrec].beamflagorg[i]) {
							if (mb_beam_ok(ping[nrec].beamflag[i])) nunflagesf++;
							else                                    nflagesf++;
						}
					}
					ndata++;
					nrec++;
				}
				else if (error > MB_ERROR_NO_ERROR) {
					done = true;
				}

				/* ----- process one record ----- */
				if (nrec > 0) {
					const int irec = (nrec >= 2) ? 1 : 0;
					center = ping[irec].beams_bath / 2;

					/* zap outer beams by number */
					if (zap_beams) {
						for (i = 0; i < MIN(zap_beams_left, center); i++) {
							if (mb_beam_ok(ping[irec].beamflag[i])) {
								ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								nouterbeams++; nflag++;
								mb_ess_save(verbose, &esf, ping[irec].time_d,
											i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
											MBP_EDIT_FILTER, &error);
							}
						}
						for (i = 0; i < MIN(zap_beams_right, center); i++) {
							int jj = ping[irec].beams_bath - i - 1;
							if (mb_beam_ok(ping[irec].beamflag[jj])) {
								ping[irec].beamflag[jj] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								nouterbeams++; nflag++;
								mb_ess_save(verbose, &esf, ping[irec].time_d,
											jj + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
											MBP_EDIT_FILTER, &error);
							}
						}
					}

					/* flag by distance */
					if (flag_distance) {
						for (i = 0; i < ping[irec].beams_bath; i++) {
							if (mb_beam_ok(ping[irec].beamflag[i]) &&
								(ping[irec].bathacrosstrack[i] <= flag_distance_left ||
								 ping[irec].bathacrosstrack[i] >= flag_distance_right)) {
								ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								nouterdistance++; nflag++;
								mb_ess_save(verbose, &esf, ping[irec].time_d,
											i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
											MBP_EDIT_FILTER, &error);
							}
						}
					}

					/* unflag by distance */
					if (unflag_distance) {
						for (i = 0; i < ping[irec].beams_bath; i++) {
							if (ping[irec].beamflag[i] != MB_FLAG_NULL &&
								!mb_beam_ok(ping[irec].beamflag[i]) &&
								ping[irec].bathacrosstrack[i] >= unflag_distance_left &&
								ping[irec].bathacrosstrack[i] <= unflag_distance_right) {
								ping[irec].beamflag[i] = MB_FLAG_NONE;
								ninnerdistance++; nflag++;
								mb_ess_save(verbose, &esf, ping[irec].time_d,
											i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
											MBP_EDIT_UNFLAG, &error);
							}
						}
					}

					/* flag by acrosstrack angle */
					if (flag_angle) {
						for (i = 0; i < ping[irec].beams_bath; i++) {
							if (mb_beam_ok(ping[irec].beamflag[i])) {
								double theta, phi, pitch, roll;
								const double xx = ping[irec].bathacrosstrack[i];
								const double yy = ping[irec].bathalongtrack[i];
								const double zz = ping[irec].bath[i] - sensordepth;
								mb_xyz_to_takeoff(verbose, xx, yy, zz, &theta, &phi, &error);
								mb_takeoff_to_rollpitch(verbose, theta, phi, &pitch, &roll, &error);
								roll = 90.0 - roll;
								if (roll <= flag_angle_left || roll >= flag_angle_right) {
									ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
									nouterangle++; nflag++;
									mb_ess_save(verbose, &esf, ping[irec].time_d,
												i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
												MBP_EDIT_FILTER, &error);
								}
							}
						}
					}

					/* unflag by acrosstrack angle */
					if (unflag_angle) {
						for (i = 0; i < ping[irec].beams_bath; i++) {
							if (ping[irec].beamflag[i] != MB_FLAG_NULL &&
								!mb_beam_ok(ping[irec].beamflag[i])) {
								double theta, phi, pitch, roll;
								const double xx = ping[irec].bathacrosstrack[i];
								const double yy = ping[irec].bathalongtrack[i];
								const double zz = ping[irec].bath[i] - sensordepth;
								mb_xyz_to_takeoff(verbose, xx, yy, zz, &theta, &phi, &error);
								mb_takeoff_to_rollpitch(verbose, theta, phi, &pitch, &roll, &error);
								roll = 90.0 - roll;
								if (roll >= unflag_angle_left && roll <= unflag_angle_right) {
									ping[irec].beamflag[i] = MB_FLAG_NONE;
									ninnerangle++; nflag++;
									mb_ess_save(verbose, &esf, ping[irec].time_d,
												i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
												MBP_EDIT_UNFLAG, &error);
								}
							}
						}
					}

					/* speed range */
					if (check_speed_good) {
						if (ping[irec].speed > speed_high || ping[irec].speed < speed_low) {
							for (i = 0; i < ping[irec].beams_bath; i++) {
								ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								nspeed++; nflag++;
								mb_ess_save(verbose, &esf, ping[irec].time_d,
											i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
											MBP_EDIT_FILTER, &error);
							}
						}
					}

					/* position bounds */
					if (check_position_bounds) {
						if (ping[irec].navlon < west || ping[irec].navlon > east ||
							ping[irec].navlat < south || ping[irec].navlat > north) {
							for (i = 0; i < ping[irec].beams_bath; i++) {
								ping[irec].beamflag[i] = MB_FLAG_NULL;
								nrangepos++; nflag++;
								mb_ess_save(verbose, &esf, ping[irec].time_d,
											i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
											MBP_EDIT_ZERO, &error);
							}
						}
					}

					/* zero position */
					if (check_zero_position) {
						if (ping[irec].navlon == 0.0 && ping[irec].navlat == 0.0) {
							for (i = 0; i < ping[irec].beams_bath; i++) {
								ping[irec].beamflag[i] = MB_FLAG_NULL;
								nzeropos++; nflag++;
								mb_ess_save(verbose, &esf, ping[irec].time_d,
											i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
											MBP_EDIT_ZERO, &error);
							}
						}
					}

					/* depth range */
					if (check_range) {
						for (i = 0; i < ping[irec].beams_bath; i++) {
							if (mb_beam_ok(ping[irec].beamflag[i]) &&
								(ping[irec].bath[i] < depth_low || ping[irec].bath[i] > depth_high)) {
								ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								ndepthrange++; nflag++;
								mb_ess_save(verbose, &esf, ping[irec].time_d,
											i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
											MBP_EDIT_FILTER, &error);
							}
						}
					}

					/* minimum range */
					if (check_range_min) {
						for (i = 0; i < ping[irec].beams_bath; i++) {
							if (mb_beam_ok(ping[irec].beamflag[i])) {
								const double xx = ping[irec].bathacrosstrack[i];
								const double yy = ping[irec].bathalongtrack[i];
								const double zz = ping[irec].bath[i] - sensordepth;
								if (sqrt(xx*xx + yy*yy + zz*zz) < range_min) {
									ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
									nminrange++; nflag++;
									mb_ess_save(verbose, &esf, ping[irec].time_d,
												i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
												MBP_EDIT_FILTER, &error);
								}
							}
						}
					}

					/* max heading rate */
					if (zap_max_heading_rate) {
						double heading_rate;
						if (nrec > 1) {
							double dh = ping[nrec-1].heading - ping[0].heading;
							if (dh >  180) dh -= 360;
							if (dh < -180) dh += 360;
							heading_rate = dh / (ping[nrec-1].time_d - ping[0].time_d);
						} else {
							heading_rate = 0.0;
						}
						for (i = 0; i < ping[irec].beams_bath; i++) {
							if (fabs(heading_rate) > max_heading_rate) {
								ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								nmax_heading_rate++; nflag++;
								mb_ess_save(verbose, &esf, ping[irec].time_d,
											i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
											MBP_EDIT_FILTER, &error);
							}
						}
					}

					/* zap rails */
					if (zap_rails) {
						lowdist = 0.0; highdist = 0.0;
						for (j = center; j < ping[irec].beams_bath; j++) {
							if (mb_beam_ok(ping[irec].beamflag[j]) &&
								ping[irec].bathacrosstrack[j] <= highdist - backup_dist) {
								ping[irec].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								nrail++; nflag++;
								mb_ess_save(verbose, &esf, ping[irec].time_d,
											j + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
											MBP_EDIT_FILTER, &error);
							} else {
								highdist = ping[irec].bathacrosstrack[j];
							}
							int kk = center - (j - center) - 1;
							if (kk >= 0 && kk < ping[irec].beams_bath &&
								mb_beam_ok(ping[irec].beamflag[kk]) &&
								ping[irec].bathacrosstrack[kk] >= lowdist + backup_dist) {
								ping[irec].beamflag[kk] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								nrail++; nflag++;
								mb_ess_save(verbose, &esf, ping[irec].time_d,
											kk + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
											MBP_EDIT_FILTER, &error);
							} else if (kk >= 0 && kk < ping[irec].beams_bath) {
								lowdist = ping[irec].bathacrosstrack[kk];
							}
						}
					}

					/* zap long acrosstrack */
					if (zap_long_across) {
						for (j = 0; j < ping[irec].beams_bath; j++) {
							if (mb_beam_ok(ping[irec].beamflag[j]) &&
								fabs(ping[irec].bathacrosstrack[j]) > max_acrosstrack) {
								ping[irec].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								nlong_across++; nflag++;
								mb_ess_save(verbose, &esf, ping[irec].time_d,
											j + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
											MBP_EDIT_FILTER, &error);
							}
						}
					}

					/* tests that loop over all available beams */
					if (check_fraction || check_deviation || check_spike || check_slope) {
						for (i = 0; i < ping[irec].beams_bath; i++) {
							if (mb_beam_ok(ping[irec].beamflag[i])) {
								if (median <= 0.0) median = ping[irec].bath[i];
								nlist = 0;
								for (j = 0; j < nrec; j++) {
									for (k = 0; k < ping[j].beams_bath; k++) {
										if (mb_beam_ok(ping[j].beamflag[k])) {
											const double ddx = ping[j].bathx[k] - ping[irec].bathx[i];
											const double ddy = ping[j].bathy[k] - ping[irec].bathy[i];
											const double dd  = sqrt(ddx*ddx + ddy*ddy);
											if (dd <= distancemax * median) {
												list[nlist] = ping[j].bath[k];
												nlist++;
											}
										}
									}
								}
								qsort((char *)list, nlist, sizeof(double), mb_double_compare);
								if (nlist > 0) median = list[nlist/2];

								/* fractional deviation from median */
								if (check_fraction && median > 0.0) {
									if (ping[irec].bath[i] / median < fraction_low ||
										ping[irec].bath[i] / median > fraction_high) {
										ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
										nfraction++; nflag++;
										mb_ess_save(verbose, &esf, ping[irec].time_d,
													i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
													MBP_EDIT_FILTER, &error);
									}
								}

								/* absolute deviation from median */
								if (check_deviation && median > 0.0) {
									if (fabs(ping[irec].bath[i] - median) > deviation_max) {
										ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
										ndeviation++; nflag++;
										mb_ess_save(verbose, &esf, ping[irec].time_d,
													i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
													MBP_EDIT_FILTER, &error);
									}
								}

								/* spike check (acrosstrack) */
								if (check_spike && (spike_mode & 1) && median > 0.0 &&
									i > 0 && i < ping[irec].beams_bath - 1 &&
									mb_beam_ok(ping[irec].beamflag[i-1]) &&
									mb_beam_ok(ping[irec].beamflag[i+1])) {
									const double ax = ping[irec].bathx[i-1] - ping[irec].bathx[i];
									const double ay = ping[irec].bathy[i-1] - ping[irec].bathy[i];
									const double dd = sqrt(ax*ax + ay*ay);
									if (dd > distancemin*median && dd <= distancemax*median) {
										const double slope = (ping[irec].bath[i-1] - ping[irec].bath[i]) / dd;
										const double bx = ping[irec].bathx[i+1] - ping[irec].bathx[i];
										const double by = ping[irec].bathy[i+1] - ping[irec].bathy[i];
										const double dd2 = sqrt(bx*bx + by*by);
										if (dd2 > distancemin*median && dd2 <= distancemax*median) {
											const double slope2 = (ping[irec].bath[i] - ping[irec].bath[i+1]) / dd2;
											if ((slope > spikemax && slope2 < -spikemax) ||
												(slope2 > spikemax && slope < -spikemax)) {
												ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
												nspike++; nflag++;
												mb_ess_save(verbose, &esf, ping[irec].time_d,
															i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
															MBP_EDIT_FILTER, &error);
											}
										}
									}
								}

								/* spike check (alongtrack, needs 3 pings) */
								if (check_spike && nrec == 3 && (spike_mode & 2) &&
									mb_beam_ok(ping[0].beamflag[i]) &&
									mb_beam_ok(ping[2].beamflag[i])) {
									const double ax = ping[0].bathx[i] - ping[1].bathx[i];
									const double ay = ping[0].bathy[i] - ping[1].bathy[i];
									const double dd = sqrt(ax*ax + ay*ay);
									if (dd > distancemin*median && dd <= distancemax*median) {
										const double slope = (ping[0].bath[i] - ping[1].bath[i]) / dd;
										const double bx = ping[2].bathx[i] - ping[1].bathx[i];
										const double by = ping[2].bathy[i] - ping[1].bathy[i];
										const double dd2 = sqrt(bx*bx + by*by);
										if (dd2 > distancemin*median && dd2 <= distancemax*median) {
											const double slope2 = (ping[1].bath[i] - ping[2].bath[i]) / dd2;
											if ((slope > spikemax && slope2 < -spikemax) ||
												(slope2 > spikemax && slope < -spikemax)) {
												ping[1].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
												nspike++; nflag++;
												mb_ess_save(verbose, &esf, ping[1].time_d,
															i + ping[1].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
															MBP_EDIT_FILTER, &error);
											}
										}
									}
								}

								/* slope check (needs 3 pings) */
								if (check_slope && nrec == 3 && median > 0.0) {
									for (j = 0; j < nrec; j++) {
										for (k = 0; k < ping[j].beams_bath; k++) {
											if (mb_beam_ok(ping[j].beamflag[k])) {
												const double ax = ping[j].bathx[k] - ping[1].bathx[i];
												const double ay = ping[j].bathy[k] - ping[1].bathy[i];
												const double dd = sqrt(ax*ax + ay*ay);
												const double slope = (dd > 0.0 && dd <= distancemax * median)
													? fabs((ping[j].bath[k] - ping[1].bath[i]) / dd) : 0.0;
												struct bad_struct bad[2] = {
													{ false, 0, 0, 0.0 }, { false, 0, 0, 0.0 }
												};
												if (slope > slopemax && dd > distancemin*median) {
													if (clean_mode == MBCLEAN_FLAG_BOTH) {
														bad[0].flag=true; bad[0].ping=j; bad[0].beam=k; bad[0].bath=ping[j].bath[k];
														bad[1].flag=true; bad[1].ping=1; bad[1].beam=i; bad[1].bath=ping[1].bath[i];
														ping[j].beamflag[k] = MB_FLAG_FLAG + MB_FLAG_FILTER;
														ping[1].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
														nbad++; nflag += 2;
														mb_ess_save(verbose, &esf, ping[j].time_d,
																	k + ping[j].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
																	MBP_EDIT_FILTER, &error);
														mb_ess_save(verbose, &esf, ping[1].time_d,
																	i + ping[1].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
																	MBP_EDIT_FILTER, &error);
													} else {
														if (fabs(ping[j].bath[k] - median) >
															fabs(ping[1].bath[i] - median)) {
															bad[0].flag=true; bad[0].ping=j; bad[0].beam=k; bad[0].bath=ping[j].bath[k];
															ping[j].beamflag[k] = MB_FLAG_FLAG + MB_FLAG_FILTER;
															mb_ess_save(verbose, &esf, ping[j].time_d,
																		k + ping[j].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
																		MBP_EDIT_FILTER, &error);
														} else {
															bad[0].flag=true; bad[0].ping=1; bad[0].beam=i; bad[0].bath=ping[1].bath[i];
															ping[1].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
															mb_ess_save(verbose, &esf, ping[1].time_d,
																		i + ping[1].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
																		MBP_EDIT_FILTER, &error);
														}
														nbad++; nflag++;
													}
												}
											}
										}
									}
								}
							}
						}
					}

					/* minimum good beams per swath side */
					if (check_num_good_min && num_good_min > 0) {
						/* port side */
						num_good = 0;
						for (i = 0; i < center; i++)
							if (mb_beam_ok(ping[irec].beamflag[i])) num_good++;
						if (num_good < num_good_min) {
							for (i = 0; i < center; i++) {
								if (mb_beam_ok(ping[irec].beamflag[i])) {
									ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
									nmin++; nflag++;
									mb_ess_save(verbose, &esf, ping[irec].time_d,
												i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
												MBP_EDIT_FILTER, &error);
								}
							}
						}
						/* starboard side */
						num_good = 0;
						for (i = center + 1; i < ping[irec].beams_bath; i++)
							if (mb_beam_ok(ping[irec].beamflag[i])) num_good++;
						if (num_good < num_good_min) {
							for (i = center + 1; i < ping[irec].beams_bath; i++) {
								if (mb_beam_ok(ping[irec].beamflag[i])) {
									ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
									nmin++; nflag++;
									mb_ess_save(verbose, &esf, ping[irec].time_d,
												i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
												MBP_EDIT_FILTER, &error);
								}
							}
						}
					}

					/* ping deviation */
					if (check_ping_deviation && nrec >= 3) {
						double devsqsum = 0.0;
						int    ndevsqsum = 0;
						for (i = 0; i < ping[irec].beams_bath; i++) {
							if (mb_beam_ok(ping[irec-1].beamflag[i]) &&
								mb_beam_ok(ping[irec  ].beamflag[i]) &&
								mb_beam_ok(ping[irec+1].beamflag[i])) {
								dev = (ping[irec].bath[i] - ping[irec+1].bath[i])
									+ (ping[irec].bath[i] - ping[irec-1].bath[i]);
								devsqsum += dev * dev;
								ndevsqsum++;
							}
						}
						if (ndevsqsum > (ping[irec].beams_bath / 4)) {
							ping_deviation = sqrt(devsqsum / ndevsqsum);
							if (ping_deviation > ping_deviation_tol) {
								for (i = 0; i < ping[irec].beams_bath; i++) {
									if (mb_beam_ok(ping[irec].beamflag[i])) {
										ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
										npingdeviation++; nflag++;
										mb_ess_save(verbose, &esf, ping[irec].time_d,
													i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
													MBP_EDIT_FILTER, &error);
									}
								}
							}
						}
					}
				}

				/* write out edits from completed pings */
				if ((status == MB_SUCCESS && nrec == 3) || done) {
					int how_many = done ? nrec : 1;
					int irec2;
					for (irec2 = 0; irec2 < how_many; irec2++) {
						for (i = 0; i < ping[irec2].beams_bath; i++) {
							if (ping[irec2].beamflag[i] != ping[irec2].beamflagorg[i]) {
								if (mb_beam_ok(ping[irec2].beamflag[i]))
									action = MBP_EDIT_UNFLAG;
								else if (mb_beam_check_flag_filter2(ping[irec2].beamflag[i]))
									action = MBP_EDIT_FILTER;
								else if (mb_beam_check_flag_filter(ping[irec2].beamflag[i]))
									action = MBP_EDIT_FILTER;
								else if (ping[irec2].beamflag[i] != MB_FLAG_NULL)
									action = MBP_EDIT_FLAG;
								else
									action = MBP_EDIT_ZERO;
								mb_esf_save(verbose, &esf, ping[irec2].time_d,
											i + ping[irec2].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
											action, &error);
							}
						}
					}
				}

				/* slide records back by one */
				if (status == MB_SUCCESS && nrec == 3) {
					nrec = 2;
					for (j = 0; j < 2; j++) {
						for (i = 0; i < 7; i++) ping[j].time_i[i] = ping[j+1].time_i[i];
						ping[j].time_d        = ping[j+1].time_d;
						ping[j].multiplicity  = ping[j+1].multiplicity;
						ping[j].navlon        = ping[j+1].navlon;
						ping[j].navlat        = ping[j+1].navlat;
						ping[j].speed         = ping[j+1].speed;
						ping[j].heading       = ping[j+1].heading;
						ping[j].beams_bath    = ping[j+1].beams_bath;
						for (i = 0; i < ping[j].beams_bath; i++) {
							ping[j].beamflag[i]        = ping[j+1].beamflag[i];
							ping[j].beamflagorg[i]     = ping[j+1].beamflagorg[i];
							ping[j].bath[i]            = ping[j+1].bath[i];
							ping[j].bathacrosstrack[i] = ping[j+1].bathacrosstrack[i];
							ping[j].bathalongtrack[i]  = ping[j+1].bathalongtrack[i];
							ping[j].bathx[i]           = ping[j+1].bathx[i];
							ping[j].bathy[i]           = ping[j+1].bathy[i];
						}
					}
				}
			}  /* per-ping while */

			status = mb_close(verbose, &mbio_ptr, &error);
			status = mb_esf_close(verbose, &esf, &error);

			if (esffile_open) {
				status = mb_pr_update_format(verbose, swathfile, true, format, &error);
				status = mb_pr_update_edit(verbose, swathfile, MBP_EDIT_ON, esffile, &error);
			}
			if (uselockfiles)
				status = mb_pr_unlockswathfile(verbose, swathfile, MBP_LOCK_EDITBATHY,
											   THIS_MODULE_NAME, &error);

			/* update totals */
			nfiletot++;
			ndatatot          += ndata;
			nflagesftot       += nflagesf;
			nunflagesftot     += nunflagesf;
			nzeroesftot       += nzeroesf;
			ndepthrangetot    += ndepthrange;
			nminrangetot      += nminrange;
			nfractiontot      += nfraction;
			ndeviationtot     += ndeviation;
			nouterbeamstot    += nouterbeams;
			nouterdistancetot += nouterdistance;
			ninnerdistancetot += ninnerdistance;
			nouterangletot    += nouterangle;
			ninnerangletot    += ninnerangle;
			nrailtot          += nrail;
			nlong_acrosstot   += nlong_across;
			nmax_heading_ratetot += nmax_heading_rate;
			nmintot           += nmin;
			nbadtot           += nbad;
			nspiketot         += nspike;
			npingdeviationtot += npingdeviation;
			nflagtot          += nflag;
			nunflagtot        += nunflag;

			GMT_Report(API, GMT_MSG_NORMAL,
				"%d records | flag:%d unflag:%d outer:%d dist:%d angle:%d rails:%d "
				"long:%d hdgrate:%d depth:%d minrange:%d fraction:%d speed:%d "
				"zeropos:%d devmedian:%d nfew:%d slopes:%d spikes:%d pingdev:%d\n",
				ndata, nflag, nunflag, nouterbeams, nouterdistance, nouterangle,
				nrail, nlong_across, nmax_heading_rate, ndepthrange, nminrange,
				nfraction, nspeed, nzeropos, ndeviation, nmin, nbad, nspike,
				npingdeviation);
		}

		/* next file? */
		if (read_datalist) {
			read_data = (mb_datalist_read(verbose, datalist, swathfile, dfile,
										  &format, &file_weight, &error) == MB_SUCCESS);
		} else {
			read_data = false;
		}
	}  /* per-file while */

	if (read_datalist) mb_datalist_close(verbose, &datalist, &error);

	GMT_Report(API, GMT_MSG_NORMAL,
		"\nMBclean Totals: files:%d records:%d esf_flag:%d esf_unflag:%d "
		"esf_zero:%d outer:%d dist:%d innerdist:%d angle:%d innerangle:%d "
		"rails:%d long:%d hdgrate:%d nfew:%d depth:%d minrange:%d frac:%d "
		"devmedian:%d slopes:%d spikes:%d pingdev:%d flagged:%d unflagged:%d\n",
		nfiletot, ndatatot, nflagesftot, nunflagesftot, nzeroesftot,
		nouterbeamstot, nouterdistancetot, ninnerdistancetot, nouterangletot,
		ninnerangletot, nrailtot, nlong_acrosstot, nmax_heading_ratetot,
		nmintot, ndepthrangetot, nminrangetot, nfractiontot,
		ndeviationtot, nbadtot, nspiketot, npingdeviationtot,
		nflagtot, nunflagtot);

	if (mb_memory_list(verbose, &error) == MB_FAILURE)
		GMT_Report(API, GMT_MSG_NORMAL,
				   "Program %s completed but leaked memory\n", THIS_MODULE_NAME);

	Return(GMT_NOERROR);
}

/* --------------------------------------------------------------------
 * mbclean_save_edit — byteswap-aware write of one edit record.
 * -------------------------------------------------------------------- */
static int mbclean_save_edit(int verbose, FILE *sofp, double time_d,
							 int beam, int action, int *error) {
	int status = MB_SUCCESS;

	if (sofp != NULL) {
#ifdef BYTESWAPPED
		mb_swap_double(&time_d);
		beam   = mb_swap_int(beam);
		action = mb_swap_int(action);
#endif
		if (fwrite(&time_d, sizeof(double), 1, sofp) != 1) {
			status = MB_FAILURE; *error = MB_ERROR_WRITE_FAIL;
		}
		if (status == MB_SUCCESS && fwrite(&beam, sizeof(int), 1, sofp) != 1) {
			status = MB_FAILURE; *error = MB_ERROR_WRITE_FAIL;
		}
		if (status == MB_SUCCESS && fwrite(&action, sizeof(int), 1, sofp) != 1) {
			status = MB_FAILURE; *error = MB_ERROR_WRITE_FAIL;
		}
	}

	if (verbose > 5) {
		/* keep the param-use linter quiet */
		(void)time_d;
	}
	return status;
}
/* end mbclean.c */
