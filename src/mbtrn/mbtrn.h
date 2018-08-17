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
#include <float.h>
#include "iowrap.h"
#include "cbuffer.h"
#include "r7kc.h"
#include "mlog.h"
#include "mconfig.h"

/////////////////////////
// Type Definitions
/////////////////////////

/// @typedef enum mbtrn_cstate mbtrn_cstate
/// @brief connection state enum
typedef enum {CS_NEW,CS_INITIALIZED,CS_CONNECTED} mbtrn_cstate;
/// @typedef enum mbtrn_ctype mbtrn_ctype
/// @brief connection endpoint types
typedef enum {CT_NULL,CT_STDIN,CT_STDOUT,CT_STDERR,CT_FILE,CT_SOCKET} mbtrn_ctype;
/// @typedef enum mbtrn_state_t mbtrn_state_t
/// @brief reader state enum
typedef enum {MBS_NEW,MBS_INITIALIZED,MBS_CONNECTED,MBS_SUBSCRIBED} mbtrn_state_t;
/// @typedef enum mbtrn_flags_t mbtrn_flags_t
/// @brief reader behavior flags
typedef enum{
    MBR_ALLOW_PARTIAL=0x01,
    MBR_FORCE=0x2,
    MBR_IFLUSH=0x4,
    MBR_OFLUSH=0x8,
    MBR_FLUSH=0x10,
    MBR_NOFLUSH=0x20,
    MBR_BLOCK=0x40,
    MBR_NONBLOCK=0x80,
    MBR_NET_STREAM=0x100,
    MBR_NF_STREAM=0x200,
    MBR_DRF_STREAM=0x400,
    MBR_RESYNC_NF=0x800,
    MBR_RESYNC_DRF=0x1000
} mbtrn_flags_t;

typedef enum{
    MBR_STATE_START=0,
    MBR_STATE_READ_ERR,
    MBR_STATE_READ_OK,
    MBR_STATE_DRF_INVALID,
    MBR_STATE_HEADER_VALID,
    MBR_STATE_READING,
    MBR_STATE_CHECKSUM_VALID,
    MBR_STATE_TIMESTAMP_VALID,
    MBR_STATE_DRF_VALID,
    MBR_STATE_DRF_REJECTED,
    MBR_STATE_NF_INVALID,
    MBR_STATE_NF_VALID,
    MBR_STATE_FRAME_VALID,
    MBR_STATE_FRAME_INVALID,
    MBR_STATE_DISCONNECTED
} mbtrn_parse_state_t;

typedef enum{
    MBR_ACTION_NOOP=0,
    MBR_ACTION_READ,
    MBR_ACTION_VALIDATE_HEADER,
    MBR_ACTION_READ_DATA,
    MBR_ACTION_VALIDATE_CHECKSUM,
    MBR_ACTION_VALIDATE_TIMESTAMP,
    MBR_ACTION_READ_NF,
    MBR_ACTION_READ_DRF,
    MBR_ACTION_RESYNC,
    MBR_ACTION_QUIT
} mbtrn_parse_action_t;

/// @typedef enum mbstat_ch_u64_t mbstat_ch_u64_t
/// @brief statistics channel indices
typedef enum{MST_SCON_TOT=0,MST_SCON_ACT,MST_CCON_TOT,MST_REC_TOT,MST_PUB_TOT} mbstat_ch_u64_t;
/// @typedef enum mbstat_ch_i64_t mbstat_ch_i64_t
/// @brief statistics channel indices
typedef enum{MST_I640=0} mbstat_ch_i64_t;
/// @typedef enum mbstat_ch_d_t mbstat_ch_d_t
/// @brief statistics channel indices
typedef enum{MST_D0=0} mbstat_ch_d_t;

#define MBTRN_STAT_CHANNELS 16

/// @typedef struct mbtrn_stats_s mbtrn_stats_t
/// @brief mbtrn reader stat
typedef struct mbtrn_stats_s{
    /// @var mbtrn_stats_s::start_time
    /// @brief total client connections
    time_t start_time;
    /// @var mbtrn_stats_s::chan_u64
    /// @brief unsigned 64 bit integers
    int64_t chan_i64[MBTRN_STAT_CHANNELS];
    /// @var mbtrn_stats_s::chan_u64
    /// @brief unsigned 64 bit integers
    uint64_t chan_u64[MBTRN_STAT_CHANNELS];
    /// @var mbtrn_stats_s::chan_d
    /// @brief double values
    double chan_d[MBTRN_STAT_CHANNELS];
}mbtrn_stats_t;

