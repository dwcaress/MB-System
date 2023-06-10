/*****************************************************************************
 * Copyright (c) 2002-2021 MBARI
 * Monterey Bay Aquarium Research Institute, all rights reserved.
 *****************************************************************************
 * @file    LcmTrn.cpp
 * @authors r. henthorn
 * @date    03/04/2021
 * @brief   TRN implementation interfacing with LCM. For LRAUV.
 *
 * Project: Precision Control
 * Summary: A Terrain-Relative Navigation implementation that uses LCM for
            external comms. After initialization an object of this class
            can listen on the configured LCM channels for vehicle position
            data, beam data, and commands (e.g., reinit, change map, etc.)
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

#include <lcm/lcm-cpp.hpp>
#include <sys/time.h>
#include <unistd.h>

#include "gitversion.h"
#include "LcmTrn.h"
#include "MathP.h"
#include "NavUtils.h"

/***********************************************************************
 * Macros
 ***********************************************************************/
// clang-format off
#define TRN_TIMES_EQUIVALENT_SEC 0.10 // When pose and meas times are within
                                      // this threshold consider them equivalent
#define N_DVL_BEAMS     4
#define N_DIM           3 // number of dimensions in output estimates (x,y,z)
#define N_COVARS        4 // number of dimensions in output covars (x,y,z,psi)
#define N_INT_VECTORS   2 // reinits, filter
#define N_FLOAT_VECTORS 3 // mle, mmse, var
#define REINIT_VECTOR   0 // REINIT is first vector in DataVectors _trnstate
#define FILTER_VECTOR   1
#define MLE_VECTOR      0
#define MMSE_VECTOR     1
#define VAR_VECTOR      2
#define SCALAR          0 // Indices into output vectors
#define POSE_X          0
#define POSE_Y          1
#define POSE_Z          2
#define POSE_PSI        3
#define COVAR_X         0
#define COVAR_Y         2
#define COVAR_Z         5
#define COVAR_PSI       44
#define TRN_MLE_EST     1
#define TRN_MMSE_EST    2
// clang-format on

static const int tl_both = TL_OMASK(TL_TRN_SERVER, TL_BOTH);
static const int tl_log  = TL_OMASK(TL_TRN_SERVER, TL_LOG);

static const char *STR_LCM_TIMEOUT    = "lcm.timeout_sec";
static const char *STR_LCM_INITIAL_TO = "lcm.initial_timeout_msec";
static const char *STR_LCM_MAX_TO     = "lcm.max_timeout_msec";
static const char *STR_LCM_TRNNAME    = "lcm.trn_channel";
static const char *STR_LCM_CMDNAME    = "lcm.cmd_channel";
static const char *STR_LCM_AHRSNAME   = "lcm.ahrs_channel";
static const char *STR_LCM_MEASNAME   = "lcm.dvl_channel";
static const char *STR_LCM_NAVNAME    = "lcm.nav_channel";
static const char *STR_TRN_ZONE       = "trn.utm_zone";
static const char *STR_TRN_PERIOD     = "trn.period_sec";
static const char *STR_TRN_COHERENCE  = "trn.temporal_coherence_sec";
static const char *STR_TRN_INSTTYPE   = "trn.inst_type";
static const char *STR_TRN_NUMBEAMS   = "trn.num_beams";
static const char *STR_TRN_MAPTYPE    = "trn.map_type";
static const char *STR_TRN_MAPNAME    = "trn.map_name";
static const char *STR_TRN_CFGNAME    = "trn.cfg_name";
static const char *STR_TRN_PARTNAME   = "trn.part_name";
static const char *STR_TRN_LOGNAME    = "trn.log_name";
static const char *STR_TRN_FILTER     = "trn.filter_type";
static const char *STR_TRN_WEIGHTING  = "trn.modified_weighting";
static const char *STR_TRN_LOWGRADE   = "trn.force_lowgrade_filter";
static const char *STR_TRN_REINITS    = "trn.allow_filter_reinit";

static const int LCM_HANDLETIMEOUT = 50; // handleTimeout(50 ms)

using namespace lcmTrn;

/***********************************************************************
 * Code
 ***********************************************************************/

// Ctor. All initialization info resides in a libconfig configuration file.
//
LcmTrn::LcmTrn(const char *configfilepath)
    : _configfile(NULL), _cfg(NULL), _lcm(NULL), _tnav(NULL),
      _lastAHRSTimestamp(-1.0), _lastDvlTimestamp(-1.0),
      _lastNavTimestamp(-1.0), _lastDepthTimestamp(-1.0),
      _lastCmdTimestamp(-1.0), _lastUpdateTimestamp(-1.0), _good(false)
{
    logs(tl_both, "LcmTrn::LcmTrn() version %s built %s %s - config file %s\n",
        __GIT_VERSION__, __DATE__, __TIME__, configfilepath);

    // Setup default config
    //
    _trnc.utm_zone    = LCMTRN_DEFAULT_ZONE;
    _trnc.period      = LCMTRN_DEFAULT_PERIOD;
    _trnc.coherence   = LCMTRN_DEFAULT_COHERENCE;
    _trnc.maptype     = TRN_MAP_OCTREE;
    _trnc.filtertype  = LCMTRN_DEFAULT_FILTER;
    _trnc.lowgrade    = LCMTRN_DEFAULT_LOWGRADE;
    _trnc.allowreinit = LCMTRN_DEFAULT_ALLOW;
    _trnc.weighting   = LCMTRN_DEFAULT_WEIGHTING;
    _trnc.instrument  = LCMTRN_DEFAULT_INSTRUMENT;

    // Required config settings
    //
    _trnc.mapn = _trnc.cfgn = _trnc.partn = _trnc.logd = NULL;
    _lcmc.ahrs = _lcmc.heading = _lcmc.pitch = _lcmc.roll = NULL;
    _lcmc.dvl = _lcmc.xvel = _lcmc.yvel = _lcmc.zvel = _lcmc.beam1 = NULL;
    _lcmc.beam2 = _lcmc.beam3 = _lcmc.beam4 = NULL;
    _lcmc.nav = _lcmc.lat = _lcmc.lon = _lcmc.depth = _lcmc.trn = NULL;
    _lcmc.cmd = _lcmc.reinit = _lcmc.estimate = _lcmc.updatetime = NULL;
    _lcmc.initial_timeout_msec = _lcmc.max_timeout_msec = 0;

    // Set config filename
    //
    if (NULL != configfilepath) {
        _configfile = strdup(configfilepath);
    } else {
        _configfile = LCMTRN_DEFAULT_CONFIG;
    }

    // All my initialization info is in the config file
    //
    init();
}

