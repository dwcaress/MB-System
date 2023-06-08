/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : CharData.cc                                                   */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#include "CharData.h"
#include "ExternalData.h"
#include "StringConverter.h"

CharData::CharData(const char *name)
  : DataField(name)
{
  _value = 0;
  setAsciiFormat("%c");
}


CharData::~CharData()
{
}


void CharData::setValue(char value)
{
  _value = value;
}


char CharData::value()
{
  return _value;
}


void CharData::write(ExternalData *externalData)
{
  externalData->set(this);
}


void CharData::read(ExternalData *externalData)
{
  externalData->get(this);
}


const char *CharData::ascii()
{
  snprintf(_asciiBuffer, ASCII_BUFFER_BYTES, asciiFormat(), _value);
  return _asciiBuffer;
}


void CharData::parseValue(const char *stringRep)
{

  if (strlen(stringRep) > 1) {
    char errorBuf[MAX_EXC_STRING_LEN];
      snprintf(errorBuf, MAX_EXC_STRING_LEN, 
	    "CharData::parseValue() - invalid representation: \"%s\"",
	    stringRep);

    throw Exception(errorBuf);
  }

  _value = stringRep[0];
}


const char *CharData::typeMnemonic()
{
  return CharTypeMnem;
}