typedef struct mbtrn_stat_chan_s
{
    uint64_t n;
    double sum;
    double min;
    double max;
    double avg;
}mbtrn_stat_chan_t;

/// @typedef struct mbtrn_cmeas_s mbtrn_cmeas_s
/// @brief structure for measuring continuous quantities
/// and intervals (e.g. floating point)
typedef struct mbtrn_cmeas_s
{
    double start;
    double stop;
    double value;
}mbtrn_cmeas_t;

typedef enum{
    MBTR_EV_FRAME_VALID=0,
    MBTR_EV_FRAME_INVALID,
    MBTR_EV_NF_VALID,
    MBTR_EV_DRF_VALID,
    MBTR_EV_NF_INVALID,
    MBTR_EV_DRF_INVALID,
    MBTR_EV_DRF_RESYNC,
    MBTR_EV_NF_RESYNC,
    MBTR_EV_NF_SHORT_READ,
    MBTR_EV_DRF_SHORT_READ,
    MBTR_EV_EDRFPROTO,
    MBTR_EV_ENFTOTALREC,
    MBTR_EV_ENFPACKETSZ,
    MBTR_EV_ENFOFFSET,
    MBTR_EV_ENFVER,
    MBTR_EV_ENFREAD,
    MBTR_EV_ESOCK,
    MBTR_EV_EDRFCHK,
    MBTR_EV_EDRFTIME,
    MBTR_EV_EDRFSIZE,
    MBTR_EV_EDRFSYNC,
    MBTR_EV_EDRFREAD,
    MBTR_EV_EFCWR,
    MBTR_EV_FC_READ,
    MBTR_EV_FC_REFILL,
    MBTR_EV_COUNT
}mbtr_stevent_id;

typedef enum{
    MBTR_STA_FRAME_VAL_BYTES=0,
    MBTR_STA_NF_VAL_BYTES,
    MBTR_STA_DRF_VAL_BYTES,
    MBTR_STA_NF_INVAL_BYTES,
    MBTR_STA_DRF_INVAL_BYTES,
    MBTR_STA_SUB_FRAMES,
    MBTR_STA_COUNT
}mbtr_ststatus_id;

typedef enum{
    MBTR_CH_MBTRN_REFILL_XT=0,
    MBTR_CH_COUNT
}mbtr_stchan_id;

typedef uint32_t mbtrn_dmeas_t;

typedef enum {MBTF_STATUS=0x1, MBTF_EVENT=0x2, MBTF_PSTAT=0x4, MBTF_ASTAT=0x8, MBTF_READER=0x10}mbtr_stat_flags;
typedef enum {MBTR_LABEL_EV=0, MBTR_LABEL_ST, MBTR_LABEL_ME, MBTR_LABEL_COUNT}mbtr_label_id;

typedef struct mbtr_stats_s{
    double stat_period_start;
    double stat_period_sec;
    uint32_t ev_n;
    uint32_t st_n;
    uint32_t tm_n;
    mbtrn_dmeas_t *events;
    mbtrn_dmeas_t *status;
    mbtrn_cmeas_t *measurements;
    mbtrn_stat_chan_t *per_stats;
    mbtrn_stat_chan_t *agg_stats;
    const char ***labels;
}mbtr_stats_t;

/// @typedef struct mbtrn_reader_s mbtrn_reader_t
/// @brief reson 7k center reader component
typedef struct mbtrn_reader_s
{
    /// @var mbtrn_reader_s::sockif
    /// @brief socket interface
    iow_socket_t *sockif;
    /// @var mbtrn_reader_s::fileif
    /// @brief file interface
    iow_file_t *fileif;
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
    /// @var mbtrn_reader_s::log
    /// @brief log (for binary data)
	mlog_t *log;
    /// @var mbtrn_reader_s::log_id
    /// @brief log ID (for binary data)
    mlog_id_t log_id;
    /// @var mbtrn_reader_s::logstream
    /// @brief log file stream
    FILE *logstream;
    /// @var mbtrn_reader_s::stats
    /// @brief reader statistics
    mbtr_stats_t *stats;
}mbtrn_reader_t;

/////////////////////////
// Macros
/////////////////////////

#ifndef MBTRN_VER
#define MBTRN_VER 1.4.3
#endif
#ifndef MBTRN_BUILD
#define MBTRN_BUILD "0000/00/00T00:00:00-0000"
#endif

