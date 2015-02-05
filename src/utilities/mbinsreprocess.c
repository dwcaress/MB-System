/*--------------------------------------------------------------------
 *    The MB-system:	mbinsreprocess.c	11/21/2004
 *
 *    $Id$
 *
 *    Copyright (c) 2014-2015 by
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
 * MBauvnavusbl reads an INS navigation file (e.g. from a Kearfott SeaDevil),
 * including information about the state of navigation aiding by GPS, DVL,
 * and other navigation sources. It then identifies time periods without
 * aiding in which the navigation drifted in free inertial. These free
 * inertial periods are typically ended with a navigation tear as the INS
 * calculates a new state. This program removes the navigation tears by
 * linear interpolation in time. The adjusted navigation is output.
 *
 * Author:	D. W. Caress
 * Date:	November 21, 2004
 *
 * command line option definitions:
 * mbsslayout 	--verbose
 * 		--help
 * 		--input=filename
 * 		--output=filename
 */	

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_aux.h"

/* local defines */
#define	NFIELDSMAX	50
#define	MAX_OPTIONS	50
#define	TYPE_UNKNOWN	0
#define	TYPE_TIMETAG	1
#define	TYPE_INTEGER	2
#define	TYPE_DOUBLE	3
#define	TYPE_ANGLE	4
#define	KEARFOTT_MONITOR_VALID_DVL		0x01
#define	KEARFOTT_MONITOR_RESERVED		0x02
#define	KEARFOTT_MONITOR_ZUPT_PROCESSED		0x04
#define	KEARFOTT_MONITOR_DVL_REJECTED		0x08
#define	KEARFOTT_MONITOR_DVL_PPROCESSED		0x10
#define	KEARFOTT_MONITOR_GPS_REJECTED		0x20
#define	KEARFOTT_MONITOR_GPS_PROCESSED		0x40
#define	KEARFOTT_MONITOR_DEPTH_LOOP_OPEN	0x80

