/****************************************************************************/
/* Copyright (c) 2017 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  : TRN test client using logged data from previous mission  */
/* Filename : TrnClient.h                                                   */
/* Author   : headley                                                       */
/* Project  :                                                              */
/* Version  : 1.0                                                           */
/* Created  : 10/24/2019                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/* Began with a copy of test_client as there is a lot of stuff to reuse     */
/****************************************************************************/
#ifndef _TRN_CLIENT_H_
#define _TRN_CLIENT_H_

#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include "TerrainNavClient.h"
#include "structDefs.h"

class DataLogReader;
struct TRN_attr;
class poseT;
class measT;
class TerrainNav;

#define  Boolean  bool


struct TRN_attr
{
    char *_mapFileName;
    long  _map_type;
    long  _filter_type;
    char *_particlesName;
    char *_vehicleCfgName;
    char *_dvlCfgName;
    char *_resonCfgName;
    char *_terrainNavServer;
    char *_lrauvDvlFilename;
    long  _terrainNavPort;
    Boolean _forceLowGradeFilter;
    Boolean _allowFilterReinits;
    long _useModifiedWeighting;
    long _samplePeriod;
    double _maxNorthingCov;
    double _maxEastingCov;
    double _maxNorthingError;
    double _maxEastingError;
    double _phiBias;
    Boolean _useIDTData;
    Boolean _useDvlSide;
    Boolean _useMbTrnData;
};


class TrnClient : public TerrainNavClient {
    
public:
    
    TrnClient(const char *host=0, int port=0);
    ~TrnClient();
    
    int loadCfgAttributes();
    int getNextKeyValue(FILE *cfg, char key[], char value[]);
    int setVerbose(int val);
    TerrainNav* connectTRN();
    
protected:
    char *logdir;
    double lastTime;
    TRN_attr *trn_attr;
    long nupdates, nreinits;
    int verbose;
    
};

#endif
