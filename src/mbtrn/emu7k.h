///
/// @file emu7k2.h
/// @authors k. headley
/// @date 06 nov 2012
 
/// 7k Center emulation
/// Reads MB data from a file and writes
/// it to a socket (e.g. emulates reson 7k center source)
 
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
#ifndef EMU7K_H
/// @def EMU7K_H
/// @brief include guard
#define EMU7K_H

/////////////////////////
// Includes 
/////////////////////////

#include <pthread.h>
#include "iowrap.h"
#include "r7kc.h"
#include "mbtrn.h"
#include "mlist.h"
#include "mconfig.h"

/////////////////////////
// Type Definitions
/////////////////////////

/// @typedef struct emu7k_stats_s emu7k_stats_t
/// @brief server stat
typedef struct emu7k_stats_s{
    /// @var emu7k_stats_s::start_time
    /// @brief start time
    time_t start_time;
    /// @var emu7k_stats_s::con_total
    /// @brief total client connections
    uint64_t con_total;
    /// @var emu7k_stats_s::con_active
    /// @brief number of active connections
    uint64_t con_active;
    /// @var emu7k_stats_s::cyc_total
    /// @brief number of cycles through the file
    uint64_t cyc_total;
    /// @var emu7k_stats_s::rec_total
    /// @brief number of records read since start
    uint64_t rec_total;
    /// @var emu7k_stats_s::pub_total
    /// @brief number of records published since start
    uint64_t pub_total;
    /// @var emu7k_stats_s::rec_cycle
    /// @brief number records read in current cycle
    uint64_t rec_cycle;
    /// @var emu7k_stats_s::pub_cycle
    /// @brief number records published in current cycle
    uint64_t pub_cycle;
}emu7k_stat_t;

/// @typedef struct app_cfg_s app_cfg_t
/// @brief application configuration parameter structure
typedef struct app_cfg_s{
    /// @var app_cfg_s::verbose
    /// @brief verbose output flag
    int verbose;
    /// @var app_cfg_s::ifile
    /// @brief data source (s7k) file
    char *file_path;
    /// @var app_cfg_s::host
    /// @brief server host
    char *host;
    /// @var app_cfg_s::port
    /// @brief server port
    int port;
    /// @var app_cfg_s::min_delay
    /// @brief minimum publish delay
    int32_t min_delay;
    /// @var app_cfg_s::restart
    /// @brief restart at file end
    bool restart;
    /// @var app_cfg_s::statn
    /// @brief stat report interval (records)
    uint32_t statn;
    /// @var app_cfg_s::xdt
    /// @brief delay every xdt sec for xds sec
    time_t xdt;
    /// @var app_cfg_s::xdstart
    /// @brief delay every xdt sec for xds sec
    time_t xdstart;
    /// @var app_cfg_s::xds
    /// @brief delay every xdn records for xds sec
    int xds;
    /// @var app_cfg_s::netframe_input
    /// @brief input file contains netframes
    bool netframe_input;
    /// @var app_cfg_s::file_list
    /// @brief data source file list
    mlist_t *file_paths;
}app_cfg_t;

/// @typedef struct emu7k_record_s emu7k_record_t
/// @brief record structure
typedef struct emu7k_record_s
{
    int64_t head;
    int64_t tail;
    int64_t data_len;
    int32_t rtype;
    double time;
    byte *header;
    byte *data;
}emu7k_record_t;

/// @typedef struct emu7k_client_s emu7k_client_t
/// @brief server client structure
typedef struct emu7k_client_s
{
    /// @var emu7k_client_s::sock_if
    /// @brief connection socket wrapper
    iow_socket_t *sock_if;
    /// @var emu7k_client_s::fd
    /// @brief connection file descriptor
    int fd;
    /// @var emu7k_client_s::sub_count
    /// @brief number of subscriptions
    uint32_t sub_count;
    /// @var emu7k_client_s::sub_list
    /// @brief subscription list
    int32_t *sub_list;
}emu7k_client_t;

/// @typedef struct emu7k_s emu7k_t
/// @brief test server structure
typedef struct emu7k_s
{
    /// @var emu7k_s::sock_if
    /// @brief socket interface
    iow_socket_t *sock_if;
    /// @var emu7k_s::t
    /// @brief server thread
    iow_thread_t *t;
    /// @var emu7k_s::w
    /// @brief worker thread
    iow_thread_t *w;
    /// @var emu7k_s::reader
    /// @brief s7k stream reader
    mbtrn_reader_t *reader;
    /// @var emu7k_s::max_clients
    /// @brief max allowed client connections
    uint32_t max_clients;
    /// @var emu7k_s::client_count
    /// @brief current number of client connections
    uint32_t client_count;
    /// @var emu7k_s::client_list
    /// @brief list of client connections
    mlist_t *client_list;
    /// @var emu7k_s::auto_free
    /// @brief autofree file/socket resources
    bool auto_free;
    /// @var emu7k_s::stop
    /// @brief stop flag (allows caller to stop server thread)
    bool stop;
    /// @var emu7k_s::stats
    /// @brief server statistics
    emu7k_stat_t stats;
    /// @var emu7k_s::cfg
    /// @brief application config
    app_cfg_t *cfg;
    /// @var emu7k_s::file_list
    /// @brief source file list
    mlist_t *file_list;
}emu7k_t;

/////////////////////////
// Macros
/////////////////////////
/// @def MAX_DELAY_DFL_SEC
/// @brief max inter-packet delay
#define MAX_DELAY_DFL_SEC ((double)3.0)

/// @def MIN_DELAY_DFL_SEC
/// @brief min inter-packet delay
#define MIN_DELAY_DFL_MSEC 0

/// @def STATN_DFL_RECORDS
/// @brief default stats output interval
#define STATN_DFL_REC 2000

/// @def RESTART_DFL
/// @brief default restart enable
#define RESTART_DFL false

/// @def VERBOSE_OUTPUT_DFL
/// @brief default verbose output enable
#define VERBOSE_OUTPUT_DFL 0

/// @def EMU_HOST_DFL
/// @brief default verbose output enable
#define EMU_HOST_DFL "localhost"

/// @def EMU_PORT_DFL
/// @brief default verbose output enable
#define EMU_PORT_DFL R7K_7KCENTER_PORT

/// @def REQ_TEST_REQ
/// @brief protocol request data
#define REQ_TEST_REQ "REQ"
/// @def REQ_SERVER_STOP
/// @brief protocol server stop
#define REQ_SERVER_STOP "STOP"
/// @def REQ_SERVER_SUB
/// @brief protocol subscribe
#define REQ_SERVER_SUB "SUB"
/// @typedef enum server_req_id server_req_id
/// @brief TBD
typedef enum {REQ=1,SUB,STOP}server_req_id;

/////////////////////////
// Exports
/////////////////////////

emu7k_client_t *emu7k_client_new(int fd, uint32_t nsubs, int32_t *subs);
void emu7k_client_destroy(emu7k_client_t **pself);

emu7k_t *emu7k_new(iow_socket_t *s, iow_file_t *mb_data, app_cfg_t *cfg);
emu7k_t *emu7k_lnew(iow_socket_t *s, mlist_t *path_list, app_cfg_t *cfg);
void emu7k_destroy(emu7k_t **pself);

void emu7k_show(emu7k_t *self, bool verbose, uint16_t indent);
void emu7k_stat_show(emu7k_stat_t *self, bool verbose, uint16_t indent);

// start test server to emulate reson
// using data from file
int emu7k_start(emu7k_t *self);
int emu7k_stop(emu7k_t *self);

// include guard
#endif