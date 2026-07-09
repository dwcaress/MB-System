/*--------------------------------------------------------------------
 *    The MB-system:	mbmapscale.c	6/5/2008
 *
 *    Copyright (c) 2008-2025 by
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
 * mbmapscale converts time values between epoch seconds (seconds since
 * 1970/01/01 00:00:00.000000) and calendar time (e.g. 2008/006/05/17/24/32/0).
 * The input time is set using the command line arguments -Mtime_d for
 * epoch seconds and -Tyear/month/day/hour/minute/second/microsecond for
 * calendar time. The output time (in the form not specified as input) is
 * written to stdout.
 *
 * Author:	D. W. Caress
 * Date:	June 5, 2008
 *
 * GMT-module rewrite of mbmapscale.cc: wrapped as GMT_mbmapscale entry
 * so it can be invoked from the GMT API (Julia FFI / Matlab MEX).
 */

#define THIS_MODULE_NAME		"mbmapscale"
#define THIS_MODULE_LIB			"mbsystem"
#define THIS_MODULE_PURPOSE		"Output scaling between geographic coordinates and local meters east/north at given latitude"
#define THIS_MODULE_KEYS		">D}"
#define THIS_MODULE_NEEDS		""
#define THIS_MODULE_OPTIONS		"-:>Vh"

#include "gmt_dev.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_status.h"

typedef enum {
	MBMAPSCALE_MODE_WGS72   = 0,
	MBMAPSCALE_MODE_ALVINXY = 1
} mapscale_mode_t;

static const char program_name[] = "mbmapscale";
static const char help_message[] =
    "mbmapscale outputs the scaling between geographic coordinates (longitude and latitude)"
    "and local meters east and north at a user defined latitude. The map scale is\n"
    "written to stdout in the form of meters per degree longitude and latitude.";
static const char usage_message[] =
    "mbmapscale [-Llatitude -A -V -H]";

EXTERN_MSC int GMT_mbmapscale(void *API, int mode, void *args);

/* --- Control structure ---------------------------------------------- */

struct MBMAPSCALE_CTRL {
	struct mms_A { bool active; } A;                       /* AlvinXY mode */
	struct mms_L { bool active; double latitude; } L;
};

static void *New_mbmapscale_Ctrl(struct GMT_CTRL *GMT) {
	struct MBMAPSCALE_CTRL *Ctrl = gmt_M_memory(GMT, NULL, 1, struct MBMAPSCALE_CTRL);
	return Ctrl;
}

static void Free_mbmapscale_Ctrl(struct GMT_CTRL *GMT, struct MBMAPSCALE_CTRL *Ctrl) {
	if (!Ctrl) return;
	gmt_M_free(GMT, Ctrl);
}

static int usage(struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose(API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return GMT_NOERROR;
	GMT_Message(API, GMT_TIME_NONE, "usage: %s\n\n", usage_message);
	if (level == GMT_SYNOPSIS) return GMT_PARSE_ERROR;
	GMT_Message(API, GMT_TIME_NONE, "\n%s\n\n", help_message);
	return GMT_PARSE_ERROR;
}

static int parse(struct GMT_CTRL *GMT, struct MBMAPSCALE_CTRL *Ctrl, struct GMT_OPTION *options) {
	unsigned int n_errors = 0;
	struct GMT_OPTION *opt;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {
		case 'A':
			Ctrl->A.active = true;
			break;
		case 'L':
			if (sscanf(opt->arg, "%lf", &Ctrl->L.latitude) > 0) Ctrl->L.active = true; else n_errors++;
			break;
		default:
			n_errors += gmt_default_error(GMT, opt->option);
			break;
		}
	}

	return n_errors ? GMT_PARSE_ERROR : GMT_OK;
}

#define bailout(code)  { gmt_M_free_options(mode); return code; }
#define Return(code)   { Free_mbmapscale_Ctrl(GMT, Ctrl); gmt_end_module(GMT, GMT_cpy); bailout(code); }

/*--------------------------------------------------------------------*/
int GMT_mbmapscale(void *V_API, int mode, void *args) {
	struct MBMAPSCALE_CTRL *Ctrl = NULL;
	struct GMT_CTRL        *GMT  = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION      *options = NULL;
	struct GMTAPI_CTRL     *API = gmt_get_api_ptr(V_API);

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

	Ctrl = New_mbmapscale_Ctrl(GMT);
	{ int perr = parse(GMT, Ctrl, options); if (perr) Return(perr); }

	int verbose = GMT->common.V.active;
	mapscale_mode_t smode = Ctrl->A.active ? MBMAPSCALE_MODE_ALVINXY : MBMAPSCALE_MODE_WGS72;
	double latitude = Ctrl->L.latitude;

	if (verbose == 1) {
		fprintf(stdout, "\nProgram %s\n", program_name);
		fprintf(stdout, "MB-system Version %s\n", MB_VERSION);
	}

	if (verbose >= 2) {
		GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  Program <%s>\n", program_name);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  MB-system Version %s\n", MB_VERSION);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  Control Parameters:\n");
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       verbose:    %d\n", verbose);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       latitude:   %f\n", latitude);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       mode:       %d\n", smode);
	}

	/* calculate mtodeglon and mtodeglat */
	double mtodeglon, mtodeglat;
	int status;
	if (smode == MBMAPSCALE_MODE_WGS72) {
		status = mb_coor_scale(verbose, latitude, &mtodeglon, &mtodeglat);
	}
	else {
		status = mb_alvinxy_scale(verbose, latitude, &mtodeglon, &mtodeglat);
	}
	fprintf(stdout, "\nLocal scaling between degrees longitude and latitude and meters east and north:\n");
	if (smode == MBMAPSCALE_MODE_WGS72) {
		fprintf(stdout, "\tUsing WGS72 ellipsoid\n");
	}
	else {
		fprintf(stdout, "\tUsing 1866 Clark Spheroid as per AlvinXY coordinates\n");
	}
	fprintf(stdout, "\tMeters per degree longitude: %.3f\n", 1.0/mtodeglon);
	fprintf(stdout, "\tMeters per degree latitude:  %.3f\n", 1.0/mtodeglat);
	fprintf(stdout, "\tMeters to degree longitude:  %.9f\n", mtodeglon);
	fprintf(stdout, "\tMeters to degree latitude:   %.9f\n", mtodeglat);

	if (verbose >= 2) {
		GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  Program <%s> completed\n", program_name);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  Ending status:\n");
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       status:  %d\n", status);
	}

	Return(MB_ERROR_NO_ERROR);
}
/*--------------------------------------------------------------------*/
