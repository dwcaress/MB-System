/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : BinaryFile.h                                                  */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#ifndef _BINARYFILE_H
#define _BINARYFILE_H

#include "FileData.h"
#define _WRITE_BUFFER_SIZE (1024*16)
/*
CLASS 
BinaryFile

DESCRIPTION
Implementation of FileData interface. Converts between primitive
data values and binary data stored in a file.

AUTHOR
Tom O'Reilly
*/
class BinaryFile : public FileData {

public:

  BinaryFile(FILE *file);

  virtual ~BinaryFile();

  ///////////////////////////////////////////////////////////////////
  // Set char value to external representation
  // [input] charData: Value to be converted
  virtual void set(CharData *charData);

  ///////////////////////////////////////////////////////////////////
  // Set short value to external representation
  // [input] shortData: Value to be converted
  virtual void set(ShortData *shortData);

  ///////////////////////////////////////////////////////////////////
  // Set int value to external representation
  // [input] integerData: Value to be converted
  virtual void set(IntegerData *integerData);

  ///////////////////////////////////////////////////////////////////
  // Set int value to external representation
  // [input] floatData: Value to be converted
  virtual void set(FloatData *floatData);

  ///////////////////////////////////////////////////////////////////
  // Set int value to external representation
  // [input] doubleData: Value to be converted
  virtual void set(DoubleData *doubleData);

  ///////////////////////////////////////////////////////////////////
  // Set string to external representation
  // [input] stringData: Value to be converted
  virtual void set(StringData *stringData);

  ///////////////////////////////////////////////////////////////////
  // Get char value from external representation
  // [output] charData: contains internal representation of data
  virtual void get(CharData *charData);

  ///////////////////////////////////////////////////////////////////
  // Get short value from external representation
  // [output] shortData: contains internal representation of data
  virtual void get(ShortData *shortData);

  ///////////////////////////////////////////////////////////////////
  // Get int value from external representation
  // [output] integerData: contains internal representation of data
  virtual void get(IntegerData *integerData);

  ///////////////////////////////////////////////////////////////////
  // Get int value from external representation
  // [output] floatData: contains internal representation of data
  virtual void get(FloatData *floatData);

  ///////////////////////////////////////////////////////////////////
  // Get int value from external representation
  // [output] doubleData: contains internal representation of data
  virtual void get(DoubleData *doubleData);

  ///////////////////////////////////////////////////////////////////
  // Get string from external representation
  // [output] stringData: contains internal representation of data
  virtual void get(StringData *stringData);

  ///////////////////////////////////////////////////////////////////
  // End a record
  void endRecord();

  char _buffer[_WRITE_BUFFER_SIZE];
  int _fd;
  unsigned int _bufferNBytes;
};

#endif
