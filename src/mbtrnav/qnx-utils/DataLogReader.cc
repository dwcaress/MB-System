/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : DataLogReader.cc                                              */
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
#include "DataLogReader.h"
#include "Exception.h"
#include "ourTypes.h"
#include "DataFieldFactory.h"
#include "AsciiFile.h"
#include "BinaryFile.h"

#define DEL_BY_WHTSPC ((char *)" ")   // tokenize by white space
#define DEL_BY_COMMA  ((char *)",")   // tokenize by comma, allowing white space in field

DataLogReader::DataLogReader(const char *fileName)
  : DataLog("datalog", DataLog::Read, DataLog::UnknownFormat)
{
  strcpy(_fileName, fileName);
  openFile();
  readHeader();
}


DataLogReader::~DataLogReader()
{
}


void DataLogReader::readHeader()
{
  char buffer[256];
  char buffer2[256];
  char errorBuf[MAX_EXC_STRING_LEN];
  char *mnem, *type, *format, *lname, *units;
  char *token;
  int nTokens;
  DataField *field;
  Boolean parseError = False;

  enum ParseState {
    ReadFormat, ReadDataFields
  };

  ParseState state = ReadFormat;

  parseError = False;

  while (!_handledHeader && !parseError) {
    mnem = type = format = lname = units = NULL;

    // Read a line
    fgets(buffer, sizeof(buffer), fileStream());


    if (buffer[strlen(buffer)-1] == '\n') 
      // Strip off trailing newlien
      buffer[strlen(buffer)-1] = '\0';

    strcpy(buffer2, buffer);

    token = strtok(buffer, DEL_BY_WHTSPC);

    if (token == 0) 
      // Blank line; skip it
      continue;

    if (strcmp(token, CommentChar)) {
      // Each header line should have leading comment character
      parseError = True;
      continue;
    }

    switch (state) {
      
    case ReadFormat:

      // Read file format mnemonic
      token = strtok(0, DEL_BY_WHTSPC);
      if (token == 0) {
	// No token? Parse error
	parseError = True;
	continue;
      }

      if (!strcmp(token, AsciiFormatMnem)) {
	_fileFormat = AsciiFormat;
	_logFile = new AsciiFile(fileStream());
      }
      else if (!strcmp(token, BinaryFormatMnem)) {
	_fileFormat = BinaryFormat;
	_logFile = new BinaryFile(fileStream());
      }
      else {
	_fileFormat = UnknownFormat;
	snprintf(errorBuf, sizeof(errorBuf), 
		"DataLogReader::readHeader() - Unknown file format \"%s\"",
		token);

	throw Exception(errorBuf);
      }

      // Read object name
      token = strtok(0, DEL_BY_WHTSPC);
      if (token == 0) {
	// No token? Parse error
	parseError = True;
	continue;
      }
      setName(token);
      setMnemonic(token);

      // Start reading DataFields
      state = ReadDataFields;
      break;

    case ReadDataFields:

      // Rewrote this buggy section.
      // Somehow, the input line was getting
      // munged during tokenization. The result
      // was that only the type and the name were
      // parsed from each line. The other fields
      // retained default values (e.g., format = %8.8e)
      // Now, extract all the tokens in one step,
      // then create the field with the tokens.
      // 
      const char Type=1, Mnem=2, Format=3, LName=4, Units=5, Done=6;
      char *type, *mnem, *format, *lname, *units;
      char *tokenDelimiter = DEL_BY_WHTSPC;
      char *line = buffer2;
      type = mnem = format = lname = units = NULL;
      for (nTokens = 0; !_handledHeader &&
                        (token = strtok(line, tokenDelimiter)); nTokens++)
      {
        line = NULL;

        // In general, switch statements are more difficult to
        // read and can be buggy with the break statements and whatnot.
        // The preferred idiom is
        //   if (case a)
        //     a stuff
        //   else if (case b)
        //     b stuff
        //     :
        //   else
        //     default stuff
        //     
        if (nTokens == 0)
        {
          // This should be "#"
        }
        else if (nTokens == Type)
        {
          // Datatype field
          type = token;
          // Done with header section
          if (!strcmp(token, BeginDataMnem)) _handledHeader = True;
        }
        else if (nTokens == Mnem)
        {
          // DataField name
          mnem = token;
        }
        else if (nTokens == Format)
        {
          // Preferred printing format
          // If token begins with a comma, then there is no ascii format field
          if (token[0] != ',') format = token;

      	  // Change the token delimiter to a comma in order
          // to allow spaces in long names and units.
          //
          tokenDelimiter = DEL_BY_COMMA;
        }
        else if (nTokens == LName)
        {
          // Optional long descriptive name
          lname = token;
          //field->setLongName(token);
        }
        else if (nTokens == Units)
        {
          // Optional units
          units = token;
          //field->setUnits(token);
        }
        else
        { 
          parseError = True;
        }
      }

      if (_handledHeader)
        break;   // done with header
      else if (nTokens < Mnem || nTokens > Done)
      {
        parseError = True;
      }
      else
      {
        field = DataFieldFactory::instanceOf()->create(type, mnem);
        if (!field)
        {
          snprintf(errorBuf, sizeof(errorBuf), "DataLogReader::readHeader() - "
                            "unknown DataField: %s %s %s ,%s ,%s\n",
                            type, mnem, format, lname, units);
          throw Exception(errorBuf);
        }
        else
          fields.add(&field);

        if (format) field->setAsciiFormat(format);
        if (lname) field->setLongName(lname);
        if (units) field->setUnits(units);
      }
    }
  }

  if (parseError) {
    const char *ns = "(NULL)";
    snprintf(errorBuf,  sizeof(errorBuf), 
            "DataLogReader::readHeader() parse error %s %s %s %s %s",
            (type   != NULL ? type   : ns), (mnem  !=NULL ? mnem  : ns),
            (format != NULL ? format : ns), (lname !=NULL ? lname : ns),
            (units  != NULL ? units  : ns));

    throw Exception(errorBuf);
  }

  _handledHeader = True;
}



int DataLogReader::read()
{
  char errorBuf[MAX_EXC_STRING_LEN];
  DataField *field = NULL;

  for (unsigned int i = 0; i < nFields(); i++) {

    fields.get(i, &field);

    if (field != NULL) field->read(_logFile);
    else
    {
      snprintf(errorBuf,  sizeof(errorBuf), 
               "DataLogReader::read() expected field %d where none was found",
               i);
      throw Exception(errorBuf);
    }
  }

  return 0;
}



void DataLogReader::print()
{
  DataField *field;
  int i;

  printf("# ");
  for (i = 0; i < fields.size(); i++) {
    fields.get(i, &field);
    printf("%s ", field->name());
  }
  printf("\n");

  try {
    while (True) {

      // Read a record
      read();

      for (i = 0; i < fields.size(); i++) {
	fields.get(i, &field);

	fprintf(stdout, "%s   ", field->ascii());
      }

      fprintf(stdout, "\n");
    }
  }
  catch (Exception e) {
    fprintf(stderr, "DataLogReader::read() - %s\n", e.msg);
    return;
  }
}




