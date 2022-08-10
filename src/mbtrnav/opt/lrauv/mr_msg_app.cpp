/*****************************************************************************
 * Copyright (c) 2002-2021 MBARI
 * Monterey Bay Aquarium Research Institute, all rights reserved.
 *****************************************************************************
 * @file    mr_msg_app.cpp
 * @authors r. henthorn
 * @date    11/03/2021
 * @brief   Main program of Terrain-Relative Co-Navigation test application
 *          interfacing with LCM.
 *
 * Project: Precision Control
 * Summary: A Terrain-Relative Co-Navigation test application that uses LCM for
            message passing. Reads a CSV file of CoNav messages originating
            from cooperating vehicles. Each line is parsed and the values used
            to populate LCM messages on the CoNav channel.
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

#include "LcmMessageWriter.h"
#include "MathP.h"
#include "NavUtils.h"
#include "TethysLcmTypes/LrauvLcmMessage.hpp"
#include <lcm/lcm-cpp.hpp>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include "CoNav.h"

// Status logging
#define ZF_LOG_LEVEL ZF_LOG_INFO
#define ZF_LOG_TAG "  mr_msg-app:"
#include <zf_log.h>


/***********************************************************************
 * Macros
 ***********************************************************************/

// CSV input data is timestamped using seconds since Jan 01, 0000
// LCM data is timestamped in epoch seconds.
// We need to subtract a constant from the CSV timestamp.
// ND = 719529 days from Jan 01, 0000 to Jan 01, 1970
// NS = 719529 * 86400 = 62167305600
#define MATLAB_TO_EPOCH 62167305600L // seconds from 01-01-0000 to 01-01-1970
#define COOP_VEH_MSG_LATENCY 3       // seconds msg takes from there to here

/***********************************************************************
 * Code
 ***********************************************************************/

using namespace std;
using namespace TethysLcmTypes;
using namespace lrauv_lcm_tools;

// File-scoped objects
static int _quit = 0;
static struct MRFilterLog::CoopVehicleNavData _coopNav = {0, 0, 0, 0, 0, 0, 0, 0};

// LCM and handler objects
class CoNavLcmHandler;
static CoNavLcmHandler *_lcmHandler;
static lcm::LCM *_lcm;
static LcmMessageWriter<std::string> _msgWriter;


// CSV FILE items
static FILE* _csv = NULL;
static int64_t _lastMsgTime = 0;  // use INT64_MAX to publish immediately
static int64_t _mrLatency = COOP_VEH_MSG_LATENCY;

// Function forward declarations
void usage(const char* app_name);
int64_t getTimeMillisec();

// Initialize the LCM context: subscribe to channels, set up channel handlers
bool init_lcm();
// Initialize the CoNavEgo channel writer
bool init_writer();
// Handle incoming LCM messages
int handle_lcm();
// Read and parse next record into the _msgWriter fields
int get_next_record();
// Publish the current Co-navigated position and variances
int publish_coop_nav();

int main(int argc, char **argv)
{
    if (argc < 2) {
        usage(argv[0]);
        return -1;
    }

    zf_log_set_output_level(ZF_LOG_DEBUG);

    //**************************************************************************
    // Initialization phase
    //**************************************************************************

    // Open the CSV file containing the CoNav data
    _csv = fopen(argv[1], "r");
    if (!_csv) {
        ZF_LOGE("No message CSV file specified!");
        usage(argv[0]);
        return -1;
    }
    ZF_LOGI("CoNav message file %s found...", argv[1]);

    // Get the first record
    if (get_next_record() != 0) {
        ZF_LOGE("Failed to read/parse first record");
        return -1;
    }

    // Initialize LCM context (create, subscribe, etc)
    if (!init_lcm()) {
        ZF_LOGE("Failed to initialize LCM");
        return -1;
    }

    // main loop
    ZF_LOGI("Main loop: listen and respond to LRAUV LCM messages");
    do {
        handle_lcm();
        publish_coop_nav();
    } while (!_quit);
    ZF_LOGI("Done!");

    if (NULL != _lcm) delete _lcm;
    if (NULL != _csv) delete _csv;

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
        ZF_LOGD("msg time = %ld", msg->epochMillisec);

        _lastMsgTime = msg->epochMillisec;
    }

    // When a Nav message is received copy data into vehicle state object
    void handleNav(const lcm::ReceiveBuffer *rbuf,
                   const std::string &chan,
                   const LrauvLcmMessage *msg) {
        ZF_LOGD("msg time = %ld", msg->epochMillisec);

        _lastMsgTime = msg->epochMillisec;
    }

    // When a Dvl message is received copy data into vehicle state object
    void handleDvl(const lcm::ReceiveBuffer *rbuf,
                   const std::string &chan,
                   const LrauvLcmMessage *msg) {
        ZF_LOGD("msg time = %ld", msg->epochMillisec);

        _lastMsgTime = msg->epochMillisec;
    }

    // When a Depth message is received copy data into vehicle state object
    void handleDepth(const lcm::ReceiveBuffer *rbuf,
                     const std::string &chan,
                     const LrauvLcmMessage *msg) {
        ZF_LOGD("msg time = %ld", msg->epochMillisec);

        _lastMsgTime = msg->epochMillisec;
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

    // Set up the ego channel writer
    if (!init_writer()) {
        return false;
    }

    return true;
}

