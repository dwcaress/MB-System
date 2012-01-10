/*--------------------------------------------------------------------
 *    The MB-system:	mbdefaults.c	1/23/93
 *	$Id$
 *
 *    Copyright (c) 1993-2012 by
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
 * MBDEFAULTS sets and retrieves the default MBIO control parameters 
 * stored in the file ~/.mbio_defaults.  Only the parameters specified
 * by command line arguments will be changed; if no ~/.mbio_defaults
 * file exists one will be created.
 *
 * Author:	D. W. Caress
 * Date:	January 23, 1993
 * $Log: mbdefaults.c,v $
 * Revision 5.6  2008/12/31 08:47:38  caress
 * Updates towards release 5.1.1
 *
 * Revision 5.5  2006/01/18 15:17:00  caress
 * Added stdlib.h include.
 *
 * Revision 5.4  2003/04/17 21:17:10  caress
 * Release 5.0.beta30
 *
 * Revision 5.3  2001/11/20 21:50:38  caress
 * The .mbio_defaults file no longer controls format,
 * pings, bounds, btime_i, and etime_i.
 *
 * Revision 5.2  2001/06/03  07:07:34  caress
 * Release 5.0.beta01.
 *
 * Revision 5.1  2001/03/22 21:14:16  caress
 * Trying to make release 5.0.beta0.
 *
 * Revision 5.0  2000/12/01  22:57:08  caress
 * First cut at Version 5.0.
 *
 * Revision 4.7  2000/10/11  01:06:15  caress
 * Convert to ANSI C
 *
 * Revision 4.6  2000/09/30  07:06:28  caress
 * Snapshot for Dale.
 *
 * Revision 4.5  1998/10/05  19:19:24  caress
 * MB-System version 4.6beta
 *
 * Revision 4.4  1997/04/21  17:19:14  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.3  1995/05/12  17:12:32  caress
 * Made exit status values consistent with Unix convention.
 * 0: ok  nonzero: error
 *
 * Revision 4.3  1995/05/12  17:12:32  caress
 * Made exit status values consistent with Unix convention.
 * 0: ok  nonzero: error
 *
 * Revision 4.2  1995/03/06  19:37:59  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.1  1994/10/21  13:02:31  caress
 * Release V4.0
 *
 * Revision 4.0  1994/03/06  00:13:22  caress
 * First cut at version 4.0
 *
 * Revision 4.0  1994/03/01  18:59:27  caress
 * First cut at new version. Any changes are associated with
 * support of three data types (beam bathymetry, beam amplitude,
 * and sidescan) instead of two (bathymetry and backscatter).
 *
 * Revision 3.3  1993/05/27  23:37:16  caress
 * Fixed handling of -F option.
 *
 * Revision 3.2  1993/05/15  14:48:54  caress
 * removed excess rcs_id message
 *
 * Revision 3.1  1993/05/14  23:48:05  sohara
 * *** empty log message ***
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	char program_name[] = "MBDEFAULTS";
	char help_message[] = "MBDEFAULTS sets and retrieves the /default MBIO control \nparameters stored in the file ~/.mbio_defaults. \nOnly the parameters specified by command line \narguments will be changed; if no ~/.mbio_defaults \nfile exists one will be created.";
	char usage_message[] = "mbdefaults [-Dpsdisplay -Ffbtversion -Iimagedisplay -Llonflip\n\t-Ttimegap -Wproject -V -H]";
	extern char *optarg;
	int	errflg = 0;
	int	c;
	int	status;
	int	error = MB_ERROR_NO_ERROR;
	int	verbose = 0;
	int	help = 0;
	int	flag = 0;
	FILE	*fp;
	char	file[MB_PATH_MAXLINE];
	char	psdisplay[MB_PATH_MAXLINE];
	char	imgdisplay[MB_PATH_MAXLINE];
	char	mbproject[MB_PATH_MAXLINE];
	char	argstring[MB_PATH_MAXLINE];
	int	fbtversion = 3;
	int	uselockfiles = 1;
	char	*HOME = "HOME";
	char	*getenv();

	/* MBIO control parameters */
	int format;
	int pings;
	int lonflip;
	double bounds[4];
	int btime_i[7];
	int etime_i[7];
	double speedmin;
	double timegap;

	/* get current default mbio values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* now get current mb environment values */
	status = mb_env(verbose,psdisplay,imgdisplay,mbproject);

	/* now get current fbtversion value */
	status = mb_fbtversion(verbose,&fbtversion);

	/* now get current uselockfiles value */
	status = mb_uselockfiles(verbose,&uselockfiles);

	/* process argument list */
	while ((c = getopt(argc, argv, "D:d:F:f:HhI:i:L:l:T:t:U:u:VvW:w:")) != -1)
	  switch (c) 
		{
		case 'D':
		case 'd':
			sscanf (optarg,"%s",psdisplay);
			flag++;
			break;
		case 'F':
		case 'f':
			sscanf (optarg,"%s",argstring);
			if (strncmp(argstring,"new",3) == 0 || strncmp(argstring,"NEW",3) == 0)
				fbtversion = 3;
			else if (strncmp(argstring,"old",2) == 0 || strncmp(argstring,"OLD",2) == 0)
				fbtversion = 2;
			else if (strncmp(argstring,"2",1) == 0)
				fbtversion = 2;
			else if (strncmp(argstring,"3",1) == 0)
				fbtversion = 3;
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", imgdisplay);
			flag++;
			break;
		case 'H':
		case 'h':
			help++;
			break;
		case 'L':
		case 'l':
			sscanf (optarg,"%d", &lonflip);
			flag++;
			break;
		case 'T':
		case 't':
			sscanf (optarg,"%lf", &timegap);
			flag++;
			break;
		case 'U':
		case 'u':
			sscanf (optarg,"%s",argstring);
			if (strncmp(argstring,"yes",3) == 0 || strncmp(argstring,"YES",3) == 0)
				uselockfiles = 1;
			else if (strncmp(argstring,"no",2) == 0 || strncmp(argstring,"NO",2) == 0)
				uselockfiles = 0;
			else if (strncmp(argstring,"1",1) == 0)
				uselockfiles = 1;
			else if (strncmp(argstring,"0",1) == 0)
				uselockfiles = 0;
			flag++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case 'W':
		case 'w':
			sscanf (optarg,"%s", mbproject);
			flag++;
			break;
		case '?':
			errflg++;
		}

	/* if error flagged then print it and exit */
	if (errflg)
		{
		fprintf(stderr,"usage: %s\n", usage_message);
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
		fprintf(stderr,"dbg2       verbose:     %d\n",verbose);
		fprintf(stderr,"dbg2       help:        %d\n",help);
		fprintf(stderr,"dbg2       format:      %d\n",format);
		fprintf(stderr,"dbg2       pings:       %d\n",pings);
		fprintf(stderr,"dbg2       lonflip:     %d\n",lonflip);
		fprintf(stderr,"dbg2       bounds[0]:   %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:   %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:    %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:    %f\n",bounds[3]);
		fprintf(stderr,"dbg2       btime_i[0]:   %d\n",btime_i[0]);
		fprintf(stderr,"dbg2       btime_i[1]:   %d\n",btime_i[1]);
		fprintf(stderr,"dbg2       btime_i[2]:   %d\n",btime_i[2]);
		fprintf(stderr,"dbg2       btime_i[3]:   %d\n",btime_i[3]);
		fprintf(stderr,"dbg2       btime_i[4]:   %d\n",btime_i[4]);
		fprintf(stderr,"dbg2       btime_i[5]:   %d\n",btime_i[5]);
		fprintf(stderr,"dbg2       btime_i[6]:   %d\n",btime_i[6]);
		fprintf(stderr,"dbg2       etime_i[0]:   %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]:   %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]:   %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]:   %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]:   %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]:   %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       etime_i[6]:   %d\n",etime_i[6]);
		fprintf(stderr,"dbg2       speedmin:     %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:      %f\n",timegap);
		fprintf(stderr,"dbg2       psdisplay:    %s\n",psdisplay);
		fprintf(stderr,"dbg2       imgdisplay:   %s\n",imgdisplay);
		fprintf(stderr,"dbg2       mbproject:    %s\n",mbproject);
		fprintf(stderr,"dbg2       fbtversion:   %d\n",fbtversion);
		fprintf(stderr,"dbg2       uselockfiles: %d\n",uselockfiles);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* write out new ~/.mbio_defaults file if needed */
	if (flag)
		{
		strcpy(file,getenv(HOME));
		strcat(file,"/.mbio_defaults");
		if ((fp = fopen(file, "w")) == NULL)
			{
			fprintf (stderr, "Could not open file %s\n", file);
			error = MB_ERROR_OPEN_FAIL;
			exit(error);
			}
		fprintf(fp,"MBIO Default Control Parameters\n");
		fprintf(fp,"lonflip:    %d\n",lonflip);
		fprintf(fp,"timegap:    %f\n",timegap);
		fprintf(fp,"ps viewer:  %s\n",psdisplay);
		fprintf(fp,"img viewer: %s\n",imgdisplay);
		fprintf(fp,"project:    %s\n",mbproject);
		fprintf(fp,"fbtversion: %d\n",fbtversion);
		fprintf(fp,"uselockfiles:%d\n",uselockfiles);
		fclose(fp);

		printf("\nNew MBIO Default Control Parameters:\n");
		printf("lonflip:    %d\n",lonflip);
		printf("timegap:    %f\n",timegap);
		printf("ps viewer:  %s\n",psdisplay);
		printf("img viewer: %s\n",imgdisplay);
		printf("project:    %s\n",mbproject);
		if (fbtversion == 2)
			printf("fbtversion: 2 (old)\n");
		else if (fbtversion == 3)
			printf("fbtversion: 3 (new)\n");
		else
			printf("fbtversion: %d\n",fbtversion);
		printf("uselockfiles: %d\n",uselockfiles);
		}

	/* else just list the current defaults */
	else
		{
		printf("\nCurrent MBIO Default Control Parameters:\n");
		printf("lonflip:    %d\n",lonflip);
		printf("timegap:    %f\n",timegap);
		printf("ps viewer:  %s\n",psdisplay);
		printf("img viewer: %s\n",imgdisplay);
		printf("project:    %s\n",mbproject);
		if (fbtversion == 2)
			printf("fbtversion: 2 (old)\n");
		else if (fbtversion == 3)
			printf("fbtversion: 3 (new)\n");
		else
			printf("fbtversion: %d\n",fbtversion);
		printf("uselockfiles: %d\n",uselockfiles);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s> completed\n",program_name);
		fprintf(stderr,"dbg2  Ending status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/
