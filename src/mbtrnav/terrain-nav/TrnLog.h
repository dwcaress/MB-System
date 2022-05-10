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
#pragma pack(push,1)
typedef struct motn_in_s
{
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
    double time;
    int data_type;
    double x;
    double y;
    double z;
    int ping_number;
    int num_meas;
    // beam data follows - use measin_beam_data()
    //meas_beam_t beams[num_meas];
}meas_in_t;
#pragma pack(pop)

class TrnLog: public DataLogWriter {

public:

    typedef enum {
        RT_INVALID = 0x0U,
        // MTNI
        MOTN_IN = 0x494e544dU,
        // MEAI
        MEAS_IN = 0x4941454dU,
        // MTNO
        MOTN_OUT = 0x4f4e544dU,
        // MEAO
        MEAS_OUT = 0x4f41454dU
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

protected:
 
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
#ifdef WITH_POSE_OUTPUTS
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
