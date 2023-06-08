/*****************************************************************************
 * Copyright (c) 2002-2021 MBARI
 * Monterey Bay Aquarium Research Institute, all rights reserved.
 *****************************************************************************
 * @file    lrconav_app.cpp
 * @authors r. henthorn
 * @date    11/10/2021
 * @brief   Main program of Terrain-Relative Co-Navigation interfacing with LCM.
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

/***********************************************************************
 * Headers
 ***********************************************************************/

#include "LcmMessageReader.h"
#include "LcmMessageWriter.h"
#include "MathP.h"
#include "NavUtils.h"
#include "TethysLcmTypes/LrauvLcmMessage.hpp"
#include <lcm/lcm-cpp.hpp>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>

// Status logging
#define ZF_LOG_TAG "lrconav-app:"
#include <zf_log.h>

#include "MathP.h"
#include "DataLogWriter.h"
#include "NavUtils.h"

#include "lrconav_app.h"
#include "EgoRobot.h"

/***********************************************************************
 * Macros
 ***********************************************************************/

/***********************************************************************
 * Code
 ***********************************************************************/

class CoNavLcmHandler;
using namespace std;
using namespace TethysLcmTypes;
using namespace lrauv_lcm_tools;

void usage()
{
    fprintf(stderr, "Usage:\n");
}

int64_t getTimeMillisec() {
    struct timeval spec;
    gettimeofday(&spec, NULL);

    double ms         = (spec.tv_sec * 1000.) + (spec.tv_usec / 1000.);
    int64_t current_ms = ms;
    return current_ms;
}

// Function prototypes/declarations
// Initialize the LCM context: subscribe to channels, set up channel handlers
bool init_lcm();
// Initialize the CoNav channel writer
bool init_conav_writer();
// Handle incoming LCM messages
int handle_lcm();
// Determine if the motion update period has elapsed
bool motion_period_elapsed();
// Update vehicle position
int process_update(CoNav::ERNavInput& navData);
// Update Cooperative Nav
int measure_update(CoNav::MRDATInput& coNavData);
// Publish the current Co-navigated position and variances
void publish_state();

// LCM and handler objects
static lcm::LCM *_lcm;
static CoNavLcmHandler *_lcmHandler;

// lrauv-lcmtypes objects
static LcmMessageWriter<std::string> _msgWriter;
static LcmMessageReader _msgReader;

// timing and trigger variables
struct CoNavTiming
{
    double egoClock;
    double lastMotionMsg;
    double lastMotionUpdate;
    double lastCoNavMsg;
    double lastCoNavUpdate;
};
static struct CoNavTiming _timing = {0.,0.,0.,0.};

static CoNav::ERNavInput _navData = {};
static EgoRobot* _ego = NULL;
static bool _logToStdErr = true;

FILE *g_log_file;

static void file_output_callback(const zf_log_message *msg, void *arg)
{
    (void)arg;
    *msg->p = '\n';
    fwrite(msg->buf, msg->p - msg->buf + 1, 1, g_log_file);
    fflush(g_log_file);
    if (_logToStdErr) {
        fwrite(msg->buf, msg->p - msg->buf + 1, 1, stderr);
        fflush(stderr);
    }
}

static void finalize(void)
{
    ZF_LOGI("Finalize");
    DELOBJ(_ego);
//    DELOBJ(_lcmHandler);
    DELOBJ(_lcm);
    fclose(g_log_file);
}

static void file_output_open(const char *const log_path)
{
    g_log_file = fopen(log_path, "a");
    if (!g_log_file) {
        ZF_LOGW("Failed to open log file %s", log_path);
        return;
    }
    atexit(finalize);
    zf_log_set_output_v(ZF_LOG_PUT_STD, 0, file_output_callback);
}

