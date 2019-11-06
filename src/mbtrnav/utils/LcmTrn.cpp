/*
  Class LcmTrn - A Terrain-Relative Navigation implementation that uses LCM for
                 external comms. After initialization an object of this class
                 can listen on the configured LCM channels for vehicle position
                 data, beam data, and commands (e.g., reinit, change map, etc.)
*/

#include <sys/time.h>
#include <lcm/lcm-cpp.hpp>

#include "LcmTrn.h"

static const char *STR_LCM_TIMEOUT   = "lcm.timeout_ms";
static const char *STR_LCM_TRNNAME   = "TRN-DATA";
static const char *STR_LCM_CMDNAME   = "TRN-CMDS";
static const char *STR_LCM_POSNAME   = "EXAMPLE";
static const char *STR_LCM_MEASNAME  = "EXAMPLE";
static const char *STR_TRN_PERIOD    = "trn.period_ms";
static const char *STR_TRN_INSTTYPE  = "trn.inst_type";
static const char *STR_TRN_MAPNAME   = "trn.map_name";
static const char *STR_TRN_MAPTYPE   = "trn.map_type";
static const char *STR_TRN_CFGNAME   = "trn.cfg_name";
static const char *STR_TRN_LOGNAME   = "trn.log_name";
static const char *STR_TRN_PARTNAME  = "trn.part_name";
static const char *STR_TRN_FILTER    = "trn.filter_type";
static const char *STR_TRN_LOWGRADE  = "trn.force_lowgrade_filter";
static const char *STR_TRN_WEIGHTING = "trn.use_modified_weighting";
static const char *STR_TRN_REINITS   = "trn.allow_filter_reinit";


using namespace lcmTrn;

// Ctor. All initialization info resides in a libconfig configuration file.
// 
LcmTrn::LcmTrn(const char* configfilepath)
  : _period(LCMTRN_DEFAULT_PERIOD), _timeout(LCMTRN_DEFAULT_TIMEOUT), _maptype(0),
    _filtertype(LCMTRN_DEFAULT_FILTER), _lowgrade(LCMTRN_DEFAULT_LOWGRADE),
    _allowreinit(LCMTRN_DEFAULT_ALLOW), _weighting(LCMTRN_DEFAULT_WEIGHTING),
    _instrument(LCMTRN_DEFAULT_INSTRUMENT), _filterstate(0), _numreinits(0),
    _lastUpdateMillisec(-1), _lcm(NULL), _trn(NULL), _mapn(NULL), _cfgn(NULL),
    _partn(NULL), _logd(NULL), _good(false)
{
  printf("LcmTrn::LcmTrn() - configuration file %s\n", configfilepath);

  if (NULL != configfilepath) _configfile = strdup(configfilepath);
  else                        _configfile = strdup(LCMTRN_DEFAULT_CONFIG);

  // All my initialization info is in the config file
  //
  init();
}

LcmTrn::~LcmTrn()
{
  if (_configfile) delete _configfile;
  if (_cfg) delete _cfg;
  cleanTrn();
  cleanLcm();
}

void LcmTrn::cleanTrn()
{
  if (NULL != _trn) delete _trn; _trn = NULL;
  if (NULL != _latestPose) delete _latestPose; _latestPose = NULL;
  if (NULL != _latestMeas) delete _latestMeas; _latestMeas = NULL;
  if (NULL != _mle) delete _mle; _mle = NULL;
  if (NULL != _mse) delete _mse; _mse = NULL;
  _lastUpdateMillisec = -1;
}

void LcmTrn::cleanLcm()
{
  if (NULL != _lcm) delete _lcm; _lcm = NULL;
}

// Initialize using the configuration in the config file
//
void LcmTrn::init()
{
  printf("LcmTrn::init() - using configuration file %s\n", _configfile);

  loadConfig();

  if (!good())
  {
    printf("LcmTrn::init() - Configuration failed using %s!\n", _configfile);
    return;
  }

#if 0
  try
  {
    _cfg.readFile(_configfile);
  }
  catch(const FileIOException &fioex)
  {
    std::cerr << "I/O error while reading file." << std::endl;
    return(EXIT_FAILURE);
  }
  catch(const ParseException &pex)
  {
    std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
              << " - " << pex.getError() << std::endl;
    return(EXIT_FAILURE);
  }
#endif

  // Initialize the LCM part first
  //
  initLcm();

  // Initialize the TRN part
  //
  initTrn();

}

