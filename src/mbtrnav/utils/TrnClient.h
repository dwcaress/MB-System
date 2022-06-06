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


#define TRNCLI_PORT_DFL 27027
#define VNORM_DIM 3
#undef WITH_VNORM_FN
#undef WITH_DEGTORAD_FN

class DataLogReader;
struct TRN_attr;
struct poseT;
struct measT;
class TerrainNav;


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
    bool _forceLowGradeFilter;
    bool _allowFilterReinits;
    long _useModifiedWeighting;
    long _samplePeriod;
    double _maxNorthingCov;
    double _maxEastingCov;
    double _maxNorthingError;
    double _maxEastingError;
    double _phiBias;
    bool _useIDTData;
    bool _useDvlSide;
    bool _useMbTrnData;
    bool _skipInit;
    TRN_attr();
    ~TRN_attr();
};


class TrnClient : public TerrainNavClient {
    
public:
    
    TrnClient(const char *host=NULL, int port=0);
    TrnClient(const char *svr_log_dir=NULL, const char *host=NULL, int port=0);
    ~TrnClient();
    int initSocket();
    int connectSocket();
    int loadCfgAttributes(const char *cfg_file);
    int getNextKeyValue(FILE *cfg, char key[], char value[]);
    int setVerbose(int val);
    void show(int indent=0, int wkey=15, int wval=18);
    TerrainNav* connectTRN();
protected:
    char *_cfg_file;
    TRN_attr *_trn_attr;
    int verbose;
    
};

#endif
