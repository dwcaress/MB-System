/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : DataLogWriter.h                                               */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#ifndef _DATALOGWRITER_H
#define _DATALOGWRITER_H

#include "DataLog.h"
#include "TimeTag.h"
#include "TimeP.h"
#include "TimeIF.h"

#define TRNLogDirName    "TRN_LOGFILES"
#define LatestLogDirName "latestTRN"
#define AutoTimeStamp True
#define NoAutoTimeStamp False

/*
CLASS 
DataLogWriter

DESCRIPTION
Creates and writes data to a DataLog object.

Subclasses of DataLogWriter should do the following:

1. Create and add DataField objects using the addField() method in
   the subclass constructor. Note that DataLogWriter adds a 
   TimeTag field automatically, so your subclass needn't.

2. Override the setFields() method; setFields() should set the value
   of the contained DataField objects. setFields() is called by 
   DataLogWriter::write().

As an example of a DataLogWriter subclass, see NavigationLog; also see
Navigation, which uses NavigationLog.

AUTHOR
Tom O'Reilly
*/
class DataLogWriter : public DataLog {

public:

  DataLogWriter(const char *name, DataLog::FileFormat fileFormat,
		Boolean autoTimestamp);

  virtual ~DataLogWriter();

  ///////////////////////////////////////////////////////////////////
  // Add a field
  void addField(DataField *field);

  ///////////////////////////////////////////////////////////////////
  // Write data from contained DataField objects to log 
  int write();

  void flush() { fflush(fileStream()); }

  ///////////////////////////////////////////////////////////////////
  // Pointer to TimeTag field
  TimeTag *timeStamp();

  ///////////////////////////////////////////////////////////////////
  // Return time in TimeIF format
  TimeIF::TimeSpec *getTimeSpec();

protected:

  ///////////////////////////////////////////////////////////////////
  // Set values of DataField objects. This virtual method is called
  // by DataLogWriter::write()
  //virtual void setFields() = 0;
  virtual void setFields() {};

  int checkLog();
  void updateAutoTimestamp();

private:

  ///////////////////////////////////////////////////////////////////
  // Write metadata to file header. Called after all fields have been added.
  void writeHeader();
  TimeTag _timeStamp;
  Boolean _autoTimestamp;
  struct  timespec _timeSpec;
  TimeIF::TimeSpec _timeIFSpec;

  Boolean _logOK;
};

#endif
