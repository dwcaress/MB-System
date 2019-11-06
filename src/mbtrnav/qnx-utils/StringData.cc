/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : StringData.cc                                                   */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/

#if defined(__APPLE__)
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#include "StringData.h"
#include "ExternalData.h"
#include "StringConverter.h"

StringData::StringData(const char *name)
  : DataField(name)
{
  _string = 0;
  setAsciiFormat("%s");
}


StringData::~StringData()
{
  free((void *)_string);
}


void StringData::setValue(char *string)
{
  free((void *)_string);
  _string = strdup(string);
}


char *StringData::value()
{
  return _string;
}


void StringData::write(ExternalData *externalData)
{
  externalData->set(this);
}


void StringData::read(ExternalData *externalData)
{
  externalData->get(this);
}


const char *StringData::ascii()
{
  return _string;
}


void StringData::parseValue(const char *stringRep)
{
  free((void *)_string);
  _string = strdup(stringRep);
}


const char *StringData::typeMnemonic()
{
  return StringTypeMnem;
}

