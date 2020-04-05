/* FILENAME      : TerrainNavLcmClient.h
 * AUTHOR        : Rich Henthorn
 * DATE          : 12/17/19
 * DESCRIPTION   : TerrainNav is used to extract terrain-correlation based 
 *                 estimates of vehicle position and attitude for use in a 
 *                 navigation system.  This class is an interface class between 
 *                 the vehicle user and the terrain navigation filter classes.
 *                 This Client version forwards most of the requests over LCM
 *                 channels to a TerrainNav process running on a LCM node.
 *
 * DEPENDENCIES  : structDefs.h, myOutput.h, TNavFilter.h, TNavPointMassFilter.h
 *                 TNavParticleFilter.h TNavExtendedKalmanFilter.h 
 *                 TNavSigmaPointFilter.h, TerrainNavClient.h
 * 
 * LAST MODIFIED : 
 * MODIFIED BY   : 
 * 2019-12-17 RGH: Created
 * -----------------------------------------------------------------------------
 * Modification History:
 * -----------------------------------------------------------------------------
 *****************************************************************************/

#include "TerrainNavLcmClient.h"
#include "NavUtils.h"
#include "MathP.h"

using namespace lcmTrn;

/*!
 * Class: TerrainNavClient
 * 
 */

/* Just calls init(). */
TerrainNavLcmClient::TerrainNavLcmClient()
{
  initLcm();
  return;
}

/* Server constructor: Just establish a connection to a server.
 * Just calls init().
 */
TerrainNavLcmClient::TerrainNavLcmClient(char *server_ip, int server_port)
{
  initLcm();
  return;
}


/* Constructor: TerrainNavLcmClient(mapName)
 * Usage: tercom = new TerrainNavClient("canyonmap");
 * -------------------------------------------------------------------------*/
/*! Initializes a new TerrainNav with terrain map "mapName.txt". The mapping 
 * AUV specs and the Point Mass Filter algorithm are used as defaults.
 * 2016-12-15 RGH: Add particle file name & the log directory name to i/f
 *
 * Just calls init().
 */
TerrainNavLcmClient::TerrainNavLcmClient(char *server_ip, int server_port,
		   char *mapName, char *vehicleSpecs, char *particlefile, char *logdir,
		   const int &filterType,
		   const int &mapType)
{
  initLcm();
  return;
}


/* Destructor: ~TerrainNavLcmClient()
 * Usage: delete tercom;
 * -------------------------------------------------------------------------*/
/*! Frees all storage associated with the TerrainNav object.
 */
TerrainNavLcmClient::~TerrainNavLcmClient()
{
  DELOBJ(_lcmTrn);
  return;
}

void TerrainNavLcmClient::initLcm()
{
  _lcm    = NULL;
  _lcmTrn = NULL;
  _lcmc   = NULL;
  _trnc   = NULL;

  _lcmTrn = new LcmTrn("config/lcm-trn.cfg");
  if (!_lcmTrn->good())
  {
    logs(tl_both,"TerrainNavLcmClient::init() - LcmTrn initialization failed\n");
    _lcmTrn = NULL;
    return;
  }

  // Get the configuration specs
  //
  _lcmc = _lcmTrn->getLcmConfig();
  _trnc = _lcmTrn->getTrnConfig();
  if (!(_lcmc && _trnc))
  {
    logs(tl_both,"TerrainNavLcmClient::init() - System configuration unavailable\n");
    _lcmTrn = NULL;
    return;
  }


  _lcm = new lcm::LCM();
  if(_lcm->good())
  {
    _lcm->subscribe(_lcmc->trn, &TerrainNavLcmClient::handleTrn, this);
  }
  else
  {
    logs(tl_both,"TerrainNavLcmClient::init() - LCM initialization failed\n");
    _lcm = NULL;
    return;
  }

  initLcmDataVectors();

  return;
}

