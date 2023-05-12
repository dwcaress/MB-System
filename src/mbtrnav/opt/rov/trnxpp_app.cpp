
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
#include "trnxpp_cfg.hpp"
#include "trnx_utils.hpp"

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
int update_mb1(trn::trnxpp *xpp);
int update_trncli(trn::trnxpp *xpp);

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

// input: OI sled DVL
// publish to: mbtrnpp , TRN server
// expects:
// b[0]   : vehicle DVL
// b[1]   : sled DVL
// a[0]   : vehicle attitude
// a[1]   : sled attitude
// geo[0] : dvlgeo
// geo[1] : oigeo
int cb_proto_oisled2(void *pargs)
{
    int retval=-1;

    TRN_NDPRINT(3, "%s:%d >>> Callback triggered <<<\n", __func__, __LINE__);

    trn::trnxpp::callback_res_t *cb_res = static_cast<trn::trnxpp::callback_res_t *>(pargs);
    trn::trnxpp *xpp = cb_res->xpp;
    trnxpp_cfg *cfg = cb_res->cfg;

    cfg->stats().trn_cb_n++;

    // iterate over contexts
    std::vector<trn::trnxpp_ctx *>::iterator it;
    for(it = xpp->ctx_list_begin(); it != xpp->ctx_list_end(); it++)
    {

        trn::trnxpp_ctx *ctx = (*it);
        // if context defined for this callback
        if(ctx == nullptr || !ctx->has_callback("cb_proto_oisled2"))
        {
            TRN_TRACE();
            // skip invalid context
            continue;
        }

        TRN_NDPRINT(5, "%s:%d processing ctx[%s]\n", __func__, __LINE__, ctx->ctx_key().c_str());

        int err_count = 0;

        std::string *bkey[2] = {ctx->bath_input_chan(0), ctx->bath_input_chan(1)};
        std::string *nkey = ctx->nav_input_chan(0);
        std::string *akey[2] = {ctx->att_input_chan(0), ctx->att_input_chan(1)};
        std::string *vkey = ctx->vel_input_chan(0);

        // vi is optional
        // bi[0] optional
        if(bkey[1] == nullptr || nkey == nullptr || akey[0] == nullptr || akey[1] == nullptr)
        {
            ostringstream ss;
            ss << (bkey[0]==nullptr ? " bkey[0]" : "");
            ss << (bkey[1]==nullptr ? " bkey[1]" : "");
            ss << (akey[0]==nullptr ? " akey[0]" : "");
            ss << (akey[1]==nullptr ? " akey[1]" : "");
            ss << (nkey==nullptr ? " nkey" : "");
            TRN_NDPRINT(5, "%s:%d WARN - NULL input key: %s\n", __func__, __LINE__, ss.str().c_str());
            err_count++;
            continue;
        }

        trn::bath_info *bi[2] = {xpp->get_bath_info(*bkey[0]), xpp->get_bath_info(*bkey[1])};
        trn::nav_info *ni = xpp->get_nav_info(*nkey);
        trn::att_info *ai[2] = {xpp->get_att_info(*akey[0]), xpp->get_att_info(*akey[1])};
        trn::vel_info *vi = (vkey == nullptr ? nullptr : xpp->get_vel_info(*vkey));

        // vi optional
        // bi[0] optional
        if(bi[0] == nullptr || bi[1] == nullptr || ni == nullptr || ai[1] == nullptr || ai[1] == nullptr || vi == nullptr)
        {
            ostringstream ss;
            ss << (bi[0]==nullptr ? " bi[0]" : "");
            ss << (bi[1]==nullptr ? " bi[1]" : "");
            ss << (ai[0]==nullptr ? " ai[0]" : "");
            ss << (ai[1]==nullptr ? " ai[1]" : "");
            ss << (ni==nullptr ? " ni" : "");
            ss << (vi==nullptr ? " vi" : "");
            TRN_NDPRINT(5, "%s:%d WARN - NULL info instance: %s\n", __func__, __LINE__, ss.str().c_str());
            err_count++;
        }

        if(bkey[0] != nullptr && bi[0] != nullptr)
        TRN_NDPRINT(6, "BATHINST.%s : %s\n",bkey[0]->c_str(), bi[0]->bathstr());
        if(bkey[1] != nullptr && bi[1] != nullptr)
        TRN_NDPRINT(6, "BATHINST.%s : %s\n",bkey[1]->c_str(), bi[1]->bathstr());

        // sled DVL beam count
        size_t n_beams = bi[1]->beam_count();

        if(n_beams > 0){

            // use sled bathy, vehicle attitude
            mb1_t *snd = trnx_utils::lcm_to_mb1(bi[1], ni, ai[0]);

            std::list<trn::beam_tup> beams = bi[1]->beams_raw();
            std::list<trn::beam_tup>::iterator it;

            // if streams_ok, bs/bp pointers have been validated
            trn::bath_input *bp[2] = {xpp->get_bath_input(*bkey[0]), xpp->get_bath_input(*bkey[1])};
            int trn_type[2] = {-1, -1};

            if(bp[0] != nullptr){
                trn_type[0] = bp[0]->bath_input_type();
            }
            if(bp[1] != nullptr){
                trn_type[1] = bp[1]->bath_input_type();
            }

            // TODO: include multibeam (MB) and/or DVL (pub only MB to mbtrnpp)
            // bp[0] optional
            if(nullptr != bp[1]) {

                dvlgeo *geo[2] = {nullptr, nullptr};
                beam_geometry *bgeo[2] = {nullptr, nullptr};

                if(nullptr != bp[0]){
                    bgeo[0] = xpp->lookup_geo(*bkey[0], trn_type[0]);
                    geo[0] = static_cast<dvlgeo *>(bgeo[0]);
                }

                bgeo[1] = xpp->lookup_geo(*bkey[1], trn_type[1]);
                geo[1] = static_cast<dvlgeo *>(bgeo[1]);

                // tranform oisled DVL beams
                trnx_utils::transform_oidvl2(bi, ai, geo, snd);
            } else {
                fprintf(stderr,"%s:%d ERR - NULL bath input; skipping transforms\n", __func__, __LINE__);
            }

            mb1_set_checksum(snd);

            // check modulus
            if(ctx->decmod() <= 0 || (ctx->cbcount() % ctx->decmod()) == 0){

                if(cfg->debug() >=4 ){
                    mb1_show(snd, (cfg->debug()>=5 ? true: false), 5);
                }

                // publish MB1 to mbtrnpp
                ctx->pub_mb1(snd, xpp->pub_list(), cfg);


                if(ctx->trncli_count() > 0){

                    // publish poseT/measT to trn-server
                    poseT *pt = trnx_utils::mb1_to_pose(snd, ai[0], (long)ctx->utm_zone());
                    measT *mt = trnx_utils::mb1_to_meas(snd, ai[0], trn_type[1], (long)ctx->utm_zone());

                    if(pt != nullptr && mt != nullptr){

                        double nav_time = ni->time_usec()/1e6;

                        // publish update TRN, publish estimate to TRN, LCM
                        ctx->pub_trn(nav_time, pt, mt, trn_type[1], xpp->pub_list(), cfg);
                    }

                    if(pt != nullptr)
                        delete pt;
                    if(mt != nullptr)
                        delete mt;
                }
            } else {
                TRN_NDPRINT(5, "%s:%d WARN - not ready count/mod[%d/%d]\n", __func__, __LINE__,ctx->cbcount(), ctx->decmod());
            }
            ctx->inc_cbcount();

            // write CSV
            // use sled bathy, vehicle attitude
            if(ctx->write_mb1_csv(snd, bi[1], ai[0], vi) > 0){
                cfg->stats().mb_csv_n++;
            }


            // release sounding memory
            mb1_destroy(&snd);
            retval=0;
        }

        if(bi[0] != nullptr)
            delete bi[0];
        if(bi[1] != nullptr)
            delete bi[1];
        if(ai[0] != nullptr)
            delete ai[0];
        if(ai[1] != nullptr)
            delete ai[1];
        if(ni != nullptr)
            delete ni;
        if(vi != nullptr)
            delete vi;
    }

    return retval;
}

