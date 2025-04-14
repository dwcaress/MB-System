#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <iomanip>
#include "trnxpp.hpp"
#include "trn_debug.hpp"
#include "trnx_utils.hpp"
#include "GeoCon.h"

#ifndef TRNX_PLUGIN_H
#define TRNX_PLUGIN_H

extern "C" int cb_proto_oisledx(void *pargs);

extern "C" void transform_dvl(trn::bath_info *bi, trn::att_info *ai, dvlgeo *geo, mb1_t *r_snd);
extern "C" int cb_proto_dvl(void *pargs);


extern "C" void transform_deltat(trn::bath_info *bi, trn::att_info *ai, mbgeo *geo, mb1_t *r_snd);
extern "C" int cb_proto_deltat(void *pargs);

extern "C" void transform_oidvl(trn::bath_info **bi, trn::att_info **ai, dvlgeo **geo, mb1_t *r_snd);
extern "C" int cb_proto_oisled(void *pargs);// dec 2022

extern "C" void transform_oidvl2(trn::bath_info **bi, trn::att_info **ai, dvlgeo **geo, mb1_t *r_snd);
extern "C" int cb_proto_oisled2(void *pargs);

void transform_mblass(trn::bath_info **bi, trn::att_info **ai, mbgeo **geo, mb1_t *r_snd);
int cb_proto_mblass(void *pargs);

void transform_xmb1(trn::mb1_info **bi, trn::att_info **ai, mbgeo **geo, mb1_t *r_snd);
int cb_proto_xmb1(void *pargs);

int transform_idtlass(trn::bath_info **bi, trn::att_info **ai, beam_geometry **geo, mb1_t *r_snd);
int cb_proto_idtlass(void *pargs);

int transform_mbminirov(trn::bath_info **bi, trn::att_info **ai, beam_geometry **geo, mb1_t *r_snd);
int cb_proto_mbminirov(void *pargs);

typedef struct sled_rofs_s {
    double Xo;
    double Yo;
    double Zo;
    double Ro;
    double Wo;
    double Xr;
    double Yr;
    double Zr;
    double dX;
    double dY;
    double dZ;
    double Wa;
    double Wr;
    double Ax;
    double Ay;
    double Az;
}sled_rofs_t;

class TrnxPlugin
{
public:
    TrnxPlugin(){}
    ~TrnxPlugin(){}

    static void register_callbacks(trn::trnxpp &xpp)
    {
        xpp.register_callback("cb_proto_dvl", cb_proto_dvl);
        xpp.register_callback("cb_proto_deltat", cb_proto_deltat);
        xpp.register_callback("cb_proto_oisled", cb_proto_oisled);
        xpp.register_callback("cb_proto_oisled2", cb_proto_oisled2);
        xpp.register_callback("cb_proto_oisledx", cb_proto_oisledx);
//        xpp.register_callback("cb_proto_mblass", cb_proto_oisledx);
        xpp.register_callback("cb_proto_mblass", cb_proto_mblass);
        xpp.register_callback("cb_proto_xmb1", cb_proto_xmb1);
        xpp.register_callback("cb_proto_idtlass", cb_proto_idtlass);
        xpp.register_callback("cb_proto_mbminirov", cb_proto_mbminirov);
    }

