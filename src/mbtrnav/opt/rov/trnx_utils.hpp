

/// @file trnx_utils.cpp
/// @authors k. headley
/// @date 21mar2022

/// Summary: utiliites for trnxpp and applications

// ///////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute
// Distributed under MIT license. See LICENSE file for more information.

#ifndef TRNX_UTILS_H
#define TRNX_UTILS_H

// /////////////////
// Includes
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <math.h>
#include <string.h>
#include <memory.h>
#include <tuple>
#include <libgen.h>
// initalizers for header-only modules
// must be defined before any framework headers are included
//#define INIT_TRN_LCM_INPUT
#include "trn_lcm_input.hpp"
#include "raw_signal_input.hpp"

#include "dvl_stat_input.hpp"
#include "rdi_dvl_input.hpp"
#include "nav_solution_input.hpp"
#include "idt_input.hpp"
#include "pcomms_input.hpp"
#include "rdi_pd4_input.hpp"
#include "kearfott_input.hpp"
#include "octans_input.hpp"
#include "structDefs.h"
#include "MathP.h"
#include "newmat.h"
#include "trn_msg.h"
#include "mb1_msg.h"
#include "udpm_sub.h"
#include "trn_debug.hpp"
#include "geo_cfg.hpp"

// /////////////////
// Macros


#ifndef DTR
#define DTR(x) ((x) * M_PI/180.)
#endif
#ifndef RTD
#define RTD(x) ((x) * 180./M_PI)
#endif


// /////////////////
// Types

// /////////////////
// Module variables

// /////////////////
// Declarations

class trnx_utils
{
public:
    trnx_utils()
    {}

    ~trnx_utils()
    {}

    static void trnest_tostream(std::ostream &os, double &ts, poseT &pt, poseT &mle, poseT &mmse, int wkey, int wval)
    {
        os << "--- TRN Estimate OK---" << "\n";
        os << "MLE[t, tm, x, y, z]  ";
        os << std::fixed << std::setprecision(3);
        os << ts << ", ";
        os << std::setprecision(2);
        os << mle.time << ", ";
        os << std::setprecision(4);
        os << mle.x << ", " << mle.y << ", " << mle.z << "\n";

        os << "MMSE[t, tm, x, y, z] ";
        os << std::fixed << std::setprecision(3);
        os << ts << ", ";
        os << std::setprecision(2);
        os << mmse.time << ", ";
        os << std::setprecision(4);
        os << mmse.x << ", " << mmse.y << ", " << mmse.z << "\n";

        os << "POS[t, tm, x, y, z]  ";
        os << std::fixed << std::setprecision(3);
        os << ts << ", ";
        os << std::setprecision(2);
        os << mmse.time << ", ";
        os << std::setprecision(4);
        os << pt.x << ", " << pt.y << ", " << pt.z << "\n";

        os << "OFS[t, tm, x, y, z]  ";
        os << std::fixed << std::setprecision(3);
        os << ts << ", ";
        os << std::setprecision(2);
        os << mmse.time << ", ";
        os << std::setprecision(4);
        os << pt.x-mmse.x << ", " << pt.y-mmse.y << ", " << pt.z-mmse.z << "\n";

        os << "COV[t, x, y, z, m]   ";
        os << std::setprecision(3);
        os << mmse.time << ", ";
        os << std::setprecision(2);
        os << mmse.covariance[0] << ", ";
        os << mmse.covariance[2] << ", ";
        os << mmse.covariance[5] << ", ";
        double ss = mmse.covariance[0] * mmse.covariance[0];
        ss += mmse.covariance[2] * mmse.covariance[2];
        ss += mmse.covariance[5] * mmse.covariance[5];
        os << sqrt(ss) << "\n";

        os << "s[t, x, y, z]        ";
        os << std::setprecision(3);
        os << mmse.time << ", ";
        os << std::setprecision(2);
        os << sqrt(mmse.covariance[0]) << ", ";
        os << sqrt(mmse.covariance[2]) << ", ";
        os << sqrt(mmse.covariance[5]) << "\n";

    }

    static std::string  trnest_tostring(double &time, poseT &pt, poseT &mle, poseT &mmse, int wkey=15, int wval=18)
    {
        std::ostringstream ss;
        trnest_tostream(ss, time, pt, mle, mmse, wkey, wval);
        return ss.str();
    }

    static void trnest_show(double &time, poseT &pt, poseT &mle, poseT &mmse, int wkey=15, int wval=18)
    {
        trnest_tostream(std::cerr, time, pt, mle, mmse, wkey, wval);
    }

    static void mbest_tostream(std::ostream &os, trnu_pub_t *mbest, int wkey=15, int wval=18)
    {
        os << "--- MB Update OK---" << "\n";

        os << std::fixed << std::setprecision(3);
        os << "POS [t, x, y, z, cov(0, 2, 5, 1)]:";
        os << std::setprecision(3);
        os << mbest->est[0].time << ", ";
        os << mbest->est[0].x << ", " << mbest->est[0].y << ", " << mbest->est[0].z;
        os << mbest->est[0].cov[0] << ", " << mbest->est[0].cov[1] << ", ";
        os << mbest->est[0].cov[2] << ", " << mbest->est[0].cov[3] << "\n";

        os << "MLE [t, x, y, z, cov(0, 2, 5, 1)]:";
        os << std::setprecision(3);
        os << mbest->est[1].time << ", ";
        os << mbest->est[1].x << ", " << mbest->est[1].y << ", " << mbest->est[0].z;
        os << mbest->est[1].cov[0] << ", " << mbest->est[1].cov[1] << ", ";
        os << mbest->est[1].cov[2] << ", " << mbest->est[1].cov[3] << "\n";

        os << std::fixed << std::setprecision(3);
        os << "MMSE [t, x, y, z, cov(0, 2, 5, 1)]:";
        os << std::setprecision(3);
        os << mbest->est[2].time << ", ";
        os << mbest->est[2].x << ", " << mbest->est[2].y << ", " << mbest->est[0].z;
        os << mbest->est[2].cov[0] << ", " << mbest->est[2].cov[1] << ", ";
        os << mbest->est[2].cov[2] << ", " << mbest->est[2].cov[3] << "\n";

        os << std::setw(wkey) << "reinit_count:" << std::setw(wkey) << mbest->reinit_count << "\n";
        os << std::setw(wkey) << "reinit_tlast:" << std::setw(wkey) << mbest->reinit_tlast << "\n";
        os << std::setw(wkey) << "filter_state:" << std::setw(wkey) << mbest->filter_state << "\n";
        os << std::setw(wkey) << "success:" << std::setw(wkey) << mbest->success << "\n";
        os << std::setw(wkey) << "is_converged:" << std::setw(wkey) << mbest->is_converged << "\n";
        os << std::setw(wkey) << "is_valid:" << std::setw(wkey) << mbest->is_valid << "\n";
        os << std::setw(wkey) << "mb1_cycle:" << std::setw(wkey) << mbest->mb1_cycle << "\n";
        os << std::setw(wkey) << "ping_number:" << std::setw(wkey) << mbest->ping_number << "\n";
        os << std::setw(wkey) << "mb1_time:" << std::setw(wkey) << mbest->mb1_time << "\n";
        os << std::setw(wkey) << "update_time:" << std::setw(wkey) << mbest->update_time << "\n";
    }

