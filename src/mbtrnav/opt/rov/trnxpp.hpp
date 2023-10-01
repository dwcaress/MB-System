////////////////////////////////////////////////////////////////////////////////
//// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
//// Distributed under MIT license. See license.txt for more information.     //
////////////////////////////////////////////////////////////////////////////////
#ifndef TRNXPP_HPP  // include guard
#define TRNXPP_HPP

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <tuple>
#include <list>
#include <string>
#ifdef XPP_PROTO_SEM_CHECK
// for threaded sem checking
#include <thread>
#include <future>
#endif

#include "pcf_utils.hpp"
#include "lcm_subscriber.hpp"
#include "lcm_pcf/string_t.hpp"
#include "trn_lcm_input.hpp"
#include "data_container.hpp"
#include "mb1_server.hpp"
#include "TrnClient.h"
#include "structDefs.h"
#include "udpm_sub.h"
#include "trn_debug.hpp"
#include "log_utils.hpp"
#include "NavUtils.h"
#include "trnxpp_cfg.hpp"
#include "trnxpp_ctx.hpp"
#include "trn/trn_pose_t.hpp"
#include "trn_msg_utils.hpp"

namespace trn
{

class trnxpp
{
public:

    typedef struct callback_res_s
    {
        trnxpp_cfg *cfg;
        trn::trnxpp *xpp;
    }callback_res_t;

    trnxpp(pcf::lcm_interface &lcm)
    :mLcm(nullptr)
    , mInputList()
    , mSemList()
    , mPubList()
    , mGeoList()
    , mCallbackRes()
    {
        mLcm = &lcm;
        mLcm->initialize();
    }

    ~trnxpp()
    {
        while(mInputList.size() > 0){
            std::string name = std::get<0>(mInputList.front());
            trn_lcm_input *sub = std::get<1>(mInputList.front());
            delete sub;
            mInputList.pop_front();
        }
        while(mPubList.size() > 0){
            std::string name = std::get<0>(mPubList.front());
            pcf::lcm_publisher *pub = std::get<1>(mPubList.front());
            delete pub;
            mPubList.pop_front();
        }
        while(!mCtx.empty()){
            delete mCtx.back();
            mCtx.pop_back();
        }

        while(mGeoList.size() > 0){

            beam_geometry *geo = std::get<2>(mGeoList.front());
            int type = std::get<1>(mGeoList.front());

            if(type == BT_DVL) {
                delete static_cast<dvlgeo *>(geo);
            } else if(type == BT_DELTAT) {
                delete static_cast<mbgeo *>(geo);
            } else if(type == BT_MULTIBEAM) {
                delete static_cast<mbgeo *>(geo);
            }
            mGeoList.pop_front();
        }
    }

