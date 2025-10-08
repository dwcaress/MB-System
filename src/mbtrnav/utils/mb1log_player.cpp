/// @file mb1log_player.cpp
/// @authors k. headley
/// @date 21mar2022

/// Summary: play back TrnBin.log to console and/or TRN server

// ///////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute
// Distributed under MIT license. See LICENSE file for more information.

// /////////////////
// Includes
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <string.h>
#include <memory.h>
#include <libgen.h>
#include <inttypes.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <list>
#include <chrono>

#include "structDefs.h"
#include "mb1_msg.h"
#include "TrnClient.h"
#include "trn_debug.hpp"
#include "flag_utils.hpp"
#include "NavUtils.h"

// /////////////////
// Macros
#define WIN_DECLSPEC
/// @def VERSION_HELPER
/// @brief version string helper.
#define VERSION_HELPER(s) #s
/// @def VERSION_STRING
/// @brief version string macro.
#define VERSION_STRING(s) VERSION_HELPER(s)

#define MB1LOG_PLAYER_NAME "trnxpp"
#ifndef MB1LOG_PLAYER_BUILD
/// @def MB1LOG_PLAYER_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMBTRN_BUILD=`date`
#define MB1LOG_PLAYER_BUILD "" VERSION_STRING(APP_BUILD)
#endif
#ifndef MB1LOG_PLAYER_VERSION
/// @def MB1LOG_PLAYER_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMBTRN_BUILD=`date`
#define MB1LOG_PLAYER_VERSION "" VERSION_STRING(MB1LOG_PLAYER_VER)
#endif

#define TRN_SERVER_PORT_DFL 27027

#ifndef DTR
#define DTR(x) ((x) * M_PI/180.)
#endif
#ifndef RTD
#define RTD(x) ((x) * 180./M_PI)
#endif

// /////////////////
// Types

typedef uint8_t byte;

// TRN stream formatter function
typedef void (*trnx_stream_fn)(std::ostream &os, poseT &pt, measT &mt);

// /////////////////
// Module variables
static int g_signal=0;
static bool g_interrupt=false;


// /////////////////
// Declarations

class MB1LogConfig
{
public:

    typedef enum{
        MOTN=0x1,
        MEAS=0x2,
        EST=0x4,
        TRNI_CSV=0x8,
        TRNO_CSV=0x10,
        MMSE=0x20,
        MLE=0x40,
        TRNI=0x3,
        ALL_CSV=0x18
    }OFlags;

   MB1LogConfig()
    : mDebug(0), mVerbose(false), mHost("localhost"), mTrnCfg(), mPort(TRN_SERVER_PORT_DFL), mServer(false), mTrnInCsvEn(false), mTrnOutCsvEn(false), mTrnInCsvPath(), mTrnOutCsvPath(), mTrnSensor(TRN_SENSOR_MB), mOFlags(), mUtmZone(10), mBeams(0), mStep(false), mSwath(0.), mSkipRecs(0), mLimitRecs(0), mTrniFormat(0)
    {

    }

    MB1LogConfig(const MB1LogConfig &other)
    : mDebug(other.mDebug), mVerbose(other.mVerbose), mHost(other.mHost), mTrnCfg(other.mTrnCfg), mPort(other.mPort), mServer(other.mServer),  mTrnInCsvEn(other.mTrnInCsvEn), mTrnOutCsvEn(other.mTrnOutCsvEn), mTrnInCsvPath(other.mTrnInCsvPath), mTrnOutCsvPath(other.mTrnOutCsvPath), mTrnSensor(other.mTrnSensor), mOFlags(other.mOFlags), mUtmZone(other.mUtmZone), mBeams(other.mBeams), mStep(other.mStep), mSwath(other.mSwath), mSkipRecs(other.mSkipRecs), mLimitRecs(other.mLimitRecs), mTrniFormat(other.mTrniFormat)
    {

    }

   ~MB1LogConfig()
    {
    }

    bool server(){return mServer;}
    bool trni_csv(){return mTrnInCsvEn;}
    bool trno_csv(){return mTrnOutCsvEn;}
    int trn_sensor(){return mTrnSensor;}
    std::string host(){return mHost;}
    std::string trn_cfg(){return mTrnCfg;}
    std::string trni_csv_path(){return mTrnInCsvPath;}
    std::string trno_csv_path(){return mTrnOutCsvPath;}
    int port(){return mPort;}
    bool oflag_set(MB1LogConfig::OFlags mask){return mOFlags.all_set(mask);}
    long utm_zone(){return mUtmZone;}
    uint32_t beams(){return mBeams;}
    bool step(){return mStep;}
    double swath(){return mSwath;}
    uint32_t skip_recs(){return mSkipRecs;}
    uint32_t lim_recs(){return mLimitRecs;}
    unsigned int trni_format(){return mTrniFormat;}

    void set_server(bool enable){mServer = enable;}
    void set_trni_csv(bool enable){mTrnInCsvEn = enable;}
    void set_trni_csv_path(const std::string &path){mTrnInCsvPath = std::string(path.c_str());}
    void set_trno_csv(bool enable){mTrnOutCsvEn = enable;}
    void set_trno_csv_path(const std::string &path){mTrnOutCsvPath = std::string(path.c_str());}
    void set_host(const std::string &host){mHost = std::string(host);}
    void set_port(int port){mPort = port;}
    void set_trn_sensor(int id){mTrnSensor = id;}
    void set_trn_cfg(const std::string &cfg){mTrnCfg = std::string(cfg);}
    void set_debug(int debug){mDebug = debug;}
    void set_verbose(bool verbose){mVerbose = verbose;}
    void set_oflags(uint32_t flags){mOFlags = flags;}
    void set_utm(long utmZone){mUtmZone = utmZone;}
    void set_beams(uint32_t beams){mBeams = beams;}
    void set_step(bool step){mStep = step;}
    void set_swath(double swath){mSwath = swath;}
    void set_skip_recs(uint32_t skip_recs){mSkipRecs = skip_recs;}
    void set_lim_recs(uint32_t lim){mLimitRecs = lim;}
    void set_trni_format(unsigned int fmt){mTrniFormat = fmt;}

    void tostream(std::ostream &os, int wkey=15, int wval=18)
    {
        os << std::setw(wkey) << "debug" << std::setw(wval) << mDebug << std::endl;
        os << std::setw(wkey) << "verbose" << std::setw(wval) << mVerbose << std::endl;
        os << std::setw(wkey) << "step" << std::setw(wval) << mStep << std::endl;
        os << std::setw(wkey) << "mHost" << std::setw(wval) << mHost.c_str() << std::endl;
        int alen = mTrnCfg.length();
        int wx = (alen >= wval ? alen + 1 : wval);
        os << std::setw(wkey) << "mTrnCfg" << std::setw(wx) << mTrnCfg.c_str() << std::endl;
        os << std::setw(wkey) << "mPort" << std::setw(wval) << mPort << std::endl;
        os << std::setw(wkey) << "mServer" << std::setw(wval) << mServer << std::endl;
        os << std::setw(wkey) << "mTrnInCsvEn" << std::setw(wval) << mTrnInCsvEn << std::endl;
        os << std::setw(wkey) << "mTrnOutCsvEn" << std::setw(wval) << mTrnOutCsvEn << std::endl;
        alen = mTrnInCsvPath.length();
        wx = (alen >= wval ? alen + 1 : wval);
        os << std::setw(wkey) << "mTrnInCsvPath" << std::setw(wx) << mTrnInCsvPath.c_str() << std::endl;
        alen = mTrnOutCsvPath.length();
        wx = (alen >= wval ? alen + 1 : wval);
        os << std::setw(wkey) << "mTrnOutCsvPath" << std::setw(wx) << mTrnOutCsvPath.c_str() << std::endl;
        os << std::setw(wkey) << "mTrnSensor" << std::setw(wval) << mTrnSensor << std::endl;
        os << std::setw(wkey) << "mUtmZone" << std::setw(wval) << mUtmZone << std::endl;
        os << std::setw(wkey) << "mBeams" << std::setw(wval) << mBeams << std::endl;
        os << std::setw(wkey) << "mSwath" << std::setw(wval) << mSwath << std::endl;
        os << std::setw(wkey) << "mOFlags" << std::hex << std::setw(wval) << (uint32_t)mOFlags.get() << std::endl;
        os << dec;
        os << std::setw(wkey) << "mSkipRecs" << std::setw(wval) << mSkipRecs << std::endl;
        os << std::setw(wkey) << "mLimitRecs" << std::setw(wval) << mLimitRecs << std::endl;
        os << std::setw(wkey) << "mTrniFormat" << std::setw(wval) << mTrniFormat << std::endl;
    }

