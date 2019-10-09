/* FILENAME      : TNavFilter.h
 * AUTHOR        : Debbie Meduna
 * DATE          : 04/24/09
 * DESCRIPTION   : TNavFilter uses a terrain navigation filter to propagate 
 *                 terrain-correlation based vehicle state estimates. It is a 
 *                 container class for various terrain navigation filter 
 *                 variants.
 * DEPENDENCIES  : TerrainMap.h, matrixArrayCalcs.h, structDefs.h, myOutput.h, 
 *                 newmat*.h
 * 
 * LAST MODIFIED : 5/7/13
 * MODIFIED BY   : Shandor Dektor
 * -----------------------------------------------------------------------------
 * Modification History:
 * -----------------------------------------------------------------------------
 *****************************************************************************/

#ifndef _TNavFilter_h
#define _TNavFilter_h

#include "TerrainMap.h"
#include "matrixArrayCalcs.h"
#include "genFilterDefs.h"
#include "structDefs.h"
#include "myOutput.h"

#include <newmatap.h>
#include <newmatio.h>

#include <math.h>
#include <fstream>
#include <iomanip>

#ifdef USE_MATLAB
#include "engine.h"
#endif

//!compassBiasT struct is used to contain information on the heading-dependent
//!compass bias.

//TODO: Delete this?  Will we ever use compassBiasT again???
struct compassBiasT{
  double* cosineCoeff;
  double* sineCoeff;
  double constCoeff;
  int seriesOrder;
  compassBiasT()
  {
    seriesOrder = 3;
    cosineCoeff = new double[seriesOrder];
    sineCoeff = new double[seriesOrder];
    if(seriesOrder == 2)
      {
	constCoeff = -0.00280307588;
	cosineCoeff[0] = 0.020827917;
	sineCoeff[0] = 0.30351486302;
	cosineCoeff[1] = 0.03350770889;
	sineCoeff[1] = -0.00226968614376;   
      }
    else
      {
	constCoeff = -0.002800887506;
	cosineCoeff[0] = 0.0192245946634;
	sineCoeff[0] = 0.3049004455222;
	cosineCoeff[1] = 0.033519791431345;
	sineCoeff[1] = -0.0022754105288;
	cosineCoeff[2] = -0.0035822997664;
	sineCoeff[2] = -0.0054575460693;
      }
  }

  ~compassBiasT()
  {
    delete [] cosineCoeff;
    delete [] sineCoeff;
  }
  
  double evalCompassBias(double psi)
  {
    double psi_bias = constCoeff;
    int i;
    
    for(i = 0; i < seriesOrder; i++)
      psi_bias += cosineCoeff[i]*cos((i+1)*psi) + sineCoeff[i]*sin((i+1)*psi);
    
    return psi_bias;
  };

};


//TODO: Do we need corrT?  This is used in PMF, maybe get rid of it if we change the structure?

//!a correlation element containing the projected x, y and z components of a 
//!sonar range measurement, given from the center of the vehicle frame.
struct corrT {
  double dx, dy, dz;
  double var;
};

/*!
 * Class: TNavFilter
 * 
 * This class is used to compute and propagate terrain navigation pose 
 * estimates based on vehicle sonar and inertial sensor measurements.  The class
 * is a container class for various specific terrain navigation filter 
 * implementations. 
 *
 * The TNavFilter class is the primary component of a TerrainNav object. It 
 * contains a TerrainMap object for storing information relating to the 
 * reference terrain map.  The class also contains basic functions
 * common to any terrain navigation filter implementation including sonar
 * projection functions and map extraction and interpolation functions.
 *
 * Virtual functions are also defined which must be included in any inheritance
 * class of the TNavFilter class.  
 *
 * Intended use:
 *       Define a TNavFilter pointer and initialize it to a particular
 *       inheritance class filter object.
 *               TNavFilter *tNavFilter;
 *               tNavFilter = new TNavSpecificFilter();
 *
 *       Add sonar and inertial measurements as they come in
 *               tNavFilter->measUpdate(currMeas);
 *               tNavFilter->motionUpdate(currNavPose);
 *
 *       Compute filter pose estimates
 *               tNavFilter->computeMLE(mlePose)
 *               tNavFilter->computeMMSE(mmsePose)
 */

class TNavFilter
{
 public:

