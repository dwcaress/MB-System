/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : FileData.h                                                    */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#ifndef _EXTERNALFILEDATA_H
#define _EXTERNALFILEDATA_H

#include <stdio.h>
#include "ExternalData.h"

/*
CLASS 
FileData

DESCRIPTION
A kind of ExternalData interface. Converts between primitive
data values and data stored in a file.

AUTHOR
Tom O'Reilly
*/
class FileData : public ExternalData {

public:

  FileData(FILE *file);
  virtual ~FileData();

  ///////////////////////////////////////////////////////////////////
  // Append terminator to record
  virtual void endRecord() = 0;

protected:

  ///////////////////////////////////////////////////////////////////
  // FileStream pointer
  FILE *_file;
};


#endif
