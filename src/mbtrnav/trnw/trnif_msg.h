///
/// @file trnif_msg.h
/// @authors k. headley
/// @date 03 jul 2019

/// TRN netif message API

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
#ifndef TRNIF_MSG_H
#define TRNIF_MSG_H

/////////////////////////
// Includes 
/////////////////////////

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "trn_msg.h"
#include "trnw.h"

/////////////////////////
// Macros
/////////////////////////
#define TRNIF_SYNC_LEN sizeof(trn_sync_t)
#define TRNIF_CHKSUM_LEN sizeof(trn_checksum_t)
#define TRNIF_HDR_LEN sizeof(trnmsg_header_t)
#define TRNIF_SYNC_CMP(b,i) ( (i>=0) && (i<TRNIF_SYNC_LEN) && (b==(((g_trn_sync>>(i*8)))&0xFF)) ? true : false)
//#define TRNIF_PDATA(msg) ((NULL==msg) ? NULL : ((byte *)msg+TRNIF_HDR_LEN))
//#define TRNIF_TPDATA(msg,type) (type *)( (NULL==msg) ? NULL : ((byte *)msg+TRNIF_HDR_LEN) )
#define TRNIF_TPDATA(msg,type) (type *)( TRNIF_PDATA(msg) )

#define TRNIF_MAX_SIZE 2048

/////////////////////////
// Type Definitions
/////////////////////////

typedef unsigned char byte;
typedef uint32_t trn_checksum_t;
typedef uint32_t trn_sync_t;
typedef uint16_t trn_id_t;

typedef enum {
    MSG_EOK=0,
    MSG_ENODATA,
    MSG_ECHK
}trn_msg_err_t;

typedef enum{
    TRNIF_INIT=0,
    TRNIF_MEAS,
    TRNIF_MOTN,
    TRNIF_MLE,
    TRNIF_MMSE,
    TRNIF_SET_MW,
    TRNIF_SET_FR,
    TRNIF_SET_IMA,
    TRNIF_SET_VDR,
    TRNIF_SET_MIM,
    TRNIF_FILT_GRD,
    TRNIF_ACK,
    TRNIF_NACK,
    TRNIF_BYE,
    TRNIF_OUT_MEAS,
    TRNIF_LAST_MEAS,
    TRNIF_IS_CONV,
    TRNIF_FILT_TYPE,
    TRNIF_FILT_STATE,
    TRNIF_FILT_REINITS,
    TRNIF_FILT_REINIT,
    TRNIF_PING,
    TRNIF_MSG_ID_COUNT
}trnmsg_id_t;
#pragma pack(push,1)

typedef struct trnmsg_header_s{
    trn_sync_t sync;
    trn_id_t msg_id;
    trn_checksum_t checksum;
    // length of data only
    uint32_t data_len;
}trnmsg_header_t;

typedef struct trnmsg_s{
    trnmsg_header_t hdr;
    // data...
}trnmsg_t;

typedef struct trn_type_s{
    int32_t parameter;
}trn_type_t;

typedef struct trn_float_s{
    int32_t parameter;
    float data;
}trn_float_t;

typedef struct trn_init_s{
    // type (e.g. filter type)
    int32_t parameter;
    uint16_t map_ofs;
    uint16_t cfg_ofs;
    uint16_t particles_ofs;
    uint16_t logdir_ofs;
    // (null terminated strings)
}trn_init_t;

typedef struct trn_meas_s{
    int32_t parameter;
    // serialized wmeast_t (variable len)
}trn_meas_t;


// MOTN, MLE, MMSE
// contain single pt_cdata_t (pose)
// (so may use this struct or
// just overlay buffer w/ pt_cdata_t*)
typedef struct trn_pose_s{
    // pose (fixed len)
}trn_pose_t;
#pragma pack(pop)

typedef trn_type_t trn_ack_t;
typedef trn_type_t trn_nack_t;
typedef trn_type_t trn_bye_t;
typedef trn_type_t trn_modwt_t;
typedef trn_type_t trn_setfr_t;
typedef trn_type_t trn_setima_t;
typedef trn_float_t trn_setvdr_t;
typedef trn_type_t trn_setmim_t;
typedef trn_type_t trn_filtgrd_t;
typedef trn_type_t trn_lastmeas_t;
typedef trn_type_t trn_isconv_t;
typedef trn_type_t trn_filttype_t;
typedef trn_type_t trn_filtstate_t;
typedef trn_type_t trn_reinits_t;
typedef pt_cdata_t trn_motn_t;
typedef pt_cdata_t trn_mle_t;
typedef pt_cdata_t trn_mmse_t;


/////////////////////////
// Exports
/////////////////////////

#ifdef __cplusplus
extern "C" {
#endif
    extern trn_sync_t g_trn_sync;

    // message API
    trnmsg_t *trnmsg_new(trnmsg_id_t id, byte *data, uint32_t data_len);
    trnmsg_t *trnmsg_new_type_msg(trnmsg_id_t id, int parameter);
    trnmsg_t *trnmsg_new_vdr_msg(trnmsg_id_t id, int parameter, float vdr);
    trnmsg_t *trnmsg_new_pose_msg(trnmsg_id_t id, wposet_t *pt);
    trnmsg_t *trnmsg_new_meas_msg(trnmsg_id_t id, int parameter, wmeast_t *mt);
    trnmsg_t *trnmsg_new_init_msg(trnmsg_id_t id, int parameter, char *map, char *cfg, char *particles, char *logdir);

    void trnmsg_destroy(trnmsg_t **pself);
    void trnmsg_show(trnmsg_t *self, bool verbose, int indent);
    int32_t trnmsg_len(trnmsg_t *self);
    trnmsg_t *trnmsg_realloc(trnmsg_t **dest, trnmsg_id_t id, byte *data, uint32_t data_len);
    int32_t trnmsg_deserialize(trnmsg_t **dest, byte *src, uint32_t len);
    int32_t trnmsg_serialize(byte **dest, uint32_t len);
    const char *trnmsg_idstr(int id);

    void trnmsg_hex_show(byte *data, uint32_t len, uint16_t cols, bool show_offsets, uint16_t indent);
    uint32_t trnmsg_checksum(byte *pdata, uint32_t len);

    byte *TRNIF_PDATA(void *msg);
    
#ifdef __cplusplus
}
#endif

#endif // THIS_FILE
