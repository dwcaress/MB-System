///
/// @file trnw.h
/// @authors k. headley
/// @date 2019-06-21

/// C wrappers for TerrainNav API
// TerrainNav methods called by trn_server
//    initialized()
//    estimatePose
//    getFilterState
//    getFilterType
//    getNumReinits
//    isConverged
//    lastMeasSuccessful
//    measUpdate
//    motionUpdate
//    outstandingMeas
//    reinitFilter
//    releaseMap
//    setFilterReinit
//    setInterpMeasAttitude()
//    setMapInterpMethod
//    setModifiedWeighting
//    setVehicleDriftRate
//    useLowGradeFilter
//    useHighGradeFilter

/// @sa doxygen-examples.c for more examples of Doxygen markup


/////////////////////////
// Terms of use
/////////////////////////
/*
 Copyright Information

 Copyright 2002-2019 MBARI
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
#ifndef TRNW_H
#define TRNW_H

/////////////////////////
// Includes
/////////////////////////
#include <stdint.h>
#include <stdbool.h>

#include "mb1_msg.h"
#include "trn_msg.h"

/////////////////////////
// Macros
/////////////////////////
#define TRNW_MSG_SIZE TRN_MSG_SIZE
#define TRNW_WMEAST_SERIAL_LEN(nmeas)  ( (2+1*nmeas)*sizeof(int) + (7+6*nmeas)*sizeof(double) + (0+1*nmeas)*sizeof(bool) + (0+1*nmeas)*sizeof(unsigned int) )

#define TRN_MAX_NCOV_DFL 49.
#define TRN_MAX_ECOV_DFL 49.
#define TRN_MAX_NERR_DFL 50.
#define TRN_MAX_EERR_DFL 50.

struct wtnav_s;
typedef struct wtnav_s wtnav_t;

struct wposet_s;
typedef struct wposet_s wposet_t;

struct wmeast_s;
typedef struct wmeast_s wmeast_t;

struct wcommst_s;
typedef struct wcommst_s wcommst_t;

typedef enum {
    TRNW_OSOUT=0x1,
    TRNW_OSERR=0x2,
    TRNW_OLOG=0x4,
    TRNW_ODEBUG=0x8,
    TRNW_OSOCKET=0x10,
}trnw_oflags_t;


typedef struct trn_config_s{
    char *trn_host;
    int  trn_port;
    char *map_file;
    char *cfg_file;
    char *particles_file;
    char *log_dir;
    int  sensor_type;
    int  filter_type;
    int  map_type;
    long int utm_zone;
    int mod_weight;
    int filter_reinit;
    int filter_grade;
    trnw_oflags_t oflags;
    double max_northing_cov;
    double max_northing_err;
    double max_easting_cov;
    double max_easting_err;
}trn_config_t;

/////////////////////////
// Exports
/////////////////////////
#ifdef __cplusplus
extern "C" {
#endif
    // TRN config API
    trn_config_t *trncfg_dnew();
    trn_config_t *trncfg_new(char *host, int port,
                             long int utm_zone,
                             int map_type,
                             int sensor_type,
                             int filter_type,
                             int filter_grade,
                             int filter_reinit,
                             int mod_weight,
                             char *map_file,
                             char *cfg_file,
                             char *particles_file,
                             char *logdir,
                             trnw_oflags_t oflags,
                             double max_northing_cov,
                             double max_northing_err,
                             double max_easting_cov,
                             double max_easting_err
                             );

    void trncfg_destroy(trn_config_t **pself);
    void trncfg_show(trn_config_t *obj, bool verbose, int indent);

    // TerrainNav API
    wtnav_t *wtnav_dnew();
    wtnav_t *wtnav_new(trn_config_t *cfg);
    void wtnav_destroy(wtnav_t *self);

    void wtnav_estimate_pose(wtnav_t *self, wposet_t *estimate, const int type);
    void wtnav_meas_update(wtnav_t *self, wmeast_t *incomingMeas, const int type);
    void wtnav_motion_update(wtnav_t *self, wposet_t *incomingNav);
    bool wtnav_last_meas_successful(wtnav_t *self);
    bool wtnav_outstanding_meas(wtnav_t *self);
    bool wtnav_initialized(wtnav_t *self);
    bool wtnav_is_converged(wtnav_t *self);
    void wtnav_set_est_nav_offset(wtnav_t *self, double ofs_x, double ofs_y, double ofs_z);
    d_triplet_t *wtnav_get_est_nav_offset(wtnav_t *self, d_triplet_t *dest);
    void wtnav_set_init_stddev_xyz(wtnav_t *self, double sdev_x, double sdev_y, double sdev_z);
    d_triplet_t *wtnav_get_init_stddev_xyz(wtnav_t *self, d_triplet_t *dest);
    void wtnav_reinit_filter(wtnav_t *self, bool lowInfoTransition);
    void wtnav_reinit_filter_offset(wtnav_t *self, bool lowInfoTransition,
                             double ofs_x, double ofs_y, double ofs_z);
    void wtnav_reinit_filter_box(wtnav_t *self, bool lowInfoTransition,
                             double ofs_x, double ofs_y, double ofs_z,
                             double sdev_x, double sdev_y, double sdev_z);
    int  wtnav_get_filter_type(wtnav_t *self);
    int  wtnav_get_filter_state(wtnav_t *self);
    void wtnav_use_highgrade_filter(wtnav_t *self);
    void wtnav_use_lowgrade_filter(wtnav_t *self);
    int  wtnav_get_num_reinits(wtnav_t *self);
    void wtnav_set_filter_reinit(wtnav_t *self, const bool allow);
    void wtnav_set_interp_meas_attitude(wtnav_t *self, const bool set);
    void wtnav_set_map_interp_method(wtnav_t *self, const int type);
    void wtnav_set_vehicle_drift_rate(wtnav_t *self, const double driftRate);
    void wtnav_set_modified_weighting(wtnav_t *self, const int use);
    void wtnav_release_map(wtnav_t *self);
    void *wtnav_obj_addr(wtnav_t *self);

    // comms (msg) API
    wcommst_t *wcommst_dnew();
    void wcommst_destroy(wcommst_t *self);
    void wcommst_show(wcommst_t *self, bool verbose, int indent);

    ct_cdata_t *wcommst_cdata_new();
    void wcommst_cdata_destroy(ct_cdata_t **pself);
    uint32_t  wcommst_cdata_serialize(char *dest, ct_cdata_t *src, int len);
    int  wcommst_cdata_unserialize(ct_cdata_t **dest, char *src);
    int wcommst_get_pt(wposet_t **dest, wcommst_t *src);
    int wcommst_set_pt(wcommst_t *self, wposet_t *wpt);
    int wcommst_get_mt(wmeast_t **dest, wcommst_t *src);
    int wcommst_set_mt(wcommst_t *self, wmeast_t *wmt);
    int wcommst_get_parameter(wcommst_t *self);
    float wcommst_get_vdr(wcommst_t *self);
    d_triplet_t *wcommst_get_xyz_sdev(wcommst_t *self, d_triplet_t *dest);
    d_triplet_t *wcommst_get_est_nav_offset(wcommst_t *self, d_triplet_t *dest);
    char wcommst_get_msg_type(wcommst_t *self);
    int  wcommst_serialize(char **dest, wcommst_t *src, int len);
    int  wcommst_unserialize(wcommst_t **dest, char *src, int len);

    void commst_meas_update(wtnav_t *self, wcommst_t *msg);
    void commst_motion_update(wtnav_t *self, wcommst_t *msg);
    void commst_estimate_pose(wtnav_t *self, wcommst_t *msg, const int type);
    void commst_initialize(wtnav_t *self, wcommst_t *msg);

    // pose API
    wposet_t *wposet_dnew();
    void wposet_destroy(wposet_t *self);
    void wposet_show(wposet_t *self, bool verbose, int indent);
    int  wposet_cdata_to_pose(wposet_t **dest, pt_cdata_t *src);
    int  wposet_pose_to_cdata(pt_cdata_t **dest, wposet_t *src);
    int  wposet_mb1_to_pose(wposet_t **dest, mb1_t *src, long int utmZone);
    int  wposet_msg_to_pose(wposet_t **dest, char *src);
    int  wposet_serialize(char **dest, wposet_t *src, int len);
    int  wposet_unserialize(wposet_t **dest, char *src, int len);

    // measurement API
    wmeast_t *wmeast_dnew();
    wmeast_t *wmeast_new(unsigned int size);
    void wmeast_destroy(wmeast_t *self);
    void wmeast_show(wmeast_t *self, bool verbose, int indent);
    int  wmeast_cdata_to_meas(wmeast_t **dest, mt_cdata_t *src);
    int  wmeast_meas_to_cdata(mt_cdata_t **dest, wmeast_t *src);
    int  wmeast_mb1_to_meas(wmeast_t **dest, mb1_t *src, long int utmZone);
    int  wmeast_msg_to_meas(wmeast_t **dest, char *src);
    int  wmeast_serialize(char **dest, wmeast_t *src, int len);
    int  wmeast_unserialize(wmeast_t **dest, char *src, int len);
    int  wmeast_get_nmeas(wmeast_t *self);

    // message API
    int32_t trnw_meas_msg(char **dest, wmeast_t *src, int msg_type, int param);
    int32_t trnw_pose_msg(char **dest, wposet_t *src, char msg_type);
    int32_t trnw_init_msg(char **dest, trn_config_t *cfg);
    int32_t trnw_ptype_msg(char **dest, char msg_type, int param);
    int32_t trnw_type_msg(char **dest, char type);
    int32_t trnw_ack_msg(char **dest);
    int32_t trnw_nack_msg(char **dest);
    int32_t trnw_triplet_msg(char **dest, char msg_type, d_triplet_t *src);
    int32_t trnw_reinit_offset_msg(char **dest, char msg_type, bool lowInfoTransition,
                                double offset_x, double offset_y, double offset_z );
    int32_t trnw_reinit_box_msg(char **dest, char msg_type, bool lowInfoTransition,
                                double offset_x, double offset_y, double offset_z,
                                double sdev_x, double sdev_y, double sdev_z );
    void trnw_msg_show(char *msg, bool verbose, int indent);

    // utils
    int trnw_utm_to_geo(double northing, double easting, long utmZone, double *lat_deg, double *lon_deg);
    int trnw_geo_to_utm(double lat_deg, double lon_deg, long int utmZone, double *northing, double *easting);


#ifdef __cplusplus
}
#endif

#endif // THIS_FILE