#define VERSION_HELPER(s) #s
#define VERSION_STRING(s) VERSION_HELPER(s)
#define LIBMBTRN_VERSION ""VERSION_STRING(MBTRN_VER)
#define LIBMBTRN_BUILD ""VERSION_STRING(MBTRN_BUILD)

// enable using MBTR_STATS_EN definition in mconfig.h
#ifdef MBTR_STATS_EN
// some of these are trivial - using macros
// so they can be compiled out with one #define
#define MBTR_SW_START(w,t)           (w.start=t)
#define MBTR_SW_STOP(w,t)            (w.stop=t)
#define MBTR_SW_LAP(w,t)             (w.value += (t-w.start))
#define MBTR_SW_REC(w)               (w.value = (w.stop-w.start))
#define MBTR_SW_DIV(w,n)             (w.value = (w.value/(double)n))
#define MBTR_SW_RESET(w)             (w.value=0.0)
#define MBTR_SW_ELAPSED(w)           (w.value)

#define MBTR_COUNTER_INC(v)         (v++)
#define MBTR_COUNTER_DEC(v)         (v--)
#define MBTR_COUNTER_ADD(v,n)       ((v) += (n))
#define MBTR_COUNTER_ADIF(v,a,b)    (v += (a-b))
#define MBTR_COUNTER_SET(v,n)       (v = n)
#define MBTR_COUNTER_GET(v)         (v)

#define MBTR_STATS_SMAX(v,a)        ( (a > v.max) ? a : v.max)
#define MBTR_STATS_SMIN(v,a)        ( (a < v.min) ? a : v.min)
#define MBTR_STATS_AVG(v)           ( (v.n>0 ? (double)(v.sum)/(v.n) : DBL_MAX) )

#else

#define MBTR_SW_START(w,t)
#define MBTR_SW_STOP(w,t)
#define MBTR_SW_LAP(w,t)
#define MBTR_SW_REC(w)
#define MBTR_SW_DIV(w,n)
#define MBTR_SW_RESET(w)
#define MBTR_SW_ELAPSED(w)          0.0

#define MBTR_COUNTER_INC(v)
#define MBTR_COUNTER_DEC(v)
#define MBTR_COUNTER_ADD(v,n)
#define MBTR_COUNTER_ADIF(v,a,b)
#define MBTR_COUNTER_SET(v,n)
#define MBTR_COUNTER_GET(v)         0.0
#define MBTR_STATS_SMAX(v,a)        0.0
#define MBTR_STATS_SMIN(v,a)        0.0
#define MBTR_STATS_AVG(v)           0.0
#endif //MBTR_STATS_EN


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
/// @def MBTRN_READ_RETRIES
/// @brief read retries
#define MBTRN_READ_RETRIES           8
/// @def MBTRN_READ_TMOUT_MSEC
/// @brief read retries
#define MBTRN_READ_TMOUT_MSEC        10
/// @def MBTRN_RETRY_DELAY_SEC
/// @brief read retrie
#define MBTRN_RETRY_DELAY_SEC        3
/// @def MBTRN_REFILL_FRAMES
/// @brief frame buffer refill
#define MBTRN_REFILL_FRAMES          1

/// @def R7K_PING_BUF_BYTES
/// @brief TBD
#define R7K_PING_BUF_BYTES R7K_TRN_PING_BYTES


/////////////////////////
// Exports
/////////////////////////

// mbtrn utility API
const char *mbtrn_get_version();
const char *mbtrn_get_build();
void mbtrn_show_app_version(const char *app_name, const char *app_version);


// mbtrn reader API
mbtrn_reader_t *mbtrn_reader_new(const char *host, int port, uint32_t capacity, uint32_t *slist,  uint32_t slist_len);
mbtrn_reader_t *mbtrn_freader_new(iow_file_t *file, uint32_t capacity, uint32_t *slist,  uint32_t slist_len);
void mbtrn_reader_destroy(mbtrn_reader_t **pself);
int mbtrn_reader_connect(mbtrn_reader_t *self, bool replace_socket);
void mbtrn_reader_set_log(mbtrn_reader_t *self, mlog_t *log, mlog_id_t id, char *desc);
void mbtrn_reader_set_logstream(mbtrn_reader_t *self, FILE *log);
mbtr_stats_t *mbtrn_reader_get_stats(mbtrn_reader_t *self);
iow_socket_t *mbtrn_reader_sockif(mbtrn_reader_t *self);
iow_file_t *mbtrn_reader_fileif(mbtrn_reader_t *self);
void mbtrn_reader_show(mbtrn_reader_t *self, bool verbose, uint16_t indent);
const char *mbtrn_strstate(mbtrn_state_t state);
bool mbtrn_reader_issub(mbtrn_reader_t *self, uint32_t record_type);
void mbtrn_reader_reset_socket(mbtrn_reader_t *self);
int mbtrn_reader_set_file(mbtrn_reader_t *self, iow_file_t *file);



