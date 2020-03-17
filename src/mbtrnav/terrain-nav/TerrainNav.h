/* FILENAME      : TerrainNav.h
 * AUTHOR        : Debbie Meduna
 * DATE          : 04/27/09
 * DESCRIPTION   : TerrainNav is used to extract terrain-correlation based
 *                 estimates of vehicle position and attitude for use in a
 *                 navigation system.  This class is an interface class between
 *                 the vehicle user and the terrain navigation filter classes.
 *
 * DEPENDENCIES  : structDefs.h, myOutput.h, TNavFilter.h, TNavPointMassFilter.h
 *                 TNavParticleFilter.h
 *
 * LAST MODIFIED : 11/30/10
 * MODIFIED BY   : Debbie Meduna
 * -----------------------------------------------------------------------------
 * Modification History:
 * -----------------------------------------------------------------------------
 *****************************************************************************/

#ifndef _TerrainNav_h
#define _TerrainNav_h

#include "structDefs.h"
#include "myOutput.h"
#include "trn_log.h"
#include "TNavFilter.h"
#include "TNavPointMassFilter.h"
#include "TNavParticleFilter.h"
#include "TNavBankFilter.h"
#include "TerrainMap.h"
#include "TerrainMapOctree.h"
#include "TerrainMapDEM.h"

#include <cmath>
#include <fstream>
#include <iomanip>

/*#include <stdlib.h>
#ifdef use_namespace
using namespace std;
#endif*/


#ifndef MEAS_BUFFER_SIZE     	//indicates number of meausurements that can
#define MEAS_BUFFER_SIZE 200 	//be stored for inclusion.
#endif

#ifndef MAX_INTERP_TIME      	//max time in seconds allowed between IMU data and
#define MAX_INTERP_TIME 5.0  	//measurement data for interpolation
#endif

#ifndef MAX_DRDT							//DVL measurement rejection heuristic
#define MAX_DRDT 10.0     		//Maximum allowable range rate for DVL (m/s)
#endif

#ifndef MAX_VEL								//DVL measurement rejection heuristic
#define MAX_VEL 2.0       		//Maximum allowable velocity (m/s)
#endif

#ifndef MAX_ACCEL							//DVL/INS measurement rejection heuristic
#define MAX_ACCEL 1.0     		//Maximum allowable acceleration (as measured by DVL)
#endif

#ifndef ALLOW_FILTER_REINIT
#define ALLOW_FILTER_REINIT 1  //Boolean indicating if filter should be allowed
#endif                         //to be reinitialized

#ifndef USE_MODIFIED_WEIGHTING
#define USE_MODIFIED_WEIGHTING 0  //boolean indicating if the modified weighting
#endif														//for flat areas should be used (currently only in PF)

#ifndef MAX_MEAS_OUTAGE
#define MAX_MEAS_OUTAGE 200.0	//Maximum allowable time of measurement outage
#endif                          //(in sec)

#ifndef MAX_VEL_OUTAGE
#define MAX_VEL_OUTAGE 100.0	//Maximum allowable time of bottom velocity
#endif                          //outage (in sec)

#ifndef MAX_FILTER_VAR     			//Variance trigger between TODO: ??? and ???
#define MAX_FILTER_VAR 0.0000001 //40000.0	//Maximum allowable x/y variance (m^2) // 1000.0 // 4000.0//
#endif

#ifndef MIN_FILTER_VAR   				//Variance trigger between TODO: ??? and ???
#define MIN_FILTER_VAR 0.000000001 //3.0 			//Minimum allowable x/y variance (m^2) //20.0
#endif

//TODO: Get rid of MIN_MEAS_VAR?  Not used for anything???
#ifndef MIN_MEAS_VAR
#define MIN_MEAS_VAR 0.0000000	//Minimum allowable measurement variance
#endif

#ifndef VAR_MARGIN							//Dead band on filter state changing (reduce excessive switching)
#define VAR_MARGIN 	0.000000000001//5.0//200.0 //100.0//	//Margin on measurement variance
#endif

//*** TODO: figure out right location for NIS parameters (NIS_WINDOW_LENGTH is used by the filter and
//is in genFilterDefs.h, MAX_NIS_VALUE is used by TerrainNav and is in TerrainNav.h)
#ifndef MAX_NIS_VALUE
#define MAX_NIS_VALUE 	1.4 //4.0//0.1		//Maximum allowable NIS Windowed Average
#endif

