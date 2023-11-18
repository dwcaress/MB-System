/*--------------------------------------------------------------------
 *    The MB-system:  mbpreprocess.c  1/8/2014
 *
 *    Copyright (c) 2014-2023 by
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
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_process.h"
#include "mb_status.h"
#include "mbsys_ldeoih.h"

constexpr int MBPREPROCESS_ALLOC_CHUNK = 1000;

typedef enum {
    MBPREPROCESS_MERGE_OFF = 0,
    MBPREPROCESS_MERGE_FILE = 1,
    MBPREPROCESS_MERGE_ASYNC = 2,
} merge_t;

constexpr mb_u_char MBPREPROCESS_TIME_LATENCY_APPLY_NONE = 0x00;
constexpr mb_u_char MBPREPROCESS_TIME_LATENCY_APPLY_NAV = 0x01;
constexpr mb_u_char MBPREPROCESS_TIME_LATENCY_APPLY_SENSORDEPTH = 0x02;
constexpr mb_u_char MBPREPROCESS_TIME_LATENCY_APPLY_ALTITUDE = 0x04;
constexpr mb_u_char MBPREPROCESS_TIME_LATENCY_APPLY_HEADING = 0x08;
constexpr mb_u_char MBPREPROCESS_TIME_LATENCY_APPLY_ATTITUDE = 0x10;
constexpr mb_u_char MBPREPROCESS_TIME_LATENCY_APPLY_SOUNDSPEED = 0x20;
constexpr mb_u_char MBPREPROCESS_TIME_LATENCY_APPLY_UNUSED = 0x40;
constexpr mb_u_char MBPREPROCESS_TIME_LATENCY_APPLY_ALL_ANCILLIARY = 0x7F;
constexpr mb_u_char MBPREPROCESS_TIME_LATENCY_APPLY_SURVEY = 0x80;
constexpr mb_u_char MBPREPROCESS_TIME_LATENCY_APPLY_ALL = 0xFF;

// #define DEBUG_MBPREPROCESS 1

constexpr char program_name[] = "mbpreprocess";
constexpr char help_message[] =
    "mbpreprocess handles preprocessing of swath sonar data as part of setting up an MB-System processing "
    "structure for a dataset.\n";
constexpr char usage_message[] =
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
    "\t--kluge-ancilliary-time-jumps=threshold\n"
    "\t--kluge-mbaripressure-time-jumps=threshold\n"
    "\t--kluge-beam-tweak=factor\n"
    "\t--kluge-soundspeed-tweak=factor\n"
    "\t--kluge-zero-attitude-correction\n"
    "\t--kluge-zero-alongtrack-angles\n"
    "\t--kluge-fix-wissl-timestamps\n"
    "\t--kluge-auv-sentry-sensordepth\n"
    "\t--kluge-ignore-snippets\n"
    "\t--kluge-sensordepth-from-heave\n";

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
  int verbose = 0;
  int format = 0;
  int pings;
  int lonflip;
  double bounds[4];
  int btime_i[7];
  int etime_i[7];
  double speedmin;
  double timegap;
  int status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

  int error = MB_ERROR_NO_ERROR;
  bool use_platform_file = false;
  int target_sensor = -1;
  mb_path nav_file = "";

  /* output fnv files for each sensor */
  bool output_sensor_fnv = false;
  bool skip_existing = false;  // output files

  double kluge_beamtweak_factor = 1.0;
  bool kluge_beamtweak = false;
  double kluge_soundspeedtweak_factor = 1.0;
  bool kluge_soundspeedtweak = false;
  bool kluge_fix_wissl_timestamps = false;
  bool kluge_auv_sentry_sensordepth = false;
  bool kluge_ignore_snippets = false;
  bool kluge_sensordepth_from_heave = false;

  mb_path sensordepth_file;
  memset(sensordepth_file, 0, sizeof(mb_path));
  mb_path heading_file = "";
  memset(heading_file, 0, sizeof(mb_path));
  mb_path altitude_file = "";
  memset(altitude_file, 0, sizeof(mb_path));
  mb_path attitude_file = "";
  memset(attitude_file, 0, sizeof(mb_path));
  mb_path soundspeed_file = "";
  memset(soundspeed_file, 0, sizeof(mb_path));
  mb_path time_latency_file = "";
  memset(time_latency_file, 0, sizeof(mb_path));
  mb_path platform_file = "";
  memset(platform_file, 0, sizeof(mb_path));
  mb_path read_file = "datalist.mb-1";
  memset(read_file, 0, sizeof(mb_path));
  bool output_directory_set = false;
  mb_path output_directory = "";
  memset(output_directory, 0, sizeof(mb_path));
  bool output_datalist_set = false;
  char output_datalist[MB_PATH_MAXLINE+100] = "datalist.mb-1";
  struct mb_preprocess_struct preprocess_pars;
  memset(&preprocess_pars, 0, sizeof(struct mb_preprocess_struct));

  merge_t nav_mode = MBPREPROCESS_MERGE_OFF;
  int nav_file_format = 0;
  int nav_async = MB_DATA_DATA;
  int nav_sensor = -1;
  merge_t sensordepth_mode = MBPREPROCESS_MERGE_OFF;
  int sensordepth_file_format = 0;
  int sensordepth_async = MB_DATA_DATA;
  int sensordepth_sensor = -1;
  merge_t heading_mode = MBPREPROCESS_MERGE_OFF;
  int heading_file_format = 0;
  int heading_async = MB_DATA_DATA;
  int heading_sensor = -1;
  merge_t altitude_mode = MBPREPROCESS_MERGE_OFF;
  int altitude_file_format = 0;
  int altitude_async = MB_DATA_DATA;
  int altitude_sensor = -1;
  merge_t attitude_mode = MBPREPROCESS_MERGE_OFF;
  int attitude_file_format = 0;
  int attitude_async = MB_DATA_DATA;
  int attitude_sensor = -1;
  merge_t soundspeed_mode = MBPREPROCESS_MERGE_OFF;
  int soundspeed_file_format = 0;
  int soundspeed_async = MB_DATA_DATA;
  int soundspeed_sensor = -1;
  int time_latency_mode = MB_SENSOR_TIME_LATENCY_NONE;
  int time_latency_format = 1;
  double time_latency_constant = 0.0;
  mb_u_char time_latency_apply = MBPREPROCESS_TIME_LATENCY_APPLY_NONE;
  double filter_length = 0.0;
  mb_u_char filter_apply = MBPREPROCESS_TIME_LATENCY_APPLY_NONE;
  double kluge_timejumps_threshold = 0.0;
  bool kluge_timejumps = false;
  double kluge_timejumps_anc_threshold = 0.0;
  bool kluge_timejumps_ancilliary = false;
  double kluge_timejumps_mba_threshold = 0.0;
  bool kluge_timejumps_mbaripressure = false;

  {
    static struct option options[] = {{"verbose", no_argument, nullptr, 0},
                                      {"help", no_argument, nullptr, 0},
                                      {"input", required_argument, nullptr, 0},
                                      {"format", required_argument, nullptr, 0},
                                      {"output-directory", required_argument, nullptr, 0},
                                      {"platform-file", required_argument, nullptr, 0},
                                      {"platform-target-sensor", required_argument, nullptr, 0},
                                      {"output-sensor-fnv", no_argument, nullptr, 0},
                                      {"skip-existing", no_argument, nullptr, 0},
                                      {"nav-file", required_argument, nullptr, 0},
                                      {"nav-file-format", required_argument, nullptr, 0},
                                      {"nav-async", required_argument, nullptr, 0},
                                      {"nav-sensor", required_argument, nullptr, 0},
                                      {"sensordepth-file", required_argument, nullptr, 0},
                                      {"sensordepth-file-format", required_argument, nullptr, 0},
                                      {"sensordepth-async", required_argument, nullptr, 0},
                                      {"sensordepth-sensor", required_argument, nullptr, 0},
                                      {"heading-file", required_argument, nullptr, 0},
                                      {"heading-file-format", required_argument, nullptr, 0},
                                      {"heading-async", required_argument, nullptr, 0},
                                      {"heading-sensor", required_argument, nullptr, 0},
                                      {"altitude-file", required_argument, nullptr, 0},
                                      {"altitude-file-format", required_argument, nullptr, 0},
                                      {"altitude-async", required_argument, nullptr, 0},
                                      {"altitude-sensor", required_argument, nullptr, 0},
                                      {"attitude-file", required_argument, nullptr, 0},
                                      {"attitude-file-format", required_argument, nullptr, 0},
                                      {"attitude-async", required_argument, nullptr, 0},
                                      {"attitude-sensor", required_argument, nullptr, 0},
                                      {"soundspeed-file", required_argument, nullptr, 0},
                                      {"soundspeed-file-format", required_argument, nullptr, 0},
                                      {"soundspeed-async", required_argument, nullptr, 0},
                                      {"soundspeed-sensor", required_argument, nullptr, 0},
                                      {"time-latency-file", required_argument, nullptr, 0},
                                      {"time-latency-file-format", required_argument, nullptr, 0},
                                      {"time-latency-constant", required_argument, nullptr, 0},
                                      {"time-latency-apply-nav", no_argument, nullptr, 0},
                                      {"time-latency-apply-sensordepth", no_argument, nullptr, 0},
                                      {"time-latency-apply-heading", no_argument, nullptr, 0},
                                      {"time-latency-apply-attitude", no_argument, nullptr, 0},
                                      {"time-latency-apply-all-ancilliary", no_argument, nullptr, 0},
                                      {"time-latency-apply-survey", no_argument, nullptr, 0},
                                      {"time-latency-apply-all", no_argument, nullptr, 0},
                                      {"time-latency-apply-nav", no_argument, nullptr, 0},
                                      {"filter", required_argument, nullptr, 0},
                                      {"filter-apply-nav", no_argument, nullptr, 0},
                                      {"filter-apply-sensordepth", no_argument, nullptr, 0},
                                      {"filter-apply-heading", no_argument, nullptr, 0},
                                      {"filter-apply-attitude", no_argument, nullptr, 0},
                                      {"filter-apply-all-ancilliary", no_argument, nullptr, 0},
                                      {"recalculate-bathymetry", no_argument, nullptr, 0},
                                      {"no-change-survey", no_argument, nullptr, 0},
                                      {"multibeam-sidescan-source", required_argument, nullptr, 0},
                                      {"sounding-amplitude-filter", required_argument, nullptr, 0},
                                      {"sounding-altitude-filter", required_argument, nullptr, 0},
                                      {"ignore-water-column", no_argument, nullptr, 0},
                                      {"head1-offsets", required_argument, nullptr, 0},
                                      {"head2-offsets", required_argument, nullptr, 0},
                                      {"kluge-time-jumps", required_argument, nullptr, 0},
                                      {"kluge-ancilliary-time-jumps", required_argument, nullptr, 0},
                                      {"kluge-mbaripressure-time-jumps", required_argument, nullptr, 0},
                                      {"kluge-beam-tweak", required_argument, nullptr, 0},
                                      {"kluge-soundspeed-tweak", required_argument, nullptr, 0},
                                      {"kluge-zero-attitude-correction", no_argument, nullptr, 0},
                                      {"kluge-zero-alongtrack-angles", no_argument, nullptr, 0},
                                      {"kluge-fix-wissl-timestamps", no_argument, nullptr, 0},
                                      {"kluge-auv-sentry-sensordepth", no_argument, nullptr, 0},
                                      {"kluge-ignore-snippets", no_argument, nullptr, 0},
                                      {"kluge-sensordepth-from-heave", no_argument, nullptr, 0},
                                      {nullptr, 0, nullptr, 0}};

    int option_index;
    bool errflg = false;
    int c;
    bool help = false;
    while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1)
      switch (c) {
      /* long options all return c=0 */
      case 0:
        if (strcmp("verbose", options[option_index].name) == 0) {
          verbose++;
        }
        else if (strcmp("help", options[option_index].name) == 0) {
          help = true;
        }
        /*-------------------------------------------------------
         * Define input file and format (usually a datalist) */
        else if (strcmp("input", options[option_index].name) == 0) {
          strcpy(read_file, optarg);
        }
        else if (strcmp("format", options[option_index].name) == 0) {
          /* n = */ sscanf(optarg, "%d", &format);
        }
        else if (strcmp("output-directory", options[option_index].name) == 0) {
          const int n = sscanf(optarg, "%1023s", output_directory);
          if (n == 1 && strlen(output_directory) > 0)
            output_directory_set = true;
        }
        else if (strcmp("output-datalist", options[option_index].name) == 0) {
          const int n = sscanf(optarg, "%1023s", output_datalist);
          if (n == 1 && strlen(output_datalist) > 0)
            output_datalist_set = true;
        }
        else if (strcmp("platform-file", options[option_index].name) == 0) {
          const int n = sscanf(optarg, "%1023s", platform_file);
          if (n == 1 && strlen(platform_file) > 0)
            use_platform_file = true;
        }
        else if (strcmp("platform-target-sensor", options[option_index].name) == 0) {
          /* n = */ sscanf(optarg, "%d", &target_sensor);
        }
        else if (strcmp("output-sensor-fnv", options[option_index].name) == 0) {
          output_sensor_fnv = true;
        }
        else if (strcmp("skip-existing", options[option_index].name) == 0) {
          skip_existing = true;
        }
        /*-------------------------------------------------------
         * Define source of navigation - could be an external file
         * or an internal asynchronous record */
        else if (strcmp("nav-file", options[option_index].name) == 0) {
          strcpy(nav_file, optarg);
          nav_mode = MBPREPROCESS_MERGE_FILE;
          preprocess_pars.recalculate_bathymetry = true;
        }
        else if (strcmp("nav-file-format", options[option_index].name) == 0) {
          /* n = */ sscanf(optarg, "%d", &nav_file_format);
        }
        else if (strcmp("nav-async", options[option_index].name) == 0) {
          const int n = sscanf(optarg, "%d", &nav_async);
          if (n == 1)
            nav_mode = MBPREPROCESS_MERGE_ASYNC;
          preprocess_pars.recalculate_bathymetry = true;
        }
        else if (strcmp("nav-sensor", options[option_index].name) == 0) {
          /* n = */ sscanf(optarg, "%d", &nav_sensor);
          preprocess_pars.recalculate_bathymetry = true;
        }
        /*-------------------------------------------------------
         * Define source of sensordepth - could be an external file
         * or an internal asynchronous record */
        else if (strcmp("sensordepth-file", options[option_index].name) == 0) {
          strcpy(sensordepth_file, optarg);
          sensordepth_mode = MBPREPROCESS_MERGE_FILE;
          preprocess_pars.recalculate_bathymetry = true;
        }
        else if (strcmp("sensordepth-file-format", options[option_index].name) == 0) {
          /* n = */ sscanf(optarg, "%d", &sensordepth_file_format);
        }
        else if (strcmp("sensordepth-async", options[option_index].name) == 0) {
          const int n = sscanf(optarg, "%d", &sensordepth_async);
          if (n == 1)
            sensordepth_mode = MBPREPROCESS_MERGE_ASYNC;
          preprocess_pars.recalculate_bathymetry = true;
        }
        else if (strcmp("sensordepth-sensor", options[option_index].name) == 0) {
          /* n = */ sscanf(optarg, "%d", &sensordepth_sensor);
          preprocess_pars.recalculate_bathymetry = true;
        }
        /*-------------------------------------------------------
         * Define source of heading - could be an external file
         * or an internal asynchronous record */
        else if (strcmp("heading-file", options[option_index].name) == 0) {
          strcpy(heading_file, optarg);
          heading_mode = MBPREPROCESS_MERGE_FILE;
          preprocess_pars.recalculate_bathymetry = true;
        }
        else if (strcmp("heading-file-format", options[option_index].name) == 0) {
          /* n = */ sscanf(optarg, "%d", &heading_file_format);
        }
        else if (strcmp("heading-async", options[option_index].name) == 0) {
          const int n = sscanf(optarg, "%d", &heading_async);
          if (n == 1)
            heading_mode = MBPREPROCESS_MERGE_ASYNC;
          preprocess_pars.recalculate_bathymetry = true;
        }
        else if (strcmp("heading-sensor", options[option_index].name) == 0) {
          /* n = */ sscanf(optarg, "%d", &heading_sensor);
          preprocess_pars.recalculate_bathymetry = true;
        }
        /*-------------------------------------------------------
         * Define source of altitude - could be an external file
         * or an internal asynchronous record */
        else if (strcmp("altitude-file", options[option_index].name) == 0) {
          strcpy(altitude_file, optarg);
          altitude_mode = MBPREPROCESS_MERGE_FILE;
        }
        else if (strcmp("altitude-file-format", options[option_index].name) == 0) {
          /* n = */ sscanf(optarg, "%d", &altitude_file_format);
        }
        else if (strcmp("altitude-async", options[option_index].name) == 0) {
          const int n = sscanf(optarg, "%d", &altitude_async);
          if (n == 1)
            altitude_mode = MBPREPROCESS_MERGE_ASYNC;
        }
        else if (strcmp("altitude-sensor", options[option_index].name) == 0) {
          /* n = */ sscanf(optarg, "%d", &altitude_sensor);
        }
        /*-------------------------------------------------------
         * Define source of attitude - could be an external file
         * or an internal asynchronous record */
        else if (strcmp("attitude-file", options[option_index].name) == 0) {
          strcpy(attitude_file, optarg);
          attitude_mode = MBPREPROCESS_MERGE_FILE;
          preprocess_pars.recalculate_bathymetry = true;
        }
        else if (strcmp("attitude-file-format", options[option_index].name) == 0) {
          /* n = */ sscanf(optarg, "%d", &attitude_file_format);
        }
        else if (strcmp("attitude-async", options[option_index].name) == 0) {
          const int n = sscanf(optarg, "%d", &attitude_async);
          if (n == 1)
            attitude_mode = MBPREPROCESS_MERGE_ASYNC;
          preprocess_pars.recalculate_bathymetry = true;
        }
        else if (strcmp("attitude-sensor", options[option_index].name) == 0) {
          /* n = */ sscanf(optarg, "%d", &attitude_sensor);
          preprocess_pars.recalculate_bathymetry = true;
        }
        /*-------------------------------------------------------
         * Define source of soundspeed - could be an external file
         * or an internal asynchronous record */
        else if (strcmp("soundspeed-file", options[option_index].name) == 0) {
          strcpy(soundspeed_file, optarg);
          soundspeed_mode = MBPREPROCESS_MERGE_FILE;
          preprocess_pars.modify_soundspeed = true;
          preprocess_pars.recalculate_bathymetry = true;
        }
        else if (strcmp("soundspeed-file-format", options[option_index].name) == 0) {
          /* n = */ sscanf(optarg, "%d", &soundspeed_file_format);
        }
        else if (strcmp("soundspeed-async", options[option_index].name) == 0) {
          const int n = sscanf(optarg, "%d", &soundspeed_async);
          if (n == 1)
            soundspeed_mode = MBPREPROCESS_MERGE_ASYNC;
          preprocess_pars.modify_soundspeed = true;
          preprocess_pars.recalculate_bathymetry = true;
        }
        else if (strcmp("soundspeed-sensor", options[option_index].name) == 0) {
          /* n = */ sscanf(optarg, "%d", &soundspeed_sensor);
          preprocess_pars.modify_soundspeed = true;
          preprocess_pars.recalculate_bathymetry = true;
        }
        /*-------------------------------------------------------
         * Define source of time_latency - could be an external file
         * or single value. Also define which data the time_latency model
         * will be applied to - nav, sensordepth, heading, attitude,
         * or all. */
        else if (strcmp("time-latency-file", options[option_index].name) == 0) {
          strcpy(time_latency_file, optarg);
          time_latency_mode = MB_SENSOR_TIME_LATENCY_MODEL;
        }
        else if (strcmp("time-latency-file-format", options[option_index].name) == 0) {
          /* n = */ sscanf(optarg, "%d", &time_latency_format);
        }
        else if (strcmp("time-latency-constant", options[option_index].name) == 0) {
          const int n = sscanf(optarg, "%lf", &time_latency_constant);
          if (n == 1)
            time_latency_mode = MB_SENSOR_TIME_LATENCY_STATIC;
        }
        else if (strcmp("time-latency-apply-nav", options[option_index].name) == 0) {
          time_latency_apply = time_latency_apply | MBPREPROCESS_TIME_LATENCY_APPLY_NAV;
          preprocess_pars.recalculate_bathymetry = true;
        }
        else if (strcmp("time-latency-apply-sensordepth", options[option_index].name) == 0) {
          time_latency_apply = time_latency_apply | MBPREPROCESS_TIME_LATENCY_APPLY_SENSORDEPTH;
          preprocess_pars.recalculate_bathymetry = true;
        }
        else if (strcmp("time-latency-apply-heading", options[option_index].name) == 0) {
          time_latency_apply = time_latency_apply | MBPREPROCESS_TIME_LATENCY_APPLY_HEADING;
          preprocess_pars.recalculate_bathymetry = true;
        }
        else if (strcmp("time-latency-apply-attitude", options[option_index].name) == 0) {
          time_latency_apply = time_latency_apply | MBPREPROCESS_TIME_LATENCY_APPLY_ATTITUDE;
          preprocess_pars.recalculate_bathymetry = true;
        }
        else if (strcmp("time-latency-apply-altitude", options[option_index].name) == 0) {
          time_latency_apply = time_latency_apply | MBPREPROCESS_TIME_LATENCY_APPLY_ATTITUDE;
          preprocess_pars.recalculate_bathymetry = true;
        }
        else if (strcmp("time-latency-apply-all-ancilliary", options[option_index].name) == 0) {
          time_latency_apply = MBPREPROCESS_TIME_LATENCY_APPLY_ALL_ANCILLIARY;
          preprocess_pars.recalculate_bathymetry = true;
        }
        else if (strcmp("time-latency-apply-survey", options[option_index].name) == 0) {
          time_latency_apply = MBPREPROCESS_TIME_LATENCY_APPLY_SURVEY;
          preprocess_pars.recalculate_bathymetry = true;
        }
        else if (strcmp("time-latency-apply-all", options[option_index].name) == 0) {
          time_latency_apply = MBPREPROCESS_TIME_LATENCY_APPLY_ALL;
          preprocess_pars.recalculate_bathymetry = true;
        }
        /*-------------------------------------------------------
         * Define time domain filtering of ancillary data such as
         * nav, sensordepth, heading, attitude, and altitude */
        else if (strcmp("filter", options[option_index].name) == 0) {
          /* n = */ sscanf(optarg, "%lf", &filter_length);
        }
        else if (strcmp("filter-apply-nav", options[option_index].name) == 0) {
          filter_apply = filter_apply | MBPREPROCESS_TIME_LATENCY_APPLY_NAV;
          preprocess_pars.recalculate_bathymetry = true;
        }
        else if (strcmp("filter-apply-sensordepth", options[option_index].name) == 0) {
          filter_apply = filter_apply | MBPREPROCESS_TIME_LATENCY_APPLY_SENSORDEPTH;
          preprocess_pars.recalculate_bathymetry = true;
        }
        else if (strcmp("filter-apply-heading", options[option_index].name) == 0) {
          filter_apply = filter_apply | MBPREPROCESS_TIME_LATENCY_APPLY_HEADING;
          preprocess_pars.recalculate_bathymetry = true;
        }
        else if (strcmp("filter-apply-attitude", options[option_index].name) == 0) {
          filter_apply = filter_apply | MBPREPROCESS_TIME_LATENCY_APPLY_ATTITUDE;
          preprocess_pars.recalculate_bathymetry = true;
        }
        else if (strcmp("filter-apply-altitude", options[option_index].name) == 0) {
          filter_apply = filter_apply | MBPREPROCESS_TIME_LATENCY_APPLY_ATTITUDE;
          preprocess_pars.recalculate_bathymetry = true;
        }
        else if (strcmp("filter-apply-all-ancilliary", options[option_index].name) == 0) {
          filter_apply = MBPREPROCESS_TIME_LATENCY_APPLY_ALL_ANCILLIARY;
          preprocess_pars.recalculate_bathymetry = true;
        }
        /*-------------------------------------------------------
         * Miscellaneous commands */
        else if (strcmp("recalculate-bathymetry", options[option_index].name) == 0) {
          preprocess_pars.recalculate_bathymetry = true;
        }
        else if (strcmp("no-change-survey", options[option_index].name) == 0) {
          preprocess_pars.no_change_survey = true;
        }
        else if (strcmp("multibeam-sidescan-source", options[option_index].name) == 0) {
          if (optarg[0] == 'S' || optarg[0] == 's')
            preprocess_pars.multibeam_sidescan_source = MB_PR_SSSOURCE_SNIPPET;
          else if (optarg[0] == 'C' || optarg[0] == 'c')
            preprocess_pars.multibeam_sidescan_source = MB_PR_SSSOURCE_CALIBRATEDSNIPPET;
          else if (optarg[0] == 'B' || optarg[0] == 'b')
            preprocess_pars.multibeam_sidescan_source = MB_PR_SSSOURCE_WIDEBEAMBACKSCATTER;
          else if (optarg[0] == 'W' || optarg[0] == 'w')
            preprocess_pars.multibeam_sidescan_source = MB_PR_SSSOURCE_CALIBRATEDWIDEBEAMBACKSCATTER;
        }
        else if (strcmp("sounding-amplitude-filter", options[option_index].name) == 0) {
          const int n = sscanf(optarg, "%lf", &preprocess_pars.sounding_amplitude_threshold);
          if (n == 1)
            preprocess_pars.sounding_amplitude_filter = true;
        }
        else if (strcmp("sounding-altitude-filter", options[option_index].name) == 0) {
          const int n = sscanf(optarg, "%lf", &preprocess_pars.sounding_target_altitude);
          if (n == 1)
            preprocess_pars.sounding_altitude_filter = true;
        }
        else if (strcmp("ignore-water-column", options[option_index].name) == 0) {
          preprocess_pars.ignore_water_column = true;
        }
        else if (strcmp("head1-offsets", options[option_index].name) == 0) {
          const int n = sscanf(optarg, "%lf/%lf/%lf/%lf/%lf/%lf",
                      &preprocess_pars.head1_offsets_x,
                      &preprocess_pars.head1_offsets_y,
                      &preprocess_pars.head1_offsets_z,
                      &preprocess_pars.head1_offsets_heading,
                      &preprocess_pars.head1_offsets_roll,
                      &preprocess_pars.head1_offsets_pitch);
          if (n == 6) {
            preprocess_pars.head1_offsets = true;
          }
        }
        else if (strcmp("head2-offsets", options[option_index].name) == 0) {
          const int n = sscanf(optarg, "%lf/%lf/%lf/%lf/%lf/%lf",
                      &preprocess_pars.head2_offsets_x,
                      &preprocess_pars.head2_offsets_y,
                      &preprocess_pars.head2_offsets_z,
                      &preprocess_pars.head2_offsets_heading,
                      &preprocess_pars.head2_offsets_roll,
                      &preprocess_pars.head2_offsets_pitch);
          if (n == 6) {
            preprocess_pars.head2_offsets = true;
          }
        }
        /*-------------------------------------------------------
         * Various fixes for specific data problems */
        else if (strcmp("kluge-time-jumps", options[option_index].name) == 0) {
          const int n = sscanf(optarg, "%lf", &kluge_timejumps_threshold);
          if (n == 1)
            kluge_timejumps = true;
        }
        else if (strcmp("kluge-ancilliary-time-jumps", options[option_index].name) == 0) {
          const int n = sscanf(optarg, "%lf", &kluge_timejumps_anc_threshold);
          if (n == 1)
            kluge_timejumps_ancilliary = true;
        }
        else if (strcmp("kluge-mbaripressure-time-jumps", options[option_index].name) == 0) {
          const int n = sscanf(optarg, "%lf", &kluge_timejumps_mba_threshold);
          if (n == 1)
            kluge_timejumps_mbaripressure = true;
        }
        else if (strcmp("kluge-beam-tweak", options[option_index].name) == 0) {
          const int n = sscanf(optarg, "%lf", &kluge_beamtweak_factor);
          if (n == 1) {
            kluge_beamtweak = true;
            preprocess_pars.kluge_id[preprocess_pars.n_kluge] = MB_PR_KLUGE_BEAMTWEAK;
            double *dptr = (double *)&preprocess_pars.kluge_pars[preprocess_pars.n_kluge * MB_PR_KLUGE_PAR_SIZE];
            *dptr = kluge_beamtweak_factor;
            preprocess_pars.n_kluge++;
            preprocess_pars.recalculate_bathymetry = true;
          }
        }
        else if (strcmp("kluge-soundspeed-tweak", options[option_index].name) == 0) {
          const int n = sscanf(optarg, "%lf", &kluge_soundspeedtweak_factor);
          if (n == 1) {
            kluge_soundspeedtweak = true;
            preprocess_pars.kluge_id[preprocess_pars.n_kluge] = MB_PR_KLUGE_SOUNDSPEEDTWEAK;
            double *dptr = (double *)&preprocess_pars.kluge_pars[preprocess_pars.n_kluge * MB_PR_KLUGE_PAR_SIZE];
            *dptr = kluge_soundspeedtweak_factor;
            preprocess_pars.n_kluge++;
            preprocess_pars.recalculate_bathymetry = true;
          }
        }
        else if (strcmp("kluge-zero-attitude-correction", options[option_index].name) == 0) {
          preprocess_pars.kluge_id[preprocess_pars.n_kluge] = MB_PR_KLUGE_ZEROATTITUDECORRECTION;
          preprocess_pars.n_kluge++;
          preprocess_pars.recalculate_bathymetry = true;
        }
        else if (strcmp("kluge-zero-alongtrack-angles", options[option_index].name) == 0) {
          preprocess_pars.kluge_id[preprocess_pars.n_kluge] = MB_PR_KLUGE_ZEROALONGTRACKANGLES;
          preprocess_pars.n_kluge++;
          preprocess_pars.recalculate_bathymetry = true;
        }
        else if (strcmp("kluge-fix-wissl-timestamps", options[option_index].name) == 0) {
          preprocess_pars.kluge_id[preprocess_pars.n_kluge] = MB_PR_KLUGE_FIXWISSLTIMESTAMPS;
          preprocess_pars.n_kluge++;
          preprocess_pars.recalculate_bathymetry = true;
          kluge_fix_wissl_timestamps = true;
        }
        else if (strcmp("kluge-auv-sentry-sensordepth", options[option_index].name) == 0) {
          preprocess_pars.kluge_id[preprocess_pars.n_kluge] = MB_PR_KLUGE_AUVSENTRYSENSORDEPTH;
          preprocess_pars.n_kluge++;
          kluge_auv_sentry_sensordepth = true;
        }
        else if (strcmp("kluge-ignore-snippets", options[option_index].name) == 0) {
          preprocess_pars.kluge_id[preprocess_pars.n_kluge] = MB_PR_KLUGE_IGNORESNIPPETS;
          preprocess_pars.n_kluge++;
          kluge_ignore_snippets = true;
        }
        else if (strcmp("kluge-sensordepth-from-heave", options[option_index].name) == 0) {
          preprocess_pars.kluge_id[preprocess_pars.n_kluge] = MB_PR_KLUGE_SENSORDEPTHFROMHEAVE;
          preprocess_pars.n_kluge++;
          kluge_sensordepth_from_heave = true;
        }

        break;
      case '?':
        errflg = true;
      }

    if (errflg) {
      fprintf(stderr, "usage: %s\n", usage_message);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_BAD_USAGE);
    }

    if (verbose == 1 || help) {
      fprintf(stderr, "\nProgram %s\n", program_name);
      fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
    }

    if (help) {
      fprintf(stderr, "\n%s\n", help_message);
      fprintf(stderr, "\nusage: %s\n", usage_message);
      exit(error);
    }
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
    if (format == 0)
      format = testformat;

    /* be sure that output datalist name does not conflict with input datalist name */
    if (format < 0 && strcmp(read_file, output_datalist) == 0) {
      snprintf(output_datalist, sizeof(output_datalist), "%sr.mb-1", fileroot);
    }
  }

  /* determine whether to read one file or a list of files */
  const bool read_datalist = format < 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Default MB-System Parameters:\n");
    fprintf(stderr, "dbg2       verbose:                      %d\n", verbose);
    fprintf(stderr, "dbg2       format:                       %d\n", format);
    fprintf(stderr, "dbg2       pings:                        %d\n", pings);
    fprintf(stderr, "dbg2       lonflip:                      %d\n", lonflip);
    fprintf(stderr, "dbg2       bounds[0]:                    %f\n", bounds[0]);
    fprintf(stderr, "dbg2       bounds[1]:                    %f\n", bounds[1]);
    fprintf(stderr, "dbg2       bounds[2]:                    %f\n", bounds[2]);
    fprintf(stderr, "dbg2       bounds[3]:                    %f\n", bounds[3]);
    fprintf(stderr, "dbg2       btime_i[0]:                   %d\n", btime_i[0]);
    fprintf(stderr, "dbg2       btime_i[1]:                   %d\n", btime_i[1]);
    fprintf(stderr, "dbg2       btime_i[2]:                   %d\n", btime_i[2]);
    fprintf(stderr, "dbg2       btime_i[3]:                   %d\n", btime_i[3]);
    fprintf(stderr, "dbg2       btime_i[4]:                   %d\n", btime_i[4]);
    fprintf(stderr, "dbg2       btime_i[5]:                   %d\n", btime_i[5]);
    fprintf(stderr, "dbg2       btime_i[6]:                   %d\n", btime_i[6]);
    fprintf(stderr, "dbg2       etime_i[0]:                   %d\n", etime_i[0]);
    fprintf(stderr, "dbg2       etime_i[1]:                   %d\n", etime_i[1]);
    fprintf(stderr, "dbg2       etime_i[2]:                   %d\n", etime_i[2]);
    fprintf(stderr, "dbg2       etime_i[3]:                   %d\n", etime_i[3]);
    fprintf(stderr, "dbg2       etime_i[4]:                   %d\n", etime_i[4]);
    fprintf(stderr, "dbg2       etime_i[5]:                   %d\n", etime_i[5]);
    fprintf(stderr, "dbg2       etime_i[6]:                   %d\n", etime_i[6]);
    fprintf(stderr, "dbg2       speedmin:                     %f\n", speedmin);
    fprintf(stderr, "dbg2       timegap:                      %f\n", timegap);
    fprintf(stderr, "dbg2  Input survey data to be preprocessed:\n");
    fprintf(stderr, "dbg2       read_file:                    %s\n", read_file);
    fprintf(stderr, "dbg2       format:                       %d\n", format);
    fprintf(stderr, "dbg2  Output directory:\n");
    if (output_directory_set)
      fprintf(stderr, "dbg2       output_directory:             %s\n", output_directory);
    else
      fprintf(stderr, "dbg2       output_directory:             not specified, use working directory\n");
    if (output_datalist_set)
      fprintf(stderr, "dbg2       output_datalist:             %s\n", output_datalist);
    else
      fprintf(stderr, "dbg2       output_datalist:             not specified, use default: %s\n", output_datalist);
    fprintf(stderr, "dbg2  Source of platform model:\n");
    if (use_platform_file)
      fprintf(stderr, "dbg2       platform_file:                %s\n", platform_file);
    else
      fprintf(stderr, "dbg2       platform_file:              not specified\n");
    fprintf(stderr, "dbg2       target_sensor:                %d\n", target_sensor);
    fprintf(stderr, "dbg2  Source of navigation data:\n");
    fprintf(stderr, "dbg2       nav_mode:                     %d\n", nav_mode);
    fprintf(stderr, "dbg2       nav_file:                     %s\n", nav_file);
    fprintf(stderr, "dbg2       nav_file_format:              %d\n", nav_file_format);
    fprintf(stderr, "dbg2       nav_async:                    %d\n", nav_async);
    fprintf(stderr, "dbg2       nav_sensor:                   %d\n", nav_sensor);
    fprintf(stderr, "dbg2  Source of sensor depth data:\n");
    fprintf(stderr, "dbg2       sensordepth_mode:             %d\n", sensordepth_mode);
    fprintf(stderr, "dbg2       sensordepth_file:             %s\n", sensordepth_file);
    fprintf(stderr, "dbg2       sensordepth_file_format:      %d\n", sensordepth_file_format);
    fprintf(stderr, "dbg2       sensordepth_async:            %d\n", sensordepth_async);
    fprintf(stderr, "dbg2       sensordepth_sensor:           %d\n", sensordepth_sensor);
    fprintf(stderr, "dbg2  Source of heading data:\n");
    fprintf(stderr, "dbg2       heading_mode:                 %d\n", heading_mode);
    fprintf(stderr, "dbg2       heading_file:                 %s\n", heading_file);
    fprintf(stderr, "dbg2       heading_file_format:          %d\n", heading_file_format);
    fprintf(stderr, "dbg2       heading_async:                %d\n", heading_async);
    fprintf(stderr, "dbg2       heading_sensor:               %d\n", heading_sensor);
    fprintf(stderr, "dbg2  Source of altitude data:\n");
    fprintf(stderr, "dbg2       altitude_mode:                %d\n", altitude_mode);
    fprintf(stderr, "dbg2       altitude_file:                %s\n", altitude_file);
    fprintf(stderr, "dbg2       altitude_file_format:         %d\n", altitude_file_format);
    fprintf(stderr, "dbg2       altitude_async:               %d\n", altitude_async);
    fprintf(stderr, "dbg2       altitude_sensor:              %d\n", altitude_sensor);
    fprintf(stderr, "dbg2  Source of attitude data:\n");
    fprintf(stderr, "dbg2       attitude_mode:                %d\n", attitude_mode);
    fprintf(stderr, "dbg2       attitude_file:                %s\n", attitude_file);
    fprintf(stderr, "dbg2       attitude_file_format:         %d\n", attitude_file_format);
    fprintf(stderr, "dbg2       attitude_async:               %d\n", attitude_async);
    fprintf(stderr, "dbg2       attitude_sensor:              %d\n", attitude_sensor);
    fprintf(stderr, "dbg2  Source of soundspeed data:\n");
    fprintf(stderr, "dbg2       soundspeed_mode:              %d\n", soundspeed_mode);
    fprintf(stderr, "dbg2       soundspeed_file:              %s\n", soundspeed_file);
    fprintf(stderr, "dbg2       soundspeed_file_format:       %d\n", soundspeed_file_format);
    fprintf(stderr, "dbg2       soundspeed_async:             %d\n", soundspeed_async);
    fprintf(stderr, "dbg2       soundspeed_sensor:            %d\n", soundspeed_sensor);
    fprintf(stderr, "dbg2  Time latency correction:\n");
    fprintf(stderr, "dbg2       time_latency_mode:            %d\n", time_latency_mode);
    fprintf(stderr, "dbg2       time_latency_constant:        %f\n", time_latency_constant);
    fprintf(stderr, "dbg2       time_latency_file:            %s\n", time_latency_file);
    fprintf(stderr, "dbg2       time_latency_format:          %d\n", time_latency_format);
    fprintf(stderr, "dbg2       time_latency_apply:           %x\n", time_latency_apply);
    fprintf(stderr, "dbg2  Time domain filtering:\n");
    fprintf(stderr, "dbg2       filter_length:                %f\n", filter_length);
    fprintf(stderr, "dbg2       filter_apply:                 %x\n", filter_apply);
    fprintf(stderr, "dbg2  Miscellaneous controls:\n");
    fprintf(stderr, "dbg2       no_change_survey:             %d\n", preprocess_pars.no_change_survey);
    fprintf(stderr, "dbg2       multibeam_sidescan_source:    %d\n", preprocess_pars.multibeam_sidescan_source);
    fprintf(stderr, "dbg2       recalculate_bathymetry:       %d\n", preprocess_pars.recalculate_bathymetry);
    fprintf(stderr, "dbg2       sounding_amplitude_filter:    %d\n", preprocess_pars.sounding_amplitude_filter);
    fprintf(stderr, "dbg2       sounding_amplitude_threshold: %f\n", preprocess_pars.sounding_amplitude_threshold);
    fprintf(stderr, "dbg2       sounding_altitude_filter:     %d\n", preprocess_pars.sounding_altitude_filter);
    fprintf(stderr, "dbg2       sounding_target_altitude:     %f\n", preprocess_pars.sounding_target_altitude);
    fprintf(stderr, "dbg2       ignore_water_column:          %d\n", preprocess_pars.ignore_water_column);
    fprintf(stderr, "dbg2       head1_offsets:                %d\n", preprocess_pars.head1_offsets);
    fprintf(stderr, "dbg2       head1_offsets_x:              %f\n", preprocess_pars.head1_offsets_x);
    fprintf(stderr, "dbg2       head1_offsets_y:              %f\n", preprocess_pars.head1_offsets_y);
    fprintf(stderr, "dbg2       head1_offsets_z:              %f\n", preprocess_pars.head1_offsets_z);
    fprintf(stderr, "dbg2       head1_offsets_heading:        %f\n", preprocess_pars.head1_offsets_heading);
    fprintf(stderr, "dbg2       head1_offsets_roll:           %f\n", preprocess_pars.head1_offsets_roll);
    fprintf(stderr, "dbg2       head1_offsets_pitch:          %f\n", preprocess_pars.head1_offsets_pitch);
    fprintf(stderr, "dbg2       head2_offsets:                %d\n", preprocess_pars.head2_offsets);
    fprintf(stderr, "dbg2       head2_offsets_x:              %f\n", preprocess_pars.head2_offsets_x);
    fprintf(stderr, "dbg2       head2_offsets_y:              %f\n", preprocess_pars.head2_offsets_y);
    fprintf(stderr, "dbg2       head2_offsets_z:              %f\n", preprocess_pars.head2_offsets_z);
    fprintf(stderr, "dbg2       head2_offsets_heading:        %f\n", preprocess_pars.head2_offsets_heading);
    fprintf(stderr, "dbg2       head2_offsets_roll:           %f\n", preprocess_pars.head2_offsets_roll);
    fprintf(stderr, "dbg2       head2_offsets_pitch:          %f\n", preprocess_pars.head2_offsets_pitch);
    fprintf(stderr, "dbg2  Various data fixes (kluges):\n");
    fprintf(stderr, "dbg2       kluge_timejumps:              %d\n", kluge_timejumps);
    fprintf(stderr, "dbg2       kluge_timejumps_threshold:    %f\n", kluge_timejumps_threshold);
    fprintf(stderr, "dbg2       kluge_timejumps_ancilliary:   %d\n", kluge_timejumps_ancilliary);
    fprintf(stderr, "dbg2       kluge_timejumps_anc_threshold:%f\n", kluge_timejumps_anc_threshold);
    fprintf(stderr, "dbg2       kluge_timejumps_mbaripressure:%d\n", kluge_timejumps_mbaripressure);
    fprintf(stderr, "dbg2       kluge_timejumps_mba_threshold:%f\n", kluge_timejumps_mba_threshold);
    fprintf(stderr, "dbg2       kluge_beamtweak:              %d\n", kluge_beamtweak);
    fprintf(stderr, "dbg2       kluge_beamtweak_factor:       %f\n", kluge_beamtweak_factor);
    fprintf(stderr, "dbg2       kluge_soundspeedtweak:        %d\n", kluge_soundspeedtweak);
    fprintf(stderr, "dbg2       kluge_soundspeedtweak_factor: %f\n", kluge_soundspeedtweak_factor);
    fprintf(stderr, "dbg2       kluge_fix_wissl_timestamps:   %d\n", kluge_fix_wissl_timestamps);
    fprintf(stderr, "dbg2       kluge_auv_sentry_sensordepth: %d\n", kluge_auv_sentry_sensordepth);
    fprintf(stderr, "dbg2       kluge_ignore_snippets:        %d\n", kluge_ignore_snippets);
    fprintf(stderr, "dbg2       kluge_sensordepth_from_heave   %d\n", kluge_sensordepth_from_heave);
    fprintf(stderr, "dbg2  Additional output:\n");
    fprintf(stderr, "dbg2       output_sensor_fnv:            %d\n", output_sensor_fnv);
    fprintf(stderr, "dbg2  Skip existing output files:\n");
    fprintf(stderr, "dbg2       skip_existing:                %d\n", skip_existing);
  }

  else if (verbose > 0) {
    fprintf(stderr, "\nProgram <  %s>\n", program_name);
    fprintf(stderr, "MB-system Version   %s\n", MB_VERSION);
    fprintf(stderr, "Input survey data to be preprocessed:\n");
    fprintf(stderr, "     read_file:                    %s\n", read_file);
    fprintf(stderr, "     format:                       %d\n", format);
    fprintf(stderr, "Output directory:\n");
    if (output_directory_set)
      fprintf(stderr, "     output_directory:             %s\n", output_directory);
    else
      fprintf(stderr, "     output_directory:             not specified, use working directory\n");
    if (output_datalist_set)
      fprintf(stderr, "     output_datalist:             %s\n", output_datalist);
    else
      fprintf(stderr, "     output_datalist:             not specified, use default: %s\n", output_datalist);
    fprintf(stderr, "Source of platform model:\n");
    if (use_platform_file)
      fprintf(stderr, "     platform_file:                %s\n", platform_file);
    else
      fprintf(stderr, "     platform_file:              not specified\n");
    fprintf(stderr, "     target_sensor:                %d\n", target_sensor);
    fprintf(stderr, "Source of navigation data:\n");
    fprintf(stderr, "     nav_mode:                     %d\n", nav_mode);
    fprintf(stderr, "     nav_file:                     %s\n", nav_file);
    fprintf(stderr, "     nav_file_format:              %d\n", nav_file_format);
    fprintf(stderr, "     nav_async:                    %d\n", nav_async);
    fprintf(stderr, "     nav_sensor:                   %d\n", nav_sensor);
    fprintf(stderr, "Source of sensor depth data:\n");
    fprintf(stderr, "     sensordepth_mode:             %d\n", sensordepth_mode);
    fprintf(stderr, "     sensordepth_file:             %s\n", sensordepth_file);
    fprintf(stderr, "     sensordepth_file_format:      %d\n", sensordepth_file_format);
    fprintf(stderr, "     sensordepth_async:            %d\n", sensordepth_async);
    fprintf(stderr, "     sensordepth_sensor:           %d\n", sensordepth_sensor);
    fprintf(stderr, "Source of heading data:\n");
    fprintf(stderr, "     heading_mode:                 %d\n", heading_mode);
    fprintf(stderr, "     heading_file:                 %s\n", heading_file);
    fprintf(stderr, "     heading_file_format:          %d\n", heading_file_format);
    fprintf(stderr, "     heading_async:                %d\n", heading_async);
    fprintf(stderr, "     heading_sensor:               %d\n", heading_sensor);
    fprintf(stderr, "Source of altitude data:\n");
    fprintf(stderr, "     altitude_mode:                %d\n", altitude_mode);
    fprintf(stderr, "     altitude_file:                %s\n", altitude_file);
    fprintf(stderr, "     altitude_file_format:         %d\n", altitude_file_format);
    fprintf(stderr, "     altitude_async:               %d\n", altitude_async);
    fprintf(stderr, "     altitude_sensor:              %d\n", altitude_sensor);
    fprintf(stderr, "Source of attitude data:\n");
    fprintf(stderr, "     attitude_mode:                %d\n", attitude_mode);
    fprintf(stderr, "     attitude_file:                %s\n", attitude_file);
    fprintf(stderr, "     attitude_file_format:         %d\n", attitude_file_format);
    fprintf(stderr, "     attitude_async:               %d\n", attitude_async);
    fprintf(stderr, "     attitude_sensor:              %d\n", attitude_sensor);
    fprintf(stderr, "Source of soundspeed data:\n");
    fprintf(stderr, "     soundspeed_mode:              %d\n", soundspeed_mode);
    fprintf(stderr, "     soundspeed_file:              %s\n", soundspeed_file);
    fprintf(stderr, "     soundspeed_file_format:       %d\n", soundspeed_file_format);
    fprintf(stderr, "     soundspeed_async:             %d\n", soundspeed_async);
    fprintf(stderr, "     soundspeed_sensor:            %d\n", soundspeed_sensor);
    fprintf(stderr, "Time latency correction:\n");
    fprintf(stderr, "     time_latency_mode:            %d\n", time_latency_mode);
    fprintf(stderr, "     time_latency_constant:        %f\n", time_latency_constant);
    fprintf(stderr, "     time_latency_file:            %s\n", time_latency_file);
    fprintf(stderr, "     time_latency_format:          %d\n", time_latency_format);
    fprintf(stderr, "     time_latency_apply:           %x\n", time_latency_apply);
    fprintf(stderr, "Time domain filtering:\n");
    fprintf(stderr, "     filter_length:                %f\n", filter_length);
    fprintf(stderr, "     filter_apply:                 %x\n", filter_apply);
    fprintf(stderr, "Miscellaneous controls:\n");
    fprintf(stderr, "     no_change_survey:             %d\n", preprocess_pars.no_change_survey);
    fprintf(stderr, "     multibeam_sidescan_source:    %d\n", preprocess_pars.multibeam_sidescan_source);
    fprintf(stderr, "     recalculate_bathymetry:       %d\n", preprocess_pars.recalculate_bathymetry);
    fprintf(stderr, "     sounding_amplitude_filter:    %d\n", preprocess_pars.sounding_amplitude_filter);
    fprintf(stderr, "     sounding_amplitude_threshold: %f\n", preprocess_pars.sounding_amplitude_threshold);
    fprintf(stderr, "     sounding_altitude_filter:     %d\n", preprocess_pars.sounding_altitude_filter);
    fprintf(stderr, "     sounding_target_altitude:     %f\n", preprocess_pars.sounding_target_altitude);
    fprintf(stderr, "     ignore_water_column:          %d\n", preprocess_pars.ignore_water_column);
    fprintf(stderr, "     head1_offsets:                %d\n", preprocess_pars.head1_offsets);
    fprintf(stderr, "     head1_offsets_x:              %f\n", preprocess_pars.head1_offsets_x);
    fprintf(stderr, "     head1_offsets_y:              %f\n", preprocess_pars.head1_offsets_y);
    fprintf(stderr, "     head1_offsets_z:              %f\n", preprocess_pars.head1_offsets_z);
    fprintf(stderr, "     head1_offsets_heading:        %f\n", preprocess_pars.head1_offsets_heading);
    fprintf(stderr, "     head1_offsets_roll:           %f\n", preprocess_pars.head1_offsets_roll);
    fprintf(stderr, "     head1_offsets_pitch:          %f\n", preprocess_pars.head1_offsets_pitch);
    fprintf(stderr, "     head2_offsets:                %d\n", preprocess_pars.head2_offsets);
    fprintf(stderr, "     head2_offsets_x:              %f\n", preprocess_pars.head2_offsets_x);
    fprintf(stderr, "     head2_offsets_y:              %f\n", preprocess_pars.head2_offsets_y);
    fprintf(stderr, "     head2_offsets_z:              %f\n", preprocess_pars.head2_offsets_z);
    fprintf(stderr, "     head2_offsets_heading:        %f\n", preprocess_pars.head2_offsets_heading);
    fprintf(stderr, "     head2_offsets_roll:           %f\n", preprocess_pars.head2_offsets_roll);
    fprintf(stderr, "     head2_offsets_pitch:          %f\n", preprocess_pars.head2_offsets_pitch);
    fprintf(stderr, "Various data fixes (kluges):\n");
    fprintf(stderr, "     kluge_timejumps:              %d\n", kluge_timejumps);
    fprintf(stderr, "     kluge_timejumps_threshold:    %f\n", kluge_timejumps_threshold);
    fprintf(stderr, "     kluge_timejumps_ancilliary:   %d\n", kluge_timejumps_ancilliary);
    fprintf(stderr, "     kluge_timejumps_anc_threshold:%f\n", kluge_timejumps_anc_threshold);
    fprintf(stderr, "     kluge_timejumps_mbaripressure:%d\n", kluge_timejumps_mbaripressure);
    fprintf(stderr, "     kluge_timejumps_mba_threshold:%f\n", kluge_timejumps_mba_threshold);
    fprintf(stderr, "     kluge_beamtweak:              %d\n", kluge_beamtweak);
    fprintf(stderr, "     kluge_beamtweak_factor:       %f\n", kluge_beamtweak_factor);
    fprintf(stderr, "     kluge_soundspeedtweak:        %d\n", kluge_soundspeedtweak);
    fprintf(stderr, "     kluge_soundspeedtweak_factor: %f\n", kluge_soundspeedtweak_factor);
    fprintf(stderr, "     kluge_fix_wissl_timestamps:   %d\n", kluge_fix_wissl_timestamps);
    fprintf(stderr, "     kluge_auv_sentry_sensordepth: %d\n", kluge_auv_sentry_sensordepth);
    fprintf(stderr, "     kluge_ignore_snippets:        %d\n", kluge_ignore_snippets);
    fprintf(stderr, "     kluge_sensordepth_from_heave:  %d\n", kluge_sensordepth_from_heave);
    fprintf(stderr, "Additional output:\n");
    fprintf(stderr, "     output_sensor_fnv:            %d\n", output_sensor_fnv);
    fprintf(stderr, "Skip existing output files:\n");
    fprintf(stderr, "     skip_existing:                %d\n", skip_existing);
  }

  /* platform definition file */
  struct mb_platform_struct *platform = nullptr;
  // struct mb_sensor_struct *sensor_bathymetry = nullptr;
  // struct mb_sensor_struct *sensor_backscatter = nullptr;
  // struct mb_sensor_struct *sensor_heave = nullptr;
  struct mb_sensor_struct *sensor_position = nullptr;
  struct mb_sensor_struct *sensor_depth = nullptr;
  struct mb_sensor_struct *sensor_heading = nullptr;
  struct mb_sensor_struct *sensor_rollpitch = nullptr;
  struct mb_sensor_struct *sensor_target = nullptr;

  /*-------------------------------------------------------------------*/
  /* load platform definition if specified */
  if (use_platform_file) {
    status = mb_platform_read(verbose, platform_file, (void **)&platform, &error);
    if (status == MB_FAILURE) {
      fprintf(stderr, "\nUnable to open and parse platform file: %s\n", platform_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }

    /* reset data sources according to commands */
    if (nav_sensor >= 0)
      platform->source_position = nav_sensor;
    if (sensordepth_sensor >= 0)
      platform->source_depth = sensordepth_sensor;
    if (heading_sensor >= 0)
      platform->source_heading = heading_sensor;
    if (attitude_sensor >= 0) {
      platform->source_rollpitch = attitude_sensor;
      platform->source_heave = attitude_sensor;
    }

    /* get sensor structures */
    // if (platform->source_bathymetry >= 0)
    //   sensor_bathymetry = &(platform->sensors[platform->source_bathymetry]);
    // if (platform->source_backscatter >= 0)
    //   sensor_backscatter = &(platform->sensors[platform->source_backscatter]);
    if (platform->source_position >= 0)
      sensor_position = &(platform->sensors[platform->source_position]);
    if (platform->source_depth >= 0)
      sensor_depth = &(platform->sensors[platform->source_depth]);
    if (platform->source_heading >= 0)
      sensor_heading = &(platform->sensors[platform->source_heading]);
    if (platform->source_rollpitch >= 0)
      sensor_rollpitch = &(platform->sensors[platform->source_rollpitch]);
    // if (platform->source_heave >= 0)
    //   sensor_heave = &(platform->sensors[platform->source_heave]);
    if (target_sensor < 0)
      target_sensor = platform->source_bathymetry;
    if (target_sensor >= 0)
      sensor_target = &(platform->sensors[target_sensor]);
  }

  /* asynchronous navigation, heading, altitude, attitude, soundspeed data */
  int n_nav = 0;
  int n_nav_alloc = 0;
  double *nav_time_d = nullptr;
  double *nav_navlon = nullptr;
  double *nav_navlat = nullptr;
  double *nav_speed = nullptr;

  int n_sensordepth = 0;
  int n_sensordepth_alloc = 0;
  double *sensordepth_time_d = nullptr;
  double *sensordepth_sensordepth = nullptr;

  int n_heading = 0;
  int n_heading_alloc = 0;
  double *heading_time_d = nullptr;
  double *heading_heading = nullptr;

  int n_altitude = 0;
  int n_altitude_alloc = 0;
  double *altitude_time_d = nullptr;
  double *altitude_altitude = nullptr;

  int n_attitude = 0;
  int n_attitude_alloc = 0;
  double *attitude_time_d = nullptr;
  double *attitude_roll = nullptr;
  double *attitude_pitch = nullptr;
  double *attitude_heave = nullptr;

  int n_soundspeed = 0;
  int n_soundspeed_alloc = 0;
  double *soundspeed_time_d = nullptr;
  double *soundspeed_soundspeed = nullptr;

  int time_latency_num = 0;
  int time_latency_alloc = 0;
  double *time_latency_time_d = nullptr;
  double *time_latency_time_latency = nullptr;

  /* time domain filtering */

  /* file indexing (used by some formats) */
  int num_indextable = 0;
  int num_indextable_alloc = 0;
  struct mb_io_indextable_struct *indextable = nullptr;
  int i_num_indextable = 0;
  struct mb_io_indextable_struct *i_indextable = nullptr;

  /* kluge various data fixes */
  double kluge_first_time_d = 0.0;
  double kluge_last_time_d = 0.0;
  double kluge_last_raw_time_d = 0.0;
  double dtime_d_expect = 0.0;
  double dtime_d = 0.0;
  bool kluge_fix_wissl_timestamps_setup1 = false;
  bool kluge_fix_wissl_timestamps_setup2 = false;

  /* MBIO read control parameters */
  void *datalist = nullptr;
  double file_weight;
  int iformat;
  int oformat;
  double btime_d;
  double etime_d;
  mb_path ifile = "";
  memset(ifile, 0, sizeof(mb_path));
  mb_path dfile = "";
  memset(dfile, 0, sizeof(mb_path));
  char ofile[MB_PATH_MAXLINE+10] = "";
  memset(ofile, 0, sizeof(mb_path));
  mb_path fileroot = "";
  memset(fileroot, 0, sizeof(mb_path));
  int beams_bath;
  int beams_amp;
  int pixels_ss;
  int obeams_bath;
  int obeams_amp;
  int opixels_ss;

  /* MBIO read values */
  void *imbio_ptr = nullptr;
  void *ombio_ptr = nullptr;
  void *istore_ptr = nullptr;
  int kind;
  int time_i[7];
  double time_d;
  double navlon;
  double navlat;
  double speed;
  double heading;
  double distance;
  double altitude;
  double sensordepth;
  double draft;
  double roll;
  double pitch;
  double heave;
  char *beamflag = nullptr;
  double *bath = nullptr;
  double *bathacrosstrack = nullptr;
  double *bathalongtrack = nullptr;
  double *amp = nullptr;
  double *ss = nullptr;
  double *ssacrosstrack = nullptr;
  double *ssalongtrack = nullptr;
  char comment[MB_COMMENT_MAXLINE];
  double navlon_org;
  double navlat_org;
  double speed_org;
  double heading_org;
  double altitude_org;
  double sensordepth_org;
  double draft_org;
  double roll_org, roll_delta;
  double pitch_org, pitch_delta;
  double heave_org;
  double depth_offset_change;

  /* arrays for asynchronous data accessed using mb_extract_nnav() */
  int nanavmax = MB_NAV_MAX;
  int nanav;
  int atime_i[7 * MB_NAV_MAX];
  double atime_d[MB_NAV_MAX];
  double alon[MB_NAV_MAX];
  double alat[MB_NAV_MAX];
  double aspeed[MB_NAV_MAX];
  double aheading[MB_NAV_MAX];
  double asensordepth[MB_NAV_MAX];
  double aroll[MB_NAV_MAX];
  double apitch[MB_NAV_MAX];
  double aheave[MB_NAV_MAX];

  /* counts of records read and written */
  int n_rt_data = 0;
  int n_rt_comment = 0;
  int n_rt_nav = 0;
  int n_rt_nav1 = 0;
  int n_rt_nav2 = 0;
  int n_rt_nav3 = 0;
  int n_rt_att = 0;
  int n_rt_att1 = 0;
  int n_rt_att2 = 0;
  int n_rt_att3 = 0;
  int n_rt_files = 0;

  int n_wt_data = 0;
  int n_wt_comment = 0;
  int n_wt_nav = 0;
  int n_wt_nav1 = 0;
  int n_wt_nav2 = 0;
  int n_wt_nav3 = 0;
  int n_wt_att = 0;
  int n_wt_att1 = 0;
  int n_wt_att2 = 0;
  int n_wt_att3 = 0;
  int n_wt_files = 0;

  char afile[MB_PATH_MAXLINE+100] = "";
  FILE *afp = nullptr;
  struct stat file_status;
  double start_time_d;
  double end_time_d;
  int istart, iend;
  int input_size, input_modtime, output_size, output_modtime;

  int isensor, ioffset;

  int testformat;
  int interp_error = MB_ERROR_NO_ERROR;
  int jnav = 0;
  int jsensordepth = 0;
  int jheading = 0;
  int jaltitude = 0;
  int jattitude = 0;
  char buffer[16] = "";

  /*-------------------------------------------------------------------*/
  /* load ancillary data from external files if requested */

  /* start by loading time latency model if required */
  if (time_latency_mode == MB_SENSOR_TIME_LATENCY_MODEL) {
    status = mb_loadtimeshiftdata(verbose, time_latency_file, time_latency_format, &time_latency_num, &time_latency_alloc,
                         &time_latency_time_d, &time_latency_time_latency, &error);

    if (status == MB_FAILURE) {
      fprintf(stderr, "\nUnable to open and parse time latency file: %s\n", time_latency_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    else if (time_latency_num < 1) {
      fprintf(stderr, "\nNo time latency values read from: %s\n", time_latency_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    else if (verbose > 0)
      fprintf(stderr, "%d time_latency records loaded from file %s\n", time_latency_num, time_latency_file);
  }

  /* import specified ancillary data */
  if (nav_mode == MBPREPROCESS_MERGE_FILE) {
    status = mb_loadnavdata(verbose, nav_file, nav_file_format, lonflip, &n_nav, &n_nav_alloc, &nav_time_d, &nav_navlon, &nav_navlat,
                   &nav_speed, &error);

    if (status == MB_FAILURE) {
      fprintf(stderr, "\nUnable to open and parse nav file: %s\n", nav_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    else if (n_nav < 1) {
      fprintf(stderr, "\nNo nav values read from: %s\n", nav_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    else if (verbose > 0)
      fprintf(stderr, "%d navigation records loaded from file %s\n", n_nav, nav_file);
  }
  if (sensordepth_mode == MBPREPROCESS_MERGE_FILE) {
    status = mb_loadsensordepthdata(verbose, sensordepth_file, sensordepth_file_format, &n_sensordepth, &n_sensordepth_alloc,
                           &sensordepth_time_d, &sensordepth_sensordepth, &error);

    if (status == MB_FAILURE) {
      fprintf(stderr, "\nUnable to open and parse sensordepth file: %s\n", sensordepth_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    else if (n_sensordepth < 1) {
      fprintf(stderr, "\nNo sensordepth values read from: %s\n", sensordepth_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    else if (verbose > 0)
      fprintf(stderr, "%d sensordepth records loaded from file %s\n", n_sensordepth, sensordepth_file);
  }
  if (heading_mode == MBPREPROCESS_MERGE_FILE) {
    status = mb_loadheadingdata(verbose, heading_file, heading_file_format, &n_heading, &n_heading_alloc, &heading_time_d,
                       &heading_heading, &error);

    if (status == MB_FAILURE) {
      fprintf(stderr, "\nUnable to open and parse heading file: %s\n", heading_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    else if (n_heading < 1) {
      fprintf(stderr, "\nNo heading values read from: %s\n", heading_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    else if (verbose > 0)
      fprintf(stderr, "%d heading records loaded from file %s\n", n_heading, heading_file);
  }
  if (altitude_mode == MBPREPROCESS_MERGE_FILE) {
    status = mb_loadaltitudedata(verbose, altitude_file, altitude_file_format, &n_altitude, &n_altitude_alloc, &altitude_time_d,
                        &altitude_altitude, &error);

    if (status == MB_FAILURE) {
      fprintf(stderr, "\nUnable to open and parse altitude file: %s\n", altitude_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    else if (n_altitude < 1) {
      fprintf(stderr, "\nNo altitude values read from: %s\n", altitude_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    else if (verbose > 0)
      fprintf(stderr, "%d altitude records loaded from file %s\n", n_altitude, altitude_file);
  }
  if (attitude_mode == MBPREPROCESS_MERGE_FILE) {
    status = mb_loadattitudedata(verbose, attitude_file, attitude_file_format, &n_attitude, &n_attitude_alloc, &attitude_time_d,
                        &attitude_roll, &attitude_pitch, &attitude_heave, &error);

    if (status == MB_FAILURE) {
      fprintf(stderr, "\nUnable to open and parse attitude file: %s\n", attitude_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    else if (n_attitude < 1) {
      fprintf(stderr, "\nNo attitude values read from: %s\n", attitude_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    else if (verbose > 0)
      fprintf(stderr, "%d attitude records loaded from file %s\n", n_attitude, attitude_file);
  }
  if (soundspeed_mode == MBPREPROCESS_MERGE_FILE) {
    status = mb_loadsoundspeeddata(verbose, soundspeed_file, soundspeed_file_format, &n_soundspeed, &n_soundspeed_alloc, &soundspeed_time_d,
                        &soundspeed_soundspeed, &error);

    if (status == MB_FAILURE) {
      fprintf(stderr, "\nUnable to open and parse soundspeed file: %s\n", soundspeed_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    else if (n_soundspeed < 1) {
      fprintf(stderr, "\nNo soundspeed values read from: %s\n", soundspeed_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    else if (verbose > 0)
      fprintf(stderr, "%d soundspeed records loaded from file %s\n", n_soundspeed, soundspeed_file);
  }

  /*-------------------------------------------------------------------*/

  /* Do first pass through the data collecting ancillary data from the desired source records */

  /* open file list */
  bool read_data = false;
  if (read_datalist) {
    const int look_processed = MB_DATALIST_LOOK_UNSET;
    if (mb_datalist_open(verbose, &datalist, read_file, look_processed, &error) != MB_SUCCESS) {
      fprintf(stderr, "\nUnable to open data list file: %s\n", read_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    read_data = mb_datalist_read(verbose, datalist, ifile, dfile, &iformat, &file_weight, &error) == MB_SUCCESS;
  }  else {
    // else copy single filename to be read
    strcpy(ifile, read_file);
    iformat = format;
    read_data = true;
  }

  /* loop over all files to be read */
  while (read_data) {
    /* if origin of the ancillary data has not been specified, figure out
       defaults based on the first file's format */
    if (nav_mode == MBPREPROCESS_MERGE_OFF) {
      if (iformat == MBF_EMOLDRAW || iformat == MBF_EM300RAW || iformat == MBF_EM710RAW) {
        nav_mode = MBPREPROCESS_MERGE_ASYNC;
        nav_async = MB_DATA_NAV;
      }
      else if (iformat == MBF_KEMKMALL) {
        nav_mode = MBPREPROCESS_MERGE_ASYNC;
        nav_async = MB_DATA_NAV1;
      }
      else if (iformat == MBF_RESON7KR) {
        nav_mode = MBPREPROCESS_MERGE_ASYNC;
        nav_async = MB_DATA_NAV1;
      }
      else if (iformat == MBF_RESON7K3) {
        nav_mode = MBPREPROCESS_MERGE_ASYNC;
        nav_async = MB_DATA_NAV;
      }
    }
    if (sensordepth_mode == MBPREPROCESS_MERGE_OFF) {
      if (iformat == MBF_EMOLDRAW || iformat == MBF_EM300RAW || iformat == MBF_EM710RAW) {
        sensordepth_mode = MBPREPROCESS_MERGE_ASYNC;
        sensordepth_async = MB_DATA_HEIGHT;
      }
      else if (iformat == MBF_KEMKMALL) {
        sensordepth_mode = MBPREPROCESS_MERGE_ASYNC;
        sensordepth_async = MB_DATA_SENSORDEPTH;
      }
      else if (iformat == MBF_RESON7KR) {
        sensordepth_mode = MBPREPROCESS_MERGE_ASYNC;
        sensordepth_async = MB_DATA_SENSORDEPTH;
      }
      else if (iformat == MBF_RESON7K3) {
        sensordepth_mode = MBPREPROCESS_MERGE_ASYNC;
        sensordepth_async = MB_DATA_NAV;
      }
    }
    if (heading_mode == MBPREPROCESS_MERGE_OFF) {
      if (iformat == MBF_EMOLDRAW || iformat == MBF_EM300RAW || iformat == MBF_EM710RAW) {
        heading_mode = MBPREPROCESS_MERGE_ASYNC;
        heading_async = MB_DATA_NAV;
      }
      else if (iformat == MBF_KEMKMALL) {
        heading_mode = MBPREPROCESS_MERGE_ASYNC;
        heading_async = MB_DATA_NAV1;
      }
      else if (iformat == MBF_RESON7KR) {
        heading_mode = MBPREPROCESS_MERGE_ASYNC;
        heading_async = MB_DATA_HEADING;
      }
      else if (iformat == MBF_RESON7K3) {
        heading_mode = MBPREPROCESS_MERGE_ASYNC;
        heading_async = MB_DATA_NAV;
      }
    }
    if (attitude_mode == MBPREPROCESS_MERGE_OFF) {
      if (iformat == MBF_EMOLDRAW || iformat == MBF_EM300RAW || iformat == MBF_EM710RAW) {
        attitude_mode = MBPREPROCESS_MERGE_ASYNC;
        attitude_async = MB_DATA_ATTITUDE;
      }
      else if (iformat == MBF_KEMKMALL) {
        attitude_mode = MBPREPROCESS_MERGE_ASYNC;
        attitude_async = MB_DATA_NAV1;
      }
      else if (iformat == MBF_RESON7KR) {
        attitude_mode = MBPREPROCESS_MERGE_ASYNC;
        attitude_async = MB_DATA_ATTITUDE;
      }
      else if (iformat == MBF_RESON7K3) {
        attitude_mode = MBPREPROCESS_MERGE_ASYNC;
        attitude_async = MB_DATA_ATTITUDE;
      }
    }

    if (verbose > 0)
      fprintf(stderr, "\nPass 1: Opening file %s %d\n", ifile, iformat);

    /* initialize reading the swath file */
    if (mb_read_init(verbose, ifile, iformat, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap,
                               &imbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
      char *message;
      mb_error(verbose, error, &message);
      fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
      fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
      fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", ifile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(error);
    }

    /* call preprocess function with pars settings before reading any data
        - for some formats this can set special read behavior
        - passing store_ptr == NULL indicates this is the pre-reading call
        - if a preprocess function does not exist for this format then
          standard preprocessing will be done - reset the error */
    if (mb_preprocess(verbose, imbio_ptr, NULL, NULL, (void *)&preprocess_pars, &error) == MB_FAILURE) {
      status = MB_SUCCESS;
      error = MB_ERROR_NO_ERROR;
    }

    beamflag = nullptr;
    bath = nullptr;
    amp = nullptr;
    bathacrosstrack = nullptr;
    bathalongtrack = nullptr;
    ss = nullptr;
    ssacrosstrack = nullptr;
    ssalongtrack = nullptr;
    status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, &error);

    /* if error initializing memory then quit */
    if (error != MB_ERROR_NO_ERROR) {
      char *message;
      mb_error(verbose, error, &message);
      fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
      fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(error);
    }

    /* zero file count records */
    int n_rf_data = 0;
    int n_rf_comment = 0;
    int n_rf_nav = 0;
    int n_rf_nav1 = 0;
    int n_rf_nav2 = 0;
    int n_rf_nav3 = 0;
    int n_rf_att = 0;
    int n_rf_att1 = 0;
    int n_rf_att2 = 0;
    int n_rf_att3 = 0;

    /* read data */
    while (error <= MB_ERROR_NO_ERROR) {
      /* reset error */
      error = MB_ERROR_NO_ERROR;

      /* read next data record */
      status = mb_get_all(verbose, imbio_ptr, &istore_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
                          &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
                          bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

      /* some nonfatal errors do not matter */
      if (error < MB_ERROR_NO_ERROR && error > MB_ERROR_UNINTELLIGIBLE) {
        error = MB_ERROR_NO_ERROR;
        status = MB_SUCCESS;
      }

      if (verbose >= 2) {
        fprintf(stderr, "\ndbg2  Data record read in program <%s>\n", program_name);
        fprintf(stderr, "dbg2       kind:           %d\n", kind);
        fprintf(stderr, "dbg2       error:          %d\n", error);
        fprintf(stderr, "dbg2       status:         %d\n", status);
      }

      /* count records */
      if (kind == MB_DATA_DATA) {
        n_rf_data++;
        n_rt_data++;
      }
      else if (kind == MB_DATA_COMMENT) {
        n_rf_comment++;
        n_rt_comment++;
      }
      else if (kind == MB_DATA_NAV) {
        n_rf_nav++;
        n_rt_nav++;
      }
      else if (kind == MB_DATA_NAV1) {
        n_rf_nav1++;
        n_rt_nav1++;
      }
      else if (kind == MB_DATA_NAV2) {
        n_rf_nav2++;
        n_rt_nav2++;
      }
      else if (kind == MB_DATA_NAV3) {
        n_rf_nav3++;
        n_rt_nav3++;
      }
      else if (kind == MB_DATA_ATTITUDE) {
        n_rf_att++;
        n_rt_att++;
      }
      else if (kind == MB_DATA_ATTITUDE1) {
        n_rf_att1++;
        n_rt_att1++;
      }
      else if (kind == MB_DATA_ATTITUDE2) {
        n_rf_att2++;
        n_rt_att2++;
      }
      else if (kind == MB_DATA_ATTITUDE3) {
        n_rf_att3++;
        n_rt_att3++;
      }

      /* look for nav if not externally defined */
      if (status == MB_SUCCESS && nav_mode == MBPREPROCESS_MERGE_ASYNC && kind == nav_async) {
        /* extract nav data */
        int extract_status = mb_extract_nnav(verbose, imbio_ptr, istore_ptr, nanavmax, &kind, &nanav, atime_i, atime_d, alon, alat,
                                 aspeed, aheading, asensordepth, aroll, apitch, aheave, &error);

        /* allocate memory if needed */
        if (extract_status == MB_SUCCESS && nanav > 0 && n_nav + nanav >= n_nav_alloc) {
          error = MB_ERROR_NO_ERROR;
          n_nav_alloc += std::max(MBPREPROCESS_ALLOC_CHUNK, nanav);
          status = mb_reallocd(verbose, __FILE__, __LINE__, n_nav_alloc * sizeof(double), (void **)&nav_time_d, &error);
          status &= mb_reallocd(verbose, __FILE__, __LINE__, n_nav_alloc * sizeof(double), (void **)&nav_navlon, &error);
          status &= mb_reallocd(verbose, __FILE__, __LINE__, n_nav_alloc * sizeof(double), (void **)&nav_navlat, &error);
          status &= mb_reallocd(verbose, __FILE__, __LINE__, n_nav_alloc * sizeof(double), (void **)&nav_speed, &error);
          if (error != MB_ERROR_NO_ERROR) {
            char *message;
            mb_error(verbose, error, &message);
            fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
            fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
            fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
            exit(error);
          }
        }

        /* copy the nav data */
        if (extract_status == MB_SUCCESS && nanav > 0) {
          for (int i = 0; i < nanav; i++) {
            if (atime_d[i] > 0.0 && alon[i] != 0.0 && alat[i] != 0.0) {
              nav_time_d[n_nav] = atime_d[i];
              nav_navlon[n_nav] = alon[i];
              nav_navlat[n_nav] = alat[i];
              nav_speed[n_nav] = aspeed[i];
              n_nav++;
            }
          }
        }
      }

      /* look for sensordepth if not externally defined */
      if (status == MB_SUCCESS && sensordepth_mode == MBPREPROCESS_MERGE_ASYNC && kind == sensordepth_async) {
        /* extract sensordepth data */
        int extract_status = status = mb_extract_nnav(verbose, imbio_ptr, istore_ptr, nanavmax, &kind, &nanav, atime_i, atime_d, alon, alat,
                                 aspeed, aheading, asensordepth, aroll, apitch, aheave, &error);

        /* allocate memory if needed */
        if (extract_status == MB_SUCCESS && nanav > 0 && n_sensordepth + nanav >= n_sensordepth_alloc) {
          error = MB_ERROR_NO_ERROR;
          n_sensordepth_alloc += std::max(MBPREPROCESS_ALLOC_CHUNK, nanav);
          status = mb_reallocd(verbose, __FILE__, __LINE__, n_sensordepth_alloc * sizeof(double),
                               (void **)&sensordepth_time_d, &error);
          status &= mb_reallocd(verbose, __FILE__, __LINE__, n_sensordepth_alloc * sizeof(double),
                               (void **)&sensordepth_sensordepth, &error);
          if (error != MB_ERROR_NO_ERROR) {
            char *message;
            mb_error(verbose, error, &message);
            fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
            fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
            fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
            exit(error);
          }
        }

        /* copy the sensordepth data */
        if (extract_status == MB_SUCCESS && nanav > 0) {
          for (int i = 0; i < nanav; i++) {
            sensordepth_time_d[n_sensordepth] = atime_d[i];
            sensordepth_sensordepth[n_sensordepth] = asensordepth[i];
            n_sensordepth++;
          }
        }
      }

      /* look for heading if not externally defined */
      if (status == MB_SUCCESS && heading_mode == MBPREPROCESS_MERGE_ASYNC && kind == heading_async) {
        /* extract heading data */
        int extract_status = mb_extract_nnav(verbose, imbio_ptr, istore_ptr, nanavmax, &kind, &nanav, atime_i, atime_d, alon, alat,
                                 aspeed, aheading, asensordepth, aroll, apitch, aheave, &error);

        /* allocate memory if needed */
        if (extract_status == MB_SUCCESS && nanav > 0 && n_heading + nanav >= n_heading_alloc) {
          error = MB_ERROR_NO_ERROR;
          n_heading_alloc += std::max(MBPREPROCESS_ALLOC_CHUNK, nanav);
          status = mb_reallocd(verbose, __FILE__, __LINE__, n_heading_alloc * sizeof(double), (void **)&heading_time_d,
                               &error);
          status &= mb_reallocd(verbose, __FILE__, __LINE__, n_heading_alloc * sizeof(double), (void **)&heading_heading,
                               &error);
          if (error != MB_ERROR_NO_ERROR) {
            char *message;
            mb_error(verbose, error, &message);
            fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
            fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
            fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
            exit(error);
          }
        }

        /* copy the heading data */
        if (extract_status == MB_SUCCESS && nanav > 0) {
          for (int i = 0; i < nanav; i++) {
            heading_time_d[n_heading] = atime_d[i];
            heading_heading[n_heading] = aheading[i];
            n_heading++;
          }
        }
      }

      /* look for altitude if not externally defined */
      if (status == MB_SUCCESS && altitude_mode == MBPREPROCESS_MERGE_ASYNC && kind == altitude_async) {
        /* extract altitude data */
        int extract_status = mb_extract_altitude(verbose, imbio_ptr, istore_ptr, &kind, &sensordepth, &altitude, &error);

        /* allocate memory if needed */
        if (extract_status == MB_SUCCESS && n_altitude + 1 >= n_altitude_alloc) {
          error = MB_ERROR_NO_ERROR;
          n_altitude_alloc += MBPREPROCESS_ALLOC_CHUNK;
          status = mb_reallocd(verbose, __FILE__, __LINE__, n_altitude_alloc * sizeof(double),
                               (void **)&altitude_time_d, &error);
          status &= mb_reallocd(verbose, __FILE__, __LINE__, n_altitude_alloc * sizeof(double),
                               (void **)&altitude_altitude, &error);
          if (error != MB_ERROR_NO_ERROR) {
            char *message;
            mb_error(verbose, error, &message);
            fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
            fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
            fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
            exit(error);
          }
        }

        /* copy the altitude data */
        if (extract_status == MB_SUCCESS) {
          altitude_time_d[n_altitude] = time_d;
          altitude_altitude[n_altitude] = altitude;
          n_altitude++;
        }
      }

      /* look for attitude if not externally defined */
      if (status == MB_SUCCESS && attitude_mode == MBPREPROCESS_MERGE_ASYNC && kind == attitude_async) {
        /* extract attitude data */
        int extract_status = mb_extract_nnav(verbose, imbio_ptr, istore_ptr, nanavmax, &kind, &nanav, atime_i, atime_d, alon, alat,
                                 aspeed, aheading, asensordepth, aroll, apitch, aheave, &error);

        /* allocate memory if needed */
        if (extract_status == MB_SUCCESS && nanav > 0 && n_attitude + nanav >= n_attitude_alloc) {
          error = MB_ERROR_NO_ERROR;
          n_attitude_alloc += std::max(MBPREPROCESS_ALLOC_CHUNK, nanav);
          status = mb_reallocd(verbose, __FILE__, __LINE__, n_attitude_alloc * sizeof(double),
                               (void **)&attitude_time_d, &error);
          status &= mb_reallocd(verbose, __FILE__, __LINE__, n_attitude_alloc * sizeof(double), (void **)&attitude_roll,
                               &error);
          status &= mb_reallocd(verbose, __FILE__, __LINE__, n_attitude_alloc * sizeof(double), (void **)&attitude_pitch,
                               &error);
          status &= mb_reallocd(verbose, __FILE__, __LINE__, n_attitude_alloc * sizeof(double), (void **)&attitude_heave,
                               &error);
          if (error != MB_ERROR_NO_ERROR) {
            char *message;
            mb_error(verbose, error, &message);
            fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
            fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
            fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
            exit(error);
          }
        }

        /* copy the attitude data */
        if (extract_status == MB_SUCCESS && nanav > 0) {
          for (int i = 0; i < nanav; i++) {
            attitude_time_d[n_attitude] = atime_d[i];
            attitude_roll[n_attitude] = aroll[i];
            attitude_pitch[n_attitude] = apitch[i];
            attitude_heave[n_attitude] = aheave[i];
            n_attitude++;
          }
        }
      }
    }

    /* copy data record index if used for this format */
    status &= mb_indextable(verbose, imbio_ptr, &i_num_indextable, (void **)&i_indextable, &error);
    if (i_num_indextable > 0) {
      if (num_indextable_alloc <= num_indextable + i_num_indextable) {
        /* allocate space */
        num_indextable_alloc += i_num_indextable;
        status &= mb_reallocd(verbose, __FILE__, __LINE__,
                            num_indextable_alloc * sizeof(struct mb_io_indextable_struct),
                            (void **)(&indextable), &error);
        if (error != MB_ERROR_NO_ERROR) {
          char *message;
          mb_error(verbose, error, &message);
          fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
          fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
          fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
          exit(error);
        }
      }

      /* copy the index */
      memcpy(&indextable[num_indextable], i_indextable,
                 i_num_indextable * sizeof(struct mb_io_indextable_struct));
      for (int i = num_indextable; i < num_indextable + i_num_indextable; i++) {
        indextable[i].file_index = n_rt_files;
      }
      num_indextable += i_num_indextable;
    }

    /* output data counts */
    if (verbose > 0) {
      fprintf(stderr, "Pass 1: Records read from input file %d: %s\n", n_rt_files, ifile);
      fprintf(stderr, "     %d survey records\n", n_rf_data);
      fprintf(stderr, "     %d comment records\n", n_rf_comment);
      fprintf(stderr, "     %d nav records\n", n_rf_nav);
      fprintf(stderr, "     %d nav1 records\n", n_rf_nav1);
      fprintf(stderr, "     %d nav2 records\n", n_rf_nav2);
      fprintf(stderr, "     %d nav3 records\n", n_rf_nav3);
      fprintf(stderr, "     %d att records\n", n_rf_att);
      fprintf(stderr, "     %d att1 records\n", n_rf_att1);
      fprintf(stderr, "     %d att2 records\n", n_rf_att2);
      fprintf(stderr, "     %d att3 records\n", n_rf_att3);
    }

    /* close the swath file */
    status &= mb_close(verbose, &imbio_ptr, &error);
        n_rt_files++;

    /* figure out whether and what to read next */
    if (read_datalist) {
      if (mb_datalist_read(verbose, datalist, ifile, dfile, &iformat, &file_weight, &error) == MB_SUCCESS)
        read_data = true;
      else
        read_data = false;
    }
    else {
      read_data = false;
    }

    /* end loop over files in list */
  }
  if (read_datalist)
    mb_datalist_close(verbose, &datalist, &error);

  /* output data counts */
  if (verbose > 0) {
    fprintf(stderr, "\n-----------------------------------------------\n");
    fprintf(stderr, "Pass 1: Total records read from %d input files:\n", n_rt_files);
    fprintf(stderr, "     %d survey records\n", n_rt_data);
    fprintf(stderr, "     %d comment records\n", n_rt_comment);
    fprintf(stderr, "     %d nav records\n", n_rt_nav);
    fprintf(stderr, "     %d nav1 records\n", n_rt_nav1);
    fprintf(stderr, "     %d nav2 records\n", n_rt_nav2);
    fprintf(stderr, "     %d nav3 records\n", n_rt_nav3);
    fprintf(stderr, "     %d att records\n", n_rt_att);
    fprintf(stderr, "     %d att1 records\n", n_rt_att1);
    fprintf(stderr, "     %d att2 records\n", n_rt_att2);
    fprintf(stderr, "     %d att3 records\n", n_rt_att3);
    fprintf(stderr, "Pass 1: Asynchronous data available for merging:\n");
    fprintf(stderr, "     %d navigation data (mode:%d)\n", n_nav, nav_mode);
    fprintf(stderr, "     %d sensordepth data (mode:%d)\n", n_sensordepth, sensordepth_mode);
    fprintf(stderr, "     %d heading data (mode:%d)\n", n_heading, heading_mode);
    fprintf(stderr, "     %d altitude data (mode:%d)\n", n_altitude, altitude_mode);
    fprintf(stderr, "     %d attitude data (mode:%d)\n", n_attitude, attitude_mode);
    fprintf(stderr, "     %d time_latency data (mode:%d)\n", time_latency_num, time_latency_mode);
    fprintf(stderr, "-----------------------------------------------\n");
  }

  /* end first pass through data */

  /*-------------------------------------------------------------------*/

#ifdef DEBUG_MBPREPROCESS
  for (int i=0;i<n_nav;i++) {
    mb_get_date(verbose,nav_time_d[i],time_i);
    fprintf(stderr,"NAV: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %.6f %.6f\n",
      time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6],
      nav_navlon[i], nav_navlat[i]);
  }

  for (int i=0;i<n_sensordepth;i++) {
    mb_get_date(verbose,sensordepth_time_d[i],time_i);
    fprintf(stderr,"DEP: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %.3f\n",
      time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6],
      sensordepth_sensordepth[i]);
  }

  for (int i=0;i<n_heading;i++) {
    mb_get_date(verbose,heading_time_d[i],time_i);
    fprintf(stderr,"HDG: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %.3f\n",
      time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6],
      heading_heading[i]);
  }

  for (int i=0;i<n_altitude;i++) {
    mb_get_date(verbose,altitude_time_d[i],time_i);
    fprintf(stderr,"HDG: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %.3f\n",
      time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6],
      altitude_altitude[i]);
  }

  for (int i=0;i<n_attitude;i++) {
    mb_get_date(verbose,attitude_time_d[i],time_i);
    fprintf(stderr,"RPH: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %.3f %.3f\n",
      time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6],
      attitude_roll[i], attitude_pitch[i]);
  }
#endif

  /* deal with correcting MBARI Mapping AUV pressure depth time jumps */
  if (kluge_timejumps_mbaripressure) {
    if (verbose > 0) {
      fprintf(stderr, "\n-----------------------------------------------\n");
      fprintf(stderr, "Applying time jump corrections to MBARI pressure depth data:\n");
    }

    /* sensordepth */
    if (n_sensordepth > 2 && n_sensordepth_alloc >= n_sensordepth) {
      dtime_d_expect = (sensordepth_time_d[n_sensordepth-1] - sensordepth_time_d[0]) / (n_sensordepth - 1);
      if (fabs((sensordepth_time_d[1] - sensordepth_time_d[0]) -  dtime_d_expect) < kluge_timejumps_mba_threshold) {
          dtime_d_expect = (sensordepth_time_d[1] - sensordepth_time_d[0]);
      }
      bool correction_on = false;
      double correction_start_time_d = 0.0;
      int correction_start_index = 0;
      int correction_end_index = -1;
      for (int i = 2;i<n_sensordepth;i++) {
        dtime_d = sensordepth_time_d[i] - sensordepth_time_d[i-1];
        if (fabs(dtime_d - dtime_d_expect) >= kluge_timejumps_mba_threshold) {
          if (!correction_on) {
            correction_on = true;
            correction_start_time_d = sensordepth_time_d[i-1];
            correction_start_index = i;
            correction_end_index = i - 1;
          }
          fprintf(stderr, "DEP MBARI FIX: i:%d t: %f %f dt: %f %f ",
              i, sensordepth_time_d[i-1], sensordepth_time_d[i], dtime_d, dtime_d_expect);
          if (sensordepth_time_d[i] < correction_start_time_d) {
            correction_end_index = i;
          }
          sensordepth_time_d[i] = sensordepth_time_d[i-1] + dtime_d_expect;
          fprintf(stderr, "newt[%d]: %f\n", i, sensordepth_time_d[i]);
        } else {
          /* if correction has been on and there was a negative jump that needs deleting */
          if (correction_on && correction_end_index > correction_start_index) {
            for (int ii = correction_start_index; ii <= correction_end_index; ii++) {
              fprintf(stderr,"DEP MBARI DELETE: i:%d t:%f\n", ii, sensordepth_time_d[ii]);
              sensordepth_time_d[ii] = 0.0;
            }
          }

          /* correction is off */
          correction_on = false;
        }
      }

      /* remove any samples that have had the timestamps zeroed */
      int nn = n_sensordepth;
      for (int i = n_sensordepth-1;i>=0;i--) {
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

  /* deal with ancillary data time jump corrections */
  if  (kluge_timejumps_ancilliary) {
    if (verbose > 0) {
      fprintf(stderr, "\n-----------------------------------------------\n");
      fprintf(stderr, "Applying time jump corrections to ancillary data:\n");
    }

    /* position */
    if (n_nav > 2 && n_nav_alloc >= n_nav) {
      dtime_d_expect = (nav_time_d[n_nav-1] - nav_time_d[0]) / (n_nav - 1);
      if (fabs((nav_time_d[1] - nav_time_d[0]) -  dtime_d_expect) < kluge_timejumps_anc_threshold)
          dtime_d_expect = (nav_time_d[1] - nav_time_d[0]);
      for (int i = 2;i<n_nav;i++) {
        dtime_d = nav_time_d[i] - nav_time_d[i-1];
        if (fabs(dtime_d - dtime_d_expect) >= kluge_timejumps_anc_threshold) {
          fprintf(stderr, "NAV TIME JUMP FIX: i:%d t: %f %f dt: %f %f ",
              i, nav_time_d[i-1], nav_time_d[i], dtime_d, dtime_d_expect);
          nav_time_d[i] = nav_time_d[i-1] + dtime_d_expect;
          fprintf(stderr, "newt[%d]: %f\n", i, nav_time_d[i]);
        }
      }
    }

    /* sensordepth */
    if (n_sensordepth > 2 && n_sensordepth_alloc >= n_sensordepth) {
      dtime_d_expect = (sensordepth_time_d[n_sensordepth-1] - sensordepth_time_d[0]) / (n_sensordepth - 1);
      if (fabs((sensordepth_time_d[1] - sensordepth_time_d[0]) -  dtime_d_expect) < kluge_timejumps_anc_threshold)
          dtime_d_expect = (sensordepth_time_d[1] - sensordepth_time_d[0]);
      for (int i = 2;i<n_sensordepth;i++) {
        dtime_d = sensordepth_time_d[i] - sensordepth_time_d[i-1];
        if (fabs(dtime_d - dtime_d_expect) >= kluge_timejumps_anc_threshold) {
          fprintf(stderr, "DEP TIME JUMP FIX: i:%d t: %f %f dt: %f %f ",
              i, sensordepth_time_d[i-1], sensordepth_time_d[i], dtime_d, dtime_d_expect);
          sensordepth_time_d[i] = sensordepth_time_d[i-1] + dtime_d_expect;
          fprintf(stderr, "newt[%d]: %f\n", i, sensordepth_time_d[i]);
        }
      }
    }

    /* heading */
    if (n_heading > 2 && n_heading_alloc >= n_heading) {
      dtime_d_expect = (heading_time_d[n_heading-1] - heading_time_d[0]) / (n_heading - 1);
      if (fabs((heading_time_d[1] - heading_time_d[0]) -  dtime_d_expect) < kluge_timejumps_anc_threshold)
          dtime_d_expect = (heading_time_d[1] - heading_time_d[0]);
      for (int i = 2;i<n_heading;i++) {
        dtime_d = heading_time_d[i] - heading_time_d[i-1];
        if (fabs(dtime_d - dtime_d_expect) >= kluge_timejumps_anc_threshold) {
          fprintf(stderr, "HDG TIME JUMP FIX: i:%d t: %f %f dt: %f %f ",
              i, heading_time_d[i-1], heading_time_d[i], dtime_d, dtime_d_expect);
          heading_time_d[i] = heading_time_d[i-1] + dtime_d_expect;
          fprintf(stderr, "newt[%d]: %f\n", i, heading_time_d[i]);
        }
      }
    }

    /* altitude */
    if (n_altitude > 2 && n_altitude_alloc >= n_altitude) {
      dtime_d_expect = (altitude_time_d[n_altitude-1] - altitude_time_d[0]) / (n_altitude - 1);
      if (fabs((altitude_time_d[1] - altitude_time_d[0]) -  dtime_d_expect) < kluge_timejumps_anc_threshold)
          dtime_d_expect = (altitude_time_d[1] - altitude_time_d[0]);
      for (int i = 2;i<n_altitude;i++) {
        dtime_d = altitude_time_d[i] - altitude_time_d[i-1];
        if (fabs(dtime_d - dtime_d_expect) >= kluge_timejumps_anc_threshold) {
          fprintf(stderr, "ALT TIME JUMP FIX: i:%d t: %f %f dt: %f %f ",
              i, altitude_time_d[i-1], altitude_time_d[i], dtime_d, dtime_d_expect);
          altitude_time_d[i] = altitude_time_d[i-1] + dtime_d_expect;
          fprintf(stderr, "newt[%d]: %f\n", i, altitude_time_d[i]);
        }
      }
    }

    /* attitude */
    if (n_attitude > 2 && n_attitude_alloc >= n_attitude) {
      dtime_d_expect = (attitude_time_d[n_attitude-1] - attitude_time_d[0]) / (n_attitude - 1);
      if (fabs((attitude_time_d[1] - attitude_time_d[0]) -  dtime_d_expect) < kluge_timejumps_anc_threshold)
          dtime_d_expect = (attitude_time_d[1] - attitude_time_d[0]);
      for (int i = 2;i<n_attitude;i++) {
        dtime_d = attitude_time_d[i] - attitude_time_d[i-1];
        if (fabs(dtime_d - dtime_d_expect) >= kluge_timejumps_anc_threshold) {
          fprintf(stderr, "ATT TIME JUMP FIX: i:%d t: %f %f dt: %f %f ",
              i, attitude_time_d[i-1], attitude_time_d[i], dtime_d, dtime_d_expect);
          attitude_time_d[i] = attitude_time_d[i-1] + dtime_d_expect;
          fprintf(stderr, "newt[%d]: %f\n", i, attitude_time_d[i]);
        }
      }
    }

#ifdef DEBUG_MBPREPROCESS
    for (int i = 0;i<n_nav;i++) {
      mb_get_date(verbose,nav_time_d[i],time_i);
      fprintf(stderr,"NAV: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %.6f %.6f\n",
        time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6],
        nav_navlon[i], nav_navlat[i]);
    }

    for (int i = 0;i<n_sensordepth;i++) {
      mb_get_date(verbose,sensordepth_time_d[i],time_i);
      fprintf(stderr,"DEP: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %.3f\n",
        time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6],
        sensordepth_sensordepth[i]);
    }

    for (int i = 0;i<n_heading;i++) {
      mb_get_date(verbose,heading_time_d[i],time_i);
      fprintf(stderr,"HDG: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %.3f\n",
        time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6],
        heading_heading[i]);
    }

    for (int i = 0;i<n_altitude;i++) {
      mb_get_date(verbose,altitude_time_d[i],time_i);
      fprintf(stderr,"HDG: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %.3f\n",
        time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6],
        altitude_altitude[i]);
    }

    for (int i = 0;i<n_attitude;i++) {
      mb_get_date(verbose,attitude_time_d[i],time_i);
      fprintf(stderr,"RPH: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %.3f %.3f\n",
        time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6],
        attitude_roll[i], attitude_pitch[i]);
    }
#endif

    for (int i = 0;i<n_sensordepth;i++) {
      mb_get_date(verbose,sensordepth_time_d[i],time_i);
      fprintf(stderr,"DEP: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %.3f\n",
        time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6],
        sensordepth_sensordepth[i]);
    }
  }

  /* deal with time latency corrections */
  if (verbose > 0) {
    fprintf(stderr, "\n-----------------------------------------------\n");
    fprintf(stderr, "Applying time latency corrections:\n");
  }

  /* position */
  if (n_nav > 0 && n_nav_alloc >= n_nav) {
    /* apply time latency correction called for in the platform file */
    if (sensor_position != nullptr && sensor_position->time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) {
      if (verbose > 0) {
        if (sensor_position->time_latency_mode == MB_SENSOR_TIME_LATENCY_STATIC)
          fprintf(stderr,
                  "Applying time latency correction from platform model to %d position data using constant offset %f\n",
                  n_nav, sensor_position->time_latency_static);
        else
          fprintf(stderr,
                  "Applying time latency correction from platform model to %d position data using time-varying model\n",
                  n_nav);
      }
      mb_apply_time_latency(verbose, n_nav, nav_time_d, sensor_position->time_latency_mode,
                            sensor_position->time_latency_static, sensor_position->num_time_latency,
                            sensor_position->time_latency_time_d, sensor_position->time_latency_value, &error);
    }

    /* apply time latency correction called for on the command line */
    if ((time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) && (time_latency_apply & MBPREPROCESS_TIME_LATENCY_APPLY_NAV)) {
      if (verbose > 0) {
        if (time_latency_mode == MB_SENSOR_TIME_LATENCY_STATIC)
          fprintf(stderr,
                  "Applying time latency correction from command line to %d position data using constant offset %f\n",
                  n_nav, time_latency_constant);
        else
          fprintf(stderr,
                  "Applying time latency correction from command line to %d position data using time-varying model\n",
                  n_nav);
      }
      mb_apply_time_latency(verbose, n_nav, nav_time_d, time_latency_mode, time_latency_constant, time_latency_num,
                            time_latency_time_d, time_latency_time_latency, &error);
    }
  }

  /* sensordepth */
  if (n_sensordepth > 0 && n_sensordepth_alloc >= n_sensordepth) {
    /* apply time latency correction called for in the platform file */
    if (sensor_depth != nullptr && sensor_depth->time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) {
      if (verbose > 0) {
        if (sensor_depth->time_latency_mode == MB_SENSOR_TIME_LATENCY_STATIC)
          fprintf(
              stderr,
              "Applying time latency correction from platform model to %d sensordepth data using constant offset %f\n",
              n_sensordepth, sensor_depth->time_latency_static);
        else
          fprintf(
              stderr,
              "Applying time latency correction from platform model to %d sensordepth data using time-varying model\n",
              n_sensordepth);
      }
      mb_apply_time_latency(verbose, n_sensordepth, sensordepth_time_d, sensor_depth->time_latency_mode,
                            sensor_depth->time_latency_static, sensor_depth->num_time_latency,
                            sensor_depth->time_latency_time_d, sensor_depth->time_latency_value, &error);
    }

    /* apply time latency correction called for on the command line */
    if ((time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) &&
        (time_latency_apply & MBPREPROCESS_TIME_LATENCY_APPLY_SENSORDEPTH)) {
      if (verbose > 0) {
        if (time_latency_mode == MB_SENSOR_TIME_LATENCY_STATIC)
          fprintf(
              stderr,
              "Applying time latency correction from command line to %d sensordepth data using constant offset %f\n",
              n_sensordepth, time_latency_constant);
        else
          fprintf(
              stderr,
              "Applying time latency correction from command line to %d sensordepth data using time-varying model\n",
              n_sensordepth);
      }
      mb_apply_time_latency(verbose, n_sensordepth, sensordepth_time_d, time_latency_mode, time_latency_constant,
                            time_latency_num, time_latency_time_d, time_latency_time_latency, &error);
    }
  }

  /* heading */
  if (n_heading > 0 && n_heading_alloc >= n_heading) {
    /* apply time latency correction called for in the platform file */
    if (sensor_heading != nullptr && sensor_heading->time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) {
      if (verbose > 0) {
        if (sensor_heading->time_latency_mode == MB_SENSOR_TIME_LATENCY_STATIC)
          fprintf(stderr,
                  "Applying time latency correction from platform model to %d heading data using constant offset %f\n",
                  n_heading, sensor_heading->time_latency_static);
        else
          fprintf(stderr,
                  "Applying time latency correction from platform model to %d heading data using time-varying model\n",
                  n_heading);
      }
      mb_apply_time_latency(verbose, n_heading, heading_time_d, sensor_heading->time_latency_mode,
                            sensor_heading->time_latency_static, sensor_heading->num_time_latency,
                            sensor_heading->time_latency_time_d, sensor_heading->time_latency_value, &error);
    }

    /* apply time latency correction called for on the command line */
    if ((time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) &&
        (time_latency_apply & MBPREPROCESS_TIME_LATENCY_APPLY_HEADING)) {
      if (verbose > 0) {
        if (time_latency_mode == MB_SENSOR_TIME_LATENCY_STATIC)
          fprintf(stderr,
                  "Applying time latency correction from command line to %d heading data using constant offset %f\n",
                  n_heading, time_latency_constant);
        else
          fprintf(stderr,
                  "Applying time latency correction from command line to %d heading data using time-varying model\n",
                  n_heading);
      }
      mb_apply_time_latency(verbose, n_heading, heading_time_d, time_latency_mode, time_latency_constant, time_latency_num,
                            time_latency_time_d, time_latency_time_latency, &error);
    }
  }

  /* altitude */
  if (n_altitude > 0 && n_altitude_alloc >= n_altitude) {
    /* apply time latency correction called for on the command line */
    if ((time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) &&
        (time_latency_apply & MBPREPROCESS_TIME_LATENCY_APPLY_ALTITUDE)) {
      if (verbose > 0) {
        if (time_latency_mode == MB_SENSOR_TIME_LATENCY_STATIC)
          fprintf(stderr,
                  "Applying time latency correction from command line to %d altitude data using constant offset %f\n",
                  n_altitude, time_latency_constant);
        else
          fprintf(stderr,
                  "Applying time latency correction from command line to %d altitude data using time-varying model\n",
                  n_altitude);
      }
      mb_apply_time_latency(verbose, n_altitude, altitude_time_d, time_latency_mode, time_latency_constant,
                            time_latency_num, time_latency_time_d, time_latency_time_latency, &error);
    }
  }

  /* attitude */
  if (n_attitude > 0 && n_attitude_alloc >= n_attitude) {
    /* apply time latency correction called for in the platform file */
    if (sensor_rollpitch != nullptr && sensor_rollpitch->time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) {
      if (verbose > 0) {
        if (sensor_rollpitch->time_latency_mode == MB_SENSOR_TIME_LATENCY_STATIC)
          fprintf(stderr,
                  "Applying time latency correction from platform model to %d attitude data using constant offset %f\n",
                  n_attitude, sensor_rollpitch->time_latency_static);
        else
          fprintf(stderr,
                  "Applying time latency correction from platform model to %d attitude data using time-varying model\n",
                  n_attitude);
      }
      mb_apply_time_latency(verbose, n_attitude, attitude_time_d, sensor_rollpitch->time_latency_mode,
                            sensor_rollpitch->time_latency_static, sensor_rollpitch->num_time_latency,
                            sensor_rollpitch->time_latency_time_d, sensor_rollpitch->time_latency_value, &error);
    }

    /* apply time latency correction called for on the command line */
    if ((time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) &&
        (time_latency_apply & MBPREPROCESS_TIME_LATENCY_APPLY_ATTITUDE)) {
      if (verbose > 0) {
        if (time_latency_mode == MB_SENSOR_TIME_LATENCY_STATIC)
          fprintf(stderr,
                  "Applying time latency correction from command line to %d attitude data using constant offset %f\n",
                  n_attitude, time_latency_constant);
        else
          fprintf(stderr,
                  "Applying time latency correction from command line to %d attitude data using time-varying model\n",
                  n_attitude);
      }
      mb_apply_time_latency(verbose, n_attitude, attitude_time_d, time_latency_mode, time_latency_constant,
                            time_latency_num, time_latency_time_d, time_latency_time_latency, &error);
    }

  }

  /*-------------------------------------------------------------------*/

  /* deal with filtering */
  if (verbose > 0) {
    fprintf(stderr, "\n-----------------------------------------------\n");
    fprintf(stderr, "Applying time domain filtering:\n");
  }

  /* filter position */
  if ((filter_apply & MBPREPROCESS_TIME_LATENCY_APPLY_NAV) && n_nav > 0 && n_nav_alloc >= n_nav) {
    if (verbose > 0)
      fprintf(stderr, "Applying %f second Gaussian filter to %d position data\n", filter_length, n_nav);
    mb_apply_time_filter(verbose, n_nav, nav_time_d, nav_navlon, filter_length, &error);
    mb_apply_time_filter(verbose, n_nav, nav_time_d, nav_navlat, filter_length, &error);
  }

  /* filter sensordepth */
  if ((filter_apply & MBPREPROCESS_TIME_LATENCY_APPLY_SENSORDEPTH) && n_sensordepth > 0 &&
      n_sensordepth_alloc >= n_sensordepth) {
    if (verbose > 0)
      fprintf(stderr, "Applying %f second Gaussian filter to %d sensordepth data\n", filter_length, n_sensordepth);
    mb_apply_time_filter(verbose, n_sensordepth, sensordepth_time_d, sensordepth_sensordepth, filter_length, &error);
  }

  /* heading */
  if ((filter_apply & MBPREPROCESS_TIME_LATENCY_APPLY_HEADING) && n_heading > 0 && n_heading_alloc >= n_heading) {
    if (verbose > 0)
      fprintf(stderr, "Applying %f second Gaussian filter to %d heading data\n", filter_length, n_heading);
    mb_apply_time_filter(verbose, n_heading, heading_time_d, heading_heading, filter_length, &error);
  }

  /* altitude */
  if ((filter_apply & MBPREPROCESS_TIME_LATENCY_APPLY_ALTITUDE) && n_altitude > 0 && n_altitude_alloc >= n_altitude) {
    if (verbose > 0)
      fprintf(stderr, "Applying %f second Gaussian filter to %d altitude data\n", filter_length, n_altitude);
    mb_apply_time_filter(verbose, n_altitude, altitude_time_d, altitude_altitude, filter_length, &error);
  }

  /* attitude */
  if ((filter_apply & MBPREPROCESS_TIME_LATENCY_APPLY_ATTITUDE) && n_attitude > 0 && n_attitude_alloc >= n_attitude) {
    if (verbose > 0)
      fprintf(stderr, "Applying %f second Gaussian filter to %d attitude data\n", filter_length, n_attitude);
    mb_apply_time_filter(verbose, n_attitude, attitude_time_d, attitude_roll, filter_length, &error);
    mb_apply_time_filter(verbose, n_attitude, attitude_time_d, attitude_pitch, filter_length, &error);
    mb_apply_time_filter(verbose, n_attitude, attitude_time_d, attitude_heave, filter_length, &error);
  }

  if (verbose > 0) {
    fprintf(stderr, "-----------------------------------------------\n");
  }

  /*-------------------------------------------------------------------*/

  /* Do second pass through the data reading everything,
      correcting survey data, and outputting everything */

  /* zero file count records */
  int n_rf_data = 0;
  int n_rf_comment = 0;
  int n_rf_nav = 0;
  int n_rf_nav1 = 0;
  int n_rf_nav2 = 0;
  int n_rf_nav3 = 0;
  int n_rf_att = 0;
  int n_rf_att1 = 0;
  int n_rf_att2 = 0;
  int n_rf_att3 = 0;
  n_rt_data = 0;
  n_rt_files = 0;

  int n_wf_data = 0;
  int n_wf_comment = 0;
  int n_wf_nav = 0;
  int n_wf_nav1 = 0;
  int n_wf_nav2 = 0;
  int n_wf_nav3 = 0;
  int n_wf_att = 0;
  int n_wf_att1 = 0;
  int n_wf_att2 = 0;
  int n_wf_att3 = 0;
  n_wt_data = 0;
  n_wt_comment = 0;
  n_wt_nav = 0;
  n_wt_nav1 = 0;
  n_wt_nav2 = 0;
  n_wt_nav3 = 0;
  n_wt_att = 0;
  n_wt_att1 = 0;
  n_wt_att2 = 0;
  n_wt_att3 = 0;
  n_wt_files = 0;

  /* if requested to output integrated nav for all survey sensors, open files */
  if (output_sensor_fnv && platform != nullptr) {
    if (verbose > 0)
      fprintf(stderr, "\nOutputting fnv files for survey sensors\n");
    for (isensor = 0; isensor < platform->num_sensors; isensor++) {
      //if (platform->sensors[isensor].capability2 != 0) {
        if (verbose > 0)
          fprintf(stderr, "Outputting sensor %d with capability %d\n", isensor, platform->sensors[isensor].capability2);
        for (ioffset = 0; ioffset < platform->sensors[isensor].num_offsets; ioffset++) {
          mb_path fnvfile = "";
          snprintf(fnvfile, sizeof(fnvfile), "sensor_%2.2d_%2.2d_%2.2d.fnv", isensor, ioffset, platform->sensors[isensor].type);
          if (verbose > 0)
            fprintf(stderr, "Outputting sensor %d offset %d in fnv file:%s\n", isensor, ioffset, fnvfile);
          if ((platform->sensors[isensor].offsets[ioffset].ofp = fopen(fnvfile, "wb")) == nullptr) {
            fprintf(stderr, "\nUnable to open sensor fnv data file <%s> for writing\n", fnvfile);
            fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
            exit(MB_ERROR_OPEN_FAIL);
          }
        }
      //}
    }
  }

  /* open file list */
  if (read_datalist) {
    const int look_processed = MB_DATALIST_LOOK_UNSET;
    if (mb_datalist_open(verbose, &datalist, read_file, look_processed, &error) != MB_SUCCESS) {
      fprintf(stderr, "\nUnable to open data list file: %s\n", read_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    read_data = mb_datalist_read(verbose, datalist, ifile, dfile, &iformat, &file_weight, &error) == MB_SUCCESS;
  } else {
    // else copy single filename to be read
    strcpy(ifile, read_file);
    iformat = format;
    read_data = true;
  }

  /* loop over all files to be read */
  while (read_data) {
    int sensorhead = 0;
    int sensortype = 0;

    /* get output format - in some cases this may be a
     * different, generally extended format
     * more suitable for processing than the original */
    if (iformat == MBF_EMOLDRAW || iformat == MBF_EM12IFRM || iformat == MBF_EM12DARW || iformat == MBF_EM300RAW ||
        iformat == MBF_EM300MBA)
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

    /* figure out the output file name */
    status = mb_get_format(verbose, ifile, fileroot, &testformat, &error);
    snprintf(ofile, sizeof(ofile), "%s.mb%d", fileroot, oformat);
    if (strcmp(ifile, ofile) == 0)
      snprintf(ofile, sizeof(ofile), "%sr.mb%d", fileroot, oformat);

    /* if a different output directory was set by user, reset file path */
    if (output_directory_set) {
      char buffer[MB_PATH_MAXLINE] = "";
      strcpy(buffer, output_directory);
      if (buffer[strlen(output_directory) - 1] != '/')
        strcat(buffer, "/");
      char *filenameptr;
      if (strrchr(ofile, '/') != nullptr)
        filenameptr = strrchr(ofile, '/') + 1;
      else
        filenameptr = ofile;
      strcat(buffer, filenameptr);
      strcpy(ofile, buffer);
    }

    /* Figure out if the file should be preprocessed - don't if it looks like
      the file was previously preprocessed and looks up to date  AND the
      appropriate request has been made */
    bool proceed = true;
    if (skip_existing) {
      if (stat(ifile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
        input_modtime = file_status.st_mtime;
        input_size = file_status.st_size;
      }
      else {
        input_modtime = 0;
        input_size = 0;
      }
      if (stat(ofile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
        output_modtime = file_status.st_mtime;
        output_size = file_status.st_size;
      }
      else {
        output_modtime = 0;
        output_size = 0;
      }
      if (output_modtime > input_modtime && output_size > input_size) {
        proceed = false;
      }
    }

    /* skip redo if requested and relevant */
    if (!proceed) {
      if (verbose > 0)
        fprintf(stderr, "\nPass 2: Skipping input file:  %s %d\n", ifile, iformat);
    }

    /* preprocess the input file */
    else {
      if (verbose > 0)
        fprintf(stderr, "\nPass 2: Opening input file:  %s %d\n", ifile, iformat);

      /* initialize reading the input file */
      if (mb_read_init(verbose, ifile, iformat, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap,
                     &imbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
        char *message;
        mb_error(verbose, error, &message);
        fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
        fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
        fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", ifile);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        exit(error);
      }

      /* call preprocess function with pars settings before reading any data
          - for some formats this can set special read behavior
          - passing store_ptr == NULL indicates this is the pre-reading call
          - if a preprocess function does not exist for this format then
            standard preprocessing will be done - reset the error */
      if (mb_preprocess(verbose, imbio_ptr, NULL, NULL, (void *)&preprocess_pars, &error) == MB_FAILURE) {
        status = MB_SUCCESS;
        error = MB_ERROR_NO_ERROR;
      }

      if (verbose > 0)
        fprintf(stderr, "Pass 2: Opening output file: %s %d\n", ofile, oformat);

      /* initialize writing the output swath file */
      if (mb_write_init(verbose, ofile, oformat, &ombio_ptr, &obeams_bath, &obeams_amp, &opixels_ss, &error) !=
        MB_SUCCESS) {
        char *message;
        mb_error(verbose, error, &message);
        fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
        fprintf(stderr, "\nMBIO Error returned from function <mb_write_init>:\n%s\n", message);
        fprintf(stderr, "\nMultibeam File <%s> not initialized for writing\n", ofile);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        exit(error);
      }

      /* initialize writing the output fast bathymetry *fbt file */
      bool make_fbt = false;
      void *fmbio_ptr = nullptr;
      void *fstore_ptr = nullptr;
      struct mb_io_struct *fmb_io_ptr = nullptr;
      struct mbsys_ldeoih_struct *fstore = nullptr;
      if (mb_should_make_fbt(verbose, oformat)) {
        char fbtfile[MB_PATH_MAXLINE+100];

        snprintf(fbtfile, sizeof(fbtfile), "%s.fbt", ofile);
        int fbeams_bath = 0;
        int fbeams_amp = 0;
        int fpixels_ss = 0;
        if (mb_write_init(verbose, fbtfile, MBF_MBLDEOIH,
                          &fmbio_ptr, &fbeams_bath, &fbeams_amp, &fpixels_ss,
                          &error) != MB_SUCCESS) {
          char *message = nullptr;
          mb_error(verbose, error, &message);
          fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
          fprintf(stderr, "\nMBIO Error returned from function <mb_write_init>:\n%s\n", message);
          fprintf(stderr, "\nMultibeam File <%s> not initialized for writing\n", ofile);
          fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
          exit(error);
        }
        fmb_io_ptr = (struct mb_io_struct *)fmbio_ptr;
        fstore = (struct mbsys_ldeoih_struct *) fmb_io_ptr->store_data;
        fstore_ptr = (void *) fstore;
        make_fbt = true;
      }

      /* initialize writing the output fast navigation *.fnv file */
      bool make_fnv = false;
      FILE *nfp = nullptr;
      if (mb_should_make_fnv(verbose, oformat)) {
        char fnvfile[MB_PATH_MAXLINE+100];
        snprintf(fnvfile, sizeof(fnvfile), "%s.fnv", ofile);
        if ((nfp = fopen(fnvfile, "w")) == nullptr) {
            fprintf(stderr, "\nUnable to open output *.fnv file <%s> for reading\n",
            fnvfile);
            fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
            exit(MB_ERROR_OPEN_FAIL);
        }
        make_fnv = true;
        fprintf(nfp,  "## <yyyy mm dd hh mm ss.ssssss> <epoch seconds> "
                      "<longitude (deg)> <latitude (deg)> <heading (deg)> <speed (km/hr)> "
                      "<draft (m)> <roll (deg)> <pitch (deg)> <heave (m)> <portlon (deg)> "
                      "<portlat (deg)> <stbdlon (deg)> <stbdlat (deg)>\n");
      }

      /* initialize bounds that will be used in call to mbinfo to generate the *.inf file */
      bool mask_bounds_init = false;
      double mask_bounds[4] = {0.0, 0.0, 0.0, 0.0};

      beamflag = nullptr;
      bath = nullptr;
      amp = nullptr;
      bathacrosstrack = nullptr;
      bathalongtrack = nullptr;
      ss = nullptr;
      ssacrosstrack = nullptr;
      ssalongtrack = nullptr;
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
      if (error == MB_ERROR_NO_ERROR)
        status =
          mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, &error);
      if (error == MB_ERROR_NO_ERROR)
        status =
          mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, &error);
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, &error);
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, &error);

      /* if error initializing memory then quit */
      if (error != MB_ERROR_NO_ERROR) {
        char *message;
        mb_error(verbose, error, &message);
        fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
        fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        exit(error);
      }

      /* delete old synchronous and synchronous files */
      snprintf(afile, sizeof(afile), "%s.ata", ofile);
      if (stat(afile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
        if (verbose > 0)
          fprintf(stderr, "Deleting old ancillary file %s\n", afile);
        remove(afile);
      }
      snprintf(afile, sizeof(afile), "%s.ath", ofile);
      if (stat(afile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
        if (verbose > 0)
          fprintf(stderr, "Deleting old ancillary file %s\n", afile);
        remove(afile);
      }
      snprintf(afile, sizeof(afile), "%s.ats", ofile);
      if (stat(afile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
        if (verbose > 0)
          fprintf(stderr, "Deleting old ancillary file %s\n", afile);
        remove(afile);
      }
      snprintf(afile, sizeof(afile), "%s.sta", ofile);
      if (stat(afile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
        if (verbose > 0)
          fprintf(stderr, "Deleting old ancillary file %s\n", afile);
        remove(afile);
      }
      snprintf(afile, sizeof(afile), "%s.baa", ofile);
      if (stat(afile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
        if (verbose > 0)
          fprintf(stderr, "Deleting old ancillary file %s\n", afile);
        remove(afile);
      }
      snprintf(afile, sizeof(afile), "%s.bah", ofile);
      if (stat(afile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
        if (verbose > 0)
          fprintf(stderr, "Deleting old ancillary file %s\n", afile);
        remove(afile);
      }
      snprintf(afile, sizeof(afile), "%s.bas", ofile);
      if (stat(afile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
        if (verbose > 0)
          fprintf(stderr, "Deleting old ancillary file %s\n", afile);
        remove(afile);
      }
      snprintf(afile, sizeof(afile), "%s.bsa", ofile);
      if (stat(afile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
        if (verbose > 0)
          fprintf(stderr, "Deleting old ancillary file %s\n", afile);
        remove(afile);
      }

      /* open synchronous attitude file */
      snprintf(afile, sizeof(afile), "%s.bsa", ofile);
      if ((afp = fopen(afile, "wb")) == nullptr) {
        fprintf(stderr, "\nUnable to open synchronous attitude data file <%s> for writing\n", afile);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        exit(MB_ERROR_OPEN_FAIL);
      }

      /* zero file count records */
      n_rf_data = 0;
      n_rf_comment = 0;
      n_rf_nav = 0;
      n_rf_nav1 = 0;
      n_rf_nav2 = 0;
      n_rf_nav3 = 0;
      n_rf_att = 0;
      n_rf_att1 = 0;
      n_rf_att2 = 0;
      n_rf_att3 = 0;
      n_wf_data = 0;
      n_wf_comment = 0;
      n_wf_nav = 0;
      n_wf_nav1 = 0;
      n_wf_nav2 = 0;
      n_wf_nav3 = 0;
      n_wf_att = 0;
      n_wf_att1 = 0;
      n_wf_att2 = 0;
      n_wf_att3 = 0;
      start_time_d = -1.0;
      end_time_d = -1.0;

      if (kluge_fix_wissl_timestamps)
        kluge_fix_wissl_timestamps_setup2 = false;

      /* ------------------------------- */
      /* write comments to output file   */

      /* ------------------------------- */
      /* start read+process,+output loop */
      while (error <= MB_ERROR_NO_ERROR) {
        /* reset error */
        status = MB_SUCCESS;
        error = MB_ERROR_NO_ERROR;

        /* read next data record */
        status = mb_get_all(verbose, imbio_ptr, &istore_ptr, &kind, time_i, &time_d, &navlon_org, &navlat_org, &speed_org,
                  &heading_org, &distance, &altitude_org, &sensordepth_org, &beams_bath, &beams_amp, &pixels_ss,
                  beamflag, bath, amp, bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment,
                  &error);

        /* some nonfatal errors do not matter */
        if (error < MB_ERROR_NO_ERROR && error > MB_ERROR_UNINTELLIGIBLE) {
          error = MB_ERROR_NO_ERROR;
          status = MB_SUCCESS;
        }

        /* detect multiple pings with the same time stamps */
        int sensorhead = 0;
        int sensortype = 0;
        if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
          int sensorhead_error = MB_ERROR_NO_ERROR;
          mb_sensorhead(verbose, imbio_ptr, istore_ptr, &sensorhead, &sensorhead_error);
          mb_sonartype(verbose, imbio_ptr, istore_ptr, &sensortype, &sensorhead_error);
        }

        /* count records */
        if (kind == MB_DATA_DATA) {
          if (n_rf_data == 0)
            start_time_d = time_d;
          end_time_d = time_d;
          n_rf_data++;
          n_rt_data++;
        }
        else if (kind == MB_DATA_COMMENT) {
          n_rf_comment++;
          n_rt_comment++;
        }
        else if (kind == MB_DATA_NAV) {
          n_rf_nav++;
          n_rt_nav++;
        }
        else if (kind == MB_DATA_NAV1) {
          n_rf_nav1++;
          n_rt_nav1++;
        }
        else if (kind == MB_DATA_NAV2) {
          n_rf_nav2++;
          n_rt_nav2++;
        }
        else if (kind == MB_DATA_NAV3) {
          n_rf_nav3++;
          n_rt_nav3++;
        }
        else if (kind == MB_DATA_ATTITUDE) {
          n_rf_att++;
          n_rt_att++;
        }
        else if (kind == MB_DATA_ATTITUDE1) {
          n_rf_att1++;
          n_rt_att1++;
        }
        else if (kind == MB_DATA_ATTITUDE2) {
          n_rf_att2++;
          n_rt_att2++;
        }
        else if (kind == MB_DATA_ATTITUDE3) {
          n_rf_att3++;
          n_rt_att3++;
        }

        /* apply preprocessing to survey data records */
        if (status == MB_SUCCESS &&
          (kind == MB_DATA_DATA || kind == MB_DATA_SUBBOTTOM_MCS || kind == MB_DATA_SUBBOTTOM_CNTRBEAM ||
           kind == MB_DATA_SUBBOTTOM_SUBBOTTOM || kind == MB_DATA_SIDESCAN2 || kind == MB_DATA_SIDESCAN3 ||
           kind == MB_DATA_WATER_COLUMN)) {
          /* call mb_extract_nav to get attitude */
          status = mb_extract_nav(verbose, imbio_ptr, istore_ptr, &kind, time_i, &time_d, &navlon_org, &navlat_org,
                      &speed_org, &heading_org, &draft_org, &roll_org, &pitch_org, &heave_org, &error);

          /* call mb_extract_altitude to get altitude */
          status &= mb_extract_altitude(verbose, imbio_ptr, istore_ptr, &kind, &sensordepth_org, &altitude_org, &error);

          /* apply time jump fix to survey record time stamps */
          bool timestamp_changed = false;
          double dtime_d_expect = 0.0;
          double dtime_d_raw = 0.0;
          double dtime_d = 0.0;
          double time_d_raw = 0.0;
//fprintf(stderr, "\n%s:%d:%s: kluge_timejumps:%d n_rt_data:%d kluge_first_time_d:%f kluge_last_time_d:%f time_d:%f dtime_d:%f\n",
//__FILE__, __LINE__, __FUNCTION__,
//kluge_timejumps, n_rt_data, kluge_first_time_d, kluge_last_time_d, time_d, time_d - kluge_last_raw_time_d);
          if (kluge_timejumps) {
            if (kind == MB_DATA_DATA) {
              if (n_rt_data == 1)
                kluge_first_time_d = time_d;
              time_d_raw = time_d;
            }
            if (n_rt_data > 2) {
              dtime_d_expect = (kluge_last_time_d - kluge_first_time_d) / (n_rt_data - 2);
              dtime_d_raw = time_d - kluge_last_raw_time_d;
              dtime_d = time_d - kluge_last_time_d;
              if (fabs(dtime_d - dtime_d_expect) >= kluge_timejumps_threshold) {
                if (fabs(dtime_d_raw - dtime_d_expect) >= kluge_timejumps_threshold) {
                  time_d = kluge_last_time_d + dtime_d_expect;
                  timestamp_changed = true;
//fprintf(stderr, "%s:%d:%s: kluge_timejumps:%d n_rt_data:%d kluge_first_time_d:%f kluge_last_time_d:%f time_d:%f dtime_d:%f dtime_d_raw:%f  dtime_d_expect:%f* change:%f\n",
//__FILE__, __LINE__, __FUNCTION__,
//kluge_timejumps, n_rt_data, kluge_first_time_d, kluge_last_time_d, time_d, dtime_d, dtime_d_raw, dtime_d_expect, time_d - time_d_raw);
                } else {
                  time_d = kluge_last_time_d + dtime_d_raw;
                  timestamp_changed = true;
//fprintf(stderr, "%s:%d:%s: kluge_timejumps:%d n_rt_data:%d kluge_first_time_d:%f kluge_last_time_d:%f time_d:%f dtime_d:%f dtime_d_raw:%f* dtime_d_expect:%f  change:%f\n",
//__FILE__, __LINE__, __FUNCTION__,
//kluge_timejumps, n_rt_data, kluge_first_time_d, kluge_last_time_d, time_d, dtime_d, dtime_d_raw, dtime_d_expect, time_d - time_d_raw);
                }
              }
            }
            if (kind == MB_DATA_DATA) {
              dtime_d = time_d - kluge_last_time_d;
              kluge_last_time_d = time_d;
              kluge_last_raw_time_d = time_d_raw;
            }
          }
//fprintf(stderr, "%s:%d:%s: kluge_timejumps:%d n_rt_data:%d kluge_first_time_d:%f kluge_last_time_d:%f time_d:%f dtime_d:%f\n",
//__FILE__, __LINE__, __FUNCTION__,
//kluge_timejumps, n_rt_data, kluge_first_time_d, kluge_last_time_d, time_d, dtime_d);

          /* if the input data are WiSSL data in format MBF_3DWISSLR
           * and kluge_fix_wissl_timestamps is enabled, call special function
           * to fix the timestmps in the file's internal index table */
          if (kind == MB_DATA_DATA && iformat == MBF_3DWISSLR
            && kluge_fix_wissl_timestamps) {
            if (!kluge_fix_wissl_timestamps_setup1) {
                status &= mb_indextablefix(verbose, imbio_ptr,
                                          num_indextable, indextable,
                                          &error);
                kluge_fix_wissl_timestamps_setup1 = true;
            }
            if (!kluge_fix_wissl_timestamps_setup2) {
                status = mb_indextableapply(verbose, imbio_ptr,
                                            num_indextable, indextable,
                                            n_rt_files, &error);
                kluge_fix_wissl_timestamps_setup2 = true;
            }
          }

          /* apply time latency correction called for in the platform file */

          if (sensor_target != nullptr && sensor_target->time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) {
            mb_apply_time_latency(verbose, 1, &time_d, sensor_target->time_latency_mode,
                        sensor_target->time_latency_static, sensor_target->num_time_latency,
                        sensor_target->time_latency_time_d, sensor_target->time_latency_value, &error);
            timestamp_changed = true;
          }

          /* apply time latency correction called for on the command line */
          if ((time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) &&
            (time_latency_apply & MBPREPROCESS_TIME_LATENCY_APPLY_SURVEY)) {
            mb_apply_time_latency(verbose, 1, &time_d, time_latency_mode, time_latency_constant, time_latency_num,
                        time_latency_time_d, time_latency_time_latency, &error);
            timestamp_changed = true;
          }

          /* use available asynchronous ancillary data to replace
            nav sensordepth heading attitude values for record timestamp  */
          // int interp_status = MB_SUCCESS;
          bool nav_changed = false;
          if (n_nav > 0) {
            /* interp_status = */ mb_linear_interp_longitude(verbose, nav_time_d - 1, nav_navlon - 1, n_nav, time_d,
                                  &navlon_org, &jnav, &interp_error);
            /* interp_status = */ mb_linear_interp_latitude(verbose, nav_time_d - 1, nav_navlat - 1, n_nav, time_d, &navlat_org,
                                  &jnav, &interp_error);
            /* interp_status = */
            mb_linear_interp(verbose, nav_time_d - 1, nav_speed - 1, n_nav, time_d, &speed_org, &jnav, &interp_error);
            nav_changed = true;
          }
          bool sensordepth_changed = false;
          if (n_sensordepth > 0) {
            /* interp_status = */ mb_linear_interp(verbose, sensordepth_time_d - 1, sensordepth_sensordepth - 1, n_sensordepth,
                             time_d, &sensordepth_org, &jsensordepth, &interp_error);
            sensordepth_changed = true;
          }
          bool heading_changed = false;
          if (n_heading > 0) {
            /* interp_status = */ mb_linear_interp_heading(verbose, heading_time_d - 1, heading_heading - 1, n_heading, time_d,
                                 &heading_org, &jheading, &interp_error);
            heading_changed = true;
          }
          bool altitude_changed = false;
          if (n_altitude > 0) {
            /* interp_status = */ mb_linear_interp(verbose, altitude_time_d - 1, altitude_altitude - 1, n_altitude, time_d,
                             &altitude_org, &jaltitude, &interp_error);
            altitude_changed = true;
          }
          bool attitude_changed = false;
          if (n_attitude > 0) {
            /* interp_status = */ mb_linear_interp(verbose, attitude_time_d - 1, attitude_roll - 1, n_attitude, time_d,
                             &roll_org, &jattitude, &interp_error);
            /* interp_status = */ mb_linear_interp(verbose, attitude_time_d - 1, attitude_pitch - 1, n_attitude, time_d,
                             &pitch_org, &jattitude, &interp_error);
            /* interp_status = */ mb_linear_interp(verbose, attitude_time_d - 1, attitude_heave - 1, n_attitude, time_d,
                             &heave_org, &jattitude, &interp_error);
            attitude_changed = true;
          }
          if (n_sensordepth > 0 || n_attitude > 0) {
            draft_org = sensordepth_org - heave_org;
          }

          /* save the original values prior to lever arm correction */
          navlon = navlon_org;
          navlat = navlat_org;
          speed = speed_org;
          heading = heading_org;
          altitude = altitude_org;
          sensordepth = sensordepth_org;
          draft = draft_org;
          roll = roll_org;
          pitch = pitch_org;
          heave = heave_org;

          /* set up preprocess structure */
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
//fprintf(stderr, "\n%s:%d:%s: n_sensordepth: %d %d\n", __FILE__, __LINE__, __FUNCTION__, 
//preprocess_pars.n_sensordepth, n_sensordepth);

          /* attempt to execute a preprocess function for these data */
          status = mb_preprocess(verbose, imbio_ptr, istore_ptr, (void *)platform, (void *)&preprocess_pars, &error);
//fprintf(stderr, "%s:%d:%s: n_sensordepth: %d %d\n\n", __FILE__, __LINE__, __FUNCTION__, 
//preprocess_pars.n_sensordepth, n_sensordepth);

          /* If a predefined preprocess function does not exist for
           * this format then standard preprocessing will be done
           *      1) Replace time tag, nav, attitude
           *   2) if attitude values changed rotate bathymetry accordingly
           *   3) if any values changed reinsert the data */
          if (status == MB_FAILURE) {
            /* reset status and error */
            status = MB_SUCCESS;
            error = MB_ERROR_NO_ERROR;

            /* if platform defined, do lever arm correction */
            if (platform != nullptr) {
              /* calculate target sensor position */
              status = mb_platform_position(verbose, (void *)platform, target_sensor, 0, navlon, navlat, sensordepth,
                              heading, roll, pitch, &navlon, &navlat, &sensordepth, &error);
              draft = sensordepth - heave;
              nav_changed = true;
              sensordepth_changed = true;

              /* calculate target sensor attitude */
              status = mb_platform_orientation_target(verbose, (void *)platform, target_sensor, 0, heading, roll, pitch,
                                  &heading, &roll, &pitch, &error);
              roll_delta = roll - roll_org;
              pitch_delta = pitch - pitch_org;
              if (roll_delta != 0.0 || pitch_delta != 0.0)
                attitude_changed = true;
            }

            /* if attitude changed apply rigid rotations to any bathymetry */
            if (attitude_changed) {
              /* loop over the beams */
              for (int i = 0; i < beams_bath; i++) {
                if (beamflag[i] != MB_FLAG_NULL) {
                  /* strip off original heave + draft */
                  bath[i] -= sensordepth_org;
                  /* rotate beam by
                     rolldelta:  Roll relative to previous correction and bias included
                     pitchdelta: Pitch relative to previous correction and bias included
                     heading:    Heading absolute (bias included) */
                  mb_platform_math_attitude_rotate_beam(verbose, bathacrosstrack[i], bathalongtrack[i], bath[i],
                                      roll_delta, pitch_delta, 0.0, &(bathacrosstrack[i]),
                                      &(bathalongtrack[i]), &(bath[i]), &error);

                  /* add heave and draft back in */
                  bath[i] += sensordepth_org;
                }
              }
            }

            /* recalculate bathymetry by changes to sensor depth  */
            if (sensordepth_changed) {
              /* get draft change */
              depth_offset_change = draft - draft_org;

              /* loop over the beams */
              for (int i = 0; i < beams_bath; i++) {
                if (beamflag[i] != MB_FLAG_NULL) {
                  /* apply transducer depth change to depths */
                  bath[i] += depth_offset_change;
                }
              }
            }

            /* insert navigation */
            if (timestamp_changed || nav_changed || heading_changed ||
              sensordepth_changed || attitude_changed) {
              status = mb_insert_nav(verbose, imbio_ptr, istore_ptr, time_i, time_d, navlon, navlat, speed, heading,
                           draft, roll, pitch, heave, &error);
            }

            /* insert altitude */
            if (altitude_changed) {
              status = mb_insert_altitude(verbose, imbio_ptr, istore_ptr, sensordepth, altitude, &error);
              if (status == MB_FAILURE) {
                status = MB_SUCCESS;
                error = MB_ERROR_NO_ERROR;
              }
            }

            /* if attitude changed apply rigid rotations to the bathymetry */
            if (!preprocess_pars.no_change_survey &&
              (attitude_changed || sensordepth_changed)) {
              status = mb_insert(verbose, imbio_ptr, istore_ptr, kind, time_i, time_d, navlon, navlat, speed, heading,
                         beams_bath, beams_amp, pixels_ss, beamflag, bath, amp, bathacrosstrack, bathalongtrack,
                         ss, ssacrosstrack, ssalongtrack, comment, &error);
            }
          }
        }

        /* write some data */
        if (error == MB_ERROR_NO_ERROR) {
          status = mb_put_all(verbose, ombio_ptr, istore_ptr, false, kind, time_i, time_d, navlon, navlat, speed, heading,
                    obeams_bath, obeams_amp, opixels_ss, beamflag, bath, amp, bathacrosstrack, bathalongtrack, ss,
                    ssacrosstrack, ssalongtrack, comment, &error);
          if (status != MB_SUCCESS) {
            char *message;
            mb_error(verbose, error, &message);
            fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__, __FUNCTION__);
            fprintf(stderr, "\nMBIO Error returned from function <mb_put>:\n%s\n", message);
            fprintf(stderr, "\nMultibeam Data Not Written To File <%s>\n", ofile);
            fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
            exit(error);
          }

          // output ancilliary files
          if (kind == MB_DATA_DATA) {

            status = mb_extract(verbose, ombio_ptr, istore_ptr, &kind, time_i, &time_d,
                                &navlon, &navlat, &speed, &heading,
                                &obeams_bath, &obeams_amp, &opixels_ss,
                                beamflag, bath, amp, bathacrosstrack, bathalongtrack,
                                ss, ssacrosstrack, ssalongtrack, comment, &error);
            status = mb_extract_nav(verbose, ombio_ptr, istore_ptr, &kind, time_i, &time_d,
                                &navlon, &navlat, &speed, &heading, &draft,
                                &roll, &pitch, &heave, &error);
            status = mb_extract_altitude(verbose, ombio_ptr, istore_ptr, &kind,
                                &sensordepth, &altitude, &error);


            /* output fbt */
            if (make_fbt) {
              fstore->sensorhead = sensorhead;
              fstore->topo_type = sensortype;
              struct mb_io_struct *imb_io_ptr = (struct mb_io_struct *)imbio_ptr;
              fstore->beam_xwidth = imb_io_ptr->beamwidth_xtrack;
              fstore->beam_lwidth = imb_io_ptr->beamwidth_ltrack;
              fstore->kind = kind;
              mb_insert_nav(verbose, fmbio_ptr, fstore_ptr, time_i, time_d,
                            navlon, navlat, speed, heading, draft,
                            roll, pitch, heave, &error);
              mb_insert_altitude(verbose, fmbio_ptr, fstore_ptr, draft, altitude, &error);
              status = mb_insert(verbose, fmbio_ptr, fstore_ptr, kind, time_i, time_d,
                                  navlon, navlat, speed, heading, obeams_bath, obeams_amp, opixels_ss,
                                  beamflag, bath, amp, bathacrosstrack, bathalongtrack,
                                  ss, ssacrosstrack, ssalongtrack, comment, &error);
              status &= mb_put_all(verbose, fmbio_ptr, fstore_ptr, false,
                                  kind, time_i, time_d, navlon, navlat, speed,
                                  heading, obeams_bath, 0, 0,
                                  beamflag, bath, nullptr, bathacrosstrack, bathalongtrack,
                                  nullptr, nullptr, nullptr, comment, &error);
            }

            // get scaling for both fnv and inf calculations
            double mtodeglon;
            double mtodeglat;
            mb_coor_scale(verbose, navlat, &mtodeglon, &mtodeglat);
            const double headingx = sin(heading * DTR);
            const double headingy = cos(heading * DTR);

            /* output fnv */
            /* mblist output: tMXYHScRPr=X=Y+X+Y */
            if (make_fnv) {
              double seconds = time_i[5] + 1e-6 * time_i[6];
              int beam_port;
              int beam_vertical;
              int beam_stbd;
              int pixel_port;
              int pixel_vertical;
              int pixel_stbd;
              status = mb_swathbounds(verbose, true, obeams_bath, 0,
                                  beamflag, bathacrosstrack, nullptr, nullptr,
                                  &beam_port, &beam_vertical, &beam_stbd,
                                  &pixel_port, &pixel_vertical, &pixel_stbd, &error);
              const double portlon = navlon
                                + headingy * mtodeglon * bathacrosstrack[beam_port]
                                + headingx * mtodeglon * bathalongtrack[beam_port];
              const double portlat = navlat
                                - headingx * mtodeglat * bathacrosstrack[beam_port]
                                + headingy * mtodeglat * bathalongtrack[beam_port];
              const double stbdlon = navlon
                                + headingy * mtodeglon * bathacrosstrack[beam_stbd]
                                + headingx * mtodeglon * bathalongtrack[beam_stbd];
              const double stbdlat = navlat
                                - headingx * mtodeglat * bathacrosstrack[beam_stbd]
                                + headingy * mtodeglat * bathalongtrack[beam_stbd];

              fprintf(nfp,
                      "%.4d %.2d %.2d %.2d %.2d %09.6f\t%.6f\t"
                      "%15.10f\t%15.10f\t%7.3f\t%6.3f\t%.4f\t%6.3f\t%6.3f\t%7.4f\t"
                      "%15.10f\t%15.10f\t%15.10f\t%15.10f\n",
                      time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], seconds,
                      time_d, navlon, navlat, heading, speed, sensordepth, roll, pitch, heave,
                      portlon, portlat, stbdlon, stbdlat);
            }

            /* get bounds for mbinfo call to generate the *.inf file
                - use only data with good navigation and valid soundings or pixels */
            if (fabs(navlon) >= 0.005 || fabs(navlat) >= 0.005) {
              if (mask_bounds_init) {
                mask_bounds[0] = std::min(mask_bounds[0], navlon);
                mask_bounds[1] = std::max(mask_bounds[1], navlon);
                mask_bounds[2] = std::min(mask_bounds[2], navlat);
                mask_bounds[3] = std::max(mask_bounds[3], navlat);
              } else {
                mask_bounds[0] = navlon;
                mask_bounds[1] = navlon;
                mask_bounds[2] = navlat;
                mask_bounds[3] = navlat;
                mask_bounds_init = true;
              }
              for (int i=0; i<obeams_bath; i++) {
                if (mb_beam_ok(beamflag[i])) {
                  double bathlon = navlon
                              + headingy * mtodeglon * bathacrosstrack[i]
                              + headingx * mtodeglon * bathalongtrack[i];
                  double bathlat = navlat
                              - headingx * mtodeglat * bathacrosstrack[i]
                              + headingy * mtodeglat * bathalongtrack[i];

                  mask_bounds[0] = std::min(mask_bounds[0], bathlon);
                  mask_bounds[1] = std::max(mask_bounds[1], bathlon);
                  mask_bounds[2] = std::min(mask_bounds[2], bathlat);
                  mask_bounds[3] = std::max(mask_bounds[3], bathlat);
                }
              }
              for (int i=0; i<opixels_ss; i++) {
                if (ss[i] > MB_SIDESCAN_NULL) {
                  double sslon = navlon
                              + headingy * mtodeglon * ssacrosstrack[i]
                              + headingx * mtodeglon * ssalongtrack[i];
                  double sslat = navlat
                              - headingx * mtodeglat * ssacrosstrack[i]
                              + headingy * mtodeglat * ssalongtrack[i];
                  mask_bounds[0] = std::min(mask_bounds[0], sslon);
                  mask_bounds[1] = std::max(mask_bounds[1], sslon);
                  mask_bounds[2] = std::min(mask_bounds[2], sslat);
                  mask_bounds[3] = std::max(mask_bounds[3], sslat);
                }
              }
            }

            /* output synchronous attitude */
            int index = 0;
            mb_put_binary_double(true, time_d, &buffer[index]);
            index += 8;
            mb_put_binary_float(true, (float)roll, &buffer[index]);
            index += 4;
            mb_put_binary_float(true, (float)pitch, &buffer[index]);
            index += 4;
            fwrite(buffer, (size_t)index, 1, afp);
          }

          /* count records */
          if (kind == MB_DATA_DATA) {
            n_wf_data++;
            n_wt_data++;
          }
          else if (kind == MB_DATA_COMMENT) {
            n_wf_comment++;
            n_wt_comment++;
          }
          else if (kind == MB_DATA_NAV) {
            n_wf_nav++;
            n_wt_nav++;
          }
          else if (kind == MB_DATA_NAV1) {
            n_wf_nav1++;
            n_wt_nav1++;
          }
          else if (kind == MB_DATA_NAV2) {
            n_wf_nav2++;
            n_wt_nav2++;
          }
          else if (kind == MB_DATA_NAV3) {
            n_wf_nav3++;
            n_wt_nav3++;
          }
          else if (kind == MB_DATA_ATTITUDE) {
            n_wf_att++;
            n_wt_att++;
          }
          else if (kind == MB_DATA_ATTITUDE1) {
            n_wf_att1++;
            n_wt_att1++;
          }
          else if (kind == MB_DATA_ATTITUDE2) {
            n_wf_att2++;
            n_wt_att2++;
          }
          else if (kind == MB_DATA_ATTITUDE3) {
            n_wf_att3++;
            n_wt_att3++;
          }
        }

        /* if requested output integrated nav */
        if (output_sensor_fnv && status == MB_SUCCESS && kind == MB_DATA_DATA) {
          /* loop over all sensors and output integrated nav for all
            sensors producing mapping data */
          for (isensor = 0; isensor < platform->num_sensors; isensor++) {
            //if (platform->sensors[isensor].capability2 != 0) {
              for (ioffset = 0; ioffset < platform->sensors[isensor].num_offsets; ioffset++) {
                if (platform->sensors[isensor].offsets[ioffset].ofp != nullptr) {
                  /* calculate position and attitude of target sensor */
                  status = mb_platform_position(verbose, (void *)platform, isensor, ioffset, navlon_org, navlat_org,
                                  sensordepth_org, heading_org, roll_org, pitch_org, &navlon, &navlat,
                                  &sensordepth, &error);
                  draft = sensordepth - heave;
                  status &= mb_platform_orientation_target(verbose, (void *)platform, isensor, ioffset, heading_org,
                                      roll_org, pitch_org, &heading, &roll, &pitch, &error);

                  /* output integrated navigation */
                  fprintf(platform->sensors[isensor].offsets[ioffset].ofp,
                      "%4.4d %2.2d %2.2d %2.2d %2.2d "
                      "%2.2d.%6.6d\t%.6f\t%.10f\t%.10f\t%.3f\t%.3f\t%.4f\t%.3f\t%.3f\t%.3f\n",
                      time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], time_d,
                      navlon, navlat, heading, speed, draft, roll, pitch, heave);
                }
              }
            //}
          }
        }
      }

      /* end read+process+output data loop */
      /* --------------------------------- */

      /* output data counts */
      if (verbose > 0) {
        fprintf(stderr, "Pass 2: Records read from input file %d: %s\n", n_rt_files, ifile);
        fprintf(stderr, "     %d survey records\n", n_rf_data);
        fprintf(stderr, "     %d comment records\n", n_rf_comment);
        fprintf(stderr, "     %d nav records\n", n_rf_nav);
        fprintf(stderr, "     %d nav1 records\n", n_rf_nav1);
        fprintf(stderr, "     %d nav2 records\n", n_rf_nav2);
        fprintf(stderr, "     %d nav3 records\n", n_rf_nav3);
        fprintf(stderr, "     %d att records\n", n_rf_att);
        fprintf(stderr, "     %d att1 records\n", n_rf_att1);
        fprintf(stderr, "     %d att2 records\n", n_rf_att2);
        fprintf(stderr, "     %d att3 records\n", n_rf_att3);
        fprintf(stderr, "Pass 2: Records written to output file %d: %s\n", n_wt_files, ofile);
        fprintf(stderr, "     %d survey records\n", n_wf_data);
        fprintf(stderr, "     %d comment records\n", n_wf_comment);
        fprintf(stderr, "     %d nav records\n", n_wf_nav);
        fprintf(stderr, "     %d nav1 records\n", n_wf_nav1);
        fprintf(stderr, "     %d nav2 records\n", n_wf_nav2);
        fprintf(stderr, "     %d nav3 records\n", n_wf_nav3);
        fprintf(stderr, "     %d att records\n", n_wf_att);
        fprintf(stderr, "     %d att1 records\n", n_wf_att1);
        fprintf(stderr, "     %d att2 records\n", n_wf_att2);
        fprintf(stderr, "     %d att3 records\n", n_wf_att3);
      }

      /* close the input ("logged") swath file */
      status &= mb_close(verbose, &imbio_ptr, &error);
      n_rt_files++;

      /* close the output ("raw") swath file */
      status &= mb_close(verbose, &ombio_ptr, &error);
      n_wt_files++;

      // close the output fbt file
      if (make_fbt)
        status &= mb_close(verbose, &fmbio_ptr, &error);

      //close the output fnv file
      if (make_fnv)
        fclose(nfp);

      // use mbinfo to generate the inf file - specify the mask bounds so that
      // only one read pass is necessary
      char command[MB_PATH_MAXLINE+100];
      // TODO(schwehr): Is is possible to have mask_bounds[0] not set.
      snprintf(command, sizeof(command), "mbinfo -F %d -I %s -G -N -O -M10/10/%.9f/%.9f/%.9f/%.9f",
              oformat, ofile,
              mask_bounds[0], mask_bounds[1], mask_bounds[2], mask_bounds[3]);
      system(command);

      /* close the synchronous attitude file */
      fclose(afp);

      /* if success then generate other ancillary files */
      if (status == MB_SUCCESS) {

        /* generate asynchronous heading file */
        if (n_heading > 0) {
          /* use only the samples relevant to survey data for this file, but
           * allow 10 seconds before and after to insure time latency corrections
           * do not overshoot the data */
          istart = 0;
          iend = 0;
          for (int i = 0; i < n_heading; i++) {
            if (heading_time_d[i] < start_time_d - 10.0)
              istart = i;
            if (heading_time_d[i] < end_time_d + 10.0)
              iend = i;
          }
          if (iend < n_heading - 1)
            iend++;
          if (iend > istart) {
            snprintf(afile, sizeof(afile), "%s.bah", ofile);
            if ((afp = fopen(afile, "wb")) == nullptr) {
              fprintf(stderr, "\nUnable to open asynchronous heading data file <%s> for writing\n", afile);
              fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
              exit(MB_ERROR_OPEN_FAIL);
            }
            if (verbose > 0)
              fprintf(stderr, "Generating bah file for %s using samples %d:%d out of %d\n", ofile, istart, iend, n_heading);
            for (int i = istart; i < iend; i++) {
              int index = 0;
              mb_put_binary_double(true, heading_time_d[i], &buffer[index]);
              index += 8;
              mb_put_binary_float(true, (float)heading_heading[i], &buffer[index]);
              index += 4;
              fwrite(buffer, (size_t)index, 1, afp);
            }
            fclose(afp);
          }
        }

        /* generate asynchronous sensordepth file */
        if (n_sensordepth > 0) {
          /* use only the samples relevant to survey data for this file, but
           * allow 10 seconds before and after to insure time latency corrections
           * do not overshoot the data */
          istart = 0;
          iend = 0;
          for (int i = 0; i < n_sensordepth; i++) {
            if (sensordepth_time_d[i] < start_time_d - 10.0)
              istart = i;
            if (sensordepth_time_d[i] < end_time_d + 10.0)
              iend = i;
          }
          if (iend < n_sensordepth - 1)
            iend++;
          if (iend > istart) {
            snprintf(afile, sizeof(afile), "%s.bas", ofile);
            if ((afp = fopen(afile, "wb")) == nullptr) {
              fprintf(stderr, "\nUnable to open asynchronous sensordepth data file <%s> for writing\n", afile);
              fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
              exit(MB_ERROR_OPEN_FAIL);
            }
            if (verbose > 0)
              fprintf(stderr, "Generating bas file for %s using samples %d:%d out of %d\n", ofile, istart, iend,
                n_sensordepth);
            for (int i = istart; i < iend; i++) {
              int index = 0;
              mb_put_binary_double(true, sensordepth_time_d[i], &buffer[index]);
              index += 8;
              mb_put_binary_float(true, (float)sensordepth_sensordepth[i], &buffer[index]);
              index += 4;
              fwrite(buffer, (size_t)index, 1, afp);
            }
            fclose(afp);
          }
        }

        /* generate asynchronous attitude file */
        if (n_attitude > 0) {
          /* use only the samples relevant to survey data for this file, but
           * allow 10 seconds before and after to insure time latency corrections
           * do not overshoot the data */
          istart = 0;
          iend = 0;
          for (int i = 0; i < n_attitude; i++) {
            if (attitude_time_d[i] < start_time_d - 10.0)
              istart = i;
            if (attitude_time_d[i] < end_time_d + 10.0)
              iend = i;
          }
          if (iend < n_attitude - 1)
            iend++;
          if (iend > istart) {
            snprintf(afile, sizeof(afile), "%s.baa", ofile);
            if ((afp = fopen(afile, "wb")) == nullptr) {
              fprintf(stderr, "\nUnable to open asynchronous attitude data file <%s> for writing\n", afile);
              fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
              exit(MB_ERROR_OPEN_FAIL);
            }
            if (verbose > 0)
              fprintf(stderr, "Generating baa file for %s using samples %d:%d out of %d\n", ofile, istart, iend,
                n_attitude);
            for (int i = istart; i < iend; i++) {
              int index = 0;
              mb_put_binary_double(true, attitude_time_d[i], &buffer[index]);
              index += 8;
              mb_put_binary_float(true, (float)attitude_roll[i], &buffer[index]);
              index += 4;
              mb_put_binary_float(true, (float)attitude_pitch[i], &buffer[index]);
              index += 4;
              fwrite(buffer, (size_t)index, 1, afp);
            }
            fclose(afp);
          }
        }
      }
    }

    /* figure out whether and what to read next */
    if (read_datalist) {
      if (mb_datalist_read(verbose, datalist, ifile, dfile, &format, &file_weight, &error) == MB_SUCCESS)
        read_data = true;
      else
        read_data = false;
    }
    else {
      read_data = false;
    }

    /* end loop over files in list */
  }
  if (read_datalist)
    mb_datalist_close(verbose, &datalist, &error);

  /* output data counts */
  if (verbose > 0) {
    fprintf(stderr, "\nPass 2: Total records read from %d input files\n", n_rt_files);
    fprintf(stderr, "     %d survey records\n", n_rt_data);
    fprintf(stderr, "     %d comment records\n", n_rt_comment);
    fprintf(stderr, "     %d nav records\n", n_rt_nav);
    fprintf(stderr, "     %d nav1 records\n", n_rt_nav1);
    fprintf(stderr, "     %d nav2 records\n", n_rt_nav2);
    fprintf(stderr, "     %d nav3 records\n", n_rt_nav3);
    fprintf(stderr, "     %d att records\n", n_rt_att);
    fprintf(stderr, "     %d att1 records\n", n_rt_att1);
    fprintf(stderr, "     %d att2 records\n", n_rt_att2);
    fprintf(stderr, "     %d att3 records\n", n_rt_att3);
    fprintf(stderr, "Pass 2: Total records written to %d output files\n", n_wt_files);
    fprintf(stderr, "     %d survey records\n", n_wt_data);
    fprintf(stderr, "     %d comment records\n", n_wt_comment);
    fprintf(stderr, "     %d nav records\n", n_wt_nav);
    fprintf(stderr, "     %d nav1 records\n", n_wt_nav1);
    fprintf(stderr, "     %d nav2 records\n", n_wt_nav2);
    fprintf(stderr, "     %d nav3 records\n", n_wt_nav3);
    fprintf(stderr, "     %d att records\n", n_wt_att);
    fprintf(stderr, "     %d att1 records\n", n_wt_att1);
    fprintf(stderr, "     %d att2 records\n", n_wt_att2);
    fprintf(stderr, "     %d att3 records\n", n_wt_att3);
  }

  /* end second pass through data */

  /*-------------------------------------------------------------------*/

  /* close any integrated navigation files */
  if (output_sensor_fnv) {
    for (isensor = 0; isensor < platform->num_sensors; isensor++) {
      //if (platform->sensors[isensor].capability2 != 0) {
        for (ioffset = 0; ioffset < platform->sensors[isensor].num_offsets; ioffset++) {
          if (platform->sensors[isensor].offsets[ioffset].ofp != nullptr) {
            fclose(platform->sensors[isensor].offsets[ioffset].ofp);
            platform->sensors[isensor].offsets[ioffset].ofp = nullptr;
          }
        }
      //}
    }
  }

  /* deallocate nav, sensordepth, heading, attitude, and time_latency arrays */
  if (n_nav_alloc > 0) {
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&nav_time_d, &error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&nav_navlon, &error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&nav_navlat, &error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&nav_speed, &error);
  }
  if (n_sensordepth_alloc > 0) {
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&sensordepth_time_d, &error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&sensordepth_sensordepth, &error);
  }
  if (n_heading_alloc > 0) {
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&heading_time_d, &error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&heading_heading, &error);
  }
  if (n_attitude_alloc > 0) {
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&attitude_time_d, &error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&attitude_roll, &error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&attitude_pitch, &error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&attitude_heave, &error);
  }
  if (time_latency_alloc > 0) {
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&time_latency_time_d, &error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&time_latency_time_latency, &error);
  }

  /* deallocate platform structure */
  if (platform != nullptr) {
    status &= mb_platform_deall(verbose, (void **)&platform, &error);
  }

  /* check memory */
  if (verbose >= 4)
    status &= mb_memory_list(verbose, &error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Program <%s> completed\n", program_name);
    fprintf(stderr, "dbg2  Ending status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  exit(error);
}
/*--------------------------------------------------------------------*/
