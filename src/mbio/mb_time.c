/*--------------------------------------------------------------------
 *    The MB-system:  mb_time.c  1/21/93
 *
 *    Copyright (c) 1993-2023 by
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
 * mb_time.c includes the "mb_" functions used to translate between
 * various time formats.
 *
 * MB-System uses these functions rather than system time functions in order to
 * insure that no time zone corrections are made. These functions make no
 * presumption about the time standard in use (e.g. GPS time, UTC time, time in
 * any particular time zone) because the time will be whatever was used for the
 * data during logging. Leap-days are handled, but no implementation of or
 * provision for leap-seconds is made in this code.
 *
 * Author:  D. W. Caress
 * Date:  January 21, 1993
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "mb_define.h"
#include "mb_status.h"

/* year-day conversion */
const int yday[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

/*--------------------------------------------------------------------*/
/*   function mb_get_time returns the number of seconds from
 *   1/1/70 00:00:00 calculated from (yy/mm/dd/hr/mi/sc). */
int mb_get_time(int verbose, int time_i[7], double *time_d) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose: %d\n", verbose);
    fprintf(stderr, "dbg2       year:    %d\n", time_i[0]);
    fprintf(stderr, "dbg2       month:   %d\n", time_i[1]);
    fprintf(stderr, "dbg2       day:     %d\n", time_i[2]);
    fprintf(stderr, "dbg2       hour:    %d\n", time_i[3]);
    fprintf(stderr, "dbg2       minute:  %d\n", time_i[4]);
    fprintf(stderr, "dbg2       second:  %d\n", time_i[5]);
    fprintf(stderr, "dbg2       microsec:%d\n", time_i[6]);
    fprintf(stderr, "dbg2       time_d:  %p\n", time_d);
    fprintf(stderr, "dbg2       *time_d: %f\n", *time_d);
  }

  int status = MB_SUCCESS;

  // See http://www.cplusplus.com/reference/ctime/tm/
  if (time_i[0] < 1930 || time_i[0] > 3000       // year
      || time_i[1] < 1 || time_i[1] > 12         // month counted from 1
      || time_i[2] < 1 || time_i[2] > 31         // day counted from 1
      || time_i[3] < 0 || time_i[3] > 23         // hour counted from 0
      || time_i[4] < 0 || time_i[4] > 59         // minute counted from 0
      || time_i[5] < 0 || time_i[5] > 59         // second counted from 0
      || time_i[6] < 0 || time_i[6] > 999999) {  // microsecond counted from 0
    if (verbose > 0
        && (time_i[0] != 0 || time_i[1] != 0 || time_i[2] != 0 || time_i[3] != 0
            || time_i[4] != 0 || time_i[5] != 0 || time_i[6] != 0)) {
      fprintf(stderr, "\nWarning in MB-System function %s: invalid time values:\n", __func__);
      fprintf(stderr, "\tyear:         %d\n", time_i[0]);
      fprintf(stderr, "\tmonth:        %d\n", time_i[1]);
      fprintf(stderr, "\tday:          %d\n", time_i[2]);
      fprintf(stderr, "\thour:         %d\n", time_i[3]);
      fprintf(stderr, "\tminute:       %d\n", time_i[4]);
      fprintf(stderr, "\tsecond:       %d\n", time_i[5]);
      fprintf(stderr, "\tmicrosecond:  %d\n", time_i[6]);
    }
    *time_d = 0.0;
    status = MB_FAILURE;
  }

  /* get time */
  else {
    int yearday = yday[time_i[1] - 1];
    if (((time_i[0] % 4 == 0 && time_i[0] % 100 != 0) || time_i[0] % 400 == 0) && (time_i[1] > 2))
      yearday++;
    const int leapday = (time_i[0] - 1969) / 4;
    *time_d = (time_i[0] - 1970) * MB_SECINYEAR
                + (yearday - 1 + leapday + time_i[2]) * MB_SECINDAY
                + time_i[3] * MB_SECINHOUR
                + time_i[4] * MB_SECINMINUTE
                + time_i[5]
                + 0.000001 * time_i[6];
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       time_d:  %f\n", *time_d);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
/*   function mb_get_date returns yy/mm/dd/hr/mi/sc calculated
 *   from the number of seconds after 1/1/70 00:00:0 */
int mb_get_date(int verbose, double time_d, int time_i[7]) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose: %d\n", verbose);
    fprintf(stderr, "dbg2       time_d:  %f\n", time_d);
  }

  /* get the date */
  const int daytotal = (int)(time_d / MB_SECINDAY);
  time_i[3] = (int)((time_d - daytotal * MB_SECINDAY) / MB_SECINHOUR);
  time_i[4] = (int)((time_d - daytotal * MB_SECINDAY - time_i[3] * MB_SECINHOUR) / MB_SECINMINUTE);
  time_i[5] = (int)(time_d - daytotal * MB_SECINDAY - time_i[3] * MB_SECINHOUR - time_i[4] * MB_SECINMINUTE);
  time_i[6] =
      (int)1000000 * (time_d - daytotal * MB_SECINDAY - time_i[3] * MB_SECINHOUR - time_i[4] * MB_SECINMINUTE - time_i[5]);
  time_i[0] = (int)(time_d / MB_SECINYEAR) + 1970;
  int leapday = (time_i[0] - 1969) / 4;
  int yearday = daytotal - 365 * (time_i[0] - 1970) - leapday + 1;
  if (yearday <= 0) {
    time_i[0]--;
    leapday = (time_i[0] - 1969) / 4;
    yearday = daytotal - 365 * (time_i[0] - 1970) - leapday + 1;
  }
  leapday = 0;
  if (((time_i[0] % 4 == 0 && time_i[0] % 100 != 0) || time_i[0] % 400 == 0) && yearday > yday[2])
    leapday = 1;
  for (int i = 0; i < 12; i++)
    if (yearday > (yday[i] + leapday))
      time_i[1] = i + 1;
  time_i[2] = yearday - yday[time_i[1] - 1] - leapday;

  /* assume success */
  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\nMBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       year:    %d\n", time_i[0]);
    fprintf(stderr, "dbg2       month:   %d\n", time_i[1]);
    fprintf(stderr, "dbg2       day:     %d\n", time_i[2]);
    fprintf(stderr, "dbg2       hour:    %d\n", time_i[3]);
    fprintf(stderr, "dbg2       minute:  %d\n", time_i[4]);
    fprintf(stderr, "dbg2       second:  %d\n", time_i[5]);
    fprintf(stderr, "dbg2       microsec:%d\n", time_i[6]);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
