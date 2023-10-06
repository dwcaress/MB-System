
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
#include <tuple>
#include <libgen.h>

// initalizers for header-only modules
// must be defined before any framework headers are included
#define INIT_PCF_LOG
#define INIT_TRN_LCM_INPUT
#include "lcm_interface.hpp"
#include "lcm_pcf/signal_t.hpp"
#include "trn_lcm_input.hpp"
#include "raw_signal_input.hpp"
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
#include "trnxpp_cfg.hpp"
#include "trnx_utils.hpp"
#include "trnx_plugin.hpp"

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

// /////////////////
// Module variables
static int g_signal=0;
static bool g_interrupt=false;

// /////////////////
// Declarations

static void s_termination_handler (int signum);
static void s_init_logging(trnxpp_cfg &cfg, int argc, char **argv);
static void s_update_cycle_stats(trnxpp_cfg &cfg);
static void s_copy_config(trnxpp_cfg &cfg, trn::trnxpp &xpp);
#ifdef WITH_TEST_STREAMS
static mb1_t *s_get_test_sounding(mb1_t *dest, int beams);
#endif
//int update_mb1(trn::trnxpp *xpp);
//int update_trncli(trn::trnxpp *xpp);

#ifdef WITH_TEST_STREAMS
void handle_test_streams(pcf::lcm_publisher &signalPub, pcf::lcm_publisher &stringPub, trn::trnxpp &xpp, trn::mb1_server *mb1svr, trnxpp_cfg &cfg);
#endif
void app_main(trnxpp_cfg &cfg);

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
            // CAUTION: stdio from signal handler may segfault on some platforms
            fprintf(stderr,"INFO - sig received[%d]\n",signum);
            g_interrupt=true;
            g_signal=signum;
            break;
        default:
            fprintf(stderr,"ERR - s_termination_handler: sig not handled[%d]\n",signum);
            break;
    }
}

static void s_init_logging(trnxpp_cfg &cfg, int argc, char **argv)
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

static void s_update_cycle_stats(trnxpp_cfg &cfg)
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

static void s_copy_config(trnxpp_cfg &cfg, trn::trnxpp &xpp)
{
    // copy terrainAid.cfg
    // make list of trn config files
    // tuple contains ctx_key, terrain_nav_cfg_path
    std::list<trn::trn_cfg_map> cfgList;

    std::list<trn::trn_host>::iterator it;

    for(it = xpp.trn_host_list_begin(); it != xpp.trn_host_list_end(); it++){

        std::string trn_key = std::get<0>(*it);
        std::string tnav_cfg = std::get<6>(*it);

        if(tnav_cfg.length() > 0){
            cfgList.emplace_back(trn_key, tnav_cfg);
        }
    }

    // copy them to new name differentiated using context key
    ostringstream ss;
    std::list<trn::trn_cfg_map>::iterator cit;
    for(cit = cfgList.begin(); cit != cfgList.end(); cit++){
        ostringstream ss;
        if(std::get<1>(*cit).size() > 0){
            ss << "cp " << std::get<1>(*cit).c_str() << " " << cfg.logdir().c_str();
            ss << "/terrainAid-";
            ss << std::get<0>(*cit).c_str() << "-";
            ss << cfg.session_string().c_str();
            ss << ".cfg";
            if(system(ss.str().c_str()) != 0)
            {
                fprintf(stderr,"%s:%d - ERR config copy failed [%s] [%d/%s]\n",__func__, __LINE__,
                        ss.str().c_str(), errno, strerror(errno));
            }
        }
    }

    ss.str(std::string());
    ss << "cp " << cfg.trnxpp_cfg_path() << " " << cfg.logdir().c_str();
    ss << "/trnxpp-";
    ss << cfg.session_string().c_str();
    ss << ".cfg";
    if(system(ss.str().c_str()) != 0 )
    {
        fprintf(stderr,"%s:%d - ERR config copy failed [%s] [%d/%s]\n",__func__, __LINE__,
                ss.str().c_str(), errno, strerror(errno));
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

#ifdef WITH_TEST_STREAMS
void handle_test_streams(pcf::lcm_publisher &signalPub, pcf::lcm_publisher &stringPub, trn::trnxpp &xpp, trn::mb1_server *mb1svr, trnxpp_cfg &cfg)
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

void app_main(trnxpp_cfg &cfg)
{

    LU_PEVENT(cfg.mlog(), "session start [%s]",cfg.session_string().c_str());

    // get an LCM instance
    pcf::lcm_interface lcm("");

    LU_PEVENT(cfg.mlog(), "lcm initialized");

    // get a trnxpp instance
    trn::trnxpp xpp(lcm);

    xpp.set_callback_res(&cfg);

    TrnxPlugin::register_callbacks(xpp);

    xpp.parse_config(&cfg);
    s_copy_config(cfg, xpp);

    // connect all TRN IO
    xpp.start_trn(&cfg, &g_interrupt);
    xpp.start_lcm_pubs();

    xpp.show();

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
        s_update_cycle_stats(cfg);

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

    TNavConfig::release();
    trn_debug::get(true);

    LU_PEVENT(cfg.mlog(), "session ended");

    return;
}

int main(int argc, char **argv)
{
    struct sigaction saStruct;
    sigemptyset(&saStruct.sa_mask);
    saStruct.sa_flags = 0;
    saStruct.sa_handler = s_termination_handler;
    sigaction(SIGINT, &saStruct, NULL);

    trnxpp_cfg cfg;

    cfg.set_ginterrupt(&g_interrupt);

    cfg.stats().start_time = logu::utils::dtime();

    setenv("XPP_SESSION",cfg.session_string().c_str(), false);

    // parse command line (first pass for config file)
    cfg.parse_args(argc, argv);

    // configure debug output (for parsing debug)
    trn_debug::get()->set_debug(cfg.debug());
    trn_debug::get()->set_verbose(cfg.verbose());

    if(cfg.config_set() > 0){
        // parse config file
        cfg.parse_file(cfg.trnxpp_cfg_path());
    }

    // reparse command line (should override config options)
    cfg.parse_args(argc, argv);

    // Start logger
    s_init_logging(cfg, argc, argv);
    LU_PEVENT(cfg.mlog(), "trnxpp_cfg:\n%s\n",cfg.tostring().c_str());

    // pass debug/verbose output config to trn_debug
    trn_debug::get()->set_verbose(cfg.verbose());
    trn_debug::get()->set_debug(cfg.debug());

    if(cfg.debug()>0){
        cfg.show();
    }

    app_main(cfg);

    return 0;
}
