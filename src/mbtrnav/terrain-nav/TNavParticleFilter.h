/* FILENAME      : TNavFilter.h
 * AUTHOR        : Debbie Meduna, Peter Kimball
 * DATE          : 05/06/09
 * DESCRIPTION   : TNavParticleFilter is an inheritance class of TNavFilter.h 
 *                 which uses a Particle Filter(PF) implementation of the 
 *                 terrain navigation filter. 
 * DEPENDENCIES  : TNavFilter.h, CommonDefs.h, particleFilterDefs.h, 
 *                 matrixArrayCalcs.h, sensCalcs.h, structDefs.h, myOutput.h,
 *                 TerrainMap.h, newmat*.h
 * 
 * LAST MODIFIED : 05/28/13
 * MODIFIED BY   : Sarah Houts
 * -----------------------------------------------------------------------------
 * Modification History
 * -----------------------------------------------------------------------------
 * 	
 ******************************************************************************/

#ifndef _TNavParticleFilter_h
#define _TNavParticleFilter_h

#include "TNavFilter.h"
#include "genFilterDefs.h"
#include "particleFilterDefs.h"
#include "matrixArrayCalcs.h"
#include "structDefs.h"
#include "myOutput.h"
#include "trn_log.h"

#include "TerrainMap.h"

#include <newmatap.h>
#include <newmatio.h>

#include <math.h>
#include <fstream>
#include <iomanip>
#include <time.h>
#include <vector>



/*!particleT struct contains a particle's weight, position, attitude,
 * and terrain state, consisting of North, East, and Heading rates.*/
struct particleT {
  double weight;							//particle weight (all weights sum to 1)
  double position[3];					//N,E,D position estimate in meters (default)
  double attitude[3];					//Attitude phi, theta, psi euler angles (triggered by ALLOW_ATTITUDE_SEARCH)
  double terrainState[3];			//N,E,heading offset of terrain (triggered by MOVING_TERRAIN)
  double alignState[3];				//Phi, theta, psi offset of sensor wrt vehicle (triggered by SEARCH_ALIGN_STATE)
  double gyroBias[3];					//gyro bias rates for phi, theta, psi in rad/s (triggered by SEARCH_GYRO_BIAS)
  double compassBias;					//heading bias estimate (triggerd by SEARCH_COMPASS_BIAS)
  double psiBerg;                               //Radians.  Iceberg orientation state triggered by SEARCH_PSI_BERG
  double dvlScaleFactor;			//DVL scale factor estimate (TODO: keep this? triggered by SEARCH_DVL_ERRORS)
  double dvlBias[3];					//DVL bias estimate (TODO: keep this? triggered by SEARCH_DVL_ERRORS)

	//TODO: Maybe put this in another place?  Not a property of the particle?
  //double expectedMeasDiff[4]; //TODO: make this not hard coded to 4 measurements, maybe use vector - not sure how this impacts efficiency, tho?
	std::vector<double> expectedMeasDiff; //TODO: make this not hard coded to 4 measurements, maybe use vector - not sure how this impacts efficiency, tho?

