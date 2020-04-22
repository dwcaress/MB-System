 /*
  Class LcmTrn - A Terrain-Relative Navigation implementation that uses LCM for
                 external comms. After initialization an object of this class
                 can listen on the configured LCM channels for vehicle position
                 data, beam data, and commands (e.g., reinit, change map, etc.)
*/

#include <sys/time.h>
#include <unistd.h>
#include <lcm/lcm-cpp.hpp>

#include "NavUtils.h"
#include "MathP.h"
#include "config_defs.h"
#include "LcmTrn.h"

#define N_DIM    3        // number of dimensions in output estimates (x,y,z)
#define N_COVARS 4        // number of dimensions in output covars (x,y,z,psi)
#define N_INT_VECTORS   2 // reinits, filter
#define N_FLOAT_VECTORS 3 // mle, mmse, var
#define REINIT_VECTOR   0 // REINIT is first vector in DataVectors _trnstate
#define FILTER_VECTOR   1
#define MLE_VECTOR      0
#define MMSE_VECTOR     1
#define VAR_VECTOR      2
#define SCALAR     0      // Indices into output vectors
#define POSE_X     0
#define POSE_Y     1
#define POSE_Z     2
#define POSE_PSI   3
#define COVAR_X    0
#define COVAR_Y    2
#define COVAR_Z    5
#define COVAR_PSI  44

static const int  tl_both = TL_OMASK(TL_TRN_SERVER, TL_BOTH);
static const int  tl_log  = TL_OMASK(TL_TRN_SERVER, TL_LOG);

static const char *STR_LCM_TIMEOUT   = "lcm.timeout_ms";
static const char *STR_LCM_TRNNAME   = "lcm.trn_channel";
static const char *STR_LCM_CMDNAME   = "lcm.cmd_channel";
static const char *STR_LCM_AHRSNAME  = "lcm.ahrs_channel";
static const char *STR_LCM_MEASNAME  = "lcm.dvl_channel";
static const char *STR_LCM_NAVNAME   = "lcm.nav_channel";
static const char *STR_TRN_ZONE      = "trn.utm_zone";
static const char *STR_TRN_PERIOD    = "trn.period_sec";
static const char *STR_TRN_COHERENCE = "trn.temporal_coherence_sec";
static const char *STR_TRN_INSTTYPE  = "trn.inst_type";
static const char *STR_TRN_NUMBEAMS  = "trn.num_beams";
static const char *STR_TRN_MAPTYPE   = "trn.map_type";
static const char *STR_TRN_MAPNAME   = "trn.map_name";
static const char *STR_TRN_CFGNAME   = "trn.cfg_name";
static const char *STR_TRN_PARTNAME  = "trn.part_name";
static const char *STR_TRN_LOGNAME   = "trn.log_name";
static const char *STR_TRN_FILTER    = "trn.filter_type";
static const char *STR_TRN_WEIGHTING = "trn.modified_weighting";
static const char *STR_TRN_LOWGRADE  = "trn.force_lowgrade_filter";
static const char *STR_TRN_REINITS   = "trn.allow_filter_reinit";

using namespace lcmTrn;

// Ctor. All initialization info resides in a libconfig configuration file.
// 
LcmTrn::LcmTrn(const char* configfilepath)
  : _lastUpdateMillisec(-1), _lcm(NULL), _tnav(NULL), _good(false)
{
  logs(tl_both,"LcmTrn::LcmTrn() - configuration file %s\n", configfilepath);

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
  _lcmc.dvl = _lcmc.xvel = _lcmc.yvel = _lcmc.zvel = _lcmc.beam1 = _lcmc.beam2 = _lcmc.beam3 = _lcmc.beam4 = NULL;
  _lcmc.nav = _lcmc.lat = _lcmc.lon = _lcmc.depth = NULL;
  _lcmc.trn = _lcmc.cmd = NULL;

  // Set config filename
  //
  if (NULL != configfilepath) _configfile = strdup(configfilepath);
  else                        _configfile = LCMTRN_DEFAULT_CONFIG;

  // All my initialization info is in the config file
  //
  init();
}

LcmTrn::~LcmTrn()
{
  DELOBJ(_configfile);
  DELOBJ(_cfg);
  cleanTrn();
  cleanLcm();
}

void LcmTrn::cleanTrn()
{
  DELOBJ(_tnav);
  _lastUpdateMillisec = -1;
}

void LcmTrn::cleanLcm()
{
  DELOBJ(_lcm);
}

// Initialize using the configuration in the config file
//
void LcmTrn::init()
{
  logs(tl_both,"LcmTrn::init() - using configuration file %s\n", _configfile);

  loadConfig();

  if (!good())
  {
    logs(tl_both,"LcmTrn::init() - Configuration failed using %s!\n", _configfile);
    return;
  }

  measT* mt = new measT(_trnc.nbeams, _trnc.instrument);
  _thisMeas = *mt;
  _lastMeas = _thisMeas;

  // Initialize the TRN part (instantiate the TerrainNav object)
  //
  initTrn();

  // Initialize the LCM structure used for publishing the TRN state
  //
  initTrnState();

  // Initialize the LCM part first
  //
  initLcm();
}