LcmTrn::~LcmTrn()
{
    DELOBJ(_configfile);
    DELOBJ(_cfg);
    //  DELOBJ(_msg_reader);
    //  DELOBJ(_msg_writer);
    cleanTrn();
    cleanLcm();
}

void LcmTrn::cleanTrn()
{
    DELOBJ(_tnav);
    _lastUpdateTimestamp = -1.0;
}

void LcmTrn::cleanLcm() { DELOBJ(_lcm); }

// Initialize using the configuration in the config file
//
void LcmTrn::init()
{
    logs(tl_both, "LcmTrn::init() - using configuration file %s\n",
         _configfile);

    loadConfig();

    if (!good()) {
        logs(tl_both, "LcmTrn::init() - Configuration failed using %s!\n",
             _configfile);
        return;
    }

    //_msg_reader = new LcmMessageReader();
    //_msg_writer = new LcmMessageWriter<std::string>();

    measT *mt = new measT(_trnc.nbeams, _trnc.instrument);
    _thisMeas = *mt;
    _lastMeas = _thisMeas;

    // Initialize the TRN part (instantiate the TerrainNav object)
    //
    initTrn();

    // Initialize the LCM structure used for publishing the TRN state
    //
    initTrnState();

    // Initialize the LCM part
    //
    initLcm();
}

// Initialize a DataVectors object representing the TRN state.
//
void LcmTrn::initTrnState()
{
    Dim sdim(0, 0);
    // scalar int value num reinits
    if (!_msg_writer.addArray(Int, _lcmc.reinits, _lcmc.reinits, "", sdim)) {
        logs(tl_both, "LcmTrn::initTrnState() - failed to add reinits");
    }

    // scalar int value filter state
    if (!_msg_writer.addArray(Int, _lcmc.filter, _lcmc.filter, "", sdim)) {
        logs(tl_both, "LcmTrn::initTrnState() - failed to add filter");
    }

    // scalar float value updatetime
    if (!_msg_writer.addArray(Float, _lcmc.updatetime, _lcmc.updatetime, "",
                              sdim)) {
        logs(tl_both, "LcmTrn::initTrnState() - failed to add updatetime");
    }

    // MLE x,y,z offsets
    Dim mdim(1, 3); // 1-dimension, size 3
    if (!_msg_writer.addArray(Float, _lcmc.mle, _lcmc.mle, "meter", mdim)) {
        logs(tl_both, "LcmTrn::initTrnState() - failed to add mle");
    }

    // MMSE x,y,z offsets
    if (!_msg_writer.addArray(Float, _lcmc.mmse, _lcmc.mmse, "meter", mdim)) {
        logs(tl_both, "LcmTrn::initTrnState() - failed to add mmse");
    }

    //  Covars
    Dim cdim(1, 4); // 1-dimension, size 4
    if (!_msg_writer.addArray(Float, _lcmc.var, _lcmc.var, "meter", cdim)) {
        logs(tl_both, "LcmTrn::initTrnState() - failed to add covar");
    }
}

// Run the app. As long as the status is good continue to perform
// the read-lcm/process-msg cycle.
// Return only when the status transitions to not good.
//
void LcmTrn::run()
{
    logs(tl_both, "LcmTrn::run() %s using bursty timeouts of %d and %d msec\n",
         __GIT_VERSION__, _lcmc.initial_timeout_msec, _lcmc.max_timeout_msec);

    // As long as the status is good, process Lcm messages
    //
    while (good()) {
        cycle();
    }
    return;
}

// Perform a TRN update with the current data set.
// Returns true if an update was performed. Otherwise false.
// Use the latest state of the poseT and measT data objects to
// determine if a TRN update should be performed.
//
bool LcmTrn::updateTrn()
{
    // Return here unless the timing required for a trn update has been met
    if (!time2Update()) {
        return false;
    }

    logs(tl_both,
         "LcmTrn::updateTrn() >>>> pose time %.2f, meas time %.2f <<<<\n",
         _thisPose.time, _thisMeas.time);

    // Note latest TRN update time is the timestamp that triggered this update
    _lastUpdateTimestamp = fmax(_thisPose.time, _thisMeas.time);

    // If the timestamps are within 100 ms, call them equal for TRN to process
    if (fabs(_thisPose.time - _thisMeas.time) < TRN_TIMES_EQUIVALENT_SEC) {
        _thisPose.time = _thisMeas.time;
        logs(tl_both,
             "LcmTrn::updateTrn() equating pose %.2f and meas times %.2f",
             _thisPose.time, _thisMeas.time);
    }

    logs(tl_both,
         "LcmTrn::updateTrn() >>>> pose time %.2f, meas time %.2f <<<<\n",
         _thisPose.time, _thisMeas.time);

    // Execute motion and measure updates
    if (_thisPose.time <= _thisMeas.time) {
        logs(tl_both, "LcmTrn::updateTrn() motionUpdate first");

        _tnav->motionUpdate(&_thisPose);
        if (_thisPose.dvlValid) { // Update meas only if valid
            _tnav->measUpdate(&_thisMeas, _thisMeas.dataType);
        }
    } else {
        logs(tl_both, "LcmTrn::updateTrn() measUpdate first");

        if (_thisPose.dvlValid) { // Update meas only if valid
            _tnav->measUpdate(&_thisMeas, _thisMeas.dataType);
        }
        _tnav->motionUpdate(&_thisPose);
    }

    // Keep this data around for the next round
    _lastMeas      = _thisMeas;
    _lastPose      = _thisPose;
    _thisPose.time = _thisMeas.time = 0.;

    // Request the estimates and TRN state
    _tnav->estimatePose(&_mle, TRN_MLE_EST);
    _tnav->estimatePose(&_mmse, TRN_MMSE_EST);
    _filterstate = _tnav->getFilterState();
    _numreinits  = _tnav->getNumReinits();

    return true;
}