	double windowedNis[20];
	unsigned int windowIndex;
  void displayParticleInfo()
    {
      int i;
      
      logs(TL_OMASK(TL_TNAV_PARTICLE_FILTER, TL_LOG),"Particle Weight: %f\n", weight);
      logs(TL_OMASK(TL_TNAV_PARTICLE_FILTER, TL_LOG),"Particle Position (N,E,D): ");
      for(i = 0; i < 3; i++)
	logs(TL_OMASK(TL_TNAV_PARTICLE_FILTER, TL_LOG),"%.3f\t", position[i]);
      logs(TL_OMASK(TL_TNAV_PARTICLE_FILTER, TL_LOG),"\nParticle Attitude in degrees (phi,theta,psi): ");
      for(i = 0; i < 3; i++)
	logs(TL_OMASK(TL_TNAV_PARTICLE_FILTER, TL_LOG),"%.2f\t", attitude[i]*180.0/PI);
      logs(TL_OMASK(TL_TNAV_PARTICLE_FILTER, TL_LOG),"\nParticle Terrain Pose: ");
      for(i = 0; i < 3; i++)
	logs(TL_OMASK(TL_TNAV_PARTICLE_FILTER, TL_LOG),"%f\t", terrainState[i]);
      logs(TL_OMASK(TL_TNAV_PARTICLE_FILTER, TL_LOG),"\nParticle DVL Alignment in degrees: ");
      for(i = 0; i < 3; i++)
	logs(TL_OMASK(TL_TNAV_PARTICLE_FILTER, TL_LOG),"%f\t", alignState[i]*180.0/PI);
      logs(TL_OMASK(TL_TNAV_PARTICLE_FILTER, TL_LOG),"\nParticle Gyro Bias (^o/sec) in y,z: ");
      for(i = 0; i < 2; i++)
	 logs(TL_OMASK(TL_TNAV_PARTICLE_FILTER, TL_LOG),"%f\t", gyroBias[i]*180.0/PI);
      logs(TL_OMASK(TL_TNAV_PARTICLE_FILTER, TL_LOG),"Particle Compass Bias in degrees: %f\n", compassBias*180.0/PI);
      logs(TL_OMASK(TL_TNAV_PARTICLE_FILTER, TL_LOG),"Particle Psi Berg in degrees: %f\n", psiBerg*180.0/PI);
      logs(TL_OMASK(TL_TNAV_PARTICLE_FILTER, TL_LOG),"Particle Expected Measurement Difference: ");

      for (std::vector<double>::size_type ix = 0; ix < expectedMeasDiff.size(); ix++)
	 logs(TL_OMASK(TL_TNAV_PARTICLE_FILTER, TL_LOG),"%f\t", expectedMeasDiff[ix]);
      logs(TL_OMASK(TL_TNAV_PARTICLE_FILTER, TL_LOG),"\n");
//	logs(TL_OMASK(TL_TNAV_PARTICLE_FILTER, TL_LOG),"Particle Expected Measurement Difference: ");
//      for(i = 0; i < 4; i++)
//	logs(TL_OMASK(TL_TNAV_PARTICLE_FILTER, TL_LOG),"%f\t", expectedMeasDiff[i]);
//      logs(TL_OMASK(TL_TNAV_PARTICLE_FILTER, TL_LOG),"\n");
    }

   //copy assignment operator
   particleT& operator=(particleT& rhs)
   {
      int i;
      if(this != &rhs)
      {
	 weight = rhs.weight;
	 for(i = 0; i < 3; i++)
	 {
	    position[i] = rhs.position[i];
	    attitude[i] = rhs.attitude[i];
	    terrainState[i] = rhs.terrainState[i];
	    alignState[i] = rhs.alignState[i];
	    dvlBias[i] = rhs.dvlBias[i];
	    gyroBias[i] = rhs.gyroBias[i];
	 }	

	 //for(i = 0; i < 4; i++) expectedMeasDiff[i] = rhs.expectedMeasDiff[i];
	 expectedMeasDiff = rhs.expectedMeasDiff;

	 compassBias = rhs.compassBias;
	 psiBerg = rhs.psiBerg;
	 dvlScaleFactor = rhs.dvlScaleFactor;
      }
	  windowIndex = rhs.windowIndex;
	  for(i=0; i<20; i++){
		  windowedNis[i] = rhs.windowedNis[i];
	  }
      return(*this);
   }

};

class TNavPFLog;

/*!
 * Class: TNavParticleFilter
 * 
 * This class inherits from the TNavFilter class. It uses a particle filter
 * implementation to compute and propagate terrain navigation pose 
 * estimates based on vehicle sonar and inertial measurements.
 *
 * Intended use:
 *      Initalize the TNavParticleFilter object
 *               TNavFilter *tNavFilter;
 *               tNavFilter = new TNavParticleFilter();
 *
 *      Add sonar and inertial measurements as they come in
 *               tNavFilter->measUpdate(currMeas);
 *               tNavFilter->motionUpdate(currNavPose);
 *
 *      Compute filter pose estimates
 *               tNavFilter->computeMLE(mlePose)
 *               tNavFilter->computeMMSE(mmsePose)
 */
class TNavParticleFilter : public TNavFilter
{
 public:
 
  /* Constructor: TNavParticleFilter()
   * Usage: trnFilter = new TNavParticleFilter("mapName.txt", vehicle.txt", 
   * "saveDir",windowVar,1);
   * -------------------------------------------------------------------------*/
  /*! Initializes a new TNavParticleFilter object with TerrainMap given by 
   * mapName.txt, vehicle specifications given by vehicle.txt, and file save
   * directory given by dir, uses windowVar to initialize particle set window,
   * uses different maps based on mapType (1:DEM,2:octree).
   */
  //TNavParticleFilter(char *mapName, char *vehicleSpecs, char *directory, const double *windowVar, const int &mapType); //Map Reload Issue
  TNavParticleFilter(TerrainMap* terrainMap, char *vehicleSpecs, char *directory, const double *windowVar, const int &mapType);