// Initialize a DataVectors object representing the TRN state.
//
void LcmTrn::initTrnState()
{
  // DataVectors-level info
  //
  _trnstate.seqNo = _trnstate.nDoubleVectors = _trnstate.nStringVectors = 0;
  _trnstate.nIntVectors = N_INT_VECTORS;
  _trnstate.nFloatVectors = N_FLOAT_VECTORS;

  // IntVector stuff
  //
  int ival = 0;
  lcmMessages::IntVector iv;

  // Initialize the IntVector objects with zero values.
  //
  iv.val.insert(iv.val.begin(), ival);

  iv.name = _lcmc.reinits;
  iv.unit = "";
  iv.nVal = 1;
  _trnstate.intVector.insert(_trnstate.intVector.begin()+REINIT_VECTOR, iv);

  iv.name = _lcmc.filter;
  _trnstate.intVector.insert(_trnstate.intVector.begin()+FILTER_VECTOR, iv);

  // FloatVector stuff
  //
  float val = 0.;
  lcmMessages::FloatVector fv;

  // Next the 3 element float vectors with zero values.
  //
  fv.val.insert(fv.val.begin()  , val);
  fv.val.insert(fv.val.begin()+1, val);
  fv.val.insert(fv.val.begin()+2, val);

  fv.name = _lcmc.mle;
  fv.unit = "meters";
  fv.nVal = 3;
  _trnstate.floatVector.insert(_trnstate.floatVector.begin()+MLE_VECTOR, fv);

  fv.name = _lcmc.mmse;
  _trnstate.floatVector.insert(_trnstate.floatVector.begin()+MMSE_VECTOR, fv);

  // Finally the 4 element float vector with zero values.
  //
  fv.val.insert(fv.val.begin()+3, val);

  fv.name = _lcmc.var;
  fv.unit = "meters";
  fv.nVal = 4;
  _trnstate.floatVector.insert(_trnstate.floatVector.begin()+VAR_VECTOR, fv);

  // Setup some handy shortcuts for later
  //
  mlev    = &_trnstate.floatVector[MLE_VECTOR];   
  mmsev   = &_trnstate.floatVector[MMSE_VECTOR];   
  varv    = &_trnstate.floatVector[VAR_VECTOR];   
  reinitv = &_trnstate.intVector[REINIT_VECTOR];   
  filterv = &_trnstate.intVector[FILTER_VECTOR];
}