    // calculate change in sled nav position due to arm rotation
    // when the arm is unrotated, the nav sensor is located at
    // Xo, Zo from the axis of rotation (vehicle/sled frame, +X: FWD +Z: Down).
    // It rotates from initial position, Xo,Zo to Xr, Zr (Y is unchanged).
    // The distance it has moved, dX, dY (=0), dZ, is the difference between
    // the rotated and unroated positions.
    // The sled arm and vehicle have an attitude outputs.
    // The actual arm angle (world) is the different betwween the arm pitch and vehicle pitch
    // the arm angle wrt horizontal (world) is the un-rotated angle (Atan(Zo/Xo)) plus the arm rotation
    // Inputs:
    // sled_nav_geo: location wrt vehicle origin
    //               expects xmap:NAx, NAy, NAz offset wrt center of nav rotation
    //               undefined values are 0.
    // veh_att: vehicle roll/pitch/heading
    // sled_att: sled roll/pitch/heading
    static void sled_nav_rot_offsets(trn::att_info *veh_att, trn::att_info *sled_att, beam_geometry *sled_nav_geo, sled_rofs_t *r_offset) {

        // validate inputs
        if(veh_att == nullptr || sled_att == nullptr || sled_nav_geo == nullptr || r_offset == nullptr) {
            std::cerr << __func__ << ": NULL input: veh_att %p sled_att %p sled_nav_geo %p  r_offset %p" << veh_att << sled_att << sled_nav_geo << r_offset << std::endl;
            return;
        }

        double Xo = sled_nav_geo->xmap["NAx"];
        double Yo = sled_nav_geo->xmap["NAy"];
        double Zo = sled_nav_geo->xmap["NAz"];

        // nav center of rotation relative to vehicle origin
        double Ax = sled_nav_geo->tr_m(0) + Xo;
        double Ay = sled_nav_geo->tr_m(1);
        double Az = sled_nav_geo->tr_m(2) - Zo;

        // r: nav sensor rotation radius
        double Ro = sqrt(Xo * Xo + Zo * Zo);
        // Wo: nav sensor un-rotated angle wrt horizontal through rotation axis
        double Wo = atan2(Zo, Xo);

        // Wa: arm tilt offset: (arm_tilt - vehicle_pitch)
        //    double Wa = ai[1]->pitch() - ai[0]->pitch();
        // force to zero (no ARM present)
        double Wa = 0;

        if(sled_att != nullptr) {
            Wa = sled_att->pitch() - veh_att->pitch();
        }

        // Wr: sled tilt angle wrt horizontal
        double Wr = (Wo + Wa);
        // Zr,Yr,Zr : nav sensor (rotated) location
        double Xr = Ro * cos(Wr);
        double Yr = Yo;
        double Zr = Ro * sin(Wr);
        // sensor::nav position offsets due to arm rotation
        double dX = Xr - Xo;
        double dY = Yr - Yo;
        double dZ = Zo - Zr;

        if(r_offset != nullptr) {
            r_offset->Xo = Xo;
            r_offset->Yo = Yo;
            r_offset->Zo = Zo;
            r_offset->Ro = Ro;

            r_offset->Xr = Xr;
            r_offset->Yr = Yr;
            r_offset->Zr = Zr;

            r_offset->dX = dX;
            r_offset->dY = dY;
            r_offset->dZ = dZ;

            r_offset->Wo = Wo;
            r_offset->Wa = Wa;
            r_offset->Wr = Wr;

            r_offset->Ax = Ax;
            r_offset->Ay = Ay;
            r_offset->Az = Az;
        }
    }

