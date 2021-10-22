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
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
// OS X before Sierra does not have clock_gettime, use clock_get_time
#if defined(__APPLE__) && !defined(__CLOCK_AVAILABILITY) && defined(__MACH__)
// host_get_clock_service
#include <mach/mach.h>
// host_get_clock_service
#include <mach/clock.h>
#endif


#include "TrnClient.h"
#include "matrixArrayCalcs.h"
#include "TNavConfig.h"
#include "TerrainNavClient.h"

#define TRNCLI_PER_DFL 2
#define TRNCLI_RUN_SEM_NAME "trncli_run_sem"

bool g_quit=false;

typedef enum{OF_ASCII=0, OF_CSV}outfmt_t;

typedef struct trn_worker_s
{
    poseT pt;
    poseT mle;
    poseT mse;
    measT mt;
    outfmt_t ofmt;
    ofstream *pfile;
    char *map;
    char *host;
    long port;
    char *logdir;
    int verbose;
    TrnClient *trncli;
    TerrainNav *tnav;
    bool quit;
    sem_t *run_sem;
}trn_worker_t;

static int numReinits = 0;

// Verbose mode print facility
//
void print(measT *mt, poseT *pt, poseT *mle, poseT *mse, char goodMeas);

void out_cons(double time, measT *mt, poseT *pt, poseT *mle, poseT *mse, char goodMeas,double *cov, int covn,int fs,int nr)
{
    fprintf(stderr,"\n");

    //display tercom estimate biases
    fprintf(stderr,"MLE[t,x,y,z] [ %.3lf, %.2f , %.4f , %.4f , %.4f ]\n",time,mse->time,mle->x-pt->x, mle->y-pt->y, mle->z-pt->z);
    fprintf(stderr,"MSE[t,x,y,z] [ %.3lf, %.2f , %.4f , %.4f , %.4f ]\n",time,mse->time, mse->x-pt->x, mse->y-pt->y, mse->z-pt->z);


    fprintf(stderr,"COV          [ %.3lf, %.2f , %.2f , %.2f ]\n",
            time,
            (N_COVAR>=6?sqrt(mse->covariance[0]):-1.0),
            (N_COVAR>=6?sqrt(mse->covariance[2]):-1.0),
            (N_COVAR>=6?sqrt(mse->covariance[5]):-1.0));

    // Continue to invoke tercom like a normal mission
    //
    fprintf(stderr,"FSTATE       [%.3lf, %d]\n",time, fs);
    fprintf(stderr,"REINIT       [%.3lf, %d]\n",time, nr);


}


static double s_etime()
{
    double retval=0.0;

    struct timespec ts;
    memset(&ts,0,sizeof(struct timespec));

    // OS X before Sierra does not have clock_gettime, use clock_get_time
#if defined(__APPLE__) && !defined(__CLOCK_AVAILABILITY) && defined(__MACH__)
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    ts.tv_sec = mts.tv_sec;
    ts.tv_nsec = mts.tv_nsec;
#else
    clock_gettime(CLOCK_REALTIME, &ts);
#endif

    retval=ts.tv_sec+(ts.tv_nsec/1.e9);

    return retval;
}
// End function s_etime

void out_csv(double time, measT *mt, poseT *pt, poseT *mle, poseT *mse, char goodMeas,double *cov, int covn,int fs,int nr)
{

    fprintf(stderr,"%.3lf,%.3lf,%.4lf,%.4lf,%.4lf,",time,mle->time,mle->x,mle->y,mle->z);
    fprintf(stderr,"%.3lf,%.4lf,%.4lf,%.4lf,",mse->time,mse->x,mse->y,mse->z);
    fprintf(stderr,"%.4lf,%.4lf,%.4lf,",pt->x, pt->y, pt->z);
    fprintf(stderr,"%.4lf,%.4lf,%.4lf,",mse->vn_x, mse->vn_y, mse->vn_z);
    fprintf(stderr,"%.3lf,%.3lf,%.3lf,",(N_COVAR>=6?sqrt(mse->covariance[0]):-1.0),(N_COVAR>=6?sqrt(mse->covariance[2]):-1.0),(N_COVAR>=6?sqrt(mse->covariance[5]):-1.0));
    fprintf(stderr,"%d,%d\n",fs,nr);
}