// Run the app. As long as the status is good continue to perform 
// the read-lcm/process-msg cycle.
// Return only when the status transitions to not good.
//
void LcmTrn::run()
{
  logs(tl_both, "LcmTrn::run()\n");

  // As long as the status is good, process Lcm messages
  //
  while (good())
  {
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
  if (!time2Update()) return false;


  logs(tl_log,"LcmTrn::updateTrn() - heading:%.2f\tpitch:%.2f\troll = %.2f\n",
            _thisPose.phi, _thisPose.theta, _thisPose.psi);

  // Execute motion and measure updates
  if (_thisPose.time <= _lastMeas.time)
  {
    _tnav->motionUpdate(&_thisPose);
    _tnav->measUpdate(&_thisMeas, _thisMeas.dataType);
  }
  else
  {
    _tnav->measUpdate(&_thisMeas, _thisMeas.dataType); 
    _tnav->motionUpdate(&_thisPose);
  }

  // Keep this data around for the next round
  _lastMeas = _thisMeas;
  _lastPose = _thisPose;

  // Request the estimates and TRN state
  _tnav->estimatePose(&_mle, 1);
  _tnav->estimatePose(&_mmse, 2);
  _filterstate = _tnav->getFilterState();
  _numreinits  = _tnav->getNumReinits();

  return true;
}

// Processing is triggered by new data updates received in LCM messages. 
// Execute a single LCM read/TRN update cycle and return.
void LcmTrn::cycle()
{
  // Listen for and handle messages on my channels.
  // The handlers simply read in the latest attitude, position, and
  // beam data. Timeout is passed in as milliseconds.
  //
  int nmsgs = _lcm->handleTimeout(_lcmc.timeout * 1000);
  if (0 == nmsgs)
  {
    logs(tl_log,"LcmTrn::cycle() - No messages handled...\n");
  }
  else if (nmsgs < 0)
  {
    logs(tl_both,"LcmTrn::cycle() - lcm->handleTimeout internal error, good = %d\n",
                    _lcm->good());
    return;
  }

  // If TRN was updated, publish the latest TRN state to
  // the TRN channel. updateTrn() performs motion and measure updates
  // and retrieves lastest position estimates. We just need to push those
  // out here as the updated TRN state.
  //
  if (updateTrn())
  {
    // populate state here from _mle and _mmse objects
    // write the mmse and mle x,y,z LCM vectors from latest estimates
    //
    mmsev->val[POSE_X] = _mmse.x;
    mmsev->val[POSE_Y] = _mmse.y;
    mmsev->val[POSE_Z] = _mmse.z;
    mlev->val[POSE_X] = _mle.x;
    mlev->val[POSE_Y] = _mle.y;
    mlev->val[POSE_Z] = _mle.z;

    // write the var LCM vector from latest estimates
    //
    varv->val[POSE_X]    = _mmse.covariance[COVAR_X];  
    varv->val[POSE_Y]    = _mmse.covariance[COVAR_Y];  
    varv->val[POSE_Z]    = _mmse.covariance[COVAR_Z];  
    varv->val[POSE_PSI]  = _mmse.covariance[COVAR_PSI]; 

    // write the reinit and filter LCM vectors from latest update
    //
    reinitv->val[SCALAR] = _numreinits;
    filterv->val[SCALAR] = _filterstate;

    _trnstate.seqNo++;
    _trnstate.epochMillisec = getTimeMillisec();

    logs(tl_log,"LcmTrn::cycle() - published TRN state %lld @ %lld...\n",
      _trnstate.seqNo, _trnstate.epochMillisec);
    _lcm->publish(_lcmc.trn, &_trnstate);
  }

}

// This is used in a motionUpdate. Read and populate the poseT object fields.
void LcmTrn::handleAhrs(const lcm::ReceiveBuffer* rbuf,
                        const std::string& chan, 
                        const lcmMessages::DataVectors* msg)
{
  // The timestamp recorded in the poseT object is that associated with the
  // AHRS data, not the position data from the DVL.
  //
  _thisPose.time = (double)msg->epochMillisec / 1000;

  // Get heading, pitch, and roll from AHRS message
  //if ((fv = getVector(msg->floatVector, _lcmc.heading)));  // future use
  //
  const lcmMessages::DoubleVector *dv = NULL;
  if ((dv = getVector(msg->doubleVector, _lcmc.heading))) _thisPose.phi   = dv->val[SCALAR];
  if ((dv = getVector(msg->doubleVector, _lcmc.pitch))  ) _thisPose.theta = dv->val[SCALAR];
  if ((dv = getVector(msg->doubleVector, _lcmc.roll))   ) _thisPose.psi   = dv->val[SCALAR];

  logs(tl_log,"%s msg: %.2f epoch sec; seqNo:%lld\n", _lcmc.ahrs, _thisPose.time, msg->seqNo);
  logs(tl_log,"%s msg: %.2f phi; %.2f theta; %.2f psi\n", _lcmc.ahrs, _thisPose.phi, _thisPose.theta, _thisPose.psi);

  // Until we get DVL data flowing lets use the poseT time for the measT time as well
  _thisMeas.time = _thisPose.time;

  return;
}

// Read Nav data for poseT motion updates.
//
void LcmTrn::handleNav(const lcm::ReceiveBuffer* rbuf,
                        const std::string& chan, 
                        const lcmMessages::DataVectors* msg)
{
  _thisPose.time = (double)msg->epochMillisec / 1000;

  // Get lat, lon, and depth from nav message, assume units are degrees. 
  // Convert to radians and/or UTM if necessary using NavUtils.
  //
  float lat_rads = 0, lon_rads = 0;
  const lcmMessages::DoubleVector *dv = NULL;
  if ((dv = getVector(msg->doubleVector, _lcmc.lat))  ) lat_rads = Math::degToRad(dv->val[SCALAR]);
  if ((dv = getVector(msg->doubleVector, _lcmc.lon))  ) lon_rads = Math::degToRad(dv->val[SCALAR]);

  // Convert to UTM for use in TNav
  NavUtils::geoToUtm(lat_rads, lon_rads, NavUtils::geoToUtmZone(lat_rads, lon_rads),
                     &_thisPose.x, &_thisPose.y);  

  logs(tl_log,"%s msg: %.2f epoch sec; seqNo:%lld\n", _lcmc.nav, _thisPose.time, msg->seqNo);
  logs(tl_log,"%s msg: %.2f north; %.2f east\n", _lcmc.nav, _thisPose.x, _thisPose.y);

  return;
}

// This is a measUpdate. Read position and beam data and populate the poseT and measT
// object attributes.
// TRN updates are triggered by these measure and position updates.
//
void LcmTrn::handleDvl(const lcm::ReceiveBuffer* rbuf,
                        const std::string& chan, 
                        const lcmMessages::DataVectors* msg)
{
  // The timestamp recorded in the measT object is that associated with the
  // DVL beam data.
  //
  _thisMeas.time = (double)msg->epochMillisec / 1000;
  _thisMeas.numMeas = 4;
  _thisPose.dvlValid = _thisPose.bottomLock = _thisPose.gpsValid = false;
  _thisMeas.ranges[0]=_thisMeas.ranges[1]=_thisMeas.ranges[2]=_thisMeas.ranges[3]=0.;
  _thisMeas.measStatus[0]=_thisMeas.measStatus[1]=_thisMeas.measStatus[2]=_thisMeas.measStatus[3]=false;

  // Get beam data from DVL message
  //
  const lcmMessages::DoubleVector *dv = NULL;
  if ((dv = getVector(msg->doubleVector, _lcmc.xvel)))
  {
    _thisPose.vx = dv->val[0];
  } else { logs(tl_log, "handleDvl() - %s not found in msg", _lcmc.xvel); }
  if ((dv = getVector(msg->doubleVector, _lcmc.yvel)))
  {
    _thisPose.vy = dv->val[0];
  } else { logs(tl_log, "handleDvl() - %s not found in msg", _lcmc.yvel); }
  if ((dv = getVector(msg->doubleVector, _lcmc.zvel)))
  {
    _thisPose.vz = dv->val[0];
  } else { logs(tl_log, "handleDvl() - %s not found in msg", _lcmc.zvel); }
  if ((dv = getVector(msg->doubleVector, _lcmc.beam1)))
  {
    _thisMeas.ranges[0] = dv->val[0]; _thisMeas.measStatus[0] = true;
  } else { logs(tl_log, "handleDvl() - %s not found in msg", _lcmc.beam1); }
  if ((dv = getVector(msg->doubleVector, _lcmc.beam2)))
  {
    _thisMeas.ranges[1] = dv->val[0]; _thisMeas.measStatus[1] = true;
  } else { logs(tl_log, "handleDvl() - %s not found in msg", _lcmc.beam2); }
  if ((dv = getVector(msg->doubleVector, _lcmc.beam3)))
  {
    _thisMeas.ranges[2] = dv->val[0]; _thisMeas.measStatus[2] = true;
  } else { logs(tl_log, "handleDvl() - %s not found in msg", _lcmc.beam3); }
  if ((dv = getVector(msg->doubleVector, _lcmc.beam4)))
  {
    _thisMeas.ranges[3] = dv->val[0]; _thisMeas.measStatus[3] = true;
  } else { logs(tl_log, "handleDvl() - %s not found in msg", _lcmc.beam4); }

  //const lcmMessages::IntVector *iv = NULL;
  int index = -1;
  if ((index = getVector(msg->intVector, _lcmc.valid)) >= 0)
  {
    _thisPose.bottomLock = msg->intVector[index].val[0] != 0 ? true : false;
  }

  logs(tl_both,"handleDvl() - %s msg: %.2f epoch sec; seqNo:%lld\n",
    _lcmc.dvl, _thisMeas.time, msg->seqNo);
  logs(tl_both,"handleDvl() - %s msg: ranges %d, %.2f , %.2f , %.2f , %.2f\n",
    _lcmc.dvl, _thisPose.dvlValid, _thisMeas.ranges[0], _thisMeas.ranges[1],
                            _thisMeas.ranges[2], _thisMeas.ranges[3]);
  logs(tl_both,"handleDvl() - %s msg: velocities %.2f , %.2f , %.2f\n",
    _lcmc.dvl, _thisPose.vx,  _thisPose.vy,  _thisPose.vz);

  return;
}

// This is a motionUpdate. Read the depth and populate the poseT
// object attribute.
//
void LcmTrn::handleDepth(const lcm::ReceiveBuffer* rbuf,
                        const std::string& chan, 
                        const lcmMessages::DataVectors* msg)
{
  // Get depth data from message
  //
  const lcmMessages::FloatVector *fv = NULL;
  if ((fv = getVector(msg->floatVector, _lcmc.veh_depth)))
  {
    //_thisPose.z = fv->val[0];
    _thisPose.z = getVectorVal(msg->floatVector, _lcmc.veh_depth);
  }

  logs(tl_log,"%s msg: %.2f epoch sec; seqNo:%lld\n", _lcmc.depth, msg->epochMillisec, msg->seqNo);
  logs(tl_log,"%s msg: depth %.2f\n", _lcmc.depth, _thisPose.z);

  return;
}

// Read commands and dispatch
void LcmTrn::handleCmd(const lcm::ReceiveBuffer* rbuf,
                        const std::string& chan, 
                        const lcmMessages::DataVectors* msg)
{
  logs(tl_log,"Cmd msg timestamp   = %lld millisec, seqNo:%lld\n", (long long)msg->epochMillisec, msg->seqNo);

  return;
}


// The LCM stuff would only be initialized once during a mission
//
void LcmTrn::initLcm()
{
  logs(tl_log,"LcmTrn::initLcm() - configuration file %s\n", _configfile);

  cleanLcm();

  _lcm = new lcm::LCM();
  if(_lcm->good())
  {
    _lcm->subscribe(_lcmc.ahrs,  &LcmTrn::handleAhrs,  this);
    _lcm->subscribe(_lcmc.nav,   &LcmTrn::handleNav,   this);
    _lcm->subscribe(_lcmc.dvl,   &LcmTrn::handleDvl,   this);
    _lcm->subscribe(_lcmc.depth, &LcmTrn::handleDepth, this);
    _lcm->subscribe(_lcmc.cmd,   &LcmTrn::handleCmd,   this);
  }

  // _good is true if config file settings are OK and LCM is good
  _good = _good && _lcm->good();
}

// The TRN stuff could be initialized many times during a mission.
// E.g., re-init using a different map, options, particle file, etc.
//
void LcmTrn::initTrn()
{
  logs(tl_log,"LcmTrn::initTrn() - configuration file %s\n", _configfile);

  cleanTrn();

  // Construct the full pathname of the cfgs, maps, and log directory
  //
  char* mapn = constructFullName("TRN_MAPFILES", _trnc.mapn);
  logs(tl_log,"LcmTrn::initTrn() - map: %s\n", mapn);

  char* cfgn = constructFullName("TRN_DATAFILES", _trnc.cfgn);
  logs(tl_log,"LcmTrn::initTrn() - cfg: %s\n", cfgn);

  char* partn = constructFullName("TRN_DATAFILES", _trnc.partn);
  logs(tl_log,"LcmTrn::initTrn() - part: %s\n", partn);

  // Instantiate the TerrainNav object using the config settings
  //
  _tnav = new TerrainNav(
    mapn,
    cfgn,
    partn,
    _trnc.filtertype,
    _trnc.maptype,
    (char*)_trnc.logd
  );

  TNavConfig::instance()->setIgnoreGps(1);

  free(mapn);
  free(cfgn);
  free(partn);

  // Continue with initial settings
  //
  if(_trnc.lowgrade) _tnav->useLowGradeFilter();
  else               _tnav->useHighGradeFilter();
   
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
void LcmTrn::reinit(const char* configfilepath)
{
  logs(tl_log,"LcmTrn::reinit() - reinitializing TRN...\n");
  if (configfilepath)
  {
    DELOBJ(_configfile);
    _configfile = strdup(configfilepath);
    
    logs(tl_log,"LcmTrn::reinit() - New configuration file %s\n", _configfile);

    initTrn();
  }
  else
  {
    logs(tl_log,"LcmTrn::reinit() - calling tnav->reinitFilter(true)\n");
    if (_tnav) _tnav->reinitFilter(true);
  }
}

int64_t LcmTrn::getTimeMillisec()
{
  int64_t   s, ms; // Milliseconds
  struct timeval spec;

  gettimeofday(&spec, NULL);

  double _ms = (spec.tv_sec * 1000.) + (spec.tv_usec / 1000.);
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
  if (_lcmc.timeout <= 0.01)
  {
    logs(tl_both,"LcmTrn::verifyLcmConfig() - timeout must be > 0\n");
    isgood = false;
  }
  if (!(_lcmc.ahrs && _lcmc.heading && _lcmc.pitch && _lcmc.roll))
  {
    logs(tl_both,"LcmTrn::verifyLcmConfig() - ahrs channel, heading, pitch, and roll names are all required.\n");
    isgood = false;
  }
  if (!(_lcmc.dvl && _lcmc.xvel && _lcmc.yvel && _lcmc.zvel && _lcmc.beam1 && _lcmc.beam2 && _lcmc.beam3 && _lcmc.beam4 && _lcmc.valid))
  {
    logs(tl_both,"LcmTrn::verifyLcmConfig() - dvl channel and beam names are all required.\n");
    isgood = false;
  }
  if (!(_lcmc.nav && _lcmc.lat && _lcmc.lon))
  {
    logs(tl_both,"LcmTrn::verifyLcmConfig() - nav channel, lat, lon, and depth names are all required.\n");
    isgood = false;
  }
  if (!(_lcmc.depth && _lcmc.veh_depth && _lcmc.pressure))
  {
    logs(tl_both,"LcmTrn::verifyLcmConfig() - depth channel, veh_depth, and pressure names are all required.\n");
    isgood = false;
  }
  if (!(_lcmc.trn && _lcmc.mle && _lcmc.mmse && _lcmc.var && _lcmc.reinits && _lcmc.filter))
  {
    logs(tl_both,"LcmTrn::verifyLcmConfig() - trn channel, mle, mmse, var, filter, and reinits names are all required.\n");
    isgood = false;
  }
  if (!(_lcmc.cmd))
  {
    logs(tl_both,"LcmTrn::verifyLcmConfig() - cmd channel required.\n");
    isgood = false;
  }
  if (!isgood)
  {
    logs(tl_both,"LcmTrn::verifyLcmConfig() - Incomplete LCM settings in %s.\n", _configfile);
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
  if (!(_trnc.mapn && _trnc.cfgn && _trnc.partn && _trnc.logd))
  {
    logs(tl_both,"LcmTrn::verifyTrnConfig() - map, config file, particle file, and log dir are all required.\n");
    isgood = false;
  }
  if (_trnc.maptype != TRN_MAP_GRID && _trnc.maptype != TRN_MAP_OCTREE)
  {
    logs(tl_both,"LcmTrn::verifyTrnConfig() - Unrecognized map type specified in %s.\n", _configfile);
    isgood = false;
  }
  if (_trnc.instrument != TRN_INST_DVL)
  {
    logs(tl_both,"LcmTrn::verifyTrnConfig() - Unrecognized instrument specified in %s.\n", _configfile);
    isgood = false;
  }
  if (_trnc.weighting < TRN_WEIGHT_NONE && _trnc.weighting > TRN_WEIGHT_SBNIS)
  {
    logs(tl_both,"LcmTrn::verifyTrnConfig() - Unrecognized weighting specified in %s.\n", _configfile);
    isgood = false;
  }
  if (_trnc.filtertype < TRN_FILTER_PM && _trnc.filtertype > TRN_FILTER_PMB)
  {
    logs(tl_both,"LcmTrn::verifyTrnConfig() - Unrecognized filter type specified in %s.\n", _configfile);
    isgood = false;
  }
  if (!isgood)
  {
    logs(tl_both,"LcmTrn::verifyTrnConfig() - Incomplete TRN settings in %s.\n", _configfile);
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
  if (NULL == _cfg) _cfg = new Config();
  _cfg->readFile(_configfile);

  // Load the TRN options next. Use defaults if not present.
  if (!_cfg->lookupValue(STR_TRN_ZONE,   _trnc.utm_zone))     _trnc.utm_zone    = LCMTRN_DEFAULT_ZONE;
  if (!_cfg->lookupValue(STR_TRN_PERIOD, _trnc.period))       _trnc.period      = LCMTRN_DEFAULT_PERIOD;
  if (!_cfg->lookupValue(STR_TRN_COHERENCE, _trnc.coherence)) _trnc.coherence   = LCMTRN_DEFAULT_COHERENCE;
  if (!_cfg->lookupValue(STR_TRN_FILTER, _trnc.filtertype))   _trnc.filtertype  = LCMTRN_DEFAULT_FILTER;
  if (!_cfg->lookupValue(STR_TRN_WEIGHTING, _trnc.weighting)) _trnc.weighting   = LCMTRN_DEFAULT_WEIGHTING;
  if (!_cfg->lookupValue(STR_TRN_LOWGRADE, _trnc.lowgrade))   _trnc.lowgrade    = LCMTRN_DEFAULT_LOWGRADE;
  if (!_cfg->lookupValue(STR_TRN_REINITS, _trnc.allowreinit)) _trnc.allowreinit = LCMTRN_DEFAULT_ALLOW;
  if (!_cfg->lookupValue(STR_TRN_INSTTYPE, _trnc.instrument)) _trnc.instrument  = LCMTRN_DEFAULT_INSTRUMENT;
  if (!_cfg->lookupValue(STR_TRN_NUMBEAMS, _trnc.nbeams))     _trnc.nbeams      = LCMTRN_DEFAULT_NUMBEAMS;
  if (!_cfg->lookupValue(STR_TRN_MAPTYPE, _trnc.maptype))     _trnc.maptype     = TRN_MAP_OCTREE;

  // Load the required TRN config stuff next. Flag error unless all are present
  if (!_cfg->lookupValue(STR_TRN_MAPNAME, _trnc.mapn))    _trnc.mapn    = NULL;
  if (!_cfg->lookupValue(STR_TRN_CFGNAME, _trnc.cfgn))    _trnc.cfgn    = NULL;
  if (!_cfg->lookupValue(STR_TRN_PARTNAME,_trnc.partn))   _trnc.partn   = NULL;
  if (!_cfg->lookupValue(STR_TRN_LOGNAME, _trnc.logd))    _trnc.logd    = NULL;

  // Load the required LCM stuff next. Flag error unless all are present
  if (!_cfg->lookupValue("lcm.timeout_sec", _lcmc.timeout))  _lcmc.timeout = -1.;

  if (!_cfg->lookupValue("lcm.ahrs_channel", _lcmc.ahrs))    _lcmc.ahrs    = NULL;
  if (!_cfg->lookupValue("lcm.ahrs_heading", _lcmc.heading)) _lcmc.heading = NULL;
  if (!_cfg->lookupValue("lcm.ahrs_pitch", _lcmc.pitch))     _lcmc.pitch   = NULL;
  if (!_cfg->lookupValue("lcm.ahrs_roll", _lcmc.roll))       _lcmc.roll    = NULL;
  logs(tl_log, "ahrs config: %s, %s, %s, %s\n",
                        _lcmc.ahrs, _lcmc.heading, _lcmc.pitch, _lcmc.roll);

  if (!_cfg->lookupValue("lcm.dvl_channel", _lcmc.dvl))      _lcmc.dvl   = NULL;
  if (!_cfg->lookupValue("lcm.dvl_xvel",  _lcmc.xvel))       _lcmc.xvel  = NULL;
  if (!_cfg->lookupValue("lcm.dvl_yvel",  _lcmc.yvel))       _lcmc.yvel  = NULL;
  if (!_cfg->lookupValue("lcm.dvl_zvel",  _lcmc.zvel))       _lcmc.zvel  = NULL;
  if (!_cfg->lookupValue("lcm.dvl_beam1", _lcmc.beam1))      _lcmc.beam1 = NULL;
  if (!_cfg->lookupValue("lcm.dvl_beam2", _lcmc.beam2))      _lcmc.beam2 = NULL;
  if (!_cfg->lookupValue("lcm.dvl_beam3", _lcmc.beam3))      _lcmc.beam3 = NULL;
  if (!_cfg->lookupValue("lcm.dvl_beam4", _lcmc.beam4))      _lcmc.beam4 = NULL;
  if (!_cfg->lookupValue("lcm.dvl_valid", _lcmc.valid))      _lcmc.valid = NULL;
  logs(tl_log, "dvl config: %s, %s, %s, %s, %s, %s\n",
                        _lcmc.dvl, _lcmc.beam1, _lcmc.beam2, _lcmc.beam3, _lcmc.beam4, _lcmc.valid);

  if (!_cfg->lookupValue("lcm.nav_channel", _lcmc.nav))      _lcmc.nav  = NULL;
  if (!_cfg->lookupValue("lcm.nav_lat", _lcmc.lat))          _lcmc.lat  = NULL;
  if (!_cfg->lookupValue("lcm.nav_lon", _lcmc.lon))          _lcmc.lon  = NULL;
  logs(tl_log, "nav config: %s, %s, %s\n",
                        _lcmc.nav, _lcmc.lat, _lcmc.lon);

  if (!_cfg->lookupValue("lcm.depth_channel", _lcmc.depth)) _lcmc.depth     = NULL;
  if (!_cfg->lookupValue("lcm.veh_depth", _lcmc.veh_depth)) _lcmc.veh_depth = NULL;
  if (!_cfg->lookupValue("lcm.pressure", _lcmc.pressure))   _lcmc.pressure  = NULL;
  logs(tl_log, "depth config: %s, %s, %s\n",
                        _lcmc.depth, _lcmc.veh_depth, _lcmc.pressure);

  if (!_cfg->lookupValue("lcm.trn_channel", _lcmc.trn))     _lcmc.trn     = NULL;
  if (!_cfg->lookupValue("lcm.trn_mle", _lcmc.mle))         _lcmc.mle     = NULL;
  if (!_cfg->lookupValue("lcm.trn_mmse", _lcmc.mmse))       _lcmc.mmse    = NULL;
  if (!_cfg->lookupValue("lcm.trn_var", _lcmc.var))         _lcmc.var     = NULL;
  if (!_cfg->lookupValue("lcm.trn_reinits", _lcmc.reinits)) _lcmc.reinits = NULL;
  if (!_cfg->lookupValue("lcm.trn_filter", _lcmc.filter))   _lcmc.filter  = NULL;
  logs(tl_log, "trn config: %s, %s, %s, %s, %s, %s\n",
                        _lcmc.trn, _lcmc.mle, _lcmc.mmse, _lcmc.var, _lcmc.reinits, _lcmc.filter);

  if (!_cfg->lookupValue("lcm.cmd_channel", _lcmc.cmd))     _lcmc.cmd     = NULL;

  // Verify the configuration options
  //
  _good = verifyTrnConfig() && verifyLcmConfig();

  logs(tl_both,"LCM timeout=%.2f sec\n", _lcmc.timeout);
  logs(tl_both,"TRN settings:\n");
  logs(tl_both,"\tperiod=%.2f sec\n", _trnc.period);
  logs(tl_both,"\tcoherence=%.2f sec\n", _trnc.coherence);
  logs(tl_both,"\tmap = %s\n\tcfg = %s\n\tpart= %s\n\tlogdir= %s\n",
      _trnc.mapn, _trnc.cfgn, _trnc.partn, _trnc.logd);
  logs(tl_both,"\tmaptype = %d\n\tfiltertype = %d\n\tweighting = %d\n",
      _trnc.maptype,  _trnc.filtertype,  _trnc.weighting);
  logs(tl_both,"\tlowgrade_filter = %d\n\tallow reinit = %d\n",
      _trnc.lowgrade,        _trnc.allowreinit);
}

// Return true if it is time to perform TRN updates
bool LcmTrn::time2Update()
{
  // Has the trn period expired yet?
  // If the TRN period has passed since the last DVL update, then yes.
  bool period = ((_lastMeas.time + _trnc.period) <= _thisMeas.time);

  // We might want to place more requirements on this in the future.
  // For example, perhaps the data should be synced to within a given threshold
  // since position data is comprised of data from multiple sources and could
  // stand for a level of cohesion.
  //
  bool synced = (fabs(_thisPose.time - _thisMeas.time) <= _trnc.coherence);

  return (period && synced);
}

// Construct the full pathname of a file given:
//   env_var: the environment variable name for the base directory
//   base_name: the base name of the file.
//
// Returns a pointer to a malloc'd char* containing the full name.
// The caller is responsible for calling free() on the returned pointer.
// 
char* LcmTrn::constructFullName(const char* env_var, const char* base_name)
{
  // Get the variable if it exists. If not use empty string.
  //
  char *env = getenv(env_var);
  if (NULL == env) env = "";

  // malloc space for the full name and construct it.
  // size = strlen(env) + strlen(base_name) + 2 for the null char and the slash
  //
  char *full_name = (char*)malloc(strlen(env)+strlen(base_name)+2);
  sprintf(full_name, "%s/%s", env, base_name);

  return full_name;
}


float LcmTrn::getVectorVal(std::vector<lcmMessages::FloatVector> fv,
                                                   std::string name)
{
  float val = 0.;
  const lcmMessages::FloatVector *item = NULL;
  // Iterate through the floatVector list looking for a match to name.
  // 
  std::vector<lcmMessages::FloatVector>::const_iterator ptr = fv.begin();
  for (int v = 0; ptr <  fv.end() && item == NULL; ptr++, v++)
  {
    if (ptr->name == name)
    {
      //item = &fv[v];   // Match found, done
      item = &*ptr;   // Match found, done
    }
  }

  if (item) val = item->val[0];

  return val;
}


// Return the FloatVector that matches the given name.
// Returns NULL if no match is found.
//
const lcmMessages::FloatVector*  LcmTrn::getVector(std::vector<lcmMessages::FloatVector> fv,
                                                   std::string name)
{
  const lcmMessages::FloatVector *item = NULL;
  // Iterate through the floatVector list looking for a match to name.
  // 
  std::vector<lcmMessages::FloatVector>::const_iterator ptr = fv.begin();
  for (int v = 0; ptr <  fv.end() && item == NULL; ptr++, v++)
  {
    if (ptr->name == name)
    {
      //item = &fv[v];   // Match found, done
      item = &*ptr;   // Match found, done
    }
  }

  return item;
}


// Return the IntVector that matches the given name.
// Returns NULL if no match is found.
//
int LcmTrn::getVector(std::vector<lcmMessages::IntVector> iv,
                                                   std::string name)
{
  int index = -1;
  // Iterate through the intVector list looking for a match to name.
  // 
  std::vector<lcmMessages::IntVector>::const_iterator ptr = iv.begin();
  for (int v = 0; ptr <  iv.end(); ptr++, v++)
  {
    if (ptr->name == name)
    {
      index = v;
      return index;
    }
  }

  return index;
}

// Return the IntVector that matches the given name.
// Returns NULL if no match is found.
//
//const lcmMessages::IntVector*  LcmTrn::getVector(std::vector<lcmMessages::IntVector> iv,
const lcmMessages::IntVector*  getVector(std::vector<lcmMessages::IntVector> iv,
                                                   std::string name)
{
  const lcmMessages::IntVector *item = NULL;
  // Iterate through the intVector list looking for a match to name.
  // 
  std::vector<lcmMessages::IntVector>::const_iterator ptr = iv.begin();
  for (int v = 0; ptr <  iv.end() && item == NULL; ptr++, v++)
  {
    if (ptr->name == name)
    {
      //item = &iv[v];   // Match found, done
      item = &*ptr;   // Match found, done
    }
  }

  return item;
}


// Return the DoubleVector that matches the given name.
// Returns NULL if no match is found.
//
const lcmMessages::DoubleVector*  LcmTrn::getVector(std::vector<lcmMessages::DoubleVector> dv,
                                                   std::string name)
{
  const lcmMessages::DoubleVector *item = NULL;
  // Iterate through the DoubleVector list looking for a match to name.
  // 
  std::vector<lcmMessages::DoubleVector>::const_iterator ptr = dv.begin();
  for (int v = 0; ptr <  dv.end() && item == NULL; ptr++, v++)
  {
    if (ptr->name == name)
    {
      //item = &dv[v];   // Match found, done
      item = &*ptr;   // Match found, done
    }
  }

  return item;
}


// Return the StringVector that matches the given name.
// Returns NULL if no match is found.
//
const lcmMessages::StringVector*  LcmTrn::getVector(std::vector<lcmMessages::StringVector> sv,
                                                   std::string name)
{
  const lcmMessages::StringVector *item = NULL;
  // Iterate through the stringVector list looking for a match to name.
  // 
  std::vector<lcmMessages::StringVector>::const_iterator ptr = sv.begin();
  for (int v = 0; ptr <  sv.end() && item == NULL; ptr++, v++)
  {
    if (ptr->name == name)
    {
      //item = &sv[v];   // Match found, done
      item = &*ptr;   // Match found, done
    }
  }

  return item;
}

#if 0
// Return the FloatVector that matches the given name.
// Returns NULL if no match is found.
//
const lcmMessages::FloatVector* LcmTrn::getVector(const lcmMessages::DataVectors* msg,
                                                  std::string name)
{
  const lcmMessages::FloatVector *item = NULL;

  // Iterate through the floatVector list looking for a match to name.
  // 
  const lcmMessages::FloatVector *fv = NULL;
  for (int v = 0; v < msg->nFloatVectors && item == NULL; v++)
  {
    const lcmMessages::FloatVector *fv = &msg->floatVector[v];
    if (msg->floatVector[v].name == name)
    {
      item = &msg->floatVector[v];   // Match found, done
    }
  }

  if (NULL == item) logs(tl_log, "No FloatVector match to %s found\n", name.c_str());

  return item;
}
#endif
