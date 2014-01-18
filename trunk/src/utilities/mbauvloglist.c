/*--------------------------------------------------------------------
 *    The MB-system:	mbauvloglist.c	8/14/2006
 *    $Id$
 *
 *    Copyright (c) 2006-2013 by
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
 * MBauvloglist prints the specified contents of an MBARI AUV mission log
 * file to stdout. The form of the output is quite flexible;
 * MBsegylist is tailored to produce ascii files in spreadsheet
 * style with data columns separated by tabs.
 *
 * Author:	D. W. Caress
 * Date:	August 14, 2006
 * Location:	R/V Western Flyer hove to in a gale offshore British Columbia
 *
 * $Log: mbauvloglist.c,v $
 * Revision 5.1  2006/11/26 09:42:01  caress
 * Making distribution 5.1.0.
 *
 * Revision 5.0  2006/11/20 19:59:21  caress
 * Added program to CVS.
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* MBIO include files */
#include "mb_status.h"
#include "mb_define.h"

/* local defines */
#define	NFIELDSMAX	50
#define	MAX_OPTIONS	50
#define	TYPE_UNKNOWN	0
#define	TYPE_TIMETAG	1
#define	TYPE_INTEGER	2
#define	TYPE_DOUBLE	3
#define	TYPE_ANGLE	4