void static s_trn_cycle(trn_worker_t *worker)
{
    try{
        double timestamp=s_etime();
        // Estimate location. 1=> MLE; 2=> MMSE
        worker->mse.covariance[0] = worker->mse.covariance[1] =
        worker->mse.covariance[2] = worker->mse.covariance[3] = 0.;

        worker->tnav->estimatePose( &worker->mle, 1);
        worker->tnav->estimatePose( &worker->mse, 2);

        char goodMeas = worker->tnav->lastMeasSuccessful();
        int fs = worker->tnav->getFilterState();
        int nr =  worker->tnav->getNumReinits();
        if (nr > numReinits)
        {
            numReinits = nr;
        }

        switch (worker->ofmt) {
            case OF_CSV:
            out_csv(timestamp,&worker->mt, &worker->pt, &worker->mle, &worker->mse, goodMeas, &worker->mse.covariance[0],0,fs,nr);
            break;

            case OF_ASCII:
            out_cons(timestamp,&worker->mt, &worker->pt, &worker->mle, &worker->mse, goodMeas, &worker->mse.covariance[0],0,fs,nr);
            default:
            break;
        }

    }catch(Exception &e){
        fprintf(stderr,"\n%s\n", e.what());
        worker->quit=true;
    }

}

void *client_worker(void *vworker)
{
    trn_worker_t *worker = (trn_worker_t *)vworker;

    while(NULL!=worker && worker->quit==false && g_quit==false){

        s_trn_cycle(worker);

        sem_wait(worker->run_sem);
    }

    fprintf(stderr,"worker thread quitting\n");
    pthread_exit(NULL);

}

static void s_sig_handler(int sig)
{
    switch (sig) {
        case SIGINT:
        g_quit=true;
        fprintf(stderr,"SIGINT received\n");
        break;

        default:
        fprintf(stderr,"caught signal [%d]\n",sig);
       break;
    }
}

static void s_delay_sec(double period)
{
    long lnsec = period*1000000000.;
    struct timespec delay={0,0};
    struct timespec rem={0,0};
    memset(&delay,0,sizeof(struct timespec));
    delay.tv_sec= (lnsec/1000000000);
    delay.tv_nsec=(lnsec%1000000000);
    memset(&rem,0,sizeof(struct timespec));
    //        fprintf(stderr,"%s - delay s[%ld] ns[%ld]\r\n",__FUNCTION__,delay.tv_sec,delay.tv_nsec);
    while ( nanosleep(&delay,&rem)!=0) {
        memcpy(&delay,&rem,sizeof(struct timespec));
        memset(&rem,0,sizeof(struct timespec));
    }
    return;
}

