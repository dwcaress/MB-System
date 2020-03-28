///
/// @file r7k-reader.h
/// @authors k. headley
/// @date 06 nov 2012

/// Reson 7KCenter reader API
/// contains r7kr_reader_t, a component that
/// reads reson 7k center multibeam data for use by MBSystem

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
#ifndef R7K_READER_H
/// @def R7K_READER_H
/// @brief TBD
#define R7K_READER_H

/////////////////////////
// Includes
/////////////////////////

#include "mframe.h"
#include "mlog.h"
#include "msocket.h"
#include "mfile.h"
#include "mconfig.h"
#include "mstats.h"
#include "r7kc.h"

/////////////////////////
// Type Definitions
/////////////////////////
/// @typedef enum r7kr_event_id r7kr_stevent_id
/// @brief diagnostic event (counter) IDs
typedef enum{
    R7KR_EV_FRAME_VALID=0,
    R7KR_EV_FRAME_INVALID,
    R7KR_EV_NF_VALID,
    R7KR_EV_DRF_VALID,
    R7KR_EV_NF_INVALID,
    R7KR_EV_DRF_INVALID,
    R7KR_EV_DRF_RESYNC,
    R7KR_EV_NF_RESYNC,
    R7KR_EV_NF_SHORT_READ,
    R7KR_EV_DRF_SHORT_READ,
    R7KR_EV_EDRFPROTO,
    R7KR_EV_ENFTOTALREC,
    R7KR_EV_ENFPACKETSZ,
    R7KR_EV_ENFOFFSET,
    R7KR_EV_ENFVER,
    R7KR_EV_ENFREAD,
    R7KR_EV_ESOCK,
    R7KR_EV_EDRFCHK,
    R7KR_EV_EDRFTIME,
    R7KR_EV_EDRFSIZE,
    R7KR_EV_EDRFSYNC,
    R7KR_EV_EDRFREAD,
    R7KR_EV_EFCWR,
    R7KR_EV_FC_READ,
    R7KR_EV_FC_REFILL,
    R7KR_EV_COUNT
}r7kr_event_id;

/// @typedef enum r7kr_status_id r7kr_status_id
/// @brief diagnostic status (counter) IDs
typedef enum{
    R7KR_STA_FRAME_VAL_BYTES=0,
    R7KR_STA_NF_VAL_BYTES,
    R7KR_STA_DRF_VAL_BYTES,
    R7KR_STA_NF_INVAL_BYTES,
    R7KR_STA_DRF_INVAL_BYTES,
    R7KR_STA_SUB_FRAMES,
    R7KR_STA_COUNT
}r7kr_status_id;

/// @typedef enum r7kr_metric_id r7kr_metric_id
/// @brief diagnostic status (floating point) measurement IDs
typedef enum{
    //    R7KR_MET_R7KRN_REFILL_XT=0
    R7KR_MET_7KFRAME_SKEW=0,
    R7KR_MET_COUNT
}r7kr_metric_id;

/// @typedef enum r7kr_cstate r7kr_cstate
/// @brief connection state enum
typedef enum {CS_NEW,CS_INITIALIZED,CS_CONNECTED} r7kr_cstate;
/// @typedef enum r7kr_ctype r7kr_ctype
/// @brief connection endpoint types
typedef enum {CT_NULL,CT_STDIN,CT_STDOUT,CT_STDERR,CT_FILE,CT_SOCKET} r7kr_ctype;
/// @typedef enum r7kr_state_t r7kr_state_t
/// @brief reader state enum
typedef enum {R7KR_NEW,R7KR_INITIALIZED,R7KR_CONNECTED,R7KR_SUBSCRIBED} r7kr_state_t;
/// @typedef enum r7kr_flags_t r7kr_flags_t
/// @brief reader behavior flags
typedef enum{
    R7KR_ALLOW_PARTIAL=0x01,
    R7KR_FORCE=0x2,
    R7KR_IFLUSH=0x4,
    R7KR_OFLUSH=0x8,
    R7KR_FLUSH=0x10,
    R7KR_NOFLUSH=0x20,
    R7KR_BLOCK=0x40,
    R7KR_NONBLOCK=0x80,
    R7KR_NET_STREAM=0x100,
    R7KR_NF_STREAM=0x200,
    R7KR_DRF_STREAM=0x400,
    R7KR_RESYNC_NF=0x800,
    R7KR_RESYNC_DRF=0x1000
} r7kr_flags_t;

