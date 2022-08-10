/*****************************************************************************
 * Copyright (c) 2002-2021 MBARI
 * Monterey Bay Aquarium Research Institute, all rights reserved.
 *****************************************************************************
 * @file    lrconav_app.h
 * @authors r. henthorn
 * @date    11/10/2021
 * @brief   Header file for the main program of Terrain-Relative Co-Navigation
 *          interfacing with LCM.
 *
 * Project: Precision Control
 * Summary: A Terrain-Relative Co-Navigation implementation that uses LCM for
            message passing,
 *****************************************************************************/
/*****************************************************************************
 * Copyright Information:
 * Copyright 2002-2021 MBARI
 * Monterey Bay Aquarium Research Institute, all rights reserved.
 *
 * Terms of Use:
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version. You can access the GPLv3 license at
 * http://www.gnu.org/licenses/gpl-3.0.html
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details (http://www.gnu.org/licenses/gpl-3.0.html).
 *
 * MBARI provides the documentation and software code "as is", with no
 * warranty, express or implied, as to the software, title, non-infringement
 * of third party rights, merchantability, or fitness for any particular
 * purpose, the accuracy of the code, or the performance or results which you
 * may obtain from its use. You assume the entire risk associated with use of
 * the code, and you agree to be responsible for the entire cost of repair or
 * servicing of the program with which you are using the code.
 *
 * In no event shall MBARI be liable for any damages,whether general, special,
 * incidental or consequential damages, arising out of your use of the
 * software, including, but not limited to,the loss or corruption of your data
 * or damages of any kind resulting from use of the software, any prohibited
 * use, or your inability to use the software. You agree to defend, indemnify
 * and hold harmless MBARI and its officers, directors, and employees against
 * any claim,loss,liability or expense,including attorneys' fees,resulting from
 * loss of or damage to property or the injury to or death of any person
 * arising out of the use of the software.
 *
 * The MBARI software is provided without obligation on the part of the
 * Monterey Bay Aquarium Research Institute to assist in its use, correction,
 * modification, or enhancement.
 *
 * MBARI assumes no responsibility or liability for any third party and/or
 * commercial software required for the database or applications. Licensee
 * agrees to obtain and maintain valid licenses for any additional third party
 * software required.
 *****************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

/***********************************************************************
 * Macros
 ***********************************************************************/

// clang-format off
#define MOTION_PERIOD 2.0      // seconds
#define AHRS_CHANNEL           "AHRS_M2"
#define NAV_CHANNEL            "DeadReckonUsingMultipleVelocitySources"
#define NAV_LAT                "latitude"
#define NAV_LON                "longitude"
#define DVL_CHANNEL            "RDI_Pathfinder"
#define DEPTH_CHANNEL          "Depth_Keller"
#define DEPTH_DEPTH            "depth"
#define COOP_CHANNEL           "CoNav_Cooperative_Vehicle_Data"
#define EGO_CHANNEL            "CoNav_Ego"
#define CONAV_VEHID_NAME       "VehId"
#define CONAV_TIME_NAME        "Timestamp"     // epoch seconds
#define CONAV_TRN_N_NAME       "TrnNorth"      // meters
#define CONAV_TRN_E_NAME       "TrnEast"       // meters
#define CONAV_TRN_Z_NAME       "TrnDepth"      // meters
#define CONAV_TRN_VAR_N_NAME   "TrnVarNorth"
#define CONAV_TRN_VAR_E_NAME   "TrnVarEast"
#define CONAV_TRN_VAR_Z_NAME   "TrnVarDepth"
#define CONAV_RANGE_NAME       "Range"         // meters
#define CONAV_BEARING_NAME     "Bearing"       // radians
#define CONAV_RANGE_VAR_NAME   "RangeVar"
#define CONAV_BEARING_VAR_NAME "BearingVar"
// clang-format on

// Define a useful way to delete and reset a pointer to an object.
// If the ptr is non-NULL, delete the object referenced by the ptr
// and reset the ptr to NULL.
// E.g.:
// SomeObj *obj = new SomeObject();
// DELOBJ(obj);
//
#define DELOBJ(ptr)                                                            \
    {                                                                          \
        if (NULL != ptr) {                                                     \
            delete ptr;                                                        \
            ptr = NULL;                                                        \
        }                                                                      \
    }

