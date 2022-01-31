///
/// @file trnclient_unit.cpp
/// @authors k. headley
/// @date 2021-09-30

/// Unit test for TerrainNavClient (TrnClient)
/// Excercises TerrainNav, commsT, TRN server/trnif

/// @sa doxygen-examples.c for more examples of Doxygen markup


/////////////////////////
// Terms of use
/////////////////////////
/*
 Copyright Information

 Copyright 2002-2019 MBARI
 Monterey Bay Aquarium Research Institute, all rights reserved.

 Terms of Use

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version. You can access the GPLv3 license at
 http://www.gnu.org/licenses/gpl-3.0.html

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details
 (http://www.gnu.org/licenses/gpl-3.0.html)

 MBARI provides the documentation and software code "as is", with no warranty,
 express or implied, as to the software, title, non-infringement of third party
 rights, merchantability, or fitness for any particular purpose, the accuracy of
 the code, or the performance or results which you may obtain from its use. You
 assume the entire risk associated with use of the code, and you agree to be
 responsible for the entire cost of repair or servicing of the program with
 which you are using the code.

 In no event shall MBARI be liable for any damages, whether general, special,
 incidental or consequential damages, arising out of your use of the software,
 including, but not limited to, the loss or corruption of your data or damages
 of any kind resulting from use of the software, any prohibited use, or your
 inability to use the software. You agree to defend, indemnify and hold harmless
 MBARI and its officers, directors, and employees against any claim, loss,
 liability or expense, including attorneys' fees, resulting from loss of or
 damage to property or the injury to or death of any person arising out of the
 use of the software.

 The MBARI software is provided without obligation on the part of the
 Monterey Bay Aquarium Research Institute to assist in its use, correction,
 modification, or enhancement.

 MBARI assumes no responsibility or liability for any third party and/or
 commercial software required for the database or applications. Licensee agrees
 to obtain and maintain valid licenses for any additional third party software
 required.
 */

/////////////////////////
// Headers
/////////////////////////

#include <getopt.h>
#include <signal.h>
#include <cstdlib>
#include <cstdio>
#include <cinttypes>
#include "trn_msg.h"
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

/////////////////////////
// Macros
/////////////////////////

// These macros should only be defined for
// application main files rather than general C files
/*
 /// @def PRODUCT
 /// @brief header software product name
 #define PRODUCT "TBD_PRODUCT"

 /// @def COPYRIGHT
 /// @brief header software copyright info
 #define COPYRIGHT "Copyright 2002-YYYY MBARI Monterey Bay Aquarium Research Institute, all rights reserved."
 /// @def NOWARRANTY
 /// @brief header software terms of use
 #define NOWARRANTY  \
 "This program is distributed in the hope that it will be useful,\n"\
 "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"\
 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"\
 "GNU General Public License for more details (http://www.gnu.org/licenses/gpl-3.0.html)\n"
 */
/// @def VERSION_HELPER
/// @brief version string helper.
#define VERSION_HELPER(s) #s
/// @def VERSION_STRING
/// @brief version string macro.
#define VERSION_STRING(s) VERSION_HELPER(s)

#ifndef TRACE
#define TRACE() fprintf(stderr,"%s:%d\n",__func__,__LINE__)
#endif

#define TRNCLI_UNIT_NAME "trncli-unit"
#ifndef TRNCLI_UNIT_VER
/// @def TRNCLI_UNIT_VER
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DTRNCLI_UNIT_VER=<version>
#define TRNCLI_UNIT_VER (dev)
#endif

#ifndef TRNCLI_UNIT_BUILD
/// @def TRNCLI_UNIT_BUILD
/// @brief MB1RS library build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DTRNCLI_UNIT_BUILD=`date`
#define TRNCLI_UNIT_BUILD "0000/00/00T00:00:00-0000"
#endif

#define TRNCLI_UNIT_VERSION_STR VERSION_STRING(TRNCLI_UNIT_VER)
#define TRNCLI_UNIT_BUILD_STR VERSION_STRING(TRNCLI_UNIT_BUILD)

#define PARSE_BOOL(s) (strcasecmp(s,"Y")==0 || strcasecmp(s,"1")==0 || strcasecmp(s,"TRUE")==0 ? true : false)
#define BOOLC_YN(b) (b ? 'Y' : 'N')
#define BOOLC_10(b) (b ? '1' : '0')
#define BOOLS_TF(b) (b ? "true" : "false")
#define TRN_DVL_BEAMS 4
#define TRN_MB_BEAMS 11
#define TRN_PENCIL_BEAMS 11
#define TRN_HOMER_BEAMS 4
#define TRN_DELTAT_BEAMS 128

