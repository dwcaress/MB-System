/*--------------------------------------------------------------------
 *    The MB-system:  mbpreprocess.c  1/8/2014
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
 * MBpreprocess handles preprocessing of swath sonar data as part of setting
 * up an MB-System processing structure for a dataset.
 *
 * This program replaces the several format-specific preprocessing programs
 * found in MB-System version 5 releases with a single program for version 6.
 *
 * Author:  D. W. Caress
 * Date:  January 8, 2014
 *
 * GMT module wrapper: re-converted from src/utilities/mbpreprocess.cc to C
 * and wrapped as a GMT module entry so it can be invoked from the GMT API
 * (and therefore from Julia FFI / Matlab MEX via GMT).
 */

#define THIS_MODULE_CLASSIC_NAME	"mbpreprocess"
#define THIS_MODULE_MODERN_NAME		"mbpreprocess"
#define THIS_MODULE_LIB			"mbgmt"
#define THIS_MODULE_PURPOSE		"Preprocess swath sonar data for MB-System processing"
#define THIS_MODULE_KEYS        "ID{,OD("
#define THIS_MODULE_NEEDS		""
#define THIS_MODULE_OPTIONS		"->V"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "->V"

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_process.h"
#include "mb_status.h"
#include "mbsys_ldeoih.h"

EXTERN_MSC int GMT_mbpreprocess(void *API, int mode, void *args);

#define MBPREPROCESS_ALLOC_CHUNK 1000

typedef enum {
	MBPREPROCESS_MERGE_OFF = 0,
	MBPREPROCESS_MERGE_FILE = 1,
	MBPREPROCESS_MERGE_ASYNC = 2
} merge_t;

#define MBPREPROCESS_TIME_LATENCY_APPLY_NONE            0x00
#define MBPREPROCESS_TIME_LATENCY_APPLY_NAV             0x01
#define MBPREPROCESS_TIME_LATENCY_APPLY_SENSORDEPTH     0x02
#define MBPREPROCESS_TIME_LATENCY_APPLY_ALTITUDE        0x04
#define MBPREPROCESS_TIME_LATENCY_APPLY_HEADING         0x08
#define MBPREPROCESS_TIME_LATENCY_APPLY_ATTITUDE        0x10
#define MBPREPROCESS_TIME_LATENCY_APPLY_SOUNDSPEED      0x20
#define MBPREPROCESS_TIME_LATENCY_APPLY_UNUSED          0x40
#define MBPREPROCESS_TIME_LATENCY_APPLY_ALL_ANCILLIARY  0x7F
#define MBPREPROCESS_TIME_LATENCY_APPLY_SURVEY          0x80
#define MBPREPROCESS_TIME_LATENCY_APPLY_ALL             0xFF

#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

static const char program_name[] = "mbpreprocess";
static const char help_message[] =
    "mbpreprocess handles preprocessing of swath sonar data as part of setting up an MB-System processing "
    "structure for a dataset.\n";
static const char usage_message[] =
    "mbpreprocess\n"
    "\t--verbose\n"
    "\t--help\n\n"
    "\t--input=datalist\n"
    "\t--format=format_id\n\n"
    "\t--platform-file=platform_file\n"
    "\t--platform-target-sensor=sensor_id\n\n"
    "\t--output-sensor-fnv\n"
    "\t--skip-existing\n\n"
    "\t--nav-file=file\n"
    "\t--nav-file-format=format_id\n"
    "\t--nav-async=record_kind\n"
    "\t--nav-sensor=sensor_id\n\n"
    "\t--sensordepth-file=file\n"
    "\t--sensordepth-file-format=format_id\n"
    "\t--sensordepth-async=record_kind\n"
    "\t--sensordepth-sensor=sensor_id\n\n"
    "\t--heading-file=file\n"
    "\t--heading-file-format=format_id\n"
    "\t--heading-async=record_kind\n"
    "\t--heading-sensor=sensor_id\n\n"
    "\t--altitude-file=file\n"
    "\t--altitude-file-format=format_id\n"
    "\t--altitude-async=record_kind\n"
    "\t--altitude-sensor=sensor_id\n"
    "\t--attitude-file=file\n"
    "\t--attitude-file-format=format_id\n"
    "\t--attitude-async=record_kind\n"
    "\t--attitude-sensor=sensor_id\n\n"
    "\t--attitude-zero-heave\n\n"
    "\t--soundspeed-file=file\n"
    "\t--soundspeed-file-format=format_id\n"
    "\t--soundspeed-async=record_kind\n"
    "\t--soundspeed-sensor=sensor_id\n\n"
    "\t--time-latency-file=file\n"
    "\t--time-latency-file-format=format_id\n"
    "\t--time-latency-constant=value\n"
    "\t--time-latency-apply-nav\n"
    "\t--time-latency-apply-sensordepth\n"
    "\t--time-latency-apply-heading\n"
    "\t--time-latency-apply-attitude\n"
    "\t--time-latency-apply-all-ancilliary\n"
    "\t--time-latency-apply-survey\n"
    "\t--time-latency-apply-all\n\n"
    "\t--filter=value\n"
    "\t--filter-apply-nav\n"
    "\t--filter-apply-sensordepth\n"
    "\t--filter-apply-heading\n"
    "\t--filter-apply-attitude\n"
    "\t--filter-apply-all-ancilliary\n\n"
    "\t--recalculate-bathymetry\n"
    "\t--no-change-survey\n"
    "\t--multibeam-sidescan-source=recordid\n"
    "\t--sounding-amplitude-filter=value\n"
    "\t--sounding-altitude-filter=value\n"
    "\t--ignore-water-column\n"
    "\t--head1-offsets=x/y/z/heading/roll/pitch\n"
    "\t--head2-offsets=x/y/z/heading/roll/pitch\n"
    "\t--kluge-time-jumps=threshold\n"
    "\t--kluge-fix-7k-timestamps=targetoffset\n"
    "\t--kluge-ancilliary-time-jumps=threshold\n"
    "\t--kluge-mbaripressure-time-jumps=threshold\n"
    "\t--kluge-beam-tweak=factor\n"
    "\t--kluge-soundspeed-tweak=factor\n"
    "\t--kluge-zero-attitude-correction\n"
    "\t--kluge-zero-alongtrack-angles\n"
    "\t--kluge-fix-wissl-timestamps\n"
    "\t--kluge-auv-sentry-sensordepth\n"
    "\t--kluge-ignore-snippets\n"
    "\t--kluge-sensordepth-from-heave\n"
    "\t--kluge-early-MBARI-Mapping-AUV\n"
    "\t--kluge-flipsign-roll\n"
    "\t--kluge-flipsign-pitch\n"
    "\t--kluge-set-beamwidths=beamwidth-acrosstrack/beamwidth-alongtrack\n"
    "\t--kluge-set-beamwidth-acrosstrack=beamwidth-acrosstrack\n"
    "\t--kluge-set-beamwidth-alongtrack=beamwidth-alongtrack\n"
    "\t--kluge-ignore-duplicate-pings\n"
    "\t--kluge-xducer-depth-from-heave\n"
    "\t--kluge-xducer-depth-from-sensordepth\n"
    "\t--kluge-xducer-depth-from-heave-and-sensordepth\n"
    "\t--kluge-rangescale\n"
    "\t--kluge-fix-wissl2-ranges\n";

/* --- Control structure ---------------------------------------------- */
struct MBPREPROCESS_CTRL {
	/* input file / format */
	bool I_active;
	mb_path read_file;
	bool F_active;
	int  format;

	/* output / aux */
	bool output_directory_set;
	mb_path output_directory;
	bool output_datalist_set;
	char output_datalist[MB_PATH_MAXLINE+100];
	bool use_platform_file;
	mb_path platform_file;
	int  target_sensor;
	bool output_sensor_fnv;
	bool skip_existing;
	bool help;

	/* nav */
	merge_t nav_mode;
	mb_path nav_file;
	int  nav_file_format;
	int  nav_async;
	int  nav_sensor;

	/* sensordepth */
	merge_t sensordepth_mode;
	mb_path sensordepth_file;
	int  sensordepth_file_format;
	int  sensordepth_async;
	int  sensordepth_sensor;

	/* heading */
	merge_t heading_mode;
	mb_path heading_file;
	int  heading_file_format;
	int  heading_async;
	int  heading_sensor;

	/* altitude */
	merge_t altitude_mode;
	mb_path altitude_file;
	int  altitude_file_format;
	int  altitude_async;
	int  altitude_sensor;

	/* attitude */
	merge_t attitude_mode;
	mb_path attitude_file;
	int  attitude_file_format;
	int  attitude_async;
	int  attitude_sensor;
	bool zero_heave;

	/* soundspeed */
	merge_t soundspeed_mode;
	mb_path soundspeed_file;
	int  soundspeed_file_format;
	int  soundspeed_async;
	int  soundspeed_sensor;

	/* time latency */
	int    time_latency_mode;
	int    time_latency_format;
	double time_latency_constant;
	mb_path time_latency_file;
	mb_u_char time_latency_apply;

	/* filter */
	double    filter_length;
	mb_u_char filter_apply;

	/* kluges */
	double kluge_timejumps_threshold;
	bool   kluge_timejumps;
	bool   kluge_fix7ktimestamps;
	double kluge_fix7ktimestamps_targetoffset;
	double kluge_timejumps_anc_threshold;
	bool   kluge_timejumps_ancilliary;
	double kluge_timejumps_mba_threshold;
	bool   kluge_timejumps_mbaripressure;
	double kluge_beamtweak_factor;
	bool   kluge_beamtweak;
	double kluge_soundspeedtweak_factor;
	bool   kluge_soundspeedtweak;
	bool   kluge_fix_wissl_timestamps;
	bool   kluge_auv_sentry_sensordepth;
	bool   kluge_ignore_snippets;
	bool   kluge_sensordepth_from_heave;
	bool   kluge_early_mbari_mapping_auv;
	bool   kluge_flipsign_roll;
	bool   kluge_flipsign_pitch;
	bool   kluge_setbeamwidthacrosstrack;
	bool   kluge_setbeamwidthalongtrack;
	bool   kluge_ignore_duplicate_pings;
	bool   kluge_xducer_depth_from_heave;
	bool   kluge_xducer_depth_from_sensordepth;
	bool   kluge_xducer_depth_from_heaveandsensordepth;
	bool   kluge_rangescale;
	double kluge_rangescale_factor;
	bool   kluge_fix_wissl2_ranges;

	/* preprocess params (passed to mb_preprocess) */
	struct mb_preprocess_struct preprocess_pars;
};

static void *New_mbpreprocess_Ctrl(struct GMT_CTRL *GMT) {
	struct MBPREPROCESS_CTRL *Ctrl = gmt_M_memory(GMT, NULL, 1, struct MBPREPROCESS_CTRL);
	strcpy(Ctrl->read_file, "datalist.mb-1");
	strcpy(Ctrl->output_datalist, "datalist.mb-1");
	Ctrl->target_sensor = -1;
	Ctrl->nav_mode = MBPREPROCESS_MERGE_OFF;
	Ctrl->nav_async = MB_DATA_DATA;
	Ctrl->nav_sensor = -1;
	Ctrl->sensordepth_mode = MBPREPROCESS_MERGE_OFF;
	Ctrl->sensordepth_async = MB_DATA_DATA;
	Ctrl->sensordepth_sensor = -1;
	Ctrl->heading_mode = MBPREPROCESS_MERGE_OFF;
	Ctrl->heading_async = MB_DATA_DATA;
	Ctrl->heading_sensor = -1;
	Ctrl->altitude_mode = MBPREPROCESS_MERGE_OFF;
	Ctrl->altitude_async = MB_DATA_DATA;
	Ctrl->altitude_sensor = -1;
	Ctrl->attitude_mode = MBPREPROCESS_MERGE_OFF;
	Ctrl->attitude_async = MB_DATA_DATA;
	Ctrl->attitude_sensor = -1;
	Ctrl->soundspeed_mode = MBPREPROCESS_MERGE_OFF;
	Ctrl->soundspeed_async = MB_DATA_DATA;
	Ctrl->soundspeed_sensor = -1;
	Ctrl->time_latency_mode = MB_SENSOR_TIME_LATENCY_NONE;
	Ctrl->time_latency_format = 1;
	Ctrl->kluge_fix7ktimestamps = true;
	Ctrl->kluge_beamtweak_factor = 1.0;
	Ctrl->kluge_soundspeedtweak_factor = 1.0;
	Ctrl->kluge_rangescale_factor = 1.0;
	memset(&Ctrl->preprocess_pars, 0, sizeof(struct mb_preprocess_struct));
	return Ctrl;
}

static void Free_mbpreprocess_Ctrl(struct GMT_CTRL *GMT, struct MBPREPROCESS_CTRL *Ctrl) {
	if (!Ctrl) return;
	gmt_M_free(GMT, Ctrl);
}

