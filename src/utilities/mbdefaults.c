/*--------------------------------------------------------------------
 *    The MB-system:	mbdefaults.c	1/23/93
 *	$Id: mbdefaults.c,v 4.4 1997-04-21 17:19:14 caress Exp $
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
 * MBDEFAULTS sets and retrieves the default MBIO control parameters 
 * stored in the file ~/.mbio_defaults.  Only the parameters specified
 * by command line arguments will be changed; if no ~/.mbio_defaults
 * file exists one will be created.
 *
 * Author:	D. W. Caress
 * Date:	January 23, 1993
 * $Log: not supported by cvs2svn $
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
#include <math.h>
#include <string.h>

/* mbio include files */
#include "../../include/mb_status.h"

/*--------------------------------------------------------------------*/

main (argc, argv)
int argc;
char **argv; 
{
static char rcs_id[]="$Id: mbdefaults.c,v 4.4 1997-04-21 17:19:14 caress Exp $";
	static char program_name[] = "MBDEFAULTS";
	static char help_message[] = "MBDEFAULTS sets and retrieves the /default MBIO control \nparameters stored in the file ~/.mbio_defaults. \nOnly the parameters specified by command line \narguments will be changed; if no ~/.mbio_defaults \nfile exists one will be created.";
	static char usage_message[] = "mbdefaults [-Dpsdisplay -Fformat -Rw/e/s/n -Ppings -Sspeed -Llonflip\n	-Byr/mo/da/hr/mn/sc -Eyr/mo/da/hr/mn/sc -Wproject -V -H]";
	extern char *optarg;
	extern int optkind;
	int	errflg = 0;
	int	c;
	int	status;
	int	error = MB_ERROR_NO_ERROR;
	int	verbose = 0;
	int	help = 0;
	int	flag = 0;
	FILE	*fp;
	char	file[128];
	char	home[128];
	char	psdisplay[128];
	char	mbproject[128];
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
	status = mb_env(verbose,psdisplay,mbproject);

	/* process argument list */
	while ((c = getopt(argc, argv, "B:b:D:d:E:e:F:f:HhL:l:P:p:R:r:S:s:T:t:VvW:w:")) != -1)
	  switch (c) 
		{
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
			sscanf (optarg,"%s",psdisplay);
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
		case 'H':
		case 'h':
			help++;
			break;
		case 'L':
		case 'l':
			sscanf (optarg,"%d", &lonflip);
			flag++;
			break;
		case 'P':
		case 'p':
			sscanf (optarg,"%d", &pings);
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
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       help:       %d\n",help);
		fprintf(stderr,"dbg2       format:     %d\n",format);
		fprintf(stderr,"dbg2       pings:      %d\n",pings);
		fprintf(stderr,"dbg2       lonflip:    %d\n",lonflip);
		fprintf(stderr,"dbg2       bounds[0]:  %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:  %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:  %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:  %f\n",bounds[3]);
		fprintf(stderr,"dbg2       btime_i[0]: %d\n",btime_i[0]);
		fprintf(stderr,"dbg2       btime_i[1]: %d\n",btime_i[1]);
		fprintf(stderr,"dbg2       btime_i[2]: %d\n",btime_i[2]);
		fprintf(stderr,"dbg2       btime_i[3]: %d\n",btime_i[3]);
		fprintf(stderr,"dbg2       btime_i[4]: %d\n",btime_i[4]);
		fprintf(stderr,"dbg2       btime_i[5]: %d\n",btime_i[5]);
		fprintf(stderr,"dbg2       btime_i[6]: %d\n",btime_i[6]);
		fprintf(stderr,"dbg2       etime_i[0]: %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]: %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]: %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]: %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]: %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]: %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       etime_i[6]: %d\n",etime_i[6]);
		fprintf(stderr,"dbg2       speedmin:   %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:    %f\n",timegap);
		fprintf(stderr,"dbg2       psdisplay:  %s\n",psdisplay);
		fprintf(stderr,"dbg2       mbproject:  %s\n",mbproject);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* write out new ~/.mbio_defaults file if needed */
	if (flag || status == MB_FAILURE)
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
		fprintf(fp,"format:     %d\n",format);
		fprintf(fp,"pings:      %d\n",pings);
		fprintf(fp,"lonflip:    %d\n",lonflip);
		fprintf(fp,"speed:      %f\n",speedmin);
		fprintf(fp,"timegap:    %f\n",timegap);
		fprintf(fp,"bounds:     %f %f %f %f\n",
			bounds[0],bounds[1],bounds[2],bounds[3]);
		fprintf(fp,"begin time: %d %d %d %d %d %d %d\n",
			btime_i[0],btime_i[1],btime_i[2],
			btime_i[3],btime_i[4],btime_i[5],btime_i[6]);
		fprintf(fp,"end time:   %d %d %d %d %d %d %d\n",
			etime_i[0],etime_i[1],etime_i[2],
			etime_i[3],etime_i[4],etime_i[5],etime_i[6]);
		fprintf(fp,"ps viewer:  %s\n",psdisplay);
		fprintf(fp,"project:    %s\n",mbproject);
		fclose(fp);

		printf("\nNew MBIO Default Control Parameters:\n");
		printf("format:   %d\n",format);
		printf("pings:    %d\n",pings);
		printf("lonflip:  %d\n",lonflip);
		printf("speed:    %f\n",speedmin);
		printf("timegap:  %f\n",timegap);
		printf("bounds: %f %f %f %f\n",
			bounds[0],bounds[1],bounds[2],bounds[3]);
		printf("begin time: %d %d %d %d %d %d %d\n",
			btime_i[0],btime_i[1],btime_i[2],
			btime_i[3],btime_i[4],btime_i[5],btime_i[6]);
		printf("end time:   %d %d %d %d %d %d %d\n",
			etime_i[0],etime_i[1],etime_i[2],
			etime_i[3],etime_i[4],etime_i[5],etime_i[6]);
		printf("ps viewer:  %s\n",psdisplay);
		printf("project:    %s\n",mbproject);
		}

	/* else just list the current defaults */
	else
		{
		printf("\nCurrent MBIO Default Control Parameters:\n");
		printf("format:   %d\n",format);
		printf("pings:    %d\n",pings);
		printf("lonflip:  %d\n",lonflip);
		printf("speed:    %f\n",speedmin);
		printf("timegap:  %f\n",timegap);
		printf("bounds: %f %f %f %f\n",
			bounds[0],bounds[1],bounds[2],bounds[3]);
		printf("begin time: %d %d %d %d %d %d %d\n",
			btime_i[0],btime_i[1],btime_i[2],
			btime_i[3],btime_i[4],btime_i[5],btime_i[6]);
		printf("end time:   %d %d %d %d %d %d %d\n",
			etime_i[0],etime_i[1],etime_i[2],
			etime_i[3],etime_i[4],etime_i[5],etime_i[6]);
		printf("ps viewer:  %s\n",psdisplay);
		printf("project:    %s\n",mbproject);
		}

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
