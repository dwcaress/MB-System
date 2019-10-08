/* FILENAME      : TNavPointMassFilter.h
 * AUTHOR        : Debbie Meduna
 * DATE          : 04/24/09
 * DESCRIPTION   : TNavPointMassFilter is an inheritance class of TNavFilter.h 
 *                which uses a Point Mass Filter(PMF) implementation of the 
 *                terrain navigation filter. The primary data structure in this
 *                class is a mapT which stores a discrete probability 
 *                distribution of the vehicle North-East location, from which 
 *                North-East position estimates can be derived.  The Depth 
 *                location of the vehicle is tracked by a maximum likelihood 
 *                depth bias estimate which is stored in a separate Matrix data
 *                structure.
 *
 * DEPENDENCIES  : TNavFilter.h, CommonDefs.h, matrixArrayCalcs.h, sensCalcs.h,
 *                 structDefs.h, myOutput.h, TerrainMap.h, newmat*.h, engine.h 
 *                 (if running with Matlab)
 *
 * LAST MODIFIED : 04/27/09 
 * MODIFIED BY   : Debbie Meduna   
 * ----------------------------------------------------------------------------
 * Modification History
 * ----------------------------------------------------------------------------
 *****************************************************************************/

#ifndef _TNavPointMassFilter_h
#define _TNavPointMassFilter_h

#include "TNavFilter.h"
#include "genFilterDefs.h"
#include "matrixArrayCalcs.h"
#include "structDefs.h"
#include "myOutput.h"
#include "TerrainMap.h"

#include <newmatap.h>
#include <newmatio.h>

#include <math.h>
#include <fstream>
#include <iomanip>

/*******************************************************************************
 * Point Mass Filter Specific Parameters
 ******************************************************************************/
#ifndef HYP_RES
#define HYP_RES 1      //double indicating resolution of the hypothesis grid
#endif

#ifndef USE_MOTION_BLUR    //boolean indicating if motion blur should be 
#define USE_MOTION_BLUR 1  //implemented for motion updates
#endif

#ifndef MOTION_BLUR_METHOD    //indicates method used for motion blurring:
#define MOTION_BLUR_METHOD 2 //1)Discrete Convolution, 2)FPE Explicit Diff.
#endif

#ifndef DEPTH_FILTER_LENGTH
#define DEPTH_FILTER_LENGTH 1 //indicates number of prev. measurements used for 
#endif                        //depth bias calculation


/*!
 * Class: TNavPointMassFilter
 * 
 * This class inherits from the TNavFilter class. It uses a point mass filter
 * implementation to compute and propagate terrain navigation pose 
 * estimates based on vehicle sonar and inertial measurements.
 * The TNavPointMassFilter object contains a mapT structure for storing the 
 * probability distribution associated with the vehicle's current North-East 
 * location.  It also conatins a set of Matrix structures which track the 
 * maximum likelihood depth bias estimate at each North-East location.
 *
 * Intended use:
 *       Initalize the TNavPointMassFilter object
 *               TNavFilter *tNavFilter;
 *               tNavFilter = new TNavPointMassFilter();
 *
 *       Add sonar and inertial measurements as they come in
 *               tNavFilter->measUpdate(currMeas);
 *               tNavFilter->motionUpdate(currNavPose);
 *
 *       Compute filter pose estimates
 *               tNavFilter->computeMLE(mlePose)
 *               tNavFilter->computeMMSE(mmsePose)
 */
class TNavPointMassFilter : public TNavFilter
{
 public:
 
  /* Constructor: TNavPointMassFilter(mapName, vehicleSpecs, directory)
   * Usage: trnFilter = new TNavPointMassFilter("mapName", "vehicle", "dir");
   * -------------------------------------------------------------------------*/
  /*! Initializes a new TNavPointMassFilter object with TerrainMap given by 
   * mapName.txt, vehicle specifications given by vehicle.txt, and file save
   * directory given by dir, uses windowVar to initialize filter window.
   */
  //Reload Map Issue
  //TNavPointMassFilter(char *mapName, char *vehicleSpecs,
  //		      char *directory, const double *windowVar, const int &mapType);
  TNavPointMassFilter(TerrainMap* terrainMap, char *vehicleSpecs,
  		      char *directory, const double *windowVar, const int &mapType);