static int usage(struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose(API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return GMT_NOERROR;
	GMT_Message(API, GMT_TIME_NONE, "usage: %s", usage_message);
	if (level == GMT_SYNOPSIS) return GMT_PARSE_ERROR;
	GMT_Message(API, GMT_TIME_NONE, "\n%s\n", help_message);
	GMT_Message(API, GMT_TIME_NONE, "MB-System Version %s\n", MB_VERSION);
	return GMT_PARSE_ERROR;
}

/*--------------------------------------------------------------------
 * Canonical long-option table — mirrors the static struct option[]
 * used by the original src/utilities/mbpreprocess.cc with getopt_long().
 * This table is kept here as the authoritative list of valid options
 * and their required-/no-argument expectation. parse() below uses it
 * to validate option names that arrive pre-tokenized through the
 * GMT_OPTION linked list (GMT modules cannot use getopt_long since
 * argv is already consumed by the GMT API). */
enum mbpp_arg { MBPP_NO_ARG = 0, MBPP_REQ_ARG = 1 };
struct mbpp_longopt { const char *name; enum mbpp_arg has_arg; };
static const struct mbpp_longopt mbpp_options[] = {
	{"verbose",                                          MBPP_NO_ARG},
	{"help",                                             MBPP_NO_ARG},
	{"input",                                            MBPP_REQ_ARG},
	{"format",                                           MBPP_REQ_ARG},
	{"output-directory",                                 MBPP_REQ_ARG},
	{"output-datalist",                                  MBPP_REQ_ARG},
	{"platform-file",                                    MBPP_REQ_ARG},
	{"platform-target-sensor",                           MBPP_REQ_ARG},
	{"output-sensor-fnv",                                MBPP_NO_ARG},
	{"skip-existing",                                    MBPP_NO_ARG},
	{"nav-file",                                         MBPP_REQ_ARG},
	{"nav-file-format",                                  MBPP_REQ_ARG},
	{"nav-async",                                        MBPP_REQ_ARG},
	{"nav-sensor",                                       MBPP_REQ_ARG},
	{"sensordepth-file",                                 MBPP_REQ_ARG},
	{"sensordepth-file-format",                          MBPP_REQ_ARG},
	{"sensordepth-async",                                MBPP_REQ_ARG},
	{"sensordepth-sensor",                               MBPP_REQ_ARG},
	{"heading-file",                                     MBPP_REQ_ARG},
	{"heading-file-format",                              MBPP_REQ_ARG},
	{"heading-async",                                    MBPP_REQ_ARG},
	{"heading-sensor",                                   MBPP_REQ_ARG},
	{"altitude-file",                                    MBPP_REQ_ARG},
	{"altitude-file-format",                             MBPP_REQ_ARG},
	{"altitude-async",                                   MBPP_REQ_ARG},
	{"altitude-sensor",                                  MBPP_REQ_ARG},
	{"attitude-file",                                    MBPP_REQ_ARG},
	{"attitude-file-format",                             MBPP_REQ_ARG},
	{"attitude-async",                                   MBPP_REQ_ARG},
	{"attitude-sensor",                                  MBPP_REQ_ARG},
	{"attitude-zero-heave",                              MBPP_NO_ARG},
	{"soundspeed-file",                                  MBPP_REQ_ARG},
	{"soundspeed-file-format",                           MBPP_REQ_ARG},
	{"soundspeed-async",                                 MBPP_REQ_ARG},
	{"soundspeed-sensor",                                MBPP_REQ_ARG},
	{"time-latency-file",                                MBPP_REQ_ARG},
	{"time-latency-file-format",                         MBPP_REQ_ARG},
	{"time-latency-constant",                            MBPP_REQ_ARG},
	{"time-latency-apply-nav",                           MBPP_NO_ARG},
	{"time-latency-apply-sensordepth",                   MBPP_NO_ARG},
	{"time-latency-apply-heading",                       MBPP_NO_ARG},
	{"time-latency-apply-attitude",                      MBPP_NO_ARG},
	{"time-latency-apply-altitude",                      MBPP_NO_ARG},
	{"time-latency-apply-all-ancilliary",                MBPP_NO_ARG},
	{"time-latency-apply-survey",                        MBPP_NO_ARG},
	{"time-latency-apply-all",                           MBPP_NO_ARG},
	{"filter",                                           MBPP_REQ_ARG},
	{"filter-apply-nav",                                 MBPP_NO_ARG},
	{"filter-apply-sensordepth",                         MBPP_NO_ARG},
	{"filter-apply-heading",                             MBPP_NO_ARG},
	{"filter-apply-attitude",                            MBPP_NO_ARG},
	{"filter-apply-altitude",                            MBPP_NO_ARG},
	{"filter-apply-all-ancilliary",                      MBPP_NO_ARG},
	{"recalculate-bathymetry",                           MBPP_NO_ARG},
	{"no-change-survey",                                 MBPP_NO_ARG},
	{"multibeam-sidescan-source",                        MBPP_REQ_ARG},
	{"sounding-amplitude-filter",                        MBPP_REQ_ARG},
	{"sounding-altitude-filter",                         MBPP_REQ_ARG},
	{"ignore-water-column",                              MBPP_NO_ARG},
	{"head1-offsets",                                    MBPP_REQ_ARG},
	{"head2-offsets",                                    MBPP_REQ_ARG},
	{"kluge-time-jumps",                                 MBPP_REQ_ARG},
	{"kluge-fix-7k-timestamps",                          MBPP_REQ_ARG},
	{"kluge-ancilliary-time-jumps",                      MBPP_REQ_ARG},
	{"kluge-mbaripressure-time-jumps",                   MBPP_REQ_ARG},
	{"kluge-beam-tweak",                                 MBPP_REQ_ARG},
	{"kluge-soundspeed-tweak",                           MBPP_REQ_ARG},
	{"kluge-zero-attitude-correction",                   MBPP_NO_ARG},
	{"kluge-zero-alongtrack-angles",                     MBPP_NO_ARG},
	{"kluge-fix-wissl-timestamps",                       MBPP_NO_ARG},
	{"kluge-auv-sentry-sensordepth",                     MBPP_NO_ARG},
	{"kluge-ignore-snippets",                            MBPP_NO_ARG},
	{"kluge-sensordepth-from-heave",                     MBPP_NO_ARG},
	{"kluge-early-MBARI-Mapping-AUV",                    MBPP_NO_ARG},
	{"kluge-flipsign-roll",                              MBPP_NO_ARG},
	{"kluge-flipsign-pitch",                             MBPP_NO_ARG},
	{"kluge-set-beamwidths",                             MBPP_REQ_ARG},
	{"kluge-set-beamwidth-acrosstrack",                  MBPP_REQ_ARG},
	{"kluge-set-beamwidth-alongtrack",                   MBPP_REQ_ARG},
	{"kluge-ignore-duplicate-pings",                     MBPP_NO_ARG},
	{"kluge-xducer-depth-from-heave",                    MBPP_NO_ARG},
	{"kluge-xducer-depth-from-sensordepth",              MBPP_NO_ARG},
	{"kluge-xducer-depth-from-heave-and-sensordepth",    MBPP_NO_ARG},
	{"kluge-rangescale",                                 MBPP_REQ_ARG},
	{"kluge-fix-wissl2-ranges",                          MBPP_NO_ARG},
	{NULL, MBPP_NO_ARG}
};

static const struct mbpp_longopt *mbpp_find_opt(const char *name) {
	for (const struct mbpp_longopt *o = mbpp_options; o->name; o++)
		if (strcmp(o->name, name) == 0) return o;
	return NULL;
}

/* split "name=value" or "name" into name + value (value may be empty string) */
static void split_longopt(const char *s, char *name, size_t name_sz, const char **val_out) {
	const char *eq = strchr(s, '=');
	if (eq) {
		size_t n = (size_t)(eq - s);
		if (n >= name_sz) n = name_sz - 1;
		memcpy(name, s, n);
		name[n] = '\0';
		*val_out = eq + 1;
	} else {
		strncpy(name, s, name_sz - 1);
		name[name_sz - 1] = '\0';
		*val_out = "";
	}
}

#define MATCH(s) (strcmp(name, (s)) == 0)

/*--------------------------------------------------------------------
 * parse() — long-option dispatch
 *
 * The original src/utilities/mbpreprocess.cc used a standalone main()
 * with getopt_long() driving a switch on c=0 + strcmp(option_index name).
 * As a GMT module, argv has already been consumed by the GMT API and
 * options arrive as a GMT_OPTION linked list, so getopt_long() cannot
 * be used here. The semantics are preserved by:
 *   - validating each option name against mbpp_options[] above,
 *   - dispatching via MATCH() (strcmp) on the name,
 *   - extracting any "=value" tail through split_longopt().
 * The set of accepted option names and their required-/no-argument
 * expectations match the original .cc options[] table exactly. */
static int parse(struct GMT_CTRL *GMT, struct MBPREPROCESS_CTRL *Ctrl, struct GMT_OPTION *options) {
	unsigned int n_errors = 0;
	struct GMT_OPTION *opt;
	struct GMTAPI_CTRL *API = GMT->parent;
	struct mb_preprocess_struct *pp = &Ctrl->preprocess_pars;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {
		case '<':
			/* positional input file or unrecognized long option text */
			if (opt->arg && opt->arg[0] == '-' && opt->arg[1] == '-') {
				/* long option arriving as positional, dispatch below */
			} else if (opt->arg) {
				strcpy(Ctrl->read_file, opt->arg);
				Ctrl->I_active = true;
				continue;
			} else {
				continue;
			}
			/* fall through into '-' handling */
		case '-': {
			char name[64];
			const char *val = "";
			const char *src = opt->arg;
			/* if arriving via '<' the leading "--" is present in opt->arg */
			if (opt->option == '<' && src && src[0] == '-' && src[1] == '-')
				src += 2;
			split_longopt(src ? src : "", name, sizeof(name), &val);

			/* validate option name and arg-presence against canonical table */
			{
				const struct mbpp_longopt *od = mbpp_find_opt(name);
				if (od == NULL) {
					GMT_Report(API, GMT_MSG_NORMAL, "Unknown option --%s\n", name);
					n_errors++;
					break;
				}
				if (od->has_arg == MBPP_REQ_ARG && (val == NULL || val[0] == '\0')) {
					GMT_Report(API, GMT_MSG_NORMAL, "Option --%s requires an argument (use --%s=value)\n", name, name);
					n_errors++;
					break;
				}
			}

			if      (MATCH("verbose"))                    { /* handled via -V */ }
			else if (MATCH("help"))                       { Ctrl->help = true; }
			/*-------------------------------------------------------
			 * Define input file and format (usually a datalist) */
			else if (MATCH("input"))                      { strcpy(Ctrl->read_file, val); Ctrl->I_active = true; }
			else if (MATCH("format"))                     { sscanf(val, "%d", &Ctrl->format); Ctrl->F_active = true; }
			else if (MATCH("output-directory")) {
				int n = sscanf(val, "%1023s", Ctrl->output_directory);
				if (n == 1 && strlen(Ctrl->output_directory) > 0) Ctrl->output_directory_set = true;
			}
			else if (MATCH("output-datalist")) {
				int n = sscanf(val, "%1023s", Ctrl->output_datalist);
				if (n == 1 && strlen(Ctrl->output_datalist) > 0) Ctrl->output_datalist_set = true;
			}
			else if (MATCH("platform-file")) {
				int n = sscanf(val, "%1023s", Ctrl->platform_file);
				if (n == 1 && strlen(Ctrl->platform_file) > 0) Ctrl->use_platform_file = true;
			}
			else if (MATCH("platform-target-sensor"))     { sscanf(val, "%d", &Ctrl->target_sensor); }
			else if (MATCH("output-sensor-fnv"))          { Ctrl->output_sensor_fnv = true; }
			else if (MATCH("skip-existing"))              { Ctrl->skip_existing = true; }

			/*-------------------------------------------------------
			 * Define source of navigation - could be an external file
			 * or an internal asynchronous record */
			else if (MATCH("nav-file"))                   { strcpy(Ctrl->nav_file, val); Ctrl->nav_mode = MBPREPROCESS_MERGE_FILE; pp->recalculate_bathymetry = true; }
			else if (MATCH("nav-file-format"))            { sscanf(val, "%d", &Ctrl->nav_file_format); }
			else if (MATCH("nav-async"))                  { if (sscanf(val, "%d", &Ctrl->nav_async) == 1) Ctrl->nav_mode = MBPREPROCESS_MERGE_ASYNC; pp->recalculate_bathymetry = true; }
			else if (MATCH("nav-sensor"))                 { sscanf(val, "%d", &Ctrl->nav_sensor); pp->recalculate_bathymetry = true; }

			/*-------------------------------------------------------
			 * Define source of sensordepth - could be an external file
			 * or an internal asynchronous record */
			else if (MATCH("sensordepth-file"))           { strcpy(Ctrl->sensordepth_file, val); Ctrl->sensordepth_mode = MBPREPROCESS_MERGE_FILE; pp->recalculate_bathymetry = true; }
			else if (MATCH("sensordepth-file-format"))    { sscanf(val, "%d", &Ctrl->sensordepth_file_format); }
			else if (MATCH("sensordepth-async"))          { if (sscanf(val, "%d", &Ctrl->sensordepth_async) == 1) Ctrl->sensordepth_mode = MBPREPROCESS_MERGE_ASYNC; pp->recalculate_bathymetry = true; }
			else if (MATCH("sensordepth-sensor"))         { sscanf(val, "%d", &Ctrl->sensordepth_sensor); pp->recalculate_bathymetry = true; }

			/*-------------------------------------------------------
			 * Define source of heading - could be an external file
			 * or an internal asynchronous record */
			else if (MATCH("heading-file"))               { strcpy(Ctrl->heading_file, val); Ctrl->heading_mode = MBPREPROCESS_MERGE_FILE; pp->recalculate_bathymetry = true; }
			else if (MATCH("heading-file-format"))        { sscanf(val, "%d", &Ctrl->heading_file_format); }
			else if (MATCH("heading-async"))              { if (sscanf(val, "%d", &Ctrl->heading_async) == 1) Ctrl->heading_mode = MBPREPROCESS_MERGE_ASYNC; pp->recalculate_bathymetry = true; }
			else if (MATCH("heading-sensor"))             { sscanf(val, "%d", &Ctrl->heading_sensor); pp->recalculate_bathymetry = true; }

			/*-------------------------------------------------------
			 * Define source of altitude - could be an external file
			 * or an internal asynchronous record */
			else if (MATCH("altitude-file"))              { strcpy(Ctrl->altitude_file, val); Ctrl->altitude_mode = MBPREPROCESS_MERGE_FILE; }
			else if (MATCH("altitude-file-format"))       { sscanf(val, "%d", &Ctrl->altitude_file_format); }
			else if (MATCH("altitude-async"))             { if (sscanf(val, "%d", &Ctrl->altitude_async) == 1) Ctrl->altitude_mode = MBPREPROCESS_MERGE_ASYNC; }
			else if (MATCH("altitude-sensor"))            { sscanf(val, "%d", &Ctrl->altitude_sensor); }

			/*-------------------------------------------------------
			 * Define source of attitude - could be an external file
			 * or an internal asynchronous record */
			else if (MATCH("attitude-file"))              { strcpy(Ctrl->attitude_file, val); Ctrl->attitude_mode = MBPREPROCESS_MERGE_FILE; pp->recalculate_bathymetry = true; }
			else if (MATCH("attitude-file-format"))       { sscanf(val, "%d", &Ctrl->attitude_file_format); }
			else if (MATCH("attitude-async"))             { if (sscanf(val, "%d", &Ctrl->attitude_async) == 1) Ctrl->attitude_mode = MBPREPROCESS_MERGE_ASYNC; pp->recalculate_bathymetry = true; }
			else if (MATCH("attitude-sensor"))            { sscanf(val, "%d", &Ctrl->attitude_sensor); pp->recalculate_bathymetry = true; }
			else if (MATCH("attitude-zero-heave"))        { Ctrl->zero_heave = true; }

			/*-------------------------------------------------------
			 * Define source of soundspeed - could be an external file
			 * or an internal asynchronous record */
			else if (MATCH("soundspeed-file"))            { strcpy(Ctrl->soundspeed_file, val); Ctrl->soundspeed_mode = MBPREPROCESS_MERGE_FILE; pp->modify_soundspeed = true; pp->recalculate_bathymetry = true; }
			else if (MATCH("soundspeed-file-format"))     { sscanf(val, "%d", &Ctrl->soundspeed_file_format); }
			else if (MATCH("soundspeed-async"))           { if (sscanf(val, "%d", &Ctrl->soundspeed_async) == 1) Ctrl->soundspeed_mode = MBPREPROCESS_MERGE_ASYNC; pp->modify_soundspeed = true; pp->recalculate_bathymetry = true; }
			else if (MATCH("soundspeed-sensor"))          { sscanf(val, "%d", &Ctrl->soundspeed_sensor); pp->modify_soundspeed = true; pp->recalculate_bathymetry = true; }

			/*-------------------------------------------------------
			 * Define source of time_latency - could be an external file
			 * or single value. Also define which data the time_latency model
			 * will be applied to - nav, sensordepth, heading, attitude,
			 * or all. */
			else if (MATCH("time-latency-file"))          { strcpy(Ctrl->time_latency_file, val); Ctrl->time_latency_mode = MB_SENSOR_TIME_LATENCY_MODEL; }
			else if (MATCH("time-latency-file-format"))   { sscanf(val, "%d", &Ctrl->time_latency_format); }
			else if (MATCH("time-latency-constant"))      { if (sscanf(val, "%lf", &Ctrl->time_latency_constant) == 1) Ctrl->time_latency_mode = MB_SENSOR_TIME_LATENCY_STATIC; }
			else if (MATCH("time-latency-apply-nav"))         { Ctrl->time_latency_apply |= MBPREPROCESS_TIME_LATENCY_APPLY_NAV;         pp->recalculate_bathymetry = true; }
			else if (MATCH("time-latency-apply-sensordepth")) { Ctrl->time_latency_apply |= MBPREPROCESS_TIME_LATENCY_APPLY_SENSORDEPTH; pp->recalculate_bathymetry = true; }
			else if (MATCH("time-latency-apply-heading"))     { Ctrl->time_latency_apply |= MBPREPROCESS_TIME_LATENCY_APPLY_HEADING;     pp->recalculate_bathymetry = true; }
			else if (MATCH("time-latency-apply-attitude"))    { Ctrl->time_latency_apply |= MBPREPROCESS_TIME_LATENCY_APPLY_ATTITUDE;    pp->recalculate_bathymetry = true; }
			else if (MATCH("time-latency-apply-altitude"))    { Ctrl->time_latency_apply |= MBPREPROCESS_TIME_LATENCY_APPLY_ATTITUDE;    pp->recalculate_bathymetry = true; }
			else if (MATCH("time-latency-apply-all-ancilliary")) { Ctrl->time_latency_apply = MBPREPROCESS_TIME_LATENCY_APPLY_ALL_ANCILLIARY; pp->recalculate_bathymetry = true; }
			else if (MATCH("time-latency-apply-survey"))      { Ctrl->time_latency_apply = MBPREPROCESS_TIME_LATENCY_APPLY_SURVEY;       pp->recalculate_bathymetry = true; }
			else if (MATCH("time-latency-apply-all"))         { Ctrl->time_latency_apply = MBPREPROCESS_TIME_LATENCY_APPLY_ALL;          pp->recalculate_bathymetry = true; }

			/*-------------------------------------------------------
			 * Define time domain filtering of ancillary data such as
			 * nav, sensordepth, heading, attitude, and altitude */
			else if (MATCH("filter"))                          { sscanf(val, "%lf", &Ctrl->filter_length); }
			else if (MATCH("filter-apply-nav"))                { Ctrl->filter_apply |= MBPREPROCESS_TIME_LATENCY_APPLY_NAV;         pp->recalculate_bathymetry = true; }
			else if (MATCH("filter-apply-sensordepth"))        { Ctrl->filter_apply |= MBPREPROCESS_TIME_LATENCY_APPLY_SENSORDEPTH; pp->recalculate_bathymetry = true; }
			else if (MATCH("filter-apply-heading"))            { Ctrl->filter_apply |= MBPREPROCESS_TIME_LATENCY_APPLY_HEADING;     pp->recalculate_bathymetry = true; }
			else if (MATCH("filter-apply-attitude"))           { Ctrl->filter_apply |= MBPREPROCESS_TIME_LATENCY_APPLY_ATTITUDE;    pp->recalculate_bathymetry = true; }
			else if (MATCH("filter-apply-altitude"))           { Ctrl->filter_apply |= MBPREPROCESS_TIME_LATENCY_APPLY_ATTITUDE;    pp->recalculate_bathymetry = true; }
			else if (MATCH("filter-apply-all-ancilliary"))     { Ctrl->filter_apply = MBPREPROCESS_TIME_LATENCY_APPLY_ALL_ANCILLIARY; pp->recalculate_bathymetry = true; }

			/*-------------------------------------------------------
			 * Miscellaneous commands */
			else if (MATCH("recalculate-bathymetry"))     { pp->recalculate_bathymetry = true; }
			else if (MATCH("no-change-survey"))           { pp->no_change_survey = true; }
			else if (MATCH("multibeam-sidescan-source")) {
				if (val[0] == 'S' || val[0] == 's') pp->multibeam_sidescan_source = MB_PR_SSSOURCE_SNIPPET;
				else if (val[0] == 'C' || val[0] == 'c') pp->multibeam_sidescan_source = MB_PR_SSSOURCE_CALIBRATEDSNIPPET;
				else if (val[0] == 'B' || val[0] == 'b') pp->multibeam_sidescan_source = MB_PR_SSSOURCE_WIDEBEAMBACKSCATTER;
				else if (val[0] == 'W' || val[0] == 'w') pp->multibeam_sidescan_source = MB_PR_SSSOURCE_CALIBRATEDWIDEBEAMBACKSCATTER;
			}
			else if (MATCH("sounding-amplitude-filter")) {
				if (sscanf(val, "%lf", &pp->sounding_amplitude_threshold) == 1) pp->sounding_amplitude_filter = true;
			}
			else if (MATCH("sounding-altitude-filter")) {
				if (sscanf(val, "%lf", &pp->sounding_target_altitude) == 1) pp->sounding_altitude_filter = true;
			}
			else if (MATCH("ignore-water-column"))        { pp->ignore_water_column = true; }
			else if (MATCH("head1-offsets")) {
				int n = sscanf(val, "%lf/%lf/%lf/%lf/%lf/%lf",
				    &pp->head1_offsets_x, &pp->head1_offsets_y, &pp->head1_offsets_z,
				    &pp->head1_offsets_heading, &pp->head1_offsets_roll, &pp->head1_offsets_pitch);
				if (n == 6) pp->head1_offsets = true;
			}
			else if (MATCH("head2-offsets")) {
				int n = sscanf(val, "%lf/%lf/%lf/%lf/%lf/%lf",
				    &pp->head2_offsets_x, &pp->head2_offsets_y, &pp->head2_offsets_z,
				    &pp->head2_offsets_heading, &pp->head2_offsets_roll, &pp->head2_offsets_pitch);
				if (n == 6) pp->head2_offsets = true;
			}

			/*-------------------------------------------------------
			 * Various fixes for specific data problems */
			else if (MATCH("kluge-time-jumps")) {
				if (sscanf(val, "%lf", &Ctrl->kluge_timejumps_threshold) == 1) Ctrl->kluge_timejumps = true;
			}
			else if (MATCH("kluge-fix-7k-timestamps")) {
				if (sscanf(val, "%lf", &Ctrl->kluge_fix7ktimestamps_targetoffset) == 1) {
					Ctrl->kluge_fix7ktimestamps = true;
					pp->kluge_id[pp->n_kluge] = MB_PR_KLUGE_FIX7KTIMESTAMPS;
					double *dptr = (double *)&pp->kluge_pars[pp->n_kluge * MB_PR_KLUGE_PAR_SIZE];
					*dptr = Ctrl->kluge_fix7ktimestamps_targetoffset;
					pp->n_kluge++;
				}
			}
			else if (MATCH("kluge-ancilliary-time-jumps")) {
				if (sscanf(val, "%lf", &Ctrl->kluge_timejumps_anc_threshold) == 1) Ctrl->kluge_timejumps_ancilliary = true;
			}
			else if (MATCH("kluge-mbaripressure-time-jumps")) {
				if (sscanf(val, "%lf", &Ctrl->kluge_timejumps_mba_threshold) == 1) Ctrl->kluge_timejumps_mbaripressure = true;
			}
			else if (MATCH("kluge-beam-tweak")) {
				if (sscanf(val, "%lf", &Ctrl->kluge_beamtweak_factor) == 1) {
					Ctrl->kluge_beamtweak = true;
					pp->kluge_id[pp->n_kluge] = MB_PR_KLUGE_BEAMTWEAK;
					double *dptr = (double *)&pp->kluge_pars[pp->n_kluge * MB_PR_KLUGE_PAR_SIZE];
					*dptr = Ctrl->kluge_beamtweak_factor;
					pp->n_kluge++;
					pp->recalculate_bathymetry = true;
				}
			}
			else if (MATCH("kluge-soundspeed-tweak")) {
				if (sscanf(val, "%lf", &Ctrl->kluge_soundspeedtweak_factor) == 1) {
					Ctrl->kluge_soundspeedtweak = true;
					pp->kluge_id[pp->n_kluge] = MB_PR_KLUGE_SOUNDSPEEDTWEAK;
					double *dptr = (double *)&pp->kluge_pars[pp->n_kluge * MB_PR_KLUGE_PAR_SIZE];
					*dptr = Ctrl->kluge_soundspeedtweak_factor;
					pp->n_kluge++;
					pp->recalculate_bathymetry = true;
				}
			}
			else if (MATCH("kluge-zero-attitude-correction")) {
				pp->kluge_id[pp->n_kluge++] = MB_PR_KLUGE_ZEROATTITUDECORRECTION;
				pp->recalculate_bathymetry = true;
			}
			else if (MATCH("kluge-zero-alongtrack-angles")) {
				pp->kluge_id[pp->n_kluge++] = MB_PR_KLUGE_ZEROALONGTRACKANGLES;
				pp->recalculate_bathymetry = true;
			}
			else if (MATCH("kluge-fix-wissl-timestamps")) {
				pp->kluge_id[pp->n_kluge++] = MB_PR_KLUGE_FIXWISSLTIMESTAMPS;
				pp->recalculate_bathymetry = true;
				Ctrl->kluge_fix_wissl_timestamps = true;
			}
			else if (MATCH("kluge-auv-sentry-sensordepth")) {
				pp->kluge_id[pp->n_kluge++] = MB_PR_KLUGE_AUVSENTRYSENSORDEPTH;
				Ctrl->kluge_auv_sentry_sensordepth = true;
			}
			else if (MATCH("kluge-ignore-snippets")) {
				pp->kluge_id[pp->n_kluge++] = MB_PR_KLUGE_IGNORESNIPPETS;
				Ctrl->kluge_ignore_snippets = true;
			}
			else if (MATCH("kluge-sensordepth-from-heave")) {
				pp->kluge_id[pp->n_kluge++] = MB_PR_KLUGE_SENSORDEPTHFROMHEAVE;
				Ctrl->kluge_sensordepth_from_heave = true;
			}
			else if (MATCH("kluge-early-MBARI-Mapping-AUV")) {
				pp->kluge_id[pp->n_kluge++] = MB_PR_KLUGE_EARLYMBARIMAPPINGAUV;
				Ctrl->kluge_early_mbari_mapping_auv = true;
			}
			else if (MATCH("kluge-flipsign-roll")) {
				pp->kluge_id[pp->n_kluge++] = MB_PR_KLUGE_FLIPSIGNROLL;
				Ctrl->kluge_flipsign_roll = true;
			}
			else if (MATCH("kluge-flipsign-pitch")) {
				pp->kluge_id[pp->n_kluge++] = MB_PR_KLUGE_FLIPSIGNPITCH;
				Ctrl->kluge_flipsign_pitch = true;
			}
			else if (MATCH("kluge-set-beamwidths")) {
				double kbwx, kbwy;
				if (sscanf(val, "%lf/%lf", &kbwx, &kbwy) == 2) {
					pp->kluge_id[pp->n_kluge] = MB_PR_KLUGE_SETBEAMWIDTHACROSSTRACK;
					double *dptr = (double *)&pp->kluge_pars[pp->n_kluge * MB_PR_KLUGE_PAR_SIZE];
					*dptr = kbwx;
					pp->n_kluge++;
					Ctrl->kluge_setbeamwidthacrosstrack = true;
					pp->kluge_id[pp->n_kluge] = MB_PR_KLUGE_SETBEAMWIDTHALONGTRACK;
					dptr = (double *)&pp->kluge_pars[pp->n_kluge * MB_PR_KLUGE_PAR_SIZE];
					*dptr = kbwy;
					pp->n_kluge++;
					Ctrl->kluge_setbeamwidthalongtrack = true;
				}
			}
			else if (MATCH("kluge-set-beamwidth-acrosstrack")) {
				double kbwx;
				if (sscanf(val, "%lf", &kbwx) == 1) {
					pp->kluge_id[pp->n_kluge] = MB_PR_KLUGE_SETBEAMWIDTHACROSSTRACK;
					double *dptr = (double *)&pp->kluge_pars[pp->n_kluge * MB_PR_KLUGE_PAR_SIZE];
					*dptr = kbwx;
					pp->n_kluge++;
					Ctrl->kluge_setbeamwidthacrosstrack = true;
				}
			}
			else if (MATCH("kluge-set-beamwidth-alongtrack")) {
				double kbwy;
				if (sscanf(val, "%lf", &kbwy) == 1) {
					pp->kluge_id[pp->n_kluge] = MB_PR_KLUGE_SETBEAMWIDTHALONGTRACK;
					double *dptr = (double *)&pp->kluge_pars[pp->n_kluge * MB_PR_KLUGE_PAR_SIZE];
					*dptr = kbwy;
					pp->n_kluge++;
					Ctrl->kluge_setbeamwidthalongtrack = true;
				}
			}
			else if (MATCH("kluge-ignore-duplicate-pings")) {
				pp->kluge_id[pp->n_kluge++] = MB_PR_KLUGE_IGNOREDUPLICATEPINGS;
				Ctrl->kluge_ignore_duplicate_pings = true;
			}
			else if (MATCH("kluge-xducer-depth-from-heave")) {
				pp->kluge_id[pp->n_kluge++] = MB_PR_KLUGE_XDUCER_DEPTH_FROM_HEAVE;
				Ctrl->kluge_xducer_depth_from_heave = true;
			}
			else if (MATCH("kluge-xducer-depth-from-sensordepth")) {
				pp->kluge_id[pp->n_kluge++] = MB_PR_KLUGE_XDUCER_DEPTH_FROM_SENSORDEPTH;
				Ctrl->kluge_xducer_depth_from_sensordepth = true;
			}
			else if (MATCH("kluge-xducer-depth-from-heave-and-sensordepth")) {
				pp->kluge_id[pp->n_kluge++] = MB_PR_KLUGE_XDUCER_DEPTH_FROM_HEAVEANDSENSORDEPTH;
				Ctrl->kluge_xducer_depth_from_heaveandsensordepth = true;
			}
			else if (MATCH("kluge-rangescale")) {
				if (sscanf(val, "%lf", &Ctrl->kluge_rangescale_factor) == 1) {
					pp->kluge_id[pp->n_kluge] = MB_PR_KLUGE_RANGESCALE;
					double *dptr = (double *)&pp->kluge_pars[pp->n_kluge * MB_PR_KLUGE_PAR_SIZE];
					*dptr = Ctrl->kluge_rangescale_factor;
					pp->n_kluge++;
					Ctrl->kluge_rangescale = true;
				}
			}
			else if (MATCH("kluge-fix-wissl2-ranges")) {
				pp->kluge_id[pp->n_kluge++] = MB_PR_KLUGE_FIXWISSL2RANGES;
				Ctrl->kluge_fix_wissl2_ranges = true;
			}
			else {
				GMT_Report(API, GMT_MSG_NORMAL, "Unknown option --%s\n", name);
				n_errors++;
			}
			break;
		}
		case 'I':
			strcpy(Ctrl->read_file, opt->arg);
			Ctrl->I_active = true;
			break;
		case 'F':
			if (sscanf(opt->arg, "%d", &Ctrl->format) == 1) Ctrl->F_active = true;
			break;
		default:
			n_errors += gmt_default_error(GMT, opt->option);
			break;
		}
	}

	return n_errors ? GMT_PARSE_ERROR : GMT_OK;
}

