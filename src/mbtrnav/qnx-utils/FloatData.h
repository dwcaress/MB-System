/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : FloatData.h                                                   */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#ifndef _FLOATDATA_H
#define _FLOATDATA_H

#include "DataField.h"

#define FloatTypeMnem "float"
#define FLTDATA_ASCII_BUFFER_BYTES 32

/*
CLASS 
FloatData

DESCRIPTION
Wrapper class for 'float' data type

AUTHOR
Tom O'Reilly
*/
class FloatData : public DataField {

public:

  FloatData(const char *name);

  virtual ~FloatData();

  ///////////////////////////////////////////////////////////////////
  // Data type mnemonic
  virtual const char *typeMnemonic();

  ///////////////////////////////////////////////////////////////////
  // Set data value
  void setValue(float value);

  ///////////////////////////////////////////////////////////////////
  // Data value
  float value();

  ///////////////////////////////////////////////////////////////////
  // Ascii representation of data value
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

  float _value;
  char _asciiBuffer[FLTDATA_ASCII_BUFFER_BYTES];
};


#endif