    static std::string mbest_tostring(trnu_pub_t *mbest, int wkey=15, int wval=18)
    {
        std::ostringstream ss;
        mbest_tostream(ss, mbest, wkey, wval);
        return ss.str();
    }

    static void mbest_show(trnu_pub_t *mbest, int wkey=15, int wval=18)
    {
        mbest_tostream(std::cerr, mbest, wkey, wval);
    }

    // 321 euler rotation R(phi, theta, psi)
    // where
    // phi:roll (rotation about X)
    // theta:pitch (rotation about Y)
    // psi:yaw (rotation about Z)
    // expects
    // attitude[0]: phi / roll (rad)
    // attitude[1]: theta / pitch (rad)
    // attitude[2]: psi / yaw (rad)
    static Matrix applyRotation(const double* attitude,  const Matrix& beamsVF)
    {
        Matrix beamsMF = beamsVF;
        double cphi = cos(attitude[0]);
        double sphi = sin(attitude[0]);
        double ctheta = cos(attitude[1]);
        double stheta = sin(attitude[1]);
        double cpsi = cos(attitude[2]);
        double spsi = sin(attitude[2]);
        double stheta_sphi = stheta * sphi;
        double stheta_cphi = stheta * cphi;

        double R11 = cpsi * ctheta;
        double R12 = spsi * ctheta;
        double R13 = -stheta;
        double R21 = -spsi * cphi + cpsi * stheta_sphi;
        double R22 = cpsi * cphi + spsi * stheta_sphi;
        double R23 = ctheta * sphi;
        double R31 = spsi * sphi + cpsi * stheta_cphi;
        double R32 = -cpsi * sphi + spsi * stheta_cphi;
        double R33 = ctheta * cphi;

        for(int i = 1; i <= beamsVF.Ncols(); i++) {
            beamsMF(1, i) = R11 * beamsVF(1, i) + R21 * beamsVF(2, i) + R31 * beamsVF(3, i);

            beamsMF(2, i) = R12 * beamsVF(1, i) + R22 * beamsVF(2, i) + R32 * beamsVF(3, i);

            beamsMF(3, i) = R13 * beamsVF(1, i) + R23 * beamsVF(2, i) + R33 * beamsVF(3, i);
        }

        return beamsMF;
    }

    static Matrix applyTranslation(const double* translation,  const Matrix& beamsVF)
    {
        Matrix beamsMF = beamsVF;

        for(int i = 1; i <= beamsVF.Ncols(); i++) {
            beamsMF(1, i) = beamsVF(1, i) + translation[0];

            beamsMF(2, i) = beamsVF(2, i) + translation[1];

            beamsMF(3, i) = beamsVF(3, i) + translation[2];
        }

        return beamsMF;

    }

    static Matrix mb_vframe_components(trn::bath_info *bi, mbgeo *geo)
    {

        if(bi == nullptr || geo == NULL){
            Matrix err_ret = Matrix(3,1);
        }

        // number of beams read (<= nominal beams)
        int nbeams = bi->beam_count();

        Matrix vf_comp = Matrix(3,nbeams);

        // beam swath angle
        double S = geo->swath_deg;
        // angle between PI and start angle
        double K = (180. - S)/2.;
        // beam angle increment
        double e = S/geo->beam_count;

        // beam components in reference sensor frame (mounted center, across track)

        std::list<trn::beam_tup> beams = bi->beams_raw();
        std::list<trn::beam_tup>::iterator it;

        // zero- and one-based indexs
        int idx[2] = {0, 1};

        TRN_NDPRINT(5, "%s: --- \n",__func__);
        TRN_NDPRINT(5, "S[%.3lf] K[%.3lf] e[%.3lf]\n", S, K, e);

        for(it=beams.begin(); it!=beams.end(); it++)
        {
            trn::beam_tup bt = static_cast<trn::beam_tup> (*it);
            double range = std::get<1>(bt);
            // beam number (0-indexed)
            int b = std::get<0>(bt);
            // ad : ith beam angle (deg)
            double ad = K + S - b*e;

            vf_comp(1, idx[1]) = 0;
            vf_comp(2, idx[1]) = cos(DTR(ad));
            vf_comp(3, idx[1]) = sin(DTR(ad));

            TRN_NDPRINT(5, "n[%3d] R[%7.2lf] X[%7.2lf] Y[%7.2lf] Z[%7.2lf] ad[%7.2lf] ar[%7.2lf]\n",
                        b,range,
                        vf_comp(1, idx[1]), vf_comp(2, idx[1]), vf_comp(3, idx[1]),
                        ad,DTR(ad)
                        );
            idx[0]++;
            idx[1]++;
        }
        TRN_NDPRINT(5, "%s: --- \n",__func__);

        return vf_comp;
    }

