/// @file trnlog_player.cpp
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
#include <sstream>
#include <fstream>
#include <iostream>
#include <list>
#include <chrono>

#include "structDefs.h"
#include "TrnLog.h"
#include "TrnClient.h"
#include "trn_debug.hpp"
#include "flag_utils.hpp"

// /////////////////
// Macros
#define WIN_DECLSPEC
/// @def VERSION_HELPER
/// @brief version string helper.
#define VERSION_HELPER(s) #s
/// @def VERSION_STRING
/// @brief version string macro.
#define VERSION_STRING(s) VERSION_HELPER(s)

#define TRNLOG_PLAYER_NAME "trnxpp"
#ifndef TRNLOG_PLAYER_BUILD
/// @def TRNLOG_PLAYER_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMBTRN_BUILD=`date`
#define TRNLOG_PLAYER_BUILD "" VERSION_STRING(APP_BUILD)
#endif
#ifndef TRNLOG_PLAYER_VERSION
/// @def TRNLOG_PLAYER_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMBTRN_BUILD=`date`
#define TRNLOG_PLAYER_VERSION "" VERSION_STRING(TRNLOG_PLAYER_VER)
#endif



#define TRN_SERVER_PORT_DFL 27027
#define IBUF_LEN_BYTES 4800

#ifndef DTR
#define DTR(x) ((x) * M_PI/180.)
#endif
#ifndef RTD
#define RTD(x) ((x) * 180./M_PI)
#endif

// /////////////////
// Types

typedef uint8_t byte;

// /////////////////
// Module variables
static int g_signal=0;
static bool g_interrupt=false;

// /////////////////
// Declarations

class TrnLogConfig
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

   TrnLogConfig()
    : mDebug(0), mVerbose(false), mHost("localhost"), mTrnCfg(), mPort(TRN_SERVER_PORT_DFL), mServer(false), mTrnInCsvEn(false), mTrnOutCsvEn(false), mTrnInCsvPath(), mTrnOutCsvPath(), mTrnSensor(TRN_SENSOR_MB), mOFlags() //mConsole(true),
    {

    }

    TrnLogConfig(const TrnLogConfig &other)
    : mDebug(other.mDebug), mVerbose(other.mVerbose), mHost(other.mHost), mTrnCfg(other.mTrnCfg), mPort(other.mPort), mServer(other.mServer),  mTrnInCsvEn(other.mTrnInCsvEn), mTrnOutCsvEn(other.mTrnOutCsvEn), mTrnInCsvPath(other.mTrnInCsvPath), mTrnOutCsvPath(other.mTrnOutCsvPath), mTrnSensor(other.mTrnSensor), mOFlags(other.mOFlags) //mConsole(other.mConsole),
    {

    }

   ~TrnLogConfig()
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
    bool oflag_set(TrnLogConfig::OFlags mask){return mOFlags.all_set(mask);}

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

    void tostream(std::ostream &os, int wkey=15, int wval=18)
    {
        os << std::setw(wkey) << "debug" << std::setw(wval) << mDebug << std::endl;
        os << std::setw(wkey) << "verbose" << std::setw(wval) << mVerbose << std::endl;
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
        os << std::setw(wkey) << "mOFlags" << std::hex << std::setw(wval) << (uint32_t)mOFlags.get() << std::endl;
        os << dec;
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

};

class TLPStats
{
public:
    TLPStats()
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

class TrnLogPlayer
{
public:

    TrnLogPlayer()
    :mConfig(), mTrn(NULL), mFile(NULL), mTrnInCsvFile(NULL), mTrnOutCsvFile(NULL), mQuit(false)
    {
    }

    TrnLogPlayer(const TrnLogConfig &cfg)
    :mConfig(cfg), mTrn(NULL), mFile(NULL), mTrnInCsvFile(NULL), mTrnOutCsvFile(NULL), mQuit(false)
    {
    }