int main(int argc, char **argv)
{
    //**************************************************************************
    // Initialization phase
    //**************************************************************************

    // Set up the logging folders in TRN log directory if defined
    std::string homedir = getenv(TRNLogDirName)? getenv(TRNLogDirName) : ".";
    // Create the latest link in the log directory
    std::string latest = LatestLogDirName;
    std::string newdir = "";
    if (DataLog::createJulianDayLogDir(newdir, homedir, latest)) {
        ZF_LOGI("Created log folder %s", newdir.c_str());
    }

    // Create syslog output file and redirect
    newdir += "/lrconav_syslog";
    file_output_open(newdir.c_str());

    zf_log_set_output_level(ZF_LOG_VERBOSE);

    // Initialize LCM context (create, subscribe, etc)
    if (!init_lcm()) {
        ZF_LOGE("LCM context failure");
        return -1;
    }

    // Create the CoNav object
    _ego = new EgoRobot();

    // main loop
    ZF_LOGI("Entering main loop");
    do {
        handle_lcm();
    } while (true);

    return 0;
}

// Create handler class
class CoNavLcmHandler
{
public:
    CoNavLcmHandler() {}
    ~CoNavLcmHandler() {}

    // When a Ahrs message is received copy data into vehicle state object
    void handleAhrs(const lcm::ReceiveBuffer *rbuf,
                   const std::string &chan,
                   const LrauvLcmMessage *msg) {
    }

    // When a Nav message is received copy data into vehicle state object
    void handleNav(const lcm::ReceiveBuffer *rbuf,
                   const std::string &chan,
                   const LrauvLcmMessage *msg) {

        //printf("handleNav: msg time = %ld\n", _timing.lastMotionMsg);
        _msgReader.setMsg(msg);
        _timing.egoClock = double(msg->epochMillisec)/1000;
        _timing.lastMotionMsg = _timing.egoClock;
        ZF_LOGD("msg time = %.3f", _timing.egoClock);

        const DoubleArray *da = _msgReader.getDoubleArray(NAV_LAT);
        if (NULL == da) {
            ZF_LOGE("failed to read %s", NAV_LAT);
            return;
        }
        double lat = da->data[0];

        if (NULL == da) {
            ZF_LOGE("failed to read %s", NAV_LON);
            return;
        }
        da = _msgReader.getDoubleArray(NAV_LON);
        double lon = da->data[0];

        // Convert to UTM for use in CoNav
        NavUtils::geoToUtm(Math::degToRad(lat), Math::degToRad(lon),
            NavUtils::geoToUtmZone(Math::degToRad(lat), Math::degToRad(lon)),
            &_navData.navN, &_navData.navE);

        // Update CoNav with the latest motion dataset
        _navData.egoTime = _timing.egoClock;
        process_update(_navData);
    }

    // When a Dvl message is received copy data into vehicle state object
    void handleDvl(const lcm::ReceiveBuffer *rbuf,
                   const std::string &chan,
                   const LrauvLcmMessage *msg) {

        _msgReader.setMsg(msg);
        _timing.egoClock = double(msg->epochMillisec)/1000;
        _timing.lastMotionMsg = _timing.egoClock;
        ZF_LOGD("msg time = %.3f", _timing.egoClock);
    }

    // When a Depth message is received copy data into vehicle state object
    void handleDepth(const lcm::ReceiveBuffer *rbuf,
                     const std::string &chan,
                     const LrauvLcmMessage *msg) {


        _msgReader.setMsg(msg);
        _timing.egoClock = double(msg->epochMillisec)/1000;
        _timing.lastMotionMsg = _timing.egoClock;
        ZF_LOGD("msg time = %.3f", _timing.egoClock);

        const DoubleArray *da = _msgReader.getDoubleArray(DEPTH_DEPTH);
        if (NULL == da) {
            ZF_LOGD("failed to read double %s", DEPTH_DEPTH);

            const FloatArray *fa = _msgReader.getFloatArray(DEPTH_DEPTH);
            if (NULL == fa) {
                ZF_LOGD("failed to read float %s", DEPTH_DEPTH);
                return;
            } else {
                _navData.navZ = (float)fa->data[0];
            }
        } else {
            _navData.navZ = da->data[0];
        }

        // Update CoNav with the latest motion dataset
        _navData.egoTime = _timing.lastMotionMsg;
        process_update(_navData);
    }

