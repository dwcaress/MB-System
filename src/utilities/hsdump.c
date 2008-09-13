/*--------------------------------------------------------------------
 *    The MB-system:	hsdump.c	6/16/93
 *    $Id: hsdump.c,v 5.8 2008-09-13 06:08:09 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000, 2003, 2006 by
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
 * HSDUMP lists the information contained in data records on
 * Hydrosweep DS data files, including survey, calibrate, water 
 * velocity and comment records. The default input stream is stdin.
 *
 * Author:	D. W. Caress
 * Date:	June 16, 1993
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.7  2008/09/11 20:20:14  caress
 * Checking in updates made during cruise AT15-36.
 *
 * Revision 5.6  2006/01/24 19:12:01  caress
 * Version 5.0.8 beta.
 *
 * Revision 5.5  2005/03/25 04:42:59  caress
 * Standardized the string lengths used for filenames and comment data.
 *
 * Revision 5.4  2003/04/17 21:17:10  caress
 * Release 5.0.beta30
 *
 * Revision 5.3  2002/10/02 23:56:06  caress
 * Release 5.0.beta24
 *
 * Revision 5.2  2001/07/20 00:34:38  caress
 * Release 5.0.beta03
 *
 * Revision 5.1  2001/03/22 21:14:16  caress
 * Trying to make release 5.0.beta0.
 *
 * Revision 5.0  2000/12/01  22:57:08  caress
 * First cut at Version 5.0.
 *
 * Revision 4.11  2000/10/11  01:06:15  caress
 * Convert to ANSI C
 *
 * Revision 4.10  2000/09/30  07:06:28  caress
 * Snapshot for Dale.
 *
 * Revision 4.9  1998/10/05  19:19:24  caress
 * MB-System version 4.6beta
 *
 * Revision 4.8  1997/04/21  17:19:14  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.7  1996/04/22  13:23:05  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.7  1996/04/22  13:23:05  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.6  1995/05/12  17:12:32  caress
 * Made exit status values consistent with Unix convention.
 * 0: ok  nonzero: error
 *
 * Revision 4.5  1995/03/06  19:37:59  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.4  1995/03/02  13:49:21  caress
 * Fixed bug related to error messages.
 *
 * Revision 4.3  1994/10/21  13:02:31  caress
 * Release V4.0
 *
 * Revision 4.2  1994/06/03  23:54:03  caress
 * Added format_num and fixed call to mb_format_inf.
 *
 * Revision 4.1  1994/03/12  01:44:37  caress
 * Added declarations of ctime and/or getenv for compatability
 * with SGI compilers.
 *
 * Revision 4.0  1994/03/06  00:13:22  caress
 * First cut at version 4.0
 *
 * Revision 4.0  1994/03/01  18:59:27  caress
 * First cut at new version. Any changes are associated with
 * support of three data types (beam bathymetry, beam amplitude,
 * and sidescan) instead of two (bathymetry and backscatter).
 *
 * Revision 3.1  1993/06/30  21:51:31  caress
 * Fixed some debug messages.
 *
 * Revision 3.0  1993/06/16  23:07:10  caress
 * Initial version derived from old program hsveldump.
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"
#include "../../include/mbsys_hsds.h"

/*--------------------------------------------------------------------*/