int LcmTrn::getLcmTimeout(unsigned int initial_msec, unsigned int max_msec)
{
    // We're tracking number of messages and the time it took to handle them
    int _nLcmMessages = 0;
    int _nLcmMillis   = 0;

    // See if there are messages to handle
    int n = _lcm->handleTimeout(initial_msec);

    // Get the start time
    struct timeval now {0, 0};
    gettimeofday(&now, nullptr);
    time_t startTimeMs = (now.tv_sec * 1000) + (now.tv_usec / 1000);
    time_t nowTimeMs   = startTimeMs;

    // Handle messages if there are any and up to the maximum
    // for (int i = 0; i < (_eliConfig._lcmMaxMsgs-1) && n > 0; i++) {
    int i = 0;
    for (i = 1; (nowTimeMs - startTimeMs) < max_msec && n > 0; i++) {
        _nLcmMessages += n;
        n = _lcm->handleTimeout(initial_msec);

        gettimeofday(&now, nullptr);
        nowTimeMs = (now.tv_sec * 1000) + (now.tv_usec / 1000);
    }

    // Get the finish time
    // gettimeofday(&now, nullptr);
    // time_t endTimeMs = (now.tv_sec * 1000) + (now.tv_usec / 1000);
    _nLcmMillis = (nowTimeMs - startTimeMs);

    if (_nLcmMessages > 0) {
        logs(tl_both, "%d msgs in %ld ms and %d handle calls", _nLcmMessages,
             _nLcmMillis, i);
    }

    return _nLcmMessages;
}

// Listen for and handle messages on my channels.
// The config value timeout_sec is taken on each call to this function
// regardless of how many message are handled, if any.
// The handlers simply read in the latest attitude, position, and
// beam data.
// Return the total number of messages handled within the timeout period.
int LcmTrn::handleMessages()
{
    // we want to track time and messages while executing the handling loop.
    struct timeval spec;
    gettimeofday(&spec, NULL);
    double startsec = (spec.tv_sec * 1.0) + (spec.tv_usec / 1000000.);
    double nowsec = 0., acc = 0.;
    double thensec = startsec;
    int nmsgs      = 0; // # messages handled in one call to handleTimeout
    int totalmsgs  = 0; // # messages handled in all handleTimeout calls
    int handled    = 0; // how many handleTimeout calls were made

    // when timeout_sec time has elapsed, return the
    // total number of messages handled.
    while ((nmsgs = _lcm->handleTimeout(LCM_HANDLETIMEOUT)) >= 0) {
        handled++;
        totalmsgs += nmsgs;

        // what time is after the last call to handleTimeout
        gettimeofday(&spec, NULL);
        nowsec = (spec.tv_sec * 1.0) + (spec.tv_usec / 1000000.);

        // Accumulate the amount of time spent actually handling messages
        if (nmsgs > 0) {
            acc += nowsec - thensec;
        }
        thensec = nowsec;

        // break from loop after timeout has expired
        if (nowsec > (startsec + _lcmc.timeout)) {
            break;
        }
    }

    // check for LCM error
    if (nmsgs < 0) {
        logs(tl_both,
             "LcmTrn::handleMessage() - lcm->handleTimeout internal error "
             "after %d msgs, lcm->good() = %d\n",
             totalmsgs, _lcm->good());
    } else if (nmsgs > 0) {
        logs(tl_both,
             "LcmTrn::handleMessages() - handling %d messages took %.2f ms and "
             "%d calls...\n",
             totalmsgs, acc, handled);
    }

    return totalmsgs;
}

// Processing is triggered by new data updates received in LCM messages.
// Execute a single LCM read/TRN update cycle and return.
void LcmTrn::cycle()
{
    // If no messages were handled there is no need to update TRN
    // if (0 == handleMessages()) return;
    if (0 ==
        getLcmTimeout(_lcmc.initial_timeout_msec, _lcmc.max_timeout_msec)) {
        return;
    }

    // Executed only once at the beginning of the message flow to
    // ensure that TRN is not updated with non-data
    if (__DBL_EPSILON__ >= _lastUpdateTimestamp) {
        _lastUpdateTimestamp = fmax(_thisPose.time, _thisMeas.time);
    }

    // If we have fresh data, publish the latest TRN state to
    // the TRN channel. updateTrn() performs motion and measure updates
    // and retrieves lastest position estimates. We just need to push those
    // out here as the updated TRN state.
    if (updateTrn()) {
        publishEstimates();
    }
}

// Use the latest MLE and MMSE estimates to populate LCM output message
// and publish on the TRN LCM channel.
void LcmTrn::publishEstimates()
{
    // set scalar values
    if (!_msg_writer.set(_lcmc.reinits, _numreinits)) {
        logs(tl_both, "LcmTrn::publishEstimates() - failed to set reinits");
    }
    if (!_msg_writer.set(_lcmc.filter, _filterstate)) {
        logs(tl_both,
             "LcmTrn::publishEstimates() - failed to set filter state");
    }
    if (!_msg_writer.set(_lcmc.updatetime, (float)getTimeMillisec())) {
        logs(tl_both, "LcmTrn::publishEstimates() - failed to set updatetime");
    }

    // set mle values
    if (!_msg_writer.set(_lcmc.mle, (float)_mle.x, POSE_X)) {
        logs(tl_both, "LcmTrn::publishEstimates() - failed to set mle x");
    }
    if (!_msg_writer.set(_lcmc.mle, (float)_mle.y, POSE_Y)) {
        logs(tl_both, "LcmTrn::publishEstimates() - failed to set mle y");
    }
    if (!_msg_writer.set(_lcmc.mle, (float)_mle.z, POSE_Z)) {
        logs(tl_both, "LcmTrn::publishEstimates() - failed to set mle z");
    }

    // set mmse values
    if (!_msg_writer.set(_lcmc.mmse, (float)_mmse.x, POSE_X)) {
        logs(tl_both, "LcmTrn::publishEstimates() - failed to set mmse x");
    }
    if (!_msg_writer.set(_lcmc.mmse, (float)_mmse.y, POSE_Y)) {
        logs(tl_both, "LcmTrn::publishEstimates() - failed to set mmse y");
    }
    if (!_msg_writer.set(_lcmc.mmse, (float)_mmse.z, POSE_Z)) {
        logs(tl_both, "LcmTrn::publishEstimates() - failed to set mmse z");
    }

    // set covar values
    if (!_msg_writer.set(_lcmc.var, (float)_mmse.covariance[COVAR_X], POSE_X)) {
        logs(tl_both, "LcmTrn::publishEstimates() - failed to set covar x");
    }
    if (!_msg_writer.set(_lcmc.var, (float)_mmse.covariance[COVAR_Y], POSE_Y)) {
        logs(tl_both, "LcmTrn::publishEstimates() - failed to set covar y");
    }
    if (!_msg_writer.set(_lcmc.var, (float)_mmse.covariance[COVAR_Z], POSE_Z)) {
        logs(tl_both, "LcmTrn::publishEstimates() - failed to set covar z");
    }
    if (!_msg_writer.set(_lcmc.var, (float)_mmse.covariance[COVAR_PSI],
                         POSE_PSI)) {
        logs(tl_both, "LcmTrn::publishEstimates() - failed to set covar psi");
    }

    // ship it
    if (!_msg_writer.publish(*_lcm, _lcmc.trn, getTimeMillisec())) {
        logs(tl_both, "LcmTrn::publishEstimates() - failed to publish message");
    }

    logs(tl_both, "LcmTrn::publishEstimates() - reinits:%d filterstate:%d",
         _numreinits, _filterstate);
    logs(tl_both, "LcmTrn::publishEstimates() - MLE  : %.2f %.2f %.2f", _mle.x,
         _mle.y, _mle.z);
    logs(tl_both, "LcmTrn::publishEstimates() - MMSE : %.2f %.2f %.2f", _mmse.x,
         _mmse.y, _mmse.z);
    logs(tl_both, "LcmTrn::publishEstimates() - COVAR: %.2f %.2f %.2f %.2f",
         _mmse.covariance[COVAR_X], _mmse.covariance[COVAR_Y],
         _mmse.covariance[COVAR_Z], _mmse.covariance[COVAR_PSI]);
}