    std::string tostring(int wkey=15, int wval=18)
    {
        ostringstream ss;
        tostream(ss, wkey, wval);
        return ss.str();
    }

    void show(int wkey=15, int wval=18)
    {
        tostream(std::cerr, wkey, wval);
    }

protected:
private:
    int mDebug;
    bool mVerbose;
    std::string mHost;
    std::string mTrnCfg;
    int mPort;
    bool mServer;
    bool mTrnInCsvEn;
    bool mTrnOutCsvEn;
    std::string mTrnInCsvPath;
    std::string mTrnOutCsvPath;
    int mTrnSensor;
    flag_var<uint32_t> mOFlags;
    long int mUtmZone;
    uint32_t mBeams;
    bool mStep;
    double mSwath;
    uint32_t mSkipRecs;
    uint32_t mLimitRecs;
    unsigned int mTrniFormat;
};

class MLPStats
{
public:
    MLPStats()
    :mFilesPlayed(0), mRecordsFound(0), mMtniRead(0), mMeaiRead(0), mMseoRead(0), mMleoRead(0), mMotnUpdate(0), mMeasUpdate(0), mEstMMSE(0), mEstMLE(0), mLastMeasSuccess(0), mTrniCsvWrite(0), mTrnoCsvWrite(0)
    {}

    void stat_tostream(ostream &os, int wkey=18, int wval=15)
    {
        os << std::setw(wkey) << "-- stats --\n";
        os << std::setw(wkey) << "mFilesPlayed" << std::setw(wkey) << mFilesPlayed << "\n";
        os << std::setw(wkey) << "mRecordsFound" << std::setw(wkey) << mRecordsFound << "\n";
        os << std::setw(wkey) << "mMtniRead" << std::setw(wkey) << mMtniRead << "\n";
        os << std::setw(wkey) << "mMeaiRead" << std::setw(wkey) << mMeaiRead << "\n";
        os << std::setw(wkey) << "mMseoRead" << std::setw(wkey) << mMseoRead << "\n";
        os << std::setw(wkey) << "mMleoRead" << std::setw(wkey) << mMleoRead << "\n";
        os << std::setw(wkey) << "mMotnUpdate" << std::setw(wkey) << mMotnUpdate << "\n";
        os << std::setw(wkey) << "mMeasUpdate" << std::setw(wkey) << mMeasUpdate << "\n";
        os << std::setw(wkey) << "mEstMMSE" << std::setw(wkey) << mEstMMSE << "\n";
        os << std::setw(wkey) << "mEstMLE" << std::setw(wkey) << mEstMLE << "\n";
        os << std::setw(wkey) << "mLastMeasSuccess" << std::setw(wkey) << mLastMeasSuccess << "\n";
        os << std::setw(wkey) << "mTrniCsvWrite" << std::setw(wkey) << mTrniCsvWrite << "\n";
    }

    std::string stat_tostring(int wkey=18, int wval=15)
    {
        ostringstream ss;
        stat_tostream(ss, wkey, wval);
        return ss.str();
    }

    void show_stats(int wkey=18, int wval=15)
    {
        stat_tostream(std::cerr, wkey, wval);
    }

    uint32_t mFilesPlayed;
    uint32_t mRecordsFound;
    uint32_t mMtniRead;
    uint32_t mMeaiRead;
    uint32_t mMseoRead;
    uint32_t mMleoRead;
    uint32_t mMotnUpdate;
    uint32_t mMeasUpdate;
    uint32_t mEstMMSE;
    uint32_t mEstMLE;
    uint32_t mLastMeasSuccess;
    uint32_t mTrniCsvWrite;
    uint32_t mTrnoCsvWrite;

};

class MB1LogPlayer
{
public:

    MB1LogPlayer()
    :mConfig(), mTrn(NULL), mFile(NULL), mTrnInCsvFile(NULL), mTrnOutCsvFile(NULL), mQuit(false)
    {
    }

    MB1LogPlayer(const MB1LogConfig &cfg)
    :mConfig(cfg), mTrn(NULL), mFile(NULL), mTrnInCsvFile(NULL), mTrnOutCsvFile(NULL), mQuit(false)
    {
    }

    ~MB1LogPlayer()
    {
        if(mFile != NULL){
            std::fclose(mFile);
        }

        if(mTrnInCsvFile != NULL){
            std::fclose(mTrnInCsvFile);
        }
        if(mTrnOutCsvFile != NULL){
            std::fclose(mTrnOutCsvFile);
        }

        if(NULL != mTrn)
            delete mTrn;

        TNavConfig::release();

    }

    int play(const std::string &src, bool *quit=NULL)
    {
        int retval = -1;
        static poseT *lastPT = NULL;
        static bool client_initialized = false;

        TRN_DPRINT("%s:%d - playing file [%s]\n", __func__, __LINE__, src.c_str());


        if(mConfig.server() && !client_initialized){
            if(init_client(quit) != 0){
                fprintf(stderr, "%s:%d - init_client failed\n", __func__, __LINE__);
                return retval;
            }
            if(trn_connect(10, 3) != 0){
                fprintf(stderr, "%s:%d - trn_connect failed\n", __func__, __LINE__);
                return retval;
            }
            client_initialized = true;
        }


        if(mFile != NULL){
            std::fclose(mFile);
        }

        if( (mFile = std::fopen(src.c_str(), "r")) == NULL)
        {
            fprintf(stderr, "%s:%d - could not open file[%s] [%d:%s]\n", __func__, __LINE__, src.c_str(), errno, strerror(errno));
            return retval;
        }

        byte ibuf[MB1_MAX_SOUNDING_BYTES]={0};

        uint32_t skip_records = 0;
        uint32_t lim_records = 0;

        while (!mQuit && next_record(ibuf, MB1_MAX_SOUNDING_BYTES) == 0) {

            if(mConfig.skip_recs() > 0 && skip_records++ < mConfig.skip_recs())
                continue;


            this->stats().mRecordsFound++;

            if(mConfig.lim_recs() > 0 && lim_records++ >= mConfig.lim_recs())
                break;

            if(NULL!=quit && *quit)
                break;

//            fprintf(stderr, "%s:%d - read MB1 record [%d]\n", __func__, __LINE__, this->stats().mRecordsFound);

            poseT *pt = NULL;
            measT *mt = NULL;

            if(read_pose(&pt, ibuf) == 0 && pt != NULL){

                this->stats().mMtniRead++;

                if(mConfig.oflag_set(MB1LogConfig::MOTN))
                {
                    show_pt(*pt);
                    std::cerr << "\n";
                }

                if(mConfig.server() && mTrn != NULL)
                {
                    try{
                        mTrn->motionUpdate(pt);
                        this->stats().mMotnUpdate++;
                    }catch(Exception e) {
                        fprintf(stderr,"%s - caught exception [%s]\n",__func__, e.what());
                    }
                }
                if(lastPT != NULL)
                    delete lastPT;
                lastPT = new poseT();
                *lastPT = *pt;
            } else {
                TRN_NDPRINT(2,"read_pose failed\n");
                if(lastPT != NULL)
                    delete lastPT;
                lastPT = NULL;

            }

            if(read_meas(&mt, ibuf, mConfig.trn_sensor()) == 0 && mt != NULL){

               this->stats().mMeaiRead++;

                if(mConfig.oflag_set(MB1LogConfig::MEAS))
                {
                    show_mt(*mt);
                    std::cerr << "\n";
                }


                if(lastPT != NULL && mConfig.trni_csv()){
                    trni_csv_tofile(&mTrnInCsvFile, *lastPT, *mt);
                    this->stats().mTrniCsvWrite++;
                }

                if(lastPT != NULL && mConfig.oflag_set(MB1LogConfig::TRNI_CSV))
                {
                    trni_csv_tostream(std::cout, *lastPT, *mt);
                }

                if(mConfig.server())
                {
                    try{

                        mTrn->measUpdate(mt, mConfig.trn_sensor());
                        this->stats().mMeasUpdate++;

                        if(mTrn->lastMeasSuccessful()){
                            this->stats().mLastMeasSuccess++;

                            auto ts_now = std::chrono::system_clock::now();
                            std::chrono::duration<double> epoch_time = ts_now.time_since_epoch();
                            poseT mle, mmse;
                            double ts = epoch_time.count();

                            mTrn->estimatePose(&mmse, TRN_EST_MMSE);
                            this->stats().mEstMMSE++;
                            mTrn->estimatePose(&mle, TRN_EST_MLE);
                            this->stats().mEstMLE++;

                            if(lastPT != NULL && mConfig.oflag_set(MB1LogConfig::EST)){
                                fprintf(stderr, "%s:%d --- EST --- \n",__func__, __LINE__);
                                show_est(ts, *lastPT, mle, mmse);
                            }

                            if(lastPT != NULL && mConfig.trno_csv()){
                                trno_csv_tofile(&mTrnOutCsvFile, ts, *lastPT, mle, mmse);
                                this->stats().mTrnoCsvWrite++;
                            }
                            if( lastPT != NULL && mConfig.oflag_set(MB1LogConfig::TRNO_CSV))
                            {
                                trno_csv_tostream(std::cout, ts, *lastPT, mle, mmse);
                            }
                        }else{
                            TRN_NDPRINT(1, "%s:%d - last meas unsuccessful\n",__func__, __LINE__);
                        }

                    }catch(Exception e) {
                        fprintf(stderr,"%s - caught exception [%s]\n",__func__, e.what());
                    }
                }
                if(lastPT != NULL)
                    delete lastPT;
                lastPT = NULL;
            } else {
                TRN_NDPRINT(2, "read_meas failed\n");
            }

            if(mt != NULL)
                delete mt;
            if(pt != NULL)
                delete pt;

            memset(ibuf, 0, MB1_MAX_SOUNDING_BYTES);

            if(mConfig.step()) {
                char c = '\0';
                cin.get(c);
                if (c == 'q') mQuit = true;
            }
        }
        return retval;
    }

