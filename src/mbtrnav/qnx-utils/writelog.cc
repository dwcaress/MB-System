/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : readlog.cc                                                    */
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
#include "DataLogWriter.h"
#include "Exception.h"

int main(int argc, char **argv)
{
  if (argc != 2) {
    fprintf(stderr, "usage: %s logfile\n", argv[0]);
    return 1;
  }

  const char *filename = argv[1];

  try {
    DataLogWriter *log = new DataLogWriter(filename, DataLog::BinaryFormat, True);
    if (log)
    {
      log->write();
      delete log;
    }
  }
  catch (Exception e) {
    fprintf(stderr, "%s\n", e.msg);
  }
  catch (...) {
    fprintf(stderr, "Caught some exception\n");
  }

  return 0;

}

