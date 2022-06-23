/*****************************************************************************
 * Copyright (c) 2002-2021 MBARI
 * Monterey Bay Aquarium Research Institute, all rights reserved.
 *****************************************************************************
 * @file    CoNav.h
 * @authors r. henthorn
 * @date    11/09/2021
 * @brief   The CoNav class contains the logic for Terrain-Relative
 *          Co-Navigation where one or more vehicles use their TRN-aided
 *          positions combined with range and bearing measurements to aid
 *          the navigation of another vehicle.
 *
 * Project: Precision Control
 * Summary:
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
#ifndef CONAV_H
#define CONAV_H

/***********************************************************************
 * Headers
 ***********************************************************************/

#include <string.h>
#include <armadillo>
#include "lrconav_app.h"
#include "CoNavLog.h"
#include "MRFilter.h"

/***********************************************************************
 * Macros
 ***********************************************************************/

#define MAX_FILTERS 10

/***********************************************************************
 * Code
 ***********************************************************************/

// CoNav class containing MR filters, ego filter, process noise filters,
// calculated best position estimate
class CoNav
{
public:
    CoNav();
   ~CoNav();

    // Process position, range, and bearing data from a cooperating vehicle
    void conav_update(MRFilterLog::CoopVehicleNavData& conavdata);
    // Process position and attitude data from the Ego vehicle (this vehicle)
    void motion_update(MRFilterLog::VehicleNavData& navdata);
    // Combine filter information to arrive at the best position estimate
    void combine_filters();
    // Process a TRN update from the Ego vehicle
    void trn_update(TrnData& trndata);
    // Return the current Co-Navigated state from the CoNav object
    void get_state(CoNavLog::CoNavState& cnstate);

protected:

    MRFilter* _mrFilters[MAX_FILTERS];
    double _northing;
    double _easting;
    double _time;
    arma::mat _pBest;

    struct CoNavLog::CoNavState _conavState;
    CoNavLog* _log;

};

#endif
