///
/// @file merror.c
/// @authors k. Headley
/// @date 01 jan 2018
 
/// General purpose error return and string facility
/// similar to errno/strerror, may be used for consistency
/// across application modules

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
#include "merror.h"

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

int me_errno=ME_OK;
static const char *err_undef="UNDEFINED_ERR";

/////////////////////////
// Function Definitions
/////////////////////////

/// @fn const char * me_strerror(int merrno)
/// @brief return error string mnemonics (application specific).
/// Applications should implement this function.
/// @param[in] merrno error number enum
/// @return error mnemonic string
const char *me_strerror(int merrno)
{
    const char *retval = err_undef;
    switch (merrno) {
        case ME_EUNKNOWN:
            retval="EUNKNOWN";
            break;
        case ME_OK:
            retval="OK";
            break;
        case ME_ECREATE:
            retval="ECREATE";
            break;
        case ME_ECONNECT:
            retval="ECONNECT";
            break;
        case ME_ESUB:
            retval="ESUB";
            break;
        case ME_EREAD:
            retval="EREAD";
            break;
        case ME_EPOLL:
            retval="EPOLL";
            break;
        case ME_EPARSE:
            retval="EPARSE";
            break;
        case ME_EINVAL:
            retval="EINVAL";
            break;
        case ME_ETMOUT:
            retval="ETMOUT";
            break;
        case ME_EINC:
            retval="EINC";
            break;
        case ME_ERCV:
            retval="ERCV";
            break;
        case ME_ESOCK:
            retval="ESOCK";
            break;
        case ME_ENOMEM:
            retval="ENOMEM";
            break;
        case ME_ENOSPACE:
            retval="ENOSPACE";
            break;
        case ME_EOF:
            retval="EOF";
            break;
            
        default:
            break;
    }
    return retval;
}
// End function me_strerror


