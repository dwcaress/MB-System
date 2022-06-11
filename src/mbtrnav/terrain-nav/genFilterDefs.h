#ifndef _genFilterDefs_h_
#define _genFilterDefs_h_

#include <math.h>

//*** TODO THIS IS TEMPORARY!!!  (NIS_WINDOW_LENGTH is used by the filter, MAX_NIS_VALUE is used by TerrainNav)
#ifndef NIS_WINDOW_LENGTH
#define NIS_WINDOW_LENGTH 	20		//Length of NIS window
#endif

/******************************************************************************
 MEASUREMENT CORRELATION PARAMETERS
******************************************************************************/
#ifndef USE_RANGE_CORR
#define USE_RANGE_CORR 0  /*!< Boolean indicating if range-based correlation
			   *    should be used instead of projection-based */
#endif

#ifndef MAX_RANGE
#define MAX_RANGE 220.0   //Maximum allowable sonar range
#endif

#ifndef MIN_RANGE
#define MIN_RANGE 5.0     //Minimum allowable sonar range
#endif

#ifndef USE_MAP_NAN
#define USE_MAP_NAN 0
#endif		      /**< Boolean indicating if measurements in NaN regions
		       *   should be used*/

#ifndef HOMER_RANGE_PER_ERROR
#define HOMER_RANGE_PER_ERROR 2.75  //!< Percent homing range error (sigma)
#endif

#ifndef AVERAGE
#define	AVERAGE 0       /*!< Boolean indicating if measurements should be
			 *   averaged at a given time step */
#endif

/******************************************************************************
 FILTER ESTIMATION PARAMETERS
******************************************************************************/

/* To use BIAUV: DEAD_RECKON, ALLOW_ATTITUDE_SEARCH, and SEARCH_GYRO_BIAS
should be turned on.  */

#ifndef INCREASE_WINDOW
#define INCREASE_WINDOW 0 /*!< Boolean indicating if the initial search
				* window should be increased after a measurement update outage */
#endif

#ifndef MOVING_TERRAIN
#define MOVING_TERRAIN 0  /*!< Boolean indicating if terrain should have a
			   *   motion model */
#endif

#ifndef USE_CONTOUR_MATCHING
#define USE_CONTOUR_MATCHING 0 /*!< Boolean indicating if TRN should perform
				*contour matching, in which case depth is not a
				*search variable, but determined by differencing
				*measurements with the map. */
#endif


#ifndef ALLOW_ATTITUDE_SEARCH
#define ALLOW_ATTITUDE_SEARCH 0 /*!< Boolean indicating if attitude states
				 *should be estimated */
#endif

#ifndef INTEG_PHI_THETA
#define INTEG_PHI_THETA 0   /*!< Boolean indicating if Phi and Theta should be
			     *determined by integrating gyros (in which case,
			     *phi/theta meas. are used in measurement updates)*/
#endif

#ifndef SEARCH_COMPASS_BIAS
#define SEARCH_COMPASS_BIAS 0 /*!< Boolean indicating if compass bias should be
			       *estimated */
#endif

#ifndef SEARCH_PSI_BERG
#define SEARCH_PSI_BERG 0     /*!< Boolean indicating if iceberg orientation should be
			       *estimated */
#endif

#ifndef SEARCH_ALIGN_STATE
#define SEARCH_ALIGN_STATE 0  /*!< Boolean indicating if DVL alignment errors
			       *should be estimated */
#endif

#ifndef SEARCH_GYRO_BIAS
#define SEARCH_GYRO_BIAS 0 /*!<Boolean indicating if gyro bias should be
			    * estimated*/
#endif

#ifndef SEARCH_GYRO_Y
#define SEARCH_GYRO_Y 0  /*!<Boolean indicating if y component of angular
			  * velocity should be estimated */
#endif

#ifndef DEAD_RECKON
#define DEAD_RECKON  0 //!<Boolean indicating if dead reckoning should be used.
#endif

#ifndef SEARCH_DVL_ERRORS
#define SEARCH_DVL_ERRORS 0  /*!< Boolean indicating if DVL bias/scale factor
			      * errors should be searched over */
