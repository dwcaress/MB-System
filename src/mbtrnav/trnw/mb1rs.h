///
/// @file mb1rs.h
/// @authors k. headley
/// @date 09 jul 2019

/// MB1 record server (for testing)

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
#ifndef MB1RS_H
#define MB1RS_H

/////////////////////////
// Includes
/////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>
#include <getopt.h>
#include <mthread.h>
#include "mfile.h"
#include "mb1_msg.h"

/////////////////////////
// Macros
/////////////////////////

#define MB1RS_NAME "mb1rs"
#ifndef MB1RS_VER
/// @def MB1RS_VER
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMB1RS_VER=<version>
#define MB1RS_VER (dev)
#endif

#ifndef MB1RS_BUILD
/// @def MB1RS_BUILD
/// @brief MB1RS library build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMB1R_BUILD=`date`
#define MB1RS_BUILD "0000/00/00T00:00:00-0000"
#endif

#define MB1RS_VERSION_STR VERSION_STRING(MB1RS_VER)
#define MB1RS_BUILD_STR VERSION_STRING(MB1RS_BUILD)

/// @def MB1RS_HOST_DFL
/// @brief default host address
# define MB1RS_HOST_DFL "localhost"

/// @def MB1RS_IP_PORT_DFL
/// @brief default IP port
#define MB1RS_IP_PORT_DFL 8000
#define MB1RS_RTO_MS_DFL 3000
#define MB1RS_DEL_MS_DFL 1000

#define MB1RS_SET_MSK(pf,m) if(NULL!=pf){*pf |= m;}
#define MB1RS_CLR_MSK(pf,m) if(NULL!=pf){*pf &= ~(m);}
#define MB1RS_GET_MSK(pf,m) (NULL!=pf ? *pf&m : 0)

/////////////////////////
// Type Definitions
/////////////////////////

typedef unsigned char byte;
typedef struct mb1rs_ctx_s mb1rs_ctx_t;
typedef int64_t (* mbrs_frame_fn)(mb1rs_ctx_t *ctx, byte *dest, uint32_t len);

typedef enum{
    MB1RS_MODE_AUTO=0x1,
    MB1RS_GEN_ERRORS=0x2
}mb1rs_flags_t;

typedef enum{
    MB1RS_ST_STOPPED=0,
    MB1RS_ST_RUNNING,
    MB1RS_ST_COUNT
}mb1rs_state_t;

typedef struct mb1rs_cfg_s{
    /// @var mb1rs_cfg_s::host
    /// @brief host address
    char *host;
    /// @var mb1rs_cfg_s::host
    /// @brief IP port
    int port;
    /// @var mb1rs_cfg_s::ifile
    /// @brief input file
    char *ifile;
    /// @var mb1rs_cfg_s::cycles
    /// @brief cycle limit (<=0 no limit)
    int lim_cyc;
    /// @var mb1rs_cfg_s::lim_retries
    /// @brief retries
    int lim_ret;
    /// @var mb1rs_cfg_s::lim_sec
    /// @brief run time limit
    double lim_sec;
    /// @var mb1rs_cfg_s::err_mod
    /// @brief error modulus (<=0 disable)
    int err_mod;
    /// @var mb1rs_cfg_s::auto_nbeams
    /// @brief number of beams for auto source
    uint32_t auto_nbeams;
    /// @var mb1rs_cfg_s::verbose
    /// @brief verbose output (<=0 disable)
    int verbose;
    /// @var mb1rs_cfg_s::flags
    /// @brief mode flags
    mb1rs_flags_t flags;
    /// @var mb1rs_cfg_s::rto_msec
    /// @brief socket read timeout msec
    uint32_t rto_ms;
    /// @var mb1rs_cfg_s::del_ms
    /// @brief loop delay msec
    uint32_t del_ms;
}mb1rs_cfg_t;

