/* FILENAME      : TNavConfig.cpp
 * AUTHOR        : Henthorn
 * DATE          : 02/06/17
 * DESCRIPTION   : TNavConfig is designed to be a singleton in the::TNavConfig()
 {
   
 }

 *                 TerrainNav app. Clean method for components to access
 *                 configuration items. 
 * DEPENDENCIES  : TerrainNav.h
 * -----------------------------------------------------------------------------
 * Modification History
 * -----------------------------------------------------------------------------
 *    
 ******************************************************************************/

#include <string.h>
#include <stdlib.h>
#include "myOutput.h"
#include "trn_log.h"
#include "TNavConfig.h"


TNavConfig::TNavConfig()
:_vehicleSpecsFile(NULL), _particlesFile(NULL), _mapFile(NULL), _configPath(NULL), _logDir(NULL)
{

   _ignoreGps = 0;  // Pay heed unless told not to
}

TNavConfig::~TNavConfig()
{
    if (NULL!=_particlesFile){
        free(_particlesFile);
    }
    if (NULL!=_mapFile){
        free(_mapFile);
    }
    if (NULL!=_vehicleSpecsFile){
        free(_vehicleSpecsFile);
    }
    if (NULL!=_logDir){
        free(_logDir);
    }
    if (NULL!=_configPath){
        free(_configPath);
    }
    _mapFile=NULL;
    _particlesFile=NULL;
    _vehicleSpecsFile=NULL;
    _logDir=NULL;
}

void TNavConfig::setIgnoreGps(char flag)
{
  _ignoreGps = flag != 0;
}

char TNavConfig::getIgnoreGps()
{
  return _ignoreGps;
}

void TNavConfig::setMapFile(char *filename)
{
   if (filename)
   {
       if (NULL != _mapFile){
           free(_mapFile);
           _mapFile=NULL;
       }
      _mapFile = strdup(filename);
      logs(TL_OMASK(TL_TNAV_CONFIG, TL_LOG),"TNavConfig::setMapFile: value is now %s\n", _mapFile);
   }
   else
   {
      logs(TL_OMASK(TL_TNAV_CONFIG, TL_LOG),"TNavConfig::setMapFile: NULL passed in! Retaining value of %s\n",
         _mapFile? _mapFile : "NULL");
   }
}

char* TNavConfig::getMapFile()
{
    // caller owns return value - must free it
   char *cp = (_mapFile ? strdup(_mapFile) : NULL);
   return cp;
}

void TNavConfig::setVehicleSpecsFile(char *filename)
{
   if (NULL!=filename)
   {
       if (_vehicleSpecsFile){
           free(_vehicleSpecsFile);
           _vehicleSpecsFile=NULL;
       }
      _vehicleSpecsFile = strdup(filename);
      logs(TL_OMASK(TL_TNAV_CONFIG, TL_LOG),"TNavConfig::setVehicleSpecsFile: value is now %s\n",
         _vehicleSpecsFile);
   }
   else
   {
      logs(TL_OMASK(TL_TNAV_CONFIG, TL_LOG),"setVehicleSpecsFile: NULL passed in! Retaining value of %s\n",
         _vehicleSpecsFile? _vehicleSpecsFile : "NULL");
   }
}

char* TNavConfig::getVehicleSpecsFile()
{
    // caller owns return value - must free it
   char *cp = (_vehicleSpecsFile ? strdup(_vehicleSpecsFile) : NULL);
   return cp;
}

void TNavConfig::setParticlesFile(char *filename)
{
   if (filename)
   {
       if (NULL != _particlesFile){
           free(_particlesFile);
           _particlesFile=NULL;
       }
       
      _particlesFile = strdup(filename);
      logs(TL_OMASK(TL_TNAV_CONFIG, TL_LOG),"TNavConfig::setParticlesFile: value is now %s\n", _particlesFile);
   }
   else
   {
      logs(TL_OMASK(TL_TNAV_CONFIG, TL_LOG),"setParticlesFile: NULL passed in! Retaining value of %s\n",
         _particlesFile? _particlesFile : "NULL");
   }
}

char* TNavConfig::getParticlesFile()
{
// copy is needed b/c it can change dynamically
// caller owns return value - must free it
  char *cp = (_particlesFile ? strdup(_particlesFile) : NULL);
  return cp;
}

void TNavConfig::setConfigPath(char *filename)
{
    if (filename)
    {
        if (NULL != _configPath){
            free(_configPath);
            _configPath=NULL;
        }

        _configPath = strdup(filename);
        logs(TL_OMASK(TL_TNAV_CONFIG, TL_LOG),"TNavConfig::setConfigPath: value is now %s\n", _configPath);
    }
    else
    {
        logs(TL_OMASK(TL_TNAV_CONFIG, TL_LOG),"setConfigPath: NULL passed in! Retaining value of %s\n",
             (_configPath ? _configPath : "NULL"));
    }
}

char* TNavConfig::getConfigPath()
{
    // copy is needed b/c it can change dynamically
    // caller owns return value - must free it
    char *cp = (_configPath ? strdup(_configPath) : NULL);
    return cp;
}

void TNavConfig::setLogDir(char *filename)
{
   if (NULL!=filename)
   {
       if (NULL != _logDir){
           free(_logDir);
           _logDir=NULL;
       }
       
      _logDir = strdup(filename);
      logs(TL_OMASK(TL_TNAV_CONFIG, TL_LOG),"TNavConfig::setLogDir: value is now %s\n", _logDir);
   }
   else
   {
      logs(TL_OMASK(TL_TNAV_CONFIG, TL_LOG),"setLogDir: NULL passed in! Retaining value of %s\n",
         _logDir? _logDir : "NULL");
   }
}

char* TNavConfig::getLogDir()
{
    // caller owns return value - must free it
   char *cp = (_logDir ? strdup(_logDir) : NULL);
   return cp;
}