    // adjust LASS MB1 sounding navigation (lat, lon, depth)
    // for the offset between the nav (on OI sled rotating arm) and bath (on vehicle)
    // ai[0]  : veh attitude
    // ai[1]  : sled attitude
    // geo[0] : veh bath geo (wrt vehicle origin)
    // geo[1] : sled nav geo (wrt vehicle origin)
    // geocon : coordinate converter
    // r_snd  : sounding
    static void adjust_mb1_nav_rotating(trn::att_info **ai, beam_geometry **geo, GeoCon *gcon, mb1_t *r_snd)
    {
        // validate inputs
        if(ai == nullptr ||  geo == nullptr || gcon == nullptr || r_snd == nullptr) {
            std::cerr << __func__ << ": NULL input: ai %p geo %p gcon %p r_snd %p" << ai << geo << gcon << r_snd << std::endl;
            return;
        }
        if(ai[0] == nullptr || ai[1] == nullptr) {
            std::cerr << __func__ << ": NULL attitude: ai[0] %p ai[1] %p" << ai[0] << ai[1] << std::endl;
            return;
        }
        if(geo[0] == nullptr || geo[1] == nullptr) {
            std::cerr << __func__ << ": NULL geo: geo[0] %p geo[1] %p" << geo[0] << geo[1] << std::endl;
            return;
        }

        trn::att_info *veh_att = ai[0];
        trn::att_info *sled_att = ai[1];
        beam_geometry *veh_bath_geo = geo[0];
        beam_geometry *sled_nav_geo = geo[1];

        // get nav coordinate offsets due to arm rotation (wrt vehicle origin/CRP)
        sled_rofs_t sled_ofs = {0}, *pofs = &sled_ofs;
        TrnxPlugin::sled_nav_rot_offsets(veh_att, sled_att, sled_nav_geo, pofs);

        // nav position, adjusted for arm rotation (relative to vehicle origin)
        double Nx = sled_nav_geo->tr_m(0) + pofs->dX;
        double Ny = sled_nav_geo->tr_m(1) + pofs->dY;
        double Nz = sled_nav_geo->tr_m(2) + pofs->dZ;
        // bath position (relative to vehicle origin)
        double Bx = veh_bath_geo->tr_m(0);
        double By = veh_bath_geo->tr_m(1);
        double Bz = veh_bath_geo->tr_m(2);
        // bath offset relative to nav, (vehicle unrotated)
        double dE = Nx - Bx;
        double dN = Ny - By;
        double dZ = Nz - Bz;
        // heading (negated for cartesian rotation about Z)
        double H = -veh_att->heading();

        // transform vehicle frame offsets Bx,By to world Se,Sn
        double vNavLoc[3] = {Nx, Ny, Nz};
        double vBathLoc[3] = {Bx, By, Bz};

        // get rotated coordinates of nav, bath (world frame)
        Matrix mBathLocWF = trnx_utils::affine2DRotatePoint(DTR(H), vBathLoc);
        Matrix mNavLocWF = trnx_utils::affine2DRotatePoint(DTR(H), vNavLoc);

        // rotated nav,bath coordinates (world frame)
        double Nyr = mNavLocWF(1,1);
        double Nxr = mNavLocWF(2,1);
        double Nzr = mNavLocWF(3,1);
        double Byr = mBathLocWF(1,1);
        double Bxr = mBathLocWF(2,1);
        double Bzr = mBathLocWF(3,1);

        // offset between Bath, Nav (Northings, Eastings)
        double dEr = Bxr - Nxr;
        double dNr = Byr - Nyr;
        double dZr = Bzr - Nzr + veh_bath_geo->xmap["depthOfs"];

        double lat_orig = r_snd->lat;
        double lon_orig = r_snd->lon;

        double snd_north = 0.;
        double snd_east = 0.;

        // convert sounding lat/lon to mercator projection (cartesian)
        gcon->geo_to_mp(DTR(r_snd->lat), DTR(r_snd->lon), &snd_north, &snd_east);
        // add offset
        snd_north += dNr;
        snd_east += dEr;

        double lat_rad = 0.;
        double lon_rad = 0.;
        // convert to lat/lon
        gcon->mp_to_geo(snd_north, snd_east, &lat_rad, &lon_rad);

        // update souding LAT/LON
        r_snd->lat = RTD(lat_rad);
        r_snd->lon = RTD(lon_rad);

        // update sounding depth
        r_snd->depth += dZr ;

        std::cerr << "N: {" << Nx << ", " << Ny << ", " << Nz << "} mag: " << sqrt((Nx*Nx) + (Ny*Ny)) << std::endl;
        std::cerr << "B: {" << Bx << ", " << By << ", " << Bz << "} mag: " << sqrt((Bx*Bx) + (By*By)) << std::endl;
        std::cerr << "H: " << H << " (" << DTR(H) << ") " << std::endl;
        std::cerr << "R: " << sqrt((Bx-Nx)*(Bx-Nx) + (By-Ny)*(By-Ny)) << std::endl;
        std::cerr << "dE: " << dE << " dN:" << dN << " dZ:" << dZ << std::endl;

        std::cerr << "pofs dX, dY, dZ: " << "{" << pofs->dX << ", " << pofs->dY << ", " << pofs->dZ << "}" << std::endl;

        std::cerr << "Nxr, Nyr, Nzr: {" << Nxr << ", " << Nyr << ", " << Nzr << "}" << std::endl;
        std::cerr << "Bxr, Byr, Bzr: {" << Bxr << ", " << Byr << ", " << Bzr << "}" << std::endl;

        std::cerr << "Nr {" << Nxr << ", " << Nyr << ", " << Nzr << "} mag: " << sqrt((Nxr*Nxr) + (Nyr*Nyr)) << std::endl;
        std::cerr << "Br {" << Bxr << ", " << Byr << ", " << Bzr << "} mag: " << sqrt((Bxr*Bxr) + (Byr*Byr)) << std::endl;
        std::cerr << "Rr: " << sqrt((Bxr-Nxr)*(Bxr-Nxr) + (Byr-Nyr)*(Byr-Nyr)) << std::endl;
        std::cerr << "depthOfs: " << veh_bath_geo->xmap["depthOfs"] << std::endl;
        std::cerr << "dEr: " << dEr << " dNr: " << dNr << " dZr: " << dZr <<  std::endl;
        std::cerr << "Lat/Lon before/after: " << "[" << std::fixed << std::setprecision(6) << lat_orig << ", " << lon_orig <<"] / [" << r_snd->lat << ", " << r_snd->lon << "]" <<  std::dec << std::setprecision(3) << std::endl;

    }