    static Matrix dvl_vframe_components(trn::bath_info *bi, dvlgeo *geo)
    {

        if(bi == nullptr || geo == NULL){
            Matrix err_ret = Matrix(3,1);
        }

        // number of beams read (<= nominal beams)
        int nbeams = bi->beam_count();

        Matrix vf_comp = Matrix(3,nbeams);

        // beam components in reference sensor frame (mounted center, across track)

        std::list<trn::beam_tup> beams = bi->beams_raw();
        std::list<trn::beam_tup>::iterator it;

        // zero- and one-based indexs
        int idx[2] = {0, 1};

        for(it=beams.begin(); it!=beams.end(); it++)
        {
            trn::beam_tup bt = static_cast<trn::beam_tup> (*it);
            double range = std::get<1>(bt);
            // beam number (0-indexed)
            int b = std::get<0>(bt);
            // beam components SF x,y,z
            // ad : ith beam angle (deg)
            double yd = geo->yaw_rf[idx[0]];
            double pd = geo->pitch_rf[idx[0]];
            double yr = DTR(yd);
            double pr = DTR(pd);

            // beam components SF
            // 1: along 2: across 3: down
            vf_comp(1, idx[1]) = sin(pr)*cos(yr);
            vf_comp(2, idx[1]) = sin(pr)*sin(yr);
            vf_comp(3, idx[1]) = cos(pr);

            TRN_NDPRINT(5, "%s - b[%3d] R[%7.2lf] Rx[%7.2lf] Ry[%7.2lf] Rz[%7.2lf] y[%7.2lf/%7.2lf] p[%7.2lf/%7.2lf] cosy[%7.2lf] siny[%7.2lf] cosp[%7.2lf] sinp[%7.2lf]\n",
                        __func__, b, range,
                        vf_comp(1,idx[1]), vf_comp(2,idx[1]), vf_comp(3,idx[1]), yd, yr, pd, pr, cos(yr), sin(yr), cos(pr), sin(pr));

            idx[0]++;
            idx[1]++;
        }

        return vf_comp;
    }

    // process DVL sounding from ocean imaging toolsled (mounted on rotating arm)
    // It probably doesn't make sense to filter DVL beams
    // using mbtrnpp, since it assumes they are distributed
    // in a linear array.
    static void transform_oidvl(trn::bath_info **bi, trn::att_info **ai, dvlgeo **geo, mb1_t *r_snd)
    {
        // validate inputs
        if(NULL == geo || geo[0] == nullptr || geo[1] == nullptr){
            fprintf(stderr, "%s - geometry error : NULL input geo[%p] {%p, %p} \n", __func__, geo, (geo?geo[0]:nullptr), (geo?geo[1]:nullptr));
            return;
        }
        if(geo[0]->beam_count <= 0 || geo[1]->beam_count <= 0){
            fprintf(stderr, "%s - geometry error : beams <= 0 {%u, %u}\n", __func__, geo[0]->beam_count, geo[1]->beam_count);
            return;
        }
        if(NULL == r_snd || NULL == ai|| NULL == bi){
            fprintf(stderr, "%s - ERR invalid argument bi[%p] ai[%p] snd[%p]\n", __func__, bi, ai, r_snd);
            return;
        }

        if(NULL == ai[0] || NULL == ai[1] || NULL == bi[0] || NULL == bi[1]){
            fprintf(stderr, "%s - ERR invalid info ai[0][%p] ai[1][%p] bi[0][%p] bi[1][%p] \n", __func__, ai[0], ai[1], bi[0], bi[1]);
            return;
        }

        // number of beams read (<= nominal beams)
        int nbeams = bi[1]->beam_count();

        // vehicle attitude (relative to NED)
        // r/p/y (phi/theta/psi)
        // MB1 assumes vehicle frame, not world frame (i.e. exclude heading)
        double VW[3] = {ai[1]->roll(), ai[1]->pitch(), 0.};

        // sensor mounting angles (relative to vehicle, radians)
        // 3-2-1 euler angles, r/p/y  (phi/theta/psi)
        // wrt sensor mounted across track, b[0] port, downward facing
        double RSV[3] = { 0., 0., 0.};

        // sensor mounting translation offsets (relative to vehicle CRP, meters)
        // +x: fwd +y: stbd, +z:down (aka FSK, fwd,stbd,keel)
        // TODO T is for transform use rSV
        double TSV[3] = {0., 0., 0.};

        // measured vehicle pitch
        double Pv = ai[0]->pitch();
        // measured arm pitch
        double Pa = ai[1]->pitch();
        // vehicle att sensor mounting pitch offset
        double Pov = geo[0]->svr_deg[2];
        // arm att sensor mounting pitch offset
        double Poa = geo[1]->svr_deg[2];

        // arm att sensor mounting X offset
        double xo = geo[1]->svt_m[0];
        // arm att sensor mounting Y offset
        double yo = geo[1]->svt_m[1];
        // arm att sensor mounting Z offset
        double zo = geo[1]->svt_m[2];

        // arm att sensor mounting roll offset
        double Ro = geo[1]->svr_deg[0];
        // arm att sensor mounting pitch offset
        double Po = geo[1]->svr_deg[1];
        // arm att sensor mounting yaw offset
        double Yo = geo[1]->svr_deg[2];

        // X distance from arm att sensor to rotation axis
        double D = geo[1]->rot_radius_m;

        // find arm angle by differencing arm and vehicle attitudes,
        // adjusted for fixed reference (geo) mounting pitch offsets
        // Note: sign inverted since +Q is -pitch
        double Qd = (Pv - Pov) - (Pa - Poa);
        double Qr = DTR(Qd);

        // The mounting offsets vary with arm angle
        // translation offsets (m)
        TSV[0] = xo - (D * (1 - cos(Qr)));
        TSV[1] = yo;
        TSV[2] = zo + (D * sin(Qr));

        // rotation offsets (radians)
        RSV[0] = DTR(Ro);
        RSV[1] = DTR(Po - Qd);
        RSV[2] = DTR(Yo);

        if(trn_debug::get()->debug() >= 5){
            fprintf(stderr, "%s:%d geo[0]:\n%s\n", __func__, __LINE__, geo[0]->tostring().c_str());
            fprintf(stderr, "%s:%d geo[1]:\n%s\n", __func__, __LINE__, geo[1]->tostring().c_str());
            fprintf(stderr, "%s:%d nbeams[%d]\n", __func__, __LINE__, nbeams);
            fprintf(stderr, "%s:%d VW[%.2lf, %.2lf, %.2lf]\n", __func__, __LINE__, VW[0], VW[1], VW[2]);
            fprintf(stderr, "%s:%d Pv[%.2lf] Pa[%.2lf]\n", __func__, __LINE__, Pv, Pa);
            fprintf(stderr, "%s:%d Pov[%.2lf] Poa[%.2lf]\n", __func__, __LINE__, Pov, Poa);
            fprintf(stderr, "%s:%d xo,yo,zo[%.2lf, %.2lf, %.2lf]\n", __func__, __LINE__, xo, yo, zo);
            fprintf(stderr, "%s:%d Ro,Po,Yo[%.2lf, %.2lf, %.2lf]\n", __func__, __LINE__, Ro, Po, Yo);
            fprintf(stderr, "%s:%d D[%.2lf]\n", __func__, __LINE__, D);
            fprintf(stderr, "%s:%d Qd[%.2lf] Qr[%.2lf]\n", __func__, __LINE__, Qd, Qr);
            fprintf(stderr, "%s:%d RSV[%.2lf, %.2lf, %.2lf]\n", __func__, __LINE__, RSV[0], RSV[1], RSV[2]);
            fprintf(stderr, "%s:%d TSV[%.2lf, %.2lf, %.2lf]\n", __func__, __LINE__, TSV[0], TSV[1], TSV[2]);
        }

        // beam components in reference sensor frame (mounted center, across track)
        Matrix comp_BVF = dvl_vframe_components(bi[1], geo[1]);

        std::list<trn::beam_tup> beams = bi[1]->beams_raw();
        std::list<trn::beam_tup>::iterator it;

        // zero- and one-based indexs
        int idx[2] = {0, 1};

        // apply coordinate transforms; order is significant:
        // apply mounting rotation
        Matrix beams_VF = applyRotation(RSV, comp_BVF);
        // apply mounting translation
        Matrix beams_TF = applyTranslation(TSV, beams_VF);
        // apply vehicle attitude (hdg, pitch, roll)
        Matrix beams_WF = applyRotation(VW, beams_TF);

        // fill in the MB1 record using transformed beams
        int k=0;
        idx[0] = 0;
        idx[1] = 1;
        for(it=beams.begin(); it!=beams.end(); it++,k++)
        {
            trn::beam_tup bt = static_cast<trn::beam_tup> (*it);
            // beam number (1-indexed for RDI)
            int b = std::get<0>(bt);
            double range = std::get<1>(bt);
            // beam components WF x,y,z

            r_snd->beams[k].beam_num = b;
            r_snd->beams[k].rhox = range * beams_WF(1, idx[0]);
            r_snd->beams[k].rhoy = range * beams_WF(2, idx[0]);
            r_snd->beams[k].rhoz = range * beams_WF(3, idx[0]);

            TRN_NDPRINT(5, "b[%3d] R[%7.2lf] rhox[%7.2lf] rhoy[%7.2lf] rhoz[%7.2lf] \n",
                        b,sqrt(r_snd->beams[k].rhox*r_snd->beams[k].rhox + r_snd->beams[k].rhoy*r_snd->beams[k].rhoy + r_snd->beams[k].rhoz*r_snd->beams[k].rhoz),
                        r_snd->beams[k].rhox,
                        r_snd->beams[k].rhoy,
                        r_snd->beams[k].rhoz
                        );
            idx[0]++;
            idx[1]++;
    }
        return;
    }

