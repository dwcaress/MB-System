/*--------------------------------------------------------------------
 *    The MB-system:	mbgetmask.c	6/15/93
 *    $Id: mbgetmask.c,v 5.5 2006-01-18 15:17:00 caress Exp $
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
 * MBGETMASK reads a multibeam data file and writes out
 * a data flag mask to stdout which can be applied to other data files
 * containing the same data (but presumably in a different
 * state of processing).  This allows editing of one data file to
 * be transferred to another with ease.  The program MBMASK is
 * used to apply the flag mask to another file.
 * The default input streams is stdin.
 *
 * Author:	D. W. Caress
 * Date:	June 15, 1993
 * 
 * $Log: not supported by cvs2svn $
 * Revision 5.4  2005/03/25 04:43:00  caress
 * Standardized the string lengths used for filenames and comment data.
 *
 * Revision 5.3  2003/04/17 21:17:10  caress
 * Release 5.0.beta30
 *
 * Revision 5.2  2001/07/20 00:34:38  caress
 * Release 5.0.beta03
 *
 * Revision 5.1  2001/03/22 21:15:49  caress
 * Trying to make release 5.0.beta0.
 *
 * Revision 5.0  2000/12/01  22:57:08  caress
 * First cut at Version 5.0.
 *
 * Revision 4.13  2000/10/11  01:06:15  caress
 * Convert to ANSI C
 *
 * Revision 4.12  2000/09/30  07:06:28  caress
 * Snapshot for Dale.
 *
 * Revision 4.11  1999/08/08  04:17:40  caress
 * Unknown changes.
 *
 * Revision 4.10  1999/03/31  18:33:06  caress
 * MB-System 4.6beta7
 *
 * Revision 4.9  1998/10/05  19:19:24  caress
 * MB-System version 4.6beta
 *
 * Revision 4.8  1997/04/21  17:19:14  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.8  1997/04/17  15:14:38  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 4.7  1996/04/22  13:23:05  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.6  1996/03/11  15:31:18  caress
 * Changed so that flag mask set for zero as well as negative
 * depths.
 *
 * Revision 4.5  1995/05/12  17:12:32  caress
 * Made exit status values consistent with Unix convention.
 * 0: ok  nonzero: error
 *
 * Revision 4.4  1995/03/06  19:37:59  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.3  1995/03/02  13:49:21  caress
 * Fixed bug related to error messages.
 *
 * Revision 4.2  1994/10/21  13:02:31  caress
 * Release V4.0
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
 * Revision 3.0  1993/06/21  01:21:00  caress
 * Initial revision.
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

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"

/*--------------------------------------------------------------------*/

