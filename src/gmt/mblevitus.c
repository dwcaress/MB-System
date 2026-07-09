/*--------------------------------------------------------------------
 *    The MB-system:  mblevitus.c   (GMT-module rewrite of mblevitus.cc)
 *
 *--------------------------------------------------------------------
 *    The MB-system:  mblevitus.c  4/15/93
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
 * MBLEVITUS generates an average water velocity profile for a
 * specified location from the Levitus temperature and salinity
 * database.
 *
 * The calculation of water sound velocity from salinity and
 * temperature observations proceeds in two steps. The first
 * step is to calculate the pressure as a function of depth
 * and latitude. We use equations from a 1989 book by Coates:
 * *
 * The second step is to calculate the water sound velocity.
 * We use the DelGrosso equation because of the results presented in
 *    Dusha, Brian D. Worcester, Peter F., Cornuelle, Bruce D.,
 *      Howe, Bruce. M. "On equations for the speed of sound
 *      in seawater", J. Acoust. Soc. Am., Vol 93, No 1,
 *      January 1993, pp 255-275.
 *
 * Author:  D. W. Caress
 * Date:  April 15, 1993
 *--------------------------------------------------------------------*/

#define THIS_MODULE_NAME    "mblevitus"
#define THIS_MODULE_LIB     "mbsystem"
#define THIS_MODULE_PURPOSE "Create a mean annual sound velocity profile for a 1x1 degree region from the Levitus database."
#define THIS_MODULE_KEYS    ">D}"
#define THIS_MODULE_NEEDS ""
#define THIS_MODULE_OPTIONS "-:>Vho"

#include "gmt_dev.h"
#include "mb_define.h"
#include "mb_status.h"

#ifndef _WIN32
#include "levitus.h"
#endif

#define NDEPTH_MAX   46
#define NLEVITUS_MAX 33

static const double MBLEVITUS_NO_DATA = -1000000000.0;

static const double depth_levels[NDEPTH_MAX] = {
	0.0,    10.0,    20.0,    30.0,    50.0,    75.0,   100.0,  125.0,
	150.0,  200.0,   250.0,   300.0,   400.0,   500.0,  600.0,  700.0,
	800.0,  900.0,   1000.0,  1100.0,  1200.0,  1300.0, 1400.0, 1500.0,
	1750.0, 2000.0,  2500.0,  3000.0,  3500.0,  4000.0, 4500.0, 5000.0,
	5500.0, 6000.0,  6500.0,  7000.0,  7500.0,  8000.0, 8500.0, 9000.0,
	9500.0, 10000.0, 10500.0, 11000.0, 11500.0, 12000.0
};

EXTERN_MSC int GMT_mblevitus(void *API, int mode, void *args);

/* --- Control structure ----------------------------------------------- */

struct MBLEVITUS_CTRL {
	struct mbl_A { bool active; } A;                  /* output D,V,T,S */
	struct mbl_H { bool active; } H;                  /* help */
	struct mbl_I { bool active; char *file; } I;      /* explicit Levitus file */
	struct mbl_L { bool active; double lon, lat; } L; /* location (also -R) */
	struct mbl_O { bool active; char *file; } O;      /* output file */
	struct mbl_Z { bool active; } Z;                  /* z positive up */
};

static void *New_mblevitus_Ctrl(struct GMT_CTRL *GMT) {
	struct MBLEVITUS_CTRL *Ctrl = gmt_M_memory(GMT, NULL, 1, struct MBLEVITUS_CTRL);
	return Ctrl;
}

static void Free_mblevitus_Ctrl(struct GMT_CTRL *GMT, struct MBLEVITUS_CTRL *Ctrl) {
	if (!Ctrl) return;
	if (Ctrl->I.file) free(Ctrl->I.file);
	if (Ctrl->O.file) free(Ctrl->O.file);
	gmt_M_free(GMT, Ctrl);
}