    // It probably doesn't make sense to filter DVL beams
    // using mbtrnpp, since it assumes they are distributed
    // in a linear array.
    static void transform_dvl(trn::bath_info *bi, trn::att_info *ai, dvlgeo *geo, mb1_t *r_snd)
    {
        if(NULL == geo || geo->beam_count<=0){
            fprintf(stderr, "%s - geometry error : beams<=0\n", __func__);
            return;
        }
        if(NULL == r_snd || NULL == ai|| NULL == bi){
            fprintf(stderr, "%s - ERR invalid argument bi[%p] ai[%p] snd[%p]\n", __func__, bi, ai, r_snd);
            return;
        }

        // vehicle attitude (relative to NED)
        // r/p/y (phi/theta/psi)
        // MB1 assumes vehicle frame, not world frame (i.e. exclude heading)
        double VW[3] = {ai->roll(), ai->pitch(), 0.};

        // sensor mounting angles (relative to vehicle, radians)
        // 3-2-1 euler angles, r/p/y  (phi/theta/psi)
        // wrt sensor mounted across track, b[0] port, downward facing
        double RSV[3] = { DTR(geo->svr_deg[0]), DTR(geo->svr_deg[1]), DTR(geo->svr_deg[2])};

        // sensor mounting translation offsets (relative to vehicle CRP, meters)
        // +x: fwd +y: stbd, +z:down (aka FSK, fwd,stbd,keel)
        // TODO T is for transform use rSV
        double TSV[3] = {geo->svt_m[0], geo->svt_m[1], geo->svt_m[2]};

        // beam components (dir cosine) matrix in vehicle frame (across, along, down)
        Matrix comp_BVF = dvl_vframe_components(bi, geo);

        // apply coordinate transforms; order is significant:
        // apply mounting rotation
        Matrix beams_VF = applyRotation(RSV, comp_BVF);
        // apply mounting translation
        Matrix beams_TF = applyTranslation(TSV, beams_VF);
        // apply vehicle attitude (hdg, pitch, roll)
        Matrix beams_WF = applyRotation(VW, beams_TF);

        // fill in the MB1 record using transformed beams

        std::list<trn::beam_tup> beams = bi->beams_raw();
        std::list<trn::beam_tup>::iterator it;

        // zero- and one-based indexs
        int idx[2] = {0, 1};
        for(it=beams.begin(); it!=beams.end(); it++, idx[0]++, idx[1]++)
        {
            trn::beam_tup bt = static_cast<trn::beam_tup> (*it);

            // beam number (0-indexed)
            int b = std::get<0>(bt);
            double range = std::get<1>(bt);
            // beam components WF x,y,z
            // matrix row/col (1 indexed)
            r_snd->beams[idx[0]].beam_num = b;
            r_snd->beams[idx[0]].rhox = range * beams_WF(1, idx[1]);
            r_snd->beams[idx[0]].rhoy = range * beams_WF(2, idx[1]);
            r_snd->beams[idx[0]].rhoz = range * beams_WF(3, idx[1]);

            if(trn_debug::get()->debug() >= 5){

                double rho[3] = {r_snd->beams[idx[0]].rhox, r_snd->beams[idx[0]].rhoy, r_snd->beams[idx[0]].rhoz};
                double rhoNorm = vnorm( rho );

                TRN_NDPRINT(5, "%s: b[%3d] r[%7.2f] R[%7.2lf] rhox[%7.2lf] rhoy[%7.2lf] rhoz[%7.2lf] \n",
                            __func__, b, range, rhoNorm,
                            r_snd->beams[idx[0]].rhox,
                            r_snd->beams[idx[0]].rhoy,
                            r_snd->beams[idx[0]].rhoz
                            );
            }
        }

        return;
    }

