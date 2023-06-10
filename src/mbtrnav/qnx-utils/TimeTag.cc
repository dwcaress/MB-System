/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : TimeTag.cc                                                    */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "ourTypes.h"
#include "TimeTag.h"
#include "TimeP.h"
#include "StringConverter.h"
#include "Exception.h"

char _monthDays[2][13] =
{
  {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};


TimeTag::TimeTag(const char *name)
  : DoubleData(name)
{
}


TimeTag::~TimeTag()
{
}


const char *TimeTag::typeMnemonic()
{
  return TimeTagTypeMnem;
}


/* *************************************************************************
NOTE:
TimeTag::parseValue() implementation depends on format of string generated
by TimeTag::ascii()
*****************************************************************************/
const char *TimeTag::ascii()
{
  time_t secs = (time_t )_value;

  // Converting to UTC
  char tzone[]="TZ=UTC0";
  putenv(tzone);

  struct tm *timePtr = gmtime(&secs);

  if (strftime(_asciiBuffer, sizeof(_asciiBuffer), "%Y:%j:%H:%M:%S",
	       timePtr) <= 0) {
    printf("TimeTag::ascii() - %s\n", strerror(errno));
  }

  char *ptr = _asciiBuffer + strlen(_asciiBuffer);

  double mantissa = _value - floor(_value);

  snprintf(ptr, 4, ".%02d", (int )(mantissa * 100));


  return _asciiBuffer;
}



void TimeTag::parseValue(const char *stringRep)
{

  enum parseState {
    ParseYear, ParseDayOfYear, ParseHours, ParseMinutes, ParseSeconds, Done
  } state;

  struct tm timeStruct;
  char buf[100];
  char errorBuf[MAX_EXC_STRING_LEN];
  double seconds;

  strncpy(buf, stringRep, sizeof(buf)-1);

  state = ParseYear;
  char *token;
  char *ptr = buf;

  while((token = strtok(ptr, ": ")) != 0) {

    ptr = 0;

    switch (state) {

    case ParseYear:
      if (!StringConverter::isInteger(token)) {
	snprintf(errorBuf, sizeof(errorBuf),
		"TimeTag::parseValue() - invalid year: \"%s\"", token);
	throw Exception(errorBuf);
      }
      timeStruct.tm_year = atoi(token) - 1900;

      state = ParseDayOfYear;
      break;

    case ParseDayOfYear:
      if (!StringConverter::isInteger(token)) {
	snprintf(errorBuf, sizeof(errorBuf),
		"TimeTag::parseValue() - invalid day-of-year: \"%s\"", token);
	throw Exception(errorBuf);
      }

      int month;

      dayOfYearToMonthDay(atoi(token),
				timeStruct.tm_year,
				&month,
				&timeStruct.tm_mday);

      // tm_mon range is from 0-11!!!
      timeStruct.tm_mon = month - 1;

      state = ParseHours;
      break;

    case ParseHours:
      if (!StringConverter::isInteger(token)) {
	snprintf(errorBuf, sizeof(errorBuf),
		"TimeTag::parseValue() - invalid hours: \"%s\"", token);
	throw Exception(errorBuf);
      }
      timeStruct.tm_hour = atoi(token);

      state = ParseMinutes;
      break;

    case ParseMinutes:
      if (!StringConverter::isInteger(token)) {
	snprintf(errorBuf, sizeof(errorBuf),
		"TimeTag::parseValue() - invalid minutes: \"%s\"", token);
	throw Exception(errorBuf);
      }
      timeStruct.tm_min = atoi(token);

      state = ParseSeconds;
      break;

    case ParseSeconds:

      if (!StringConverter::isFloat(token)) {
	snprintf(errorBuf, sizeof(errorBuf),
		"TimeTag::parseValue() - invalid seconds: \"%s\"", token);
	throw Exception(errorBuf);
      }

      seconds = atof(token);

      timeStruct.tm_sec = (int )seconds;

      state = Done;
      break;

    default:
      snprintf(errorBuf, sizeof(errorBuf), 
        "TimeTag::parseValue() - extra tokens in \"%s\"",
	      stringRep);

      throw Exception(errorBuf);
    }
  }

  if (state != Done) {
    snprintf(errorBuf, sizeof(errorBuf),
	    "TimeTag::parseValue() - bad number of tokens in \"%s\"",
	    stringRep);

    throw Exception(errorBuf);
  }

  // Converting to UTC
  char tzone[]="TZ=UTC0";
  putenv(tzone);
  time_t t = mktime(&timeStruct);

  // Subtract integer seconds then add floating point seconds
  _value = t - timeStruct.tm_sec + seconds;
}

Boolean TimeTag::leapYear(int year)
{
  return (((year % 4) == 0) && ((year % 100) != 0)) || ((year % 400) == 0);
}

int TimeTag::dayOfYearToMonthDay(int doy, int year, int *month, int *day)
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
