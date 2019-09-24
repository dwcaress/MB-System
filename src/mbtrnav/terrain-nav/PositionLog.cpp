////////////////////////////////////////////////////////////////////////////////
//
// PURPOSE:  PositionLog Log Class for TRN.
// AUTHOR:   Henthorn.
// DATE:     01/30/19
// COMMENTS: 
//
////////////////////////////////////////////////////////////////////////////////
//
#include "structDefs.h"
#include "PositionLog.h"

PositionLog::PositionLog(DataLog::FileFormat fileFormat, const char* logname,
  const char *mnem)
  : DataLogWriter(logname, fileFormat, AutoTimeStamp)
{
  char mnembuf[100];
  snprintf(mnembuf, sizeof(mnembuf), "%s.data", mnem);
  setMnemonic(mnembuf);

  addField((ptime = new DoubleData("trn_pos.navTime")));
  printf("setting time format to 14.4\n");
  ptime->setLongName("Nav data timestamp");
  ptime->setAsciiFormat("%14.4f"); 
  ptime->setUnits("epoch seconds");

  addField((x = new DoubleData("trn_pos.x")));
  x->setAsciiFormat("%13.2f");             //Necessary for large UTM values.
  x->setLongName("Northing");
  x->setUnits("meters");

  addField((y = new DoubleData("trn_pos.y")));
  y->setAsciiFormat("%13.2f");
  y->setLongName("Easting");
  y->setUnits("meters");

  addField((z = new DoubleData("trn_pos.z")));
  z->setAsciiFormat("%13.2f");
  z->setLongName("Down");
  z->setUnits("meters");

  addField((vx = new DoubleData("trn_pos.vx")));
  vx->setLongName("Veh V wrt iceberg"); vx->setUnits("m/s");
  addField((vy = new DoubleData("trn_pos.vy")));
  addField((vz = new DoubleData("trn_pos.vz")));
  addField((ve = new DoubleData("trn_pos.ve")));

  addField((vw_x = new DoubleData("trn_pos.vw_x")));
  vw_x->setLongName("Veh V wrt water"); vw_x->setUnits("m/s");
  addField((vw_y = new DoubleData("trn_pos.vw_y")));
  addField((vw_z = new DoubleData("trn_pos.vw_z")));

  addField((vn_x = new DoubleData("trn_pos.vn_x")));
  vn_x->setLongName("Veh V wrt an inertial frame"); vn_x->setUnits("m/s");
  addField((vn_y = new DoubleData("trn_pos.vn_y")));
  addField((vn_z = new DoubleData("trn_pos.vn_z")));

  addField((wx = new DoubleData("trn_pos.wx")));
  wx->setLongName("Veh angular V wrt an inertial frame"); wx->setUnits("rad/s");
  addField((wy = new DoubleData("trn_pos.wy")));
  addField((wz = new DoubleData("trn_pos.wz")));

  addField((ax = new DoubleData("trn_pos.ax")));
  ax->setLongName("Veh acceleration wrt an inertial frame"); ax->setUnits("m/s^2");
  addField((ay = new DoubleData("trn_pos.ay")));
  addField((az = new DoubleData("trn_pos.az")));

  addField((phi = new DoubleData("trn_pos.phi")));
  phi->setLongName("3-2-1 Euler angles body to NED frame"); phi->setUnits("radians");
  addField((theta = new DoubleData("trn_pos.theta")));
  addField((psi = new DoubleData("trn_pos.psi")));

  addField((dvlValid = new IntegerData("trn_pos.dvlValid")));
  dvlValid->setLongName("Validity flags"); dvlValid->setUnits("bool");
  addField((gpsValid = new IntegerData("trn_pos.gpsValid")));
  addField((bottomLock = new IntegerData("trn_pos.bottomLock")));

}


PositionLog::~PositionLog()
{
  if (ptime) delete ptime;

  if (x) delete x;
  if (y) delete y;
  if (z) delete z;

  if (vx) delete vx;
  if (vy) delete vy;
  if (vz) delete vz;
  if (ve) delete ve;

  if (vw_x) delete vw_x;
  if (vw_y) delete vw_y;
  if (vw_z) delete vw_z;

  if (vn_x) delete vn_x;
  if (vn_y) delete vn_y;
  if (vn_z) delete vn_z;

  if (wx) delete wx;
  if (wy) delete wy;
  if (wz) delete wz;

  if (ax) delete ax;
  if (ay) delete ay;
  if (az) delete az;

  if (phi) delete phi;
  if (theta) delete theta;
  if (psi) delete psi;

  if (dvlValid) delete dvlValid;
  if (gpsValid) delete gpsValid;
  if (bottomLock) delete bottomLock;

}

// This log function takes the place of the setFields()
// function. To log a the data in a poseT simply pass a
// pointer to the poseT object. Avoids declaring this class
// as a friend to a TRN class just to access the poseT in
// a TRN object. This method also allows any process 
//
// Note: One may still use the "friend-class-setFields()"
// method.
//
void PositionLog::log(poseT* pt)
{
  if (pt)
  {
    //printf("PositionLog::setFields()\n");
    ptime->setValue(pt->time);

    x->setValue(   pt->x);
    y->setValue(   pt->y);
    z->setValue(   pt->z);

    vx->setValue(  pt->vx);
    vy->setValue(  pt->vy);
    vz->setValue(  pt->vz);
    ve->setValue(  pt->ve);

    vw_x->setValue(pt->vw_x);
    vw_y->setValue(pt->vw_y);
    vw_z->setValue(pt->vw_z);

    vn_x->setValue(pt->vn_x);
    vn_y->setValue(pt->vn_y);
    vn_z->setValue(pt->vn_z);

    wx->setValue(  pt->wx);
    wy->setValue(  pt->wy);
    wz->setValue(  pt->wz);

    ax->setValue(  pt->ax);
    ay->setValue(  pt->ay);
    az->setValue(  pt->az);

    phi->setValue(  pt->phi);
    theta->setValue(pt->theta);
    psi->setValue(  pt->psi);

    dvlValid->setValue(  pt->dvlValid);
    gpsValid->setValue(  pt->gpsValid);
    bottomLock->setValue(pt->bottomLock);

    write();
  }

}