    // adjust MB1 sounding navigation (lat, lon, depth)
    // for the offset between (non-rotating) nav and bath (on vehicle)
    // ai[0]  : veh attitude
    // ai[1]  : sled attitude
    // geo[0] : veh bath geo (wrt vehicle origin)
    // geo[1] : sled nav geo (wrt vehicle origin)
    // geocon : coordinate converter
    // r_snd  : sounding
    static void adjust_mb1_nav_fixed(trn::att_info **ai, beam_geometry **geo, GeoCon *gcon, mb1_t *r_snd)
    {
        // validate inputs
        if(ai == nullptr || geo == nullptr || gcon == nullptr || r_snd == nullptr) {
            std::cerr << __func__ << ": NULL input: ai %p geo %p gcon %p r_snd %p" << ai << geo << gcon << r_snd << std::endl;
            return;
        }
        if(ai[0] == nullptr || ai[1] == nullptr) {
            std::cerr << __func__ << ": NULL attitude: ai[0] %p ai[1] %p" << ai[0] << ai[1] << std::endl;
            return;
        }
        if(geo[0] == nullptr || geo[1] == nullptr) {
            std::cerr << __func__ << ": NULL geo: geo[0] %p geo[1] %p" << geo[0] << geo[1] << std::endl;
            return;
        }

        trn::att_info *veh_att = ai[0];
        trn::att_info *sled_att = ai[1];
        beam_geometry *veh_bath_geo = geo[0];
        beam_geometry *sled_nav_geo = geo[1];

        // nav position, adjusted for arm rotation (relative to vehicle origin)
        double Nx = sled_nav_geo->tr_m(0);
        double Ny = sled_nav_geo->tr_m(1);
        double Nz = sled_nav_geo->tr_m(2);
        // bath position (relative to vehicle origin)
        double Bx = veh_bath_geo->tr_m(0);
        double By = veh_bath_geo->tr_m(1);
        double Bz = veh_bath_geo->tr_m(2);
        // bath offset relative to nav, (vehicle unrotated)
        double dE = Nx - Bx;
        double dN = Ny - By;
        double dZ = Nz - Bz;
        // heading (negated for cartesian rotation about Z)
        double H = -veh_att->heading();

        // transform vehicle frame offsets Bx,By to world Se,Sn
        double vNavLoc[3] = {Nx, Ny, Nz};
        double vBathLoc[3] = {Bx, By, Bz};

        // get rotated coordinates of nav, bath (world frame)
        Matrix mBathLocWF = trnx_utils::affine2DRotatePoint(DTR(H), vBathLoc);
        Matrix mNavLocWF = trnx_utils::affine2DRotatePoint(DTR(H), vNavLoc);

        // rotated nav,bath coordinates (world frame)
        double Nyr = mNavLocWF(1,1);
        double Nxr = mNavLocWF(2,1);
        double Nzr = mNavLocWF(3,1);
        double Byr = mBathLocWF(1,1);
        double Bxr = mBathLocWF(2,1);
        double Bzr = mBathLocWF(3,1);

        // offset between Bath, Nav (Northings, Eastings)
        double dEr = Bxr - Nxr;
        double dNr = Byr - Nyr;
        double dZr = Bzr - Nzr + veh_bath_geo->xmap["depthOfs"];

        double snd_north = 0.;
        double snd_east = 0.;

        // convert sounding lat/lon to mercator projection (cartesian)
        gcon->geo_to_mp(DTR(r_snd->lat), DTR(r_snd->lon), &snd_north, &snd_east);
        // add offset
        snd_north += dNr;
        snd_east += dEr;

        double lat_rad = 0.;
        double lon_rad = 0.;
        // convert to lat/lon
        gcon->mp_to_geo(snd_north, snd_east, &lat_rad, &lon_rad);

        // update souding LAT/LON
        r_snd->lat = RTD(lat_rad);
        r_snd->lon = RTD(lon_rad);

        // update sounding depth
        r_snd->depth += dZr ;

        std::cerr << "N: {" << Nx << ", " << Ny << ", " << Nz << "} mag: " << sqrt((Nx*Nx) + (Ny*Ny)) << std::endl;
        std::cerr << "B: {" << Bx << ", " << By << ", " << Bz << "} mag: " << sqrt((Bx*Bx) + (By*By)) << std::endl;
        std::cerr << "H: " << H << " (" << DTR(H) << ") " << std::endl;
        std::cerr << "R: " << sqrt((Bx-Nx)*(Bx-Nx) + (By-Ny)*(By-Ny)) << std::endl;
        std::cerr << "dE: " << dE << " dN:" << dN << " dZ:" << dZ << std::endl;

        std::cerr << "Nxr, Nyr, Nzr: {" << Nxr << ", " << Nyr << ", " << Nzr << "}" << std::endl;
        std::cerr << "Bxr, Byr, Bzr: {" << Bxr << ", " << Byr << ", " << Bzr << "}" << std::endl;

        std::cerr << "Nr {" << Nxr << ", " << Nyr << ", " << Nzr << "} mag: " << sqrt((Nxr*Nxr) + (Nyr*Nyr)) << std::endl;
        std::cerr << "Br {" << Bxr << ", " << Byr << ", " << Bzr << "} mag: " << sqrt((Bxr*Bxr) + (Byr*Byr)) << std::endl;
        std::cerr << "Rr: " << sqrt((Bxr-Nxr)*(Bxr-Nxr) + (Byr-Nyr)*(Byr-Nyr)) << std::endl;
        std::cerr << "depthOfs: " << veh_bath_geo->xmap["depthOfs"] << std::endl;
        std::cerr << "dEr: " << dEr << " dNr: " << dNr << " dZr: " << dZr <<  std::endl;

    }
};

#endif
