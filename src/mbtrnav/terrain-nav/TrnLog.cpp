////////////////////////////////////////////////////////////////////////////////
//
// PURPOSE:  TrnLog Log Class for TRN.
// AUTHOR:   k headley
// DATE:     2022-05-05
// COMMENTS: 
//
////////////////////////////////////////////////////////////////////////////////
//
#include "structDefs.h"
#include "TrnLog.h"

TrnLog::TrnLog(DataLog::FileFormat fileFormat, const char* logname,
               const char *mnem, uint32_t max_beams)
: DataLogWriter(logname, fileFormat, AutoTimeStamp), _mtRanges(NULL), _mtStatus(NULL), _mtBeamNums(NULL), _max_beams(max_beams)
{
    char mnembuf[100] = {0};
    snprintf(mnembuf, sizeof(mnembuf), "%s.data", mnem);
    setMnemonic(mnembuf);

    _mtRanges = new DoubleData *[max_beams];
    _mtStatus = new ShortData *[max_beams];
    _mtBeamNums = new ShortData *[max_beams];

    addField((_recordID = new IntegerData("trn.recordID")));

    // motion update (poseT) data
    addField((_ptTime = new DoubleData("trn.ptTime")));
    _ptTime->setLongName("Nav data timestamp");
    _ptTime->setAsciiFormat("%14.4f");
    _ptTime->setUnits("epoch seconds");
    addField((_ptX = new DoubleData("trn.ptX")));
    addField((_ptY = new DoubleData("trn.ptY")));
    addField((_ptZ = new DoubleData("trn.ptZ")));
    addField((_ptVx = new DoubleData("trn.ptVx")));
    addField((_ptVy = new DoubleData("trn.ptVy")));
    addField((_ptVz = new DoubleData("trn.ptVz")));
    addField((_ptPhi = new DoubleData("trn.ptPhi")));
    addField((_ptTheta = new DoubleData("trn.ptTheta")));
    addField((_ptPsi = new DoubleData("trn.ptPsi")));
    addField((_ptDvlValid = new ShortData("trn.ptDvlValid")));
    addField((_ptGpsValid = new ShortData("trn.ptGpsValid")));
    addField((_ptBottomLock = new ShortData("trn.ptBottomLock")));

#ifdef WITH_POSE_OUTPUTS
    addField((_ptVe = new DoubleData("trn.ptVe")));
    addField((_ptVwx = new DoubleData("trn.ptVwx")));
    addField((_ptVwy = new DoubleData("trn.ptvwy")));
    addField((_ptVwz = new DoubleData("trn.ptVwz")));
    addField((_ptVnx = new DoubleData("trn.ptVnx")));
    addField((_ptVny = new DoubleData("trn.ptvny")));
    addField((_ptVnz = new DoubleData("trn.ptVnz")));
    addField((_ptWx = new DoubleData("trn.ptWx")));
    addField((_ptWy = new DoubleData("trn.ptWy")));
    addField((_ptWz = new DoubleData("trn.ptWz")));
    addField((_ptAx = new DoubleData("trn.ptAx")));
    addField((_ptAy = new DoubleData("trn.ptAy")));
    addField((_ptAz = new DoubleData("trn.ptAz")));
    addField((_ptPsiBerg = new DoubleData("trn.ptPsiBerg")));
    addField((_ptPsiDotBerg = new DoubleData("trn.ptPsiDotBerg")));
    for (int i = 0; i < N_COVAR; i++)
    {
        snprintf(mnembuf, sizeof(mnembuf), "trn.ptCovariance_%02d", i);
        addField((_ptCovariance[i]  = new DoubleData(mnembuf)));
    }
#endif

    // Measurement data
    addField((_mtTime = new DoubleData("trn.mtTime")));
    _mtTime->setLongName("Measurement data timestamp");
    _mtTime->setAsciiFormat("%14.4f");
    _mtTime->setUnits("epoch seconds");

    addField((_mtDataType = new IntegerData("trn.mtDataType")));
    addField((_mtX = new DoubleData("trn.mtX")));
    addField((_mtY = new DoubleData("trn.mtY")));
    addField((_mtZ = new DoubleData("trn.mtZ")));
    addField((_mtPingNumber = new IntegerData("trn.mtPingNumber")));
    addField((_mtNumMeas = new IntegerData("trn.mtNumMeas")));

    for (int i = 0; i < _max_beams; i++)
    {
        snprintf(mnembuf, sizeof(mnembuf), "trn.mtRange_%02d", i);
        addField((_mtRanges[i]  = new DoubleData(mnembuf)));
    }

    for (int i = 0; i < _max_beams; i++)
    {
        snprintf(mnembuf, sizeof(mnembuf), "trn.mtStatus_%02d", i);
        addField((_mtStatus[i]  = new ShortData(mnembuf)));
    }

    for (int i = 0; i < _max_beams; i++)
    {
        snprintf(mnembuf, sizeof(mnembuf), "trn.mtBeamNum_%02d", i);
        addField((_mtBeamNums[i]  = new ShortData(mnembuf)));
    }
#ifdef WITH_MEAS_OUTPUTS
    _mtCrosstrack = new DoubleData *[max_beams];
    _mtAlongtrack = new DoubleData *[max_beams];
    _mtAltitudes = new DoubleData *[max_beams];
    _mtAlphas = new DoubleData *[max_beams];

    addField((_mtCovariance = new DoubleData("trn.mtCovariance")));

    for (int i = 0; i < _max_beams; i++)
    {
        snprintf(mnembuf, sizeof(mnembuf), "trn.mtCrosstrack_%02d", i);
        addField((_mtCrosstrack[i]  = new DoubleData(mnembuf)));
        snprintf(mnembuf, sizeof(mnembuf), "trn.mtAlongtrack_%02d", i);
        addField((_mtAlongtrack[i]  = new DoubleData(mnembuf)));
        snprintf(mnembuf, sizeof(mnembuf), "trn.mtAltitudes_%02d", i);
        addField((_mtAltitudes[i]  = new DoubleData(mnembuf)));
        snprintf(mnembuf, sizeof(mnembuf), "trn.mtAlphas_%02d", i);
        addField((_mtAlphas[i]  = new DoubleData(mnembuf)));
    }
#endif

}


