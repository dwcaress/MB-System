////////////////////////////////////////////////////////////////////////////////
//
// PURPOSE:  TerrainNavLog Log Class for TRN.
// AUTHOR:   Henthorn.
// DATE:     02/05/19
// COMMENTS: 
//
////////////////////////////////////////////////////////////////////////////////
//
#include "structDefs.h"
#include "TerrainNavLog.h"

TerrainNavLog::TerrainNavLog(DataLog::FileFormat fileFormat, const char* logname,
  const char *mnem)
  : DataLogWriter(logname, fileFormat, AutoTimeStamp)
{
  char mnembuf[100];
  snprintf(mnembuf, sizeof(mnembuf), "%s.data", mnem);
  setMnemonic(mnembuf);

  // Nav data
  addField((_navTime = new DoubleData("ta.navTime")));
  _navTime->setLongName("Nav data timestamp");
  _navTime->setAsciiFormat("%14.4f"); 
  _navTime->setUnits("epoch seconds");
  addField((_navN    = new DoubleData("ta.navN")));
  addField((_navE    = new DoubleData("ta.navE")));
  addField((_depth   = new DoubleData("ta.depth")));
  addField((_roll  = new DoubleData("ta.roll")));
  addField((_pitch = new DoubleData("ta.pitch")));
  addField((_yaw   = new DoubleData("ta.yaw")));

  // Measurement data
  addField((_measTime = new DoubleData("ta.measTime")));
  _measTime->setLongName("Measurement data timestamp");
  _measTime->setAsciiFormat("%14.4f"); 
  _measTime->setUnits("epoch seconds");
  addField((_dataType   = new IntegerData("ta.dataType")));
  addField((_pingNumber = new IntegerData("ta.pingNumber")));
  addField((_numMeas    = new IntegerData("ta.numMeas")));
  addField((_goodBeams  = new IntegerData("ta.goodBeams")));

  for (int i = 0; i < TRN_MAX_BEAMS; i++)
  {
    snprintf(mnembuf, sizeof(mnembuf), "ta.range_%02d", i);
    addField((_ranges[i]  = new DoubleData(mnembuf)));
  }

  for (int i = 0; i < TRN_MAX_BEAMS; i++)
  {
    snprintf(mnembuf, sizeof(mnembuf), "ta.crossTrack_%02d", i);
    addField((_crossTrack[i]  = new DoubleData(mnembuf)));
  }

  for (int i = 0; i < TRN_MAX_BEAMS; i++)
  {
    snprintf(mnembuf, sizeof(mnembuf), "ta.alongTrack_%02d", i);
    addField((_alongTrack[i]  = new DoubleData(mnembuf)));
  }

  for (int i = 0; i < TRN_MAX_BEAMS; i++)
  {
    snprintf(mnembuf, sizeof(mnembuf), "ta.altitudes_%02d", i);
    addField((_altitudes[i]  = new DoubleData(mnembuf)));
  }

  for (int i = 0; i < TRN_MAX_BEAMS; i++)
  {
    snprintf(mnembuf, sizeof(mnembuf), "ta.status_%02d", i);
    addField((_status[i]  = new IntegerData(mnembuf)));
  }

  for (int i = 0; i < TRN_MAX_BEAMS; i++)
  {
    snprintf(mnembuf, sizeof(mnembuf), "ta.beamNum_%02d", i);
    addField((_beamNums[i]  = new IntegerData(mnembuf)));
  }

  // MMSE data
  addField((_mmseTime = new DoubleData("ta.mmseTime")));
  _mmseTime->setLongName("Estimate timestamp");
  _mmseTime->setAsciiFormat("%14.4f"); 
  _mmseTime->setUnits("epoch seconds");
  addField((_mmseN    = new DoubleData("ta.mmseN")));
  addField((_mmseE    = new DoubleData("ta.mmseE")));
  addField((_mmseZ    = new DoubleData("ta.mmseZ")));
  addField((_mmseVarN = new DoubleData("ta.mmseVarN")));
  addField((_mmseVarE = new DoubleData("ta.mmseVarE")));
  addField((_mmseVarZ = new DoubleData("ta.mmseVarZ")));
  addField((_mmsePhi  = new DoubleData("ta.mmseVarPhi")));
  addField((_mmseTheta = new DoubleData("ta.mmseVarTheta")));
  addField((_mmsePsi  = new DoubleData("ta.mmseVarPsi")));
  addField((_mmsePsiBerg     = new DoubleData("ta.mmsePsiBerg")));
  addField((_mmseVarPsiBerg  = new DoubleData("ta.mmseVarPsiBerg")));

  // MMSE data
  addField((_mleN    = new DoubleData("ta.mleN")));
  addField((_mleE    = new DoubleData("ta.mleE")));
  addField((_mleZ    = new DoubleData("ta.mleZ")));
  addField((_mlePsiBerg    = new DoubleData("ta.mlePsiBerg")));
  addField((_mleVarPsiBerg = new DoubleData("ta.mleVarPsiBerg")));

  addField((_numReinits = new IntegerData("ta.numReinits")));
  
}


