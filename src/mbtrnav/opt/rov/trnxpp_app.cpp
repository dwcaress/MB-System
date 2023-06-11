
/// @file trnxpp_app.cpp
/// @authors k. headley
/// @date 21mar2022

/// Summary: app code for trnxpp (LCM TRN preprocessing for ROVs)

// ///////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute
// Distributed under MIT license. See LICENSE file for more information.

// /////////////////
// Includes
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <math.h>
#include <string.h>
#include <memory.h>
#include <libgen.h>
// initalizers for header-only modules
// must be defined before any framework headers are included
#define INIT_PCF_LOG
#define INIT_TRN_LCM_INPUT
#include "lcm_interface.hpp"
#include "lcm_pcf/signal_t.hpp"
#include "trn_lcm_input.hpp"
#include "raw_signal_input.hpp"
#include "dvl_stat_input.hpp"
#include "rdi_dvl_input.hpp"
#include "nav_solution_input.hpp"
#include "idt_input.hpp"
#include "pcomms_input.hpp"
#include "rdi_pd4_input.hpp"
#include "kearfott_input.hpp"
#include "octans_input.hpp"
#include "trnxpp.hpp"
#include "mb1_server.hpp"
#include "NavUtils.h"
#include "trn_msg_utils.hpp"
#include "trn/trn_pose_t.hpp"
#include "udpm_sub.h"
#include "newmatap.h"
#include "newmatio.h"
#include "trn_debug.hpp"
#include "log_utils.hpp"

// /////////////////
// Macros
/// @def VERSION_HELPER
/// @brief version string helper.
#define VERSION_HELPER(s) #s
/// @def VERSION_STRING
/// @brief version string macro.
#define VERSION_STRING(s) VERSION_HELPER(s)

#define TRNXPP_NAME "trnxpp"
#ifndef TRNXPP_BUILD
/// @def TRNXPP_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMBTRN_BUILD=`date`
#define TRNXPP_BUILD "" VERSION_STRING(APP_BUILD)
#endif
#ifndef TRNXPP_VERSION
/// @def TRNXPP_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMBTRN_BUILD=`date`
#define TRNXPP_VERSION "" VERSION_STRING(TRNXPP_VER)
#endif

/// @def WIN_DECLSPEC
/// @brief declaration for windows.
//#ifdef _WIN64
////define something for Windows (64-bit only)
//#       define WIN_DECLSPEC __declspec(dllimport)
//#else // is WIN32
////define something for Windows (32-bit only)
//#       define WIN_DECLSPEC __declspec(dllimport)
//#endif // WIN64

#if defined(__CYGWIN__)
/// @def WIN_DECLSPEC
/// @brief declaration for windows.
#define WIN_DECLSPEC __declspec(dllimport)
#else
#define WIN_DECLSPEC
#endif


#ifndef DTR
#define DTR(x) ((x) * M_PI/180.)
#endif
#ifndef RTD
#define RTD(x) ((x) * 180./M_PI)
#endif

// /////////////////
// Types
class mbgeo
{
public:
    static const int MBG_PDEG=0;

    mbgeo()
    :beam_count(0.),swath_deg(0.)
    {
        size_t asz = 3 * sizeof(double);
        memset(svr_deg, 0, asz);
        memset(svt_m, 0, asz);
    }

    mbgeo(uint16_t nbeams, double swath, double *rot, double *tran)
    :beam_count(nbeams),swath_deg(swath)
    {
        for(int i=0; i<3; i++)
        {
            if(rot != NULL)
                svr_deg[i] = rot[i];
            if(tran != NULL)
                svt_m[i] = tran[i];
        }
    }

    ~mbgeo(){}

    void tostream(std::ostream &os, int wkey=15, int wval=18)
    {
        os << std::setw(wkey) << "beam_count";
        os << std::setw(wval) << beam_count << "\n";
        os << std::setw(wkey) << "swath";
        os << std::setw(wval) << swath_deg << "\n";
        os << std::setw(wkey) << "rotation" ;
        os << std::setw(wval) << "[";
        os << svr_deg[0] << "," << svr_deg[1] << "," << svr_deg[2] << "]\n";
        os << std::setw(wkey) << "translation";
        os << std::setw(wval) << "[";
        os << svt_m[0] << "," << svt_m[1] << "," << svt_m[2] << "]";
    }

    std::string tostring(int wkey=15, int wval=18)
    {
        std::ostringstream ss;
        tostream(ss, wkey, wval);
        return ss.str();
    }

    void show(int wkey=15, int wval=18)
    {
        tostream(std::cerr, wkey, wval);
    }

    // number of beams
    uint16_t beam_count;
    // angle between first and last beam
    double swath_deg;
    // sensor rotation relative to vehicle CRP (r/p/y  aka phi/theta/psi deg)
    double svr_deg[3];
    // sensor translation relative to vehicle CRP (x/y/z m)
    // +x: fwd +y: stbd, +z:down
    double svt_m[3];

};

class dvlgeo
{
public:


    dvlgeo()
    :beam_count(0), yaw_rf(NULL), pitch_rf(NULL)
    {
        memset(svr_deg,0,3*sizeof(double));
        memset(svt_m,0,3*sizeof(double));
        fprintf(stderr,"%s:%d this[%p] yrf[%p] prf[%p]\n",__func__,__LINE__,this, yaw_rf,pitch_rf);
        yaw_rf = NULL;
        pitch_rf = NULL;
        fprintf(stderr,"%s:%d this[%p] yrf[%p] prf[%p]\n",__func__,__LINE__,this, yaw_rf,pitch_rf);
    }

    dvlgeo(uint16_t nbeams, const char *bspec, double *rot, double *tran)
    :beam_count(nbeams), yaw_rf(NULL), pitch_rf(NULL)
    {
        for(int i=0; i<3; i++)
        {
            if(rot != NULL)
                svr_deg[i] = rot[i];
            if(tran != NULL)
                svt_m[i] = tran[i];
        }

        beam_count = nbeams;
        size_t asz = sizeof(double) * nbeams;
        yaw_rf = (double *)malloc(asz);
        pitch_rf = (double *)malloc(asz);
        memset(yaw_rf, 0, asz);
        memset(pitch_rf, 0, asz);
        parse_bspec(bspec);
    }

    ~dvlgeo()
    {
        free(yaw_rf);
        free(pitch_rf);
    }

    void parse_bspec(const char *bspec)
    {
        if(NULL == bspec)
            return;

        char *spec = strdup(bspec);
        if(spec[0] == 'A'){
            double yb_deg = 0;
            double yi_deg = 0;
            double pb_deg = 0;
            double pi_deg = 0;
            if(sscanf(spec, "A,%lf,%lf,%lf,%lf",&yb_deg,&yi_deg,&pb_deg,&pi_deg ) == 4){
                for(int i=0 ; i<beam_count; i++){
                    yaw_rf[i] = yb_deg + i * yi_deg;
                    pitch_rf[i] = pb_deg + i * pi_deg;
                }
            } else {
                fprintf(stderr, "ERR - invalid auto beam spec [%s]\n",spec);
            }
        } else if(spec[0] == 'L') {
            char *scpy = strdup(spec);
            char *junk = strtok(scpy,",");
            if(NULL != junk){
                for(int i=0; i<beam_count; i++){
                    char *next_y = strtok(NULL,",");
                    char *next_p = strtok(NULL,",");

                    if(next_y==NULL){
                        fprintf(stderr, "ERR - not enough tokens [%s]\n",spec);
                        break;
                    }
                    if(next_p==NULL){
                        fprintf(stderr, "ERR - not enough tokens [%s]\n",spec);
                        break;
                    }
                    if(sscanf(next_y,"%lf",&yaw_rf[i])!=1){
                        fprintf(stderr, "ERR - Y[%d] invalid [%s]\n",i,next_y);
                    }
                    if(sscanf(next_p,"%lf",&pitch_rf[i])!=1){
                        fprintf(stderr, "ERR - P[%d] invalid [%s]\n",i,next_p);
                    }
               }
            } else {
                fprintf(stderr, "ERR - not enough tokens [%s]\n",spec);
            }
            free(scpy);
        } else {
            fprintf(stderr, "ERR - unsupported beam spec type [%s]\n",spec);
        }

        for(int i=0 ; i<beam_count; i++){
            // normalize yaw to 0 : 360
            if(yaw_rf[i] < 0.)
                yaw_rf[i] = (fmod(yaw_rf[i],360.)) + 360.;
            if(yaw_rf[i] > 360.)
                yaw_rf[i] = fmod(yaw_rf[i],360.);
            // normalize pitch to -90 : 90
            pitch_rf[i] = fmod(pitch_rf[i],90.);
        }

        free(spec);
    }

    void tostream(std::ostream &os, int wkey=15, int wval=18)
    {
        os << std::setw(wkey) << "beam_count";
        os << std::setw(wval) << beam_count << "\n";
        os << std::setw(wkey) << "rotation" ;
        os << std::setw(wval) << "[";
        os << svr_deg[0] << "," << svr_deg[1] << "," << svr_deg[2] << "]\n";
        os << std::setw(wkey) << "translation";
        os << std::setw(wval) << "[";
        os << svt_m[0] << "," << svt_m[1] << "," << svt_m[2] << "]";
        if(beam_count > 0){
        os << std::setw(wkey) << "beam angles (Yi,Pi)";
        os << std::setw(wval) << "[";
        for(int i=0; i<beam_count;i++){
            os << (yaw_rf!=NULL?yaw_rf[i]:-1.) << "," << (pitch_rf!=NULL?pitch_rf[i]:-1.);
            if(i!=beam_count-1)
                os << ",";
        }
        os << std::setw(wval) << "]\n";
        }
        os << "\n";
    }

    std::string tostring(int wkey=15, int wval=18)
    {
        std::ostringstream ss;
        tostream(ss, wkey, wval);
        return ss.str();
    }

    void show(int wkey=15, int wval=18)
    {
        tostream(std::cerr, wkey, wval);
    }

    // number of beams
    uint16_t beam_count;
    // sensor rotation relative to vehicle CRP (r/p/y  aka phi/theta/psi deg)
    double svr_deg[3];
    // sensor translation relative to vehicle CRP (x/y/z m)
    // +x: fwd +y: stbd, +z:down
    double svt_m[3];
    // transducer yaw angles (in sensor reference frame, deg)
    double *yaw_rf;
    // transducer pitch angles (in sensor reference frame, deg)
    double *pitch_rf;
};

class app_stats
{
public:
    app_stats()
    : start_time(0), end_time(0), cycle_n(0),
    sem_test_n(0), sem_call_n(0), sem_err_n(0),
    trn_cb_n(0), trn_motn_n(0), trn_meas_n(0),trn_mle_n(0), trn_mmse_n(0),
    trn_csv_n(0), trn_est_val_n(0), trn_pub_motn_n(0), trn_pub_meas_n(0),
    trn_pub_est_n(0), trn_pub_stat_n(0), trn_est_ok_n(0), trn_err_n(0), trn_cli_con(0), trn_cli_dis(0),
    mb_cb_n(0), mb_pub_n(0), mb_csv_n(0), mb_pub_mb1_n(0),
    mb_pub_est_n(0), mb_est_n(0), mb_est_ok_n(0), mb_err_n(0),
    mb_cli_con(0), mb_cli_dis(0)
    {}

    ~app_stats()
    {}

    void tostream(std::ostream &os, int wkey=15, int wval=18)
    {
        os << std::dec << std::setfill(' ');
        os << "--- stats ---" << "\n";
        os << std::setw(wkey) << "addr" << std::setw(wval) << this <<"\n";
        os << std::fixed << std::setprecision(3) << std::setfill(' ');
        os << std::setw(wkey) << "start_time" << std::setw(wval) << start_time <<"\n";
        os << std::setw(wkey) << "end_time" << std::setw(wval) << end_time <<"\n";
        os << std::dec;
        os << std::setw(wkey) << "cycle_n" << std::setw(wval) << cycle_n <<"\n";
        os << std::setw(wkey) << "sem_test_n" << std::setw(wval) << sem_test_n <<"\n";
        os << std::setw(wkey) << "sem_call_n" << std::setw(wval) << sem_call_n <<"\n";
        os << std::setw(wkey) << "sem_err_n" << std::setw(wval) << sem_err_n <<"\n";
        os << std::setw(wkey) << "trn_cb_n" << std::setw(wval) << trn_cb_n <<"\n";
        os << std::setw(wkey) << "trn_motn_n" << std::setw(wval) << trn_motn_n <<"\n";
        os << std::setw(wkey) << "trn_meas_n" << std::setw(wval) << trn_meas_n <<"\n";
        os << std::setw(wkey) << "trn_mle_n" << std::setw(wval) << trn_mle_n <<"\n";
        os << std::setw(wkey) << "trn_mmse_n" << std::setw(wval) << trn_mmse_n <<"\n";
        os << std::setw(wkey) << "trn_csv_n" << std::setw(wval) << trn_csv_n <<"\n";
        os << std::setw(wkey) << "trn_est_val_n" << std::setw(wval) << trn_est_val_n <<"\n";
        os << std::setw(wkey) << "trn_pub_motn_n" << std::setw(wval) << trn_pub_motn_n <<"\n";
        os << std::setw(wkey) << "trn_pub_meas_n" << std::setw(wval) << trn_pub_meas_n <<"\n";
        os << std::setw(wkey) << "trn_pub_est_n" << std::setw(wval) << trn_pub_est_n <<"\n";
        os << std::setw(wkey) << "trn_pub_stat_n" << std::setw(wval) << trn_pub_stat_n <<"\n";
        os << std::setw(wkey) << "trn_est_ok_n" << std::setw(wval) << trn_est_ok_n <<"\n";
        os << std::setw(wkey) << "trn_err_n" << std::setw(wval) << trn_err_n <<"\n";
        os << std::setw(wkey) << "trn_cli_con" << std::setw(wval) << trn_cli_con <<"\n";
        os << std::setw(wkey) << "trn_cli_dis" << std::setw(wval) << trn_cli_dis <<"\n";
        os << std::setw(wkey) << "mb_cb_n" << std::setw(wval) << mb_cb_n <<"\n";
        os << std::setw(wkey) << "mb_pub_n" << std::setw(wval) << mb_pub_n <<"\n";
        os << std::setw(wkey) << "mb_csv_n" << std::setw(wval) << mb_csv_n <<"\n";
        os << std::setw(wkey) << "mb_pub_mb1_n" << std::setw(wval) << mb_pub_mb1_n <<"\n";
        os << std::setw(wkey) << "mb_pub_est_n" << std::setw(wval) << mb_pub_est_n <<"\n";
        os << std::setw(wkey) << "mb_est_n" << std::setw(wval) << mb_est_n <<"\n";
        os << std::setw(wkey) << "mb_est_ok_n" << std::setw(wval) << mb_est_ok_n <<"\n";
        os << std::setw(wkey) << "mb_err_n" << std::setw(wval) << mb_err_n <<"\n";
        os << std::setw(wkey) << "mb_cli_con" << std::setw(wval) << mb_cli_con <<"\n";
        os << std::setw(wkey) << "mb_cli_dis" << std::setw(wval) << mb_cli_dis <<"\n";
    }

