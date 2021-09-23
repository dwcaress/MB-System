/****************************************************************************/
/* Copyright (c) 2017 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  : Linux ed. Replay TRN using logged data from previous mission  */
/* Filename : Replay.cc                                                     */
/* Author   : Henthorn                                                      */
/* Project  : Iceberg AUV                                                   */
/* Version  : 1.0                                                           */
/* Created  : 07/03/2017                                                    */
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

#include "DataLogReader.h"
#include "MathP.h"
#include "NavUtils.h"
#include "TimeTag.h"
#include "FloatData.h"
#include "IntegerData.h"
#include "DataField.h"
#include "Replay.h"
#include "TRNUtils.h"
#include "TerrainNavClient.h"

#ifdef _LCMTRN
#include "TerrainNavLcmClient.h"
#endif

// Common to QNX and NIX versions
Replay::Replay(const char* loghome, const char *map, const char *host, int port)
  : logdir(0),
     lastTime(0.0),  nupdates(0L), nreinits(0),  trn_log(0),  dvl_log(0), nav_log(0), mbtrn_log(0), dvl_csv(0)
{
  logdir = strdup(loghome);

  try
  {
    trn_attr = new TRN_attr;
    if (0 != loadCfgAttributes())
    {
      fprintf(stderr, "\nreplay - Log directory %s not found\n\n", logdir);
    }
    if (0 != openLogFiles())
    {
      fprintf(stderr, "\nreplay - Failed to open log files in %s\n\n", logdir);
    }
  }
  catch (Exception e)
  {
    printf("\n];\n");
  }

  if (trn_attr->_useIDTData)
  {
    fprintf(stderr, "\nreplay - DeltaT data replay not implemented at the moment\n\n");
  }

  char par_format[] = "Particles   :";
  if (!strcmp("NotSpecified", trn_attr->_particlesName))
  {
      strcpy(trn_attr->_particlesName, "");
      strcpy(par_format, "");
  }

  // Use TRN config from the command line
  // for map, host, and port if they were provided.
  if (map)
  {
    if (trn_attr->_mapFileName) delete trn_attr->_mapFileName;
    trn_attr->_mapFileName = strdup(map);
  }

  if (host)
  {
    trn_attr->_terrainNavServer = strdup(host);
    trn_attr->_terrainNavPort = port;
  }

  fprintf(stderr, "\n"
                "Server      : %s  %d\n"
                "Vehicle Cfg : %s\n"
                "Map File    : %s Type %ld\n"
                "%s %s\n\n",
                host, port, trn_attr->_vehicleCfgName, trn_attr->_mapFileName,
                trn_attr->_map_type, par_format, trn_attr->_particlesName);

}

Replay::~Replay()
{
  if (trn_attr) delete trn_attr;
  if (logdir)   delete logdir;
  if (trn_log)  delete(trn_log);
  if (dvl_log)  delete(dvl_log);
  if (nav_log)  delete(nav_log);
  if (mbtrn_log)  delete(mbtrn_log);
}

/*
** Take the standard 2-norm. This one returns the answer, since it is a scalar.
*/
static double Vnorm( double v[] )
{
   double Vnorm2 = 0.;
   int i;
   for(i=0; i<REPLAY_VNORM_DIM; i++) Vnorm2 += pow(v[i],2.);
   return( sqrt( Vnorm2 ) );
}

#if WITH_REPLAY_DEGTORAD
static double degToRad(double deg)
{
  double const RadsPerDeg = M_PI  / 180.0;
  return deg*RadsPerDeg;
}
#endif

