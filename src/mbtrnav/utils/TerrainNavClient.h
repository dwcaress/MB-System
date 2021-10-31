/* FILENAME      : TerrainNavClient.h
 * AUTHOR        : Rich Henthorn
 * DATE          : 02/17/13
 * DESCRIPTION   : TerrainNav is used to extract terrain-correlation based 
 *                 estimates of vehicle position and attitude for use in a 
 *                 navigation system.  This class is an interface class between 
 *                 the vehicle user and the terrain navigation filter classes.
 *                 The Client version forwards most of the requests over a TCP
 *                 connection to a TerrainNav process running on another node.
 *
 * DEPENDENCIES  : structDefs.h, myOutput.h, TNavFilter.h, TNavPointMassFilter.h
 *                 TNavParticleFilter.h TNavExtendedKalmanFilter.h 
 *                 TNavSigmaPointFilter.h
 * 
 * LAST MODIFIED : 
 * MODIFIED BY   : 
 * 2016-12-15 RGH: Add particle file name & the log directory name to i/f
 * -----------------------------------------------------------------------------
 * Modification History:
 * -----------------------------------------------------------------------------
 *****************************************************************************/

#ifndef _TerrainNavClient_h
#define _TerrainNavClient_h

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "TerrainNav.h"

/*!
 * Class: TerrainNavClient
 * 
 */

class TerrainNavClient : public TerrainNav
{
 public:

  /* Default Constructor:
   */
  TerrainNavClient();

  /* Server constructor: Just establish a connection to a server.
   */
  TerrainNavClient(char *server_ip, int server_port);


  /* Constructor: TerrainNavClient(mapName)
   * Usage: tercom = new TerrainNavClient("canyonmap");
   * -------------------------------------------------------------------------*/
  /*! Initializes a new TerrainNav with terrain map "mapName.txt". The mapping 
   * AUV specs and the Point Mass Filter algorithm are used as defaults.
   * 2016-12-15 RGH: Add particle file name & the log directory name to i/f
   */
  TerrainNavClient(char *server_ip, int server_port,
		   char *mapName, char *vehicleSpecs, char *particlefile, char *logdir,
		   const int &filterType,
		   const int &mapType);


#if 0
  /* Constructor: TerrainNavClient(mapName)
   * Usage: tercom = new TerrainNavClient("canyonmap");
   * -------------------------------------------------------------------------*/
  /*! Initializes a new TerrainNav with terrain map "mapName.txt". The mapping 
   * AUV specs and the Point Mass Filter algorithm are used as defaults.
   */
  TerrainNavClient(char *mapName);


  /* Constructor: TerrainNavClient(mapName, vehicleName)
   * Usage: tercom = new TerrainNavClient("canyonmap", "mappingAuv");
   * -------------------------------------------------------------------------*/
  /*! Sets private variables mapFile = "mapName.txt", vehicleSpecFile = 
   * "vehicleName.txt", and saveDirectory = NULL.  
   * Makes a call to createFilter(1) to initialize a new TerrainNav
   * object with default type = 1 (Point Mass Filter). 
   */
  TerrainNavClient(char *mapName, char *vehicleSpecs);


  /* Constructor: TerrainNavClient(mapName, vehicleName, filterType)
   * Usage: tercom = new TerrainNavClient("canyonmap", "mappingAuv", 1);
   * -------------------------------------------------------------------------*/
  /*! Sets private variables mapFile = "mapName.txt", vehicleSpecFile = 
   * "vehicleName.txt", and saveDirectory = NULL.  
   * Makes a call to createFilter(filterType) to initialize a new TerrainNav
   * object with specified type. 
   */
  TerrainNavClient(char *server_ip, int server_port,
		   char *mapName, char *vehicleSpecs, 
		   const int &filterType, const int &mapType);


  /* Constructor: TerrainNavClient(mapName, vehicleName, filterType, directory)
   * Usage: tercom = new TerrainNavClient("canyonmap", "mappingAuv", 1, "/home/");
   * -------------------------------------------------------------------------*/
  /*! Sets private variables mapFile = "mapName.txt", vehicleSpecFile = 
   * "vehicleName.txt", and saveDirectory = "directory".  
   * Makes a call to createFilter(filterType) to initialize a new TerrainNav
   * object with specified type. 
   */
  TerrainNavClient(char *mapName, char *vehicleSpecs, 
		   const int &filterType, char *directory, int mapType);

#endif

  /* Destructor: ~TerrainNav
   * Usage: delete tercom;
   * -------------------------------------------------------------------------*/
  /*! Frees all storage associated with the TerrainNav object.
   */
  virtual ~TerrainNavClient();


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
  virtual bool outstandingMeas();

  /* Function: lastMeasSuccessful
   * Usage: tercom->lastMeasSuccessful();
   * -------------------------------------------------------------------------*/
  /*! Returns a boolean indicating if the last sonar measurement 
   * was successfully incorporated into the filter or not.
   */
  virtual bool lastMeasSuccessful();


