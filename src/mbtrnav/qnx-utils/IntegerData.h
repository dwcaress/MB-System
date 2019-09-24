/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : IntegerData.h                                                 */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#ifndef _INTEGERDATA_H
#define _INTEGERDATA_H

#include "DataField.h"

#define IntegerTypeMnem "integer"

/*
CLASS 
IntegerData

DESCRIPTION
Wrapper class for 'int' data type

AUTHOR
Tom O'Reilly
*/
class IntegerData : public DataField {

public:

  IntegerData(const char *name);

  virtual ~IntegerData();

  ///////////////////////////////////////////////////////////////////
  // Data type mnemonic
  virtual const char *typeMnemonic();

  ///////////////////////////////////////////////////////////////////
  // Set data value
  void setValue(int value);

  ///////////////////////////////////////////////////////////////////
  // Data value
  int value();

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
  // Set data value, given string representation
  virtual void parseValue(const char *stringRep);


protected:

  int _value;
  char _asciiBuffer[32];
};


#endif
