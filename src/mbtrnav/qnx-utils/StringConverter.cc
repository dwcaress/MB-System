/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : StringConverter.cc                                            */
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
#include <ctype.h>
#include <string.h>

#if !defined(__APPLE__)
#include <malloc.h>
#endif

#include <stdlib.h>
#include "StringConverter.h"

char StringConverter::_errorBuf[512];

Boolean StringConverter::isInteger(const char *str)
{
  unsigned int i;
  int state = Starting;
  int ndigits = 0;
  
  for (i = 0; i < strlen(str); i++)
  {
    if (!isdigit(str[i]))
    {
      switch ((int )str[i])
      {
        case '-':
        case '+':
        /* Allow leading '-' or '+' */
        if (state != Starting)
          return 0;
        break;
        
        case ' ':
        /* Allow leading or trailing blank */
        switch (state)
        {
          case Starting:
          continue;
          
          case Instance:
          state = Done;
          break;
        }
        break;
        
        default:
        /* Any other non-digit is illegal */
        return False;
      }
    }
    else
    {
      /* Got a digit */
      ndigits++;
      
      switch (state)
      {
        case Starting:
        state = Instance;
        break;
        
        case Done:
        return False;

        default:
        continue;
      }
    }
  }
  if (state != Instance && state != Done)
    return False;
  else if (ndigits <= 0)
    return 0;
  else
    return True;
}



Boolean StringConverter::isFloat(const char *str)
{
  unsigned int i;
  int n_dot = 0;
  Boolean start_num = False;
  Boolean trail_blank = False;
  Boolean start_exp = False;
  
  for (i = 0; i < strlen(str); i++)
  {
    if (trail_blank && str[i] != ' ')
      /* Already hit trailing blank(s); nothing else allowed */
      return False;
    
    if (!isdigit(str[i]))
    {
      switch ((int )str[i])
      {
        case '-':
        case '+':
        /* Allow leading '-' or '+' */
        if (start_num)
          return False;

        start_num = True;
        break;
        
        case '.':
        /* One decimal point allowed in mantissa, not allowed in exponent */
        if (++n_dot != 1 || start_exp)
          return False;

        start_num = True;
        break;

	case 'e':
	case 'E':
	/* The rest of the string must be an integer */
	start_exp = True;
	start_num = False;
	break;
	
        case ' ':
        /* Allow leading or trailing blanks */
        if (i == 0)
          continue;
        
        if (start_num)
        {
          /* Trailing (ok) or embedded (bad) blank */
          trail_blank = True;
        }
        break;
        
        default:
        return False;
      }
    }
    else
      start_num = True;
    
  }
  return True;
}


Boolean StringConverter::isBoolean(const char *str)
{
  // Need to make copy of input so we don't alter it
  // with case-folding
  char *string = strdup(str);

  // Case-fold to lower case
  for (unsigned int i = 0; i < strlen(str); i++) {
    string[i] = tolower(str[i]);
  }

  Boolean isBool = False;

  if (!strcmp(string, TrueMnem) || !strcmp(string, FalseMnem) ||
      !strcmp(string, "t") || !strcmp(string, "f") ||
      !strcmp(string, "1") || !strcmp(string, "0") ||
      !strcmp(string, "y") || !strcmp(string, "n") ||
      !strcmp(string, "yes") || !strcmp(string, "no"))
    isBool = True;

  free((void *)string);
  return isBool;
}


int StringConverter::stringToInteger(const char *string)
{
  if (!isInteger(string)) {
    printf(_errorBuf, "Not an integer: \"%s\"\n", string);
  }

  return atoi(string);
}


double StringConverter::stringToFloat(const char *string)
{
  if (!isFloat(string)) {
    printf(_errorBuf, "Not a float: \"%s\"\n", string);
  }

  return atof(string);
}


Boolean StringConverter::stringToBoolean(const char *str)
{
  if (!isBoolean(str)) {
    printf(_errorBuf, "Not a boolean: \"%s\"\n", str);
  }

  // Need to make copy of input so we don't alter it
  // with case-folding
  char *string = strdup(str);

  // Case-fold to lower case
  for (unsigned int i = 0; i < strlen(str); i++) {
    string[i] = tolower(str[i]);
  }

  Boolean value;

  if (!strcmp(string, TrueMnem) || 
      !strcmp(string, "t") || 
      !strcmp(string, "1") || 
      !strcmp(string, "y") || 
      !strcmp(string, "yes"))
    value = True;
  else
    value = False;

  free((void *)string);

  return value;
}

