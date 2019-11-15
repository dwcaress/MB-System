/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : DataLogReader.h                                               */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#ifndef _DATALOGREADER_H
#define _DATALOGREADER_H

#include "DataLog.h"

/*
CLASS 
DataLogReader

DESCRIPTION
Reads data log created by DataLogWriter subclass.

AUTHOR
Tom O'Reilly
*/
class DataLogReader : public DataLog {

public:
  
  ///////////////////////////////////////////////////////////////////
  // Constructor
  // [input]: fileName: name of log file
  DataLogReader(const char *fileName);

  ~DataLogReader();

  ///////////////////////////////////////////////////////////////////
  // Read data from log, setting values of contained DataField objects
  int read();

  ///////////////////////////////////////////////////////////////////
  // Print log contents to stdout
  void print();


protected:

private:

  ///////////////////////////////////////////////////////////////////
  // Read metadata from file header; construct Data fields accordingly
  void readHeader();

};

#endif
