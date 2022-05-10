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

#include "structDefs.h"
#include "TrnLog.h"
#include "TrnClient.h"
#include "trn_debug.hpp"

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

#ifndef DTR
#define DTR(x) (x * M_PI/180.)
#endif
#ifndef RTD
#define RTD(x) (x * 180./M_PI)
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

   TrnLogConfig()
    : mDebug(0), mVerbose(false), mHost("localhost"), mTrnCfg(), mPort(TRN_SERVER_PORT_DFL), mServer(false), mConsole(true), mCsv(false), mCsvPath(), mTrnSensor(TRN_SENSOR_MB)
    {

    }

    TrnLogConfig(const TrnLogConfig &other)
    : mDebug(other.mDebug), mVerbose(other.mVerbose), mHost(other.mHost), mTrnCfg(other.mTrnCfg), mPort(other.mPort), mServer(other.mServer), mConsole(other.mConsole), mCsv(other.mCsv), mCsvPath(other.mCsvPath), mTrnSensor(other.mTrnSensor)
    {

    }

   ~TrnLogConfig()
    {
    }

    bool server(){return mServer;}
    bool console(){return mConsole;}
    bool csv(){return mCsv;}
    int trn_sensor(){return mTrnSensor;}
    std::string host(){return mHost;}
    std::string trn_cfg(){return mTrnCfg;}
    std::string csv_path(){return mCsvPath;}
    int port(){return mPort;}

    void set_console(bool enable){mConsole = enable;}
    void set_server(bool enable){mServer = enable;}
    void set_csv(bool enable){mCsv = enable;}
    void set_csv_path(const std::string &path){mCsvPath = std::string(path);}
    void set_host(const std::string &host){mHost = std::string(host);}
    void set_port(int port){mPort = port;}
    void set_trn_sensor(int id){mTrnSensor = id;}
    void set_trn_cfg(const std::string &cfg){mTrnCfg = std::string(cfg);}
    void set_debug(int debug){mDebug = debug;}
    void set_verbose(bool verbose){mVerbose = verbose;}

protected:
private:
    int mDebug;
    bool mVerbose;
    std::string mHost;
    std::string mTrnCfg;
    int mPort;
    bool mServer;
    bool mConsole;
    bool mCsv;
    std::string mCsvPath;
    int mTrnSensor;

};

class TrnLogPlayer
{
public:

    TrnLogPlayer()
    :mConfig(), mTrn(NULL), mFile(NULL), mCsvFile(NULL), mQuit(false)
    {
    }

    TrnLogPlayer(const TrnLogConfig &cfg)
    :mConfig(cfg), mTrn(NULL), mFile(NULL), mCsvFile(NULL), mQuit(false)
    {
    }