  /* Constructor: TNavFilter(mapName, vehicleName, directory, windowVar, mapType)
   * Usage: tnavFilter = new TNavFilter("canyonmap", "mappingAuv", "dir", windowVar, 1);
   * -------------------------------------------------------------------------*/
  /*! Initializes a new TNavFilter object with terrain map "mapName.txt", 
   * vehicle specs contained in "vehicleName.txt", and file save directory
   * given by "directory", uses windowVar to initialize filter window, uses mapType
   * to choose which type of map to use (1=DEM, 2=octree). 
   */
  //TNavFilter(char *mapName, char *vehicleSpecs, char *directory, const double *windowVar, const int &mapType);//Reload Map Issue
  TNavFilter(TerrainMap* terrainMap, char *vehicleSpecs, char *directory, const double *windowVar, const int &mapType);

  /* Destructor: ~TNavFilter()
   * Usage: delete tnavFilter;
   * -------------------------------------------------------------------------*/
  /*! Frees all storage associated with the TNavFilter object.
   */
  virtual ~TNavFilter();


  /* Function: setMapInterpMethod()
   * Usage: tercom->setMapInterpMethod(1);
   * -------------------------------------------------------------------------*/
  /*! Specifies interpolation method to use for inter-grid map depths.
   * 0: nearest-neighbor (no interp.)
   * 1: bilinear
   * 2: bicubic
   * 3: spline
   * Default = 0;
   */
  inline void setMapInterpMethod(const int &type){terrainMap->setMapInterpMethod(type);}
	
  int GetInterpMapMethod(void){ return terrainMap->GetInterpMethod(); }
  //TODO comment

  
  /* Function: setInterpMeasAttitude()
   * Usage: tercom->setInterpMeasAttitude(1);
   * -------------------------------------------------------------------------*/
  /*! Sets a boolean indicating if the sonar measurement attitude should be 
   * determined from interpolated inertial poses.
   * Default = 0;
   */
  inline void setInterpMeasAttitude(bool set){this->interpMeasAttitude = set;}

	/* Function: setOctreeMap()
   * Usage: tercom->setOctreeMap();
   * -------------------------------------------------------------------------*/
  /*! Sets octreeMap in tNavFilter to oMap.
   */
  //inline void setOctreeMap(pcl::octree::OctreePointCloudSearch<pcl::PointXYZ> *oMap){this->octreeMap = oMap;}

  /* Function: setVehicleDriftRate()
   * Usage: tercom->setVehicleDriftRate(5)
   * -------------------------------------------------------------------------*/
  /*! Sets the vehicle inertial drift rate parameter.  The default value is
   * determined by the vehicle specification sheet. The driftRate parameter
   * is given in units of % drift in meters/sec.
   */
  inline void setVehicleDriftRate(const double &driftRate){this->vehicle->driftRate = driftRate;}


  /* Function: setInitDistribType()
   * Usage: tercom->setInitDistribType(0)
   * -------------------------------------------------------------------------*/
  /*! Sets the initial distribution type:
   * 0: Uniform
   * 1: Gaussian
   * The default value is 0.
   */
  inline void setInitDistribType(const int &distType){this->initDistribType = distType;}
  

  /* Function: setLowResMap()
   * Usage: tercom->setLowResMap(mapName)
   * -------------------------------------------------------------------------*/
  /*! Load a mapsrc pointer for the low resolution map given by "mapName" into
   * the terrainMap object.
	 *  IF A LOW-RES MAP IS LOADED, IT WILL BE USED WHEN NOT ON THE HIGH-RES MAP 	 
   */
  inline void setLowResMap(char* mapName){this->terrainMap->setLowResMap(mapName);}


	//TODO: Rename Dead Reckon?  What does it specifically mean?
  /* Function: useLowGradeFilter()
   * Usage: tercom->useLowGradeFilter()
   * -------------------------------------------------------------------------*/
  /*! Force filter settings for low grade system:
   * 7DOF system with ALLOW_ATTITUDE_SEARCH=1, DEAD_RECKON=1, SEARCH_GYRO_BIAS=1
   */
  inline void useLowGradeFilter(){this->forceLowGradeFilter=true;this->forceHighGradeFilter=false;}


