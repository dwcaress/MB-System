/* File: structDefs.h
 * -------------------
 * structDefs defines various structs and their member functions for use in
 * terrainNav type programs.
 *
 * Dependencies:
 * newmat*.h, myOutput.h, matrixArrayCalcs.h
 *
 * Written by: Debbie Meduna
 * 2014/??/??: Henthorn. Updated to support client/server architecture.
 * 2016/12/19: Henthorn. Updated to pass log dir name and particle file name to
 *             the server during initialization.
 ******************************************************************************/

#ifndef _structDefs_h_
#define _structDefs_h_

#include "matrixArrayCalcs.h"
#include "genFilterDefs.h"
#include "trn_common.h"

// Define safer versions of strdup(char*)
// If original is NULL, copy is null
#define  STRDUPNULL(char_star) (char_star != NULL ? strdup(char_star) : NULL)

#define TRN_MAX_BEAMS  120

// Sensor types
//1: DVL, 2: Multibeam, 3: Single Beam, 5: Imagenex delta t multibeam
#define  TRN_SENSOR_DVL     1
#define  TRN_SENSOR_MB      2
#define  TRN_SENSOR_PENCIL  3
#define  TRN_SENSOR_HOMER   4
#define  TRN_SENSOR_DELTAT  5

#define TRN_EST_MLE 1
#define TRN_EST_MMSE 2

// InitVars is a structure that enables TRN reinits
// with search radius that may be configured at run time.
// TerrainNav uses a default version in it's constructor.
// To customize, callers must call reinitFilter, passing in an InitVars
// instance. Calling reinitFilter is currently the only
// way to change the values, ensuring that the values are not changed
// while the filter is running.
class InitVars{
public:
    // TODO: incorporate other parameters?
    // default CTOR
    InitVars();

    // initializing CTOR
    InitVars(double x, double y, double z);

    explicit InitVars(d_triplet_t *xyz);

    // copy CTOR
    explicit InitVars(InitVars *xyz);

    // DTOR
    ~InitVars();

    d_triplet_t *getXYZ(d_triplet_t *dest);

    int setXYZ(d_triplet_t *src);

    int setXYZ(double x, double y, double z);

    double x();
    double y();
    double z();
private:
    trn_initvars_t _ivars;
};

//!The mapT struct stores data in a gridded Matrix.
struct mapT {
    Matrix depths; //Positive down
    Matrix depthVariance;
    double* xpts; //North
    double* ypts; //East
    double dx, dy;
    double xcen, ycen;
    int numX, numY;

    mapT();
    ~mapT();
    void clean();
#ifdef WITH_RESAMPLE_MAP
    void reSampleMap(const double newRes);
#endif
    void subSampleMap(const int subRes);
    void displayMap();
    mapT& operator=(mapT& rhs);
};

//!The poseT struct stores vehicle pose information.
//#define N_COVAR 36               //XYZ, phi, theta, psi, wy, wz covariance
#define N_COVAR 45               //XYZ, phi, theta, psi, wy, wz covariance
struct poseT {

    double x, y, z;                //North, East, Down position (m)
    double vx, vy, vz, ve;         //Vehicle velocity wrto iceberg, coordinatized in Body Frame(m/s)
    double vw_x, vw_y, vw_z;       //Vehicle velocity wrto water, coordinatized in Body (m/s)
    double vn_x, vn_y, vn_z;       // was Vehicle velocity wrto an inertial frame, coordinatized in Body (m/s)
    // 18 June 2019; reassigned vn_* to pass lastNavPose.x, y and z.
    double wx, wy, wz;             //Vehicle angular velocity wrto an inertial frame, coordinatized in Body (rad/sec)
    double ax, ay, az;             //Vehicle aceleration wrto an inertial frame coordinatized in Body (m/s^2)
    double phi, theta, psi;        //3-2-1 Euler angles relating the B frame to an inertial NED frame (rad).
    double psi_berg, psi_dot_berg; //TRN states

    double time;		  		 				 //Time (s)

