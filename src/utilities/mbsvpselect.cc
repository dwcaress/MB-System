/*--------------------------------------------------------------------
 *    The MB-system:	mbsvpselect.c	03.03.2014
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
 * Mbsvpselect chooses and implements the best available sound speed model
 * for each swath file in a survey. The user provides a list of the
 * available sound speed models and specifies the criteria used for
 * model selection. The program uses mbset to turn on bathymetry
 * recalculation by raytracing through the sound speed model selected
 * for each swath file.
 *
 * Author:	Ammar Aljuhne (ammaraljuhne@gmail.com)
 * Co-author:   Christian do Santos Ferreira (cferreira@marum.de)
 *              MARUM, University of Bremen
 * Date:	3 March 2014
 *
 * Description:
 *
 * The tool aims to help users to automatically apply the sound velocity
 * correction to the survey files. since most surveys involve several SVPs,
 * the seletion of the appropriate SVP for each survey profile is still
 * missing in MB-System.
 *
 * After finding the appropriate svp for each profile based on the chosen
 * method, the results are copied to a txt file that shows each survey
 * profile with the corresponding SVP. the tool also calls mbset automatically
 * so no need to assign SVP to the data. it is done automatically.
 *
 * There are 5 methods for choosing the appropriate SVP for each survey
 * profile. These methods are:
 *
 * 1. Nearest SVP in position: the middle position of each survey profile
 *    is calculated and the geodesics (shortest distance on the ellipsoid)
 *    to all SVPs are calculated. and the SVP with the shortest distance is
 *    chosen. when the middle position of the survey profile is calculated
 *    there is an option to check for 0 lat 0 long wrong values. if it is
 *    found at the starting the geodesic will be calculated to the end of
 *    the profile.
 *
 * 2. Nearest in time: the time interveal between the starting time of
 *    the profile and the time of the SVP, and the SVP with  the shortest
 *    interval will be chosen.
 *
 * 3. Nearest in position within time: a default time radius from the
 *    profile is set as 10 hours, and within this period the nearest SVP
 *    in position is chosen. if none of the SVPs are within this period the
 *    nearest in position will be taken despit of the period threshold. The
 *    period threshold can be set by the user.
 *
 * 4. Nearest in time within range: similar to the previous option but
 *    this time a default range of 10000 meters is set and within this range
 *    the svp nearest in time is chosen. also this 10000 meter value could
 *    be set by the user.
 *
 * 5. Nearest in season within range: similar to the previous option the
 *    selected SVP could be chosen based on the month only not on the year.
 *    it means within the specified range the user could chose either the svp
 *    nearest in time or the svp nearest in month (this could be interpreted
 *    as the svp that falls in the same seasonal period despite of the year
 *    when it was taken).
 *
 * Mbsvpselect reads the .inf file of each swath file referenced in a recursive
 * datalist structure to determine the location and collection time of the
 * relevant data. The ancillary *.inf, *.fbt, and *.fnv files must be created
 * first. The water sound speed models (called SVPs by convention as an acronym
 * for Sound Velocity Profiles) to be used must include one of three supported
 * file headers specifying the time and location of the model.
 *
 * University of Bremen SVP headers:
 *   MB-SVP 2011/01/08 19:30:00 -52.965437  -36.986314
 *   (keyword yyyy/mm/dd hh:mm:ss latitude longitude)
 *
 * MB-System SVPs as now output by mbsvplist:
 *   ## MB-SVP 2011/01/08 19:30:00 -36.986314 -52.965437
 *   (keyword yyyy/mm/dd hh:mm:ss longitude latitude)
 *
 * CARIS sound velocity header format:
 *   Section 2013-150 23:22:18 -57:02:01 -26:02:18
 *   (keyword yyyy-yearDay  hh:mm:ss latitude (degree:min:sec) longitude (degree:min:sec))
 *
 * Mbsvpselect supports SVP files with single models or files with multiple models where
 * new headers occur between models.
 *
 * Instructions:
 *
 * 1) Set up a survey (or surveys) for MB-System processing in the usual way,
 *    including creating a datalist file referencing the swath data of interest
 *    and generating the ancillary *.inf, *.fbt, and *.fnv files for each of
 *    the swath files.
 * 2) Create an svplist file (analogous to a datalist, but referencing the
 *    relevant SVP files). Each SVP file is expected to be a text file with
 *    depth-sound speed pairs on each line (depth in meters, sound speed in
 *    meters/second) excepting for a header line at the start of each discrete
 *    model. Any of the header formats listed above will work.
 *    that refers to a local svp datalist. the local datalists includes
 * 3) In order to turn on bathymetry recalculation by raytracing through the
 *    most appropriate sound speed model for each swath file, call mbsvpselect:
 *
 *      mbsvpselect -N -V -Idatalist -Ssvplist [-P0, -P1, -P2/period, -P3/range, -P3/range/1]
 *
 *    -N is the option to check 0 latitude 0 longitude in the survey lines.
 *    -V verbosity.
 *    -I input datalist
 *    -S input svp datalist
 *    -P the method for choosing the svp where:
 *        -P or -P0                 is the nearest in position
 *        -P1                       is the nearest in time
 *        -P2                       is nearest in position within time (default time period is 10 hours)
 *        -P2/time                  is nearest in position within specified time period (in hours)
 *        -P3                       is nearest in time within range   (default range is 10000 meters)
 *        -P3/range or -P3/range/0  is nearest in time within specified range (in meters)
 *        -P3/range/1                     is nearest in month (seasonal) within specified range in meter.
 *
 * Example
 *
 * Suppose you are working in a directory called Survey_1 containing
 * swath files that need to have the bathymetry recalculated by
 * raytracing through water sound speed models. The local datalist
 * file might contain something like:
 *      13349457_3934_2845.mb88 88
 *      13645323_3433_5543.mb88 88
 *      46372536_6563_4637.mb88 88
 *      64362825_6344_2635.mb88 88
 *
 * or, if you use absolute passwords, something like:
 *
 *      /MyMac/User/Survey_1/13349457_3934_2845.mb88 88
 *      /MyMac/User/Survey_1/13645323_3433_5543.mb88 88
 *      /MyMac/User/Survey_1/46372536_6563_4637.mb88 88
 *      /MyMac/User/Survey_1/64362825_6344_2635.mb88 88
 *
 * By convention, this datalist will be named something
 * like datalist.mb-1, where the ".mb-1" suffix indicates to
 * MB-System programs that this is a datalist file. As
 * documented elsewhere, datalist files can contain entries
 * that reference datalists rather than single files; thus
 * datalists can be recursive.
 *
 * Suppose that the water properties were variable during this
 * survey, with the variability dominated by location.Further suppose
 * that there are three SVP files in a separate directory with
 * names such as svp1.svp, svp2.svp, and svp3.svp. Each of these files
 * contains a single model derived from CTD casts at a particular
 * place and time indicated in the single header line. In that directory
 * one can create an svplist file named SVP_list.mb-1 with contents:
 *      svp1.svp
 *      svp2.svp
 *      svp3.svp
 * Since mbsvpselect allows svplists to be recursive (like datalists),
 * one can create a second svplist named my_svplist.mb-1in the survey
 * processing directory that references the first with an entry like:
 *
 *      /MyMac/User/Survey_1/SVP_folder/SVP_list.mb-1 -1
 *
 * In order to turn on bathymetry recalculation for all of the
 * swath files referenced by datalist.mb-1 using the most appropriate
 * of the available sound speed models, run mbsvpselect with arguments
 * like:
 *
 *      mbsvpselect -N -V -I datalist.mb-1 -S my_svplist.mb-1 -P2/50
 *
 * Here the -P2/50 option specifies that the sound speed model to be
 * used for each file will be the closest one collected within 50 hours
 * of the swath data. The bathymetry recalculation will be turned on
 * using an mbset call of the form:
 *
 *      mbset -Idatalist.mb-1 -PSVPFILE:/MyMac/User/Survey_1/SVP_folder/svp1.svp
 *
 * Following the mbsvpselect usage, mbprocess must be run to actually
 * reprocess the swath data, including bathymetry recalculation by
 * raytracing.
 *
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <getopt.h>
#include <unistd.h>

#include <geodesic.h>

#include "mb_define.h"
#include "mb_status.h"

/* struct info_holder (shortly inf) hold the information from auxiliary files .inf
 * that are created from mbdatalist command */

