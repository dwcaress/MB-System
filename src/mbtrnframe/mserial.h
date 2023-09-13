///
/// @file mserial.h
/// @authors k. headley
/// @date 11 aug 2017

/// mframe serial port IO wrappers

/// @sa doxygen-examples.c for more examples of Doxygen markup


/////////////////////////
// Terms of use
/////////////////////////
/*
 Copyright Information
  
 Copyright 2002-2017 MBARI
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
#ifndef MSERIAL_H
#define MSERIAL_H

/////////////////////////
// Includes 
/////////////////////////
#include "mframe.h"

/////////////////////////
// Macros
/////////////////////////
/// @def SIN_ID
/// @brief stdin device handle
#define SIN_ID  0
/// @def SOUT_ID
/// @brief stdout device handle
#define SOUT_ID 1
/// @def SERR_ID
/// @brief sterr device handle
#define SERR_ID 2

#define MSER_PAR2STR(n) (n>=0 && n<MSER_MAX_PAR ? MSER_PAR_STR[n]:NULL)
#define MSER_FLOW2STR(n) (n>=0 && n<MSER_MAX_FLOW ? MSER_FLOW_STR[n]:NULL)
#define MSER_CS2U32(n) (n>=0 && n<MSER_MAX_CS ? MSER_CSIZE_U32[n]:0xFFFFFFFF)


/////////////////////////
// Type Definitions
/////////////////////////
/// @typedef struct mser_device_s mser_device_t
/// @brief serial device configuration type
typedef struct mser_device_s mser_device_t;
/// @typedef struct mser_serial_s mser_serial_t
/// @brief serial device type
typedef struct mser_serial_s mser_serial_t;
/// @typedef int32_t mser_id_t;
/// @brief serial device handle type
typedef int32_t mser_id_t;

/// @enum mser_parity_t
/// @brief parity configuration enumeration
typedef enum {MSER_PAR_N, MSER_PAR_E, MSER_PAR_O,MSER_MAX_PAR}mser_parity_t;
/// @enum mser_stopb_t
/// @brief stop bit configuration enumeration
typedef enum {MSER_STOPB_0=0, MSER_STOPB_1, MSER_STOPB_2,MSER_MAX_STOPB}mser_stopb_t;
/// @enum mser_csize_t
/// @brief character size (data bits) configuration enumeration
typedef enum {MSER_CS_5, MSER_CS_6, MSER_CS_7, MSER_CS_8,MSER_MAX_CS}mser_csize_t;
/// @enum mser_flow_t
/// @brief flow control configuration enumeration
typedef enum {MSER_FLOW_N, MSER_FLOW_H, MSER_FLOW_X,MSER_MAX_FLOW}mser_flow_t;
/// @enum mser_direction_t
/// @brief direction enumeration
typedef enum {MSER_TX, MSER_RX, MSER_RXTX}mser_direction_t;

typedef struct mser_term_s{
    char *path;
    mser_id_t hnd;
    int speed;
    mser_parity_t par;
    mser_csize_t cs;
    mser_flow_t flow;
    mser_stopb_t stopb;
    unsigned vm;
    unsigned vt;
}mser_term_t;
/////////////////////////
// Exports
/////////////////////////

#ifdef __cplusplus
extern "C" {
#endif
    
    extern const char *MSER_PAR_STR[MSER_MAX_PAR];
    extern const char *MSER_FLOW_STR[MSER_MAX_FLOW];
    extern uint32_t MSER_CSIZE_U32[MSER_MAX_CS];
    void mser_init();

    mser_id_t mser_lookup_id(char *path);

    void mser_serial_show(mser_id_t id, bool verbose, uint16_t indent);

    mser_id_t mser_open(const char *path, int speed, mser_parity_t parity,
                        mser_csize_t csize, mser_stopb_t stopb,
                        mser_flow_t flow, unsigned min, unsigned time_dsec);
   
    int mser_close(mser_id_t id);
    
    int mser_drain(mser_id_t id);

    int mser_flush(mser_id_t id, mser_direction_t dir);

    int mser_send_break(mser_id_t id, int msec);

    int mser_set_blocking(mser_id_t id, bool enable);
    int mser_set_canonical(mser_id_t id, bool enable);
    int mser_set_echo(mser_id_t id, bool enable);

    void mser_release();

    mser_term_t *mser_term_new(char *path, int speed, mser_parity_t parity,
                               mser_csize_t csize, mser_stopb_t stopb,
                               mser_flow_t flow, unsigned vm, unsigned vt);
    mser_term_t *mser_parse_term(mser_term_t **dest, char *path, char *term_str);
    void mser_term_destroy(mser_term_t **pself);
    mser_id_t mser_term_open(mser_term_t *term);
    void mser_term_show(mser_term_t *self, bool verbose, uint16_t indent);

    int mser_save_term(mser_id_t id);
    
    int mser_restore_term(mser_id_t id);
    
    int64_t mser_read(mser_id_t id, byte *buf, uint32_t len);
    
    int64_t mser_read_str(mser_id_t id, char *buf, uint32_t len);
    
    int64_t mser_read_del(mser_id_t id, byte *buf, uint32_t len, const char *del, uint32_t dlen);
    
    int64_t mser_write(mser_id_t id, byte *buf, uint32_t len);
    
    int64_t mser_write_str(mser_id_t id, char *buf);
    
    int64_t mser_sync_str(mser_id_t id, const char *sync, uint32_t max_len);
    
    int64_t mser_sync_n(mser_id_t id, uint32_t n);

    int mser_test();

#ifdef __cplusplus
}
#endif

#endif // THIS_FILE
