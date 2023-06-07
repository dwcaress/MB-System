/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : CharData.h                                                    */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#ifndef _CHARDATA_H
#define _CHARDATA_H

#include "DataField.h"

#define CharTypeMnem "char"
#define ASCII_BUFFER_BYTES 32

/*
CLASS 
CharData

DESCRIPTION
Wrapper class for 'char' data type

AUTHOR
Tom O'Reilly
*/
class CharData : public DataField {

public:

  CharData(const char *name);

  virtual ~CharData();

  ///////////////////////////////////////////////////////////////////
  // Data type mnemonic
  virtual const char *typeMnemonic();

  ///////////////////////////////////////////////////////////////////
  // Set data value
  void setValue(char value);

  ///////////////////////////////////////////////////////////////////
  // Data value
  char value();

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

  char _value;
  char _asciiBuffer[ASCII_BUFFER_BYTES];
};


#endif
