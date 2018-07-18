///
/// @file mconfig.c
/// @authors k. Headley
/// @date 01 jan 2018
 
/// Project module initialization and configuration

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
/////////////////////////
// Headers 
/////////////////////////

#include "mconfig.h"

/////////////////////////
// Macros
/////////////////////////

// These macros should only be defined for 
// application main files rather than general C files
/*
/// @def PRODUCT
/// @brief header software product name
#define PRODUCT "MBRT"

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

/////////////////////////
// Declarations 
/////////////////////////

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////

/// @var module_debug_config_t debug_config_dfl[]
/// @brief application specific module configuration table
/// Applications should define this table, using module ID enum
/// defined in mconfig.h
module_debug_config_t debug_config_dfl[]={
    {MBTRN,   MDL_ERROR},
    {R7K,     MDL_ERROR},
    {MREADER, MDL_ERROR},
    {RPARSER, MDL_ERROR},
    {DRFCON,  MDL_ERROR},
    {APP,     MDL_ERROR},
    {APP1,    MDL_ERROR},
    {APP2,    MDL_ERROR},
    {APP3,    MDL_ERROR},
    {APP4,    MDL_ERROR},
    {APP5,    MDL_ERROR},
    {0, 0}
};

/////////////////////////
// Function Definitions
/////////////////////////

/// @fn void mcfg_configure(module_debug_config_t * dcfg, uint32_t entries)
/// @brief Application specific module configuration.
/// @param[in] dcfg configuration data (or NULL to use defaults defined
/// in mconfig.c)
/// @param[in] entries number of configuration entries.
/// @return none
void mcfg_configure(module_debug_config_t *dcfg, uint32_t entries)
{
    if (NULL!=dcfg) {
        for (uint32_t i=0; i<entries; i++) {
            mdb_set(dcfg[i].module, dcfg[i].level);
        }
    }else{
        module_debug_config_t *pcfg=debug_config_dfl;
        while(pcfg->module != 0) {
            mdb_set(pcfg->module, pcfg->level);
            pcfg++;
        }
    }
}
// End function mcfg_configure