#define bailout(code)   { gmt_M_free_options(mode); return code; }
#define Return(code)    { Free_mbpreprocess_Ctrl(GMT, Ctrl); gmt_end_module(GMT, GMT_cpy); bailout(code); }
#define FAIL(code)      do { error = (code); goto cleanup; } while (0)

/*--------------------------------------------------------------------*/
int GMT_mbpreprocess(void *V_API, int mode, void *args) {
	struct MBPREPROCESS_CTRL *Ctrl = NULL;
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
	if ((GMT = gmt_init_module(API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout(API->error);
#else
	GMT = gmt_begin_module(API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, &GMT_cpy);
#endif
	if (GMT_Parse_Common(API, THIS_MODULE_OPTIONS, options)) Return(API->error);

	Ctrl = New_mbpreprocess_Ctrl(GMT);
	{ int perr = parse(GMT, Ctrl, options); if (perr) Return(perr); }

	int verbose = GMT->common.V.active;

	int format = Ctrl->F_active ? Ctrl->format : 0;
	int pings;
	int lonflip;
	double bounds[4];
	int btime_i[7];
	int etime_i[7];
	double speedmin;
	double timegap;
	int status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);
	int error = MB_ERROR_NO_ERROR;

	bool use_platform_file = Ctrl->use_platform_file;
	int  target_sensor     = Ctrl->target_sensor;
	bool output_sensor_fnv = Ctrl->output_sensor_fnv;
	bool skip_existing     = Ctrl->skip_existing;

	double kluge_timejumps_threshold = Ctrl->kluge_timejumps_threshold;
	bool   kluge_timejumps = Ctrl->kluge_timejumps;
	bool   kluge_fix7ktimestamps = Ctrl->kluge_fix7ktimestamps;
	double kluge_fix7ktimestamps_targetoffset = Ctrl->kluge_fix7ktimestamps_targetoffset;
	double kluge_timejumps_anc_threshold = Ctrl->kluge_timejumps_anc_threshold;
	bool   kluge_timejumps_ancilliary = Ctrl->kluge_timejumps_ancilliary;
	double kluge_timejumps_mba_threshold = Ctrl->kluge_timejumps_mba_threshold;
	bool   kluge_timejumps_mbaripressure = Ctrl->kluge_timejumps_mbaripressure;
	double kluge_beamtweak_factor = Ctrl->kluge_beamtweak_factor;
	bool   kluge_beamtweak = Ctrl->kluge_beamtweak;
	double kluge_soundspeedtweak_factor = Ctrl->kluge_soundspeedtweak_factor;
	bool   kluge_soundspeedtweak = Ctrl->kluge_soundspeedtweak;
	bool   kluge_fix_wissl_timestamps = Ctrl->kluge_fix_wissl_timestamps;
	bool   kluge_auv_sentry_sensordepth = Ctrl->kluge_auv_sentry_sensordepth;
	bool   kluge_ignore_snippets = Ctrl->kluge_ignore_snippets;
	bool   kluge_sensordepth_from_heave = Ctrl->kluge_sensordepth_from_heave;
	bool   kluge_early_mbari_mapping_auv = Ctrl->kluge_early_mbari_mapping_auv;
	bool   kluge_flipsign_roll = Ctrl->kluge_flipsign_roll;
	bool   kluge_flipsign_pitch = Ctrl->kluge_flipsign_pitch;
	bool   kluge_setbeamwidthacrosstrack = Ctrl->kluge_setbeamwidthacrosstrack;
	bool   kluge_setbeamwidthalongtrack = Ctrl->kluge_setbeamwidthalongtrack;
	bool   kluge_ignore_duplicate_pings = Ctrl->kluge_ignore_duplicate_pings;
	bool   kluge_xducer_depth_from_heave = Ctrl->kluge_xducer_depth_from_heave;
	bool   kluge_xducer_depth_from_sensordepth = Ctrl->kluge_xducer_depth_from_sensordepth;
	bool   kluge_xducer_depth_from_heaveandsensordepth = Ctrl->kluge_xducer_depth_from_heaveandsensordepth;
	bool   kluge_rangescale = Ctrl->kluge_rangescale;
	double kluge_rangescale_factor = Ctrl->kluge_rangescale_factor;
	bool   kluge_fix_wissl2_ranges = Ctrl->kluge_fix_wissl2_ranges;

	mb_path nav_file;             strcpy(nav_file, Ctrl->nav_file);
	mb_path sensordepth_file;     strcpy(sensordepth_file, Ctrl->sensordepth_file);
	mb_path heading_file;         strcpy(heading_file, Ctrl->heading_file);
	mb_path altitude_file;        strcpy(altitude_file, Ctrl->altitude_file);
	mb_path attitude_file;        strcpy(attitude_file, Ctrl->attitude_file);
	mb_path soundspeed_file;      strcpy(soundspeed_file, Ctrl->soundspeed_file);
	mb_path time_latency_file;    strcpy(time_latency_file, Ctrl->time_latency_file);
	mb_path platform_file;        strcpy(platform_file, Ctrl->platform_file);
	mb_path read_file;            strcpy(read_file, Ctrl->read_file);
	bool output_directory_set = Ctrl->output_directory_set;
	mb_path output_directory;     strcpy(output_directory, Ctrl->output_directory);
	bool output_datalist_set = Ctrl->output_datalist_set;
	char output_datalist[MB_PATH_MAXLINE+100];
	strcpy(output_datalist, Ctrl->output_datalist);
	struct mb_preprocess_struct preprocess_pars = Ctrl->preprocess_pars;

	merge_t nav_mode             = Ctrl->nav_mode;
	int     nav_file_format      = Ctrl->nav_file_format;
	int     nav_async            = Ctrl->nav_async;
	int     nav_sensor           = Ctrl->nav_sensor;
	merge_t sensordepth_mode     = Ctrl->sensordepth_mode;
	int     sensordepth_file_format = Ctrl->sensordepth_file_format;
	int     sensordepth_async    = Ctrl->sensordepth_async;
	int     sensordepth_sensor   = Ctrl->sensordepth_sensor;
	merge_t heading_mode         = Ctrl->heading_mode;
	int     heading_file_format  = Ctrl->heading_file_format;
	int     heading_async        = Ctrl->heading_async;
	int     heading_sensor       = Ctrl->heading_sensor;
	merge_t altitude_mode        = Ctrl->altitude_mode;
	int     altitude_file_format = Ctrl->altitude_file_format;
	int     altitude_async       = Ctrl->altitude_async;
	int     altitude_sensor      = Ctrl->altitude_sensor;
	merge_t attitude_mode        = Ctrl->attitude_mode;
	int     attitude_file_format = Ctrl->attitude_file_format;
	int     attitude_async       = Ctrl->attitude_async;
	int     attitude_sensor      = Ctrl->attitude_sensor;
	bool    zero_heave           = Ctrl->zero_heave;
	merge_t soundspeed_mode      = Ctrl->soundspeed_mode;
	int     soundspeed_file_format = Ctrl->soundspeed_file_format;
	int     soundspeed_async     = Ctrl->soundspeed_async;
	int     soundspeed_sensor    = Ctrl->soundspeed_sensor;
	int     time_latency_mode    = Ctrl->time_latency_mode;
	int     time_latency_format  = Ctrl->time_latency_format;
	double  time_latency_constant = Ctrl->time_latency_constant;
	mb_u_char time_latency_apply = Ctrl->time_latency_apply;
	double  filter_length        = Ctrl->filter_length;
	mb_u_char filter_apply       = Ctrl->filter_apply;

	if (verbose == 1 || Ctrl->help) {
		fprintf(stderr, "\nProgram %s\n", program_name);
		fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
	}
	if (Ctrl->help) {
		GMT_Message(API, GMT_TIME_NONE, "\n%s\n", help_message);
		GMT_Message(API, GMT_TIME_NONE, "\nusage: %s\n", usage_message);
		Return(GMT_NOERROR);
	}

	/* if no affected data have been specified apply time_latency to all */
	if (time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE && time_latency_apply == MBPREPROCESS_TIME_LATENCY_APPLY_NONE)
		time_latency_apply = MBPREPROCESS_TIME_LATENCY_APPLY_ALL_ANCILLIARY;

	/* if no affected data have been specified apply filtering to all ancillary data */
	if (filter_length > 0.0 && filter_apply == MBPREPROCESS_TIME_LATENCY_APPLY_NONE)
		filter_apply = MBPREPROCESS_TIME_LATENCY_APPLY_ALL_ANCILLIARY;

	/* get read format if required */
	{
		mb_path fileroot;
		int testformat = 0;
		int formaterror = MB_ERROR_NO_ERROR;
		mb_get_format(verbose, read_file, fileroot, &format, &formaterror);
		if (format == 0) format = testformat;
		if (format < 0 && strcmp(read_file, output_datalist) == 0)
			snprintf(output_datalist, sizeof(output_datalist), "%sr.mb-1", fileroot);
	}

	/* determine whether to read one file or a list of files */
	const bool read_datalist = format < 0;

	if (verbose >= 2) {
		GMT_Report(API, GMT_MSG_INFORMATION, "\ndbg2  Program <%s>\n", program_name);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2  MB-system Version %s\n", MB_VERSION);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2  Default MB-System Parameters:\n");
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       verbose:                      %d\n", verbose);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       format:                       %d\n", format);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       pings:                        %d\n", pings);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       lonflip:                      %d\n", lonflip);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       bounds[0]:                    %f\n", bounds[0]);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       bounds[1]:                    %f\n", bounds[1]);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       bounds[2]:                    %f\n", bounds[2]);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       bounds[3]:                    %f\n", bounds[3]);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       btime_i[0]:                   %d\n", btime_i[0]);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       btime_i[1]:                   %d\n", btime_i[1]);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       btime_i[2]:                   %d\n", btime_i[2]);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       btime_i[3]:                   %d\n", btime_i[3]);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       btime_i[4]:                   %d\n", btime_i[4]);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       btime_i[5]:                   %d\n", btime_i[5]);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       btime_i[6]:                   %d\n", btime_i[6]);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       etime_i[0]:                   %d\n", etime_i[0]);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       etime_i[1]:                   %d\n", etime_i[1]);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       etime_i[2]:                   %d\n", etime_i[2]);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       etime_i[3]:                   %d\n", etime_i[3]);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       etime_i[4]:                   %d\n", etime_i[4]);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       etime_i[5]:                   %d\n", etime_i[5]);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       etime_i[6]:                   %d\n", etime_i[6]);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       speedmin:                     %f\n", speedmin);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       timegap:                      %f\n", timegap);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2  Input survey data to be preprocessed:\n");
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       read_file:                    %s\n", read_file);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       format:                       %d\n", format);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2  Output directory:\n");
		if (output_directory_set)
			GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       output_directory:             %s\n", output_directory);
		else
			GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       output_directory:             not specified, use working directory\n");
		if (output_datalist_set)
			GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       output_datalist:             %s\n", output_datalist);
		else
			GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       output_datalist:             not specified, use default: %s\n", output_datalist);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2  Source of platform model:\n");
		if (use_platform_file)
			GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       platform_file:                %s\n", platform_file);
		else
			GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       platform_file:              not specified\n");
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       target_sensor:                %d\n", target_sensor);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2  Source of navigation data:\n");
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       nav_mode:                     %d\n", nav_mode);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       nav_file:                     %s\n", nav_file);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       nav_file_format:              %d\n", nav_file_format);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       nav_async:                    %d\n", nav_async);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       nav_sensor:                   %d\n", nav_sensor);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2  Source of sensor depth data:\n");
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       sensordepth_mode:             %d\n", sensordepth_mode);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       sensordepth_file:             %s\n", sensordepth_file);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       sensordepth_file_format:      %d\n", sensordepth_file_format);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       sensordepth_async:            %d\n", sensordepth_async);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       sensordepth_sensor:           %d\n", sensordepth_sensor);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2  Source of heading data:\n");
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       heading_mode:                 %d\n", heading_mode);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       heading_file:                 %s\n", heading_file);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       heading_file_format:          %d\n", heading_file_format);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       heading_async:                %d\n", heading_async);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       heading_sensor:               %d\n", heading_sensor);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2  Source of altitude data:\n");
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       altitude_mode:                %d\n", altitude_mode);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       altitude_file:                %s\n", altitude_file);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       altitude_file_format:         %d\n", altitude_file_format);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       altitude_async:               %d\n", altitude_async);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       altitude_sensor:              %d\n", altitude_sensor);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2  Source of attitude data:\n");
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       attitude_mode:                %d\n", attitude_mode);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       attitude_file:                %s\n", attitude_file);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       attitude_file_format:         %d\n", attitude_file_format);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       attitude_async:               %d\n", attitude_async);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       attitude_sensor:              %d\n", attitude_sensor);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       attitude_zero_heave:          %d\n", zero_heave);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2  Source of soundspeed data:\n");
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       soundspeed_mode:              %d\n", soundspeed_mode);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       soundspeed_file:              %s\n", soundspeed_file);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       soundspeed_file_format:       %d\n", soundspeed_file_format);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       soundspeed_async:             %d\n", soundspeed_async);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       soundspeed_sensor:            %d\n", soundspeed_sensor);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2  Time latency correction:\n");
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       time_latency_mode:            %d\n", time_latency_mode);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       time_latency_constant:        %f\n", time_latency_constant);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       time_latency_file:            %s\n", time_latency_file);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       time_latency_format:          %d\n", time_latency_format);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       time_latency_apply:           %x\n", time_latency_apply);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2  Time domain filtering:\n");
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       filter_length:                %f\n", filter_length);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       filter_apply:                 %x\n", filter_apply);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2  Miscellaneous controls:\n");
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       no_change_survey:             %d\n", preprocess_pars.no_change_survey);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       multibeam_sidescan_source:    %d\n", preprocess_pars.multibeam_sidescan_source);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       recalculate_bathymetry:       %d\n", preprocess_pars.recalculate_bathymetry);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       sounding_amplitude_filter:    %d\n", preprocess_pars.sounding_amplitude_filter);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       sounding_amplitude_threshold: %f\n", preprocess_pars.sounding_amplitude_threshold);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       sounding_altitude_filter:     %d\n", preprocess_pars.sounding_altitude_filter);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       sounding_target_altitude:     %f\n", preprocess_pars.sounding_target_altitude);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       ignore_water_column:          %d\n", preprocess_pars.ignore_water_column);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       head1_offsets:                %d\n", preprocess_pars.head1_offsets);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       head1_offsets_x:              %f\n", preprocess_pars.head1_offsets_x);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       head1_offsets_y:              %f\n", preprocess_pars.head1_offsets_y);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       head1_offsets_z:              %f\n", preprocess_pars.head1_offsets_z);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       head1_offsets_heading:        %f\n", preprocess_pars.head1_offsets_heading);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       head1_offsets_roll:           %f\n", preprocess_pars.head1_offsets_roll);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       head1_offsets_pitch:          %f\n", preprocess_pars.head1_offsets_pitch);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       head2_offsets:                %d\n", preprocess_pars.head2_offsets);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       head2_offsets_x:              %f\n", preprocess_pars.head2_offsets_x);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       head2_offsets_y:              %f\n", preprocess_pars.head2_offsets_y);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       head2_offsets_z:              %f\n", preprocess_pars.head2_offsets_z);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       head2_offsets_heading:        %f\n", preprocess_pars.head2_offsets_heading);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       head2_offsets_roll:           %f\n", preprocess_pars.head2_offsets_roll);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       head2_offsets_pitch:          %f\n", preprocess_pars.head2_offsets_pitch);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2  Various data fixes (kluges):\n");
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_timejumps:                     %d\n", kluge_timejumps);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_timejumps_threshold:           %f\n", kluge_timejumps_threshold);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_timejumps_ancilliary:          %d\n", kluge_timejumps_ancilliary);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_fix7ktimestamps:               %d\n", kluge_fix7ktimestamps);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_fix7ktimestamps_targetoffset:  %f\n", kluge_fix7ktimestamps_targetoffset);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_timejumps_anc_threshold:       %f\n", kluge_timejumps_anc_threshold);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_timejumps_mbaripressure:       %d\n", kluge_timejumps_mbaripressure);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_timejumps_mba_threshold:       %f\n", kluge_timejumps_mba_threshold);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_beamtweak:                     %d\n", kluge_beamtweak);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_beamtweak_factor:              %f\n", kluge_beamtweak_factor);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_soundspeedtweak:               %d\n", kluge_soundspeedtweak);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_soundspeedtweak_factor:        %f\n", kluge_soundspeedtweak_factor);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_fix_wissl_timestamps:          %d\n", kluge_fix_wissl_timestamps);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_auv_sentry_sensordepth:        %d\n", kluge_auv_sentry_sensordepth);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_ignore_snippets:               %d\n", kluge_ignore_snippets);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_sensordepth_from_heave         %d\n", kluge_sensordepth_from_heave);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_early_mbari_mapping_auv        %d\n", kluge_early_mbari_mapping_auv);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_flipsign_roll                  %d\n", kluge_flipsign_roll);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_flipsign_pitch                 %d\n", kluge_flipsign_pitch);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_setbeamwidthacrosstrack        %d\n", kluge_setbeamwidthacrosstrack);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_setbeamwidthalongtrack         %d\n", kluge_setbeamwidthalongtrack);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_ignore_duplicate_pings         %d\n", kluge_ignore_duplicate_pings);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_xducer_depth_from_heave        %d\n", kluge_xducer_depth_from_heave);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_xducer_depth_from_sensordepth  %d\n", kluge_xducer_depth_from_sensordepth);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_xducer_depth_from_heaveandsensordepth  %d\n", kluge_xducer_depth_from_heaveandsensordepth);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_rangescale                     %d\n", kluge_rangescale);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_rangescale_factor              %f\n", kluge_rangescale_factor);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kluge_fix_wissl2_ranges              %d\n", kluge_fix_wissl2_ranges);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2  Additional output:\n");
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       output_sensor_fnv:            %d\n", output_sensor_fnv);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2  Skip existing output files:\n");
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       skip_existing:                %d\n", skip_existing);
	}
	else if (verbose > 0) {
		GMT_Report(API, GMT_MSG_INFORMATION, "\nProgram <  %s>\n", program_name);
		GMT_Report(API, GMT_MSG_INFORMATION, "MB-system Version   %s\n", MB_VERSION);
		GMT_Report(API, GMT_MSG_INFORMATION, "Input survey data to be preprocessed:\n");
		GMT_Report(API, GMT_MSG_INFORMATION, "     read_file:                    %s\n", read_file);
		GMT_Report(API, GMT_MSG_INFORMATION, "     format:                       %d\n", format);
		GMT_Report(API, GMT_MSG_INFORMATION, "Source of platform model:\n");
		if (use_platform_file)
			GMT_Report(API, GMT_MSG_INFORMATION, "     platform_file:                %s\n", platform_file);
		else
			GMT_Report(API, GMT_MSG_INFORMATION, "     platform_file:              not specified\n");
		GMT_Report(API, GMT_MSG_INFORMATION, "     target_sensor:                %d\n", target_sensor);
		GMT_Report(API, GMT_MSG_INFORMATION, "Time latency correction:\n");
		GMT_Report(API, GMT_MSG_INFORMATION, "     time_latency_mode:            %d\n", time_latency_mode);
		GMT_Report(API, GMT_MSG_INFORMATION, "     time_latency_constant:        %f\n", time_latency_constant);
		GMT_Report(API, GMT_MSG_INFORMATION, "     time_latency_file:            %s\n", time_latency_file);
		GMT_Report(API, GMT_MSG_INFORMATION, "     time_latency_apply:           %x\n", time_latency_apply);
		GMT_Report(API, GMT_MSG_INFORMATION, "Time domain filtering:\n");
		GMT_Report(API, GMT_MSG_INFORMATION, "     filter_length:                %f\n", filter_length);
		GMT_Report(API, GMT_MSG_INFORMATION, "     filter_apply:                 %x\n", filter_apply);
		GMT_Report(API, GMT_MSG_INFORMATION, "Skip existing output files:\n");
		GMT_Report(API, GMT_MSG_INFORMATION, "     skip_existing:                %d\n", skip_existing);
	}

	/* platform definition file */
	struct mb_platform_struct *platform = NULL;
	struct mb_sensor_struct *sensor_position = NULL;
	struct mb_sensor_struct *sensor_depth = NULL;
	struct mb_sensor_struct *sensor_heading = NULL;
	struct mb_sensor_struct *sensor_rollpitch = NULL;
	struct mb_sensor_struct *sensor_target = NULL;

	/* asynchronous arrays */
	int n_nav = 0, n_nav_alloc = 0;
	double *nav_time_d = NULL, *nav_navlon = NULL, *nav_navlat = NULL, *nav_speed = NULL;
	int n_sensordepth = 0, n_sensordepth_alloc = 0;
	double *sensordepth_time_d = NULL, *sensordepth_sensordepth = NULL;
	int n_heading = 0, n_heading_alloc = 0;
	double *heading_time_d = NULL, *heading_heading = NULL;
	int n_altitude = 0, n_altitude_alloc = 0;
	double *altitude_time_d = NULL, *altitude_altitude = NULL;
	int n_attitude = 0, n_attitude_alloc = 0;
	double *attitude_time_d = NULL, *attitude_roll = NULL, *attitude_pitch = NULL, *attitude_heave = NULL;
	int n_soundspeed = 0, n_soundspeed_alloc = 0;
	double *soundspeed_time_d = NULL, *soundspeed_soundspeed = NULL;
	int time_latency_num = 0, time_latency_alloc = 0;
	double *time_latency_time_d = NULL, *time_latency_time_latency = NULL;

	int num_indextable = 0, num_indextable_alloc = 0;
	struct mb_io_indextable_struct *indextable = NULL;
	int i_num_indextable = 0;
	struct mb_io_indextable_struct *i_indextable = NULL;

	double kluge_first_time_d = 0.0;
	double kluge_last_time_d = 0.0;
	double kluge_last_raw_time_d = 0.0;
	double dtime_d_expect = 0.0;
	double dtime_d = 0.0;
	bool kluge_fix_wissl_timestamps_setup1 = false;
	bool kluge_fix_wissl_timestamps_setup2 = false;

	void *datalist = NULL;
	double file_weight;
	int iformat;
	int oformat;
	double btime_d, etime_d;
	mb_path ifile = "";
	mb_path dfile = "";
	char ofile[MB_PATH_MAXLINE+10] = "";
	mb_path fileroot = "";
	int beams_bath, beams_amp, pixels_ss;
	int obeams_bath, obeams_amp, opixels_ss;

	void *imbio_ptr = NULL, *ombio_ptr = NULL, *istore_ptr = NULL;
	int kind;
	int time_i[7];
	double time_d;
	double navlon, navlat, speed, heading, distance, altitude, sensordepth, draft, roll, pitch, heave;
	char *beamflag = NULL;
	double *bath = NULL, *bathacrosstrack = NULL, *bathalongtrack = NULL, *amp = NULL;
	double *ss = NULL, *ssacrosstrack = NULL, *ssalongtrack = NULL;
	char comment[MB_COMMENT_MAXLINE];
	double navlon_org, navlat_org, speed_org, heading_org, altitude_org, sensordepth_org, draft_org;
	double roll_org, roll_delta;
	double pitch_org, pitch_delta;
	double heave_org;
	double depth_offset_change;

	int nanavmax = MB_NAV_MAX;
	int nanav;
	int atime_i[7 * MB_NAV_MAX];
	double atime_d[MB_NAV_MAX];
	double alon[MB_NAV_MAX], alat[MB_NAV_MAX], aspeed[MB_NAV_MAX], aheading[MB_NAV_MAX];
	double asensordepth[MB_NAV_MAX], aroll[MB_NAV_MAX], apitch[MB_NAV_MAX], aheave[MB_NAV_MAX];

	int n_rt_data = 0, n_rt_comment = 0;
	int n_rt_nav = 0, n_rt_nav1 = 0, n_rt_nav2 = 0, n_rt_nav3 = 0;
	int n_rt_att = 0, n_rt_att1 = 0, n_rt_att2 = 0, n_rt_att3 = 0;
	int n_rt_files = 0, n_rt_dup_timestamp = 0;

	int n_wt_data = 0, n_wt_comment = 0;
	int n_wt_nav = 0, n_wt_nav1 = 0, n_wt_nav2 = 0, n_wt_nav3 = 0;
	int n_wt_att = 0, n_wt_att1 = 0, n_wt_att2 = 0, n_wt_att3 = 0;
	int n_wt_files = 0;

	char afile[MB_PATH_MAXLINE+100] = "";
	FILE *afp = NULL;
	struct stat file_status;
	double start_time_d, end_time_d;
	int istart, iend;
	int input_size, input_modtime, output_size, output_modtime;
	int isensor, ioffset;
	int testformat;
	int interp_error = MB_ERROR_NO_ERROR;
	int jnav = 0, jsensordepth = 0, jheading = 0, jaltitude = 0, jattitude = 0;
	char buffer[16] = "";

	if (use_platform_file) {
		status = mb_platform_read(verbose, platform_file, (void **)&platform, &error);
		if (status == MB_FAILURE) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open and parse platform file: %s\n", platform_file);
			GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", program_name);
			FAIL(MB_ERROR_OPEN_FAIL);
		}
		if (nav_sensor >= 0)         platform->source_position = nav_sensor;
		if (sensordepth_sensor >= 0) platform->source_depth = sensordepth_sensor;
		if (heading_sensor >= 0)     platform->source_heading = heading_sensor;
		if (attitude_sensor >= 0) {
			platform->source_rollpitch = attitude_sensor;
			platform->source_heave = attitude_sensor;
		}
		if (platform->source_position >= 0)  sensor_position  = &(platform->sensors[platform->source_position]);
		if (platform->source_depth >= 0)     sensor_depth     = &(platform->sensors[platform->source_depth]);
		if (platform->source_heading >= 0)   sensor_heading   = &(platform->sensors[platform->source_heading]);
		if (platform->source_rollpitch >= 0) sensor_rollpitch = &(platform->sensors[platform->source_rollpitch]);
		if (target_sensor < 0)               target_sensor    = platform->source_bathymetry;
		if (target_sensor >= 0)              sensor_target    = &(platform->sensors[target_sensor]);
	}

	/* start by loading time latency model if required */
	if (time_latency_mode == MB_SENSOR_TIME_LATENCY_MODEL) {
		status = mb_loadtimeshiftdata(verbose, time_latency_file, time_latency_format, &time_latency_num, &time_latency_alloc,
		                              &time_latency_time_d, &time_latency_time_latency, &error);
		if (status == MB_FAILURE) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open and parse time latency file: %s\n", time_latency_file);
			GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", program_name);
			FAIL(MB_ERROR_OPEN_FAIL);
		}
		else if (time_latency_num < 1) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nNo time latency values read from: %s\n", time_latency_file);
			GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", program_name);
			FAIL(MB_ERROR_OPEN_FAIL);
		}
		else if (verbose > 0)
			GMT_Report(API, GMT_MSG_INFORMATION, "%d time_latency records loaded from file %s\n", time_latency_num, time_latency_file);
	}

	/* import specified ancillary data */
	if (nav_mode == MBPREPROCESS_MERGE_FILE) {
		status = mb_loadnavdata(verbose, nav_file, nav_file_format, lonflip, &n_nav, &n_nav_alloc,
		                        &nav_time_d, &nav_navlon, &nav_navlat, &nav_speed, &error);
		if (status == MB_FAILURE) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open and parse nav file: %s\n", nav_file);
			GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", program_name);
			FAIL(MB_ERROR_OPEN_FAIL);
		}
		else if (n_nav < 1) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nNo nav values read from: %s\n", nav_file);
			FAIL(MB_ERROR_OPEN_FAIL);
		}
		else if (verbose > 0)
			GMT_Report(API, GMT_MSG_INFORMATION, "%d navigation records loaded from file %s\n", n_nav, nav_file);
	}
	if (sensordepth_mode == MBPREPROCESS_MERGE_FILE) {
		status = mb_loadsensordepthdata(verbose, sensordepth_file, sensordepth_file_format, &n_sensordepth, &n_sensordepth_alloc,
		                                &sensordepth_time_d, &sensordepth_sensordepth, &error);
		if (status == MB_FAILURE) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open and parse sensordepth file: %s\n", sensordepth_file);
			FAIL(MB_ERROR_OPEN_FAIL);
		}
		else if (n_sensordepth < 1) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nNo sensordepth values read from: %s\n", sensordepth_file);
			FAIL(MB_ERROR_OPEN_FAIL);
		}
		else if (verbose > 0)
			GMT_Report(API, GMT_MSG_INFORMATION, "%d sensordepth records loaded from file %s\n", n_sensordepth, sensordepth_file);
	}
	if (heading_mode == MBPREPROCESS_MERGE_FILE) {
		status = mb_loadheadingdata(verbose, heading_file, heading_file_format, &n_heading, &n_heading_alloc,
		                            &heading_time_d, &heading_heading, &error);
		if (status == MB_FAILURE) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open and parse heading file: %s\n", heading_file);
			FAIL(MB_ERROR_OPEN_FAIL);
		}
		else if (n_heading < 1) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nNo heading values read from: %s\n", heading_file);
			FAIL(MB_ERROR_OPEN_FAIL);
		}
		else if (verbose > 0)
			GMT_Report(API, GMT_MSG_INFORMATION, "%d heading records loaded from file %s\n", n_heading, heading_file);
	}
	if (altitude_mode == MBPREPROCESS_MERGE_FILE) {
		status = mb_loadaltitudedata(verbose, altitude_file, altitude_file_format, &n_altitude, &n_altitude_alloc,
		                             &altitude_time_d, &altitude_altitude, &error);
		if (status == MB_FAILURE) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open and parse altitude file: %s\n", altitude_file);
			FAIL(MB_ERROR_OPEN_FAIL);
		}
		else if (n_altitude < 1) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nNo altitude values read from: %s\n", altitude_file);
			FAIL(MB_ERROR_OPEN_FAIL);
		}
		else if (verbose > 0)
			GMT_Report(API, GMT_MSG_INFORMATION, "%d altitude records loaded from file %s\n", n_altitude, altitude_file);
	}
	if (attitude_mode == MBPREPROCESS_MERGE_FILE) {
		status = mb_loadattitudedata(verbose, attitude_file, attitude_file_format, &n_attitude, &n_attitude_alloc,
		                             &attitude_time_d, &attitude_roll, &attitude_pitch, &attitude_heave, &error);
		if (status == MB_FAILURE) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open and parse attitude file: %s\n", attitude_file);
			FAIL(MB_ERROR_OPEN_FAIL);
		}
		else if (n_attitude < 1) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nNo attitude values read from: %s\n", attitude_file);
			FAIL(MB_ERROR_OPEN_FAIL);
		}
		else if (verbose > 0)
			GMT_Report(API, GMT_MSG_INFORMATION, "%d attitude records loaded from file %s\n", n_attitude, attitude_file);
	}
	if (soundspeed_mode == MBPREPROCESS_MERGE_FILE) {
		status = mb_loadsoundspeeddata(verbose, soundspeed_file, soundspeed_file_format, &n_soundspeed, &n_soundspeed_alloc,
		                               &soundspeed_time_d, &soundspeed_soundspeed, &error);
		if (status == MB_FAILURE) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open and parse soundspeed file: %s\n", soundspeed_file);
			FAIL(MB_ERROR_OPEN_FAIL);
		}
		else if (n_soundspeed < 1) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nNo soundspeed values read from: %s\n", soundspeed_file);
			FAIL(MB_ERROR_OPEN_FAIL);
		}
		else if (verbose > 0)
			GMT_Report(API, GMT_MSG_INFORMATION, "%d soundspeed records loaded from file %s\n", n_soundspeed, soundspeed_file);
	}

	/* open file list */
	bool read_data = false;
	if (read_datalist) {
		const int look_processed = MB_DATALIST_LOOK_UNSET;
		if (mb_datalist_open(verbose, &datalist, read_file, look_processed, &error) != MB_SUCCESS) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open data list file: %s\n", read_file);
			FAIL(MB_ERROR_OPEN_FAIL);
		}
		read_data = mb_datalist_read(verbose, datalist, ifile, dfile, &iformat, &file_weight, &error) == MB_SUCCESS;
	} else {
		strcpy(ifile, read_file);
		iformat = format;
		read_data = true;
	}

	while (read_data) {
		if (iformat == MBF_EMOLDRAW || iformat == MBF_EM12IFRM || iformat == MBF_EM12DARW ||
		    iformat == MBF_EM300RAW || iformat == MBF_EM300MBA)
			oformat = MBF_EM300MBA;
		else if (iformat == MBF_EM710RAW || iformat == MBF_EM710MBA)
			oformat = MBF_EM710MBA;
		else if (iformat == MBF_IMAGE83P)
			oformat = MBF_IMAGEMBA;
		else if (iformat == MBF_3DWISSLR)
			oformat = MBF_3DWISSLP;
		else if (iformat == MBF_OICGEODA)
			oformat = MBF_OICMBARI;
		else if (iformat == MBF_SB2100RW)
			oformat = MBF_SB2100B2;
		else
			oformat = iformat;

		status = mb_get_format(verbose, ifile, fileroot, &testformat, &error);
		snprintf(ofile, sizeof(ofile), "%s.mb%d", fileroot, oformat);
		if (strcmp(ifile, ofile) == 0)
			snprintf(ofile, sizeof(ofile), "%sr.mb%d", fileroot, oformat);

		if (output_directory_set) {
			char buf[MB_PATH_MAXLINE] = "";
			strcpy(buf, output_directory);
			if (buf[strlen(output_directory) - 1] != '/') strcat(buf, "/");
			char *filenameptr;
			if (strrchr(ofile, '/') != NULL) filenameptr = strrchr(ofile, '/') + 1;
			else filenameptr = ofile;
			strcat(buf, filenameptr);
			strcpy(ofile, buf);
		}

		bool proceed = true;
		if (skip_existing) {
			if (stat(ifile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
				input_modtime = file_status.st_mtime;
				input_size = file_status.st_size;
			} else { input_modtime = 0; input_size = 0; }
			if (stat(ofile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
				output_modtime = file_status.st_mtime;
				output_size = file_status.st_size;
			} else { output_modtime = 0; output_size = 0; }
			if (output_modtime > input_modtime && output_size > input_size) proceed = false;
		}

		if (!proceed) {
			if (verbose > 0)
				GMT_Report(API, GMT_MSG_INFORMATION, "\nPass 1: Skipping input file:  %s %d\n", ifile, iformat);
		} else {
			if (nav_mode == MBPREPROCESS_MERGE_OFF) {
				if (iformat == MBF_EMOLDRAW || iformat == MBF_EM300RAW || iformat == MBF_EM710RAW) { nav_mode = MBPREPROCESS_MERGE_ASYNC; nav_async = MB_DATA_NAV; }
				else if (iformat == MBF_KEMKMALL) { nav_mode = MBPREPROCESS_MERGE_ASYNC; nav_async = MB_DATA_NAV1; }
				else if (iformat == MBF_RESON7KR) { nav_mode = MBPREPROCESS_MERGE_ASYNC; nav_async = MB_DATA_NAV1; }
				else if (iformat == MBF_RESON7K3) { nav_mode = MBPREPROCESS_MERGE_ASYNC; nav_async = MB_DATA_NAV; }
			}
			if (sensordepth_mode == MBPREPROCESS_MERGE_OFF) {
				if (iformat == MBF_EMOLDRAW || iformat == MBF_EM300RAW || iformat == MBF_EM710RAW) { sensordepth_mode = MBPREPROCESS_MERGE_ASYNC; sensordepth_async = MB_DATA_HEIGHT; }
				else if (iformat == MBF_KEMKMALL) { sensordepth_mode = MBPREPROCESS_MERGE_ASYNC; sensordepth_async = MB_DATA_SENSORDEPTH; }
				else if (iformat == MBF_RESON7KR) { sensordepth_mode = MBPREPROCESS_MERGE_ASYNC; sensordepth_async = MB_DATA_SENSORDEPTH; }
				else if (iformat == MBF_RESON7K3) { sensordepth_mode = MBPREPROCESS_MERGE_ASYNC; sensordepth_async = MB_DATA_NAV; }
			}
			if (heading_mode == MBPREPROCESS_MERGE_OFF) {
				if (iformat == MBF_EMOLDRAW || iformat == MBF_EM300RAW || iformat == MBF_EM710RAW) { heading_mode = MBPREPROCESS_MERGE_ASYNC; heading_async = MB_DATA_NAV; }
				else if (iformat == MBF_KEMKMALL) { heading_mode = MBPREPROCESS_MERGE_ASYNC; heading_async = MB_DATA_NAV1; }
				else if (iformat == MBF_RESON7KR) { heading_mode = MBPREPROCESS_MERGE_ASYNC; heading_async = MB_DATA_HEADING; }
				else if (iformat == MBF_RESON7K3) { heading_mode = MBPREPROCESS_MERGE_ASYNC; heading_async = MB_DATA_NAV; }
			}
			if (attitude_mode == MBPREPROCESS_MERGE_OFF) {
				if (iformat == MBF_EMOLDRAW || iformat == MBF_EM300RAW || iformat == MBF_EM710RAW) { attitude_mode = MBPREPROCESS_MERGE_ASYNC; attitude_async = MB_DATA_ATTITUDE; }
				else if (iformat == MBF_KEMKMALL) { attitude_mode = MBPREPROCESS_MERGE_ASYNC; attitude_async = MB_DATA_NAV1; }
				else if (iformat == MBF_RESON7KR) { attitude_mode = MBPREPROCESS_MERGE_ASYNC; attitude_async = MB_DATA_ATTITUDE; }
				else if (iformat == MBF_RESON7K3) { attitude_mode = MBPREPROCESS_MERGE_ASYNC; attitude_async = MB_DATA_ATTITUDE; }
			}

			if (verbose > 0)
				GMT_Report(API, GMT_MSG_INFORMATION, "\nPass 1: Opening file %s %d\n", ifile, iformat);

			if (mb_read_init(verbose, ifile, iformat, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap,
			                 &imbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
				char *message;
				mb_error(verbose, error, &message);
				GMT_Report(API, GMT_MSG_NORMAL, "%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
				GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
				GMT_Report(API, GMT_MSG_NORMAL, "\nMultibeam File <%s> not initialized for reading\n", ifile);
				FAIL(error);
			}

			if (mb_preprocess(verbose, imbio_ptr, NULL, NULL, (void *)&preprocess_pars, &error) == MB_FAILURE) {
				status = MB_SUCCESS;
				error = MB_ERROR_NO_ERROR;
			}

			beamflag = NULL; bath = NULL; amp = NULL;
			bathacrosstrack = NULL; bathalongtrack = NULL;
			ss = NULL; ssacrosstrack = NULL; ssalongtrack = NULL;
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char),   (void **)&beamflag, &error);
			if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
			if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE,  sizeof(double), (void **)&amp, &error);
			if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, &error);
			if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, &error);
			if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,   sizeof(double), (void **)&ss, &error);
			if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,   sizeof(double), (void **)&ssacrosstrack, &error);
			if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,   sizeof(double), (void **)&ssalongtrack, &error);

			if (error != MB_ERROR_NO_ERROR) {
				char *message;
				mb_error(verbose, error, &message);
				GMT_Report(API, GMT_MSG_NORMAL, "%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
				GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
				FAIL(error);
			}

			int n_rf_data = 0, n_rf_comment = 0;
			int n_rf_nav = 0, n_rf_nav1 = 0, n_rf_nav2 = 0, n_rf_nav3 = 0;
			int n_rf_att = 0, n_rf_att1 = 0, n_rf_att2 = 0, n_rf_att3 = 0;

			while (error <= MB_ERROR_NO_ERROR) {
				error = MB_ERROR_NO_ERROR;
				status = mb_get_all(verbose, imbio_ptr, &istore_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
				                    &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
				                    bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

				if (error < MB_ERROR_NO_ERROR && error > MB_ERROR_UNINTELLIGIBLE) {
					error = MB_ERROR_NO_ERROR;
					status = MB_SUCCESS;
				}

				if (verbose >= 2) {
					GMT_Report(API, GMT_MSG_INFORMATION, "\ndbg2  Data record read in program <%s>\n", program_name);
					GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       kind:           %d\n", kind);
					GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       error:          %d\n", error);
					GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       status:         %d\n", status);
				}

				if      (kind == MB_DATA_DATA)      { n_rf_data++;    n_rt_data++; }
				else if (kind == MB_DATA_COMMENT)   { n_rf_comment++; n_rt_comment++; }
				else if (kind == MB_DATA_NAV)       { n_rf_nav++;     n_rt_nav++; }
				else if (kind == MB_DATA_NAV1)      { n_rf_nav1++;    n_rt_nav1++; }
				else if (kind == MB_DATA_NAV2)      { n_rf_nav2++;    n_rt_nav2++; }
				else if (kind == MB_DATA_NAV3)      { n_rf_nav3++;    n_rt_nav3++; }
				else if (kind == MB_DATA_ATTITUDE)  { n_rf_att++;     n_rt_att++; }
				else if (kind == MB_DATA_ATTITUDE1) { n_rf_att1++;    n_rt_att1++; }
				else if (kind == MB_DATA_ATTITUDE2) { n_rf_att2++;    n_rt_att2++; }
				else if (kind == MB_DATA_ATTITUDE3) { n_rf_att3++;    n_rt_att3++; }

				if (status == MB_SUCCESS && nav_mode == MBPREPROCESS_MERGE_ASYNC && kind == nav_async) {
					int extract_status = mb_extract_nnav(verbose, imbio_ptr, istore_ptr, nanavmax, &kind, &nanav, atime_i, atime_d, alon, alat,
					                                     aspeed, aheading, asensordepth, aroll, apitch, aheave, &error);
					if (extract_status == MB_SUCCESS && nanav > 0 && n_nav + nanav >= n_nav_alloc) {
						error = MB_ERROR_NO_ERROR;
						n_nav_alloc += MAX(MBPREPROCESS_ALLOC_CHUNK, nanav);
						status  = mb_reallocd(verbose, __FILE__, __LINE__, n_nav_alloc * sizeof(double), (void **)&nav_time_d, &error);
						status &= mb_reallocd(verbose, __FILE__, __LINE__, n_nav_alloc * sizeof(double), (void **)&nav_navlon, &error);
						status &= mb_reallocd(verbose, __FILE__, __LINE__, n_nav_alloc * sizeof(double), (void **)&nav_navlat, &error);
						status &= mb_reallocd(verbose, __FILE__, __LINE__, n_nav_alloc * sizeof(double), (void **)&nav_speed,  &error);
						if (error != MB_ERROR_NO_ERROR) {
							char *message; mb_error(verbose, error, &message);
							GMT_Report(API, GMT_MSG_NORMAL, "%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
							GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
							FAIL(error);
						}
					}
					if (extract_status == MB_SUCCESS && nanav > 0) {
						for (int i = 0; i < nanav; i++) {
							if (atime_d[i] > 0.0 && alon[i] != 0.0 && alat[i] != 0.0) {
								nav_time_d[n_nav] = atime_d[i];
								nav_navlon[n_nav] = alon[i];
								nav_navlat[n_nav] = alat[i];
								nav_speed[n_nav]  = aspeed[i];
								n_nav++;
							}
						}
					}
				}

				if (status == MB_SUCCESS && sensordepth_mode == MBPREPROCESS_MERGE_ASYNC && kind == sensordepth_async) {
					int extract_status = mb_extract_nnav(verbose, imbio_ptr, istore_ptr, nanavmax, &kind, &nanav, atime_i, atime_d, alon, alat,
					                                     aspeed, aheading, asensordepth, aroll, apitch, aheave, &error);
					if (extract_status == MB_SUCCESS && nanav > 0 && n_sensordepth + nanav >= n_sensordepth_alloc) {
						error = MB_ERROR_NO_ERROR;
						n_sensordepth_alloc += MAX(MBPREPROCESS_ALLOC_CHUNK, nanav);
						status  = mb_reallocd(verbose, __FILE__, __LINE__, n_sensordepth_alloc * sizeof(double), (void **)&sensordepth_time_d, &error);
						status &= mb_reallocd(verbose, __FILE__, __LINE__, n_sensordepth_alloc * sizeof(double), (void **)&sensordepth_sensordepth, &error);
						if (error != MB_ERROR_NO_ERROR) { FAIL(error); }
					}
					if (extract_status == MB_SUCCESS && nanav > 0) {
						for (int i = 0; i < nanav; i++) {
							sensordepth_time_d[n_sensordepth] = atime_d[i];
							sensordepth_sensordepth[n_sensordepth] = asensordepth[i];
							n_sensordepth++;
						}
					}
				}

				if (status == MB_SUCCESS && heading_mode == MBPREPROCESS_MERGE_ASYNC && kind == heading_async) {
					int extract_status = mb_extract_nnav(verbose, imbio_ptr, istore_ptr, nanavmax, &kind, &nanav, atime_i, atime_d, alon, alat,
					                                     aspeed, aheading, asensordepth, aroll, apitch, aheave, &error);
					if (extract_status == MB_SUCCESS && nanav > 0 && n_heading + nanav >= n_heading_alloc) {
						error = MB_ERROR_NO_ERROR;
						n_heading_alloc += MAX(MBPREPROCESS_ALLOC_CHUNK, nanav);
						status  = mb_reallocd(verbose, __FILE__, __LINE__, n_heading_alloc * sizeof(double), (void **)&heading_time_d, &error);
						status &= mb_reallocd(verbose, __FILE__, __LINE__, n_heading_alloc * sizeof(double), (void **)&heading_heading, &error);
						if (error != MB_ERROR_NO_ERROR) { FAIL(error); }
					}
					if (extract_status == MB_SUCCESS && nanav > 0) {
						for (int i = 0; i < nanav; i++) {
							heading_time_d[n_heading] = atime_d[i];
							heading_heading[n_heading] = aheading[i];
							n_heading++;
						}
					}
				}

				if (status == MB_SUCCESS && altitude_mode == MBPREPROCESS_MERGE_ASYNC && kind == altitude_async) {
					int extract_status = mb_extract_altitude(verbose, imbio_ptr, istore_ptr, &kind, &sensordepth, &altitude, &error);
					if (extract_status == MB_SUCCESS && n_altitude + 1 >= n_altitude_alloc) {
						error = MB_ERROR_NO_ERROR;
						n_altitude_alloc += MBPREPROCESS_ALLOC_CHUNK;
						status  = mb_reallocd(verbose, __FILE__, __LINE__, n_altitude_alloc * sizeof(double), (void **)&altitude_time_d,   &error);
						status &= mb_reallocd(verbose, __FILE__, __LINE__, n_altitude_alloc * sizeof(double), (void **)&altitude_altitude, &error);
						if (error != MB_ERROR_NO_ERROR) { FAIL(error); }
					}
					if (extract_status == MB_SUCCESS) {
						altitude_time_d[n_altitude] = time_d;
						altitude_altitude[n_altitude] = altitude;
						n_altitude++;
					}
				}

				if (status == MB_SUCCESS && attitude_mode == MBPREPROCESS_MERGE_ASYNC && kind == attitude_async) {
					int extract_status = mb_extract_nnav(verbose, imbio_ptr, istore_ptr, nanavmax, &kind, &nanav, atime_i, atime_d, alon, alat,
					                                     aspeed, aheading, asensordepth, aroll, apitch, aheave, &error);
					if (extract_status == MB_SUCCESS && nanav > 0 && n_attitude + nanav >= n_attitude_alloc) {
						error = MB_ERROR_NO_ERROR;
						n_attitude_alloc += MAX(MBPREPROCESS_ALLOC_CHUNK, nanav);
						status  = mb_reallocd(verbose, __FILE__, __LINE__, n_attitude_alloc * sizeof(double), (void **)&attitude_time_d, &error);
						status &= mb_reallocd(verbose, __FILE__, __LINE__, n_attitude_alloc * sizeof(double), (void **)&attitude_roll,   &error);
						status &= mb_reallocd(verbose, __FILE__, __LINE__, n_attitude_alloc * sizeof(double), (void **)&attitude_pitch,  &error);
						status &= mb_reallocd(verbose, __FILE__, __LINE__, n_attitude_alloc * sizeof(double), (void **)&attitude_heave,  &error);
						if (error != MB_ERROR_NO_ERROR) { FAIL(error); }
					}
					if (extract_status == MB_SUCCESS && nanav > 0) {
						for (int i = 0; i < nanav; i++) {
							attitude_time_d[n_attitude] = atime_d[i];
							attitude_roll[n_attitude]   = aroll[i];
							attitude_pitch[n_attitude]  = apitch[i];
							attitude_heave[n_attitude]  = aheave[i];
							n_attitude++;
						}
					}
				}
			}

			status &= mb_indextable(verbose, imbio_ptr, &i_num_indextable, (void **)&i_indextable, &error);
			if (i_num_indextable > 0) {
				if (num_indextable_alloc <= num_indextable + i_num_indextable) {
					num_indextable_alloc += i_num_indextable;
					status &= mb_reallocd(verbose, __FILE__, __LINE__,
					                      num_indextable_alloc * sizeof(struct mb_io_indextable_struct),
					                      (void **)(&indextable), &error);
					if (error != MB_ERROR_NO_ERROR) { FAIL(error); }
				}
				memcpy(&indextable[num_indextable], i_indextable,
				       i_num_indextable * sizeof(struct mb_io_indextable_struct));
				for (int i = num_indextable; i < num_indextable + i_num_indextable; i++)
					indextable[i].file_index = n_rt_files;
				num_indextable += i_num_indextable;
			}

			if (verbose > 0) {
				GMT_Report(API, GMT_MSG_INFORMATION, "Pass 1: Records read from input file %d: %s\n", n_rt_files, ifile);
				GMT_Report(API, GMT_MSG_INFORMATION, "     %d survey records\n", n_rf_data);
				GMT_Report(API, GMT_MSG_INFORMATION, "     %d comment records\n", n_rf_comment);
				GMT_Report(API, GMT_MSG_INFORMATION, "     %d nav records\n", n_rf_nav);
				GMT_Report(API, GMT_MSG_INFORMATION, "     %d nav1 records\n", n_rf_nav1);
				GMT_Report(API, GMT_MSG_INFORMATION, "     %d nav2 records\n", n_rf_nav2);
				GMT_Report(API, GMT_MSG_INFORMATION, "     %d nav3 records\n", n_rf_nav3);
				GMT_Report(API, GMT_MSG_INFORMATION, "     %d att records\n", n_rf_att);
				GMT_Report(API, GMT_MSG_INFORMATION, "     %d att1 records\n", n_rf_att1);
				GMT_Report(API, GMT_MSG_INFORMATION, "     %d att2 records\n", n_rf_att2);
				GMT_Report(API, GMT_MSG_INFORMATION, "     %d att3 records\n", n_rf_att3);
			}

			status &= mb_close(verbose, &imbio_ptr, &error);
			n_rt_files++;
		}

		if (read_datalist) {
			if (mb_datalist_read(verbose, datalist, ifile, dfile, &iformat, &file_weight, &error) == MB_SUCCESS) read_data = true;
			else read_data = false;
		} else read_data = false;
	}
	if (read_datalist) mb_datalist_close(verbose, &datalist, &error);

	if (verbose > 0) {
		GMT_Report(API, GMT_MSG_INFORMATION, "\n-----------------------------------------------\n");
		GMT_Report(API, GMT_MSG_INFORMATION, "Pass 1: Total records read from %d input files:\n", n_rt_files);
		GMT_Report(API, GMT_MSG_INFORMATION, "     %d survey records\n", n_rt_data);
		GMT_Report(API, GMT_MSG_INFORMATION, "     %d comment records\n", n_rt_comment);
		GMT_Report(API, GMT_MSG_INFORMATION, "     %d nav records\n", n_rt_nav);
		GMT_Report(API, GMT_MSG_INFORMATION, "     %d att records\n", n_rt_att);
		GMT_Report(API, GMT_MSG_INFORMATION, "Pass 1: Asynchronous data available for merging:\n");
		GMT_Report(API, GMT_MSG_INFORMATION, "     %d navigation data (mode:%d)\n", n_nav, nav_mode);
		GMT_Report(API, GMT_MSG_INFORMATION, "     %d sensordepth data (mode:%d)\n", n_sensordepth, sensordepth_mode);
		GMT_Report(API, GMT_MSG_INFORMATION, "     %d heading data (mode:%d)\n", n_heading, heading_mode);
		GMT_Report(API, GMT_MSG_INFORMATION, "     %d altitude data (mode:%d)\n", n_altitude, altitude_mode);
		GMT_Report(API, GMT_MSG_INFORMATION, "     %d attitude data (mode:%d)\n", n_attitude, attitude_mode);
		GMT_Report(API, GMT_MSG_INFORMATION, "     %d time_latency data (mode:%d)\n", time_latency_num, time_latency_mode);
		GMT_Report(API, GMT_MSG_INFORMATION, "-----------------------------------------------\n");
	}

	if (zero_heave) {
		for (int i = 0; i < n_attitude; i++) attitude_heave[i] = 0.0;
	}

	/* MBARI pressure-depth time jumps */
	if (kluge_timejumps_mbaripressure) {
		if (verbose > 0) {
			GMT_Report(API, GMT_MSG_INFORMATION, "\n-----------------------------------------------\n");
			GMT_Report(API, GMT_MSG_INFORMATION, "Applying time jump corrections to MBARI pressure depth data:\n");
		}
		if (n_sensordepth > 2 && n_sensordepth_alloc >= n_sensordepth) {
			dtime_d_expect = (sensordepth_time_d[n_sensordepth-1] - sensordepth_time_d[0]) / (n_sensordepth - 1);
			if (fabs((sensordepth_time_d[1] - sensordepth_time_d[0]) - dtime_d_expect) < kluge_timejumps_mba_threshold)
				dtime_d_expect = (sensordepth_time_d[1] - sensordepth_time_d[0]);
			bool correction_on = false;
			double correction_start_time_d = 0.0;
			int correction_start_index = 0;
			int correction_end_index = -1;
			for (int i = 2; i < n_sensordepth; i++) {
				dtime_d = sensordepth_time_d[i] - sensordepth_time_d[i-1];
				if (fabs(dtime_d - dtime_d_expect) >= kluge_timejumps_mba_threshold) {
					if (!correction_on) {
						correction_on = true;
						correction_start_time_d = sensordepth_time_d[i-1];
						correction_start_index = i;
						correction_end_index = i - 1;
					}
					GMT_Report(API, GMT_MSG_INFORMATION, "DEP MBARI FIX: i:%d t: %f %f dt: %f %f ",
					        i, sensordepth_time_d[i-1], sensordepth_time_d[i], dtime_d, dtime_d_expect);
					if (sensordepth_time_d[i] < correction_start_time_d) correction_end_index = i;
					sensordepth_time_d[i] = sensordepth_time_d[i-1] + dtime_d_expect;
					GMT_Report(API, GMT_MSG_INFORMATION, "newt[%d]: %f\n", i, sensordepth_time_d[i]);
				} else {
					if (correction_on && correction_end_index > correction_start_index) {
						for (int ii = correction_start_index; ii <= correction_end_index; ii++) {
							GMT_Report(API, GMT_MSG_INFORMATION, "DEP MBARI DELETE: i:%d t:%f\n", ii, sensordepth_time_d[ii]);
							sensordepth_time_d[ii] = 0.0;
						}
					}
					correction_on = false;
				}
			}
			int nn = n_sensordepth;
			for (int i = n_sensordepth - 1; i >= 0; i--) {
				if (sensordepth_time_d[i] == 0.0) {
					for (int ii = i; ii < nn - 1; ii++) {
						sensordepth_time_d[ii] = sensordepth_time_d[ii+1];
						sensordepth_sensordepth[ii] = sensordepth_sensordepth[ii+1];
					}
					nn--;
				}
			}
			n_sensordepth = nn;
		}
	}

	/* ancillary time jump corrections */
	if (kluge_timejumps_ancilliary) {
		if (verbose > 0) {
			GMT_Report(API, GMT_MSG_INFORMATION, "\n-----------------------------------------------\n");
			GMT_Report(API, GMT_MSG_INFORMATION, "Applying time jump corrections to ancillary data:\n");
		}
		if (n_nav > 2 && n_nav_alloc >= n_nav) {
			dtime_d_expect = (nav_time_d[n_nav-1] - nav_time_d[0]) / (n_nav - 1);
			if (fabs((nav_time_d[1] - nav_time_d[0]) - dtime_d_expect) < kluge_timejumps_anc_threshold)
				dtime_d_expect = (nav_time_d[1] - nav_time_d[0]);
			for (int i = 2; i < n_nav; i++) {
				dtime_d = nav_time_d[i] - nav_time_d[i-1];
				if (fabs(dtime_d - dtime_d_expect) >= kluge_timejumps_anc_threshold) {
					GMT_Report(API, GMT_MSG_INFORMATION, "NAV TIME JUMP FIX: i:%d t: %f %f dt: %f %f ", i, nav_time_d[i-1], nav_time_d[i], dtime_d, dtime_d_expect);
					nav_time_d[i] = nav_time_d[i-1] + dtime_d_expect;
					GMT_Report(API, GMT_MSG_INFORMATION, "newt[%d]: %f\n", i, nav_time_d[i]);
				}
			}
		}
		if (n_sensordepth > 2 && n_sensordepth_alloc >= n_sensordepth) {
			dtime_d_expect = (sensordepth_time_d[n_sensordepth-1] - sensordepth_time_d[0]) / (n_sensordepth - 1);
			if (fabs((sensordepth_time_d[1] - sensordepth_time_d[0]) - dtime_d_expect) < kluge_timejumps_anc_threshold)
				dtime_d_expect = (sensordepth_time_d[1] - sensordepth_time_d[0]);
			for (int i = 2; i < n_sensordepth; i++) {
				dtime_d = sensordepth_time_d[i] - sensordepth_time_d[i-1];
				if (fabs(dtime_d - dtime_d_expect) >= kluge_timejumps_anc_threshold) {
					GMT_Report(API, GMT_MSG_INFORMATION, "DEP TIME JUMP FIX: i:%d t: %f %f dt: %f %f ", i, sensordepth_time_d[i-1], sensordepth_time_d[i], dtime_d, dtime_d_expect);
					sensordepth_time_d[i] = sensordepth_time_d[i-1] + dtime_d_expect;
					GMT_Report(API, GMT_MSG_INFORMATION, "newt[%d]: %f\n", i, sensordepth_time_d[i]);
				}
			}
		}
		if (n_heading > 2 && n_heading_alloc >= n_heading) {
			dtime_d_expect = (heading_time_d[n_heading-1] - heading_time_d[0]) / (n_heading - 1);
			if (fabs((heading_time_d[1] - heading_time_d[0]) - dtime_d_expect) < kluge_timejumps_anc_threshold)
				dtime_d_expect = (heading_time_d[1] - heading_time_d[0]);
			for (int i = 2; i < n_heading; i++) {
				dtime_d = heading_time_d[i] - heading_time_d[i-1];
				if (fabs(dtime_d - dtime_d_expect) >= kluge_timejumps_anc_threshold) {
					GMT_Report(API, GMT_MSG_INFORMATION, "HDG TIME JUMP FIX: i:%d t: %f %f dt: %f %f ", i, heading_time_d[i-1], heading_time_d[i], dtime_d, dtime_d_expect);
					heading_time_d[i] = heading_time_d[i-1] + dtime_d_expect;
					GMT_Report(API, GMT_MSG_INFORMATION, "newt[%d]: %f\n", i, heading_time_d[i]);
				}
			}
		}
		if (n_altitude > 2 && n_altitude_alloc >= n_altitude) {
			dtime_d_expect = (altitude_time_d[n_altitude-1] - altitude_time_d[0]) / (n_altitude - 1);
			if (fabs((altitude_time_d[1] - altitude_time_d[0]) - dtime_d_expect) < kluge_timejumps_anc_threshold)
				dtime_d_expect = (altitude_time_d[1] - altitude_time_d[0]);
			for (int i = 2; i < n_altitude; i++) {
				dtime_d = altitude_time_d[i] - altitude_time_d[i-1];
				if (fabs(dtime_d - dtime_d_expect) >= kluge_timejumps_anc_threshold) {
					GMT_Report(API, GMT_MSG_INFORMATION, "ALT TIME JUMP FIX: i:%d t: %f %f dt: %f %f ", i, altitude_time_d[i-1], altitude_time_d[i], dtime_d, dtime_d_expect);
					altitude_time_d[i] = altitude_time_d[i-1] + dtime_d_expect;
					GMT_Report(API, GMT_MSG_INFORMATION, "newt[%d]: %f\n", i, altitude_time_d[i]);
				}
			}
		}
		if (n_attitude > 2 && n_attitude_alloc >= n_attitude) {
			dtime_d_expect = (attitude_time_d[n_attitude-1] - attitude_time_d[0]) / (n_attitude - 1);
			if (fabs((attitude_time_d[1] - attitude_time_d[0]) - dtime_d_expect) < kluge_timejumps_anc_threshold)
				dtime_d_expect = (attitude_time_d[1] - attitude_time_d[0]);
			for (int i = 2; i < n_attitude; i++) {
				dtime_d = attitude_time_d[i] - attitude_time_d[i-1];
				if (fabs(dtime_d - dtime_d_expect) >= kluge_timejumps_anc_threshold) {
					GMT_Report(API, GMT_MSG_INFORMATION, "ATT TIME JUMP FIX: i:%d t: %f %f dt: %f %f ", i, attitude_time_d[i-1], attitude_time_d[i], dtime_d, dtime_d_expect);
					attitude_time_d[i] = attitude_time_d[i-1] + dtime_d_expect;
					GMT_Report(API, GMT_MSG_INFORMATION, "newt[%d]: %f\n", i, attitude_time_d[i]);
				}
			}
		}
	}

	/* time latency corrections */
	if (verbose > 0) {
		GMT_Report(API, GMT_MSG_INFORMATION, "\n-----------------------------------------------\n");
		GMT_Report(API, GMT_MSG_INFORMATION, "Applying time latency corrections:\n");
	}
	if (n_nav > 0 && n_nav_alloc >= n_nav) {
		if (sensor_position != NULL && sensor_position->time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) {
			if (verbose > 0) {
				if (sensor_position->time_latency_mode == MB_SENSOR_TIME_LATENCY_STATIC)
					GMT_Report(API, GMT_MSG_INFORMATION, "Applying time latency correction from platform model to %d position data using constant offset %f\n", n_nav, sensor_position->time_latency_static);
				else
					GMT_Report(API, GMT_MSG_INFORMATION, "Applying time latency correction from platform model to %d position data using time-varying model\n", n_nav);
			}
			mb_apply_time_latency(verbose, n_nav, nav_time_d, sensor_position->time_latency_mode,
			                      sensor_position->time_latency_static, sensor_position->num_time_latency,
			                      sensor_position->time_latency_time_d, sensor_position->time_latency_value, &error);
		}
		if ((time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) && (time_latency_apply & MBPREPROCESS_TIME_LATENCY_APPLY_NAV)) {
			if (verbose > 0) {
				if (time_latency_mode == MB_SENSOR_TIME_LATENCY_STATIC)
					GMT_Report(API, GMT_MSG_INFORMATION, "Applying time latency correction from command line to %d position data using constant offset %f\n", n_nav, time_latency_constant);
				else
					GMT_Report(API, GMT_MSG_INFORMATION, "Applying time latency correction from command line to %d position data using time-varying model\n", n_nav);
			}
			mb_apply_time_latency(verbose, n_nav, nav_time_d, time_latency_mode, time_latency_constant, time_latency_num,
			                      time_latency_time_d, time_latency_time_latency, &error);
		}
	}
	if (n_sensordepth > 0 && n_sensordepth_alloc >= n_sensordepth) {
		if (sensor_depth != NULL && sensor_depth->time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) {
			if (verbose > 0) GMT_Report(API, GMT_MSG_INFORMATION, "Applying time latency correction from platform model to %d sensordepth data\n", n_sensordepth);
			mb_apply_time_latency(verbose, n_sensordepth, sensordepth_time_d, sensor_depth->time_latency_mode,
			                      sensor_depth->time_latency_static, sensor_depth->num_time_latency,
			                      sensor_depth->time_latency_time_d, sensor_depth->time_latency_value, &error);
		}
		if ((time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) && (time_latency_apply & MBPREPROCESS_TIME_LATENCY_APPLY_SENSORDEPTH)) {
			if (verbose > 0) GMT_Report(API, GMT_MSG_INFORMATION, "Applying time latency correction from command line to %d sensordepth data\n", n_sensordepth);
			mb_apply_time_latency(verbose, n_sensordepth, sensordepth_time_d, time_latency_mode, time_latency_constant,
			                      time_latency_num, time_latency_time_d, time_latency_time_latency, &error);
		}
	}
	if (n_heading > 0 && n_heading_alloc >= n_heading) {
		if (sensor_heading != NULL && sensor_heading->time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) {
			if (verbose > 0) GMT_Report(API, GMT_MSG_INFORMATION, "Applying time latency correction from platform model to %d heading data\n", n_heading);
			mb_apply_time_latency(verbose, n_heading, heading_time_d, sensor_heading->time_latency_mode,
			                      sensor_heading->time_latency_static, sensor_heading->num_time_latency,
			                      sensor_heading->time_latency_time_d, sensor_heading->time_latency_value, &error);
		}
		if ((time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) && (time_latency_apply & MBPREPROCESS_TIME_LATENCY_APPLY_HEADING)) {
			if (verbose > 0) GMT_Report(API, GMT_MSG_INFORMATION, "Applying time latency correction from command line to %d heading data\n", n_heading);
			mb_apply_time_latency(verbose, n_heading, heading_time_d, time_latency_mode, time_latency_constant, time_latency_num,
			                      time_latency_time_d, time_latency_time_latency, &error);
		}
	}
	if (n_altitude > 0 && n_altitude_alloc >= n_altitude) {
		if ((time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) && (time_latency_apply & MBPREPROCESS_TIME_LATENCY_APPLY_ALTITUDE)) {
			if (verbose > 0) GMT_Report(API, GMT_MSG_INFORMATION, "Applying time latency correction from command line to %d altitude data\n", n_altitude);
			mb_apply_time_latency(verbose, n_altitude, altitude_time_d, time_latency_mode, time_latency_constant,
			                      time_latency_num, time_latency_time_d, time_latency_time_latency, &error);
		}
	}
	if (n_attitude > 0 && n_attitude_alloc >= n_attitude) {
		if (sensor_rollpitch != NULL && sensor_rollpitch->time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) {
			if (verbose > 0) GMT_Report(API, GMT_MSG_INFORMATION, "Applying time latency correction from platform model to %d attitude data\n", n_attitude);
			mb_apply_time_latency(verbose, n_attitude, attitude_time_d, sensor_rollpitch->time_latency_mode,
			                      sensor_rollpitch->time_latency_static, sensor_rollpitch->num_time_latency,
			                      sensor_rollpitch->time_latency_time_d, sensor_rollpitch->time_latency_value, &error);
		}
		if ((time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) && (time_latency_apply & MBPREPROCESS_TIME_LATENCY_APPLY_ATTITUDE)) {
			if (verbose > 0) GMT_Report(API, GMT_MSG_INFORMATION, "Applying time latency correction from command line to %d attitude data\n", n_attitude);
			mb_apply_time_latency(verbose, n_attitude, attitude_time_d, time_latency_mode, time_latency_constant,
			                      time_latency_num, time_latency_time_d, time_latency_time_latency, &error);
		}
	}

	if (verbose > 0) {
		GMT_Report(API, GMT_MSG_INFORMATION, "\n-----------------------------------------------\n");
		GMT_Report(API, GMT_MSG_INFORMATION, "Applying time domain filtering:\n");
	}
	if ((filter_apply & MBPREPROCESS_TIME_LATENCY_APPLY_NAV) && n_nav > 0 && n_nav_alloc >= n_nav) {
		if (verbose > 0) GMT_Report(API, GMT_MSG_INFORMATION, "Applying %f second Gaussian filter to %d position data\n", filter_length, n_nav);
		mb_apply_time_filter(verbose, n_nav, nav_time_d, nav_navlon, filter_length, &error);
		mb_apply_time_filter(verbose, n_nav, nav_time_d, nav_navlat, filter_length, &error);
	}
	if ((filter_apply & MBPREPROCESS_TIME_LATENCY_APPLY_SENSORDEPTH) && n_sensordepth > 0 && n_sensordepth_alloc >= n_sensordepth) {
		if (verbose > 0) GMT_Report(API, GMT_MSG_INFORMATION, "Applying %f second Gaussian filter to %d sensordepth data\n", filter_length, n_sensordepth);
		mb_apply_time_filter(verbose, n_sensordepth, sensordepth_time_d, sensordepth_sensordepth, filter_length, &error);
	}
	if ((filter_apply & MBPREPROCESS_TIME_LATENCY_APPLY_HEADING) && n_heading > 0 && n_heading_alloc >= n_heading) {
		if (verbose > 0) GMT_Report(API, GMT_MSG_INFORMATION, "Applying %f second Gaussian filter to %d heading data\n", filter_length, n_heading);
		mb_apply_time_filter(verbose, n_heading, heading_time_d, heading_heading, filter_length, &error);
	}
	if ((filter_apply & MBPREPROCESS_TIME_LATENCY_APPLY_ALTITUDE) && n_altitude > 0 && n_altitude_alloc >= n_altitude) {
		if (verbose > 0) GMT_Report(API, GMT_MSG_INFORMATION, "Applying %f second Gaussian filter to %d altitude data\n", filter_length, n_altitude);
		mb_apply_time_filter(verbose, n_altitude, altitude_time_d, altitude_altitude, filter_length, &error);
	}
	if ((filter_apply & MBPREPROCESS_TIME_LATENCY_APPLY_ATTITUDE) && n_attitude > 0 && n_attitude_alloc >= n_attitude) {
		if (verbose > 0) GMT_Report(API, GMT_MSG_INFORMATION, "Applying %f second Gaussian filter to %d attitude data\n", filter_length, n_attitude);
		mb_apply_time_filter(verbose, n_attitude, attitude_time_d, attitude_roll,  filter_length, &error);
		mb_apply_time_filter(verbose, n_attitude, attitude_time_d, attitude_pitch, filter_length, &error);
		mb_apply_time_filter(verbose, n_attitude, attitude_time_d, attitude_heave, filter_length, &error);
	}
	if (verbose > 0) GMT_Report(API, GMT_MSG_INFORMATION, "-----------------------------------------------\n");

	if ((kluge_flipsign_roll || kluge_flipsign_pitch) && n_attitude > 0 && n_attitude_alloc >= n_attitude) {
		if (verbose > 0) {
			GMT_Report(API, GMT_MSG_INFORMATION, "\n-----------------------------------------------\n");
			if (kluge_flipsign_roll)  GMT_Report(API, GMT_MSG_INFORMATION, "Flipping sign of roll\n");
			if (kluge_flipsign_pitch) GMT_Report(API, GMT_MSG_INFORMATION, "Flipping sign of pitch\n");
		}
		if (kluge_flipsign_roll)  for (int i = 0; i < n_attitude; i++) attitude_roll[i]  *= -1.0;
		if (kluge_flipsign_pitch) for (int i = 0; i < n_attitude; i++) attitude_pitch[i] *= -1.0;
		if (verbose > 0) GMT_Report(API, GMT_MSG_INFORMATION, "-----------------------------------------------\n");
	}

	int n_rf_data = 0, n_rf_comment = 0;
	int n_rf_nav = 0, n_rf_nav1 = 0, n_rf_nav2 = 0, n_rf_nav3 = 0;
	int n_rf_att = 0, n_rf_att1 = 0, n_rf_att2 = 0, n_rf_att3 = 0;
	int n_rf_dup_timestamp = 0;
	n_rt_data = 0; n_rt_files = 0; n_rt_dup_timestamp = 0;

	int n_wf_data = 0, n_wf_comment = 0;
	int n_wf_nav = 0, n_wf_nav1 = 0, n_wf_nav2 = 0, n_wf_nav3 = 0;
	int n_wf_att = 0, n_wf_att1 = 0, n_wf_att2 = 0, n_wf_att3 = 0;
	n_wt_data = 0; n_wt_comment = 0;
	n_wt_nav = 0; n_wt_nav1 = 0; n_wt_nav2 = 0; n_wt_nav3 = 0;
	n_wt_att = 0; n_wt_att1 = 0; n_wt_att2 = 0; n_wt_att3 = 0;
	n_wt_files = 0;

	if (output_sensor_fnv && platform != NULL) {
		if (verbose > 0) GMT_Report(API, GMT_MSG_INFORMATION, "\nOutputting fnv files for survey sensors\n");
		for (isensor = 0; isensor < platform->num_sensors; isensor++) {
			if (verbose > 0) GMT_Report(API, GMT_MSG_INFORMATION, "Outputting sensor %d with capability %d\n", isensor, platform->sensors[isensor].capability2);
			for (ioffset = 0; ioffset < platform->sensors[isensor].num_offsets; ioffset++) {
				mb_path fnvfile = "";
				snprintf(fnvfile, sizeof(fnvfile), "sensor_%2.2d_%2.2d_%2.2d.fnv", isensor, ioffset, platform->sensors[isensor].type);
				if (verbose > 0) GMT_Report(API, GMT_MSG_INFORMATION, "Outputting sensor %d offset %d in fnv file:%s\n", isensor, ioffset, fnvfile);
				if ((platform->sensors[isensor].offsets[ioffset].ofp = fopen(fnvfile, "wb")) == NULL) {
					GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open sensor fnv data file <%s> for writing\n", fnvfile);
					FAIL(MB_ERROR_OPEN_FAIL);
				}
			}
		}
	}

	if (read_datalist) {
		const int look_processed = MB_DATALIST_LOOK_UNSET;
		if (mb_datalist_open(verbose, &datalist, read_file, look_processed, &error) != MB_SUCCESS) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open data list file: %s\n", read_file);
			FAIL(MB_ERROR_OPEN_FAIL);
		}
		read_data = mb_datalist_read(verbose, datalist, ifile, dfile, &iformat, &file_weight, &error) == MB_SUCCESS;
	} else {
		strcpy(ifile, read_file);
		iformat = format;
		read_data = true;
	}

	while (read_data) {
		int sensorhead = 0;
		int sensortype = 0;

		if (iformat == MBF_EMOLDRAW || iformat == MBF_EM12IFRM || iformat == MBF_EM12DARW ||
		    iformat == MBF_EM300RAW || iformat == MBF_EM300MBA)
			oformat = MBF_EM300MBA;
		else if (iformat == MBF_EM710RAW || iformat == MBF_EM710MBA) oformat = MBF_EM710MBA;
		else if (iformat == MBF_IMAGE83P) oformat = MBF_IMAGEMBA;
		else if (iformat == MBF_3DWISSLR) oformat = MBF_3DWISSLP;
		else if (iformat == MBF_OICGEODA) oformat = MBF_OICMBARI;
		else if (iformat == MBF_SB2100RW) oformat = MBF_SB2100B2;
		else oformat = iformat;

		status = mb_get_format(verbose, ifile, fileroot, &testformat, &error);
		snprintf(ofile, sizeof(ofile), "%s.mb%d", fileroot, oformat);
		if (strcmp(ifile, ofile) == 0)
			snprintf(ofile, sizeof(ofile), "%sr.mb%d", fileroot, oformat);

		if (output_directory_set) {
			char buf[MB_PATH_MAXLINE] = "";
			strcpy(buf, output_directory);
			if (buf[strlen(output_directory) - 1] != '/') strcat(buf, "/");
			char *filenameptr;
			if (strrchr(ofile, '/') != NULL) filenameptr = strrchr(ofile, '/') + 1;
			else filenameptr = ofile;
			strcat(buf, filenameptr);
			strcpy(ofile, buf);
		}

		bool proceed = true;
		if (skip_existing) {
			if (stat(ifile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
				input_modtime = file_status.st_mtime; input_size = file_status.st_size;
			} else { input_modtime = 0; input_size = 0; }
			if (stat(ofile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
				output_modtime = file_status.st_mtime; output_size = file_status.st_size;
			} else { output_modtime = 0; output_size = 0; }
			if (output_modtime > input_modtime && output_size > input_size) proceed = false;
		}

		if (!proceed) {
			if (verbose > 0) GMT_Report(API, GMT_MSG_INFORMATION, "\nPass 2: Skipping input file:  %s %d\n", ifile, iformat);
		} else {
			if (verbose > 0) GMT_Report(API, GMT_MSG_INFORMATION, "\nPass 2: Opening input file:  %s %d\n", ifile, iformat);

			if (mb_read_init(verbose, ifile, iformat, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap,
			                 &imbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
				char *message; mb_error(verbose, error, &message);
				GMT_Report(API, GMT_MSG_NORMAL, "%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
				GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
				GMT_Report(API, GMT_MSG_NORMAL, "\nMultibeam File <%s> not initialized for reading\n", ifile);
				FAIL(error);
			}

			if (mb_preprocess(verbose, imbio_ptr, NULL, NULL, (void *)&preprocess_pars, &error) == MB_FAILURE) {
				status = MB_SUCCESS; error = MB_ERROR_NO_ERROR;
			}

			if (verbose > 0) GMT_Report(API, GMT_MSG_INFORMATION, "Pass 2: Opening output file: %s %d\n", ofile, oformat);

			if (mb_write_init(verbose, ofile, oformat, &ombio_ptr, &obeams_bath, &obeams_amp, &opixels_ss, &error) != MB_SUCCESS) {
				char *message; mb_error(verbose, error, &message);
				GMT_Report(API, GMT_MSG_NORMAL, "%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
				GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error returned from function <mb_write_init>:\n%s\n", message);
				GMT_Report(API, GMT_MSG_NORMAL, "\nMultibeam File <%s> not initialized for writing\n", ofile);
				FAIL(error);
			}

			bool make_fbt = false;
			void *fmbio_ptr = NULL;
			void *fstore_ptr = NULL;
			struct mb_io_struct *fmb_io_ptr = NULL;
			struct mbsys_ldeoih_struct *fstore = NULL;
			if (mb_should_make_fbt(verbose, oformat)) {
				char fbtfile[MB_PATH_MAXLINE+100];
				snprintf(fbtfile, sizeof(fbtfile), "%s.fbt", ofile);
				int fbeams_bath = 0, fbeams_amp = 0, fpixels_ss = 0;
				if (mb_write_init(verbose, fbtfile, MBF_MBLDEOIH, &fmbio_ptr, &fbeams_bath, &fbeams_amp, &fpixels_ss, &error) != MB_SUCCESS) {
					char *message = NULL; mb_error(verbose, error, &message);
					GMT_Report(API, GMT_MSG_NORMAL, "%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
					GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error returned from function <mb_write_init>:\n%s\n", message);
					FAIL(error);
				}
				fmb_io_ptr = (struct mb_io_struct *)fmbio_ptr;
				fstore = (struct mbsys_ldeoih_struct *) fmb_io_ptr->store_data;
				fstore_ptr = (void *) fstore;
				make_fbt = true;
			}

			bool make_fnv = false;
			FILE *nfp = NULL;
			if (mb_should_make_fnv(verbose, oformat)) {
				char fnvfile[MB_PATH_MAXLINE+100];
				snprintf(fnvfile, sizeof(fnvfile), "%s.fnv", ofile);
				if ((nfp = fopen(fnvfile, "w")) == NULL) {
					GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open output *.fnv file <%s> for writing\n", fnvfile);
					FAIL(MB_ERROR_OPEN_FAIL);
				}
				make_fnv = true;
				fprintf(nfp, "## <yyyy mm dd hh mm ss.ssssss> <epoch seconds> "
				             "<longitude (deg)> <latitude (deg)> <heading (deg)> <speed (km/hr)> "
				             "<draft (m)> <roll (deg)> <pitch (deg)> <heave (m)> <portlon (deg)> "
				             "<portlat (deg)> <stbdlon (deg)> <stbdlat (deg)>\n");
			}

			bool mask_bounds_init = false;
			double mask_bounds[4] = {0.0, 0.0, 0.0, 0.0};

			beamflag = NULL; bath = NULL; amp = NULL;
			bathacrosstrack = NULL; bathalongtrack = NULL;
			ss = NULL; ssacrosstrack = NULL; ssalongtrack = NULL;
			if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char),   (void **)&beamflag, &error);
			if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
			if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE,  sizeof(double), (void **)&amp, &error);
			if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, &error);
			if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, &error);
			if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,   sizeof(double), (void **)&ss, &error);
			if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,   sizeof(double), (void **)&ssacrosstrack, &error);
			if (error == MB_ERROR_NO_ERROR) status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,   sizeof(double), (void **)&ssalongtrack, &error);
			if (error != MB_ERROR_NO_ERROR) {
				char *message; mb_error(verbose, error, &message);
				GMT_Report(API, GMT_MSG_NORMAL, "%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
				GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
				FAIL(error);
			}

			/* delete old synchronous/asynchronous ancillary files */
			const char *aexts[] = {"ata", "ath", "ats", "sta", "baa", "bah", "bas", "bsa"};
			for (int ae = 0; ae < (int)(sizeof(aexts)/sizeof(aexts[0])); ae++) {
				snprintf(afile, sizeof(afile), "%s.%s", ofile, aexts[ae]);
				if (stat(afile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
					if (verbose > 0) GMT_Report(API, GMT_MSG_INFORMATION, "Deleting old ancillary file %s\n", afile);
					remove(afile);
				}
			}

			snprintf(afile, sizeof(afile), "%s.bsa", ofile);
			if ((afp = fopen(afile, "wb")) == NULL) {
				GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open synchronous attitude data file <%s> for writing\n", afile);
				FAIL(MB_ERROR_OPEN_FAIL);
			}

			n_rf_data = 0; n_rf_comment = 0;
			n_rf_nav = 0; n_rf_nav1 = 0; n_rf_nav2 = 0; n_rf_nav3 = 0;
			n_rf_att = 0; n_rf_att1 = 0; n_rf_att2 = 0; n_rf_att3 = 0;
			n_rf_dup_timestamp = 0;
			n_wf_data = 0; n_wf_comment = 0;
			n_wf_nav = 0; n_wf_nav1 = 0; n_wf_nav2 = 0; n_wf_nav3 = 0;
			n_wf_att = 0; n_wf_att1 = 0; n_wf_att2 = 0; n_wf_att3 = 0;
			start_time_d = -1.0; end_time_d = -1.0;

			if (kluge_fix_wissl_timestamps) kluge_fix_wissl_timestamps_setup2 = false;

			double last_survey_time_d[MB_SUBSENSOR_NUM_MAX];
			memset(last_survey_time_d, 0, MB_SUBSENSOR_NUM_MAX * sizeof(double));
			double time_prior_d = 0.0;

			while (error <= MB_ERROR_NO_ERROR) {
				status = MB_SUCCESS;
				error = MB_ERROR_NO_ERROR;
				bool output_ok = true;

				status = mb_get_all(verbose, imbio_ptr, &istore_ptr, &kind, time_i, &time_d, &navlon_org, &navlat_org, &speed_org,
				                    &heading_org, &distance, &altitude_org, &sensordepth_org, &beams_bath, &beams_amp, &pixels_ss,
				                    beamflag, bath, amp, bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment,
				                    &error);

				if (error < MB_ERROR_NO_ERROR && error > MB_ERROR_UNINTELLIGIBLE) {
					error = MB_ERROR_NO_ERROR;
					status = MB_SUCCESS;
				}

				sensorhead = 0;
				sensortype = 0;
				if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
					int sensorhead_error = MB_ERROR_NO_ERROR;
					mb_sensorhead(verbose, imbio_ptr, istore_ptr, &sensorhead, &sensorhead_error);
					mb_sonartype(verbose, imbio_ptr, istore_ptr, &sensortype, &sensorhead_error);
				}

				if (kind == MB_DATA_DATA) {
					if (n_rf_data == 0) start_time_d = time_d;
					end_time_d = time_d;
					n_rf_data++; n_rt_data++;
				}
				else if (kind == MB_DATA_COMMENT)   { n_rf_comment++; n_rt_comment++; }
				else if (kind == MB_DATA_NAV)       { n_rf_nav++;     n_rt_nav++; }
				else if (kind == MB_DATA_NAV1)      { n_rf_nav1++;    n_rt_nav1++; }
				else if (kind == MB_DATA_NAV2)      { n_rf_nav2++;    n_rt_nav2++; }
				else if (kind == MB_DATA_NAV3)      { n_rf_nav3++;    n_rt_nav3++; }
				else if (kind == MB_DATA_ATTITUDE)  { n_rf_att++;     n_rt_att++; }
				else if (kind == MB_DATA_ATTITUDE1) { n_rf_att1++;    n_rt_att1++; }
				else if (kind == MB_DATA_ATTITUDE2) { n_rf_att2++;    n_rt_att2++; }
				else if (kind == MB_DATA_ATTITUDE3) { n_rf_att3++;    n_rt_att3++; }

				if (status == MB_SUCCESS &&
				    (kind == MB_DATA_DATA || kind == MB_DATA_SUBBOTTOM_MCS || kind == MB_DATA_SUBBOTTOM_CNTRBEAM ||
				     kind == MB_DATA_SUBBOTTOM_SUBBOTTOM || kind == MB_DATA_SIDESCAN2 || kind == MB_DATA_SIDESCAN3 ||
				     kind == MB_DATA_WATER_COLUMN)) {

					status  = mb_extract_nav(verbose, imbio_ptr, istore_ptr, &kind, time_i, &time_d, &navlon_org, &navlat_org,
					                         &speed_org, &heading_org, &draft_org, &roll_org, &pitch_org, &heave_org, &error);
					status &= mb_extract_altitude(verbose, imbio_ptr, istore_ptr, &kind, &sensordepth_org, &altitude_org, &error);

					bool timestamp_changed = false;
					if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
						if (sensorhead >= 0 && sensorhead < MB_SUBSENSOR_NUM_MAX) {
							if (fabs(time_d - last_survey_time_d[sensorhead]) < MB_ESF_MAXTIMEDIFF) {
								time_d += MB_ESF_MAXTIMEDIFF_X10;
								timestamp_changed = true;
								n_rf_dup_timestamp++; n_rt_dup_timestamp++;
							}
							last_survey_time_d[sensorhead] = time_d;
						}
					}

					double dt_expect = 0.0;
					double dt_raw    = 0.0;
					double dt        = 0.0;
					double time_d_raw = 0.0;
					if (kluge_timejumps) {
						if (kind == MB_DATA_DATA) {
							if (n_rt_data == 1) kluge_first_time_d = time_d;
							time_d_raw = time_d;
						}
						if (n_rt_data > 2) {
							dt_expect = (kluge_last_time_d - kluge_first_time_d) / (n_rt_data - 2);
							dt_raw    = time_d - kluge_last_raw_time_d;
							dt        = time_d - kluge_last_time_d;
							if (fabs(dt - dt_expect) >= kluge_timejumps_threshold) {
								if (fabs(dt_raw - dt_expect) >= kluge_timejumps_threshold) {
									time_d = kluge_last_time_d + dt_expect;
									timestamp_changed = true;
								} else {
									time_d = kluge_last_time_d + dt_raw;
									timestamp_changed = true;
								}
							}
						}
						if (kind == MB_DATA_DATA) {
							dt = time_d - kluge_last_time_d;
							kluge_last_time_d = time_d;
							kluge_last_raw_time_d = time_d_raw;
						}
					}

					if (kind == MB_DATA_DATA && iformat == MBF_3DWISSLR && kluge_fix_wissl_timestamps) {
						if (!kluge_fix_wissl_timestamps_setup1) {
							status &= mb_indextablefix(verbose, imbio_ptr, num_indextable, indextable, &error);
							kluge_fix_wissl_timestamps_setup1 = true;
						}
						if (!kluge_fix_wissl_timestamps_setup2) {
							status = mb_indextableapply(verbose, imbio_ptr, num_indextable, indextable, n_rt_files, &error);
							kluge_fix_wissl_timestamps_setup2 = true;
						}
					}

					if (sensor_target != NULL && sensor_target->time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) {
						mb_apply_time_latency(verbose, 1, &time_d, sensor_target->time_latency_mode,
						                      sensor_target->time_latency_static, sensor_target->num_time_latency,
						                      sensor_target->time_latency_time_d, sensor_target->time_latency_value, &error);
						timestamp_changed = true;
					}
					if ((time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) && (time_latency_apply & MBPREPROCESS_TIME_LATENCY_APPLY_SURVEY)) {
						mb_apply_time_latency(verbose, 1, &time_d, time_latency_mode, time_latency_constant, time_latency_num,
						                      time_latency_time_d, time_latency_time_latency, &error);
						timestamp_changed = true;
					}

					bool nav_changed = false;
					if (n_nav > 0) {
						mb_linear_interp_longitude(verbose, nav_time_d - 1, nav_navlon - 1, n_nav, time_d, &navlon_org, &jnav, &interp_error);
						mb_linear_interp_latitude (verbose, nav_time_d - 1, nav_navlat - 1, n_nav, time_d, &navlat_org, &jnav, &interp_error);
						mb_linear_interp          (verbose, nav_time_d - 1, nav_speed  - 1, n_nav, time_d, &speed_org,  &jnav, &interp_error);
						nav_changed = true;
					}
					bool sensordepth_changed = false;
					if (n_sensordepth > 0) {
						mb_linear_interp(verbose, sensordepth_time_d - 1, sensordepth_sensordepth - 1, n_sensordepth,
						                 time_d, &sensordepth_org, &jsensordepth, &interp_error);
						sensordepth_changed = true;
					}
					bool heading_changed = false;
					if (n_heading > 0) {
						mb_linear_interp_heading(verbose, heading_time_d - 1, heading_heading - 1, n_heading, time_d,
						                        &heading_org, &jheading, &interp_error);
						heading_changed = true;
					}
					bool altitude_changed = false;
					if (n_altitude > 0) {
						mb_linear_interp(verbose, altitude_time_d - 1, altitude_altitude - 1, n_altitude, time_d,
						                 &altitude_org, &jaltitude, &interp_error);
						altitude_changed = true;
					}
					bool attitude_changed = false;
					if (n_attitude > 0) {
						mb_linear_interp(verbose, attitude_time_d - 1, attitude_roll  - 1, n_attitude, time_d, &roll_org,  &jattitude, &interp_error);
						mb_linear_interp(verbose, attitude_time_d - 1, attitude_pitch - 1, n_attitude, time_d, &pitch_org, &jattitude, &interp_error);
						mb_linear_interp(verbose, attitude_time_d - 1, attitude_heave - 1, n_attitude, time_d, &heave_org, &jattitude, &interp_error);
						attitude_changed = true;
					}
					if (n_sensordepth > 0 || n_attitude > 0) draft_org = sensordepth_org - heave_org;

					navlon = navlon_org; navlat = navlat_org; speed = speed_org;
					heading = heading_org; altitude = altitude_org; sensordepth = sensordepth_org;
					draft = draft_org; roll = roll_org; pitch = pitch_org; heave = heave_org;

					if (timestamp_changed) mb_get_date(verbose, time_d, time_i);

					preprocess_pars.target_sensor = target_sensor;
					preprocess_pars.timestamp_changed = timestamp_changed;
					preprocess_pars.time_d = time_d;
					preprocess_pars.n_nav = n_nav;
					preprocess_pars.nav_time_d = nav_time_d;
					preprocess_pars.nav_lon = nav_navlon;
					preprocess_pars.nav_lat = nav_navlat;
					preprocess_pars.nav_speed = nav_speed;
					preprocess_pars.n_sensordepth = n_sensordepth;
					preprocess_pars.sensordepth_time_d = sensordepth_time_d;
					preprocess_pars.sensordepth_sensordepth = sensordepth_sensordepth;
					preprocess_pars.n_heading = n_heading;
					preprocess_pars.heading_time_d = heading_time_d;
					preprocess_pars.heading_heading = heading_heading;
					preprocess_pars.n_altitude = n_altitude;
					preprocess_pars.altitude_time_d = altitude_time_d;
					preprocess_pars.altitude_altitude = altitude_altitude;
					preprocess_pars.n_attitude = n_attitude;
					preprocess_pars.attitude_time_d = attitude_time_d;
					preprocess_pars.attitude_roll = attitude_roll;
					preprocess_pars.attitude_pitch = attitude_pitch;
					preprocess_pars.attitude_heave = attitude_heave;
					preprocess_pars.n_soundspeed = n_soundspeed;
					preprocess_pars.soundspeed_time_d = soundspeed_time_d;
					preprocess_pars.soundspeed_soundspeed = soundspeed_soundspeed;

					status = mb_preprocess(verbose, imbio_ptr, istore_ptr, (void *)platform, (void *)&preprocess_pars, &error);

					if (status == MB_FAILURE) {
						status = MB_SUCCESS;
						error = MB_ERROR_NO_ERROR;

						if (platform != NULL) {
							status = mb_platform_position(verbose, (void *)platform, target_sensor, 0, navlon, navlat, sensordepth,
							                              heading, roll, pitch, &navlon, &navlat, &sensordepth, &error);
							draft = sensordepth - heave;
							nav_changed = true; sensordepth_changed = true;

							status = mb_platform_orientation_target(verbose, (void *)platform, target_sensor, 0, heading, roll, pitch,
							                                        &heading, &roll, &pitch, &error);
							roll_delta = roll - roll_org;
							pitch_delta = pitch - pitch_org;
							if (roll_delta != 0.0 || pitch_delta != 0.0) attitude_changed = true;
						}

						if (attitude_changed) {
							for (int i = 0; i < beams_bath; i++) {
								if (beamflag[i] != MB_FLAG_NULL) {
									bath[i] -= sensordepth_org;
									mb_platform_math_attitude_rotate_beam(verbose, bathacrosstrack[i], bathalongtrack[i], bath[i],
									                                     roll_delta, pitch_delta, 0.0, &(bathacrosstrack[i]),
									                                     &(bathalongtrack[i]), &(bath[i]), &error);
									bath[i] += sensordepth_org;
								}
							}
						}

						if (sensordepth_changed) {
							depth_offset_change = draft - draft_org;
							for (int i = 0; i < beams_bath; i++) {
								if (beamflag[i] != MB_FLAG_NULL) bath[i] += depth_offset_change;
							}
						}

						if (timestamp_changed || nav_changed || heading_changed || sensordepth_changed || attitude_changed) {
							status = mb_insert_nav(verbose, imbio_ptr, istore_ptr, time_i, time_d, navlon, navlat, speed, heading,
							                       draft, roll, pitch, heave, &error);
						}

						if (altitude_changed) {
							status = mb_insert_altitude(verbose, imbio_ptr, istore_ptr, sensordepth, altitude, &error);
							if (status == MB_FAILURE) { status = MB_SUCCESS; error = MB_ERROR_NO_ERROR; }
						}

						if (!preprocess_pars.no_change_survey && (attitude_changed || sensordepth_changed)) {
							status = mb_insert(verbose, imbio_ptr, istore_ptr, kind, time_i, time_d, navlon, navlat, speed, heading,
							                   beams_bath, beams_amp, pixels_ss, beamflag, bath, amp, bathacrosstrack, bathalongtrack,
							                   ss, ssacrosstrack, ssalongtrack, comment, &error);
						}
					}
				}

				if (kluge_ignore_duplicate_pings && kind == MB_DATA_DATA) {
					if (fabs(time_d - time_prior_d) < MB_ESF_MAXTIMEDIFF) {
						output_ok = false;
						GMT_Report(API, GMT_MSG_INFORMATION, "Kluge ignore duplicate pings - duplicate ping skipped - Timestamps: %.6f %.6f\n", time_d, time_prior_d);
					}
				}
				if (kind == MB_DATA_DATA) time_prior_d = time_d;

				if (error == MB_ERROR_NO_ERROR && output_ok) {
					status = mb_put_all(verbose, ombio_ptr, istore_ptr, false, kind, time_i, time_d, navlon, navlat, speed, heading,
					                    obeams_bath, obeams_amp, opixels_ss, beamflag, bath, amp, bathacrosstrack, bathalongtrack, ss,
					                    ssacrosstrack, ssalongtrack, comment, &error);
					if (status != MB_SUCCESS) {
						char *message; mb_error(verbose, error, &message);
						GMT_Report(API, GMT_MSG_NORMAL, "%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
						GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error returned from function <mb_put_all>:\n%s\n", message);
						GMT_Report(API, GMT_MSG_NORMAL, "\nMultibeam Data Not Written To File <%s>\n", ofile);
						FAIL(error);
					}

					if (kind == MB_DATA_DATA) {
						status = mb_extract(verbose, ombio_ptr, istore_ptr, &kind, time_i, &time_d,
						                    &navlon, &navlat, &speed, &heading,
						                    &obeams_bath, &obeams_amp, &opixels_ss,
						                    beamflag, bath, amp, bathacrosstrack, bathalongtrack,
						                    ss, ssacrosstrack, ssalongtrack, comment, &error);
						status = mb_extract_nav(verbose, ombio_ptr, istore_ptr, &kind, time_i, &time_d,
						                        &navlon, &navlat, &speed, &heading, &draft, &roll, &pitch, &heave, &error);
						status = mb_extract_altitude(verbose, ombio_ptr, istore_ptr, &kind, &sensordepth, &altitude, &error);

						if (make_fbt) {
							fstore->sensorhead = sensorhead;
							fstore->topo_type = sensortype;
							struct mb_io_struct *imb_io_ptr = (struct mb_io_struct *)imbio_ptr;
							fstore->beam_xwidth = imb_io_ptr->beamwidth_xtrack;
							fstore->beam_lwidth = imb_io_ptr->beamwidth_ltrack;
							fstore->kind = kind;
							mb_insert_nav(verbose, fmbio_ptr, fstore_ptr, time_i, time_d, navlon, navlat, speed, heading, draft, roll, pitch, heave, &error);
							mb_insert_altitude(verbose, fmbio_ptr, fstore_ptr, draft, altitude, &error);
							status  = mb_insert(verbose, fmbio_ptr, fstore_ptr, kind, time_i, time_d,
							                    navlon, navlat, speed, heading, obeams_bath, obeams_amp, opixels_ss,
							                    beamflag, bath, amp, bathacrosstrack, bathalongtrack,
							                    ss, ssacrosstrack, ssalongtrack, comment, &error);
							status &= mb_put_all(verbose, fmbio_ptr, fstore_ptr, false,
							                    kind, time_i, time_d, navlon, navlat, speed,
							                    heading, obeams_bath, 0, 0,
							                    beamflag, bath, NULL, bathacrosstrack, bathalongtrack,
							                    NULL, NULL, NULL, comment, &error);
						}

						double mtodeglon, mtodeglat;
						mb_coor_scale(verbose, navlat, &mtodeglon, &mtodeglat);
						const double headingx = sin(heading * DTR);
						const double headingy = cos(heading * DTR);

						if (make_fnv) {
							double seconds = time_i[5] + 1e-6 * time_i[6];
							int beam_port, beam_vertical, beam_stbd;
							int pixel_port, pixel_vertical, pixel_stbd;
							status = mb_swathbounds(verbose, true, obeams_bath, 0, beamflag, bathacrosstrack, NULL, NULL,
							                        &beam_port, &beam_vertical, &beam_stbd,
							                        &pixel_port, &pixel_vertical, &pixel_stbd, &error);
							const double portlon = navlon + headingy * mtodeglon * bathacrosstrack[beam_port] + headingx * mtodeglon * bathalongtrack[beam_port];
							const double portlat = navlat - headingx * mtodeglat * bathacrosstrack[beam_port] + headingy * mtodeglat * bathalongtrack[beam_port];
							const double stbdlon = navlon + headingy * mtodeglon * bathacrosstrack[beam_stbd] + headingx * mtodeglon * bathalongtrack[beam_stbd];
							const double stbdlat = navlat - headingx * mtodeglat * bathacrosstrack[beam_stbd] + headingy * mtodeglat * bathalongtrack[beam_stbd];

							fprintf(nfp,
							        "%.4d %.2d %.2d %.2d %.2d %09.6f\t%.6f\t"
							        "%15.10f\t%15.10f\t%7.3f\t%6.3f\t%.4f\t%6.3f\t%6.3f\t%7.4f\t"
							        "%15.10f\t%15.10f\t%15.10f\t%15.10f\n",
							        time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], seconds,
							        time_d, navlon, navlat, heading, speed, sensordepth, roll, pitch, heave,
							        portlon, portlat, stbdlon, stbdlat);
						}

						if (fabs(navlon) >= 0.005 || fabs(navlat) >= 0.005) {
							if (mask_bounds_init) {
								mask_bounds[0] = MIN(mask_bounds[0], navlon);
								mask_bounds[1] = MAX(mask_bounds[1], navlon);
								mask_bounds[2] = MIN(mask_bounds[2], navlat);
								mask_bounds[3] = MAX(mask_bounds[3], navlat);
							} else {
								mask_bounds[0] = navlon; mask_bounds[1] = navlon;
								mask_bounds[2] = navlat; mask_bounds[3] = navlat;
								mask_bounds_init = true;
							}
							for (int i = 0; i < obeams_bath; i++) {
								if (mb_beam_ok(beamflag[i])) {
									double bathlon = navlon + headingy * mtodeglon * bathacrosstrack[i] + headingx * mtodeglon * bathalongtrack[i];
									double bathlat = navlat - headingx * mtodeglat * bathacrosstrack[i] + headingy * mtodeglat * bathalongtrack[i];
									mask_bounds[0] = MIN(mask_bounds[0], bathlon);
									mask_bounds[1] = MAX(mask_bounds[1], bathlon);
									mask_bounds[2] = MIN(mask_bounds[2], bathlat);
									mask_bounds[3] = MAX(mask_bounds[3], bathlat);
								}
							}
							for (int i = 0; i < opixels_ss; i++) {
								if (ss[i] > MB_SIDESCAN_NULL) {
									double sslon = navlon + headingy * mtodeglon * ssacrosstrack[i] + headingx * mtodeglon * ssalongtrack[i];
									double sslat = navlat - headingx * mtodeglat * ssacrosstrack[i] + headingy * mtodeglat * ssalongtrack[i];
									mask_bounds[0] = MIN(mask_bounds[0], sslon);
									mask_bounds[1] = MAX(mask_bounds[1], sslon);
									mask_bounds[2] = MIN(mask_bounds[2], sslat);
									mask_bounds[3] = MAX(mask_bounds[3], sslat);
								}
							}
						}

						int index = 0;
						mb_put_binary_double(true, time_d,        &buffer[index]); index += 8;
						mb_put_binary_float (true, (float)roll,   &buffer[index]); index += 4;
						mb_put_binary_float (true, (float)pitch,  &buffer[index]); index += 4;
						fwrite(buffer, (size_t)index, 1, afp);
					}

					if      (kind == MB_DATA_DATA)      { n_wf_data++;    n_wt_data++; }
					else if (kind == MB_DATA_COMMENT)   { n_wf_comment++; n_wt_comment++; }
					else if (kind == MB_DATA_NAV)       { n_wf_nav++;     n_wt_nav++; }
					else if (kind == MB_DATA_NAV1)      { n_wf_nav1++;    n_wt_nav1++; }
					else if (kind == MB_DATA_NAV2)      { n_wf_nav2++;    n_wt_nav2++; }
					else if (kind == MB_DATA_NAV3)      { n_wf_nav3++;    n_wt_nav3++; }
					else if (kind == MB_DATA_ATTITUDE)  { n_wf_att++;     n_wt_att++; }
					else if (kind == MB_DATA_ATTITUDE1) { n_wf_att1++;    n_wt_att1++; }
					else if (kind == MB_DATA_ATTITUDE2) { n_wf_att2++;    n_wt_att2++; }
					else if (kind == MB_DATA_ATTITUDE3) { n_wf_att3++;    n_wt_att3++; }
				}

				if (output_sensor_fnv && status == MB_SUCCESS && kind == MB_DATA_DATA) {
					for (isensor = 0; isensor < platform->num_sensors; isensor++) {
						for (ioffset = 0; ioffset < platform->sensors[isensor].num_offsets; ioffset++) {
							if (platform->sensors[isensor].offsets[ioffset].ofp != NULL) {
								status = mb_platform_position(verbose, (void *)platform, isensor, ioffset, navlon_org, navlat_org,
								                              sensordepth_org, heading_org, roll_org, pitch_org, &navlon, &navlat,
								                              &sensordepth, &error);
								draft = sensordepth - heave;
								status &= mb_platform_orientation_target(verbose, (void *)platform, isensor, ioffset, heading_org,
								                                         roll_org, pitch_org, &heading, &roll, &pitch, &error);
								fprintf(platform->sensors[isensor].offsets[ioffset].ofp,
								        "%4.4d %2.2d %2.2d %2.2d %2.2d "
								        "%2.2d.%6.6d\t%.6f\t%.10f\t%.10f\t%.3f\t%.3f\t%.4f\t%.3f\t%.3f\t%.3f\n",
								        time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], time_d,
								        navlon, navlat, heading, speed, draft, roll, pitch, heave);
							}
						}
					}
				}
			}
			/* end record loop */

			if (verbose > 0) {
				GMT_Report(API, GMT_MSG_INFORMATION, "Pass 2: Records read from input file %d: %s\n", n_rt_files, ifile);
				GMT_Report(API, GMT_MSG_INFORMATION, "     %d survey records\n", n_rf_data);
				GMT_Report(API, GMT_MSG_INFORMATION, "     %d comment records\n", n_rf_comment);
				GMT_Report(API, GMT_MSG_INFORMATION, "     %d nav records\n", n_rf_nav);
				GMT_Report(API, GMT_MSG_INFORMATION, "     %d att records\n", n_rf_att);
				if (n_rf_dup_timestamp > 0)
					GMT_Report(API, GMT_MSG_INFORMATION, "     %d duplicate timestamps fixed ****\n", n_rf_dup_timestamp);
				GMT_Report(API, GMT_MSG_INFORMATION, "Pass 2: Records written to output file %d: %s\n", n_wt_files, ofile);
				GMT_Report(API, GMT_MSG_INFORMATION, "     %d survey records\n", n_wf_data);
				GMT_Report(API, GMT_MSG_INFORMATION, "     %d comment records\n", n_wf_comment);
				GMT_Report(API, GMT_MSG_INFORMATION, "     %d nav records\n", n_wf_nav);
				GMT_Report(API, GMT_MSG_INFORMATION, "     %d att records\n", n_wf_att);
			}

			status = mb_close(verbose, &imbio_ptr, &error);
			n_rt_files++;
			status &= mb_close(verbose, &ombio_ptr, &error);
			n_wt_files++;

			if (make_fbt) status = mb_close(verbose, &fmbio_ptr, &error);
			if (make_fnv) fclose(nfp);

			char command[MB_PATH_MAXLINE+100];
			snprintf(command, sizeof(command), "mbinfo -F %d -I %s -G -N -O -M10/10/%.9f/%.9f/%.9f/%.9f",
			         oformat, ofile, mask_bounds[0], mask_bounds[1], mask_bounds[2], mask_bounds[3]);
			system(command);

			fclose(afp);

			if (status == MB_SUCCESS) {
				/* generate asynchronous heading file */
				if (n_heading > 0) {
					istart = 0; iend = 0;
					for (int i = 0; i < n_heading; i++) {
						if (heading_time_d[i] < start_time_d - 10.0) istart = i;
						if (heading_time_d[i] < end_time_d + 10.0)   iend = i;
					}
					if (iend < n_heading - 1) iend++;
					if (iend > istart) {
						snprintf(afile, sizeof(afile), "%s.bah", ofile);
						if ((afp = fopen(afile, "wb")) == NULL) {
							GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open asynchronous heading data file <%s> for writing\n", afile);
							FAIL(MB_ERROR_OPEN_FAIL);
						}
						if (verbose > 0) GMT_Report(API, GMT_MSG_INFORMATION, "Generating bah file for %s using samples %d:%d out of %d\n", ofile, istart, iend, n_heading);
						for (int i = istart; i < iend; i++) {
							int index = 0;
							mb_put_binary_double(true, heading_time_d[i], &buffer[index]); index += 8;
							mb_put_binary_float (true, (float)heading_heading[i], &buffer[index]); index += 4;
							fwrite(buffer, (size_t)index, 1, afp);
						}
						fclose(afp);
					}
				}

				/* generate asynchronous sensordepth file */
				if (n_sensordepth > 0) {
					istart = 0; iend = 0;
					for (int i = 0; i < n_sensordepth; i++) {
						if (sensordepth_time_d[i] < start_time_d - 10.0) istart = i;
						if (sensordepth_time_d[i] < end_time_d + 10.0)   iend = i;
					}
					if (iend < n_sensordepth - 1) iend++;
					if (iend > istart) {
						snprintf(afile, sizeof(afile), "%s.bas", ofile);
						if ((afp = fopen(afile, "wb")) == NULL) {
							GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open asynchronous sensordepth data file <%s> for writing\n", afile);
							FAIL(MB_ERROR_OPEN_FAIL);
						}
						if (verbose > 0) GMT_Report(API, GMT_MSG_INFORMATION, "Generating bas file for %s using samples %d:%d out of %d\n", ofile, istart, iend, n_sensordepth);
						for (int i = istart; i < iend; i++) {
							int index = 0;
							mb_put_binary_double(true, sensordepth_time_d[i], &buffer[index]); index += 8;
							mb_put_binary_float (true, (float)sensordepth_sensordepth[i], &buffer[index]); index += 4;
							fwrite(buffer, (size_t)index, 1, afp);
						}
						fclose(afp);
					}
				}

				/* generate asynchronous attitude file */
				if (n_attitude > 0) {
					istart = 0; iend = 0;
					for (int i = 0; i < n_attitude; i++) {
						if (attitude_time_d[i] < start_time_d - 10.0) istart = i;
						if (attitude_time_d[i] < end_time_d + 10.0)   iend = i;
					}
					if (iend < n_attitude - 1) iend++;
					if (iend > istart) {
						snprintf(afile, sizeof(afile), "%s.baa", ofile);
						if ((afp = fopen(afile, "wb")) == NULL) {
							GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open asynchronous attitude data file <%s> for writing\n", afile);
							FAIL(MB_ERROR_OPEN_FAIL);
						}
						if (verbose > 0) GMT_Report(API, GMT_MSG_INFORMATION, "Generating baa file for %s using samples %d:%d out of %d\n", ofile, istart, iend, n_attitude);
						for (int i = istart; i < iend; i++) {
							int index = 0;
							mb_put_binary_double(true, attitude_time_d[i], &buffer[index]); index += 8;
							mb_put_binary_float (true, (float)attitude_roll[i],  &buffer[index]); index += 4;
							mb_put_binary_float (true, (float)attitude_pitch[i], &buffer[index]); index += 4;
							fwrite(buffer, (size_t)index, 1, afp);
						}
						fclose(afp);
					}
				}
			}
		}

		if (read_datalist) {
			if (mb_datalist_read(verbose, datalist, ifile, dfile, &format, &file_weight, &error) == MB_SUCCESS) read_data = true;
			else read_data = false;
		} else read_data = false;
	}
	if (read_datalist) mb_datalist_close(verbose, &datalist, &error);

	if (verbose > 0) {
		GMT_Report(API, GMT_MSG_INFORMATION, "\nPass 2: Total records read from %d input files\n", n_rt_files);
		GMT_Report(API, GMT_MSG_INFORMATION, "     %d survey records\n", n_rt_data);
		GMT_Report(API, GMT_MSG_INFORMATION, "     %d comment records\n", n_rt_comment);
		GMT_Report(API, GMT_MSG_INFORMATION, "     %d nav records\n", n_rt_nav);
		GMT_Report(API, GMT_MSG_INFORMATION, "     %d att records\n", n_rt_att);
		if (n_rt_dup_timestamp > 0)
			GMT_Report(API, GMT_MSG_INFORMATION, "     %d duplicate timestamps fixed ****\n", n_rt_dup_timestamp);
		GMT_Report(API, GMT_MSG_INFORMATION, "Pass 2: Total records written to %d output files\n", n_wt_files);
		GMT_Report(API, GMT_MSG_INFORMATION, "     %d survey records\n", n_wt_data);
		GMT_Report(API, GMT_MSG_INFORMATION, "     %d comment records\n", n_wt_comment);
	}

	if (output_sensor_fnv && platform != NULL) {
		for (isensor = 0; isensor < platform->num_sensors; isensor++) {
			for (ioffset = 0; ioffset < platform->sensors[isensor].num_offsets; ioffset++) {
				if (platform->sensors[isensor].offsets[ioffset].ofp != NULL) {
					fclose(platform->sensors[isensor].offsets[ioffset].ofp);
					platform->sensors[isensor].offsets[ioffset].ofp = NULL;
				}
			}
		}
	}

	/* === SECTION_FINAL_DBG2_AND_DEALLOC === */