// Call the appropriate function based on which instruments were used
// for TRN, determined by flags set in the terrainAid.cfg file and
// loaded here in loadCfgAttributes().
int Replay::getNextRecordSet(poseT *pt, measT *mt)
{
  nupdates++;

  if (trn_attr->_lrauvDvlFilename)
  {
    // Read all poseT and measT data from the LRAUV dvl CSV file
    return getLRAUVDvlRecordSet(pt, mt);
  }
  else if (trn_attr->_useMbTrnData)
  {
    // Read all poseT and measT data from the MbTrn.log
    return getMbTrnRecordSet(pt, mt);
  }


  // Get the data from the other log files
  DataField *f=NULL;

  try
  {
    // Read a TRN record. TRN logs every 3 seconds, or 0.33 HZ
    trn_log->read();
    pt->time = trn_log->timeTag()->value();

    // Get [x,y,z], [phi,theta,psi], [wx,wy,wz], [vx,vy,vz],
    // and flags from the TRN record
    // navN, navN, depth
    trn_log->fields.get( 3,&f); pt->x = atof(f->ascii());
    trn_log->fields.get( 4,&f); pt->y = atof(f->ascii());
    trn_log->fields.get( 5,&f); pt->z = atof(f->ascii());

    // phi, theta, psi (roll, pitch, yaw)
    trn_log->fields.get( 6,&f); pt->phi   = atof(f->ascii());
    trn_log->fields.get( 7,&f); pt->theta = atof(f->ascii());
    trn_log->fields.get( 8,&f); pt->psi   = atof(f->ascii());

    // wx, wy, wz, (omega1, 2, 3)
    trn_log->fields.get( 9,&f); pt->wx = atof(f->ascii());
    trn_log->fields.get(10,&f); pt->wy = atof(f->ascii());
    trn_log->fields.get(11,&f); pt->wz = atof(f->ascii());

    // dvlVelocity 1, 2, 3 (bottomTrackVelocity or waterMassVelocity)
    trn_log->fields.get(17,&f); pt->vx = atof(f->ascii());
    trn_log->fields.get(18,&f); pt->vy = atof(f->ascii());
    trn_log->fields.get(19,&f); pt->vz = atof(f->ascii());

    // dvlValid, gpsValid, bottomlock flags
    trn_log->fields.get(20,&f); pt->dvlValid   = atoi(f->ascii());
    trn_log->fields.get(21,&f); pt->gpsValid   = atoi(f->ascii());
    trn_log->fields.get(22,&f); pt->bottomLock = atoi(f->ascii());

    mt->time = pt->time;

    {
      trn_log->fields.get(13,&f); mt->ranges[0] = atof(f->ascii());
      trn_log->fields.get(14,&f); mt->ranges[1] = atof(f->ascii());
      trn_log->fields.get(15,&f); mt->ranges[2] = atof(f->ascii());
      trn_log->fields.get(16,&f); mt->ranges[3] = atof(f->ascii());
      mt->measStatus[0] = mt->measStatus[1] = mt->measStatus[2]
                        = mt->measStatus[3] = true;


      mt->x     = pt->x;
      mt->y     = pt->y;
      mt->z     = pt->z;
      mt->phi   = pt->phi;
      mt->theta = pt->theta;
      mt->psi   = pt->psi;

      // Collect the remaining measT elements nav record
      double nav_time = 0.;
      do
      {
        nav_log->read();
        nav_time = nav_log->timeTag()->value();
      }
      while( (fabs(nav_time - pt->time) > NAV4TRN) && (nav_time < pt->time) );

      //fprintf(stderr, "Found matching nav record at time %f\n", nav_time);
      nav_log->fields.get(7,&f); mt->phi   = atof(f->ascii());
      nav_log->fields.get(8,&f); mt->theta = atof(f->ascii());
      nav_log->fields.get(9,&f); mt->psi   = atof(f->ascii());

    }

    if (trn_attr->_useIDTData) mt->dataType = TRN_SENSOR_DELTAT;
    else if (trn_attr->_useMbTrnData) mt->dataType = TRN_SENSOR_MB;
    else mt->dataType = TRN_SENSOR_DVL;

    return 1;
    //fprintf(stdout, "\n");
  }
  catch (...) {
    fprintf(stderr, "\nEnd of log!\n");
    return 0;
  }

  return 1;
}