    std::string tostring(int wkey=15, int wval=18)
    {
        std::ostringstream ss;
        tostream(ss, wkey, wval);
        return ss.str();
    }

    void show(int wkey=15, int wval=18)
    {
        tostream(std::cerr, wkey, wval);
    }

    double start_time;
    double end_time;
    int cycle_n;
    int sem_test_n;
    int sem_call_n;
    int sem_err_n;

    int trn_cb_n;
    int trn_motn_n;
    int trn_meas_n;
    int trn_mle_n;
    int trn_mmse_n;
    int trn_csv_n;
    int trn_est_val_n;
    int trn_pub_motn_n;
    int trn_pub_meas_n;
    int trn_pub_est_n;
    int trn_pub_stat_n;
    int trn_est_ok_n;
    int trn_err_n;
    int trn_cli_con;
    int trn_cli_dis;

    int mb_cb_n;
    int mb_pub_n;
    int mb_csv_n;
    int mb_pub_mb1_n;
    int mb_pub_est_n;
    int mb_est_n;
    int mb_est_ok_n;
    int mb_err_n;
    int mb_cli_con;
    int mb_cli_dis;
};

class app_cfg
{
public:
    app_cfg()
    : mVerbose(false), mDebug(0), mCycles(-1), mHost(NULL),
    mPort(MB1SVR_PORT_DFL), mTrnuGroup(NULL), mTrnuPort(UDPMS_MCAST_PORT_DFL), mTrnuTTL(1), mDelay(0),
    mFakeMB1(false), mPubTrnEst(false), mPubTrnMotn(false), mPubTrnMeas(false), mPubMB1(false),
    mPubMBEst(false), mTrnDecN(1), mMBGeo(NULL), mDVLGeo(NULL), mTrnCsv(),mMB1Csv(),mLogDirStr("."), mMsgLog(), mStats(),
    mStatPeriod(), mConfigSet(false)
    {
        mHost = strdup(MB1SVR_HOST_DFL);
        mTrnuGroup = strdup(UDPMS_GROUP_DFL);
        char session_string[64]={0};

        auto now = std::chrono::system_clock::now();

        // convert the time to a time_t type
        std::time_t now_t = std::chrono::system_clock::to_time_t(now);

        // create a buffer with YYYYMMDD-HHMMSS format
        // note: not using std::put_time as it was broken until GCC 5.0,
        // best to use strftime for portability
        std::strftime(session_string, sizeof(session_string), "%Y%m%d-%H%M%S",
                      std::localtime(&now_t));

        // create a formatted time stamp string
        std::stringstream ss;

        ss << std::string(session_string);

        mSessionStr = ss.str();
    }

    ~app_cfg()
    {
        free(mHost);

        free(mTrnuGroup);

        if(mDVLGeo!=NULL)
            delete mDVLGeo;

        if(mMBGeo!=NULL)
            delete mMBGeo;

        while (mInputList.size()>0)
            mInputList.pop_front();
    }