static char rcs_id[] = "$Id$";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	char program_name[] = "MBauvloglist";
	char help_message[] =  "MBauvloglist lists table data from an MBARI AUV mission log file.";
	char usage_message[] = "MBauvloglist -Ifile [-Fprintformat -Llonflip -Olist -H -V]";
	extern char *optarg;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* MBIO status variables */
	int	status = MB_SUCCESS;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;

	/* MBIO read control parameters */
	int	pings;
	int	format;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	speedmin;
	double	timegap;

	/* auv log data */
	FILE	*fp;
	char	file[MB_PATH_MAXLINE];
	struct	field
		{
		int	type;
		int	size;
		int	index;
		char	name[MB_PATH_MAXLINE];
		char	format[MB_PATH_MAXLINE];
		char	description[MB_PATH_MAXLINE];
		char	units[MB_PATH_MAXLINE];
		double	scale;
		};
	struct	printfield
		{
		char	name[MB_PATH_MAXLINE];
		int	index;
		int	formatset;
		char	format[MB_PATH_MAXLINE];
		};
	int	nfields = 0;
	struct field fields[NFIELDSMAX];
	int	nprintfields = 0;
	struct printfield printfields[NFIELDSMAX];
	int	nrecord;
	int	recordsize;
	int	printheader = MB_NO;
	int	angles_in_degrees = MB_NO;

	double	time_d;
	int	time_i[7];
	int	time_j[5];
	char	buffer[MB_PATH_MAXLINE];
	char	type[MB_PATH_MAXLINE];
	char	printformat[MB_PATH_MAXLINE];
	char	*result;
	int	nscan;
	double	dvalue;
	int	ivalue;
	int	index;
	int	i, j;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set file to null */
	file[0] = '\0';
	strcpy(printformat, "default");

	/* process argument list */
	while ((c = getopt(argc, argv, "F:f:I:i:L:l:O:o:PpSsVvWwHh")) != -1)
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
			sscanf (optarg,"%s", printformat);
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
		case 'O':
		case 'o':
			nscan = sscanf (optarg,"%s", printfields[nprintfields].name);
			if (strlen(printformat) > 0 && strcmp(printformat, "default") != 0)
				{
				printfields[nprintfields].formatset = MB_YES;
				strcpy(printfields[nprintfields].format,printformat);
				}
			else
				{
				printfields[nprintfields].formatset = MB_NO;
				strcpy(printfields[nprintfields].format,"");
				}
			printfields[nprintfields].index = -1;
			nprintfields++;
			flag++;
			break;
		case 'P':
		case 'p':
			printheader = MB_YES;
			flag++;
			break;
		case 'S':
		case 's':
			angles_in_degrees = MB_YES;
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
		fprintf(stderr,"dbg2       printheader:    %d\n",printheader);
		fprintf(stderr,"dbg2       angles_in_degrees:%d\n",angles_in_degrees);
		fprintf(stderr,"dbg2       nprintfields:   %d\n",nprintfields);
		for (i=0;i<nprintfields;i++)
			fprintf(stderr,"dbg2         printfields[%d]:      %s %d %s\n",
						i,printfields[i].name,
						printfields[i].formatset,
						printfields[i].format);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* open the input file */
	if ((fp = fopen(file, "r")) == NULL)
		{
		error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
		fprintf(stderr,"\nUnable to open log file <%s> for reading\n",file);
		exit(status);
		}

	nfields = 0;
	recordsize = 0;
	while ((result = fgets(buffer,MB_PATH_MAXLINE,fp)) == buffer
		&& strncmp(buffer, "# begin",7) != 0)
		{
		nscan = sscanf(buffer, "# %s %s %s",
				type,
				fields[nfields].name,
				fields[nfields].format);
		if (nscan == 2)
			{
			if (printheader == MB_YES)
				fprintf(stdout,"# csv %s\n",  fields[nfields].name);
			}

		else if (nscan == 3)
			{
			if (printheader == MB_YES)
				fprintf(stdout,"%s",buffer);

			result = (char *) strchr(buffer, ',');
			strcpy(fields[nfields].description, &(result[1]));
			result = (char *) strchr(fields[nfields].description, ',');
			result[0] = 0;
			result = (char *) strrchr(buffer, ',');
			strcpy(fields[nfields].units, &(result[1]));

			fields[nfields].index = recordsize;
			if (strcmp(type, "double") == 0)
				{
				fields[nfields].type = TYPE_DOUBLE;
				fields[nfields].size = 8;
				if (angles_in_degrees == MB_YES
					&&(strcmp(fields[nfields].name, "mLatK") == 0
						|| strcmp(fields[nfields].name, "mLonK") == 0
						|| strcmp(fields[nfields].name, "mLonK") == 0
						|| strcmp(fields[nfields].name, "mRollK") == 0
						|| strcmp(fields[nfields].name, "mPitchK") == 0
						|| strcmp(fields[nfields].name, "mHeadK") == 0))
					fields[nfields].scale = RTD;
				else
					fields[nfields].scale = 1.0;
				recordsize += 8;
				}
			else if (strcmp(type, "integer") == 0)
				{
				fields[nfields].type = TYPE_INTEGER;
				fields[nfields].size = 4;
				fields[nfields].scale = 1.0;
				recordsize += 4;
				}
			else if (strcmp(type, "timeTag") == 0)
				{
				fields[nfields].type = TYPE_TIMETAG;
				fields[nfields].size = 8;
				fields[nfields].scale = 1.0;
				recordsize += 8;
				}
			else if (strcmp(type, "angle") == 0)
				{
				fields[nfields].type = TYPE_ANGLE;
				fields[nfields].size = 8;
				if (angles_in_degrees == MB_YES
					&&(strcmp(fields[nfields].name, "mRollCB") == 0
						|| strcmp(fields[nfields].name, "mOmega_xCB") == 0
						|| strcmp(fields[nfields].name, "mPitchCB") == 0
						|| strcmp(fields[nfields].name, "mOmega_yCB") == 0
						|| strcmp(fields[nfields].name, "mYawCB") == 0
						|| strcmp(fields[nfields].name, "mOmega_zCB") == 0))
					fields[nfields].scale = RTD;
				else
					fields[nfields].scale = 1.0;
				recordsize += 8;
				}
			nfields++;
			}
		}

	/* end here if asked only to print header */
	if (nprintfields == 0 && printheader == MB_YES)
		exit(error);

	/* by default print everything */
	if (nprintfields == 0)
		{
		nprintfields = nfields;
		for (i=0;i<nfields;i++)
			{
			strcpy(printfields[i].name, fields[i].name);
			printfields[i].index = i;
			printfields[i].formatset = MB_NO;
			strcpy(printfields[i].format, fields[i].format);
			}
		}

	/* check the fields to be printed */
	for (i=0;i<nprintfields;i++)
		{
		if (strcmp(printfields[i].name,"zero") == 0)
			{
			printfields[i].index = -1;
			if (printfields[i].formatset == MB_NO)
				{
				strcpy(printfields[i].format, "%d");
				}
			}
		else
			{
			for (j=0;j<nfields;j++)
				{
				if (strcmp(printfields[i].name, fields[j].name) == 0)
					printfields[i].index = j;
				}
			if (printfields[i].formatset == MB_NO)
				{
				strcpy(printfields[i].format, fields[printfields[i].index].format);
				}
			}
		}

	/* if verbose print list of print field names */
	if (verbose > 0)
		{
		for (i=0;i<nprintfields;i++)
			{
			if (i == 0)
				fprintf(stdout, "# ");
			fprintf(stdout, "%s", printfields[i].name);
			if (i < nprintfields-1)
				fprintf(stdout, " | ");
			else
				fprintf(stdout, "\n");
			}
		}

	/* read the data records in the auv log file */
	nrecord = 0;
	while (fread(buffer, recordsize, 1, fp) == 1)
		{
		for (i=0;i<nprintfields;i++)
			{
			index = printfields[i].index;
			if (index == -1)
				{
				dvalue = 0.0;
				fprintf(stdout, printfields[i].format, dvalue);
				}
			else if (fields[index].type == TYPE_DOUBLE)
				{
				mb_get_binary_double(MB_YES, &buffer[fields[index].index], &dvalue);
				dvalue *= fields[index].scale;
				if (strcmp(fields[nfields].name, "mHeadK") == 0
					&& angles_in_degrees == MB_YES
					&& dvalue < 0.0)
					dvalue += 360.0;
				fprintf(stdout, printfields[i].format, dvalue);
				}
			else if (fields[index].type == TYPE_INTEGER)
				{
				mb_get_binary_int(MB_YES, &buffer[fields[index].index], &ivalue);
				fprintf(stdout, printfields[i].format, ivalue);
				}
			else if (fields[index].type == TYPE_TIMETAG)
				{
				mb_get_binary_double(MB_YES, &buffer[fields[index].index], &dvalue);
				if (strcmp(printfields[i].format, "time_i") == 0)
					{
					time_d = dvalue;
					mb_get_date(verbose,time_d,time_i);
					fprintf(stdout,"%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d",
						time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6]);
					}
				else if (strcmp(printfields[i].format, "time_j") == 0)
					{
					time_d = dvalue;
					mb_get_date(verbose,time_d,time_i);
					mb_get_jtime(verbose,time_i,time_j);
					fprintf(stdout,"%4.4d %3.3d %2.2d %2.2d %2.2d.%6.6d",
						time_i[0],time_j[1],time_i[3],time_i[4],time_i[5],time_i[6]);
					}
				else
					fprintf(stdout, printfields[i].format, dvalue);
				}
			else if (fields[index].type == TYPE_ANGLE)
				{
				mb_get_binary_double(MB_YES, &buffer[fields[index].index], &dvalue);
				dvalue *= fields[index].scale;
				if (strcmp(fields[nfields].name, "mYawCB") == 0
					&& angles_in_degrees == MB_YES
					&& dvalue < 0.0)
					dvalue += 360.0;
				fprintf(stdout, printfields[i].format, dvalue);
				}
			if (i < nprintfields - 1)
				fprintf(stdout, "\t");
			else
				fprintf(stdout, "\n");
			}
		nrecord++;
		}
	fclose(fp);

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
