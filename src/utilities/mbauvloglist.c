/*--------------------------------------------------------------------
 *    The MB-system:	mbauvloglist.c	8/14/2006
 *
 *    Copyright (c) 2006-2019 by
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
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_status.h"

#define NFIELDSMAX 512
#define MAX_OPTIONS 512
#define TYPE_UNKNOWN 0
#define TYPE_TIMETAG 1
#define TYPE_INTEGER 2
#define TYPE_DOUBLE 3
#define TYPE_ANGLE 4

#define INDEX_ZERO -1
#define INDEX_MERGE_LON -2
#define INDEX_MERGE_LAT -3
#define INDEX_MERGE_HEADING -4
#define INDEX_MERGE_SPEED -5
#define INDEX_MERGE_SENSORDEPTH -6
#define INDEX_MERGE_ROLL -7
#define INDEX_MERGE_PITCH -8
#define INDEX_MERGE_HEAVE -9
#define INDEX_CALC_CONDUCTIVITY -10
#define INDEX_CALC_TEMPERATURE -11
#define INDEX_CALC_PRESSURE -12
#define INDEX_CALC_SALINITY -13
#define INDEX_CALC_SOUNDSPEED -14
#define INDEX_CALC_POTENTIALTEMP -15
#define INDEX_CALC_DENSITY -16
#define INDEX_CALC_KTIME -17

#define OUTPUT_MODE_TAB 0
#define OUTPUT_MODE_CSV 1
#define OUTPUT_MODE_BINARY 2

struct ctd_calibration_struct {
		double pa0;
		double pa1;
		double pa2;
		double ptempa0;
		double ptempa1;
		double ptempa2;
		double ptca0;
		double ptca1;
		double ptca2;
		double ptcb0;
		double ptcb1;
		double ptcb2;
		
		double a0;
		double a1;
		double a2;
		double a3;
		
		double g;
		double h;
		double i;
		double j;
		double cpcor;
		double ctcor;
		};

static const char program_name[] = "MBauvloglist";
static const char help_message[] = "MBauvloglist lists table data from an MBARI AUV mission log file.";
static const char usage_message[] = "MBauvloglist -Ifile [-Fprintformat -Llonflip -Olist -Rid -S -H -V]";

/*--------------------------------------------------------------------*/

//returns pressure in dbar. Returned pressure is zero at surface, assuming
//atmospheric pressure fixed at 14.7PSI
double calcPressure(struct ctd_calibration_struct *calibration_ptr,
				double presCounts, double temperature)
{
  double t = calibration_ptr->ptempa0
								+ calibration_ptr->ptempa1*temperature
								+ calibration_ptr->ptempa2*temperature*temperature;
  double x = (double)presCounts - calibration_ptr->ptca0
								- calibration_ptr->ptca1*t
								- calibration_ptr->ptca2*t*t;
  double n = x*calibration_ptr->ptcb0 / (calibration_ptr->ptcb0
								+ calibration_ptr->ptcb1*t
								+ calibration_ptr->ptcb2*t*t);
  double pres = calibration_ptr->pa0
								+ calibration_ptr->pa1*n
								+ calibration_ptr->pa2*n*n;

  pres = (pres-14.7)*.6894757; //per note on page 34 of the SBE49 Manual
  return pres;
}

//return ITS90 temperature
double calcTemp(const struct ctd_calibration_struct *calibration_ptr,
				double tempCounts)
{
  const double mv = (double)((double)tempCounts - 524288.0) / (double)1.6e7;
  const double r  = (mv * (double)2.295e10 + (double)9.216e8)
                      / ((double)6.144e4 - mv*(double)5.3e5);
  const double ln_r = log(r);

  const double temp = 1. / ( calibration_ptr->a0 + calibration_ptr->a1*ln_r
					  + calibration_ptr->a2*ln_r*ln_r
					  + calibration_ptr->a3*ln_r*ln_r*ln_r)
                      - (double)273.15;
  return temp;
}

double calcCond(const struct ctd_calibration_struct *calibration_ptr,
				double cFreq, double temp, double pressure)
{

  // pressure *= 1.45; // Convert to psia for this formula

  cFreq /= 1000.0; 
  double cond = (calibration_ptr->g + calibration_ptr->h*cFreq*cFreq
				 + calibration_ptr->i*cFreq*cFreq*cFreq
				 + calibration_ptr->j*cFreq*cFreq*cFreq*cFreq)
                        / (1 + calibration_ptr->ctcor*temp
							+ calibration_ptr->cpcor*pressure);

  return cond;
}