/// @typedef struct mb1rs_s mb1rs_t
/// @brief mb1r server
typedef struct mb1rs_ctx_s
{
    /// @var mb1rs_ctx_s::cfg
    /// @brief server configuration
    mb1rs_cfg_t *cfg;
    /// @var mb1rs_ctx_s::stop_req
    /// @brief thread stop request (!=0)
    mthread_thread_t *worker;
    /// @var mb1rs_ctx_s::frame_func
    /// @brief frame function (get next frame)
    mbrs_frame_fn frame_func;
    /// @var mb1rs_ctx_s::state
    /// @brief server state
    mb1rs_state_t state;
    /// @var mb1rs_ctx_s::stop_req
    /// @brief thread stop request (!=0)
    int stop_req;
    /// @var mb1rs_ctx_s::rfile
    /// @brief input file
    mfile_file_t *rfile;
    /// @var mb1rs_ctx_s::cyc_count
    /// @brief server cycles
    uint32_t cyc_count;
    /// @var mb1rs_ctx_s::ret_count
    /// @brief retry counter
    uint32_t ret_count;
    /// @var mb1rs_ctx_s::err_count
    /// @brief error counter
    uint32_t err_count;
    /// @var mb1rs_ctx_s::tx_count
    /// @brief transmit (reply) counter
    uint32_t tx_count;
    /// @var mb1rs_ctx_s::tx_bytes
    /// @brief transmit (reply) cumulative bytes
    int64_t tx_bytes;
}mb1rs_ctx_t;


/////////////////////////
// Exports
/////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

// mb1rs server API

/// @fn mb1rs_cfg_t *mb1rs_cfg_new()
/// @brief create new mb1rs configuration instance
/// @return new instance reference on success, NULL otherwise.
mb1rs_cfg_t *mb1rs_cfg_new();

/// @fn void mb1rs_cfg_destroy(mb1rs_cfg_t **pself)
/// @brief release instance resources.
/// @param[in] pself pointer to message reference
/// @return none
void mb1rs_cfg_destroy(mb1rs_cfg_t **pself);

/// @fn mb1rs_t *mb1rs_dfl_new())
/// @brief create new mb1rs server instance
/// @return new instance reference on success, NULL otherwise.
mb1rs_ctx_t *mb1rs_dfl_new();

/// @fn mb1rs_t *mb1rs_new(const char *host, int port)
/// @brief create new mb1rs server instance
/// @param[in] cfg configuration instance
/// @return new instance reference on success, NULL otherwise.
mb1rs_ctx_t *mb1rs_new(mb1rs_cfg_t *cfg);

/// @fn void mb1rs_destroy(mb1rs_t **pself)
/// @brief release message structure resources.
/// @param[in] pself pointer to message reference
/// @return none
void mb1rs_destroy(mb1rs_ctx_t **pself);

/// @fn void mb1_show(mb1_t * self, _Bool verbose, uint16_t indent)
/// @brief output mb1 message parameter summary to stderr.
/// @param[in] self mb1 message reference
/// @param[in] verbose use verbose output
/// @param[in] indent output indent spaces
/// @return none
void mb1rs_show(mb1rs_ctx_t *self, bool verbose, uint16_t indent);

/// @fn int mb1rs_start(mb1rs_t **pself)
/// @brief start server thread.
/// @param[in] self instance reference
/// @return none
int mb1rs_start(mb1rs_ctx_t *self);

/// @fn int mb1rs_stop(mb1rs_t **pself)
/// @brief stop server thread.
/// @param[in] self instance reference
/// @return none
int mb1rs_stop(mb1rs_ctx_t *self);

/// @fn void mb1rs_show_app_version(const char *app_name, const char *app_version)
/// @brief show version info (stdout)
/// @param[in] app_name app name string
/// @param[in] app_version app version string
/// @return none
void mb1rs_show_app_version(const char *app_name, const char *app_version);

#ifdef __cplusplus
}
#endif

#endif // THIS_FILE

