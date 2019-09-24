/* FILENAME      : TNavSigmaPointFilter.h
 * AUTHOR        : Debbie Meduna
 * DATE          : 08/06/09
 * DESCRIPTION   : TNavSigmaPointFilter is an inheritance class of TNavFilter.h 
 *                which uses a Sigma Point Kalman Filter(SPKF) implementation of
 *                the terrain navigation filter. The primary data structures are
 *                (1) a Matrix containing the covariance, (2) a ColumnVector 
 *                containing the mean, and (3) an array of sigma points.  The
 *                filter has the ability to estimate 8 states - full 3D position
 *                ,full 3D attitude, and 2 gryo biases.
 *
 * DEPENDENCIES  : TNavFilter.h, CommonDefs.h, matrixArrayCalcs.h, sensCalcs.h,
 *                 structDefs.h, myOutput.h, TerrainMap.h, newmat*.h
 *
 * LAST MODIFIED : 05/20/10 
 * MODIFIED BY   : Debbie Meduna   
 * ----------------------------------------------------------------------------
 * Modification History
 * ----------------------------------------------------------------------------
 *****************************************************************************/


#ifndef _TNavSigmaPointFilter_h
#define _TNavSigmaPointFilter_h

#include "TNavFilter.h"
#include "matrixArrayCalcs.h"
#include "genFilterDefs.h"
#include "structDefs.h"
#include "myOutput.h"
#include "TerrainMap.h"

#include <newmatap.h>
#include <newmatio.h>

#include <math.h>
#include <fstream>
#include <iomanip>
#include <iostream>


/*******************************************************************************
 * Sigma Point Filter Specific Parameters
 ******************************************************************************/
#ifndef SIGMA_FACTOR
#define SIGMA_FACTOR 5.0
#endif

#ifndef AUGMENT_STATE
#define AUGMENT_STATE 0
#endif

#ifndef ALPHA
#define ALPHA 0.2
#endif

#ifndef BETA
#define BETA  2.0
#endif

#ifndef KAPPA
#define KAPPA 0
#endif

/*! Sigma point structure.  Contains pose and attitude states, along with gyro
  biases if being searched over*/
struct sigmaPointT {
  ColumnVector pose;
  double weight_m;
  double weight_c;

  void displaySigmaPointInfo()
    {
      int i;
      
      logs(TL_OMASK(TL_TNAV_SIGMA_POINT_FILTER, TL_LOG),"Motion Weight: %f\n", weight_c);
      logs(TL_OMASK(TL_TNAV_SIGMA_POINT_FILTER, TL_LOG),"Meas Weight: %f\n", weight_m);
      logs(TL_OMASK(TL_TNAV_SIGMA_POINT_FILTER, TL_LOG),"Position (N,E,D): ");
      for(i = 0; i < 3; i++)
	logs(TL_OMASK(TL_TNAV_SIGMA_POINT_FILTER, TL_LOG),"%.3f\t", pose(i+1));
      logs(TL_OMASK(TL_TNAV_SIGMA_POINT_FILTER, TL_LOG),"\nAttitude in degrees (phi,theta,psi): ");
      for(i = 3; i < 6; i++)
	logs(TL_OMASK(TL_TNAV_SIGMA_POINT_FILTER, TL_LOG),"%.2f\t", pose(i+1)*180.0/PI);
      logs(TL_OMASK(TL_TNAV_SIGMA_POINT_FILTER, TL_LOG),"\nGyro Bias (^o/sec) in y,z: ");
      for(i = 6; i < 8; i++)
	logs(TL_OMASK(TL_TNAV_SIGMA_POINT_FILTER, TL_LOG),"%f\t", pose(i+1)*180.0/PI);
      logs(TL_OMASK(TL_TNAV_SIGMA_POINT_FILTER, TL_LOG),"\n");
    }
};

