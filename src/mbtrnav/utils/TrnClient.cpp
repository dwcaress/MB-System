// Copyright (c) 2017 MBARI
// MBARI Proprietary Information. All rights reserved.
// ***************************************************************************
// Summary  : TRn test client
// Filename : TrnClient.cpp
// Author   : headley
// Project  :
// Version  : 1.0
// Created  : 24oct2019
// Modified :
// Archived :
// Modification History:
// Began with a copy of test_client as there is a lot of stuff to reuse

// TrnClient extends TerrainNavClient (and TerrainNav).
// It provides better control over initialization than TerrainNavClient,
// which calls initializes communications, logs, etc. in most constructors.
// In some contexts, this is not desirable, since configuration steps may
// be required after instantiation. This behavior varies among its constructors.
//
// TrnClient consistently provides minimal object initialization, and delegates
// configuration and connection to the calling application.
//
// TrnClient and TRN host are independent; the TRN host (trn_server, e.g.)
// may be on different machines, and configuration and log files may be in
// different filesystem locations.
// The TrnClient is a message passing front end to the TRN host. 
// As such, it does not create directories, or need to be fully configured.
//
// TrnClient uses config file (e.g. terrainAid.cfg)
// to populate a TrnAttr instance. TrnAttr content is used to configure the TRN host.
//   mapFile, vehicleSpecFile, particlesFile (optional), sessionPrefix
//
// The TRN host uses environment variables (or other config) to determine the path
// to these files on it's filesystem.
//
// The session directory will be created by the TRN host (trn_server) when it
// recieves an init message. The session prefix is used by the TRN host
// to create the session directory (e.g. <prefix>-TRN.dd).
// The TRN host copies TRN configuration files to the session directory:
//   vehicleSpecs, sensorSpecs, particlesFile
// and stores TRN logs there.
//
// TrnClient generates the TRN session prefix that the TRN host uses
// to name it's session data directories. Typically, this is something like
// <prefix>-TRN.<dd> where prefix is YYYY-JJJ and dd are session numbers
// 01,02...


#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <ostream>
#include <sstream>

#include "TrnClient.h"
#include "MathP.h"
#include "NavUtils.h"
#include "TimeTag.h"
#include "FloatData.h"
#include "IntegerData.h"
#include "DataField.h"
#include "TRNUtils.h"

#define MAPNAME_BUF_BYTES 1024
#define CFGNAME_BUF_BYTES 1024
#define VEHICLENAME_BUF_BYTES 1024
#define PARTICLENAME_BUF_BYTES 1024
#define SESSIONDIR_BUF_BYTES 1024

// Common to QNX and NIX versions
// 
TrnClient::TrnClient()
: TerrainNavClient()
, verbose(0)
, _quit_ref(NULL)
, _cfg_file(NULL)
, _sessionPrefix(NULL)
{
//    fprintf(stderr,"%s:%d - <<<<< DFL CTOR >>>>>>\n", __func__, __LINE__);
    _initialized = false;

}

TrnClient::TrnClient(const char *host, int port)
: TerrainNavClient()
, verbose(0)
, _quit_ref(NULL)
, _cfg_file(NULL)
, _sessionPrefix(NULL)
{
//    fprintf(stderr,"%s:%d - <<<<< INIT CTOR (host/port) >>>>>\n", __func__, __LINE__);

    if (NULL != host){
        if(_server_ip != NULL)
            free(_server_ip);
        _server_ip = strdup(host);
        _sockport = port;
    }

    _initialized = false;
}

