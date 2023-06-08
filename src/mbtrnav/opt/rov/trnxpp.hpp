//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef TRNXPP_HPP  // include guard
#define TRNXPP_HPP

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <tuple>
#include <list>
#include <string>

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

namespace trn
{
// message callback type
typedef int (* msg_callback)(void *);
// channel, input ptr
using lcm_input = std::tuple<const std::string, trn_lcm_input*>;
// channel, timeout_sec, callback func, pargs, sem_count
using sem_reg = std::tuple<const std::string, int, msg_callback, void *, int>;
// channel, publisher
using lcm_pub = std::tuple<const std::string, pcf::lcm_publisher *>;

typedef enum{
    // these index into trnxpp::mCtx; change w/ care
    CTX_MBTRN=0,
    CTX_TRNSVR=1,
    CTX_COUNT
}trnxpp_ctx_id_t;
typedef uint32_t ctx_id_t;

class trnxpp_ctx
{
public:

    trnxpp_ctx()
    :mMB1Server(nullptr), mTrnCliRef(nullptr), mTerrainNavRef(nullptr), mUdpmSubRef(nullptr), mUtmZone(10), mCsvPath(), mCsvFile(nullptr)
    {}

    trnxpp_ctx(const trnxpp_ctx &other)
    :mMB1Server(other.mMB1Server), mTrnCliRef(other.mTrnCliRef), mTerrainNavRef(other.mTerrainNavRef),  mUdpmSubRef(other.mUdpmSubRef), mUtmZone(other.mUtmZone), mCsvPath(other.mCsvPath), mCsvFile(other.mCsvFile)
    {}

    ~trnxpp_ctx()
    {
        // don't delete pointers
    }

    void set_mb1_server(trn::mb1_server *svr)
    {
        mMB1Server = svr;
    }

    trn::mb1_server *mb1_server()
    {
        return mMB1Server;
    }

    void set_trn_client(TrnClient *tcli)
    {
        mTrnCliRef = tcli;
    }

    TrnClient *trn_client()
    {
        return mTrnCliRef;
    }

    void set_terrain_nav(TerrainNav *tcli)
    {
        mTerrainNavRef = tcli;
    }

    TerrainNav *terrain_nav()
    {
        return mTerrainNavRef;
    }

    void set_udpm_sub(udpm_sub_t *udpms)
    {
        mUdpmSubRef = udpms;
    }

    udpm_sub_t *udpm_sub()
    {
        return mUdpmSubRef;
    }

    void set_utm_zone(long int utm)
    {
        mUtmZone = utm;
    }

    long int utm_zone()
    {
        return mUtmZone;
    }

    void set_bath_input(const std::string &inp)
    {
        mBathInput = std::string(inp);
    }

    std::string bath_input()
    {
        return mBathInput;
    }

    void set_nav_input(const std::string &inp)
    {
        mNavInput = std::string(inp);
    }

    std::string nav_input()
    {
        return mNavInput;
    }

    void set_att_input(const std::string &inp)
    {
        mAttInput = std::string(inp);
    }

    std::string att_input()
    {
        return mAttInput;
    }

    void set_vel_input(const std::string &inp)
    {
        mVelInput = std::string(inp);
    }

    std::string vel_input()
    {
        return mVelInput;
    }

    void set_csv_path(const std::string &inp)
    {
        mCsvPath = std::string(inp);
    }

    std::string csv_path()
    {
        return mCsvPath;
    }

    FILE *csv_open()
    {
        if(mCsvFile == nullptr){

            TRN_NDPRINT(2, "%s:%d - opening CSV file[%s]\n", __func__, __LINE__, mCsvPath.c_str());
            if(logu::utils::open_file(&mCsvFile, mCsvPath, mCsvPath, true) == 0)
            {
                TRN_NDPRINT(2, "%s:%d - opened CSV file[%s]\n", __func__, __LINE__, mCsvPath.c_str());

            } else {
                TRN_DPRINT("%s:%d - ERR open CSV file[%s] failed\n", __func__, __LINE__, mCsvPath.c_str());
            }
        }
        return mCsvFile;
    }

    FILE *csv_file()
    {
        return mCsvFile;
    }

