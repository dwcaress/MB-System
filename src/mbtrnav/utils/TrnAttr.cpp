///
/// @file TrnAttr.cpp
/// @authors k. headley
/// @date 2025-09-16
///
/// TRN configuration file (terrainAid.cfg) parser and configuration
///
/// Copyright 2025 Monterey Bay Aquarium Research Institute
/// see LICENSE file for terms of use and license information.

#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include "TrnAttr.h"
#include "structDefs.h"

#define CHKNULL(s) (s != NULL ? s : "")

TrnAttr::TrnAttr()
:mapName(NULL)
,particlesName(NULL)
,vehicleCfgName(NULL)
,dvlCfgName(NULL)
,resonCfgName(NULL)
,lrauvDvlName(NULL)
,terrainNavServer(NULL)
,terrainNavPort(27027)
,mapType(2)
,filterType(2)
,allowFilterReinits(false)
,useMbTrnData(false)
,useIDTData(false)
,useDvlSide(false)
,skipInit(false)
,useModifiedWeighting(TRN_WT_NORM)
,maxNorthingCov(0)
,maxNorthingError(0)
,maxEastingCov(0)
,maxEastingError(0)
,samplePeriod(3000)
,forceLowGradeFilter(false)
,phiBias(0)
,_cfg_file(NULL)
{
//    fprintf(stderr,"%s:%d - <<<<< DFL CTOR >>>>>\n", __func__, __LINE__);
}

TrnAttr::TrnAttr(const char *cfg_path)
:mapName(NULL)
,particlesName(NULL)
,vehicleCfgName(NULL)
,dvlCfgName(NULL)
,resonCfgName(NULL)
,lrauvDvlName(NULL)
,terrainNavServer(NULL)
,terrainNavPort(27027)
,mapType(2)
,filterType(2)
,allowFilterReinits(false)
,useMbTrnData(false)
,useIDTData(false)
,useDvlSide(false)
,skipInit(false)
,useModifiedWeighting(TRN_WT_NORM)
,maxNorthingCov(0)
,maxNorthingError(0)
,maxEastingCov(0)
,maxEastingError(0)
,samplePeriod(3000)
,forceLowGradeFilter(false)
,phiBias(0)
,_cfg_file(NULL)
{
    //    fprintf(stderr,"%s:%d - <<<<< INIT CTOR >>>>>\n", __func__, __LINE__);
    if(cfg_path == NULL || strlen(cfg_path) == 0){
        _cfg_file = NULL;
    }else{
        int slen = strlen(cfg_path) + 1;
        _cfg_file = (char *)malloc(slen);
        snprintf(_cfg_file, slen, "%s", cfg_path);
    }
}

TrnAttr::TrnAttr(const TrnAttr& other)
:mapName(NULL)
,particlesName(NULL)
,vehicleCfgName(NULL)
,dvlCfgName(NULL)
,resonCfgName(NULL)
,lrauvDvlName(NULL)
,terrainNavServer(NULL)
,terrainNavPort(27027)
,mapType(2)
,filterType(2)
,allowFilterReinits(false)
,useMbTrnData(false)
,useIDTData(false)
,useDvlSide(false)
,skipInit(false)
,useModifiedWeighting(TRN_WT_NORM)
,maxNorthingCov(0)
,maxNorthingError(0)
,maxEastingCov(0)
,maxEastingError(0)
,samplePeriod(3000)
,forceLowGradeFilter(false)
,phiBias(0)
,_cfg_file(NULL)
{
    //    fprintf(stderr,"%s:%d - <<<<< COPY CTOR >>>>>\n", __func__, __LINE__);
    TrnAttr::chkSetString(&_cfg_file, other._cfg_file);
    TrnAttr::chkSetString(&mapName, other.mapName);
    TrnAttr::chkSetString(&particlesName, other.particlesName);
    TrnAttr::chkSetString(&vehicleCfgName, other.vehicleCfgName);
    TrnAttr::chkSetString(&dvlCfgName, other.dvlCfgName);
    TrnAttr::chkSetString(&resonCfgName, other.resonCfgName);
    TrnAttr::chkSetString(&lrauvDvlName, other.lrauvDvlName);
    TrnAttr::chkSetString(&terrainNavServer, other.terrainNavServer);
    terrainNavPort = other.terrainNavPort;
    mapType = other.mapType;
    filterType = other.filterType;
    allowFilterReinits = other.allowFilterReinits;
    useMbTrnData = other.useMbTrnData;
    useIDTData = other.useIDTData;
    useDvlSide = other.useDvlSide;
    skipInit = other.skipInit;
    useModifiedWeighting = other.useModifiedWeighting;
    maxNorthingCov = other.maxNorthingCov;
    maxNorthingError = other.maxNorthingError;
    maxEastingCov = other.maxEastingCov;
    maxEastingError = other.maxEastingError;
    samplePeriod = other.samplePeriod;
    forceLowGradeFilter = other.forceLowGradeFilter;
    phiBias = other.phiBias;
}