// This is used in a motionUpdate. Read and populate the poseT object fields.
void LcmTrn::handleAhrs(const lcm::ReceiveBuffer *rbuf, const std::string &chan,
                        const LrauvLcmMessage *msg)
{
    _msg_reader.setMsg(msg);
    std::vector<const char *> names = _msg_reader.listNames();

    // for (size_t i = 0; i < names.size(); ++i)
    //     logs(tl_both, "item %zu: %s", i, names[i]);

    // The timestamp recorded in the poseT object is that associated with the
    // AHRS data, not the position data from the DVL.
    _thisPose.time     = (double)msg->epochMillisec / 1000;
    _lastAHRSTimestamp = (double)msg->epochMillisec / 1000;

    // Get heading, pitch, and roll from AHRS message
    const DoubleArray *da = NULL;
    if ((da = _msg_reader.getDoubleArray(_lcmc.heading))) {
        _thisPose.psi = da->data[SCALAR];
    } else {
        logs(tl_both, "handleAhrs() - %s item not found in ahrs msg",
             _lcmc.heading);
    }

    if ((da = _msg_reader.getDoubleArray(_lcmc.pitch))) {
        _thisPose.theta = da->data[SCALAR];
    } else {
        logs(tl_both, "handleAhrs() - %s item not found in ahrs msg",
             _lcmc.pitch);
    }

    if ((da = _msg_reader.getDoubleArray(_lcmc.roll))) {
        _thisPose.phi = da->data[SCALAR];
    } else {
        logs(tl_both, "handleAhrs() - %s item not found in ahrs msg",
             _lcmc.roll);
    }

    logs(tl_both, "%s msg: %.2f epoch sec; seqNo:%lld\n", _lcmc.ahrs,
         _thisPose.time, msg->seqNo);
    logs(tl_both, "%s msg: %.2f phi; %.2f theta; %.2f psi\n", _lcmc.ahrs,
         _thisPose.phi, _thisPose.theta, _thisPose.psi);

    return;
}

// Read Nav data for poseT motion updates.
void LcmTrn::handleNav(const lcm::ReceiveBuffer *rbuf, const std::string &chan,
                       const LrauvLcmMessage *msg)
{
    _msg_reader.setMsg(msg);
    std::vector<const char *> names = _msg_reader.listNames();
    // for (size_t i = 0; i < names.size(); ++i) {
    //     logs(tl_both, "item %d: %s", i, names[i]);
    // }

    _thisPose.time    = (double)msg->epochMillisec / 1000;
    _lastNavTimestamp = (double)msg->epochMillisec / 1000;

    // Get lat, lon, and depth from nav message, assume units are degrees.
    // Convert to radians and/or UTM if necessary using NavUtils.
    float lat_rads = 0, lon_rads = 0;
    const DoubleArray *da = NULL;

    if ((da = _msg_reader.getDoubleArray(_lcmc.lat))) {
        lat_rads = Math::degToRad(da->data[SCALAR]);
    } else {
        logs(tl_both, "handleNav() - %s item not found in nav msg", _lcmc.lat);
    }

    if ((da = _msg_reader.getDoubleArray(_lcmc.lon))) {
        lon_rads = Math::degToRad(da->data[SCALAR]);
    } else {
        logs(tl_both, "handleNav() - %s item not found in nav msg", _lcmc.lon);
    }

    // Convert to UTM for use in TNav
    NavUtils::geoToUtm(lat_rads, lon_rads,
                       NavUtils::geoToUtmZone(lat_rads, lon_rads), &_thisPose.x,
                       &_thisPose.y);
    logs(tl_both,
         "handleNav() - %s msg: %.2f epoch sec; seqNo:%lld; %.2f north; %.2f "
         "east\n",
         _lcmc.nav, _thisPose.time, msg->seqNo, _thisPose.x, _thisPose.y);

    return;
}

