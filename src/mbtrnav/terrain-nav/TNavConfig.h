/* FILENAME      : TNavConfig.h
 * AUTHOR        : Henthorn
 * DATE          : 02/06/17
 * DESCRIPTION   : TNavConfig is designed to be a singleton class in the
 *                 TerrainNav app. Clean method for components to access
 *                 configuration items.
 *                 TerrainNav creates and initializes the configuration items
 *                 in TNavConfig. Those items are then used throughout the app.
 * DEPENDENCIES  : TerrainNav.h
 * -----------------------------------------------------------------------------
 * Modification History
 * -----------------------------------------------------------------------------
 *    
 ******************************************************************************/

#ifndef _TNavConfig_h
#define _TNavConfig_h

class TerrainNav;
#include <stdlib.h>           // For atoi()

class TNavConfig
{
   //friend TerrainNav;

   //TNavConfig();

public:
    static void release(){
        TNavConfig::instance(true);
    }
   static TNavConfig* instance(bool release=false)
   {
       
      static TNavConfig *_instance;

       if (release) {
           // release the instance
           // used to release resources
           // and/or force new instance
           if (_instance) {
               delete _instance;
           }
           _instance=NULL;
       }else if (NULL==_instance){
           _instance = new TNavConfig();
       }
     return _instance;
   }

   TNavConfig();
   ~TNavConfig();

   char *getVehicleSpecsFile();
   char *getParticlesFile();
   char *getMapFile();
   char *getLogDir();
   void setMapFile(char *filename);


   void setParticlesFile(char *filename);
   void setVehicleSpecsFile(char *filename);
   void setLogDir(char *filename);
    
   void setIgnoreGps(char flag);
   char getIgnoreGps();

protected:
   char *_vehicleSpecsFile;
   char *_particlesFile;
   char *_mapFile;
   char *_logDir;

   char _ignoreGps;   // flag indicates whether to ignore gpsValid
};

#endif
