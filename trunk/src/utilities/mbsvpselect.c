/*--------------------------------------------------------------------
 *    The MB-system:	mbsvpselect.c	03.03.2014
 *	$Id$
 *
 *    Copyright (c) 2014-2014 by
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
 * Mbsvpselect chooses and implements the best available sound speed model
 * for each swath file in a survey. The user provides a list of the
 * available sound speed models and specifies the criteria used for
 * model selection. The program uses mbset to turn on bathymetry
 * recalculation by raytracing through the sound speed model selected
 * for each swath file.
 *
 * Author:	Ammar Aljuhe (ammaraljuhne@gmail.com)
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
 * After finding the appropriate svp for each profile based on the choosed
 * method, the results are copied to a txt file that shows each survey
 * profile with the corresponding SVP. the tool also calls mbset automatically
 * so no need to assign SVP to the data. it is done automatically.
 *
 * There are 5 methods for choosing the appropriate SVP for each survey
 * profile. These methods are:
 *
 * 1. Nearest SVP in position: the middle position of each survey profile
 *    is calculated and the geodesics (shortest distance on the ellipsoid)
 *    to all SVPs are calcualted. and the SVP with the shortest distance is
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
 * relevant data. The ancilliary *.inf, *.fbt, and *.fnv files must be created
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
 *    and generating the ancilliary *.inf, *.fbt, and *.fnv files for each of
 *    the swath files.
 * 2) Create an svplist file (analagous to a datalist, but referencing the
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

/* ---------------------------------------------------------------- */

/* include files */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_status.h"

#include "geodesic.h"

/* struct info_holder (shortly inf) hold the information from auxiliary files .inf
 * that are created from mbdatalist command */

