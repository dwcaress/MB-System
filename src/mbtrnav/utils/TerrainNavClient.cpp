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
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#ifdef _QNX
#include "Exception.h"
#else
#include "myexcept.h"
#endif
#include "TerrainNavClient.h"
#include "TRNUtils.h"

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
//static int transitionMatrix[5][3] = {{0,0,1},{1,1,1},{1,2,2},{2,2,2},{2,2,2}};
//force reinit with well-converged
//static int transitionMatrix[5][3] = {{2,0,1},{1,1,1},{1,2,2},{2,2,2},{2,2,2}};

TerrainNavClient::TerrainNavClient()
  :TerrainNav(),
_logdir(NULL),
_connected(false),
_mbtrn_server_type(true),
_server_ip(NULL),
_sockfd(-1),
_sockport(-1)
{
    this->mapFile = NULL;
    this->vehicleSpecFile = NULL;
    this->saveDirectory = NULL;
    this->particlesFile = NULL;
    this->tNavFilter = NULL;
    this->terrainMap = NULL;
    this->filterType = 1;
    this->mapType = 1;
    this->allowFilterReinits = true;
    memset(_comms_buf,0,TRN_MSG_SIZE);
    _initialized = false;
}

// Just establish a connection to a server that does not need initialization.
// Used to connect to the Trn server in mbtrnpp
TerrainNavClient::TerrainNavClient(char *server_ip, int port)
:TerrainNav(), _connected(false), _mbtrn_server_type(true), _server_ip(NULL),
_sockfd(-1), _sockport(port)
{
  _server_ip = (NULL!=server_ip ? strdup(server_ip) : NULL);
    memset(_comms_buf,0,TRN_MSG_SIZE);
  init_comms();
  _initialized = true;
}

// Use default ctor and initialize only the members needed for client
TerrainNavClient::TerrainNavClient(char *server_ip, int port,
           char *mapName, char *vehicleSpecs, char *particlefile, char *logdir,
           const int &filterType, const int &mapType)
: TerrainNav(), _connected(false), _mbtrn_server_type(true), _sockfd(-1), _sockport(port)
{
  // initialize member variables from the argument list
  // do not load maps, do not call initVariables() as those are uneeded
  this->mapFile = STRDUPNULL(mapName);
  this->vehicleSpecFile = STRDUPNULL(vehicleSpecs);
  this->saveDirectory = STRDUPNULL(logdir);
  this->particlesFile = STRDUPNULL(particlefile);
  this->tNavFilter = NULL;
  this->terrainMap = NULL;
  this->filterType = filterType;
  this->mapType = mapType;
  this->allowFilterReinits = true;
    memset(_comms_buf,0,TRN_MSG_SIZE);

    if(this->mapType == 1) {
        terrainMap = new TerrainMapDEM(this->mapFile);
    } else {
        terrainMap = new TerrainMapOctree(this->mapFile);
    }

  // Initialize TNavConfig
  TNavConfig::instance()->setMapFile(this->mapFile);
  TNavConfig::instance()->setVehicleSpecsFile(this->vehicleSpecFile);
  TNavConfig::instance()->setParticlesFile(this->particlesFile);
  TNavConfig::instance()->setLogDir(this->saveDirectory);
  _server_ip = STRDUPNULL(server_ip);
  _logdir    = (NULL!=logdir ? strdup(logdir) : NULL);
  _initialized = false;

  init_comms();
  init_server();
  logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),
    "TerrainNavClient::Constructor finished.\n");
}

TerrainNavClient::~TerrainNavClient()
{
    close(_sockfd);
    free(_server_ip);
    free(_logdir);
}

