/****************************************************************************/
/* Copyright (c) 2017 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  : TRn test client  */
/* Filename : TrnClient.cpp                                                 */
/* Author   : headley                                                       */
/* Project  :                                                    */
/* Version  : 1.0                                                           */
/* Created  : 24oct2019                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/* Began with a copy of test_client as there is a lot of stuff to reuse     */
/****************************************************************************/

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

#define MAPNAME_BUF_BYTES 512
#define CFGNAME_BUF_BYTES 512
#define VEHICLENAME_BUF_BYTES 512
#define PARTICLENAME_BUF_BYTES 512
#define SESSIONDIR_BUF_BYTES 512

// Common to QNX and NIX versions
// 
TrnClient::TrnClient(const char *host, int port)
: TerrainNavClient(),
    _cfg_file(NULL),
   verbose(0)
{
    try{
        _trn_attr = new TrnAttr();
        char *ld =getenv("TRN_LOGDIR");
        _logdir    = (NULL!=ld ? STRDUPNULL(ld): STRDUPNULL(LOGDIR_DFL));

    }catch (Exception e){
        printf("\n];\n");
    }

    if (NULL!=host){
        free(_server_ip);
        _server_ip = strdup(host);
        _sockport = port;
        free(_trn_attr->terrainNavServer);
        _trn_attr->terrainNavServer = strdup(host);
        _trn_attr->terrainNavPort = port;
    }

    fprintf(stderr, "\nServer      : %s :%ld\n",_trn_attr->terrainNavServer, _trn_attr->terrainNavPort);
    fprintf(stderr,"Vehicle Cfg : %s\n\n", _trn_attr->vehicleCfgName);
    _initialized = false;
    
}

TrnClient::TrnClient(const char *svr_log_dir, const char *host, int port)
: TerrainNavClient(),
_cfg_file(NULL),
verbose(0)
{
    try {
        _trn_attr = new TrnAttr();

        // set TRN server log dir to
        // $TRN_LOGDIR (default if set)
        // svr_log_dir (specified by application)
        // CWD if neither set
        const char *ld =getenv("TRN_LOGDIR");
        if(NULL!=svr_log_dir){
            ld=svr_log_dir;
        }
        _logdir    = (NULL!=ld ? STRDUPNULL(ld):STRDUPNULL(LOGDIR_DFL));

    }catch (Exception e){
        printf("\n];\n");
    }

    if (NULL!=host){
        free(_server_ip);
        _server_ip = strdup(host);
        _sockport = port;
        free(_trn_attr->terrainNavServer);
        _trn_attr->terrainNavServer = strdup(host);
        _trn_attr->terrainNavPort = port;
    }

    fprintf(stderr, "\nServer      : %s :%ld\n",_trn_attr->terrainNavServer, _trn_attr->terrainNavPort);
    fprintf(stderr,"Vehicle Cfg : %s\n\n", _trn_attr->vehicleCfgName);

    _initialized = false;

}

TrnClient::~TrnClient()
{
    free(_cfg_file);
    delete _trn_attr;
    // base class closes _sockfd and free's _server_ip and _logdir
}