static int usage(struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose(API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return GMT_NOERROR;
	GMT_Message(API, GMT_TIME_NONE,
		"usage: mblevitus -Llon/lat [-A] [-I<levitus_file>] [-O[<outfile>]] [-z] [-V] [-H]\n\n");
	if (level == GMT_SYNOPSIS) return GMT_PARSE_ERROR;
	GMT_Message(API, GMT_TIME_NONE,
		"\t-L Longitude/latitude of the SVP location (-R is also accepted).\n"
		"\t-A Output (depth, velocity, temperature, salinity) instead of (depth, velocity).\n"
		"\t-H Print description and exit.\n"
		"\t-I Path to LevitusAnnual82.dat (default: search GMT share dir).\n"
		"\t-O Write SVP to <outfile>. -O alone writes to file 'velocity'.\n"
		"\t   Default (no -O) sends data to GMT stdout (or back to caller via API).\n"
		"\t-z Make z positive up (default is positive down).\n");
	GMT_Option(API, "V,:,o");
	return GMT_PARSE_ERROR;
}

static int parse(struct GMT_CTRL *GMT, struct MBLEVITUS_CTRL *Ctrl, struct GMT_OPTION *options) {
	unsigned int n_errors = 0;
	struct GMT_OPTION *opt;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {
		case '<':
			if (Ctrl->I.file == NULL && gmt_check_filearg(GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET))
				Ctrl->I.file = strdup(opt->arg);
			else
				n_errors++;
			break;
		case '>':
			if (opt->arg && opt->arg[0] && Ctrl->O.file == NULL) {
				Ctrl->O.file = strdup(opt->arg);
				Ctrl->O.active = true;
			}
			break;
		case 'A':
			Ctrl->A.active = true;
			break;
		case 'H':
			Ctrl->H.active = true;
			break;
		case 'I':
			if (opt->arg && opt->arg[0]) {
				if (Ctrl->I.file) free(Ctrl->I.file);
				Ctrl->I.file = strdup(opt->arg);
				Ctrl->I.active = true;
			} else n_errors++;
			break;
		case 'L':
		case 'R': {
			char *arg = strdup(opt->arg);
			char *slash = strchr(arg, '/');
			if (slash) {
				*slash = '\0';
				Ctrl->L.lon = mb_ddmmss_to_degree(arg);
				Ctrl->L.lat = mb_ddmmss_to_degree(slash + 1);
				Ctrl->L.active = true;
			} else {
				GMT_Report(API, GMT_MSG_NORMAL, "Syntax -L option: expected lon/lat\n");
				n_errors++;
			}
			free(arg);
			break;
		}
		case 'O':
			if (Ctrl->O.file) free(Ctrl->O.file);
			if (opt->arg && opt->arg[0])
				Ctrl->O.file = strdup(opt->arg);
			else
				Ctrl->O.file = strdup("velocity");
			Ctrl->O.active = true;
			break;
		case 'z': case 'Z':
			Ctrl->Z.active = true;
			break;
		default:
			n_errors += gmt_default_error(GMT, opt->option);
			break;
		}
	}

	return n_errors ? GMT_PARSE_ERROR : GMT_OK;
}

#define bailout(code) { gmt_M_free_options(mode); return (code); }
#define Return(code)  { Free_mblevitus_Ctrl(GMT, Ctrl); gmt_end_module(GMT, GMT_cpy); bailout(code); }

/* ====================================================================
 * GMT_mblevitus -- GMT module entry point.
 * ==================================================================== */

