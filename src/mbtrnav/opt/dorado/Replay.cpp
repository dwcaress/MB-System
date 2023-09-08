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
/* Modified : 04/26/2022 RGH Use CSV files for devices besides DVL, clean.  */
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

// Measurement data constants used to index into CSV file records
#define MEAS_STATUS     1
#define MEAS_RANGE      2
#define MEAS_ALONGTRACK 3
#define MEAS_CROSSTRACK 4
#define MEAS_ALTITUDE   5
#define BUF_512 512
#define BUF_1024 1024

static char csvbuf[50000];

/****************************************************************************/

// Seconds within which a DVL record matches TRN record
const double Replay::DVL4TRN = 0.4;
// Seconds within which a nav record matches TRN record
const double Replay::NAV4TRN = 0.2;
// array depth for VNORM
const uint16_t Replay::REPLAY_VNORM_DIM = 3;

// Common to QNX and NIX versions
Replay::Replay(const char* loghome, const char *map, const char *host, int port)
  : logdir(0),
     lastTime(0.0),  nupdates(0L), nreinits(0),  trn_log(0),  dvl_log(0),
     nav_log(0), mbtrn_log(0), tnav_log(0), dvl_csv(0)
{
  logdir = strdup(loghome);

  try {
    trn_attr = new TRN_attr;
      trn_attr->_mapFileName = NULL;
      trn_attr->_particlesName = NULL;
      trn_attr->_vehicleCfgName = NULL;
      trn_attr->_dvlCfgName = NULL;
      trn_attr->_resonCfgName = NULL;
      trn_attr->_terrainNavServer = NULL;
      trn_attr->_lrauvDvlFilename = NULL;

    if (0 != loadCfgAttributes()) {
      fprintf(stderr, "\nreplay - Log directory %s not found\n\n", logdir);
    }
    if (0 != openLogFiles()) {
      fprintf(stderr, "\nreplay - Failed to open log files in %s\n\n", logdir);
    }
  }
  catch (Exception e) {
    printf("\n];\n");
  }

  if (trn_attr->_useIDTData) {
    fprintf(stderr, "\nreplay - DeltaT data replay not implemented at the moment\n\n");
  }

  // Use TRN config from the command line
  // for map, host, and port if they were provided.
  if (map) {
    if (trn_attr->_mapFileName) free(trn_attr->_mapFileName);
    trn_attr->_mapFileName = strdup(map);
  }

  if (host) {
    free(trn_attr->_terrainNavServer);
    trn_attr->_terrainNavServer = strdup(host);
    trn_attr->_terrainNavPort = port;
  }

  fprintf(stderr, "\n"
          "Server      : %s  %d\n"
          "Vehicle Cfg : %s\n"
          "Map File    : %s Type %ld\n"
          "Particles   : %s\n\n",
          host, port,
          trn_attr->_vehicleCfgName,
          trn_attr->_mapFileName, trn_attr->_map_type,
          trn_attr->_particlesName);
}

Replay::~Replay()
{

    fclose(dvl_csv);
    free(logdir);
    delete(trn_log);
    delete(dvl_log);
    delete(nav_log);
    delete(mbtrn_log);
    free(trn_attr->_mapFileName);
    free(trn_attr->_particlesName);
    free(trn_attr->_vehicleCfgName);
    free(trn_attr->_dvlCfgName);
    free(trn_attr->_resonCfgName);
    free(trn_attr->_terrainNavServer);
    free(trn_attr->_lrauvDvlFilename);
    delete trn_attr;
    delete trn_log;
    delete dvl_log;
    delete nav_log;
    delete mbtrn_log;
}

/*
** Take the standard 2-norm. This one returns the answer, since it is a scalar.
*/
static double Vnorm( double v[] )
{
   double Vnorm2 = 0.;
   int i;
   for(i=0; i < Replay::REPLAY_VNORM_DIM; i++) Vnorm2 += pow(v[i],2.);
   return( sqrt( Vnorm2 ) );
}

#if WITH_REPLAY_DEGTORAD
static double degToRad(double deg)
{
  double const RadsPerDeg = M_PI  / 180.0;
  return deg*RadsPerDeg;
}
#endif

