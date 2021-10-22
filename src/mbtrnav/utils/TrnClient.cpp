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


TRN_attr::TRN_attr()
{
    // Initialize to default values
    //
    _mapFileName = NULL;
    _map_type = 2;
    _filter_type = 2;
    _particlesName = NULL;
    _vehicleCfgName = NULL;
    _dvlCfgName = NULL;
    _resonCfgName = NULL;
    _terrainNavServer = NULL;
    _lrauvDvlFilename = 0;
    _terrainNavPort = 27027;
    _forceLowGradeFilter = false;
    _allowFilterReinits = false;
    _useModifiedWeighting = TRN_WT_NORM;
    _samplePeriod = 3000;
    _maxNorthingCov = 0.;
    _maxNorthingError = 0.;
    _maxEastingCov = 0.;
    _maxEastingError = 0.;
    _phiBias = 0;
    _useIDTData = false;
    _useDvlSide = false;
    _useMbTrnData = false;
}

TRN_attr::~TRN_attr()
{
    free(_mapFileName);
    free(_particlesName);
    free(_vehicleCfgName);
    free(_dvlCfgName);
    free(_terrainNavServer);
    free(_resonCfgName);
}

// Common to QNX and NIX versions
// 
TrnClient::TrnClient(const char *host, int port)
: TerrainNavClient(),
    _cfg_file(NULL),
   verbose(0)
{
    try{
        _trn_attr = new TRN_attr;
        char *ld =getenv("TRN_LOGDIR");
        _logdir    = (NULL!=ld ? STRDUPNULL(ld): STRDUPNULL(LOGDIR_DFL));

    }catch (Exception e){
        printf("\n];\n");
    }

    if (NULL!=host){
        free(_server_ip);
        _server_ip = strdup(host);
        _sockport = port;
        free(_trn_attr->_terrainNavServer);
        _trn_attr->_terrainNavServer = strdup(host);
        _trn_attr->_terrainNavPort = port;
    }

    fprintf(stderr, "\nServer      : %s :%ld\n",_trn_attr->_terrainNavServer, _trn_attr->_terrainNavPort);
    fprintf(stderr,"Vehicle Cfg : %s\n\n", _trn_attr->_vehicleCfgName);
    _initialized = false;
    
}

TrnClient::TrnClient(const char *svr_log_dir, const char *host, int port)
: TerrainNavClient(),
_cfg_file(NULL),
verbose(0)
{
    try {
        _trn_attr = new TRN_attr;

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
        free(_trn_attr->_terrainNavServer);
        _trn_attr->_terrainNavServer = strdup(host);
        _trn_attr->_terrainNavPort = port;
    }

    fprintf(stderr, "\nServer      : %s :%ld\n",_trn_attr->_terrainNavServer, _trn_attr->_terrainNavPort);
    fprintf(stderr,"Vehicle Cfg : %s\n\n", _trn_attr->_vehicleCfgName);

    _initialized = false;

}

TrnClient::~TrnClient()
{
    free(_cfg_file);
    delete _trn_attr;
}


