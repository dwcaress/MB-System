/*--------------------------------------------------------------------
 *    The MB-system:	mb_time.c	1/21/93
 *    $Id: mb_time.c,v 4.13 2000-09-30 06:32:11 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000 by
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
 * $Log: not supported by cvs2svn $
 * Revision 4.12  2000/03/06  21:53:54  caress
 * Implemented Suzanne Ohara's fixes to Y2K leapday problems.
 *
 * Revision 4.11  1999/03/31  18:11:35  caress
 * MB-System 4.6beta7
 *
 * Revision 4.10  1998/10/05  18:32:27  caress
 * MB-System version 4.6beta
 *
 * Revision 4.9  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.9  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.8  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.8  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.7  1996/03/11  15:33:52  caress
 * Fixed handling of leap days as per note from Suzanne O'Hara
 * on the R/V Palmer.
 *
 * Revision 4.6  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.5  1995/02/14  21:59:53  caress
 * Initialize time_i[1] in mb_get_itime() to avoid core dump when
 * doing mbmerge on format 54 data.
 *
 * Revision 4.4  1995/01/03  22:49:33  caress
 * Fixed bug in mb_get_date() as per David Brock of ASA.
 *
 * Revision 4.3  1994/10/21  12:11:53  caress
 * Release V4.0
 *
 * Revision 4.2  1994/07/29  18:46:51  caress
 * Changes associated with supporting Lynx OS (byte swapped) and
 * using unix second time base (for time_d values).
 *
 * Revision 4.1  1994/04/27  23:37:06  caress
 * Changed reference to time_d value; time_d now defined as
 * minutes since 1/1/1971 00:00:00.
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.1  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.0  1994/02/21  04:04:35  caress
 * First cut at new version.  No changes.
 *
 * Revision 3.2  1993/05/15  14:40:30  caress
 * removed excess rcs_id messages
 *
 * Revision 3.1  1993/05/14  22:43:31  sohara
 * fixed rcs_id message
 *
 * Revision 3.0  1993/04/23  19:00:00  dale
 * Initial version
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"

/* time conversion constants and variables */
#define SECINYEAR 31536000.0
#define SECINDAY     86400.0
#define SECINHOUR     3600.0
#define SECINMINUTE     60.0
#define IMININHOUR 60
int	yday[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
static char rcs_id[]="$Id: mb_time.c,v 4.13 2000-09-30 06:32:11 caress Exp $";

/*--------------------------------------------------------------------*/
/* 	function mb_get_time returns the number of seconds from
 * 	1/1/70 00:00:00 calculated from (yy/mm/dd/hr/mi/sc). */
int mb_get_time(verbose,time_i,time_d)
int verbose;
int time_i[7];
double *time_d;
{
  char	*function_name = "mb_get_time";
	int	status;
	int	yearday;
	int	leapday;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
int mb_get_date(verbose,time_d,time_i)
int verbose;
double time_d;
int time_i[7];
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
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
		fprintf(stderr,"\nMBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:");
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
/* 	function mb_get_jtime returns the day of year calculated 
 *	from (yy/mm/dd/hr/mi/sc). */
int mb_get_jtime(verbose,time_i,time_j)
int verbose;
int time_i[7];
int time_j[5];
{
	char	*function_name = "mb_get_jtime";
	int	status;
	int	leapday;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
int mb_get_itime(verbose,time_j,time_i)
int verbose;
int time_j[5];
int time_i[7];
{
	char	*function_name = "mb_get_itime";
	int	status;
	int	leapday;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
int mb_fix_y2k(verbose,year_short,year_long)
int verbose;
int year_short;
int *year_long;
{
	char	*function_name = "mb_fix_y2k";
	int	status;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
int mb_unfix_y2k(verbose,year_long,year_short)
int verbose;
int year_long;
int *year_short;
{
	char	*function_name = "mb_unfix_y2k";
	int	status;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       year_short: %d\n",*year_short);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return success */
	return(status);
}
/*--------------------------------------------------------------------*/
