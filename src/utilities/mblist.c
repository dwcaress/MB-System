/*--------------------------------------------------------------------
 *    The MB-system:	mblist.c	2/1/93
 *    $Id: mblist.c,v 4.8 1994-12-21 20:22:30 caress Exp $
 *
 *    Copyright (c) 1993, 1994 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * MBLIST prints the specified contents of a multibeam data  
 * file to stdout. The form of the output is quite flexible; 
 * MBLIST is tailored to produce ascii files in spreadsheet  
 * style with data columns separated by tabs.
 *
 * Author:	D. W. Caress
 * Date:	February 1, 1993
 *
 * Note:	This program is based on the program mblist created
 *		by A. Malinverno (currently at Schlumberger, formerly
 *		at L-DEO) in August 1991.  It also includes elements
 *		derived from the program mbdump created by D. Caress
 *		in 1990.
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.7  1994/10/21  13:02:31  caress
 * Release V4.0
 *
 * Revision 4.6  1994/07/29  19:02:56  caress
 * Changes associated with supporting byte swapped Lynx OS and
 * using unix second time base.
 *
 * Revision 4.5  1994/06/21  22:53:08  caress
 * Changes to support PCs running Lynx OS.
 *
 * Revision 4.4  1994/06/05  22:31:22  caress
 * Major revision changing the manner in which complete
 * dumps occur.  Options added to control the range of beams
 * or pixels which are output.
 *
 * Revision 4.3  1994/06/01  21:00:55  caress
 * Added new dump modes with acrosstrack and alongtrack
 * values. Added topography dumps.
 *
 * Revision 4.2  1994/04/29  18:01:20  caress
 * Added output option "j" for time string in form: year jday daymin sec
 *
 * Revision 4.1  1994/04/12  18:50:33  caress
 * Added #ifdef IRIX statements for compatibility with
 * SGI machines.  The system routine timegm does not exist
 * on SGI's; mktime must be used instead.
 *
 * Revision 4.0  1994/03/06  00:13:22  caress
 * First cut at version 4.0
 *
 * Revision 4.0  1994/03/01  18:59:27  caress
 * First cut at new version. Any changes are associated with
 * support of three data types (beam bathymetry, beam amplitude,
 * and sidescan) instead of two (bathymetry and backscatter).
 *
 * Revision 3.6  1993/09/11  15:44:30  caress
 * Fixed bug so that mblist no longer checks for a valid depth
 * even if it is not outputting depth.
 *
 * Revision 3.5  1993/06/13  07:47:20  caress
 * Added new time output options "m" and "u" and changed
 * old options "U" to "M" and "u" to "U
 * Now time options are:
 *   T  for a time string (yyyy/mm/dd/hh/mm/ss)
 *   t  for a time string (yyyy mm dd hh mm ss)
 *   J  for a time string (yyyy jd hh mm ss)
 *   M  for mbio time in minutes since 1/1/81 00:00:00
 *   m  for time in minutes since first record
 *   U  for unix time in seconds since 1/1/70 00:00:00
 *   u  for time in seconds since first record
 *
 * Revision 3.4  1993/06/13  03:02:18  caress
 * Added new output option "A", which outputs the apparent
 * crosstrack seafloor slope in degrees from vertical. This
 * value is calculated by fitting a line to the bathymetry
 * data for each ping. Obtaining a time series of the apparent
 * seafloor slope is useful for detecting errors in the vertical
 * reference used by the multibeam sonar.
 *
 * Revision 3.3  1993/06/09  11:47:44  caress
 * Fixed problem with unix time value output. In unix time
 * months are counted 0-11 instead of 1-12.
 *
 * Revision 3.2  1993/05/17  16:37:18  caress
 * Fixed problem where program crashed if -F was used.
 *
 * Revision 3.0  1993/05/04  22:20:17  dale
 * Initial version
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <time.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"

/* DTR define */
#ifndef M_PI
#define	M_PI	3.14159265358979323846
#endif
#define DTR	(M_PI/180.)