    void tostream(std::ostream &os, int wkey=15, int wval=18)
    {
        os << std::dec << std::setfill(' ');
        os << "--- trnxpp ---" << "\n";
        os << std::setw(wkey) << "addr" << std::setw(wval) << this <<"\n";
        os << std::setw(wkey) << "lcm" << std::setw(wval) << &mLcm <<"\n";
        os << std::setw(wkey) << "inputs"  << std::setw(wval) << mInputList.size() << "\n";
        if(mInputList.size()>0){
            std::list<lcm_input>::iterator it;
            int i=0;
            for(it = mInputList.begin(); it != mInputList.end() ; it++, i++) {
                trn_lcm_input *x = std::get<1>(*it);
                os << std::dec << std::setfill(' ');
                os << std::setw(wkey-3) << "input["<< std::setw(2) << i <<"]\n";
                x->tostream(os, wkey, wval);
                os << "\n";
            }
        }

        os << std::setw(wkey) << "geo"  << std::setw(wval) << mGeoList.size() << "\n";
        std::list<beam_geo>::iterator it;
        if(mGeoList.size() > 0){
            int i = 0;
            for(it = mGeoList.begin(); it != mGeoList.end(); it++){

                const std::string chan_x = std::get<0>(*it);
                int type_x = std::get<1>(*it);

                os << std::setw(wkey-3) << "geo["<< std::setw(2) << i++ <<"]";

                ostringstream ss;
                ss << chan_x << "," << type_x;
                int alen = ss.str().length();
                int wx = (alen >= wval ? (alen + 1) : wval);
                os << setw(wx) << ss.str().c_str() << std::endl;
            }
            os << "\n";
        }
        os << std::setw(wkey) << std::dec << std::setfill(' ');
        os << std::setw(wkey) << "semaphores" << std::setw(wval) << mSemList.size() << "\n";
        if(mSemList.size()>0){
            std::list<sem_reg>::iterator it;
            int i=0;
            for(it=mSemList.begin(); it!=mSemList.end() ; it++, i++) {

                std::string name = std::get<0>(*it);
                int tmo = std::get<1>(*it);
                msg_callback cb = std::get<2>(*it);
                void *res = std::get<3>(*it);
                int count = std::get<4>(*it);

                os << std::setw(wkey-3) << "sem[" << std::setw(2) << i << "]\n";
                os << std::dec << std::setfill(' ');
                os << std::setw(wkey+1) << "name" << std::setw(wval) << name << "\n";
                os << std::setw(wkey+1) << "to_sec" << std::setw(wval) << tmo << "\n";
                os << std::setw(wkey+1) << "callback" << std::setw(wval) << (void *)cb << "\n";
                os << std::setw(wkey+1) << "res" << std::setw(wval) << res << "\n";
                os << std::dec << std::setfill(' ');
                os << std::setw(wkey+1) << "count" << std::setw(wval) << count << "\n";
            }
            os << "\n";
        }

        os << std::setw(wkey) << "contexts" << std::setw(wval) << mCtx.size() << "\n";
        if(mCtx.size()>0){
            std::vector<trnxpp_ctx *>::iterator it;
            int i=0;
            for(it = mCtx.begin(); it != mCtx.end(); it++){
                trnxpp_ctx *ctx = static_cast< trnxpp_ctx *>(*it);
                os << std::setw(wkey-3) << "mCtx[" << std::setw(2) << i++ << "]" << "\n";
                os << ctx->tostring(wkey, wval);
            }
            os << "\n";
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

    int start()
    {
        mLcm->start();
        return 0;
    }

    int stop()
    {
        mLcm->stop();
        return 0;
    }

    std::vector<trnxpp_ctx *>::iterator ctx_list_begin()
    {
        return mCtx.begin();
    }

    std::vector<trnxpp_ctx *>::iterator ctx_list_end()
    {
        return mCtx.end();
    }

    int add_pub(const std::string &channel)
    {
        pcf::lcm_publisher *pub = new pcf::lcm_publisher(channel);
        mLcm->add_publisher(*pub);
        mPubList.emplace_back(std::string(channel), pub);
        return 0;
    }

    pcf::lcm_publisher *get_pub(const std::string& channel)
    {
        if(!mPubList.empty()){
            std::list<lcm_pub>::iterator it;

            for(it=mPubList.begin(); it!=mPubList.end() ; it++) {
                const std::string key = std::get<0>(*it);
                if(key.compare(channel)==0){
                    pcf::lcm_publisher *x = std::get<1>(*it);
                    return x;
                }
            }
        }
        return nullptr;
    }

    std::list<lcm_pub> &pub_list()
    {
        return mPubList;
    }

    int start_lcm_pubs()
    {
        TRN_NDPRINT(1, "adding LCM pubs\n");
        // add publishers for LCM types produced
        // TRN server inputs
        add_pub("TRN_MOTN");
        add_pub("TRN_MEAS");
        // TRN server estimates (output)
        add_pub("TRN_EST");
        // mbtrnpp MB1 inputs
        add_pub("MB1_PUB");
        // mbtrnpp MB1 estimates (output)
        add_pub("MB1_EST");

        return -1;
    }

    int add_sem(const std::string& channel, int count=0)
    {
        trn_lcm_input *inp = get_input(channel);

        if(inp != nullptr){
            return inp->add_sem(channel, count);
        }
        return -1;
    }

    // return 0 if callback invoked, -1 otherwise
    // if r_stat non-NULL, set to calback status
    // if clear_pending true, set sem count to 0 after callback
    int test_sem(const std::string& channel, int to_msec, msg_callback cb, int &r_stat, void *parg=nullptr, bool clear_pending=false)
    {
        trn_lcm_input *inp = get_input(channel);
        if(inp != nullptr){
            pcf::semaphore *sem = inp->get_sem(channel);

            TRN_NDPRINT(6, "testing sem chan[%s] count[%u]\n", channel.c_str(), sem->get_count());

            if(inp->test_sem(channel, to_msec)){

                TRN_NDPRINT(6, "calling sem chan[%s] count[%u]\n", channel.c_str(), sem->get_count());
                TRN_NDPRINT(6, "testing sem cb[%p]\n", cb);

                int stat = cb(parg);
                sem->clear_count();
                r_stat = stat;
                return 0;
            }
        }
        return -1;
    }


#ifdef XPP_PROTO_SEM_CHECK
    // this may be a Bad Idea, since it could process inputs out of sequence?
    void sem_worker_fn(const std::string& channel, int to_msec, msg_callback cb, void *parg, bool clear_pending, std::promise<int> rv_promise, std::promise<int> st_promise)
    {
        TRN_TRACE();
        int stat = -1;
        TRN_TRACE();
        int test = test_sem(channel, to_msec, cb, stat, parg, clear_pending);
        TRN_TRACE();
        rv_promise.set_value(test);
        TRN_TRACE();
        st_promise.set_value(stat);
        TRN_TRACE();
    }

    int list_test_sem(bool clear_pending, int &r_tested, int &r_called, int &r_error)
    {
        int stat = 0;
        std::list<trn::sem_reg>::iterator it = mSemList.begin();

        std::vector<std::thread *> workers;
        std::vector<std::future<int>> stat_futures;

        std::vector<std::future<int>> rval_futures;
        std::vector<int> stat_vals;

        while(it != mSemList.end()){

            const std::string& channel = std::get<0>(*it);
            int to_msec = std::get<1>(*it);
            trn::msg_callback cb = std::get<2>(*it);
            void *parg = std::get<3>(*it);

            if(cb != nullptr && parg != nullptr){

                TRN_NDPRINT(1, "INFO - testing sem channel[%s]\n", channel.c_str());

                std::promise<int> rv_prom;
                std::promise<int> st_prom;

                rval_futures.push_back(rv_prom.get_future());
                stat_futures.push_back(st_prom.get_future());

                std::thread *worker = new std::thread(&trnxpp::sem_worker_fn, this, channel, to_msec, cb, parg, clear_pending, std::move(rv_prom),  std::move(st_prom));

                workers.push_back(worker);
            } else {
                TRN_NDPRINT(1, "ERR - invalid sem arg cb[%p] parg[%p]\n", cb, parg);
            }
            it++;
            r_tested++;
        }
        TRN_NDPRINT(1, "INFO - workers size[%d]\n", workers.size());

        for(int i=0; i < workers.size(); i++){
            stat_futures.at(i).wait();
            rval_futures.at(i).wait();
            int r = rval_futures.at(i).get();
            int s = stat_futures.at(i).get();
            r_called += (r == 0 ? 1 : 0);
            r_error += ( s != 0 ? 1 : 0);
            workers.at(i)->join();
            TRN_NDPRINT(1, "INFO - joining worker[%d] stat[%d] rval[%d]\n", i, s, r);
        }
        TRN_TRACE();

        workers.clear();
        return stat;
    }
#endif

    int list_test_sem(bool clear_pending, int &r_tested, int &r_called, int &r_error)
    {
        int stat = 0;
        std::list<trn::sem_reg>::iterator it = mSemList.begin();


        while(it != mSemList.end()){

            const std::string& channel = std::get<0>(*it);
            int to_msec = std::get<1>(*it);
            trn::msg_callback cb = std::get<2>(*it);
            void *parg = std::get<3>(*it);
            int r_stat = 0;
            if(cb != nullptr && parg != nullptr){

                int stat = test_sem( channel, to_msec, cb, r_stat, parg, clear_pending);

                r_called += (stat == 0 ? 1 : 0);
                r_error += (r_stat != 0 ? 1 : 0);
            } else {
                TRN_NDPRINT(1, "ERR - invalid sem arg cb[%p] parg[%p]\n", cb, parg);
            }
            it++;
            r_tested++;
        }
        return stat;
    }

    trn::sem_reg *lookup_sem(const std::string &channel, const std::string &cb_key)
    {
        trn::sem_reg *retval = nullptr;

        msg_callback cb_ptr = lookup_callback(cb_key);

        if(cb_ptr != nullptr){
            std::list<trn::sem_reg>::iterator it;
            for(it = mSemList.begin(); it != mSemList.end(); it++){
                // channel, timeout_sec, callback func, pargs, sem_count
                std::string sch = std::get<0>(*it);
                msg_callback vcb = std::get<2>(*it);
                if(cb_ptr == vcb && sch.compare(channel) == 0){
                    retval = &(*it);
                }
            }
        }

        return retval;
    }

    int list_add_sem(const std::string& channel, int to_msec, msg_callback cb, void *parg=nullptr, int count=0)
    {
        trn_lcm_input *inp = get_input(channel);
        if(inp != nullptr){

            mSemList.emplace_back(channel, to_msec, cb, parg, count);
            return inp->add_sem(channel, count);
        }
        return -1;
    }


    msg_callback lookup_callback(const std::string &key)
    {
        msg_callback retval = nullptr;

        std::list<callback_kv>::iterator it;

        for(it = mCallbackList.begin(); it != mCallbackList.end(); it++) {
            std::string lkey = std::get<0>(*it);
            if(lkey.compare(key) == 0){
                // found key
                retval = std::get<1>(*it);
                break;
            }
        }
        return retval;
    }

    void register_callback(const char *key, msg_callback cb)
    {
        std::list<callback_kv>::iterator it;
        bool is_registered = false;
        for(it = mCallbackList.begin(); it != mCallbackList.end(); it++) {
            std::string lkey = std::get<0>(*it);
            if(lkey.compare(key) == 0){
                // key exists, reset it
                std::get<1>(*it) = cb;
                is_registered = true;
                break;
            }
        }
        if(!is_registered){
            // new entry
            mCallbackList.emplace_back(std::string(key), cb);
        }
    }

    void set_callback_res(trnxpp_cfg *cfg)
    {
        mCallbackRes.cfg = cfg;
        mCallbackRes.xpp = this;
    }

    callback_res_t *callback_res()
    {
        return &mCallbackRes;
    }

    beam_geometry *lookup_geo(const std::string chan, int type)
    {
        beam_geometry *retval = nullptr;

        std::list<beam_geo>::iterator it;
        for(it = mGeoList.begin(); it != mGeoList.end(); it++){

            const std::string chan_x = std::get<0>(*it);
            int type_x = std::get<1>(*it);

            if(type_x == type){
                if(chan_x.compare(chan) == 0){
                    retval = std::get<2>(*it);
                    break;
                }
            }
        }
        return retval;
    }

    int add_input(const std::string& name = "UNKNOWN", uint32_t depth=0)
    {
        trn_lcm_input *sub = new trn_lcm_input(name, depth);
        mLcm->add_subscriber(*sub);
        lcm_input item = std::make_tuple(std::string(name), sub);
        mInputList.push_back(item);
        return 0;
    }

    int add_input(const std::string& name = "UNKNOWN", trn_lcm_input *sub=nullptr)
    {
        if(sub != nullptr){
            mLcm->add_subscriber(*sub);
            lcm_input item = std::make_tuple(std::string(name), sub);
            mInputList.push_back(item);
            return 0;
        }
        return -1;
    }

    trn_lcm_input *get_input(const std::string& channel)
    {
        if(!mInputList.empty()){
            std::list<lcm_input>::iterator it;

            for(it=mInputList.begin(); it!=mInputList.end() ; it++) {
                const std::string key = std::get<0>(*it);
                if(key.compare(channel)==0){
                    trn_lcm_input *x = std::get<1>(*it);
                    return x;
                }
            }
        }
        return nullptr;
    }

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
    trn_lcm_input *create_input(const std::string &channel, int buf_depth)
    {
        if(channel.compare("RAW_SIGNAL")==0)
        {
            return new trn::raw_signal_input("RAW_SIGNAL", buf_depth);
        }
        else if(channel.compare("STRING_MSG")==0)
        {
            return new trn::raw_signal_input("STRING_MSG", buf_depth);
        }
        else if(channel.compare("OPENINS_DVL_STAT")==0)
        {
            trn_lcm_input *obj = new trn::dvl_stat_input("OPENINS_DVL_STAT", buf_depth);
            dynamic_cast<trn::bath_input *>(obj)->set_bath_input_type(BT_DVL);
            return obj;
        }
        else if(channel.compare("DVL_KEARFOTT_OI")==0)
        {
            trn_lcm_input *obj = new trn::rdi_pd4_input("DVL_KEARFOTT_OI", buf_depth);
            dynamic_cast<trn::bath_input *>(obj)->set_bath_input_type(BT_DVL);
            return obj;
        }
        else if(channel.compare("IDT_PUB")==0)
        {
            trn_lcm_input *obj = new trn::idt_input("IDT_PUB", 10);
            dynamic_cast<trn::bath_input *>(obj)->set_bath_input_type(BT_DELTAT);
            return obj;
        }
        else if(channel.compare("GSS_NAV_SOLUTION")==0)
        {
            return new trn::nav_solution_input("GSS_NAV_SOLUTION", buf_depth);
        }
        else if(channel.compare("OPENINS_NAV_SOLUTION")==0)
        {
            return new trn::nav_solution_input("OPENINS_NAV_SOLUTION", buf_depth);
        }
        else if(channel.compare("SONARDYNE_SPRINT_STAT")==0)
        {
            return new trn::pcomms_input("SONARDYNE_SPRINT_STAT", buf_depth);
        }
        else if(channel.compare("INS_KEARFOTT_OI")==0)
        {
            return new trn::kearfott_input("INS_KEARFOTT_OI", buf_depth);
        }
        else if(channel.compare("IMU_OCTANS")==0)
        {
            return new trn::octans_input("IMU_OCTANS", buf_depth);
        }
        std::cerr << __func__ << ": ERR - Unsupported type [" << channel << "]\n";
        return nullptr;
    }

    trn::bath_input *get_bath_input(const std::string chan)
    {
        trn::bath_input *retval = nullptr;

        trn::trn_lcm_input *li = this->get_input(chan);
        if(li != nullptr){
            trn::bath_input *ip = dynamic_cast<trn::bath_input *>(li);
            retval = ip;
        }
        return retval;
    }

    trn::bath_info *get_bath_info(const std::string chan)
    {
        trn::bath_info *retval = nullptr;
        trn::bath_input *ip = get_bath_input(chan);

        if(ip != nullptr){
            retval = ip->bath_inst();
        }

        return retval;
    }

    trn::nav_input *get_nav_input(const std::string chan)
    {
        trn::nav_input *retval = nullptr;

        trn::trn_lcm_input *li = this->get_input(chan);
        if(li != nullptr){
            trn::nav_input *ip = dynamic_cast<trn::nav_input *>(li);
            retval = ip;
        }
        return retval;
    }

    trn::nav_info *get_nav_info(const std::string chan)
    {
        trn::nav_info *retval = nullptr;
        trn::nav_input *ip = get_nav_input(chan);

        if(ip != nullptr){
            retval = ip->nav_inst();
        }

        return retval;
    }

    trn::att_input *get_att_input(const std::string chan)
    {
        trn::att_input *retval = nullptr;

        trn::trn_lcm_input *li = this->get_input(chan);
        if(li != nullptr){
            trn::att_input *ip = dynamic_cast<trn::att_input *>(li);
            retval = ip;
        }
        return retval;
    }

    trn::att_info *get_att_info(const std::string chan)
    {
        trn::att_info *retval = nullptr;
        trn::att_input *ip = get_att_input(chan);

        if(ip != nullptr){
            retval = ip->att_inst();
        }

        return retval;
    }

    trn::vel_input *get_vel_input(const std::string chan)
    {
        trn::vel_input *retval = nullptr;

        trn::trn_lcm_input *li = this->get_input(chan);
        if(li != nullptr){
            trn::vel_input *ip = dynamic_cast<trn::vel_input *>(li);
            retval = ip;
        }
        return retval;
    }

    trn::vel_info *get_vel_info(const std::string chan)
    {
        trn::vel_info *retval = nullptr;
        trn::vel_input *ip = get_vel_input(chan);

        if(ip != nullptr){
            retval = ip->vel_inst();
        }

        return retval;
    }

    trn_host *lookup_trn_host(const std::string &key)
    {
        std::list<trn_host>::iterator it;
        for(it = mTrnHostList.begin(); it != mTrnHostList.end(); it++)
        {
            std::string hkey = std::get<0>(*it);

            if(hkey.compare(key) == 0)
            {
                return &(*it);
            }
        }
        return nullptr;
    }

    int start_trn(trnxpp_cfg *cfg, bool *user_int)
    {
        int retval = -1;
        int errors = 0;

        for(int i=0; i<mCtx.size(); i++) {

            trnxpp_ctx *ctx = mCtx.at(i);
            if(nullptr != ctx) {
                if(ctx->start_trn(cfg, user_int) != 0){
                    errors++;
                }
                if(ctx->init_mb1_csv_file(cfg) != 0){
                    errors++;
                }
                if(ctx->init_mb1_bin_file(cfg) != 0){
                    errors++;
                }
                if(ctx->init_trnest_csv_file(cfg) != 0){
                    errors++;
                }
                if(ctx->init_mbest_csv_file(cfg) != 0){
                    errors++;
                }
            }
        }
        retval = errors;

        return retval;
    }

    std::list<trn_host>::iterator trn_host_list_begin()
    {
        return mTrnHostList.begin();
    }

    std::list<trn_host>::iterator trn_host_list_end()
    {
        return mTrnHostList.end();
    }

    int parse_trn(const char *str)
    {
        int retval = -1;

        if(NULL != str){
            char *cpy = strdup(str);
            char *key_s = strtok(cpy, ",");
            char *type_s = strtok(NULL, ",");
            char *host_s = strtok(NULL, ",");
            char *cfg_s = strtok(NULL, ",");

            TRN_NDPRINT(2, "%s:%d - key[%s] type[%s] host[%s] cfg[%s]\n", __func__, __LINE__, key_s, type_s, host_s, cfg_s);

            bool parse_err = false;
            bool trn_nohost = false;
            char *key = nullptr;
            char *type = nullptr;
            char *host = nullptr;
            char *cfg = nullptr;
            int port = -1;
            int ttl = 0;

            if(NULL != type_s) {
                type = trnxpp_cfg::trim(type_s);
                if(NULL == type){
                    TRN_TRACE();
                    parse_err = true;
                } else if(strcmp(type, "mb1pub") == 0) {

                } else if(strcmp(type, "udpms") == 0) {

                } else if(strcmp(type, "trncli") == 0) {
                    if(cfg_s != nullptr){
                        cfg = trnxpp_cfg::trim(cfg_s);
                    }else{
                        // config required for trncli
                        parse_err = true;
                    }

                } else if(strcmp(type, "trn") == 0) {
                    trn_nohost = true;
                } else {
                    TRN_TRACE();
                    parse_err = true;
                }
            } else {
                TRN_TRACE();
                parse_err = true;
            }

            if(!parse_err && NULL != key_s) {
                key = trnxpp_cfg::trim(key_s);
            } else {
                TRN_TRACE();
                parse_err = true;
            }

            if(!trn_nohost){
                if(!parse_err && NULL != host_s) {
                    char *ts = trnxpp_cfg::trim(host_s);
                    if(NULL != ts){
                        char *tcpy = strdup(ts);
                        char *addr_s = strtok(tcpy, ":");
                        char *port_s = strtok(NULL, ":");
                        char *ttl_s = strtok(NULL, ":");

                        TRN_NDPRINT(5, "%s:%d - addr[%s] port[%s] ttl[%s]\n", __func__, __LINE__, addr_s, port_s, ttl_s);

                        if(nullptr != addr_s) {
                            // OK
                            host = strdup(addr_s);
                        } else {
                            TRN_TRACE();
                            parse_err = true;
                        }
                        if(sscanf(port_s, "%d", &port) == 1 && port >= 0 ){
                            // OK
                        } else {
                            TRN_TRACE();
                            parse_err = true;
                        }
                        if(NULL != ttl_s && sscanf(ttl_s, "%d", &ttl) == 1 && ttl >= 0 ){
                            // OK
                        } // optional, use default
                        free(tcpy);
                    }
                } else {
                    TRN_TRACE();
                    parse_err = true;
                }
            }

            if(!parse_err) {

                TRN_NDPRINT(5, "%s:%d adding TRN key[%s] host[%s, %s:%d:%d] cfg[%s]\n", __func__, __LINE__, key, type, (host==NULL?"-":host), port, ttl, cfg);
                mTrnHostList.emplace_back(key, type, (host==NULL?"-":host), port, ttl, (void *)NULL, (cfg==NULL?"-":cfg));
                retval = 0;
            }

            free(host);
            host = nullptr;
            free(cpy);
        }  else {
            TRN_TRACE();
        }
        return retval;
    }

    int parse_input(const char *str)
    {
        int retval = -1;

        if(NULL != str){
            char *cpy = strdup(str);
            char *cur = NULL;
            char *chan_s = strtok_r(cpy, ",", &cur);
            char *opt_s = strtok_r(NULL, ",", &cur);

            enum {CHAN=0x1, DEPTH=0x2, GEO=0x4, INVPITCH=0x8, ERR=0x10};

            // set required flag values (vel optional)
            // DEO required for bath input only
            unsigned int flags = (CHAN);
            char *chan = NULL;
            char *geo_s = NULL;
            int depth = 10;
            bool invert_pitch = false;

            if(chan_s != NULL){
                chan = trnxpp_cfg::trim(chan_s);
                flags &= ~CHAN;
            } else {
                flags |= ERR;
            }

            while(NULL != opt_s && (flags & ERR)==0 )
            {

                TRN_NDPRINT(5,  "%s:%d - parsing opt_s[%s] \n", __func__, __LINE__, opt_s);

                if(strstr(opt_s, "depth:") != NULL) {

                    // discard option "depth:"
                    strtok(opt_s,":");
                    char *key_s = strtok(NULL,":");
                    char *val_key = trnxpp_cfg::trim(key_s);

                    if(strlen(val_key) > 0) {
                      TRN_NDPRINT(5,  "%s:%d - opt[%s] val_key[%s]\n", __func__, __LINE__, opt_s, val_key);

                        sscanf(val_key, "%d", &depth);

                        flags &= ~DEPTH;
                    } else {
                        flags |= ERR;
                    }
                } else if(strstr(opt_s, "invert-pitch:") != NULL) {

                    // discard option "invert-pitch:"
                    strtok(opt_s,":");
                    char *key_s = strtok(NULL,":");
                    char *val_key = trnxpp_cfg::trim(key_s);

                    if(strlen(val_key) > 0) {
                        TRN_NDPRINT(5,  "%s:%d - opt[%s] val_key[%s]\n", __func__, __LINE__, opt_s, val_key);

                        if(strcmp(val_key, "1") == 0 || strcmp(val_key, "y") == 0 ||strcmp(val_key, "Y") == 0)
                            invert_pitch = true;
                        else
                            invert_pitch = false;

                        flags &= ~INVPITCH;
                    } else {
                        flags |= ERR;
                    }
                } else if(strstr(opt_s, "geo:") != NULL) {

                    // discard option "geo:"
                    strtok(opt_s,":");
                    char *key_s = strtok(NULL,"*");
                    char *val_key = trnxpp_cfg::trim(key_s);

                    if(strlen(val_key) > 0) {
                        TRN_NDPRINT(5,  "%s:%d - opt[%s] val_key[%s]\n", __func__, __LINE__, opt_s, val_key);

                        geo_s = val_key;

                        flags &= ~GEO;
                    } else {
                        flags |= ERR;
                    }
                }
                TRN_NDPRINT(5,  "%s:%d - loop end: opt_s[%s] cur[%s]\n",__func__, __LINE__, opt_s, cur);

                if( (cur != NULL) && strstr(cur,"geo") != NULL )
                {
                    // geo contains ',', so parse to '*' geo terminator
                    opt_s = strtok_r(NULL, "*", &cur);
                } else {
                    opt_s = strtok_r(NULL, ",", &cur);
                }
            } // while

            if(flags == 0){
                TRN_NDPRINT(5,  "%s:%d - checking input chan[%s] depth[%d] geo[%s], inv_pitch[%c]\n", __func__, __LINE__, chan, depth, geo_s, (invert_pitch?'Y':'N'));

                trn::trn_lcm_input *listener = get_input(chan);

                if(listener == NULL){
                    // create input (chan, depth) if it doesn't exist
                    listener = create_input(chan, depth);

                    if(listener != nullptr) {

                        if(listener->provides_bath()){
                            // bath must provide GEO
                            flags |= GEO;
                            if( geo_s != NULL){
                                // parse geometry
                                // extra_s may contain additional options
                                trn::bath_input *bi = dynamic_cast<trn::bath_input *>(listener);
                                int btype = bi->bath_input_type();

                                TRN_NDPRINT(5,  "%s:%d - btype[%d] geo[%s]\n", __func__, __LINE__, btype, geo_s);

                                if(btype == BT_DVL) {

                                    dvlgeo *geo = dvlgeo::parse_dvlgeo(geo_s);
                                    mGeoList.emplace_back(chan, btype, geo);

                                    TRN_NDPRINT(5,  "%s:%d - added dvlgeo[%s, %d, %p]:\n%s\n", __func__, __LINE__, chan, btype, geo, geo->tostring().c_str());

                                } else if(btype == BT_DELTAT) {

                                    mbgeo *geo = mbgeo::parse_mbgeo(geo_s);
                                    mGeoList.emplace_back(chan, btype, geo);

                                    TRN_NDPRINT(5,  "%s:%d - added mbgeo[%s, %d, %p]\n", __func__, __LINE__, chan, btype, geo);

                                } else if(btype == BT_MULTIBEAM) {

                                    mbgeo *geo = mbgeo::parse_mbgeo(geo_s);
                                    mGeoList.emplace_back(chan, btype, geo);

                                    TRN_NDPRINT(5,  "%s:%d - added mbgeo[%s, %d, %p]\n", __func__, __LINE__, chan, btype, geo);

                                } else {
                                    TRN_NDPRINT(5,  "%s:%d - invalid btype[%d]\n", __func__, __LINE__, btype);
                                    // invalid type
                                    flags |= ERR;
                                }
                                flags &= ~GEO;
                            } else {
                                flags |= ERR;
                            }
                        }

                        if(listener->provides_att() && invert_pitch) {

                            trn::att_input *ap = dynamic_cast<trn::att_input *>(listener);
                            
                            ap->flags().set(trn::AF_INVERT_PITCH);
                        }

                        TRN_NDPRINT(2, "%s:%d - add input chan[%s] @[%p]\n",__func__, __LINE__, chan, listener);

                        // add input
                        add_input(chan, listener);


                        retval = 0;
                    }
                }
            }
            free(cpy);
        } // if str
        return retval;
    }

    // note: r_chan, r_sem return dynamically allocated strings, caller must free
    int parse_ctx_input(char *opt_s, const char *key, int *r_idx, char **r_chan, char **r_cb=NULL, int *r_tmout=NULL)
    {
        int retval = -1;

        if(NULL == opt_s || NULL == key || NULL == r_idx || NULL == r_chan)
        {
            return retval;
        }

        char *cpy = strdup(opt_s);

        char *raw_s = strtok(cpy, ":");
        char *key_s = trnxpp_cfg::trim(raw_s);

        TRN_NDPRINT(5,  "%s:%d - opt_s[%s] key[%s] \n", __func__, __LINE__, opt_s, key);


        if(NULL != key_s && strcmp(key, key_s) == 0) {

            char *idx_s = strtok(NULL, ":");
            char *chan_s = strtok(NULL, ":");
            char *cb_s = strtok(NULL, ":");
            char *tmout_s = strtok(NULL, ":");

            TRN_NDPRINT(5,  "%s:%d - key_s[%s] idx_s[%s] chan_s[%s] cb_s[%s] tmout_s[%s]\n", __func__, __LINE__, key_s, idx_s, chan_s, cb_s, tmout_s);

            int test_idx = -1;
            int test_tmout = -1;
            char *test_ch = NULL;
            char *test_cb = NULL;
            bool parse_err = false;

            if(NULL != idx_s) {
                if(sscanf(idx_s, "%d", &test_idx) != 1 || test_idx < 0){
                    TRN_TRACE();
                    parse_err = true;
                }
            } else {
                TRN_TRACE();
                parse_err = true;
            }

            if(NULL != chan_s) {
                test_ch = strdup(chan_s);
                if(test_ch == NULL){
                    TRN_TRACE();
                    parse_err = true;
                }
            } else {
                TRN_TRACE();
                parse_err = true;
            }

            if(NULL != cb_s && NULL != r_cb) {
                test_cb = strdup(cb_s);
                if(NULL == test_cb){
                    TRN_TRACE();
                    parse_err = true;
                }
            }

            if(NULL != tmout_s && NULL != r_tmout) {
                if(sscanf(tmout_s, "%d", &test_tmout) != 1 || test_tmout < 0){
                    TRN_TRACE();
                    parse_err = true;
                }
            }

            if(!parse_err){

                *r_idx = test_idx;

                *r_chan = test_ch;

                if(NULL != r_cb){
                    *r_cb = test_cb;
                }
                if(NULL != r_tmout){
                    *r_tmout = test_tmout;
                }

                retval = 0;
            } else {
                TRN_TRACE();
            }
        } else {
            TRN_TRACE();
        }

        free(cpy);
        return retval;
    }

    void add_ctx_sem(char *chan, char *cb_key, int tmout)
    {
        if(NULL != chan && NULL != cb_key){

            msg_callback cb_ptr = lookup_callback(cb_key);
            void *vp_cb_res = static_cast<void *>(callback_res());

            if(lookup_sem(chan, std::string(cb_key)) == nullptr) {
                list_add_sem(chan, tmout, cb_ptr, vp_cb_res);
                TRN_NDPRINT(5,  "%s:%d - add sem callback ch[%s] cb[%s/%p] to[%d]\n", __func__, __LINE__, chan, cb_key, cb_ptr, tmout);
            } else {
                fprintf(stderr, "%s:%d - ERR sem not added ch[%s] cb[%s/%p] to[%d]\n", __func__, __LINE__, chan, cb_key, cb_ptr, tmout);
            }
        }
    }

    int parse_ctx(const char *str)
    {
        int retval = -1;

        if(NULL != str){
            char *cpy = strdup(str);
            char *cur = NULL;
            char *opt_s = strtok_r(cpy, ",", &cur);

            enum {BATH=0x1, NAV=0x2, ATT=0x4, VEL=0x8, TRN=0x10, CB=0x20, KEY=0x40, LCM=0x80, ERR=0x100};

            // set required flag values (vel optional)
            // optional: KEY, VEL, LCM
            unsigned int flags = (CB|TRN|BATH|ATT|NAV);

            // create, configure context
            trnxpp_ctx *ctx = new trnxpp_ctx();

            while(NULL != opt_s && (flags & ERR)==0 )
            {
                int bath_idx = -1;
                int att_idx = -1;
                int nav_idx = -1;
                int vel_idx = -1;

                char *bath_ch = NULL;
                char *att_ch = NULL;
                char *nav_ch = NULL;
                char *vel_ch = NULL;

                char *bath_cb = NULL;
                char *nav_cb = NULL;
                char *att_cb = NULL;
                char *vel_cb = NULL;

                int bath_to = 100;
                int nav_to = 100;
                int att_to = 100;
                int vel_to = 100;

                char *mbcsv_path = nullptr;
                char *mbbin_path = nullptr;
                char *tecsv_path = nullptr;
                char *mecsv_path = nullptr;
                long int utm = 10;

                TRN_NDPRINT(5,  "%s:%d - parsing opt_s[%s] \n", __func__, __LINE__, opt_s);

                if(strstr(opt_s, "key:") != NULL) {

                    // discard option "key:"
                    strtok(opt_s,":");
                    char *key_s = strtok(NULL,":");
                    char *val_key = trnxpp_cfg::trim(key_s);

                    if(strlen(val_key) > 0) {
                        TRN_NDPRINT(5,  "%s:%d - opt[%s] val_key[%s]\n", __func__, __LINE__, opt_s, val_key);

                        ctx->set_ctx_key(val_key);

                        flags &= ~KEY;
                    } else {
                        flags |= ERR;
                    }
                } else if(strstr(opt_s, "cb:") != NULL) {

                    // discard option "cb:"
                    strtok(opt_s,":");
                    char *key_s = strtok(NULL,":");
                    char *val_key = trnxpp_cfg::trim(key_s);

                    if(strlen(val_key) > 0) {
                        TRN_NDPRINT(5,  "%s:%d - opt[%s] val_key[%s]\n", __func__, __LINE__, opt_s, val_key);

                        ctx->add_callback_key(val_key);

                        flags &= ~CB;
                    } else {
                        flags |= ERR;
                    }
                }  else if(strstr(opt_s, "decmod:") != NULL) {

                    // discard option "decmod:"
                    strtok(opt_s,":");
                    char *key_s = strtok(NULL,":");
                    char *val_key = trnxpp_cfg::trim(key_s);

                    if(strlen(val_key) > 0) {
                        TRN_NDPRINT(5,  "%s:%d - opt[%s] val_key[%s]\n", __func__, __LINE__, opt_s, val_key);
                        int decmod = 0;
                        if(sscanf(val_key, "%d", &decmod) == 1){
                            ctx->set_decmod(decmod);
                        } else {
                            flags |= ERR;
                        }
                    } else {
                        flags |= ERR;
                    }
                } else if(strstr(opt_s, "udpms:") != NULL) {

                    // discard option "udpms:"
                    strtok(opt_s,":");
                    char *key_s = strtok(NULL,":");
                    char *val_key = trnxpp_cfg::trim(key_s);

                    TRN_NDPRINT(5,  "%s:%d - key_s[%s] val_key[%s]\n", __func__, __LINE__, key_s, val_key);

                    if(strlen(val_key) > 0) {
                        TRN_NDPRINT(5,  "%s:%d - opt[%s] val_key[%s]\n", __func__, __LINE__, opt_s, val_key);

                        ctx->add_udpm_host(val_key, lookup_trn_host(val_key));

                        flags &= ~TRN;
                    } else {
                        flags |= ERR;
                    }
                }  else if(strstr(opt_s, "mbcsv:") != NULL)  {

                    // discard option "mbcsv:"
                    strtok(opt_s,":");
                    char *key_s = strtok(NULL,":");
                    char *val_key = trnxpp_cfg::trim(key_s);

                    mbcsv_path = trnxpp_cfg::trim(val_key);

                    TRN_NDPRINT(5,  "%s:%d - mbcsv_path[%s]\n", __func__, __LINE__, mbcsv_path);

                    ctx->set_mb1_csv_path(mbcsv_path);

                }  else if(strstr(opt_s, "mbbin:") != NULL)  {

                    // discard option "mbbin:"
                    strtok(opt_s,":");
                    char *key_s = strtok(NULL,":");
                    char *val_key = trnxpp_cfg::trim(key_s);

                    mbbin_path = trnxpp_cfg::trim(val_key);

                    TRN_NDPRINT(5,  "%s:%d - mbbin_path[%s]\n", __func__, __LINE__, mbbin_path);

                    ctx->set_mb1_bin_path(mbbin_path);

                } else if(strstr(opt_s, "tecsv:") != NULL)  {

                    // discard option "tecsv:"
                    strtok(opt_s,":");
                    char *key_s = strtok(NULL,":");
                    char *val_key = trnxpp_cfg::trim(key_s);

                    tecsv_path = trnxpp_cfg::trim(val_key);

                    TRN_NDPRINT(5,  "%s:%d - tecsv_path[%s]\n", __func__, __LINE__, tecsv_path);

                    ctx->set_trnest_csv_path(tecsv_path);

                } else if(strstr(opt_s, "mecsv:") != NULL)  {

                    // discard option "mecsv:"
                    strtok(opt_s,":");
                    char *key_s = strtok(NULL,":");
                    char *val_key = trnxpp_cfg::trim(key_s);

                    mecsv_path = trnxpp_cfg::trim(val_key);

                    TRN_NDPRINT(5,  "%s:%d - mecsv_path[%s]\n", __func__, __LINE__, tecsv_path);

                    ctx->set_mbest_csv_path(mecsv_path);

                } else if(strstr(opt_s, "utm:") != NULL)  {

                    // discard option "utm:"
                    strtok(opt_s,":");
                    char *key_s = strtok(NULL,":");
                    char *val_key = trnxpp_cfg::trim(key_s);

                    TRN_NDPRINT(5,  "%s:%d - val_key[%s]\n", __func__, __LINE__, val_key);

                    if(sscanf(val_key, "%ld", &utm) == 1){

                        TRN_NDPRINT(5,  "%s:%d - utm[%ld]\n", __func__, __LINE__, utm);

                        ctx->set_utm_zone(utm);
                    }

                } else  if(strstr(opt_s, "bi:") != NULL) {

                    if(parse_ctx_input(opt_s, "bi", &bath_idx, &bath_ch, &bath_cb, &bath_to) == 0){

                        TRN_NDPRINT(5,  "%s:%d - bath idx[%d] ch[%s] cb[%s]\n", __func__, __LINE__, bath_idx, bath_ch, bath_cb);

                        ctx->set_bath_input(bath_idx, bath_ch);

                        if(NULL != bath_cb){
                            add_ctx_sem(bath_ch, bath_cb, bath_to);
                            ctx->add_callback_key(bath_cb);
                        }

                        // free strings allocated by parse_ctx_input
                        free(bath_ch);
                        free(bath_cb);

                        flags &= ~BATH;
                    } else {
                        flags |= ERR;
                    }
                } else if(strstr(opt_s, "ai:") != NULL) {

                    if(parse_ctx_input(opt_s, "ai", &att_idx, &att_ch, &att_cb, &att_to) == 0){

                        TRN_NDPRINT(5,  "%s:%d - att idx[%d] ch[%s] cb[%s]\n", __func__, __LINE__, att_idx, att_ch, att_cb);

                        ctx->set_att_input_chan(att_idx, att_ch);

                        if(NULL != att_cb){
                            add_ctx_sem(att_ch, att_cb, att_to);
                            ctx->add_callback_key(att_cb);
                        }

                        // free strings allocated by parse_ctx_input
                        free(att_ch);
                        free(att_cb);

                        flags &= ~ATT;
                    } else {
                        flags |= ERR;
                    }
                } else if(strstr(opt_s, "ni:") != NULL) {

                    if(parse_ctx_input(opt_s, "ni", &nav_idx, &nav_ch, &nav_cb, &nav_to) == 0){

                        TRN_NDPRINT(5,  "%s:%d - nav idx[%d] ch[%s] cb[%s]\n", __func__, __LINE__, nav_idx, nav_ch, nav_cb);

                        ctx->set_nav_input_chan(nav_idx, nav_ch);

                        if(NULL != nav_cb){
                            add_ctx_sem(nav_ch, nav_cb, nav_to);
                            ctx->add_callback_key(nav_cb);
                        }

                        // free strings allocated by parse_ctx_input
                        free(nav_ch);
                        free(nav_cb);

                        flags &= ~NAV;
                    } else {
                        flags |= ERR;
                    }
                }  else if(strstr(opt_s, "vi:") != NULL) {

                    if(parse_ctx_input(opt_s, "vi", &vel_idx, &vel_ch, &vel_cb, &vel_to) == 0){

                        TRN_NDPRINT(5,  "%s:%d - vel idx[%d] ch[%s] cb[%s]\n", __func__, __LINE__, vel_idx, vel_ch, vel_cb);

                        ctx->set_vel_input_chan(vel_idx, vel_ch);

                        if(NULL != vel_cb){
                            add_ctx_sem(vel_ch, vel_cb, vel_to);
                            ctx->add_callback_key(vel_cb);
                        }

                        // free strings allocated by parse_ctx_input
                        free(vel_ch);
                        free(vel_cb);

                        flags &= ~VEL;
                    } else {
                        flags |= ERR;
                    }
                } else if(strstr(opt_s, "trn:") != NULL) {

                    // discard option "trn:"
                    strtok(opt_s,":");
                    char *key_s = strtok(NULL,":");
                    char *val_key = trnxpp_cfg::trim(key_s);

                    if(strlen(val_key) > 0) {
                        TRN_NDPRINT(5,  "%s:%d - opt[%s] val_key[%s]\n", __func__, __LINE__, opt_s, val_key);

                        trn::trn_host *thost = nullptr;
                        if((thost = lookup_trn_host(val_key)) != nullptr){
                            std::string trn_type = std::get<1>(*thost);
                            if(trn_type.compare("trncli") == 0){
                                ctx->add_trn_host(val_key, thost);
                            } else if(trn_type.compare("mb1pub") == 0){
                                ctx->add_mb1svr_host(val_key, thost);
                            }  else if(trn_type.compare("udpms") == 0){
                                ctx->add_udpm_host(val_key, thost);
                            } else {
                                TRN_NDPRINT(5,  "%s:%d - invalid trn type [%s]\n", __func__, __LINE__, trn_type.c_str());
                                flags |= ERR;
                            }
                        }

                        flags &= ~TRN;
                    } else {
                        flags |= ERR;
                    }
                } else if(strstr(opt_s, "lcm:") != NULL)  {

                    // discard option "lcm:"
                    strtok(opt_s,":");
                    char *key_s = strtok(NULL,":");
                    char *val_key = trnxpp_cfg::trim(key_s);

                    if(strlen(val_key) > 0) {
                        TRN_NDPRINT(5,  "%s:%d - opt[%s] val_key[%s]\n", __func__, __LINE__, opt_s, val_key);

                        ctx->parse_lcm_flags(val_key);

                        flags &= ~LCM;
                    } else {
                        flags |= ERR;
                    }
                } else {
                    flags |= ERR;
                }

                // get next
                opt_s = strtok_r(NULL, ",", &cur);
        }

            // OK if no flags set
            if(flags == 0){
                // add context
                mCtx.push_back(ctx);
                retval = 0;
            } else {
                TRN_NDPRINT(5,  "%s:%d - ERR flags set[%08X]\n", __func__, __LINE__, flags);
                // error, delete context
                delete ctx;
            }

            free(cpy);
        }
        return retval;
    }

    int parse_sem(const char *str)
    {
        int retval = -1;

        enum {CB=0x1, CHAN=0x2, ERR=0x4};
        // set required flag values (tmout optional)
        int flags = (CB|CHAN);

        if(NULL != str){
            char *cpy = strdup(str);
            char *cur = NULL;
            char *opt_s = strtok_r(cpy, ",", &cur);

            char *callback = NULL;
            char *chan = NULL;
            int tmout = 100;

            while(opt_s != NULL){
                if(strstr(opt_s, "cb") != NULL){
                    char *cp = strstr(opt_s, ":") + 1;
                    callback = trnxpp_cfg::trim(cp);
                    flags &= ~CB;
                } else if(strstr(opt_s, "chan") != NULL){
                    char *cp = strstr(opt_s, ":") + 1;
                    chan = trnxpp_cfg::trim(cp);
                    flags &= ~CHAN;
                } else if(strstr(opt_s, "tmout") != NULL){
                    char *cp = strstr(opt_s, ":") + 1;
                    sscanf(trnxpp_cfg::trim(cp), "%d", &tmout);
                } else  {
                    flags |= ERR;
                }
                opt_s = strtok_r(NULL, ",", &cur);
            }
            TRN_NDPRINT(5,  "%s:%d - cb[%s] chan[%s] tmout[%d]\n", __func__, __LINE__, callback, chan, tmout);

            if(flags == 0){
                add_ctx_sem(chan, callback, tmout);
                retval = 0;
            } else {
                fprintf(stderr, "%s:%d - ERR could not add sem\n", __func__, __LINE__);
            }
            free(cpy);
        }
        return retval;
    }

    void parse_config(trnxpp_cfg *cfg)
    {
        if(cfg == nullptr){
            fprintf(stderr, "%s:%d ERR - NULL config\n",__func__, __LINE__);
            return;
        }

        // create/configure TRN outputs
        std::list<std::string>::iterator it;

        std::list<std::string> list = cfg->trn_list();

        if(list.size() > 0) {
            for(it=list.begin(); it != list.end(); it++)
            {
                std::string str = static_cast<std::string>(*it);
                int test = parse_trn(str.c_str());
                TRN_NDPRINT(5,  "parsed trn[%s] stat[%d]\n\n", str.c_str(), test);
            }
        }

        list = cfg->input_list();
        if(list.size() > 0) {
            for(it=list.begin(); it != list.end(); it++)
            {
                std::string str = static_cast<std::string>(*it);
                int test = parse_input(str.c_str());
                TRN_NDPRINT(5,  "parsed input[%s] stat[%d]\n\n", str.c_str(), test);
            }
        }

        // create/configure semaphores
        list = cfg->sem_list();
        if(list.size() > 0) {
            for(it=list.begin(); it != list.end(); it++)
            {
                std::string str = static_cast<std::string>(*it);
                int test = parse_sem(str.c_str());
                TRN_NDPRINT(5,  "parsed sem[%s] stat[%d]\n\n", str.c_str(), test);
            }
        }

        // create/configure contexts
        list = cfg->ctx_list();
        if(list.size() > 0) {
            for(it=list.begin(); it != list.end(); it++)
            {
                std::string str = static_cast<std::string>(*it);
                int test = parse_ctx(str.c_str());
                TRN_NDPRINT(5,  "parsed ctx[%s] stat[%d]\n\n", str.c_str(), test);
            }
        }

    }

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
    std::vector<trnxpp_ctx *> mCtx;
    std::list<callback_kv> mCallbackList;
    callback_res_t mCallbackRes;
};

}

#endif