TrnLog::~TrnLog()
{
    if(_ptTime) delete _ptTime;
    if(_ptX) delete _ptX;
    if(_ptY) delete _ptY;
    if(_ptZ) delete _ptZ;
    if(_ptVx) delete _ptVx;
    if(_ptVy) delete _ptVy;
    if(_ptVz) delete _ptVz;
    if(_ptPhi) delete _ptPhi;
    if(_ptTheta) delete _ptTheta;
    if(_ptPsi) delete _ptPsi;
    if(_ptDvlValid) delete _ptDvlValid;
    if(_ptGpsValid) delete _ptGpsValid;
    if(_ptBottomLock) delete _ptBottomLock;
#ifdef WITH_POSE_OUTPUTS
    if(_ptVe) delete _ptVe;
    if(_ptVwx) delete _ptVwx;
    if(_ptVwy) delete _ptVwy;
    if(_ptVwz) delete _ptVwz;
    if(_ptVnx) delete _ptVnx;
    if(_ptVny) delete _ptVny;
    if(_ptVnz) delete _ptVnz;
    if(_ptWx) delete _ptWx;
    if(_ptWy) delete _ptWy;
    if(_ptWz) delete _ptWz;
    if(_ptAx) delete _ptAx;
    if(_ptAy) delete _ptAy;
    if(_ptAz) delete _ptAz;
    if(_ptPsiBerg) delete _ptPsiBerg;
    if(_ptPsiDotBerg) delete _ptPsiDotBerg;

    for (int i = 0; i < N_COVAR; i++)
    {
        if (_ptCovariance[i]) delete _ptCovariance[i];
    }
#endif

    if (_mtTime) delete _mtTime;
    if (_mtDataType) delete _mtDataType;
    if(_mtX) delete _mtX;
    if(_mtY) delete _mtY;
    if(_mtZ) delete _mtZ;
    if (_mtPingNumber) delete _mtPingNumber;
    if (_mtNumMeas) delete _mtNumMeas;
    for (int i = 0; i < _max_beams; i++)
    {
        if (_mtRanges[i]) delete _mtRanges[i];
        if (_mtStatus[i]) delete _mtStatus[i];
        if (_mtBeamNums[i]) delete _mtBeamNums[i];
    }

    delete [] _mtRanges;
    delete [] _mtStatus;
    delete [] _mtBeamNums;

#ifdef WITH_MEAS_OUTPUTS
    for (int i = 0; i < _max_beams; i++)
    {
        if (_mtCovariance[i]) delete _mtCovariance[i];
        if (_mtCrosstrack[i]) delete _mtCrosstrack[i];
        if (_mtAlongtrack[i]) delete _mtAlongtrack[i];
        if (_mtAltitudes[i]) delete _mtAltitudes[i];
        if (_mtAlphas[i]) delete _mtAlphas[i];
    }
    delete [] _mtCovariance;
    delete [] _mtCrosstrack;
    delete [] _mtAlongtrack;
    delete [] _mtAltitudes;
    delete [] _mtAlphas;
#endif

}

