/*--------------------------------------------------------------------
 *    The MB-system:	mbgetmask.c	6/15/93
 *    $Id: mbgetmask.c,v 4.4 1995-03-06 19:37:59 caress Exp $
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
#include <math.h>
#include <string.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"

/*--------------------------------------------------------------------*/

main (argc, argv)
int argc;
char **argv; 
{
	/* id variables */
	static char rcs_id[] = "$Id: mbgetmask.c,v 4.4 1995-03-06 19:37:59 caress Exp $";
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
	char	ifile[128];
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	char	*imbio_ptr = NULL;

	/* mbio read and write values */
	char	*store_ptr;
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
	int	idata = 0;
	int	icomment = 0;
	int	omask = 0;
	int	ocomment = 0;
	int	flagged = 0;
	int	data_use;
	char	comment[256];

	/* time, user, host variables */
	long int	right_now;
	char	date[25], user[128], host[128];

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
	while ((c = getopt(argc, argv, "VvHhB:b:E:S:s:F:f:I:i:")) != -1)
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
		case 'S':
		case 's':
			sscanf (optarg,"%lf", &speedmin);
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
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(MB_ERROR_NO_ERROR);
		}

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
	right_now = time((long *)0);
	strncpy(date,"\0",25);
	right_now = time((long *)0);
	strncpy(date,ctime(&right_now),24);
	strcpy(user,getenv("USER"));
	gethostname(host,128);
	printf("# Run by user <%s> on cpu <%s> at <%s>\n",user,host,date);
	printf("# Lines beginning with # are comments.  The first\n");
	printf("#   uncommented line has the numbers of bathymetry\n");
	printf("#   and amplitude beams in each ping.  Each ping\n");
	printf("#   is represented by three lines.  The first line\n");
	printf("#   contains the time tag.  The second line consists of\n");
	printf("#   the mask values for the bathymetry beams.  The third\n");
	printf("#   line consists of the mask values for the amplitude\n");
	printf("#   beams.  Mask values of 0 denote flagged beams and\n");
	printf("#   mask values of 1 denote unflagged beams.\n");
	printf("# Bathymetry beams:   %d\n",beams_bath);
	printf("# Amplitude beams:    %d\n",beams_amp);
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
	printf("%4d %4d\n",beams_bath,beams_amp);

	/* read and write */
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* read some data */
		error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		status = mb_get_all(verbose,imbio_ptr,&store_ptr,&kind,
			time_i,&time_d,&navlon,&navlat,&speed,
			&heading,&distance,
			&beams_bath,&beams_amp,&pixels_ss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,&error);

		/* increment counter */
		if (error <= MB_ERROR_NO_ERROR 
			&& kind == MB_DATA_DATA)
			idata = idata + pings;
		else if (error <= MB_ERROR_NO_ERROR 
			&& kind == MB_DATA_COMMENT)
			icomment++;

		/* time gaps do not matter to mbgetmask */
		if (error == MB_ERROR_TIME_GAP)
			{
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
			}

		/* output error messages */
		if (verbose >= 1 && error == MB_ERROR_COMMENT)
			{
			if (icomment == 1)
				fprintf(stderr,"\nComments:\n");
			fprintf(stderr,"%s\n",comment);
			}
		else if (verbose >= 1 && error < MB_ERROR_NO_ERROR
			&& error >= MB_ERROR_OTHER)
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

		/* process some data */
		data_use = MB_NO;
		if (kind == MB_DATA_DATA
			&& error == MB_ERROR_NO_ERROR)
			{
			omask++;
			printf("%4d %2d %2d %2d %2d %2d %6.6d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6]);
			for (i=0;i<beams_bath;i++)
				if (bath[i] >= 0.0)
					printf("1");
				else
					{
					printf("0");
					flagged++;
					}
			printf("\n");
			for (i=0;i<beams_amp;i++)
				if (amp[i] >= 0.0)
					printf("1");
				else
					{
					printf("0");
					flagged++;
					}
			printf("\n");
			}
		}

	/* close the file */
	status = mb_close(verbose,&imbio_ptr,&error);

	/* deallocate memory for data arrays */
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
		fprintf(stderr,"\n%d input data records\n",idata);
		fprintf(stderr,"%d input comment records\n",icomment);
		fprintf(stderr,"%d output mask records\n",omask);
		fprintf(stderr,"%d output comment records\n",ocomment);
		fprintf(stderr,"%d beams flagged\n",flagged);
		}

	/* end it all */
	exit(status);
}
/*--------------------------------------------------------------------*/