#if 0
  else
  {
    printf("Error - Unsupported Replay instrument spec in terrainAid.cfg\n"
           "  Dvl      data: %d  (unsupported)\n"
           "  Dvl Side data: %d  (unsupported)\n"
           "  MbTrn    data: %d  (supported)\n"
           "  IDT      data: %d  (unsupported)\n",
           (!trn_attr->_useDvlSide &&
            !trn_attr->_useMbTrnData &&
            !trn_attr->_useIDTData),
           trn_attr->_useDvlSide,
           trn_attr->_useMbTrnData,
           trn_attr->_useIDTData);
    return 0;
  }
#endif

// This function returns the next DVL record set in pt and mt.
// Read from the CSV file specified in the config.
// Function returns 1 when a record was retrieved successfully
// and 0 when unsuccessful (usually means EOF was reached).
int Replay::getLRAUVDvlRecordSet(poseT *pt, measT *mt)
{
  // Requires an open CSV file.
  if (!dvl_csv)
  {
    fprintf(stderr, "\n\tReplay - No dvl data file: %s\n\n", trn_attr->_lrauvDvlFilename);
    return 0;
  }

  // Read next line into buffer. Retirn 0 upon EOF
  char csvbuf[3000];
  if (!fgets((char*)csvbuf, sizeof(csvbuf), dvl_csv))
    return 0;
  else
    return parseDvlCsvLine(csvbuf, pt, mt);
}

// This function returns the next MbTrn record set in pt and mt.
// Function returns 1 when a record was retrieved successfully
// and 0 when unsuccessful (usually means EOF was reached).
int Replay::getMbTrnRecordSet(poseT *pt, measT *mt)
{
    DataField *f=NULL;

  try
  {
    // Read a TRN record. TRN logs every 3 seconds, or 0.33 HZ
    mbtrn_log->read();
    double lat, lon;
    mbtrn_log->fields.get( 1,&f); pt->time = atof(f->ascii());
    mbtrn_log->fields.get( 2,&f); lat = atof(f->ascii());
    mbtrn_log->fields.get( 3,&f); lon = atof(f->ascii());

    NavUtils::geoToUtm(Math::degToRad(lat),
                       Math::degToRad(lon),
                       NavUtils::geoToUtmZone(Math::degToRad(lat),Math::degToRad(lon)),
                       &pt->x, &pt->y);

    mbtrn_log->fields.get( 4,&f); pt->z = atof(f->ascii());
    mbtrn_log->fields.get( 5,&f); pt->psi = atof(f->ascii());
    pt->phi = pt->theta = 0.;
    pt->dvlValid = True;
    pt->gpsValid = pt->z < 2;    // Depths below 2 m  have no GPS
    pt->bottomLock = 1;
    pt->wx = -3.332e-002;
    pt->wy = -9.155e-003;
    pt->wz = -3.076e-002;
    pt->vx = pt->vy = pt->vz = 0.01;

    mt->time = pt->time;
    mt->dataType = 2;
    mt->x = pt->x;
    mt->y = pt->y;
    mt->z = pt->z;
    mbtrn_log->fields.get( 6,&f); mt->ping_number = atof(f->ascii());
    mbtrn_log->fields.get( 7,&f); mt->numMeas = atoi(f->ascii());

    mt->ranges      = new double[mt->numMeas];
    mt->alongTrack  = new double[mt->numMeas];
    mt->crossTrack  = new double[mt->numMeas];
    mt->altitudes   = new double[mt->numMeas];
    mt->alphas      = new double[mt->numMeas];
    mt->beamNums    = new int[mt->numMeas];
    mt->measStatus  = new bool[mt->numMeas];

    for (int i = 0; i < mt->numMeas; i++)
    {
      mbtrn_log->fields.get(8+((i*4)+0),&f); mt->beamNums[i] = atoi(f->ascii());
      mbtrn_log->fields.get(8+((i*4)+1),&f); mt->alongTrack[i] = atof(f->ascii());
      mbtrn_log->fields.get(8+((i*4)+2),&f); mt->crossTrack[i] = atof(f->ascii());
      mbtrn_log->fields.get(8+((i*4)+3),&f); mt->altitudes[i] = atof(f->ascii());
      double rho[3] = {mt->alongTrack[i], mt->crossTrack[i], mt->altitudes[i]};
      double rhoNorm = Vnorm( rho );
      mt->ranges[i] = rhoNorm;
      if (rhoNorm > 1) mt->measStatus[i] = True;
      else mt->measStatus[i] = False;
    }

    return 1;

  }
  catch (...) {
    fprintf(stderr, "\nEnd of log!\n");
    return 0;
  }

  fprintf(stderr, "TRN EOF\n");
  return 0;
}