    void parse_args(int argc, char **argv)
    {
        extern char WIN_DECLSPEC *optarg;
        int option_index=0;
        int c;
        bool help=false;
        bool version=false;
        static struct option options[] =
        {
            {"verbose", no_argument, NULL, 0},
            {"debug", required_argument, NULL, 0},
            {"help", no_argument, NULL, 0},
            {"cycles", required_argument, NULL, 0},
            {"version", no_argument, NULL, 0},
            {"host", required_argument, NULL, 0},
            {"delay", required_argument, NULL, 0},
            {"stats", required_argument, NULL, 0},
            {"logdir", required_argument, NULL, 0},
            {"fake-mb1", no_argument, NULL, 0},
            {"trn-csv", required_argument, NULL, 0},
            {"mb1-csv", required_argument, NULL, 0},
            {"trn-cfg", required_argument, NULL, 0},
            {"trn-decn", required_argument, NULL, 0},
            {"pub-trnest", no_argument, NULL, 0},
            {"pub-trnmeas", no_argument, NULL, 0},
            {"pub-trnmotn", no_argument, NULL, 0},
            {"pub-mb1", no_argument, NULL, 0},
            {"pub-mbest", no_argument, NULL, 0},
            {"input", required_argument, NULL, 0},
            {"cfg", required_argument, NULL, 0},
            {"mb-geo", required_argument, NULL, 0},
            {"dvl-geo", required_argument, NULL, 0},
            {"trnum", required_argument, NULL, 0},
            {NULL, 0, NULL, 0}
        };
        // reset optind
        optind=1;
        // process argument list
        while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1){
            char *acpy = nullptr;
            char *host_str = nullptr;
            char *port_str = nullptr;

            switch (c) {
                    // long options all return c=0
                case 0:
                    // verbose
                    if (strcmp("verbose", options[option_index].name) == 0) {
                        mVerbose=true;
                    }
                    // debug
                    else if (strcmp("debug", options[option_index].name) == 0) {
                        sscanf(optarg,"%d",&mDebug);
                    }
                    // help
                    else if (strcmp("help", options[option_index].name) == 0) {
                        help = true;
                    }
                    // version
                    else if (strcmp("version", options[option_index].name) == 0) {
                        version = true;
                    }

                    if(!mConfigSet){
                        // cfg
                        if (strcmp("cfg", options[option_index].name) == 0) {
                            mAppCfg = std::string(optarg);
                        }
                       mConfigSet = true;
                        break;
                    }else{
                        // host
                        if (strcmp("host", options[option_index].name) == 0) {
                            acpy = strdup(optarg);
                            host_str = strtok(acpy,":");
                            port_str = strtok(NULL,":");
                            if(NULL!=host_str){
                                free(mHost);
                                mHost = strdup(host_str);
                            }
                            if(NULL != port_str){
                                sscanf(port_str,"%d",&mPort);
                            }
                            free(acpy);
                        }
                        // delay
                        else if (strcmp("delay", options[option_index].name) == 0) {
                            sscanf(optarg,"%u",&mDelay);
                        }
                        // stats
                        else if (strcmp("stats", options[option_index].name) == 0) {
                            sscanf(optarg,"%lf,%d", &mStatPeriod, &mStatLevel);
                        }
                        // logdir
                        else if (strcmp("logdir", options[option_index].name) == 0) {
                            mLogDirStr=optarg;
                        }
                        // fake-mb1
                        else if (strcmp("fake-mb1", options[option_index].name) == 0) {
                            mFakeMB1=true;
                        }
                        // trn-csv
                        else if (strcmp("trn-csv", options[option_index].name) == 0) {
                            mTrnCsv = std::string(optarg);
                        }
                        // mb1-csv
                        else if (strcmp("mb1-csv", options[option_index].name) == 0) {
                            mMB1Csv = std::string(optarg);
                        }
                        // trn-cfg
                        else if (strcmp("trn-cfg", options[option_index].name) == 0) {
                            mTrnCfg = std::string(optarg);
                        }
                        // trn-decn
                        else if (strcmp("trn-decn", options[option_index].name) == 0) {
                            mTrnDecN = atoi(optarg);
                        }
                        // pub-trnest
                        else if (strcmp("pub-trnest", options[option_index].name) == 0) {
                            mPubTrnEst = true;
                        }
                        // pub-trnmeas
                        else if (strcmp("pub-trnmeas", options[option_index].name) == 0) {
                            mPubTrnMeas = true;
                        }
                        // pub-trnmotn
                        else if (strcmp("pub-trnmotn", options[option_index].name) == 0) {
                            mPubTrnMotn = true;
                        }
                        // pub-mb1
                        else if (strcmp("pub-mb1", options[option_index].name) == 0) {
                            mPubMB1 = true;
                        }
                        // pub-mbest
                        else if (strcmp("pub-mbest", options[option_index].name) == 0) {
                            mPubMBEst = true;
                        }
                        // input
                        else if (strcmp("input", options[option_index].name) == 0) {
                            std::list<std::string>::iterator it;
                            bool on_list=false;
                            for(it = mInputList.begin(); it != mInputList.end(); it++)
                            {
                                if(it->compare(optarg)==0){
                                    on_list=true;
                                    break;
                                }
                            }
                            if(!on_list){
                                mInputList.push_back(std::string(optarg));
                            }
                        }
                        // cycles
                        else if (strcmp("cycles", options[option_index].name) == 0) {
                            mCycles = atoi(optarg);
                        }
                        // mb-geo
                        else if (strcmp("mb-geo", options[option_index].name) == 0) {
                            char *acpy = strdup(optarg);
                            char *sbeams = strtok(acpy,":");
                            char *sswath = strtok(NULL,":");
                            char *srot = strtok(NULL,":");
                            char *strn = strtok(NULL,":");
                            double svr[3] = {0};
                            double svt[3] = {0};
                            uint16_t beams=0;
                            double swath=0;
                            if(NULL!=sbeams)
                                sscanf(sbeams,"%hu",&beams);
                            if(NULL!=sswath)
                                sscanf(sswath,"%lf",&swath);
                            if(NULL!=srot)
                                sscanf(srot,"%lf,%lf,%lf",&svr[0],&svr[1],&svr[2]);
                            if(NULL!=strn)
                                sscanf(strn,"%lf,%lf,%lf",&svt[0],&svt[1],&svt[2]);

                            if(NULL != mMBGeo){
                                // invalidate existing entry
                                delete mMBGeo;
                            }

                            mMBGeo = new mbgeo(beams, swath, &svr[0], &svt[0]);

                            free(acpy);
                        }
                        // dvl-geo
                        else if (strcmp("dvl-geo", options[option_index].name) == 0) {
                            char *acpy = strdup(optarg);
                            char *snbeams = strtok(acpy,":");
                            char *sbspec = strtok(NULL,":");
                            char *srot = strtok(NULL,":");
                            char *strn = strtok(NULL,":");
                            double svr[3] = {0};
                            double svt[3] = {0};
                            uint16_t nbeams=0;
                            if(NULL!=snbeams)
                                sscanf(snbeams,"%hu",&nbeams);
                            if(NULL!=srot)
                                sscanf(srot,"%lf,%lf,%lf",&svr[0],&svr[1],&svr[2]);
                            if(NULL!=strn)
                                sscanf(strn,"%lf,%lf,%lf",&svt[0],&svt[1],&svt[2]);

                            if(mDVLGeo!=NULL){
                                // invalidate existing entry
                                delete mDVLGeo;
                            }

                            mDVLGeo = new dvlgeo(nbeams, sbspec, &svr[0], &svt[0]);

                            free(acpy);
                        }
                        // trnum
                        else if (strcmp("trnum", options[option_index].name) == 0) {
                            char *acpy = strdup(optarg);
                            char *sgroup = strtok(acpy,":");
                            char *sport = strtok(NULL,":");
                            char *sttl = strtok(NULL,":");

                            if(NULL!=sgroup){
                                free(mTrnuGroup);
                                mTrnuGroup = strdup(sgroup);
                            }
                            if(NULL!=sport){
                                sscanf(sport,"%d",&mTrnuPort);
                            }
                            if(NULL!=sttl){
                                sscanf(sttl,"%d",&mTrnuTTL);
                            }
                            free(acpy);
                        }

                    }
                     break;
                default:
                    help=true;
                    break;
            }

            if (version) {
                fprintf(stderr, "%s: version %s build %s\n", TRNXPP_NAME, TRNXPP_VERSION, TRNXPP_BUILD);
                exit(0);
            }
            if (help) {
                //            mbtrn_show_app_version(TCPC_NAME,TCPC_BUILD);
                app_cfg::show_help();
                exit(0);
            }
        }// while
    }

    static void show_help()
    {
        // monitor mode:
        char help_message[] = "\n LCM TRN preprocessor\n";
        char usage_message[] = "\n use: trnxpp [options]\n"
        "\n"
        " Options\n"
        " --verbose             : verbose output\n"
        " --debug=d             : debug output\n"
        //    " --log-en      : enable app logging\n"
        " --help                : output help message\n"
        " --version             : output version info\n"
        " --cfg=s               : configuration file path\n"
        " --host=addr[:port]    : MB1 server\n"
        " --trnum=addr[:port:ttl] : TRN UDP mcast config (from mbtrnpp)\n"
        " --delay=u             : main loop delay\n"
        " --logdir=s            : log directory\n"
        " --fake-mb1            : publish fake MB1 output\n"
        " --trn-csv=s           : TRN CSV file path\n"
        " --mb1-csv=s           : MB1 CSV file path\n"
        " --trn-cfg=s           : trn config file path\n"
        " --trn-decn=d          : trn update modulus (every nth sounding)\n"
        " --pub-trnest          : publish TRN estimate LCM\n"
        " --pub-trnmeas         : publish TRN measurement update LCM\n"
        " --pub-trnmotm         : publish TRN motion update LCM\n"
        " --pub-mb1             : publish TRN motion update LCM\n"
        " --pub-mbest           : publish TRN motion update LCM\n"
        " --input=<ispec>       : specify input mapping/behavior\n"
        "                         ispec is an input specifier using format\n"
        "                           chan:<ctx_spec>[:<ctx_spec>...]\n"
        "                         where\n"
        "                           chan     : LCM channel name\n"
        "                           ctx_spec : context specifier using format:\n\n"
        "                             ctx/par[/sem]\n\n"
        "                         where:\n"
        "                           ctx : Context [mbtrn, trnsvr]\n"
        "                           par : Use this channel to provide\n"
        "                                 one or more parameters in\n"
        "                                 this context(comma-separated)\n"
        "                                   a:attitude\n"
        "                                   b:bathymetry\n"
        "                                   n:navigation\n"
        "                                   v:velocity\n"
        "                           sem : Optionally specify a semaphore callback\n"
        "                                 and/or override timeout (comma separated)\n"
        "                                 Valid callback values include:\n"
        "                                   pubmb1: MB1 publisher callback\n"
        "                                   pubtrn: TRNSVR publisher callback\n"
        "                                 Semaphore timeout values are in msec\n"
        "                         see also: Examples\n"
        " --mbgeo=<spec>        : specify multibeam geometry (used for MB1 output)\n"
        "                         spec is a specifier using format\n"
        "                          n:swath:svr(y,p,r deg):svt(x,y,z m)\n"
        "                         where\n"
        "                          n     : number of sonar beams\n"
        "                          swath : total beam angle\n"
        "                          svr   : sensor-vehicle rotation angles (y,p,r deg)\n"
        "                          svt   : sensor-vehicle translation distances (x,y,z m)\n"
        "                                  +x: fwd +y: stbd +z: down\n"
        " --dvlgeo=<spec>       : specify DVL geometry (used for MB1 output)\n"
        "                         spec is a specifier using format\n"
        "                          n:bspec:svr(y,p,r deg):svt(x,y,z m)\n"
        "                         where\n"
        "                          n     : number of sonar beams\n"
        "                          bspec uses one of two forms:\n"
        "                           A,yb,yi,pb,pi where\n"
        "                            yb    : yaw start angle (deg)\n"
        "                            yi    : yaw increment (deg)\n"
        "                            pb    : pitch start angle (deg)\n"
        "                            pi    : pitch increment (deg)\n"
        "                           or L,y0,p0...yn,pn where\n"
        "                            yn : yaw angle of beam[n] (reference frame, deg)\n"
        "                            pn : pitch angle of beam[n] (reference frame, deg)\n"
        "                          svr   : sensor-vehicle rotation angles (y,p,r deg)\n"
        "                          svt   : sensor-vehicle translation distances (x,y,z m)\n"
        "                                  +x: fwd +y: stbd +z: down\n"
        "\n"
        " --cycles=u             : stop after u cycles (for debugging)\n"
        " --stats=f,d            : stats output period (log, decimal sec), level (console) \n"
        " Notes:\n"
        "  Supported Input channels\n"
        "\n"
        "  Channel                LCM                 Provides\n"
        "  -------                ---                 --------\n"
        "  OPENINS_DVL_STAT       dvl_stat.lcm        bath, vel\n"
        "  IDT_PUB                idt_pub.lcm         bath\n"
        "  GSS_NAV_SOLUTION       nav_solution_t.lcm  nav, att\n"
        "  SONARDYNE_SPRINT_STAT  pcomms_t.lcm        nav, att\n"
        "\n"
        " Examples:\n"
        "  # Input Definitions\n"
        "  # use IDT_PUB to provide bathymetry for mbtrnpp data\n"
        "  # add a semaphore for this channel to call the MB1 publish callback\n"
        "    --input=IDT_PUB:mbtrn/b/pubmb1,100\n"
        "\n"
        "  # use GSS_NAV_SOLUTION pub to provide navigation and attitude for\n"
        "  # both mbtrnpp and trn-server data\n"
        "  # No semaphores are for this input\n."
        "    --input=GSS_NAV_SOLUTION:mbtrn/a,n:trnsvr/a,n\n"
        "\n"
        "  # use OPENINS_DVL_STAT to provide bathymetry and velocity\n"
        "  # for trn-server data\n"
        "  # add a semaphore for this channel to call the TRN publish callback\n"
        "    --input=OPENINS_DVL_STAT:mbtrn/v:trnsvr/b,v/pubtrn,100\n"
        "\n";
        printf("%s", help_message);
        printf("%s", usage_message);
    }

    char *comment(char *src)
    {
        TRN_NDPRINT(4,"%s:%d >>> comment[%s]\n",__func__,__LINE__,src);
        char *bp = src;
        char *retval = src;
        if(NULL != src){
            while(*bp != '\0'){
                if(isspace(*bp)){
                    retval = bp;
                } else if(*bp == '#'){
                    TRN_NDPRINT(4,"%s:%d\n",__func__,__LINE__);
                    *bp = '\0';
                    retval = bp;
                    break;
                } else if (*bp == '/' && bp[1]=='/'){
                    TRN_NDPRINT(4,"%s:%d\n",__func__,__LINE__);
                    *bp = '\0';
                    retval = bp;
                    break;
                }else{
                    TRN_NDPRINT(4,"%s:%d\n",__func__,__LINE__);
                    retval = bp;
                    break;
                }
                bp++;
            }
        }
        return retval;
    }

    char *trim(char *src)
    {
        char *bp = NULL;
        if(NULL!=src){
            bp = src;
            char *ep = src+strlen(src);
            while(isspace(*bp) && (*bp != '\0')){
                bp++;
            }
            while((isspace(*ep) || *ep == '\0') && (ep >= src)){
                *ep = '\0';
                ep--;
            }
        }
        return bp;
    }

    void parse_key_val(char *src, const char *del, char **pkey, char **pval)
    {
        char *scpy = strdup(src);
        char *key = strtok(scpy, del);
        char *val = strtok(NULL, del);
        if(NULL!=key)
            *pkey = strdup(key);
        else
            *pkey = NULL;
        if(NULL!=val)
            *pval = strdup(val);
        else
            *pval = NULL;
        free(scpy);
    }

    char *expand_env(char *src)
    {
        char *retval = NULL;

        if(NULL!=src && strlen(src)>0 ){
            char *obuf=NULL;
            int bsz = strlen(src)+1;
            char *wp = (char *)malloc(bsz);
            char *sp = wp;
            memset(wp,0,bsz);
            snprintf(wp, bsz, "%s",src);
            char *pb;

            while( (pb = strstr(wp,"$")) != NULL)
            {
                TRN_NDPRINT(4,">>> wp[%s]\n",wp);
                char *pe = pb+1;
                TRN_NDPRINT(4,">>> pe...");
                while( (isalnum(*pe) || *pe=='-' || *pe == '_') && *pe!='\0' ){
                    TRN_NDPRINT(4,"[%c] ",*pe);
                    pe++;
                }
                TRN_NDPRINT(4,"\n");
                // pe points to char AFTER end of var name
                if(pe>pb){
                    size_t var_len = pe-pb;
                    char var_buf[var_len+1];
                    memset(var_buf,0,var_len+1);
                    for(uint32_t i=1;i<var_len;i++){
                        var_buf[i-1] = pb[i];
                    }
                    TRN_NDPRINT(4,">>> var_buf[%s]\n",var_buf);
                    char *val = getenv(var_buf);
                    size_t val_len = (val!=NULL?strlen(val):0);
                    size_t new_len = strlen(wp) - var_len + val_len + 1;
                    *pb='\0';
                    *(pe-1)='\0';
                    char *pecpy = strdup(pe);
                    char *rebuf = (char *)malloc(new_len);
                    memset(rebuf,0,new_len);
                    snprintf(rebuf, new_len, "%s%s%s",wp,val,pecpy);
                    free(pecpy);
                    free(obuf);
                    obuf = rebuf;
                    wp = obuf;
                    retval = obuf;
                }
            }
            free(sp);
        }
        return retval;
    }

    void parse_file(const std::string &file_path)
    {
        std::ifstream file(file_path.c_str(), std::ifstream::in);

        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line) && file.good()) {
                // using printf() in all tests for consistency
                TRN_NDPRINT(4,">>> line : [%s]\n", line.c_str());
                if(line.length()>0){
                    char *lcp = strdup(line.c_str());
                    char *wp = trim(lcp);
                    TRN_NDPRINT(4,">>> wp[%s]\n", wp);
                    if(wp==NULL || strlen(wp)<=0){
                        // empty/comment
                    } else {
                        char *cp = comment(wp);
                        TRN_NDPRINT(4,">>> cp[%s]\n", cp);
                        if(strlen(cp) > 0){
                            char *key=NULL;
                            char *val=NULL;
                            parse_key_val(cp, "=", &key, &val);
                            char *tkey = trim(key);
                            char *tval = trim(val);
                            TRN_NDPRINT(4,">>> key[%s] val[%s]\n",tkey,tval);
                            char *etval = expand_env(tval);
                            if(etval==NULL)
                                etval=tval==NULL?strdup(""):strdup(tval);

                            TRN_NDPRINT(4,">>> key[%s] etval[%s]\n",tkey,etval);
                            size_t cmd_len = strlen(key) + strlen(etval) + 4;
                            char *cmd_buf = (char *)malloc(cmd_len);
                            memset(cmd_buf,0,cmd_len);
                            snprintf(cmd_buf, cmd_len, "--%s%s%s", key,(strlen(etval)>0?"=":""),etval);
                            char dummy[]{'f','o','o','\0'};
                            char *cmdv[2]={dummy,cmd_buf};
                            TRN_NDPRINT(4,">>> cmd_buf[%s] cmdv[%p]\n",cmd_buf,&cmdv[0]);

                            // parse this argument
                            parse_args(2,&cmdv[0]);

                            free(key);
                            free(val);
                            free(etval);
                            free(cmd_buf);
                            cmd_buf = NULL;
                            key=NULL;
                            val=NULL;
                            etval=NULL;
                        }else{
                            TRN_NDPRINT(4, ">>> [comment line]\n");
                        }
                    }
                    free(lcp);
                    lcp=NULL;
                }
            }
            file.close();
        } else {
            fprintf(stderr, "ERR - file open failed [%s] [%d/%s]", file_path.c_str(), errno, strerror(errno));
        }
    }

    bool verbose(){return mVerbose;}
    int debug(){return mDebug;}
    char *host(){return mHost;}
    int port(){return mPort;}
    int delay(){return mDelay;}
    bool fakemb1(){return mFakeMB1;}
    bool pub_trn_est(){return mPubTrnEst;}
    bool pub_trn_meas(){return mPubTrnMeas;}
    bool pub_trn_motn(){return mPubTrnMotn;}
    bool pub_mb1(){return mPubMB1;}
    bool pub_mbest(){return mPubMBEst;}
    std::string trn_cfg(){return mTrnCfg;}
    std::string app_cfg_path(){return mAppCfg;}
    std::list<std::string> input_list(){return mInputList;}
    int trn_decn(){return mTrnDecN;}
    mbgeo *mb_geo(){return mMBGeo;}
    dvlgeo *dvl_geo(){return mDVLGeo;}
    std::string trn_csv(){return mTrnCsv;}
    std::string mb1_csv(){return mMB1Csv;}
    std::string session_string(){return mSessionStr;}
    std::string logdir(){return mLogDirStr;}
    logu::logger &mlog(){return mMsgLog;}
    int cycles(){return mCycles;}
    app_stats &stats(){return mStats;}
    double stat_period(){return mStatPeriod;}
    int stat_level(){return mStatLevel;}
    char *trnu_group(){return mTrnuGroup;}
    int trnu_port(){return mTrnuPort;}
    int trnu_ttl(){return mTrnuTTL;}

    void tostream(std::ostream &os, int wkey=15, int wval=18)
    {
        os << std::dec << std::setfill(' ');
        os << std::setw(wkey) << "verbose " << std::setw(wval) << (mVerbose?"Y":"N") << "\n";
        os << std::setw(wkey) << "debug " << std::setw(wval) << mDebug  << "\n";
        os << std::setw(wkey) << "cfg " << std::setw(wval) << mAppCfg  << "\n";
        os << std::setw(wkey) << "host " << std::setw(wval) << mHost << ":" << mPort << "\n";
        os << std::setw(wkey) << "trnu " << std::setw(wval) << mTrnuGroup << ":" <<  mTrnuPort << ":" << mTrnuTTL<< "\n";
        os << std::setw(wkey) << "cycles " << std::setw(wval) << mCycles  << "\n";
        os << std::setw(wkey) << "delay " << std::setw(wval) << mDelay << "\n";
        os << std::fixed << std::setprecision(3);
        os << std::setw(wkey) << "stat_period " << std::setw(wval) << mStatPeriod << "\n";
        os << std::dec;
        os << std::setw(wkey) << "stat_level " << std::setw(wval) << mStatLevel << "\n";
        os << std::setw(wkey) << "logdir " << std::setw(wval) << mLogDirStr.c_str() << "\n";
        os << std::setw(wkey) << "session " << std::setw(wval) << mSessionStr << "\n";
        os << std::setw(wkey) << "pub-trnest " << std::setw(wval) << (mPubTrnEst?"Y":"N")  << "\n";
        os << std::setw(wkey) << "pub-trnmeas " << std::setw(wval) << (mPubTrnMeas?"Y":"N")  << "\n";
        os << std::setw(wkey) << "pub-trnmotn " << std::setw(wval) << (mPubTrnMotn?"Y":"N")  << "\n";
        os << std::setw(wkey) << "pub-mb1 " << std::setw(wval) << (mPubMB1?"Y":"N")  << "\n";
        os << std::setw(wkey) << "pub-mbest " << std::setw(wval) << (mPubMBEst?"Y":"N")  << "\n";
        os << std::setw(wkey) << "trn-cfg " << std::setw(wval) << mTrnCfg  << "\n";
        os << std::setw(wkey) << "trn-csv " << std::setw(wval) << mTrnCsv  << "\n";
        os << std::setw(wkey) << "mb1-csv " << std::setw(wval) << mMB1Csv  << "\n";
        os << std::setw(wkey) << "trn-decn " << std::setw(wval) << mTrnDecN  << "\n";
        os << std::setw(wkey) << "fakemb1 " << std::setw(wval) << (mFakeMB1?"Y":"N")  << "\n";
        os << std::setw(wkey) << "inputs"<< "\n";

        std::list<std::string>::iterator it;
        for(it=mInputList.begin(); it != mInputList.end(); it++)
        {
            os << std::setw(wkey) << " " << std::setw(wval) << static_cast<std::string>(*it) << "\n";
        }

        os << std::setw(wkey) << "-- mbgeo --\n";
        if(NULL != mMBGeo)
        os << mMBGeo->tostring(wkey,wval) << "\n";
        os << std::setw(wkey) << "-- dvlgeo --\n";
        if(NULL != mDVLGeo)
        os << mDVLGeo->tostring(wkey,wval) << "\n";
    }

    std::string tostring(int wkey=15, int wval=18)
    {
        std::ostringstream ss;
        tostream(ss, wkey, wval);
        return ss.str();
    }

    void show(int wkey=15, int wval=18)
    {
        tostream(std::cerr, wkey, wval);
    }
    bool config_set(){return mConfigSet;}

