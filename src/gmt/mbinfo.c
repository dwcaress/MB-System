/*--------------------------------------------------------------------
 *    The MB-system:  mbinfo.c   (GMT-module rewrite of mbinfo.cc)
 *
 *    Original mbinfo.c (1993) by D. W. Caress.
 *    Re-converted from current upstream src/utilities/mbinfo.cc back
 *    to C and wrapped as a GMT module entry so it can be invoked from
 *    the GMT API (and therefore from Julia FFI / Matlab MEX via GMT).
 *
 *    Copyright (c) 1993-2026 by
 *    David W. Caress (caress@mbari.org)  — MBARI
 *    Dale N. Chayes  — University of New Hampshire CCOM
 *    Christian dos Santos Ferreira  — MARUM
 *
 *    See README.md for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * MBINFO reads a swath sonar data file and outputs basic statistics.
 * Full parity with mbinfo.cc: variance computation (-P), JSON/XML output
 * (-X), coverage mask (-M), debug record options, metadata parsing.
 */

#define THIS_MODULE_NAME    "mbinfo"
#define THIS_MODULE_LIB		"mbsystem"
#define THIS_MODULE_PURPOSE "Read swath sonar data file and output statistics"
#define THIS_MODULE_KEYS    ""
#define THIS_MODULE_NEEDS		""
#define THIS_MODULE_OPTIONS		"->V"

#include "gmt_dev.h"

#include "mb_define.h"
#include "mb_info.h"
#include "mb_io.h"
#include "mb_status.h"

#define MBINFO_MAXPINGS 50

typedef enum {
	FREE_TEXT = 0,
	JSON = 1,
	XML = 2,
	MAX_OUTPUT_FORMAT = 2
} output_format_t;

struct mbi_ping {
	char   *beamflag;
	double *bath;
	double *bathlon;
	double *bathlat;
	double *amp;
	double *ss;
	double *sslon;
	double *sslat;
};

/* --- Control structure ----------------------------------------------- */

struct MBINFO_CTRL {
	struct mbi_B { bool active; int t[7]; } B;
	struct mbi_C { bool active; } C;   /* comments */
	struct mbi_E { bool active; int t[7]; } E;
	struct mbi_F { bool active; int format; } F;
	struct mbi_G { bool active; } G;   /* good-nav-only */
	struct mbi_I { bool active; char *inputfile; } I;
	struct mbi_L { bool active; int lonflip; } L;
	struct mbi_M { bool active; int nx, ny; double bounds[4]; bool bounds_set; } M;
	struct mbi_N { bool active; } N;   /* print notices */
	struct mbi_O { bool active; } O;   /* output to file */
	struct mbi_P { bool active; int pings; } P;
	struct mbi_Q { bool active; } Q;   /* quick */
	struct mbi_R { bool active; double bounds[4]; } R;
	struct mbi_S { bool active; double speedmin; } S;
	struct mbi_T { bool active; double timegap; } T;
	struct mbi_W { bool active; } W;   /* use feet */
	struct mbi_X { bool active; output_format_t output_format; } X;
};

static void *New_mbinfo_Ctrl(struct GMT_CTRL *GMT) {
	struct MBINFO_CTRL *Ctrl = gmt_M_memory(GMT, NULL, 1, struct MBINFO_CTRL);
	Ctrl->X.output_format = FREE_TEXT;
	return Ctrl;
}

static void Free_mbinfo_Ctrl(struct GMT_CTRL *GMT, struct MBINFO_CTRL *Ctrl) {
	if (!Ctrl) return;
	if (Ctrl->I.inputfile) free(Ctrl->I.inputfile);
	gmt_M_free(GMT, Ctrl);
}

static int usage(struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose(API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return GMT_NOERROR;
	GMT_Message(API, GMT_TIME_NONE,
	    "usage: mbinfo [-Byr/mo/da/hr/mn/sc -C -Eyr/mo/da/hr/mn/sc -Fformat -G\n"
	    "\t-Ifile -Llonflip -Mnx/ny[/w/e/s/n] -N -O -Ppings -Q\n"
	    "\t-Rw/e/s/n -Sspeed -Ttimegap -W -Xfmt -V -H]\n\n");
	if (level == GMT_SYNOPSIS) return GMT_PARSE_ERROR;
	return GMT_PARSE_ERROR;
}

static int parse(struct GMT_CTRL *GMT, struct MBINFO_CTRL *Ctrl, struct GMT_OPTION *options) {
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
			} else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax -< option\n"); n_errors++; }
			break;
		case 'B':
			Ctrl->B.t[6] = 0;
			n = sscanf(opt->arg, "%d/%d/%d/%d/%d/%d",
			           &Ctrl->B.t[0], &Ctrl->B.t[1], &Ctrl->B.t[2],
			           &Ctrl->B.t[3], &Ctrl->B.t[4], &Ctrl->B.t[5]);
			if (n == 6) Ctrl->B.active = true; else n_errors++;
			break;
		case 'C': Ctrl->C.active = true; break;
		case 'E':
			Ctrl->E.t[6] = 0;
			n = sscanf(opt->arg, "%d/%d/%d/%d/%d/%d",
			           &Ctrl->E.t[0], &Ctrl->E.t[1], &Ctrl->E.t[2],
			           &Ctrl->E.t[3], &Ctrl->E.t[4], &Ctrl->E.t[5]);
			if (n == 6) Ctrl->E.active = true; else n_errors++;
			break;
		case 'F':
			n = sscanf(opt->arg, "%d", &Ctrl->F.format);
			if (n > 0) Ctrl->F.active = true; else n_errors++;
			break;
		case 'G': Ctrl->G.active = true; break;
		case 'I':
			if (!gmt_access(GMT, opt->arg, R_OK)) {
				Ctrl->I.inputfile = strdup(opt->arg);
				Ctrl->I.active = true;
				n_files = 1;
			} else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax -I option (file not found)\n"); n_errors++; }
			break;
		case 'L':
			n = sscanf(opt->arg, "%d", &Ctrl->L.lonflip);
			if (n > 0) Ctrl->L.active = true; else n_errors++;
			break;
		case 'M': {
			n = sscanf(opt->arg, "%d/%d/%lf/%lf/%lf/%lf",
			           &Ctrl->M.nx, &Ctrl->M.ny,
			           &Ctrl->M.bounds[0], &Ctrl->M.bounds[1],
			           &Ctrl->M.bounds[2], &Ctrl->M.bounds[3]);
			if (n >= 2 && Ctrl->M.nx > 0 && Ctrl->M.ny > 0) Ctrl->M.active = true;
			if (n == 6 && Ctrl->M.bounds[1] > Ctrl->M.bounds[0] && Ctrl->M.bounds[3] > Ctrl->M.bounds[2])
				Ctrl->M.bounds_set = true;
			break;
		}
		case 'N': Ctrl->N.active = true; break;
		case 'O': Ctrl->O.active = true; break;
		case 'P':
			n = sscanf(opt->arg, "%d", &Ctrl->P.pings);
			if (n > 0) Ctrl->P.active = true; else n_errors++;
			break;
		case 'Q': Ctrl->Q.active = true; break;
		case 'R':
			mb_get_bounds(opt->arg, Ctrl->R.bounds);
			Ctrl->R.active = true;
			break;
		case 'S':
			n = sscanf(opt->arg, "%lf", &Ctrl->S.speedmin);
			if (n > 0) Ctrl->S.active = true; else n_errors++;
			break;
		case 'T':
			n = sscanf(opt->arg, "%lf", &Ctrl->T.timegap);
			if (n > 0) Ctrl->T.active = true; else n_errors++;
			break;
		case 'W': Ctrl->W.active = true; break;
		case 'X': {
			int tmp;
			n = sscanf(opt->arg, "%d", &tmp);
			if (n > 0 && tmp >= 0 && tmp <= MAX_OUTPUT_FORMAT) {
				Ctrl->X.output_format = (output_format_t)tmp;
				Ctrl->X.active = true;
			} else { GMT_Report(API, GMT_MSG_NORMAL, "Invalid output format for -X\n"); n_errors++; }
			break;
		}
		default:
			n_errors += gmt_default_error(GMT, opt->option);
			break;
		}
	}

	n_errors += gmt_M_check_condition(GMT, n_files != 1,
	                                  "Syntax: Must specify one input file\n");
	return n_errors ? GMT_PARSE_ERROR : GMT_OK;
}

#define bailout(code)  { gmt_M_free_options(mode); return (code); }
#define Return(code)   { Free_mbinfo_Ctrl(GMT, Ctrl); gmt_end_module(GMT, GMT_cpy); bailout(code); }

/* helper: emit JSON metadata (single-row pattern from mbinfo.cc) */
static void emit_meta_json(FILE *out, int *meta, const char *tag, const char *value) {
	if (*meta == 0) fprintf(out, "\"%s\":\"%s\",\n", tag, value);
	(*meta)++;
}

