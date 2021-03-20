///
/// @file trnu_cli.h
/// @authors k. headley
/// @date 09 jul 2019

/// TRN update UDP client API

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
#ifndef TRNU_CLI_H
#define TRNU_CLI_H

/////////////////////////
// Includes 
/////////////////////////
#define WITH_ASYNC_TRNU


#include "trnw.h"
#include "mframe.h"
#include "msocket.h"
#include "medebug.h"
#ifdef WITH_ASYNC_TRNU
#include "mthread.h"
#include "mlog.h"
#endif

/////////////////////////
// Macros
/////////////////////////
// check flags
// f: flag variable
// m: mask (flags to check)
#define UCF_ISSET(f,m)  ( (f&m) != 0 ? true : false )
#define UCF_ISCLR(f,m)  ( (f&m) == 0 ? true : false )
#define UCF_ANYSET(f,m) ( (f&m) != 0 ? true : false )
#define UCF_ANYCLR(f,m) ( (f&m) != m ? true : false )
#define UCF_ALLCLR(f,m) ( (f&m) == 0 ? true : false )
#define UCF_ALLSET(f,m) ( (f&m) == m ? true : false )
// set flags
// pf: flag pointer
// m: mask (flags to set)
#define UCF_MSET(pf,m) do{ if(NULL!=pf)*pf|=m; }while(0)
// clear flags
// pf: flag pointer
// m: mask (flags to set)
#define UCF_MCLR(pf,m) do{ if(NULL!=pf)*pf&=~(m); }while(0)
// update string buffer len (holds max string)
#define TRNUC_STR_LEN 2048
// number of fields in CSV record
#define TRNUC_CSV_FIELDS 50 //41
// max CSV record string length
#define TRNUC_CSV_LINE_BYTES 512

#define TRNUC_HBEAT_TO_SEC_DFL      30.0
#define TRNUC_LISTEN_TO_MSEC_DFL    50
#define TRNUC_RECON_TO_SEC_DFL      10.0
#define TRNUC_ENODATA_DEL_MSEC_DFL  50
#define TRNUC_ERECON_DEL_MSEC_DFL 5000

/////////////////////////
// Type Definitions
/////////////////////////

// Configuration flags
// TRNUC_BLK_CON    block on connect
// TRNUC_BLK_LISTEN block on listen
// TRNUC_CON_MSG    send connect message
// TRNUC_MCAST      using multicast (unidirectional)
typedef enum{
    TRNUC_BLK_CON   =0x010,
    TRNUC_BLK_LISTEN=0x020,
    TRNUC_CON_MSG   =0x100,
    TRNUC_MCAST     =0x200
}trnucli_flags_t;

// callback function typedef
typedef int (* update_callback_fn)(trnu_pub_t *update);

// trnu_cli instance
typedef struct trnucli_s{
    // struct trnucli_s::trnu
    // server socket connection
    msock_connection_t *trnu;
    // struct trnucli_s::update
    // last update received (NULL if unset)
    trnu_pub_t *update;
    // struct trnucli_s::update_fn
    // callback function (NULL if unset)
    update_callback_fn update_fn;
    // struct trnucli_s::flags
    // configuration flags
    trnucli_flags_t flags;
    // struct trnucli_s::hbeat_to_sec
    // heartbeat timeout (s)
    double hbeat_to_sec;
}trnucli_t;

#ifdef WITH_ASYNC_TRNU

typedef enum{
    CTX_STOPPED=0,
    CTX_DISCONNECTED,
    CTX_CONNECTING,
    CTX_LISTENING,
    CTX_INVALID,
    CTX_STATES
}trnucli_state_t;

typedef enum{
    ACT_NOP=0,
    ACT_CONNECT,
    ACT_LISTEN,
    ACT_DISCONNECT,
    CTX_ACTIONS
}trnucli_action_t;

typedef enum{
    CTX_LOG_EN=0x1
}trnuctx_flags_t;