// Turn on V deltaT
#ifndef INTG_POS
#define INTG_POS 0          //Boolean to substitude integrated dvl velocity for the deltaPose.
#endif

//#ifndef NIS_WINDOW_LENGTH
//#define NIS_WINDOW_LENGTH 	20		//Length of NIS window
//#endif

class PositionLog;
class TerrainNavLog;

/*!
 * Class: TerrainNav
 *
 * This class is used as an interface between a user and the underlying
 * TNavFilter object which uses a terrain navigation algorithm to compute
 * pose estimates based on sonar and inertial sensor measurements.
 * This class's primary purpose is to appropriately handle incoming inertial
 * and sonar measurements, pass them to the underlying TNavFilter object, and
 * return pose estimates when requested by the user.
 *
 * Intended use:
 *
 *      Generate terrainNav object and estimate structures once at the start:
 *            TerrainNav *tercom = new TerrainNav(mapName, vehicleName,
 *                                               filterType);
 *	      poseT* tercomEst = new poseT;
 *            poseT* currInertialPose = new poseT;
 *            measT* currMeas = new measT;
 *
 *      Incorporate inertial sensor information as it comes in:
 *  	      tercom->motionUpdate(currInertialPose);
 *
 *      Incorporate sonar measurements as they come in:
 *            tercom->measUpdate(currMeas, dvl);
 *
 *      Ask for most recent terrain navigation pose estimate:
 *            tercom->estimatePose(tercomEst, mmse);
 *
 *      ***NAVIGATION AND MEASUREMENTS MUST BE ADDED SEQUENTIALLY IN TIME,
 *         OTHERWISE SOME MEASUREMENTS MAY BE SKIPPED***
 *

 *
 * Note: This class can handle various combinations of sonar and
 * inertial sensor data rates.  If the sonar and inertial sensor measurements
 * are out of sync, the inertial pose measurements are interpolated to provide
 * an estimate of the vehicle pose associated with each sonar measurement.
 * If the sonar data rate is faster than the inertial sensor data rate, sonar
 * measurements are collected and stored until the next inertial sensor
 * measurement comes in, at which point all collected measurements are added
 * into the navigation filter in sequence.
 */
class TerrainNav
{
 public:

    TerrainNav();

  /* Constructor: TerrainNav(mapName)
   * Usage: tercom = new TerrainNav("canyonmap");
   * -------------------------------------------------------------------------*/
  /*! Initializes a new TerrainNav with terrain map "mapName.txt". The mapping
   * AUV specs and the Point Mass Filter algorithm are used as defaults.
   */
  TerrainNav(char *mapName);


  /* Constructor: TerrainNav(mapName, vehicleName)
   * Usage: tercom = new TerrainNav("canyonmap", "mappingAuv");
   * -------------------------------------------------------------------------*/
  /*! Sets private variables mapFile = "mapName.txt", vehicleSpecFile =
   * "vehicleName.txt", and saveDirectory = NULL, default mapType = 1 (DEM).
   * Makes a call to createFilter(1) to initialize a new TerrainNav
   * object with default type = 1 (Point Mass Filter).
   */
  TerrainNav(char *mapName, char *vehicleSpecs);


  /* Constructor: TerrainNav(mapName, vehicleName, filterType)
   * Usage: tercom = new TerrainNav("canyonmap", "mappingAuv", 1);
   * -------------------------------------------------------------------------*/
  /*! Sets private variables mapFile = "mapName.txt", vehicleSpecFile =
   * "vehicleName.txt", and saveDirectory = NULL, default mapType = 1 (DEM).
   * Makes a call to createFilter(filterType) to initialize a new TerrainNav
   * object with specified type.
   */
  TerrainNav(char *mapName, char *vehicleSpecs,
	     const int &filterType);


  /* Constructor: TerrainNav(mapName, vehicleName, filterType, directory)
   * Usage: tercom = new TerrainNav("canyonmap", "mappingAuv", 1, "/home/");
   * -------------------------------------------------------------------------*/
  /*! Sets private variables mapFile = "mapName.txt", vehicleSpecFile =
   * "vehicleName.txt", and saveDirectory = "directory", default mapType = 1 (DEM).
   * Makes a call to createFilter(filterType) to initialize a new TerrainNav
   * object with specified type.
   */
  TerrainNav(char *mapName, char *vehicleSpecs,
	     const int &filterType, char *directory);

