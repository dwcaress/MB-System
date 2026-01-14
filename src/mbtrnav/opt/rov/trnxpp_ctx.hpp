
//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef TRNXPP_CTX_HPP  // include guard
#define TRNXPP_CTX_HPP

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <tuple>
#include <thread>
#include <future>
#include <chrono>
#include <list>
#include <string>
#include <map>
#include <mutex>

#include <unistd.h>

#include "pcf_utils.hpp"
#include "lcm_interface.hpp"
#include "lcm_publisher.hpp"
#include "trn_lcm_input.hpp"
#include "mb1_server.hpp"
#include "TrnClient.h"
#include "udpm_sub.h"
#include "trn_debug.hpp"
#include "NavUtils.h"
#include "trnxpp_cfg.hpp"
#include "bathymetry_provider_IF.hpp"
#include "navigation_provider_IF.hpp"
#include "velocity_provider_IF.hpp"
#include "attitude_provider_IF.hpp"
#include "depth_provider_IF.hpp"
#include "mb1_provider_IF.hpp"
#include "trn_msg_utils.hpp"
#include "trnx_utils.hpp"
#include "GeoCon.hpp"

namespace trn
{
// fwd declaration
class trnxpp;

// message callback type
typedef int (* msg_callback)(void *);

class TrnHostX {
public:

    TrnHostX()
    : trnc_host(NULL)
    , udpms_host(NULL)
    , mb1s_host(NULL)
    { }

    ~TrnHostX()
    {
        // delegate deletion to owners
    }

    TrnClient *trnc_host;
    udpm_sub_t *udpms_host;
    mb1_server *mb1s_host;

};

// channel, input ptr
using lcm_input = std::tuple<const std::string, trn_lcm_input*>;
// channel, timeout_sec, callback func, pargs, sem_count
using sem_reg = std::tuple<const std::string, int, msg_callback, void *, int>;
// channel, publisher
using lcm_pub = std::tuple<const std::string, pcf::lcm_publisher *>;
// key, typestr, host, port, ttl, instance ptr, cfg path
// instance ptr is void to support different types
//using trn_host = std::tuple<std::string, std::string, std::string, int, int, void *, std::string>;
using trn_host = std::tuple<std::string, std::string, std::string, int, int, TrnHostX, std::string>;

// channel, type, geo_ptr
using beam_geo = std::tuple<const std::string, int, beam_geometry *>;
// key, callback
using callback_kv = std::tuple<const std::string, msg_callback>;
// ctx_key,
using trn_cfg_map = std::tuple<const std::string, const std::string>;

typedef enum{
    // these index into trnxpp::mCtx; change w/ care
    CTX_MBTRN=0,
    CTX_TRNSVR=1,
    CTX_COUNT
}trnxpp_ctx_id_t;

typedef uint32_t ctx_id_t;

typedef enum{
    LCM_NONE = 0x0,
    LCM_MBEST = 0x1,
    LCM_MB1SVR = 0x2,
    LCM_TRN_MOTN = 0x4,
    LCM_TRN_MEAS = 0x8,
    LCM_TRN_EST = 0x10,
    LCM_TRN_STAT = 0x20,
    LCM_TRN_ALL = 0x3C,
    LCM_MB_ALL = 0x3,
}lcm_flag_t;

class trnxpp_ctx
{
public:

    trnxpp_ctx()
    :
    mMB1CsvFile(nullptr)
    , mMB1BinFile(nullptr)
    , mTrnEstCsvFile(nullptr)
    , mMBEstCsvFile(nullptr)
    , mRawBathCsvFile(nullptr)
    , mUtmZone(10)
    , mGeoCRS(std::string(""))
    , mGeoCon(nullptr)
    , mDecMod(0)
    , mCBCount(0)
    , mCtxKey("undefined")
    , mMB1CsvPath()
    , mMB1BinPath()
    , mTrnEstCsvPath()
    , mMBEstCsvPath()
    , mRawBathCsvPath()
    , mLcmFlags(0)
    {
        // Use UTM by default (zone 10, Monterey Bay)
        // To configure, pass either utm:<zone> or geocrs:<crs>
        // The last one passed will be used
        mGeoCon = new GeoCon(mUtmZone);
    }

    trnxpp_ctx(const trnxpp_ctx &other)
    :
    mMB1CsvFile(other.mMB1CsvFile)
    , mMB1BinFile(other.mMB1BinFile)
    , mTrnEstCsvFile(other.mTrnEstCsvFile)
    , mMBEstCsvFile(other.mMBEstCsvFile)
    , mRawBathCsvFile(other.mRawBathCsvFile)
    , mUtmZone(other.mUtmZone)
    , mGeoCRS(other.mGeoCRS)
    , mGeoCon(other.mGeoCon)
    , mDecMod(other.mDecMod)
    , mCBCount(other.mCBCount)
    , mCtxKey(other.mCtxKey)
    , mMB1CsvPath(other.mMB1CsvPath)
    , mMB1BinPath(other.mMB1BinPath)
    , mTrnEstCsvPath(other.mTrnEstCsvPath)
    , mMBEstCsvPath(other.mMBEstCsvPath)
    , mRawBathCsvPath(other.mMBEstCsvPath)
    , mLcmFlags(other.mLcmFlags)
    {
    }

    ~trnxpp_ctx()
    {
        std::list<trn::trn_host>::iterator it;
        for(it = mMB1SvrList.begin(); it != mMB1SvrList.end(); it++){
            trn::trn_host thost = *it;
//            void *vp_inst = std::get<5>(thost);
//            trn::mb1_server *pinst = static_cast<trn::mb1_server *>(vp_inst);
  
            TrnHostX uhost = std::get<5>(thost);
            trn::mb1_server *pinst = uhost.mb1s_host;
if(pinst != nullptr)
            {
                delete pinst;
            }
        }
        for(it = mUdpmSubList.begin(); it != mUdpmSubList.end(); it++){
            trn::trn_host thost = *it;
//            void *vp_inst = std::get<5>(thost);
//            udpm_sub_t *pinst = static_cast<udpm_sub_t *>(vp_inst);
            TrnHostX uhost = std::get<5>(thost);
            udpm_sub_t *pinst = uhost.udpms_host;
          if(pinst != nullptr)
            {
                delete pinst;
            }
        }

        for(it = mTrnCliList.begin(); it != mTrnCliList.end(); it++){
            trn::trn_host thost = *it;
//            void *vp_inst = std::get<5>(thost);
//            TrnClient *pinst = static_cast<TrnClient *>(vp_inst);
            TrnHostX uhost = std::get<5>(thost);
            TrnClient *pinst = uhost.trnc_host;

            if(pinst != nullptr)
            {
                delete pinst;
            }
        }

        if(mMB1CsvFile != nullptr)
        {
            fclose(mMB1CsvFile);
        }

        if(mMB1BinFile != nullptr)
        {
            fclose(mMB1BinFile);
        }

        if(mTrnEstCsvFile != nullptr)
        {
            fclose(mTrnEstCsvFile);
        }

        if(mMBEstCsvFile != nullptr)
        {
            fclose(mMBEstCsvFile);
        }
        if(mRawBathCsvFile != nullptr)
        {
            fclose(mRawBathCsvFile);
        }
    }

