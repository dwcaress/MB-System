/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : AngleData.cc                                                  */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#include <string.h>
#include "AngleData.h"
#include "MathP.h"

AngleData::AngleData(const char *name)
  : DoubleData(name)
{
}


AngleData::~AngleData()
{
}


const char *AngleData::typeMnemonic()
{
  return AngleTypeMnem;
}


const char *AngleData::ascii()
{
  // Convert to degrees and print
  snprintf(_asciiBuffer, DBLDATA_ASCII_BUFFER_BYTES, asciiFormat(), _value / Math::RadsPerDeg);
  return _asciiBuffer;
}



void AngleData::parseValue(const char *stringRep)
{
  // Parse value in degrees 
  DoubleData::parseValue(stringRep);

  // Convert to radians
  _value *= Math::RadsPerDeg;
}