TrnAttr::~TrnAttr()
{
    //    fprintf(stderr,"%s:%d - <<<<< DTOR >>>>>\n", __func__, __LINE__);

    if(_cfg_file != NULL)
        free(_cfg_file);
    _cfg_file = NULL;
    if(mapName != NULL)
        free(mapName);
    mapName = NULL;
    if(particlesName != NULL)
        free(particlesName);
    particlesName = NULL;
    if(vehicleCfgName != NULL)
        free(vehicleCfgName);
    vehicleCfgName = NULL;
    if(dvlCfgName != NULL)
        free(dvlCfgName);
    dvlCfgName = NULL;
    if(resonCfgName != NULL)
        free(resonCfgName);
    resonCfgName = NULL;
    if(lrauvDvlName != NULL)
        free(lrauvDvlName);
    lrauvDvlName = NULL;
    if(terrainNavServer != NULL)
        free(terrainNavServer);
    terrainNavServer = NULL;
}



void TrnAttr::chkSetString(char **dest, const char *src)
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

void TrnAttr::setCfgFile(const char *cfg_path)
{
    TrnAttr::chkSetString(&_cfg_file, NULL);

    if(cfg_path != NULL){
        int slen = strlen(cfg_path) + 1;
        _cfg_file = (char *)malloc(slen);
        snprintf(_cfg_file, slen, "%s", cfg_path);
    }
}

void TrnAttr::reset()
{
    // Initialize to default values
    TrnAttr::chkSetString(&mapName, NULL);
    TrnAttr::chkSetString(&particlesName, NULL);
    TrnAttr::chkSetString(&vehicleCfgName, NULL);
    TrnAttr::chkSetString(&dvlCfgName, NULL);
    TrnAttr::chkSetString(&resonCfgName, NULL);
    TrnAttr::chkSetString(&lrauvDvlName, NULL);
    TrnAttr::chkSetString(&terrainNavServer, NULL);

    mapType = 2;
    filterType = 2;
    terrainNavPort = 27027;
    forceLowGradeFilter = false;
    allowFilterReinits = false;
    useModifiedWeighting = TRN_WT_NORM;
    samplePeriod = 3000;
    maxNorthingCov = 0.;
    maxNorthingError = 0.;
    maxEastingCov = 0.;
    maxEastingError = 0.;
    phiBias = 0;
    useIDTData = false;
    useDvlSide = false;
    useMbTrnData   = false;
}