protected:
private:
    bool mVerbose;
    int mDebug;
    int mCycles;
    char *mHost;
    int mPort;
    char *mTrnuGroup;
    int mTrnuPort;
    int mTrnuTTL;
    unsigned int mDelay;
    bool mFakeMB1;
    bool mPubTrnEst;
    bool mPubTrnMotn;
    bool mPubTrnMeas;
    bool mPubMB1;
    bool mPubMBEst;
    std::list<std::string> mInputList;
    std::string mTrnCfg;
    std::string mAppCfg;
    int mTrnDecN;
    mbgeo *mMBGeo;
    dvlgeo *mDVLGeo;
    std::string mTrnCsv;
    std::string mMB1Csv;
    std::string mSessionStr;
    std::string mLogDirStr;
    logu::logger mMsgLog;
    app_stats mStats;
    double mStatPeriod;
    int mStatLevel;
    bool mConfigSet;
};

typedef struct callback_res_s
{
    app_cfg *cfg;
    trn::trnxpp *xpp;
}callback_res_t;

// /////////////////
// Module variables
static int g_signal=0;
static bool g_interrupt=false;

// /////////////////
// Declarations
static void s_termination_handler (int signum);
int update_mb1(trn::trnxpp *xpp);
int update_trncli(trn::trnxpp *xpp);

// /////////////////
// Function Definitions

/// @fn void termination_handler (int signum)
/// @brief termination signal handler.
/// @param[in] signum signal number
/// @return none
static void s_termination_handler (int signum)
{
    switch (signum) {
        case SIGINT:
        case SIGHUP:
        case SIGTERM:
            fprintf(stderr,"INFO - sig received[%d]\n",signum);
            g_interrupt=true;
            g_signal=signum;
            break;
        default:
            fprintf(stderr,"ERR - s_termination_handler: sig not handled[%d]\n",signum);
            break;
    }
}

#ifdef WITH_TEST_STREAMS
static mb1_t *s_get_test_sounding(mb1_t *dest, int beams)
{
    static int cx=0;
    int test_beams = (beams <= 0 ? 4 : beams);
    int k=0;
    mb1_t *snd = (dest!=nullptr ? dest : mb1_new(test_beams));

    snd->hdg = 45.+5.*sin(cx*M_PI/180.);
    snd->depth = 50.+10.*sin(cx*M_PI/180.);
    snd->lat = 30.2+cx/1000.;
    snd->lon = -130.4+cx/1000.;
    snd->type = MB1_TYPE_ID;
    snd->size = MB1_SOUNDING_BYTES(test_beams);
    snd->nbeams = test_beams;
    snd->ping_number = cx;
    snd->ts = time(NULL);
    for(k=0;k<test_beams;k++){
        snd->beams[k].beam_num=k;
        snd->beams[k].rhox = .1*k + sin(cx*1.*M_PI/180);
        snd->beams[k].rhoy = .1*k + sin(cx*2.*M_PI/180);//cx*2.;
        snd->beams[k].rhoz = .1*k + sin(cx*3.*M_PI/180);//cx*4.;
    }
    cx++;

    mb1_set_checksum(snd);
    return snd;
}
#endif

int cb_raw_sig(void *pargs){
    TRN_NDPRINT(2, "%s:%d >>> Callback triggered <<<\n", __func__, __LINE__);
    // do something with the stream...
    callback_res_t *cb_res = static_cast<callback_res_t *>(pargs);
    trn::trnxpp *xpp = cb_res->xpp;//static_cast<trn::trnxpp *>(pargs);
//    app_cfg *cfg = cb_res->cfg;

    trn::raw_signal_input *rs = static_cast<trn::raw_signal_input *>(xpp->get_input("RAW_SIGNAL"));
    rs->set_bar((1+rs->bar()));
    std::cerr << "raw_signal.bar:" << rs->bar() << "\n";
    return 0;
}

int cb_string(void *pargs){
    TRN_NDPRINT(2, "%s:%d >>> Callback triggered <<<\n", __func__, __LINE__);
//    callback_res_t *cb_res = static_cast<callback_res_t *>(pargs);
//    trn::trnxpp *xpp = cb_res->xpp;//static_cast<trn::trnxpp *>(pargs);
//    app_cfg *cfg = cb_res->cfg;
    return 0;
}

int cb_bath_oins_dvl(void *pargs){
    TRN_NDPRINT(2, "%s:%d >>> Callback triggered <<<\n", __func__, __LINE__);
    callback_res_t *cb_res = static_cast<callback_res_t *>(pargs);
    trn::trnxpp *xpp = cb_res->xpp;//static_cast<trn::trnxpp *>(pargs);
//    app_cfg *cfg = cb_res->cfg;

    trn::bath_input *dvl = dynamic_cast<trn::bath_input *>(xpp->get_input("OPENINS_DVL_STAT"));
    dvl->show();

    trn::bath_info *bi = dvl->bath_inst();
    TRN_NDPRINT(2, ">>>> BATHINST[%s] flags[%08X]\n",bi->bathstr(),bi->flags().get());
    delete bi;
    return 0;
}

int cb_nav_gss_nav_sol(void *pargs){
    TRN_NDPRINT(2, "%s:%d >>> Callback triggered <<<\n", __func__, __LINE__);
    callback_res_t *cb_res = static_cast<callback_res_t *>(pargs);
    trn::trnxpp *xpp = cb_res->xpp;//static_cast<trn::trnxpp *>(pargs);
//    app_cfg *cfg = cb_res->cfg;

    trn::nav_input *ns = dynamic_cast<trn::nav_input *>(xpp->get_input("GSS_NAV_SOLUTION"));
    ns->show();

    trn::nav_info *ni = ns->nav_inst();
    TRN_NDPRINT(2, ">>>> NAVINST.gssnav[%s] flags[%08X]\n",ni->navstr(),ni->flags().get());
    delete ni;

    return 0;
}

int cb_bath_idt(void *pargs){
    TRN_NDPRINT(2, "%s:%d >>> Callback triggered <<<\n", __func__, __LINE__);
    callback_res_t *cb_res = static_cast<callback_res_t *>(pargs);
    trn::trnxpp *xpp = cb_res->xpp;//static_cast<trn::trnxpp *>(pargs);
//    app_cfg *cfg = cb_res->cfg;

    trn::bath_input *bs = dynamic_cast<trn::bath_input *>(xpp->get_input("IDT_PUB"));
    bs->show();

    trn::bath_info *bi = bs->bath_inst();
    TRN_NDPRINT(2, ">>>> BATHINST.idt[%s]\n",bi->bathstr());
    delete bi;

    return 0;
}

int cb_nav_pcomms(void *pargs){
    TRN_NDPRINT(2, "%s:%d >>> Callback triggered <<<\n", __func__, __LINE__);
    callback_res_t *cb_res = static_cast<callback_res_t *>(pargs);
    trn::trnxpp *xpp = cb_res->xpp;//static_cast<trn::trnxpp *>(pargs);
//    app_cfg *cfg = cb_res->cfg;

    trn::nav_input *ns = dynamic_cast<trn::nav_input *>(xpp->get_input("SONARDYNE_SPRINT_STAT"));
    ns->show();

    trn::nav_info *ni = ns->nav_inst();
    TRN_NDPRINT(2, ">>>> NAVINST.pcomms[%s] flags[%08X]\n",ni->navstr(),ni->flags().get());
    delete ni;

    return 0;
}

int write_csv(FILE *fp, trn::bath_info *bi, trn::att_info *ai, trn::nav_info *ni, trn::vel_info *vi=nullptr)
{
    // time, lat, lon, roll, pitch, hdg,valid,1,1,...nbeams,beamnum, range,...\n
    std::ostringstream ss;
    if(NULL != fp && bi && ai && ni)
    {
        double lat = ni->lat();
        double lon = ni->lon();
        double pos_N=0;
        double pos_E=0;
        long int utm = NavUtils::geoToUtmZone(Math::degToRad(lat),
                                              Math::degToRad(lon));

        // NavUtils::geoToUtm(latitude, longitude, utmZone, *northing, *easting)
        NavUtils::geoToUtm(Math::degToRad(lat), Math::degToRad(lon), utm, &pos_N, &pos_E);
        // Note that TRN uses N,E,D frame (i.e. N:x E:y D:z)
        // time POSIX epoch sec
        // northings
        // eastings
        // depth
        // heading
        // pitch
        // roll
        // flag (0)
        // flag (0)
        // flag (0)
        // vx (0)
        // xy (0)
        // vz (0)
        // sounding valid flag
        // bottom lock valid flag
        // number of beams
        // beam[i] number
        // beam[i] valid (1)
        // beam[i] range
        // ...
        // NEWLINE

        ss << std::dec << std::setfill(' ') << std::fixed << std::setprecision(7);
        ss << bi->time_usec()/1000000. << ",";
        ss << std::setprecision(7);
        ss << pos_N << ",";
        ss << pos_E << ",";
        ss << ni->depth() << ",";
        ss << ai->heading() << ",";
        ss << ai->pitch() << ",";
        ss << ai->roll() << ",";
        ss << 0 << ",";
        ss << 0 << ",";
        ss << 0 << ",";
        if(vi != nullptr){
            ss << vi->vx_ms() << ",";
            ss << vi->vy_ms() << ",";
            ss << vi->vz_ms() << ",";
        }else{
            ss << 0. << ",";
            ss << 0. << ",";
            ss << 0. << ",";
        }
        ss << std::setprecision(1);
        ss << (bi->flags().is_set(trn::BF_VALID)?1:0) << ",";
        ss << (bi->flags().is_set(trn::BF_BLOCK)?1:0) << ",";
        ss << bi->beam_count() << ",";
        ss << std::setprecision(4);
        std::list<trn::beam_tup>::iterator it;
        std::list<trn::beam_tup>beam_list = bi->beams_raw();
        int k=0;
        for(it=beam_list.begin(); it!=beam_list.end(); k++)
        {
            trn::beam_tup bt = static_cast<trn::beam_tup> (*it);
            ss <<  std::get<0>(bt) << ",";
            ss << 1 << ",";
            ss << std::get<1>(bt);
            it++;
            if(it != beam_list.end())
                ss << ",";
        }

        fprintf(fp, "%s\n", ss.str().c_str());
    }
    return ss.str().length();
}

void trnest_tostream(std::ostream &os, double &ts, poseT &pt, poseT &mle, poseT &mmse, int wkey=15, int wval=1)
{
    os << "--- TRN Estimate OK---" << "\n";
    os << "MLE[t,tm,x,y,z] ";
    os << std::fixed << std::setprecision(3);
    os << ts << ",";
    os << std::setprecision(2);
    os << mle.time << ",";
    os << std::setprecision(4);
    os << mle.x << "," << mle.y << "," << mle.z << "\n";

    os << "MMSE[t,tm,x,y,z] ";
    os << std::fixed << std::setprecision(3);
    os << ts << ",";
    os << std::setprecision(2);
    os << mmse.time << ",";
    os << std::setprecision(4);
    os << mmse.x << "," << mmse.y << "," << mmse.z << "\n";

    os << "POS[t,tm,x,y,z]  ";
    os << std::fixed << std::setprecision(3);
    os << ts << ",";
    os << std::setprecision(2);
    os << mmse.time << ",";
    os << std::setprecision(4);
    os << pt.x << "," << pt.y << "," << pt.z << "\n";

    os << "OFS[t,tm,x,y,z]  ";
    os << std::fixed << std::setprecision(3);
    os << ts << ",";
    os << std::setprecision(2);
    os << mmse.time << ",";
    os << std::setprecision(4);
    os << pt.x-mmse.x << "," << pt.y-mmse.y << "," << pt.z-mmse.z << "\n";

    os << "COV[t,x,y,z]     ";
    os << std::setprecision(3);
    os << mmse.time << ",";
    os << std::setprecision(2);
    os << sqrt(mmse.covariance[0]) << ",";
    os << sqrt(mmse.covariance[2]) << ",";
    os << sqrt(mmse.covariance[5]) << "\n";

//    fprintf(stderr,"MLE[t,tx,y,z] [ %.3lf, %.2f , %.4f , %.4f , %.4f ]\n",time,mle.time,mle.x, mle.y, mle.z);
//    fprintf(stderr,"MSE[t,x,y,z] [ %.3lf, %.2f , %.4f , %.4f , %.4f ]\n",time,mmse.time, mmse.x, mmse.y, mmse.z);
//    fprintf(stderr,"COV[t,x,y,z] [ %.3lf, %.2f , %.2f , %.2f ]\n",
//            mmse.time,
//            sqrt(mmse.covariance[0]),
//            sqrt(mmse.covariance[2]),
//            sqrt(mmse.covariance[5]));

}

std::string  trnest_tostring(double &time, poseT &pt, poseT &mle, poseT &mmse, int wkey=15, int wval=1)
{
    std::ostringstream ss;
    trnest_tostream(ss, time, pt, mle, mmse, wkey, wval);
    return ss.str();
}

void trnest_show(double &time, poseT &pt, poseT &mle, poseT &mmse, int wkey=15, int wval=18)
{
    trnest_tostream(std::cerr, time, pt, mle, mmse, wkey, wval);
}

