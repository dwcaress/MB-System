/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : AngleData.h                                                   */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#ifndef _ANGLEDATA_H
#define _ANGLEDATA_H

#include "DoubleData.h"


#define AngleTypeMnem "angle"

/*
CLASS 
AngleData

DESCRIPTION
Wrapper class for angular data, which is stored internally as 'double' in
radians; hence AngleData is subclassed from DoubleData. The ascii() method 
outputs a string in degrees, and parseValue() converts from degrees
to radians.

AUTHOR
Tom O'Reilly
*/
class AngleData : public DoubleData {

public:

  AngleData(const char *name);

  virtual ~AngleData();

  ///////////////////////////////////////////////////////////////////
  // Data type mnemonic
  virtual const char *typeMnemonic();

  ///////////////////////////////////////////////////////////////////
  // Ascii representation of value
  virtual const char *ascii();

  ///////////////////////////////////////////////////////////////////
  // Set field's value, given string representation
  virtual void parseValue(const char *stringRep);

};

#endif


