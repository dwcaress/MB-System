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

#define MAPNAME_BUF_BYTES 1024 //512
#define CFGNAME_BUF_BYTES 1024 //512
#define VEHICLENAME_BUF_BYTES 1024 //512
#define PARTICLENAME_BUF_BYTES 1024 //512
#define SESSIONDIR_BUF_BYTES 1024 //512
#define LOGDIR_BUF_BYTES 64

// Common to QNX and NIX versions
// 
TrnClient::TrnClient(const char *host, int port)
: TerrainNavClient()
, verbose(0)
, _quit_ref(NULL)
, _cfg_file(NULL)
//, _trn_attr(NULL)
{

    fprintf(stderr,"%s:%d - TrnClient CTOR >>>>>>>>>>>>>>>>>>\n", __func__, __LINE__);
    try{
//        _trn_attr = new TrnAttr();

        // set TRN server log dir to
        // $TRN_LOGDIR (default if set)
        // svr_log_dir (specified by application)
        // CWD if neither set
        char *ld =getenv("TRN_LOGDIR");
        _logdir    = (NULL!=ld ? STRDUPNULL(ld): STRDUPNULL(LOGDIR_DFL));

    }catch (Exception e){
        fprintf(stderr, "\n%s Exception - could not initialize TrnAttr\n", __func__);
    }

    if (NULL!=host){
        if(_server_ip != NULL)
            free(_server_ip);
        _server_ip = strdup(host);
        _sockport = port;
        _trn_attr.terrainNavServer = strdup(host);
        _trn_attr.terrainNavPort = port;
    }

    fprintf(stderr, "\n%s:%d - this %p trn_attr[%p]\n",__func__, __LINE__, this, &_trn_attr);
    fprintf(stderr, "%s - Server      : %s :%ld\n", __func__, _trn_attr.terrainNavServer, _trn_attr.terrainNavPort);
    fprintf(stderr,"%s - Vehicle Cfg : %s\n\n", __func__, _trn_attr.vehicleCfgName);
    _initialized = false;
    
}

TrnClient::TrnClient(const char *svr_log_dir, const char *host, int port)
: TerrainNavClient()
, verbose(0)
, _quit_ref(NULL)
, _cfg_file(NULL)
//, _trn_attr(NULL)
{
    fprintf(stderr,"%s:%d - TrnClient CTOR >>>>>>>>>>>>>>>>>>\n", __func__, __LINE__);
    try {
//        _trn_attr = new TrnAttr();

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
        fprintf(stderr, "\n%s Exception - could not initialize TrnAttr\n", __func__);
    }

    if (NULL!=host){
        if(_server_ip != NULL)
            free(_server_ip);
        _server_ip = strdup(host);
        _sockport = port;
        _trn_attr.terrainNavServer = strdup(host);
        _trn_attr.terrainNavPort = port;
    }

    fprintf(stderr, "\n%s:%d - this %p trn_attr[%p]\n",__func__, __LINE__, this, &_trn_attr);
    fprintf(stderr, "%s - Server      : %s :%ld\n", __func__, _trn_attr.terrainNavServer, _trn_attr.terrainNavPort);
    fprintf(stderr,"%s - Vehicle Cfg : %s\n\n", __func__, _trn_attr.vehicleCfgName);

    _initialized = false;

}

TrnClient::TrnClient(const TrnClient& other)
{
    fprintf(stderr,"%s:%d - TrnClient COPY CTOR >>>>>>>>>>>>>>>>>>\n", __func__, __LINE__);
    this->verbose = other.verbose;
    this->_quit_ref = other._quit_ref;
    this->_cfg_file = STRDUPNULL(other._cfg_file);
    //TODO KLH : this could be a/the problem (not OK to re-assign object like this)
//    this->_trn_attr = TrnAttr(other._trn_attr);
    this->_trn_attr = other._trn_attr;

    this->_connected = other._connected;
    this->_mbtrn_server_type = other._mbtrn_server_type;
    this->_server_ip = STRDUPNULL(other._server_ip);
    this->_sockfd = other._sockfd;
    this->_logdir = STRDUPNULL(other._logdir);
    this->_sockfd = other._sockfd;
    this->_sockport = other._sockport;
    std::memcpy(&_server_addr, &other._server_addr, sizeof(struct sockaddr_in));
    this->_server_msg = other._server_msg;
    std::memcpy(this->_comms_buf, other._comms_buf, TRN_MSG_SIZE);
}

