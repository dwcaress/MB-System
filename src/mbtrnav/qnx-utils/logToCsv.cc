/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : logToMatlab.cc                                                */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
//
// PURPOSE: Read in binary or ascii log file created by the Altex logging
//          routines (DataLogWriter class), and produce corresponding 
//          Matlab-readable ASCII files.
//
// AUTHOR:  O'Reilly, with some features added by McEwen.
//
// DATE:    99/11/30
//
// NOTES:
//
//////////////////////////////////////////////////////////////////////////////
//
#include <stdio.h>
#include "DataLogReader.h"
#include "Exception.h"
#include "TimeTag.h"
#include "StringConverter.h"

int getOptions(int argc, char **argv, 
	       Boolean *relativeTime, 
	       int *skippedRecords);


int main(int argc, char **argv)
{
  Boolean relativeTime = False;

  // Decimation option
  int skippedRecords = 0;

  if (getOptions(argc, argv, &relativeTime, &skippedRecords) == -1)
  {
    return 1;
  }

  // Last argument is input file name
  const char *filename = argv[argc-1];

  DataLogReader *log;

  try
  {
    log = new DataLogReader(filename);
  }
  catch (Exception e)
  {
    fprintf(stderr, "%s\n\n", e.msg);
    return 1;
  }

  DataField *field;
  int i;

  //
  // Add column labels with units if available
  //
  fprintf(stdout, "%s\n", filename);
  for (i = 0; i < log->fields.size(); i++)
  {
    log->fields.get(i, &field);
    fprintf(stdout, "%s", field->name()); //, i+1);
    if (strncasecmp("UNKNOWN", field->units(), strlen("UNKNOWN")))
    {
      fprintf(stdout, " (%s)", field->units());
    }
    fprintf(stdout, ", ");
  }
  fprintf(stdout, "\n");

  // Data array name is log's name
  // printf("\n %s=[\n", log->mnemonic() );

  try
  {
    for (int nRecord = 0; ; nRecord++)
    {

      // Read a record
      log->read();

      // Decimate; skip 
      if ((nRecord % (skippedRecords + 1)) != 0) continue;

      // Print each field in record
      for (i = 0; i < log->fields.size(); i++)
      {

        log->fields.get(i, &field);

        if (field == (DataField *)log->timeTag())
        {
          //
          // Print the absolute time in the first column.  This is necessary
          // so that the various log files can be synchronized in time before
          // the back-difference is done.  See cvs version 1.1 for the 
          // back-differencing implementation.
          //
          if (!relativeTime)
          {
            fprintf(stdout, "%.4f , ", log->timeTag()->value());
          }
          else
          {
            // Relative time option in effect
            double startTime = (nRecord == 0) ? log->timeTag()->value() : 0.;

            fprintf(stdout, "%.4f , ", log->timeTag()->value() - startTime);
          }
        }
        else
        {
          fprintf(stdout, "%s , ", field->ascii());
        }
      }

      // End of record
      fprintf(stdout, "\n");
    }
  }
  catch (Exception e)
  {
    fprintf(stderr, "%s\n\n", e.msg);
    return 0;
  }

  return 0;
}



int getOptions(int argc, char **argv, Boolean *relativeTime, 
		int *skippedRecords)
{
  Boolean error = False;

  if (argc < 2)
  {
    error = True;
  }

  //
  // Scan arguments for options. Last argument is reserved for file name!
  for (int i = 1; i < argc-1; i++)
  {
    if (!strcmp(argv[i], "-rel"))
    {
      *relativeTime = True;
    }
    else if (!strcmp(argv[i], "-skip") && i < argc - 2)
    {

      if (StringConverter::isInteger(argv[++i]) && atoi(argv[i]) > 0)
      {
        *skippedRecords = atoi(argv[i]);
      }
      else
      {
        fprintf(stderr, "Invalid #skipped records: %s\n", argv[i]);
        fprintf(stderr, "Must be positive integer\n");
        error = True;
      }
    }
    else
    {
      fprintf(stderr, "Unknown or incomplete option: %s\n", argv[i]);
      error = True;
    }
  }

  if (error)
  {
    fprintf(stderr, "Usage: %s [-rel][-dec n]<file>\n", argv[0]);
    fprintf(stderr, " -rel: Print relative instead of absolute time\n");
    fprintf(stderr, " -skip n: Print record, then skip n records\n");
    return -1;
  }

  // Okay
  return 0;
}

