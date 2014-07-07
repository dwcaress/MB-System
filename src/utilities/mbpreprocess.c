/*--------------------------------------------------------------------
 *    The MB-system:	mbpreprocess.c	1/8/2014
 *    $Id$
 *
 *    Copyright (c) 2014-2014 by
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
#define MBPREPROCESS_TIMESHIFT_OFF 	0
#define MBPREPROCESS_TIMESHIFT_FILE 	1
#define MBPREPROCESS_TIMESHIFT_CONSTANT 2

#define MBPREPROCESS_TIMESHIFT_APPLY_NONE		0x00
#define MBPREPROCESS_TIMESHIFT_APPLY_NAV		0x01
#define MBPREPROCESS_TIMESHIFT_APPLY_SENSORDEPTH	0x02
#define MBPREPROCESS_TIMESHIFT_APPLY_HEADING		0x04
#define MBPREPROCESS_TIMESHIFT_APPLY_ATTITUDE		0x08
#define MBPREPROCESS_TIMESHIFT_APPLY_ALL_ANCILLIARY	0x7F
#define MBPREPROCESS_TIMESHIFT_APPLY_SURVEY		0x80
#define MBPREPROCESS_TIMESHIFT_APPLY_ALL		0xFF

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
	 * 		--format=format
	 * 		--nav_file=file
	 * 		--nav_file_format=format_id
	 * 		--nav_async=record_kind
	 * 		--sensordepth_file=file
	 * 		--sensordepth_file_format=format_id
	 * 		--sensordepth_async=record_kind
	 * 		--heading_file=file
	 * 		--heading_file_format=format_id
	 * 		--heading_async=record_kind
	 * 		--attitude_file=file
	 * 		--attitude_file_format=format_id
	 * 		--attitude_async=record_kind
	 * 		--sensor_offsets=offset_file
	 */
	static struct option options[] =
		{
		{"verbose",			no_argument, 		NULL, 		0},
		{"help",			no_argument, 		NULL, 		0},
		{"verbose",			no_argument, 		NULL, 		0},
		{"input",			required_argument, 	NULL, 		0},
		{"format",			required_argument, 	NULL, 		0},
		{"nav_file",			required_argument, 	NULL, 		0},
		{"nav_file_format",		required_argument, 	NULL, 		0},
		{"nav_async",			required_argument, 	NULL, 		0},
		{"sensordepth_file",		required_argument, 	NULL, 		0},
		{"sensordepth_file_format",	required_argument, 	NULL, 		0},
		{"sensordepth_async",		required_argument, 	NULL, 		0},
		{"heading_file",		required_argument, 	NULL, 		0},
		{"heading_file_format",		required_argument, 	NULL, 		0},
		{"heading_async",		required_argument, 	NULL, 		0},
		{"attitude_file",		required_argument, 	NULL, 		0},
		{"attitude_file_format",	required_argument, 	NULL, 		0},
		{"attitude_async",		required_argument, 	NULL, 		0},
		{"timeshift_file",		required_argument, 	NULL, 		0},
		{"timeshift_constant",		required_argument, 	NULL, 		0},
		{"timeshift_apply_nav",		no_argument, 		NULL, 		0},
		{"timeshift_apply_sensordepth",	no_argument, 		NULL, 		0},
		{"timeshift_apply_heading",	no_argument, 		NULL, 		0},
		{"timeshift_apply_attitude",	no_argument, 		NULL, 		0},
		{"timeshift_apply_all_ancilliary",	no_argument, 		NULL, 		0},
		{"timeshift_apply_survey",	no_argument, 		NULL, 		0},
		{"timeshift_apply_all",		no_argument, 		NULL, 		0},
		{"sensor_offsets",		required_argument, 	NULL, 		0},
		{NULL,				0, 			NULL, 		0}
		};

	/* asynchronous navigation, heading, attitude data */
	int	nav_mode = MBPREPROCESS_MERGE_OFF;
	mb_path	nav_file;
	int	nav_file_format = 0;
	int	nav_async = MB_DATA_DATA;
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
	int	sensordepth_num = 0;
	int	sensordepth_alloc = 0;
	double	*sensordepth_time_d = NULL;
	double	*sensordepth_sensordepth = NULL;

	int	heading_mode = MBPREPROCESS_MERGE_OFF;
	mb_path	heading_file;
	int	heading_file_format = 0;
	int	heading_async = MB_DATA_DATA;
	int	heading_num = 0;
	int	heading_alloc = 0;
	double	*heading_time_d = NULL;
	double	*heading_heading = NULL;

	int	attitude_mode = MBPREPROCESS_MERGE_OFF;
	mb_path	attitude_file;
	int	attitude_file_format = 0;
	int	attitude_async = MB_DATA_DATA;
	int	attitude_num = 0;
	int	attitude_alloc = 0;
	double	*attitude_time_d = NULL;
	double	*attitude_roll = NULL;
	double	*attitude_pitch = NULL;
	double	*attitude_heave = NULL;

	int	timeshift_mode = MBPREPROCESS_TIMESHIFT_OFF;
	mb_u_char timeshift_apply = MBPREPROCESS_TIMESHIFT_APPLY_NONE;
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
	int	n_rt_data = 0;
	int	n_rt_comment = 0;
	int	n_rt_nav = 0;
	int	n_rt_nav1 = 0;
	int	n_rt_nav2 = 0;
	int	n_rt_nav3 = 0;
	
	int	n_wf_data = 0;
	int	n_wf_comment = 0;
	int	n_wf_nav = 0;
	int	n_wf_nav1 = 0;
	int	n_wf_nav2 = 0;
	int	n_wf_nav3 = 0;
	int	n_wt_data = 0;
	int	n_wt_comment = 0;
	int	n_wt_nav = 0;
	int	n_wt_nav1 = 0;
	int	n_wt_nav2 = 0;
	int	n_wt_nav3 = 0;
	
	int	testformat;
	int	interp_status = MB_SUCCESS;
	int	interp_error = MB_ERROR_NO_ERROR;
	double	timeshift;
	int	jsurvey = 0;
	int	jnav = 0;
	int	jsensordepth = 0;
	int	jheading = 0;
	int	jattitude = 0;
	int	survey_changed;
	int	nav_changed;
	int	sensordepth_changed;
	int	heading_changed;
	int	attitude_changed;
	int	i, j, n;

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
			 * Define source of navigation - could be an external file
			 * or an internal asynchronous record */
			
			/* nav_file */
			else if (strcmp("nav_file", options[option_index].name) == 0)
				{
				strcpy(nav_file, optarg);
				nav_mode = MBPREPROCESS_MERGE_FILE;
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
					nav_mode = MBPREPROCESS_MERGE_ASYNC;
				}
				
			/*-------------------------------------------------------
			 * Define source of sensordepth - could be an external file
			 * or an internal asynchronous record */
			
			/* sensordepth_file */
			else if (strcmp("sensordepth_file", options[option_index].name) == 0)
				{
				strcpy(sensordepth_file, optarg);
				sensordepth_mode = MBPREPROCESS_MERGE_FILE;
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
					sensordepth_mode = MBPREPROCESS_MERGE_ASYNC;
				}
				
			/*-------------------------------------------------------
			 * Define source of heading - could be an external file
			 * or an internal asynchronous record */
			
			/* heading_file */
			else if (strcmp("heading_file", options[option_index].name) == 0)
				{
				strcpy(heading_file, optarg);
				heading_mode = MBPREPROCESS_MERGE_FILE;
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
					heading_mode = MBPREPROCESS_MERGE_ASYNC;
				}
				
			/*-------------------------------------------------------
			 * Define source of attitude - could be an external file
			 * or an internal asynchronous record */
			
			/* attitude_file */
			else if (strcmp("attitude_file", options[option_index].name) == 0)
				{
				strcpy(attitude_file, optarg);
				attitude_mode = MBPREPROCESS_MERGE_FILE;
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
					attitude_mode = MBPREPROCESS_MERGE_ASYNC;
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
				timeshift_mode = MBPREPROCESS_TIMESHIFT_FILE;
				}
			
			/* timeshift_constant */
			else if (strcmp("timeshift_constant", options[option_index].name) == 0)
				{
				n = sscanf(optarg, "%lf", &timeshift_constant);
				if (n == 1)
					timeshift_mode = MBPREPROCESS_TIMESHIFT_CONSTANT;
				}
			
			/* timeshift_apply_nav */
			else if (strcmp("timeshift_apply_nav", options[option_index].name) == 0)
				{
				timeshift_apply =  timeshift_apply | MBPREPROCESS_TIMESHIFT_APPLY_NAV;
				}
			
			/* timeshift_apply_sensordepth */
			else if (strcmp("timeshift_apply_sensordepth", options[option_index].name) == 0)
				{
				timeshift_apply =  timeshift_apply | MBPREPROCESS_TIMESHIFT_APPLY_SENSORDEPTH;
				}
			
			/* timeshift_apply_heading */
			else if (strcmp("timeshift_apply_heading", options[option_index].name) == 0)
				{
				timeshift_apply =  timeshift_apply | MBPREPROCESS_TIMESHIFT_APPLY_HEADING;
				}
			
			/* timeshift_apply_attitude */
			else if (strcmp("timeshift_apply_attitude", options[option_index].name) == 0)
				{
				timeshift_apply =  timeshift_apply | MBPREPROCESS_TIMESHIFT_APPLY_ATTITUDE;
				}
			
			/* timeshift_apply_all_ancilliary */
			else if (strcmp("timeshift_apply_all_ancilliary", options[option_index].name) == 0)
				{
				timeshift_apply =  MBPREPROCESS_TIMESHIFT_APPLY_ALL_ANCILLIARY;
				}
			
			/* timeshift_apply_survey */
			else if (strcmp("timeshift_apply_survey", options[option_index].name) == 0)
				{
				timeshift_apply =  MBPREPROCESS_TIMESHIFT_APPLY_SURVEY;
				}
			
			/* timeshift_apply_all */
			else if (strcmp("timeshift_apply_all", options[option_index].name) == 0)
				{
				timeshift_apply =  MBPREPROCESS_TIMESHIFT_APPLY_ALL;
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
		fprintf(stderr,"dbg2       nav_mode:             %d\n",nav_mode);
		fprintf(stderr,"dbg2       nav_file:             %s\n",nav_file);
		fprintf(stderr,"dbg2       nav_file_format:           %d\n",nav_file_format);
		fprintf(stderr,"dbg2       nav_async:            %d\n",nav_async);
		fprintf(stderr,"dbg2       sensordepth_mode:     %d\n",sensordepth_mode);
		fprintf(stderr,"dbg2       sensordepth_file:     %s\n",sensordepth_file);
		fprintf(stderr,"dbg2       sensordepth_file_format:   %d\n",sensordepth_file_format);
		fprintf(stderr,"dbg2       sensordepth_async:    %d\n",sensordepth_async);
		fprintf(stderr,"dbg2       heading_mode:         %d\n",heading_mode);
		fprintf(stderr,"dbg2       heading_file:         %s\n",heading_file);
		fprintf(stderr,"dbg2       heading_file_format:       %d\n",heading_file_format);
		fprintf(stderr,"dbg2       heading_async:        %d\n",heading_async);
		fprintf(stderr,"dbg2       attitude_mode:        %d\n",attitude_mode);
		fprintf(stderr,"dbg2       attitude_file:        %s\n",attitude_file);
		fprintf(stderr,"dbg2       attitude_file_format:      %d\n",attitude_file_format);
		fprintf(stderr,"dbg2       attitude_async:       %d\n",attitude_async);
		fprintf(stderr,"dbg2       timeshift_mode:       %d\n",timeshift_mode);
		fprintf(stderr,"dbg2       timeshift_file:       %s\n",timeshift_file);
		fprintf(stderr,"dbg2       timeshift_format:     %d\n",timeshift_format);
		fprintf(stderr,"dbg2       timeshift_apply:      %x\n",timeshift_apply);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}
		
	/* load ancilliary data from external files if requested */
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
	if (attitude_mode == MBPREPROCESS_MERGE_FILE)
		{
		mb_loadattitudedata(verbose, attitude_file, attitude_file_format,
			       &attitude_num, &attitude_alloc,
			       &attitude_time_d, &attitude_roll, &attitude_pitch, &attitude_heave, &error);
		
		if (verbose > 0)
			fprintf(stderr,"%d attitude records loaded from file %s\n", heading_num, heading_file);
		}
	if (timeshift_mode == MBPREPROCESS_MERGE_FILE)
		{
		mb_loadtimeshiftdata(verbose, timeshift_file, timeshift_format,
			       &timeshift_num, &timeshift_alloc,
			       &timeshift_time_d, &timeshift_timeshift, &error);
		
		if (verbose > 0)
			fprintf(stderr,"%d timeshift records loaded from file %s\n", heading_num, heading_file);
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
				
			/* look for nav if not externally defined */
			if (status == MB_SUCCESS
				&& nav_mode == MBPREPROCESS_MERGE_ASYNC
				&& kind == sensordepth_async)
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
						nav_time_d[nav_num] = atime_d[i];
						nav_navlon[nav_num] = alon[i];
						nav_navlat[nav_num] = alat[i];
						nav_speed[nav_num] = aspeed[i];
						nav_num++;
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
				
			/* look for attitude if not externally defined */
			if (status == MB_SUCCESS
				&& attitude_mode == MBPREPROCESS_MERGE_ASYNC
				&& kind == sensordepth_async)
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
						attitude_roll[attitude_num] = alon[i];
						attitude_pitch[attitude_num] = alat[i];
						attitude_heave[attitude_num] = aspeed[i];
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
		fprintf(stderr,"     %d nav records\n", n_rt_nav);
		fprintf(stderr,"     %d nav1 records\n", n_rt_nav1);
		fprintf(stderr,"     %d nav2 records\n", n_rt_nav2);
		fprintf(stderr,"     %d nav3 records\n", n_rt_nav3);
		}
		
	/* end first pass through data */
	
	/*-------------------------------------------------------------------*/
	
	/* Apply any specified timeshift to the chosen data */
	if (timeshift_mode != MBPREPROCESS_TIMESHIFT_OFF)
		{
		/* if no affected data have been specified apply timeshift to all */
		if (timeshift_apply == MBPREPROCESS_TIMESHIFT_APPLY_NONE)
			timeshift_apply =  MBPREPROCESS_TIMESHIFT_APPLY_ALL_ANCILLIARY;
			
		/* apply timeshift to nav data */
		if (timeshift_apply &  MBPREPROCESS_TIMESHIFT_APPLY_NAV)
			{
			if (timeshift_mode == MBPREPROCESS_TIMESHIFT_FILE)
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
			else if (timeshift_mode == MBPREPROCESS_TIMESHIFT_CONSTANT)
				{
				for (i=0;i<nav_num;i++)
					{
					nav_time_d[i] -= timeshift_constant;
					}
				}
			}
			
		/* apply timeshift to sensordepth data */
		if (timeshift_apply & MBPREPROCESS_TIMESHIFT_APPLY_SENSORDEPTH)
			{
			if (timeshift_mode == MBPREPROCESS_TIMESHIFT_FILE)
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
			else if (timeshift_mode == MBPREPROCESS_TIMESHIFT_CONSTANT)
				{
				for (i=0;i<sensordepth_num;i++)
					{
					sensordepth_time_d[i] -= timeshift_constant;
					}
				}
			}
			
		/* apply timeshift to heading data */
		if (timeshift_apply & MBPREPROCESS_TIMESHIFT_APPLY_HEADING)
			{
			if (timeshift_mode == MBPREPROCESS_TIMESHIFT_FILE)
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
			else if (timeshift_mode == MBPREPROCESS_TIMESHIFT_CONSTANT)
				{
				for (i=0;i<heading_num;i++)
					{
					heading_time_d[i] -= timeshift_constant;
					}
				}
			}
			
		/* apply timeshift to attitude data */
		if (timeshift_apply & MBPREPROCESS_TIMESHIFT_APPLY_ATTITUDE)
			{
			if (timeshift_mode == MBPREPROCESS_TIMESHIFT_FILE)
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
			else if (timeshift_mode == MBPREPROCESS_TIMESHIFT_CONSTANT)
				{
				for (i=0;i<attitude_num;i++)
					{
					attitude_time_d[i] -= timeshift_constant;
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
	n_rf_nav = 0;
	n_rf_nav1 = 0;
	n_rf_nav2 = 0;
	n_rf_nav3 = 0;
	n_rt_data = 0;
	n_rt_comment = 0;
	n_rt_nav = 0;
	n_rt_nav1 = 0;
	n_rt_nav2 = 0;
	n_rt_nav3 = 0;
	n_wf_data = 0;
	n_wf_comment = 0;
	n_wf_nav = 0;
	n_wf_nav1 = 0;
	n_wf_nav2 = 0;
	n_wf_nav3 = 0;
	n_wt_data = 0;
	n_wt_comment = 0;
	n_wt_nav = 0;
	n_wt_nav1 = 0;
	n_wt_nav2 = 0;
	n_wt_nav3 = 0;

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
				
		/* zero file count records */
		n_rf_data = 0;
		n_rf_comment = 0;
		n_rf_nav = 0;
		n_rf_nav1 = 0;
		n_rf_nav2 = 0;
		n_rf_nav3 = 0;
		n_wf_data = 0;
		n_wf_comment = 0;
		n_wf_nav = 0;
		n_wf_nav1 = 0;
		n_wf_nav2 = 0;
		n_wf_nav3 = 0;

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
				/* start out with no change defined */
				survey_changed = MB_NO;
				nav_changed = MB_NO;
				sensordepth_changed = MB_NO;
				heading_changed = MB_NO;
				attitude_changed = MB_NO;
				
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
					
				/* apply timeshift to survey data */
				if (timeshift_apply & MBPREPROCESS_TIMESHIFT_APPLY_SURVEY)
					{
					if (timeshift_mode == MBPREPROCESS_TIMESHIFT_FILE)
						{
						interp_status = mb_linear_interp(verbose,
										timeshift_time_d-1, timeshift_timeshift-1,
										timeshift_num, time_d, &timeshift, &jsurvey,
										&interp_error);
						time_d += timeshift;						
						}
					else if (timeshift_mode == MBPREPROCESS_TIMESHIFT_CONSTANT)
						{
						time_d += timeshift_constant;
						}
					survey_changed = MB_YES;
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
					
				/* attempt to execute a preprocess function for these data */
				status = mb_preprocess(verbose, imbio_ptr, istore_ptr,
							time_d, navlon, navlat, speed,
							heading, sensordepth,
							roll, pitch,heave, &error);
				if (status == MB_SUCCESS)
					survey_changed = MB_YES;

				
				/* if a predefined preprocess function does not exist then
				 * execute standard preprocessing:
				 * 	1) if attitude values changed rotate bathymetry accordingly
				 * 	2) if any values changed reinsert the data */
				else if (status == MB_FAILURE)
					{
					/* reset status and error */
					status = MB_SUCCESS;
					error = MB_ERROR_NO_ERROR;
					
					/* if attitude changed rotate the bathymetry */
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
			fprintf(stderr,"Pass 2: Records written to output file %s\n", ofile);
			fprintf(stderr,"     %d survey records\n", n_wf_data);
			fprintf(stderr,"     %d comment records\n", n_wf_comment);
			fprintf(stderr,"     %d nav records\n", n_wf_nav);
			fprintf(stderr,"     %d nav1 records\n", n_wf_nav1);
			fprintf(stderr,"     %d nav2 records\n", n_wf_nav2);
			fprintf(stderr,"     %d nav3 records\n", n_wf_nav3);
			}
	
		/* close the input swath file */
		status = mb_close(verbose,&imbio_ptr,&error);

		/* close the output swath file */
		status = mb_close(verbose,&ombio_ptr,&error);
	
		/* generate inf fnv and fbt files */
		if (status == MB_SUCCESS)
			status = mb_make_info(verbose, MB_YES, ofile, oformat, &error);
	
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
		fprintf(stderr,"Pass 2: Total records written to all output files\n");
		fprintf(stderr,"     %d survey records\n", n_wt_data);
		fprintf(stderr,"     %d comment records\n", n_wt_comment);
		fprintf(stderr,"     %d nav records\n", n_wt_nav);
		fprintf(stderr,"     %d nav1 records\n", n_wt_nav1);
		fprintf(stderr,"     %d nav2 records\n", n_wt_nav2);
		fprintf(stderr,"     %d nav3 records\n", n_wt_nav3);
		}
		
	/* end second pass through data */
	
	/*-------------------------------------------------------------------*/

	/* deallocate nav, sensordepth, heading, attitude, and timeshift arrays */
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
	if (timeshift_alloc > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&timeshift_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&timeshift_timeshift,&error);
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
