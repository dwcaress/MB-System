///
/// @file iowrap.h
/// @authors k. headley
/// @date 06 nov 2012
 
/// MBRT platform-independent IO wrappers
 
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
#ifndef IOWRAP_H
/// @def IOWRAP_H
/// @brief include guard
#define IOWRAP_H

/////////////////////////
// Includes 
/////////////////////////

#if defined(__unix__) || defined(__APPLE__) || defined(__CYGWIN__)
#include <stdint.h>
#include <stdbool.h>
#include "iowrap-posix.h"
#include "merror.h"
/// @def IOW_ADDR_LEN
/// @brief address structure size
#define IOW_ADDR_LEN sizeof(struct sockaddr_in)
//#pragma message "Compiling __unix__"
#if defined(__CYGWIN__)
//#pragma message "__CYGWIN__ is defined"
#define WIN_DECLSPEC __declspec(dllimport)
#else
/// @def WIN_DECLSPEC
/// @brief TBD
#define WIN_DECLSPEC
#endif
#elif defined(__WIN32)
//#pragma message "Compiling __WIN32"
//define something for Windows (32-bit and 64-bit, this part is common)
#include "iowrap-win.h"
#ifdef _WIN64
//define something for Windows (64-bit only)
#else
//define something for Windows (32-bit only)
#endif
#endif

/////////////////////////
// Type Definitions
/////////////////////////

/// @def MAX_ADDR_BYTES
/// @brief address length
#define MAX_ADDR_BYTES 64
/// @def ADDR_OCTETS
/// @brief address octets
#define ADDR_OCTETS 4

/// @struct ip_addr_s
/// @brief IP address structure
struct ip_addr_s;
/// @typedef struct ip_addr_s iow_addr_t
/// @brief IP address typedef
typedef struct ip_addr_s iow_addr_t;

/// @struct iow_socket_s
/// @brief wrapped socket structure
struct iow_socket_s;
/// @typedef struct iow_socket_s iow_socket_t
/// @brief socket struct typedef
typedef struct iow_socket_s iow_socket_t;

/// @struct iow_pstats_s
/// @brief peer connection statistics
struct iow_pstats_s;
/// @typedef struct iow_pstats_s iow_pstats_t
/// @brief typedef for peer
typedef struct iow_pstats_s iow_pstats_t;

/// @struct iow_peer_s
/// @brief peer (client) connection structure
struct iow_peer_s;
/// @typedef struct iow_peer_s iow_peer_t
/// @brief peer (client) connection typedef
typedef struct iow_peer_s iow_peer_t;

/// @struct iow_file_s
/// @brief wrapped file representation (posix implementation)
struct iow_file_s;
/// @typedef struct iow_file_s iow_file_t
/// @brief wrapped file typedef
typedef struct iow_file_s iow_file_t;

/// @typedef uint8_t byte
/// @brief typedef for byte
typedef uint8_t byte;

/// @struct iow_thread_s
/// @brief wrapped thread representation (posix implementation)
struct iow_thread_s;
/// @typedef struct iow_thread_s iow_thread_t
/// @brief wrapped thread typedef
typedef struct iow_thread_s iow_thread_t;

/// @struct iow_mutex_s
/// @brief wrapped mutex representation (posix implementation)
struct iow_mutex_s;
/// @typedef struct iow_mutex_s iow_mutex_t
/// @brief wrapped mutex typedef
typedef struct iow_mutex_s iow_mutex_t;

/// @typedef void *(*)(void *) mbtrn_thread_fn
/// @brief thread function
typedef void *(* mbtrn_thread_fn)(void *arg);

/// @typedef enum mbtrn_ctype mbtrn_ctype
/// @brief connection endpoint types
typedef enum {CT_NULL,CT_STDIN,CT_STDOUT,CT_STDERR,CT_FILE,CT_SOCKET} mbtrn_ctype;

/// @typedef enum mbtrn_socket_state mbtrn_socket_state
/// @brief socket states
typedef enum {SS_ERROR=-1,SS_CREATED,SS_CONFIGURED, SS_BOUND, SS_LISTENING, SS_LISTENOK,SS_CONNECTED,} mbtrn_socket_state;

/// @typedef enum mbtrn_status_id mbtrn_status_id
/// @brief connection status
typedef enum {IO_OK=0,IO_ETMOUT,IO_ERCV,IO_ESEL,IO_ESOCK, IO_EINC} mbtrn_status_id;