// These log functions takes the place of the setFields()
// function. To log a the data in a poseT, pass a
// pointer to the poseT object. Avoids declaring this class
// as a friend to a TRN class just to access the poseT in
// a TRN object. This method also allows any process 
//
// Note: One may still use the "friend-class-setFields()"
// method.
//

// log measurement update
void TrnLog::logMotn(poseT* pt, TrnLog::TrnRecID recID)
{
    if (pt && (recID==MOTN_IN || recID==MOTN_OUT))
    {
        if(pre_write() != 0)
            return;

        _recordID->setValue(recID);
        _recordID->write(_logFile);

        _ptTime->setValue(pt->time);
        _ptTime->write(_logFile);

        _ptX->setValue(pt->x);
        _ptX->write(_logFile);

        _ptY->setValue(pt->y);
        _ptY->write(_logFile);

        _ptZ->setValue(pt->z);
        _ptZ->write(_logFile);

        _ptVx->setValue(pt->vx);
        _ptVx->write(_logFile);

        _ptVy->setValue(pt->vy);
        _ptVy->write(_logFile);

        _ptVz->setValue(pt->vz);
        _ptVz->write(_logFile);

        _ptPhi->setValue(pt->phi);
        _ptPhi->write(_logFile);

        _ptTheta->setValue(pt->theta);
        _ptTheta->write(_logFile);

        _ptPsi->setValue(pt->psi);
        _ptPsi->write(_logFile);

        _ptDvlValid->setValue(pt->dvlValid ? 1 : 0);
        _ptDvlValid->write(_logFile);

        _ptGpsValid->setValue(pt->gpsValid ? 1 : 0);
        _ptGpsValid->write(_logFile);

        _ptBottomLock->setValue(pt->bottomLock ? 1 : 0);
        _ptBottomLock->write(_logFile);

#ifdef WITH_POSE_OUTPUTS

#endif
        // Terminate this record
        _logFile->endRecord();
   }
}

// log measurement update
// recordID (TRN_MOTN)
// time
// dataType (sensor ID, e.g. TRN_SENSOR_MB)
// x (UTM Northing)
// y (UTM Easting)
// z (depth)
// ping number
// beam count
// beams[...] beam_number, valid, range