int TrnClient::loadCfgAttributes(const char *cfg_file)
{
    char cfg_buf[512];
    char *cfg_path=NULL;

    if(NULL!=cfg_file){
        // set _cfg_file member and use that
        TrnAttr::chkSetString(&_cfg_file, cfg_file);
        cfg_path = _cfg_file;
    }else{
        // use default path ($TRN_CONFIGDIR/terrainAid.cfg or ./terrainAid.cfg)
        const char *cfg_dir = getenv("TRN_DATAFILES");
        snprintf(cfg_buf, 512, "%s/terrainAid.cfg", (NULL!=cfg_dir ? cfg_dir : "."));
        TrnAttr::chkSetString(&_cfg_file, cfg_buf);
        cfg_path = _cfg_file;
    }

    _trn_attr->setCfgFile(_cfg_file);

    _trn_attr->parseConfig();

    try {

        char mapname[MAPNAME_BUF_BYTES]={0};
        char vehiclename[VEHICLENAME_BUF_BYTES]={0};
        char particlename[PARTICLENAME_BUF_BYTES]={0};
        char sessiondir[SESSIONDIR_BUF_BYTES]={0};

        char* mapPath = getenv("TRN_MAPFILES");
        char* cfgPath = getenv("TRN_DATAFILES");
        char* logPath = getenv("TRN_LOGFILES");

        char cwd[] = ".";
        if(mapPath == NULL) {
            mapPath = cwd;
        }
        if(cfgPath == NULL) {
            cfgPath = cwd;
        }
        if(logPath == NULL) {
            logPath = cwd;
        }

        // get directory name for session (mission)
        // NOTE:
        // The session directory will be created by the trn_server when it
        // recieves an init message. During init, trn_server creates a new
        // TerrainNav instance, which invokes TerrainNav::initVariables,
        // which calls TerrainNav::copyToLogDir.
        // copyToLogDir computes and creates the next mission name (which should match
        // getSessionDir)
        // ParticleFilter is created in initVariables (i.e. at TerrainNav construction),
        // and will attempt to save particle files to saveDirectory if it is non-NULL.
        //
        // For TRNClient, saveDirectory must be set to the log directory
        // (which may not exist yet), which is passed via init message to trn_server
        // and used to create the directory.
        if(_logdir==NULL || strcasecmp(_logdir,"session")==0){
            free(_logdir);
            _logdir = (char *)malloc(32);
            time_t tt_now=time(NULL);
            struct tm *tm_now = gmtime(&tt_now);
            strftime(_logdir,32,"%Y-%j",tm_now);
        }
        getSessionDir(_logdir, sessiondir,512,false);


        if(_trn_attr->mapName != NULL){
            snprintf(mapname, MAPNAME_BUF_BYTES, "%s/%s", mapPath, _trn_attr->mapName);
            free(mapFile);
            this->mapFile = STRDUPNULL(mapname);
        }else{
            this->mapFile = NULL;
        }

        if(_trn_attr->vehicleCfgName != NULL){
            snprintf(vehiclename, VEHICLENAME_BUF_BYTES, "%s/%s", cfgPath, _trn_attr->vehicleCfgName);
            free(vehicleSpecFile);
            this->vehicleSpecFile = STRDUPNULL(vehiclename);
        }else{
            this->vehicleSpecFile = NULL;
        }

        if(_trn_attr->particlesName!=NULL){
            snprintf(particlename, PARTICLENAME_BUF_BYTES, "%s/%s", cfgPath, _trn_attr->particlesName);
            free(particlesFile);
            this->particlesFile = STRDUPNULL(particlename);
        }else{
            this->particlesFile = NULL;
        }

        free(saveDirectory);
        this->saveDirectory = STRDUPNULL(sessiondir);

        this->mapType = _trn_attr->mapType;
        this->filterType = _trn_attr->filterType;


#ifdef TRNCLIENT_CREATE_MAP
        if(this->mapType == 1) {
            terrainMap = new TerrainMapDEM(this->mapFile);
        } else {
            terrainMap = new TerrainMapOctree(this->mapFile);
        }
#endif
        // Initialize TNavConfig
        TNavConfig::instance()->setMapFile(this->mapFile);
        TNavConfig::instance()->setVehicleSpecsFile(this->vehicleSpecFile);
        TNavConfig::instance()->setParticlesFile(this->particlesFile);
        TNavConfig::instance()->setLogDir(_logdir);
        TNavConfig::instance()->setConfigPath(cfg_path);

        fprintf(stderr, "%s: map    : %s\n", __func__, mapname);
        fprintf(stderr, "%s: logPath: %s\n", __func__, logPath);
        fprintf(stderr, "%s: logdir : %s\n", __func__, _logdir);
        fprintf(stderr, "%s: veh    : %s\n", __func__, vehiclename);
        fprintf(stderr, "%s: par    : %s\n", __func__, particlename);
        fprintf(stderr, "%s: cfg    : %s\n", __func__, cfg_path);
        fprintf(stderr, "%s: save   : %s\n", __func__, saveDirectory);
        fprintf(stderr, "%s: trnsvr : %s\n", __func__, _trn_attr->terrainNavServer);

    }catch (Exception e){
        printf("\n];\n");
    }

    fprintf(stderr, "%s: server : %s:%d\n", __func__, _server_ip, _sockport);
    fprintf(stderr, "%s: vehcfg : %s\n", __func__, _trn_attr->vehicleCfgName);

    return 0;
}