int Replay::getNextTRNRecordSet(poseT *pt, measT *mt)
{
  // Get the data from the other log files
  DataField *f=NULL;

  try {
    // Read a TRN log record.
    tnav_log->read();
    tnav_log->fields.get( 1,&f); pt->time = atof(f->ascii());

    // Get [x,y,z], [phi,theta,psi], [wx,wy,wz], [vx,vy,vz],
    // and flags from the TRN record
    // navN, navN, depth
    tnav_log->fields.get( 2,&f); pt->x = atof(f->ascii());
    tnav_log->fields.get( 3,&f); pt->y = atof(f->ascii());
    tnav_log->fields.get( 4,&f); pt->z = atof(f->ascii());

    tnav_log->fields.get( 5,&f); pt->vx = atof(f->ascii());
    tnav_log->fields.get( 6,&f); pt->vy = atof(f->ascii());
    tnav_log->fields.get( 7,&f); pt->vz = atof(f->ascii());

    // phi, theta, psi (roll, pitch, yaw)
    tnav_log->fields.get( 8,&f); pt->phi   = atof(f->ascii());
    tnav_log->fields.get( 9,&f); pt->theta = atof(f->ascii());
    tnav_log->fields.get(10,&f); pt->psi   = atof(f->ascii());

    // wx, wy, wz, (omega1, 2, 3)
    pt->wx = pt->wy = pt->wz = 0.;

    // dvlValid, gpsValid, bottomlock flags
    pt->dvlValid   = 1;

    if (pt->z > 0.3) {
      pt->gpsValid = false;
    } else {
      pt->gpsValid = true;
    }

    pt->bottomLock = !pt->gpsValid;

    tnav_log->fields.get(11,&f); mt->time = atof(f->ascii());
    tnav_log->fields.get(12,&f); mt->dataType = atoi(f->ascii());
    tnav_log->fields.get(14,&f); int nb = atoi(f->ascii());

    for (int i = 0; i < nb; i++) {
      tnav_log->fields.get( 16+i,&f); mt->ranges[i] = atof(f->ascii());
      tnav_log->fields.get(380+i,&f); mt->measStatus[i] = atoi(f->ascii());
    }

    mt->x     = pt->x;
    mt->y     = pt->y;
    mt->z     = pt->z;
    mt->phi   = pt->phi;
    mt->theta = pt->theta;
    mt->psi   = pt->psi;
  }
  catch (...) {
    fprintf(stderr, "\nEnd of log!\n");
    return 0;
  }

  return 1;
}

// Call the appropriate function based on which instruments were used
// for TRN, determined by flags set in the terrainAid.cfg file and
// loaded here in loadCfgAttributes().
int Replay::getNextRecordSet(poseT *pt, measT *mt)
{
  nupdates++;

  // Read all poseT and measT data from a CSV file
  if (trn_attr->_lrauvDvlFilename) {
    return getLRAUVDvlRecordSet(pt, mt);

  // Read all poseT and measT data from the MbTrn.log
  } else if (trn_attr->_useMbTrnData) {
    return getMbTrnRecordSet(pt, mt);

  // Read all poseT and measT data from the TerrainNav.log
  } else if (tnav_log) {
    return getNextTRNRecordSet(pt, mt);
  }



  // Get the poseT and measT data from the standard Dorado log file set
  DataField *f=NULL;

  try {
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
      do {
        nav_log->read();
        nav_time = nav_log->timeTag()->value();
      }
      while( (fabs(nav_time - pt->time) > Replay::NAV4TRN) &&
             (nav_time < pt->time) );

      //fprintf(stderr, "Found matching nav record at time %f\n", nav_time);
      nav_log->fields.get(7,&f); mt->phi   = atof(f->ascii());
      nav_log->fields.get(8,&f); mt->theta = atof(f->ascii());
      nav_log->fields.get(9,&f); mt->psi   = atof(f->ascii());

    }

    if (trn_attr->_useIDTData) {
      mt->dataType = TRN_SENSOR_DELTAT;
    } else if (trn_attr->_useMbTrnData) {
      mt->dataType = TRN_SENSOR_MB;
    } else {
      mt->dataType = TRN_SENSOR_DVL;
    }

    return 1;
  }
  catch (...) {
    fprintf(stderr, "\nEnd of log!\n");
    return 0;
  }

  return 1;
}