    void set_server(bool enable){mConfig.set_server(enable);}
    void quit(){
        TRN_DPRINT("setting player quit flag\n");
        mQuit = true;
    }

    MLPStats &stats(){return mStats;}

    void show_cfg()
    {
        mConfig.show();
    }

protected:

    static void trni_csv_tostream_rock(std::ostream &os, poseT &pt, measT &mt)
    {
        // Note that TRN uses N,E,D frame (i.e. N:x E:y D:z)
        // no ping number
        // pitch,roll =0
        // v* 0.1

        // [ 0] time POSIX epoch sec
        // [ 2] northings
        // [ 3] eastings
        // [ 4] depth
        // [ 5] heading
        // [ 6] pitch
        // [ 7] roll
        // [ 8] flag (0)
        // [ 9] flag (0)
        // [10] flag (0)
        // [11] vx (0)
        // [12] xy (0)
        // [13] vz (0)
        // [14] sounding valid flag
        // [15] bottom lock valid flag
        // [16] number of beams
        // beam[i] number
        // beam[i] valid (1)
        // beam[i] range
        // ...
        // NEWLINE

        os << std::dec << std::setfill(' ') << std::fixed << std::setprecision(7);
        os << pt.time << ",";
        os << pt.x << ",";
        os << pt.y << ",";
        os << pt.z << ",";
        os << pt.psi << ",";
        os << 0 << ",";
        os << 0 << ",";
        os << 0 << ",";
        os << 0 << ",";
        os << 0 << ",";
        os << (pt.vx == 0 ? 0.1 : pt.vx) << ",";
        os << (pt.vy == 0 ? 0.1 : pt.vy) << ",";
        os << (pt.vz == 0 ? 0.1 : pt.vz) << ",";
        os << std::setprecision(1);
        os << (pt.dvlValid?1:0) << ",";
        os << (pt.bottomLock?1:0) << ",";
        os << mt.numMeas << ",";
        os << std::setprecision(6);
        for(int i=0; i< mt.numMeas; i++)
        {
            os << mt.beamNums[i] << ",";
            os << mt.measStatus[i] << ",";
            os << mt.ranges[i] << ",";
            os << mt.alongTrack[i] << ",";
            os << mt.crossTrack[i] << ",";
            os << mt.altitudes[i];
            if(i != mt.numMeas-1)
                os << ",";

        }
        os << "\n";
    }
    static void trni_csv_tostream_default(std::ostream &os, poseT &pt, measT &mt)
    {
        // Note that TRN uses N,E,D frame (i.e. N:x E:y D:z)
        // [ 0] time POSIX epoch sec
        // [ 1] ping_number
        // [ 2] northings
        // [ 3] eastings
        // [ 4] depth
        // [ 5] heading
        // [ 6] pitch
        // [ 7] roll
        // [ 8] flag (0)
        // [ 9] flag (0)
        // [10] flag (0)
        // [11] vx (0)
        // [12] xy (0)
        // [13] vz (0)
        // [14] sounding valid flag
        // [15] bottom lock valid flag
        // [16] number of beams
        // beam[i] number
        // beam[i] valid (1)
        // beam[i] range
        // ...
        // NEWLINE

        os << std::dec << std::setfill(' ') << std::fixed << std::setprecision(7);
        os << pt.time << ",";
        os << mt.ping_number << ",";
        os << pt.x << ",";
        os << pt.y << ",";
        os << pt.z << ",";
        os << pt.psi << ",";
        os << pt.theta << ",";
        os << pt.phi << ",";
        os << 0 << ",";
        os << 0 << ",";
        os << 0 << ",";
        os << pt.vx << ",";
        os << pt.vy << ",";
        os << pt.vz << ",";
        os << std::setprecision(1);
        os << (pt.dvlValid?1:0) << ",";
        os << (pt.bottomLock?1:0) << ",";
        os << mt.numMeas << ",";
        os << std::setprecision(6);
        for(int i=0; i< mt.numMeas; i++)
        {
            os << mt.beamNums[i] << ",";
            os << mt.measStatus[i] << ",";
            os << mt.ranges[i] << ",";
            os << mt.alongTrack[i] << ",";
            os << mt.crossTrack[i] << ",";
            os << mt.altitudes[i];
            if(i != mt.numMeas-1)
                os << ",";

        }
        os << "\n";
    }

    void trni_csv_tostream(std::ostream &os, poseT &pt, measT &mt) 
    {
        int format = mConfig.trni_format() % mTrniFormatCount;
        mTrniFormatList[format](os, pt, mt);
    }

    std::string trni_csv_tostring(poseT &pt, measT &mt)
    {
        ostringstream ss;

        trni_csv_tostream(ss, pt, mt);
        return ss.str();
    }

    void trni_csv_tofile(FILE **fp, poseT &pt, measT &mt)
    {

        if(NULL == *fp)
        {
            TRN_DPRINT("%s:%d INFO - opening trni_csv file[%s]\n", __func__, __LINE__, mConfig.trni_csv_path().c_str());

            *fp = std::fopen(mConfig.trni_csv_path().c_str(), "a");
        }

        if(NULL != *fp){
            // if file open, generate and write CSV output
            std::string csv = trni_csv_tostring(pt, mt);
            fwrite(csv.c_str(),csv.length(),1,*fp);
        } else {
            TRN_DPRINT("%s:%d ERR - could not open trni_csv file[%s] [%d:%s]\n", __func__, __LINE__, mConfig.trni_csv_path().c_str(), errno, strerror(errno));
        }
    }

    void show_trni_csv(poseT &pt, measT &mt)
    {
        trni_csv_tostream(std::cerr, pt, mt);
    }