/*
** Take the standard 2-norm. This one returns the answer, since it is a scalar.
*/

#ifdef WITH_VNORM_FN
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
                fprintf(stderr, "%s : connect OK [%s:%d]\n", __FUNCTION__, _server_ip, _sockport);
                _connected = true;
                retval = 0;
            } else {
                fprintf(stderr, "%s : connect ERR fd[%d] [%s:%d] [%d/%s]\n", __FUNCTION__, _sockfd, _server_ip, _sockport, errno,strerror(errno));
                perror("TrnClient::connectSocket");
            }
        } else {
            fprintf(stderr,"%s: ERR - NULL _server_ip\n",__FUNCTION__);
        }
    return retval;
}

// Common to QNX and NIX versions
// 
TerrainNav* TrnClient::connectTRN()
{
    TerrainNav *_tercom = NULL;
    
    fprintf(stderr, "TrnClient - Using TerrainNav [%s:%ld]\n",
            _trn_attr->terrainNavServer, _trn_attr->terrainNavPort);
    
    try
    {
        if(connectSocket()==0){
            _tercom = static_cast<TerrainNav *>(this);
        }

//        if(_connected){
//            ::close(_sockfd);
//            _connected=false;
//        }
//
//        init_comms();

        // On linux and cygwin, we can use a native TerrainNav object instead
        // of relying on a trn_server. Call useTRNServer() to decide.
        //
//        printf("Connecting to %s...\n", _trn_attr->terrainNavServer);
//        _tercom = static_cast<TerrainNav *>(this);
    }
    catch (Exception e)
    {
        fprintf(stderr, "TrnClient - Failed TRN connection. Check TRN error messages...\n");
    }
    
    if (NULL!=_tercom && _tercom->is_connected() && !_trn_attr->skipInit){
        init_server();

        setModifiedWeighting(_trn_attr->useModifiedWeighting);
        // filterReinits
        setFilterReinit(_trn_attr->allowFilterReinits);
        if(_trn_attr->forceLowGradeFilter)
            useLowGradeFilter();
        else
            useHighGradeFilter();

        // server initialization creates log directory
        // copy config file to directory (if TRN on same host)
        char copybuf[512] = {0};
        char *cfg_path = TNavConfig::instance()->getConfigPath();

        if (this->saveDirectory && NULL != cfg_path)
        {
            snprintf(copybuf, 512, "cp %s %s/.", cfg_path,
                    this->saveDirectory);
            if (0 != system(copybuf))
                fprintf(stderr, "%s: ERR - config copy [%s] failed [%d/%s]\n",
                        __func__, copybuf, errno, strerror(errno));
            else
                fprintf(stderr, "%s: copied config [%s] to [%s]\n",
                        __func__, cfg_path, saveDirectory);
        }
        free(cfg_path);

        // If we reach here then we've connected
        //
        fprintf(stderr, "TrnClient - connected to server if no error messages...\n");

    } else{
        fprintf(stderr, "TrnClient - Not initialized. skipInit(%c) See trn_server error messages...\n", _trn_attr->skipInit ? 'Y' : 'N');
    }
    
    return _tercom;
}

void TrnClient::show(int indent, int wkey, int wval)
{
    fprintf(stderr,"%*s%*s %*s\n",indent,(indent>0?" ":""),wkey,"host",wval,_server_ip);
    fprintf(stderr,"%*s%*s %*d\n",indent,(indent>0?" ":""),wkey,"port",wval,_sockport);
    fprintf(stderr,"%*s%*s %*d\n",indent,(indent>0?" ":""),wkey,"fd",wval,_sockfd);
    fprintf(stderr,"%*s%*s %*c\n",indent,(indent>0?" ":""),wkey,"connected",wval,_connected?'Y':'N');
    fprintf(stderr,"%*s%*s %*s\n",indent,(indent>0?" ":""),wkey,"trncfg",wval,_cfg_file);
    fprintf(stderr,"%*s%*s %*s\n",indent,(indent>0?" ":""),wkey,"logdir",wval,_logdir);
    fprintf(stderr,"%*s%*s %*c\n",indent,(indent>0?" ":""),wkey,"trn svr type",wval,_mbtrn_server_type?'1':'0');

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
