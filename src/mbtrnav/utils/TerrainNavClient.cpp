/* FILENAME      : TerrainNavClient.cpp
 * AUTHOR        : Rich Henthorn
 * DATE          : 04/17/13
 * 
 * LAST MODIFIED : 
 * MODIFIED BY   : 
 * -----------------------------------------------------------------------------
 * Modification History
 * -----------------------------------------------------------------------------
 ******************************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#ifdef _QNX
#include "Exception.h"
#else
#include "myexcept.h"
#endif
#include "TRNUtils.h"
#include "TerrainNavClient.h"

/******************************************************************************
 TRANSITION MATRIX
 State 0: Well localized
 State 1: Localizing - medium uncertainty
 State 2: High uncertainty

 Trigger 0: North/East Variance < MIN_FILTER_VAR
 Trigger 1: MIN_FILTER_VAR+VAR_MARGIN < North/East Variance < MAX_FILTER_VAR-
            VAR_MARGIN and filter is either in state 0 or 2
 Trigger 2: North/East Variance > MAX_FILTER_VAR
 Trigger 3: Missing valid range measurements for > MAX_MEAS_OUTAGE OR
            Vehicle does not have bottom lock for > MAX_VEL_OUTAGE
 Trigger 4: Vehicle has been over flat terrain for too long
******************************************************************************/
//normal case
static int transitionMatrix[5][3] = {{0,0,1},{1,1,1},{1,2,2},{2,2,2},{2,2,2}};
//force reinit with well-converged
//static int transitionMatrix[5][3] = {{2,0,1},{1,1,1},{1,2,2},{2,2,2},{2,2,2}};

TerrainNavClient::TerrainNavClient(char *server_ip, int port,
				   char *mapName, char *vehicleSpecs, char *particlefile, char *logdir,
				   const int &filterType, const int &mapType)
  : TerrainNav(mapName, vehicleSpecs, particlefile, filterType, mapType, logdir),
    _server_ip(NULL), _sockport(port), _connected(false)
{
  _server_ip = strdup(server_ip);
  _logdir    = strdup(logdir);
  _initialized = false;

  init_comms();
  init_server();

}

TerrainNavClient::~TerrainNavClient()
{
  close(_sockfd);
  if (_server_ip) delete _server_ip;
}

