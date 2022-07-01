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

#define _STR(x) #x
#define STR(x) _STR(x)

#ifdef WITH_TRNLOG
#pragma message( __FILE__":" STR(__LINE__) " - feature WITH_TRNLOG_OUT enabled (see FEATURE_OPTIONS in Makefile)" )
#endif

#ifdef WITH_TRNLOG_EST_OUT
#pragma message( __FILE__":" STR(__LINE__) " - feature WITH_TRNLOG_EST_OUT enabled (see FEATURE_OPTIONS in Makefile)" )
#ifndef WITH_TRNLOG
#pragma message( __FILE__":" STR(__LINE__) " - feature WITH_TRNLOG_EST_OUT requires WITH_TRNLOG (see FEATURE_OPTIONS in Makefile)" )
#endif
#endif


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

#ifdef WITH_TRNLOG_EST_OUT
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

    for (uint32_t i = 0; i < _max_beams; i++)
    {
        snprintf(mnembuf, sizeof(mnembuf), "trn.mtRange_%02d", i);
        addField((_mtRanges[i]  = new DoubleData(mnembuf)));
    }

    for (uint32_t i = 0; i < _max_beams; i++)
    {
        snprintf(mnembuf, sizeof(mnembuf), "trn.mtStatus_%02d", i);
        addField((_mtStatus[i]  = new ShortData(mnembuf)));
    }

    for (uint32_t i = 0; i < _max_beams; i++)
    {
        snprintf(mnembuf, sizeof(mnembuf), "trn.mtBeamNum_%02d", i);
        addField((_mtBeamNums[i]  = new ShortData(mnembuf)));
    }

    _mtCrosstrack = new DoubleData *[max_beams];
    _mtAlongtrack = new DoubleData *[max_beams];
    _mtAltitudes = new DoubleData *[max_beams];
    for (uint32_t i = 0; i < _max_beams; i++)
    {
        snprintf(mnembuf, sizeof(mnembuf), "trn.mtCrosstrack_%02d", i);
        addField((_mtCrosstrack[i]  = new DoubleData(mnembuf)));
        snprintf(mnembuf, sizeof(mnembuf), "trn.mtAlongtrack_%02d", i);
        addField((_mtAlongtrack[i]  = new DoubleData(mnembuf)));
        snprintf(mnembuf, sizeof(mnembuf), "trn.mtAltitudes_%02d", i);
        addField((_mtAltitudes[i]  = new DoubleData(mnembuf)));
    }

#ifdef WITH_MEAS_OUTPUTS
    _mtAlphas = new DoubleData *[max_beams];

    addField((_mtCovariance = new DoubleData("trn.mtCovariance")));

    _mtAlphas = new DoubleData *[max_beams];
    for (int i = 0; i < _max_beams; i++)
    {
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
#ifdef WITH_TRNLOG_EST_OUT
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
    for (uint32_t i = 0; i < _max_beams; i++)
    {
        if (_mtRanges[i]) delete _mtRanges[i];
        if (_mtStatus[i]) delete _mtStatus[i];
        if (_mtBeamNums[i]) delete _mtBeamNums[i];
        if (_mtCrosstrack[i]) delete _mtCrosstrack[i];
        if (_mtAlongtrack[i]) delete _mtAlongtrack[i];
        if (_mtAltitudes[i]) delete _mtAltitudes[i];
    }

    delete [] _mtRanges;
    delete [] _mtStatus;
    delete [] _mtBeamNums;
    delete [] _mtCrosstrack;
    delete [] _mtAlongtrack;
    delete [] _mtAltitudes;

#ifdef WITH_MEAS_OUTPUTS
    for (int i = 0; i < _max_beams; i++)
    {
        if (_mtCovariance[i]) delete _mtCovariance[i];
        if (_mtAlphas[i]) delete _mtAlphas[i];
    }
    delete [] _mtCovariance;
    delete [] _mtAlphas;
#endif

}