typedef struct trnucli_stats_s{
    // struct trnucli_stats_s::n_cycle
    // number of async client cycles
    int n_cycle;
    // struct trnucli_stats_s::n_connect
    // number of server connections
    int n_connect;
    // struct trnucli_stats_s::n_disconnect
    // number of server disconnections
    int n_disconnect;
    // struct trnucli_stats_s::n_reset
    // number of TRN resets
    int n_reset;
    // struct trnucli_stats_s::n_update
    // number of updates received
    int n_update;
    // struct trnucli_stats_s::n_hbeat
    // number of heartbeat timeouts
    int n_hbeat;
    // struct trnucli_stats_s::n_rcto
    // number of reconnect timeouts
    int n_rcto;
    // struct trnucli_stats_s::n_elisten
    // number of listen errors
    int n_elisten;
    // struct trnucli_stats_s::n_econnect
    // number of connect errors
    int n_econnect;
    // struct trnucli_stats_s::t_session
    // session run  time
    double t_session;
    // struct trnucli_stats_s::t_listening
    // time in listening state
    double t_listening;
    // struct trnucli_stats_s::t_connecting
    // time in connecting state
    double t_connecting;
}trnucli_stats_t;

// async trnu_cli context
typedef struct trnucli_ctx_s{
    // struct trnucli_ctx_s::trnucli
    // client instance
    trnucli_t *cli;
    // struct trnucli_ctx_s::host
    // client host IP
    char *host;
    // struct trnucli_ctx_s::port
    // client host port
    int port;
    // struct trnucli_ctx_s::ttl
    // multicast ttl
    int ttl;
    // struct trnucli_ctx_s::worker
    // client worker thread
    mthread_thread_t *worker;
    // struct trnucli_ctx_s::update
    // last update received (NULL if unset)
    trnu_pub_t *update;
    // struct trnucli_ctx_s::update_trx
    // last update time
    double update_trx;
    // struct trnucli_ctx_s::update_mtx
    // mutex guarding last update data
    mthread_mutex_t *mtx;
    // struct trnucli_ctx_s::hbeat_to_sec
    // heartbeat timeout
    double hbeat_to_sec;
    // struct trnucli_ctx_s::recon_to_sec
    // reconnect timeout (if no data available for recon_to_sec)
    double recon_to_sec;
    // struct trnucli_ctx_s::listen_to_ms
    // listen timeout msec (0 to disable)
    uint32_t listen_to_ms;
    // struct trnucli_ctx_s::enodata_delay_ms
    // delay after if no data available
    uint32_t enodata_delay_ms;
    // struct trnucli_ctx_s::erecon_delay_ms
    // delay between reconnect attempts
    uint32_t erecon_delay_ms;
    // struct trnucli_ctx_s::stats_log_sec
    // stats logging period (0 to disable)
    uint32_t stats_log_sec;
    // struct trnucli_ctx_s::state
    // context state
    trnucli_state_t state;
    // struct trnucli_ctx_s::action
    // context action
    trnucli_action_t action;
    // struct trnucli_ctx_s::stop
    // request thread stop
    bool stop;
    // struct trnucli_ctx_s::reconnect
    // request reconnect
    bool reconnect;
    // struct trnucli_ctx_s::rc_timer
    // reconnect timer
    double rc_timer;
    // struct trnucli_ctx_s::hb_timer
    // reconnect timer
    double hb_timer;
    // struct trnucli_ctx_s::connecting_timer
    // connecting state timer
    double connecting_timer;
    // struct trnucli_ctx_s::listening_timer
    // listening state timer
    double listening_timer;
    // struct trnucli_ctx_s::stats_log_timer
    // stats logging timer
    double stats_log_timer;
    // struct trnucli_ctx_s::session_timer
    // session timer
    double session_timer;
    // struct trnucli_ctx_s::new_count
    // number of updates since last read
    uint32_t new_count;
    // struct trnucli_ctx_s::stats
    // performance stats
    trnucli_stats_t stats;
    /// @var trnucli_ctx_s::flags
    /// @brief TBD
    trnuctx_flags_t flags;
    /// @var trnucli_ctx_s::log_cfg
    /// @brief TBD
    mlog_config_t *log_cfg;
    /// @var trnucli_ctx_s::log_id
    /// @brief TBD
    mlog_id_t log_id;
    /// @var trnucli_ctx_s::log_name
    /// @brief TBD
    char *log_name;
    /// @var trnucli_ctx_s::log_dir
    /// @brief TBD
    char *log_dir;
    /// @var trnucli_ctx_s::log_path
    /// @brief TBD
    char *log_path;
    /// @var trnucli_ctx_s::status
    /// @brief TBD
   int status;
}trnucli_ctx_t;
#endif