main (int argc, char **argv)
{
	/* id variables */
	static char rcs_id[] = "$Id: hsdump.c,v 5.8 2008-09-13 06:08:09 caress Exp $";
	static char program_name[] = "HSDUMP";
	static char help_message[] =  "HSDUMP lists the information contained in data records on\n\tHydrosweep DS data files, including survey, calibrate, water \n\tvelocity and comment records. The default input stream is stdin.";
	static char usage_message[] = "hsdump [-Fformat -V -H -Iinfile -Okind]";

	/* parsing variables */
	extern char *optarg;
	extern int optkind;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* MBIO status variables */
	int	status = MB_SUCCESS;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;
	char	format_description[MB_DESCRIPTION_LENGTH];
	char	*message = NULL;

	/* MBIO read and write control parameters */
	int	format = 0;
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	char	file[MB_PATH_MAXLINE];
	void	*mbio_ptr = NULL;

	/* mbio read and write values */
	void	*store_ptr;
	struct mbsys_hsds_struct *store;
	int	kind;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	double	altitude;
	double	sonardepth;
	int	nbath;
	int	namp;
	int	nss;
	char	*beamflag = NULL;
	double	*bath = NULL;
	double	*bathacrosstrack = NULL;
	double	*bathalongtrack = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*ssacrosstrack = NULL;
	double	*ssalongtrack = NULL;
	char	comment[MB_COMMENT_MAXLINE];

	/* dump control parameters */
	int	mb_data_data_list = MB_NO;
	int	mb_data_comment_list = MB_NO;
	int	mb_data_calibrate_list = MB_NO;
	int	mb_data_mean_velocity_list = MB_NO;
	int	mb_data_velocity_profile_list = MB_NO;
	int	mb_data_standby_list = MB_NO;
	int	mb_data_nav_source_list = MB_NO;
	int	mb_data_data_count = 0;
	int	mb_data_comment_count = 0;
	int	mb_data_calibrate_count = 0;
	int	mb_data_mean_velocity_count = 0;
	int	mb_data_velocity_profile_count = 0;
	int	mb_data_standby_count = 0;
	int	mb_data_nav_source_count = 0;

	/* output stream for basic stuff (stdout if verbose <= 1,
		stderr if verbose > 1) */
	FILE	*output;

	int	i, j, k, l, m;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* reset all defaults */
	format = MBF_HSATLRAW;
	pings = 1;
	lonflip = 0;
	bounds[0] = -360.;
	bounds[1] = 360.;
	bounds[2] = -90.;
	bounds[3] = 90.;
	btime_i[0] = 1962;
	btime_i[1] = 2;
	btime_i[2] = 21;
	btime_i[3] = 10;
	btime_i[4] = 30;
	btime_i[5] = 0;
	btime_i[6] = 0;
	etime_i[0] = 2062;
	etime_i[1] = 2;
	etime_i[2] = 21;
	etime_i[3] = 10;
	etime_i[4] = 30;
	etime_i[5] = 0;
	etime_i[6] = 0;
	speedmin = 0.0;
	timegap = 1000000000.0;

	/* set default input and output */
	strcpy (file, "stdin");

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhF:f:I:i:O:o:")) != -1)
	  switch (c) 
		{
		case 'H':
		case 'h':
			help++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &format);
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", file);
			flag++;
			break;
		case 'O':
		case 'o':
			sscanf (optarg,"%d", &kind);
			if (kind == MB_DATA_DATA)
				mb_data_data_list = MB_YES;
			if (kind == MB_DATA_COMMENT)
				mb_data_comment_list = MB_YES;
			if (kind == MB_DATA_CALIBRATE)
				mb_data_calibrate_list = MB_YES;
			if (kind == MB_DATA_MEAN_VELOCITY)
				mb_data_mean_velocity_list = MB_YES;
			if (kind == MB_DATA_VELOCITY_PROFILE)
				mb_data_velocity_profile_list = MB_YES;
			if (kind == MB_DATA_STANDBY)
				mb_data_standby_list = MB_YES;
			if (kind == MB_DATA_NAV_SOURCE)
				mb_data_nav_source_list = MB_YES;
			flag++;
			break;
		case '?':
			errflg++;
		}

	/* set output stream */
	if (verbose <= 1)
		output = stdout;
	else
		output = stderr;

	/* if error flagged then print it and exit */
	if (errflg)
		{
		fprintf(output,"usage: %s\n", usage_message);
		fprintf(output,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
		}

	/* print starting message */
	if (verbose == 1 || help)
		{
		fprintf(output,"\nProgram %s\n",program_name);
		fprintf(output,"Version %s\n",rcs_id);
		fprintf(output,"MB-system Version %s\n",MB_VERSION);
		}

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(output,"\ndbg2  Program <%s>\n",program_name);
		fprintf(output,"dbg2  Version %s\n",rcs_id);
		fprintf(output,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(output,"dbg2  Control Parameters:\n");
		fprintf(output,"dbg2       verbose:         %d\n",verbose);
		fprintf(output,"dbg2       help:            %d\n",help);
		fprintf(output,"dbg2       format:          %d\n",format);
		fprintf(output,"dbg2       pings:           %d\n",pings);
		fprintf(output,"dbg2       lonflip:         %d\n",lonflip);
		fprintf(output,"dbg2       bounds[0]:       %f\n",bounds[0]);
		fprintf(output,"dbg2       bounds[1]:       %f\n",bounds[1]);
		fprintf(output,"dbg2       bounds[2]:       %f\n",bounds[2]);
		fprintf(output,"dbg2       bounds[3]:       %f\n",bounds[3]);
		fprintf(output,"dbg2       btime_i[0]:      %d\n",btime_i[0]);
		fprintf(output,"dbg2       btime_i[1]:      %d\n",btime_i[1]);
		fprintf(output,"dbg2       btime_i[2]:      %d\n",btime_i[2]);
		fprintf(output,"dbg2       btime_i[3]:      %d\n",btime_i[3]);
		fprintf(output,"dbg2       btime_i[4]:      %d\n",btime_i[4]);
		fprintf(output,"dbg2       btime_i[5]:      %d\n",btime_i[5]);
		fprintf(output,"dbg2       btime_i[6]:      %d\n",btime_i[6]);
		fprintf(output,"dbg2       etime_i[0]:      %d\n",etime_i[0]);
		fprintf(output,"dbg2       etime_i[1]:      %d\n",etime_i[1]);
		fprintf(output,"dbg2       etime_i[2]:      %d\n",etime_i[2]);
		fprintf(output,"dbg2       etime_i[3]:      %d\n",etime_i[3]);
		fprintf(output,"dbg2       etime_i[4]:      %d\n",etime_i[4]);
		fprintf(output,"dbg2       etime_i[5]:      %d\n",etime_i[5]);
		fprintf(output,"dbg2       etime_i[6]:      %d\n",etime_i[6]);
		fprintf(output,"dbg2       speedmin:        %f\n",speedmin);
		fprintf(output,"dbg2       timegap:         %f\n",timegap);
		fprintf(output,"dbg2       input file:      %s\n",file);
		fprintf(output,"dbg2       mb_data_data_list:             %d\n",
			mb_data_data_list);
		fprintf(output,"dbg2       mb_data_comment_list:          %d\n",
			mb_data_comment_list);
		fprintf(output,"dbg2       mb_data_calibrate_list:        %d\n",
			mb_data_calibrate_list);
		fprintf(output,"dbg2       mb_data_mean_velocity_list:    %d\n",
			mb_data_mean_velocity_list);
		fprintf(output,"dbg2       mb_data_velocity_profile_list: %d\n",
			mb_data_velocity_profile_list);
		fprintf(output,"dbg2       mb_data_standby_list:          %d\n",
			mb_data_standby_list);
		fprintf(output,"dbg2       mb_data_nav_source_list:       %d\n",
			mb_data_nav_source_list);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(output,"\n%s\n",help_message);
		fprintf(output,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* if bad format specified then print it and exit */
	status = mb_format(verbose,&format,&error);
	if (format != MBF_HSATLRAW && format != MBF_HSLDEOIH)
		{
		fprintf(output,"\nProgram <%s> requires complete Hydrosweep DS data stream\n",program_name);
		fprintf(output,"!!Format %d is unacceptable, only formats %d and %d can be used\n",format,MBF_HSATLRAW,MBF_HSLDEOIH);
		fprintf(output,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_FORMAT;
		exit(error);
		}

	/* initialize reading the input multibeam file */
	if ((status = mb_read_init(
		verbose,file,format,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&mbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(output,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(output,"\nMultibeam File <%s> not initialized for reading\n",file);
		fprintf(output,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* allocate memory for data arrays */
	status = mb_mallocd(verbose,__FILE__,__LINE__,beams_bath*sizeof(char),(void **)&beamflag,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,beams_bath*sizeof(double),(void **)&bath,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,beams_bath*sizeof(double),
				(void **)&bathacrosstrack,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,beams_bath*sizeof(double),
				(void **)&bathalongtrack,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,beams_amp*sizeof(double),(void **)&amp,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,pixels_ss*sizeof(double),(void **)&ss,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,pixels_ss*sizeof(double),
			(void **)&ssacrosstrack,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,pixels_ss*sizeof(double),
			(void **)&ssalongtrack,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(output,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(output,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* printf out file and format */
	mb_format_description(verbose, &format, format_description, &error);
	fprintf(output,"\nHydrosweep DS Data File:  %s\n",file);
	fprintf(output,"MBIO Data Format ID:  %d\n",format);
	fprintf(output,"%s",format_description);

	/* read and list */
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* read some data */
		error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		status = mb_get_all(verbose,mbio_ptr,&store_ptr,&kind,
				time_i,&time_d,&navlon,&navlat,
				&speed,&heading,
				&distance,&altitude,&sonardepth,
				&nbath,&namp,&nss,
				beamflag,bath,amp,bathacrosstrack,bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment,&error);

		/* get data structure pointer */
		store = (struct mbsys_hsds_struct *) store_ptr;

		/* non-survey data do not matter to hsdump */
		if (error >= MB_ERROR_OTHER && error < MB_ERROR_NO_ERROR)
			{
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
			}

		/* output error messages */
		if (verbose >= 1 && error <= MB_ERROR_OTHER)
			{
			mb_error(verbose,error,&message);
			fprintf(output,"\nNonfatal MBIO Error:\n%s\n",message);
			}
		else if (verbose >= 1 && error > MB_ERROR_NO_ERROR 
			&& error != MB_ERROR_EOF)
			{
			mb_error(verbose,error,&message);
			fprintf(output,"\nFatal MBIO Error:\n%s\n",message);
			}

		/* deal with survey data record */
		if (kind == MB_DATA_DATA && mb_data_data_list == MB_YES)
			{
			mb_data_data_count++;
			fprintf(output,"\n");
			fprintf(output,"Survey Data Record (ERGNMESS + ERGNSLZT +ERGNAMPL):\n");
			fprintf(output,"  Time:            %2d/%2d/%4d %2.2d:%2.2d:%2.2d\n",
				store->month,store->day,store->year,
				store->hour,store->minute,store->second);
			fprintf(output,"  Alternate Time:   %4d  %4d\n",
				store->alt_minute,store->alt_second);
			fprintf(output,"  Longitude:        %f\n",store->lon);
			fprintf(output,"  Latitude:         %f\n",store->lat);
			fprintf(output,"  Course:           %f\n",
				store->course_true);
			fprintf(output,"  Course On Ground: %f\n",
				store->course_ground);
			fprintf(output,"  Speed:            %f\n",
				store->speed);
			fprintf(output,"  Speed On Ground:  %f\n",
				store->speed_ground);
			fprintf(output,"  Transverse Speed: %f\n",
				store->speed_transverse);
			fprintf(output,"  Speed Reference:  %c%c\n",
				store->speed_reference[0],
				store->speed_reference[1]);
			fprintf(output,"  Roll:             %f\n",
				store->roll);
			fprintf(output,"  Pitch:            %f\n",
				store->pitch);
			fprintf(output,"  Heave:            %f\n",
				store->heave);
			fprintf(output,"  Track:            %d\n",
				store->track);
			fprintf(output,"  Center Depth:     %f\n",
				store->depth_center);
			fprintf(output,"  Depth Scale:      %f\n",
				store->depth_scale);
			fprintf(output,"  Spare:            %d\n",
				store->spare);
			fprintf(output,"  Crosstrack Distances and Depths:\n");
			for (i=0;i<MBSYS_HSDS_BEAMS;i++)
				fprintf(output,"                    %5d %5d\n",
					store->distance[i],store->depth[i]);
			fprintf(output,"  Center Travel Time: %f\n",
				store->time_center);
			fprintf(output,"  Time Scale:       %f\n",
				store->time_scale);
			fprintf(output,"  Travel Times:\n");
			for (i=0;i<MBSYS_HSDS_BEAMS;i++)
				fprintf(output,"            %5d\n",
					store->time[i]);
			fprintf(output,"  Gyro Headings:\n");
			for (i=0;i<11;i++)
				fprintf(output,"            %f\n",
					store->gyro[i]);
			fprintf(output,"  Mode:             %c%c\n",
				store->mode[0],store->mode[1]);
			fprintf(output,"  Transmit Starboard: %d\n",
				store->trans_strbd);
			fprintf(output,"  Transmit Vertical:  %d\n",
				store->trans_vert);
			fprintf(output,"  Transmit Port:      %d\n",
				store->trans_port);
			fprintf(output,"  Pulse Starboard:    %d\n",
				store->pulse_len_strbd);
			fprintf(output,"  Pulse Vertical:     %d\n",
				store->pulse_len_vert);
			fprintf(output,"  Pulse Port:         %d\n",
				store->pulse_len_port);
			fprintf(output,"  Gain Start:         %d\n",
				store->gain_start);
			fprintf(output,"  Compensation Factor:%d\n",
				store->r_compensation_factor);
			fprintf(output,"  Compensation Start: %d\n",
				store->compensation_start);
			fprintf(output,"  Increase Start:     %d\n",
				store->increase_start);
			fprintf(output,"  Near TVC:           %d\n",
				store->tvc_near);
			fprintf(output,"  Far TVC:            %d\n",
				store->tvc_far);
			fprintf(output,"  Near Increase:      %d\n",
				store->increase_int_near);
			fprintf(output,"  Far Increase:       %d\n",
				store->increase_int_far);
			fprintf(output,"  Center Gain:        %d\n",
				store->gain_center);
			fprintf(output,"  Filter Gain:        %f\n",
				store->filter_gain);
			fprintf(output,"  Center Amplitude:   %d\n",
				store->amplitude_center);
			fprintf(output,"  Center Echo Time:   %d\n",
				store->echo_duration_center);
			fprintf(output,"  Echo Scale:         %d\n",
				store->echo_scale_center);

			fprintf(output,"  Amplitudes and Durations:\n");
			for (i=0;i<MBSYS_HSDS_BEAMS;i++)
				fprintf(output,"            %5d %5d\n",
					store->amplitude[i],
					store->echo_duration[i]);
			fprintf(output,"  Echo Gains and Scales:\n");
			for (i=0;i<16;i++)
				fprintf(output,"            %5d %5d\n",
					store->gain[i],
					store->echo_scale[i]);
			}

		/* deal with comment record */
		if (kind == MB_DATA_COMMENT && mb_data_comment_list == MB_YES)
			{
			mb_data_comment_count++;
			fprintf(output,"\n");
			fprintf(output,"Comment Record (LDEOCMNT):\n");
			fprintf(output,"  %s\n",store->comment);
			}

		/* deal with calibrate data record */
		if (kind == MB_DATA_CALIBRATE 
			&& mb_data_calibrate_list == MB_YES)
			{
			mb_data_calibrate_count++;
			fprintf(output,"\n");
			fprintf(output,"Calibrate Data Record (ERGNEICH + ERGNSLZT +ERGNAMPL):\n");
			fprintf(output,"  Time:            %2d/%2d/%4d %2.2d:%2.2d:%2.2d\n",
				store->month,store->day,store->year,
				store->hour,store->minute,store->second);
			fprintf(output,"  Alternate Time:   %4d  %4d\n",
				store->alt_minute,store->alt_second);
			fprintf(output,"  Longitude:        %f\n",store->lon);
			fprintf(output,"  Latitude:         %f\n",store->lat);
			fprintf(output,"  Course:           %f\n",
				store->course_true);
			fprintf(output,"  Course On Ground: %f\n",
				store->course_ground);
			fprintf(output,"  Speed:            %f\n",
				store->speed);
			fprintf(output,"  Speed On Ground:  %f\n",
				store->speed_ground);
			fprintf(output,"  Transverse Speed: %f\n",
				store->speed_transverse);
			fprintf(output,"  Speed Reference:  %c%c\n",
				store->speed_reference[0],
				store->speed_reference[1]);
			fprintf(output,"  Roll:             %f\n",
				store->roll);
			fprintf(output,"  Pitch:            %f\n",
				store->pitch);
			fprintf(output,"  Heave:            %f\n",
				store->heave);
			fprintf(output,"  Track:            %d\n",
				store->track);
			fprintf(output,"  Center Depth:     %f\n",
				store->depth_center);
			fprintf(output,"  Depth Scale:      %f\n",
				store->depth_scale);
			fprintf(output,"  Spare:            %d\n",
				store->spare);
			fprintf(output,"  Crosstrack Distances and Depths:\n");
			for (i=0;i<MBSYS_HSDS_BEAMS;i++)
				fprintf(output,"                    %5d %5d\n",
					store->distance[i],store->depth[i]);
			fprintf(output,"  Center Travel Time: %f\n",
				store->time_center);
			fprintf(output,"  Time Scale:       %f\n",
				store->time_scale);
			fprintf(output,"  Travel Times:\n");
			for (i=0;i<MBSYS_HSDS_BEAMS;i++)
				fprintf(output,"            %5d\n",
					store->time[i]);
			fprintf(output,"  Gyro Headings:\n");
			for (i=0;i<11;i++)
				fprintf(output,"            %f\n",
					store->gyro[i]);
			fprintf(output,"  Mode:             %c%c\n",
				store->mode[0],store->mode[1]);
			fprintf(output,"  Transmit Starboard: %d\n",
				store->trans_strbd);
			fprintf(output,"  Transmit Vertical:  %d\n",
				store->trans_vert);
			fprintf(output,"  Transmit Port:      %d\n",
				store->trans_port);
			fprintf(output,"  Pulse Starboard:    %d\n",
				store->pulse_len_strbd);
			fprintf(output,"  Pulse Vertical:     %d\n",
				store->pulse_len_vert);
			fprintf(output,"  Pulse Port:         %d\n",
				store->pulse_len_port);
			fprintf(output,"  Gain Start:         %d\n",
				store->gain_start);
			fprintf(output,"  Compensation Factor:%d\n",
				store->r_compensation_factor);
			fprintf(output,"  Compensation Start: %d\n",
				store->compensation_start);
			fprintf(output,"  Increase Start:     %d\n",
				store->increase_start);
			fprintf(output,"  Near TVC:           %d\n",
				store->tvc_near);
			fprintf(output,"  Far TVC:            %d\n",
				store->tvc_far);
			fprintf(output,"  Near Increase:      %d\n",
				store->increase_int_near);
			fprintf(output,"  Far Increase:       %d\n",
				store->increase_int_far);
			fprintf(output,"  Center Gain:        %d\n",
				store->gain_center);
			fprintf(output,"  Filter Gain:        %f\n",
				store->filter_gain);
			fprintf(output,"  Center Amplitude:   %d\n",
				store->amplitude_center);
			fprintf(output,"  Center Echo Time:   %d\n",
				store->echo_duration_center);
			fprintf(output,"  Echo Scale:         %d\n",
				store->echo_scale_center);

			fprintf(output,"  Amplitudes and Durations:\n");
			for (i=0;i<MBSYS_HSDS_BEAMS;i++)
				fprintf(output,"            %5d %5d\n",
					store->amplitude[i],
					store->echo_duration[i]);
			fprintf(output,"  Echo Gains and Scales:\n");
			for (i=0;i<16;i++)
				fprintf(output,"            %5d %5d\n",
					store->gain[i],
					store->echo_scale[i]);
			}

		/* deal with mean velocity data record */
		if (kind == MB_DATA_MEAN_VELOCITY 
			&& mb_data_mean_velocity_list == MB_YES)
			{
			mb_data_mean_velocity_count++;
			fprintf(output,"\n");
			fprintf(output,"Mean Water Velocity Record (ERGNHYDI):\n");
			fprintf(output,"  Time:            %2d/%2d/%4d %2.2d:%2.2d:%2.2d\n",
				store->month,store->day,store->year,
				store->hour,store->minute,store->second);
			fprintf(output,"  Alternate Time:   %4d  %4d\n",
				store->alt_minute,store->alt_second);
			fprintf(output,"  Longitude:        %f\n",store->lon);
			fprintf(output,"  Latitude:         %f\n",store->lat);
			fprintf(output,"  Draught:          %f\n",
				store->draught);
			fprintf(output,"  Mean velocity:    %f\n",
				store->vel_mean);
			fprintf(output,"  Keel velocity:    %f\n",
				store->vel_keel);
			fprintf(output,"  Tide:             %f\n",store->tide);
			}

		/* deal with velocity profile data record */
		if (kind == MB_DATA_VELOCITY_PROFILE
			&& mb_data_velocity_profile_list == MB_YES)
			{
			mb_data_velocity_profile_count++;
			fprintf(output,"\n");
			fprintf(output,"Water Velocity Profile Record (ERGNCTDS):\n");
			fprintf(output,"  Time:             %2d/%2d/%4d %2.2d:%2.2d:%2.2d\n",
				store->month,store->day,store->year,
				store->hour,store->minute,store->second);
			fprintf(output,"  Longitude:        %f\n",store->lon);
			fprintf(output,"  Latitude:         %f\n",store->lat);
			fprintf(output,"  Number of points: %d\n",
				store->num_vel);
			fprintf(output,"  Water Velocity Profile:\n");
			for (i=0;i<store->num_vel;i++)
				fprintf(output,"    %f %f\n",
					store->vdepth[i],store->velocity[i]);
			}

		/* deal with standby data record */
		if (kind == MB_DATA_STANDBY
			&& mb_data_standby_list == MB_YES)
			{
			mb_data_standby_count++;
			fprintf(output,"\n");
			fprintf(output,"Standby Data Record (ERGNPARA):\n");
			fprintf(output,"  Time:            %2d/%2d/%4d %2.2d:%2.2d:%2.2d\n",
				store->month,store->day,store->year,
				store->hour,store->minute,store->second);
			fprintf(output,"  Alternate Time:   %4d  %4d\n",
				store->alt_minute,store->alt_second);
			fprintf(output,"  Longitude:        %f\n",store->lon);
			fprintf(output,"  Latitude:         %f\n",store->lat);
			}

		/* deal with navigation source data record */
		if (kind == MB_DATA_NAV_SOURCE
			&& mb_data_nav_source_list == MB_YES)
			{
			mb_data_nav_source_count++;
			fprintf(output,"\n");
			fprintf(output,"Standby Data Record (ERGNPARA):\n");
			fprintf(output,"  Time:            %2d/%2d/%4d %2.2d:%2.2d:%2.2d\n",
				store->month,store->day,store->year,
				store->hour,store->minute,store->second);
			fprintf(output,"  Alternate Time:   %4d  %4d\n",
				store->alt_minute,store->alt_second);
			fprintf(output,"  Longitude:        %f\n",store->lon);
			fprintf(output,"  Latitude:         %f\n",store->lat);
			fprintf(output,"  X Correction:     %f\n",
				store->pos_corr_x);
			fprintf(output,"  Y Correction:     %f\n",
				store->pos_corr_y);
			fprintf(output,"  Sensors:          ");
			for (i=0;i<10;i++)
				fprintf(output,"%c",store->sensors[i]);
			fprintf(output,"\n");
			}

		}

	/* close the file */
	status = mb_close(verbose,&mbio_ptr,&error);

	/* deallocate memory for data arrays */
	mb_freed(verbose,__FILE__,__LINE__,(void **)&beamflag,&error); 
	mb_freed(verbose,__FILE__,__LINE__,(void **)&bath,&error); 
	mb_freed(verbose,__FILE__,__LINE__,(void **)&bathacrosstrack,&error); 
	mb_freed(verbose,__FILE__,__LINE__,(void **)&bathalongtrack,&error); 
	mb_freed(verbose,__FILE__,__LINE__,(void **)&amp,&error); 
	mb_freed(verbose,__FILE__,__LINE__,(void **)&ss,&error); 
	mb_freed(verbose,__FILE__,__LINE__,(void **)&ssacrosstrack,&error); 
	mb_freed(verbose,__FILE__,__LINE__,(void **)&ssalongtrack,&error); 

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* give the statistics */
	fprintf(output,"\n");
	if (mb_data_data_list == MB_YES)
		fprintf(output,"%d survey data records listed\n",
			mb_data_data_count);
	if (mb_data_comment_list == MB_YES)
		fprintf(output,"%d comment records listed\n",
			mb_data_comment_count);
	if (mb_data_calibrate_list == MB_YES)
		fprintf(output,"%d calibrate data records listed\n",
			mb_data_calibrate_count);
	if (mb_data_mean_velocity_list == MB_YES)
		fprintf(output,"%d mean velocity data records listed\n",
			mb_data_mean_velocity_count);
	if (mb_data_velocity_profile_list == MB_YES)
		fprintf(output,"%d velocity profile data records listed\n",
			mb_data_velocity_profile_count);
	if (mb_data_standby_list == MB_YES)
		fprintf(output,"%d standby data records listed\n",
			mb_data_standby_count);
	if (mb_data_nav_source_list == MB_YES)
		fprintf(output,"%d navigation source data records listed\n",
			mb_data_nav_source_count);

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/