  /* Destructor: ~TNavPointMassFilter()
   * Usage: delete trnFilter;
   * -------------------------------------------------------------------------*/
  /*! Frees all storage associated with the TNavPointMassFilter object.
   */
  virtual ~TNavPointMassFilter();


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
   * Usage: motionUpdate(currNavPose);
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
  /*! Checks if the current TRN filter has converged to a single estimate with
   * Gaussian-like distribution.
   */
  void checkConvergence();


  /* Function: saveCurrDistrib()
   * Usage: saveCurrDistrib(file)
   * -------------------------------------------------------------------------*/
  /*! Saves the current TRN filter information to the specified output file. For
   * the point mass filter, this corresponds to saving the current priorPDF.
   */
  void saveCurrDistrib(ofstream &outputFile);


  /* Function: getCurrPDF
   * Usage: success = getCurrPDF(currPDF);
   * ------------------------------------------------------------------------*/
  /*! Extracts the current PDF stored in the TNavPointMassFilter object.  
   * Returns a bool indicating if the PDF was successfully accessed.
   */
  bool getCurrPDF(mapT &currPDF);


  /* Function: getLikeSurf
   * Usage: success = getLikeSurf(currLikeSurf);
   * ------------------------------------------------------------------------*/
  /*! Extracts the current Likelihood surface stored in the TNavPointMassFilter
   * object. Returns a bool indicating if the surface was successfully accessed.
   */
  bool getLikeSurf(mapT &currLikeSurf);

 private:

  /* Function: initVariables()
   * Usage: initVariables();
   * -------------------------------------------------------------------------*/
  /*! Initializes private variables associated with a TNavPointMassFilter object
   */
  void initVariables();


  /* Helper Function: initPriorPDF
   * Usage: initPriorPDF(insPose);
   * -------------------------------------------------------------------------*/
  /*! Initializes the priorPDF to a uniform distribution.  Also initializes
   * all depth bias related Matrix structures to zeros.
   */
  void initPriorPDF(const poseT& initNavPose);


  /* Helper Function: generateMeasCorrData
   * Usage: generateMeasCorrData(currMeas);
   * -------------------------------------------------------------------------*/
  /*! Generates the corrData structure for the current sonar measurment,currMeas
   * , to be used in creating a likelihood correlation surface. Returns a 
   * boolean indicating if the correlation vector is non-zero size.
   */
  bool generateMeasCorrData(const measT& currMeas);


  /* Helper Function: generateCorrelationSurf
   * Usage: likeSurf = generateCorrelationSurf(containsNaN);
   * -------------------------------------------------------------------------*/
  /*! This function is the main workhorse of the TNavPointMassFilter class.  It 
   * generates a likelihood surface using a TERCOM-style correlation between 
   * measurements stored in corrData and the current loaded terrainMap->map.  
   * The function performs the correlation centered at the lastNavPose and 
   * returns the resulting likelihood surface as a Matrix.
   * The function takes in and modifies a boolean indicating if the correlation 
   * process involves NaN map data. When USE_MAP_NAN=false and containsNaN=true,
   * the current measurement is not incorporated.
   */
  Matrix generateCorrelationSurf(bool &containsNaN); 


  /* Helper Function: extractDepthCompareValues
   * Usage: extractDepthCompareValues(depthVals, beamNum)
   * -------------------------------------------------------------------------*/
  /*! This function is used to extract depth values from the current stored map
   * which correspond to the (x,y) location of measurement beam beamNum.
   */
  void extractDepthCompareValues(Matrix &depthMat, Matrix &varMat, 
				 const int measNum);

 
  /* Helper Function: generateDepthCorrelation
   * Usage: Like = generateDepthCorrelation(M,Esq,E,row,col)
   * -------------------------------------------------------------------------*/
  /*! This function is used to generate the correlation value using a likelihood
   * function based upon maximum likelihood depth bias estimation.  It computes
   * the correlation value for a single point in priorPDF specified by the 
   * input row & col.
   */
  double generateDepthCorrelation(double invVarSum, double sqCorrError,
				  double corrError, int row, int col);