struct info_holder
	{
	int flag;
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
struct svp_holder
	{
	char *file_name;
	long double s_lat;
	long double s_lon;
	struct tm svp_datum_time;
	time_t svp_Time;
	};
typedef struct svp_holder svp;

int Is_Leap(int year);
void calc_ave_dateTime(inf *inf_hold);
void JulianToGregorian(int year, int yearDay, int* year_tm, int* month, int* wDay);
void GregorianToJulian(int year, int month, int day, int* yearDay);
void fill_struct_inf(inf *inf_hold, char *holder);
void fill_struct_svp(svp *svp_hold, char *holder);
void read_list(char *list, char *list_2);
void trim_newline(char string[]);
void pause_screen();
double convert_decimal(int deg, int min, int sec);
void print_inf(inf *cd);
void print_svp(svp *cd);
void mid_point(long double lat1,
	long double lon1,
	long double lat2,
	long double lon2,
	long double *lat3,
	long double *lon3);

/* global variables */
int counter_i_i2=0;
int p_flag=0;
int p_3_time = 10;
int p_4_range = 10000;
int p_4_flage = 0;
int verbose=0;
int zero_test=0;
int size_2=0;
int n_p2 = 0;
char dHolder[1000][1000];
char sdHolder[1000][1000];


char holder[1000][1000];	/* copy the buffer into holder array for indexing */
char holder_2[1000][1000];	/* copy the buffer into holder array for indexing */
char holder_3[1000][1000];
char holder_4[1000][1000];
char dBuffer[BUFSIZ];
char sdBuffer[BUFSIZ];
char buffer[BUFSIZ];		/* String to hold the file name */
char buffer_2[BUFSIZ];

static char version_id[] = "$Id$";

/* ------------------------------------------------------------------- */

int main
(
	int argc,
	char ** argv
)
{
	char program_name[] = "mbsvpselect";
	char help_message[] =  "Program mbsvpselect chooses and implements the best available sound speed\n"
                                "model for each swath file in a survey. The user provides a list of the\n"
                                "available sound speed models and specifies the criteria used for\n"
                                "model selection. The program uses mbset to turn on bathymetry\n"
                                "recalculation by raytracing through the sound speed model selected\n"
                                "for each swath file.";
	char usage_message[] = "mbsvpselect -H -N -Idatalist -Ssvplist "
                                "[-P0, -P1, -P2/period, -P3/range, -P3/range/1]  -V";
	extern char *optarg;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* MBIO status variables */
	int	status = MB_SUCCESS;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;

	char datalist[BUFSIZ];
	char svplist[BUFSIZ];

	int n;
	int n1, n2, n3;

	strcpy(datalist, "datalist.mb-1");
	strcpy(svplist, "svplist.mb-1");
	while ((c = getopt(argc, argv, "HhI:i:S:s:P:p:VvNn")) != -1)
		switch (c)
			{
		case 'H':
		case 'h':
			help++;
			break;
		case 'I':
		case 'i':
			sscanf(optarg, "%s", datalist);
			flag++;
			break;
		case 'N':
		case 'n':
			zero_test+=1;
			break;
		case 'P':
		case 'p':
			n = sscanf(optarg, "%d/%d/%d", &n1, &n2, &n3);
			n_p2 =n;
			/* printf("\nthis is n %d \n", n); */
			if ((n1!=0) && (n1!=1) && (n1!=2) && (n1!=3))
				{
				puts("Only four options are available: 0 for nearest position, 1 for nearest in time, 2 for both, 3 for nearest in time within range");
				puts("The default is svp_nearest in position");
				puts("If option 2 is chosen without specifying time period, 10 hours is the default value");
				puts("If option 3 is chosen without specifying range, 10000 meters is the default value");
				puts("If option 3 is chosen two options are available : nearest in time and nearest in month");
				pause_screen();
				exit(0);
				}
			else
				{
				if (n == 0)
					p_flag = 0;
				if (n==1)
					{
					p_flag = n1;
					if (p_flag ==2)
						{
						p_3_time = 10;
						n2 = p_3_time;
						}
					if (p_flag==3)
						{
						p_4_range = 10000;
						n2 = p_4_range;
						}
					}
				if (n ==2)
					{
					p_flag = n1;
					if ((p_flag==0) || (p_flag ==1))
						puts(
							"The options -P0 for nearest in position or -P1 for nearest in time do not need further arguments");

					if (p_flag==2)
						p_3_time = n2;
					if (p_flag == 3)
						p_4_range = n2;
					}
				if(n==3)
					{
					p_flag = n1;
					p_4_range = n2;
					p_4_flage = n3;
					if ((p_flag==0) || (p_flag ==1))
						puts(
							"The options -P0 for nearest in position or -P1 for nearest in time do not need further arguments");

					if ((p_4_flage!=0) && (p_4_flage!=1))
						{
						puts(
							"If option 3 is chosen two options are available : nearest in time with -P3/0 and nearest in month with -P3/1");
						pause_screen();
						exit(0);
						}
					}
				}
			flag++;
			break;
		case 'S':
		case 's':
			sscanf(optarg, "%s", svplist);
			flag++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		default:
			break;
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
		fprintf(stderr,"Version %s\n",version_id);
		fprintf(stderr,"MB-system Version %s\n",MB_VERSION);
		}

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s>\n",program_name);
		fprintf(stderr,"dbg2  Version %s\n",version_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Control Parameters:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       help:       %d\n",help);
		fprintf(stderr,"dbg2       datalist:   %s\n",datalist);
		fprintf(stderr,"dbg2       svplist:    %s\n",svplist);
		fprintf(stderr,"dbg2       p_flag:     %d\n",p_flag);
		fprintf(stderr,"dbg2       p_3_time:   %d\n",p_3_time);
		fprintf(stderr,"dbg2       p_4_range:  %d\n",p_4_range);
		fprintf(stderr,"dbg2       p_4_flage:  %d\n",p_4_flage);
		fprintf(stderr,"dbg2       zero_test:  %d\n",zero_test);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

        /* do the work */        
	read_list(datalist, svplist);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s> completed\n",
			program_name);
		fprintf(stderr,"dbg2  Ending status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		fprintf(stderr,"dbg2       error:   %d\n",error);
		}

	/* end it all */
	exit(error);
        
} /* main */
/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */
/* Is leap old */
int Is_Leap
(
	int year
)
{
	if ( year%400 == 0)
		return 0;
	else if ( year%100 == 0)
		return 1;
	else if ( year%4 == 0 )
		return 0;
	else
		return 1;
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
void JulianToGregorian
(
	int year,
	int yearDay,
	int* year_tm,
	int* month,
	int* wDay
)
{
	*year_tm = year-1900;
	if(Is_Leap(year) == 0)
		{
		if(yearDay > 335)
			{
			*month = 11; *wDay = yearDay - 335;
			}
		if((yearDay > 305) && (yearDay<=335))
			{
			*month = 10; *wDay = yearDay - 305;
			}
		if((yearDay > 274) && (yearDay<=305))
			{
			*month = 9; *wDay = yearDay - 274;
			}
		if((yearDay > 244) && (yearDay<=274))
			{
			*month = 8; *wDay = yearDay - 244;
			}
		if((yearDay > 213) && (yearDay<=244))
			{
			*month = 7; *wDay = yearDay - 213;
			}
		if((yearDay > 182) && (yearDay<=213))
			{
			*month = 6; *wDay = yearDay - 182;
			}
		if((yearDay > 152) && (yearDay<=182))
			{
			*month = 5; *wDay = yearDay - 152;
			}
		if((yearDay > 121) && (yearDay<=152))
			{
			*month = 4; *wDay = yearDay - 121;
			}
		if((yearDay > 91) && (yearDay<=121))
			{
			*month = 3; *wDay = yearDay - 91;
			}
		if((yearDay > 60) && (yearDay<=91))
			{
			*month = 2; *wDay = yearDay - 60;
			}
		if((yearDay > 31) && (yearDay<=60))
			{
			*month = 1; *wDay = yearDay - 274;
			}
		if(yearDay <31)
			{
			*month = 0; *wDay = yearDay;
			}
		}
	else
		{
		if(yearDay > 334)
			{
			*month = 11; *wDay = yearDay - 334;
			}
		if((yearDay > 304) && (yearDay<=334))
			{
			*month = 10; *wDay = yearDay - 304;
			}
		if((yearDay > 273) && (yearDay<=304))
			{
			*month = 9; *wDay = yearDay - 273;
			}
		if((yearDay > 243) && (yearDay<=273))
			{
			*month = 8; *wDay = yearDay - 243;
			}
		if((yearDay > 212) && (yearDay<=243))
			{
			*month = 7; *wDay = yearDay - 212;
			}
		if((yearDay > 181) && (yearDay<=212))
			{
			*month = 6; *wDay = yearDay - 181;
			}
		if((yearDay > 151) && (yearDay<=181))
			{
			*month = 5; *wDay = yearDay - 151;
			}
		if((yearDay > 120) && (yearDay<=151))
			{
			*month = 4; *wDay = yearDay - 120;
			}
		if((yearDay > 90) && (yearDay<=120))
			{
			*month = 3; *wDay = yearDay - 90;
			}
		if((yearDay > 59) && (yearDay<=90))
			{
			*month = 2; *wDay = yearDay - 59;
			}
		if((yearDay > 31) && (yearDay<=59))
			{
			*month = 1; *wDay = yearDay - 31;
			}
		if(yearDay <31)
			{
			*month = 0; *wDay = yearDay;
			}
		}
} /* JulianToGregorian */
/* ---------------------------------------------------------------- */
void GregorianToJulian
(
	int year,
	int month,
	int day,
	int* yearDay
)
{
	if(Is_Leap(year))
		switch (month)
			{
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
		switch (month)
			{
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
/* ---------------------------------------------------------------- */
/*this function fills the inf struct with the appropriate values
 * it takes a pointer to the inf_struct and the file_name (file_name.inf) to read information from
 */
void fill_struct_inf
(
	inf *inf_hold,
	char *holder
)
{
	int mon, year;
	double s_sec;
	inf_hold->flag=0;
	FILE *fileName;
	inf_hold->file_name = holder;
	struct tm *ptr;
        
	/* reading relative inf file */
	fileName = fopen(inf_hold->file_name, "r");
	if(fileName == NULL)
		{
		printf("%s could not be opened Please check the datalist files\n", inf_hold->file_name);
		exit(1);
		}

	/* reaching start of data key word */


	while(fgets(buffer, sizeof buffer, fileName)!=NULL)
		if (strcmp(buffer, "Start of Data:\n")==0)
			break;


	/* parsing date and time*/

	fgets(buffer, sizeof buffer, fileName);

	sscanf(buffer,
		"%*s %d %d %d %i:%i:%lf %*s %*s",
		&mon,
		&inf_hold->s_datum_time.tm_mday,
		&year,
		&inf_hold->s_datum_time.tm_hour,
		&inf_hold->s_datum_time.tm_min,
		&s_sec);
	inf_hold->s_datum_time.tm_mon = mon-1;
	inf_hold->s_datum_time.tm_year = year-1900;
	s_sec = floor(s_sec);
	inf_hold->s_datum_time.tm_sec = (int) s_sec;
	GregorianToJulian(inf_hold->s_datum_time.tm_year,
		inf_hold->s_datum_time.tm_mon,
		inf_hold->s_datum_time.tm_mday,
		&inf_hold->s_datum_time.tm_yday);

	ptr = &inf_hold->s_datum_time;
	inf_hold->s_Time = mktime(ptr);

	/* Lon and Lat processing*/

	fgets(buffer, sizeof buffer, fileName);

	sscanf(buffer, "%*s %Lf %*s %Lf %*s %*lf %*s", &inf_hold->s_lon, &inf_hold->s_lat);

	/*parsing end of data information*/

	while(fgets(buffer, sizeof buffer, fileName)!=NULL)
		if (strcmp(buffer, "End of Data:\n")==0)
			break;

	fgets(buffer, sizeof buffer, fileName);

	sscanf(buffer,
		"%*s %d %d %d %i:%i:%lf %*s %*s",
		&mon,
		&inf_hold->e_datum_time.tm_mday,
		&year,
		&inf_hold->e_datum_time.tm_hour,
		&inf_hold->e_datum_time.tm_min,
		&s_sec);

	inf_hold->e_datum_time.tm_mon = mon-1;
	inf_hold->e_datum_time.tm_year = year-1900;
	s_sec = floor(s_sec);
	inf_hold->e_datum_time.tm_sec = (int) s_sec;

	struct tm *ptr2 = &inf_hold->e_datum_time;
	inf_hold->e_time = mktime(ptr2);

	/* calc_ave_dateTime(inf_hold); */

	/* Lon and Lat processing */
	fgets(buffer, sizeof buffer, fileName);

	sscanf(buffer, "%*s %Lf %*s %Lf %*s %*lf %*s", &inf_hold->e_lon, &inf_hold->e_lat);
	if (zero_test>0)
		{
		if ((inf_hold->s_lat == 0.0) && (inf_hold->s_lon == 0.0))
			{
			if ((inf_hold->e_lon == 0.0) && (inf_hold->e_lon == 0.0))
				inf_hold->flag=3;
			else
				inf_hold->flag=1;
			}
		else
			{
			if ((inf_hold->e_lon == 0.0) && (inf_hold->e_lon == 0.0))
				inf_hold->flag=2;
			else
				inf_hold->flag=0;
			}
		}
	/* calculate the mid_point */
	mid_point(inf_hold->s_lat,
		inf_hold->s_lon,
		inf_hold->e_lat,
		inf_hold->e_lon,
		&inf_hold->ave_lat,
		&inf_hold->ave_lon);

	fclose(fileName);
} /* fill_struct_inf */
/* ------------------------------------------------------------------- */
/* SVP tool is able to read only two SVP formats
 *
 *
 */

/* ------------------------------------------------------------------- */
void fill_struct_svp
(
	svp *svp_hold,
	char *holder
)
{
	int yearDay, month, year;
	double seconds;
	int s_lat_min=0;
	int s_lat_deg=0;
	int s_lat_sec =0;
	int s_lon_min=0;
	int s_lon_deg=0;
	int s_lon_sec =0;
	FILE *fileName;
	svp_hold->file_name = holder;
	char caris_str[] = "Section";
	char mb1_str[] = "## MB-SVP";
	char mb2_str[] = "MB-SVP";
	char *ptr_caris = NULL;
	char *ptr_mb1 = NULL;
	char *ptr_mb2 = NULL;

	/* reading relative svp file */
	fileName = fopen(svp_hold->file_name, "r");

	if(fileName == NULL)
		{
		printf("%s could not be opend\n", svp_hold->file_name);
		exit(1);
		}

	/* reaching start of data */

	while(fgets(buffer, sizeof buffer, fileName)!=NULL)
		{
		ptr_caris = strstr(buffer, caris_str);
		ptr_mb1 = strstr(buffer, mb1_str);
		ptr_mb2 = strstr(buffer, mb2_str);
		if (ptr_caris != NULL)
			{
			printf("\n%s\n", buffer);
			sscanf(buffer,
				"%*s %d-%d  %i:%i:%i %d:%d:%d %d:%d:%d",
				&year,
				&yearDay,
				&svp_hold->svp_datum_time.tm_hour,
				&svp_hold->svp_datum_time.tm_min,
				&svp_hold->svp_datum_time.tm_sec,
				&s_lat_deg,
				&s_lat_min,
				&s_lat_sec,
				&s_lon_deg,
				&s_lon_min,
				&s_lon_sec);
			svp_hold->svp_datum_time.tm_year = year-1900;
			svp_hold->svp_datum_time.tm_yday= yearDay;
			JulianToGregorian(year,
				yearDay,
				&svp_hold->svp_datum_time.tm_year,
				&svp_hold->svp_datum_time.tm_mon,
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
		else if (ptr_mb1 != NULL)
			{
			printf("\n%s\n", buffer);
			sscanf(buffer,
				"## MB-SVP %d/%d/%d %d:%d:%lf %Lf %Lf",
				&year,
				&month,
				&svp_hold->svp_datum_time.tm_mday,
				&svp_hold->svp_datum_time.tm_hour,
				&svp_hold->svp_datum_time.tm_min,
				&seconds,
				&svp_hold->s_lon,
				&svp_hold->s_lat);

			svp_hold->svp_datum_time.tm_mon = month - 1;
			svp_hold->svp_datum_time.tm_year = year-1900;
			svp_hold->svp_datum_time.tm_sec = seconds;
			GregorianToJulian(svp_hold->svp_datum_time.tm_year,
				svp_hold->svp_datum_time.tm_mon,
				svp_hold->svp_datum_time.tm_mday,
				&svp_hold->svp_datum_time.tm_yday);
			struct tm *ptr = &svp_hold->svp_datum_time;
			svp_hold->svp_Time = mktime(ptr);

			break;
			}
		else if (ptr_mb2 != NULL)
			{
			printf("\n%s\n", buffer);
			sscanf(buffer,
				"%*s %d/%d/%d %i:%i %Lf %Lf",
				&year,
				&month,
				&svp_hold->svp_datum_time.tm_mday,
				&svp_hold->svp_datum_time.tm_hour,
				&svp_hold->svp_datum_time.tm_min,
				&svp_hold->s_lat,
				&svp_hold->s_lon);

			svp_hold->svp_datum_time.tm_mon = month - 1;
			svp_hold->svp_datum_time.tm_year = year-1900;
			svp_hold->svp_datum_time.tm_sec = 0;
			GregorianToJulian(svp_hold->svp_datum_time.tm_year,
				svp_hold->svp_datum_time.tm_mon,
				svp_hold->svp_datum_time.tm_mday,
				&svp_hold->svp_datum_time.tm_yday);
			struct tm *ptr = &svp_hold->svp_datum_time;
			svp_hold->svp_Time = mktime(ptr);

			break;
			}
		}

	fclose(fileName);
} /* fill_struct_svp */
/* ------------------------------------------------------------------- */
void read_list
(
	char *list,
	char *list_2
)
{
	int size =0;
	int i=0;
	int i2=0;
	/* int counter_i2 = 0; */
	int j=0;
	FILE *fDatalist;
	FILE *fSvp;
        FILE *testOpen;
	FILE *fresult;
	FILE *sdHold_file;
	inf *inf_hold = NULL;
	svp *svp_hold = NULL;
	double dist[size][size_2];
	double time_hold[size][size_2];
	double hour_hold[size][size_2];
	double min_hold[size][size_2];
	double day_hold[size][size_2];
	/* double dist_time[size][size_2]; */
	double min_time[size];
	double min_dis[size];
	/* double max_dist[size]; */
	int n = 0;
	struct geod_geodesic g;
	double azi1, azi2;
        int count;

	atexit(pause_screen);				/* pause the screen */

	/* open datalist.mb-1 for names of the files */
	fDatalist = fopen(list, "r");
	fSvp      = fopen(list_2, "r");
	if(fDatalist == NULL)
		{
		printf("%s Could not be found", list);
		exit(1);
		}

	if(fSvp == NULL)
		{
		printf("%s Could not be found", list_2);
		exit(1);
		}
	fresult = fopen("result.txt", "w+");
	if(fresult == NULL)
		{
		printf("result.txt could not be found");
		exit(1);
		}
	/* ------------------------------ */

	while(fgets(dBuffer, sizeof dBuffer, fDatalist)!=NULL)
		{
		strcpy(holder[i], dBuffer);
		while(holder[i][strlen(holder[i])-1] != ' ')
			holder[i][strlen(holder[i])-1] = '\0';
		holder[i][strlen(holder[i])-1] = '\0';
		strcat(holder[i], ".inf");
		testOpen = fopen(holder[i], "r");
		if (testOpen==NULL)
			{
			i2=i;
			strcpy(dHolder[i], dBuffer);
			while(dHolder[i][strlen(dHolder[i])-1] != ' ')
				dHolder[i][strlen(dHolder[i])-1] = '\0';
			dHolder[i][strlen(dHolder[i])-1] = '\0';

			FILE *dHold_file;
			strcpy(holder_3[i], dHolder[i]);
			while (holder_3[i][strlen(holder_3[i])-1] != '/')
				holder_3[i][strlen(holder_3[i])-1] = '\0';

			dHold_file = fopen(dHolder[i], "r");

			if(dHold_file == NULL)
				{
				printf("%s Could not be found file in absolute path", dHolder[i]);
				exit(1);
				}
			else
				{
				while(fgets(buffer, sizeof buffer, dHold_file)!=NULL)
					{
					strcpy(holder[i2], holder_3[i]);

					/* printf("  \ntest 1 %s \n", holder[i2]); */
					strcat(holder[i2], buffer);

					/* printf("  \n %s \n", holder_2[i2]); */
					/* printf("  \ntest 1 %s \n", holder[i2]); */
					while(holder[i2][strlen(holder[i2])-1] != ' ')
						holder[i2][strlen(holder[i2])-1] = '\0';
					holder[i2][strlen(holder[i2])-1] = '\0';
					strcat(holder[i2], ".inf");
					/* printf("  \n %s %d %d\n", holder[i2], i , i2); */
					/* printf("  \n %s \n", holder[i2]); */
					/*fclose(dHold_file);*/
					i2++;
					counter_i_i2+=1;
					}
				/* counter_i2 += i2; */

				fclose(dHold_file);
				}
			printf(" \n\n %d --- %d\n\n", i, i2);
			}
		if (i<i2)
			i=i2;
		else
			i++;

		}

	/* ------------------------------ */

	/* fill size */
	if (i>i2)
		size =i;
	else
		size=i2;
	printf("\n\n%d files to be read in the datalist\n\n", size);

	/* Allocate memory for inf_struct */
	inf_hold = malloc((size)*sizeof(inf));
	if(inf_hold==NULL)
		{
		printf("no memory for the process end of process");
		exit(1);
		}

	for(i=0; i<size; i++)
		{
		fill_struct_inf(&inf_hold[i], holder[i]);
		if (verbose==1)
			print_inf(&inf_hold[i]);
		}

	/* reset for svp_hold */
	i=0;
	i2=0;
	/* ------------------------ */
	while(fgets(sdBuffer, sizeof sdBuffer, fSvp)!=NULL)
		{
		strcpy(sdHolder[i], sdBuffer);
		while(sdHolder[i][strlen(sdHolder[i])-1] != ' ')
			sdHolder[i][strlen(sdHolder[i])-1] = '\0';
		sdHolder[i][strlen(sdHolder[i])-1] = '\0';
		strcpy(holder_4[i], sdHolder[i]);
		while (holder_4[i][strlen(holder_4[i])-1] != '/')
			holder_4[i][strlen(holder_4[i])-1] = '\0';

		sdHold_file = fopen(sdHolder[i], "r");
		if(sdHold_file == NULL)
			{
			printf("Failure to open file: %s\n", sdHolder[i]);
			exit(1);
			}
		else
			{
			while(fgets(buffer_2, sizeof buffer_2, sdHold_file)!=NULL)
				{
				strcpy(holder_2[i2], holder_4[i]);
				strcat(holder_2[i2], buffer_2);
				trim_newline(holder_2[i2]);

				i2++;
				}
			fclose(sdHold_file);
			}
		i++;
		}
	/* ------------------------ */
	printf("\n\n\n%d svp to be read\n\n\n", i2);
	/* fill size of svp_list */
	size_2=i2;

	/* Allocate memory for svp_struct */
	svp_hold = malloc((size_2)*sizeof(svp));
	if(svp_hold==NULL)
		{
		printf("no memory for the process end of process");
		exit(1);
		}

	for(i=0; i<size_2; i++)
		{
		fill_struct_svp(&svp_hold[i], holder_2[i]);
		if (verbose==1)
			print_svp(&svp_hold[i]);
		}

	/* calculating the distances and choose the appropriate file */
	if(p_flag==0)
		printf("\n Method chosen is %d nearest in position\n", p_flag);
	if(p_flag==1)
		printf("\n Method chosen is %d nearest in time\n", p_flag);
	if(p_flag==2)
		{
		printf("\n Method chosen is %d nearest in position within time\n", p_flag);
		if (n_p2==1)
			printf(
				"\n No specific time period was entered and the default time period %d hours will be taken\n",
				p_3_time);
		if (n_p2==2)
			printf("\n Time period %d hours will be taken\n", p_3_time);
		}
	if(p_flag==3)
		{
		printf("\n Method chosen is %d nearest in time within range\n", p_flag);
		printf("\n range  %d meters will be taken\n", p_4_range);
		if (p_4_flage==0)
			printf("\n Option 0 was chosen. The nearest in time within range will be calculated\n");

		if (p_4_flage==1)
			printf(
				"\n Option 1 was chosen. The nearest in month within range will be calculated. This will calculate within the specified range the SVP with the nearest month to the profile regardless of the year. This is the seasonal interpretation \n");

		}
	geod_init(&g, A_, F_);
	for(i=0; i<size; i++)
		{
		char all_in_sys[BUFSIZ]= "mbset";
		if(p_flag==0)
			{
			switch (inf_hold[i].flag)
				{
			case 0:

				if (verbose==1)
					puts("\n\n========N check passed no 0.0 position was found===========\n\n");
				if (verbose==1)
					printf("\nCalculating the distances to all svp profiles for %s\n",
						inf_hold[i].file_name);
				for(j=0; j<size_2; j++)
					{
					geod_inverse(&g,
						inf_hold[i].ave_lat,
						inf_hold[i].ave_lon,
						svp_hold[j].s_lat,
						svp_hold[j].s_lon,
						&dist[i][j],
						&azi1,
						&azi2);
					if (verbose==1)
						printf("Distance number %d is : %lf\n", j, dist[i][j]);
					}
				if (verbose==1)
					printf("\nSearching for the SVP with nearest position\n");
				min_dis[i] = dist[i][0];
				for(j=0; j<size_2; j++)
					if(min_dis[i]>dist[i][j])
						{
						min_dis[i] = dist[i][j];
						n=j;
						}
				if (verbose==1)
					printf("the shortest distance is number %d from the list\n", n);
				if (verbose==1)
					puts("==================================================");

				fprintf(fresult,
					"%s\n",
					"============================================================");
				fprintf(fresult, "%s\t", inf_hold[i].file_name);
				fprintf(fresult, "%s\n", svp_hold[n].file_name);
				fprintf(fresult,
					"%s\n",
					"=============================================================");
				printf("Calling mbset\n");
				/* printf("%s\n", all_in_sys); */
				strcat(all_in_sys, " -I ");
				inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
				strcat(all_in_sys, inf_hold[i].file_name);
				/* printf("%s\n", all_in_sys); */
				strcat(all_in_sys, " -PSVPFILE:");
				strcat(all_in_sys, svp_hold[n].file_name);
				printf("%s\n", all_in_sys);
				system(all_in_sys);
				break;
			case 1:
				if (verbose==1)
					puts(
						"\n\n=====================N check:   0.0 position was found=====================\n\n");

				if (verbose==1)
					printf(
						"\nThe file %s has no navigation information at the start position and the svp profile will be assigned to the end point of the file\n",
						inf_hold[i].file_name);
				for(j=0; j<size_2; j++)
					{
					geod_inverse(&g,
						inf_hold[i].s_lat,
						inf_hold[i].s_lon,
						svp_hold[j].s_lat,
						svp_hold[j].s_lon,
						&dist[i][j],
						&azi1,
						&azi2);
					if (verbose==1)
						printf("Distance number %d is : %lf\n", j, dist[i][j]);
					}

				if (verbose==1)
					printf("\nSearching for the SVP with the nearest position\n");

				min_dis[i] = dist[i][0];

				for(j=0; j<size_2; j++)
					if(min_dis[i]>dist[i][j])
						{
						min_dis[i] = dist[i][j];
						n=j;
						}
				if (verbose==1)
					printf("the shortest distance is number %d from the list\n", n);
				if (verbose==1)
					puts("==================================================");

				fprintf(fresult,
					"%s\n",
					"============================================================");
				fprintf(fresult, "%s\t", inf_hold[i].file_name);
				fprintf(fresult, "%s\n", svp_hold[n].file_name);
				fprintf(fresult,
					"%s\n",
					"=============================================================");
				printf("Building the parameters to call mbset\n");
				/* printf("%s\n", all_in_sys); */
				strcat(all_in_sys, " -I ");
				inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
				strcat(all_in_sys, inf_hold[i].file_name);
				/* printf("%s\n", all_in_sys); */
				strcat(all_in_sys, " -P ");
				strcat(all_in_sys, svp_hold[n].file_name);
				printf("%s\n", all_in_sys);
				system(all_in_sys);
				break;
			case 2:
				if (verbose==1)
					puts(
						"\n\n==============N check:   0.0 position was found===================\n\n");

				if (verbose==1)
					printf(
						"\nThe file %s has no navigation information at the end position and the svp profile will be assigned to the start point of the file\n",
						inf_hold[i].file_name);
				for(j=0; j<size_2; j++)
					{
					geod_inverse(&g,
						inf_hold[i].e_lat,
						inf_hold[i].e_lon,
						svp_hold[j].s_lat,
						svp_hold[j].s_lon,
						&dist[i][j],
						&azi1,
						&azi2);
					if (verbose==1)
						printf("Distance number %d is : %lf\n", j, dist[i][j]);
					}
				if (verbose==1)
					printf("\nSearching for the SVP with the nearest position\n");

				min_dis[i] = dist[i][0];

				for(j=0; j<size_2; j++)
					if(min_dis[i]>dist[i][j])
						{
						min_dis[i] = dist[i][j];
						n=j;
						}
				if (verbose==1)
					printf("the shortest distance is number %d from the list\n", n);
				if (verbose==1)
					puts("==================================================");
				fprintf(fresult,
					"%s\n",
					"============================================================");
				fprintf(fresult, "%s\t", inf_hold[i].file_name);
				fprintf(fresult, "%s\n", svp_hold[n].file_name);
				printf("Building the parameters to call mbset\n");
				/* printf("%s\n", all_in_sys); */
				strcat(all_in_sys, " -I ");
				inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
				strcat(all_in_sys, inf_hold[i].file_name);
				/* printf("%s\n", all_in_sys); */
				strcat(all_in_sys, " -P ");
				strcat(all_in_sys, svp_hold[n].file_name);
				printf("%s\n", all_in_sys);
				system(all_in_sys);
				fprintf(fresult,
					"%s\n",
					"=============================================================");
				break;
			case 3:
				if (verbose==1)
					{
					puts(
						"\n\n==============N check:   0.0 position was found====================\n\n");
					printf(
						"\n!!!The file %s has no navigation information and no svp will be assigned to it!!!\n",
						inf_hold[i].file_name);
					fprintf(fresult,
						"%s\n",
						"============================================================");
					fprintf(fresult, "%s\t", inf_hold[i].file_name);
					fprintf(fresult, "%s\n", "NaN");
					}
				break;

			default:
				break;
				} /* switch */
			}
		else
			{
			if (p_flag==1)			/* calculate the nearest in time */
				{
				if (verbose==1)
					puts("==================================================");
				if (verbose==1)
					printf("\nCalculating the nearest svp in time for for %s\n",
						inf_hold[i].file_name);
				for(j=0; j<size_2; j++)
					{
					time_hold[i][j] = abs(difftime(inf_hold[i].s_Time, svp_hold[j].svp_Time));
					if (verbose==1)
						printf("Time number %d is : %lf\n", j, time_hold[i][j]);
					}
				if (verbose==1)
					printf("\nSearch for the SVP that is the nearest in Time\n");
				min_time[i] = time_hold[i][0];
				for(j=0; j<size_2; j++)
					if(min_time[i]>time_hold[i][j])
						{
						min_time[i] = time_hold[i][j];
						n=j;
						}
				if (verbose==1)
					printf("the shortest time interval is %d\n", n);
				if (verbose==1)
					puts("==================================================");

				fprintf(fresult,
					"%s\n",
					"============================================================");
				fprintf(fresult, "%s\t", inf_hold[i].file_name);
				fprintf(fresult, "%s\n", svp_hold[n].file_name);
				fprintf(fresult,
					"%s\n",
					"=============================================================");
				printf("Building the parameters to call mbset\n");
				/* printf("%s\n", all_in_sys); */
				strcat(all_in_sys, " -I ");
				inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
				strcat(all_in_sys, inf_hold[i].file_name);
				/* printf("%s\n", all_in_sys); */
				strcat(all_in_sys, " -PSVPFILE:");
				strcat(all_in_sys, svp_hold[n].file_name);
				printf("%s\n", all_in_sys);
				system(all_in_sys);
				}
			/************calculate the nearest in position within time***************************/
			if(p_flag==2)
				{
				if (verbose==1)
					puts("==================================================");
				if (verbose==1)
					printf(
						"\nCalculating the nearest svp in position within %d time period for for %s\n",
						p_3_time,
						inf_hold[i].file_name);
				for(j=0; j<size_2; j++)
					{
					time_hold[i][j] = abs(difftime(inf_hold[i].s_Time, svp_hold[j].svp_Time));
					/* dist[i][j] = distVincenty(inf_hold[i].ave_lat, inf_hold[i].ave_lon,
					   svp_hold[j].s_lat, svp_hold[j].s_lon); */
					geod_inverse(&g,
						inf_hold[i].ave_lat,
						inf_hold[i].ave_lon,
						svp_hold[j].s_lat,
						svp_hold[j].s_lon,
						&dist[i][j],
						&azi1,
						&azi2);
					if (verbose==1)
						printf("Time %d is : %lf\n", j, time_hold[i][j]);
					if (verbose==1)
						printf("Dist %d is : %lf\n", j, dist[i][j]);
					}
				min_dis[i] = dist[i][0];

				int count;
				count = 0;

				for(j=0; j<size_2; j++)
					if(min_dis[i]>dist[i][j])
						{
						min_dis[i] = dist[i][j];
						n=j;
						}
				for(j=0; j<size_2; j++)
					{
					int c = 0;
					c = time_hold[i][j] - (p_3_time*3600);
					if (c <= 0)
						{
						if (count == 0)
							{
							min_dis[i] = dist[i][j];
							n=j;
							count+=1;
							}
						else if (min_dis[i]>dist[i][j])
							{
							min_dis[i] = dist[i][j];
							n=j;
							}
						}
					}
				fprintf(fresult,
					"%s\n",
					"============================================================");
				fprintf(fresult, "%s\t", inf_hold[i].file_name);
				fprintf(fresult, "%s\n", svp_hold[n].file_name);
				fprintf(fresult,
					"%s\n",
					"=============================================================");
				printf("Building the parameters to call mbset\n");
				/* printf("%s\n", all_in_sys); */
				strcat(all_in_sys, " -I ");
				inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
				inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
				strcat(all_in_sys, inf_hold[i].file_name);
				/* printf("%s\n", all_in_sys); */
				strcat(all_in_sys, " -PSVPFILE:");
				strcat(all_in_sys, svp_hold[n].file_name);
				printf("%s\n", all_in_sys);
				system(all_in_sys);
				}
			if (p_flag==3)
				{
				if (verbose==1)
					puts("==================================================");
				if (verbose==1)
					printf("\nCalculating the nearest svp in time within %d range for for %s\n",
						p_4_range,
						inf_hold[i].file_name);
				if(p_4_flage ==0)
					printf("\n Calculating the nearest SVP in time\n");
				if(p_4_flage ==1)
					printf("\n Calculating the nearest SVP in month (seasonal selection)\n");
				for(j=0; j<size_2; j++)
					{
					day_hold[i][j] = abs(
						inf_hold[i].s_datum_time.tm_yday - svp_hold[j].svp_datum_time.tm_yday);
					hour_hold[i][j] = abs(
						inf_hold[i].s_datum_time.tm_hour - svp_hold[j].svp_datum_time.tm_hour);
					min_hold[i][j] = abs(
						inf_hold[i].s_datum_time.tm_min - svp_hold[j].svp_datum_time.tm_min);
					time_hold[i][j] = abs(difftime(inf_hold[i].s_Time, svp_hold[j].svp_Time));
					/* dist[i][j] = distVincenty(inf_hold[i].ave_lat, inf_hold[i].ave_lon,
					   svp_hold[j].s_lat, svp_hold[j].s_lon); */
					geod_inverse(&g,
						inf_hold[i].ave_lat,
						inf_hold[i].ave_lon,
						svp_hold[j].s_lat,
						svp_hold[j].s_lon,
						&dist[i][j],
						&azi1,
						&azi2);
					if (verbose==1)
						printf("Time %d is : %lf\n", j, time_hold[i][j]);
					if (verbose==1)
						printf("distance %d is : %lf\n", j, dist[i][j]);
					}
				if(p_4_flage == 0)
					{
					min_time[i] = time_hold[i][0];
					count = 0;
					for(j=0; j<size_2; j++)
						if(min_time[i]>time_hold[i][j])
							{
							min_time[i] = time_hold[i][j];
							n=j;
							}
					for(j=0; j<size_2; j++)
						{
						int c = 0;
						c = dist[i][j] - p_4_range;
						if (c <= 0)
							{
							if (count == 0)
								{
								min_time[i] = time_hold[i][j];
								n=j;
								count+=1;
								}
							else if (min_time[i]>time_hold[i][j])
								{
								min_time[i] = time_hold[i][j];
								n=j;
								}
							}
						}
					fprintf(fresult,
						"%s\n",
						"============================================================");
					fprintf(fresult, "%s\t", inf_hold[i].file_name);
					fprintf(fresult, "%s\n", svp_hold[n].file_name);
					fprintf(fresult,
						"%s\n",
						"=============================================================");
					printf("Building the parameters to call mbset\n");
					/* printf("%s\n", all_in_sys); */
					strcat(all_in_sys, " -I ");
					inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
					inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
					inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
					inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
					strcat(all_in_sys, inf_hold[i].file_name);
					/* printf("%s\n", all_in_sys); */
					strcat(all_in_sys, " -PSVPFILE:");
					strcat(all_in_sys, svp_hold[n].file_name);
					printf("%s\n", all_in_sys);
					system(all_in_sys);
					}
				else
					{
					min_time[i] = day_hold[i][0];
					count = 0;
					for(j=0; j<size_2; j++)
						{
						if(min_time[i]>day_hold[i][j])
							{
							min_time[i] = day_hold[i][j];
							n=j;
							}
						else if(min_time[i]==day_hold[i][j])
							{
							if (hour_hold[i][n] > hour_hold[i][j])
								{
								min_time[i] = day_hold [i][j];
								n = j;
								}
							else
								{
								if (hour_hold[i][n] == hour_hold[i][j])
									if (min_hold[i][n] > min_hold[i][j])
										{
										min_time[i] = day_hold [i][j];
										n = j;
										}
								}
							}
						}
					for(j=0; j<size_2; j++)
						{
						int c = 0;
						c = dist[i][j] - p_4_range;
						if (c <= 0)
							{
							if (count == 0)
								{
								min_time[i] = time_hold[i][j];
								n=j;
								count+=1;
								}
							else if (min_time[i]>time_hold[i][j])
								{
								min_time[i] = time_hold[i][j];
								n=j;
								}
							}
						}
					fprintf(fresult,
						"%s\n",
						"============================================================");
					fprintf(fresult, "%s\t", inf_hold[i].file_name);
					fprintf(fresult, "%s\n", svp_hold[n].file_name);
					fprintf(fresult,
						"%s\n",
						"=============================================================");
					printf("Building the parameters to call mbset\n");
					/* printf("%s\n", all_in_sys); */
					strcat(all_in_sys, " -I ");
					inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
					inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
					inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
					inf_hold[i].file_name[strlen(inf_hold[i].file_name)-1]='\0';
					strcat(all_in_sys, inf_hold[i].file_name);
					/* printf("%s\n", all_in_sys); */
					strcat(all_in_sys, " -PSVPFILE:");
					strcat(all_in_sys, svp_hold[n].file_name);
					printf("%s\n", all_in_sys);
					system(all_in_sys);
					}
				}
			}
		}
	free(inf_hold);
	free(svp_hold);
	fclose(fDatalist);
	fclose(fSvp);
	fclose(fresult);
} /* read_list */
/* ---------------------------------------------------------------- */
/*
 *  Function trim_newline
 *	Delete the '\n' char from string
 */
void trim_newline
(
	char string[]
)
{
	if(string[strlen(string)-1] == '\n')
		string[strlen(string)-1] = '\0';
}
/* ---------------------------------------------------------------- */
/*
 * pause the screen at the exit of the program
 */
void pause_screen
	()
{
	printf("\nEnd the program press ENTER");
	fflush(stdin);
	getchar();
}
/* ---------------------------------------------------------------- */
/*
 * convert_decimal function
 * convert lat or lon from deg(int):min(int):sec(int) to decimal format
 */
double convert_decimal
(
	int deg,
	int min,
	int sec
)
{
	if(deg>=0)
		return (double)deg + (((double)min)/60) + (((double)sec)/3600);

	return -(fabs((double)deg) + (((double)min)/60) + (((double)sec)/3600));
}
/* ---------------------------------------------------------------- */
/* print the inf information on the screen */
void print_inf
(
	inf *cd
)
{
	struct tm *temp = &cd->s_datum_time;
	puts("==================================================");
	printf("file_name: %s\n", cd->file_name);
	puts("starting Date and time");
	printf("\n%s\n", asctime(temp));
	temp = NULL;
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
	temp = NULL;
}
/* --------------------------------------------------------------- */
/* print the svp information on the screen */
void print_svp
(
	svp *cd
)
{
	struct tm *temp = &cd->svp_datum_time;
	puts("==================================================");
	printf("file_name: %s\n", cd->file_name);
	puts("Date and time");
	printf("\n%s\n", asctime(temp));
	/*printf("%d-",cd->svp_datum_time.tm_year);
	   printf("%d\t",cd->svp_datum_time.tm_yday);
	   printf("%d:",cd->svp_datum_time.tm_hour);
	   printf("%d:",cd->svp_datum_time.tm_min);
	   printf("%d\n",cd->svp_datum_time.tm_sec);*/
	puts("position");
	printf("lat: %Lf\t", cd->s_lat);
	printf("lon: %Lf\n", cd->s_lon);
	puts("==================================================");
}
/* --------------------------------------------------------------- */
/* calculate the average position of two points */
/* http://www.movable-type.co.uk/scripts/latlong.html */
void mid_point
(
	long double lat1,
	long double lon1,
	long double lat2,
	long double lon2,
	long double *lat3,
	long double *lon3
)
{
	double dLon = DTR * ((lon2)-(lon1));
	double lat1_rad = DTR * ((lat1));
	double lat2_rad = DTR * ((lat2));
	double lon1_rad = DTR * ((lon1));
	double bx = cos(lat2_rad)*cos(dLon);
	double by = cos(lat2_rad)*sin(dLon);
	*(lat3) =
		atan2(sin(lat1_rad) + sin(lat2_rad),
		sqrt((((cos(lat1_rad))+bx)*((cos(lat1_rad))+bx)) + (by*by)))*RTD;
	*(lon3) = (lon1_rad + atan2(by, (cos(lat1_rad) + bx)))*RTD;
}
/* --------------------------------------------------------------- */
