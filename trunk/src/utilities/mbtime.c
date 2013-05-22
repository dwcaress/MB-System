/*--------------------------------------------------------------------
 *    The MB-system:	mbtime.c	6/5/2008
 *    $Id$
 *
 *    Copyright (c) 2008-2013 by
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
 * MBTIME converts time values between epoch seconds (seconds since
 * 1970/01/01 00:00:00.000000) and calendar time (e.g. 2008/006/05/17/24/32/0).
 * The input time is set using the command line arguments -Mtime_d for
 * epoch seconds and -Tyear/month/day/hour/minute/second/microsecond for
 * calendar time. The output time (in the form not specified as input) is
 * written to stdout.
 *
 * Author:	D. W. Caress
 * Date:	June 5, 2008
 *
 * $Log: mbtime.c,v $
 * Revision 5.0  2008/07/19 07:52:03  caress
 * Added program mbtime.
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

/* MBIO include files */
#include "mb_status.h"
#include "mb_define.h"

#define MBTIME_INPUT_EPOCH	0
#define MBTIME_INPUT_CALENDAR	1

static char rcs_id[] = "$Id$";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	char program_name[] = "MBTIME";
	char help_message[] =  "MBTIME converts time values between epoch seconds (seconds since \n1970/01/01 00:00:00.000000) and calendar time (e.g. 2008/006/05/17/24/32/0). \nThe input time is set using the command line arguments -Mtime_d for \nepoch seconds and -Tyear/month/day/hour/minute/second/microsecond for \ncalendar time. The output time (in the form not specified as input) is \nwritten to stdout.";
	char usage_message[] = "mbtime [-Mtime_d -Tyear/month/day/hour/minute/second -V -H]";
	extern char *optarg;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* MBIO status variables */
	int	status = MB_SUCCESS;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;

	/* time conversion variables */
	int	mode = MBTIME_INPUT_EPOCH;
	int	time_i[7];
	double	time_d = 0.0;
	double	seconds;

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhM:m:T:t:")) != -1)
	  switch (c)
		{
		case 'H':
		case 'h':
			help++;
			break;
		case 'M':
		case 'm':
			sscanf (optarg,"%lf", &time_d);
			mode = MBTIME_INPUT_EPOCH;
			flag++;
			break;
		case 'T':
		case 't':
			sscanf (optarg,"%d/%d/%d/%d/%d/%lf",
				&time_i[0],&time_i[1],&time_i[2],
				&time_i[3],&time_i[4],&seconds);
			time_i[5] = (int)seconds;
			time_i[6] = (int)(1000000 * (seconds - time_i[5]));
			mode = MBTIME_INPUT_CALENDAR;
			flag++;
			break;
		case 'V':
		case 'v':
			verbose++;
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
		fprintf(stdout,"\nProgram %s\n",program_name);
		fprintf(stdout,"Version %s\n",rcs_id);
		fprintf(stdout,"MB-system Version %s\n",MB_VERSION);
		}

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s>\n",program_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Control Parameters:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       help:       %d\n",help);
		fprintf(stderr,"dbg2       mode:       %d\n",mode);
		fprintf(stderr,"dbg2       time_i[0]:  %d\n",time_i[0]);
		fprintf(stderr,"dbg2       time_i[1]:  %d\n",time_i[1]);
		fprintf(stderr,"dbg2       time_i[2]:  %d\n",time_i[2]);
		fprintf(stderr,"dbg2       time_i[3]:  %d\n",time_i[3]);
		fprintf(stderr,"dbg2       time_i[4]:  %d\n",time_i[4]);
		fprintf(stderr,"dbg2       time_i[5]:  %d\n",time_i[5]);
		fprintf(stderr,"dbg2       time_i[6]:  %d\n",time_i[6]);
		fprintf(stderr,"dbg2       time_d:     %f\n",time_d);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* convert to calendar time and output */
	if (mode == MBTIME_INPUT_EPOCH)
		{
		mb_get_date(verbose, time_d, time_i);
		fprintf(stdout, "%4.4d/%2.2d/%2.2d/%2.2d/%2.2d/%2.2d.%6.6d\n",
			time_i[0], time_i[1], time_i[2],
			time_i[3], time_i[4], time_i[5], time_i[6]);
		}

	/* else convert to epoch time and output */
	else
		{
		mb_get_time(verbose, time_i, &time_d);
		fprintf(stdout, "%f\n", time_d);
		}

	/* set program status */
	status = MB_SUCCESS;

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