int64_t mbtrn_read_nf(mbtrn_reader_t *self, byte *dest, uint32_t len, mbtrn_flags_t flags, double newer_than, uint32_t timeout_msec, uint32_t *sync_bytes);
int64_t mbtrn_read_drf(mbtrn_reader_t *self, byte *dest, uint32_t len, mbtrn_flags_t flags, double newer_than, uint32_t timeout_msec, uint32_t *sync_bytes);
int64_t mbtrn_read_frame(mbtrn_reader_t *self, byte *dest, uint32_t len, mbtrn_flags_t flags, double newer_than, uint32_t timeout_msec, uint32_t *sync_bytes);
int64_t mbtrn_read_stripped_frame(mbtrn_reader_t *self, byte *dest, uint32_t len, mbtrn_flags_t flags, double newer_than, uint32_t timeout_msec, uint32_t *sync_bytes );

// Deprecated reader API
int64_t mbtrn_reader_poll(mbtrn_reader_t *self, byte *dest, uint32_t len, uint32_t tmout_ms);
int64_t mbtrn_reader_xread(mbtrn_reader_t *self, byte *dest, uint32_t len, uint32_t tmout_ms,  mbtrn_flags_t flags, uint32_t max_age_ms);
int64_t mbtrn_reader_xread_orig(mbtrn_reader_t *self, byte *dest, uint32_t len, uint32_t tmout_ms, mbtrn_flags_t flags);
int64_t mbtrn_reader_xread_frame(mbtrn_reader_t *self, byte *dest, uint32_t len);
int64_t mbtrn_read_frame_dep(mbtrn_reader_t *self, byte *dest, uint32_t len, mbtrn_flags_t flags, double newer_than, uint32_t timeout_msec );
int64_t mbtrn_reader_parse(mbtrn_reader_t *self, byte *src, uint32_t len, r7k_drf_container_t *dest);

int64_t mbtrn_reader_read(mbtrn_reader_t *self, byte *dest, uint32_t len);
int64_t mbtrn_reader_seek(mbtrn_reader_t *self, uint32_t ofs);
int64_t mbtrn_reader_tell(mbtrn_reader_t *self);
void mbtrn_reader_flush(mbtrn_reader_t *self, uint32_t len, int32_t retries, uint32_t tmout_ms);
void mbtrn_reader_purge(mbtrn_reader_t *self);
uint32_t mbtrn_reader_frames(mbtrn_reader_t *self);
r7k_drf_t *mbtrn_reader_enumerate(mbtrn_reader_t *self);
r7k_drf_t *mbtrn_reader_next(mbtrn_reader_t *self);

// mbtrn stats API
mbtr_stats_t *mbtrn_stats_new(uint32_t ev_counters, uint32_t status_counters, uint32_t tm_channels, const char ***labels);
void mbtrn_stats_destroy(mbtr_stats_t **pself);
int mbtrn_update_stats(mbtr_stats_t *stats, uint32_t channels, mbtr_stat_flags flags);
void mbtrn_reset_pstats(mbtr_stats_t *stats, uint32_t channels);
void mbtrn_stats_set_period(mbtr_stats_t *self, double period_start, double period_sec);
int mbtrn_log_stats(mbtr_stats_t *stats, double now, mlog_id_t log_id, mbtr_stat_flags flags);
int mbtrn_log_timing(mlog_id_t log_id, mbtrn_stat_chan_t *stats,  double timestamp, char *type_str, const char **labels, int channels);
int mbtrn_log_counts(mlog_id_t log_id, uint32_t *counts, double timestamp, char *type_str, const char **labels, int channels);

// functions for peer comparisons
// used in mbtrnpreprocess

bool mbtrn_peer_cmp(void *a, void *b);
bool mbtrn_peer_vcmp(void *item, void *value);

// include guard
#endif