void LcmTrn::loadConfig()
{
  _good = false;

  // Create my config object from my config file
  //
  if (NULL == _cfg) _cfg = new Config();
  _cfg->readFile(_configfile);

  // Set up timing variables according to the config. Use default if not present
  if (!_cfg->lookupValue(STR_LCM_TIMEOUT,_timeout)) _timeout = LCMTRN_DEFAULT_TIMEOUT;
  if (!_cfg->lookupValue(STR_TRN_PERIOD, _period))  _period  = LCMTRN_DEFAULT_TIMEOUT;

  // Load the TRN options next. Use defaults if not present.
  if (!_cfg->lookupValue(STR_TRN_FILTER, _filtertype))   _filtertype  = LCMTRN_DEFAULT_FILTER;
  if (!_cfg->lookupValue(STR_TRN_WEIGHTING, _weighting)) _weighting   = LCMTRN_DEFAULT_WEIGHTING;
  if (!_cfg->lookupValue(STR_TRN_LOWGRADE, _lowgrade))   _lowgrade    = LCMTRN_DEFAULT_LOWGRADE;
  if (!_cfg->lookupValue(STR_TRN_REINITS, _allowreinit)) _allowreinit = LCMTRN_DEFAULT_ALLOW;
  if (!_cfg->lookupValue(STR_TRN_INSTTYPE, _instrument)) _instrument  = LCMTRN_DEFAULT_INSTRUMENT;

  // Load the required TRN stuff next. Throw exception unless all are present
  if (!_cfg->lookupValue(STR_TRN_MAPNAME, _mapn))    _mapn    = NULL;
  if (!_cfg->lookupValue(STR_TRN_MAPTYPE, _maptype)) _maptype = -1;
  if (!_cfg->lookupValue(STR_TRN_CFGNAME, _cfgn))    _cfgn    = NULL;
  if (!_cfg->lookupValue(STR_TRN_PARTNAME,_partn))   _partn   = NULL;
  if (!_cfg->lookupValue(STR_TRN_LOGNAME, _logd))    _logd    = NULL;
  if (!(_mapn && _cfgn && _partn && _logd) || (_maptype < 1 || _maptype > 2))
  {
    printf("LcmTrn::init() - poor trn config settings!\n");
  }
  else
  {
    _good = true;
  }

  printf("LCMTRN settings:\n");
  printf("\tLCM timeout=%d ms\n\tTRN period=%d ms\n",_timeout, _period);
  printf("\tmap = %s\n\tcfg = %s\n\tpart= %s\n\tlogdir= %s\n", _mapn, _cfgn, _partn, _logd);
  printf("\tmaptype = %d\n\tfiltertype = %d\n\tweighting = %d\n", _maptype, _filtertype, _weighting);
  printf("\tlowgrade_filter = %d\n\tallow_reinit = %d\n", _lowgrade, _allowreinit);

}

// The LCM stuff would only be initialized once during a mission
//
void LcmTrn::initLcm()
{
  printf("LcmTrn::initLcm() - configuration file %s\n", _configfile);

  cleanLcm();

  _lcm = new lcm::LCM();
  if(_lcm->good())
  {
    _lcm->subscribe(STR_LCM_POSNAME, &LcmTrn::handlePos, this);
    _lcm->subscribe(STR_LCM_MEASNAME, &LcmTrn::handleMeas, this);
    _lcm->subscribe(STR_LCM_CMDNAME, &LcmTrn::handleCmd, this);
  }

  // _good is true if config file was OK
  _good = _good && _lcm->good();
}

// The TRN stuff could be initialized many times during a mission.
// E.g., re-init using a different map, options, particle file, etc.
//
void LcmTrn::initTrn()
{
  printf("LcmTrn::initTrn() - configuration file %s\n", _configfile);

  cleanTrn();

  _trn = new TerrainNav(
    (char*)_mapn,
    (char*)_cfgn,
    (char*)_partn,
    _filtertype,
    _maptype,
    (char*)_logd
  );

  if(_lowgrade) _trn->useLowGradeFilter();
  else          _trn->useHighGradeFilter();
   
  _trn->setFilterReinit(_allowreinit);
  _trn->setModifiedWeighting(_weighting);
  _trn->setInterpMeasAttitude(true);

  // Create pos and meas structures
  _latestMeas = new measT(4, _instrument);   // DVL == 4 beams
  _latestPose = new poseT;
  _mle = new poseT;
  _mse = new poseT;
}