/* local options */
#define	MAX_OPTIONS	25
#define	DUMP_MODE_LIST	1
#define	DUMP_MODE_BATH	2
#define	DUMP_MODE_TOPO	3
#define	DUMP_MODE_AMP	4
#define	DUMP_MODE_SS	5

/*--------------------------------------------------------------------*/

main (argc, argv)
int argc;
char **argv; 
{
	static char rcs_id[] = "$Id: mblist.c,v 4.8 1994-12-21 20:22:30 caress Exp $";
	static char program_name[] = "MBLIST";
	static char help_message[] =  "MBLIST prints the specified contents of a multibeam data \nfile to stdout. The form of the output is quite flexible; \nMBLIST is tailored to produce ascii files in spreadsheet \nstyle with data columns separated by tabs.";
	static char usage_message[] = "mblist [-Byr/mo/da/hr/mn/sc -Ddump_mode -Eyr/mo/da/hr/mn/sc \n-Fformat -H -Ifile -Llonflip -Mbeam_start/beam_end -Npixel_start/pixel_end \n-Ooptions -Ppings -Rw/e/s/n -Sspeed -Ttimegap -V]";
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
	int	format;
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	char	file[128];
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;

	/* output format list controls */
	char	list[MAX_OPTIONS];
	int	n_list;
	int	beam_set = MB_NO;
	int	beam_start;
	int	beam_end;
	int	pixel_set = MB_NO;
	int	pixel_start;
	int	pixel_end;
	int	dump_mode = 1;
	double	distance_total;
	int	nread;
	int	beam_status = MB_SUCCESS;
	int	pixel_status = MB_SUCCESS;
	int	time_j[5];
	int	use_bath = MB_NO;
	int	use_amp = MB_NO;
	int	use_ss = MB_NO;
	int	check_values = MB_YES;
	int	check_bath = MB_NO;
	int	check_amp = MB_NO;
	int	check_ss = MB_NO;
	int	first = MB_YES;

	/* MBIO read values */
	char	*mbio_ptr = NULL;
	int	kind;
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	double	*bath = NULL;
	double	*bathacrosstrack = NULL;
	double	*bathalongtrack = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*ssacrosstrack = NULL;
	double	*ssalongtrack = NULL;
	char	comment[256];
	int	icomment = 0;

	/* additional time variables */
	int	first_m = MB_YES;
	double	time_d_ref;
	struct tm	time_tm;
	int	first_u = MB_YES;
	time_t	time_u;
	time_t	time_u_ref;

	/* crosstrack slope values */
	double	slope;
	double	sx, sy, sxx, sxy;
	int	ns;
	double	delta, a, b, theta;
	double	dlon, dlat;
	double	headingx, headingy, mtodeglon, mtodeglat;

	int	i, j, k;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input to stdin */
	strcpy (file, "stdin");

	/* set up the default list controls 
		(lon, lat, along-track distance, center beam depth) */
	list[0]='X';
	list[1]='Y';
	list[2]='L';
	list[3]='Z';
	n_list = 4;

	/* set dump mode flag to DUMP_MODE_LIST */
	dump_mode = DUMP_MODE_LIST;

	/* process argument list */
	while ((c = getopt(argc, argv, "B:b:D:d:E:e:F:f:I:i:L:l:M:m:N:n:O:o:P:p:QqR:r:S:s:T:t:VvHh")) != -1)
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
			sscanf (optarg,"%d", &dump_mode);
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
		case 'I':
		case 'i':
			sscanf (optarg,"%s", file);
			flag++;
			break;
		case 'L':
		case 'l':
			sscanf (optarg,"%d", &lonflip);
			flag++;
			break;
		case 'M':
		case 'm':
			sscanf (optarg,"%d/%d", &beam_start,&beam_end);
			beam_set = MB_YES;
			flag++;
			break;
		case 'N':
		case 'n':
			sscanf (optarg,"%d/%d", &pixel_start,&pixel_end);
			pixel_set = MB_YES;
			flag++;
			break;
		case 'O':
		case 'o':
			for(j=0,n_list=0;j<strlen(optarg);j++,n_list++)
				if (n_list<MAX_OPTIONS)
					list[n_list] = optarg[j];
			flag++;
			break;
		case 'P':
		case 'p':
			sscanf (optarg,"%d", &pings);
			flag++;
			break;
		case 'Q':
		case 'q':
			check_values = MB_NO;
			flag++;
			break;
		case 'R':
		case 'r':
			sscanf (optarg,"%lf/%lf/%lf/%lf", 
				&bounds[0],&bounds[1],&bounds[2],&bounds[3]);
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
		case '?':
			errflg++;
		}

	/* if error flagged then print it and exit */
	if (errflg)
		{
		fprintf(stderr,"usage: %s\n", usage_message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(MB_FAILURE);
		}

	/* print starting message */
	if (verbose == 1)
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
		fprintf(stderr,"dbg2       file:           %s\n",file);
		fprintf(stderr,"dbg2       beam_set:       %d\n",beam_set);
		fprintf(stderr,"dbg2       beam_start:     %d\n",beam_start);
		fprintf(stderr,"dbg2       beam_end:       %d\n",beam_end);
		fprintf(stderr,"dbg2       pixel_set:      %d\n",pixel_set);
		fprintf(stderr,"dbg2       pixel_start:    %d\n",pixel_start);
		fprintf(stderr,"dbg2       pixel_end:      %d\n",pixel_end);
		fprintf(stderr,"dbg2       dump_mode:      %d\n",dump_mode);
		fprintf(stderr,"dbg2       check_values:   %d\n",check_values);
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
		exit(MB_ERROR_NO_ERROR);
		}

	/* initialize reading the multibeam file */
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

	/* figure out whether bath, amp, or ss will be used */
	if (dump_mode == DUMP_MODE_BATH || dump_mode == DUMP_MODE_TOPO)
		use_bath = MB_YES;
	else if (dump_mode == DUMP_MODE_AMP)
		use_amp = MB_YES;
	else if (dump_mode == DUMP_MODE_SS)
		use_ss = MB_YES;
	else
		for (i=0; i<n_list; i++) 
			{
			if (list[i] == 'Z' || list[i] == 'z'
				|| list[i] == 'A' || list[i] == 'a')
				use_bath = MB_YES;
			if (list[i] == 'B')
				use_amp = MB_YES;
			if (list[i] == 'b')
				use_ss = MB_YES;
			}
	if (check_values == MB_YES)
		{
		if (use_bath == MB_YES) check_bath = MB_YES;
		if (use_amp == MB_YES) check_amp = MB_YES;
		if (use_ss == MB_YES) check_ss = MB_YES;
		}

	/* allocate memory for data arrays */
	status = mb_malloc(verbose,beams_bath*sizeof(double),&bath,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
			&bathacrosstrack,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
			&bathalongtrack,&error);
	status = mb_malloc(verbose,beams_amp*sizeof(double),&amp,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),&ss,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),&ssacrosstrack,
			&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),&ssalongtrack,
			&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
			message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* read and print data */
	distance_total = 0.0;
	nread = 0;
	first = MB_YES;
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* read a ping of data */
		status = mb_get(verbose,mbio_ptr,&kind,&pings,time_i,&time_d,
			&navlon,&navlat,&speed,&heading,&distance,
			&beams_bath,&beams_amp,&pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);

		/* time gaps are not a problem here */
		if (error == MB_ERROR_TIME_GAP)
			{
			error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}

		/* increment counter and set cumulative distance */
		if (error <= MB_ERROR_NO_ERROR 
			&& error != MB_ERROR_COMMENT)
			{
			nread++;
			distance_total += distance;
			}

		/* print debug statements */
		if (verbose >= 2)
			{
			fprintf(stderr,"\ndbg2  Ping read in program <%s>\n",
				program_name);
			fprintf(stderr,"dbg2       kind:           %d\n",kind);
			fprintf(stderr,"dbg2       error:          %d\n",error);
			fprintf(stderr,"dbg2       status:         %d\n",status);
			}

		/* print comments */
		if (verbose >= 1 && kind == MB_DATA_COMMENT)
			{
			if (icomment == 0)
				{
				fprintf(stderr,"\nComments:\n");
				icomment++;
				}
			fprintf(stderr,"%s\n",comment);
			}

		/* set output beams and pixels */
		if (error == MB_ERROR_NO_ERROR && first == MB_YES)
			{
			/* reset first flag */
			first = MB_NO;

			/* set and/or check beams and pixels to be output */
			status = set_output(verbose,
				beams_bath,beams_amp,pixels_ss,
				use_bath,use_amp,use_ss,
				dump_mode,beam_set,pixel_set,
				&beam_start,&beam_end,&pixel_start,&pixel_end,
				&n_list,list,&error);

			if (status == MB_FAILURE)
				{
				fprintf(stderr,"\nProgram <%s> Terminated\n",
					program_name);
				exit(MB_FAILURE);
				}


			/* print debug statements */
			if (verbose >= 2)
				{
				fprintf(stderr,"\ndbg2  Beams set for output in <%s>\n",
					program_name);
				fprintf(stderr,"dbg2       status:       %d\n",
					status);
				fprintf(stderr,"dbg2       error:        %d\n",
					error);
				fprintf(stderr,"dbg2       use_bath:     %d\n",
					use_bath);
				fprintf(stderr,"dbg2       use_amp:      %d\n",
					use_amp);
				fprintf(stderr,"dbg2       use_ss:       %d\n",
					use_ss);
				fprintf(stderr,"dbg2       beam_start:   %d\n",
					beam_start);
				fprintf(stderr,"dbg2       beam_end:     %d\n",
					beam_end);
				fprintf(stderr,"dbg2       pixel_start:  %d\n",
					pixel_start);
				fprintf(stderr,"dbg2       pixel_end:    %d\n",
					pixel_end);
				fprintf(stderr,"dbg2       check_bath:   %d\n",
					check_bath);
				fprintf(stderr,"dbg2       check_amp:    %d\n",
					check_amp);
				fprintf(stderr,"dbg2       check_ss:     %d\n",
					check_ss);
				fprintf(stderr,"dbg2       n_list:       %d\n",
					n_list);
				for (i=0;i<n_list;i++)
					fprintf(stderr,"dbg2       list[%d]:      %c\n",
						i,list[i]);
				}
			}

		/* get factors for lon lat calculations */
		if (error == MB_ERROR_NO_ERROR)
			{
			mb_coor_scale(verbose,navlat,&mtodeglon,&mtodeglat);
			headingx = sin(DTR*heading);
			headingy = cos(DTR*heading);
			}

		/* now loop over beams */
		if (error == MB_ERROR_NO_ERROR)
		for (j=beam_start;j<=beam_end;j++)
		  {
		  /* check beam status */
		  beam_status = MB_SUCCESS;
		  if (check_bath == MB_YES && bath[j] <= 0)
			beam_status = MB_FAILURE;
		  if (check_amp == MB_YES && amp[j] <= 0)
			beam_status = MB_FAILURE;
		  if (check_ss == MB_YES && j != beams_bath/2)
			beam_status = MB_FAILURE;
		  else if (check_ss == MB_YES && j == beams_bath/2)
			if (ss[pixels_ss/2] <= 0)
				beam_status = MB_FAILURE;

		  /* print out good pings */
		  if (beam_status == MB_SUCCESS)
		    {
		    for (i=0; i<n_list; i++) 
			{
			switch (list[i]) 
				{
				case 'A': /* Seafloor crosstrack slope */
				case 'a':
					ns = 0;
					sx = 0.0;
					sy = 0.0;
					sxx = 0.0;
					sxy = 0.0;
					for (k=0;k<beams_bath;k++)
					  if (bath[k] > 0.0)
					    {
					    sx += bathacrosstrack[k];
					    sy += bath[k];
					    sxx += bathacrosstrack[k]
						*bathacrosstrack[k];
					    sxy += bathacrosstrack[k]*bath[k];
					    ns++;
					    }
					if (ns > 0)
					  {
					  delta = ns*sxx - sx*sx;
					  a = (sxx*sy - sx*sxy)/delta;
					  b = (ns*sxy - sx*sy)/delta;
					  slope = atan(b)/DTR;
					  }
					else
					  slope = 0.0;
					printf("%.4f",slope);
					break;
				case 'B': /* amplitude */
					printf("%.3f",amp[j]);
					break;
				case 'b': /* sidescan */
					printf("%.3f",ss[pixels_ss/2]);
					break;
				case 'D': /* acrosstrack dist. */
				case 'd':
					printf("%.3f",
					bathacrosstrack[j]);
					break;
				case 'E': /* alongtrack dist. */
				case 'e':
					printf("%.3f",
					bathalongtrack[j]);
					break;
				case 'H': /* heading */
					printf("%5.1f",heading);
					break;
				case 'J': /* time string */
					mb_get_jtime(verbose,time_i,time_j);
					printf(
					"%.4d %.3d %.2d %.2d %.2d.%6.6d",
					time_j[0],time_j[1],
					time_i[3],time_i[4],
					time_i[5],time_i[6]);
					break;
				case 'j': /* time string */
					mb_get_jtime(verbose,time_i,time_j);
					printf(
					"%.4d %.3d %.4d %.2d.%6.6d",
					time_j[0],time_j[1],
					time_j[2],time_j[3],time_j[4]);
					break;
				case 'L': /* along-track dist. */
					printf("%7.3f",distance_total);
					break;
				case 'M': /* Decimal unix seconds since 
						1/1/70 00:00:00 */
					printf("%.6f",time_d);
					break;
				case 'm': /* time in decimal seconds since 
						first record */
					if (first_m == MB_YES)
						{
						time_d_ref = time_d;
						first_m = MB_NO;
						}
					printf("%.6f",time_d - time_d_ref);
					break;
				case 'N': /* ping counter */
					printf("%6d",nread);
					break;
				case 'S': /* speed */
					printf("%5.2f",speed);
					break;
				case 'T': /* yyyy/mm/dd/hh/mm/ss time string */
					printf(
					"%.4d/%.2d/%.2d/%.2d/%.2d/%.2d.%.6d",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],
					time_i[6]);
					break;
				case 't': /* yyyy mm dd hh mm ss time string */
					printf(
					"%.4d %.2d %.2d %.2d %.2d %.2d.%.6d",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],
					time_i[6]);
					break;
				case 'U': /* unix time in seconds since 1/1/70 00:00:00 */
					time_u = (int) time_d;
					printf("%d",time_u);
					break;
				case 'u': /* time in seconds since first record */
					time_u = (int) time_d;
					if (first_u == MB_YES)
						{
						time_u_ref = time_u;
						first_u = MB_NO;
						}
					printf("%d",time_u - time_u_ref);
					break;
				case 'X': /* longitude */
				case 'x':
					if (j == beams_bath/2)
						printf("%11.6f",navlon);
					else
						{
						dlon = navlon 
						+ headingy*mtodeglon
							*bathacrosstrack[j]
						+ headingx*mtodeglon
							*bathalongtrack[j];
						printf("%11.6f",dlon);
						}
					break;
				case 'Y': /* latitude */
				case 'y':
					if (j == beams_bath/2)
						printf("%10.6f",navlat);	
					else
						{
						dlat = navlat 
						- headingx*mtodeglat
							*bathacrosstrack[j]
						+ headingy*mtodeglat
							*bathalongtrack[j];
						printf("%11.6f",dlat);
						}
					break;
				case 'Z': /* topography */
					printf("%.3f",-bath[j]);
					break;
				case 'z': /* depth */
					printf("%6.3f",bath[j]);
					break;
				case '#': /* beam number */
					printf("%6d",j);
					break;
				default:
					printf("<Invalid Option: %c>",
						list[i]);
					break;
				}
				if (i<(n_list-1)) printf ("\t");
				else printf ("\n");
			}
		    }
		  }

		/* now loop over pixels */
		if (error == MB_ERROR_NO_ERROR)
		for (j=pixel_start;j<=pixel_end;j++)
		  {
		  /* check pixel status */
		  pixel_status = MB_SUCCESS;
		  if (check_bath == MB_YES && j != pixels_ss/2)
			pixel_status = MB_FAILURE;
		  else if (check_bath == MB_YES && j == pixels_ss/2)
			if (bath[beams_bath/2] <= 0)
				pixel_status = MB_FAILURE;
		  if (check_amp == MB_YES && j != pixels_ss/2)
			pixel_status = MB_FAILURE;
		  else if (check_amp == MB_YES && j == pixels_ss/2)
			if (amp[beams_bath/2] <= 0)
				pixel_status = MB_FAILURE;
		  if (check_ss == MB_YES && ss[j] <= 0)
			pixel_status = MB_FAILURE;

		  /* print out good pings */
		  if (pixel_status == MB_SUCCESS)
		    {
		    for (i=0; i<n_list; i++) 
			{
			switch (list[i]) 
				{
				case 'A': /* Seafloor crosstrack slope */
				case 'a':
					ns = 0;
					sx = 0.0;
					sy = 0.0;
					sxx = 0.0;
					sxy = 0.0;
					for (k=0;k<beams_bath;k++)
					  if (bath[k] > 0.0)
					    {
					    sx += bathacrosstrack[k];
					    sy += bath[k];
					    sxx += bathacrosstrack[k]
						*bathacrosstrack[k];
					    sxy += bathacrosstrack[k]*bath[k];
					    ns++;
					    }
					if (ns > 0)
					  {
					  delta = ns*sxx - sx*sx;
					  a = (sxx*sy - sx*sxy)/delta;
					  b = (ns*sxy - sx*sy)/delta;
					  slope = atan(b)/DTR;
					  }
					else
					  slope = 0.0;
					printf("%.4f",slope);
					break;
				case 'B': /* amplitude */
					printf("%.3f",amp[beams_bath/2]);
					break;
				case 'b': /* sidescan */
					printf("%6d",ss[j]);
					break;
				case 'D': /* acrosstrack dist. */
				case 'd':
					printf("%5d",
					ssacrosstrack[j]);
					break;
				case 'E': /* alongtrack dist. */
				case 'e':
					printf("%5d",
					ssalongtrack[j]);
					break;
				case 'H': /* heading */
					printf("%5.1f",heading);
					break;
				case 'J': /* time string */
					mb_get_jtime(verbose,time_i,time_j);
					printf(
					"%.4d %.3d %.2d %.2d %.2d",
					time_j[0],time_j[1],
					time_i[3],time_i[4],time_i[5]);
					break;
				case 'j': /* time string */
					mb_get_jtime(verbose,time_i,time_j);
					printf(
					"%.4d %.3d %.4d %.2d",
					time_j[0],time_j[1],
					time_j[2],time_j[3]);
					break;
				case 'L': /* along-track dist. */
					printf("%7.3f",distance_total);
					break;
				case 'M': /* Unix time in decimal seconds since 1/1/70 00:00:00 */
					printf("%.6f",time_d);
					break;
				case 'm': /* time in decimal seconds since first record */
					if (first_m == MB_YES)
						{
						time_d_ref = time_d;
						first_m = MB_NO;
						}
					printf("%.6f",time_d - time_d_ref);
					break;
				case 'N': /* ping counter */
					printf("%6d",nread);
					break;
				case 'S': /* speed */
					printf("%5.2f",speed);
					break;
				case 'T': /* yyyy/mm/dd/hh/mm/ss time string */
					printf(
					"%.4d/%.2d/%.2d/%.2d/%.2d/%.2d",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5]);
					break;
				case 't': /* yyyy mm dd hh mm ss time string */
					printf(
					"%.4d %.2d %.2d %.2d %.2d %.2d",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5]);
					break;
				case 'U': /* unix time in seconds since 1/1/70 00:00:00 */
					time_u = (int) time_d;
					printf("%d",time_u);
					break;
				case 'u': /* time in seconds since first record */
					time_u = (int) time_d;
					if (first_u == MB_YES)
						{
						time_u_ref = time_u;
						first_u = MB_NO;
						}
					printf("%d",time_u - time_u_ref);
					break;
				case 'X': /* longitude */
				case 'x':
					if (j == pixels_ss/2)
						printf("%11.6f",navlon);
					else
						{
						dlon = navlon 
						+ headingy*mtodeglon
							*ssacrosstrack[j]
						+ headingy*mtodeglon
							*ssalongtrack[j];
						printf("%11.6f",dlon);
						}
					break;
				case 'Y': /* latitude */
				case 'y':
					if (j == pixels_ss/2)
						printf("%10.6f",navlat);	
					else
						{
						dlat = navlat 
						- headingx*mtodeglat
							*ssacrosstrack[j]
						- headingx*mtodeglat
							*ssalongtrack[j];
						printf("%11.6f",dlat);
						}
					break;
				case 'Z': /* topography */
					printf("%6d",-bath[beams_bath/2]);
					break;
				case 'z': /* depth */
					printf("%6d",bath[beams_bath/2]);
					break;
				case '#': /* pixel number */
					printf("%6d",j);
					break;
				default:
					printf("<Invalid Option: %c>",
						list[i]);
					break;
				}
				if (i<(n_list-1)) printf ("\t");
				else printf ("\n");
			}
		    }
		  }

		}

	/* close the multibeam file */
	status = mb_close(verbose,&mbio_ptr,&error);

	/* deallocate memory used for data arrays */
	mb_free(verbose,&bath,&error); 
	mb_free(verbose,&bathacrosstrack,&error); 
	mb_free(verbose,&bathalongtrack,&error); 
	mb_free(verbose,&amp,&error); 
	mb_free(verbose,&ss,&error); 
	mb_free(verbose,&ssacrosstrack,&error); 
	mb_free(verbose,&ssalongtrack,&error); 

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
	exit(status);
}
/*--------------------------------------------------------------------*/
int set_output(verbose,beams_bath,beams_amp,pixels_ss,
		use_bath,use_amp,use_ss,dump_mode,beam_set,pixel_set,
		beam_start,beam_end,pixel_start,pixel_end,
		n_list,list,error)