	 /* Constructor: TerrainNav(mapName, vehicleName, filterType, mapType)
   * Usage: tercom = new TerrainNav("canyonmap", "mappingAuv", 1, 1);
   * -------------------------------------------------------------------------*/
  /*! Sets private variables mapFile = "mapName.txt", vehicleSpecFile =
   * "vehicleName.txt", and mapType = mapType (1 is DEM, 2 is octree).
   * Makes a call to createFilter(filterType) to initialize a new TerrainNav
   * object with specified type.
   */
  TerrainNav(char *mapName, char *vehicleSpecs,
	     const int &filterType, const int &mapType);

	/* Constructor: TerrainNav(mapName, vehicleName, filterType, mapType, directory)
   * Usage: tercom = new TerrainNav("canyonmap", "mappingAuv", 1, 1, "/home/");
   * -------------------------------------------------------------------------*/
  /*! Sets private variables mapFile = "mapName.txt", vehicleSpecFile =
   * "vehicleName.txt", and saveDirectory = "directory", mapType = mapType
   * (1 is DEM, 2 is octree).
   * Makes a call to createFilter(filterType) to initialize a new TerrainNav
   * object with specified type.
   */
  TerrainNav(char *mapName, char *vehicleSpecs,
	     const int &filterType, const int &mapType, char *directory);

  /* Constructor: TerrainNav(map, vehicleSpecs, particles, filter, mapType, dir)
   * Usage: tercom = new TerrainNav("canyon", "mapping", "particles" 1, 1, "/log/");
   * -------------------------------------------------------------------------*/
  /*! Sets private variables mapFile = "mapName.txt", vehicleSpecFile =
   * "vehicleName.txt", particlesFile = "particles", and saveDirectory = "directory",
   * mapType = mapType
   * (1 is DEM, 2 is octree).
   * Makes a call to createFilter(filterType) to initialize a new TerrainNav
   * object with specified type.
   */
  TerrainNav(char* mapName, char* vehicleSpecs, char* particles,
             const int& filterType, const int& mapType, char* directory);

  /* Destructor: ~TerrainNav
   * Usage: delete tercom;
   * -------------------------------------------------------------------------*/
  /*! Frees all storage associated with the TerrainNav object.
   */
  virtual ~TerrainNav();


  /* Function: estimatePose
   * Usage: tercom->estimatePose(currEstimate, type);
   * -------------------------------------------------------------------------*/
  /*! This is the primary function in the TerrainNav class.  The function takes
   * in a pointer to a poseT which it fills with the terrain correlation pose
   * estimate based on previous measurements.  The time stamp of the pose
   * estimate indicates the last update time of the navigation filter.  Type
   * indicates the estimator type:
   * 1: Maximum Likelihood Estimate
   * 2: Minimum Mean Square Error Estimate.
   */
  virtual void estimatePose(poseT* estimate, const int &type);


  /* Function: measUpdate
   * Usage: tercom->measUpdate(currMeas, type);
   * -------------------------------------------------------------------------*/
  /*! Passes the current sonar measurement information along to the TNavFilter
   * object.  The function takes in a pointer to a measT containing the current
   * sonar measurement information. It also takes in an integer type variable
   * which relates the sonar measurement type as follows:
   * 1: DVL
   * 2: Multibeam
   * 3: Single Beam
   * 4: Homer Relative Measurement

   */
  virtual void measUpdate(measT* incomingMeas, const int &type);


  /* Function: mbUpdate
   * Usage: tercom->mbUpdate(currMeas, type);
   * -------------------------------------------------------------------------*/
  /*! Passes the current multibeam range information along to the TNavFilter
   * object.  The function takes in a pointer to a mbT containing the current
   * multibeam measurement ranges.

   */
  //virtual void mbUpdate(mbT* incomingMeas);


  /* Function: motionUpdate
   * Usage: tercom->motionUpdate(currEstimate);
   * -------------------------------------------------------------------------*/
  /*! Passes the current inertial measurement information along
   * to the TNavFilter object.  The function takes in a pointer to a poseT
   * containing the current inertial measurement information.
   */
  virtual void motionUpdate(poseT* incomingNav);


  /* Function: outstandingMeas
   * Usage: tercom->outstandingMeas;
   * -------------------------------------------------------------------------*/
  /*! Returns a boolean indicating if there are any measurements
   * not yet incorporated into the PDF and waiting for more recent inertial
   * measurement data.
   */
  virtual inline bool outstandingMeas(){return (this->numWaitingMeas > 0);}


