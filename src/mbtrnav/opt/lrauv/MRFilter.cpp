/*****************************************************************************
 * Copyright (c) 2002-2021 MBARI
 * Monterey Bay Aquarium Research Institute, all rights reserved.
 *****************************************************************************
 * @file    CoNav.cpp
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

/***********************************************************************
 * Headers
 ***********************************************************************/

#include <cmath>
#include <math.h>

// Status logging
#define ZF_LOG_TAG "MRFilter:"
#include <zf_log.h>

#include "MRFilter.h"

/***********************************************************************
 * Macros
 ***********************************************************************/

/***********************************************************************
 * Code
 ***********************************************************************/

// Floating-point modulo
// The result (the remainder) has same sign as the divisor.
// Similar to matlab's mod(); Not similar to fmod() -   Mod(-3,4)= 1   fmod(-3,4)= -3
double Mod(double x, double y)
{
    if (0. == y) {
        return x;
    }
    double m= x - y * floor(x/y);

    // handle boundary cases resulted from floating-point cut off:
    if (y > 0) {            // modulo range: [0..y)
        if (m>=y) {         // Mod(-1e-16             , 360.    ): m= 360.
            return 0;
        }
        if (m<0 ) {
            if (y+m == y)
                return 0  ; // just in case...
            else
                return y+m; // Mod(106.81415022205296 , _TWO_PI ): m= -1.421e-14
        }
    } else {                // modulo range: (y..0]
        if (m<=y) {         // Mod(1e-16              , -360.   ): m= -360.
            return 0;
        }
        if (m>0 ) {
            if (y+m == y)
                return 0  ; // just in case...
            else
                return y+m; // Mod(-106.81415022205296, -_TWO_PI): m= 1.421e-14
        }
    }
    return m;
}

#define M_2PI (2*M_PI)

// wrap [rad] angle to [-PI..PI)
inline double WrapPosNegPI(double fAng)
{
    return Mod(fAng + M_PI, M_2PI) - M_PI;
}

// wrap [rad] angle to [0..TWO_PI)
inline double WrapTwoPI(double fAng)
{
    return Mod(fAng, M_2PI);
}

// wrap [deg] angle to [-180..180)
inline double WrapPosNeg180(double fAng)
{
    return Mod(fAng + 180., 360.) - 180.;
}

// wrap [deg] angle to [0..360)
inline double Wrap360(double fAng)
{
    return Mod(fAng ,360.);
}

MRFilter::MRFilter(int filterId, double initN, double initE, double initTime)
    : _id(filterId), _origin(0), _northing(initN), _easting(initE),
      _deltaN(0.0), _deltaE(0.0), _time(initTime), _timelast(initTime)
{
    _lastNav.egoTime = initTime;
    _lastNav.navN    = initN;
    _lastNav.navE    = initE;

    memset(&_lastMRData, 0, sizeof(_lastMRData));

    arma::mat pn(2, 2, arma::fill::eye);
    _Pij = pn;
    _Pij *= PIJ_INIT;

    std::string logname = MRFilterLogName + std::to_string(_id);
    _log = new MRFilterLog(_id, logname);
}

MRFilter::~MRFilter()
{
    if (_log != NULL) {
        delete _log;
        _log = NULL;
    }
}

