
/// @file trnxpp_cfg.cpp
/// @authors k. headley
/// @date 08nov2022

/// Summary: trnxpp application configuration code

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
#include <fcntl.h>
#include <memory.h>
#include <libgen.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <list>
#include <tuple>
#include <string>
#include "geo_cfg.hpp"
#include "udpm_sub.h"
#include "mb1_server.hpp"

#include "trn_debug.hpp"
#include "log_utils.hpp"

#ifndef TRNXPP_CFG_H
#define TRNXPP_CFG_H
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

class app_stats
{
public:
    app_stats()
    : start_time(0)
    , end_time(0)
    , cycle_n(0)
    , sem_test_n(0)
    , sem_call_n(0)
    , sem_err_n(0)
    , trn_cb_n(0)
    , trn_motn_n(0)
    , trn_meas_n(0)
    , trn_mle_n(0)
    , trn_mmse_n(0)
    , trn_csv_n(0)
    , trn_est_val_n(0)
    , trn_pub_motn_n(0)
    , trn_pub_meas_n(0)
    , trn_pub_est_n(0)
    , trn_pub_stat_n(0)
    , trn_est_ok_n(0)
    , trn_err_n(0)
    , trn_cli_con(0)
    , trn_cli_dis(0)
    , mb_cb_n(0)
    , mb_pub_n(0)
    , mb_csv_n(0)
    , mb_log_mb1_n(0)
    , mb_pub_mb1_n(0)
    , mb_pub_est_n(0)
    , mb_est_n(0)
    , mb_est_ok_n(0)
    , mb_err_n(0)
    , mb_cli_con(0)
    , mb_cli_dis(0)
    , err_plugin_n(0)
    , err_nobeams_n(0)
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
        os << std::setw(wkey) << "mb_log_mb1_n" << std::setw(wval) << mb_log_mb1_n <<"\n";
        os << std::setw(wkey) << "mb_pub_mb1_n" << std::setw(wval) << mb_pub_mb1_n <<"\n";
        os << std::setw(wkey) << "mb_pub_est_n" << std::setw(wval) << mb_pub_est_n <<"\n";
        os << std::setw(wkey) << "mb_est_n" << std::setw(wval) << mb_est_n <<"\n";
        os << std::setw(wkey) << "mb_est_ok_n" << std::setw(wval) << mb_est_ok_n <<"\n";
        os << std::setw(wkey) << "mb_err_n" << std::setw(wval) << mb_err_n <<"\n";
        os << std::setw(wkey) << "mb_cli_con" << std::setw(wval) << mb_cli_con <<"\n";
        os << std::setw(wkey) << "mb_cli_dis" << std::setw(wval) << mb_cli_dis <<"\n";
        os << std::setw(wkey) << "err_plugin_n" << std::setw(wval) << err_plugin_n <<"\n";
        os << std::setw(wkey) << "err_nobeams_n" << std::setw(wval) << err_nobeams_n <<"\n";
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
    int mb_log_mb1_n;
    int mb_pub_mb1_n;
    int mb_pub_est_n;
    int mb_est_n;
    int mb_est_ok_n;
    int mb_err_n;
    int mb_cli_con;
    int mb_cli_dis;

    int err_plugin_n;
    int err_nobeams_n;
};