int Replay::openLogFiles()
{
  char logfile[1000];
  fprintf(stdout, "Replay - Loading log files in %s...\n", logdir);

  // Open the "special cases" if needed
  if (trn_attr->_lrauvDvlFilename)
  {
    // Open the dvl CSV file and prep for reading
    sprintf(logfile, "%s/%s", logdir, trn_attr->_lrauvDvlFilename);
    fprintf(stdout, "replay - Loading CSV file %s...\n", logfile);
    dvl_csv = fopen(logfile, "r");
    if (dvl_csv)
      return 0;
    else
      return 1;
  }
  else if (trn_attr->_useMbTrnData)
  {
    // Open the MbTrn file and prep for reading
    sprintf(logfile, "%s/MbTrn.log", logdir);
    fprintf(stdout, "replay - Loading MbTrn.log file %s...\n", logfile);
    mbtrn_log = new DataLogReader(logfile);
    return 0;
  }

  // Default log files
  sprintf(logfile, "%s/TerrainAid.log", logdir);
  fprintf(stdout, "Replay - Opening %s...\n", logfile);
  trn_log = new DataLogReader(logfile);

  if (trn_attr->_useDvlSide)
    sprintf(logfile, "%s/dvlSide.log", logdir);
  else
    sprintf(logfile, "%s/navigation.log", logdir);
  fprintf(stdout, "Replay - Opening %s...\n", logfile);
  nav_log = new DataLogReader(logfile);

  return 0;
}

// On Linux and Cygwin, it is possible to simply use a native TerrainNav
// object for TRN calcs rather than go through a client/server connection.
// If the user enters "native" as the argument in the -h option,
// return false.
// On QNX, always use the trn_server for TRN
Boolean Replay::useTRNServer()
{
#ifdef _QNX
  return True;
#else
  return strcmp(trn_attr->_terrainNavServer, "native");
#endif
}

Boolean Replay::useLcmTrn()
{
#ifndef _LCMTRN
  fprintf(stderr, "Replay - trn_replay was not build for LCMTRN!\n");
  return False;
#endif

#ifdef _QNX
  return True;
#else
  return !strcmp(trn_attr->_terrainNavServer, LCM_HOST);
#endif
}


/****************************************************************************/



// Could be common to QNX and NIX versions.
// Just use this function in the QNX version
// since it works OK.
int Replay::loadCfgAttributes()
{
  char cfgfile[300];
  sprintf(cfgfile, "%s/terrainAid.cfg", logdir);
  if (access(cfgfile, F_OK) < 0)
  {
    fprintf(stderr, "replay - Could not find %s", cfgfile);
    return 1;
  }

  FILE *cfg = fopen(cfgfile, "r");
  if (!cfg)
  {
    fprintf(stderr, "replay - Could not open %s", cfgfile);
    return 1;
  }

  // Initialize to default values
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
  trn_attr->_useMbTrnData   = false;

  // Get   key = value   pairs from non-comment lines in the cfg.
  // If the key matches a known config item, extract and save the value.
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

    // Use the MbTrn.log file data when using either MbTrn data mode
    else if (!strcmp("useMbTrnData",         key))  trn_attr->_useMbTrnData = strcasecmp("false", value);
    else if (!strcmp("useMbTrnServer",       key))  trn_attr->_useMbTrnData |= strcasecmp("false", value);
    else
      fprintf(stderr, "\n\tReplay: Unknown key in cfg: %s\n\n", key);
  }

  return 0;
}

