/****************************************************************************/
/* Copyright (c) 2017 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  : Linux ed. Replay TRN using logged data from previous mission  */
/* Filename : replay.cc                                                     */
/* Author   : Henthorn                                                      */
/* Project  : Iceberg AUV                                                   */
/* Version  : 1.0                                                           */
/* Created  : 07/03/2017                                                    */
/* Modified : 09/06/2019  - Added filter logging option                     */
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

#include "Replay.h"
#include "matrixArrayCalcs.h"
#include "TNavConfig.h"
#include "TerrainNavClient.h"

static int numReinits = 0;

// Verbose mode print facility
// 
void print(long int n, measT *mt, poseT *pt, poseT *mle, poseT *mse, char goodMeas);

int main(int argc, char* argv[])
{
  // Get the connection parameters from the command line
  //
  char *map = 0, *host=0, *logdir=0, verbose=0;
  ofstream *pfile = NULL;
  unsigned int ptype = SAVE_PARTICLES;
  long port=27027;
  int c;

  // Deal with cmd-line options
  // 
  while ( (c = getopt(argc, argv, "m:h:p:l:vf:L")) != EOF )
  {
      if (c == 'l'){
          free(logdir);
          logdir = strdup(optarg);   // Log directory
      }
      else if (c == 'L'){
          free(host);
          host = strdup(LCM_HOST);   // Send updates to LCM channels
      }
      else if (c == 'h'){
          free(host);
          host = strdup(optarg);     // TRN host overrides host in config file
      }
      else if (c == 'm'){
          free(map);
          map = strdup(optarg);      // TRN map overrides map in config file
      }
    else if (c == 'p')
      port = atol(optarg);       // TRN port overrides port in config file
    else if (c == 'v')
      verbose = 1;
    else if (c == 'f')
    {
      if      (!strncasecmp(optarg,"h", 1))
        ptype = HISTOGRAMTOFILE;   // Log histograms
      else if (!strncasecmp(optarg,"p", 1))
        ptype = PARTICLESTOFILE;   // Log particles
      else
      {
        fprintf(stderr, "\n\tBad filter log option:%s - using %s\n\n",
          optarg, SAVE_PARTICLES? "particles" : "histogram");
        ptype = SAVE_PARTICLES;
      }

      fprintf(stderr, "\tFilter log option set to:%s \n",
        ptype == PARTICLESTOFILE? "particles" : "histogram");

      pfile = new ofstream;
    }
  }

  // Log directory is required argument
  // 
  if (!logdir)
  {
    fprintf(stderr," No log directory specified.\n"
                  "Usage:\n  trn_replay -l dir [-h ip -p num -m map -f p|h ] \n"
                  "    -l dir  The Dorado log directory created by the mission you want to replay\n"
                  "    -h ip   Alternate TRN server ip address to override the address in terrainAid.cfg (use \"native\" to run locally)\n"
                  "    -p num  Alternate TRN server port to override the port in terrainAid.cfg or the default port\n"
                  "    -m map  Alternate map name to override the map specified in terrainAid.cfg\n"
                  "    -f p|h  Directive to log filter distributions in filterDistrib.txt (can be very large), log particles or histograms\n"
                  );
//                  "  Use the \"make_replay_csvs.sh\" utility to convert the QNX logs to compressed zip files\n");
      free(logdir);
      free(host);
      free(map);
      delete pfile;
    return 1;
  }

          tl_mconfig(TL_TRN_SERVER, TL_SERR, TL_ALL);
    //    tl_mconfig(TL_STRUCT_DEFS, TL_SERR, TL_NC);
    //    tl_mconfig(TL_TERRAIN_NAV, TL_SERR, TL_NC);
    //    tl_mconfig(TL_TERRAIN_NAV_AID_LOG, TL_SERR, TL_NC);
    //    tl_mconfig(TL_TNAV_CONFIG, TL_SERR, TL_NC);
          tl_mconfig(TL_TNAV_PARTICLE_FILTER, TL_SERR, TL_NC);
          tl_mconfig(TL_TNAV_FILTER, TL_SERR, TL_NC);

  // Create and initialize the Replay object
  // 
  Replay *r = new Replay(logdir, map, host, port);

  // Load the terrainAid.cfg from the given log dir
  // 
  //r->loadCfgAttributes();

  // Open the log files and use those for data
  // 
  //r->openLogFiles();

  // Open connection to the TRN server. The server
  // initialization will fail unless the correct
  // map and vehicle configuration files are present
  // on the server.
  // 
  TerrainNav *_tercom = r->connectTRN();
  if (NULL==_tercom)
  {
    fprintf(stderr," TRN server connection failed.\n");
      free(logdir);
      free(host);
      free(map);
      delete pfile;
      delete r;
    return 1;
  }

  // User has opted to save the filter distrib
  //
  if (pfile)
  {
    char filename[512];
    pfile->open(charCat(filename, TNavConfig::instance()->getLogDir(),
                                  "/filterDistrib.txt"));
    _tercom->tNavFilter->setDistribToSave(ptype);
  }

  // ********************** MAIN LOOP *******************
  // 
  // Use the data files in the log directory to get the
  // motion and measure updates that were used by TRN in
  // this mission, and send them again.
  // 
  long int nupdates, nu, ng;
  poseT pt, mle, mse;
  measT mt;
  mt.numMeas    = 4;
  mt.ranges     = (double *)malloc(11*sizeof(double));
  mt.crossTrack = (double *)malloc(11*sizeof(double));
  mt.alongTrack = (double *)malloc(11*sizeof(double));
  mt.beamNums   = (int *)malloc(11*sizeof(int));
  mt.altitudes  = (double *)malloc(11*sizeof(double));
  mt.alphas     = (double *)malloc(11*sizeof(double));
  mt.measStatus = (bool *)malloc(11*sizeof(bool));

  // Conntinue as long as measure and motion update
  // data remains in the mission log files.
  // 
  int s;
  for (nupdates=0, nu=0, ng=0; (s = r->getNextRecordSet(&pt, &mt)) != 0; nupdates += 2)
  {
    // Skip this record if indicated
    // 
    if (s < 0) continue;

    nu++;
    //sleep(3);

    //continue;
    // Order is significant, so if the measT timestamp is
    // earlier then perform a measUpdate first.
    // 
    if (pt.time <= mt.time)
    {
      _tercom->motionUpdate(&pt);
      _tercom->measUpdate(&mt, mt.dataType);
    }
    else
    {
      _tercom->measUpdate(&mt, mt.dataType);
      _tercom->motionUpdate(&pt);
    }

    // System::milliSleep(0);
    // Estimate location. 1=> MLE; 2=> MMSE
    // 
    mse.covariance[0] = mse.covariance[1] = 
                        mse.covariance[2] = mse.covariance[3] = 0.;

    _tercom->estimatePose( &mle, 1);
    _tercom->estimatePose( &mse, 2);

    // Spew if requested
    // 
    char goodMeas = _tercom->lastMeasSuccessful();
    if (verbose)
    {
      print(nu, &mt, &pt, &mle, &mse, goodMeas);
    }

    if (goodMeas)
    {
      ng++;
      if (pfile) _tercom->tNavFilter->saveCurrDistrib(*pfile);

      //display tercom estimate biases
      printf("MLE: %.2f , ",mle.time);
      printf("%9.4f , %9.4f , %9.4f , ",
        mle.x-pt.x, mle.y-pt.y, mle.z-pt.z);
      printf("MSE: %.2f , ", mse.time);
      printf("%9.4f , %9.4f , %9.4f , ",
       mse.x-pt.x, mse.y-pt.y, mse.z-pt.z);
       
      if (N_COVAR >= 6)
        printf("COVAR: %8.2f , %8.2f , %8.2f\n",
              sqrt(mse.covariance[0]),
              sqrt(mse.covariance[2]),
              sqrt(mse.covariance[5]));
    }

    // Continue to invoke tercom like a normal mission
    // 
    int nr = _tercom->getNumReinits();
    if (nr > numReinits)
    {
      fprintf(stderr,"TRN reinit number %d\n", nr);
      //sleep(1);
      numReinits = nr;
    }

    if (pfile) _tercom->tNavFilter->saveCurrDistrib(*pfile);

//    char buf[10]; usleep(200000L); gets(buf);
  }

  // Done
  // 
  fprintf(stderr,"Done. Close the connection after %ld updates, %ld good meas"
                " and %d reinits %ld updates...\n", nu, ng, numReinits, nupdates);
    free(logdir);
    free(host);
    free(map);
    delete pfile;
    delete r;
  delete _tercom;
    TNavConfig::instance(true);
  return 0;
}