// This is a measUpdate. Read position and beam data and populate the poseT and
// measT object attributes. TRN updates are triggered by these measure and
// position updates.
void LcmTrn::handleDvl(const lcm::ReceiveBuffer *rbuf, const std::string &chan,
                       const LrauvLcmMessage *msg)
{
    _msg_reader.setMsg(msg);

    // The timestamp recorded in the measT object is that associated with the
    // DVL beam data.
    _thisMeas.time    = (double)msg->epochMillisec / 1000;
    _lastDvlTimestamp = (double)msg->epochMillisec / 1000;

    if (_lastMeas.time < 1.) {
        _lastMeas.time = _thisMeas.time;
    }

    _thisMeas.numMeas  = 4;
    _thisPose.dvlValid = _thisPose.bottomLock = _thisPose.gpsValid = false;
    _thisMeas.ranges[0] = _thisMeas.ranges[1] = _thisMeas.ranges[2] =
        _thisMeas.ranges[3]                   = 0.;
    _thisMeas.measStatus[0]                   = _thisMeas.measStatus[1] =
        _thisMeas.measStatus[2] = _thisMeas.measStatus[3] = false;

    // Get beam data from DVL message
    const FloatArray *fa = NULL;
    if ((fa = _msg_reader.getFloatArray(_lcmc.xvel))) {
        _thisPose.vx = fa->data[SCALAR];
    } else {
        logs(tl_both, "handleDVL() - %s item not found in dvl msg", _lcmc.xvel);
    }

    if ((fa = _msg_reader.getFloatArray(_lcmc.yvel))) {
        _thisPose.vy = fa->data[SCALAR];
    } else {
        logs(tl_both, "handleDVL() - %s item not found in dvl msg", _lcmc.yvel);
    }

    if ((fa = _msg_reader.getFloatArray(_lcmc.zvel))) {
        _thisPose.vz = fa->data[SCALAR];
    } else {
        logs(tl_both, "handleDVL() - %s item not found in dvl msg", _lcmc.zvel);
    }

    const char *keys[] = {_lcmc.beam1, _lcmc.beam2, _lcmc.beam3, _lcmc.beam4};
    for (int b = 0; b < N_DVL_BEAMS; b++) {
        if ((fa = _msg_reader.getFloatArray(keys[b]))) {
            _thisMeas.ranges[b]     = fa->data[SCALAR];
            _thisMeas.measStatus[b] = true;
        } else {
            _thisMeas.ranges[b]     = 0;
            _thisMeas.measStatus[b] = false;
            logs(tl_both, "handleDVL() - %s item not found in dvl msg",
                 keys[b]);
        }
    }

    const IntArray *ia = NULL;
    if ((ia = _msg_reader.getIntArray(_lcmc.valid))) {
        _thisPose.bottomLock = ia->data[SCALAR];
    } else {
        _thisPose.bottomLock = 0;
        logs(tl_both, "handleDVL() - %s item not found in dvl msg",
             _lcmc.valid);
    }
    _thisPose.dvlValid = (_thisPose.bottomLock != 0);

    logs(tl_both, "handleDvl() - %s msg: %.2f epoch sec; seqNo:%lld\n",
         _lcmc.dvl, _thisMeas.time, msg->seqNo);
    logs(tl_both,
         "handleDvl() - %s msg: ranges %d, %.2f , %.2f , %.2f , %.2f\n",
         _lcmc.dvl, _thisPose.dvlValid, _thisMeas.ranges[0],
         _thisMeas.ranges[1], _thisMeas.ranges[2], _thisMeas.ranges[3]);
    logs(tl_both, "handleDvl() - %s msg: velocities %.2f , %.2f , %.2f\n",
         _lcmc.dvl, _thisPose.vx, _thisPose.vy, _thisPose.vz);

    return;
}

// This is a motionUpdate. Read the depth and populate the poseT
// object attribute.
void LcmTrn::handleDepth(const lcm::ReceiveBuffer *rbuf,
                         const std::string &chan, const LrauvLcmMessage *msg)
{
    _msg_reader.setMsg(msg);

    long long sec = msg->epochMillisec;
    long long seq = msg->seqNo;

    _lastDepthTimestamp = (double)msg->epochMillisec / 1000;

    // Get depth data from message
    const FloatArray *fa = NULL;
    if ((fa = _msg_reader.getFloatArray(_lcmc.veh_depth))) {
        _thisPose.z = fa->data[0];
    } else {
        _thisPose.z = 0;
        logs(tl_both, "handleDepth() - %s item not found in dvl msg",
             _lcmc.veh_depth);
    }

    _thisPose.gpsValid = (_thisPose.z < 0.6);

    logs(tl_both,
         "handleDepth()-%s msg: %lld epoch sec; seqNo:%lld; depth %.2f\n",
         _lcmc.depth, sec, seq, _thisPose.z);

    return;
}

// Read commands and dispatch
void LcmTrn::handleCmd(const lcm::ReceiveBuffer *rbuf, const std::string &chan,
                       const LrauvLcmMessage *msg)
{
    _msg_reader.setMsg(msg);
    logs(tl_log, "Cmd msg timestamp   = %lld millisec, seqNo:%lld\n",
         (long long)msg->epochMillisec, msg->seqNo);

    const IntArray *ia = NULL;
    // Reinit TRN filters
    if ((ia = _msg_reader.getIntArray(_lcmc.reinit))) {
        bool lowinfo = (0 == ia->data[SCALAR]);
        if (_tnav)
            _tnav->reinitFilter(lowinfo);
    }

    // Publish latest estimates now
    if ((ia = _msg_reader.getIntArray(_lcmc.estimate))) {
        publishEstimates();
    }

    return;
}

// The LCM stuff would only be initialized once during a mission
//
void LcmTrn::initLcm()
{
    logs(tl_log, "LcmTrn::initLcm() - configuration file %s\n", _configfile);

    cleanLcm();

    _lcm = new lcm::LCM();
    if (_lcm->good()) {
        _lcm->subscribe(_lcmc.ahrs, &LcmTrn::handleAhrs, this);
        _lcm->subscribe(_lcmc.nav, &LcmTrn::handleNav, this);
        _lcm->subscribe(_lcmc.dvl, &LcmTrn::handleDvl, this);
        _lcm->subscribe(_lcmc.depth, &LcmTrn::handleDepth, this);
        _lcm->subscribe(_lcmc.cmd, &LcmTrn::handleCmd, this);
    }

    // _good is true if config file settings are OK and LCM is good
    _good = _good && _lcm->good();
}