  /* Destructor: ~TNavParticleFilter()
   * Usage: delete trnFilter;
   * -------------------------------------------------------------------------*/
  /*! Frees all storage associated with the TNavParticleFilter object.
   */
  virtual ~TNavParticleFilter();


  /* Function: initFilter
   * Usage: initFilter(currNavPose);
   * -------------------------------------------------------------------------*/
  /*! Initializes the particles based on initial distribution parameters.
   */
  void initFilter(poseT& initNavPose);


	//TODO: UPDATE THIS DESCRIPTION
  /* Function: measUpdate
   * Usage: 1 = measUpdate(currMeas);
   * -------------------------------------------------------------------------*/
  /*! Incorporates the current measurement information into the particle
   * distribution, updating the particle weights.  Resamples the particle
   * distribution if appropriate.
   *
	 * Updates the weight of each single particle based on the current
   * measurement.  The expected measurement difference is calculated using
   * getExpectedMeasDiffParticle.  Each particle weight is then calculated as 
   * 
   * normalizer*exp(-(1/2)*inv(Sigma)*(z1^2 + z2^2 + z3^2 + ... +zn^2))
   *
   * where z1, z2, z3, ..., zn are the altitude errors for the n soundings 
   * since the last resample operation.
   * Returns a boolean indicating if the particle measurement update was 
   * successful.  Update is only unsuccessful if a NaN value is encountered
   * in the map AND the USE_MAP_NAN flag is set to false in TNavFilter.h.
   *
   * Returns a boolean indicating if the measurement was successfully added.
   */
  bool measUpdate(measT& currMeas);


	//TODO: UPDATE THIS DESCRIPTION
	/* Function: getExpectedMeasDiffParticle
   * Usage: 1 =  ;
   * -------------------------------------------------------------------------*/
  /*! Computes the difference between the expected and actual measurement for 
	each particle, stores the result in expecteMeasDiff
	  Modifies mapVar and returns the variance associated with the map
    * Returns false if none of the beams should be used to compare particles.
    * Returns true if at least one beam can be used	
   */
	bool getExpectedMeasDiffParticle(particleT& particle, const Matrix& beamsSF, 
								double* beamRanges, const int* beamIndices, double& mapVar);


  /* Function: motionUpdate
   * Usage: motionUpdate(currNavPose);
   * -------------------------------------------------------------------------*/
  /*! Updates all particle positions based on displacement between currNavPose
   * and the stored lastNavPose. 
   */
  void motionUpdate(poseT& currNavPose);


  /* Function: computeMLE
   * Usage: computeMLE(mleEstimate);
   * -------------------------------------------------------------------------*/
  /*! Returns the pose of the particle with the highest weight.
   */
  void computeMLE(poseT* mlePose);


  /* Function: computeMMSE
   * Usage: computeMMSE(mleEstimate);
   * -------------------------------------------------------------------------*/
  /*! Computes and returns a pose estimate based on minimum mean square 
   * estimation - i.e., the probabilistic mean of the data.  This function also
   * computes the covariance of the particle distribution about the MMSE
   * estimate.
   */
  void computeMMSE(poseT* mmsePose);


  /* Function: checkConvergence()
   * Usage: checkConvergence()
   * -------------------------------------------------------------------------*/
  /*! Checks if the current TRN filter has converged to a single estimate with
   * Gaussian-like distribution using Kullback-Liebler divergence
   * TODO: do we want this to also depend on the size of the uncertainty?
   */
  void checkConvergence();


  /* Function: saveCurrDistrib()
   * Usage: saveCurrDistrib(file)
   * -------------------------------------------------------------------------*/
  /*! Saves the current TRN filter information to the specified output file. For
   * the particle filter, this corresponds to the particle information in
   * allParticles.
   */
  void saveCurrDistrib(ofstream &outputFile);


  /* Function: getParticles()
   * Usage: getParticles(curParticles)
   * -------------------------------------------------------------------------*/
  /*! Extracts the current particle distribution for access by outside functions
   */
  particleT* getParticles();
    

  /* Function: saveCurrParticles()
   * Usage: saveCurrParticles(file)
   * -------------------------------------------------------------------------*/
  /*! Saves the current set of particles in allParticles to the specified 
   * output file. 
   */
  void saveCurrParticles(ofstream &outputFile);


  //Public structures and components of a TNavParticleFilter object:
  /*********************************************************/
  

 private:
  
  /* Function: initVariables()
   * Usage: initVariables();
   * -------------------------------------------------------------------------*/
  /*! Initializes private variables associated with a TNavParticleFilter object
   */
  void initVariables();
  