// Process position, range, and bearing data from a cooperating vehicle
void MRFilter::measure_update(CoNav::MRDATInput& mrdata)
{
    // Computation modeled after Steve Rock's Matlab script
    _time   = mrdata.egoTime;
    _state.egoClock = _time;
    _lastMRData = mrdata;
    _log->setMRFilterMeas(mrdata);
    _log->setMRFilterState(_state);

    double convergence = sqrt(mrdata.njCovar + mrdata.ejCovar);
    // The sending vehicle's TRN filter must be converged and the
    // range value must be less than the threshold
    if (convergence     >  SIGMA_THRESHOLD ||
        mrdata.range <= RANGE_THRESHOLD) {
        ZF_LOGD("MRFilter %d threshold fail: sigma is %.2f, range is %.2f",
                _id, convergence, mrdata.range);
        // log measurement data regardless
        _origin = ORIGIN_THRESHOLD_FAIL;
        _log->setMRFilterP(_origin, _Pij);
        _log->write();
        return;
    }

    // If last event was also an accepted measurement do not allow two in a row
    if (ORIGIN_MEASUREMENT == _origin) {
        ZF_LOGD("MRFilter %d: last event was a good measurement, skipping...",
                _id);
        return;
    }

    // Process valid hit from cooperating vehicle
    // log everything here
    arma::mat deltaz(2, 1, arma::fill::zeros);
    arma::mat rij(2, 2, arma::fill::zeros);
    arma::mat hi(2, 2, arma::fill::zeros);
    arma::mat hj(2, 2, arma::fill::zeros);
    arma::mat pj(2, 2, arma::fill::zeros);
    range_bearing(deltaz, rij, hi, hj, pj);

    // Record data for logging
    _log->setMRFilterDeltaz(deltaz);
    _log->setMRFilterRij(rij);
    _log->setMRFilterHi(hi);
    _log->setMRFilterHj(hj);
    _log->setMRFilterPj(pj);

    // Update and record filter
    arma::mat pij = _Pij;
    meas_update_MRFilter(FILTER_FUN_CI, _Pij, deltaz, pij, rij, hi, hj, pj);
    _origin = ORIGIN_MEASUREMENT;
    _log->setMRFilterP(_origin, _Pij);
    _timelast = mrdata.datTime;

    // Log data
    _log->write();
}

// Computation modeled after the Range_Bearing function in Rock's scripts
//    range_bearing(deltaz, rij, hi, hj, pj);
void MRFilter::range_bearing(arma::mat& deltaz, arma::mat& rij,
                             arma::mat& hi, arma::mat& hj, arma::mat& pj)
{
    /*
    function [deltaz,Rij,Hi,Hj,Pj] = Range_Bearing(Range,Bearing,RangeSigma,BearingSigma,
                                                   nijTDAT,eijTDAT,Nj,Ej,Njvar,Ejvar)
      1 R1z=[RangeSigma^2 0 ; 0 BearingSigma^2];
      2 R1x=[Njvar 0 ; 0 Ejvar];
        %Bearing=zeroto2pi(Bearing);
      3 Bearing=wrapTo2Pi(Bearing);
        Hi=zeros(2,2);
        Hi(1,1)= cos(Bearing);   %range/N  also (Nhat-N1)/range
        Hi(1,2)= sin(Bearing);   %range/E  also (Ehat-E1)/range
        Hi(2,1)= -(eijTDAT-Ej)/Range^2;   %bearing/N
        Hi(2,2)=  (nijTDAT-Nj)/Range^2;   %bearing/E
      4 Hj=-Hi;
      5 Pj=R1x;
      6 Rij=R1z;
      7 z=[Range;Bearing];
        %zhat=[sqrt((nijTDAT-Nj)^2+(eijTDAT-Ej)^2); zeroto2pi(atan2(eijTDAT-Ej,nijTDAT-Nj))];
      8 zhat=[sqrt((nijTDAT-Nj)^2+(eijTDAT-Ej)^2); wrapTo2Pi(atan2(eijTDAT-Ej,nijTDAT-Nj))];
      9 deltaz=[z(1)-zhat(1);-angdiff(z(2),zhat(2))];
    */
    // Data timestamp from sending vehicle is less than now, so
    // interpolate to achieve my vehicle's position at time of measurement
    //double f = (_time - _lastMRData.timestamp) / (_time - _timelast);
    double f = (_time - _lastMRData.egoTime) / (_deltaT);
    _state.nijTDAT = _northing - (f * _deltaN);  // add to log
    _state.eijTDAT = _easting -  (f * _deltaE);
    ZF_LOGD("MRFilter %d interpolated: %.2f  %.2f",
        _id, _state.nijTDAT, _state.eijTDAT);
    _log->setMRFilterMeasPosition(_state.nijTDAT, _state.eijTDAT);

    // step 1
    double rangeSigma = _lastMRData.range * _lastMRData.rangeSigma;
    arma::mat r1z = { { pow(rangeSigma, 2), 0 },
                      { 0, pow(_lastMRData.bearingSigma, 2) }
                    };
    // step 2
    arma::mat r1x = { { _lastMRData.njCovar, 0 },
                      { 0 , _lastMRData.ejCovar}
                    };
    // step 3
    double bearing = WrapTwoPI(_lastMRData.bearing);
    hi = { { cos(bearing), sin(bearing) },
           { (0.0 - ((_state.eijTDAT-_lastMRData.ej)/pow(_lastMRData.range,2))),
             (      ((_state.nijTDAT-_lastMRData.nj)/pow(_lastMRData.range,2))),
           }
         };
    // step 4
    hj = 0 - hi;
    // step 5
    pj = r1x;
    // step 6
    rij = r1z;
    // step 7
    arma::mat z = { _lastMRData.range, bearing};
    // step 8
    arma::mat zhat = {
        sqrt( pow(_state.nijTDAT-_lastMRData.nj,2) +
              pow(_state.eijTDAT-_lastMRData.ej,2) ),
        WrapTwoPI( atan2(_state.eijTDAT-_lastMRData.ej,
                         _state.nijTDAT-_lastMRData.nj) )
    };
    // step 9
    deltaz(0,0) = (z(0) - zhat(0));
    // In Matlab delta = angdiff(alpha,beta) calculates the difference between
    // the angles alpha and beta. This function subtracts alpha from beta with
    // the result wrapped on the interval [-pi,pi].
    double angdiff = zhat(1) - z(1);
    deltaz(1,0) = 0 - WrapPosNegPI(angdiff);
}