    ~TrnLogPlayer()
    {
        if(mFile != NULL){
            fclose(mFile);
        }

        if(mCsvFile != NULL){
            fclose(mCsvFile);
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

        TrnLog::TrnRecID rec_type = TrnLog::RT_INVALID;
        byte ibuf[2048]={0};

        while (!mQuit && next_record(ibuf, 2048, rec_type) == 0) {

            memset(ibuf, 0, 2048);

            if(rec_type == TrnLog::MOTN_IN) {

                poseT *pt = NULL;
                if(read_pose(&pt, ibuf) == 0 && pt != NULL){

                    if(mConfig.console())
                    {
                        show_pt(*pt);
                        std::cerr << "\n";
                    }

                    if(mConfig.server() && mTrn != NULL)
                    {
                        try{
                            mTrn->motionUpdate(pt);
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

                    if(mConfig.console())
                    {
                        show_mt(*mt);
                        std::cerr << "\n";
                    }

                    if(mConfig.server())
                    {
                        try{
                            mTrn->measUpdate(mt, mConfig.trn_sensor());

                            if(mTrn->lastMeasSuccessful()){
                                poseT mle, mmse;
                                mTrn->estimatePose(&mmse, TRN_EST_MMSE);
                                mTrn->estimatePose(&mle, TRN_EST_MLE);
                                if(mConfig.console()){
                                    auto ts_now = std::chrono::system_clock::now();
                                    std::chrono::duration<double> epoch_time = ts_now.time_since_epoch();
                                    double ts = epoch_time.count();

                                    show_est(ts, lastPT, mle, mmse);
                                }
                                if(mConfig.csv()){
                                    csv_tofile(mCsvFile, lastPT, *mt);
                                    if(mConfig.console()){
                                        show_csv(lastPT, *mt);
                                    }
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
            } else {
                TRN_NDPRINT(2,"invalid record type[%d]\n",rec_type);
            }
        }
        return retval;
    }

    void set_console(bool enable){mConfig.set_console(enable);}
    void set_server(bool enable){mConfig.set_server(enable);}
    void quit(){
        TRN_DPRINT("setting player quit flag\n");
        mQuit = true;
    }

protected:

    void csv_tostream(std::ostream &os, poseT &pt, measT &mt)
    {
        // Note that TRN uses N,E,D frame (i.e. N:x E:y D:z)
        // time POSIX epoch sec
        // northings
        // eastings
        // depth
        // heading
        // pitch
        // roll
        // flag (0)
        // flag (0)
        // flag (0)
        // vx (0)
        // xy (0)
        // vz (0)
        // sounding valid flag
        // bottom lock valid flag
        // number of beams
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

    std::string csv_tostring(poseT &pt, measT &mt)
    {
        ostringstream ss;
        csv_tostream(ss, pt, mt);
        return ss.str();
    }

    void csv_tofile(FILE *fp, poseT &pt, measT &mt)
    {
        std::string csv = csv_tostring(pt, mt);
        if(NULL == fp)
        {
            fp = fopen(mConfig.csv_path().c_str(),"a");
        }

        if(NULL != fp){
            fprintf(fp, "%s",csv.c_str());
        } else {
            TRN_DPRINT("ERR - could not open file[%s]\n",mConfig.csv_path().c_str());
        }
    }

    void show_csv(poseT &pt, measT &mt)
    {
        csv_tostream(std::cerr, pt, mt);
        std::cerr << "\n";
    }

    void est_tostream(std::ostream &os, double ts, poseT &pt, poseT &mle, poseT &mmse, int wkey=15, int wval=18)
    {
        os << "--- TRN Estimate OK---" << "\n";
        os << "MLE[t,tm,x,y,z]  [";
        os << std::fixed << std::setprecision(3);
        os << ts << ",";
        os << std::setprecision(2);
        os << mle.time << ",";
        os << std::setprecision(4);
        os << mle.x << "," << mle.y << "," << mle.z << "]\n";

        os << "MMSE[t,tm,x,y,z] [";
        os << std::fixed << std::setprecision(3);
        os << ts << ",";
        os << std::setprecision(2);
        os << mmse.time << ",";
        os << std::setprecision(4);
        os << mmse.x << "," << mmse.y << "," << mmse.z << "]\n";

        os << "POS[t,tm,x,y,z]  [";
        os << std::fixed << std::setprecision(3);
        os << ts << ",";
        os << std::setprecision(2);
        os << mmse.time << ",";
        os << std::setprecision(4);
        os << pt.x << "," << pt.y << "," << pt.z << "]\n";

        os << "OFS[t,tm,x,y,z]  [";
        os << std::fixed << std::setprecision(3);
        os << ts << ",";
        os << std::setprecision(2);
        os << mmse.time << ",";
        os << std::setprecision(4);
        os << pt.x-mmse.x << "," << pt.y-mmse.y << "," << pt.z-mmse.z << "]\n";

        os << "COV[t,x,y,z]     [";
        os << std::setprecision(3);
        os << mmse.time << ",";
        os << std::setprecision(2);
        os << sqrt(mmse.covariance[0]) << ",";
        os << sqrt(mmse.covariance[2]) << ",";
        os << sqrt(mmse.covariance[5]) << "]\n";

        //    fprintf(stderr,"MLE[t,tx,y,z] [ %.3lf, %.2f , %.4f , %.4f , %.4f ]\n",time,mle.time,mle.x, mle.y, mle.z);
        //    fprintf(stderr,"MSE[t,x,y,z] [ %.3lf, %.2f , %.4f , %.4f , %.4f ]\n",time,mmse.time, mmse.x, mmse.y, mmse.z);
        //    fprintf(stderr,"COV[t,x,y,z] [ %.3lf, %.2f , %.2f , %.2f ]\n",
        //            mmse.time,
        //            sqrt(mmse.covariance[0]),
        //            sqrt(mmse.covariance[2]),
        //            sqrt(mmse.covariance[5]));

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
            os << std::setw(wval-9) << "[" << (mt.measStatus[i] ? 1 : 0) << ",";
            os << std::fixed << std::setprecision(2) << setw(6) << mt.ranges[i] << "]\n";
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

    int next_record(byte *dest, size_t len, TrnLog::TrnRecID &r_type)
    {
        int retval = -1;
        byte buf[5]={0}, *cur = buf;
        typedef enum{
            START,
            MTN, MEA,
            OK, EEOF, ERR
        }state_t;
        state_t stat = START;

        while(stat != OK && stat != EEOF && stat != ERR)
        {
            if(fread(cur,1,1,mFile) == 1)
            {
//                fprintf(stderr,"%c ", (*cur>0x20?*cur:'.'));
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
                        if(stat==MTN){
                            cur++;
                            stat = OK;
                            r_type = TrnLog::MOTN_OUT;
                        } else if(stat==MEA){
                            cur++;
                            stat = OK;
                            r_type = TrnLog::MEAS_OUT;
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
            retval = 0;
        }
        return retval;
    }

    int read_meas(measT **pdest, byte *src)
    {
        int retval = -1;
        // read data from file to buffer
        byte *bp = (byte *)src;
        size_t readlen = sizeof(meas_in_t);


//        fprintf(stderr,"%s:%d readlen[%zu]\n", __func__, __LINE__, readlen);

        if(fread(bp, readlen, 1, mFile) == 1)
        {
            bp += readlen;
            meas_in_t *measin =  (meas_in_t *)src;
            readlen = measin->num_meas * sizeof(meas_beam_t);

//            fprintf(stderr,"%s:%d      time [%.3lf]\n", __func__, __LINE__,measin->time);
//            fprintf(stderr,"%s:%d data_type [%d]\n", __func__, __LINE__,measin->data_type);
//            fprintf(stderr,"%s:%d         x [%.3lf]\n", __func__, __LINE__,measin->x);
//            fprintf(stderr,"%s:%d         y [%.3lf]\n", __func__, __LINE__,measin->y);
//            fprintf(stderr,"%s:%d         z [%.3lf]\n", __func__, __LINE__,measin->z);
//            fprintf(stderr,"%s:%d  ping_num [%d]\n", __func__, __LINE__,measin->ping_number);
//            fprintf(stderr,"%s:%d  num_meas [%d]\n", __func__, __LINE__,measin->num_meas);

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

                    meas_beam_t *beams = measin_beam_data(measin);
                    for(int i=0; i<dest->numMeas; i++)
                    {
                        dest->beamNums[i] = beams[i].beam_num;
                        dest->measStatus[i] = beams[i].status;
                        dest->ranges[i] = beams[i].range;
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

    int read_pose(poseT **pdest, byte *src)
    {
        int retval = -1;
        // read data from file to buffer
        byte *bp = (byte *)src;
        size_t readlen = sizeof(motn_in_t);
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

private:
    TrnLogConfig mConfig;
    TrnClient *mTrn;
    FILE *mFile;
    FILE *mCsvFile;
    bool mQuit;
};


class app_cfg
{
public:
    app_cfg()
    : mDebug(0), mVerbose(false), mAppCfg(), mInputList(), mTBConfig()
    {
    }
    ~app_cfg(){}

    void parse_args(int argc, char **argv, bool ignore_cfg=false)
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
            {"csv", required_argument, NULL, 0},
            {"console", no_argument, NULL, 0},
            {"noconsole", no_argument, NULL, 0},
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
                    // host
                    else if (strcmp("trn-host", options[option_index].name) == 0) {
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
                    // logdir
//                    else if (strcmp("logdir", options[option_index].name) == 0) {
//                        mLogDirStr=optarg;
//                    }
                    // trn-cfg
                    else if (strcmp("trn-cfg", options[option_index].name) == 0) {
                        mTBConfig.set_trn_cfg(std::string(optarg));
                    }
                    // trn-sensor
                    else if (strcmp("trn-sensor", options[option_index].name) == 0) {
                        int sensor=0;
                        if(sscanf(optarg,"%d", &sensor) == 1)
                            mTBConfig.set_trn_sensor(sensor);
                    }
                    // input
                    else if (strcmp("input", options[option_index].name) == 0) {
                        mInputList.push_back(std::string(optarg));
                    }
                    // cfg
                    else if (!ignore_cfg && strcmp("cfg", options[option_index].name) == 0) {
                        mAppCfg = std::string(optarg);
                    }
                    // console
                    else if (!ignore_cfg && strcmp("console", options[option_index].name) == 0) {
                        mTBConfig.set_console(true);
                    }
                    // noconsole
                    else if (!ignore_cfg && strcmp("noconsole", options[option_index].name) == 0) {
                        mTBConfig.set_console(false);
                    }
                    // server
                    else if (!ignore_cfg && strcmp("server", options[option_index].name) == 0) {
                        mTBConfig.set_server(true);
                    }
                    // noserver
                    else if (!ignore_cfg && strcmp("noserver", options[option_index].name) == 0) {
                        mTBConfig.set_server(false);
                    }
                    // csv
                    else if (!ignore_cfg && strcmp("csv", options[option_index].name) == 0) {
                        mTBConfig.set_csv(true);
                        mTBConfig.set_csv_path(std::string(optarg));
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
        char help_message[] = "\n TRN Bin Log Player\n";
        char usage_message[] = "\n use: trnbin-replay [options]\n"
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
//        " --logdir=s            : log directory\n"
        " --input=s              : specify input file path (may be used multiple times)\n"
        " --csv                  : write to CSV file\n"
        " --console              : enable output to console\n"
        " --noconsole            : disable output to console\n"
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

        if(NULL!=src && strlen(src)>0 ){
            char *obuf=NULL;
            char *wp = (char *)malloc(strlen(src)+1);
            char *sp = wp;
            memset(wp,0,strlen(src)+1);
            sprintf(wp,"%s",src);
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
                    for(int i=1;i<var_len;i++){
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
                    sprintf(rebuf,"%s%s%s",wp,val,pecpy);
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
        std::ifstream file(file_path);
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
                        continue;
                    }
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
                        sprintf(cmd_buf, "--%s%s%s", key,(strlen(etval)>0?"=":""),etval);
                        char dummy[]={'f','o','o','\0'};
                        char *cmdv[2]={dummy,cmd_buf};
                        TRN_NDPRINT(4,">>> cmd_buf[%s] cmdv[%p]\n",cmd_buf,&cmdv[0]);
                        parse_args(2,&cmdv[0]);
                        free(key);
                        free(val);
                        free(etval);
                        free(cmd_buf);
                    }else{
                        TRN_NDPRINT(4, ">>> [comment line]\n");
                    }
                    free(lcp);
                }
            }
            file.close();
        }
    }

    const TrnLogConfig &tb_config()
    {
        return mTBConfig;
    }
    std::list<std::string>::iterator input_first(){ return mInputList.begin();}
    std::list<std::string>::iterator input_last(){ return mInputList.end();}
    int debug(){return mDebug;}
    bool verbose(){return mVerbose;}
protected:
private:
    int mDebug;
    bool mVerbose;
    std::string mAppCfg;
    std::list<std::string> mInputList;
    TrnLogConfig mTBConfig;
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
    cfg.parse_args(argc, argv);

    // configure debug output
    trn_debug::get()->set_debug(cfg.debug());

    // get log player
    TrnLogPlayer tbplayer(cfg.tb_config());

    // play back log files
    std::list<string>::iterator it;
    for(it=cfg.input_first(); it!=cfg.input_last(); it++)
    {
        std::string input = *it;

        TRN_NDPRINT(1,"playing[%s]\n",input.c_str());
        tbplayer.play(input, &g_interrupt);

        if(g_interrupt){
            // stop for SIGINT (CTRL-C)
            tbplayer.quit();
            break;
        }
    }

    // release trn_debug resources;
    trn_debug::get(true);

    TRN_DPRINT("%s:%d done\n",__func__, __LINE__);
    return 0;
}
