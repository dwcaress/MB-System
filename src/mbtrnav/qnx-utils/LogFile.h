/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : LogFile.h                                                     */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#ifndef _LOGFILE_H
#define _LOGFILE_H
#include <stdio.h>

class LogFile {

public:

  LogFile(const char *fileName);
  ~LogFile();

  FILE *file();

private:

  FILE *_outputFile;

};

#endif

