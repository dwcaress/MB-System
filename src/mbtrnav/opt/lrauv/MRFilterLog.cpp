/****************************************************************************/
/* Copyright (c) 2022 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : MRFilterLog.cpp                                               */
/* Author   : henthorn                                                      */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 01/10/2022                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/

#include <string>
#include <stdint.h>

// Status logging
#define ZF_LOG_TAG "MRFilterLog:"
#include <zf_log.h>

#include "MRFilterLog.h"

///////////////////////////////////////////////////////////////////
// Constructor
MRFilterLog::MRFilterLog(int id, const std::string& logname,
            DataLog::FileFormat fileFormat)
  : DataLogWriter(logname.c_str(), fileFormat, AutoTimeStamp), _id(id)
{
  _missionTime = NULL;
  _northing = _easting = _deltaN = _deltaE = _distance = NULL;
  _motionE = _motionN = _motionZ = _motionTime = NULL;
  _measTime = _measTrnN = _measTrnE = _measTrnZ = NULL;
  _measTrnNVar = _measTrnEVar = _measTrnZVar = NULL;
  _measRange = _measBear = _measRangeVar = _measBearVar = NULL;
  _origin = NULL;
  _p11 = _p12 = _p21 = _p22 = NULL;
  _pb11 = _pb12 = _pb21 = _pb22 = NULL;
  _qb11 = _qb12 = _qb21 = _qb22 = NULL;
  _dz11 = _dz21 = NULL;
  _rij11 = _rij12 = _rij21 = _rij22 = NULL;
  _hi11 = _hi12 = _hi21 = _hi22 = NULL;
  _hj11 = _hj12 = _hj21 = _hj22 = NULL;
  _pj11 = _pj12 = _pj21 = _pj22 = NULL;
  _measN = _measE = NULL;
  _wopt = NULL;

  init();
}

// Free up the heap memory used by this log
MRFilterLog::~MRFilterLog()
{
  DELOBJ(_missionTime);

  DELOBJ(_northing);
  DELOBJ(_easting);
  DELOBJ(_deltaN);
  DELOBJ(_deltaE);
  DELOBJ(_distance);

  DELOBJ(_motionTime);
  DELOBJ(_motionN);
  DELOBJ(_motionE);
  DELOBJ(_motionZ);

  DELOBJ(_measTime);
  DELOBJ(_measTrnN);
  DELOBJ(_measTrnE);
  DELOBJ(_measTrnZ);
  DELOBJ(_measTrnNVar);
  DELOBJ(_measTrnEVar);
  DELOBJ(_measTrnZVar);
  DELOBJ(_measRange);
  DELOBJ(_measBear);
  DELOBJ(_measRangeVar);
  DELOBJ(_measBearVar);

  DELOBJ(_origin);
  DELOBJ(_p11);
  DELOBJ(_p12);
  DELOBJ(_p21);
  DELOBJ(_p22);

  DELOBJ(_pb11);
  DELOBJ(_pb12);
  DELOBJ(_pb21);
  DELOBJ(_pb22);

  DELOBJ(_qb11);
  DELOBJ(_qb12);
  DELOBJ(_qb21);
  DELOBJ(_qb22);

  DELOBJ(_dz11);
  DELOBJ(_dz21);

  DELOBJ(_rij11);
  DELOBJ(_rij12);
  DELOBJ(_rij21);
  DELOBJ(_rij22);

  DELOBJ(_hi11);
  DELOBJ(_hi12);
  DELOBJ(_hi21);
  DELOBJ(_hi22);

  DELOBJ(_hj11);
  DELOBJ(_hj12);
  DELOBJ(_hj21);
  DELOBJ(_hj22);

  DELOBJ(_pj11);
  DELOBJ(_pj12);
  DELOBJ(_pj21);
  DELOBJ(_pj22);

  DELOBJ(_measN);
  DELOBJ(_measE);

  DELOBJ(_wopt);
}

