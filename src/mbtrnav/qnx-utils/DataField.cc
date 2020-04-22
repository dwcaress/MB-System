/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : DataField.cc                                                  */
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

#if defined(__APPLE__)
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#include "DataField.h"


DataField::DataField(const char *name)
{
  setName(name);
  setLongName(name);
  setUnits("UNKNOWN");
}


DataField::DataField(const char *name, const char *lname, const char *units)
{
  setName(name);
  setLongName(lname);
  setUnits(units);
}


DataField::~DataField()
{
}


const char *DataField::name()
{
  return (const char *)_name;
}

const char *DataField::longName()
{
  return (const char *)_longName;
}

const char *DataField::units()
{
  return (const char *)_units;
}


void DataField::setName(const char *newName)
{
  if (newName != NULL)
  {
    // Just use the first token in the name
    //
    char *nameCpy = strdup(newName);
    char *token = strtok(nameCpy, " \t");
    strncpy((char *)_name, (char *)token, sizeof(_name));
    _name[sizeof(_name) - 1] = '\0';

    free(nameCpy);
  }
}

void DataField::setLongName(const char *newLongName)
{
  strncpy((char *)_longName, (char *)newLongName, sizeof(_longName));
  _longName[sizeof(_longName) - 1] = '\0';
}

void DataField::setUnits(const char *newUnits)
{
  strncpy((char *)_units, (char *)newUnits, sizeof(_units));
  _units[sizeof(_units) - 1] = '\0';
}


void DataField::setAsciiFormat(const char *format)
{
  strncpy(_asciiFormat, (char *)format, sizeof(_asciiFormat));
  _asciiFormat[sizeof(_asciiFormat) - 1] = '\0';
}


const char *DataField::asciiFormat()
{
  return (const char *)_asciiFormat;
}

