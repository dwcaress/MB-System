/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : StringConverter.h                                             */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#ifndef _STRINGCONVERTER_H
#define _STRINGCONVERTER_H

#include <string.h>
#include "ourTypes.h"
#include "Exception.h"

#define TrueMnem "true"
#define FalseMnem "false"



/*
CLASS 
StringConverter

DESCRIPTION
Utility functions to convert character "value strings" to various
data type values.

AUTHOR
Tom O'Reilly
*/

class StringConverter {

public:

  class Error : public Exception {

  public:

    Error(char *errorMsg) : Exception(errorMsg) {
    }
  };

  ///////////////////////////////////////////////////////////////////
  // True if string represents an integer value, else False
  static Boolean isInteger(const char *string);

  ///////////////////////////////////////////////////////////////////
  // True if string represents a float value, else False
  static Boolean isFloat(const char *string);

  ///////////////////////////////////////////////////////////////////
  // True if string represents a Boolean value, else False
  static Boolean isBoolean(const char *string);
  
  ///////////////////////////////////////////////////////////////////
  // Return integer value represented by string
  static int stringToInteger(const char *string);
//    throw (Error);

  ///////////////////////////////////////////////////////////////////
  // Return float value represented by string
  static double stringToFloat(const char *string);
//    throw (Error);

  ///////////////////////////////////////////////////////////////////
  // Return Boolean value represented by string
  static Boolean stringToBoolean(const char *string);
//    throw (Error);


private:

  enum ParseState
  {
    Starting, Instance, Done
  };

  static char _errorBuf[512];
};


#endif
