/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : ExternalData.h                                                */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#ifndef _EXTERNALDATA_H
#define _EXTERNALDATA_H

class StringData;
class CharData;
class ShortData;
class IntegerData;
class FloatData;
class DoubleData;

/*
CLASS 
ExternalData

DESCRIPTION
Converts between internal primitive data values and some external
"stream" representation. The external representation is a stream in
that set() and get() sequentially set or get values. 
This is an "abstract interface" class (all methods are pure virtual).

Note that the set() and get() methods deal with DataField subclasses,
rather than primitive data types. This is because some ExternalData
objects may require the "formatting" information that comes along with
the DataField.

AUTHOR
Tom O'Reilly
*/

class ExternalData {

public:
  

  ///////////////////////////////////////////////////////////////////
  // Set char value to external representation
  // [input] charData: Value to be converted
  virtual void set(CharData *charData) = 0;

  ///////////////////////////////////////////////////////////////////
  // Set short value to external representation
  // [input] shortData: Value to be converted
  virtual void set(ShortData *shortData) = 0;

  ///////////////////////////////////////////////////////////////////
  // Set int value to external representation
  // [input] integerData: Value to be converted
  virtual void set(IntegerData *integerData) = 0;

  ///////////////////////////////////////////////////////////////////
  // Set float value to external representation
  // [input] floatData: Value to be converted
  virtual void set(FloatData *floatData) = 0;

  ///////////////////////////////////////////////////////////////////
  // Set double value to external representation
  // [input] doubleData: Value to be converted
  virtual void set(DoubleData *doubleData) = 0;

  ///////////////////////////////////////////////////////////////////
  // Set string to external representation
  // [input] stringData: Value to be converted
  virtual void set(StringData *stringData) = 0;

  ///////////////////////////////////////////////////////////////////
  // Get char value from external representation
  // [output] charData: contains internal representation of data
  virtual void get(CharData *charData) = 0;

  ///////////////////////////////////////////////////////////////////
  // Get short value from external representation
  // [output] shortData: contains internal representation of data
  virtual void get(ShortData *shortData) = 0;

  ///////////////////////////////////////////////////////////////////
  // Get int value from external representation
  // [output] integerData: contains internal representation of data
  virtual void get(IntegerData *integerData) = 0;

  ///////////////////////////////////////////////////////////////////
  // Get float value from external representation
  // [output] floatData: contains internal representation of data
  virtual void get(FloatData *floatData) = 0;

  ///////////////////////////////////////////////////////////////////
  // Get double value from external representation
  // [output] doubleData: contains internal representation of data
  virtual void get(DoubleData *doubleData) = 0;

  ///////////////////////////////////////////////////////////////////
  // Get string from external representation
  // [output] stringData: contains internal representation of data
  virtual void get(StringData *stringData) = 0;

};


#endif