  /* Function: lastMeasSuccessful
   * Usage: tercom->lastMeasSuccessful();
   * -------------------------------------------------------------------------*/
  /*! Returns a boolean indicating if the last sonar measurement
   * was successfully incorporated into the filter or not.
   */
  virtual inline bool lastMeasSuccessful(){return this->lastMeasSuccess;}


  /* Function: setInterpMeasAttitude()
   * Usage: tercom->setInterpMeasAttitude(1);
   * -------------------------------------------------------------------------*/
  /*! Sets a boolean indicating if the sonar measurement attitude
   * information should be determined from interpolated inertial poses.
   * Default = 0;
   */
  virtual inline void setInterpMeasAttitude(bool set){this->tNavFilter->setInterpMeasAttitude(set);}

  /* Function: setMapInterpMethod()
   * Usage: tercom->setMapInterpMethod(1);
   * -------------------------------------------------------------------------*/
  /*! Specifies the interpolation method to use for determining
   * inter-grid map depth values.
   * 0: nearest-neighbor (no interpolation)
   * 1: bilinear
   * 2: bicubic
   * 3: spline
   * Default = 0;
   */
  virtual inline void setMapInterpMethod(const int &type){this->tNavFilter->setMapInterpMethod(type);}


  /* Function: setVehicleDriftRate()
   * Usage: tercom->setVehicleDriftRate(5)
   * -------------------------------------------------------------------------*/
  /*! Sets the vehicle inertial drift rate parameter.  The default value is
   * determined by the vehicle specification sheet.  The driftRate parameter
   * is given in units of % drift in meters/sec.
   */
  virtual inline void setVehicleDriftRate(const double &driftRate){this->tNavFilter->setVehicleDriftRate(driftRate);}


  /* Function: isConverged()
   * Usage: converged = tercom->isConverged();
   * ------------------------------------------------------------------------*/
  /*! Returns a boolean indicating if the terrain navigation filter has
   * converged to an estimate.
   */
  virtual inline bool isConverged(){return this->tNavFilter->isConverged();}


  /* Function: getFilterType()
   * Usage: filterType = tercom->getFilterType();
   * ------------------------------------------------------------------------*/
  /*! Returns the integer filterType describing the current TNavFilter type.
   */
  virtual inline int getFilterType(){return this->filterType;}

//TODO: Currently not using this
   /* Function: useLowGradeFilter()
   * Usage: tercom->useLowGradeFilter()
   * -------------------------------------------------------------------------*/
  /*! Force filter settings for low grade system:
   * 7DOF system with ALLOW_ATTITUDE_SEARCH=1, DEAD_RECKON=1, SEARCH_GYRO=1
   */
  virtual inline void useLowGradeFilter(){this->tNavFilter->useLowGradeFilter();}


//TODO: Currently not using this, all done with ALLOW_ATTITUDE_SEARCH, DEAD_RECKON and SEARCH_GYRO
  /* Function: useHighGradeFilter()
   * Usage: tercom->useHighGradeFilter()
   * -------------------------------------------------------------------------*/
  /*! Force filter settings for low grade system:
   * 3DOF system with ALLOW_ATTITUDE_SEARCH=0, DEAD_RECKON=0, SEARCH_GYRO=0
   */
  virtual inline void useHighGradeFilter(){this->tNavFilter->useHighGradeFilter();}


  /* Function: setFilterReinit(allow)
   * Usage: tercom->setFilterReinit(allow)
   * -------------------------------------------------------------------------*/
  /*! Overwrite boolean allowFilterReinits with input argument, allow.
   */
  virtual inline void setFilterReinit(const bool allow){this->allowFilterReinits = allow;}

  /* Helper Function: reinitFilter
   * Usage: reinitFilter()
   * -------------------------------------------------------------------------*/
  /*! This function reinitializes the filter.  It will copy relevant information
   * from the current filter object, delete that object, then re-create the
   * filter.
   *
   * 23-Aug-2016 RGH : Exposing for use with TerrainAid and TerrainNavClient
   */
  //old state machine version: void reinitFilter(int newState, bool lowInfoTransition);
  void reinitFilter(bool lowInfoTransition);

