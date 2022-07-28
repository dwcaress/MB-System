/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : DoubleData.h                                                  */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#ifndef _DOUBLEDATA_H
#define _DOUBLEDATA_H

#include "DataField.h"

#define DoubleTypeMnem "double"


/*
CLASS 
DoubleData

DESCRIPTION
Wrapper class for 'double' data type

AUTHOR
Tom O'Reilly
*/
class DoubleData : public DataField {

public:

  DoubleData(const char *name);
  DoubleData(const char *name, const char *longname, const char *units);

  virtual ~DoubleData();

  ///////////////////////////////////////////////////////////////////
  // Data type mnemonic
  virtual const char *typeMnemonic();

  ///////////////////////////////////////////////////////////////////
  // Set data value
  virtual void setValue(double value);

  ///////////////////////////////////////////////////////////////////
  // Data value
  double value();

  ///////////////////////////////////////////////////////////////////
  // Ascii representation of value
  virtual const char *ascii();

  ///////////////////////////////////////////////////////////////////
  // Set value of ExternalData object
  virtual void write(ExternalData *externalData);

  ///////////////////////////////////////////////////////////////////
  // Get value from ExternalData object
  virtual void read(ExternalData *externalData);

  ///////////////////////////////////////////////////////////////////
  // Set field's value, given string representation
  virtual void parseValue(const char *stringRep);

protected:

  double _value;
  char _asciiBuffer[32];
};


#endif