    void show(int wkey=15, int wval=18)
    {
        std::list<lcm_input>::iterator it;
        std::cerr << std::dec << std::setfill(' ');
        std::cerr << "--- trnxpp CTX ---" << "\n";
        std::cerr << std::setw(wkey) << "addr" << std::setw(wval) << this <<"\n";
        std::cerr << std::setw(wkey) << "mb1_svr" << std::setw(wval) << mMB1Server <<"\n";
        std::cerr << std::setw(wkey) << "trncli" << std::setw(wval) << mTrnCliRef <<"\n";
        std::cerr << std::setw(wkey) << "tnav" << std::setw(wval) << mTerrainNavRef <<"\n";
        std::cerr << std::setw(wkey) << "udpms" << std::setw(wval) << mUdpmSubRef <<"\n";
        std::cerr << std::setw(wkey) << "utm zone" << std::setw(wval) << mUtmZone <<"\n";
        std::cerr << std::setw(wkey) << "bath src" << std::setw(wval) << mBathInput <<"\n";
        std::cerr << std::setw(wkey) << "nav src" << std::setw(wval) << mNavInput <<"\n";
        std::cerr << std::setw(wkey) << "att src" << std::setw(wval) << mAttInput <<"\n";
        std::cerr << std::setw(wkey) << "vel src" << std::setw(wval) << mVelInput <<"\n";
        std::cerr << std::setw(wkey) << "csv path" << std::setw(wval) << mCsvPath <<"\n";
        std::cerr << std::setw(wkey) << "csv file" << std::setw(wval) << &mCsvFile <<"\n";
        std::cerr << "\n";
    }

protected:
private:
    trn::mb1_server *mMB1Server;
    TrnClient *mTrnCliRef;
    TerrainNav *mTerrainNavRef;
    udpm_sub_t *mUdpmSubRef;
    long int mUtmZone;
    std::string mBathInput;
    std::string mVelInput;
    std::string mNavInput;
    std::string mAttInput;
    std::string mCsvPath;
    FILE *mCsvFile;
};

class trnxpp
{
public:

    trnxpp(pcf::lcm_interface &lcm)
    :mLcm(nullptr), mInputList(), mSemList(), mPubList()
    {
        for(int i=0; i< CTX_COUNT;i++){
            mCtx[i] = trnxpp_ctx();
            mCtx[i].set_utm_zone(10);
        }
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

            TRN_NDPRINT(3, "testing sem chan[%s] count[%u]\n", channel.c_str(), sem->get_count());

            if(inp->test_sem(channel, to_msec)){

                TRN_NDPRINT(3, "calling sem chan[%s] count[%u]\n", channel.c_str(), sem->get_count());

                int stat = cb(parg);
                sem->clear_count();
                r_stat = stat;
                return 0;
            }
        }
        return -1;
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
            int stat = test_sem( channel, to_msec, cb, r_stat, parg, clear_pending);
            r_called += (stat==0 ? 1 : 0);
            r_error += (r_stat!=0 ? 1 : 0);
            it++;
            r_tested++;
        }
        return stat;
    }

    trnxpp_ctx &ctx(ctx_id_t id)
    {
        return mCtx[id];
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

    void publish_mb1(byte *data, size_t len)
    {
        if(mCtx[CTX_MBTRN].mb1_server() != nullptr)
            mCtx[CTX_MBTRN].mb1_server()->publish(data, len);
    }

    int trncli_connect(int retries, uint32_t delay_sec, bool *quit=NULL)
    {
        int retval = -1;
        int rem = retries;
        TrnClient *trn_client = mCtx[CTX_TRNSVR].trn_client();
        if(trn_client != nullptr){
            do{
                TerrainNav *mTerrainNavRef = trn_client->connectTRN();
                if(mTerrainNavRef!=NULL && trn_client->is_connected()){
                    mCtx[CTX_TRNSVR].set_terrain_nav(mTerrainNavRef);
                    retval=0;
                   break;
                }
                if(quit!=NULL && *quit)
                    break;
                if(delay_sec>0)
                    sleep(delay_sec);
            }while( (retries > 0 ? --rem > 0 : true) );
        }
        return retval;
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
            for(it=mInputList.begin(); it!=mInputList.end() ; it++, i++) {
                trn_lcm_input *x = std::get<1>(*it);
                os << std::dec << std::setfill(' ');
                os << std::setw(wkey-5) << "input["<< std::setw(3) << i <<"]\n";
                x->tostream(os, wkey, wval);
                os << "\n";
            }
        }
        os << std::setw(wkey) << std::dec << std::setfill(' ');
        os << "semaphores\n";
        if(mSemList.size()>0){
            std::list<sem_reg>::iterator it;
            int i=0;
            for(it=mSemList.begin(); it!=mSemList.end() ; it++, i++) {

                std::string name = std::get<0>(*it);
                int tmo = std::get<1>(*it);
                msg_callback cb = std::get<2>(*it);
                void *res = std::get<3>(*it);
                int count = std::get<4>(*it);

                os << std::setw(wkey-7) << "sem[" << std::setw(2) << i << "]\n";
                os << std::dec << std::setfill(' ');
                os << std::setw(wkey+1) << "name" << std::setw(wval) << name << "\n";
                os << std::setw(wkey+1) << "to_sec" << std::setw(wval) << tmo << "\n";
                os << std::setw(wkey+1) << "callback" << std::setw(wval) << (void *)cb << "\n";
                os << std::setw(wkey+1) << "res" << std::setw(wval) << res << "\n";
                os << std::dec << std::setfill(' ');
                os << std::setw(wkey+1) << "count" << std::setw(wval) << count << "\n";
            }
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
    
protected:

private:
    // LCM instance
    pcf::lcm_interface *mLcm;
    // input stream list
    std::list<lcm_input> mInputList;
    std::list<sem_reg> mSemList;
    std::list<lcm_pub> mPubList;
    trnxpp_ctx mCtx[CTX_COUNT];
};

}

#endif
