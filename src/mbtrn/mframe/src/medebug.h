///
/// @file medebug.h
/// @authors k. headley
/// @date 27 apr 2019

/// Simplified general purpose debug macros

/// medebug implements a set pf macros for writing messages
/// to stderr, and to group them so that some may be compiled out
/// with a single option (i.e. verbose output or debug messages).
/// In addition, there are macro variants that
/// that support printf formatting, with and without line ends or
/// simple string messages, as well as automatic generation of special
/// formatting information including error/warning text and code location.

/// medebug's three basic macro types differ by argment type
/// and end-of-line (EOL) inclusion:
///
/// *MSG    : args: single string  EOL: Y
/// *WRITE  : args: format, data   EOL: N
/// *PRINT  : args: format, data   EOL: Y
///
/// One or more macro prefixes are used to indicate persistence
/// and special output format features (e.g. error, warning, function/line, etc.)

/// Format prefixes:
/// E* : error      - error messages    format includes: ERR -
/// W* : warning    - warning messages  format includes: WARN -
/// T* : trace      - trace messages    format includes: function:line
///
/// Persistence prefixes:
/// O* : optional   - default: excluded include using WITH_MEDEBUG_OPTIONAL
/// D* : debug      - default: excluded include using WITH_MEDEBUG_DEBUG

/// Several compilation options exist to configure inclusion/exclusion
/// of medebug persistence groups:
/// compile using -DWITH_MEDEBUG_DEBUG to include T*(), D*()
/// compile using -DWITH_MEDEBUG_OPTIONAL to include O*()
/// compile using -DWITHOUT_MEDEBUG_REQUIRED to exclude
/// By default, the optional and debug groups are not included (compiled out)
/// The options may be set using environment variables of the same name
///
/// For example,include debug macros by building mframe using
/// WITH_MEDEBUG_DEBUG=1 make clean all
///
/// The unit test included (medebug-test.c) demonstrates use.

/// @sa mmdebug.c for similar messaging macros that enable run-time, per-module configuration
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
#ifndef MEDEBUG_H
/// @def MEDEBUG_H
/// @brief include guard
#define MEDEBUG_H

/////////////////////////
// Includes
/////////////////////////

#include "mframe.h"

/////////////////////////
// Type Definitions
/////////////////////////

/// @def EOL
/// @brief End of line
#define EOL "\n"

// Portable debug (for systems w/o variadic macro support (e.g. C89)
// these are used internally for portability.
// Note that P*PRINT macros require 2 sets of parentheses:
// PDPRINT((fmt,args...));
// If the macro were defined as:
// PDPRINT fprint
// the compiler will generate warnings for non-debug builds
#if defined(WITH_PDEBUG)
#define PDPRINT(x) fprintf x
#define PVPRINT(x) fprintf(stderr,"%s:%d ",__FUNCTION__,__LINE__); fprintf x
#define PWPRINT(x) fprintf(stderr,"%s:%d WARN - ",__FUNCTION__,__LINE__); fprintf x
#define PTRACE()   fprintf(stderr,"%s:%d\r\n",__FUNCTION__,__LINE__)
#else
#define PDPRINT(x)
#define PVPRINT(x)
#define PWPRINT(x)
#define PTRACE()
#endif
#define PEPRINT(x) fprintf(stderr,"%s:%d ERR - ",__FUNCTION__,__LINE__); fprintf x

#if defined(__QNX__)

