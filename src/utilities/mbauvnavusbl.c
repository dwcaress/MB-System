/*--------------------------------------------------------------------
 *    The MB-system:	mbauvnavusbl.c	11/21/2004
 *
 *    $Id$
 *
 *    Copyright (c) 2004-2014 by
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
 * MBauvnavusbl reads a primary navigation file (usually from a submerged platform
 * swath survey) and also reads secondary navigation (e.g. USBL fixes).
 * The program calculates position offsets between the raw survey navigation
 * and the secondary navigation every 3600 seconds (10 minutes), and then
 * linearly interpolates and applies this adjustment vector for each
 * primary navigation position. The adjusted navigation is output.
 *
 * Author:	D. W. Caress
 * Date:	November 21, 2004
 *
 * $Log: mbauvnavusbl.c,v $
 * Revision 5.3  2008/09/11 20:20:14  caress
 * Checking in updates made during cruise AT15-36.
 *
 * Revision 5.2  2006/01/18 15:17:00  caress
 * Added stdlib.h include.
 *
 * Revision 5.1  2004/12/18 01:38:52  caress
 * Working towards release 5.0.6.
 *
 * Revision 5.0  2004/12/02 06:41:47  caress
 * New program to help process ROV/AUV navigation data.
 *
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

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_aux.h"

/* local defines */
#define	NCHARMAX	256