int	verbose;
int	beams_bath;
int	beams_amp;
int	pixels_ss;
int	use_bath;
int	use_amp;
int	use_ss;
int	dump_mode;
int	beam_set;
int	pixel_set;
int	*beam_start;
int	*beam_end;
int	*pixel_start;
int	*pixel_end;
int	*n_list;
char	*list;
int	*error;
{
	char	*function_name = "set_output";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBCOPY function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:       %d\n",verbose);
		fprintf(stderr,"dbg2       beams_bath:    %d\n",beams_bath);
		fprintf(stderr,"dbg2       beams_amp:     %d\n",beams_amp);
		fprintf(stderr,"dbg2       pixels_ss:     %d\n",pixels_ss);
		fprintf(stderr,"dbg2       use_bath:      %d\n",use_bath);
		fprintf(stderr,"dbg2       use_amp:       %d\n",use_amp);
		fprintf(stderr,"dbg2       use_ss:        %d\n",use_ss);
		fprintf(stderr,"dbg2       dump_mode:     %d\n",dump_mode);
		fprintf(stderr,"dbg2       beam_set:      %d\n",beam_set);
		fprintf(stderr,"dbg2       pixel_set:     %d\n",pixel_set);
		fprintf(stderr,"dbg2       beam_start:    %d\n",*beam_start);
		fprintf(stderr,"dbg2       beam_end:      %d\n",*beam_end);
		fprintf(stderr,"dbg2       pixel_start:   %d\n",*pixel_start);
		fprintf(stderr,"dbg2       pixel_end:     %d\n",*pixel_end);
		fprintf(stderr,"dbg2       n_list:        %d\n",*n_list);
		for (i=0;i<*n_list;i++)
		fprintf(stderr,"dbg2       list[%2d]:      %c\n",
						i,list[i]);
		}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	if (beam_set == MB_NO && pixel_set == MB_YES)
		{
		*beam_start = 0;
		*beam_end = -1;
		}
	else if (beam_set == MB_NO && beams_bath <= 0)
		{
		*beam_start = 0;
		*beam_end = -1;
		*pixel_start = pixels_ss/2;
		*pixel_end = pixels_ss/2;
		}
	else if (beam_set == MB_NO)
		{
		*beam_start = beams_bath/2;
		*beam_end = beams_bath/2;
		}
	if (pixel_set == MB_NO && beams_bath > 0)
		{
		*pixel_start = 0;
		*pixel_end = -1;
		}

	/* deal with dump_mode if set */
	if (dump_mode == DUMP_MODE_BATH)
		{
		*beam_start = 0;
		*beam_end = beams_bath - 1;
		*pixel_start = 0;
		*pixel_end = -1;
		strcpy(list,"XYz");
		*n_list = 3;
		}
	else if (dump_mode == DUMP_MODE_TOPO)
		{
		*beam_start = 0;
		*beam_end = beams_bath - 1;
		*pixel_start = 0;
		*pixel_end = -1;
		strcpy(list,"XYZ");
		*n_list = 3;
		}
	else if (dump_mode == DUMP_MODE_AMP)
		{
		*beam_start = 0;
		*beam_end = beams_bath - 1;
		*pixel_start = 0;
		*pixel_end = -1;
		strcpy(list,"XYB");
		*n_list = 3;
		}
	else if (dump_mode == DUMP_MODE_SS)
		{
		*beam_start = 0;
		*beam_end = -1;
		*pixel_start = 0;
		*pixel_end = pixels_ss - 1;
		strcpy(list,"XYb");
		*n_list = 3;
		}

	/* check if beam and pixel range is ok */
	if ((use_bath == MB_YES || *beam_end >= *beam_start)
		&& beams_bath <= 0)
		{
		fprintf(stderr,"\nBathymetry data not available\n");
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_USAGE;
		}
	else if (*beam_end >= *beam_start && 
		(*beam_start < 0 || *beam_end >= beams_bath))
		{
		fprintf(stderr,"\nBeam range %d to %d exceeds available beams 0 to %d\n",
			*beam_start,*beam_end,beams_bath-1);
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_USAGE;
		}
	if (*error == MB_ERROR_NO_ERROR 
		&& use_amp == MB_YES && beams_amp <= 0)
		{
		fprintf(stderr,"\nAmplitude data not available\n");
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_USAGE;
		}
	else if (*error == MB_ERROR_NO_ERROR 
		&& *beam_end >= *beam_start && use_amp == MB_YES &&
		(*beam_start < 0 || *beam_end >= beams_amp))
		{
		fprintf(stderr,"\nAmplitude beam range %d to %d exceeds available beams 0 to %d\n",
			*beam_start,*beam_end,beams_amp-1);
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_USAGE;
		}
	if (*error == MB_ERROR_NO_ERROR 
		&& (use_ss == MB_YES || *pixel_end >= *pixel_start)
		&& pixels_ss <= 0)
		{
		fprintf(stderr,"\nSidescan data not available\n");
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_USAGE;
		}
	else if (*error == MB_ERROR_NO_ERROR 
		&& *pixel_end >= *pixel_start && 
		(*pixel_start < 0 || *pixel_end >= pixels_ss))
		{
		fprintf(stderr,"\nPixels range %d to %d exceeds available pixels 0 to %d\n",
			*pixel_start,*pixel_end,pixels_ss-1);
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_USAGE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBCOPY function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2       beam_start:    %d\n",*beam_start);
		fprintf(stderr,"dbg2       beam_end:      %d\n",*beam_end);
		fprintf(stderr,"dbg2       pixel_start:   %d\n",*pixel_start);
		fprintf(stderr,"dbg2       pixel_end:     %d\n",*pixel_end);
		fprintf(stderr,"dbg2       n_list:        %d\n",*n_list);
		for (i=0;i<*n_list;i++)
			fprintf(stderr,"dbg2       list[%2d]:      %c\n",
						i,list[i]);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