int TrnAttr::parseConfig()
{
    if(_cfg_file == NULL) {

        fprintf(stderr, "%s - ERR Config file NULL/unset\n", __func__);
        return 1;
    }

    if (access(_cfg_file, F_OK) < 0) {
        fprintf(stderr, "%s - ERR - Could not find %s\n", __func__, _cfg_file);
        return 1;
    }

    FILE *cfg = fopen(_cfg_file, "r");
    if (!cfg) {
        fprintf(stderr, "%s - ERR - Could not open %s\n", __func__, _cfg_file);
        return 1;
    }

    reset();

    // Get   key = value   pairs from non-comment lines in the cfg.
    // If the key matches a known config item, extract and save the value.
    char key[TA_KEY_BYTES], value[TA_VALUE_BYTES];

    while (getNextKeyValue(cfg, key, TA_KEY_BYTES, value, TA_VALUE_BYTES)) {
        fprintf(stderr, "%s:%d key[%s] value[%s]\n",__func__, __LINE__, key, value);
        if (!strcmp(TA_MAPNAME_KEY, key)) {
            chkSetString(&mapName, value);
        } else if (!strcmp(TA_PARNAME_KEY, key)) {
            chkSetString(&particlesName, value);
        } else if (!strcmp(TA_VEHNAME_KEY, key)) {
            chkSetString(&vehicleCfgName, value);
        } else if (!strcmp(TA_DVLNAME_KEY, key)) {
            chkSetString(&dvlCfgName, value);
        } else if (!strcmp(TA_RESONNAME_KEY, key)) {
            chkSetString(&resonCfgName, value);
        } else if (!strcmp(TA_TRNSVR_KEY, key)) {
            chkSetString(&terrainNavServer, value);
        } else if (!strcmp(TA_LRAUVDVL_KEY, key)) {
            chkSetString(&lrauvDvlName, value);
        } else if (!strcmp(TA_MAPTYPE_KEY, key))  {
            mapType = atoi(value);
        } else if (!strcmp(TA_FILTERTYPE_KEY, key)) {
            filterType = atoi(value);
        } else if (!strcmp(TA_TRNPORT_KEY, key)) {
            terrainNavPort = atol(value);
        } else if (!strcmp(TA_FORCELGF_KEY,  key)) {
            forceLowGradeFilter = strcasecmp("false", value);
        } else if (!strcmp(TA_ALLOWREINIT_KEY, key)) {
            allowFilterReinits = strcasecmp("false", value);
        } else if (!strcmp(TA_USEMODWT_KEY, key)) {
            useModifiedWeighting = atoi(value);
        } else if (!strcmp(TA_SAMPLEPER_KEY, key)) {
            samplePeriod = atoi(value);
        } else if (!strcmp(TA_MAXNCOV_KEY, key)) {
            maxNorthingCov = atof(value);
        } else if (!strcmp(TA_MAXNERR_KEY, key)) {
            maxNorthingError = atof(value);
        } else if (!strcmp(TA_MAXECOV_KEY, key)) {
            maxEastingCov = atof(value);
        } else if (!strcmp(TA_MAXEERR_KEY, key)) {
            maxEastingError = atof(value);
        } else if (!strcmp(TA_ROLLOFS_KEY, key)) {
            phiBias = atof(value);
        } else if (!strcmp(TA_USEIDTDATA_KEY, key)) {
            useIDTData = (strcasecmp("false", value) == 0 ? false : true);
        } else if (!strcmp(TA_USEDVLSIDE_KEY, key)) {
            useDvlSide = (strcasecmp("false", value) == 0 ? false : true);
        } else if (!strcmp(TA_USEMBTRNDATA_KEY, key)) {
            // Use the MbTrn.log file data when using either MbTrn data mode
            useMbTrnData = (strcasecmp("false", value) == 0 ? false : true);
        } else if (!strcmp(TA_USEMBTRNSVR_KEY, key)) {
            useMbTrnData = (strcasecmp("false", value) == 0 ? false : true);
        }else if (!strcmp(TA_SKIPINIT_KEY, key)) {
            skipInit = (strcasecmp("false", value) == 0 ? false : true);
        }  else if(strlen(key) > 0){
            fprintf(stderr, "\n%s - ERR - Unknown key in cfg: %s\n", __func__, key);
        }
        memset(key, 0, TA_KEY_BYTES);
        memset(value, 0, TA_VALUE_BYTES);
    }

    fclose(cfg);
    return 0;
}

// Return the next key and value pair in the cfg file.
// Function returns 0 if no key/value pair was found,
// otherwise 1.
int TrnAttr::getNextKeyValue(FILE *cfg, char key[], uint16_t klen, char value[], uint16_t vlen)
{
    // Continue reading from cfg skipping comment lines
    //
    char line[TA_LINEBUF_BYTES];
    while (fgets(line, sizeof(line), cfg)) {
        // Chop off leading whitespace, then look for '//'
        size_t i = 0;
        while(line[i] == ' ' || line[i] == '\t' || line[i] == '\n' || line[i] == '\r'
              || line[i] == '\f' || line[i] == '\v') i++;

        if (strncmp("//", line+i, 2)) {
            // If a non-comment line was read from the config file,
            // extract the key and value
            char *loc;
            char sfmt[64]={0};
            // generate format string with with width limits to
            // prevent buffer overflows (e.g. "%100s = %100s"
            snprintf(sfmt, 64, "%%%ds = %%%ds", (klen-1), (vlen-1));
//            sscanf(line, "%"(klen-1)"s = %"(vlen-1)"s", klen-1, key, vlen-1, value);
            sscanf(line, sfmt, key, value);
            if ( (loc = strchr(value, ';')) ) *loc = '\0';  // Remove ';' from the value
            return 1;
        }
    }

    return 0;
}