#else
#ifndef WITHOUT_MEDEBUG_REQUIRED
/// @def MSG
/// @brief print debug single message to stderr (msg EOL)
#define MSG(msg)         fprintf(stderr,msg""EOL)
/// @def PRINT
/// @brief print formatted debug message to stderr (formatted_data EOL)
#define PRINT(fmt, ... ) fprintf(stderr,fmt""EOL,__VA_ARGS__)
/// @def WRITE
/// @brief print formatted debug message to stderr (formatted_data)
#define WRITE(fmt, ... ) fprintf(stderr,fmt,__VA_ARGS__)
/// @def EMSG
/// @brief print error single message to stderr (ERR - msg EOL)
#define EMSG(msg) fprintf(stderr,"ERR - "msg""EOL)
/// @def EPRINT
/// @brief print formatted error message to stderr (ERR - formatted_data EOL)
#define EPRINT(fmt, ... ) fprintf(stderr,"ERR - "fmt""EOL,__VA_ARGS__)
/// @def EWRITE
/// @brief print formatted error message to stderr (ERR - msg formatted_data)
#define EWRITE(fmt, ... ) fprintf(stderr,"ERR - "fmt,__VA_ARGS__)
/// @def ETPRINT
/// @brief print formatted error message to stderr (function:line ERR - formatted_data EOL)
#define ETPRINT(fmt, ... ) fprintf(stderr,"%s:%d ERR - "fmt""EOL,__FUNCTION__,__LINE__,__VA_ARGS__)
/// @def ETWRITE
/// @brief print formatted error message to stderr (function:line ERR - formatted_data)
#define ETWRITE(fmt, ... ) fprintf(stderr,"%s:%d ERR - "fmt,__FUNCTION__,__LINE__,__VA_ARGS__)
/// @def WMSG
/// @brief print warning single message to stderr (WARN - msg EOL)
#define WMSG(msg) fprintf(stderr,"WARN - "msg""EOL)
/// @def WPRINT
/// @brief print formatted warning message to stderr (WARN - formatted_data EOL)
#define WPRINT(fmt, ... ) fprintf(stderr,"WARN - "fmt""EOL,__VA_ARGS__)
/// @def WWRITE
/// @brief print formatted warning message to stderr (WARN - msg formatted_data)
#define WWRITE(fmt, ... ) fprintf(stderr,"WARN - "fmt,__VA_ARGS__)
/// @def WTPRINT
/// @brief print formatted warning message to stderr (function:line WARN - formatted_data EOL)
#define WTPRINT(fmt, ... ) fprintf(stderr,"%s:%d WARN - "fmt""EOL,__FUNCTION__,__LINE__,__VA_ARGS__)
/// @def WTWRITE
/// @brief print formatted warning message to stderr (function:line WARN - formatted_data)
#define WTWRITE(fmt, ... ) fprintf(stderr,"%s:%d WARN - "fmt,__FUNCTION__,__LINE__,__VA_ARGS__)
#else
#define MSG(msg)
#define PRINT(fmt, ... )
#define WRITE(fmt, ... )
#define EMSG(msg)
#define EPRINT(fmt, ... )
#define EWRITE(fmt, ... )
#define ETPRINT(fmt, ... )
#define ETWRITE(fmt, ... )
#define WMSG(msg)
#define WPRINT(fmt, ... )
#define WWRITE(fmt, ... )
#define WTPRINT(fmt, ... )
#define WTWRITE(fmt, ... )
#endif //WITHOUT_MEDEBUG_REQUIRED

