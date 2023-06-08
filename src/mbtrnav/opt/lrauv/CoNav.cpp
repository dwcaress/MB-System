/*****************************************************************************
 * Copyright (c) 2002-2021 MBARI
 * Monterey Bay Aquarium Research Institute, all rights reserved.
 *****************************************************************************
 * @file    CoNav.cpp
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

/***********************************************************************
 * Headers
 ***********************************************************************/

#include <cmath>
#include <math.h>

// Status logging
//#define ZF_LOG_LEVEL ZF_LOG_INFO
#define ZF_LOG_TAG "CoNav:"
#include <zf_log.h>

#include "CoNav.h"

/***********************************************************************
 * Macros
 ***********************************************************************/

/***********************************************************************
 * Code
 ***********************************************************************/

// CoNav class
// Purpose is to contain and manage the MRFilters
// Performs final calculation that combines the best of the filters
CoNav::CoNav()
    : _northing(0.0), _easting(0.0), _log(NULL)
{
    memset(&_conavState, 0, sizeof(_conavState));

    for (int i = 0; i < MAX_FILTERS; i++) {
        _mrFilters[i] = NULL;
    }

    arma::mat pb(2, 2, arma::fill::eye);
    _pBest = pb;
    _pBest *= 10000.;

    std::string logname = CoNavLogName;
    _log = new CoNavLog(logname);
}

CoNav::~CoNav()
{
    // Release my MRFilters
    for (int i = 0; i < MAX_FILTERS; i++) {
        DELOBJ(_mrFilters[i]);
    }
    DELOBJ(_log);
}

// A conav update occurs when this vehicle receives a data packet from a
// cooperating vehicle - i.e., a vehicle that is tracking this vehicle
// using a DAT instrument. The CoopVehicleNavData contains the range
// and bearing to this vehicle, the cooperating vehicle's N and E, etc.
// This function passes the data on to the associated MRFilter for
// processing and then calls combine_filters() to obtain the best
// estimate from all the filters.
void CoNav::conav_update(MRFilterLog::CoopVehicleNavData& conavdata)
{
    MRFilter* mrf = NULL;

    if (conavdata.coopClock > _time) {
        ZF_LOGW("CoNav time > nav time: %.3f > %.3f",
            conavdata.coopClock, _time);
    }

    // Delegate to the associated MRFilter if one exists.
    // Otherwise create a MRFilter for this cooperating vehicle.
    if (NULL != _mrFilters[conavdata.vehId]) {
        // Very simplistic method of organizing the MR filters.
        // The vehId (range 0 to 9) is used to index into an array
        // of pointers to dynamically allocated MRFilter objects.
        mrf = _mrFilters[conavdata.vehId];
    } else {
        // allocate a new MRFilter and add it into the list
        if (conavdata.vehId >= MAX_FILTERS) {
            ZF_LOGE("Unable to create new MRFilter!");
            ZF_LOGE("Vehicle Id %d out of range", conavdata.vehId);
            return;
        }
        mrf = new MRFilter(conavdata.vehId, _northing, _easting, _time);
        _mrFilters[conavdata.vehId] = mrf;
        ZF_LOGD("Created filter %d", mrf->getId());
    }

    // Process the measurement update on the MRFilter
    if (NULL != mrf) {
        mrf->conav_update(conavdata);
        // Obtain the best estimate from all the filters
        //_conavState.timestamp = fmax(_conavState.timestamp,conavdata.timestamp);
        _conavState.timestamp = conavdata.egoClock;
        combine_filters();
        // show nav position, conav position, and difference
        ZF_LOGD("conavstate, %.2f %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f",
                _time, _northing, _easting, _conavState.timestamp,
                _conavState.bestNorthing, _conavState.bestEasting,
                _northing - _conavState.bestNorthing,
                _easting - _conavState.bestEasting);
        // log data
        _log->setCoNavMeas(conavdata);
        _log->write();
    } else {
        ZF_LOGE("No MRFilter object for Vehicle Id %d", conavdata.vehId);
    }
}


// A motion update occurs at a rate determined by the calling application.
// The VehicleNavData record contains fresh vehicle position data produced
// by the vehicle control system.  
void CoNav::motion_update(MRFilterLog::VehicleNavData& navdata)
{
    // Process navdata on my filter
    _northing = navdata.northing;
    _easting  = navdata.easting;
    _time = navdata.egoClock;

    // TODO: Replace this with config strings
    // Initialize with two MR filters with id 1 and 2
    if (_mrFilters[1] == NULL) {
        _mrFilters[1] = new MRFilter(1, _northing, _easting, _time);
        ZF_LOGD("Created filter %d", _mrFilters[1]->getId());
        _mrFilters[2] = new MRFilter(2, _northing, _easting, _time);
        ZF_LOGD("Created filter %d", _mrFilters[2]->getId());
    }

    // Process navdata on my MRFilters
    for (int i = 0; i < MAX_FILTERS; i++) {
        if (NULL != _mrFilters[i]) {
            _mrFilters[i]->motion_update(navdata, _pBest);
        }
    }

    _conavState.timestamp = navdata.egoClock;
    combine_filters();
    _log->setCoNavMotion(navdata);
    _log->write();
}

// Combine filter information to arrive at the best position estimate
/* Computation modeled after Steve Rock's Matlab script
function [N3best,E3best,Pbest] = CombineFilters(N31,E31,N32,E32,P31,P32)

      1 x1=[N31;E31];
        x2=[N32;E32];
        |---------- B -------------|
                  |------ A ------|   <-- accumulated in the MRFilter loop
      2 Pbest=inv(inv(P31)+inv(P32));
        |---------- B -------------|
                    |---------- A ----------|  
      3 xbest=Pbest*(inv(P31)*x1+inv(P32)*x2);  <-- accumulated in the loop
      4 N3best=xbest(1);
        E3best=xbest(2);

end
*/

void CoNav::combine_filters()
{
    // step 1
    arma::mat pb(2, 2, arma::fill::zeros);
    arma::mat xb(2, 1, arma::fill::zeros);

    // iterate over the filters accumulate 2A and 3A
    for (int i = 0; i < MAX_FILTERS; i++) {
        MRFilter* mrf = _mrFilters[i];
        if (mrf != NULL) {
            ZF_LOGD("Combining filter %d", i);
            // step 2A
            pb += mrf->_P.i();
            // step 3A
            arma::mat x(2,1);
            x(0,0) = mrf->northing();
            x(1,0) = mrf->easting();
            xb += (mrf->_P.i() * x);
        }
    }

    // step 2B
    _pBest = pb.i();
    // step 3B
    arma::mat xbest = _pBest * xb;
    _conavState.bestNorthing = xbest(0);
    _conavState.bestEasting = xbest(1);
    ZF_LOGD("N3best:%.2f E3best:%.2f  best diff %.2f %.2f",
        xbest(0), xbest(1), xbest(0) - _northing, xbest(1) - _easting);

    _log->setCoNavPbest(_pBest);
    _log->setCoNavState(_conavState);

    return;
}

// Occurs when a TRN update is published by this vehicle
void CoNav::trn_update(TrnData& trndata)
{
}

// Return the current best filter estimate
void CoNav::get_state(CoNavLog::CoNavState& cnstate)
{
    cnstate = _conavState;
}
