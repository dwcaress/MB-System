////////////////////////////////////////////////////////////////////////////////////
//
// PURPOSE:  PositionLog Log Class for TRN.
// AUTHOR:   Henthorn.
// DATE:     01/30/19
// COMMENTS: 
//
////////////////////////////////////////////////////////////////////////////////////
//

#ifndef POSITIONLOG_H
#define POSITIONLOG_H

#define PositionLogName  "TRNPosition"
#define PositionMnemName "trn_pos"

#include "DataLogWriter.h"
#include "IntegerData.h"
#include "DoubleData.h"
#include "TimeTag.h"

class PositionLog: public DataLogWriter {

public:

  //////////////////////////////////////////////////////////////////////////////////
  // Constructor
  // [input] fileFormat: Specifies either DataLog::AsciiFormat or 
  //                     DataLog::BinaryFormat.
  //         logname   : Optional user-supplied filename. Uses the 
  //                     PositionLogName above by default.
  //         mnem      : Optional user-supplied mnemonic. Uses the 
  //                     PositionMnemName above by default. Cannot be NULL.
  // Examples:
  //    tlog = new PositionLog(DataLog::BinaryFormat)  // Creates TRNPosition.log
  //    plog = new PositionLog(DataLog::BinaryFormat, "poseT")  // Creates poseT.log
  //    mlog = new PositionLog(DataLog::BinaryFormat, "poseT", "pose")  // data mnem is "pose"
  // 
  PositionLog(DataLog::FileFormat fileFormat, const char* logname=PositionLogName,
              const char *mnem=PositionMnemName);

  virtual ~PositionLog();

  //////////////////////////////////////////////////////////////////////////////////
  // This log(poseT*) function takes the place of the setFields()
  // function. To log a the data in a poseT simply pass a
  // pointer to the poseT object. Avoids having to declare this
  // class a friend to a TRN class just to access the poseT data in
  // a TRN object. Simple and more direct.
  //
  // Note: One may still use the legacy "friend-class-setFields()"
  // design since DataLogWriter::write() still calls setFields(). 
  // Here, setFields() does nothing.
  //
  void log(poseT* pt);

protected:

  DoubleData *ptime;
  DoubleData *x, *y, *z;
  DoubleData *vx, *vy, *vz, *ve;
  DoubleData *vw_x, *vw_y, *vw_z;
  DoubleData *vn_x, *vn_y, *vn_z;
  DoubleData *wx, *wy, *wz;
  DoubleData *ax, *ay, *az;
  DoubleData *phi, *theta, *psi;
  IntegerData *dvlValid, *gpsValid, *bottomLock;

};

#endif