  /* Function: useHighGradeFilter()
   * Usage: tercom->useHighGradeFilter()
   * -------------------------------------------------------------------------*/
  /*! Force filter settings for low grade system:
   * 3DOF system with ALLOW_ATTITUDE_SEARCH=0, DEAD_RECKON=0, SEARCH_GYRO=0
   */
  inline void useHighGradeFilter(){this->forceHighGradeFilter=true;this->forceLowGradeFilter=false;}

	/* Function: setModifiedWeighting
	 * Usage: tercom->setModifiedWeighting(use)
   * -------------------------------------------------------------------------*/
	/*! Set filter to use modified weighting scheme, which prevents the filter from
	 * becoming overconfident in low information terrain by downweighting new
	 * measurements.
         * Updated to decode umw value and bank filter parameters in a single integer value.
         * The default method here is to just decode the umw code (the ones digit).
         * 
	 */
  virtual void setModifiedWeighting(const int use){this->useModifiedWeighting = use % 10;}

  /* Function: withinRefMap
   * Usage: terrainMap->withinRefMap();
   * ------------------------------------------------------------------------*/
  /*! Returns a boolean indicating if the vehicle is within the reference 
   * terrain map according to the most recent inertial navigation readings.
   */
  bool withinRefMap();


  /* Function: withinValidMapRegion
   * Usage: tNavFilter->withinValidMapRegion(x, y);
   * ------------------------------------------------------------------------*/
  /*! Indicates if given location is within a valid portion of the provided
   * reference terrain map (i.e., a non-NaN location).  Examines region
   * around the point to make sure it is well on the map.
   * The size of the region is dependent on the size of the initialization window
   */
  bool withinValidMapRegion(const double& north, const double& east);


	/* Function: withinValidMapRegionPoint
   * Usage: tNavFilter->withinValidMapRegionPoint(x, y);
   * ------------------------------------------------------------------------*/
  /*! Indicates if given location is within a valid portion of the provided
   * reference terrain map (i.e., a non-NaN location).
   */
  bool withinValidMapRegionPoint(const double& north, const double& east);


	//TODO: Make this do something??  maybe have it call checkConvergence?
  /* Function: isConverged()
   * Usage: converged = tercom->isConverged();
   * ------------------------------------------------------------------------*/
  /*! Returns a boolean indicating if the terrain navigation filter has 
   * converged to an estimate.
   */
  bool isConverged();


	//TODO: This will have to change if the measurement input structure is changed
  //eg to a more general range, rotation matrix/direction and covariance format

  /* Function: findMeasSensorIndex
   * Usage: found = findMeasSensorIndex(measType, idx);
   * -------------------------------------------------------------------------*/
  /*! Determines the associated vehicle sensor for the current measurement and
   * stores its index in the vehicle sensor array into "idx".  If no 
   * corresponding sensor can be found, the function returns false.  Otherwise,
   * it returns true.
   */
  bool findMeasSensorIndex(int measType, int &sensorIndex);


  /* Helper Function: applyRotation
   * Usage: beamsMF = applyRotation(attitude, beamsVF);
   * -------------------------------------------------------------------------*/
  /*! Transforms position vectors from vehicle frame to map frame, using the 
   * given vehicle attitude angles.  The vehicle frame position vectors are
   * given in Matrix format, with each vector representating a column of 
   * beamsVF.
   */
  Matrix applyRotation(const double* attitude,  const Matrix &beamsVF);


	//TODO: Get rid of this if we get rid of mainPlot
  //      Or should be renamed getDEM or be overloaded with the map type?
  //			Also, figure out how to plot octrees (*cough*, David)
  /* Function: getTerrainMap
   * Usage: success = getTerrainMap(currMap);
   * ------------------------------------------------------------------------*/
  /*! Extracts the current map stored in the TerrainMap object.  Returns a bool
   * indicating if the map was successfully accessed.
   */
  bool getTerrainMap(mapT &currMap);


	//TODO: Get rid of this if we get rid of mainPlot
  /* Function: getTerrainMapBounds
   * Usage: success = getTerrainMapBounds();
   * ------------------------------------------------------------------------*/
  /*! Extracts the bounds of the current map stored in the TerrainMap object.  
   * Returns a bool indicating if the map was successfully accessed.
   * Format of returned double is [minX, maxX, minY, maxY].
   */
  bool getTerrainMapBounds(double *currMapBounds);


