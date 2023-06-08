/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : IntegerData.cc                                                */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#include "IntegerData.h"
#include "ExternalData.h"
#include "StringConverter.h"

IntegerData::IntegerData(const char *name)
  : DataField(name)
{
  _value = 0;
  setAsciiFormat("%d");
}


IntegerData::~IntegerData()
{
}


void IntegerData::setValue(int value)
{
  _value = value;
}


int IntegerData::value()
{
  return _value;
}


void IntegerData::write(ExternalData *externalData)
{
  externalData->set(this);
}


void IntegerData::read(ExternalData *externalData)
{
  externalData->get(this);
}


const char *IntegerData::ascii()
{
  snprintf(_asciiBuffer, IDATA_ASCII_BUFFER_BYTES, asciiFormat(), _value);
  return _asciiBuffer;
}


void IntegerData::parseValue(const char *stringRep)
{
  _value = StringConverter::stringToInteger(stringRep);
}


const char *IntegerData::typeMnemonic()
{
  return IntegerTypeMnem;
}

