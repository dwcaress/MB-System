/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : Time.h                                                        */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#ifndef _TIMEP_H
#define _TIMEP_H

#include <time.h>
#include "ourTypes.h"
#include "TimeIF.h"

/*
CLASS 
Time

DESCRIPTION
Various static utility methods for time 

AUTHOR
Tom O'Reilly
*/

class TimeP {

public:
    TimeP();
    virtual ~TimeP();

  static const int SecondsPerDay;
  static const int SecondsPerHour;
  static const int SecondsPerMinute;

  ///////////////////////////////////////////////////////////////////
  // Double-precision seconds represented by input timespec structure
  // [input] timeSpec: timespec structure representation of time
  static double seconds(timespec *timeSpec);
  static double seconds(TimeIF::TimeSpec *timeSpec);

  ///////////////////////////////////////////////////////////////////
  // returns system time in milliseconds
  //
  // WARNING: This will not be consistent between processes.
  // Each process will have it's own epoch.  That is, milliseconds()
  // returns the time since that particular process started.  A different
  // process may have an offset.
  static unsigned long milliseconds();
  static void getEpoch(timespec *timeSpec);

  ///////////////////////////////////////////////////////////////////
  // returns system time in seconds and nanoseconds 
  static void gettime(TimeIF::TimeSpec *timeSpec);
  static void gettime(struct  timespec *timeSpec);

  ///////////////////////////////////////////////////////////////////
  // Convert from seconds to timestring in dd:hh:mm:ss format
  // Return values: -1 on error, else 0
  // [input] secs: Seconds from arbitrary epoch
  // [output] timeString: Time string in dd:hh:mm:ss.x format
  static void secsToHourMinSec(double secs, char *timeString);

  ///////////////////////////////////////////////////////////////////
  // Convert timestring in dd:hh:mm:ss format to seconds
  // Return values: -1 on error, else 0
  // [input] timeString: Time string in dd:hh:mm:ss.x format from 
  //                     arbitrary epoch
  // [output] seconds: Seconds from arbitrary epoch
  static int hourMinSecToSecs(char *timeString, double *seconds);

  ///////////////////////////////////////////////////////////////////
  // True if specified year is a leap year
  // [input] year: Year
  static Boolean leapYear(int year);

  ///////////////////////////////////////////////////////////////////
  // Number of days in specified year (365 or 366)
  // [input] year: Year
  static int daysInYear(int year);

  ///////////////////////////////////////////////////////////////////
  // Compute Julian day-of-year (0-366) for input year, month, and day.
  // Return values: -1 if invalid year, month, or day; 
  // else day-of-year
  // [input] year: Year (valid > 0)
  // [input] month: Month (valid 1-12)
  // [input] day: Day (valid (1-31)
  static int dayOfYear(int year, int month, int day);

  ///////////////////////////////////////////////////////////////////
  // Compute month and day from Julian day-of-year
  // [input] doy: Julian day-of-year
  // [input] year: Year 
  // [output] month: Month (1-12)
  // [output] day: Day (1-31)
  static int dayOfYearToMonthDay(int doy, int year, int *month, int *day);

  static  void cleanup();
    
private:
  static char _monthDays[2][13];
  static struct timespec _epoch;
  static Boolean _epochAssigned;

};


#endif