static char rcs_id[] = "$Id$";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	/* id variables */
	char program_name[] = "MBauvnavusbl";
	char help_message[] = "MBauvnavusbl reads a primary navigation file (usually from a submerged platform\n swath survey) and also reads secondary navigation (e.g. USBL fixes).\n The program calculates position offsets between the raw survey navigation\n and the secondary navigation every 3600 seconds (10 minutes), and then\n linearly interpolates and applies this adjustment vector for each\n primary navigation position. The adjusted navigation is output.";
	char usage_message[] = "mbauvnavusbl -Inavfile -Ooutfile -Uusblfile [-Fnavformat -Llonflip -Musblformat -V -H ]";

	/* parsing variables */
	extern char *optarg;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* MBIO status variables */
	int	status = MB_SUCCESS;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;
	char	*message;

	/* Files and formats */
	char	ifile[MB_PATH_MAXLINE];
	char	ofile[MB_PATH_MAXLINE];
	char	ufile[MB_PATH_MAXLINE];
	int	navformat = 9;
	int	usblformat = 165;
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

	/* read and write values */
	int	time_i[7];
	double	navlon;
	double	navlat;
	double	heading;
	double	sonardepth;

	/* navigation handling variables */
	int	useaverage = MB_NO;
	double	tieinterval = 600.0;
	int	nnav;
	double	*ntime = NULL;
	double	*nlon = NULL;
	double	*nlat = NULL;
	double	*nheading = NULL;
	double	*nspeed = NULL;
	double	*nsonardepth = NULL;
	double	*nroll = NULL;
	double	*npitch = NULL;
	double	*nheave = NULL;
	int	nusbl;
	double	*utime = NULL;
	double	*ulon = NULL;
	double	*ulat = NULL;
	double	*uheading = NULL;
	double	*usonardepth = NULL;
	double	*alon = NULL;
	double	*alat = NULL;
	double	*aheading = NULL;
	double	*asonardepth = NULL;
	int	ntie;
	double	*ttime = NULL;
	double	*tlon = NULL;
	double	*tlat = NULL;
	double	*theading = NULL;
	double	*tsonardepth = NULL;
	double	loncoravg;
	double	latcoravg;

	int	nav_ok;
	int	nstime_i[7], nftime_i[7];
	int	ustime_i[7], uftime_i[7];

	char	buffer[NCHARMAX], *result;
	int	nget;
	int	year;
	int	jday;
	double	timetag;
	double	easting, northing;
	double	rov_altitude, rov_roll, rov_pitch;
	int	position_flag, heading_flag, altitude_flag, attitude_flag, pressure_flag;
	double	sec;
	int	intstat;
	int	i, j;

	/* get current default values - only interested in lonflip */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input and output */
	strcpy (ifile, "stdin");
	strcpy (ofile, "stdout");
	strcpy (ufile, "\0");

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhAaF:f:L:l:I:i:O:o:M:m:U:u:")) != -1)
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
		case 'A':
		case 'a':
			useaverage = MB_YES;
			flag++;
			break;
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &navformat);
			flag++;
			break;
		case 'L':
		case 'l':
			sscanf (optarg,"%d", &lonflip);
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", ifile);
			flag++;
			break;
		case 'O':
		case 'o':
			sscanf (optarg,"%s", ofile);
			flag++;
			break;
		case 'M':
		case 'm':
			sscanf (optarg,"%d", &usblformat);
			flag++;
			break;
		case 'U':
		case 'u':
			sscanf (optarg,"%s", ufile);
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
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       help:            %d\n",help);
		fprintf(stderr,"dbg2       lonflip:         %d\n",lonflip);
		fprintf(stderr,"dbg2       input file:      %s\n",ifile);
		fprintf(stderr,"dbg2       output file:     %s\n",ofile);
		fprintf(stderr,"dbg2       usbl file:       %s\n",ufile);
		fprintf(stderr,"dbg2       nav format:      %d\n",navformat);
		fprintf(stderr,"dbg2       usbl format:     %d\n",usblformat);
		fprintf(stderr,"dbg2       useaverage:      %d\n",useaverage);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* count the nav points */
	nnav = 0;
	if ((fp = fopen(ifile, "r")) == NULL)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to Open Navigation File <%s> for reading\n",ifile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	while ((result = fgets(buffer,NCHARMAX,fp)) == buffer)
		nnav++;
	fclose(fp);

	/* allocate space for the nav points */
	status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&ntime,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&nlon,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&nlat,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&nheading,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&nspeed,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&nsonardepth,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&nroll,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&npitch,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&nheave,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&alon,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&alat,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&aheading,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&asonardepth,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&ttime,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&tlon,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&tlat,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&theading,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&tsonardepth,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* read in nav points */
	nnav = 0;
	if ((fp = fopen(ifile, "r")) == NULL)
		{
		status = MB_FAILURE;
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to Open Navigation File <%s> for reading\n",ifile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	strncpy(buffer,"\0",sizeof(buffer));
	while ((result = fgets(buffer,NCHARMAX,fp)) == buffer)
		{
		nav_ok = MB_NO;

		nget = sscanf(buffer,"%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
			&time_i[0],&time_i[1],&time_i[2],
			&time_i[3],&time_i[4],&sec,
			&ntime[nnav],
			&nlon[nnav],&nlat[nnav],
			&nheading[nnav],&nspeed[nnav],
			&nsonardepth[nnav],
			&nroll[nnav],&npitch[nnav],&nheave[nnav]);
		if (nget >= 12)
			nav_ok = MB_YES;

		/* make sure longitude is defined according to lonflip */
		if (nav_ok == MB_YES)
			{
			if (lonflip == -1 && nlon[nnav] > 0.0)
				nlon[nnav] = nlon[nnav] - 360.0;
			else if (lonflip == 0 && nlon[nnav] < -180.0)
				nlon[nnav] = nlon[nnav] + 360.0;
			else if (lonflip == 0 && nlon[nnav] > 180.0)
				nlon[nnav] = nlon[nnav] - 360.0;
			else if (lonflip == 1 && nlon[nnav] < 0.0)
				nlon[nnav] = nlon[nnav] + 360.0;
			}

		/* output some debug values */
		if (verbose >= 5 && nav_ok == MB_YES)
			{
			fprintf(stderr,"\ndbg5  New navigation point read in program <%s>\n",program_name);
			fprintf(stderr,"dbg5       nav[%d]: %f %f %f\n",
				nnav,ntime[nnav],nlon[nnav],nlat[nnav]);
			}
		else if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  Error parsing line in navigation file in program <%s>\n",program_name);
			fprintf(stderr,"dbg5       line: %s\n",buffer);
			}

		/* check for reverses or repeats in time */
		if (nav_ok == MB_YES)
			{
			if (nnav == 0)
				nnav++;
			else if (ntime[nnav] > ntime[nnav-1])
				nnav++;
			else if (nnav > 0 && ntime[nnav] <= ntime[nnav-1]
				&& verbose >= 5)
				{
				fprintf(stderr,"\ndbg5  Navigation time error in program <%s>\n",program_name);
				fprintf(stderr,"dbg5       nav[%d]: %f %f %f\n",
					nnav-1,ntime[nnav-1],nlon[nnav-1],
					nlat[nnav-1]);
				fprintf(stderr,"dbg5       nav[%d]: %f %f %f\n",
					nnav,ntime[nnav],nlon[nnav],
					nlat[nnav]);
				}
			}
		strncpy(buffer,"\0",sizeof(buffer));
		}
	fclose(fp);

	/* check for nav */
	if (nnav < 2)
		{
		fprintf(stderr,"\nNo navigation read from file <%s>\n",ifile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* count the usbl points */
	nusbl = 0;
	if ((fp = fopen(ufile, "r")) == NULL)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to Open USBL Navigation File <%s> for reading\n",ufile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	while ((result = fgets(buffer,NCHARMAX,fp)) == buffer)
		nusbl++;
	fclose(fp);

	/* allocate space for the nav points */
	status = mb_mallocd(verbose,__FILE__,__LINE__,nusbl*sizeof(double),(void **)&utime,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,nusbl*sizeof(double),(void **)&ulon,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,nusbl*sizeof(double),(void **)&ulat,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,nusbl*sizeof(double),(void **)&uheading,&error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,nusbl*sizeof(double),(void **)&usonardepth,&error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* read in usbl points */
	nusbl = 0;
	if ((fp = fopen(ufile, "r")) == NULL)
		{
		status = MB_FAILURE;
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to Open USBL Navigation File <%s> for reading\n",ufile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	strncpy(buffer,"\0",sizeof(buffer));
	while ((result = fgets(buffer,NCHARMAX,fp)) == buffer)
		{
		nav_ok = MB_NO;

		/* ignore comments */
		if (buffer[0] == '#')
			{
			}
		else if (strchr(buffer, ',') != NULL)
			{
			nget = sscanf(buffer,
				"%d,%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%d,%d,%d,%d,%d",
				&year,
				&jday,
				&timetag,
				&utime[nusbl],
				&ulat[nusbl],
				&ulon[nusbl],
				&easting,
				&northing,
				&usonardepth[nusbl],
				&uheading[nusbl],
				&rov_altitude,
				&rov_pitch,
				&rov_roll,
				&position_flag,
				&pressure_flag,
				&heading_flag,
				&altitude_flag,
				&attitude_flag);
			}
		    else
			{
			nget = sscanf(buffer,
				"%d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf,%d,%d,%d,%d,%d",
				&year,
				&jday,
				&timetag,
				&utime[nusbl],
				&ulat[nusbl],
				&ulon[nusbl],
				&easting,
				&northing,
				&usonardepth[nusbl],
				&uheading[nusbl],
				&rov_altitude,
				&rov_pitch,
				&rov_roll,
				&position_flag,
				&pressure_flag,
				&heading_flag,
				&altitude_flag,
				&attitude_flag);
			}
		if (nget == 18)
			nav_ok = MB_YES;

		/* make sure longitude is defined according to lonflip */
		if (nav_ok == MB_YES)
			{
			if (lonflip == -1 && ulon[nusbl] > 0.0)
				ulon[nusbl] = ulon[nusbl] - 360.0;
			else if (lonflip == 0 && ulon[nusbl] < -180.0)
				ulon[nusbl] = ulon[nusbl] + 360.0;
			else if (lonflip == 0 && ulon[nusbl] > 180.0)
				ulon[nusbl] = ulon[nusbl] - 360.0;
			else if (lonflip == 1 && ulon[nusbl] < 0.0)
				ulon[nusbl] = ulon[nusbl] + 360.0;
			}

		/* output some debug values */
		if (verbose >= 5 && nav_ok == MB_YES)
			{
			fprintf(stderr,"\ndbg5  New USBL navigation point read in program <%s>\n",program_name);
			fprintf(stderr,"dbg5       usbl[%d]: %f %f %f\n",
				nusbl,utime[nusbl],ulon[nusbl],ulat[nusbl]);
			}
		else if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  Error parsing line in navigation file in program <%s>\n",program_name);
			fprintf(stderr,"dbg5       line: %s\n",buffer);
			}

		/* check for reverses or repeats in time */
		if (nav_ok == MB_YES)
			{
			if (nusbl == 0)
				nusbl++;
			else if (utime[nusbl] > utime[nusbl-1])
				nusbl++;
			else if (nusbl > 0 && utime[nusbl] <= utime[nusbl-1]
				&& verbose >= 5)
				{
				fprintf(stderr,"\ndbg5  USBL Navigation time error in program <%s>\n",program_name);
				fprintf(stderr,"dbg5       usbl[%d]: %f %f %f\n",
					nusbl-1,utime[nusbl-1],ulon[nusbl-1],
					ulat[nusbl-1]);
				fprintf(stderr,"dbg5       nav[%d]: %f %f %f\n",
					nusbl,utime[nusbl],ulon[nusbl],
					ulat[nusbl]);
				}
			}
		strncpy(buffer,"\0",sizeof(buffer));
		}
	fclose(fp);

	/* check for nav */
	if (nusbl < 2)
		{
		fprintf(stderr,"\nNo USBL navigation read from file <%s>\n",ufile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* get start and finish times of nav */
	mb_get_date(verbose,ntime[0],nstime_i);
	mb_get_date(verbose,ntime[nnav-1],nftime_i);
	mb_get_date(verbose,utime[0],ustime_i);
	mb_get_date(verbose,utime[nusbl-1],uftime_i);

	/* give the statistics */
	if (verbose >= 1)
		{
		fprintf(stderr,"\n%d navigation records read\n",nnav);
		fprintf(stderr,"Nav start time: %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
			nstime_i[0],nstime_i[1],nstime_i[2],nstime_i[3],
			nstime_i[4],nstime_i[5],nstime_i[6]);
		fprintf(stderr,"Nav end time:   %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
			nftime_i[0],nftime_i[1],nftime_i[2],nftime_i[3],
			nftime_i[4],nftime_i[5],nftime_i[6]);
		fprintf(stderr,"\n%d USBL navigation records read\n",nusbl);
		fprintf(stderr,"Nav start time: %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
			ustime_i[0],ustime_i[1],ustime_i[2],ustime_i[3],
			ustime_i[4],ustime_i[5],ustime_i[6]);
		fprintf(stderr,"Nav end time:   %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
			uftime_i[0],uftime_i[1],uftime_i[2],uftime_i[3],
			uftime_i[4],uftime_i[5],uftime_i[6]);
		}

	/* now loop over nav data getting ties every tieinterval fixes */
	ntie = 0;
	loncoravg = 0.0;
	latcoravg = 0.0;
	for (i=0;i<nnav;i++)
		{
		if (ntie == 0
			|| (ntime[i] - ttime[ntie-1]) > tieinterval)
			{
			/* get time */
			ttime[ntie] = ntime[i];

			/* interpolate navigation from usbl navigation */
			intstat = mb_linear_interp(verbose,
					utime-1, ulon-1,
					nusbl, ttime[ntie], &navlon, &j,
					&error);
			intstat = mb_linear_interp(verbose,
					utime-1, ulat-1,
					nusbl, ttime[ntie], &navlat, &j,
					&error);
			intstat = mb_linear_interp(verbose,
					utime-1, uheading-1,
					nusbl, ttime[ntie], &heading, &j,
					&error);
			intstat = mb_linear_interp(verbose,
					utime-1, usonardepth-1,
					nusbl, ttime[ntie], &sonardepth, &j,
					&error);

			/* get adjustments */
			tlon[ntie] = navlon - nlon[i];
			tlat[ntie] = navlat - nlat[i];
			theading[ntie] = heading - nheading[i];
			if (theading[ntie] < -180.0)
				theading[ntie] += 360.0;
			if (theading[ntie] > 180.0)
				theading[ntie] -= 360.0;
			tsonardepth[ntie] = sonardepth - nsonardepth[i];
			ntie++;

			/* get averages */
			loncoravg += tlon[ntie-1];
			latcoravg += tlat[ntie-1];
			}
		}

	/* get averages */
	if (ntie > 0)
		{
		loncoravg /= ntie;
		latcoravg /= ntie;
		}

fprintf(stderr,"\nCalculated %d adjustment points:\n",ntie);
for (i=0;i<ntie;i++)
fprintf(stderr,"time:%f lon:%f lat:%f heading:%f sonardepth:%f\n",
ttime[i],tlon[i],tlat[i],theading[i],tsonardepth[i]);
fprintf(stderr,"Average lon:%f lat:%f\n",loncoravg,latcoravg);

	/* open output file */
	if ((fp = fopen(ofile, "w")) == NULL)
		{
		status = MB_FAILURE;
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to Open Output Navigation File <%s> for writing\n",ofile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* now loop over nav data applying adjustments */
	for (i=0;i<nnav;i++)
		{
		/* interpolate adjustment */
		if (useaverage == MB_NO)
			{
			/* get adjustment by interpolation */
			intstat = mb_linear_interp(verbose,
					ttime-1, tlon-1,
					ntie, ntime[i], &navlon, &j,
					&error);
			intstat = mb_linear_interp(verbose,
					ttime-1, tlat-1,
					ntie, ntime[i], &navlat, &j,
					&error);

			/* apply adjustment */
			nlon[i] += navlon;
			nlat[i] += navlat;
			}

		/* else use average adjustments */
		else
			{
			/* apply adjustment */
			nlon[i] += loncoravg;
			nlat[i] += latcoravg;
			}

		/* write out the adjusted navigation */
		mb_get_date(verbose,ntime[i],time_i);
		sprintf(buffer,
			"%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.6f %.6f %.2f %.2f %.2f %.2f %.2f %.2f\n",
			time_i[0],
			time_i[1],
			time_i[2],
			time_i[3],
			time_i[4],
			time_i[5],
			time_i[6],
			ntime[i],
			nlon[i],
			nlat[i],
			nheading[i],
			nspeed[i],
			nsonardepth[i],
			nroll[i],
			npitch[i],
			nheave[i]);
		if (fputs(buffer, fp) == EOF)
			{
			error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		}
	fclose(fp);

	/* deallocate memory for data arrays */
	mb_freed(verbose,__FILE__,__LINE__,(void **)&ntime,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&nlon,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&nlat,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&nheading,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&nspeed,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&nsonardepth,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&nroll,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&npitch,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&nheave,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&alon,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&alat,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&aheading,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&asonardepth,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&utime,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&ulon,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&ulat,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&uheading,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&usonardepth,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&ttime,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&tlon,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&tlat,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&theading,&error);
	mb_freed(verbose,__FILE__,__LINE__,(void **)&tsonardepth,&error);

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* give the statistics */
	if (verbose >= 1)
		{
		fprintf(stderr,"\n%d input navigation records\n",nnav);
		fprintf(stderr,"%d input usbl records\n",nusbl);
		}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/