class trnxpp_cfg
{
public:
    trnxpp_cfg()
    : mVerbose(false)
    , mGInterrupt(nullptr)
    , mDebug(0)
    , mCycles(-1)
    , mDelay(0)
    , mFakeMB1(false)
    , mLogDirStr(".")
    , mMsgLog()
    , mStats()
    , mStatPeriod()
    , mConfigSet(false)
    {
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

    ~trnxpp_cfg()
    {
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
            {"delay", required_argument, NULL, 0},
            {"stats", required_argument, NULL, 0},
            {"logdir", required_argument, NULL, 0},
            {"fake-mb1", no_argument, NULL, 0},
            {"config", required_argument, NULL, 0},
            {"mb1pub", required_argument, NULL, 0},
            {"trncli", required_argument, NULL, 0},
            {"trn", required_argument, NULL, 0},
            {"input", required_argument, NULL, 0},
            {"sem", required_argument, NULL, 0},
            {"ctx", required_argument, NULL, 0},
            {NULL, 0, NULL, 0}
        };
        // reset optind
        optind=1;
        // process argument list
        while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1){

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
                        // config
                        if (strcmp("config", options[option_index].name) == 0) {
                            mAppCfg = std::string(optarg);
                        }
                        mConfigSet = true;
                        break;
                    }else{
                        // delay
                        if (strcmp("delay", options[option_index].name) == 0) {
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
                        // cycles
                        else if (strcmp("cycles", options[option_index].name) == 0) {
                            mCycles = atoi(optarg);
                        }
                        // trn
                        else if (strcmp("trn", options[option_index].name) == 0) {
                            add_to_str_list(mTrnList, optarg);
                        }
                        // input
                        else if (strcmp("input", options[option_index].name) == 0) {
                            add_to_str_list(mInputList, optarg);
                        }
                        // sem
                        else if (strcmp("sem", options[option_index].name) == 0) {
                            add_to_str_list(mSemList, optarg);
                        }
                        // ctx
                        else if (strcmp("ctx", options[option_index].name) == 0) {
                            add_to_str_list(mCtxList, optarg);
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
                trnxpp_cfg::show_help();
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
        " --help                : output help message\n"
        " --version             : output version info\n"
        " --config=s            : configuration file path\n"
        " --delay=u             : main loop delay\n"
        " --logdir=s            : log directory\n"
        " --cycles=u            : stop after u cycles (for debugging)\n"
        " --stats=f,d           : stats output period (log, decimal sec), level (console) \n"
        " --fake-mb1            : publish fake MB1 output\n"
        " --trn=<tspec>         : specify TRN output\n"
        " --sem=<sspec>         : specify semaphore callback\n"
        " --input=<ispec>       : specify input channel\n"
        " --ctx=<cspec>         : specify processing context (input/output mapping)\n"
        "\n"
        " Notes:\n"
        "\n"
        "Specfiers:\n"
        "\n"
        "  Specifiers are generally key=value pairs.\n"
        "  Keys are short strings naming a config parameter.\n"
        "  values are comma-delimited lists, using ':' as a sub-item delimiter.\n"
        "  Whitespace may separate list items, not sublist items.\n"
        "\n"
        "  # tspec: TRN specifier\n"
        "    trn=<UKEY>,<TYPE>,<HSPEC>\n"
        "\n"
        "    UKEY : unique key, string, no whitespace,alpha-num,-,_)\n"
        "    TYPE : [mbtrn|udpms|trnsvr]\n"
        "\n"
        "  # hspec: Host specifier\n"
        "    <ADDR>:<PORT>[:<TTL>]\n"
        "\n"
        "    ADDR : host IP address or multicast group, required\n"
        "    PORT : host IP port or multicast port, required\n"
        "    TTL  : multicast time to live, optional\n"
        "\n"
        "  # sspec: Semaphore specifier\n"
        "    sem=<CB_KEY>,chan:<CHAN>[,tmout:<TMOUT>]\n"
        "\n"
        "    CB_KEY : callback key, required, [cb_proto_trncli, cb_proto_mbtrn]\n"
        "    CHAN   : LCM channel name, required\n"
        "    TMOUT  : read timeout, optional\n"
        "\n"
        "  # ispec: Input specifier\n"
        "\n"
        "    input=<CHAN>,[<DEPTH>]<IARGS>\n"
        "    CHAN  : LCM channel, required\n"
        "    DEPTH : input queue depth, optional\n"
        "    IARGS : input arguments and/or options\n"
        "\n"
        "    invert-pitch:<BOOL> : invert attitude provider pitch angle, optional\n"
        "    <GSPEC>             : bathymetry provider geometry spec, required\n"
        "\n"
        "  # BOOL  : boolean value [0|1]\n"
        "  # GSPEC : Sensor geometry specifier\n"
        "      geo:<GTYPE>:<GARGS>...*\n"
        "      GTYPE : geometry type, required\n"
        "        <LIN>|<RADA>|<RADL>\n"
        "      GARGS : arguments, per GTYPE\n"
        "\n"
        "  # LIN : linear beam geometry\n"
        "\n"
        "      lin:<NBEAMS>:<SWATH>:<SVR>:<SVT>[:<RROT>]\n"
        "      NBEAMS : number of sonar beams\n"
        "      SWATH  : total beam angle\n"
        "      SVR    : sensor-vehicle rotation angles (r,p,y) deg\n"
        "               321 Euler angles, NED\n"
        "               r:roll (+stbd) p:pitch (+down) y:yaw (+stbd)\n"
        "        <ANGLE_D>,<ANGLE_D>,<ANGLE_D>\n"
        "      SVT    : sensor-vehicle translation distances (x,y,z) m\n"
        "               +x: fwd +y: stbd +z: down\n"
        "        <DIST_M>,<DIST_M>,<DIST_M>\n"
        "      RROT   : distance to arm rotation axis m (for OI toolsled)\n"
        "        <DIST_M>\n"
        "\n"
        "  # ANGLE_D : angle (deg)\n"
        "\n"
        "  # DIST_M : distance, (m)\n"
        "\n"
        "  # RADA : radial array beam geometry\n"
        "\n"
        "      rada:<NBEAMS>:<BARRAY>:<SVR>:<SVT>\n"
        "      NBEAMS : number of sonar beams\n"
        "\n"
        "  # BARRAY: radial array with regular geometry\n"
        "\n"
        "      A,<Yb>,<Yi>,<Pb>,<Pi>\n"
        "      Yb : Yaw angle begin (deg)\n"
        "      Yi : Yaw angle increment (deg)\n"
        "      Pb : Pitch angle begin (deg)\n"
        "      Pi : Pitch angle increment (deg)\n"
        "\n"
        "  # RADL  : radial list beam geometry\n"
        "\n"
        "      radl:<NBEAMS>:<BLIST>:<SVR>:<SVT>\n"
        "      NBEAMS : number of sonar beams\n"
        "\n"
        "  # BLIST: list of beam angles/pitches (irregular geometry)\n"
        "\n"
        "      L,<Yi>,<Pi>...\n"
        "      Yi : Yaw angle increment (deg)\n"
        "      Pi : Pitch angle increment (deg)\n"
        "      SVR    : sensor-vehicle rotation angles (roll, pitch, yaw) deg\n"
        "      SVT    : sensor-vehicle translation distances (x, y, z) m\n"
        "\n"
        "  # cspec: Context specifier\n"
        "\n"
        "    ctx:<CKEY>,<CALLBACK>,<DECMOD>,<TRN_KEY>,<UDPMS>,<CSV>,<TRN-CFG>,<INSPEC>,<LCM>\n"
        "\n"
        "  # CKEY     : unique context name, required, must be first argument\n"
        "  # CALLBACK : callback key, required\n"
        "\n"
        "      cb:<CB_KEY>\n"
        "  # DECMOD  : decimation modulus, optional\n"
        "\n"
        "      decmod:<INT>\n"
        "\n"
        "  # <INT> : integer\n"
        "  # TRN_KEY     : TRN output host key, required, must match trn definition\n"
        "\n"
        "      trn:<UKEY>\n"
        "  # UDPMS   : UDP mcast TRN key, optional, use with mbtrn TRN hosts, must match trn definition\n"
        "\n"
        "      udpms:<UKEY>\n"
        "  # CSV     : CSV output path, optional\n"
        "\n"
        "      csv:<PATH>\n"
        "  # TRN-CFG : terrain nav config file path, required for TRN server\n"
        "\n"
        "      trn-cfg:<PATH>\n"
        "  # IDX     : input index, as required by specified callback\n"
        "\n"
        "      idx:<INT>\n"
        "  # INSPEC  : one or more input spec\n"
        "\n"
        "     <TYPE>:<IDX>:<CHAN>\n"
        "\n"
        "  # TYPE : input type [bi|ni|ai|vi]\n"
        "  # IDX  : input index [int >= 0]\n"
        "  # CHAN : LCM channel, must match input definition\n"
        "  # PATH : file path\n"
        "\n"
        "  # LCM  : LCM output configuration\n"
        "\n"
        "    lcm:<LCM_FLAGS>\n"
        "    LCM_FLAGS : output flag mnemonics [mb1svr|mbest|trnmotn|trnmeas|trnest|trnstat]"
        "\n"
        "\n"
        "  Supported Input channels\n"
        "\n"
        "  Channel                LCM                 Provides\n"
        "  -------                ---                 --------\n"
        "  OPENINS_DVL_STAT       dvl_stat.lcm        bath, vel\n"
        "  IDT_PUB                idt_pub.lcm         bath\n"
        "  GSS_NAV_SOLUTION       nav_solution_t.lcm  nav, att\n"
        "  SONARDYNE_SPRINT_STAT  pcomms_t.lcm        nav, att\n"
        "\n"
        "  Supported Callback\n"
        "\n"
        "  Callback         Requirements\n"
        "  -------          -----------\n"
        "  cb_proto_trn     inputs: bath, nav, att, vel\n"
        "                   trn: trncli\n"
        "  cb_proto_mbtrn   inputs: bath, nav, att; output: mbtrnpp\n"
        "                   trn: mbtrnpp, udpms\n"
        "\n"
        " Examples:\n"
        "  trn=cherry,mb1pub,192.168.1.101:7007\n"
        "  trn=orange,udpms,$TRN_GROUP:7667:1\n"
        "  trn=grape,trncli,192.168.1.1:8001\n"
        "\n"
        "  input=IDT_PUB,10,lin:120:120:-65,0,-90:0,0,0\n"
        "  input=SONARDYNE_SPRINT_STAT,10,invert-pitch:1\n"
        "  input=GSS_NAV_SOLUTION,10,invert-pitch:0\n"
        "  input=OPENINS_DVL_STAT,10,radl:4:-45,-30,135,-30,45,-30,-135,-30:0,0,0:0,0,0\n"
        "\n"
        "  sem=cb:cb_proto_mbtrn,chan:IDT_PUB,tmout:100\n"
        "  sem=cb:cb_proto_trn,chan:OPENINS_DVL_STAT,tmout:100\n"
        "\n"

        "  ctx=key:mango,cb:cb_proto_mbtrn,decmod:3,trn:cherry,udpms:orange,csv:$TRN_LOGFILES/xpp-mb-$XPP_SESSION.csv,bi:0:IDT_PUB,bi:1:DVL_KEARFOTT_OI,ai:0:SONARDYNE_SPRINT_STAT,ni:0:GSS_NAV_SOLUTION,lcm:mb1svr|mbest\n"
        "\n"
        "  ctx=key:blueberry,cb:cb_proto_trn,decmod:1,trn:grape,trn-cfg:$TRN_DATAFILES/terrainAid.cfg,udpms:orange,csv:$TRN_LOGFILES/xpp-trn-$XPP_SESSION.csv,bi:0:IDT_PUB,bi:1:DVL_KEARFOTT_OI,ai:0:SONARDYNE_SPRINT_STAT,ni:0:GSS_NAV_SOLUTION,vi:0:OPENINS_DVL_STAT,lcm:trnmeas|trnmotn|trnest\n"
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

    static char *trim(char *src)
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

    static char *expand_env(char *src)
    {
        char *retval = NULL;

        if(NULL!=src && strlen(src)>0 ){
            char *obuf=NULL;
            size_t len = strlen(src)+1;
            char *wp = (char *)malloc(len);
            char *sp = wp;
            memset(wp, 0, len);
            snprintf(wp, len, "%s", src);
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
                    memset(rebuf, 0, new_len);
                    snprintf(rebuf, new_len, "%s%s%s", wp, val, pecpy);
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
                            memset(cmd_buf, 0, cmd_len);
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

    bool verbose()
    {
        return mVerbose;
    }

    int debug()
    {
        return mDebug;
    }

    int delay()
    {
        return mDelay;
    }

    bool fakemb1()
    {
        return mFakeMB1;
    }

    std::string trnxpp_cfg_path()
    {
        return mAppCfg;
    }

    std::string session_string()
    {
        return mSessionStr;
    }

    std::string logdir()
    {
        return mLogDirStr;
    }

    logu::logger &mlog()
    {
        return mMsgLog;
    }

    int cycles()
    {
        return mCycles;
    }

    app_stats &stats()
    {
        return mStats;
    }

    double stat_period()
    {
        return mStatPeriod;
    }

    int stat_level()
    {
        return mStatLevel;
    }

    void tostream(std::ostream &os, int wkey=15, int wval=18)
    {
        os << std::dec << std::setfill(' ');
        os << std::setw(wkey) << "verbose " << std::setw(wval) << (mVerbose?"Y":"N") << "\n";
        os << std::setw(wkey) << "debug " << std::setw(wval) << mDebug  << "\n";
        os << std::setw(wkey) << "config " << std::setw(wval) << mAppCfg  << "\n";
        os << std::setw(wkey) << "cycles " << std::setw(wval) << mCycles  << "\n";
        os << std::setw(wkey) << "delay " << std::setw(wval) << mDelay << "\n";
        os << std::fixed << std::setprecision(3);
        os << std::setw(wkey) << "stat_period " << std::setw(wval) << mStatPeriod << "\n";
        os << std::dec;
        os << std::setw(wkey) << "stat_level " << std::setw(wval) << mStatLevel << "\n";
        os << std::setw(wkey) << "logdir " << std::setw(wval) << mLogDirStr.c_str() << "\n";
        os << std::setw(wkey) << "session " << std::setw(wval) << mSessionStr << "\n";
        os << std::setw(wkey) << "fakemb1 " << std::setw(wval) << (mFakeMB1?"Y":"N")  << "\n";
        os << std::setw(wkey) << "inputs"<< std::setw(wval) << mInputList.size() << "\n";
        std::list<std::string>::iterator it;
        for(it=mInputList.begin(); it != mInputList.end(); it++)
        {
            os << std::setw(wkey) << " " << std::setw(wval) << static_cast<std::string>(*it) << "\n";
        }
        os << std::setw(wkey) << "contexts" << std::setw(wval) << mCtxList.size() << "\n";
        for(it=mCtxList.begin(); it != mCtxList.end(); it++)
        {
            os << std::setw(wkey) << " " << std::setw(wval) << static_cast<std::string>(*it) << "\n";
        }
        os << std::setw(wkey) << "trn" << std::setw(wval) << mTrnList.size() << "\n";
        for(it=mTrnList.begin(); it != mTrnList.end(); it++)
        {
            os << std::setw(wkey) << " " << std::setw(wval) << static_cast<std::string>(*it) << "\n";
        }

        os << std::setw(wkey) << "sem" << std::setw(wval) << mSemList.size() << "\n";
        for(it=mSemList.begin(); it != mSemList.end(); it++)
        {
            os << std::setw(wkey) << " " << std::setw(wval) << static_cast<std::string>(*it) << "\n";
        }
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

    bool config_set()
    {
        return mConfigSet;
    }

    std::list<std::string> trn_list()
    {
        return mTrnList;
    }

    std::list<std::string> input_list()
    {
        return mInputList;
    }

    std::list<std::string> sem_list()
    {
        return mSemList;
    }

    std::list<std::string> ctx_list()
    {
        return mCtxList;
    }

    bool *ginterrupt()
    {
        return mGInterrupt;
    }

    void set_ginterrupt(bool *g_int = nullptr)
    {
        mGInterrupt = g_int;
    }

protected:

    void add_to_str_list(std::list<std::string> &list, const char *str)
    {
        std::list<std::string>::iterator it;
        bool on_list=false;
        for(it = list.begin(); it != list.end(); it++)
        {
            std::string lstr = static_cast<std::string>(*it);
            if(lstr.compare(str)==0) {
                on_list=true;
                break;
            }
        }
        if(!on_list){
            list.push_back(std::string(str));
        }
    }
private:
    bool mVerbose;
    bool *mGInterrupt;
    int mDebug;
    int mCycles;
    unsigned int mDelay;
    bool mFakeMB1;
    std::string mAppCfg;
    std::string mSessionStr;
    std::string mLogDirStr;
    logu::logger mMsgLog;
    app_stats mStats;
    double mStatPeriod;
    int mStatLevel;
    bool mConfigSet;
    std::list<std::string> mInputList;
    std::list<std::string> mTrnList;
    std::list<std::string> mSemList;
    std::list<std::string> mCtxList;
    std::list<std::string> mPluginList;
};

#endif

