/// @file csvlog_player.cpp
/// @authors k. headley
/// @date 21mar2022

/// Summary: play back CSV TRN log to console and/or TRN server

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

#define CSVLOG_PLAYER_NAME "trnxpp"
#ifndef CSVLOG_PLAYER_BUILD
/// @def CSVLOG_PLAYER_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMBTRN_BUILD=`date`
#define CSVLOG_PLAYER_BUILD "" VERSION_STRING(APP_BUILD)
#endif
#ifndef CSVLOG_PLAYER_VERSION
/// @def CSVLOG_PLAYER_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMBTRN_BUILD=`date`
#define CSVLOG_PLAYER_VERSION "" VERSION_STRING(CSVLOG_PLAYER_VER)
#endif



#define TRN_SERVER_PORT_DFL 27027
#define STRBUF_BYTES 8096

#ifndef DTR
#define DTR(x) ((x) * M_PI/180.)
#endif
#ifndef RTD
#define RTD(x) ((x) * 180./M_PI)
#endif

// /////////////////
// Types

typedef uint8_t byte;

typedef struct geo_s {
    // nominal number of beams
    double beam_count;
    // nominal angle subtended
    double swath_deg;
    // rotation vector (321Euler, phi/theta/psi radians)
    double rot_r[3];
    // translation vector (x/y/z m)
    double tran_m[3];
}geo_t;

typedef struct token_s {
    const char *key;
    int idx;
    const char *fmt;
}token_t;

typedef struct parse_kv_s {
    const char *key;
    void *pval;
}parse_kv_t;

token_t mb1_header_fmt[] = {
    {"time", 0, "%lf"},
    {"ping_number", 1, "%u"},
    {"posx", 2, "%lf"},
    {"posy", 3, "%lf"},
    {"depth", 4, "%lf"},
    {"heading", 5, "%lf"},
    {"pitch", 6, "%lf"},
    {"roll", 7, "%lf"},
    {"vx", 8, "%lf"},
    {"vy", 9, "%lf"},
    {"vz", 10, "%lf"},
    {"dvlValid", 11, "%d"},
    {"bottomLock", 12, "%d"},
    {"numMeas", 13, "%d"},
    {"b_start", 14, NULL},
    {"b_fields", 5, NULL},
    {"b_number", 0, "%d"},
    {"b_valid", 1, "%lf"},
    {"b_along", 2, "%lf"},
    {"b_across", 3, "%lf"},
    {"b_down", 4, "%lf"},
    {NULL, -1, NULL}
};

token_t idt_header_fmt[] = {
    {"time", 0, "%lf"},
    {"posx", 1, "%lf"},
    {"posy", 2, "%lf"},
    {"depth", 3, "%lf"},
    {"pitch", 4, "%lf"},
    {"roll", 5, "%lf"},
    {"heading", 6, "%lf"},
    {"flag0", 7, "%d"},
    {"flag1", 8, "%d"},
    {"flag2", 9, "%d"},
    {"vx", 10, "%lf"},
    {"vy", 11, "%lf"},
    {"vz", 12, "%lf"},
    {"dvlValid", 13, "%d"},
    {"bottomLock", 14, "%d"},
    {"numMeas", 15, "%d"},
    {"b_start", 16, NULL},
    {"b_fields", 2, NULL},
    {"b_number", 0, "%d"},
    {"b_range", 1, "%lf"},
    {NULL, -1, NULL}
};

// /////////////////
// Module variables
static int g_signal=0;
static bool g_interrupt=false;

// /////////////////
// Declarations

class CSVLogConfig
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

    typedef enum {
        FMT_MB1=0x01,
        FMT_IDT=0x02,
    } FmtFlags;


   CSVLogConfig()
    : mDebug(0), mVerbose(false), mHost("localhost"), mTrnCfg(), mPort(TRN_SERVER_PORT_DFL), mServer(false), mTrnInCsvEn(false), mTrnOutCsvEn(false), mTrnInCsvPath(), mTrnOutCsvPath(), mTrnSensor(TRN_SENSOR_MB), mOFlags(0), mFmtFlags(0), mUtmZone(10), mBeams(0), mStep(false), mSwath(0.)
    {
        memset(mSFRot,0,3*sizeof(double));
    }

    CSVLogConfig(const CSVLogConfig &other)
    : mDebug(other.mDebug), mVerbose(other.mVerbose), mHost(other.mHost), mTrnCfg(other.mTrnCfg), mPort(other.mPort), mServer(other.mServer),  mTrnInCsvEn(other.mTrnInCsvEn), mTrnOutCsvEn(other.mTrnOutCsvEn), mTrnInCsvPath(other.mTrnInCsvPath), mTrnOutCsvPath(other.mTrnOutCsvPath), mTrnSensor(other.mTrnSensor), mOFlags(other.mOFlags),  mFmtFlags(other.mFmtFlags), mUtmZone(other.mUtmZone), mBeams(other.mBeams), mStep(other.mStep), mSwath(other.mSwath)
    {
        memset(mSFRot,0,3*sizeof(double));
    }

   ~CSVLogConfig()
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
    bool oflag_set(CSVLogConfig::OFlags mask){return mOFlags.all_set(mask);}
    bool fflag_set(CSVLogConfig::FmtFlags mask){return mFmtFlags.all_set(mask);}
    long utm_zone(){return mUtmZone;}
    uint32_t beams(){return mBeams;}
    double *sfrot(){return mSFRot;}
    bool step(){return mStep;}
    double swath(){return mSwath;}

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
    void set_fflags(uint32_t flags){mFmtFlags = flags;}
    void set_utm(long utmZone){mUtmZone = utmZone;}
    void set_beams(uint32_t beams){mBeams = beams;}
    void set_sfrot(double phi_deg, double theta_deg, double psi_deg) {
        mSFRot[0] = Math::degToRad(phi_deg);
        mSFRot[1] = Math::degToRad(theta_deg);
        mSFRot[2] = Math::degToRad(psi_deg);
    }
    void set_step(bool step){mStep = step;}
    void set_swath(double swath){mSwath = swath;}

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
        os << std::setw(wkey) << "mSFRot" << std::setw(wval) << "[" << mSFRot[0] << "," <<mSFRot[1] << "," <<mSFRot[2] << "]" << std::endl;
        os << std::setw(wkey) << "mOFlags" << std::hex << std::setw(wval) << (uint32_t)mOFlags.get() << std::endl;
        os << std::setw(wkey) << "mFmtFlags" << std::hex << std::setw(wval) << (uint32_t)mFmtFlags.get() << std::endl;
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
    flag_var<uint32_t> mFmtFlags;
    long int mUtmZone;
    uint32_t mBeams;
    double mSFRot[3];
    bool mStep;
    double mSwath;
};

