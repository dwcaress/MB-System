///
/// @file mxd_app.h
/// @authors k. Headley
/// @date 10 jul 2023

/// mxdebug module ID defiinitions
/// This header satisfies module definition dependencies in the absence
/// of a global mxd_app.h.
/// Applications using multiple libraries that include mxdebug should combine their module IDs
/// into a single mxd_app.h

/////////////////////////
// Terms of use
/////////////////////////
/*
 Copyright Information

 Copyright 2000-2023 MBARI
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

#ifndef MXD_APP_H
#define MXD_APP_H

#include "mxdebug.h"
#include "mxdebug-common.h"

typedef enum{
    MBTRNPP = MX_APP_RANGE,
    R7KR,
    R7KR_DEBUG,
    R7KR_ERROR,
    R7KC,
    R7KC_DEBUG,
    R7KC_ERROR,
    R7KC_PARSER,
    R7KC_DRFCON,
    MB1R,
    MB1R_DEBUG,
    MB1R_ERROR,
    NETIF,
    NETIF_DEBUG,
    NETIF_ERROR,
    TRNC,
    TRNC_DEBUG,
    TRNC_ERROR,
    MB1IO,
    MB1IO_DEBUG,
    MB1IO_ERROR,
    EMU7K,
    EMU7K_DEBUG,
    EMU7K_ERROR,
    FRAMES7K,
    FRAMES7K_DEBUG,
    FRAMES7K_ERROR,
    MBTNAVC,
    MBTNAVC_DEBUG,
    MBTNAVC_ERROR,
    STREAM7K,
    STREAM7K_DEBUG,
    STREAM7K_ERROR,
    TBINX,
    TBINX_DEBUG,
    TBINX_ERROR,
}mxd_id_t;

#endif
