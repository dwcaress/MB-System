
///
/// @file TrnAttr.h
/// @authors k. headley
/// @date 2025-09-16
///
/// TRN configuration file (terrainAid.cfg) parser and configuration
///
/// Copyright 2025 Monterey Bay Aquarium Research Institute
/// see LICENSE file for terms of use and license information.

#ifndef _TRNATTR_H_
#define _TRNATTR_H_

#include <stdlib.h>
#include <cstdlib>
#include <cstdio>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <cstring>

// Buffer sizes
#define TA_LINEBUF_BYTES 512
#define TA_PATH_BYTES 1024

// config parameter key definitions
#define TA_MAPNAME_KEY "mapFileName"
#define TA_PARNAME_KEY "particlesName"
#define TA_VEHNAME_KEY "vehicleCfgName"
#define TA_DVLNAME_KEY "dvlCfgName"
#define TA_RESONNAME_KEY "resonCfgName"
#define TA_TRNSVR_KEY "terrainNavServer"
#define TA_LRAUVDVL_KEY "lrauvDvlFilename"
#define TA_MAPTYPE_KEY "map_type"
#define TA_FILTERTYPE_KEY "filterType"
#define TA_TRNPORT_KEY "terrainNavPort"
#define TA_FORCELGF_KEY "forceLowGradeFilter"
#define TA_ALLOWREINIT_KEY "allowFilterReinits"
#define TA_USEMODWT_KEY "useModifiedWeighting"
#define TA_SAMPLEPER_KEY "samplePeriod"
#define TA_MAXNCOV_KEY "maxNorthingCov"
#define TA_MAXNERR_KEY "maxNorthingError"
#define TA_MAXECOV_KEY "maxEastingCov"
#define TA_MAXEERR_KEY "maxEastingError"
#define TA_ROLLOFS_KEY "RollOffset"
#define TA_USEIDTDATA_KEY "useIDTData"
#define TA_USEDVLSIDE_KEY "useDVLSide"
#define TA_USEMBTRNDATA_KEY "useMbTrnData"
#define TA_USEMBTRNSVR_KEY "useMbTrnServer"

class TrnAttr
{
public:

    TrnAttr();
    TrnAttr(const char *cfg_path);
    ~TrnAttr();

    // set the configuration file
    void setCfgFile(const char *cfg_path);

    // load parameters from the configuration file
    int parseConfig();

    // formatted output stream
    void tostream(std::ostream &os, int wkey=20, int wval=28);

    // format as std:string
    std::string tostring(int wkey=20, int wval=28);

    // Check value, free if non-NULL, then set to strdup(src)
    // Only use this on strings that have been malloc'd
    static void chkSetString(char **dest, const char *src);

    // configuration parameters are public
    char *mapName;
    char *particlesName;
    char *vehicleCfgName;
    char *dvlCfgName;
    char *resonCfgName;
    char *lrauvDvlName;
    char *terrainNavServer;
    long terrainNavPort;
    long mapType;
    long filterType;
    bool allowFilterReinits;
    bool useMbTrnData;
    bool useIDTData;
    bool useDvlSide;
    bool skipInit;
    long useModifiedWeighting;
    double maxNorthingCov;
    double maxNorthingError;
    double maxEastingCov;
    double maxEastingError;
    long samplePeriod;
    bool forceLowGradeFilter;
    double phiBias;

protected:
    // config file parser
    static int getNextKeyValue(FILE *cfg, char key[], char value[]);
    // set to default values
    void reset();

private:
    // path to configuration file (e.g. terrainAid.cfg)
    char *_cfg_file;
};

#endif
