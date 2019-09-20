/*--------------------------------------------------------------------
 *    The MB-system:	mbmakeplatform.c	9/5/2015
 *
 *    Copyright (c) 2015-2019 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * Mbmakeplatform creates an MB-System platform file from command line arguments
 * specifying the positional and angular offsets between the various sensors
 *
 * Author:	D. W. Caress
 * Date:	September 5, 2015
 *
 */

#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"

#define SENSOR_OFF 0
#define SENSOR_ADD 1
#define SENSOR_MODIFY 2

static const char program_name[] = "mbmakeplatform";
static const char help_message[] =
    "mbmakeplatform creates or modifies an MB-System platform file.\n";
static const char usage_message[] =
    "mbmakeplatform \n"
    "\t[\n"
    "\t--verbose\n"
    "\t--help\n"
    "\t--input=plffile\n"
    "\t--swath=datalist\n"
    "\t--swath=swathfile\n"
    "\t--swath-format=value\n"
    "\t]\n"
    "\t--output=plffile\n"
    "\t[\n"
    "\t--platform-type-surface-vessel\n"
    "\t--platform-type-tow-body\n"
    "\t--platform-type-rov\n"
    "\t--platform-type-auv\n"
    "\t--platform-type-aircraft\n"
    "\t--platform-type-satellite\n"
    "\t--platform-name=string\n"
    "\t--platform-organization=string\n"
    "\t--platform-documentation-url\n"
    "\t--platform-start-time\n"
    "\t--platform-end-time\n"
    "\t--add-sensor-sonar-echosounder\n"
    "\t--add-sensor-sonar-multiechosounder\n"
    "\t--add-sensor-sonar-sidescan\n"
    "\t--add-sensor-sonar-interferometry\n"
    "\t--add-sensor-sonar-multibeam\n"
    "\t--add-sensor-sonar-multibeam-twohead\n"
    "\t--add-sensor-sonar-subbottom\n"
    "\t--add-sensor-camera-mono\n"
    "\t--add-sensor-camera-stereo\n"
    "\t--add-sensor-camera-video\n"
    "\t--add-sensor-lidar-scan\n"
    "\t--add-sensor-lidar-swath\n"
    "\t--add-sensor-position\n"
    "\t--add-sensor-compass\n"
    "\t--add-sensor-vru\n"
    "\t--add-sensor-imu\n"
    "\t--add-sensor-ins\n"
    "\t--add-sensor-ins-with-pressure\n"
    "\t--add-sensor-ctd\n"
    "\t--add-sensor-pressure\n"
    "\t--add-sensor-soundspeed\n"
    "\t--end-sensor\n"
    "\t--sensor-model=string\n"
    "\t--sensor-manufacturer=string\n"
    "\t--sensor-serialnumber=string\n"
    "\t--sensor-capability-position\n"
    "\t--sensor-capability-depth\n"
    "\t--sensor-capability-altitude\n"
    "\t--sensor-capability-velocity\n"
    "\t--sensor-capability-acceleration\n"
    "\t--sensor-capability-pressure\n"
    "\t--sensor-capability-rollpitch\n"
    "\t--sensor-capability-heading\n"
    "\t--sensor-capability-magneticfield\n"
    "\t--sensor-capability-temperature\n"
    "\t--sensor-capability-conductivity\n"
    "\t--sensor-capability-salinity\n"
    "\t--sensor-capability-soundspeed\n"
    "\t--sensor-capability-gravity\n"
    "\t--sensor-capability-topography-echosounder\n"
    "\t--sensor-capability-topography-interferometry\n"
    "\t--sensor-capability-topography-sass\n"
    "\t--sensor-capability-topography-multibeam\n"
    "\t--sensor-capability-topography-photogrammetry\n"
    "\t--sensor-capability-topography-structurefrommotion\n"
    "\t--sensor-capability-topography-lidar\n"
    "\t--sensor-capability-topography-structuredlight\n"
    "\t--sensor-capability-topography-laserscanner\n"
    "\t--sensor-capability-backscatter-echosounder\n"
    "\t--sensor-capability-backscatter-sidescan\n"
    "\t--sensor-capability-backscatter-interferometry\n"
    "\t--sensor-capability-backscatter-sass\n"
    "\t--sensor-capability-backscatter-multibeam\n"
    "\t--sensor-capability-backscatter-lidar\n"
    "\t--sensor-capability-backscatter-structuredlight\n"
    "\t--sensor-capability-backscatter-laserscanner\n"
    "\t--sensor-capability-photography\n"
    "\t--sensor-capability-stereophotography\n"
    "\t--sensor-capability-video\n"
    "\t--sensor-capability-stereovideo\n"
    "\t--sensor-capability1=value\n"
    "\t--sensor-capability2=value\n"
    "\t--sensor-offsets=x/y/z/azimuth/roll/pitch\n"
    "\t--sensor-offset-positions=x/y/z\n"
    "\t--sensor-offset-angles=azimuth/roll/pitch\n"
    "\t--sensor-time-latency=value\n"
    "\t--sensor-time-latency-model=file\n"
    "\t--sensor-source-bathymetry\n"
    "\t--sensor-source-bathymetry1\n"
    "\t--sensor-source-bathymetry2\n"
    "\t--sensor-source-bathymetry3\n"
    "\t--sensor-source-backscatter\n"
    "\t--sensor-source-backscatter1\n"
    "\t--sensor-source-backscatter2\n"
    "\t--sensor-source-backscatter3\n"
    "\t--sensor-source-subbottom\n"
    "\t--sensor-source-subbottom1\n"
    "\t--sensor-source-subbottom2\n"
    "\t--sensor-source-subbottom3\n"
    "\t--sensor-source-position\n"
    "\t--sensor-source-position1\n"
    "\t--sensor-source-position2\n"
    "\t--sensor-source-position3\n"
    "\t--sensor-source-depth\n"
    "\t--sensor-source-depth1\n"
    "\t--sensor-source-depth2\n"
    "\t--sensor-source-depth3\n"
    "\t--sensor-source-heading\n"
    "\t--sensor-source-heading1\n"
    "\t--sensor-source-heading2\n"
    "\t--sensor-source-heading3\n"
    "\t--sensor-source-rollpitch\n"
    "\t--sensor-source-rollpitch1\n"
    "\t--sensor-source-rollpitch2\n"
    "\t--sensor-source-rollpitch3\n"
    "\t--sensor-source-heave\n"
    "\t--sensor-source-heave1\n"
    "\t--sensor-source-heave2\n"
    "\t--sensor-source-heave3\n"
    "\t--modify-sensor=sensorid\n"
    "\t--modify-sensor-bathymetry\n"
    "\t--modify-sensor-bathymetry1\n"
    "\t--modify-sensor-bathymetry2\n"
    "\t--modify-sensor-bathymetry3\n"
    "\t--modify-sensor-backscatter\n"
    "\t--modify-sensor-backscatter1\n"
    "\t--modify-sensor-backscatter2\n"
    "\t--modify-sensor-backscatter3\n"
    "\t--modify-sensor-subbottom\n"
    "\t--modify-sensor-subbottom1\n"
    "\t--modify-sensor-subbottom2\n"
    "\t--modify-sensor-subbottom3\n"
    "\t--modify-sensor-position\n"
    "\t--modify-sensor-position1\n"
    "\t--modify-sensor-position2\n"
    "\t--modify-sensor-position3\n"
    "\t--modify-sensor-depth\n"
    "\t--modify-sensor-depth1\n"
    "\t--modify-sensor-depth2\n"
    "\t--modify-sensor-depth3\n"
    "\t--modify-sensor-heading\n"
    "\t--modify-sensor-heading1\n"
    "\t--modify-sensor-heading2\n"
    "\t--modify-sensor-heading3\n"
    "\t--modify-sensor-rollpitch\n"
    "\t--modify-sensor-rollpitch1\n"
    "\t--modify-sensor-rollpitch2\n"
    "\t--modify-sensor-rollpitch3\n"
    "\t--modify-sensor-heave\n"
    "\t--modify-sensor-heave1\n"
    "\t--modify-sensor-heave2\n"
    "\t--modify-sensor-heave3\n"
    "\t--modify-offsets=ioff/x/y/z/azimuth/roll/pitch\n"
    "\t--modify-offset-positions=ioff/x/y/z\n"
    "\t--modify-offset-angles=ioff/azimuth/roll/pitch\n"
    "\t--modify-time-latency=value\n"
    "\t--modify-time-latency-model=file\n"
    "\t]\n";


