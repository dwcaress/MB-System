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
#include <lcmMessages/DataVectors.hpp>
#include "structDefs.h"
#include "TerrainNav.h"

using namespace std;
using namespace libconfig;

#define  LCMTRN_DEFAULT_CONFIG     "./lcm-trn.cfg"
#define  LCMTRN_DEFAULT_PERIOD     5000   // 5 seconds between TRN updates
#define  LCMTRN_DEFAULT_TIMEOUT    1000   // 1 second LCM timeout
#define  LCMTRN_DEFAULT_INSTRUMENT 1
#define  LCMTRN_DEFAULT_FILTER     1
#define  LCMTRN_DEFAULT_WEIGHTING  1
#define  LCMTRN_DEFAULT_LOWGRADE   false
#define  LCMTRN_DEFAULT_ALLOW      true

namespace lcmTrn
{

class LcmTrn 
{
  public:

    LcmTrn(const char* configfilepath);

   ~LcmTrn();

    bool good() { return _good; }
    void reinit(const char* configfilepath = NULL);
    void run();      // Run "forever"
    void cycle();    // Execute a single listen-update cycle and return

    void handlePos(const lcm::ReceiveBuffer* rbuf,
                    const std::string& chan, 
                    const lcmMessages::DataVectors* msg);
    void handleMeas(const lcm::ReceiveBuffer* rbuf,
                    const std::string& chan, 
                    const lcmMessages::DataVectors* msg);
    void handleCmd(const lcm::ReceiveBuffer* rbuf,
                    const std::string& chan, 
                    const lcmMessages::DataVectors* msg);
  protected:

    void init();
    void loadConfig();

    void initLcm();
    void initTrn();
    void cleanTrn();
    void cleanLcm();

    bool updateTrn();

    int64_t getTimeMillisec();

    // Config items
    const char* _configfile;
    Config  *_cfg;
    int _period, _timeout;
    const char *_mapn, *_cfgn, *_partn, *_logd;           // TRN cfg files
    int  _maptype, _filtertype, _weighting, _instrument;  // TRN options
    bool _allowreinit, _lowgrade;

    // Internal data
    lcm::LCM *_lcm;
    TerrainNav *_trn;

    poseT *_latestPose, *_mle, *_mse;
    measT *_latestMeas;
    int _filterstate, _numreinits;

    int64_t  _lastUpdateMillisec;

    bool _good;
};

}

#endif
