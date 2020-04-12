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


// Common to QNX and NIX versions
// 
TrnClient::TrnClient(const char *host, int port)
: TerrainNavClient(),
logdir(NULL),
   lastTime(0.0), nupdates(0L), nreinits(0L), verbose(0)
{

    
  try
  {
    trn_attr = new TRN_attr;
    if (0 != loadCfgAttributes())
    {
      fprintf(stderr, "\nTrnClient - Log directory %s not found\n\n", logdir);
    }
  }
  catch (Exception e)
  {
    printf("\n];\n");
  }

  if (NULL!=host)
  {
      _server_ip = strdup(host);
      _sockport = port;
      trn_attr->_terrainNavServer = strdup(host);
    trn_attr->_terrainNavPort = port;
  }
  
    fprintf(stderr, "\nServer      : %s :%d\n",host, port);
    fprintf(stderr,"Vehicle Cfg : %s\n\n", trn_attr->_vehicleCfgName);
    
    _logdir    = (NULL!=logdir ? strdup(logdir):NULL);
    _initialized = false;
    
}

TrnClient::~TrnClient()
{
  if (trn_attr) delete trn_attr;
  if (logdir)   delete logdir;
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

// Could be common to QNX and NIX versions. 
// Just use this function in the QNX version
// since it works OK.
// 
int TrnClient::loadCfgAttributes()
{
  char cfgfile[300];
  sprintf(cfgfile, "%s/terrainAid.cfg", logdir);
  if (access(cfgfile, F_OK) < 0)
  {
    fprintf(stderr, "TrnClient - Could not find %s", cfgfile);
    return 1;
  }

  FILE *cfg = fopen(cfgfile, "r");
  if (!cfg)
  {
    fprintf(stderr, "TrnClient - Could not open %s", cfgfile);
    return 1;
  }

  // Initialize to default values
  // 
  trn_attr->_mapFileName = NULL;
  trn_attr->_map_type = 2;
  trn_attr->_filter_type = 2;
  trn_attr->_particlesName = NULL;
  trn_attr->_vehicleCfgName = NULL;
  trn_attr->_dvlCfgName = NULL;
  trn_attr->_resonCfgName = NULL;
  trn_attr->_terrainNavServer = NULL;
  trn_attr->_lrauvDvlFilename = 0;
  trn_attr->_terrainNavPort = 27027;
  trn_attr->_forceLowGradeFilter = false;
  trn_attr->_allowFilterReinits = false;
  trn_attr->_useModifiedWeighting = TRN_WT_NORM;
  trn_attr->_samplePeriod = 3000;
  trn_attr->_maxNorthingCov = 0.;
  trn_attr->_maxNorthingError = 0.;
  trn_attr->_maxEastingCov = 0.;
  trn_attr->_maxEastingError = 0.;
  trn_attr->_phiBias = 0;
  trn_attr->_useIDTData = false;
  trn_attr->_useDvlSide = false;
  trn_attr->_useMbTrnData = false;

  // Get   key = value   pairs from non-comment lines in the cfg.
  // If the key matches a known config item, extract and save the value.
  // 
  char key[100], value[200];
  while (getNextKeyValue(cfg, key, value))
  {
    if      (!strcmp("mapFileName",          key))  trn_attr->_mapFileName = strdup(value);
    else if (!strcmp("map_type",             key))  trn_attr->_map_type = atoi(value);
    else if (!strcmp("filterType",           key))  trn_attr->_filter_type = atoi(value);
    else if (!strcmp("particlesName",        key))  trn_attr->_particlesName = strdup(value);
    else if (!strcmp("vehicleCfgName",       key))  trn_attr->_vehicleCfgName = strdup(value);
    else if (!strcmp("dvlCfgName",           key))  trn_attr->_dvlCfgName = strdup(value);
    else if (!strcmp("resonCfgName",         key))  trn_attr->_resonCfgName = strdup(value);
    else if (!strcmp("terrainNavServer",     key))  trn_attr->_terrainNavServer = strdup(value);
    else if (!strcmp("lrauvDvlFilename",     key))  trn_attr->_lrauvDvlFilename = strdup(value);
    else if (!strcmp("terrainNavPort",       key))  trn_attr->_terrainNavPort = atol(value);
    else if (!strcmp("forceLowGradeFilter",  key))  trn_attr->_forceLowGradeFilter = strcasecmp("false", value);
    else if (!strcmp("allowFilterReinits",   key))  trn_attr->_allowFilterReinits = strcasecmp("false", value);
    else if (!strcmp("useModifiedWeighting", key))  trn_attr->_useModifiedWeighting = atoi(value);
    else if (!strcmp("samplePeriod",         key))  trn_attr->_samplePeriod = atoi(value);
    else if (!strcmp("maxNorthingCov",       key))  trn_attr->_maxNorthingCov = atof(value);
    else if (!strcmp("maxNorthingError",     key))  trn_attr->_maxNorthingError = atof(value);
    else if (!strcmp("maxEastingCov",        key))  trn_attr->_maxEastingCov = atof(value);
    else if (!strcmp("maxEastingError",      key))  trn_attr->_maxEastingError = atof(value);
    else if (!strcmp("RollOffset",           key))  trn_attr->_phiBias = atof(value);
    else if (!strcmp("useIDTData",           key))  trn_attr->_useIDTData = strcasecmp("false", value);
    else if (!strcmp("useDVLSide",           key))  trn_attr->_useDvlSide = strcasecmp("false", value);
    else if (!strcmp("useMbTrnData",         key))  trn_attr->_useMbTrnData = strcasecmp("false", value);
    else
      fprintf(stderr, "\n\tTrnClient: Unknown key in cfg: %s\n\n", key);
  }

  return 0;
}

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
            trn_attr->_terrainNavServer, trn_attr->_terrainNavPort);
    
    try
    {
        init_comms();
        
        // On linux and cygwin, we can use a native TerrainNav object instead
        // of relying on a trn_server. Call useTRNServer() to decide.
        //
        printf("Connecting to %s...\n", trn_attr->_terrainNavServer);
        _tercom = static_cast<TerrainNav *>(this);
    }
    catch (Exception e)
    {
        fprintf(stderr, "TrnClient - Failed TRN connection. Check TRN error messages...\n");
    }
    
    if (NULL!=_tercom && _tercom->is_connected() ){
        // If we reach here then we've connected
        //
        fprintf(stdout, "TrnClient - connected to server if no error messages...\n");
        
        fprintf(stdout, "TrnClient - configuring remote TRN instance...\n");
        try{
            // The following calls to setup TRN lifted from TerrainAidDriver
            //
            _tercom->setInterpMeasAttitude(true);

            if(verbose>0)
            fprintf(stderr,"%s:%d\n",__FUNCTION__,__LINE__);

            //choose filter settings based on whether kearfott is available and if
            //filter forcing is set
            if(trn_attr->_forceLowGradeFilter)
                _tercom->useLowGradeFilter();
            else
                _tercom->useHighGradeFilter();
            
            if(verbose>0)
            fprintf(stderr,"%s:%d\n",__FUNCTION__,__LINE__);

            //turn on filter reintialization if set in terrainAid.cfg
            _tercom->setFilterReinit(trn_attr->_allowFilterReinits);

            if(verbose>0)
            fprintf(stderr,"%s:%d\n",__FUNCTION__,__LINE__);

            //turn on modified weighting if set in terrainAid.cfg
            _tercom->setModifiedWeighting(trn_attr->_useModifiedWeighting);

            if(verbose>0)
            fprintf(stderr,"%s:%d\n",__FUNCTION__,__LINE__);
        }catch (Exception e)
        {
            fprintf(stderr, "TrnClient - caught exception...\n");
        }

    } else{
        fprintf(stderr, "TrnClient - Not initialized. See trn_server error messages...\n");
    }
    
    return _tercom;
}
/*
int TrnClient::send_msg(commsT& msg)
{
    int sl = 0;
    fprintf(stderr,"%s:%d\n",__FUNCTION__,__LINE__);
    //printf("TrnClient - send_msg(): Sending %s\n",
    //msg.to_s(_comms_buf, sizeof(_comms_buf)));
    
    // Check to see if client is still connected first
    //
    if (is_connected()) {
        memset(_comms_buf, 0, sizeof(_comms_buf));
        msg.serialize(_comms_buf);
        
        // Send the whole message

        // TerrainNavClient sends one chunk, then the rest...why?
//        for (sl = 0; sl < TRN_CHUNK_SIZE;) {
//            sl += send(_sockfd, _comms_buf+sl, TRN_CHUNK_SIZE-sl, 0);
//            //printf("server:send_msg - sent %d bytes\n", sl);
//        }
//        for (sl = TRN_CHUNK_SIZE; sl < sizeof(_comms_buf);) {
//            sl += send(_sockfd, _comms_buf+sl, sizeof(_comms_buf)-sl, 0);
//            //printf("server:send_msg - sent %d bytes\n", sl);
//        }
        sl = send(_sockfd, _comms_buf, sizeof(_comms_buf), 0);
    }
    else {
        fprintf(stderr,"%s - Can't send - no server connection!\n",__FUNCTION__);
        throw Exception("TRN Server connection lost");
    }
    return sl;
}
*/