#endif

#ifndef USE_COMPASS_BIAS
#define USE_COMPASS_BIAS 0  /*!< Boolean indicating if known compass bias should
			     * be applied */
#endif

#ifndef MAP_NOISE_MULTIPLIER
#define MAP_NOISE_MULTIPLIER 1.0  /*!< Multiplier on map noise in particle filter
					* should be 1.0 usually */
#endif

#ifndef MOTION_NOISE_MULTIPLIER
#define MOTION_NOISE_MULTIPLIER 2.0  /*!< Multiplier on motion noise std in particle filter
					* should be 1.0 usually */
#endif

/******************************************************************************
 FILTER INITIALIZATION PARAMETERS
******************************************************************************/
// [XYZ]_STDDEV_INIT values establish a map search area; values are application-specific.
// If the distribution is uniform, these comprise a box (see initDistribType, TNavFilter.h).
// If set incorrectly, TRN may fail to initialize correctly, resulting in errors, e.g.:
//   TerrainNav::Filter not initialized - vehicle is currently within a non-valid
//   region of the reference map
//   TerrainNav::Cannot compute pose estimate; motion has not been initialized.
// Typical values include:
// [XY]_STDDEV_INIT
//   60.0 Portuguese Ledge/Dorado
//   600.0 Axial/Sentry
// [Z]_STDDEV_INIT
//   5.0 Portuguese Ledge/Dorado, Axial/Sentry
//   10.0 ?

#ifndef X_STDDEV_INIT       //! x standard deviation (m) for initialization
#define X_STDDEV_INIT 60.0
#endif

#ifndef Y_STDDEV_INIT       //!  y standard deviation (m) for initialization
#define Y_STDDEV_INIT 60.0
#endif

#ifndef Z_STDDEV_INIT       //! z standard deviation (m) for initialization
#define Z_STDDEV_INIT 5.0
#endif

#ifndef PHI_STDDEV_INIT   //! roll standard deviation (rad) for initialization
#define PHI_STDDEV_INIT 0.5*PI/180.0
#endif

#ifndef THETA_STDDEV_INIT //! pitch standard deviation (rad) for initialization
#define THETA_STDDEV_INIT 0.5*PI/180.0
#endif

#ifndef PSI_STDDEV_INIT //! heading standard deviation (rad) for initialization
#define PSI_STDDEV_INIT 15.0*PI/180.0 //30.0*PI/180.0 //
#endif

#ifndef TERRAIN_DXDT_STDDEV_INIT   //terrain x velocity standard deviation for
#define TERRAIN_DXDT_STDDEV_INIT 0 //initialization (m/s)
#endif

#ifndef TERRAIN_DYDT_STDDEV_INIT   //terrain y velocity standard deviation for
#define TERRAIN_DYDT_STDDEV_INIT 0 //initialization (m/s)
#endif

#ifndef TERRAIN_DHDT_STDDEV_INIT   //terrain heading velocity standard deviation
#define TERRAIN_DHDT_STDDEV_INIT 0 // for initialization (m/s)
#endif

#ifndef COMPASS_BIAS_STDDEV_INIT   //compass bias uniform distrib half-width
#define COMPASS_BIAS_STDDEV_INIT 0 //for initialization (rad)
#endif

#ifndef PHI_ALIGN_ERROR_STDDEV_INIT  //phi alignment error uniform distrib (rad)
#define PHI_ALIGN_ERROR_STDDEV_INIT 1.5*PI/180.0 //half-width for initialization
#endif

#ifndef THETA_ALIGN_ERROR_STDDEV_INIT  //theta align. error uniform distrib(rad)
#define THETA_ALIGN_ERROR_STDDEV_INIT 1.5*PI/180 //half-width for initialization
#endif

#ifndef PSI_ALIGN_ERROR_STDDEV_INIT  //psi alignment error uniform distrib (rad)
#define PSI_ALIGN_ERROR_STDDEV_INIT 0.5*PI/180.0 //halfwidth for initialization
#endif