/// @typedef enum iow_socket_type iow_socket_type
/// @brief socket connection types (TCP, UDP)
typedef enum {ST_TCP=0,ST_UDP}iow_socket_type;

/// @typedef enum iow_flags_t iow_flags_t
/// @brief file attribute flags
typedef enum {IOW_RONLY=0x1,IOW_WONLY=0x2,IOW_RDWR=0x4,IOW_APPEND=0x8,IOW_CREATE=0x10,IOW_TRUNC=0x20,IOW_NONBLOCK=0x40} iow_flags_t;

/// @typedef enum iow_mode_t iow_mode_t
/// @brief file permission flags
typedef enum{IOW_RWXU=0x800,IOW_RU=0x400,IOW_WU=0x200,IOW_XU=0x100,IOW_RWXG=0x80,IOW_RG=0x40,IOW_WG=0x20,IOW_XG=0x10,IOW_RWXO=0x8,IOW_RO=0x4,IOW_WO=0x2,IOW_XO=0x1} iow_mode_t;

/// @typedef enum iow_mode_t iow_whence_t
/// @brief file seek flags
typedef enum{IOW_SET=0, IOW_CUR, IOW_END} iow_whence_t;

/////////////////////////
// Macros
/////////////////////////

/////////////////////////
// Exports
/////////////////////////

// iow socket API

iow_socket_t *iow_socket_new(const char *host, int port, iow_socket_type type);
void iow_socket_destroy(iow_socket_t **pself);
iow_addr_t *iow_addr_new();
void iow_addr_destroy(iow_addr_t **pself);
void iow_addr_init(iow_addr_t *self);
iow_peer_t *iow_peer_new();
void iow_peer_destroy(iow_peer_t **pself);
void iow_peer_free(void *pself);
void iow_pstats_show(iow_pstats_t *self, bool verbose, uint16_t indent);


// pass in NULL socket to create dynamically

int iow_configure(iow_socket_t *s, const char *host, int port, iow_socket_type type, uint16_t qlen);
iow_socket_t *iow_wrap_fd(int fd);

int iow_bind(iow_socket_t *s);
int iow_connect(iow_socket_t *s);
int iow_listen(iow_socket_t *s);
int64_t iow_send(iow_socket_t *s,byte *buf, uint32_t len);
int64_t iow_sendto(iow_socket_t *s, iow_addr_t *peer, byte *buf, uint32_t len);
int64_t iow_recv(iow_socket_t *s, byte *buf, uint32_t len);
int64_t iow_recvfrom(iow_socket_t *s, iow_addr_t *peer, byte *buf, uint32_t len);
int64_t iow_read_tmout(iow_socket_t *s, byte *buf, uint32_t len, uint32_t timeout_msec);
int iow_set_blocking(iow_socket_t *s, bool enabled);
char *iow_addr2str(iow_socket_t *s, char *dest, size_t len);

// iow file API

iow_file_t *iow_file_new(const char *path);
void iow_file_destroy(iow_file_t **pself);
void iow_file_show(iow_file_t *self, bool verbose, uint16_t indent);

// open file, return fd

int iow_open(iow_file_t *self,iow_flags_t flags);
int iow_close(iow_file_t *self);
int iow_mopen(iow_file_t *self,iow_flags_t flags,iow_mode_t mode );
int64_t iow_seek(iow_file_t *self, uint32_t ofs, iow_whence_t whence);
int64_t iow_read(iow_file_t *self, byte *dest, uint32_t len);
int64_t iow_write(iow_file_t *self, byte *src, uint32_t len);
int iow_ftruncate(iow_file_t *self, uint32_t len);
int iow_fprintf(iow_file_t *self, char *fmt, ...);
int iow_vfprintf(iow_file_t *self, char *fmt, va_list args);
int iow_flush(iow_file_t *self);
int64_t iow_fsize(iow_file_t *self);
time_t iow_mtime(const char *path);
int iow_rename(iow_file_t *self,const char *path);

// iow thread API

iow_thread_t *iow_thread_new();
void iow_thread_destroy(iow_thread_t **pself);

int iow_thread_start(iow_thread_t *thread, mbtrn_thread_fn func, void *arg);
int iow_thread_join(iow_thread_t *thread);

// iow mutex API

iow_mutex_t *iow_mutex_new();
void iow_mutex_destroy(iow_mutex_t **pself);
int iow_mutex_lock(iow_mutex_t *self);
int iow_mutex_unlock(iow_mutex_t *self);

#ifdef WITH_TEST
int iow_test();
void *iow_test_svr(void *);
#endif

// include guard
#endif