//////////////////////////////////////////////////////////////////////
// Initialize connection to server and send state
void TerrainNavClient::init_comms()
{
  if (is_connected()) {
    close(_sockfd);
  }

  _connected = false;

  printf("TerrainNavClient: initializing...mbtrn_server_type? %s\n",
    _mbtrn_server_type ? "yes" : "no");

  // Setup socket to receive the deltaT packets
  if ( (_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 ) {
    printf("TerrinNavClient: can't create socket: %d\n", errno);
  }

  memset((void*)&_server_addr, 0, sizeof(_server_addr));

  // Set up client info
  // Beam former sends data to us
  if(NULL==_server_ip)
  {
     fprintf(stderr,"WARN - %s NULL _server_ip\n",__FUNCTION__);
  }

  _server_addr.sin_family      = AF_INET;
  _server_addr.sin_addr.s_addr = inet_addr(_server_ip);
  _server_addr.sin_port        = htons(_sockport);

  if (connect(_sockfd, (struct sockaddr *)&_server_addr, sizeof(_server_addr))
	      < 0)
  {
    perror("TerrainNavClient: can't connect to server");
    fprintf(stderr, "IP: %s  Port: %d\n", _server_ip, _sockport);
  }
  else
  {
      struct timeval tv={0,0};
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
    if (_connected) {
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100L;

        fd_set svr_r_fd;
        fd_set svr_e_fd;
        FD_ZERO(&svr_r_fd);
        FD_ZERO(&svr_e_fd);
        FD_SET(_sockfd, &svr_r_fd);
        FD_SET(_sockfd, &svr_e_fd);
        int fdmax = _sockfd+1;

        int stat =   select(fdmax, &svr_r_fd, 0, &svr_e_fd, &tv);
        if (stat != -1) {
            for (int i=_sockfd; i<=fdmax; i++)
            {
                if (FD_ISSET(i, &svr_r_fd))
                {
                    if (i==_sockfd)
                    {
                        // client ready to read
                        char temp[5]={0};
                        int test=0;
                        if( (test=recv(_sockfd, temp, 1, MSG_PEEK)) == 0) {
                            if(test==0){
                                fprintf(stderr,"%s - client socket closed by svr fd[%d] test[%d] [%d:%s]\n", __FUNCTION__, i, test, errno, strerror(errno));
                            }
                            ::close(_sockfd);    // Client disconnected
                            _connected = false;
                        } else {
                            if(test<0){
                                if(errno == EAGAIN){
                                    fprintf(stderr,"%s - client nothing to read fd[%d] test[%d] [%d:%s]\n", __FUNCTION__, i, test, errno, strerror(errno));
                                }else{
                                    fprintf(stderr,"%s - client socket error fd[%d] test[%d] [%d:%s]\n", __FUNCTION__, i, test, errno, strerror(errno));
                                    ::close(_sockfd);    // Client disconnected
                                    _connected = false;
                                }
                            } else {
                                // data ready
                                fprintf(stderr,"%s : client ready to read fd[%d] test[%d] [%d/%s]\n", __FUNCTION__, i, test, errno, strerror(errno));
                            }
                        }
                    }
                }
                if (FD_ISSET(i, &svr_e_fd))
                {
                    if (i==_sockfd)
                    {
                        // client ready to read
                        fprintf(stderr,"%s : client socket err fd[%d] [%d/%s]\n", __FUNCTION__, i, errno, strerror(errno));
                        _connected = false;
                    }
                }
            }
        }// else select failed
    }
    return _connected;
}

// throws a TRNConnection exception if TRN has hung up
char TerrainNavClient::get_msg()
{
  bool stat_OK = true;
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
            stat_OK=false;
            break;
        }
        else
        {
        	len += sl;
        }
    }

    if (len > 0) {
      _server_msg.unserialize(_comms_buf, TRN_MSG_SIZE);
        // fprintf(stderr,"%s:%d %s - %s/%d/%d\n",__FILE__,__LINE__,__func__, _server_msg.to_s(_comms_buf, TRN_MSG_SIZE),len,TRN_MSG_SIZE);
    }
    //printf("got %d bytes in response\n", len);

  }
  else
  {
    // No connection
      stat_OK = false;
  }


  // Server hung-up
  if (!stat_OK)
  {
    _connected = false;
    printf("TerrainNavClient::get_msg() - server connection failed/terminated\n");
    throw Exception("TRN Server connection lost");
  }

  return _server_msg.msg_type;

}

#define TRN_CHUNK_SIZE  512

