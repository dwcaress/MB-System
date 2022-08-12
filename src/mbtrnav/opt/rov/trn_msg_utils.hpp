/// @file trn_msg_utils.hpp
/// @authors k. headley
/// @date 21mar2022

/// Summary: TRN to LCM message conversion

// ///////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute
// Distributed under MIT license. See LICENSE file for more information.

#ifndef TRN_MSG_UTILS_HPP
#define TRN_MSG_UTILS_HPP

#include <errno.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "mb1_msg.h"
#include "trn_msg.h"
#include "trn/trn_meas_t.hpp"
#include "trn/trn_pose_t.hpp"
#include "trn/trnupub_t.hpp"
#include "trn/trn_mb1_t.hpp"
#include "trn/trn_stat_t.hpp"

#define TMU_DBG(...) if(mDebug != 0)fprintf(__VA_ARGS__)
#define TMU_NDBG(n,...) if(n <= mDebug)fprintf(__VA_ARGS__)

namespace trn {

class trn_msg_utils
{
public:
    trn_msg_utils()
    {
    }

    ~trn_msg_utils()
    {}

    static void set_debug(int dbg)
    {
        mDebug=dbg;
    }

    static trn_meas_t &meas_to_lcm(trn_meas_t &dest, measT &src)
    {
        dest.time = src.time;
        dest.dataType = src.dataType;
        dest.ping_number = src.ping_number;
        dest.phi = src.phi;
        dest.theta = src.theta;
        dest.psi = src.psi;
        dest.x = src.x;
        dest.y = src.y;
        dest.z = src.z;
        dest.numMeas = src.numMeas;
        for(int i=0; i < src.numMeas; i++)
        {
            dest.covariance.push_back(src.covariance[i]);
            dest.ranges.push_back(src.ranges[i]);
            dest.crossTrack.push_back(src.crossTrack[i]);
            dest.alongTrack.push_back(src.alongTrack[i]);
            dest.altitudes.push_back(src.altitudes[i]);
            dest.alphas.push_back(src.alphas[i]);
            dest.measStatus.push_back(src.measStatus[i]);
            dest.beamNums.push_back(src.beamNums[i] ? 1 : 0);
        }
        return dest;
    }

    static trn_pose_t &pose_to_lcm(trn_pose_t &dest, poseT &src)
    {
        dest.x = src.x;
        dest.y = src.y;
        dest.z = src.z;
        dest.vx = src.vx;
        dest.vy = src.vy;
        dest.vz = src.vz;
        dest.ve = src.ve;
        dest.vw_x = src.vw_x;
        dest.vw_y = src.vw_y;
        dest.vw_z = src.vw_z;
        dest.vn_x = src.vn_x;
        dest.vn_y = src.vn_y;
        dest.vn_z = src.vn_z;
        dest.ax = src.ax;
        dest.ay = src.ay;
        dest.az = src.az;
        dest.phi = src.phi;
        dest.theta = src.theta;
        dest.psi = src.psi;
        dest.psi_berg = src.psi_berg;
        dest.psi_dot_berg = src.psi_dot_berg;
        dest.time = src.time;
        dest.dvlValid = src.dvlValid?1:0;
        dest.gpsValid = src.gpsValid?1:0;
        dest.bottomLock = src.bottomLock?1:0;
        dest.ncovar = N_COVAR;
        for(int i=0; i < N_COVAR; i++)
        {
            dest.covariance.push_back(src.covariance[i]);
        }
        return dest;
    }