  /* Function: setInterpMeasAttitude()
   * Usage: tercom->setInterpMeasAttitude(1);
   * -------------------------------------------------------------------------*/
  /*! Sets a boolean indicating if the sonar measurement attitude 
   * information should be determined from interpolated inertial poses.
   * Default = 0;
   */
  virtual void setInterpMeasAttitude(bool set);


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
  virtual void setMapInterpMethod(const int &type);


  /* Function: setVehicleDriftRate()
   * Usage: tercom->setVehicleDriftRate(5)
   * -------------------------------------------------------------------------*/
  /*! Sets the vehicle inertial drift rate parameter.  The default value is
   * determined by the vehicle specification sheet.  The driftRate parameter
   * is given in units of % drift in meters/sec.
   */
  virtual void setVehicleDriftRate(const double &driftRate);


  /* Function: isConverged()
   * Usage: converged = tercom->isConverged();
   * ------------------------------------------------------------------------*/
  /*! Returns a boolean indicating if the terrain navigation filter has 
   * converged to an estimate.
   */
  virtual bool isConverged();


   /* Function: useLowGradeFilter()
   * Usage: tercom->useLowGradeFilter()
   * -------------------------------------------------------------------------*/
  /*! Force filter settings for low grade system:
   * 7DOF system with ALLOW_ATTITUDE_SEARCH=1, DEAD_RECKON=1, SEARCH_GYRO=1
   */
  virtual void useLowGradeFilter();
  

  /* Function: useHighGradeFilter()
   * Usage: tercom->useHighGradeFilter()
   * -------------------------------------------------------------------------*/
  /*! Force filter settings for low grade system:
   * 7DOF system with ALLOW_ATTITUDE_SEARCH=0, DEAD_RECKON=0, SEARCH_GYRO=0
   */
  virtual void useHighGradeFilter();


  /* Function: setFilterReinit(allow)
   * Usage: tercom->setFilterReinit(allow)
   * -------------------------------------------------------------------------*/
  /*! Overwrite boolean allowFilterReinits with input argument, allow.
   */
  virtual void setFilterReinit(const bool allow);

	/* Function: setModifiedWeighting(use)
   * Usage: tercom->tNavFilter->setModifiedWeighting(use)
   * -------------------------------------------------------------------------*/
  /*! Overwrite boolean useModifiedWeighting with input argument, use.
   */
  virtual void setModifiedWeighting(const int use);


  /* Function: getFilterState()
   * Usage: filterType = tercom->getFilterState();
   * ------------------------------------------------------------------------*/
  /*! Returns the integer filterState describing the current filter state.
   */
  virtual int getFilterState();


   /* Function: getNumReinits()
   * Usage: filterType = tercom->getNumReinits();
   * ------------------------------------------------------------------------*/
  /*! Returns the integer numReinits describing the number of reinitializations.
   */
  virtual int getNumReinits();

   /* Function: reinitFilter(bool lowInfoTransition)
   * Usage: filterType = tercom->reinitFilter(true);
   * ------------------------------------------------------------------------*/
  /*! Reinitializes the TRN filter.
   */
  virtual void reinitFilter(const bool lowInfoTransition);

    /* Function: reinitFilterBox(bool lowInfoTransition,
     *                           double offset_x, double offset_y, double offset_z,
     *                           double sdev_x, double sdev_y, double sdev_z)
     * Usage: filterType = tercom->reinitFilterBox(true,...);
     * ------------------------------------------------------------------------*/
    /*! Reinitializes the TRN filter.
     */
    virtual void reinitFilterBox(const bool lowInfoTransition,
                                 double offset_x, double offset_y, double offset_z,
                                 double sdev_x, double sdev_y, double sdev_z);

    virtual void reinitFilterOffset(const bool lowInfoTransition,
                                 double offset_x, double offset_y, double offset_z);

    virtual void setEstNavOffset(double offset_x, double offset_y, double offset_z);

    virtual d_triplet_t *getEstNavOffset(d_triplet_t *dest);

    virtual void setInitStdDevXYZ(double sdev_x, double sdev_y, double sdev_z);

    virtual d_triplet_t *getInitStdDevXYZ(d_triplet_t *dest);

    virtual void setInitVars(InitVars *init_vars);

    bool is_connected();

 protected:

  //////////////////////////////////////////////////////////////////////
  // Other important state variables
  char *_logdir;          // name of the log directory on the MVC

  //////////////////////////////////////////////////////////////////////
  // Initialize connection to server and send state
  bool _connected;
  bool _mbtrn_server_type;
  char *_server_ip;
  int _sockfd;
  int _sockport;
  struct sockaddr_in _server_addr;

  struct commsT _server_msg;
  char _comms_buf[TRN_MSG_SIZE];

  void init_comms();
  void init_server();

  /////////////////////////////////////////////////////////////////////
  // Communication functions
  size_t  send_msg(commsT& msg); // Pack and send the message
  char get_msg();       // Returns the type of message as defined in
                        // structDefs.h, message placed in _server_msg
  bool requestAndConfirm(commsT& msg, char expected_ret_type);

};

#endif