int cb_update_trncli(void *pargs)
{
    static uint32_t cx=0;
    static int decn=0;
    int retval=0;

    TRN_NDPRINT(3, "%s:%d >>> Callback triggered <<<\n", __func__, __LINE__);

    callback_res_t *cb_res = static_cast<callback_res_t *>(pargs);
    trn::trnxpp *xpp = cb_res->xpp;//static_cast<trn::trnxpp *>(pargs);
    app_cfg *cfg = cb_res->cfg;

    cfg->stats().trn_cb_n++;

    if(cfg->trn_decn()>0 && (decn++ % cfg->trn_decn())!=0){
        return -1;
    }

    //    TRN_NDPRINT(3, "%s:%d >>> update_trncli decn[%d]<<<\n", __func__, __LINE__,decn);

    trn::trn_lcm_input *bs = xpp->get_input(xpp->ctx(trn::CTX_TRNSVR).bath_input());
    trn::trn_lcm_input *ns = xpp->get_input(xpp->ctx(trn::CTX_TRNSVR).nav_input());
    trn::trn_lcm_input *as = xpp->get_input(xpp->ctx(trn::CTX_TRNSVR).att_input());
    trn::trn_lcm_input *vs = xpp->get_input(xpp->ctx(trn::CTX_TRNSVR).vel_input());

    bool streams_ok=true;

    if(bs==nullptr){
        fprintf(stderr,"%s:%d WARN - bath input invalid s[%p]\n", __func__, __LINE__, bs);
        streams_ok=false;
    }
    if(ns==nullptr){
        fprintf(stderr,"%s:%d WARN - nav input invalid s[%p]\n", __func__, __LINE__, ns);
        streams_ok=false;
    }
    if(vs==nullptr){
        fprintf(stderr,"%s:%d WARN - vel input invalid s[%p]\n", __func__, __LINE__, vs);
        streams_ok=false;
    }
    if(as==nullptr){
        fprintf(stderr,"%s:%d WARN - att input invalid s[%p]\n", __func__, __LINE__, as);
        streams_ok=false;
    }

    trn::bath_input *bp = nullptr;
    trn::nav_input *np = nullptr;
    trn::vel_input *vp = nullptr;
    trn::att_input *ap = nullptr;
    if(streams_ok){
        bp = dynamic_cast<trn::bath_input *>(bs);
        np = dynamic_cast<trn::nav_input *>(ns);
        vp = dynamic_cast<trn::vel_input *>(vs);
        ap = dynamic_cast<trn::att_input *>(as);
        if(bp==nullptr){
            fprintf(stderr,"%s:%d WARN - bath IF invalid p[%p]\n", __func__, __LINE__, bp);
            streams_ok=false;
        }
        if(vp==nullptr){
            fprintf(stderr,"%s:%d WARN - vel IF invalid p[%p]\n", __func__, __LINE__, vp);
            streams_ok=false;
        }
        if(np==nullptr){
            fprintf(stderr,"%s:%d WARN - nav IF invalid p[%p]\n", __func__, __LINE__, bp);
            streams_ok=false;
        }
        if(ap==nullptr){
            fprintf(stderr,"%s:%d WARN - att IF invalid p[%p]\n", __func__, __LINE__, ap);
            streams_ok=false;
        }
    }

    trn::bath_info *bi = nullptr;
    trn::nav_info *ni = nullptr;
    trn::att_info *ai = nullptr;
    trn::vel_info *vi = nullptr;

    if(streams_ok){
        bi = bp->bath_inst();
        ni = np->nav_inst();
        ai = ap->att_inst();
        vi = vp->vel_inst();

        if(bi==nullptr){
            fprintf(stderr,"%s:%d WARN - bath info invalid i[%p]\n", __func__, __LINE__, bi);
            streams_ok=false;
        }
        if(vi==nullptr){
            fprintf(stderr,"%s:%d WARN - vel info invalid i[%p]\n", __func__, __LINE__, vi);
            streams_ok=false;
        }
        if(ni==nullptr){
            fprintf(stderr,"%s:%d WARN - nav info invalid i[%p]\n", __func__, __LINE__, bi);
            streams_ok=false;
        }
        if(ai==nullptr){
            fprintf(stderr,"%s:%d WARN - att info invalid i[%p]\n", __func__, __LINE__, ai);
            streams_ok=false;
        }
    }

    TrnClient *trn = NULL;
    if(streams_ok){
        trn=xpp->ctx(trn::CTX_TRNSVR).trn_client();
        if(trn==nullptr || trn==NULL){
            fprintf(stderr,"%s:%d WARN - trn client invalid trn[%p]\n", __func__, __LINE__, trn);
            streams_ok=false;
        } else if(!trn->is_connected()){
            fprintf(stderr,"%s:%d WARN - trn client not connected trn[%p]\n", __func__, __LINE__, trn);
            cfg->stats().trn_cli_dis++;
            int test=0;
            if( (test = xpp->trncli_connect(1,0,&g_interrupt)) != 0){
                fprintf(stderr,"%s:%d ERR - trn client reconnect failed [%d]\n", __func__, __LINE__, test);
                streams_ok=false;
            }else{
                trn=xpp->ctx(trn::CTX_TRNSVR).trn_client();
                fprintf(stderr,"%s:%d INFO - trn client reconnect OK trn[%p]\n", __func__, __LINE__, trn);
                cfg->stats().trn_cli_con++;
            }
        }
    }

    if(streams_ok){
        double lat = ni->lat();
        double lon = ni->lon();
        long int utm = NavUtils::geoToUtmZone(Math::degToRad(lat),
                                              Math::degToRad(lon));
        ai->flags().set(trn::AF_INVERT_PITCH);
        // ai->flags().set(trn::AF_INVERT_ROLL);
        double x = 0.;
        double y = 0.;
        double z = ni->depth();
        double phi = ai->roll();
        double theta = ai->pitch();
        double psi = ai->heading();
        // TRN requires vx != 0 to initialize
        // vy, vz not strictly required
        double vx = vi->vx_ms();
        double vy = 0.;
        double vz = 0.;
        double time = ni->time_usec()/1e6;
        bool dvlValid = bi->flags().is_set(trn::BF_VALID);
        bool gpsValid = ni->flags().is_set(trn::NF_POS_VALID);
        bool bottomLock = bi->flags().is_set(trn::BF_BLOCK);

        // NavUtils::geoToUtm(latitude, longitude, utmZone, *northing, *easting)
        // Note that TRN uses N,E,D frame (i.e. N:x E:y D:z)
        NavUtils::geoToUtm(Math::degToRad(lat),
                           Math::degToRad(lon),
                           utm, &x, &y);

        //    std::cerr << "  <<<<<<<<<<< !!!!! flag data !!!!! >>>>>>>>>>>>\n";
        //    std::cerr << std::hex;
        //    std::cerr << "  bi->flags : " << bi->flags().get() << "\n";
        //    std::cerr << "  ni->flags : " << ni->flags().get() << "\n";
        //    std::cerr << std::dec;
        //    std::cerr << "  dvlValid : " << dvlValid << "\n";
        //    std::cerr << "  gpsValid : " << gpsValid << "\n";
        //    std::cerr << "bottomLock : " << bottomLock << "\n";


        TRN_NDPRINT(2, "%s:%d lat[%.6lf] lon[%.6lf] utm[%ld]\n", __func__, __LINE__, lat, lon, utm);
        TRN_NDPRINT(2, "%s:%d x[%.4lf] y[%.4lf] depth[%.1lf] p/r/y[%.2lf%s %.2lf, %.2lf] vx[%.2lf]\n", __func__, __LINE__, x, y, z, theta, (ai->flags().is_set(trn::AF_INVERT_PITCH)? "*," :","), phi, psi, vx);

        poseT pt;
        pt.x = x;
        pt.y = y;
        pt.z = z;
        pt.phi = phi;
        pt.theta = theta;
        pt.psi = psi;
        pt.time = time;
        pt.dvlValid=dvlValid;
        pt.gpsValid=gpsValid;
        pt.bottomLock=bottomLock;
        // w* required? (values from trnw/replay/dorado)?
        pt.wx = -3.332e-002;
        pt.wy = -9.155e-003;
        pt.wz = -3.076e-002;
        // doesn't work if v* not set - why?
        // OK for v* all the same (e.g.0.01 as in mbtrnpp)?
        pt.vx = vx;
        pt.vy = vy;
        pt.vz = vz;

        size_t n_beams = bi->beam_count();

        measT mt(n_beams,TRN_SENSOR_DVL);
        mt.x = x;
        mt.y = y;
        mt.z = z;
        mt.phi = phi;
        mt.theta = theta;
        mt.psi = psi;
        mt.time = time;
        mt.ping_number = cx;

        std::list<trn::beam_tup> beams = bi->beams_raw();
        std::list<trn::beam_tup>::iterator it;
        int k=0;
        for(it=beams.begin(); it!=beams.end(); it++,k++)
        {
            trn::beam_tup bt = static_cast<trn::beam_tup> (*it);
            mt.ranges[k] = std::get<1>(bt);
            mt.beamNums[k] = std::get<0>(bt);
            mt.measStatus[k] = (mt.ranges[k] > 1 ? true : false);
        }
        TRN_NDPRINT(2, "%s:%d nbeams[%u] ranges[%.2lf, %.2lf, %.2lf, %.2lf] status[%c, %c, %c, %c]\n", __func__, __LINE__,n_beams,
                    n_beams>0?mt.ranges[0]:-1.,
                    n_beams>1?mt.ranges[1]:-1.,
                    n_beams>2?mt.ranges[2]:-1.,
                    n_beams>3?mt.ranges[3]:-1.,
                    n_beams>0?mt.measStatus[0] ? 'Y' : 'N':'?',
                    n_beams>1?mt.measStatus[1] ? 'Y' : 'N':'?',
                    n_beams>2?mt.measStatus[2] ? 'Y' : 'N':'?',
                    n_beams>3?mt.measStatus[3] ? 'Y' : 'N':'?');

        try{
            trn->motionUpdate(&pt);
            cfg->stats().trn_motn_n++;
            trn->measUpdate(&mt, bp->bath_input_type());
            cfg->stats().trn_meas_n++;

            poseT mle, mmse;
            trn->estimatePose(&mmse, TRN_EST_MMSE);
            cfg->stats().trn_mmse_n++;
            trn->estimatePose(&mle, TRN_EST_MLE);
            cfg->stats().trn_mle_n++;

            write_csv(xpp->ctx(trn::CTX_TRNSVR).csv_file(), bi, ai, ni, vi);
            cfg->stats().trn_csv_n++;

            retval=0;

            if(cfg->pub_trn_motn()){
                pcf::lcm_publisher *pub = xpp->get_pub("TRN_MOTN");
                if(pub != nullptr){
                    trn::trn_pose_t motn_msg;
                    trn::trn_msg_utils::pose_to_lcm(motn_msg, pt);
                    pub->publish(motn_msg);
                    cfg->stats().trn_pub_motn_n++;
                }
            }

            if(cfg->pub_trn_meas()){
                pcf::lcm_publisher *pub = xpp->get_pub("TRN_MEAS");
                if(pub != nullptr){
                    trn::trn_meas_t meas_msg;
                    trn::trn_msg_utils::meas_to_lcm(meas_msg, mt);
                    pub->publish(meas_msg);
                    cfg->stats().trn_pub_meas_n++;
                }
            }

            if(cfg->pub_trn_est()){
                pcf::lcm_publisher *pub = xpp->get_pub("TRN_EST");
                if(pub != nullptr){
                    trn::trn_pose_t mmse_msg;
                    trn::trn_msg_utils::pose_to_lcm(mmse_msg, mmse);
                    pub->publish(mmse_msg);
                    cfg->stats().trn_pub_est_n++;

                    trn::trn_stat_t trnstat_msg;
                    trn::trn_msg_utils::trn_to_lcm(trnstat_msg, "TRNSVR", pt, mmse, mle);
                    pub->publish(mmse_msg);
                    cfg->stats().trn_pub_stat_n++;
              }
            }

            if(trn->lastMeasSuccessful()){
                cfg->stats().trn_est_ok_n++;
                trnest_show(time, pt, mle, mmse);
                fprintf(stderr, "\n");
                LU_PEVENT(cfg->mlog(), "trn est:\n%s\n", trnest_tostring(time, pt, mle, mmse).c_str());
            }else{
                TRN_NDPRINT(3, "%s:%d - lastMeasSuccessful ERR\n",__func__,__LINE__);
            }
        }catch(Exception e){
            fprintf(stderr,"%s - caught exception in TRN update [%s]\n",__func__, e.what());
            cfg->stats().trn_err_n++;
        }
    }else{
        cfg->stats().trn_err_n++;
    }

    delete bi;
    delete ai;
    delete ni;
    delete vi;

    cx++;

    return retval;
}

Matrix applyRotation(const double* attitude,  const Matrix& beamsVF)
{
    Matrix beamsMF = beamsVF;
    double cphi = cos(attitude[0]);
    double sphi = sin(attitude[0]);
    double ctheta = cos(attitude[1]);
    double stheta = sin(attitude[1]);
    double cpsi = cos(attitude[2]);
    double spsi = sin(attitude[2]);
    double stheta_sphi = stheta * sphi;
    double stheta_cphi = stheta * cphi;

    double R11 = cpsi * ctheta;
    double R12 = spsi * ctheta;
    double R13 = -stheta;
    double R21 = -spsi * cphi + cpsi * stheta_sphi;
    double R22 = cpsi * cphi + spsi * stheta_sphi;
    double R23 = ctheta * sphi;
    double R31 = spsi * sphi + cpsi * stheta_cphi;
    double R32 = -cpsi * sphi + spsi * stheta_cphi;
    double R33 = ctheta * cphi;

    for(int i = 1; i <= beamsVF.Ncols(); i++) {
        beamsMF(1, i) = R11 * beamsVF(1, i) + R21 * beamsVF(2, i) + R31 * beamsVF(3, i);

        beamsMF(2, i) = R12 * beamsVF(1, i) + R22 * beamsVF(2, i) + R32 * beamsVF(3, i);

        beamsMF(3, i) = R13 * beamsVF(1, i) + R23 * beamsVF(2, i) + R33 * beamsVF(3, i);
    }

    return beamsMF;
}

Matrix applyTranslation(const double* translation,  const Matrix& beamsVF)
{
    Matrix beamsMF = beamsVF;

    for(int i = 1; i <= beamsVF.Ncols(); i++) {
        beamsMF(1, i) = beamsVF(1, i) + translation[1];

        beamsMF(2, i) = beamsVF(2, i) + translation[2];

        beamsMF(3, i) = beamsVF(3, i) + translation[3];
    }

    return beamsMF;

}