TrnClient::TrnClient(const TrnClient& other)
: TerrainNavClient(other)
{
//    fprintf(stderr,"%s:%d - <<<<< COPY CTOR >>>>>\n", __func__, __LINE__);
    this->verbose = other.verbose;
    this->_quit_ref = other._quit_ref;
    this->_cfg_file = STRDUPNULL(other._cfg_file);
    this->_trn_attr = other._trn_attr;

    this->_connected = other._connected;
    this->_mbtrn_server_type = other._mbtrn_server_type;
    this->_server_ip = STRDUPNULL(other._server_ip);
    this->_sockfd = other._sockfd;
    this->_sessionPrefix = STRDUPNULL(other._sessionPrefix);
    this->_sockfd = other._sockfd;
    this->_sockport = other._sockport;
    std::memcpy(&_server_addr, &other._server_addr, sizeof(struct sockaddr_in));
    this->_server_msg = other._server_msg;
    std::memcpy(this->_comms_buf, other._comms_buf, TRN_MSG_SIZE);
}

TrnClient::~TrnClient()
{
//    fprintf(stderr,"%s:%d - <<<<< DTOR >>>>>\n", __func__, __LINE__);
    if(_cfg_file != NULL)
        free(_cfg_file);
    if(_sessionPrefix != NULL)
        free(_sessionPrefix);
    // base class closes _sockfd and frees _server_ip
}

void TrnClient::chkSetString(char **dest, const char *src)
{
    if(dest==NULL)
        return;

    // free destination if allocated
    if(*dest != NULL)
        free(*dest);

    // set destination with copy of source
    if(src == NULL)
        *dest = NULL;
    else
        *dest = strdup(src);
}

// Reads cfg_file, parses it to set TrnAttr and TrnClient members
//
// The following TrnAttr members are passed to the server:
//  mapName         passed in init
//  particlesName   passed in init
//  vehicleCfgName  passed in init
//  sessionPrefix   passed in init
//  mapType         passed in init
//  filterType      passed in init
//  allowFilterReinits
//
// The following TrnAttr members are used to set TrnClient members:
//  terrainNavServer  TerrainNavClient::_server_ip
//  terrainNavPort    TerrainNavClient::_sockport
//  mapType           TerrainNav::mapType
//  filterType        TerrainNav::filterType
//
// The following TrnClient members are set
//  _cfg_file
//  _sessionPrefix
//

int TrnClient::loadCfgAttributes(const char *cfg_file)
{
    // set configuation file to load from
    // set TrnClient and TrnAttr _cfg_file members
    if(NULL != cfg_file){
        // set _cfg_file member and use that
        TrnAttr::chkSetString(&this->_cfg_file, cfg_file);
    }else{
        // use default path ($TRN_CONFIGDIR/terrainAid.cfg or ./terrainAid.cfg)
        const char *cfg_dir = getenv("TRN_DATAFILES");
        if(cfg_dir == NULL)
            cfg_dir = ".";

        size_t cb_len = strlen(cfg_dir) + strlen("/terrainAid.cfg") + 1;
        char cfg_buf[cb_len];
        memset(cfg_buf, 0, cb_len);

        snprintf(cfg_buf, cb_len, "%s/terrainAid.cfg", cfg_dir);
        TrnAttr::chkSetString(&this->_cfg_file, cfg_buf);
    }

    fprintf(stderr, "%s:%d _cfg_file [%s]\n", __func__, __LINE__, _cfg_file);

    _trn_attr.setCfgFile(_cfg_file);

    // parse TRN attributes config file
    _trn_attr.parseConfig();

    fprintf(stderr, "%s:%d trn_attr[%p]:\n%s\n", __func__, __LINE__, &_trn_attr,  _trn_attr.tostring().c_str());

    try {

        // get session (mission) directory prefix

        fprintf(stderr,"%s:%d - _sessionPrefix[%s]\n",__func__,  __LINE__, _sessionPrefix);

        if(_sessionPrefix == NULL || strcasecmp(_sessionPrefix,"session") == 0){
            if(_sessionPrefix != NULL)
                free(_sessionPrefix);

            _sessionPrefix = (char *)malloc(SESSION_PREFIX_BUF_BYTES);
            memset(_sessionPrefix, 0, SESSION_PREFIX_BUF_BYTES);

            sessionPrefix(&_sessionPrefix, SESSION_PREFIX_BUF_BYTES, SID_LOC, SID_LCMTRN);
        }
        fprintf(stderr,"%s:%d - _sessionPrefix[%s]\n",__func__,  __LINE__, _sessionPrefix);

        chkSetString(&this->_server_ip, _trn_attr.terrainNavServer);
        if(_trn_attr.terrainNavPort > 0){
            _sockport = _trn_attr.terrainNavPort;
        }

        this->mapType = _trn_attr.mapType;
        this->filterType = _trn_attr.filterType;

        fprintf(stderr, "%s: cfg_file      : %s\n", __func__, _cfg_file);
        fprintf(stderr, "%s: sessionPrefix : %s\n", __func__, _sessionPrefix);
        fprintf(stderr, "%s: map           : %s\n", __func__, _trn_attr.mapName);
        fprintf(stderr, "%s: veh           : %s\n", __func__, _trn_attr.vehicleCfgName);
        fprintf(stderr, "%s: par           : %s\n", __func__, _trn_attr.particlesName);
        fprintf(stderr, "%s: svraddr       : %s\n", __func__, _trn_attr.terrainNavServer);
        fprintf(stderr, "%s: svrport       : %ld\n", __func__, _trn_attr.terrainNavPort);

    }catch (Exception e){
        printf("\n%s:%d Exception parsing \n", __func__, __LINE__);
    }

    return 0;
}

