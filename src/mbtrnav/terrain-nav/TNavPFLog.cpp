////////////////////////////////////////////////////////////////////////////////
//
// PURPOSE:  TNavPFLog Log Class for TRN.
// AUTHOR:   Henthorn.
// DATE:     05/20/2019
// COMMENTS: 
//
////////////////////////////////////////////////////////////////////////////////
//
#include "TNavPFLog.h"

TNavPFLog::TNavPFLog(DataLog::FileFormat fileFormat, const char* logname,
              const char *mnem)
  : DataLogWriter(logname, fileFormat, AutoTimeStamp), _dirty(0)
{
  char mnembuf[100];
  snprintf(mnembuf, sizeof(mnembuf), "%s.data", mnem);
  setMnemonic(mnembuf);

  snprintf(mnembuf, sizeof(mnembuf), "%s.soundings", mnem);
  addField((_soundings    = new IntegerData(mnembuf)));
  snprintf(mnembuf, sizeof(mnembuf), "%s.usedBeams", mnem);
  addField((_usedBeams    = new IntegerData(mnembuf)));
  snprintf(mnembuf, sizeof(mnembuf), "%s.subcloudNIS", mnem);
  addField((_subcloudNIS  = new DoubleData(mnembuf)));
  snprintf(mnembuf, sizeof(mnembuf), "%s.sumWeights", mnem);
  addField((_sumWeights  = new DoubleData(mnembuf)));
  snprintf(mnembuf, sizeof(mnembuf), "%s.sumSquaredError", mnem);
  addField((_sumSquaredError  = new DoubleData(mnembuf)));

  for (int i = 0; i < TRN_MAX_BEAMS; i++)
  {
    snprintf(mnembuf, sizeof(mnembuf), "%s.subcloudNIS_%02d", mnem, i);
    addField((_subcloudCounts[i]  = new IntegerData(mnembuf)));
  }

  for (int i = 0; i < TRN_MAX_BEAMS; i++)
  {
    snprintf(mnembuf, sizeof(mnembuf), "%s.meanExpMeasDif_%02d", mnem, i);
    addField((_meanExpMeasDif[i]  = new DoubleData(mnembuf)));
  }

  for (int i = 0; i < TRN_MAX_BEAMS; i++)
  {
    snprintf(mnembuf, sizeof(mnembuf), "%s.alpha_%02d", mnem, i);
    addField((_alpha[i]  = new DoubleData(mnembuf)));
  }

}

TNavPFLog::~TNavPFLog()
{
  if (_soundings) delete _soundings;
	if (_usedBeams) delete _usedBeams;
	if (_subcloudNIS) delete _subcloudNIS;
  for (int i = 0; i < TRN_MAX_BEAMS; i++)
  {
    if (_subcloudCounts[i]) delete _subcloudCounts[i];
    if (_meanExpMeasDif[i]) delete _meanExpMeasDif[i];
    if (_alpha[i]) delete _alpha[i];
  }
    if (_sumWeights) delete _sumWeights;
    if (_sumSquaredError) delete _sumSquaredError;
}

