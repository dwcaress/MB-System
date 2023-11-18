/*--------------------------------------------------------------------
 *    The MB-system:	mbinsreprocess.c	11/21/2004
 *
 *
 *    Copyright (c) 2014-2023 by
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

constexpr int NFIELDSMAX = 50;
constexpr int MAX_OPTIONS = 50;
typedef enum {
    TYPE_UNKNOWN = 0,
    TYPE_TIMETAG = 1,
    TYPE_INTEGER = 2,
    TYPE_DOUBLE = 3,
    TYPE_ANGLE = 4,
} field_type_t;
// TODO(schwehr): Should these be unsigned values for flags?
// constexpr int KEARFOTT_MONITOR_VALID_DVL = 0x01;
// constexpr int KEARFOTT_MONITOR_RESERVED = 0x02;
// constexpr int KEARFOTT_MONITOR_ZUPT_PROCESSED = 0x04;
// constexpr int KEARFOTT_MONITOR_DVL_REJECTED = 0x08;
constexpr int KEARFOTT_MONITOR_DVL_PPROCESSED = 0x10;
// constexpr int KEARFOTT_MONITOR_GPS_REJECTED = 0x20;
// constexpr int KEARFOTT_MONITOR_GPS_PROCESSED = 0x40;
// constexpr int KEARFOTT_MONITOR_DEPTH_LOOP_OPEN = 0x80;

/* auv log data */
struct field {
	field_type_t type;
	int size;
	int index;
	char name[MB_PATH_MAXLINE];
	char format[MB_PATH_MAXLINE];
	char description[MB_PATH_MAXLINE];
	char units[MB_PATH_MAXLINE];
	double scale;
};
struct printfield {
	char name[MB_PATH_MAXLINE];
	int index;
	// int formatset;
	char format[MB_PATH_MAXLINE];
};

constexpr char program_name[] = "MBinsreprocess";
constexpr char help_message[] =
    "MBinsreprocess reads an INS navigation file (e.g. from a Kearfott SeaDevil),\n"
    "including information about the state of navigation aiding by GPS, DVL,\n"
    "and other navigation sources. It then identifies time periods without\n"
    "aiding in which the navigation drifted in free inertial. These free\n"
    "inertial periods are typically ended with a navigation tear as the INS\n"
    "calculates a new state. This program removes the navigation tears by\n"
    "linear interpolation in time. The adjusted navigation is output.\n";