// Create the fields needed in the log
void MRFilterLog::init(void)
{
  // initialize fields
  char dname[100];
  sprintf(dname, "mrf%d.data", _id);
  setMnemonic(dname);

  // Mission time
  sprintf(dname, "%s.missionTime", name());
  addField((_missionTime = new DoubleData(dname, "Ego Vehicle mission time", "epoch seconds")));
  _missionTime->setAsciiFormat("%14.3f");

  // filter state
  sprintf(dname, "%s.northing", name());
  addField((_northing = new DoubleData(dname, "UTM Northing", "meters")));
  sprintf(dname, "%s.easting", name());
  addField((_easting = new DoubleData(dname, "UTM Easting", "meters")));
  sprintf(dname, "%s.deltaN", name());
  addField((_deltaN = new DoubleData(dname,
           "Delta Northing between motion updates", "meters")));
  sprintf(dname, "%s.deltaE", name());
  addField((_deltaE = new DoubleData(dname,
           "Delta Easting between motion updates", "meters")));
  sprintf(dname, "%s.distance", name());
  addField((_distance = new DoubleData(dname, "Distance traveled", "meters")));

  // Motion update
  sprintf(dname, "%s.motionTime", name());
  addField((_motionTime = new DoubleData(dname, "Ego Vehicle Nav timestamp", "epoch seconds")));
  _motionTime->setAsciiFormat("%14.3f");
  sprintf(dname, "%s.motionN", name());
  addField((_motionN    = new DoubleData(dname, "Ego Vehicle Nav northing", "meters")));
  sprintf(dname, "%s.motionE", name());
  addField((_motionE    = new DoubleData(dname, "Ego Vehicle Nav easting", "meters")));
  sprintf(dname, "%s.motionZ", name());
  addField((_motionZ    = new DoubleData(dname, "Ego Vehicle Nav depth", "meters")));

  // CoNav update
  sprintf(dname, "%s.measTime", name());
  addField((_measTime     = new DoubleData(dname, "Filter Vehicle Measurement timestamp", "epoch seconds")));
  _measTime->setAsciiFormat("%14.3f");
  sprintf(dname, "%s.measTrnN", name());
  addField((_measTrnN     = new DoubleData(dname, "Measurement TRN northing", "meters")));
  sprintf(dname, "%s.measTrnE", name());
  addField((_measTrnE     = new DoubleData(dname, "Measurement TRN easting", "meters")));
  sprintf(dname, "%s.measTrnZ", name());
  addField((_measTrnZ     = new DoubleData(dname, "Measurement TRN depth", "meters")));
  sprintf(dname, "%s.measTrnNVar", name());
  addField((_measTrnNVar  = new DoubleData(dname, "Measurement TRN northing sigma", "")));
  sprintf(dname, "%s.measTrnEVar", name());
  addField((_measTrnEVar  = new DoubleData(dname, "Measurement TRN easting sigma", "")));
  sprintf(dname, "%s.measTrnZVar", name());
  addField((_measTrnZVar = new DoubleData(dname, "Measurement TRN depth sigma", "")));
  sprintf(dname, "%s.measRange", name());
  addField((_measRange    = new DoubleData(dname, "Measurement range to Ego vehicle", "meters")));
  sprintf(dname, "%s.measBear", name());
  addField((_measBear     = new DoubleData(dname, "Measurement bearing to Ego vehicle", "radians")));
  sprintf(dname, "%s.measRangeVar", name());
  addField((_measRangeVar = new DoubleData(dname, "Measurement range sigma", "")));
  sprintf(dname, "%s.measBearVar", name());
  addField((_measBearVar  = new DoubleData(dname, "Measurement bearing sigma", "")));

  // P matrix
  sprintf(dname, "%s.origin", name());
  //addField((_origin = new IntegerData(dname, "Origin of record: 0=>motion, 1=>measure", "")));
  addField((_origin = new IntegerData(dname)));
  _origin->setLongName("Origin of record: 0=>motion 1=>measure");
  sprintf(dname, "%s.P_11", name());
  addField((_p11 = new DoubleData(dname, "Element 1.1 of P matrix", "")));
  sprintf(dname, "%s.P_12", name());
  addField((_p12 = new DoubleData(dname, "Element 1.2 of P matrix", "")));
  sprintf(dname, "%s.P_21", name());
  addField((_p21 = new DoubleData(dname, "Element 2.1 of P matrix", "")));
  sprintf(dname, "%s.P_22", name());
  addField((_p22 = new DoubleData(dname, "Element 2.2 of P matrix", "")));

  // PBest matrix
  sprintf(dname, "%s.PBest_11", name());
  addField((_pb11 = new DoubleData(dname, "Element 1.1 of PBest matrix", "")));
  sprintf(dname, "%s.PBest_12", name());
  addField((_pb12 = new DoubleData(dname, "Element 1.2 of PBest matrix", "")));
  sprintf(dname, "%s.PBest_21", name());
  addField((_pb21 = new DoubleData(dname, "Element 2.1 of PBest matrix", "")));
  sprintf(dname, "%s.PBest_22", name());
  addField((_pb22 = new DoubleData(dname, "Element 2.2 of PBest matrix", "")));

  // QBar matrix
  sprintf(dname, "%s.QBar_11", name());
  addField((_qb11 = new DoubleData(dname, "Element 1.1 of QBar matrix", "")));
  sprintf(dname, "%s.QBar_12", name());
  addField((_qb12 = new DoubleData(dname, "Element 1.2 of QBar matrix", "")));
  sprintf(dname, "%s.QBar_21", name());
  addField((_qb21 = new DoubleData(dname, "Element 2.1 of QBar matrix", "")));
  sprintf(dname, "%s.QBar_22", name());
  addField((_qb22 = new DoubleData(dname, "Element 2.2 of QBar matrix", "")));

  // Deltaz matrix
  sprintf(dname, "%s.Deltaz_11", name());
  addField((_dz11 = new DoubleData(dname, "Element 1.1 of Deltaz matrix", "")));
  sprintf(dname, "%s.Deltaz_21", name());
  addField((_dz21 = new DoubleData(dname, "Element 2.1 of Deltaz matrix", "")));

  // Rij matrix
  sprintf(dname, "%s.Rij_11", name());
  addField((_rij11 = new DoubleData(dname, "Element 1.1 of Rij matrix", "")));
  sprintf(dname, "%s.Rij_12", name());
  addField((_rij12 = new DoubleData(dname, "Element 1.2 of Rij matrix", "")));
  sprintf(dname, "%s.Rij_21", name());
  addField((_rij21 = new DoubleData(dname, "Element 2.1 of Rij matrix", "")));
  sprintf(dname, "%s.Rij_22", name());
  addField((_rij22 = new DoubleData(dname, "Element 2.2 of Rij matrix", "")));

  // Hi matrix
  sprintf(dname, "%s.Hi_11", name());
  addField((_hi11 = new DoubleData(dname, "Element 1.1 of Hi matrix", "")));
  sprintf(dname, "%s.Hi_12", name());
  addField((_hi12 = new DoubleData(dname, "Element 1.2 of Hi matrix", "")));
  sprintf(dname, "%s.Hi_21", name());
  addField((_hi21 = new DoubleData(dname, "Element 2.1 of Hi matrix", "")));
  sprintf(dname, "%s.Hi_22", name());
  addField((_hi22 = new DoubleData(dname, "Element 2.2 of Hi matrix", "")));

  // Hj matrix
  sprintf(dname, "%s.Hj_11", name());
  addField((_hj11 = new DoubleData(dname, "Element 1.1 of Hj matrix", "")));
  sprintf(dname, "%s.Hj_12", name());
  addField((_hj12 = new DoubleData(dname, "Element 1.2 of Hj matrix", "")));
  sprintf(dname, "%s.Hj_21", name());
  addField((_hj21 = new DoubleData(dname, "Element 2.1 of Hj matrix", "")));
  sprintf(dname, "%s.Hj_22", name());
  addField((_hj22 = new DoubleData(dname, "Element 2.2 of Hj matrix", "")));

  // Pj matrix
  sprintf(dname, "%s.Pj_11", name());
  addField((_pj11 = new DoubleData(dname, "Element 1.1 of Pj matrix", "")));
  sprintf(dname, "%s.Pj_12", name());
  addField((_pj12 = new DoubleData(dname, "Element 1.2 of Pj matrix", "")));
  sprintf(dname, "%s.Pj_21", name());
  addField((_pj21 = new DoubleData(dname, "Element 2.1 of Pj matrix", "")));
  sprintf(dname, "%s.Pj_22", name());
  addField((_pj22 = new DoubleData(dname, "Element 2.2 of Pj matrix", "")));

  // Measure position
  sprintf(dname, "%s.measN", name());
  addField((_measN = new DoubleData(dname, "Interpolated N", "meters")));
  sprintf(dname, "%s.measE", name());
  addField((_measE = new DoubleData(dname, "Interpolated E", "meters")));

  // wopt
  sprintf(dname, "%s.Wopt", name());
  addField((_wopt = new DoubleData(dname, "Wout", "")));
}