    // This is only for inputs mapped to mbtrnpp output
    static void transform_deltat(trn::bath_info *bi, trn::att_info *ai, mbgeo *geo, mb1_t *r_snd)
    {
        if(NULL == geo || geo->beam_count<=0){
            fprintf(stderr, "%s - geometry error : beams<=0\n", __func__);
            return;
        }
        if(NULL == r_snd || NULL == ai|| NULL == bi){
            fprintf(stderr, "%s - ERR invalid argument bi[%p] ai[%p] snd[%p]\n", __func__, bi, ai, r_snd);
            return;
        }

        // vehicle attitude (relative to NED, radians)
        // r/p/y  (phi/theta/psi)
        // MB1 assumes vehicle frame, not world frame (i.e. exclude heading)
        double VW[3] = {ai->roll(), ai->pitch(), 0.};

        // sensor mounting angles (relative to vehicle, radians)
        // 3-2-1 euler angles, r/p/y  (phi/theta/psi)
        // wrt sensor mounted across track, b[0] port, downward facing
        double RSV[3] = { DTR(geo->svr_deg[0]), DTR(geo->svr_deg[1]), DTR(geo->svr_deg[2])};

        // sensor mounting translation offsets (relative to vehicle CRP, meters)
        // +x: fwd +y: stbd, +z:down
        double TSV[3] = {geo->svt_m[0], geo->svt_m[1], geo->svt_m[2]};

        // beam components in reference sensor frame
        // (i.e., directional cosine unit vectors)
        Matrix comp_BVF = mb_vframe_components(bi, geo);

        std::list<trn::beam_tup> beams = bi->beams_raw();
        std::list<trn::beam_tup>::iterator it;
        TRN_NDPRINT(5, "%s: --- \n",__func__);

        TRN_NDPRINT(5, "VW[%.3lf, %.3lf, %.3lf]\n", VW[0], VW[1], VW[2]);
        TRN_NDPRINT(5, "RSV[%.3lf, %.3lf, %.3lf]\n", RSV[0], RSV[1], RSV[2]);
        TRN_NDPRINT(5, "TSV[%.3lf, %.3lf, %.3lf]\n", TSV[0], TSV[1], TSV[2]);

        TRN_NDPRINT(5, "VW roll[%.2lf] pitch[%.2lf%s] hdg[%.2lf (%.2lf)]\n",
                    Math::radToDeg(VW[0]), Math::radToDeg(VW[1]), (ai->flags().is_set(trn::AF_INVERT_PITCH) ? " i" : " "), Math::radToDeg(VW[2]), Math::radToDeg(ai->heading()));

        // apply coordinate transforms; order is significant:
        // apply mounting rotation
        Matrix beams_VF = applyRotation(RSV, comp_BVF);
        // apply mounting translation
        Matrix beams_TF = applyTranslation(TSV, beams_VF);
        // apply vehicle attitude (hdg, pitch, roll)
        Matrix beams_WF = applyRotation(VW, beams_TF);

        // write beam data to MB1 sounding
        int idx[2] = {0, 1};
        for(it=beams.begin(); it!=beams.end(); it++, idx[0]++, idx[1]++)
        {
            trn::beam_tup bt = static_cast<trn::beam_tup> (*it);
            // beam number (0-indexed)
            int b = std::get<0>(bt);
            double range = std::get<1>(bt);
            // beam components WF x,y,z
            // matrix row/col (1 indexed)

            r_snd->beams[idx[0]].beam_num = b;
            r_snd->beams[idx[0]].rhox = range * beams_WF(1, idx[1]);
            r_snd->beams[idx[0]].rhoy = range * beams_WF(2, idx[1]);
            r_snd->beams[idx[0]].rhoz = range * beams_WF(3, idx[1]);

            double rho[3] = {r_snd->beams[idx[0]].rhox, r_snd->beams[idx[0]].rhoy, r_snd->beams[idx[0]].rhoz};
            double rhoNorm = vnorm( rho );

            TRN_NDPRINT(5, "b[%3d] r[%7.2lf] R[%7.2lf] rhox[%7.2lf] rhoy[%7.2lf] rhoz[%7.2lf] ax[%6.2lf] ay[%6.2lf] az[%6.2lf]\n",
                        b, range, rhoNorm,
                        r_snd->beams[idx[0]].rhox,
                        r_snd->beams[idx[0]].rhoy,
                        r_snd->beams[idx[0]].rhoz,
                        (range==0. ? 0. : Math::radToDeg(acos(r_snd->beams[idx[0]].rhox/range))),
                        (range==0. ? 0. : Math::radToDeg(acos(r_snd->beams[idx[0]].rhoy/range))),
                        (range==0. ? 0. : Math::radToDeg(acos(r_snd->beams[idx[0]].rhoz/range)))
                        );
        }
        TRN_NDPRINT(5, "%s: --- \n",__func__);

        return;
    }

