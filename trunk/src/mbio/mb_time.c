/*--------------------------------------------------------------------
 *    The MB-system:	mb_time.c	1/21/93
 *    $Id$
 *
 *    Copyright (c) 1993-2016 by
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
 * mb_time.c includes the "mb_" functions used to translate between
 * various time formats.
 *
 * Author:	D. W. Caress
 * Date:	January 21, 1993
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_define.h"

/* time conversion constants and variables */
#define SECINYEAR 31536000.0
#define SECINDAY     86400.0
#define SECINHOUR     3600.0
#define SECINMINUTE     60.0
#define IMININHOUR 60
int	yday[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
/* 	function mb_get_time returns the number of seconds from
 * 	1/1/70 00:00:00 calculated from (yy/mm/dd/hr/mi/sc). */
int mb_get_time(int verbose, int time_i[7], double *time_d)
{
  char	*function_name = "mb_get_time";
	int	status;
	int	yearday;
	int	leapday;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose: %d\n",verbose);
		fprintf(stderr,"dbg2       year:    %d\n",time_i[0]);
		fprintf(stderr,"dbg2       month:   %d\n",time_i[1]);
		fprintf(stderr,"dbg2       day:     %d\n",time_i[2]);
		fprintf(stderr,"dbg2       hour:    %d\n",time_i[3]);
		fprintf(stderr,"dbg2       minute:  %d\n",time_i[4]);
		fprintf(stderr,"dbg2       second:  %d\n",time_i[5]);
		fprintf(stderr,"dbg2       microsec:%d\n",time_i[6]);
		}

	/* get time */
	yearday = yday[time_i[1]-1];
	if (((time_i[0] % 4 == 0 && time_i[0] % 100 != 0)
		|| time_i[0]%400==0)
		&& (time_i[1] > 2))
		yearday++;
	leapday = (time_i[0] - 1969)/4;
	*time_d = (time_i[0] - 1970)*SECINYEAR
		+ (yearday - 1 + leapday + time_i[2])*SECINDAY
		+ time_i[3]*SECINHOUR + time_i[4]*SECINMINUTE
		+ time_i[5] + 0.000001*time_i[6];

	/* assume success */
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       time_d:  %f\n",*time_d);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return success */
	return(status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_get_date returns yy/mm/dd/hr/mi/sc calculated
 * 	from the number of seconds after 1/1/70 00:00:0 */
int mb_get_date(int verbose, double time_d, int time_i[7])
{

	char	*function_name = "mb_get_date";
	int	status;
	int	i;
	int	daytotal;
	int	yearday;
	int	leapday;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose: %d\n",verbose);
		fprintf(stderr,"dbg2       time_d:  %f\n",time_d);
		}

	/* get the date */
	daytotal = (int) (time_d/SECINDAY);
	time_i[3] = (int) ((time_d - daytotal*SECINDAY)/SECINHOUR);
	time_i[4] = (int) ((time_d - daytotal*SECINDAY
			- time_i[3]*SECINHOUR)/SECINMINUTE);
	time_i[5] = (int) (time_d - daytotal*SECINDAY
			- time_i[3]*SECINHOUR - time_i[4]*SECINMINUTE);
	time_i[6] = (int) 1000000*(time_d - daytotal*SECINDAY
			- time_i[3]*SECINHOUR - time_i[4]*SECINMINUTE
			- time_i[5]);
	time_i[0] = (int) (time_d/SECINYEAR) + 1970;
	leapday = (time_i[0] - 1969)/4;
	yearday = daytotal - 365*(time_i[0] - 1970) - leapday + 1;
	if (yearday <= 0)
		{
		time_i[0]--;
		leapday = (time_i[0] - 1969)/4;
		yearday = daytotal - 365*(time_i[0] - 1970) - leapday + 1;
		}
	leapday = 0;
	if (((time_i[0] % 4 == 0 && time_i[0] % 100 != 0)
		|| time_i[0]%400 == 0)
		&& yearday > yday[2])
		leapday = 1;
	for (i=0;i<12;i++)
		if (yearday > (yday[i] + leapday))
			time_i[1] = i + 1;
	time_i[2] = yearday - yday[time_i[1]-1] - leapday;

	/* assume success */
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\nMBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       year:    %d\n",time_i[0]);
		fprintf(stderr,"dbg2       month:   %d\n",time_i[1]);
		fprintf(stderr,"dbg2       day:     %d\n",time_i[2]);
		fprintf(stderr,"dbg2       hour:    %d\n",time_i[3]);
		fprintf(stderr,"dbg2       minute:  %d\n",time_i[4]);
		fprintf(stderr,"dbg2       second:  %d\n",time_i[5]);
		fprintf(stderr,"dbg2       microsec:%d\n",time_i[6]);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return success */
	return(status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_get_date_string returns a string formated as:
 *          yyyy/mm/dd:hh:mm:ss.ssssss
 * 	from the number of seconds after 1/1/70 00:00:0 */
int mb_get_date_string(int verbose, double time_d, char *string)
{

	char	*function_name = "mb_get_date_string";
	int	status;
	int	time_i[7];

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose: %d\n",verbose);
		fprintf(stderr,"dbg2       time_d:  %f\n",time_d);
		}

	/* get the date */
	mb_get_date(verbose, time_d, time_i);
	sprintf(string, "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d",
		time_i[0], time_i[1], time_i[2], time_i[3],
		time_i[4], time_i[5], time_i[6]);

	/* assume success */
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\nMBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       string: %s\n",string);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return success */
	return(status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_get_jtime returns the day of year calculated
 *	from (yy/mm/dd/hr/mi/sc). */
int mb_get_jtime(int verbose, int time_i[7], int time_j[5])
{
	char	*function_name = "mb_get_jtime";
	int	status;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       year:       %d\n",time_i[0]);
		fprintf(stderr,"dbg2       month:      %d\n",time_i[1]);
		fprintf(stderr,"dbg2       day:        %d\n",time_i[2]);
		fprintf(stderr,"dbg2       hour:       %d\n",time_i[3]);
		fprintf(stderr,"dbg2       minute:     %d\n",time_i[4]);
		fprintf(stderr,"dbg2       second:     %d\n",time_i[5]);
		fprintf(stderr,"dbg2       microsecond:%d\n",time_i[6]);
		}

	/* get time with day of year */
	time_j[0] = time_i[0];
	time_j[1] = yday[time_i[1]-1] + time_i[2];
	if (((time_i[0] % 4 == 0 && time_i[0] % 100 != 0)
		|| time_i[0] % 400 == 0)
		&& (time_i[1] > 2))
		time_j[1]++;
	time_j[2] = time_i[3]*IMININHOUR + time_i[4];
	time_j[3] = time_i[5];
	time_j[4] = time_i[6];

	/* assume success */
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       year:       %d\n",time_j[0]);
		fprintf(stderr,"dbg2       day of year:%d\n",time_j[1]);
		fprintf(stderr,"dbg2       minute:     %d\n",time_j[2]);
		fprintf(stderr,"dbg2       second:     %d\n",time_j[3]);
		fprintf(stderr,"dbg2       microsecond:%d\n",time_j[4]);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return success */
	return(status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_get_itime returns the time in (yy/mm/dd/hr/mi/sc)
 *	calculated from the time in (yy/yd/hr/mi/sc) where yd is the
 *	day of the year.
 */
int mb_get_itime(int verbose, int time_j[5], int time_i[7])
{
	char	*function_name = "mb_get_itime";
	int	status;
	int	leapday;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       year:       %d\n",time_j[0]);
		fprintf(stderr,"dbg2       day of year:%d\n",time_j[1]);
		fprintf(stderr,"dbg2       minute:     %d\n",time_j[2]);
		fprintf(stderr,"dbg2       second:     %d\n",time_j[3]);
		fprintf(stderr,"dbg2       microsecond:%d\n",time_j[4]);
		}

	/* get the date */
	time_i[0] = time_j[0];
	time_i[3] = time_j[2]/IMININHOUR;
	time_i[4] = time_j[2] - time_i[3]*IMININHOUR;
	time_i[5] = time_j[3];
	time_i[6] = time_j[4];
	if (((time_j[0] % 4 == 0 && time_j[0] % 100 != 0)
		|| time_j[0] % 400 == 0)
		&& time_j[1] > yday[2])
		leapday = 1;
	else
		leapday = 0;
	time_i[1] = 0;
	for (i=0;i<12;i++)
		if (time_j[1] > (yday[i] + leapday))
			time_i[1] = i + 1;
	if(leapday==1 && time_j[1] == yday[2]+1)
		leapday=0;
	time_i[2] = time_j[1] - yday[time_i[1]-1] - leapday;

	/* assume success */
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       year:       %d\n",time_i[0]);
		fprintf(stderr,"dbg2       month:      %d\n",time_i[1]);
		fprintf(stderr,"dbg2       day:        %d\n",time_i[2]);
		fprintf(stderr,"dbg2       hour:       %d\n",time_i[3]);
		fprintf(stderr,"dbg2       minute:     %d\n",time_i[4]);
		fprintf(stderr,"dbg2       second:     %d\n",time_i[5]);
		fprintf(stderr,"dbg2       microsecond:%d\n",time_i[6]);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return success */
	return(status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_fix_y2k translates a two digit year value
 *      into a four digit year value using the following rule:
 *          if (year_short >= 62)
 *		year_long = year_short + 1900;
 *          else
 *              year_long = year_short + 2000;
 *      The rationale for this rule is that multibeam sonars were
 *      patented and first built in 1962. Thus, no digital swath
 *      data can have timestamps dating prior to 1962.
 */
int mb_fix_y2k(int verbose, int year_short, int *year_long)
{
	char	*function_name = "mb_fix_y2k";
	int	status;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       year_short: %d\n",year_short);
		}

	/* get the four digit year value */
	if (year_short >= 62)
	    *year_long = year_short + 1900;
	else
	    *year_long = year_short + 2000;

	/* assume success */
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       year_long:  %d\n",*year_long);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return success */
	return(status);
}
/*--------------------------------------------------------------------*/
/* 	function mb_unfix_y2k translates a four digit year value
 *      into a two digit year value using the following rule:
 *          if (year_long < 2000)
 *		year_short = year_long - 1900;
 *          else
 *              year_short = year_long - 2000;
 *      The rationale for this rule is that multibeam sonars were
 *      patented and first built in 1962. Thus, no digital swath
 *      data can have timestamps dating prior to 1962.
 *
 *      Of course, if you look below you will see that the
 *      same thing was accomplished using
 *          year_short = year_long % 100;
 */
int mb_unfix_y2k(int verbose, int year_long, int *year_short)
{
	char	*function_name = "mb_unfix_y2k";
	int	status;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       year_long:  %d\n",year_long);
		}

	/* get the two digit year value */
	*year_short = year_long % 100;

	/* assume success */
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       year_short: %d\n",*year_short);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return success */
	return(status);
}
/*--------------------------------------------------------------------*/
