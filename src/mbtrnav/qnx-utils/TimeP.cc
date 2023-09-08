/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : TimeP.cc                                                       */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "TimeP.h"

#define TPDEBUG 1


char TimeP::_monthDays[2][13] =
{
  {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};


const int TimeP::SecondsPerDay = 86400;
const int TimeP::SecondsPerHour = 3600;
const int TimeP::SecondsPerMinute = 60;

TimeP::TimeP()
{
    if (TPDEBUG) printf("\n****TimeP::TimeP ctor[%p]\n",this);
}

TimeP::~TimeP()
{
    if (TPDEBUG) printf("\n****TimeP::TimeP dtor[%p]\n",this);
}
void TimeP::cleanup(){
}


Boolean TimeP::_epochAssigned = False;
struct timespec TimeP::_epoch = {0, 0};

double TimeP::seconds(timespec *timeSpec)
{
  return (timeSpec->tv_sec + timeSpec->tv_nsec / 1.e9);
}

double TimeP::seconds(TimeIF::TimeSpec *timeSpec)
{
  return (timeSpec->seconds + timeSpec->nanoSeconds / 1.e9);
}


unsigned long TimeP::milliseconds()
{
  struct timespec timeNow;

  TimeP::gettime(&timeNow);

  if (!_epochAssigned) {
    // Initialize epoch the first time through
    _epoch.tv_sec  = timeNow.tv_sec;
    _epoch.tv_nsec = timeNow.tv_nsec;
    _epochAssigned = True;
  }

  long delta =
    (timeNow.tv_sec - _epoch.tv_sec) * 1000 +
      (timeNow.tv_nsec - _epoch.tv_nsec) / 1.e6;

  return (unsigned long )delta;
}

void TimeP::gettime(struct timespec *timeSpec)
{
   	clock_gettime(CLOCK_REALTIME, timeSpec);

//  printf(":\t\t\t\tTimeP::gettime() - seconds %ld\n", timeSpec->tv_sec);
}

void TimeP::gettime(TimeIF::TimeSpec *timeSpec)
{
  struct timespec timeNow;
  TimeP::gettime(&timeNow);
  timeSpec->seconds     = timeNow.tv_sec;
  timeSpec->nanoSeconds = timeNow.tv_nsec;
}

void TimeP::getEpoch(timespec *timeSpec)
{
  timeSpec->tv_sec     = _epoch.tv_sec;
  timeSpec->tv_nsec    = _epoch.tv_nsec;
}



void TimeP::secsToHourMinSec(double secs, char *timestring)
{
  unsigned int days;
  unsigned int hrs;
  unsigned int min;
  unsigned int isecs;

  isecs = secs;

  if ((days = isecs / SecondsPerDay) > 0)
  {
    isecs = isecs % SecondsPerDay;
  }
  if ((hrs = isecs / SecondsPerHour) > 0)
  {
    isecs = isecs % SecondsPerHour;
  }
  if ((min = isecs / SecondsPerMinute) > 0)
  {
    isecs = isecs % SecondsPerMinute;
  }
  secs = isecs + (secs - (int )secs);

  snprintf(timestring, 15, "%03d:%02d:%02d:%02.1f", days, hrs, min, secs);

  return;
}


int TimeP::hourMinSecToSecs(char *timestring, double *secs)
{
  char *token;
  char buf[100];
  char *bufptr;
  int nflds = 0;
  double val[4];

  strncpy(buf, timestring, sizeof(buf)-1);
  bufptr = buf;
  while ((token = strtok(bufptr, ":")) != NULL)
  {
    bufptr = NULL;
    if (sscanf(token, "%lf", &val[nflds]) <= 0)
    {
      return -1;
    }
    nflds++;
    if (nflds > 4)
      return -1;
  }

  *secs = 0.;

  switch (nflds)
  {
    case 4:
    /* Specified days, hours, mins, secs */
    *secs += val[0] * SecondsPerDay;
    *secs += val[1] * SecondsPerHour;
    *secs += val[2] * SecondsPerMinute;
    *secs += val[3];
    return 0;
    break;

    case 3:
    /* Specified hours, mins, secs */
    *secs += val[0] * SecondsPerHour;
    *secs += val[1] * SecondsPerMinute;
    *secs += val[2];
    return 0;
    break;


    default:
    return -1;

  }
}


Boolean TimeP::leapYear(int year)
{
  return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}


int TimeP::daysInYear(int year)
{
  if (leapYear(year))
    return 366;
  else
    return 365;
}


int TimeP::dayOfYear(int year, int month, int day)
{
  int i, leap;
  if ( (year < 0) || (month < 1) || (month > 12) || (day < 1) || (day > 31))
    return -1;

  leap = ((year % 4 == 0) && (year % 100 != 0)) || ( (year % 400) == 0);
  for (i = 1; i < month; i++)
    day += _monthDays[leap][i];

  return day;
}


int TimeP::dayOfYearToMonthDay(int doy, int year, int *month, int *day)
{
  Boolean leap;
  int sum;

  leap = leapYear(year);
  if (doy < 1 || (!leap && doy > 365) || (leap && doy > 366))
    return -1;

  *month = 0;
  for (sum = 0, *month = 0; sum + _monthDays[leap][*month] < doy;
       sum += _monthDays[leap][*month], (*month)++)
  {
    ;
  }
  *day = doy - sum;
  return 0;
}