  /* Function: increaseInitSearchWin
   * Usage: success = increaseInitSearchWin(windowVarIncrement);
   * ------------------------------------------------------------------------*/
  /*! Increases the initial search window for the filter by the given increment.
   */
  void increaseInitSearchWin(double *windowVarIncrement);

  unsigned int setDistribToSave(unsigned int distrib);

  /* Virtual functions required by any inheritance class:
   * ----------------------------------------------------*/
  //!initFilter(initNavPose): initializes terrain navigation filter
  virtual void initFilter(poseT& initNavPose) = 0;

  /*!success = measUpdate(currMeas): incorporate current sonar measurement 
   *                                 into the navigation filter and return a
   *                                 boolean indicating if this was successful*/
  virtual bool measUpdate(measT& currMeas) = 0;

  /*! motionUpdate(currNavPose): incorporate current. inertial pose measurment
   *                             into the navigation filter */
  virtual void motionUpdate(poseT& currNavPose) = 0;

  //!computeMLE(estimate): compute maximum likelihood pose estimate
  virtual void computeMLE(poseT* estimate) = 0;

  //!computeMMSE(estimate): compute minimum mean square error pose estimate
  virtual void computeMMSE(poseT* estimate) = 0;

	//TODO: keep this, or axe it?
  //!checkConvergence(): check if the filter has successfully converged
  virtual void checkConvergence() = 0;

  //!saveCurrDistrib(file): write current filter information to file
  virtual void saveCurrDistrib(ofstream &outputFile) = 0;

  
  virtual double GetMapResolutionX(void){return this->terrainMap->Getdx();}
  virtual double GetMapResolutionY(void){return this->terrainMap->Getdy();}

  //Public structures and components of a TNavFilter object:
  /*********************************************************/

  //!poseT indicating the last incorporated inertial navigation pose
  poseT* lastNavPose;

  //!vehicle object containing sensor information specific to vehicle
  vehicleT* vehicle;  
  
	//Ax this if we don't and won't ever need it
  //!compass bias struct
  compassBiasT* compassBias; 

  //TODO: Check if this is time OF last valid dvl velocity data
  //!time since last valid dvl velocity data
  double timeLastDvlValid;

  //!Windowed average of the normalized measurement innovations
  double windowedNIS;

  //!Normalized measurement innovation log
  double windowedNISlog[NIS_WINDOW_LENGTH];

  //! NIS alternative
  double SubcloudNIS;
  
  //!estimate of water current velocity
  double currentVel[3];
  
  //!most recently calculated x and y variance of the filter
  double currVar[2];
  
  
  //!boolean indicating if measurement attitude should be found from pose 
  //!interpolation
  bool interpMeasAttitude;
  
  //!measurement variance
  double measVariance;

  //!total amount of time in attempting to initialize filter
  double totalAttemptTime;

  //!force filter to use MAUV (high-grade-vehicle) settings
  bool forceHighGradeFilter;
  //!force filter to use BIAUV (low-grade-vehicle) settings
  bool forceLowGradeFilter;
  
  //!indicator if modified weighting should be used. 
  //!Initialized to TRN_WT_NORM from structDefs.h
  //!under normal circumstances.
  int  useModifiedWeighting;

  
 protected:

  /* Helper Function: initVariables
   * Usage: initVariables();
   * -------------------------------------------------------------------------*/
  /*! Intitializes the value of TNavFilter public and protected variables not 
   * set in the constructor.
   */
  void initVariables();


  /* Helper Function: projectMeasVF
   * Usage: projectMeasVF(beamsVF, currMeas, beamIndices);
   * -------------------------------------------------------------------------*/
  /*! Projects current measurement into the vehicle frame. This projection is
   * contained in the 3xN maxtrix beamsVF, where N is the number of individual
   * sonar beams which are valid for the current measurement.  The correspondance 
   * between beamsVF index and currMeas index is stored in beamIndices.
   * Returns a boolean indicating if the projection resulted in at least one
   * valid sonar beam. Calls projectMeasSF() as part of this routine.
   */
  bool projectMeasVF(Matrix &beamsVF, const measT& currMeas, int* beamIndices);


  /* Helper Function: projectMeasSF
   * Usage: projectMeasSF(beamsSF, currMeas);
   * -------------------------------------------------------------------------*/
  /*! Projects current measurement into the sensor frame. This projection is
   * contained in the 3xN maxtrix beamsSF, where N is the number of individual
   * sonar beams which are valid for the current measurement.  The correspondance 
   * between beamsVF index and currMeas index is stored in beamIndices.
   * Returns a boolean indicating if the projection resulted in at least one
   * valid sonar beam.
   */
  bool projectMeasSF(Matrix &beamsSF, const measT& currMeas, int* beamIndices);


