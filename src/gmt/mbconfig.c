/*--------------------------------------------------------------------
 *    The MB-system:	mbconfig.c	5/5/2017
 *
 *    Copyright (c) 2014-2025 by
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
 * mbconfig provides command line access to the MB-System version and to the
 * locations of the levitus database and the OTPS tidal correction software.
 *
 * Author:	D. W. Caress
 * Date:	May 5, 2017
 *
 * GMT-module rewrite of mbconfig.cc: wrapped as GMT_mbconfig entry
 * so it can be invoked from the GMT API (Julia FFI / Matlab MEX).
 *
 * NOTE on option scheme: the original mbconfig CLI used GNU long options
 * exclusively (--prefix, --cflags, --libs, --version, --version-id,
 * --version-major, --version-minor, --version-archive, --levitus, --otps).
 * GMT modules cannot use getopt_long — argv has already been consumed by the
 * GMT API and options arrive as a `GMT_OPTION` linked list keyed by single
 * letters. The original `static struct option options[]` table is re-expressed
 * as the `mode_keywords[]` lookup table below, dispatched by a single -X
 * option whose argument names the desired output, e.g.:
 *
 *     mbconfig -Xprefix -Xversion-id
 *
 * Multiple -X options may be chained. The keyword list and meaning are
 * preserved verbatim from the original options[] table.
 */

#define THIS_MODULE_NAME		"mbconfig"
#define THIS_MODULE_LIB			"mbsystem"
#define THIS_MODULE_PURPOSE		"Command line access to MB-System version, install prefix, compile/link flags, and database locations"
#define THIS_MODULE_KEYS		">D}"
#define THIS_MODULE_NEEDS		""
#define THIS_MODULE_OPTIONS		"-:>Vh"

#include "gmt_dev.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_status.h"

#ifndef MBSYSTEM_INSTALL_PREFIX
#define MBSYSTEM_INSTALL_PREFIX "/usr/local"
#endif
#ifndef ENV_LEVITUSANNUAL82
#define ENV_LEVITUSANNUAL82 "share/mbsystem/LevitusAnnual82.dat"
#endif
#ifndef ENV_OTPSDIR
#define ENV_OTPSDIR "/usr/local/src/otps"
#endif

static const char help_message[] =
    "mbconfig provides command line access to the MB-System installation location, "
    "the compile and libs flags needed to compile and link programs using MB-System "
    "libraries, and the locations of the levitus database and the OTPS tidal "
    "correction software.\n";
static const char usage_message[] =
    "mbconfig [-Xprefix] [-Xcflags] [-Xlibs] [-Xversion] [-Xversion-id] "
    "[-Xversion-major] [-Xversion-minor] [-Xversion-archive] [-Xlevitus] [-Xotps] [-V] [-H]";

EXTERN_MSC int GMT_mbconfig(void *API, int mode, void *args);

/* --- Lookup table -------------------------------------------------- */
/* Re-expression of the original `static struct option options[]` table
   from mbconfig.cc. Each entry: long-form keyword name + whether the
   keyword takes an argument (mbconfig: none ever did). Dispatched via -X. */

typedef enum {
	MBC_MODE_HELP            = 0,
	MBC_MODE_PREFIX          = 1,
	MBC_MODE_CFLAGS          = 2,
	MBC_MODE_LIBS            = 3,
	MBC_MODE_VERSION         = 4,
	MBC_MODE_VERSION_ID      = 5,
	MBC_MODE_VERSION_MAJOR   = 6,
	MBC_MODE_VERSION_MINOR   = 7,
	MBC_MODE_VERSION_ARCHIVE = 8,
	MBC_MODE_LEVITUS         = 9,
	MBC_MODE_OTPS            = 10,
	MBC_MODE_COUNT           = 11
} mbc_mode_t;

#define MBC_NO_ARG  0
#define MBC_REQ_ARG 1

