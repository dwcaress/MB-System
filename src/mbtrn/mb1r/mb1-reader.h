///
/// @file mb1-reader.h
/// @authors k. headley
/// @date 06 nov 2012

/// Reson 7KCenter reader API
/// contains mb1r_reader_t, a component that
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
#ifndef MB1_READER_H
/// @def MB1_READER_H
/// @brief TBD
#define MB1_READER_H

/////////////////////////
// Includes
/////////////////////////

#include "mframe.h"
#include "mlog.h"
#include "msocket.h"
#include "mfile.h"
#include "mconfig.h"
#include "mstats.h"
#include "mb1-io.h"

/////////////////////////
// Type Definitions
/////////////////////////

/// @typedef enum mb1r_event_id mb1r_event_id
/// @brief diagnostic event (counter) IDs
typedef enum{
    MB1R_EV_FRAME_VALID=0,
    MB1R_EV_FRAME_INVALID,
    MB1R_EV_HDR_VALID,
    MB1R_EV_DATA_VALID,
    MB1R_EV_HDR_INVALID,
    MB1R_EV_DATA_INVALID,
    MB1R_EV_DATA_RESYNC,
    MB1R_EV_HDR_RESYNC,
    MB1R_EV_HDR_SHORT_READ,
    MB1R_EV_DATA_SHORT_READ,
    MB1R_EV_EHDRTYPE,
    MB1R_EV_EHDRSZ,
    MB1R_EV_EHDRTS,
    MB1R_EV_ECHKSUM,
    MB1R_EV_EHDRREAD,
    MB1R_EV_ESOCK,
    MB1R_EV_EDATASYNC,
    MB1R_EV_EDATAREAD,
    MB1R_EV_EFCWR,
    MB1R_EV_COUNT
}mb1r_event_id;

/// @typedef enum mb1r_status_id mb1r_status_id
/// @brief diagnostic status (counter) IDs
typedef enum{
    MB1R_STA_FRAME_VAL_BYTES=0,
    MB1R_STA_HDR_VAL_BYTES,
    MB1R_STA_DATA_VAL_BYTES,
    MB1R_STA_HDR_INVAL_BYTES,
    MB1R_STA_DATA_INVAL_BYTES,
    MB1R_STA_COUNT
}mb1r_status_id;

/// @typedef enum mb1r_metric_id mb1r_metric_id
/// @brief diagnostic status (floating point) measurement IDs
typedef enum{
    //    MB1R_MET_MB1RN_REFILL_XT=0
    MB1R_MET_7KFRAME_SKEW=0,
    MB1R_MET_COUNT
}mb1r_metric_id;

/// @typedef enum mb1r_cstate mb1r_cstate
/// @brief connection state enum
typedef enum {CS_NEW,CS_INITIALIZED,CS_CONNECTED} mb1r_cstate;
/// @typedef enum mb1r_ctype mb1r_ctype
/// @brief connection endpoint types
typedef enum {CT_NULL,CT_STDIN,CT_STDOUT,CT_STDERR,CT_FILE,CT_SOCKET} mb1r_ctype;
/// @typedef enum mb1r_state_t mb1r_state_t
/// @brief reader state enum
typedef enum {MB1R_NEW,MB1R_INITIALIZED,MB1R_CONNECTED,MB1R_SUBSCRIBED} mb1r_state_t;
/// @typedef enum mb1r_flags_t mb1r_flags_t
/// @brief reader behavior flags
typedef enum{
    MB1R_NOFLAGS=0x0,
    MB1R_ALLOW_PARTIAL=0x01,
    MB1R_FORCE=0x2,
    MB1R_IFLUSH=0x4,
    MB1R_OFLUSH=0x8,
    MB1R_FLUSH=0x10,
    MB1R_NOFLUSH=0x20,
    MB1R_BLOCK=0x40,
    MB1R_NONBLOCK=0x80,
    MB1R_NET_STREAM=0x100,
    MB1R_NF_STREAM=0x200,
    MB1R_DRF_STREAM=0x400,
    MB1R_RESYNC_HEADER=0x800,
    MB1R_RESYNC_DATA=0x1000
} mb1r_flags_t;

