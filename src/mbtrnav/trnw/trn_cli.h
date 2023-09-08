///
/// @file trn_cli.h
/// @authors k. headley
/// @date 09 jul 2019

/// TRN net client API

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
#ifndef TRN_CLI_H
#define TRN_CLI_H

/////////////////////////
// Includes 
/////////////////////////

#include "trnw.h"
#include "mb1_msg.h"
#include "msocket.h"

/////////////////////////
// Macros
/////////////////////////
#define TRNCLI_NBEAMS 25
#define UTM_MONTEREY_BAY 10L
#define UTM_AXIAL        12L

#define TRNCLI_UTM_DFL UTM_MONTEREY_BAY

/////////////////////////
// Type Definitions
/////////////////////////

typedef struct trncli_s{
    msock_connection_t *trn;
    long int utm_zone;
    wmeast_t *measurement;
}trncli_t;

/////////////////////////
// Exports
/////////////////////////

#ifdef __cplusplus
extern "C" {
#endif
    trncli_t *trncli_new(long int utm_zone);
    void trncli_destroy(trncli_t **pself);
    int trncli_connect(trncli_t *self, char *host, int port);
    int trncli_disconnect(trncli_t *self);

    /// High-level API
    // translate MB1 to meas/pose, update motion. update meas
    int trncli_send_update(trncli_t *self, mb1_t *src, wposet_t **pt_out, wmeast_t **mt_out);

    // mle estimate, mmse estimates, if last meas valid, fill _out structs
    int trncli_get_bias_estimates(trncli_t *self, wposet_t *pt, pt_cdata_t **pt_out, pt_cdata_t **mle_out, pt_cdata_t **mse_out);

    // return commsT parameter value for specified message
    // for boolean messages, 1: true 0: false
    int trncli_ptype_get(trncli_t *self, int msg_type);

    // return 0 if successful (ACK received), -1 otherwise
    int trncli_ptype_set(trncli_t *self, int msg_type, int param);

    // Conversion helper functions
    int trncli_mb1_to_meas(wmeast_t **dest, mb1_t *src, long int utmZone);
    int trncli_cdata_to_pose(wposet_t **dest, pt_cdata_t *src);
    int trncli_cdata_to_meas(wmeast_t **dest, mt_cdata_t *src);

    /// Low-level API
    // meas update
    int32_t trncli_update_measurement(trncli_t *self, wmeast_t *meas);
    // motion update
    int32_t trncli_update_motion(trncli_t *self, wposet_t *pose);
    // estimate pose (mle estimate, mmse estimate)
    int32_t trncli_estimate_pose(trncli_t *self, wposet_t **pose, char msg_type);

    /// Control API
    // init TRN
    int32_t trncli_init_trn(trncli_t *self, trn_config_t *cfg);
    // filter reinit
    int trncli_reinit_filter(trncli_t *self);
    // filter reinit w/ offset
    int trncli_reinit_filter_offset(trncli_t *self, bool lowInfoTransition,
                             double offset_x, double offset_y, double offset_z);
    // filter reinit w/ offset, variance bound inits
    int trncli_reinit_filter_box(trncli_t *self, bool lowInfoTransition,
                        double offset_x, double offset_y, double offset_z,
                        double sdev_x, double sdev_y, double sdev_z);
    // get filter type
    int trncli_get_filter_type(trncli_t *self);
    // get filter state
    int trncli_get_filter_state(trncli_t *self);
    // set filter reinit
    int trncli_set_filter_reinit(trncli_t *self, int value);
    // set modified weighting
    int trncli_set_modified_weighting(trncli_t *self, int value);
    // set interp meas alt (IMA)
    int trncli_set_ima(trncli_t *self, int value);
    // set map interp method (MIM)
    int trncli_set_mim(trncli_t *self, int value);
    // set vehicle drift rate (VDR)
    int trncli_set_vdr(trncli_t *self, int value);
    // set filter gradient
    int trncli_set_filter_gradient(trncli_t *self, int value);
    // set UTM zone
    int trncli_set_utm(trncli_t *self, long int utm_zone);
    // get init xyz (reinit variance bounds)
    int trncli_get_init_stddev_xyz(trncli_t *self, d_triplet_t *dest);
    // set init xyz (reinit variance bounds)
    int trncli_set_init_stddev_xyz(trncli_t *self, d_triplet_t *src);
    // get est_nav_offset (reinit offset)
    int trncli_get_est_nav_ofs(trncli_t *self, d_triplet_t *dest);
    // set est_nav_offset (reinit offset)
    int trncli_set_est_nav_ofs(trncli_t *self, d_triplet_t *src);

	/// Status API

    // last meas successful
    bool trncli_last_meas_succesful(trncli_t *self);
    // num reinits
    int trncli_reinit_count(trncli_t *self);
    // outstanding meas
    bool trncli_outstanding_meas(trncli_t *self);
    // is converged
    bool trncli_is_converged(trncli_t *self);
    // is intialized
    bool trncli_is_intialized(trncli_t *self);
    // ping
    bool trncli_ping(trncli_t *self);
    // ack
    int32_t trncli_ack_server(trncli_t *self);

#ifdef __cplusplus
}
#endif

#endif // THIS_FILE
