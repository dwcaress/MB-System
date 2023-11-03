/*--------------------------------------------------------------------
 *    The MB-system:	mbauvloglist.c	8/14/2006
 *
 *    Copyright (c) 2006-2023 by
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
 * MBauvloglist prints the specified contents of an MBARI AUV mission log
 * file to stdout. The form of the output is quite flexible;
 * MBsegylist is tailored to produce ascii files in spreadsheet
 * style with data columns separated by tabs.
 *
 * Author:	D. W. Caress
 * Date:	August 14, 2006
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
#include "mb_status.h"

constexpr int NFIELDSMAX = 512;
constexpr int MAX_OPTIONS = 512;
typedef enum {
    TYPE_UNKNOWN = 0,
    TYPE_TIMETAG = 1,
    TYPE_SHORT = 2,
    TYPE_INTEGER = 3,
    TYPE_DOUBLE = 4,
    TYPE_ANGLE = 5,
} field_type_t;

typedef enum {
    INDEX_ZERO = -1,
    INDEX_MERGE_ALTITUDE = -2,
    INDEX_MERGE_LON = -3,
    INDEX_MERGE_LAT = -4,
    INDEX_MERGE_HEADING = -5,
    INDEX_MERGE_SPEED = -6,
    INDEX_MERGE_SENSORDEPTH = -7,
    INDEX_MERGE_ROLL = -8,
    INDEX_MERGE_PITCH = -9,
    INDEX_MERGE_HEAVE = -10,
    INDEX_CALC_CONDUCTIVITY = -11,
    INDEX_CALC_TEMPERATURE = -12,
    INDEX_CALC_PRESSURE = -13,
    INDEX_CALC_SALINITY = -14,
    INDEX_CALC_SOUNDSPEED = -15,
    INDEX_CALC_POTENTIALTEMP = -16,
    INDEX_CALC_DENSITY = -17,
    INDEX_CALC_KTIME = -18,
    INDEX_CALC_KSPEED = -19,
    INDEX_TIME_INTERVAL = -20,
} index_t;

typedef enum {
    OUTPUT_MODE_TAB = 0,
    OUTPUT_MODE_CSV = 1,
    OUTPUT_MODE_BINARY = 2,
} output_t;

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

struct field {
	field_type_t type;
	int size;
	index_t index;
	char name[MB_PATH_MAXLINE];
	char format[MB_PATH_MAXLINE];
	char description[MB_PATH_MAXLINE];
	char units[MB_PATH_MAXLINE];
	double scale;
};

struct printfield {
	char name[MB_PATH_MAXLINE];
	int index;
	bool formatset;
	char format[MB_PATH_MAXLINE];
};

constexpr char program_name[] = "MBauvloglist";
constexpr char help_message[] = "MBauvloglist lists table data from an MBARI AUV mission log file.";
constexpr char usage_message[] = "MBauvloglist -Ifile [-Fprintformat -Llonflip -Olist -Rid -S -H -V]";

/*--------------------------------------------------------------------*/

//returns pressure in dbar. Returned pressure is zero at surface, assuming
//atmospheric pressure fixed at 14.7PSI
double calcPressure(struct ctd_calibration_struct *calibration_ptr,
				double presCounts, double temperature)
{
  const double t = calibration_ptr->ptempa0
								+ calibration_ptr->ptempa1*temperature
								+ calibration_ptr->ptempa2*temperature*temperature;
  const double x = (double)presCounts - calibration_ptr->ptca0
								- calibration_ptr->ptca1*t
								- calibration_ptr->ptca2*t*t;
  const double n = x*calibration_ptr->ptcb0 / (calibration_ptr->ptcb0
								+ calibration_ptr->ptcb1*t
								+ calibration_ptr->ptcb2*t*t);
  double pressure = calibration_ptr->pa0
								+ calibration_ptr->pa1*n
								+ calibration_ptr->pa2*n*n;

  pressure = (pressure-14.7)*.6894757; //per note on page 34 of the SBE49 Manual
  return pressure;
}
/*--------------------------------------------------------------------*/

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
/*--------------------------------------------------------------------*/

