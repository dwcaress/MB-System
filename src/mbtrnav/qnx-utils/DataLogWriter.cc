/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : DataLogWriter.cc                                              */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>

#if !defined(__APPLE__)
#include <malloc.h>
#endif

#include <string.h>
#include <time.h>
#include "DataLogWriter.h"
#include "TimeP.h"
#include "ourTypes.h"
#include "Exception.h"
#include "BinaryFile.h"
#include "AsciiFile.h"

#define DLDEBUG 0

DataLogWriter::DataLogWriter(const char *objectName,
			     DataLog::FileFormat fileFormat,
			     Boolean autoTimestamp)
  : DataLog(objectName, DataLog::Write, fileFormat),
    _timeStamp(TimeTagFieldName), _logOK(False)
{
  _autoTimestamp = autoTimestamp;

  //////////////////////////////////////////////////////////////////
  // Build file name and open the file
  const char *trnLogDir = getenv(TRNLogDirName);
  if (trnLogDir == 0) {
    // No ENV for logs. Use current directory
    //
    //printf("\n\n\tDataLog::DataLog() - environment variable %s not set\n",
    //  TRNLogDirName);
    //printf("\tDataLog::DataLog() - Log directory is in local directory!\n\n");
    trnLogDir = ".";
  }

  snprintf(_fileName, DLOG_FILENAME_BYTES, "%s/%s/%s.log", trnLogDir, LatestLogDirName, name());

  openFile();

  switch (fileFormat) {

  case AsciiFormat:
    _logFile = new AsciiFile(fileStream());
    _logOK = True;
    break;

  case BinaryFormat:
    _logFile = new BinaryFile(fileStream());
    _logOK = True;
    break;

  default:
    _logFile = NULL;
    _logOK = False;
    if (DLDEBUG) printf("DataLogWriter::DataLogWriter() - unknown file format\n");
  }

  addField(&_timeStamp);


}


DataLogWriter::~DataLogWriter()
{
  if (_logFile) delete _logFile;
}


void DataLogWriter::addField(DataField *field)
{
  if (!_logOK)
    printf("DataLogWriter::addField() - %s failed to create log filestream %s\n",
      name(), _fileName);

  // Check for spaces in name
  if (strchr(field->name(), ' ') || strchr(field->name(), '\t')) {
    // Spaces not allowed in field name
    char errorBuf[MAX_EXC_STRING_LEN];
    snprintf(errorBuf, sizeof(errorBuf),
	    "Illegal field name: \"%s\"; whitespace not allowed in name",
	    field->name());

    throw Exception(errorBuf);
  }

  fields.add(&field);
}


void DataLogWriter::writeHeader()
{
  if (_logFile == NULL || _logOK == False) {
    if (DLDEBUG) printf("DataLogWriter::writeHeader() - no log file!\n");
    return;
  }

  if (DLDEBUG) printf("DataLogWriter::writeHeader()\n");

  switch (_fileFormat) {

  case BinaryFormat:
    fprintf(fileStream(), "%s %s %s\n",
	    CommentChar, BinaryFormatMnem, mnemonic());
    break;

  case AsciiFormat:
    fprintf(fileStream(), "%s %s %s\n", CommentChar, AsciiFormatMnem, name());
    break;

  default:
    if (DLDEBUG) printf("DataLogWriter::writeHeader() - unknown format!\n");
    exit(1);
  }

  DataField *field = NULL;

  for (unsigned int i = 0; i < nFields(); i++) {

    fields.get(i, &field);

    // Print field type, name, and preferred ascii print format
    // May03 - also print the descriptive name and the units for each field
    //       - delimit long name and units fields by commas rather than space
    //
    fprintf(fileStream(), "%s %s %s %s ,%s ,%s \n",
	    CommentChar, field->typeMnemonic(), field->name(),
	    field->asciiFormat(),
	    field->longName(), field->units());
  }

  fprintf(fileStream(), "%s %s\n", CommentChar, BeginDataMnem);

  _handledHeader = True;

  if (DLDEBUG) printf("DataLogWriter::writeHeader() - done\n");
  fflush(fileStream());
}

int DataLogWriter::checkLog()
{
    if (_logFile == NULL || _logOK == False) {
        return -1;
    }
    return 0;
}

void DataLogWriter::updateAutoTimestamp()
{
    if (_autoTimestamp) {
        //struct timespec timeSpec;
        //clock_gettime(CLOCK_REALTIME, &_timeSpec);
        TimeP::gettime(&_timeSpec);
        _timeStamp.setValue((double )(_timeSpec.tv_sec + _timeSpec.tv_nsec/1.e9));
    }
}

int DataLogWriter::write()
{
    if (_logFile == NULL || _logOK == False) {
        if (DLDEBUG) printf("DataLogWriter::write() - no log file!\n");
        return -1;
    }

    if (!_handledHeader) {
        // Need to write header. Note that write() is not called until
        // after all Data fields have been added.
        writeHeader();
    }


    if (_autoTimestamp) {
        //struct timespec timeSpec;
        //clock_gettime(CLOCK_REALTIME, &_timeSpec);
        TimeP::gettime(&_timeSpec);
        _timeStamp.setValue((double )(_timeSpec.tv_sec + _timeSpec.tv_nsec/1.e9));
    }

  if (DLDEBUG) printf("DataLogWriter::write() - call setFields()\n");

  // Call subclass-defined method to set field values
  setFields();

  DataField *field = NULL;

  // Write each field value to logfile
  for (unsigned int i = 0; i < nFields(); i++) {

    fields.get(i, &field);

    if (DLDEBUG && field!=NULL) printf("DataLogWriter::write() - field %s\n", field->name());
    if (field != NULL) field->write(_logFile);
    else
    {
      char errorBuf[MAX_EXC_STRING_LEN];

      snprintf(errorBuf, sizeof(errorBuf),
               "DataLogWriter::write() expected field %d where none was found",
               i);
      throw Exception(errorBuf);
    }
  }

  if (DLDEBUG) printf("DataLogWriter::write() - done with each field\n");

  // Terminate this record
  _logFile->endRecord();

  return 0;
}


TimeTag *DataLogWriter::timeStamp()
{
  return &_timeStamp;
}

TimeIF::TimeSpec *DataLogWriter::getTimeSpec()
{
   _timeIFSpec.seconds     = _timeSpec.tv_sec;
   _timeIFSpec.nanoSeconds = _timeSpec.tv_nsec;
   return &_timeIFSpec;
}