#define TRNUC_FMT_PRETTY_STR "pretty"
#define TRNUC_FMT_CSV_STR "csv"
#define TRNUC_FMT_HEX_STR "hex"
#define TRNUC_FMT_PRETTY_HEX_STR "pretty hex"
#define TRNUC_OFILE_SOUT_STR "stdout"
#define TRNUC_OFILE_SERR_STR "stderr"

// update string format flags
// TRNUC_FMT_PRETTY : ASCII, for console
// TRNUC_FMT_CSV    : comma separated value
// TRNUC_FMT_HEX    : hex
// TRNUC_FMT_PRETTY_HEX: formatted hex (w/ offsets)
typedef enum{
    TRNUC_FMT_PRETTY=0X1,
    TRNUC_FMT_CSV=0X2,
    TRNUC_FMT_HEX=0X4,
    TRNUC_FMT_PRETTY_HEX=0X8
}trnuc_fmt_t;

/////////////////////////
// Exports
/////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

    // TODO: add update age threshold/rejection
    // TODO: add UDP buffer configuration
    // TODO: add drain function to empty UDP buffer
    // TODO: add update copy and/or buffering

    // get a new trnu_cli instance
    // caller must release resources using trnucli_destroy
    // fn   : optional update callback (may be NULL)
    // flags: blocking and other flags
    // hbeat_to_sec : heartbeat timeout (s)
    // returns new instance
    trnucli_t *trnucli_new(update_callback_fn update_fn, trnucli_flags_t flags, double hbeat_to_sec);

    // release trnu_cli instance resources
    // pself : pointer to instance pointer
    // return: none, instance pointer set to NULL
    void trnucli_destroy(trnucli_t **pself);

    // connect to trnusvr
    // host: host name or IP address
    // port: port number
    // returns 0 on success, -1 otherwise
    int trnucli_connect(trnucli_t *self, char *host, int port);

    // disconnect from trnusvr
    // returns 0 on success, -1 otherwise
    int trnucli_disconnect(trnucli_t *self);

    // connect to trnumsvr (multicast UDP)
    // host: host name or IP address
    // port: port number
    //  ttl: multicast time-to-live
    // returns 0 on success, -1 otherwise
    int trnucli_mcast_connect(trnucli_t *self, char *host, int port, int ttl);

    // optionally set a callback to be called by listen
    // when an update is received
    // func: optional update callback (may be NULL)
    // returns 0 on success, -1 otherwise
    int trnucli_set_callback(trnucli_t *self, update_callback_fn func);

    // listen for updates, invoke callback if set
    // returns 0 on success (instance update current), -1 otherwise (instance update empty)
    int trnucli_listen(trnucli_t *self, bool callback_en);

    // issue TRN reset request
    // must check subsequent update reinit count to confirm
    // returns 0 on request ACK, -1 otherwise
    int trnucli_reset_trn(trnucli_t *self);

    // issue heartbeat request
    // caller must manage timing
    // returns 0 on request ACK, -1 otherwise
    int trnucli_hbeat(trnucli_t *self);

    // convert update to formatted string
    // caller must free destination
    // dest: pointer to buffer (will dynamically allocate if *dest==NULL)
    // len: size of buffer (<=0 if *dest==NULL)
    // fmt: format flags
    // return length of string or -1 on error
    int trnucli_update_str(trnu_pub_t *self, char **dest, int len, trnuc_fmt_t fmt);

    // get update measurement data timestamp (mb1_time epoch sec)
    double trnucli_update_mb1time(trnu_pub_t *update);

    // get update measurement age (system time - mb1_time, sec)
    double trnucli_update_mb1age(trnu_pub_t *update);

    // get host timestamp (time processed, server host system time, epoch sec)
    double trnucli_update_hosttime(trnu_pub_t *update);

    // get host age (system time - server host system time, sec)
    double trnucli_update_hostage(trnu_pub_t *update);