    void trno_csv_tostream(std::ostream &os, double ts, poseT &pt, poseT &mle, poseT &mmse)
    {
        // Format (compatible with tlp-plot)
        // session-time,
        // mmse.time,
        // mmse.x,
        // mmse.y,
        // mmse.z,
        // pos.time,
        // ofs.x,
        // ofs.y,
        // ofs.z,
        // cov.0,
        // cov.2,
        // cov.5,
        // pos.time,
        // pos.x,
        // pos.y,
        // pos.z,
        // mle.time,
        // mle.x,
        // mle.y,
        // mle.z

        os << std::fixed << std::setprecision(3);
        os << ts << ",";

        // mmse
        os << mmse.time << ",";
        os << std::setprecision(4);
        os << mmse.x << "," << mmse.y << "," << mmse.z << ",";

        // ofs
        os << std::fixed << std::setprecision(3);
        os << pt.time << ",";
        os << std::setprecision(4);
        os << mmse.x - pt.x << "," << mmse.y - pt.y << "," << mmse.z - pt.z << ",";

        // cov
        os << std::fixed << std::setprecision(3);
        os << (mmse.covariance[0]) << ",";
        os << (mmse.covariance[2]) << ",";
        os << (mmse.covariance[5]) << ",";

        // pos
        os << std::setprecision(3);
        os << pt.time << ",";
        os << std::setprecision(4);
        os << pt.x << "," << pt.y << "," << pt.z << ",";

        // mle
        os << std::fixed << std::setprecision(3);
        os << mle.time << ",";
        os << std::setprecision(4);
        os << mle.x << "," << mle.y << "," << mle.z << "\n";

    }

    std::string trno_csv_tostring(double ts, poseT &pt, poseT &mle, poseT &mmse)
    {
        ostringstream ss;
        trno_csv_tostream(ss, ts, pt, mle, mmse);
        return ss.str();
    }

    void trno_csv_tofile(FILE **fp, double ts, poseT &pt, poseT &mle, poseT &mmse)
    {
        if(NULL == *fp)
        {
            *fp = std::fopen(mConfig.trno_csv_path().c_str(),"a");
        }

        if(NULL != *fp){
            std::string csv = trno_csv_tostring(ts, pt, mle, mmse);
            fwrite(csv.c_str(),csv.length(),1,*fp);
        } else {
            TRN_DPRINT("ERR - could not open file[%s]\n",mConfig.trno_csv_path().c_str());
        }
    }

    void show_trno_csv(double ts, poseT &pt, poseT &mle, poseT &mmse)
    {
        trno_csv_tostream(std::cerr, ts, pt, mle, mmse);
    }

    void est_tostream(std::ostream &os, double ts, poseT &pt, poseT &mle, poseT &mmse, int wkey=15, int wval=18)
    {
        os << "--- TRN Estimate OK---" << "\n";
        os << "MMSE[t, tm, x, y, z] ";
        os << std::fixed << std::setprecision(3);
        os << ts << ", ";
        os << std::setprecision(2);
        os << mmse.time << ", ";
        os << std::setprecision(4);
        os << mmse.x << ", " << mmse.y << ", " << mmse.z << "\n";

        os << "OFS[t, tm, x, y, z]  ";
        os << std::fixed << std::setprecision(3);
        os << ts << ", ";
        os << std::setprecision(2);
        os << mmse.time << ", ";
        os << std::setprecision(4);
        os << mmse.x - pt.x << "," << mmse.y - pt.y << "," << mmse.z - pt.z << "\n";

        os << "COV[t, x, y, z, m]   ";
        os << std::setprecision(3);
        os << mmse.time << ", ";
        os << std::setprecision(2);
        os << mmse.covariance[0] << ", ";
        os << mmse.covariance[2] << ", ";
        os << mmse.covariance[5] << ", ";
        double ss = mmse.covariance[0] * mmse.covariance[0];
        ss += mmse.covariance[2] * mmse.covariance[2];
        ss += mmse.covariance[5] * mmse.covariance[5];
        os << sqrt(ss) << "\n";

        os << "s[t, x, y, z]        ";
        os << std::setprecision(3);
        os << mmse.time << ", ";
        os << std::setprecision(2);
        os << sqrt(mmse.covariance[0]) << ", ";
        os << sqrt(mmse.covariance[2]) << ", ";
        os << sqrt(mmse.covariance[5]) << "\n";

        os << "POS[t, tm, x, y, z]  ";
        os << std::fixed << std::setprecision(3);
        os << ts << ", ";
        os << std::setprecision(2);
        os << pt.time << ", ";
        os << std::setprecision(4);
        os << pt.x << ", " << pt.y << ", " << pt.z << "\n";

        os << "MLE[t, tm, x, y, z]  ";
        os << std::fixed << std::setprecision(3);
        os << ts << ", ";
        os << std::setprecision(2);
        os << mle.time << ", ";
        os << std::setprecision(4);
        os << mle.x << ", " << mle.y << ", " << mle.z << "\n";
    }


    std::string est_tostring(double ts, poseT &pt, poseT &mle, poseT &mmse, int wkey=15, int wval=18)
    {
        std::ostringstream ss;
        est_tostream(ss, ts, pt, mle, mmse, wkey, wval);
        return ss.str();
    }

    void show_est(double ts, poseT &pt, poseT &mle, poseT &mmse, int wkey=15, int wval=18)
    {
        est_tostream(std::cerr, ts, pt, mle, mmse, wkey, wval);
        std::cerr << "\n";
    }

    void pt_tostream(poseT &pt, std::ostream &os, int wkey=15, int wval=18)
    {
        os << "-- poseT --\n";
        os << std::fixed << std::setprecision(3);
        os << std::setw(wkey) <<  "time" << std::setw(wval) << pt.time << "\n";
        os << std::setw(wkey) <<  "x" << std::setw(wval) << pt.x << "\n";
        os << std::setw(wkey) <<  "y" << std::setw(wval) << pt.y << "\n";
        os << std::setw(wkey) <<  "z" << std::setw(wval) << pt.z << "\n";
        os << std::setw(wkey) <<  "vx" << std::setw(wval) << pt.vx << "\n";
        os << std::setw(wkey) <<  "vy" << std::setw(wval) << pt.vy << "\n";
        os << std::setw(wkey) <<  "vz" << std::setw(wval) << pt.vz << "\n";
        os << std::setw(wkey) <<  "phi" << std::setw(wval) << pt.phi << "\n";
        os << std::setw(wkey) <<  "theta" << std::setw(wval) << pt.theta << "\n";
        os << std::setw(wkey) <<  "psi" << std::setw(wval) << pt.psi << "\n";
        os << std::setw(wkey) <<  "dvlValid" << std::setw(wval) << (pt.dvlValid?'Y':'N') << "\n";
        os << std::setw(wkey) <<  "gpsValid" << std::setw(wval) << (pt.gpsValid?'Y':'N') << "\n";
        os << std::setw(wkey) <<  "bottomLock" << std::setw(wval) << (pt.bottomLock?'Y':'N') << "\n";
    }

    std::string pt_tostring(poseT &pt, int wkey=15, int wval=18)
    {
        std::ostringstream ss;
        pt_tostream(pt, ss, wkey, wval);
        return ss.str();
    }

    void show_pt(poseT &pt, int wkey=15, int wval=18)
    {
        pt_tostream(pt, std::cerr, wkey, wval);
    }