// The TRN stuff could be initialized many times during a mission.
// E.g., re-init using a different map, options, particle file, etc.
//
void LcmTrn::initTrn()
{
    logs(tl_log, "LcmTrn::initTrn() version %s - configuration file %s\n",
        __GIT_VERSION__, _configfile);

    cleanTrn();

    // Construct the full pathname of the cfgs, maps, and log directory
    //
    char *mapn = constructFullName("TRN_MAPFILES", _trnc.mapn);
    logs(tl_log, "LcmTrn::initTrn() - map: %s\n", mapn);

    char *cfgn = constructFullName("TRN_DATAFILES", _trnc.cfgn);
    logs(tl_log, "LcmTrn::initTrn() - cfg: %s\n", cfgn);

    char *partn = constructFullName("TRN_DATAFILES", _trnc.partn);
    logs(tl_log, "LcmTrn::initTrn() - part: %s\n", partn);

    // Instantiate the TerrainNav object using the config settings
    //
    _tnav = new TerrainNav(mapn, cfgn, partn, _trnc.filtertype, _trnc.maptype,
                           (char *)_trnc.logd);

    TNavConfig::instance()->setIgnoreGps(1);

    // Continue with initial settings
    //
    if (_trnc.lowgrade) {
        _tnav->useLowGradeFilter();
    } else {
        _tnav->useHighGradeFilter();
    }

    _tnav->setFilterReinit(_trnc.allowreinit);
    _tnav->setModifiedWeighting(_trnc.weighting);
    _tnav->setInterpMeasAttitude(true);

    // Initialize data timestamps
    _lastPose.time = _lastMeas.time = _lastMeas.ping_number = 0;
}

// Reinitialize. A different config file may be used here.
// A NULL configfilepath results in a TRN reinit call that
// reinitializes the filters.
//
void LcmTrn::reinit(const char *configfilepath)
{
    logs(tl_log, "LcmTrn::reinit() - reinitializing TRN...\n");
    if (configfilepath) {
        DELOBJ(_configfile);
        _configfile = strdup(configfilepath);

        logs(tl_log, "LcmTrn::reinit() - New configuration file %s\n",
             _configfile);

        initTrn();
    } else {
        logs(tl_log, "LcmTrn::reinit() - calling tnav->reinitFilter(true)\n");
        if (_tnav)
            _tnav->reinitFilter(true);
    }
}

int64_t LcmTrn::getTimeMillisec()
{
    struct timeval spec;

    gettimeofday(&spec, NULL);

    double _ms         = (spec.tv_sec * 1000.) + (spec.tv_usec / 1000.);
    int64_t current_ms = _ms;

    return current_ms;
}

// Perform verification checks on the LCM config settings.
//
bool LcmTrn::verifyLcmConfig()
{
    bool isgood = true;
    // Check required settings
    //
    if (_lcmc.timeout <= 0.01) {
        logs(tl_both, "LcmTrn::verifyLcmConfig() - timeout must be > 0\n");
        isgood = false;
    }
    if (!(_lcmc.ahrs && _lcmc.heading && _lcmc.pitch && _lcmc.roll)) {
        logs(tl_both, "LcmTrn::verifyLcmConfig() - ahrs channel, heading, "
                      "pitch, and roll names are all required.\n");
        isgood = false;
    }
    if (!(_lcmc.dvl && _lcmc.xvel && _lcmc.yvel && _lcmc.zvel && _lcmc.beam1 &&
          _lcmc.beam2 && _lcmc.beam3 && _lcmc.beam4 && _lcmc.valid)) {
        logs(tl_both, "LcmTrn::verifyLcmConfig() - dvl channel and beam names "
                      "are all required.\n");
        isgood = false;
    }
    if (!(_lcmc.nav && _lcmc.lat && _lcmc.lon)) {
        logs(tl_both, "LcmTrn::verifyLcmConfig() - nav channel, lat, lon, and "
                      "depth names are all required.\n");
        isgood = false;
    }
    if (!(_lcmc.depth && _lcmc.veh_depth && _lcmc.pressure)) {
        logs(tl_both, "LcmTrn::verifyLcmConfig() - depth channel, veh_depth, "
                      "and pressure names are all required.\n");
        isgood = false;
    }
    if (!(_lcmc.trn && _lcmc.mle && _lcmc.mmse && _lcmc.var && _lcmc.reinits &&
          _lcmc.filter)) {
        logs(tl_both, "LcmTrn::verifyLcmConfig() - trn channel, mle, mmse, "
                      "var, filter, and reinits names are all required.\n");
        isgood = false;
    }
    if (!(_lcmc.cmd && _lcmc.reinit && _lcmc.estimate)) {
        logs(tl_both, "LcmTrn::verifyLcmConfig() - cmd channel, reinit, and "
                      "estimate are all required.\n");
        isgood = false;
    }
    if (!isgood) {
        logs(tl_both,
             "LcmTrn::verifyLcmConfig() - Incomplete LCM settings in %s.\n",
             _configfile);
    }
    return isgood;
}

// Perform verification checks on the TRN config settings.
//
bool LcmTrn::verifyTrnConfig()
{
    bool isgood = true;
    // Check required settings
    //
    if (!(_trnc.mapn && _trnc.cfgn && _trnc.partn && _trnc.logd)) {
        logs(tl_both, "LcmTrn::verifyTrnConfig() - map, config file, particle "
                      "file, and log dir are all required.\n");
        isgood = false;
    }
    if (_trnc.maptype != TRN_MAP_GRID && _trnc.maptype != TRN_MAP_OCTREE) {
        logs(tl_both,
             "LcmTrn::verifyTrnConfig() - Unrecognized map type specified in "
             "%s.\n",
             _configfile);
        isgood = false;
    }
    if (_trnc.instrument != TRN_INST_DVL) {
        logs(tl_both,
             "LcmTrn::verifyTrnConfig() - Unrecognized instrument specified in "
             "%s.\n",
             _configfile);
        isgood = false;
    }
    // The weighting can
    if (_trnc.weighting < TRN_WEIGHT_NONE ||
        _trnc.weighting > TRN_WEIGHT_SBNIS) {
        logs(tl_both,
             "LcmTrn::verifyTrnConfig() - Unrecognized weighting specified in "
             "%s.\n",
             _configfile);
        isgood = false;
    }
    // Only Particle filter and Point Mass filter are supported
    if (_trnc.filtertype != TRN_FILTER_PM &&
        _trnc.filtertype != TRN_FILTER_PF) {
        logs(tl_both,
             "LcmTrn::verifyTrnConfig() - Unrecognized filter type specified "
             "in %s.\n",
             _configfile);
        isgood = false;
    }

    if (!isgood) {
        logs(tl_both,
             "LcmTrn::verifyTrnConfig() - Incomplete or unsupported settings "
             "in %s.\n",
             _configfile);
    }
    return isgood;
}

