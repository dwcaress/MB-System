/*****************************************************************************
 * Copyright (c) 2002-2021 MBARI
 * Monterey Bay Aquarium Research Institute, all rights reserved.
 *****************************************************************************
 * @file    MRFilter.h
 * @authors r. henthorn
 * @date    12/03/2021
 * @brief   The MRFilter class contains the logic for Terrain-Relative
 *          Co-Navigation multi-robot filters. These filters track the EGO
 *          vehicle position based on the measurements from cooperating
 *          vehicles. One filter is created for every cooperating vehicle.
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
#ifndef MRFILTER_H
#define MRFILTER_H

/***********************************************************************
 * Headers
 ***********************************************************************/

#include <string.h>
#include <armadillo>
#include "lrconav_app.h"
#include "MRFilterLog.h"

/***********************************************************************
 * Macros
 ***********************************************************************/

#define RANGE_THRESHOLD 50.0
#define SIGMA_THRESHOLD 7.0
#define FILTER_FUN_KF 1
#define FILTER_FUN_CI 0

/***********************************************************************
 * Code
 ***********************************************************************/

// TRN info from the TRN instance on this vehicle (est nav, particles, etc)
struct TrnData
{
    // TBD
};

// MRFilter class
// Most of the computaions calculating ego vehicle estimates
// occur in the multi-robot filters
class MRFilter
{
public:
    MRFilter(int filterId, double initialN, double initialE, double initialTime);
   ~MRFilter();

   int getId() { return _id; }

    // Process position, range, and bearing data from a cooperating vehicle
    void conav_update(MRFilterLog::CoopVehicleNavData& conavdata);
    // Process position and attitude data from the Ego vehicle (this vehicle)
    void motion_update(MRFilterLog::VehicleNavData& navdata, const arma::mat& pBest);
    // accessors
    double northing() { return _northing; }
    double easting() { return _easting; }

    // computational matrices
    // result of filter_fun()
    arma::mat _P;        // state covariance

//protected:
    int _id;
    int _origin;
    double _northing;    // updated ego position estimate
    double _easting;
    double _deltaN;      // delta N/E from latest motion_update
    double _deltaE;
    double _deltaT;      // time delta of last two motion_updates
    double _time;        // time from last motion_update() data
    double _timelast;    // time from last conav_update() data

    MRFilterLog *_log;
    MRFilterLog::MRFilterState _state;

    // copy of most recently used datasets
    MRFilterLog::VehicleNavData _lastNav;
    MRFilterLog::CoopVehicleNavData _lastCoopData;

    // Computes and updates all matrix parameters
    void range_bearing(arma::mat& deltaz, arma::mat& rij,
                       arma::mat& hi, arma::mat& hj, arma::mat& pj);

    // Computes and updates P (process noise)
    void filter_fun(int kf, arma::mat& P, const arma::mat deltaz,
                    const arma::mat pij, const arma::mat rij,
                    const arma::mat hi, const arma::mat hj,
                    const arma::mat pj);

    // Computes and updates P (process noise)
    void kf_filter_fun(arma::mat& P, const arma::mat deltaz,
                       const arma::mat pij, const arma::mat rij,
                       const arma::mat hi, const arma::mat hj,
                       const arma::mat pj);

    // Computes and updates P (process noise)
    void ci_filter_fun(arma::mat& P, const arma::mat deltaz,
                       const arma::mat pij, const arma::mat rij,
                       const arma::mat hi, const arma::mat hj,
                       const arma::mat pj);

    double wopt(const arma::mat& Pij, const arma::mat& Hi,
                const arma::mat& Hj, const arma::mat& R1x,
                const arma::mat& Rij);

    // Computes and updates Qbar
    void calculate_qbar(arma::mat& Qbar, const arma::mat& q,
                        const arma::mat& p, const arma::mat& pBest);
};

#endif