/////////////////////////
// Declarations
/////////////////////////

bool g_quit=false;

#define RES_TYPE 0
#define RES_OFS_X 0
#define RES_OFS_Y 1
#define RES_OFS_Z 2
#define RES_SDEV_X 3
#define RES_SEDV_Y 4
#define RES_SDEV_Z 5
#define RES_DRIFT_RATE 0
#define RES_SET 0
#define RES_ALLOW 0
#define RES_USE 0

// TRN test resources
// test/message return values
typedef struct test_res_s{
    poseT *pt;
    poseT *mle;
    poseT *mse;
    measT *mt;
    int i_res[1];
    bool bool_res[1];
    double lf_res[6];
    d_triplet_t dtrip_res;
    InitVars init_vars;
}test_res_t;

typedef struct app_cfg_s
{
    char *trn_cfg;
    char *trn_host;
    long trn_port;
    // TRN log name prefix
    // i.e. TRN server logs written to
    //   TRN_LOGFILES/<log_id>-TRN.nn
    char *log_id;
    // sensor id (measT::dataType)
    //    TRN_SENSOR_DVL
    //    TRN_SENSOR_MB
    //    TRN_SENSOR_HOMER
    //    TRN_SENSOR_PENCIL
    //    TRN_SENSOR_DELTAT
    int sensor_type;
    // number of sensor beams (measT::numMeas)
    int sensor_beams;
    // TRN interp measurement attitude enable
    bool trn_ima;
    // interp method
    // 0: nearest-neighbor (no interpolation)
    // 1: bilinear
    // 2: bicubic
    // 3: spline
    // Default = 0;
    int trn_im;
    // vehicle drift rate
    double trn_vdr;
    // enable filter reinit
    bool trn_fren;
    // filter reinit lowInfoTransition
    bool trn_frl;
    // modified weighting algorihm
    // 0: TRN_WT_NONE     - No weighting modifications at all.
    // 1: TRN_WT_NORM     - Shandor's original alpha modification.
    // 2: TRN_WT_XBEAM    - Crossbeam with original
    // 3: TRN_WT_SUBCL    - Subcloud  with original
    // 4: TRN_FORCE_SUBCL - Forced to do Subcloud on every measurement
    // 5: TRN_WT_INVAL    - Any value here and above is invalid
    int trn_mw;
    int verbose;
    bool quit;
    TrnClient *trncli;
    // TRN remote instance (TRN server)
    TerrainNav *tnav;
    // test resources (hold test/message return values)
    test_res_t res;
}app_cfg_t;

static double s_etime();
// method test functions
static int s_test_estimatePose(app_cfg_t *cfg);
static int s_test_measUpdate(app_cfg_t *cfg);
static int s_test_motionUpdate(app_cfg_t *cfg);
static int s_test_outstandingMeas(app_cfg_t *cfg);
static int s_test_lastMeasSuccessful(app_cfg_t *cfg);
static int s_test_setInterpMeasAttitude(app_cfg_t *cfg);
static int s_test_setMapInterpMethod(app_cfg_t *cfg);
static int s_test_setVehicleDriftRate(app_cfg_t *cfg);
static int s_test_isConverged(app_cfg_t *cfg);
static int s_test_useLowGradeFilter(app_cfg_t *cfg);
static int s_test_useHighGradeFilter(app_cfg_t *cfg);
static int s_test_setFilterReinit(app_cfg_t *cfg);
static int s_test_setModifiedWeighting(app_cfg_t *cfg);
static int s_test_getFilterState(app_cfg_t *cfg);
static int s_test_getNumReinits(app_cfg_t *cfg);
static int s_test_reinitFilter(app_cfg_t *cfg);
static int s_test_reinitFilterOffset(app_cfg_t *cfg);
static int s_test_reinitFilterBox(app_cfg_t *cfg);
static int s_test_xEstNavOffset(app_cfg_t *cfg);
static int s_test_setEstNavOffset(app_cfg_t *cfg);
static int s_test_getEstNavOffset(app_cfg_t *cfg);
static int s_test_xInitStdDevXYZ(app_cfg_t *cfg);
static int s_test_setInitStdDevXYZ(app_cfg_t *cfg);
static int s_test_getInitStdDevXYZ(app_cfg_t *cfg);
static int s_test_setInitVars(app_cfg_t *cfg);
static int s_test_is_connected(app_cfg_t *cfg);

typedef int (* utest_fn)(struct app_cfg_s *);

// This table defines the test order
// duplicates are allowed
// (e.g. get/set/get to confirm defaults, changes
typedef struct fntab_s{
    utest_fn func;
    const char *name;
    int result;
}fntab_t;