// Load the configuration file values. Set _good flag accordingly.
//
void LcmTrn::loadConfig()
{
    _good = true;

    // Create my config object from my config file
    //
    if (NULL == _cfg) {
        _cfg = new Config();
    }
    _cfg->readFile(_configfile);

    // Load the TRN options next. Use defaults if not present.
    if (!_cfg->lookupValue(STR_TRN_ZONE, _trnc.utm_zone)) {
        _trnc.utm_zone = LCMTRN_DEFAULT_ZONE;
    }
    if (!_cfg->lookupValue(STR_TRN_PERIOD, _trnc.period)) {
        _trnc.period = LCMTRN_DEFAULT_PERIOD;
    }
    if (!_cfg->lookupValue(STR_TRN_COHERENCE, _trnc.coherence)) {
        _trnc.coherence = LCMTRN_DEFAULT_COHERENCE;
    }
    if (!_cfg->lookupValue(STR_TRN_FILTER, _trnc.filtertype)) {
        _trnc.filtertype = LCMTRN_DEFAULT_FILTER;
    }
    if (!_cfg->lookupValue(STR_TRN_WEIGHTING, _trnc.weighting)) {
        _trnc.weighting = LCMTRN_DEFAULT_WEIGHTING;
    }
    if (!_cfg->lookupValue(STR_TRN_LOWGRADE, _trnc.lowgrade)) {
        _trnc.lowgrade = LCMTRN_DEFAULT_LOWGRADE;
    }
    if (!_cfg->lookupValue(STR_TRN_REINITS, _trnc.allowreinit)) {
        _trnc.allowreinit = LCMTRN_DEFAULT_ALLOW;
    }
    if (!_cfg->lookupValue(STR_TRN_INSTTYPE, _trnc.instrument)) {
        _trnc.instrument = LCMTRN_DEFAULT_INSTRUMENT;
    }
    if (!_cfg->lookupValue(STR_TRN_NUMBEAMS, _trnc.nbeams)) {
        _trnc.nbeams = LCMTRN_DEFAULT_NUMBEAMS;
    }
    if (!_cfg->lookupValue(STR_TRN_MAPTYPE, _trnc.maptype)) {
        _trnc.maptype = TRN_MAP_OCTREE;
    }

    // Load the required TRN config stuff next. Flag error unless all are
    // present
    if (!_cfg->lookupValue(STR_TRN_MAPNAME, _trnc.mapn)) {
        _trnc.mapn = NULL;
    }
    if (!_cfg->lookupValue(STR_TRN_CFGNAME, _trnc.cfgn)) {
        _trnc.cfgn = NULL;
    }
    if (!_cfg->lookupValue(STR_TRN_PARTNAME, _trnc.partn)) {
        _trnc.partn = NULL;
    }
    if (!_cfg->lookupValue(STR_TRN_LOGNAME, _trnc.logd)) {
        _trnc.logd = NULL;
    }

    // Load the required LCM stuff next. Flag error unless all are present
    if (!_cfg->lookupValue(STR_LCM_TIMEOUT, _lcmc.timeout)) {
        _lcmc.timeout = LCMTRN_DEFAULT_PERIOD;
    }
    if (!_cfg->lookupValue(STR_LCM_INITIAL_TO, _lcmc.initial_timeout_msec)) {
        _lcmc.initial_timeout_msec = LCMTRN_DEFAULT_INITIAL;
    }
    if (!_cfg->lookupValue(STR_LCM_MAX_TO, _lcmc.max_timeout_msec)) {
        _lcmc.max_timeout_msec = LCMTRN_DEFAULT_MAXIMUM;
    }

    if (!_cfg->lookupValue(STR_LCM_AHRSNAME, _lcmc.ahrs)) {
        _lcmc.ahrs = NULL;
    }
    if (!_cfg->lookupValue("lcm.ahrs_heading", _lcmc.heading)) {
        _lcmc.heading = NULL;
    }
    if (!_cfg->lookupValue("lcm.ahrs_pitch", _lcmc.pitch)) {
        _lcmc.pitch = NULL;
    }
    if (!_cfg->lookupValue("lcm.ahrs_roll", _lcmc.roll)) {
        _lcmc.roll = NULL;
    }
    logs(tl_log, "ahrs config: %s, %s, %s, %s\n", _lcmc.ahrs, _lcmc.heading,
         _lcmc.pitch, _lcmc.roll);

    if (!_cfg->lookupValue(STR_LCM_MEASNAME, _lcmc.dvl)) {
        _lcmc.dvl = NULL;
    }
    if (!_cfg->lookupValue("lcm.dvl_xvel", _lcmc.xvel)) {
        _lcmc.xvel = NULL;
    }
    if (!_cfg->lookupValue("lcm.dvl_yvel", _lcmc.yvel)) {
        _lcmc.yvel = NULL;
    }
    if (!_cfg->lookupValue("lcm.dvl_zvel", _lcmc.zvel)) {
        _lcmc.zvel = NULL;
    }
    if (!_cfg->lookupValue("lcm.dvl_beam1", _lcmc.beam1)) {
        _lcmc.beam1 = NULL;
    }
    if (!_cfg->lookupValue("lcm.dvl_beam2", _lcmc.beam2)) {
        _lcmc.beam2 = NULL;
    }
    if (!_cfg->lookupValue("lcm.dvl_beam3", _lcmc.beam3)) {
        _lcmc.beam3 = NULL;
    }
    if (!_cfg->lookupValue("lcm.dvl_beam4", _lcmc.beam4)) {
        _lcmc.beam4 = NULL;
    }
    if (!_cfg->lookupValue("lcm.dvl_valid", _lcmc.valid)) {
        _lcmc.valid = NULL;
    }
    logs(tl_log, "dvl config: %s, %s, %s, %s, %s, %s\n", _lcmc.dvl, _lcmc.beam1,
         _lcmc.beam2, _lcmc.beam3, _lcmc.beam4, _lcmc.valid);

    if (!_cfg->lookupValue(STR_LCM_NAVNAME, _lcmc.nav)) {
        _lcmc.nav = NULL;
    }
    if (!_cfg->lookupValue("lcm.nav_lat", _lcmc.lat)) {
        _lcmc.lat = NULL;
    }
    if (!_cfg->lookupValue("lcm.nav_lon", _lcmc.lon)) {
        _lcmc.lon = NULL;
    }
    logs(tl_log, "nav config: %s, %s, %s\n", _lcmc.nav, _lcmc.lat, _lcmc.lon);

    if (!_cfg->lookupValue("lcm.depth_channel", _lcmc.depth)) {
        _lcmc.depth = NULL;
    }
    if (!_cfg->lookupValue("lcm.veh_depth", _lcmc.veh_depth)) {
        _lcmc.veh_depth = NULL;
    }
    if (!_cfg->lookupValue("lcm.pressure", _lcmc.pressure)) {
        _lcmc.pressure = NULL;
    }
    logs(tl_log, "depth config: %s, %s, %s\n", _lcmc.depth, _lcmc.veh_depth,
         _lcmc.pressure);

    if (!_cfg->lookupValue(STR_LCM_TRNNAME, _lcmc.trn)) {
        _lcmc.trn = NULL;
    }
    if (!_cfg->lookupValue("lcm.trn_mle", _lcmc.mle)) {
        _lcmc.mle = NULL;
    }
    if (!_cfg->lookupValue("lcm.trn_mmse", _lcmc.mmse)) {
        _lcmc.mmse = NULL;
    }
    if (!_cfg->lookupValue("lcm.trn_var", _lcmc.var)) {
        _lcmc.var = NULL;
    }
    if (!_cfg->lookupValue("lcm.trn_reinits", _lcmc.reinits)) {
        _lcmc.reinits = NULL;
    }
    if (!_cfg->lookupValue("lcm.trn_filter", _lcmc.filter)) {
        _lcmc.filter = NULL;
    }
    if (!_cfg->lookupValue("lcm.trn_updatetime", _lcmc.updatetime)) {
        _lcmc.updatetime = NULL;
    }
    logs(tl_log, "trn config: %s, %s, %s, %s, %s, %s\n", _lcmc.trn, _lcmc.mle,
         _lcmc.mmse, _lcmc.var, _lcmc.reinits, _lcmc.filter);

    if (!_cfg->lookupValue(STR_LCM_CMDNAME, _lcmc.cmd)) {
        _lcmc.cmd = NULL;
    }
    if (!_cfg->lookupValue("lcm.cmd_reinit", _lcmc.reinit)) {
        _lcmc.reinit = NULL;
    }
    if (!_cfg->lookupValue("lcm.cmd_estimate", _lcmc.estimate)) {
        _lcmc.estimate = NULL;
    }

    // Verify the configuration options
    //
    _good = verifyTrnConfig() && verifyLcmConfig();

    logs(tl_both, "LCM timeout=%.2f sec\n", _lcmc.timeout);
    logs(tl_both, "TRN settings:\n");
    logs(tl_both, "\tperiod=%.2f sec\n", _trnc.period);
    logs(tl_both, "\tcoherence=%.2f sec\n", _trnc.coherence);
    logs(tl_both, "\tmap = %s\n\tcfg = %s\n\tpart= %s\n\tlogdir= %s\n",
         _trnc.mapn, _trnc.cfgn, _trnc.partn, _trnc.logd);
    logs(tl_both, "\tmaptype = %d\n\tfiltertype = %d\n\tweighting = %d\n",
         _trnc.maptype, _trnc.filtertype, _trnc.weighting);
    logs(tl_both, "\tlowgrade_filter = %d\n\tallow reinit = %d\n",
         _trnc.lowgrade, _trnc.allowreinit);
    logs(tl_both, "\tcmd_estimate = %s\n\tcmd_reinit = %s\n", _lcmc.estimate,
         _lcmc.reinit);
}