    // returns allocated mb1_t; caller must release using mb1_destroy()
    static mb1_t *lcm_to_mb1(trn::bath_info *bi, trn::nav_info *ni, trn::att_info *ai, uint32_t ping_number)
    {
        mb1_t *retval = nullptr;

        if(bi == nullptr || ni == nullptr || ai == nullptr){
            fprintf(stderr,"%s:%d ERR - invalid arg bi[%p] ni[%p] ai[%p]\n", __func__, __LINE__, bi, ni, ai);
            return retval;
        }

        size_t n_beams = bi->beam_count();

        if(n_beams <= 0){
            fprintf(stderr,"%s:%d WARN - beams <= 0 %lu\n", __func__, __LINE__, (long unsigned)n_beams);
        }

        mb1_t *snd = mb1_new(n_beams);
        snd->hdg = ai->heading();
        snd->depth = ni->depth();
        snd->lat = ni->lat();
        snd->lon = ni->lon();
        snd->type = MB1_TYPE_ID;
        snd->size = MB1_SOUNDING_BYTES(n_beams);
        snd->nbeams = n_beams;
        snd->ping_number = ping_number;
        snd->ts = ni->time_usec()/1e6;
        retval = snd;

        return retval;
    }

    // returns new poseT; caller must delete
    static poseT *lcm_to_poset(trn::bath_info *bi, trn::nav_info *ni, trn::att_info *ai, trn::vel_info *vi)
    {

        double lat = ni->lat();
        double lon = ni->lon();
        long int utm = NavUtils::geoToUtmZone(Math::degToRad(lat),
                                              Math::degToRad(lon));
        ai->flags().set(trn::AF_INVERT_PITCH);
        // ai->flags().set(trn::AF_INVERT_ROLL);
        double x = 0.;
        double y = 0.;
        double z = ni->depth();
        double psi = ai->heading();
        double theta = ai->pitch();
        double phi = ai->roll();
        // TRN requires vx != 0 to initialize
        // vy, vz not strictly required
        double vx = vi->vx_ms();
        double vy = 0.;
        double vz = 0.;
        double time = ni->time_usec()/1e6;
        bool dvlValid = bi->flags().is_set(trn::BF_VALID);
        bool gpsValid = (z < 2 ? true : false);// ni->flags().is_set(trn::NF_POS_VALID);
        bool bottomLock = bi->flags().is_set(trn::BF_BLOCK);

        // NavUtils::geoToUtm(latitude, longitude, utmZone, *northing, *easting)
        // Note that TRN uses N,E,D frame (i.e. N:x E:y D:z)
        NavUtils::geoToUtm(Math::degToRad(lat),
                           Math::degToRad(lon),
                           utm, &x, &y);

        TRN_NDPRINT(2, "%s:%d lat[%.6lf] lon[%.6lf] utm[%ld]\n", __func__, __LINE__, lat, lon, utm);
        TRN_NDPRINT(2, "%s:%d x[%.4lf] y[%.4lf] depth[%.1lf] r/p/y[%.2lf %.2lf, %.2lf]%c vx[%.2lf]\n", __func__, __LINE__, x, y, z, phi, theta, psi, (ai->flags().is_set(trn::AF_INVERT_PITCH)? '-' :'+'), vx);

        poseT *pt = new poseT();

        pt->x = x;
        pt->y = y;
        pt->z = z;
        pt->phi = phi;
        pt->theta = theta;
        pt->psi = psi;
        pt->time = time;
        pt->dvlValid=dvlValid;
        pt->gpsValid=gpsValid;
        pt->bottomLock=bottomLock;
        // w* required? (values from trnw/replay/dorado)?
//        pt->wx = -3.332e-002;
//        pt->wy = -9.155e-003;
//        pt->wz = -3.076e-002;
        pt->wx = 0.;
        pt->wy = 0.;
        pt->wz = 0.;
        // doesn't work if v* not set - why?
        // OK for v* all the same (e.g.0.01 as in mbtrnpp)?
        pt->vx = vx;
        pt->vy = vy;
        pt->vz = vz;

        return pt;
    }

    // returns new measT; caller must delete
    static measT *lcm_to_meast(trn::bath_info *bi, trn::nav_info *ni, trn::att_info *ai, dvlgeo *geo, uint32_t ping_number)
    {
        double lat = ni->lat();
        double lon = ni->lon();
        long int utm = NavUtils::geoToUtmZone(Math::degToRad(lat),
                                              Math::degToRad(lon));
        double x = 0.;
        double y = 0.;
        double z = ni->depth();
        double psi = ai->heading();
        double theta = ai->pitch();
        double phi = ai->roll();

        double time = ni->time_usec()/1e6;
        size_t n_beams = bi->beam_count();

        // NavUtils::geoToUtm(latitude, longitude, utmZone, *northing, *easting)
        // Note that TRN uses N,E,D frame (i.e. N:x E:y D:z)
        NavUtils::geoToUtm(Math::degToRad(lat),
                           Math::degToRad(lon),
                           utm, &x, &y);

        measT *mt = new measT(n_beams,TRN_SENSOR_DVL);

        mt->x = x;
        mt->y = y;
        mt->z = z;
        mt->phi = phi;
        mt->theta = theta;
        mt->psi = psi;
        mt->time = time;
        mt->ping_number = ping_number;

        // vehicle attitude (relative to NED)
        // r/p/y (phi/theta/psi)
        // MB1 assumes vehicle frame, not world frame (i.e. exclude heading)
        double VW[3] = {ai->roll(), ai->pitch(), 0.};

        // sensor mounting angles (relative to vehicle, radians)
        // 3-2-1 euler angles, r/p/y  (phi/theta/psi)
        // wrt sensor mounted across track, b[0] port, downward facing
        double RSV[3] = { DTR(geo->svr_deg[0]), DTR(geo->svr_deg[1]), DTR(geo->svr_deg[2])};

        // sensor mounting translation offsets (relative to vehicle CRP, meters)
        // +x: fwd +y: stbd, +z:down (aka FSK, fwd,stbd,keel)
        // TODO T is for transform use rSV
        double TSV[3] = {geo->svt_m[0], geo->svt_m[1], geo->svt_m[2]};

        // beam components (dir cosine) matrix in vehicle frame (across, along, down)
        Matrix comp_BVF = dvl_vframe_components(bi, geo);

        TRN_NDPRINT(5, "%s:%d VW[%.3lf, %.3lf, %.3lf]\n", __func__, __LINE__, VW[0], VW[1], VW[2]);
        TRN_NDPRINT(5, "%s:%d RSV[%.3lf, %.3lf, %.3lf]\n", __func__, __LINE__, RSV[0], RSV[1], RSV[2]);
        TRN_NDPRINT(5, "%s:%d TSV[%.3lf, %.3lf, %.3lf]\n", __func__, __LINE__, TSV[0], TSV[1], TSV[2]);

        // apply coordinate transforms; order is significant:
        // apply mounting rotation
        Matrix beams_VF = applyRotation(RSV, comp_BVF);
        // apply mounting translation
        Matrix beams_TF = applyTranslation(TSV, beams_VF);
        // apply vehicle attitude (hdg, pitch, roll)
        Matrix beams_WF = applyRotation(VW, beams_TF);

        std::list<trn::beam_tup> beams = bi->beams_raw();
        std::list<trn::beam_tup>::iterator it;

        // zero- and one-based indexs
        int idx[2] = {0, 1};
        for(it=beams.begin(); it!=beams.end(); it++, idx[0]++, idx[1]++)
        {
            trn::beam_tup bt = static_cast<trn::beam_tup> (*it);

            // beam number (0-indexed)
            int b = std::get<0>(bt);
            double range = std::get<1>(bt);

            mt->ranges[idx[0]] = range;
            mt->measStatus[idx[0]] = (range > 1 ? true : false);

            mt->beamNums[idx[0]] = b;
            mt->alongTrack[idx[0]] = range * beams_WF(1, idx[1]);
            mt->crossTrack[idx[0]] = range * beams_WF(2, idx[1]);
            mt->altitudes[idx[0]]  = range * beams_WF(3, idx[1]);

            if(trn_debug::get()->debug() >= 5){

                double rho[3] = {mt->alongTrack[idx[0]], mt->crossTrack[idx[0]], mt->altitudes[idx[0]]};
                double rhoNorm = vnorm( rho );

                TRN_NDPRINT(5, "%s: b[%3d] r[%7.2f] R[%7.2lf] rhox[%7.2lf] rhoy[%7.2lf] rhoz[%7.2lf] \n",
                            __func__, mt->beamNums[idx[0]],
                            mt->ranges[idx[0]], rhoNorm,
                            mt->alongTrack[idx[0]],
                            mt->crossTrack[idx[0]],
                            mt->altitudes[idx[0]]
                            );
            }
        }

        return mt;
    }

