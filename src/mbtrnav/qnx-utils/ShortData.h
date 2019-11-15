/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : ShortData.h                                                   */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#ifndef _SHORTDATA_H
#define _SHORTDATA_H

#include "DataField.h"

#define ShortTypeMnem "short"

/*
CLASS 
ShortData

DESCRIPTION
Wrapper class for 'short' data type

AUTHOR
Tom O'Reilly
*/
class ShortData : public DataField {

public:

  ShortData(const char *name);

  virtual ~ShortData();

  ///////////////////////////////////////////////////////////////////
  // Data type mnemonic
  virtual const char *typeMnemonic();

  ///////////////////////////////////////////////////////////////////
  // Set data value
  void setValue(short value);

  ///////////////////////////////////////////////////////////////////
  // Data value
  short value();

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

  short _value;
  char _asciiBuffer[32];
};


#endif