double calcCond(const struct ctd_calibration_struct *calibration_ptr,
		double cFreq, double temp, double pressure)
{

  // pressure *= 1.45; // Convert to psia for this formula

  cFreq /= 1000.0;
  const double cond = (calibration_ptr->g + calibration_ptr->h*cFreq*cFreq
				 + calibration_ptr->i*cFreq*cFreq*cFreq
				 + calibration_ptr->j*cFreq*cFreq*cFreq*cFreq)
                        / (1 + calibration_ptr->ctcor*temp
							+ calibration_ptr->cpcor*pressure);

  return cond;
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

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	int verbose = 0;
	int pings;
	int format;
	int lonflip;
	double bounds[4];
	int btime_i[7];
	int etime_i[7];
	double speedmin;
	double timegap;

	int status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

	bool printheader = false;
	char file[MB_PATH_MAXLINE] = "";
	mb_path altitude_file = "";
	bool altitude_merge = false;
	mb_path nav_file = "";
	bool nav_merge = false;
    bool merge_clip = false;
    int decimate = 1;
	output_t output_mode = OUTPUT_MODE_TAB;
	int nprintfields = 0;
	struct printfield printfields[NFIELDSMAX];
	bool calc_potentialtemp = false;
	bool calc_soundspeed = false;
	bool calc_density = false;
	bool calc_ktime = false;
    bool calc_kspeed = false;
	bool recalculate_ctd = false;
	int ctd_calibration_id = 0;
	bool angles_in_degrees = false;
    bool calculate_time_interval  = false;

	{
		bool errflg = false;
		int c;
		bool help = false;
		char printformat[MB_PATH_MAXLINE] = "default";  // TODO(schwehr): Is this used correctly?
		while ((c = getopt(argc, argv, "A:a:CcD:d:F:f:I:i:L:l:M:m:N:n:O:o:PpR:r:SsVvWwHh")) != -1)
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
				sscanf(optarg, "%1023s", altitude_file);
				altitude_merge = true;
				break;
			case 'C':
			case 'c':
				merge_clip = true;
				break;
			case 'D':
			case 'd':
				sscanf(optarg, "%d", &decimate);
				break;
			case 'F':
			case 'f':
				sscanf(optarg, "%1023s", printformat);
				break;
			case 'I':
			case 'i':
				sscanf(optarg, "%1023s", file);
				break;
			case 'L':
			case 'l':
				sscanf(optarg, "%d", &lonflip);
				break;
			case 'M':
			case 'm':
			{
				int tmp;
				sscanf(optarg, "%d", &tmp);
				output_mode = (output_t)tmp;  // TODO(schwehr): Range check
				break;
			}
			case 'N':
			case 'n':
				sscanf(optarg, "%1023s", nav_file);
				nav_merge = true;
				break;
			case 'O':
			case 'o':
			{
				/* const int nscan = */ sscanf(optarg, "%1023s", printfields[nprintfields].name);
				if (strlen(printformat) > 0 && strcmp(printformat, "default") != 0) {
					printfields[nprintfields].formatset = true;
					strcpy(printfields[nprintfields].format, printformat);
				}
				else {
					printfields[nprintfields].formatset = false;
					strcpy(printfields[nprintfields].format, "");
				}
				if (strcmp(printfields[nprintfields].name, "calcPotentialTemperature") == 0)
					calc_potentialtemp = true;
				if (strcmp(printfields[nprintfields].name, "calcSoundspeed") == 0)
					calc_soundspeed = true;
				if (strcmp(printfields[nprintfields].name, "calcDensity") == 0)
					calc_density = true;
				if (strcmp(printfields[nprintfields].name, "calcKTime") == 0)
					calc_ktime = true;
				if (strcmp(printfields[nprintfields].name, "calcKSpeed") == 0)
					calc_kspeed = true;
        		if (strcmp(printfields[nprintfields].name, "timeInterval") == 0)
          			calculate_time_interval = true;
				printfields[nprintfields].index = -1;
				nprintfields++;
				break;
			}
			case 'P':
			case 'p':
				printheader = true;
				break;
			case 'R':
			case 'r':
				recalculate_ctd = true;
				sscanf(optarg, "%d", &ctd_calibration_id);
				break;
			case 'S':
			case 's':
				angles_in_degrees = true;
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
			fprintf(stderr, "dbg2       verbose:                  %d\n", verbose);
			fprintf(stderr, "dbg2       help:                     %d\n", help);
			fprintf(stderr, "dbg2       lonflip:                  %d\n", lonflip);
			fprintf(stderr, "dbg2       bounds[0]:                %f\n", bounds[0]);
			fprintf(stderr, "dbg2       bounds[1]:                %f\n", bounds[1]);
			fprintf(stderr, "dbg2       bounds[2]:                %f\n", bounds[2]);
			fprintf(stderr, "dbg2       bounds[3]:                %f\n", bounds[3]);
			fprintf(stderr, "dbg2       btime_i[0]:               %d\n", btime_i[0]);
			fprintf(stderr, "dbg2       btime_i[1]:               %d\n", btime_i[1]);
			fprintf(stderr, "dbg2       btime_i[2]:               %d\n", btime_i[2]);
			fprintf(stderr, "dbg2       btime_i[3]:               %d\n", btime_i[3]);
			fprintf(stderr, "dbg2       btime_i[4]:               %d\n", btime_i[4]);
			fprintf(stderr, "dbg2       btime_i[5]:               %d\n", btime_i[5]);
			fprintf(stderr, "dbg2       btime_i[6]:               %d\n", btime_i[6]);
			fprintf(stderr, "dbg2       etime_i[0]:               %d\n", etime_i[0]);
			fprintf(stderr, "dbg2       etime_i[1]:               %d\n", etime_i[1]);
			fprintf(stderr, "dbg2       etime_i[2]:               %d\n", etime_i[2]);
			fprintf(stderr, "dbg2       etime_i[3]:               %d\n", etime_i[3]);
			fprintf(stderr, "dbg2       etime_i[4]:               %d\n", etime_i[4]);
			fprintf(stderr, "dbg2       etime_i[5]:               %d\n", etime_i[5]);
			fprintf(stderr, "dbg2       etime_i[6]:               %d\n", etime_i[6]);
			fprintf(stderr, "dbg2       speedmin:                 %f\n", speedmin);
			fprintf(stderr, "dbg2       timegap:                  %f\n", timegap);
			fprintf(stderr, "dbg2       file:                     %s\n", file);
			fprintf(stderr, "dbg2       altitude_merge:           %d\n", altitude_merge);
			fprintf(stderr, "dbg2       altitude_file:            %s\n", altitude_file);
			fprintf(stderr, "dbg2       nav_merge:                %d\n", nav_merge);
			fprintf(stderr, "dbg2       nav_file:                 %s\n", nav_file);
			fprintf(stderr, "dbg2       merge_clip:               %d\n", merge_clip);
			fprintf(stderr, "dbg2       decimate:                 %d\n", decimate);
			fprintf(stderr, "dbg2       nav_file:                 %s\n", nav_file);
			fprintf(stderr, "dbg2       output_mode:              %d\n", output_mode);
			fprintf(stderr, "dbg2       printheader:              %d\n", printheader);
			fprintf(stderr, "dbg2       angles_in_degrees:        %d\n", angles_in_degrees);
			fprintf(stderr, "dbg2       calc_potentialtemp:       %d\n", calc_potentialtemp);
			fprintf(stderr, "dbg2       recalculate_ctd:          %d\n", recalculate_ctd);
			fprintf(stderr, "dbg2       ctd_calibration_id:       %d\n", ctd_calibration_id);
			fprintf(stderr, "dbg2       calc_ktime:               %d\n", calc_ktime);
      fprintf(stderr, "dbg2       calculate_time_interval:  %d\n", calculate_time_interval);
			fprintf(stderr, "dbg2       nprintfields:             %d\n", nprintfields);
			for (int i = 0; i < nprintfields; i++)
				fprintf(stderr, "dbg2         printfields[%d]:          %s %d %s\n", i, printfields[i].name, printfields[i].formatset,
				        printfields[i].format);
		}

		if (help) {
			fprintf(stderr, "\n%s\n", help_message);
			fprintf(stderr, "\nusage: %s\n", usage_message);
			exit(MB_ERROR_NO_ERROR);
		}
	}

	int error = MB_ERROR_NO_ERROR;
	char buffer[MB_PATH_MAXLINE];

	/* altitude data for merging */
	int alt_alloc = 0;
	int alt_num = 0;
	double *alt_time_d = nullptr;
	double *alt_altitude = nullptr;

	/* navigation, heading, attitude data for merging in fnv format */
	int nav_alloc = 0;
	int nav_num = 0;
	double *nav_time_d = nullptr;
	double *nav_navlon = nullptr;
	double *nav_navlat = nullptr;
	double *nav_heading = nullptr;
	double *nav_speed = nullptr;
	double *nav_sensordepth = nullptr;
	double *nav_roll = nullptr;
	double *nav_pitch = nullptr;
	double *nav_heave = nullptr;

	int time_i[7];

	/* if altitude merging to be done get altitude */
	if (altitude_merge && strlen(altitude_file) > 0) {
		/* count the data points in the altitude file */
		alt_num = 0;
		const int nchar = MB_PATH_MAXLINE - 1;
		FILE *fp = fopen(altitude_file, "r");
		if (fp == nullptr) {
			fprintf(stderr, "\nUnable to Open Altitude File <%s> for reading\n", altitude_file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_OPEN_FAIL);
		}
		char *result;
		while ((result = fgets(buffer, nchar, fp)) == buffer)
			alt_num++;
		fclose(fp);

		/* allocate arrays for nav */
		if (alt_num > 0) {
			alt_alloc = alt_num;
			/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, alt_num * sizeof(double), (void **)&alt_time_d, &error);
			/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, alt_num * sizeof(double), (void **)&alt_altitude, &error);

			/* if error initializing memory then quit */
			if (error != MB_ERROR_NO_ERROR) {
				char *message = nullptr;
				mb_error(verbose, error, &message);
				fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
				fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
				exit(error);
			}
		}

		/* read the data points in the altitude file */
		alt_num = 0;
		if ((fp = fopen(altitude_file, "r")) == nullptr) {
			fprintf(stderr, "\nUnable to open altitude file <%s> for reading\n", altitude_file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_OPEN_FAIL);
		}
    if (fp != NULL) {
      bool done = false;
      while (!done) {
        memset(buffer, 0, MB_PATH_MAXLINE);
      	char *line_ptr = fgets(buffer, MB_PATH_MAXLINE, fp);
      	if (line_ptr == NULL) {
          done = true;
        }
        else if (buffer[0] != '#') {
    			double sec;
    			const int nget = sscanf(
    				buffer, "%lf %lf", &alt_time_d[alt_num], &alt_altitude[alt_num]);
    			bool alt_ok = nget >= 2;
    			if (alt_num > 0 && alt_time_d[alt_num] <= alt_time_d[alt_num - 1])
    				alt_ok = false;
    			if (alt_ok)
    				alt_num++;
        }
      }
		  fclose(fp);
    }
	}
  if (altitude_merge)
		fprintf(stderr, "%d %d records read from altitude file %s\n", alt_alloc, alt_num, altitude_file);

	/* if nav merging to be done get nav */
	if (nav_merge && strlen(nav_file) > 0) {
		/* count the data points in the nav file */
		nav_num = 0;
		const int nchar = MB_PATH_MAXLINE - 1;
		FILE *fp = fopen(nav_file, "r");
		if (fp == nullptr) {
			fprintf(stderr, "\nUnable to Open Navigation File <%s> for reading\n", nav_file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_OPEN_FAIL);
		}
		char *result;
		while ((result = fgets(buffer, nchar, fp)) == buffer)
			nav_num++;
		fclose(fp);

		/* allocate arrays for nav */
		if (nav_num > 0) {
			nav_alloc = nav_num;
			/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nav_alloc * sizeof(double), (void **)&nav_time_d, &error);
			/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nav_alloc * sizeof(double), (void **)&nav_navlon, &error);
			/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nav_alloc * sizeof(double), (void **)&nav_navlat, &error);
			/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nav_alloc * sizeof(double), (void **)&nav_heading, &error);
			/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nav_alloc * sizeof(double), (void **)&nav_speed, &error);
			/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nav_alloc * sizeof(double), (void **)&nav_sensordepth, &error);
			/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nav_alloc * sizeof(double), (void **)&nav_roll, &error);
			/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nav_alloc * sizeof(double), (void **)&nav_pitch, &error);
			/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nav_alloc * sizeof(double), (void **)&nav_heave, &error);

			/* if error initializing memory then quit */
			if (error != MB_ERROR_NO_ERROR) {
				char *message = nullptr;
				mb_error(verbose, error, &message);
				fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
				fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
				exit(error);
			}
		}

		/* read the data points in the nav file */
		nav_num = 0;
		if ((fp = fopen(nav_file, "r")) == nullptr) {
			fprintf(stderr, "\nUnable to Open navigation File <%s> for reading\n", nav_file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_OPEN_FAIL);
		}
    if (fp != NULL) {
      bool done = false;
      while (!done) {
        memset(buffer, 0, MB_PATH_MAXLINE);
      	char *line_ptr = fgets(buffer, MB_PATH_MAXLINE, fp);
      	if (line_ptr == NULL) {
          done = true;
        }
        else if (buffer[0] != '#') {
    			double sec;
    			const int nget = sscanf(
    				buffer, "%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &time_i[0], &time_i[1], &time_i[2],
    				&time_i[3], &time_i[4], &sec, &nav_time_d[nav_num], &nav_navlon[nav_num], &nav_navlat[nav_num],
    				&nav_heading[nav_num], &nav_speed[nav_num], &nav_sensordepth[nav_num], &nav_roll[nav_num],
    				&nav_pitch[nav_num], &nav_heave[nav_num]);
    			bool nav_ok = nget >= 9;
    			if (nav_num > 0 && nav_time_d[nav_num] <= nav_time_d[nav_num - 1])
    				nav_ok = false;
    			if (nav_ok)
    				nav_num++;
        }
      }
		  fclose(fp);
    }
	}
  if (nav_merge)
		fprintf(stderr, "%d %d records read from nav file %s\n", nav_alloc, nav_num, nav_file);

	/* open the input file */
	FILE *fp = fopen(file, "r");
	if (fp == nullptr) {
		error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
		fprintf(stderr, "\nUnable to open log file <%s> for reading\n", file);
		exit(status);
	}

	int nfields = 0;
	int recordsize = 0;
	bool ktime_available = false;
  bool kvelocity_available = false;

	/* auv log data */
	struct field fields[NFIELDSMAX];

	bool cond_frequency_available = false;
	bool temp_counts_available = false;
	bool pressure_counts_available = false;
	bool thermistor_available = false;
	bool conductivity_available = false;
	bool temperature_available = false;
	bool pressure_available = false;

	char *result;
	while ((result = fgets(buffer, MB_PATH_MAXLINE, fp)) == buffer &&
               strncmp(buffer, "# begin", 7) != 0) {
		char type[MB_PATH_MAXLINE];
		const int nscan = sscanf(buffer, "# %1023s %1023s %1023s", type, fields[nfields].name, fields[nfields].format);
		if (nscan == 2) {
			if (printheader)
				fprintf(stdout, "# csv %s\n", fields[nfields].name);
		} else if (nscan == 3) {
			if (printheader)
				fprintf(stdout, "%s", buffer);

			result = (char *)strchr(buffer, ',');
			strcpy(fields[nfields].description, &(result[1]));
			result = (char *)strchr(fields[nfields].description, ',');
			result[0] = 0;
			result = (char *)strrchr(buffer, ',');
			strcpy(fields[nfields].units, &(result[1]));

			fields[nfields].index = static_cast<index_t>(recordsize);
			if (strcmp(type, "double") == 0) {
				fields[nfields].type = TYPE_DOUBLE;
				fields[nfields].size = 8;
				/* TODO(schwehr): Is something missing? */
				if (angles_in_degrees &&
				    (strcmp(fields[nfields].name, "mLonK") == 0   || strcmp(fields[nfields].name, "mLatK") == 0 ||
             strcmp(fields[nfields].name, "mRollK") == 0  || strcmp(fields[nfields].name, "mPitchK") == 0 ||
             strcmp(fields[nfields].name, "mHeadK") == 0  || strcmp(fields[nfields].name, "mYawK") == 0 ||
             strcmp(fields[nfields].name, "mLonCB") == 0  || strcmp(fields[nfields].name, "mLatCB") == 0 ||
             strcmp(fields[nfields].name, "mRollCB") == 0 || strcmp(fields[nfields].name, "mPitchCB") == 0 ||
             strcmp(fields[nfields].name, "mHeadCB") == 0 || strcmp(fields[nfields].name, "mYawCB") == 0))
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
			else if (strcmp(type, "short") == 0) {
				fields[nfields].type = TYPE_SHORT;
				fields[nfields].size = 2;
				fields[nfields].scale = 1.0;
				recordsize += 2;
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

      /* check if kearfott time is in this file */
      if (strcmp(fields[nfields].name, "utcTime") == 0)
          ktime_available = true;

      /* check if kearfott velocity vector is in this file */
      if (strcmp(fields[nfields].name, "mVbodyxK") == 0)
          kvelocity_available = true;

      /* check if raw and processed ctd data are in this file */
      if (strcmp(fields[nfields].name, "cond_frequency") == 0)
          cond_frequency_available = true;
      else if (strcmp(fields[nfields].name, "temp_counts") == 0)
          temp_counts_available = true;
      else if (strcmp(fields[nfields].name, "pressure_counts") == 0)
          pressure_counts_available = true;
      else if (strcmp(fields[nfields].name, "pressure_temp_comp_voltage_reading") == 0)
          thermistor_available = true;
      else if (strcmp(fields[nfields].name, "conductivity") == 0)
          conductivity_available = true;
      else if (strcmp(fields[nfields].name, "temperature") == 0)
          temperature_available = true;
      else if (strcmp(fields[nfields].name, "pressure") == 0)
          pressure_available = true;

            /* increment counter */
			nfields++;
		}
	}

	/* end here if asked only to print header */
	if (nprintfields == 0 && printheader)
		exit(error);

	/* by default print everything */
	if (nprintfields == 0) {
		nprintfields = nfields;
		for (int i = 0; i < nfields; i++) {
			strcpy(printfields[i].name, fields[i].name);
			printfields[i].index = i;
			printfields[i].formatset = false;
			strcpy(printfields[i].format, fields[i].format);
		}
	}

	/* recalculate ctd data from fundamental observations */
	struct ctd_calibration_struct ctd_calibration;

	/* if recalculating CTD data then check for available raw CTD data */
	if (recalculate_ctd) {
	  if (!cond_frequency_available
	        || !temp_counts_available
	        || !pressure_counts_available
	        || !thermistor_available) {
		  fprintf(stderr, "\nUnable to recalculate CTD data as requested, raw CTD data not in file <%s>\n", file);
		  exit(MB_ERROR_BAD_FORMAT);
	  } else {
	        calibration_MAUV1_2017(&ctd_calibration);
	  }
	}
	bool ctd_available = false;
	if (conductivity_available
	    && temperature_available
	    && pressure_available) {
	    ctd_available = true;
	}
	if (calc_potentialtemp
	    || calc_soundspeed
	    || calc_density) {
	    if (!recalculate_ctd && !ctd_available) {
	            error = MB_ERROR_BAD_FORMAT;
	            status = MB_FAILURE;
	            fprintf(stderr, "\nUnable to calculate CTD data products as requested, CTD data not in file <%s>\n", file);
	            exit(status);
	    }
	}

    /* if calculating speed from Kearfott velocity vector check for available Kearfott data */
    if (calc_kspeed && !kvelocity_available) {
		fprintf(stderr, "\nUnable to calculate speed from Kearfott data as requested, Kearfoot velocity data not in file <%s>\n", file);
		exit(MB_ERROR_BAD_FORMAT);
    }

	/* check the fields to be printed */
	for (int i = 0; i < nprintfields; i++) {
		if (strcmp(printfields[i].name, "zero") == 0) {
			printfields[i].index = INDEX_ZERO;
			if (!printfields[i].formatset) {
				strcpy(printfields[i].format, "%f");
			}
		}
		else if (strcmp(printfields[i].name, "timeTag") == 0) {
			printfields[i].index = INDEX_ZERO;
			if (!printfields[i].formatset) {
				strcpy(printfields[i].format, "%.8f");
			}
		}
		else if (strcmp(printfields[i].name, "mergeAltitude") == 0) {
			printfields[i].index = INDEX_MERGE_ALTITUDE;
			if (!printfields[i].formatset) {
				strcpy(printfields[i].format, "%.3f");
			}
		}
		else if (strcmp(printfields[i].name, "mergeLon") == 0) {
			printfields[i].index = INDEX_MERGE_LON;
			if (!printfields[i].formatset) {
				strcpy(printfields[i].format, "%.9f");
			}
		}
		else if (strcmp(printfields[i].name, "mergeLat") == 0) {
			printfields[i].index = INDEX_MERGE_LAT;
			if (!printfields[i].formatset) {
				strcpy(printfields[i].format, "%.9f");
			}
		}
		else if (strcmp(printfields[i].name, "mergeHeading") == 0) {
			printfields[i].index = INDEX_MERGE_HEADING;
			if (!printfields[i].formatset) {
				strcpy(printfields[i].format, "%.3f");
			}
		}
		else if (strcmp(printfields[i].name, "mergeSpeed") == 0) {
			printfields[i].index = INDEX_MERGE_SPEED;
			if (!printfields[i].formatset) {
				strcpy(printfields[i].format, "%.3f");
			}
		}
		else if (strcmp(printfields[i].name, "mergeDraft") == 0) {
			printfields[i].index = INDEX_MERGE_SENSORDEPTH;
			if (!printfields[i].formatset) {
				strcpy(printfields[i].format, "%.3f");
			}
		}
		else if (strcmp(printfields[i].name, "mergeSensordepth") == 0) {
			printfields[i].index = INDEX_MERGE_SENSORDEPTH;
			if (!printfields[i].formatset) {
				strcpy(printfields[i].format, "%.3f");
			}
		}
		else if (strcmp(printfields[i].name, "mergeRoll") == 0) {
			printfields[i].index = INDEX_MERGE_ROLL;
			if (!printfields[i].formatset) {
				strcpy(printfields[i].format, "%.3f");
			}
		}
		else if (strcmp(printfields[i].name, "mergePitch") == 0) {
			printfields[i].index = INDEX_MERGE_PITCH;
			if (!printfields[i].formatset) {
				strcpy(printfields[i].format, "%.3f");
			}
		}
		else if (strcmp(printfields[i].name, "mergeHeave") == 0) {
			printfields[i].index = INDEX_MERGE_HEAVE;
			if (!printfields[i].formatset) {
				strcpy(printfields[i].format, "%.3f");
			}
		}
		else if (strcmp(printfields[i].name, "calcConductivity") == 0) {
			printfields[i].index = INDEX_CALC_CONDUCTIVITY;
			if (!printfields[i].formatset) {
				strcpy(printfields[i].format, "%.8f");
			}
		}
		else if (strcmp(printfields[i].name, "calcTemperature") == 0) {
			printfields[i].index = INDEX_CALC_TEMPERATURE;
			if (!printfields[i].formatset) {
				strcpy(printfields[i].format, "%.8f");
			}
		}
		else if (strcmp(printfields[i].name, "calcPressure") == 0) {
			printfields[i].index = INDEX_CALC_PRESSURE;
			if (!printfields[i].formatset) {
				strcpy(printfields[i].format, "%.8f");
			}
		}
		else if (strcmp(printfields[i].name, "calcSalinity") == 0) {
			printfields[i].index = INDEX_CALC_SALINITY;
			if (!printfields[i].formatset) {
				strcpy(printfields[i].format, "%.8f");
			}
		}
		else if (strcmp(printfields[i].name, "calcSoundspeed") == 0) {
			printfields[i].index = INDEX_CALC_SOUNDSPEED;
			if (!printfields[i].formatset) {
				strcpy(printfields[i].format, "%.8f");
			}
		}
		else if (strcmp(printfields[i].name, "calcPotentialTemperature") == 0) {
			printfields[i].index = INDEX_CALC_POTENTIALTEMP;
			if (!printfields[i].formatset) {
				strcpy(printfields[i].format, "%.8f");
			}
		}
		else if (strcmp(printfields[i].name, "calcDensity") == 0) {
			printfields[i].index = INDEX_CALC_DENSITY;
			if (!printfields[i].formatset) {
				strcpy(printfields[i].format, "%.8f");
			}
		}
		else if (strcmp(printfields[i].name, "calcKTime") == 0) {
			printfields[i].index = INDEX_CALC_KTIME;
			if (!printfields[i].formatset) {
				strcpy(printfields[i].format, "%.8f");
			}
		}
		else if (strcmp(printfields[i].name, "calcKSpeed") == 0) {
			printfields[i].index = INDEX_CALC_KSPEED;
			if (!printfields[i].formatset) {
				strcpy(printfields[i].format, "%.3f");
			}
		}
		else if (strcmp(printfields[i].name, "timeInterval") == 0) {
			printfields[i].index = INDEX_TIME_INTERVAL;
			if (!printfields[i].formatset) {
				strcpy(printfields[i].format, "%.3f");
			}
		}
		else {
			for (int j = 0; j < nfields; j++) {
				if (strcmp(printfields[i].name, fields[j].name) == 0)
					printfields[i].index = j;
			}
			if (!printfields[i].formatset) {
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

	/* values used to calculate some water properties */
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
    double time_interval = 0.0;
    double prior_time_d = 0.0;

	/* read the data records in the auv log file */
	int decimate_count = 0;
	int nrecord = 0;
	while (fread(buffer, recordsize, 1, fp) == 1) {
        bool output_ok = true;
        decimate_count++;
	    double time_d = 0.0;
        for (int ii = 0; ii < nfields; ii++) {
          if (strcmp(fields[ii].name, "time") == 0)
            mb_get_binary_double(true, &buffer[fields[ii].index], &time_d);
        }

        /* if needed check timestamp */
        if (nav_merge && merge_clip) {
          for (int ii = 0; ii < nfields; ii++) {
            if (strcmp(fields[ii].name, "time") == 0)
              mb_get_binary_double(true, &buffer[fields[ii].index], &time_d);
          }
          if (time_d < nav_time_d[1] || time_d > nav_time_d[nav_num-2])
            output_ok = false;
        }

        /* calculate timeInterval */
        if (calculate_time_interval) {
          if (prior_time_d > 0.0) {
            time_interval = time_d - prior_time_d;
          }
          prior_time_d = time_d;
        }

        /* recalculate CTD data if requested */
        if (recalculate_ctd) {
            for (int ii = 0; ii < nfields; ii++) {
                if (strcmp(fields[ii].name, "cond_frequency") == 0)
                    mb_get_binary_double(true, &buffer[fields[ii].index], &cond_frequency);
                else if (strcmp(fields[ii].name, "temp_counts") == 0)
                    mb_get_binary_double(true, &buffer[fields[ii].index], &temp_counts);
                else if (strcmp(fields[ii].name, "pressure_counts") == 0)
                    mb_get_binary_double(true, &buffer[fields[ii].index], &pressure_counts);
                else if (strcmp(fields[ii].name, "pressure_temp_comp_voltage_reading") == 0)
                    mb_get_binary_double(true, &buffer[fields[ii].index], &thermistor);
            }
            temperature_calc = calcTemp(&ctd_calibration, temp_counts);
            pressure_calc = calcPressure(&ctd_calibration, pressure_counts,
                                thermistor);
            conductivity_calc = calcCond(&ctd_calibration, cond_frequency,
                                temperature_calc, pressure_calc);
            mb_seabird_salinity(verbose, conductivity_calc, temperature_calc,
                                pressure_calc, &salinity_calc, &error);
            mb_seabird_soundspeed(verbose, MB_SOUNDSPEEDALGORITHM_DELGROSSO,
                                        salinity_calc, temperature_calc, pressure_calc,
                                        &soundspeed_calc, &error);
            mb_potential_temperature(verbose, temperature_calc, salinity_calc, pressure_calc,
                                        &potentialtemperature_calc, &error);
            mb_seabird_density(verbose, salinity_calc, temperature_calc, pressure_calc, &density_calc, &error);
        } else if (ctd_available) {
            /* else deal with existing values if available */
            for (int ii = 0; ii < nprintfields; ii++) {
      				if (strcmp(fields[ii].name, "temperature") == 0)
      						mb_get_binary_double(true, &buffer[fields[ii].index], &temperature_calc);
      				else if (strcmp(fields[ii].name, "calculated_salinity") == 0)
      						mb_get_binary_double(true, &buffer[fields[ii].index], &salinity_calc);
      				else if (strcmp(fields[ii].name, "conductivity") == 0)
      						mb_get_binary_double(true, &buffer[fields[ii].index], &conductivity_calc);
      				else if (strcmp(fields[ii].name, "pressure") == 0)
      						mb_get_binary_double(true, &buffer[fields[ii].index], &pressure_calc);
      				}
            mb_seabird_soundspeed(verbose, MB_SOUNDSPEEDALGORITHM_DELGROSSO,
                                        salinity_calc, temperature_calc, pressure_calc,
                                        &soundspeed_calc, &error);
            mb_potential_temperature(verbose, temperature_calc, salinity_calc, pressure_calc,
                                        &potentialtemperature_calc, &error);
            mb_seabird_density(verbose, salinity_calc, temperature_calc, pressure_calc, &density_calc, &error);
        }

        /* calculate timestamp by adding Kearfott second-of-day value (utcTime) to seconds to the start of day
         * from the overall timestamp (time) */
	    double ktime_calc = 0.0;
        if (ktime_available && calc_ktime) {
            /* else deal with existing values if available */
            double startofday_time_d = 0.0;
            ktime_calc = 0.0;
            for (int ii = 0; ii < nfields; ii++) {
        				if (strcmp(fields[ii].name, "time") == 0) {
                  mb_get_binary_double(true, &buffer[fields[ii].index], &time_d);
                  startofday_time_d = MB_SECINDAY * floor(time_d / MB_SECINDAY);
        				}
        				else if (strcmp(fields[ii].name, "utcTime") == 0) {
                  mb_get_binary_double(true, &buffer[fields[ii].index], &ktime_calc);
        				}
            }
            ktime_calc += startofday_time_d;
        }

        /* calculate lateral speed in km/hr from x and y velocity components in Kearfott data */
        double kspeed_calc = 0.0;
        if (kvelocity_available && calc_kspeed) {
          double mVbodyxK = 0.0;
          double mVbodyyK = 0.0;
          for (int ii = 0; ii < nfields; ii++) {
      				if (strcmp(fields[ii].name, "mVbodyxK") == 0) {
                mb_get_binary_double(true, &buffer[fields[ii].index], &mVbodyxK);
      				}
      				else if (strcmp(fields[ii].name, "mVbodyyK") == 0) {
                mb_get_binary_double(true, &buffer[fields[ii].index], &mVbodyyK);
      				}
          }
          kspeed_calc = 3.6 * sqrt(mVbodyxK * mVbodyxK + mVbodyyK * mVbodyyK);
        }

		/* loop over the printfields */
		if (decimate > 1) {
			if (decimate_count >= decimate) {
				decimate_count = 0;
			}
			else {
				output_ok = false;
			}
		}
    	if (output_ok) {
  		  for (int i = 0; i < nprintfields; i++) {
  			const index_t index = static_cast<index_t>(printfields[i].index);
  			// TODO(schwehr): Make this a switch.
  			if (index == INDEX_ZERO) {
  				double dvalue = 0.0;
  				if (output_mode == OUTPUT_MODE_BINARY)
  					fwrite(&dvalue, sizeof(double), 1, stdout);
  				else
  					fprintf(stdout, printfields[i].format, dvalue);
  			}
  			else if (index == INDEX_MERGE_ALTITUDE) {
  				double dvalue = 0.0;
  				int jinterp = 0;
  				mb_linear_interp(verbose, alt_time_d - 1, alt_altitude - 1, alt_num, time_d, &dvalue,
  				                                           &jinterp, &error);
  				if (jinterp < 2 || jinterp > nav_num - 2)
  					dvalue = 0.0;
  				if (output_mode == OUTPUT_MODE_BINARY)
  					fwrite(&dvalue, sizeof(double), 1, stdout);
  				else
  					fprintf(stdout, printfields[i].format, dvalue);
  			}
  			else if (index == INDEX_MERGE_LON) {
  				double dvalue = 0.0;
  				int jinterp = 0;
  				mb_linear_interp_longitude(verbose, nav_time_d - 1, nav_navlon - 1, nav_num, time_d, &dvalue,
  				                                           &jinterp, &error);
  				if (jinterp < 2 || jinterp > nav_num - 2)
  					dvalue = 0.0;
  				if (output_mode == OUTPUT_MODE_BINARY)
  					fwrite(&dvalue, sizeof(double), 1, stdout);
  				else
  					fprintf(stdout, printfields[i].format, dvalue);
  			}
  			else if (index == INDEX_MERGE_LAT) {
  				double dvalue = 0.0;
  				int jinterp = 0;
  				mb_linear_interp_latitude(verbose, nav_time_d - 1, nav_navlat - 1, nav_num, time_d, &dvalue,
  				                                          &jinterp, &error);
  				if (jinterp < 2 || jinterp > nav_num - 2)
  					dvalue = 0.0;
  				if (output_mode == OUTPUT_MODE_BINARY)
  					fwrite(&dvalue, sizeof(double), 1, stdout);
  				else
  					fprintf(stdout, printfields[i].format, dvalue);
  			}
  			else if (index == INDEX_MERGE_HEADING) {
  				double dvalue = 0.0;
  				int jinterp = 0;
  				mb_linear_interp_heading(verbose, nav_time_d - 1, nav_heading - 1, nav_num, time_d, &dvalue,
  				                                         &jinterp, &error);
  				if (jinterp < 2 || jinterp > nav_num - 2)
  					dvalue = 0.0;
  				if (output_mode == OUTPUT_MODE_BINARY)
  					fwrite(&dvalue, sizeof(double), 1, stdout);
  				else
  					fprintf(stdout, printfields[i].format, dvalue);
  			}
  			else if (index == INDEX_MERGE_SPEED) {
  				double dvalue = 0.0;
  				int jinterp = 0;
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
  				double dvalue = 0.0;
  				int jinterp = 0;
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
  				double dvalue = 0.0;
  				int jinterp = 0;
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
  				double dvalue = 0.0;
  				int jinterp = 0;
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
  				double dvalue = 0.0;
  				int jinterp = 0;
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
  			else if (index == INDEX_CALC_KSPEED) {
  				if (output_mode == OUTPUT_MODE_BINARY)
  					fwrite(&kspeed_calc, sizeof(double), 1, stdout);
  				else
  					fprintf(stdout, printfields[i].format, kspeed_calc);
  			}
  			else if (index == INDEX_TIME_INTERVAL) {
  				if (output_mode == OUTPUT_MODE_BINARY)
  					fwrite(&time_interval, sizeof(double), 1, stdout);
  				else
  					fprintf(stdout, printfields[i].format, time_interval);
  			}
  			else if (fields[index].type == TYPE_DOUBLE) {
  				double dvalue = 0.0;
  				mb_get_binary_double(true, &buffer[fields[index].index], &dvalue);
  				dvalue *= fields[index].scale;
  				if ((strcmp(fields[nfields].name, "mHeadK") == 0 || strcmp(fields[nfields].name, "mYawK") == 0) &&
  				    angles_in_degrees && dvalue < 0.0)
  					dvalue += 360.0;
  				if (output_mode == OUTPUT_MODE_BINARY)
  					fwrite(&dvalue, sizeof(double), 1, stdout);
  				else
  					fprintf(stdout, printfields[i].format, dvalue);
  			}
  			else if (fields[index].type == TYPE_INTEGER) {
  				int ivalue = 0;
  				mb_get_binary_int(true, &buffer[fields[index].index], &ivalue);
  				if (output_mode == OUTPUT_MODE_BINARY)
  					fwrite(&ivalue, sizeof(int), 1, stdout);
  				else
  					fprintf(stdout, printfields[i].format, ivalue);
  			}
  			else if (fields[index].type == TYPE_SHORT) {
  				short ivalue = 0;
  				mb_get_binary_short(true, &buffer[fields[index].index], &ivalue);
  				if (output_mode == OUTPUT_MODE_BINARY)
  					fwrite(&ivalue, sizeof(short), 1, stdout);
  				else
  					fprintf(stdout, printfields[i].format, ivalue);
  			}
  			else if (fields[index].type == TYPE_TIMETAG) {
  				double dvalue = 0.0;
  				mb_get_binary_double(true, &buffer[fields[index].index], &dvalue);
  				time_d = dvalue;
  				if (strcmp(printfields[i].format, "time_i") == 0) {
  					mb_get_date(verbose, time_d, time_i);
  					if (output_mode == OUTPUT_MODE_BINARY) {
  						fwrite(time_i, sizeof(int), 7, stdout);
  					}
  					else if (output_mode == OUTPUT_MODE_CSV) {
  						fprintf(stdout, "%4.4d,%2.2d,%2.2d,%2.2d,%2.2d,%2.2d.%6.6d", time_i[0], time_i[1], time_i[2], time_i[3],
  						        time_i[4], time_i[5], time_i[6]);
            }
            else {
  						fprintf(stdout, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d", time_i[0], time_i[1], time_i[2], time_i[3],
  						        time_i[4], time_i[5], time_i[6]);
  					}
  				}
  				else if (strcmp(printfields[i].format, "time_j") == 0) {
  					mb_get_date(verbose, time_d, time_i);
  					int time_j[5];
  					mb_get_jtime(verbose, time_i, time_j);
  					if (output_mode == OUTPUT_MODE_BINARY) {
  						fwrite(&time_i[0], sizeof(int), 1, stdout);
  						fwrite(&time_j[1], sizeof(int), 1, stdout);
  						fwrite(&time_i[3], sizeof(int), 1, stdout);
  						fwrite(&time_i[4], sizeof(int), 1, stdout);
  						fwrite(&time_i[5], sizeof(int), 1, stdout);
  						fwrite(&time_i[6], sizeof(int), 1, stdout);
  					}
  					else if (output_mode == OUTPUT_MODE_CSV) {
  						fprintf(stdout, "%4.4d,%3.3d,%2.2d,%2.2d,%2.2d.%6.6d", time_i[0], time_j[1], time_i[3], time_i[4],
  						        time_i[5], time_i[6]);
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
  				double dvalue = 0.0;
  				mb_get_binary_double(true, &buffer[fields[index].index], &dvalue);
  				dvalue *= fields[index].scale;
  				if (strcmp(fields[index].name, "mYawCB") == 0 && angles_in_degrees && dvalue < 0.0)
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
	}
	fclose(fp);

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