void TerrainNavLcmClient::initLcmDataVectors()
{
  double dval = 0.;
  lcmMessages::DoubleVector dv;
  dv.nVal = 1;
  dv.val.insert(dv.val.begin(), dval);

  float fval = 0.;
  lcmMessages::FloatVector fv;
  fv.nVal = 1;
  fv.val.insert(fv.val.begin(), fval);

  int ival = 0;
  lcmMessages::IntVector iv;
  iv.nVal = 1;
  iv.val.insert(iv.val.begin(), ival);

  // AHRS DataVectors-level info
  //
  _ahrsstate.seqNo = _ahrsstate.nFloatVectors = _ahrsstate.nIntVectors = _ahrsstate.nStringVectors = 0;
  _ahrsstate.nDoubleVectors = 3;

  dv.unit = "radians";
  dv.name = _lcmc->heading;
  _ahrsstate.doubleVector.insert(_ahrsstate.doubleVector.end(), dv);
  dv.name = _lcmc->pitch;
  _ahrsstate.doubleVector.insert(_ahrsstate.doubleVector.end(), dv);
  dv.name = _lcmc->roll;
  _ahrsstate.doubleVector.insert(_ahrsstate.doubleVector.end(), dv);


  // DVL DataVectors-level info
  //
  _dvlstate.seqNo = _dvlstate.nFloatVectors = _dvlstate.nStringVectors = 0;
  _dvlstate.nDoubleVectors = 7;
  _dvlstate.nIntVectors = 1;

  dv.unit = "meters";
  dv.name = _lcmc->beam1;
  _dvlstate.doubleVector.insert(_dvlstate.doubleVector.end(), dv);
  dv.name = _lcmc->beam2;
  _dvlstate.doubleVector.insert(_dvlstate.doubleVector.end(), dv);
  dv.name = _lcmc->beam3;
  _dvlstate.doubleVector.insert(_dvlstate.doubleVector.end(), dv);
  dv.name = _lcmc->beam4;
  _dvlstate.doubleVector.insert(_dvlstate.doubleVector.end(), dv);
  dv.unit = "meters/sec";
  dv.name = _lcmc->xvel;
  _dvlstate.doubleVector.insert(_dvlstate.doubleVector.end(), dv);
  dv.name = _lcmc->yvel;
  _dvlstate.doubleVector.insert(_dvlstate.doubleVector.end(), dv);
  dv.name = _lcmc->zvel;
  _dvlstate.doubleVector.insert(_dvlstate.doubleVector.end(), dv);

  iv.unit = "";
  iv.name = _lcmc->valid;
  _dvlstate.intVector.insert(_dvlstate.intVector.end(), iv);


  // NAV DataVectors-level info
  //
  _navstate.seqNo = _navstate.nFloatVectors = _navstate.nIntVectors = _navstate.nStringVectors = 0;
  _navstate.nDoubleVectors = 2;

  dv.unit = "decimal degrees";
  dv.name = _lcmc->lat;
  _navstate.doubleVector.insert(_navstate.doubleVector.end(), dv);
  dv.name = _lcmc->lon;
  _navstate.doubleVector.insert(_navstate.doubleVector.end(), dv);


  // DEPTH DataVectors-level info (Depth_Keller uses FloatVectors)
  //
  _depthstate.seqNo = _depthstate.nDoubleVectors = _depthstate.nIntVectors = _depthstate.nStringVectors = 0;
  _depthstate.nFloatVectors = 1;

  fv.unit = "meters";
  fv.name = _lcmc->veh_depth;
  _depthstate.floatVector.insert(_depthstate.floatVector.end(), fv);


  return;
}

void TerrainNavLcmClient::handleTrn (const lcm::ReceiveBuffer* rbuf,
                                     const std::string& chan, 
                                     const lcmMessages::DataVectors* msg)
{
}


/* Function: estimatePose
 * Usage: tercom->estimatePose(currEstimate, type);
 * -------------------------------------------------------------------------*/
void TerrainNavLcmClient::estimatePose(poseT* estimate, const int &type)
{
  return;
}
		
/* Function: measUpdate
 * Usage: tercom->measUpdate(currMeas, type);
 * -------------------------------------------------------------------------*/
void TerrainNavLcmClient::measUpdate(measT* incomingMeas, const int &type)
{
  _dvlstate.doubleVector[0].val[0] = incomingMeas->ranges[0];
  _dvlstate.doubleVector[1].val[0] = incomingMeas->ranges[1];
  _dvlstate.doubleVector[2].val[0] = incomingMeas->ranges[2];
  _dvlstate.doubleVector[3].val[0] = incomingMeas->ranges[3];

  logs(tl_log,"TerrainNavLcmClient::measUpdate() - publish LCM/%s\n", _lcmc->dvl);
  logs(tl_log,"TerrainNavLcmClient::measUpdate() - %.2f, %.2f, %.2f, %.2f\n",
    incomingMeas->ranges[0], incomingMeas->ranges[1], incomingMeas->ranges[2], incomingMeas->ranges[3]);
  _dvlstate.epochMillisec = incomingMeas->time * 1000.;
  _lcm->publish(_lcmc->dvl, &_dvlstate);

  return;
}

   
/* Function: motionUpdate
 * Usage: tercom->motionUpdate(currEstimate);
 * -------------------------------------------------------------------------*/