int main(int argc, char** argv)
{

    signal(SIGINT,s_sig_handler);

    trn_worker_t worker_s, *worker=&worker_s;
    memset((void *)worker,0,sizeof(trn_worker_t));

    worker->ofmt = OF_ASCII;
    worker->pfile=NULL;
    worker->host=strdup("127.0.0.1");
    worker->port=27027;
    worker->map=NULL;
    worker->verbose=0;
    worker->logdir=NULL;
    worker->trncli=NULL;
    worker->tnav=NULL;
    worker->quit=false;

    worker->mt.numMeas    = 4;
    worker->mt.ranges     = new double[11];
    worker->mt.crossTrack = new double[11];
    worker->mt.alongTrack = new double[11];
    worker->mt.beamNums   = new int[11];
    worker->mt.altitudes  = new double[11];
    worker->mt.alphas     = new double[11];
    worker->mt.measStatus = new bool[11];   //mt.numMeas];

    bool is_threaded=true;
    int c=0;
    double delay_sec=0.0;

    while ( (c = getopt(argc, argv, "a:f:hl:m:o:p:st:v:")) != EOF )
    {
        if (c == 'l'){
            if(NULL!=worker->logdir)
            free(worker->logdir);
            worker->logdir = strdup(optarg);   // Log directory
        }
        else if (c == 'a'){
            if(NULL!=worker->host)
            free(worker->host);
            worker->host = strdup(optarg);     // TRN host overrides host in config file
        }
        else if (c == 'm'){
            if(NULL!=worker->map)
            free(worker->map);
            worker->map = strdup(optarg);     // TRN map overrides map in config file
        }
        else if (c == 'p'){
            worker->port = atol(optarg);       // TRN port overrides port in config file
        }
        else if (c == 'o'){
            switch (optarg[0]) {
                case 'c':
                worker->ofmt=OF_CSV;
                break;

                case 'a':
                default:
                worker->ofmt=OF_ASCII;
                break;
            }
        }
        else if (c == 'v'){
            worker->verbose = atoi(optarg);
        }
        else if (c == 't'){
            //            int p=0;
            //            sscanf(optarg,"%d",&p);
            //            period_sec=(p>0?p:TRNCLI_PER_DFL);
            double d =0.0;
            sscanf(optarg,"%lf",&d);
            delay_sec =(d>0?d:TRNCLI_PER_DFL);
        }
        else if (c == 's'){
            is_threaded=false;
        }
        else if (c == 'h'){
            fprintf(stderr,"\n");
            fprintf(stderr," trnclient_test [options]\n");
            fprintf(stderr,"\n");
            fprintf(stderr,"  -l dir : log directory\n");
            fprintf(stderr,"  -a ip  : host IP addr\n");
            fprintf(stderr,"  -p n   : host IP port\n");
            fprintf(stderr,"  -o c   : output fmt (a|c)\n");
            fprintf(stderr,"  -v n   : verbose output level\n");
            fprintf(stderr,"  -t n   : period sec\n");
            fprintf(stderr,"  -h     : help message\n");
            fprintf(stderr,"\n");
            exit(0);
        }
    }

    // configure TRN logging
    tl_mconfig(TL_TRN_SERVER, TL_SERR, TL_ALL);
    //    tl_mconfig(TL_STRUCT_DEFS, TL_SERR, TL_NC);
    //    tl_mconfig(TL_TERRAIN_NAV, TL_SERR, TL_NC);
    //    tl_mconfig(TL_TERRAIN_NAV_AID_LOG, TL_SERR, TL_NC);
    //    tl_mconfig(TL_TNAV_CONFIG, TL_SERR, TL_NC);
    tl_mconfig(TL_TNAV_PARTICLE_FILTER, TL_SERR, TL_NC);
    tl_mconfig(TL_TNAV_FILTER, TL_SERR, TL_NC);

    // Create and initialize the Replay object
    //
    worker->trncli = new TrnClient(worker->host, worker->port);
    worker->trncli->setVerbose(worker->verbose);

    // Open connection to the TRN server. The server
    // initialization will fail unless the correct
    // map and vehicle configuration files are present
    // on the server.
    //
    worker->tnav = worker->trncli->connectTRN();
    if (NULL==worker->tnav)
    {
        fprintf(stderr," TRN server connection failed.\n");
        return 1;
    }

    pthread_t worker_thread;
    if(is_threaded){
        if( pthread_create(&worker_thread, NULL, client_worker, (void *)worker)!=0){
            fprintf(stderr,"worker thread create failed [%d/%s]\n",errno,strerror(errno));
            exit(-1);
        }
//        sem_init(&worker->run_sem,0,0);
        worker->run_sem = sem_open(TRNCLI_RUN_SEM_NAME,O_CREAT);
        if(NULL == worker->run_sem){
            fprintf(stderr,"NULL semaphore [%d/%s]\n",errno,strerror(errno));
            exit(0);
        }
    }

    while(g_quit==false){
        s_delay_sec(delay_sec);
        if(is_threaded){
            sem_post(worker->run_sem);
        }else{
            s_trn_cycle(worker);
        }
    }

    if(is_threaded){
        fprintf(stderr,"quit flag set, signaling worker thread...\n");
        sem_post(worker->run_sem);
        fprintf(stderr,"waiting for worker thread...\n");
        pthread_join( worker_thread, NULL);
    }

    if(NULL!=worker->logdir)
    free(worker->logdir);
    if(NULL!=worker->map)
    free(worker->map);
    if(NULL!=worker->host)
    free(worker->host);

    sem_close(worker->run_sem);
    sem_unlink(TRNCLI_RUN_SEM_NAME);
    fprintf(stderr,"done\n");
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
