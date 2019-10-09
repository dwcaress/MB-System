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
#include "trn_msg.h"
#include "mframe.h"
#include "msocket.h"
#include "medebug.h"

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
typedef enum {
    TCLI_SUB=0,
    TCLI_DATA,
    TCLI_CONF
} trncli_type_id;


typedef struct trncli_s{
    msock_connection_t *trn;
    trncli_type_id type;
    long int utm_zone;
    wmeast_t *measurement;
}trncli_t;


/////////////////////////
// Exports
/////////////////////////

#ifdef __cplusplus
extern "C" {
#endif
    trncli_t *trncli_new(trncli_type_id type, long int utm_zone);
    void trncli_destroy(trncli_t **pself);
    int trncli_connect(trncli_t *self, char *host, int port);
    int trncli_disconnect(trncli_t *self);
    int trncli_set_utm(trncli_t *self, long int utm_zone);
    int32_t trncli_init_server(trncli_t *self, trn_config_t *cfg);
    int32_t trncli_update_measurement(trncli_t *self, wmeast_t *meas);
    int32_t trncli_update_motion(trncli_t *self, wposet_t *pose);
    int32_t trncli_estimate_pose(trncli_t *self, wposet_t **pose, char msg_type);
    bool trncli_last_meas_valid(trncli_t *self);
    int32_t trncli_reinit_filter(trncli_t *self);
    int trncli_mb1_to_meas(wmeast_t **dest, mb1_t *src, long int utmZone);
    int trncli_motion_to_pose(wposet_t **dest, pt_cdata_t *src);
    
    int trncli_send_update(trncli_t *self, mb1_t *src, wposet_t **pt_out, wmeast_t **mt_out);
    int trncli_get_bias_estimates(trncli_t *self, wposet_t *pt, pt_cdata_t **pt_out, pt_cdata_t **mle_out, pt_cdata_t **mse_out);


#ifdef __cplusplus
}
#endif

#endif // THIS_FILE
