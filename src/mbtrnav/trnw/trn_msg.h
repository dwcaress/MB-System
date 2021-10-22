///
/// @file trn_msg.h
/// @authors k. headley
/// @date 09 jul 2019

/// MBSystem TRN legacy message defs for C

/// @sa doxygen-examples.c for more examples of Doxygen markup


/////////////////////////
// Terms of use
/////////////////////////
/*
 Copyright Information

 Copyright 2002-YYYY MBARI
 Monterey Bay Aquarium Research Institute, all rights reserved.

 Terms of Use

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version. You can access the GPLv3 license at
 http://www.gnu.org/licenses/gpl-3.0.html

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details
 (http://www.gnu.org/licenses/gpl-3.0.html)

 MBARI provides the documentation and software code "as is", with no warranty,
 express or implied, as to the software, title, non-infringement of third party
 rights, merchantability, or fitness for any particular purpose, the accuracy of
 the code, or the performance or results which you may obtain from its use. You
 assume the entire risk associated with use of the code, and you agree to be
 responsible for the entire cost of repair or servicing of the program with
 which you are using the code.

 In no event shall MBARI be liable for any damages, whether general, special,
 incidental or consequential damages, arising out of your use of the software,
 including, but not limited to, the loss or corruption of your data or damages
 of any kind resulting from use of the software, any prohibited use, or your
 inability to use the software. You agree to defend, indemnify and hold harmless
 MBARI and its officers, directors, and employees against any claim, loss,
 liability or expense, including attorneys' fees, resulting from loss of or
 damage to property or the injury to or death of any person arising out of the
 use of the software.

 The MBARI software is provided without obligation on the part of the
 Monterey Bay Aquarium Research Institute to assist in its use, correction,
 modification, or enhancement.

 MBARI assumes no responsibility or liability for any third party and/or
 commercial software required for the database or applications. Licensee agrees
 to obtain and maintain valid licenses for any additional third party software
 required.
 */

// Always do this
#ifndef TRN_MSG_H
#define TRN_MSG_H

/////////////////////////
// Includes
/////////////////////////
#include "trn_common.h"
#include <stdint.h>

/////////////////////////
// Macros
/////////////////////////
#define TRN_MSG_SIZE 8000

#ifndef N_COVAR
#define N_COVAR 45
#endif
#define  TRN_SENSOR_DVL     1
#define  TRN_SENSOR_MB      2
#define  TRN_SENSOR_PENCIL  3
#define  TRN_SENSOR_HOMER   4
#define  TRN_SENSOR_DELTAT  5

#define TRN_FILT_NONE       0
#define TRN_FILT_POINTMASS  1
#define TRN_FILT_PARTICLE   2
#define TRN_FILT_BANK       3

#define TRN_FILT_HIGH  1
#define TRN_FILT_LOW   0

#define TRN_FILT_REINIT_EN  1
#define TRN_FILT_REINIT_DIS 0

// 0 - No weighting modifications.
// 1 - Shandor's original alpha modification.
// 2 - Crossbeam with Shandor's weighting.
// 3 - Subcloud with Shandor's original.
// 4 - Subcloud with modified NIS always on.
#define TRN_MWEIGHT_NONE             0
#define TRN_MWEIGHT_SHANDOR          1
#define TRN_MWEIGHT_CROSSBEAM        2
#define TRN_MWEIGHT_SUBCLOUD_SHANDOR 3
#define TRN_MWEIGHT_SUBCLOUD_NISON   4

#define TRN_MAP_DEM  1
#define TRN_MAP_BO   2

#define TRN_MEAS_TYPE_DVL  1
#define TRN_MEAS_TYPE_MB   2
#define TRN_MEAS_TYPE_SB   3
#define TRN_MEAS_TYPE_HREL 4
#define TRN_MEAS_TYPE_IMMB 5
#define TRN_MEAS_TYPE_SDVL 6

#define TRN_POSE_MLE    1
#define TRN_POSE_MMSE   2

#define TRN_MSG_INIT        'I'
#define TRN_MSG_MEAS        'M'
#define TRN_MSG_MOTN        'N'
#define TRN_MSG_MLE         'E'
#define TRN_MSG_MMSE        'S'
#define TRN_MSG_SET_MW      'W'
#define TRN_MSG_SET_FR      'F'
#define TRN_MSG_SET_IMA     'A'
#define TRN_MSG_SET_VDR     'D'
#define TRN_MSG_SET_MIM     'Q'
#define TRN_MSG_FILT_GRD    'G'
#define TRN_MSG_ACK         '+'
#define TRN_MSG_NACK        '-'
#define TRN_MSG_BYE         'B'
#define TRN_MSG_OUT_MEAS    'O'
#define TRN_MSG_LAST_MEAS   'L'
#define TRN_MSG_IS_CONV     'C'
#define TRN_MSG_FILT_TYPE   'T'
#define TRN_MSG_FILT_STATE  'H'
#define TRN_MSG_N_REINITS   'R'
#define TRN_MSG_FILT_REINIT 'r'
#define TRN_MSG_FILT_REINIT_OFFSET 'o'
#define TRN_MSG_FILT_REINIT_BOX 'b'
#define TRN_MSG_SET_INITSTDDEVXYZ 'x'
#define TRN_MSG_GET_INITSTDDEVXYZ 'X'
#define TRN_MSG_SET_ESTNAVOFS 'j'
#define TRN_MSG_GET_ESTNAVOFS 'J'
// extension
#define TRN_MSG_PING        '?'
#define TRN_MSG_IS_INIT     'i'

