///
/// @file mbtrn.h
/// @authors k. headley
/// @date 06 nov 2012

/// MBSystem Terrain Relative Navigation library
/// contains mbtrn_reader, a component that connects
/// to reson 7k center and buffers data for use by MBSystem

/// @sa doxygen-examples.c for more examples of Doxygen markup


/////////////////////////
// Terms of use
/////////////////////////
/*
 Copyright Information
 
 Copyright 2000-2018 MBARI
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

// include guard
#ifndef MBTRN_H
/// @def MBTRN_H
/// @brief TBD
#define MBTRN_H

/////////////////////////
// Includes
/////////////////////////

#include <stdint.h>
#include <stdbool.h>
#include "iowrap.h"
#include "cbuffer.h"
#include "r7kc.h"

/////////////////////////
// Type Definitions
/////////////////////////

/// @typedef enum mbtrn_cstate mbtrn_cstate
/// @brief connection state enum
typedef enum {CS_NEW,CS_INITIALIZED,CS_CONNECTED} mbtrn_cstate;
/// @typedef enum mbtrn_state_t mbtrn_state_t
/// @brief reader state enum
typedef enum {MBS_NEW,MBS_INITIALIZED,MBS_CONNECTED,MBS_SUBSCRIBED} mbtrn_state_t;
/// @typedef enum mbtrn_flags_t mbtrn_flags_t
/// @brief reader behavior flags
typedef enum{MBR_ALLOW_PARTIAL=0x01, MBR_FORCE=0x2, MBR_IFLUSH=0x4, MBR_OFLUSH=0x8, MBR_FLUSH=0x10} mbtrn_flags_t;

/// @typedef struct mbtrn_connection_s mbtrn_connection_t
/// @brief connection structure
typedef struct mbtrn_connection_s
{
    /// @var mbtrn_connection_s::type
    /// @brief connection type (socket, file)
    mbtrn_ctype type;
    /// @var mbtrn_connection_s::state
    /// @brief connection state
    mbtrn_cstate state;
    /// @var mbtrn_connection_s::sock_if
    /// @brief socket interface
    iow_socket_t *sock_if;
    /// @var mbtrn_connection_s::file_if
    /// @brief file interface
    iow_file_t *file_if;
    /// @var mbtrn_connection_s::auto_free
    /// @brief auto free resources when connection destroyed
    bool auto_free;
    /// @var mbtrn_connection_s::capacity
    /// @brief connection buffer size
    uint32_t capacity;
    /// @var mbtrn_connection_s::wp
    /// @brief write pointer
    byte *wp;
    /// @var mbtrn_connection_s::rp
    /// @brief read pointer
    byte *rp;
    /// @var mbtrn_connection_s::buf
    /// @brief buffer
    byte *buf;
}mbtrn_connection_t;

/// @typedef struct mbtrn_reader_s mbtrn_reader_t
/// @brief reson 7k center reader component
typedef struct mbtrn_reader_s
{
    /// @var mbtrn_reader_s::s
    /// @brief socket interface
    iow_socket_t *s;
    /// @var mbtrn_reader_s::src
    /// @brief connection configuration
    mbtrn_connection_t *src;
    /// @var mbtrn_reader_s::fc
    /// @brief Data Record Frame container component
    r7k_drf_container_t *fc;
    /// @var mbtrn_reader_s::state
    /// @brief reader state
    int state;
    /// @var mbtrn_reader_s::sub_count
    /// @brief reson 7k center subscription count
    uint32_t sub_count;
    /// @var mbtrn_reader_s::sub_list
    /// @brief reson 7k center subscription list
    uint32_t *sub_list;
}mbtrn_reader_t;

/////////////////////////
// Macros
/////////////////////////

/// @def MAX_FRAME_BYTES_7K
/// @brief max DRF bytes
#define MAX_FRAME_BYTES_7K        60000
/// @def IP_PORT_7K
/// @brief reson 7k center IP port
#define IP_PORT_7K                 7000
/// @def MBTRN_POLL_TIMEOUT_MSEC
/// @brief reader poll timeout default
#define MBTRN_POLL_TIMEOUT_MSEC    5000
/// @def MBTRN_FLUSH_RETRIES
/// @brief reader poll retries default
#define MBTRN_FLUSH_RETRIES          10

// for the TRN application, use
// empirical numbers for
// TODO: make this configurable

/// @def MBTRN_TRN_MESSAGE_SUBS
/// @brief number of reson 7k center subscription messages
#define MBTRN_TRN_MESSAGE_SUBS       12
/// @def MBTRN_TRN_PING_MSEC
/// @brief ping interval (msec)
#define MBTRN_TRN_PING_MSEC         350
/// @def MBTRN_TRN_PING_BYTES
/// @brief max bytes per ping
#define MBTRN_TRN_PING_BYTES     250000
/// @def MBTRN_TRN_REC_HINT
/// @brief TBD
#define MBTRN_TRN_REC_HINT          128
/// @def MBTRN_PING_INTERVAL_USEC
/// @brief ping interval (usec)
#define MBTRN_PING_INTERVAL_USEC 350000
/// @def MBTRN_PING_INTERVAL_MSEC
/// @brief ping interval (msec)
#define MBTRN_PING_INTERVAL_MSEC    350

/// @def R7K_PING_BUF_BYTES
/// @brief TBD
#define R7K_PING_BUF_BYTES R7K_TRN_PING_BYTES

/////////////////////////
// Exports
/////////////////////////

// mbtrn utility API

// mbtrn reader API

mbtrn_reader_t *mbtrn_reader_create(const char *host, int port, uint32_t capacity, uint32_t *slist,  uint32_t slist_len);
void mbtrn_reader_destroy(mbtrn_reader_t **pself);

int mbtrn_reader_connect(mbtrn_reader_t *self);
iow_socket_t *mbtrn_reader_sockif(mbtrn_reader_t *self);
void mbtrn_reader_show(mbtrn_reader_t *self, bool verbose, uint16_t indent);
const char *mbtrn_strstate(mbtrn_state_t state);

int64_t mbtrn_reader_poll(mbtrn_reader_t *self, byte *dest, uint32_t len, uint32_t tmout_ms);
int64_t mbtrn_reader_xread(mbtrn_reader_t *self, byte *dest, uint32_t len, uint32_t tmout_ms, mbtrn_flags_t flags);
int64_t mbtrn_reader_parse(mbtrn_reader_t *self, byte *src, uint32_t len, r7k_drf_container_t *dest);

int64_t mbtrn_reader_read(mbtrn_reader_t *self, byte *dest, uint32_t len);
int64_t mbtrn_reader_seek(mbtrn_reader_t *self, uint32_t ofs);
int64_t mbtrn_reader_tell(mbtrn_reader_t *self);
void mbtrn_reader_flush(mbtrn_reader_t *self, uint32_t len, int32_t retries, uint32_t tmout_ms);
void mbtrn_reader_purge(mbtrn_reader_t *self);
uint32_t mbtrn_reader_frames(mbtrn_reader_t *self);
r7k_drf_t *mbtrn_reader_enumerate(mbtrn_reader_t *self);
r7k_drf_t *mbtrn_reader_next(mbtrn_reader_t *self);

// mbtrn_connection API

mbtrn_connection_t *mbtrn_scon_new(iow_socket_t *s);
mbtrn_connection_t *mbtrn_fcon_new(iow_file_t *f);
void mbtrn_connection_destroy(mbtrn_connection_t **pself);
int64_t mbtrn_con_read(mbtrn_connection_t *self, byte *dest, uint32_t len, int *error);

// functions for peer comparisons
// used in mbtrnpreprocess

bool mbtrn_peer_cmp(void *a, void *b);
bool mbtrn_peer_vcmp(void *item, void *value);

// include guard
#endif
