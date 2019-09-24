///
/// @file mframe.h
/// @authors k. headley
/// @date 11 aug 2017

/// Common headers and definitions

/// @sa doxygen-examples.c for more examples of Doxygen markup


/////////////////////////
// Terms of use
/////////////////////////
/*
 Copyright Information
 
 qframe - simple framework for building command line utilities
 
 Copyright 2002-2017 MBARI
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
#ifndef MFRAME_H
#define MFRAME_H

/////////////////////////
// Includes 
/////////////////////////
#ifndef MFRAME_VER
/// @def MFRAME_VER
/// @brief MFRAME library build version.
#define MFRAME_VER 1.4.5
#endif
#ifndef MFRAME_BUILD
/// @def MFRAME_BUILD
/// @brief MFRAME library build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMFRAME_BUILD=`date`
#define MFRAME_BUILD "0000/00/00T00:00:00-0000"
#endif

/// @def VERSION_HELPER
/// @brief version string helper.
#define VERSION_HELPER(s) #s
/// @def VERSION_STRING
/// @brief version string macro.
#define VERSION_STRING(s) VERSION_HELPER(s)
/// @def LIBR7KR_VERSION
/// @brief library version string macro.
#define LIBMFRAME_VERSION ""VERSION_STRING(MFRAME_VER)
/// @def LIBR7KR_BUILD
/// @brief library version build date string macro.
#define LIBMFRAME_BUILD ""VERSION_STRING(MFRAME_BUILD)

/// @def MFRAME_SHOW_VERSION(app_name,app_version)
/// @param[in] app_name application name
/// @param[in] app_version application version
/// @brief print version info
#define MFRAME_SHOW_VERSION(app_name,app_version) printf("\n %s build[%s] mframe[v%s] \n\n",app_name, app_version, LIBMFRAME_VERSION)

#if defined(__CYGWIN__)
#include <Windows.h>
#endif

// for OSX clock functions
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

//#if defined(__unix__)
//#pragma message "__unix__ DEFINED"
//#endif
//#if defined(__APPLE__)
//#pragma message "__APPLE__ DEFINED"
//#endif
//#if defined(__CYGWIN__)
//#pragma message "__CYGWIN__ DEFINED"
//#endif
//#if defined (__QNX__)
//#pragma message ( "__QNX__ DEFINED" );
//#endif


#if defined (__QNX__)
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <math.h>
//#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
//#include <sys/queue.h>
//#include <sys/poll.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <dirent.h>
#include <time.h>
#include <ctype.h>
#include <termios.h>
//#include <pthread.h>
#include <signal.h>
#include <float.h>

// assert *only* used in unit tests
#include <assert.h>
// *NIX sys/queue.h (for QNX only)
#include "sysq.h"
#include <unix.h>
#endif

#if defined(__unix__) || defined(__APPLE__) || defined(__CYGWIN__)
//#pragma message "including std headers"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <math.h>
#include <memory.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <fcntl.h>
#include <netdb.h>
#include <dirent.h>
#include <time.h>
#include <ctype.h>
#include <termios.h>
#include <pthread.h>
#include <signal.h>
#include <float.h>
#include <unistd.h>
#include <arpa/inet.h>

// assert *only* used in unit tests
#include <assert.h>

#if defined(__unix__) || defined(__APPLE__)
#include <sys/poll.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#else
#include <winsock2.h>
#include <WS2tcpip.h>
#endif

#endif

//#elif defined(_WIN32)
////#pragma message "Compiling __WIN32"
////define something for Windows (32-bit and 64-bit, this part is common)
//#include <stdint.h>
//#include <stdbool.h>
//#include "iowrap-posix.h"
//#include "merror.h"
//#if _MSC_VER < 1900
//#       define snprintf _snprintf
//#endif //MSC_VER
//
//#define IOW_ADDR_LEN sizeof(struct sockaddr_in)
//#ifdef _WIN64
////define something for Windows (64-bit only)
//#       define WIN_DECLSPEC __declspec(dllimport)
//#else // is WIN32
////define something for Windows (32-bit only)
//#       define WIN_DECLSPEC __declspec(dllimport)
//#endif // WIN64


/////////////////////////
// Macros
/////////////////////////

#if defined(__CYGWIN__)

/// @def WIN_DECLSPEC
/// @brief declaration for windows.
#define WIN_DECLSPEC __declspec(dllimport)
#else
#define WIN_DECLSPEC
#endif

/// @def VERSION_HELPER
/// @brief version string helper.
#define VERSION_HELPER(s) #s
/// @def VERSION_STRING
/// @brief version string macro.
#define VERSION_STRING(s) VERSION_HELPER(s)

#if defined(__CYGWIN__)
//#pragma message "__CYGWIN__ is defined"
/// @def MF_EXPORT
/// @brief TBD
#define MF_EXPORT __declspec(dllimport)
#else
/// @def MF_EXPORT
/// @brief TBD
#define MF_EXPORT
#endif


/////////////////////////
// Type Definitions
/////////////////////////
#if defined(__QNX__)
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned long int uint64_t;
typedef long int int64_t;
typedef int bool;
typedef unsigned int size_t;
// dummy typedefs -
// mthread, mmem API not implemented for QNX
typedef int pthread_t;
typedef int pthread_attr_t;
typedef int pthread_mutex_t;
#define PTHREAD_MUTEX_INITIALIZER 0

#define OFFT_CAST(x) ((long int) x)
#ifndef PRIu32
#define PRIu32 "lu"
#endif
#ifndef PRId32
#define PRId32 "ld"
#endif
#ifndef PRId64
#define PRId64 "ld"
#endif
#ifndef __FUNCTION__
#define __FUNCTION__ ((const char *)"func_TBD")
#endif
#define true ((bool)1)
#define false ((bool)0)

#define O_RSYNC 0x0
#define O_ASYNC 0x0
#define B230400 230400L
#define IXANY 0x0L
#define FNDELAY O_NONBLOCK
#define CRTSCTS (IHFLOW|OHFLOW)
#define CLOCK_MONOTONIC CLOCK_REALTIME
#define gmtime_r _gmtime
#define vdprintf
#else //!__QNX__

#if defined(__APPLE__)
#define OFFT_CAST(x) ((long long) x)
#elif defined(__unix__) || defined(__CYGWIN__)
#define OFFT_CAST(x) ((long) x)
#endif

#endif

/// @typedef unsigned char  byte
/// @brief typedef for byte
typedef unsigned char  byte;


/////////////////////////
// Exports
/////////////////////////

#ifdef __cplusplus
extern "C" {
#endif
    const char *mframe_name();
    const char *mframe_version();
    const char *mframe_build();
#ifdef __cplusplus
}
#endif

#endif // THIS_FILE
