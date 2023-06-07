/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : ShortData.cc                                                  */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#include "ShortData.h"
#include "ExternalData.h"
#include "StringConverter.h"

ShortData::ShortData(const char *name)
  : DataField(name)
{
  _value = 0;
  setAsciiFormat("%d");
}


ShortData::~ShortData()
{
}


void ShortData::setValue(short value)
{
  _value = value;
}


short ShortData::value()
{
  return _value;
}


void ShortData::write(ExternalData *externalData)
{
  externalData->set(this);
}


void ShortData::read(ExternalData *externalData)
{
  externalData->get(this);
}


const char *ShortData::ascii()
{
  snprintf(_asciiBuffer, SDATA_ASCII_BUFFER_BYTES, asciiFormat(), _value);
  return _asciiBuffer;
}


void ShortData::parseValue(const char *stringRep)
{
  // Note: may overflow short value!
  _value = (short )StringConverter::stringToInteger(stringRep);
}


const char *ShortData::typeMnemonic()
{
  return ShortTypeMnem;
}