// set the field values for the filter state
int MRFilterLog::setMRFilterState(const struct CoNav::MRFilterState& state)
{
  if (_northing && _easting && _deltaN && _deltaE && _distance) {
    _northing->setValue(state.nij);
    _easting ->setValue(state.eij);
    _deltaN  ->setValue(state.deltaN);
    _deltaE  ->setValue(state.deltaE);
    _distance->setValue(state.distance);
    return 0;
  } else {
    ZF_LOGE("One or more state fields are NULL");
    return -1;
  }
}

// set the field values for the motion data
int MRFilterLog::setMRFilterMotion(const struct CoNav::ERNavInput& nd)
{
  if (_motionE && _motionN && _motionZ && _motionTime) {
    _motionTime->setValue(nd.egoTime);
    _motionN   ->setValue(nd.navN);
    _motionE   ->setValue(nd.navE);
    _motionZ   ->setValue(nd.navZ);
    return 0;
  } else {
    ZF_LOGE("One or more motion fields are NULL");
    return -1;
  }
}

// set the field values for the measure data
int MRFilterLog::setMRFilterMeas(const struct CoNav::MRDATInput& cnd)
{
  if (_measTime    && _measTrnN    && _measTrnE     && _measTrnZ &&
      _measTrnNVar && _measTrnEVar && _measTrnZVar  &&
      _measRange   && _measBear    && _measRangeVar && _measBearVar) {
    _measTime->setValue(cnd.datTime);
    _measTrnN->setValue(cnd.nj);
    _measTrnE->setValue(cnd.ej);
    _measTrnZ->setValue(cnd.dj);
    _measTrnNVar->setValue(cnd.njCovar);
    _measTrnEVar->setValue(cnd.ejCovar);
    _measTrnZVar->setValue(cnd.djCovar);
    _measRange->setValue(cnd.range);
    _measRangeVar->setValue(cnd.rangeSigma * cnd.range);
    _measBear->setValue(cnd.bearing);
    _measBearVar->setValue(cnd.bearingSigma);
    return 0;
  } else {
    ZF_LOGE("One or more measure fields are NULL");
    return -1;
  }
}