    bool dvlValid;							  //Validity flag for dvl motion measurement
    bool gpsValid;							  //Validity flag for GPS measurement
    bool bottomLock;						  //Validity flag for DVL lock onto seafloor

    double covariance[N_COVAR];   //XYZ, phi, theta, psi, wy, wz covariance (passively stable in roll) (see above units)


    poseT();
    poseT& operator=(const poseT& rhs);
    poseT& operator-=(const poseT& rhs);
    poseT& operator+=(const poseT& rhs);

    int   serialize(char *buf, int buflen);
    int unserialize(char *buf, int buflen);
};

//!The measT struct stores sonar measurement information.
struct measT {
    double time;	//Measurement time (s)
    int dataType; //1: DVL, 2: Multibeam, 3: Single Beam,
    //4: Homer Relative Measurement, 5: Imagenex multibeam, 6: Side-looking DVL
    double phi, theta, psi;
    double x, y, z;
    double* covariance;
    double* ranges;
    double* crossTrack;
    double* alongTrack;
    double* altitudes;
    double* alphas;
    bool* measStatus;
    unsigned int ping_number;
    int numMeas;            // aka, number of beams

    // For use in sensors that vary the number of beams (e.g., MB-system)
    int* beamNums;

    measT();
    measT(unsigned int nummeas, int datatype);
    ~measT();
    void clean();
    measT& operator=(const measT& rhs);

    int   serialize(char *buf, int buflen);
    int unserialize(char *buf, int buflen);
};

//!The transformT struct stores a rotation and translation vector.
struct transformT
{
    //TODO: perhaps also add a precomputed transformation matrix?
    double rotation[3];
    double translation[3];
    int dr;
    void displayTransformInfo();
};

//!The sensorT struct stores sonar sensor-specific information.
struct sensorT
{
    char name[256];
    char filename[300];
    int numBeams;
    double percentRangeError;
    double beamWidth;
    int type;  //should match type of associated measT measurements
    transformT* T_bs;			//TODO:Beam to Sensor transform (TODO:rename)

    sensorT();
    explicit sensorT(char* fileName);
    ~sensorT();
    void parseSensorSpecs(char* fileName);
    void displaySensorInfo();
};

//!The vehicleT struct stores vehicle-specific information.
struct vehicleT
{
    char name[256];
    int numSensors;
    double driftRate;
    transformT* T_sv;			//TODO:Sensor to Vehicle transform (TODO:rename)
    sensorT* sensors;

    vehicleT();
    explicit vehicleT(char* fileName);
    ~vehicleT();
    void parseVehicleSpecs(char* fileName);
    void displayVehicleInfo();
};

///////////////////////////////////////////////////////////////
// Definitions for client-server comms
//
//#define TRN_MSG_SIZE 2500       // Increased to accomomdate DeltaT (120 beams)
//#define TRN_MSG_SIZE 30000      // Increased to accomomdate Reson (512 beams)
#define TRN_MSG_SIZE 8000         // Decreased.  30000 started to cause Nacks in sim.

// Payload size does not include header of one char and the message length
// For use in socket reads and writes
#define TRN_PAYLOAD_SIZE (TRN_MSG_SIZE - 2*sizeof(char) - sizeof(unsigned int))

// Structure used to standardize marshalling client-server messages
// Usage:
//   {
//     char comms_buf[TRN_MSG_SIZE];
//     measT  mt;
//     :                                           // Do stuff with measT
//     commsT my_ct(TRN_MEAS, 0, 0, &mt);          // Create commsT
//     my_ct.serialize(comms_buf, TRN_MSG_SIZE);   // Flatten into buffer
//     send(sockfd, comms_buf, TRN_MSG_SIZE, 0);   // Send to server
//     recv(sockfd, comms_buf, TRN_MSG_SIZE, 0);   // Response
//     my_ct.unserialize(comms_buf, TRN_MSG_SIZE); // Look for ack/nack
//     if (my_ct.msg_type == TRN_ACK);             // OK
//     if (my_ct.msg_type == TRN_NACK);            // Problem
//   }
//
struct commsT
{
    char msg_type;
    int  parameter;
    float vdr;
    poseT pt;
    measT mt;
    d_triplet_t xyz_sdev;
    d_triplet_t est_nav_ofs;
    char *mapname;
    char *cfgname;
    char *particlename;
    char *logname;