// Return true if it is time to perform TRN updates
bool LcmTrn::time2Update()
{
    // Need data from all sources before updates
    if (_lastAHRSTimestamp < 0. || _lastDvlTimestamp < 0. ||
        _lastNavTimestamp < 0. || _lastDepthTimestamp < 0.) {
        logs(tl_both,
             "Waiting for fresh data: AHRS(%.2f), Dvl(%.2f), Nav(%.2f), "
             "Depth(%.2f)",
             _lastAHRSTimestamp, _lastDvlTimestamp, _lastNavTimestamp,
             _lastDepthTimestamp);
        return false;
    }

    // Need new version of poseT before updates
    if (_thisPose.time < 0.1) {
        return false;
    }

    // Special handling for re-running missions LCM logs
    // Reset timestamps when running replays
    if (_thisMeas.time > 1. && _thisMeas.time < _lastMeas.time) {
        _lastMeas.time = _thisMeas.time;
    }
    if (_thisPose.time > 1. && _thisPose.time < _lastPose.time) {
        _lastPose.time = _thisPose.time;
    }

    // Has the trn period expired yet?
    // If the TRN period has passed since the last TRN update, then yes.
    double now  = fmax(_thisPose.time, _thisMeas.time);
    bool period = ((_lastUpdateTimestamp + _trnc.period) <= now);
    logs(tl_log, "waiting for %.2f, time is %.2f\n",
         (_lastUpdateTimestamp + _trnc.period), now);

    // We might want to place more requirements on this in the future.
    // For example, perhaps the data should be synced to within a given
    // threshold since position data is comprised of data from multiple sources
    // and could stand for a level of cohesion.
    //
    // bool synced = (fabs(_thisPose.time - _thisMeas.time) <= _trnc.coherence);
    // logs(tl_log,"sync is %.2f, coherence is %.2f\n", fabs(_thisPose.time -
    // _thisMeas.time),
    //   _trnc.coherence);
    // No sync requirement
    bool synced = true;

    return (period && synced);
}

// Construct the full pathname of a file given:
//   env_var: the environment variable name for the base directory
//   base_name: the base name of the file.
//
// Returns a pointer to a malloc'd char* containing the full name.
// The caller is responsible for calling free() on the returned pointer.
//
char *LcmTrn::constructFullName(const char *env_var, const char *base_name)
{
    // Get the variable if it exists. If not use empty string.
    //
    const char *env = getenv(env_var);
    if (NULL == env) {
        env = "";
    }

    // malloc space for the full name and construct it.
    // size = strlen(env) + strlen(base_name) + 2 for the null char and the
    // slash
    //
    char *full_name = (char *)malloc(strlen(env) + strlen(base_name) + 2);
    sprintf(full_name, "%s/%s", env, base_name);

    return full_name;
}