  /* Helper Function: getRotMatrix
   * Usage: R = getRotMatrix(attitude);
   * -------------------------------------------------------------------------*/
  /*! Computes and returns the rotation matrix based on the vehicle attitude 
   * angles provided.  
   */
  Matrix getRotMatrix(const double* attitude);


  //TODO: Either use this or get rid of it... this feels like the right way as 
  // it reduces the number of sin and cosine computations
  /* Helper Function: applyDVLRotation
   * Usage: beamsMF = applyDVLRotation(beamsSF);
   * -------------------------------------------------------------------------*/
  /*! Transforms position vectors from DVL sensor frame to vehicle frame, using 
   * the dvlRotMatrix variable stored in the TNavFilter object.  The vehicle 
   * frame position vectors are given in Matrix format, with each vector 
   * representing a column of beamsSF.
   */
  Matrix applyDVLrotation(const Matrix &beamsSF);

	//TODO: This function does not seem to be used. Eliminate?
  /* Helper Function: modifyBeamDir
   * Usage: beamsMF = modifyBeamDir(beamsMF, vehPose);
   * -------------------------------------------------------------------------*/
  /*! This function takes in Map-Frame beam vectors (beamsMF) and modifies them
   * to correspond with the closest possible beam return within the beam's 
   * measurement cone.  This calculation is done by assuming a planar terrain
   * in the vincinity of the projected beam location.  The vehicle pose is also
   * needed as input to this function, but is not modified.
   */
  void modifyBeamDir(Matrix &beamsMF, const double* vehPose);
  
	//TODO: This function does not seem to be used. Eliminate?
  /* Helper Function: modifyBeamDirExtensive
   * Usage: beamsMF = modifyBeamDirExtensive(beamsMF, vehPose);
   * -------------------------------------------------------------------------*/
  /*! This function takes in Map-Frame beam vectors (beamsMF) and modifies them
   * to correspond with the closest possible beam return within the beam's 
   * measurement cone.  This calculation is done by computing the distance
   * to all points in the currently extracted map and finding the closest
   * point within the beam cone. The total range of the beam vectors is 
   * preserved. The vehicle pose is also needed as input to this function, 
   * but is not modified.
   */
  void modifyBeamDirExtensive(Matrix &beamsMF, const double* vehPose);

	

  
  /* Usage: calculateNIS(measCov, meanDiff, nisVal, currMeas, beamIndices)
   * -------------------------------------------------------------------------*/
  /*! This function is used to calculate the Normalized Innovations Squared for 
   * a given covariance matrix (measCov) and the mean of the expected 
   * measurement difference (meanDiff). The NIS value is returned in nisVal
   */
  void calculateNIS(SymmetricMatrix &measCov, ColumnVector &meanDiff, double &nisVal, const measT &currMeas, int* beamIndices);


  /* Usage: updateNISwindow(double& nisVal)
   * -------------------------------------------------------------------------*/
  /*! This function updates windowedNISlog and windowedNIS with the most 
   * recent NIS value
   */
  void updateNISwindow(double& nisVal);

  //Protected structures and components of a TNavFilter object:
  /************************************************************/

	//TODO: This is what should be checked/assigned/whatever by checkConvergence
  //!boolean indicating if the navigation filter has converged
  bool converged;

  //!double array containing the dvl sensor attitude 
  double* dvlAttitude;

  //!rotation matrix containing dvl sensor attitude rotation matrix
  Matrix dvlRotMatrix;

  //!char array indicating directory in which to save files
  char* saveDirectory;

  //!terrainMap object containing information about current map being used
  TerrainMap* terrainMap;

	
  //!initialization distribution type (0: Uniform, 1: Gaussian)
  int initDistribType;

  //!window initialization variance
  double initWindowVar[N_COVAR];
  
  //!map type (1: DEM, 2: octree)
  int mapType;

  unsigned int _distribType;

/*#ifdef USE_MATLAB
  //!Matlab engine for debug mode
  Engine* matlabEng;
#endif*/

 private:
  //There are no private variables or functions for the TNavFilter class 


};

#endif