  /* Function: initParticleDist
   * Usage: initParticleDist(intialGuess);
   * -------------------------------------------------------------------------*/
  /*! Initializes the particle distribution based on an initial guess
   */
  void initParticleDist(particleT& initialGuess);


  /* Function: attitudeMeasUpdate
   * Usage: attitudeMeasUpdate(currPose);
   * -------------------------------------------------------------------------*/
  /*! Incorporates the current roll and pitch measurement information into the 
   * particle distribution, using them to update the particle weights.  
   * Resamples the particle distribution if appropriate.
   * Note that this funciton is only called when INTEG_PHI_THETA is set to 1.
   */
  void attitudeMeasUpdate(poseT& currPose);


  /* Function: homerMeasUpdate
   * Usage: homerMeasUpdate(initialGuess);
   * -------------------------------------------------------------------------*/
  /*! Computes the estimated homer location in inertial space for all particles.
   * Based on this new particle distribution, the mean and variance of the homer
   * location is then computed in North and East.  
   * The homer location particle set along with the mean and variance are saved
   * to the files homerParticles.txt and homerMmse.txt respectively.
   */
  bool homerMeasUpdate(const measT& currMeas);


  /* Function: motionUpdateParticle
   * Usage: motionUpdateParticle(particle, dx, dy, dz, dphi, dtheta, dpsi, 
   * lastPsi, dt);
   * -------------------------------------------------------------------------*/
  /*! Updates the terrain-relative pose of a single particle based on the
   * displacements dx, dy, dz, dphi, dtheta, dpsi, over the last dt seconds.
   * Uses terrain motion information stored by the particle.
   */
  void motionUpdateParticle(particleT& particle, poseT& diffPose,
			    double* velocity_sf_sigma, const double& gyroStddev);   

  /* Function: resampParticleDist
   * Usage: resampParticleDist();
   * -------------------------------------------------------------------------*/
  /*! Resamples the particle distibution.  Draws a set of nParticles particles
   * from the distribution, allParticles, with replacement.  Particles are
   * drawn with probability proportional to their weight.
   */
  void resampParticleDist();


  /* Function: getDistBounds
   * Usage: getDistBounds(Nmin, Nmax, Emin, Emax);
   * -------------------------------------------------------------------------*/
  /*! Sets Nmin to the min. North position of all particle in the distribution.
   * Sets Nmax to the max. North position of all particle in the distribution.
   * Sets Emin to the min. East position of all particle in the distribution.
   * Sets Emax to the max. East position of all particle in the distribution.
   */
  void getDistBounds(double& Nmin, double& Nmax, double& Emin, double& Emax);


	

  /* Function: getParticlePose
   * Usage: getParticlePose(particle, particlePose);
   * -------------------------------------------------------------------------*/
  /*! Fills in the poseT object, particlePose, with the pose of the particle
   * pointed to by particle.  Pose particle particle pose pose particle pose.
   */
  void getParticlePose(const particleT& particle, poseT* particlePose);


  /* Function: updateParticleDist()
   * Usage: updateParticleDist();
   * -------------------------------------------------------------------------*/
  /*! Updates the current particle distribution defined in allParticles to the
   * distribution defind by resampParticles.
   */
  void updateParticleDist();


  /* Function: computeKLdiv_gaussian_particles()
   * Usage: computeKLdiv_gaussian_particles();
   * -------------------------------------------------------------------------*/
  /*! Computes the KL divergence in the x,y dimensions for the current particle 
   * distribution.
	 * This compares the particle distribution to a gaussian distribution
   */
  double computeKLdiv_gaussian_particles();


  /* Function: computeInnovationsMatrices()
   * Usage: computeInnovationsMatrices();
   * -------------------------------------------------------------------------*/
  /*! Computes the innovations matrix and mean innovation based on expectedMeasDiff.  
   */
  void computeInnovationsMatrices(particleT* particles, SymmetricMatrix &measVarMat, ColumnVector &measDiff);


  /* Function: plotMapMatlab
   * Usage: plotMapMatlab(Map, xpts, ypts, "title('Map Surface');", 
   * "figure(1);");
   * -------------------------------------------------------------------------*/
  /*! Plots the surface heights in Surf (usually a terrain submap) against the 
   * xpts and ypts given.  Plot appears in the matlab figure defined by 
   * figureNum. The character arrays should be the same as the corresponding 
   * Matlab commands that would be typed into the command window, i.e. 
   * "title('blah')".
   */
  void plotMapMatlab(const Matrix &Surf, double* xpts, double* ypts, 
		     const char* plotTitle, const char* figureNum);


