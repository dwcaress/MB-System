/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : FloatData.cc                                                  */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#include "FloatData.h"
#include "ExternalData.h"
#include "StringConverter.h"

FloatData::FloatData(const char *name)
  : DataField(name)
{
  _value = 0.;
  setAsciiFormat("%f");
}


FloatData::~FloatData()
{
}


void FloatData::setValue(float value)
{
  _value = value;
}


float FloatData::value()
{
  return _value;
}


void FloatData::write(ExternalData *externalData)
{
  externalData->set(this);
}


void FloatData::read(ExternalData *externalData)
{
  externalData->get(this);
}


const char *FloatData::ascii()
{
  snprintf(_asciiBuffer, FLTDATA_ASCII_BUFFER_BYTES, asciiFormat(), _value);
  return _asciiBuffer;
}


void FloatData::parseValue(const char *stringRep)
{
  _value = StringConverter::stringToFloat(stringRep);
}


const char *FloatData::typeMnemonic()
{
  return FloatTypeMnem;
}