/// @typedef enum mb1r_parse_state_t mb1r_parse_state_t
/// @brief 7k frame parsing states
typedef enum{
    MB1R_STATE_START=0,
    MB1R_STATE_READ_ERR,
    MB1R_STATE_READ_OK,
    MB1R_STATE_READING,
//    MB1R_STATE_CHECKSUM_VALID,
//    MB1R_STATE_TIMESTAMP_VALID,
    MB1R_STATE_DATA_VALID,
    MB1R_STATE_DATA_INVALID,
//    MB1R_STATE_DATA_REJECTED,
    MB1R_STATE_HEADER_INVALID,
    MB1R_STATE_HEADER_VALID,
    MB1R_STATE_FRAME_VALID,
    MB1R_STATE_FRAME_INVALID,
    MB1R_STATE_COMPLETE,
    MB1R_STATE_DISCONNECTED
} mb1r_parse_state_t;

/// @typedef enum mb1r_parse_action_t mb1r_parse_action_t
/// @brief 7k frame parsing actions
typedef enum{
    MB1R_ACTION_NOOP=0,
    MB1R_ACTION_READ,
    MB1R_ACTION_VALIDATE_HEADER,
    MB1R_ACTION_VALIDATE_DATA,
    MB1R_ACTION_READ_HEADER,
    MB1R_ACTION_READ_DATA,
//    MB1R_ACTION_VALIDATE_CHECKSUM,
//    MB1R_ACTION_VALIDATE_TIMESTAMP,
    MB1R_ACTION_RESYNC,
    MB1R_ACTION_QUIT
} mb1r_parse_action_t;

/// @typedef struct mb1r_sm_ctx_s mb1r_sm_ctx_t
/// @brief MB1 reader state machine context
typedef struct mb1r_sm_ctx_s
{
    mb1r_parse_state_t state;
    mb1r_parse_action_t action;

    mb1r_flags_t flags;
    mb1r_flags_t rflags;
    double newer_than;
    uint32_t timeout_msec;
    uint32_t *sync_bytes;

    bool header_pending;
    bool data_pending;
    bool sync_found;

    int merrno;
    mb1_sounding_t *psnd;
    byte *dest;
    byte *pbuf;
    byte *psync;
    uint32_t len;
    int64_t frame_bytes;
    int64_t lost_bytes;
    uint32_t pending_bytes;
    uint32_t header_bytes;
    uint32_t data_bytes;
    int64_t read_bytes;
    uint32_t read_len;
    int32_t cx;
}mb1r_sm_ctx_t;

/// @typedef struct mb1r_reader_s mb1r_reader_t
/// @brief reson 7k center reader component
typedef struct mb1r_reader_s
{
    /// @var mb1r_reader_s::sockif
    /// @brief socket interface
    msock_socket_t *sockif;
    /// @var mb1r_reader_s::fileif
    /// @brief file interface
    mfile_file_t *fileif;
    /// @var mb1r_reader_s::state
    /// @brief reader state
    int state;
    /// @var mb1r_reader_s::log_id
    /// @brief log ID (for binary data)
    mlog_id_t log_id;
    /// @var mb1r_reader_s::logstream
    /// @brief log file stream
    FILE *logstream;
    /// @var mb1r_reader_s::stats
    /// @brief reader statistics
    mstats_t *stats;
    /// @var mb1r_reader_s::watch
    /// @brief timing stopwatch
    mtime_stopwatch_t *watch;
}mb1r_reader_t;

