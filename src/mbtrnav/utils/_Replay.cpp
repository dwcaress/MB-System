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

#include "Replay.h"
#include "TerrainNavClient.h"

static int numReinits = 0;

// Verbose mode print facility
// 
void print(measT *mt, poseT *pt, poseT *mle, poseT *mse, char goodMeas);

int main(int argc, char* argv[])
{
  // Get the connection parameters from the command line
  //
  char *host=0, *logdir=0, verbose=0;
  long port=27027;
  int c;

  // Deal with cmd-line options
  // 
  while ( (c = getopt(argc, argv, "h:p:l:v")) != EOF )
  {
    if (c == 'l')
      logdir = strdup(optarg);   // Log directory
    else if (c == 'h')
      host = strdup(optarg);     // TRN host overrides host in config file
    else if (c == 'p')
      port = atol(optarg);       // TRN port overrides port in config file
    else if (c == 'v')
      verbose = 1;
  }

  // Log directory is required argument
  // 
  if (!logdir)
  {
    fprintf(stderr," No log directory specified.\n"
                  "Usage:\n  replay -l logdir [-h host -p port] \n"
                  "  Uses a native TerrainNav object rather than a TRN server when host is \"native\"\n"
                  "  Use the \"make_replay_csvs.sh\" utility to convert the QNX logs to compressed zip files\n");
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
  Replay *r = new Replay(logdir, host, port);

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
  if (!_tercom)
  {
    fprintf(stderr," TRN server connection failed.\n");
    return 1;
  }

  // ********************** MAIN LOOP *******************
  // 
  // Use the data files in the log directory to get the
  // motion and measure updates that were used by TRN in
  // this mission, and send them again.
  // 
  long int nupdates;
  poseT pt, mle, mse;
  measT mt;
  mt.numMeas    = 4;
  mt.ranges     = new double[mt.numMeas];
  mt.altitudes  = new double[mt.numMeas];
  mt.alphas     = new double[mt.numMeas];
  mt.measStatus = new bool[mt.numMeas];

  // Conntinue as long as measure and motion update
  // data remains in the mission log files.
  // 
  for (nupdates = 0; r->getNextRecordSet(&pt, &mt); nupdates += 2)
  {
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
      print(&mt, &pt, &mle, &mse, goodMeas);
    }

    if (goodMeas)
    {
      //display tercom estimate biases
      printf("%.2f , ",mle.time);
      printf("%.4f , %.4f , %.4f , ",
        mle.x-pt.x, mle.y-pt.y, mle.z-pt.z);
      printf("%.2f , ", mse.time);
      printf("%.4f , %.4f , %.4f , ",
       mse.x-pt.x, mse.y-pt.y, mse.z-pt.z);
       
      printf("%.2f , %.2f , %.2f\n",
              sqrt(mse.covariance[0]),
              sqrt(mse.covariance[2]),
              sqrt(mse.covariance[5]));
    }

    // Continue to invoke tercom like a normal mission
    // 
    int fs = _tercom->getFilterState();
    int nr = _tercom->getNumReinits();
    if (nr > numReinits)
    {
      fprintf(stderr,"TRN reinit number %d", nr);
      sleep(1);
      numReinits = nr;
    }
    char buf[10];
//    gets(buf);
  }

  // Done
  // 
  fprintf(stderr,"Done. Close the connection after %ld updates"
                " and %d reinits...\n", nupdates, numReinits);
  delete _tercom;

  return 0;
}

// Verbose mode print facility
// 
void print(measT *mt, poseT *pt, poseT *mle, poseT *mse, char goodMeas)
{
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
  fprintf(stderr,"\nmeasT: %.6f\n"
         "  beam1: %.6f\n"
         "  beam2: %.6f\n"
         "  beam3: %.6f\n"
         "  beam4: %.6f\n"
         "  phi  : %.6f\n"
         "  theta: %.6f\n"
         "  psi  : %.6f\n",
         mt->time, mt->ranges[0], mt->ranges[1], mt->ranges[2], mt->ranges[3],
         mt->phi, mt->theta, mt->psi);

  // Print position estimates only when they were successful
  // 
  if (goodMeas)
  {
    fprintf(stderr,"\nmmse : %.6f\n"
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