// Reinitialize. A different config file may be used here.
// A NULL configfilepath results in a reinit using the Config that is currently
// loaded.
//
void LcmTrn::reinit(const char* configfilepath)
{
  printf("LcmTrn::reinit() - reinitializing TRN...\n");
  if (configfilepath)
  {
    if (_configfile) delete _configfile;
    _configfile = strdup(configfilepath);
    
    printf("LcmTrn::reinit() - New configuration file %s\n", _configfile);
  }

  init();
}

void LcmTrn::run()      // Run "forever"
{
  printf("LcmTrn::run()\n");

  // while (true)
  for (int n = 0; n < 12; n++)
  {
    cycle();
  }
  return;
}

bool LcmTrn::updateTrn()
{
  // See if the sample period has expired
  int64_t current_ms = getTimeMillisec();
  if (current_ms < (_lastUpdateMillisec + _period))
    return false;
  else
    _latestPose->time = _latestMeas->time = getTimeMillisec();   // fake it

  // Then see if there is new data to use by examing the poseT and measT structures
  // and comparing timestamps. Need new data from both instruments.
  if (_latestPose->time > _lastUpdateMillisec && _latestMeas->time > _lastUpdateMillisec)
  {
    printf("LcmTrn::updateTrn() - Updating TRN (fake for now)...\n");
    _lastUpdateMillisec = current_ms;

    if (_latestPose->time <= _latestMeas->time)
    {
      _trn->motionUpdate(_latestPose);
      _trn->measUpdate(_latestMeas, _latestMeas->dataType);
    }
    else
    {
      _trn->measUpdate(_latestMeas, _latestMeas->dataType);
      _trn->motionUpdate(_latestPose);
    }
    _trn->estimatePose(_mle, 1);
    _trn->estimatePose(_mse, 2);

    _filterstate = _trn->getFilterState();
    _numreinits  = _trn->getNumReinits();
    return true;
  }


  return false;
}

void LcmTrn::cycle()    // Execute a single LCM read/TRN update cycle and return
{
  if (!good())
  {
    printf("LcmTrn::cycle() - NOT good!\n");
    return;
  }

  _lcm->handleTimeout(_timeout);
  printf("LcmTrn::cycle() - LCM returned, examine new data...\n");

  // If TRN was updated, publish the latest TRN state
  if (updateTrn())
  {
    printf("LcmTrn::cycle() - publishing updated TRN filter state: %d\n",
      _filterstate);

    lcmMessages::DataVectors trnstate;
    // populate state here

    //_lcm->publish(STR_LCM_TRNNAME, &trnstate);
  }
}

int64_t LcmTrn::getTimeMillisec()
{
  long            ms; // Milliseconds
  time_t          s;  // Seconds
  struct timeval spec;

  gettimeofday(&spec, NULL);

  s  = spec.tv_sec;
  ms = spec.tv_usec / 1000; // Convert microseconds to milliseconds

  int64_t current_ms = s*1000 + ms;
  return current_ms;
}

// This is a motionUpdate. Read position data and populate the poseT object.
void LcmTrn::handlePos(const lcm::ReceiveBuffer* rbuf,
                        const std::string& chan, 
                        const lcmMessages::DataVectors* msg)
{
  printf("LcmTrn::handlePos() - Received message on \"%s\":\n", chan.c_str());
  printf("timestamp   = %lld millisec\n", (long long)msg->epochMillisec);
  printf("seqNo: %lld\n", msg->seqNo);
}

// This is a measUpdate. Read position data and populate the measT object.
void LcmTrn::handleMeas(const lcm::ReceiveBuffer* rbuf,
                        const std::string& chan, 
                        const lcmMessages::DataVectors* msg)
{
  printf("LcmTrn::handlePos() - Received message on \"%s\":\n", chan.c_str());
  printf("timestamp   = %lld millisec\n", (long long)msg->epochMillisec);
  printf("seqNo: %lld\n", msg->seqNo);
}

// Read commands and dispatch
void LcmTrn::handleCmd(const lcm::ReceiveBuffer* rbuf,
                        const std::string& chan, 
                        const lcmMessages::DataVectors* msg)
{
  printf("LcmTrn::handlePos() - Received message on \"%s\":\n", chan.c_str());
  printf("timestamp   = %lld millisec\n", (long long)msg->epochMillisec);
  printf("seqNo: %lld\n", msg->seqNo);
}