static const struct {
	const char *name;
	int         has_arg;
	mbc_mode_t  mode;
} mode_keywords[] = {
	{"help",            MBC_NO_ARG, MBC_MODE_HELP},
	{"prefix",          MBC_NO_ARG, MBC_MODE_PREFIX},
	{"cflags",          MBC_NO_ARG, MBC_MODE_CFLAGS},
	{"libs",            MBC_NO_ARG, MBC_MODE_LIBS},
	{"version",         MBC_NO_ARG, MBC_MODE_VERSION},
	{"version-id",      MBC_NO_ARG, MBC_MODE_VERSION_ID},
	{"version-major",   MBC_NO_ARG, MBC_MODE_VERSION_MAJOR},
	{"version-minor",   MBC_NO_ARG, MBC_MODE_VERSION_MINOR},
	{"version-archive", MBC_NO_ARG, MBC_MODE_VERSION_ARCHIVE},
	{"levitus",         MBC_NO_ARG, MBC_MODE_LEVITUS},
	{"otps",            MBC_NO_ARG, MBC_MODE_OTPS},
	{NULL,              0,          MBC_MODE_COUNT}
};

/* --- Control structure ---------------------------------------------- */

struct MBCONFIG_CTRL {
	struct mbc_X { bool active; bool flag[MBC_MODE_COUNT]; } X;
};

static void *New_mbconfig_Ctrl(struct GMT_CTRL *GMT) {
	struct MBCONFIG_CTRL *Ctrl = gmt_M_memory(GMT, NULL, 1, struct MBCONFIG_CTRL);
	return Ctrl;
}

static void Free_mbconfig_Ctrl(struct GMT_CTRL *GMT, struct MBCONFIG_CTRL *Ctrl) {
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

static int parse(struct GMT_CTRL *GMT, struct MBCONFIG_CTRL *Ctrl, struct GMT_OPTION *options) {
	unsigned int n_errors = 0;
	struct GMT_OPTION *opt;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {
		case 'X': {
			bool matched = false;
			for (int k = 0; mode_keywords[k].name != NULL; k++) {
				if (strcmp(opt->arg, mode_keywords[k].name) == 0) {
					Ctrl->X.flag[mode_keywords[k].mode] = true;
					Ctrl->X.active = true;
					matched = true;
					break;
				}
			}
			if (!matched) {
				GMT_Report(API, GMT_MSG_NORMAL, "Syntax -X option: unknown keyword '%s'\n", opt->arg);
				n_errors++;
			}
			break;
		}
		default:
			n_errors += gmt_default_error(GMT, opt->option);
			break;
		}
	}

	return n_errors ? GMT_PARSE_ERROR : GMT_OK;
}

#define bailout(code)  { gmt_M_free_options(mode); return code; }
#define Return(code)   { Free_mbconfig_Ctrl(GMT, Ctrl); gmt_end_module(GMT, GMT_cpy); bailout(code); }