fntab_t test_table[] = {
    {s_test_motionUpdate,"motionUpdate"},
    {s_test_measUpdate,"measUpdate"},
    {s_test_estimatePose,"estimatePose"},
    {s_test_outstandingMeas,"outstandingMeas"},

     {s_test_lastMeasSuccessful,"lastMeasSuccessful"},
     {s_test_setInterpMeasAttitude,"setInterpMeasAttitude"},
     {s_test_setMapInterpMethod,"setMapInterpMethod"},
     {s_test_setVehicleDriftRate,"setVehicleDriftRate"},

     {s_test_isConverged,"isConverged"},
     {s_test_useLowGradeFilter,"useLowGradeFilter"},
     {s_test_useHighGradeFilter,"useHighGradeFilter"},
     {s_test_setFilterReinit,"setFilterReinit"},

     {s_test_setModifiedWeighting,"setModifiedWeighting"},
     {s_test_getFilterState,"getFilterState"},
     {s_test_xInitStdDevXYZ,"xInitStdDevXYZ"},
     {s_test_xEstNavOffset,"xEstNavOffset"},

     {s_test_reinitFilter,"reinitFilter"},
     {s_test_getNumReinits,"getNumReinits"},
     {s_test_reinitFilterBox,"reinitFilterBox"},
     {s_test_reinitFilterOffset,"reinitFilterOffset"},
     {s_test_setInitVars,"setInitVars"},

     {s_test_is_connected,"is_connected"},
     {NULL,""}
};
utest_fn test_table_orig[] = {
    s_test_motionUpdate,
    s_test_measUpdate,
    s_test_estimatePose,
    s_test_outstandingMeas,

    s_test_lastMeasSuccessful,
    s_test_setInterpMeasAttitude,
    s_test_setMapInterpMethod,
    s_test_setVehicleDriftRate,

    s_test_isConverged,
    s_test_useLowGradeFilter,
    s_test_useHighGradeFilter,
    s_test_setFilterReinit,

    s_test_setModifiedWeighting,
    s_test_getFilterState,
    s_test_xInitStdDevXYZ,
    s_test_xEstNavOffset,

    s_test_reinitFilter,
    s_test_getNumReinits,
    s_test_reinitFilterBox,
    s_test_setInitVars,
    s_test_reinitFilterOffset,

    s_test_is_connected,
    NULL
};

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////

///////////////////////////
//// Function Definitions
///////////////////////////

static void s_show_help()
{
    char help_message[] = "\nTRN client unit test\n";
    char usage_message[] = "\ntrnclient-unit [options]\n"
    "--verbose      : verbose output\n"
    "--help         : output help message\n"
    "--version      : output version info\n"
    "--host=a[:p]   : TRN server host address, port\n"
    "--trn-cfg      : path to terrainAid.cfg\n"
    "--log-id=s     : TRN server log directory prefix (<log-id>-TRN.nn)\n"
    "--sensor=c     : TRN data (sensor) type\n"
    "                  [d1]:TRN_SENSOR_DVL\n"
    "                  [m2]:TRN_SENSOR_MB\n"
    "                  [p3]:TRN_SENSOR_PENCIL\n"
    "                  [h4]:TRN_SENSOR_HOMER\n"
    "                  [t5]:TRN_SENSOR_DELTAT-T\n"
    "--trn-mw=i     : set TRN modified weighting algorithm\n"
    "                   0: TRN_WT_NONE     - No weighting modifications at all."
    "                   1: TRN_WT_NORM     - Shandor's original alpha modification."
    "                   2: TRN_WT_XBEAM    - Crossbeam with original"
    "                   3: TRN_WT_SUBCL    - Subcloud  with original"
    "                   4: TRN_FORCE_SUBCL - Forced to do Subcloud on every measurement"
    "                   5: TRN_WT_INVAL    - Any value >= 5 is invalid"
    "                  0: TRN_WT_NONE"
    "--trn-im=i     : TRN interp method\n"
    "                   0: nearest-neighbor (no interpolation)"
    "                   1: bilinear"
    "                   2: bicubic"
    "                   3: spline"
    "--trn-ima=b    : set TRN interp measurement attitude\n"
    "--trn-vdr=d    : velocity drift rate\n"
    "--trn-fren=b   : filter reinit enable\n"
    "--trn-frl=b    : filter reinit lowInfoTransition\n"
    "\n";
    printf("%s",help_message);
    printf("%s",usage_message);
}

#define WIN_DECLSPEC