  /* Function: plotParticleDistMatlab
   * Usage: plotParticleDist(particles, "figure(1);");
   * -------------------------------------------------------------------------*/
  /*! Plots the particle distribution in three space using the (position[0],
   * position[1], position[2]) particle properties in a Matlab figure, 
   * figureNum. The character array should be the same as the corresponding 
   * Matlab commands that would be typed into the command window, 
   * i.e. "figure(1);".
   */
  void plotParticleDistMatlab(particleT* particles, const char* figureNum);

  
  /* Function: plotBeamMatlab
   * Usage: plotBeamDist(npos,epos,zpos,beamN,beamE,beamZ, "figure(1)")
   * -------------------------------------------------------------------------*/
  /*! Plots the projected beam locations given by (npos+beamN, epos+beamE,
   * zpos+beamZ) in the current figure window.
   */
  void plotBeamMatlab(const double& npos, const double& epos, 
		      const double& zpos, const double& beamN, 
		      const double& beamE, const double& beamZ, 
		      const char* figureNum);


  /* Function: writeParticlesToFile
   * Usage: writeParticlesToFile(particles, particleFile);
   * -------------------------------------------------------------------------*/
  /*! Writes all information for each particle in "particles" to the file
   * specified by "particleFile".  Each particle is written to a single line
   * in this file with the following format:
   * 
   * index # into particle array - weight - position - attitude - terrainState
   *
   * This function thus adds a data block of size Mx11 to the end of the 
   * current file stream, where M is the size of the "particles" array.
   */
  void writeParticlesToFile(particleT* particles, ofstream &particlesFile);


  /* Function: writeHistDistribToFile
   * Usage: writeHistDistribToFile(particles, particleFile);
   * -------------------------------------------------------------------------*/
  /*! Computes the histogram of the current particle distribution in North, East
   * , Depth and Heading, and writes these histograms to the file specified by
   * "particleFile".  For each histogram, the following two lines of 
   * information are written to file:
   * 
   * identity # of state - min. state value - max. state value - # hist bins
   * value of histogram bins for the above specified state
   *
   * This function thus adds a data block of size 4xN to the end of the 
   * current file stream, where N is the number of histogram bins.
   */
  void writeHistDistribToFile(particleT* particles, ofstream &particlesFile);

  
		//TODO: Are we still going to do all this submap stuff?
		//			This will probably morph into something else if we try to reduce map
		//			loading and unloading in TerrainMap
		/* Function: defineAndLoadSubMap
   * Usage: defineAndLoadSubMap(beamsVF);
   * -------------------------------------------------------------------------*/
		/*! This function loads a map segment from terrainMap->refMap which covers the
   * relevant correlation area.  The bounds of this map are determined by taking
   * the min and max North and East boundaries of all the particles and adding
   * onto that the expected maximum beam projection in North and East.  The 
   * maximum beam projection is determined by using the 3sigma values of the 
   * attitude angles in the current particle distrubtion.
   */
		int defineAndLoadSubMap(const Matrix &beamsVF);

  

  //Private structures and components of a TNavParticleFilter object:
  /*********************************************************/
 
  //! particle array for swapping during resampling
  particleT particleArray1[MAX_PARTICLES];
  
  //!resampled particle array
  particleT particleArray2[MAX_PARTICLES];

  //!pointers to particle and resampled particle arrays
  particleT* allParticles;
  particleT* resampParticles;
  
  //!int keeping track of how many particles the filter is using
  int nParticles;

  //!int showing whether map has been plotted for matlab debugging
  int mapPlotted;

  /*!int keeping track of how many soundings have been used in computing 
   *  current weights*/
  int nSoundings;

  //!boolean indicating if filter has been resampled
  bool resampled;

  //output files for writing various intermediate filter calculations
  ofstream allParticlesFile;
  ofstream resampParticlesFile;  
  ofstream particleWeightsFile;
  ofstream homerParticlesFile;
  ofstream homerMmseFile;
  ofstream measWeightsFile;

  //parameters for augmented MCL (keeping track of measurement likelihoods)
  double a_slow;
  double a_fast;
  double w_slow;
  double w_fast;
  
  //!array of current measurement weights (initialized when first measurement update is applied)
  double currMeasWeights[MAX_PARTICLES];

  // KruChanges not used in this iteration
  // bool KruChanges_;
    
  bool* tempUseBeam;
  bool* useBeam;

  double navData_x_, navData_y_;

  TNavPFLog  *pfLog;
  
};

#endif