/*--------------------------------------------------------------------*/
int GMT_mbconfig(void *V_API, int mode, void *args) {
	int error = MB_ERROR_NO_ERROR;

	struct MBCONFIG_CTRL *Ctrl = NULL;
	struct GMT_CTRL      *GMT  = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION    *options = NULL;
	struct GMTAPI_CTRL   *API = gmt_get_api_ptr(V_API);

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

	Ctrl = New_mbconfig_Ctrl(GMT);
	if ((error = parse(GMT, Ctrl, options)) != 0) Return (error);

	int verbose = GMT->common.V.active;

	bool mode_set            = Ctrl->X.active;
	bool mode_help           = Ctrl->X.flag[MBC_MODE_HELP];
	bool mode_prefix         = Ctrl->X.flag[MBC_MODE_PREFIX];
	bool mode_cflags         = Ctrl->X.flag[MBC_MODE_CFLAGS];
	bool mode_libs           = Ctrl->X.flag[MBC_MODE_LIBS];
	bool mode_version        = Ctrl->X.flag[MBC_MODE_VERSION];
	bool mode_version_id     = Ctrl->X.flag[MBC_MODE_VERSION_ID];
	bool mode_version_major  = Ctrl->X.flag[MBC_MODE_VERSION_MAJOR];
	bool mode_version_minor  = Ctrl->X.flag[MBC_MODE_VERSION_MINOR];
	bool mode_version_archive = Ctrl->X.flag[MBC_MODE_VERSION_ARCHIVE];
	bool mode_levitus        = Ctrl->X.flag[MBC_MODE_LEVITUS];
	bool mode_otps           = Ctrl->X.flag[MBC_MODE_OTPS];

	/* if no mode specified then just do version */
	if (!mode_set)
		mode_version = true;

	mb_path version_string;
	int version_id;
	int version_major;
	int version_minor;
	int version_archive;

	int status = mb_version(verbose, version_string, &version_id, &version_major, &version_minor, &version_archive, &error);

	if (verbose == 1 || mode_help) {
		fprintf(stderr, "\n# Program %s\n", THIS_MODULE_NAME);
		fprintf(stderr, "# MB-system Version %s\n", version_string);
	}

	if (verbose >= 2) {
		GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  Program <%s>\n", THIS_MODULE_NAME);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  MB-system Version %s\n", version_string);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  Default MB-System Parameters:\n");
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       verbose:                    %d\n", verbose);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       mode_set:                   %d\n", mode_set);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       mode_help:                  %d\n", mode_help);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       mode_prefix:                %d\n", mode_prefix);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       mode_cflags:                %d\n", mode_cflags);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       mode_libs:                  %d\n", mode_libs);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       mode_version:               %d\n", mode_version);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       mode_version_id:            %d\n", mode_version_id);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       mode_version_major:         %d\n", mode_version_major);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       mode_version_minor:         %d\n", mode_version_minor);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       mode_version_archive:       %d\n", mode_version_archive);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       mode_levitus:               %d\n", mode_levitus);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       mode_otps:                  %d\n", mode_otps);
	}

	if (mode_help) {
		fprintf(stderr, "\n%s\n", help_message);
		fprintf(stderr, "\nusage: %s\n", usage_message);
	}

	if (mode_prefix) {
		if (verbose > 0)
			fprintf(stdout, "\n# MB-System install prefix:\n");
		fprintf(stdout, "%s\n", MBSYSTEM_INSTALL_PREFIX);
	}

	if (mode_cflags) {
		if (verbose > 0)
			fprintf(stdout, "\n# MB-System compile flags:\n");
		fprintf(stdout, "-I%s/include\n", MBSYSTEM_INSTALL_PREFIX);
	}

	if (mode_libs) {
		if (verbose > 0)
			fprintf(stdout, "\n# MB-System link flags:\n");
		fprintf(stdout, "-L%s/lib -lmbaux -lmbsapi -lmbbsio -lmbview -lmbgsf -lmbxgr -lmbio\n",
		        MBSYSTEM_INSTALL_PREFIX);
	}

	if (mode_version) {
		if (verbose > 0)
			fprintf(stdout, "\n# MB-System version:\n");
		fprintf(stdout, "%s\n", version_string);
	}

	if (mode_version_id) {
		if (verbose > 0)
			fprintf(stdout, "\n# MB-System version id:\n");
		fprintf(stdout, "%d\n", version_id);
	}

	if (mode_version_major) {
		if (verbose > 0)
			fprintf(stdout, "\n# MB-System major version:\n");
		fprintf(stdout, "%d\n", version_major);
	}

	if (mode_version_minor) {
		if (verbose > 0)
			fprintf(stdout, "\n# MB-System minor version:\n");
		fprintf(stdout, "%d\n", version_minor);
	}

	if (mode_version_archive) {
		if (verbose > 0)
			fprintf(stdout, "\n# MB-System archive version:\n");
		fprintf(stdout, "%d\n", version_archive);
	}

	if (mode_levitus) {
		if (verbose > 0)
			fprintf(stdout, "# MB-System Levitus database location:\n");
		fprintf(stdout, "%s/%s\n", MBSYSTEM_INSTALL_PREFIX, ENV_LEVITUSANNUAL82);
	}

	if (mode_otps) {
		if (verbose > 0)
			fprintf(stdout, "\n# OTPS tide modeling package location:\n");
		fprintf(stdout, "%s\n", ENV_OTPSDIR);
	}

	if (verbose >= 4)
		status = mb_memory_list(verbose, &error);

	if (verbose >= 2) {
		GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  Program <%s> completed\n", THIS_MODULE_NAME);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  Ending status:\n");
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       status:  %d\n", status);
	}

	Return(error);
}
/*--------------------------------------------------------------------*/
