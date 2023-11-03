/*--------------------------------------------------------------------
 *    The MB-system:	mbsslayout.c	1/8/2014
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
 * MBsslayout reads sidescan in raw time series form, lays the sidescan
 * out regularly sampled on a specified topography model, and outputs
 * the sidescan to format 71 (MBF_MBLDEOIH) files.
 *
 * Author:	D. W. Caress
 * Date:	April 21, 2014
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_ldeoih.h"

constexpr int MBSSLAYOUT_ALLOC_CHUNK = 1024;
constexpr int MBSSLAYOUT_ALLOC_NUM = 128;

constexpr int MBSSLAYOUT_LINE_OFF = 0;
constexpr int MBSSLAYOUT_LINE_TIME = 1;
constexpr int MBSSLAYOUT_LINE_ROUTE = 2;

constexpr int MBSSLAYOUT_LAYOUT_FLATBOTTOM = 0;
constexpr int MBSSLAYOUT_LAYOUT_3DTOPO = 1;

constexpr int MBSSLAYOUT_ALTITUDE_ALTITUDE = 0;
constexpr int MBSSLAYOUT_ALTITUDE_BOTTOMPICK = 1;
constexpr int MBSSLAYOUT_ALTITUDE_TOPO_GRID = 2;

constexpr int MBSSLAYOUT_GAIN_OFF = 0;
constexpr int MBSSLAYOUT_GAIN_TVG = 1;

constexpr int MBSSLAYOUT_SWATHWIDTH_VARIABLE = 0;
constexpr int MBSSLAYOUT_SWATHWIDTH_CONSTANT = 1;

constexpr int MBSSLAYOUT_MERGE_OFF = 0;
constexpr int MBSSLAYOUT_MERGE_FILE = 1;
constexpr int MBSSLAYOUT_MERGE_ASYNC = 2;

constexpr int MBSSLAYOUT_TIME_LATENCY_OFF = 0;
constexpr int MBSSLAYOUT_TIME_LATENCY_FILE = 1;
constexpr int MBSSLAYOUT_TIME_LATENCY_CONSTANT = 2;

constexpr mb_u_char MBSSLAYOUT_TIME_LATENCY_APPLY_NONE = 0x00;
constexpr mb_u_char MBSSLAYOUT_TIME_LATENCY_APPLY_NAV = 0x01;
constexpr mb_u_char MBSSLAYOUT_TIME_LATENCY_APPLY_SENSORDEPTH = 0x02;
constexpr mb_u_char MBSSLAYOUT_TIME_LATENCY_APPLY_ALTITUDE = 0x04;
constexpr mb_u_char MBSSLAYOUT_TIME_LATENCY_APPLY_HEADING = 0x08;
constexpr mb_u_char MBSSLAYOUT_TIME_LATENCY_APPLY_ATTITUDE = 0x10;
constexpr mb_u_char MBSSLAYOUT_TIME_LATENCY_APPLY_SOUNDSPEED = 0x20;
constexpr mb_u_char MBSSLAYOUT_TIME_LATENCY_APPLY_UNUSED = 0x40;
constexpr mb_u_char MBSSLAYOUT_TIME_LATENCY_APPLY_ALL_ANCILLIARY = 0x7F;
constexpr mb_u_char MBSSLAYOUT_TIME_LATENCY_APPLY_SURVEY = 0x80;
constexpr mb_u_char MBSSLAYOUT_TIME_LATENCY_APPLY_ALL = 0xFF;

constexpr int MBSSLAYOUT_ROUTE_WAYPOINT_NONE = 0;
constexpr int MBSSLAYOUT_ROUTE_WAYPOINT_SIMPLE = 1;
constexpr int MBSSLAYOUT_ROUTE_WAYPOINT_TRANSIT = 2;
constexpr int MBSSLAYOUT_ROUTE_WAYPOINT_STARTLINE = 3;
constexpr int MBSSLAYOUT_ROUTE_WAYPOINT_ENDLINE = 4;

// #define MBSSLAYOUT_ONLINE_THRESHOLD 15.0
// #define MBSSLAYOUT_ONLINE_COUNT 30

constexpr int MBSSLAYOUT_SSDIMENSION = 4001;

constexpr int MBSSLAYOUT_NUM_ANGLES = 171;
constexpr double MBSSLAYOUT_ANGLE_MAX = 85.0;

constexpr char program_name[] = "mbsslayout";
constexpr char help_message[] =
    "MBsslayout reads sidescan in raw time series form, lays the sidescan\n"
    "out regularly sampled on a specified topography model, and outputs\n"
    "the sidescan to format 71 (MBF_MBLDEOIH) files.\n";
constexpr char usage_message[] =
    "mbsslayout [--verbose --help --input=datalist --format=format";

/*--------------------------------------------------------------------*/
int mbsslayout_get_flatbottom_table(int verbose, int nangle, double angle_min, double angle_max, double navlon, double navlat,
                                    double altitude, double pitch, double *table_angle, double *table_xtrack,
                                    double *table_ltrack, double *table_altitude, double *table_range, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBSSLAYOUT function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       nangle:          %d\n", nangle);
		fprintf(stderr, "dbg2       angle_min:       %f\n", angle_min);
		fprintf(stderr, "dbg2       angle_max:       %f\n", angle_max);
		fprintf(stderr, "dbg2       navlon:          %f\n", navlon);
		fprintf(stderr, "dbg2       navlat:          %f\n", navlat);
		fprintf(stderr, "dbg2       pitch:           %f\n", pitch);
	}

	/* loop over all of the angles */
	const double dangle = (angle_max - angle_min) / (nangle - 1);
	const double alpha = pitch;
	const double zz = altitude;

	double theta;
	double phi;
	for (int i = 0; i < nangle; i++) {
		/* get angles in takeoff coordinates */
		table_angle[i] = angle_min + dangle * i;
		const double beta = 90.0 - table_angle[i];
		mb_rollpitch_to_takeoff(verbose, alpha, beta, &theta, &phi, error);

		/* calculate range required to achieve desired altitude */
		const double rr = zz / cos(DTR * theta);

		/* get the position */
		const double xx = rr * sin(DTR * theta);
		table_xtrack[i] = xx * cos(DTR * phi);
		table_ltrack[i] = xx * sin(DTR * phi);
		table_altitude[i] = zz;
		table_range[i] = rr;
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBSSLAYOUT function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       Lookup tables:\n");
		for (int i = 0; i < nangle; i++)
			fprintf(stderr, "dbg2         %d %f %f %f %f %f\n", i, table_angle[i], table_xtrack[i], table_ltrack[i],
			        table_altitude[i], table_range[i]);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}

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

	/* command line option definitions */
	static struct option options[] = {{"verbose", no_argument, nullptr, 0},
	                                  {"help", no_argument, nullptr, 0},
	                                  {"verbose", no_argument, nullptr, 0},
	                                  {"input", required_argument, nullptr, 0},
	                                  {"format", required_argument, nullptr, 0},
	                                  {"platform-file", required_argument, nullptr, 0},
	                                  {"platform-target-sensor", required_argument, nullptr, 0},
	                                  {"output-source", required_argument, nullptr, 0},
	                                  {"line-time-list", required_argument, nullptr, 0},
	                                  {"line-position-list", required_argument, nullptr, 0},
	                                  {"line-range-threshold", required_argument, nullptr, 0},
	                                  {"line-name1", required_argument, nullptr, 0},
	                                  {"line-name2", required_argument, nullptr, 0},
	                                  {"output-name1", required_argument, nullptr, 0},
	                                  {"output-name2", required_argument, nullptr, 0},
	                                  {"topo-grid-file", required_argument, nullptr, 0},
	                                  {"altitude-altitude", no_argument, nullptr, 0},
	                                  {"altitude-bottompick", no_argument, nullptr, 0},
	                                  {"altitude-topo-grid", no_argument, nullptr, 0},
	                                  {"altitude-bottompick-threshold", required_argument, nullptr, 0},
	                                  {"channel-swap", required_argument, nullptr, 0},
	                                  {"swath-width", required_argument, nullptr, 0},
	                                  {"gain", required_argument, nullptr, 0},
	                                  {"interpolation", required_argument, nullptr, 0},
	                                  {"nav-file", required_argument, nullptr, 0},
	                                  {"nav-file-format", required_argument, nullptr, 0},
	                                  {"nav-async", required_argument, nullptr, 0},
	                                  {"sensordepth-file", required_argument, nullptr, 0},
	                                  {"sensordepth-file-format", required_argument, nullptr, 0},
	                                  {"sensordepth-async", required_argument, nullptr, 0},
	                                  {"altitude-file", required_argument, nullptr, 0},
	                                  {"altitude-file-format", required_argument, nullptr, 0},
	                                  {"altitude-async", required_argument, nullptr, 0},
	                                  {"heading-file", required_argument, nullptr, 0},
	                                  {"heading-file-format", required_argument, nullptr, 0},
	                                  {"heading-async", required_argument, nullptr, 0},
	                                  {"attitude-file", required_argument, nullptr, 0},
	                                  {"attitude-file-format", required_argument, nullptr, 0},
	                                  {"attitude-async", required_argument, nullptr, 0},
	                                  {"soundspeed-constant", required_argument, nullptr, 0},
	                                  {"soundspeed-file", required_argument, nullptr, 0},
	                                  {"soundspeed-file-format", required_argument, nullptr, 0},
	                                  {"soundspeed-async", required_argument, nullptr, 0},
	                                  {"time-latency-file", required_argument, nullptr, 0},
	                                  {"time-latency-constant", required_argument, nullptr, 0},
	                                  {"time-latency-apply-nav", no_argument, nullptr, 0},
	                                  {"time-latency-apply-sensordepth", no_argument, nullptr, 0},
	                                  {"time-latency-apply-altitude", no_argument, nullptr, 0},
	                                  {"time-latency-apply-heading", no_argument, nullptr, 0},
	                                  {"time-latency-apply-attitude", no_argument, nullptr, 0},
	                                  {"time-latency-apply-all-ancilliary", no_argument, nullptr, 0},
	                                  {"time-latency-apply-survey", no_argument, nullptr, 0},
	                                  {"time-latency-apply-all", no_argument, nullptr, 0},
	                                  {nullptr, 0, nullptr, 0}};

	mb_path read_file = "datalist.mb-1";
	mb_path platform_file = "";
	bool use_platform_file = false;
	int target_sensor = -1;
	int output_source = MB_DATA_DATA;
	mb_path line_name1 = "Survey";
	mb_path line_name2 = "sidescan";
	mb_path line_time_list = "";
	int line_mode = MBSSLAYOUT_LINE_OFF;
	mb_path line_route = "";
	mb_path topo_grid_file = "";
	int layout_mode = MBSSLAYOUT_LAYOUT_FLATBOTTOM;
	int ss_altitude_mode = MBSSLAYOUT_ALTITUDE_ALTITUDE;
	double bottompick_threshold = 0.5;
  double bottompick_blank = 0.0;
	bool channel_swap = false;
	double swath_width = 0.0;
	int swath_mode = MBSSLAYOUT_SWATHWIDTH_VARIABLE;
	int gain_mode = MBSSLAYOUT_GAIN_OFF;
	double gain = 1.0;
	int interpolation = 0;
	mb_path nav_file = "";
	int nav_mode = MBSSLAYOUT_MERGE_OFF;
	int nav_file_format = 0;
	int nav_async = MB_DATA_DATA;
	int nav_sensor = -1;
	mb_path sensordepth_file = "";
	int sensordepth_mode = MBSSLAYOUT_MERGE_OFF;
	int sensordepth_file_format = 0;
	int sensordepth_async = MB_DATA_DATA;
	int sensordepth_sensor = -1;
	mb_path heading_file = "";
	int heading_mode = MBSSLAYOUT_MERGE_OFF;
	int heading_file_format = 0;
	int heading_async = MB_DATA_DATA;
	int heading_sensor = -1;
	mb_path altitude_file = "";
	int altitude_mode = MBSSLAYOUT_MERGE_OFF;
	int altitude_file_format = 0;
	int altitude_async = MB_DATA_DATA;
	int altitude_sensor = -1;
	mb_path attitude_file = "";
	int attitude_mode = MBSSLAYOUT_MERGE_OFF;
	int attitude_file_format = 0;
	int attitude_async = MB_DATA_DATA;
	int attitude_sensor = -1;
	double soundspeed_constant = 1500.0;
	int soundspeed_mode = MBSSLAYOUT_MERGE_OFF;
	mb_path soundspeed_file = "";
	int soundspeed_file_format = 0;
	int soundspeed_async = MB_DATA_DATA;
	mb_path time_latency_file = "";
	int time_latency_mode = MB_SENSOR_TIME_LATENCY_NONE;
	int time_latency_format = 1;
	double time_latency_constant = 0.0;
	mb_u_char time_latency_apply = MBSSLAYOUT_TIME_LATENCY_APPLY_NONE;
	/* time domain filtering */
	mb_u_char filter_apply = MBSSLAYOUT_TIME_LATENCY_APPLY_NONE;
	double filter_length = 0.0;

	{
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
				else if (strcmp("platform-file", options[option_index].name) == 0) {
					const int n = sscanf(optarg, "%1023s", platform_file);
					if (n == 1)
						use_platform_file = true;
				}
				else if (strcmp("platform-target-sensor", options[option_index].name) == 0) {
					/* n = */ sscanf(optarg, "%d", &target_sensor);
				}
				else if (strcmp("output-source", options[option_index].name) == 0 ||
					 strcmp("output_source", options[option_index].name) == 0) {
                if (strcmp(optarg, "SIDESCAN") == 0 || strcmp(optarg, "sidescan") == 0)
                    output_source = MB_DATA_DATA;
                else if (strcmp(optarg, "LOW") == 0 || strcmp(optarg, "low") == 0)
                    output_source = MB_DATA_DATA;
                else if (strcmp(optarg, "HIGH") == 0 || strcmp(optarg, "high") == 0)
                    output_source = MB_DATA_SIDESCAN2;
					else
                    /* n = */ sscanf(optarg, "%d", &output_source);
				}
				else if ((strcmp("line-name1", options[option_index].name) == 0)
                     || (strcmp("line_name1", options[option_index].name) == 0)
				         || (strcmp("output-name1", options[option_index].name) == 0)
                     || (strcmp("output_name1", options[option_index].name) == 0)) {
					strcpy(line_name1, optarg);
				}
				else if ((strcmp("line-name2", options[option_index].name) == 0)
                    || (strcmp("line_name2", options[option_index].name) == 0)
				        || (strcmp("output-name2", options[option_index].name) == 0)
                    || (strcmp("output_name2", options[option_index].name) == 0)) {
					strcpy(line_name2, optarg);
				}
				/*-------------------------------------------------------
				 * Define survey line specification */
				else if ((strcmp("line-time-list", options[option_index].name) == 0)
                     || (strcmp("line_time_list", options[option_index].name) == 0)){
					strcpy(line_time_list, optarg);
					line_mode = MBSSLAYOUT_LINE_TIME;
				}
				else if (strcmp("line-route", options[option_index].name) == 0) {
					strcpy(line_route, optarg);
					line_mode = MBSSLAYOUT_LINE_ROUTE;
				}
				/*-------------------------------------------------------
				 * Define sidescan layout algorithm parameters */
				else if ((strcmp("topo-grid-file", options[option_index].name) == 0)
                    || (strcmp("topo_grid_file", options[option_index].name) == 0)) {
					strcpy(topo_grid_file, optarg);
					layout_mode = MBSSLAYOUT_LAYOUT_3DTOPO;
					ss_altitude_mode = MBSSLAYOUT_ALTITUDE_TOPO_GRID;
				}
				else if (strcmp("altitude-altitude", options[option_index].name) == 0) {
					ss_altitude_mode = MBSSLAYOUT_ALTITUDE_ALTITUDE;
				}
				else if (strcmp("altitude-bottompick", options[option_index].name) == 0) {
					ss_altitude_mode = MBSSLAYOUT_ALTITUDE_BOTTOMPICK;
				}
				else if (strcmp("altitude-bottompick-threshold", options[option_index].name) == 0) {
					/*n = */ sscanf(optarg, "%lf/%lf", &bottompick_threshold, &bottompick_blank);
					ss_altitude_mode = MBSSLAYOUT_ALTITUDE_BOTTOMPICK;
				}
				else if ((strcmp("altitude-topo-grid", options[option_index].name) == 0)
                    || (strcmp("altitude_topo_grid", options[option_index].name) == 0)) {
					ss_altitude_mode = MBSSLAYOUT_ALTITUDE_TOPO_GRID;
				}
				else if (strcmp("channel-swap", options[option_index].name) == 0) {
					channel_swap = true;
				}
				else if (strcmp("swath-width", options[option_index].name) == 0) {
					/* n = */ sscanf(optarg, "%lf", &swath_width);
					swath_mode = MBSSLAYOUT_SWATHWIDTH_CONSTANT;
				}
				else if (strcmp("gain", options[option_index].name) == 0) {
					/* n = */ sscanf(optarg, "%lf", &gain);
					gain_mode = MBSSLAYOUT_GAIN_TVG;
				}
				else if (strcmp("interpolation", options[option_index].name) == 0) {
					/* n = */ sscanf(optarg, "%d", &interpolation);
				}
				/*-------------------------------------------------------
				 * Define source of navigation - could be an external file
				 * or an internal asynchronous record */
				else if (strcmp("nav-file", options[option_index].name) == 0) {
					strcpy(nav_file, optarg);
					nav_mode = MBSSLAYOUT_MERGE_FILE;
				}
				else if (strcmp("nav-file-format", options[option_index].name) == 0) {
					/* n = */ sscanf(optarg, "%d", &nav_file_format);
				}
				else if (strcmp("nav-async", options[option_index].name) == 0) {
					const int n = sscanf(optarg, "%d", &nav_async);
					if (n == 1)
						nav_mode = MBSSLAYOUT_MERGE_ASYNC;
				}
				else if (strcmp("nav-sensor", options[option_index].name) == 0) {
					/* n = */ sscanf(optarg, "%d", &nav_sensor);
				}
				/*-------------------------------------------------------
				 * Define source of sensordepth - could be an external file
				 * or an internal asynchronous record */
				else if (strcmp("sensordepth-file", options[option_index].name) == 0) {
					strcpy(sensordepth_file, optarg);
					sensordepth_mode = MBSSLAYOUT_MERGE_FILE;
				}
				else if (strcmp("sensordepth-file-format", options[option_index].name) == 0) {
					/* n = */ sscanf(optarg, "%d", &sensordepth_file_format);
				}
				else if (strcmp("sensordepth-async", options[option_index].name) == 0) {
					const int n = sscanf(optarg, "%d", &sensordepth_async);
					if (n == 1)
						sensordepth_mode = MBSSLAYOUT_MERGE_ASYNC;
				}
				else if (strcmp("sensordepth-sensor", options[option_index].name) == 0) {
					/* n = */ sscanf(optarg, "%d", &sensordepth_sensor);
				}
				/*-------------------------------------------------------
				 * Define source of heading - could be an external file
				 * or an internal asynchronous record */
				else if (strcmp("heading-file", options[option_index].name) == 0) {
					strcpy(heading_file, optarg);
					heading_mode = MBSSLAYOUT_MERGE_FILE;
				}
				else if (strcmp("heading-file-format", options[option_index].name) == 0) {
					/* n = */ sscanf(optarg, "%d", &heading_file_format);
				}
				else if (strcmp("heading-async", options[option_index].name) == 0) {
					const int n = sscanf(optarg, "%d", &heading_async);
					if (n == 1)
						heading_mode = MBSSLAYOUT_MERGE_ASYNC;
				}
				else if (strcmp("heading-sensor", options[option_index].name) == 0) {
					/* n = */ sscanf(optarg, "%d", &heading_sensor);
				}

				/*-------------------------------------------------------
				 * Define source of altitude - could be an external file
				 * or an internal asynchronous record */

				/* altitude-file */
				else if (strcmp("altitude-file", options[option_index].name) == 0) {
					strcpy(altitude_file, optarg);
					altitude_mode = MBSSLAYOUT_MERGE_FILE;
				}

				/* altitude-file-format */
				else if (strcmp("altitude-file-format", options[option_index].name) == 0) {
					/* n = */ sscanf(optarg, "%d", &altitude_file_format);
				}

				/* altitude-async */
				else if (strcmp("altitude-async", options[option_index].name) == 0) {
					const int n = sscanf(optarg, "%d", &altitude_async);
					if (n == 1)
						altitude_mode = MBSSLAYOUT_MERGE_ASYNC;
				}
				else if (strcmp("altitude-sensor", options[option_index].name) == 0) {
					/* n = */ sscanf(optarg, "%d", &altitude_sensor);
				}
				/*-------------------------------------------------------
				 * Define source of attitude - could be an external file
				 * or an internal asynchronous record */
				else if (strcmp("attitude-file", options[option_index].name) == 0) {
					strcpy(attitude_file, optarg);
					attitude_mode = MBSSLAYOUT_MERGE_FILE;
				}
				else if (strcmp("attitude-file-format", options[option_index].name) == 0) {
					/* n = */ sscanf(optarg, "%d", &attitude_file_format);
				}
				else if (strcmp("attitude-async", options[option_index].name) == 0) {
					const int n = sscanf(optarg, "%d", &attitude_async);
					if (n == 1)
						attitude_mode = MBSSLAYOUT_MERGE_ASYNC;
				}
				else if (strcmp("attitude-sensor", options[option_index].name) == 0) {
					/* n = */ sscanf(optarg, "%d", &attitude_sensor);
				}
				/*-------------------------------------------------------
				 * Define source of sound speed - could be an external file
				 * or an internal asynchronous record */
				else if (strcmp("soundspeed-constant", options[option_index].name) == 0) {
					/* n = */ sscanf(optarg, "%lf", &soundspeed_constant);
					soundspeed_mode = MBSSLAYOUT_MERGE_OFF;
				}
				else if (strcmp("soundspeed-file", options[option_index].name) == 0) {
					strcpy(soundspeed_file, optarg);
					soundspeed_mode = MBSSLAYOUT_MERGE_FILE;
				}
				else if (strcmp("soundspeed-file-format", options[option_index].name) == 0) {
					/* n = */ sscanf(optarg, "%d", &soundspeed_file_format);
				}
				else if (strcmp("soundspeed-async", options[option_index].name) == 0) {
					const int n = sscanf(optarg, "%d", &soundspeed_async);
					if (n == 1)
						soundspeed_mode = MBSSLAYOUT_MERGE_ASYNC;
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
					time_latency_apply = time_latency_apply | MBSSLAYOUT_TIME_LATENCY_APPLY_NAV;
				}
				else if (strcmp("time-latency-apply-sensordepth", options[option_index].name) == 0) {
					time_latency_apply = time_latency_apply | MBSSLAYOUT_TIME_LATENCY_APPLY_SENSORDEPTH;
				}
				else if (strcmp("time-latency-apply-heading", options[option_index].name) == 0) {
					time_latency_apply = time_latency_apply | MBSSLAYOUT_TIME_LATENCY_APPLY_HEADING;
				}
				else if (strcmp("time-latency-apply-attitude", options[option_index].name) == 0) {
					time_latency_apply = time_latency_apply | MBSSLAYOUT_TIME_LATENCY_APPLY_ATTITUDE;
				}
				else if (strcmp("time-latency-apply-altitude", options[option_index].name) == 0) {
					time_latency_apply = time_latency_apply | MBSSLAYOUT_TIME_LATENCY_APPLY_ATTITUDE;
				}
				else if (strcmp("time-latency-apply-all-ancilliary", options[option_index].name) == 0) {
					time_latency_apply = MBSSLAYOUT_TIME_LATENCY_APPLY_ALL_ANCILLIARY;
				}
				else if (strcmp("time-latency-apply-survey", options[option_index].name) == 0) {
					time_latency_apply = MBSSLAYOUT_TIME_LATENCY_APPLY_SURVEY;
				}
				else if (strcmp("time-latency-apply-all", options[option_index].name) == 0) {
					time_latency_apply = MBSSLAYOUT_TIME_LATENCY_APPLY_ALL;
				}
				/*-------------------------------------------------------
				 * Define time domain filtering of ancillary data such as
				 * nav, sensordepth, heading, attitude, and altitude */
				else if (strcmp("filter", options[option_index].name) == 0) {
					/* n = */ sscanf(optarg, "%lf", &filter_length);
				}
				else if (strcmp("filter-apply-nav", options[option_index].name) == 0) {
					filter_apply = filter_apply | MBSSLAYOUT_TIME_LATENCY_APPLY_NAV;
				}
				else if (strcmp("filter-apply-sensordepth", options[option_index].name) == 0) {
					filter_apply = filter_apply | MBSSLAYOUT_TIME_LATENCY_APPLY_SENSORDEPTH;
				}
				else if (strcmp("filter-apply-heading", options[option_index].name) == 0) {
					filter_apply = filter_apply | MBSSLAYOUT_TIME_LATENCY_APPLY_HEADING;
				}
				else if (strcmp("filter-apply-attitude", options[option_index].name) == 0) {
					filter_apply = filter_apply | MBSSLAYOUT_TIME_LATENCY_APPLY_ATTITUDE;
				}
				else if (strcmp("filter-apply-altitude", options[option_index].name) == 0) {
					filter_apply = filter_apply | MBSSLAYOUT_TIME_LATENCY_APPLY_ATTITUDE;
				}
				else if (strcmp("filter-apply-all-ancilliary", options[option_index].name) == 0) {
					filter_apply = MBSSLAYOUT_TIME_LATENCY_APPLY_ALL_ANCILLIARY;
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

		if (verbose >= 2) {
			fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
			fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
			fprintf(stderr, "dbg2  Default MB-System Parameters:\n");
			fprintf(stderr, "dbg2       verbose:                    %d\n", verbose);
			fprintf(stderr, "dbg2       help:                       %d\n", help);
			fprintf(stderr, "dbg2       pings:                      %d\n", pings);
			fprintf(stderr, "dbg2       lonflip:                    %d\n", lonflip);
			fprintf(stderr, "dbg2       bounds[0]:                  %f\n", bounds[0]);
			fprintf(stderr, "dbg2       bounds[1]:                  %f\n", bounds[1]);
			fprintf(stderr, "dbg2       bounds[2]:                  %f\n", bounds[2]);
			fprintf(stderr, "dbg2       bounds[3]:                  %f\n", bounds[3]);
			fprintf(stderr, "dbg2       btime_i[0]:                 %d\n", btime_i[0]);
			fprintf(stderr, "dbg2       btime_i[1]:                 %d\n", btime_i[1]);
			fprintf(stderr, "dbg2       btime_i[2]:                 %d\n", btime_i[2]);
			fprintf(stderr, "dbg2       btime_i[3]:                 %d\n", btime_i[3]);
			fprintf(stderr, "dbg2       btime_i[4]:                 %d\n", btime_i[4]);
			fprintf(stderr, "dbg2       btime_i[5]:                 %d\n", btime_i[5]);
			fprintf(stderr, "dbg2       btime_i[6]:                 %d\n", btime_i[6]);
			fprintf(stderr, "dbg2       etime_i[0]:                 %d\n", etime_i[0]);
			fprintf(stderr, "dbg2       etime_i[1]:                 %d\n", etime_i[1]);
			fprintf(stderr, "dbg2       etime_i[2]:                 %d\n", etime_i[2]);
			fprintf(stderr, "dbg2       etime_i[3]:                 %d\n", etime_i[3]);
			fprintf(stderr, "dbg2       etime_i[4]:                 %d\n", etime_i[4]);
			fprintf(stderr, "dbg2       etime_i[5]:                 %d\n", etime_i[5]);
			fprintf(stderr, "dbg2       etime_i[6]:                 %d\n", etime_i[6]);
			fprintf(stderr, "dbg2       speedmin:                   %f\n", speedmin);
			fprintf(stderr, "dbg2       timegap:                    %f\n", timegap);
			fprintf(stderr, "dbg2  Data Input Parameters:\n");
			fprintf(stderr, "dbg2       read_file:                  %s\n", read_file);
			fprintf(stderr, "dbg2       format:                     %d\n", format);
			fprintf(stderr, "dbg2  Platform Definition:\n");
			fprintf(stderr, "dbg2       use_platform_file:          %d\n", use_platform_file);
			fprintf(stderr, "dbg2       platform_file:              %s\n", platform_file);
			fprintf(stderr, "dbg2       target_sensor:              %d\n", target_sensor);
			fprintf(stderr, "dbg2  Source Data Parameters:\n");
			fprintf(stderr, "dbg2       output_source:              %d\n", output_source);
			fprintf(stderr, "dbg2       line_name1:                 %s\n", line_name1);
			fprintf(stderr, "dbg2       line_name2:                 %s\n", line_name2);
			fprintf(stderr, "dbg2  Survey Line Parameters:\n");
			fprintf(stderr, "dbg2       line_mode:                  %d\n", line_mode);
			fprintf(stderr, "dbg2       line_time_list:             %s\n", line_time_list);
			fprintf(stderr, "dbg2       line_route:                 %s\n", line_route);
			// fprintf(stderr, "dbg2       line_range_threshold:       %f\n", line_range_threshold);
			fprintf(stderr, "dbg2  Sidescan Layout Algorithm Parameters:\n");
			fprintf(stderr, "dbg2       layout_mode:                %d\n", layout_mode);
			fprintf(stderr, "dbg2       topo_grid_file:             %s\n", topo_grid_file);
			fprintf(stderr, "dbg2       ss_altitude_mode:           %d\n", ss_altitude_mode);
			fprintf(stderr, "dbg2       bottompick_threshold:       %f\n", bottompick_threshold);
			fprintf(stderr, "dbg2       bottompick_blank:           %f\n", bottompick_blank);
			fprintf(stderr, "dbg2       channel_swap:               %d\n", channel_swap);
			fprintf(stderr, "dbg2       swath_mode:                 %d\n", swath_mode);
			fprintf(stderr, "dbg2       swath_width:                %f\n", swath_width);
			fprintf(stderr, "dbg2       gain_mode:                  %d\n", gain_mode);
			fprintf(stderr, "dbg2       gain:                       %f\n", gain);
			fprintf(stderr, "dbg2       interpolation:              %d\n", interpolation);
			fprintf(stderr, "dbg2  Navigation Source Parameters:\n");
			fprintf(stderr, "dbg2       nav_mode:                   %d\n", nav_mode);
			fprintf(stderr, "dbg2       nav_file:                   %s\n", nav_file);
			fprintf(stderr, "dbg2       nav_file_format:            %d\n", nav_file_format);
			fprintf(stderr, "dbg2       nav_async:                  %d\n", nav_async);
			fprintf(stderr, "dbg2  Sensor Depth Source Parameters:\n");
			fprintf(stderr, "dbg2       sensordepth_mode:           %d\n", sensordepth_mode);
			fprintf(stderr, "dbg2       sensordepth_file:           %s\n", sensordepth_file);
			fprintf(stderr, "dbg2       sensordepth_file_format:    %d\n", sensordepth_file_format);
			fprintf(stderr, "dbg2       sensordepth_async:          %d\n", sensordepth_async);
			fprintf(stderr, "dbg2  Altitude Source Parameters:\n");
			fprintf(stderr, "dbg2       altitude_mode:              %d\n", altitude_mode);
			fprintf(stderr, "dbg2       altitude_file:              %s\n", altitude_file);
			fprintf(stderr, "dbg2       altitude_file_format:       %d\n", altitude_file_format);
			fprintf(stderr, "dbg2       altitude_async:             %d\n", altitude_async);
			fprintf(stderr, "dbg2  Heading Source Parameters:\n");
			fprintf(stderr, "dbg2       heading_mode:               %d\n", heading_mode);
			fprintf(stderr, "dbg2       heading_file:               %s\n", heading_file);
			fprintf(stderr, "dbg2       heading_file_format:        %d\n", heading_file_format);
			fprintf(stderr, "dbg2       heading_async:              %d\n", heading_async);
			fprintf(stderr, "dbg2  Attitude Source Parameters:\n");
			fprintf(stderr, "dbg2       attitude_mode:              %d\n", attitude_mode);
			fprintf(stderr, "dbg2       attitude_file:              %s\n", attitude_file);
			fprintf(stderr, "dbg2       attitude_file_format:       %d\n", attitude_file_format);
			fprintf(stderr, "dbg2       attitude_async:             %d\n", attitude_async);
			fprintf(stderr, "dbg2  Sound Speed Source Parameters:\n");
			fprintf(stderr, "dbg2       soundspeed_mode:            %d\n", soundspeed_mode);
			fprintf(stderr, "dbg2       soundspeed_constant:        %f\n", soundspeed_constant);
			fprintf(stderr, "dbg2       soundspeed_file:            %s\n", soundspeed_file);
			fprintf(stderr, "dbg2       soundspeed_file_format:     %d\n", soundspeed_file_format);
			fprintf(stderr, "dbg2       soundspeed_async:           %d\n", soundspeed_async);
			fprintf(stderr, "dbg2  Time Latency Source Parameters:\n");
			fprintf(stderr, "dbg2       time_latency_mode:             %d\n", time_latency_mode);
			fprintf(stderr, "dbg2       time_latency_file:             %s\n", time_latency_file);
			fprintf(stderr, "dbg2       time_latency_format:           %d\n", time_latency_format);
			fprintf(stderr, "dbg2       time_latency_constant:         %f\n", time_latency_constant);
			fprintf(stderr, "dbg2       time_latency_apply:            %x\n", time_latency_apply);
		}

		if (help) {
			fprintf(stderr, "\n%s\n", help_message);
			fprintf(stderr, "\nusage: %s\n", usage_message);
			exit(MB_ERROR_NO_ERROR);
		}
	}

	if (verbose == 1) {
		fprintf(stderr, "\nProgram <%s>\n", program_name);
		fprintf(stderr, "MB-system Version %s\n\n", MB_VERSION);
		fprintf(stderr, "Data Input Parameters:\n");
		fprintf(stderr, "     read_file:                  %s\n", read_file);
		fprintf(stderr, "     format:                     %d\n", format);
		fprintf(stderr, "Source of platform model:\n");
		if (use_platform_file)
			fprintf(stderr, "     platform_file:              %s\n", platform_file);
		else
			fprintf(stderr, "     platform_file:              not specified\n");
		fprintf(stderr, "     target_sensor:              %d\n", target_sensor);
		fprintf(stderr, "Output Channel Parameters:\n");
		if (output_source != MB_DATA_NONE) {
			fprintf(stderr, "     output_source:            %d\n", output_source);
			fprintf(stderr, "     line_name1:               %s\n", line_name1);
			fprintf(stderr, "     line_name2:               %s\n", line_name2);
		}
		fprintf(stderr, "Survey Line Parameters:\n");
		if (line_mode == MBSSLAYOUT_LINE_OFF) {
			fprintf(stderr, "     line_mode:                Data not recast into survey lines.\n");
		}
		else if (line_mode == MBSSLAYOUT_LINE_TIME) {
			fprintf(stderr, "     line_mode:                Lines defined by waypoint time list.\n");
			fprintf(stderr, "     line_time_list:           %s\n", line_time_list);
		}
		else if (line_mode == MBSSLAYOUT_LINE_ROUTE) {
			fprintf(stderr, "     line_mode:                Lines defined by route waypoint position list.\n");
			fprintf(stderr, "     line_route:               %s\n", line_route);
		}
		fprintf(stderr, "Sidescan Layout Algorithm Parameters:\n");
		if (layout_mode == MBSSLAYOUT_LAYOUT_FLATBOTTOM) {
			fprintf(stderr, "     layout_mode:              Flat bottom layout using altitude\n");
		}
		else if (layout_mode == MBSSLAYOUT_LAYOUT_3DTOPO) {
			fprintf(stderr, "     layout_mode:              3D layout using topography model\n");
			fprintf(stderr, "     topo_grid_file:           %s\n", topo_grid_file);
		}
		if (ss_altitude_mode == MBSSLAYOUT_ALTITUDE_ALTITUDE) {
			fprintf(stderr, "     ss_altitude_mode:         Existing altitude value used\n");
		}
		else if (ss_altitude_mode == MBSSLAYOUT_ALTITUDE_BOTTOMPICK) {
			fprintf(stderr, "     ss_altitude_mode:         Altitude calculated using bottom pick in time series\n");
			fprintf(stderr, "     bottompick_threshold:     %f\n", bottompick_threshold);
			fprintf(stderr, "     bottompick_blank:         %f\n", bottompick_blank);
		}
		else if (layout_mode == MBSSLAYOUT_ALTITUDE_TOPO_GRID) {
			fprintf(stderr, "     ss_altitude_mode:         Altitude calculated during 3D layout on topography model\n");
		}
		if (channel_swap)
			fprintf(stderr, "     channel_swap:             Swapping port and starboard\n");
		else
			fprintf(stderr, "     channel_swap:             No swap\n");
		if (swath_mode == MBSSLAYOUT_SWATHWIDTH_CONSTANT) {
			fprintf(stderr, "     swath_mode:               Constant swath width\n");
			fprintf(stderr, "     swath_width:              %f\n", swath_width);
		}
		else {
			fprintf(stderr, "     swath_mode:               Variable swath width\n");
		}
		if (gain_mode == MBSSLAYOUT_SWATHWIDTH_CONSTANT) {
			fprintf(stderr, "     gain_mode:                Gain applied\n");
			fprintf(stderr, "     gain:                     %f\n", gain);
		}
		else {
			fprintf(stderr, "     gain_mode:                Gain not applied\n");
		}
		fprintf(stderr, "     interpolation:            %d\n", interpolation);
		fprintf(stderr, "Navigation Source Parameters:\n");
		if (nav_mode == MBSSLAYOUT_MERGE_OFF) {
			fprintf(stderr, "     nav_mode:                   No navigation merging\n");
		}
		else if (nav_mode == MBSSLAYOUT_MERGE_FILE) {
			fprintf(stderr, "     nav_mode:                   Navigation merged from external file\n");
			fprintf(stderr, "     nav_file:                   %s\n", nav_file);
			fprintf(stderr, "     nav_file_format:            %d\n", nav_file_format);
		}
		else if (nav_mode == MBSSLAYOUT_MERGE_ASYNC) {
			fprintf(stderr, "     nav_mode:                   Navigation merged from asynchronous data records\n");
			fprintf(stderr, "     nav_async:                  %d\n", nav_async);
		}

		fprintf(stderr, "Sensor Depth Source Parameters:\n");
		if (sensordepth_mode == MBSSLAYOUT_MERGE_OFF) {
			fprintf(stderr, "     sensordepth_mode:           No sensor depth merging\n");
		}
		else if (sensordepth_mode == MBSSLAYOUT_MERGE_FILE) {
			fprintf(stderr, "     sensordepth_mode:           Sensor depth merged from external file\n");
			fprintf(stderr, "     sensordepth_file:           %s\n", sensordepth_file);
			fprintf(stderr, "     sensordepth_file_format:    %d\n", sensordepth_file_format);
		}
		else if (sensordepth_mode == MBSSLAYOUT_MERGE_ASYNC) {
			fprintf(stderr, "     sensordepth_mode:           Sensor depth merged from asynchronous data records\n");
			fprintf(stderr, "     sensordepth_async:          %d\n", sensordepth_async);
		}

		fprintf(stderr, "Altitude Source Parameters:\n");
		if (altitude_mode == MBSSLAYOUT_MERGE_OFF) {
			fprintf(stderr, "     altitude_mode:              No altitude merging\n");
		}
		else if (altitude_mode == MBSSLAYOUT_MERGE_FILE) {
			fprintf(stderr, "     altitude_mode:              Altitude merged from external file\n");
			fprintf(stderr, "     altitude_file:              %s\n", altitude_file);
			fprintf(stderr, "     altitude_file_format:       %d\n", altitude_file_format);
		}
		else if (altitude_mode == MBSSLAYOUT_MERGE_ASYNC) {
			fprintf(stderr, "     altitude_mode:              Altitude merged from asynchronous data records\n");
			fprintf(stderr, "     altitude_async:             %d\n", altitude_async);
		}

		fprintf(stderr, "Heading Source Parameters:\n");
		if (heading_mode == MBSSLAYOUT_MERGE_OFF) {
			fprintf(stderr, "     heading_mode:               No heading merging\n");
		}
		else if (heading_mode == MBSSLAYOUT_MERGE_FILE) {
			fprintf(stderr, "     heading_mode:               Heading merged from external file\n");
			fprintf(stderr, "     heading_file:               %s\n", heading_file);
			fprintf(stderr, "     heading_file_format:        %d\n", heading_file_format);
		}
		else if (heading_mode == MBSSLAYOUT_MERGE_ASYNC) {
			fprintf(stderr, "     heading_mode:               Heading merged from asynchronous data records\n");
			fprintf(stderr, "     heading_async:              %d\n", heading_async);
		}

		fprintf(stderr, "Attitude Source Parameters:\n");
		if (attitude_mode == MBSSLAYOUT_MERGE_OFF) {
			fprintf(stderr, "     attitude_mode:              No attitude merging\n");
		}
		else if (attitude_mode == MBSSLAYOUT_MERGE_FILE) {
			fprintf(stderr, "     attitude_mode:              Attitude merged from external file\n");
			fprintf(stderr, "     attitude_file:              %s\n", attitude_file);
			fprintf(stderr, "     attitude_file_format:       %d\n", attitude_file_format);
		}
		else if (attitude_mode == MBSSLAYOUT_MERGE_ASYNC) {
			fprintf(stderr, "     attitude_mode:              Attitude merged from asynchronous data records\n");
			fprintf(stderr, "     attitude_async:             %d\n", attitude_async);
		}
		fprintf(stderr, "Sound Speed Source Parameters:\n");
		if (soundspeed_mode == MBSSLAYOUT_MERGE_OFF) {
			fprintf(stderr, "     soundspeed_mode:            No sound speed merging, constant value\n");
			fprintf(stderr, "     soundspeed_constant:        %f meters/second\n", soundspeed_constant);
		}
		else if (soundspeed_mode == MBSSLAYOUT_MERGE_FILE) {
			fprintf(stderr, "     soundspeed_mode:            Sound speed merged from external file\n");
			fprintf(stderr, "     soundspeed_file:            %s\n", soundspeed_file);
			fprintf(stderr, "     soundspeed_file_format:     %d\n", soundspeed_file_format);
		}
		else if (soundspeed_mode == MBSSLAYOUT_MERGE_ASYNC) {
			fprintf(stderr, "     soundspeed_mode:            Sound speed merged from asynchronous data records\n");
			fprintf(stderr, "     soundspeed_async:           %d\n", soundspeed_async);
		}
		fprintf(stderr, "Time Shift Source Parameters:\n");
		if (time_latency_mode == MBSSLAYOUT_TIME_LATENCY_OFF) {
			fprintf(stderr, "     time_latency_mode:             No time shift\n");
		}
		else if (time_latency_mode == MBSSLAYOUT_TIME_LATENCY_FILE) {
			fprintf(stderr, "     time_latency_mode:             Time shift model read from external file\n");
			fprintf(stderr, "     time_latency_file:             %s\n", time_latency_file);
			fprintf(stderr, "     time_latency_format:           %d\n", time_latency_format);
		}
		else if (time_latency_mode == MBSSLAYOUT_TIME_LATENCY_CONSTANT) {
			fprintf(stderr, "     time_latency_mode:             Constant time shift\n");
			fprintf(stderr, "     time_latency_constant:         %f\n", time_latency_constant);
		}
		fprintf(stderr, "\n");
	}

	int error = MB_ERROR_NO_ERROR;

	/*-------------------------------------------------------------------*/
	/* load platform definition if specified */

	/* platform definition file */
	struct mb_platform_struct *platform = nullptr;
	struct mb_sensor_struct *sensor_position = nullptr;
	struct mb_sensor_struct *sensor_depth = nullptr;
	struct mb_sensor_struct *sensor_heading = nullptr;
	struct mb_sensor_struct *sensor_rollpitch = nullptr;

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
		// struct mb_sensor_struct *sensor_bathymetry = nullptr;
		// if (platform->source_bathymetry >= 0)
		//	sensor_bathymetry = &(platform->sensors[platform->source_bathymetry]);
		// struct mb_sensor_struct *sensor_backscatter = nullptr;
		// if (platform->source_backscatter >= 0)
		//	sensor_backscatter = &(platform->sensors[platform->source_backscatter]);
		if (platform->source_position >= 0)
			sensor_position = &(platform->sensors[platform->source_position]);
		if (platform->source_depth >= 0)
			sensor_depth = &(platform->sensors[platform->source_depth]);
		if (platform->source_heading >= 0)
			sensor_heading = &(platform->sensors[platform->source_heading]);
		if (platform->source_rollpitch >= 0)
			sensor_rollpitch = &(platform->sensors[platform->source_rollpitch]);
		// struct mb_sensor_struct *sensor_heave = nullptr;
		// if (platform->source_heave >= 0)
		//	sensor_heave = &(platform->sensors[platform->source_heave]);
		if (target_sensor < 0)
			target_sensor = platform->source_bathymetry;
		// struct mb_sensor_struct *sensor_target = nullptr;
		// if (target_sensor >= 0)
		//	sensor_target = &(platform->sensors[target_sensor]);
	}

	void *topogrid_ptr = nullptr;

	/* read topography grid if 3D bottom correction specified */
	if (layout_mode == MBSSLAYOUT_LAYOUT_3DTOPO) {
		status = mb_topogrid_init(verbose, topo_grid_file, &lonflip, &topogrid_ptr, &error);
		if (error != MB_ERROR_NO_ERROR) {
			char *message;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error loading topography grid: %s\n%s\n", topo_grid_file, message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
		}
	}

	/*-------------------------------------------------------------------*/
	/* load ancillary data from external files if requested */

	int time_latency_num = 0;
	int time_latency_alloc = 0;
	double *time_latency_time_d = nullptr;
	double *time_latency_time_latency = nullptr;

	/* start by loading time latency model if required */
	if (time_latency_mode == MB_SENSOR_TIME_LATENCY_MODEL) {
		mb_loadtimeshiftdata(verbose, time_latency_file, time_latency_format, &time_latency_num, &time_latency_alloc,
		                     &time_latency_time_d, &time_latency_time_latency, &error);

		if (verbose > 0)
			fprintf(stderr, "%d time_latency records loaded from file %s\n", time_latency_num, time_latency_file);
	}

	/* import specified ancillary data */

	/* asynchronous navigation, heading, attitude data */
	int n_nav = 0;
	int n_nav_alloc = 0;
	double *nav_time_d = nullptr;
	double *nav_navlon = nullptr;
	double *nav_navlat = nullptr;
	double *nav_speed = nullptr;

	if (nav_mode == MBSSLAYOUT_MERGE_FILE) {
		mb_loadnavdata(verbose, nav_file, nav_file_format, lonflip, &n_nav, &n_nav_alloc, &nav_time_d, &nav_navlon, &nav_navlat,
		               &nav_speed, &error);

		if (verbose > 0)
			fprintf(stderr, "%d navigation records loaded from file %s\n", n_nav, nav_file);
	}

	int n_sensordepth = 0;
	int n_sensordepth_alloc = 0;
	double *sensordepth_time_d = nullptr;
	double *sensordepth_sensordepth = nullptr;

	if (sensordepth_mode == MBSSLAYOUT_MERGE_FILE) {
		mb_loadsensordepthdata(verbose, sensordepth_file, sensordepth_file_format, &n_sensordepth, &n_sensordepth_alloc,
		                       &sensordepth_time_d, &sensordepth_sensordepth, &error);

		if (verbose > 0)
			fprintf(stderr, "%d sensordepth records loaded from file %s\n", n_sensordepth, sensordepth_file);
	}

	int n_heading = 0;
	int n_heading_alloc = 0;
	double *heading_time_d = nullptr;
	double *heading_heading = nullptr;

	if (heading_mode == MBSSLAYOUT_MERGE_FILE) {
		mb_loadheadingdata(verbose, heading_file, heading_file_format, &n_heading, &n_heading_alloc, &heading_time_d,
		                   &heading_heading, &error);

		if (verbose > 0)
			fprintf(stderr, "%d heading records loaded from file %s\n", n_heading, heading_file);
	}

	int n_altitude = 0;
	int n_altitude_alloc = 0;
	double *altitude_time_d = nullptr;
	double *altitude_altitude = nullptr;

	if (altitude_mode == MBSSLAYOUT_MERGE_FILE) {
		mb_loadaltitudedata(verbose, altitude_file, altitude_file_format, &n_altitude, &n_altitude_alloc, &altitude_time_d,
		                    &altitude_altitude, &error);

		if (verbose > 0)
			fprintf(stderr, "%d altitude records loaded from file %s\n", n_altitude, altitude_file);
	}

	int n_attitude = 0;
	int n_attitude_alloc = 0;
	double *attitude_time_d = nullptr;
	double *attitude_roll = nullptr;
	double *attitude_pitch = nullptr;
	double *attitude_heave = nullptr;

	if (attitude_mode == MBSSLAYOUT_MERGE_FILE) {
		mb_loadattitudedata(verbose, attitude_file, attitude_file_format, &n_attitude, &n_attitude_alloc, &attitude_time_d,
		                    &attitude_roll, &attitude_pitch, &attitude_heave, &error);

		if (verbose > 0)
			fprintf(stderr, "%d attitude records loaded from file %s\n", n_attitude, attitude_file);
	}

	int n_soundspeed = 0;
	int n_soundspeed_alloc = 0;
	double *soundspeed_time_d = nullptr;
	double *soundspeed_soundspeed = nullptr;

	if (soundspeed_mode == MBSSLAYOUT_MERGE_FILE) {
		mb_loadsoundspeeddata(verbose, soundspeed_file, soundspeed_file_format, &n_soundspeed, &n_soundspeed_alloc,
		                      &soundspeed_time_d, &soundspeed_soundspeed, &error);

		if (verbose > 0)
			fprintf(stderr, "%d soundspeed records loaded from file %s\n", n_soundspeed, soundspeed_file);
	}

	/*-------------------------------------------------------------------*/

	/* new output file obviously needed */
	bool new_output_file = true;
	// bool rawroutefile = false;
	// bool oktowrite = false;
	// bool linechange = false;
	double navlon;
	double navlat;
	double heading;
	double time_d;
	int ntimepoint = 0;
	double *routelon = nullptr;
	double *routelat = nullptr;
	double *routeheading = nullptr;
	int *routewaypoint = nullptr;
	double *routetime_d = nullptr;
	int activewaypoint = -1;
	double mtodeglon;
	double mtodeglat;
	double rangelast;
	/* survey line variables */
	double line_range_threshold = 50.0;
	double topo;
	int nroutepoint = 0;
	int nroutepointalloc = 0;

	char comment[MB_COMMENT_MAXLINE];

	/* if specified read route time list file */
	if (line_mode == MBSSLAYOUT_LINE_TIME) {
		/* open the input file */
		FILE *fp = fopen(line_time_list, "r");
		if (fp == nullptr) {
			error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
			fprintf(stderr, "\nUnable to open time list file <%s> for reading\n", line_time_list);
			exit(status);
		}
		// bool rawroutefile = false;
		int ntimepointalloc = 0;
		char *result = nullptr;
		while ((result = fgets(comment, MB_PATH_MAXLINE, fp)) == comment) {
			if (comment[0] != '#') {
				int i;
				int waypoint;
				/* int nget = */ sscanf(comment, "%d %d %lf %lf %lf %lf", &i, &waypoint, &navlon, &navlat, &heading, &time_d);

				/* if good data check for need to allocate more space */
				if (ntimepoint + 1 > ntimepointalloc) {
					ntimepointalloc += MBSSLAYOUT_ALLOC_NUM;
					status =
					    mb_reallocd(verbose, __FILE__, __LINE__, ntimepointalloc * sizeof(double), (void **)&routelon, &error);
					status &=
					    mb_reallocd(verbose, __FILE__, __LINE__, ntimepointalloc * sizeof(double), (void **)&routelat, &error);
					status &= mb_reallocd(verbose, __FILE__, __LINE__, ntimepointalloc * sizeof(double), (void **)&routeheading,
					                     &error);
					status &=
					    mb_reallocd(verbose, __FILE__, __LINE__, ntimepointalloc * sizeof(int), (void **)&routewaypoint, &error);
					status &=
					    mb_reallocd(verbose, __FILE__, __LINE__, ntimepointalloc * sizeof(double), (void **)&routetime_d, &error);
					if (status != MB_SUCCESS) {
						char *message;
						mb_error(verbose, error, &message);
						fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
						fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
						exit(error);
					}
				}

				/* add good point to route */
				if (ntimepointalloc > ntimepoint) {
					routewaypoint[ntimepoint] = waypoint;
					routelon[ntimepoint] = navlon;
					routelat[ntimepoint] = navlat;
					routeheading[ntimepoint] = heading;
					routetime_d[ntimepoint] = time_d;
					ntimepoint++;
				}
			}
		}

		fclose(fp);
		fp = nullptr;

		activewaypoint = 1;
		mb_coor_scale(verbose, routelat[activewaypoint], &mtodeglon, &mtodeglat);
		rangelast = 1000 * line_range_threshold;
		// oktowrite = false;
		// linechange = false;

		/* output status */
		if (verbose > 0) {
			/* output info on file output */
			fprintf(stderr, "Read %d waypoints from time list file: %s\n", ntimepoint, line_time_list);
		}
	}

	/* if specified read route file */
	else if (line_mode == MBSSLAYOUT_LINE_ROUTE) {
		/* open the input file */
		FILE *fp = fopen(line_route, "r");
		if (fp == nullptr) {
			error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
			fprintf(stderr, "\nUnable to open route file <%s> for reading\n", line_route);
			exit(status);
		}
		bool rawroutefile = false;
		char *result = nullptr;
		while ((result = fgets(comment, MB_PATH_MAXLINE, fp)) == comment) {
			if (comment[0] == '#') {
				if (strncmp(comment, "## Route File Version", 21) == 0) {
					rawroutefile = false;
				}
			} else {
				int waypoint;
				const int nget = sscanf(comment, "%lf %lf %lf %d %lf", &navlon, &navlat, &topo, &waypoint, &heading);
				if (comment[0] == '#') {
					fprintf(stderr, "buffer:%s", comment);
					if (strncmp(comment, "## Route File Version", 21) == 0) {
						rawroutefile = false;
					}
				}
				bool point_ok =
					(rawroutefile && nget >= 2) ||
					(!rawroutefile && nget >= 3 && waypoint > MBSSLAYOUT_ROUTE_WAYPOINT_NONE);

				/* if good data check for need to allocate more space */
				if (point_ok && nroutepoint + 1 > nroutepointalloc) {
					nroutepointalloc += MBSSLAYOUT_ALLOC_NUM;
					status =
					    mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double), (void **)&routelon, &error);
					status &=
					    mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double), (void **)&routelat, &error);
					status &= mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double), (void **)&routeheading,
					                     &error);
					status &=
					    mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(int), (void **)&routewaypoint, &error);
					if (status != MB_SUCCESS) {
						char *message;
						mb_error(verbose, error, &message);
						fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
						fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
						exit(error);
					}
				}

				/* add good point to route */
				if (point_ok && nroutepointalloc > nroutepoint + 1) {
					routelon[nroutepoint] = navlon;
					routelat[nroutepoint] = navlat;
					routeheading[nroutepoint] = heading;
					routewaypoint[nroutepoint] = waypoint;
					nroutepoint++;
				}
			}
		}

		fclose(fp);
		fp = nullptr;

		/* set starting values */
		activewaypoint = 1;
		mb_coor_scale(verbose, routelat[activewaypoint], &mtodeglon, &mtodeglat);
		rangelast = 1000 * line_range_threshold;
		// oktowrite = false;
		// linechange = false;

		/* output status */
		if (verbose > 0) {
			/* output info on file output */
			fprintf(stderr, "\nImported %d waypoints from route file: %s\n", nroutepoint, line_route);
		}
	}

	/* set up plotting script file */
	char scriptfile[2*MB_PATH_MAXLINE+20] = "";
	snprintf(scriptfile, sizeof(scriptfile), "%s_%s_ssswathplot.cmd", line_name1, line_name2);
	FILE *sfp = fopen(scriptfile, "w");
	if (sfp == nullptr) {
		error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
		fprintf(stderr, "\nUnable to open plotting script file <%s> \n", scriptfile);
		exit(status);
	} else {
    char user[256], host[256], date[32];
    status = mb_user_host_date(verbose, user, host, date, &error);
		fprintf(sfp, "# Swath plot generation script\n");
		fprintf(sfp, "#   Written by MB-System program %s\n", program_name);
		fprintf(sfp, "#   MB-system Version %s\n", MB_VERSION);
		fprintf(sfp, "#   Run run by %s on %s at %s\n#\n", user, host, date);
	}

	/*-------------------------------------------------------------------*/

	/* Do first pass through the data collecting ancillary data from the desired source records */

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose, read_file, nullptr, &format, &error);

	/* determine whether to read one file or a list of files */
	const bool read_datalist = format < 0;
	bool read_data;
	void *datalist;
	mb_path ifile = "";
	mb_path dfile = "";
	int iformat;
	double file_weight;

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
		/* else copy single filename to be read */
		strcpy(ifile, read_file);
		iformat = format;
		read_data = true;
	}

	void *imbio_ptr = nullptr;
	double btime_d;
	double etime_d;
	int beams_bath;
	int beams_amp;
	int pixels_ss;
	char *beamflag = nullptr;
	double *bath = nullptr;
	double *bathacrosstrack = nullptr;
	double *bathalongtrack = nullptr;
	double *amp = nullptr;
	double *ss = nullptr;
	double *ssacrosstrack = nullptr;
	double *ssalongtrack = nullptr;

	/* counts of records read and written */
	int n_rf_data = 0;
	int n_rf_comment = 0;
	int n_rf_ss2 = 0;
	int n_rf_ss3 = 0;
	int n_rf_sbp = 0;
	int n_rf_nav = 0;
	int n_rf_nav1 = 0;
	int n_rf_nav2 = 0;
	int n_rf_nav3 = 0;

	void *istore_ptr = nullptr;

	int kind;
	int time_i[7];
	double speed;
	double distance;
	double altitude;
	double sensordepth;

	int n_rt_data = 0;
	int n_rt_comment = 0;
	int n_rt_ss2 = 0;
	int n_rt_ss3 = 0;
	int n_rt_sbp = 0;
	int n_rt_nav = 0;
	int n_rt_nav1 = 0;
	int n_rt_nav2 = 0;
	int n_rt_nav3 = 0;

	/* arrays for asynchronous data accessed using mb_extract_nnav() */
	int nanavmax = MB_NAV_MAX;
	int nanav;
	int antime_i[7 * MB_NAV_MAX];
	double antime_d[MB_NAV_MAX];
	double anlon[MB_NAV_MAX];
	double anlat[MB_NAV_MAX];
	double anspeed[MB_NAV_MAX];
	double anheading[MB_NAV_MAX];
	double ansensordraft[MB_NAV_MAX];
	double anroll[MB_NAV_MAX];
	double anpitch[MB_NAV_MAX];
	double anheave[MB_NAV_MAX];

	double sensordraft;
	double roll;
	double pitch;
	double heave;

	int nactd;
	double actime_d[MB_CTD_MAX];
	double acconductivity[MB_CTD_MAX];
	double actemperature[MB_CTD_MAX];
	double acdepth[MB_CTD_MAX];
	double acsalinity[MB_CTD_MAX];
	double acsoundspeed[MB_CTD_MAX];

	/* loop over all files to be read */
	while (read_data) {
		if (verbose > 0)
			fprintf(stderr, "\nPass 1: Opening file %s %d\n", ifile, iformat);

		/* initialize reading the swath file */
		if (mb_read_init(verbose, ifile, iformat, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap,
		                           &imbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
			char *message;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
			fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", ifile);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}

		beamflag = nullptr;
		bath = nullptr;
		amp = nullptr;
		bathacrosstrack = nullptr;
		bathalongtrack = nullptr;
		ss = nullptr;
		ssacrosstrack = nullptr;
		ssalongtrack = nullptr;
		if (error == MB_ERROR_NO_ERROR)
			/* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status = */
			    mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status = */
			    mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status = */ mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, &error);

		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR) {
			char *message;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}

		/* zero file count records */
		n_rf_data = 0;
		n_rf_comment = 0;
		n_rf_ss2 = 0;
		n_rf_ss3 = 0;
		n_rf_sbp = 0;
		n_rf_nav = 0;
		n_rf_nav1 = 0;
		n_rf_nav2 = 0;
		n_rf_nav3 = 0;

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
			else if (kind == MB_DATA_SIDESCAN2) {
				n_rf_ss2++;
				n_rt_ss2++;
			}
			else if (kind == MB_DATA_SIDESCAN3) {
				n_rf_ss3++;
				n_rt_ss3++;
			}
			else if (kind == MB_DATA_SUBBOTTOM_SUBBOTTOM) {
				n_rf_sbp++;
				n_rt_sbp++;
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

			/* look for nav if not externally defined */
			if (status == MB_SUCCESS && nav_mode == MBSSLAYOUT_MERGE_ASYNC && kind == sensordepth_async) {
				/* extract nav data */
				status = mb_extract_nnav(verbose, imbio_ptr, istore_ptr, nanavmax, &kind, &nanav, antime_i, antime_d, anlon,
				                         anlat, anspeed, anheading, ansensordraft, anroll, anpitch, anheave, &error);

				/* allocate memory if needed */
				if (status == MB_SUCCESS && nanav > 0 && n_nav + nanav >= n_nav_alloc) {
					n_nav_alloc += std::max(MBSSLAYOUT_ALLOC_CHUNK, nanav);
					status = mb_reallocd(verbose, __FILE__, __LINE__, n_nav_alloc * sizeof(double), (void **)&nav_time_d, &error);
					status &= mb_reallocd(verbose, __FILE__, __LINE__, n_nav_alloc * sizeof(double), (void **)&nav_navlon, &error);
					status &= mb_reallocd(verbose, __FILE__, __LINE__, n_nav_alloc * sizeof(double), (void **)&nav_navlat, &error);
					status &= mb_reallocd(verbose, __FILE__, __LINE__, n_nav_alloc * sizeof(double), (void **)&nav_speed, &error);
					if (error != MB_ERROR_NO_ERROR) {
						char *message;
						mb_error(verbose, error, &message);
						fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
						fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
						exit(error);
					}
				}

				/* copy the nav data */
				if (status == MB_SUCCESS && nanav > 0) {
					for (int i = 0; i < nanav; i++) {
						nav_time_d[n_nav] = antime_d[i];
						nav_navlon[n_nav] = anlon[i];
						nav_navlat[n_nav] = anlat[i];
						nav_speed[n_nav] = anspeed[i];
						n_nav++;
					}
				}
			}

			/* look for sensordepth if not externally defined */
			if (status == MB_SUCCESS && sensordepth_mode == MBSSLAYOUT_MERGE_ASYNC && kind == sensordepth_async) {
				/* extract sensordepth data */
				status = mb_extract_nnav(verbose, imbio_ptr, istore_ptr, nanavmax, &kind, &nanav, antime_i, antime_d, anlon,
				                         anlat, anspeed, anheading, ansensordraft, anroll, anpitch, anheave, &error);

				/* allocate memory if needed */
				if (status == MB_SUCCESS && nanav > 0 && n_sensordepth + nanav >= n_sensordepth_alloc) {
					n_sensordepth_alloc += std::max(MBSSLAYOUT_ALLOC_CHUNK, nanav);
					status = mb_reallocd(verbose, __FILE__, __LINE__, n_sensordepth_alloc * sizeof(double),
					                     (void **)&sensordepth_time_d, &error);
					status &= mb_reallocd(verbose, __FILE__, __LINE__, n_sensordepth_alloc * sizeof(double),
					                     (void **)&sensordepth_sensordepth, &error);
					if (error != MB_ERROR_NO_ERROR) {
						char *message;
						mb_error(verbose, error, &message);
						fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
						fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
						exit(error);
					}
				}

				/* copy the sensordepth data */
				if (status == MB_SUCCESS && nanav > 0) {
					for (int i = 0; i < nanav; i++) {
						sensordepth_time_d[n_sensordepth] = antime_d[i];
						sensordepth_sensordepth[n_sensordepth] = ansensordraft[i] + anheave[i];
						n_sensordepth++;
					}
				}
			}

			/* look for altitude if not externally defined */
			if (status == MB_SUCCESS && altitude_mode == MBSSLAYOUT_MERGE_ASYNC && kind == altitude_async) {
				/* extract altitude data */
				status = mb_extract_nav(verbose, imbio_ptr, istore_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed,
				                        &heading, &sensordraft, &roll, &pitch, &heave, &error);
				status &= mb_extract_altitude(verbose, imbio_ptr, istore_ptr, &kind, &sensordepth, &altitude, &error);

				/* allocate memory if needed */
				if (status == MB_SUCCESS && n_altitude + 1 >= n_altitude_alloc) {
					n_altitude_alloc += std::max(MBSSLAYOUT_ALLOC_CHUNK, nanav);
					status = mb_reallocd(verbose, __FILE__, __LINE__, n_altitude_alloc * sizeof(double),
					                     (void **)&altitude_time_d, &error);
					status &= mb_reallocd(verbose, __FILE__, __LINE__, n_altitude_alloc * sizeof(double),
					                     (void **)&altitude_altitude, &error);
					if (error != MB_ERROR_NO_ERROR) {
						char *message;
						mb_error(verbose, error, &message);
						fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
						fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
						exit(error);
					}
				}

				/* copy the altitude data */
				if (status == MB_SUCCESS && nanav > 0) {
					altitude_time_d[n_altitude] = time_d;
					altitude_altitude[n_altitude] = altitude;
					n_altitude++;
				}
			}

			/* look for heading if not externally defined */
			if (status == MB_SUCCESS && heading_mode == MBSSLAYOUT_MERGE_ASYNC && kind == heading_async) {
				/* extract heading data */
				status = mb_extract_nnav(verbose, imbio_ptr, istore_ptr, nanavmax, &kind, &nanav, antime_i, antime_d, anlon,
				                         anlat, anspeed, anheading, ansensordraft, anroll, anpitch, anheave, &error);

				/* allocate memory if needed */
				if (status == MB_SUCCESS && nanav > 0 && n_heading + nanav >= n_heading_alloc) {
					n_heading_alloc += std::max(MBSSLAYOUT_ALLOC_CHUNK, nanav);
					status = mb_reallocd(verbose, __FILE__, __LINE__, n_heading_alloc * sizeof(double), (void **)&heading_time_d,
					                     &error);
					status &= mb_reallocd(verbose, __FILE__, __LINE__, n_heading_alloc * sizeof(double), (void **)&heading_heading,
					                     &error);
					if (error != MB_ERROR_NO_ERROR) {
						char *message;
						mb_error(verbose, error, &message);
						fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
						fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
						exit(error);
					}
				}

				/* copy the heading data */
				if (status == MB_SUCCESS && nanav > 0) {
					for (int i = 0; i < nanav; i++) {
						heading_time_d[n_heading] = antime_d[i];
						heading_heading[n_heading] = anheading[i];
						n_heading++;
					}
				}
			}

			/* look for attitude if not externally defined */
			if (status == MB_SUCCESS && attitude_mode == MBSSLAYOUT_MERGE_ASYNC && kind == attitude_async) {
				/* extract attitude data */
				status = mb_extract_nnav(verbose, imbio_ptr, istore_ptr, nanavmax, &kind, &nanav, antime_i, antime_d, anlon,
				                         anlat, anspeed, anheading, ansensordraft, anroll, anpitch, anheave, &error);

				/* allocate memory if needed */
				if (status == MB_SUCCESS && nanav > 0 && n_attitude + nanav >= n_attitude_alloc) {
					n_attitude_alloc += std::max(MBSSLAYOUT_ALLOC_CHUNK, nanav);
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
						fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
						fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
						exit(error);
					}
				}

				/* copy the attitude data */
				if (status == MB_SUCCESS && nanav > 0) {
					for (int i = 0; i < nanav; i++) {
						attitude_time_d[n_attitude] = antime_d[i];
						attitude_roll[n_attitude] = anroll[i];
						attitude_pitch[n_attitude] = anpitch[i];
						attitude_heave[n_attitude] = anheave[i];
						n_attitude++;
					}
				}
			}

			/* look for soundspeed if not externally defined */
			if (status == MB_SUCCESS && soundspeed_mode == MBSSLAYOUT_MERGE_ASYNC && kind == sensordepth_async) {
				/* extract soundspeed data */
				status = mb_ctd(verbose, imbio_ptr, istore_ptr, &kind, &nactd, actime_d, acconductivity, actemperature, acdepth,
				                acsalinity, acsoundspeed, &error);

				/* allocate memory if needed */
				if (status == MB_SUCCESS && nactd > 0 && n_soundspeed + nactd >= n_soundspeed_alloc) {
					n_soundspeed_alloc += std::max(MBSSLAYOUT_ALLOC_CHUNK, nactd);
					status = mb_reallocd(verbose, __FILE__, __LINE__, n_soundspeed_alloc * sizeof(double),
					                     (void **)&soundspeed_time_d, &error);
					status &= mb_reallocd(verbose, __FILE__, __LINE__, n_soundspeed_alloc * sizeof(double),
					                     (void **)&soundspeed_soundspeed, &error);
					if (error != MB_ERROR_NO_ERROR) {
						char *message;
						mb_error(verbose, error, &message);
						fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
						fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
						exit(error);
					}
				}

				/* copy the soundspeed data */
				if (status == MB_SUCCESS && nactd > 0) {
					for (int i = 0; i < nactd; i++) {
						soundspeed_time_d[n_soundspeed] = actime_d[i];
						soundspeed_soundspeed[n_soundspeed] = acsoundspeed[i];
						n_soundspeed++;
					}
				}
			}
		}

		/* output data counts */
		if (verbose > 0) {
			fprintf(stderr, "Pass 1: Records read from input file %s\n", ifile);
			fprintf(stderr, "     %d survey records\n", n_rf_data);
			fprintf(stderr, "     %d comment records\n", n_rf_comment);
			fprintf(stderr, "     %d sidescan2 records\n", n_rf_ss2);
			fprintf(stderr, "     %d sidescan3 records\n", n_rf_ss3);
			fprintf(stderr, "     %d subbottom records\n", n_rf_sbp);
			fprintf(stderr, "     %d nav records\n", n_rf_nav);
			fprintf(stderr, "     %d nav1 records\n", n_rf_nav1);
			fprintf(stderr, "     %d nav2 records\n", n_rf_nav2);
			fprintf(stderr, "     %d nav3 records\n", n_rf_nav3);
		}

		/* close the swath file */
		status = mb_close(verbose, &imbio_ptr, &error);

		/* figure out whether and what to read next */
		if (read_datalist) {
			read_data = mb_datalist_read(verbose, datalist, ifile, dfile, &iformat, &file_weight, &error) == MB_SUCCESS;
		} else {
			read_data = false;
		}

		/* end loop over files in list */
	}
	if (read_datalist)
		mb_datalist_close(verbose, &datalist, &error);

	/* output data counts */
	if (verbose > 0) {
		fprintf(stderr, "\nPass 1: Total records read from all input files\n");
		fprintf(stderr, "     %d survey records\n", n_rt_data);
		fprintf(stderr, "     %d comment records\n", n_rt_comment);
		fprintf(stderr, "     %d sidescan2 records\n", n_rt_ss2);
		fprintf(stderr, "     %d sidescan3 records\n", n_rt_ss3);
		fprintf(stderr, "     %d subbottom records\n", n_rt_sbp);
		fprintf(stderr, "     %d nav records\n", n_rt_nav);
		fprintf(stderr, "     %d nav1 records\n", n_rt_nav1);
		fprintf(stderr, "     %d nav2 records\n", n_rt_nav2);
		fprintf(stderr, "     %d nav3 records\n", n_rt_nav3);

	/* end first pass through data */

	/*-------------------------------------------------------------------*/

	/* deal with time latency corrections */

		fprintf(stderr, "\n-----------------------------------------------\n");
		fprintf(stderr, "Applying time latency corrections:\n");
	}

	/* position */
	if (n_nav > 0 && n_nav_alloc >= n_nav) {
		/* apply time latency correction called for in the platform file */
		if (sensor_position != nullptr && sensor_position->time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) {
			if (verbose > 0)
				fprintf(stderr, "Applying time latency correction from platform model to %d position data using mode %d\n", n_nav,
				        sensor_position->time_latency_mode);
			mb_apply_time_latency(verbose, n_nav, nav_time_d, sensor_position->time_latency_mode,
			                      sensor_position->time_latency_static, sensor_position->num_time_latency,
			                      sensor_position->time_latency_time_d, sensor_position->time_latency_value, &error);
		}

		/* apply time latency correction called for on the command line */
		if ((time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) && (time_latency_apply & MBSSLAYOUT_TIME_LATENCY_APPLY_NAV)) {
			if (verbose > 0)
				fprintf(stderr, "Applying time latency correction from command line to %d position data using mode %d\n", n_nav,
				        time_latency_mode);
			mb_apply_time_latency(verbose, n_nav, nav_time_d, time_latency_mode, time_latency_constant, time_latency_num,
			                      time_latency_time_d, time_latency_time_latency, &error);
		}
	}

	/* sensordepth */
	if (n_sensordepth > 0 && n_sensordepth_alloc >= n_sensordepth) {
		/* apply time latency correction called for in the platform file */
		if (sensor_depth != nullptr && sensor_depth->time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) {
			if (verbose > 0)
				fprintf(stderr, "Applying time latency correction from platform model to %d sensordepth data using mode %d\n",
				        n_sensordepth, sensor_depth->time_latency_mode);
			mb_apply_time_latency(verbose, n_sensordepth, sensordepth_time_d, sensor_depth->time_latency_mode,
			                      sensor_depth->time_latency_static, sensor_depth->num_time_latency,
			                      sensor_depth->time_latency_time_d, sensor_depth->time_latency_value, &error);
		}

		/* apply time latency correction called for on the command line */
		if ((time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) &&
		    (time_latency_apply & MBSSLAYOUT_TIME_LATENCY_APPLY_SENSORDEPTH)) {
			if (verbose > 0)
				fprintf(stderr, "Applying time latency correction from command line to %d sensordepth data using mode %d\n",
				        n_sensordepth, time_latency_mode);
			mb_apply_time_latency(verbose, n_sensordepth, sensordepth_time_d, time_latency_mode, time_latency_constant,
			                      time_latency_num, time_latency_time_d, time_latency_time_latency, &error);
		}
	}

	/* heading */
	if (n_heading > 0 && n_heading_alloc >= n_heading) {
		/* apply time latency correction called for in the platform file */
		if (sensor_heading != nullptr && sensor_heading->time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) {
			if (verbose > 0)
				fprintf(stderr, "Applying time latency correction from platform model to %d heading data using mode %d\n",
				        n_heading, sensor_heading->time_latency_mode);
			mb_apply_time_latency(verbose, n_heading, heading_time_d, sensor_heading->time_latency_mode,
			                      sensor_heading->time_latency_static, sensor_heading->num_time_latency,
			                      sensor_heading->time_latency_time_d, sensor_heading->time_latency_value, &error);
		}

		/* apply time latency correction called for on the command line */
		if ((time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) && (time_latency_apply & MBSSLAYOUT_TIME_LATENCY_APPLY_HEADING)) {
			if (verbose > 0)
				fprintf(stderr, "Applying time latency correction from command line to %d heading data using mode %d\n",
				        n_heading, time_latency_mode);
			mb_apply_time_latency(verbose, n_heading, heading_time_d, time_latency_mode, time_latency_constant, time_latency_num,
			                      time_latency_time_d, time_latency_time_latency, &error);
		}
	}

	/* altitude */
	if (n_altitude > 0 && n_altitude_alloc >= n_altitude) {
		/* apply time latency correction called for on the command line */
		if ((time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) && (time_latency_apply & MBSSLAYOUT_TIME_LATENCY_APPLY_ALTITUDE)) {
			if (verbose > 0)
				fprintf(stderr, "Applying time latency correction from command line to %d altitude data using mode %d\n",
				        n_altitude, time_latency_mode);
			mb_apply_time_latency(verbose, n_altitude, altitude_time_d, time_latency_mode, time_latency_constant,
			                      time_latency_num, time_latency_time_d, time_latency_time_latency, &error);
		}
	}

	/* attitude */
	if (n_attitude > 0 && n_attitude_alloc >= n_attitude) {
		/* apply time latency correction called for in the platform file */
		fprintf(stderr, "Attitude first sample before: %f %f %f\n", attitude_time_d[0], attitude_roll[0], attitude_pitch[0]);
		if (sensor_rollpitch != nullptr && sensor_rollpitch->time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) {
			if (verbose > 0)
				fprintf(stderr, "Applying time latency correction from platform model to %d attitude data using mode %d\n",
				        n_attitude, sensor_rollpitch->time_latency_mode);
			mb_apply_time_latency(verbose, n_attitude, attitude_time_d, sensor_rollpitch->time_latency_mode,
			                      sensor_rollpitch->time_latency_static, sensor_rollpitch->num_time_latency,
			                      sensor_rollpitch->time_latency_time_d, sensor_rollpitch->time_latency_value, &error);
		}

		/* apply time latency correction called for on the command line */
		if ((time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) && (time_latency_apply & MBSSLAYOUT_TIME_LATENCY_APPLY_ATTITUDE)) {
			if (verbose > 0)
				fprintf(stderr, "Applying time latency correction from command line to %d attitude data using mode %d\n",
				        n_attitude, time_latency_mode);
			mb_apply_time_latency(verbose, n_attitude, attitude_time_d, time_latency_mode, time_latency_constant,
			                      time_latency_num, time_latency_time_d, time_latency_time_latency, &error);
		}
		fprintf(stderr, "Attitude first sample after: %f %f %f\n", attitude_time_d[0], attitude_roll[0], attitude_pitch[0]);
	}

	/* sound speed */
	if (n_soundspeed > 0 && n_soundspeed_alloc >= n_soundspeed) {
		/* apply time latency correction called for on the command line */
		if ((time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE) && (time_latency_apply & MBSSLAYOUT_TIME_LATENCY_APPLY_ATTITUDE)) {
			if (verbose > 0)
				fprintf(stderr, "Applying time latency correction from command line to %d soundspeed data using mode %d\n",
				        n_soundspeed, time_latency_mode);
			mb_apply_time_latency(verbose, n_soundspeed, soundspeed_time_d, time_latency_mode, time_latency_constant,
			                      time_latency_num, time_latency_time_d, time_latency_time_latency, &error);
		}
		fprintf(stderr, "Attitude first sample after: %f %f\n", soundspeed_time_d[0], soundspeed_soundspeed[0]);
	}

	/*-------------------------------------------------------------------*/

	/* Do second pass through the data reading everything,
	    correcting survey data, and outputting everything */

	/* zero file count records */
	n_rf_data = 0;
	n_rf_comment = 0;
	n_rf_ss2 = 0;
	n_rf_ss3 = 0;
	n_rf_sbp = 0;
	n_rf_nav = 0;
	n_rf_nav1 = 0;
	n_rf_nav2 = 0;
	n_rf_nav3 = 0;
	n_rt_data = 0;
	n_rt_comment = 0;
	n_rt_ss2 = 0;
	n_rt_ss3 = 0;
	n_rt_sbp = 0;
	n_rt_nav = 0;
	n_rt_nav1 = 0;
	n_rt_nav2 = 0;
	n_rt_nav3 = 0;
	int n_wf_data = 0;
	int n_wf_comment = 0;
	int n_wt_data = 0;
	int n_wt_comment = 0;

	/* if generating survey line files the line number is initialized to 0 so the first line is 1 */
	int line_number = 0;
	if (line_mode != MBSSLAYOUT_LINE_OFF) {
		line_number = activewaypoint;
		new_output_file = true;
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
		/* else copy single filename to be read */
		strcpy(ifile, read_file);
		iformat = format;
		read_data = true;
	}

	int num_samples_stbd_alloc = 0;
	int num_samples_port_alloc = 0;

	/* MBIO read control parameters */
	char output_file[2*MB_PATH_MAXLINE+100] = "";
	mb_path ifileroot;
	mb_path ofile = "";

	/* MBIO read values */
	void *ombio_ptr = nullptr;
	struct mb_io_struct *omb_io_ptr;
	void *ostore_ptr = nullptr;
	struct mbsys_ldeoih_struct *ostore;

	double soundspeed;
	double navlon_org;
	double navlat_org;
	double speed_org;
	double heading_org;
	double altitude_org;
	double sensordepth_org;
	double draft_org;
	double roll_org;
	double pitch_org;
	double heave_org;
	double ss_altitude;

	/* raw sidescan */
	int sidescan_type = MB_SIDESCAN_LINEAR;
	double sample_interval;
	double beamwidth_xtrack = 0.0;
	double beamwidth_ltrack = 0.0;
	int num_samples_port = 0;
	double *raw_samples_port = nullptr;
	int num_samples_stbd = 0;
	double *raw_samples_stbd = nullptr;

	/* bottom layout parameters */
	int nangle = MBSSLAYOUT_NUM_ANGLES;
	double angle_min = -MBSSLAYOUT_ANGLE_MAX;
	double angle_max = MBSSLAYOUT_ANGLE_MAX;
	double table_angle[MBSSLAYOUT_NUM_ANGLES];
	double table_xtrack[MBSSLAYOUT_NUM_ANGLES];
	double table_ltrack[MBSSLAYOUT_NUM_ANGLES];
	double table_altitude[MBSSLAYOUT_NUM_ANGLES];
	double table_range[MBSSLAYOUT_NUM_ANGLES];

	/* output sidescan data */
	int obeams_bath;
	int obeams_amp;
	int opixels_ss;
	double oss[MBSSLAYOUT_SSDIMENSION];
	double ossacrosstrack[MBSSLAYOUT_SSDIMENSION];
	double ossalongtrack[MBSSLAYOUT_SSDIMENSION];
	int ossbincount[MBSSLAYOUT_SSDIMENSION];
	double pixel_width;

	/* loop over all files to be read */
	while (read_data) {
		if (verbose > 0)
			fprintf(stderr, "\nPass 2: Opening input file:  %s %d\n", ifile, iformat);

		/* initialize reading the input file */
		if (mb_read_init(verbose, ifile, iformat, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap,
		                           &imbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
			char *message;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
			fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", ifile);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}

		int interp_error = MB_ERROR_NO_ERROR;
		double time_latency;
		int jsurvey = 0;
		int jnav = 0;
		int jsensordepth = 0;
		int jaltitude = 0;
		int jheading = 0;
		int jattitude = 0;
		int jsoundspeed = 0;
		double channelmax;
		double threshold;
		double ttime;
		int portchannelpick;
		int stbdchannelpick;
		double factor;
		double fraction;
		int format_nottobeused = 0;

	        // get the fileroot (but don't use the format id returned here, we already
		// know the format, probably from a datalist)
		int error_format = MB_ERROR_NO_ERROR;

	        const int status_format = mb_get_format(verbose, ifile, ifileroot, &format_nottobeused, &error_format);

	        if (status_format != MB_SUCCESS)
			strcpy(ifileroot, ifile);

		/* if not generating survey line files then open output file to coincide with this input file */
		if (line_mode == MBSSLAYOUT_LINE_OFF)
			new_output_file = true;

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
			fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}

		/* zero file count records */
		n_rf_data = 0;
		n_rf_comment = 0;
		n_rf_ss2 = 0;
		n_rf_ss3 = 0;
		n_rf_sbp = 0;
		n_rf_nav = 0;
		n_rf_nav1 = 0;
		n_rf_nav2 = 0;
		n_rf_nav3 = 0;

		/* ------------------------------- */
		/* start read + output loop */
		while (error <= MB_ERROR_NO_ERROR) {
			/* reset error */
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;

			/* read next data record */
			/* status = */ mb_get_all(verbose, imbio_ptr, &istore_ptr, &kind, time_i, &time_d, &navlon_org, &navlat_org, &speed_org,
			                    &heading_org, &distance, &altitude_org, &sensordepth_org, &beams_bath, &beams_amp, &pixels_ss,
			                    beamflag, bath, amp, bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment,
			                    &error);

			/* some nonfatal errors do not matter */
			if (error < MB_ERROR_NO_ERROR && error > MB_ERROR_UNINTELLIGIBLE) {
				error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
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
			else if (kind == MB_DATA_SIDESCAN2) {
				n_rf_ss2++;
				n_rt_ss2++;
			}
			else if (kind == MB_DATA_SIDESCAN3) {
				n_rf_ss3++;
				n_rt_ss3++;
			}
			else if (kind == MB_DATA_SUBBOTTOM_SUBBOTTOM) {
				n_rf_sbp++;
				n_rt_sbp++;
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

			/* check for new line only if generating survey line files
			    and new line not already set
			    and this record is target data */
			if (status == MB_SUCCESS && line_mode != MBSSLAYOUT_LINE_OFF && !new_output_file && kind == output_source) {
				/* check waypoint time list */
				if (line_mode == MBSSLAYOUT_LINE_TIME && time_d >= routetime_d[activewaypoint] && activewaypoint < ntimepoint) {
					new_output_file = true;
					activewaypoint++;
					line_number = activewaypoint;
				}

				/* check waypoint position list */
				else if (line_mode == MBSSLAYOUT_LINE_ROUTE) {
					const double dx = (navlon - routelon[activewaypoint]) / mtodeglon;
					const double dy = (navlat - routelat[activewaypoint]) / mtodeglat;
					const double range = sqrt(dx * dx + dy * dy);
					if (range < line_range_threshold && (activewaypoint == 0 || range > rangelast) &&
					    activewaypoint < nroutepoint - 1) {
						new_output_file = true;
						activewaypoint++;
						line_number = activewaypoint;
					}
				}
			}

			/* open output files if needed */
			if (new_output_file) {
				/* reset flag */
				new_output_file = false;

				if (output_source != MB_DATA_NONE) {
					/* close any old output file unless a single file has been specified */
					if (ombio_ptr != nullptr) {
						/* close the swath file */
						/* status = */ mb_close(verbose, &ombio_ptr, &error);

						/* generate inf file */
						/* if (status == MB_SUCCESS)
						    {
						    status = mb_make_info(verbose, true,
						                output_file,
						                MBF_MBLDEOIH,
						                &error);
						    }*/

						/* output counts */
						if (verbose > 0) {
							fprintf(stdout, "\nPass 2: Closing output file: %s\n", output_file);
							fprintf(stdout, "Pass 2: Records written to output file %s\n", output_file);
							fprintf(stdout, "     %d survey records\n", n_wf_data);
							fprintf(stdout, "     %d comment records\n", n_wf_comment);
						}

						/* output commands to first cut plotting script file */
						fprintf(sfp, "# Generate swath plot of sidescan file: %s\n", output_file);
						fprintf(sfp, "mbm_plot -I %s -N -G5 -S -Pb -V -O %s_ssrawplot\n", output_file, output_file);
						fprintf(sfp, "%s_ssrawplot.cmd $1\n", output_file);
                        fprintf(sfp, "gmt psconvert %s_ssrawplot.ps -Tj -A -E300 -P\n\n", output_file);
                        fflush(sfp);
					}

					/* define the filename */
					if (line_mode == MBSSLAYOUT_LINE_OFF)
						snprintf(output_file, sizeof(output_file), "%s_%s.mb%2.2d", ifileroot, line_name2, MBF_MBLDEOIH);
					else
						snprintf(output_file, sizeof(output_file), "%s_%s_%4.4d.mb%2.2d", line_name1, line_name2, line_number, MBF_MBLDEOIH);

					/* open the new file */
					if (verbose > 0)
						fprintf(stderr, "Pass 2: Opening output file:  %s %d\n", output_file, MBF_MBLDEOIH);
					if ((status = mb_write_init(verbose, output_file, MBF_MBLDEOIH, &ombio_ptr, &obeams_bath, &obeams_amp,
					                            &opixels_ss, &error)) != MB_SUCCESS) {
						char *message;
						mb_error(verbose, error, &message);
						fprintf(stderr, "\nMBIO Error returned from function <mb_write_init>:\n%s\n", message);
						fprintf(stderr, "\nMultibeam File <%s> not initialized for writing\n", output_file);
						fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
						exit(error);
					}

					/* get pointers to data storage */
					omb_io_ptr = (struct mb_io_struct *)ombio_ptr;
					ostore_ptr = omb_io_ptr->store_data;
					ostore = (struct mbsys_ldeoih_struct *)ostore_ptr;

					n_wf_data = 0;
					n_wf_comment = 0;
				}
			}

			/* if data of interest have been read process them */
			if (status == MB_SUCCESS && kind == output_source) {
				/* start out with no change defined */
				// bool data_changed = false;

				/* call mb_extract_rawssdimensions() */
				status = mb_extract_rawssdimensions(verbose, imbio_ptr, istore_ptr, &kind, &sample_interval, &num_samples_port,
				                                    &num_samples_stbd, &error);

				/* allocate memory if necessary */
				if (num_samples_port > num_samples_port_alloc) {
					num_samples_port_alloc = num_samples_port;
					status = mb_reallocd(verbose, __FILE__, __LINE__, num_samples_port_alloc * sizeof(double),
					                     (void **)&raw_samples_port, &error);
					if (error != MB_ERROR_NO_ERROR) {
						char *message;
						mb_error(verbose, error, &message);
						fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
						fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
						exit(error);
					}
				}
				if (num_samples_stbd > num_samples_stbd_alloc) {
					num_samples_stbd_alloc = num_samples_stbd;
					status = mb_reallocd(verbose, __FILE__, __LINE__, num_samples_stbd_alloc * sizeof(double),
					                     (void **)&raw_samples_stbd, &error);
					if (error != MB_ERROR_NO_ERROR) {
						char *message;
						mb_error(verbose, error, &message);
						fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
						fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
						exit(error);
					}
				}

				/* call mb_extract_rawss() */
				/* status = */ mb_extract_rawss(verbose, imbio_ptr, istore_ptr, &kind, &sidescan_type, &sample_interval,
				                          &beamwidth_xtrack, &beamwidth_ltrack, &num_samples_port, raw_samples_port,
				                          &num_samples_stbd, raw_samples_stbd, &error);

				/* call mb_extract_nav to get attitude */
				/* status = */ mb_extract_nav(verbose, imbio_ptr, istore_ptr, &kind, time_i, &time_d, &navlon_org, &navlat_org,
				                        &speed_org, &heading_org, &draft_org, &roll_org, &pitch_org, &heave_org, &error);

				/* save the original values */
				navlon = navlon_org;
				navlat = navlat_org;
				speed = speed_org;
				heading = heading_org;
				altitude = altitude_org;
				sensordepth = sensordepth_org;
				// const double draft = draft_org;
				roll = roll_org;
				pitch = pitch_org;
				heave = heave_org;
				soundspeed = soundspeed_constant;
				// int interp_status = MB_SUCCESS;

				/* apply time_latency to survey data */
				if (time_latency_apply & MBSSLAYOUT_TIME_LATENCY_APPLY_SURVEY) {
					if (time_latency_mode == MBSSLAYOUT_TIME_LATENCY_FILE) {
						/* interp_status = */ mb_linear_interp(verbose, time_latency_time_d - 1, time_latency_time_latency - 1,
						                                 time_latency_num, time_d, &time_latency, &jsurvey, &interp_error);
						time_d += time_latency;
					}
					else if (time_latency_mode == MBSSLAYOUT_TIME_LATENCY_CONSTANT) {
						time_d += time_latency_constant;
					}
				}

				/* get nav sensordepth heading attitude values for record timestamp */
				if (n_nav > 0) {
					/* interp_status = */ mb_linear_interp_longitude(verbose, nav_time_d - 1, nav_navlon - 1, n_nav, time_d, &navlon,
					                                           &jnav, &interp_error);
					/* interp_status = */ mb_linear_interp_latitude(verbose, nav_time_d - 1, nav_navlat - 1, n_nav, time_d, &navlat,
					                                          &jnav, &interp_error);
					/* interp_status = */
					    mb_linear_interp(verbose, nav_time_d - 1, nav_speed - 1, n_nav, time_d, &speed, &jnav, &interp_error);
					// data_changed = true;
				}
				if (n_sensordepth > 0) {
					/* interp_status = */ mb_linear_interp(verbose, sensordepth_time_d - 1, sensordepth_sensordepth - 1, n_sensordepth,
					                                 time_d, &sensordepth, &jsensordepth, &interp_error);
					// data_changed = true;
				}
				if (n_altitude > 0) {
					/* interp_status = */ mb_linear_interp(verbose, altitude_time_d - 1, altitude_altitude - 1, n_altitude, time_d,
					                                 &altitude, &jaltitude, &interp_error);
					// data_changed = true;
				}
				if (n_heading > 0) {
					/* interp_status = */ mb_linear_interp_heading(verbose, heading_time_d - 1, heading_heading - 1, n_heading, time_d,
					                                         &heading, &jheading, &interp_error);
					// data_changed = true;
				}
				if (n_attitude > 0) {
					/* interp_status = */ mb_linear_interp(verbose, attitude_time_d - 1, attitude_roll - 1, n_attitude, time_d, &roll,
					                                 &jattitude, &interp_error);
					/* interp_status = */ mb_linear_interp(verbose, attitude_time_d - 1, attitude_pitch - 1, n_attitude, time_d, &pitch,
					                                 &jattitude, &interp_error);
					/* interp_status = */ mb_linear_interp(verbose, attitude_time_d - 1, attitude_heave - 1, n_attitude, time_d, &heave,
					                                 &jattitude, &interp_error);
					// data_changed = true;
				}
				// if (n_sensordepth > 0 || n_attitude > 0) {
				//	draft = sensordepth - heave;
				// }
				if (n_soundspeed > 0) {
					/* interp_status = */ mb_linear_interp(verbose, soundspeed_time_d - 1, soundspeed_soundspeed - 1, n_soundspeed,
					                                 time_d, &soundspeed, &jsoundspeed, &interp_error);
					// data_changed = true;
				}

				/* if platform defined, do lever arm correction */
				if (platform != nullptr) {
					/* calculate target sensor position */
					status = mb_platform_position(verbose, (void *)platform, target_sensor, 0, navlon, navlat, sensordepth,
					                              heading, roll, pitch, &navlon, &navlat, &sensordepth, &error);
					// draft = sensordepth - heave;
					// data_changed = true;

					/* calculate target sensor attitude */
					/* status = */ mb_platform_orientation_target(verbose, (void *)platform, target_sensor, 0, heading, roll, pitch,
					                                        &heading, &roll, &pitch, &error);
				}

				/* if specified get altitude from raw sidescan */
				if (ss_altitude_mode == MBSSLAYOUT_ALTITUDE_BOTTOMPICK) {
          int istart = bottompick_blank / sample_interval;

					/* get bottom arrival in port trace */
					channelmax = 0.0;
					for (int i = 0; i < num_samples_port; i++) {
						channelmax = std::max(raw_samples_port[i], channelmax);
					}
					portchannelpick = 0;
					threshold = bottompick_threshold * channelmax;
					for (int i = istart; i < num_samples_port && portchannelpick == 0; i++) {
						if (portchannelpick == 0 && raw_samples_port[i] >= threshold)
							portchannelpick = i;
					}

					/* get bottom arrival in starboard trace */
					channelmax = 0.0;
					for (int i = 0; i < num_samples_stbd; i++) {
						channelmax = std::max(raw_samples_stbd[i], channelmax);
					}
					stbdchannelpick = 0;
					threshold = bottompick_threshold * channelmax;
					for (int i = istart; i < num_samples_stbd && stbdchannelpick == 0; i++) {
						if (raw_samples_stbd[i] >= threshold)
							stbdchannelpick = i;
					}
					ttime = 0.5 * ((portchannelpick + stbdchannelpick) * sample_interval);
					ss_altitude = 0.5 * soundspeed * ttime;
				}

				/* else if getting altitude from topography model set initial value zero */
				else if (ss_altitude_mode == MBSSLAYOUT_ALTITUDE_TOPO_GRID) {
					mb_topogrid_topo(verbose, topogrid_ptr, navlon, navlat, &topo, &error);
					ss_altitude = -sensordepth - topo;
				}

				/* else just use existing altitude value */
				else if (ss_altitude_mode == MBSSLAYOUT_ALTITUDE_ALTITUDE) {
					ss_altitude = altitude;
				}

				/* get flat bottom layout table */
				if (layout_mode == MBSSLAYOUT_LAYOUT_FLATBOTTOM)
					mbsslayout_get_flatbottom_table(verbose, nangle, angle_min, angle_max, navlon, navlat, ss_altitude, 0.0,
					                                table_angle, table_xtrack, table_ltrack, table_altitude, table_range, &error);
				/* else get 3D bottom layout table */
				else {
					mb_topogrid_getangletable(verbose, topogrid_ptr, nangle, angle_min, angle_max, navlon, navlat, heading,
					                          ss_altitude, sensordepth, pitch, table_angle, table_xtrack, table_ltrack,
					                          table_altitude, table_range, &error);
				}
				/* set some values */
				ostore->depth_scale = 0;
				ostore->distance_scale = 0;
				ostore->beam_xwidth = beamwidth_xtrack;
				ostore->beam_lwidth = beamwidth_ltrack;
				ostore->kind = MB_DATA_DATA;
				ostore->ss_type = sidescan_type;
				opixels_ss = MBSSLAYOUT_SSDIMENSION;

				/* set one bathymetry sample from sensor depth and altitude */
				obeams_bath = 1;
				bath[0] = sensordepth + altitude;
				bathacrosstrack[0] = 0.0;
				bathalongtrack[0] = 0.0;

				/* get swath width and pixel size */
				if (swath_mode == MBSSLAYOUT_SWATHWIDTH_VARIABLE) {
					const double rr = 0.5 * soundspeed * sample_interval * std::max(num_samples_port, num_samples_stbd);
					swath_width = 2.2 * sqrt(rr * rr - ss_altitude * ss_altitude);
				}
				pixel_width = swath_width / (opixels_ss - 1);

				/* initialize the output sidescan */
				for (int j = 0; j < opixels_ss; j++) {
					oss[j] = 0.0;
					ossacrosstrack[j] = pixel_width * (double)(j - (opixels_ss / 2));
					ossalongtrack[j] = 0.0;
					ossbincount[j] = 0;
				}

				/* find minimum range */
				double rangemin = table_range[0];
				int kstart = 0;
				for (int kangle = 1; kangle < nangle; kangle++) {
					if (table_range[kangle] < rangemin) {
						rangemin = table_range[kangle];
						kstart = kangle;
					}
				}

				/* bin port trace */
				int istart = rangemin / (0.5 * soundspeed * sample_interval);
//const double rr = 0.5 * soundspeed * sample_interval * std::max(num_samples_port, num_samples_stbd);
//fprintf(stderr, "%s:%d:%s: sensordepth:%f altitude:%f swath_width:%f rr:%f rangemin:%f kstart:%d soundspeed:%f sample_interval:%f istart:%d\n",
//__FILE__, __LINE__, __FUNCTION__, sensordepth, altitude, swath_width, rr, table_range[kstart], kstart, soundspeed, sample_interval, istart);
				for (int i = istart; i < num_samples_port; i++) {
					/* get sample range */
					const double rr = 0.5 * soundspeed * sample_interval * i;

					/* look up position(s) for this range */
					bool done = false;
					for (int kangle = kstart; kangle > 0 && !done; kangle--) {
						bool found = false;
						double xtrack;
						double ltrack;
						if (rr <= table_range[kstart]) {
							xtrack = table_xtrack[kstart];
							ltrack = table_ltrack[kstart];
							done = true;
							found = true;
						}
						else if (rr > table_range[kangle] && rr <= table_range[kangle - 1]) {
							factor = (rr - table_range[kangle]) / (table_range[kangle - 1] - table_range[kangle]);
							xtrack = table_xtrack[kangle] + factor * (table_xtrack[kangle - 1] - table_xtrack[kangle]);
							ltrack = table_ltrack[kangle] + factor * (table_ltrack[kangle - 1] - table_ltrack[kangle]);
							found = true;
							done = true;
						}
						else if (rr < table_range[kangle] && rr >= table_range[kangle - 1]) {
							factor = (rr - table_range[kangle]) / (table_range[kangle - 1] - table_range[kangle]);
							xtrack = table_xtrack[kangle] + factor * (table_xtrack[kangle - 1] - table_xtrack[kangle]);
							ltrack = table_ltrack[kangle] + factor * (table_ltrack[kangle - 1] - table_ltrack[kangle]);
							found = true;
							done = true;
						}

						/* bin the value and position */
						if (found) {
							const int j = opixels_ss / 2 + (int)(xtrack / pixel_width);
//fprintf(stderr, "port sample %d: rr:%f xtrack:%f pixel_width:%f j:%d\n", i, rr, xtrack, pixel_width, j);
							if (j >= 0 && j < opixels_ss) {
								oss[j] += raw_samples_port[i];
								ossbincount[j]++;
								ossalongtrack[j] += ltrack;
							}
						}
					}
				}

				/* find minimum range */
				rangemin = table_range[0];
				kstart = 0;
				for (int kangle = 1; kangle < nangle; kangle++) {
					if (table_range[kangle] < rangemin) {
						rangemin = table_range[kangle];
						kstart = kangle;
					}
				}

				/* bin stbd trace */
				istart = rangemin / (0.5 * soundspeed * sample_interval);
				for (int i = istart; i < num_samples_stbd; i++) {
					/* get sample range */
					const double rr = 0.5 * soundspeed * sample_interval * i;

					/* look up position for this range */
					bool done = false;
					for (int kangle = kstart; kangle < nangle - 1 && !done; kangle++) {
						bool found = false;
						double xtrack;
						double ltrack;
						if (rr <= table_range[kstart]) {
							xtrack = table_xtrack[kstart];
							ltrack = table_ltrack[kstart];
							done = true;
							found = true;
						}
						else if (rr > table_range[kangle] && rr <= table_range[kangle + 1]) {
							factor = (rr - table_range[kangle]) / (table_range[kangle + 1] - table_range[kangle]);
							xtrack = table_xtrack[kangle] + factor * (table_xtrack[kangle + 1] - table_xtrack[kangle]);
							ltrack = table_ltrack[kangle] + factor * (table_ltrack[kangle + 1] - table_ltrack[kangle]);
							found = true;
							done = true;
						}
						else if (rr < table_range[kangle] && rr >= table_range[kangle + 1]) {
							factor = (rr - table_range[kangle]) / (table_range[kangle + 1] - table_range[kangle]);
							xtrack = table_xtrack[kangle] + factor * (table_xtrack[kangle + 1] - table_xtrack[kangle]);
							ltrack = table_ltrack[kangle] + factor * (table_ltrack[kangle + 1] - table_ltrack[kangle]);
							found = true;
							done = true;
						}

						/* bin the value and position */
						if (found) {
							const int j = opixels_ss / 2 + (int)(xtrack / pixel_width);
							if (j >= 0 && j < opixels_ss) {
								oss[j] += raw_samples_stbd[i];
								ossbincount[j]++;
								ossalongtrack[j] += ltrack;
							}
						}
					}
				}

				/* calculate the output sidescan */
				int jport = -1;
				// int jstbd = -1;
				for (int j = 0; j < opixels_ss; j++) {
					if (ossbincount[j] > 0) {
						oss[j] /= (double)ossbincount[j];
						ossalongtrack[j] /= (double)ossbincount[j];
						if (jport < 0)
							jport = j;
						// jstbd = j;
					}
					else
						oss[j] = MB_SIDESCAN_NULL;
				}

				/* interpolate gaps in the output sidescan */
				int previous = opixels_ss;
				for (int j = 0; j < opixels_ss; j++) {
					if (ossbincount[j] > 0) {
						const int interpable = j - previous - 1;
						if (interpable > 0 && interpable <= interpolation) {
							const double dss = oss[j] - oss[previous];
							const double dssl = ossalongtrack[j] - ossalongtrack[previous];
							for (int jj = previous + 1; jj < j; jj++) {
								fraction = ((double)(jj - previous)) / ((double)(j - previous));
								oss[jj] = oss[previous] + fraction * dss;
								ossalongtrack[jj] = ossalongtrack[previous] + fraction * dssl;
							}
						}
						previous = j;
					}
				}

				/* insert data */
				mb_insert_nav(verbose, ombio_ptr, (void *)ostore, time_i, time_d, navlon, navlat, speed, heading, sensordraft,
				              roll, pitch, heave, &error);
				/* status = */ mb_insert_altitude(verbose, ombio_ptr, (void *)ostore, sensordepth, ss_altitude, &error);
				/* status = */ mb_insert(verbose, ombio_ptr, (void *)ostore, MB_DATA_DATA, time_i, time_d, navlon, navlat, speed,
				                   heading, beams_bath, beams_amp, opixels_ss, beamflag, bath, amp, bathacrosstrack,
				                   bathalongtrack, oss, ossacrosstrack, ossalongtrack, comment, &error);
			}

			/* write some data */
			if (error == MB_ERROR_NO_ERROR && kind == output_source) {
				/* write the record */
				status = mb_write_ping(verbose, ombio_ptr, (void *)ostore, &error);
				if (status != MB_SUCCESS) {
					char *message;
					mb_error(verbose, error, &message);
					fprintf(stderr, "\nMBIO Error returned from function <mb_put>:\n%s\n", message);
					fprintf(stderr, "\nMultibeam Data Not Written To File <%s>\n", ofile);
					fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
					exit(error);
				}

				/* count records */
				if (kind == MB_DATA_COMMENT) {
					n_wf_comment++;
					n_wt_comment++;
				}
				else {
					n_wf_data++;
					n_wt_data++;
				}
			}
		}
		/* end read+process+output data loop */
		/* --------------------------------- */

		/* output data counts */
		if (verbose > 0) {
			fprintf(stderr, "Pass 2: Records read from input file %s\n", ifile);
			fprintf(stderr, "     %d survey records\n", n_rf_data);
			fprintf(stderr, "     %d comment records\n", n_rf_comment);
			fprintf(stderr, "     %d sidescan2 records\n", n_rf_ss2);
			fprintf(stderr, "     %d sidescan3 records\n", n_rf_ss3);
			fprintf(stderr, "     %d subbottom records\n", n_rf_sbp);
			fprintf(stderr, "     %d nav records\n", n_rf_nav);
			fprintf(stderr, "     %d nav1 records\n", n_rf_nav1);
			fprintf(stderr, "     %d nav2 records\n", n_rf_nav2);
			fprintf(stderr, "     %d nav3 records\n", n_rf_nav3);
		}

		/* status = */ mb_close(verbose, &imbio_ptr, &error);

		/* figure out whether and what to read next */
		if (read_datalist) {
			read_data = mb_datalist_read(verbose, datalist, ifile, dfile, &format, &file_weight, &error) == MB_SUCCESS;
		} else {
			read_data = false;
		}

		/* end loop over files in list */
	}
	if (read_datalist)
		mb_datalist_close(verbose, &datalist, &error);

	/* close any open output file */
	if (ombio_ptr != nullptr) {
		/* status = */ mb_close(verbose, &ombio_ptr, &error);

		/* generate inf file */
		/* if (status == MB_SUCCESS)
		    {
		    status = mb_make_info(verbose, true,
		                output_file,
		                MBF_MBLDEOIH,
		                &error);
		    }*/

		/* output counts */
		if (verbose > 0) {
			fprintf(stdout, "\nClosing output file: %s\n", output_file);
			fprintf(stdout, "Pass 2: Records written to output file %s\n", output_file);
			fprintf(stdout, "     %d survey records\n", n_wf_data);
			fprintf(stdout, "     %d comment records\n", n_wf_comment);
		}

		/* output commands to first cut plotting script file */
		fprintf(sfp, "# Generate swath plot of sidescan file: %s\n", output_file);
		fprintf(sfp, "mbm_plot -I %s -N -G5 -S -Pb -V -O %s_ssrawplot\n", output_file, output_file);
		fprintf(sfp, "%s_ssrawplot.cmd $1\n", output_file);
		fprintf(sfp, "gmt psconvert %s_ssrawplot.ps -Tj -A -E300 -P\n\n", output_file);
		fflush(sfp);
	}

	/* close plotting script file */
	fclose(sfp);
	char command[2*MB_PATH_MAXLINE+50] = "";
	snprintf(command, sizeof(command), "chmod +x %s", scriptfile);
	/* int shellstatus = */ system(command);

	/* output data counts */
	if (verbose > 0) {
		fprintf(stderr, "\nPass 2: Total records read from all input files\n");
		fprintf(stderr, "     %d survey records\n", n_rt_data);
		fprintf(stderr, "     %d comment records\n", n_rt_comment);
		fprintf(stderr, "     %d sidescan2 records\n", n_rt_ss2);
		fprintf(stderr, "     %d sidescan3 records\n", n_rt_ss3);
		fprintf(stderr, "     %d subbottom records\n", n_rt_sbp);
		fprintf(stderr, "     %d nav records\n", n_rt_nav);
		fprintf(stderr, "     %d nav1 records\n", n_rt_nav1);
		fprintf(stderr, "     %d nav2 records\n", n_rt_nav2);
		fprintf(stderr, "     %d nav3 records\n", n_rt_nav3);
		fprintf(stderr, "Pass 2: Total records written to all output files\n");
		fprintf(stderr, "     %d survey records\n", n_wt_data);
		fprintf(stderr, "     %d comment records\n", n_wt_comment);
	}

	/* end second pass through data */

	/*-------------------------------------------------------------------*/

	/* deallocate raw sidescan arrays */
	if (num_samples_stbd_alloc > 0) {
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&raw_samples_stbd, &error);
		// num_samples_stbd_alloc = 0;
	}
	if (num_samples_port_alloc > 0) {
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&raw_samples_port, &error);
		// num_samples_port_alloc = 0;
	}

	/* deallocate nav, sensordepth, heading, attitude, and time_latency arrays */
	if (n_nav_alloc > 0) {
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&nav_time_d, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&nav_navlon, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&nav_navlat, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&nav_speed, &error);
		n_nav_alloc = 0;
	}
	if (n_sensordepth_alloc > 0) {
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&sensordepth_time_d, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&sensordepth_sensordepth, &error);
		n_sensordepth_alloc = 0;
	}
	if (n_heading_alloc > 0) {
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&heading_time_d, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&heading_heading, &error);
		n_heading_alloc = 0;
	}
	if (n_attitude_alloc > 0) {
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&attitude_time_d, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&attitude_roll, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&attitude_pitch, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&attitude_heave, &error);
		n_attitude_alloc = 0;
	}
	if (time_latency_alloc > 0) {
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&time_latency_time_d, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&time_latency_time_latency, &error);
		time_latency_alloc = 0;
	}

	/* deallocate route arrays */
	if (line_mode == MBSSLAYOUT_LINE_TIME) {
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routelon, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routelat, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routeheading, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routewaypoint, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routetime_d, &error);
	}
	else if (line_mode == MBSSLAYOUT_LINE_ROUTE) {
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routelon, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routelat, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routeheading, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routewaypoint, &error);
	}

	/* deallocate topography grid array if necessary */
	if (layout_mode == MBSSLAYOUT_LAYOUT_3DTOPO)
		status &= mb_topogrid_deall(verbose, &topogrid_ptr, &error);

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