    ~TrnLogPlayer()
    {
        if(mFile != NULL){
            fclose(mFile);
        }

        if(mTrnInCsvFile != NULL){
            fclose(mTrnInCsvFile);
        }
        if(mTrnOutCsvFile != NULL){
            fclose(mTrnOutCsvFile);
        }

        if(NULL != mTrn)
            delete mTrn;

        TNavConfig::release();

    }

    int play(const std::string &src, bool *quit=NULL)
    {
        int retval = -1;
        static poseT lastPT;
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
            fclose(mFile);
        }

        if( (mFile = fopen(src.c_str(), "r")) == NULL)
        {
            fprintf(stderr, "%s:%d - could not open file[%s] [%d:%s]\n", __func__, __LINE__, src.c_str(), errno, strerror(errno));
            return retval;
        }

        if(skip_header() != 0)
        {
            fprintf(stderr, "%s:%d - data start not found in file[%s] [%d:%s]\n", __func__, __LINE__, src.c_str(), errno, strerror(errno));
            return retval;
        }

        TrnLog::TrnRecID rec_type = TrnLog::RT_INVALID;
        byte ibuf[IBUF_LEN_BYTES]={0};

        while (!mQuit && next_record(ibuf, IBUF_LEN_BYTES, rec_type) == 0) {
            this->stats().mRecordsFound++;

            if(NULL!=quit && *quit)
                break;

            if(rec_type == TrnLog::MOTN_IN) {

                poseT *pt = NULL;
                if(read_pose(&pt, ibuf) == 0 && pt != NULL){

                    this->stats().mMtniRead++;

                    if(mConfig.oflag_set(TrnLogConfig::MOTN))
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
                    lastPT = *pt;
                    delete pt;
                } else {
                    TRN_NDPRINT(2,"read_pose failed\n");
                }
            } else if(rec_type == TrnLog::MEAS_IN) {

                measT *mt = NULL;
                if(read_meas(&mt, ibuf) == 0 && mt != NULL){

                    this->stats().mMeaiRead++;

                    if(mConfig.oflag_set(TrnLogConfig::MEAS))
                    {
                        show_mt(*mt);
                        std::cerr << "\n";
                    }


                    if(mConfig.server())
                    {
                        try{
                            if(mConfig.trni_csv()){
                                trni_csv_tofile(&mTrnInCsvFile, lastPT, *mt);
                                this->stats().mTrniCsvWrite++;
                            }
                            if(mConfig.oflag_set(TrnLogConfig::TRNI_CSV))
                            {
                                show_trni_csv(lastPT, *mt);
                            }

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

                                if(mConfig.oflag_set(TrnLogConfig::EST)){
                                    show_est(ts, lastPT, mle, mmse);
                                }

                                if(mConfig.trno_csv()){
                                    trno_csv_tofile(&mTrnOutCsvFile, ts, lastPT, mle, mmse);
                                    this->stats().mTrnoCsvWrite++;
                                }
                                if(mConfig.oflag_set(TrnLogConfig::TRNO_CSV))
                                {
                                    show_trno_csv(ts, lastPT, mle, mmse);
                                }
                            }else{
                                fprintf(stderr,"%s:%d - last meas unsuccessful\n",__func__, __LINE__);
                            }

                        }catch(Exception e) {
                            fprintf(stderr,"%s - caught exception [%s]\n",__func__, e.what());
                        }
                    }
                    delete mt;
                } else {
                    TRN_NDPRINT(2,"read_meas failed\n");
                }
            } else if(rec_type == TrnLog::MSE_OUT) {

                poseT *pt = NULL;
                if(read_est(&pt, ibuf) == 0 && pt != NULL){

                    this->stats().mMseoRead++;

                    if(mConfig.oflag_set(TrnLogConfig::MMSE))
                        show_esto(*pt);
                }else {
                    TRN_NDPRINT(2,"read_est failed\n");
                }
                delete pt;
            } else if(rec_type == TrnLog::MLE_OUT) {
                
                poseT *pt = NULL;
                if(read_est(&pt, ibuf) == 0 && pt != NULL){
                    this->stats().mMleoRead++;
                    if(mConfig.oflag_set(TrnLogConfig::MLE))
                        show_esto(*pt);
                }else {
                    TRN_NDPRINT(2,"read_est failed\n");
                }
                delete pt;
            }else {
                TRN_NDPRINT(2,"invalid record type[%d]\n",rec_type);
            }

