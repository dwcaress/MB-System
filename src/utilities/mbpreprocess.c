/*--------------------------------------------------------------------
 *    The MB-system:	mbpreprocess.c	1/8/2014
 *    $Id$
 *
 *    Copyright (c) 2014-2015 by
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
 * MBpreprocess handles preprocessing of swath sonar data as part of setting
 * up an MB-System processing structure for a dataset.
 *
 * This program replaces the several format-specific preprocessing programs
 * found in MB-System version 5 releases with a single program for version 6.
 *
 * Author:	D. W. Caress
 * Date:	January 8, 2014
 *
 *
 */

/* source file version string */
static char version_id[] = "$Id$";

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

/* MBIO include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_io.h"
#include "mb_aux.h"

/* local defines */
#define MBPREPROCESS_ALLOC_CHUNK 1000

#define MBPREPROCESS_MERGE_OFF 		0
#define MBPREPROCESS_MERGE_FILE 	1
#define MBPREPROCESS_MERGE_ASYNC 	2

#define MBPREPROCESS_TIME_LATENCY_APPLY_NONE		0x00
#define MBPREPROCESS_TIME_LATENCY_APPLY_NAV		0x01
#define MBPREPROCESS_TIME_LATENCY_APPLY_SENSORDEPTH	0x02
#define MBPREPROCESS_TIME_LATENCY_APPLY_HEADING		0x04
#define MBPREPROCESS_TIME_LATENCY_APPLY_ALTITUDE		0x08
#define MBPREPROCESS_TIME_LATENCY_APPLY_ATTITUDE		0x10
#define MBPREPROCESS_TIME_LATENCY_APPLY_ALL_ANCILLIARY	0x7F
#define MBPREPROCESS_TIME_LATENCY_APPLY_SURVEY		0x80
#define MBPREPROCESS_TIME_LATENCY_APPLY_ALL		0xFF

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	char program_name[] = "mbpreprocess";
	char help_message[] =  "mbpreprocess handles preprocessing of swath sonar data as part of setting up an MB-System processing structure for a dataset.\n";
	char usage_message[] = "mbpreprocess --verbose --help --merge-nav-from-file=file";
	extern char *optarg;
	int	option_index;
	int	errflg = 0;
	int	c;
	int	help = 0;

	/* MBIO status variables */
	int	status = MB_SUCCESS;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;
	char	*message;

	/* command line option definitions */
	/* mbpreprocess --verbose
	 * 		--help
	 * 		--input=datalist
	 * 		--format=format_id
	 * 		--nav-file=file
	 * 		--nav-file-format=format_id
	 * 		--output-sensor-fnv
	 * 		--nav-async=record_kind
	 * 		--nav-sensor=sensor_id
	 * 		--sensordepth-file=file
	 * 		--sensordepth-file-format=format_id
	 * 		--sensordepth-async=record_kind
	 * 		--sensordepth-sensor=sensor_id
	 * 		--heading-file=file
	 * 		--heading-file-format=format_id
	 * 		--heading-async=record_kind
	 * 		--heading-sensor=sensor_id
	 * 		--altitude-file=file
	 * 		--altitude-file-format=format_id
	 * 		--altitude-async=record_kind
	 * 		--altitude-sensor=sensor_id
	 * 		--attitude-file=file
	 * 		--attitude-file-format=format_id
	 * 		--attitude-async=record_kind
	 * 		--attitude-sensor=sensor_id
	 * 		--time-latency-file=file
	 * 		--time-latency-file-format=format_id
	 * 		--time-latency-constant=value
	 * 		--time-latency-apply-nav
	 * 		--time-latency-apply-sensordepth
	 * 		--time-latency-apply-heading
	 * 		--time-latency-apply-attitude
	 * 		--time-latency-apply-all-ancilliary
	 * 		--time-latency-apply-survey
	 * 		--time-latency-apply-all
	 * 		--filter=value
	 * 		--filter-apply-nav
	 * 		--filter-apply-sensordepth
	 * 		--filter-apply-heading
	 * 		--filter-apply-attitude
	 * 		--filter-apply-all-ancilliary
	 * 		--platform-file=platform_file
	 * 		--platform-target-sensor=sensor_id
	 * 		--sensordepth-offsets=offset_x/offset_y/offset_z
	 * 		--sonar-offsets=offset_x/offset_y/offset_z/offset_heading/offset_roll/offset_pitch
	 * 		--vru-offsets=offset_x/offset_y/offset_z/offset_heading/offset_roll/offset_pitch
	 * 		--navigation-offsets=offset_x/offset_y/offset_z/offset_heading/offset_roll/offset_pitch
	 * 		--no-change-survey
	 */
	static struct option options[] =
		{
		{"verbose",							no_argument, 		NULL, 		0},
		{"help",							no_argument, 		NULL, 		0},
		{"verbose",							no_argument, 		NULL, 		0},
		{"input",							required_argument, 	NULL, 		0},
		{"format",							required_argument, 	NULL, 		0},
		{"output-sensor-fnv",				no_argument, 		NULL, 		0},
		{"nav-file",						required_argument, 	NULL, 		0},
		{"nav-file-format",					required_argument, 	NULL, 		0},
		{"nav-async",						required_argument, 	NULL, 		0},
		{"nav-sensor",						required_argument, 	NULL, 		0},
		{"sensordepth-file",				required_argument, 	NULL, 		0},
		{"sensordepth-file-format",			required_argument, 	NULL, 		0},
		{"sensordepth-async",				required_argument, 	NULL, 		0},
		{"sensordepth-sensor",				required_argument, 	NULL, 		0},
		{"heading-file",					required_argument, 	NULL, 		0},
		{"heading-file-format",				required_argument, 	NULL, 		0},
		{"heading-async",					required_argument, 	NULL, 		0},
		{"heading-sensor",					required_argument, 	NULL, 		0},
		{"altitude-file",					required_argument, 	NULL, 		0},
		{"altitude-file-format",			required_argument, 	NULL, 		0},
		{"altitude-async",					required_argument, 	NULL, 		0},
		{"altitude-sensor",					required_argument, 	NULL, 		0},
		{"attitude-file",					required_argument, 	NULL, 		0},
		{"attitude-file-format",			required_argument, 	NULL, 		0},
		{"attitude-async",					required_argument, 	NULL, 		0},
		{"attitude-sensor",					required_argument, 	NULL, 		0},
		{"time-latency-file",					required_argument, 	NULL, 		0},
		{"time-latency-file-format",				required_argument, 	NULL, 		0},
		{"time-latency-constant",				required_argument, 	NULL, 		0},
		{"time-latency-apply-nav",				no_argument, 		NULL, 		0},
		{"time-latency-apply-sensordepth",		no_argument, 		NULL, 		0},
		{"time-latency-apply-heading",			no_argument, 		NULL, 		0},
		{"time-latency-apply-attitude",			no_argument, 		NULL, 		0},
		{"time-latency-apply-all-ancilliary",	no_argument, 		NULL, 		0},
		{"time-latency-apply-survey",			no_argument, 		NULL, 		0},
		{"time-latency-apply-all",				no_argument, 		NULL, 		0},
		{"time-latency-apply-nav",				no_argument, 		NULL, 		0},
		{"filter",							required_argument, 		NULL, 		0},
		{"filter-apply-sensordepth",		no_argument, 		NULL, 		0},
		{"filter-apply-heading",			no_argument, 		NULL, 		0},
		{"filter-apply-attitude",			no_argument, 		NULL, 		0},
		{"filter-apply-all-ancilliary",		no_argument, 		NULL, 		0},
		{"platform-file",					required_argument, 	NULL, 		0},
		{"platform-target-sensor",			required_argument, 	NULL, 		0},
		{"sensordepth-offsets",				required_argument, 	NULL, 		0},
		{"sonar-offsets",					required_argument, 	NULL, 		0},
		{"vru-offsets",						required_argument, 	NULL, 		0},
		{"navigation-offsets",				required_argument, 	NULL, 		0},
		{"no-change-survey",				no_argument,		NULL,		0},
		{NULL,								0, 					NULL, 		0}
		};

	/* asynchronous navigation, heading, attitude data */
	int	nav_mode = MBPREPROCESS_MERGE_OFF;
	mb_path	nav_file;
	int	nav_file_format = 0;
	int	nav_async = MB_DATA_DATA;
	int	nav_sensor = -1;
	int	nav_num = 0;
	int	nav_alloc = 0;
	double	*nav_time_d = NULL;
	double	*nav_navlon = NULL;
	double	*nav_navlat = NULL;
	double	*nav_speed = NULL;

	int	sensordepth_mode = MBPREPROCESS_MERGE_OFF;
	mb_path	sensordepth_file;
	int	sensordepth_file_format = 0;
	int	sensordepth_async = MB_DATA_DATA;
	int	sensordepth_sensor = -1;
	int	sensordepth_num = 0;
	int	sensordepth_alloc = 0;
	double	*sensordepth_time_d = NULL;
	double	*sensordepth_sensordepth = NULL;

	int	heading_mode = MBPREPROCESS_MERGE_OFF;
	mb_path	heading_file;
	int	heading_file_format = 0;
	int	heading_async = MB_DATA_DATA;
	int	heading_sensor = -1;
	int	heading_num = 0;
	int	heading_alloc = 0;
	double	*heading_time_d = NULL;
	double	*heading_heading = NULL;

	int	altitude_mode = MBPREPROCESS_MERGE_OFF;
	mb_path	altitude_file;
	int	altitude_file_format = 0;
	int	altitude_async = MB_DATA_DATA;
	int	altitude_sensor = -1;
	int	altitude_num = 0;
	int	altitude_alloc = 0;
	double	*altitude_time_d = NULL;
	double	*altitude_altitude = NULL;

	int	attitude_mode = MBPREPROCESS_MERGE_OFF;
	mb_path	attitude_file;
	int	attitude_file_format = 0;
	int	attitude_async = MB_DATA_DATA;
	int	attitude_sensor = -1;
	int	attitude_num = 0;
	int	attitude_alloc = 0;
	double	*attitude_time_d = NULL;
	double	*attitude_roll = NULL;
	double	*attitude_pitch = NULL;
	double	*attitude_heave = NULL;

	int	time_latency_mode = MB_SENSOR_TIME_LATENCY_NONE;
	mb_u_char time_latency_apply = MBPREPROCESS_TIME_LATENCY_APPLY_NONE;
	mb_path	time_latency_file;
	int	time_latency_format = 1;
	int	time_latency_num = 0;
	int	time_latency_alloc = 0;
	double	*time_latency_time_d = NULL;
	double	*time_latency_time_latency = NULL;
	double	time_latency_constant = 0.0;
	
	/* time domain filtering */
	mb_u_char filter_apply = MBPREPROCESS_TIME_LATENCY_APPLY_NONE;
	double	filter_length = 0.0;
	
	/* platform definition file */
	mb_path	platform_file;
	int	use_platform_file = MB_NO;
	struct mb_platform_struct *platform = NULL;
	struct mb_sensor_struct *sensor_bathymetry = NULL;
	struct mb_sensor_struct *sensor_backscatter = NULL;
	struct mb_sensor_struct *sensor_position = NULL;
	struct mb_sensor_struct *sensor_depth = NULL;
	struct mb_sensor_struct *sensor_heading = NULL;
	struct mb_sensor_struct *sensor_rollpitch = NULL;
	struct mb_sensor_struct *sensor_heave = NULL;
	struct mb_sensor_struct *sensor_target = NULL;
	int platform_target_sensor = -1;
	
	int output_sensor_fnv = MB_NO;
	
	int	no_change_survey = MB_NO;
	
	int	timestamp_changed = MB_NO;
	int	nav_changed = MB_NO;
	int	heading_changed = MB_NO;
	int	sensordepth_changed = MB_NO;
	int	altitude_changed = MB_NO;
	int	attitude_changed = MB_NO;

	/* MBIO read control parameters */
	int	read_datalist = MB_NO;
	int	read_data;
	mb_path	read_file;
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_UNSET;
	double	file_weight;
	int	format = 0;
	int	iformat;
	int	oformat;
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	mb_path	ifile;
	mb_path	ofile;
	mb_path	fileroot;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	int	obeams_bath;
	int	obeams_amp;
	int	opixels_ss;

	/* MBIO read values */
	void	*imbio_ptr = NULL;
	void	*ombio_ptr = NULL;
	void	*istore_ptr = NULL;
	int	kind;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	double	altitude;
	double	sensordepth;
	double	draft;
	double	roll;
	double	pitch;
	double	heave;
	char	*beamflag = NULL;
	double	*bath = NULL;
	double	*bathacrosstrack = NULL;
	double	*bathalongtrack = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*ssacrosstrack = NULL;
	double	*ssalongtrack = NULL;
	char	comment[MB_COMMENT_MAXLINE];
	double	navlon_org;
	double	navlat_org;
	double	speed_org;
	double	heading_org;
	double	altitude_org;
	double	sensordepth_org;
	double	draft_org;
	double	roll_org;
	double	pitch_org;
	double	heave_org;
	double	depth_offset_change;
	
	/* arrays for asynchronous data accessed using mb_extract_nnav() */
	int	nanavmax = MB_NAV_MAX;
	int	nanav;
	int	atime_i[7*MB_NAV_MAX];
	double	atime_d[MB_NAV_MAX];
	double	alon[MB_NAV_MAX];
	double	alat[MB_NAV_MAX];
	double	aspeed[MB_NAV_MAX];
	double	aheading[MB_NAV_MAX];
	double	asensordepth[MB_NAV_MAX];
	double	aroll[MB_NAV_MAX];
	double	apitch[MB_NAV_MAX];
	double	aheave[MB_NAV_MAX];
	
	/* counts of records read and written */
	int	n_rf_data = 0;
	int	n_rf_comment = 0;
	int	n_rf_nav = 0;
	int	n_rf_nav1 = 0;
	int	n_rf_nav2 = 0;
	int	n_rf_nav3 = 0;
	int	n_rf_att = 0;
	int	n_rf_att1 = 0;
	int	n_rf_att2 = 0;
	int	n_rf_att3 = 0;
	int	n_rt_data = 0;
	int	n_rt_comment = 0;
	int	n_rt_nav = 0;
	int	n_rt_nav1 = 0;
	int	n_rt_nav2 = 0;
	int	n_rt_nav3 = 0;
	int	n_rt_att = 0;
	int	n_rt_att1 = 0;
	int	n_rt_att2 = 0;
	int	n_rt_att3 = 0;
	
	int	n_wf_data = 0;
	int	n_wf_comment = 0;
	int	n_wf_nav = 0;
	int	n_wf_nav1 = 0;
	int	n_wf_nav2 = 0;
	int	n_wf_nav3 = 0;
	int	n_wf_att = 0;
	int	n_wf_att1 = 0;
	int	n_wf_att2 = 0;
	int	n_wf_att3 = 0;
	int	n_wt_data = 0;
	int	n_wt_comment = 0;
	int	n_wt_nav = 0;
	int	n_wt_nav1 = 0;
	int	n_wt_nav2 = 0;
	int	n_wt_nav3 = 0;
	int	n_wt_att = 0;
	int	n_wt_att1 = 0;
	int	n_wt_att2 = 0;
	int	n_wt_att3 = 0;
	
	mb_path afile;
	FILE *afp;
	int	istart, iend;
	double start_time_d;
	double end_time_d;
	
	mb_path fnvfile;
	int	isensor, ioffset;
	
	int	testformat;
	int	interp_status = MB_SUCCESS;
	int	interp_error = MB_ERROR_NO_ERROR;
	int	jnav = 0;
	int	jsensordepth = 0;
	int	jheading = 0;
	int	jaltitude = 0;
	int	jattitude = 0;
	int	i, n;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input to datalist.mb-1 */
	strcpy (read_file, "datalist.mb-1");

	/* process argument list */
	while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1)
	  switch (c)
		{
		/* long options all return c=0 */
		case 0:
			/* verbose */
			if (strcmp("verbose", options[option_index].name) == 0)
				{
				verbose++;
				}
			
			/* help */
			else if (strcmp("help", options[option_index].name) == 0)
				{
				help = MB_YES;
				}
				
			/*-------------------------------------------------------
			 * Define input file and format (usually a datalist) */
			
			/* input */
			else if (strcmp("input", options[option_index].name) == 0)
				{
				strcpy(read_file, optarg);
				}
			
			/* format */
			else if (strcmp("format", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &format);
				}
			
			/* output-sensor-fnv */
			else if (strcmp("output-sensor-fnv", options[option_index].name) == 0)
				{
				output_sensor_fnv = MB_YES;
				}
				
			/*-------------------------------------------------------
			 * Define source of navigation - could be an external file
			 * or an internal asynchronous record */
			
			/* nav-file */
			else if (strcmp("nav-file", options[option_index].name) == 0)
				{
				strcpy(nav_file, optarg);
				nav_mode = MBPREPROCESS_MERGE_FILE;
				}
			
			/* nav-file-format */
			else if (strcmp("nav-file-format", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &nav_file_format);
				}
			
			/* nav-async */
			else if (strcmp("nav-async", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &nav_async);
				if (n == 1)
					nav_mode = MBPREPROCESS_MERGE_ASYNC;
				}
			
			/* nav-sensor */
			else if (strcmp("nav-sensor", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &nav_sensor);
				}
				
			/*-------------------------------------------------------
			 * Define source of sensordepth - could be an external file
			 * or an internal asynchronous record */
			
			/* sensordepth-file */
			else if (strcmp("sensordepth-file", options[option_index].name) == 0)
				{
				strcpy(sensordepth_file, optarg);
				sensordepth_mode = MBPREPROCESS_MERGE_FILE;
				}
			
			/* sensordepth-file-format */
			else if (strcmp("sensordepth-file-format", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &sensordepth_file_format);
				}
			
			/* sensordepth-async */
			else if (strcmp("sensordepth-async", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &sensordepth_async);
				if (n == 1)
					sensordepth_mode = MBPREPROCESS_MERGE_ASYNC;
				}
			
			/* sensordepth-sensor */
			else if (strcmp("sensordepth-sensor", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &sensordepth_sensor);
				}
				
			/*-------------------------------------------------------
			 * Define source of heading - could be an external file
			 * or an internal asynchronous record */
			
			/* heading-file */
			else if (strcmp("heading-file", options[option_index].name) == 0)
				{
				strcpy(heading_file, optarg);
				heading_mode = MBPREPROCESS_MERGE_FILE;
				}
			
			/* heading-file-format */
			else if (strcmp("heading-file-format", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &heading_file_format);
				}
			
			/* heading-async */
			else if (strcmp("heading-async", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &heading_async);
				if (n == 1)
					heading_mode = MBPREPROCESS_MERGE_ASYNC;
				}
			
			/* heading-sensor */
			else if (strcmp("heading-sensor", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &heading_sensor);
				}
				
			/*-------------------------------------------------------
			 * Define source of altitude - could be an external file
			 * or an internal asynchronous record */
			
			/* altitude-file */
			else if (strcmp("altitude-file", options[option_index].name) == 0)
				{
				strcpy(altitude_file, optarg);
				altitude_mode = MBPREPROCESS_MERGE_FILE;
				}
			
			/* altitude-file-format */
			else if (strcmp("altitude-file-format", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &altitude_file_format);
				}
			
			/* altitude-async */
			else if (strcmp("altitude-async", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &altitude_async);
				if (n == 1)
					altitude_mode = MBPREPROCESS_MERGE_ASYNC;
				}
			
			/* altitude-sensor */
			else if (strcmp("altitude-sensor", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &altitude_sensor);
				}
				
			/*-------------------------------------------------------
			 * Define source of attitude - could be an external file
			 * or an internal asynchronous record */
			
			/* attitude-file */
			else if (strcmp("attitude-file", options[option_index].name) == 0)
				{
				strcpy(attitude_file, optarg);
				attitude_mode = MBPREPROCESS_MERGE_FILE;
				}
			
			/* attitude-file-format */
			else if (strcmp("attitude-file-format", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &attitude_file_format);
				}
			
			/* attitude-async */
			else if (strcmp("attitude-async", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &attitude_async);
				if (n == 1)
					attitude_mode = MBPREPROCESS_MERGE_ASYNC;
				}
			
			/* attitude-sensor */
			else if (strcmp("attitude-sensor", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &attitude_sensor);
				}
				
			/*-------------------------------------------------------
			 * Define source of time_latency - could be an external file
			 * or single value. Also define which data the time_latency model
			 * will be applied to - nav, sensordepth, heading, attitude,
			 * or all. */
			
			/* time-latency-file */
			else if (strcmp("time-latency-file", options[option_index].name) == 0)
				{
				strcpy(time_latency_file, optarg);
				time_latency_mode = MB_SENSOR_TIME_LATENCY_MODEL;
				}
			
			/* time-latency-file-format */
			else if (strcmp("time-latency-file-format", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &time_latency_format);
				}
			
			/* time-latency-constant */
			else if (strcmp("time-latency-constant", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%lf", &time_latency_constant);
				if (n == 1)
					time_latency_mode = MB_SENSOR_TIME_LATENCY_STATIC;
				}
			
			/* time-latency-apply-nav */
			else if (strcmp("time-latency-apply-nav", options[option_index].name) == 0)
				{
				time_latency_apply =  time_latency_apply | MBPREPROCESS_TIME_LATENCY_APPLY_NAV;
				}
			
			/* time-latency-apply-sensordepth */
			else if (strcmp("time-latency-apply-sensordepth", options[option_index].name) == 0)
				{
				time_latency_apply =  time_latency_apply | MBPREPROCESS_TIME_LATENCY_APPLY_SENSORDEPTH;
				}
			
			/* time-latency-apply-heading */
			else if (strcmp("time-latency-apply-heading", options[option_index].name) == 0)
				{
				time_latency_apply =  time_latency_apply | MBPREPROCESS_TIME_LATENCY_APPLY_HEADING;
				}
			
			/* time-latency-apply-attitude */
			else if (strcmp("time-latency-apply-attitude", options[option_index].name) == 0)
				{
				time_latency_apply =  time_latency_apply | MBPREPROCESS_TIME_LATENCY_APPLY_ATTITUDE;
				}
			
			/* time-latency-apply-altitude */
			else if (strcmp("time-latency-apply-altitude", options[option_index].name) == 0)
				{
				time_latency_apply =  time_latency_apply | MBPREPROCESS_TIME_LATENCY_APPLY_ATTITUDE;
				}
			
			/* time-latency-apply-all-ancilliary */
			else if (strcmp("time-latency-apply-all-ancilliary", options[option_index].name) == 0)
				{
				time_latency_apply =  MBPREPROCESS_TIME_LATENCY_APPLY_ALL_ANCILLIARY;
				}
			
			/* time-latency-apply-survey */
			else if (strcmp("time-latency-apply-survey", options[option_index].name) == 0)
				{
				time_latency_apply =  MBPREPROCESS_TIME_LATENCY_APPLY_SURVEY;
				}
			
			/* time-latency-apply-all */
			else if (strcmp("time-latency-apply-all", options[option_index].name) == 0)
				{
				time_latency_apply =  MBPREPROCESS_TIME_LATENCY_APPLY_ALL;
				}
				
			/*-------------------------------------------------------
			 * Define time domain filtering of ancilliary data such as
			 * nav, sensordepth, heading, attitude, and altitude */
			
			/* filter */
			else if (strcmp("filter", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%lf", &filter_length);
				}
			
			/* filter-apply-nav */
			else if (strcmp("filter-apply-nav", options[option_index].name) == 0)
				{
				filter_apply =  filter_apply | MBPREPROCESS_TIME_LATENCY_APPLY_NAV;
				}
			
			/* filter-apply-sensordepth */
			else if (strcmp("filter-apply-sensordepth", options[option_index].name) == 0)
				{
				filter_apply =  filter_apply | MBPREPROCESS_TIME_LATENCY_APPLY_SENSORDEPTH;
				}
			
			/* filter-apply-heading */
			else if (strcmp("filter-apply-heading", options[option_index].name) == 0)
				{
				filter_apply =  filter_apply | MBPREPROCESS_TIME_LATENCY_APPLY_HEADING;
				}
			
			/* filter-apply-attitude */
			else if (strcmp("filter-apply-attitude", options[option_index].name) == 0)
				{
				filter_apply =  filter_apply | MBPREPROCESS_TIME_LATENCY_APPLY_ATTITUDE;
				}
			
			/* filter-apply-altitude */
			else if (strcmp("filter-apply-altitude", options[option_index].name) == 0)
				{
				filter_apply =  filter_apply | MBPREPROCESS_TIME_LATENCY_APPLY_ATTITUDE;
				}
			
			/* filter-apply-all-ancilliary */
			else if (strcmp("filter-apply-all-ancilliary", options[option_index].name) == 0)
				{
				filter_apply =  MBPREPROCESS_TIME_LATENCY_APPLY_ALL_ANCILLIARY;
				}

			/*-------------------------------------------------------
			 * Set platform file */
			
			/* platform-file */
			else if (strcmp("platform-file", options[option_index].name) == 0)
				{
				n = sscanf (optarg,"%s", platform_file);
				if (n == 1)
					use_platform_file = MB_YES;
				}
			
			/* platform-target-sensor */
			else if (strcmp("platform-target-sensor", options[option_index].name) == 0)
				{
				n = sscanf (optarg,"%d", &platform_target_sensor);
				}
						
			/* no-change-survey */
			else if (strcmp("no-change-survey", options[option_index].name) == 0)
				{
				no_change_survey =  MB_YES;
				}
			
			break;
		case '?':
			errflg++;
		}

	/* if error flagged then print it and exit */
	if (errflg)
		{
		fprintf(stderr,"usage: %s\n", usage_message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
		}
	
	/* if no affected data have been specified apply time_latency to all */
	if (time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE
			&& time_latency_apply == MBPREPROCESS_TIME_LATENCY_APPLY_NONE)
			time_latency_apply =  MBPREPROCESS_TIME_LATENCY_APPLY_ALL_ANCILLIARY;
	
	/* if no affected data have been specified apply filtering to all ancilliary data */
	if (filter_length > 0.0
			&& filter_apply == MBPREPROCESS_TIME_LATENCY_APPLY_NONE)
			filter_apply =  MBPREPROCESS_TIME_LATENCY_APPLY_ALL_ANCILLIARY;

	/* print starting message */
	if (verbose == 1 || help)
		{
		fprintf(stderr,"\nProgram %s\n",program_name);
		fprintf(stderr,"Source File Version %s\n",version_id);
		fprintf(stderr,"MB-system Version %s\n",MB_VERSION);
		}

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s>\n",program_name);
		fprintf(stderr,"dbg2  Version %s\n",version_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Control Parameters:\n");
		fprintf(stderr,"dbg2       verbose:                    %d\n",verbose);
		fprintf(stderr,"dbg2       help:                       %d\n",help);
		fprintf(stderr,"dbg2       format:                     %d\n",format);
		fprintf(stderr,"dbg2       pings:                      %d\n",pings);
		fprintf(stderr,"dbg2       lonflip:                    %d\n",lonflip);
		fprintf(stderr,"dbg2       bounds[0]:                  %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:                  %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:                  %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:                  %f\n",bounds[3]);
		fprintf(stderr,"dbg2       btime_i[0]:                 %d\n",btime_i[0]);
		fprintf(stderr,"dbg2       btime_i[1]:                 %d\n",btime_i[1]);
		fprintf(stderr,"dbg2       btime_i[2]:                 %d\n",btime_i[2]);
		fprintf(stderr,"dbg2       btime_i[3]:                 %d\n",btime_i[3]);
		fprintf(stderr,"dbg2       btime_i[4]:                 %d\n",btime_i[4]);
		fprintf(stderr,"dbg2       btime_i[5]:                 %d\n",btime_i[5]);
		fprintf(stderr,"dbg2       btime_i[6]:                 %d\n",btime_i[6]);
		fprintf(stderr,"dbg2       etime_i[0]:                 %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]:                 %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]:                 %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]:                 %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]:                 %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]:                 %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       etime_i[6]:                 %d\n",etime_i[6]);
		fprintf(stderr,"dbg2       speedmin:                   %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:                    %f\n",timegap);
		fprintf(stderr,"dbg2       read_file:                  %s\n",read_file);
		fprintf(stderr,"dbg2       output_sensor_fnv:          %d\n",output_sensor_fnv);
		fprintf(stderr,"dbg2       nav_mode:                   %d\n",nav_mode);
		fprintf(stderr,"dbg2       nav_file:                   %s\n",nav_file);
		fprintf(stderr,"dbg2       nav_file_format:            %d\n",nav_file_format);
		fprintf(stderr,"dbg2       nav_async:                  %d\n",nav_async);
		fprintf(stderr,"dbg2       nav_sensor:                 %d\n",nav_sensor);
		fprintf(stderr,"dbg2       sensordepth_mode:           %d\n",sensordepth_mode);
		fprintf(stderr,"dbg2       sensordepth_file:           %s\n",sensordepth_file);
		fprintf(stderr,"dbg2       sensordepth_file_format:    %d\n",sensordepth_file_format);
		fprintf(stderr,"dbg2       sensordepth_async:          %d\n",sensordepth_async);
		fprintf(stderr,"dbg2       sensordepth_sensor:         %d\n",sensordepth_sensor);
		fprintf(stderr,"dbg2       heading_mode:               %d\n",heading_mode);
		fprintf(stderr,"dbg2       heading_file:               %s\n",heading_file);
		fprintf(stderr,"dbg2       heading_file_format:        %d\n",heading_file_format);
		fprintf(stderr,"dbg2       heading_async:              %d\n",heading_async);
		fprintf(stderr,"dbg2       heading_sensor:             %d\n",heading_sensor);
		fprintf(stderr,"dbg2       altitude_mode:              %d\n",altitude_mode);
		fprintf(stderr,"dbg2       altitude_file:              %s\n",altitude_file);
		fprintf(stderr,"dbg2       altitude_file_format:       %d\n",altitude_file_format);
		fprintf(stderr,"dbg2       altitude_async:             %d\n",altitude_async);
		fprintf(stderr,"dbg2       altitude_sensor:            %d\n",altitude_sensor);
		fprintf(stderr,"dbg2       attitude_mode:              %d\n",attitude_mode);
		fprintf(stderr,"dbg2       attitude_file:              %s\n",attitude_file);
		fprintf(stderr,"dbg2       attitude_file_format:       %d\n",attitude_file_format);
		fprintf(stderr,"dbg2       attitude_async:             %d\n",attitude_async);
		fprintf(stderr,"dbg2       attitude_sensor:            %d\n",attitude_sensor);
		fprintf(stderr,"dbg2       time_latency_mode:          %d\n",time_latency_mode);
		fprintf(stderr,"dbg2       time_latency_file:          %s\n",time_latency_file);
		fprintf(stderr,"dbg2       time_latency_format:        %d\n",time_latency_format);
		fprintf(stderr,"dbg2       time_latency_apply:         %x\n",time_latency_apply);
		fprintf(stderr,"dbg2       filter_length:              %f\n",filter_length);
		fprintf(stderr,"dbg2       filter_apply:               %x\n",filter_apply);
		fprintf(stderr,"dbg2       use_platform_file:          %d\n",use_platform_file);
		fprintf(stderr,"dbg2       platform_file:              %s\n",platform_file);
		fprintf(stderr,"dbg2       platform_target_sensor:     %d\n",platform_target_sensor);
		fprintf(stderr,"dbg2       no_change_survey:           %d\n",no_change_survey);
		}

	/* print starting verbose */
	else if (verbose > 0)
		{
		fprintf(stderr,"\nProgram <%s>\n",program_name);
		fprintf(stderr,"Version %s\n",version_id);
		fprintf(stderr,"MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"Input survey data to be preprocessed:\n");
		fprintf(stderr,"     read_file:                  %s\n",read_file);
		fprintf(stderr,"     format:                     %d\n",format);
		fprintf(stderr,"Source of platform model:\n");
		if (use_platform_file == MB_YES)
			fprintf(stderr,"     platform_file:              %s\n",platform_file);
		else
			fprintf(stderr,"     platform_file:              not specified\n");
		fprintf(stderr,"     platform_target_sensor:     %d\n",platform_target_sensor);
		fprintf(stderr,"Source of navigation data:\n");
		fprintf(stderr,"     nav_mode:                   %d\n",nav_mode);
		fprintf(stderr,"     nav_file:                   %s\n",nav_file);
		fprintf(stderr,"     nav_file_format:            %d\n",nav_file_format);
		fprintf(stderr,"     nav_async:                  %d\n",nav_async);
		fprintf(stderr,"     nav_sensor:                 %d\n",nav_sensor);
		fprintf(stderr,"Source of navigation data:\n");
		fprintf(stderr,"     sensordepth_mode:           %d\n",sensordepth_mode);
		fprintf(stderr,"     sensordepth_file:           %s\n",sensordepth_file);
		fprintf(stderr,"     sensordepth_file_format:    %d\n",sensordepth_file_format);
		fprintf(stderr,"     sensordepth_async:          %d\n",sensordepth_async);
		fprintf(stderr,"     sensordepth_sensor:         %d\n",sensordepth_sensor);
		fprintf(stderr,"Source of heading data:\n");
		fprintf(stderr,"     heading_mode:               %d\n",heading_mode);
		fprintf(stderr,"     heading_file:               %s\n",heading_file);
		fprintf(stderr,"     heading_file_format:        %d\n",heading_file_format);
		fprintf(stderr,"     heading_async:              %d\n",heading_async);
		fprintf(stderr,"     heading_sensor:             %d\n",heading_sensor);
		fprintf(stderr,"Source of altitude data:\n");
		fprintf(stderr,"     altitude_mode:              %d\n",altitude_mode);
		fprintf(stderr,"     altitude_file:              %s\n",altitude_file);
		fprintf(stderr,"     altitude_file_format:       %d\n",altitude_file_format);
		fprintf(stderr,"     altitude_async:             %d\n",altitude_async);
		fprintf(stderr,"     altitude_sensor:            %d\n",altitude_sensor);
		fprintf(stderr,"Source of attitude data:\n");
		fprintf(stderr,"     attitude_mode:              %d\n",attitude_mode);
		fprintf(stderr,"     attitude_file:              %s\n",attitude_file);
		fprintf(stderr,"     attitude_file_format:       %d\n",attitude_file_format);
		fprintf(stderr,"     attitude_async:             %d\n",attitude_async);
		fprintf(stderr,"     attitude_sensor:            %d\n",attitude_sensor);
		fprintf(stderr,"Time latency correction:\n");
		fprintf(stderr,"     time_latency_mode:          %d\n",time_latency_mode);
		fprintf(stderr,"     time_latency_file:          %s\n",time_latency_file);
		fprintf(stderr,"     time_latency_format:        %d\n",time_latency_format);
		fprintf(stderr,"     time_latency_apply:         %x\n",time_latency_apply);
		fprintf(stderr,"Time domain filtering:\n");
		fprintf(stderr,"     filter_length:              %f\n",filter_length);
		fprintf(stderr,"     filter_apply:               %x\n",filter_apply);
		fprintf(stderr,"Limit consequences:\n");
		fprintf(stderr,"     no_change_survey:           %d\n",no_change_survey);
		fprintf(stderr,"Additional output:\n");
		fprintf(stderr,"     output_sensor_fnv:          %d\n",output_sensor_fnv);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/*-------------------------------------------------------------------*/
	/* load platform definition if specified or if offsets otherwise specified create a platform structure */
	if (use_platform_file == MB_YES)
		{
		status = mb_platform_read(verbose, platform_file, (void **)&platform, &error);
		if (status == MB_FAILURE)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to open and parse platform file: %s\n", platform_file);
			fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
			exit(error);				
			}
			
		/* reset data sources according to commands */
		if (nav_sensor >= 0)
			platform->source_position = nav_sensor;
		if (sensordepth_sensor >= 0)
			platform->source_depth = sensordepth_sensor;
		if (heading_sensor >= 0)
			platform->source_heading = heading_sensor;
		if (attitude_sensor >= 0)
			{
			platform->source_rollpitch = attitude_sensor;
			platform->source_heave = attitude_sensor;
			}
		
		/* get sensor structures */
		if (platform->source_bathymetry >= 0)
			sensor_bathymetry = &(platform->sensors[platform->source_bathymetry]);
		if (platform->source_backscatter >= 0)
			sensor_backscatter = &(platform->sensors[platform->source_backscatter]);
		if (platform->source_position >= 0)
			sensor_position = &(platform->sensors[platform->source_position]);
		if (platform->source_depth >= 0)
			sensor_depth = &(platform->sensors[platform->source_depth]);
		if (platform->source_heading >= 0)
			sensor_heading = &(platform->sensors[platform->source_heading]);
		if (platform->source_rollpitch >= 0)
			sensor_rollpitch = &(platform->sensors[platform->source_rollpitch]);
		if (platform->source_heave >= 0)
			sensor_heave = &(platform->sensors[platform->source_heave]);
		if (platform_target_sensor < 0)
			platform_target_sensor = platform->source_bathymetry;
		if (platform_target_sensor >= 0)
			sensor_target = &(platform->sensors[platform_target_sensor]);
		}
		
	/*-------------------------------------------------------------------*/
	/* load ancilliary data from external files if requested */
	
	/* start by loading time latency model if required */
	if (time_latency_mode == MB_SENSOR_TIME_LATENCY_MODEL)
		{
		mb_loadtimeshiftdata(verbose, time_latency_file, time_latency_format,
			       &time_latency_num, &time_latency_alloc,
			       &time_latency_time_d, &time_latency_time_latency, &error);
		
		if (verbose > 0)
			fprintf(stderr,"%d time_latency records loaded from file %s\n", time_latency_num, time_latency_file);
		}
	
	/* import specified ancilliary data */
	if (nav_mode == MBPREPROCESS_MERGE_FILE)
		{
		mb_loadnavdata(verbose, nav_file, nav_file_format, lonflip,
			       &nav_num, &nav_alloc,
			       &nav_time_d, &nav_navlon, &nav_navlat, &nav_speed, &error);
		
		if (verbose > 0)
			fprintf(stderr,"%d navigation records loaded from file %s\n", nav_num, nav_file);
		}
	if (sensordepth_mode == MBPREPROCESS_MERGE_FILE)
		{
		mb_loadsensordepthdata(verbose, sensordepth_file, sensordepth_file_format,
				       &sensordepth_num, &sensordepth_alloc,
			       &sensordepth_time_d, &sensordepth_sensordepth, &error);
		
		if (verbose > 0)
			fprintf(stderr,"%d sensordepth records loaded from file %s\n", sensordepth_num, sensordepth_file);
		}
	if (heading_mode == MBPREPROCESS_MERGE_FILE)
		{
		mb_loadheadingdata(verbose, heading_file, heading_file_format,
			       &heading_num, &heading_alloc,
			       &heading_time_d, &heading_heading, &error);
		
		if (verbose > 0)
			fprintf(stderr,"%d heading records loaded from file %s\n", heading_num, heading_file);
		}
	if (altitude_mode == MBPREPROCESS_MERGE_FILE)
		{
		mb_loadaltitudedata(verbose, altitude_file, altitude_file_format,
			       &altitude_num, &altitude_alloc,
			       &altitude_time_d, &altitude_altitude, &error);
		
		if (verbose > 0)
			fprintf(stderr,"%d altitude records loaded from file %s\n", altitude_num, altitude_file);
		}
	if (attitude_mode == MBPREPROCESS_MERGE_FILE)
		{
		mb_loadattitudedata(verbose, attitude_file, attitude_file_format,
			       &attitude_num, &attitude_alloc,
			       &attitude_time_d, &attitude_roll, &attitude_pitch, &attitude_heave, &error);
		
		if (verbose > 0)
			fprintf(stderr,"%d attitude records loaded from file %s\n", heading_num, heading_file);
		}

	/*-------------------------------------------------------------------*/
	
	/* Do first pass through the data collecting ancilliary data from the desired source records */

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose,read_file,NULL,&format,&error);

	/* determine whether to read one file or a list of files */
	if (format < 0)
		read_datalist = MB_YES;

	/* open file list */
	if (read_datalist == MB_YES)
	    {
	    if ((status = mb_datalist_open(verbose,&datalist,
					    read_file,look_processed,&error)) != MB_SUCCESS)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to open data list file: %s\n",
			read_file);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	    if ((status = mb_datalist_read(verbose,datalist,
			    ifile,&iformat,&file_weight,&error))
			    == MB_SUCCESS)
			read_data = MB_YES;
	    else
			read_data = MB_NO;
	    }
	/* else copy single filename to be read */
	else
	    {
	    strcpy(ifile, read_file);
	    iformat = format;
	    read_data = MB_YES;
	    }

	/* loop over all files to be read */
	while (read_data == MB_YES)
		{
		if (verbose > 0)
			fprintf(stderr,"\nPass 1: Opening file %s %d\n", ifile, iformat);
			
		/* initialize reading the swath file */
		if ((status = mb_read_init(
			verbose,ifile,iformat,pings,lonflip,bounds,
			btime_i,etime_i,speedmin,timegap,
			&imbio_ptr,&btime_d,&etime_d,
			&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
			fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",ifile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
	
		beamflag = NULL;
		bath = NULL;
		amp = NULL;
		bathacrosstrack = NULL;
		bathalongtrack = NULL;
		ss = NULL;
		ssacrosstrack = NULL;
		ssalongtrack = NULL;
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							sizeof(char), (void **)&beamflag, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							sizeof(double), (void **)&bath, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE,
							sizeof(double), (void **)&amp, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							sizeof(double), (void **)&bathacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							sizeof(double), (void **)&bathalongtrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
							sizeof(double), (void **)&ss, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
							sizeof(double), (void **)&ssacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
							sizeof(double), (void **)&ssalongtrack, &error);
	
		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
				message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
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
	
		/* read data */
		while (error <= MB_ERROR_NO_ERROR)
			{
			/* reset error */
			error = MB_ERROR_NO_ERROR;
	
			/* read next data record */
			status = mb_get_all(verbose,imbio_ptr,&istore_ptr,&kind,
					    time_i,&time_d,&navlon,&navlat,
					    &speed,&heading,
					    &distance,&altitude,&sensordepth,
					    &beams_bath,&beams_amp,&pixels_ss,
					    beamflag,bath,amp,bathacrosstrack,bathalongtrack,
					    ss,ssacrosstrack,ssalongtrack,
					    comment,&error);
	
			/* some nonfatal errors do not matter */
			if (error < MB_ERROR_NO_ERROR && error > MB_ERROR_UNINTELLIGIBLE)
				{
				error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
				}

			/* print debug statements */
			if (verbose >= 2)
				{
				fprintf(stderr,"\ndbg2  Data record read in program <%s>\n",
					program_name);
				fprintf(stderr,"dbg2       kind:           %d\n",kind);
				fprintf(stderr,"dbg2       error:          %d\n",error);
				fprintf(stderr,"dbg2       status:         %d\n",status);
				}
				
			/* count records */
			if (kind == MB_DATA_DATA)
				{
				n_rf_data++;
				n_rt_data++;
				}
			else if (kind == MB_DATA_COMMENT)
				{
				n_rf_comment++;
				n_rt_comment++;
				}
			else if (kind == MB_DATA_NAV)
				{
				n_rf_nav++;
				n_rt_nav++;
				}
			else if (kind == MB_DATA_NAV1)
				{
				n_rf_nav1++;
				n_rt_nav1++;
				}
			else if (kind == MB_DATA_NAV2)
				{
				n_rf_nav2++;
				n_rt_nav2++;
				}
			else if (kind == MB_DATA_NAV3)
				{
				n_rf_nav3++;
				n_rt_nav3++;
				}
			else if (kind == MB_DATA_ATTITUDE)
				{
				n_rf_att++;
				n_rt_att++;
				}
			else if (kind == MB_DATA_ATTITUDE1)
				{
				n_rf_att1++;
				n_rt_att1++;
				}
			else if (kind == MB_DATA_ATTITUDE2)
				{
				n_rf_att2++;
				n_rt_att2++;
				}
			else if (kind == MB_DATA_ATTITUDE3)
				{
				n_rf_att3++;
				n_rt_att3++;
				}
				
			/* look for nav if not externally defined */
			if (status == MB_SUCCESS
				&& nav_mode == MBPREPROCESS_MERGE_ASYNC
				&& kind == nav_async)
				{
				/* extract nav data */
				status = mb_extract_nnav(verbose, imbio_ptr, istore_ptr,
							nanavmax, &kind, &nanav,
							atime_i, atime_d,
							alon, alat,
							aspeed, aheading, asensordepth,
							aroll, apitch, aheave,
							&error);

				/* allocate memory if needed */
				if (status == MB_SUCCESS
					&& nanav > 0
					&& nav_num + nanav >= nav_alloc)
					{
					nav_alloc += MAX(MBPREPROCESS_ALLOC_CHUNK, nanav);
					status = mb_reallocd(verbose,__FILE__,__LINE__,nav_alloc*sizeof(double),(void **)&nav_time_d,&error);
					status = mb_reallocd(verbose,__FILE__,__LINE__,nav_alloc*sizeof(double),(void **)&nav_navlon,&error);
					status = mb_reallocd(verbose,__FILE__,__LINE__,nav_alloc*sizeof(double),(void **)&nav_navlat,&error);
					status = mb_reallocd(verbose,__FILE__,__LINE__,nav_alloc*sizeof(double),(void **)&nav_speed,&error);
					if (error != MB_ERROR_NO_ERROR)
						{
						mb_error(verbose,error,&message);
						fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
						fprintf(stderr,"\nProgram <%s> Terminated\n",
						    program_name);
						exit(error);
						}
					}
					
				/* copy the nav data */
				if (status == MB_SUCCESS
					&& nanav > 0)
					{
					for (i=0;i<nanav;i++)
						{
						if (atime_d[i] > 0.0 && alon[i] != 0.0 && alat[i] != 0.0)
							{
							nav_time_d[nav_num] = atime_d[i];
							nav_navlon[nav_num] = alon[i];
							nav_navlat[nav_num] = alat[i];
							nav_speed[nav_num] = aspeed[i];
							nav_num++;
							}
						}
					}
				}
				
			/* look for sensordepth if not externally defined */
			if (status == MB_SUCCESS
				&& sensordepth_mode == MBPREPROCESS_MERGE_ASYNC
				&& kind == sensordepth_async)
				{
				/* extract sensordepth data */
				status = mb_extract_nnav(verbose, imbio_ptr, istore_ptr,
							nanavmax, &kind, &nanav,
							atime_i, atime_d,
							alon, alat,
							aspeed, aheading, asensordepth,
							aroll, apitch, aheave,
							&error);

				/* allocate memory if needed */
				if (status == MB_SUCCESS
					&& nanav > 0
					&& sensordepth_num + nanav >= sensordepth_alloc)
					{
					sensordepth_alloc += MAX(MBPREPROCESS_ALLOC_CHUNK, nanav);
					status = mb_reallocd(verbose,__FILE__,__LINE__,sensordepth_alloc*sizeof(double),(void **)&sensordepth_time_d,&error);
					status = mb_reallocd(verbose,__FILE__,__LINE__,sensordepth_alloc*sizeof(double),(void **)&sensordepth_sensordepth,&error);
					if (error != MB_ERROR_NO_ERROR)
						{
						mb_error(verbose,error,&message);
						fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
						fprintf(stderr,"\nProgram <%s> Terminated\n",
						    program_name);
						exit(error);
						}
					}
					
				/* copy the sensordepth data */
				if (status == MB_SUCCESS
					&& nanav > 0)
					{
					for (i=0;i<nanav;i++)
						{
						sensordepth_time_d[sensordepth_num] = atime_d[i];
						sensordepth_sensordepth[sensordepth_num] = asensordepth[i];
						sensordepth_num++;
						}
					}
				}
				
			/* look for heading if not externally defined */
			if (status == MB_SUCCESS
				&& heading_mode == MBPREPROCESS_MERGE_ASYNC
				&& kind == heading_async)
				{
				/* extract heading data */
				status = mb_extract_nnav(verbose, imbio_ptr, istore_ptr,
							nanavmax, &kind, &nanav,
							atime_i, atime_d,
							alon, alat,
							aspeed, aheading, asensordepth,
							aroll, apitch, aheave,
							&error);

				/* allocate memory if needed */
				if (status == MB_SUCCESS
					&& nanav > 0
					&& heading_num + nanav >= heading_alloc)
					{
					heading_alloc += MAX(MBPREPROCESS_ALLOC_CHUNK, nanav);
					status = mb_reallocd(verbose,__FILE__,__LINE__,heading_alloc*sizeof(double),(void **)&heading_time_d,&error);
					status = mb_reallocd(verbose,__FILE__,__LINE__,heading_alloc*sizeof(double),(void **)&heading_heading,&error);
					if (error != MB_ERROR_NO_ERROR)
						{
						mb_error(verbose,error,&message);
						fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
						fprintf(stderr,"\nProgram <%s> Terminated\n",
						    program_name);
						exit(error);
						}
					}
					
				/* copy the heading data */
				if (status == MB_SUCCESS
					&& nanav > 0)
					{
					for (i=0;i<nanav;i++)
						{
						heading_time_d[heading_num] = atime_d[i];
						heading_heading[heading_num] = aheading[i];
						heading_num++;
						}
					}
				}
				
			/* look for altitude if not externally defined */
			if (status == MB_SUCCESS
				&& altitude_mode == MBPREPROCESS_MERGE_ASYNC
				&& kind == altitude_async)
				{
				/* extract altitude data */
				status = mb_extract_altitude(verbose, imbio_ptr, istore_ptr,
							&kind, &sensordepth, &altitude,
							&error);

				/* allocate memory if needed */
				if (status == MB_SUCCESS
					&& altitude_num + 1 >= altitude_alloc)
					{
					altitude_alloc += MBPREPROCESS_ALLOC_CHUNK;
					status = mb_reallocd(verbose,__FILE__,__LINE__,altitude_alloc*sizeof(double),(void **)&altitude_time_d,&error);
					status = mb_reallocd(verbose,__FILE__,__LINE__,altitude_alloc*sizeof(double),(void **)&altitude_altitude,&error);
					if (error != MB_ERROR_NO_ERROR)
						{
						mb_error(verbose,error,&message);
						fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
						fprintf(stderr,"\nProgram <%s> Terminated\n",
						    program_name);
						exit(error);
						}
					}
					
				/* copy the altitude data */
				if (status == MB_SUCCESS)
					{
					altitude_time_d[altitude_num] = time_d;
					altitude_altitude[altitude_num] = altitude;
					altitude_num++;
					}
				}
				
			/* look for attitude if not externally defined */
			if (status == MB_SUCCESS
				&& attitude_mode == MBPREPROCESS_MERGE_ASYNC
				&& kind == attitude_async)
				{
				/* extract attitude data */
				status = mb_extract_nnav(verbose, imbio_ptr, istore_ptr,
							nanavmax, &kind, &nanav,
							atime_i, atime_d,
							alon, alat,
							aspeed, aheading, asensordepth,
							aroll, apitch, aheave,
							&error);

				/* allocate memory if needed */
				if (status == MB_SUCCESS
					&& nanav > 0
					&& attitude_num + nanav >= attitude_alloc)
					{
					attitude_alloc += MAX(MBPREPROCESS_ALLOC_CHUNK, nanav);
					status = mb_reallocd(verbose,__FILE__,__LINE__,attitude_alloc*sizeof(double),(void **)&attitude_time_d,&error);
					status = mb_reallocd(verbose,__FILE__,__LINE__,attitude_alloc*sizeof(double),(void **)&attitude_roll,&error);
					status = mb_reallocd(verbose,__FILE__,__LINE__,attitude_alloc*sizeof(double),(void **)&attitude_pitch,&error);
					status = mb_reallocd(verbose,__FILE__,__LINE__,attitude_alloc*sizeof(double),(void **)&attitude_heave,&error);
					if (error != MB_ERROR_NO_ERROR)
						{
						mb_error(verbose,error,&message);
						fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
						fprintf(stderr,"\nProgram <%s> Terminated\n",
						    program_name);
						exit(error);
						}
					}
					
				/* copy the attitude data */
				if (status == MB_SUCCESS
					&& nanav > 0)
					{
					for (i=0;i<nanav;i++)
						{
						attitude_time_d[attitude_num] = atime_d[i];
						attitude_roll[attitude_num] = aroll[i];
						attitude_pitch[attitude_num] = apitch[i];
						attitude_heave[attitude_num] = aheave[i];
						attitude_num++;
						}
					}
				}
			}
		
		/* output data counts */
		if (verbose > 0)
			{
			fprintf(stderr,"Pass 1: Records read from input file %s\n", ifile);
			fprintf(stderr,"     %d survey records\n", n_rf_data);
			fprintf(stderr,"     %d comment records\n", n_rf_comment);
			fprintf(stderr,"     %d nav records\n", n_rf_nav);
			fprintf(stderr,"     %d nav1 records\n", n_rf_nav1);
			fprintf(stderr,"     %d nav2 records\n", n_rf_nav2);
			fprintf(stderr,"     %d nav3 records\n", n_rf_nav3);
			fprintf(stderr,"     %d att records\n", n_rf_att);
			fprintf(stderr,"     %d att1 records\n", n_rf_att1);
			fprintf(stderr,"     %d att2 records\n", n_rf_att2);
			fprintf(stderr,"     %d att3 records\n", n_rf_att3);
			}
			
		/* close the swath file */
		status = mb_close(verbose,&imbio_ptr,&error);
	
		/* figure out whether and what to read next */
		if (read_datalist == MB_YES)
			{
			if ((status = mb_datalist_read(verbose,datalist,
				    ifile,&iformat,&file_weight,&error))
				    == MB_SUCCESS)
				read_data = MB_YES;
			else
				read_data = MB_NO;
			}
		else
			{
			read_data = MB_NO;
			}
	
		/* end loop over files in list */
		}
	if (read_datalist == MB_YES)
		mb_datalist_close(verbose,&datalist,&error);
	
	/* output data counts */
	if (verbose > 0)
		{
		fprintf(stderr,"\n-----------------------------------------------\n");
		fprintf(stderr,"Pass 1: Total records read from all input files:\n");
		fprintf(stderr,"     %d survey records\n", n_rt_data);
		fprintf(stderr,"     %d comment records\n", n_rt_comment);
		fprintf(stderr,"     %d nav records\n", n_rt_nav);
		fprintf(stderr,"     %d nav1 records\n", n_rt_nav1);
		fprintf(stderr,"     %d nav2 records\n", n_rt_nav2);
		fprintf(stderr,"     %d nav3 records\n", n_rt_nav3);
		fprintf(stderr,"     %d att records\n", n_rt_att);
		fprintf(stderr,"     %d att1 records\n", n_rt_att1);
		fprintf(stderr,"     %d att2 records\n", n_rt_att2);
		fprintf(stderr,"     %d att3 records\n", n_rt_att3);
		fprintf(stderr,"Pass 1: Asynchronous data available for merging:\n");
		fprintf(stderr,"     %d navigation data (mode:%d)\n", nav_num, nav_mode);
		fprintf(stderr,"     %d sensordepth data (mode:%d)\n", sensordepth_num, sensordepth_mode);
		fprintf(stderr,"     %d heading data (mode:%d)\n", heading_num, heading_mode);
		fprintf(stderr,"     %d altitude data (mode:%d)\n", altitude_num, altitude_mode);
		fprintf(stderr,"     %d attitude data (mode:%d)\n", attitude_num, attitude_mode);
		fprintf(stderr,"     %d time_latency data (mode:%d)\n", time_latency_num, time_latency_mode);
		fprintf(stderr,"-----------------------------------------------\n");
		}
		
	/* end first pass through data */
	
	/*-------------------------------------------------------------------*/

	/* deal with time latency corrections */
	if (verbose > 0)
		{
		fprintf(stderr,"\n-----------------------------------------------\n");
		fprintf(stderr,"Applying time latency corrections:\n");
		}
	
	/* position */
	if (nav_num > 0 && nav_alloc >= nav_num)
		{
		/* apply time latency correction called for in the platform file */
		if (sensor_position != NULL
			&& sensor_position->time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE)
			{
			if (verbose > 0)
				fprintf(stderr,"Applying time latency correction from platform model to %d position data\n", nav_num);
			mb_apply_time_latency(verbose, nav_num, nav_time_d,
									sensor_position->time_latency_mode,
									sensor_position->time_latency_static,
									sensor_position->num_time_latency,
									sensor_position->time_latency_time_d,
									sensor_position->time_latency_value,
									&error);
			}
			
		/* apply time latency correction called for on the command line */
		if ((time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE)
			&& (time_latency_apply & MBPREPROCESS_TIME_LATENCY_APPLY_NAV))
			{
			if (verbose > 0)
				fprintf(stderr,"Applying time latency correction from command line to %d position data\n", nav_num);
			mb_apply_time_latency(verbose, nav_num, nav_time_d,
									time_latency_mode,
									time_latency_constant,
									time_latency_num,
									time_latency_time_d,
									time_latency_time_latency,
									&error);
			}
		}
	
	/* sensordepth */
	if (sensordepth_num > 0 && sensordepth_alloc >= sensordepth_num)
		{
		/* apply time latency correction called for in the platform file */
		if (sensor_depth != NULL
			&& sensor_depth->time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE)
			{
			if (verbose > 0)
				fprintf(stderr,"Applying time latency correction from platform model to %d sensordepth data\n", sensordepth_num);
			mb_apply_time_latency(verbose, sensordepth_num, sensordepth_time_d,
									sensor_depth->time_latency_mode,
									sensor_depth->time_latency_static,
									sensor_depth->num_time_latency,
									sensor_depth->time_latency_time_d,
									sensor_depth->time_latency_value,
									&error);
			}
			
		/* apply time latency correction called for on the command line */
		if ((time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE)
			&& (time_latency_apply & MBPREPROCESS_TIME_LATENCY_APPLY_SENSORDEPTH))
			{
			if (verbose > 0)
				fprintf(stderr,"Applying time latency correction from command line to %d sensordepth data\n", sensordepth_num);
			mb_apply_time_latency(verbose, sensordepth_num, sensordepth_time_d,
									time_latency_mode,
									time_latency_constant,
									time_latency_num,
									time_latency_time_d,
									time_latency_time_latency,
									&error);
			}
		}
	
	/* heading */
	if (heading_num > 0 && heading_alloc >= heading_num)
		{
		/* apply time latency correction called for in the platform file */
		if (sensor_heading != NULL
			&& sensor_heading->time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE)
			{
			if (verbose > 0)
				fprintf(stderr,"Applying time latency correction from platform model to %d heading data\n", heading_num);
			mb_apply_time_latency(verbose, heading_num, heading_time_d,
									sensor_heading->time_latency_mode,
									sensor_heading->time_latency_static,
									sensor_heading->num_time_latency,
									sensor_heading->time_latency_time_d,
									sensor_heading->time_latency_value,
									&error);
			}
			
		/* apply time latency correction called for on the command line */
		if ((time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE)
			&& (time_latency_apply & MBPREPROCESS_TIME_LATENCY_APPLY_HEADING))
			{
			if (verbose > 0)
				fprintf(stderr,"Applying time latency correction from command line to %d heading data\n", heading_num);
			mb_apply_time_latency(verbose, heading_num, heading_time_d,
									time_latency_mode,
									time_latency_constant,
									time_latency_num,
									time_latency_time_d,
									time_latency_time_latency,
									&error);
			}
		}

	
	/* altitude */
	if (altitude_num > 0 && altitude_alloc >= altitude_num)
		{
		/* apply time latency correction called for on the command line */
		if ((time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE)
			&& (time_latency_apply & MBPREPROCESS_TIME_LATENCY_APPLY_ALTITUDE))
			{
			if (verbose > 0)
				fprintf(stderr,"Applying time latency correction from command line to %d altitude data\n", altitude_num);
			mb_apply_time_latency(verbose, altitude_num, altitude_time_d,
									time_latency_mode,
									time_latency_constant,
									time_latency_num,
									time_latency_time_d,
									time_latency_time_latency,
									&error);
			}
		}

	
	/* attitude */
	if (attitude_num > 0 && attitude_alloc >= attitude_num)
		{
		/* apply time latency correction called for in the platform file */
		if (sensor_rollpitch != NULL
			&& sensor_rollpitch->time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE)
			{
			if (verbose > 0)
				fprintf(stderr,"Applying time latency correction from platform model to %d attitude data\n", attitude_num);
			mb_apply_time_latency(verbose, attitude_num, attitude_time_d,
									sensor_rollpitch->time_latency_mode,
									sensor_rollpitch->time_latency_static,
									sensor_rollpitch->num_time_latency,
									sensor_rollpitch->time_latency_time_d,
									sensor_rollpitch->time_latency_value,
									&error);
			}
			
		/* apply time latency correction called for on the command line */
		if ((time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE)
			&& (time_latency_apply & MBPREPROCESS_TIME_LATENCY_APPLY_ATTITUDE))
			{
			if (verbose > 0)
				fprintf(stderr,"Applying time latency correction from command line to %d attitude data\n", attitude_num);
			mb_apply_time_latency(verbose, attitude_num, attitude_time_d,
									time_latency_mode,
									time_latency_constant,
									time_latency_num,
									time_latency_time_d,
									time_latency_time_latency,
									&error);
			}
		}

	/*-------------------------------------------------------------------*/

	/* deal with filtering */
	if (verbose > 0)
		{
		fprintf(stderr,"\n-----------------------------------------------\n");
		fprintf(stderr,"Applying time domain filtering:\n");
		}
	
	/* filter position */
	if ((filter_apply & MBPREPROCESS_TIME_LATENCY_APPLY_NAV)
		&& nav_num > 0 && nav_alloc >= nav_num)
		{
		if (verbose > 0)
			fprintf(stderr,"Applying %f second Gaussian filter to %d position data\n", filter_length, nav_num);
		mb_apply_time_filter(verbose, nav_num, nav_time_d, nav_navlon,
								filter_length, &error);
		mb_apply_time_filter(verbose, nav_num, nav_time_d, nav_navlat,
								filter_length, &error);
		}
	
	/* filter sensordepth */
	if ((filter_apply & MBPREPROCESS_TIME_LATENCY_APPLY_SENSORDEPTH)
		&& sensordepth_num > 0 && sensordepth_alloc >= sensordepth_num)
		{
		if (verbose > 0)
			fprintf(stderr,"Applying %f second Gaussian filter to %d sensordepth data\n", filter_length, sensordepth_num);
		mb_apply_time_filter(verbose, sensordepth_num, sensordepth_time_d, sensordepth_sensordepth,
								filter_length, &error);
		}
	
	/* heading */
	if ((filter_apply & MBPREPROCESS_TIME_LATENCY_APPLY_HEADING)
		&& heading_num > 0 && heading_alloc >= heading_num)
		{
		if (verbose > 0)
			fprintf(stderr,"Applying %f second Gaussian filter to %d heading data\n", filter_length, heading_num);
		mb_apply_time_filter(verbose, heading_num, heading_time_d, heading_heading,
								filter_length, &error);
		}

	/* altitude */
	if ((filter_apply & MBPREPROCESS_TIME_LATENCY_APPLY_ALTITUDE)
		&& altitude_num > 0 && altitude_alloc >= altitude_num)
		{
		if (verbose > 0)
			fprintf(stderr,"Applying %f second Gaussian filter to %d altitude data\n", filter_length, altitude_num);
		mb_apply_time_filter(verbose, altitude_num, altitude_time_d, altitude_altitude,
								filter_length, &error);
		}

	/* attitude */
	if ((filter_apply & MBPREPROCESS_TIME_LATENCY_APPLY_ATTITUDE)
		&& attitude_num > 0 && attitude_alloc >= attitude_num)
		{
		if (verbose > 0)
			fprintf(stderr,"Applying %f second Gaussian filter to %d attitude data\n", filter_length, attitude_num);
		mb_apply_time_filter(verbose, attitude_num, attitude_time_d, attitude_roll,
								filter_length, &error);
		mb_apply_time_filter(verbose, attitude_num, attitude_time_d, attitude_pitch,
								filter_length, &error);
		mb_apply_time_filter(verbose, attitude_num, attitude_time_d, attitude_heave,
								filter_length, &error);
		}

	if (verbose > 0)
		{
		fprintf(stderr,"-----------------------------------------------\n");
		}
	
	/*-------------------------------------------------------------------*/

//for (i=0;i<nav_num;i++)
//fprintf(stderr,"NAV %d %f %f %f\n",i,nav_time_d[i],nav_navlon[i],nav_navlat[i]);
//fprintf(stderr," \n");

	/* Do second pass through the data reading everything,
		correcting survey data, and outputting everything */
				
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
	n_rt_data = 0;
	n_rt_comment = 0;
	n_rt_nav = 0;
	n_rt_nav1 = 0;
	n_rt_nav2 = 0;
	n_rt_nav3 = 0;
	n_rt_att = 0;
	n_rt_att1 = 0;
	n_rt_att2 = 0;
	n_rt_att3 = 0;
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
	
	/* if requested to output integrated nav for all survey sensors, open files */
fprintf(stderr,"Checking to see if sensor fnv files to be output:%d\n",output_sensor_fnv);
	if (output_sensor_fnv == MB_YES && platform != NULL)
		{
		for (isensor=0; isensor < platform->num_sensors; isensor++)
			{
fprintf(stderr,"Checking sensor %d: %d\n",isensor,platform->sensors[isensor].capability2);
			if (platform->sensors[isensor].capability2 != 0)
				{
				for (ioffset = 0; ioffset < platform->sensors[isensor].num_offsets; ioffset++)
					{
fprintf(stderr,"Outputting sensor %d offset %d\n",isensor,ioffset);
					sprintf(fnvfile, "sensor_%2.2d_%2.2d_%2.2d.fnv", isensor, ioffset, platform->sensors[isensor].type);
					if ((platform->sensors[isensor].offsets[ioffset].ofp = fopen(fnvfile, "w")) == NULL)
						{
						error = MB_ERROR_OPEN_FAIL;
						fprintf(stderr,"\nUnable to open sensor fnv data file <%s> for writing\n",fnvfile);
						fprintf(stderr,"\nProgram <%s> Terminated\n",
							program_name);
						exit(error);
						}
					}
				}
			}
		}

	/* open file list */
	if (read_datalist == MB_YES)
	    {
	    if ((status = mb_datalist_open(verbose,&datalist,
					    read_file,look_processed,&error)) != MB_SUCCESS)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to open data list file: %s\n",
			read_file);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	    if ((status = mb_datalist_read(verbose,datalist,
			    ifile,&iformat,&file_weight,&error))
			    == MB_SUCCESS)
		read_data = MB_YES;
	    else
		read_data = MB_NO;
	    }
	/* else copy single filename to be read */
	else
	    {
	    strcpy(ifile, read_file);
	    iformat = format;
	    read_data = MB_YES;
	    }

	/* loop over all files to be read */
	while (read_data == MB_YES)
		{
		/* get output format - in some cases this may be a
		 * different, generally extended format
		 * more suitable for processing than the original */
		if (iformat == MBF_EMOLDRAW
			|| iformat == MBF_EM12IFRM
			|| iformat == MBF_EM12DARW
			|| iformat == MBF_EM300RAW
			|| iformat == MBF_EM300MBA)
			oformat = MBF_EM300MBA;
		else if (iformat == MBF_EM710RAW
			|| iformat == MBF_EM710MBA)
			oformat = MBF_EM710MBA;
		else
			oformat = iformat;
		
		/* figure out the output file name */
		status = mb_get_format(verbose, ifile, fileroot, &testformat, &error);
		sprintf(ofile, "%s.mb%d", fileroot, oformat);
		if (strcmp(ifile,ofile) == 0)
			sprintf(ofile, "%sr.mb%d", fileroot, oformat);

		if (verbose > 0)
			fprintf(stderr,"\nPass 2: Opening input file:  %s %d\n", ifile, iformat);
			
		/* initialize reading the input file */
		if ((status = mb_read_init(
			verbose,ifile,iformat,pings,lonflip,bounds,
			btime_i,etime_i,speedmin,timegap,
			&imbio_ptr,&btime_d,&etime_d,
			&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
			fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",ifile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}

		if (verbose > 0)
			fprintf(stderr,"Pass 2: Opening output file: %s %d\n", ofile, oformat);

		/* initialize writing the output swath file */
		if ((status = mb_write_init(
			verbose,ofile,oformat,&ombio_ptr,
			&obeams_bath,&obeams_amp,&opixels_ss,&error)) != MB_SUCCESS)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error returned from function <mb_write_init>:\n%s\n",message);
			fprintf(stderr,"\nMultibeam File <%s> not initialized for writing\n",ofile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
	
		beamflag = NULL;
		bath = NULL;
		amp = NULL;
		bathacrosstrack = NULL;
		bathalongtrack = NULL;
		ss = NULL;
		ssacrosstrack = NULL;
		ssalongtrack = NULL;
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							sizeof(char), (void **)&beamflag, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							sizeof(double), (void **)&bath, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE,
							sizeof(double), (void **)&amp, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							sizeof(double), (void **)&bathacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
							sizeof(double), (void **)&bathalongtrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
							sizeof(double), (void **)&ss, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
							sizeof(double), (void **)&ssacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN,
							sizeof(double), (void **)&ssalongtrack, &error);
	
		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
				message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
			
		/* open synchronous attitude file */
		sprintf(afile,"%s.sta",ofile);
		if ((afp = fopen(afile, "w")) == NULL)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to open synchronous attitude data file <%s> for writing\n",afile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
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

		/* ------------------------------- */
		/* write comments to output file   */

		/* ------------------------------- */
		/* start read+process,+output loop */
		while (error <= MB_ERROR_NO_ERROR)
			{
			/* reset error */
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
	
			/* read next data record */
			status = mb_get_all(verbose,imbio_ptr,&istore_ptr,&kind,
					    time_i,&time_d,&navlon_org,&navlat_org,
					    &speed_org,&heading_org,
					    &distance,&altitude_org,&sensordepth_org,
					    &beams_bath,&beams_amp,&pixels_ss,
					    beamflag,bath,amp,bathacrosstrack,bathalongtrack,
					    ss,ssacrosstrack,ssalongtrack,
					    comment,&error);
	
			/* some nonfatal errors do not matter */
			if (error < MB_ERROR_NO_ERROR && error > MB_ERROR_UNINTELLIGIBLE)
				{
				error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
				}
				
			/* count records */
			if (kind == MB_DATA_DATA)
				{
				n_rf_data++;
				n_rt_data++;
				if (start_time_d <= 0.0)
					start_time_d = time_d;
				end_time_d = time_d;
				}
			else if (kind == MB_DATA_COMMENT)
				{
				n_rf_comment++;
				n_rt_comment++;
				}
			else if (kind == MB_DATA_NAV)
				{
				n_rf_nav++;
				n_rt_nav++;
				}
			else if (kind == MB_DATA_NAV1)
				{
				n_rf_nav1++;
				n_rt_nav1++;
				}
			else if (kind == MB_DATA_NAV2)
				{
				n_rf_nav2++;
				n_rt_nav2++;
				}
			else if (kind == MB_DATA_NAV3)
				{
				n_rf_nav3++;
				n_rt_nav3++;
				}
			else if (kind == MB_DATA_ATTITUDE)
				{
				n_rf_att++;
				n_rt_att++;
				}
			else if (kind == MB_DATA_ATTITUDE1)
				{
				n_rf_att1++;
				n_rt_att1++;
				}
			else if (kind == MB_DATA_ATTITUDE2)
				{
				n_rf_att2++;
				n_rt_att2++;
				}
			else if (kind == MB_DATA_ATTITUDE3)
				{
				n_rf_att3++;
				n_rt_att3++;
				}

			timestamp_changed = MB_NO;
			nav_changed = MB_NO;
			heading_changed = MB_NO;
			sensordepth_changed = MB_NO;
			attitude_changed = MB_NO;
				
			/* apply preprocessing to survey data records */
			if (status == MB_SUCCESS
				&& (kind == MB_DATA_DATA
					|| kind == MB_DATA_SUBBOTTOM_MCS
					|| kind == MB_DATA_SUBBOTTOM_CNTRBEAM
					|| kind == MB_DATA_SUBBOTTOM_SUBBOTTOM
					|| kind == MB_DATA_SIDESCAN2
					|| kind == MB_DATA_SIDESCAN3
					|| kind == MB_DATA_WATER_COLUMN))
				{
					
				/* apply time latency correction called for in the platform file */
				if (sensor_target != NULL
					&& sensor_target->time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE)
					mb_apply_time_latency(verbose, 1, &time_d,
											sensor_target->time_latency_mode,
											sensor_target->time_latency_static,
											sensor_target->num_time_latency,
											sensor_target->time_latency_time_d,
											sensor_target->time_latency_value,
											&error);
			
				/* apply time latency correction called for on the command line */
				if ((time_latency_mode != MB_SENSOR_TIME_LATENCY_NONE)
					&& (time_latency_apply & MBPREPROCESS_TIME_LATENCY_APPLY_SURVEY))
					mb_apply_time_latency(verbose, 1, &time_d,
											time_latency_mode,
											time_latency_constant,
											time_latency_num,
											time_latency_time_d,
											time_latency_time_latency,
											&error);
				
				/* attempt to execute a preprocess function for these data */
				status = mb_preprocess(verbose, imbio_ptr, istore_ptr, (void *)platform,
							nav_num, nav_time_d, nav_navlon, nav_navlat, nav_speed,
							sensordepth_num, sensordepth_time_d, sensordepth_sensordepth,
							heading_num, heading_time_d, heading_heading,
							altitude_num, altitude_time_d, altitude_altitude,
							attitude_num, attitude_time_d, attitude_roll, attitude_pitch, attitude_heave,
							&error);
				//status = MB_FAILURE;
				
				/* If a predefined preprocess function does not exist for 
				 * this format then standard preprocessing will be done
				 *      1) Replace time tag, nav, attitude
				 * 	2) if attitude values changed rotate bathymetry accordingly
				 * 	3) if any values changed reinsert the data */
				if (status == MB_FAILURE)
					{
					/* reset status and error */
					status = MB_SUCCESS;
					error = MB_ERROR_NO_ERROR;

					/* call mb_extract_nav to get attitude */
					status = mb_extract_nav(verbose, imbio_ptr, istore_ptr, &kind,
								time_i, &time_d, &navlon_org, &navlat_org,
								&speed_org, &heading_org, &draft_org,
								&roll_org, &pitch_org, &heave_org, &error);
					
					/* call mb_extract_altitude to get altitude */
					status = mb_extract_altitude(verbose, imbio_ptr, istore_ptr,
								&kind, &sensordepth_org, &altitude_org,
								&error);
					
					/* save the original values */
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
	
					/* get nav sensordepth heading attitude values for record timestamp */
					if (nav_num > 0)
						{
						interp_status = mb_linear_interp_longitude(verbose,
									nav_time_d-1, nav_navlon-1, nav_num, 
									time_d, &navlon, &jnav,
									&interp_error);
						interp_status = mb_linear_interp_latitude(verbose,
									nav_time_d-1, nav_navlat-1, nav_num, 
									time_d, &navlat, &jnav,
									&interp_error);
						interp_status = mb_linear_interp(verbose,
									nav_time_d-1, nav_speed-1, nav_num, 
									time_d, &speed, &jnav,
									&interp_error);
						nav_changed = MB_YES;
						}
					if (sensordepth_num > 0)
						{
						interp_status = mb_linear_interp(verbose,
									sensordepth_time_d-1, sensordepth_sensordepth-1, sensordepth_num, 
									time_d, &sensordepth, &jsensordepth,
									&interp_error);
						sensordepth_changed = MB_YES;
						}
					if (heading_num > 0)
						{
						interp_status = mb_linear_interp_heading(verbose,
									heading_time_d-1, heading_heading-1, heading_num, 
									time_d, &heading, &jheading,
									&interp_error);
						heading_changed = MB_YES;
						}
					if (altitude_num > 0)
						{
						interp_status = mb_linear_interp(verbose,
									altitude_time_d-1, altitude_altitude-1, altitude_num, 
									time_d, &altitude, &jaltitude,
									&interp_error);
						altitude_changed = MB_YES;
						}
					if (attitude_num > 0)
						{
						interp_status = mb_linear_interp(verbose,
									attitude_time_d-1, attitude_roll-1, attitude_num, 
									time_d, &roll, &jattitude,
									&interp_error);
						interp_status = mb_linear_interp(verbose,
									attitude_time_d-1, attitude_pitch-1, attitude_num, 
									time_d, &pitch, &jattitude,
									&interp_error);
						interp_status = mb_linear_interp(verbose,
									attitude_time_d-1, attitude_heave-1, attitude_num, 
									time_d, &heave, &jattitude,
									&interp_error);
						attitude_changed = MB_YES;
						}
					if (sensordepth_num > 0 || attitude_num > 0)
						{
						draft = sensordepth - heave;
						}
		
					if (platform != NULL)
						{
						/* calculate position of target sensor */
						status = mb_platform_position (verbose, (void *)platform,
										platform_target_sensor, 0,
										navlon, navlat, sensordepth,
										heading, roll, pitch,
										&navlon, &navlat, &sensordepth,
										&error);
						draft = sensordepth - heave;
						nav_changed = MB_YES;
						sensordepth_changed = MB_YES;

						/* Update swathsensor attitude (note: no longer vehicle attitude) */
						status = mb_platform_orientation_target (verbose, (void *)platform,
										platform_target_sensor, 0,
										heading, roll, pitch,
										&heading, &roll, &pitch,
										&error);
						attitude_changed = MB_YES;
						}

					/* if attitude changed apply rigid rotations to any bathymetry */
					if (attitude_changed == MB_YES)
						{
						/* loop over the beams */
						for (i=0;i<beams_bath;i++)
							{
							if (beamflag[i] != MB_FLAG_NULL)
								{	
								/* strip off original heave + draft */
								bath[i] -= sensordepth_org;
								/* rotate beam by 
								   rolldelta:  Roll relative to previous correction and bias included
								   pitchdelta: Pitch relative to previous correction and bias included
								   heading:    Heading absolute (bias included) */
								mb_platform_math_attitude_rotate_beam (
										verbose,
										bathacrosstrack[i], bathalongtrack[i], bath[i],
										roll,  pitch,  0.0,
										&(bathacrosstrack[i]), &(bathalongtrack[i]), &(bath[i]),
										&error);
								}
								/* add heave and draft back in */
								bath[i] += sensordepth_org;
								}
							}

					/* recalculate bathymetry by changes to sensor depth  */
					if (sensordepth_changed == MB_YES)
						{
						/* get draft change */
						depth_offset_change = draft - draft_org;

						/* loop over the beams */
						for (i=0;i<beams_bath;i++)
							{
							if (beamflag[i] != MB_FLAG_NULL)
								{
								/* apply transducer depth change to depths */
								bath[i] += depth_offset_change;			
								}
							}
						}

					/* insert navigation */
					if (nav_changed == MB_YES
						|| heading_changed == MB_YES
						|| sensordepth_changed == MB_YES
						|| attitude_changed == MB_YES)
						{
						status = mb_insert_nav(verbose, imbio_ptr, istore_ptr,
									time_i, time_d, navlon, navlat,
									speed, heading, draft,
									roll, pitch, heave, &error);
						}
				
					/* insert altitude */
					if (altitude_changed == MB_YES)
						{
						status = mb_insert_altitude(verbose, imbio_ptr, istore_ptr,
									sensordepth, altitude, &error);
						if (status == MB_FAILURE)
							{
							status = MB_SUCCESS;
							error = MB_ERROR_NO_ERROR;
							}
						}
						
					/* if attitude changed apply rigid rotations to the bathymetry */
					if (no_change_survey == MB_NO
						&& (attitude_changed == MB_YES
							|| sensordepth_changed == MB_YES))
						{
						status = mb_insert(verbose,imbio_ptr,
									istore_ptr, kind,
									time_i, time_d,
									navlon, navlat, speed, heading,
									beams_bath,beams_amp,pixels_ss,
									beamflag, bath, amp, bathacrosstrack, bathalongtrack,
									ss, ssacrosstrack, ssalongtrack,
									comment, &error);
						}
					}
				}

			/* write some data */
			if (error == MB_ERROR_NO_ERROR)
				{
				status = mb_put_all(verbose,ombio_ptr,
						istore_ptr,MB_NO,kind,
						time_i,time_d,
						navlon,navlat,speed,heading,
						obeams_bath,obeams_amp,opixels_ss,
						beamflag,bath,amp,bathacrosstrack,bathalongtrack,
						ss,ssacrosstrack,ssalongtrack,
						comment,&error);
				if (status != MB_SUCCESS)
					{
					mb_error(verbose,error,&message);
					fprintf(stderr,"\nMBIO Error returned from function <mb_put>:\n%s\n",message);
					fprintf(stderr,"\nMultibeam Data Not Written To File <%s>\n",ofile);
					fprintf(stderr,"\nProgram <%s> Terminated\n",
						program_name);
					exit(error);
					}
					
				/* output synchronous attitude */
				if (kind == MB_DATA_DATA)
					fprintf(afp, "%0.6f\t%0.3f\t%0.3f\n", time_d, roll, pitch);

				/* count records */
				if (kind == MB_DATA_DATA)
					{
					n_wf_data++;
					n_wt_data++;
					}
				else if (kind == MB_DATA_COMMENT)
					{
					n_wf_comment++;
					n_wt_comment++;
					}
				else if (kind == MB_DATA_NAV)
					{
					n_wf_nav++;
					n_wt_nav++;
					}
				else if (kind == MB_DATA_NAV1)
					{
					n_wf_nav1++;
					n_wt_nav1++;
					}
				else if (kind == MB_DATA_NAV2)
					{
					n_wf_nav2++;
					n_wt_nav2++;
					}
				else if (kind == MB_DATA_NAV3)
					{
					n_wf_nav3++;
					n_wt_nav3++;
					}
				else if (kind == MB_DATA_ATTITUDE)
					{
					n_wf_att++;
					n_wt_att++;
					}
				else if (kind == MB_DATA_ATTITUDE1)
					{
					n_wf_att1++;
					n_wt_att1++;
					}
				else if (kind == MB_DATA_ATTITUDE2)
					{
					n_wf_att2++;
					n_wt_att2++;
					}
				else if (kind == MB_DATA_ATTITUDE3)
					{
					n_wf_att3++;
					n_wt_att3++;
					}
				}
				
			/* if requested output integrated nave */
			if (output_sensor_fnv == MB_YES
				&& status == MB_SUCCESS
				&& kind == MB_DATA_DATA)
				{
				/* save the current values */
				navlon_org = navlon;
				navlat_org = navlat;
				speed_org = speed;
				heading_org = heading;
				altitude_org = altitude;
				sensordepth_org = sensordepth;
				draft_org = draft;
				roll_org = roll;
				pitch_org = pitch;
				heave_org = heave;
				
				for (isensor=0; isensor < platform->num_sensors; isensor++)
					{
					if (platform->sensors[isensor].capability2 != 0)
						{
						for (ioffset=0; ioffset < platform->sensors[isensor].num_offsets; ioffset++)
							{
							if (platform->sensors[isensor].offsets[ioffset].ofp != NULL)
								{
								/* calculate position and attitude of target sensor */
								status = mb_platform_position (verbose, (void *)platform,
												isensor, ioffset,
												navlon_org, navlat_org, sensordepth_org,
												heading_org, roll_org, pitch_org,
												&navlon, &navlat, &sensordepth,
												&error);
								draft = sensordepth - heave;
								status = mb_platform_orientation_target (verbose, (void *)platform,
												isensor, ioffset,
												heading_org, roll_org, pitch_org,
												&heading, &roll, &pitch,
												&error);
		
								/* output integrated navigation */
								fprintf(platform->sensors[isensor].offsets[ioffset].ofp,
										"%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d\t%.6f\t%.10f\t%.10f\t%.3f\t%.3f\t%.4f\t%.3f\t%.3f\t%.3f\n",
										time_i[0], time_i[1], time_i[2], time_i[3],
										time_i[4], time_i[5], time_i[6], time_d,
										navlon, navlat,
										heading, speed, draft,
										roll, pitch, heave);
								}
							}
						}
					}
				}
			}
			
		/* end read+process+output data loop */
		/* --------------------------------- */
		
		/* output data counts */
		if (verbose > 0)
			{
			fprintf(stderr,"Pass 2: Records read from input file %s\n", ifile);
			fprintf(stderr,"     %d survey records\n", n_rf_data);
			fprintf(stderr,"     %d comment records\n", n_rf_comment);
			fprintf(stderr,"     %d nav records\n", n_rf_nav);
			fprintf(stderr,"     %d nav1 records\n", n_rf_nav1);
			fprintf(stderr,"     %d nav2 records\n", n_rf_nav2);
			fprintf(stderr,"     %d nav3 records\n", n_rf_nav3);
			fprintf(stderr,"     %d att records\n", n_rf_att);
			fprintf(stderr,"     %d att1 records\n", n_rf_att1);
			fprintf(stderr,"     %d att2 records\n", n_rf_att2);
			fprintf(stderr,"     %d att3 records\n", n_rf_att3);
			fprintf(stderr,"Pass 2: Records written to output file %s\n", ofile);
			fprintf(stderr,"     %d survey records\n", n_wf_data);
			fprintf(stderr,"     %d comment records\n", n_wf_comment);
			fprintf(stderr,"     %d nav records\n", n_wf_nav);
			fprintf(stderr,"     %d nav1 records\n", n_wf_nav1);
			fprintf(stderr,"     %d nav2 records\n", n_wf_nav2);
			fprintf(stderr,"     %d nav3 records\n", n_wf_nav3);
			fprintf(stderr,"     %d att records\n", n_wf_att);
			fprintf(stderr,"     %d att1 records\n", n_wf_att1);
			fprintf(stderr,"     %d att2 records\n", n_wf_att2);
			fprintf(stderr,"     %d att3 records\n", n_wf_att3);
			}
	
		/* close the input swath file */
		status = mb_close(verbose,&imbio_ptr,&error);

		/* close the output swath file */
		status = mb_close(verbose,&ombio_ptr,&error);
		
		/* close the synchronous attitude file */
		fclose(afp);
	
		/* generate inf fnv and fbt files */
		if (status == MB_SUCCESS)
			status = mb_make_info(verbose, MB_YES, ofile, oformat, &error);
			
		/* generate asynchronous heading file */
		if (heading_num > 0)
			{
			istart = 0;
			iend = heading_num - 1;
			for (i=0;i<heading_num;i++)
				{
				if (heading_time_d[i] < start_time_d)
					istart = i;
				if (heading_time_d[i] < end_time_d)
					iend = i;
				}
			if (iend > istart)
				{
				sprintf(afile,"%s.ath",ofile);
				if ((afp = fopen(afile, "w")) == NULL)
					{
					error = MB_ERROR_OPEN_FAIL;
					fprintf(stderr,"\nUnable to open asynchronous heading data file <%s> for writing\n",afile);
					fprintf(stderr,"\nProgram <%s> Terminated\n",
						program_name);
					exit(error);
					}
				fprintf(stderr, "Generating ath file for %s\n", ofile);
				for (i=0;i<heading_num;i++)
					{
					fprintf(afp, "%0.6f\t%7.3f\n", heading_time_d[i], heading_heading[i]);
					}
				fclose(afp);
				}
			}
			
		/* generate asynchronous sensordepth file */
		if (sensordepth_num > 0)
			{
			istart = 0;
			iend = sensordepth_num - 1;
			for (i=0;i<sensordepth_num;i++)
				{
				if (sensordepth_time_d[i] < start_time_d)
					istart = i;
				if (sensordepth_time_d[i] < end_time_d)
					iend = i;
				}
			if (iend > istart)
				{
				sprintf(afile,"%s.ats",ofile);
				if ((afp = fopen(afile, "w")) == NULL)
					{
					error = MB_ERROR_OPEN_FAIL;
					fprintf(stderr,"\nUnable to open asynchronous sensordepth data file <%s> for writing\n",afile);
					fprintf(stderr,"\nProgram <%s> Terminated\n",
						program_name);
					exit(error);
					}
				fprintf(stderr, "Generating ats file for %s\n", ofile);
				for (i=0;i<sensordepth_num;i++)
					{
					fprintf(afp, "%0.6f\t%7.3f\n", sensordepth_time_d[i], sensordepth_sensordepth[i]);
					}
				fclose(afp);
				}
			}
			
		/* generate asynchronous attitude file */
		if (attitude_num > 0)
			{
			istart = 0;
			iend = attitude_num - 1;
			for (i=0;i<attitude_num;i++)
				{
				if (attitude_time_d[i] < start_time_d)
					istart = i;
				if (attitude_time_d[i] < end_time_d)
					iend = i;
				}
			if (iend > istart)
				{
				sprintf(afile,"%s.ata",ofile);
				if ((afp = fopen(afile, "w")) == NULL)
					{
					error = MB_ERROR_OPEN_FAIL;
					fprintf(stderr,"\nUnable to open asynchronous attitude data file <%s> for writing\n",afile);
					fprintf(stderr,"\nProgram <%s> Terminated\n",
						program_name);
					exit(error);
					}
				fprintf(stderr, "Generating ata file for %s\n", ofile);
				for (i=0;i<attitude_num;i++)
					{
					fprintf(afp, "%0.6f\t%0.3f\t%0.3f\n", attitude_time_d[i], attitude_roll[i], attitude_pitch[i]);
					}
				fclose(afp);
				}
			}
	
		/* figure out whether and what to read next */
		if (read_datalist == MB_YES)
			{
			if ((status = mb_datalist_read(verbose,datalist,
				    ifile,&format,&file_weight,&error))
				    == MB_SUCCESS)
				read_data = MB_YES;
			else
				read_data = MB_NO;
			}
		else
			{
			read_data = MB_NO;
			}
	
		/* end loop over files in list */
		}
	if (read_datalist == MB_YES)
		mb_datalist_close(verbose,&datalist,&error);
	
	/* output data counts */
	if (verbose > 0)
		{
		fprintf(stderr,"\nPass 2: Total records read from all input files\n");
		fprintf(stderr,"     %d survey records\n", n_rt_data);
		fprintf(stderr,"     %d comment records\n", n_rt_comment);
		fprintf(stderr,"     %d nav records\n", n_rt_nav);
		fprintf(stderr,"     %d nav1 records\n", n_rt_nav1);
		fprintf(stderr,"     %d nav2 records\n", n_rt_nav2);
		fprintf(stderr,"     %d nav3 records\n", n_rt_nav3);
		fprintf(stderr,"     %d att records\n", n_rt_att);
		fprintf(stderr,"     %d att1 records\n", n_rt_att1);
		fprintf(stderr,"     %d att2 records\n", n_rt_att2);
		fprintf(stderr,"     %d att3 records\n", n_rt_att3);
		fprintf(stderr,"Pass 2: Total records written to all output files\n");
		fprintf(stderr,"     %d survey records\n", n_wt_data);
		fprintf(stderr,"     %d comment records\n", n_wt_comment);
		fprintf(stderr,"     %d nav records\n", n_wt_nav);
		fprintf(stderr,"     %d nav1 records\n", n_wt_nav1);
		fprintf(stderr,"     %d nav2 records\n", n_wt_nav2);
		fprintf(stderr,"     %d nav3 records\n", n_wt_nav3);
		fprintf(stderr,"     %d att records\n", n_wt_att);
		fprintf(stderr,"     %d att1 records\n", n_wt_att1);
		fprintf(stderr,"     %d att2 records\n", n_wt_att2);
		fprintf(stderr,"     %d att3 records\n", n_wt_att3);
		}
		
	/* end second pass through data */
	
	/*-------------------------------------------------------------------*/
	
	/* close any integrated navigation files */
	if (output_sensor_fnv == MB_YES)
		{
		for (isensor=0; isensor < platform->num_sensors; isensor++)
			{
			if (platform->sensors[isensor].capability2 != 0)
				{
				for (ioffset=0; ioffset < platform->sensors[isensor].num_offsets; ioffset++)
					{
					if (platform->sensors[isensor].offsets[ioffset].ofp != NULL)
						{
						fclose(platform->sensors[isensor].offsets[ioffset].ofp);
						platform->sensors[isensor].offsets[ioffset].ofp  = NULL;
						}
					}
				}
			}
		}

	/* deallocate nav, sensordepth, heading, attitude, and time_latency arrays */
	if (nav_alloc > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nav_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nav_navlon,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nav_navlat,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nav_speed,&error);
		}
	if (sensordepth_alloc > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&sensordepth_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&sensordepth_sensordepth,&error);
		}
	if (heading_alloc > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&heading_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&heading_heading,&error);
		}
	if (attitude_alloc > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&attitude_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&attitude_roll,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&attitude_pitch,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&attitude_heave,&error);
		}
	if (time_latency_alloc > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&time_latency_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&time_latency_time_latency,&error);
		}

	/* deallocate platform structure */
	if (platform != NULL)
		{
		status = mb_platform_deall(verbose, (void **)&platform, &error);
		}

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s> completed\n",
			program_name);
		fprintf(stderr,"dbg2  Ending status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/