// This is only for inputs mapped to mbtrnpp output
// It probably doesn't make sense to filter DVL beams
// using mbtrnpp, since it assumes they are distributed
// in a linear array.
void transform_dvl(trn::bath_info *bi, trn::att_info *ai, dvlgeo *geo, mb1_t *r_snd)
{
    if(NULL == geo || geo->beam_count<=0){
        fprintf(stderr, "%s - geometry error : beams<=0\n", __func__);
        return;
    }
    if(NULL == r_snd || NULL == ai|| NULL == bi){
        fprintf(stderr, "%s - ERR invalid argument bi[%p] ai[%p] snd[%p]\n", __func__, bi, ai, r_snd);
        return;
    }

    // number of beams read (<= nominal beams)
    int nbeams = bi->beam_count();

    // vehicle attitude (relative to NED)
    // r/p/y (phi/theta/psi)
    // MB1 assumes vehicle frame, not world frame (i.e. exclude heading)
    double VW[3] = {ai->roll(), ai->pitch(), 0};

    // sensor mounting angles (relative to vehicle, radians)
    // 3-2-1 euler angles, r/p/y  (phi/theta/psi)
    // wrt sensor mounted across track, b[0] port, downward facing
    double RSV[3] = { DTR(geo->svr_deg[0]), DTR(geo->svr_deg[1]), DTR(geo->svr_deg[2])};

    // sensor mounting translation offsets (relative to vehicle CRP, meters)
    // +x: fwd +y: stbd, +z:down (aka FSK, fwd,stbd,keel)
    // TODO T is for transform use rSV
    double TSV[3] = {geo->svt_m[0], geo->svt_m[1], geo->svt_m[2]};

    // beam components in reference sensor frame (mounted center, across track)
    Matrix comp_RSF(3,nbeams);

    std::list<trn::beam_tup> beams = bi->beams_raw();
    std::list<trn::beam_tup>::iterator it;

    for(it=beams.begin(); it!=beams.end(); it++)
    {
        trn::beam_tup bt = static_cast<trn::beam_tup> (*it);
        double range = std::get<1>(bt);
        // beam number (0-indexed)
        int b = std::get<0>(bt);
        // beam components SF x,y,z
        // matrix row/col (1 indexed)
        int c = b+1;

        // ad : ith beam angle (deg)
        double yd = geo->yaw_rf[b];
        double pd = geo->pitch_rf[b];
        double yr = DTR(yd);
        double pr = DTR(pd);

        // beam components SF x,y,z w/ translation
        comp_RSF(1,c) = cos(yr);
        comp_RSF(2,c) = -sin(yr);
        comp_RSF(3,c) = sin(yr) + cos(pr);
        TRN_NDPRINT(5, "n[%3d] R[%7.2lf] X[%7.2lf] Y[%7.2lf] Z[%7.2lf] yd[%7.2lf] pd[%7.2lf] yr[%7.2lf] yr[%7.2lf]\n",
                    b,range,
                    comp_RSF(1,c), comp_RSF(2,c), comp_RSF(3,c),
                    yd, pd, yr, pr);
    }

    // apply coordinate transforms; order is significant:
    // apply mounting rotation
    Matrix beams_VF = applyRotation(RSV, comp_RSF);
    // apply mounting translation
    Matrix beams_TF = applyTranslation(TSV, beams_VF);
    // apply vehicle attitude (hdg, pitch, roll)
    Matrix beams_WF = applyRotation(VW, beams_TF);

    // fill in the MB1 record using transformed beams
    int k=0;
    for(it=beams.begin(); it!=beams.end(); it++,k++)
    {
        trn::beam_tup bt = static_cast<trn::beam_tup> (*it);
        // beam number (0-indexed)
        int b = std::get<0>(bt);
        double range = std::get<1>(bt);
        // beam components WF x,y,z
        // matrix row/col (1 indexed)
        int c = b + 1;
        r_snd->beams[k].beam_num = b;
        r_snd->beams[k].rhox = range * beams_WF(1,c);
        r_snd->beams[k].rhoy = range * beams_WF(2,c);
        r_snd->beams[k].rhoz = range * beams_WF(3,c);

        TRN_NDPRINT(5, "b[%3d] R[%7.2lf] rhox[%7.2lf] rhoy[%7.2lf] rhoz[%7.2lf] \n",
                    b,sqrt(r_snd->beams[k].rhox*r_snd->beams[k].rhox + r_snd->beams[k].rhoy*r_snd->beams[k].rhoy + r_snd->beams[k].rhoz*r_snd->beams[k].rhoz),
                    r_snd->beams[k].rhox,
                    r_snd->beams[k].rhoy,
                    r_snd->beams[k].rhoz
                    );
    }
}

// this is only called for inputs mapped to mbtrnpp output
void transform_deltat(trn::bath_info *bi, trn::att_info *ai, mbgeo *geo, mb1_t *r_snd)
{
    if(NULL == geo || geo->beam_count<=0){
        fprintf(stderr, "%s - geometry error : beams<=0\n", __func__);
        return;
    }
    if(NULL == r_snd || NULL == ai|| NULL == bi){
        fprintf(stderr, "%s - ERR invalid argument bi[%p] ai[%p] snd[%p]\n", __func__, bi, ai, r_snd);
        return;
    }
    // number of beams read (<= nominal beams)
    int nbeams = bi->beam_count();

    // vehicle attitude (relative to NED, radians)
    // r/p/y  (phi/theta/psi)
    // MB1 assumes vehicle frame, not world frame (i.e. exclude heading)
    double VW[3] = {ai->roll(), ai->pitch(), 0};

    // sensor mounting angles (relative to vehicle, radians)
    // 3-2-1 euler angles, r/p/y  (phi/theta/psi)
    // wrt sensor mounted across track, b[0] port, downward facing
    double RSV[3] = { DTR(geo->svr_deg[0]), DTR(geo->svr_deg[1]), DTR(geo->svr_deg[2])};

    // sensor mounting translation offsets (relative to vehicle CRP, meters)
    // +x: fwd +y: stbd, +z:down
    double TSV[3] = {geo->svt_m[0], geo->svt_m[1], geo->svt_m[2]};

    // beam swath angle
    double S = geo->swath_deg;
    // angle between PI and start angle
    double K = (180. - S)/2.;
    // beam angle increment
    double e = S/geo->beam_count;

    // beam components in reference sensor frame
    // (i.e., directional cosine unit vectors)
    Matrix comp_RSF(3,nbeams);

    std::list<trn::beam_tup> beams = bi->beams_raw();
    std::list<trn::beam_tup>::iterator it;
    TRN_NDPRINT(5, "roll[%.2lf] pitch[%.2lf%s] hdg[%.2lf (%.2lf)] SVR[%.2lf, %.2lf, %.2lf] S[%.2lf] K[%.2lf] e[%.2lf]\n",
                Math::radToDeg(VW[0]), Math::radToDeg(VW[1]), (ai->flags().is_set(trn::AF_INVERT_PITCH) ? " i" : " "), Math::radToDeg(VW[2]), Math::radToDeg(ai->heading()),
                Math::radToDeg(RSV[0]), Math::radToDeg(RSV[1]), Math::radToDeg(RSV[2]),
                S,K,e
                );

    for(it=beams.begin(); it!=beams.end(); it++)
    {
        trn::beam_tup bt = static_cast<trn::beam_tup> (*it);
        double range = std::get<1>(bt);
        // beam number (0-indexed)
        int b = std::get<0>(bt);
        // beam components SF x,y,z
        // matrix row/col (1 indexed)
        int c = b + 1;

        // ad : ith beam angle (deg)
        double ad = K + S - b*e;

        comp_RSF(1,c) = 0;
        comp_RSF(2,c) = cos(DTR(ad));
        comp_RSF(3,c) = sin(DTR(ad));

        TRN_NDPRINT(5, "n[%3d] R[%7.2lf] X[%7.2lf] Y[%7.2lf] Z[%7.2lf] ad[%7.2lf] ar[%7.2lf]\n",
                b,range,
                    comp_RSF(1,c), comp_RSF(2,c), comp_RSF(3,c),
                    ad,DTR(ad)
                );
    }
    // apply coordinate transforms; order is significant:
    // apply mounting rotation
    Matrix beams_VF = applyRotation(RSV, comp_RSF);
    // apply mounting translation
    Matrix beams_TF = applyTranslation(TSV, beams_VF);
    // apply vehicle attitude (hdg, pitch, roll)
    Matrix beams_WF = applyRotation(VW, beams_TF);

    // write beam data to MB1 sounding
    int k=0;
    for(it=beams.begin(); it!=beams.end(); it++,k++)
    {
        trn::beam_tup bt = static_cast<trn::beam_tup> (*it);
        // beam number (0-indexed)
        int b = std::get<0>(bt);
        double range = std::get<1>(bt);
        // beam components WF x,y,z
        // matrix row/col (1 indexed)
        int c = b + 1;
        r_snd->beams[k].beam_num = b;
        r_snd->beams[k].rhox = range * beams_WF(1,c);
        r_snd->beams[k].rhoy = range * beams_WF(2,c);
        r_snd->beams[k].rhoz = range * beams_WF(3,c);

        TRN_NDPRINT(5, "b[%3d] R[%7.2lf] rhox[%7.2lf] rhoy[%7.2lf] rhoz[%7.2lf] ax[%6.2lf] ay[%6.2lf] az[%6.2lf]\n",
                    b,sqrt(r_snd->beams[k].rhox*r_snd->beams[k].rhox + r_snd->beams[k].rhoy*r_snd->beams[k].rhoy + r_snd->beams[k].rhoz*r_snd->beams[k].rhoz),
                    r_snd->beams[k].rhox,
                    r_snd->beams[k].rhoy,
                    r_snd->beams[k].rhoz,
                    (range==0. ? 0. : Math::radToDeg(acos(r_snd->beams[k].rhox/range))),
                    (range==0. ? 0. : Math::radToDeg(acos(r_snd->beams[k].rhoy/range))),
                    (range==0. ? 0. : Math::radToDeg(acos(r_snd->beams[k].rhoz/range)))
                    );
    }
}

void mbest_tostream(std::ostream &os, trnu_pub_t *mbest, int wkey=15, int wval=1)
{
    os << "--- MB Update OK---" << "\n";

    os << std::fixed << std::setprecision(3);
    os << "POS [t,x,y,z,cov(0,2,5,1)]:";
    os << std::setprecision(3);
    os << mbest->est[0].time << ",";
    os << mbest->est[0].x << "," << mbest->est[0].y << "," << mbest->est[0].z;
    os << mbest->est[0].cov[0] << "," << mbest->est[0].cov[1] << ",";
    os << mbest->est[0].cov[2] << "," << mbest->est[0].cov[3] << "\n";

    os << "MLE [t,x,y,z,cov(0,2,5,1)]:";
    os << std::setprecision(3);
    os << mbest->est[1].time << ",";
    os << mbest->est[1].x << "," << mbest->est[1].y << "," << mbest->est[0].z;
    os << mbest->est[1].cov[0] << "," << mbest->est[1].cov[1] << ",";
    os << mbest->est[1].cov[2] << "," << mbest->est[1].cov[3] << "\n";

    os << std::fixed << std::setprecision(3);
    os << "MMSE [t,x,y,z,cov(0,2,5,1)]:";
    os << std::setprecision(3);
    os << mbest->est[2].time << ",";
    os << mbest->est[2].x << "," << mbest->est[2].y << "," << mbest->est[0].z;
    os << mbest->est[2].cov[0] << "," << mbest->est[2].cov[1] << ",";
    os << mbest->est[2].cov[2] << "," << mbest->est[2].cov[3] << "\n";

    os << std::setw(wkey) << "reinit_count:" << std::setw(wkey) << mbest->reinit_count << "\n";
    os << std::setw(wkey) << "reinit_tlast:" << std::setw(wkey) << mbest->reinit_tlast << "\n";
    os << std::setw(wkey) << "filter_state:" << std::setw(wkey) << mbest->filter_state << "\n";
    os << std::setw(wkey) << "success:" << std::setw(wkey) << mbest->success << "\n";
    os << std::setw(wkey) << "is_converged:" << std::setw(wkey) << mbest->is_converged << "\n";
    os << std::setw(wkey) << "is_valid:" << std::setw(wkey) << mbest->is_valid << "\n";
    os << std::setw(wkey) << "mb1_cycle:" << std::setw(wkey) << mbest->mb1_cycle << "\n";
    os << std::setw(wkey) << "ping_number:" << std::setw(wkey) << mbest->ping_number << "\n";
    os << std::setw(wkey) << "mb1_time:" << std::setw(wkey) << mbest->mb1_time << "\n";
    os << std::setw(wkey) << "update_time:" << std::setw(wkey) << mbest->update_time << "\n";
}

std::string  mbest_tostring(trnu_pub_t *mbest, int wkey=15, int wval=1)
{
    std::ostringstream ss;
    mbest_tostream(ss, mbest, wkey, wval);
    return ss.str();
}

void mbest_show(trnu_pub_t *mbest, int wkey=15, int wval=18)
{
    mbest_tostream(std::cerr, mbest, wkey, wval);
}