            memset(ibuf, 0, IBUF_LEN_BYTES);
        }
        return retval;
    }

    void set_server(bool enable){mConfig.set_server(enable);}
    void quit(){
        TRN_DPRINT("setting player quit flag\n");
        mQuit = true;
    }

    TLPStats &stats(){return mStats;}

    void show_cfg()
    {
        mConfig.show();
    }

protected:

    void trni_csv_tostream(std::ostream &os, poseT &pt, measT &mt)
    {
        // Note that TRN uses N,E,D frame (i.e. N:x E:y D:z)
        // [ 0] time POSIX epoch sec
        // [ 1] northings
        // [ 2] eastings
        // [ 3] depth
        // [ 4] heading
        // [ 5] pitch
        // [ 6] roll
        // [ 7] flag (0)
        // [ 8] flag (0)
        // [ 9] flag (0)
        // [10] vx (0)
        // [11] xy (0)
        // [12] vz (0)
        // [13] sounding valid flag
        // [14] bottom lock valid flag
        // [15] number of beams
        // beam[i] number
        // beam[i] valid (1)
        // beam[i] range
        // ...
        // NEWLINE

        os << std::dec << std::setfill(' ') << std::fixed << std::setprecision(7);
        os << pt.time << ",";
        os << std::setprecision(7);
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
        os << std::setprecision(4);
        for(int i=0; i< mt.numMeas; i++)
        {
            os << mt.beamNums[i] << ",";
            os << mt.measStatus[i] << ",";
            os << mt.ranges[i];
            if(i != mt.numMeas-1)
                os << ",";

        }
        os << "\n";
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

            *fp = fopen(mConfig.trni_csv_path().c_str(), "a");
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
        os << pt.x-mmse.x << "," << pt.y-mmse.y << "," << pt.z-mmse.z << ",";

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
            *fp = fopen(mConfig.trno_csv_path().c_str(),"a");
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
        std::cerr << "\n";
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
        os << pt.x-mmse.x << ", " << pt.y-mmse.y << ", " << pt.z-mmse.z << "\n";

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
        os << std::setw(wkey) <<  "x" << std::setw(wval) << mt.x << "\n";
        os << std::setw(wkey) <<  "y" << std::setw(wval) << mt.y << "\n";
        os << std::setw(wkey) <<  "z" << std::setw(wval) << mt.z << "\n";
        os << std::setw(wkey) <<  "ping_number" << std::setw(wval) << mt.ping_number << "\n";
        os << std::setw(wkey) <<  "num_meas" << std::setw(wval) << mt.numMeas << "\n";
        os << std::setw(wkey) <<  "beams" << std::setw(wval) << "[stat, range]" << "\n";
        os << std::setprecision(2) ;
        for(int i=0; i<mt.numMeas; i++){
            os << std::setw(wkey-4) <<  "[" << std::setw(3) << mt.beamNums[i] << "]";
            os << std::setw(wval-9) << "[" << (mt.measStatus[i] ? 1 : 0) << ", ";
            os << std::fixed << std::setprecision(2) << setw(6) << mt.ranges[i] << ", ";
            os << mt.crossTrack[i] << ", ";
            os << mt.alongTrack[i] << ", ";
            os << mt.altitudes[i] << "]\n";
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
    int skip_header()
    {
        int retval = -1;
        byte buf[8]={0}, *cur = buf;
        typedef enum{
            START,
            B,E,G,I,N,
            OK, EEOF, ERR
        }state_t;
        state_t stat = START;

        while(stat != OK && stat != EEOF && stat != ERR)
        {
            if(fread(cur,1,1,mFile) == 1)
            {
                if(*cur == 'b'){
                    if(stat==START){
                        stat=B;
                        cur++;
                    } else {
                        stat=START;
                        cur=buf;
                    }
                } else if(*cur == 'e'){
                    if(stat==B){
                        stat=E;
                        cur++;
                    } else {
                        stat=START;
                        cur=buf;
                    }
                } else if(*cur == 'g'){
                    if(stat==E){
                        stat=G;
                        cur++;
                    } else {
                        stat=START;
                        cur=buf;
                    }
                } else if(*cur == 'i'){
                    if(stat==G){
                        stat=I;
                        cur++;
                    } else {
                        stat=START;
                        cur=buf;
                    }
                } else if(*cur == 'n'){
                    if(stat==I){
                        stat=OK;
                        cur++;
                    } else {
                        stat=START;
                        cur=buf;
                    }
                } else {
                    stat=START;
                    cur=buf;
                }
            } else {
                if(feof(mFile)){
                    stat = EEOF;
                    TRN_NDPRINT(2, "end of data file\n");
                } else {
                    stat = ERR;
                    fprintf(stderr,"%s:%d - ERR data file read failed [%d:%s]\n",__func__, __LINE__, errno, strerror(errno));
                }
            }
        }

        if(stat == OK)
        {
            TRN_NDPRINT(2,"%s:%d - stat OK %s\n", __func__, __LINE__, buf);
            retval = 0;
        }
        return retval;
    }

    // finds and reads next record ID
    int next_record(byte *dest, size_t len, TrnLog::TrnRecID &r_type)
    {
        int retval = -1;
        byte buf[5]={0}, *cur = buf;
        typedef enum{
            START,
            MTN, MEA, MSE, MLE,
            OK, EEOF, ERR
        }state_t;
        state_t stat = START;

        while(stat != OK && stat != EEOF && stat != ERR)
        {
            if(fread(cur,1,1,mFile) == 1)
            {
                switch (*cur) {
                    case 'M':
                        if(stat==START){
                            cur++;
                        } else {
                            cur = buf;
                            stat = START;
                        }
                        break;
                    case 'T':
                        if(stat==START){
                            cur++;
                            stat = MTN;
                        } else {
                            cur = buf;
                            stat = START;
                        }
                        break;
                    case 'N':
                        if(stat==MTN){
                            cur++;
                        } else {
                            cur = buf;
                            stat = START;
                        }
                        break;
                    case 'E':
                        if(stat==START){
                            cur++;
                            stat = MEA;
                        } else if(stat == MSE || stat == MLE){
                            cur++;
                        } else {
                            cur = buf;
                            stat = START;
                        }
                        break;
                    case 'A':
                        if(stat==MEA){
                            cur++;
                        } else {
                            cur = buf;
                            stat = START;
                        }
                        break;
                    case 'I':
                        if(stat==MTN){
                            cur++;
                            stat = OK;
                            r_type = TrnLog::MOTN_IN;
                        } else if(stat==MEA){
                            cur++;
                            stat = OK;
                            r_type = TrnLog::MEAS_IN;
                        } else {
                            cur = buf;
                            stat = START;
                        }
                        break;
                    case 'O':
                        if(stat==MSE){
                            cur++;
                            stat = OK;
                            r_type = TrnLog::MSE_OUT;
                        }  else if(stat==MLE){
                            cur++;
                            stat = OK;
                            r_type = TrnLog::MLE_OUT;
                        } else {
                            cur = buf;
                            stat = START;
                        }
                        break;

                    case 'S':
                        if(stat==START){
                            cur++;
                            stat = MSE;
                        } else {
                            cur = buf;
                            stat = START;
                        }
                        break;
                    case 'L':
                        if(stat==START){
                            cur++;
                            stat = MLE;
                        } else {
                            cur = buf;
                            stat = START;
                        }
                        break;
                    default:
                        cur = buf;
                        stat = START;
                        break;
                }
            } else {
                if(feof(mFile)){
                    stat = EEOF;
                    TRN_NDPRINT(2, "end of data file\n");
                } else {
                    stat = ERR;
                    fprintf(stderr,"%s:%d - ERR data file read failed [%d:%s]\n",__func__, __LINE__, errno, strerror(errno));
                }
            }
        }

        if(stat == OK)
        {
            TRN_NDPRINT(2,"%s:%d - stat OK %s/%X\n", __func__, __LINE__, buf, r_type);
            memcpy(dest,buf,TL_RID_SIZE);
            retval = 0;
        }
        return retval;
    }

    // src points to rec_id (already read by next_record)
    int read_meas(measT **pdest, byte *src)
    {
        int retval = -1;
        // read data from file to buffer
        byte *bp = (byte *)src+TL_RID_SIZE;
        size_t readlen = TL_MEAI_HDR_SIZE;

        TRN_NDPRINT(2, "%s:%d readlen[%lu]\n", __func__, __LINE__, (unsigned long)readlen);
        if(fread(bp, readlen, 1, mFile) == 1)
        {
            bp += readlen;
            meas_in_t *measin =  (meas_in_t *)src;
            readlen = TL_MEAI_BEAM_SIZE(measin->num_meas);

            TRN_NDPRINT(2, "%s:%d readlen[%lu] num_meas[%d]\n", __func__, __LINE__, (unsigned long)readlen, measin->num_meas);
            if((bp + readlen) > (src + IBUF_LEN_BYTES)){
                TRN_NDPRINT(2, "%s:%d ERR: readlen exceeds buffer size [%lu/%d]\n", __func__, __LINE__, (bp + readlen - src), IBUF_LEN_BYTES);
                return retval;
            }

            if(fread(bp, readlen, 1, mFile) == 1)
            {
                measT *dest = new measT(measin->num_meas, measin->data_type);
                if(NULL != dest){
                    dest->time = measin->time;
                    dest->dataType = measin->data_type;
                    dest->x = measin->x;
                    dest->y = measin->y;
                    dest->z = measin->z;
                    dest->ping_number = measin->ping_number;
                    dest->numMeas = measin->num_meas;

                    meas_beam_t *beams = TrnLog::meaiBeamData(measin);
                    for(int i=0; i<dest->numMeas; i++)
                    {
                        dest->beamNums[i] = beams[i].beam_num;
                        dest->measStatus[i] = beams[i].status;
                        dest->ranges[i] = beams[i].range;
                        dest->crossTrack[i] = beams[i].cross_track;
                        dest->alongTrack[i] = beams[i].along_track;
                        dest->altitudes[i] = beams[i].altitude;
                    }
                    *pdest = dest;
                    retval = 0;
                } // else error
            } else {
                TRN_DPRINT("meas data read failed bp[%p] readlen[%zu] num_meas[%d]\n", bp, readlen, measin->num_meas);
            }
        } else {
            TRN_DPRINT("meas header read failed bp[%p] readlen[%zu]\n", bp, readlen);
        } // else error

        return retval;
    }

    // src points to rec_id (already read by next_record)
    int read_pose(poseT **pdest, byte *src)
    {
        int retval = -1;
        // read data from file to buffer
        byte *bp = (byte *)src+TL_RID_SIZE;
        size_t readlen = TL_MTNI_SIZE;
        if(fread(bp, readlen, 1, mFile) == 1)
        {
            poseT *dest = new poseT();
            if(NULL != dest)
            {
                bp += readlen;
                motn_in_t *motnin =  (motn_in_t *)src;

                dest->time = motnin->time;
                dest->x = motnin->x;
                dest->y = motnin->y;
                dest->z = motnin->z;
                dest->vx = motnin->vx;
                dest->vy = motnin->vy;
                dest->vz = motnin->vz;
                dest->phi = motnin->phi;
                dest->theta = motnin->theta;
                dest->psi = motnin->psi;
                dest->dvlValid = (motnin->dvl_valid != 0);
                dest->gpsValid = (motnin->gps_valid != 0);
                dest->bottomLock = (motnin->bottom_lock != 0);
                *pdest = dest;
                retval = 0;
            }
        }
        return retval;
    }

    // src points to rec_id (already read by next_record)
    int read_est(poseT **pdest, byte *src)
    {
        int retval = -1;
        // read data from file to buffer
        byte *bp = (byte *)src+TL_RID_SIZE;
        size_t readlen = TL_MSEO_SIZE;
        if(fread(bp, readlen, 1, mFile) == 1)
        {
            poseT *dest = new poseT();
            if(NULL != dest)
            {
                bp += readlen;
                est_out_t *est =  (est_out_t *)src;

                dest->time = est->time;
                dest->x = est->x;
                dest->y = est->y;
                dest->z = est->z;
                dest->vx = est->vx;
                dest->vy = est->vy;
                dest->vz = est->vz;
                dest->ve = est->ve;
                dest->vw_x = est->vw_x;
                dest->vw_y = est->vw_y;
                dest->vw_z = est->vw_z;
                dest->vn_x = est->vn_x;
                dest->vn_y = est->vn_y;
                dest->vn_z = est->vn_z;
                dest->wx = est->wx;
                dest->wy = est->wy;
                dest->wz = est->wz;
                dest->ax = est->ax;
                dest->ay = est->ay;
                dest->az = est->az;
                dest->phi = est->phi;
                dest->theta = est->theta;
                dest->psi = est->psi;
                dest->psi_berg = est->psi_berg;
                dest->psi_dot_berg = est->psi_dot_berg;

                dest->dvlValid = (est->dvl_valid != 0);
                dest->gpsValid = (est->gps_valid != 0);
                dest->bottomLock = (est->bottom_lock != 0);
                memcpy(dest->covariance,est->covariance, N_COVAR*sizeof(double));

                *pdest = dest;
                retval = 0;
            }
        }
        return retval;
    }

private:
    TrnLogConfig mConfig;
    TrnClient *mTrn;
    FILE *mFile;
    FILE *mTrnInCsvFile;
    FILE *mTrnOutCsvFile;
    bool mQuit;
    TLPStats mStats;

};


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
            {"show", required_argument, NULL, 0},
            {"server", no_argument, NULL, 0},
            {"noserver", no_argument, NULL, 0},
            {"logdir", required_argument, NULL, 0},
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
                                oflags |= TrnLogConfig::TRNI;
                            }
                            if(strstr(optarg,"trno") != NULL){
                                oflags |= TrnLogConfig::EST;
                            }
                            if(strstr(optarg,"est") != NULL){
                                oflags |= TrnLogConfig::EST;
                            }
                            if(strstr(optarg,"mmse") != NULL){
                                oflags |= TrnLogConfig::MMSE;
                            }
                            if(strstr(optarg,"mle") != NULL){
                                oflags |= TrnLogConfig::MLE;
                            }
                            if(strstr(optarg,"motn") != NULL){
                                oflags |= TrnLogConfig::MOTN;
                            }
                            if(strstr(optarg,"meas") != NULL){
                                oflags |= TrnLogConfig::MEAS;
                            }
                            if(strstr(optarg,"icsv") != NULL){
                                oflags |= TrnLogConfig::TRNI_CSV;
                            }
                            if(strstr(optarg,"ocsv") != NULL){
                                oflags |= TrnLogConfig::TRNO_CSV;
                            }
                            if(strstr(optarg,"*csv") != NULL){
                                oflags |= TrnLogConfig::ALL_CSV;
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
                    }
                    break;
                default:
                    help=true;
                    break;
            }

            if (version) {
                fprintf(stderr, "%s: version %s build %s\n", TRNLOG_PLAYER_NAME, TRNLOG_PLAYER_VERSION, TRNLOG_PLAYER_BUILD);
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
        char usage_message[] = "\n use: trnlog_player [options]\n"
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
        " --trno-csv=s           : write TRN outputs (estimates) to CSV file\n"
        " --server               : enable output to server\n"
        " --noserver             : disable output to server\n"
        " Notes:\n"
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

    const TrnLogConfig &tb_config()
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
    TrnLogConfig mTBConfig;
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

    // get log player
    TrnLogPlayer tbplayer(cfg.tb_config());

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