struct info_holder {
	int flag;  // TODO(schwehr): Make this an enum
	char *file_name;
	long double s_lat;
	long double s_lon;
	double e_sec;
	long double e_lat;
	long double e_lon;
	long double ave_lat;
	long double ave_lon;
	struct tm s_datum_time;
	struct tm e_datum_time;
	struct tm ave_datum_time;
	time_t s_Time;
	time_t e_time;
};
typedef struct info_holder inf;

/* struct svp_holder (shortly svp) hold the information from auxiliary files .inf
 * that are created from mbdatalist command */
struct svp_holder {
	char *file_name;
	long double s_lat;
	long double s_lon;
	struct tm svp_datum_time;
	time_t svp_Time;
};
typedef struct svp_holder svp;


/* global variables */
// int counter_i_i2 = 0;
int p_flag = 0;  // TODO(schwehr): Make this an enum
int p_3_time = 10;
int p_4_range = 10000;
int p_4_flage = 0;
int zero_test = 0;
int size_2 = 0;
int n_p2 = 0;
char dHolder[1000][1000];
char sdHolder[1000][1000];
char svps[1000][1000];
char swaths[1000][1000];
int verbose = 0;
char holder[1000][1000];   /* copy the buffer into holder array for indexing */
char holder_2[1000][1000]; /* copy the buffer into holder array for indexing */
char holder_3[1000][1000];
char holder_4[1000][1000];
char dBuffer[BUFSIZ];
char sdBuffer[BUFSIZ];
char buffer[BUFSIZ]; /* String to hold the file name */
char buffer_2[BUFSIZ];
int svp_total = 0;
int surveyLines_total = 0;

constexpr char program_name[] = "mbsvpselect";
constexpr char help_message[] =
    "Program mbsvpselect chooses and implements the best available sound speed\n"
    "model for each swath file in a survey. The user provides a list of the\n"
    "available sound speed models and specifies the criteria used for\n"
    "model selection. The program uses mbset to turn on bathymetry\n"
    "recalculation by raytracing through the sound speed model selected\n"
    "for each swath file.";
constexpr char usage_message[] =
    "mbsvpselect -H -N -Idatalist -Ssvplist "
    "[-P0, -P1, -P2/period, -P3/range, -P3/range/1]  -V";

/* ---------------------------------------------------------------- */
/* Is leap old */
bool Is_Leap(int year) {
	if (year % 400 == 0)
		return false;
	else if (year % 100 == 0)
		return true;
	else if (year % 4 == 0)
		return false;
	return true;
}
/* ---------------------------------------------------------------- */
/*
 * calc_ave_dateTime function fill the struct with average date and time. it takes the starting date
 *of the file and the end date of the file and then calculate the diffeence in  seconds
 * then it transform the sec into time format and add it to the starting date
 *
 void calc_ave_dateTime(inf *inf_hold)
 {
 //time_t *temp1 = &inf_hold->s_Time;
 //time_t *temp2 = &inf_hold->e_time;
 double s = difftime(inf_hold->s_Time, inf_hold->e_time);
 if(s>86400.0)
 {
 printf("\n\n\n\n%f\n\n\n", s);
 }
 else
 {
 printf("\n\n\n\n%f\n\n\n", s);
 }
 }*/
/* ---------------------------------------------------------------- */
void JulianToGregorian(int year, int yearDay, int *year_tm, int *month, int *wDay) {
	*year_tm = year - 1900;
	if (!Is_Leap(year)) {
		if (yearDay > 335) {
			*month = 11;
			*wDay = yearDay - 335;
		}
		if ((yearDay > 305) && (yearDay <= 335)) {
			*month = 10;
			*wDay = yearDay - 305;
		}
		if ((yearDay > 274) && (yearDay <= 305)) {
			*month = 9;
			*wDay = yearDay - 274;
		}
		if ((yearDay > 244) && (yearDay <= 274)) {
			*month = 8;
			*wDay = yearDay - 244;
		}
		if ((yearDay > 213) && (yearDay <= 244)) {
			*month = 7;
			*wDay = yearDay - 213;
		}
		if ((yearDay > 182) && (yearDay <= 213)) {
			*month = 6;
			*wDay = yearDay - 182;
		}
		if ((yearDay > 152) && (yearDay <= 182)) {
			*month = 5;
			*wDay = yearDay - 152;
		}
		if ((yearDay > 121) && (yearDay <= 152)) {
			*month = 4;
			*wDay = yearDay - 121;
		}
		if ((yearDay > 91) && (yearDay <= 121)) {
			*month = 3;
			*wDay = yearDay - 91;
		}
		if ((yearDay > 60) && (yearDay <= 91)) {
			*month = 2;
			*wDay = yearDay - 60;
		}
		if ((yearDay > 31) && (yearDay <= 60)) {
			*month = 1;
			*wDay = yearDay - 274;
		}
		if (yearDay < 31) {
			*month = 0;
			*wDay = yearDay;
		}
	}
	else {
		if (yearDay > 334) {
			*month = 11;
			*wDay = yearDay - 334;
		}
		if ((yearDay > 304) && (yearDay <= 334)) {
			*month = 10;
			*wDay = yearDay - 304;
		}
		if ((yearDay > 273) && (yearDay <= 304)) {
			*month = 9;
			*wDay = yearDay - 273;
		}
		if ((yearDay > 243) && (yearDay <= 273)) {
			*month = 8;
			*wDay = yearDay - 243;
		}
		if ((yearDay > 212) && (yearDay <= 243)) {
			*month = 7;
			*wDay = yearDay - 212;
		}
		if ((yearDay > 181) && (yearDay <= 212)) {
			*month = 6;
			*wDay = yearDay - 181;
		}
		if ((yearDay > 151) && (yearDay <= 181)) {
			*month = 5;
			*wDay = yearDay - 151;
		}
		if ((yearDay > 120) && (yearDay <= 151)) {
			*month = 4;
			*wDay = yearDay - 120;
		}
		if ((yearDay > 90) && (yearDay <= 120)) {
			*month = 3;
			*wDay = yearDay - 90;
		}
		if ((yearDay > 59) && (yearDay <= 90)) {
			*month = 2;
			*wDay = yearDay - 59;
		}
		if ((yearDay > 31) && (yearDay <= 59)) {
			*month = 1;
			*wDay = yearDay - 31;
		}
		if (yearDay < 31) {
			*month = 0;
			*wDay = yearDay;
		}
	}
} /* JulianToGregorian */
/* ---------------------------------------------------------------- */
void GregorianToJulian(int year, int month, int day, int *yearDay) {
	if (Is_Leap(year))
		switch (month) {
		case 0:
			*yearDay = day;
			break;
		case 1:
			*yearDay = day + 31;
			break;
		case 2:
			*yearDay = day + 59;
			break;
		case 3:
			*yearDay = day + 90;
			break;
		case 4:
			*yearDay = day + 120;
			break;
		case 5:
			*yearDay = day + 151;
			break;
		case 6:
			*yearDay = day + 181;
			break;
		case 7:
			*yearDay = day + 212;
			break;
		case 8:
			*yearDay = day + 243;
			break;
		case 9:
			*yearDay = day + 273;
			break;
		case 10:
			*yearDay = day + 304;
			break;
		case 11:
			*yearDay = day + 334;
			break;

		default:
			break;
		} /* switch */
	else
		switch (month) {
		case 0:
			*yearDay = day;
			break;
		case 1:
			*yearDay = day + 31;
			break;
		case 2:
			*yearDay = day + 60;
			break;
		case 3:
			*yearDay = day + 91;
			break;
		case 4:
			*yearDay = day + 121;
			break;
		case 5:
			*yearDay = day + 152;
			break;
		case 6:
			*yearDay = day + 182;
			break;
		case 7:
			*yearDay = day + 213;
			break;
		case 8:
			*yearDay = day + 244;
			break;
		case 9:
			*yearDay = day + 274;
			break;
		case 10:
			*yearDay = day + 305;
			break;
		case 11:
			*yearDay = day + 335;
			break;

		default:
			break;
		} /* switch */
} /* GregorianToJulian */
/* --------------------------------------------------------------- */
/* calculate the average position of two points */
/* http://www.movable-type.co.uk/scripts/latlong.html */
void mid_point(long double lat1, long double lon1, long double lat2, long double lon2, long double *lat3, long double *lon3) {
	const double dLon = DTR * ((lon2) - (lon1));
	const double lat1_rad = DTR * ((lat1));
	const double lat2_rad = DTR * ((lat2));
	const double lon1_rad = DTR * ((lon1));
	const double bx = cos(lat2_rad) * cos(dLon);
	const double by = cos(lat2_rad) * sin(dLon);
	*(lat3) = atan2(sin(lat1_rad) + sin(lat2_rad), sqrt((((cos(lat1_rad)) + bx) * ((cos(lat1_rad)) + bx)) + (by * by))) * RTD;
	*(lon3) = (lon1_rad + atan2(by, (cos(lat1_rad) + bx))) * RTD;
}
/* ---------------------------------------------------------------- */
/*this function fills the inf struct with the appropriate values
 * it takes a pointer to the inf_struct and the file_name (file_name.inf) to read information from
 */