    static std::string lcm_to_csv(trn::bath_info *bi, trn::att_info *ai, trn::nav_info *ni, trn::vel_info *vi=nullptr)
    {
        // time, lat, lon, roll, pitch, hdg,valid,1,1,...nbeams,beamnum, range,...\n
        std::ostringstream ss;
        if(nullptr != bi && nullptr != ai && nullptr != ni)
        {
            double lat = ni->lat();
            double lon = ni->lon();
            double pos_N=0;
            double pos_E=0;
            long int utm = NavUtils::geoToUtmZone(Math::degToRad(lat),
                                                  Math::degToRad(lon));

            // NavUtils::geoToUtm(latitude, longitude, utmZone, *northing, *easting)
            NavUtils::geoToUtm(Math::degToRad(lat), Math::degToRad(lon), utm, &pos_N, &pos_E);
            // Note that TRN uses N,E,D frame (i.e. N:x E:y D:z)
            // time POSIX epoch sec
            // northings
            // eastings
            // depth
            // heading
            // pitch
            // roll
            // flag (0)
            // flag (0)
            // flag (0)
            // vx (0)
            // xy (0)
            // vz (0)
            // sounding valid flag
            // bottom lock valid flag
            // number of beams
            // beam[i] number
            // beam[i] valid (1)
            // beam[i] range
            // ...
            // NEWLINE

            ss << std::dec << std::setfill(' ') << std::fixed << std::setprecision(7);
            ss << bi->time_usec()/1000000. << ",";
            ss << std::setprecision(7);
            ss << pos_N << ",";
            ss << pos_E << ",";
            ss << ni->depth() << ",";
            ss << ai->heading() << ",";
            ss << ai->pitch() << ",";
            ss << ai->roll() << ",";
            ss << 0 << ",";
            ss << 0 << ",";
            ss << 0 << ",";
            if(vi != nullptr){
                ss << vi->vx_ms() << ",";
                ss << vi->vy_ms() << ",";
                ss << vi->vz_ms() << ",";
            }else{
                ss << 0. << ",";
                ss << 0. << ",";
                ss << 0. << ",";
            }
            ss << std::setprecision(1);
            ss << (bi->flags().is_set(trn::BF_VALID)?1:0) << ",";
            ss << (bi->flags().is_set(trn::BF_BLOCK)?1:0) << ",";
            ss << bi->beam_count() << ",";
            ss << std::setprecision(4);
            std::list<trn::beam_tup>::iterator it;
            std::list<trn::beam_tup>beam_list = bi->beams_raw();
            int k=0;
            for(it=beam_list.begin(); it!=beam_list.end(); k++)
            {
                trn::beam_tup bt = static_cast<trn::beam_tup> (*it);
                ss <<  std::get<0>(bt) << ",";
                ss << 1 << ",";
                ss << std::get<1>(bt);
                it++;
                if(it != beam_list.end())
                    ss << ",";
            }
        }
        return ss.str();
    }

    static void pose_tostream(std::ostream &os, const poseT &src, int wkey=15, int wval=18)
    {
        os << std::dec << std::setprecision(3) << std::setfill(' ');

        os << std::setw(wkey) << "time" << std::setw(wval) << src.time <<"\n";
        os << std::setw(wkey) << "x" << std::setw(wval) << src.x <<"\n";
        os << std::setw(wkey) << "y" << std::setw(wval) << src.y <<"\n";
        os << std::setw(wkey) << "z" << std::setw(wval) << src.z <<"\n";
        os << std::setw(wkey) << "phi" << std::setw(wval) << src.phi <<"\n";
        os << std::setw(wkey) << "theta" << std::setw(wval) << src.theta <<"\n";
        os << std::setw(wkey) << "psi" << std::setw(wval) << src.psi <<"\n";
        os << std::setw(wkey) << "gpsValid" << std::setw(wval) << src.gpsValid <<"\n";
        os << std::setw(wkey) << "bottomLock" << std::setw(wval) << src.bottomLock <<"\n";
        os << std::setw(wkey) << "dvlValid" << std::setw(wval) << src.dvlValid <<"\n";
        os << std::setw(wkey) << "vx" << std::setw(wval) << src.vx <<"\n";
        os << std::setw(wkey) << "vy" << std::setw(wval) << src.vy <<"\n";
        os << std::setw(wkey) << "vz" << std::setw(wval) << src.vz <<"\n";
        os << std::setw(wkey) << "wx" << std::setw(wval) << src.wx <<"\n";
        os << std::setw(wkey) << "wy" << std::setw(wval) << src.wy <<"\n";
        os << std::setw(wkey) << "wz" << std::setw(wval) << src.wz <<"\n";

    }
    static std::string pose_tostring(const poseT &src, int wkey=15, int wval=18)
    {
        std::ostringstream ss;
        pose_tostream(ss, src, wkey, wval);
        return ss.str();
    }