constexpr char usage_message[] =
    "mbinsreprocess --input=filename --output=filename [--help --verbose]";

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	int verbose = 0;
	/* MBIO default parameters - only use lonflip */
	int format;
	int pings;
	int lonflip;
	double bounds[4];
	int btime_i[7];
	int etime_i[7];
	double speedmin;
	double timegap;
	int status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

	char ifile[MB_PATH_MAXLINE] = "stdin";

	{
		const struct option options[] =
		    {{"verbose", no_argument, nullptr, 0},
		     {"help", no_argument, nullptr, 0},
		     {"verbose", no_argument, nullptr, 0},
		     {"input", required_argument, nullptr, 0},
		     {"output", required_argument, nullptr, 0},
		     {nullptr, 0, nullptr, 0}};

		bool errflg = false;
		int c;
		bool help = false;
		int option_index;
		char ofile[MB_PATH_MAXLINE] = "stdout";

		while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1)
		{
			switch (c) {
			/* long options all return c=0 */
			case 0:
				if (strcmp("verbose", options[option_index].name) == 0) {
					verbose++;
				}
				else if (strcmp("help", options[option_index].name) == 0) {
					help = true;
				}
				// Define input and output files
				else if (strcmp("input", options[option_index].name) == 0) {
					strcpy(ifile, optarg);
				}
				else if (strcmp("output", options[option_index].name) == 0) {
					strcpy(ofile, optarg);
				}

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
			fprintf(stderr, "dbg2  Default MB-System Parameters:\n");
			fprintf(stderr, "dbg2       verbose:                    %d\n", verbose);
			fprintf(stderr, "dbg2       help:                       %d\n", help);
			fprintf(stderr, "dbg2       lonflip:                    %d\n", lonflip);
			fprintf(stderr, "dbg2  Input and Output Files:\n");
			fprintf(stderr, "dbg2       ifile:                      %s\n", ifile);
			fprintf(stderr, "dbg2       ofile:                      %s\n", ofile);
		}

		if (help) {
			fprintf(stderr, "\n%s\n", help_message);
			fprintf(stderr, "\nusage: %s\n", usage_message);
			exit(MB_ERROR_NO_ERROR);
		}
	}

	/* open the input file */
	FILE *fp = fopen(ifile, "r");
	if (fp == nullptr) {
		fprintf(stderr, "\nUnable to open log file <%s> for reading\n", ifile);
		exit(status);
	}

	/* parse the ascii header listing the included data fields */
	struct field fields[NFIELDSMAX];
	int nfields = 0;
	size_t recordsize = 0;
	bool angles_in_degrees = true;
	char *result;
	char buffer[MB_PATH_MAXLINE];
	char type[MB_PATH_MAXLINE];
	while ((result = fgets(buffer, MB_PATH_MAXLINE, fp)) == buffer && strncmp(buffer, "# begin", 7) != 0) {
		const int nscan = sscanf(buffer, "# %1023s %1023s %1023s", type, fields[nfields].name, fields[nfields].format);
		if (nscan == 3) {
			result = (char *)strchr(buffer, ',');
			strcpy(fields[nfields].description, &(result[1]));
			result = (char *)strchr(fields[nfields].description, ',');
			result[0] = 0;
			result = (char *)strrchr(buffer, ',');
			strcpy(fields[nfields].units, &(result[1]));

			fields[nfields].index = recordsize;
			if (strcmp(type, "double") == 0) {
				fields[nfields].type = TYPE_DOUBLE;
				fields[nfields].size = 8;
				// TODO(schwehr): Was the second mLatK supposed to be something else?
				if (angles_in_degrees &&
				    (strcmp(fields[nfields].name, "mLatK") == 0 || strcmp(fields[nfields].name, "mLonK") == 0 ||
				     /* strcmp(fields[nfields].name, "mLatK") == 0 || */ strcmp(fields[nfields].name, "mRollK") == 0 ||
				     strcmp(fields[nfields].name, "mPitchK") == 0 || strcmp(fields[nfields].name, "mHeadK") == 0 ||
				     strcmp(fields[nfields].name, "mYawK") == 0 || strcmp(fields[nfields].name, "mLonCB") == 0 ||
				     strcmp(fields[nfields].name, "mLatCB") == 0 || strcmp(fields[nfields].name, "mRollCB") == 0 ||
				     strcmp(fields[nfields].name, "mPitchCB") == 0 || strcmp(fields[nfields].name, "mHeadCB") == 0 ||
				     strcmp(fields[nfields].name, "mYawCB") == 0))
					fields[nfields].scale = RTD;
				else
					fields[nfields].scale = 1.0;
				recordsize += 8;
			}
			else if (strcmp(type, "integer") == 0) {
				fields[nfields].type = TYPE_INTEGER;
				fields[nfields].size = 4;
				fields[nfields].scale = 1.0;
				recordsize += 4;
			}
			else if (strcmp(type, "timeTag") == 0) {
				fields[nfields].type = TYPE_TIMETAG;
				fields[nfields].size = 8;
				fields[nfields].scale = 1.0;
				recordsize += 8;
			}
			else if (strcmp(type, "angle") == 0) {
				fields[nfields].type = TYPE_ANGLE;
				fields[nfields].size = 8;
				if (angles_in_degrees &&
				    (strcmp(fields[nfields].name, "mRollCB") == 0 || strcmp(fields[nfields].name, "mOmega_xCB") == 0 ||
				     strcmp(fields[nfields].name, "mPitchCB") == 0 || strcmp(fields[nfields].name, "mOmega_yCB") == 0 ||
				     strcmp(fields[nfields].name, "mYawCB") == 0 || strcmp(fields[nfields].name, "mOmega_zCB") == 0))
					fields[nfields].scale = RTD;
				else
					fields[nfields].scale = 1.0;
				recordsize += 8;
			}
			nfields++;
		}
	}

	/* count the data records in the auv log file */
	int nrecord = 0;
	size_t fp_startpos = ftell(fp);
	while (fread(buffer, recordsize, 1, fp) == 1) {
		nrecord++;
	}
	fseek(fp, fp_startpos, SEEK_SET);

	int error = MB_ERROR_NO_ERROR;

	double *time = nullptr;
	int *mCyclesK = nullptr;
	int *mModeK = nullptr;
	int *mMonK = nullptr;
	double *mLatK = nullptr;
	double *mLonK = nullptr;
	double *mNorthK = nullptr;
	double *mEastK = nullptr;
	double *mDepthK = nullptr;
	double *mRollK = nullptr;
	double *mPitchK = nullptr;
	double *mHeadK = nullptr;
	double *mVbodyxK = nullptr;
	double *mVbodyyK = nullptr;
	double *mVbodyzK = nullptr;
	double *mAccelxK = nullptr;
	double *mAccelyK = nullptr;
	double *mAccelzK = nullptr;
	double *mPrateK = nullptr;
	double *mQrateK = nullptr;
	double *mRrateK = nullptr;
	double *utcTime = nullptr;

	if (nrecord > 0) {
		/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&time, &error);
		/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(int), (void **)&mCyclesK, &error);
		/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(int), (void **)&mModeK, &error);
		/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(int), (void **)&mMonK, &error);
		/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mLatK, &error);
		/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mLonK, &error);
		/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mNorthK, &error);
		/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mEastK, &error);
		/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mDepthK, &error);
		/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mRollK, &error);
		/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mPitchK, &error);
		/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mHeadK, &error);
		/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mVbodyxK, &error);
		/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mVbodyyK, &error);
		/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mVbodyzK, &error);
		/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mAccelxK, &error);
		/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mAccelyK, &error);
		/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mAccelzK, &error);
		/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mPrateK, &error);
		/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mQrateK, &error);
		/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&mRrateK, &error);
		/* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, nrecord * sizeof(double), (void **)&utcTime, &error);
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
		if (error != MB_ERROR_NO_ERROR) {
			char *message;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}
	}

	/* read the records */
	int irecord = 0;
	while (fread(buffer, recordsize, 1, fp) == 1) {
		/* loop over the fields in the record */
		for (int ifield = 0; ifield < nfields; ifield++) {
			if (fields[ifield].type == TYPE_DOUBLE) {
				double dvalue = 0.0;
				mb_get_binary_double(true, &buffer[fields[ifield].index], &dvalue);
				dvalue *= fields[ifield].scale;
				if ((strcmp(fields[ifield].name, "mHeadK") == 0 || strcmp(fields[ifield].name, "mYawK") == 0) &&
				    angles_in_degrees && dvalue < 0.0)
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
			else if (fields[ifield].type == TYPE_INTEGER) {
				int ivalue = 0;
				mb_get_binary_int(true, &buffer[fields[ifield].index], &ivalue);
				if (strcmp(fields[ifield].name, "mCyclesK") == 0)
					mCyclesK[irecord] = ivalue;
				if (strcmp(fields[ifield].name, "mModeK") == 0)
					mModeK[irecord] = ivalue;
				if (strcmp(fields[ifield].name, "mMonK") == 0)
					mMonK[irecord] = ivalue;
			}
			else if (fields[ifield].type == TYPE_TIMETAG) {
				double dvalue = 0.0;
				mb_get_binary_double(true, &buffer[fields[ifield].index], &dvalue);
				if (strcmp(fields[ifield].name, "time") == 0)
					time[irecord] = dvalue;
			}
			else if (fields[ifield].type == TYPE_ANGLE) {
				double dvalue = 0.0;
				mb_get_binary_double(true, &buffer[fields[ifield].index], &dvalue);
				dvalue *= fields[ifield].scale;
				if (strcmp(fields[ifield].name, "mYawCB") == 0 && angles_in_degrees && dvalue < 0.0)
					dvalue += 360.0;
			}
		}

		irecord++;
	}
	fclose(fp);

	/* output the data */
	double rr = 0.0;
	int time_i[7];
	for (irecord = 0; irecord < nrecord; irecord++) {
		if (irecord > 0) {
			const double dx = mEastK[irecord] - mEastK[irecord - 1];
			const double dy = mNorthK[irecord] - mNorthK[irecord - 1];
			rr = sqrt(dx * dx + dy * dy);
		}
		const char dvl_char =
			(mMonK[irecord] & KEARFOTT_MONITOR_DVL_PPROCESSED)
			? 'X' : ' ';
		const char jump_char = rr > 1.0 ? '*' : ' ';
		mb_get_date(verbose, time[irecord], time_i);
		fprintf(stderr,
		        "%7d %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %16.6f %14.9f %14.9f %10.3f %10.3f %7d %7d |   %c %10.3f "
		        "%c%c%c%c%c%c\n",
		        irecord, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], time[irecord],
		        mLonK[irecord], mLatK[irecord], mDepthK[irecord], mHeadK[irecord], mModeK[irecord], mMonK[irecord], dvl_char, rr,
		        jump_char, jump_char, jump_char, jump_char, jump_char, jump_char);
	}

	/* deallocate memory for data arrays */
	mb_freed(verbose, __FILE__, __LINE__, (void **)&time, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&mCyclesK, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&mModeK, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&mMonK, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&mLatK, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&mLonK, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&mNorthK, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&mEastK, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&mDepthK, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&mRollK, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&mPitchK, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&mHeadK, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&mVbodyxK, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&mVbodyyK, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&mVbodyzK, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&mAccelxK, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&mAccelyK, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&mAccelzK, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&mPrateK, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&mQrateK, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&mRrateK, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&utcTime, &error);

	if (verbose >= 4)
		status &= mb_memory_list(verbose, &error);

	if (verbose >= 1) {
		fprintf(stderr, "\n%d input ins records\n", nrecord);
	}

	if (status == MB_FAILURE) {
		fprintf(stderr, "WARNING: status is MB_FAILURE\n");
	}

	exit(error);
}
/*--------------------------------------------------------------------*/