void fill_struct_inf(inf *inf_hold, char *holder) {
	inf_hold->flag = 0;
	inf_hold->file_name = holder;

	/* reading relative inf file */
	FILE *fileName = fopen(inf_hold->file_name, "r");
	if (fileName == nullptr) {
		printf("%s could not be opened Please check the datalist files\n", inf_hold->file_name);
		exit(1);
	}

	/* reaching start of data key word */
	while ((fgets(buffer, sizeof buffer, fileName)) != nullptr)
		if (strcmp(buffer, "Start of Data:\n") == 0)
			break;

	/* parsing date and time*/
	fgets(buffer, sizeof buffer, fileName);

	int mon;
	int year;
	double s_sec;
	sscanf(buffer, "%*s %d %d %d %i:%i:%lf %*s %*s", &mon, &inf_hold->s_datum_time.tm_mday, &year,
	       &inf_hold->s_datum_time.tm_hour, &inf_hold->s_datum_time.tm_min, &s_sec);
	inf_hold->s_datum_time.tm_mon = mon - 1;
	inf_hold->s_datum_time.tm_year = year - 1900;
	s_sec = floor(s_sec);
	inf_hold->s_datum_time.tm_sec = (int)s_sec;
	GregorianToJulian(inf_hold->s_datum_time.tm_year, inf_hold->s_datum_time.tm_mon, inf_hold->s_datum_time.tm_mday,
	                  &inf_hold->s_datum_time.tm_yday);

	struct tm *ptr = &inf_hold->s_datum_time;
	inf_hold->s_Time = mktime(ptr);

	/* Lon and Lat processing*/

	fgets(buffer, sizeof buffer, fileName);

	sscanf(buffer, "%*s %Lf %*s %Lf %*s %*f %*s", &inf_hold->s_lon, &inf_hold->s_lat);

	/*parsing end of data information*/

	while ((fgets(buffer, sizeof buffer, fileName)) != nullptr)
		if (strcmp(buffer, "End of Data:\n") == 0)
			break;

	fgets(buffer, sizeof buffer, fileName);

	sscanf(buffer, "%*s %d %d %d %i:%i:%lf %*s %*s", &mon, &inf_hold->e_datum_time.tm_mday, &year,
	       &inf_hold->e_datum_time.tm_hour, &inf_hold->e_datum_time.tm_min, &s_sec);

	inf_hold->e_datum_time.tm_mon = mon - 1;
	inf_hold->e_datum_time.tm_year = year - 1900;
	s_sec = floor(s_sec);
	inf_hold->e_datum_time.tm_sec = (int)s_sec;

	struct tm *ptr2 = &inf_hold->e_datum_time;
	inf_hold->e_time = mktime(ptr2);

	/* calc_ave_dateTime(inf_hold); */

	/* Lon and Lat processing */
	fgets(buffer, sizeof buffer, fileName);

	sscanf(buffer, "%*s %Lf %*s %Lf %*s %*f %*s", &inf_hold->e_lon, &inf_hold->e_lat);
	if (zero_test > 0) {
		if ((inf_hold->s_lat == 0.0) && (inf_hold->s_lon == 0.0)) {
			// TODO(schwehr): Was there supposed to be a _lat?
			if (inf_hold->e_lon == 0.0 /* && inf_hold->e_lon == 0.0 */ )
				inf_hold->flag = 3;
			else
				inf_hold->flag = 1;
		}
		else {
			// TODO(schwehr): Was there supposed to be a _lat?
			if (inf_hold->e_lon == 0.0 /* && (inf_hold->e_lon == 0.0 */ )
				inf_hold->flag = 2;
			else
				inf_hold->flag = 0;
		}
	}

	mid_point(inf_hold->s_lat, inf_hold->s_lon, inf_hold->e_lat, inf_hold->e_lon, &inf_hold->ave_lat, &inf_hold->ave_lon);

	fclose(fileName);
} /* fill_struct_inf */
/* ---------------------------------------------------------------- */
/*
 * convert_decimal function
 * convert lat or lon from deg(int):min(int):sec(int) to decimal format
 */
double convert_decimal(int deg, int min, int sec) {
	if (deg >= 0)
		return (double)deg + (((double)min) / 60) + (((double)sec) / 3600);

	return -(fabs((double)deg) + (((double)min) / 60) + (((double)sec) / 3600));
}
/* ------------------------------------------------------------------- */
/* SVP tool is able to read only two SVP formats
 *
 *
 */