void TrnLog::writeField(FILE *file, DataField *field)
{
    fprintf(file, "%s %s %s %s ,%s ,%s \n",
            CommentChar, field->typeMnemonic(), field->name(),
            field->asciiFormat(),
            field->longName(), field->units());
}

void TrnLog::writeHeader()
{

    fprintf(fileStream(), "%s %s %s\n",
            CommentChar, BinaryFormatMnem, mnemonic());

    fprintf(fileStream(), "%s Contains TRN input records\n", CommentChar);
    fprintf(fileStream(), "%s structured as follows:\n", CommentChar);
    fprintf(fileStream(), "%s\n", CommentChar);
    fprintf(fileStream(), "%s TRN motion update input\n", CommentChar);
    writeField(fileStream(), _recordID);
    writeField(fileStream(), _ptTime);
    writeField(fileStream(), _ptX);
    writeField(fileStream(), _ptY);
    writeField(fileStream(), _ptZ);
    writeField(fileStream(), _ptVx);
    writeField(fileStream(), _ptVy);
    writeField(fileStream(), _ptVz);
    writeField(fileStream(), _ptPhi);
    writeField(fileStream(), _ptTheta);
    writeField(fileStream(), _ptPsi);
    writeField(fileStream(), _ptDvlValid);
    writeField(fileStream(), _ptGpsValid);
    writeField(fileStream(), _ptBottomLock);
    fprintf(fileStream(), "%s\n", CommentChar);

    fprintf(fileStream(), "%s TRN measurement update input\n", CommentChar);
    writeField(fileStream(), _recordID);
    writeField(fileStream(), _mtTime);
    writeField(fileStream(), _mtDataType);
    writeField(fileStream(), _mtX);
    writeField(fileStream(), _mtY);
    writeField(fileStream(), _mtZ);
    writeField(fileStream(), _mtPingNumber);
    writeField(fileStream(), _mtNumMeas);
    fprintf(fileStream(), "%s followed by an array of beam entries, e.g.:\n", CommentChar);
    writeField(fileStream(), _mtBeamNums[0]);
    writeField(fileStream(), _mtStatus[0]);
    writeField(fileStream(), _mtRanges[0]);
    writeField(fileStream(), _mtCrosstrack[0]);
    writeField(fileStream(), _mtAlongtrack[0]);
    writeField(fileStream(), _mtAltitudes[0]);
    fprintf(fileStream(), "%s\n", CommentChar);

    fprintf(fileStream(), "%s Record IDs are 32-bit (4 byte) printable ASCII sequences:\n", CommentChar);
    fprintf(fileStream(), "%s  'MTNI' : motion update input\n", CommentChar);
    fprintf(fileStream(), "%s  'MEAI' : measurement update input\n", CommentChar);
    fprintf(fileStream(), "%s  'MTNO' : motion update output (not implemented)\n", CommentChar);
    fprintf(fileStream(), "%s  'MEAO' : measurement update (not implemented)\n", CommentChar);
    fprintf(fileStream(), "%s Record order is not guaranteed.\n", CommentChar);
    fprintf(fileStream(), "%s %s\n", CommentChar, BeginDataMnem);

    _handledHeader = True;

    fflush(fileStream());

}