TrnClient::~TrnClient()
{
    fprintf(stderr,"%s:%d - TrnClient DTOR <<<<<<<<<<<<<<<<<<<<<<\n", __func__, __LINE__);
    free(_cfg_file);
//    if(_trn_attr != nullptr)
//        delete _trn_attr;
//    _trn_attr = nullptr;
    // base class closes _sockfd and free's _server_ip and _logdir
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

int TrnClient::loadCfgAttributes(const char *cfg_file)
{
    char cfg_buf[512];
    char *cfg_path=NULL;

    if(NULL != cfg_file){
        // set _cfg_file member and use that
        TrnAttr::chkSetString(&_cfg_file, cfg_file);
        cfg_path = _cfg_file;
        fprintf(stderr, "%s:%d cfg_file [%s]\n", __func__, __LINE__, cfg_file);
    }else{
        // use default path ($TRN_CONFIGDIR/terrainAid.cfg or ./terrainAid.cfg)
        const char *cfg_dir = getenv("TRN_DATAFILES");
        snprintf(cfg_buf, 512, "%s/terrainAid.cfg", (NULL!=cfg_dir ? cfg_dir : "."));
        TrnAttr::chkSetString(&_cfg_file, cfg_buf);
        cfg_path = _cfg_file;
    }

    fprintf(stderr, "%s:%d _cfg_file [%s]\n", __func__, __LINE__, _cfg_file);
    
    _trn_attr.setCfgFile(_cfg_file);

    _trn_attr.parseConfig();

    fprintf(stderr, "%s:%d trn_attr[%p]:\n%s\n", __func__, __LINE__, &_trn_attr,  _trn_attr.tostring().c_str());

    try {

        char mapname[MAPNAME_BUF_BYTES]={0};
        char vehiclename[VEHICLENAME_BUF_BYTES]={0};
        char particlename[PARTICLENAME_BUF_BYTES]={0};
        char sessiondir[SESSIONDIR_BUF_BYTES]={0};

        const char* mapPath = getenv("TRN_MAPFILES");
        const char* cfgPath = getenv("TRN_DATAFILES");
        const char* logPath = getenv("TRN_LOGFILES");

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
            if(_logdir != NULL)
                free(_logdir);
            _logdir = (char *)malloc(LOGDIR_BUF_BYTES);
            time_t tt_now=time(NULL);
            struct tm *tm_now = gmtime(&tt_now);
            strftime(_logdir,LOGDIR_BUF_BYTES,"%Y-%j",tm_now);
        }
        getSessionDir(_logdir, sessiondir, SESSIONDIR_BUF_BYTES, false);


        if(_trn_attr.mapName != NULL){
            snprintf(mapname, MAPNAME_BUF_BYTES, "%s/%s", mapPath, _trn_attr.mapName);
            chkSetString(&this->mapFile, mapname);
        }else{
            chkSetString(&this->mapFile, NULL);
        }

        if(_trn_attr.vehicleCfgName != NULL){
            snprintf(vehiclename, VEHICLENAME_BUF_BYTES, "%s/%s", cfgPath, _trn_attr.vehicleCfgName);
            chkSetString(&this->vehicleSpecFile, vehiclename);
        }else{
            chkSetString(&this->vehicleSpecFile, NULL);
        }

        if(_trn_attr.particlesName!=NULL){
            snprintf(particlename, PARTICLENAME_BUF_BYTES, "%s/%s", cfgPath, _trn_attr.particlesName);
            chkSetString(&this->particlesFile, particlename);
        }else{
            chkSetString(&this->particlesFile, NULL);
        }
        
        chkSetString(&this->_server_ip, _trn_attr.terrainNavServer);

        if(_trn_attr.terrainNavPort > 0){
            _sockport = _trn_attr.terrainNavPort;
        }

        if(saveDirectory != NULL)
            free(saveDirectory);
        this->saveDirectory = STRDUPNULL(sessiondir);

        this->mapType = _trn_attr.mapType;
        this->filterType = _trn_attr.filterType;


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
        fprintf(stderr, "%s: trnsvr : %s\n", __func__, _trn_attr.terrainNavServer);

    }catch (Exception e){
        printf("\n%s:%dException parsing \n", __func__, __LINE__);
    }

    fprintf(stderr, "%s: server : %s:%d\n", __func__, _server_ip, _sockport);
    fprintf(stderr, "%s: vehcfg : %s\n", __func__, _trn_attr.vehicleCfgName);

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
    
    fprintf(stderr, "%s:%d - TrnClient - this[%p] trn_attr [%p]\n", __func__, __LINE__, this, (void *)patt);
    fprintf(stderr, "%s:%d - this addr:\n", __func__, __LINE__);

    fprintf(stderr, "%s:%d -------- sizeof(TrnClient) [%zu] sizeof(*this) [%zu] sizeof(TrnAttr) [%zu] sizeof(_trn_attr) [%zu] --------\n", __func__, __LINE__, sizeof(TrnClient), sizeof(*this), sizeof(TrnAttr), sizeof(_trn_attr));

    show_addr();

    try
    {

        fprintf(stderr, "%s:%d - this _trn_attr/att addr[%p/%p]\n", __func__, __LINE__, &_trn_attr, patt);

        if(patt != NULL) {
            fprintf(stderr, "%s:%d - this trn_attr:\n%s\n", __func__, __LINE__, patt->atostring().c_str());

            fprintf(stderr, "TrnClient - TerrainNavServer [%s:%ld]\n", patt->terrainNavServer, patt->terrainNavPort);
        }

        if(connectSocket() == 0){
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
//        printf("Connecting to %s...\n", _trn_attr.terrainNavServer);
//        _tercom = static_cast<TerrainNav *>(this);
    }
    catch (Exception e)
    {
        fprintf(stderr, "TrnClient - Failed TRN connection. Check TRN error messages...\n");
    }
    
    if (NULL!=_tercom && _tercom->is_connected() && !this->_trn_attr.skipInit){
        init_server();

        setModifiedWeighting(this->_trn_attr.useModifiedWeighting);
        // filterReinits
        setFilterReinit(_trn_attr.allowFilterReinits);
        if(this->_trn_attr.forceLowGradeFilter)
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
        fprintf(stderr, "%s:%d TrnClient - connected to server if no error messages...\n", __func__, __LINE__);

    } else{
        fprintf(stderr, "%s:%d TrnClient - Not initialized. skipInit(%c) See trn_server error messages...\n", __func__, __LINE__, this->_trn_attr.skipInit ? 'Y' : 'N');
    }
    
    return _tercom;
}

void TrnClient::show(int indent, int wkey, int wval)
{
//    fprintf(stderr,"%*s%*s %*s\n",indent,(indent>0?" ":""),wkey,"host",wval,(_server_ip!=NULL?_server_ip:"NULL"));
//    fprintf(stderr,"%*s%*s %*d\n",indent,(indent>0?" ":""),wkey,"port",wval,_sockport);
//    fprintf(stderr,"%*s%*s %*d\n",indent,(indent>0?" ":""),wkey,"fd",wval,_sockfd);
//    fprintf(stderr,"%*s%*s %*c\n",indent,(indent>0?" ":""),wkey,"connected",wval,_connected?'Y':'N');
//    fprintf(stderr,"%*s%*s %*s\n",indent,(indent>0?" ":""),wkey,"trncfg",wval,(_cfg_file!=NULL?_cfg_file:"NULL"));
//    fprintf(stderr,"%*s%*s %*s\n",indent,(indent>0?" ":""),wkey,"logdir",wval,(_logdir!=NULL?_logdir:"NULL"));
//    fprintf(stderr,"%*s%*s %*c\n",indent,(indent>0?" ":""),wkey,"trn svr type",wval,_mbtrn_server_type?'1':'0');
//    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"trn_attr",wval, (void *)&_trn_attr);

    fprintf(stderr,"%*s%*s %*s\n",indent,(indent>0?" ":""),wkey,"logdir",wval,_logdir);
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

    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"saveDirectory",wval,(void *)saveDirectory);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"vehicleSpecFile",wval,(void *)vehicleSpecFile);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"particlesFile",wval,(void *)particlesFile);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"mapFile",wval,(void *)mapFile);



    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"logdir",wval,(void *)_logdir);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"connected",wval,(void *)&_connected);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"trn_server_type",wval,(void *)&_mbtrn_server_type);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"server_ip",wval,(void *)_server_ip);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"sockport",wval,(void *)&_sockport);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"sockfd",wval,(void *)&_sockfd);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"server_addr",wval,(void *)&_server_addr);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"server_msg",wval,(void *)&_server_msg);
    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"comms_buf",wval,(void *)_comms_buf);

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