//int TrnClient::loadCfgAttributes_orig(const char *cfg_file, const char *usr_log_path)
//{
//    char *cfg_path = NULL;
//
//    if(NULL != cfg_file){
//        // set _cfg_file member and use that
//        TrnAttr::chkSetString(&_cfg_file, cfg_file);
//        cfg_path = _cfg_file;
//        fprintf(stderr, "%s:%d cfg_file [%s]\n", __func__, __LINE__, cfg_file);
//    }else{
//        // use default path ($TRN_CONFIGDIR/terrainAid.cfg or ./terrainAid.cfg)
//        const char *cfg_dir = getenv("TRN_DATAFILES");
//        if(cfg_dir == NULL)
//            cfg_dir = ".";
//
//        size_t cb_len = strlen(cfg_dir) + strlen("/terrainAid.cfg") + 1;
//        char cfg_buf[cb_len];
//        memset(cfg_buf, 0, cb_len);
//
//        snprintf(cfg_buf, cb_len, "%s/terrainAid.cfg", cfg_dir);
//        TrnAttr::chkSetString(&_cfg_file, cfg_buf);
//        cfg_path = _cfg_file;
//    }
//
//    fprintf(stderr, "%s:%d _cfg_file [%s] cfg_path[%s]\n", __func__, __LINE__, _cfg_file, cfg_path);
//
//    _trn_attr.setCfgFile(_cfg_file);
//
//    _trn_attr.parseConfig();
//
//    fprintf(stderr, "%s:%d trn_attr[%p]:\n%s\n", __func__, __LINE__, &_trn_attr,  _trn_attr.tostring().c_str());
//
//    try {
//
//        char mapname[MAPNAME_BUF_BYTES]={0};
//        char vehiclename[VEHICLENAME_BUF_BYTES]={0};
//        char particlename[PARTICLENAME_BUF_BYTES]={0};
//        char sessiondir[SESSIONDIR_BUF_BYTES]={0};
//        char *psession = sessiondir;
//
//        const char* mapPath = getenv("TRN_MAPFILES");
//        const char* cfgPath = getenv("TRN_DATAFILES");
//        const char* logPath = getenv("TRN_LOGFILES");
//
//        char cwd[] = ".";
//        if(mapPath == NULL) {
//            mapPath = cwd;
//        }
//        if(cfgPath == NULL) {
//            cfgPath = cwd;
//        }
//        if(usr_log_path != NULL) {
//            logPath = usr_log_path;
//        } else if (logPath == NULL) {
//            logPath = cwd;
//        }
//
//        // get directory name for session (mission)
//        // NOTE:
//        // The session directory will be created by the trn_server when it
//        // recieves an init message. During init, trn_server creates a new
//        // TerrainNav instance, which invokes TerrainNav::initVariables,
//        // which calls TerrainNav::copyToLogDir.
//        // copyToLogDir computes and creates the next mission name (which should match
//        // getSessionDir)
//        // ParticleFilter is created in initVariables (i.e. at TerrainNav construction),
//        // and will attempt to save particle files to saveDirectory if it is non-NULL.
//        //
//        // For TRNClient, saveDirectory must be set to the log directory
//        // (which may not exist yet), which is passed via init message to trn_server
//        // and used to create the directory.
//        fprintf(stderr,"%s:%d - _sessionPrefix[%s]\n",__func__,  __LINE__, _sessionPrefix);
//
//        if(_sessionPrefix == NULL || strcasecmp(_sessionPrefix,"session") == 0){
//            if(_sessionPrefix != NULL)
//                free(_sessionPrefix);
//            _sessionPrefix = (char *)malloc(SESSION_PREFIX_BUF_BYTES);
//            sessionDir(&_sessionPrefix, SESSION_PREFIX_BUF_BYTES);
//        }
//        fprintf(stderr,"%s:%d - _sessionPrefix[%s]\n",__func__,  __LINE__, _sessionPrefix);
//
//        // initialize session directory info to pass to TRN server,
//        // but don't create the directory or symlink (TRN server does)
//        initSessionDirectory(logPath, _sessionPrefix, &psession, SESSIONDIR_BUF_BYTES, false, false);
//
//        if(_trn_attr.mapName != NULL){
//            snprintf(mapname, MAPNAME_BUF_BYTES, "%s/%s", mapPath, _trn_attr.mapName);
//            chkSetString(&this->mapFile, mapname);
//        }else{
//            chkSetString(&this->mapFile, NULL);
//        }
//
//        if(_trn_attr.vehicleCfgName != NULL){
//            snprintf(vehiclename, VEHICLENAME_BUF_BYTES, "%s/%s", cfgPath, _trn_attr.vehicleCfgName);
//            chkSetString(&this->vehicleSpecFile, vehiclename);
//        }else{
//            chkSetString(&this->vehicleSpecFile, NULL);
//        }
//
//        if(_trn_attr.particlesName!=NULL){
//            snprintf(particlename, PARTICLENAME_BUF_BYTES, "%s/%s", cfgPath, _trn_attr.particlesName);
//            chkSetString(&this->particlesFile, particlename);
//        }else{
//            chkSetString(&this->particlesFile, NULL);
//        }
//
//        chkSetString(&this->_server_ip, _trn_attr.terrainNavServer);
//
//        if(_trn_attr.terrainNavPort > 0){
//            _sockport = _trn_attr.terrainNavPort;
//        }
//
//        // saveDirectory passed to TRN server; TRN session log directory path
//        if(saveDirectory != NULL)
//            free(saveDirectory);
//        this->saveDirectory = STRDUPNULL(sessiondir);
//
//        this->mapType = _trn_attr.mapType;
//        this->filterType = _trn_attr.filterType;
//
//
//#ifdef TRNCLIENT_CREATE_MAP
//        if(this->mapType == 1) {
//            terrainMap = new TerrainMapDEM(this->mapFile);
//        } else {
//            terrainMap = new TerrainMapOctree(this->mapFile);
//        }
//#endif
//        // Initialize TNavConfig
//        TNavConfig::instance()->setMapFile(this->mapFile);
//        TNavConfig::instance()->setVehicleSpecsFile(this->vehicleSpecFile);
//        TNavConfig::instance()->setParticlesFile(this->particlesFile);
//        TNavConfig::instance()->setLogDir(_sessionPrefix);
//        TNavConfig::instance()->setConfigPath(cfg_path);
//
//        fprintf(stderr, "%s: map    : %s\n", __func__, mapname);
//        fprintf(stderr, "%s: logPath: %s\n", __func__, logPath);
//        fprintf(stderr, "%s: sessionPrefix : %s\n", __func__, _sessionPrefix);
//        fprintf(stderr, "%s: veh    : %s\n", __func__, vehiclename);
//        fprintf(stderr, "%s: par    : %s\n", __func__, particlename);
//        fprintf(stderr, "%s: cfg    : %s\n", __func__, cfg_path);
//        fprintf(stderr, "%s: save   : %s\n", __func__, saveDirectory);
//        fprintf(stderr, "%s: trnsvr : %s\n", __func__, _trn_attr.terrainNavServer);
//
//    }catch (Exception e){
//        printf("\n%s:%d Exception parsing \n", __func__, __LINE__);
//    }
//
//    fprintf(stderr, "%s: server : %s:%d\n", __func__, _server_ip, _sockport);
//    fprintf(stderr, "%s: vehcfg : %s\n", __func__, _trn_attr.vehicleCfgName);
//
//    return 0;
//}