/// @typedef enum r7kr_parse_state_t r7kr_parse_state_t
/// @brief 7k frame parsing states
typedef enum{
    R7KR_STATE_START=0,
    R7KR_STATE_READ_ERR,
    R7KR_STATE_READ_OK,
    R7KR_STATE_DRF_INVALID,
    R7KR_STATE_HEADER_VALID,
    R7KR_STATE_READING,
    R7KR_STATE_CHECKSUM_VALID,
    R7KR_STATE_TIMESTAMP_VALID,
    R7KR_STATE_DRF_VALID,
    R7KR_STATE_DRF_REJECTED,
    R7KR_STATE_NF_INVALID,
    R7KR_STATE_NF_VALID,
    R7KR_STATE_FRAME_VALID,
    R7KR_STATE_FRAME_INVALID,
    R7KR_STATE_DISCONNECTED
} r7kr_parse_state_t;

/// @typedef enum r7kr_parse_action_t r7kr_parse_action_t
/// @brief 7k frame parsing actions
typedef enum{
    R7KR_ACTION_NOOP=0,
    R7KR_ACTION_READ,
    R7KR_ACTION_VALIDATE_HEADER,
    R7KR_ACTION_READ_DATA,
    R7KR_ACTION_VALIDATE_CHECKSUM,
    R7KR_ACTION_VALIDATE_TIMESTAMP,
    R7KR_ACTION_READ_NF,
    R7KR_ACTION_READ_DRF,
    R7KR_ACTION_RESYNC,
    R7KR_ACTION_QUIT
} r7kr_parse_action_t;

/// @typedef struct r7kr_reader_s r7kr_reader_t
/// @brief reson 7k center reader component
typedef struct r7kr_reader_s
{
    /// @var r7kr_reader_s::sockif
    /// @brief socket interface
    msock_socket_t *sockif;
    /// @var r7kr_reader_s::fileif
    /// @brief file interface
    mfile_file_t *fileif;
    /// @var r7kr_reader_s::fc
    /// @brief Data Record Frame container component
    r7k_drf_container_t *fc;
    /// @var r7kr_reader_s::state
    /// @brief reader state
    int state;
    /// @var r7kr_reader_s::sub_count
    /// @brief reson 7k center subscription count
    uint32_t sub_count;
    /// @var r7kr_reader_s::sub_list
    /// @brief reson 7k center subscription list
    uint32_t *sub_list;
    /// @var r7kr_reader_s::log
    /// @brief log (for binary data)
//    mlog_t *log;
    /// @var r7kr_reader_s::log_id
    /// @brief log ID (for binary data)
    mlog_id_t log_id;
    /// @var r7kr_reader_s::logstream
    /// @brief log file stream
    FILE *logstream;
    /// @var r7kr_reader_s::stats
    /// @brief reader statistics
    mstats_t *stats;
    /// @var r7kr_reader_s::watch
    /// @brief timing stopwatch
    mtime_stopwatch_t *watch;
}r7kr_reader_t;

/////////////////////////
// Macros
/////////////////////////
#define R7KR_NAME "r7k-reader"
#ifndef R7KR_VER
/// @def R7KR_VER
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DR7KR_VER=<version>
#define R7KR_VER (dev)
#endif
#ifndef R7KR_BUILD
/// @def R7KR_BUILD
/// @brief R7KRN library build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DR7KR_BUILD=`date`
//#define R7KR_BUILD "0000/00/00T00:00:00-0000"
#define R7KR_BUILD VERSION_STRING(FRAMES7K_VER) "" LIBMFRAME_BUILD
#endif

#define R7KR_VERSION_STR VERSION_STRING(R7KR_VER)

/// @def MAX_FRAME_BYTES_7K
/// @brief max DRF bytes
#define MAX_FRAME_BYTES_7K        60000
/// @def IP_PORT_7K
/// @brief reson 7k center IP port
#define IP_PORT_7K                 7000
/// @def R7KR_POLL_TIMEOUT_MSEC
/// @brief reader poll timeout default
#define R7KR_POLL_TIMEOUT_MSEC    5000
/// @def R7KR_FLUSH_RETRIES
/// @brief reader poll retries default
#define R7KR_FLUSH_RETRIES          10

// for the TRN application, use
// empirical numbers for
// TODO: make this configurable