int TrnClient::loadCfgAttributes(const char *cfg_file)
{
    char cfg_buf[300];
    char *cfg_path=NULL;

    if(NULL!=cfg_file){
        // set _cfg_file member and use that
        free(_cfg_file);
        _cfg_file=strdup(cfg_file);
        cfg_path = _cfg_file;
    }else{
        // use default path ($TRN_CONFIGDIR/terrainAid.cfg or ./terrainAid.cfg)
        cfg_path=cfg_buf;
        const char *cfg_dir = getenv("TRN_DATAFILES");
        sprintf(cfg_buf, "%s/terrainAid.cfg", (NULL!=cfg_dir ? cfg_dir : "."));
    }

    if (access(cfg_path, F_OK) < 0){
        fprintf(stderr, "TrnClient - Could not find config file %s\n", cfg_path);
        return 1;
    }

    fprintf(stderr, "TrnClient - opening config file %s\n", cfg_path);
    FILE *cfg = fopen(cfg_path, "r");
    if (!cfg)
    {
        fprintf(stderr, "TrnClient - Could not open %s\n", cfg_path);
        return 1;
    }


    // Get   key = value   pairs from non-comment lines in the cfg.
    // If the key matches a known config item, extract and save the value.
    //
    char key[100], value[200];
    while (getNextKeyValue(cfg, key, value))
    {
        if      (!strcmp("mapFileName",          key)){
            free(_trn_attr->_mapFileName);
            _trn_attr->_mapFileName = strdup(value);
        }
        else if (!strcmp("particlesName",        key)){
            free(_trn_attr->_particlesName);
            _trn_attr->_particlesName = strdup(value);
        }
        else if (!strcmp("vehicleCfgName",       key)){
            free(_trn_attr->_vehicleCfgName);
            _trn_attr->_vehicleCfgName = strdup(value);
        }
        else if (!strcmp("dvlCfgName",           key)){
            free(_trn_attr->_dvlCfgName);
            _trn_attr->_dvlCfgName = strdup(value);
        }
        else if (!strcmp("resonCfgName",         key)){
            free(_trn_attr->_resonCfgName);
            _trn_attr->_resonCfgName = strdup(value);
        }
        else if (!strcmp("terrainNavServer",     key)){
            free(_trn_attr->_terrainNavServer);
            _trn_attr->_terrainNavServer = strdup(value);
        }
        else if (!strcmp("lrauvDvlFilename",     key)){
            free(_trn_attr->_lrauvDvlFilename);
            _trn_attr->_lrauvDvlFilename = strdup(value);
        }
        else if (!strcmp("map_type",             key))  _trn_attr->_map_type = atoi(value);
        else if (!strcmp("filterType",           key))  _trn_attr->_filter_type = atoi(value);
        else if (!strcmp("terrainNavPort",       key))  _trn_attr->_terrainNavPort = atol(value);
        else if (!strcmp("forceLowGradeFilter",  key))  _trn_attr->_forceLowGradeFilter = strcasecmp("false", value);
        else if (!strcmp("allowFilterReinits",   key))  _trn_attr->_allowFilterReinits = strcasecmp("false", value);
        else if (!strcmp("useModifiedWeighting", key))  _trn_attr->_useModifiedWeighting = atoi(value);
        else if (!strcmp("samplePeriod",         key))  _trn_attr->_samplePeriod = atoi(value);
        else if (!strcmp("maxNorthingCov",       key))  _trn_attr->_maxNorthingCov = atof(value);
        else if (!strcmp("maxNorthingError",     key))  _trn_attr->_maxNorthingError = atof(value);
        else if (!strcmp("maxEastingCov",        key))  _trn_attr->_maxEastingCov = atof(value);
        else if (!strcmp("maxEastingError",      key))  _trn_attr->_maxEastingError = atof(value);
        else if (!strcmp("RollOffset",           key))  _trn_attr->_phiBias = atof(value);
        else if (!strcmp("useIDTData",           key))  _trn_attr->_useIDTData = strcasecmp("false", value);
        else if (!strcmp("useDVLSide",           key))  _trn_attr->_useDvlSide = strcasecmp("false", value);
        else if (!strcmp("useMbTrnData",         key))  _trn_attr->_useMbTrnData = strcasecmp("false", value);
        else
            fprintf(stderr, "\n\tTrnClient: Unknown key in cfg: %s\n\n", key);
    }

    // configure this instance and TRNConfig
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

        getSessionDir(_logdir, sessiondir,512,false);

        sprintf(mapname, "%s/%s", mapPath, _trn_attr->_mapFileName);
        sprintf(vehiclename, "%s/%s", cfgPath, _trn_attr->_vehicleCfgName);
        sprintf(particlename, "%s/%s", cfgPath, _trn_attr->_particlesName);

        free(mapFile);
        free(vehicleSpecFile);
        free(particlesFile);
        free(saveDirectory);
        this->mapFile = STRDUPNULL(mapname);
        this->vehicleSpecFile = STRDUPNULL(vehiclename);
        this->particlesFile = STRDUPNULL(particlename);
        this->saveDirectory = STRDUPNULL(sessiondir);

        this->mapType = _trn_attr->_map_type;
        this->filterType = _trn_attr->_filter_type;

        fprintf(stderr, "%s: map    : %s\n", __func__, mapname);
        fprintf(stderr, "%s: logPath: %s\n", __func__, logPath);
        fprintf(stderr, "%s: logdir : %s\n", __func__, _logdir);
        fprintf(stderr, "%s: veh    : %s\n", __func__, vehiclename);
        fprintf(stderr, "%s: par    : %s\n", __func__, particlename);
        fprintf(stderr, "%s: save   : %s\n", __func__, saveDirectory);

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

    }catch (Exception e){
        printf("\n];\n");
    }

    fprintf(stderr, "\nServer      : %s :%d\n",_server_ip, _sockport);
    fprintf(stderr,"Vehicle Cfg : %s\n\n", _trn_attr->_vehicleCfgName);

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

/****************************************************************************/


// Return the next key and value pair in the cfg file.
// Function returns 0 if no key/value pair was found,
// otherwise 1.
// 
int TrnClient::getNextKeyValue(FILE *cfg, char key[], char value[])
{
  // Continue reading from cfg skipping comment lines
  // 
  char line[300]; 
  while (fgets(line, sizeof(line), cfg))
  {
    // Chop off leading whitespace, then look for '//'
    // 
    size_t i = 0;
    while(line[i] == ' ' || line[i] == '\t' || line[i] == '\n' || line[i] == '\r'
      || line[i] == '\f' || line[i] == '\v') i++;

    if (strncmp("//", line+i, 2))
    {
      // If a non-comment line was read from the config file,
      // extract the key and value
      // 
      char *loc; 
      sscanf(line, "%s = %s", key, value);
      if ( (loc = strchr(value, ';')) ) *loc = '\0';  // Remove ';' from the value
      return 1;
    }
  }

  return 0;

}

int TrnClient::setVerbose(int val)
{
    verbose=val;
    return 0;
}


// Common to QNX and NIX versions
// 
TerrainNav* TrnClient::connectTRN()
{
    TerrainNav *_tercom = NULL;
    
    fprintf(stdout, "TrnClient - Using TerrainNav [%s:%ld]\n",
            _trn_attr->_terrainNavServer, _trn_attr->_terrainNavPort);
    
    try
    {
        init_comms();

        // On linux and cygwin, we can use a native TerrainNav object instead
        // of relying on a trn_server. Call useTRNServer() to decide.
        //
        printf("Connecting to %s...\n", _trn_attr->_terrainNavServer);
        _tercom = static_cast<TerrainNav *>(this);
    }
    catch (Exception e)
    {
        fprintf(stderr, "TrnClient - Failed TRN connection. Check TRN error messages...\n");
    }
    
    if (NULL!=_tercom && _tercom->is_connected() ){
        init_server();
        // If we reach here then we've connected
        //
        fprintf(stdout, "TrnClient - connected to server if no error messages...\n");

    } else{
        fprintf(stderr, "TrnClient - Not initialized. See trn_server error messages...\n");
    }
    
    return _tercom;
}