    commsT();
    explicit commsT(char msg_type);
    commsT(char msg_type, int parameter);
    commsT(char msg_type, int parameter, float vdr);
    commsT(char msg_type, const poseT& pt);
    commsT(char msg_type, int parameter, const measT& mt);
    commsT(char msg_type, int parameter, char *map, char *cfg, char *particles, char *logdir);
    commsT(char msg_type, double x, double y, double z);
    commsT(char msg_type, int parameter, double x, double y, double z);
    commsT(char msg_type, int parameter, double ofs_x, double ofs_y, double ofs_z,
           double sdev_x, double sdev_y, double sdev_z);
    ~commsT();

    char* to_s(char *buf, int buflen); // Write a string representation of the object
    void clean();                     // Clear state
    void release();                   // release resources
    int   serialize(char *buf, int buflen=TRN_MSG_SIZE);
    int unserialize(char *buf, int buflen=TRN_MSG_SIZE);
    static const char *typestr(char type);
};

// Conversations are initiated by the client through request messages.
// The server responds to all requests with either the information requested,
// or a ack/nack.
//
// TRN message are simple flat buffers of length TRN_MSG_SIZE.
// The first 6 bytes always consist of "header" information.
//   buf[0]:
//        char         msg_type;              // Message type (defined below)
//   buf[1]:
//        char         parameter;             // Simple parameter (1, 0, etc.)
//   buf[2*sizeof(char)]:
//        unsigned int msg_len                // Length of remaining message
//   buf[2*sizeof(char)+sizeof(unsigned int)]:
//        char         msg[TRN_PAYLOAD_SIZE]; // Payload (defined below)
//
/////////////////////////////////////////////////////////////////////////
// Message descriptions:
//

#define TRN_INIT 'I'

// Initialization messages.
// Client sends Map file name, vehicle config file name, and filter type.
// msg[0] = map file name, null-terminated string (variable length = M)
// msg[M] = vehicle config file name, null-terminated string (variable length = V)
// msg[M+V] = filter type, one byte.
//
// Example:
//  msg = "mapname.map\0confignamej.cfg\02" (2 is the number 0x02, not '2')
//
// Server responds with an ack if initialization was successful, otherwise nack.

#define TRN_MEAS 'M'

// Multibeam sonar Update message from client to server.
// buf[1] = Sonar measurement type, one byte.
// msg[0] = serialized measT object (variable length = M).
//
// Server responds with an ack if initialization was successful, otherwise nack.

#define TRN_MOTN 'N'

// Motion Update messages from client to server drive the process.
// msg[0] = serialized poseT object (constant length = P).
//
// Server responds with an ack if initialization was successful, otherwise nack.

#define TRN_MLE 'E'

// MLE Estimated Position messages from client to server are requests for data from
// the server. The requests consist of only the message type (the rest of the
// message is ignored). The server reponds with a 'E' type message with a
// serialized poseT structure containing the requested data, or a nack message.
//
// From server:
// msg[0] = serialized poseT object representing MLE location (constance length = E)
//

#define TRN_MMSE 'S'

// MMSE Estimated Position messages from client to server are requests for data from
// the server. The requests consist of only the message type (the rest of the
// message is ignored). The server reponds with a 'S' type message with a
// serialized poseT structure containing the requested data, or a nack message.
//
// From server:
// msg[0] = serialized poseT object representing MMSE location (constance length = S)
//

#define TRN_SET_MW 'W'

// Set modified weighting message.
// buf[1] = valid weighting from enum below
//
// Server responds with an ack if initialization was successful, otherwise nack.