// return < 0 when the size of the matrix p is not the expected 2x2
int MRFilterLog::setMRFilterP(int origin, const arma::mat& p)
{
  // set the values of the fields if the size of p is ok
  if (_origin && _p11 && _p12 && _p21 && _p22) {
    if (p.n_rows == 2 && p.n_cols == 2) {
      _p11->setValue(p(0,0));
      _p12->setValue(p(0,1));
      _p21->setValue(p(1,0));
      _p22->setValue(p(1,1));
      _origin->setValue(origin);
      return 0;
    } else {
      ZF_LOGE("P matrix param has bad size: %llu x %llu", p.n_rows, p.n_cols);
      return -1;
    }
  } else {
    ZF_LOGE("One or more P matrix fields are NULL");
    return -1;
  }
}

// return < 0 when the size of the matrix pbest is not the expected 2x2
int MRFilterLog::setMRFilterPbest(const arma::mat& pbest)
{
  // set the values of the fields if the size of pbest is ok
  if (_pb11 && _pb12 && _pb21 && _pb22) {
    if (pbest.n_rows == 2 && pbest.n_cols == 2) {
      _pb11->setValue(pbest(0,0));
      _pb12->setValue(pbest(0,1));
      _pb21->setValue(pbest(1,0));
      _pb22->setValue(pbest(1,1));
      return 0;
    } else {
      ZF_LOGE("Pbest matrix param has bad size: %llu x %llu",
              pbest.n_rows, pbest.n_cols);
      return -1;
    }
  } else {
    ZF_LOGE("One or more Pbest matrix fields are NULL");
    return -1;
  }
}

// return < 0 when the size of the matrix qbar is not the expected 2x2
int MRFilterLog::setMRFilterQbar(const arma::mat& qbar)
{
  // set the values of the fields if the size of qbar is ok
  if (_qb11 && _qb12 && _qb21 && _qb22) {
    if (qbar.n_rows == 2 && qbar.n_cols == 2) {
      _qb11->setValue(qbar(0,0));
      _qb12->setValue(qbar(0,1));
      _qb21->setValue(qbar(1,0));
      _qb22->setValue(qbar(1,1));
      return 0;
    } else {
      ZF_LOGE("Qbar matrix param has bad size: %llu x %llu",
              qbar.n_rows, qbar.n_cols);
      return -1;
    }
  } else {
    ZF_LOGE("One or more Qbar matrix fields are NULL");
    return -1;
  }
}