    void mt_tostream(measT &mt, std::ostream &os, int wkey=15, int wval=18)
    {
        os << "-- measT --\n";
        os << std::fixed << std::setprecision(3);
        os << std::setw(wkey) <<  "time" << std::setw(wval) << mt.time << "\n";
        os << std::setw(wkey) <<  "dataType" << std::setw(wval) << mt.dataType << "\n";
        os << std::setw(wkey) <<  "ping_number" << std::setw(wval) << mt.ping_number << "\n";
        os << std::setw(wkey) <<  "x" << std::setw(wval) << mt.x << "\n";
        os << std::setw(wkey) <<  "y" << std::setw(wval) << mt.y << "\n";
        os << std::setw(wkey) <<  "z" << std::setw(wval) << mt.z << "\n";
        os << std::setw(wkey) <<  "phi" << std::setw(wval) << mt.phi << "\n";
        os << std::setw(wkey) <<  "theta" << std::setw(wval) << mt.theta << "\n";
        os << std::setw(wkey) <<  "psi" << std::setw(wval) << mt.psi << "\n";
        os << std::setw(wkey) <<  "num_meas" << std::setw(wval) << mt.numMeas << "\n";
        os << std::setw(wkey) <<  "beams" << std::setw(wval) << "[stat, range]" << "\n";
        os << std::setprecision(2) ;
        for(int i=0; i<mt.numMeas; i++){
            os << std::setw(wkey-4) <<  "[" << std::setw(3) << mt.beamNums[i] << "]";
            os << std::setw(wval-9) << "[" << (mt.measStatus[i] ? 1 : 0) << ", ";
            os << std::fixed << std::setfill(' ') << std::setprecision(2) << std::setw(7) << mt.ranges[i] << ", ";
            os << std::setw(7) << mt.crossTrack[i] << ", ";
            os << std::setw(7) << mt.alongTrack[i] << ", ";
            os << std::setw(7) << mt.altitudes[i] << "]\n";
        }
    }

    std::string mt_tostring(measT &mt, int wkey=15, int wval=18)
    {
        std::ostringstream ss;
        mt_tostream(mt, ss, wkey, wval);
        return ss.str();
    }

    void show_mt(measT &mt, int wkey=15, int wval=18)
    {
        mt_tostream(mt, std::cerr, wkey, wval);
    }

    void esto_tostream(poseT &pt, std::ostream &os, int wkey=15, int wval=18)
    {
        os << "-- poseT [est] --\n";
        os << std::fixed << std::setprecision(3);
        os << std::setw(wkey) <<  "time" << std::setw(wval) << pt.time << "\n";
        os << std::setw(wkey) <<  "x" << std::setw(wval) << pt.x << "\n";
        os << std::setw(wkey) <<  "y" << std::setw(wval) << pt.y << "\n";
        os << std::setw(wkey) <<  "z" << std::setw(wval) << pt.z << "\n";
        os << std::setw(wkey) <<  "vx" << std::setw(wval) << pt.vx << "\n";
        os << std::setw(wkey) <<  "vy" << std::setw(wval) << pt.vy << "\n";
        os << std::setw(wkey) <<  "vz" << std::setw(wval) << pt.vz << "\n";
        os << std::setw(wkey) <<  "vw_x" << std::setw(wval) << pt.vw_x << "\n";
        os << std::setw(wkey) <<  "vw_y" << std::setw(wval) << pt.vw_y << "\n";
        os << std::setw(wkey) <<  "vw_z" << std::setw(wval) << pt.vw_z << "\n";
        os << std::setw(wkey) <<  "vn_x" << std::setw(wval) << pt.vn_x << "\n";
        os << std::setw(wkey) <<  "vn_y" << std::setw(wval) << pt.vn_y << "\n";
        os << std::setw(wkey) <<  "vn_z" << std::setw(wval) << pt.vn_z << "\n";
        os << std::setw(wkey) <<  "wx" << std::setw(wval) << pt.wx << "\n";
        os << std::setw(wkey) <<  "wy" << std::setw(wval) << pt.wy << "\n";
        os << std::setw(wkey) <<  "wz" << std::setw(wval) << pt.wz << "\n";
        os << std::setw(wkey) <<  "ax" << std::setw(wval) << pt.ax << "\n";
        os << std::setw(wkey) <<  "ay" << std::setw(wval) << pt.ay << "\n";
        os << std::setw(wkey) <<  "az" << std::setw(wval) << pt.az << "\n";
        os << std::setw(wkey) <<  "phi" << std::setw(wval) << pt.phi << "\n";
        os << std::setw(wkey) <<  "theta" << std::setw(wval) << pt.theta << "\n";
        os << std::setw(wkey) <<  "psi" << std::setw(wval) << pt.psi << "\n";
        os << std::setw(wkey) <<  "psi_berg" << std::setw(wval) << pt.psi_berg << "\n";
        os << std::setw(wkey) <<  "psi_dot_berg" << std::setw(wval) << pt.psi_dot_berg << "\n";
        os << std::setw(wkey) <<  "dvlValid" << std::setw(wval) << (pt.dvlValid?'Y':'N') << "\n";
        os << std::setw(wkey) <<  "gpsValid" << std::setw(wval) << (pt.gpsValid?'Y':'N') << "\n";
        os << std::setw(wkey) <<  "bottomLock" << std::setw(wval) << (pt.bottomLock?'Y':'N') << "\n";
        for(int i=0;i<N_COVAR;i++)
            os << std::setw(wkey-4) <<  "cov[" << std::setw(2) << i << "]" << std::setw(wval) << pt.covariance[i] << "\n";
    }

    std::string esto_tostring(poseT &pt, int wkey=15, int wval=18)
    {
        std::ostringstream ss;
        esto_tostream(pt, ss, wkey, wval);
        return ss.str();
    }

    void show_esto(poseT &pt, int wkey=15, int wval=18)
    {
        esto_tostream(pt, std::cerr, wkey, wval);
    }

    int trn_connect(int retries, uint32_t delay_sec)
    {
        int retval = -1;
        int rem = retries;
        if(mTrn != nullptr){
            do{
                TerrainNav *mTerrainNavRef = mTrn->connectTRN();
                if(mTerrainNavRef!=NULL && mTrn->is_connected()){
                    retval=0;
                    break;
                }
                if(mTrn->isQuitSet())
                    break;
                if(delay_sec>0)
                    sleep(delay_sec);
            }while( (retries > 0 ? --rem > 0 : true) );
        }
        return retval;
    }

    int init_client(bool *quit)
    {
        int retval = -1;

        if(NULL == mTrn){
            mTrn = new TrnClient(mConfig.host().c_str(), mConfig.port());
            if(NULL != mTrn)
            {
                mTrn->setQuitRef(quit);
                mTrn->loadCfgAttributes(mConfig.trn_cfg().c_str());
                retval = 0;
            }

        }

        return retval;
    }

