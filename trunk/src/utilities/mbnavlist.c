/*--------------------------------------------------------------------
 *    The MB-system:	mbnavlist.c	2/1/93
 *    $Id: mbnavlist.c,v 5.12 2007/10/17 20:34:00 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000, 2003 by
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
 * mbnavlist prints the specified contents of navigation records
 * in a swath sonar data file to stdout. The form of the 
 * output is quite flexible; mbnavlist is tailored to produce 
 * ascii files in spreadsheet style with data columns separated by tabs.
 *
 * Author:	D. W. Caress
 * Date:	November 11, 1999
 *
 *
 * $Log: mbnavlist.c,v $
 * Revision 5.12  2007/10/17 20:34:00  caress
 * Release 5.1.1beta11
 * Added decimation option.
 *
 * Revision 5.11  2006/01/18 15:17:00  caress
 * Added stdlib.h include.
 *
 * Revision 5.10  2005/11/05 01:07:54  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.9  2004/12/02 06:37:42  caress
 * Fixes while supporting Reson 7k data.
 *
 * Revision 5.8  2003/07/30 16:41:06  caress
 * Fixed handling of time gap errors in data .
 *
 * Revision 5.7  2003/07/02 18:14:19  caress
 * Release 5.0.0
 *
 * Revision 5.6  2003/04/17 21:18:57  caress
 * Release 5.0.beta30
 *
 * Revision 5.5  2002/05/29 23:43:09  caress
 * Release 5.0.beta18
 *
 * Revision 5.4  2001/07/24 17:40:54  caress
 * Fixed typo.
 *
 * Revision 5.3  2001/07/20 00:34:38  caress
 * Release 5.0.beta03
 *
 * Revision 5.2  2001/03/22  21:15:49  caress
 * Trying to make release 5.0.beta0.
 *
 * Revision 5.1  2000/12/10  20:30:44  caress
 * Version 5.0.alpha02
 *
 * Revision 5.0  2000/12/01  22:57:08  caress
 * First cut at Version 5.0.
 *
 * Revision 4.4  2000/10/11  01:06:15  caress
 * Convert to ANSI C
 *
 * Revision 4.3  2000/09/30  07:06:28  caress
 * Snapshot for Dale.
 *
 * Revision 4.2  2000/09/11  20:10:02  caress
 * Linked to new datalist parsing functions. Now supports recursive datalists
 * and comments in datalists.
 *
 * Revision 4.1  2000/03/08  00:03:45  caress
 * Release 4.6.10
 *
 * Revision 4.0  1999/12/29  00:58:18  caress
 * Release 4.6.8
 *
 * Revision 1.1  1999/12/29  00:35:11  caress
 * Initial revision
 *
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"

/* local options */
#define	MAX_OPTIONS	25

/* function prototypes */
int printsimplevalue(int verbose, 
	double value, int width, int precision, 
	int ascii, int *invert, int *flipsign, int *error);
int printNaN(int verbose, int ascii, int *invert, int *flipsign, int *error);

/* NaN value */
double	NaN;

/*--------------------------------------------------------------------*/

