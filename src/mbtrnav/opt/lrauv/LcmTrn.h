/*****************************************************************************
 * Copyright (c) 2002-2021 MBARI
 * Monterey Bay Aquarium Research Institute, all rights reserved.
 *****************************************************************************
 * @file    LcmTrn.h
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

#ifndef LCMTRN_H
#define LCMTRN_H

/***********************************************************************
 * Headers
 ***********************************************************************/

#include <libconfig.h++>
#include <stdio.h>

#include "LcmMessageReader.h"
#include "LcmMessageWriter.h"
#include "TethysLcmTypes/LrauvLcmMessage.hpp"
#include <lcm/lcm-cpp.hpp>

#include "TNavConfig.h"
#include "TerrainNav.h"
#include "structDefs.h"
#include "macros.h"

using namespace std;
using namespace libconfig;
using namespace TethysLcmTypes;
using namespace lrauv_lcm_tools;

/***********************************************************************
 * Macros
 ***********************************************************************/

// Definitions used in conjunction with configuration file (e.g., lcm-trn.cfg)
#define TRN_MAP_GRID 1
#define TRN_MAP_OCTREE 2

#define TRN_INST_DVL 1

#define TRN_FILTER_PM 1
#define TRN_FILTER_PF 2
#define TRN_FILTER_PMB 3

#define TRN_WEIGHT_NONE 0
#define TRN_WEIGHT_ALPHA 1
#define TRN_WEIGHT_CROSS 2
#define TRN_WEIGHT_SB 3
#define TRN_WEIGHT_SBNIS 4

// Other definitions
#define LCMTRN_CONFIG_ENV "TRN_DATAFILES"
#define LCMTRN_DEFAULT_CONFIG "lcm-trn.cfg"
#define LCMTRN_DEFAULT_ZONE 10        // default utm zone Monterey Bay
#define LCMTRN_DEFAULT_PERIOD 5       // min seconds between TRN updates
#define LCMTRN_DEFAULT_COHERENCE 0.25 // max seconds between AHRS and DVL
#define LCMTRN_DEFAULT_TIMEOUT 1000   // 1 second LCM timeout for normal
#define LCMTRN_DEFAULT_INITIAL 50     // 50 msec initial timeout for bursty
#define LCMTRN_DEFAULT_MAXIMUM 250    // 250 msec max timeout for bursty
#define LCMTRN_DEFAULT_INSTRUMENT 1   // DVL instrument
#define LCMTRN_DEFAULT_NUMBEAMS 4     // DVL instrument
#define LCMTRN_DEFAULT_FILTER 1
#define LCMTRN_DEFAULT_WEIGHTING 1
#define LCMTRN_DEFAULT_LOWGRADE false
#define LCMTRN_DEFAULT_ALLOW true

class TrnLcmDecoder;
namespace lcmTrn
{

typedef struct lcmconfig_ {
    float timeout;
    unsigned int initial_timeout_msec, max_timeout_msec;
    const char *ahrs, *heading, *pitch, *roll;
    const char *dvl, *xvel, *yvel, *zvel, *beam1, *beam2, *beam3, *beam4,
        *valid;
    const char *nav, *lat, *lon;
    const char *depth, *veh_depth, *pressure;
    const char *trn, *cmd, *reinit, *estimate;
} LcmConfig;

typedef struct trnconfig_ {
    int utm_zone;
    float period, coherence;
    const char *mapn, *cfgn, *partn, *logd;                 // TRN cfg filenames
    int maptype, filtertype, weighting, instrument, nbeams; // TRN options
    bool allowreinit, lowgrade;
} TrnConfig;

class LcmTrn
{
  public:
    LcmTrn(const char *configfilepath);

    ~LcmTrn();

    static char *constructFullName(const char *env_var, const char *base_name);

    bool good() { return _good; }
    void run();   // Run while good()
    void cycle(); // Execute a single listen-update cycle and return

    LcmConfig *getLcmConfig() { return &_lcmc; }
    TrnConfig *getTrnConfig() { return &_trnc; }

  protected:
    void init();
    void reinit(const char *configfilepath = NULL);
    void loadConfig();

    void initTrn();
    bool verifyTrnConfig();

    void cleanTrn();
    void initLcm();
    bool verifyLcmConfig();
    void cleanLcm();
    int handleMessages();
    int getLcmTimeout(unsigned int initial_timeout, unsigned int max_timeout);
    void publishEstimatesDecoder();

    void handleAhrs(const lcm::ReceiveBuffer *rbuf, const std::string &chan,
                    const LrauvLcmMessage *msg);
    void handleDvl(const lcm::ReceiveBuffer *rbuf, const std::string &chan,
                   const LrauvLcmMessage *msg);
    void handleDepth(const lcm::ReceiveBuffer *rbuf, const std::string &chan,
                     const LrauvLcmMessage *msg);
    void handleCmd(const lcm::ReceiveBuffer *rbuf, const std::string &chan,
                   const LrauvLcmMessage *msg);
    void handleNav(const lcm::ReceiveBuffer *rbuf, const std::string &chan,
                   const LrauvLcmMessage *msg);

    bool time2Update();
    bool updateTrn();

    int64_t getTimeMillisec();

    // Config items
    const char *_configfile;
    Config *_cfg;

    int _lcmtimeout;
    LcmConfig _lcmc;
    TrnConfig _trnc;

    // Main players
    lcm::LCM *_lcm;
    TerrainNav *_tnav;

    // Internal TRN data
    poseT _thisPose, _lastPose, _mle, _mmse;
    measT _thisMeas, _lastMeas;
    int _filterstate, _numreinits;
    int _lastUtmZone;
    int64_t _seqNo;

    double _lastAHRSTimestamp;
    double _lastDvlTimestamp;
    double _lastNavTimestamp;
    double _lastDepthTimestamp;
    double _lastCmdTimestamp;
    double _lastUpdateTimestamp;

    bool _good;

    LcmMessageReader _msg_reader;
    TrnLcmDecoder* _trn_decoder;
};

} // namespace lcmTrn

#endif
