/*--------------------------------------------------------------------
 *    The MB-system:  mbareaclean.c   (GMT-module rewrite of mbareaclean.cc)
 *
 *    Original mbareaclean.c (2003) by D. W. Caress.
 *    Re-converted from current upstream src/utilities/mbareaclean.cc back
 *    to C and wrapped as a GMT module entry so it can be invoked from
 *    the GMT API (and therefore from Julia FFI / Matlab MEX via GMT).
 *
 *    Copyright (c) 2003-2026 by
 *    David W. Caress (caress@mbari.org)  — MBARI
 *    Dale N. Chayes  — University of New Hampshire CCOM
 *    Christian dos Santos Ferreira  — MARUM
 *
 *    See README.md for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbareaclean identifies and flags artifacts in swath sonar bathymetry data
 * within a defined geographic area. Soundings inside the area are binned;
 * statistical tests (median, median-density, standard deviation) flag bad
 * (or unflag good) soundings. Edits are written to esf files.
 */

#define THIS_MODULE_NAME    "mbareaclean"
#define THIS_MODULE_LIB     "mbsystem"
#define THIS_MODULE_PURPOSE "Identifies and flags artifacts in swath bathymetry data over an area"
#define THIS_MODULE_KEYS    ""
#define THIS_MODULE_NEEDS		""
#define THIS_MODULE_OPTIONS		"-:>Vh"

#include "gmt_dev.h"

#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_info.h"
#include "mb_io.h"
#include "mb_swap.h"
#include "mb_process.h"

/* allocation */
#define FILEALLOCNUM 16
#define PINGALLOCNUM 128
#define SNDGALLOCNUM 128

struct mbareaclean_sndg_struct {
	int    sndg_file;
	int    sndg_ping;
	int    sndg_beam;
	double sndg_depth;
	double sndg_x;
	double sndg_y;
	char   sndg_beamflag_org;
	char   sndg_beamflag_esf;
	char   sndg_beamflag;
	bool   sndg_edit;
};

struct mbareaclean_file_struct {
	char   filelist[MB_PATH_MAXLINE];
	int    file_format;
	int    nping;
	int    nping_alloc;
	int    nnull;
	int    nflag;
	int    ngood;
	int    nunflagged;
	int    nflagged;
	double *ping_time_d;
	int    *pingmultiplicity;
	double *ping_altitude;
	int    nsndg;
	int    nsndg_alloc;
	int    sndg_countstart;
	int    beams_bath;
	struct mbareaclean_sndg_struct *sndg;
};

/* module-scope sounding storage (kept file-static to match original) */
static int nfile = 0;
static int nfile_alloc = 0;
static struct mbareaclean_file_struct *files = NULL;
static int nsndg_g = 0;
static int nsndg_alloc_g = 0;
static int **gsndg = NULL;
static int *gsndgnum = NULL;
static int *gsndgnum_alloc = NULL;

EXTERN_MSC int GMT_mbareaclean(void *API, int mode, void *args);

/* --- Control structure ----------------------------------------------- */

struct MBAREACLEAN_CTRL {
	struct mbac_B { bool active; } B;   /* output bad (flag) */
	struct mbac_D { bool active; double threshold; int nmin; } D;   /* std dev filter */
	struct mbac_F { bool active; int format; } F;
	struct mbac_G { bool active; } G;   /* output good (unflag) */
	struct mbac_I { bool active; char *inputfile; } I;
	struct mbac_M { bool active; double threshold; int nmin; int nmax; bool density; } M;  /* median */
	struct mbac_N { bool active; int min_beam; int max_beam_no; bool beam_in; } N;
	struct mbac_P { bool active; } P;   /* plane fit (parsed but not implemented in original) */
	struct mbac_R { bool active; double bounds[4]; } R;
	struct mbac_S { bool active; double binsize; } S;
	struct mbac_T { bool active; int flag_detect; } T;
};

/* --- forward decls --------------------------------------------------- */

static int getsoundingptr(int verbose, int soundingid,
                          struct mbareaclean_sndg_struct **sndgptr, int *error);
static int flag_sounding(int verbose, bool flag, bool output_bad, bool output_good,
                         struct mbareaclean_sndg_struct *s, int *error);

/* --- ctor / dtor / usage --------------------------------------------- */

static void *New_mbareaclean_Ctrl(struct GMT_CTRL *GMT) {
	struct MBAREACLEAN_CTRL *Ctrl = gmt_M_memory(GMT, NULL, 1, struct MBAREACLEAN_CTRL);

	/* defaults */
	Ctrl->D.threshold = 2.0;  Ctrl->D.nmin = 10;
	Ctrl->M.threshold = 0.25; Ctrl->M.nmin = 10; Ctrl->M.nmax = 0;
	Ctrl->T.flag_detect = MB_DETECT_AMPLITUDE;

	return Ctrl;
}