cleanup:
	if (n_nav_alloc > 0) {
		mb_freed(verbose, __FILE__, __LINE__, (void **)&nav_time_d, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&nav_navlon, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&nav_navlat, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&nav_speed, &error);
	}
	if (n_sensordepth_alloc > 0) {
		mb_freed(verbose, __FILE__, __LINE__, (void **)&sensordepth_time_d, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&sensordepth_sensordepth, &error);
	}
	if (n_heading_alloc > 0) {
		mb_freed(verbose, __FILE__, __LINE__, (void **)&heading_time_d, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&heading_heading, &error);
	}
	if (n_altitude_alloc > 0) {
		mb_freed(verbose, __FILE__, __LINE__, (void **)&altitude_time_d, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&altitude_altitude, &error);
	}
	if (n_attitude_alloc > 0) {
		mb_freed(verbose, __FILE__, __LINE__, (void **)&attitude_time_d, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&attitude_roll, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&attitude_pitch, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&attitude_heave, &error);
	}
	if (n_soundspeed_alloc > 0) {
		mb_freed(verbose, __FILE__, __LINE__, (void **)&soundspeed_time_d, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&soundspeed_soundspeed, &error);
	}
	if (time_latency_alloc > 0) {
		mb_freed(verbose, __FILE__, __LINE__, (void **)&time_latency_time_d, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&time_latency_time_latency, &error);
	}
	if (num_indextable_alloc > 0) {
		mb_freed(verbose, __FILE__, __LINE__, (void **)&indextable, &error);
	}
	if (platform != NULL) {
		mb_platform_deall(verbose, (void **)&platform, &error);
	}
	if (verbose >= 4) mb_memory_list(verbose, &error);

	if (verbose >= 2) {
		GMT_Report(API, GMT_MSG_INFORMATION, "\ndbg2  Program <%s> completed\n", program_name);
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2  Ending status:\n");
		GMT_Report(API, GMT_MSG_INFORMATION, "dbg2       status:  %d\n", status);
	}
	Return(error);
}
/*--------------------------------------------------------------------*/