int GMT_mbinfo(void *V_API, int mode, void *args) {
	int    error = MB_ERROR_NO_ERROR;

	struct MBINFO_CTRL *Ctrl = NULL;
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

	Ctrl = New_mbinfo_Ctrl(GMT);
	if ((error = parse(GMT, Ctrl, options)) != 0) Return (error);

	int verbose = GMT->common.V.active;
	int format, pings_get = 1, lonflip;
	double bounds[4];
	int btime_i[7], etime_i[7];
	double speedmin, timegap;
	int status = mb_defaults(verbose, &format, &pings_get, &lonflip, bounds,
	                         btime_i, etime_i, &speedmin, &timegap);

	if (Ctrl->F.active) format = Ctrl->F.format;
	if (Ctrl->L.active) lonflip = Ctrl->L.lonflip;
	if (Ctrl->S.active) speedmin = Ctrl->S.speedmin;
	if (Ctrl->T.active) timegap = Ctrl->T.timegap;
	if (Ctrl->R.active) for (int i = 0; i < 4; i++) bounds[i] = Ctrl->R.bounds[i];
	if (Ctrl->B.active) for (int i = 0; i < 7; i++) btime_i[i] = Ctrl->B.t[i];
	if (Ctrl->E.active) for (int i = 0; i < 7; i++) etime_i[i] = Ctrl->E.t[i];

	mb_path read_file;
	strcpy(read_file, Ctrl->I.active ? Ctrl->I.inputfile : "stdin");

	bool   quick = Ctrl->Q.active;
	bool   comments = Ctrl->C.active;
	bool   good_nav_only = Ctrl->G.active;
	bool   lonflip_set = Ctrl->L.active;
	int    lonflip_use = lonflip;
	bool   coverage_mask = Ctrl->M.active;
	bool   coverage_mask_bounds = Ctrl->M.bounds_set;
	int    mask_nx = Ctrl->M.nx, mask_ny = Ctrl->M.ny;
	double maskbounds[4] = {Ctrl->M.bounds[0], Ctrl->M.bounds[1], Ctrl->M.bounds[2], Ctrl->M.bounds[3]};
	bool   print_notices = Ctrl->N.active;
	bool   output_usefile = Ctrl->O.active;
	int    pings_read = Ctrl->P.active ? Ctrl->P.pings : 1;
	if (pings_read < 1) pings_read = 1;
	if (pings_read > MBINFO_MAXPINGS) pings_read = MBINFO_MAXPINGS;
	bool   bathy_in_meters = !Ctrl->W.active;
	output_format_t output_format = Ctrl->X.output_format;

	int    timbeg_i[7] = {0}, timend_i[7] = {0};
	int    notice_list[MB_NOTICE_MAX] = {0};
	int    notice_list_tot[MB_NOTICE_MAX] = {0};

	if (format == 0) mb_get_format(verbose, read_file, NULL, &format, &error);

	const double bathy_scale = bathy_in_meters ? 1.0 : (1.0 / 0.3048);
	const bool   read_datalist = (format < 0);
	bool   read_data;

	if (read_datalist || quick) pings_read = 1;

	FILE *stream = (verbose <= 1) ? stdout : stderr;
		
	if (verbose == 1) {
		fprintf(stream, "\nProgram %s\n", THIS_MODULE_NAME);
		fprintf(stream, "MB-system Version %s\n", MB_VERSION);
	}

	if (verbose >= 2) {
		fprintf(stream, "\ndbg2  Program <%s>\n", THIS_MODULE_NAME);
		fprintf(stream, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stream, "dbg2  Control Parameters:\n");
		fprintf(stream, "dbg2       verbose:    %d\n", verbose);
		fprintf(stream, "dbg2       format:     %d\n", format);
		fprintf(stream, "dbg2       pings:      %d\n", pings_read);
		fprintf(stream, "dbg2       lonflip:    %d\n", lonflip);
		fprintf(stream, "dbg2       bounds[0]:  %f\n", bounds[0]);
		fprintf(stream, "dbg2       bounds[1]:  %f\n", bounds[1]);
		fprintf(stream, "dbg2       bounds[2]:  %f\n", bounds[2]);
		fprintf(stream, "dbg2       bounds[3]:  %f\n", bounds[3]);
		fprintf(stream, "dbg2       btime_i[0]: %d\n", btime_i[0]);
		fprintf(stream, "dbg2       btime_i[1]: %d\n", btime_i[1]);
		fprintf(stream, "dbg2       btime_i[2]: %d\n", btime_i[2]);
		fprintf(stream, "dbg2       btime_i[3]: %d\n", btime_i[3]);
		fprintf(stream, "dbg2       btime_i[4]: %d\n", btime_i[4]);
		fprintf(stream, "dbg2       btime_i[5]: %d\n", btime_i[5]);
		fprintf(stream, "dbg2       btime_i[6]: %d\n", btime_i[6]);
		fprintf(stream, "dbg2       etime_i[0]: %d\n", etime_i[0]);
		fprintf(stream, "dbg2       etime_i[1]: %d\n", etime_i[1]);
		fprintf(stream, "dbg2       etime_i[2]: %d\n", etime_i[2]);
		fprintf(stream, "dbg2       etime_i[3]: %d\n", etime_i[3]);
		fprintf(stream, "dbg2       etime_i[4]: %d\n", etime_i[4]);
		fprintf(stream, "dbg2       etime_i[5]: %d\n", etime_i[5]);
		fprintf(stream, "dbg2       etime_i[6]: %d\n", etime_i[6]);
		fprintf(stream, "dbg2       speedmin:   %f\n", speedmin);
		fprintf(stream, "dbg2       timegap:    %f\n", timegap);
		fprintf(stream, "dbg2       good_nav:   %d\n", good_nav_only);
		fprintf(stream, "dbg2       comments:   %d\n", comments);
		fprintf(stream, "dbg2       file:       %s\n", read_file);
		fprintf(stream, "dbg2       quick:      %d\n", quick);
		fprintf(stream, "dbg2       bathy meters:%d\n", bathy_in_meters);
		fprintf(stream, "dbg2       lonflip_set:%d\n", lonflip_set);
		fprintf(stream, "dbg2       coverage:   %d\n", coverage_mask);
		if (coverage_mask) {
			fprintf(stream, "dbg2       mask_nx:    %d\n", mask_nx);
			fprintf(stream, "dbg2       mask_ny:    %d\n", mask_ny);
		}
		if (coverage_mask_bounds) {
			fprintf(stream, "dbg2       maskbounds[0]: %f\n", maskbounds[0]);
			fprintf(stream, "dbg2       maskbounds[1]: %f\n", maskbounds[1]);
			fprintf(stream, "dbg2       maskbounds[2]: %f\n", maskbounds[2]);
			fprintf(stream, "dbg2       maskbounds[3]: %f\n", maskbounds[3]);
		}
	}

	FILE *output = NULL;
	if (output_usefile) {
		char output_file[MB_PATH_MAXLINE];
		strcpy(output_file, read_file);
		switch (output_format) {
		case FREE_TEXT: strcat(output_file, ".inf"); break;
		case JSON:      strcat(output_file, "_inf.json"); break;
		case XML:       strcat(output_file, "_inf.xml"); break;
		default: break;
		}
		if ((output = fopen(output_file, "w")) == NULL) output = stream;
	} else output = stream;

	switch (output_format) {
	case FREE_TEXT: break;
	case JSON: fprintf(output, "{\n"); break;
	case XML:
		fprintf(output, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<mbinfo>\n");
		break;
	default: break;
	}

	char format_description[MB_DESCRIPTION_LENGTH];
	int pings;
	double file_weight;
	double btime_d, etime_d;
	char dpath_share[MB_PATH_MAXLINE];
	int beams_bath_alloc = 0, beams_amp_alloc = 0, pixels_ss_alloc = 0;
	int beams_bath_max = 0, beams_amp_max = 0, pixels_ss_max = 0;
	int beams_bath = 0, beams_amp = 0, pixels_ss = 0;

	struct mbi_ping data[MBINFO_MAXPINGS];
	struct mbi_ping *datacur;
	int    time_i[7];
	double time_d, navlon, navlat, speed, heading, distance, altitude, sensordepth;
	char   *beamflag = NULL;
	double *bath = NULL, *bathlon = NULL, *bathlat = NULL;
	double *amp = NULL, *ss = NULL, *sslon = NULL, *sslat = NULL;
	char   comment[MB_COMMENT_MAXLINE];
	int    icomment = 0;

	double speed_threshold = 50.0;
	double lonmin = 0.0, lonmax = 0.0, latmin = 0.0, latmax = 0.0;
	double sdpmin = 0.0, sdpmax = 0.0, altmin = 0.0, altmax = 0.0;
	double bathmin = 0.0, bathmax = 0.0;
	double ampmin = 0.0, ampmax = 0.0, ssmin = 0.0, ssmax = 0.0;
	double bathbeg = 0.0, bathend = 0.0;
	double lonbeg = 0.0, latbeg = 0.0, lonend = 0.0, latend = 0.0;
	double spdbeg = 0.0, hdgbeg = 0.0, sdpbeg = 0.0, altbeg = 0.0;
	double spdend = 0.0, hdgend = 0.0, sdpend = 0.0, altend = 0.0;
	double timbeg = 0.0, timend = 0.0;
	int    timbeg_j[5], timend_j[5];
	double timtot = 0.0, distot = 0.0, spdavg = 0.0;
	int    irec = 0, isbtmrec = 0;
	double timbegfile = 0.0, timendpath = 0.0;
	double distotfile = 0.0, timtotfile = 0.0, spdavgfile = 0.0;
	int    irecfile = 0;
	int    ntdbeams = 0, ngdbeams = 0, nzdbeams = 0, nfdbeams = 0;
	int    ntabeams = 0, ngabeams = 0, nzabeams = 0, nfabeams = 0;
	int    ntsbeams = 0, ngsbeams = 0, nzsbeams = 0, nfsbeams = 0;
	bool   beginnav = false, beginsdp = false, beginalt = false;
	bool   beginbath = false, beginamp = false, beginss = false;
	int    nread = 0;

	int    namp, nss;
	double delta, a, b, dev, mean;
	double *bathmean = NULL, *bathvar = NULL; int *nbathvar = NULL;
	double *ampmean  = NULL, *ampvar  = NULL; int *nampvar  = NULL;
	double *ssmean   = NULL, *ssvar   = NULL; int *nssvar   = NULL;
	int    nbathtot_alloc = 0, namptot_alloc = 0, nsstot_alloc = 0;
	double *bathmeantot = NULL, *bathvartot = NULL; int *nbathvartot = NULL;
	double *ampmeantot  = NULL, *ampvartot  = NULL; int *nampvartot  = NULL;
	double *ssmeantot   = NULL, *ssvartot   = NULL; int *nssvartot   = NULL;

	double mask_dx = 0.0, mask_dy = 0.0;
	int    *mask = NULL;

	char   *fileprint;
	char   string[500];
	double speed_apparent;
	double time_d_last = 0.0;
	double val_double;
	int    pass = 0;

	int imetadata = 0;
	int meta_vessel=0, meta_institution=0, meta_platform=0, meta_sonar=0;
	int meta_sonarversion=0, meta_cruiseid=0, meta_cruisename=0;
	int meta_pi=0, meta_piinstitution=0, meta_client=0;
	int meta_svcorrected=0, meta_tidecorrected=0;
	int meta_batheditmanual=0, meta_batheditauto=0;
	int meta_rollbias=0, meta_pitchbias=0, meta_headingbias=0, meta_draft=0;

	void *datalist = NULL;

	if (!quick) {
		bool done = false;
		while (!done) {
			char path[MB_PATH_MAXLINE];
			char ppath[MB_PATH_MAXLINE] = "";
			char apath[MB_PATH_MAXLINE] = "";
			char dpath[MB_PATH_MAXLINE] = "";
			int  pstatus;
			int  astatus = 0;
			if (read_datalist) {
				const int look_processed = MB_DATALIST_LOOK_UNSET;
				if (mb_datalist_open(verbose, &datalist, read_file, look_processed, &error) != MB_SUCCESS) {
					fprintf(stderr, "\nUnable to open data list file: %s\n", read_file);
					Return(MB_ERROR_OPEN_FAIL);
				}
				read_data = (mb_datalist_read3(verbose, datalist, &pstatus, path, ppath,
				                               &astatus, apath, dpath, &format,
				                               &file_weight, &error) == MB_SUCCESS);
			} else {
				strcpy(path, read_file);
				read_data = true;
				astatus = 0;
			}

			while (read_data) {
				void *mbio_ptr = NULL;
				if (mb_read_init_altnav(verbose, path, format, pings_get, lonflip, bounds,
				                        btime_i, etime_i, speedmin, timegap, astatus, apath,
				                        &mbio_ptr, &btime_d, &etime_d, &beams_bath_alloc,
				                        &beams_amp_alloc, &pixels_ss_alloc, &error) != MB_SUCCESS) {
					char *message;
					mb_error(verbose, error, &message);
					fprintf(stream, "\nmb_read_init failed: %s\nFile: %s\n", message, path);
					Return(error);
				}

				memset(data, 0, MBINFO_MAXPINGS * sizeof(struct mbi_ping));
				for (int i = 0; i < pings_read; i++) {
					datacur = &data[i];
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char),   (void **)&datacur->beamflag, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&datacur->bath, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double),  (void **)&datacur->amp, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&datacur->bathlon, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&datacur->bathlat, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),   (void **)&datacur->ss, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),   (void **)&datacur->sslon, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),   (void **)&datacur->sslat, &error);
				}
				if (pings_read > 1 && pass == 0) {
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathmean, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathvar,  &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(int),    (void **)&nbathvar, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double),  (void **)&ampmean, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double),  (void **)&ampvar,  &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(int),     (void **)&nampvar, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),   (void **)&ssmean, &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),   (void **)&ssvar,  &error);
					if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(int),      (void **)&nssvar, &error);
				}

				if (coverage_mask && (coverage_mask_bounds || pass == 1)) {
					if (pass == 1) {
						maskbounds[0] = lonmin; maskbounds[1] = lonmax;
						maskbounds[2] = latmin; maskbounds[3] = latmax;
					}
					if (mask_nx > 1 && mask_ny <= 0) {
						if ((maskbounds[1] - maskbounds[0]) > (maskbounds[3] - maskbounds[2])) {
							mask_ny = mask_nx * (maskbounds[3] - maskbounds[2]) / (maskbounds[1] - maskbounds[0]);
						} else {
							mask_ny = mask_nx;
							mask_nx = mask_ny * (maskbounds[1] - maskbounds[0]) / (maskbounds[3] - maskbounds[2]);
							if (mask_ny < 2) mask_ny = 2;
						}
					}
					if (mask_nx < 2) mask_nx = 2;
					if (mask_ny < 2) mask_ny = 2;
					mask_dx = (maskbounds[1] - maskbounds[0]) / mask_nx;
					mask_dy = (maskbounds[3] - maskbounds[2]) / mask_ny;
					status = mb_mallocd(verbose, __FILE__, __LINE__, mask_nx * mask_ny * sizeof(int), (void **)&mask, &error);
				}

				if (error != MB_ERROR_NO_ERROR) {
					char *message;
					mb_error(verbose, error, &message);
					fprintf(stream, "\nMBIO Error allocating data arrays: %s\n", message);
					Return(error);
				}

				irecfile = 0;
				distotfile = 0.0;
				timtotfile = 0.0;
				spdavgfile = 0.0;
				if (pass == 0 && pings_read > 1) {
					for (int i = 0; i < beams_bath_alloc; i++) { bathmean[i] = 0.0; bathvar[i] = 0.0; nbathvar[i] = 0; }
					for (int i = 0; i < beams_amp_alloc; i++)  { ampmean[i]  = 0.0; ampvar[i]  = 0.0; nampvar[i]  = 0; }
					for (int i = 0; i < pixels_ss_alloc; i++)  { ssmean[i]   = 0.0; ssvar[i]   = 0.0; nssvar[i]   = 0; }
				}
				if (coverage_mask && (coverage_mask_bounds || pass == 1))
					for (int i = 0; i < mask_nx * mask_ny; i++) mask[i] = 0;

				meta_vessel = meta_institution = meta_platform = meta_sonar = 0;
				meta_sonarversion = meta_cruiseid = meta_cruisename = 0;
				meta_pi = meta_piinstitution = meta_client = 0;
				meta_svcorrected = meta_tidecorrected = 0;
				meta_batheditmanual = meta_batheditauto = 0;
				meta_rollbias = meta_pitchbias = meta_headingbias = meta_draft = 0;

				if (pass == 0) {
					if (strrchr(path, '/') == NULL) fileprint = path;
					else                            fileprint = strrchr(path, '/') + 1;
					mb_format_description(verbose, &format, format_description, &error);
					switch (output_format) {
					case JSON: {
						fprintf(output, "\"file_info\": {\n");
						fprintf(output, "\"swath_data_file\": \"%s\",\n", fileprint);
						fprintf(output, "\"mbio_data_format_id\": \"%d\",\n", format);
						size_t len1 = strspn(format_description, "Formatname: ");
						size_t len2 = strcspn(&format_description[len1], "\n");
						strncpy(string, &format_description[len1], len2); string[len2] = '\0';
						fprintf(output, "\"format_name\": \"%s\",\n", string);
						len1 += len2 + 1;
						len1 += strspn(&format_description[len1], "InformalDescription: ");
						len2 = strcspn(&format_description[len1], "\n");
						strncpy(string, &format_description[len1], len2); string[len2] = '\0';
						fprintf(output, "\"informal_description\": \"%s\",\n", string);
						len1 += len2 + 1;
						len1 += strspn(&format_description[len1], "Attributes: ");
						format_description[strlen(format_description) - 1] = '\0';
						for (len2 = len1; len2 <= strlen(format_description); len2++)
							if (format_description[len2] == 10) format_description[len2] = ';';
						fprintf(output, "\"attributes\": \"%s\"\n", &format_description[len1]);
						fprintf(output, "},\n");
						break;
					}
					case XML: {
						fprintf(output, "\t<file_info>\n");
						fprintf(output, "\t\t<swath_data_file>%s</swath_data_file>\n", fileprint);
						fprintf(output, "\t\t<mbio_data_format_id>%d</mbio_data_format_id>\n", format);
						size_t len1 = strspn(format_description, "Formatname: ");
						size_t len2 = strcspn(&format_description[len1], "\n");
						strncpy(string, &format_description[len1], len2); string[len2] = '\0';
						fprintf(output, "\t\t<format_name>%s</format_name>\n", string);
						len1 += len2 + 1;
						len1 += strspn(&format_description[len1], "InformalDescription: ");
						len2 = strcspn(&format_description[len1], "\n");
						strncpy(string, &format_description[len1], len2); string[len2] = '\0';
						fprintf(output, "\t\t<informal_description>%s</informal_description>\n", string);
						len1 += len2 + 1;
						len1 += strspn(&format_description[len1], "Attributes: ");
						format_description[strlen(format_description) - 1] = '\0';
						for (len2 = len1; len2 <= strlen(format_description); len2++)
							if (format_description[len2] == 10) format_description[len2] = ' ';
						fprintf(output, "\t\t<attributes>%s</attributes>\n", &format_description[len1]);
						fprintf(output, "\t</file_info>\n");
						break;
					}
					case FREE_TEXT:
					default:
						fprintf(output, "\nSwath Data File:      %s\n", fileprint);
						fprintf(output, "MBIO Data Format ID:  %d\n", format);
						fprintf(output, "%s", format_description);
						break;
					}
				}

				while (error <= MB_ERROR_NO_ERROR) {
					nread = 0;
					error = MB_ERROR_NO_ERROR;
					while (nread < pings_read && error == MB_ERROR_NO_ERROR) {
						int kind;
						datacur = &data[nread];
						status = mb_read(verbose, mbio_ptr, &kind, &pings, time_i, &time_d, &navlon, &navlat,
						                 &speed, &heading, &distance, &altitude, &sensordepth,
						                 &beams_bath, &beams_amp, &pixels_ss, datacur->beamflag,
						                 datacur->bath, datacur->amp, datacur->bathlon, datacur->bathlat,
						                 datacur->ss, datacur->sslon, datacur->sslat, comment, &error);

						beamflag = datacur->beamflag;
						bath = datacur->bath; amp = datacur->amp;
						bathlon = datacur->bathlon; bathlat = datacur->bathlat;
						ss = datacur->ss; sslon = datacur->sslon; sslat = datacur->sslat;

						if (pass == 0 && (error == MB_ERROR_NO_ERROR || error == MB_ERROR_TIME_GAP)) {
							irec++; irecfile++; nread++;
						}

						/* comments */
						if (pass == 0 && error == MB_ERROR_COMMENT && comments) {
							if (strncmp(comment, "META", 4) != 0) {
								if (icomment == 0 && output_format == FREE_TEXT) {
									fprintf(output, "\nComments in file %s:\n", path);
									icomment++;
								}
								switch (output_format) {
								case FREE_TEXT: fprintf(output, "  %s\n", comment); break;
								case JSON:      fprintf(output, "\"comment\": \"%s\",\n", comment); break;
								case XML:       fprintf(output, "\t<comment>%s</comment>\n", comment); break;
								default: break;
								}
							}
						}

						/* metadata */
						if (pass == 0 && error == MB_ERROR_COMMENT && strncmp(comment, "META", 4) == 0) {
							if (output_format == FREE_TEXT) {
								if (imetadata == 0) { fprintf(output, "\nMetadata:\n"); imetadata++; }
								if (strncmp(comment, "METAVESSEL:", 11) == 0)        { if (meta_vessel == 0)        fprintf(output, "Vessel:                 %s\n", &comment[11]); meta_vessel++; }
								else if (strncmp(comment, "METAINSTITUTION:", 16) == 0) { if (meta_institution == 0)  fprintf(output, "Institution:            %s\n", &comment[16]); meta_institution++; }
								else if (strncmp(comment, "METAPLATFORM:", 13) == 0)    { if (meta_platform == 0)     fprintf(output, "Platform:               %s\n", &comment[13]); meta_platform++; }
								else if (strncmp(comment, "METASONARVERSION:", 17) == 0){ if (meta_sonarversion == 0) fprintf(output, "Sonar Version:          %s\n", &comment[17]); meta_sonarversion++; }
								else if (strncmp(comment, "METASONAR:", 10) == 0)       { if (meta_sonar == 0)        fprintf(output, "Sonar:                  %s\n", &comment[10]); meta_sonar++; }
								else if (strncmp(comment, "METACRUISEID:", 13) == 0)    { if (meta_cruiseid == 0)     fprintf(output, "Cruise ID:              %s\n", &comment[13]); meta_cruiseid++; }
								else if (strncmp(comment, "METACRUISENAME:", 15) == 0)  { if (meta_cruisename == 0)   fprintf(output, "Cruise Name:            %s\n", &comment[15]); meta_cruisename++; }
								else if (strncmp(comment, "METAPI:", 7) == 0)           { if (meta_pi == 0)           fprintf(output, "PI:                     %s\n", &comment[7]); meta_pi++; }
								else if (strncmp(comment, "METAPIINSTITUTION:", 18) == 0){ if (meta_piinstitution == 0) fprintf(output, "PI Institution:         %s\n", &comment[18]); meta_piinstitution++; }
								else if (strncmp(comment, "METACLIENT:", 11) == 0)      { if (meta_client == 0)       fprintf(output, "Client:                 %s\n", &comment[11]); meta_client++; }
								else if (strncmp(comment, "METASVCORRECTED:", 16) == 0) {
									if (meta_svcorrected == 0) { int v; sscanf(comment, "METASVCORRECTED:%d", &v);
										fprintf(output, "Corrected Depths:       %s\n", v ? "YES" : "NO"); }
									meta_svcorrected++;
								}
								else if (strncmp(comment, "METATIDECORRECTED:", 18) == 0) {
									if (meta_tidecorrected == 0) { int v; sscanf(comment, "METATIDECORRECTED:%d", &v);
										fprintf(output, "Tide Corrected:         %s\n", v ? "YES" : "NO"); }
									meta_tidecorrected++;
								}
								else if (strncmp(comment, "METABATHEDITMANUAL:", 19) == 0) {
									if (meta_batheditmanual == 0) { int v; sscanf(comment, "METABATHEDITMANUAL:%d", &v);
										fprintf(output, "Depths Manually Edited: %s\n", v ? "YES" : "NO"); }
									meta_batheditmanual++;
								}
								else if (strncmp(comment, "METABATHEDITAUTO:", 17) == 0) {
									if (meta_batheditauto == 0) { int v; sscanf(comment, "METABATHEDITAUTO:%d", &v);
										fprintf(output, "Depths Auto-Edited:     %s\n", v ? "YES" : "NO"); }
									meta_batheditauto++;
								}
								else if (strncmp(comment, "METAROLLBIAS:", 13) == 0)    { if (meta_rollbias == 0) { sscanf(comment, "METAROLLBIAS:%lf", &val_double); fprintf(output, "Roll Bias:              %f degrees\n", val_double); } meta_rollbias++; }
								else if (strncmp(comment, "METAPITCHBIAS:", 14) == 0)   { if (meta_pitchbias == 0){ sscanf(comment, "METAPITCHBIAS:%lf", &val_double); fprintf(output, "Pitch Bias:             %f degrees\n", val_double); } meta_pitchbias++; }
								else if (strncmp(comment, "METAHEADINGBIAS:", 16) == 0) { if (meta_headingbias == 0){ sscanf(comment, "METAHEADINGBIAS:%lf", &val_double); fprintf(output, "Heading Bias:           %f degrees\n", val_double); } meta_headingbias++; }
								else if (strncmp(comment, "METADRAFT:", 10) == 0)       { if (meta_draft == 0)    { sscanf(comment, "METADRAFT:%lf", &val_double); fprintf(output, "Draft:                  %f m\n", val_double); } meta_draft++; }
							}
							else if (output_format == JSON) {
								if      (strncmp(comment, "METAVESSEL:", 11) == 0)        emit_meta_json(output, &meta_vessel, "vessel", &comment[11]);
								else if (strncmp(comment, "METAINSTITUTION:", 16) == 0)   emit_meta_json(output, &meta_institution, "institution", &comment[16]);
								else if (strncmp(comment, "METAPLATFORM:", 13) == 0)      emit_meta_json(output, &meta_platform, "platform", &comment[13]);
								else if (strncmp(comment, "METASONARVERSION:", 17) == 0)  emit_meta_json(output, &meta_sonarversion, "sonar_version", &comment[17]);
								else if (strncmp(comment, "METASONAR:", 10) == 0)         emit_meta_json(output, &meta_sonar, "sonar", &comment[10]);
								else if (strncmp(comment, "METACRUISEID:", 13) == 0)      emit_meta_json(output, &meta_cruiseid, "cruise_id", &comment[13]);
								else if (strncmp(comment, "METACRUISENAME:", 15) == 0)    emit_meta_json(output, &meta_cruisename, "cruise_name", &comment[15]);
								else if (strncmp(comment, "METAPI:", 7) == 0)             emit_meta_json(output, &meta_pi, "pi", &comment[7]);
								else if (strncmp(comment, "METAPIINSTITUTION:", 18) == 0) emit_meta_json(output, &meta_piinstitution, "pi_institution", &comment[18]);
								else if (strncmp(comment, "METACLIENT:", 11) == 0)        emit_meta_json(output, &meta_client, "client", &comment[11]);
								else if (strncmp(comment, "METASVCORRECTED:", 16) == 0)   { if (meta_svcorrected == 0)   { int v; sscanf(comment, "METASVCORRECTED:%d", &v);   fprintf(output, "\"corrected_depths\": \"%s\",\n", v ? "YES" : "NO"); } meta_svcorrected++; }
								else if (strncmp(comment, "METATIDECORRECTED:", 18) == 0) { if (meta_tidecorrected == 0) { int v; sscanf(comment, "METATIDECORRECTED:%d", &v); fprintf(output, "\"tide_corrected\": \"%s\",\n", v ? "YES" : "NO"); } meta_tidecorrected++; }
								else if (strncmp(comment, "METABATHEDITMANUAL:", 19) == 0){ if (meta_batheditmanual == 0){ int v; sscanf(comment, "METABATHEDITMANUAL:%d", &v); fprintf(output, "\"depths_manually_edited\": \"%s\",\n", v ? "YES" : "NO"); } meta_batheditmanual++; }
								else if (strncmp(comment, "METABATHEDITAUTO:", 17) == 0)  { if (meta_batheditauto == 0)  { int v; sscanf(comment, "METABATHEDITAUTO:%d", &v);   fprintf(output, "\"depths_auto-edited\": \"%s\",\n", v ? "YES" : "NO"); } meta_batheditauto++; }
								else if (strncmp(comment, "METAROLLBIAS:", 13) == 0)      { if (meta_rollbias == 0)      { sscanf(comment, "METAROLLBIAS:%lf", &val_double);     fprintf(output, "\"roll_bias\": \"%f\",\n", val_double); } meta_rollbias++; }
								else if (strncmp(comment, "METAPITCHBIAS:", 14) == 0)     { if (meta_pitchbias == 0)     { sscanf(comment, "METAPITCHBIAS:%lf", &val_double);    fprintf(output, "\"pitch_bias\": \"%f\",\n", val_double); } meta_pitchbias++; }
								else if (strncmp(comment, "METAHEADINGBIAS:", 16) == 0)   { if (meta_headingbias == 0)   { sscanf(comment, "METAHEADINGBIAS:%lf", &val_double);  fprintf(output, "\"heading_bias\": \"%f\",\n", val_double); } meta_headingbias++; }
								else if (strncmp(comment, "METADRAFT:", 10) == 0)         { if (meta_draft == 0)         { sscanf(comment, "METADRAFT:%lf", &val_double);        fprintf(output, "\"draft\": \"%f\",\n", val_double); } meta_draft++; }
							}
							else if (output_format == XML) {
								if (imetadata == 0) { fprintf(output, "\t<metadata>\n"); imetadata++; }
								if      (strncmp(comment, "METAVESSEL:", 11) == 0)        { if (meta_vessel == 0)        fprintf(output, "\t\t<vessel>%s</vessel>\n", &comment[11]); meta_vessel++; }
								else if (strncmp(comment, "METAINSTITUTION:", 16) == 0)   { if (meta_institution == 0)   fprintf(output, "\t\t<institution>%s</institution>\n", &comment[16]); meta_institution++; }
								else if (strncmp(comment, "METAPLATFORM:", 13) == 0)      { if (meta_platform == 0)      fprintf(output, "\t\t<platform>%s</platform>\n", &comment[13]); meta_platform++; }
								else if (strncmp(comment, "METASONARVERSION:", 17) == 0)  { if (meta_sonarversion == 0)  fprintf(output, "\t\t<sonar_version>%s</sonar_version>\n", &comment[17]); meta_sonarversion++; }
								else if (strncmp(comment, "METASONAR:", 10) == 0)         { if (meta_sonar == 0)         fprintf(output, "\t\t<sonar>%s</sonar>\n", &comment[10]); meta_sonar++; }
								else if (strncmp(comment, "METACRUISEID:", 13) == 0)      { if (meta_cruiseid == 0)      fprintf(output, "\t\t<cruise_id>%s</cruise_id>\n", &comment[13]); meta_cruiseid++; }
								else if (strncmp(comment, "METACRUISENAME:", 15) == 0)    { if (meta_cruisename == 0)    fprintf(output, "\t\t<cruise_name>%s</cruise_name>\n", &comment[15]); meta_cruisename++; }
								else if (strncmp(comment, "METAPI:", 7) == 0)             { if (meta_pi == 0)            fprintf(output, "\t\t<pi>%s</pi>\n", &comment[7]); meta_pi++; }
								else if (strncmp(comment, "METAPIINSTITUTION:", 18) == 0) { if (meta_piinstitution == 0) fprintf(output, "\t\t<pi_institution>%s</pi_institution>\n", &comment[18]); meta_piinstitution++; }
								else if (strncmp(comment, "METACLIENT:", 11) == 0)        { if (meta_client == 0)        fprintf(output, "\t\t<client>%s</client>\n", &comment[11]); meta_client++; }
								else if (strncmp(comment, "METASVCORRECTED:", 16) == 0)   { if (meta_svcorrected == 0)   { int v; sscanf(comment, "METASVCORRECTED:%d", &v);   fprintf(output, "\t\t<corrected_depths>%s</corrected_depths>\n", v ? "YES" : "NO"); } meta_svcorrected++; }
								else if (strncmp(comment, "METATIDECORRECTED:", 18) == 0) { if (meta_tidecorrected == 0) { int v; sscanf(comment, "METATIDECORRECTED:%d", &v); fprintf(output, "\t\t<tide_corrected>%s</tide_corrected>\n", v ? "YES" : "NO"); } meta_tidecorrected++; }
								else if (strncmp(comment, "METABATHEDITMANUAL:", 19) == 0){ if (meta_batheditmanual == 0){ int v; sscanf(comment, "METABATHEDITMANUAL:%d", &v); fprintf(output, "\t\t<depths_manually_edited>%s</depths_manually_edited>\n", v ? "YES" : "NO"); } meta_batheditmanual++; }
								else if (strncmp(comment, "METABATHEDITAUTO:", 17) == 0)  { if (meta_batheditauto == 0)  { int v; sscanf(comment, "METABATHEDITAUTO:%d", &v);   fprintf(output, "\t\t<depths_auto_edited>%s</depths_auto_edited>\n", v ? "YES" : "NO"); } meta_batheditauto++; }
								else if (strncmp(comment, "METAROLLBIAS:", 13) == 0)      { if (meta_rollbias == 0)      { sscanf(comment, "METAROLLBIAS:%lf", &val_double);     fprintf(output, "\t\t<roll_bias>%f</roll_bias>\n", val_double); } meta_rollbias++; }
								else if (strncmp(comment, "METAPITCHBIAS:", 14) == 0)     { if (meta_pitchbias == 0)     { sscanf(comment, "METAPITCHBIAS:%lf", &val_double);    fprintf(output, "\t\t<pitch_bias>%f</pitch_bias>\n", val_double); } meta_pitchbias++; }
								else if (strncmp(comment, "METAHEADINGBIAS:", 16) == 0)   { if (meta_headingbias == 0)   { sscanf(comment, "METAHEADINGBIAS:%lf", &val_double);  fprintf(output, "\t\t<heading_bias>%f</heading_bias>\n", val_double); } meta_headingbias++; }
								else if (strncmp(comment, "METADRAFT:", 10) == 0)         { if (meta_draft == 0)         { sscanf(comment, "METADRAFT:%lf", &val_double);        fprintf(output, "\t\t<draft>%fm</draft>\n\t</metadata>\n", val_double); } meta_draft++; }
							}
						}

						if (beams_bath > beams_bath_max) beams_bath_max = beams_bath;
						if (beams_amp > beams_amp_max)   beams_amp_max  = beams_amp;
						if (pixels_ss > pixels_ss_max)   pixels_ss_max  = pixels_ss;

						if (pass == 0 && (error == MB_ERROR_NO_ERROR || error == MB_ERROR_TIME_GAP)) {
							ntdbeams += beams_bath;
							ntabeams += beams_amp;
							ntsbeams += pixels_ss;

							if (!lonflip_set && (navlon != 0.0 || navlat != 0.0)) {
								lonflip_set = true;
								if      (navlon < -270.0)                  lonflip_use = 0;
								else if (navlon >= -270.0 && navlon < -90.0) lonflip_use = -1;
								else if (navlon >= -90.0  && navlon < 90.0)  lonflip_use = 0;
								else if (navlon >= 90.0   && navlon < 270.0) lonflip_use = 1;
								else if (navlon >= 270.0)                  lonflip_use = 0;
								if (lonflip_use != lonflip) {
									struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
									mb_io_ptr->lonflip = lonflip_use;
									lonflip = lonflip_use;
									if (lonflip_use == -1) {
										if (navlon > 0.0) navlon -= 360.0;
										for (int i = 0; i < beams_bath; i++) if (bathlon[i] > 0.0) bathlon[i] -= 360.0;
										for (int i = 0; i < pixels_ss; i++) if (sslon[i] > 0.0) sslon[i] -= 360.0;
									} else if (lonflip_use == 1) {
										if (navlon < 0.0) navlon += 360.0;
										for (int i = 0; i < beams_bath; i++) if (bathlon[i] < 0.0) bathlon[i] += 360.0;
										for (int i = 0; i < pixels_ss; i++) if (sslon[i] < 0.0) sslon[i] += 360.0;
									} else {
										if (navlon < -180.0) navlon += 360.0;
										if (navlon >  180.0) navlon -= 360.0;
										for (int i = 0; i < beams_bath; i++) { if (bathlon[i] < -180.0) bathlon[i] += 360.0; if (bathlon[i] > 180.0) bathlon[i] -= 360.0; }
										for (int i = 0; i < pixels_ss; i++) { if (sslon[i] < -180.0) sslon[i] += 360.0; if (sslon[i] > 180.0) sslon[i] -= 360.0; }
									}
								}
							}

							if (irec == 1) {
								if (beams_bath > 0) {
									if (mb_beam_ok(beamflag[beams_bath / 2])) bathbeg = bath[beams_bath / 2];
									else                                       bathbeg = altitude + sensordepth;
								}
								lonbeg = navlon; latbeg = navlat;
								timbeg = time_d; timbegfile = time_d;
								for (int i = 0; i < 7; i++) timbeg_i[i] = time_i[i];
								spdbeg = speed; hdgbeg = heading; sdpbeg = sensordepth; altbeg = altitude;
							} else if (good_nav_only) {
								if (lonbeg == 0.0 && latbeg == 0.0 && navlon != 0.0 && navlat != 0.0) {
									lonbeg = navlon;
									if (beams_bath > 0) {
										if (mb_beam_ok(beamflag[beams_bath / 2])) bathbeg = bath[beams_bath / 2];
										else                                       bathbeg = altitude + sensordepth;
									}
									latbeg = navlat;
									if (spdbeg == 0.0 && speed != 0.0)         spdbeg = speed;
									if (hdgbeg == 0.0 && heading != 0.0)       hdgbeg = heading;
									if (sdpbeg == 0.0 && sensordepth != 0.0)   sdpbeg = sensordepth;
									if (altbeg == 0.0 && altitude != 0.0)      altbeg = altitude;
								}
							}

							if (beams_bath > 0) {
								if (mb_beam_ok(beamflag[beams_bath / 2])) bathend = bath[beams_bath / 2];
								else                                       bathend = altitude + sensordepth;
							}
							lonend = navlon; latend = navlat;
							spdend = speed; hdgend = heading; sdpend = sensordepth; altend = altitude;
							timend = time_d; timendpath = time_d;
							for (int i = 0; i < 7; i++) timend_i[i] = time_i[i];

							speed_apparent = 3600.0 * distance / (time_d - time_d_last);
							bool good_nav = true;
							if (good_nav_only) {
								if ((navlon > -0.005 && navlon < 0.005) && (navlat > -0.005 && navlat < 0.005)) good_nav = false;
								else if (beginnav && speed_apparent >= speed_threshold)                          good_nav = false;
							}
							if (!good_nav_only || (good_nav && speed_apparent < speed_threshold)) {
								distot += distance; distotfile += distance;
							}

							if (!beginnav && good_nav) {
								lonmin = lonmax = navlon; latmin = latmax = navlat; beginnav = true;
							}
							if (!beginsdp && sensordepth > 0.0) { sdpmin = sdpmax = sensordepth; beginsdp = true; }
							if (!beginalt && altitude > 0.0)    { altmin = altmax = altitude;   beginalt = true; }
							if (!beginbath && beams_bath > 0)
								for (int i = 0; i < beams_bath; i++)
									if (mb_beam_ok(beamflag[i])) { bathmin = bathmax = bath[i]; beginbath = true; }
							if (!beginamp && beams_amp > 0)
								for (int i = 0; i < beams_amp; i++)
									if (mb_beam_ok(beamflag[i])) { ampmin = ampmax = amp[i]; beginamp = true; }
							if (!beginss && pixels_ss > 0)
								for (int i = 0; i < pixels_ss; i++)
									if (ss[i] > MB_SIDESCAN_NULL) { ssmin = ssmax = ss[i]; beginss = true; }

							if (good_nav && beginnav) {
								if (navlon < lonmin) lonmin = navlon; if (navlon > lonmax) lonmax = navlon;
								if (navlat < latmin) latmin = navlat; if (navlat > latmax) latmax = navlat;
							}
							if (beginsdp) {
								if (sensordepth < sdpmin) sdpmin = sensordepth; if (sensordepth > sdpmax) sdpmax = sensordepth;
							}
							if (beginalt) {
								if (altitude < altmin) altmin = altitude; if (altitude > altmax) altmax = altitude;
							}
							for (int i = 0; i < beams_bath; i++) {
								if (mb_beam_ok(beamflag[i])) {
									if (good_nav && beginnav) {
										if (bathlon[i] < lonmin) lonmin = bathlon[i]; if (bathlon[i] > lonmax) lonmax = bathlon[i];
										if (bathlat[i] < latmin) latmin = bathlat[i]; if (bathlat[i] > latmax) latmax = bathlat[i];
									}
									if (bath[i] < bathmin) bathmin = bath[i]; if (bath[i] > bathmax) bathmax = bath[i];
									ngdbeams++;
								} else if (beamflag[i] == MB_FLAG_NULL) nzdbeams++;
								  else                                   nfdbeams++;
							}
							for (int i = 0; i < beams_amp; i++) {
								if (mb_beam_ok(beamflag[i])) {
									if (amp[i] < ampmin) ampmin = amp[i]; if (amp[i] > ampmax) ampmax = amp[i];
									ngabeams++;
								} else if (beamflag[i] == MB_FLAG_NULL) nzabeams++;
								  else                                   nfabeams++;
							}
							for (int i = 0; i < pixels_ss; i++) {
								if (ss[i] > MB_SIDESCAN_NULL) {
									if (good_nav && beginnav) {
										if (sslon[i] < lonmin) lonmin = sslon[i]; if (sslon[i] > lonmax) lonmax = sslon[i];
										if (sslat[i] < latmin) latmin = sslat[i]; if (sslat[i] > latmax) latmax = sslat[i];
									}
									if (ss[i] < ssmin) ssmin = ss[i]; if (ss[i] > ssmax) ssmax = ss[i];
									ngsbeams++;
								} else if (ss[i] == 0.0) nzsbeams++;
								  else                   nfsbeams++;
							}
							time_d_last = time_d;
						}

						if ((coverage_mask && (coverage_mask_bounds || pass == 1))
						    && (error == MB_ERROR_NO_ERROR || error == MB_ERROR_TIME_GAP)) {
							int ix = (int)((navlon - maskbounds[0]) / mask_dx);
							int iy = (int)((navlat - maskbounds[2]) / mask_dy);
							if (ix >= 0 && ix < mask_nx && iy >= 0 && iy < mask_ny) mask[ix + iy * mask_nx] = 1;
							for (int i = 0; i < beams_bath; i++) {
								if (mb_beam_ok(beamflag[i])) {
									ix = (int)((bathlon[i] - maskbounds[0]) / mask_dx);
									iy = (int)((bathlat[i] - maskbounds[2]) / mask_dy);
									if (ix >= 0 && ix < mask_nx && iy >= 0 && iy < mask_ny) mask[ix + iy * mask_nx] = 1;
								}
							}
							for (int i = 0; i < pixels_ss; i++) {
								if (ss[i] > MB_SIDESCAN_NULL) {
									ix = (int)((sslon[i] - maskbounds[0]) / mask_dx);
									iy = (int)((sslat[i] - maskbounds[2]) / mask_dy);
									if (ix >= 0 && ix < mask_nx && iy >= 0 && iy < mask_ny) mask[ix + iy * mask_nx] = 1;
								}
							}
						}

						if (pass == 0 && (error == MB_ERROR_NO_ERROR || error == MB_ERROR_TIME_GAP)) {
							if (navlon == 0.0 || navlat == 0.0) mb_notice_log_problem(verbose, mbio_ptr, MB_PROBLEM_ZERO_NAV);
							else if (beginnav && speed_apparent >= speed_threshold) mb_notice_log_problem(verbose, mbio_ptr, MB_PROBLEM_TOO_FAST);
							for (int i = 0; i < beams_bath; i++)
								if (mb_beam_ok(beamflag[i]) && bath[i] > 11000.0)
									mb_notice_log_problem(verbose, mbio_ptr, MB_PROBLEM_TOO_DEEP);
						}
					}

					if (verbose >= 2) {
						fprintf(stream, "\ndbg2  Reading loop finished in program <%s>\n", THIS_MODULE_NAME);
						fprintf(stream, "dbg2       status:     %d\n", status);
						fprintf(stream, "dbg2       error:      %d\n", error);
						fprintf(stream, "dbg2       nread:      %d\n", nread);
						fprintf(stream, "dbg2       pings_read: %d\n", pings_read);
					}

					/* variance */
					if (pass == 0 && pings_read > 2 && nread == pings_read &&
					    (error == MB_ERROR_NO_ERROR || error == MB_ERROR_TIME_GAP)) {
						for (int i = 0; i < beams_bath; i++) {
							int nbath = 0;
							double sumx = 0.0, sumxx = 0.0, sumy = 0.0, sumxy = 0.0;
							for (int j = 0; j < nread; j++) {
								datacur = &data[j]; bath = datacur->bath; beamflag = datacur->beamflag;
								if (mb_beam_ok(beamflag[i])) {
									nbath++;
									sumx += j; sumxx += j*j;
									sumy += bath[i]; sumxy += j * bath[i];
								}
							}
							if (nbath == pings_read) {
								double variance = 0.0;
								delta = nbath * sumxx - sumx * sumx;
								a = (sumxx * sumy - sumx * sumxy) / delta;
								b = (nbath * sumxy - sumx * sumy) / delta;
								for (int j = 0; j < nread; j++) {
									datacur = &data[j]; bath = datacur->bath; beamflag = datacur->beamflag;
									if (mb_beam_ok(beamflag[i])) { dev = bath[i] - a - b*j; variance += dev*dev; }
								}
								bathmean[i] += sumy; bathvar[i] += variance; nbathvar[i] += nbath;
							}
						}
						for (int i = 0; i < beams_amp; i++) {
							namp = 0; mean = 0.0;
							for (int j = 0; j < nread; j++) {
								datacur = &data[j]; amp = datacur->amp; beamflag = datacur->beamflag;
								if (mb_beam_ok(beamflag[i])) { namp++; mean += amp[i]; }
							}
							if (namp == pings_read) {
								double variance = 0.0; mean /= namp;
								for (int j = 0; j < nread; j++) {
									datacur = &data[j]; amp = datacur->amp;
									if (mb_beam_ok(beamflag[i])) { dev = amp[i] - mean; variance += dev*dev; }
								}
								ampmean[i] += namp * mean; ampvar[i] += variance; nampvar[i] += namp;
							}
						}
						for (int i = 0; i < pixels_ss; i++) {
							nss = 0; mean = 0.0;
							for (int j = 0; j < nread; j++) {
								datacur = &data[j]; ss = datacur->ss;
								if (ss[i] > MB_SIDESCAN_NULL) { nss++; mean += ss[i]; }
							}
							if (nss == pings_read) {
								double variance = 0.0; mean /= nss;
								for (int j = 0; j < nread; j++) {
									datacur = &data[j]; ss = datacur->ss;
									if (ss[i] > MB_SIDESCAN_NULL) { dev = ss[i] - mean; variance += dev*dev; }
								}
								ssmean[i] += nss * mean; ssvar[i] += variance; nssvar[i] += nss;
							}
						}
					}

					if (verbose >= 2) {
						fprintf(stream, "\ndbg2  Processing loop finished in program <%s>\n", THIS_MODULE_NAME);
						fprintf(stream, "dbg2       status:     %d\n", status);
						fprintf(stream, "dbg2       error:      %d\n", error);
						fprintf(stream, "dbg2       nread:      %d\n", nread);
						fprintf(stream, "dbg2       pings_read: %d\n", pings_read);
					}
				}

				timtotfile = (timendpath - timbegfile) / 3600.0;
				if (timtotfile > 0.0) { timtot += timtotfile; spdavgfile = distotfile / timtotfile; }
				if (irecfile <= 0)        mb_notice_log_problem(verbose, mbio_ptr, MB_PROBLEM_NO_DATA);
				else if (timtotfile > 0.0 && spdavgfile >= speed_threshold)
					mb_notice_log_problem(verbose, mbio_ptr, MB_PROBLEM_AVG_TOO_FAST);

				if (print_notices && pass == 0) {
					status = mb_notice_get_list(verbose, mbio_ptr, notice_list);
					for (int i = 0; i < MB_NOTICE_MAX; i++) notice_list_tot[i] += notice_list[i];
				}

				if (pings_read > 2) {
					if (nbathtot_alloc < beams_bath_max) {
						mb_reallocd(verbose, __FILE__, __LINE__, beams_bath_max * sizeof(double), (void **)&bathmeantot, &error);
						mb_reallocd(verbose, __FILE__, __LINE__, beams_bath_max * sizeof(double), (void **)&bathvartot,  &error);
						mb_reallocd(verbose, __FILE__, __LINE__, beams_bath_max * sizeof(int),    (void **)&nbathvartot, &error);
						for (int i = nbathtot_alloc; i < beams_bath_max; i++) { bathmeantot[i] = 0.0; bathvartot[i] = 0.0; nbathvartot[i] = 0; }
						nbathtot_alloc = beams_bath_max;
					}
					if (namptot_alloc < beams_amp_max) {
						mb_reallocd(verbose, __FILE__, __LINE__, beams_amp_max * sizeof(double), (void **)&ampmeantot, &error);
						mb_reallocd(verbose, __FILE__, __LINE__, beams_amp_max * sizeof(double), (void **)&ampvartot,  &error);
						mb_reallocd(verbose, __FILE__, __LINE__, beams_amp_max * sizeof(int),    (void **)&nampvartot, &error);
						for (int i = namptot_alloc; i < beams_amp_max; i++) { ampmeantot[i] = 0.0; ampvartot[i] = 0.0; nampvartot[i] = 0; }
						namptot_alloc = beams_amp_max;
					}
					if (nsstot_alloc < pixels_ss_max) {
						mb_reallocd(verbose, __FILE__, __LINE__, pixels_ss_max * sizeof(double), (void **)&ssmeantot, &error);
						mb_reallocd(verbose, __FILE__, __LINE__, pixels_ss_max * sizeof(double), (void **)&ssvartot,  &error);
						mb_reallocd(verbose, __FILE__, __LINE__, pixels_ss_max * sizeof(int),    (void **)&nssvartot, &error);
						for (int i = nsstot_alloc; i < pixels_ss_max; i++) { ssmeantot[i] = 0.0; ssvartot[i] = 0.0; nssvartot[i] = 0; }
						nsstot_alloc = pixels_ss_max;
					}
					for (int i = 0; i < beams_bath; i++) { bathmeantot[i] += bathmean[i]; bathvartot[i] += bathvar[i]; nbathvartot[i] += nbathvar[i]; }
					for (int i = 0; i < beams_amp; i++)  { ampmeantot[i]  += ampmean[i];  ampvartot[i]  += ampvar[i];  nampvartot[i]  += nampvar[i];  }
					for (int i = 0; i < pixels_ss; i++)  { ssmeantot[i]   += ssmean[i];   ssvartot[i]   += ssvar[i];   nssvartot[i]   += nssvar[i];   }
				}

				status &= mb_close(verbose, &mbio_ptr, &error);

				if (read_datalist)
					read_data = (mb_datalist_read(verbose, datalist, path, dpath, &format, &file_weight, &error) == MB_SUCCESS);
				else
					read_data = false;
			}
			if (read_datalist) mb_datalist_close(verbose, &datalist, &error);

			if (pass > 0 || !coverage_mask || (coverage_mask && coverage_mask_bounds)) done = true;
			pass++;
		}
	}
	else {
		struct mb_info_struct mb_info;
		status = mb_get_info_datalist(verbose, read_file, &format, &mb_info, lonflip_use, &error);
		if (status == MB_SUCCESS) {
			irec = mb_info.nrecords;
			notice_list_tot[MB_DATA_SIDESCAN2]            = mb_info.nrecords_ss1;
			notice_list_tot[MB_DATA_SIDESCAN3]            = mb_info.nrecords_ss2;
			notice_list_tot[MB_DATA_SUBBOTTOM_SUBBOTTOM]  = mb_info.nrecords_sbp;
			notice_list_tot[MB_DATA_SUBBOTTOM_MCS]        = 0;
			notice_list_tot[MB_DATA_SUBBOTTOM_CNTRBEAM]   = 0;
			notice_list_tot[MB_DATA_WATER_COLUMN]         = 0;
			beams_bath_max = mb_info.nbeams_bath;
			ntdbeams = mb_info.nbeams_bath_total;
			ngdbeams = mb_info.nbeams_bath_good;
			nzdbeams = mb_info.nbeams_bath_zero;
			nfdbeams = mb_info.nbeams_bath_flagged;
			beams_amp_max = mb_info.nbeams_amp;
			ntabeams = mb_info.nbeams_amp_total;
			ngabeams = mb_info.nbeams_amp_good;
			nzabeams = mb_info.nbeams_amp_zero;
			nfabeams = mb_info.nbeams_amp_flagged;
			pixels_ss_max = mb_info.npixels_ss;
			ntsbeams = mb_info.npixels_ss_total;
			ngsbeams = mb_info.npixels_ss_good;
			nzsbeams = mb_info.npixels_ss_zero;
			nfsbeams = mb_info.npixels_ss_flagged;
			timtot = mb_info.time_total; distot = mb_info.dist_total; spdavg = mb_info.speed_avg;
			timbeg = mb_info.time_start; mb_get_date(verbose, timbeg, timbeg_i);
			lonbeg = mb_info.lon_start; latbeg = mb_info.lat_start; bathbeg = mb_info.depth_start;
			hdgbeg = mb_info.heading_start; spdbeg = mb_info.speed_start;
			sdpbeg = mb_info.sensordepth_start; altbeg = mb_info.sonaraltitude_start;
			timend = mb_info.time_end; mb_get_date(verbose, timend, timend_i);
			lonend = mb_info.lon_end; latend = mb_info.lat_end; bathend = mb_info.depth_end;
			hdgend = mb_info.heading_end; spdend = mb_info.speed_end;
			sdpend = mb_info.sensordepth_end; altend = mb_info.sonaraltitude_end;
			lonmin = mb_info.lon_min; lonmax = mb_info.lon_max;
			latmin = mb_info.lat_min; latmax = mb_info.lat_max;
			sdpmin = mb_info.sensordepth_min; sdpmax = mb_info.sensordepth_max;
			altmin = mb_info.altitude_min; altmax = mb_info.altitude_max;
			bathmin = mb_info.depth_min; bathmax = mb_info.depth_max;
			ampmin = mb_info.amp_min; ampmax = mb_info.amp_max;
			ssmin = mb_info.ss_min; ssmax = mb_info.ss_max;
			notice_list_tot[MB_PROBLEM_NO_DATA      + MB_DATA_KINDS - (MB_ERROR_MIN)] = mb_info.problem_nodata;
			notice_list_tot[MB_PROBLEM_ZERO_NAV     + MB_DATA_KINDS - (MB_ERROR_MIN)] = mb_info.problem_zeronav;
			notice_list_tot[MB_PROBLEM_TOO_FAST     + MB_DATA_KINDS - (MB_ERROR_MIN)] = mb_info.problem_toofast;
			notice_list_tot[MB_PROBLEM_AVG_TOO_FAST + MB_DATA_KINDS - (MB_ERROR_MIN)] = mb_info.problem_avgtoofast;
			notice_list_tot[MB_PROBLEM_TOO_DEEP     + MB_DATA_KINDS - (MB_ERROR_MIN)] = mb_info.problem_toodeep;
			notice_list_tot[MB_PROBLEM_BAD_DATAGRAM + MB_DATA_KINDS - (MB_ERROR_MIN)] = mb_info.problem_baddatagram;
		}
	}

	if (pings_read > 2) {
		for (int i = 0; i < nbathtot_alloc; i++)
			if (nbathvartot[i] > 0) { bathmeantot[i] /= nbathvartot[i]; bathvartot[i] /= nbathvartot[i]; }
		for (int i = 0; i < namptot_alloc; i++)
			if (nampvartot[i] > 0) { ampmeantot[i] /= nampvartot[i]; ampvartot[i] /= nampvartot[i]; }
		for (int i = 0; i < nsstot_alloc; i++)
			if (nssvartot[i] > 0) { ssmeantot[i] /= nssvartot[i]; ssvartot[i] /= nssvartot[i]; }
	}

	double ngd_percent = ntdbeams > 0 ? 100.0 * ngdbeams / ntdbeams : 0.0;
	double nzd_percent = ntdbeams > 0 ? 100.0 * nzdbeams / ntdbeams : 0.0;
	double nfd_percent = ntdbeams > 0 ? 100.0 * nfdbeams / ntdbeams : 0.0;
	double nga_percent = ntabeams > 0 ? 100.0 * ngabeams / ntabeams : 0.0;
	double nza_percent = ntabeams > 0 ? 100.0 * nzabeams / ntabeams : 0.0;
	double nfa_percent = ntabeams > 0 ? 100.0 * nfabeams / ntabeams : 0.0;
	double ngs_percent = ntsbeams > 0 ? 100.0 * ngsbeams / ntsbeams : 0.0;
	double nzs_percent = ntsbeams > 0 ? 100.0 * nzsbeams / ntsbeams : 0.0;
	double nfs_percent = ntsbeams > 0 ? 100.0 * nfsbeams / ntsbeams : 0.0;

	if (!quick) timtot = (timend - timbeg) / 3600.0;
	if (timtot > 0.0) spdavg = distot / timtot;
	mb_get_jtime(verbose, timbeg_i, timbeg_j);
	mb_get_jtime(verbose, timend_i, timend_j);

	switch (output_format) {
	case FREE_TEXT:
		fprintf(output, "\nData Totals:\nNumber of Records:                    %8d\n", irec);
		isbtmrec = notice_list_tot[MB_DATA_SUBBOTTOM_MCS] + notice_list_tot[MB_DATA_SUBBOTTOM_CNTRBEAM] +
		           notice_list_tot[MB_DATA_SUBBOTTOM_SUBBOTTOM];
		if (isbtmrec > 0) fprintf(output, "Number of Subbottom Records:          %8d\n", isbtmrec);
		if (notice_list_tot[MB_DATA_SIDESCAN2] > 0) fprintf(output, "Number of Secondary Sidescan Records: %8d\n", notice_list_tot[MB_DATA_SIDESCAN2]);
		if (notice_list_tot[MB_DATA_SIDESCAN3] > 0) fprintf(output, "Number of Tertiary Sidescan Records:  %8d\n", notice_list_tot[MB_DATA_SIDESCAN3]);
		if (notice_list_tot[MB_DATA_WATER_COLUMN] > 0) fprintf(output, "Number of Water Column Records:       %8d\n", notice_list_tot[MB_DATA_WATER_COLUMN]);
		fprintf(output, "Bathymetry Data (%d beams):\n", beams_bath_max);
		fprintf(output, "  Number of Beams:         %8d\n", ntdbeams);
		fprintf(output, "  Number of Good Beams:    %8d     %5.2f%%\n", ngdbeams, ngd_percent);
		fprintf(output, "  Number of Zero Beams:    %8d     %5.2f%%\n", nzdbeams, nzd_percent);
		fprintf(output, "  Number of Flagged Beams: %8d     %5.2f%%\n", nfdbeams, nfd_percent);
		fprintf(output, "Amplitude Data (%d beams):\n", beams_amp_max);
		fprintf(output, "  Number of Beams:         %8d\n", ntabeams);
		fprintf(output, "  Number of Good Beams:    %8d     %5.2f%%\n", ngabeams, nga_percent);
		fprintf(output, "  Number of Zero Beams:    %8d     %5.2f%%\n", nzabeams, nza_percent);
		fprintf(output, "  Number of Flagged Beams: %8d     %5.2f%%\n", nfabeams, nfa_percent);
		fprintf(output, "Sidescan Data (%d pixels):\n", pixels_ss_max);
		fprintf(output, "  Number of Pixels:        %8d\n", ntsbeams);
		fprintf(output, "  Number of Good Pixels:   %8d     %5.2f%%\n", ngsbeams, ngs_percent);
		fprintf(output, "  Number of Zero Pixels:   %8d     %5.2f%%\n", nzsbeams, nzs_percent);
		fprintf(output, "  Number of Flagged Pixels:%8d     %5.2f%%\n", nfsbeams, nfs_percent);
		fprintf(output, "\nNavigation Totals:\nTotal Time:         %10.4f hours\n", timtot);
		fprintf(output, "Total Track Length: %10.4f km\n", distot);
		fprintf(output, "Average Speed:      %10.4f km/hr (%7.4f knots)\n", spdavg, spdavg / 1.85);
		fprintf(output, "\nStart of Data:\nTime:  %2.2d %2.2d %4.4d %2.2d:%2.2d:%2.2d.%6.6d  JD%d (%4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d.%6.6d)\n",
		        timbeg_i[1], timbeg_i[2], timbeg_i[0], timbeg_i[3], timbeg_i[4], timbeg_i[5], timbeg_i[6], timbeg_j[1],
		        timbeg_i[0], timbeg_i[1], timbeg_i[2], timbeg_i[3], timbeg_i[4], timbeg_i[5], timbeg_i[6]);
		if (bathy_in_meters) fprintf(output, "Lon: %15.9f     Lat: %15.9f     Depth: %10.4f meters\n", lonbeg, latbeg, bathbeg);
		else                 fprintf(output, "Lon: %15.9f     Lat: %15.9f     Depth: %10.4f feet\n",   lonbeg, latbeg, bathy_scale * bathbeg);
		fprintf(output, "Speed: %7.4f km/hr (%7.4f knots)  Heading:%9.4f degrees\n", spdbeg, spdbeg/1.85, hdgbeg);
		fprintf(output, "Sonar Depth:%10.4f m  Sonar Altitude:%10.4f m\n", sdpbeg, altbeg);
		fprintf(output, "\nEnd of Data:\nTime:  %2.2d %2.2d %4.4d %2.2d:%2.2d:%2.2d.%6.6d  JD%d (%4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d.%6.6d)\n",
		        timend_i[1], timend_i[2], timend_i[0], timend_i[3], timend_i[4], timend_i[5], timend_i[6], timend_j[1],
		        timend_i[0], timend_i[1], timend_i[2], timend_i[3], timend_i[4], timend_i[5], timend_i[6]);
		if (bathy_in_meters) fprintf(output, "Lon: %15.9f     Lat: %15.9f     Depth: %10.4f meters\n", lonend, latend, bathend);
		else                 fprintf(output, "Lon: %15.9f     Lat: %15.9f     Depth: %10.4f feet\n",   lonend, latend, bathy_scale * bathend);
		fprintf(output, "Speed: %7.4f km/hr (%7.4f knots)  Heading:%9.4f degrees\n", spdend, spdend/1.85, hdgend);
		fprintf(output, "Sonar Depth:%10.4f m  Sonar Altitude:%10.4f m\n", sdpend, altend);
		fprintf(output, "\nLimits:\nMinimum Longitude:   %15.9f   Maximum Longitude:   %15.9f\n", lonmin, lonmax);
		fprintf(output, "Minimum Latitude:    %15.9f   Maximum Latitude:    %15.9f\n", latmin, latmax);
		fprintf(output, "Minimum Sonar Depth: %10.4f   Maximum Sonar Depth: %10.4f\n", sdpmin, sdpmax);
		fprintf(output, "Minimum Altitude:    %10.4f   Maximum Altitude:    %10.4f\n", altmin, altmax);
		if (ngdbeams > 0 || verbose >= 1) fprintf(output, "Minimum Depth:       %10.4f   Maximum Depth:       %10.4f\n", bathy_scale * bathmin, bathy_scale * bathmax);
		if (ngabeams > 0 || verbose >= 1) fprintf(output, "Minimum Amplitude:   %10.4f   Maximum Amplitude:   %10.4f\n", ampmin, ampmax);
		if (ngsbeams > 0 || verbose >= 1) fprintf(output, "Minimum Sidescan:    %10.4f   Maximum Sidescan:    %10.4f\n", ssmin, ssmax);
		break;
	case JSON:
		fprintf(output, "\"data_totals\": {\n\"number_of_records\": \"%d\"", irec);
		isbtmrec = notice_list_tot[MB_DATA_SUBBOTTOM_MCS] + notice_list_tot[MB_DATA_SUBBOTTOM_CNTRBEAM] +
		           notice_list_tot[MB_DATA_SUBBOTTOM_SUBBOTTOM];
		if (isbtmrec > 0) fprintf(output, ",\n\"number_of_subbottom_records\":\"%d\"\n", isbtmrec);
		if (notice_list_tot[MB_DATA_SIDESCAN2] > 0) fprintf(output, ",\n\"number_of_secondary_sidescan_records\": \"%d\"", notice_list_tot[MB_DATA_SIDESCAN2]);
		if (notice_list_tot[MB_DATA_SIDESCAN3] > 0) fprintf(output, ",\n\"number_of_tertiary_sidescan_records\": \"%d\"", notice_list_tot[MB_DATA_SIDESCAN3]);
		if (notice_list_tot[MB_DATA_WATER_COLUMN] > 0) fprintf(output, ",\n\"number_of_water_column_records\": \"%d\"", notice_list_tot[MB_DATA_WATER_COLUMN]);
		fprintf(output, "\n},\n");
		fprintf(output, "\"bathymetry_data\": {\n\"max_beams_per_ping\": \"%d\",\n\"number_beams\": \"%d\",\n", beams_bath_max, ntdbeams);
		fprintf(output, "\"number_good_beams\": \"%d\",\n\"percent_good_beams\": \"%5.2f\",\n", ngdbeams, ngd_percent);
		fprintf(output, "\"number_zero_beams\": \"%d\",\n\"percent_zero_beams\": \"%5.2f\",\n", nzdbeams, nzd_percent);
		fprintf(output, "\"number_flagged_beams\": \"%d\",\n\"percent_flagged_beams\": \"%5.2f\"\n},\n", nfdbeams, nfd_percent);
		fprintf(output, "\"amplitude_data\": {\n\"max_beams_per_ping\": \"%d\",\n\"number_beams\": \"%d\",\n", beams_amp_max, ntabeams);
		fprintf(output, "\"number_good_beams\": \"%d\",\n\"percent_good_beams\": \" %5.2f\",\n", ngabeams, nga_percent);
		fprintf(output, "\"number_zero_beams\": \"%d\",\n\"percent_zero_beams\": \"%5.2f\",\n", nzabeams, nza_percent);
		fprintf(output, "\"number_flagged_beams\": \"%d\",\n\"percent_flagged_beams\": \"%5.2f\"\n},\n", nfabeams, nfa_percent);
		fprintf(output, "\"sidescan_data\": {\n\"max_pixels_per_ping\": \"%d\",\n\"number_of_pixels\": \"%d\",\n", pixels_ss_max, ntsbeams);
		fprintf(output, "\"number_good_pixels\": \"%d\",\n\"percent_good_pixels\": \"%5.2f\",\n", ngsbeams, ngs_percent);
		fprintf(output, "\"number_zero_pixels\": \"%d\",\n\"percent_zero_pixels\": \"%5.2f\",\n", nzsbeams, nzs_percent);
		fprintf(output, "\"number_flagged_pixels\": \"%d\",\n\"percent_flagged_pixels\": \"%5.2f\"\n},\n", nfsbeams, nfs_percent);
		fprintf(output, "\"navigation_totals\": {\n\"total_time_hours\": \"%.4f\",\n\"total_track_length_km\": \"%.4f\",\n", timtot, distot);
		fprintf(output, "\"average_speed_km_per_hr\": \"%.4f\",\n\"average_speed_knots\": \"%.4f\"\n},\n", spdavg, spdavg / 1.85);
		fprintf(output, "\"start_of_data\": {\n\"time\": \"%2.2d %2.2d %4.4d %2.2d:%2.2d:%2.2d.%6.6d  JD%d\",\n", timbeg_i[1], timbeg_i[2], timbeg_i[0], timbeg_i[3], timbeg_i[4], timbeg_i[5], timbeg_i[6], timbeg_j[1]);
		fprintf(output, "\"time_iso\": \"%4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d.%6.6d\",\n", timbeg_i[0], timbeg_i[1], timbeg_i[2], timbeg_i[3], timbeg_i[4], timbeg_i[5], timbeg_i[6]);
		if (bathy_in_meters) fprintf(output, "\"longitude\": \"%.9f\",\n\"latitude\": \"%.9f\",\n\"depth_meters\": \"%.4f\",\n", lonbeg, latbeg, bathbeg);
		else                 fprintf(output, "\"longitude\": \"%.9f\",\n\"latitude\": \"%.9f\",\n\"depth_feet\": \"%.4f\",\n",   lonbeg, latbeg, bathy_scale * bathbeg);
		fprintf(output, "\"speed_km_per_hour\": \"%.4f\",\n\"speed_knots\": \"%.4f\",\n\"heading_degrees\": \"%.4f\",\n", spdbeg, spdbeg/1.85, hdgbeg);
		fprintf(output, "\"sonar_depth_meters\": \"%.4f\",\n\"sonar_altitude_meters\": \"%.4f\"\n},\n", sdpbeg, altbeg);
		fprintf(output, "\"end_of_data\": {\n\"time\": \"%2.2d %2.2d %4.4d %2.2d:%2.2d:%2.2d.%6.6d  JD%d\",\n", timend_i[1], timend_i[2], timend_i[0], timend_i[3], timend_i[4], timend_i[5], timend_i[6], timend_j[1]);
		fprintf(output, "\"time_iso\": \"%4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d.%6.6d\",\n", timend_i[0], timend_i[1], timend_i[2], timend_i[3], timend_i[4], timend_i[5], timend_i[6]);
		if (bathy_in_meters) fprintf(output, "\"longitude\": \"%.9f\",\n\"latitude\": \"%.9f\",\n\"depth_meters\": \"%.4f\",\n", lonend, latend, bathend);
		else                 fprintf(output, "\"longitude\": \"%.9f\",\n\"latitude\": \"%.9f\",\n\"depth_feet\": \"%.4f\",\n",   lonend, latend, bathy_scale * bathend);
		fprintf(output, "\"speed_km_per_hour\": \"%.4f\",\n\"speed_knots\": \"%.4f\",\n\"heading_degrees\": \"%.4f\",\n", spdend, spdend/1.85, hdgend);
		fprintf(output, "\"sonar_depth_meters\": \"%.4f\",\n\"sonar_altitude_meters\": \"%.4f\"\n},\n", sdpend, altend);
		fprintf(output, "\"limits\": {\n\"minimum_longitude\": \"%.9f\",\n\"maximum_longitude\": \"%.9f\",\n", lonmin, lonmax);
		fprintf(output, "\"minimum_latitude\": \"%.9f\",\n\"maximum_latitude\": \"%.9f\",\n", latmin, latmax);
		fprintf(output, "\"minimum_sonar_depth\": \"%.4f\",\n\"maximum_sonar_depth\": \"%.4f\",\n", sdpmin, sdpmax);
		fprintf(output, "\"minimum_altitude\": \"%.4f\",\n\"maximum_altitude\": \"%.4f\"", altmin, altmax);
		if (ngdbeams > 0 || verbose >= 1) fprintf(output, ",\n\"minimum_depth\": \"%.4f\",\n\"maximum_depth\": \"%.4f\"", bathy_scale * bathmin, bathy_scale * bathmax);
		if (ngabeams > 0 || verbose >= 1) fprintf(output, ",\n\"minimum_amplitude\": \"%.4f\",\n\"maximum_amplitude\": \"%.4f\"", ampmin, ampmax);
		if (ngsbeams > 0 || verbose >= 1) fprintf(output, ",\n\"minimum_sidescan\": \"%.4f\",\n\"maximum_sidescan\": \"%.4f\"", ssmin, ssmax);
		fprintf(output, "\n}");
		break;
	case XML:
		fprintf(output, "\t<data_totals>\n\t\t<number_of_records>%d</number_of_records>\n", irec);
		isbtmrec = notice_list_tot[MB_DATA_SUBBOTTOM_MCS] + notice_list_tot[MB_DATA_SUBBOTTOM_CNTRBEAM] +
		           notice_list_tot[MB_DATA_SUBBOTTOM_SUBBOTTOM];
		if (isbtmrec > 0) fprintf(output, "\t\t<number_of_subbottom_records>%d</number_of_subbottom_records>\n", isbtmrec);
		if (notice_list_tot[MB_DATA_SIDESCAN2] > 0) fprintf(output, "\t\t<number_of_secondary_sidescan_records>%d</number_of_secondary_sidescan_records>\n", notice_list_tot[MB_DATA_SIDESCAN2]);
		if (notice_list_tot[MB_DATA_SIDESCAN3] > 0) fprintf(output, "\t\t<number_of_tertiary_sidescan_records>%d</number_of_tertiary_sidescan_records>\n", notice_list_tot[MB_DATA_SIDESCAN3]);
		if (notice_list_tot[MB_DATA_WATER_COLUMN] > 0) fprintf(output, "\t\t<number_of_water_column_records>%d</number_of_water_column_records>\n", notice_list_tot[MB_DATA_WATER_COLUMN]);
		fprintf(output, "\t</data_totals>\n");
		fprintf(output, "\t<bathymetry_data>\n\t\t<max_beams_per_ping>%d</max_beams_per_ping>\n\t\t<number_beams>%d</number_beams>\n", beams_bath_max, ntdbeams);
		fprintf(output, "\t\t<number_good_beams>%d</number_good_beams>\n\t\t<percent_good_beams>%.2f</percent_good_beams>\n", ngdbeams, ngd_percent);
		fprintf(output, "\t\t<number_zero_beams>%d</number_zero_beams>\n\t\t<percent_zero_beams>%.2f</percent_zero_beams>\n", nzdbeams, nzd_percent);
		fprintf(output, "\t\t<number_flagged_beams>%d</number_flagged_beams>\n\t\t<percent_flagged_beams>%.2f</percent_flagged_beams>\n\t</bathymetry_data>\n", nfdbeams, nfd_percent);
		fprintf(output, "\t<amplitude_data>\n\t\t<max_beams_per_ping>%d</max_beams_per_ping>\n\t\t<number_beams>%d</number_beams>\n", beams_bath_max, ntabeams);
		fprintf(output, "\t\t<number_good_beams>%d</number_good_beams>\n\t\t<percent_good_beams>%.2f</percent_good_beams>\n", ngabeams, nga_percent);
		fprintf(output, "\t\t<number_zero_beams>%d</number_zero_beams>\n\t\t<percent_zero_beams>%.2f</percent_zero_beams>\n", nzabeams, nza_percent);
		fprintf(output, "\t\t<number_flagged_beams>%d</number_flagged_beams>\n\t\t<percent_flagged_beams>%.2f</percent_flagged_beams>\n\t</amplitude_data>\n", nfabeams, nfa_percent);
		fprintf(output, "\t<sidescan_data>\n\t\t<max_pixels_per_ping>%d</max_pixels_per_ping>\n\t\t<number_pixels>%d</number_pixels>\n", pixels_ss_max, ntsbeams);
		fprintf(output, "\t\t<number_good_pixels>%d</number_good_pixels>\n\t\t<percent_good_pixels>%.2f</percent_good_pixels>\n", ngsbeams, ngs_percent);
		fprintf(output, "\t\t<number_zero_pixels>%d</number_zero_pixels>\n\t\t<percent_zero_pixels>%.2f</percent_zero_pixels>\n", nzsbeams, nzs_percent);
		fprintf(output, "\t\t<number_flagged_pixels>%d</number_flagged_pixels>\n\t\t<percent_flagged_pixels>%.2f</percent_flagged_pixels>\n\t</sidescan_data>\n", nfsbeams, nfs_percent);
		fprintf(output, "\t<tnavigation_totals>\n\t\t<total_time_hours>%.4f</total_time_hours>\n", timtot);
		fprintf(output, "\t\t<total_track_length_km>%.4f</total_track_length_km>\n", distot);
		fprintf(output, "\t\t<average_speed_km_per_hr>%.4f</average_speed_km_per_hr>\n", spdavg);
		fprintf(output, "\t\t<average_speed_knots>%.4f</average_speed_knots>\n\t</tnavigation_totals>\n", spdavg / 1.85);
		fprintf(output, "\t<start_of_data>\n\t\t<time>%2.2d %2.2d %4.4d %2.2d:%2.2d:%2.2d.%6.6d  JD%d</time>\n", timbeg_i[1], timbeg_i[2], timbeg_i[0], timbeg_i[3], timbeg_i[4], timbeg_i[5], timbeg_i[6], timbeg_j[1]);
		fprintf(output, "\t\t<time_iso>%4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d.%6.6d</time_iso>\n", timbeg_i[0], timbeg_i[1], timbeg_i[2], timbeg_i[3], timbeg_i[4], timbeg_i[5], timbeg_i[6]);
		fprintf(output, "\t\t<longitude>%.9f</longitude>\n\t\t<latitude>%.9f</latitude>\n", lonbeg, latbeg);
		fprintf(output, "\t\t<depth_meters>%.4f</depth_meters>\n", bathy_in_meters ? bathbeg : bathy_scale * bathbeg);
		fprintf(output, "\t\t<speed_km_per_hour>%.4f</speed_km_per_hour>\n\t\t<speed_knots>%.4f</speed_knots>\n", spdbeg, spdbeg/1.85);
		fprintf(output, "\t\t<heading_degrees>%.4f</heading_degrees>\n", hdgbeg);
		fprintf(output, "\t\t<sonar_depth_meters>%.4f</sonar_depth_meters>\n\t\t<sonar_altitude_meters>%.4f</sonar_altitude_meters>\n\t</start_of_data>\n", sdpbeg, altbeg);
		fprintf(output, "\t<end_of_data>\n\t\t<time>%2.2d %2.2d %4.4d %2.2d:%2.2d:%2.2d.%6.6d  JD%d</time>\n", timend_i[1], timend_i[2], timend_i[0], timend_i[3], timend_i[4], timend_i[5], timend_i[6], timend_j[1]);
		fprintf(output, "\t\t<time_iso>%4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d.%6.6d</time_iso>\n", timend_i[0], timend_i[1], timend_i[2], timend_i[3], timend_i[4], timend_i[5], timend_i[6]);
		fprintf(output, "\t\t<longitude>%.9f</longitude>\n\t\t<latitude>%.9f</latitude>\n", lonend, latend);
		fprintf(output, "\t\t<depth_meters>%.4f</depth_meters>\n", bathy_in_meters ? bathend : bathy_scale * bathend);
		fprintf(output, "\t\t<speed_km_per_hour>%.4f</speed_km_per_hour>\n\t\t<speed_knots>%.4f</speed_knots>\n", spdend, spdend/1.85);
		fprintf(output, "\t\t<heading_degrees>%.4f</heading_degrees>\n", hdgend);
		fprintf(output, "\t\t<sonar_depth_meters>%.4f</sonar_depth_meters>\n\t\t<sonar_altitude_meters>%.4f</sonar_altitude_meters>\n\t</end_of_data>\n", sdpend, altend);
		fprintf(output, "\t<limits>\n\t\t<minimum_longitude>%.9f</minimum_longitude>\n\t\t<maximum_longitude>%.9f</maximum_longitude>\n", lonmin, lonmax);
		fprintf(output, "\t\t<minimum_latitude>%.9f</minimum_latitude>\n\t\t<maximum_latitude>%.9f</maximum_latitude>\n", latmin, latmax);
		fprintf(output, "\t\t<minimum_sonar_depth>%.4f</minimum_sonar_depth>\n\t\t<maximum_sonar_depth>%.4f</maximum_sonar_depth>\n", sdpmin, sdpmax);
		fprintf(output, "\t\t<minimum_altitude>%.4f</minimum_altitude>\n\t\t<maximum_altitude>%.4f</maximum_altitude>\n", altmin, altmax);
		if (ngdbeams > 0 || verbose >= 1) { fprintf(output, "\t\t<minimum_depth>%.4f</minimum_depth>\n\t\t<maximum_depth>%.4f</maximum_depth>\n", bathy_scale * bathmin, bathy_scale * bathmax); }
		if (ngabeams > 0 || verbose >= 1) { fprintf(output, "\t\t<minimum_amplitude>%.4f</minimum_amplitude>\n\t\t<maximum_amplitude>%.4f</maximum_amplitude>\n", ampmin, ampmax); }
		if (ngsbeams > 0 || verbose >= 1) { fprintf(output, "\t\t<minimum_sidescan>%.4f</minimum_sidescan>\n\t\t<maximum_sidescan>%.4f</maximum_sidescan>\n", ssmin, ssmax); }
		fprintf(output, "\t</limits>\n");
		break;
	default: break;
	}

	if (pings_read > 2 && beams_bath_max > 0 && (ngdbeams > 0 || verbose >= 1)) {
		switch (output_format) {
		case FREE_TEXT:
			fprintf(output, "\nBeam Bathymetry Variances:\nPings Averaged: %d\n", pings_read);
			fprintf(output, " Beam     N      Mean     Variance    Sigma\n ----     -      ----     --------    -----\n");
			for (int i = 0; i < beams_bath_max; i++)
				fprintf(output, "%4d  %5d   %8.2f   %8.2f  %8.2f\n", i, nbathvartot[i],
				        bathy_scale * bathmeantot[i],
				        bathy_scale * bathy_scale * bathvartot[i],
				        bathy_scale * sqrt(bathvartot[i]));
			fprintf(output, "\n");
			break;
		case JSON:
			fprintf(output, ",\n\"beam_bathymetry_variances\":{\n\"pings_averaged\": \"%d\",\n", pings_read);
			fprintf(output, "\"columns\" : \"#beam,N,mean,variance,sigma\",\n\"values\": [\n");
			for (int i = 0; i < beams_bath_max; i++) {
				if (i > 0) fprintf(output, ",\n");
				double sigma = bathy_scale * sqrt(bathvartot[i]);
				if (isnan(sigma)) sigma = 0.0;
				fprintf(output, "{\"row\":\"%d,%d,%.2f,%.2f,%.2f\"}", i, nbathvartot[i],
				        bathy_scale * bathmeantot[i], bathy_scale * bathy_scale * bathvartot[i], sigma);
			}
			fprintf(output, "]}");
			break;
		case XML:
			fprintf(output, "\t<beam_bathymetry_variances>\n\t\t<pings_averaged>%d</pings_averaged>\n", pings_read);
			fprintf(output, "\t\t<columns>pixel,N,mean,variance,sigma</columns>\n\t\t<values>\n");
			for (int i = 0; i < beams_bath_max; i++) {
				double sigma = bathy_scale * sqrt(bathvartot[i]);
				if (isnan(sigma)) sigma = 0.0;
				fprintf(output, "\t\t\t<row>%d,%d,%.2f,%.2f,%.2f</row>\n", i, nbathvartot[i],
				        bathy_scale * bathmeantot[i], bathy_scale * bathy_scale * bathvartot[i], sigma);
			}
			fprintf(output, "\t\t</values>\n\t</beam_bathymetry_variances>\n");
			break;
		default: break;
		}
	}
	if (pings_read > 2 && beams_amp_max > 0 && (ngabeams > 0 || verbose >= 1)) {
		switch (output_format) {
		case FREE_TEXT:
			fprintf(output, "\nBeam Amplitude Variances:\nPings Averaged: %d\n", pings_read);
			fprintf(output, " Beam     N      Mean     Variance    Sigma\n ----     -      ----     --------    -----\n");
			for (int i = 0; i < beams_amp_max; i++)
				fprintf(output, "%4d  %5d   %8.2f   %8.2f  %8.2f\n", i, nampvartot[i], ampmeantot[i], ampvartot[i], sqrt(ampvartot[i]));
			fprintf(output, "\n");
			break;
		case JSON:
			fprintf(output, ",\n\"beam_amplitude_variances\":{\n\"pings_averaged\": \"%d\",\n", pings_read);
			fprintf(output, "\"columns\":\"beam,N,mean,variance,sigma\",\n\"values\": [\n");
			for (int i = 0; i < beams_amp_max; i++) {
				if (i > 0) fprintf(output, ",\n");
				double sigma = sqrt(ampvartot[i]); if (isnan(sigma)) sigma = 0;
				fprintf(output, "{\"row\" : \"%d,%d,%.2f,%.2f,%.2f\"}", i, nampvartot[i], ampmeantot[i], ampvartot[i], sigma);
			}
			fprintf(output, "\n]}");
			break;
		case XML:
			fprintf(output, "\t<beam_amplitude_variances>\n\t\t<pings_averaged>%d</pings_averaged>\n", pings_read);
			fprintf(output, "\t\t<columns>pixel,N,mean,variance,sigma</columns>\n\t\t<values>\n");
			for (int i = 0; i < beams_amp_max; i++) {
				double sigma = sqrt(ampvartot[i]); if (isnan(sigma)) sigma = 0.0;
				fprintf(output, "\t\t\t<row>%d,%d,%.2f,%.2f,%.2f</row>\n", i, nampvartot[i], ampmeantot[i], ampvartot[i], sigma);
			}
			fprintf(output, "\t\t</values>\n\t</beam_amplitude_variances>\n");
			break;
		default: break;
		}
	}
	if (pings_read > 2 && pixels_ss_max > 0 && (ngsbeams > 0 || verbose >= 1)) {
		switch (output_format) {
		case FREE_TEXT:
			fprintf(output, "\nPixel Sidescan Variances:\nPings Averaged: %d\n", pings_read);
			fprintf(output, " Beam     N      Mean     Variance    Sigma\n ----     -      ----     --------    -----\n");
			for (int i = 0; i < pixels_ss_max; i++)
				fprintf(output, "%4d  %5d   %8.2f   %8.2f  %8.2f\n", i, nssvartot[i], ssmeantot[i], ssvartot[i], sqrt(ssvartot[i]));
			fprintf(output, "\n");
			break;
		case JSON:
			fprintf(output, ",\n\"pixel_sidescan_variances\":{\n\"pings_averaged\": \"%d\",\n", pings_read);
			fprintf(output, "\"columns\":\"pixel,N,mean,variance,sigma\",\n\"values\": [\n");
			for (int i = 0; i < pixels_ss_max; i++) {
				if (i > 0) fprintf(output, ",\n");
				double sigma = sqrt(ssvartot[i]); if (isnan(sigma)) sigma = 0.0;
				fprintf(output, "{\"row\":\"%d,%d,%.2f,%.2f,%.2f\"}", i, nssvartot[i], ssmeantot[i], ssvartot[i], sigma);
			}
			fprintf(output, "\n]\n}");
			break;
		case XML:
			fprintf(output, "\t<pixel_sidescan_variances>\n\t\t<pings_averaged>%d</pings_averaged>\n", pings_read);
			fprintf(output, "\t\t<columns>pixel,N,mean,variance,sigma</columns>\n\t\t<values>\n");
			for (int i = 0; i < pixels_ss_max; i++) {
				double sigma = sqrt(ssvartot[i]); if (isnan(sigma)) sigma = 0.0;
				fprintf(output, "\t\t\t<row>%d,%d,%.2f,%.2f,%.2f</row>\n", i, nssvartot[i], ssmeantot[i], ssvartot[i], sigma);
			}
			fprintf(output, "\t\t</values>\n\t</pixel_sidescan_variances>\n");
			break;
		default: break;
		}
	}

	if (print_notices) {
		switch (output_format) {
		case FREE_TEXT:
			fprintf(output, "\nData Record Type Notices:\n");
			for (int i = 0; i <= MB_DATA_KINDS; i++)
				if (notice_list_tot[i] > 0) { char *m; mb_notice_message(verbose, i, &m); fprintf(output, "DN: %d %s\n", notice_list_tot[i], m); }
			fprintf(output, "\nNonfatal Error Notices:\n");
			for (int i = MB_DATA_KINDS + 1; i <= MB_DATA_KINDS - (MB_ERROR_MIN); i++)
				if (notice_list_tot[i] > 0) { char *m; mb_notice_message(verbose, i, &m); fprintf(output, "EN: %d %s\n", notice_list_tot[i], m); }
			fprintf(output, "\nProblem Notices:\n");
			for (int i = MB_DATA_KINDS - (MB_ERROR_MIN) + 1; i < MB_NOTICE_MAX; i++)
				if (notice_list_tot[i] > 0) { char *m; mb_notice_message(verbose, i, &m); fprintf(output, "PN: %d %s\n", notice_list_tot[i], m); }
			break;
		case JSON: {
			fprintf(output, ",\n\"notices\": {\n\"data_record_type_notices\": [\n");
			int notice_total = 0;
			for (int i = 0; i <= MB_DATA_KINDS; i++)
				if (notice_list_tot[i] > 0) {
					char *m; mb_notice_message(verbose, i, &m);
					if (notice_total > 0) fprintf(output, ",\n");
					fprintf(output, "{\"notice\": {\n\"notice_number\": \"%d\",\n\"notice_message\": \"%s\"\n}}", notice_list_tot[i], m);
					notice_total++;
				}
			if (notice_total > 0) fprintf(output, "\n");
			fprintf(output, "]");
			notice_total = 0;
			fprintf(output, ",\n\"nonfatal_error_notices\": [\n");
			for (int i = MB_DATA_KINDS + 1; i <= MB_DATA_KINDS - (MB_ERROR_MIN); i++)
				if (notice_list_tot[i] > 0) {
					char *m; mb_notice_message(verbose, i, &m);
					if (notice_total > 0) fprintf(output, ",\n");
					fprintf(output, "{\"notice\": {\n\"notice_number\": \"%d\",\n\"notice_message\": \"%s\"\n}}", notice_list_tot[i], m);
					notice_total++;
				}
			if (notice_total > 0) fprintf(output, "\n");
			fprintf(output, "]");
			notice_total = 0;
			fprintf(output, ",\n\"problem_notices\": [\n");
			for (int i = MB_DATA_KINDS - (MB_ERROR_MIN) + 1; i < MB_NOTICE_MAX; i++)
				if (notice_list_tot[i] > 0) {
					char *m; mb_notice_message(verbose, i, &m);
					if (notice_total > 0) fprintf(output, ",\n");
					fprintf(output, "{\"notice\": {\n\"notice_number\": \"%d\",\n\"notice_message\": \"%s\"\n}}", notice_list_tot[i], m);
					notice_total++;
				}
			if (notice_total > 0) fprintf(output, "\n");
			fprintf(output, "]\n}");
			break;
		}
		case XML:
			fprintf(output, "\t<data_record_type_notices>\n");
			for (int i = 0; i <= MB_DATA_KINDS; i++)
				if (notice_list_tot[i] > 0) {
					char *m; mb_notice_message(verbose, i, &m);
					fprintf(output, "\t\t<notice_number>%d</notice_number>\n\t\t<notice_messsage>%s</notice_messsage>\n", notice_list_tot[i], m);
				}
			fprintf(output, "\t</data_record_type_notices>\n\t<nonfatal_error_notices>\n");
			for (int i = MB_DATA_KINDS + 1; i <= MB_DATA_KINDS - (MB_ERROR_MIN); i++)
				if (notice_list_tot[i] > 0) {
					char *m; mb_notice_message(verbose, i, &m);
					fprintf(output, "\t\t<notice_number>%d</notice_number>\n\t\t<notice_messsage>%s</notice_messsage>\n", notice_list_tot[i], m);
				}
			fprintf(output, "\t</nonfatal_error_notices>\n\t<problem_notices>\n");
			for (int i = MB_DATA_KINDS - (MB_ERROR_MIN) + 1; i < MB_NOTICE_MAX; i++)
				if (notice_list_tot[i] > 0) {
					char *m; mb_notice_message(verbose, i, &m);
					fprintf(output, "\t\t<notice_number>%d</notice_number>\n\t\t<notice_messsage>%s</notice_messsage>\n", notice_list_tot[i], m);
				}
			fprintf(output, "\t</problem_notices>\n");
			break;
		default: break;
		}
	}

	if (coverage_mask) {
		switch (output_format) {
		case FREE_TEXT:
			fprintf(output, "\nCoverage Mask:\nCM dimensions: %d %d\n", mask_nx, mask_ny);
			for (int j = mask_ny - 1; j >= 0; j--) {
				fprintf(output, "CM:  ");
				for (int i = 0; i < mask_nx; i++) fprintf(output, " %1d", mask[i + j * mask_nx]);
				fprintf(output, "\n");
			}
			break;
		case JSON:
			fprintf(output, ",\n\"coverage_mask\": {\n\"dimensions_nx\": \"%d\",\n\"dimensions_ny\": \"%d\",\n\"mask\": \" ", mask_nx, mask_ny);
			for (int j = mask_ny - 1; j >= 0; j--) {
				for (int i = 0; i < mask_nx; i++) { if (i > 0) fprintf(output, ","); fprintf(output, "%1d", mask[i + j * mask_nx]); }
				fprintf(output, "\n");
			}
			fprintf(output, "\"}");
			break;
		case XML: default: break;
		}
	}

	switch (output_format) {
	case FREE_TEXT: break;
	case JSON:      fprintf(output, "}\n"); break;
	case XML:       fprintf(output, "</mbinfo>\n"); break;
	default: break;
	}

	if (output_usefile && output != NULL && output != stream) fclose(output);

	mb_freed(verbose, __FILE__, __LINE__, (void **)&bathmeantot, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&bathvartot,  &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&nbathvartot, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&ampmeantot,  &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&ampvartot,   &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&nampvartot,  &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&ssmeantot,   &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&ssvartot,    &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&nssvartot,   &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&mask,        &error);

	if (mb_memory_list(verbose, &error) == MB_FAILURE)
		fprintf(stderr, "Program %s completed but leaked memory\n", THIS_MODULE_NAME);

	Return(GMT_NOERROR);
}
/* end mbinfo.c */