static void s_parse_args(int argc, char **argv, app_cfg_t *cfg)
{
    extern char WIN_DECLSPEC *optarg;
    int option_index;
    int c;
    bool help=false;
    bool version=false;
    char cmnem=0;
    static struct option options[] = {
        {"verbose", no_argument, NULL, 0},
        {"help", no_argument, NULL, 0},
        {"version", no_argument, NULL, 0},
        {"host", required_argument, NULL, 0},
        {"trn-cfg", required_argument, NULL, 0},
        {"log-id", required_argument, NULL, 0},
        {"sensor", required_argument, NULL, 0},
        {"trn-ima", required_argument, NULL, 0},
        {"trn-im", required_argument, NULL, 0},
        {"trn-vdr", required_argument, NULL, 0},
        {"trn-fren", required_argument, NULL, 0},
        {"trn_frl", required_argument, NULL, 0},
        {NULL, 0, NULL, 0}};
    while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1){
        char *scpy = NULL;
        char *shost = NULL;
        char *sport = NULL;
        switch (c) {
                // long options all return c=0
            case 0:
                if (strcmp("verbose", options[option_index].name) == 0) {
                    cfg->verbose=true;
                }

                // help
                else if (strcmp("help", options[option_index].name) == 0) {
                    help = true;
                }
                // version
                else if (strcmp("version", options[option_index].name) == 0) {
                    version = true;
                }

                // thost
                else if (strcmp("host", options[option_index].name) == 0) {
                    scpy = strdup(optarg);
                    shost = strtok(scpy,":");
                    sport = strtok(NULL,":");
                    if(NULL!=shost){
                        free(cfg->trn_host);
                        cfg->trn_host=strdup(shost);
                    }
                    if(NULL!=sport){
                        cfg->trn_port=atoi(sport);
                    }
                    free(scpy);
                }
                // trn_cfg
                else if (strcmp("trn-cfg", options[option_index].name) == 0) {
                    free(cfg->trn_cfg);
                    cfg->trn_cfg=strdup(optarg);
                }
                // log-id
                else if (strcmp("log-id", options[option_index].name) == 0) {
                    free(cfg->log_id);
                    cfg->log_id=strdup(optarg);
                    // remove trailing slash if found
                    char *slash = NULL;
                    if(NULL!=cfg->log_id && (slash = strrchr( cfg->log_id, '/' )) == (cfg->log_id+strlen(cfg->log_id)-1)){
                        *slash = '\0';
                    }
                }
                // sensor
                else if (strcmp("sensor", options[option_index].name) == 0) {
                    sscanf(optarg,"%c",&cmnem);
                    switch (cmnem) {
                        case 'd':
                        case 'D':
                        case '1':
                            cfg->sensor_type=TRN_SENSOR_DVL;
                            cfg->sensor_beams=TRN_DVL_BEAMS;
                            break;
                        case 'm':
                        case 'M':
                        case '2':
                            cfg->sensor_type=TRN_SENSOR_MB;
                            cfg->sensor_beams=TRN_MB_BEAMS;
                            break;
                        case 'p':
                        case 'P':
                        case '3':
                            cfg->sensor_type=TRN_SENSOR_PENCIL;
                            cfg->sensor_beams=TRN_PENCIL_BEAMS;
                            break;
                        case 'h':
                        case 'H':
                        case '4':
                            cfg->sensor_type=TRN_SENSOR_HOMER;
                            cfg->sensor_beams=TRN_HOMER_BEAMS;
                            break;
                        case 't':
                        case 'T':
                        case '5':
                            cfg->sensor_type=TRN_SENSOR_DELTAT;
                            cfg->sensor_beams=TRN_DELTAT_BEAMS;
                            break;
                        default:
                            fprintf(stderr,"invalid dtype[%c]\n",cmnem);
                            break;
                    }
                    cfg->res.mt->dataType=cfg->sensor_type;
                    cfg->res.mt->numMeas=cfg->sensor_beams;

                }
                // trn-ima
                else if (strcmp("trn-ima", options[option_index].name) == 0) {
                    cfg->trn_ima = PARSE_BOOL(optarg);
                }
                // trn-im
                else if (strcmp("trn-im", options[option_index].name) == 0) {
                    int test=-1;
                    if(sscanf(optarg,"%d",&test)==1 && test>=0 && test<=3){
                        cfg->trn_im = test;
                    }else{
                        fprintf(stderr,"WARN: invalid trn-im (expect 0-4)\n");
                    }
                }
                // trn-mw
                else if (strcmp("trn-mw", options[option_index].name) == 0) {
                    int test=-1;
                    if(sscanf(optarg,"%d",&test)==1 && test>=0 && test<=4){
                        cfg->trn_mw = test;
                    }else{
                        fprintf(stderr,"WARN: invalid trn-mw \n");
                    }
                }
                // trn-vdr
                else if (strcmp("trn-vdr", options[option_index].name) == 0) {
                    double test=0.;
                    if(sscanf(optarg,"%lf",&test)==1){
                        cfg->trn_vdr = test;
                    }else{
                        fprintf(stderr,"WARN: invalid trn-vdr (expect double)\n");
                    }
                }
                // trn-fren
                else if (strcmp("trn-fren", options[option_index].name) == 0) {
                    cfg->trn_fren = PARSE_BOOL(optarg);
                }
                // trn-frl
                else if (strcmp("trn-frl", options[option_index].name) == 0) {
                    cfg->trn_frl = PARSE_BOOL(optarg);
                }
                }// end switch

        if (version) {
            //            mbtrn_show_app_version(TCPC_NAME,TCPC_BUILD);
            fprintf(stderr,"%s ver[%s] build[%s]\n",TRNCLI_UNIT_NAME,TRNCLI_UNIT_VERSION_STR,TRNCLI_UNIT_BUILD_STR);
            exit(0);
        }
        if (help) {
            //            mbtrn_show_app_version(TCPC_NAME,TCPC_BUILD);
            s_show_help();
            exit(0);
        }
    }// while

    int wkey=10;
    int wval=16;
    fprintf(stderr,"%*s\n",wkey,"--- config options ---");
    fprintf(stderr,"%*s %*c\n",wkey,"verbose",wval,BOOLC_YN(cfg->verbose));
    fprintf(stderr,"%*s %*s\n",wkey,"host",wval,cfg->trn_host);
    fprintf(stderr,"%*s %*ld\n",wkey,"port",wval,cfg->trn_port);
    fprintf(stderr,"%*s %*s\n",wkey,"trn_cfg",wval,cfg->trn_cfg);
    fprintf(stderr,"%*s %*s\n",wkey,"log-id",wval,cfg->log_id);
    fprintf(stderr,"%*s %*d\n",wkey,"sensor",wval,cfg->sensor_type);
    fprintf(stderr,"%*s %*d\n",wkey,"trn-mw ",wval,cfg->trn_mw);
    fprintf(stderr,"%*s %*c\n",wkey,"trn-ima",wval,BOOLC_YN(cfg->trn_ima));
    fprintf(stderr,"%*s %*d\n",wkey,"trn-im ",wval,cfg->trn_im);
    fprintf(stderr,"%*s %*lf\n",wkey,"trn-vdr ",wval,cfg->trn_vdr);
    fprintf(stderr,"%*s %*c\n",wkey,"trn-fren",wval,BOOLC_YN(cfg->trn_fren));
    fprintf(stderr,"%*s %*c\n",wkey,"trn-frl",wval,BOOLC_YN(cfg->trn_frl));

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