	/* Function: setModifiedWeighting(use)
   * Usage: tercom->tNavFilter->setModifiedWeighting(use)
	 * tercom->tNavFilter->setModifiedWeighting(use) default setting is TRN_WT_NORM
   * from structDefs.h
   * -------------------------------------------------------------------------*/
  /*! Overwrite boolean useModifiedWeighting with input argument, use.
   */
  virtual inline void setModifiedWeighting(const int use)
  {
    useModifiedWeighting = use;    // cache the value for use in future reinits
    this->tNavFilter->setModifiedWeighting(use);
    logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TerrainNav::modified weighting set to %i\n",
      use);
  }


  //TODO delete this
  /* Function: getFilterState()
   * Usage: filterType = tercom->getFilterState();
   * ------------------------------------------------------------------------*/
  /*! Returns the integer filterState describing the current filter state.
   */
  virtual inline int getFilterState(){
	//old state machine version: return this->filterState;
	return 0;
  }


   /* Function: getNumReinits()
   * Usage: filterType = tercom->getNumReinits();
   * ------------------------------------------------------------------------*/
  /*! Returns the integer numReinits describing the number of reinitializations.
   */
  virtual inline int getNumReinits(){return this->numReinits;}

    virtual inline bool initialized() { return _initialized; }

  //Public structures and components of a TerrainNav object:
  /*********************************************************/

  /*! tNavFilter object used to recursively incorporate vehicle measurements
   according to a terrain navigation algorithm */
  TNavFilter* tNavFilter;

  virtual bool is_connected() { return true; }

  virtual void releaseMap()
  {
    if (this->terrainMap) delete this->terrainMap;
    this->terrainMap = NULL;
  }

 protected:

  /* Function: copyToLogDir() - copies configuration files to the log directory
   *                            for future reference
   * A log directory (saveDirectory) must be specified in the initialization
   * message, otherwise nothing is copied.
   */
  void copyToLogDir();

	/* Function: setOctreeMap()
   * RGH: Made protected to keep it out of the client-server protocol.
   * Usage: tercom->setOctreeMap();
   * -------------------------------------------------------------------------*/
  /*! Sets octreeMap in tNavFilter to oMap.
   */
  //virtual inline void setOctreeMap(pcl::octree::OctreePointCloudSearch<pcl::PointXYZ> *oMap){this->tNavFilter->setOctreeMap(oMap);}

  /* Helper Function: createFilter(filterType)
   * Usage: createFilter(1);
   * -------------------------------------------------------------------------*/
  /*! Initializes the terrain navigation filter object based on the specified
   * filter type:
   * 1: 3D Point Mass Filter (default - TODO:Make Other Filter Types work?)
   * 2: 8D Particle Filter
   * 3. 3D Extended Kalman Filter
   * 4. 8D Sigma Point (Unscented) Kalman Filter
   *
   * windowVar is used to size the initalization window for the new filter.
   *
   * If the filter object already exists, this function will delete the current
   * filter object and create a new one with stored saveDirectory,
   * vehicleSpecFile and mapFile.
   */
  void createFilter(const int filterType, const double *windowVar);


  /* Helper Function: initVariables()
   * Usage: initVariables();
   * -------------------------------------------------------------------------*/
  /*! Initializes private variables associated with a terrainNav object
   */
  void initVariables();


  /* Helper Function: attemptInitFilter()
   * Usage: attemptInitFilter(initEstimate);
   * -------------------------------------------------------------------------*/
  /*! Try to initialize the terrain navigation filter, TNavFilter, based on the
   * first inertial sensor measurement contained in the initEstimate poseT
   * structure. Initialization is only allowed if the following conditions are
   * met:
   * 1. Vehicle has bottom lock
   * 2. Vehicle has valid velocity measurements
   * 3. Vehicle is within a valid region of the terrain map
   * 4. Vehicle has valid sonar measurements
   * 5. Vehicle is below the surface
   */
  void attemptInitFilter(poseT& initEstimate);


  /* Helper Function: initMotion
   * Usage: initMotion(initEstimate);
   * -------------------------------------------------------------------------*/
  /*! Initializes the terrain navigation filter, TNavFilter, based on the
   * first inertial sensor measurement contained in the initEstimate poseT
   * structure.
   */
  void initMotion(poseT& initEstimate);


  /* Helper Function: interpolatePoses
   * Usage: interpolatePoses(pose1, pose2, newPose, t);
   * -------------------------------------------------------------------------*/
  /*! Linearly interpolates pose information from pose1 and pose2 to a new
   * pose at time newTime.  The interpolated pose is recorded in newPose.
   */
  bool interpolatePoses(const poseT& pose1, const poseT& pose2, poseT& newPose,
			const double newTime);


  /* Helper Function: computeMeasVariance
   * Usage: computeMeasVariance(currMeas);
   * -------------------------------------------------------------------------*/
  /*! Fills in the covariance array of the "currMeas" measT struct with a %
   * of the measured range for each beam.  If the covariance array is already
   * filled or the current measurement has no corresponding sensor type, this
   * function returns without making any changes.  Otherwise, the
   * percentRangeError property of the corresponding sensor is used to evaluate
   * the variance as follows:  sigma = percentRangeError*range;
   */
  void computeMeasVariance(measT& currMeas);


  /* Helper Function: checkVelocityValidity
   * Usage: checkVelocityValidity(currPose);
   * -------------------------------------------------------------------------*/
  /*! Check validity of DVL/INS velocity. If invalid, changes dvlValid flag in
   * currPose to false.  Validity check is based on the following:
   * 1. velocity is below specified MAX
   * 2. vehicle x-direction velocity is greater than zero
   *   (vehicle not stationary)
   * 3. estimated acceleration exceeds specified MAX_ACCEL
   */
  void checkVelocityValidity(poseT& currPose);


  /* Helper Function: checkRangeValidity
   * Usage: checkRangeValidity(currMeas);
   * -------------------------------------------------------------------------*/
  /*! Check validity of Measured ranges. If invalid, changes beamStatus flags in
   * currMeas to false.  Validity check is based on the following:
   * 1. NaN range values
   * 2. range falls within MIN_RANGE < r < MAX_RANGE boundaries
   * 3. dr/dt of an individual beam exceeds MAX_DRDT
   * 4. all but one range measurement are equal
   */
  void checkRangeValidity(measT& currMeas);

  /* Helper Function: checkFilterHealth
   * Usage: checkFilterHealth()
   * -------------------------------------------------------------------------*/
  /*! This function checks the health of the filter and calls the reinitFilter()
   * function if necessary.  This checks the health by performing the following
   * checks:
   *	a. Check if the length of time since the last successful measurement
   *		exceeds a set maximum. (uses lastMeasSuccessTime)
   *	b. Check if the x/y uncertainty of the filter exceeds a set maximum.
 	 *		(uses currVar)
   */
   bool checkFilterHealth();


  //Protected structures and components of a TerrainNav object:
  /*********************************************************/

  //!static array containing all unincorporated sonar measurements
  measT waitingMeas[MEAS_BUFFER_SIZE];

  //!integer indicating the current number of unincorporated sonar measurements
  int numWaitingMeas;

  /*!boolean indicating if the previous sonar measurement was successfully
    incoporated into the navigation filter*/
  bool lastMeasSuccess;

  /*!array containing the last good velocity measurements and bottom-lock status
    / Velocity is given in the body frame*/
  double lastValidVel[3];
  bool lastVelBotLock;

  //!status of last measurement - only used for testing initialization
  //TODO: why are these limited to 4 beams?
  bool lastMeasValid;
  double lastValidRange[4];
  double lastValidRangeTime[4];
  bool noValidRange[4];

  //!last time associated with successful measurement update
  double lastMeasSuccessTime;

  //!time of last filter init attempt
  double lastInitAttemptTime;

  //!time of last bottom lock velocity
  double lastBottomLockTime;

  //!last valid estimated navigation offset
  poseT estNavOffset;

  //!boolean indicating if filter reinitializations are allowed.
  //Initialized as TRUE but can be set with setFilterReinit
  bool allowFilterReinits;

  //!boolean indicating if modified weighting should be used.
  //Initialized as FALSE but can be set using setModifiedWeighting
  int useModifiedWeighting;

  //!counter for number of filter reinitializations
  int numReinits;

  //file paths for map file, vehicle specs file, and save directory
  char* saveDirectory;
  char* vehicleSpecFile;
  char* particlesFile;
  char* mapFile;

  //!type of filter
  int filterType;

  //!type of map
  int mapType;

  //!terrainMap object containing information about current map being used
  TerrainMap* terrainMap;

  //TODO delete this
  //!current state of filter
  //old state machine declaration: int filterState;

  //!pcl pointcloud object containing current map when using octree
  //pcl::octree::OctreePointCloudSearch<pcl::PointXYZ> *octreeMap;

	//!boolean indiating true if initialized successfully (i.e., the mapfile was found)
  bool _initialized;

  poseT _incomingNav;  // maintain the incoming poseT object, not a pointer

  // log files
  TerrainNavLog *_trnLog;

};

#endif
