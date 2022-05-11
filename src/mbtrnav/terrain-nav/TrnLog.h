////////////////////////////////////////////////////////////////////////////////////
//
// PURPOSE:  TrnLog Log Class for TRN.
// AUTHOR:   k headley
// DATE:     2022-05-05
// COMMENTS: 
//
////////////////////////////////////////////////////////////////////////////////////
//

#ifndef TRNLOG_H
#define TRNLOG_H

#define TrnLogName  "TrnBin"
#define TrnMnemName "trn"

#include "DataLogWriter.h"
#include "IntegerData.h"
#include "DoubleData.h"
#include "ShortData.h"
#include "TimeTag.h"
#include "structDefs.h"

#define TRN_MAX_BEAMS_DFL TRN_MAX_BEAMS
// size of record ID
#define TL_RID_SIZE sizeof(uint32_t)
// size of motn_in w/o record id
#define TL_MTNI_SIZE (sizeof(motn_in_t)-TL_RID_SIZE)
// size of meas_in w/o record id
#define TL_MEAI_HDR_SIZE (sizeof(meas_in_t)-TL_RID_SIZE)
// size of beam data for n beams
#define TL_MEAI_BEAM_SIZE(n) (sizeof(meas_beam_t) * n)
// size of mse_out w/o record id
#define TL_MSEO_SIZE (sizeof(est_out_t)-TL_RID_SIZE)

#pragma pack(push,1)
typedef struct motn_in_s
{
    uint32_t rec_id;
    double time;
    double x;
    double y;
    double z;
    double vx;
    double vy;
    double vz;
    double phi;
    double theta;
    double psi;
    short int dvl_valid;
    short int gps_valid;
    short int bottom_lock;
}motn_in_t;

typedef struct meas_beam_s{
    short int beam_num;
    short int status;
    double range;
}meas_beam_t;

typedef struct meas_in_s
{
    uint32_t rec_id;
    double time;
    int data_type;
    double x;
    double y;
    double z;
    int ping_number;
    int num_meas;
    // beam data follows; measin_beam_data() returns pointer
    //meas_beam_t beams[num_meas];
}meas_in_t;

typedef struct est_out_s
{
    uint32_t rec_id;
    double time;
    double x;
    double y;
    double z;
    double vx;
    double vy;
    double vz;
    double ve;
    double vw_x;
    double vw_y;
    double vw_z;
    double vn_x;
    double vn_y;
    double vn_z;
    double wx;
    double wy;
    double wz;
    double ax;
    double ay;
    double az;
    double phi;
    double theta;
    double psi;
    double psi_berg;
    double psi_dot_berg;
    short int dvl_valid;
    short int gps_valid;
    short int bottom_lock;
    double covariance[N_COVAR];
}est_out_t;

#pragma pack(pop)

class TrnLog: public DataLogWriter {

public:

    typedef enum {
        RT_INVALID = 0x0U,
        // MTNI
        MOTN_IN = 0x494E544DU,
        // MEAI
        MEAS_IN = 0x4941454DU,
        // MSEO
        MSE_OUT = 0x4F45534DU,
        // MLEO
        MLE_OUT = 0x4F454C4DU
    }TrnRecID;
    //////////////////////////////////////////////////////////////////////////////////
    // Constructor
    // [input] fileFormat: Specifies either DataLog::AsciiFormat or
    //                     DataLog::BinaryFormat.
    //         logname   : Optional user-supplied filename. Uses the
    //                     TrnLogName above by default.
    //         mnem      : Optional user-supplied mnemonic. Uses the
    //                     TerrainNavMnemName above by default. Cannot be NULL.
    // Examples:
    //    tlog = new TrnLog(DataLog::BinaryFormat)  // Creates TerrainNav.log
    //    plog = new TrnLog(DataLog::BinaryFormat, "poseT")  // Creates poseT.log
    //    mlog = new TrnLog(DataLog::BinaryFormat, "poseT", "pose")  // data mnem is "pose"
    //
    explicit TrnLog(DataLog::FileFormat fileFormat, const char* logname=TrnLogName,
                    const char *mnem=TrnMnemName, uint32_t max_beams=TRN_MAX_BEAMS_DFL);

    ~TrnLog();

    //////////////////////////////////////////////////////////////////////////////////
    // These log(poseT*) functions takes the place of the setFields()
    // function. To log a the data in a poseT simply pass a
    // pointer to the poseT object. Avoids having to declare this
    // class a friend to a TRN class just to access the poseT data in
    // a TRN object. Simple and more direct.
    //
    // Note: One may still use the legacy "friend-class-setFields()"
    // design since DataLogWriter::write() still calls setFields().
    // Here, setFields() does nothing.
    //
    void logMeas(measT* meas, TrnRecID recID);
    void logMotn(poseT* pose, TrnRecID recID);
#ifdef WITH_TRNLOG_EST_OUT
    void logEst(poseT* pose, TrnRecID recID);
#endif
    static meas_beam_t *meaiBeamData(meas_in_t *self);

protected:
    void writeField(FILE *file, DataField *field);
    void writeHeader();
    int pre_write();

    IntegerData *_recordID;
    // Vehicle position and orientation data input from IMUs
    // (e.g. DVL, Kearfott)
    //
    DoubleData *_ptTime;
    DoubleData *_ptX;
    DoubleData *_ptY;
    DoubleData *_ptZ;
    DoubleData *_ptVx;
    DoubleData *_ptVy;
    DoubleData *_ptVz;
    DoubleData *_ptPhi;
    DoubleData *_ptTheta;
    DoubleData *_ptPsi;
    ShortData *_ptDvlValid;
    ShortData *_ptGpsValid;
    ShortData *_ptBottomLock;
#ifdef WITH_TRNLOG_EST_OUT
    DoubleData *_ptVe;
    DoubleData *_ptVwx;
    DoubleData *_ptVwy;
    DoubleData *_ptVwz;
    DoubleData *_ptVnx;
    DoubleData *_ptVny;
    DoubleData *_ptVnz;
    DoubleData *_ptWx;
    DoubleData *_ptWy;
    DoubleData *_ptWz;
    DoubleData *_ptAx;
    DoubleData *_ptAy;
    DoubleData *_ptAz;
    DoubleData *_ptPsiBerg;
    DoubleData *_ptPsiDotBerg;
    DoubleData *_ptCovariance[N_COVAR];
#endif
    // Range/measurement data input from sensing instruments
    // (e.g., sonars, lidars)
    //
    // Instrument code (1=>DVL, 2=>Multibeam, etc.)
    DoubleData  *_mtTime;
    IntegerData *_mtDataType;
    DoubleData *_mtX;
    DoubleData *_mtY;
    DoubleData *_mtZ;
    IntegerData *_mtPingNumber;
    IntegerData *_mtNumMeas;
    DoubleData  **_mtRanges;
    ShortData **_mtStatus;
    ShortData **_mtBeamNums;
#ifdef WITH_MEAS_OUTPUTS
    DoubleData **_mtCovariance;
    DoubleData **_mtCrosstrack;
    DoubleData **_mtAlongtrack;
    DoubleData **_mtAltitudes;
    DoubleData **_mtAlphas;
#endif

    uint32_t _max_beams;

};

#ifdef __cplusplus
extern "C" {
#endif
//meas_in_t *measin_new(int num_meas);
//measin_destroy(meas_in_t **pself);
meas_beam_t *measin_beam_data(meas_in_t *self);
size_t measin_size(meas_in_t *self);
#ifdef __cplusplus
}
#endif


#endif