static void Free_mbareaclean_Ctrl(struct GMT_CTRL *GMT, struct MBAREACLEAN_CTRL *Ctrl) {
	if (!Ctrl) return;
	if (Ctrl->I.inputfile) free(Ctrl->I.inputfile);
	gmt_M_free(GMT, Ctrl);
}

static int usage(struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose(API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return GMT_NOERROR;

	GMT_Message(API, GMT_TIME_NONE,
	    "usage: mbareaclean [-Fformat -Iinfile -Rwest/east/south/north -B -G\n"
	    "\t-Sbinsize -Mthreshold[/nmin[/nmax]] -Dthreshold[/nmin]\n"
	    "\t-Ttype -N[-]minbeam/maxbeam] [-V -H]\n\n");
	if (level == GMT_SYNOPSIS) return GMT_PARSE_ERROR;

	GMT_Message(API, GMT_TIME_NONE, "\t<inputfile> is an MB-System datalist or single swath file.\n\n");
	return GMT_PARSE_ERROR;
}

/* --- parse ----------------------------------------------------------- */

static int parse(struct GMT_CTRL *GMT, struct MBAREACLEAN_CTRL *Ctrl, struct GMT_OPTION *options) {
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
			} else {
				GMT_Report(API, GMT_MSG_NORMAL, "Syntax error: only one input file is allowed.\n");
				n_errors++;
			}
			break;

		case 'B':
			Ctrl->B.active = true;
			break;

		case 'D': {
			double d1 = Ctrl->D.threshold;
			int i1 = Ctrl->D.nmin;
			n = sscanf(opt->arg, "%lf/%d", &d1, &i1);
			if (n > 0) { Ctrl->D.threshold = d1; Ctrl->D.nmin = i1; Ctrl->D.active = true; }
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -D option\n"); n_errors++; }
			break;
		}

		case 'F':
			n = sscanf(opt->arg, "%d", &Ctrl->F.format);
			if (n > 0) Ctrl->F.active = true;
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -F option\n"); n_errors++; }
			break;

		case 'G':
			Ctrl->G.active = true;
			break;

		case 'I':
			if (!gmt_access(GMT, opt->arg, R_OK)) {
				Ctrl->I.inputfile = strdup(opt->arg);
				Ctrl->I.active = true;
				n_files = 1;
			} else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -I option (file not found)\n"); n_errors++; }
			break;

		case 'M': {
			double d1 = Ctrl->M.threshold;
			int i1 = Ctrl->M.nmin, i2 = 0;
			n = sscanf(opt->arg, "%lf/%d/%d", &d1, &i1, &i2);
			if (n > 0) Ctrl->M.threshold = d1;
			if (n > 1) Ctrl->M.nmin = i1;
			if (n > 2) { Ctrl->M.density = true; Ctrl->M.nmax = i2; }
			Ctrl->M.active = true;
			break;
		}

		case 'N': {
			int min_beam = 0, max_beam_no = 0;
			Ctrl->N.beam_in = true;
			sscanf(opt->arg, "%d/%d", &min_beam, &max_beam_no);
			if (opt->arg[0] == '-') { min_beam = -min_beam; Ctrl->N.beam_in = false; }
			if (max_beam_no < 0) max_beam_no = -max_beam_no;
			Ctrl->N.min_beam = min_beam;
			Ctrl->N.max_beam_no = max_beam_no;
			Ctrl->N.active = true;
			break;
		}

		case 'P':
			Ctrl->P.active = true;
			break;

		case 'R':
			mb_get_bounds(opt->arg, Ctrl->R.bounds);
			Ctrl->R.active = true;
			break;

		case 'S':
			n = sscanf(opt->arg, "%lf", &Ctrl->S.binsize);
			if (n > 0) Ctrl->S.active = true;
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -S option\n"); n_errors++; }
			break;

		case 'T':
			n = sscanf(opt->arg, "%d", &Ctrl->T.flag_detect);
			if (n > 0) Ctrl->T.active = true;
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -T option\n"); n_errors++; }
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
#define Return(code)   { Free_mbareaclean_Ctrl(GMT, Ctrl); gmt_end_module(GMT, GMT_cpy); bailout(code); }

/* ====================================================================
 * GMT_mbareaclean  — GMT module entry point.
 * ==================================================================== */

int GMT_mbareaclean(void *V_API, int mode, void *args) {
	int   error = MB_ERROR_NO_ERROR;

	struct MBAREACLEAN_CTRL *Ctrl = NULL;
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

	Ctrl = New_mbareaclean_Ctrl(GMT);
	if ((error = parse(GMT, Ctrl, options)) != 0) Return (error);

	int    verbose = GMT->common.V.active;
	int    format, pings, lonflip;
	double bounds[4];
	int    btime_i[7], etime_i[7];
	double speedmin, timegap;
	int    status = mb_defaults(verbose, &format, &pings, &lonflip, bounds,
	                            btime_i, etime_i, &speedmin, &timegap);

	/* reset defaults but keep format/lonflip; honor user -F */
	if (Ctrl->F.active) format = Ctrl->F.format;
	else                format = 0;
	pings = 1;
	bounds[0] = -360.; bounds[1] = 360.; bounds[2] = -90.; bounds[3] = 90.;
	btime_i[0]=1962; btime_i[1]=2; btime_i[2]=21; btime_i[3]=10; btime_i[4]=30; btime_i[5]=0; btime_i[6]=0;
	etime_i[0]=2062; etime_i[1]=2; etime_i[2]=21; etime_i[3]=10; etime_i[4]=30; etime_i[5]=0; etime_i[6]=0;
	speedmin = 0.0;
	timegap = 1000000000.0;

	mb_path read_file;
	strcpy(read_file, Ctrl->I.active ? Ctrl->I.inputfile : "datalist.mb-1");

	bool   output_bad = Ctrl->B.active;
	bool   output_good = Ctrl->G.active;
	bool   median_filter = Ctrl->M.active;
	double median_filter_threshold = Ctrl->M.threshold;
	int    median_filter_nmin = Ctrl->M.nmin;
	bool   mediandensity_filter = Ctrl->M.density;
	int    mediandensity_filter_nmax = Ctrl->M.nmax;
	bool   std_dev_filter = Ctrl->D.active;
	double std_dev_threshold = Ctrl->D.threshold;
	int    std_dev_nmin = Ctrl->D.nmin;
	bool   plane_fit = Ctrl->P.active;
	bool   limit_beams = Ctrl->N.active;
	int    min_beam = Ctrl->N.min_beam;
	int    max_beam_no = Ctrl->N.max_beam_no;
	int    max_beam = max_beam_no;
	bool   beam_in = Ctrl->N.beam_in;
	if (max_beam < min_beam) max_beam = min_beam;
	bool   use_detect = Ctrl->T.active;
	int    flag_detect = Ctrl->T.flag_detect;
	double areabounds[4];
	bool   areaboundsset = Ctrl->R.active;
	if (areaboundsset) {
		areabounds[0] = Ctrl->R.bounds[0];
		areabounds[1] = Ctrl->R.bounds[1];
		areabounds[2] = Ctrl->R.bounds[2];
		areabounds[3] = Ctrl->R.bounds[3];
	}
	double binsize = Ctrl->S.active ? Ctrl->S.binsize : 0.0;
	bool   binsizeset = Ctrl->S.active;

	/* turn on median filter if nothing specified */
	if (!median_filter && !plane_fit && !std_dev_filter) median_filter = true;
	/* turn on output bad if nothing specified */
	if (!output_bad && !output_good) output_bad = true;

	int   formatread;
	void *datalist = NULL;

	/* if bounds not set, derive from datalist info */
	if (!areaboundsset) {
		formatread = format;
		struct mb_info_struct mb_info;
		memset(&mb_info, 0, sizeof(struct mb_info_struct));
		status &= mb_get_info_datalist(verbose, read_file, &formatread, &mb_info, lonflip, &error);
		areabounds[0] = mb_info.lon_min;
		areabounds[1] = mb_info.lon_max;
		areabounds[2] = mb_info.lat_min;
		areabounds[3] = mb_info.lat_max;
		if (!binsizeset) binsize = 0.2 * mb_info.altitude_max;
	}

	/* grid props */
	double mtodeglon, mtodeglat;
	mb_coor_scale(verbose, 0.5 * (areabounds[2] + areabounds[3]), &mtodeglon, &mtodeglat);
	if (binsize <= 0.0)
		binsize = (areabounds[1] - areabounds[0]) / 101 / mtodeglon;
	double dx = binsize * mtodeglon;
	double dy = binsize * mtodeglat;
	const int nx = 1 + (int)((areabounds[1] - areabounds[0]) / dx);
	const int ny = 1 + (int)((areabounds[3] - areabounds[2]) / dy);
	if (nx > 1 && ny > 1) {
		dx = (areabounds[1] - areabounds[0]) / (nx - 1);
		dy = (areabounds[3] - areabounds[2]) / (ny - 1);
	}

	nsndg_g = 0;
	nsndg_alloc_g = 0;
	status &= mb_mallocd(verbose, __FILE__, __LINE__, nx * ny * sizeof(int *), (void **)&gsndg, &error);
	if (status == MB_SUCCESS)
		status &= mb_mallocd(verbose, __FILE__, __LINE__, nx * ny * sizeof(int), (void **)&gsndgnum, &error);
	if (status == MB_SUCCESS)
		status &= mb_mallocd(verbose, __FILE__, __LINE__, nx * ny * sizeof(int), (void **)&gsndgnum_alloc, &error);
	if (error != MB_ERROR_NO_ERROR || status != MB_SUCCESS) {
		char *message = NULL;
		mb_error(verbose, error, &message);
		GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
		GMT_Report(API, GMT_MSG_NORMAL, "Program <%s> Terminated\n", THIS_MODULE_NAME);
		Return(error);
	}
	for (int i = 0; i < nx * ny; i++) {
		gsndg[i] = NULL;
		gsndgnum[i] = 0;
		gsndgnum_alloc[i] = 0;
	}

	if (verbose >= 0) {
		GMT_Report(API, GMT_MSG_NORMAL, "Area of interest:\n");
		GMT_Report(API, GMT_MSG_NORMAL, "     Minimum Longitude: %.6f Maximum Longitude: %.6f\n", areabounds[0], areabounds[1]);
		GMT_Report(API, GMT_MSG_NORMAL, "     Minimum Latitude:  %.6f Maximum Latitude:  %.6f\n", areabounds[2], areabounds[3]);
		GMT_Report(API, GMT_MSG_NORMAL, "     Bin Size:   %f\n", binsize);
		GMT_Report(API, GMT_MSG_NORMAL, "     Dimensions: %d %d\n", nx, ny);
	}

	if (format == 0) mb_get_format(verbose, read_file, NULL, &format, &error);

	const bool read_datalist = (format < 0);
	bool   read_data;
	char   swathfile[MB_PATH_MAXLINE];
	char   swathfileread[MB_PATH_MAXLINE];
	char   dfile[MB_PATH_MAXLINE];
	double file_weight;

	if (read_datalist) {
		const int look_processed = MB_DATALIST_LOOK_UNSET;
		if (mb_datalist_open(verbose, &datalist, read_file, look_processed, &error) != MB_SUCCESS) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open data list file: %s\n", read_file);
			Return(MB_ERROR_OPEN_FAIL);
		}
		read_data = (mb_datalist_read(verbose, datalist, swathfile, dfile, &format, &file_weight, &error) == MB_SUCCESS);
	} else {
		strcpy(swathfile, read_file);
		read_data = true;
	}

	void *mbio_ptr = NULL;
	double btime_d, etime_d;
	int    beams_bath, beams_amp, pixels_ss;
	char   *beamflag = NULL, *beamflagorg = NULL;
	int    *detect = NULL;
	double *bath = NULL, *amp = NULL, *bathlon = NULL, *bathlat = NULL;
	double *ss = NULL, *sslon = NULL, *sslat = NULL;

	char   esffile[MB_PATH_MAXLINE];
	struct mb_esf_struct esf;
	memset(&esf, 0, sizeof(struct mb_esf_struct));
	int    files_tot = 0;

	int    kind, pingsread;
	int    time_i[7];
	double time_d, navlon, navlat, speed, heading, distance, altitude, sensordepth;
	char   comment[MB_COMMENT_MAXLINE];
	void  *store_ptr = NULL;
	int    pingmultiplicity;
	int    pings_tot = 0, beams_tot = 0;
	int    beams_good_org_tot = 0, beams_flag_org_tot = 0, beams_null_org_tot = 0;

	struct mbareaclean_sndg_struct *sndg_p = NULL;

	while (read_data) {
		bool variable_beams, traveltime, beam_flagging;
		if ((status = mb_format_flags(verbose, &format, &variable_beams, &traveltime, &beam_flagging, &error)) != MB_SUCCESS) {
			char *message = NULL;
			mb_error(verbose, error, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error from mb_format_flags fmt %d:\n%s\n", format, message);
			Return(error);
		}

		strcpy(swathfileread, swathfile);
		formatread = format;
		if (!use_detect) mb_get_fbt(verbose, swathfileread, &formatread, &error);

		if (mb_read_init(verbose, swathfileread, formatread, pings, lonflip, bounds, btime_i, etime_i, speedmin,
		                 timegap, &mbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
			char *message = NULL;
			mb_error(verbose, error, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nmb_read_init failed: %s\nfile: %s\n", message, swathfileread);
			Return(error);
		}

		int pings_file = 0;
		GMT_Report(API, GMT_MSG_NORMAL, "\nProcessing %s\n", swathfileread);

		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(int), (void **)&detect, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlon, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlat, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslon, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslat, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflagorg, &error);

		if (error != MB_ERROR_NO_ERROR) {
			char *message = NULL;
			mb_error(verbose, error, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nmb_register_array failed: %s\n", message);
			Return(error);
		}

		if (nfile >= nfile_alloc) {
			nfile_alloc += FILEALLOCNUM;
			status &= mb_reallocd(verbose, __FILE__, __LINE__, nfile_alloc * sizeof(struct mbareaclean_file_struct),
			                      (void **)&files, &error);
			if (error != MB_ERROR_NO_ERROR) {
				char *message = NULL;
				mb_error(verbose, error, &message);
				GMT_Report(API, GMT_MSG_NORMAL, "\nrealloc files failed: %s\n", message);
				Return(error);
			}
		}

		strcpy(files[nfile].filelist, swathfile);
		files[nfile].file_format = format;
		files[nfile].nping = 0;
		files[nfile].nping_alloc = PINGALLOCNUM;
		files[nfile].nnull = 0;
		files[nfile].nflag = 0;
		files[nfile].ngood = 0;
		files[nfile].nflagged = 0;
		files[nfile].nunflagged = 0;
		files[nfile].ping_time_d = NULL;
		files[nfile].pingmultiplicity = NULL;
		files[nfile].ping_altitude = NULL;
		files[nfile].nsndg = 0;
		files[nfile].nsndg_alloc = SNDGALLOCNUM;
		files[nfile].sndg_countstart = nsndg_g;
		files[nfile].beams_bath = beams_bath;
		files[nfile].sndg = NULL;
		status &= mb_mallocd(verbose, __FILE__, __LINE__, files[nfile].nping_alloc * sizeof(double),
		                     (void **)&(files[nfile].ping_time_d), &error);
		if (status == MB_SUCCESS)
			status &= mb_mallocd(verbose, __FILE__, __LINE__, files[nfile].nping_alloc * sizeof(int),
			                     (void **)&(files[nfile].pingmultiplicity), &error);
		if (status == MB_SUCCESS)
			status &= mb_mallocd(verbose, __FILE__, __LINE__, files[nfile].nping_alloc * sizeof(double),
			                     (void **)&(files[nfile].ping_altitude), &error);
		if (status == MB_SUCCESS)
			status &= mb_mallocd(verbose, __FILE__, __LINE__,
			                     files[nfile].nsndg_alloc * sizeof(struct mbareaclean_sndg_struct),
			                     (void **)&(files[nfile].sndg), &error);
		if (error != MB_ERROR_NO_ERROR) {
			char *message = NULL;
			mb_error(verbose, error, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nfile alloc failed: %s\n", message);
			Return(error);
		}
		nfile++;

		if (status == MB_SUCCESS) {
			int esf_error = MB_ERROR_NO_ERROR;
			mb_esf_load(verbose, THIS_MODULE_NAME, swathfile, true, false, esffile, &esf, &esf_error);
		}

		bool done = false;
		files_tot++;
		pings_file = 0;
		int beams_good_org_file = 0, beams_flag_org_file = 0, beams_null_org_file = 0;

		while (!done) {
			error = MB_ERROR_NO_ERROR;
			status &= mb_read(verbose, mbio_ptr, &kind, &pingsread, time_i, &time_d, &navlon, &navlat, &speed, &heading,
			                  &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
			                  bathlon, bathlat, ss, sslon, sslat, comment, &error);

			if (status == MB_SUCCESS && kind == MB_DATA_DATA) {
				for (int i = 0; i < beams_bath; i++) beamflagorg[i] = beamflag[i];

				mb_get_store(verbose, mbio_ptr, &store_ptr, &error);
				int detect_error;
				const int detect_status = mb_detects(verbose, mbio_ptr, store_ptr, &kind, &beams_bath, detect, &detect_error);
				if (detect_status != MB_SUCCESS) {
					for (int i = 0; i < beams_bath; i++) detect[i] = MB_DETECT_UNKNOWN;
				}
				int sensorhead;
				int sensorhead_error = MB_ERROR_NO_ERROR;
				const int sensorhead_status = mb_sensorhead(verbose, mbio_ptr, store_ptr, &sensorhead, &sensorhead_error);

				if (files[nfile - 1].nping >= files[nfile - 1].nping_alloc) {
					files[nfile - 1].nping_alloc += PINGALLOCNUM;
					status = mb_reallocd(verbose, __FILE__, __LINE__, files[nfile - 1].nping_alloc * sizeof(double),
					                     (void **)&(files[nfile - 1].ping_time_d), &error);
					if (status == MB_SUCCESS)
						status = mb_reallocd(verbose, __FILE__, __LINE__, files[nfile - 1].nping_alloc * sizeof(int),
						                     (void **)&(files[nfile - 1].pingmultiplicity), &error);
					if (status == MB_SUCCESS)
						mb_reallocd(verbose, __FILE__, __LINE__, files[nfile - 1].nping_alloc * sizeof(double),
						            (void **)&(files[nfile - 1].ping_altitude), &error);
					if (error != MB_ERROR_NO_ERROR) {
						char *message = NULL;
						mb_error(verbose, error, &message);
						GMT_Report(API, GMT_MSG_NORMAL, "\nrealloc per-file ping arrays failed: %s\n", message);
						Return(error);
					}
				}

				if (sensorhead_status == MB_SUCCESS) {
					pingmultiplicity = sensorhead;
				} else if (files[nfile - 1].nping > 0
				           && fabs(time_d - files[nfile - 1].ping_time_d[files[nfile - 1].nping - 1]) < MB_ESF_MAXTIMEDIFF) {
					pingmultiplicity = files[nfile - 1].pingmultiplicity[files[nfile - 1].nping - 1] + 1;
				} else {
					pingmultiplicity = 0;
				}
				status = mb_esf_apply(verbose, &esf, time_d, pingmultiplicity, beams_bath, beamflagorg, &error);

				pings_tot++;
				pings_file++;
				for (int i = 0; i < beams_bath; i++) {
					if (mb_beam_ok(beamflagorg[i])) {
						beams_tot++; beams_good_org_tot++; beams_good_org_file++;
						files[nfile - 1].ngood++;
					} else if (beamflagorg[i] == MB_FLAG_NULL) {
						beams_null_org_tot++; beams_null_org_file++;
						files[nfile - 1].nnull++;
					} else {
						beams_tot++; beams_flag_org_tot++; beams_flag_org_file++;
						files[nfile - 1].nflag++;
					}
				}

				files[nfile - 1].ping_time_d[files[nfile - 1].nping] = time_d;
				files[nfile - 1].pingmultiplicity[files[nfile - 1].nping] = pingmultiplicity;
				files[nfile - 1].ping_altitude[files[nfile - 1].nping] = altitude;
				files[nfile - 1].nping++;

				if (limit_beams && max_beam_no == 0) max_beam = beams_bath - min_beam;

				for (int ib = 0; ib < beams_bath; ib++) {
					if (beamflagorg[ib] != MB_FLAG_NULL) {
						const int ix = (int)((bathlon[ib] - areabounds[0] - 0.5 * dx) / dx);
						const int iy = (int)((bathlat[ib] - areabounds[2] - 0.5 * dy) / dy);
						const int kgrid = ix * ny + iy;

						if (ix >= 0 && ix < nx && iy >= 0 && iy < ny) {
							if (files[nfile - 1].nsndg >= files[nfile - 1].nsndg_alloc) {
								files[nfile - 1].nsndg_alloc += SNDGALLOCNUM;
								status = mb_reallocd(verbose, __FILE__, __LINE__,
								                     files[nfile - 1].nsndg_alloc * sizeof(struct mbareaclean_sndg_struct),
								                     (void **)&files[nfile - 1].sndg, &error);
								if (error != MB_ERROR_NO_ERROR) {
									char *message = NULL;
									mb_error(verbose, error, &message);
									GMT_Report(API, GMT_MSG_NORMAL, "\nrealloc sndg failed: %s\n", message);
									Return(error);
								}
							}
							if (gsndgnum[kgrid] >= gsndgnum_alloc[kgrid]) {
								gsndgnum_alloc[kgrid] += SNDGALLOCNUM;
								status = mb_reallocd(verbose, __FILE__, __LINE__, gsndgnum_alloc[kgrid] * sizeof(int),
								                     (void **)&gsndg[kgrid], &error);
								if (error != MB_ERROR_NO_ERROR) {
									char *message = NULL;
									mb_error(verbose, error, &message);
									GMT_Report(API, GMT_MSG_NORMAL, "\nrealloc gsndg failed: %s\n", message);
									Return(error);
								}
							}

							sndg_p = &(files[nfile - 1].sndg[files[nfile - 1].nsndg]);
							sndg_p->sndg_file = nfile - 1;
							sndg_p->sndg_ping = files[nfile - 1].nping - 1;
							sndg_p->sndg_beam = ib;
							sndg_p->sndg_depth = bath[ib];
							sndg_p->sndg_x = bathlon[ib];
							sndg_p->sndg_y = bathlat[ib];
							sndg_p->sndg_beamflag_org = beamflag[ib];
							sndg_p->sndg_beamflag_esf = beamflagorg[ib];
							sndg_p->sndg_beamflag = beamflagorg[ib];
							sndg_p->sndg_edit = true;
							if (use_detect && detect[ib] != flag_detect) sndg_p->sndg_edit = false;
							if (limit_beams) {
								if (min_beam <= ib && ib <= max_beam) {
									if (!beam_in) sndg_p->sndg_edit = false;
								} else {
									if (beam_in) sndg_p->sndg_edit = false;
								}
							}
							files[nfile - 1].nsndg++;
							nsndg_g++;
							gsndg[kgrid][gsndgnum[kgrid]] = files[nfile - 1].sndg_countstart + files[nfile - 1].nsndg - 1;
							gsndgnum[kgrid]++;
						}
					}
				}
			} else if (error > MB_ERROR_NO_ERROR) {
				done = true;
			}
		}

		status = mb_close(verbose, &mbio_ptr, &error);
		mb_esf_close(verbose, &esf, &error);

		if (verbose >= 4) status &= mb_memory_list(verbose, &error);

		GMT_Report(API, GMT_MSG_NORMAL, "pings:%4d  beams: %7d good %7d flagged %7d null \n",
		           pings_file, beams_good_org_file, beams_flag_org_file, beams_null_org_file);

		if (read_datalist) {
			read_data = (mb_datalist_read(verbose, datalist, swathfile, dfile, &format, &file_weight, &error) == MB_SUCCESS);
		} else {
			read_data = false;
		}
	}
	if (read_datalist) mb_datalist_close(verbose, &datalist, &error);

	/* find max bin count */
	double *bindepths = NULL;
	double threshold;
	int    binnummax = 0;
	for (int ix = 0; ix < nx; ix++) {
		for (int iy = 0; iy < ny; iy++) {
			const int kgrid = ix * ny + iy;
			if (gsndgnum[kgrid] > binnummax) binnummax = gsndgnum[kgrid];
		}
	}
	mb_mallocd(verbose, __FILE__, __LINE__, binnummax * sizeof(double), (void **)&bindepths, &error);
	if (error != MB_ERROR_NO_ERROR) {
		char *message = NULL;
		mb_error(verbose, error, &message);
		GMT_Report(API, GMT_MSG_NORMAL, "\nbindepths alloc failed: %s\n", message);
		Return(error);
	}

	/* median filter */
	if (median_filter) {
		for (int ix = 0; ix < nx; ix++)
			for (int iy = 0; iy < ny; iy++) {
				const int kgrid = ix * ny + iy;
				int binnum = 0;
				for (int i = 0; i < gsndgnum[kgrid]; i++) {
					getsoundingptr(verbose, gsndg[kgrid][i], &sndg_p, &error);
					if (mb_beam_ok(sndg_p->sndg_beamflag)) {
						bindepths[binnum] = sndg_p->sndg_depth;
						binnum++;
					}
				}
				if (binnum >= median_filter_nmin) {
					qsort((void *)bindepths, binnum, sizeof(double), mb_double_compare);
					const double median_depth = bindepths[binnum / 2];
					double median_depth_low, median_depth_high;
					if (mediandensity_filter && binnum / 2 - mediandensity_filter_nmax / 2 >= 0)
						median_depth_low = bindepths[binnum / 2 + mediandensity_filter_nmax / 2];
					else
						median_depth_low = bindepths[0];
					if (mediandensity_filter && binnum / 2 + mediandensity_filter_nmax / 2 < binnum)
						median_depth_high = bindepths[binnum / 2 + mediandensity_filter_nmax / 2];
					else
						median_depth_high = bindepths[binnum - 1];

					for (int i = 0; i < gsndgnum[kgrid]; i++) {
						getsoundingptr(verbose, gsndg[kgrid][i], &sndg_p, &error);
						threshold = fabs(median_filter_threshold * files[sndg_p->sndg_file].ping_altitude[sndg_p->sndg_ping]);
						bool flagsounding = false;
						if (fabs(sndg_p->sndg_depth - median_depth) > threshold) flagsounding = true;
						if (mediandensity_filter &&
						    (sndg_p->sndg_depth > median_depth_high || sndg_p->sndg_depth < median_depth_low))
							flagsounding = true;
						flag_sounding(verbose, flagsounding, output_bad, output_good, sndg_p, &error);
					}
				}
			}
	}

	/* std dev filter */
	if (std_dev_filter) {
		for (int ix = 0; ix < nx; ix++)
			for (int iy = 0; iy < ny; iy++) {
				const int kgrid = ix * ny + iy;
				const double xx = areabounds[0] + 0.5 * dx + ix * dx;
				const double yy = areabounds[3] + 0.5 * dy + iy * dy;
				double mean = 0.0;
				int    binnum = 0;
				for (int i = 0; i < gsndgnum[kgrid]; i++) {
					getsoundingptr(verbose, gsndg[kgrid][i], &sndg_p, &error);
					if (mb_beam_ok(sndg_p->sndg_beamflag)) { mean += sndg_p->sndg_depth; binnum++; }
				}
				if (binnum > 0) mean /= binnum;
				double std_dev = 0.0;
				for (int i = 0; i < gsndgnum[kgrid]; i++) {
					getsoundingptr(verbose, gsndg[kgrid][i], &sndg_p, &error);
					if (mb_beam_ok(sndg_p->sndg_beamflag))
						std_dev += (sndg_p->sndg_depth - mean) * (sndg_p->sndg_depth - mean);
				}
				if (binnum > 0) std_dev = sqrt(std_dev / binnum);
				threshold = std_dev * std_dev_threshold;

				if (binnum > 0)
					GMT_Report(API, GMT_MSG_NORMAL,
					           "bin: %d %d %d  pos: %f %f  nsoundings:%d / %d mean:%f std_dev:%f\n",
					           ix, iy, kgrid, xx, yy, binnum, gsndgnum[kgrid], mean, std_dev);

				if (binnum >= std_dev_nmin) {
					for (int i = 0; i < gsndgnum[kgrid]; i++) {
						getsoundingptr(verbose, gsndg[kgrid][i], &sndg_p, &error);
						flag_sounding(verbose,
						              fabs(sndg_p->sndg_depth - mean) > threshold,
						              output_bad, output_good, sndg_p, &error);
					}
				}
			}
	}

	/* write changed soundings */
	for (int i = 0; i < nfile; i++) {
		status = mb_esf_load(verbose, THIS_MODULE_NAME, files[i].filelist, false, true, esffile, &esf, &error);
		bool esffile_open = false;
		if (status == MB_SUCCESS && esf.esffp != NULL) esffile_open = true;
		if (status == MB_FAILURE && error == MB_ERROR_OPEN_FAIL) {
			esffile_open = false;
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open new edit save file %s\n", esf.esffile);
		}

		for (int j = 0; j < files[i].nsndg; j++) {
			sndg_p = &(files[i].sndg[j]);
			if (sndg_p->sndg_beamflag != sndg_p->sndg_beamflag_org) {
				int action = 0;
				if (mb_beam_ok(sndg_p->sndg_beamflag))                  action = MBP_EDIT_UNFLAG;
				else if (mb_beam_check_flag_manual(sndg_p->sndg_beamflag)) action = MBP_EDIT_FLAG;
				else if (mb_beam_check_flag_filter(sndg_p->sndg_beamflag)) action = MBP_EDIT_FILTER;
				mb_esf_save(verbose, &esf, files[i].ping_time_d[sndg_p->sndg_ping],
				            sndg_p->sndg_beam + files[i].pingmultiplicity[sndg_p->sndg_ping] * MB_ESF_MULTIPLICITY_FACTOR,
				            action, &error);
			}
		}

		mb_esf_close(verbose, &esf, &error);

		if (esffile_open) {
			status &= mb_pr_update_format(verbose, files[i].filelist, true, files[i].file_format, &error);
			status &= mb_pr_update_edit(verbose, files[i].filelist, MBP_EDIT_ON, esffile, &error);
		}
	}

	GMT_Report(API, GMT_MSG_NORMAL, "\nMBareaclean Processing Totals:\n-------------------------\n");
	GMT_Report(API, GMT_MSG_NORMAL, "%d total swath data files processed\n", files_tot);
	GMT_Report(API, GMT_MSG_NORMAL, "%d total pings processed\n", pings_tot);
	GMT_Report(API, GMT_MSG_NORMAL, "%d total soundings processed\n-------------------------\n", beams_tot);
	for (int i = 0; i < nfile; i++) {
		GMT_Report(API, GMT_MSG_NORMAL, "%3d soundings:%7d flagged:%7d unflagged:%7d  file:%s\n",
		           i, files[i].ngood + files[i].nflag, files[i].nflagged, files[i].nunflagged, files[i].filelist);
	}

	mb_freed(verbose, __FILE__, __LINE__, (void **)&bindepths, &error);
	for (int i = 0; i < nx * ny; i++)
		if (gsndg[i] != NULL) mb_freed(verbose, __FILE__, __LINE__, (void **)&gsndg[i], &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&gsndg, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&gsndgnum, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&gsndgnum_alloc, &error);

	for (int i = 0; i < nfile; i++) {
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(files[i].ping_time_d), &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(files[i].pingmultiplicity), &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(files[i].ping_altitude), &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(files[i].sndg), &error);
	}
	mb_freed(verbose, __FILE__, __LINE__, (void **)&files, &error);

	if (verbose >= 4) status &= mb_memory_list(verbose, &error);

	/* reset module-static state so a second invocation starts clean */
	nfile = 0; nfile_alloc = 0; files = NULL;
	nsndg_g = 0; nsndg_alloc_g = 0;
	gsndg = NULL; gsndgnum = NULL; gsndgnum_alloc = NULL;

	Return(GMT_NOERROR);
}

/* --------------------------------------------------------------------
 * helpers
 * -------------------------------------------------------------------- */

static int getsoundingptr(int verbose, int soundingid,
                          struct mbareaclean_sndg_struct **sndgptr, int *error) {
	(void)verbose;
	*sndgptr = NULL;
	for (int i = 0; i < nfile && *sndgptr == NULL; i++) {
		if (soundingid >= files[i].sndg_countstart &&
		    soundingid <  files[i].sndg_countstart + files[i].nsndg) {
			const int j = soundingid - files[i].sndg_countstart;
			*sndgptr = &(files[i].sndg[j]);
		}
	}
	*error = MB_ERROR_NO_ERROR;
	return MB_SUCCESS;
}

static int flag_sounding(int verbose, bool flag, bool output_bad, bool output_good,
                         struct mbareaclean_sndg_struct *s, int *error) {
	(void)verbose;
	if (s->sndg_edit) {
		if (output_bad && mb_beam_ok(s->sndg_beamflag) && flag) {
			s->sndg_beamflag = MB_FLAG_FLAG + MB_FLAG_FILTER;
			files[s->sndg_file].nflagged++;
		} else if (output_good && !mb_beam_ok(s->sndg_beamflag) &&
		           s->sndg_beamflag != MB_FLAG_NULL && !flag) {
			s->sndg_beamflag = MB_FLAG_NONE;
			files[s->sndg_file].nunflagged++;
		} else if (output_good && !mb_beam_ok(s->sndg_beamflag) &&
		           s->sndg_beamflag != MB_FLAG_NULL && flag) {
			s->sndg_edit = false;
		}
	}
	*error = MB_ERROR_NO_ERROR;
	return MB_SUCCESS;
}
/* end mbareaclean.c */