    static trnupub_t &trnupub_to_lcm(trnupub_t &dest, trnu_pub_t *src)
    {
        dest.sync = src->sync;
        for(int i=0; i < 5; i++){
            dest.est[i].time = src->est[i].time;
            dest.est[i].x = src->est[i].x;
            dest.est[i].y = src->est[i].y;
            dest.est[i].z = src->est[i].z;
            dest.est[i].cov[0] = src->est[i].cov[0];
            dest.est[i].cov[1] = src->est[i].cov[1];
            dest.est[i].cov[2] = src->est[i].cov[2];
            dest.est[i].cov[3] = src->est[i].cov[3];
        }
        dest.reinit_count = src->reinit_count;
        dest.reinit_tlast = src->reinit_tlast;
        dest.filter_state = src->filter_state;
        dest.success = src->success;
        dest.is_converged = src->is_converged;
        dest.is_valid = src->is_valid;
        dest.mb1_cycle = src->mb1_cycle;
        dest.ping_number = src->ping_number;
        dest.n_con_seq = src->n_con_seq;
        dest.n_con_tot = src->n_con_tot;
        dest.n_uncon_seq = src->n_uncon_seq;
        dest.n_uncon_tot = src->n_uncon_tot;
        dest.mb1_time = src->mb1_time;
        dest.reinit_time = src->reinit_time;
        dest.update_time = src->update_time;

        return dest;
    }

    static trn_mb1_t &mb1_to_lcm(trn_mb1_t &dest, mb1_t *src)
    {
        dest.type = src->type;
        dest.size = src->size;
        dest.ts = src->ts;
        dest.lat = src->lat;
        dest.lon = src->lon;
        dest.depth = src->depth;
        dest.hdg = src->hdg;
        dest.ping_number = src->ping_number;
        dest.nbeams = src->nbeams;
        dest.checksum = MB1_GET_CHECKSUM(src);
        for(uint32_t i=0; i < src->nbeams; i++)
        {
            dest.beams.push_back(mb1_beam_t());
            dest.beams[i].beam_num = src->beams[i].beam_num;
            dest.beams[i].rhox = src->beams[i].rhox;
            dest.beams[i].rhoy = src->beams[i].rhoy;
            dest.beams[i].rhoz = src->beams[i].rhoz;
        }
        return dest;
    }

    static trn_stat_t &trn_to_lcm(trn_stat_t &dest, const std::string &src_name, poseT &pose_src, poseT &mmse_src, poseT &mle_src)
    {
        dest.update_time = mmse_src.time;
        dest.source = src_name;

        dest.est[0].time = pose_src.time;
        dest.est[0].x = pose_src.x;
        dest.est[0].y = pose_src.y;
        dest.est[0].z = pose_src.z;
        dest.est[0].cov[0] = pose_src.covariance[0];
        dest.est[0].cov[1] = pose_src.covariance[2];
        dest.est[0].cov[2] = pose_src.covariance[5];
        dest.est[0].cov[3] = pose_src.covariance[1];

        dest.est[1].time = mle_src.time;
        dest.est[1].x = mle_src.x;
        dest.est[1].y = mle_src.y;
        dest.est[1].z = mle_src.z;
        dest.est[1].cov[0] = mle_src.covariance[0];
        dest.est[1].cov[1] = mle_src.covariance[2];
        dest.est[1].cov[2] = mle_src.covariance[5];
        dest.est[1].cov[3] = mle_src.covariance[1];

        dest.est[2].time = mmse_src.time;
        dest.est[2].x = mmse_src.x;
        dest.est[2].y = mmse_src.y;
        dest.est[2].z = mmse_src.z;
        dest.est[2].cov[0] = mmse_src.covariance[0];
        dest.est[2].cov[1] = mmse_src.covariance[2];
        dest.est[2].cov[2] = mmse_src.covariance[5];
        dest.est[2].cov[3] = mmse_src.covariance[1];

        dest.est[3].time = mmse_src.time;
        dest.est[3].x = pose_src.x - mmse_src.x;
        dest.est[3].y = pose_src.y - mmse_src.y;
        dest.est[3].z = pose_src.z - mmse_src.z;
        dest.est[3].cov[0] = sqrt(mmse_src.covariance[0]);
        dest.est[3].cov[1] = sqrt(mmse_src.covariance[2]);
        dest.est[3].cov[2] = sqrt(mmse_src.covariance[5]);
        dest.est[3].cov[3] = sqrt(mmse_src.covariance[1]);
        return dest;
    }


private:
    static int mDebug;
};
}
#endif