#ifdef WITH_VNORM_FN
// Take the standard 2-norm. This one returns the answer, since it is a scalar.
static double Vnorm( double v[] )
{
   double Vnorm2 = 0.;
   int i;
   for(i=0; i<VNORM_DIM; i++) Vnorm2 += pow(v[i],2.);
   return( sqrt( Vnorm2 ) );
}
#endif

#ifdef WITH_DEGTORAD_FN
static double degToRad(double deg)
{
  double const RadsPerDeg = M_PI  / 180.0;
  return deg*RadsPerDeg;
}
#endif

int TrnClient::setVerbose(int val)
{
    verbose=val;
    return 0;
}

void TrnClient::initServer()
{
    // filter type and map type encoded in single integer
    // param = filter*100 + map
    char cmt = char(mapType);
    cmt *= 10;
    char cft = char(filterType);
    char param = cmt + cft;

    // Use basenames (no pathnames) of files and folders when connecting
    // to trn-server. Let the server find the files in its environment.
    fprintf(stderr,"%s:%d - server initializating...\n", __func__, __LINE__);

    commsT init(TRN_INIT, param,
                TRNUtils::basename(_trn_attr.mapName),
                TRNUtils::basename(_trn_attr.vehicleCfgName),
                TRNUtils::basename(_trn_attr.particlesName),
                TRNUtils::basename(_sessionPrefix)
                );

    _initialized = false;

    if (0 != send_msg(init)) {
        // Check for ack response
        if (TRN_ACK != get_msg()) {
            fprintf(stderr,"%s:%d - server initialization failed!\n", __func__, __LINE__);
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

int TrnClient::initSocket()
{
    int retval = -1;

    if(_sockfd > 0){
        return 0;
    }
    // Setup socket to receive the deltaT packets
    if ( (_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) > 0 ){
        int wstat = 16;

        fprintf(stderr,"%*s %s fd[%d]\n",wstat,"create socket","OK", _sockfd);


        // disable linger, 0 sec
        struct linger lv = { 0, 0};
        if (setsockopt(_sockfd, SOL_SOCKET, SO_LINGER,
                       (const void *)&lv, (socklen_t)sizeof(struct linger)) == 0)
        {
            fprintf(stderr,"%*s %s fd[%d]\n",wstat,"setsockopt SO_LINGER","OK", _sockfd);
        }else {
            fprintf(stderr,"%*s %s fd[%d] [%d/%s]\n",wstat,"setsockopt SO_LINGER","ERR", _sockfd, errno, strerror(errno));
            perror("setsockopt SO_LINGER");
        }

        struct timeval tv={0,0};
        tv.tv_sec = 150;
        tv.tv_usec = 0;
        if(setsockopt(_sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval)) == 0){
            fprintf(stderr,"%*s %s fd[%d]\n",wstat,"setsockopt SO_RCVTIMEO","OK", _sockfd);
        }else {
            fprintf(stderr,"%*s %s fd[%d] [%d/%s]\n",wstat,"setsockopt SO_RCVTIMEO","ERR", _sockfd, errno, strerror(errno));
            perror("setsockopt SO_REUSEADDR");
        }

        retval = 0;
    } else {
        fprintf(stderr, "%s: socket create failed [%d] [%d/%s]\n", __func__, _sockfd, errno,strerror(errno));
    }


    return retval;
}

int TrnClient::connectSocket()
{
    int retval = -1;

        ::close(_sockfd);
        _connected=false;
        _sockfd = -1;
        initSocket();

        memset((void*)&_server_addr, 0, sizeof(struct sockaddr_in));

        // Set up client info
        // Beam former sends data to us
        if(NULL != _server_ip) {
            _server_addr.sin_family      = AF_INET;
            _server_addr.sin_addr.s_addr = inet_addr(_server_ip);
            _server_addr.sin_port        = htons(_sockport);

            if(connect(_sockfd, (struct sockaddr *)&_server_addr, sizeof(struct sockaddr_in)) == 0)
            {
                fprintf(stderr, "%s : connect OK [%s:%d]\n", __func__, _server_ip, _sockport);
                _connected = true;
                retval = 0;
            } else {
                fprintf(stderr, "%s : connect ERR fd[%d] [%s:%d] [%d/%s]\n", __func__, _sockfd, _server_ip, _sockport, errno,strerror(errno));
                perror("TrnClient::connectSocket");
            }
        } else {
            fprintf(stderr,"%s: ERR - NULL _server_ip\n",__func__);
        }
    return retval;
}

// Common to QNX and NIX versions
// 
TerrainNav* TrnClient::connectTRN()
{
    TerrainNav *_tercom = NULL;
    TrnAttr *patt = &_trn_attr;

    try
    {
        printf("Connecting to %s:%ld...\n", _trn_attr.terrainNavServer, _trn_attr.terrainNavPort);

        if(connectSocket() == 0){
            _tercom = static_cast<TerrainNav *>(this);
        }
    }
    catch (Exception e)
    {
        fprintf(stderr, "TrnClient - Failed TRN connection. Check TRN error messages...\n");
    }

    if (NULL!=_tercom && _tercom->is_connected() && !this->_trn_attr.skipInit){

        TrnClient::initServer();

        // note: application should copy TRN configuration file(s) to log directory
        // (e.g. terrainAid-<key>-<session>.cfg)

        // configure server (TerrainNavClient methods send messages to server)
        // these are the options exposed by TerrainNavClient that are covered in TrnAttr (terrainNav.cfg)
        if(_trn_attr.forceLowGradeFilter) {
            TerrainNavClient::useLowGradeFilter();
        } else {
            TerrainNavClient::useHighGradeFilter();
        }
        TerrainNavClient::setModifiedWeighting(_trn_attr.useModifiedWeighting);
        TerrainNavClient::setFilterReinit(_trn_attr.allowFilterReinits);

        // If we reach here then we've connected
        fprintf(stderr, "%s:%d TrnClient - connected to server if no error messages...\n", __func__, __LINE__);

    } else{
        fprintf(stderr, "%s:%d TrnClient - Not initialized. skipInit(%c) See trn_server error messages...\n", __func__, __LINE__, this->_trn_attr.skipInit ? 'Y' : 'N');
    }

    return _tercom;
}

//TerrainNav* TrnClient::connectTRN_orig()
//{
//    TerrainNav *_tercom = NULL;
//    TrnAttr *patt = &_trn_attr;
//
//    try
//    {
//        if(connectSocket() == 0){
//            _tercom = static_cast<TerrainNav *>(this);
//        }
//
////        if(_connected){
////            ::close(_sockfd);
////            _connected=false;
////        }
////
////        init_comms();
//
//        // On linux and cygwin, we can use a native TerrainNav object instead
//        // of relying on a trn_server. Call useTRNServer() to decide.
//        //
////        printf("Connecting to %s...\n", _trn_attr.terrainNavServer);
////        _tercom = static_cast<TerrainNav *>(this);
//    }
//    catch (Exception e)
//    {
//        fprintf(stderr, "TrnClient - Failed TRN connection. Check TRN error messages...\n");
//    }
//
//    if (NULL!=_tercom && _tercom->is_connected() && !this->_trn_attr.skipInit){
//
//        init_server();
//
//        // TODO: does client need to configure it's own attributes?
////        setModifiedWeighting(this->_trn_attr.useModifiedWeighting);
////        // filterReinits
////        setFilterReinit(_trn_attr.allowFilterReinits);
////        if(this->_trn_attr.forceLowGradeFilter)
////            useLowGradeFilter();
////        else
////            useHighGradeFilter();
//
//        // server initialization creates log directory
//        // copy config file to directory (if TRN on same host)
//        // TODO: if server drops connection, it creates a new session
//        // directory, but this copies config file to it's
//        // current session (overwriting it).
//        // Maybe this isn't the place/time to do this...
//        char *cfg_path = TNavConfig::instance()->getConfigPath();
//
//
//        if (NULL != this->saveDirectory && NULL != cfg_path)
//        {
//            size_t cb_size = strlen(cfg_path) + strlen(this->saveDirectory) + strlen("cp  /.") + 1;
//            char copybuf[cb_size];
//            memset(copybuf, 0, cb_size);
//
//            snprintf(copybuf, 512, "cp %s %s/.", cfg_path,
//                    this->saveDirectory);
//            if (0 != system(copybuf)) {
//                fprintf(stderr, "%s:%d: ERR - config copy [%s] failed [%d/%s]\n",
//                        __func__, __LINE__, copybuf, errno, strerror(errno));
//            } else {
//                fprintf(stderr, "%s:%d: copied config [%s] to [%s]\n",
//                        __func__, __LINE__, cfg_path, saveDirectory);
//            }
//        }
//        free(cfg_path);
//
//        // If we reach here then we've connected
//        //
//        fprintf(stderr, "%s:%d TrnClient - connected to server if no error messages...\n", __func__, __LINE__);
//
//    } else{
//        fprintf(stderr, "%s:%d TrnClient - Not initialized. skipInit(%c) See trn_server error messages...\n", __func__, __LINE__, this->_trn_attr.skipInit ? 'Y' : 'N');
//    }
//
//    return _tercom;
//}

void TrnClient::show(int indent, int wkey, int wval)
{
    fprintf(stderr,"%*s%*s %*s\n",indent,(indent>0?" ":""),wkey,"sessionPrefix",wval,_sessionPrefix);
    fprintf(stderr,"%*s%*s %*c\n",indent,(indent>0?" ":""),wkey,"connected",wval,(_connected?'Y':'N'));
    fprintf(stderr,"%*s%*s %*d\n",indent,(indent>0?" ":""),wkey,"trn_server_type",wval,_mbtrn_server_type);
    fprintf(stderr,"%*s%*s %*s\n",indent,(indent>0?" ":""),wkey,"server_ip",wval,_server_ip);
    fprintf(stderr,"%*s%*s %*d\n",indent,(indent>0?" ":""),wkey,"sockport",wval,_sockport);
    fprintf(stderr,"%*s%*s %*d\n",indent,(indent>0?" ":""),wkey,"sockfd",wval,_sockfd);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"server_addr",wval,&_server_addr);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"server_msg",wval,&_server_msg);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"comms_buf",wval,&_comms_buf);

    fprintf(stderr,"%*s%*s %*c\n",indent,(indent>0?" ":""),wkey,"verbose",wval,(verbose?'Y':'N'));
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"quit_ref",wval,_quit_ref);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"cfg_file",wval,_cfg_file);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"trn_attr",wval,(void *)&_trn_attr);
}