/* ------------------------------------------------------------------- */
void fill_struct_svp(svp *svp_hold, char *holder) {
	svp_hold->file_name = holder;

	/* reading relative svp file */
	FILE *fileName = fopen(svp_hold->file_name, "r");
	if (fileName == nullptr) {
		printf("%s could not be opend\n", svp_hold->file_name);
		exit(1);
	}

	int yearDay;
	int month;
	int year;
	double seconds;
	int s_lat_min = 0;
	int s_lat_deg = 0;
	int s_lat_sec = 0;
	int s_lon_min = 0;
	int s_lon_deg = 0;
	int s_lon_sec = 0;
	char caris_str[] = "Section";
	char mb1_str[] = "## MB-SVP";
	char mb2_str[] = "# MB-SVP";
	char *ptr_caris = nullptr;
	char *ptr_mb1 = nullptr;
	char *ptr_mb2 = nullptr;

	/* reaching start of data */
	while ((fgets(buffer, sizeof buffer, fileName)) != nullptr) {
		ptr_caris = strstr(buffer, caris_str);
		ptr_mb1 = strstr(buffer, mb1_str);
		ptr_mb2 = strstr(buffer, mb2_str);
		if (ptr_caris != nullptr) {
			printf("\n%s\n", buffer);
			sscanf(buffer, "%*s %d-%d  %i:%i:%i %d:%d:%d %d:%d:%d", &year, &yearDay, &svp_hold->svp_datum_time.tm_hour,
			       &svp_hold->svp_datum_time.tm_min, &svp_hold->svp_datum_time.tm_sec, &s_lat_deg, &s_lat_min, &s_lat_sec,
			       &s_lon_deg, &s_lon_min, &s_lon_sec);
			svp_hold->svp_datum_time.tm_year = year - 1900;
			svp_hold->svp_datum_time.tm_yday = yearDay;
			JulianToGregorian(year, yearDay, &svp_hold->svp_datum_time.tm_year, &svp_hold->svp_datum_time.tm_mon,
			                  &svp_hold->svp_datum_time.tm_mday);
			/* Julian to Gregorian date */
			/* svp_hold->svp_datum_time.tm_yday = yearDay - 1; */
			/* svp_hold->svp_datum_time.tm_year = year-1900; */

			struct tm *ptr1 = &svp_hold->svp_datum_time;
			svp_hold->svp_Time = mktime(ptr1);

			/* latitude to decimal */
			svp_hold->s_lat = convert_decimal(s_lat_deg, s_lat_min, s_lat_sec);
			/* longitude to decimal */
			svp_hold->s_lon = convert_decimal(s_lon_deg, s_lon_min, s_lon_sec);
			break;
		}
		else if (ptr_mb1 != nullptr) {
			printf("\n%s\n", buffer);
			sscanf(buffer, "## MB-SVP %d/%d/%d %d:%d:%lf %Lf %Lf", &year, &month, &svp_hold->svp_datum_time.tm_mday,
			       &svp_hold->svp_datum_time.tm_hour, &svp_hold->svp_datum_time.tm_min, &seconds, &svp_hold->s_lon,
			       &svp_hold->s_lat);

			svp_hold->svp_datum_time.tm_mon = month - 1;
			svp_hold->svp_datum_time.tm_year = year - 1900;
			svp_hold->svp_datum_time.tm_sec = seconds;
			GregorianToJulian(svp_hold->svp_datum_time.tm_year, svp_hold->svp_datum_time.tm_mon, svp_hold->svp_datum_time.tm_mday,
			                  &svp_hold->svp_datum_time.tm_yday);
			struct tm *ptr = &svp_hold->svp_datum_time;
			svp_hold->svp_Time = mktime(ptr);

			break;
		}
		else if (ptr_mb2 != nullptr) {
			printf("\n%s\n", buffer);
			sscanf(buffer, "%*s %*s %d/%d/%d %d:%d:%d %Lf %Lf", &year, &month, &svp_hold->svp_datum_time.tm_mday,
			       &svp_hold->svp_datum_time.tm_hour, &svp_hold->svp_datum_time.tm_min, &svp_hold->svp_datum_time.tm_sec,
			       &svp_hold->s_lon, &svp_hold->s_lat);

			svp_hold->svp_datum_time.tm_mon = month - 1;
			svp_hold->svp_datum_time.tm_year = year - 1900;
			GregorianToJulian(svp_hold->svp_datum_time.tm_year, svp_hold->svp_datum_time.tm_mon, svp_hold->svp_datum_time.tm_mday,
			                  &svp_hold->svp_datum_time.tm_yday);
			struct tm *ptr = &svp_hold->svp_datum_time;
			svp_hold->svp_Time = mktime(ptr);

			break;
		}
	}

	fclose(fileName);
} /* fill_struct_svp */
/* --------------------------------------------------------------- */
/*copy string and handle possible buffer overlap*/
char *my_strcpy(char *a, char *b) {
	if (a == nullptr || b == nullptr) {
		return nullptr;
	}

	memmove(a, b, strlen(b) + 1);
	return a;
}
/* ---------------------------------------------------------------- */
/*
 *  Function trim_newline
 *	Delete the '\n' char from string
 */
void trim_newline(char string[]) {
	if (string[strlen(string) - 1] == '\n')
		string[strlen(string) - 1] = '\0';
}
/*---------------------------------------------------------------------*/
int read_recursive2(char *fname) {
	char original[1024] = {""};
	my_strcpy(original, fname);
	const char *result = fname;
	int counter = 0;
	trim_newline(fname);
	strcat(fname, ".inf");
	FILE *dataFile = fopen(fname, "r");
	if (dataFile != nullptr) {
		my_strcpy(holder[surveyLines_total], fname);
		counter += 1;
		surveyLines_total += 1;
		fclose(dataFile);
		return counter;
	}

	const char *ret = strchr(result, ' ');
	if (ret == nullptr) {
		char file2[1024] = {""};
		my_strcpy(file2, original);
		trim_newline(file2);
		FILE *dataFile2 = fopen(file2, "r");
		if (dataFile2 == nullptr) {
			printf("Could not open the file %s", file2);
			return counter;
		}
		while ((fgets(dBuffer, sizeof dBuffer, dataFile2)) != nullptr) {
			//char strHolder[strlen(original)];		INVALID JL
			char strHolder[1024];
			my_strcpy(strHolder, original);
			while (strHolder[strlen(strHolder) - 1] != '/')
				strHolder[strlen(strHolder) - 1] = 0;
			strcat(strHolder, dBuffer);
			int tmp = read_recursive2(strHolder);
			if (tmp == 0) {
				tmp = read_recursive2(dBuffer);
			}
			// counter +=tmp;
		}
		fclose(dataFile2);
	}
	else {
		char strHolder[1024];
		my_strcpy(strHolder, original);
		while (strHolder[strlen(strHolder) - 1] != ' ')
			strHolder[strlen(strHolder) - 1] = 0;
		strHolder[strlen(strHolder) - 1] = 0;
		const int tmp2 = read_recursive2(strHolder);
		counter += tmp2;
	}

	return counter;
}
/*---------------------------------------------------------------------*/
int read_recursive(char *fileName) {
	trim_newline(fileName);
	FILE *dataFile = fopen(fileName, "r");
	if (dataFile == nullptr) {
		printf("Could not open the file %s", fileName);
		return 0;
	}

	char str[BUFSIZ];
	fgets(str, sizeof str, dataFile);
	trim_newline(dBuffer);
	// initialize the end condition for the svps
	const char caris_str[] = "Section";
	const char *ptr_caris = strstr(str, caris_str);
	const char mb1_str[] = "## MB-SVP";
	const char *ptr_mb1 = strstr(str, mb1_str);
	const char mb2_str[] = "MB-SVP";
	const char *ptr_mb2 = strstr(str, mb2_str);

	// initialize the end condition for the swaths

	int counter = 0;
	if ((ptr_caris != nullptr) || (ptr_mb1 != nullptr) || (ptr_mb2 != nullptr)) {
		my_strcpy(svps[svp_total], fileName);
		counter += 1;
	} else {
		read_recursive(str);
	}
	fclose(dataFile);

	return counter;
}
/* ---------------------------------------------------------------- */
/* print the inf information on the screen */
void print_inf(inf *cd) {
	struct tm *temp = &cd->s_datum_time;
	puts("==================================================");
	printf("file_name: %s\n", cd->file_name);
	puts("starting Date and time");
	printf("\n%s\n", asctime(temp));

	temp = &cd->e_datum_time;
	puts("ending Date and time");
	printf("\n%s\n", asctime(temp));
	puts("Start position");
	printf("lat: %Lf\t", cd->s_lat);
	printf("lon: %Lf\n", cd->s_lon);
	puts("End position");
	printf("e_lat: %Lf\t", cd->e_lat);
	printf("e_lon: %Lf\n", cd->e_lon);
	puts("Average position");
	printf("ave_lat: %Lf\t", cd->ave_lat);
	printf("ave_lon: %Lf\n", cd->ave_lon);
	puts("==================================================");
}
/* --------------------------------------------------------------- */
/* print the svp information on the screen */
void print_svp(svp *cd) {
	struct tm *temp = &cd->svp_datum_time;
	puts("==================================================");
	printf("file_name: %s\n", cd->file_name);
	puts("Date and time");
	printf("\n%s\n", asctime(temp));
	puts("position");
	printf("lat: %Lf\t", cd->s_lat);
	printf("lon: %Lf\n", cd->s_lon);
	puts("==================================================");
}