    // finds and reads next record ID
    int next_record(byte *dest, size_t len)
    {
        int retval = -1;

        typedef enum{
            START,
            MB1,
            OK, EEOF, ERR
        }state_t;
        state_t stat = START;

        size_t msg_buf_len = MB1_MAX_SOUNDING_BYTES + sizeof(mb1_t);
        byte msg_buf[msg_buf_len];

        while(stat != OK && stat != EEOF && stat != ERR)
        {
            memset(msg_buf, 0, msg_buf_len);
            mb1_t *mb1 = (mb1_t *) &msg_buf[0];
            byte *ptype = (byte *)(&(mb1->type));

            bool ferr=false;
            int64_t rbytes=0;
            bool sync_valid=false;

            byte *sp = (byte *)mb1;
            bool header_valid=false;
            bool rec_valid=false;
            bool data_valid = true;

            while (!sync_valid) {
                if( ((rbytes = std::fread((void *)sp, 1, 1, mFile)) == 1) && *sp=='M') {
                    sp++;
                    if( ((rbytes = std::fread((void *)sp, 1, 1, mFile)) == 1) && *sp=='B'){
                        sp++;
                        if( ((rbytes = std::fread((void *)sp, 1, 1, mFile)) == 1) && *sp=='1'){
                            sp++;
                            if( ((rbytes = std::fread((void *)sp, 1, 1, mFile)) == 1) && *sp=='\0'){
                                sync_valid=true;
                                TRN_NDPRINT(2, "sync read slen[%d]\n", MB1_TYPE_BYTES);
                                TRN_NDPRINT(2, "  sync     ['%c''%c''%c''%c']/[%02X %02X %02X %02X]\n",
                                          ptype[0],
                                          ptype[1],
                                          ptype[2],
                                          ptype[3],
                                          ptype[0],
                                          ptype[1],
                                          ptype[2],
                                          ptype[3]);
                                break;
                            }else{
                                sp=ptype;
                            }
                        }else{
                            sp=ptype;
                        }
                    }else{
                        sp=ptype;
                    }
                }
                if(rbytes<=0){
                    TRN_NDPRINT(1, "reached EOF looking for sync buf[%s] rbytes[%ld] err[%d:%s] fpos[%ld] feof[%d] ferr[%d]\n", msg_buf, rbytes, errno, strerror(errno), ftell(mFile), std::feof(mFile), std::ferror(mFile));
                    ferr = true;
                    break;
                }
            }

            if(g_interrupt)
                ferr = true;

            if (sync_valid && !ferr) {

                // read the rest of the sounding header
                byte *psnd = (byte *)&mb1->size;
                uint32_t readlen =(MB1_HEADER_BYTES-MB1_TYPE_BYTES);
                if((rbytes = std::fread((void *)psnd, 1, readlen, mFile)) == readlen){

                    int32_t cmplen = MB1_SOUNDING_BYTES(mb1->nbeams);

                    if ((int32_t)mb1->size == cmplen ) {
                        header_valid=true;
                        TRN_NDPRINT(2, "sounding header read len[%" PRIu32 "/%" PRId64 "]\n", (uint32_t)readlen, rbytes);
                        TRN_NDPRINT(3, "  size   [%d]\n", mb1->size);
                        TRN_NDPRINT(3, "  time   [%.3f]\n", mb1->ts);
                        TRN_NDPRINT(3, "  lat    [%.3f]\n", mb1->lat);
                        TRN_NDPRINT(3, "  lon    [%.3f]\n", mb1->lon);
                        TRN_NDPRINT(3, "  depth  [%.3f]\n", mb1->depth);
                        TRN_NDPRINT(3, "  hdg    [%.3f]\n", mb1->hdg);
                        TRN_NDPRINT(3, "  ping   [%06d]\n", mb1->ping_number);
                        TRN_NDPRINT(3, "  nbeams [%d]\n", mb1->nbeams);
                    } else {
                        TRN_DPRINT( "message len invalid l[%d] l*[%d]\n", mb1->size, cmplen);
                    }

                } else {
                    fprintf(stderr, "could not read header bytes [%" PRId64 "/%" PRIu32 "] [%d:%s]\n", rbytes, readlen, errno, strerror(errno));
                    ferr=true;
                }
            }

            if(g_interrupt)
                ferr = true;

            bool dflags[3] = {true,true,true};
            if (header_valid && ferr == false ) {

                if(mb1->nbeams > 0){
                    // read beam data
                    byte *bp = (byte *)mb1->beams;
                    uint32_t readlen = MB1_BEAM_ARRAY_BYTES(mb1->nbeams);

                    if((rbytes = std::fread((void *)bp, 1, readlen, mFile)) == readlen){

                        TRN_NDPRINT(2, "beams read blen[%d/%" PRId64 "]\n", readlen, rbytes);

                    } else {
                        TRN_NDPRINT(2, "beam read failed pb[%p] read[%" PRId64 "] [%d:%s]\n", bp, rbytes,  errno, strerror(errno));
                    }

                } else {
                    TRN_NDPRINT(2, "no beams read [%" PRIu32 "]\n", mb1->nbeams);
                }

                byte *cp = (byte *)MB1_PCHECKSUM(mb1);
                if((rbytes = std::fread((void *)cp, 1, MB1_CHECKSUM_BYTES, mFile)) == MB1_CHECKSUM_BYTES){
                    //                                    TRN_NDPRINT(2, "chksum read clen[%" PRId64 "]\n", rbytes);
                    //                                    TRN_NDPRINT(3, "  chksum [%0X]\n", pmessage->chksum);

                    if(mb1->nbeams <= 0 || mb1->nbeams > MB1_MAX_BEAMS)
                    {
                        fprintf(stderr, "%s:%d ERR nbeams %d (ping %07d)\n", __func__, __LINE__, mb1->nbeams, mb1->ping_number);
                        data_valid = false;
                        dflags[0] = false;
                    } else if(mb1->ts <= 0)
                    {
                        fprintf(stderr, "%s:%d ERR time %.3lf (ping %07d)\n", __func__, __LINE__, mb1->ts, mb1->ping_number);
                        data_valid = false;
                        dflags[1] = false;
                    } else if ((mb1->lat > -1. && mb1->lat < 1.)  || (mb1->lon > -1. && mb1->lon < 1.) || (mb1->depth > -1. && mb1->depth < 1.)) {
                        fprintf(stderr, "%s:%d ERR lat,lon,depth [%.3lf, %.3lf, %.3lf] (ping %07d)\n", __func__, __LINE__, mb1->lat, mb1->lon, mb1->depth, mb1->ping_number);
                        data_valid = false;
                        dflags[2] = false;
                    }else {
                        rec_valid=true;
                    }

                }else{
                    TRN_DPRINT( "chksum read failed [%" PRId64 "] [%d:%s]\n", rbytes, errno, strerror(errno));
                }

            }else{
                TRN_DPRINT( "header read failed [%" PRId64 "]\n", rbytes);
            }

            if(g_interrupt)
                ferr = true;


            if (rec_valid && ferr == false) {
                // TODO : update stats?
                stat = OK;
            } else {
                if(std::feof(mFile)){
                    stat = EEOF;
                    TRN_NDPRINT(2, "end of data file\n");
                } else if(!data_valid) {
                    //fprintf(stderr,"%s:%d - ERR data invalid beams: %s time: %s lat/lon/depth: %s\n",__func__, __LINE__,  (dflags[0]?"OK":"ERR"), (dflags[1]?"OK":"ERR"), (dflags[2]?"OK":"ERR"));
                    stat = OK;
                } else {
                    stat = ERR;
                    fprintf(stderr,"%s:%d - ERR read failed [%d:%s]\n",__func__, __LINE__, errno, strerror(errno));
                }
            }
        }

        if(stat == OK)
        {
            TRN_NDPRINT(2,"%s:%d - stat OK %p\n", __func__, __LINE__, dest);
            mb1_t *snd = (mb1_t *)msg_buf;
            memcpy(dest, msg_buf, MB1_SOUNDING_BYTES(snd->nbeams));
            retval = 0;
        }

        return retval;
    }

    static double vnorm( double v[] )
    {
        double vnorm2 = 0.0;
        int i=0;
        for(i=0; i<3; i++) vnorm2 += pow(v[i],2.0);
        return( sqrt( vnorm2 ) );
    }

    // read mb1 to measT
    int read_meas(measT **pdest, byte *src, int data_type)
    {
        int retval = -1;

        mb1_t *snd = (mb1_t *)src;
        int src_beams = snd->nbeams;
        int dest_beams = ((mConfig.beams() > 0) ? mConfig.beams() : src_beams);

        measT *dest = new measT(dest_beams, data_type);

        if(NULL != dest){

            TRN_NDPRINT(2, "%s:%d dest_beams[%lu] src_beams[%d]\n", __func__, __LINE__, dest_beams, src_beams);

            dest->time = snd->ts;
            dest->dataType = data_type;

            double pos_N=0., pos_E=0.;
            double lat=snd->lat, lon=snd->lon;

            NavUtils::geoToUtm( Math::degToRad(lat),
                               Math::degToRad(lon),
                               mConfig.utm_zone(), &pos_N, &pos_E);
            dest->x = pos_N;
            dest->y = pos_E;
            dest->z = snd->depth;
            dest->ping_number = snd->ping_number;

            double swath_lim = mConfig.swath()/2.;
            int mod = 1;
            if(mConfig.beams() > 0){
                if(src_beams > dest_beams){
                    if(mConfig.swath() > 0.) {
                        mod = mConfig.swath() / dest_beams;
                    } else {
                        mod = src_beams / dest_beams;
                    }
                } // else use all beams (mod = 1)
            }

            if(mod <= 0)
                mod = 1;

            int j = 0;
            for(int i=0; i < snd->nbeams; i++)
            {
                bool use_beam = false;

                int bx=0;
                double wb=0.;
                if(snd->beams[i].rhoy != 0. && snd->beams[i].rhoz != 0.) {

                    bx = snd->beams[i].beam_num;

                    if ((bx % mod) == 0) {

                        wb = RTD(atan2(snd->beams[i].rhoy, snd->beams[i].rhoz));

                        if(mConfig.swath() <= 0. || fabs(wb) <= swath_lim){
                            use_beam = true;
                        }
                    }
                }


                double rho[3] = {snd->beams[i].rhox, snd->beams[i].rhoy, snd->beams[i].rhoz};
                double range = vnorm(rho);

//                fprintf(stderr, "%s:%d beam[%3d] x,y,z,r,w[%+8.3lf, %+8.3lf, %+8.3lf, %+8.3lf, %+8.3lf] swath_max[%+8.3lf] %c\n", __func__, __LINE__, bx, snd->beams[i].rhox, snd->beams[i].rhoy, snd->beams[i].rhoz, range, wb, swath_lim, (use_beam?'Y':'N'));

                if (range > 0. && use_beam) {
                    dest->beamNums[j] = bx;
                    dest->alongTrack[j] = snd->beams[i].rhox;
                    dest->crossTrack[j] = snd->beams[i].rhoy;
                    dest->altitudes[j]  = snd->beams[i].rhoz;
                    dest->ranges[j] = range;
                    dest->measStatus[j] = true;
                    j++;
                }

                if(j >= dest_beams)
                    break;
            }

            *pdest = dest;
            retval = 0;
        } // else error

        return retval;
    }