// Computation modeled after the Filterfun function in Rock's scripts
//void MRFilter::meas_update_MRFilter(int kf)
void MRFilter::meas_update_MRFilter(int kf, arma::mat& P, const arma::mat deltaz,
                          const arma::mat pij, const arma::mat rij,
                          const arma::mat hi, const arma::mat hj,
                          const arma::mat pj)
{
    if (1 == kf) {
        kf_filter_fun(P, deltaz, pij, rij, hi, hj, pj);
    } else if (0 == kf) {
        ci_filter_fun(P, deltaz, pij, rij, hi, hj, pj);
    } else {
        ZF_LOGE("MRFilter %d unknown kf = %d:", _id, kf);
    }
    ZF_LOGD("MRFilter %d N: %.2f  E: %.2f  diff: %.2f %.2f",
            _id, _northing, _easting,
            _lastNav.navN - _northing, _lastNav.navE - _easting);
}

double MRFilter::wopt(const arma::mat& pij, const arma::mat& hi,
                      const arma::mat& hj, const arma::mat& r1x,
                      const arma::mat& rij)
{
    /*
    function [ wout ] = wopt( Pij,Hi,Hj,R1x,Rij )
    val=1000000;
    Pj=R1x;
    for j=1:1:99;
        w=.01*j;
        P1=(1/w)*Pij;
        P2=(1/(1-w))*Hj*R1x*Hj'+Rij; // was P2=(1/(1-w))*Hj*Pj*Hj'+Rij;
        K=P1*Hi'*inv(Hi*P1*Hi'+P2);
        Pijplus=(eye(2)-K*Hi)*P1;
        valc=trace(Pijplus);
        if valc<val
            val=valc;
            wout=w;
        end
    end

    _Pij === Pij  P31 is passed into wopt as Pij via Filterfun
    Also noted the line computing p2 was wrong (_Pij was used instead of _pj)
    */
    double wout = 0.01;
    double val = 1000000.;
    arma::mat aye(2, 2, arma::fill::eye);
    for (int i = 1; i < 100; i++) {
        double w = 0.01 * i;
        arma::mat p1 = (1/w) * pij;
        arma::mat p2 = (1/(1-w)) * (hj * r1x * hj.t()) + rij;
        arma::mat a = hi * p1 * hi.t() + p2;
        arma::mat k = p1 * hi.t() * a.i();
        arma::mat pijPlus = (aye - (k * hi)) * p1;
        double valc = arma::trace(pijPlus);

        if (valc < val) {
            val = valc;
            wout = w;
        }
    }
    ZF_LOGD("MRFilter %d final wout: %.2f", _id, wout);

    _log->setMRFilterWopt(wout);
    return wout;
}

