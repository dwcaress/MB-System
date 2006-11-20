/*--------------------------------------------------------------------
 *    The MB-system:	mbauvloglist.c	8/14/2006
 *    $Id: mbauvloglist.c,v 5.0 2006-11-20 19:59:21 caress Exp $
 *
 *    Copyright (c) 2006 by
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
 * $Log: not supported by cvs2svn $
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
#include "../../include/mb_define.h"

/* local defines */
#define	NFIELDSMAX	50
#define	MAX_OPTIONS	50
#define	TYPE_UNKNOWN	0
#define	TYPE_TIMETAG	1
#define	TYPE_INTEGER	2
#define	TYPE_DOUBLE	3

static char rcs_id[] = "$Id: mbauvloglist.c,v 5.0 2006-11-20 19:59:21 caress Exp $";

/*--------------------------------------------------------------------*/

main (int argc, char **argv)
{
	static char program_name[] = "MBauvloglist";
	static char help_message[] =  "MBsegylist lists table data from an MBARI AUV mission log file.";
	static char usage_message[] = "MBauvloglist -Ifile [-A -Llonflip -Olist -H -V]";
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
	int	pings;
	int	format;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
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
	
	/* log variables */
	double time;
	double  mPos_x;
	double  mPos_y;
	double  mDepth;
	double  mGpsNorth;
	double  mGpsEast;
	int mGpsValid;
	double  mPhi;
	double  mTheta;
	double  mPsi;
	double  mOmega_x;
	double  mOmega_y;
	double  mOmega_z;
	double  mPsaRange;
	double  mAltitude;
	double  mDvlAltitude;
	double  mWaterSpeed;
	int mDvlValid;
	int mDvlNewData;
	double  mDeltaT;
	double  nfix;
	double  efix;
	double  filter_north;
	double  filter_east;
	double  filter_depth;
	double  north_current;
	double  east_current;
	double  speed_bias;
	double  heading_bias;
	double  latitude;
	double  longitude;

	int time_id = -1;
	int mPos_x_id = -1;
	int mPos_y_id = -1;
	int mDepth_id = -1;
	int mGpsNorth_id = -1;
	int mGpsEast_id = -1;
	int mGpsValid_id = -1;
	int mPhi_id = -1;
	int mTheta_id = -1;
	int mPsi_id = -1;
	int mOmega_x_id = -1;
	int mOmega_y_id = -1;
	int mOmega_z_id = -1;
	int mPsaRange_id = -1;
	int mAltitude_id = -1;
	int mDvlAltitude_id = -1;
	int mWaterSpeed_id = -1;
	int mDvlValid_id = -1;
	int mDvlNewData_id = -1;
	int mDeltaT_id = -1;
	int nfix_id = -1;
	int efix_id = -1;
	int filter_north_id = -1;
	int filter_east_id = -1;
	int filter_depth_id = -1;
	int north_current_id = -1;
	int east_current_id = -1;
	int speed_bias_id = -1;
	int heading_bias_id = -1;
	int latitude_id = -1;
	int longitude_id = -1;
	
	int conductivity_id = -1;
	int temperature_id = -1;
	int pressure_id = -1;
	int calculated_salinity_id = -1;
	int cond_frequency_id = -1;
	int temp_counts_id = -1;
	int pressure_counts_id = -1;
	int pressure_temp_comp_voltage_reading_id = -1;
	int calculated_sound_velocity_id = -1;

	int depth_id = -1;
	int temp_id = -1;
	int temp_period_id = -1;
	int pres_period_id = -1;

	char	buffer[MB_PATH_MAXLINE];
	char	type[MB_PATH_MAXLINE];
	char	printformat[MB_PATH_MAXLINE];
	char	*result;
	int	nscan;
	double	dvalue;
	int	ivalue;
	int	index;
	int	i;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set file to null */
	file[0] = '\0';
	printformat[0] = '\0';

	/* process argument list */
	while ((c = getopt(argc, argv, "F:f:I:i:L:l:O:o:VvWwHh")) != -1)
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
			if (strlen(printformat) > 0)
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
		status == MB_FAILURE;
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
				&(fields[nfields].name),  
				&(fields[nfields].format)); 
		if (nscan == 2)
			{
			if (verbose > 0)
				fprintf(stdout,"# csv %s\n",  fields[nfields].name);
			}
			
		else if (nscan == 3)
			{
			if (verbose > 0)
				fprintf(stdout,"%s",buffer);

			if (strcmp(fields[nfields].name, "time") == 0) time_id = nfields;
			if (strcmp(fields[nfields].name, "mPos_x") == 0) mPos_x_id = nfields;
			if (strcmp(fields[nfields].name, "mPos_y") == 0) mPos_y_id = nfields;
			if (strcmp(fields[nfields].name, "mDepth") == 0) mDepth_id = nfields;
			if (strcmp(fields[nfields].name, "mGpsNorth") == 0) mGpsNorth_id = nfields;
			if (strcmp(fields[nfields].name, "mGpsEast") == 0) mGpsEast_id = nfields;
			if (strcmp(fields[nfields].name, "mGpsValid") == 0) mGpsValid_id = nfields;
			if (strcmp(fields[nfields].name, "mPhi") == 0) mPhi_id = nfields;
			if (strcmp(fields[nfields].name, "mTheta") == 0) mTheta_id = nfields;
			if (strcmp(fields[nfields].name, "mPsi") == 0) mPsi_id = nfields;
			if (strcmp(fields[nfields].name, "mOmega_x") == 0) mOmega_x_id = nfields;
			if (strcmp(fields[nfields].name, "mOmega_y") == 0) mOmega_y_id = nfields;
			if (strcmp(fields[nfields].name, "mOmega_z") == 0) mOmega_z_id = nfields;
			if (strcmp(fields[nfields].name, "mPsaRange") == 0) mPsaRange_id = nfields;
			if (strcmp(fields[nfields].name, "mAltitude") == 0) mAltitude_id = nfields;
			if (strcmp(fields[nfields].name, "mDvlAltitude") == 0) mDvlAltitude_id = nfields;
			if (strcmp(fields[nfields].name, "mWaterSpeed") == 0) mWaterSpeed_id = nfields;
			if (strcmp(fields[nfields].name, "mDvlValid") == 0) mDvlValid_id = nfields;
			if (strcmp(fields[nfields].name, "mDvlNewData") == 0) mDvlNewData_id = nfields;
			if (strcmp(fields[nfields].name, "mDeltaT") == 0) mDeltaT_id = nfields;
			if (strcmp(fields[nfields].name, "nfix") == 0) nfix_id = nfields;
			if (strcmp(fields[nfields].name, "efix") == 0) efix_id = nfields;
			if (strcmp(fields[nfields].name, "filter_north") == 0) filter_north_id = nfields;
			if (strcmp(fields[nfields].name, "filter_east") == 0) filter_east_id = nfields;
			if (strcmp(fields[nfields].name, "filter_depth") == 0) filter_depth_id = nfields;
			if (strcmp(fields[nfields].name, "north_current") == 0) north_current_id = nfields;
			if (strcmp(fields[nfields].name, "east_current") == 0) east_current_id = nfields;
			if (strcmp(fields[nfields].name, "speed_bias") == 0) speed_bias_id = nfields;
			if (strcmp(fields[nfields].name, "heading_bias") == 0) heading_bias_id = nfields;
			if (strcmp(fields[nfields].name, "latitude") == 0) latitude_id = nfields;
			if (strcmp(fields[nfields].name, "longitude") == 0) longitude_id = nfields;

			if (strcmp(fields[nfields].name, "conductivity") == 0) conductivity_id = nfields;
			if (strcmp(fields[nfields].name, "temperature") == 0) temperature_id = nfields;
			if (strcmp(fields[nfields].name, "pressure") == 0) pressure_id = nfields;
			if (strcmp(fields[nfields].name, "calculated_salinity") == 0) calculated_salinity_id = nfields;
			if (strcmp(fields[nfields].name, "cond_frequency") == 0) cond_frequency_id = nfields;
			if (strcmp(fields[nfields].name, "temp_counts") == 0) temp_counts_id = nfields;
			if (strcmp(fields[nfields].name, "pressure_counts") == 0) pressure_counts_id = nfields;
			if (strcmp(fields[nfields].name, "pressure_temp_comp_voltage_reading") == 0) pressure_temp_comp_voltage_reading_id = nfields;
			if (strcmp(fields[nfields].name, "calculated_sound_velocity") == 0) calculated_sound_velocity_id = nfields;

			if (strcmp(fields[nfields].name, "depth") == 0) depth_id = nfields;
			if (strcmp(fields[nfields].name, "temp") == 0) temp_id = nfields;
			if (strcmp(fields[nfields].name, "pressure") == 0) pressure_id = nfields;
			if (strcmp(fields[nfields].name, "temp_period") == 0) temp_period_id = nfields;
			if (strcmp(fields[nfields].name, "pres_period") == 0) pres_period_id = nfields;

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
				recordsize += 8;
				}
			else if (strcmp(type, "integer") == 0)
				{
				fields[nfields].type = TYPE_INTEGER;
				fields[nfields].size = 4;
				recordsize += 4;
				}
			else if (strcmp(type, "timeTag") == 0)
				{
				fields[nfields].type = TYPE_TIMETAG;
				fields[nfields].size = 8;
				recordsize += 8;
				}
			nfields++;
			}
		}
		
	/* check the fields to be printed */
	for (i=0;i<nprintfields;i++)
		{	
		if (strcmp(printfields[i].name, "time") == 0) printfields[i].index = time_id;
		if (strcmp(printfields[i].name, "mPos_x") == 0) printfields[i].index = mPos_x_id;
		if (strcmp(printfields[i].name, "mPos_y") == 0) printfields[i].index = mPos_y_id;
		if (strcmp(printfields[i].name, "mDepth") == 0) printfields[i].index = mDepth_id;
		if (strcmp(printfields[i].name, "mGpsNorth") == 0) printfields[i].index = mGpsNorth_id;
		if (strcmp(printfields[i].name, "mGpsEast") == 0) printfields[i].index = mGpsEast_id;
		if (strcmp(printfields[i].name, "mGpsValid") == 0) printfields[i].index = mGpsValid_id;
		if (strcmp(printfields[i].name, "mPhi") == 0) printfields[i].index = mPhi_id;
		if (strcmp(printfields[i].name, "mTheta") == 0) printfields[i].index = mTheta_id;
		if (strcmp(printfields[i].name, "mPsi") == 0) printfields[i].index = mPsi_id;
		if (strcmp(printfields[i].name, "mOmega_x") == 0) printfields[i].index = mOmega_x_id;
		if (strcmp(printfields[i].name, "mOmega_y") == 0) printfields[i].index = mOmega_y_id;
		if (strcmp(printfields[i].name, "mOmega_z") == 0) printfields[i].index = mOmega_z_id;
		if (strcmp(printfields[i].name, "mPsaRange") == 0) printfields[i].index = mPsaRange_id;
		if (strcmp(printfields[i].name, "mAltitude") == 0) printfields[i].index = mAltitude_id;
		if (strcmp(printfields[i].name, "mDvlAltitude") == 0) printfields[i].index = mDvlAltitude_id;
		if (strcmp(printfields[i].name, "mWaterSpeed") == 0) printfields[i].index = mWaterSpeed_id;
		if (strcmp(printfields[i].name, "mDvlValid") == 0) printfields[i].index = mDvlValid_id;
		if (strcmp(printfields[i].name, "mDvlNewData") == 0) printfields[i].index = mDvlNewData_id;
		if (strcmp(printfields[i].name, "mDeltaT") == 0) printfields[i].index = mDeltaT_id;
		if (strcmp(printfields[i].name, "nfix") == 0) printfields[i].index = nfix_id;
		if (strcmp(printfields[i].name, "efix") == 0) printfields[i].index = efix_id;
		if (strcmp(printfields[i].name, "filter_north") == 0) printfields[i].index = filter_north_id;
		if (strcmp(printfields[i].name, "filter_east") == 0) printfields[i].index = filter_east_id;
		if (strcmp(printfields[i].name, "filter_depth") == 0) printfields[i].index = filter_depth_id;
		if (strcmp(printfields[i].name, "north_current") == 0) printfields[i].index = north_current_id;
		if (strcmp(printfields[i].name, "east_current") == 0) printfields[i].index = east_current_id;
		if (strcmp(printfields[i].name, "speed_bias") == 0) printfields[i].index = speed_bias_id;
		if (strcmp(printfields[i].name, "heading_bias") == 0) printfields[i].index = heading_bias_id;
		if (strcmp(printfields[i].name, "latitude") == 0) printfields[i].index = latitude_id;
		if (strcmp(printfields[i].name, "longitude") == 0) printfields[i].index = longitude_id;

		if (strcmp(printfields[i].name, "conductivity") == 0) printfields[i].index = conductivity_id;
		if (strcmp(printfields[i].name, "temperature") == 0) printfields[i].index = temperature_id;
		if (strcmp(printfields[i].name, "pressure") == 0) printfields[i].index = pressure_id;
		if (strcmp(printfields[i].name, "calculated_salinity") == 0) printfields[i].index = calculated_salinity_id;
		if (strcmp(printfields[i].name, "cond_frequency") == 0) printfields[i].index = cond_frequency_id;
		if (strcmp(printfields[i].name, "temp_counts") == 0) printfields[i].index = temp_counts_id;
		if (strcmp(printfields[i].name, "pressure_counts") == 0) printfields[i].index = pressure_counts_id;
		if (strcmp(printfields[i].name, "pressure_temp_comp_voltage_reading") == 0) printfields[i].index = pressure_temp_comp_voltage_reading_id;
		if (strcmp(printfields[i].name, "calculated_sound_velocity") == 0) printfields[i].index = calculated_sound_velocity_id;

		if (strcmp(printfields[i].name, "depth") == 0) printfields[i].index = depth_id;
		if (strcmp(printfields[i].name, "temp") == 0) printfields[i].index = temp_id;
		if (strcmp(printfields[i].name, "pressure") == 0) printfields[i].index = pressure_id;
		if (strcmp(printfields[i].name, "temp_period") == 0) printfields[i].index = temp_period_id;
		if (strcmp(printfields[i].name, "pres_period") == 0) printfields[i].index = pres_period_id;
		
		if (printfields[i].formatset == MB_NO)
			{
			strcpy(printfields[i].format, fields[printfields[i].index].format);
			}
		}
		
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
		
	/* print list of field names */
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
			if (fields[index].type == TYPE_DOUBLE)
				{
				mb_get_binary_double(MB_YES, &buffer[fields[index].index], &dvalue);	
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