/*!
 * Class: TNavSigmaPointFilter
 * 
 * This class inherits from the TNavFilter class. It uses an extended kalman
 * filter implementation to compute and propagate terrain navigation pose 
 * estimates based on vehicle sonar and inertial measurements.  The filter 
 * maintains two states: vehicle Northing and vehicle Easting.
 *
 * Intended use:
 *       Initalize the TNavSigmaPointFilter object
 *               TNavFilter *tNavFilter;
 *               tNavFilter = new TNavSigmaPointFilter();
 *
 *       Add sonar and inertial measurements as they come in
 *               tNavFilter->measUpdate(currMeas);
 *               tNavFilter->motionUpdate(currNavPose);
 *
 *       Compute filter pose estimates
 *               tNavFilter->computeMLE(mlePose)
 *               tNavFilter->computeMMSE(mmsePose)
 */
class TNavSigmaPointFilter : public TNavFilter
{

 public:

  /* Constructor: TNavSigmaPointFilter(mapName, vehicleSpecs, directory)
   * Usage: trnFilter = new TNavSigmaPointFilter("mapName", "vehicle", "dir");
   * -------------------------------------------------------------------------*/
  /*! Initializes a new TNavSigmaPointFilter object with TerrainMap given by 
   * mapName.txt, vehicle specifications given by vehicle.txt, and file save
   * directory given by dir, uses windowVar to initialize filter window.
   */
  TNavSigmaPointFilter(char *mapName, char *vehicleSpecs,
		      char *directory, const double *windowVar);


  /* Destructor: ~TNavSigmaPointFilter()
   * Usage: delete trnFilter;
   * -------------------------------------------------------------------------*/
  /*! Frees all storage associated with the TNavSigmaPointFilter object.
   */
  virtual ~TNavSigmaPointFilter();


  /* Function: initFilter
   * Usage: initFilter(currNavPose);
   * -------------------------------------------------------------------------*/
  /*! Initializes the filter components. 
   */
  void initFilter(poseT& initNavPose);

 
  /* Function: measUpdate
   * Usage: 1 = measUpdate(currMeas);
   * -------------------------------------------------------------------------*/
  /*! Incorporates the current measurement information into priorPDF. 
   * Returns a boolean indicating if the measurement was successfully added.
   */
  bool measUpdate(measT& currMeas);


  /* Function: motionUpdate
   * Usage: tercom->motionUpdate(currNavPose);
   * -------------------------------------------------------------------------*/
  /*! Updates the center of priorPDF to the navigation data in currNavPose.
   * Performs a convolution time update based on the time elapsed between 
   * currNavPose and the stored lastNavPose. 
   */
  void motionUpdate(poseT& currNavPose);


  /* Function: computeMLE
   * Usage: computeMLE(mleEstimate);
   * -------------------------------------------------------------------------*/
  /*! Computes and returns a pose estimate based on maximum likelihood 
   * estimation.
   */
  void computeMLE(poseT* mlePose);


  /* Function: computeMMSE
   * Usage: computeMMSE(mleEstimate);
   * -------------------------------------------------------------------------*/
  /*! Computes and returns a pose estimate based on minimum mean square 
   * estimation - i.e., the probabilistic mean of the data.  This function also
   * computes the covariance of the likelihood surface about the MMSE estimate.
   */
  void computeMMSE(poseT* mmsePose);


  /* Function: checkConvergence()
   * Usage: checkConvergence()
   * -------------------------------------------------------------------------*/
  /*! Checks if the current TRN filter has converged.  For the SPKF, the filter
   * is always converged.
   */
  void checkConvergence();