void TrnAttr::tostream(std::ostream &os, int wkey, int wval){
    os << std::setw(wkey) << std::setfill(' ') << "this";
    os << std::setw(wval) << (void *)(this) << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_MAPNAME_KEY;
    os << std::setw(wval) << CHKNULL(mapName) << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_VEHNAME_KEY;
    os << std::setw(wval) << CHKNULL(vehicleCfgName) << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_RESONNAME_KEY;
    os << std::setw(wval) << CHKNULL(resonCfgName) << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_DVLNAME_KEY;
    os << std::setw(wval) << CHKNULL(dvlCfgName) << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_PARNAME_KEY;
    os << std::setw(wval) << CHKNULL(particlesName) << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_LRAUVDVL_KEY;
    os << std::setw(wval) << CHKNULL(lrauvDvlName) << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_TRNSVR_KEY;
    os << std::setw(wval) << CHKNULL(terrainNavServer) << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_TRNPORT_KEY;
    os << std::setw(wval) << terrainNavPort << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_FILTERTYPE_KEY;
    os << std::setw(wval) << filterType << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_MAPTYPE_KEY;
    os << std::setw(wval) << mapType << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_ALLOWREINIT_KEY;
    os << std::setw(wval) << (allowFilterReinits ? 'Y' : 'N') << std::endl;

    os << std::setw(wkey) << std::setfill(' ') << TA_USEIDTDATA_KEY;
    os << std::setw(wval) << (useIDTData ? 'Y' : 'N') << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_USEMBTRNDATA_KEY;
    os << std::setw(wval) << (useMbTrnData ? 'Y' : 'N') << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_USEDVLSIDE_KEY;
    os << std::setw(wval) << (useDvlSide ? 'Y' : 'N') << std::endl;

    os << std::setw(wkey) << std::setfill(' ') << TA_SAMPLEPER_KEY;
    os << std::setw(wval) << samplePeriod << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_MAXNCOV_KEY;
    os << std::setw(wval) << maxNorthingCov << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_MAXNERR_KEY;
    os << std::setw(wval) << maxNorthingError << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_MAXECOV_KEY;
    os << std::setw(wval) << maxEastingCov << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_MAXEERR_KEY;
    os << std::setw(wval) << maxEastingError << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_ROLLOFS_KEY;
    os << std::setw(wval) << phiBias << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_USEMODWT_KEY;
    os << std::setw(wval) << useModifiedWeighting << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_FORCELGF_KEY;
    os << std::setw(wval) << (forceLowGradeFilter ? 'Y' : 'N') << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_SKIPINIT_KEY;
    os << std::setw(wval) << (skipInit ? 'Y' : 'N') << std::endl;
}

std::string TrnAttr::tostring(int wkey, int wval)
{
    std::ostringstream ss;
    tostream(ss, wkey, wval);
    return ss.str();
}

void TrnAttr::atostream(std::ostream &os, int wkey, int wval){
    os << std::setw(wkey) << std::setfill(' ') << "this";
    os << std::setw(wval) << (void *)(this) << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_MAPNAME_KEY;
    os << std::setw(wval) << (void *)(mapName) << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_PARNAME_KEY;
    os << std::setw(wval) << (void *)(particlesName) << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_VEHNAME_KEY;
    os << std::setw(wval) << (void *)(vehicleCfgName) << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_DVLNAME_KEY;
    os << std::setw(wval) << (void *)(dvlCfgName) << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_RESONNAME_KEY;
    os << std::setw(wval) << (void *)(resonCfgName) << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_LRAUVDVL_KEY;
    os << std::setw(wval) << (void *)(lrauvDvlName) << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_TRNSVR_KEY;
    os << std::setw(wval) << (void *)(terrainNavServer) << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_TRNPORT_KEY;
    os << std::setw(wval) << (void *)&terrainNavPort << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_MAPTYPE_KEY;
    os << std::setw(wval) << (void *)&mapType << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_FILTERTYPE_KEY;
    os << std::setw(wval) << (void *)&filterType << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_ALLOWREINIT_KEY;
    os << std::setw(wval) << (void *)&allowFilterReinits << std::endl;

    os << std::setw(wkey) << std::setfill(' ') << TA_USEMBTRNDATA_KEY;
    os << std::setw(wval) << (void *)(&useMbTrnData) << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_USEIDTDATA_KEY;
    os << std::setw(wval) << (void *)(useIDTData) << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_USEDVLSIDE_KEY;
    os << std::setw(wval) << (void *)(&useDvlSide) << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_SKIPINIT_KEY;
    os << std::setw(wval) << (void *)(&skipInit) << std::endl;

    os << std::setw(wkey) << std::setfill(' ') << TA_USEMODWT_KEY;
    os << std::setw(wval) << (void *)&useModifiedWeighting << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_MAXNCOV_KEY;
    os << std::setw(wval) << (void *)&maxNorthingCov << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_MAXNERR_KEY;
    os << std::setw(wval) << (void *)&maxNorthingError << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_MAXECOV_KEY;
    os << std::setw(wval) << (void *)&maxEastingCov << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_MAXEERR_KEY;
    os << std::setw(wval) << (void *)&maxEastingError << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_SAMPLEPER_KEY;
    os << std::setw(wval) << (void *)&samplePeriod << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_FORCELGF_KEY;
    os << std::setw(wval) << (void *)(&forceLowGradeFilter) << std::endl;
    os << std::setw(wkey) << std::setfill(' ') << TA_ROLLOFS_KEY;
    os << std::setw(wval) << (void *)&phiBias << std::endl;
}

std::string TrnAttr::atostring(int wkey, int wval)
{
    std::ostringstream ss;
    atostream(ss, wkey, wval);
    return ss.str();
}
