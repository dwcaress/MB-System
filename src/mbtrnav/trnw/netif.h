///
/// @file netif.h
/// @authors k. headley
/// @date 2019-06-21

/// TRN net interface API

/// @sa doxygen-examples.c for more examples of Doxygen markup


/////////////////////////
// Terms of use
/////////////////////////
/*
 Copyright Information

 Copyright 2002-2019 MBARI
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
#ifndef NETIF_H
#define NETIF_H

/////////////////////////
// Includes
/////////////////////////
#include "mframe.h"
#include "msocket.h"
#include "mlist.h"
#include "mlog.h"
#include "mstats.h"
#include "mmdebug.h"

/////////////////////////
// Macros
/////////////////////////
#ifndef NETIF_VER
/// @def NETIF_VER
/// @brief NETIF library build version.
#define NETIF_VER "1.0.0"
#endif
#ifndef NETIF_BUILD
/// @def NETIF_BUILD
/// @brief NETIF library build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DNETIF_BUILD=`date`
#define NETIF_BUILD "0000/00/00T00:00:00-0000"
#endif

/// @def VERSION_HELPER
/// @brief version string helper.
#define VERSION_HELPER(s) #s
/// @def VERSION_STRING
/// @brief version string macro.
#define VERSION_STRING(s) VERSION_HELPER(s)
/// @def LIBNETIF_VERSION
/// @brief library version string macro.
#define LIBNETIF_VERSION ""VERSION_STRING(NETIF_VER)
/// @def LIBNETIF_BUILD
/// @brief library version build date string macro.
#define LIBNETIF_BUILD ""VERSION_STRING(NETIF_BUILD)

#define WITH_NETIF_TEST

/////////////////////////
// Type Definitions
/////////////////////////

#ifdef NETIF_MMDEBUG
// definitions are used when not
// defined in mconfig.h (i.e. by application)

typedef enum{
    MOD_NETIF=MM_MODULE_COUNT,
    APP_MODULE_COUNT
}app_module_ids;

/// @enum netif_channel_id
/// @brief test module channel IDs
/// [note : starting above reserved mframe channel IDs]
typedef enum{
    ID_NETIF_V1=MM_CHANNEL_COUNT,
    ID_NETIF_V2,
    ID_NETIF_V3,
    ID_NETIF_V4,
    NETIF_CHAN_COUNT
}netif_channel_id;

/// @enum netif_channel_mask
/// @brief test module channel masks
typedef enum{
    NETIF_V1= (1<<ID_NETIF_V1),
    NETIF_V2= (1<<ID_NETIF_V2),
    NETIF_V3= (1<<ID_NETIF_V3),
    NETIF_V4= (1<<ID_NETIF_V4)
}netif_channel_mask;

#endif  // NETIF_MMDEBUG

typedef enum{
    NETIF_EV_ESRC_SOCKET=0,
    NETIF_EV_ESRC_CON,
    NETIF_EV_ECLI_RXZ,
    NETIF_EV_ECLI_RXE,
    NETIF_EV_EAGAIN,
    NETIF_EV_ECLI_TXZ,
    NETIF_EV_ECLI_TXE,
    NETIF_EV_EPUB_TX,
    NETIF_EV_EPROTO_RD,
    NETIF_EV_EPROTO_HND,
    NETIF_EV_CLI_CONN,
    NETIF_EV_CLI_DISN,
    NETIF_EV_CLI_RXN,
    NETIF_EV_CLI_TXN,
    NETIF_EV_CLI_REQRESN,
    NETIF_EV_CLI_PUBN,
    NETIF_EV_COUNT
}prof_event_id;

typedef enum{
    NETIF_STA_CLI_LIST_LEN=0,
    NETIF_STA_CLI_RX_BYTES,
    NETIF_STA_CLI_TX_BYTES,
    NETIF_STA_CLI_RES_BYTES,
    NETIF_STA_CLI_PUB_BYTES,
    NETIF_STA_COUNT
}prof_status_id;

typedef enum{
    NETIF_CH_UDCON_XT=0,
    NETIF_CH_CHKHB_XT,
    NETIF_CH_READ_XT,
    NETIF_CH_HANDLE_XT,
    NETIF_CH_REQRES_XT,
    NETIF_CH_PUB_XT,
    NETIF_CH_COUNT
}prof_chan_id;

#define NETIF_MLOG_NAME "netif"
#define NETIF_MLOG_DESC "netif message log"
#define NETIF_LOG_EXT ".log"
#define NETIF_LOG_DIR_DFL "."
#define NETIF_LOG_PATH_BYTES 512
#define NETIF_PORT_DFL 8000
#define NETIF_HOST_DFL  "localhost"
#define NETIF_MSG_CON_LEN 4
#define NETIF_MSG_HDR_LEN 4
#define NETIF_QUEUE_DFL 8
#define NETIF_HBTO_DFL (double)5.0
#define NETIF_UDP_BUF_LEN 128

struct netif_s;
typedef struct netif_s netif_t;

typedef int (* netifserver_action_fn)(netif_t *self);
typedef int (* netif_msg_read_fn)(byte **dest, uint32_t *len, netif_t *self, msock_connection_t *peer, int *errout);
typedef int (* netif_msg_handle_fn)(void *msg, netif_t *self, msock_connection_t *peer, int *errout);
typedef int (* netif_msg_pub_fn)(netif_t *self, msock_connection_t *peer, char *data, size_t len);

// modes (request/response, pub only)
typedef enum {IFM_REQRES,IFM_PUB} netif_mode_t;

struct netif_s{
    int port;
    msock_socket_t *socket;
    msock_connection_t *peer;
    mlist_t *list;
    netif_msg_read_fn read_fn;
    netif_msg_handle_fn handle_fn;
    netif_msg_pub_fn pub_fn;
    double hbto;
    int ttl;
    netif_mode_t mode;
    mstats_profile_t *profile;
    mlog_id_t mlog_id;
    bool stop;
    msock_socket_ctype ctype;
    void *rr_res;
    void *pub_res;
    char *host;
    char *mlog_path;
    char *log_dir;
    char *cmdline;
    char *port_name;
};

/////////////////////////
// Exports
/////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

    /// @fn netif_t *netif_new()
    /// @brief create a new netif instance.
    /// user owns, must release resources using netif_destroy
    /// @param[in] host hostname
    /// @param[in] port IP port
    /// @param[in] ctype socket type (ST_TCP, ST_UDP)
    /// @param[in] mode IFM_REQRES, IFM_PUB
    /// @param[in] hbto heartbeat timeout (no timeout if <=0)
    /// @param[in] read_fn read function
    /// @param[in] handle_fn message handler
    /// @param[in] pub_fn publish function
    /// @return new instance on success, NULL otherwise
    netif_t *netif_new(char *name, char *host, int port,
                       msock_socket_ctype ctype,
                       netif_mode_t mode,
                       double hbto,
                       netif_msg_read_fn read_fn,
                       netif_msg_handle_fn handle_fn,
                       netif_msg_pub_fn pub_fn);

    netif_t *netif_mcast_new(char *name, char *host, int port,
                             msock_socket_ctype ctype,
                             netif_mode_t mode,
                             int ttl,
                             netif_msg_read_fn read_fn,
                             netif_msg_handle_fn handle_fn,
                             netif_msg_pub_fn pub_fn);

    netif_t *netif_tcp_new(char *name, char *host, int port,
                           double hbto,
                           netif_mode_t mode,
                           netif_msg_read_fn reader,
                           netif_msg_handle_fn handler);

    netif_t *netif_udp_new(char *name, char *host, int port,
                           double hbto,
                           netif_mode_t mode,
                           netif_msg_read_fn reader,
                           netif_msg_handle_fn handler);

    /// @fn void netif_destroy(netif_t **pself)
    /// @brief release netif_t resources
    /// @param[in] pself pointer to instance
    /// @return none
    void netif_destroy(netif_t **pself);

    /// @fn void netif_show(netif_t *pself, bool verbose, int indent)
    /// @brief print instance summary to console
    /// @param[in] pself pointer to instance
    /// @param[in] verbose enable verbose output
    /// @param[in] indent number of spaces to indent output
    /// @return none
    void netif_show(netif_t *self, bool verbose, int indent);

    /// @fn int netif_start(netif_t *self)
    /// @brief start netif (req/res mode)
    /// @param[in] self netif_t instance
    /// @param[in] delay_msec delay between cycles
    /// @return 0 on success, -1 otherwise
    int netif_start(netif_t *self, uint32_t delay_msec);

    /// @fn int netif_restart(netif_t *self)
    /// @brief restart netif (req/res mode)
    /// @param[in] self netif_t instance
    /// @return 0 on success, -1 otherwise
    int netif_restart(netif_t *self);

    /// @fn netif_stop(netif_t *self, int sig)
    /// @brief stop netif (req/res mode)
    /// @param[in] self netif_t instance
    /// @param[in] sig signal number (if stopped by signal)
    /// @return 0 on success, -1 otherwise
    int netif_stop(netif_t *self, int sig);

    /// @fn int netif_init_log(netif_t *self, char *log_dir)
    /// @brief initialize message log
    /// @param[in] self netif_t instance
    /// @param[in] log_dir log output directory path
    /// @return 0 on success, -1 otherwise
    int netif_init_log(netif_t *self, char *log_name, char *log_dir, char *session_str);

    /// @fn int netif_configure_debug(netif_t *self, int level)
    /// @brief set debug configuration
    /// @param[in] self netif_t instance
    /// @param[in] level output level (0-5)
    /// @return 0 on success, -1 otherwise
    int netif_configure_debug(netif_t *self, int level);

    /// @fn int netif_test()
    /// @brief unit test
    /// @param[in]
    /// @return 0 on success, -1 otherwise
    int netif_test();

    void netif_set_reqres_res(netif_t *self, void *res);
    void netif_set_pub_res(netif_t *self, void *res);

    int netif_connect(netif_t *self);
    int netif_update_connections(netif_t *self);
    int netif_connections(netif_t *self);

    int netif_reqres(netif_t *self);
    int netif_pub(netif_t *self, char *output_buffer, size_t len);
    /// @fn const char *netif_get_version()
    /// @brief get build string.
    /// @return version string
    const char *netif_get_version();

    /// @fn const char *netif_get_build()
    /// @brief get build string.
    /// @return version string
    const char *netif_get_build();

    void netif_init_mmd();

    mstats_t *netif_stats(netif_t *self);
    mlog_id_t netif_log(netif_t *self);

#ifdef __cplusplus
}
#endif

#endif // THIS_FILE