/*--------------------------------------------------------------------*/
int main(int argc, char **argv) {
	int option_index;
	int errflg = 0;
	int c;

	/* MBIO status variables */
	int verbose = 0;
	int error = MB_ERROR_NO_ERROR;
	char *message;

	/* MBIO read control parameters */
	int read_datalist = MB_NO;
	mb_path swath_file;
	mb_path dfile;
	void *datalist;
	int look_processed = MB_DATALIST_LOOK_UNSET;
	int read_data;
	double file_weight;
	int pings = 1;
	int lonflip;
	double bounds[4];
	int btime_i[7];
	int etime_i[7];
	double btime_d;
	double etime_d;
	double speedmin;
	double timegap;
	int beams_bath;
	int beams_amp;
	int pixels_ss;

	/* MBIO read values */
	void *mbio_ptr = NULL;
	struct mb_io_struct *mb_io_ptr = NULL;
	void *store_ptr = NULL;
	int kind;

	/* data record source types */
	int platform_source;
	int nav_source;
	int heading_source;
	int sensordepth_source;
	int attitude_source;
	int svp_source;

	/* platform */
	struct mb_platform_struct *platform = NULL;
	struct mb_sensor_struct tmp_sensor;
	struct mb_sensor_struct *active_sensor;
	struct mb_sensor_offset_struct tmp_offsets[4];
	int platform_num_sensors = 0;
	mb_path input_platform_file;
	mb_path input_swath_file;
	int input_swath_format = 0;
	int input_swath_platform_defined = MB_NO;
	mb_path output_platform_file;
	int output_platform_file_defined = MB_NO;
	mb_path time_latency_model_file;
	FILE *tfp = NULL;
	char buffer[MB_PATH_MAXLINE];
	char *result;
	int sensor_mode = SENSOR_OFF;
	int sensor_id;
	int ioffset;

	int nscan;
	double d1, d2, d3, d4, d5, d6;
	double seconds;
	int index;
	int j;

	static struct option options[] = {{"verbose", no_argument, NULL, 0},
	                                  {"help", no_argument, NULL, 0},
	                                  {"input", required_argument, NULL, 0},
	                                  {"swath", required_argument, NULL, 0},
	                                  {"swath-format", required_argument, NULL, 0},
	                                  {"output", required_argument, NULL, 0},
	                                  {"platform-type-surface-vessel", no_argument, NULL, 0},
	                                  {"platform-type-tow-body", no_argument, NULL, 0},
	                                  {"platform-type-rov", no_argument, NULL, 0},
	                                  {"platform-type-auv", no_argument, NULL, 0},
	                                  {"platform-type-aircraft", no_argument, NULL, 0},
	                                  {"platform-type-satellite", no_argument, NULL, 0},
	                                  {"platform-name", required_argument, NULL, 0},
	                                  {"platform-organization", required_argument, NULL, 0},
	                                  {"platform-documenation-url", required_argument, NULL, 0},
	                                  {"platform-start-time", required_argument, NULL, 0},
	                                  {"platform-end-time", required_argument, NULL, 0},
	                                  {"add-sensor-sonar-echosounder", no_argument, NULL, 0},
	                                  {"add-sensor-sonar-multiechosounder", no_argument, NULL, 0},
	                                  {"add-sensor-sonar-sidescan", no_argument, NULL, 0},
	                                  {"add-sensor-sonar-interferometry", no_argument, NULL, 0},
	                                  {"add-sensor-sonar-multibeam", no_argument, NULL, 0},
	                                  {"add-sensor-sonar-multibeam-twohead", no_argument, NULL, 0},
	                                  {"add-sensor-sonar-subbottom", no_argument, NULL, 0},
	                                  {"add-sensor-camera-mono", no_argument, NULL, 0},
	                                  {"add-sensor-camera-stereo", no_argument, NULL, 0},
	                                  {"add-sensor-camera-video", no_argument, NULL, 0},
	                                  {"add-sensor-lidar-scan", no_argument, NULL, 0},
	                                  {"add-sensor-lidar-swath", no_argument, NULL, 0},
	                                  {"add-sensor-position", no_argument, NULL, 0},
	                                  {"add-sensor-compass", no_argument, NULL, 0},
	                                  {"add-sensor-vru", no_argument, NULL, 0},
	                                  {"add-sensor-imu", no_argument, NULL, 0},
	                                  {"add-sensor-ins", no_argument, NULL, 0},
	                                  {"add-sensor-ins-with-pressure", no_argument, NULL, 0},
	                                  {"add-sensor-ctd", no_argument, NULL, 0},
	                                  {"add-sensor-pressure", no_argument, NULL, 0},
	                                  {"add-sensor-soundspeed", no_argument, NULL, 0},
	                                  {"modify-sensor", required_argument, NULL, 0},
	                                  {"modify-sensor-bathymetry", no_argument, NULL, 0},
	                                  {"modify-sensor-bathymetry1", no_argument, NULL, 0},
	                                  {"modify-sensor-bathymetry2", no_argument, NULL, 0},
	                                  {"modify-sensor-bathymetry3", no_argument, NULL, 0},
	                                  {"modify-sensor-backscatter", no_argument, NULL, 0},
	                                  {"modify-sensor-backscatter1", no_argument, NULL, 0},
	                                  {"modify-sensor-backscatter2", no_argument, NULL, 0},
	                                  {"modify-sensor-backscatter3", no_argument, NULL, 0},
	                                  {"modify-sensor-subbottom", no_argument, NULL, 0},
	                                  {"modify-sensor-subbottom1", no_argument, NULL, 0},
	                                  {"modify-sensor-subbottom2", no_argument, NULL, 0},
	                                  {"modify-sensor-subbottom3", no_argument, NULL, 0},
	                                  {"modify-sensor-position", no_argument, NULL, 0},
	                                  {"modify-sensor-position1", no_argument, NULL, 0},
	                                  {"modify-sensor-position2", no_argument, NULL, 0},
	                                  {"modify-sensor-position3", no_argument, NULL, 0},
	                                  {"modify-sensor-depth", no_argument, NULL, 0},
	                                  {"modify-sensor-depth1", no_argument, NULL, 0},
	                                  {"modify-sensor-depth2", no_argument, NULL, 0},
	                                  {"modify-sensor-depth3", no_argument, NULL, 0},
	                                  {"modify-sensor-heading", no_argument, NULL, 0},
	                                  {"modify-sensor-heading1", no_argument, NULL, 0},
	                                  {"modify-sensor-heading2", no_argument, NULL, 0},
	                                  {"modify-sensor-heading3", no_argument, NULL, 0},
	                                  {"modify-sensor-rollpitch", no_argument, NULL, 0},
	                                  {"modify-sensor-rollpitch1", no_argument, NULL, 0},
	                                  {"modify-sensor-rollpitch2", no_argument, NULL, 0},
	                                  {"modify-sensor-rollpitch3", no_argument, NULL, 0},
	                                  {"modify-sensor-heave", no_argument, NULL, 0},
	                                  {"modify-sensor-heave1", no_argument, NULL, 0},
	                                  {"modify-sensor-heave2", no_argument, NULL, 0},
	                                  {"modify-sensor-heave3", no_argument, NULL, 0},
	                                  {"sensor-model", required_argument, NULL, 0},
	                                  {"sensor-manufacturer", required_argument, NULL, 0},
	                                  {"sensor-serialnumber", required_argument, NULL, 0},
	                                  {"sensor-capability-position", no_argument, NULL, 0},
	                                  {"sensor-capability-depth", no_argument, NULL, 0},
	                                  {"sensor-capability-altitude", no_argument, NULL, 0},
	                                  {"sensor-capability-velocity", no_argument, NULL, 0},
	                                  {"sensor-capability-acceleration", no_argument, NULL, 0},
	                                  {"sensor-capability-pressure", no_argument, NULL, 0},
	                                  {"sensor-capability-rollpitch", no_argument, NULL, 0},
	                                  {"sensor-capability-heading", no_argument, NULL, 0},
	                                  {"sensor-capability-magneticfield", no_argument, NULL, 0},
	                                  {"sensor-capability-temperature", no_argument, NULL, 0},
	                                  {"sensor-capability-conductivity", no_argument, NULL, 0},
	                                  {"sensor-capability-salinity", no_argument, NULL, 0},
	                                  {"sensor-capability-soundspeed", no_argument, NULL, 0},
	                                  {"sensor-capability-gravity", no_argument, NULL, 0},
	                                  {"sensor-capability-topography-echosounder", no_argument, NULL, 0},
	                                  {"sensor-capability-topography-interferometry", no_argument, NULL, 0},
	                                  {"sensor-capability-topography-sass", no_argument, NULL, 0},
	                                  {"sensor-capability-topography-multibeam", no_argument, NULL, 0},
	                                  {"sensor-capability-topography-photogrammetry", no_argument, NULL, 0},
	                                  {"sensor-capability-topography-structurefrommotion", no_argument, NULL, 0},
	                                  {"sensor-capability-topography-lidar", no_argument, NULL, 0},
	                                  {"sensor-capability-topography-structuredlight", no_argument, NULL, 0},
	                                  {"sensor-capability-topography-laserscanner", no_argument, NULL, 0},
	                                  {"sensor-capability-backscatter-echosounder", no_argument, NULL, 0},
	                                  {"sensor-capability-backscatter-sidescan", no_argument, NULL, 0},
	                                  {"sensor-capability-backscatter-interferometry", no_argument, NULL, 0},
	                                  {"sensor-capability-backscatter-sass", no_argument, NULL, 0},
	                                  {"sensor-capability-backscatter-multibeam", no_argument, NULL, 0},
	                                  {"sensor-capability-backscatter-lidar", no_argument, NULL, 0},
	                                  {"sensor-capability-backscatter-structuredlight", no_argument, NULL, 0},
	                                  {"sensor-capability-backscatter-laserscanner", no_argument, NULL, 0},
	                                  {"sensor-capability-photography", no_argument, NULL, 0},
	                                  {"sensor-capability-stereophotography", no_argument, NULL, 0},
	                                  {"sensor-capability-video", no_argument, NULL, 0},
	                                  {"sensor-capability-stereovideo", no_argument, NULL, 0},
	                                  {"sensor-capability1", required_argument, NULL, 0},
	                                  {"sensor-capability2", required_argument, NULL, 0},
	                                  {"sensor-offsets", required_argument, NULL, 0},
	                                  {"sensor-offset-positions", required_argument, NULL, 0},
	                                  {"sensor-offset-angles", required_argument, NULL, 0},
	                                  {"sensor-time-latency", required_argument, NULL, 0},
	                                  {"sensor-time-latency-model", required_argument, NULL, 0},
	                                  {"sensor-source-bathymetry", no_argument, NULL, 0},
	                                  {"sensor-source-bathymetry1", no_argument, NULL, 0},
	                                  {"sensor-source-bathymetry2", no_argument, NULL, 0},
	                                  {"sensor-source-bathymetry3", no_argument, NULL, 0},
	                                  {"sensor-source-backscatter", no_argument, NULL, 0},
	                                  {"sensor-source-backscatter1", no_argument, NULL, 0},
	                                  {"sensor-source-backscatter2", no_argument, NULL, 0},
	                                  {"sensor-source-backscatter3", no_argument, NULL, 0},
	                                  {"sensor-source-subbottom", no_argument, NULL, 0},
	                                  {"sensor-source-subbottom1", no_argument, NULL, 0},
	                                  {"sensor-source-subbottom2", no_argument, NULL, 0},
	                                  {"sensor-source-subbottom3", no_argument, NULL, 0},
	                                  {"sensor-source-position", no_argument, NULL, 0},
	                                  {"sensor-source-position1", no_argument, NULL, 0},
	                                  {"sensor-source-position2", no_argument, NULL, 0},
	                                  {"sensor-source-position3", no_argument, NULL, 0},
	                                  {"sensor-source-depth", no_argument, NULL, 0},
	                                  {"sensor-source-depth1", no_argument, NULL, 0},
	                                  {"sensor-source-depth2", no_argument, NULL, 0},
	                                  {"sensor-source-depth3", no_argument, NULL, 0},
	                                  {"sensor-source-heading", no_argument, NULL, 0},
	                                  {"sensor-source-heading1", no_argument, NULL, 0},
	                                  {"sensor-source-heading2", no_argument, NULL, 0},
	                                  {"sensor-source-heading3", no_argument, NULL, 0},
	                                  {"sensor-source-rollpitch", no_argument, NULL, 0},
	                                  {"sensor-source-rollpitch1", no_argument, NULL, 0},
	                                  {"sensor-source-rollpitch2", no_argument, NULL, 0},
	                                  {"sensor-source-rollpitch3", no_argument, NULL, 0},
	                                  {"sensor-source-heave", no_argument, NULL, 0},
	                                  {"sensor-source-heave1", no_argument, NULL, 0},
	                                  {"sensor-source-heave2", no_argument, NULL, 0},
	                                  {"sensor-source-heave3", no_argument, NULL, 0},
	                                  {"modify-offsets", required_argument, NULL, 0},
	                                  {"modify-offset-positions", required_argument, NULL, 0},
	                                  {"modify-offset-angles", required_argument, NULL, 0},
	                                  {"modify-time-latency", required_argument, NULL, 0},
	                                  {"modify-time-latency-model", required_argument, NULL, 0},
	                                  {"end-sensor", no_argument, NULL, 0},
	                                  {NULL, 0, NULL, 0}};

	/* get current default values */
	int status = mb_defaults(verbose, &input_swath_format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);
	input_swath_format = 0;
	pings = 1;
	bounds[0] = -360.0;
	bounds[1] = 360.0;
	bounds[2] = -90.0;
	bounds[3] = 90.0;

	/* initialize platform structure */
	status = mb_platform_init(verbose, (void **)&platform, &error);

	/* initialize tmp_sensor and tmp_offsets */
	memset(&tmp_sensor, 0, sizeof(struct mb_sensor_struct));
	memset(tmp_offsets, 0, 4 * sizeof(struct mb_sensor_offset_struct));

	/* process argument list - for this program all the action
	    happens in this loop and the order of arguments matters
	    - the input and output arguments must be given first */
	while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1) {
		switch (c) {
		/* long options all return c=0 */
		case 0:
			/* verbose */
			if (strcmp("verbose", options[option_index].name) == 0) {
				verbose++;

				if (verbose == 1) {
					fprintf(stderr, "\nProgram %s\n", program_name);
					fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
				}
			}

			/* help */
			else if (strcmp("help", options[option_index].name) == 0) {
				fprintf(stderr, "\n%s\n", help_message);
				fprintf(stderr, "\nusage: %s\n", usage_message);
				exit(error);
			}

			/*-------------------------------------------------------
			 * Define input platform file to be modified */

			/* input platform file */
			else if (strcmp("input", options[option_index].name) == 0) {
				/* set the name of the input platform file */
				strcpy(input_platform_file, optarg);

				/* read the pre-existing platform file */
				status = mb_platform_read(verbose, input_platform_file, (void **)&platform, &error);
				if (status == MB_FAILURE) {
					error = MB_ERROR_OPEN_FAIL;
					fprintf(stderr, "\nUnable to read the pre-existing platform file: %s\n", input_platform_file);
					fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
					exit(error);
				}
				platform_num_sensors = platform->num_sensors;

				if (verbose > 0) {
					fprintf(stderr, "\nRead existing platform file <%s>\n", input_platform_file);
					fprintf(stderr, "    platform->type:                        %d <%s>\n", platform->type,
					        mb_platform_type(platform->type));
					fprintf(stderr, "    platform->name:                        %s\n", platform->name);
					fprintf(stderr, "    platform->organization:                %s\n", platform->organization);
					fprintf(stderr, "    platform->documentation_url:           %s\n", platform->documentation_url);
					fprintf(stderr, "    platform->start_time_d:                %f\n", platform->start_time_d);
					fprintf(stderr, "    platform->start_time_i:                %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
					        platform->start_time_i[0], platform->start_time_i[1], platform->start_time_i[2],
					        platform->start_time_i[3], platform->start_time_i[4], platform->start_time_i[5],
					        platform->start_time_i[6]);
					fprintf(stderr, "    platform->end_time_d:                  %f\n", platform->end_time_d);
					fprintf(stderr, "    platform->end_time_i:                %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
					        platform->end_time_i[0], platform->end_time_i[1], platform->end_time_i[2], platform->end_time_i[3],
					        platform->end_time_i[4], platform->end_time_i[5], platform->end_time_i[6]);
					fprintf(stderr, "    platform->source_bathymetry:           %d\n", platform->source_bathymetry);
					fprintf(stderr, "    platform->source_bathymetry1:          %d\n", platform->source_bathymetry1);
					fprintf(stderr, "    platform->source_bathymetry2:          %d\n", platform->source_bathymetry2);
					fprintf(stderr, "    platform->source_bathymetry3:          %d\n", platform->source_bathymetry3);
					fprintf(stderr, "    platform->source_backscatter:          %d\n", platform->source_backscatter);
					fprintf(stderr, "    platform->source_backscatter1:         %d\n", platform->source_backscatter1);
					fprintf(stderr, "    platform->source_backscatter2:         %d\n", platform->source_backscatter2);
					fprintf(stderr, "    platform->source_backscatter3:         %d\n", platform->source_backscatter3);
					fprintf(stderr, "    platform->source_subbottom:            %d\n", platform->source_subbottom);
					fprintf(stderr, "    platform->source_subbottom1:           %d\n", platform->source_subbottom1);
					fprintf(stderr, "    platform->source_subbottom2:           %d\n", platform->source_subbottom2);
					fprintf(stderr, "    platform->source_subbottom3:           %d\n", platform->source_subbottom3);
					fprintf(stderr, "    platform->source_position:             %d\n", platform->source_position);
					fprintf(stderr, "    platform->source_position1:            %d\n", platform->source_position1);
					fprintf(stderr, "    platform->source_position2:            %d\n", platform->source_position2);
					fprintf(stderr, "    platform->source_position3:            %d\n", platform->source_position3);
					fprintf(stderr, "    platform->source_depth:                %d\n", platform->source_depth);
					fprintf(stderr, "    platform->source_depth1:               %d\n", platform->source_depth1);
					fprintf(stderr, "    platform->source_depth2:               %d\n", platform->source_depth2);
					fprintf(stderr, "    platform->source_depth3:               %d\n", platform->source_depth3);
					fprintf(stderr, "    platform->source_heading:              %d\n", platform->source_heading);
					fprintf(stderr, "    platform->source_heading1:             %d\n", platform->source_heading1);
					fprintf(stderr, "    platform->source_heading2:             %d\n", platform->source_heading2);
					fprintf(stderr, "    platform->source_heading3:             %d\n", platform->source_heading3);
					fprintf(stderr, "    platform->source_rollpitch:            %d\n", platform->source_rollpitch);
					fprintf(stderr, "    platform->source_rollpitch1:           %d\n", platform->source_rollpitch1);
					fprintf(stderr, "    platform->source_rollpitch2:           %d\n", platform->source_rollpitch2);
					fprintf(stderr, "    platform->source_rollpitch3:           %d\n", platform->source_rollpitch3);
					fprintf(stderr, "    platform->source_heave:                %d\n", platform->source_heave);
					fprintf(stderr, "    platform->source_heave1:               %d\n", platform->source_heave1);
					fprintf(stderr, "    platform->source_heave2:               %d\n", platform->source_heave2);
					fprintf(stderr, "    platform->source_heave3:               %d\n", platform->source_heave3);
					fprintf(stderr, "    platform->num_sensors:                 %d\n", platform->num_sensors);
					for (int i = 0; i < platform->num_sensors; i++) {
						index = 0;
						for (j = 0; j < NUM_MB_SENSOR_TYPES; j++)
							if (mb_sensor_type_id[j] == platform->sensors[i].type)
								index = j;
						fprintf(stderr, "    platform->sensors[%d].type:                 %d <%s>\n", i, platform->sensors[i].type,
						        mb_sensor_type_string[index]);
						fprintf(stderr, "    platform->sensors[%d].model:                %s\n", i, platform->sensors[i].model);
						fprintf(stderr, "    platform->sensors[%d].manufacturer:         %s\n", i,
						        platform->sensors[i].manufacturer);
						fprintf(stderr, "    platform->sensors[%d].serialnumber:         %s\n", i,
						        platform->sensors[i].serialnumber);
						fprintf(stderr, "    platform->sensors[%d].capability1:          %d\n", i,
						        platform->sensors[i].capability1);
						fprintf(stderr, "    platform->sensors[%d].capability2:          %d\n", i,
						        platform->sensors[i].capability2);
						fprintf(stderr, "    platform->sensors[%d].num_offsets:          %d\n", i,
						        platform->sensors[i].num_offsets);
						for (j = 0; j < platform->sensors[i].num_offsets; j++) {
							fprintf(stderr, "    platform->sensors[%d].offsets[%d].position_offset_mode:       %d\n", i, j,
							        platform->sensors[i].offsets[j].position_offset_mode);
							fprintf(stderr, "    platform->sensors[%d].offsets[%d].position_offset_x:          %f\n", i, j,
							        platform->sensors[i].offsets[j].position_offset_x);
							fprintf(stderr, "    platform->sensors[%d].offsets[%d].position_offset_y:          %f\n", i, j,
							        platform->sensors[i].offsets[j].position_offset_y);
							fprintf(stderr, "    platform->sensors[%d].offsets[%d].position_offset_z:          %f\n", i, j,
							        platform->sensors[i].offsets[j].position_offset_z);
							fprintf(stderr, "    platform->sensors[%d].offsets[%d].attitude_offset_mode:       %d\n", i, j,
							        platform->sensors[i].offsets[j].attitude_offset_mode);
							fprintf(stderr, "    platform->sensors[%d].offsets[%d].attitude_offset_heading:    %f\n", i, j,
							        platform->sensors[i].offsets[j].attitude_offset_heading);
							fprintf(stderr, "    platform->sensors[%d].offsets[%d].attitude_offset_roll:       %f\n", i, j,
							        platform->sensors[i].offsets[j].attitude_offset_roll);
							fprintf(stderr, "    platform->sensors[%d].offsets[%d].attitude_offset_pitch:      %f\n", i, j,
							        platform->sensors[i].offsets[j].attitude_offset_pitch);
						}
						fprintf(stderr, "    platform->sensors[%d].time_latency_mode:    %d\n", i,
						        platform->sensors[i].time_latency_mode);
						fprintf(stderr, "    platform->sensors[%d].time_latency_static:  %f\n", i,
						        platform->sensors[i].time_latency_static);
						fprintf(stderr, "    platform->sensors[%d].num_time_latency:     %d\n", i,
						        platform->sensors[i].num_time_latency);
						for (j = 0; j < platform->sensors[i].num_time_latency; j++) {
							fprintf(stderr, "    platform->sensors[%d].time_latency[%d]:                       %16.6f %8.6f\n", i,
							        j, platform->sensors[i].time_latency_time_d[j], platform->sensors[i].time_latency_value[j]);
						}
					}
				}
			}

			/*-------------------------------------------------------
			 * Define input swath data from which to extract an initial platform model */

			/* input swath file or datalist */
			else if (strcmp("swath", options[option_index].name) == 0) {
				/* set the name of the input platform file */
				strcpy(input_swath_file, optarg);

				/* get format if required */
				if (input_swath_format == 0)
					mb_get_format(verbose, input_swath_file, NULL, &input_swath_format, &error);

				/* open datalist or single swath file */
				if (input_swath_format < 0) {
					if ((status = mb_datalist_open(verbose, &datalist, input_swath_file, look_processed, &error)) != MB_SUCCESS) {
						error = MB_ERROR_OPEN_FAIL;
						fprintf(stderr, "\nUnable to open data list file: %s\n", input_swath_file);
						fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
						exit(error);
					}
					if ((status = mb_datalist_read(verbose, datalist, swath_file, dfile, &input_swath_format, &file_weight,
					                               &error)) == MB_SUCCESS)
						read_data = MB_YES;
					else
						read_data = MB_NO;
				}
				else {
					strcpy(swath_file, input_swath_file);
					read_data = MB_YES;
				}

				/* loop over all files to be read */
				while (read_data == MB_YES && input_swath_platform_defined == MB_NO) {
					/* check format and get data sources */
					if ((status =
					         mb_format_source(verbose, &input_swath_format, &platform_source, &nav_source, &sensordepth_source,
					                          &heading_source, &attitude_source, &svp_source, &error)) == MB_FAILURE) {
						mb_error(verbose, error, &message);
						fprintf(stderr, "\nMBIO Error returned from function <mb_format_source>:\n%s\n", message);
						fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
						exit(error);
					}

					/* initialize reading the swath file */
					if ((status = mb_read_init(verbose, swath_file, input_swath_format, pings, lonflip, bounds, btime_i, etime_i,
					                           speedmin, timegap, &mbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp,
					                           &pixels_ss, &error)) != MB_SUCCESS) {
						mb_error(verbose, error, &message);
						fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
						fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", swath_file);
						fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
						exit(error);
					}

					/* get store_ptr */
					mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
					store_ptr = (void *)mb_io_ptr->store_data;

					/* read data */
					// fprintf(stderr,"Look for platform data in file? error:%d input_swath_platform_defined:%d\n",
					// error,input_swath_platform_defined);
					while (error <= MB_ERROR_NO_ERROR && input_swath_platform_defined == MB_NO) {
						status = mb_read_ping(verbose, mbio_ptr, store_ptr, &kind, &error);

						/* if platform_source kind then extract platform definition */
						// fprintf(stderr,"error:%d kind:%d platform_source:%d YES/NO:%d\n",
						// error,kind,platform_source,
						//(error <= MB_ERROR_NO_ERROR && kind == platform_source && platform_source != MB_DATA_NONE));
						if (error <= MB_ERROR_NO_ERROR && kind == platform_source && platform_source != MB_DATA_NONE) {
							/* extract platform */
							status = mb_extract_platform(verbose, mbio_ptr, store_ptr, &kind, (void **)&platform, &error);

							/* note success */
							if (status == MB_SUCCESS && platform != NULL) {
								input_swath_platform_defined = MB_YES;
								platform_num_sensors = platform->num_sensors;
							}
						}
					}

					/* close the swath file */
					status = mb_close(verbose, &mbio_ptr, &error);

					/* figure out whether and what to read next */
					if (read_datalist == MB_YES) {
						if ((status = mb_datalist_read(verbose, datalist, swath_file, dfile, &input_swath_format, &file_weight,
						                               &error)) == MB_SUCCESS)
							read_data = MB_YES;
						else
							read_data = MB_NO;
					}
					else {
						read_data = MB_NO;
					}
				}

				if (read_datalist == MB_YES)
					mb_datalist_close(verbose, &datalist, &error);

				if (verbose > 0 && input_swath_platform_defined == MB_YES) {
					fprintf(stderr, "\nExtracted platform from swath data <%s>\n", input_swath_file);
					fprintf(stderr, "    platform->type:                        %d <%s>\n", platform->type,
					        mb_platform_type(platform->type));
					fprintf(stderr, "    platform->name:                        %s\n", platform->name);
					fprintf(stderr, "    platform->organization:                %s\n", platform->organization);
					fprintf(stderr, "    platform->documentation_url:           %s\n", platform->documentation_url);
					fprintf(stderr, "    platform->start_time_d:                %f\n", platform->start_time_d);
					fprintf(stderr, "    platform->start_time_i:                %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
					        platform->start_time_i[0], platform->start_time_i[1], platform->start_time_i[2],
					        platform->start_time_i[3], platform->start_time_i[4], platform->start_time_i[5],
					        platform->start_time_i[6]);
					fprintf(stderr, "    platform->end_time_d:                  %f\n", platform->end_time_d);
					fprintf(stderr, "    platform->end_time_i:                %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
					        platform->end_time_i[0], platform->end_time_i[1], platform->end_time_i[2], platform->end_time_i[3],
					        platform->end_time_i[4], platform->end_time_i[5], platform->end_time_i[6]);
					fprintf(stderr, "    platform->source_bathymetry:           %d\n", platform->source_bathymetry);
					fprintf(stderr, "    platform->source_bathymetry1:          %d\n", platform->source_bathymetry1);
					fprintf(stderr, "    platform->source_bathymetry2:          %d\n", platform->source_bathymetry2);
					fprintf(stderr, "    platform->source_bathymetry3:          %d\n", platform->source_bathymetry3);
					fprintf(stderr, "    platform->source_backscatter:          %d\n", platform->source_backscatter);
					fprintf(stderr, "    platform->source_backscatter1:         %d\n", platform->source_backscatter1);
					fprintf(stderr, "    platform->source_backscatter2:         %d\n", platform->source_backscatter2);
					fprintf(stderr, "    platform->source_backscatter3:         %d\n", platform->source_backscatter3);
					fprintf(stderr, "    platform->source_subbottom:            %d\n", platform->source_subbottom);
					fprintf(stderr, "    platform->source_subbottom1:           %d\n", platform->source_subbottom1);
					fprintf(stderr, "    platform->source_subbottom2:           %d\n", platform->source_subbottom2);
					fprintf(stderr, "    platform->source_subbottom3:           %d\n", platform->source_subbottom3);
					fprintf(stderr, "    platform->source_position:             %d\n", platform->source_position);
					fprintf(stderr, "    platform->source_position1:            %d\n", platform->source_position1);
					fprintf(stderr, "    platform->source_position2:            %d\n", platform->source_position2);
					fprintf(stderr, "    platform->source_position3:            %d\n", platform->source_position3);
					fprintf(stderr, "    platform->source_depth:                %d\n", platform->source_depth);
					fprintf(stderr, "    platform->source_depth1:               %d\n", platform->source_depth1);
					fprintf(stderr, "    platform->source_depth2:               %d\n", platform->source_depth2);
					fprintf(stderr, "    platform->source_depth3:               %d\n", platform->source_depth3);
					fprintf(stderr, "    platform->source_heading:              %d\n", platform->source_heading);
					fprintf(stderr, "    platform->source_heading1:             %d\n", platform->source_heading1);
					fprintf(stderr, "    platform->source_heading2:             %d\n", platform->source_heading2);
					fprintf(stderr, "    platform->source_heading3:             %d\n", platform->source_heading3);
					fprintf(stderr, "    platform->source_rollpitch:            %d\n", platform->source_rollpitch);
					fprintf(stderr, "    platform->source_rollpitch1:           %d\n", platform->source_rollpitch1);
					fprintf(stderr, "    platform->source_rollpitch2:           %d\n", platform->source_rollpitch2);
					fprintf(stderr, "    platform->source_rollpitch3:           %d\n", platform->source_rollpitch3);
					fprintf(stderr, "    platform->source_heave:                %d\n", platform->source_heave);
					fprintf(stderr, "    platform->source_heave1:               %d\n", platform->source_heave1);
					fprintf(stderr, "    platform->source_heave2:               %d\n", platform->source_heave2);
					fprintf(stderr, "    platform->source_heave3:               %d\n", platform->source_heave3);
					fprintf(stderr, "    platform->num_sensors:                 %d\n", platform->num_sensors);
					for (int i = 0; i < platform->num_sensors; i++) {
						index = 0;
						for (j = 0; j < NUM_MB_SENSOR_TYPES; j++)
							if (mb_sensor_type_id[j] == platform->sensors[i].type)
								index = j;
						fprintf(stderr, "    platform->sensors[%d].type:                 %d <%s>\n", i, platform->sensors[i].type,
						        mb_sensor_type_string[index]);
						fprintf(stderr, "    platform->sensors[%d].model:                %s\n", i, platform->sensors[i].model);
						fprintf(stderr, "    platform->sensors[%d].manufacturer:         %s\n", i,
						        platform->sensors[i].manufacturer);
						fprintf(stderr, "    platform->sensors[%d].serialnumber:         %s\n", i,
						        platform->sensors[i].serialnumber);
						fprintf(stderr, "    platform->sensors[%d].capability1:          %d\n", i,
						        platform->sensors[i].capability1);
						fprintf(stderr, "    platform->sensors[%d].capability2:          %d\n", i,
						        platform->sensors[i].capability2);
						fprintf(stderr, "    platform->sensors[%d].num_offsets:          %d\n", i,
						        platform->sensors[i].num_offsets);
						for (j = 0; j < platform->sensors[i].num_offsets; j++) {
							fprintf(stderr, "    platform->sensors[%d].offsets[%d].position_offset_mode:       %d\n", i, j,
							        platform->sensors[i].offsets[j].position_offset_mode);
							fprintf(stderr, "    platform->sensors[%d].offsets[%d].position_offset_x:          %f\n", i, j,
							        platform->sensors[i].offsets[j].position_offset_x);
							fprintf(stderr, "    platform->sensors[%d].offsets[%d].position_offset_y:          %f\n", i, j,
							        platform->sensors[i].offsets[j].position_offset_y);
							fprintf(stderr, "    platform->sensors[%d].offsets[%d].position_offset_z:          %f\n", i, j,
							        platform->sensors[i].offsets[j].position_offset_z);
							fprintf(stderr, "    platform->sensors[%d].offsets[%d].attitude_offset_mode:       %d\n", i, j,
							        platform->sensors[i].offsets[j].attitude_offset_mode);
							fprintf(stderr, "    platform->sensors[%d].offsets[%d].attitude_offset_heading:    %f\n", i, j,
							        platform->sensors[i].offsets[j].attitude_offset_heading);
							fprintf(stderr, "    platform->sensors[%d].offsets[%d].attitude_offset_roll:       %f\n", i, j,
							        platform->sensors[i].offsets[j].attitude_offset_roll);
							fprintf(stderr, "    platform->sensors[%d].offsets[%d].attitude_offset_pitch:      %f\n", i, j,
							        platform->sensors[i].offsets[j].attitude_offset_pitch);
						}
						fprintf(stderr, "    platform->sensors[%d].time_latency_mode:    %d\n", i,
						        platform->sensors[i].time_latency_mode);
						fprintf(stderr, "    platform->sensors[%d].time_latency_static:  %f\n", i,
						        platform->sensors[i].time_latency_static);
						fprintf(stderr, "    platform->sensors[%d].num_time_latency:     %d\n", i,
						        platform->sensors[i].num_time_latency);
						for (j = 0; j < platform->sensors[i].num_time_latency; j++) {
							fprintf(stderr, "    platform->sensors[%d].time_latency[%d]:                       %16.6f %8.6f\n", i,
							        j, platform->sensors[i].time_latency_time_d[j], platform->sensors[i].time_latency_value[j]);
						}
					}
				}
			}

			/* input swath format */
			else if (strcmp("swath-format", options[option_index].name) == 0) {
				/* set the swath format */
				sscanf(optarg, "%d", &input_swath_format);
			}

			/*-------------------------------------------------------
			 * Define output platform file to be created or modified */

			/* output platform file */
			else if (strcmp("output", options[option_index].name) == 0) {
				/* set output platform file */
				strcpy(output_platform_file, optarg);
				output_platform_file_defined = MB_YES;
			}

			/*-------------------------------------------------------
			 * Set platform type */

			/* platform-type-surface-vessel */
			else if (strcmp("platform-type-surface-vessel", options[option_index].name) == 0) {
				platform->type = MB_PLATFORM_SURFACE_VESSEL;
			}

			/* platform-type-surface-vessel */
			else if (strcmp("platform-type-tow-body", options[option_index].name) == 0) {
				platform->type = MB_PLATFORM_TOW_BODY;
			}

			/* platform-type-surface-vessel */
			else if (strcmp("platform-type-rov", options[option_index].name) == 0) {
				platform->type = MB_PLATFORM_ROV;
			}

			/* platform-type-surface-vessel */
			else if (strcmp("platform-type-auv", options[option_index].name) == 0) {
				platform->type = MB_PLATFORM_AUV;
			}

			/* platform-type-surface-vessel */
			else if (strcmp("platform-type-aircraft", options[option_index].name) == 0) {
				platform->type = MB_PLATFORM_AIRCRAFT;
			}

			/* platform-type-surface-vessel */
			else if (strcmp("platform-type-satellite", options[option_index].name) == 0) {
				platform->type = MB_PLATFORM_SATELLITE;
			}

			/*-------------------------------------------------------
			 * Set platform name and organization */

			/* platform-name */
			else if (strcmp("platform-name", options[option_index].name) == 0) {
				strcpy(platform->name, optarg);
			}

			/* platform-organization */
			else if (strcmp("platform-organization", options[option_index].name) == 0) {
				strcpy(platform->organization, optarg);
			}

			/* platform-documentation-url */
			else if (strcmp("platform-documentation-url", options[option_index].name) == 0) {
				strcpy(platform->documentation_url, optarg);
			}

			/* platform-start-time */
			else if (strcmp("platform-start-time", options[option_index].name) == 0) {
				sscanf(optarg, "%d/%d/%d %d:%d:%lf", &platform->start_time_i[0], &platform->start_time_i[1],
				       &platform->start_time_i[2], &platform->start_time_i[3], &platform->start_time_i[4], &seconds);
				platform->start_time_i[5] = (int)floor(seconds);
				platform->start_time_i[6] = (int)(1000000 * (seconds - floor(seconds)));
				mb_get_time(verbose, platform->start_time_i, &platform->start_time_d);
			}

			/* platform-end-time */
			else if (strcmp("platform-end-time", options[option_index].name) == 0) {
				sscanf(optarg, "%d/%d/%d %d:%d:%lf", &platform->end_time_i[0], &platform->end_time_i[1], &platform->end_time_i[2],
				       &platform->end_time_i[3], &platform->end_time_i[4], &seconds);
				platform->end_time_i[5] = (int)floor(seconds);
				platform->end_time_i[6] = (int)(1000000 * (seconds - floor(seconds)));
				mb_get_time(verbose, platform->end_time_i, &platform->end_time_d);
			}

			/*-------------------------------------------------------
			 * Start sensor */

			/* add-sensor-sonar-echosounder */
			else if (strcmp("add-sensor-sonar-echosounder", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_OFF) {
					sensor_mode = SENSOR_ADD;
					active_sensor = &tmp_sensor;
					memset(&tmp_sensor, 0, sizeof(struct mb_sensor_struct));
					memset(tmp_offsets, 0, 4 * sizeof(struct mb_sensor_offset_struct));
					tmp_sensor.type = MB_SENSOR_TYPE_SONAR_ECHOSOUNDER;
					sensor_id = platform_num_sensors;
					platform_num_sensors++;
				}
			}

			/* add-sensor-sonar-multiechosounder */
			else if (strcmp("add-sensor-sonar-multiechosounder", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_OFF) {
					sensor_mode = SENSOR_ADD;
					active_sensor = &tmp_sensor;
					memset(&tmp_sensor, 0, sizeof(struct mb_sensor_struct));
					memset(tmp_offsets, 0, 4 * sizeof(struct mb_sensor_offset_struct));
					tmp_sensor.type = MB_SENSOR_TYPE_SONAR_MULTIECHOSOUNDER;
					sensor_id = platform_num_sensors;
					platform_num_sensors++;
				}
			}

			/* add-sensor-sonar-sidescan */
			else if (strcmp("add-sensor-sonar-sidescan", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_OFF) {
					sensor_mode = SENSOR_ADD;
					active_sensor = &tmp_sensor;
					memset(&tmp_sensor, 0, sizeof(struct mb_sensor_struct));
					memset(tmp_offsets, 0, 4 * sizeof(struct mb_sensor_offset_struct));
					tmp_sensor.type = MB_SENSOR_TYPE_SONAR_SIDESCAN;
					sensor_id = platform_num_sensors;
					platform_num_sensors++;
				}
			}

			/* add-sensor-sonar-interferometry */
			else if (strcmp("add-sensor-sonar-interferometry", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_OFF) {
					sensor_mode = SENSOR_ADD;
					active_sensor = &tmp_sensor;
					memset(&tmp_sensor, 0, sizeof(struct mb_sensor_struct));
					memset(tmp_offsets, 0, 4 * sizeof(struct mb_sensor_offset_struct));
					tmp_sensor.type = MB_SENSOR_TYPE_SONAR_INTERFEROMETRY;
					sensor_id = platform_num_sensors;
					platform_num_sensors++;
				}
			}

			/* add-sensor-sonar-multibeam */
			else if (strcmp("add-sensor-sonar-multibeam", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_OFF) {
					sensor_mode = SENSOR_ADD;
					active_sensor = &tmp_sensor;
					memset(&tmp_sensor, 0, sizeof(struct mb_sensor_struct));
					memset(tmp_offsets, 0, 4 * sizeof(struct mb_sensor_offset_struct));
					tmp_sensor.type = MB_SENSOR_TYPE_SONAR_MULTIBEAM;
					sensor_id = platform_num_sensors;
					platform_num_sensors++;
				}
			}

			/* add-sensor-sonar-multibeam-twohead */
			else if (strcmp("add-sensor-sonar-multibeam-twohead", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_OFF) {
					sensor_mode = SENSOR_ADD;
					active_sensor = &tmp_sensor;
					memset(&tmp_sensor, 0, sizeof(struct mb_sensor_struct));
					memset(tmp_offsets, 0, 4 * sizeof(struct mb_sensor_offset_struct));
					tmp_sensor.type = MB_SENSOR_TYPE_SONAR_MULTIBEAM_TWOHEAD;
					sensor_id = platform_num_sensors;
					platform_num_sensors++;
				}
			}

			/* add-sensor-sonar-subbottom */
			else if (strcmp("add-sensor-sonar-subbottom", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_OFF) {
					sensor_mode = SENSOR_ADD;
					active_sensor = &tmp_sensor;
					memset(&tmp_sensor, 0, sizeof(struct mb_sensor_struct));
					memset(tmp_offsets, 0, 4 * sizeof(struct mb_sensor_offset_struct));
					tmp_sensor.type = MB_SENSOR_TYPE_SONAR_SUBBOTTOM;
					sensor_id = platform_num_sensors;
					platform_num_sensors++;
				}
			}

			/* add-sensor-camera-mono */
			else if (strcmp("add-sensor-camera-mono", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_OFF) {
					sensor_mode = SENSOR_ADD;
					active_sensor = &tmp_sensor;
					memset(&tmp_sensor, 0, sizeof(struct mb_sensor_struct));
					memset(tmp_offsets, 0, 4 * sizeof(struct mb_sensor_offset_struct));
					tmp_sensor.type = MB_SENSOR_TYPE_CAMERA_MONO;
					sensor_id = platform_num_sensors;
					platform_num_sensors++;
				}
			}

			/* add-sensor-camera-stereo */
			else if (strcmp("add-sensor-camera-stereo", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_OFF) {
					sensor_mode = SENSOR_ADD;
					active_sensor = &tmp_sensor;
					memset(&tmp_sensor, 0, sizeof(struct mb_sensor_struct));
					memset(tmp_offsets, 0, 4 * sizeof(struct mb_sensor_offset_struct));
					tmp_sensor.type = MB_SENSOR_TYPE_CAMERA_STEREO;
					sensor_id = platform_num_sensors;
					platform_num_sensors++;
				}
			}

			/* add-sensor-camera-video */
			else if (strcmp("add-sensor-camera-video", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_OFF) {
					sensor_mode = SENSOR_ADD;
					active_sensor = &tmp_sensor;
					memset(&tmp_sensor, 0, sizeof(struct mb_sensor_struct));
					memset(tmp_offsets, 0, 4 * sizeof(struct mb_sensor_offset_struct));
					tmp_sensor.type = MB_SENSOR_TYPE_CAMERA_VIDEO;
					sensor_id = platform_num_sensors;
					platform_num_sensors++;
				}
			}

			/* add-sensor-lidar-scan */
			else if (strcmp("add-sensor-lidar-scan", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_OFF) {
					sensor_mode = SENSOR_ADD;
					active_sensor = &tmp_sensor;
					memset(&tmp_sensor, 0, sizeof(struct mb_sensor_struct));
					memset(tmp_offsets, 0, 4 * sizeof(struct mb_sensor_offset_struct));
					tmp_sensor.type = MB_SENSOR_TYPE_LIDAR_SCAN;
					sensor_id = platform_num_sensors;
					platform_num_sensors++;
				}
			}

			/* add-sensor-lidar-swath */
			else if (strcmp("add-sensor-lidar-swath", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_OFF) {
					sensor_mode = SENSOR_ADD;
					active_sensor = &tmp_sensor;
					memset(&tmp_sensor, 0, sizeof(struct mb_sensor_struct));
					memset(tmp_offsets, 0, 4 * sizeof(struct mb_sensor_offset_struct));
					tmp_sensor.type = MB_SENSOR_TYPE_LIDAR_SWATH;
					sensor_id = platform_num_sensors;
					platform_num_sensors++;
				}
			}

			/* add-sensor-position */
			else if (strcmp("add-sensor-position", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_OFF) {
					sensor_mode = SENSOR_ADD;
					active_sensor = &tmp_sensor;
					memset(&tmp_sensor, 0, sizeof(struct mb_sensor_struct));
					memset(tmp_offsets, 0, 4 * sizeof(struct mb_sensor_offset_struct));
					tmp_sensor.type = MB_SENSOR_TYPE_POSITION;
					sensor_id = platform_num_sensors;
					platform_num_sensors++;
				}
			}

			/* add-sensor-compass */
			else if (strcmp("add-sensor-compass", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_OFF) {
					sensor_mode = SENSOR_ADD;
					active_sensor = &tmp_sensor;
					memset(&tmp_sensor, 0, sizeof(struct mb_sensor_struct));
					memset(tmp_offsets, 0, 4 * sizeof(struct mb_sensor_offset_struct));
					tmp_sensor.type = MB_SENSOR_TYPE_COMPASS;
					sensor_id = platform_num_sensors;
					platform_num_sensors++;
				}
			}

			/* add-sensor-vru */
			else if (strcmp("add-sensor-vru", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_OFF) {
					sensor_mode = SENSOR_ADD;
					active_sensor = &tmp_sensor;
					memset(&tmp_sensor, 0, sizeof(struct mb_sensor_struct));
					memset(tmp_offsets, 0, 4 * sizeof(struct mb_sensor_offset_struct));
					tmp_sensor.type = MB_SENSOR_TYPE_VRU;
					sensor_id = platform_num_sensors;
					platform_num_sensors++;
				}
			}

			/* add-sensor-imu */
			else if (strcmp("add-sensor-imu", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_OFF) {
					sensor_mode = SENSOR_ADD;
					active_sensor = &tmp_sensor;
					memset(&tmp_sensor, 0, sizeof(struct mb_sensor_struct));
					memset(tmp_offsets, 0, 4 * sizeof(struct mb_sensor_offset_struct));
					tmp_sensor.type = MB_SENSOR_TYPE_IMU;
					sensor_id = platform_num_sensors;
					platform_num_sensors++;
				}
			}

			/* add-sensor-ins */
			else if (strcmp("add-sensor-ins", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_OFF) {
					sensor_mode = SENSOR_ADD;
					active_sensor = &tmp_sensor;
					memset(&tmp_sensor, 0, sizeof(struct mb_sensor_struct));
					memset(tmp_offsets, 0, 4 * sizeof(struct mb_sensor_offset_struct));
					tmp_sensor.type = MB_SENSOR_TYPE_INS;
					sensor_id = platform_num_sensors;
					platform_num_sensors++;
				}
			}

			/* add-sensor-ins-with-pressure */
			else if (strcmp("add-sensor-ins-with-pressure", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_OFF) {
					sensor_mode = SENSOR_ADD;
					active_sensor = &tmp_sensor;
					memset(&tmp_sensor, 0, sizeof(struct mb_sensor_struct));
					memset(tmp_offsets, 0, 4 * sizeof(struct mb_sensor_offset_struct));
					tmp_sensor.type = MB_SENSOR_TYPE_INS_WITH_PRESSURE;
					sensor_id = platform_num_sensors;
					platform_num_sensors++;
				}
			}

			/* add-sensor-ctd */
			else if (strcmp("add-sensor-ctd", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_OFF) {
					sensor_mode = SENSOR_ADD;
					active_sensor = &tmp_sensor;
					memset(&tmp_sensor, 0, sizeof(struct mb_sensor_struct));
					memset(tmp_offsets, 0, 4 * sizeof(struct mb_sensor_offset_struct));
					tmp_sensor.type = MB_SENSOR_TYPE_CTD;
					sensor_id = platform_num_sensors;
					platform_num_sensors++;
				}
			}

			/* add-sensor-pressure */
			else if (strcmp("add-sensor-pressure", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_OFF) {
					sensor_mode = SENSOR_ADD;
					active_sensor = &tmp_sensor;
					memset(&tmp_sensor, 0, sizeof(struct mb_sensor_struct));
					memset(tmp_offsets, 0, 4 * sizeof(struct mb_sensor_offset_struct));
					tmp_sensor.type = MB_SENSOR_TYPE_PRESSURE;
					sensor_id = platform_num_sensors;
					platform_num_sensors++;
				}
			}

			/* add-sensor-soundspeed */
			else if (strcmp("add-sensor-soundspeed", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_OFF) {
					sensor_mode = SENSOR_ADD;
					active_sensor = &tmp_sensor;
					memset(&tmp_sensor, 0, sizeof(struct mb_sensor_struct));
					memset(tmp_offsets, 0, 4 * sizeof(struct mb_sensor_offset_struct));
					tmp_sensor.type = MB_SENSOR_TYPE_SOUNDSPEED;
					sensor_id = platform_num_sensors;
					platform_num_sensors++;
				}
			}

			/*-------------------------------------------------------
			 * Set sensor  model, organization and serialnumber */

			/* sensor-model */
			else if (strcmp("sensor-model", options[option_index].name) == 0) {
				strcpy(tmp_sensor.model, optarg);
			}

			/* sensor-manufacturer */
			else if (strcmp("sensor-manufacturer", options[option_index].name) == 0) {
				strcpy(tmp_sensor.manufacturer, optarg);
			}

			/* sensor-serialnumber */
			else if (strcmp("sensor-serialnumber", options[option_index].name) == 0) {
				strcpy(tmp_sensor.serialnumber, optarg);
			}

			/*-------------------------------------------------------
			 * Set sensor position, attitude and other data capabilities */

			/* sensor-capability-position */
			else if (strcmp("sensor-capability-position", options[option_index].name) == 0) {
				tmp_sensor.capability1 = tmp_sensor.capability1 | MB_SENSOR_CAPABILITY1_POSITION;
			}

			/* sensor-capability-depth */
			else if (strcmp("sensor-capability-depth", options[option_index].name) == 0) {
				tmp_sensor.capability1 = tmp_sensor.capability1 | MB_SENSOR_CAPABILITY1_DEPTH;
			}

			/* sensor-capability-altitude */
			else if (strcmp("sensor-capability-altitude", options[option_index].name) == 0) {
				tmp_sensor.capability1 = tmp_sensor.capability1 | MB_SENSOR_CAPABILITY1_ALTITUDE;
			}

			/* sensor-capability-velocity */
			else if (strcmp("sensor-capability-velocity", options[option_index].name) == 0) {
				tmp_sensor.capability1 = tmp_sensor.capability1 | MB_SENSOR_CAPABILITY1_VELOCITY;
			}

			/* sensor-capability-acceleration */
			else if (strcmp("sensor-capability-acceleration", options[option_index].name) == 0) {
				tmp_sensor.capability1 = tmp_sensor.capability1 | MB_SENSOR_CAPABILITY1_ACCELERATION;
			}

			/* sensor-capability-pressure */
			else if (strcmp("sensor-capability-pressure", options[option_index].name) == 0) {
				tmp_sensor.capability1 = tmp_sensor.capability1 | MB_SENSOR_CAPABILITY1_PRESSURE;
			}

			/* sensor-capability-rollpitch */
			else if (strcmp("sensor-capability-rollpitch", options[option_index].name) == 0) {
				tmp_sensor.capability1 = tmp_sensor.capability1 | MB_SENSOR_CAPABILITY1_ROLLPITCH;
			}

			/* sensor-capability-heading */
			else if (strcmp("sensor-capability-heading", options[option_index].name) == 0) {
				tmp_sensor.capability1 = tmp_sensor.capability1 | MB_SENSOR_CAPABILITY1_HEADING;
			}

			/* sensor-capability-magneticfield */
			else if (strcmp("sensor-capability-magneticfield", options[option_index].name) == 0) {
				tmp_sensor.capability1 = tmp_sensor.capability1 | MB_SENSOR_CAPABILITY1_MAGNETICFIELD;
			}

			/* sensor-capability-temperature */
			else if (strcmp("sensor-capability-temperature", options[option_index].name) == 0) {
				tmp_sensor.capability1 = tmp_sensor.capability1 | MB_SENSOR_CAPABILITY1_TEMPERATURE;
			}

			/* sensor-capability-conductivity */
			else if (strcmp("sensor-capability-conductivity", options[option_index].name) == 0) {
				tmp_sensor.capability1 = tmp_sensor.capability1 | MB_SENSOR_CAPABILITY1_CONDUCTIVITY;
			}

			/* sensor-capability-salinity */
			else if (strcmp("sensor-capability-salinity", options[option_index].name) == 0) {
				tmp_sensor.capability1 = tmp_sensor.capability1 | MB_SENSOR_CAPABILITY1_SALINITY;
			}

			/* sensor-capability-soundspeed */
			else if (strcmp("sensor-capability-soundspeed", options[option_index].name) == 0) {
				tmp_sensor.capability1 = tmp_sensor.capability1 | MB_SENSOR_CAPABILITY1_SOUNDSPEED;
			}

			/* sensor-capability-gravity */
			else if (strcmp("sensor-capability-gravity", options[option_index].name) == 0) {
				tmp_sensor.capability1 = tmp_sensor.capability1 | MB_SENSOR_CAPABILITY1_GRAVITY;
			}

			/*-------------------------------------------------------
			 * Set sensor sensor mapping capabilities */

			/* sensor-capability-topography-echosounder */
			else if (strcmp("sensor-capability-topography-echosounder", options[option_index].name) == 0) {
				tmp_sensor.capability2 = tmp_sensor.capability2 | MB_SENSOR_CAPABILITY2_TOPOGRAPHY_ECHOSOUNDER;
			}

			/* sensor-capability-topography-interferometry */
			else if (strcmp("sensor-capability-topography-interferometry", options[option_index].name) == 0) {
				tmp_sensor.capability2 = tmp_sensor.capability2 | MB_SENSOR_CAPABILITY2_TOPOGRAPHY_INTERFEROMETRY;
			}

			/* sensor-capability-topography-sass */
			else if (strcmp("sensor-capability-topography-sass", options[option_index].name) == 0) {
				tmp_sensor.capability2 = tmp_sensor.capability2 | MB_SENSOR_CAPABILITY2_TOPOGRAPHY_SASS;
			}

			/* sensor-capability-topography-multibeam */
			else if (strcmp("sensor-capability-topography-multibeam", options[option_index].name) == 0) {
				tmp_sensor.capability2 = tmp_sensor.capability2 | MB_SENSOR_CAPABILITY2_TOPOGRAPHY_MULTIBEAM;
			}

			/* sensor-capability-topography-photogrammetry */
			else if (strcmp("sensor-capability-topography-photogrammetry", options[option_index].name) == 0) {
				tmp_sensor.capability2 = tmp_sensor.capability2 | MB_SENSOR_CAPABILITY2_TOPOGRAPHY_PHOTOGRAMMETRY;
			}

			/* sensor-capability-topography-structurefrommotion */
			else if (strcmp("sensor-capability-topography-structurefrommotion", options[option_index].name) == 0) {
				tmp_sensor.capability2 = tmp_sensor.capability2 | MB_SENSOR_CAPABILITY2_TOPOGRAPHY_STRUCTUREFROMMOTION;
			}

			/* sensor-capability-topography-lidar */
			else if (strcmp("sensor-capability-topography-lidar", options[option_index].name) == 0) {
				tmp_sensor.capability2 = tmp_sensor.capability2 | MB_SENSOR_CAPABILITY2_TOPOGRAPHY_LIDAR;
			}

			/* sensor-capability-topography-structuredlight */
			else if (strcmp("sensor-capability-topography-structuredlight", options[option_index].name) == 0) {
				tmp_sensor.capability2 = tmp_sensor.capability2 | MB_SENSOR_CAPABILITY2_TOPOGRAPHY_STRUCTUREDLIGHT;
			}

			/* sensor-capability-topography-laserscanner */
			else if (strcmp("sensor-capability-topography-laserscanner", options[option_index].name) == 0) {
				tmp_sensor.capability2 = tmp_sensor.capability2 | MB_SENSOR_CAPABILITY2_TOPOGRAPHY_LASERSCANNER;
			}

			/* sensor-capability-backscatter-echosounder */
			else if (strcmp("sensor-capability-backscatter-echosounder", options[option_index].name) == 0) {
				tmp_sensor.capability2 = tmp_sensor.capability2 | MB_SENSOR_CAPABILITY2_BACKSCATTER_ECHOSOUNDER;
			}

			/* sensor-capability-backscatter-sidescan */
			else if (strcmp("sensor-capability-backscatter-sidescan", options[option_index].name) == 0) {
				tmp_sensor.capability2 = tmp_sensor.capability2 | MB_SENSOR_CAPABILITY2_BACKSCATTER_SIDESCAN;
			}

			/* sensor-capability-backscatter-interferometry */
			else if (strcmp("sensor-capability-backscatter-interferometry", options[option_index].name) == 0) {
				tmp_sensor.capability2 = tmp_sensor.capability2 | MB_SENSOR_CAPABILITY2_BACKSCATTER_INTERFEROMETRY;
			}

			/* sensor-capability-backscatter-sass */
			else if (strcmp("sensor-capability-backscatter-sass", options[option_index].name) == 0) {
				tmp_sensor.capability2 = tmp_sensor.capability2 | MB_SENSOR_CAPABILITY2_BACKSCATTER_SASS;
			}

			/* sensor-capability-backscatter-multibeam */
			else if (strcmp("sensor-capability-backscatter-multibeam", options[option_index].name) == 0) {
				tmp_sensor.capability2 = tmp_sensor.capability2 | MB_SENSOR_CAPABILITY2_BACKSCATTER_MULTIBEAM;
			}

			/* sensor-capability-backscatter-lidar */
			else if (strcmp("sensor-capability-backscatter-lidar", options[option_index].name) == 0) {
				tmp_sensor.capability2 = tmp_sensor.capability2 | MB_SENSOR_CAPABILITY2_BACKSCATTER_LIDAR;
			}

			/* sensor-capability-backscatter-structuredlight */
			else if (strcmp("sensor-capability-backscatter-structuredlight", options[option_index].name) == 0) {
				tmp_sensor.capability2 = tmp_sensor.capability2 | MB_SENSOR_CAPABILITY2_BACKSCATTER_STRUCTUREDLIGHT;
			}

			/* sensor-capability-backscatter-laserscanner */
			else if (strcmp("sensor-capability-backscatter-laserscanner", options[option_index].name) == 0) {
				tmp_sensor.capability2 = tmp_sensor.capability2 | MB_SENSOR_CAPABILITY2_BACKSCATTER_LASERSCANNER;
			}

			/* sensor-capability-photography */
			else if (strcmp("sensor-capability-photography", options[option_index].name) == 0) {
				tmp_sensor.capability2 = tmp_sensor.capability2 | MB_SENSOR_CAPABILITY2_PHOTOGRAPHY;
			}

			/* sensor-capability-stereophotography */
			else if (strcmp("sensor-capability-stereophotography", options[option_index].name) == 0) {
				tmp_sensor.capability2 = tmp_sensor.capability2 | MB_SENSOR_CAPABILITY2_STEREOPHOTOGRAPHY;
			}

			/* sensor-capability-video */
			else if (strcmp("sensor-capability-video", options[option_index].name) == 0) {
				tmp_sensor.capability2 = tmp_sensor.capability2 | MB_SENSOR_CAPABILITY2_VIDEO;
			}

			/* sensor-capability-stereovideo */
			else if (strcmp("sensor-capability-stereovideo", options[option_index].name) == 0) {
				tmp_sensor.capability2 = tmp_sensor.capability2 | MB_SENSOR_CAPABILITY2_STEREOVIDEO;
			}

			/*-------------------------------------------------------
			 * Set sensor capability bitmasks directly */

			/* sensor-capability1 */
			else if (strcmp("sensor-capability1", options[option_index].name) == 0) {
				sscanf(optarg, "%d", &tmp_sensor.capability1);
			}

			/* sensor-capability2 */
			else if (strcmp("sensor-capability2", options[option_index].name) == 0) {
				sscanf(optarg, "%d", &tmp_sensor.capability2);
			}

			/*-------------------------------------------------------
			 * Set sensor offsets */

			/* sensor-offsets */
			else if (strcmp("sensor-offsets", options[option_index].name) == 0) {
				sscanf(optarg, "%lf/%lf/%lf/%lf/%lf/%lf", &tmp_offsets[tmp_sensor.num_offsets].position_offset_x,
				       &tmp_offsets[tmp_sensor.num_offsets].position_offset_y,
				       &tmp_offsets[tmp_sensor.num_offsets].position_offset_z,
				       &tmp_offsets[tmp_sensor.num_offsets].attitude_offset_heading,
				       &tmp_offsets[tmp_sensor.num_offsets].attitude_offset_roll,
				       &tmp_offsets[tmp_sensor.num_offsets].attitude_offset_pitch);
				tmp_offsets[tmp_sensor.num_offsets].position_offset_mode = MB_YES;
				tmp_offsets[tmp_sensor.num_offsets].attitude_offset_mode = MB_YES;
				tmp_sensor.num_offsets++;
			}

			/* sensor-offset-positions */
			else if (strcmp("sensor-offset-positions", options[option_index].name) == 0) {
				sscanf(optarg, "%lf/%lf/%lf", &tmp_offsets[tmp_sensor.num_offsets].position_offset_x,
				       &tmp_offsets[tmp_sensor.num_offsets].position_offset_y,
				       &tmp_offsets[tmp_sensor.num_offsets].position_offset_z);
				tmp_offsets[tmp_sensor.num_offsets].position_offset_mode = MB_YES;
				tmp_offsets[tmp_sensor.num_offsets].attitude_offset_mode = MB_NO;
				tmp_sensor.num_offsets++;
			}

			/* sensor-offset-angles */
			else if (strcmp("sensor-offset-angles", options[option_index].name) == 0) {
				sscanf(optarg, "%lf/%lf/%lf", &tmp_offsets[tmp_sensor.num_offsets].attitude_offset_heading,
				       &tmp_offsets[tmp_sensor.num_offsets].attitude_offset_roll,
				       &tmp_offsets[tmp_sensor.num_offsets].attitude_offset_pitch);
				tmp_offsets[tmp_sensor.num_offsets].position_offset_mode = MB_NO;
				tmp_offsets[tmp_sensor.num_offsets].attitude_offset_mode = MB_YES;
				tmp_sensor.num_offsets++;
			}

			/*-------------------------------------------------------*/
			/* Set sensor time latency */

			/* sensor-time-latency */
			else if (strcmp("sensor-time-latency", options[option_index].name) == 0) {
				sscanf(optarg, "%lf", &tmp_sensor.time_latency_static);
				tmp_sensor.time_latency_mode = MB_SENSOR_TIME_LATENCY_STATIC;
			}

			/* sensor-time-latency-model */
			else if (strcmp("sensor-time-latency-model", options[option_index].name) == 0) {
				/* set the name of the input time latency file */
				strcpy(time_latency_model_file, optarg);
				tmp_sensor.time_latency_mode = MB_SENSOR_TIME_LATENCY_MODEL;

				/* count the data points in the time latency file */
				tmp_sensor.num_time_latency = 0;
				if ((tfp = fopen(time_latency_model_file, "r")) == NULL) {
					error = MB_ERROR_OPEN_FAIL;
					fprintf(stderr, "\nUnable to open time latency model file <%s> for reading\n", time_latency_model_file);
					fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
					exit(error);
				}
				while ((result = fgets(buffer, MB_PATH_MAXLINE, tfp)) == buffer)
					if (buffer[0] != '#')
						tmp_sensor.num_time_latency++;
				rewind(tfp);

				/* allocate arrays for time latency */
				if (tmp_sensor.num_time_latency > tmp_sensor.num_time_latency_alloc) {
					status = mb_mallocd(verbose, __FILE__, __LINE__, tmp_sensor.num_time_latency * sizeof(double),
					                    (void **)&tmp_sensor.time_latency_time_d, &error);
					if (error == MB_ERROR_NO_ERROR)
						status = mb_mallocd(verbose, __FILE__, __LINE__, tmp_sensor.num_time_latency * sizeof(double),
						                    (void **)&tmp_sensor.time_latency_value, &error);
					if (error != MB_ERROR_NO_ERROR) {
						mb_error(verbose, error, &message);
						fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
						fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
						exit(error);
					}
					tmp_sensor.num_time_latency_alloc = tmp_sensor.num_time_latency;
				}

				/* read the data points in the time latency file */
				tmp_sensor.num_time_latency = 0;
				while ((result = fgets(buffer, MB_PATH_MAXLINE, tfp)) == buffer) {
					if (buffer[0] != '#') {
						/* read the time and time latency pair */
						if (sscanf(buffer, "%lf %lf", &tmp_sensor.time_latency_time_d[tmp_sensor.num_time_latency],
						           &tmp_sensor.time_latency_value[tmp_sensor.num_time_latency]) == 2)
							tmp_sensor.num_time_latency++;
					}
				}
				fclose(tfp);
			}

			/*-------------------------------------------------------*/
			/* Set current sensor to be a data source for the platform */

			/* sensor-source-bathymetry */
			else if (strcmp("sensor-source-bathymetry", options[option_index].name) == 0) {
				platform->source_bathymetry = sensor_id;
			}

			/* sensor-source-bathymetry1 */
			else if (strcmp("sensor-source-bathymetry1", options[option_index].name) == 0) {
				platform->source_bathymetry1 = sensor_id;
			}

			/* sensor-source-bathymetry2 */
			else if (strcmp("sensor-source-bathymetry2", options[option_index].name) == 0) {
				platform->source_bathymetry2 = sensor_id;
			}

			/* sensor-source-bathymetry3 */
			else if (strcmp("sensor-source-bathymetry3", options[option_index].name) == 0) {
				platform->source_bathymetry3 = sensor_id;
			}

			/* sensor-source-backscatter */
			else if (strcmp("sensor-source-backscatter", options[option_index].name) == 0) {
				platform->source_backscatter = sensor_id;
			}

			/* sensor-source-backscatter1 */
			else if (strcmp("sensor-source-backscatter1", options[option_index].name) == 0) {
				platform->source_backscatter1 = sensor_id;
			}

			/* sensor-source-backscatter2 */
			else if (strcmp("sensor-source-backscatter2", options[option_index].name) == 0) {
				platform->source_backscatter2 = sensor_id;
			}

			/* sensor-source-backscatter3 */
			else if (strcmp("sensor-source-backscatter3", options[option_index].name) == 0) {
				platform->source_backscatter3 = sensor_id;
			}

			/* sensor-source-subbottom */
			else if (strcmp("sensor-source-subbottom", options[option_index].name) == 0) {
				platform->source_subbottom = sensor_id;
			}

			/* sensor-source-subbottom1 */
			else if (strcmp("sensor-source-subbottom1", options[option_index].name) == 0) {
				platform->source_subbottom1 = sensor_id;
			}

			/* sensor-source-subbottom2 */
			else if (strcmp("sensor-source-subbottom2", options[option_index].name) == 0) {
				platform->source_subbottom2 = sensor_id;
			}

			/* sensor-source-subbottom3 */
			else if (strcmp("sensor-source-subbottom3", options[option_index].name) == 0) {
				platform->source_subbottom3 = sensor_id;
			}

			/* sensor-source-position */
			else if (strcmp("sensor-source-position", options[option_index].name) == 0) {
				platform->source_position = sensor_id;
			}

			/* sensor-source-position1 */
			else if (strcmp("sensor-source-position1", options[option_index].name) == 0) {
				platform->source_position1 = sensor_id;
			}

			/* sensor-source-position2 */
			else if (strcmp("sensor-source-position2", options[option_index].name) == 0) {
				platform->source_position2 = sensor_id;
			}

			/* sensor-source-position3 */
			else if (strcmp("sensor-source-position3", options[option_index].name) == 0) {
				platform->source_position3 = sensor_id;
			}

			/* sensor-source-depth */
			else if (strcmp("sensor-source-depth", options[option_index].name) == 0) {
				platform->source_depth = sensor_id;
			}

			/* sensor-source-depth1 */
			else if (strcmp("sensor-source-depth1", options[option_index].name) == 0) {
				platform->source_depth1 = sensor_id;
			}

			/* sensor-source-depth2 */
			else if (strcmp("sensor-source-depth2", options[option_index].name) == 0) {
				platform->source_depth2 = sensor_id;
			}

			/* sensor-source-depth3 */
			else if (strcmp("sensor-source-depth3", options[option_index].name) == 0) {
				platform->source_depth3 = sensor_id;
			}

			/* sensor-source-heading */
			else if (strcmp("sensor-source-heading", options[option_index].name) == 0) {
				platform->source_heading = sensor_id;
			}

			/* sensor-source-heading1 */
			else if (strcmp("sensor-source-heading1", options[option_index].name) == 0) {
				platform->source_heading1 = sensor_id;
			}

			/* sensor-source-heading2 */
			else if (strcmp("sensor-source-heading2", options[option_index].name) == 0) {
				platform->source_heading2 = sensor_id;
			}

			/* sensor-source-heading3 */
			else if (strcmp("sensor-source-heading3", options[option_index].name) == 0) {
				platform->source_heading3 = sensor_id;
			}

			/* sensor-source-rollpitch */
			else if (strcmp("sensor-source-rollpitch", options[option_index].name) == 0) {
				platform->source_rollpitch = sensor_id;
			}

			/* sensor-source-rollpitch1 */
			else if (strcmp("sensor-source-rollpitch1", options[option_index].name) == 0) {
				platform->source_rollpitch1 = sensor_id;
			}

			/* sensor-source-rollpitch2 */
			else if (strcmp("sensor-source-rollpitch2", options[option_index].name) == 0) {
				platform->source_rollpitch2 = sensor_id;
			}

			/* sensor-source-rollpitch3 */
			else if (strcmp("sensor-source-rollpitch3", options[option_index].name) == 0) {
				platform->source_rollpitch3 = sensor_id;
			}

			/* sensor-source-heave */
			else if (strcmp("sensor-source-heave", options[option_index].name) == 0) {
				platform->source_heave = sensor_id;
			}

			/* sensor-source-heave1 */
			else if (strcmp("sensor-source-heave1", options[option_index].name) == 0) {
				platform->source_heave1 = sensor_id;
			}

			/* sensor-source-heave2 */
			else if (strcmp("sensor-source-heave2", options[option_index].name) == 0) {
				platform->source_heave2 = sensor_id;
			}

			/* sensor-source-heave3 */
			else if (strcmp("sensor-source-heave3", options[option_index].name) == 0) {
				platform->source_heave3 = sensor_id;
			}

			/*-------------------------------------------------------*/
			/* End current sensor */

			/* end-sensor */
			else if (strcmp("end-sensor", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_ADD) {
					status = mb_platform_add_sensor(verbose, (void *)platform, tmp_sensor.type, tmp_sensor.model,
					                                tmp_sensor.manufacturer, tmp_sensor.serialnumber, tmp_sensor.capability1,
					                                tmp_sensor.capability2, tmp_sensor.num_offsets, tmp_sensor.num_time_latency,
					                                &error);
					for (ioffset = 0; ioffset < tmp_sensor.num_offsets; ioffset++) {
						status = mb_platform_set_sensor_offset(
						    verbose, (void *)platform, sensor_id, ioffset, tmp_offsets[ioffset].position_offset_mode,
						    tmp_offsets[ioffset].position_offset_x, tmp_offsets[ioffset].position_offset_y,
						    tmp_offsets[ioffset].position_offset_z, tmp_offsets[ioffset].attitude_offset_mode,
						    tmp_offsets[ioffset].attitude_offset_heading, tmp_offsets[ioffset].attitude_offset_roll,
						    tmp_offsets[ioffset].attitude_offset_pitch, &error);
					}
					status = mb_platform_set_sensor_timelatency(
					    verbose, (void *)platform, sensor_id, tmp_sensor.time_latency_mode, tmp_sensor.time_latency_static,
					    tmp_sensor.num_time_latency, tmp_sensor.time_latency_time_d, tmp_sensor.time_latency_value, &error);
					sensor_mode = SENSOR_OFF;
					sensor_id = -1;
				}
			}

			/*-------------------------------------------------------*/
			/* Modify existing sensor */

			/* modify-sensor */
			else if (strcmp("modify-sensor", options[option_index].name) == 0) {
				sscanf(optarg, "%d", &sensor_id);
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-bathymetry */
			else if (strcmp("modify-sensor-bathymetry", options[option_index].name) == 0) {
				sensor_id = platform->source_bathymetry;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-bathymetry1 */
			else if (strcmp("modify-sensor-bathymetry1", options[option_index].name) == 0) {
				sensor_id = platform->source_bathymetry1;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-bathymetry2 */
			else if (strcmp("modify-sensor-bathymetry2", options[option_index].name) == 0) {
				sensor_id = platform->source_bathymetry2;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-bathymetry3 */
			else if (strcmp("modify-sensor-bathymetry3", options[option_index].name) == 0) {
				sensor_id = platform->source_bathymetry3;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-backscatter */
			else if (strcmp("modify-sensor-backscatter", options[option_index].name) == 0) {
				sensor_id = platform->source_backscatter;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-backscatter1 */
			else if (strcmp("modify-sensor-backscatter1", options[option_index].name) == 0) {
				sensor_id = platform->source_backscatter1;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-backscatter2 */
			else if (strcmp("modify-sensor-backscatter2", options[option_index].name) == 0) {
				sensor_id = platform->source_backscatter2;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-backscatter3 */
			else if (strcmp("modify-sensor-backscatter3", options[option_index].name) == 0) {
				sensor_id = platform->source_backscatter3;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-subbottom */
			else if (strcmp("modify-sensor-subbottom", options[option_index].name) == 0) {
				sensor_id = platform->source_subbottom;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-subbottom1 */
			else if (strcmp("modify-sensor-subbottom1", options[option_index].name) == 0) {
				sensor_id = platform->source_subbottom1;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-subbottom2 */
			else if (strcmp("modify-sensor-subbottom2", options[option_index].name) == 0) {
				sensor_id = platform->source_subbottom2;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-subbottom3 */
			else if (strcmp("modify-sensor-subbottom3", options[option_index].name) == 0) {
				sensor_id = platform->source_subbottom3;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-position */
			else if (strcmp("modify-sensor-position", options[option_index].name) == 0) {
				sensor_id = platform->source_position;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-position1 */
			else if (strcmp("modify-sensor-position1", options[option_index].name) == 0) {
				sensor_id = platform->source_position1;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-position2 */
			else if (strcmp("modify-sensor-position2", options[option_index].name) == 0) {
				sensor_id = platform->source_position2;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-position3 */
			else if (strcmp("modify-sensor-position3", options[option_index].name) == 0) {
				sensor_id = platform->source_position3;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-depth */
			else if (strcmp("modify-sensor-depth", options[option_index].name) == 0) {
				sensor_id = platform->source_depth;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-depth1 */
			else if (strcmp("modify-sensor-depth1", options[option_index].name) == 0) {
				sensor_id = platform->source_depth1;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-depth2 */
			else if (strcmp("modify-sensor-depth2", options[option_index].name) == 0) {
				sensor_id = platform->source_depth2;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-depth3 */
			else if (strcmp("modify-sensor-depth3", options[option_index].name) == 0) {
				sensor_id = platform->source_depth3;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-heading */
			else if (strcmp("modify-sensor-heading", options[option_index].name) == 0) {
				sensor_id = platform->source_heading;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-heading1 */
			else if (strcmp("modify-sensor-heading1", options[option_index].name) == 0) {
				sensor_id = platform->source_heading1;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-heading2 */
			else if (strcmp("modify-sensor-heading2", options[option_index].name) == 0) {
				sensor_id = platform->source_heading2;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-heading3 */
			else if (strcmp("modify-sensor-heading3", options[option_index].name) == 0) {
				sensor_id = platform->source_heading3;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-rollpitch */
			else if (strcmp("modify-sensor-rollpitch", options[option_index].name) == 0) {
				sensor_id = platform->source_rollpitch;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-rollpitch1 */
			else if (strcmp("modify-sensor-rollpitch1", options[option_index].name) == 0) {
				sensor_id = platform->source_rollpitch1;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-rollpitch2 */
			else if (strcmp("modify-sensor-rollpitch2", options[option_index].name) == 0) {
				sensor_id = platform->source_rollpitch2;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-rollpitch3 */
			else if (strcmp("modify-sensor-rollpitch3", options[option_index].name) == 0) {
				sensor_id = platform->source_rollpitch3;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-heave */
			else if (strcmp("modify-sensor-heave", options[option_index].name) == 0) {
				sensor_id = platform->source_heave;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-heave1 */
			else if (strcmp("modify-sensor-heave1", options[option_index].name) == 0) {
				sensor_id = platform->source_heave1;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-heave2 */
			else if (strcmp("modify-sensor-heave2", options[option_index].name) == 0) {
				sensor_id = platform->source_heave2;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-sensor-heave3 */
			else if (strcmp("modify-sensor-heave3", options[option_index].name) == 0) {
				sensor_id = platform->source_heave3;
				sensor_mode = SENSOR_MODIFY;
				active_sensor = &platform->sensors[sensor_id];
			}

			/* modify-offsets */
			else if (strcmp("modify-offsets", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_MODIFY || sensor_mode == SENSOR_ADD) {
					nscan = sscanf(optarg, "%d/%lf/%lf/%lf/%lf/%lf/%lf", &ioffset, &d1, &d2, &d3, &d4, &d5, &d6);
					if (nscan == 7 && ioffset >= 0 && ioffset < active_sensor->num_offsets) {
						active_sensor->offsets[ioffset].position_offset_x = d1;
						active_sensor->offsets[ioffset].position_offset_y = d2;
						active_sensor->offsets[ioffset].position_offset_z = d3;
						active_sensor->offsets[ioffset].attitude_offset_heading = d4;
						active_sensor->offsets[ioffset].attitude_offset_roll = d5;
						active_sensor->offsets[ioffset].attitude_offset_pitch = d6;
						active_sensor->offsets[ioffset].position_offset_mode = MB_YES;
						active_sensor->offsets[ioffset].attitude_offset_mode = MB_YES;
					}
				}
			}

			/* modify-offset-positions */
			else if (strcmp("modify-offset-positions", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_MODIFY || sensor_mode == SENSOR_ADD) {
					nscan = sscanf(optarg, "%d/%lf/%lf/%lf", &ioffset, &d1, &d2, &d3);
					if (nscan == 4 && ioffset >= 0 && ioffset < active_sensor->num_offsets) {
						active_sensor->offsets[ioffset].position_offset_x = d1;
						active_sensor->offsets[ioffset].position_offset_y = d2;
						active_sensor->offsets[ioffset].position_offset_z = d3;
						active_sensor->offsets[ioffset].position_offset_mode = MB_YES;
						active_sensor->offsets[ioffset].attitude_offset_mode = MB_NO;
					}
				}
			}

			/* modify-offset-angles */
			else if (strcmp("modify-offset-angles", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_MODIFY || sensor_mode == SENSOR_ADD) {
					nscan = sscanf(optarg, "%d/%lf/%lf/%lf", &ioffset, &d1, &d2, &d3);
					if (nscan == 4 && ioffset >= 0 && ioffset < active_sensor->num_offsets) {
						active_sensor->offsets[ioffset].attitude_offset_heading = d1;
						active_sensor->offsets[ioffset].attitude_offset_roll = d2;
						active_sensor->offsets[ioffset].attitude_offset_pitch = d3;
						active_sensor->offsets[ioffset].position_offset_mode = MB_NO;
						active_sensor->offsets[ioffset].attitude_offset_mode = MB_YES;
					}
				}
			}

			/* modify-time-latency */
			else if (strcmp("modify-time-latency", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_MODIFY || sensor_mode == SENSOR_ADD) {
					sscanf(optarg, "%lf", &active_sensor->time_latency_static);
					active_sensor->time_latency_mode = MB_SENSOR_TIME_LATENCY_STATIC;
				}
			}

			/* modify-time-latency-model */
			else if (strcmp("modify-time-latency-model", options[option_index].name) == 0) {
				if (sensor_mode == SENSOR_MODIFY || sensor_mode == SENSOR_ADD) {
					/* set the name of the input time latency file */
					strcpy(time_latency_model_file, optarg);
					active_sensor->time_latency_mode = MB_SENSOR_TIME_LATENCY_MODEL;

					/* count the data points in the time latency file */
					active_sensor->num_time_latency = 0;
					if ((tfp = fopen(time_latency_model_file, "r")) == NULL) {
						error = MB_ERROR_OPEN_FAIL;
						fprintf(stderr, "\nUnable to open time latency model file <%s> for reading\n", time_latency_model_file);
						fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
						exit(error);
					}
					while ((result = fgets(buffer, MB_PATH_MAXLINE, tfp)) == buffer)
						if (buffer[0] != '#')
							active_sensor->num_time_latency++;
					rewind(tfp);

					/* allocate arrays for time latency */
					if (active_sensor->num_time_latency > active_sensor->num_time_latency_alloc) {
						status = mb_mallocd(verbose, __FILE__, __LINE__, active_sensor->num_time_latency * sizeof(double),
						                    (void **)&active_sensor->time_latency_time_d, &error);
						if (error == MB_ERROR_NO_ERROR)
							status = mb_mallocd(verbose, __FILE__, __LINE__, active_sensor->num_time_latency * sizeof(double),
							                    (void **)&active_sensor->time_latency_value, &error);
						if (error != MB_ERROR_NO_ERROR) {
							mb_error(verbose, error, &message);
							fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
							fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
							exit(error);
						}
						active_sensor->num_time_latency_alloc = active_sensor->num_time_latency;
					}

					/* read the data points in the time latency file */
					active_sensor->num_time_latency = 0;
					while ((result = fgets(buffer, MB_PATH_MAXLINE, tfp)) == buffer) {
						if (buffer[0] != '#') {
							/* read the time and time latency pair */
							if (sscanf(buffer, "%lf %lf", &active_sensor->time_latency_time_d[active_sensor->num_time_latency],
							           &active_sensor->time_latency_value[active_sensor->num_time_latency]) == 2)
								active_sensor->num_time_latency++;
						}
					}
					fclose(tfp);
				}
			}

			/*-------------------------------------------------------*/

			/* end-modify */
			else if (strcmp("end-modify", options[option_index].name) == 0) {
				sensor_mode = SENSOR_OFF;
				sensor_id = -1;
			}

			break;
		case '?':
			errflg++;
		}

		if (sensor_mode == SENSOR_MODIFY || sensor_mode == SENSOR_ADD) {
			if (platform == NULL || sensor_id < 0 || sensor_id > platform->num_sensors) {
				sensor_id = -1;
				sensor_mode = SENSOR_OFF;
			}
		}
	}

	/* if error flagged then print it and exit */
	if (errflg) {
		fprintf(stderr, "usage: %s\n", usage_message);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
	}

	/* if an output has been specified but there are still not sensors in the
	 * platform, make a generic null platform with one sensor that is the source
	 * for all data, with no offsets */
	if (output_platform_file_defined == MB_YES && platform != NULL && platform->num_sensors == 0) {
		status =
		    mb_platform_add_sensor(verbose, (void *)platform, MB_SENSOR_TYPE_NONE, NULL, NULL, NULL,
		                           (MB_SENSOR_CAPABILITY1_POSITION + MB_SENSOR_CAPABILITY1_DEPTH + MB_SENSOR_CAPABILITY1_HEAVE +
		                            MB_SENSOR_CAPABILITY1_ROLLPITCH + MB_SENSOR_CAPABILITY1_HEADING),
		                           MB_SENSOR_CAPABILITY2_TOPOGRAPHY_MULTIBEAM, 1, 0, &error);
		status = mb_platform_set_sensor_offset(verbose, (void *)platform, 0, 0, MB_SENSOR_POSITION_OFFSET_STATIC, 0.0, 0.0, 0.0,
		                                       MB_SENSOR_ATTITUDE_OFFSET_STATIC, 0.0, 0.0, 0.0, &error);
	}

	/* write out the platform file */
	if (status == MB_SUCCESS && output_platform_file_defined == MB_YES) {
		status = mb_platform_write(verbose, output_platform_file, (void *)platform, &error);
	}

	if (verbose > 0) {
		fprintf(stderr, "\nOutput platform file <%s>\n", output_platform_file);
		fprintf(stderr, "    platform->type:                        %d <%s>\n", platform->type,
		        mb_platform_type(platform->type));
		fprintf(stderr, "    platform->name:                        %s\n", platform->name);
		fprintf(stderr, "    platform->organization:                %s\n", platform->organization);
		fprintf(stderr, "    platform->documentation_url:           %s\n", platform->documentation_url);
		fprintf(stderr, "    platform->start_time_d:                %f\n", platform->start_time_d);
		fprintf(stderr, "    platform->start_time_i:                %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
		        platform->start_time_i[0], platform->start_time_i[1], platform->start_time_i[2], platform->start_time_i[3],
		        platform->start_time_i[4], platform->start_time_i[5], platform->start_time_i[6]);
		fprintf(stderr, "    platform->end_time_d:                  %f\n", platform->end_time_d);
		fprintf(stderr, "    platform->end_time_i:                %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
		        platform->end_time_i[0], platform->end_time_i[1], platform->end_time_i[2], platform->end_time_i[3],
		        platform->end_time_i[4], platform->end_time_i[5], platform->end_time_i[6]);
		fprintf(stderr, "    platform->source_bathymetry:           %d\n", platform->source_bathymetry);
		fprintf(stderr, "    platform->source_bathymetry1:          %d\n", platform->source_bathymetry1);
		fprintf(stderr, "    platform->source_bathymetry2:          %d\n", platform->source_bathymetry2);
		fprintf(stderr, "    platform->source_bathymetry3:          %d\n", platform->source_bathymetry3);
		fprintf(stderr, "    platform->source_backscatter:          %d\n", platform->source_backscatter);
		fprintf(stderr, "    platform->source_backscatter1:         %d\n", platform->source_backscatter1);
		fprintf(stderr, "    platform->source_backscatter2:         %d\n", platform->source_backscatter2);
		fprintf(stderr, "    platform->source_backscatter3:         %d\n", platform->source_backscatter3);
		fprintf(stderr, "    platform->source_subbottom:            %d\n", platform->source_subbottom);
		fprintf(stderr, "    platform->source_subbottom1:           %d\n", platform->source_subbottom1);
		fprintf(stderr, "    platform->source_subbottom2:           %d\n", platform->source_subbottom2);
		fprintf(stderr, "    platform->source_subbottom3:           %d\n", platform->source_subbottom3);
		fprintf(stderr, "    platform->source_position:             %d\n", platform->source_position);
		fprintf(stderr, "    platform->source_position1:            %d\n", platform->source_position1);
		fprintf(stderr, "    platform->source_position2:            %d\n", platform->source_position2);
		fprintf(stderr, "    platform->source_position3:            %d\n", platform->source_position3);
		fprintf(stderr, "    platform->source_depth:                %d\n", platform->source_depth);
		fprintf(stderr, "    platform->source_depth1:               %d\n", platform->source_depth1);
		fprintf(stderr, "    platform->source_depth2:               %d\n", platform->source_depth2);
		fprintf(stderr, "    platform->source_depth3:               %d\n", platform->source_depth3);
		fprintf(stderr, "    platform->source_heading:              %d\n", platform->source_heading);
		fprintf(stderr, "    platform->source_heading1:             %d\n", platform->source_heading1);
		fprintf(stderr, "    platform->source_heading2:             %d\n", platform->source_heading2);
		fprintf(stderr, "    platform->source_heading3:             %d\n", platform->source_heading3);
		fprintf(stderr, "    platform->source_rollpitch:            %d\n", platform->source_rollpitch);
		fprintf(stderr, "    platform->source_rollpitch1:           %d\n", platform->source_rollpitch1);
		fprintf(stderr, "    platform->source_rollpitch2:           %d\n", platform->source_rollpitch2);
		fprintf(stderr, "    platform->source_rollpitch3:           %d\n", platform->source_rollpitch3);
		fprintf(stderr, "    platform->source_heave:                %d\n", platform->source_heave);
		fprintf(stderr, "    platform->source_heave1:               %d\n", platform->source_heave1);
		fprintf(stderr, "    platform->source_heave2:               %d\n", platform->source_heave2);
		fprintf(stderr, "    platform->source_heave3:               %d\n", platform->source_heave3);
		fprintf(stderr, "    platform->num_sensors:                 %d\n", platform->num_sensors);
		for (int i = 0; i < platform->num_sensors; i++) {
			index = 0;
			for (j = 0; j < NUM_MB_SENSOR_TYPES; j++)
				if (mb_sensor_type_id[j] == platform->sensors[i].type)
					index = j;
			fprintf(stderr, "    platform->sensors[%d].type:                 %d <%s>\n", i, platform->sensors[i].type,
			        mb_sensor_type_string[index]);
			fprintf(stderr, "    platform->sensors[%d].model:                %s\n", i, platform->sensors[i].model);
			fprintf(stderr, "    platform->sensors[%d].manufacturer:         %s\n", i, platform->sensors[i].manufacturer);
			fprintf(stderr, "    platform->sensors[%d].serialnumber:         %s\n", i, platform->sensors[i].serialnumber);
			fprintf(stderr, "    platform->sensors[%d].capability1:          %d\n", i, platform->sensors[i].capability1);
			fprintf(stderr, "    platform->sensors[%d].capability2:          %d\n", i, platform->sensors[i].capability2);
			fprintf(stderr, "    platform->sensors[%d].num_offsets:          %d\n", i, platform->sensors[i].num_offsets);
			for (j = 0; j < platform->sensors[i].num_offsets; j++) {
				fprintf(stderr, "    platform->sensors[%d].offsets[%d].position_offset_mode:       %d\n", i, j,
				        platform->sensors[i].offsets[j].position_offset_mode);
				fprintf(stderr, "    platform->sensors[%d].offsets[%d].position_offset_x:          %f\n", i, j,
				        platform->sensors[i].offsets[j].position_offset_x);
				fprintf(stderr, "    platform->sensors[%d].offsets[%d].position_offset_y:          %f\n", i, j,
				        platform->sensors[i].offsets[j].position_offset_y);
				fprintf(stderr, "    platform->sensors[%d].offsets[%d].position_offset_z:          %f\n", i, j,
				        platform->sensors[i].offsets[j].position_offset_z);
				fprintf(stderr, "    platform->sensors[%d].offsets[%d].attitude_offset_mode:       %d\n", i, j,
				        platform->sensors[i].offsets[j].attitude_offset_mode);
				fprintf(stderr, "    platform->sensors[%d].offsets[%d].attitude_offset_heading:    %f\n", i, j,
				        platform->sensors[i].offsets[j].attitude_offset_heading);
				fprintf(stderr, "    platform->sensors[%d].offsets[%d].attitude_offset_roll:       %f\n", i, j,
				        platform->sensors[i].offsets[j].attitude_offset_roll);
				fprintf(stderr, "    platform->sensors[%d].offsets[%d].attitude_offset_pitch:      %f\n", i, j,
				        platform->sensors[i].offsets[j].attitude_offset_pitch);
			}
			fprintf(stderr, "    platform->sensors[%d].time_latency_mode:    %d\n", i, platform->sensors[i].time_latency_mode);
			fprintf(stderr, "    platform->sensors[%d].time_latency_static:  %f\n", i, platform->sensors[i].time_latency_static);
			fprintf(stderr, "    platform->sensors[%d].num_time_latency:     %d\n", i, platform->sensors[i].num_time_latency);
			for (j = 0; j < platform->sensors[i].num_time_latency; j++) {
				fprintf(stderr, "    platform->sensors[%d].time_latency[%d]:                       %16.6f %8.6f\n", i, j,
				        platform->sensors[i].time_latency_time_d[j], platform->sensors[i].time_latency_value[j]);
			}
		}
	}

	/* deallocate platform structure */
	if (platform != NULL) {
		status = mb_platform_deall(verbose, (void **)&platform, &error);
	}

	if (verbose >= 4)
		status = mb_memory_list(verbose, &error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s> completed\n", program_name);
		fprintf(stderr, "dbg2  Ending status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	exit(error);
}
/*--------------------------------------------------------------------*/