    // When a CoNav message is received extract info and call EgoRobot object
    void handleCoNav(const lcm::ReceiveBuffer *rbuf,
                     const std::string &chan,
                     const LrauvLcmMessage *msg) {

        _msgReader.setMsg(msg);
        _timing.egoClock = double(msg->epochMillisec)/1000;
        _timing.lastMotionMsg = _timing.egoClock;
        ZF_LOGD("msg time = %.3f", _timing.egoClock);

        CoNav::MRDATInput mrInput = {};
        // Update CoNav with this CoNav message data
        const DoubleArray *fa = _msgReader.getDoubleArray(MR_TIME_NAME);
        if (NULL == fa) {
            ZF_LOGE("failed to read %s", MR_TIME_NAME);
            return;
        }
        mrInput.datTime = double(fa->data[0]);
        ZF_LOGD("read %s as %.2f", MR_TIME_NAME, fa->data[0]);

        const IntArray *ia = _msgReader.getIntArray(MR_VEHID_NAME);
        if (NULL == ia) {
            ZF_LOGE("failed to read %s", MR_VEHID_NAME);
            return;
        }
        mrInput.vehId = ia->data[0];

        fa = _msgReader.getDoubleArray(MR_TRN_N_NAME);
        if (NULL == fa) {
            ZF_LOGE("failed to read %s", MR_TRN_N_NAME);
            return;
        }
        mrInput.nj = fa->data[0];

        fa = _msgReader.getDoubleArray(MR_TRN_E_NAME);
        if (NULL == fa) {
            ZF_LOGE("failed to read %s", MR_TRN_E_NAME);
            return;
        }
        mrInput.ej = fa->data[0];

        fa = _msgReader.getDoubleArray(MR_TRN_Z_NAME);
        if (NULL == fa) {
            ZF_LOGE("failed to read %s", MR_TRN_Z_NAME);
            return;
        }
        mrInput.dj = fa->data[0];

        fa = _msgReader.getDoubleArray(MR_TRN_VAR_N_NAME);
        if (NULL == fa) {
            ZF_LOGE("failed to read %s", MR_TRN_VAR_N_NAME);
            return;
        }
        mrInput.njCovar = fa->data[0];

        fa = _msgReader.getDoubleArray(MR_TRN_VAR_E_NAME);
        if (NULL == fa) {
            ZF_LOGE("failed to read %s", MR_TRN_VAR_E_NAME);
            return;
        }
        mrInput.ejCovar = fa->data[0];

        fa = _msgReader.getDoubleArray(MR_TRN_VAR_Z_NAME);
        if (NULL == fa) {
            ZF_LOGE("failed to read %s", MR_TRN_VAR_Z_NAME);
            return;
        }
        mrInput.djCovar = fa->data[0];

        fa = _msgReader.getDoubleArray(MR_RANGE_NAME);
        if (NULL == fa) {
            ZF_LOGE("failed to read %s", MR_RANGE_NAME);
            return;
        }
        mrInput.range = fa->data[0];

        fa = _msgReader.getDoubleArray(MR_BEARING_NAME);
        if (NULL == fa) {
            ZF_LOGE("failed to read %s", MR_BEARING_NAME);
            return;
        }
        mrInput.bearing = fa->data[0];

        fa = _msgReader.getDoubleArray(MR_RANGE_VAR_NAME);
        if (NULL == fa) {
            ZF_LOGE("failed to read %s", MR_RANGE_VAR_NAME);
            return;
        }
        mrInput.rangeSigma = fa->data[0];

        fa = _msgReader.getDoubleArray(MR_BEARING_VAR_NAME);
        if (NULL == fa) {
            ZF_LOGE("failed to read %s", MR_BEARING_VAR_NAME);
            return;
        }
        mrInput.bearingSigma = fa->data[0];

        mrInput.egoTime = _timing.egoClock;
        measure_update(mrInput);
    }

};