/// @def R7KR_TRN_MESSAGE_SUBS
/// @brief number of reson 7k center subscription messages
#define R7KR_TRN_MESSAGE_SUBS       12
/// @def R7KR_TRN_PING_MSEC
/// @brief ping interval (msec)
#define R7KR_TRN_PING_MSEC         350
/// @def R7KR_TRN_PING_BYTES
/// @brief max bytes per ping
#define R7KR_TRN_PING_BYTES     250000
/// @def R7KR_TRN_REC_HINT
/// @brief TBD
#define R7KR_TRN_REC_HINT          128
/// @def R7KR_PING_INTERVAL_USEC
/// @brief ping interval (usec)
#define R7KR_PING_INTERVAL_USEC 350000
/// @def R7KR_PING_INTERVAL_MSEC
/// @brief ping interval (msec)
#define R7KR_PING_INTERVAL_MSEC    350
/// @def R7KR_READ_RETRIES
/// @brief read retries
#define R7KR_READ_RETRIES           8
/// @def R7KR_READ_TMOUT_MSEC
/// @brief read retries
#define R7KR_READ_TMOUT_MSEC        10
/// @def R7KR_RETRY_DELAY_SEC
/// @brief read retrie
#define R7KR_RETRY_DELAY_SEC        3
/// @def R7KR_REFILL_FRAMES
/// @brief frame buffer refill
#define R7KR_REFILL_FRAMES          1

/// @def R7K_PING_BUF_BYTES
/// @brief TBD
#define R7K_PING_BUF_BYTES R7K_TRN_PING_BYTES

/////////////////////////
// Exports
/////////////////////////
//mmd_module_config_t *mmd_r7kr_config;

// r7krn utility API
//const char *r7kr_get_version();
//const char *r7kr_get_build();
//void r7kr_show_app_version(const char *app_name, const char *app_version);


// r7krn reader API
r7kr_reader_t *r7kr_reader_new(const char *host, int port, uint32_t capacity, uint32_t *slist,  uint32_t slist_len);
r7kr_reader_t *r7kr_freader_new(mfile_file_t *file, uint32_t capacity, uint32_t *slist,  uint32_t slist_len);
void r7kr_reader_destroy(r7kr_reader_t **pself);
int r7kr_reader_connect(r7kr_reader_t *self, bool replace_socket);
void r7kr_reader_set_log(r7kr_reader_t *self, mlog_id_t id);
void r7kr_reader_set_logstream(r7kr_reader_t *self, FILE *log);
mstats_t *r7kr_reader_get_stats(r7kr_reader_t *self);
const char ***r7kr_reader_get_statlabels();

msock_socket_t *r7kr_reader_sockif(r7kr_reader_t *self);
mfile_file_t *r7kr_reader_fileif(r7kr_reader_t *self);
void r7kr_reader_show(r7kr_reader_t *self, bool verbose, uint16_t indent);
const char *r7kr_strstate(r7kr_state_t state);
bool r7kr_reader_issub(r7kr_reader_t *self, uint32_t record_type);
void r7kr_reader_reset_socket(r7kr_reader_t *self);
int r7kr_reader_set_file(r7kr_reader_t *self, mfile_file_t *file);
void r7kr_reader_flush(r7kr_reader_t *self, uint32_t len, int32_t retries, uint32_t tmout_ms);

int64_t r7kr_read_nf(r7kr_reader_t *self, byte *dest, uint32_t len, r7kr_flags_t flags, double newer_than, uint32_t timeout_msec, uint32_t *sync_bytes);
int64_t r7kr_read_drf(r7kr_reader_t *self, byte *dest, uint32_t len, r7kr_flags_t flags, double newer_than, uint32_t timeout_msec, uint32_t *sync_bytes);
int64_t r7kr_read_frame(r7kr_reader_t *self, byte *dest, uint32_t len, r7kr_flags_t flags, double newer_than, uint32_t timeout_msec, uint32_t *sync_bytes);
int64_t r7kr_read_stripped_frame(r7kr_reader_t *self, byte *dest, uint32_t len, r7kr_flags_t flags, double newer_than, uint32_t timeout_msec, uint32_t *sync_bytes );

int64_t r7kr_reader_seek(r7kr_reader_t *self, uint32_t ofs);
int64_t r7kr_reader_tell(r7kr_reader_t *self);
void r7kr_reader_purge(r7kr_reader_t *self);
r7k_drf_t    *r7kr_reader_next(r7kr_reader_t *self);
r7k_drf_t *r7kr_reader_enumerate(r7kr_reader_t *self);
uint32_t r7kr_reader_frames(r7kr_reader_t *self);
int64_t r7kr_reader_read(r7kr_reader_t *self, byte *dest, uint32_t len);


// functions for peer comparisons
// used in r7krnpreprocess

bool r7kr_peer_cmp(void *a, void *b);
bool r7kr_peer_vcmp(void *item, void *value);

#ifdef WITH_R7KR_TEST
int r7kr_test(int argc, char **argv);
#endif // WITH_R7KR_TEST

// include guard
#endif
