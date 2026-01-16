
//////////////////////////////////////////////////////////////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute                 //
// Distributed under MIT license. See license.txt for more information.     //
//////////////////////////////////////////////////////////////////////////////
#ifndef LCM_TRN_CTX_H  // include guard
#define LCM_TRN_CTX_H

#include <thread>
#include <future>

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

//namespace trn
//{
// fwd declaration
class TrnXpp;

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
    trn::mb1_server *mb1s_host;

};

// channel, input ptr
using lcm_input = std::tuple<const std::string, trn::trn_lcm_input*>;
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
    // these index into Xpp::mCtx; change w/ care
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

class LcmTrnCtx
{
public:

    LcmTrnCtx();

    LcmTrnCtx(const LcmTrnCtx &other);

    ~LcmTrnCtx();


    void tostream(std::ostream &os, int wkey=15, int wval=18);


    std::string tostring(int wkey=15, int wval=18);

    void show(int wkey=15, int wval=18);

    void set_ctx_key(const char *key);

    std::string &ctx_key();

    void set_utm_zone(long int utm);

    long int utm_zone();

    void set_geo_crs(std::string crs);

    std::string geo_crs();

    GeoCon *geocon();

    void add_callback_key(const std::string &key);

    bool has_callback(const std::string &key);

    int decmod();

    void set_decmod(int n);

    int cbcount();

    void set_cbcount(int n);

    void inc_cbcount();

    void set_bath_input(int i, const std::string &inp);

    std::string *bath_input_chan(int i);

    void set_mb1_input(int i, const std::string &inp);

    std::string *mb1_input_chan(int i);

    void set_depth_input(int i, const std::string &inp);

    std::string *depth_input_chan(int i);

    void set_nav_input_chan(int i, const std::string &inp);

    std::string *nav_input_chan(int i);

    void set_att_input_chan(int i, const std::string &inp);

    std::string *att_input_chan(int i);

    void set_vel_input_chan(int i, const std::string &inp);

    pcf::lcm_publisher *get_pub(std::list<lcm_pub> &pubs, const std::string& channel);

    std::string *vel_input_chan(int i);
    ;

    std::string trnest_csv_path();

    int init_trnest_csv_file(trnxpp_cfg *cfg);

    FILE *trnest_csv_open();

    FILE *trnest_csv_file();

    int write_trnest_csv(double &stime, poseT &pt, poseT &mle, poseT &mmse);
/////////////////
    void set_trnest_csv_path(const std::string &inp);
    void set_mbest_csv_path(const std::string &inp);

    std::string mbest_csv_path();

    int init_mbest_csv_file(trnxpp_cfg *cfg);

    FILE *mbest_csv_open();

    FILE *mbest_csv_file();

    int write_mbest_csv(trnu_pub_t &mbest);

//////////////////
    void set_mb1_csv_path(const std::string &inp);

    std::string mb1_csv_path();

    int init_mb1_csv_file(trnxpp_cfg *cfg);

    FILE *mb1_csv_open();

    FILE *mb1_csv_file();

    int write_mbest_csv(double &stime, trnu_pub_t &mbest);

    int write_mb1_csv(mb1_t *snd, trn::bath_info *bi, trn::att_info *ai, trn::vel_info *vi=nullptr);

    int write_mb1_csv(mb1_t *snd, trn::mb1_info *bi, trn::att_info *ai, trn::vel_info *vi=nullptr);

    int write_csv_orig(trn::bath_info *bi, trn::att_info *ai, trn::nav_info *ni, trn::vel_info *vi=nullptr);

    void set_mb1_bin_path(const std::string &inp);

    std::string mb1_bin_path();

    int init_mb1_bin_file(trnxpp_cfg *cfg);

    FILE *mb1_bin_open();

    FILE *mb1_bin_file();

    size_t write_mb1_bin(mb1_t *snd);

    /////////////////
    void set_rawbath_csv_path(const std::string &inp);

    std::string rawbath_csv_path();

    int init_rawbath_csv_file(trnxpp_cfg *cfg);

    FILE *rawbath_csv_open();

    FILE *rawbath_csv_file();

    int write_rawbath_csv(trn::bath_info *bi, trn::nav_info *ni, trn::att_info *ai, trn::vel_info *vi, long int utm, double alt_depth=-1.);

    trn_host *lookup_udpm_host(std::string key);

    udpm_sub_t *lookup_udpm_sub(std::string key);

    int add_udpm_host(std::string key, trn_host *host);

    // returns number of bytes received or -1 on error
    int get_udpms_update(const std::string &key, trnxpp_cfg *cfg, byte *dest, size_t len);


    int start_udpmsub(const std::string &key, trnxpp_cfg *cfg);


    trn_host *lookup_mb1svr_host(std::string key);

    int add_mb1svr_host(std::string key, trn_host *host);

    int set_mb1svr_inst(std::string key, trn::mb1_server *inst);

    int start_mb1svr(const std::string &key, trnxpp_cfg *cfg);

    int mb1svr_count();

    int pub_mb1(mb1_t *sounding, std::list<lcm_pub> &pubs, trnxpp_cfg *cfg);


    trn_host *lookup_trncli_host(std::string key);

    int add_trn_host(std::string key, trn_host *host);

    int trncli_connect(const std::string &key, int retries, uint32_t delay_sec, bool *quit=NULL);

    int start_trncli(const std::string &key, trnxpp_cfg *cfg, bool force_reconnect, bool *user_int);

    int start_trn(trnxpp_cfg *cfg, bool *user_int);

    static void tcli_start_worker_fn(TrnClient *trncli, std::promise<bool> *con_promise);

    // check connection, re-connect as needed (in separate thread)
    // return true if connected;
    bool trncli_check_connection(int idx, TrnClient *trnc, trnxpp_cfg *cfg);

    int trncli_count();

    void dump_trnhosts();

    void dump_cheese();

    int pub_trn(double nav_time, poseT *pt, measT *mt, int trn_type, std::list<lcm_pub> &pubs,  trnxpp_cfg *cfg);

    void parse_lcm_flags(const char *flags);

    void set_lcm_flags(uint32_t mask);

    void clr_lcm_flags(uint32_t mask);

    uint32_t lcm_flags();

    std::string lcm_flags_str();

    bool lcm_is_enabled(lcm_flag_t mask);

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

    std::list<trn_host> mMB1SvrList;
    std::list<trn_host> mUdpmSubList;
    std::list<trn_host> mTrnCliList;
};
//}
#endif