main (int argc, char **argv)
{
	/* id variables */
	static char rcs_id[] = "$Id: mbgetmask.c,v 5.5 2006-01-18 15:17:00 caress Exp $";
	static char program_name[] = "MBGETMASK";
	static char help_message[] =  "MBGETMASK reads a multibeam data file and writes out \na data flag mask to stdout which can be applied to other data files \ncontaining the same data (but presumably in a different \nstate of processing).  This allows editing of one data file to \nbe transferred to another with ease.  The program MBMASK is \nused to apply the flag mask to another file. \nThe default input stream is stdin.";
	static char usage_message[] = "mbgetmask [-Fformat -Byr/mo/da/hr/mn/sc -Eyr/mo/da/hr/mn/sc -Sspeed -Iinfile -V -H]";

	/* parsing variables */
	extern char *optarg;
	extern int optkind;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* MBIO status variables */
	int	status;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;
	char	*message = NULL;

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
	char	ifile[MB_PATH_MAXLINE];
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	void	*imbio_ptr = NULL;

	/* mbio read and write values */
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
	int	idata = 0;
	int	omask = 0;
	int	beam_ok = 0;
	int	beam_null = 0;
	int	beam_flag = 0;
	int	beam_flag_manual = 0;
	int	beam_flag_filter = 0;
	int	beam_flag_sonar = 0;
	char	comment[MB_COMMENT_MAXLINE];
	int	kluge = 0;

	/* time, user, host variables */
	time_t	right_now;
	char	date[25], user[128], *user_ptr, host[128];

	int	i, j, k, l, m;

	char	*ctime();
	char	*getenv();

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* reset all defaults but the format and lonflip */
	pings = 1;
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
	strcpy (ifile, "stdin");

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhB:b:E:F:f:I:i:K:k:S:s:")) != -1)
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
			sscanf (optarg,"%s", ifile);
			flag++;
			break;
		case 'K':
		case 'k':
			sscanf (optarg,"%d", &kluge);
			flag++;
			break;
		case 'S':
		case 's':
			sscanf (optarg,"%lf", &speedmin);
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
		fprintf(stderr,"dbg2       data format:    %d\n",format);
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
		fprintf(stderr,"dbg2       input file:     %s\n",ifile);
		fprintf(stderr,"dbg2       kluge:	   %d\n",kluge);
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
		mb_get_format(verbose,ifile,NULL,&format,&error);

	/* initialize reading the input multibeam file */
	if ((status = mb_read_init(
		verbose,ifile,format,pings,lonflip,bounds,
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

	/* allocate memory for data arrays */
	status = mb_malloc(verbose,beams_bath*sizeof(char),&beamflag,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),&bath,&error);
	status = mb_malloc(verbose,beams_amp*sizeof(double),&amp,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
			&bathacrosstrack,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
			&bathalongtrack,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),&ss,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&ssacrosstrack,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),
			&ssalongtrack,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* write comments to beginning of output mask */
	printf("# Multibeam Data Flagging Mask\n");
	printf("# Created by program %s\n# Version: %s\n",program_name,rcs_id);
	printf("# MB-System Version: %s\n",MB_VERSION);
	strncpy(date,"\0",25);
	right_now = time((time_t *)0);
	strncpy(date,ctime(&right_now),24);
	if ((user_ptr = getenv("USER")) == NULL)
		user_ptr = getenv("LOGNAME");
	if (user_ptr != NULL)
		strcpy(user,user_ptr);
	else
		strcpy(user, "unknown");
	gethostname(host,128);
	printf("# Run by user <%s> on cpu <%s> at <%s>\n",user,host,date);
	printf("# These ASCII lines at the beginning of the mask file\n");
	printf("#   are comments. One additional line of text follows\n");
	printf("#   the comments; this gives the maximum number of\n");
	printf("#   beams and the mask version:\n");
	printf("#       beams_max dummy version\n");
	printf("#   The mask data is binary. For each ping,  the initial\n");
	printf("#   32 bytes consist of eight four int values:\n");
	printf("#       int time_i[0] = year\n");
	printf("#       int time_i[1] = month\n");
	printf("#       int time_i[2] = day\n");
	printf("#       int time_i[3] = hour\n");
	printf("#       int time_i[4] = minute\n");
	printf("#       int time_i[5] = second\n");
	printf("#       int time_i[6] = usecond\n");
	printf("#       int beams_bath = number of beams\n");
	printf("#   These values are followed by an array of mask values\n");
	printf("#   (one byte each) beams_bath long. The mask values \n");
	printf("#   are as follows:\n");
	printf("#       0: good beam\n");
	printf("#       1: null beam\n");
	printf("#       3: beam flagged, manual editing\n");
	printf("#       4: beam flagged, filter\n");
	printf("#       5: beam flagged, > 1X IHO accuracy\n");
	printf("#       6: beam flagged, > 2X IHO accuracy\n");
	printf("#       7: beam flagged, footprint too large\n");
	printf("#       8: beam flagged, by sonar\n");
	printf("# Max number of bathymetry beams:   %d\n",beams_bath);
	printf("# Control Parameters:\n");
	printf("#   MBIO data format:   %d\n",format);
	printf("#   Input file:         %s\n",ifile);
	printf("#   Longitude flip:     %d\n",lonflip);
	printf("#   Longitude bounds:   %f %f\n",bounds[0],bounds[1]);
	printf("#   Latitude bounds:    %f %f\n",bounds[2],bounds[3]);
	printf("#   Begin time:         %d %d %d %d %d %d.%6.6d\n",
			btime_i[0],btime_i[1],btime_i[2],
			btime_i[3],btime_i[4],btime_i[5],btime_i[6]);
	printf("#   End time:           %d %d %d %d %d %d.%6.6d\n",
			etime_i[0],etime_i[1],etime_i[2],
			etime_i[3],etime_i[4],etime_i[5],etime_i[6]);
	printf("#   Minimum speed:      %f\n",speedmin);
	printf("# \n");
	
	/* set version 3 in header */
	printf("%4d %4d 3\n",beams_bath,0);

	/* read and write */
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* read some data */
		error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		status = mb_get_all(verbose,imbio_ptr,&store_ptr,&kind,
			time_i,&time_d,&navlon,&navlat,
			&speed,&heading,
			&distance,&altitude,&sonardepth,
			&nbath,&namp,&nss,
			beamflag,bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);

		/* increment counter */
		if (error <= MB_ERROR_NO_ERROR 
			&& kind == MB_DATA_DATA)
			idata = idata + pings;

		/* time gaps do not matter to mbgetmask */
		if (error == MB_ERROR_TIME_GAP)
			{
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
			}

		/* time bounds do not matter to mbgetmask */
		if (error == MB_ERROR_OUT_TIME)
			{
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
			}

		/* space bounds do not matter to mbgetmask */
		if (error == MB_ERROR_OUT_BOUNDS)
			{
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
			}

		/* output error messages */
		if (verbose >= 1 && error < MB_ERROR_NO_ERROR
			&& error >= MB_ERROR_OTHER
			&& error != MB_ERROR_COMMENT)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nNonfatal MBIO Error:\n%s\n",message);
			fprintf(stderr,"Input Record: %d\n",idata);
			fprintf(stderr,"Time: %d %d %d %d %d %d %d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6]);
			}
		else if (verbose >= 1 && error < MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nNonfatal MBIO Error:\n%s\n",message);
			fprintf(stderr,"Number of good records so far: %d\n",idata);
			}
		else if (verbose >= 1 && error != MB_ERROR_NO_ERROR 
			&& error != MB_ERROR_EOF)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nFatal MBIO Error:\n%s\n",message);
			fprintf(stderr,"Last Good Time: %d %d %d %d %d %d %d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6]);
			}
			
		/* fix a problem with EM300/EM3000 data in HDCS format */
		if (format == 151 && kluge == 1)
		    {
		    for (i=0;i<nbath-1;i++)
			beamflag[i] = beamflag[i+1];
		    beamflag[nbath-1] = MB_FLAG_FLAG;
		    }
		    
		/* count the flags */
		for (i=0;i<nbath;i++)
		    {
		    if (mb_beam_ok(beamflag[i]))
			{
			beam_ok++;
			}
		    else if (mb_beam_check_flag_null(beamflag[i]))
			{
			beam_null++;
			}
		    else
			{
			beam_flag++;
			if (mb_beam_check_flag_manual(beamflag[i]))
			    beam_flag_manual++;
			if (mb_beam_check_flag_filter(beamflag[i]))
			    beam_flag_filter++;
			if (mb_beam_check_flag_sonar(beamflag[i]))
			    beam_flag_sonar++;
			}
		    }

		/* write some flags */
		if (kind == MB_DATA_DATA
			&& error == MB_ERROR_NO_ERROR)
			{
			omask++;
			fwrite(time_i, sizeof(int), 7, stdout);
			fwrite(&nbath, sizeof(int), 1, stdout);
			fwrite(beamflag, 1, nbath, stdout);
			}
		}

	/* close the file */
	status = mb_close(verbose,&imbio_ptr,&error);

	/* deallocate memory for data arrays */
	mb_free(verbose,&beamflag,&error); 
	mb_free(verbose,&bath,&error); 
	mb_free(verbose,&amp,&error); 
	mb_free(verbose,&bathacrosstrack,&error); 
	mb_free(verbose,&bathalongtrack,&error); 
	mb_free(verbose,&ss,&error); 
	mb_free(verbose,&ssacrosstrack,&error); 
	mb_free(verbose,&ssalongtrack,&error); 

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* give the statistics */
	if (verbose >= 1)
		{
		fprintf(stderr,"\nData records:\n");
		fprintf(stderr,"\t%d input data records\n",idata);
		fprintf(stderr,"\t%d output mask records\n",omask);
		fprintf(stderr,"\nBeam flag totals:\n");
		fprintf(stderr,"\t%d beams ok\n",beam_ok);
		fprintf(stderr,"\t%d beams null\n",beam_null);
		fprintf(stderr,"\t%d beams flagged\n",beam_flag);
		fprintf(stderr,"\t%d beams flagged manually\n",beam_flag_manual);
		fprintf(stderr,"\t%d beams flagged by filter\n",beam_flag_filter);
		fprintf(stderr,"\t%d beams flagged by sonar\n",beam_flag_sonar);
		}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/