// Initialize the Cooperative Vehicle Nav channel writer
bool init_writer()
{
    Dim sdim(0, 0);
    if (!_msgWriter.addArray(Int, CONAV_VEHID_NAME, CONAV_VEHID_NAME,
                             "", sdim)) {
        ZF_LOGE("Creating %s failed", CONAV_VEHID_NAME);
        return false;
    }
    if (!_msgWriter.addArray(Double, CONAV_TIME_NAME, CONAV_TIME_NAME,
                             "seconds", sdim)) {
        ZF_LOGE("Creating %s failed", CONAV_TIME_NAME);
        return false;
    }
    if (!_msgWriter.addArray(Double, CONAV_TRN_N_NAME, CONAV_TRN_N_NAME,
                             "meters", sdim)) {
        ZF_LOGE("Creating %s failed", CONAV_TRN_N_NAME);
        return false;
    }
    if (!_msgWriter.addArray(Double, CONAV_TRN_E_NAME, CONAV_TRN_E_NAME,
                             "meters", sdim)) {
        ZF_LOGE("Creating %s failed", CONAV_TRN_E_NAME);
        return false;
    }
    if (!_msgWriter.addArray(Double, CONAV_TRN_Z_NAME, CONAV_TRN_Z_NAME,
                             "meters", sdim)) {
        ZF_LOGE("Creating %s failed", CONAV_TRN_Z_NAME);
        return false;
    }
    if (!_msgWriter.addArray(Double, CONAV_TRN_VAR_N_NAME, CONAV_TRN_VAR_N_NAME,
                             "", sdim)) {
        ZF_LOGE("Creating %s failed", CONAV_TRN_VAR_N_NAME);
        return false;
    }
    if (!_msgWriter.addArray(Double, CONAV_TRN_VAR_E_NAME, CONAV_TRN_VAR_E_NAME,
                             "", sdim)) {
        ZF_LOGE("Creating %s failed", CONAV_TRN_VAR_E_NAME);
        return false;
    }
    if (!_msgWriter.addArray(Double, CONAV_TRN_VAR_Z_NAME, CONAV_TRN_VAR_Z_NAME,
                             "", sdim)) {
        ZF_LOGE("Creating %s failed", CONAV_TRN_VAR_Z_NAME);
        return false;
    }
    if (!_msgWriter.addArray(Double, CONAV_RANGE_NAME, CONAV_RANGE_NAME,
                             "meters", sdim)) {
        ZF_LOGE("Creating %s failed", CONAV_RANGE_NAME);
        return false;
    }
    if (!_msgWriter.addArray(Double, CONAV_BEARING_NAME, CONAV_BEARING_NAME,
                             "radians", sdim)) {
        ZF_LOGE("Creating %s failed", CONAV_BEARING_NAME);
        return false;
    }
    if (!_msgWriter.addArray(Double, CONAV_RANGE_VAR_NAME, CONAV_RANGE_VAR_NAME,
                             "", sdim)) {
        ZF_LOGE("Creating %s failed", CONAV_RANGE_VAR_NAME);
        return false;
    }
    if (!_msgWriter.addArray(Double, CONAV_BEARING_VAR_NAME,
                             CONAV_BEARING_VAR_NAME, "", sdim)) {
        ZF_LOGE("Creating %s failed", CONAV_BEARING_VAR_NAME);
        return false;
    }
    ZF_LOGD("LCM msgWriter initialized");
    return true;
}

// Handle incoming LCM messages
int handle_lcm()
{
    // Handle the bursty nature of the message flow
    int nm = _lcm->handleTimeout(2000);
    ZF_LOGD("%d messages handled", nm);
    return nm;
}

// Read the next record line from the msg file.
// Parse and place the values into the msgWriter fields
int get_next_record()
{
    char line[500];

    if (NULL == fgets(line, sizeof(line)-1, _csv)) {
        return -1;
    }

    int64_t time;
    int32_t id;
    float a, b, c, d, e, f, g, h, i, j;
    int rc =
        sscanf(line,
               "%11ld,%d,%20f,%20f,%20f,%20f,%20f,%20f,%20f,%20f,%20f,%20f",
               &time, &id,
               &a,  &b,  &c,
               &d,  &e,  &f,
               &g,  &h,
               &i,  &j
               );

    _coopNav.coopClock = (double)time;
    _coopNav.vehId = (double)id;
    _coopNav.trnN = (double)a;
    _coopNav.trnE = (double)b;
    _coopNav.trnZ = (double)c;
    _coopNav.trnNVar = (double)d;
    _coopNav.trnEVar = (double)e;
    _coopNav.trnZVar = (double)f;
    _coopNav.range = (double)g;
    _coopNav.bearing = (double)h;
    _coopNav.rangeVar = (double)i;
    _coopNav.bearingVar = (double)j;

    if (rc != 12) {
        return 1;
    } else {
        _coopNav.coopClock -= MATLAB_TO_EPOCH;
        ZF_LOGI("Read record scheduled for %.3f with range %.2f",
            _coopNav.coopClock, _coopNav.range);
        return 0;
    }
}

