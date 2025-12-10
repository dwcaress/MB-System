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
    if(cfg_path == NULL || strlen(cfg_path) == 0){
        _cfg_file = NULL;
    }else{
        int slen = strlen(cfg_path) + 1;
        _cfg_file = (char *)malloc(slen);
        snprintf(_cfg_file, slen, "%s", cfg_path);
    }
}

TrnAttr::~TrnAttr()
{
    TrnAttr::chkSetString(&_cfg_file, NULL);
    TrnAttr::chkSetString(&mapName, NULL);
    TrnAttr::chkSetString(&particlesName, NULL);
    TrnAttr::chkSetString(&vehicleCfgName, NULL);
    TrnAttr::chkSetString(&dvlCfgName, NULL);
    TrnAttr::chkSetString(&resonCfgName, NULL);
    TrnAttr::chkSetString(&lrauvDvlName, NULL);
    TrnAttr::chkSetString(&terrainNavServer, NULL);
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

    if(cfg_path != NULL || strlen(cfg_path) > 0){
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
    char key[100], value[200];
    while (getNextKeyValue(cfg, key, value)) {
        if (!strcmp(TA_MAPNAME_KEY, key)) {
            free(mapName);
            mapName = strdup(value);
        } else if (!strcmp(TA_PARNAME_KEY, key)) {
            free(particlesName);
            particlesName = strdup(value);
        } else if (!strcmp(TA_VEHNAME_KEY, key)) {
            free(vehicleCfgName);
            vehicleCfgName = strdup(value);
        } else if (!strcmp(TA_DVLNAME_KEY, key)) {
            free(dvlCfgName);
            dvlCfgName = strdup(value);
        } else if (!strcmp(TA_RESONNAME_KEY, key)) {
            free(resonCfgName);
            resonCfgName = strdup(value);
        } else if (!strcmp(TA_TRNSVR_KEY, key)) {
            free(terrainNavServer);
            terrainNavServer = strdup(value);
        } else if (!strcmp(TA_LRAUVDVL_KEY, key)) {
            free(lrauvDvlName);
            lrauvDvlName = strdup(value);
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
            useIDTData = strcasecmp("false", value);
        } else if (!strcmp(TA_USEDVLSIDE_KEY, key)) {
            useDvlSide = strcasecmp("false", value);
        } else if (!strcmp(TA_USEMBTRNDATA_KEY, key)) {
            // Use the MbTrn.log file data when using either MbTrn data mode
            useMbTrnData = strcasecmp("false", value);
        } else if (!strcmp(TA_USEMBTRNSVR_KEY, key)) {
            useMbTrnData |= strcasecmp("false", value);
        } else {
            fprintf(stderr, "\n%s - ERR - Unknown key in cfg: %s\n", __func__, key);
        }
    }

    fclose(cfg);
    return 0;
}

// Return the next key and value pair in the cfg file.
// Function returns 0 if no key/value pair was found,
// otherwise 1.
int TrnAttr::getNextKeyValue(FILE *cfg, char key[], char value[])
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
            sscanf(line, "%s = %s", key, value);
            if ( (loc = strchr(value, ';')) ) *loc = '\0';  // Remove ';' from the value
            return 1;
        }
    }

    return 0;
}

void TrnAttr::tostream(std::ostream &os, int wkey, int wval){
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
}

std::string TrnAttr::tostring(int wkey, int wval)
{
    std::ostringstream ss;
    tostream(ss, wkey, wval);
    return ss.str();
}
