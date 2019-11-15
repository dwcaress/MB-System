/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : LogFile.cc                                                    */
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
#include "LogFile.h"

LogFile::LogFile(const char *fileName)
{
  _outputFile = fopen(fileName, "w");

  if (!_outputFile) {
    fprintf(stderr, "Simulator::Log::Log() - couldn't open log file %s\n",
	    fileName);
    exit(1);
  }
}


LogFile::~LogFile()
{
  if (_outputFile) {
    fprintf(_outputFile, "\n];\n");
    fclose(_outputFile);
  }
}



FILE *LogFile::file()
{
  return _outputFile;
}