int cb_update_mb1(void *pargs)
{
    static uint32_t cx=0;
    int retval = -1;
    TRN_NDPRINT(3, "%s:%d >>> Callback triggered <<<\n", __func__, __LINE__);
    callback_res_t *cb_res = static_cast<callback_res_t *>(pargs);
    trn::trnxpp *xpp = cb_res->xpp;//static_cast<trn::trnxpp *>(pargs);
    app_cfg *cfg = cb_res->cfg;

    cfg->stats().mb_cb_n++;

//    TRN_NDPRINT(4, "%s:%d >>> update_mb1 <<<\n", __func__, __LINE__);

    bool streams_ok=true;
    trn::trn_lcm_input *bs = xpp->get_input(xpp->ctx(trn::CTX_MBTRN).bath_input());
    trn::trn_lcm_input *ns = xpp->get_input(xpp->ctx(trn::CTX_MBTRN).nav_input());
    trn::trn_lcm_input *as = xpp->get_input(xpp->ctx(trn::CTX_MBTRN).att_input());
    trn::trn_lcm_input *vs = xpp->get_input(xpp->ctx(trn::CTX_MBTRN).vel_input());

    if(bs==nullptr){
        fprintf(stderr,"%s:%d WARN - bath input invalid s[%p]\n", __func__, __LINE__, bs);
        streams_ok=false;
    }
    if(ns==nullptr){
        fprintf(stderr,"%s:%d WARN - nav input invalid s[%p]\n", __func__, __LINE__, ns);
        streams_ok=false;
    }
    if(vs==nullptr){
        fprintf(stderr,"%s:%d WARN - vel input invalid s[%p]\n", __func__, __LINE__, vs);
        streams_ok=false;
    }
    if(as==nullptr){
        fprintf(stderr,"%s:%d WARN - att input invalid s[%p]\n", __func__, __LINE__, as);
        streams_ok=false;
    }

    trn::bath_input *bp = nullptr;
    trn::nav_input *np = nullptr;
    trn::vel_input *vp = nullptr;
    trn::att_input *ap = nullptr;
    if(streams_ok){
        bp = dynamic_cast<trn::bath_input *>(bs);
        np = dynamic_cast<trn::nav_input *>(ns);
        vp = dynamic_cast<trn::vel_input *>(vs);
        ap = dynamic_cast<trn::att_input *>(as);
        if(bp==nullptr){
            fprintf(stderr,"%s:%d WARN - bath IF invalid p[%p]\n", __func__, __LINE__, bp);
            streams_ok=false;
        }
        if(vp==nullptr){
            fprintf(stderr,"%s:%d WARN - vel IF invalid p[%p]\n", __func__, __LINE__, vp);
            streams_ok=false;
        }
        if(np==nullptr){
            fprintf(stderr,"%s:%d WARN - nav IF invalid p[%p]\n", __func__, __LINE__, bp);
            streams_ok=false;
        }
        if(ap==nullptr){
            fprintf(stderr,"%s:%d WARN - att IF invalid p[%p]\n", __func__, __LINE__, ap);
            streams_ok=false;
        }
    }

    trn::bath_info *bi = nullptr;
    trn::nav_info *ni = nullptr;
    trn::att_info *ai = nullptr;
    trn::vel_info *vi = nullptr;

    if(streams_ok){
        bi = bp->bath_inst();
        ni = np->nav_inst();
        ai = ap->att_inst();
        vi = vp->vel_inst();

        if(bi==nullptr){
            fprintf(stderr,"%s:%d WARN - bath info invalid i[%p]\n", __func__, __LINE__, bi);
            streams_ok=false;
        }
        if(vi==nullptr){
            fprintf(stderr,"%s:%d WARN - vel info invalid i[%p]\n", __func__, __LINE__, vi);
            streams_ok=false;
        }
        if(ni==nullptr){
            fprintf(stderr,"%s:%d WARN - nav info invalid i[%p]\n", __func__, __LINE__, bi);
            streams_ok=false;
        }
        if(ai==nullptr){
            fprintf(stderr,"%s:%d WARN - att info invalid i[%p]\n", __func__, __LINE__, ai);
            streams_ok=false;
        }
    }

    if(streams_ok){
        TRN_NDPRINT(4, "BATHINST.%s : %s\n",xpp->ctx(trn::CTX_MBTRN).bath_input().c_str(), bi->bathstr());

        size_t n_beams = bi->beam_count();
        if(n_beams>0){
            ai->flags().set(trn::AF_INVERT_PITCH);
//            ai->flags().set(trn::AF_INVERT_ROLL);
            mb1_t *snd = mb1_new(n_beams);
            snd->hdg = ai->heading();
            snd->depth = ni->depth();
            snd->lat = ni->lat();
            snd->lon = ni->lon();
            snd->type = MB1_TYPE_ID;
            snd->size = MB1_SOUNDING_BYTES(n_beams);
            snd->nbeams = n_beams;
            snd->ping_number = cx;
            snd->ts = ni->time_usec()/1e6;
            std::list<trn::beam_tup> beams = bi->beams_raw();
            std::list<trn::beam_tup>::iterator it;

            if(bp->bath_input_type()==trn::BT_DVL)
            {
                transform_dvl(bi, ai, cfg->dvl_geo(), snd);
            } else if(bp->bath_input_type()==trn::BT_DELTAT)
            {
                transform_deltat(bi, ai, cfg->mb_geo(), snd);
            } else {
                fprintf(stderr,"%s:%d ERR - unsupported input_type[%d] beam transformation invalid\n", __func__, __LINE__, bp->bath_input_type());
            }

            mb1_set_checksum(snd);

            if(cfg->debug()>=4){
                mb1_show(snd, (cfg->debug()>=5 ? true: false), 5);
            }

            xpp->publish_mb1((byte *)snd,snd->size);
            cfg->stats().mb_pub_n++;
            write_csv(xpp->ctx(trn::CTX_MBTRN).csv_file(), bi, ai, ni, vi);
            cfg->stats().mb_csv_n++;

            retval=0;
            if(cfg->pub_mb1()){
                pcf::lcm_publisher *pub = xpp->get_pub("MB1_PUB");
                if(pub != nullptr){
                    trn::trn_mb1_t mb1_msg;
                    trn::trn_msg_utils::mb1_to_lcm(mb1_msg, snd);
                    pub->publish(mb1_msg);
                    cfg->stats().mb_pub_mb1_n++;
               }
            }

            udpm_sub_t *trnum_cli = xpp->ctx(trn::CTX_MBTRN).udpm_sub();
            if(NULL != trnum_cli && cfg->pub_mbest())
            {
                int test_con=0;
                if(!udpms_is_connected(trnum_cli))
                {
                    TRN_NDPRINT(4, "connecting TRNUM client\n");
                    cfg->stats().mb_cli_dis++;
                    test_con=udpms_connect(trnum_cli, true, false, false);
                    if(test_con == 0){
                        cfg->stats().mb_cli_con++;
                    }
               }
                if(test_con == 0){
                    byte iobuf[512]={0};
                    memset(iobuf, 0, 512);
                    TRN_NDPRINT(4, "TRNUM client listening...\n");
                    int64_t test = udpms_listen(trnum_cli, iobuf, 512, 1000, 0);
                    if(test>0){
                        cfg->stats().mb_est_n++;
                        TRN_NDPRINT(4, "TRNUM update -> LCM...\n");
                        trn::trnupub_t trnu_msg;
                        trn::trn_msg_utils::trnupub_to_lcm(trnu_msg, (trnu_pub_t *)iobuf);
                        trnu_pub_t *x = (trnu_pub_t *)iobuf;
                        if(x->success != 0){
                            cfg->stats().mb_est_ok_n++;
                        }
                        pcf::lcm_publisher *pub = xpp->get_pub("MB1_EST");
                        pub->publish(trnu_msg);
                        cfg->stats().mb_pub_est_n++;
                    }else{
                        TRN_NDPRINT(4, "TRNUM no update\n");
                    }
                }else{
                    TRN_NDPRINT(4, "TRNUM not connected\n");
                }
            }
            cx++;
            mb1_destroy(&snd);
        }
    }

    delete bi;
    delete ai;
    delete ni;
    delete vi;

    return retval;
}

// parse the context specifier of an input specifier
// - creates input
// - optionally adds semaphore
// - sets provider inputs for channel
static void s_parse_ctx( char *i_ctx, const char *chan, callback_res_t *cb_res)
{
    TRN_NDPRINT(3, "%s:%d - *** parsing chan[%s] ctx[%s] ***\n",__func__, __LINE__, chan, i_ctx);
    void *vp_cbres = static_cast<void *>(cb_res);
    trn::trnxpp *xpp = cb_res->xpp;
//    app_cfg *cfg = cb_res->cfg;

    char *ctx = strtok(i_ctx,"/");
    char *prv = strtok(NULL,"/");
    char *sem = strtok(NULL,"/");
    char *cb=NULL;
    char *to=NULL;
    int to_val = 100;

    if(sem !=NULL){
        cb = strtok(sem,",");
        to = strtok(NULL,",");
        to_val = (to!=NULL ? atoi(to) : 100);
    }

    TRN_NDPRINT(5, "%s:%d - ctx %s\n",__func__, __LINE__, ctx);
    TRN_NDPRINT(5, "%s:%d - prv %s\n",__func__, __LINE__, prv);
    TRN_NDPRINT(5, "%s:%d - sem %s\n",__func__, __LINE__, sem);
    TRN_NDPRINT(5, "%s:%d -  cb %s\n",__func__, __LINE__, cb);
    TRN_NDPRINT(5, "%s:%d -  to %s/%d\n",__func__, __LINE__, to, to_val);

    trn::ctx_id_t ctx_id = trn::CTX_MBTRN;
    if(strstr(ctx,"trnsvr")!=NULL)
        ctx_id = trn::CTX_TRNSVR;

    trn::trn_lcm_input *listener = xpp->get_input(chan);
    if(listener == NULL){
        // create input (chan, depth) if it doesn't exist
        listener = xpp->create_input(chan, 10);
        TRN_NDPRINT(2, "%s:%d - add input chan[%s] @[%p]\n",__func__, __LINE__, chan, listener);
        if(listener == nullptr){
            fprintf(stderr,"%s:%d ERR - NULL create_input returned NULL listener - check configuration for chan [%s]\n", __func__, __LINE__, chan);

        }
        // add input
        xpp->add_input(chan, listener);
    }

    // add sem if specified (chan, to, cb, pargs)
    if(NULL != cb && strcmp(cb,"pubmb1")==0){
        TRN_NDPRINT(2, "%s:%d - add sem chan[%s] cb[%s] \n",__func__, __LINE__, chan, cb);
        xpp->list_add_sem(std::string(chan), to_val, cb_update_mb1, vp_cbres);
    } else if(NULL != cb && strcmp(cb,"pubtrn")==0){
        TRN_NDPRINT(2, "%s:%d - add sem chan[%s] cb[%s] \n",__func__, __LINE__, chan, cb);
        xpp->list_add_sem(std::string(chan), to_val, cb_update_trncli, vp_cbres);
    } else {
        TRN_NDPRINT(2, "%s:%d - WARN no sem chan[%s] cb[%s]\n",__func__, __LINE__, chan, cb);
    }

    if(prv != NULL){
        // set providers for ctx
        // set ctx bath input
        if(prv!=NULL && strstr(prv,"b")!=NULL){
            if(listener->provides_bath())
                xpp->ctx(ctx_id).set_bath_input(chan);
            else
                TRN_NDPRINT(2, "WARN - input does not provide bathymetry\n");
        }

        if(prv!=NULL && strstr(prv,"v")!=NULL){
            if(listener->provides_vel())
                xpp->ctx(ctx_id).set_vel_input(chan);
            else
                TRN_NDPRINT(2, "WARN - input does not provide velocity\n");
        }

        if(prv!=NULL && strstr(prv,"a")!=NULL){
            if(listener->provides_att())
                xpp->ctx(ctx_id).set_att_input(chan);
            else
                TRN_NDPRINT(2, "WARN - input does not provide attitude\n");
        }

        if(prv!=NULL && strstr(prv,"n")!=NULL){
            if(listener->provides_nav())
                xpp->ctx(ctx_id).set_nav_input(chan);
            else
                TRN_NDPRINT(2, "WARN - input %s does not provide navigation\n", chan);
        }
    }
}

// parse input specifier strings from --input
// input specifiers use format:
//      chan:<ctx_spec>[:<ctx_spec>...]
//    where
//      chan     : LCM channel name
//      ctx_spec : context specifier using format:
//        ctx/par[/sem]
//    where:
//      ctx : Context [mbtrn, trnsvr]
//      par : Use this channel to provide
//            one or more parameters in
//            this context(comma-separated)
//              a:attitude
//              b:bathymetry
//              n:navigation
//              v:velocity
//      sem : Optionally specify a semaphore callback
//            and/or override timeout (comma separated)
//            Valid callback values include:
//              pubmb1: MB1 publisher callback
//              pubtrn: TRNSVR publisher callback
//            Semaphore timeout values are in msec

// to map channels to input and semaphore configuration
// examples:
// IDT_PUB:mbtrn/b/pubmb1,100
// GSS_NAV_SOLUTION:mbtrn/a,n:trnsvr/a,n
// OPENINS_DVL_STAT:mbtrn/v:trnsvr/b,v/pubtrn,100
static void s_parse_input(std::string in_spec, callback_res_t *cbres)
{
    char *acpy = strdup(in_spec.c_str());
    char *chan = strtok(acpy,":");
    char *ctxa = strtok(NULL,":");
    char *ctxb = strtok(NULL,":");

    TRN_NDPRINT(5, "%s:%d **********************\n",__func__, __LINE__);
    TRN_NDPRINT(5, "%s:%d - chan %s\n",__func__, __LINE__, chan);
    TRN_NDPRINT(5, "%s:%d - ctxa %s\n",__func__, __LINE__, ctxa);
    TRN_NDPRINT(5, "%s:%d - ctxb %s\n",__func__, __LINE__, ctxb);

    if(chan != NULL)
    {
        if(ctxa != NULL)
        {
            s_parse_ctx(ctxa, chan, cbres);
        }

        if(ctxb != NULL)
        {
            s_parse_ctx(ctxb, chan, cbres);
        }
    }

    free(acpy);
}