  /* Function: saveCurrDistrib()
   * Usage: saveCurrDistrib(file)
   * -------------------------------------------------------------------------*/
  /*! Saves the current TRN filter information to the specified output file. For
   * the SPKF, this corresponds to the current mean vector and Sigma matrix, as
   * well as the current sigma points:
   *          mu | Sigma
   *         --------------------------------
   *          weight_m(1) weight_c(1) pose(1) attitude(1) gyroBias(1)
   *            .         .           .       .           .
   *            .         .           .       .           . 
   *          weight_m(n) weight_c(n) pose(n) attitude(n) gyroBias(n)
   */
  void saveCurrDistrib(ofstream &outputFile);
  

 private:
  /* Function: initVariables()
   * Usage: initVariables()
   * ------------------------------------------------------------------------*/
  /*! Initialize number of states and other private variables.
   */
  void initVariables();


  /* Function: initSigmaPoints()
   * Usage: initSigmaPoints();
   * -------------------------------------------------------------------------*/
  /*! Initializes the sigma points using the initial mean and Sigma.
   */
  void initSigmaPoints();


  /* Function: updateSigmaPointPoses()
   * Usage: updateSigmaPointPoses();
   * -------------------------------------------------------------------------*/
  /*! Updates the sigma points using the current mean and Sigma.
   */
  void updateSigmaPointPoses();


  /* Function: incorporateMeas()
   * Usage: incorporateMeas()
   * -------------------------------------------------------------------------*/
  /*! Incorporates the current measurements stored in corrData by computing the
   * Kalman matrix and measurement innovation for each sigma point.
   * Returns a boolean indicating if the measurement was successfully added 
   * (will be successful unless no valid terrain map information is available
   * for the current measurement).
   */
  bool incorporateMeas(Matrix& beamsVF, measT& currMeas);


  /* Function: defineAndLoadSubMap(beamsVF)
   * Usage: defineAndLoadSubMap(beamsVF)
   * -------------------------------------------------------------------------*/
  /*! This function loads a map segment from terrainMap->refMap which covers the
   * relevant correlation area.  The bounds of this map are determined by taking
   * the maximum sigma point bounds around the current pose estimate and adding
   * onto that the expected maximum beam projections in North and East.
   */
  int defineAndLoadSubMap(const Matrix &beamsVF);
  

  /* Function: computeMotionSigmaAugmentState(diffPose)
   * Usage: R = computeMotionSigmaAugmentState(diffPose)
   * -------------------------------------------------------------------------*/
  /*! Computes the symmetric motion noise covariance matrix for the augmented 
   * state version of the SPKF.  For this version, the returned covariance 
   * matrix has the following states:
   * (1) GPS Update - valid dx/dy data
   *     v = {dx, dy, _, dz, dphi, dtheta, wx, wy}
   * (2) Dead Reckoning Update - velocity integration
   *     v = {vx, vy, vz, dz, dphi, dtheta, wx, wy}
   */
  SymmetricMatrix computeMotionSigmaAugmentState(const poseT& diffPose);


  /* Function: computeMotionSigma(diffPose)
   * Usage: R = computeMotionSigma(diffPose)
   * -------------------------------------------------------------------------*/
  /*! Computes the symmetric motion noise covariance matrix for the 
   * non-augmented state version of the SPKF.  For this version, the returned
   * covariance matrix has the following states:
   * v = {dx, dy, dz, dphi, dtheta, dpsi, wx, wy}
   */
  SymmetricMatrix computeMotionSigma(const poseT& diffPose);


  SymmetricMatrix Sigma; //!< Covariance matrix for SPKF
  ColumnVector mu; //!< Mean vector for SPKF
  double lambda;
  DiagonalMatrix Rv_sub;

  int numStates; //!< number of regular states in SPKF
  int numAugStates; //!< number of augmented states in SPKF
  
  sigmaPointT* sigmaPoints; //!< dynamic array of sigma points
  int numSigmaPoints;

  //!indices into mu/pose vector for sigmaPoints
  int procNoiseStartIdx;
  int measNoiseStartIdx;

  //test file
  ofstream sigmaPointBeforeFile;
  ofstream sigmaPointAfterFile;
  
};

#endif
