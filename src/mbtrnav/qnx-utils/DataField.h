/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : DataField.h                                                   */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#ifndef _DATAFIELD_H
#define _DATAFIELD_H

#include <stdio.h>

class ExternalData;

/*
CLASS 
DataField

DESCRIPTION
Wrapper interface class for primitive data types. Also contains
useful conversion and utility methods.

To subclass DataField, do the following:

* Override pure virtual functions

* Add code to DataFieldFactory::create() to handle your new subclass


AUTHOR
Tom O'Reilly
*/

class DataField {

public:

  DataField(const char *name);

  DataField(const char *name, const char *longname, const char *units);

  virtual ~DataField();

  ///////////////////////////////////////////////////////////////////
  // Data type mnemonic. THIS NAME MUST BE UNIQUE TO SUBCLASS. Used
  // by DataFieldFactory.
  virtual const char *typeMnemonic() = 0;

  ///////////////////////////////////////////////////////////////////
  // Data item name
  const char *name();

  ///////////////////////////////////////////////////////////////////
  // Set name of the data item. Must be a single token with no
  // whitespace. setName("This data") results in the name being
  // set to "This". Maximum length is 64 characters.
  void setName(const char *name);

  ///////////////////////////////////////////////////////////////////
  // Data item descriptive name.
  const char *longName();

  ///////////////////////////////////////////////////////////////////
  // Set descriptive name of the data item, maximum length is 300
  // characters
  void setLongName(const char *lname);

  ///////////////////////////////////////////////////////////////////
  // Data item units
  const char *units();

  ///////////////////////////////////////////////////////////////////
  // Set units of the data item, maximum length is 64 characters
  void setUnits(const char *units);

  ///////////////////////////////////////////////////////////////////
  // Set format for printing      
  void setAsciiFormat(const char *format);

  ///////////////////////////////////////////////////////////////////
  // Format for printing      
  virtual const char *asciiFormat();

  ///////////////////////////////////////////////////////////////////
  // Set value of ExternalData object
  virtual void write(ExternalData *externalData) = 0;

  ///////////////////////////////////////////////////////////////////
  // Get value from ExternalData object
  virtual void read(ExternalData *externalData) = 0;

  ///////////////////////////////////////////////////////////////////
  // Print ascii representation of value into buffer
  virtual const char *ascii() = 0;

  ///////////////////////////////////////////////////////////////////
  // Set value, given string representation
  virtual void parseValue(const char *stringRep) = 0;


private:

  char _name[65];
  char _asciiFormat[16];
  char _longName[301];
  char _units[65];
};


#endif

