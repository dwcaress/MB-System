/* FILENAME      : TNavExtendedKalmanFilter.h
 * AUTHOR        : Debbie Meduna
 * DATE          : 04/30/09
 * DESCRIPTION   : TNavExtendedKalmanFilter is an inheritance class of 
 *                TNavFilter.h which uses an Extended Kalman Filter(EKF) 
 *                implementation of the terrain navigation filter. The primary 
 *                data structures are (1) a Matrix containing the covariance, 
 *                and (2) a ColumnVector containing the mean. 
 *
 * DEPENDENCIES  : TNavFilter.h, CommonDefs.h, matrixArrayCalcs.h, sensCalcs.h,
 *                 structDefs.h, myOutput.h, TerrainMap.h, newmat*.h
 *
 * LAST MODIFIED : 04/30/09 
 * MODIFIED BY   : Debbie Meduna   
 * ----------------------------------------------------------------------------
 * Modification History
 * ----------------------------------------------------------------------------
 *****************************************************************************/

#ifndef _TNavExtendKalmanFilter_h
#define _TNavExtendKalmanFilter_h

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


/*!
 * Class: TNavExtendKalmanFilter
 * 
 * This class inherits from the TNavFilter class. It uses an extended kalman
 * filter implementation to compute and propagate terrain navigation pose 
 * estimates based on vehicle sonar and inertial measurements.  The filter 
 * maintains two states: vehicle Northing and vehicle Easting.
 *
 * Intended use:
 * <pre>
 *        Initalize the TNavExtendKalmanFilter object
 *               TNavFilter *tNavFilter;
 *               tNavFilter = new TNavExtendKalmanFilter();
 *
 *        Add sonar and inertial measurements as they come in
 *               tNavFilter->measUpdate(currMeas);
 *               tNavFilter->motionUpdate(currNavPose);
 *
 *        Compute filter pose estimates
 *               tNavFilter->computeMLE(mlePose)
 *               tNavFilter->computeMMSE(mmsePose)
 * </pre>
 */
class TNavExtendKalmanFilter : public TNavFilter
{
 public:
  
  /* Constructor: TNavExtendKalmanFilter(mapName, vehicleSpecs, directory)
   * Usage: trnFilter = new TNavExtendKalmanFilter("mapName", "vehicle", "dir");
   * -------------------------------------------------------------------------*/
  /*! Initializes a new TNavExtendKalmanFilter object with TerrainMap given by 
   * mapName.txt, vehicle specifications given by vehicle.txt, and file save
   * directory given by dir, uses windowVar to initialize filter window.
   */
  TNavExtendKalmanFilter(char *mapName, char *vehicleSpecs,
			 char *directory, const double *windowVar);


  /* Destructor: ~TNavExtendKalmanFilter()
   * Usage: delete trnFilter;
   * -------------------------------------------------------------------------*/
  /*! Frees all storage associated with the TNavExtendKalmanFilter object.
   */
  virtual ~TNavExtendKalmanFilter();


  /* Function: initFilter
   * Usage: initFilter(currNavPose);
   * -------------------------------------------------------------------------*/
  /*! Initializes the filter components by calling InitPriorPDF(). 
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
  /*! Checks if the current TRN filter has converged.  For the EKF, the filter
   * is always converged.
   */
  void checkConvergence();


  /* Function: saveCurrDistrib()
   * Usage: saveCurrDistrib(file)
   * -------------------------------------------------------------------------*/
  /*! Saves the current TRN filter information to the specified output file. For
   * the EKF, this corresponds to the current mean vector and Sigma matrix:
   *          mu | Sigma
   */
  void saveCurrDistrib(ofstream &outputFile);
  

 private:

  /* Function: incorporateMeas()
   * Usage: incorporateMeas()
   * -------------------------------------------------------------------------*/
  /*! Incorporates the current measurements stored in corrData by computing the
   * Kalman matrix and measurement innovation.  The H matrix is computed here
   * by computing the terrain gradients local to each measurement beam.
   * Returns a boolean indicating if the measurment was successfully added 
   * (will be successful unless no valid terrain map information is available
   * for the current measurement).
   */
  bool incorporateMeas();


  /* Function: generateMeasCorrData(currMeas)
   * Usage: generateMeasCorrData(currMeas)
   * -------------------------------------------------------------------------*/
  /*! Generates the corrData structure for the current sonar measurment,currMeas
   * , to be used in computing the EKF measurement update. Returns a 
   * boolean indicating if the correlation vector is non-zero size.
   */
  bool generateMeasCorrData(measT* currMeas);


  /* Function: extractMap(currPose)
   * Usage: extractMap(currPose)
   * -------------------------------------------------------------------------*/
  /*! This function loads a map segment from terrainMap->refMap which covers the
   * relevant correlation area.  The bounds of this map are determined by taking
   * a 3-sigma circumference around the current pose estimate and adding
   * onto that the expected maximum beam projections in North and East.
   */
  int extractMap(const poseT &currPose);
  

  SymmetricMatrix Sigma; //!< Covariance matrix for EKF
  ColumnVector mu;  //!< Mean vector for EKF

  //!dynamic array containing current correlation data
  corrT* corrData;
  int numCorr;
  
};

#endif