// Publish the current Cooperative Vehicle nav data
int publish_coop_nav()
{
    // When the most recent LCM timestamp >= the coop timestamp then
    // publish the coop data
    if (_lastMsgTime <
        (_mrLatency + _coopNav.coopClock)*1000) {
        return -1;
    }

    if (!_msgWriter.set(CONAV_VEHID_NAME, _coopNav.vehId)) {
        ZF_LOGE("Setting %s failed", CONAV_VEHID_NAME);
    }
    //double ft = 1.0 * _coopNav.timestamp;
    // LCM timestamp is epoch milliseconds
    // coopClock timestamp is epoch seconds
    // Convert to epoch seconds if necessary
    //double ft = 1.0 * (_lastMsgTime - _mrLatency*1000);
    // ft *= 1.0/1000.;
    double ft = 1.0 * _coopNav.coopClock;
    if (!_msgWriter.set(CONAV_TIME_NAME, ft)) {
        ZF_LOGE("Setting %s failed", CONAV_TIME_NAME);
    }
    if (!_msgWriter.set(CONAV_TRN_N_NAME, (double)_coopNav.trnN)) {
        ZF_LOGE("Setting %s failed", CONAV_TRN_N_NAME);
    }
    if (!_msgWriter.set(CONAV_TRN_E_NAME, (double)_coopNav.trnE)) {
        ZF_LOGE("Setting %s failed", CONAV_TRN_E_NAME);
    }
    if (!_msgWriter.set(CONAV_TRN_Z_NAME, (double)_coopNav.trnZ)) {
        ZF_LOGE("Setting %s failed", CONAV_TRN_Z_NAME);
    }
    if (!_msgWriter.set(CONAV_TRN_VAR_N_NAME, (double)_coopNav.trnNVar)) {
        ZF_LOGE("Setting %s failed", CONAV_TRN_VAR_N_NAME);
    }
    if (!_msgWriter.set(CONAV_TRN_VAR_E_NAME, (double)_coopNav.trnEVar)) {
        ZF_LOGE("Setting %s failed", CONAV_TRN_VAR_E_NAME);
    }
    if (!_msgWriter.set(CONAV_TRN_VAR_Z_NAME, (double)_coopNav.trnZVar)) {
        ZF_LOGE("Setting %s failed", CONAV_TRN_VAR_Z_NAME);
    }
    if (!_msgWriter.set(CONAV_RANGE_NAME, (double)_coopNav.range)) {
        ZF_LOGE("Setting %s failed", CONAV_RANGE_NAME);
    }
    if (!_msgWriter.set(CONAV_BEARING_NAME, (double)_coopNav.bearing)) {
        ZF_LOGE("Setting %s failed", CONAV_BEARING_NAME);
    }
    if (!_msgWriter.set(CONAV_RANGE_VAR_NAME, (double)_coopNav.rangeVar)) {
        ZF_LOGE("Setting %s failed", CONAV_RANGE_VAR_NAME);
    }
    if (!_msgWriter.set(CONAV_BEARING_VAR_NAME, (double)_coopNav.bearingVar)) {
        ZF_LOGE("Setting %s failed", CONAV_BEARING_VAR_NAME);
    }
    ZF_LOGI("Publishing %s msg vehId %d from %.3f at %ld", COOP_CHANNEL,
        _coopNav.vehId, ft/1000., _lastMsgTime);
        //_coopNav.vehId, _coopNav.timestamp, _lastMsgTime);
    int64_t t = ((int64_t)_coopNav.coopClock) * 1000;
    _msgWriter.publish(*_lcm, COOP_CHANNEL, _lastMsgTime);

    // Get the next coop data set if any
    if (get_next_record() != 0) {
        ZF_LOGI("End of message file reached");
        _quit = 1;
    }

    return 0;
}

void usage(const char* app)
{
    fprintf(stderr, "\nUsage:\n");
    fprintf(stderr, "  %s  path/to/MRMsg.csv\n", app);
}

int64_t getTimeMillisec()
{
    struct timeval spec;
    gettimeofday(&spec, NULL);

    double ms         = (spec.tv_sec * 1000.) + (spec.tv_usec / 1000.);
    int64_t current_ms = ms;
    return current_ms;
}