// Initialize the LCM context: subscribe to channels, set up channel handlers
bool init_lcm()
{
    // Create the LCM context
    _lcm = new lcm::LCM();
    if (!_lcm->good()) {
        return false;
    }

    // Set up the channel handlers
    _lcmHandler = new CoNavLcmHandler();
    _lcm->subscribe(AHRS_CHANNEL,  &CoNavLcmHandler::handleAhrs,  _lcmHandler);
    _lcm->subscribe(NAV_CHANNEL,   &CoNavLcmHandler::handleNav,   _lcmHandler);
    _lcm->subscribe(DVL_CHANNEL,   &CoNavLcmHandler::handleDvl,   _lcmHandler);
    _lcm->subscribe(DEPTH_CHANNEL, &CoNavLcmHandler::handleDepth, _lcmHandler);
    _lcm->subscribe(MR_DAT_CHANNEL, &CoNavLcmHandler::handleCoNav, _lcmHandler);

    // Set up the conav channel writer
    if (!init_conav_writer()) {
        return false;
    }

    return true;
}

// Initialize the CoNavEgo channel writer
bool init_conav_writer()
{
    Dim sdim(0, 0);
    if (!_msgWriter.addArray(Int, "EgoVehId", "EgoVehId", "", sdim)) {
        return false;
    }
    if (!_msgWriter.addArray(Double, "EgoN", "EgoN", "", sdim)) {
        return false;
    }
    if (!_msgWriter.addArray(Double, "EgoE", "EgoE", "", sdim)) {
        return false;
    }
    if (!_msgWriter.addArray(Double, "EgoZ", "EgoZ", "", sdim)) {
        return false;
    }
    if (!_msgWriter.addArray(Double, "EgoVarN", "EgoVarN", "", sdim)) {
        return false;
    }
    if (!_msgWriter.addArray(Double, "EgoVarE", "EgoVarE", "", sdim)) {
        return false;
    }
    if (!_msgWriter.addArray(Double, "EgoVarZ", "EgoVarZ", "", sdim)) {
        return false;
    }

    return true;
}

// Handle incoming LCM messages
int handle_lcm()
{
    // Handle the bursty nature of the message flow
    int nm = _lcm->handleTimeout(1000);
    if (nm > 0) {
        ZF_LOGD("timing: MotionM %.3f \tMotionU %.3f\tMeasM %.3f\tMeasU %.3f",
                _timing.lastMotionMsg, _timing.lastMotionUpdate,
                _timing.lastCoNavMsg, _timing.lastCoNavUpdate);
    }
    //printf("%d messages handled\n", nm);
    return nm;
}

// Update vehicle position
int process_update(CoNav::ERNavInput& nav)
{
    // Call _ego->process_update() when it is "time" for another position update
    if (!motion_period_elapsed()) {
        return -1;
    }

    // Update EgoRobot with latest motion dataset and publish updated state
    _ego->process_update(nav);
    _timing.lastMotionUpdate = _timing.egoClock;
    ZF_LOGD("motion update time = %.3f", _timing.lastMotionUpdate);
    publish_state();

    return 0;
}

// Update Cooperative Nav
int measure_update(CoNav::MRDATInput& coNavData)
{
    // Call _ego->measure_update() and publish updated state
    _ego->measure_update(coNavData);
    _timing.lastCoNavUpdate = _timing.egoClock;
    ZF_LOGD("measure update time = %.3f", _timing.lastCoNavUpdate);
    publish_state();

    return 0;
}

// Publish the current Co-navigated position and variances
void publish_state()
{
    // Get the updated Co-Navigated vehicle position and publish
    // _ego->get_state();
    // _msgWriter.set("VehId", id);
    _msgWriter.publish(*_lcm, CONAV_CHANNEL, getTimeMillisec());
}

// Check if the update period has elapsed
// Return true if period has elapsed, otherwise false
bool motion_period_elapsed()
{
    if (_timing.lastMotionMsg >= (_timing.lastMotionUpdate + MOTION_PERIOD)) {
        return true;
    } else {
        return false;
    }
}