//////////////////////////////////////////////////////////////////////
// Initialize connection to server and send state
void TerrainNavClient::init_comms()
{
  if (is_connected()) {
    close(_sockfd);
  }

  _connected = false;

  printf("TerrainNavClient: initializing...\n");

  // Setup socket to receive the deltaT packets
  if ( (_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 ) {
    printf("TerrinNavClient: can't create socket: %d\n", errno);
  }

  memset((void*)&_server_addr, 0, sizeof(_server_addr));

  // Set up client info
  // Beam former sends data to us
  //
  _server_addr.sin_family      = AF_INET;
  _server_addr.sin_addr.s_addr = inet_addr(_server_ip);
  _server_addr.sin_port        = htons(_sockport);

  if (connect(_sockfd, (struct sockaddr *)&_server_addr, sizeof(_server_addr))
	      < 0) {
    printf("TerrainNavClient: can't connect to server: %d\n", errno);
  }
  else {
    struct timeval tv;
    tv.tv_sec = 150;
    tv.tv_usec = 0;
    setsockopt(_sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    _connected = true;
    printf("TerrainNavClient::init_comms() - Connected with message size %d!\n", TRN_MSG_SIZE);
  }
}

bool TerrainNavClient::is_connected()
{
  // Don't bother checking unless we have initialized comms successfully
  //
  if (_connected) {
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100L;

    fd_set servfd;
    FD_ZERO(&servfd);
    FD_SET(_sockfd, &servfd);
    char temp[5];
    int nready = select(_sockfd+1, &servfd, 0, 0, &tv);
    if (nready > 0) {
      if (0 == recv(_sockfd, temp, 1, MSG_PEEK)) {
	       printf("TerrainNavClient::is_connected() - Server closed connection!\n");
	       ::close(_sockfd);    // Client disconnected
         _connected = false; 
      }
      else {
	       _connected = true;   // Connected and there is data to read 
      }
    }
    else {
      _connected = true;     // Connected but no data to read
    }
  }

  return _connected;
}

// throws a TRNConnection exception if TRN has hung up
//
char TerrainNavClient::get_msg()
{
  bool Ok = true;
  _server_msg.msg_type = 0;
  if (is_connected())
  {

    int ntries = 3;
    int len = 0;
    for (len = 0; len < TRN_MSG_SIZE;) {
      int sl = recv(_sockfd, _comms_buf+len, TRN_MSG_SIZE-len, 0);
      int saveErrno=errno;
        if (sl <= 0)
        {
            //printf("TerrainNavClient::get_msg() - timeout in recv. errno = %d\n",
            //errno);
            //printf("TerrainNavClient::get_msg() - timeout in recv after %d bytes\n",
            //len);
            perror("TerrainNavClient::get_msg");
            printf("\n\t\tTerrainNavClient::get_msg, errno = %d [%s]\n", saveErrno,strerror(saveErrno));
            
            //if ((sl == 0 || errno == EINTR) && --ntries > 0) { // try again
            if ((saveErrno == EINTR) && --ntries > 0)
            { // try again
                //printf("Trying to get rest of message after an interrupt.\n");
                continue;
            }
            
            // Check to see if server hung up
            if (sl==0) {
                // server orderly shutdown
                printf("\n\t\tTerrainNavClient::recv return 0 - server shut down\n");
            }
            
            // disconnected or unrecoverable error, quit loop
            Ok=false;
            break;
        }
        else
        {
        	len += sl;
        }
    }

    if (len > 0) {
      _server_msg.unserialize(_comms_buf, TRN_MSG_SIZE);
      //printf("TerrainNavClient got %s\n", _server_msg.to_s(_comms_buf, TRN_MSG_SIZE));
    }
    //printf("got %d bytes in response\n", len);

  }
  else
  {
    // No connection
    //
    Ok = false;
  }


  // Server hung-up
  //
  if (!Ok)
  {
    _connected = false;
    printf("TerrainNavClient::get_msg() - server connection failed/terminated\n");
    throw Exception("TRN Server connection lost");
  }

  return _server_msg.msg_type;

}

#define TRN_CHUNK_SIZE  512

// throws a TRNConnection exception if TRN has hung up
//
int TerrainNavClient::send_msg(commsT& msg)
{
  int sl = 0;
  //printf("TerrainNavClient - send_msg(): Sending %s\n",
  //msg.to_s(_comms_buf, sizeof(_comms_buf)));

  // Check to see if client is still connected first
  //
  if (is_connected()) {
    memset(_comms_buf, 0, sizeof(_comms_buf));
    msg.serialize(_comms_buf);

    // Send the whole message
    //
#if 0
    if (msg.msg_type == TRN_MEAS)
    {
      int i;
      printf("client\n"); for (i = 0; i < msg.mt.numMeas; i++) printf("%.1f  ", msg.mt.altitudes[i]);
      printf("\n"); for (i=493; i<543; i++) printf("%d:%2x ", i, _comms_buf[i]);
      printf("\n"); for (i=543; i<593; i++) printf("%d:%2x ", i, _comms_buf[i]);
      printf("\n"); for (i=593; i<643; i++) printf("%d:%2x ", i, _comms_buf[i]);
      printf("\n"); for (i=643; i<693; i++) printf("%d:%2x ", i, _comms_buf[i]);
      printf("\n");
    }
#endif

    for (sl = 0; sl < TRN_CHUNK_SIZE;) {
      sl += send(_sockfd, _comms_buf+sl, TRN_CHUNK_SIZE-sl, 0);
      //printf("server:send_msg - sent %d bytes\n", sl);
    }
    for (sl = TRN_CHUNK_SIZE; sl < sizeof(_comms_buf);) {
      sl += send(_sockfd, _comms_buf+sl, sizeof(_comms_buf)-sl, 0);
      //printf("server:send_msg - sent %d bytes\n", sl);
    }
  }
  else {
    printf("TerrainNavClient::send_msg() - Can't send - no server connection!\n");
    throw Exception("TRN Server connection lost");
  }
  return sl;
}

// Initialize the server with map and config filenames. 
//
void TerrainNavClient::init_server()
{
  // filter type and map type encoded in single integer
  // param = filter*100 + map
  //
  char cmt = char(mapType);
  cmt *= 10;
  char cft = char(filterType);
  char param = cmt + cft;

  printf("TerrainNavClient::init_server() - server initializatiing...\n");
//  commsT init(TRN_INIT, param, this->mapFile, this->vehicleSpecFile, this->particlesFile, _logdir);
  commsT init(TRN_INIT, param,
    TRNUtils::basename(this->mapFile), TRNUtils::basename(this->vehicleSpecFile),
    TRNUtils::basename(this->particlesFile), TRNUtils::basename(_logdir));

  _initialized = false;
  if (0 != send_msg(init)) {
    // Check for ack response
    //
    if (TRN_ACK != get_msg()) {
      printf("TerrainNavClient::init_server() - server initialization failed!\n");
      _initialized = false;
    }
    else {
      _initialized = true;
    }
  }
  else {
    _initialized = false;
  }
  if( !_initialized ) throw Exception("TRN Server initialization failed!");
}

void TerrainNavClient::estimatePose(poseT* estimate, const int &type)
{
  // Send request to server for estimate
  // Type = 1 is MLE, 2 is MMSE
  //
  commsT *pose;
  if (type == 1)
    pose = new commsT(TRN_MLE, *estimate);
  else
    pose = new commsT(TRN_MMSE, *estimate);

  for (int i = 0; i < 2; i++)
    if (0 != send_msg(*pose)) {
      // Check for response
      //
      char ret_msg = get_msg();
      if (TRN_MLE != ret_msg && TRN_MMSE != ret_msg) {
        printf("TerrainNavClient::estimatePose() - server choked on estimate\n");
        printf("%s\n", _server_msg.to_s(_comms_buf, TRN_MSG_SIZE));
      }
      else {
        *estimate = _server_msg.pt;
        break;
      }
    }

  delete pose;
  return;   
}

void TerrainNavClient::motionUpdate(poseT* incomingNav)
{
  // Send measurement update to server
  //
  commsT motn(TRN_MOTN, *incomingNav);

  for (int i = 0; i < 2; i++)
    if (0 != send_msg(motn)) {
      // Check for response
      //
      char ret_msg = get_msg();
      if (TRN_ACK != ret_msg) {
        printf("TerrainNavClient::motionUpdate() - server choked on motion update\n");
        printf("%s\n", _server_msg.to_s(_comms_buf, TRN_MSG_SIZE));
      }
      else break;
    }

  return;
}

void TerrainNavClient::measUpdate(measT* incomingMeas, const int &type) 
{
  // Send measurement update to server
  //
  commsT meas(TRN_MEAS, type, *incomingMeas);

  for (int i = 0; i < 2; i++)
    if (0 != send_msg(meas)) {
      // Check for response
      //
      char ret_msg = get_msg();

      // The return message is the measT object with updated alphas
      // 
      // if (TRN_ACK != ret_msg) {
      if (TRN_MEAS != ret_msg) {
        printf("TerrainNavClient::measUpdate() - server choked on measure update\n");
        printf("%s\n", _server_msg.to_s(_comms_buf, TRN_MSG_SIZE));
     }
      else
      {
        //printf("TerrainNavClient::recv'd TRN_MEAS %f\n", _server_msg.mt.alphas[0]);
        // Unpack the returned measT, then copy the contents into incomingMeas
        // 
        *incomingMeas = _server_msg.mt;        
        break;
      }
    }

  return;
}


/* Function: outstandingMeas
 * Usage: tercom->outstandingMeas;
 * -------------------------------------------------------------------------*/
/*! Returns a boolean indicating if there are any measurements 
 * not yet incorporated into the PDF and waiting for more recent inertial
 * measurement data. 
 */
bool TerrainNavClient::outstandingMeas()
{
  commsT out(TRN_OUT_MEAS);
  //printf("%s\n", out.to_s(_comms_buf, TRN_MSG_SIZE));
  out.serialize(_comms_buf);

  for (int i = 0; i < 2; i++)
    if (0 != send_msg(out)) {
      // Check for response
      //
      char ret_msg = get_msg();
      if (TRN_ACK != ret_msg) {
        printf("TerrainNavClient::outstandingMeas() - server choked on request\n");
        printf("%s\n", _server_msg.to_s(_comms_buf, TRN_MSG_SIZE));
      }
      else {
        return (_server_msg.parameter > 0);
      }
    }

  return false;
}

/* Function: lastMeasSuccessful
 * Usage: tercom->lastMeasSuccessful();
 * -------------------------------------------------------------------------*/
/*! Returns a boolean indicating if the last sonar measurement 
 * was successfully incorporated into the filter or not.
 */
bool TerrainNavClient::lastMeasSuccessful()
{
  commsT last(TRN_LAST_MEAS);
  //printf("%s\n", last.to_s(_comms_buf, TRN_MSG_SIZE));
  last.serialize(_comms_buf);

  for (int i = 0; i < 2; i++)
    if (0 != send_msg(last)) {
      // Check for response
      //
      char ret_msg = get_msg();
      if (TRN_ACK != ret_msg) {
        printf("TerrainNavClient::lastMeasSuccessful() - server choked on request\n");
        printf("%s\n", _server_msg.to_s(_comms_buf, TRN_MSG_SIZE));
      }
      else {
        return (_server_msg.parameter > 0);
      }
    }

  return false;
}


/* Function: setInterpMeasAttitude()
 * Usage: tercom->setInterpMeasAttitude(1);
 * -------------------------------------------------------------------------*/
/*! Sets a boolean indicating if the sonar measurement attitude 
 * information should be determined from interpolated inertial poses.
 * Default = 0;
 */
void TerrainNavClient::setInterpMeasAttitude(bool set)
{
  int iset = set? 1: 0;

  commsT ima(TRN_SET_IMA, iset);
  //printf("%s\n", ima.to_s(_comms_buf, TRN_MSG_SIZE));
  ima.serialize(_comms_buf);

  for (int i = 0; i < 2; i++)
    if (0 != send_msg(ima)) {
      // Check for response
      //
      char ret_msg = get_msg();
      if (TRN_ACK != ret_msg) {
        printf("TerrainNavClient::setInterpMeasAttitude() - server choked on request\n");
        printf("%s\n", _server_msg.to_s(_comms_buf, TRN_MSG_SIZE));
      }
      else break;
    }

  return;
}


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
void TerrainNavClient::setMapInterpMethod(const int &type)
{
  //printf("Setting MIM to %d\n", type);
  commsT mim(TRN_SET_MIM, 0);
  //printf("%s\n", mim.to_s(_comms_buf, TRN_MSG_SIZE));
  mim.serialize(_comms_buf);

  for (int i = 0; i < 2; i++)
    if (0 != send_msg(mim)) {
      // Check for response
      //
      char ret_msg = get_msg();
      if (TRN_ACK != ret_msg) {
        printf("TerrainNavClient::setMapInterpMethod() - server choked on request\n");
        printf("%s\n", _server_msg.to_s(_comms_buf, TRN_MSG_SIZE));
      }
    }
    else break;

  return;
}


/* Function: setVehicleDriftRate()
 * Usage: tercom->setVehicleDriftRate(5)
 * -------------------------------------------------------------------------*/
/*! Sets the vehicle inertial drift rate parameter.  The default value is
 * determined by the vehicle specification sheet.  The driftRate parameter
 * is given in units of % drift in meters/sec.
 */
void TerrainNavClient::setVehicleDriftRate(const double &driftRate)
{
  commsT vdr(TRN_SET_VDR, 0, driftRate);
  //printf("%s\n", vdr.to_s(_comms_buf, TRN_MSG_SIZE));
  vdr.serialize(_comms_buf);

  for (int i = 0; i < 2; i++)
    if (0 != send_msg(vdr)) {
      // Check for response
      //
      char ret_msg = get_msg();
      if (TRN_ACK != ret_msg) {
        printf("TerrainNavClient::setVehicleDriftRate() - server choked on request\n");
        printf("%s\n", _server_msg.to_s(_comms_buf, TRN_MSG_SIZE));
      }
    }
    else break;

  return;
}


/* Function: isConverged()
 * Usage: converged = tercom->isConverged();
 * ------------------------------------------------------------------------*/
/*! Returns a boolean indicating if the terrain navigation filter has 
 * converged to an estimate.
 */
bool TerrainNavClient::isConverged()
{
  commsT conv(TRN_IS_CONV);
  //printf("%s\n", conv.to_s(_comms_buf, TRN_MSG_SIZE));
  conv.serialize(_comms_buf);

  for (int i = 0; i < 2; i++)
    if (0 != send_msg(conv)) {
      // Check for response
      //
      char ret_msg = get_msg();
      if (TRN_ACK != ret_msg) {
        printf("TerrainNavClient::isConverged() - server choked on request\n");
        printf("%s\n", _server_msg.to_s(_comms_buf, TRN_MSG_SIZE));
      }
      else {
        return (_server_msg.parameter > 0);
      }
    }

  return false;
}


/* Function: useLowGradeFilter()
 * Usage: tercom->useLowGradeFilter()
 * -------------------------------------------------------------------------*/
/*! Force filter settings for low grade system:
 * 7DOF system with ALLOW_ATTITUDE_SEARCH=1, DEAD_RECKON=1, SEARCH_GYRO=1
 */
void TerrainNavClient::useLowGradeFilter()
{
  commsT low(TRN_FILT_GRD, 0);
  //printf("%s\n", low.to_s(_comms_buf, TRN_MSG_SIZE));
  low.serialize(_comms_buf);

  for (int i = 0; i < 2; i++)
    if (0 != send_msg(low)) {
      // Check for response
      //
      char ret_msg = get_msg();
      if (TRN_ACK != ret_msg) {
        printf("TerrainNavClient::useLowGradeFilter() - server choked on request\n");
        printf("%s\n", _server_msg.to_s(_comms_buf, TRN_MSG_SIZE));
      }
    }
    else break;
}


/* Function: useHighGradeFilter()
 * Usage: tercom->useHighGradeFilter()
 * -------------------------------------------------------------------------*/
/*! Force filter settings for low grade system:
 * 7DOF system with ALLOW_ATTITUDE_SEARCH=0, DEAD_RECKON=0, SEARCH_GYRO=0
 */
void TerrainNavClient::useHighGradeFilter()
{
  commsT high(TRN_FILT_GRD, 1);
  //printf("%s\n", high.to_s(_comms_buf, TRN_MSG_SIZE));
  high.serialize(_comms_buf);

  for (int i = 0; i < 2; i++)
    if (0 != send_msg(high)) {
      // Check for response
      //
      char ret_msg = get_msg();
      if (TRN_ACK != ret_msg) {
        printf("TerrainNavClient::useHighGradeFilter() - server choked on request\n");
        printf("%s\n", _server_msg.to_s(_comms_buf, TRN_MSG_SIZE));
      }
    }
    else break;
}


/* Function: setFilterReinit(allow)
 * Usage: tercom->setFilterReinit(allow)
 * -------------------------------------------------------------------------*/
/*! Overwrite boolean allowFilterReinits with input argument, allow.
 */
void TerrainNavClient::setFilterReinit(const bool allow)
{
  int iset = allow? 1: 0;

  commsT reinit(TRN_SET_FR, iset);
  //printf("%s\n", reinit.to_s(_comms_buf, TRN_MSG_SIZE));
  reinit.serialize(_comms_buf);

  for (int i = 0; i < 2; i++)
    if (0 != send_msg(reinit)) {
      // Check for response
      //
      char ret_msg = get_msg();
      if (TRN_ACK != ret_msg) {
        printf("TerrainNavClient::setFilterReinit() - server choked on request\n");
        printf("%s\n", _server_msg.to_s(_comms_buf, TRN_MSG_SIZE));
      }
    }
    else break;

  return;
}

/* Function: setModifiedWeighting(use)
 * Usage: tercom->tNavFilter->setModifiedWeighting(use)
 * -------------------------------------------------------------------------*/
/*! Overwrite boolean useModifiedWeighting with input argument, use.
 */
void TerrainNavClient::setModifiedWeighting(const int use)
{ 
  commsT smw(TRN_SET_MW, use);
  //printf("%s\n", smw.to_s(_comms_buf, TRN_MSG_SIZE));
  smw.serialize(_comms_buf);
  
  for (int i = 0; i < 2; i++)
    if (0 != send_msg(smw)) {
      // Check for response
      //
      char ret_msg = get_msg();
      if (TRN_ACK != ret_msg) {
        printf("TerrainNavClient::setModifiedWeighting() - server choked on request\n");
        printf("%s\n", _server_msg.to_s(_comms_buf, TRN_MSG_SIZE));
      }
    }
    else break;

  return;
}
  

/* Function: getFilterState()
 * Usage: filterType = tercom->getFilterState();
 * ------------------------------------------------------------------------*/
/*! Returns the integer filterState describing the current filter state.
 */
int TerrainNavClient::getFilterState()
{
  commsT state(TRN_FILT_STATE);
  //printf("%s\n", state.to_s(_comms_buf, TRN_MSG_SIZE));
  state.serialize(_comms_buf);

  for (int i = 0; i < 2; i++)
    if (0 != send_msg(state)) {
      // Check for response
      //
      char ret_msg = get_msg();
      if (TRN_ACK != ret_msg) {
        printf("TerrainNavClient::getFilterState() - server choked on request\n");
        printf("%s\n", _server_msg.to_s(_comms_buf, TRN_MSG_SIZE));
      }
      else {
        return _server_msg.parameter;
      }
    }

  return 0;
}


/* Function: getNumReinits()
 * Usage: filterType = tercom->getNumReinits();
 * ------------------------------------------------------------------------*/
/*! Returns the integer numReinits describing the number of reinitializations.
 */
int TerrainNavClient::getNumReinits()
{
  commsT reinits(TRN_N_REINITS);
  //printf("%s\n", reinits.to_s(_comms_buf, TRN_MSG_SIZE));
  reinits.serialize(_comms_buf);

  if (requestAndConfirm(reinits, TRN_ACK))
    return _server_msg.parameter;
  else
    printf("TerrainNavClient::getNumReinits() - comms failure\n");

  return 0;
}

void TerrainNavClient::reinitFilter(const bool lowInfoTransition)
{
  commsT reinit(TRN_FILT_REINIT);
  //printf("%s\n", reinits.to_s(_comms_buf, TRN_MSG_SIZE));
  reinit.serialize(_comms_buf);
  
  if(!requestAndConfirm(reinit, TRN_ACK)) {
    printf("TerrainNavClient::reinitFilter() - comms failure\n");
  }
}

bool TerrainNavClient::requestAndConfirm(commsT& msg,
					    char expected_ret_type)
{
  const int MAX_SEND_ATTEMPTS = 3;

  for (int i = 0; i < MAX_SEND_ATTEMPTS; i++) {
    if (0 != send_msg(msg)) {
      // Check the response if any
      //
      char ret_msg = get_msg();
      if (expected_ret_type != ret_msg) {
        //printf("%d: TerrainNavClient::requestAndConfirm()-unexpected response",
        //i+1);
        //printf("%s", _server_msg.to_s(_comms_buf, TRN_MSG_SIZE));
      }
      else return true;
    }
  }

  return false;
}

