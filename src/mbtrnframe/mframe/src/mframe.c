///
/// @file mframe.c
/// @authors k. Headley
/// @date 11 aug 2017

/// General description of module here

/// @sa doxygen-examples.c for more examples of Doxygen markup


/////////////////////////
// Terms of use
/////////////////////////
/*
 Copyright Information
 
 Copyright 2002-2017 MBARI
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

/////////////////////////
// Headers 
/////////////////////////
#include "mframe.h"

/////////////////////////
// Macros
/////////////////////////

// These macros should only be defined for
// application main files rather than general C files
/*
 /// @def PRODUCT
 /// @brief header software product name
 #define PRODUCT "MFRAME"
 
 /// @def COPYRIGHT
 /// @brief header software copyright info
 #define COPYRIGHT "Copyright 2002-2013 MBARI Monterey Bay Aquarium Research Institute, all rights reserved."
 /// @def NOWARRANTY
 /// @brief header software terms of use
 #define NOWARRANTY  \
 "This program is distributed in the hope that it will be useful,\n"\
 "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"\
 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"\
 "GNU General Public License for more details (http://www.gnu.org/licenses/gpl-3.0.html)\n"
 */

#ifndef MFRAME_NAME
/// @def MFRAME_NAME
/// @brief MFRAME library name
#define MFRAME_NAME "mframe"
#endif

#ifndef MFRAME_VER
/// @def MFRAME_VER
/// @brief MFRAME library build version.
#define MFRAME_VER 1.0.0
#endif

#ifndef MFRAME_BUILD
/// @def MFRAME_BUILD
/// @brief MFRAME library build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMFRAME_BUILD=`date`
#define MFRAME_BUILD "0000/00/00T00:00:00-0000"
#endif

/// @def MFRAME_NAME_STR
/// @brief library name string macro.
#define MFRAME_NAME_STR ""VERSION_STRING(MFRAME_NAME)
/// @def MFRAME_VERSION_STR
/// @brief library version string macro.
#define MFRAME_VERSION_STR ""VERSION_STRING(MFRAME_VER)
/// @def MFRAME_BUILD_STR
/// @brief library build date string macro.
#define MFRAME_BUILD_STR ""VERSION_STRING(MFRAME_BUILD)

/////////////////////////
// Declarations
/////////////////////////

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////

/////////////////////////
// Function Definitions
/////////////////////////
/// @fn const char mframe_name()
/// @brief get name string.
/// @return name string
const char *mframe_name()
{
    return MFRAME_NAME_STR;
}
// End function mframe_name

/// @fn const char *mframe_version()
/// @brief get version string.
/// @return version string
const char *mframe_version()
{
    return MFRAME_VERSION_STR;
}
// End function mframe_version

/// @fn const char *mframe_build()
/// @brief get build string.
/// @return version string
const char *mframe_build()
{
    return MFRAME_BUILD_STR;
}

// End function mframe_build