// TRN_MSG_FILT_GRD param
typedef enum{
TRN_GRD_LOW=0,
TRN_GRD_HIGH=1
}trn_fgird_id;

// TRN_MSG_SET_MW param
typedef enum{
    TRN_WTYPE_NONE  = 0,       // No weighting modifications at all.
    TRN_WTYPE_NORM  = 1,       // Shandor's original alpha modification.
    TRN_WTYPE_XBEAM = 2,       // Crossbeam with original
    TRN_WTYPE_SUBCL = 3,       // Subcloud  with original
    TRN_WTYPE_FORCE_SUBCL = 4,    // Forced to do Subcloud on every measurement
    TRN_WTYPE_INVAL = 5        // Any value here and above is invalid
}trn_wmethod_id;

// TRN_MSG_SET_MIM param
typedef enum {
    TRN_MIM_NONE     =0,
    TRN_MIM_BILINEAR =1,
    TRN_MIM_BICUBIC  =2,
    TRN_MIM_SPLINE   =3
}trn_mim_id;

// TRN_MSG_FILT_STATE param
typedef enum
{
    TRN_FTYPE_NONE  = 0,       // Undefined.
    TRN_FTYPE_POINTMASS  = 1,  // Point Mass Filter
    TRN_FTYPE_PARTICLE   = 2,  // Particle Filter
    TRN_FTYPE_BANK       = 3,  // Bank of Particle Filters.
}trn_filter_state_id;

/////////////////////////
// Type Definitions
/////////////////////////

#pragma pack(push,1)
typedef struct pt_cdata_s{
    //North, East, Down position (m)
    double x, y, z;
    //Vehicle velocity wrto iceberg, coordinatized in Body Frame(m/s)
    double vx, vy, vz, ve;
    //Vehicle velocity wrto water, coordinatized in Body (m/s)
    double vw_x, vw_y, vw_z;
    //Vehicle velocity wrto an inertial frame, coordinatized in Body (m/s)
    double vn_x, vn_y, vn_z;
	//Vehicle angular velocity wrto an inertial frame, coordinatized in Body (rad/sec)
    double wx, wy, wz;
    //Vehicle aceleration wrto an inertial frame coordinatized in Body (m/s^2)
    double ax, ay, az;
    //3-2-1 Euler angles relating the B frame to an inertial NED frame (rad).
    double phi, theta, psi;
    //TRN states
    double psi_berg, psi_dot_berg;

    //Time (s)
    double time;

    //Validity flag for dvl motion measurement
    unsigned char dvlValid;
    //Validity flag for GPS measurement
    unsigned char gpsValid;
    //Validity flag for DVL lock onto seafloor
    unsigned char bottomLock;

    //XYZ, phi, theta, psi, wy, wz covariance (passively stable in roll) (see above units)
    double covariance[N_COVAR];

}pt_cdata_t;
#pragma pack(pop)

#define TRNU_EST_DIM 5
#define TRNU_COV_DIM 4

#define TRNU_EST_PT   0
#define TRNU_EST_MLE  1
#define TRNU_EST_MMSE 2
#define TRNU_EST_OFFSET 3
#define TRNU_EST_LAST_GOOD 4
#define TRNU_COV_X   0
#define TRNU_COV_Y   1
#define TRNU_COV_Z   2
#define TRNU_COV_XY  3
#define TRNU_PUB_SYNC 0x53445400
#define TRNU_PUB_BYTES (sizeof(trnu_pub_t))

#pragma pack(push,1)
typedef struct trn_estimate_s{
    // Time (epoch s)
    double time;
    // North
    double x;
    // East
    double y;
    // Down
    double z;
    // Covariance matrix
    // - symmetric 3x3 matrix
    // - only (4) elements needed: diagonal and COV(XY) elements
    // [0] : x : poset.covariance[0]
    // [1] : y : poset.covariance[2]
    // [2] : z : poset.covariance[5]
    // [3] : xy: poset.covariance[1]
    double cov[4];
}trnu_estimate_t;

typedef struct trnu_pub_org_s{
    // sync bytes (see TRNU_PUB_SYNC)
    uint32_t sync;
    // TRN estimates
    // 0:pose_t 1:mle 2:mmse
    trnu_estimate_t est[3];
    // number of reinits
    int reinit_count;
    // time of last reinint (not implemented)
    double reinit_tlast;
    // TRN filter state
    int filter_state;
    // last measurement successful
    int success;
    // TRN is_converged (deprecated, use is_valid)
    short int is_converged;
    // TRN is_valid (covariance thresholds)
    short int is_valid;
    // mbtrnpp MB1 cycle counter
    int mb1_cycle;
    // MB1 ping number
    int ping_number;
    // MB1 timestamp
    double mb1_time;
    // TRN update time (taken in mbtrnpp)
    double update_time;
}trnu_pub_org_t;