// throws a TRNConnection exception if TRN has hung up
size_t TerrainNavClient::send_msg(commsT& msg)
{
  size_t sl = 0;
  //printf("TerrainNavClient - send_msg(): Sending %s\n",
  //msg.to_s(_comms_buf, sizeof(_comms_buf)));

  // Check to see if client is still connected first
  if (is_connected()) {
    memset(_comms_buf, 0, sizeof(_comms_buf));
    msg.serialize(_comms_buf);

    // Send the whole message
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

    // When interacting with a mbtrn_server (as opposed to a traditional trn_server)
    // send the message in send call. We can get away with this since the messages
    // are relatively short (< 1 kb).
    // Otherwise, to workaround the QNX tcp/ip bug we need to break messages up
    // into 512-byte chunks. The bug comes into play when sending measure updates
    // with many beams.
    if (_mbtrn_server_type)
    {
      sl = send(_sockfd, _comms_buf, sizeof(_comms_buf), 0);
    }
    else
    {
        size_t test=0;
        for (sl = 0; sl < TRN_CHUNK_SIZE;) {
            if((test=send(_sockfd, _comms_buf+sl, TRN_CHUNK_SIZE-sl, 0))>=0){
                sl += test;
            }else{
                fprintf(stderr,"%s:%d: WARN  - send failed [%d/%s]\n",__func__,__LINE__,errno,strerror(errno));
            }
            //printf("server:send_msg - sent %d bytes\n", sl);
        }
        for (sl = TRN_CHUNK_SIZE; sl < sizeof(_comms_buf);) {
            if((test=send(_sockfd, _comms_buf+sl, sizeof(_comms_buf)-sl, 0))>=0){
                sl += test;
            }else{
                fprintf(stderr,"%s:%d: WARN  - send failed [%d/%s]\n",__func__,__LINE__,errno,strerror(errno));
            }
            //printf("server:send_msg - sent %d bytes\n", sl);
        }
    }
  }
  else {
      fprintf(stderr,"TerrainNavClient::send_msg() - Can't send - no server connection!\n");
    throw Exception("TRN Server connection lost");
  }
  return sl;
}