void TrnClient::show_addr(int indent, int wkey, int wval)
{
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"this",wval,(void *)this);
    fprintf(stderr,"%*s ----- TerrainNav -----\n",indent,(indent>0?" ":""));
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"saveDirectory",wval,(void *)saveDirectory);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"vehicleSpecFile",wval,(void *)vehicleSpecFile);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"particlesFile",wval,(void *)particlesFile);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"mapFile",wval,(void *)mapFile);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"filterType",wval,(void *)&filterType);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"mapType",wval,(void *)&mapType);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"terrainMap",wval,(void *)terrainMap);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"_initialized",wval,(void *)&_initialized);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"_trnLog",wval,(void *)_trnLog);
#ifdef WITH_TRNLOG
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"_trnBinLog",wval,(void *)_trnBinLog);
#endif

    fprintf(stderr,"%*s ----- TerrainNavClient -----\n",indent,(indent>0?" ":""));


    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"sessionPrefix",wval,(void *)_sessionPrefix);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"connected",wval,(void *)&_connected);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"trn_server_type",wval,(void *)&_mbtrn_server_type);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"server_ip",wval,(void *)_server_ip);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"sockfd",wval,(void *)&_sockfd);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"sockport",wval,(void *)&_sockport);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"server_addr",wval,(void *)&_server_addr);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"server_msg",wval,(void *)&_server_msg);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"comms_buf",wval,(void *)_comms_buf);

    fprintf(stderr,"%*s ----- TrnClient -----\n",indent,(indent>0?" ":""));

    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"verbose",wval,(void *)&verbose);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"quit_ref",wval,(void *)&_quit_ref);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"cfg_file",wval,(void *)_cfg_file);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"trn_attr",wval, (void *)&_trn_attr);
}

