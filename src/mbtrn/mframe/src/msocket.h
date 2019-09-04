///
/// @file msocket.h
/// @authors k. headley
/// @date 06 nov 2012
 
/// mframe cross-platform socket IO wrappers
 
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
#ifndef MSOCKET_H
/// @def MSOCKET_H
/// @brief include guard
#define MSOCKET_H

/////////////////////////
// Includes 
/////////////////////////
#include "mframe.h"


/////////////////////////
// Macros
/////////////////////////

#if defined(__unix__) || defined(__APPLE__) || defined(__CYGWIN__)
//#pragma message "Compiling __unix__"
/// @def MSOCK_ADDR_LEN
/// @brief address structure size
#define MSOCK_ADDR_LEN (sizeof(struct sockaddr_in))
#elif defined(__WIN32)
//#pragma message "Compiling __WIN32"
//define something for Windows (32-bit and 64-bit, this part is common)
#ifdef _WIN64
//define something for Windows (64-bit only)
#else
//define something for Windows (32-bit only)
#endif
#endif

/// @def MSOCK_MAX_QUEUE
/// @brief max client connections
#define MSOCK_MAX_QUEUE 8

/// @def MAX_ADDR_BYTES
/// @brief address length
#define MAX_ADDR_BYTES 64
/// @def ADDR_OCTETS
/// @brief number of octets
#define ADDR_OCTETS 4
/// @def PORTSTR_BYTES
/// @brief length or port string
#define PORTSTR_BYTES 16
/// @def ADDRSTR_BYTES
/// @brief address string length
#define ADDRSTR_BYTES 64
/// @def NSEC_PER_SEC
/// @brief time conversion
#ifndef NSEC_PER_SEC
#define NSEC_PER_SEC 1000000000L
#endif
/// @def USEC_PER_SEC
/// @brief time conversion
#ifndef USEC_PER_SEC
#define USEC_PER_SEC 1000000L
#endif
/// @def MSEC_PER_SEC
/// @brief time conversion
#define MSEC_PER_SEC 1000L

/////////////////////////
// Type Definitions
/////////////////////////

/// @struct ip_addr_s
/// @brief IP address structure
struct ip_addr_s;
/// @typedef struct ip_addr_s msock_addr_t
/// @brief IP address typedef
typedef struct ip_addr_s msock_addr_t;

/// @struct msock_socket_s
/// @brief wrapped socket structure
struct msock_socket_s;
/// @typedef struct msock_socket_s msock_socket_t
/// @brief socket struct typedef
typedef struct msock_socket_s msock_socket_t;

/// @struct msock_pstats_s
/// @brief connection statistics
struct msock_pstats_s;
/// @typedef struct msock_pstats_s msock_pstats_t
/// @brief typedef for connection statistics
typedef struct msock_pstats_s msock_pstats_t;

/// @struct msock_connection_s
/// @brief connection (client) connection structure
struct msock_connection_s;
/// @typedef struct msock_connection_s msock_connection_t
/// @brief connection (client) connection typedef
typedef struct msock_connection_s msock_connection_t;

/// @typedef enum msock_socket_ctype msock_socket_ctype
/// @brief socket connection types (TCP, UDP)
typedef enum {ST_TCP=1,ST_UDP}msock_socket_ctype;

/// @typedef enum msock_status_t msock_status_t
/// @brief socket states
typedef enum {
SS_ERROR=-1,
SS_CREATED,
SS_CONFIGURED,
SS_BOUND,
SS_LISTENING,
SS_LISTENOK,
SS_CONNECTED
} msock_status_t;


/// @typedef struct ip_addr_s msock_addr_t
/// @brief IP address structure
struct ip_addr_s
{
    /// @var ip_addr_s::hints
    /// @brief address for opening a socket
    struct addrinfo hints;
    /// @var ip_addr_s::ainfo
    /// @brief active socket address
    struct addrinfo *ainfo;
    /// @var ip_addr_s::alist
    /// @brief list of sockets returned by underlying socket
    struct addrinfo *alist;
    /// @var ip_addr_s::port
    /// @brief IP port as int
    uint16_t port;
    /// @var ip_addr_s::host
    /// @brief socket endpoint
    char *host;
    /// @var ip_addr_s::portstr
    /// @brief IP port as string
    char portstr[PORTSTR_BYTES];
};

