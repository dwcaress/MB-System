/****************************************************************************/
/* Copyright (c) 2022 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : CoNavLog.cpp                                                  */
/* Author   : henthorn                                                      */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 01/27/2022                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/

#include <string>
#include <stdint.h>

// Status logging
#define ZF_LOG_TAG "CoNavLog:"
#include <zf_log.h>

#include "lrconav_app.h"
#include "CoNavLog.h"

///////////////////////////////////////////////////////////////////
// Constructor
CoNavLog::CoNavLog(const std::string& logname,
                   DataLog::FileFormat fileFormat)
  : DataLogWriter(logname.c_str(), fileFormat, AutoTimeStamp)
{
  _timestamp = _northing = _easting = NULL;
  _motionE = _motionN = _motionZ = _motionTime = NULL;
  _event = _id = NULL;
  _measTime = _measTrnN = _measTrnE = _measTrnZ = NULL;
  _measTrnNVar = _measTrnEVar = _measTrnZVar = NULL;
  _measRange = _measBear = _measRangeVar = _measBearVar = NULL;
  _pb11 = _pb12 = _pb21 = _pb22 = NULL;

  init();
}

// Free up the heap memory used by this log
CoNavLog::~CoNavLog()
{
  DELOBJ(_event);

  DELOBJ(_northing);
  DELOBJ(_easting);

  DELOBJ(_motionTime);
  DELOBJ(_motionN);
  DELOBJ(_motionE);
  DELOBJ(_motionZ);

  DELOBJ(_id);
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

  DELOBJ(_pb11);
  DELOBJ(_pb12);
  DELOBJ(_pb21);
  DELOBJ(_pb22);
}

// Create the fields needed in the log
void CoNavLog::init(void)
{
  // initialize fields
  char dname[100];
  sprintf(dname, "conav.data");
  setMnemonic(dname);

  // filter state
  sprintf(dname, "%s.timestamp", name());
  addField((_timestamp = new DoubleData(dname, "Vehicle timestamp", "epoch seconds")));
  _timestamp->setAsciiFormat("%14.3f");
  sprintf(dname, "%s.northing", name());
  addField((_northing = new DoubleData(dname, "UTM Northing", "meters")));
  _northing->setAsciiFormat("%.3f");
  sprintf(dname, "%s.easting", name());
  addField((_easting = new DoubleData(dname, "UTM Easting", "meters")));
  _easting->setAsciiFormat("%.3f");

  // PBest matrix state
  sprintf(dname, "%s.PBest_11", name());
  addField((_pb11 = new DoubleData(dname, "Element 1.1 of PBest matrix", "")));
  sprintf(dname, "%s.PBest_12", name());
  addField((_pb12 = new DoubleData(dname, "Element 1.2 of PBest matrix", "")));
  sprintf(dname, "%s.PBest_21", name());
  addField((_pb21 = new DoubleData(dname, "Element 2.1 of PBest matrix", "")));
  sprintf(dname, "%s.PBest_22", name());
  addField((_pb22 = new DoubleData(dname, "Element 2.2 of PBest matrix", "")));

  // Triggering event (motion input, meas input)
  sprintf(dname, "%s.event", name());
  addField((_event = new IntegerData(dname)));
  _event->setLongName("Triggering event (0=motion 1=meas from vehicle 1 etc)");

  // Motion update input
  sprintf(dname, "%s.motionTime", name());
  addField((_motionTime = new DoubleData(dname, "Ego Vehicle Nav timestamp", "epoch seconds")));
  _motionTime->setAsciiFormat("%14.3f");
  sprintf(dname, "%s.motionN", name());
  addField((_motionN    = new DoubleData(dname, "Ego Vehicle Nav northing", "meters")));
  _motionN->setAsciiFormat("%.3f");
  sprintf(dname, "%s.motionE", name());
  addField((_motionE    = new DoubleData(dname, "Ego Vehicle Nav easting", "meters")));
  _motionE->setAsciiFormat("%.3f");
  sprintf(dname, "%s.motionZ", name());
  addField((_motionZ    = new DoubleData(dname, "Ego Vehicle Nav depth", "meters")));
  _motionZ->setAsciiFormat("%.3f");

  // CoNav update input
  sprintf(dname, "%s.id", name());
  addField((_id = new IntegerData(dname)));
  _id->setLongName("MRFilter ID of measuring vehicle");
  sprintf(dname, "%s.measTime", name());
  addField((_measTime     = new DoubleData(dname, "Filter Vehicle Measurement timestamp", "epoch seconds")));
  _measTime->setAsciiFormat("%14.3f");
  sprintf(dname, "%s.measTrnN", name());
  addField((_measTrnN     = new DoubleData(dname, "Measurement TRN northing", "meters")));
  _measTrnN->setAsciiFormat("%.3f");
  sprintf(dname, "%s.measTrnE", name());
  addField((_measTrnE     = new DoubleData(dname, "Measurement TRN easting", "meters")));
  _measTrnE->setAsciiFormat("%.3f");
  sprintf(dname, "%s.measTrnZ", name());
  addField((_measTrnZ     = new DoubleData(dname, "Measurement TRN depth", "meters")));
  _measTrnZ->setAsciiFormat("%.3f");
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
}

// set the field values for the filter state
int CoNavLog::setCoNavState(const struct CoNavState& state)
{
  if (_timestamp && _northing && _easting) {
    _timestamp->setValue(state.timestamp);
    _northing ->setValue(state.bestNorthing);
    _easting  ->setValue(state.bestEasting);
    return 0;
  } else {
    ZF_LOGE("One or more state fields are NULL");
    return -1;
  }
}

// set the field values for the motion data
int CoNavLog::setCoNavMotion(const struct MRFilterLog::VehicleNavData& nd)
{
  if (_event && _motionE && _motionN && _motionZ && _motionTime) {
    _event     ->setValue(0);
    _motionTime->setValue(nd.egoClock);
    _motionN   ->setValue(nd.northing);
    _motionE   ->setValue(nd.easting);
    _motionZ   ->setValue(nd.depth);
    return 0;
  } else {
    ZF_LOGE("One or more motion fields are NULL");
    return -1;
  }
}

// set the field values for the measure data
int CoNavLog::setCoNavMeas(const struct MRFilterLog::CoopVehicleNavData& cnd)
{
  if (_event && _id && _measTime && _measTrnN && _measTrnE && _measTrnZ &&
      _measTrnNVar && _measTrnEVar && _measTrnZVar  &&
      _measRange   && _measBear    && _measRangeVar && _measBearVar) {
    _event->setValue(cnd.vehId);
    _id->setValue(cnd.vehId);
    _measTime->setValue(cnd.coopClock);
    _measTrnN->setValue(cnd.trnN);
    _measTrnE->setValue(cnd.trnE);
    _measTrnZ->setValue(cnd.trnZ);
    _measTrnNVar->setValue(cnd.trnNVar);
    _measTrnEVar->setValue(cnd.trnEVar);
    _measTrnZVar->setValue(cnd.trnZVar);
    _measRange->setValue(cnd.range);
    _measRangeVar->setValue(cnd.rangeVar * cnd.range);
    _measBear->setValue(cnd.bearing);
    _measBearVar->setValue(cnd.bearingVar);
    return 0;
  } else {
    ZF_LOGE("One or more measure fields are NULL");
    return -1;
  }
}

// return < 0 when the size of the matrix pbest is not the expected 2x2
int CoNavLog::setCoNavPbest(const arma::mat& pbest)
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

void CoNavLog::setFields()
{
  // this operations of this function is handled by the setMRFilter* functions
}
