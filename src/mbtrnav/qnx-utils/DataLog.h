/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : DataLog.h                                                     */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#ifndef _DATALOG_H
#define _DATALOG_H

#include <string>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "DynamicArray.h"
#include "FileData.h"
#include "DataField.h"

#define NoName "logfile"     // Used as the file name when none is supplied
#define CommentChar "#"

#define FormatMnem "format"
#define BinaryFormatMnem "binary"
#define AsciiFormatMnem "ascii"
#define BeginDataMnem "begin"

#define TimeTagFieldName "time"
class TimeTag;

/*
CLASS 
DataLog

DESCRIPTION
Non-volatile repository for "tabular" data. Columns are specified by the 
contained DataField objects; data is accessed through the contained
FileData object.

AUTHOR
Tom O'Reilly
*/
class DataLog {

public:

  typedef struct timespec TimeSpec;


  enum Access {
    Read, Write
  };

  enum FileFormat {
    UnknownFormat, BinaryFormat, AsciiFormat
  };

  // Function attempts to make a unique name for a directory to be created
  // inside the directory specified in homeDir. The new name uses the format:
  //     YYYY.JJJ.NNN  (e.g. 2021.001.000 <- first name for Jan 1, 2021)
  // where YYYY is the 4-digit year, JJJ is the Julian day of the year plus one,
  // and NNN is an integer appended to ensure the name is unique.
  // The directory is NOT created by this function.
  //
  // The new directory name is returned in dirname.
  // Returns true if the directory can be created, otherwise false.
  static bool newJulianDayLogDirName(std::string& dirname,
                                     const std::string& homeDir);


  // Function attempts to create a new directory for log files inside the
  // existing directory specified in homeDir. The new directory name is created
  // using newJulianDayLogDirName() function above. If homeDir does not exist,
  // the current directory will be used (i.e., ".").
  // Assuming the new directory is created in homeDir, this function will also
  // create a "latest" symbolic link to the new directory if one is provided
  // in the latest argument. To avoid creating the link, use and empty string.
  // The new directory name is returned in dirname.
  // Returns true if successful, otherwise false.
  static bool createJulianDayLogDir(std::string& dirname,
                                    const std::string& homeDir,
                                    const std::string& latest);

  ///////////////////////////////////////////////////////////////////
  // Constructor
  // Use this constructor when file format is known.
  // [input] name: Name of log object (NOT the filename!)
  // [input] access: DataLog::Read, DataLog::Write, or DataLog::ReadWrite
  DataLog(const char *name, Access access, FileFormat fileFormat);

  virtual ~DataLog();

  ///////////////////////////////////////////////////////////////////
  // Name of log object (NOT file name)
  const char *name() {
    return _name;
  }

  ///////////////////////////////////////////////////////////////////
  // Set name of log object
  void setName(const char *name);

  ///////////////////////////////////////////////////////////////////
  // Set "mnemonic" of log object
  void setMnemonic(const char *name);

  ///////////////////////////////////////////////////////////////////
  // "mnemonic" of log object
  const char *mnemonic() {
    return _mnemonic;
  }

  ///////////////////////////////////////////////////////////////////
  // Name of log file
  const char *fileName() {
    return _fileName;
  }

  ///////////////////////////////////////////////////////////////////
  // Access mode
  Access access() {
    return _access;
  }

  ///////////////////////////////////////////////////////////////////
  // Data fields 
  DynamicArray<DataField *> fields;


  ///////////////////////////////////////////////////////////////////
  // Pointer to TimeTag field (null if not found)
  TimeTag *timeTag();

protected:

  ///////////////////////////////////////////////////////////////////
  // Number of fields per record
  unsigned nFields() {
    return fields.size();
  }

  ///////////////////////////////////////////////////////////////////
  // Open the log file
  void openFile();

  FILE *fileStream() {
    return _logFileStream;
  }

  Boolean _handledHeader;

  FileData *_logFile;

  FileFormat _fileFormat;

  char _fileName[256];

private:

  const char *_name;
  const char *_mnemonic;
  Access _access;
  FILE *_logFileStream;
};

#endif
