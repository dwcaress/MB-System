/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : DataFieldFactory.cc                                           */
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
#include "DataFieldFactory.h"
#include "CharData.h"
#include "ShortData.h"
#include "IntegerData.h"
#include "FloatData.h"
#include "ourTypes.h"
#include "DoubleData.h"
#include "StringData.h"
#include "AngleData.h"
#include "TimeTag.h"

DataFieldFactory *DataFieldFactory::_instanceOf = 0;


DataFieldFactory *DataFieldFactory::instanceOf()
{
  if (!_instanceOf)
    // Doesn't exist yet
    _instanceOf = new DataFieldFactory();

  return _instanceOf;
}


DataFieldFactory::DataFieldFactory()
{
}


DataFieldFactory::~DataFieldFactory()
{
}


DataField *DataFieldFactory::create(const char *typeMnem, const char *name)
{
  DataField *dataField;

  if (!strcmp(typeMnem, CharTypeMnem))
    dataField = new CharData(name);

  else if (!strcmp(typeMnem, ShortTypeMnem))
    dataField = new ShortData(name);

  else if (!strcmp(typeMnem, IntegerTypeMnem))
    dataField = new IntegerData(name);

  else if (!strcmp(typeMnem, FloatTypeMnem))
    dataField = new FloatData(name);

  else if (!strcmp(typeMnem, DoubleTypeMnem))
    dataField = new DoubleData(name);

  else if (!strcmp(typeMnem, StringTypeMnem))
    dataField = new StringData(name);

  else if (!strcmp(typeMnem, AngleTypeMnem))
    dataField = new AngleData(name);

  else if (!strcmp(typeMnem, TimeTagTypeMnem))
    dataField = new TimeTag(name);

  else
    // Unknown type mnemonic
    dataField = 0;

  return dataField;
}