void TerrainNavLcmClient::motionUpdate(poseT* incomingNav)
{
  _ahrsstate.doubleVector[0].val[0] = incomingNav->phi;
  _ahrsstate.doubleVector[1].val[0] = incomingNav->theta;
  _ahrsstate.doubleVector[2].val[0] = incomingNav->psi;

  logs(tl_both,"TerrainNavLcmClient::measUpdate() - publish LCM/%s\n", _lcmc->ahrs);
  _ahrsstate.epochMillisec = incomingNav->time * 1000.;
  _lcm->publish(_lcmc->ahrs, &_ahrsstate);


  double lat, lon;
  NavUtils::utmToGeo(incomingNav->x, incomingNav->y, 10,
                     &lat, &lon);
  _navstate.doubleVector[0].val[0] = Math::radToDeg(lat);
  _navstate.doubleVector[1].val[0] = Math::radToDeg(lon);

  logs(tl_both,"TerrainNavLcmClient::measUpdate() - publish LCM/%s\n", _lcmc->nav);
  _navstate.epochMillisec = incomingNav->time * 1000.;
  _lcm->publish(_lcmc->nav, &_navstate);


  _depthstate.floatVector[0].val[0] = incomingNav->z;

  logs(tl_both,"TerrainNavLcmClient::measUpdate() - publish LCM/%s\n", _lcmc->depth);
  _depthstate.epochMillisec = incomingNav->time * 1000.;
  _lcm->publish(_lcmc->depth, &_depthstate);

  _dvlstate.doubleVector[4].val[0] = incomingNav->vx;
  _dvlstate.doubleVector[5].val[0] = incomingNav->vy;
  _dvlstate.doubleVector[6].val[0] = incomingNav->vz;
  _dvlstate.intVector[0].val[0] = incomingNav->bottomLock;

  return;
}

/* Function: outstandingMeas
 * Usage: tercom->outstandingMeas;
 * -------------------------------------------------------------------------*/
bool TerrainNavLcmClient::outstandingMeas()
{
  return false;
}

/* Function: lastMeasSuccessful
 * Usage: tercom->lastMeasSuccessful();
 * -------------------------------------------------------------------------*/
bool TerrainNavLcmClient::lastMeasSuccessful()
{
  return true;
}


/* Function: setInterpMeasAttitude()
 * Usage: tercom->setInterpMeasAttitude(1);
 * -------------------------------------------------------------------------*/
void TerrainNavLcmClient::setInterpMeasAttitude(bool set)
{
  logs(tl_both,"TerrainNavLcmClient::setInterpMeasAttitude - initialized by LcmTrn\n");
  return;
}


/* Function: setMapInterpMethod()
 * Usage: tercom->setMapInterpMethod(1);
 * -------------------------------------------------------------------------*/
void TerrainNavLcmClient::setMapInterpMethod(const int &type)
{
  logs(tl_both,"TerrainNavLcmClient::setMapInterpMethod - initialized by LcmTrn\n");
  return;
}


/* Function: setVehicleDriftRate()
 * Usage: tercom->setVehicleDriftRate(5)
 * -------------------------------------------------------------------------*/
void TerrainNavLcmClient::setVehicleDriftRate(const double &driftRate)
{
  logs(tl_both,"TerrainNavLcmClient::setVehicleDriftRate - initialized by LcmTrn\n");
  return;
}


/* Function: isConverged()
 * Usage: converged = tercom->isConverged();
 * ------------------------------------------------------------------------*/
bool TerrainNavLcmClient::isConverged()
{
  return true;
}


 /* Function: useLowGradeFilter()
 * Usage: tercom->useLowGradeFilter()
 * -------------------------------------------------------------------------*/
void TerrainNavLcmClient::useLowGradeFilter()
{
  logs(tl_both,"TerrainNavLcmClient::useLowGradeFilter - initialized by LcmTrn\n");
  return;
}
  

/* Function: useHighGradeFilter()
 * Usage: tercom->useHighGradeFilter()
 * -------------------------------------------------------------------------*/
void TerrainNavLcmClient::useHighGradeFilter()
{
  logs(tl_both,"TerrainNavLcmClient::useHighGradeFilter - initialized by LcmTrn\n");
  return;
}


/* Function: setFilterReinit(allow)
 * Usage: tercom->setFilterReinit(allow)
 * -------------------------------------------------------------------------*/
void TerrainNavLcmClient::setFilterReinit(const bool allow)
{
  logs(tl_both,"TerrainNavLcmClient::setFilterReinit - initialized by LcmTrn\n");
  return;
}

/* Function: setModifiedWeighting(use)
 * Usage: tercom->tNavFilter->setModifiedWeighting(use)
 * -------------------------------------------------------------------------*/
void TerrainNavLcmClient::setModifiedWeighting(const int use)
{
  logs(tl_both,"TerrainNavLcmClient::setModifiedWeighting - initialized by LcmTrn\n");
  return;
}


/* Function: getFilterState()
 * Usage: filterType = tercom->getFilterState();
 * ------------------------------------------------------------------------*/
int TerrainNavLcmClient::getFilterState()
{
  return 1;
}


 /* Function: getNumReinits()
 * Usage: filterType = tercom->getNumReinits();
 * ------------------------------------------------------------------------*/
int TerrainNavLcmClient::getNumReinits()
{
  return 0;
}

/* Function: reinitFilter(bool lowInfoTransition)
 * Usage: filterType = tercom->reinitFilter(true);
 * ------------------------------------------------------------------------*/
void TerrainNavLcmClient::reinitFilter(const bool lowInfoTransition)
{
  return;
}

bool TerrainNavLcmClient::is_connected()
{
  if (_lcm != NULL && _lcmTrn != NULL && _lcmTrn->good() &&
      _lcmc !=NULL && _trnc != NULL)
    return true;
  else
    return false;
}