TerrainNavLog::~TerrainNavLog()
{
  if(_navTime) delete _navTime;
  if(_navN)    delete _navN;
  if(_navE)    delete _navE;
  if(_depth)   delete _depth;
  if(_roll)    delete _roll;
  if(_pitch)   delete _pitch;
  if(_yaw)     delete _yaw;

  if (_measTime) delete _measTime;
  if (_dataType) delete _dataType;
  if (_pingNumber) delete _pingNumber;
  if (_numMeas) delete _numMeas;
  if (_goodBeams) delete _goodBeams;
  for (int i = 0; i < TRN_MAX_BEAMS; i++)
  {
    if (_ranges[i]) delete _ranges[i];
    if (_crossTrack[i]) delete _crossTrack[i];
    if (_alongTrack[i]) delete _alongTrack[i];
    if (_altitudes[i]) delete _altitudes[i];
    if (_status[i]) delete _status[i];
    if (_beamNums[i]) delete _beamNums[i];
  }

  if (_mmseN) delete _mmseN;
  if (_mmseE) delete _mmseE;
  if (_mmseZ) delete _mmseZ;
  if (_mmseVarN) delete _mmseVarN;
  if (_mmseVarE) delete _mmseVarE;
  if (_mmseVarZ) delete _mmseVarZ;
  if (_mmsePhi) delete _mmsePhi;
  if (_mmseTheta) delete _mmseTheta;
  if (_mmsePsi) delete _mmsePsi;
  if (_mmseTime) delete _mmseTime;
  if (_mmsePsiBerg) delete _mmsePsiBerg;
  if (_mmseVarPsiBerg) delete _mmseVarPsiBerg;

  if (_mleN) delete _mleN;
  if (_mleE) delete _mleE;
  if (_mleZ) delete _mleZ;
  if (_mlePsiBerg) delete _mlePsiBerg;
  if (_mleVarPsiBerg) delete _mleVarPsiBerg;
  if (_numReinits) delete _numReinits;

}

// These log functions takes the place of the setFields()
// function. To log a the data in a poseT simply pass a
// pointer to the poseT object. Avoids declaring this class
// as a friend to a TRN class just to access the poseT in
// a TRN object. This method also allows any process 
//
// Note: One may still use the "friend-class-setFields()"
// method.
//

// Grab the data from the MLE estimate
//
void TerrainNavLog::logMLE(poseT* pt)
{
  if (pt)
  {
    _mleN->setValue(pt->x);
    _mleE->setValue(pt->y);
    _mleZ->setValue(pt->z);
    _mlePsiBerg->setValue(pt->psi_berg);
    _mleVarPsiBerg->setValue(pt->covariance[44]);
  }
}

// Grab the Nav data:
//
void TerrainNavLog::logNav(poseT* pt)
{
  if (pt)
  {
     _navTime->setValue(pt->time);
     _navN->setValue(pt->x);
     _navE->setValue(pt->y);
     _depth->setValue(pt->z);
     _roll-> setValue(pt->phi);
     _pitch->setValue(pt->theta);
     _yaw->  setValue(pt->psi);
  }
}

// Grab the meas data:
//
void TerrainNavLog::logMeas(measT* mt)
{
  int goodBeams = 0;
  if (mt)
  {
    _dataType->setValue(abs(mt->dataType));
    _measTime->setValue(mt->time);
    _numMeas->setValue(mt->numMeas);
    _pingNumber->setValue(mt->ping_number);
  }
  for (int i = 0; i < _numMeas->value(); i++)
  {
    _ranges[i]->setValue(mt->ranges[i]);
    _status[i]->setValue((mt->measStatus[i]? 1 : 0));

    if (mt->measStatus[i]) goodBeams++;

    if (_dataType->value() == TRN_SENSOR_MB)
    {
        if(NULL != mt->crossTrack)
            _crossTrack[i]->setValue(mt->crossTrack[i]);
        if(NULL != mt->alongTrack)
            _alongTrack[i]->setValue(mt->alongTrack[i]);
        if(NULL != mt->altitudes)
            _altitudes[i]->setValue(mt->altitudes[i]);
        if(NULL != mt->beamNums)
            _beamNums[i]->setValue(mt->beamNums[i]);
    }
  }
  _goodBeams->setValue(goodBeams);
}

void TerrainNavLog::logReinits(double numReinits)
{
   _numReinits->setValue(numReinits);
}

// Grab the data from the MMSE estimate
//
void TerrainNavLog::logMMSE(poseT* pt)
{
  if (pt)
  {
    _mmseTime->setValue(pt->time);
    _mmseN->setValue(pt->x);
    _mmseE->setValue(pt->y);
    _mmseZ->setValue(pt->z);
    _mmseVarN->setValue(pt->covariance[0]);
    _mmseVarE->setValue(pt->covariance[2]);
    _mmseVarZ->setValue(pt->covariance[5]);
    _mmsePhi->setValue(pt->phi);
    _mmseTheta->setValue(pt->theta);
    _mmsePsi->setValue(pt->psi);
    _mmsePsiBerg->    setValue(pt->psi_berg);
    _mmseVarPsiBerg-> setValue(pt->covariance[44]);
  }

}