#ifdef WITH_MEDEBUG_OPTIONAL
/// @def OMSG
/// @brief print debug single message to stderr (msg EOL)
#define OMSG(msg)         fprintf(stderr,msg""EOL)
/// @def OPRINT
/// @brief print formatted debug message to stderr (formatted_data EOL)
#define OPRINT(fmt, ... ) fprintf(stderr,fmt""EOL,__VA_ARGS__)
/// @def OWRITE
/// @brief print formatted debug message to stderr (formatted_data)
#define OWRITE(fmt, ... ) fprintf(stderr,fmt,__VA_ARGS__)
/// @def OEMSG
/// @brief print error single message to stderr (ERR - msg EOL)
#define OEMSG(msg) fprintf(stderr,"ERR - "msg""EOL)
/// @def OEPRINT
/// @brief print formatted error message to stderr (ERR - formatted_data EOL)
#define OEPRINT(fmt, ... ) fprintf(stderr,"ERR - "fmt""EOL,__VA_ARGS__)
/// @def OEWRITE
/// @brief print formatted error message to stderr (ERR - msg formatted_data)
#define OEWRITE(fmt, ... ) fprintf(stderr,"ERR - "fmt,__VA_ARGS__)
/// @def OETPRINT
/// @brief print formatted error message to stderr (function:line ERR - formatted_data EOL)
#define OETPRINT(fmt, ... ) fprintf(stderr,"%s:%d ERR - "fmt""EOL,__FUNCTION__,__LINE__,__VA_ARGS__)
/// @def ETWRITE
/// @brief print formatted error message to stderr (function:line ERR - formatted_data)
#define OETWRITE(fmt, ... ) fprintf(stderr,"%s:%d ERR - "fmt,__FUNCTION__,__LINE__,__VA_ARGS__)
/// @def OWMSG
/// @brief print warning single message to stderr (WARN - msg EOL)
#define OWMSG(msg) fprintf(stderr,"WARN - "msg""EOL)
/// @def OWPRINT
/// @brief print formatted warning message to stderr (WARN - formatted_data EOL)
#define OWPRINT(fmt, ... ) fprintf(stderr,"WARN - "fmt""EOL,__VA_ARGS__)
/// @def OWWRITE
/// @brief print formatted warning message to stderr (WARN - msg formatted_data)
#define OWWRITE(fmt, ... ) fprintf(stderr,"WARN - "fmt,__VA_ARGS__)
/// @def WTPRINT
/// @brief print formatted warning message to stderr (function:line WARN - formatted_data EOL)
#define OWTPRINT(fmt, ... ) fprintf(stderr,"%s:%d WARN - "fmt""EOL,__FUNCTION__,__LINE__,__VA_ARGS__)
/// @def WTWRITE
/// @brief print formatted warning message to stderr (function:line WARN - formatted_data)
#define OWTWRITE(fmt, ... ) fprintf(stderr,"%s:%d WARN - "fmt,__FUNCTION__,__LINE__,__VA_ARGS__)
#else
#define OMSG(msg)
#define OPRINT(fmt, ... )
#define OWRITE(fmt, ... )
#define OEMSG(msg)
#define OEPRINT(fmt, ... )
#define OEWRITE(fmt, ... )
#define OETPRINT(fmt, ... )
#define OETWRITE(fmt, ... )
#define OWMSG(msg)
#define OWPRINT(fmt, ... )
#define OWWRITE(fmt, ... )
#define OWTPRINT(fmt, ... )
#define OWTWRITE(fmt, ... )
#endif //WITH_MEDEBUG_OPTIONAL

#ifdef WITH_MEDEBUG_DEBUG
/// @def TRACE
/// @brief print trace location to stderr (function:line EOL)
#define TRACE() fprintf(stderr,"%s:%d"EOL,__FUNCTION__,__LINE__)
/// @def TRACEW
/// @brief print trace location to stderr (function:line)
#define TRACEW() fprintf(stderr,"%s:%d",__FUNCTION__,__LINE__)
/// @def TMSG
/// @brief print trace message to stderr (function:line msg EOL)
#define TMSG(msg) fprintf(stderr,"%s:%d "msg""EOL,__FUNCTION__,__LINE__)
/// @def TMSG
/// @brief print trace message to stderr (function:line formatted_data EOL)
#define TPRINT(fmt, ... ) fprintf(stderr,"%s:%d "fmt""EOL,__FUNCTION__,__LINE__,__VA_ARGS__)
/// @def TMSG
/// @brief print trace message to stderr (function:line formatted_data)
#define TWRITE(fmt, ... ) fprintf(stderr,"%s:%d "fmt,__FUNCTION__,__LINE__,__VA_ARGS__)
/// @def DMSG
/// @brief print debug single message to stderr (msg EOL)
#define DMSG(msg)         fprintf(stderr,msg""EOL)
/// @def DPRINT
/// @brief print formatted debug message to stderr (formatted_data EOL)
#define DPRINT(fmt, ... ) fprintf(stderr,fmt""EOL,__VA_ARGS__)
/// @def DWRITE
/// @brief print formatted debug message to stderr (formatted_data)
#define DWRITE(fmt, ... ) fprintf(stderr,fmt,__VA_ARGS__)
#else
#define TRACE()
#define TRACEW()
#define TMSG(msg)
#define TPRINT(fmt, ... )
#define TWRITE(fmt, ... )
#define DMSG(msg)
#define DPRINT(fmt, ... )
#define DWRITE(fmt, ... )
#endif //WITH_MEDEBUG_DEBUG

#endif // ! __QNX__
// include guard
#endif // MEDEBUG_H
