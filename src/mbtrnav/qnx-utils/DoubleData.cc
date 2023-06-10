/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : DoubleData.cc                                                 */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#include "DoubleData.h"
#include "ExternalData.h"
#include "StringConverter.h"

DoubleData::DoubleData(const char *name)
  : DataField(name)
{
  _value = 0.;
  setAsciiFormat("%8.8e");
}


DoubleData::DoubleData(const char *name, const char *longnm, const char *units)
  : DataField(name, longnm, units)
{
  _value = 0.;
  setAsciiFormat("%8.8e");
}


DoubleData::~DoubleData()
{
} 


void DoubleData::setValue(double value)
{
  _value = value;
}


double DoubleData::value()
{
  return _value;
}


void DoubleData::write(ExternalData *externalData)
{
  externalData->set(this);
}


void DoubleData::read(ExternalData *externalData)
{
  externalData->get(this);
}


const char *DoubleData::ascii()
{
  snprintf(_asciiBuffer, DBLDATA_ASCII_BUFFER_BYTES, asciiFormat(), _value);
  return _asciiBuffer;
}


void DoubleData::parseValue(const char *stringRep)
{
  _value = StringConverter::stringToFloat(stringRep);
}


const char *DoubleData::typeMnemonic()
{
  return DoubleTypeMnem;
}