    void tostream(std::ostream &os, int wkey=15, int wval=18)
    {
        int wx = wval;
        int alen = 0;

        std::list<lcm_input>::iterator it;
        os << std::dec << std::setfill(' ');

        os << std::setw(wkey) << "addr" << std::setw(wval) << this <<"\n";
        os << std::setw(wkey) << "key" << std::setw(wval) << mCtxKey.c_str() <<"\n";

        alen = strlen(lcm_flags_str().c_str());
        wx = (alen > wval ? alen+1 : wval);
        os << std::setw(wkey) << "lcm_flags" << std::setw(wx) << lcm_flags_str().c_str() <<"\n";

        os << std::setw(wkey) << "mb1_csv_file" << std::setw(wval) << mMB1CsvFile <<"\n";
        os << std::setw(wkey) << "mb1_bin_file" << std::setw(wval) << mMB1BinFile <<"\n";
        os << std::setw(wkey) << "trnest_csv_file" << std::setw(wval) << mTrnEstCsvFile <<"\n";
        os << std::setw(wkey) << "rawBath_csv_file" << std::setw(wval) << mRawBathCsvFile <<"\n";
        os << std::setw(wkey) << "utm zone" << std::setw(wval) << mUtmZone <<"\n";
        os << std::setw(wkey) << "geo_crs" << std::setw(wval) << mGeoCRS.c_str() <<"\n";
        os << std::setw(wkey) << "geocon" << std::setw(wval) << (mGeoCon != nullptr ? mGeoCon->typestr() : "NULL" ) <<"\n";
        os << std::setw(wkey) << "cb_count" << std::setw(wval) << mCBCount <<"\n";
        os << std::setw(wkey) << "cb_mod" << std::setw(wval) << mDecMod <<"\n";

        alen = strlen(mMB1CsvPath.c_str());
        wx = (alen > wval ? alen+1 : wval);
        os << std::setw(wkey) << "mb1_csv_path" << std::setw(wx) << mMB1CsvPath.c_str() <<"\n";

        alen = strlen(mMB1BinPath.c_str());
        wx = (alen > wval ? alen+1 : wval);
        os << std::setw(wkey) << "mb1_bin_path" << std::setw(wx) << mMB1BinPath.c_str() <<"\n";

        alen = strlen(mTrnEstCsvPath.c_str());
        wx = (alen > wval ? alen+1 : wval);
        os << std::setw(wkey) << "trnest_csv_path" << std::setw(wx) << mTrnEstCsvPath.c_str() <<"\n";

        alen = strlen(mRawBathCsvPath.c_str());
        wx = (alen > wval ? alen+1 : wval);
        os << std::setw(wkey) << "rawBath_csv_path" << std::setw(wx) << mRawBathCsvPath.c_str() <<"\n";

        os << std::endl;
        os << std::setw(wkey) << "MB1Servers" << std::setw(wval) << mMB1SvrList.size() <<"\n";
        std::list<trn::trn_host>::iterator hit;
        int i=0;
        for(i=0, hit = mMB1SvrList.begin(); hit != mMB1SvrList.end(); hit++){
            trn::trn_host thost = *hit;
            ostringstream ss;
            std::string key = std::get<0>(thost);
            std::string type = std::get<1>(thost);
            std::string host = std::get<2>(thost);
            int port = std::get<3>(thost);
            int ttl = std::get<4>(thost);
//             void *inst = std::get<5>(thost);
            TrnHostX inst = std::get<5>(thost);

            std::string cfg_path = std::get<6>(thost);
            ss << key.c_str() << ", ";
            ss << type.c_str() << ", ";
            ss << host.c_str() << ":" << port << ":" << ttl << ", ";
            ss << std::hex << (void *)&inst << ", " << std::dec;
            ss << cfg_path.c_str() << std::endl;

            alen = strlen(ss.str().c_str());
            wx = (alen > wval ? alen+1 : wval);
            os << std::setw(wkey-3) << "mb1[" << std::setw(2) << i << "]" << std::setw(wx) << ss.str().c_str() << std::endl;
        }

        os << std::setw(wkey) << "UDPm Subs" << std::setw(wval) << mUdpmSubList.size() <<"\n";
        for(i=0, hit = mUdpmSubList.begin(); hit != mUdpmSubList.end(); hit++){
            trn::trn_host thost = *hit;
            ostringstream ss;
            std::string key = std::get<0>(thost);
            std::string type = std::get<1>(thost);
            std::string host = std::get<2>(thost);
            int port = std::get<3>(thost);
            int ttl = std::get<4>(thost);
//             void *inst = std::get<5>(thost);
            TrnHostX inst = std::get<5>(thost);
            std::string cfg_path = std::get<6>(thost);
            ss << key.c_str() << ", ";
            ss << type.c_str() << ", ";
            ss << host.c_str() << ":" << port << ":" << ttl << ", ";
            ss << std::hex << &inst << ", " << std::dec;
            ss << cfg_path.c_str() << std::endl;

            alen = strlen(ss.str().c_str());
            wx = (alen > wval ? alen+1 : wval);
            os << std::setw(wkey-3) << "trncli[" << std::setw(2) << i << "]" << std::setw(wx) << ss.str().c_str() << std::endl;
        }

        os << std::setw(wkey) << "TrnClients" << std::setw(wval) << mTrnCliList.size() << std::endl;

        for(i=0, hit = mTrnCliList.begin(); hit != mTrnCliList.end(); hit++){
            trn::trn_host thost = *hit;
            ostringstream ss;
            std::string key = std::get<0>(thost);
            std::string type = std::get<1>(thost);
            std::string host = std::get<2>(thost);
            int port = std::get<3>(thost);
            int ttl = std::get<4>(thost);
//             void *inst = std::get<5>(thost);
            TrnHostX inst = std::get<5>(thost);
            std::string cfg_path = std::get<6>(*hit);
            ss << key.c_str() << ", ";
            ss << type.c_str() << ", ";
            ss << host.c_str() << ":" << port << ":" << ttl << ", ";
            ss << std::hex << &inst << ", " << std::dec;
            ss << cfg_path.c_str() << std::endl;

            alen = strlen(ss.str().c_str());
            wx = (alen > wval ? alen+1 : wval);
            os << std::setw(wkey-3) << "trncli[" << std::setw(2) << i << "]" << std::setw(wx) << ss.str().c_str() << std::endl;
        }

        os << std::endl;
        os << std::setw(wkey) << "Inputs" << std::setw(wval) << std::endl;

        std::vector<std::string>::iterator vit;

        for(i=0, vit = mBathInputKeys.begin(); vit != mBathInputKeys.end(); vit++, i++){
            std::string st = *vit;
            alen = strlen(st.c_str());
            wx = (alen > wval ? alen+1 : wval);
            os << std::setw(wkey-2) << "bath src[" << i << "]" << std::setw(wx) << st <<"\n";
        }
        for(i=0, vit = mMB1InputKeys.begin(); vit != mMB1InputKeys.end(); vit++, i++){
            std::string st = *vit;
            alen = strlen(st.c_str());
            wx = (alen > wval ? alen+1 : wval);
            os << std::setw(wkey-2) << "mb1 src[" << i << "]" << std::setw(wx) << st <<"\n";
        }
        for(i=0, vit = mDepthInputKeys.begin(); vit != mDepthInputKeys.end(); vit++, i++){
            std::string st = *vit;
            alen = strlen(st.c_str());
            wx = (alen > wval ? alen+1 : wval);
            os << std::setw(wkey-2) << "depth src[" << i << "]" << std::setw(wx) << st <<"\n";
        }
        for(i=0, vit = mNavInputKeys.begin(); vit != mNavInputKeys.end(); vit++, i++){
            std::string st = *vit;
            alen = strlen(st.c_str());
            wx = (alen > wval ? alen+1 : wval);
            os << std::setw(wkey-2) << "nav src[" << i << "]" << std::setw(wx) << st <<"\n";
        }
        for(i=0, vit = mAttInputKeys.begin(); vit != mAttInputKeys.end(); vit++, i++){
            std::string st = *vit;
            alen = strlen(st.c_str());
            wx = (alen > wval ? alen+1 : wval);
            os << std::setw(wkey-2) << "att src[" << i << "]" << std::setw(wx) << st <<"\n";
        }
        for(i=0, vit = mVelInputKeys.begin(); vit != mVelInputKeys.end(); vit++, i++){
            std::string st = *vit;
            alen = strlen(st.c_str());
            wx = (alen > wval ? alen+1 : wval);
            os << std::setw(wkey-2) << "vel src[" << i << "]" << std::setw(wx) << st <<"\n";
        }
        for(i=0, vit = mCallbackKeys.begin(); vit != mCallbackKeys.end(); vit++, i++){
            std::string st = *vit;
            alen = strlen(st.c_str());
            wx = (alen > wval ? alen+1 : wval);
            os << std::setw(wkey-2) << "callback[" << i << "]" << std::setw(wx) << st <<"\n";
        }

        os << std::endl;
        os << std::setw(wkey) << "Maps" << std::setw(wval) << std::endl;

        os << std::setw(wkey) << ": mUmap :";
        os << std::setw(wval - 2) << "[" << mUmap.size() << "]" << "\n";
        std::map<std::string, uint64_t>::iterator umit = mUmap.begin();
        while (umit != mUmap.end()) {
            os << std::setw(wkey) << umit->first;
            os << std::setw(wval) << static_cast<uint64_t>(umit->second) << "\n";
            ++umit;
        }

        os << std::setw(wkey) << ": mDmap :";
        os << std::setw(wval - 2) << "[" << mDmap.size() << "]" << "\n";
        std::map<std::string, double>::iterator dmit = mDmap.begin();
        while (dmit != mDmap.end()) {
            os << std::setw(wkey) << dmit->first;
            os << std::setw(wval) << static_cast<double>(dmit->second) << "\n";
            ++dmit;
        }

        os << std::setw(wkey) << ": mImap :";
        os << std::setw(wval - 2) << "[" << mImap.size() << "]" << "\n";
        std::map<std::string, int64_t>::iterator imit = mImap.begin();
        while (imit != mImap.end()) {
            os << std::setw(wkey) << imit->first;
            os << std::setw(wval) << static_cast<double>(imit->second) << "\n";
            ++imit;
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

    void set_ctx_key(const char *key)
    {
        mCtxKey = std::string(key);
    }

    std::string &ctx_key()
    {
        return mCtxKey;
    }

    void set_utm_zone(long int utm)
    {
        mUtmZone = utm;
        if(mGeoCon != nullptr)
            delete(mGeoCon);
        mGeoCon = new GeoCon(mUtmZone);
    }

    long int utm_zone()
    {
        return mUtmZone;
    }

    void set_geo_crs(std::string crs)
    {
        mGeoCRS = crs;
        if(mGeoCon != nullptr)
            delete(mGeoCon);
        mGeoCon = new GeoCon(crs.c_str());
    }

    std::string geo_crs()
    {
        return mGeoCRS;
    }

    GeoCon *geocon()
    {
        return mGeoCon;
    }

    void add_callback_key(const std::string &key){
        if(!has_callback(key))
            mCallbackKeys.push_back(key);
    }

    bool has_callback(const std::string &key)
    {
        std::vector<std::string>::iterator it;
        for(it = mCallbackKeys.begin(); it != mCallbackKeys.end(); it++){
            if((*it).compare(key) == 0){
                return true;
            }
        }
        return false;
    }

    int decmod()
    {
        return mDecMod;
    }

    void set_decmod(int n)
    {
        mDecMod = n;
    }

    int cbcount()
    {
        return mCBCount;
    }

    void set_cbcount(int n)
    {
        mCBCount = n;
    }

    void inc_cbcount()
    {
        mCBCount++;
    }


    void set_bath_input(int i, const std::string &inp)
    {
        if(mBathInputKeys.size() <= i) {
            TRN_NDPRINT(2, "%s - resizing %d > %d\n", __func__, mBathInputKeys.size(), i+1);
            mBathInputKeys.resize(i+1);
        }

        mBathInputKeys.at(i) = std::string(inp);
    }

    std::string *bath_input_chan(int i)
    {
        return mBathInputKeys.size() > i ? &mBathInputKeys.at(i) : nullptr;
    }

    void set_mb1_input(int i, const std::string &inp)
    {
        if(mMB1InputKeys.size() <= i) {
            TRN_NDPRINT(2, "%s - resizing %d > %d\n", __func__, mMB1InputKeys.size(), i+1);
            mMB1InputKeys.resize(i+1);
        }

        mMB1InputKeys.at(i) = std::string(inp);
    }

    std::string *mb1_input_chan(int i)
    {
        return mMB1InputKeys.size() > i ? &mMB1InputKeys.at(i) : nullptr;
    }

    void set_depth_input(int i, const std::string &inp)
    {
        if(mDepthInputKeys.size() <= i) {
            TRN_NDPRINT(2, "%s - resizing %d > %d\n", __func__, mDepthInputKeys.size(), i+1);
            mDepthInputKeys.resize(i+1);
        }

        mDepthInputKeys.at(i) = std::string(inp);
    }

    std::string *depth_input_chan(int i)
    {
        return mDepthInputKeys.size() > i ? &mDepthInputKeys.at(i) : nullptr;
    }

    void set_nav_input_chan(int i, const std::string &inp)
    {
        if(mNavInputKeys.size() <= i) {
            TRN_NDPRINT(2, "%s - resizing %d > %d\n", __func__, mNavInputKeys.size(), i+1);
            mNavInputKeys.resize(i+1);
        }

        mNavInputKeys.at(i) = std::string(inp);
    }

    std::string *nav_input_chan(int i)
    {
        return mNavInputKeys.size() > i ? &mNavInputKeys.at(i) : nullptr;
    }

    void set_att_input_chan(int i, const std::string &inp)
    {
        if(mAttInputKeys.size() <= i) {
            TRN_NDPRINT(2, "%s - resizing %d > %d\n", __func__, mAttInputKeys.size(), i+1);
            mAttInputKeys.resize(i+1);
        }

        mAttInputKeys.at(i) = std::string(inp);
    }

    std::string *att_input_chan(int i)
    {
        return mAttInputKeys.size() > i ? &mAttInputKeys.at(i) : nullptr;
    }

    void set_vel_input_chan(int i, const std::string &inp)
    {
        if(mVelInputKeys.size() <= i) {
            TRN_NDPRINT(2, "%s - resizing %d > %d\n", __func__, mVelInputKeys.size(), i+1);
            mVelInputKeys.resize(i+1);
        }

        mVelInputKeys.at(i) = std::string(inp);
    }

    pcf::lcm_publisher *get_pub(std::list<lcm_pub> &pubs, const std::string& channel)
    {
        if(!pubs.empty()){
            std::list<lcm_pub>::iterator it;

            for(it=pubs.begin(); it!=pubs.end() ; it++) {
                lcm_pub pub = (*it);
                const std::string key = std::get<0>(pub);
                if(key.compare(channel)==0){
                    pcf::lcm_publisher *x = std::get<1>(pub);
                    return x;
                }
            }
        }
        return nullptr;
    }

    std::string *vel_input_chan(int i)
    {
        return mVelInputKeys.size() > i ? &mVelInputKeys.at(i) : nullptr;
    }
////////////
    void set_trnest_csv_path(const std::string &inp)
    {
        mTrnEstCsvPath = std::string(inp);
    }

    std::string trnest_csv_path()
    {
        return mTrnEstCsvPath;
    }

    int init_trnest_csv_file(trnxpp_cfg *cfg)
    {
        int retval = -1;

        if(mTrnEstCsvFile != NULL)
        {
            fclose(mTrnEstCsvFile);
            mTrnEstCsvFile = nullptr;
        }

        set_trnest_csv_path(trnest_csv_path());
        mTrnEstCsvFile = trnest_csv_open();

        if(mTrnEstCsvFile == nullptr){
            LU_PERROR(cfg->mlog(), "TrnEst CSV file open failed");
        }
        return retval;
    }

    FILE *trnest_csv_open()
    {
        if(mTrnEstCsvFile == nullptr){

            TRN_NDPRINT(2, "%s:%d - opening TrnEst CSV file[%s]\n", __func__, __LINE__, mTrnEstCsvPath.c_str());
            if(logu::utils::open_file(&mTrnEstCsvFile, mTrnEstCsvPath, mTrnEstCsvPath, true) == 0)
            {
                TRN_NDPRINT(2, "%s:%d - opened TrnEst CSV file[%s]\n", __func__, __LINE__, mTrnEstCsvPath.c_str());

            } else {
                TRN_DPRINT("%s:%d - ERR open TrnEst CSV file[%s] failed\n", __func__, __LINE__, mTrnEstCsvPath.c_str());
            }
        }
        return mTrnEstCsvFile;
    }

    FILE *trnest_csv_file()
    {
        return mTrnEstCsvFile;
    }

    int write_trnest_csv(double &stime, poseT &pt, poseT &mle, poseT &mmse)
    {
        int retval = 0;

        // vi optional, valid if NULL
        if(nullptr != mTrnEstCsvFile){

            std::string ss = trnx_utils::trnest_tocsv(stime, pt, mle, mmse);

            fprintf(mTrnEstCsvFile, "%s\n", ss.c_str());

            //            TRN_NDPRINT(6, "%s\n", ss.c_str());

            retval = ss.length();
        } else {
            TRN_DPRINT("%s:%d - invalid arg mTrnEstCsvFile[%p]\n", __func__, __LINE__, mTrnEstCsvFile);
        }

        return retval;
    }
/////////////////
    void set_mbest_csv_path(const std::string &inp)
    {
        mMBEstCsvPath = std::string(inp);
    }

    std::string mbest_csv_path()
    {
        return mMBEstCsvPath;
    }

    int init_mbest_csv_file(trnxpp_cfg *cfg)
    {
        int retval = -1;

        if(mMBEstCsvFile != NULL)
        {
            fclose(mMBEstCsvFile);
            mMBEstCsvFile = nullptr;
        }

        set_mbest_csv_path(mbest_csv_path());
        mMBEstCsvFile = mbest_csv_open();

        if(mMBEstCsvFile == nullptr){
            LU_PERROR(cfg->mlog(), "MBEst CSV file open failed");
        }
        return retval;
    }

    FILE *mbest_csv_open()
    {
        if(mMBEstCsvFile == nullptr){

            TRN_NDPRINT(2, "%s:%d - opening MBEst CSV file[%s]\n", __func__, __LINE__, mMBEstCsvPath.c_str());
            if(logu::utils::open_file(&mMBEstCsvFile, mMBEstCsvPath, mMBEstCsvPath, true) == 0)
            {
                TRN_NDPRINT(2, "%s:%d - opened MBEst CSV file[%s]\n", __func__, __LINE__, mMBEstCsvPath.c_str());

            } else {
                TRN_DPRINT("%s:%d - ERR open MBEst CSV file[%s] failed\n", __func__, __LINE__, mMBEstCsvPath.c_str());
            }
        }
        return mMBEstCsvFile;
    }

    FILE *mbest_csv_file()
    {
        return mMBEstCsvFile;
    }

    int write_mbest_csv(trnu_pub_t &mbest)
    {
        int retval = 0;

        // vi optional, valid if NULL
        if(nullptr != mMBEstCsvFile){

            std::string ss = trnx_utils::mbest_tocsv(mbest);


            //trnx_utils::mbest_tostream(std::cerr, &mbest);

            fprintf(mMBEstCsvFile, "%s\n", ss.c_str());

            //            TRN_NDPRINT(6, "%s\n", ss.c_str());

            retval = ss.length();
        } else {
            TRN_DPRINT("%s:%d - invalid arg mMBEstCsvFile[%p]\n", __func__, __LINE__, mMBEstCsvFile);
        }

        return retval;
    }

//////////////////
    void set_mb1_csv_path(const std::string &inp)
    {
        mMB1CsvPath = std::string(inp);
    }

    std::string mb1_csv_path()
    {
        return mMB1CsvPath;
    }

    int init_mb1_csv_file(trnxpp_cfg *cfg)
    {
        int retval = -1;

        if(mMB1CsvFile != NULL)
        {
            fclose(mMB1CsvFile);
            mMB1CsvFile = nullptr;
        }

        set_mb1_csv_path(mb1_csv_path());
        mMB1CsvFile = mb1_csv_open();

        if(mMB1CsvFile != nullptr){
            fprintf(mMB1CsvFile,"# trnxpp TRN session start %s\n", cfg->session_string().c_str());
        } else {
            LU_PERROR(cfg->mlog(), "TRN CSV file open failed");
        }
        return retval;
    }

    FILE *mb1_csv_open()
    {
        if(mMB1CsvFile == nullptr){

            TRN_NDPRINT(2, "%s:%d - opening CSV file[%s]\n", __func__, __LINE__, mMB1CsvPath.c_str());
            if(logu::utils::open_file(&mMB1CsvFile, mMB1CsvPath, mMB1CsvPath, true) == 0)
            {
                TRN_NDPRINT(2, "%s:%d - opened CSV file[%s]\n", __func__, __LINE__, mMB1CsvPath.c_str());

            } else {
                TRN_DPRINT("%s:%d - ERR open CSV file[%s] failed\n", __func__, __LINE__, mMB1CsvPath.c_str());
            }
        }
        return mMB1CsvFile;
    }

    FILE *mb1_csv_file()
    {
        return mMB1CsvFile;
    }

    int write_mbest_csv(double &stime, trnu_pub_t &mbest)
    {
        int retval = 0;

        // vi optional, valid if NULL
        if(nullptr != mMBEstCsvFile){

            std::string ss = trnx_utils::mbest_tocsv(mbest);

            fprintf(mMBEstCsvFile, "%s\n", ss.c_str());

            //            TRN_NDPRINT(6, "%s\n", ss.c_str());

            retval = ss.length();
        } else {
            TRN_DPRINT("%s:%d - invalid arg mMBEstCsvFile[%p]\n", __func__, __LINE__, mMBEstCsvFile);
        }

        return retval;
    }

    int write_mb1_csv(mb1_t *snd, trn::bath_info *bi, trn::att_info *ai, trn::vel_info *vi=nullptr)
    {
        int retval = 0;

        // vi optional, valid if NULL
        if(nullptr != mMB1CsvFile && nullptr != snd && nullptr != ai && nullptr != bi){

            std::string ss = trnx_utils::mb1_to_csv(snd, bi, ai, mUmap["MBFMT"], vi);

            if(ss.length() > 1){
                fprintf(mMB1CsvFile, "%s\n", ss.c_str());
            }

            //            TRN_NDPRINT(6, "%s\n", ss.c_str());

            retval = ss.length();
        } else {
            TRN_DPRINT("%s:%d - invalid arg snd[%p] ai[%p] mMB1CsvFile[%p]\n", __func__, __LINE__, snd, ai, mMB1CsvFile);
        }

        return retval;
    }

    int write_mb1_csv(mb1_t *snd, trn::mb1_info *bi, trn::att_info *ai, trn::vel_info *vi=nullptr)
    {
        int retval = 0;

        // vi optional, valid if NULL
        if(nullptr != mMB1CsvFile && nullptr != snd && nullptr != ai && nullptr != bi){

            std::string ss = trnx_utils::mb1_to_csv(snd, bi, ai, mUmap["MBFMT"], vi);
            if(ss.length() > 1){
                fprintf(mMB1CsvFile, "%s\n", ss.c_str());
            }

            //            TRN_NDPRINT(6, "%s\n", ss.c_str());

            retval = ss.length();
        } else {
            TRN_DPRINT("%s:%d - invalid arg snd[%p] ai[%p] mMB1CsvFile[%p]\n", __func__, __LINE__, snd, ai, mMB1CsvFile);
        }

        return retval;
    }

    int write_csv_orig(trn::bath_info *bi, trn::att_info *ai, trn::nav_info *ni, trn::vel_info *vi=nullptr)
    {
        int retval = 0;

        // vi optional, valid if NULL
        if(nullptr != mMB1CsvFile && nullptr != bi && nullptr != ni && nullptr != ai){

            std::string ss = trnx_utils::lcm_to_csv_raw(bi, ai, ni, vi);

            fprintf(mMB1CsvFile, "%s\n", ss.c_str());

//            TRN_NDPRINT(6, "%s\n", ss.c_str());

            retval = ss.length();
        }

        return retval;
    }

    void set_mb1_bin_path(const std::string &inp)
    {
        mMB1BinPath = std::string(inp);
    }

    std::string mb1_bin_path()
    {
        return mMB1BinPath;
    }

    int init_mb1_bin_file(trnxpp_cfg *cfg)
    {
        int retval = -1;

        if(mMB1BinFile != NULL)
        {
            fclose(mMB1BinFile);
            mMB1BinFile = nullptr;
        }

        set_mb1_bin_path(mb1_bin_path());
        mMB1BinFile = mb1_bin_open();

        if(mMB1BinFile == nullptr){
            LU_PERROR(cfg->mlog(), "TRN MB1 file open failed");
        }
        return retval;
    }

    FILE *mb1_bin_open()
    {
        if(mMB1BinFile == nullptr){

            TRN_NDPRINT(2, "%s:%d - opening MB1 file[%s]\n", __func__, __LINE__, mMB1BinPath.c_str());
            if(logu::utils::open_file(&mMB1BinFile, mMB1BinPath, mMB1BinPath, true) == 0)
            {
                TRN_NDPRINT(2, "%s:%d - opened MB1 file[%s]\n", __func__, __LINE__, mMB1BinPath.c_str());

            } else {
                TRN_DPRINT("%s:%d - ERR open MB1 file[%s] failed\n", __func__, __LINE__, mMB1BinPath.c_str());
            }
        }
        return mMB1BinFile;
    }

    FILE *mb1_bin_file()
    {
        return mMB1BinFile;
    }

    size_t write_mb1_bin(mb1_t *snd)
    {
        size_t retval = -1;

        if(nullptr != snd && mMB1BinFile != nullptr){
            retval = fwrite(snd, snd->size, 1, mMB1BinFile);
            if(retval < 0){
                fprintf(stderr, "%s:%d - MB1 write failed [%d/%s]\n", __func__, __LINE__, errno, strerror(errno));
            }
            else {
                fprintf(stderr, "%s:%d - MB1 wrote [%lu] size[%u]\n", __func__, __LINE__, (unsigned long)retval, snd->size);
            }
        }
        return retval;
    }

    /////////////////
    void set_rawbath_csv_path(const std::string &inp)
    {
        mRawBathCsvPath = std::string(inp);
    }

    std::string rawbath_csv_path()
    {
        return mRawBathCsvPath;
    }

    int init_rawbath_csv_file(trnxpp_cfg *cfg)
    {
        int retval = -1;

        if(mRawBathCsvFile != NULL)
        {
            fclose(mRawBathCsvFile);
            mRawBathCsvFile = nullptr;
        }

        set_rawbath_csv_path(rawbath_csv_path());
        mRawBathCsvFile = rawbath_csv_open();

        if(mRawBathCsvFile != nullptr) {
//            fprintf(mRawBathCsvFile, "# unix_sec, ping_num, lat_r, lon_r, ");
//            fprintf(mRawBathCsvFile, "utmN_m, utmE_m, depth_m, roll_r, pitch_r, ");
//            fprintf(mRawBathCsvFile, "heading_r, n_beams, beam, range...\n");

        } else {
            LU_PERROR(cfg->mlog(), "RawBath CSV file open failed");
        }
        return retval;
    }

    FILE *rawbath_csv_open()
    {
        if(mRawBathCsvFile == nullptr){

            TRN_NDPRINT(2, "%s:%d - opening RawBath CSV file[%s]\n", __func__, __LINE__, mRawBathCsvPath.c_str());
            if(logu::utils::open_file(&mRawBathCsvFile, mRawBathCsvPath, mRawBathCsvPath, true) == 0)
            {
                TRN_NDPRINT(2, "%s:%d - opened RawBath CSV file[%s]\n", __func__, __LINE__, mRawBathCsvPath.c_str());

            } else {
                TRN_DPRINT("%s:%d - ERR open RawBath CSV file[%s] failed\n", __func__, __LINE__, mRawBathCsvPath.c_str());
            }
        }
        return mRawBathCsvFile;
    }

    FILE *rawbath_csv_file()
    {
        return mRawBathCsvFile;
    }

    int write_rawbath_csv(trn::bath_info *bi, trn::nav_info *ni, trn::att_info *ai, trn::vel_info *vi, long int utm, double alt_depth=-1.)
    {
        int retval = 0;

        // vi optional, valid if NULL
        if(nullptr != mRawBathCsvFile){

            std::string ss = trnx_utils::rawbath_tocsv(bi, ni, ai, vi, utm, mUmap["RBFMT"] ,alt_depth);

            fprintf(mRawBathCsvFile, "%s\n", ss.c_str());

            retval = ss.length();
        } else {
            TRN_DPRINT("%s:%d - invalid arg mRawBathCsvFile[%p]\n", __func__, __LINE__, mRawBathCsvFile);
        }

        return retval;
    }

    trn::trn_host *lookup_udpm_host(std::string key)
    {
        trn::trn_host *retval = nullptr;
        std::list<trn::trn_host>::iterator it;
        for(it = mUdpmSubList.begin(); it != mUdpmSubList.end(); it++){
            std::string list_key = std::get<0>(*it);
            if(list_key.compare(key) == 0){
                retval = &(*it);
                break;
            }
        }
        return retval;
    }

    udpm_sub_t *lookup_udpm_sub(std::string key)
    {
        udpm_sub_t *retval = nullptr;
        std::list<trn::trn_host>::iterator it;
        for(it = mUdpmSubList.begin(); it != mUdpmSubList.end(); it++){
            std::string list_key = std::get<0>(*it);
//            void *inst = std::get<5>(*it);
            TrnHostX uhost = std::get<5>(*it);
            if(list_key.compare(key) == 0){
//                retval = static_cast<udpm_sub_t *>(inst);
                retval = uhost.udpms_host;
                break;
            }
        }
        return retval;
    }

    int add_udpm_host(std::string key, trn::trn_host *host)
    {
        int retval = -1;

        trn::trn_host *list_host = lookup_udpm_host(key);

        if(list_host == nullptr){
            // copy host
            mUdpmSubList.push_back(*host);
            retval = 0;
        }

        return retval;
    }

    // returns number of bytes received or -1 on error
    int get_udpms_update(const std::string &key, trnxpp_cfg *cfg, byte *dest, size_t len)
    {
        int retval = 0;

        udpm_sub_t *trnum_cli = lookup_udpm_sub(key);

        if(NULL != trnum_cli)
        {
            int test_con=0;

            // check/connect UDPm client connection
            if(!udpms_is_connected(trnum_cli))
            {
                TRN_NDPRINT(4, "connecting TRNUM client\n");
                cfg->stats().mb_cli_dis++;
                test_con = udpms_connect(trnum_cli, true, false, false);

                if(test_con == 0){
                    cfg->stats().mb_cli_con++;
                }
            }

            if(test_con == 0){

                byte iobuf[512]={0};
                memset(iobuf, 0, 512);

                TRN_NDPRINT(4, "TRNUM client listening...\n");

                // listen for UDP mcast TRN update (trnum)
                uint32_t update_len = sizeof(trnu_pub_t);
                int64_t test = udpms_listen(trnum_cli, iobuf, update_len, 100, 0);

                if(test == update_len){
                    uint32_t *msg_id = (uint32_t *)iobuf;
                    if(len >= test && *msg_id == TRNU_PUB_SYNC){

//                        TRN_NDPRINT(4, "%s:%d TRNUM sync bytes %04X vs %04X\n", __func__, __LINE__, *msg_id, (uint32_t)TRNU_PUB_SYNC);
//                        mb1_hex_show(iobuf, update_len, 16, true, 5);

                        memset(dest, 0, len);
                        memcpy(dest, iobuf, test);
                        retval = test;

                        // got UDPm TRN estimate; update stats
                        cfg->stats().mb_est_n++;

                    } else {

                    }

                }else{
                    TRN_NDPRINT(4, "TRNUM no update\n");
                }
            }else{
                TRN_NDPRINT(4, "TRNUM not connected\n");
                retval = -1;
            }
        }

        return retval;
    }

    int start_udpmsub(const std::string &key, trnxpp_cfg *cfg)
    {
        int retval = -1;

        trn::trn_host *udpm_host = lookup_udpm_host(key);

        if(udpm_host == nullptr)
            return retval;

//        void *vp = std::get<5>(*udpm_host);
        TrnHostX uhost = std::get<5>(*udpm_host);
        std::string group = std::get<2>(*udpm_host);
        int port = std::get<3>(*udpm_host);
        int ttl = std::get<4>(*udpm_host);

//        udpm_sub_t *udpmsub = static_cast<udpm_sub_t *>(vp);
        udpm_sub_t *udpmsub = uhost.udpms_host;

        if(udpmsub == nullptr){
            // create if not set
            udpmsub = udpms_cnew(group.c_str(), port, ttl);
        }

        if(udpmsub == nullptr)
            return retval;

//        std::get<5>(*udpm_host) = static_cast<void *>(udpmsub);
        uhost.udpms_host = udpmsub;
        std::get<5>(*udpm_host) = uhost;

        int debug = (NULL != cfg ? cfg->debug() : 0);
        udpms_set_debug(debug);

        udpms_connect(udpmsub, true, false, false);

        if(udpms_is_connected(udpmsub)){

            cfg->stats().mb_cli_con++;

            LU_PEVENT(cfg->mlog(), "trnum_cli connected [%s:%d] ttl[%d]",group.c_str(), port, ttl);

        } else {
            LU_PERROR(cfg->mlog(), "trnum_cli connect failed [%s:%d] ttl[%d] [%d/%s]",group.c_str(), port, ttl, errno, strerror(errno));
        }

        return retval;
    }

    trn::trn_host *lookup_mb1svr_host(std::string key)
    {
        trn::trn_host *retval = nullptr;
        std::list<trn::trn_host>::iterator it;
        for(it = mMB1SvrList.begin(); it != mMB1SvrList.end(); it++){
            std::string list_key = std::get<0>(*it);
            if(list_key.compare(key) == 0){
                retval = &(*it);
                break;
            }
        }
        return retval;
    }

    int add_mb1svr_host(std::string key, trn::trn_host *host)
    {
        int retval = -1;

        trn::trn_host *list_host = lookup_mb1svr_host(key);

        if(list_host == nullptr){
            // copy host
            mMB1SvrList.push_back(*host);
            retval = 0;
        }

        return retval;
    }

    int set_mb1svr_inst(std::string key, trn::mb1_server *inst)
    {
        int retval = -1;
        std::list<trn::trn_host>::iterator it;
        for(it = mMB1SvrList.begin(); it != mMB1SvrList.end(); it++){
            std::string list_key = std::get<0>(*it);
            if(list_key.compare(key) == 0){
//                std::get<5>(*it) = static_cast<void *>(inst);
                TrnHostX uhost = std::get<5>(*it);
                uhost.mb1s_host = inst;
                std::get<5>(*it) = uhost;
                retval = 0;
                break;
            }
        }
        return retval;
    }

    int start_mb1svr(const std::string &key, trnxpp_cfg *cfg)
    {
        int retval = -1;

        trn::trn_host *mb1_host = lookup_mb1svr_host(key);

        if(mb1_host == nullptr)
            return retval;

//        void *vp = std::get<5>(*mb1_host);
        TrnHostX uhost = std::get<5>(*mb1_host);
//        trn::mb1_server *mb1svr = static_cast<trn::mb1_server *>(vp);
        trn::mb1_server *mb1svr = uhost.mb1s_host;

        if(nullptr != mb1svr) {
            delete mb1svr;
            mb1svr = nullptr;
        }

        std::string host = std::get<2>(*mb1_host);
        int port = std::get<3>(*mb1_host);
        mb1svr = new trn::mb1_server((char *)host.c_str(), port);

        if(mb1svr == nullptr)
            return retval;

        uhost.mb1s_host = mb1svr;
        std::get<5>(*mb1_host) = uhost;

        set_mb1svr_inst(key, mb1svr);

        int debug = (NULL != cfg ? cfg->debug() : 0);
        mb1svr->set_debug(debug);

        mb1svr->initialize(host.c_str(), port);
        int test = mb1svr->connect_svr();
        if(test == 0){
            LU_PEVENT(cfg->mlog(), "mb1svr connected [%s %s:%d]",key.c_str(),host.c_str(), port);
        }

        return retval;
    }

    int mb1svr_count()
    {
        return mMB1SvrList.size();
    }

    int pub_mb1(mb1_t *sounding, std::list<lcm_pub> &pubs, trnxpp_cfg *cfg)
    {
        int retval = -1;

        if(nullptr == sounding)
            return retval;

        if(mMB1SvrList.size() <= 0){
            return 0;
        }

        // publish to MB1 servers
        std::list<trn::trn_host>::iterator it;
        for(it = mMB1SvrList.begin(); it != mMB1SvrList.end(); it++){
            trn::trn_host thost = (*it);
            std::string key = std::get<0>(thost);

//            void *vp = std::get<5>(thost);
            TrnHostX uhost = std::get<5>(thost);
            void *vp = uhost.mb1s_host;

            if(vp == nullptr){
                continue;
            }

            TRN_NDPRINT(5, "%s:%d - pub MB1SVR key[%s] vp[%p]\n", __func__, __LINE__, key.c_str(), vp);
//            trn::mb1_server* mb1svr = static_cast<trn::mb1_server *>(vp);
            trn::mb1_server* mb1svr = uhost.mb1s_host;

            mb1svr->publish((byte *)sounding, sounding->size);
            cfg->stats().mb_pub_n++;
        }

        // publish TRN inputs to LCM
        if(lcm_is_enabled(trn::LCM_MB1SVR)){

            pcf::lcm_publisher *pub = get_pub(pubs, "MB1_PUB");
            if(pub != nullptr){
                TRN_NDPRINT(5, "%s:%d - pub MB1_PUB\n", __func__, __LINE__);
                trn::trn_mb1_t mb1_msg;
                trn::trn_msg_utils::mb1_to_lcm(mb1_msg, sounding);
                pub->publish(mb1_msg);
                if(cfg != nullptr){
                    cfg->stats().mb_pub_mb1_n++;
                }
            }
        }

        // publish udpm updates to LCM
        if(lcm_is_enabled(trn::LCM_MBEST)){
            pcf::lcm_publisher *pub = get_pub(pubs, "MB1_EST");
            if(pub != nullptr){

                for(it = mUdpmSubList.begin(); it != mUdpmSubList.end(); it++){
                    trn::trn_host thost = (*it);
                    std::string key = std::get<0>(thost);

//                    void *vp = std::get<5>(thost);
                    TrnHostX uhost = std::get<5>(thost);
                    void *vp = uhost.udpms_host;

                    if(vp == nullptr){
                        continue;
                    }

                    // get TRNUM update/estimate
                    byte iobuf[512];
                    memset(iobuf, 0, 512);

                    int update_bytes = get_udpms_update(key, cfg, iobuf, sizeof(trnu_pub_t));

                    TRN_NDPRINT(5, "%s:%d - UDPM update key[%s] vp[%p] update_bytes[%d] trn_pub_t size[%u]\n", __func__, __LINE__, key.c_str(), vp, update_bytes, sizeof(trnu_pub_t));

                    if(update_bytes > 0){

                        trnu_pub_t *mbest = (trnu_pub_t *)iobuf;

                        std::string est_str = trnx_utils::mbest_tostring(mbest);

//                        LU_PEVENT(cfg->mlog(), "udpm est:\n%s\n", est_str.c_str());

                        if(cfg->debug() >= 5 ){
                            fprintf(stderr, "%s - udpm est:\n%s\n", __func__,
                            est_str.c_str());
                        }

                        // write TRN estimate CSV (compatible w/ tlp-plot)
                        write_mbest_csv(*mbest);

                        TRN_NDPRINT(5, "%s:%d - pub MB1_EST\n", __func__, __LINE__);
                        trn::trnupub_t trnu_msg;
                        trn::trn_msg_utils::trnupub_to_lcm(trnu_msg, mbest);

                        if(cfg != nullptr && mbest->success != 0){
                            cfg->stats().mb_est_ok_n++;
                        }

                        pub->publish(trnu_msg);
                        cfg->stats().mb_pub_est_n++;
                    }
                }
            }
        }

        return retval;
    }

    trn::trn_host *lookup_trncli_host(std::string key)
    {
        trn::trn_host *retval = nullptr;
        std::list<trn::trn_host>::iterator it;
        int i=0;
        for(it = mTrnCliList.begin(); it != mTrnCliList.end(); it++, i++){
            std::string list_key = std::get<0>(*it);
            if(list_key.compare(key) == 0){
                retval = &(*it);
                break;
            }
        }
        return retval;
    }

    int add_trn_host(std::string key, trn::trn_host *host)
    {
        int retval = -1;

        trn::trn_host *list_host = lookup_trncli_host(key);
        fprintf(stderr, "%s:%d - ************************ key %s host %p list_host %p\n", __func__, __LINE__, key.c_str(), host, list_host);

        if(list_host == nullptr){
            // copy host
            mTrnCliList.push_back(*host);
            retval = 0;
        }

        return retval;
    }

    int trncli_connect(const std::string &key, int retries, uint32_t delay_sec, bool *quit=NULL)
    {
        int retval = -1;
        int rem = retries;

        trn::trn_host *trnc_host = lookup_trncli_host(key);

        if(trnc_host == nullptr)
            return retval;
//
//        void *vp = std::get<5>(*trnc_host);
//        TrnClient *trncli = static_cast<TrnClient *>(vp);

        TrnHostX uhost = std::get<5>(*trnc_host);
        void *vp = uhost.trnc_host;
        TrnClient *trncli = uhost.trnc_host;

        if(trncli != nullptr){
            TrnAttr &att_ref = trncli->getTrnAttr();
            TrnAttr *patt = &att_ref;
            do{
                fprintf(stderr, "%s:%d trncli[%p] trn_attr[%p] rem[%d] del[%d]\n", __func__, __LINE__, trncli, patt, rem, delay_sec);
                TerrainNav *tnav = trncli->connectTRN();
                if(tnav!=NULL && trncli->is_connected()){
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

    int start_trncli(const std::string &key, trnxpp_cfg *cfg, bool force_reconnect, bool *user_int)
    {
        int retval = -1;

        if(NULL == cfg)
            return retval;

        trn::trn_host *trnc_host = lookup_trncli_host(key);

        if(trnc_host == nullptr) {
            fprintf(stderr, "%s:%d trncli_host[%s] not found\n", __func__, __LINE__, key.c_str());
            return retval;
        }

        TrnHostX uhost = std::get<5>(*trnc_host);
        void *vp = uhost.trnc_host;
        TrnClient *trncli = uhost.trnc_host;

        if(NULL != trncli) {

            if(!force_reconnect && trncli->is_connected()){
                fprintf(stderr, "%s:%d trncli[%p] already connected\n", __func__, __LINE__, trncli);
               return 0;
            }

            fprintf(stderr, "%s:%d !!!!!!!!!!! DELETING trncli[%p] !!!!!!!!!!!!\n", __func__, __LINE__, trncli);
            delete trncli;
            trncli = NULL;
            uhost.trnc_host = NULL;
            std::get<5>(*trnc_host) = uhost;
        }

        fprintf(stderr, "%s:%d constructing trncli[%p]\n", __func__, __LINE__, trncli);
        trncli = new TrnClient("localhost",TRNCLI_PORT_DFL);
        fprintf(stderr, "%s:%d new trncli[%p]\n", __func__, __LINE__, trncli);


        // TODO: WIP is TrnClient initialization correct?
        std::string cfg_path_str = std::get<6>(*trnc_host);
        // set up log directory (using app config --logdir)
        trncli->initLogDirectory(cfg->logdir().c_str(), NULL, 0, true);
        // load TRN configuration and configure TrnClient
        trncli->loadCfgAttributes(cfg_path_str.c_str()), cfg->logdir().c_str();

        uhost.trnc_host = trncli;
        std::get<5>(*trnc_host) = uhost;

        TrnAttr &att_ref = trncli->getTrnAttr();
        TrnAttr *patt = &att_ref;

        fprintf(stderr, "%s:%d cfg_path_str [%s] key[%s] host.cli[%p] trncli[%p] trn_attr[%p]  ---------------------------\n", __func__, __LINE__, cfg_path_str.c_str(), key.c_str(), std::get<5>(*trnc_host).trnc_host, trncli, patt);

//        int tcc = trncli_connect(key, 10, 3, user_int);
        int tcc = trncli_connect(key, 1, 0, user_int);

        if(trncli->is_connected()){
            LU_PEVENT(cfg->mlog(), "trn client connected");
            TRN_NDPRINT(1, "%s:%d trn client connected\n", __func__, __LINE__);
            cfg->stats().trn_cli_con++;
            retval = 0;
        } else {
            LU_PERROR(cfg->mlog(), "trn client connect failed [%d]", tcc);
            TRN_NDPRINT(1, "%s:%d trn client failed [%d]\n", __func__, __LINE__, tcc);
        }

        if(cfg->debug()>0 && trncli != NULL){
            trncli->show();
        }

        fprintf(stderr, "%s:%d -  //////////// START_TRNCLI EXITING ////////////\n", __func__, __LINE__);

        return retval;
    }

    int start_trn(trnxpp_cfg *cfg, bool *user_int)
    {
        int retval = -1;

        int err_count = 0;
        std::list<trn::trn_host>::iterator it;

        TRN_NDPRINT(1, "%s:%d - starting mb1pubs [%d]\n",__func__, __LINE__, mMB1SvrList.size());

        for(it = mMB1SvrList.begin(); it != mMB1SvrList.end(); it++){
            trn::trn_host thost = *it;

            std::string key = std::get<0>(thost);

            TRN_NDPRINT(1, "%s:%d - starting mb1pub [%s]\n",__func__, __LINE__, key.c_str());

            if(start_mb1svr(key, cfg) != 0){
                err_count++;
            }
        }

        TRN_NDPRINT(1, "%s:%d - starting udpms [%d]\n",__func__, __LINE__, mUdpmSubList.size());

        for(it = mUdpmSubList.begin(); it != mUdpmSubList.end(); it++){

            trn::trn_host thost = *it;
            std::string key = std::get<0>(thost);

            TRN_NDPRINT(1, "%s:%d - starting udpm sub [%s]\n",__func__, __LINE__, key.c_str());

            if(start_udpmsub(key, cfg) != 0){
                err_count++;
            }
        }

        TRN_NDPRINT(1, "%s:%d - starting trnclis [%d]\n",__func__, __LINE__, mTrnCliList.size());

        for(it = mTrnCliList.begin(); it != mTrnCliList.end(); it++){

            trn::trn_host thost = *it;
            std::string key = std::get<0>(thost);

            TRN_NDPRINT(1, "%s:%d - starting trncli [%s]\n",__func__, __LINE__, key.c_str());

            if(start_trncli(key, cfg, true, user_int) != 0){
                err_count++;
            }
        }

        retval = (err_count == 0 ? 0 : -1);

        return retval;
    }

    static void tcli_start_worker_fn(TrnClient *trncli, std::promise<bool> *con_promise)
    {
        try{
            TrnAttr &att_ref = trncli->getTrnAttr();
            TrnAttr *patt = &att_ref;

            fprintf(stderr, "%s:%d starting trncli:  trncli[%p] trn_attr[%p] trncli_sz[%zu]\n",__func__, __LINE__, trncli, patt, sizeof(*trncli));

            TerrainNav *tnav = trncli->connectTRN();

            bool iscon = false;

            if(tnav != nullptr)
                iscon = trncli->is_connected();

            fprintf(stderr, "%s:%d setting promise: tnav[%p] trncli[%p] trn_attr[%p]  iscon[%c]\n",__func__, __LINE__, tnav, trncli, patt, (iscon?'Y':'N'));

            con_promise->set_value(iscon);
        } catch(Exception e) {
            fprintf(stderr, "%s:%d - Exception during connectTRN\n", __func__, __LINE__);
        }
    }

    // check connection, re-connect as needed (in separate thread)
    // return true if connected;
    bool trncli_check_connection(int idx, TrnClient *trnc, trnxpp_cfg *cfg)
    {
        bool retval = false;

        static std::vector<bool> connection_pending;
        static std::vector<bool> is_connected;
        static std::vector<std::thread *> workers;
        static std::vector<std::future<bool>> con_futures;
        static std::vector<std::promise<bool> *> con_promises;

// original: should probably check mTrnCliList size, rather than one-time
//        if(workers.size() == 0){
//            for(int i=0; i<mTrnCliList.size(); i++){
//                connection_pending.push_back(false);
//                is_connected.push_back(false);
//                workers.push_back(nullptr);
//                con_promises.push_back(nullptr);
//            }
//            con_futures.resize(mTrnCliList.size());
//        }
        // TODO: should (re-)initialize any new values
        if(workers.size() < mTrnCliList.size()){
            size_t old_size = workers.size();
            size_t new_size = mTrnCliList.size();
            connection_pending.resize(mTrnCliList.size());
            is_connected.resize(mTrnCliList.size());
            workers.resize(mTrnCliList.size());
            con_promises.resize(mTrnCliList.size());
            con_futures.resize(mTrnCliList.size());

            for( size_t i=old_size; i < new_size; i++) {
                connection_pending.at(i) = false;
                is_connected.at(i) = false;
                workers.at(i) = nullptr;
                con_promises.at(i) = nullptr;
            }
        }


        // return if already connected
        if(trnc->is_connected() && !connection_pending.at(idx))
            return true;

        fprintf(stderr, "%s:%d - idx[%d] workers.size[%zu] mTrnCliList.size[%zu]\n", __func__, __LINE__, idx, workers.size(), mTrnCliList.size());

        if(!connection_pending.at(idx)){
            TrnAttr &att_ref = trnc->getTrnAttr();
            TrnAttr *patt = &att_ref;

            LU_PEVENT(cfg->mlog(), "ERR TrnClient[%d] is DISCONNECTED trnc[%p] trn_attr[%p]\n", __func__, __LINE__, idx, trnc, (void *)patt);
            fprintf(stderr, "%s:%d - ERR TrnClient[%d]  is DISCONNECTED trnc[%p] trn_attr[%p]\n", __func__, __LINE__, idx, trnc, (void *)patt);

            // update disconnect stats
            if(cfg->stats().trn_cli_dis <= cfg->stats().trn_cli_con)
                cfg->stats().trn_cli_dis++;

            // start a worker thread for client reconnect
            std::cerr << __func__ << ":" << __LINE__ << " starting worker thread [" << idx << "]\n";

            // clean up resources from previous cycle
            if(con_promises.at(idx) != nullptr){
                delete con_promises.at(idx);
                con_promises.at(idx) = nullptr;
            }
            if(workers.at(idx) != nullptr){
                delete workers.at(idx);
                workers.at(idx) = nullptr;
            }

            // generate new shared context
            std::promise<bool> *con_promise = new std::promise<bool>;
            con_promises.at(idx) = con_promise;
            con_futures.at(idx) = con_promise->get_future();
            connection_pending.at(idx) = true;

            // start worker thread, add to list
//            std::thread *worker = new std::thread(&trnxpp_ctx::tcli_start_worker_fn, this, trnc,  std::move(*con_promise));
//            std::thread *worker = new std::thread(&trnxpp_ctx::tcli_start_worker_fn,  trnc,  std::move(*con_promise));
            std::thread *worker = new std::thread(&trnxpp_ctx::tcli_start_worker_fn,  trnc,  con_promise);

            workers.at(idx) = worker;

            // update connection state
            is_connected.at(idx) = false;
        } else {

            // check worker state
            std::cerr << __func__ << ":" << __LINE__ <<  " - worker thread [" << idx << "] pending\n";
            std::cerr << __func__ << ":" << __LINE__ <<  " - con_future[" << idx << "] valid[" << con_futures.at(idx).valid() << "]\n";

            bool ready = false;
            std::future_status con_status = con_futures.at(idx).wait_for(std::chrono::milliseconds(100));
            switch (con_status) {
                case std::future_status::deferred:
                    std::cerr << __func__ << ":" << __LINE__ <<  " - con_status[" << idx << "] DEFERRED\n";
                    break;
                case std::future_status::timeout:
                    std::cerr << __func__ << ":" << __LINE__ <<  " - con_status[" << idx << "] TIMEOUT\n";
                    break;
                case std::future_status::ready:
                    std::cerr << __func__ << ":" << __LINE__ <<  " - con_status[" << idx << "] READY!\n";
                    ready = true;
                    break;
                default:
                    std::cerr << __func__ << ":" << __LINE__ <<  " - con_status[" << idx << "] UNKNOWN!\n";
                    break;
            }

            std::cerr << __func__ << ":" << __LINE__ <<  " - worker thread [" << idx << "] con_status[" << (int)con_status << "] ready[" << (ready?'Y':'N') << "]\n";

            if(ready){

                // thread finished, update state
                is_connected.at(idx) = con_futures.at(idx).get();
                connection_pending.at(idx) = false;

                retval = is_connected.at(idx);

                std::cerr << __func__ << ":" << __LINE__ <<  " - joining worker [" << idx << "] is_connected[" << (is_connected.at(idx)?'Y':'N') << "]\n";
                workers.at(idx)->join();

                if(is_connected.at(idx)){

                    // update connect stats
                    cfg->stats().trn_cli_con++;

                    LU_PEVENT(cfg->mlog(), "INFO TrnClient is RECONNECTED [%p]\n", __func__, __LINE__, trnc);
                    fprintf(stderr, "%s:%d - INFO TrnClient is RECONNECTED [%p]\n", __func__, __LINE__, trnc);

                } else{
                    LU_PEVENT(cfg->mlog(), "ERR TrnClient reconnect failed [%p]\n", __func__, __LINE__, trnc);
                    fprintf(stderr, "%s:%d - ERR TrnClient reconnect failed [%p]\n", __func__, __LINE__, trnc);
                }

                // clean up thread resources
                if(workers.at(idx) != nullptr){
                    delete workers.at(idx);
                    workers.at(idx) = nullptr;
                }

                if(con_promises.at(idx) != nullptr){
                    delete con_promises.at(idx);
                    con_promises.at(idx) = nullptr;
                }
                connection_pending.at(idx) = false;


            } else {
                std::cerr << __func__ << ":" << __LINE__ <<  " - worker thread [" << idx << "] not ready, continuing\n";
            }
        }
        return retval;
    }

    int trncli_count()
    {
        return mTrnCliList.size();
    }

    void dump_trnhosts()
    {
        fprintf(stderr,"%s:%d - ctx[%p] mTrnCliList[%p]\n",__func__, __LINE__,this,&mTrnCliList);

        // publish to MB1 servers
        int i=0;
        std::list<trn::trn_host>::iterator it;

        for(it = mTrnCliList.begin(); it != mTrnCliList.end(); it++, i++){

            trn::trn_host thost = (*it);
            std::string key = std::get<0>(thost);

            TrnHostX uhost = std::get<5>(thost);
            void *vp = uhost.trnc_host;
            TrnClient *trncli = uhost.trnc_host;
            TrnAttr &att_ref = trncli->getTrnAttr();
            TrnAttr *patt = &att_ref;

            if(trncli != NULL) {

                fprintf(stderr, "%s:%d - TRNCLI[%d] key[%s] vp[%p] trncli[%p] trn_attr[%p]:\n", __func__, __LINE__, i, key.c_str(), vp, trncli, patt);
                fprintf(stderr, "%s:%d - TRNCLI[%d] SZ trncli[%zu] trn_attr[%zu]:\n", __func__, __LINE__, i, sizeof(*trncli), sizeof(*patt));

                fprintf(stderr, "%s:%d - TRNCLI[%d] show_addr:\n", __func__, __LINE__,i);
                trncli->show_addr();

                std::string att_str = att_ref.atostring();
                fprintf(stderr, "%s:%d - TRNCLI[%d] trn_attr addr:\n%s\n", __func__, __LINE__, i, att_str.c_str());

                att_str = att_ref.tostring();
                fprintf(stderr, "%s:%d - TRNCLI[%d] trn_attr members:\n%s\n", __func__, __LINE__, i, att_str.c_str());


                if(NULL != patt) {
                    fprintf(stderr, "%s:%d - TRNCLI[%d] terrainNavServer [%s]\n", __func__, __LINE__, i, patt->terrainNavServer);
                }else {
                    fprintf(stderr, "%s:%d - TRNCLI[%d]  !!! TRN_ATTR is NULL !!!\n", __func__, __LINE__, i);
                }
            } else {
                fprintf(stderr, "%s:%d - TRNCLI[%d]  is NULL\n", __func__, __LINE__, i);
            }
        }
        return;
    }

    int pub_trn(double nav_time, poseT *pt, measT *mt, int trn_type, std::list<lcm_pub> &pubs,  trnxpp_cfg *cfg)
    {
        int retval = -1;

        if(mTrnCliList.size() <= 0){
            return 0;
        }

        fprintf(stderr,"%s:%d - ctx[%p] mTrnCliList[%p]\n",__func__, __LINE__,this,&mTrnCliList);
        // publish to MB1 servers
        int i=0;
        std::list<trn::trn_host>::iterator it;

        for(it = mTrnCliList.begin(); it != mTrnCliList.end(); it++, i++) {

            trn::trn_host thost = (*it);
            std::string key = std::get<0>(thost);

            TrnHostX uhost = std::get<5>(thost);
            void *vp = uhost.trnc_host;
            TrnClient *trncli = uhost.trnc_host;
            TrnAttr &att_ref = trncli->getTrnAttr();
            TrnAttr *patt = &att_ref;

            TRN_NDPRINT(5, "%s:%d - TRNCLI[%d] key[%s] vp[%p] trncli[%p] ctx[%p]:\n", __func__, __LINE__, i, key.c_str(), vp, trncli, this);

            if(trncli == NULL){

                fprintf(stderr, "%s:%d - ERR TrnClient is NULL\n", __func__, __LINE__);

                // initialize TrnClient (sets TrnClient host in mTrnCliList)
                start_trncli(key, cfg, false, cfg->ginterrupt());


                // TODO: is it valid to re-read thost, or do we need to reiterate?
                // update variable from TrnClientList reference
                uhost = std::get<5>(thost);
                vp = uhost.trnc_host;

                if(NULL == vp){
                    LU_PEVENT(cfg->mlog(), "ERR start_trncli failed\n", __func__, __LINE__);
                    continue;
                }

                // update local reference
                trncli = uhost.trnc_host;

                // update stats
                cfg->stats().trn_cli_con++;

                LU_PEVENT(cfg->mlog(), "TrnClient started [%p]\n", __func__, __LINE__, (void *)trncli);
            }

            // check connection, restart as needed (in separate thread)
            bool is_connected = trncli_check_connection( i, trncli, cfg);

            if(!is_connected){
                fprintf(stderr, "%s:%d - TrnClient not connected; connecting/continuing\n", __func__, __LINE__);
                continue;
            }

            try{
                // update TRN
                trncli->motionUpdate(pt);
                cfg->stats().trn_motn_n++;
                TRN_NDPRINT(5, "%s:%d - motion update\n", __func__, __LINE__);

                trncli->measUpdate(mt, trn_type);
                cfg->stats().trn_meas_n++;
                TRN_NDPRINT(5, "%s:%d - meas update trn_type[%d]\n", __func__, __LINE__, trn_type);

            } catch(Exception e) {
                fprintf(stderr,"%s - caught exception in TRN update trn_type[%d] [%s]\n", __func__, trn_type, e.what());
                cfg->stats().trn_err_n++;
                continue;
            }

            // get TRN MMSE/MLE estimates
            poseT mle, mmse;
            trncli->estimatePose(&mmse, TRN_EST_MMSE);
            cfg->stats().trn_mmse_n++;

            trncli->estimatePose(&mle, TRN_EST_MLE);
            cfg->stats().trn_mle_n++;

            if(trncli->lastMeasSuccessful()){

                cfg->stats().trn_est_ok_n++;

                // log estimate
                std::string est_str = trnx_utils::trnest_tostring(nav_time, *pt, mle, mmse);
                LU_PEVENT(cfg->mlog(), "trn est:\n%s\n", est_str.c_str());

                // write TRN estimate CSV (compatible w/ tlp-plot)
                write_trnest_csv(nav_time, *pt, mle, mmse);

                if(cfg->verbose()){
                    // output estimate to console
                    fprintf(stderr, "%s\n", est_str.c_str());
                }

            } else {
                TRN_NDPRINT(3, "%s:%d - lastMeasSuccessful ERR\n",__func__,__LINE__);
            }

            // publish LCM outputs
            if(lcm_is_enabled(trn::LCM_TRN_MOTN)){

                TRN_NDPRINT(5, "%s:%d - PUB TRN_MOTN\n", __func__, __LINE__);

                pcf::lcm_publisher *pub = get_pub(pubs, "TRN_MOTN");

                if(pub != nullptr){
                    trn::trn_pose_t motn_msg;
                    trn_msg_utils::pose_to_lcm(motn_msg, *pt);
                    pub->publish(motn_msg);
                    cfg->stats().trn_pub_motn_n++;
                }
            }

            if(lcm_is_enabled(trn::LCM_TRN_MEAS)){

                TRN_NDPRINT(5, "%s:%d - PUB TRN_MEAS\n", __func__, __LINE__);

                pcf::lcm_publisher *pub = get_pub(pubs, "TRN_MEAS");

                if(pub != nullptr){
                    trn::trn_meas_t meas_msg;
                    trn::trn_msg_utils::meas_to_lcm(meas_msg, *mt);
                    pub->publish(meas_msg);
                    cfg->stats().trn_pub_meas_n++;
                }
            }

            if(lcm_is_enabled(trn::LCM_TRN_EST)){

                TRN_NDPRINT(5, "%s:%d - PUB TRN_EST\n", __func__, __LINE__);

                pcf::lcm_publisher *pub = get_pub(pubs, "TRN_EST");

                if(pub != nullptr){
                    trn::trn_pose_t mmse_msg;
                    trn::trn_msg_utils::pose_to_lcm(mmse_msg, mmse);
                    pub->publish(mmse_msg);
                    cfg->stats().trn_pub_est_n++;
                }
            }

            if(lcm_is_enabled(trn::LCM_TRN_STAT)){

                TRN_NDPRINT(5, "%s:%d - PUB TRN_STAT\n", __func__, __LINE__);

                pcf::lcm_publisher *pub = get_pub(pubs, "TRN_EST");

                if(pub != nullptr){
                    trn::trn_stat_t trnstat_msg;
                    trn::trn_msg_utils::trn_to_lcm(trnstat_msg, "TRNSVR", *pt, mmse, mle);
                    pub->publish(trnstat_msg);
                    cfg->stats().trn_pub_stat_n++;
                }
            }

        }

        return retval;
    }

    void parse_lcm_flags(const char *flags)
    {
        mLcmFlags = 0;
        if(strstr(flags,"trnmotn") != NULL)
            mLcmFlags |= LCM_TRN_MOTN;
        if(strstr(flags,"trnmeas") != NULL)
            mLcmFlags |= LCM_TRN_MEAS;
        if(strstr(flags,"trnest") != NULL)
            mLcmFlags |= LCM_TRN_EST;
        if(strstr(flags,"trnstat") != NULL)
            mLcmFlags |= LCM_TRN_STAT;
        if(strstr(flags,"mb1svr") != NULL)
            mLcmFlags |= LCM_MB1SVR;
        if(strstr(flags,"mbest") != NULL)
            mLcmFlags |= LCM_MBEST;
    }

    void set_lcm_flags(uint32_t mask)
    {
        mLcmFlags |= mask;
    }

    void clr_lcm_flags(uint32_t mask)
    {
        mLcmFlags &= (~mask);
    }

    uint32_t lcm_flags()
    {
        return mLcmFlags;
    }

    std::string lcm_flags_str()
    {
        std::ostringstream ss;
        if( (mLcmFlags & LCM_MB1SVR) != 0)
            ss << "mb1svr";
        if( (mLcmFlags & LCM_MBEST) != 0){
            if(ss.str().length() > 0)
                ss << "|";
            ss << "mbest";
        }
        if( (mLcmFlags & LCM_TRN_MEAS) != 0){
            if(ss.str().length() > 0)
                ss << "|";
            ss << "trnmeas";
        }
        if( (mLcmFlags & LCM_TRN_MOTN) != 0){
            if(ss.str().length() > 0)
                ss << "|";
            ss << "trnmotn";
        }
        if( (mLcmFlags & LCM_TRN_EST) != 0){
            if(ss.str().length() > 0)
                ss << "|";
            ss << "trnest";
        }
        if( (mLcmFlags & LCM_TRN_STAT) != 0){
            if(ss.str().length() > 0)
                ss << "|";
            ss << "trnstat";
        }

        return ss.str();
    }

    bool lcm_is_enabled(lcm_flag_t mask)
    {
        return ((mLcmFlags & mask) != 0 ? true : false);
    }

    // extra parameters (key/value pairs)
    // keys may contain [a-zA-Z0-9_-.]
    std::map<std::string, double> mDmap;
    std::map<std::string, uint64_t> mUmap;
    std::map<std::string, int64_t> mImap;

protected:
private:

    FILE *mMB1CsvFile;
    FILE *mMB1BinFile;
    FILE *mTrnEstCsvFile;
    FILE *mMBEstCsvFile;
    FILE *mRawBathCsvFile;
    long int mUtmZone;
    std::string mGeoCRS;
    GeoCon *mGeoCon;
    int mDecMod;
    int mCBCount;
    std::string mCtxKey;
    std::string mMB1CsvPath;
    std::string mMB1BinPath;
    std::string mTrnEstCsvPath;
    std::string mMBEstCsvPath;
    std::string mRawBathCsvPath;
    uint32_t mLcmFlags;

    std::vector<std::string> mBathInputKeys;
    std::vector<std::string> mVelInputKeys;
    std::vector<std::string> mNavInputKeys;
    std::vector<std::string> mAttInputKeys;
    std::vector<std::string> mCallbackKeys;
    std::vector<std::string> mMB1InputKeys;
    std::vector<std::string> mDepthInputKeys;

    std::list<trn::trn_host> mMB1SvrList;
    std::list<trn::trn_host> mUdpmSubList;
    std::list<trn::trn_host> mTrnCliList;
};
}
#endif
