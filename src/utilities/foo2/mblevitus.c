/*--------------------------------------------------------------------
 *    The MB-system:	mblevitus.c	4/15/93
 *
 *    Copyright (c) 1993-2019 by
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
 * MBLEVITUS generates an average water velocity profile for a
 * specified location from the Levitus temperature and salinity
 * database.
 *
 * The calculation of water sound velocity from salinity and
 * temperature observations proceeds in two steps. The first
 * step is to calculate the pressure as a function of depth
 * and latitude. We use equations from a 1989 book by Coates:
 * *
 * The second step is to calculate the water sound velocity.
 * We use the DelGrosso equation because of the results presented in
 *    Dusha, Brian D. Worcester, Peter F., Cornuelle, Bruce D.,
 *      Howe, Bruce. M. "On equations for the speed of sound
 *      in seawater", J. Acoust. Soc. Am., Vol 93, No 1,
 *      January 1993, pp 255-275.
 *
 * Author:	D. W. Caress
 * Date:	April 15, 1993
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_status.h"

#define MBLEVITUS_NO_DATA -1000000000.0
#define NDEPTH_MAX 46
#define NLEVITUS_MAX 33

/* Windows header file */
#ifdef _WIN32
#include <windows.h>
#endif

/* Windows implementation of GMT_runtime_bindir */
#ifdef _WIN32
char *GMT_runtime_bindir_win32(char *result);
#endif

static const char program_name[] = "MBLEVITUS";
static const char help_message[] =
    "MBLEVITUS generates an average water velocity profile for a \nspecified location from the Levitus "
    "temperature and salinity database.";
static const char usage_message[] =
    "mblevitus [-Rlon/lat -Ooutfile -V -H]";

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	int errflg = 0;
	int c;
	int status = MB_SUCCESS;
	int verbose = 0;
	int help = 0;
	int flag = 0;
	int error = MB_ERROR_NO_ERROR;

/* input file set in include file */
#ifndef _WIN32
#include "levitus.h"
#else
	/* But on Windows get it from the bin dir */
	char *pch, levitusfile[PATH_MAX + 1];
#endif

	/* control parameters */
	char ofile[128];
	FILE *ifp, *ofp, *outfp;
	int record_size;
	long location;
	double longitude = 0.0;
	double latitude = 0.0;
	double lon_actual;
	double lat_actual;
	int ilon;
	int ilat;
	int nvelocity;
	int nvelocity_tot;
	float temperature[NLEVITUS_MAX][180];
	float salinity[NLEVITUS_MAX][180];
	float velocity[NDEPTH_MAX];
	static float
	    depth[NDEPTH_MAX] = {0.,    10.,   20.,   30.,   50.,   75.,    100.,   125.,   150.,   200.,  250.,  300.,
	                         400.,  500.,  600.,  700.,  800.,  900.,   1000.,  1100.,  1200.,  1300., 1400., 1500.,
	                         1750., 2000., 2500., 3000., 3500., 4000.,  4500.,  5000.,  5500.,  6000., 6500., 7000.,
	                         7500., 8000., 8500., 9000., 9500., 10000., 10500., 11000., 11500., 12000.}; /* 46 depth values */
	double pressure;
	double c0, dltact, dltacs, dltacp, dcstp;
	double zero = 0.0;

	/* time, user, host variables */
	time_t right_now;
	char date[32], user[128], *user_ptr, host[128];

	char *lonptr, *latptr;
	int last_good;

	char *ctime();
	char *getenv();

	/* set default output */
	strcpy(ofile, "velocity");

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhR:r:O:o:")) != -1)
		switch (c) {
		case 'H':
		case 'h':
			help++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case 'R':
		case 'r':
			lonptr = strtok(optarg, "/");
			latptr = strtok(NULL, "/");
			if (lonptr != NULL && latptr != NULL) {
				longitude = mb_ddmmss_to_degree(lonptr);
				latitude = mb_ddmmss_to_degree(latptr);
			}
			flag++;
			break;
		case 'O':
		case 'o':
			sscanf(optarg, "%s", ofile);
			flag++;
			break;
		case '?':
			errflg++;
		}

	/* if error flagged then print it and exit */
	if (errflg) {
		fprintf(stderr, "usage: %s\n", usage_message);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
	}

	/* set output stream */
	if (verbose <= 1)
		outfp = stdout;
	else
		outfp = stderr;

#ifdef _WIN32
	/* Find the path to the bin directory and from it, the location of the Levitus file */
	GMT_runtime_bindir_win32(levitusfile);
	pch = strrchr(levitusfile, '\\');
	pch[0] = '\0';
	strcat(levitusfile, "\\share\\mbsystem\\LevitusAnnual82.dat");