void TrnClient::setQuitRef(bool *pvar)
{
    _quit_ref=pvar;
}

bool TrnClient::isQuitSet()
{
    if(NULL != _quit_ref)
        return *_quit_ref;
    return false;
}

TrnAttr &TrnClient::getTrnAttr()
{
    return _trn_attr;
}

char *TrnClient::attGetServer()
{
    return _trn_attr.terrainNavServer;
}

void TrnClient::setSessionID(const std::string &session_str)
{
    _sessionID = session_str;
}

// return static buffer (may change, not thread-safe)
// optionally provide buffer r_dest
// return's pointer to r_dest if written to, or static buffer otherwise
char *TrnClient::sessionPrefix(char **r_dest, size_t len, SIDTime sid_time, SIDFormat sid_fmt)
{
    static char session_buf[SESSION_PREFIX_BUF_BYTES];
    memset(session_buf, 0, SESSION_PREFIX_BUF_BYTES);
    char *retval = session_buf;

    time_t tt_now=time(NULL);
    struct tm *tm_now = NULL;

    if(sid_time == SID_LOC || sid_fmt == SID_LCMTRN) {
        tm_now = localtime(&tt_now);
    } else {
        tm_now = gmtime(&tt_now);
    }
    if(sid_fmt == SID_ISO8601) {
        strftime(session_buf, SESSION_PREFIX_BUF_BYTES, "%Y%m%dT%H%M%S", tm_now);
    } else if(sid_fmt == SID_LCMTRN) {
        strftime(session_buf, SESSION_PREFIX_BUF_BYTES, "%Y%m%d-%H%M%S", tm_now);
    } else if(sid_fmt == SID_YYYYJJJHHMM) {
        strftime(session_buf, SESSION_PREFIX_BUF_BYTES, "%Y-%j-%H%M%S", tm_now);
    } else {
        strftime(session_buf, SESSION_PREFIX_BUF_BYTES, "%Y-%j", tm_now);
    }

    if(NULL != r_dest) {
        if(NULL != *r_dest && len > strlen(session_buf)) {
            // return in r_dest if non-NULL and adequate size
            snprintf(*r_dest, len, "%s", session_buf);
            retval = *r_dest;
        }
    }

    return retval;
}