static app_cfg_t *s_app_cfg_new()
{
    app_cfg_t *instance = (app_cfg_t *)malloc(sizeof(app_cfg_t));
    if(NULL!=instance){
        memset((void *)instance,0,sizeof(app_cfg_t));
        instance->trn_cfg=NULL;
        instance->trn_host=strdup("127.0.0.1");
        instance->trn_port=27027;
        instance->verbose=0;
        instance->trncli=NULL;
        instance->tnav=NULL;
        instance->quit=false;
        instance->log_id=strdup("tcu");

        instance->sensor_type=TRN_SENSOR_DVL;
        instance->sensor_beams=TRN_DVL_BEAMS;
        instance->trn_mw=0;
        instance->trn_ima=true;
        instance->trn_im=0;
        instance->trn_vdr=0.0;
        instance->trn_fren=true;
        instance->trn_frl=true;

        instance->res.pt = new poseT;
        instance->res.mle = new poseT;
        instance->res.mse = new poseT;
        instance->res.mt = new measT;

        instance->res.mt->dataType   = TRN_SENSOR_DVL;
        instance->res.mt->numMeas    = TRN_DVL_BEAMS;
        instance->res.mt->time       = s_etime();
        instance->res.mt->x          = 0.0;
        instance->res.mt->y          = 0.0;
        instance->res.mt->z          = 0.0;
        instance->res.mt->phi        = 0.0;
        instance->res.mt->theta      = 0.0;
        instance->res.mt->phi        = 0.0;
        instance->res.mt->covariance = new double[TRN_MAX_BEAMS];
        instance->res.mt->ranges     = new double[TRN_MAX_BEAMS];
        instance->res.mt->crossTrack = new double[TRN_MAX_BEAMS];
        instance->res.mt->alongTrack = new double[TRN_MAX_BEAMS];
        instance->res.mt->beamNums   = new int[TRN_MAX_BEAMS];
        instance->res.mt->altitudes  = new double[TRN_MAX_BEAMS];
        instance->res.mt->alphas     = new double[TRN_MAX_BEAMS];
        instance->res.mt->measStatus = new bool[TRN_MAX_BEAMS];
    }
    return instance;
}