// Initialize the server with map and config filenames.
void TerrainNavClient::init_server()
{
    // filter type and map type encoded in single integer
    // param = filter*100 + map
    char cmt = char(mapType);
    cmt *= 10;
    char cft = char(filterType);
    char param = cmt + cft;

    // Use basenames (no pathnames) of files and folders when connecting
    // to trn-server. Let the server find the files in its environment.
    fprintf(stderr,"TerrainNavClient::init_server() - server initializatiing...\n");
    // TNavConfig::get* use strdup - caller must free
    char *map = TNavConfig::instance()->getMapFile();
    char *veh=TNavConfig::instance()->getVehicleSpecsFile();
    char *par=TNavConfig::instance()->getParticlesFile();
    char *log=TNavConfig::instance()->getLogDir();

    commsT init(TRN_INIT, param,
                TRNUtils::basename(map),
                TRNUtils::basename(veh),
                TRNUtils::basename(par),
                TRNUtils::basename(log)
                );

    free(map);
    free(veh);
    free(par);
    free(log);
    _initialized = false;
    if (0 != send_msg(init)) {
        // Check for ack response
        if (TRN_ACK != get_msg()) {
            fprintf(stderr,"TerrainNavClient::init_server() - server initialization failed!\n");
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
  commsT *pose;
  if (type == 1) {
    pose = new commsT(TRN_MLE, *estimate);
  } else {
    pose = new commsT(TRN_MMSE, *estimate);
  }

  for (int i = 0; i < 2; i++)
    if (0 != send_msg(*pose)) {
      // Check for response
      char ret_msg = get_msg();
      if (TRN_NACK == ret_msg)
      {
        printf("TerrainNavClient::estimatePose() - server NACKed\n");
        continue;
      }
      else if (TRN_MLE != ret_msg && TRN_MMSE != ret_msg) {
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
  commsT motn(TRN_MOTN, *incomingNav);

  for (int i = 0; i < 2; i++)
    if (0 != send_msg(motn)) {
      // Check for response
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
  commsT meas(TRN_MEAS, type, *incomingMeas);

  for (int i = 0; i < 2; i++)
    if (0 != send_msg(meas)) {
      // Check for response
      char ret_msg = get_msg();

      // The return message is the measT object with updated alphas
      // if (TRN_ACK != ret_msg) {
      if (TRN_MEAS != ret_msg) {
        printf("TerrainNavClient::measUpdate() - server choked on measure update\n");
        printf("%s\n", _server_msg.to_s(_comms_buf, TRN_MSG_SIZE));
     }
      else
      {
        //printf("TerrainNavClient::recv'd TRN_MEAS %f\n", _server_msg.mt.alphas[0]);
        // Unpack the returned measT, then copy the contents into incomingMeas
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

void TerrainNavClient::reinitFilterOffset(const bool lowInfoTransition,
                                       double ofs_x, double ofs_y, double ofs_z)
{
    commsT reinit(TRN_FILT_REINIT_OFFSET,(lowInfoTransition?1:0),
                  ofs_x, ofs_y, ofs_z);

    reinit.serialize(_comms_buf);

    if(!requestAndConfirm(reinit, TRN_ACK)) {
        printf("TerrainNavClient::reinitFilterBox() - comms failure\n");
    }
}

void TerrainNavClient::reinitFilterBox(const bool lowInfoTransition,
                                       double ofs_x, double ofs_y, double ofs_z,
                                       double sdev_x, double sdev_y, double sdev_z)
{
    commsT reinit(TRN_FILT_REINIT_BOX,(lowInfoTransition?1:0),
                  ofs_x, ofs_y, ofs_z,
                  sdev_x, sdev_y, sdev_z);

    reinit.serialize(_comms_buf);

    if(!requestAndConfirm(reinit, TRN_ACK)) {
        printf("TerrainNavClient::reinitFilterBox() - comms failure\n");
    }
}


void TerrainNavClient::setEstNavOffset(double offset_x, double offset_y, double offset_z)
{
    commsT seteno(TRN_SET_ESTNAVOFS, offset_x, offset_y, offset_z);

    seteno.serialize(_comms_buf);

    if (!requestAndConfirm(seteno, TRN_ACK)){
        printf("TerrainNavClient::setEstNavOffset() - comms failure\n");
    }
    return;
}

d_triplet_t *TerrainNavClient::getEstNavOffset(d_triplet_t *dest)
{
    d_triplet_t *retval = NULL;
    commsT seteno(TRN_GET_ESTNAVOFS, 0., 0., 0.);
    seteno.serialize(_comms_buf);

    if (requestAndConfirm(seteno, TRN_GET_ESTNAVOFS)){
        if(NULL != dest){
            if(_server_msg.msg_type == TRN_GET_ESTNAVOFS){
                memcpy(dest, &_server_msg.est_nav_ofs, sizeof(d_triplet_t));
                retval = dest;
            }
        }
    }else{
        printf("TerrainNavClient::getEstNavOffset() - comms failure\n");
    }

    return retval;
}

void TerrainNavClient::setInitStdDevXYZ(double sdev_x, double sdev_y, double sdev_z)
{
    commsT seteno(TRN_SET_INITSTDDEVXYZ, sdev_x, sdev_y, sdev_z);

    seteno.serialize(_comms_buf);

    if (!requestAndConfirm(seteno, TRN_ACK)){
        printf("TerrainNavClient::setEstNavOffset() - comms failure\n");
    }
    return;
}

d_triplet_t *TerrainNavClient::getInitStdDevXYZ(d_triplet_t *dest)
{
    d_triplet_t *retval = NULL;
    commsT seteno(TRN_GET_INITSTDDEVXYZ, 0., 0., 0.);

    seteno.serialize(_comms_buf);

    if (requestAndConfirm(seteno, TRN_GET_INITSTDDEVXYZ)){
        if(NULL != dest){
            if(_server_msg.msg_type == TRN_GET_INITSTDDEVXYZ){
                memcpy(dest, &_server_msg.xyz_sdev, sizeof(d_triplet_t));
                retval = dest;
            }else{
                fprintf(stderr,"%s:%d ERR msg type[%d]\n",__func__,__LINE__,_server_msg.msg_type);
            }
        }
    }else{
        printf("TerrainNavClient::getEstNavOffset() - comms failure\n");
    }

    return retval;
}

void TerrainNavClient::setInitVars(InitVars *init_vars)
{
    fprintf(stderr,"%s:%d - %s not implemented\n",__FILE__,__LINE__,__func__);
    return;
}


bool TerrainNavClient::requestAndConfirm(commsT& msg,
					    char expected_ret_type)
{
    const int MAX_SEND_ATTEMPTS = 3;

    for (int i = 0; i < MAX_SEND_ATTEMPTS; i++) {
        size_t test=0;
        if ( (test=send_msg(msg)) !=0 ) {
            // Check the response if any
            char ret_msg = get_msg();
            if (expected_ret_type != ret_msg) {
                fprintf(stderr,"%s:%d %s - unexpected response %d/%d\n",__FILE__,__LINE__,__func__,expected_ret_type, ret_msg);
                //printf("%d: TerrainNavClient::requestAndConfirm()-unexpected response",
                //i+1);
                //printf("%s", _server_msg.to_s(_comms_buf, TRN_MSG_SIZE));
            }else{
                return true;
            }
        }else{
            fprintf(stderr,"%s:%d %s - ERR send_msg ret [%zu]\n",__FILE__,__LINE__,__func__,test);
        }
    }

    return false;
}

