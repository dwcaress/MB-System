/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : TimeTag.h                                                     */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#ifndef _TIMETAG_H
#define _TIMETAG_H

#include "DoubleData.h"


#define TimeTagTypeMnem "timeTag"


/*
CLASS 
TimeTag

DESCRIPTION
Wrapper class for time data, which is stored internally as 'double';
hence TimeTag is subclassed from DoubleData. The ascii() method 
outputs a time string in human-readable format.

AUTHOR
Tom O'Reilly
*/
class TimeTag : public DoubleData {

public:

  TimeTag(const char *name);

  virtual ~TimeTag();

  ///////////////////////////////////////////////////////////////////
  // Data type mnemonic
  virtual const char *typeMnemonic();

  ///////////////////////////////////////////////////////////////////
  // Ascii representation of value
  virtual const char *ascii();

  ///////////////////////////////////////////////////////////////////
  // Set field's value, given string representation
  virtual void parseValue(const char *stringRep);

  ///////////////////////////////////////////////////////////////////
  // Compute month and day from Julian day-of-year
  // [input] doy: Julian day-of-year
  // [input] year: Year 
  // [output] month: Month (1-12)
  // [output] day: Day (1-31)
  int dayOfYearToMonthDay(int doy, int year, int *month, int *day);

  ///////////////////////////////////////////////////////////////////
  // True if specified year is a leap year
  // [input] year: Year
  Boolean leapYear(int year);


};

#endif