#ifndef GYRO_BIAS_STDDEV_INIT   //gyro bias uniform distrib half-width for
#define GYRO_BIAS_STDDEV_INIT 0.5*PI/180.0  //initialization (rad/sec)
#endif

#ifndef PSI_BERG_STDDEV_INIT   //uniform distrib half-width for berg.
#define PSI_BERG_STDDEV_INIT (PI/sqrt(3))  //initialization (rad/sec)
#endif

#ifndef DVL_SF_STDDEV_INIT   //dvl velocity scale factor uniform distrib
#define DVL_SF_STDDEV_INIT 0.005  //half-width for initialization
#endif

#ifndef DVL_BIAS_STDDEV_INIT  //dvl velocity bias uniform distrib half-width
#define DVL_BIAS_STDDEV_INIT 0.3/100.0 //for initialization (m/sec)
#endif

#ifndef USE_PARTICLE_FILE  //Boolean to initialize using a prespecified set of points in particles.cfg file
#define USE_PARTICLE_FILE 0 //1=true, 0=false (default)
#endif


/******************************************************************************
 FILTER MOTION UPDATE PARAMETERS
******************************************************************************/
#ifndef USE_ACCEL
#define USE_ACCEL 0 //Boolean indicating if estimated acceleration should
#endif              //be used in motion updates

#ifndef PSI_BERG_PROCESS_STD
//#define PSI_BERG_PROCESS_STD (PI/12/60)  //15 degrees per sqrt(hour), in
					 //rad/sqrt(sec).  Need to account
					 //for 3 sec sampling period.
#define PSI_BERG_PROCESS_STD (PI/12/20)
#endif

#ifndef VEL_PER_ERROR      //! Percent ground velocity error (sigma)
#define VEL_PER_ERROR 5.0
#endif

#ifndef WATER_VEL_PER_ERROR      //! Percent water velocity error (sigma)
#define WATER_VEL_PER_ERROR 60.0
#endif

#ifndef VEL_STDDEV
#define VEL_STDDEV 0.0 //Additional velocity noise sigma added above percent
#endif                 //velocity error

#ifndef DX_FRAC_STDDEV  //Std dev of Gaussian noise added to northing displ.(m)
#define DX_FRAC_STDDEV .03  //Given as a fraction of distance travelled in x
#endif

#ifndef DY_FRAC_STDDEV  //Std dev of Gaussian noise added to easting displ. (m)
#define DY_FRAC_STDDEV .03  //Given as a fraction of distance travelled in y
#endif

#ifndef DZ_STDDEV  //! Std dev of Gaussian noise added to vertical displ. (m)
#define DZ_STDDEV .5
#endif

#ifndef DPHI_STDDEV  //! Std dev of Gaussian noise added to roll displ.(rad)
#define DPHI_STDDEV 0.1*PI/180.0
#endif

#ifndef DTHETA_STDDEV  //! Std dev of Gaussian noise added to pitch displ.(rad)
#define DTHETA_STDDEV 0.1*PI/180.0
#endif

#ifndef DPSI_STDDEV   //! Std dev of Gaussian noise added to heading displ.(rad)
#define DPSI_STDDEV 0.1*PI/180.0
#endif

#ifndef DPSI_RATE_FACTOR_STDDEV  //Std dev of Gaussian noise to be multiplied by
#define DPSI_RATE_FACTOR_STDDEV 0.0*PI/180.0  //current psi_dot estimate (rad)
#endif

#ifndef DALIGN_STDDEV  //Std dev of Gaussian noise added to dvl align angles
#define DALIGN_STDDEV 0.05*PI/180.0  //(rad)
#endif

#ifndef DGBIAS_ERROR
#define DGBIAS_ERROR 2.0*PI/(60.0*180.0) //Gyro noise parameter from spec. sheet
#endif                                   // (rad/s^0.5)

#ifndef DDVLSF_STDDEV
#define DDVLSF_STDDEV 0.0001
#endif

#ifndef DDVLBIAS_STDDEV
#define DDVLBIAS_STDDEV 0.0001
#endif


#endif