void TrnLog::logMeas(measT* mt, TrnLog::TrnRecID recID)
{
    if (mt && (recID==MEAS_IN || recID==MEAS_OUT))
    {
        if(pre_write() != 0)
            return;

//        fprintf(stderr,"%s:%d      time   [%.3lf]\n", __func__, __LINE__,mt->time);
//        fprintf(stderr,"%s:%d data_type   [%d]\n", __func__, __LINE__,mt->dataType);
//        fprintf(stderr,"%s:%d         x   [%.3lf]\n", __func__, __LINE__,mt->x);
//        fprintf(stderr,"%s:%d         y   [%.3lf]\n", __func__, __LINE__,mt->y);
//        fprintf(stderr,"%s:%d         z   [%.3lf]\n", __func__, __LINE__,mt->z);
//        fprintf(stderr,"%s:%d  ping_num   [%d]\n", __func__, __LINE__,mt->ping_number);
//        fprintf(stderr,"%s:%d  num_meas   [%d]\n", __func__, __LINE__,mt->numMeas);
//        fprintf(stderr,"%s:%d  beamNums   [%p]\n", __func__, __LINE__,mt->beamNums);
//        fprintf(stderr,"%s:%d  measStatus [%p]\n", __func__, __LINE__,mt->measStatus);
//        fprintf(stderr,"%s:%d  ranges     [%p]\n", __func__, __LINE__,mt->ranges);
//        for(int i=0; i<mt->numMeas; i++){
//            fprintf(stderr,"%s:%d      beam   [%d, %d, %.2lf]\n", __func__, __LINE__,mt->beamNums ? mt->beamNums[i]:i,mt->measStatus[i]?1:0, mt->ranges[i]);
//
//        }

        _recordID->setValue(recID);
        _recordID->write(_logFile);

        _mtTime->setValue(mt->time);
        _mtTime->write(_logFile);

        _mtDataType->setValue(abs(mt->dataType));
        _mtDataType->write(_logFile);

        _mtX->setValue(mt->x);
        _mtX->write(_logFile);

        _mtY->setValue(mt->y);
        _mtY->write(_logFile);

        _mtZ->setValue(mt->z);
        _mtZ->write(_logFile);

        _mtPingNumber->setValue(mt->ping_number);
        _mtPingNumber->write(_logFile);

        _mtNumMeas->setValue(mt->numMeas);
        _mtNumMeas->write(_logFile);

        for (int i = 0; i < _mtNumMeas->value(); i++)
        {
//            fprintf(stderr,"%s:%d >>>i[%d/%d]\n",__func__,__LINE__,i,mt->numMeas);
//            fprintf(stderr,"%s:%d _mtBeamNums[%d][%p]\n",__func__,__LINE__,i,_mtBeamNums[i]);
//            fprintf(stderr,"%s:%d _mtStatus[%d][%p]\n",__func__,__LINE__,i,_mtStatus[i]);
//            fprintf(stderr,"%s:%d _mtRanges[%d][%p]\n",__func__,__LINE__,i,_mtRanges[i]);
//            fprintf(stderr,"%s:%d mt->beamNums[%p]\n",__func__,__LINE__,mt->beamNums);
//            fprintf(stderr,"%s:%d mt->beamNums[%d][%p]\n",__func__,__LINE__,i,&mt->beamNums[i]);

            if(NULL != mt->beamNums){
                _mtBeamNums[i]->setValue(mt->beamNums[i]);
                _mtBeamNums[i]->write(_logFile);
            }else{
                _mtBeamNums[i]->setValue(i);
                _mtBeamNums[i]->write(_logFile);
            }
            _mtStatus[i]->setValue((mt->measStatus[i]? 1 : 0));
            _mtStatus[i]->write(_logFile);

            _mtRanges[i]->setValue(mt->ranges[i]);
            _mtRanges[i]->write(_logFile);

        }
        // Terminate this record
        _logFile->endRecord();
    }
}

meas_beam_t *measin_beam_data(meas_in_t *self)
{
    if(NULL != self){
        uint8_t *bp = (uint8_t *)self;
        bp += sizeof(meas_in_t);
        return (meas_beam_t *)bp;//(self+1);
    }
    return NULL;
}

size_t measin_size(meas_in_t *self)
{
    if(NULL != self)
        return (sizeof(meas_in_t) + self->num_meas*sizeof(meas_beam_t));
    return 0;
}
