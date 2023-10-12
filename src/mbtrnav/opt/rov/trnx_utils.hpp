

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

#include "structDefs.h"
#include "MathP.h"
#include "newmat.h"
#include "trn_msg.h"
#include "mb1_msg.h"
#include "udpm_sub.h"
#include "trn_debug.hpp"
#include "geo_cfg.hpp"
#include "NavUtils.h"

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
        os << std::setw(5) << "MLE" << std::setw(21) << " [t, tm, x, y, z] ";
        os << std::fixed << std::setprecision(3);
        os << std::setw(13) << ts << ", ";
        os << std::setw(13) << mle.time << ", ";
        os << std::setw(7) << mle.x << ", ";
        os << std::setw(7) << mle.y << ", ";
        os << std::setw(7) << mle.z << "\n";

        os << std::setw(5) << "MMSE" << std::setw(21) << " [t, tm, x, y, z] ";
        os << std::fixed << std::setprecision(3);
        os << std::setw(13) << ts << ", ";
        os << std::setw(13) << mmse.time << ", ";
        os << std::setw(7) << mmse.x << ", ";
        os << std::setw(7) << mmse.y << ", ";
        os << std::setw(7) << mmse.z << "\n";

        os << std::setw(5) << "POS" << std::setw(21) << " [t, tm, x, y, z] ";
        os << std::fixed << std::setprecision(3);
        os << std::setw(13) << ts << ", ";
        os << std::setw(13) << mmse.time << ", ";
        os << std::setw(7) << pt.x << ", ";
        os << std::setw(7) << pt.y << ", ";
        os << std::setw(7) << pt.z << "\n";

        os << std::setw(5) << "OFS" << std::setw(21) << " [t, tm, x, y, z] ";
        os << std::fixed << std::setprecision(3);
        os << std::setw(13) << ts << ", ";
        os << std::setw(13) << mmse.time << ", ";
        os << std::setw(7) << mmse.x - pt.x << ", ";
        os << std::setw(7) << mmse.y - pt.y << ", ";
        os << std::setw(7) << mmse.z - pt.z << "\n";

        os << std::setw(5) << "COV" << std::setw(21) << " [t, x, y, z, m] ";
        os << std::setprecision(3);
        os << std::setw(13) << mmse.time << ", ";
        os << std::setw(7) << mmse.covariance[0] << ", ";
        os << std::setw(7) << mmse.covariance[2] << ", ";
        os << std::setw(7) << mmse.covariance[5] << ", ";
        double ss = mmse.covariance[0] * mmse.covariance[0];
        ss += mmse.covariance[2] * mmse.covariance[2];
        ss += mmse.covariance[5] * mmse.covariance[5];
        os << std::setw(7) << sqrt(ss) << "\n";

        os << std::setw(5) << "s" << std::setw(21) << " [t, x, y, z] ";
        os << std::setprecision(3);
        os << mmse.time << ", ";
        os << std::setprecision(2);
        os << sqrt(mmse.covariance[0]) << ", ";
        os << sqrt(mmse.covariance[2]) << ", ";
        os << sqrt(mmse.covariance[5]) << "\n";

    }

    static std::string trnest_tocsv(double &stime, poseT &pt, poseT &mle, poseT &mmse)
    {
        // Format (compatible with tlp-plot)
        // session-time,
        // mmse.time,
        // mmse.x,
        // mmse.y,
        // mmse.z,
        // pos.time,
        // ofs.x,
        // ofs.y,
        // ofs.z,
        // cov.0,
        // cov.2,
        // cov.5,
        // pos.time,
        // pos.x,
        // pos.y,
        // pos.z,
        // mle.time,
        // mle.x,
        // mle.y,
        // mle.z

        ostringstream os;
        os << std::fixed << std::setprecision(3);
        os << stime << ",";

        // mmse
        os << mmse.time << ",";
        os << std::setprecision(4);
        os << mmse.x << "," << mmse.y << "," << mmse.z << ",";

        // ofs
        os << std::fixed << std::setprecision(3);
        os << pt.time << ",";
        os << std::setprecision(4);
        os << mmse.x - pt.x << "," << mmse.y - pt.y << "," << mmse.z - pt.z << ",";

        // cov
        os << std::fixed << std::setprecision(3);
        os << (mmse.covariance[0]) << ",";
        os << (mmse.covariance[2]) << ",";
        os << (mmse.covariance[5]) << ",";

        // pos
        os << std::setprecision(3);
        os << pt.time << ",";
        os << std::setprecision(4);
        os << pt.x << "," << pt.y << "," << pt.z << ",";

        // mle
        os << std::fixed << std::setprecision(3);
        os << mle.time << ",";
        os << std::setprecision(4);
        os << mle.x << "," << mle.y << "," << mle.z << "\n";

        return os.str();
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

    static std::string mbest_tocsv(trnu_pub_t &mbest)
    {
        // Format (compatible with tlp-plot)
        // session-time,
        // mmse.time,
        // mmse.x,
        // mmse.y,
        // mmse.z,
        // pos.time,
        // ofs.x,
        // ofs.y,
        // ofs.z,
        // cov.0,
        // cov.2,
        // cov.5,
        // pos.time,
        // pos.x,
        // pos.y,
        // pos.z,
        // mle.time,
        // mle.x,
        // mle.y,
        // mle.z

        if(mbest.mb1_time == 0.){
            return std::string("");
        }

        ostringstream os;
        os << std::fixed << std::setprecision(3);
        os << mbest.mb1_time << ",";

        // mmse
        os << mbest.est[2].time << ",";
        os << std::setprecision(4);
        os << mbest.est[2].x << "," << mbest.est[2].y << "," << mbest.est[2].z << ",";

        // ofs
        os << std::fixed << std::setprecision(3);
        os << mbest.est[3].time << ",";
        os << std::setprecision(4);
        os << mbest.est[3].x << "," << mbest.est[3].y << "," << mbest.est[3].z << ",";

        // cov (mmse)
        os << std::fixed << std::setprecision(3);
        os << (mbest.est[2].cov[0]) << ",";
        os << (mbest.est[2].cov[1]) << ",";
        os << (mbest.est[2].cov[2]) << ",";

        // pos
        os << std::setprecision(3);
        os << mbest.est[0].time << ",";
        os << std::setprecision(4);
        os << mbest.est[0].x << "," << mbest.est[0].y << "," << mbest.est[0].z << ",";

        // mle (not in trnu_pub_t)
        os << std::fixed << std::setprecision(3);
        os << mbest.est[1].time << ",";
        os << std::setprecision(4);
        os << mbest.est[1].x << "," << mbest.est[1].y << "," << mbest.est[1].z << ",";

        return os.str();
    }

    static void mbest_tostream(std::ostream &os, trnu_pub_t *mbest, int wkey=15, int wval=18)
    {
        os << "--- MB Update OK---" << "\n";

        os << std::fixed << std::setprecision(3);
        os << std::setw(5) << "MLE" << std::setw(21) << " [t, x, y, z] ";
        os << std::setprecision(3);
        os << mbest->est[1].time << ", ";
        os << std::setw(7) << mbest->est[1].x << ", ";
        os << std::setw(7) << mbest->est[1].y << ", ";
        os << std::setw(7) << mbest->est[1].z << "\n";

        os << std::setw(5) << "MMSE" << std::setw(21) << " [t, x, y, z] ";
        os << std::setprecision(3);
        os << std::setw(13) << mbest->est[2].time << ", ";
        os << std::setw(7) << mbest->est[2].x << ", ";
        os << std::setw(7) << mbest->est[2].y << ", ";
        os << std::setw(7) << mbest->est[0].z << "\n";

        os << std::setw(5) << "POSE" << std::setw(21) << " [t, x, y, z] ";
        os << std::setprecision(3);
        os << std::setw(13) << mbest->est[0].time << ", ";
        os << std::setw(7) << mbest->est[0].x << ", ";
        os << std::setw(7) << mbest->est[0].y << ", ";
        os << std::setw(7) << mbest->est[0].z << "\n";

        os << std::setw(5) << "OFS" << std::setw(21) << " [t, x, y, z] ";
        os << std::setprecision(3);
        os << std::setw(13) << mbest->est[2].time << ", ";
        os << std::setw(7) << mbest->est[2].x - mbest->est[0].x << ", ";
        os << std::setw(7) << mbest->est[2].y - mbest->est[0].y << ", ";
        os << std::setw(7) << mbest->est[2].z - mbest->est[0].z << "\n";

        os << std::setw(5) << "COV" << std::setw(21) << " [t, x, y, z, xy, m] ";
        os << std::setprecision(3);
        os << std::setw(13) << mbest->est[2].time << ", ";
        os << std::setw(7) << mbest->est[2].cov[0] << ", ";
        os << std::setw(7) << mbest->est[2].cov[1] << ", ";
        os << std::setw(7) << mbest->est[2].cov[2] << ", ";
        os << std::setw(7) << mbest->est[2].cov[3] << ", ";
        double ss = mbest->est[2].cov[0] * mbest->est[2].cov[0];
        ss += mbest->est[2].cov[1] * mbest->est[2].cov[1];
        ss += mbest->est[2].cov[2] * mbest->est[2].cov[2];
        os << sqrt(ss) << "\n";

        os << std::setw(5) << "s" << std::setw(21) << " [t, x, y, z] ";
        os << std::setprecision(3);
        os << std::setw(13) << mbest->est[2].time << ", ";
        os << std::setw(7) << sqrt(mbest->est[2].cov[0]) << ", ";
        os << std::setw(7) << sqrt(mbest->est[2].cov[1]) << ", ";
        os << std::setw(7) << sqrt(mbest->est[2].cov[2]) << "\n";

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

    static void matrix_tostream(std::ostream &os, Matrix m, const char *name=nullptr, int width=7, int precision=3, int wkey=5)
    {

        std::ios_base::fmtflags orig_settings = std::cout.flags();

        if(name != nullptr)
            os << std::setw(wkey) << name << " [" << m.Nrows() << "r " << m.Ncols() << "c]" << std::endl;

        os << std::fixed << std::setprecision(precision);

        for(int i = 1 ; i <= m.Nrows() ; i++ ){
            os << std::setw(wkey) << " [" << i << "] :";
            for(int j = 1 ; j <= m.Ncols() ; j++ ){
                os << " " << std::setw(width) << m(i, j);
            }
            os << std::endl;
        }

        std::cout.flags(orig_settings);
    }

    static std::string matrix_tostring(Matrix m, const char *name=nullptr, int width=7, int precision=3, int wkey=5)
    {
        ostringstream ss;
        matrix_tostream(ss, m, name, precision, wkey);
        return ss.str();
    }

    static void matrix_show(Matrix m, const char *name=nullptr, int width=7, int precision=3, int wkey=5)
    {
        matrix_tostream(std::cerr, m, name, width, precision, wkey);
    }

    static Matrix affineMultiply(const Matrix& A, const Matrix& B)
    {
        Matrix outMat = B;

        for(int r = 1; r <= A.Nrows(); r++) {
            outMat(r, 1) = A(r,1) * B(1, 1) + A(r,2) * B(2, 1) + A(r,3) * B(3, 1) + A(r,4) * B(4, 1);
            outMat(r, 2) = A(r,1) * B(1, 2) + A(r,2) * B(2, 2) + A(r,3) * B(3, 2) + A(r,4) * B(4, 2);
            outMat(r, 3) = A(r,1) * B(1, 3) + A(r,2) * B(2, 3) + A(r,3) * B(3, 3) + A(r,4) * B(4, 3);
            outMat(r, 4) = A(r,1) * B(1, 4) + A(r,2) * B(2, 4) + A(r,3) * B(3, 4) + A(r,4) * B(4, 4);
        }
        return outMat;
    }

    // 321 euler rotation R(phi, theta, psi)
    // where
    // phi: roll (rotation about X)
    // theta: pitch (rotation about Y)
    // psi: yaw (rotation about Z)
    // expects
    // rot_rad[0]: phi / roll (rad)
    // rot_rad[1]: theta / pitch (rad)
    // rot_rad[2]: psi / yaw (rad)
    static Matrix affine321Rotation(double *rot_rad)
    {
        Matrix mat(4,4);

        double cphi = cos(rot_rad[0]);
        double sphi = sin(rot_rad[0]);
        double ctheta = cos(rot_rad[1]);
        double stheta = sin(rot_rad[1]);
        double cpsi = cos(rot_rad[2]);
        double spsi = sin(rot_rad[2]);
        double stheta_sphi = stheta * sphi;
        double stheta_cphi = stheta * cphi;

        mat(1, 1) = cpsi * ctheta;
        mat(1, 2) = spsi * ctheta;
        mat(1, 3) = -stheta;
        mat(1, 4) = 0.;
        mat(2, 1) = -spsi * cphi + cpsi * stheta_sphi;
        mat(2, 2) = cpsi * cphi + spsi * stheta_sphi;
        mat(2, 3) = ctheta * sphi;
        mat(2, 4) = 0.;
        mat(3, 1) = spsi * sphi + cpsi * stheta_cphi;
        mat(3, 2) = -cpsi * sphi + spsi * stheta_cphi;
        mat(3, 3) = ctheta * cphi;
        mat(3, 4) = 0.;
        mat(4, 1) = 0.;
        mat(4, 2) = 0.;
        mat(4, 3) = 0.;
        mat(4, 4) = 1.;

        return mat;
    }

    static Matrix affineTranslation(double *tran_m)
    {
        Matrix mat(4,4);

        mat(1, 1) = 1.;
        mat(1, 2) = 0.;
        mat(1, 3) = 0.;
        mat(1, 4) = tran_m[0];
        mat(2, 1) = 0.;
        mat(2, 2) = 1.;
        mat(2, 3) = 0.;
        mat(2, 4) = tran_m[1];
        mat(3, 1) = 0.;
        mat(3, 2) = 0.;
        mat(3, 3) = 1.;
        mat(3, 4) = tran_m[2];
        mat(4, 1) = 0.;
        mat(4, 2) = 0.;
        mat(4, 3) = 0.;
        mat(4, 4) = 1.;

        return mat;
    }

    static int test_affine()
    {
        // validate matrix geometry operations
        // by rotating/translating a square
        fprintf(stderr,"%s - entry\n", __func__);

        // define vertices of 1x1 square centered on origin
        Matrix square(4,4);
        double lower_left[3] = {-0.5, -0.5, 0.};
        square(1,1) = lower_left[0] + 0.;
        square(2,1) = lower_left[1] + 0.;
        square(3,1) = lower_left[2] + 0.;
        square(4,1) = 1.;
        square(1,2) = lower_left[0] + 1.;
        square(2,2) = lower_left[1] + 0.;
        square(3,2) = lower_left[2] + 0.;
        square(4,2) = 1.;
        square(1,3) = lower_left[0] + 1.;
        square(2,3) = lower_left[1] + 1.;
        square(3,3) = lower_left[2] + 0.;
        square(4,3) = 1.;
        square(1,4) = lower_left[0] + 0.;
        square(2,4) = lower_left[1] + 1.;
        square(3,4) = lower_left[2] + 0.;
        square(4,4) = 1.;

        // 90 deg CW rotation about Z (psi)
        double rot[3] = {Math::degToRad(0.),Math::degToRad(0.),Math::degToRad(90.)};

        // translation sequence
        // X  1.5 Y  1.5
        // X -1.0 Y  0.0
        // X  0.0 Y -1.0
        // X  1.0 Y  0.0
        // X  0.0 Y  1.0
        // X -1.5 Y -1.5
        double trn[6][3] = {\
            { 1.5,  1.5, 0.0},
            {-1.0,  0.0, 0.0},
            { 0.0, -1.0, 0.0},
            { 1.0,  0.0, 0.0},
            { 0.0,  1.0, 0.0},
            {-1.5, -1.5, 0.0}
        };

        fprintf(stderr,"before TX\n");
        matrix_show(square,"square");

        // affine operation matrix
        Matrix OP(4,4);
        // current square vertices
        Matrix X = square;

        // run translation sequence
        // should return to original position
        for(int i=0; i<6; i++){
            OP = affineTranslation(&trn[i][0]);
            X = OP * X;
            fprintf(stderr,"after TX[%d]\n",i);
            matrix_show(square, "X");
        }
        fprintf(stderr,"\n");

        int errors = 0;
        double eps = 1.0e-6;

        // check vertices (should be same as start position)
        for(int r = 1; r<= 4; r++){
            for(int c = 1; c<= 4; c++){
                if(fabs(square(r,c) - X(r, c)) > eps){
                    errors++;
                    fprintf(stderr,"%s - tranlation err for element (%d, %d) delta[%.3g]\n", __func__, r, c, (square(r,c)-X(r,c)));
                }
            }
        }

        // position square w/ lower left at (1,1)
        OP = affineTranslation(&trn[0][0]);
        X = OP * square;
        Matrix Xo = X;

        fprintf(stderr,"before ROT\n");
        matrix_show(square, "X");

        // run rotation sequence 4 x 90 deg CW
        // should match start location
        OP = affine321Rotation(rot);
        for(int i=0;i<4;i++){
            X = OP * X;
            fprintf(stderr,"after ROT[%d]\n",i);
            matrix_show(square, "X");
        }
        fprintf(stderr,"\n");

        for(int r = 1; r<= 4; r++){
            for(int c = 1; c<= 4; c++){
                if(fabs(Xo(r,c) - X(r, c)) > eps){
                    errors++;
                    fprintf(stderr,"%s - tranlation err for element (%d, %d) delta[%.3g]\n", __func__, r, c, (Xo(r,c)-X(r,c)));
                }
            }
        }

        fprintf(stderr,"%s - returning %d/%s\n", __func__, errors, (errors > 0 ? "ERR" : "OK"));

        return errors;
    }

    // in : beams in MB1 format (along, across, down)
    // out : directional cosine matrix (along, across, down)
    static Matrix mb_sframe_components(trn::mb1_info *bi, mbgeo *geo)
    {
        if(bi == nullptr || geo == NULL){
            Matrix err_ret = Matrix(4,1);
        }

        // number of beams read (<= nominal beams)
        int nbeams = bi->beam_count();

        Matrix sf_comp = Matrix(4,nbeams);


        // beam components in reference sensor frame (mounted center, across track)

        std::list<trn::mb1_beam_tup> beams = bi->beams_raw();
        std::list<trn::mb1_beam_tup>::iterator it;

        // zero- and one-based indexs
        int idx[2] = {0, 1};

        TRN_NDPRINT(TRNDL_UTILS_MBSFCOMP, "%s: --- \n",__func__);

        for(it = beams.begin(); it != beams.end(); it++)
        {
            trn::mb1_beam_tup bt = static_cast<trn::mb1_beam_tup> (*it);

            // beam components already computed, just copy

            // beam number (0-indexed)
            int b = std::get<0>(bt);
            double x = std::get<1>(bt);
            double y = std::get<2>(bt);
            double z = std::get<3>(bt);
            double range = sqrt((x * x) +(y * y) + (z * z));
            // beam components (reference orientation, sensor frame)
            // 1: along (x) 2: across (y) 3: down (z)
            sf_comp(1, idx[1]) = x/range;
            sf_comp(2, idx[1]) = y/range;
            sf_comp(3, idx[1]) = z/range;
            sf_comp(4, idx[1]) = 0.;

            if(trn_debug::get()->debug() >= TRNDL_UTILS_MBSFCOMP){

                double rho[3] = {sf_comp(1,idx[1]), sf_comp(2,idx[1]), sf_comp(3,idx[1])};

                double rhoNorm = vnorm(rho);

                const char *sep = (b == 60 ? "****" : "    ");
                TRN_NDPRINT(TRNDL_UTILS_MBSFCOMP_H, "%s - b[%3d] r[%7.2lf] R[%7.2lf] %s Rx[%7.2lf] Ry[%7.2lf] Rz[%7.2lf]\n",
                            __func__, b, range, rhoNorm,
                            sep, sf_comp(1,idx[1]), sf_comp(2,idx[1]), sf_comp(3,idx[1]));
            }

            idx[0]++;
            idx[1]++;
        }
        TRN_NDPRINT(TRNDL_UTILS_MBSFCOMP, "%s: --- \n\n",__func__);

        return sf_comp;
    }

    // in : beam ranges, sensor geometry
    // out : directional cosine matrix (along, across, down)
    static Matrix mb_sframe_components(trn::bath_info *bi, mbgeo *geo)
    {
        if(bi == nullptr || geo == NULL){
            Matrix err_ret = Matrix(4,1);
        }

        // number of beams read (<= nominal beams)
        int nbeams = bi->beam_count();

        Matrix sf_comp = Matrix(4,nbeams);

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

        TRN_NDPRINT(TRNDL_UTILS_MBSFCOMP, "%s: --- \n",__func__);
        TRN_NDPRINT(TRNDL_UTILS_MBSFCOMP, "sensor frame:\nalong: x (fwd +)\nacross: y (stb+)\ndown: z (down+)\n");
        TRN_NDPRINT(TRNDL_UTILS_MBSFCOMP, "S[%.3lf] K[%.3lf] e[%.3lf]\n", S, K, e);

        for(it=beams.begin(); it!=beams.end(); it++)
        {
            trn::beam_tup bt = static_cast<trn::beam_tup> (*it);
            double range = std::get<1>(bt);
            // beam number (0-indexed)
            int b = std::get<0>(bt);

            // beam yaw, pitch angles (sensor frame)
            double yd = 0;
            double xd = (K + (b * e));
            double pd = xd;

            if(xd > 90.) {
                // normalize pitch to +/- 90 deg
                yd = 180.;
                pd = 180. - xd;
            }

            // psi (yaw), phi (pitch) to radians
            double yr = DTR(yd);
            double pr = DTR(pd);

            // beam components (reference orientation, sensor frame)
            // 1: along (x) 2: across (y) 3: down (z)
            sf_comp(1, idx[1]) = cos(pr)*cos(yr);
            sf_comp(2, idx[1]) = cos(pr)*sin(yr);
            sf_comp(3, idx[1]) = sin(pr);
            sf_comp(4, idx[1]) = 0.;

            if(trn_debug::get()->debug() >= TRNDL_UTILS_MBSFCOMP_H){
                double rho[3] = {sf_comp(1,idx[1]), sf_comp(2,idx[1]), sf_comp(3,idx[1])};

                double rhoNorm = vnorm(rho);

                const char *sep = (b == 60 ? "****" : "    ");
                TRN_NDPRINT(TRNDL_UTILS_MBSFCOMP_H, "%s - b[%3d] r[%7.2lf] R[%7.2lf] %s Rx[%7.2lf] Ry[%7.2lf] Rz[%7.2lf] %s yd[%7.2lf] pd[%7.2lf] %s cy[%7.2lf] sy[%7.2lf] cp[%7.2lf] sp[%7.2lf]\n",
                            __func__, b, range, rhoNorm,
                            sep, sf_comp(1,idx[1]), sf_comp(2,idx[1]), sf_comp(3,idx[1]),
                            sep, yd, pd, cos(yr), sep, sin(yr), cos(pr), sin(pr));
            }

            idx[0]++;
            idx[1]++;
        }
        TRN_NDPRINT(TRNDL_UTILS_MBSFCOMP, "%s: --- \n\n",__func__);

        return sf_comp;
    }

    static Matrix dvl_sframe_components(trn::bath_info *bi, dvlgeo *geo)
    {

        if(bi == nullptr || geo == NULL){
            Matrix err_ret = Matrix(3,1);
        }

        // number of beams read (<= nominal beams)
        int nbeams = bi->beam_count();

        Matrix sf_comp = Matrix(4,nbeams);

        // beam components in reference sensor frame (mounted center, across track)

        std::list<trn::beam_tup> beams = bi->beams_raw();
        std::list<trn::beam_tup>::iterator it;

        // zero- and one-based indexs
        int idx[2] = {0, 1};

        TRN_NDPRINT(TRNDL_UTILS_DVLSFCOMP, "%s: --- \n",__func__);

        for(it=beams.begin(); it!=beams.end(); it++)
        {
            trn::beam_tup bt = static_cast<trn::beam_tup> (*it);
            double range = std::get<1>(bt);
            // beam number (0-indexed)
            int b = std::get<0>(bt);

            // beam yaw, pitch angles (sensor frame)
            double yd = geo->yaw_rf[idx[0]];
            double pd = geo->pitch_rf[idx[0]];
            double yr = DTR(yd);
            double pr = DTR(pd);

            // beam components (reference orientation, sensor frame)
            // 1: along (x) 2: across (y) 3: down (z)
            sf_comp(1, idx[1]) = cos(pr)*cos(yr);
            sf_comp(2, idx[1]) = cos(pr)*sin(yr);
            sf_comp(3, idx[1]) = sin(pr);
            sf_comp(4, idx[1]) = 0.;

            if(trn_debug::get()->debug() >= 5){

                double rho[3] = {sf_comp(1,idx[1]), sf_comp(2,idx[1]), sf_comp(3,idx[1])};

                double rhoNorm = vnorm(rho);

                const char *sep = (b == 60 ? "****" : "    ");
                TRN_NDPRINT(TRNDL_UTILS_DVLSFCOMP_H, "%s - b[%3d] r[%7.2lf] R[%7.2lf] %s Rx[%7.2lf] Ry[%7.2lf] Rz[%7.2lf] %s yd[%7.2lf] pd[%7.2lf] %s cy[%7.2lf] sy[%7.2lf] cp[%7.2lf] sp[%7.2lf]\n",
                            __func__, b, range, rhoNorm,
                            sep, sf_comp(1,idx[1]), sf_comp(2,idx[1]), sf_comp(3,idx[1]),
                            sep, yd, pd, cos(yr), sep, sin(yr), cos(pr), sin(pr));
            }

            idx[0]++;
            idx[1]++;
        }
        TRN_NDPRINT(TRNDL_UTILS_DVLSFCOMP, "%s: --- \n\n",__func__);

        return sf_comp;
    }

    // process Delta-T sounding where
    // Delta-T mounded on vehicle frame
    // vehcle attitude
    // oisled nav (mounted on rotating arm)
    // oisled attitude (mounted on rotating arm)
    // expects:
    // bi[0] - vehicle bath (deltaT)
    // ai[0] - vehicle att
    // ai[1] - sled att
    // geo[0] - DeltaT geometry
    // xgeo[0] - DVL/Kearfott geometry
    // snd - sounding (w. navigation in vehicle frame)
    static void transform_oi_deltat(trn::bath_info **bi, trn::att_info **ai, dvlgeo **xgeo, mbgeo **geo, mb1_t *r_snd)
    {
        // validate inputs
        if(NULL == geo || geo[0] == nullptr || xgeo[0] ==  nullptr){
            fprintf(stderr, "%s - geometry error : NULL input geo[%p] {%p} xgeo[%p] {%p}\n", __func__, geo, (geo?geo[0]:nullptr), xgeo, (xgeo?xgeo[0]:nullptr));
            return;
        }
        if(geo[0]->beam_count <= 0){
            fprintf(stderr, "%s - geometry error : beams <= 0 {%u}\n", __func__, geo[0]->beam_count);
            return;
        }
        if(NULL == r_snd || NULL == ai|| NULL == bi){
            fprintf(stderr, "%s - ERR invalid argument bi[%p] ai[%p] snd[%p]\n", __func__, bi, ai, r_snd);
            return;
        }

        if(NULL == ai[0] || NULL == ai[1] || NULL == bi[0] ){
            fprintf(stderr, "%s - ERR invalid info ai[0][%p] ai[1][%p] bi[0][%p]\n", __func__, ai[0], ai[1], bi[0]);
            return;
        }

        // vehicle attitude (relative to NED)
        // r/p/y (phi/theta/psi)
        // MB1 assumes vehicle frame, not world frame (i.e. exclude heading)
        double VATT[3] = {ai[1]->roll(), ai[1]->pitch(), 0.};

        // sensor mounting angles (relative to vehicle, radians)
        // 3-2-1 euler angles, r/p/y  (phi/theta/psi)
        // wrt sensor mounted across track, b[0] port, downward facing
        double SROT[3] = { DTR(geo[0]->svr_deg[0]), DTR(geo[0]->svr_deg[1]), DTR(geo[0]->svr_deg[2])};

        // sensor mounting translation offsets (relative to vehicle CRP, meters)
        // +x: fwd +y: stbd, +z:down (aka FSK, fwd,stbd,keel)
        // TODO T is for transform use rSV
        double STRN[3] = {geo[0]->svt_m[0], geo[0]->svt_m[1], geo[0]->svt_m[2]};

        double XTRN[3] = {xgeo[0]->rot_radius_m, 0., 0.};
        double XR = ai[1]->pitch() - ai[0]->pitch();
        double XROT[3] = {0., XR, 0.};

        // beam components in reference sensor frame (mounted center, across track)
        Matrix beams_SF = dvl_sframe_components(bi[0], xgeo[0]);

        TRN_NDPRINT(TRNDL_PLUGOIDELTAT, "%s: --- \n",__func__);

        TRN_NDPRINT(TRNDL_PLUGOIDELTAT, "VATT[%.3lf, %.3lf, %.3lf]\n", VATT[0], VATT[1], VATT[2]);
        TRN_NDPRINT(TRNDL_PLUGOIDELTAT, "SROT[%.3lf, %.3lf, %.3lf]\n", SROT[0], SROT[1], SROT[2]);
        TRN_NDPRINT(TRNDL_PLUGOIDELTAT, "STRN[%.3lf, %.3lf, %.3lf]\n", STRN[0], STRN[1], STRN[2]);

        const char *pinv = (ai[0]->flags().is_set(trn::AF_INVERT_PITCH)? "(p-)" :"(p+)");

        TRN_NDPRINT(TRNDL_PLUGOIDELTAT, "VATT (deg) [%.2lf, %.2lf, %.2lf (%.2lf)] %s\n",
                    Math::radToDeg(VATT[0]), Math::radToDeg(VATT[1]), Math::radToDeg(VATT[2]), Math::radToDeg(ai[0]->heading()), pinv);
        TRN_NDPRINT(TRNDL_PLUGOIDELTAT, "XTRN[%.3lf, %.3lf, %.3lf]\n", XTRN[0], XTRN[1], XTRN[2]);
        TRN_NDPRINT(TRNDL_PLUGOIDELTAT, "XROT[%.3lf, %.3lf, %.3lf]\n", XROT[0], XROT[1], XROT[2]);
        TRN_NDPRINT(TRNDL_PLUGOIDELTAT, "pitch (deg) veh[%.3lf] ois[%.3lf] angle[%.3lf]\n", Math::radToDeg(ai[0]->pitch()), Math::radToDeg(ai[1]->pitch()), Math::radToDeg(XR));
        TRN_NDPRINT(TRNDL_PLUGOIDELTAT, "\n");

        // generate coordinate tranformation matrices

        // translate arm rotation point to sled origin
        Matrix mat_XTRN = affineTranslation(XTRN);
        // sled arm rotation
        Matrix mat_XROT = affine321Rotation(XROT);
        // mounting rotation matrix
        Matrix mat_SROT = affine321Rotation(SROT);
        // mounting translation matrix
        Matrix mat_STRN = affineTranslation(STRN);
        // vehicle attitude (pitch, roll, heading)
        Matrix mat_VATT = affine321Rotation(VATT);

        // combine to get composite tranformation
        // order is significant:
        // mounting rotations, translate
        Matrix S0 = mat_XTRN * mat_SROT;
        // arm rotation
        Matrix S1 = mat_XROT * S0;
        // translate to position on arm
        Matrix S2 = mat_STRN * S1;
        // appy vehicle attitude
        Matrix Q = mat_VATT * S2;

        // apply coordinate transforms
        Matrix beams_VF = Q * beams_SF;

        std::list<trn::beam_tup> beams = bi[0]->beams_raw();
        std::list<trn::beam_tup>::iterator it;

        // fill in the MB1 record using transformed beams
        // zero- and one-based indexs
        int idx[2] = {0, 1};
        for(it=beams.begin(); it!=beams.end(); it++, idx[0]++, idx[1]++)
        {
            // write beam data to MB1 sounding
            trn::beam_tup bt = static_cast<trn::beam_tup> (*it);

            // beam number (0-indexed)
            int b = std::get<0>(bt);
            double range = std::get<1>(bt);
            // beam components WF x,y,z
            // matrix row/col (1 indexed)
            r_snd->beams[idx[0]].beam_num = b;
            r_snd->beams[idx[0]].rhox = range * beams_VF(1, idx[1]);
            r_snd->beams[idx[0]].rhoy = range * beams_VF(2, idx[1]);
            r_snd->beams[idx[0]].rhoz = range * beams_VF(3, idx[1]);

            if(trn_debug::get()->debug() >= 5){

                // calculated beam range (should match measured range)
                double rho[3] = {r_snd->beams[idx[0]].rhox, r_snd->beams[idx[0]].rhoy, r_snd->beams[idx[0]].rhoz};

                double rhoNorm = vnorm( rho );

                // calculate component angles wrt vehicle axes
                double axr = (range==0. ? 0. :acos(r_snd->beams[idx[0]].rhox/range));
                double ayr = (range==0. ? 0. :acos(r_snd->beams[idx[0]].rhoy/range));
                double azr = (range==0. ? 0. :acos(r_snd->beams[idx[0]].rhoz/range));

                TRN_NDPRINT(TRNDL_PLUGOIDELTAT_H, "%s: b[%3d] r[%7.2lf] R[%7.2lf]     rhox[%7.2lf] rhoy[%7.2lf] rhoz[%7.2lf]     ax[%6.2lf] ay[%6.2lf] az[%6.2lf]\n",
                            __func__, b, range, rhoNorm,
                            r_snd->beams[idx[0]].rhox,
                            r_snd->beams[idx[0]].rhoy,
                            r_snd->beams[idx[0]].rhoz,
                            Math::radToDeg(axr),
                            Math::radToDeg(ayr),
                            Math::radToDeg(azr)
                            );
            }
        }
        TRN_NDPRINT(TRNDL_PLUGOIDELTAT, "%s: --- \n\n",__func__);

        return;
    }

    // process Delta-T sounding where
    // Delta-T mounded on vehicle frame
    // vehcle attitude
    // oisled nav (mounted on rotating arm)
    // oisled attitude (mounted on rotating arm)
    // expects:
    // bi[0] - vehicle bath (deltaT)
    // ai[0] - vehicle att
    // ai[1] - sled att
    // geo[0] - DeltaT geometry
    // xgeo[0] - DVL/Kearfott geometry
    // snd - sounding (w. navigation in vehicle frame)
//    static void transform_oi_deltat_orig(trn::bath_info **bi, trn::att_info **ai, dvlgeo **xgeo, mbgeo **geo, mb1_t *r_snd)
//    {
//        // validate inputs
//        if(NULL == geo || geo[0] == nullptr || xgeo[0] ==  nullptr){
//            fprintf(stderr, "%s - geometry error : NULL input geo[%p] {%p} xgeo[%p] {%p}\n", __func__, geo, (geo?geo[0]:nullptr), xgeo, (xgeo?xgeo[0]:nullptr));
//            return;
//        }
//        if(geo[0]->beam_count <= 0){
//            fprintf(stderr, "%s - geometry error : beams <= 0 {%u}\n", __func__, geo[0]->beam_count);
//            return;
//        }
//        if(NULL == r_snd || NULL == ai|| NULL == bi){
//            fprintf(stderr, "%s - ERR invalid argument bi[%p] ai[%p] snd[%p]\n", __func__, bi, ai, r_snd);
//            return;
//        }
//
//        if(NULL == ai[0] || NULL == ai[1] || NULL == bi[0] ){
//            fprintf(stderr, "%s - ERR invalid info ai[0][%p] ai[1][%p] bi[0][%p]\n", __func__, ai[0], ai[1], bi[0]);
//            return;
//        }
//
//        // vehicle attitude (relative to NED)
//        // r/p/y (phi/theta/psi)
//        // MB1 assumes vehicle frame, not world frame (i.e. exclude heading)
//        double VATT[3] = {ai[1]->roll(), ai[1]->pitch(), 0.};
//
//        // sensor mounting angles (relative to vehicle, radians)
//        // 3-2-1 euler angles, r/p/y  (phi/theta/psi)
//        // wrt sensor mounted across track, b[0] port, downward facing
//        double SROT[3] = { DTR(geo[0]->svr_deg[0]), DTR(geo[0]->svr_deg[1]), DTR(geo[0]->svr_deg[2])};
//
//        // sensor mounting translation offsets (relative to vehicle CRP, meters)
//        // +x: fwd +y: stbd, +z:down (aka FSK, fwd,stbd,keel)
//        // TODO T is for transform use rSV
//        double STRN[3] = {geo[0]->svt_m[0], geo[0]->svt_m[1], geo[0]->svt_m[2]};
//
//        double XTRN[3] = {xgeo[0]->rot_radius_m, 0., 0.};
//        double XR = ai[1]->pitch() - ai[0]->pitch();
//        double XROT[3] = {0., XR, 0.};
//
//        // beam components in reference sensor frame (mounted center, across track)
//        Matrix beams_SF = dvl_sframe_components(bi[0], xgeo[0]);
//
//        TRN_NDPRINT(TRNDL_PLUGOIDELTAT, "%s: --- \n",__func__);
//
//        TRN_NDPRINT(TRNDL_PLUGOIDELTAT, "VATT[%.3lf, %.3lf, %.3lf]\n", VATT[0], VATT[1], VATT[2]);
//        TRN_NDPRINT(TRNDL_PLUGOIDELTAT, "SROT[%.3lf, %.3lf, %.3lf]\n", SROT[0], SROT[1], SROT[2]);
//        TRN_NDPRINT(TRNDL_PLUGOIDELTAT, "STRN[%.3lf, %.3lf, %.3lf]\n", STRN[0], STRN[1], STRN[2]);
//
//        const char *pinv = (ai[0]->flags().is_set(trn::AF_INVERT_PITCH)? "(p-)" :"(p+)");
//
//        TRN_NDPRINT(TRNDL_PLUGOIDELTAT, "VATT (deg) [%.2lf, %.2lf, %.2lf (%.2lf)] %s\n",
//                    Math::radToDeg(VATT[0]), Math::radToDeg(VATT[1]), Math::radToDeg(VATT[2]), Math::radToDeg(ai[0]->heading()), pinv);
//        TRN_NDPRINT(TRNDL_PLUGOIDELTAT, "XTRN[%.3lf, %.3lf, %.3lf]\n", XTRN[0], XTRN[1], XTRN[2]);
//        TRN_NDPRINT(TRNDL_PLUGOIDELTAT, "XROT[%.3lf, %.3lf, %.3lf]\n", XROT[0], XROT[1], XROT[2]);
//        TRN_NDPRINT(TRNDL_PLUGOIDELTAT, "pitch (deg) veh[%.3lf] ois[%.3lf] angle[%.3lf]\n", Math::radToDeg(ai[0]->pitch()), Math::radToDeg(ai[1]->pitch()), Math::radToDeg(XR));
//        TRN_NDPRINT(TRNDL_PLUGOIDELTAT, "\n");
//
//        // generate coordinate tranformation matrices
//
//        // translate arm rotation point to sled origin
//        Matrix mat_XTRN = affineTranslation(XTRN);
//        // sled arm rotation
//        Matrix mat_XROT = affine321Rotation(XROT);
//        // mounting rotation matrix
//        Matrix mat_SROT = affine321Rotation(SROT);
//        // mounting translation matrix
//        Matrix mat_STRN = affineTranslation(STRN);
//        // vehicle attitude (pitch, roll, heading)
//        Matrix mat_VATT = affine321Rotation(VATT);
//
//        // combine to get composite tranformation
//        // order is significant:
//        // mounting rotations, translate
//        Matrix S0 = mat_XTRN * mat_SROT;
//        // arm rotation
//        Matrix S1 = mat_XROT * S0;
//        // translate to position on arm
//        Matrix S2 = mat_STRN * S1;
//        // appy vehicle attitude
//        Matrix Q = mat_VATT * S2;
//
//        // apply coordinate transforms
//        Matrix beams_VF = Q * beams_SF;
//
//        std::list<trn::beam_tup> beams = bi[0]->beams_raw();
//        std::list<trn::beam_tup>::iterator it;
//
//        // fill in the MB1 record using transformed beams
//        // zero- and one-based indexs
//        int idx[2] = {0, 1};
//        for(it=beams.begin(); it!=beams.end(); it++, idx[0]++, idx[1]++)
//        {
//            // write beam data to MB1 sounding
//            trn::beam_tup bt = static_cast<trn::beam_tup> (*it);
//
//            // beam number (0-indexed)
//            int b = std::get<0>(bt);
//            double range = std::get<1>(bt);
//            // beam components WF x,y,z
//            // matrix row/col (1 indexed)
//            r_snd->beams[idx[0]].beam_num = b;
//            r_snd->beams[idx[0]].rhox = range * beams_VF(1, idx[1]);
//            r_snd->beams[idx[0]].rhoy = range * beams_VF(2, idx[1]);
//            r_snd->beams[idx[0]].rhoz = range * beams_VF(3, idx[1]);
//
//            if(trn_debug::get()->debug() >= 5){
//
//                // calculated beam range (should match measured range)
//                double rho[3] = {r_snd->beams[idx[0]].rhox, r_snd->beams[idx[0]].rhoy, r_snd->beams[idx[0]].rhoz};
//
//                double rhoNorm = vnorm( rho );
//
//                // calculate component angles wrt vehicle axes
//                double axr = (range==0. ? 0. :acos(r_snd->beams[idx[0]].rhox/range));
//                double ayr = (range==0. ? 0. :acos(r_snd->beams[idx[0]].rhoy/range));
//                double azr = (range==0. ? 0. :acos(r_snd->beams[idx[0]].rhoz/range));
//
//                TRN_NDPRINT(TRNDL_PLUGOIDELTAT_HI, "%s: b[%3d] r[%7.2lf] R[%7.2lf]     rhox[%7.2lf] rhoy[%7.2lf] rhoz[%7.2lf]     ax[%6.2lf] ay[%6.2lf] az[%6.2lf]\n",
//                            __func__, b, range, rhoNorm,
//                            r_snd->beams[idx[0]].rhox,
//                            r_snd->beams[idx[0]].rhoy,
//                            r_snd->beams[idx[0]].rhoz,
//                            Math::radToDeg(axr),
//                            Math::radToDeg(ayr),
//                            Math::radToDeg(azr)
//                            );
//            }
//        }
//        TRN_NDPRINT(TRNDL_PLUGOIDELTAT, "%s: --- \n\n",__func__);
//
//        return;
//    }

    // returns allocated mb1_t; caller must release using mb1_destroy()
    static mb1_t *lcm_to_mb1(trn::bath_info *bi, trn::nav_info *ni, trn::att_info *ai)
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
        snd->ping_number = bi->ping_number();//ping_number;
        snd->ts = ni->time_usec()/1e6;
        retval = snd;


        return retval;
    }

    static mb1_t *lcm_to_mb1(trn::mb1_info *bi, trn::nav_info *ni, trn::att_info *ai)
    {
        mb1_t *retval = nullptr;

        if(bi == nullptr || ni == nullptr || ai == nullptr){
            fprintf(stderr,"%s:%d ERR - invalid arg bi[%p] ni[%p] ai[%p]\n", __func__, __LINE__, bi, ni, ai);
            return retval;
        }

        size_t n_beams = bi->nbeams();

        if(n_beams <= 0){
            fprintf(stderr,"%s:%d WARN - beams <= 0 %lu\n", __func__, __LINE__, (long unsigned)n_beams);
        }

        mb1_t *snd = mb1_new(n_beams);
        snd->hdg = bi->heading();
        snd->depth = bi->depth();
        snd->lat = bi->lat();
        snd->lon = bi->lon();
        snd->type = MB1_TYPE_ID;
        snd->size = MB1_SOUNDING_BYTES(n_beams);
        snd->nbeams = n_beams;
        snd->ping_number = bi->ping_number();//ping_number;
        snd->ts = bi->time_usec()/1e6;
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

        const char *pinv = (ai->flags().is_set(trn::AF_INVERT_PITCH)? "p-" :"(p+)");

        TRN_NDPRINT(2, "%s:%d x[%.4lf] y[%.4lf] depth[%.1lf] r/p/y[%.2lf %.2lf, %.2lf] %s vx[%.2lf]\n", __func__, __LINE__, x, y, z, phi, theta, psi, pinv, vx);

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

    static std::string mb1_to_csv(mb1_t *snd, trn::bath_info *bi, trn::att_info *ai, trn::vel_info *vi=nullptr)
    {

        std::ostringstream ss;
        if(nullptr != snd && nullptr != ai)
        {
            double lat = snd->lat;
            double lon = snd->lon;
            double pos_N=0;
            double pos_E=0;
            long int utm = NavUtils::geoToUtmZone(Math::degToRad(lat),
                                                  Math::degToRad(lon));

            // NavUtils::geoToUtm(latitude, longitude, utmZone, *northing, *easting)
            NavUtils::geoToUtm(Math::degToRad(lat), Math::degToRad(lon), utm, &pos_N, &pos_E);
            // Note that TRN uses N,E,D frame (i.e. N:x E:y D:z)
            // [ 0] time POSIX epoch sec
            // [ 1] northings
            // [ 2] eastings
            // [ 3] depth
            // [ 4] heading
            // [ 5] pitch
            // [ 6] roll
            // [ 7] flag (0)
            // [ 8] flag (0)
            // [ 9] flag (0)
            // [10] vx (0)
            // [11] xy (0)
            // [12] vz (0)
            // [13] sounding valid flag (1)
            // [14] bottom lock valid flag (1)
            // [15] number of beams
            // beam[16 + i*3] number
            // beam[17 + i*3] valid (always 1)
            // beam[18 + i*3] range
            // ...
            // NEWLINE

            ss << std::dec << std::setfill(' ') << std::fixed << std::setprecision(7);
            ss << snd->ts << ",";
            ss << std::setprecision(7);
            ss << pos_N << ",";
            ss << pos_E << ",";
            ss << snd->depth << ",";
            ss << snd->hdg << ",";
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
            // sounding valid flag
            ss << (bi->flags().is_set(trn::BF_VALID)?1:0) << ",";
            // bottom lock flag
            ss << (bi->flags().is_set(trn::BF_BLOCK)?1:0) << ",";
            ss << snd->nbeams << ",";
            ss << std::setprecision(4);

            for(int i=0; i < snd->nbeams; i++)
            {
                // beam number
                ss <<  snd->beams[i].beam_num << ",";
                // valid (set to 1)
                ss << 1 << ",";
                // range
                double comp[3] = {snd->beams[i].rhox, snd->beams[i].rhoz, snd->beams[i].rhoz};
                double range = vnorm(comp);

                ss << range;
                if(i < (snd->nbeams-1))
                    ss << ",";
            }
        }
        return ss.str();
    }

    static std::string mb1_to_csv(mb1_t *snd, trn::mb1_info *bi, trn::att_info *ai, trn::vel_info *vi=nullptr)
    {

        std::ostringstream ss;
        if(nullptr != snd && nullptr != ai)
        {
            double lat = snd->lat;
            double lon = snd->lon;
            double pos_N=0;
            double pos_E=0;
            long int utm = NavUtils::geoToUtmZone(Math::degToRad(lat),
                                                  Math::degToRad(lon));

            // NavUtils::geoToUtm(latitude, longitude, utmZone, *northing, *easting)
            NavUtils::geoToUtm(Math::degToRad(lat), Math::degToRad(lon), utm, &pos_N, &pos_E);
            // Note that TRN uses N,E,D frame (i.e. N:x E:y D:z)
            // [ 0] time POSIX epoch sec
            // [ 1] northings
            // [ 2] eastings
            // [ 3] depth
            // [ 4] heading
            // [ 5] pitch
            // [ 6] roll
            // [ 7] flag (0)
            // [ 8] flag (0)
            // [ 9] flag (0)
            // [10] vx (0)
            // [11] xy (0)
            // [12] vz (0)
            // [13] sounding valid flag (1)
            // [14] bottom lock valid flag (1)
            // [15] number of beams
            // beam[16 + i*3] number
            // beam[17 + i*3] valid (always 1)
            // beam[18 + i*3] range
            // ...
            // NEWLINE

            ss << std::dec << std::setfill(' ') << std::fixed << std::setprecision(7);
            ss << snd->ts << ",";
            ss << std::setprecision(7);
            ss << pos_N << ",";
            ss << pos_E << ",";
            ss << snd->depth << ",";
            ss << snd->hdg << ",";
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
            // sounding valid flag
            ss << (bi->flags().is_set(trn::BF_VALID)?1:0) << ",";
            // bottom lock flag
            ss << (bi->flags().is_set(trn::BF_BLOCK)?1:0) << ",";
            ss << snd->nbeams << ",";
            ss << std::setprecision(4);

            for(int i=0; i < snd->nbeams; i++)
            {
                // beam number
                ss <<  snd->beams[i].beam_num << ",";
                // valid (set to 1)
                ss << 1 << ",";
                // range
                double comp[3] = {snd->beams[i].rhox, snd->beams[i].rhoz, snd->beams[i].rhoz};
                double range = vnorm(comp);

                ss << range;
                if(i < (snd->nbeams-1))
                    ss << ",";
            }
        }
        return ss.str();
    }

    static std::string lcm_to_csv_raw(trn::bath_info *bi, trn::att_info *ai, trn::nav_info *ni, trn::vel_info *vi=nullptr)
    {
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
            // [ 0] time POSIX epoch sec
            // [ 1] northings
            // [ 2] eastings
            // [ 3] depth
            // [ 4] heading
            // [ 5] pitch
            // [ 6] roll
            // [ 7] flag (0)
            // [ 8] flag (0)
            // [ 9] flag (0)
            // [10] vx (0)
            // [11] xy (0)
            // [12] vz (0)
            // [13] sounding valid flag
            // [14] bottom lock valid flag
            // [15] number of beams
            // beam[16 + i*3] number
            // beam[17 + i*3] valid (always 1)
            // beam[18 + i*3] range
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
                // beam number
                ss <<  std::get<0>(bt) << ",";
                // valid (set to 1)
                ss << 1 << ",";
                // range
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
        os << std::setw(wkey) << "beam range" << " [rx, ry, rz, val]" <<"\n";

        for(int i=0; i < src.numMeas; i++){
            os << std::setw(wkey) << " [" << setw(3) << src.beamNums[i] << "]";
            ostringstream ss;
            ss << std::fixed << std::setprecision(2);
            ss << std::setw(7) << src.ranges[i];
            ss << " [";
            ss << std::setw(7) << src.alongTrack[i] << ", ";
            ss << std::setw(7) << src.crossTrack[i] << ", ";
            ss << std::setw(7) << src.altitudes[i] << ", ";
            ss << std::setw(2) << src.measStatus[i] << "]";
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
           obj->phi = ai->roll();
           obj->theta = ai->pitch();
           obj->psi = src->hdg;
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
            obj->phi = ai->roll();
            obj->theta = ai->pitch();
            obj->psi = src->hdg;
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