// Return the next key and value pair in the cfg file.
// Function returns 0 if no key/value pair was found,
// otherwise 1.
int Replay::getNextKeyValue(FILE *cfg, char key[], char value[])
{
  // Continue reading from cfg skipping comment lines
  //
  char line[300];
  while (fgets(line, sizeof(line), cfg))
  {
    // Chop off leading whitespace, then look for '//'
    size_t i = 0;
    while(line[i] == ' ' || line[i] == '\t' || line[i] == '\n' || line[i] == '\r'
      || line[i] == '\f' || line[i] == '\v') i++;

    if (strncmp("//", line+i, 2))
    {
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


// Common to QNX and NIX versions
TerrainNav* Replay::connectTRN()
{
  TerrainNav *_tercom = 0;

  fprintf(stdout, "replay - Using TerrainNav at %s on port %ld\n",
    trn_attr->_terrainNavServer, trn_attr->_terrainNavPort);
  fprintf(stdout, "replay - Using TerrainNav with map %s and config %s\n",
   trn_attr->_mapFileName, trn_attr->_vehicleCfgName);

  try
  {
    char buf[REPLAY_PATHNAME_LENGTH];
    char *mapdir  = getenv("TRN_MAPFILES");
    char *datadir = getenv("TRN_DATAFILES");
    sprintf(buf, "%s/%s", mapdir, trn_attr->_mapFileName);
    free(trn_attr->_mapFileName); trn_attr->_mapFileName = strdup(buf);
    sprintf(buf, "%s/%s", datadir, trn_attr->_vehicleCfgName);
    free(trn_attr->_vehicleCfgName); trn_attr->_vehicleCfgName = strdup(buf);
    sprintf(buf, "%s/%s", datadir, trn_attr->_particlesName);
    free(trn_attr->_particlesName); trn_attr->_particlesName = strdup(buf);
    fprintf(stdout, "%s\n%s\n%s\n", trn_attr->_mapFileName, trn_attr->_vehicleCfgName, trn_attr->_particlesName);

    if (useLcmTrn())
    {
#ifdef _LCMTRN
      printf("Connecting to %s ...\n", trn_attr->_terrainNavServer);
      _tercom = new TerrainNavLcmClient();
#endif
    }
    else if (useTRNServer())
    {
      printf("Connecting to %s in 2...\n", trn_attr->_terrainNavServer);
      sleep(1);
      _tercom = new TerrainNavClient(trn_attr->_terrainNavServer,
                                     trn_attr->_terrainNavPort,
                                     trn_attr->_mapFileName,
                                     trn_attr->_vehicleCfgName,
                                     trn_attr->_particlesName,
                                     TRNUtils::basename(logdir),
                                     trn_attr->_filter_type, trn_attr->_map_type);
    }
    else
    {
      // On macOS, linux, and cygwin, we can use a native TerrainNav object
      // instead of relying on a trn_server. Call useTRNServer() to decide.
      _tercom = new TerrainNav(trn_attr->_mapFileName,
                               trn_attr->_vehicleCfgName,
                               trn_attr->_particlesName,
                               trn_attr->_filter_type,
                               trn_attr->_map_type,
                               TRNUtils::basename(logdir));
    }
  }
  catch (Exception e)
  {
    fprintf(stderr, "replay - Failed TRN connection. Check TRN error messages...\n");
    _tercom = 0;
  }

  if (_tercom)
  {
    // After moving is_connected() to public section, we can use this code block
    if (_tercom &&  (!_tercom->is_connected() || !_tercom->initialized()))
    {
      fprintf(stderr, "replay -:Not initialized. See trn_server error messages...\n");
      //return -1;
    }

    // If we reach here then we've connected
    fprintf(stdout, "replay -:Should be Connected to server if no error messages...\n");

    fprintf(stdout, "replay -:Lets just set the interpret measure attitude flag to true...\n");

    // The following calls to setup TRN lifted from TerrainAidDriver
    _tercom->setInterpMeasAttitude(true);

    //choose filter settings based on whether kearfott is available and if
    //filter forcing is set
    if(trn_attr->_forceLowGradeFilter)
      _tercom->useLowGradeFilter();
    else
      _tercom->useHighGradeFilter();

    //turn on filter reintialization if set in terrainAid.cfg
    _tercom->setFilterReinit(trn_attr->_allowFilterReinits);

    //turn on modified weighting if set in terrainAid.cfg
    _tercom->setModifiedWeighting(trn_attr->_useModifiedWeighting);
  }

  return _tercom;

}

#define DVL_SAMPLE_PERIOD 3.0     // 3 seconds

int Replay::parseDvlCsvLine(const char *line, poseT *pt, measT *mt)
{
  char *buf = strdup(line);

  mt->dataType = TRN_SENSOR_DVL;
  mt->numMeas  = 0;

  // Get position data
  for (int c = 0; c < DVL_RANGES; c++)
  {
    char *token = strtok((char*)buf, ",");
    buf = NULL;

    if (NULL == token)
    {
      fprintf(stderr, "Replay - unexpected EOL parsing line %ld\n", nupdates);
      return 0;
    }
    //printf("  %s\n", token);

    if (DVL_TIME == c)
    {
      pt->time = atof(token);

      // Ensure that measurements are at least 3 seconds apart
      if (pt->time < lastTime + DVL_SAMPLE_PERIOD)
        return -1;

      lastTime = pt->time;
    }
    else if (DVL_NORTH == c)  pt->x     = atof(token);
    else if (DVL_EAST == c)   pt->y     = atof(token);
    else if (DVL_DEPTH == c)  pt->z     = atof(token);
    else if (DVL_PSI == c)    pt->psi   = atof(token);
    else if (DVL_THETA == c)  pt->theta = atof(token);
    else if (DVL_PHI == c)    pt->phi   = atof(token);
    else if (DVL_WX == c)     pt->wx    = atof(token);
    else if (DVL_WY == c)     pt->wy    = atof(token);
    else if (DVL_WZ == c)     pt->wz    = atof(token);
    else if (DVL_VX == c)     pt->vx    = atof(token);
    else if (DVL_VY == c)     pt->vy    = atof(token);
    else if (DVL_VZ == c)     pt->vz    = atof(token);
    else if (DVL_VALID == c)  pt->dvlValid   = atoi(token);
    else if (DVL_LOCK == c)   pt->bottomLock = atoi(token);
    else if (DVL_NBEAMS == c) mt->numMeas    = atoi(token);
  }
  //printf("  yaw = %.6f\n", pt->phi); // sleep(1);

  // Now get measure data == beam data
  mt->time = pt->time;
  mt->covariance = (double*)realloc(mt->covariance, N_COVAR*sizeof(double));
  mt->ranges     = (double*)realloc(mt->ranges,     mt->numMeas*sizeof(double));
  mt->crossTrack = (double*)realloc(mt->crossTrack, mt->numMeas*sizeof(double));
  mt->alongTrack = (double*)realloc(mt->alongTrack, mt->numMeas*sizeof(double));
  mt->altitudes  = (double*)realloc(mt->altitudes,  mt->numMeas*sizeof(double));
  mt->alphas     = (double*)realloc(mt->alphas,     mt->numMeas*sizeof(double));
  mt->measStatus = (bool*)  realloc(mt->measStatus, mt->numMeas*sizeof(bool));

  // There are 3 data items per beam, 3 * numMeas
  for (int b = 0; b < mt->numMeas * 3; b++)
  {
    char *token = strtok((char*)buf, ",");
    buf = NULL;

    if (NULL == token)
    {
      fprintf(stderr, "Replay - unexpected EOL parsing line %ld\n", nupdates);
      return 0;
    }
    //printf("  %d = %s\n", b/3, token);

    // these three go with beam number (b/3)
    if (0 == b%3)      // skip beam number
      ;
    else if (1 == b%3) // measStatus
      mt->measStatus[b/3] = atoi(token);
    else if (2 == b%3) // range
      mt->ranges[b/3] = atof(token);

  }

  // There shouldn't be any more tokens. Let's check...
  if (strtok((char*)buf, ","))
  {
    fprintf(stderr, "Replay - unexpected tokens at the end of line %ld\n",
      nupdates);
  }

  return 1;
}