// input: OI sled DVL
// publish to: mbtrnpp, TRN server
// expects:
// b[0]   : vehicle DVL
// b[1]   : sled DVL
// a[0]   : vehicle attitude
// a[1]   : sled attitude
// geo[0] : dvlgeo
// geo[1] : oigeo
int cb_proto_oisled(void *pargs) // dec 2022
{
    int retval=-1;

    TRN_NDPRINT(3, "%s:%d >>> Callback triggered <<<\n", __func__, __LINE__);

    trn::trnxpp::callback_res_t *cb_res = static_cast<trn::trnxpp::callback_res_t *>(pargs);
    trn::trnxpp *xpp = cb_res->xpp;
    trnxpp_cfg *cfg = cb_res->cfg;

    cfg->stats().trn_cb_n++;

    // iterate over contexts
    std::vector<trn::trnxpp_ctx *>::iterator it;
    for(it = xpp->ctx_list_begin(); it != xpp->ctx_list_end(); it++)
    {

        trn::trnxpp_ctx *ctx = (*it);
        // if context defined for this callback
        if(ctx == nullptr || !ctx->has_callback("cb_proto_oisled"))
        {
            TRN_TRACE();
            // skip invalid context
            continue;
        }

        TRN_NDPRINT(5, "%s:%d processing ctx[%s]\n", __func__, __LINE__, ctx->ctx_key().c_str());

        int err_count = 0;

        std::string *bkey[2] = {ctx->bath_input_chan(0), ctx->bath_input_chan(1)};
        std::string *nkey = ctx->nav_input_chan(0);
        std::string *akey[2] = {ctx->att_input_chan(0), ctx->att_input_chan(1)};
        std::string *vkey = ctx->vel_input_chan(0);

        // vi is optional
        if(bkey[0] == nullptr || bkey[1] == nullptr || nkey == nullptr || akey[0] == nullptr || akey[1] == nullptr)
        {
            ostringstream ss;
            ss << (bkey[0]==nullptr ? " bkey[0]" : "");
            ss << (bkey[1]==nullptr ? " bkey[1]" : "");
            ss << (akey[0]==nullptr ? " akey[0]" : "");
            ss << (akey[1]==nullptr ? " akey[1]" : "");
            ss << (nkey==nullptr ? " nkey" : "");
            TRN_NDPRINT(5, "%s:%d WARN - NULL input key: %s\n", __func__, __LINE__, ss.str().c_str());
            err_count++;
            continue;
        }

        trn::bath_info *bi[2] = {xpp->get_bath_info(*bkey[0]), xpp->get_bath_info(*bkey[1])};
        trn::nav_info *ni = xpp->get_nav_info(*nkey);
        trn::att_info *ai[2] = {xpp->get_att_info(*akey[0]), xpp->get_att_info(*akey[1])};
        trn::vel_info *vi = (vkey == nullptr ? nullptr : xpp->get_vel_info(*vkey));

        if(bi[0] == nullptr || bi[1] == nullptr || ni == nullptr || ai[1] == nullptr || ai[1] == nullptr || vi == nullptr)
        {
            ostringstream ss;
            ss << (bi[0]==nullptr ? " bi[0]" : "");
            ss << (bi[1]==nullptr ? " bi[1]" : "");
            ss << (ai[0]==nullptr ? " ai[0]" : "");
            ss << (ai[1]==nullptr ? " ai[1]" : "");
            ss << (ni==nullptr ? " ni" : "");
            ss << (vi==nullptr ? " vi" : "");
            TRN_NDPRINT(5, "%s:%d WARN - NULL info instance\n", __func__, __LINE__);
            err_count++;
        }

        if(bkey[0] != nullptr && bi[0]!=nullptr)
            TRN_NDPRINT(6, "BATHINST.%s : %s\n",bkey[0]->c_str(), bi[0]->bathstr());
        if(bkey[1] != nullptr && bi[1]!=nullptr)
            TRN_NDPRINT(6, "BATHINST.%s : %s\n",bkey[1]->c_str(), bi[1]->bathstr());

        size_t n_beams = bi[0]->beam_count();

        if(n_beams>0){

            // use sled bathy, vehicle attitude
            mb1_t *snd = trnx_utils::lcm_to_mb1(bi[1], ni, ai[0]);

            std::list<trn::beam_tup> beams = bi[1]->beams_raw();
            std::list<trn::beam_tup>::iterator it;

            // if streams_ok, bs/bp pointers have been validated
            trn::bath_input *bp[2] = {xpp->get_bath_input(*bkey[0]), xpp->get_bath_input(*bkey[1])};
            int trn_type[2] = {bp[0]->bath_input_type(), bp[1]->bath_input_type()};

            // TODO: include multibeam (MB) and/or DVL (pub only MB to mbtrnpp)
            if(nullptr != bp[0] && nullptr != bp[1]) {

                dvlgeo *geo[2] = {nullptr, nullptr};
                beam_geometry *bgeo[2] = {nullptr, nullptr};

                bgeo[0] = xpp->lookup_geo(*bkey[0], trn_type[0]);
                geo[0] = static_cast<dvlgeo *>(bgeo[0]);

                bgeo[1] = xpp->lookup_geo(*bkey[1], trn_type[1]);
                geo[1] = static_cast<dvlgeo *>(bgeo[1]);

                // tranform oisled DVL beams
                trnx_utils::transform_oidvl(bi, ai, geo, snd);

            } else {
                fprintf(stderr,"%s:%d ERR - NULL bath input; skipping transforms\n", __func__, __LINE__);
            }

            mb1_set_checksum(snd);

            // check modulus
            if(ctx->decmod() <= 0 || (ctx->cbcount() % ctx->decmod()) == 0){

                if(cfg->debug() >=4 ){
                    mb1_show(snd, (cfg->debug()>=5 ? true: false), 5);
                }

                // publish MB1 to mbtrnpp
                ctx->pub_mb1(snd, xpp->pub_list(), cfg);


                if(ctx->trncli_count() > 0){

                    // publish poseT/measT to trn-server
                    poseT *pt = trnx_utils::mb1_to_pose(snd, ai[0], (long)ctx->utm_zone());
                    measT *mt = trnx_utils::mb1_to_meas(snd, ai[0], trn_type[1], (long)ctx->utm_zone());

                    if(pt != nullptr && mt != nullptr){

                        double nav_time = ni->time_usec()/1e6;

                        // publish update TRN, publish estimate to TRN, LCM
                        ctx->pub_trn(nav_time, pt, mt, trn_type[1], xpp->pub_list(), cfg);
                    }

                    if(pt != nullptr)
                        delete pt;
                    if(mt != nullptr)
                        delete mt;
                }
            } else {
                TRN_NDPRINT(5, "%s:%d WARN - not ready count/mod[%d/%d]\n", __func__, __LINE__,ctx->cbcount(), ctx->decmod());
            }
            ctx->inc_cbcount();

            // write CSV
            // use sled bathy, vehicle attitude
            if(ctx->write_mb1_csv(snd, bi[1], ai[0], vi) > 0){
                cfg->stats().mb_csv_n++;
            }


            // release sounding memory
            mb1_destroy(&snd);
            retval=0;
        }

        if(bi[0] != nullptr)
            delete bi[0];
        if(bi[1] != nullptr)
            delete bi[1];
        if(ai[0] != nullptr)
            delete ai[0];
        if(ai[1] != nullptr)
            delete ai[1];
        if(ni != nullptr)
            delete ni;
        if(vi != nullptr)
            delete vi;
    }

    return retval;
}


