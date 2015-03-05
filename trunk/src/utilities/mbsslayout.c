/*--------------------------------------------------------------------
 *    The MB-system:	mbsslayout.c	1/8/2014
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
 * MBsslayout reads sidescan in raw time series form, lays the sidescan
 * out regularly sampled on a specified topography model, and outputs
 * the sidescan to format 71 (MBF_MBLDEOIH) files.
 *
 * Author:	D. W. Caress
 * Date:	April 21, 2014
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
#include "mbsys_ldeoih.h"

/* local defines */
#define MBSSLAYOUT_ALLOC_CHUNK 			1024
#define MBSSLAYOUT_ALLOC_NUM			128

#define	MBSSLAYOUT_LINE_OFF			0
#define	MBSSLAYOUT_LINE_TIME			1
#define	MBSSLAYOUT_LINE_ROUTE			2

#define MBSSLAYOUT_LAYOUT_FLATBOTTOM		0
#define MBSSLAYOUT_LAYOUT_3DTOPO		1
#define MBSSLAYOUT_ALTITUDE_ALTITUDE		0
#define MBSSLAYOUT_ALTITUDE_BOTTOMPICK		1
#define MBSSLAYOUT_ALTITUDE_TOPO_GRID		2
#define MBSSLAYOUT_GAIN_OFF			0
#define MBSSLAYOUT_GAIN_TVG			1
#define	MBSSLAYOUT_SWATHWIDTH_VARIABLE		0
#define	MBSSLAYOUT_SWATHWIDTH_CONSTANT		1

#define MBSSLAYOUT_MERGE_OFF 			0
#define MBSSLAYOUT_MERGE_FILE 			1
#define MBSSLAYOUT_MERGE_ASYNC 			2

#define MBSSLAYOUT_TIMESHIFT_OFF 		0
#define MBSSLAYOUT_TIMESHIFT_FILE 		1
#define MBSSLAYOUT_TIMESHIFT_CONSTANT 2
#define MBSSLAYOUT_TIMESHIFT_APPLY_NONE		0x00
#define MBSSLAYOUT_TIMESHIFT_APPLY_NAV		0x01
#define MBSSLAYOUT_TIMESHIFT_APPLY_SENSORDEPTH	0x02
#define MBSSLAYOUT_TIMESHIFT_APPLY_ALTITUDE	0x04
#define MBSSLAYOUT_TIMESHIFT_APPLY_HEADING		0x08
#define MBSSLAYOUT_TIMESHIFT_APPLY_ATTITUDE		0x10
#define MBSSLAYOUT_TIMESHIFT_APPLY_SOUNDSPEED		0x20
#define MBSSLAYOUT_TIMESHIFT_APPLY_UNUSED		0x40
#define MBSSLAYOUT_TIMESHIFT_APPLY_ALL_ANCILLIARY	0x7F
#define MBSSLAYOUT_TIMESHIFT_APPLY_SURVEY		0x80
#define MBSSLAYOUT_TIMESHIFT_APPLY_ALL		0xFF

#define MBSSLAYOUT_ROUTE_WAYPOINT_NONE		0
#define MBSSLAYOUT_ROUTE_WAYPOINT_SIMPLE		1
#define MBSSLAYOUT_ROUTE_WAYPOINT_TRANSIT		2
#define MBSSLAYOUT_ROUTE_WAYPOINT_STARTLINE	3
#define MBSSLAYOUT_ROUTE_WAYPOINT_ENDLINE		4
#define MBSSLAYOUT_ONLINE_THRESHOLD		15.0
#define MBSSLAYOUT_ONLINE_COUNT			30

#define MBSSLAYOUT_SSDIMENSION			4001

#define	MBSSLAYOUT_NUM_ANGLES			171
#define	MBSSLAYOUT_ANGLE_MAX			85.0

