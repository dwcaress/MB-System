/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : Exception.h                                                   */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include <string.h>

/*
CLASS 
Exception

DESCRIPTION
Base class for thrown exceptions

AUTHOR
Tom O'Reilly
*/
class Exception {

public:

  ///////////////////////////////////////////////////////////////////
  // Constructor
  // [input] message: Descriptive message
  Exception(const char *message) {
    strncpy(msg, message, sizeof(msg));
  }

  ///////////////////////////////////////////////////////////////////
  // Descriptive text message
  char msg[100];
};

#endif