// Computation modeled after the Filterfun function in Rock's scripts
void MRFilter::ci_filter_fun(arma::mat& P, const arma::mat deltaz,
                             const arma::mat pij, const arma::mat rij,
                             const arma::mat hi, const arma::mat hj,
                             const arma::mat pj)
{
    /*
    NF = x1 = _northing;
    EF = x2 = _easting;
  1 Pij = P;              % Pij is modified but not P
    Rij = Rij;
    w= wopt( Pij,Hi,Hj,Pj,Rij );    % note Pj=R1x or R2x??

  2 P1=(1/w)*Pij;
       |--------- C ----------|
                 |---- B -----|
                 |-- A --|
  3 P2=(1/(1-w))*Hj*Pj*Hj'+Rij;    %split CI
      |---------- C ----------|
                |----- B -----|
                 |-- A --|
  4 K=P1*Hi'*inv(Hi*P1*Hi'+ P2); %Eq. 2.51,52 for R1x inclusion
  5 Pij=(eye(2)-K*Hi)*P1;

  6 deltax=K*deltaz;
    x1=x1+deltax(1);
    x2=x2+deltax(2);
    NF=x1;
    EF=x2;
    */

    // step 1
    // Pij = P;
    // Rij = Rij;
    double w = wopt(pij, hi, hj, pj, rij);
    // step 2
    arma::mat p1 = (1/w) * pij;
    // step 3
    arma::mat a = hj * pj * hj.t();
    arma::mat b(2, 2, arma::fill::eye);
    // arma::mat b = a + rij;
    // arma::mat p2 = (1/(1-w)) * b;
    arma::mat p2 = (1/(1-w)) * a + rij;
    // step 4
    a = hi * p1 * hi.t();
    b = a + p2;
    arma::mat k = p1 * hi.t() * b.i();
    // step 5
    arma::mat aye(2, 2, arma::fill::eye);
    P = (aye - k * hi) * p1;
    arma::mat deltax = k * deltaz;
    // step 6
    // x1=x1+deltax(1)
    // x2=x2+deltax(2)
    _northing += deltax(0);
    _easting += deltax(1);
}

// Computation modeled after the Filterfun function in Rock's scripts
void MRFilter::kf_filter_fun(arma::mat& P, const arma::mat deltaz,
                             const arma::mat pij, const arma::mat rij,
                             const arma::mat hi, const arma::mat hj,
                             const arma::mat pj)
{
    /*
    NF = x1 = _northing;
    EF = x2 = _easting;
    Pij = P;              % P is modified
    Rij = Rij;
                  |----------- C ---------|
                  |--- A ---|     |-- B --|
step 1 K=Pij*Hi'*inv(Hi*Pij*Hi'+Rij +Hj*Pj*Hj'); %Eq. 2.51,52 for R1x inclusion
step 2 Pij=(eye(2)-K*Hi)*Pij;
step 3 deltax=K*deltaz;
      x1=x1+deltax(1);
      x2=x2+deltax(2);
      NF=x1;
      EF=x2;

    https://www.mathworks.com/matlabcentral/answers/
    96960-does-matlab-pass-parameters-using-call-by-value-or-call-by-reference
    */
    // step 1
    // Hi * Pij * Hi'
    arma::mat a = (hi * pij * hi.t());
    // Hj * Pj * Hj'
    arma::mat b = (hj * pj * hj.t());
    // (Hi * Pij * Hi') + Rij + (Hj * Pj * Hj')
    arma::mat c = a + rij + b;
    // K = Pij * Hi' * inv(Hi*Pij*Hi'+ Rij + Hj*Pj*Hj')
    arma::mat k = pij * hi.t() * c.i();
    // step 2
    arma::mat aye(2, 2, arma::fill::eye);
    P = (aye - k * hi) * pij;
    // step 3
    // deltax=K*deltaz
    arma::mat deltax = k * deltaz;
    // x1=x1+deltax(1)
    // x2=x2+deltax(2)
    _northing += deltax(0);
    _easting += deltax(1);

    ZF_LOGD("MRFilter %d N: %.2f  E: %.2f  diff: %.2f %.2f",
            _id, _northing, _easting,
            _lastNav.navN - _northing, _lastNav.navE - _easting);
}

