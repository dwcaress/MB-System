////////////////////////////////////////////////////////////////////////////////
//// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
//// Distributed under MIT license. See license.txt for more information.     //
////////////////////////////////////////////////////////////////////////////////
#ifndef LCM_TRN_PP_H  // include guard
#define LCM_TRN_PP_H

#include <iostream>
#include <tuple>
#include <list>
#include <string>
#include <map>
//
#ifdef XPP_PROTO_SEM_CHECK
// for threaded sem checking
#include <thread>
#include <future>
#endif

#include "trn_debug.hpp"
#include "trnxpp_cfg.hpp"
#include "LcmTrnCtx.h"

#include "depth_input.hpp"
#include "x_mb1_input.hpp"
#include "dvl_stat_input.hpp"
#include "rdi_dvl_input.hpp"
#include "nav_solution_input.hpp"
#include "idt_input.hpp"
#include "pcomms_input.hpp"
#include "rdi_pd4_input.hpp"
#include "kearfott_input.hpp"
#include "octans_input.hpp"
#include "parosci_stat_input.hpp"
#include "ctd_fastcat_input.hpp"
#include "trn_mb1_input.hpp"
#include "norbit_input.hpp"
#include "vn100_input.hpp"
#include "vn100s_input.hpp"
#include "rph_input.hpp"
#include "rph_angrate_input.hpp"
#include "senlcm_gps_fix_input.hpp"

#define TRNHOSTLIST_STR_NONE "-"

class LcmTrnPP;

typedef struct callback_res_s
{
    trnxpp_cfg *cfg;
    LcmTrnPP *xpp;
}callback_res_t;

class LcmTrnPP
{
public:


    LcmTrnPP(pcf::lcm_interface &lcm);

    ~LcmTrnPP();

    void tostream(std::ostream &os, int wkey=15, int wval=18);

    std::string tostring(int wkey=15, int wval=18);

    void show(int wkey=15, int wval=18);

    int start();

    int stop();

    std::vector<LcmTrnCtx *>::iterator ctx_list_begin();

    std::vector<LcmTrnCtx *>::iterator ctx_list_end();

    int add_pub(const std::string &channel);

    pcf::lcm_publisher *get_pub(const std::string& channel);

    std::list<lcm_pub> &pub_list();

    int start_lcm_pubs();

    int add_sem(const std::string& channel, int count=0);

    // return 0 if callback invoked, -1 otherwise
    // if r_stat non-NULL, set to calback status
    // if clear_pending true, set sem count to 0 after callback
    int test_sem(const std::string& channel, int to_msec, msg_callback cb, int &r_stat, void *parg=nullptr, bool clear_pending=false);


#ifdef XPP_PROTO_SEM_CHECK
    // this may be a Bad Idea, since it could process inputs out of sequence?
    void sem_worker_fn(const std::string& channel, int to_msec, msg_callback cb, void *parg, bool clear_pending, std::promise<int> rv_promise, std::promise<int> st_promise);

    int list_test_sem(bool clear_pending, int &r_tested, int &r_called, int &r_error);
#endif

    int list_test_sem(bool clear_pending, int &r_tested, int &r_called, int &r_error);

    sem_reg *lookup_sem(const std::string &channel, const std::string &cb_key);

    int list_add_sem(const std::string& channel, int to_msec, msg_callback cb, void *parg=nullptr, int count=0);

    msg_callback lookup_callback(const std::string &key);

    void register_callback(const char *key, msg_callback cb);

    void set_callback_res(trnxpp_cfg *cfg);

    callback_res_t *callback_res();

    beam_geometry *lookup_geo(const std::string chan, int type);

    int add_input(const std::string& name = "UNKNOWN", uint32_t depth=0);

    int add_input(const std::string& name = "UNKNOWN", trn::trn_lcm_input *sub=nullptr);

    trn::trn_lcm_input *get_input(const std::string& channel);

    // Factory method creates appropriate inputs
    // for specified channel name.
    // bathymetry providers (inputs) must set the TRN input type
    // defined in structDefs.h (bathymetry_provider_IF.hpp):
    //  BT_DVL
    //  BT_MULTIBEAM
    //  BT_PENCIL
    //  BT_HOMER
    //  BT_DELTAT
    // the input type is used in TrnClient::measUpdate()
    trn::trn_lcm_input *create_input(const std::string &channel, int buf_depth);

    trn::bath_input *get_bath_input(const std::string chan);

    trn::mb1_info *get_mb1_info(const std::string chan);

    trn::mb1_input *get_mb1_input(const std::string chan);

    trn::depth_info *get_depth_info(const std::string chan);

    trn::depth_input *get_depth_input(const std::string chan);

    trn::bath_info *get_bath_info(const std::string chan);

    trn::nav_input *get_nav_input(const std::string chan);

    trn::nav_info *get_nav_info(const std::string chan);

    trn::att_input *get_att_input(const std::string chan);

    trn::att_info *get_att_info(const std::string chan);

    trn::vel_input *get_vel_input(const std::string chan);

    trn::vel_info *get_vel_info(const std::string chan);

    trn_host *lookup_trn_host(const std::string &key);

    int start_trn(trnxpp_cfg *cfg, bool *user_int);

    std::list<trn_host>::iterator trn_host_list_begin();

    std::list<trn_host>::iterator trn_host_list_end();

    int parse_trn(const char *str);


    int parse_input(const char *str);

    int parse_ctx_dmap(const char *map_spec, std::map<std::string, double>& kvmap);

    int parse_ctx_umap(const char *map_spec, std::map<std::string, uint64_t>& kvmap);

    int parse_ctx_imap(const char *map_spec, std::map<std::string, int64_t>& kvmap);

    // note: r_chan, r_sem return dynamically allocated strings, caller must free
    int parse_ctx_input(char *opt_s, const char *key, int *r_idx, char **r_chan, char **r_cb=NULL, int *r_tmout=NULL);

    void add_ctx_sem(char *chan, char *cb_key, int tmout);

    int parse_ctx(const char *str);

    int parse_sem(const char *str);

    void parse_config(trnxpp_cfg *cfg);

protected:

private:
    // LCM instance
    pcf::lcm_interface *mLcm;
    // input stream list
    std::list<lcm_input> mInputList;
    std::list<sem_reg> mSemList;
    std::list<lcm_pub> mPubList;
    std::list<trn_host> mTrnHostList;
    std::list<beam_geo> mGeoList;
    std::vector<LcmTrnCtx *> mCtx;
    std::list<callback_kv> mCallbackList;
    callback_res_t mCallbackRes;

};

#endif
