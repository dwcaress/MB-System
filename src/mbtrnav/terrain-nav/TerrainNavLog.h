////////////////////////////////////////////////////////////////////////////////////
//
// PURPOSE:  TerrainNavLog Log Class for TRN.
// AUTHOR:   Henthorn.
// DATE:     01/30/19
// COMMENTS: 
//
////////////////////////////////////////////////////////////////////////////////////
//

#ifndef TerrainNavLog_H
#define TerrainNavLog_H

#define TerrainNavLogName  "TerrainNav"
#define TerrainNavMnemName "ta"

#include "DataLogWriter.h"
#include "IntegerData.h"
#include "DoubleData.h"
#include "TimeTag.h"
#include "structDefs.h"

class TerrainNavLog: public DataLogWriter {

public:

  //////////////////////////////////////////////////////////////////////////////////
  // Constructor
  // [input] fileFormat: Specifies either DataLog::AsciiFormat or 
  //                     DataLog::BinaryFormat.
  //         logname   : Optional user-supplied filename. Uses the 
  //                     TerrainNavLogName above by default.
  //         mnem      : Optional user-supplied mnemonic. Uses the 
  //                     TerrainNavMnemName above by default. Cannot be NULL.
  // Examples:
  //    tlog = new TerrainNavLog(DataLog::BinaryFormat)  // Creates TerrainNav.log
  //    plog = new TerrainNavLog(DataLog::BinaryFormat, "poseT")  // Creates poseT.log
  //    mlog = new TerrainNavLog(DataLog::BinaryFormat, "poseT", "pose")  // data mnem is "pose"
  // 
  explicit TerrainNavLog(DataLog::FileFormat fileFormat, const char* logname=TerrainNavLogName,
              const char *mnem=TerrainNavMnemName);

  ~TerrainNavLog();

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
  void logMLE(poseT* mle);
  void logMMSE(poseT* mmse);
  void logNav(poseT* nav);
  void logMeas(measT* meas);
  void logReinits(double numReinits);

protected:

  // Vehicle position and orientation data input from IMUs
  // (e.g. DVL, Kearfott)
  //
  DoubleData *_navTime;
  DoubleData *_navN;
  DoubleData *_navE;
  DoubleData *_depth;
  DoubleData *_roll;
  DoubleData *_pitch;
  DoubleData *_yaw;

  // Range/measurement data input from sensing instruments
  // (e.g., sonars, lidars)
  //
  IntegerData *_dataType;           // Instrument code (1=>DVL, 2=>Multibeam, etc.)
  IntegerData *_pingNumber;
  IntegerData *_numMeas;
  IntegerData *_goodBeams;
  DoubleData  *_measTime;
  DoubleData  *_ranges[TRN_MAX_BEAMS];
  DoubleData  *_crossTrack[TRN_MAX_BEAMS];
  DoubleData  *_alongTrack[TRN_MAX_BEAMS];
  DoubleData  *_altitudes[TRN_MAX_BEAMS];
  IntegerData *_status[TRN_MAX_BEAMS];
  IntegerData *_beamNums[TRN_MAX_BEAMS];

  // TRN estimated position
  // Minimum Mean Square Estimation
  //
  DoubleData *_mmseTime;
  DoubleData *_mmseN;
  DoubleData *_mmseE;
  DoubleData *_mmseZ;
  DoubleData *_mmseVarN;
  DoubleData *_mmseVarE;
  DoubleData *_mmseVarZ;
  DoubleData *_mmsePhi;
  DoubleData *_mmseTheta;
  DoubleData *_mmsePsi;
  DoubleData *_mmsePsiBerg;
  DoubleData *_mmseVarPsiBerg;

  // Maximum Likelihood Estimation
  //
  DoubleData *_mleN;
  DoubleData *_mleE;
  DoubleData *_mleZ;
  DoubleData *_mlePsiBerg;
  DoubleData *_mleVarPsiBerg;

  IntegerData *_numReinits;
  

};

#endif