    // read MB1 to poseT
    int read_pose(poseT **pdest, byte *src)
    {
        int retval = -1;

        poseT *dest = new poseT();
        if(NULL != dest)
        {
            mb1_t *snd =  (mb1_t *)src;

            dest->time = snd->ts;
            NavUtils::geoToUtm( Math::degToRad(snd->lat),
                               Math::degToRad(snd->lon),
                               mConfig.utm_zone(), &(dest->x), &(dest->y));
            dest->z = snd->depth;
            // MB1 doesn't contain vx, vy, vz
            // set vx !=0 to enable TRN motion initialization
            dest->vx = 0.1;
            dest->vy = 0.;
            dest->vz = 0.;
            dest->wx = 0.;
            dest->wy = 0.;
            dest->wz = 0.;
            dest->phi = 0.;
            dest->theta = 0.;
            dest->psi = snd->hdg;
            dest->gpsValid = (snd->depth < 2 ? true : false);
            dest->dvlValid = true;
            dest->bottomLock = true;
            *pdest = dest;
            retval = 0;
        }

        return retval;
    }

private:
    MB1LogConfig mConfig;
    TrnClient *mTrn;
    FILE *mFile;
    FILE *mTrnInCsvFile;
    FILE *mTrnOutCsvFile;
    bool mQuit;
    MLPStats mStats;
    static int mTrniFormatCount;
    static trnx_stream_fn mTrniFormatList[];

};

// MB1LogPlayer static initializers
int MB1LogPlayer::mTrniFormatCount=2;

trnx_stream_fn MB1LogPlayer::mTrniFormatList[] = {
    MB1LogPlayer::trni_csv_tostream_default,
    MB1LogPlayer::trni_csv_tostream_rock
};