typedef struct trnu_pub_s{
    // sync bytes (see TRNU_PUB_SYNC)
    uint32_t sync;
    // TRN estimates
    // 0:pose_t 1:mle 2:mmse 3: offset 4: most recent useful offset
    trnu_estimate_t est[5];
    // number of reinits
    int reinit_count;
    // time of last reinint (not implemented)
    double reinit_tlast;
    // TRN filter state
    int filter_state;
    // last measurement successful
    int success;
    // TRN is_converged
    short int is_converged;
    // TRN is_valid (covariance thresholds) - flag indicating this offset is reliable and can be used
    short int is_valid;
    // mbtrnpp MB1 cycle counter
    int mb1_cycle;
    // MB1 ping number
    int ping_number;
    // Number of current streak of converged estimates
    int n_con_seq;
    // Total number of unconverged estimates
    int n_con_tot;
    // Number of current streak of converged estimates
    int n_uncon_seq;
    // Total number of unconverged estimates
    int n_uncon_tot;
    // MB1 timestamp
    double mb1_time;
    // Time of most recent reinit (epoch seconds)
    double reinit_time;
    // TRN update time (taken in mbtrnpp)
    double update_time;
}trnu_pub_t;

#if !defined(__QNX__)
typedef struct trnu_pub_future_s{
    // sync bytes (see TRNU_PUB_SYNC)
    uint32_t sync;

    // Ping time (epoch seconds)
    double time;

    // Realtime position (pose_t)
    double nav_x; // North (meters)
    double nav_y; // East (meters)
    double nav_z; // Down (meters)

    // TRN MMSE estimate position and covariance
    double trn_x; // North (meters)
    double trn_y; // East (meters)
    double trn_z; // Down (meters)
    // Covariance matrix
    // - symmetric 3x3 matrix
    // - only (4) elements needed: diagonal and COV(XY) elements
    // [0] : x : poset.covariance[0]
    // [1] : y : poset.covariance[2]
    // [2] : z : poset.covariance[5]
    // [3] : xy: poset.covariance[1]
    double trn_cov[4];

    // Current Navigation offset estimate (trn - nav)
    double off_x; // North (meters)
    double off_y; // East (meters)
    double off_z; // Down (meters)

    // Most recent reliable navigation offset estimate (trn - nav)
    double off_use_time;  // Epoch seconds
    double off_use_x; // North (meters)
    double off_use_y; // East (meters)
    double off_use_z; // Down (meters)

    // Metrics
    int mb1_cycle;      // mbtrnpp MB1 cycle counter
    int ping_number;    // Current multibeam ping number
    int n_con_seq;      // Number of current streak of converged estimates
    int n_con_tot;      // Total number of unconverged estimates
    int n_uncon_seq;    // Number of current streak of converged estimates
    int n_uncon_tot;    // Total number of unconverged estimates
    bool is_converged;  // TRN filter converged by its measure
    bool is_reliable;   // Current offset estimate is reliable enough to be used
    double reinit_time; // Time of most recent reinit (epoch seconds)
    double update_time; // TRN update time (taken in mbtrnpp)
}trnu_pub_future_t;
#endif
#pragma pack(pop)

typedef struct trn_update_s{
    pt_cdata_t *pt_dat;
    pt_cdata_t *mle_dat;
    pt_cdata_t *mse_dat;
    int reinit_count;
    double reinit_tlast;
    int filter_state;
    int success;
    short int is_converged;
    short int is_valid;
    int mb1_cycle;
    int ping_number;
    double mb1_time;
    double update_time;
}trn_update_t;

typedef struct mt_cdata_s{
    //Measurement time (s)
    double time;
	//1: DVL, 2: Multibeam, 3: Single Beam,
    //4: Homer Relative Measurement, 5: Imagenex multibeam, 6: Side-looking DVL
    int dataType;
    unsigned int ping_number;
    double phi, theta, psi;
    double x, y, z;
    // number of beams
    int numMeas;
    double* covariance;
    double* ranges;
    double* crossTrack;
    double* alongTrack;
    double* altitudes;
    double* alphas;
#if defined(__QNX__)
    Boolean* measStatus;
#else
    bool* measStatus;
#endif
    // For use in sensors that vary the number of beams (e.g., MB-system)
    int* beamNums;
}mt_cdata_t;

struct wposet_s;
typedef struct wposet_s wposet_t;

struct wmeast_s;
typedef struct wmeast_s wmeast_t;

typedef struct ct_cdata_s{
    char msg_type;
    int  parameter;
    float vdr;
    wposet_t *pt;
    wmeast_t *mt;
    d_triplet_t xyz_sdev;
    d_triplet_t est_nav_ofs;
    char *mapname;
    char *cfgname;
    char *particlename;
    char *logname;
}ct_cdata_t;

/////////////////////////
// Exports
/////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif // THIS_FILE
