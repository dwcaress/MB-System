///
/// @file mconfig.h
/// @authors k. headley
/// @date 06 nov 2012
 
/// Library configuration
/// set debug parameters for modules in this project
 
/// @sa doxygen-examples.c for more examples of Doxygen markup
 

/////////////////////////
// Terms of use 
/////////////////////////
/*
 Copyright Information
 
 Copyright 2000-2018 MBARI
 Monterey Bay Aquarium Research Institute, all rights reserved.
 
 Terms of Use
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version. You can access the GPLv3 license at
 http://www.gnu.org/licenses/gpl-3.0.html
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details 
 (http://www.gnu.org/licenses/gpl-3.0.html)
 
 MBARI provides the documentation and software code "as is", with no warranty,
 express or implied, as to the software, title, non-infringement of third party 
 rights, merchantability, or fitness for any particular purpose, the accuracy of
 the code, or the performance or results which you may obtain from its use. You 
 assume the entire risk associated with use of the code, and you agree to be 
 responsible for the entire cost of repair or servicing of the program with 
 which you are using the code.
 
 In no event shall MBARI be liable for any damages, whether general, special,
 incidental or consequential damages, arising out of your use of the software, 
 including, but not limited to, the loss or corruption of your data or damages 
 of any kind resulting from use of the software, any prohibited use, or your 
 inability to use the software. You agree to defend, indemnify and hold harmless
 MBARI and its officers, directors, and employees against any claim, loss, 
 liability or expense, including attorneys' fees, resulting from loss of or 
 damage to property or the injury to or death of any person arising out of the 
 use of the software.
 
 The MBARI software is provided without obligation on the part of the 
 Monterey Bay Aquarium Research Institute to assist in its use, correction, 
 modification, or enhancement.
 
 MBARI assumes no responsibility or liability for any third party and/or 
 commercial software required for the database or applications. Licensee agrees 
 to obtain and maintain valid licenses for any additional third party software 
 required.
 */

// include guard
#ifndef MCONFIG_H
/// @def MCONFIG_H
/// @brief include guard
#define MCONFIG_H

/////////////////////////
// Includes 
/////////////////////////

#include <stdlib.h>
#include <stdint.h>
#include "mdebug.h"

/////////////////////////
// Type Definitions
/////////////////////////

/// @def MBTRN
/// @brief debug module ID.use these module IDs with mdebug.h MM* macros (MMDEBUG etc.)
/// The value 0 is reserved (for global setting); values must be >= 1
#define MBTRN      1
/// @def MBTRNV
/// @brief debug module ID.use these module IDs with mdebug.h MM* macros (MMDEBUG etc.)
/// The value 0 is reserved (for global setting); values must be >= 1
#define MBTRNV     2
/// @def R7K
/// @brief debug module ID. use these module IDs with mdebug.h MM* macros (MMDEBUG etc.)
/// The value 0 is reserved (for global setting); values must be >= 1
#define R7K        3
/// @def MREADER
/// @brief debug module ID. use these module IDs with mdebug.h MM* macros (MMDEBUG etc.)
/// The value 0 is reserved (for global setting); values must be >= 1
#define MREADER    4
/// @def RPARSER
/// @brief debug module ID. use these module IDs with mdebug.h MM* macros (MMDEBUG etc.)
/// The value 0 is reserved (for global setting); values must be >= 1
#define RPARSER    5
/// @def DRFCON
/// @brief debug module ID. use these module IDs with mdebug.h MM* macros (MMDEBUG etc.)
/// The value 0 is reserved (for global setting); values must be >= 1
#define DRFCON     6
/// @def IOW
/// @brief debug module ID. use these module IDs with mdebug.h MM* macros (MMDEBUG etc.)
/// The value 0 is reserved (for global setting); values must be >= 1
#define IOW        7
/// @def APP
/// @brief debug module ID. use these module IDs with mdebug.h MM* macros (MMDEBUG etc.)
/// The value 0 is reserved (for global setting); values must be >= 1
#define APP        8
/// @def APP1
/// @brief debug module ID. use these module IDs with mdebug.h MM* macros (MMDEBUG etc.)
/// The value 0 is reserved (for global setting); values must be >= 1
#define APP1       9
/// @def APP2
/// @brief debug module ID. use these module IDs with mdebug.h MM* macros (MMDEBUG etc.)
/// The value 0 is reserved (for global setting); values must be >= 1
#define APP2       10
/// @def APP3
/// @brief debug module ID. use these module IDs with mdebug.h MM* macros (MMDEBUG etc.)
/// The value 0 is reserved (for global setting); values must be >= 1
#define APP3       11
/// @def APP4
/// @brief debug module ID. use these module IDs with mdebug.h MM* macros (MMDEBUG etc.)
/// The value 0 is reserved (for global setting); values must be >= 1
#define APP4       12
/// @def APP5
/// @brief debug module ID. use these module IDs with mdebug.h MM* macros (MMDEBUG etc.)
/// The value 0 is reserved (for global setting); values must be >= 1
#define APP5       13
/// @def MAX_MODULE
/// @brief debug module ID. use these module IDs with mdebug.h MM* macros (MMDEBUG etc.)
/// The value 0 is reserved (for global setting); values must be >= 1
#define MAX_MODULE 14

/////////////////////////
// Macros
/////////////////////////

/// @def MBTRN_TIMING
/// @brief enable timing output measurements and console output
/// [iowrap-posix.c, mbtrn.c...]
#undef MBTRN_TIMING

/// @def MBTR_STATS_EN
/// @brief enable statistics measurements and logging
/// [mbtrnpreprocess.c, mbtrn.c...]
#define MBTR_STATS_EN
/// @def MBTRNPP_STAT_PERIOD_SEC
/// @brief default period at which to log statistics measurements
/// may set on mbtrnpreprocess command line using --statsec option
/// [mbtrnpreprocess.c, mbtrn.c...]
#define MBTRNPP_STAT_PERIOD_SEC ((double)20.0)

/// @def MC_DFL_LEVEL
/// @brief default debug level
#define MC_DFL_LEVEL MDL_ERROR

/////////////////////////
// Exports
/////////////////////////

// this may be defined and called once at startup by the main project module
// implementation should initialize module(s) default values
// void mcfg_init();

/// @def mcfg_init
/// @brief One-time application initialization (not implemented by default)
/// this may be defined and called once at startup by the main project module
/// implementation should initialize module(s) default values
#define mcfg_init()

// define and call to set module debug configuration(s) at run time
// call using dcfg=NULL to use compile-time default values defined in mconfig.c

void mcfg_configure(module_debug_config_t *dcfg, uint32_t entries);

// include guard
#endif