// application config
// configures a MB1LogConfig instance used to
// initialize the MB1LogPlayer instance
class app_cfg
{
public:
    app_cfg()
    : mDebug(0), mVerbose(false), mAppCfg(), mSessionStr(), mInputList(), mTBConfig(), mConfigSet(false)
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
    ~app_cfg(){}

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
            {"version", no_argument, NULL, 0},
            {"cfg", required_argument, NULL, 0},
            {"input", required_argument, NULL, 0},
            {"trn-host", required_argument, NULL, 0},
            {"trn-cfg", required_argument, NULL, 0},
            {"trn-sensor", required_argument, NULL, 0},
            {"trni-csv", required_argument, NULL, 0},
            {"trno-csv", required_argument, NULL, 0},
            {"utm", required_argument, NULL, 0},
            {"show", required_argument, NULL, 0},
            {"server", no_argument, NULL, 0},
            {"noserver", no_argument, NULL, 0},
            {"logdir", required_argument, NULL, 0},
            {"beams", required_argument, NULL, 0},
            {"step", no_argument, NULL, 0},
            {"swath", required_argument, NULL, 0},
            {"skip-recs", required_argument, NULL, 0},
            {"lim-recs", required_argument, NULL, 0},
            {"trni-fmt", required_argument, NULL, 0},
            {NULL, 0, NULL, 0}
        };
        // reset optind
        optind=1;

        // process argument list
        while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1){
            char *acpy = nullptr;
            char *host_str = nullptr;
            char *port_str = nullptr;
            TRN_NDPRINT(1,"++++ PARSING OPTION [%s / %s]\n",options[option_index].name, optarg);

            switch (c) {
                    // long options all return c=0
                case 0:
                    // verbose
                    if (strcmp("verbose", options[option_index].name) == 0) {
                        mVerbose=true;
                        mTBConfig.set_verbose(true);
                    }
                    // debug
                    else if (strcmp("debug", options[option_index].name) == 0) {
                        if(sscanf(optarg,"%d",&mDebug) == 1)
                            mTBConfig.set_debug(mDebug);
                        TRN_TRACE();
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
                            mConfigSet=true;
                            break;
                        }
                    } else {
                        // host
                        if (strcmp("trn-host", options[option_index].name) == 0) {
                            acpy = strdup(optarg);
                            host_str = strtok(acpy,":");
                            port_str = strtok(NULL,":");
                            if(NULL!=host_str){
                                mTBConfig.set_host(std::string(host_str));
                            }
                            if(NULL != port_str){
                                int port;
                                if(sscanf(port_str,"%d",&port) == 1)
                                    mTBConfig.set_port(port);
                            }
                            free(acpy);

                            mTBConfig.set_server(true);
                        }
                        // trn-sensor
                        else if (strcmp("trn-sensor", options[option_index].name) == 0) {
                            int sensor=0;
                            if(sscanf(optarg,"%d", &sensor) == 1)
                                mTBConfig.set_trn_sensor(sensor);
                        }
                        // trn-cfg
                        else if (strcmp("trn-cfg", options[option_index].name) == 0) {
                            mTBConfig.set_trn_cfg(std::string(optarg));
                        }
                        // utm
                        else if (strcmp("utm", options[option_index].name) == 0) {
                            long int utm=0;
                            if(sscanf(optarg,"%ld", &utm) == 1)
                                mTBConfig.set_utm(utm);
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
                        // show
                        else if (strcmp("show", options[option_index].name) == 0) {
                            uint32_t oflags=0;
                            if(strstr(optarg,"trni") != NULL){
                                oflags |= MB1LogConfig::TRNI;
                            }
                            if(strstr(optarg,"trno") != NULL){
                                oflags |= MB1LogConfig::EST;
                            }
                            if(strstr(optarg,"est") != NULL){
                                oflags |= MB1LogConfig::EST;
                            }
                            if(strstr(optarg,"mmse") != NULL){
                                oflags |= MB1LogConfig::MMSE;
                            }
                            if(strstr(optarg,"mle") != NULL){
                                oflags |= MB1LogConfig::MLE;
                            }
                            if(strstr(optarg,"motn") != NULL){
                                oflags |= MB1LogConfig::MOTN;
                            }
                            if(strstr(optarg,"meas") != NULL){
                                oflags |= MB1LogConfig::MEAS;
                            }
                            if(strstr(optarg,"icsv") != NULL){
                                oflags |= MB1LogConfig::TRNI_CSV;
                            }
                            if(strstr(optarg,"ocsv") != NULL){
                                oflags |= MB1LogConfig::TRNO_CSV;
                            }
                            if(strstr(optarg,"*csv") != NULL){
                                oflags |= MB1LogConfig::ALL_CSV;
                           }
                            if(oflags>0){
                                mTBConfig.set_oflags(oflags);
                            }
                        }
                        // server
                        else if (strcmp("server", options[option_index].name) == 0) {
                            mTBConfig.set_server(true);
                        }
                        // noserver
                        else if (strcmp("noserver", options[option_index].name) == 0) {
                            mTBConfig.set_server(false);
                        }
                        // trni-csv
                        else if (strcmp("trni-csv", options[option_index].name) == 0) {
                            mTBConfig.set_trni_csv(true);
                            mTBConfig.set_trni_csv_path(std::string(optarg));
                        }
                        // trno-csv
                        else if (strcmp("trno-csv", options[option_index].name) == 0) {
                            mTBConfig.set_trno_csv(true);
                            mTBConfig.set_trno_csv_path(std::string(optarg));
                        }
                        // beams
                        else if (strcmp("beams", options[option_index].name) == 0) {
                            uint32_t beams=0;
                            if(sscanf(optarg,"%" PRIu32 "", &beams) == 1)
                                mTBConfig.set_beams(beams);
                        }
                        // step
                        else if (strcmp("step", options[option_index].name) == 0) {
                            mTBConfig.set_step(true);
                        }
                        // swath
                        else if (strcmp("swath", options[option_index].name) == 0) {
                            double swath_deg =0.;
                            if(sscanf(optarg, "%lf", &swath_deg) == 1)
                            mTBConfig.set_swath(swath_deg);
                        }
                        // skip-recs
                        else if (strcmp("skip-recs", options[option_index].name) == 0) {
                            uint32_t u32val =0;
                            if(sscanf(optarg, "%" PRIu32 "", &u32val) == 1)
                            mTBConfig.set_skip_recs(u32val);
                        }
                        else if (strcmp("lim-recs", options[option_index].name) == 0) {
                            uint32_t u32val =0;
                            if(sscanf(optarg, "%" PRIu32 "", &u32val) == 1)
                            mTBConfig.set_lim_recs(u32val);
                        }
                        else if (strcmp("trni-fmt", options[option_index].name) == 0) {
                            uint32_t u32val = 0;
                            if(sscanf(optarg, "%" PRIu32 "", &u32val) == 1)
                            mTBConfig.set_trni_format(u32val);
                        }
                    }
                    break;
                default:
                    help=true;
                    break;
            }

            if (version) {
                fprintf(stderr, "%s: version %s build %s\n", MB1LOG_PLAYER_NAME, MB1LOG_PLAYER_VERSION, MB1LOG_PLAYER_BUILD);
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
        char help_message[] = "\n TRN Log Player\n";
        char usage_message[] = "\n use: mb1log_player [options]\n"
        "\n"
        " Options\n"
        " --verbose              : verbose output\n"
        " --debug=d              : debug output\n"
        " --help                 : output help message\n"
        " --cfg=s                : app config file\n"
        " --version              : output version info\n"
        " --trn-host=addr[:port] : send output to TRN server\n"
        " --trn-cfg=s            : TRN config file\n"
        " --trn-sensor=n         : TRN sensor type\n"
        " --utm=n                : UTM zone\n"
        " --beams=n              : number of output beams\n"
        " --swath=f              : limit beams to center swath degrees\n"
        " --input=s              : specify input file path (may be used multiple times)\n"
        " --show=s               : specify console outputs\n"
        "                           trni     : TRN inputs (motion/poseT, meas/measT)\n"
        "                           trno|est : TRN outputs             (pose, mmse, ofs, cov, mle)\n"
        "                           motn     : TRN motion updates      (poseT)\n"
        "                           meas     : TRN measurement updates (measT)\n"
        "                           icsv     : TRN input csv           (motion/poseT, meas/measT)\n"
        "                           ocsv     : TRN output csv          (pose, mmse, ofs, cov, mle)\n"
        "                           *csv     : TRN input and output csv\n"
        " --trni-csv=s           : write TRN inputs to CSV file\n"
        " --trni-fmt=d           : TRN input CSV format\n"
        "                          0: default\n"
        "                          1: no ping number, pitch,roll=0\n"
        " --trno-csv=s           : write TRN outputs (estimates) to CSV file\n"
        " --server               : enable output to server\n"
        " --noserver             : disable output to server\n"
        " --step                 : step through entries\n"
        " --skip-recs            : skip records\n"
        " --lim-recs             : number of records to process\n"
        " Notes:\n"
        "  [1] beams option\n"
        "      unset : beams_out = input source beams\n"
        "      <= 0  : beams_out = input source beams\n"
        "       > 0  : beams_out = specified number of beams\n"
        "              modulus   = INT(max(src_beams / beams_out, 1))\n"
        "\n"
        "  [2] swath option\n"
        "      unset : no swath mask applied"
        "      >= 0  : mask beams outside of swath/2 either side of center beam\n"
        "              use modulus max(swath/beams_out, 1)\n"
        "\n"
        " Examples:\n"
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

        if(NULL != src && strlen(src) > 0 ){
            char *obuf = NULL;
            size_t wlen = strlen(src) + 1;
            char *wp = (char *)malloc(wlen);
            char *sp = wp;
            memset(wp, 0, wlen);
            snprintf(wp, wlen, "%s", src);
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
                    for(unsigned int i=1;i<var_len;i++){
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

            while (std::getline(file, line)) {
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

                            char dummy[]={'f','o','o','\0'};
                            char *cmdv[2]={dummy,cmd_buf};
                            TRN_NDPRINT(4,">>> cmd_buf[%s] cmdv[%p]\n",cmd_buf,&cmdv[0]);
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
                    lcp = NULL;
                }
            }
            file.close();
        } else {
            fprintf(stderr, "ERR - file open failed [%s] [%d/%s]", file_path.c_str(), errno, strerror(errno));
        }
    }

    MB1LogConfig &tb_config()
    {
        return mTBConfig;
    }

    void show_tb_config()
    {
        mTBConfig.show();
    }

    std::string cfg(){return mAppCfg;}
    std::string session_string(){return mSessionStr;}
    std::list<std::string>::iterator input_first(){ return mInputList.begin();}
    std::list<std::string>::iterator input_last(){ return mInputList.end();}
    int debug(){return mDebug;}
    bool verbose(){return mVerbose;}
    bool config_set(){return mConfigSet;}
    void set_config_set(){mConfigSet =  true;}
protected:
private:
    int mDebug;
    bool mVerbose;
    std::string mAppCfg;
    std::string mSessionStr;
    std::list<std::string> mInputList;
    MB1LogConfig mTBConfig;
    bool mConfigSet;
};

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

int main(int argc, char **argv)
{
    struct sigaction saStruct;
    sigemptyset(&saStruct.sa_mask);
    saStruct.sa_flags = 0;
    saStruct.sa_handler = s_termination_handler;
    sigaction(SIGINT, &saStruct, NULL);

    // get configuration from command line, file
    app_cfg cfg;

    setenv("TLP_SESSION",cfg.session_string().c_str(), false);

    // parse command line (first pass for config file)
    cfg.parse_args(argc, argv);

    // configure debug output (for parsing debug)
    trn_debug::get()->set_debug(cfg.debug());
    trn_debug::get()->set_verbose(cfg.verbose());

    if(cfg.config_set() > 0){
        // parse config file
        cfg.parse_file(cfg.cfg());
    } else {
        cfg.set_config_set();
    }
    // reparse command line (should override config options)
    cfg.parse_args(argc, argv);

    // configure debug output
    trn_debug::get()->set_debug(cfg.debug());
    trn_debug::get()->set_verbose(cfg.verbose());

    TRN_NDPRINT(1,"session [%s]\n",cfg.session_string().c_str());
    TRN_NDPRINT(1,"session env[%s]\n",getenv("TLP_SESSION"));

    cfg.show_tb_config();

    // get log player
    MB1LogPlayer tbplayer(cfg.tb_config());

    if(cfg.verbose()){
        std::cerr << "App Player Config:" << std::endl;
        cfg.show_tb_config();
        std::cerr << std::endl;

        std::cerr << "Player Config:" << std::endl;
        tbplayer.show_cfg();
        std::cerr << std::endl;
    }

    // play back log files
    std::list<string>::iterator it;
    for(it=cfg.input_first(); it!=cfg.input_last(); it++)
    {
        std::string input = *it;

        TRN_NDPRINT(1,"playing[%s]\n",input.c_str());
        tbplayer.play(input, &g_interrupt);
        tbplayer.stats().mFilesPlayed++;

        if(g_interrupt){
            // stop for SIGINT (CTRL-C)
            tbplayer.quit();
            break;
        }
    }
    tbplayer.stats().show_stats();

    // release trn_debug resources;
    trn_debug::get(true);

    TRN_DPRINT("%s:%d done\n",__func__, __LINE__);
    return 0;
}