static char version_id[] = "$Id$";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	/* id variables */
	char program_name[] = "MBinsreprocess";
	char help_message[] = "MBinsreprocess reads an INS navigation file (e.g. from a Kearfott SeaDevil), \nincluding information about the state of navigation aiding by GPS, DVL, \nand other navigation sources. It then identifies time periods without \naiding in which the navigation drifted in free inertial. These free \ninertial periods are typically ended with a navigation tear as the INS \ncalculates a new state. This program removes the navigation tears by \nlinear interpolation in time. The adjusted navigation is output.\n";
	char usage_message[] = "mbinsreprocess --input=filename --output=filename [--help --verbose]";
	
	static struct option options[] =
		{
		{"verbose",			no_argument, 		NULL, 		0},
		{"help",			no_argument, 		NULL, 		0},
		{"verbose",			no_argument, 		NULL, 		0},
		{"input",			required_argument, 	NULL, 		0},
		{"output",			required_argument, 	NULL, 		0},
		{NULL,				0, 			NULL, 		0}
		};
		

	/* parsing variables */
	extern char *optarg;
	int	option_index;
	int	errflg = 0;
	int	c;
	int	help = 0;

	/* MBIO status variables */
	int	status = MB_SUCCESS;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;
	char	*message;

	/* Files and formats */
	char	ifile[MB_PATH_MAXLINE];
	char	ofile[MB_PATH_MAXLINE];
	FILE	*fp;

	/* MBIO default parameters - only use lonflip */
	int	format;
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	speedmin;
	double	timegap;

	/* auv log data */
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

	/* read and write values */
	double	*time = NULL;
	int	*mCyclesK = NULL;
	int	*mModeK = NULL;
	int	*mMonK = NULL;
	double	*mLatK = NULL;
	double	*mLonK = NULL;
	double	*mNorthK = NULL;
	double	*mEastK = NULL;
	double	*mDepthK = NULL;
	double	*mRollK = NULL;
	double	*mPitchK = NULL;
	double	*mHeadK = NULL;
	double	*mVbodyxK = NULL;
	double	*mVbodyyK = NULL;
	double	*mVbodyzK = NULL;
	double	*mAccelxK = NULL;
	double	*mAccelyK = NULL;
	double	*mAccelzK = NULL;
	double	*mPrateK = NULL;
	double	*mQrateK = NULL;
	double	*mRrateK = NULL;
	double	*utcTime = NULL;
	
	int	angles_in_degrees = MB_YES;
	
	int	time_i[7];
	int	nrecord, irecord, nscan, ifield;
	size_t	recordsize = 0;
	size_t	fp_startpos = 0;
	char	*result;
	char	buffer[MB_PATH_MAXLINE];
	char	type[MB_PATH_MAXLINE];
	double	dvalue;
	int	ivalue;
	char	dvl_char, jump_char;
	double	dx, dy, rr;

	/* get current default values - only interested in lonflip */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input and output */
	strcpy (ifile, "stdin");
	strcpy (ofile, "stdout");

	/* process argument list */
	while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1)
	  switch (c)
		{
		/* long options all return c=0 */
		case 0:
			/* verbose */
			if (strcmp("verbose", options[option_index].name) == 0)
				{
				verbose++;
				}
			
			/* help */
			else if (strcmp("help", options[option_index].name) == 0)
				{
				help = MB_YES;
				}
				
			/*-------------------------------------------------------
			 * Define input and output files */
			
			/* input */
			else if (strcmp("input", options[option_index].name) == 0)
				{
				strcpy(ifile, optarg);
				}
			
			/* output */
			else if (strcmp("output", options[option_index].name) == 0)
				{
				strcpy(ofile, optarg);
				}
						
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
		fprintf(stderr,"Source File Version %s\n",version_id);
		fprintf(stderr,"MB-system Version %s\n",MB_VERSION);
		}

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s>\n",program_name);
		fprintf(stderr,"dbg2  Version %s\n",version_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Default MB-System Parameters:\n");
		fprintf(stderr,"dbg2       verbose:                    %d\n",verbose);
		fprintf(stderr,"dbg2       help:                       %d\n",help);
		fprintf(stderr,"dbg2       lonflip:                    %d\n",lonflip);
		fprintf(stderr,"dbg2  Input and Output Files:\n");
		fprintf(stderr,"dbg2       ifile:                      %s\n",ifile);
		fprintf(stderr,"dbg2       ofile:                      %s\n",ofile);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* count the number of records in the  */
		
	/* open the input file */
	if ((fp = fopen(ifile, "r")) == NULL)
		{
		error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
		fprintf(stderr,"\nUnable to open log file <%s> for reading\n",ifile);
		exit(status);
		}

	/* parse the ascii header listing the included data fields */
	nfields = 0;
	recordsize = 0;
	while ((result = fgets(buffer,MB_PATH_MAXLINE,fp)) == buffer
		&& strncmp(buffer, "# begin",7) != 0)
		{
		nscan = sscanf(buffer, "# %s %s %s",
				type,
				fields[nfields].name,
				fields[nfields].format);
		if (nscan == 3)
			{
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
						|| strcmp(fields[nfields].name, "mLatK") == 0
						|| strcmp(fields[nfields].name, "mRollK") == 0
						|| strcmp(fields[nfields].name, "mPitchK") == 0
						|| strcmp(fields[nfields].name, "mHeadK") == 0
						|| strcmp(fields[nfields].name, "mYawK") == 0
						|| strcmp(fields[nfields].name, "mLonCB") == 0
						|| strcmp(fields[nfields].name, "mLatCB") == 0
						|| strcmp(fields[nfields].name, "mRollCB") == 0
						|| strcmp(fields[nfields].name, "mPitchCB") == 0
						|| strcmp(fields[nfields].name, "mHeadCB") == 0
						|| strcmp(fields[nfields].name, "mYawCB") == 0))
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

	/* count the data records in the auv log file */
	nrecord = 0;
	fp_startpos = ftell(fp);
	while (fread(buffer, recordsize, 1, fp) == 1)
		{
		nrecord++;
		}
	fseek(fp, fp_startpos, SEEK_SET);

	/* allocate arrays */
	if (nrecord > 0)
		{
		status = mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&time, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(int), (void **)&mCyclesK, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(int), (void **)&mModeK, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(int), (void **)&mMonK, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mLatK, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mLonK, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mNorthK, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mEastK, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mDepthK, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mRollK, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mPitchK, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mHeadK, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mVbodyxK, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mVbodyyK, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mVbodyzK, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mAccelxK, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mAccelyK, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mAccelzK, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mPrateK, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mQrateK, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mRrateK, &error);
		status = mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&utcTime, &error);
		memset(time, 0, nrecord * sizeof(double));
		memset(mCyclesK, 0, nrecord * sizeof(int));
		memset(mModeK, 0, nrecord * sizeof(int));
		memset(mMonK, 0, nrecord * sizeof(int));
		memset(mLatK, 0, nrecord * sizeof(double));
		memset(mLonK, 0, nrecord * sizeof(double));
		memset(mNorthK, 0, nrecord * sizeof(double));
		memset(mEastK, 0, nrecord * sizeof(double));
		memset(mDepthK, 0, nrecord * sizeof(double));
		memset(mRollK, 0, nrecord * sizeof(double));
		memset(mPitchK, 0, nrecord * sizeof(double));
		memset(mHeadK, 0, nrecord * sizeof(double));
		memset(mVbodyxK, 0, nrecord * sizeof(double));
		memset(mVbodyyK, 0, nrecord * sizeof(double));
		memset(mVbodyzK, 0, nrecord * sizeof(double));
		memset(mAccelxK, 0, nrecord * sizeof(double));
		memset(mAccelyK, 0, nrecord * sizeof(double));
		memset(mAccelzK, 0, nrecord * sizeof(double));
		memset(mPrateK, 0, nrecord * sizeof(double));
		memset(mQrateK, 0, nrecord * sizeof(double));
		memset(mRrateK, 0, nrecord * sizeof(double));
		memset(utcTime, 0, nrecord * sizeof(double));

		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		}
    
	/* read the records */
	irecord = 0;
	while (fread(buffer, recordsize, 1, fp) == 1)
		{
		/* loop over the fields in the record */
		for (ifield=0;ifield<nfields;ifield++)
			{
			if (fields[ifield].type == TYPE_DOUBLE)
				{
				mb_get_binary_double(MB_YES, &buffer[fields[ifield].index], &dvalue);
//fprintf(stderr,"irecord:%d ifield:%d TYPE_DOUBLE name:%s dvalue:%f\n",
//	irecord, ifield, fields[ifield].name, dvalue);
				dvalue *= fields[ifield].scale;
				if ((strcmp(fields[ifield].name, "mHeadK") == 0
					|| strcmp(fields[ifield].name, "mYawK") == 0)
					&& angles_in_degrees == MB_YES
					&& dvalue < 0.0)
					dvalue += 360.0;
				if (strcmp(fields[ifield].name, "mLatK") == 0)
					mLatK[irecord] = dvalue;
				if (strcmp(fields[ifield].name, "mLonK") == 0)
					mLonK[irecord] = dvalue;
				if (strcmp(fields[ifield].name, "mNorthK") == 0)
					mNorthK[irecord] = dvalue;
				if (strcmp(fields[ifield].name, "mEastK") == 0)
					mEastK[irecord] = dvalue;
				if (strcmp(fields[ifield].name, "mDepthK") == 0)
					mDepthK[irecord] = dvalue;
				if (strcmp(fields[ifield].name, "mRollK") == 0)
					mRollK[irecord] = dvalue;
				if (strcmp(fields[ifield].name, "mPitchK") == 0)
					mPitchK[irecord] = dvalue;
				if (strcmp(fields[ifield].name, "mHeadK") == 0)
					mHeadK[irecord] = dvalue;
				if (strcmp(fields[ifield].name, "mVbodyxK") == 0)
					mVbodyxK[irecord] = dvalue;
				if (strcmp(fields[ifield].name, "mVbodyyK") == 0)
					mVbodyyK[irecord] = dvalue;
				if (strcmp(fields[ifield].name, "mVbodyzK") == 0)
					mVbodyzK[irecord] = dvalue;
				if (strcmp(fields[ifield].name, "mAccelxK") == 0)
					mAccelxK[irecord] = dvalue;
				if (strcmp(fields[ifield].name, "mAccelyK") == 0)
					mAccelyK[irecord] = dvalue;
				if (strcmp(fields[ifield].name, "mAccelzK") == 0)
					mAccelzK[irecord] = dvalue;
				if (strcmp(fields[ifield].name, "mPrateK") == 0)
					mPrateK[irecord] = dvalue;
				if (strcmp(fields[ifield].name, "mQrateK") == 0)
					mQrateK[irecord] = dvalue;
				if (strcmp(fields[ifield].name, "mRrateK") == 0)
					mRrateK[irecord] = dvalue;
				if (strcmp(fields[ifield].name, "utcTime") == 0)
					utcTime[irecord] = dvalue;
				}
			else if (fields[ifield].type == TYPE_INTEGER)
				{
				mb_get_binary_int(MB_YES, &buffer[fields[ifield].index], &ivalue);
//fprintf(stderr,"irecord:%d ifield:%d TYPE_INTEGER name:%s ivalue:%d\n",
//	irecord, ifield, fields[ifield].name, ivalue);
				if (strcmp(fields[ifield].name, "mCyclesK") == 0)
					mCyclesK[irecord] = ivalue;
				if (strcmp(fields[ifield].name, "mModeK") == 0)
					mModeK[irecord] = ivalue;
				if (strcmp(fields[ifield].name, "mMonK") == 0)
					mMonK[irecord] = ivalue;
				}
			else if (fields[ifield].type == TYPE_TIMETAG)
				{
				mb_get_binary_double(MB_YES, &buffer[fields[ifield].index], &dvalue);
//fprintf(stderr,"irecord:%d ifield:%d TYPE_TIMETAG name:%s dvalue:%f\n",
//	irecord, ifield, fields[ifield].name, dvalue);
				if (strcmp(fields[ifield].name, "time") == 0)
					time[irecord] = dvalue;
				}
			else if (fields[ifield].type == TYPE_ANGLE)
				{
				mb_get_binary_double(MB_YES, &buffer[fields[ifield].index], &dvalue);
				dvalue *= fields[ifield].scale;
				if (strcmp(fields[ifield].name, "mYawCB") == 0
					&& angles_in_degrees == MB_YES
					&& dvalue < 0.0)
					dvalue += 360.0;
				}
			}
		
//		fprintf(stderr,"%7d %16.6f %14.9f %14.9f %10.3f %10.3f %7d %7d\n",
//				irecord, time[irecord], mLonK[irecord], mLatK[irecord],
//				mDepthK[irecord], mHeadK[irecord], mModeK[irecord], mMonK[irecord]);
		/* increment record */
		irecord++;
		}
	fclose(fp);
	
	/* output the data */
	dx = 0.0;
	dy = 0.0;
	for (irecord=0;irecord<nrecord;irecord++)
		{
		if (irecord > 0)
			{
			dx = mEastK[irecord] - mEastK[irecord-1];
			dy = mNorthK[irecord] - mNorthK[irecord-1];
			rr = sqrt(dx * dx + dy * dy);
			}
		if (mMonK[irecord] & KEARFOTT_MONITOR_DVL_PPROCESSED)
			dvl_char = 'X';
		else
			dvl_char = ' ';
		if (rr > 1.0)
			jump_char = '*';
		else
			jump_char = ' ';
		mb_get_date(verbose, time[irecord], time_i);
		fprintf(stderr,"%7d %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %16.6f %14.9f %14.9f %10.3f %10.3f %7d %7d |   %c %10.3f %c%c%c%c%c%c\n",
				irecord, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
				time[irecord], mLonK[irecord], mLatK[irecord],
				mDepthK[irecord], mHeadK[irecord], mModeK[irecord], mMonK[irecord],
				dvl_char, rr, jump_char, jump_char, jump_char, jump_char, jump_char, jump_char);
		}

	/* deallocate memory for data arrays */
	mb_freed(verbose,__FILE__,__LINE__,(void **)&time, &error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&mCyclesK, &error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&mModeK, &error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&mMonK, &error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&mLatK, &error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&mLonK, &error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&mNorthK, &error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&mEastK, &error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&mDepthK, &error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&mRollK, &error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&mPitchK, &error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&mHeadK, &error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&mVbodyxK, &error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&mVbodyyK, &error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&mVbodyzK, &error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&mAccelxK, &error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&mAccelyK, &error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&mAccelzK, &error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&mPrateK, &error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&mQrateK, &error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&mRrateK, &error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&utcTime, &error);

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* give the statistics */
	if (verbose >= 1)
		{
		fprintf(stderr,"\n%d input ins records\n",nrecord);
		}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/
