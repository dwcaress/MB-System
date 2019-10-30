/****************************************************************************/
/* Copyright (c) 2017 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  : TrnClient test app wrapper  */
/* Filename : trnclient_test.cpp                                            */
/* Author   : headley                                                       */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 10/24/2019                                                    */
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

#include "TrnClient.h"
#include "matrixArrayCalcs.h"
#include "TNavConfig.h"
#include "TerrainNavClient.h"

static int numReinits = 0;

// Verbose mode print facility
// 
void print(measT *mt, poseT *pt, poseT *mle, poseT *mse, char goodMeas);

int main(int argc, char* argv[])
{
    // Get the connection paramteres from the command line
    //
    char *map = 0, *host=strdup("127.0.0.1"), *logdir=0;
    int verbose=0;
    ofstream *pfile = NULL;
    long port=27027;
    int c;
#define TRNCLI_PER_DFL 2
    unsigned int period_sec=TRNCLI_PER_DFL;
    
    // Deal with cmd-line options
    //
    while ( (c = getopt(argc, argv, "m:h:p:l:v:f:t:")) != EOF )
    {
        if (c == 'l')
            logdir = strdup(optarg);   // Log directory
        else if (c == 'h')
            host = strdup(optarg);     // TRN host overrides host in config file
        else if (c == 'm')
            map = strdup(optarg);     // TRN map overrides map in config file
        else if (c == 'p')
            port = atol(optarg);       // TRN port overrides port in config file
        else if (c == 'v')
            verbose = atoi(optarg);
        else if (c == 't'){
            int p=0;
            sscanf(optarg,"%d",&p);
            period_sec=(p>0?p:TRNCLI_PER_DFL);
        }
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
    TrnClient *trncli = new TrnClient(host, port);
    
    trncli->setVerbose(verbose);
    
    // Open connection to the TRN server. The server
    // initialization will fail unless the correct
    // map and vehicle configuration files are present
    // on the server.
    //
    TerrainNav *tnav = trncli->connectTRN();
    if (!tnav)
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
    
    poseT pt, mle, mse;
    measT mt;
    mt.numMeas    = 4;
    mt.ranges     = new double[11];
    mt.crossTrack = new double[11];
    mt.alongTrack = new double[11];
    mt.beamNums   = new int[11];
    mt.altitudes  = new double[11];
    mt.alphas     = new double[11];
    mt.measStatus = new bool[11];   //mt.numMeas];
    
    // Conntinue as long as measure and motion update
    // data remains in the mission log files.
    //
    
    while(1)
    {
        fprintf(stderr,"\n");
        sleep(period_sec);
 
        // System::milliSleep(0);
        // Estimate location. 1=> MLE; 2=> MMSE
        //
        mse.covariance[0] = mse.covariance[1] =
        mse.covariance[2] = mse.covariance[3] = 0.;
        
        if(verbose>0)
        fprintf(stderr, "%s:%d - req MLE est\n",__FUNCTION__,__LINE__);

        tnav->estimatePose( &mle, 1);

        if(verbose>0)
        fprintf(stderr, "%s:%d - req MSE est\n",__FUNCTION__,__LINE__);

        tnav->estimatePose( &mse, 2);

        if(verbose>0)
        fprintf(stderr, "%s:%d - req lastMeasSuccessful\n",__FUNCTION__,__LINE__);

        // Spew if requested
        //
            char goodMeas = tnav->lastMeasSuccessful();
        if (verbose)
        {
            print(&mt, &pt, &mle, &mse, goodMeas);
        }
        
        //display tercom estimate biases
        printf("MLE[t,x,y,z] [ %.2f , %.4f , %.4f , %.4f ]\n",mle.time, mle.x-pt.x, mle.y-pt.y, mle.z-pt.z);
        printf("MSE[t,x,y,z] [ %.2f , %.4f , %.4f , %.4f ]\n",mse.time, mse.x-pt.x, mse.y-pt.y, mse.z-pt.z);
        
        if (N_COVAR >= 6){
	        printf("COVAR        [ %.2f , %.2f , %.2f ]\n",
                   sqrt(mse.covariance[0]),
                   sqrt(mse.covariance[2]),
                   sqrt(mse.covariance[5]));
        }
        
        // Continue to invoke tercom like a normal mission
        //
        int fs = tnav->getFilterState();
        int nr = tnav->getNumReinits();
        if (nr > numReinits)
        {
            fprintf(stderr,"REINIT[%d]\n", nr);
            numReinits = nr;
        }
     }
    
    // Done
    //
    fprintf(stderr,"Done. Close the connection "
            " reinits[%d]\n",  numReinits);
    delete tnav;
    
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
    
    if (mt->numMeas >= 4)
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