main (int argc, char **argv)
{
	static char rcs_id[] = "$Id: mbnavlist.c,v 5.12 2007/10/17 20:34:00 caress Exp $";
	static char program_name[] = "mbnavlist";
	static char help_message[] =  "mbnavlist prints the specified contents of navigation records\nin a swath sonar data file to stdout. The form of the \noutput is quite flexible; mbnavlist is tailored to produce \nascii files in spreadsheet style with data columns separated by tabs.";
	static char usage_message[] = "mbnavlist [-Byr/mo/da/hr/mn/sc -Ddecimate -Eyr/mo/da/hr/mn/sc \n-Fformat -Gdelimiter -H -Ifile -Llonflip \n-Ooptions -Rw/e/s/n -Sspeed \n-Ttimegap -V -Zsegment]";
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
	char	*message;

	/* MBIO read control parameters */
	int	read_datalist = MB_NO;
	char	read_file[MB_PATH_MAXLINE];
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_UNSET;
	double	file_weight;
	int	format;
	int	pings;
	int	decimate;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	char	file[MB_PATH_MAXLINE];
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	
	/* data record source types */
	int	nav_source;
	int	heading_source;
	int	vru_source;
	int	svp_source;
	int	aux_nav_channel = -1;

	/* output format list controls */
	char	list[MAX_OPTIONS];
	int	n_list;
	double	distance_total;
	int	nread;
	int	time_j[5];
	int	invert_next_value = MB_NO;
	int	signflip_next_value = MB_NO;
	int	first = MB_YES;
	int	ascii = MB_YES;
	int	segment = MB_NO;
	char	segment_tag[MB_PATH_MAXLINE];
	char	delimiter[MB_PATH_MAXLINE];

	/* MBIO read values */
	void	*mbio_ptr = NULL;
	void	*store_ptr;
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
	int	icomment = 0;
	int	atime_i[7 * MB_ASYNCH_SAVE_MAX];
	double	atime_d[MB_ASYNCH_SAVE_MAX];
	double	anavlon[MB_ASYNCH_SAVE_MAX];
	double	anavlat[MB_ASYNCH_SAVE_MAX];
	double	aspeed[MB_ASYNCH_SAVE_MAX];
	double	aheading[MB_ASYNCH_SAVE_MAX];
	double	adraft[MB_ASYNCH_SAVE_MAX];
	double	aroll[MB_ASYNCH_SAVE_MAX];
	double	apitch[MB_ASYNCH_SAVE_MAX];
	double	aheave[MB_ASYNCH_SAVE_MAX];

	/* additional time variables */
	int	first_m = MB_YES;
	double	time_d_ref;
	struct tm	time_tm;
	int	first_u = MB_YES;
	time_t	time_u;
	time_t	time_u_ref;

	/* course calculation variables */
	double	dlon, dlat, minutes;
	int	degrees;
	char	hemi;
	double	headingx, headingy, mtodeglon, mtodeglat;
	double	course, course_old;
	double	time_d_old;
	double	time_interval;
	double	speed_made_good, speed_made_good_old;
	double	navlon_old, navlat_old;
	double	dx, dy;
	double	b;

	int	read_data;
	int	inav, n;
	int	nnav;
	int	i, j, k;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input to datalist.mb-1 */
	strcpy (read_file, "datalist.mb-1");

	/* set up the default list controls 
		(lon, lat, along-track distance, center beam depth) */
	list[0]='t';
	list[1]='M';
	list[2]='X';
	list[3]='Y';
	list[4]='H';
	list[5]='s';
	n_list = 6;
	sprintf(delimiter, "\t");
	decimate = 1;

	/* process argument list */
	while ((c = getopt(argc, argv, "AaB:b:D:d:E:e:F:f:G:g:I:i:L:l:N:n:O:o:R:r:S:s:T:t:Z:z:VvHh")) != -1)
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
		case 'A':
		case 'a':
			ascii = MB_NO;
			flag++;
			break;
		case 'B':
		case 'b':
			sscanf (optarg,"%d/%d/%d/%d/%d/%d",
				&btime_i[0],&btime_i[1],&btime_i[2],
				&btime_i[3],&btime_i[4],&btime_i[5]);
			btime_i[6] = 0;
			flag++;
			break;
		case 'D':
		case 'd':
			sscanf (optarg,"%d", &decimate);
			flag++;
			break;
		case 'E':
		case 'e':
			sscanf (optarg,"%d/%d/%d/%d/%d/%d",
				&etime_i[0],&etime_i[1],&etime_i[2],
				&etime_i[3],&etime_i[4],&etime_i[5]);
			etime_i[6] = 0;
			flag++;
			break;
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &format);
			flag++;
			break;
		case 'G':
		case 'g':
			sscanf (optarg,"%s", delimiter);
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", read_file);
			flag++;
			break;
		case 'L':
		case 'l':
			sscanf (optarg,"%d", &lonflip);
			flag++;
			break;
		case 'N':
		case 'n':
			sscanf (optarg,"%d", &aux_nav_channel);
			flag++;
			break;
		case 'O':
		case 'o':
			for(j=0,n_list=0;j<(int)strlen(optarg);j++,n_list++)
				if (n_list<MAX_OPTIONS)
					list[n_list] = optarg[j];
			flag++;
			break;
		case 'R':
		case 'r':
			mb_get_bounds(optarg, bounds);
			flag++;
			break;
		case 'S':
		case 's':
			sscanf (optarg,"%lf", &speedmin);
			flag++;
			break;
		case 'T':
		case 't':
			sscanf (optarg,"%lf", &timegap);
			flag++;
			break;
		case 'Z':
		case 'z':
			segment = MB_YES;
			sscanf (optarg,"%s", segment_tag);
			flag++;
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
		fprintf(stderr,"Version %s\n",rcs_id);
		fprintf(stderr,"MB-system Version %s\n",MB_VERSION);
		}

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s>\n",program_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Control Parameters:\n");
		fprintf(stderr,"dbg2       verbose:        %d\n",verbose);
		fprintf(stderr,"dbg2       help:           %d\n",help);
		fprintf(stderr,"dbg2       format:         %d\n",format);
		fprintf(stderr,"dbg2       pings:          %d\n",pings);
		fprintf(stderr,"dbg2       lonflip:        %d\n",lonflip);
		fprintf(stderr,"dbg2       decimate:       %d\n",decimate);
		fprintf(stderr,"dbg2       bounds[0]:      %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:      %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:      %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:      %f\n",bounds[3]);
		fprintf(stderr,"dbg2       btime_i[0]:     %d\n",btime_i[0]);
		fprintf(stderr,"dbg2       btime_i[1]:     %d\n",btime_i[1]);
		fprintf(stderr,"dbg2       btime_i[2]:     %d\n",btime_i[2]);
		fprintf(stderr,"dbg2       btime_i[3]:     %d\n",btime_i[3]);
		fprintf(stderr,"dbg2       btime_i[4]:     %d\n",btime_i[4]);
		fprintf(stderr,"dbg2       btime_i[5]:     %d\n",btime_i[5]);
		fprintf(stderr,"dbg2       btime_i[6]:     %d\n",btime_i[6]);
		fprintf(stderr,"dbg2       etime_i[0]:     %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]:     %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]:     %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]:     %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]:     %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]:     %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       etime_i[6]:     %d\n",etime_i[6]);
		fprintf(stderr,"dbg2       speedmin:       %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:        %f\n",timegap);
		fprintf(stderr,"dbg2       ascii:          %d\n",ascii);
		fprintf(stderr,"dbg2       segment:        %d\n",segment);
		fprintf(stderr,"dbg2       segment_tag:    %s\n",segment_tag);
		fprintf(stderr,"dbg2       delimiter:      %s\n",delimiter);
		fprintf(stderr,"dbg2       file:           %s\n",file);
		fprintf(stderr,"dbg2       n_list:         %d\n",n_list);
		for (i=0;i<n_list;i++)
			fprintf(stderr,"dbg2         list[%d]:      %c\n",
						i,list[i]);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

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
	    if (status = mb_datalist_read(verbose,datalist,
			    file,&format,&file_weight,&error)
			    == MB_SUCCESS)
		read_data = MB_YES;
	    else
		read_data = MB_NO;
	    }
	/* else copy single filename to be read */
	else
	    {
	    strcpy(file, read_file);
	    read_data = MB_YES;
	    }

	/* loop over all files to be read */
	while (read_data == MB_YES)
	{		
	/* check format and get data sources */
	if ((status = mb_format_source(verbose, &format, 
			&nav_source, &heading_source, 
			&vru_source, &svp_source, 
			&error)) == MB_FAILURE)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_format_source>:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
		
	/* set auxilliary nav source if requested */
	if (aux_nav_channel > 0)
		{
		if (aux_nav_channel == 1)
		    nav_source = MB_DATA_NAV1;
		else if (aux_nav_channel == 2)
		    nav_source = MB_DATA_NAV2;
		else if (aux_nav_channel == 3)
		    nav_source = MB_DATA_NAV3;
		}

	/* initialize reading the swath file */
	if ((status = mb_read_init(
		verbose,file,format,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&mbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",file);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* allocate memory for data arrays */
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(char), (void **)&beamflag, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bath, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE,
						sizeof(double), (void **)&amp, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bathacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bathalongtrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&ss, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&ssacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, 
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
		
	/* output separator for GMT style segment file output */
	if (segment == MB_YES && ascii == MB_YES)
		{
		printf("%s\n", segment_tag);
		}

	/* read and print data */
	distance_total = 0.0;
	nread = 0;
	nnav = 0;
	first = MB_YES;
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* read a ping of data */
		status = mb_get_all(verbose,mbio_ptr,&store_ptr,&kind,
			time_i,&time_d,&navlon,&navlat,
			&speed,&heading,
			&distance,&altitude,&sonardepth,
			&beams_bath,&beams_amp,&pixels_ss,
			beamflag,bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);

		/* time gaps are not a problem here */
		if (error == MB_ERROR_TIME_GAP)
			{
			error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
			
		/* check for appropriate navigation record */
		if (error <= MB_ERROR_NO_ERROR
			&& kind != nav_source)
			{
			error = MB_ERROR_IGNORE;
			status = MB_FAILURE;
			}
		else if (error <= MB_ERROR_NO_ERROR
			&& kind == nav_source)
			{
			error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}

		/* extract additional nav info */
		if (error == MB_ERROR_NO_ERROR)
		   	status = mb_extract_nnav(verbose,mbio_ptr,store_ptr,
					MB_ASYNCH_SAVE_MAX, &kind, &n,
				    	atime_i,atime_d,anavlon,anavlat,
				    	aspeed,aheading,adraft,
					aroll,apitch,aheave,&error);

		/* increment counter */
		if (error == MB_ERROR_NO_ERROR)
			nread++;

		/* print debug statements */
		if (verbose >= 2)
			{
			fprintf(stderr,"\ndbg2  Nsv data read in program <%s>\n",
				program_name);
			fprintf(stderr,"dbg2       kind:           %d\n",kind);
			fprintf(stderr,"dbg2       error:          %d\n",error);
			fprintf(stderr,"dbg2       status:         %d\n",status);
			fprintf(stderr,"dbg2       n:              %d\n",n);
			}
			
		/* loop over the n navigation points, outputting each one */
		/* calculate course made good and distance */
		if (error == MB_ERROR_NO_ERROR && n > 0)
			{
			for (inav=0;inav<n;inav++)
				{
				/* get data */
				for (j=0;j<7;j++)
					time_i[j] = atime_i[inav * 7 + j];
				time_d = atime_d[inav];
				navlon = anavlon[inav];
				navlat = anavlat[inav];
				speed = aspeed[inav];
				heading = aheading[inav];
				draft = adraft[inav];
				roll = aroll[inav];
				pitch = apitch[inav];
				heave = aheave[inav];
				
/*fprintf(stdout, "kind:%d error:%d %d of %d: time:%4d/%2d/%2d %2.2d:%2.2d:%2.2d.%6.6d\n", 
kind, error, i, n, 
time_i[0],  time_i[1],  time_i[2],  
time_i[3],  time_i[4],  time_i[5],   time_i[6]);*/
		
				/* calculate course made good and distance */
				mb_coor_scale(verbose,navlat, &mtodeglon, &mtodeglat);
				headingx = sin(DTR * heading);
				headingy = cos(DTR * heading);
				if (first == MB_YES)
					{
					time_interval = 0.0;
					course = heading;
					speed_made_good = 0.0;
					course_old = heading;
					speed_made_good_old = speed;
					distance = 0.0;
					}
				else
					{
					time_interval = time_d - time_d_old;
					dx = (navlon - navlon_old)/mtodeglon;
					dy = (navlat - navlat_old)/mtodeglat;
					distance = sqrt(dx*dx + dy*dy);
					if (distance > 0.0)
						course = RTD*atan2(dx/distance,dy/distance);
					else
						course = course_old;
					if (course < 0.0)
						course = course + 360.0;
					if (time_interval > 0.0)
						speed_made_good = 3.6*distance/time_interval;
					else
						speed_made_good 
							= speed_made_good_old;
					}
				distance_total += 0.001 * distance;

				/* reset old values */
				navlon_old = navlon;
				navlat_old = navlat;
				course_old = course;
				speed_made_good_old = speed_made_good;
				time_d_old = time_d;

				/* now loop over list of output parameters */
				if (nnav % decimate == 0)
				for (i=0; i<n_list; i++) 
					{
					switch (list[i]) 
						{
						case '/': /* Inverts next simple value */
							invert_next_value = MB_YES;
							break;
						case '-': /* Flip sign on next simple value */
							signflip_next_value = MB_YES;
							break;
						case 'c': /* Sonar transducer depth (m) */
							printsimplevalue(verbose, sonardepth, 0, 3, ascii, 
									    &invert_next_value, 
									    &signflip_next_value, &error);
							break;
						case 'H': /* heading */
							printsimplevalue(verbose, heading, 6, 2, ascii, 
									    &invert_next_value, 
									    &signflip_next_value, &error);
							break;
						case 'h': /* course */
							printsimplevalue(verbose, course, 6, 2, ascii, 
									    &invert_next_value, 
									    &signflip_next_value, &error);
							break;
						case 'J': /* time string */
							mb_get_jtime(verbose,time_i,time_j);
							if (ascii == MB_YES)
							    {
							    printf("%.4d %.3d %.2d %.2d %.2d.%6.6d",
								time_j[0],time_j[1],
								time_i[3],time_i[4],
								time_i[5],time_i[6]);
							    }
							else
							    {
							    b = time_j[0];
							    fwrite(&b, sizeof(double), 1, stdout);
							    b = time_j[1];
							    fwrite(&b, sizeof(double), 1, stdout);
							    b = time_i[3];
							    fwrite(&b, sizeof(double), 1, stdout);
							    b = time_i[4];
							    fwrite(&b, sizeof(double), 1, stdout);
							    b = time_i[5];
							    fwrite(&b, sizeof(double), 1, stdout);
							    b = time_i[6];
							    fwrite(&b, sizeof(double), 1, stdout);
							    }
							break;
						case 'j': /* time string */
							mb_get_jtime(verbose,time_i,time_j);
							if (ascii == MB_YES)
							    {
							    printf("%.4d %.3d %.4d %.2d.%6.6d",
								time_j[0],time_j[1],
								time_j[2],time_j[3],time_j[4]);
							    }
							else
							    {
							    b = time_j[0];
							    fwrite(&b, sizeof(double), 1, stdout);
							    b = time_j[1];
							    fwrite(&b, sizeof(double), 1, stdout);
							    b = time_j[2];
							    fwrite(&b, sizeof(double), 1, stdout);
							    b = time_j[3];
							    fwrite(&b, sizeof(double), 1, stdout);
							    b = time_j[4];
							    fwrite(&b, sizeof(double), 1, stdout);
							    }
							break;
						case 'L': /* along-track distance (km) */
							printsimplevalue(verbose, distance_total, 7, 3, ascii, 
									    &invert_next_value, 
									    &signflip_next_value, &error);
							break;
						case 'l': /* along-track distance (m) */
							printsimplevalue(verbose, 1000.0 * distance_total, 7, 3, ascii, 
									    &invert_next_value, 
									    &signflip_next_value, &error);
							break;
						case 'M': /* Decimal unix seconds since 
								1/1/70 00:00:00 */
							printsimplevalue(verbose, time_d, 0, 6, ascii, 
									    &invert_next_value, 
									    &signflip_next_value, &error);
							break;
						case 'm': /* time in decimal seconds since 
								first record */
							if (first_m == MB_YES)
								{
								time_d_ref = time_d;
								first_m = MB_NO;
								}
							b = time_d - time_d_ref;
							printsimplevalue(verbose, b, 0, 6, ascii, 
									    &invert_next_value, 
									    &signflip_next_value, &error);
							break;
						case 'P': /* pitch */
							printsimplevalue(verbose, pitch, 5, 2, ascii, 
									    &invert_next_value, 
									    &signflip_next_value, &error);
							break;
						case 'p': /* draft */
							printsimplevalue(verbose, draft, 5, 2, ascii, 
									    &invert_next_value, 
									    &signflip_next_value, &error);
							break;
						case 'R': /* roll */
							printsimplevalue(verbose, roll, 5, 2, ascii, 
									    &invert_next_value, 
									    &signflip_next_value, &error);
							break;
						case 'r': /* heave */
							printsimplevalue(verbose, heave, 5, 2, ascii, 
									    &invert_next_value, 
									    &signflip_next_value, &error);
							break;
						case 'S': /* speed */
							printsimplevalue(verbose, speed, 5, 2, ascii, 
									    &invert_next_value, 
									    &signflip_next_value, &error);
							break;
						case 's': /* speed made good */
							printsimplevalue(verbose, speed_made_good, 5, 2, ascii, 
									    &invert_next_value, 
									    &signflip_next_value, &error);
							break;
						case 'T': /* yyyy/mm/dd/hh/mm/ss time string */
							if (ascii == MB_YES)
							    printf("%.4d/%.2d/%.2d/%.2d/%.2d/%.2d.%.6d",
								time_i[0],time_i[1],time_i[2],
								time_i[3],time_i[4],time_i[5],
								time_i[6]);
							else
							    {
							    b = time_i[0];
							    fwrite(&b, sizeof(double), 1, stdout);
							    b = time_i[1];
							    fwrite(&b, sizeof(double), 1, stdout);
							    b = time_i[2];
							    fwrite(&b, sizeof(double), 1, stdout);
							    b = time_i[3];
							    fwrite(&b, sizeof(double), 1, stdout);
							    b = time_i[4];
							    fwrite(&b, sizeof(double), 1, stdout);
							    b = time_i[5] + 1e-6 * time_i[6];
							    fwrite(&b, sizeof(double), 1, stdout);
							    }
							break;
						case 't': /* yyyy mm dd hh mm ss time string */
							if (ascii == MB_YES)
							    printf("%.4d %.2d %.2d %.2d %.2d %.2d.%.6d",
								time_i[0],time_i[1],time_i[2],
								time_i[3],time_i[4],time_i[5],
								time_i[6]);
							else
							    {
							    b = time_i[0];
							    fwrite(&b, sizeof(double), 1, stdout);
							    b = time_i[1];
							    fwrite(&b, sizeof(double), 1, stdout);
							    b = time_i[2];
							    fwrite(&b, sizeof(double), 1, stdout);
							    b = time_i[3];
							    fwrite(&b, sizeof(double), 1, stdout);
							    b = time_i[4];
							    fwrite(&b, sizeof(double), 1, stdout);
							    b = time_i[5] + 1e-6 * time_i[6];
							    fwrite(&b, sizeof(double), 1, stdout);
							    }
							break;
						case 'U': /* unix time in seconds since 1/1/70 00:00:00 */
							time_u = (int) time_d;
							if (ascii == MB_YES)
							    printf("%d",time_u);
							else
							    {
							    b = time_u;
							    fwrite(&b, sizeof(double), 1, stdout);
							    }
							break;
						case 'u': /* time in seconds since first record */
							time_u = (int) time_d;
							if (first_u == MB_YES)
								{
								time_u_ref = time_u;
								first_u = MB_NO;
								}
							if (ascii == MB_YES)
							    printf("%d",time_u - time_u_ref);
							else
							    {
							    b = time_u - time_u_ref;
							    fwrite(&b, sizeof(double), 1, stdout);
							    }
							break;
						case 'V': /* time in seconds since last ping */
						case 'v': 
							if (ascii == MB_YES)
							    {
							    if ( fabs(time_interval) > 100. )
								printf("%g",time_interval); 
							    else
								printf("%7.3f",time_interval);
							    }
							else
							    {
							    fwrite(&time_interval, sizeof(double), 1, stdout);
							    }
							break;
						case 'X': /* longitude decimal degrees */
							dlon = navlon;
							printsimplevalue(verbose, dlon, 11, 6, ascii, 
									    &invert_next_value, 
									    &signflip_next_value, &error);
							break;
						case 'x': /* longitude degress + decimal minutes */
							dlon = navlon;
							if (dlon < 0.0)
								{
								hemi = 'W';
								dlon = -dlon;
								}
							else
								hemi = 'E';
							degrees = (int) dlon;
							minutes = 60.0*(dlon - degrees);
							if (ascii == MB_YES)
							    {
							    printf("%3d %8.5f%c",
								degrees, minutes, hemi);
							    }
							else
							    {
							    b = degrees;
							    if (hemi == 'W') b = -b;
							    fwrite(&b, sizeof(double), 1, stdout);
							    b = minutes;
							    fwrite(&b, sizeof(double), 1, stdout);
							    }
							break;
						case 'Y': /* latitude decimal degrees */
							dlat = navlat;
							printsimplevalue(verbose, dlat, 11, 6, ascii, 
									    &invert_next_value, 
									    &signflip_next_value, &error);
							break;
						case 'y': /* latitude degrees + decimal minutes */
							dlat = navlat;
							if (dlat < 0.0)
								{
								hemi = 'S';
								dlat = -dlat;
								}
							else
								hemi = 'N';
							degrees = (int) dlat;
							minutes = 60.0*(dlat - degrees);
							if (ascii == MB_YES)
							    {
							    printf("%3d %8.5f%c",
								degrees, minutes, hemi);
							    }
							else
							    {
							    b = degrees;
							    if (hemi == 'S') b = -b;
							    fwrite(&b, sizeof(double), 1, stdout);
							    b = minutes;
							    fwrite(&b, sizeof(double), 1, stdout);
							    }
							break;
						default:
							if (ascii == MB_YES)
							    printf("<Invalid Option: %c>",
								list[i]);
							break;
						}
					if (ascii == MB_YES)
						{
						if (i<(n_list-1)) printf ("%s", delimiter);
						else printf ("\n");
						}
					}
				nnav++;				
				first = MB_NO;
				}
			}
		}

	/* close the swath file */
	status = mb_close(verbose,&mbio_ptr,&error);

	/* figure out whether and what to read next */
        if (read_datalist == MB_YES)
                {
		if (status = mb_datalist_read(verbose,datalist,
			    file,&format,&file_weight,&error)
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
int printsimplevalue(int verbose, 
	double value, int width, int precision, 
	int ascii, int *invert, int *flipsign, int *error)
{
	char	*function_name = "printsimplevalue";
	int	status = MB_SUCCESS;
	char	format[24];
	int	i;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBlist function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       value:           %f\n",value);
		fprintf(stderr,"dbg2       width:           %d\n",width);
		fprintf(stderr,"dbg2       precision:       %d\n",precision);
		fprintf(stderr,"dbg2       ascii:           %d\n",ascii);
		fprintf(stderr,"dbg2       invert:          %d\n",*invert);
		fprintf(stderr,"dbg2       flipsign:        %d\n",*flipsign);
		}
		
	/* make print format */
	format[0] = '%';
	if (*invert == MB_YES)
	    strcpy(format, "%g");
	else if (width > 0)
	    sprintf(&format[1], "%d.%df", width, precision);
	else
	    sprintf(&format[1], ".%df", precision);
	
	/* invert value if desired */
	if (*invert == MB_YES)
	    {
	    *invert = MB_NO;
	    if (value != 0.0)
		value = 1.0 / value;
	    }
	
	/* flip sign value if desired */
	if (*flipsign == MB_YES)
	    {
	    *flipsign = MB_NO;
	    value = -value;
	    }
	    
	/* print value */
	if (ascii == MB_YES)
	    printf(format, value);
	else
	    fwrite(&value, sizeof(double), 1, stdout);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBlist function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       invert:          %d\n",*invert);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int printNaN(int verbose, int ascii, int *invert, int *flipsign, int *error)
{
	char	*function_name = "printNaN";
	int	status = MB_SUCCESS;
	int	i;
	

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBlist function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       ascii:           %d\n",ascii);
		fprintf(stderr,"dbg2       invert:          %d\n",*invert);
		fprintf(stderr,"dbg2       flipsign:        %d\n",*flipsign);
		}
		
	/* reset invert flag */
	if (*invert == MB_YES)
	    *invert = MB_NO;
		
	/* reset flipsign flag */
	if (*flipsign == MB_YES)
	    *flipsign = MB_NO;
	    
	/* print value */
	if (ascii == MB_YES)
	    printf("NaN");
	else
	    fwrite(&NaN, sizeof(double), 1, stdout);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBlist function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       invert:          %d\n",*invert);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
