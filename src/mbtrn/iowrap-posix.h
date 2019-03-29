///
/// @file iowrap-posix.h
/// @authors k. headley
/// @date 06 nov 2012
 
/// MBRTN platform-dependent IO wrappers implementation
/// for *nix/Cygwin

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
#ifndef IOW_POSIX_H
/// @def IOW_POSIX_H
/// @brief TBD
#define IOW_POSIX_H

/////////////////////////
// Includes 
/////////////////////////

#include <sys/types.h>
#ifdef _WIN32
#	include <winsock2.h>
#else
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <netdb.h>
#endif
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>


#ifdef _WIN32
#	include <sys/stat.h>
#   include <io.h>

typedef int mode_t;

/// @Note If STRICT_UGO_PERMISSIONS is not defined, then setting Read for any
///       of User, Group, or Other will set Read for User and setting Write
///       will set Write for User.  Otherwise, Read and Write for Group and
///       Other are ignored.
///
/// @Note For the POSIX modes that do not have a Windows equivalent, the modes
///       defined here use the POSIX values left shifted 16 bits.

static const mode_t S_ISUID      = 0x08000000;           ///< does nothing
static const mode_t S_ISGID      = 0x04000000;           ///< does nothing
static const mode_t S_ISVTX      = 0x02000000;           ///< does nothing
static const mode_t S_IRUSR      = (mode_t)(_S_IREAD);     ///< read by user
static const mode_t S_IWUSR      = (mode_t)(_S_IWRITE);    ///< write by user
static const mode_t S_IXUSR      = 0x00400000;           ///< does nothing
#   ifndef STRICT_UGO_PERMISSIONS
static const mode_t S_IRGRP      = (mode_t)(_S_IREAD);     ///< read by *USER*
static const mode_t S_IWGRP      = (mode_t)(_S_IWRITE);    ///< write by *USER*
static const mode_t S_IXGRP      = 0x00080000;           ///< does nothing
static const mode_t S_IROTH      = (mode_t)(_S_IREAD);     ///< read by *USER*
static const mode_t S_IWOTH      = (mode_t)(_S_IWRITE);    ///< write by *USER*
static const mode_t S_IXOTH      = 0x00010000;           ///< does nothing
#   else
static const mode_t S_IRGRP      = 0x00200000;           ///< does nothing
static const mode_t S_IWGRP      = 0x00100000;           ///< does nothing
static const mode_t S_IXGRP      = 0x00080000;           ///< does nothing
static const mode_t S_IROTH      = 0x00040000;           ///< does nothing
static const mode_t S_IWOTH      = 0x00020000;           ///< does nothing
static const mode_t S_IXOTH      = 0x00010000;           ///< does nothing
#   endif
static const mode_t MS_MODE_MASK = 0x0000ffff;           ///< low word

#if !defined (S_IRWXU)
#	define S_IRWXU 00700         /* read, write, execute: owner. */
#endif /* !S_IRWXU */
#if !defined (S_IRWXG)
#	define S_IRWXG 00070
#endif /* S_IRWXG */
#if !defined (S_IRWXO)
#	define S_IRWXO 00007
#endif /* S_IRWXO */

#endif


/////////////////////////
// Type Definitions
/////////////////////////

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

/// @typedef struct ip_addr_s iow_addr_t
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

/// @struct iow_socket_s
/// @brief wrapped socket
struct iow_socket_s
{
    /// @var iow_socket_s::addr
    /// @brief socket endpoint address
    struct ip_addr_s *addr;
    /// @var iow_socket_s::type
    /// @brief socket type (ST_TCP, ST_UDP)
    int type;
    /// @var iow_socket_s::fd
    /// @brief underlying socket file descriptor
    int fd;
    /// @var iow_socket_s::qlen
    /// @brief number of clients
    uint16_t qlen;
    /// @var iow_socket_s::status
    /// @brief socket status
    /// @sa iowrap.h
    int status;
};

/// @struct iow_pstats_s
/// @brief peer connection statistics
struct iow_pstats_s
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

/// @struct iow_peer_s
/// @brief peer connection structure definition
struct iow_peer_s
{
    /// @var iow_peer_s::addr
    /// @brief IP address
    struct ip_addr_s *addr;
    /// @var iow_peer_s::chost
    /// @brief peer hostname
    char chost[NI_MAXHOST];
    /// @var iow_peer_s::service
    /// @brief peer IP port/service (string)
    char service[NI_MAXSERV];
    /// @var iow_peer_s::id
    /// @brief peer port/service (int)
    int id;
    /// @var iow_peer_s::heartbeat
    /// @brief heartbeat value
    /// applications may use to track UDP connection status
    uint16_t heartbeat;
    /// @var iow_peer_s::stats
    /// @brief connection statistics
    struct iow_pstats_s stats;
    /// @var iow_peer_s::next
    /// @brief applications may use to form linked lists
    /// @sa mlist.h
    struct iow_peer_s *next;
};

/// @struct iow_file_s
/// @brief wrapped file representation (posix implementation)
struct iow_file_s
{
    /// @var iow_file_s::path
    /// @brief file path
    char *path;
    /// @var iow_file_s::fd
    /// @brief file descriptor
    int fd;
    /// @var iow_file_s::flags
    /// @brief file attribute flags
    int flags;
    /// @var iow_file_s::mode
    /// @brief file permissions flags
    mode_t mode;
};

/// @typedef struct iow_thread_s iow_thread_t
/// @brief wrapped thread representation (posix implementation)
struct iow_thread_s
{
    /// @var iow_thread_s::t
    /// @brief posix thread
    pthread_t t;
    /// @var iow_thread_s::attr
    /// @brief thread attributes
    pthread_attr_t attr;
    /// @var iow_thread_s::status
    /// @brief thread exit status
    void **status;
};

/// @typedef struct iow_mutex_s iow_mutex_t
/// @brief wrapped mutex representation (posix implementation)
struct iow_mutex_s
{
    /// @var iow_mutex_s::m
    /// @brief posix mutex
    pthread_mutex_t m;
};

/////////////////////////
// Macros
/////////////////////////

/////////////////////////
// Exports
/////////////////////////

// include guard
#endif
