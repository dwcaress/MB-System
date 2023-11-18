/*--------------------------------------------------------------------
 *    The MB-system:	mbauvnavusbl.c	11/21/2004
 *
 *    Copyright (c) 2004-2023 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
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
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <getopt.h>
#include <unistd.h>

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_format.h"
#include "mb_status.h"

constexpr int NCHARMAX = 256;

/*--------------------------------------------------------------------*/

constexpr char program_name[] = "MBauvnavusbl";
constexpr char help_message[] =
    "MBauvnavusbl reads a primary navigation file (usually from a submerged platform\n"
    "swath survey) and also reads secondary navigation (e.g. USBL fixes).\n"
    "The program calculates position offsets between the raw survey navigation\n"
    "and the secondary navigation every 3600 seconds (10 minutes), and then\n"
    "linearly interpolates and applies this adjustment vector for each\n"
    "primary navigation position. The adjusted navigation is output.";
constexpr char usage_message[] =
    "mbauvnavusbl -Inavfile -Ooutfile -Uusblfile [-Fnavformat -Llonflip -Musblformat -V -H ]";

int main(int argc, char **argv) {
	int verbose = 0;
	// int status;

	/* get current default values - only interested in lonflip */
	int lonflip;
	{
		int format;
		int pings;
		double bounds[4];
		int btime_i[7];
		int etime_i[7];
		double speedmin;
		double timegap;
		/* status = */ mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i,
                                     etime_i, &speedmin, &timegap);
	}

	/* Files and formats */
	char ifile[MB_PATH_MAXLINE] = "stdin";
	char ofile[MB_PATH_MAXLINE] = "stdout";
	char ufile[MB_PATH_MAXLINE] = "";
	int navformat = 9;
	int usblformat = 165;

	bool useaverage = false;
	int error = MB_ERROR_NO_ERROR;

	/* process argument list */
	{
		bool errflg = false;
		int c;
		bool help = false;
		while ((c = getopt(argc, argv, "VvHhAaF:f:L:l:I:i:O:o:M:m:U:u:")) != -1)
		{
			switch (c) {
			case 'H':
			case 'h':
				help = true;
				break;
			case 'V':
			case 'v':
				verbose++;
				break;
			case 'A':
			case 'a':
				useaverage = true;
				break;
			case 'F':
			case 'f':
				sscanf(optarg, "%d", &navformat);
				break;
			case 'L':
			case 'l':
				sscanf(optarg, "%d", &lonflip);
				break;
			case 'I':
			case 'i':
				sscanf(optarg, "%1023s", ifile);
				break;
			case 'O':
			case 'o':
				sscanf(optarg, "%1023s", ofile);
				break;
			case 'M':
			case 'm':
				sscanf(optarg, "%d", &usblformat);
				break;
			case 'U':
			case 'u':
				sscanf(optarg, "%1023s", ufile);
				break;
			case '?':
				errflg = true;
			}
		}

		if (errflg) {
			fprintf(stderr, "usage: %s\n", usage_message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_BAD_USAGE);
		}

		if (verbose == 1 || help) {
			fprintf(stderr, "\nProgram %s\n", program_name);
			fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
		}

		if (verbose >= 2) {
			fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
			fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
			fprintf(stderr, "dbg2  Control Parameters:\n");
			fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
			fprintf(stderr, "dbg2       help:            %d\n", help);
			fprintf(stderr, "dbg2       lonflip:         %d\n", lonflip);
			fprintf(stderr, "dbg2       input file:      %s\n", ifile);
			fprintf(stderr, "dbg2       output file:     %s\n", ofile);
			fprintf(stderr, "dbg2       usbl file:       %s\n", ufile);
			fprintf(stderr, "dbg2       nav format:      %d\n", navformat);
			fprintf(stderr, "dbg2       usbl format:     %d\n", usblformat);
			fprintf(stderr, "dbg2       useaverage:      %d\n", useaverage);
		}

		if (help) {
			fprintf(stderr, "\n%s\n", help_message);
			fprintf(stderr, "\nusage: %s\n", usage_message);
			exit(error);
		}
	} // end command line arg parsing

	/* count the nav points */
	int nnav = 0;
	FILE *fp = fopen(ifile, "r");
	if (fp == nullptr) {
		fprintf(stderr, "\nUnable to Open Navigation File <%s> for reading\n", ifile);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(MB_ERROR_OPEN_FAIL);
	}
	char *result;
	char buffer[NCHARMAX];
	while ((result = fgets(buffer, NCHARMAX, fp)) == buffer)
		nnav++;
	fclose(fp);

	/* read and write values */
	int time_i[7];
	double navlon;
	double navlat;
	double heading;
	double sonardepth;

	/* navigation handling variables */
	double tieinterval = 600.0;
	double *ntime = nullptr;
	double *nlon = nullptr;
	double *nlat = nullptr;
	double *nheading = nullptr;
	double *nspeed = nullptr;
	double *nsonardepth = nullptr;
	double *nroll = nullptr;
	double *npitch = nullptr;
	double *nheave = nullptr;
	int nusbl;
	double *utime = nullptr;
	double *ulon = nullptr;
	double *ulat = nullptr;
	double *uheading = nullptr;
	double *usonardepth = nullptr;
	double *alon = nullptr;
	double *alat = nullptr;
	double *aheading = nullptr;
	double *asonardepth = nullptr;
	int ntie;
	double *ttime = nullptr;
	double *tlon = nullptr;
	double *tlat = nullptr;
	double *theading = nullptr;
	double *tsonardepth = nullptr;
	double loncoravg;
	double latcoravg;

	int nstime_i[7];
	int nftime_i[7];
	int ustime_i[7];
	int uftime_i[7];

	/* allocate space for the nav points */
	/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&ntime, &error);
	/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&nlon, &error);
	/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&nlat, &error);
	/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&nheading, &error);
	/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&nspeed, &error);
	/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&nsonardepth, &error);
	/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&nroll, &error);
	/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&npitch, &error);
	/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&nheave, &error);
	/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&alon, &error);
	/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&alat, &error);
	/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&aheading, &error);
	/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&asonardepth, &error);
	/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&ttime, &error);
	/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&tlon, &error);
	/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&tlat, &error);
	/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&theading, &error);
	/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&tsonardepth, &error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR) {
		char *message;
		mb_error(verbose, error, &message);
		fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* read in nav points */
	nnav = 0;
	if ((fp = fopen(ifile, "r")) == nullptr) {
		fprintf(stderr, "\nUnable to Open Navigation File <%s> for reading\n", ifile);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(MB_ERROR_OPEN_FAIL);
	}
	strncpy(buffer, "", sizeof(buffer));


	while ((result = fgets(buffer, NCHARMAX, fp)) == buffer) {
		bool nav_ok = false;
		double sec;

		int nget = sscanf(buffer, "%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &time_i[0], &time_i[1], &time_i[2],
		              &time_i[3], &time_i[4], &sec, &ntime[nnav], &nlon[nnav], &nlat[nnav], &nheading[nnav], &nspeed[nnav],
		              &nsonardepth[nnav], &nroll[nnav], &npitch[nnav], &nheave[nnav]);
		if (nget >= 12)
			nav_ok = true;

		/* make sure longitude is defined according to lonflip */
		if (nav_ok) {
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
		if (verbose >= 5 && nav_ok) {
			fprintf(stderr, "\ndbg5  New navigation point read in program <%s>\n", program_name);
			fprintf(stderr, "dbg5       nav[%d]: %f %f %f\n", nnav, ntime[nnav], nlon[nnav], nlat[nnav]);
		}
		else if (verbose >= 5) {
			fprintf(stderr, "\ndbg5  Error parsing line in navigation file in program <%s>\n", program_name);
			fprintf(stderr, "dbg5       line: %s\n", buffer);
		}

		/* check for reverses or repeats in time */
		if (nav_ok) {
			if (nnav == 0)
				nnav++;
			else if (ntime[nnav] > ntime[nnav - 1])
				nnav++;
			else if (nnav > 0 && ntime[nnav] <= ntime[nnav - 1] && verbose >= 5) {
				fprintf(stderr, "\ndbg5  Navigation time error in program <%s>\n", program_name);
				fprintf(stderr, "dbg5       nav[%d]: %f %f %f\n", nnav - 1, ntime[nnav - 1], nlon[nnav - 1], nlat[nnav - 1]);
				fprintf(stderr, "dbg5       nav[%d]: %f %f %f\n", nnav, ntime[nnav], nlon[nnav], nlat[nnav]);
			}
		}
		strncpy(buffer, "", sizeof(buffer));
	}
	fclose(fp);

	/* check for nav */
	if (nnav < 2) {
		fprintf(stderr, "\nNo navigation read from file <%s>\n", ifile);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* count the usbl points */
	nusbl = 0;
	if ((fp = fopen(ufile, "r")) == nullptr) {
		fprintf(stderr, "\nUnable to Open USBL Navigation File <%s> for reading\n", ufile);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(MB_ERROR_OPEN_FAIL);
	}
	while ((result = fgets(buffer, NCHARMAX, fp)) == buffer)
		nusbl++;
	fclose(fp);

	/* allocate space for the nav points */
	/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nusbl * sizeof(double), (void **)&utime, &error);
	/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nusbl * sizeof(double), (void **)&ulon, &error);
	/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nusbl * sizeof(double), (void **)&ulat, &error);
	/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nusbl * sizeof(double), (void **)&uheading, &error);
	/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nusbl * sizeof(double), (void **)&usonardepth, &error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR) {
		char *message;
		mb_error(verbose, error, &message);
		fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* read in usbl points */
	nusbl = 0;
	if ((fp = fopen(ufile, "r")) == nullptr) {
		fprintf(stderr, "\nUnable to Open USBL Navigation File <%s> for reading\n", ufile);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(MB_ERROR_OPEN_FAIL);
	}
	strncpy(buffer, "", sizeof(buffer));
	while ((result = fgets(buffer, NCHARMAX, fp)) == buffer) {
		bool nav_ok = false;
		int nget = 0;

		if (buffer[0] == '#') {
			// Ignore comments
		} else if (strchr(buffer, ',') != nullptr) {
			int year;
			int jday;
			double timetag;
			double easting;
			double northing;
			double rov_altitude;
			double rov_roll;
			double rov_pitch;
			int position_flag;
			int heading_flag;
			int altitude_flag;
			int attitude_flag;
			int pressure_flag;
			nget = sscanf(buffer, "%d,%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%d,%d,%d,%d,%d", &year, &jday, &timetag,
			              &utime[nusbl], &ulat[nusbl], &ulon[nusbl], &easting, &northing, &usonardepth[nusbl], &uheading[nusbl],
			              &rov_altitude, &rov_pitch, &rov_roll, &position_flag, &pressure_flag, &heading_flag, &altitude_flag,
			              &attitude_flag);
		} else {
			int year;
			int jday;
			double timetag;
			double easting;
			double northing;
			double rov_altitude;
			double rov_roll;
			double rov_pitch;
			int position_flag;
			int heading_flag;
			int altitude_flag;
			int attitude_flag;
			int pressure_flag;
			nget = sscanf(buffer, "%d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf,%d,%d,%d,%d,%d", &year, &jday, &timetag,
			              &utime[nusbl], &ulat[nusbl], &ulon[nusbl], &easting, &northing, &usonardepth[nusbl], &uheading[nusbl],
			              &rov_altitude, &rov_pitch, &rov_roll, &position_flag, &pressure_flag, &heading_flag, &altitude_flag,
			              &attitude_flag);
		}

		if (nget == 18) {
			nav_ok = true;
		}

		/* make sure longitude is defined according to lonflip */
		if (nav_ok) {
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
		if (verbose >= 5 && nav_ok) {
			fprintf(stderr, "\ndbg5  New USBL navigation point read in program <%s>\n", program_name);
			fprintf(stderr, "dbg5       usbl[%d]: %f %f %f\n", nusbl, utime[nusbl], ulon[nusbl], ulat[nusbl]);
		}
		else if (verbose >= 5) {
			fprintf(stderr, "\ndbg5  Error parsing line in navigation file in program <%s>\n", program_name);
			fprintf(stderr, "dbg5       line: %s\n", buffer);
		}

		/* check for reverses or repeats in time */
		if (nav_ok) {
			if (nusbl == 0)
				nusbl++;
			else if (utime[nusbl] > utime[nusbl - 1])
				nusbl++;
			else if (nusbl > 0 && utime[nusbl] <= utime[nusbl - 1] && verbose >= 5) {
				fprintf(stderr, "\ndbg5  USBL Navigation time error in program <%s>\n", program_name);
				fprintf(stderr, "dbg5       usbl[%d]: %f %f %f\n", nusbl - 1, utime[nusbl - 1], ulon[nusbl - 1], ulat[nusbl - 1]);
				fprintf(stderr, "dbg5       nav[%d]: %f %f %f\n", nusbl, utime[nusbl], ulon[nusbl], ulat[nusbl]);
			}
		}
		strncpy(buffer, "", sizeof(buffer));
	}

	fclose(fp);

	/* check for nav */
	if (nusbl < 2) {
		fprintf(stderr, "\nNo USBL navigation read from file <%s>\n", ufile);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* get start and finish times of nav */
	mb_get_date(verbose, ntime[0], nstime_i);
	mb_get_date(verbose, ntime[nnav - 1], nftime_i);
	mb_get_date(verbose, utime[0], ustime_i);
	mb_get_date(verbose, utime[nusbl - 1], uftime_i);

	/* give the statistics */
	if (verbose >= 1) {
		fprintf(stderr, "\n%d navigation records read\n", nnav);
		fprintf(stderr, "Nav start time: %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n", nstime_i[0], nstime_i[1], nstime_i[2],
		        nstime_i[3], nstime_i[4], nstime_i[5], nstime_i[6]);
		fprintf(stderr, "Nav end time:   %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n", nftime_i[0], nftime_i[1], nftime_i[2],
		        nftime_i[3], nftime_i[4], nftime_i[5], nftime_i[6]);
		fprintf(stderr, "\n%d USBL navigation records read\n", nusbl);
		fprintf(stderr, "Nav start time: %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n", ustime_i[0], ustime_i[1], ustime_i[2],
		        ustime_i[3], ustime_i[4], ustime_i[5], ustime_i[6]);
		fprintf(stderr, "Nav end time:   %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n", uftime_i[0], uftime_i[1], uftime_i[2],
		        uftime_i[3], uftime_i[4], uftime_i[5], uftime_i[6]);
	}

	/* now loop over nav data getting ties every tieinterval fixes */
	ntie = 0;
	loncoravg = 0.0;
	latcoravg = 0.0;
	for (int i = 0; i < nnav; i++) {
		if (ntie == 0 || (ntime[i] - ttime[ntie - 1]) > tieinterval) {
			/* get time */
			ttime[ntie] = ntime[i];

			/* interpolate navigation from usbl navigation */
			/* int intstat; */
			int j;
			/* intstat = */ mb_linear_interp(verbose, utime - 1, ulon - 1, nusbl, ttime[ntie], &navlon, &j, &error);
			/* intstat = */ mb_linear_interp(verbose, utime - 1, ulat - 1, nusbl, ttime[ntie], &navlat, &j, &error);
			/* intstat = */ mb_linear_interp(verbose, utime - 1, uheading - 1, nusbl, ttime[ntie], &heading, &j, &error);
			/* intstat = */ mb_linear_interp(verbose, utime - 1, usonardepth - 1, nusbl, ttime[ntie], &sonardepth, &j, &error);

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
			loncoravg += tlon[ntie - 1];
			latcoravg += tlat[ntie - 1];
		}
	}

	/* get averages */
	if (ntie > 0) {
		loncoravg /= ntie;
		latcoravg /= ntie;
	}

	fprintf(stderr, "\nCalculated %d adjustment points:\n", ntie);
	for (int i = 0; i < ntie; i++)
		fprintf(stderr, "time:%f lon:%f lat:%f heading:%f sonardepth:%f\n", ttime[i], tlon[i], tlat[i], theading[i],
		        tsonardepth[i]);
	fprintf(stderr, "Average lon:%f lat:%f\n", loncoravg, latcoravg);

	/* open output file */
	if ((fp = fopen(ofile, "w")) == nullptr) {
		fprintf(stderr, "\nUnable to Open Output Navigation File <%s> for writing\n", ofile);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(MB_ERROR_OPEN_FAIL);
	}

	/* now loop over nav data applying adjustments */
	for (int i = 0; i < nnav; i++) {
		/* interpolate adjustment */
		if (!useaverage) {
			/* get adjustment by interpolation */
			int j;
			/* intstat = */ mb_linear_interp(verbose, ttime - 1, tlon - 1, ntie, ntime[i], &navlon, &j, &error);
			/* intstat = */ mb_linear_interp(verbose, ttime - 1, tlat - 1, ntie, ntime[i], &navlat, &j, &error);

			/* apply adjustment */
			nlon[i] += navlon;
			nlat[i] += navlat;
		}

		/* else use average adjustments */
		else {
			/* apply adjustment */
			nlon[i] += loncoravg;
			nlat[i] += latcoravg;
		}

		/* write out the adjusted navigation */
		mb_get_date(verbose, ntime[i], time_i);
		snprintf(buffer, sizeof(buffer),
            "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.6f %.6f %.2f %.2f %.2f %.2f %.2f %.2f\n", time_i[0],
		        time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], ntime[i], nlon[i], nlat[i], nheading[i],
		        nspeed[i], nsonardepth[i], nroll[i], npitch[i], nheave[i]);
		if (fputs(buffer, fp) == EOF) {
			error = MB_ERROR_WRITE_FAIL;
			// status = MB_FAILURE;
		}
		else {
			error = MB_ERROR_NO_ERROR;
			// status = MB_SUCCESS;
		}
	}
	fclose(fp);

	/* deallocate memory for data arrays */
	mb_freed(verbose, __FILE__, __LINE__, (void **)&ntime, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&nlon, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&nlat, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&nheading, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&nspeed, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&nsonardepth, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&nroll, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&npitch, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&nheave, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&alon, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&alat, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&aheading, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&asonardepth, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&utime, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&ulon, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&ulat, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&uheading, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&usonardepth, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&ttime, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&tlon, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&tlat, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&theading, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&tsonardepth, &error);

	/* check memory */
	if (verbose >= 4)
		/* status = */ mb_memory_list(verbose, &error);

	/* give the statistics */
	if (verbose >= 1) {
		fprintf(stderr, "\n%d input navigation records\n", nnav);
		fprintf(stderr, "%d input usbl records\n", nusbl);
	}

	exit(error);
}
/*--------------------------------------------------------------------*/