/* ---------------------------------------------------------------- */
/*
 * pause the screen at the exit of the program
 */
void pause_screen() {
	printf("\nEnd the program press ENTER");
	// TODO(schwehr): Undefine behavior.  Was fflush on stdout intended?
	// fflush(stdin);
	getchar();
}
/* ------------------------------------------------------------------- */
void read_list(char *list, char *list_2) {
	atexit(pause_screen); /* pause the screen */

	/* open datalist.mb-1 for names of the files */
	FILE *fDatalist = fopen(list, "r");
	if (fDatalist == nullptr) {
		printf("%s Could not be found", list);
		exit(1);
	}

	FILE *fSvp = fopen(list_2, "r");
	if (fSvp == nullptr) {
		printf("%s Could not be found", list_2);
		exit(1);
	}
	FILE *fresult = fopen("result.txt", "w+");
	if (fresult == nullptr) {
		printf("result.txt could not be found");
		exit(1);
	}
	// int count_size2;
	/* ------------------------------ */
	while ((fgets(dBuffer, sizeof dBuffer, fDatalist)) != nullptr) {
		/* count_size2 = */ read_recursive2(dBuffer);
	}

	/* ------------------------------ */

	/* fill size */

	/* Allocate memory for inf_struct */
	inf *inf_hold = static_cast<inf *>(malloc((surveyLines_total) * sizeof(inf)));
	if (inf_hold == nullptr) {
		printf("no memory for the process end of process");
		exit(1);
	}
	int size = surveyLines_total;
	for (int i = 0; i < surveyLines_total; i++) {
		fill_struct_inf(&inf_hold[i], holder[i]);
		if (verbose == 1)
			print_inf(&inf_hold[i]);
	}

	/* reset for svp_hold */
	/* ------------------------ */

	while ((fgets(sdBuffer, sizeof sdBuffer, fSvp) != nullptr)) {
		int count_size = read_recursive(sdBuffer);
		svp_total += count_size;
	}
	/* ------------------------ */
	printf("\n\n\n%d svp to be read\n\n\n", svp_total);
	/* fill size of svp_list */
	size_2 = svp_total;

	/* Allocate memory for svp_struct */
	svp *svp_hold = static_cast<svp *>(malloc((svp_total) * sizeof(svp)));
	if (svp_hold == nullptr) {
		printf("no memory for the process end of process");
		exit(1);
	}
#ifdef _WIN32
	int hour_hold[100][100];	// Have no idea if it's enough JL
	int min_hold[100][100];
	int day_hold[100][100];
#else
	int hour_hold[size][size_2];	// INVALID STANDARD C
	int min_hold[size][size_2];
	int day_hold[size][size_2];
#endif
	for (int i = 0; i < size_2; i++) {
		fill_struct_svp(&svp_hold[i], svps[i]);
		if (verbose == 1)
			print_svp(&svp_hold[i]);
	}

	/* calculating the distances and choose the appropriate file */
	if (p_flag == 0)
		printf("\n Method chosen is %d nearest in position\n", p_flag);
	if (p_flag == 1)
		printf("\n Method chosen is %d nearest in time\n", p_flag);
	if (p_flag == 2) {
		printf("\n Method chosen is %d nearest in position within time\n", p_flag);
		if (n_p2 == 1)
			printf("\n No specific time period was entered and the default time period %d hours will be taken\n", p_3_time);
		if (n_p2 == 2)
			printf("\n Time period %d hours will be taken\n", p_3_time);
	}
	if (p_flag == 3) {
		printf("\n Method chosen is %d nearest in time within range\n", p_flag);
		printf("\n range  %d meters will be taken\n", p_4_range);
		if (p_4_flage == 0)
			printf("\n Option 0 was chosen. The nearest in time within range will be calculated\n");

		if (p_4_flage == 1)
			printf("\n Option 1 was chosen. The nearest in month within range will be calculated. This will calculate within the "
			       "specified range the SVP with the nearest month to the profile regardless of the year. This is the seasonal "
			       "interpretation \n");
	}
	int n = 0;
	struct geod_geodesic g;
	double azi1, azi2;

  // WGS84 ellipsoid parameters
  const double radius_equatorial = 6378137.0;
  const double radius_polar = 6356752.314245;
  const double flattening = 0.00335281066;

	geod_init(&g, radius_equatorial, flattening);
	size = surveyLines_total;
	for (int i = 0; i < size; i++) {
#ifdef _WIN32
		double dist[100][100];				// Have no idea if it's enough JL
		double time_hold[100][100];
#else
		double dist[size][size_2];
		double time_hold[size][size_2];
#endif
		char all_in_sys[BUFSIZ] = "mbset";
		if (p_flag == 0) {
			switch (inf_hold[i].flag) {
			case 0:
			{
				if (verbose == 1) {
					puts("\n\n========N check passed no 0.0 position was found===========\n\n");
					printf("\nCalculating the distances to all svp profiles for %s\n", inf_hold[i].file_name);
				}
				double temp_dist = 0.0;
				for (int j = 0; j < size_2; j++) {
					geod_inverse(&g, inf_hold[i].ave_lat, inf_hold[i].ave_lon, svp_hold[j].s_lat, svp_hold[j].s_lon, &dist[0][j],
					             &azi1, &azi2);
					if (j == 0) {
						temp_dist = dist[0][j];
					}
					if (temp_dist >= dist[0][j]) {
						temp_dist = dist[0][j];
						n = j;
					}
					if (verbose == 1)
						printf("Distance number %d is : %lf\n", j, dist[0][j]);
				}
				if (verbose == 1) {
					printf("\nSearching for the SVP with nearest position\n");
					printf("the shortest distance is number %d from the list\n", n);
					puts("==================================================");
				}

				fprintf(fresult, "%s\n", "============================================================");
				fprintf(fresult, "%s\t", inf_hold[i].file_name);
				fprintf(fresult, "%s\n", svp_hold[n].file_name);
				fprintf(fresult, "%s\n", "=============================================================");
				printf("Calling mbset\n");
				/* printf("%s\n", all_in_sys); */
				strcat(all_in_sys, " -I ");
				inf_hold[i].file_name[strlen(inf_hold[i].file_name) - 1] = '\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name) - 1] = '\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name) - 1] = '\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name) - 1] = '\0';
				strcat(all_in_sys, inf_hold[i].file_name);
				/* printf("%s\n", all_in_sys); */
				strcat(all_in_sys, " -PSVPFILE:");
				strcat(all_in_sys, svp_hold[n].file_name);
				printf("%s\n", all_in_sys);
				/* int shellstatus = */ system(all_in_sys);
				break;
			}
			case 1:
			{
				if (verbose == 1) {
					puts("\n\n=====================N check:   0.0 position was found=====================\n\n");
					printf("\nThe file %s has no navigation information at the start position and the svp profile will be "
					       "assigned to the end point of the file\n",
					       inf_hold[i].file_name);
				}
				double temp_dist = 0.0;
				for (int j = 0; j < size_2; j++) {
					geod_inverse(&g, inf_hold[i].s_lat, inf_hold[i].s_lon, svp_hold[j].s_lat, svp_hold[j].s_lon, &dist[0][j],
					             &azi1, &azi2);
					if (j == 0) {
						temp_dist = dist[0][j];
					}
					if (temp_dist >= dist[0][j]) {
						temp_dist = dist[0][j];
						n = j;
					}
					if (verbose == 1)
						printf("Distance number %d is : %lf\n", j, dist[0][j]);
				}
				if (verbose == 1) {
					printf("\nSearching for the SVP with nearest position\n");
					printf("the shortest distance is number %d from the list\n", n);
					puts("==================================================");
				}

				fprintf(fresult, "%s\n", "============================================================");
				fprintf(fresult, "%s\t", inf_hold[i].file_name);
				fprintf(fresult, "%s\n", svp_hold[n].file_name);
				fprintf(fresult, "%s\n", "=============================================================");
				printf("Building the parameters to call mbset\n");
				/* printf("%s\n", all_in_sys); */
				strcat(all_in_sys, " -I ");
				inf_hold[i].file_name[strlen(inf_hold[i].file_name) - 1] = '\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name) - 1] = '\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name) - 1] = '\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name) - 1] = '\0';
				strcat(all_in_sys, inf_hold[i].file_name);
				/* printf("%s\n", all_in_sys); */
				strcat(all_in_sys, " -P ");
				strcat(all_in_sys, svp_hold[n].file_name);
				printf("%s\n", all_in_sys);
				/* int shellstatus = */ system(all_in_sys);
				break;
			}
			case 2:
			{
				if (verbose == 1) {
					puts("\n\n==============N check:   0.0 position was found===================\n\n");
					printf("\nThe file %s has no navigation information at the end position and the svp profile will be assigned "
					       "to the start point of the file\n",
					       inf_hold[i].file_name);
				}
				double temp_dist = 0.0;
				for (int j = 0; j < size_2; j++) {
					geod_inverse(&g, inf_hold[i].e_lat, inf_hold[i].e_lon, svp_hold[j].s_lat, svp_hold[j].s_lon, &dist[0][j],
					             &azi1, &azi2);
					if (j == 0) {
						temp_dist = dist[0][j];
					}
					if (temp_dist >= dist[0][j]) {
						temp_dist = dist[0][j];
						n = j;
					}
					if (verbose == 1)
						printf("Distance number %d is : %lf\n", j, dist[0][j]);
				}
				if (verbose == 1) {
					printf("\nSearching for the SVP with nearest position\n");
					printf("the shortest distance is number %d from the list\n", n);
					puts("==================================================");
				}
				fprintf(fresult, "%s\n", "============================================================");
				fprintf(fresult, "%s\t", inf_hold[i].file_name);
				fprintf(fresult, "%s\n", svp_hold[n].file_name);
				printf("Building the parameters to call mbset\n");
				/* printf("%s\n", all_in_sys); */
				strcat(all_in_sys, " -I ");
				inf_hold[i].file_name[strlen(inf_hold[i].file_name) - 1] = '\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name) - 1] = '\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name) - 1] = '\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name) - 1] = '\0';
				strcat(all_in_sys, inf_hold[i].file_name);
				/* printf("%s\n", all_in_sys); */
				strcat(all_in_sys, " -P ");
				strcat(all_in_sys, svp_hold[n].file_name);
				printf("%s\n", all_in_sys);
				/* int shellstatus = */ system(all_in_sys);
				fprintf(fresult, "%s\n", "=============================================================");
				break;
			}
			case 3:
			{
				if (verbose == 1) {
					puts("\n\n==============N check:   0.0 position was found====================\n\n");
					printf("\n!!!The file %s has no navigation information and no svp will be assigned to it!!!\n",
					       inf_hold[i].file_name);
					fprintf(fresult, "%s\n", "============================================================");
					fprintf(fresult, "%s\t", inf_hold[i].file_name);
					fprintf(fresult, "%s\n", "NaN");
				}
				break;
			}

			default:
				break;
			} /* switch */
		} else {
			if (p_flag == 1) /* calculate the nearest in time */
			{
				if (verbose == 1) {
					puts("==================================================");
					printf("\nCalculating the nearest svp in time for for %s\n", inf_hold[i].file_name);
				}
				double temp_time = 0;
				for (int j = 0; j < size_2; j++) {
					time_hold[0][j] = fabs(difftime(inf_hold[i].s_Time, svp_hold[j].svp_Time));
					if (j == 0) {
						temp_time = time_hold[0][j];
					}
					if (temp_time >= time_hold[0][j]) {
						temp_time = time_hold[0][j];
						n = j;
					}
					if (verbose == 1)
						printf("Time difference number %d is : %lf\n", j, time_hold[0][j]);
				}
				if (verbose == 1) {
					printf("\nSearch for the SVP that is the nearest in Time\n");
					printf("the shortest time interval is time difference number %d\n", n);
					puts("==================================================");
				}

				fprintf(fresult, "%s\n", "============================================================");
				fprintf(fresult, "%s\t", inf_hold[i].file_name);
				fprintf(fresult, "%s\n", svp_hold[n].file_name);
				fprintf(fresult, "%s\n", "=============================================================");
				printf("Building the parameters to call mbset\n");
				/* printf("%s\n", all_in_sys); */
				strcat(all_in_sys, " -I ");
				inf_hold[i].file_name[strlen(inf_hold[i].file_name) - 1] = '\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name) - 1] = '\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name) - 1] = '\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name) - 1] = '\0';
				strcat(all_in_sys, inf_hold[i].file_name);
				/* printf("%s\n", all_in_sys); */
				strcat(all_in_sys, " -PSVPFILE:");
				strcat(all_in_sys, svp_hold[n].file_name);
				printf("%s\n", all_in_sys);
				/* int shellstatus = */ system(all_in_sys);
			}
			/************calculate the nearest in position within time***************************/
			if (p_flag == 2) {
				if (verbose == 1) {
					puts("==================================================");
					printf("\nCalculating the nearest svp in position within %d time period for for %s\n", p_3_time,
					       inf_hold[i].file_name);
				}
				double temp_dist = 0.0;
				double temp_dist2 = 0.0;
				int count = 0;
				int n_pos_time = 0;
				int n_pos = 0;
				for (int j = 0; j < size_2; j++) {
					time_hold[0][j] = fabs(difftime(inf_hold[i].s_Time, svp_hold[j].svp_Time)) - (p_3_time * 3600);

					/* dist[i][j] = distVincenty(inf_hold[i].ave_lat, inf_hold[i].ave_lon,
					 svp_hold[j].s_lat, svp_hold[j].s_lon); */
					geod_inverse(&g, inf_hold[i].ave_lat, inf_hold[i].ave_lon, svp_hold[j].s_lat, svp_hold[j].s_lon, &dist[0][j],
					             &azi1, &azi2);
					if (verbose == 1)
						printf("Time difference number %d is : %lf\n", j, time_hold[0][j]);
					printf("position difference number %d is : %lf\n", j, dist[0][j]);
					if (time_hold[0][j] < 0) {
						count = 1;
						if (j == 0 || temp_dist == 0) {
							temp_dist = dist[0][j];
							n_pos_time = j;
						}
						else {
							if (temp_dist >= dist[0][j]) {
								temp_dist = dist[0][j];
								n_pos_time = j;
							}
						}
					}
					else {

						if (j == 0 || temp_dist2 == 0) {
							temp_dist2 = dist[0][j];
							n_pos = j;
						}
						else {
							if (temp_dist2 >= dist[0][j]) {
								temp_dist2 = dist[0][j];
								n_pos = j;
							}
						}
					}
				}
				if (count == 0) {
					if (verbose == 1) {
						printf("\nnon of the SVP profiles are within the time period, The tool is selecting nearest in position "
						       "without time considaration\n");
						printf("the shortest distance is number %d from the list\n", n_pos);
					}
					n = n_pos;
				}
				else {
					if (verbose == 1)
						printf("the shortest distance within time is number %d from the list\n", n_pos_time);
					n = n_pos_time;
				}
				fprintf(fresult, "%s\n", "============================================================");
				fprintf(fresult, "%s\t", inf_hold[i].file_name);
				fprintf(fresult, "%s\n", svp_hold[n].file_name);
				fprintf(fresult, "%s\n", "=============================================================");
				printf("Building the parameters to call mbset\n");
				/* printf("%s\n", all_in_sys); */
				strcat(all_in_sys, " -I ");
				inf_hold[i].file_name[strlen(inf_hold[i].file_name) - 1] = '\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name) - 1] = '\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name) - 1] = '\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name) - 1] = '\0';
				strcat(all_in_sys, inf_hold[i].file_name);
				/* printf("%s\n", all_in_sys); */
				strcat(all_in_sys, " -PSVPFILE:");
				strcat(all_in_sys, svp_hold[n].file_name);
				printf("%s\n", all_in_sys);
				/* int shellstatus = */ system(all_in_sys);
			}
			if (p_flag == 3) {
				// SVP nearest in time within range
				if (verbose == 1) {
					puts("==================================================");
					printf("\nCalculating the nearest svp in time within %d range for for %s\n", p_4_range,
					       inf_hold[i].file_name);
				}
				if (p_4_flage == 0)
					printf("\n Calculating the nearest SVP in time\n");
				if (p_4_flage == 1)
					printf("\n Calculating the nearest SVP in month (seasonal selection)\n");
				double temp_time = -9999;
				double temp_day = -9999;
				double temp_hour = -9999;
				double temp_min = -9999;
				double temp_time2 = -9999;
				double temp_day2 = -9999;
				double temp_hour2 = -9999;
				double temp_min2 = -9999;
				int count = 0;
				int n_time_noSeason = 0;
				int n_time_season = 0;
				int n_pos_seasn = 0;
				int n_pos_noSeason = 0;

				for (int j = 0; j < size_2; j++) {
					// processing time differences
					day_hold[0][j] = abs(inf_hold[i].s_datum_time.tm_yday - svp_hold[j].svp_datum_time.tm_yday);
					hour_hold[0][j] = abs(inf_hold[i].s_datum_time.tm_hour - svp_hold[j].svp_datum_time.tm_hour);
					min_hold[0][j] = abs(inf_hold[i].s_datum_time.tm_min - svp_hold[j].svp_datum_time.tm_min);
					time_hold[0][j] = fabs(difftime(inf_hold[i].s_Time, svp_hold[j].svp_Time));
					// processing distance differences
					geod_inverse(&g, inf_hold[i].ave_lat, inf_hold[i].ave_lon, svp_hold[j].s_lat, svp_hold[j].s_lon, &dist[0][j],
					             &azi1, &azi2);
					dist[0][j] -= p_4_range;
					// if the SVP is within the range
					puts("==================================================");
					printf("year day difference %d is : %d\n", j, day_hold[0][j]);
					printf("hour difference %d is : %d\n", j, hour_hold[0][j]);
					printf("minute difference %d is : %d\n", j, min_hold[0][j]);
					printf("Time difference %d is : %lf\n", j, time_hold[0][j]);
					printf("distance - range (if positive then SVP out of range if negative then the SVP within range) %d is : "
					       "%lf\n",
					       j, dist[0][j]);

					if (dist[0][j] < 0) {
						count = 1;
						if (p_4_flage == 0) {
							if (j == 0 || temp_time == -9999) {
								temp_time = time_hold[0][j];
								n_time_noSeason = j;
							}
							else {
								if (temp_time >= time_hold[0][j]) {
									temp_time = time_hold[0][j];
									n_time_noSeason = j;
								}
							}
						}
						else {
							if (j == 0 || (temp_day == -9999 && temp_hour == -9999 && temp_min == -9999)) {
								temp_day = day_hold[0][j];
								temp_hour = hour_hold[0][j];
								temp_min = min_hold[0][j];
								n_time_season = j;
							}
							else {
								if (temp_day > day_hold[0][j]) {
									temp_day = day_hold[0][j];
									temp_hour = hour_hold[0][j];
									temp_min = min_hold[0][j];
									n_time_season = j;
								}
								else if (temp_day == day_hold[0][j]) {
									if (temp_hour > hour_hold[0][j]) {
										temp_day = day_hold[0][j];
										temp_hour = hour_hold[0][j];
										temp_min = min_hold[0][j];
										n_time_season = j;
									}
									else if (temp_hour == hour_hold[0][j]) {
										if (temp_min > min_hold[0][j]) {
											temp_day = day_hold[0][j];
											temp_hour = hour_hold[0][j];
											temp_min = min_hold[0][j];
											n_time_season = j;
										}
									}
								}
							}
						}
					}

					// if the SVP is out of the range
					else {
						if (p_4_flage == 0) {
							if (j == 0 || temp_time2 == -9999) {
								temp_time2 = time_hold[0][j];
								n_pos_noSeason = j;
							}
							else {
								if (temp_time2 >= time_hold[0][j]) {
									temp_time2 = time_hold[0][j];
									n_pos_noSeason = j;
								}
							}
						}
						else {
							if (j == 0 || (temp_day2 == -9999 && temp_hour2 == -9999 && temp_min2 == -9999)) {
								temp_day2 = day_hold[0][j];
								temp_hour2 = hour_hold[0][j];
								temp_min2 = min_hold[0][j];
								n_pos_seasn = j;
							}
							else {
								if (temp_day2 > day_hold[0][j]) {
									temp_day2 = day_hold[0][j];
									temp_hour2 = hour_hold[0][j];
									temp_min2 = min_hold[0][j];
									n_pos_seasn = j;
								}
								else if (temp_day2 == day_hold[0][j]) {
									if (temp_hour2 > hour_hold[0][j]) {
										temp_day2 = day_hold[0][j];
										temp_hour2 = hour_hold[0][j];
										temp_min2 = min_hold[0][j];
										n_pos_seasn = j;
									}
									else if (temp_hour2 == hour_hold[0][j]) {
										if (temp_min2 > min_hold[0][j]) {
											temp_day2 = day_hold[0][j];
											temp_hour2 = hour_hold[0][j];
											temp_min2 = min_hold[0][j];
											n_pos_seasn = j;
										}
									}
								}
							}
						}
					}
				}

				if (count == 0) {
					if (p_4_flage == 0) {
						if (verbose == 1) {
							printf("\nnon of the SVP profiles are within the specified range, The tool is selecting nearest in "
							       "time without range considaration\n");
							printf("the nearest in time is number %d from the list\n", n_pos_noSeason);
						}
						n = n_pos_noSeason;
					}
					else {
						if (verbose == 1) {
							printf("\nnon of the SVP profiles are within the specified range, The tool is selecting nearest in "
							       "time without range considaration\n");
							printf("the nearest in season is number %d from the list\n", n_pos_seasn);
						}
						n = n_pos_seasn;
					}
				}
				else {
					if (p_4_flage == 0) {
						if (verbose == 1)
							printf("the nearest in time within range is number %d from the list\n", n_time_noSeason);
						n = n_time_noSeason;
					}
					else {
						if (verbose == 1)
							printf("the nearest in season within range is number %d from the list\n", n_time_season);
						n = n_time_season;
					}
				}
				fprintf(fresult, "%s\n", "============================================================");
				fprintf(fresult, "%s\t", inf_hold[i].file_name);
				fprintf(fresult, "%s\n", svp_hold[n].file_name);
				fprintf(fresult, "%s\n", "=============================================================");
				printf("Building the parameters to call mbset\n");
				strcat(all_in_sys, " -I ");
				inf_hold[i].file_name[strlen(inf_hold[i].file_name) - 1] = '\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name) - 1] = '\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name) - 1] = '\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name) - 1] = '\0';
				strcat(all_in_sys, inf_hold[i].file_name);
				strcat(all_in_sys, " -PSVPFILE:");
				strcat(all_in_sys, svp_hold[n].file_name);
				printf("%s\n", all_in_sys);
				/* int shellstatus = */ system(all_in_sys);
			}
		}
	}
	free(inf_hold);
	free(svp_hold);
	fclose(fDatalist);
	fclose(fSvp);
	fclose(fresult);
} /* read_list */
/* ------------------------------------------------------------------- */