#define DRIFT_RATE  0.03   // nominal process noise (from CoNavDemo.m)

// Process position and attitude data from the Ego vehicle (this vehicle)
void MRFilter::process_update(CoNav::ERNavInput& navdata, const arma::mat& pBest)
{
    ZF_LOGD("MRFilter %d processing data @ %.2f -> %.2f | %.2f | %.2f",
            _id, navdata.egoTime, navdata.navN, navdata.navE, navdata.navZ);

    _log->setMRFilterMotion(navdata);

    // Process the motion update
    _time   = navdata.egoTime;
    _deltaN = navdata.navN - _lastNav.navN;
    _deltaE = navdata.navE - _lastNav.navE;
    _deltaT = navdata.egoTime - _lastNav.egoTime;
    _northing += _deltaN;
    _easting += _deltaE;

    _state.nij    = _northing;
    _state.eij    = _easting;
    _state.deltaN = _deltaN;
    _state.deltaE = _deltaE;
    _state.distance = sqrt(pow(_deltaN, 2) + pow(_deltaE, 2));
    _state.egoClock = _time;

    ZF_LOGD("MRFilter %d deltaN:%.2f deltaE:%.2f dist:%.2f",
            _id, _state.deltaN, _state.deltaE, _state.distance);

    // Modify the process noise to deal with correlation
    arma::mat qbar(2, 2, arma::fill::eye);
    arma::mat q(2, 2, arma::fill::eye);
    q *= pow(_state.distance*DRIFT_RATE, 2);
    calculate_qbar(qbar, q, _Pij, pBest);
    // process update of state covariance
    // P31=P31+Q1bar;   %Process update of the state covariance
    _Pij += qbar;

    // Log data
    _lastNav = navdata;
    _origin = ORIGIN_MOTION;
    _log->setMRFilterState(_state);
    _log->setMRFilterP(_origin, _Pij);
    _log->setMRFilterPbest(pBest);
    _log->setMRFilterQbar(qbar);
    _log->write();
}

void MRFilter::calculate_qbar(arma::mat& Qbar, const arma::mat& q,
                              const arma::mat& p, const arma::mat& pBest)
{
    /* Computation modeled after Steve Rock's Matlab script
    1  Q=(Qsig*distanceTravelled)^2 * eye(2);   %Process noise covariance of a normal KF

    function [Q1bar Q2bar]= CalculateQbar(Q, P31, P32, Pbest)

         PbestI=inv(Pbest);
      2  Q1bar518=Q+Q*PbestI*P31+P31*PbestI*Q+Q*PbestI*Q+Q*PbestI*P31*PbestI*Q;
         Q2bar518=Q+Q*PbestI*P32+P32*PbestI*Q+Q*PbestI*Q+Q*PbestI*P32*PbestI*Q;
         Q1bar520=Q+Q*PbestI*P31+P31*PbestI*Q;
         Q2bar520=Q+Q*PbestI*P32+P32*PbestI*Q;
      3  Q1bar=Q+Q1bar518;
         Q2bar=Q+Q2bar518;
    end

    */
    // step 1
    // normal KF process noise
    // step 2
    arma::mat pBestI = pBest.i();
    arma::mat qb(2, 2, arma::fill::zeros);
    // Qbar=Q+(Q*PbestI*P) + (P*PbestI*Q) + (Q*PbestI*Q) + (Q*PbestI*P*PbestI*Q)
    qb = q + (q * pBestI * p) +             // Q + (Q * PbestI * P) +
             (p * pBestI * q) +             //     (P * PbestI * Q) +
             (q * pBestI * q) +             //     (Q * PbestI * Q)   +
             (q * pBestI * p * pBestI * q); //     (Q * PbestI * P * PbestI * Q)
    // step 3
    Qbar = q + qb;
}