// This function returns the next DVL record set in pt and mt.
// Read from the CSV file specified in the config.
// Function returns 1 when a record was retrieved successfully
// and 0 when unsuccessful (usually means EOF was reached).
int Replay::getLRAUVDvlRecordSet(poseT *pt, measT *mt)
{
  // Requires an open CSV file.
  if (!dvl_csv) {
    fprintf(stderr, "\n\tReplay - No dvl data file: %s\n\n", trn_attr->_lrauvDvlFilename);
    return 0;
  }

  // Read next line into buffer. Retirn 0 upon EOF
  csvbuf[0] = '\0';
  if (!fgets((char*)csvbuf, sizeof(csvbuf), dvl_csv)) {
    return 0;
  } else {
    return parseDvlCsvLine(csvbuf, pt, mt);
  }
}

// This function returns the next MbTrn record set in pt and mt.
// Function returns 1 when a record was retrieved successfully
// and 0 when unsuccessful (usually means EOF was reached).
int Replay::getMbTrnRecordSet(poseT *pt, measT *mt)
{
  DataField *f=NULL;

  try {
    // Read a TRN record. TRN logs every 3 seconds, or 0.33 HZ
    mbtrn_log->read();
    double lat, lon;
    mbtrn_log->fields.get( 1,&f); pt->time = atof(f->ascii());
    mbtrn_log->fields.get( 2,&f); lat = atof(f->ascii());
    mbtrn_log->fields.get( 3,&f); lon = atof(f->ascii());

    NavUtils::geoToUtm(Math::degToRad(lat), Math::degToRad(lon),
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

    for (int i = 0; i < mt->numMeas; i++) {
      mbtrn_log->fields.get(8+((i*4)+0),&f); mt->beamNums[i] = atoi(f->ascii());
      mbtrn_log->fields.get(8+((i*4)+1),&f); mt->alongTrack[i] = atof(f->ascii());
      mbtrn_log->fields.get(8+((i*4)+2),&f); mt->crossTrack[i] = atof(f->ascii());
      mbtrn_log->fields.get(8+((i*4)+3),&f); mt->altitudes[i] = atof(f->ascii());
      double rho[3] = {mt->alongTrack[i], mt->crossTrack[i], mt->altitudes[i]};
      double rhoNorm = Vnorm( rho );
      mt->ranges[i] = rhoNorm;
      if (rhoNorm > 1) {
        mt->measStatus[i] = True;
      } else {
        mt->measStatus[i] = False;
      }
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

  char logfile[BUF_1024];
  fprintf(stdout, "Replay - Loading log files in %s...\n", logdir);

  // Open the "special cases" if needed
  if (trn_attr->_lrauvDvlFilename) {
    // Open the dvl CSV file and prep for reading
    snprintf(logfile, BUF_1024, "%s/%s", logdir, trn_attr->_lrauvDvlFilename);
    fprintf(stdout, "replay - Loading CSV file %s...\n", logfile);
    dvl_csv = fopen(logfile, "r");
    if (dvl_csv) {
      return 0;
    } else {
      return 1;
    }
  } else if (trn_attr->_useMbTrnData) {
    // Open the MbTrn file and prep for reading
    snprintf(logfile, BUF_1024, "%s/MbTrn.log", logdir);
    fprintf(stdout, "replay - Loading MbTrn.log file %s...\n", logfile);
    mbtrn_log = new DataLogReader(logfile);
    return 0;
  }

  snprintf(logfile, BUF_1024, "%s/TerrainNav.log", logdir);
  if (access(logfile, R_OK) == 0) {
    // TerrainNav log files
    fprintf(stdout, "Replay - Opening %s...\n", logfile);
    tnav_log = new DataLogReader(logfile);
  } else {
    // Default log files
    snprintf(logfile, BUF_1024, "%s/TerrainAid.log", logdir);
    fprintf(stdout, "Replay - Opening %s...\n", logfile);
    trn_log = new DataLogReader(logfile);

    if (trn_attr->_useDvlSide) {
      snprintf(logfile, BUF_1024, "%s/dvlSide.log", logdir);
    } else {
      snprintf(logfile, BUF_1024, "%s/navigation.log", logdir);
    }
    fprintf(stdout, "Replay - Opening %s...\n", logfile);
    nav_log = new DataLogReader(logfile);
  }

  return 0;
}

// On Linux and Cygwin, it is possible to simply use a native TerrainNav
// object for TRN calcs rather than go through a client/server connection.
// If the user enters "native" as the argument in the -h option,
// return false.
// On QNX, always use the trn_server for TRN
bool Replay::useTRNServer()
{
#ifdef _QNX
  return True;
#else
  return strcmp(trn_attr->_terrainNavServer, "native");
#endif
}


/****************************************************************************/



// Could be common to QNX and NIX versions.
// Just use this function in the QNX version
// since it works OK.
int Replay::loadCfgAttributes()
{
  char cfgfile[BUF_512];
  snprintf(cfgfile, BUF_512, "%s/terrainAid.cfg", logdir);
  if (access(cfgfile, F_OK) < 0) {
    fprintf(stderr, "replay - Could not find %s", cfgfile);
    return 1;
  }

  FILE *cfg = fopen(cfgfile, "r");
  if (!cfg) {
    fprintf(stderr, "replay - Could not open %s", cfgfile);
    return 1;
  }

  // Initialize to default values
  free(trn_attr->_mapFileName);
  free(trn_attr->_particlesName);
  free(trn_attr->_vehicleCfgName);
  free(trn_attr->_dvlCfgName);
  free(trn_attr->_resonCfgName);
  free(trn_attr->_terrainNavServer);
  free(trn_attr->_lrauvDvlFilename);
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
  while (getNextKeyValue(cfg, key, value)) {
    if      (!strcmp("mapFileName",          key)) {
      free(trn_attr->_mapFileName);
      trn_attr->_mapFileName = strdup(value);
    } else if (!strcmp("particlesName",        key)) {
      free(trn_attr->_particlesName);
      trn_attr->_particlesName = strdup(value);
    } else if (!strcmp("vehicleCfgName",       key)) {
      free(trn_attr->_vehicleCfgName);
      trn_attr->_vehicleCfgName = strdup(value);
    } else if (!strcmp("dvlCfgName",           key)) {
      free(trn_attr->_dvlCfgName);
      trn_attr->_dvlCfgName = strdup(value);
    } else if (!strcmp("resonCfgName",         key)) {
      free(trn_attr->_resonCfgName);
      trn_attr->_resonCfgName = strdup(value);
    } else if (!strcmp("terrainNavServer",     key)) {
      free(trn_attr->_terrainNavServer);
      trn_attr->_terrainNavServer = strdup(value);
    } else if (!strcmp("lrauvDvlFilename",     key)) {
      free(trn_attr->_lrauvDvlFilename);
      trn_attr->_lrauvDvlFilename = strdup(value);
    } else if (!strcmp("map_type",             key))  {
      trn_attr->_map_type = atoi(value);
    } else if (!strcmp("filterType",           key)) {
      trn_attr->_filter_type = atoi(value);
    } else if (!strcmp("terrainNavPort",       key)) {
      trn_attr->_terrainNavPort = atol(value);
    } else if (!strcmp("forceLowGradeFilter",  key)) {
      trn_attr->_forceLowGradeFilter = strcasecmp("false", value);
    } else if (!strcmp("allowFilterReinits",   key)) {
      trn_attr->_allowFilterReinits = strcasecmp("false", value);
    } else if (!strcmp("useModifiedWeighting", key)) {
      trn_attr->_useModifiedWeighting = atoi(value);
    } else if (!strcmp("samplePeriod",         key)) {
      trn_attr->_samplePeriod = atoi(value);
    } else if (!strcmp("maxNorthingCov",       key)) {
      trn_attr->_maxNorthingCov = atof(value);
    } else if (!strcmp("maxNorthingError",     key)) {
      trn_attr->_maxNorthingError = atof(value);
    } else if (!strcmp("maxEastingCov",        key)) {
      trn_attr->_maxEastingCov = atof(value);
    } else if (!strcmp("maxEastingError",      key)) {
      trn_attr->_maxEastingError = atof(value);
    } else if (!strcmp("RollOffset",           key)) {
      trn_attr->_phiBias = atof(value);
    } else if (!strcmp("useIDTData",           key)) {
      trn_attr->_useIDTData = strcasecmp("false", value);
    } else if (!strcmp("useDVLSide",           key)) {
      trn_attr->_useDvlSide = strcasecmp("false", value);
    } else if (!strcmp("useMbTrnData",         key)) {
      // Use the MbTrn.log file data when using either MbTrn data mode
      trn_attr->_useMbTrnData = strcasecmp("false", value);
    } else if (!strcmp("useMbTrnServer",       key)) {
      trn_attr->_useMbTrnData |= strcasecmp("false", value);
    } else {
      fprintf(stderr, "\n\tReplay: Unknown key in cfg: %s\n\n", key);
    }
  }

  fclose(cfg);
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


// Common to QNX and NIX versions
TerrainNav* Replay::connectTRN() {
    TerrainNav *_tercom = 0;

    fprintf(stdout, "replay - Using TerrainNav at %s on port %ld\n",
            trn_attr->_terrainNavServer, trn_attr->_terrainNavPort);
    fprintf(stdout, "replay - Using TerrainNav with map %s and config %s\n",
            trn_attr->_mapFileName, trn_attr->_vehicleCfgName);

    try {
        char buf[REPLAY_PATHNAME_LENGTH];
        char *mapdir  = getenv("TRN_MAPFILES");
        char *datadir = getenv("TRN_DATAFILES");

        // set path names (only if configured in trn_attr)
        memset(buf,0,REPLAY_PATHNAME_LENGTH);
        if(NULL!=trn_attr->_mapFileName) {
            snprintf(buf, REPLAY_PATHNAME_LENGTH, "%s/%s", mapdir, trn_attr->_mapFileName);
            free(trn_attr->_mapFileName); trn_attr->_mapFileName = strdup(buf);
        }
        memset(buf,0,REPLAY_PATHNAME_LENGTH);
        if(NULL!=trn_attr->_vehicleCfgName) {
            snprintf(buf, REPLAY_PATHNAME_LENGTH, "%s/%s", datadir, trn_attr->_vehicleCfgName);
            free(trn_attr->_vehicleCfgName); trn_attr->_vehicleCfgName = strdup(buf);
        }
        memset(buf,0,REPLAY_PATHNAME_LENGTH);
        if(NULL!=trn_attr->_particlesName) {
            snprintf(buf, REPLAY_PATHNAME_LENGTH, "%s/%s", datadir, trn_attr->_particlesName);
            free(trn_attr->_particlesName); trn_attr->_particlesName = strdup(buf);
        }
        fprintf(stdout, "%s:%d - %s\n%s\n%s\n", __FILE__,__LINE__,trn_attr->_mapFileName, trn_attr->_vehicleCfgName, trn_attr->_particlesName);

        if (useTRNServer()) {
            fprintf(stderr,"Connecting to %s in 2...\n", trn_attr->_terrainNavServer);
            sleep(1);
            _tercom = new TerrainNavClient(trn_attr->_terrainNavServer,
                                           trn_attr->_terrainNavPort,
                                           trn_attr->_mapFileName,
                                           trn_attr->_vehicleCfgName,
                                           trn_attr->_particlesName,
                                           TRNUtils::basename(logdir),
                                           trn_attr->_filter_type,
                                           trn_attr->_map_type);
        } else {
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
    catch (Exception e) {
        fprintf(stderr, "replay - Failed TRN connection. Check TRN error messages...\n");
        _tercom = 0;
    }

    if (_tercom) {
        // After moving is_connected() to public section, we can use this code block
        if (_tercom &&  (!_tercom->is_connected() || !_tercom->initialized())) {
            fprintf(stderr, "replay -:Not initialized. See trn_server error messages...\n");
        }
        // If we reach here then we've connected
        fprintf(stdout, "replay -:Should be Connected to server if no error messages...\n");
        fprintf(stdout, "replay -:Lets just set the interpret measure attitude flag to true...\n");
        // The following calls to setup TRN lifted from TerrainAidDriver
        _tercom->setInterpMeasAttitude(true);
        //choose filter settings based on whether kearfott is available and if
        //filter forcing is set
        if(trn_attr->_forceLowGradeFilter) {
            _tercom->useLowGradeFilter();
        } else {
            _tercom->useHighGradeFilter();
        }
        //turn on filter reintialization if set in terrainAid.cfg
        _tercom->setFilterReinit(trn_attr->_allowFilterReinits);
        //turn on modified weighting if set in terrainAid.cfg
        _tercom->setModifiedWeighting(trn_attr->_useModifiedWeighting);
    }

    return _tercom;
}

int Replay::parseDvlCsvLine(const char *line, poseT *pt, measT *mt)
{
  char *lcopy = strdup(line);
  char *buf = lcopy;

  // Calculate the sample period in seconds to use
  double period = (double)trn_attr->_samplePeriod / 1000.;  // ms => seconds

  // initialize the number of measurements to zero
  mt->numMeas = 0;
  // Parse position data and the number of measurements from the current line
  // Those are the items upto the start of the range values
  for (int c = 0; c < DVL_RANGES; c++)
  {
    char* token = strtok((char*)buf, ",");
    buf = NULL;

    // we're done when a malformed line is detected
    if (NULL == token) {
      fprintf(stderr, "Replay - unexpected EOL parsing line %ld\n", nupdates);
      free(lcopy);
      return 0;
    }
    //printf("  %s\n", token);

    // Extract the poseT values (line items up to the start of the ranges)
    if (DVL_TIME == c) {
      pt->time = atof(token);

      // skip lines with timestamp inside the previous sample period
      if (pt->time < lastTime + period) {
        free(lcopy);
        return -1;
      }

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

  // skip unless valid numMeas
  if (mt->numMeas < 0) {
    fprintf(stderr, "Replay - invalid numMeas (%d) on input line %ld\n",
      mt->numMeas, nupdates);
    return -1;
  }

  // use poseT timestamp
  mt->time = pt->time;
  // allocate for a complete measT with numMeas measurements
  mt->covariance = (double*)realloc(mt->covariance, N_COVAR*sizeof(double));
  mt->ranges     = (double*)realloc(mt->ranges,     mt->numMeas*sizeof(double));
  mt->crossTrack = (double*)realloc(mt->crossTrack, mt->numMeas*sizeof(double));
  mt->alongTrack = (double*)realloc(mt->alongTrack, mt->numMeas*sizeof(double));
  mt->altitudes  = (double*)realloc(mt->altitudes,  mt->numMeas*sizeof(double));
  mt->alphas     = (double*)realloc(mt->alphas,     mt->numMeas*sizeof(double));
  mt->measStatus =   (bool*)realloc(mt->measStatus, mt->numMeas*sizeof(bool));

  // initialize to zeros
  memset(mt->covariance,0,N_COVAR*sizeof(double));
  memset(mt->ranges,0,mt->numMeas*sizeof(double));
  memset(mt->crossTrack,0,mt->numMeas*sizeof(double));
  memset(mt->alongTrack,0,mt->numMeas*sizeof(double));
  memset(mt->altitudes,0,mt->numMeas*sizeof(double));
  memset(mt->alphas,0,mt->numMeas*sizeof(double));
  memset(mt->measStatus,0,mt->numMeas*sizeof(bool));

  // Now continue and get measure data == beam data
  // Folks use the csv file for a variety of different sensors
  // default is dvl and 3 items.
  // Traditionally, there have been 3 data items per beam, or 3 * numMeas
  // However, with the advent of MB1 records in CSV file format there now
  // may be 6 items per measurement.
  int nItems = 0;
  const char *instr = "";
  if (trn_attr->_useIDTData) {
    mt->dataType = TRN_SENSOR_DELTAT;
    instr = "DeltaT";
    nItems = 3;
  } else if (trn_attr->_useMbTrnData) {
    mt->dataType = TRN_SENSOR_MB;
    instr = "Multibeam";
    nItems = 6;
  } else {
    mt->dataType = TRN_SENSOR_DVL;
    instr = "DVL";
    nItems = 3;
  }

  char *lastToken = NULL;
  for (int b = 0; b < mt->numMeas * nItems; b++) {
    char* token = strtok((char*)buf, ",");
    buf = NULL;
    int bi = b / nItems;   // beam index
    int bs = b % nItems;   // beam sub-index

    // there must be nItems elements in the record for each measurement
    if (NULL == token) {
      fprintf(stderr, "Replay - unexpected EOL parsing record %ld of %s data\n",
        nupdates, instr);
      fprintf(stderr, "Replay - last parsed token: %s \n",
        (lastToken == NULL ? "" : lastToken));
      fprintf(stderr, "Replay - expecting %d items, EOL detected after beam #%d\n",
        1+DVL_RANGES+(mt->numMeas*nItems), bi);
      fprintf(stderr, "Replay - expecting %d items per beam with %s data\n",
        nItems, instr);
      free(lcopy);
      return 0;
    }

    // these go with beam index (b/nItems) (or measurement number)
    // 1st item is beam number and is ignored
    if (MEAS_STATUS == bs)              // 2nd item is measStatus
      mt->measStatus[bi] = atoi(token);
    else if (MEAS_RANGE == bs)          // 3rd item is range
      mt->ranges[bi] = atof(token);
    else if (MEAS_ALONGTRACK == bs)     // 4th item is alongtrack
      mt->alongTrack[bi] = atof(token);
    else if (MEAS_CROSSTRACK == bs)     // 5th item is crosstrack
      mt->crossTrack[bi] = atof(token);
    else if (MEAS_ALTITUDE == bs)       // 6th item is altitude
      mt->altitudes[bi] = atof(token);

  }

  free(lcopy);

  return 1;
}