int TrnLog::pre_write()
{
    if (checkLog() != 0) {
        return -1;
    }

    if (!_handledHeader) {
        // Need to write header. Note that write() is not called until
        // after all Data fields have been added.
        writeHeader();
    }

    updateAutoTimestamp();

    return 0;
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
    if (pt && recID==MOTN_IN)
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
    if (mt && recID==MEAS_IN)
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

            if(NULL != mt->measStatus){
                _mtStatus[i]->setValue((mt->measStatus[i]? 1 : 0));
                _mtStatus[i]->write(_logFile);
            }else{
                _mtStatus[i]->setValue(0);
                _mtStatus[i]->write(_logFile);
           }
            if(NULL != mt->ranges){
                _mtRanges[i]->setValue(mt->ranges[i]);
                _mtRanges[i]->write(_logFile);
            }else{
                _mtRanges[i]->setValue(0);
                _mtRanges[i]->write(_logFile);
            }
            if(NULL != mt->crossTrack){
                _mtCrosstrack[i]->setValue(mt->crossTrack[i]);
                _mtCrosstrack[i]->write(_logFile);
            }else{
                _mtCrosstrack[i]->setValue(0);
                _mtCrosstrack[i]->write(_logFile);
            }
            if(NULL != mt->alongTrack){
                _mtAlongtrack[i]->setValue(mt->alongTrack[i]);
                _mtAlongtrack[i]->write(_logFile);
            }else{
                _mtAlongtrack[i]->setValue(0);
                _mtAlongtrack[i]->write(_logFile);
            }
            if(NULL != mt->altitudes){
                _mtAltitudes[i]->setValue(mt->altitudes[i]);
                _mtAltitudes[i]->write(_logFile);
            }else{
               _mtAltitudes[i]->setValue(0);
               _mtAltitudes[i]->write(_logFile);
            }
        }
        // Terminate this record
        _logFile->endRecord();
    }
}

#ifdef WITH_TRNLOG_EST_OUT

void TrnLog::logEst(poseT* pt, TrnLog::TrnRecID recID)
{
    if (pt && (recID==MSE_OUT || recID==MLE_OUT))
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

        _ptVe->setValue(pt->ve);
        _ptVe->write(_logFile);

        _ptVwx->setValue(pt->vw_x);
        _ptVwx->write(_logFile);

        _ptVwy->setValue(pt->vw_y);
        _ptVwy->write(_logFile);

        _ptVwz->setValue(pt->vw_z);
        _ptVwz->write(_logFile);

        _ptVnx->setValue(pt->vn_x);
        _ptVnx->write(_logFile);

        _ptVny->setValue(pt->vn_y);
        _ptVny->write(_logFile);

        _ptVnz->setValue(pt->vn_z);
        _ptVnz->write(_logFile);

        _ptWx->setValue(pt->wx);
        _ptWx->write(_logFile);

        _ptWy->setValue(pt->wy);
        _ptWy->write(_logFile);

        _ptWz->setValue(pt->wz);
        _ptWz->write(_logFile);

        _ptAx->setValue(pt->ax);
        _ptAx->write(_logFile);

        _ptAy->setValue(pt->ay);
        _ptAy->write(_logFile);

        _ptAz->setValue(pt->az);
        _ptAz->write(_logFile);

        _ptPhi->setValue(pt->phi);
        _ptPhi->write(_logFile);

        _ptTheta->setValue(pt->theta);
        _ptTheta->write(_logFile);

        _ptPsi->setValue(pt->psi);
        _ptPsi->write(_logFile);

        _ptPsiBerg->setValue(pt->psi_berg);
        _ptPsiBerg->write(_logFile);

        _ptPsiDotBerg->setValue(pt->psi_dot_berg);
        _ptPsiDotBerg->write(_logFile);

        _ptDvlValid->setValue(pt->dvlValid ? 1 : 0);
        _ptDvlValid->write(_logFile);

        _ptGpsValid->setValue(pt->gpsValid ? 1 : 0);
        _ptGpsValid->write(_logFile);

        _ptBottomLock->setValue(pt->bottomLock ? 1 : 0);
        _ptBottomLock->write(_logFile);

        for(int i=0; i< N_COVAR; i++)
        {
            _ptCovariance[i]->setValue(pt->covariance[i]);
            _ptCovariance[i]->write(_logFile);

        }

        // Terminate this record
        _logFile->endRecord();
    }
}
#endif

meas_beam_t *TrnLog::meaiBeamData(meas_in_t *self)
{
    if(NULL != self){
        uint8_t *bp = (uint8_t *)self;
        bp += sizeof(meas_in_t);
        return (meas_beam_t *)bp;
    }
    return NULL;
}
