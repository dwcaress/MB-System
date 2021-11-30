/*
  Class LcmTrn - A Terrain-Relative Navigation implementation that uses LCM for
                 external comms. After initialization an object of this class
                 can listen on the configured LCM channels for vehicle position
                 data and beam data
*/

#ifndef  LCMTRN_H
#define  LCMTRN_H

#include <stdio.h>
#include <libconfig.h++>

#include <lcm/lcm-cpp.hpp>
#include "TethysLcmTypes/LrauvLcmMessage.hpp"
#include "LcmMessageReader.h"
#include "LcmMessageWriter.h"

#include "structDefs.h"
#include "TerrainNav.h"
#include "TNavConfig.h"

using namespace std;
using namespace libconfig;
using namespace TethysLcmTypes;
using namespace lrauv_lcm_tools;

#define  LCMTRN_CONFIG_ENV         "TRN_DATAFILES"
#define  LCMTRN_DEFAULT_CONFIG     "lcm-trn.cfg"
#define  LCMTRN_DEFAULT_ZONE       10     // default utm zone Monterey Bay
#define  LCMTRN_DEFAULT_PERIOD     5      // min seconds between TRN updates
#define  LCMTRN_DEFAULT_COHERENCE  0.25   // max seconds between AHRS and DVL
#define  LCMTRN_DEFAULT_TIMEOUT    1000   // 1 second LCM timeout
#define  LCMTRN_DEFAULT_INSTRUMENT 1      // DVL instrument
#define  LCMTRN_DEFAULT_NUMBEAMS   4      // DVL instrument
#define  LCMTRN_DEFAULT_FILTER     1
#define  LCMTRN_DEFAULT_WEIGHTING  1
#define  LCMTRN_DEFAULT_LOWGRADE   false
#define  LCMTRN_DEFAULT_ALLOW      true

// Define a useful way to delete and reset a pointer to an object.
// If the ptr is non-NULL, delete the object referenced by the ptr
// and reset the ptr to NULL.
// E.g.:
// SomeObj *obj = new SomeObject();
// DELOBJ(obj);
//
#define DELOBJ(ptr) { if (NULL != ptr) {delete ptr; ptr = NULL;} }

namespace lcmTrn
{

typedef struct lcmconfig_
{
    float timeout;
    const char *ahrs, *heading, *pitch, *roll;
    const char *dvl, *xvel, *yvel, *zvel, *beam1, *beam2, *beam3, *beam4, *valid;
    const char *nav, *lat, *lon;
    const char *depth, *veh_depth, *pressure;
    const char *trn, *mle, *mmse, *var, *reinits, *filter, *updatetime;
    const char *cmd, *reinit, *estimate;
} LcmConfig;

typedef struct trnconfig_
{
    int utm_zone;
    float period, coherence;
    const char *mapn, *cfgn, *partn, *logd;                     // TRN cfg filenames
    int  maptype, filtertype, weighting, instrument, nbeams;    // TRN options
    bool allowreinit, lowgrade;
} TrnConfig;


class LcmTrn
{
  public:

    LcmTrn(const char* configfilepath);

   ~LcmTrn();

    static char* constructFullName(const char* env_var, const char* base_name);

    bool good() { return _good; }
    void run();              // Run while good()
    void cycle();            // Execute a single listen-update cycle and return

    LcmConfig* getLcmConfig() { return &_lcmc; }
    TrnConfig* getTrnConfig() { return &_trnc; }


  protected:

    void init();
    void reinit(const char* configfilepath = NULL);
    void loadConfig();

    void initTrn();
    bool verifyTrnConfig();
    void initTrnState();

    void cleanTrn();
    void initLcm();
    bool verifyLcmConfig();
    void cleanLcm();
    int  handleMessages();
    void publishEstimates();

    void handleAhrs(const lcm::ReceiveBuffer* rbuf,
                    const std::string& chan,
                    const LrauvLcmMessage* msg);
    void handleDvl (const lcm::ReceiveBuffer* rbuf,
                    const std::string& chan,
                    const LrauvLcmMessage* msg);
    void handleDepth(const lcm::ReceiveBuffer* rbuf,
                    const std::string& chan,
                    const LrauvLcmMessage* msg);
    void handleCmd (const lcm::ReceiveBuffer* rbuf,
                    const std::string& chan,
                    const LrauvLcmMessage* msg);
    void handleNav (const lcm::ReceiveBuffer* rbuf,
                    const std::string& chan,
                    const LrauvLcmMessage* msg);

    bool time2Update();
    bool updateTrn();

    int64_t getTimeMillisec();

    // Config items
    const char *_configfile;
    Config  *_cfg;

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
    int64_t _seqNo;

    double  _lastUpdateTimestamp;

    bool _good;

    LcmMessageReader _msg_reader;
    LcmMessageWriter<std::string> _msg_writer;

};

}

#endif