    static void pose_show(const poseT &src, int wkey=15, int wval=18)
    {
        pose_tostream(std::cerr, src, wkey, wval);
    }

    static void meas_tostream(std::ostream &os, const measT &src, int wkey=15, int wval=18)
    {
        os << std::dec << std::setprecision(3) << std::setfill(' ');

        os << std::setw(wkey) << "time" << std::setw(wval) << src.time <<"\n";
        os << std::setw(wkey) << "ping_number" << std::setw(wval) << src.ping_number <<"\n";
        os << std::setw(wkey) << "dataType" << std::setw(wval) << src.dataType <<"\n";
        os << std::setw(wkey) << "x" << std::setw(wval) << src.x <<"\n";
        os << std::setw(wkey) << "y" << std::setw(wval) << src.y <<"\n";
        os << std::setw(wkey) << "z" << std::setw(wval) << src.z <<"\n";
        os << std::setw(wkey) << "phi" << std::setw(wval) << src.phi <<"\n";
        os << std::setw(wkey) << "theta" << std::setw(wval) << src.theta <<"\n";
        os << std::setw(wkey) << "psi" << std::setw(wval) << src.psi <<"\n";
        os << std::setw(wkey) << "numMeas" << std::setw(wval) << src.numMeas <<"\n";

        for(int i=0; i < src.numMeas; i++){
            os << std::setw(wkey) << "b[" << src.beamNums[i] << "]";
            ostringstream ss;
            ss << "r[";
            ss << src.ranges[i];
            ss << "] rx,ry,rz,v";
            ss << "[";
            ss << src.alongTrack[i] << ", ";
            ss << src.crossTrack[i] << ", ";
            ss << src.altitudes[i] << ", ";
            ss << src.measStatus[i] << "]";
            string bs = ss.str();
            int alen = ss.str().length();
            int wx = (alen >= wval ? alen + 1 : wval);
            os << std::setw(wx) << bs.c_str() <<"\n";

        }
    }

    static std::string meas_tostring(const measT &src, int wkey=15, int wval=18)
    {
        std::ostringstream ss;
        meas_tostream(ss, src, wkey, wval);
        return ss.str();
    }

    static void meas_show(const measT &src, int wkey=15, int wval=18)
    {
        meas_tostream(std::cerr, src, wkey, wval);
    }

    // returns new poseT; caller must release
    static poseT *mb1_to_pose(mb1_t *src, trn::att_info *ai, long int utmZone)
    {
        if(nullptr == src)
            return nullptr;

        poseT *obj = new poseT();

        if(nullptr != obj){

           obj->time = src->ts;

           NavUtils::geoToUtm( Math::degToRad(src->lat),
                              Math::degToRad(src->lon),
                              utmZone, &(obj->x), &(obj->y));

           obj->z = src->depth;
           obj->psi = src->hdg;
           obj->theta = ai->pitch();
           obj->phi = ai->roll();
           obj->gpsValid = (obj->z < 2 ? true : false);
           obj->bottomLock = true;
           obj->dvlValid = true;
           // TRN can't intialize if vx == 0
           obj->vx = 0.01;
           obj->vy = 0.;
           obj->vz = 0.;
           // wx not required; can use these (how determined?)
           // obj->wx = -3.332e-002;
           // obj->wy = -9.155e-003;
           // obj->wz = -3.076e-002;
           obj->wx = 0.;
           obj->wy = 0.;
           obj->wz = 0.;

        }
        return obj;
    }

    // returns new measT; caller must release
    static measT *mb1_to_meas(mb1_t *src, trn::att_info *ai, int data_type, long int utmZone)
    {
        if(nullptr == src)
            return nullptr;

        measT *obj = new measT(src->nbeams, data_type);

        if(nullptr != obj){
            
            obj->time = src->ts;
            obj->ping_number = src->ping_number;
            obj->dataType = data_type;
            obj->psi = src->hdg;
            obj->theta = ai->pitch();
            obj->phi = ai->roll();
            obj->z = src->depth;

            NavUtils::geoToUtm( Math::degToRad(src->lat),
                               Math::degToRad(src->lon),
                               utmZone, &(obj->x), &(obj->y));

            for(int i = 0; i < obj->numMeas; i++){
                // TODO: fill in measT from ping...
                obj->beamNums[i] = src->beams[i].beam_num;
                obj->alongTrack[i] = src->beams[i].rhox;
                obj->crossTrack[i] = src->beams[i].rhoy;
                obj->altitudes[i]  = src->beams[i].rhoz;

                double rho[3] = {obj->alongTrack[i], obj->crossTrack[i], obj->altitudes[i]};
                double rhoNorm = vnorm( rho );
                obj->ranges[i] = rhoNorm;
                // [rhoNorm = sqrt(ax^2 + ay^2 + az^2)] (i.e. range magnitude)
                obj->measStatus[i] = rhoNorm > 1 ? true : false;
                //                obj->covariance[i] = 0.0;
                //                obj->alphas[i]     = 0.0;
            }

        }
        return obj;
    }

    static double vnorm( double v[] )
    {
        double vnorm2 = 0.0;
        int i=0;
        for(i=0; i<3; i++) vnorm2 += pow(v[i],2.0);
        return( sqrt( vnorm2 ) );
    }

protected:
private:
};

#endif