static void s_init_logging(app_cfg &cfg, int argc, char **argv)
{

    // generate message log path (path/xppm-<session>.log)
    ostringstream ss;
    ss << cfg.logdir().c_str() << "/";
    ss << "xpp-msg-";
    ss << cfg.session_string().c_str();
    ss << ".log";

    // add an message log (reference using key "mlog")
    cfg.mlog().add_file("mlog",ss.str().c_str(),"a+",true);

    // set level for PNDEBUG evaluation
    cfg.mlog().set_level(cfg.debug());

    // set destinations and formats
    // optionally, set according to application options

    // profile destination keys
    std::vector<std::string> debug_keys = {"stderr"};
    std::vector<std::string> verbose_keys = {"stderr"};
    std::vector<std::string> info_keys = {"stderr"};
    std::vector<std::string> event_keys = {"mlog"};
    std::vector<std::string> warn_keys = {"stderr", "mlog"};
    std::vector<std::string> error_keys = {"stderr", "mlog"};
    std::vector<std::string> dfl_keys = {"stderr", "mlog"};

    // profile formats
    flag_var<uint32_t> rec_fmt = (logu::LF_TIME_ISO8601 | logu::LF_LVL_SHORT | logu::LF_SEP_COMMA | logu::LF_DEL_UNIX);
    flag_var<uint32_t> dfl_fmt = (logu::LF_TIME_POSIX_MS | logu::LF_SEP_COMMA | logu::LF_DEL_UNIX);

    // define profiles per level
    cfg.mlog().set_profile(logu::LL_DEBUG, debug_keys, rec_fmt);
    // intentionally omit verbose (should use default profile)
    // foo.set_profile(logu::LL_VERBOSE, verbose_keys, rec_fmt);
    cfg.mlog().set_profile(logu::LL_INFO, info_keys, rec_fmt);
    cfg.mlog().set_profile(logu::LL_EVENT, event_keys, rec_fmt);
    cfg.mlog().set_profile(logu::LL_WARN, warn_keys, rec_fmt);
    cfg.mlog().set_profile(logu::LL_ERR, error_keys, rec_fmt);
    // if default profile is unset, just goes to stderr as-is
    cfg.mlog().set_profile(logu::LL_DFL, dfl_keys, dfl_fmt);

    LU_ULOG(cfg.mlog(),"mlog","# trnxpp_app message log session start %s\n", cfg.session_string().c_str());

    // log command line args
    ostringstream sc;
    sc << "cmdline:" << argv[0] << " ";
    for(int i=1; i<argc; i++){
        sc << argv[i];
        if(i<argc-1)
            sc<<",";
    }
    LU_PEVENT(cfg.mlog(), "%s", sc.str().c_str());

    // log environment
    sc.str("");
    sc << "env:\n";
    char *ep = getenv("TRN_HOST");
    sc << "TRN_HOST =" << (NULL==ep ? "" : ep ) << "\n";
    ep = getenv("TRN_LOGFILES");
    sc << "TRN_LOGFILES =" << (NULL==ep ? "" : ep ) << "\n";
    ep = getenv("TRN_DATAFILES");
    sc << "TRN_DATAFILES =" << (NULL==ep ? "" : ep )<< "\n";
    ep = getenv("TRN_MAPFILES");
    sc << "TRN_MAPFILES =" << (NULL==ep ? "" : ep ) << "\n";
    ep = getenv("TRN_GROUP");
    sc << "TRN_GROUP =" << (NULL==ep ? "" : ep ) << "\n";
    ep = getenv("LCM_DEFAULT_URL");
    sc << "LCM_DEFAULT_URL =" << (NULL==ep ? "" : ep ) << "\n";
    ep = getenv("CLASSPATH");
    sc << "CLASSPATH =" << (NULL==ep ? "" : ep ) << "\n";
    LU_PEVENT(cfg.mlog(), "%s", sc.str().c_str());

    return;
}

#ifdef WITH_TEST_STREAMS
void handle_test_streams(pcf::lcm_publisher &signalPub, pcf::lcm_publisher &stringPub, trn::trnxpp &xpp, trn::mb1_server *mb1svr, app_cfg &cfg)
{
    lcm_pcf::signal_t signalMsg;
    lcm_pcf::string_t stringMsg;
    // publish messages for xpp to receive
    signalPub.publish(signalMsg);
    stringPub.publish(stringMsg);

    // update message contents
    signalMsg.signal+=1.0;
    stringMsg.val = "Hello from stringPub! - " +
    std::to_string(signalPub.get_sequence());

    // check the semaphore(s), trigger callback if notified
    // (callbacks should be fast and non-blocking)
    int r_stat=0;
    xpp.test_sem("RAW_SIGNAL", 100, cb_raw_sig, r_stat, &xpp);
    xpp.test_sem("STRING_MSG", 100, cb_string, r_stat, &xpp);

    if(cfg.fakemb1())
    {
        int test_beams = 32;
        mb1_t *snd = nullptr;
        size_t len = MB1_SOUNDING_BYTES(test_beams);
        snd = s_get_test_sounding(snd, test_beams);
        mb1svr->publish((byte *)snd, len);
        mb1_destroy(&snd);
    }
}
#endif

void update_cycle_stats(app_cfg &cfg)
{
    static double stat_tmr = logu::utils::dtime();

    double now = logu::utils::dtime();
    cfg.stats().cycle_n++;
    cfg.stats().end_time = now;

    if(cfg.stat_period()>0)
    {
        if((now-stat_tmr) > cfg.stat_period())
        {
            // log stats
            LU_PEVENT(cfg.mlog(),"stats:\n%s\n",cfg.stats().tostring().c_str());

            if(cfg.stat_level() <= cfg.debug())
            {
                // show stats (per debug level)
                cfg.stats().show();
            }
            stat_tmr = now;
        }
    }
}

void copy_config(app_cfg &cfg)
{
    // copy terrainAid.cfg
    ostringstream ss;
    ss << "cp " << cfg.trn_cfg() << " " << cfg.logdir().c_str();
    ss << "/terrainAid-";
    ss << cfg.session_string().c_str();
    ss << ".cfg";
    if(system(ss.str().c_str()) != 0)
    {
        fprintf(stderr,"%s:%d - ERR config copy failed [%s] [%d/%s]\n",__func__, __LINE__,
                ss.str().c_str(), errno, strerror(errno));
    }

    ss.str(std::string());
    ss << "cp " << cfg.app_cfg_path() << " " << cfg.logdir().c_str();
    ss << "/trnxpp-";
    ss << cfg.session_string().c_str();
    ss << ".cfg";
    if(system(ss.str().c_str()) != 0 )
    {
        fprintf(stderr,"%s:%d - ERR config copy failed [%s] [%d/%s]\n",__func__, __LINE__,
                ss.str().c_str(), errno, strerror(errno));
    }
}

int main(int argc, char **argv)
{
    struct sigaction saStruct;
    sigemptyset(&saStruct.sa_mask);
    saStruct.sa_flags = 0;
    saStruct.sa_handler = s_termination_handler;
    sigaction(SIGINT, &saStruct, NULL);

    app_cfg cfg;

    cfg.stats().start_time = logu::utils::dtime();

    setenv("XPP_SESSION",cfg.session_string().c_str(), false);

    // parse command line (first pass for config file)
    cfg.parse_args(argc, argv);

    // configure debug output (for parsing debug)
    trn_debug::get()->set_debug(cfg.debug());
    trn_debug::get()->set_verbose(cfg.verbose());

    if(cfg.config_set() > 0){
        // parse config file
        cfg.parse_file(cfg.app_cfg_path());
    }

    // reparse command line (should override config options)
    cfg.parse_args(argc, argv);

    // Start logger
    s_init_logging(cfg, argc, argv);
    LU_PEVENT(cfg.mlog(), "app_cfg:\n%s\n",cfg.tostring().c_str());

    // pass debug/verbose output config to trn_debug
    trn_debug::get()->set_verbose(cfg.verbose());
    trn_debug::get()->set_debug(cfg.debug());

    if(cfg.debug()>0){
        cfg.show();
    }

    copy_config(cfg);

    LU_PEVENT(cfg.mlog(), "session start [%s]",cfg.session_string().c_str());

    // get an LCM instance
    pcf::lcm_interface lcm("");

    LU_PEVENT(cfg.mlog(), "lcm initialized");

    // for testing, add publishers
#ifdef WITH_TEST_STREAMS
    pcf::lcm_publisher signalPub("RAW_SIGNAL");
    pcf::lcm_publisher stringPub("STRING_MSG");
    lcm.add_publisher(signalPub);
    lcm.add_publisher(stringPub);
#endif

    trn::trnxpp xpp(lcm);
    callback_res_t cb_res = {&cfg, &xpp};

    // configure TRN client, connect to trn-server
    TrnClient trn_client = TrnClient("localhost",TRNCLI_PORT_DFL);
    trn_client.loadCfgAttributes(cfg.trn_cfg().c_str());
    xpp.ctx(trn::CTX_TRNSVR).set_trn_client(&trn_client);
    xpp.ctx(trn::CTX_TRNSVR).set_csv_path(cfg.trn_csv());
    FILE *trn_csv = xpp.ctx(trn::CTX_TRNSVR).csv_open();

    if(trn_csv != nullptr){
        fprintf(trn_csv,"# trnxpp TRN session start %s\n", cfg.session_string().c_str());
    } else {
        LU_PERROR(cfg.mlog(), "TRN CSV file open failed");
    }

    int tcc = xpp.trncli_connect(10,3,&g_interrupt);

    if(trn_client.is_connected()){
        LU_PEVENT(cfg.mlog(), "trn client connected");
        cfg.stats().trn_cli_con++;
    } else {
        LU_PERROR(cfg.mlog(), "trn client connect failed [%d]",tcc);
    }

    if(cfg.debug()>0){
        trn_client.show();
    }

    // Configure TRN update client, connect to mbtrnpp (UDP mcast)
    char *trnum_group = cfg.trnu_group();
    int trnum_port = cfg.trnu_port();
    int trnum_ttl = cfg.trnu_ttl();
    udpm_sub_t *trnum_cli = udpms_cnew(trnum_group, trnum_port, trnum_ttl);
    xpp.ctx(trn::CTX_MBTRN).set_udpm_sub(trnum_cli);
    xpp.ctx(trn::CTX_MBTRN).set_csv_path(cfg.mb1_csv());
    FILE *mb1_csv = xpp.ctx(trn::CTX_MBTRN).csv_open();

    if(trn_csv != nullptr){
        fprintf(mb1_csv,"# trnxpp MB1 session start %s\n", cfg.session_string().c_str());
    } else {
        LU_PERROR(cfg.mlog(), "MB CSV file open failed");
    }

    udpms_set_debug(cfg.debug());
    udpms_connect(trnum_cli, true, false, false);

    if(udpms_is_connected(trnum_cli)){
        LU_PEVENT(cfg.mlog(), "trnum_cli connected [%s:%d] ttl[%d]",trnum_group, trnum_port, trnum_ttl);
        cfg.stats().mb_cli_con++;
    } else {
        LU_PERROR(cfg.mlog(), "trnum_cli connect failed [%s:%d] ttl[%d] [%d/%s]",trnum_group, trnum_port, trnum_ttl, errno, strerror(errno));
    }

    // add LCM input streams (subscribers)
    // (trnxpp will release all inputs)

    std::list<std::string> inputs = cfg.input_list();
    std::list<std::string>::iterator it;
    for(it=inputs.begin(); it != inputs.end(); it++)
    {
        s_parse_input(static_cast<std::string>(*it), &cb_res);
    }

    // add publishers for LCM types produced
    // TRN server inputs
    xpp.add_pub("TRN_MOTN");
    xpp.add_pub("TRN_MEAS");
    // TRN server estimates (output)
    xpp.add_pub("TRN_EST");
    // mbtrnpp MB1 inputs
    xpp.add_pub("MB1_PUB");
    // mbtrnpp MB1 estimates (output)
    xpp.add_pub("MB1_EST");

#ifdef WITH_TEST_STREAMS
    // add a trn_lcm_input (base class):
    // xpp.add_input("STRING_MSG", 10);
    // use factory method to create generate input for specified channel
    trn::trn_lcm_input *str_listener = xpp.create_input("STRING_MSG", 10);
    xpp.add_input("STRING_MSG", str_listener);
    trn::trn_lcm_input *raw_listener = xpp.create_input("RAW_SIGNAL", 10);
    xpp.add_input("RAW_SIGNAL", raw_listener);
#endif

    // add semaphore(s) to be notified when
    // a message arrives on selected channel(s)
#ifdef WITH_TEST_STREAMS
    xpp.add_sem("RAW_SIGNAL");
    xpp.add_sem("STRING_MSG");
#endif

    // configure and connect MB1 server instance
    LU_PEVENT(cfg.mlog(), "configuring MB1 server [%s:%d]", cfg.host(), cfg.port());

    trn::mb1_server *mb1svr = new trn::mb1_server(cfg.host(), cfg.port());

    mb1svr->set_debug(cfg.debug());

    mb1svr->initialize(cfg.host(), cfg.port());
    mb1svr->connect_svr();

    // give trnxpp a server reference
    // to enable publishing from callbacks
    xpp.ctx(trn::CTX_MBTRN).set_mb1_server(mb1svr);

    // formatted summary to log
    LU_PEVENT(cfg.mlog(), "xpp starting:\n%s\n", xpp.tostring().c_str());

    // start listening
    xpp.start();

    LU_PEVENT(cfg.mlog(), "starting main loop");
    // loop until
    int cycles = 0;

    while(!g_interrupt){

        // iterate over sem list, test/invoke callbacks
        int n_tested=0, n_called=0, n_error=0;
        xpp.list_test_sem(true, n_tested, n_called, n_error);

        cfg.stats().sem_call_n += n_called;
        cfg.stats().sem_test_n += n_tested;
        cfg.stats().sem_err_n += n_error;

#ifdef WITH_TEST_STREAMS
        handle_test_streams(signalPub, stringPub, xpp, cfg);
#endif
        update_cycle_stats(cfg);

        cycles += 1;

        // exit if cycle counter expired
        if(cfg.cycles()>0 && cycles>cfg.cycles()){
            break;
        }
        // exit if interrupted
        if(g_interrupt ){
            break;
        }
        // sleep if loop delay set
        if(cfg.delay()>0){
            sleep(cfg.delay());
        }
    }

    // stop listening
    xpp.stop();

    // record session end time
    cfg.stats().end_time = logu::utils::dtime();

    LU_PEVENT(cfg.mlog(), "xpp:\n%s\n", xpp.tostring().c_str());
    LU_PEVENT(cfg.mlog(), "stats:\n%s\n", cfg.stats().tostring().c_str());

    LU_PNDEBUG(cfg.mlog(), 2, "xpp:\n%s\n", xpp.tostring().c_str());
    LU_PNDEBUG(cfg.mlog(), 2, "stats:\n%s\n", cfg.stats().tostring().c_str());

    if(mb1svr!=nullptr)
        delete mb1svr;

    udpms_destroy(&trnum_cli);

    TNavConfig::release();
    trn_debug::get(true);

    LU_PEVENT(cfg.mlog(), "session ended");

    return 0;
}