  /* Helper Function: generateDepthFilterCorrelation
   * Usage: Like = generateDepthFilterCorrelation(M,Esq,E,row,col)
   * -------------------------------------------------------------------------*/
  /*! This function is used to generate the correlation value using a likelihood
   * function based upon maximum likelihood depth bias estimation.  It computes
   * the correlation value for a single point in priorPDF specified by the 
   * input row & col.  This function computes the max. likelihood depth bias 
   * based on the previous DEPTH_FILTER_LENGTH measurements.
   */
  double generateDepthFilterCorrelation(double invVarSum, double sqCorrError,
					double sumCorrError, int row, 
					int col);


  /* Helper Function: defineSearchBoundsAndMap
   * Usage: defineSearchBoundsAndMap([x y])
   * -------------------------------------------------------------------------*/
  /*! This function defines the search region for a given correlation step based
   * on vehicle location and user-defined variables SEARCH_X and SEARCH_Y. Once
   * the search region is defined, a corresponding depth map is extracted and 
   * the hypBounds indices are defined accordingly. 
   */
  int defineHypBoundsAndMap(const double* loc);


  /* Helper Function: zeroPad
   * Usage: zeroPad(LikeSurf);
   * -------------------------------------------------------------------------*/
  /*! Pads the given likelihood surface with zeros such that the resulting 
   * matrix is the same size as priorPDF. 
   */
  void zeroPad(Matrix &Like);


  /* Helper Function: motionBlur
   * Usage: motionBlur(dt, dx);
   * -------------------------------------------------------------------------*/
  /*! Performs motion blurring. Called by motionUpdate();
   */
  void motionBlur(const double dt, const double dx, const SymmetricMatrix Cov);


  /* Helper Function: motionBlur_convolve
   * Usage: motionBlur_convolve(dx);
   * -------------------------------------------------------------------------*/
  /*! Performs motion blurring by a discrete convolution with a gaussian 
   * blurring matrix. Called by motionBlur();
   */
  bool motionBlur_convolve(const SymmetricMatrix Cov);


  /* Helper Function: motionBlur_FPEexplicit
   * Usage: motionBlur(alpha);
   * -------------------------------------------------------------------------*/
  /*! Performs motion blurring by an Euler explicit implementation of the 
   * Fokker-Planck equation.  Called by motionBlur();
   */
  void motionBlur_FPEexplicit(const double dt, const SymmetricMatrix Cov);


  /* Helper Function: plotMatlabSurf
   * Usage: plotMatlabSurf(A, "title('A');", "figure(1);");
   * -------------------------------------------------------------------------*/
  /*! Plots the surface A using the Matlab engine.  The title and figure strings
   * are needed to specify labels of the corresponding Matlab figure.
   */
  void plotMatlabSurf(const Matrix &Surf, const char* plotTitle, 
		      const char* figureNum);



  //Private structures and components of a TNavPointMassFilter object:
  /*********************************************************/

  //!dynamic array containing current correlation data
  corrT* corrData;
  int numCorr;

  //!integer array of boundaries on priorPDF specifying the non-zero elements
  int hypBounds[4];

  //!mapT structure for storing PMF probability distribution information
  mapT* priorPDF;
  mapT* likeSurf;

  //!double indicating the vehicle's motion since the last motion blurring
  double dx_old;
  SymmetricMatrix Cov_Old;

  //Matrix structures needed for computing and tracking the depth bias
  Matrix totalSumInvVar;
  Matrix currSumInvVar;  
  Matrix currSumError;
  Matrix depthBias;
  Matrix tempDepthBias;

  //!structures used for tracking depth bias filter
  Matrix measSumError[DEPTH_FILTER_LENGTH];
  Matrix measSumInvVar[DEPTH_FILTER_LENGTH];
  int currMeasPointer;
  
  //output files for writing various intermediate filter calculations
  ofstream gradientFile;
  ofstream measFile;  
  ofstream numMeasFile;
  ofstream likeSurfFile;
  ofstream postSurfFile;
  ofstream depthBiasFile;

};

#endif
