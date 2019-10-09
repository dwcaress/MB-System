/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : DataFieldFactory.h                                            */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#ifndef _DATAFIELDFACTORY_H
#define _DATAFIELDFACTORY_H

#include "DataField.h"

/*
CLASS 
DataFieldFactory

DESCRIPTION
Factory creates DataField objects based on DataField subclass 
"type mnemonic".

DataFieldFactory is a singleton.

AUTHOR
Tom O'Reilly
*/

class DataFieldFactory {
  
public:

  DataField *create(const char *typeMnemomic, const char *name);

  static DataFieldFactory *instanceOf();

protected:

  DataFieldFactory();

  ~DataFieldFactory();

  static DataFieldFactory *_instanceOf;
};


#endif