/*   function mb_get_date_string returns a string formatted as:
 *          yyyy/mm/dd:hh:mm:ss.ssssss
 *   from the number of seconds after 1/1/70 00:00:0 */
int mb_get_date_string(int verbose, double time_d, char *string) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose: %d\n", verbose);
    fprintf(stderr, "dbg2       time_d:  %f\n", time_d);
  }

  int time_i[7];
  mb_get_date(verbose, time_d, time_i);
  sprintf(string, "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5],
          time_i[6]);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\nMBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       string: %s\n", string);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
/*   function mb_get_jtime returns the day of year calculated
 *  from (yy/mm/dd/hr/mi/sc). */
int mb_get_jtime(int verbose, int time_i[7], int time_j[5]) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose: %d\n", verbose);
    fprintf(stderr, "dbg2       year:    %d\n", time_i[0]);
    fprintf(stderr, "dbg2       month:   %d\n", time_i[1]);
    fprintf(stderr, "dbg2       day:     %d\n", time_i[2]);
    fprintf(stderr, "dbg2       hour:    %d\n", time_i[3]);
    fprintf(stderr, "dbg2       minute:  %d\n", time_i[4]);
    fprintf(stderr, "dbg2       second:  %d\n", time_i[5]);
    fprintf(stderr, "dbg2       microsec:%d\n", time_i[6]);
  }

  int status = MB_SUCCESS;

  // See http://www.cplusplus.com/reference/ctime/tm/
  if (time_i[0] < 1930 || time_i[0] > 3000       // year
      || time_i[1] < 1 || time_i[1] > 12         // month counted from 1
      || time_i[2] < 1 || time_i[2] > 31         // day counted from 1
      || time_i[3] < 0 || time_i[3] > 23         // hour counted from 0
      || time_i[4] < 0 || time_i[4] > 59         // minute counted from 0
      || time_i[5] < 0 || time_i[5] > 59         // second counted from 0
      || time_i[6] < 0 || time_i[6] > 999999) {  // microsecond counted from 0
    if (verbose > 0
        && (time_j[0] != 0 || time_j[1] != 0 || time_j[2] != 0 || time_j[3] != 0
            || time_j[4] != 0 )) {
      fprintf(stderr, "\nWarning in MB-System function %s: invalid time values:\n", __func__);
      fprintf(stderr, "\tyear:         %d\n", time_i[0]);
      fprintf(stderr, "\tmonth:        %d\n", time_i[1]);
      fprintf(stderr, "\tday:          %d\n", time_i[2]);
      fprintf(stderr, "\thour:         %d\n", time_i[3]);
      fprintf(stderr, "\tminute:       %d\n", time_i[4]);
      fprintf(stderr, "\tsecond:       %d\n", time_i[5]);
      fprintf(stderr, "\tmicrosecond:  %d\n", time_i[6]);
    }
    time_j[0] = 0;
    time_j[1] = 0;
    time_j[2] = 0;
    time_j[3] = 0;
    time_j[4] = 0;
    status = MB_FAILURE;
  }

  /* get time */
  else {

    /* get time with day of year */
    time_j[0] = time_i[0];
    time_j[1] = yday[time_i[1] - 1] + time_i[2];
    if (((time_i[0] % 4 == 0 && time_i[0] % 100 != 0) || time_i[0] % 400 == 0) && (time_i[1] > 2))
      time_j[1]++;
    time_j[2] = time_i[3] * MB_IMININHOUR + time_i[4];
    time_j[3] = time_i[5];
    time_j[4] = time_i[6];
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       year:       %d\n", time_j[0]);
    fprintf(stderr, "dbg2       day of year:%d\n", time_j[1]);
    fprintf(stderr, "dbg2       minute:     %d\n", time_j[2]);
    fprintf(stderr, "dbg2       second:     %d\n", time_j[3]);
    fprintf(stderr, "dbg2       microsecond:%d\n", time_j[4]);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
/*   function mb_get_itime returns the time in (yy/mm/dd/hr/mi/sc)
 *  calculated from the time in (yy/yd/hr/mi/sc) where yd is the
 *  day of the year.
 */
int mb_get_itime(int verbose, int time_j[5], int time_i[7]) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       year:       %d\n", time_j[0]);
    fprintf(stderr, "dbg2       day of year:%d\n", time_j[1]);
    fprintf(stderr, "dbg2       minute:     %d\n", time_j[2]);
    fprintf(stderr, "dbg2       second:     %d\n", time_j[3]);
    fprintf(stderr, "dbg2       microsecond:%d\n", time_j[4]);
  }

  int status = MB_SUCCESS;

  // See http://www.cplusplus.com/reference/ctime/tm/
  if (time_j[0] < 1930 || time_j[0] > 3000       // year
      || time_j[1] < 1 || time_j[1] > 366        // yearday counted from 1
      || time_j[2] < 0 || time_j[2] > 1439       // minute counted from 0
      || time_j[3] < 0 || time_j[3] > 59         // second counted from 0
      || time_j[4] < 0 || time_j[4] > 999999) {  // microsecond counted from 0
    if (verbose > 0
        && (time_j[0] != 0 || time_j[1] != 0 || time_j[2] != 0 || time_j[3] != 0
            || time_j[4] != 0 )) {
      fprintf(stderr, "\nWarning in MB-System function %s: invalid time values:\n", __func__);
      fprintf(stderr, "\tyear:         %d\n", time_j[0]);
      fprintf(stderr, "\tyearday:      %d\n", time_j[1]);
      fprintf(stderr, "\tdayminute:    %d\n", time_j[2]);
      fprintf(stderr, "\tsecond:       %d\n", time_j[3]);
      fprintf(stderr, "\tmicrosecond:  %d\n", time_j[4]);
    }
    time_i[0] = 0;
    time_i[1] = 0;
    time_i[2] = 0;
    time_i[3] = 0;
    time_i[4] = 0;
    time_i[5] = 0;
    time_i[6] = 0;
    status = MB_FAILURE;
  }

  /* get time */
  else {

    /* get the date */
    time_i[0] = time_j[0];
    time_i[3] = time_j[2] / MB_IMININHOUR;
    time_i[4] = time_j[2] - time_i[3] * MB_IMININHOUR;
    time_i[5] = time_j[3];
    time_i[6] = time_j[4];
    int leapday;
    if (((time_j[0] % 4 == 0 && time_j[0] % 100 != 0) || time_j[0] % 400 == 0) && time_j[1] > yday[2])
      leapday = 1;
    else
      leapday = 0;
    time_i[1] = 0;
    for (int i = 0; i < 12; i++)
      if (time_j[1] > (yday[i] + leapday))
        time_i[1] = i + 1;
    if (leapday == 1 && time_j[1] == yday[2] + 1)
      leapday = 0;
    time_i[2] = time_j[1] - yday[time_i[1] - 1] - leapday;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       year:       %d\n", time_i[0]);
    fprintf(stderr, "dbg2       month:      %d\n", time_i[1]);
    fprintf(stderr, "dbg2       day:        %d\n", time_i[2]);
    fprintf(stderr, "dbg2       hour:       %d\n", time_i[3]);
    fprintf(stderr, "dbg2       minute:     %d\n", time_i[4]);
    fprintf(stderr, "dbg2       second:     %d\n", time_i[5]);
    fprintf(stderr, "dbg2       microsecond:%d\n", time_i[6]);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
/*   function mb_fix_y2k translates a two digit year value
 *      into a four digit year value using the following rule:
 *          if (year_short >= 62)
 *    year_long = year_short + 1900;
 *          else
 *              year_long = year_short + 2000;
 *      The rationale for this rule is that multibeam sonars were
 *      patented and first built in 1962. Thus, no digital swath
 *      data can have timestamps dating prior to 1962.
 */
int mb_fix_y2k(int verbose, int year_short, int *year_long) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       year_short: %d\n", year_short);
  }

  /* get the four digit year value */
  if (year_short >= 62)
    *year_long = year_short + 1900;
  else
    *year_long = year_short + 2000;

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       year_long:  %d\n", *year_long);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
/*   function mb_unfix_y2k translates a four digit year value
 *      into a two digit year value using the following rule:
 *          if (year_long < 2000)
 *    year_short = year_long - 1900;
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
int mb_unfix_y2k(int verbose, int year_long, int *year_short) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       year_long:  %d\n", year_long);
  }

  /* get the two digit year value */
  *year_short = year_long % 100;

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       year_short: %d\n", *year_short);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