static void s_app_cfg_destroy(app_cfg_t **pself)
{
    if(NULL!=pself){
        app_cfg_t *self = *pself;
        if(NULL!=self){

            free(self->log_id);
            free(self->trn_cfg);
            free(self->trn_host);
            delete self->trncli;
            // self->tnav released by TRNClient
//             delete self->tnav;

            delete self->res.pt;
            delete self->res.mle;
            delete self->res.mse;
            delete self->res.mt;

            free(self);
        }
        *pself = NULL;
    }
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

static int s_test_estimatePose(app_cfg_t *cfg)
{
    int retval=0;
    //poseT* estimate, const int &type
    cfg->trncli->estimatePose(cfg->res.pt, cfg->sensor_type );
    fprintf(stderr,"%s: ret[%d]\n",__func__,retval);
    return retval;
}

static int s_test_measUpdate(app_cfg_t *cfg)
{
    int retval=0;
    //measT* incomingMeas, const int &type
    cfg->trncli->measUpdate(cfg->res.mt, cfg->sensor_type );
    fprintf(stderr,"%s: ret[%d]\n",__func__,retval);
    return retval;
}

static int s_test_motionUpdate(app_cfg_t *cfg)
{
    int retval=0;
    //poseT* incomingNav
    cfg->trncli->motionUpdate(cfg->res.pt);
    fprintf(stderr,"%s: ret[%d]\n",__func__,retval);
    return retval;
}

static int s_test_outstandingMeas(app_cfg_t *cfg)
{
    int retval=-1;
    bool test = cfg->trncli->outstandingMeas();
    retval = (test ? -1 : 0);
    fprintf(stderr,"%s: test[%c] ret[%d]\n",__func__,BOOLC_YN(test),retval);
    return retval;
}

static int s_test_lastMeasSuccessful(app_cfg_t *cfg)
{
    int retval=-1;
    bool test = cfg->trncli->lastMeasSuccessful();
    retval = (test ? -1 : 0);
    fprintf(stderr,"%s: test[%c] ret[%d]\n",__func__,BOOLC_YN(test),retval);
    return retval;
}

static int s_test_setInterpMeasAttitude(app_cfg_t *cfg)
{
    int retval=0;
    //bool set
    cfg->trncli->setInterpMeasAttitude(cfg->trn_ima);
    fprintf(stderr,"%s: ret[%d]\n",__func__,retval);
    return retval;
}

static int s_test_setMapInterpMethod(app_cfg_t *cfg)
{
    int retval=0;
    //const int &type
    // interp method
    // 0: nearest-neighbor (no interpolation)
    // 1: bilinear
    // 2: bicubic
    // 3: spline
    // Default = 0;
    cfg->trncli->setMapInterpMethod(cfg->trn_im);
    fprintf(stderr,"%s: ret[%d]\n",__func__,retval);
    return retval;
}

static int s_test_setVehicleDriftRate(app_cfg_t *cfg)
{
    int retval=0;
    //const double &driftRate
    cfg->trncli->setVehicleDriftRate(cfg->trn_vdr);
    fprintf(stderr,"%s: ret[%d]\n",__func__,retval);
    return retval;
}

static int s_test_isConverged(app_cfg_t *cfg)
{
    int retval=0;
    bool test = cfg->trncli->isConverged();
    retval = (test ? -1 : 0);
    fprintf(stderr,"%s: test[%c] ret[%d]\n",__func__,BOOLC_YN(test),retval);
    return retval;
}

static int s_test_useLowGradeFilter(app_cfg_t *cfg)
{
    int retval=0;
    cfg->trncli->useLowGradeFilter();
    fprintf(stderr,"%s: ret[%d]\n",__func__,retval);
    return retval;
}

static int s_test_useHighGradeFilter(app_cfg_t *cfg)
{
    int retval=0;
    cfg->trncli->useHighGradeFilter();
    fprintf(stderr,"%s: ret[%d]\n",__func__,retval);
    return retval;
}

static int s_test_setFilterReinit(app_cfg_t *cfg)
{
    int retval=0;
    //const bool allow
    cfg->trncli->setFilterReinit(cfg->trn_fren);
    fprintf(stderr,"%s: ret[%d]\n",__func__,retval);
    return retval;
}

static int s_test_setModifiedWeighting(app_cfg_t *cfg)
{
    int retval=0;
    //const int use
    cfg->trncli->setModifiedWeighting(cfg->trn_mw);
    fprintf(stderr,"%s: ret[%d]\n",__func__,retval);
    return retval;
}

static int s_test_getFilterState(app_cfg_t *cfg)
{
    int retval=0;
    int test = cfg->trncli->getFilterState();
    retval = (test!=0 ? -1 : 0);
    fprintf(stderr,"%s: test[%d] ret[%d]\n",__func__,test,retval);
    return retval;
}

static int s_test_getNumReinits(app_cfg_t *cfg)
{
    int retval=0;
    int test = cfg->trncli->getNumReinits();
    retval = (test!=1 ? -1 : 0);
    fprintf(stderr,"%s: test[%d] ret[%d]\n",__func__,test,retval);
    return retval;
}

static int s_test_reinitFilter(app_cfg_t *cfg)
{
    int retval=0;
    //bool lowInfoTransition
    cfg->trncli->reinitFilter(cfg->trn_frl);
    fprintf(stderr,"%s: ret[%d]\n",__func__,retval);
    return retval;
}

static int s_test_reinitFilterOffset(app_cfg_t *cfg)
{
    int retval=0;
    //const bool lowInfoTransition,
    //double offset_x, double offset_y, double offset_z,
    //double sdev_x, double sdev_y, double sdev_z
    cfg->trncli->reinitFilterOffset(true,5.,5.,5.);
    fprintf(stderr,"%s: ret[%d]\n",__func__,retval);
    return retval;
}

static int s_test_reinitFilterBox(app_cfg_t *cfg)
{
    int retval=0;
    //const bool lowInfoTransition,
    //double offset_x, double offset_y, double offset_z,
    //double sdev_x, double sdev_y, double sdev_z
    cfg->trncli->reinitFilterBox(true,3.,3.,3.,4.,4.,4.);
    fprintf(stderr,"%s: ret[%d]\n",__func__,retval);
    return retval;
}

static int s_test_setEstNavOffset(app_cfg_t *cfg)
{
    int retval=0;
    //double offset_x, double offset_y, double offset_z
    cfg->trncli->setEstNavOffset(1.,1.,1.);
    fprintf(stderr,"%s: ret[%d]\n",__func__,retval);
    return retval;
}

static int s_test_getEstNavOffset(app_cfg_t *cfg)
{
    int retval=0;
    //d_triplet_t *dest
    cfg->trncli->getEstNavOffset(&cfg->res.dtrip_res);
    fprintf(stderr,"%s: test[%lf,%lf,%lf] ret[%d]\n",__func__,
            cfg->res.dtrip_res.x,cfg->res.dtrip_res.y,cfg->res.dtrip_res.z,retval);
    return retval;
}

static int s_test_xEstNavOffset(app_cfg_t *cfg)
{
    int retval=0;
    bool err=false;

    // get/set/get should return default, then set value
    int x = s_test_getEstNavOffset(cfg);
    err = (x != 0 ||
           cfg->res.dtrip_res.x!=0.0 ||
           cfg->res.dtrip_res.y!=0.0 ||
           cfg->res.dtrip_res.z!=0.0);

    x = s_test_setEstNavOffset(cfg);
    err = (err || x!=0);

    x = s_test_getEstNavOffset(cfg);
    err = (err ||
           x!=0 ||
           cfg->res.dtrip_res.x!=1.0 ||
           cfg->res.dtrip_res.y!=1.0 ||
           cfg->res.dtrip_res.z!=1.0);

    retval = (!err ? 0 : retval);
    fprintf(stderr,"%s: test[%lf,%lf,%lf] err[%c] ret[%d]\n",__func__,
            cfg->res.dtrip_res.x,cfg->res.dtrip_res.y,cfg->res.dtrip_res.z,BOOLC_YN(err),retval);
    return retval;
}

static int s_test_setInitStdDevXYZ(app_cfg_t *cfg)
{
    int retval=0;
    //double sdev_x, double sdev_y, double sdev_z
    cfg->trncli->setInitStdDevXYZ(2.,2.,2.);
    fprintf(stderr,"%s: ret[%d]\n",__func__,retval);
    return retval;
}

static int s_test_getInitStdDevXYZ(app_cfg_t *cfg)
{
    int retval=0;
    //d_triplet_t *dest
    cfg->trncli->getInitStdDevXYZ(&cfg->res.dtrip_res);
    fprintf(stderr,"%s: test[%lf,%lf,%lf] ret[%d]\n",__func__,
            cfg->res.dtrip_res.x,cfg->res.dtrip_res.y,cfg->res.dtrip_res.z,retval);
    return retval;
}

static int s_test_xInitStdDevXYZ(app_cfg_t *cfg)
{
    int retval=0;
    //d_triplet_t *dest
     bool err=false;

    int x = s_test_getInitStdDevXYZ(cfg);
    err = (x!=0 ||
           cfg->res.dtrip_res.x!=X_STDDEV_INIT ||
           cfg->res.dtrip_res.y!=Y_STDDEV_INIT ||
           cfg->res.dtrip_res.z!=Z_STDDEV_INIT);

    x = s_test_setInitStdDevXYZ(cfg);
    err = (err ||
           x != 0);

    x = s_test_getInitStdDevXYZ(cfg);
    err = (err ||
           x!=0 ||
           cfg->res.dtrip_res.x!=2.0 ||
           cfg->res.dtrip_res.y!=2.0 ||
           cfg->res.dtrip_res.z!=2.0);

    retval = (!err ? 0 : retval);
    fprintf(stderr,"%s: test[%lf,%lf,%lf] err[%c] ret[%d]\n",__func__,
            cfg->res.dtrip_res.x,cfg->res.dtrip_res.y,cfg->res.dtrip_res.z,BOOLC_YN(err),retval);
    return retval;
}

static int s_test_setInitVars(app_cfg_t *cfg)
{
    int retval=0;
    //InitVars *init_vars
    cfg->trncli->setInitVars(&cfg->res.init_vars);
    fprintf(stderr,"%s: ret[%d]\n",__func__,retval);
    return retval;
}

static int s_test_is_connected(app_cfg_t *cfg)
{
    int retval=-1;
    bool test = cfg->trncli->is_connected();
    retval = ( test ? 0 : -1);
    fprintf(stderr,"%s: test[%c] ret[%d]\n",__func__,BOOLC_YN(test),retval);
    return retval;
}

int app_main(app_cfg_t *cfg, uint16_t *r_count, uint64_t *r_stat)
{
    int retval=-1;
    uint16_t cur=0;
    uint64_t eflags=0x0;

    // Create and initialize the TRNCient instance
    cfg->trncli = new TrnClient(cfg->log_id, cfg->trn_host, cfg->trn_port);
    cfg->trncli->setVerbose(cfg->verbose);

    // Load TRN configuration from config (or use defaults if NULL)
    // the loaded config will be passed to the TRN server
    // and used by this app
    // The server initialization will fail unless the correct
    // map and vehicle configuration files are present on the server.

    cfg->trncli->loadCfgAttributes(cfg->trn_cfg);

    // Open connection to the TRN server.
    // The TRN instance is created by trn_server using
    // values in TNavConfig (set in loadCfgAttributes

    if ((cfg->tnav = cfg->trncli->connectTRN()) != NULL){

        // run the tests in the order defined in test table
        fntab_t *p_test = &test_table[0];
        while( (*p_test->func) != NULL){
            if( (p_test->result = (*p_test->func)(cfg)) != 0 ) {
                eflags |= (1<<cur);

            }
            cur++;
            p_test++;
        }

        // set return values
        if(NULL!=r_count) *r_count=cur;
        if(NULL!=r_stat) *r_stat=eflags;

    }else{
        fprintf(stderr," TRN server connection failed.\n");
        retval = -1;
    }
    fprintf(stderr,"\ntests complete\n");
    return retval;
}

int main(int argc, char** argv)
{
    // handle CTRL-C
    signal(SIGINT,s_sig_handler);

    // get app_config
    app_cfg_t *cfg=s_app_cfg_new();

    // parse command line options
    s_parse_args(argc, argv, cfg);

    uint64_t stat=0x0;
    uint16_t count=0;
    app_main(cfg, &count, &stat);

    fprintf(stderr,"\nreleasing resources...\n");
    // release config instance resources
    s_app_cfg_destroy(&cfg);
    // TNavConfig is a singleton; invoke w/ true release
    TNavConfig::instance(true);

    // print summary
    fprintf(stderr,"\n---Test Summary\n");
    fprintf(stderr,"%*s : n[%3hu] : [x%06" PRIX64 "]\n",24,"tests complete",count, stat);
    fntab_t *p_test = &test_table[0];
    uint16_t cur=0;
    while( (*p_test->func) != NULL){
        fprintf(stderr,"%*s : %06" PRIX64 " : [%3s]\n",24,p_test->name,
                (uint64_t)(1<<cur),
                (p_test->result==0?"OK":"ERR"));
        cur++;
        p_test++;
    }

    fprintf(stderr,"\n\n");
    return 0;
}