#ifdef WITH_ASYNC_TRNU

    // get a new trnu_ctx instance
    // caller must release resources using trnucli_ctx_destroy
    // host             : host/multicast group
    // port             : port
    // ttl              : multicast time to live (TTL)
    // ctx_flags        : context option flags (logging, multicast)
    // cli_flags        : client option flags (blocking, multicast)
    // hbeat_to_sec     : heartbeat timeout (s); set <=0 to disable
    // recon_to_sec     : reconnect timeout (if no data available for recon_to_sec)
    // listen_to_ms     : listen timeout (socket SO_RCVTIMEO for blocking reads)
    // erecon_delay_ms  : delay between reconnect attempts (msec)
    // enodata_delay_ms : delay if no data available (msec)
    // update_fn        : optional update callback (NULL to disable)
    // returns new instance
    trnucli_ctx_t *trnucli_ctx_new(char *host,
                                   int port,
                                   int ttl,
                                   trnuctx_flags_t ctx_flags,
                                   trnucli_flags_t cli_flags,
                                   double hbeat_to_sec,
                                   double recon_to_sec,
                                   uint32_t listen_to_ms,
                                   uint32_t enodata_delay_ms,
                                   uint32_t erecon_delay_ms,
                                   update_callback_fn update_fn);

    // release trnu_ctx instance resources
    // pself : pointer to instance pointer
    // return: none, instance pointer set to NULL
    void trnucli_ctx_destroy(trnucli_ctx_t **pself);

    // start async trncli thread
    // return: 0 on success, -1 otherwise
    int trnucli_ctx_start(trnucli_ctx_t *self);

    // stop async trncli thread (disconnects from data host)
    // return: 0 on success, -1 otherwise
    int trnucli_ctx_stop(trnucli_ctx_t *self);

    // set update callback function
    // func : update callback function pointer
    // return: 0 on success, -1 otherwise
    int trnucli_ctx_set_callback(trnucli_ctx_t *self, update_callback_fn func);

    // set period for stats logging (<=0.0 to disable)
    // interval_sec : stats log period (sec)
    // return: 0 on success, -1 otherwise
    int trnucli_ctx_set_stats_log_period(trnucli_ctx_t *self, double interval_sec);

    // copy latest update and (optionally) age (time since arrival, sec)
    //    dest: pointer to destination update structure
    // age_sec: return update age if non-NULL (optional)
    // return: 0 on success, -1 otherwise
    int trnucli_ctx_last_update(trnucli_ctx_t *self, trnu_pub_t *dest, double *age_sec);

    // get arrival time of latest update (epoch sec)
    double trnucli_ctx_update_arrtime(trnucli_ctx_t *self);

    // get arrival age of latest update (time since arrival, sec)
    double trnucli_ctx_update_arrage(trnucli_ctx_t *self);

    // get update measurement data timestamp (mb1_time, epoch sec)
    double trnucli_ctx_update_mb1time(trnucli_ctx_t *self);

    // get update measurement age (system time - mb1_time, sec)
    double trnucli_ctx_update_mb1age(trnucli_ctx_t *self);

    // get host timestamp (time processed, server host system time, epoch sec)
    double trnucli_ctx_update_hosttime(trnucli_ctx_t *self);

    // get host age (sec, system time - server host system time, sec)
    double trnucli_ctx_update_hostage(trnucli_ctx_t *self);

    // get number of updates since last read (trnucli_ctx_last_update)
    uint32_t trnucli_ctx_new_count(trnucli_ctx_t *self);

    // issue TRN update
    // return: 0 on success, -1 otherwise
    int trnucli_ctx_reset_trn(trnucli_ctx_t *self);

    // disconnect and reconnect to server host
    // return: 0 on success, -1 otherwise
    int trnucli_ctx_reconnect(trnucli_ctx_t *self);

    // get current context state
    trnucli_state_t trnucli_ctx_state(trnucli_ctx_t *self);

    // get current context state (as string)
    const char *trnucli_ctx_state_str(trnucli_ctx_t *self);

    // check connection status
    // return: non-zero if connected, 0 otherwise
    int trnucli_ctx_isconnected(trnucli_ctx_t *self);

    // get current context stats
    // dest - pointer to stats buffer reference
    // dynamically allocate if stats buffer reference is NULL - caller must release
    // return: 0 connected, -1 otherwise
    int trnucli_ctx_stats(trnucli_ctx_t *self, trnucli_stats_t **pdest);

    // output summary of context to stderr
    // return: number of bytes written
    int trnucli_ctx_show(trnucli_ctx_t *self,bool verbose, int indent);

    // output summary of context stats to stderr
    // return: number of bytes written
    int trnucli_ctx_stat_show(trnucli_stats_t *self,bool verbose, int indent);

    // log stats
    // return: number of bytes written
    int trnucli_ctx_stat_log(trnucli_ctx_t *self);

#endif

#ifdef __cplusplus
}
#endif

#endif // THIS_FILE