int main(int argc, char **argv) {
	int error = MB_ERROR_NO_ERROR;

	char datalist[BUFSIZ] = "datalist.mb-1";
	char svplist[BUFSIZ] = "svplist.mb-1";

	{
		bool errflg = false;
		int c;
		bool help = false;
		while ((c = getopt(argc, argv, "HhI:i:S:s:P:p:VvNn")) != -1)
			switch (c) {
			case 'H':
			case 'h':
				help = true;
				break;
			case 'I':
			case 'i':
				sscanf(optarg, "%1023s", datalist);
				break;
			case 'N':
			case 'n':
				zero_test += 1;
				break;
			case 'P':
			case 'p':
			{
				int n1;
				int n2;
				int n3;
				const int n = sscanf(optarg, "%d/%d/%d", &n1, &n2, &n3);
				n_p2 = n;
				/* printf("\nthis is n %d \n", n); */
				if ((n1 != 0) && (n1 != 1) && (n1 != 2) && (n1 != 3)) {
					puts("Only four options are available: 0 for nearest position, 1 for nearest in time, 2 for both, 3 for nearest "
					     "in time within range");
					puts("The default is svp_nearest in position");
					puts("If option 2 is chosen without specifying time period, 10 hours is the default value");
					puts("If option 3 is chosen without specifying range, 10000 meters is the default value");
					puts("If option 3 is chosen two options are available : nearest in time and nearest in month");
					pause_screen();
					exit(0);
				}
				else {
					if (n == 0)
						p_flag = 0;
					if (n == 1) {
						p_flag = n1;
						if (p_flag == 2) {
							p_3_time = 10;
							n2 = p_3_time;
						}
						if (p_flag == 3) {
							p_4_range = 10000;
							n2 = p_4_range;
						}
					}
					if (n == 2) {
						p_flag = n1;
						if ((p_flag == 0) || (p_flag == 1))
							puts("The options -P0 for nearest in position or -P1 for nearest in time do not need further arguments");

						if (p_flag == 2)
							p_3_time = n2;
						if (p_flag == 3)
							p_4_range = n2;
					}
					if (n == 3) {
						p_flag = n1;
						p_4_range = n2;
						p_4_flage = n3;
						if ((p_flag == 0) || (p_flag == 1))
							puts("The options -P0 for nearest in position or -P1 for nearest in time do not need further arguments");

						if ((p_4_flage != 0) && (p_4_flage != 1)) {
							puts("If option 3 is chosen two options are available : nearest in time with -P3/0 and nearest in month "
							     "with -P3/1");
							pause_screen();
							exit(0);
						}
					}
				}
				break;
			}
			case 'S':
			case 's':
				sscanf(optarg, "%1023s", svplist);
				break;
			case 'V':
			case 'v':
				verbose++;
				break;
			default:
				break;
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
			fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
			fprintf(stderr, "dbg2       help:       %d\n", help);
			fprintf(stderr, "dbg2       datalist:   %s\n", datalist);
			fprintf(stderr, "dbg2       svplist:    %s\n", svplist);
			fprintf(stderr, "dbg2       p_flag:     %d\n", p_flag);
			fprintf(stderr, "dbg2       p_3_time:   %d\n", p_3_time);
			fprintf(stderr, "dbg2       p_4_range:  %d\n", p_4_range);
			fprintf(stderr, "dbg2       p_4_flage:  %d\n", p_4_flage);
			fprintf(stderr, "dbg2       zero_test:  %d\n", zero_test);
		}

		if (help) {
			fprintf(stderr, "\n%s\n", help_message);
			fprintf(stderr, "\nusage: %s\n", usage_message);
			exit(error);
		}
	}

	read_list(datalist, svplist);

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s> completed\n", program_name);
		fprintf(stderr, "dbg2  Ending status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		fprintf(stderr, "dbg2       error:   %d\n", error);
	}

	exit(error);

} /* main */
/* ---------------------------------------------------------------- */