int mbsslayout_get_flatbottom_table(int verbose, int nangle, double angle_min, double angle_max,
					double navlon, double navlat, double altitude, double pitch,
					double *table_angle, double *table_xtrack, double *table_ltrack,
					double *table_altitude, double *table_range,
					int *error);

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	char program_name[] = "mbsslayout";
	char help_message[] =  "MBsslayout reads sidescan in raw time series form, lays the sidescan \nout regularly sampled on a specified topography model, and outputs \n the sidescan to format 71 (MBF_MBLDEOIH) files.\n";
	char usage_message[] = "mbsslayout [--verbose --help --input=datalist --format=format";
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
	/* mbsslayout --verbose
	 * 		--help
	 * 		--input=datalist
	 * 		--format=format
	 * 		
	 * 		--output_source=record_kind
	 * 		--output_name1=name
	 * 		--output_name2=name
	 * 		
	 * 		--line_nameroot=name
	 * 		--line_time_list=filename
	 * 		--line_route=filename
	 * 		--line_check_bearing
	 * 		--line_range_threshold=value
	 *
	 * 		--topo_grid_file=filename
	 * 		--altitude_altitude
	 * 		--altitude_bottomppick
	 * 		--altitude_bottompick_threshold=value
	 * 		--altitude_topo_grid
	 * 		--channel_swap
	 * 		--swath_width=value
	 * 		--gain=value
	 * 		--interpolation=value
	 * 		
	 * 		--nav_file=filename
	 * 		--nav_file_format=format_id
	 * 		--nav_async=record_kind
	 * 		--sensordepth_file=filename
	 * 		--sensordepth_file_format=format_id
	 * 		--sensordepth_async=record_kind
	 * 		--altitude_file=filename
	 * 		--altitude_file_format=format_id
	 * 		--altitude_async=record_kind
	 * 		--heading_file=filename
	 * 		--heading_file_format=format_id
	 * 		--heading_async=record_kind
	 * 		--attitude_file=filename
	 * 		--attitude_file_format=format_id
	 * 		--attitude_async=record_kind
	 * 		--soundspeed_constant=value
	 * 		--soundspeed_file=filename
	 * 		--soundspeed_file_format=format_id
	 * 		--soundspeed_async=record_kind
	 * 		--timeshift_file=filename
	 * 		--timeshift_constant=value
	 * 		--timeshift_apply_nav
	 * 		--timeshift_apply_sensordepth
	 * 		--timeshift_apply_altitude
	 * 		--timeshift_apply_heading
	 * 		--timeshift_apply_attitude
	 * 		--timeshift_apply_all_ancilliary
	 * 		--timeshift_apply_survey
	 * 		--timeshift_apply_all
	 * 		--sensor_offsets=filename
	 */
	static struct option options[] =
		{
		{"verbose",			no_argument, 		NULL, 		0},
		{"help",			no_argument, 		NULL, 		0},
		{"verbose",			no_argument, 		NULL, 		0},
		{"input",			required_argument, 	NULL, 		0},
		{"format",			required_argument, 	NULL, 		0},
		{"output_source",		required_argument, 	NULL, 		0},
		{"output_name1",		required_argument, 	NULL, 		0},
		{"output_name2",		required_argument, 	NULL, 		0},
		{"line_time_list",		required_argument, 	NULL, 		0},
		{"line_position_list",		required_argument, 	NULL, 		0},
		{"line_check_bearing",		no_argument, 		NULL, 		0},
		{"line_range_threshold",	required_argument, 	NULL, 		0},
		{"topo_grid_file",		required_argument, 	NULL, 		0},
		{"altitude_altitude",		no_argument, 		NULL, 		0},
		{"altitude_bottomppick",	no_argument, 		NULL, 		0},
		{"altitude_topo_grid",		no_argument, 		NULL, 		0},
		{"bottompick_threshold",	required_argument, 	NULL, 		0},
		{"channel_swap",		required_argument, 	NULL, 		0},
		{"swath_width",			required_argument, 	NULL, 		0},
		{"gain",			required_argument, 	NULL, 		0},
		{"interpolation",		required_argument, 	NULL, 		0},
		{"nav_file",			required_argument, 	NULL, 		0},
		{"nav_file_format",		required_argument, 	NULL, 		0},
		{"nav_async",			required_argument, 	NULL, 		0},
		{"sensordepth_file",		required_argument, 	NULL, 		0},
		{"sensordepth_file_format",	required_argument, 	NULL, 		0},
		{"sensordepth_async",		required_argument, 	NULL, 		0},
		{"altitude_file",		required_argument, 	NULL, 		0},
		{"altitude_file_format",	required_argument, 	NULL, 		0},
		{"altitude_async",		required_argument, 	NULL, 		0},
		{"heading_file",		required_argument, 	NULL, 		0},
		{"heading_file_format",		required_argument, 	NULL, 		0},
		{"heading_async",		required_argument, 	NULL, 		0},
		{"attitude_file",		required_argument, 	NULL, 		0},
		{"attitude_file_format",	required_argument, 	NULL, 		0},
		{"attitude_async",		required_argument, 	NULL, 		0},
		{"soundspeed_constant",		required_argument, 	NULL, 		0},
		{"soundspeed_file",		required_argument, 	NULL, 		0},
		{"soundspeed_file_format",	required_argument, 	NULL, 		0},
		{"soundspeed_async",		required_argument, 	NULL, 		0},
		{"timeshift_file",		required_argument, 	NULL, 		0},
		{"timeshift_constant",		required_argument, 	NULL, 		0},
		{"timeshift_apply_nav",		no_argument, 		NULL, 		0},
		{"timeshift_apply_sensordepth",	no_argument, 		NULL, 		0},
		{"timeshift_apply_altitude",	no_argument, 		NULL, 		0},
		{"timeshift_apply_heading",	no_argument, 		NULL, 		0},
		{"timeshift_apply_attitude",	no_argument, 		NULL, 		0},
		{"timeshift_apply_all_ancilliary",	no_argument, 	NULL, 		0},
		{"timeshift_apply_survey",	no_argument, 		NULL, 		0},
		{"timeshift_apply_all",		no_argument, 		NULL, 		0},
		{"sensor_offsets",		required_argument, 	NULL, 		0},
		{NULL,				0, 			NULL, 		0}
		};
		
	/* output variables */
	int	output_source = MB_DATA_NONE;
	mb_path	output_name1 = "Survey";
	mb_path	output_name2 = "sidescan";
	
	/* survey line variables */
	int	line_mode = MBSSLAYOUT_LINE_OFF;
	mb_path	line_time_list;
	mb_path	line_route;
	int	line_check_bearing = MB_NO;
	double	line_range_threshold = 50.0;
	
	/* sidescan layout variables */
	int	layout_mode = MBSSLAYOUT_LAYOUT_FLATBOTTOM;
	int	ss_altitude_mode = MBSSLAYOUT_ALTITUDE_ALTITUDE;
	mb_path	topo_grid_file;
	double	bottompick_threshold = 0.5;
	int	channel_swap = MB_NO;
	int	swath_mode = MBSSLAYOUT_SWATHWIDTH_VARIABLE;
	double	swath_width = 0.0;
	int	gain_mode = MBSSLAYOUT_GAIN_OFF;
	double	gain = 1.0;
	int	interpolation = 0;
	
	/* asynchronous navigation, heading, attitude data */
	int	nav_mode = MBSSLAYOUT_MERGE_OFF;
	mb_path	nav_file;
	int	nav_file_format = 0;
	int	nav_async = MB_DATA_DATA;
	int	nav_num = 0;
	int	nav_alloc = 0;
	double	*nav_time_d = NULL;
	double	*nav_navlon = NULL;
	double	*nav_navlat = NULL;
	double	*nav_speed = NULL;

	int	sensordepth_mode = MBSSLAYOUT_MERGE_OFF;
	mb_path	sensordepth_file;
	int	sensordepth_file_format = 0;
	int	sensordepth_async = MB_DATA_DATA;
	int	sensordepth_num = 0;
	int	sensordepth_alloc = 0;
	double	*sensordepth_time_d = NULL;
	double	*sensordepth_sensordepth = NULL;

	int	altitude_mode = MBSSLAYOUT_MERGE_OFF;
	mb_path	altitude_file;
	int	altitude_file_format = 0;
	int	altitude_async = MB_DATA_DATA;
	int	altitude_num = 0;
	int	altitude_alloc = 0;
	double	*altitude_time_d = NULL;
	double	*altitude_altitude = NULL;

	int	heading_mode = MBSSLAYOUT_MERGE_OFF;
	mb_path	heading_file;
	int	heading_file_format = 0;
	int	heading_async = MB_DATA_DATA;
	int	heading_num = 0;
	int	heading_alloc = 0;
	double	*heading_time_d = NULL;
	double	*heading_heading = NULL;

	int	attitude_mode = MBSSLAYOUT_MERGE_OFF;
	mb_path	attitude_file;
	int	attitude_file_format = 0;
	int	attitude_async = MB_DATA_DATA;
	int	attitude_num = 0;
	int	attitude_alloc = 0;
	double	*attitude_time_d = NULL;
	double	*attitude_roll = NULL;
	double	*attitude_pitch = NULL;
	double	*attitude_heave = NULL;

	int	soundspeed_mode = MBSSLAYOUT_MERGE_OFF;
	double	soundspeed_constant = 1500.0;
	mb_path	soundspeed_file;
	int	soundspeed_file_format = 0;
	int	soundspeed_async = MB_DATA_DATA;
	int	soundspeed_num = 0;
	int	soundspeed_alloc = 0;
	double	*soundspeed_time_d = NULL;
	double	*soundspeed_soundspeed = NULL;

	int	timeshift_mode = MBSSLAYOUT_TIMESHIFT_OFF;
	mb_u_char timeshift_apply = MBSSLAYOUT_TIMESHIFT_APPLY_NONE;
	mb_path	timeshift_file;
	int	timeshift_format = 0;
	int	timeshift_num = 0;
	int	timeshift_alloc = 0;
	double	*timeshift_time_d = NULL;
	double	*timeshift_timeshift = NULL;
	double	timeshift_constant = 0.0;

	/* MBIO read control parameters */
	int	read_datalist = MB_NO;
	int	read_data;
	mb_path	read_file;
	mb_path	output_file;
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_UNSET;
	double	file_weight;
	int	format = 0;
	int	iformat;
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
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;

	/* MBIO read values */
	void	*imbio_ptr = NULL;
	void	*ombio_ptr = NULL;
	struct mb_io_struct *omb_io_ptr;
	void	*istore_ptr = NULL;
	void	*ostore_ptr = NULL;
	struct mbsys_ldeoih_struct *ostore;

	int	kind;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	double	altitude;
	double	sensordraft;
	double	sensordepth;
	double	draft;
	double	roll;
	double	pitch;
	double	heave;
	double	soundspeed;
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
	double	ss_altitude;

	/* arrays for asynchronous data accessed using mb_extract_nnav() */
	int	nanavmax = MB_NAV_MAX;
	int	nanav;
	int	antime_i[7*MB_NAV_MAX];
	double	antime_d[MB_NAV_MAX];
	double	anlon[MB_NAV_MAX];
	double	anlat[MB_NAV_MAX];
	double	anspeed[MB_NAV_MAX];
	double	anheading[MB_NAV_MAX];
	double	ansensordraft[MB_NAV_MAX];
	double	anroll[MB_NAV_MAX];
	double	anpitch[MB_NAV_MAX];
	double	anheave[MB_NAV_MAX];

	/* arrays for asynchronous data accessed using mb_ctd() */
	int	nactd;
	double	actime_d[MB_CTD_MAX];
	double	acconductivity[MB_CTD_MAX];
	double	actemperature[MB_CTD_MAX];
	double	acdepth[MB_CTD_MAX];
	double	acsalinity[MB_CTD_MAX];
	double	acsoundspeed[MB_CTD_MAX];
	
	/* raw sidescan */
	int	sidescan_type = MB_SIDESCAN_LINEAR;
	double	sample_interval;
	double	beamwidth_xtrack = 0.0;
	double	beamwidth_ltrack = 0.0;
	int	num_samples_port = 0;
	int	num_samples_port_alloc = 0;
	double	*raw_samples_port = NULL;
	int	num_samples_stbd = 0;
	int	num_samples_stbd_alloc = 0;
	double	*raw_samples_stbd = NULL;

	/* bottom layout parameters */
	int	nangle = MBSSLAYOUT_NUM_ANGLES;
	double	angle_min = -MBSSLAYOUT_ANGLE_MAX;
	double	angle_max = MBSSLAYOUT_ANGLE_MAX;
	double	table_angle[MBSSLAYOUT_NUM_ANGLES];
	double	table_xtrack[MBSSLAYOUT_NUM_ANGLES];
	double	table_ltrack[MBSSLAYOUT_NUM_ANGLES];
	double	table_altitude[MBSSLAYOUT_NUM_ANGLES];
	double	table_range[MBSSLAYOUT_NUM_ANGLES];

	/* output sidescan data */
	int	obeams_bath;
	int	obeams_amp;
	int	opixels_ss;
	double	oss[MBSSLAYOUT_SSDIMENSION];
	double	ossacrosstrack[MBSSLAYOUT_SSDIMENSION];
	double	ossalongtrack[MBSSLAYOUT_SSDIMENSION];
	int	ossbincount[MBSSLAYOUT_SSDIMENSION];
	double	pixel_width;
	
	/* counts of records read and written */
	int	n_rf_data = 0;
	int	n_rf_comment = 0;
	int	n_rf_ss2 = 0;
	int	n_rf_ss3 = 0;
	int	n_rf_sbp = 0;
	int	n_rf_nav = 0;
	int	n_rf_nav1 = 0;
	int	n_rf_nav2 = 0;
	int	n_rf_nav3 = 0;
	
	int	n_rt_data = 0;
	int	n_rt_comment = 0;
	int	n_rt_ss2 = 0;
	int	n_rt_ss3 = 0;
	int	n_rt_sbp = 0;
	int	n_rt_nav = 0;
	int	n_rt_nav1 = 0;
	int	n_rt_nav2 = 0;
	int	n_rt_nav3 = 0;
	
	int	n_wf_data = 0;
	int	n_wf_comment = 0;
	int	n_wt_data = 0;
	int	n_wt_comment = 0;
	
	mb_path	command;
	int	interp_status = MB_SUCCESS;
	int	interp_error = MB_ERROR_NO_ERROR;
	int	shellstatus;
	double	timeshift;
	int	jsurvey = 0;
	int	jnav = 0;
	int	jsensordepth = 0;
	int	jaltitude = 0;
	int	jheading = 0;
	int	jattitude = 0;
	int	jsoundspeed = 0;
	int	data_changed;
	int	new_output_file = MB_NO;
	int	rawroutefile = MB_NO;
	int	oktowrite = MB_NO;
	int	point_ok = MB_NO;
	int	linechange = MB_NO;
	int	line_number = 0;
	int	nget;
	int	waypoint;
	int	activewaypoint = -1;
	double	topo;
	double	rangelast;
	int	ntimepoint = 0;
	int	ntimepointalloc = 0;
	int	nroutepoint = 0;
	int	nroutepointalloc = 0;
	double	*routelon = NULL;
	double	*routelat = NULL;
	double	*routeheading = NULL;
	double	*routetime_d = NULL;
	int	*routewaypoint = NULL;
	double	mtodeglon, mtodeglat;
	void	*topogrid_ptr = NULL;
	mb_path	scriptfile;
	char	*result = NULL;
	FILE	*fp, *sfp;
	double	dx, dy, range;
	double	channelmax, threshold, ttime;
	int	portchannelpick, stbdchannelpick;
	int	kangle, kstart;
	double	xtrack, ltrack, rr, rangemin, factor, fraction;
	int	done, found;
	int	istart;
	int	jport, jstbd;
	int	previous, interpable;
	double	dss, dssl;
	
	int	i, j, jj, n;

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
				
			/*-------------------------------------------------------
			 * Define source data */
			
			/* output_source */
			else if (strcmp("output_source", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &output_source);
				}
			
			/* output_name1 */
			else if (strcmp("output_name1", options[option_index].name) == 0)
				{
				strcpy(output_name1, optarg);
				}
			
			/* output_name2 */
			else if (strcmp("output_name2", options[option_index].name) == 0)
				{
				strcpy(output_name2, optarg);
				}
												
			/*-------------------------------------------------------
			 * Define survey line specification */
			
			/* line_time_list */
			else if (strcmp("line_time_list", options[option_index].name) == 0)
				{
				strcpy(line_time_list, optarg);
				line_mode = MBSSLAYOUT_LINE_TIME;
				}
			
			/* line_route */
			else if (strcmp("line_route", options[option_index].name) == 0)
				{
				strcpy(line_route, optarg);
				line_mode = MBSSLAYOUT_LINE_ROUTE;
				}
			
			/* line_check_bearing */
			else if (strcmp("line_check_bearing", options[option_index].name) == 0)
				{
				line_check_bearing = MB_YES;
				}
							
			/*-------------------------------------------------------
			 * Define sidescan layout algorithm parameters */
			
			/* topo_grid_file */
			else if (strcmp("topo_grid_file", options[option_index].name) == 0)
				{
				strcpy(topo_grid_file, optarg);
				layout_mode = MBSSLAYOUT_LAYOUT_3DTOPO;
				}
			
			/* altitude_altitude */
			else if (strcmp("altitude_altitude", options[option_index].name) == 0)
				{
				ss_altitude_mode = MBSSLAYOUT_ALTITUDE_ALTITUDE;
				}
			
			/* altitude_bottomppick */
			else if (strcmp("altitude_bottomppick", options[option_index].name) == 0)
				{
				ss_altitude_mode = MBSSLAYOUT_ALTITUDE_BOTTOMPICK;
				}
			
			/* bottompick_threshold */
			else if (strcmp("bottompick_threshold", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%lf", &bottompick_threshold);
				ss_altitude_mode = MBSSLAYOUT_ALTITUDE_BOTTOMPICK;
				}
			
			/* altitude_topo_grid */
			else if (strcmp("altitude_topo_grid", options[option_index].name) == 0)
				{
				ss_altitude_mode = MBSSLAYOUT_ALTITUDE_TOPO_GRID;
				}
			
			/* channel_swap */
			else if (strcmp("channel_swap", options[option_index].name) == 0)
				{
				channel_swap = MB_YES;
				}
			
			/* swath_width */
			else if (strcmp("swath_width", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%lf", &swath_width);
				swath_mode = MBSSLAYOUT_SWATHWIDTH_CONSTANT;
				}
			
			/* gain */
			else if (strcmp("gain", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%lf", &gain);
				gain_mode = MBSSLAYOUT_GAIN_TVG;
				}
			
			/* interpolation */
			else if (strcmp("interpolation", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &interpolation);
				}
			
			/*-------------------------------------------------------
			 * Define source of navigation - could be an external file
			 * or an internal asynchronous record */
			
			/* nav_file */
			else if (strcmp("nav_file", options[option_index].name) == 0)
				{
				strcpy(nav_file, optarg);
				nav_mode = MBSSLAYOUT_MERGE_FILE;
				}
			
			/* nav_file_format */
			else if (strcmp("nav_file_format", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &nav_file_format);
				}
			
			/* nav_async */
			else if (strcmp("nav_async", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &nav_async);
				if (n == 1)
					nav_mode = MBSSLAYOUT_MERGE_ASYNC;
				}
				
			/*-------------------------------------------------------
			 * Define source of sensordepth - could be an external file
			 * or an internal asynchronous record */
			
			/* sensordepth_file */
			else if (strcmp("sensordepth_file", options[option_index].name) == 0)
				{
				strcpy(sensordepth_file, optarg);
				sensordepth_mode = MBSSLAYOUT_MERGE_FILE;
				}
			
			/* sensordepth_file_format */
			else if (strcmp("sensordepth_file_format", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &sensordepth_file_format);
				}
			
			/* sensordepth_async */
			else if (strcmp("sensordepth_async", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &sensordepth_async);
				if (n == 1)
					sensordepth_mode = MBSSLAYOUT_MERGE_ASYNC;
				}
				
			/*-------------------------------------------------------
			 * Define source of altitude - could be an external file
			 * or an internal asynchronous record */
			
			/* altitude_file */
			else if (strcmp("altitude_file", options[option_index].name) == 0)
				{
				strcpy(altitude_file, optarg);
				altitude_mode = MBSSLAYOUT_MERGE_FILE;
				}
			
			/* altitude_file_format */
			else if (strcmp("altitude_file_format", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &altitude_file_format);
				}
			
			/* altitude_async */
			else if (strcmp("altitude_async", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &altitude_async);
				if (n == 1)
					altitude_mode = MBSSLAYOUT_MERGE_ASYNC;
				}
				
			/*-------------------------------------------------------
			 * Define source of heading - could be an external file
			 * or an internal asynchronous record */
			
			/* heading_file */
			else if (strcmp("heading_file", options[option_index].name) == 0)
				{
				strcpy(heading_file, optarg);
				heading_mode = MBSSLAYOUT_MERGE_FILE;
				}
			
			/* heading_file_format */
			else if (strcmp("heading_file_format", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &heading_file_format);
				}
			
			/* heading_async */
			else if (strcmp("heading_async", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &heading_async);
				if (n == 1)
					heading_mode = MBSSLAYOUT_MERGE_ASYNC;
				}
				
			/*-------------------------------------------------------
			 * Define source of attitude - could be an external file
			 * or an internal asynchronous record */
			
			/* attitude_file */
			else if (strcmp("attitude_file", options[option_index].name) == 0)
				{
				strcpy(attitude_file, optarg);
				attitude_mode = MBSSLAYOUT_MERGE_FILE;
				}
			
			/* attitude_file_format */
			else if (strcmp("attitude_file_format", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &attitude_file_format);
				}
			
			/* attitude_async */
			else if (strcmp("attitude_async", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &attitude_async);
				if (n == 1)
					attitude_mode = MBSSLAYOUT_MERGE_ASYNC;
				}
				
			/*-------------------------------------------------------
			 * Define source of sound speed - could be an external file
			 * or an internal asynchronous record */
			
			/* soundspeed_constant */
			else if (strcmp("soundspeed_constant", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%lf", &soundspeed_constant);
				soundspeed_mode = MBSSLAYOUT_MERGE_OFF;
				}
			
			/* soundspeed_file */
			else if (strcmp("soundspeed_file", options[option_index].name) == 0)
				{
				strcpy(soundspeed_file, optarg);
				soundspeed_mode = MBSSLAYOUT_MERGE_FILE;
				}
			
			/* soundspeed_file_format */
			else if (strcmp("soundspeed_file_format", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &soundspeed_file_format);
				}
			
			/* soundspeed_async */
			else if (strcmp("soundspeed_async", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%d", &soundspeed_async);
				if (n == 1)
					soundspeed_mode = MBSSLAYOUT_MERGE_ASYNC;
				}
				
			/*-------------------------------------------------------
			 * Define source of timeshift - could be an external file
			 * or single value. Also define which data the timeshift model
			 * will be applied to - nav, sensordepth, heading, attitude,
			 * or all. */
			
			/* timeshift_file */
			else if (strcmp("timeshift_file", options[option_index].name) == 0)
				{
				strcpy(timeshift_file, optarg);
				timeshift_mode = MBSSLAYOUT_TIMESHIFT_FILE;
				}
			
			/* timeshift_constant */
			else if (strcmp("timeshift_constant", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%lf", &timeshift_constant);
				if (n == 1)
					timeshift_mode = MBSSLAYOUT_TIMESHIFT_CONSTANT;
				}
			
			/* timeshift_apply_nav */
			else if (strcmp("timeshift_apply_nav", options[option_index].name) == 0)
				{
				timeshift_apply =  timeshift_apply | MBSSLAYOUT_TIMESHIFT_APPLY_NAV;
				}
			
			/* timeshift_apply_sensordepth */
			else if (strcmp("timeshift_apply_sensordepth", options[option_index].name) == 0)
				{
				timeshift_apply =  timeshift_apply | MBSSLAYOUT_TIMESHIFT_APPLY_SENSORDEPTH;
				}
			
			/* timeshift_apply_altitude */
			else if (strcmp("timeshift_apply_altitude", options[option_index].name) == 0)
				{
				timeshift_apply =  timeshift_apply | MBSSLAYOUT_TIMESHIFT_APPLY_ALTITUDE;
				}
			
			/* timeshift_apply_heading */
			else if (strcmp("timeshift_apply_heading", options[option_index].name) == 0)
				{
				timeshift_apply =  timeshift_apply | MBSSLAYOUT_TIMESHIFT_APPLY_HEADING;
				}
			
			/* timeshift_apply_attitude */
			else if (strcmp("timeshift_apply_attitude", options[option_index].name) == 0)
				{
				timeshift_apply =  timeshift_apply | MBSSLAYOUT_TIMESHIFT_APPLY_ATTITUDE;
				}
			
			/* timeshift_apply_soundspeed */
			else if (strcmp("timeshift_apply_soundspeed", options[option_index].name) == 0)
				{
				timeshift_apply =  timeshift_apply | MBSSLAYOUT_TIMESHIFT_APPLY_SOUNDSPEED;
				}
			
			/* timeshift_apply_all_ancilliary */
			else if (strcmp("timeshift_apply_all_ancilliary", options[option_index].name) == 0)
				{
				timeshift_apply =  MBSSLAYOUT_TIMESHIFT_APPLY_ALL_ANCILLIARY;
				}
			
			/* timeshift_apply_survey */
			else if (strcmp("timeshift_apply_survey", options[option_index].name) == 0)
				{
				timeshift_apply =  MBSSLAYOUT_TIMESHIFT_APPLY_SURVEY;
				}
			
			/* timeshift_apply_all */
			else if (strcmp("timeshift_apply_all", options[option_index].name) == 0)
				{
				timeshift_apply =  MBSSLAYOUT_TIMESHIFT_APPLY_ALL;
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
		fprintf(stderr,"dbg2  Default MB-System Parameters:\n");
		fprintf(stderr,"dbg2       verbose:                    %d\n",verbose);
		fprintf(stderr,"dbg2       help:                       %d\n",help);
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
		fprintf(stderr,"dbg2  Data Input Parameters:\n");
		fprintf(stderr,"dbg2       read_file:                  %s\n",read_file);
		fprintf(stderr,"dbg2       format:                     %d\n",format);
		fprintf(stderr,"dbg2  Source Data Parameters:\n");
		fprintf(stderr,"dbg2       output_source:              %d\n",output_source);
		fprintf(stderr,"dbg2       output_name1:               %s\n",output_name1);
		fprintf(stderr,"dbg2       output_name2:               %s\n",output_name2);
		fprintf(stderr,"dbg2  Survey Line Parameters:\n");
		fprintf(stderr,"dbg2       line_mode:                  %d\n",line_mode);
		fprintf(stderr,"dbg2       line_time_list:             %s\n",line_time_list);
		fprintf(stderr,"dbg2       line_route:                 %s\n",line_route);
		fprintf(stderr,"dbg2       line_check_bearing:         %d\n",line_check_bearing);
		fprintf(stderr,"dbg2       line_range_threshold:       %f\n",line_range_threshold);
		fprintf(stderr,"dbg2  Sidescan Layout Algorithm Parameters:\n");
		fprintf(stderr,"dbg2       layout_mode:                %d\n",layout_mode);
		fprintf(stderr,"dbg2       topo_grid_file:             %s\n",topo_grid_file);
		fprintf(stderr,"dbg2       ss_altitude_mode:           %d\n",ss_altitude_mode);
		fprintf(stderr,"dbg2       bottompick_threshold:       %f\n",bottompick_threshold);
		fprintf(stderr,"dbg2       channel_swap:               %d\n",channel_swap);
		fprintf(stderr,"dbg2       swath_mode:                 %d\n",swath_mode);
		fprintf(stderr,"dbg2       swath_width:                %f\n",swath_width);
		fprintf(stderr,"dbg2       gain_mode:                  %d\n",gain_mode);
		fprintf(stderr,"dbg2       gain:                       %f\n",gain);
		fprintf(stderr,"dbg2       interpolation:              %d\n",interpolation);
		fprintf(stderr,"dbg2  Navigation Source Parameters:\n");
		fprintf(stderr,"dbg2       nav_mode:                   %d\n",nav_mode);
		fprintf(stderr,"dbg2       nav_file:                   %s\n",nav_file);
		fprintf(stderr,"dbg2       nav_file_format:            %d\n",nav_file_format);
		fprintf(stderr,"dbg2       nav_async:                  %d\n",nav_async);
		fprintf(stderr,"dbg2  Sensor Depth Source Parameters:\n");
		fprintf(stderr,"dbg2       sensordepth_mode:           %d\n",sensordepth_mode);
		fprintf(stderr,"dbg2       sensordepth_file:           %s\n",sensordepth_file);
		fprintf(stderr,"dbg2       sensordepth_file_format:    %d\n",sensordepth_file_format);
		fprintf(stderr,"dbg2       sensordepth_async:          %d\n",sensordepth_async);
		fprintf(stderr,"dbg2  Altitude Source Parameters:\n");
		fprintf(stderr,"dbg2       altitude_mode:              %d\n",altitude_mode);
		fprintf(stderr,"dbg2       altitude_file:              %s\n",altitude_file);
		fprintf(stderr,"dbg2       altitude_file_format:       %d\n",altitude_file_format);
		fprintf(stderr,"dbg2       altitude_async:             %d\n",altitude_async);
		fprintf(stderr,"dbg2  Heading Source Parameters:\n");
		fprintf(stderr,"dbg2       heading_mode:               %d\n",heading_mode);
		fprintf(stderr,"dbg2       heading_file:               %s\n",heading_file);
		fprintf(stderr,"dbg2       heading_file_format:        %d\n",heading_file_format);
		fprintf(stderr,"dbg2       heading_async:              %d\n",heading_async);
		fprintf(stderr,"dbg2  Attitude Source Parameters:\n");
		fprintf(stderr,"dbg2       attitude_mode:              %d\n",attitude_mode);
		fprintf(stderr,"dbg2       attitude_file:              %s\n",attitude_file);
		fprintf(stderr,"dbg2       attitude_file_format:       %d\n",attitude_file_format);
		fprintf(stderr,"dbg2       attitude_async:             %d\n",attitude_async);
		fprintf(stderr,"dbg2  Sound Speed Source Parameters:\n");
		fprintf(stderr,"dbg2       soundspeed_mode:            %d\n",soundspeed_mode);
		fprintf(stderr,"dbg2       soundspeed_constant:        %f\n",soundspeed_constant);
		fprintf(stderr,"dbg2       soundspeed_file:            %s\n",soundspeed_file);
		fprintf(stderr,"dbg2       soundspeed_file_format:     %d\n",soundspeed_file_format);
		fprintf(stderr,"dbg2       soundspeed_async:           %d\n",soundspeed_async);
		fprintf(stderr,"dbg2  Time Shift Source Parameters:\n");
		fprintf(stderr,"dbg2       timeshift_mode:             %d\n",timeshift_mode);
		fprintf(stderr,"dbg2       timeshift_file:             %s\n",timeshift_file);
		fprintf(stderr,"dbg2       timeshift_format:           %d\n",timeshift_format);
		fprintf(stderr,"dbg2       timeshift_constant:         %f\n",timeshift_constant);
		fprintf(stderr,"dbg2       timeshift_apply:            %x\n",timeshift_apply);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}
		
	/* print starting debug statements */
	if (verbose == 1)
		{
		fprintf(stderr,"\nProgram <%s>\n",program_name);
		fprintf(stderr,"Version %s\n",version_id);
		fprintf(stderr,"MB-system Version %s\n\n",MB_VERSION);
		fprintf(stderr,"Data Input Parameters:\n");
		fprintf(stderr,"     read_file:                  %s\n",read_file);
		fprintf(stderr,"     format:                     %d\n",format);
		fprintf(stderr,"Output Channel Parameters:\n");
		if (output_source != MB_DATA_NONE)
			{
			fprintf(stderr,"     output_source:            %d\n",output_source);
			fprintf(stderr,"     output_name1:             %s\n",output_name1);
			fprintf(stderr,"     output_name2:             %s\n",output_name2);
			}
		fprintf(stderr,"Survey Line Parameters:\n");
		if (line_mode == MBSSLAYOUT_LINE_OFF)
			{
			fprintf(stderr,"     line_mode:                Data not recast into survey lines.\n");
			}
		else if (line_mode == MBSSLAYOUT_LINE_TIME)
			{
			fprintf(stderr,"     line_mode:                Lines defined by waypoint time list.\n");
			fprintf(stderr,"     line_time_list:           %s\n",line_time_list);
			fprintf(stderr,"     line_check_bearing:       %d\n",line_check_bearing);
			
			}
		else if (line_mode == MBSSLAYOUT_LINE_ROUTE)
			{
			fprintf(stderr,"     line_mode:                Lines defined by route waypoint position list.\n");
			fprintf(stderr,"     line_route:               %s\n",line_route);
			fprintf(stderr,"     line_check_bearing:       %d\n",line_check_bearing);
			}
		fprintf(stderr,"Sidescan Layout Algorithm Parameters:\n");
		if (layout_mode == MBSSLAYOUT_LAYOUT_FLATBOTTOM)
			{
			fprintf(stderr,"     layout_mode:              Flat bottom layout using altitude\n");	
			}
		else if (layout_mode == MBSSLAYOUT_LAYOUT_3DTOPO)
			{
			fprintf(stderr,"     layout_mode:              3D layout using topography model\n");	
			fprintf(stderr,"     topo_grid_file:           %s\n",topo_grid_file);
			}
		if (ss_altitude_mode == MBSSLAYOUT_ALTITUDE_ALTITUDE)
			{
			fprintf(stderr,"     ss_altitude_mode:         Existing altitude value used\n");
			}
		else if (ss_altitude_mode == MBSSLAYOUT_ALTITUDE_BOTTOMPICK)
			{
			fprintf(stderr,"     ss_altitude_mode:         Altitude calculated using bottom pick in time series\n");
			fprintf(stderr,"     bottompick_threshold:     %f\n",bottompick_threshold);
			}
		else if (layout_mode == MBSSLAYOUT_ALTITUDE_TOPO_GRID)
			{
			fprintf(stderr,"     ss_altitude_mode:         Altitude calculated during 3D layout on topography model\n");
			}
		if (channel_swap == MB_YES)
			fprintf(stderr,"     channel_swap:             Swapping port and starboard\n");
		else
			fprintf(stderr,"     channel_swap:             No swap\n");
		if (swath_mode == MBSSLAYOUT_SWATHWIDTH_CONSTANT)
			{
			fprintf(stderr,"     swath_mode:               Constant swath width\n");
			fprintf(stderr,"     swath_width:              %f\n",swath_width);
			}
		else
			{
			fprintf(stderr,"     swath_mode:               Variable swath width\n");
			}
		if (gain_mode == MBSSLAYOUT_SWATHWIDTH_CONSTANT)
			{
			fprintf(stderr,"     gain_mode:                Gain applied\n");
			fprintf(stderr,"     gain:                     %f\n",gain);
			}
		else
			{
			fprintf(stderr,"     gain_mode:                Gain not applied\n");
			}
		fprintf(stderr,"     interpolation:            %d\n",interpolation);
		fprintf(stderr,"Navigation Source Parameters:\n");
		if (nav_mode == MBSSLAYOUT_MERGE_OFF)
			{
			fprintf(stderr,"     nav_mode:                   No navigation merging\n");
			}
		else if (nav_mode == MBSSLAYOUT_MERGE_FILE)
			{
			fprintf(stderr,"     nav_mode:                   Navigation merged from external file\n");
			fprintf(stderr,"     nav_file:                   %s\n",nav_file);
			fprintf(stderr,"     nav_file_format:            %d\n",nav_file_format);
			}
		else if (nav_mode == MBSSLAYOUT_MERGE_ASYNC)
			{
			fprintf(stderr,"     nav_mode:                   Navigation merged from asynchronous data records\n");
			fprintf(stderr,"     nav_async:                  %d\n",nav_async);
			}

		fprintf(stderr,"Sensor Depth Source Parameters:\n");
		if (sensordepth_mode == MBSSLAYOUT_MERGE_OFF)
			{
			fprintf(stderr,"     sensordepth_mode:           No sensor depth merging\n");
			}
		else if (sensordepth_mode == MBSSLAYOUT_MERGE_FILE)
			{
			fprintf(stderr,"     sensordepth_mode:           Sensor depth merged from external file\n");
			fprintf(stderr,"     sensordepth_file:           %s\n",sensordepth_file);
			fprintf(stderr,"     sensordepth_file_format:    %d\n",sensordepth_file_format);
			}
		else if (sensordepth_mode == MBSSLAYOUT_MERGE_ASYNC)
			{
			fprintf(stderr,"     sensordepth_mode:           Sensor depth merged from asynchronous data records\n");
			fprintf(stderr,"     sensordepth_async:          %d\n",sensordepth_async);
			}

		fprintf(stderr,"Altitude Source Parameters:\n");
		if (altitude_mode == MBSSLAYOUT_MERGE_OFF)
			{
			fprintf(stderr,"     altitude_mode:              No altitude merging\n");
			}
		else if (altitude_mode == MBSSLAYOUT_MERGE_FILE)
			{
			fprintf(stderr,"     altitude_mode:              Altitude merged from external file\n");
			fprintf(stderr,"     altitude_file:              %s\n",altitude_file);
			fprintf(stderr,"     altitude_file_format:       %d\n",altitude_file_format);
			}
		else if (altitude_mode == MBSSLAYOUT_MERGE_ASYNC)
			{
			fprintf(stderr,"     altitude_mode:              Altitude merged from asynchronous data records\n");
			fprintf(stderr,"     altitude_async:             %d\n",altitude_async);
			}

		fprintf(stderr,"Heading Source Parameters:\n");
		if (heading_mode == MBSSLAYOUT_MERGE_OFF)
			{
			fprintf(stderr,"     heading_mode:               No heading merging\n");
			}
		else if (heading_mode == MBSSLAYOUT_MERGE_FILE)
			{
			fprintf(stderr,"     heading_mode:               Heading merged from external file\n");
			fprintf(stderr,"     heading_file:               %s\n",heading_file);
			fprintf(stderr,"     heading_file_format:        %d\n",heading_file_format);
			}
		else if (heading_mode == MBSSLAYOUT_MERGE_ASYNC)
			{
			fprintf(stderr,"     heading_mode:               Heading merged from asynchronous data records\n");
			fprintf(stderr,"     heading_async:              %d\n",heading_async);
			}

		fprintf(stderr,"Attitude Source Parameters:\n");
		if (attitude_mode == MBSSLAYOUT_MERGE_OFF)
			{
			fprintf(stderr,"     attitude_mode:              No attitude merging\n");
			}
		else if (attitude_mode == MBSSLAYOUT_MERGE_FILE)
			{
			fprintf(stderr,"     attitude_mode:              Attitude merged from external file\n");
			fprintf(stderr,"     attitude_file:              %s\n",attitude_file);
			fprintf(stderr,"     attitude_file_format:       %d\n",attitude_file_format);
			}
		else if (attitude_mode == MBSSLAYOUT_MERGE_ASYNC)
			{
			fprintf(stderr,"     attitude_mode:              Attitude merged from asynchronous data records\n");
			fprintf(stderr,"     attitude_async:             %d\n",attitude_async);
			}
		fprintf(stderr,"Sound Speed Source Parameters:\n");
		if (soundspeed_mode == MBSSLAYOUT_MERGE_OFF)
			{
			fprintf(stderr,"     soundspeed_mode:            No sound speed merging, constant value\n");
			fprintf(stderr,"     soundspeed_constant:        %f meters/second\n",soundspeed_constant);
			}
		else if (soundspeed_mode == MBSSLAYOUT_MERGE_FILE)
			{
			fprintf(stderr,"     soundspeed_mode:            Sound speed merged from external file\n");
			fprintf(stderr,"     soundspeed_file:            %s\n",soundspeed_file);
			fprintf(stderr,"     soundspeed_file_format:     %d\n",soundspeed_file_format);
			}
		else if (soundspeed_mode == MBSSLAYOUT_MERGE_ASYNC)
			{
			fprintf(stderr,"     soundspeed_mode:            Sound speed merged from asynchronous data records\n");
			fprintf(stderr,"     soundspeed_async:           %d\n",soundspeed_async);
			}
		fprintf(stderr,"Time Shift Source Parameters:\n");
		if (timeshift_mode == MBSSLAYOUT_TIMESHIFT_OFF)
			{
			fprintf(stderr,"     timeshift_mode:             No time shift\n");
			}
		else if (timeshift_mode == MBSSLAYOUT_TIMESHIFT_FILE)
			{
			fprintf(stderr,"     timeshift_mode:             Time shift model read from external file\n");
			fprintf(stderr,"     timeshift_file:             %s\n",timeshift_file);
			fprintf(stderr,"     timeshift_format:           %d\n",timeshift_format);
			}
		else if (timeshift_mode == MBSSLAYOUT_TIMESHIFT_CONSTANT)
			{
			fprintf(stderr,"     timeshift_mode:             Constant time shift\n");
			fprintf(stderr,"     timeshift_constant:         %f\n",timeshift_constant);
			}
		fprintf(stderr,"\n");
		}

	/* read topography grid if 3D bottom correction specified */
	if (layout_mode == MBSSLAYOUT_LAYOUT_3DTOPO)
		{
		status = mb_topogrid_init(verbose, topo_grid_file, &lonflip, &topogrid_ptr, &error);
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error loading topography grid: %s\n%s\n", topo_grid_file, message);
			fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
			}
		}

	/* load ancilliary data from external files if requested */
	if (nav_mode == MBSSLAYOUT_MERGE_FILE)
		{
		mb_loadnavdata(verbose, nav_file, nav_file_format, lonflip,
			       &nav_num, &nav_alloc,
			       &nav_time_d, &nav_navlon, &nav_navlat, &nav_speed, &error);
		
		if (verbose > 0)
			fprintf(stderr,"%d navigation records loaded from file %s\n", nav_num, nav_file);
		}
	if (sensordepth_mode == MBSSLAYOUT_MERGE_FILE)
		{
		mb_loadsensordepthdata(verbose, sensordepth_file, sensordepth_file_format,
				       &sensordepth_num, &sensordepth_alloc,
			       &sensordepth_time_d, &sensordepth_sensordepth, &error);
		
		if (verbose > 0)
			fprintf(stderr,"%d sensordepth records loaded from file %s\n", sensordepth_num, sensordepth_file);
		}
	if (altitude_mode == MBSSLAYOUT_MERGE_FILE)
		{
		mb_loadaltitudedata(verbose, altitude_file, altitude_file_format,
				       &altitude_num, &altitude_alloc,
			       &altitude_time_d, &altitude_altitude, &error);
		
		if (verbose > 0)
			fprintf(stderr,"%d altitude records loaded from file %s\n", altitude_num, altitude_file);
		}
	if (heading_mode == MBSSLAYOUT_MERGE_FILE)
		{
		mb_loadheadingdata(verbose, heading_file, heading_file_format,
			       &heading_num, &heading_alloc,
			       &heading_time_d, &heading_heading, &error);
		
		if (verbose > 0)
			fprintf(stderr,"%d heading records loaded from file %s\n", heading_num, heading_file);
		}
	if (attitude_mode == MBSSLAYOUT_MERGE_FILE)
		{
		mb_loadattitudedata(verbose, attitude_file, attitude_file_format,
			       &attitude_num, &attitude_alloc,
			       &attitude_time_d, &attitude_roll, &attitude_pitch, &attitude_heave, &error);
		
		if (verbose > 0)
			fprintf(stderr,"%d attitude records loaded from file %s\n", attitude_num, attitude_file);
		}
	if (soundspeed_mode == MBSSLAYOUT_MERGE_FILE)
		{
		mb_loadsoundspeeddata(verbose, soundspeed_file, soundspeed_file_format,
			       &soundspeed_num, &soundspeed_alloc,
			       &soundspeed_time_d, &soundspeed_soundspeed, &error);
		
		if (verbose > 0)
			fprintf(stderr,"%d soundspeed records loaded from file %s\n", soundspeed_num, soundspeed_file);
		}
	if (timeshift_mode == MBSSLAYOUT_MERGE_FILE)
		{
		mb_loadtimeshiftdata(verbose, timeshift_file, timeshift_format,
			       &timeshift_num, &timeshift_alloc,
			       &timeshift_time_d, &timeshift_timeshift, &error);
		
		if (verbose > 0)
			fprintf(stderr,"%d timeshift records loaded from file %s\n", heading_num, heading_file);
		}

	/* new output file obviously needed */
	new_output_file = MB_YES;

	/* if specified read route time list file */
	if (line_mode == MBSSLAYOUT_LINE_TIME)
		{
		/* open the input file */
		if ((fp = fopen(line_time_list, "r")) == NULL)
			{
			error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
			fprintf(stderr,"\nUnable to open time list file <%s> for reading\n",line_time_list);
			exit(status);
			}
		rawroutefile = MB_NO;
		while ((result = fgets(comment,MB_PATH_MAXLINE,fp)) == comment)
		    	{
			if (comment[0] != '#')
				{
				nget = sscanf(comment,"%d %d %lf %lf %lf %lf",
				    &i, &waypoint, &navlon, &navlat, &heading, &time_d);

				/* if good data check for need to allocate more space */
				if (ntimepoint + 1 > ntimepointalloc)
				    	{
				    	ntimepointalloc += MBSSLAYOUT_ALLOC_NUM;
					status = mb_reallocd(verbose, __FILE__, __LINE__, ntimepointalloc * sizeof(double),
								(void **)&routelon, &error);
					status = mb_reallocd(verbose, __FILE__, __LINE__, ntimepointalloc * sizeof(double),
								(void **)&routelat, &error);
					status = mb_reallocd(verbose, __FILE__, __LINE__, ntimepointalloc * sizeof(double),
								(void **)&routeheading, &error);
					status = mb_reallocd(verbose, __FILE__, __LINE__, ntimepointalloc * sizeof(int),
								(void **)&routewaypoint, &error);
					status = mb_reallocd(verbose, __FILE__, __LINE__, ntimepointalloc * sizeof(double),
								(void **)&routetime_d, &error);
				    	if (status != MB_SUCCESS)
					    	{
						mb_error(verbose,error,&message);
						fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
							message);
						fprintf(stderr,"\nProgram <%s> Terminated\n",
							program_name);
						exit(error);
					    	}
				    	}

				/* add good point to route */
				if (ntimepointalloc > ntimepoint)
					{
					routewaypoint[ntimepoint] = waypoint;
					routelon[ntimepoint] = navlon;
					routelat[ntimepoint] = navlat;
					routeheading[ntimepoint] = heading;
					routetime_d[ntimepoint] = time_d;
					ntimepoint++;
					}
				}
			}

		/* close the file */
		fclose(fp);
		fp = NULL;

		activewaypoint = 1;
		mb_coor_scale(verbose,routelat[activewaypoint], &mtodeglon, &mtodeglat);
		rangelast = 1000 * line_range_threshold;
		oktowrite = 0;
		linechange = MB_NO;

		/* output status */
		if (verbose > 0)
			{
			/* output info on file output */
			fprintf(stderr,"Read %d waypoints from time list file: %s\n",
				ntimepoint, line_time_list);
			}
		}

	/* if specified read route file */
	else if (line_mode == MBSSLAYOUT_LINE_ROUTE)
		{
		/* open the input file */
		if ((fp = fopen(line_route, "r")) == NULL)
			{
			error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
			fprintf(stderr,"\nUnable to open route file <%s> for reading\n",line_route);
			exit(status);
			}
		rawroutefile = MB_NO;
		while ((result = fgets(comment,MB_PATH_MAXLINE,fp)) == comment)
		    	{
			if (comment[0] == '#')
				{
				if (strncmp(comment,"## Route File Version", 21) == 0)
					{
					rawroutefile = MB_NO;
					}
				}
			else
				{
				nget = sscanf(comment,"%lf %lf %lf %d %lf",
				    &navlon, &navlat, &topo, &waypoint, &heading);
				if (comment[0] == '#')
					{
					fprintf(stderr,"buffer:%s",comment);
					if (strncmp(comment,"## Route File Version", 21) == 0)
						{
						rawroutefile = MB_NO;
						}
					}
		    		if ((rawroutefile == MB_YES && nget >= 2)
					|| (rawroutefile == MB_NO && nget >= 3 && waypoint > MBSSLAYOUT_ROUTE_WAYPOINT_NONE))
					point_ok = MB_YES;
				else
					point_ok = MB_NO;

				/* if good data check for need to allocate more space */
				if (point_ok == MB_YES
					&& nroutepoint + 1 > nroutepointalloc)
				    	{
				    	nroutepointalloc += MBSSLAYOUT_ALLOC_NUM;
					status = mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double),
								(void **)&routelon, &error);
					status = mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double),
								(void **)&routelat, &error);
					status = mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double),
								(void **)&routeheading, &error);
					status = mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(int),
								(void **)&routewaypoint, &error);
				    	if (status != MB_SUCCESS)
					    	{
						mb_error(verbose,error,&message);
						fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
							message);
						fprintf(stderr,"\nProgram <%s> Terminated\n",
							program_name);
						exit(error);
					    	}
				    	}

				/* add good point to route */
				if (point_ok == MB_YES && nroutepointalloc > nroutepoint + 1)
					{
					routelon[nroutepoint] = navlon;
					routelat[nroutepoint] = navlat;
					routeheading[nroutepoint] = heading;
					routewaypoint[nroutepoint] = waypoint;
					nroutepoint++;
					}
				}
			}

		/* close the file */
		fclose(fp);
		fp = NULL;

		/* set starting values */
		activewaypoint = 1;
		mb_coor_scale(verbose,routelat[activewaypoint], &mtodeglon, &mtodeglat);
		rangelast = 1000 * line_range_threshold;
		oktowrite = 0;
		linechange = MB_NO;

		/* output status */
		if (verbose > 0)
			{
			/* output info on file output */
			fprintf(stderr,"\nImported %d waypoints from route file: %s\n",
				nroutepoint, line_route);
			}
		}

	/* set up plotting script file */
	if ((line_mode == MBSSLAYOUT_LINE_ROUTE && nroutepoint > 1) ||
		(line_mode == MBSSLAYOUT_LINE_TIME && ntimepoint > 1))
		{
		sprintf(scriptfile, "%s_%s_ssswathplot.cmd", output_name1, output_name2);
		}
	else
		{
		sprintf(scriptfile, "%s_ssswathplot.cmd", read_file);
		}
	if ((sfp = fopen(scriptfile, "w")) == NULL)
		{
		error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
		fprintf(stderr,"\nUnable to open plotting script file <%s> \n",scriptfile);
		exit(status);
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
		n_rf_ss2 = 0;
		n_rf_ss3 = 0;
		n_rf_sbp = 0;
		n_rf_nav = 0;
		n_rf_nav1 = 0;
		n_rf_nav2 = 0;
		n_rf_nav3 = 0;
	
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
			else if (kind == MB_DATA_SIDESCAN2)
				{
				n_rf_ss2++;
				n_rt_ss2++;
				}
			else if (kind == MB_DATA_SIDESCAN3)
				{
				n_rf_ss3++;
				n_rt_ss3++;
				}
			else if (kind == MB_DATA_SUBBOTTOM_SUBBOTTOM)
				{
				n_rf_sbp++;
				n_rt_sbp++;
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
				
			/* look for nav if not externally defined */
			if (status == MB_SUCCESS
				&& nav_mode == MBSSLAYOUT_MERGE_ASYNC
				&& kind == sensordepth_async)
				{
				/* extract nav data */
				status = mb_extract_nnav(verbose, imbio_ptr, istore_ptr,
							nanavmax, &kind, &nanav,
							antime_i, antime_d,
							anlon, anlat,
							anspeed, anheading, ansensordraft,
							anroll, anpitch, anheave,
							&error);

				/* allocate memory if needed */
				if (status == MB_SUCCESS
					&& nanav > 0
					&& nav_num + nanav >= nav_alloc)
					{
					nav_alloc += MAX(MBSSLAYOUT_ALLOC_CHUNK, nanav);
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
						nav_time_d[nav_num] = antime_d[i];
						nav_navlon[nav_num] = anlon[i];
						nav_navlat[nav_num] = anlat[i];
						nav_speed[nav_num] = anspeed[i];
						nav_num++;
						}
					}
				}
				
			/* look for sensordepth if not externally defined */
			if (status == MB_SUCCESS
				&& sensordepth_mode == MBSSLAYOUT_MERGE_ASYNC
				&& kind == sensordepth_async)
				{
				/* extract sensordepth data */
				status = mb_extract_nnav(verbose, imbio_ptr, istore_ptr,
							nanavmax, &kind, &nanav,
							antime_i, antime_d,
							anlon, anlat,
							anspeed, anheading, ansensordraft,
							anroll, anpitch, anheave,
							&error);

				/* allocate memory if needed */
				if (status == MB_SUCCESS
					&& nanav > 0
					&& sensordepth_num + nanav >= sensordepth_alloc)
					{
					sensordepth_alloc += MAX(MBSSLAYOUT_ALLOC_CHUNK, nanav);
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
						sensordepth_time_d[sensordepth_num] = antime_d[i];
						sensordepth_sensordepth[sensordepth_num] = ansensordraft[i] + anheave[i];
						sensordepth_num++;
						}
					}
				}
				
			/* look for altitude if not externally defined */
			if (status == MB_SUCCESS
				&& altitude_mode == MBSSLAYOUT_MERGE_ASYNC
				&& kind == altitude_async)
				{
				/* extract altitude data */
				status = mb_extract_nav(verbose, imbio_ptr, istore_ptr,
							&kind, time_i, &time_d,
							&navlon, &navlat,
							&speed, &heading, &sensordraft,
							&roll, &pitch, &heave,
							&error);
				status = mb_extract_altitude(verbose, imbio_ptr, istore_ptr,
							&kind, &sensordepth, &altitude,
							&error);

				/* allocate memory if needed */
				if (status == MB_SUCCESS
					&& altitude_num + 1 >= altitude_alloc)
					{
					altitude_alloc += MAX(MBSSLAYOUT_ALLOC_CHUNK, nanav);
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
				if (status == MB_SUCCESS
					&& nanav > 0)
					{
					altitude_time_d[altitude_num] = time_d;
					altitude_altitude[altitude_num] = altitude;
					altitude_num++;
					}
				}
				
			/* look for heading if not externally defined */
			if (status == MB_SUCCESS
				&& heading_mode == MBSSLAYOUT_MERGE_ASYNC
				&& kind == heading_async)
				{
				/* extract heading data */
				status = mb_extract_nnav(verbose, imbio_ptr, istore_ptr,
							nanavmax, &kind, &nanav,
							antime_i, antime_d,
							anlon, anlat,
							anspeed, anheading, ansensordraft,
							anroll, anpitch, anheave,
							&error);

				/* allocate memory if needed */
				if (status == MB_SUCCESS
					&& nanav > 0
					&& heading_num + nanav >= heading_alloc)
					{
					heading_alloc += MAX(MBSSLAYOUT_ALLOC_CHUNK, nanav);
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
						heading_time_d[heading_num] = antime_d[i];
						heading_heading[heading_num] = anheading[i];
						heading_num++;
						}
					}
				}
				
			/* look for attitude if not externally defined */
			if (status == MB_SUCCESS
				&& attitude_mode == MBSSLAYOUT_MERGE_ASYNC
				&& kind == attitude_async)
				{
				/* extract attitude data */
				status = mb_extract_nnav(verbose, imbio_ptr, istore_ptr,
							nanavmax, &kind, &nanav,
							antime_i, antime_d,
							anlon, anlat,
							anspeed, anheading, ansensordraft,
							anroll, anpitch, anheave,
							&error);

				/* allocate memory if needed */
				if (status == MB_SUCCESS
					&& nanav > 0
					&& attitude_num + nanav >= attitude_alloc)
					{
					attitude_alloc += MAX(MBSSLAYOUT_ALLOC_CHUNK, nanav);
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
						attitude_time_d[attitude_num] = antime_d[i];
						attitude_roll[attitude_num] = anroll[i];
						attitude_pitch[attitude_num] = anpitch[i];
						attitude_heave[attitude_num] = anheave[i];
						attitude_num++;
						}
					}
				}
				
			/* look for soundspeed if not externally defined */
			if (status == MB_SUCCESS
				&& soundspeed_mode == MBSSLAYOUT_MERGE_ASYNC
				&& kind == sensordepth_async)
				{
				/* extract soundspeed data */
				status = mb_ctd(verbose, imbio_ptr, istore_ptr,
							&kind, &nactd,
							actime_d, acconductivity, actemperature,
							acdepth, acsalinity, acsoundspeed,
							&error);

				/* allocate memory if needed */
				if (status == MB_SUCCESS
					&& nactd > 0
					&& soundspeed_num + nactd >= soundspeed_alloc)
					{
					soundspeed_alloc += MAX(MBSSLAYOUT_ALLOC_CHUNK, nactd);
					status = mb_reallocd(verbose,__FILE__,__LINE__,soundspeed_alloc*sizeof(double),(void **)&soundspeed_time_d,&error);
					status = mb_reallocd(verbose,__FILE__,__LINE__,soundspeed_alloc*sizeof(double),(void **)&soundspeed_soundspeed,&error);
					if (error != MB_ERROR_NO_ERROR)
						{
						mb_error(verbose,error,&message);
						fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
						fprintf(stderr,"\nProgram <%s> Terminated\n",
						    program_name);
						exit(error);
						}
					}
					
				/* copy the soundspeed data */
				if (status == MB_SUCCESS
					&& nactd > 0)
					{
					for (i=0;i<nactd;i++)
						{
						soundspeed_time_d[soundspeed_num] = actime_d[i];
						soundspeed_soundspeed[soundspeed_num] = acsoundspeed[i];
						soundspeed_num++;
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
			fprintf(stderr,"     %d sidescan2 records\n", n_rf_ss2);
			fprintf(stderr,"     %d sidescan3 records\n", n_rf_ss3);
			fprintf(stderr,"     %d subbottom records\n", n_rf_sbp);
			fprintf(stderr,"     %d nav records\n", n_rf_nav);
			fprintf(stderr,"     %d nav1 records\n", n_rf_nav1);
			fprintf(stderr,"     %d nav2 records\n", n_rf_nav2);
			fprintf(stderr,"     %d nav3 records\n", n_rf_nav3);
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
		fprintf(stderr,"\nPass 1: Total records read from all input files\n");
		fprintf(stderr,"     %d survey records\n", n_rt_data);
		fprintf(stderr,"     %d comment records\n", n_rt_comment);
		fprintf(stderr,"     %d sidescan2 records\n", n_rt_ss2);
		fprintf(stderr,"     %d sidescan3 records\n", n_rt_ss3);
		fprintf(stderr,"     %d subbottom records\n", n_rt_sbp);
		fprintf(stderr,"     %d nav records\n", n_rt_nav);
		fprintf(stderr,"     %d nav1 records\n", n_rt_nav1);
		fprintf(stderr,"     %d nav2 records\n", n_rt_nav2);
		fprintf(stderr,"     %d nav3 records\n", n_rt_nav3);
		}
		
	/* end first pass through data */
	
	/*-------------------------------------------------------------------*/
	
	/* Apply any specified timeshift to the chosen data */
	if (timeshift_mode != MBSSLAYOUT_TIMESHIFT_OFF)
		{
		/* if no affected data have been specified apply timeshift to all */
		if (timeshift_apply == MBSSLAYOUT_TIMESHIFT_APPLY_NONE)
			timeshift_apply =  MBSSLAYOUT_TIMESHIFT_APPLY_ALL_ANCILLIARY;
			
		/* apply timeshift to nav data */
		if (timeshift_apply &  MBSSLAYOUT_TIMESHIFT_APPLY_NAV)
			{
			if (timeshift_mode == MBSSLAYOUT_TIMESHIFT_FILE)
				{
				j = 0;
				for (i=0;i<nav_num;i++)
					{
					interp_status = mb_linear_interp(verbose,
								timeshift_time_d-1, timeshift_timeshift-1,
								timeshift_num, nav_time_d[i], &timeshift, &j,
								&interp_error);
					nav_time_d[i] -= timeshift;
					}
				
				}
			else if (timeshift_mode == MBSSLAYOUT_TIMESHIFT_CONSTANT)
				{
				for (i=0;i<nav_num;i++)
					{
					nav_time_d[i] -= timeshift_constant;
					}
				}
			}
			
		/* apply timeshift to sensordepth data */
		if (timeshift_apply & MBSSLAYOUT_TIMESHIFT_APPLY_SENSORDEPTH)
			{
			if (timeshift_mode == MBSSLAYOUT_TIMESHIFT_FILE)
				{
				j = 0;
				for (i=0;i<sensordepth_num;i++)
					{
					interp_status = mb_linear_interp(verbose,
								timeshift_time_d-1, timeshift_timeshift-1,
								timeshift_num, sensordepth_time_d[i], &timeshift, &j,
								&interp_error);
					sensordepth_time_d[i] -= timeshift;
					}
				
				}
			else if (timeshift_mode == MBSSLAYOUT_TIMESHIFT_CONSTANT)
				{
				for (i=0;i<sensordepth_num;i++)
					{
					sensordepth_time_d[i] -= timeshift_constant;
					}
				}
			}
			
		/* apply timeshift to heading data */
		if (timeshift_apply & MBSSLAYOUT_TIMESHIFT_APPLY_HEADING)
			{
			if (timeshift_mode == MBSSLAYOUT_TIMESHIFT_FILE)
				{
				j = 0;
				for (i=0;i<heading_num;i++)
					{
					interp_status = mb_linear_interp(verbose,
								timeshift_time_d-1, timeshift_timeshift-1,
								timeshift_num, heading_time_d[i], &timeshift, &j,
								&interp_error);
					heading_time_d[i] -= timeshift;
					}
				
				}
			else if (timeshift_mode == MBSSLAYOUT_TIMESHIFT_CONSTANT)
				{
				for (i=0;i<heading_num;i++)
					{
					heading_time_d[i] -= timeshift_constant;
					}
				}
			}
			
		/* apply timeshift to attitude data */
		if (timeshift_apply & MBSSLAYOUT_TIMESHIFT_APPLY_ATTITUDE)
			{
			if (timeshift_mode == MBSSLAYOUT_TIMESHIFT_FILE)
				{
				j = 0;
				for (i=0;i<attitude_num;i++)
					{
					interp_status = mb_linear_interp(verbose,
								timeshift_time_d-1, timeshift_timeshift-1,
								timeshift_num, attitude_time_d[i], &timeshift, &j,
								&interp_error);
					attitude_time_d[i] -= timeshift;
					}
				
				}
			else if (timeshift_mode == MBSSLAYOUT_TIMESHIFT_CONSTANT)
				{
				for (i=0;i<attitude_num;i++)
					{
					attitude_time_d[i] -= timeshift_constant;
					}
				}
			}
			
		/* apply timeshift to soundspeed data */
		if (timeshift_apply & MBSSLAYOUT_TIMESHIFT_APPLY_SOUNDSPEED)
			{
			if (timeshift_mode == MBSSLAYOUT_TIMESHIFT_FILE)
				{
				j = 0;
				for (i=0;i<soundspeed_num;i++)
					{
					interp_status = mb_linear_interp(verbose,
								timeshift_time_d-1, timeshift_timeshift-1,
								timeshift_num, soundspeed_time_d[i], &timeshift, &j,
								&interp_error);
					soundspeed_time_d[i] -= timeshift;
					}
				
				}
			else if (timeshift_mode == MBSSLAYOUT_TIMESHIFT_CONSTANT)
				{
				for (i=0;i<soundspeed_num;i++)
					{
					soundspeed_time_d[i] -= timeshift_constant;
					}
				}
			}
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
	n_wf_data = 0;
	n_wf_comment = 0;
	n_wt_data = 0;
	n_wt_comment = 0;
	
	/* if generating survey line files the line number is initialized to 0 so the first line is 1 */
	if (line_mode != MBSSLAYOUT_LINE_OFF)
		{
		line_number = activewaypoint;
		new_output_file = MB_YES;
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
			
		/* if not generating survey line files then open output file to coincide with this input file */
		if (line_mode == MBSSLAYOUT_LINE_OFF)
			new_output_file = MB_YES;
	
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
		n_rf_ss2 = 0;
		n_rf_ss3 = 0;
		n_rf_sbp = 0;
		n_rf_nav = 0;
		n_rf_nav1 = 0;
		n_rf_nav2 = 0;
		n_rf_nav3 = 0;

		/* ------------------------------- */
		/* start read + output loop */
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
				}
			else if (kind == MB_DATA_COMMENT)
				{
				n_rf_comment++;
				n_rt_comment++;
				}
			else if (kind == MB_DATA_SIDESCAN2)
				{
				n_rf_ss2++;
				n_rt_ss2++;
				}
			else if (kind == MB_DATA_SIDESCAN3)
				{
				n_rf_ss3++;
				n_rt_ss3++;
				}
			else if (kind == MB_DATA_SUBBOTTOM_SUBBOTTOM)
				{
				n_rf_sbp++;
				n_rt_sbp++;
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
				
			/* check for new line only if generating survey line files
				and new line not already set
				and this record is target data */
			if (status == MB_SUCCESS
				&& line_mode != MBSSLAYOUT_LINE_OFF
				&& new_output_file == MB_NO
				&& kind == output_source)
				{
				/* check waypoint time list */
				if (line_mode == MBSSLAYOUT_LINE_TIME
					&& time_d >= routetime_d[activewaypoint]
					&& activewaypoint < ntimepoint)
					{
					new_output_file = MB_YES;
/* fprintf(stderr,"LINECHANGE BY TIME!! dx:%f dy:%f range:%f activewaypoint:%d time_d: %f %f\n",
dx,dy,range,activewaypoint,time_d,routetime_d[activewaypoint]); */
					activewaypoint++;
					line_number = activewaypoint;
					}
					
				/* check waypoint position list */
				else if (line_mode == MBSSLAYOUT_LINE_ROUTE)
					{
					dx = (navlon - routelon[activewaypoint]) / mtodeglon;
					dy = (navlat - routelat[activewaypoint]) / mtodeglat;
					range = sqrt(dx * dx + dy * dy);
/* fprintf(stderr,"CHECK WAYPOINT: activewaypoint:%d range:%f line_range_threshold:%f\n",activewaypoint,range,line_range_threshold); */
					if (range < line_range_threshold
						&& (activewaypoint == 0 || range > rangelast)
						&& activewaypoint < nroutepoint - 1)
						{
						new_output_file = MB_YES;
/* fprintf(stderr,"LINECHANGE BY WAYPOINT!! dx:%f dy:%f range:%f activewaypoint:%d time_d: %f %f\n",
dx,dy,range,activewaypoint,time_d,routetime_d[activewaypoint]); */
						activewaypoint++;
						line_number = activewaypoint;
						}
					}
				}
				
			/* open output files if needed */
			if (new_output_file == MB_YES)
				{
				/* reset flag */
				new_output_file = MB_NO;
				
				if (output_source != MB_DATA_NONE)
					{
					/* close any old output file unless a single file has been specified */
					if (ombio_ptr != NULL)
						{
						/* close the swath file */
						status = mb_close(verbose,&ombio_ptr,&error);
		
						/* generate inf file */
						/* if (status == MB_SUCCESS)
							{
							status = mb_make_info(verbose, MB_YES,
										output_file,
										MBF_MBLDEOIH,
										&error);
							}*/
		
						/* output counts */
						if (verbose > 0)
							{
							fprintf(stdout, "\nPass 2: Closing output file: %s\n", output_file);
							fprintf(stdout, "Pass 2: Records written to output file %s\n", output_file);
							fprintf(stdout, "     %d survey records\n", n_wf_data);
							fprintf(stdout, "     %d comment records\n", n_wf_comment);
							}
		
						/* output commands to first cut plotting script file */
						fprintf(sfp, "# Generate swath plot of sidescan file: %s\n", output_file);
						fprintf(sfp, "mbm_plot -I %s -N -G5 -S -Pb -V -O %s_ssrawplot\n",
							output_file, output_file);
						fprintf(sfp, "%s_ssrawplot.cmd $1\n\n", output_file);
						fprintf(sfp, "convert -density 100 %s_ssrawplot.ps -trim -quality 75 %s_ssrawplot.jpg", output_file, output_file);
						}
		
					/* define the filename */
					if (line_mode == MBSSLAYOUT_LINE_OFF)
						sprintf(output_file, "%s_%s.mb%2.2d",
							ifile, output_name2, MBF_MBLDEOIH);
					else
						sprintf(output_file, "%s_%s_%4.4d.mb%2.2d",
							output_name1, output_name2, line_number, MBF_MBLDEOIH);

					/* open the new file */
					if (verbose > 0)
						fprintf(stderr,"Pass 2: Opening output file:  %s %d\n", output_file, MBF_MBLDEOIH);
					if ((status = mb_write_init(
						verbose,output_file, MBF_MBLDEOIH,
						&ombio_ptr, &obeams_bath, &obeams_amp, &opixels_ss, &error)) != MB_SUCCESS)
						{
						mb_error(verbose,error,&message);
						fprintf(stderr,"\nMBIO Error returned from function <mb_write_init>:\n%s\n",message);
						fprintf(stderr,"\nMultibeam File <%s> not initialized for writing\n",output_file);
						fprintf(stderr,"\nProgram <%s> Terminated\n",
							program_name);
						exit(error);
						}
		
					/* get pointers to data storage */
					omb_io_ptr = (struct mb_io_struct *) ombio_ptr;
					ostore_ptr = omb_io_ptr->store_data;
					ostore = (struct mbsys_ldeoih_struct *) ostore_ptr;
					
					n_wf_data = 0;
					n_wf_comment = 0;
					}
				}
	
			/* if data of interest have been read process them */
			if (status == MB_SUCCESS
				&& kind == output_source)
				{
				/* start out with no change defined */
				data_changed = MB_NO;
				
				/* call mb_extract_rawssdimensions() */
				status = mb_extract_rawssdimensions(verbose,imbio_ptr,istore_ptr,
								&kind, &sample_interval,
								&num_samples_port, &num_samples_stbd, &error);
				
				/* allocate memory if necessary */
				if (num_samples_port > num_samples_port_alloc)
					{
					num_samples_port_alloc = num_samples_port;
					status = mb_reallocd(verbose,__FILE__,__LINE__,num_samples_port_alloc*sizeof(double),(void **)&raw_samples_port,&error);
					if (error != MB_ERROR_NO_ERROR)
						{
						mb_error(verbose,error,&message);
						fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
						fprintf(stderr,"\nProgram <%s> Terminated\n",
						    program_name);
						exit(error);
						}
					}
				if (num_samples_stbd > num_samples_stbd_alloc)
					{
					num_samples_stbd_alloc = num_samples_stbd;
					status = mb_reallocd(verbose,__FILE__,__LINE__,num_samples_stbd_alloc*sizeof(double),(void **)&raw_samples_stbd,&error);
					if (error != MB_ERROR_NO_ERROR)
						{
						mb_error(verbose,error,&message);
						fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
						fprintf(stderr,"\nProgram <%s> Terminated\n",
						    program_name);
						exit(error);
						}
					}
				
				/* call mb_extract_rawss() */
				status = mb_extract_rawss(verbose,imbio_ptr,istore_ptr,
								&kind, &sidescan_type, &sample_interval,
								&beamwidth_xtrack, &beamwidth_ltrack,
								&num_samples_port, raw_samples_port,
								&num_samples_stbd, raw_samples_stbd, &error);
				
				/* call mb_extract_nav to get attitude */
				status = mb_extract_nav(verbose,imbio_ptr,istore_ptr,&kind,
						time_i,&time_d,&navlon_org,&navlat_org,
						&speed_org,&heading_org,&draft_org,&roll_org,&pitch_org,&heave_org,&error);
				
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
				soundspeed = soundspeed_constant;
					
				/* apply timeshift to survey data */
				if (timeshift_apply & MBSSLAYOUT_TIMESHIFT_APPLY_SURVEY)
					{
					if (timeshift_mode == MBSSLAYOUT_TIMESHIFT_FILE)
						{
						interp_status = mb_linear_interp(verbose,
										timeshift_time_d-1, timeshift_timeshift-1,
										timeshift_num, time_d, &timeshift, &jsurvey,
										&interp_error);
						time_d += timeshift;						
						}
					else if (timeshift_mode == MBSSLAYOUT_TIMESHIFT_CONSTANT)
						{
						time_d += timeshift_constant;
						}
					}

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
					data_changed = MB_YES;
					}
				if (sensordepth_num > 0)
					{
					interp_status = mb_linear_interp(verbose,
								sensordepth_time_d-1, sensordepth_sensordepth-1, sensordepth_num, 
								time_d, &sensordepth, &jsensordepth,
								&interp_error);
					data_changed = MB_YES;
					}
				if (altitude_num > 0)
					{
					interp_status = mb_linear_interp(verbose,
								altitude_time_d-1, altitude_altitude-1, altitude_num, 
								time_d, &altitude, &jaltitude,
								&interp_error);
					data_changed = MB_YES;
					}
				if (heading_num > 0)
					{
					interp_status = mb_linear_interp_heading(verbose,
								heading_time_d-1, heading_heading-1, heading_num, 
								time_d, &heading, &jheading,
								&interp_error);
					data_changed = MB_YES;
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
					data_changed = MB_YES;
					}
				if (sensordepth_num > 0 || attitude_num > 0)
					{
					draft = sensordepth - heave;
					}
				if (soundspeed_num > 0)
					{
					interp_status = mb_linear_interp(verbose,
								soundspeed_time_d-1, soundspeed_soundspeed-1, soundspeed_num, 
								time_d, &soundspeed, &jsoundspeed,
								&interp_error);
					data_changed = MB_YES;
					}
				
				/* if specified get altitude from raw sidescan */
				if (ss_altitude_mode == MBSSLAYOUT_ALTITUDE_BOTTOMPICK)
					{
					/* get bottom arrival in port trace */
					channelmax = 0.0;
					for (i=0;i<num_samples_port;i++)
						{
						channelmax = MAX(raw_samples_port[i], channelmax);
						}
					portchannelpick = 0;
					threshold = bottompick_threshold * channelmax;
					for (i=0;i<num_samples_port && portchannelpick == 0;i++)
						{
						if (raw_samples_port[i] >= threshold)
							portchannelpick = i;
						}

					/* get bottom arrival in starboard trace */
					channelmax = 0.0;
					for (i=0;i<num_samples_stbd;i++)
						{
						channelmax = MAX(raw_samples_stbd[i], channelmax);
						}
					stbdchannelpick = 0;
					threshold = bottompick_threshold * channelmax;
					for (i=0;i<num_samples_stbd && stbdchannelpick == 0;i++)
						{
						if (raw_samples_stbd[i] >= threshold)
							stbdchannelpick = i;
						}
					ttime = 0.5 * ((portchannelpick + stbdchannelpick) * sample_interval);
					ss_altitude = 0.5 * soundspeed * ttime;
					}
				
				/* else if getting altitude from topography model set initial value zero */
				else if (ss_altitude_mode == MBSSLAYOUT_ALTITUDE_TOPO_GRID)
					{
					mb_topogrid_topo(verbose, topogrid_ptr, navlon, navlat, &topo, &error);
					ss_altitude = -sensordepth - topo;
					}
				
				/* else just use existing altitude value */
				else if (ss_altitude_mode == MBSSLAYOUT_ALTITUDE_ALTITUDE)
					{
					ss_altitude = altitude;
					}

				/* get flat bottom layout table */
				if (layout_mode == MBSSLAYOUT_LAYOUT_FLATBOTTOM)
					mbsslayout_get_flatbottom_table(verbose, nangle, angle_min, angle_max,
									navlon, navlat, ss_altitude, 0.0,
									table_angle, table_xtrack, table_ltrack, table_altitude,
									table_range, &error);
				/* else get 3D bottom layout table */
				else
					{
					mb_topogrid_getangletable(verbose, topogrid_ptr, nangle, angle_min, angle_max,
									navlon, navlat, heading,
									ss_altitude, sensordepth, pitch,
									table_angle, table_xtrack, table_ltrack,
									table_altitude, table_range, &error);
					}
/* fprintf(stderr,"altitude:%f sensordepth:%f pitch:%f\n",ss_altitude,sensordepth,pitch);
for (i=0;i<nangle;i++)
fprintf(stderr,"%d %f %f %f %f %f\n",i,table_angle[i],table_xtrack[i],table_ltrack[i],table_altitude[i],table_range[i]);*/
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
				if (swath_mode == MBSSLAYOUT_SWATHWIDTH_VARIABLE)
					{
					rr = 0.5 * soundspeed * sample_interval * MAX(num_samples_port,num_samples_stbd);
					swath_width = 2.2 * sqrt(rr * rr - ss_altitude * ss_altitude);
					}
				pixel_width = swath_width / (opixels_ss - 1);

				/* initialize the output sidescan */
				for (j=0;j<opixels_ss;j++)
					{
					oss[j] = 0.0;
					ossacrosstrack[j] = pixel_width * (double)(j - (opixels_ss / 2));
					ossalongtrack[j] = 0.0;
					ossbincount[j] = 0;
					}

				/* find minimum range */
				rangemin = table_range[0];
				kstart = 0;
				for (kangle=1;kangle<nangle;kangle++)
					{
					if (table_range[kangle] < rangemin)
						{
						rangemin = table_range[kangle];
						kstart = kangle;
						}
					}
/* fprintf(stderr,"port minimum range:%f kstart:%d\n",rangemin,kstart);*/

				/* bin port trace */
				istart = rangemin / (soundspeed * sample_interval);
				for (i=istart;i<num_samples_port;i++)
					{
					/* get sample range */
					rr = 0.5 * soundspeed * sample_interval * i;

					/* look up position(s) for this range */
					done = MB_NO;
					for (kangle=kstart;kangle>0 && done == MB_NO;kangle--)
						{
						found = MB_NO;
						if (rr <= table_range[kstart])
							{
							xtrack = table_xtrack[kstart];
							ltrack = table_ltrack[kstart];
							done = MB_YES;
							found = MB_YES;
							}
						else if (rr > table_range[kangle] && rr <= table_range[kangle-1])
							{
							factor = (rr - table_range[kangle])
								/ (table_range[kangle-1] - table_range[kangle]);
							xtrack = table_xtrack[kangle]
								+ factor * (table_xtrack[kangle-1] - table_xtrack[kangle]);
							ltrack = table_ltrack[kangle]
								+ factor * (table_ltrack[kangle-1] - table_ltrack[kangle]);
							found = MB_YES;
							done = MB_YES;
							}
						else if (rr < table_range[kangle] && rr >= table_range[kangle-1])
							{
							factor = (rr - table_range[kangle])
								/ (table_range[kangle-1] - table_range[kangle]);
							xtrack = table_xtrack[kangle]
								+ factor * (table_xtrack[kangle-1] - table_xtrack[kangle]);
							ltrack = table_ltrack[kangle]
								+ factor * (table_ltrack[kangle-1] - table_ltrack[kangle]);
							found = MB_YES;
							done = MB_YES;
							}

						/* bin the value and position */
						if (found == MB_YES)
							{
							j = opixels_ss / 2 + (int)(xtrack / pixel_width);
							if (j >= 0 && j < opixels_ss)
								{
								oss[j] += raw_samples_port[i];
								ossbincount[j]++;
								ossalongtrack[j] += ltrack;
								}
/* fprintf(stderr,"port:%5d rr:%10.2f x:%10.2f l:%10.2f kangle:%d\n",
i,rr,xtrack,ltrack,kangle); */
							}
						}
					}

				/* find minimum range */
				rangemin = table_range[0];
				kstart = 0;
				for (kangle=1;kangle<nangle;kangle++)
					{
					if (table_range[kangle] < rangemin)
						{
						rangemin = table_range[kangle];
						kstart = kangle;
						}
					}
/* fprintf(stderr,"stbd minimum range:%f kstart:%d\n",rr,kstart); */
/* fprintf(stderr,"kstart:%d angle:%f range:%f xtrack:%f ltrack:%f\n",
kstart,
angle_min + kstart * (angle_max - angle_min) / (nangle - 1),
table_range[kstart],table_xtrack[kstart],table_ltrack[kstart]);*/

				/* bin stbd trace */
				istart = rangemin / (soundspeed * sample_interval);
				for (i=istart;i<num_samples_stbd;i++)
					{
					/* get sample range */
					rr = 0.5 * soundspeed * sample_interval * i;

					/* look up position for this range */
					done = MB_NO;
					for (kangle=kstart;kangle<nangle-1 && done == MB_NO;kangle++)
						{
						found = MB_NO;
						if (rr <= table_range[kstart])
							{
							xtrack = table_xtrack[kstart];
							ltrack = table_ltrack[kstart];
							done = MB_YES;
							found = MB_YES;
							}
						else if (rr > table_range[kangle] && rr <= table_range[kangle+1])
							{
							factor = (rr - table_range[kangle])
								/ (table_range[kangle+1] - table_range[kangle]);
							xtrack = table_xtrack[kangle]
								+ factor * (table_xtrack[kangle+1] - table_xtrack[kangle]);
							ltrack = table_ltrack[kangle]
								+ factor * (table_ltrack[kangle+1] - table_ltrack[kangle]);
							found = MB_YES;
							done = MB_YES;
							}
						else if (rr < table_range[kangle] && rr >= table_range[kangle+1])
							{
							factor = (rr - table_range[kangle])
								/ (table_range[kangle+1] - table_range[kangle]);
							xtrack = table_xtrack[kangle]
								+ factor * (table_xtrack[kangle+1] - table_xtrack[kangle]);
							ltrack = table_ltrack[kangle]
								+ factor * (table_ltrack[kangle+1] - table_ltrack[kangle]);
							found = MB_YES;
							done = MB_YES;
							}

						/* bin the value and position */
						if (found == MB_YES)
							{
							j = opixels_ss / 2 + (int)(xtrack / pixel_width);
							if (j >= 0 && j < opixels_ss)
								{
								oss[j] += raw_samples_stbd[i];
								ossbincount[j]++;
								ossalongtrack[j] += ltrack;
								}
/* fprintf(stderr,"stbd:%5d rr:%10.2f x:%10.2f l:%10.2f kangle:%d\n",
i,rr,xtrack,ltrack,kangle); */
							}
						}
					}

				/* calculate the output sidescan */
				jport = -1;
				jstbd = -1;
				for (j=0;j<opixels_ss;j++)
					{
					if (ossbincount[j] > 0)
						{
						oss[j] /= (double) ossbincount[j];
						ossalongtrack[j] /= (double) ossbincount[j];
						if (jport < 0)
							jport = j;
						jstbd = j;
						}
					else
						oss[j] = MB_SIDESCAN_NULL;
					}
/* fprintf(stderr,"SS bounds: %d %d      %f %f   %f\n",jport,jstbd,(jport - opixels_ss/2)*pixel_width,(jstbd - opixels_ss/2)*pixel_width,
(jport - opixels_ss/2)*pixel_width - (jstbd - opixels_ss/2)*pixel_width);*/
/*for (j=0;j<opixels_ss;j++)
{
fprintf(stderr,"AAA j:%d x:%7.2f l:%7.2f s:%6.2f\n",j,ossacrosstrack[j],ossalongtrack[j],oss[j]);
}*/

				/* interpolate gaps in the output sidescan */
				previous = opixels_ss;
				for (j=0;j<opixels_ss;j++)
					{
					if (ossbincount[j] > 0)
						{
						interpable = j - previous - 1;
						if (interpable > 0 && interpable <= interpolation)
							{
							dss = oss[j] - oss[previous];
							dssl = ossalongtrack[j] - ossalongtrack[previous];
							for (jj=previous+1;jj<j;jj++)
								{
								fraction = ((double)(jj - previous))
										/ ((double)(j - previous));
								oss[jj] = oss[previous] + fraction * dss;
								ossalongtrack[jj] = ossalongtrack[previous] + fraction * dssl;
								}
							}
						previous = j;
						}
					}
/*for (j=0;j<opixels_ss;j++)
{
fprintf(stderr,"III j:%d x:%7.2f l:%7.2f s:%6.2f\n",j,ossacrosstrack[j],ossalongtrack[j],oss[j]);
}*/

				/* insert data */
				mb_insert_nav(verbose, ombio_ptr, (void *)ostore,
						time_i, time_d,
						navlon, navlat, speed, heading, sensordraft,
						roll, pitch, heave,
						&error);
				status = mb_insert_altitude(verbose, ombio_ptr, (void *)ostore,
						sensordepth, ss_altitude, &error);
				status = mb_insert(verbose, ombio_ptr, (void *)ostore,
						MB_DATA_DATA, time_i, time_d,
						navlon, navlat, speed, heading,
						beams_bath,beams_amp,opixels_ss,
						beamflag,bath,amp,bathacrosstrack,
						bathalongtrack,
						oss,ossacrosstrack,ossalongtrack,
						comment, &error);
				}

			/* write some data */
			if (error == MB_ERROR_NO_ERROR
				&& kind == output_source)
				{
				/* write the record */
				status = mb_write_ping(verbose, ombio_ptr, (void *)ostore, &error);
				if (status != MB_SUCCESS)
					{
					mb_error(verbose,error,&message);
					fprintf(stderr,"\nMBIO Error returned from function <mb_put>:\n%s\n",message);
					fprintf(stderr,"\nMultibeam Data Not Written To File <%s>\n",ofile);
					fprintf(stderr,"\nProgram <%s> Terminated\n",
						program_name);
					exit(error);
					}
				
				/* count records */
				if (kind == MB_DATA_COMMENT)
					{
					n_wf_comment++;
					n_wt_comment++;
					}
				else
					{
					n_wf_data++;
					n_wt_data++;
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
			fprintf(stderr,"     %d sidescan2 records\n", n_rf_ss2);
			fprintf(stderr,"     %d sidescan3 records\n", n_rf_ss3);
			fprintf(stderr,"     %d subbottom records\n", n_rf_sbp);
			fprintf(stderr,"     %d nav records\n", n_rf_nav);
			fprintf(stderr,"     %d nav1 records\n", n_rf_nav1);
			fprintf(stderr,"     %d nav2 records\n", n_rf_nav2);
			fprintf(stderr,"     %d nav3 records\n", n_rf_nav3);
			}
	
		/* close the input swath file */
		status = mb_close(verbose,&imbio_ptr,&error);
	
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

	/* close any open output file */
	if (ombio_ptr != NULL)
		{
		/* close the swath file */
		status = mb_close(verbose,&ombio_ptr,&error);

		/* generate inf file */
		/* if (status == MB_SUCCESS)
			{
			status = mb_make_info(verbose, MB_YES,
						output_file,
						MBF_MBLDEOIH,
						&error);
			}*/

		/* output counts */
		if (verbose > 0)
			{
			fprintf(stdout, "\nClosing output file: %s\n", output_file);
			fprintf(stdout, "Pass 2: Records written to output file %s\n", output_file);
			fprintf(stdout, "     %d survey records\n", n_wf_data);
			fprintf(stdout, "     %d comment records\n", n_wf_comment);
			}

		/* output commands to first cut plotting script file */
		fprintf(sfp, "# Generate swath plot of sidescan file: %s\n", output_file);
		fprintf(sfp, "mbm_plot -I %s -N -G5 -S -Pb -V -O %s_ssrawplot\n",
			output_file, output_file);
		fprintf(sfp, "%s_ssrawplot.cmd $1\n\n", output_file);
		fprintf(sfp, "convert -density 100 %s_ssrawplot.ps -trim -quality 75 %s_ssrawplot.jpg", output_file, output_file);
		}

	/* close plotting script file */
	fclose(sfp);
	sprintf(command, "chmod +x %s", scriptfile);
	shellstatus = system(command);

	/* output data counts */
	if (verbose > 0)
		{
		fprintf(stderr,"\nPass 2: Total records read from all input files\n");
		fprintf(stderr,"     %d survey records\n", n_rt_data);
		fprintf(stderr,"     %d comment records\n", n_rt_comment);
		fprintf(stderr,"     %d sidescan2 records\n", n_rt_ss2);
		fprintf(stderr,"     %d sidescan3 records\n", n_rt_ss3);
		fprintf(stderr,"     %d subbottom records\n", n_rt_sbp);
		fprintf(stderr,"     %d nav records\n", n_rt_nav);
		fprintf(stderr,"     %d nav1 records\n", n_rt_nav1);
		fprintf(stderr,"     %d nav2 records\n", n_rt_nav2);
		fprintf(stderr,"     %d nav3 records\n", n_rt_nav3);
		fprintf(stderr,"Pass 2: Total records written to all output files\n");
		fprintf(stderr,"     %d survey records\n", n_wt_data);
		fprintf(stderr,"     %d comment records\n", n_wt_comment);
		}
		
	/* end second pass through data */
	
	/*-------------------------------------------------------------------*/
	
	/* deallocate raw sidescan arrays */
	if (num_samples_stbd_alloc > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&raw_samples_stbd,&error);
		num_samples_stbd_alloc = 0;
		}
	if (num_samples_port_alloc > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&raw_samples_port,&error);
		num_samples_port_alloc = 0;
		}

	/* deallocate nav, sensordepth, heading, attitude, and timeshift arrays */
	if (nav_alloc > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nav_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nav_navlon,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nav_navlat,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nav_speed,&error);
		nav_alloc = 0;
		}
	if (sensordepth_alloc > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&sensordepth_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&sensordepth_sensordepth,&error);
		sensordepth_alloc = 0;
		}
	if (heading_alloc > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&heading_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&heading_heading,&error);
		heading_alloc = 0;
		}
	if (attitude_alloc > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&attitude_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&attitude_roll,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&attitude_pitch,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&attitude_heave,&error);
		attitude_alloc = 0;
		}
	if (timeshift_alloc > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&timeshift_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&timeshift_timeshift,&error);
		timeshift_alloc = 0;
		}

	/* deallocate route arrays */
	if (line_mode == MBSSLAYOUT_LINE_TIME)
		{
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&routelon, &error);
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&routelat, &error);
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&routeheading, &error);
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&routewaypoint, &error);
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&routetime_d, &error);
		}
	else if (line_mode == MBSSLAYOUT_LINE_ROUTE)
		{
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&routelon, &error);
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&routelat, &error);
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&routeheading, &error);
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&routewaypoint, &error);
		}

	/* deallocate topography grid array if necessary */
	if (layout_mode == MBSSLAYOUT_LAYOUT_3DTOPO)
		status = mb_topogrid_deall(verbose, &topogrid_ptr, &error);

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
int mbsslayout_get_flatbottom_table(int verbose, int nangle, double angle_min, double angle_max,
					double navlon, double navlat, double altitude, double pitch,
					double *table_angle, double *table_xtrack, double *table_ltrack,
					double *table_altitude, double *table_range,
					int *error)
{
	char	*function_name = "mbsslayout_get_flatbottom_table";
	int	status = MB_SUCCESS;
	double	dangle;
	double	rr, xx, zz;
	double	alpha, beta, theta, phi;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSSLAYOUT function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n", verbose);
		fprintf(stderr,"dbg2       nangle:          %d\n", nangle);
		fprintf(stderr,"dbg2       angle_min:       %f\n", angle_min);
		fprintf(stderr,"dbg2       angle_max:       %f\n", angle_max);
		fprintf(stderr,"dbg2       navlon:          %f\n", navlon);
		fprintf(stderr,"dbg2       navlat:          %f\n", navlat);
		fprintf(stderr,"dbg2       pitch:           %f\n", pitch);
		}

	/* loop over all of the angles */
	dangle = (angle_max - angle_min) / (nangle - 1);
	alpha = pitch;
	zz = altitude;
	for (i=0;i<nangle;i++)
		{
		/* get angles in takeoff coordinates */
		table_angle[i] = angle_min + dangle * i;
		beta = 90.0 - table_angle[i];
		mb_rollpitch_to_takeoff(
			verbose,
			alpha, beta,
			&theta, &phi,
			error);

		/* calculate range required to achieve desired altitude */
		rr = zz / cos(DTR * theta);

		/* get the position */
		xx = rr * sin(DTR * theta);
		table_xtrack[i] = xx * cos(DTR * phi);
		table_ltrack[i] = xx * sin(DTR * phi);
		table_altitude[i] = zz;
		table_range[i] = rr;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBSSLAYOUT function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       Lookup tables:\n");
		for (i=0;i<nangle;i++)
			fprintf(stderr,"dbg2         %d %f %f %f %f %f\n",
				i, table_angle[i], table_xtrack[i], table_ltrack[i], table_altitude[i], table_range[i]);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