// Verbose mode print facility
// 
void print(long int nu, measT *mt, poseT *pt, poseT *mle, poseT *mse, char goodMeas)
{
  fprintf(stderr, "Iteration: %ld\n",nu);
  fprintf(stderr,"\nposeT: %.6f\n"
         "  x    : %.6f\n"
         "  y    : %.6f\n"
         "  z    : %.6f\n"
         "  phi  : %.6f\n"
         "  theta: %.6f\n"
         "  psi  : %.6f\n"
         "  dvlV : %d\n"
         "  gpsV : %d\n"
         "  BLock: %d\n",
         pt->time, pt->x, pt->y, pt->z,
                  pt->phi, pt->theta, pt->psi,
                  pt->dvlValid, pt->gpsValid, pt->bottomLock);

  if (mt->numMeas >= 4)
    fprintf(stderr,"\nmeasT: %.6f\t%d beams\n"
         "  beam1: %.6f\n"
         "  beam2: %.6f\n"
         "  beam3: %.6f\n"
         "  beam4: %.6f\n"
         "  phi  : %.6f\n"
         "  theta: %.6f\n"
         "  psi  : %.6f\n",
         mt->time, mt->numMeas, mt->ranges[0], mt->ranges[1], mt->ranges[2], mt->ranges[3],
         mt->phi, mt->theta, mt->psi);

  // Print position estimates only when they were successful
  // 
  if (goodMeas && N_COVAR >= 4)
  {
    fprintf(stderr,"\nmmse :\n"
           "  lestX: %.6f\n"
           "  lestY: %.6f\n"
           "  lestZ: %.6f\n"
           "  sestX: %.6f\n"
           "  sestY: %.6f\n"
           "  sestZ: %.6f\n"
           "  sigmN: %.6f\n"
           "  sigmE: %.6f\n"
           "  sigZ: %.6f\n",
           mle->x-pt->x, mle->y-pt->y, mle->z-pt->z,
           mse->x-pt->x, mse->y-pt->y, mse->z-pt->z,
           sqrt(mse->covariance[0]), sqrt(mse->covariance[2]),
           sqrt(mse->covariance[3]));
  }
}

#if 0
// Extra poseT fields if needed
// 
         "  vx   : %.6f\n"
         "  vy   : %.6f\n"
         "  vz   : %.6f\n"
         "  ve   : %.6f\n"
         "  vw_y : %.6f\n"
         "  vw_y : %.6f\n"
         "  vw_z : %.6f\n"
         "  vn_x : %.6f\n"
         "  vn_y : %.6f\n"
         "  vn_z : %.6f\n"
         "  wx   : %.6f\n"
         "  wy   : %.6f\n"
         "  wz   : %.6f\n"

                pt->vx, pt->vy, pt->vz, pt->ve,
                pt->vw_x, pt->vw_y, pt->vw_z,
                pt->vn_x, pt->vn_y, pt->vn_z,
                pt->wx, pt->wy, pt->wz,
#endif