// Possible weighting algorithm to use in Particle filter
enum
{
    TRN_WT_NONE  = 0,       // No weighting modifications at all.
    TRN_WT_NORM  = 1,       // Shandor's original alpha modification.
    TRN_WT_XBEAM = 2,       // Crossbeam with original
    TRN_WT_SUBCL = 3,       // Subcloud  with original
    TRN_FORCE_SUBCL = 4,    // Forced to do Subcloud on every measurement
    TRN_WT_INVAL = 5        // Any value here and above is invalid
};

#define TRN_SET_FR 'F'

// Set filter reinit message.
// buf[0] = 1 for true, 0 for false
//
// Server responds with an ack if initialization was successful, otherwise nack.

#define TRN_SET_IMA 'A'

// Set Interpolate measurement attitude message.
// buf[0] = 1 for true, 0 for false
//
// Server responds with an ack if initialization was successful, otherwise nack.

#define TRN_SET_VDR 'D'

// Set drift rate message.
// msg[0] = float value
//
// Server responds with an ack if initialization was successful, otherwise nack.

#define TRN_SET_MIM 'Q'

// Set Map interpolation method message.
// msg[0] = method
// * 0: nearest-neighbor (no interpolation)
// * 1: bilinear
// * 2: bicubic
// * 3: spline
// * Default = 0;

//
// Server responds with an ack if initialization was successful, otherwise nack.

#define TRN_FILT_GRD 'G'

// Use filter grade message.
// buf[0] = 1 for high, 0 for low
//
// Server responds with an ack if initialization was successful, otherwise nack.

#define TRN_ACK '+'

// Ack last message. Request succeeded.
// Sent when no data other response is expected.
//

#define TRN_NACK '-'

// Nack last message. Request failed.
// Sent when no data other response is expected, or a data request failed.
//

#define TRN_BYE 'B'

// Optional message to close the link.
// No response expected.
//

#define TRN_OUT_MEAS  'O'

// Server returns a boolean indicating whether there are any
// outstanding measurements not incorporated in the PDF in the
// parameter of the returned message (1 = true, 0 = false).
//

#define TRN_LAST_MEAS  'L'

// Server returns a boolean indicating whether the last sonar
// measurement was successfully incorporated in the filter in
// the parameter of the returned message (1 = true, 0 = false).
//

#define TRN_IS_CONV    'C'

// Server returns a boolean indicating whether the TRN filter
// has converged to an estimate in the
// parameter of the returned message (1 = true, 0 = false).
//

#define TRN_FILT_TYPE  'T'

// Server returns a integer filter type describing the current
// TNavFilter type in the
// parameter of the returned message.
//

// Filter Types
enum
{
    TRN_FT_NONE  = 0,       // Undefined.
    TRN_FT_POINTMASS  = 1,  // Point Mass Filter
    TRN_FT_PARTICLE   = 2,  // Particle Filter
    TRN_FT_BANK       = 3,  // Bank of Particle Filters.
};

#define TRN_FILT_STATE 'H'

// Server returns a integer filter state describing the current
// TNavFilter state in the
// parameter of the returned message.
//

#define TRN_N_REINITS 'R'

// Server returns a integer describing the number
// of reinitializations in the
// parameter of the returned message.
//

#define TRN_FILT_REINIT 'r'

// Server returns a integer describing the number
// of reinitializations in the
// parameter of the returned message.
//

#define TRN_FILT_REINIT_BOX 'b'
#define TRN_FILT_REINIT_OFFSET 'o'

// Server sets the values if [xyz]_sdev_init used for filter reinitialization.
// (particle window variance)
//

#define TRN_SET_INITSTDDEVXYZ 'x'
#define TRN_GET_INITSTDDEVXYZ 'X'

// Server sets the values if estNavOffset used for filter reinitialization.
// (particle window variance)
//

#define TRN_IS_INIT 'i'
#define TRN_SET_ESTNAVOFS 'j'
#define TRN_GET_ESTNAVOFS 'J'

// Server instruct TRN to reinitialize its filter
//

#endif
