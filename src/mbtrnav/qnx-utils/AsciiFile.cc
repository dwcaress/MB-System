/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : AsciiFile.cc                                                  */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "AsciiFile.h"
#include "Exception.h"
#include "ourTypes.h"
#include "CharData.h"
#include "ShortData.h"
#include "IntegerData.h"
#include "FloatData.h"
#include "DoubleData.h"
#include "StringData.h"

AsciiFile::AsciiFile(FILE *file)
  : FileData(file)
{
  // This should be variable from user
  _delimiter = ' ';

  // This will cause nextToken() to get a line from the file
  memset((void *)_buffer, 0, sizeof(_buffer));
  _bufferPtr = _buffer;
}


AsciiFile::~AsciiFile()
{
}


void AsciiFile::set(CharData *charData)
{
  char errorBuf[MAX_EXC_STRING_LEN];

  if (fprintf(_file, "%s%c", charData->ascii(), _delimiter) < 0) {
    snprintf(errorBuf, MAX_EXC_STRING_LEN, "AsciiFile::set(CharData) - %s", strerror(errno));
    throw Exception(errorBuf);
  }
}


void AsciiFile::set(ShortData *shortData)
{
  char errorBuf[MAX_EXC_STRING_LEN];

  if (fprintf(_file, "%s%c", shortData->ascii(), _delimiter) < 0) {
    snprintf(errorBuf, MAX_EXC_STRING_LEN, "AsciiFile::set(ShortData) - %s", strerror(errno));
    throw Exception(errorBuf);
  }
}


void AsciiFile::set(IntegerData *integerData)
{
  char errorBuf[MAX_EXC_STRING_LEN];

  if (fprintf(_file, "%s%c", integerData->ascii(), _delimiter) < 0) {
    snprintf(errorBuf, MAX_EXC_STRING_LEN, "AsciiFile::set(IntegerData) - %s", strerror(errno));
    throw Exception(errorBuf);
  }
}


void AsciiFile::set(FloatData *floatData)
{
  char errorBuf[MAX_EXC_STRING_LEN];

  if (fprintf(_file, "%s%c", floatData->ascii(), _delimiter) < 0) {
    snprintf(errorBuf, MAX_EXC_STRING_LEN, "AsciiFile::set(FloatData) - %s", strerror(errno));
    throw Exception(errorBuf);
  }
}


void AsciiFile::set(DoubleData *doubleData)
{
  char errorBuf[MAX_EXC_STRING_LEN];

  if (fprintf(_file, "%s%c", doubleData->ascii(), _delimiter) < 0) {
    snprintf(errorBuf, MAX_EXC_STRING_LEN, "AsciiFile::set(double) - %s", strerror(errno));
    throw Exception(errorBuf);
  }
}


void AsciiFile::set(StringData *stringData)
{
  char errorBuf[MAX_EXC_STRING_LEN];

  // Make sure string does not contain delimiter character
  if (strchr(stringData->ascii(), _delimiter) != 0) {

    snprintf(errorBuf, MAX_EXC_STRING_LEN,
	    "AsciiFile::set(string) - string \"%s\" contains delimiter",
	    stringData->ascii());

    throw Exception(errorBuf);
  }

  if (fprintf(_file, "%s%c", stringData->ascii(), _delimiter) < 0) {
    snprintf(errorBuf, MAX_EXC_STRING_LEN, "AsciiFile::set(string) - %s", strerror(errno));
    throw Exception(errorBuf);
  }
}


void AsciiFile::get(CharData *charData)
{
  charData->parseValue(nextToken());
}


void AsciiFile::get(ShortData *shortData)
{
  shortData->parseValue(nextToken());
}


void AsciiFile::get(IntegerData *integerData)
{
  integerData->parseValue(nextToken());
}


void AsciiFile::get(FloatData *floatData)
{
  floatData->parseValue(nextToken());
}


void AsciiFile::get(DoubleData *doubleData)
{
  doubleData->parseValue(nextToken());
}


void AsciiFile::get(StringData *stringData)
{
  stringData->parseValue(nextToken());
}


void AsciiFile::endRecord()
{
  char errorBuf[MAX_EXC_STRING_LEN];

  if (fprintf(_file, "\n") < 0) {
    snprintf(errorBuf, MAX_EXC_STRING_LEN, "AsciiFile::endRecord() - %s", strerror(errno));
    throw Exception(errorBuf);
  }
}


char *AsciiFile::nextToken()
{
  char *endOfToken;

  while ((_bufferPtr = startNextToken(_bufferPtr)) == 0) {

    // No token on line; get next line from file
    if (fgets(_buffer, sizeof(_buffer), _file) == 0)
      // End of file
      throw Exception("eof");

    // Point to start of line
    _bufferPtr = _buffer;
  }

  // Find end of token
  for (endOfToken = _bufferPtr + 1; 
       *endOfToken != '\0' && *endOfToken != _delimiter;
       endOfToken++) {
    ;
  }

  strncpy(_token, _bufferPtr, endOfToken - _bufferPtr);
  _token[endOfToken - _bufferPtr] = '\0';

  _bufferPtr = endOfToken;

  return _token;
}


char *AsciiFile::startNextToken(char *buffer) 
{
  char *ptr;
  
  // Find first non-white character
  for (ptr = buffer; *ptr != '\0'; ptr++) {

    if (!isspace(*ptr))
      return ptr;
  }

  return 0;
}


