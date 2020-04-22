////////////////////////////////////////////////////////////////////////////////////
//
// PURPOSE:  TNavPFLog Log Class for TRN.
// AUTHOR:   Henthorn.
// DATE:     05/20/2019
// COMMENTS: 
//
////////////////////////////////////////////////////////////////////////////////////
//

#ifndef TNavPFLog_H
#define TNavPFLog_H

#define TNAVPF_LOGFILE_NAMELEN 100
#define TNAVPF_MNEM_NAMELEN    50
#define TNavPFLogName  "TNavPFLog"
#define TNavPFMnemName "tnpf"
#define TNavBFLogName  "TNavBFLog"
#define TNavBFMnemName "tnbf"


#include "structDefs.h"
#include "DataLogWriter.h"
#include "IntegerData.h"
#include "DoubleData.h"
#include "TimeTag.h"

class TNavPFLog: public DataLogWriter {

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
  //    tlog = new TNavLog(DataLog::BinaryFormat)  // Creates TNavFilter.log
  //    plog = new TNavLog(DataLog::BinaryFormat, "TNF")  // Creates TNF.log
  //    mlog = new TNavLog(DataLog::BinaryFormat, "TNF", "tnav")  // data mnem is "tnav"
  // 
  TNavPFLog(DataLog::FileFormat fileFormat, const char* logname=TNavPFLogName,
              const char *mnem=TNavPFMnemName);

  ~TNavPFLog();

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

  void setSoundings(int soundings)
  {
    _soundings->setValue(soundings);
    _dirty = 1;
  }

  void setUsedBeams(int usedBeams)
  {
    _usedBeams->setValue(usedBeams);
    _dirty = 1;
  }

  void setSubcloudNIS(double nis)
  {
    _subcloudNIS->setValue(nis);
    _dirty = 1;
  }

  void setSubcloudCounts(unsigned i, int counts)
  {
    if (i < TRN_MAX_BEAMS) _subcloudCounts[i]->setValue(counts);
    _dirty = 1;
  }

  void setMeanExpMeasDif(unsigned i, double diff)
  {
    if (i < TRN_MAX_BEAMS) _meanExpMeasDif[i]->setValue(diff);
    _dirty = 1;
  }

  void setAlpha(unsigned i, double alpha)
  {
    if (i < TRN_MAX_BEAMS) _alpha[i]->setValue(alpha);
    _dirty = 1;
  }

  void setSumSquaredError(double error)
  {
    _sumSquaredError->setValue(error);
    _dirty = 1;
  }

  void setSumWeights(double sum)
  {
    _sumWeights->setValue(sum);
    _dirty = 1;
  }

  int write()
  {
    if (_dirty)
    {
      _dirty = 0;
      return DataLogWriter::write();
    }
    return 0;
  }

protected:

  char _dirty;
  IntegerData *_soundings;
  IntegerData *_usedBeams;
  DoubleData  *_subcloudNIS;
  DoubleData  *_sumWeights;
  DoubleData  *_sumSquaredError;
  IntegerData *_subcloudCounts[TRN_MAX_BEAMS];
  DoubleData  *_meanExpMeasDif[TRN_MAX_BEAMS];
  DoubleData  *_alpha[TRN_MAX_BEAMS];

};

#endif