class MLPStats
{
public:
    MLPStats()
    :mFilesPlayed(0), mRecordsFound(0),mInvalidRecords(0), mMtniRead(0), mMeaiRead(0), mMseoRead(0), mMleoRead(0), mMotnUpdate(0), mMeasUpdate(0), mEstMMSE(0), mEstMLE(0), mLastMeasSuccess(0), mTrniCsvWrite(0), mTrnoCsvWrite(0)
    {}

    void stat_tostream(ostream &os, int wkey=18, int wval=15)
    {
        os << std::setw(wkey) << "-- stats --\n";
        os << std::setw(wkey) << "mFilesPlayed" << std::setw(wkey) << mFilesPlayed << "\n";
        os << std::setw(wkey) << "mRecordsFound" << std::setw(wkey) << mRecordsFound << "\n";
        os << std::setw(wkey) << "mInvalidRecords" << std::setw(wkey) << mInvalidRecords << "\n";
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
    uint32_t mInvalidRecords;
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

class CSVLogPlayer
{
public:

    CSVLogPlayer()
    :mConfig(), mTrn(NULL), mFile(NULL), mTrnInCsvFile(NULL), mTrnOutCsvFile(NULL), mQuit(false)
    {
    }

    CSVLogPlayer(const CSVLogConfig &cfg)
    :mConfig(cfg), mTrn(NULL), mFile(NULL), mTrnInCsvFile(NULL), mTrnOutCsvFile(NULL), mQuit(false)
    {
    }

    ~CSVLogPlayer()
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
            fclose(mFile);
        }

        if( (mFile = fopen(src.c_str(), "r")) == NULL)
        {
            fprintf(stderr, "%s:%d - could not open file[%s] [%d:%s]\n", __func__, __LINE__, src.c_str(), errno, strerror(errno));
            return retval;
        }

        byte ibuf[MB1_MAX_SOUNDING_BYTES]={0};

        poseT *ppt = NULL;
        measT *pmt = NULL;

        while (!mQuit && !g_interrupt) {

            int test = next_record(&ppt, &pmt);
            if(test > 0)
                break;
            if(test < 0)
                continue;

            this->stats().mRecordsFound++;

            if(NULL!=quit && *quit){
                break;
            }

            measT *mt = pmt;
            poseT *pt = ppt;

            if(pt != NULL){

                this->stats().mMtniRead++;

                if(mConfig.oflag_set(CSVLogConfig::MOTN))
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

            if (mt != NULL) {

                this->stats().mMeaiRead++;

                if(mConfig.oflag_set(CSVLogConfig::MEAS))
                {
                    show_mt(*mt);
                    std::cerr << "\n";
                }

                if(lastPT != NULL && mConfig.trni_csv()){
                    trni_csv_tofile(&mTrnInCsvFile, *lastPT, *mt);
                    this->stats().mTrniCsvWrite++;
                }

                if(lastPT != NULL && mConfig.oflag_set(CSVLogConfig::TRNI_CSV))
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

                            if(lastPT != NULL && mConfig.oflag_set(CSVLogConfig::EST)){
                                fprintf(stderr, "%s:%d --- EST --- \n",__func__, __LINE__);
                                show_est(ts, *lastPT, mle, mmse);
                            }

                            if(lastPT != NULL && mConfig.trno_csv()){
                                trno_csv_tofile(&mTrnOutCsvFile, ts, *lastPT, mle, mmse);
                                this->stats().mTrnoCsvWrite++;
                            }
                            if(lastPT != NULL && mConfig.oflag_set(CSVLogConfig::TRNO_CSV))
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

            if(pt != NULL)
                delete pt;
            if(mt != NULL)
                delete mt;

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

    void trni_csv_tostream(std::ostream &os, poseT &pt, measT &mt)
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


    int parse_tokens(char *src, char ***dest, int start, int len, const char *del)
    {
        int i = 0;
        int TA_INC = 128;

        int ta_len = TA_INC;
        char **toks = (char **)malloc(ta_len * sizeof(char *));

        if (NULL != src && NULL != toks && NULL != del) {
            memset(toks, 0, ta_len*sizeof(char *));

            char *scpy = strdup(src);

            // skip start tokens
            int cur = 0;
            while ( cur < start) {
                strtok((cur++ == 0 ? scpy : NULL), del);
            }

            while (len <= 0 || i < len) {

                char *tok = strtok((cur == 0 ? scpy : NULL), del);

                // stop if no more tokens
                if (NULL == tok) {
                    break;
                }

                toks[i] = strdup(tok);

                i++;
                cur++;

                if( (i >= ta_len)) {

                    char **tmp = (char **)realloc(toks, (ta_len + TA_INC) * sizeof(char *));

                    if(tmp != NULL) {
                        memset(tmp+ta_len, 0, TA_INC*sizeof(char *));
                        ta_len += TA_INC;
                        toks = tmp;

                    } else {
                        fprintf(stderr, "%s:%d - ERR: token array resize failed [%d:%s]\n", __func__, __LINE__, errno, strerror(errno));
                        break;
                    }
                }
            }

            // release source copy
            free(scpy);

            if (i > 0) {

                if (*dest == NULL) {
                    // dest NULL
                    // assign toks to dest
                    *dest = toks;
                } else {
                    *dest = toks;
                    // dest exists, copy toks
                    char **cp_start = toks;
                    int cp_entries = ( (len <= 0) || (len > i) ? i : len);
                    int cp_size = cp_entries * sizeof(char *);
                    memcpy(*dest, cp_start, cp_size);
                    free(toks);
                }
            }
        }
        return i;
    }

    const char *map_fmt(token_t *map, const char *key) {
        token_t *cur = map;
        while(cur != NULL && cur->idx >= 0) {
            if(strcmp(key, cur->key) == 0)
                return cur->fmt;
            cur++;
        }
        return NULL;
    }

    int map_idx(token_t *map, const char *key) {
        token_t *cur = map;
        while(cur != NULL && cur->idx >= 0) {
            if(strcmp(key, cur->key) == 0)
                return cur->idx;
            cur++;
        }
        return -1;
    }


    poseT *parse_pose(char **src, token_t *map)
    {
        // struct poseT

        // North, East, Down position (m)
        // double x, y, z

        // Vehicle velocity wrt iceberg, Body Frame(m/s)
        // double vx, vy, vz, ve
        // Vehicle velocity wrt iceberg, Body Frame(m/s)
        // was Vehicle velocity wrt an inertial frame, Body (m/s)
        // double vw_x, vw_y, vw_z
        // Vehicle velocity wrt iceberg, Body Frame(m/s)
        // double vn_x, vn_y, vn_z
        // Vehicle velocity wrt iceberg, Body Frame(m/s)
        // double wx, wy, wz
        // Vehicle aceleration wrt an inertial frame Body (m/s^2)
        // double ax, ay, az

        // 3-2-1 Euler angles relating the B frame to an inertial NED frame (rad).
        // double phi, theta, psi
        // TRN states
        // double psi_berg, psi_dot_berg
        // Time (s)
        // double time
        // Validity flag for dvl motion measurement
        // bool dvlValid
        // Validity flag for GPS measurement
        // bool gpsValid
        // Validity flag for DVL lock onto seafloor
        // bool bottomLock


        poseT *dest = new poseT();
        if(dest == NULL)
            return NULL;

        parse_kv_t dval_map[] {
            {"time", &dest->time},
            {"posx", &dest->x},
            {"posy", &dest->y},
            {"depth", &dest->z},
            {"pitch", &dest->phi},
            {"roll", &dest->theta},
            {"heading", &dest->psi},
            {"vx", &dest->vx},
            {"vy", &dest->vy},
            {"vz", &dest->vz},
            {NULL, NULL}
        };

        parse_kv_t bval_map[] {
            {"dvlValid", &dest->dvlValid},
            {"bottomLock", &dest->bottomLock},
            {NULL, NULL}
        };

        dest->ve = 0.;

        dest->vw_x = 0.;
        dest->vw_y = 0.;
        dest->vw_z = 0.;

        dest->vn_x = 0.;
        dest->vn_y = 0.;
        dest->vn_z = 0.;

        dest->wx = 0.;
        dest->wy = 0.;
        dest->wz = 0.;

        dest->ax = 0.;
        dest->ay = 0.;
        dest->az = 0.;

        dest->psi_berg = 0.;
        dest->psi_dot_berg = 0.;

        const parse_kv_t *pcur = dval_map;

        while(pcur->key != NULL) {
            const char *fmt = map_fmt(map, pcur->key);
            int idx = map_idx(map, pcur->key);

            if(fmt != NULL && idx >= 0){
                if (sscanf(src[idx], fmt, pcur->pval) != 1) {
                    fprintf(stderr, "%s:%d - ERR (dfields) scan %s from %s fmt %s var %p val %lf\n", __func__, __LINE__, pcur->key, src[idx], fmt, pcur->pval, *((double *)pcur->pval));
                }
            } else {
                fprintf(stderr, "%s:%d - ERR (dfields) invalid arg fmt %p idx %d\n", __func__, __LINE__, fmt, idx);
            }

            pcur++;
        }

        pcur = bval_map;

        while(pcur->key != NULL) {
            const char *fmt = map_fmt(map, pcur->key);
            int idx = map_idx(map, pcur->key);

            if(fmt != NULL && idx >= 0){
                int ival = -1;
                if(sscanf(src[idx], fmt, &ival) ==1) {
                    *((bool *)pcur->pval) = (ival != 0 ? true : false);
                } else {
                    fprintf(stderr, "%s:%d - ERR (bfields) scan %s from %s fmt %s var %p val %d\n", __func__, __LINE__, pcur->key, src[idx], fmt, pcur->pval, *((bool *)pcur->pval));
                }
            } else {
                fprintf(stderr, "%s:%d - ERR (bfields) invalid arg fmt %p idx %d\n", __func__, __LINE__, fmt, idx);
            }

            pcur++;
        }

        if(dest->z < 2.) {
            // on surface
            dest->gpsValid = true;
            dest->bottomLock = false;
            dest->dvlValid = false;
        } else {
            dest->gpsValid = false;
            dest->bottomLock = true;
            dest->dvlValid = true;
        }

        if(mConfig.fflag_set(CSVLogConfig::FMT_MB1)) {

            double pos_N=0., pos_E=0.;
            double lat = dest->x, lon=dest->y;
            NavUtils::geoToUtm(Math::degToRad(lat), Math::degToRad(lon), mConfig.utm_zone(), &pos_N, &pos_E);
            dest->x = pos_N;
            dest->y = pos_E;
            // for MB1, phi/theta have already been applied
            dest->phi = 0.;
            dest->theta = 0.;
            // force velocity to dummy values to match mb1 binary
            dest->vx = 0.1;
            dest->vy = 0.0;
            dest->vz = 0.0;

        }

        return dest;
    }


    static void matrix_show(Matrix m, const char *name=nullptr, int width=7, int precision=3, int wkey=5)
    {
        matrix_tostream(std::cerr, m, name, width, precision, wkey);
    }

    static void matrix_tostream(std::ostream &os, Matrix m, const char *name=nullptr, int width=7, int precision=3, int wkey=5)
    {

        std::ios_base::fmtflags orig_settings = std::cout.flags();

        if(name != nullptr)
            os << std::setw(wkey) << name << " [" << m.Nrows() << "r " << m.Ncols() << "c]" << std::endl;

        os << std::fixed << std::setprecision(precision);

        for(int i = 1 ; i <= m.Nrows() ; i++ ){
            os << std::setw(wkey) << " [" << i << "] :";
            for(int j = 1 ; j <= m.Ncols() ; j++ ){
                os << " " << std::setw(width) << m(i, j);
            }
            os << std::endl;
        }

        std::cout.flags(orig_settings);
    }

    // 321 euler rotation R(phi, theta, psi)
    // where
    // phi: roll (rotation about X)
    // theta: pitch (rotation about Y)
    // psi: yaw (rotation about Z)
    // expects
    // rot_rad[0]: phi / roll (rad)
    // rot_rad[1]: theta / pitch (rad)
    // rot_rad[2]: psi / yaw (rad)
    static Matrix affine321Rotation(double *rot_rad)
    {
        Matrix mat(4,4);

        double cphi = cos(rot_rad[0]);
        double sphi = sin(rot_rad[0]);
        double ctheta = cos(rot_rad[1]);
        double stheta = sin(rot_rad[1]);
        double cpsi = cos(rot_rad[2]);
        double spsi = sin(rot_rad[2]);
        double stheta_sphi = stheta * sphi;
        double stheta_cphi = stheta * cphi;

        mat(1, 1) = cpsi * ctheta;
        mat(1, 2) = spsi * ctheta;
        mat(1, 3) = -stheta;
        mat(1, 4) = 0.;
        mat(2, 1) = -spsi * cphi + cpsi * stheta_sphi;
        mat(2, 2) = cpsi * cphi + spsi * stheta_sphi;
        mat(2, 3) = ctheta * sphi;
        mat(2, 4) = 0.;
        mat(3, 1) = spsi * sphi + cpsi * stheta_cphi;
        mat(3, 2) = -cpsi * sphi + spsi * stheta_cphi;
        mat(3, 3) = ctheta * cphi;
        mat(3, 4) = 0.;
        mat(4, 1) = 0.;
        mat(4, 2) = 0.;
        mat(4, 3) = 0.;
        mat(4, 4) = 1.;

        return mat;
    }

    static Matrix affineTranslation(double *tran_m)
    {
        Matrix mat(4,4);

        mat(1, 1) = 1.;
        mat(1, 2) = 0.;
        mat(1, 3) = 0.;
        mat(1, 4) = tran_m[0];
        mat(2, 1) = 0.;
        mat(2, 2) = 1.;
        mat(2, 3) = 0.;
        mat(2, 4) = tran_m[1];
        mat(3, 1) = 0.;
        mat(3, 2) = 0.;
        mat(3, 3) = 1.;
        mat(3, 4) = tran_m[2];
        mat(4, 1) = 0.;
        mat(4, 2) = 0.;
        mat(4, 3) = 0.;
        mat(4, 4) = 1.;

        return mat;
    }

    Matrix mb_sframe_components(measT *mt, geo_t *geo) {

        if(mt == nullptr || geo == NULL){
            Matrix err_ret = Matrix(4,1);
        }

      // number of beams read (<= nominal beams)
        int nbeams = mt->numMeas;
        Matrix sf_comp = Matrix(4, nbeams);

        double S = geo->swath_deg;
        double K = (180. - S)/2.;
        double e = S/geo->beam_count;

        // zero- and one-based indexs
        int idx[2] = {0, 1};

        for (int i = 0; i < nbeams; i++)
        {
            // beam number (0-indexed)
            int b = mt->beamNums[i];

            // beam yaw, pitch angles (sensor frame)
            double yd = 0;
            double xd = (K + S - (b * e));
            double pd = xd;

            if(xd > 90.) {
                // normalize pitch to +/- 90 deg
                yd = 180.;
                pd = 180. - xd;
            }

            // psi (yaw), phi (pitch) to radians
            double yr = DTR(yd);
            double pr = DTR(pd);

            // beam components (reference orientation, sensor frame)
            // 1: along (x) 2: across (y) 3: down (z)
            sf_comp(1, idx[1]) = cos(pr)*cos(yr);
            sf_comp(2, idx[1]) = cos(pr)*sin(yr);
            sf_comp(3, idx[1]) = sin(pr);
            // set M[4,i]
            // 1.: points
            // 0.: vectors
            sf_comp(4, idx[1]) = 1;

            idx[0]++;
            idx[1]++;
        }
        return sf_comp;
    }

    measT *parse_meas(char **src, token_t *map)
    {
        // struct measT

        // double time
        // 1: DVL
        // 2: Multibeam
        // 3: Single Beam
        // 4: Homer Relative Measurement
        // 5: Imagenex multibeam
        // 6: Side-looking DVL
        // int dataType
        // double phi, theta, psi
        // double x, y, z
        // double* covariance
        // double* ranges
        // double* crossTrack
        // double* alongTrack
        // double* altitudes
        // double* alphas
        // bool* measStatus
        // unsigned int ping_number
        // number of beams
        // int numMeas
        // For use in sensors that vary the number of beams (e.g., MB-system)
        // int* beamNums

        // parse number of beams
        // to make measT instance
        int src_beams = 0;
        const char *fx = map_fmt(map, "numMeas");
        int ix = map_idx(map, "numMeas");
        sscanf(src[ix], fx, &src_beams);

        if(src_beams < 0 || src_beams > MB1_MAX_BEAMS){
            fprintf(stderr, "%s:%d ERR numMeas %d\n", __func__, __LINE__, src_beams);
            return NULL;
        }

        // use source beams by default, config beams if set
        int dest_beams = ((mConfig.beams() > 0) ? mConfig.beams() : src_beams);

        measT *dest = new measT(dest_beams, mConfig.trn_sensor());

        double swath_lim = mConfig.swath()/2.;
        int mod = 1;
        if(mConfig.beams() > 0){
            if(mConfig.swath() > 0.) {
                mod = mConfig.swath() / mConfig.beams();
            } else {
                mod = src_beams / mConfig.beams();
            }
        }

        if(mod <= 0)
            mod = 1;

//        fprintf(stderr, "%s:%d src_beams %d mConfig.beams %d dest_beams %d dest.numMeas %d mod %d\n", __func__, __LINE__, src_beams, mConfig.beams(), dest_beams, dest->numMeas, mod);

        if(dest == NULL){
            fprintf(stderr, "%s:%d ERR malloc\n", __func__, __LINE__);
            return NULL;
        }

        parse_kv_t dval_map[] {
            {"time", &dest->time},
            {"posx", &dest->x},
            {"posy", &dest->y},
            {"depth", &dest->z},
            {"pitch", &dest->phi},
            {"roll", &dest->theta},
            {"heading", &dest->psi},
          {NULL, NULL}
        };

        parse_kv_t uval_map[] {
            {"ping_number", &dest->ping_number},
            {NULL, NULL}
        };

        dest->dataType = mConfig.trn_sensor();

        parse_kv_t *cur = dval_map;

        while(cur->key != NULL) {
            const char *fmt = map_fmt(map, cur->key);
            int idx = map_idx(map, cur->key);

            if(fmt != NULL && idx >= 0){
                if (sscanf(src[idx], fmt, cur->pval) != 1){
                    fprintf(stderr, "%s:%d - ERR (dfields) scan %s from %s fmt %s var %p val %lf\n", __func__, __LINE__, cur->key, src[idx], fmt, cur->pval, *((double *)cur->pval));
                }
            } else {
                fprintf(stderr, "%s:%d - ERR (ifields) invalid arg fmt %p idx %d\n", __func__, __LINE__, fmt, idx);
            }

            cur++;
        }

        cur = uval_map;

        while(cur->key != NULL) {
            const char *fmt = map_fmt(map, cur->key);
            int idx = map_idx(map, cur->key);

            if(fmt != NULL && idx >= 0){
                if(sscanf(src[idx], fmt, cur->pval) != 1){
                    fprintf(stderr, "%s:%d - ERR (ufields) scan %s from %s fmt %s var %p val %u\n", __func__, __LINE__, cur->key, src[idx], fmt, cur->pval, *((unsigned int *)cur->pval));
                }
            } else {
                fprintf(stderr, "%s:%d - ERR (ufields) invalid arg fmt %p idx %d\n", __func__, __LINE__, fmt, idx);
            }

            cur++;
        }


        if (dest->numMeas <= 0 || dest->numMeas > MB1_MAX_BEAMS) {
            fprintf(stderr, "%s:%d ERR numMeas %d\n", __func__, __LINE__, dest->numMeas);
            free(dest);
            return NULL;
        } else if (dest->time <= 0) {
            fprintf(stderr, "%s:%d ERR time %.3lf\n", __func__, __LINE__, dest->time);
            free(dest);
            return NULL;
        } else if ((dest->x > -1. && dest->x < 1.)  || (dest->y > -1. && dest->y < 1.) || (dest->z > -1. && dest->z < 1.)) {
            fprintf(stderr, "%s:%d ERR x,y,z [%.3lf, %.3lf, %.3lf]\n", __func__, __LINE__, dest->x, dest->y, dest->z);
            free(dest);
            return NULL;
        }

        if(mConfig.fflag_set(CSVLogConfig::FMT_IDT)) {

            double pos_N=0., pos_E=0.;
            double lat=dest->x, lon=dest->y;
            NavUtils::geoToUtm(Math::degToRad(lat), Math::degToRad(lon), mConfig.utm_zone(), &pos_N, &pos_E);
            dest->x = pos_N;
            dest->y = pos_E;

            int b_start = map_idx(map, "b_start");
            int b_fields = map_idx(map, "b_fields");
            int b_num = map_idx(map, "b_number");
            int b_range = map_idx(map, "b_range");
            int x=mod * b_fields;
            int f_end = b_start + x * dest->numMeas;

            fprintf(stderr, "%s:%d - b_start %d nmeas %d bfields %d bnofs %d brofs %d f_end %d\n", __func__, __LINE__, b_start, dest->numMeas, b_fields, b_num, b_range, f_end);

            if(b_start >= 0 ) {

                // parse status, beam numbers, ranges
                // needed for components
                int idx[2] = {0, 1};

                for (int i = b_start; i < f_end; i += (x)) {
                    int beam_n = 0;
                    double range = 0;
                    int valid = 1;

                    sscanf(src[i+b_num], "%d", &beam_n);
                    sscanf(src[i+b_range], "%lf", &range);

                    if(range <= 0.){
                        valid = 0;
                    }

                    dest->measStatus[idx[0]] = (valid != 0 ? true : false);
                    dest->beamNums[idx[0]] = beam_n;
                    dest->ranges[idx[0]] = range;
                    idx[0]++;
                }

                geo_t sf_geo = {120, 120, {0.,0.,0.}, {0.,0.,0.}};
                Matrix sf_comp = mb_sframe_components(dest, &sf_geo);

                idx[0]=0;
                idx[1]=1;

                if(mConfig.trn_sensor() == TRN_SENSOR_DELTAT) {

                    for (int i = b_start; i < f_end; i += (x)) {
                        dest->alongTrack[idx[0]] = dest->ranges[idx[0]] * sf_comp(1, idx[1]);
                        dest->crossTrack[idx[0]] = dest->ranges[idx[0]] * sf_comp(2, idx[1]);
                        dest->altitudes[idx[0]] = dest->ranges[idx[0]] * sf_comp(3, idx[1]);

                        //                    fprintf(stderr, "%s:%d - field[%d] beam_n[%d] r,x,y,z[%.2lf, %.2lf, %.2lf, %.2lf] sfc[%.2lf, %.2lf, %.2lf]\n", __func__, __LINE__, i, dest->beamNums[idx[0]], dest->ranges[idx[0]], dest->crossTrack[idx[0]], dest->alongTrack[idx[0]], dest->altitudes[idx[0]], sf_comp(1, idx[1]), sf_comp(2, idx[1]), sf_comp(3, idx[1]));
                        idx[0]++;
                        idx[1]++;
                    }

                } else {
                    double v_att[3] = {dest->phi, dest->theta, 0.};
                    Matrix m_att = affine321Rotation(v_att).t();
                    Matrix m_dr = affine321Rotation(mConfig.sfrot());
                    Matrix m_all = m_dr * m_att * m_dr.t() * sf_comp;
//                    Matrix m_all = sf_comp;

                    for (int i = b_start; i < f_end; i += (x)) {

                        dest->alongTrack[idx[0]] = dest->ranges[idx[0]] * m_all(1, idx[1]);
                        dest->crossTrack[idx[0]] = dest->ranges[idx[0]] * m_all(2, idx[1]);
                        dest->altitudes[idx[0]] = dest->ranges[idx[0]] * m_all(3, idx[1]);

                        int b = dest->beamNums[idx[0]];
                        if( (b < 30) || (b > 90) || (b % 6 != 0))
                        {
                            dest->measStatus[idx[0]] = false;
                        }

                        //                    fprintf(stderr, "%s:%d - field[%d] beam_n[%d] r,x,y,z[%.2lf, %.2lf, %.2lf, %.2lf] sfc[%.2lf, %.2lf, %.2lf]\n", __func__, __LINE__, i, dest->beamNums[idx[0]], dest->ranges[idx[0]], dest->crossTrack[idx[0]], dest->alongTrack[idx[0]], dest->altitudes[idx[0]], sf_comp(1, idx[1]), sf_comp(2, idx[1]), sf_comp(3, idx[1]));
                        idx[0]++;
                        idx[1]++;
                    }
                }
            }
        } else if(mConfig.fflag_set(CSVLogConfig::FMT_MB1)) {

            double pos_N=0., pos_E=0.;
            double lat=dest->x, lon=dest->y;
            NavUtils::geoToUtm(Math::degToRad(lat), Math::degToRad(lon), mConfig.utm_zone(), &pos_N, &pos_E);
            dest->x = pos_N;
            dest->y = pos_E;
            dest->phi = 0.;
            dest->theta = 0.;
            dest->psi = 0.;

            int b_start = map_idx(map, "b_start");
            int b_fields = map_idx(map, "b_fields");
            int b_num = map_idx(map, "b_number");
            int b_valid = map_idx(map, "b_valid");
            int b_across = map_idx(map, "b_across");
            int b_along = map_idx(map, "b_along");
            int b_down = map_idx(map, "b_down");
//            int x = b_fields * mod ;
//            int f_end = b_start + ( x * dest->numMeas);
            int x = b_fields ;
            int f_end = b_start + ( x * src_beams);

//            fprintf(stderr, "%s:%d - b_start %d nmeas %d bfields %d bnum %d bval %d bacross %d balong %d bdown %d f_end %d\n", __func__, __LINE__, b_start, dest->numMeas, b_fields, b_num, b_valid, b_across, b_along, b_down, f_end);
//            fprintf(stderr, "%s:%d - src_beams[%d] dest %p dest->numMeas %p/%d\n", __func__, __LINE__, src_beams, dest, &dest->numMeas,dest->numMeas);



            int j = 0;
            if(b_start >= 0) {
                for (int i = b_start; i < f_end; i += (x) ) {
                    int beam_n = 0;
                    double rho[3] = {0., 0., 0.};
                    int valid = 0;

                    sscanf(src[i + b_num], "%d", &beam_n);
                    sscanf(src[i + b_valid], "%d", &valid);
                    sscanf(src[i + b_along], "%lf", &rho[0]);
                    sscanf(src[i + b_across], "%lf", &rho[1]);
                    sscanf(src[i + b_down], "%lf", &rho[2]);

                    double range = vnorm(rho);

                    if(range <= 0.){
                        valid = 0;
                    }

                    bool use_beam = false;
                    double wb = 0.;
                    if ((beam_n % mod) == 0) {

                        wb = RTD(atan2(rho[1], rho[2]));

                        if(mConfig.swath() <= 0. || fabs(wb) <= swath_lim){
                            use_beam = true;
                        }
                    }
//                    fprintf(stderr, "%s:%d beam[%3d] x,y,z,r,w[%+8.3lf, %+8.3lf, %+8.3lf, %+8.3lf, %+8.3lf] swath_max[%+8.3lf] %c\n", __func__, __LINE__, beam_n, rho[0], rho[1], rho[2], range, wb, swath_lim, (use_beam?'Y':'N'));

                    if(valid && use_beam){
                        dest->measStatus[j] = true;
                        dest->beamNums[j] = beam_n;
                        dest->alongTrack[j] = rho[0];
                        dest->crossTrack[j] = rho[1];
                        dest->altitudes[j] = rho[2];
                        dest->ranges[j] = range;
                        j++;
                    }

                    if(j >= dest_beams)
                        break;
                }
            }
        }

       return dest;
    }

    void release_toks(char ***pp, int ntoks) {
        if(NULL != pp) {
            char **ptoks = *pp;
            if(NULL != ptoks){
                // release token memory
                for(int i = 0; i < ntoks; i++ ) {
                    free(ptoks[i]);
                }
                free(ptoks);
                *pp= NULL;
            }
        }
    }

    // finds and reads next record ID
    int next_record(poseT **ppose, measT **pmeas)
    {
        int retval = -1;

        typedef enum{
            START,
            CSV,
            OK, EEOF, ERR
        }state_t;
        state_t stat = START;

        size_t msg_buf_len = MB1_MAX_SOUNDING_BYTES + sizeof(mb1_t);
        byte msg_buf[msg_buf_len];

        char str_buf[STRBUF_BYTES] = {0};


        while(stat != OK && stat != EEOF && stat != ERR)
        {
            memset(msg_buf, 0, msg_buf_len);
            memset(str_buf, 0, STRBUF_BYTES);

            bool ferr=false;
            bool rec_valid = false;

            // read CSV record
            if (fgets(str_buf, STRBUF_BYTES, mFile) == str_buf) {

                char **toks = NULL;
                int ntoks = parse_tokens(str_buf, &toks, 0, 0, ",");

                if (  ntoks > 1) {

                    token_t *format_map = mConfig.fflag_set(CSVLogConfig::FMT_IDT) ? idt_header_fmt : mb1_header_fmt;

                    *ppose = parse_pose(toks, format_map);
                    *pmeas = parse_meas(toks, format_map);

                    if(*ppose != NULL && *pmeas != NULL) {
                        TRN_NDPRINT(5, "%s:%d parsed line %s\n", __func__, __LINE__, str_buf);
                        rec_valid = true;
                    } else {
                        fprintf(stderr, "%s:%d invalid record (pose:%p meas:%p] : %s\n", __func__, __LINE__, ppose, pmeas, str_buf);

                        if(*pmeas != NULL)
                            delete(*pmeas);
                        if(*ppose != NULL)
                            delete(*ppose);

                        *ppose = NULL;
                        *pmeas = NULL;

                        mStats.mInvalidRecords++;
                    }

                    release_toks(&toks, ntoks);
                }
                toks = NULL;
            }

            if (rec_valid && ferr == false) {
                // TODO : update stats?
                stat = OK;
           } else {

                if(feof(mFile)){
                    TRN_NDPRINT(2,"%s:%d - EOF\n", __func__, __LINE__);
                    stat = EEOF;
                } else {
                    TRN_NDPRINT(2,"%s:%d - ERR\n", __func__, __LINE__);
                    stat = ERR;
                }
            }
        }

        if (stat == OK) {
            TRN_NDPRINT(2,"%s:%d - stat OK\n", __func__, __LINE__);
            retval = 0;
        } else if (stat == ERR) {
            TRN_NDPRINT(2,"%s:%d - stat ERR\n", __func__, __LINE__);

        } else if (stat == EEOF) {
            TRN_NDPRINT(2,"%s:%d - stat EEOF (end of input file)\n", __func__, __LINE__);
            retval = 1;
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

private:
    CSVLogConfig mConfig;
    TrnClient *mTrn;
    FILE *mFile;
    FILE *mTrnInCsvFile;
    FILE *mTrnOutCsvFile;
    bool mQuit;
    MLPStats mStats;
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
            {"utm", required_argument, NULL, 0},
            {"beams", required_argument, NULL, 0},
            {"show", required_argument, NULL, 0},
            {"server", no_argument, NULL, 0},
            {"noserver", no_argument, NULL, 0},
            {"logdir", required_argument, NULL, 0},
            {"format", required_argument, NULL, 0},
            {"sfrot", required_argument, NULL, 0},
            {"step", no_argument, NULL, 0},
            {"swath", required_argument, NULL, 0},
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
                        // utm
                        else if (strcmp("utm", options[option_index].name) == 0) {
                            long int utm=0;
                            if(sscanf(optarg,"%ld", &utm) == 1)
                                mTBConfig.set_utm(utm);
                        }
                        // beams
                        else if (strcmp("beams", options[option_index].name) == 0) {
                            uint32_t beams=0;
                            if(sscanf(optarg,"%" PRIu32 "", &beams) == 1)
                                mTBConfig.set_beams(beams);
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
                                oflags |= CSVLogConfig::TRNI;
                            }
                            if(strstr(optarg,"trno") != NULL){
                                oflags |= CSVLogConfig::EST;
                            }
                            if(strstr(optarg,"est") != NULL){
                                oflags |= CSVLogConfig::EST;
                            }
                            if(strstr(optarg,"mmse") != NULL){
                                oflags |= CSVLogConfig::MMSE;
                            }
                            if(strstr(optarg,"mle") != NULL){
                                oflags |= CSVLogConfig::MLE;
                            }
                            if(strstr(optarg,"motn") != NULL){
                                oflags |= CSVLogConfig::MOTN;
                            }
                            if(strstr(optarg,"meas") != NULL){
                                oflags |= CSVLogConfig::MEAS;
                            }
                            if(strstr(optarg,"icsv") != NULL){
                                oflags |= CSVLogConfig::TRNI_CSV;
                            }
                            if(strstr(optarg,"ocsv") != NULL){
                                oflags |= CSVLogConfig::TRNO_CSV;
                            }
                            if(strstr(optarg,"*csv") != NULL){
                                oflags |= CSVLogConfig::ALL_CSV;
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
                        // sfrot
                        else if (strcmp("sfrot", options[option_index].name) == 0) {
                            double tmp[3] = {0., 0., 0.};
                            if(sscanf(optarg, "%lf,%lf,%lf", &tmp[0], &tmp[1], &tmp[2]) == 3){
                                mTBConfig.set_sfrot(tmp[0], tmp[1], tmp[2]);
                            }
                        }
                        // format
                        else if (strcmp("format", options[option_index].name) == 0) {
                            uint32_t flags = 0;
                            if(strstr(optarg, "idt") != NULL) {
                                flags |= CSVLogConfig::FMT_IDT;
                            }
                            if(strstr(optarg, "mb1") != NULL) {
                                flags |= CSVLogConfig::FMT_MB1;
                            }
                            if(flags>0){
                                mTBConfig.set_fflags(0x0);
                                mTBConfig.set_fflags(flags);
                            }
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
                    }
                    break;
                default:
                    help=true;
                    break;
            }

            if (version) {
                fprintf(stderr, "%s: version %s build %s\n", CSVLOG_PLAYER_NAME, CSVLOG_PLAYER_VERSION, CSVLOG_PLAYER_BUILD);
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
        char usage_message[] = "\n use: csvlog_player [options]\n"
        "\n"
        " Options\n"
        " --verbose              : verbose output\n"
        " --debug=d              : debug output\n"
        " --help                 : output help message\n"
        " --cfg=s                : app config file\n"
        " --version              : output version info\n"
        " --format=d             : input CSV format: mb1|idt\n"
        " --trn-host=addr[:port] : send output to TRN server\n"
        " --trn-cfg=s            : TRN config file\n"
        " --trn-sensor=n         : TRN sensor type\n"
        " --utm=n                : UTM zone\n"
        " --beams=n              : number of output beams\n"
        " --swath=f              : limit beams to center swath degrees\n"
        " --sfrot=d,d,d          : sensor frame rotation (phi, theta, psi; deg)\n"
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
        " --step                 : step through entries\n"
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
        "              use modulus max(swath / beams_out, 1)\n"
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

    const CSVLogConfig &tb_config()
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
    CSVLogConfig mTBConfig;
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
    CSVLogPlayer tbplayer(cfg.tb_config());

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