// input: DeltaT or DVL
// publish to: mbtrnpp, TRN server
// expects:
// bi     : bathymetry, DVL or deltaT (on vehicle frame)
// ni     : navigation (on vehicle frame)
// ai     : attitude (on vehicle frame)
// vi     : velocity (optional, may be NULL)
int cb_proto_deltat(void *pargs)
{
    int retval=-1;

    int FN_DEBUG_HI = 6;
    int FN_DEBUG = 5;
//    int FN_INFO_HI = 4;
    int FN_INFO = 3;
//    int FN_INFO_LO = 2;

    TRN_NDPRINT(FN_INFO, "%s:%d >>> Callback triggered <<<\n", __func__, __LINE__);

    trn::trnxpp::callback_res_t *cb_res = static_cast<trn::trnxpp::callback_res_t *>(pargs);
    trn::trnxpp *xpp = cb_res->xpp;
    trnxpp_cfg *cfg = cb_res->cfg;

    cfg->stats().trn_cb_n++;

    // iterate over contexts
    std::vector<trn::trnxpp_ctx *>::iterator it;
    for(it = xpp->ctx_list_begin(); it != xpp->ctx_list_end(); it++)
    {
        trn::trnxpp_ctx *ctx = (*it);
        // if context defined for this callback
        if(ctx == nullptr || !ctx->has_callback("cb_proto_deltat"))
        {
            // skip invalid context
            continue;
        }

        TRN_NDPRINT(FN_DEBUG, "%s:%d processing ctx[%s]\n", __func__, __LINE__, ctx->ctx_key().c_str());

        int err_count = 0;

        std::string *bkey = ctx->bath_input_chan(0);
        std::string *nkey = ctx->nav_input_chan(0);
        std::string *akey = ctx->att_input_chan(0);
        std::string *vkey = ctx->vel_input_chan(0);

        // vi is optional
        if(bkey == nullptr || nkey == nullptr || akey == nullptr)
        {
            TRN_NDPRINT(FN_DEBUG, "%s:%d WARN - NULL input key\n", __func__, __LINE__);
            err_count++;
            continue;
        }

        trn::bath_info *bi = xpp->get_bath_info(*bkey);
        trn::nav_info *ni = xpp->get_nav_info(*nkey);
        trn::att_info *ai = xpp->get_att_info(*akey);
        trn::vel_info *vi = (vkey == nullptr ? nullptr : xpp->get_vel_info(*vkey));

        if(bi == nullptr || ni == nullptr || ai == nullptr || vi == nullptr)
        {
            TRN_NDPRINT(FN_DEBUG, "%s:%d WARN - NULL info instance\n", __func__, __LINE__);
            TRN_NDPRINT(FN_DEBUG, "%s:%d   bi[%p] ni[%p] ai[%p] vi[%p]\n", __func__, __LINE__);
            err_count++;
        }

        std::string *bath_0_key = ctx->bath_input_chan(0);
        TRN_NDPRINT(FN_DEBUG_HI, "BATHINST.%s : %s\n",bath_0_key->c_str(), bi->bathstr());

        size_t n_beams = bi->beam_count();

        if(n_beams>0){

            // generate MB1 sounding (raw beams)
            mb1_t *snd = trnx_utils::lcm_to_mb1(bi, ni, ai);

            // if streams_ok, bs/bp pointers have been validated
            trn::bath_input *bp = xpp->get_bath_input(*bkey);

            if(nullptr != bp) {

                if(bp->bath_input_type() == trn::BT_DVL)
                {
                    beam_geometry *bgeo = xpp->lookup_geo(*bath_0_key, trn::BT_DVL);
                    dvlgeo *geo = static_cast<dvlgeo *>(bgeo);

                    // compute MB1 beam components in vehicle frame
                    trnx_utils::transform_dvl(bi, ai, geo, snd);

                } else if(bp->bath_input_type() == trn::BT_DELTAT)
                {
                    beam_geometry *bgeo = xpp->lookup_geo(*bath_0_key, trn::BT_DELTAT);
                    mbgeo *geo = static_cast<mbgeo *>(bgeo);

                    // compute MB1 beam components in vehicle frame
                    trnx_utils::transform_deltat(bi, ai, geo, snd);

                } else {
                    fprintf(stderr,"%s:%d ERR - unsupported input_type[%d] beam transformation invalid\n", __func__, __LINE__, bp->bath_input_type());
                }
            } else {
                fprintf(stderr,"%s:%d ERR - NULL bath input; skipping transforms\n", __func__, __LINE__);
            }

            mb1_set_checksum(snd);

            // check modulus
            if(ctx->decmod() <= 0 || (ctx->cbcount() % ctx->decmod()) == 0){

                if(cfg->debug() >= FN_DEBUG ){
                    fprintf(stderr,"%s - >>>>>>> Publishing MB1:\n",__func__);
                    mb1_show(snd, (cfg->debug()>=5 ? true: false), 5);
                }

                // publish MB1 to mbtrnpp
                ctx->pub_mb1(snd, xpp->pub_list(), cfg);

                if(ctx->trncli_count() > 0){

                    // publish poseT/measT to trn-server
                    int trn_type = bp->bath_input_type();

                    poseT *pt = trnx_utils::mb1_to_pose(snd, ai, (long)ctx->utm_zone());
                    measT *mt = trnx_utils::mb1_to_meas(snd, ai, trn_type, (long)ctx->utm_zone());

                    if(cfg->debug() >= FN_DEBUG ){
                        fprintf(stderr,"%s - >>>>>>> Publishing POSE:\n",__func__);
                        trnx_utils::pose_show(*pt);
                        fprintf(stderr,"%s - >>>>>>> Publishing MEAS:\n",__func__);
                        trnx_utils::meas_show(*mt);
                    }


                    if(pt != nullptr && mt != nullptr){

                        double nav_time = ni->time_usec()/1e6;

                        // publish update TRN, publish estimate to TRN, LCM
                        ctx->pub_trn(nav_time, pt, mt, trn_type, xpp->pub_list(), cfg);
                    }

                    if(pt != nullptr)
                        delete pt;
                    if(mt != nullptr)
                        delete mt;
                }

            } else {
                TRN_NDPRINT(FN_DEBUG, "%s:%d WARN - not ready count/mod[%d/%d]\n", __func__, __LINE__,ctx->cbcount(), ctx->decmod());
            }
            ctx->inc_cbcount();


            // write CSV
            if(ctx->write_mb1_csv(snd, bi, ai, vi) > 0){
                cfg->stats().mb_csv_n++;
            }

            ctx->write_mb1_bin(snd);

            retval=0;

            // release sounding memory
            mb1_destroy(&snd);
        }

        if(bi != nullptr)
            delete bi;
        if(ai != nullptr)
            delete ai;
        if(ni != nullptr)
            delete ni;
        if(vi != nullptr)
            delete vi;
    }

    return retval;
}

