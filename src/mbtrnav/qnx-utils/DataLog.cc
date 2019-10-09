/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : DataLog.cc                                                    */
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
#include "DataLog.h"
#include "ourTypes.h"
#include "Exception.h"
#include "BinaryFile.h"

DataLog::DataLog(const char *name, DataLog::Access access, 
		 DataLog::FileFormat fileFormat)
{
  if (name == NULL || strlen(name) < 1)
  {
    printf("DataLog::DataLog() - No log name supplied. Using \"logfile\"\n");
    _name = strdup(NoName);
  }
  else
  {
    _name = strdup(name);
  }

  _mnemonic = strdup(_name);

  _access = access;
  _fileFormat = fileFormat;
  _handledHeader = False;
  _logFile = 0;
  strcpy(_fileName, "");

  /* *************
  char *auvLogDir = getenv(AuvLogDirName);

  if (auvLogDir == 0) {

    printf("DataLog::DataLog() - environment variable %s not set\n",
		  AuvLogDirName);
    exit(1);
  }

  sprintf(_fileName, "%s/%s.log", auvLogDir, _name);

  openFile();
  ************* */

}


DataLog::~DataLog()
{
  //  delete _logFile;

  if (_logFileStream)
    fclose(_logFileStream);

  free((void *)_name);
}


void DataLog::openFile()
{
  char accessString[10];

  switch (access()) {

  case Read:
    strcpy(accessString, "r");
    break;

  case Write:
    strcpy(accessString, "w");
    break;
  }

  FILE *logfile;
  char origName[100];
  sprintf(origName,"%s", "");
  strcpy(origName, _fileName);

  // only do this check if access mode is write
  // otherwise we "break" logToMatlab. 
  if (access() == Write) { 
    int numtries = 1;
    char trybuf[10];
 
    while ((logfile = fopen(_fileName, "r")) != NULL) {
      printf("logfile already exists!\n");
      printf("I will try to append a %d to your file\n", numtries);
      sprintf(_fileName, "%s.%d",origName,numtries);
      fclose(logfile);
      numtries++;
      // exit(1);
    }
  }

  if ((_logFileStream = fopen(_fileName, accessString)) == 0) {

    printf("DataLog::DataLog() - couldn't open log file %s\n",
		  _fileName);
    exit(1);
  }
}



void DataLog::setName(const char *name)
{
  free((void *)_name);
  _name = strdup(name);
}


void DataLog::setMnemonic(const char *mnemonic)
{
  free((void *)_mnemonic);
  _mnemonic = strdup(mnemonic);
}



TimeTag *DataLog::timeTag()
{
  DataField *field;

  for (int i = 0; i < fields.size(); i++) {
    fields.get(i, &field);
    if (!strcmp(field->name(), TimeTagFieldName))
      return (TimeTag *)field;
  }

  // Field not found!
  return 0;
}