/////////////////////////
// Macros
/////////////////////////
#define MB1R_NAME "mb1-reader"
#ifndef MB1R_VER
/// @def MB1R_VER
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMB1R_VER=<version>
#define MB1R_VER (dev)
#endif
#ifndef MB1R_BUILD
/// @def MB1R_BUILD
/// @brief MB1RN library build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMB1R_BUILD=`date`
//#define MB1R_BUILD "0000/00/00T00:00:00-0000"
#define MB1R_BUILD VERSION_STRING(FRAMES7K_VER)" "LIBMFRAME_BUILD
#endif

#define MB1R_VERSION_STR VERSION_STRING(MB1R_VER)

/// @def MAX_MB1_FRAME_BYTES
/// @brief max MB1 bytes
#define MAX_MB1_FRAME_BYTES        MB1_SOUNDING_BYTES(512)
/// @def IP_PORT_MB1
/// @brief reson 7k center IP port
#define MB1_IO_PORT               7007
/// @def MB1R_POLL_TIMEOUT_MSEC
/// @brief reader poll timeout default
#define MB1R_POLL_TIMEOUT_MSEC    5000
/// @def MB1R_FLUSH_RETRIES
/// @brief reader poll retries default
#define MB1R_FLUSH_RETRIES          10

/// @def MB1R_READ_RETRIES
/// @brief read retries
#define MB1R_READ_RETRIES           8
/// @def MB1R_READ_TMOUT_MSEC
/// @brief read retries
#define MB1R_READ_TMOUT_MSEC        10
/// @def MB1R_RETRY_DELAY_SEC
/// @brief read retrie
#define MB1R_RETRY_DELAY_SEC        3

/// @def MB1_PING_BUF_BYTES
/// @brief TBD
#define MB1_PING_BUF_BYTES MB1_TRN_PING_BYTES

/////////////////////////
// Exports
/////////////////////////
//mmd_module_config_t *mmd_mb1r_config;

// mb1rn utility API
//const char *mb1r_get_version();
//const char *mb1r_get_build();
//void mb1r_show_app_version(const char *app_name, const char *app_version);


// mb1rn reader API
mb1r_reader_t *mb1r_reader_new(const char *host, int port, uint32_t capacity);
mb1r_reader_t *mb1r_freader_new(mfile_file_t *file, uint32_t capacity, uint32_t *slist,  uint32_t slist_len);
void mb1r_reader_destroy(mb1r_reader_t **pself);
int mb1r_reader_connect(mb1r_reader_t *self, bool replace_socket);
void mb1r_reader_set_log(mb1r_reader_t *self, mlog_id_t id);
void mb1r_reader_set_logstream(mb1r_reader_t *self, FILE *log);
mstats_t *mb1r_reader_get_stats(mb1r_reader_t *self);
const char ***mb1r_reader_get_statlabels();

msock_socket_t *mb1r_reader_sockif(mb1r_reader_t *self);
mfile_file_t *mb1r_reader_fileif(mb1r_reader_t *self);
void mb1r_reader_show(mb1r_reader_t *self, bool verbose, uint16_t indent);
const char *mb1r_strstate(mb1r_state_t state);
bool mb1r_reader_issub(mb1r_reader_t *self, uint32_t record_type);
void mb1r_reader_reset_socket(mb1r_reader_t *self);
int mb1r_reader_set_file(mb1r_reader_t *self, mfile_file_t *file);
void mb1r_reader_flush(mb1r_reader_t *self, uint32_t len, int32_t retries, uint32_t tmout_ms);

int64_t mb1r_read_frame(mb1r_reader_t *self, byte *dest, uint32_t len, mb1r_flags_t flags, double newer_than, uint32_t timeout_msec, uint32_t *sync_bytes);
const char *mb1r_ctx_strstate(mb1r_parse_state_t state);
const char *mb1r_ctx_straction(mb1r_parse_action_t state);


// functions for peer comparisons
// used in mb1rnpreprocess

bool mb1r_peer_cmp(void *a, void *b);
bool mb1r_peer_vcmp(void *item, void *value);

#ifdef WITH_MB1R_TEST
int mb1r_test(int argc, char **argv);
#endif // WITH_MB1R_TEST

// include guard
#endif