int GMT_mblevitus(void *V_API, int mode, void *args) {
	int error  = MB_ERROR_NO_ERROR;

	struct MBLEVITUS_CTRL *Ctrl = NULL;
	struct GMT_CTRL    *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION  *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr(V_API);

	if (API == NULL) return GMT_NOT_A_SESSION;
	if (mode == GMT_MODULE_PURPOSE) return usage(API, GMT_MODULE_PURPOSE);
	options = GMT_Create_Options(API, mode, args);
	if (API->error) return API->error;
	if (!options || options->option == GMT_OPT_USAGE)  bailout(usage(API, GMT_USAGE));
	if (options->option == GMT_OPT_SYNOPSIS)           bailout(usage(API, GMT_SYNOPSIS));


#if GMT_MAJOR_VERSION >= 6
	if ((GMT = gmt_init_module(API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout(API->error);
#else
	GMT = gmt_begin_module(API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy);
#endif
	if (GMT_Parse_Common(API, THIS_MODULE_OPTIONS, options)) Return(API->error);

	Ctrl = New_mblevitus_Ctrl(GMT);
	if ((error = parse(GMT, Ctrl, options)) != 0) Return (error);

	int verbose = GMT->common.V.active;
	if (GMT->current.setting.verbose >= GMT_MSG_DEBUG) verbose = 2;

	const bool   help      = Ctrl->H.active;
	const double longitude = Ctrl->L.lon;
	const double latitude  = Ctrl->L.lat;

	char  ofile_buf[MB_PATH_MAXLINE] = {""};
	char *ofile = NULL;
	if (Ctrl->O.active && Ctrl->O.file) {
		strncpy(ofile_buf, Ctrl->O.file, MB_PATH_MAXLINE - 1);
		ofile = ofile_buf;
	}

	if (verbose == 1 || help) {
		GMT_Message(API, GMT_TIME_NONE, "Program %s\n", THIS_MODULE_NAME);
		GMT_Message(API, GMT_TIME_NONE, "MB-system Version %s\n", MB_VERSION);
	}

	if (help) {
		GMT_Message(API, GMT_TIME_NONE,
			"\nMBLEVITUS generates an average water velocity profile for a\n"
			"specified location from the Levitus temperature and salinity database.\n");
		GMT_Message(API, GMT_TIME_NONE,
			"\nusage: mblevitus -Llon/lat [-A] [-I<levitus_file>] [-O[<outfile>]] [-z] [-V] [-H]\n");
		Return(MB_ERROR_NO_ERROR);
	}

	/* Locate the Levitus database file:
	 *   1. -I argument
	 *   2. compile-time path from levitus.h (Unix only)
	 *   3. ${GMT_SHAREDIR}/mbsystem/LevitusAnnual82.dat
	 */
	char levitus_file[MB_PATH_MAXLINE] = {""};
	if (Ctrl->I.file && gmt_access(GMT, Ctrl->I.file, R_OK) == 0) {
		strncpy(levitus_file, Ctrl->I.file, MB_PATH_MAXLINE - 1);
	}
#ifndef _WIN32
	else if (levitusfile != NULL && levitusfile[0] != '\0'
			 && gmt_access(GMT, (char *)levitusfile, R_OK) == 0) {
		strncpy(levitus_file, levitusfile, MB_PATH_MAXLINE - 1);
	}
#endif
	if (levitus_file[0] == '\0') {
		char trial[MB_PATH_MAXLINE];
		snprintf(trial, sizeof(trial), "%s/%s",
				 GMT->session.SHAREDIR, "mbsystem/LevitusAnnual82.dat");
		if (gmt_access(GMT, trial, R_OK) == 0)
			strncpy(levitus_file, trial, MB_PATH_MAXLINE - 1);
	}

	if (levitus_file[0] == '\0') {
		GMT_Report(API, GMT_MSG_NORMAL,
				   "Could not find the Levitus database file LevitusAnnual82.dat. "
				   "Use -I<file> to provide an explicit path.\n");
		Return(MB_ERROR_OPEN_FAIL);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s>\n", THIS_MODULE_NAME);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Control Parameters:\n");
		fprintf(stderr, "dbg2       verbose:     %d\n", verbose);
		fprintf(stderr, "dbg2       help:        %d\n", (int)help);
		fprintf(stderr, "dbg2       levitusfile: %s\n", levitus_file);
		fprintf(stderr, "dbg2       ofile:       %s\n", ofile ? ofile : "(stdout)");
		fprintf(stderr, "dbg2       longitude:   %f\n", longitude);
		fprintf(stderr, "dbg2       latitude:    %f\n", latitude);
	}

	if (longitude < -360.0 || longitude > 360.0 || latitude < -90.0 || latitude > 90.0) {
		GMT_Report(API, GMT_MSG_NORMAL,
				   "Invalid location specified: longitude=%f latitude=%f\n", longitude, latitude);
		Return(MB_ERROR_BAD_PARAMETER);
	}

	FILE *ifp = fopen(levitus_file, "rb");
	if (ifp == NULL) {
		GMT_Report(API, GMT_MSG_NORMAL,
				   "Unable to open Levitus database file <%s> for reading\n", levitus_file);
		Return(MB_ERROR_OPEN_FAIL);
	}

	/* longitude/latitude indices into the Levitus 1x1 grid */
	int ilon;
	if (longitude < 0.0)        ilon = (int)(longitude + 360.0);
	else if (longitude >= 360.0) ilon = (int)(longitude - 360.0);
	else                        ilon = (int)longitude;
	const double lon_actual = ilon + 0.5;
	const int    ilat       = (int)(latitude + 90.0);
	const double lat_actual = ilat - 89.5;

	GMT_Report(API, GMT_MSG_VERBOSE, "Location for mean annual water velocity profile:\n");
	GMT_Report(API, GMT_MSG_VERBOSE, "  Requested: %6.4f longitude  %6.4f latitude\n", longitude, latitude);
	GMT_Report(API, GMT_MSG_VERBOSE, "  Used:      %6.4f longitude  %6.4f latitude\n", lon_actual, lat_actual);

	int status = MB_SUCCESS;

	/* read the temperature slab */
	const int record_size = (int)(sizeof(float) * NLEVITUS_MAX * 180);
	long location = (long)ilon * record_size;
	fseek(ifp, location, 0);
	float temperature[NLEVITUS_MAX][180];
	if ((int)fread(&temperature[0][0], 1, record_size, ifp) != record_size) {
		status = MB_FAILURE;
		error  = MB_ERROR_EOF;
		GMT_Report(API, GMT_MSG_NORMAL, "ERROR: EOF reading temperature\n");
	}

	/* read the salinity slab */
	location = location + 360L * record_size;
	fseek(ifp, location, 0);
	float salinity[NLEVITUS_MAX][180];
	if ((int)fread(&salinity[0][0], 1, record_size, ifp) != record_size) {
		status = MB_FAILURE;
		error  = MB_ERROR_EOF;
		GMT_Report(API, GMT_MSG_NORMAL, "ERROR: EOF reading salinity\n");
	}

	fclose(ifp);

#ifdef BYTESWAPPED
	for (int i = 0; i < NLEVITUS_MAX; i++) {
		mb_swap_float(&temperature[i][ilat]);
		mb_swap_float(&salinity[i][ilat]);
	}
#endif

	/* compute velocity from temperature and salinity (DelGrosso) */
	int    nvelocity     = 0;
	int    nvelocity_tot = 0;
	int    last_good     = -1;
	double velocity[NDEPTH_MAX];
	for (int i = 0; i < NDEPTH_MAX; i++) {
		if (i < NLEVITUS_MAX && salinity[i][ilat] > MBLEVITUS_NO_DATA) {
			last_good = i;
			nvelocity++;
		}
		if (last_good >= 0) {
			nvelocity_tot++;

			/* pressure for a given depth as a function of latitude */
			const double pressure_dbar = 1.0052405 * depth_levels[i] * (1.0 + 0.00528 * sin(DTR * lat_actual) * sin(DTR * lat_actual)) +
			                             0.00000236 * depth_levels[i] * depth_levels[i];

			/* convert decibar to kg/cm**2 */
			const double pressure = pressure_dbar * 0.1019716;
			const double c0 = 1402.392;
			const double T  = temperature[last_good][ilat];
			const double S  = salinity[last_good][ilat];
			const double dltact = T * (5.01109398873 + T * (-0.0550946843172 + T * 0.000221535969240));
			const double dltacs = S * (1.32952290781 + S * 0.000128955756844);
			const double dltacp = pressure * (0.156059257041E0 + pressure * (0.000024499868841 + pressure * -0.00000000883392332513));
			const double dcstp = T * (-0.0127562783426 * S + pressure * (0.00635191613389 +
			                     pressure * (0.265484716608E-7 * T - 0.00000159349479045 +
			                     0.522116437235E-9 * pressure) - 0.000000438031096213 * T * T)) +
				                 S * (-0.161674495909E-8 * S * pressure * pressure + T * (0.0000968403156410 * T +
			                     pressure * (0.00000485639620015 * S - 0.000340597039004)));
			velocity[i] = c0 + dltact + dltacs + dltacp + dcstp;
		} else {
			velocity[i] = 0.0;
		}
	}

	if (nvelocity < 1) {
		GMT_Report(API, GMT_MSG_NORMAL, "No water velocity profile available for the specified location.\n");
		GMT_Report(API, GMT_MSG_NORMAL, "This place is probably subaerial; no output created.\n");
		Return(MB_ERROR_BAD_PARAMETER);
	}

	/* Build a GMT dataset: 1 table, 1 segment, nvelocity_tot rows, 2 or 4 cols */
	uint64_t dim[4] = {1, 1, 0, 2};
	if (Ctrl->A.active) dim[3] = 4;
	struct GMT_DATASET *D = GMT_Create_Data(API, GMT_IS_DATASET, GMT_IS_LINE, 0, dim, NULL, NULL, 0, 0, NULL);
	if (D == NULL) Return(API->error);

	char header[GMT_BUFSIZ * 2] = {""};
	char buff[GMT_LEN256];
	snprintf(header, sizeof(header), "Sound velocity profile created by %s MB-system Version %s", THIS_MODULE_NAME, MB_VERSION); {
		char user[256] = {""}, host[256] = {""}, date[32] = {""};
		mb_user_host_date(verbose, user, host, date, &error);
		snprintf(buff, sizeof(buff), "\n# Run by user <%s> on cpu <%s> at <%s>", user, host, date);
		strncat(header, buff, sizeof(header) - strlen(header) - 1);
	}
	snprintf(buff, sizeof(buff), "\n# Water velocity profile derived from Levitus temperature and salinity database.");
	strncat(header, buff, sizeof(header) - strlen(header) - 1);
	snprintf(buff, sizeof(buff), "\n# Annual average for a 1x1 degree area centered at %6.4f longitude and %6.4f latitude.",
			 lon_actual, lat_actual);
	strncat(header, buff, sizeof(header) - strlen(header) - 1);
	if (Ctrl->A.active)
		snprintf(buff, sizeof(buff), "\n# Columns: depth(m) velocity(m/s) temperature(C) salinity(PSU).");
	else
		snprintf(buff, sizeof(buff), "\n# Columns: depth(m) velocity(m/s).");
	strncat(header, buff, sizeof(header) - strlen(header) - 1);
	snprintf(buff, sizeof(buff),
			 "\n# First %d velocity values use Levitus T/S directly; remaining %d use the deepest available T/S.",
			 nvelocity, nvelocity_tot - nvelocity);
	strncat(header, buff, sizeof(header) - strlen(header) - 1);

	struct GMT_DATASEGMENT *S = GMT_Alloc_Segment(API, GMT_NO_STRINGS, nvelocity_tot, dim[3], NULL, NULL);
	S->header = strdup(header);

	const double zdir = Ctrl->Z.active ? -1.0 : 1.0;
	const unsigned int ix = (GMT->current.setting.io_lonlat_toggle[GMT_IN]) ? 1 : 0;
	const unsigned int iy = 1 - ix;
	for (int i = 0; i < nvelocity_tot; i++) {
		S->data[ix][i] = zdir * depth_levels[i];
		S->data[iy][i] = velocity[i];
	}
	if (Ctrl->A.active) {
		for (int i = 0; i < nvelocity; i++) {
			S->data[2][i] = temperature[i][ilat];
			S->data[3][i] = salinity[i][ilat];
		}
		for (int i = nvelocity; i < nvelocity_tot; i++) {
			S->data[2][i] = 0.0;
			S->data[3][i] = 0.0;
		}
	}
	S->n_rows = nvelocity_tot;
	D->table[0]->segment[0] = S;
	D->table[0]->n_records += nvelocity_tot;
	D->n_records           += nvelocity_tot;

	GMT_Report(API, GMT_MSG_VERBOSE, "Values defined directly by Levitus database:      %d\n", nvelocity);
	GMT_Report(API, GMT_MSG_VERBOSE, "Values assuming deepest salinity and temperature: %d\n", nvelocity_tot - nvelocity);
	GMT_Report(API, GMT_MSG_VERBOSE, "Velocity points written:                          %d\n", nvelocity_tot);

	/* Use '#' as the segment marker so the single-segment header looks like comments */
	const char saved_seg_marker = GMT->current.setting.io_seg_marker[GMT_OUT];
	GMT->current.setting.io_seg_marker[GMT_OUT] = '#';
	if (GMT->current.setting.io_lonlat_toggle[GMT_IN])
		GMT->current.setting.io_lonlat_toggle[GMT_OUT] = false;

	const int wstatus = GMT_Write_Data(API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE,
									   GMT_WRITE_SET, NULL, ofile, D);
	GMT->current.setting.io_seg_marker[GMT_OUT] = saved_seg_marker;
	if (wstatus != GMT_NOERROR) Return(API->error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s> completed\n", THIS_MODULE_NAME);
		fprintf(stderr, "dbg2  Ending status:\n");
		fprintf(stderr, "dbg2       status: %d\n", status);
		fprintf(stderr, "dbg2       error:  %d\n", error);
	}

	Return(error);
}