int main(int argc, char **argv) {
	int errflg = 0;
	int c;
	int help = 0;
	int flag = 0;

	/* MBIO status variables */
	int verbose = 0;
	int error = MB_ERROR_NO_ERROR;
	char *message = NULL;

	/* MBIO read control parameters */
	int pings;
	int format;
	int lonflip;
	double bounds[4];
	int btime_i[7];
	int etime_i[7];
	double speedmin;
	double timegap;

	/* auv log data */
	FILE *fp;
	char file[MB_PATH_MAXLINE];
	struct field {
		int type;
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
		int formatset;
		char format[MB_PATH_MAXLINE];
	};
	int nfields = 0;
	struct field fields[NFIELDSMAX];
	int nprintfields = 0;
	struct printfield printfields[NFIELDSMAX];
	int nrecord;
	int recordsize;
	int printheader = MB_NO;
	int angles_in_degrees = MB_NO;

	/* navigation, heading, attitude data for merging in fnv format */
	int nav_merge = MB_NO;
	mb_path nav_file;
	int nav_num = 0;
	int nav_alloc = 0;
	double *nav_time_d = NULL;
	double *nav_navlon = NULL;
	double *nav_navlat = NULL;
	double *nav_heading = NULL;
	double *nav_speed = NULL;
	double *nav_sensordepth = NULL;
	double *nav_roll = NULL;
	double *nav_pitch = NULL;
	double *nav_heave = NULL;
    
    /* calculate time by adding time of date  to Kearfott time of day value */
    int calc_ktime = MB_NO;
    int ktime_available = MB_NO;
    double startofday_time_d = 0.0;
    double ktime_calc = 0.0;
    
    /* recalculate ctd data from fundamental observations */
    int recalculate_ctd = MB_NO;
    int ctd_calibration_id = 0;
    struct ctd_calibration_struct ctd_calibration;
    int cond_frequency_available = MB_NO;
    int temp_counts_available = MB_NO;
    int pressure_counts_available = MB_NO;
    int thermistor_available = MB_NO;
    int conductivity_available = MB_NO;
    int temperature_available = MB_NO;
    int pressure_available = MB_NO;
    int ctd_available = MB_NO;

	/* values used to calculate some water properties */
    int calc_potentialtemp = MB_NO;
    int calc_soundspeed = MB_NO;
    int calc_density = MB_NO;
	double cond_frequency = 0.0;
	double temp_counts = 0.0;
	double pressure_counts = 0.0;
    double thermistor = 0.0;
	double temperature_calc = 0.0;
	double salinity_calc = 0.0;
    double conductivity_calc = 0.0;
	double pressure_calc = 0.0;
    double soundspeed_calc = 0.0;
    double potentialtemperature_calc = 0.0;
    double density_calc = 0.0;

	/* output control */
	int output_mode = OUTPUT_MODE_TAB;

	double time_d = 0.0;
	int time_i[7];
	int time_j[5];
	char buffer[MB_PATH_MAXLINE];
	char type[MB_PATH_MAXLINE];
	char printformat[MB_PATH_MAXLINE];
	char *result;
	int nscan;
	double dvalue;
	double sec;
	int ivalue;
	int index;
	int jinterp = 0;

	/* get current default values */
	int status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

	/* set file to null */
	file[0] = '\0';
	nav_file[0] = '\0';
	strcpy(printformat, "default");

	/* process argument list */
	while ((c = getopt(argc, argv, "F:f:I:i:L:l:M:m:N:n:O:o:PpR:r:SsVvWwHh")) != -1)
		switch (c) {
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
			sscanf(optarg, "%s", printformat);
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf(optarg, "%s", file);
			flag++;
			break;
		case 'L':
		case 'l':
			sscanf(optarg, "%d", &lonflip);
			flag++;
			break;
		case 'M':
		case 'm':
			sscanf(optarg, "%d", &output_mode);
			flag++;
			break;
		case 'N':
		case 'n':
			sscanf(optarg, "%s", nav_file);
			nav_merge = MB_YES;
			flag++;
			break;
		case 'O':
		case 'o':
			nscan = sscanf(optarg, "%s", printfields[nprintfields].name);
			if (strlen(printformat) > 0 && strcmp(printformat, "default") != 0) {
				printfields[nprintfields].formatset = MB_YES;
				strcpy(printfields[nprintfields].format, printformat);
			}
			else {
				printfields[nprintfields].formatset = MB_NO;
				strcpy(printfields[nprintfields].format, "");
			}
            if (strcmp(printfields[nprintfields].name, "calcPotentialTemperature") == 0)
                calc_potentialtemp = MB_YES;
            if (strcmp(printfields[nprintfields].name, "calcSoundspeed") == 0)
                calc_soundspeed = MB_YES;
            if (strcmp(printfields[nprintfields].name, "calcDensity") == 0)
                calc_density = MB_YES;
            if (strcmp(printfields[nprintfields].name, "calcKTime") == 0)
                calc_ktime = MB_YES;
			printfields[nprintfields].index = -1;
			nprintfields++;
			flag++;
			break;
		case 'P':
		case 'p':
			printheader = MB_YES;
			flag++;
			break;
		case 'R':
		case 'r':
			recalculate_ctd = MB_YES;
			sscanf(optarg, "%d", &ctd_calibration_id);
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
	if (errflg) {
		fprintf(stderr, "usage: %s\n", usage_message);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
	}

	if (verbose == 1 || help) {
		fprintf(stderr, "\nProgram %s\n", program_name);
		fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Control Parameters:\n");
		fprintf(stderr, "dbg2       verbose:              %d\n", verbose);
		fprintf(stderr, "dbg2       help:                 %d\n", help);
		fprintf(stderr, "dbg2       lonflip:              %d\n", lonflip);
		fprintf(stderr, "dbg2       bounds[0]:            %f\n", bounds[0]);
		fprintf(stderr, "dbg2       bounds[1]:            %f\n", bounds[1]);
		fprintf(stderr, "dbg2       bounds[2]:            %f\n", bounds[2]);
		fprintf(stderr, "dbg2       bounds[3]:            %f\n", bounds[3]);
		fprintf(stderr, "dbg2       btime_i[0]:           %d\n", btime_i[0]);
		fprintf(stderr, "dbg2       btime_i[1]:           %d\n", btime_i[1]);
		fprintf(stderr, "dbg2       btime_i[2]:           %d\n", btime_i[2]);
		fprintf(stderr, "dbg2       btime_i[3]:           %d\n", btime_i[3]);
		fprintf(stderr, "dbg2       btime_i[4]:           %d\n", btime_i[4]);
		fprintf(stderr, "dbg2       btime_i[5]:           %d\n", btime_i[5]);
		fprintf(stderr, "dbg2       btime_i[6]:           %d\n", btime_i[6]);
		fprintf(stderr, "dbg2       etime_i[0]:           %d\n", etime_i[0]);
		fprintf(stderr, "dbg2       etime_i[1]:           %d\n", etime_i[1]);
		fprintf(stderr, "dbg2       etime_i[2]:           %d\n", etime_i[2]);
		fprintf(stderr, "dbg2       etime_i[3]:           %d\n", etime_i[3]);
		fprintf(stderr, "dbg2       etime_i[4]:           %d\n", etime_i[4]);
		fprintf(stderr, "dbg2       etime_i[5]:           %d\n", etime_i[5]);
		fprintf(stderr, "dbg2       etime_i[6]:           %d\n", etime_i[6]);
		fprintf(stderr, "dbg2       speedmin:             %f\n", speedmin);
		fprintf(stderr, "dbg2       timegap:              %f\n", timegap);
		fprintf(stderr, "dbg2       file:                 %s\n", file);
		fprintf(stderr, "dbg2       nav_file:             %s\n", nav_file);
		fprintf(stderr, "dbg2       output_mode:          %d\n", output_mode);
		fprintf(stderr, "dbg2       printheader:          %d\n", printheader);
		fprintf(stderr, "dbg2       angles_in_degrees:    %d\n", angles_in_degrees);
		fprintf(stderr, "dbg2       calc_potentialtemp:   %d\n", calc_potentialtemp);
		fprintf(stderr, "dbg2       recalculate_ctd:      %d\n", recalculate_ctd);
		fprintf(stderr, "dbg2       ctd_calibration_id:   %d\n", ctd_calibration_id);
		fprintf(stderr, "dbg2       calc_ktime:           %d\n", calc_ktime);
		fprintf(stderr, "dbg2       nprintfields:         %d\n", nprintfields);
		for (int i = 0; i < nprintfields; i++)
			fprintf(stderr, "dbg2         printfields[%d]:      %s %d %s\n", i, printfields[i].name, printfields[i].formatset,
			        printfields[i].format);
	}

	/* if help desired then print it and exit */
	if (help) {
		fprintf(stderr, "\n%s\n", help_message);
		fprintf(stderr, "\nusage: %s\n", usage_message);
		exit(error);
	}

	/* if nav merging to be done get nav */
	if (nav_merge == MB_YES && strlen(nav_file) > 0) {
		/* count the data points in the nav file */
		nav_num = 0;
		const int nchar = MB_PATH_MAXLINE - 1;
		if ((fp = fopen(nav_file, "r")) == NULL) {
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr, "\nUnable to Open Navigation File <%s> for reading\n", nav_file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}
		while ((result = fgets(buffer, nchar, fp)) == buffer)
			nav_num++;
		fclose(fp);

		/* allocate arrays for nav */
		if (nav_num > 0) {
			nav_alloc = nav_num;
			status = mb_mallocd(verbose, __FILE__, __LINE__, nav_alloc * sizeof(double), (void **)&nav_time_d, &error);
			status = mb_mallocd(verbose, __FILE__, __LINE__, nav_alloc * sizeof(double), (void **)&nav_navlon, &error);
			status = mb_mallocd(verbose, __FILE__, __LINE__, nav_alloc * sizeof(double), (void **)&nav_navlat, &error);
			status = mb_mallocd(verbose, __FILE__, __LINE__, nav_alloc * sizeof(double), (void **)&nav_heading, &error);
			status = mb_mallocd(verbose, __FILE__, __LINE__, nav_alloc * sizeof(double), (void **)&nav_speed, &error);
			status = mb_mallocd(verbose, __FILE__, __LINE__, nav_alloc * sizeof(double), (void **)&nav_sensordepth, &error);
			status = mb_mallocd(verbose, __FILE__, __LINE__, nav_alloc * sizeof(double), (void **)&nav_roll, &error);
			status = mb_mallocd(verbose, __FILE__, __LINE__, nav_alloc * sizeof(double), (void **)&nav_pitch, &error);
			status = mb_mallocd(verbose, __FILE__, __LINE__, nav_alloc * sizeof(double), (void **)&nav_heave, &error);

			/* if error initializing memory then quit */
			if (error != MB_ERROR_NO_ERROR) {
				mb_error(verbose, error, &message);
				fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
				fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
				exit(error);
			}
		}

		/* read the data points in the nav file */
		nav_num = 0;
		if ((fp = fopen(nav_file, "r")) == NULL) {
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr, "\nUnable to Open navigation File <%s> for reading\n", nav_file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}
		while ((result = fgets(buffer, nchar, fp)) == buffer) {
			const int nget = sscanf(
				buffer, "%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &time_i[0], &time_i[1], &time_i[2],
				&time_i[3], &time_i[4], &sec, &nav_time_d[nav_num], &nav_navlon[nav_num], &nav_navlat[nav_num],
				&nav_heading[nav_num], &nav_speed[nav_num], &nav_sensordepth[nav_num], &nav_roll[nav_num],
				&nav_pitch[nav_num], &nav_heave[nav_num]);
			int nav_ok;
			if (nget >= 9)
				nav_ok = MB_YES;
			else
				nav_ok = MB_NO;
			if (nav_num > 0 && nav_time_d[nav_num] <= nav_time_d[nav_num - 1])
				nav_ok = MB_NO;
			if (nav_ok == MB_YES)
				nav_num++;
		}
		fclose(fp);
	}
	if (nav_merge == MB_YES)
		fprintf(stderr, "%d %d records read from nav file %s\n", nav_alloc, nav_num, nav_file);

	/* open the input file */
	if ((fp = fopen(file, "r")) == NULL) {
		error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
		fprintf(stderr, "\nUnable to open log file <%s> for reading\n", file);
		exit(status);
	}

	nfields = 0;
	recordsize = 0;
	while ((result = fgets(buffer, MB_PATH_MAXLINE, fp)) == buffer &&
           strncmp(buffer, "# begin", 7) != 0) {
		nscan = sscanf(buffer, "# %s %s %s", type, fields[nfields].name, fields[nfields].format);
		if (nscan == 2) {
			if (printheader == MB_YES)
				fprintf(stdout, "# csv %s\n", fields[nfields].name);
		}

		else if (nscan == 3) {
			if (printheader == MB_YES)
				fprintf(stdout, "%s", buffer);

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
				/* TODO(schwehr): Is something missing? */
				if (angles_in_degrees == MB_YES &&
				    (/* strcmp(fields[nfields].name, "mLatK") == 0 || */ strcmp(fields[nfields].name, "mLonK") == 0 ||
				     strcmp(fields[nfields].name, "mLatK") == 0 || strcmp(fields[nfields].name, "mRollK") == 0 ||
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
				if (angles_in_degrees == MB_YES &&
				    (strcmp(fields[nfields].name, "mRollCB") == 0 || strcmp(fields[nfields].name, "mOmega_xCB") == 0 ||
                    strcmp(fields[nfields].name, "mPitchCB") == 0 || strcmp(fields[nfields].name, "mOmega_yCB") == 0 ||
				    strcmp(fields[nfields].name, "mYawCB") == 0 || strcmp(fields[nfields].name, "mOmega_zCB") == 0))
					fields[nfields].scale = RTD;
				else
					fields[nfields].scale = 1.0;
				recordsize += 8;
			}
            
            /* check if kearfott time is in this file */
            if (strcmp(fields[nfields].name, "utcTime") == 0)
                ktime_available = MB_YES;
            
            /* check if raw and processed ctd data are in this file */
            if (strcmp(fields[nfields].name, "cond_frequency") == 0)
                cond_frequency_available = MB_YES;
            else if (strcmp(fields[nfields].name, "temp_counts") == 0)
                temp_counts_available = MB_YES;
            else if (strcmp(fields[nfields].name, "pressure_counts") == 0)
                pressure_counts_available = MB_YES;
            else if (strcmp(fields[nfields].name, "pressure_temp_comp_voltage_reading") == 0)
                thermistor_available = MB_YES;
            else if (strcmp(fields[nfields].name, "conductivity") == 0)
                conductivity_available = MB_YES;
            else if (strcmp(fields[nfields].name, "temperature") == 0)
                temperature_available = MB_YES;
            else if (strcmp(fields[nfields].name, "pressure") == 0)
                pressure_available = MB_YES;
            
            /* increment counter */
			nfields++;
		}
	}

	/* end here if asked only to print header */
	if (nprintfields == 0 && printheader == MB_YES)
		exit(error);

	/* by default print everything */
	if (nprintfields == 0) {
		nprintfields = nfields;
		for (int i = 0; i < nfields; i++) {
			strcpy(printfields[i].name, fields[i].name);
			printfields[i].index = i;
			printfields[i].formatset = MB_NO;
			strcpy(printfields[i].format, fields[i].format);
		}
	}
    
    /* if recalculating CTD data then check for available raw CTD data */
    if (recalculate_ctd == MB_YES) {
        if (cond_frequency_available == MB_NO
            || temp_counts_available == MB_NO
            || pressure_counts_available == MB_NO
            || thermistor_available == MB_NO) {
            error = MB_ERROR_BAD_FORMAT;
		    status = MB_FAILURE;
		    fprintf(stderr, "\nUnable to recalculate CTD data as requested, raw CTD data not in file <%s>\n", file);
		    exit(status);
        } else {
            calibration_MAUV1_2017(&ctd_calibration);
        }
    }
    if (conductivity_available == MB_YES
        && temperature_available == MB_YES
        && pressure_available == MB_YES) {
        ctd_available = MB_YES;
    }
    if (calc_potentialtemp == MB_YES
        || calc_soundspeed == MB_YES
        || calc_density == MB_YES) {
        if (recalculate_ctd == MB_NO
            && ctd_available == MB_NO) {
                error = MB_ERROR_BAD_FORMAT;
                status = MB_FAILURE;
                fprintf(stderr, "\nUnable to calculate CTD data products as requested, CTD data not in file <%s>\n", file);
                exit(status);
        }
    }

	/* check the fields to be printed */
	for (int i = 0; i < nprintfields; i++) {
		if (strcmp(printfields[i].name, "zero") == 0) {
			printfields[i].index = INDEX_ZERO;
			if (printfields[i].formatset == MB_NO) {
				strcpy(printfields[i].format, "%f");
			}
		}
		else if (strcmp(printfields[i].name, "timeTag") == 0) {
			printfields[i].index = INDEX_ZERO;
			if (printfields[i].formatset == MB_NO) {
				strcpy(printfields[i].format, "%.8f");
			}
		}
		else if (strcmp(printfields[i].name, "mergeLon") == 0) {
			printfields[i].index = INDEX_MERGE_LON;
			if (printfields[i].formatset == MB_NO) {
				strcpy(printfields[i].format, "%.9f");
			}
		}
		else if (strcmp(printfields[i].name, "mergeLat") == 0) {
			printfields[i].index = INDEX_MERGE_LAT;
			if (printfields[i].formatset == MB_NO) {
				strcpy(printfields[i].format, "%.9f");
			}
		}
		else if (strcmp(printfields[i].name, "mergeHeading") == 0) {
			printfields[i].index = INDEX_MERGE_HEADING;
			if (printfields[i].formatset == MB_NO) {
				strcpy(printfields[i].format, "%.3f");
			}
		}
		else if (strcmp(printfields[i].name, "mergeSpeed") == 0) {
			printfields[i].index = INDEX_MERGE_SPEED;
			if (printfields[i].formatset == MB_NO) {
				strcpy(printfields[i].format, "%.3f");
			}
		}
		else if (strcmp(printfields[i].name, "mergeDraft") == 0) {
			printfields[i].index = INDEX_MERGE_SENSORDEPTH;
			if (printfields[i].formatset == MB_NO) {
				strcpy(printfields[i].format, "%.3f");
			}
		}
		else if (strcmp(printfields[i].name, "mergeSensordepth") == 0) {
			printfields[i].index = INDEX_MERGE_SENSORDEPTH;
			if (printfields[i].formatset == MB_NO) {
				strcpy(printfields[i].format, "%.3f");
			}
		}
		else if (strcmp(printfields[i].name, "mergeRoll") == 0) {
			printfields[i].index = INDEX_MERGE_ROLL;
			if (printfields[i].formatset == MB_NO) {
				strcpy(printfields[i].format, "%.3f");
			}
		}
		else if (strcmp(printfields[i].name, "mergePitch") == 0) {
			printfields[i].index = INDEX_MERGE_PITCH;
			if (printfields[i].formatset == MB_NO) {
				strcpy(printfields[i].format, "%.3f");
			}
		}
		else if (strcmp(printfields[i].name, "mergeHeave") == 0) {
			printfields[i].index = INDEX_MERGE_HEAVE;
			if (printfields[i].formatset == MB_NO) {
				strcpy(printfields[i].format, "%.3f");
			}
		}
		else if (strcmp(printfields[i].name, "calcConductivity") == 0) {
			printfields[i].index = INDEX_CALC_CONDUCTIVITY;
			if (printfields[i].formatset == MB_NO) {
				strcpy(printfields[i].format, "%.8f");
			}
		}
		else if (strcmp(printfields[i].name, "calcTemperature") == 0) {
			printfields[i].index = INDEX_CALC_TEMPERATURE;
			if (printfields[i].formatset == MB_NO) {
				strcpy(printfields[i].format, "%.8f");
			}
		}
		else if (strcmp(printfields[i].name, "calcPressure") == 0) {
			printfields[i].index = INDEX_CALC_PRESSURE;
			if (printfields[i].formatset == MB_NO) {
				strcpy(printfields[i].format, "%.8f");
			}
		}
		else if (strcmp(printfields[i].name, "calcSalinity") == 0) {
			printfields[i].index = INDEX_CALC_SALINITY;
			if (printfields[i].formatset == MB_NO) {
				strcpy(printfields[i].format, "%.8f");
			}
		}
		else if (strcmp(printfields[i].name, "calcSoundspeed") == 0) {
			printfields[i].index = INDEX_CALC_SOUNDSPEED;
			if (printfields[i].formatset == MB_NO) {
				strcpy(printfields[i].format, "%.8f");
			}
		}
		else if (strcmp(printfields[i].name, "CalcPotentialTemperature") == 0) {
			printfields[i].index = INDEX_CALC_POTENTIALTEMP;
			if (printfields[i].formatset == MB_NO) {
				strcpy(printfields[i].format, "%.8f");
			}
		}
		else if (strcmp(printfields[i].name, "calcDensity") == 0) {
			printfields[i].index = INDEX_CALC_DENSITY;
			if (printfields[i].formatset == MB_NO) {
				strcpy(printfields[i].format, "%.8f");
			}
		}
		else if (strcmp(printfields[i].name, "calcKTime") == 0) {
			printfields[i].index = INDEX_CALC_KTIME;
			if (printfields[i].formatset == MB_NO) {
				strcpy(printfields[i].format, "%.8f");
			}
		}
		else {
			for (int j = 0; j < nfields; j++) {
				if (strcmp(printfields[i].name, fields[j].name) == 0)
					printfields[i].index = j;
			}
			if (printfields[i].formatset == MB_NO) {
				strcpy(printfields[i].format, fields[printfields[i].index].format);
			}
		}
	}

	/* if verbose print list of print field names */
	if (verbose > 0) {
		for (int i = 0; i < nprintfields; i++) {
			if (i == 0)
				fprintf(stdout, "# ");
			fprintf(stdout, "%s", printfields[i].name);
			if (i < nprintfields - 1)
				fprintf(stdout, " | ");
			else
				fprintf(stdout, "\n");
		}
	}

	/* int interp_status;  TODO(schwehr): Remove or test. */

	/* read the data records in the auv log file */
	nrecord = 0;
	while (fread(buffer, recordsize, 1, fp) == 1) {
        /* recalculate CTD data if requested */
        if (recalculate_ctd == MB_YES) {
            for (int ii = 0; ii < nfields; ii++) {
                if (strcmp(fields[ii].name, "cond_frequency") == 0)
                    mb_get_binary_double(MB_YES, &buffer[fields[ii].index], &cond_frequency);
                else if (strcmp(fields[ii].name, "temp_counts") == 0)
                    mb_get_binary_double(MB_YES, &buffer[fields[ii].index], &temp_counts);
                else if (strcmp(fields[ii].name, "pressure_counts") == 0)
                    mb_get_binary_double(MB_YES, &buffer[fields[ii].index], &pressure_counts);
                else if (strcmp(fields[ii].name, "pressure_temp_comp_voltage_reading") == 0)
                    mb_get_binary_double(MB_YES, &buffer[fields[ii].index], &thermistor);
            }
            temperature_calc = calcTemp(&ctd_calibration, temp_counts);
            pressure_calc = calcPressure(&ctd_calibration, pressure_counts,
                                thermistor);
            conductivity_calc = calcCond(&ctd_calibration, cond_frequency,
                                temperature_calc, pressure_calc);
            /* interp_status = */ mb_seabird_salinity(verbose, conductivity_calc, temperature_calc,
                                pressure_calc, &salinity_calc, &error);
            /* interp_status = */ mb_seabird_soundspeed(verbose, MB_SOUNDSPEEDALGORITHM_DELGROSSO,
                                        salinity_calc, temperature_calc, pressure_calc,
                                        &soundspeed_calc, &error);
            /* interp_status = */ mb_potential_temperature(verbose, temperature_calc, salinity_calc, pressure_calc,
                                        &potentialtemperature_calc, &error);
            /* interp_status = */ mb_seabird_density(verbose, salinity_calc, temperature_calc, pressure_calc, &density_calc, &error);
        } else if (ctd_available == MB_YES) {
            /* else deal with existing values if available */
            for (int ii = 0; ii < nprintfields; ii++) {
				if (strcmp(fields[ii].name, "temperature") == 0)
						mb_get_binary_double(MB_YES, &buffer[fields[ii].index], &temperature_calc);
				else if (strcmp(fields[ii].name, "calculated_salinity") == 0)
						mb_get_binary_double(MB_YES, &buffer[fields[ii].index], &salinity_calc);
				else if (strcmp(fields[ii].name, "conductivity") == 0)
						mb_get_binary_double(MB_YES, &buffer[fields[ii].index], &conductivity_calc);
				else if (strcmp(fields[ii].name, "pressure") == 0)
						mb_get_binary_double(MB_YES, &buffer[fields[ii].index], &pressure_calc);
				}
            /* interp_status = */ mb_seabird_soundspeed(verbose, MB_SOUNDSPEEDALGORITHM_DELGROSSO,
                                        salinity_calc, temperature_calc, pressure_calc,
                                        &soundspeed_calc, &error);
            /* interp_status = */ mb_potential_temperature(verbose, temperature_calc, salinity_calc, pressure_calc,
                                        &potentialtemperature_calc, &error);
            /* interp_status = */ mb_seabird_density(verbose, salinity_calc, temperature_calc, pressure_calc, &density_calc, &error);
        }
    
        /* calculate timestamp by adding Kearfott second-of-day value (utcTime) to seconds to the start of day
         * from the overall timestamp (time) */
        if (ktime_available == MB_YES && calc_ktime == MB_YES) {
            /* else deal with existing values if available */
            startofday_time_d = 0.0;
            ktime_calc = 0.0;
            for (int ii = 0; ii < nfields; ii++) {
				if (strcmp(fields[ii].name, "time") == 0) {
                        mb_get_binary_double(MB_YES, &buffer[fields[ii].index], &time_d);
                        startofday_time_d = MB_SECINDAY * floor(time_d / MB_SECINDAY);
				}
				else if (strcmp(fields[ii].name, "utcTime") == 0) {
                        mb_get_binary_double(MB_YES, &buffer[fields[ii].index], &ktime_calc);
				}
            }
            ktime_calc += startofday_time_d;
        }

        /* loop over the printfields */
		for (int i = 0; i < nprintfields; i++) {
			index = printfields[i].index;
			if (index == INDEX_ZERO) {
				dvalue = 0.0;
				if (output_mode == OUTPUT_MODE_BINARY)
					fwrite(&dvalue, sizeof(double), 1, stdout);
				else
					fprintf(stdout, printfields[i].format, dvalue);
			}
			else if (index == INDEX_MERGE_LON) {
				/* interp_status = */ mb_linear_interp_longitude(verbose, nav_time_d - 1, nav_navlon - 1, nav_num, time_d, &dvalue,
				                                           &jinterp, &error);
				if (jinterp < 2 || jinterp > nav_num - 2)
					dvalue = 0.0;
				if (output_mode == OUTPUT_MODE_BINARY)
					fwrite(&dvalue, sizeof(double), 1, stdout);
				else
					fprintf(stdout, printfields[i].format, dvalue);
			}
			else if (index == INDEX_MERGE_LAT) {
				/* interp_status = */ mb_linear_interp_latitude(verbose, nav_time_d - 1, nav_navlat - 1, nav_num, time_d, &dvalue,
				                                          &jinterp, &error);
				if (jinterp < 2 || jinterp > nav_num - 2)
					dvalue = 0.0;
				if (output_mode == OUTPUT_MODE_BINARY)
					fwrite(&dvalue, sizeof(double), 1, stdout);
				else
					fprintf(stdout, printfields[i].format, dvalue);
			}
			else if (index == INDEX_MERGE_HEADING) {
				/* interp_status = */ mb_linear_interp_heading(verbose, nav_time_d - 1, nav_heading - 1, nav_num, time_d, &dvalue,
				                                         &jinterp, &error);
				if (jinterp < 2 || jinterp > nav_num - 2)
					dvalue = 0.0;
				if (output_mode == OUTPUT_MODE_BINARY)
					fwrite(&dvalue, sizeof(double), 1, stdout);
				else
					fprintf(stdout, printfields[i].format, dvalue);
			}
			else if (index == INDEX_MERGE_SPEED) {
				/* interp_status = */
				    mb_linear_interp(verbose, nav_time_d - 1, nav_speed - 1, nav_num, time_d, &dvalue, &jinterp, &error);
				if (jinterp < 2 || jinterp > nav_num - 2)
					dvalue = 0.0;
				if (output_mode == OUTPUT_MODE_BINARY)
					fwrite(&dvalue, sizeof(double), 1, stdout);
				else
					fprintf(stdout, printfields[i].format, dvalue);
			}
			else if (index == INDEX_MERGE_SENSORDEPTH) {
				/* interp_status = */
				    mb_linear_interp(verbose, nav_time_d - 1, nav_sensordepth - 1, nav_num, time_d, &dvalue, &jinterp, &error);
				if (jinterp < 2 || jinterp > nav_num - 2)
					dvalue = 0.0;
				if (output_mode == OUTPUT_MODE_BINARY)
					fwrite(&dvalue, sizeof(double), 1, stdout);
				else
					fprintf(stdout, printfields[i].format, dvalue);
			}
			else if (index == INDEX_MERGE_ROLL) {
				/* interp_status = */
				    mb_linear_interp(verbose, nav_time_d - 1, nav_roll - 1, nav_num, time_d, &dvalue, &jinterp, &error);
				if (jinterp < 2 || jinterp > nav_num - 2)
					dvalue = 0.0;
				if (output_mode == OUTPUT_MODE_BINARY)
					fwrite(&dvalue, sizeof(double), 1, stdout);
				else
					fprintf(stdout, printfields[i].format, dvalue);
			}
			else if (index == INDEX_MERGE_PITCH) {
				/* interp_status = */
				    mb_linear_interp(verbose, nav_time_d - 1, nav_pitch - 1, nav_num, time_d, &dvalue, &jinterp, &error);
				if (jinterp < 2 || jinterp > nav_num - 2)
					dvalue = 0.0;
				if (output_mode == OUTPUT_MODE_BINARY)
					fwrite(&dvalue, sizeof(double), 1, stdout);
				else
					fprintf(stdout, printfields[i].format, dvalue);
			}
			else if (index == INDEX_MERGE_HEAVE) {
				/* interp_status = */
				    mb_linear_interp(verbose, nav_time_d - 1, nav_heave - 1, nav_num, time_d, &dvalue, &jinterp, &error);
				if (jinterp < 2 || jinterp > nav_num - 2)
					dvalue = 0.0;
				if (output_mode == OUTPUT_MODE_BINARY)
					fwrite(&dvalue, sizeof(double), 1, stdout);
				else
					fprintf(stdout, printfields[i].format, dvalue);
			}
			else if (index == INDEX_CALC_CONDUCTIVITY) {
				if (output_mode == OUTPUT_MODE_BINARY)
					fwrite(&conductivity_calc, sizeof(double), 1, stdout);
				else
					fprintf(stdout, printfields[i].format, conductivity_calc);
			}
			else if (index == INDEX_CALC_TEMPERATURE) {
				if (output_mode == OUTPUT_MODE_BINARY)
					fwrite(&temperature_calc, sizeof(double), 1, stdout);
				else
					fprintf(stdout, printfields[i].format, temperature_calc);
			}
			else if (index == INDEX_CALC_PRESSURE) {
				if (output_mode == OUTPUT_MODE_BINARY)
					fwrite(&pressure_calc, sizeof(double), 1, stdout);
				else
					fprintf(stdout, printfields[i].format, pressure_calc);
			}
			else if (index == INDEX_CALC_SALINITY) {
				if (output_mode == OUTPUT_MODE_BINARY)
					fwrite(&salinity_calc, sizeof(double), 1, stdout);
				else
					fprintf(stdout, printfields[i].format, salinity_calc);
			}
			else if (index == INDEX_CALC_SOUNDSPEED) {
				if (output_mode == OUTPUT_MODE_BINARY)
					fwrite(&soundspeed_calc, sizeof(double), 1, stdout);
				else
					fprintf(stdout, printfields[i].format, soundspeed_calc);
			}
			else if (index == INDEX_CALC_POTENTIALTEMP) {
				if (output_mode == OUTPUT_MODE_BINARY)
					fwrite(&potentialtemperature_calc, sizeof(double), 1, stdout);
				else
					fprintf(stdout, printfields[i].format, potentialtemperature_calc);
			}
			else if (index == INDEX_CALC_DENSITY) {
				if (output_mode == OUTPUT_MODE_BINARY)
					fwrite(&density_calc, sizeof(double), 1, stdout);
				else
					fprintf(stdout, printfields[i].format, density_calc);
			}
			else if (index == INDEX_CALC_KTIME) {
				if (output_mode == OUTPUT_MODE_BINARY)
					fwrite(&ktime_calc, sizeof(double), 1, stdout);
				else
					fprintf(stdout, printfields[i].format, ktime_calc);
			}
			else if (fields[index].type == TYPE_DOUBLE) {
				mb_get_binary_double(MB_YES, &buffer[fields[index].index], &dvalue);
				dvalue *= fields[index].scale;
				if ((strcmp(fields[nfields].name, "mHeadK") == 0 || strcmp(fields[nfields].name, "mYawK") == 0) &&
				    angles_in_degrees == MB_YES && dvalue < 0.0)
					dvalue += 360.0;
				if (output_mode == OUTPUT_MODE_BINARY)
					fwrite(&dvalue, sizeof(double), 1, stdout);
				else
					fprintf(stdout, printfields[i].format, dvalue);
			}
			else if (fields[index].type == TYPE_INTEGER) {
				mb_get_binary_int(MB_YES, &buffer[fields[index].index], &ivalue);
				if (output_mode == OUTPUT_MODE_BINARY)
					fwrite(&ivalue, sizeof(int), 1, stdout);
				else
					fprintf(stdout, printfields[i].format, ivalue);
			}
			else if (fields[index].type == TYPE_TIMETAG) {
				mb_get_binary_double(MB_YES, &buffer[fields[index].index], &dvalue);
				time_d = dvalue;
				if (strcmp(printfields[i].format, "time_i") == 0) {
					mb_get_date(verbose, time_d, time_i);
					if (output_mode == OUTPUT_MODE_BINARY) {
						fwrite(time_i, sizeof(int), 7, stdout);
					}
					else {
						fprintf(stdout, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d", time_i[0], time_i[1], time_i[2], time_i[3],
						        time_i[4], time_i[5], time_i[6]);
					}
				}
				else if (strcmp(printfields[i].format, "time_j") == 0) {
					mb_get_date(verbose, time_d, time_i);
					mb_get_jtime(verbose, time_i, time_j);
					if (output_mode == OUTPUT_MODE_BINARY) {
						fwrite(&time_i[0], sizeof(int), 1, stdout);
						fwrite(&time_j[1], sizeof(int), 1, stdout);
						fwrite(&time_i[3], sizeof(int), 1, stdout);
						fwrite(&time_i[4], sizeof(int), 1, stdout);
						fwrite(&time_i[5], sizeof(int), 1, stdout);
						fwrite(&time_i[6], sizeof(int), 1, stdout);
					}
					else {
						fprintf(stdout, "%4.4d %3.3d %2.2d %2.2d %2.2d.%6.6d", time_i[0], time_j[1], time_i[3], time_i[4],
						        time_i[5], time_i[6]);
					}
				}
				else {
					if (output_mode == OUTPUT_MODE_BINARY)
						fwrite(&dvalue, sizeof(double), 1, stdout);
					else
						fprintf(stdout, printfields[i].format, time_d);
				}
			}
			else if (fields[index].type == TYPE_ANGLE) {
				mb_get_binary_double(MB_YES, &buffer[fields[index].index], &dvalue);
				dvalue *= fields[index].scale;
				if (strcmp(fields[index].name, "mYawCB") == 0 && angles_in_degrees == MB_YES && dvalue < 0.0)
					dvalue += 360.0;
				if (output_mode == OUTPUT_MODE_BINARY)
					fwrite(&dvalue, sizeof(double), 1, stdout);
				else
					fprintf(stdout, printfields[i].format, dvalue);
			}
			if (output_mode == OUTPUT_MODE_TAB) {
				if (i < nprintfields - 1)
					fprintf(stdout, "\t");
				else
					fprintf(stdout, "\n");
			}
			else if (output_mode == OUTPUT_MODE_CSV) {
				if (i < nprintfields - 1)
					fprintf(stdout, ",");
				else
					fprintf(stdout, "\n");
			}
		}
		nrecord++;
	}
	fclose(fp);

	/* deallocate arrays for navigation */
	if (nav_alloc > 0) {
		mb_freed(verbose, __FILE__, __LINE__, (void **)&nav_time_d, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&nav_navlon, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&nav_navlat, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&nav_heading, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&nav_speed, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&nav_sensordepth, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&nav_roll, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&nav_pitch, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&nav_heave, &error);
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s> completed\n", program_name);
		fprintf(stderr, "dbg2  Ending status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	exit(error);
}
/*--------------------------------------------------------------------*/

void calibration_MAUV1_2017(struct ctd_calibration_struct *calibration_ptr)
{
	calibration_ptr->pa0 = 8.580044e-001;
	calibration_ptr->pa1 = 1.108702e-001;
	calibration_ptr->pa2 = -2.247276e-009;
	calibration_ptr->ptempa0 = 5.929376e+001;
	calibration_ptr->ptempa1 = -3.132766e+001;
	calibration_ptr->ptempa2 = 3.934270e+000;
	calibration_ptr->ptca0 = 5.247614e+005;
	calibration_ptr->ptca1 = 1.857443e+000;
	calibration_ptr->ptca2 = 2.311606e-003;
	calibration_ptr->ptcb0 = 2.769200e+001;
	calibration_ptr->ptcb1 = 4.400000e-003;
	calibration_ptr->ptcb2 = 0.;

	calibration_ptr->a0 = 8.391167e-004;
	calibration_ptr->a1 = 2.789202e-004;
	calibration_ptr->a2 = -1.769508e-006;
	calibration_ptr->a3 = 1.831480e-007;

	calibration_ptr->g = -1.000098e+000;
	calibration_ptr->h = 1.542017e-001;
	calibration_ptr->i = -4.018137e-004;
	calibration_ptr->j = 5.724026e-005;
	calibration_ptr->cpcor = -9.5700e-008;
	calibration_ptr->ctcor = 3.2500e-006;
}