/// @struct msock_socket_s
/// @brief wrapped socket
struct msock_socket_s
{
    /// @var msock_socket_s::addr
    /// @brief socket endpoint address
    struct ip_addr_s *addr;
    /// @var msock_socket_s::type
    /// @brief socket type (ST_TCP, ST_UDP)
    int type;
    /// @var msock_socket_s::fd
    /// @brief underlying socket file descriptor
    int fd;
    /// @var msock_socket_s::status
    /// @brief socket status
    /// @sa msocket.h
    int status;
};

/// @struct msock_pstats_s
/// @brief peer connection statistics
struct msock_pstats_s
{
    time_t t_connect;
    time_t t_disconnect;
    uint32_t tx_count;
    uint32_t tx_bytes;
    uint32_t rx_count;
    uint32_t rx_bytes;
    uint32_t hbeats;
    uint32_t err_count;
};

/// @struct msock_connection_s
/// @brief peer connection structure definition
struct msock_connection_s
{

    /// @var msock_connection_s::s
    /// @brief socket (fd wrapper)
    msock_socket_t *sock;
    /// @var msock_connection_s::addr
    /// @brief IP address
    struct ip_addr_s *addr;
    /// @var msock_connection_s::chost
    /// @brief peer hostname
    char chost[NI_MAXHOST];
    /// @var msock_connection_s::service
    /// @brief peer IP port/service (string)
    char service[NI_MAXSERV];
    /// @var msock_connection_s::id
    /// @brief peer port/service (int)
    int id;
    /// @var msock_connection_s::heartbeat
    /// @brief heartbeat value
    /// applications may use to track UDP connection status
    uint16_t heartbeat;
    double hbtime;
    /// @var msock_connection_s::stats
    /// @brief connection statistics
    struct msock_pstats_s stats;
    /// @var msock_connection_s::next
    /// @brief applications may use to form linked lists
    /// @sa mlist.h
    struct msock_connection_s *next;
};

/////////////////////////
// Exports
/////////////////////////
#ifdef __cplusplus
extern "C" {
#endif
    
    // socket API
    
    msock_socket_t *msock_socket_new(const char *host, int port, msock_socket_ctype type);
    void msock_socket_destroy(msock_socket_t **pself);
    // pass in NULL socket to create dynamically
    int msock_configure(msock_socket_t *s, const char *host, int port, msock_socket_ctype type);
    int msock_set_blocking(msock_socket_t *s, bool enabled);
    int msock_bind(msock_socket_t *s);
    int msock_connect(msock_socket_t *s);
    int msock_listen(msock_socket_t *s, int queue);
    int msock_accept(msock_socket_t *s, msock_addr_t *addr);
    
    int64_t msock_send(msock_socket_t *s,byte *buf, uint32_t len);
    int64_t msock_recv(msock_socket_t *s, byte *buf, uint32_t len,int flags);
    int64_t msock_sendto(msock_socket_t *s, msock_addr_t *addr, byte *buf, uint32_t len, int32_t flags);
    int64_t msock_recvfrom(msock_socket_t *s, msock_addr_t *addr, byte *buf, uint32_t len,int flags);
    int64_t msock_read_tmout(msock_socket_t *s, byte *buf, uint32_t len, uint32_t timeout_msec);
    char *msock_addr2str(msock_socket_t *s, char *dest, size_t len);
    int msock_close(msock_socket_t *self);
    msock_socket_t *msock_wrap_fd(int fd);
    
    msock_addr_t *msock_addr_new();
    void msock_addr_destroy(msock_addr_t **pself);
    void msock_addr_init(msock_addr_t *self);
    msock_connection_t *msock_connection_new();
    void msock_connection_destroy(msock_connection_t **pself);
    void msock_connection_free(void *pself);
    
    int msock_connection_addr2str(msock_connection_t *self);
    
    void msock_pstats_show(msock_pstats_t *self, bool verbose, uint16_t indent);
    
    void msock_set_debug(int level);
    
    int msock_test();
    
#ifdef __cplusplus
}
#endif

// include guard
#endif
