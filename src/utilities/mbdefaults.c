/*--------------------------------------------------------------------
 *    The MB-system:	mbdefaults.c	3.00	1/23/93
 *	$Id: mbdefaults.c,v 3.3 1993-05-27 23:37:16 caress Exp $
 *
 *    Copyright (c) 1993 by 
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
#include <strings.h>

/* mbio include files */
#include "../../include/mb_status.h"

/*--------------------------------------------------------------------*/

main (argc, argv)
int argc;
char **argv; 
{
static char rcs_id[]="$Id: mbdefaults.c,v 3.3 1993-05-27 23:37:16 caress Exp $";
	static char program_name[] = "MBDEFAULTS";
	static char help_message[] = "MBDEFAULTS sets and retrieves the default MBIO control \nparameters stored in the file ~/.mbio_defaults. \nOnly the parameters specified by command line \narguments will be changed; if no ~/.mbio_defaults \nfile exists one will be created.";
	static char usage_message[] = "mbdefaults [-Fformat -Rw/e/s/n -Ppings -Sspeed -Llonflip\n	-Byr/mo/da/hr/mn/sc -Eyr/mo/da/hr/mn/sc -V -H]";
	extern char *optarg;
	extern int optkind;
	int	errflg = 0;
	int	c;
	int	status;
	int	verbose = 0;
	int	help = 0;
	int	flag = 0;
	FILE	*fp;
	char	file[128];
	char	home[128];
	char	*HOME = "HOME";
	char	*getenv();

	/* MBIO control parameters */
	int format;
	int pings;
	int lonflip;
	double bounds[4];
	int btime_i[6];
	int etime_i[6];
	double speedmin;
	double timegap;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhF:f:P:p:L:l:R:r:B:b:E:e:S:s:T:t:")) != -1)
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
		case 'P':
		case 'p':
			sscanf (optarg,"%d", &pings);
			flag++;
			break;
		case 'L':
		case 'l':
			sscanf (optarg,"%d", &lonflip);
			flag++;
			break;
		case 'R':
		case 'r':
			sscanf (optarg,"%lf/%lf/%lf/%lf", 
				&bounds[0],&bounds[1],&bounds[2],&bounds[3]);
			flag++;
			break;
		case 'B':
		case 'b':
			sscanf (optarg,"%d/%d/%d/%d/%d/%d",
				&btime_i[0],&btime_i[1],&btime_i[2],
				&btime_i[3],&btime_i[4],&btime_i[5]);
			flag++;
			break;
		case 'E':
		case 'e':
			sscanf (optarg,"%d/%d/%d/%d/%d/%d",
				&etime_i[0],&etime_i[1],&etime_i[2],
				&etime_i[3],&etime_i[4],&etime_i[5]);
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
		fprintf(stderr,"dbg2       etime_i[0]: %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]: %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]: %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]: %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]: %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]: %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       speedmin:   %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:    %f\n",timegap);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(MB_SUCCESS);
		}

	/* write out new ~/.mbio_defaults file if needed */
	if (flag || status == MB_FAILURE)
		{
		strcpy(file,getenv(HOME));
		strcat(file,"/.mbio_defaults");
		if ((fp = fopen(file, "w")) == NULL)
			{
			fprintf (stderr, "Could not open file %s\n", file);
			exit(1);
			}
		fprintf(fp,"MBIO Default Control Parameters\n");
		fprintf(fp,"format:     %d\n",format);
		fprintf(fp,"pings:      %d\n",pings);
		fprintf(fp,"lonflip:    %d\n",lonflip);
		fprintf(fp,"speed:      %f\n",speedmin);
		fprintf(fp,"timegap:    %f\n",timegap);
		fprintf(fp,"bounds:     %f %f %f %f\n",
			bounds[0],bounds[1],bounds[2],bounds[3]);
		fprintf(fp,"begin time: %d %d %d %d %d %d\n",
			btime_i[0],btime_i[1],btime_i[2],
			btime_i[3],btime_i[4],btime_i[5]);
		fprintf(fp,"end time:   %d %d %d %d %d %d\n",
			etime_i[0],etime_i[1],etime_i[2],
			etime_i[3],etime_i[4],etime_i[5]);
		fclose(fp);

		printf("\nNew MBIO Default Control Parameters:\n");
		printf("format:   %d\n",format);
		printf("pings:    %d\n",pings);
		printf("lonflip:  %d\n",lonflip);
		printf("speed:    %f\n",speedmin);
		printf("timegap:  %f\n",timegap);
		printf("bounds: %f %f %f %f\n",
			bounds[0],bounds[1],bounds[2],bounds[3]);
		printf("begin time: %d %d %d %d %d %d\n",
			btime_i[0],btime_i[1],btime_i[2],
			btime_i[3],btime_i[4],btime_i[5]);
		printf("end time:   %d %d %d %d %d %d\n\n",
			etime_i[0],etime_i[1],etime_i[2],
			etime_i[3],etime_i[4],etime_i[5]);
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
		printf("begin time: %d %d %d %d %d %d\n",
			btime_i[0],btime_i[1],btime_i[2],
			btime_i[3],btime_i[4],btime_i[5]);
		printf("end time:   %d %d %d %d %d %d\n\n",
			etime_i[0],etime_i[1],etime_i[2],
			etime_i[3],etime_i[4],etime_i[5]);
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
	exit(status);
}