// return < 0 when the size of the matrix deltaz is not the expected 2x1
int MRFilterLog::setMRFilterDeltaz(const arma::mat& deltaz)
{
  // set the values of the fields if the size of deltaz is ok
  if (_dz11 && _dz21) {
    if (deltaz.n_rows == 2 && deltaz.n_cols == 1) {
      _dz11->setValue(deltaz(0,0));
      _dz21->setValue(deltaz(1,0));
      return 0;
    } else {
      ZF_LOGE("deltaz matrix param has bad size: %llu x %llu",
              deltaz.n_rows, deltaz.n_cols);
      return -1;
    }
  } else {
    ZF_LOGE("One or more deltaz matrix fields are NULL");
    return -1;
  }
}

// return < 0 when the size of the matrix rij is not the expected 2x2
int MRFilterLog::setMRFilterRij(const arma::mat& rij)
{
  // set the values of the fields if the size of rij is ok
  if (_rij11 && _rij12 && _rij21 && _rij22) {
    if (rij.n_rows == 2 && rij.n_cols == 2) {
      _rij11->setValue(rij(0,0));
      _rij12->setValue(rij(0,1));
      _rij21->setValue(rij(1,0));
      _rij22->setValue(rij(1,1));
      return 0;
    } else {
      ZF_LOGE("Rij matrix param has bad size: %llu x %llu",
              rij.n_rows, rij.n_cols);
      return -1;
    }
  } else {
    ZF_LOGE("One or more Rij matrix fields are NULL");
    return -1;
  }
}

// return < 0 when the size of the matrix hi is not the expected 2x2
int MRFilterLog::setMRFilterHi(const arma::mat& hi)
{
  // set the values of the fields if the size of hi is ok
  if (_hi11 && _hi12 && _hi21 && _hi22) {
    if (hi.n_rows == 2 && hi.n_cols == 2) {
      _hi11->setValue(hi(0,0));
      _hi12->setValue(hi(0,1));
      _hi21->setValue(hi(1,0));
      _hi22->setValue(hi(1,1));
      return 0;
    } else {
      ZF_LOGE("Hi matrix param has bad size: %llu x %llu",
              hi.n_rows, hi.n_cols);
      return -1;
    }
  } else {
    ZF_LOGE("One or more Hi matrix fields are NULL");
    return -1;
  }
}

// return < 0 when the size of the matrix hj is not the expected 2x2
int MRFilterLog::setMRFilterHj(const arma::mat& hj)
{
  // set the values of the fields if the size of hj is ok
  if (_hj11 && _hj12 && _hj21 && _hj22) {
    if (hj.n_rows == 2 && hj.n_cols == 2) {
      _hj11->setValue(hj(0,0));
      _hj12->setValue(hj(0,1));
      _hj21->setValue(hj(1,0));
      _hj22->setValue(hj(1,1));
      return 0;
    } else {
      ZF_LOGE("Hj matrix param has bad size: %llu x %llu",
              hj.n_rows, hj.n_cols);
      return -1;
    }
  } else {
    ZF_LOGE("One or more Hj matrix fields are NULL");
    return -1;
  }
}

// return < 0 when the size of the matrix pj is not the expected 2x2
int MRFilterLog::setMRFilterPj(const arma::mat& pj)
{
  // set the values of the fields if the size of pj is ok
  if (_pj11 && _pj12 && _pj21 && _pj22) {
    if (pj.n_rows == 2 && pj.n_cols == 2) {
      _pj11->setValue(pj(0,0));
      _pj12->setValue(pj(0,1));
      _pj21->setValue(pj(1,0));
      _pj22->setValue(pj(1,1));
      return 0;
    } else {
      ZF_LOGE("Pj matrix param has bad size: %llu x %llu",
              pj.n_rows, pj.n_cols);
      return -1;
    }
  } else {
    ZF_LOGE("One or more Pj matrix fields are NULL");
    return -1;
  }
}

int MRFilterLog::setMRFilterMeasPosition(double measN, double measE)
{
  if (_measN && _measE) {
    _measN->setValue(measN);
    _measE->setValue(measE);
    return 0;
  } else {
    ZF_LOGE("One or more position fields are NULL");
    return -1;
  }
}

int MRFilterLog::setMRFilterWopt(double wopt)
{
  if (_wopt) {
    _wopt->setValue(wopt);
    return 0;
  } else {
    ZF_LOGE("Wout field is NULL");
    return -1;
  }
}

void MRFilterLog::setFields()
{
  // this operations of this function is handled by the setMRFilter* functions
}
