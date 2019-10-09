/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : AsciiFile.h                                                   */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#ifndef _ASCIIFILE_H
#define _ASCIIFILE_H

#include "FileData.h"

/*
CLASS 
AsciiFile

DESCRIPTION
Implementation of ExternalFileData interface. Converts between primitive
data values and ASCII data stored in a file. Values in file are
separated by whitespace (blank or tab); records are delimited by
newline.

AUTHOR
Tom O'Reilly
*/
class AsciiFile : public FileData {

public:

  AsciiFile(FILE *file);

  virtual ~AsciiFile();

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
  // End a record by appending a newline
  void endRecord();


protected:

  char *nextToken();

  char *startNextToken(char *buffer);

  // Buffer to hold lines from file
  char _buffer[512];
  char *_bufferPtr;
  char _token[512];
  char _delimiter;
};

#endif