// input: DVL
// publish to: TRN server
// expects:
// bi     : bathymetry, DVL or deltaT (on vehicle frame)
// ni     : navigation (on vehicle frame)
// ai     : attitude (on vehicle frame)
// vi     : velocity (optional, may be NULL)
int cb_proto_dvl(void *pargs)
{
    int retval=-1;
    static uint32_t ping_number = 0;
    int FN_DEBUG = 5;

    TRN_NDPRINT(3, "%s:%d >>> Callback triggered <<<\n", __func__, __LINE__);

    trn::trnxpp::callback_res_t *cb_res = static_cast<trn::trnxpp::callback_res_t *>(pargs);
    trn::trnxpp *xpp = cb_res->xpp;
    trnxpp_cfg *cfg = cb_res->cfg;

    cfg->stats().trn_cb_n++;

    // iterate over contexts
    std::vector<trn::trnxpp_ctx *>::iterator it;
    for(it = xpp->ctx_list_begin(); it != xpp->ctx_list_end(); it++)
    {
        trn::trnxpp_ctx *ctx = (*it);
        // if context defined for this callback
        if(ctx == nullptr || !ctx->has_callback("cb_proto_dvl"))
        {
            // skip invalid context
            continue;
        }

        TRN_NDPRINT(FN_DEBUG, "%s:%d processing ctx[%s]\n", __func__, __LINE__, ctx->ctx_key().c_str());

        int err_count = 0;

        std::string *bkey[1] = {ctx->bath_input_chan(0)};
        std::string *nkey = ctx->nav_input_chan(0);
        std::string *akey[1] = {ctx->att_input_chan(0)};
        std::string *vkey = ctx->vel_input_chan(0);

        if(bkey[0] == nullptr || nkey == nullptr || akey[0] == nullptr || vkey == nullptr)
        {
            TRN_NDPRINT(FN_DEBUG, "%s:%d WARN - NULL input key\n", __func__, __LINE__);
            err_count++;
            continue;
        }

        trn::bath_info *bi = xpp->get_bath_info(*bkey[0]);
        trn::nav_info *ni = xpp->get_nav_info(*nkey);
        trn::att_info *ai = xpp->get_att_info(*akey[0]);
        trn::vel_info *vi = xpp->get_vel_info(*vkey);

        if(bi == nullptr || ni == nullptr || ai == nullptr || vi == nullptr)
        {
            fprintf(stderr,"%s:%d WARN - NULL info instance\n", __func__, __LINE__);
            err_count++;
        }


        // if no errors, bs/bp pointers have been validated

        double nav_time = ni->time_usec()/1e6;

        mb1_t *snd = trnx_utils::lcm_to_mb1(bi, ni, ai);

        beam_geometry *bgeo = xpp->lookup_geo(*bkey[0], trn::BT_DVL);

        dvlgeo *geo = static_cast<dvlgeo *>(bgeo);
        trn::bath_input *bp[1] = {xpp->get_bath_input(*bkey[0])};
        int trn_type = bp[0]->bath_input_type();

        // compute beam components in vehicle frame
        trnx_utils::transform_dvl(bi, ai, geo, snd);

        // check modulus
        if(ctx->decmod() <= 0 || (ctx->cbcount() % ctx->decmod()) == 0){


            // construct poseT/measT TRN inputs

            poseT *pt = trnx_utils::mb1_to_pose(snd, ai, (long)ctx->utm_zone());
            measT *mt = trnx_utils::mb1_to_meas(snd, ai, trn_type, (long)ctx->utm_zone());

            // publish update TRN, publish estimate to TRN, LCM
            ctx->pub_trn(nav_time, pt, mt, trn_type, xpp->pub_list(), cfg);

            if(pt != nullptr)
                delete pt;
            if(mt != nullptr)
                delete mt;

        } else {
            TRN_NDPRINT(FN_DEBUG, "%s:%d WARN - not ready count/mod[%d/%d]\n", __func__, __LINE__,ctx->cbcount(), ctx->decmod());
        }
        ctx->inc_cbcount();


        // write to CSV
        if(ctx->write_mb1_csv(snd, bi, ai, vi) > 0){
            cfg->stats().trn_csv_n++;
        }
        if(snd != nullptr)
            mb1_destroy(&snd);

        if(bi != nullptr)
            delete bi;
        if(ai != nullptr)
            delete ai;
        if(ni != nullptr)
            delete ni;
        if(vi != nullptr)
            delete vi;
    }

    ping_number++;

    return retval;
}

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

    xpp.register_callback("cb_proto_dvl", cb_proto_dvl);
    xpp.register_callback("cb_proto_deltat", cb_proto_deltat);
    xpp.register_callback("cb_proto_oisled", cb_proto_oisled);
    xpp.register_callback("cb_proto_oisled2", cb_proto_oisled2);

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