#endif

	if (verbose == 1 || help) {
		fprintf(outfp, "\nProgram %s\n", program_name);
		fprintf(outfp, "MB-system Version %s\n", MB_VERSION);
	}

	if (verbose >= 2) {
		fprintf(outfp, "\ndbg2  Program <%s>\n", program_name);
		fprintf(outfp, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(outfp, "dbg2  Control Parameters:\n");
		fprintf(outfp, "dbg2       verbose:          %d\n", verbose);
		fprintf(outfp, "dbg2       help:             %d\n", help);
		fprintf(outfp, "dbg2       levitusfile:      %s\n", levitusfile);
		fprintf(outfp, "dbg2       ofile:            %s\n", ofile);
		fprintf(outfp, "dbg2       longitude:        %f\n", longitude);
		fprintf(outfp, "dbg2       latitude:         %f\n", latitude);
	}

	/* if help desired then print it and exit */
	if (help) {
		fprintf(outfp, "\n%s\n", help_message);
		fprintf(outfp, "\nusage: %s\n", usage_message);
		exit(error);
	}

	/* open the data file */
	if ((ifp = fopen(levitusfile, "rb")) == NULL) {
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr, "\nUnable to Open Levitus database file <%s> for reading\n", levitusfile);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* check for sensible location */
	if (longitude < -360.0 || longitude > 360.0 || latitude < -90.0 || latitude > 90.0) {
		error = MB_ERROR_BAD_PARAMETER;
		fprintf(stderr, "\nInvalid location specified:  longitude: %f  latitude: %f\n", longitude, latitude);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* get the longitude and latitude indices */
	if (longitude < 0.0)
		ilon = (int)(longitude + 360.0);
	else if (longitude >= 360.0)
		ilon = (int)(longitude - 360.0);
	else
		ilon = (int)longitude;
	lon_actual = ilon + 0.5;
	ilat = (int)(latitude + 90.0);
	lat_actual = ilat - 89.5;
	fprintf(outfp, "\nLocation for mean annual water velocity profile:\n");
	fprintf(outfp, "  Requested:  %6.4f longitude   %6.4f latitude\n", longitude, latitude);
	fprintf(outfp, "  Used:       %6.4f longitude   %6.4f latitude\n", lon_actual, lat_actual);

	/* read the temperature */
	record_size = sizeof(float) * NLEVITUS_MAX * 180;
	location = ilon * record_size;
	status = fseek(ifp, location, 0);
	if ((status = fread(&temperature[0][0], 1, record_size, ifp)) == record_size) {
		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;
	}
	else {
		status = MB_FAILURE;
		error = MB_ERROR_EOF;
	}

	/* read the salinity */
	location = location + 360 * record_size;
	status = fseek(ifp, location, 0);
	if ((status = fread(&salinity[0][0], 1, record_size, ifp)) == record_size) {
		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;
	}
	else {
		status = MB_FAILURE;
		error = MB_ERROR_EOF;
	}

	/* close input file */
	fclose(ifp);

/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	for (int i = 0; i < NLEVITUS_MAX; i++) {
		mb_swap_float(&temperature[i][ilat]);
		mb_swap_float(&salinity[i][ilat]);
	}
#endif

	/* calculate velocity from temperature and salinity */
	nvelocity = 0;
	nvelocity_tot = 0;
	last_good = -1;
	for (int i = 0; i < NDEPTH_MAX; i++) {
		if (i < NLEVITUS_MAX)
			if (salinity[i][ilat] > MBLEVITUS_NO_DATA) {
				last_good = i;
				nvelocity++;
			}
		if (last_good >= 0) {
			/* set counter */
			nvelocity_tot++;

			/* get pressure for a given depth
			    as a function of latitude */
			pressure = 1.0052405 * depth[i] * (1.0 + 0.00528 * sin(DTR * latitude) * sin(DTR * latitude)) +
			           0.00000236 * depth[i] * depth[i];

			/* calculate water sound speed using
			    DelGrosso equations */
			/* convert decibar to kg/cm**2 */
			pressure = pressure * 0.1019716;
			c0 = 1402.392;
			dltact = temperature[last_good][ilat] *
			         (5.01109398873 +
			          temperature[last_good][ilat] * (-0.0550946843172 + temperature[last_good][ilat] * 0.000221535969240));
			dltacs = salinity[last_good][ilat] * (1.32952290781 + salinity[last_good][ilat] * 0.000128955756844);

			dltacp = pressure * (0.156059257041E0 + pressure * (0.000024499868841 + pressure * -0.00000000883392332513));
			dcstp = temperature[last_good][ilat] *
			            (-0.0127562783426 * salinity[last_good][ilat] +
			             pressure * (0.00635191613389 +
			                         pressure * (0.265484716608E-7 * temperature[last_good][ilat] - 0.00000159349479045 +
			                                     0.522116437235E-9 * pressure) -
			                         0.000000438031096213 * temperature[last_good][ilat] * temperature[last_good][ilat])) +
			        salinity[last_good][ilat] *
			            (-0.161674495909E-8 * salinity[last_good][ilat] * pressure * pressure +
			             temperature[last_good][ilat] *
			                 (0.0000968403156410 * temperature[last_good][ilat] +
			                  pressure * (0.00000485639620015 * salinity[last_good][ilat] - 0.000340597039004)));
			velocity[i] = c0 + dltact + dltacs + dltacp + dcstp;
		}
		else
			velocity[i] = salinity[i][ilat];
	}

	/* check for existence of water velocity profile */
	if (nvelocity < 1) {
		error = MB_ERROR_BAD_PARAMETER;
		fprintf(stderr, "\nNo water velocity profile available for specified location.\n");
		fprintf(stderr, "This place is probably subaerial!\n");
		fprintf(stderr, "No output file created.\n");
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	if ((ofp = fopen(ofile, "w")) == NULL) {
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr, "\nUnable to Open output file <%s> for writing\n", ofile);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	fprintf(ofp, "# Water velocity profile created by program %s\n", program_name);
	fprintf(ofp, "# MB-system Version %s\n", MB_VERSION);
	right_now = time((time_t *)0);
	strcpy(date, ctime(&right_now));
	date[strlen(date) - 1] = '\0';
	if ((user_ptr = getenv("USER")) == NULL)
		user_ptr = getenv("LOGNAME");
	if (user_ptr != NULL)
		strcpy(user, user_ptr);
	else
		strcpy(user, "unknown");
	gethostname(host, 128);
	fprintf(ofp, "# Run by user <%s> on cpu <%s> at <%s>\n", user, host, date);
	fprintf(ofp, "# Water velocity profile derived from Levitus\n");
	fprintf(ofp, "# temperature and salinity database.  This profile\n");
	fprintf(ofp, "# represents the annual average water velocity\n");
	fprintf(ofp, "# structure for a 1 degree X 1 degree area centered\n");
	fprintf(ofp, "# at %6.4f longitude and %6.4f latitude.\n", lon_actual, lat_actual);
	fprintf(ofp, "# This water velocity profile is in the form\n");
	fprintf(ofp, "# of discrete (depth, velocity) points where\n");
	fprintf(ofp, "# the depth is in meters and the velocity in\n");
	fprintf(ofp, "# meters/second.\n");
	fprintf(ofp, "# The first %d velocity values are defined using the\n", nvelocity);
	fprintf(ofp, "# salinity and temperature values available in the\n");
	fprintf(ofp, "# Levitus database; the remaining %d velocity values are\n", nvelocity_tot - nvelocity);
	fprintf(ofp, "# calculated using the deepest temperature\n");
	fprintf(ofp, "# and salinity value available.\n");

	for (int i = 0; i < nvelocity_tot; i++)
		fprintf(ofp, "%f %f\n", depth[i], velocity[i]);
	fprintf(outfp, "Values defined directly by Levitus database:      %2d\n", nvelocity);
	fprintf(outfp, "Values assuming deepest salinity and temperature: %2d\n", nvelocity_tot - nvelocity);
	fprintf(outfp, "Velocity points written:                          %2d\n", nvelocity_tot);
	fprintf(outfp, "Output file: %s\n", ofile);
	if (verbose >= 1) {
		fprintf(outfp, "\nMean annual water column profile:\n");
		fprintf(outfp, "     Depth Temperature Salinity   Velocity\n");
		for (int i = 0; i < nvelocity_tot; i++) {
			if (i < nvelocity)
				fprintf(outfp, "%10.4f %9.4f %9.4f   %9.4f\n", depth[i], temperature[i][ilat], salinity[i][ilat], velocity[i]);
			else
				fprintf(outfp, "%10.4f %9.4f %9.4f   %9.4f\n", depth[i], zero, zero, velocity[i]);
		}
	}

	/* close the output file */
	fclose(ofp);

	if (verbose >= 2) {
		fprintf(outfp, "\ndbg2  Program <%s> completed\n", program_name);
		fprintf(outfp, "dbg2  Ending status:\n");
		fprintf(outfp, "dbg2       status:  %d\n", status);
	}

	exit(error);
}

/*--------------------------------------------------------------------*/

/* Windows implementation of GMT_runtime_bindir */
#ifdef _WIN32
char *GMT_runtime_bindir_win32(char *result) {
	TCHAR path[PATH_MAX + 1];
	char *c;

	/* Get absolute path of executable */
	if (GetModuleFileName(NULL, path, PATH_MAX) == PATH_MAX)
		/* Path to long */
		return NULL;

/* Convert to cstring */
#ifdef _UNICODE
	/* TCHAR is wchar_t* */
	wcstombs(result, path, PATH_MAX);
#else
	/* TCHAR is char * */
	strncpy(result, path, PATH_MAX);
#endif

	/* Truncate full path to dirname */
	if ((c = strrchr(result, '\\')) && c != result)
		*c = '\0';

	return result;
}
#endif
/*--------------------------------------------------------------------*/
