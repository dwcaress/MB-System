///
/// @file mdebug.h
/// @authors k. headley
/// @date 06 nov 2012

/// General purpose debug macros

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
#ifndef MDEBUG_H
/// @def MDEBUG_H
/// @brief TBD
#define MDEBUG_H

/////////////////////////
// Includes
/////////////////////////

#include <stdint.h>

/////////////////////////
// Type Definitions
/////////////////////////

/// @def MD_MAX_MODULES
///	@brief max number of presets (modules)
#define MD_MAX_MODULES 128

/// @def MD_UNSET
///	@brief debug level : not set
#define MD_UNSET -1
/// @def MD_NONE
///	@brief debug level : disabled
#define MD_NONE   0
/// @def MD_FATAL
///	@brief debug level : critical
#define MD_FATAL  1
/// @def MD_ERROR
///	@brief debug level : errors
#define MD_ERROR  2
/// @def MD_WARN
///	@brief debug level : warnings
#define MD_WARN   3
/// @def MD_INFO
///	@brief debug level : info
#define MD_INFO   4
/// @def MD_DEBUG
///	@brief debug level : debug (most verbose)
#define MD_DEBUG  5

/// @enum md_level_t
/// @brief debug level values
/// @typedef enum md_level_t md_level_t
/// @brief TBD
typedef enum{
    MDL_UNSET = MD_UNSET,
    MDL_NONE  = MD_NONE,
    MDL_FATAL = MD_FATAL,
    MDL_ERROR = MD_ERROR,
    MDL_WARN  = MD_WARN,
    MDL_INFO  = MD_INFO,
    MDL_DEBUG = MD_DEBUG
}md_level_t;


/// @typedef md_module_id_t
/// @brief module ID
/// @typedef uint32_t md_module_id_t
/// @brief module ID
typedef uint32_t md_module_id_t;

/// @typedef struct module_debug_config_s module_debug_config_t
/// @brief entry defines a module ID and it's debug level
typedef struct module_debug_config_s
{
    /// @var module_debug_config_s::module
    /// @brief module ID
    md_module_id_t module;
    /// @var module_debug_config_s::level
    /// @brief debug level
    md_level_t level;
}module_debug_config_t;

// global, set in mdebug.c
extern md_level_t MD_ALL_LEVEL;
extern md_module_id_t MDI_ALL;

/////////////////////////
// Macros
/////////////////////////

// Note: MTRACE, MVPRINT and MQPRINT use GCC extensions
/// @def MTPRINT
/// @brief TBD
#define MTPRINT() fprintf(stderr,"%s: %d\n",__FUNCTION__,__LINE__)
/// @def MVPRINT(fmt,__VA_ARGS__)
/// @brief TBD
#define MVPRINT(fmt, ...) fprintf(stderr,"%s:%s:%d - "fmt, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
/// @def MQPRINT(fmt,__VA_ARGS__)
/// @brief TBD
#define MQPRINT(fmt, ...) fprintf(stderr,"%s: "fmt,  __FUNCTION__, ##__VA_ARGS__)

/// @def MFATAL(fmt, args...)
/// @brief Prints formatted critical error message.
/// Called as printf and friends
/// @param fmt message format string
/// @param args variable argument list

/// @def MERROR(fmt, args...)
/// @brief Prints formatted error message.
/// Called as printf and friends
/// @param fmt message format string
/// @param args variable argument list

/// @def MDEBUG(fmt, args...)
/// @brief Prints formatted debug message.
/// Called as printf and friends
/// @param fmt message format string
/// @param args variable argument list
/// @note it expects at least one argment

/// @def MINFO(fmt, args...)
/// @brief Prints formatted info message.
/// Called as printf and friends
/// @param fmt message format string
/// @param args variable argument list
/// @note it expects at least one argment

/// @def MWARN(fmt, args...)
/// @brief Prints formatted warning message.
/// Called as printf and friends
/// @param fmt message format string
/// @param args variable argument list
/// @note it expects at least one argment

/// @def MMDEBUG(mod,fmt, ...)
/// @brief Prints formatted debug message.
/// @param mod module ID
/// @param fmt message format string
/// @param args variable argument list

/// @def MMINFO(mod,fmt, ...)
/// @brief Prints formatted info message.
/// @param mod module ID
/// @param fmt message format string
/// @param args variable argument list

/// @def MMWARN(mod,fmt, ...)
/// @brief Prints formatted warning message.
/// @param mod module ID
/// @param fmt message format string
/// @param args variable argument list

/// @def MMERROR(mod,fmt, ...)
/// @brief Prints formatted error message.
/// @param mod module ID
/// @param fmt message format string
/// @param args variable argument list

/// @def MMFATAL(mod,fmt, ...)
/// @brief Prints formatted critical error message.
/// @param mod module ID
/// @param fmt message format string
/// @param args variable argument list


/// @def MD_DEFAULT_DEBUG_LEVEL
/// @brief define default debug level
#define MD_DEFAULT_DEBUG_LEVEL MD_INFO

#ifdef MD_OLEVEL
//#pragma message "MD_OLEVEL DEFINED "

#if MD_OLEVEL==MD_NONE
//#pragma message "MD_OLEVEL N"
#define MD_DEBUG_LEVEL MD_NONE
#elif MD_OLEVEL==MD_DEBUG
//#pragma message "MD_OLEVEL D"
#define MD_DEBUG_LEVEL MD_DEBUG
#elif MD_OLEVEL==MD_INFO
//#pragma message "MD_OLEVEL I"
#define MD_DEBUG_LEVEL MD_INFO
#elif MD_OLEVEL==MD_WARN
//#pragma message "MD_OLEVEL W"
#define MD_DEBUG_LEVEL MD_WARN
#elif MD_OLEVEL==MD_ERROR
//#pragma message "MD_OLEVEL E"
#define MD_DEBUG_LEVEL MD_ERROR
#elif MD_OLEVEL==MD_FATAL
//#pragma message "MD_OLEVEL F"
#define MD_DEBUG_LEVEL MD_FATAL
#else
//#pragma message "MD_OLEVEL DFL"
#define MD_DEBUG_LEVEL MD_DEFAULT_DEBUG_LEVEL
#endif

#else
//#pragma message "MD_OLEVEL UNDEFINED"

/// @def MD_DEBUG_LEVEL
/// @brief configure system debug level
// [excludes unused levels from comipilation]
#define MD_DEBUG_LEVEL MD_DEFAULT_DEBUG_LEVEL
#endif

/// @def MD_EN_TRACE
/// @brief define EN_TRACE to enable MTRACE() statements
// [excludes them from compilation if undefined]
#define MD_EN_TRACE

/// @def MD_EN_MOD
/// @brief define module level (group) debug macros
#define MD_EN_MOD

/// @def MD_EN_TRACE
/// @brief trace enabled (as string)
#ifdef MD_EN_TRACE
/// @def MD_TRACE_S
/// @brief trace status string
#define MD_TRACE_S "ENABLED"
#else
#define MD_TRACE_S "DISBLED"
#endif

#if MD_DEBUG_LEVEL==MD_NONE
#undef MD_LEVEL_DEBUG
#undef MD_LEVEL_INFO
#undef MD_LEVEL_WARN
#undef MD_LEVEL_ERROR
#undef MD_LEVEL_FATAL
#define MD_LEVEL_S "NONE"
#elif MD_DEBUG_LEVEL==MD_DEBUG
//#pragma message "MD_DEBUG_LEVEL D"
#define MD_LEVEL_DEBUG
#define MD_LEVEL_INFO
#define MD_LEVEL_WARN
#define MD_LEVEL_ERROR
#define MD_LEVEL_FATAL
#define MD_LEVEL_S "DEBUG"
#elif MD_DEBUG_LEVEL==MD_INFO
//#pragma message "MD_DEBUG_LEVEL I"
#undef MD_LEVEL_DEBUG
/// @def MD_LEVEL_INFO
/// @brief debug level
#define MD_LEVEL_INFO
/// @def MD_LEVEL_WARN
/// @brief debug level
#define MD_LEVEL_WARN
/// @def MD_LEVEL_ERROR
/// @brief debug level
#define MD_LEVEL_ERROR
/// @def MD_LEVEL_FATAL
/// @brief debug level
#define MD_LEVEL_FATAL
/// @def MD_LEVEL_S
/// @brief debug level
#define MD_LEVEL_S "INFO"
#elif MD_DEBUG_LEVEL==MD_WARN
//#pragma message "MD_DEBUG_LEVEL W"
#undef MD_LEVEL_DEBUG
#undef MD_LEVEL_INFO
#define MD_LEVEL_WARN
#define MD_LEVEL_ERROR
#define MD_LEVEL_FATAL
#define MD_LEVEL_S "WARN"
#elif MD_DEBUG_LEVEL==MD_ERROR
//#pragma message "MD_DEBUG_LEVEL E"
#undef MD_LEVEL_DEBUG
#undef MD_LEVEL_INFO
#undef MD_LEVEL_WARN
#define MD_LEVEL_ERROR
#define MD_LEVEL_FATAL
#define MD_LEVEL_S "ERROR"
#elif MD_DEBUG_LEVEL==MD_FATAL
//#pragma message "MD_DEBUG_LEVEL F"
#undef MD_LEVEL_DEBUG
#undef MD_LEVEL_INFO
#undef MD_LEVEL_WARN
#undef MD_LEVEL_ERROR
#define MD_LEVEL_FATAL
#define MD_LEVEL_S "FATAL"
#else
//#pragma message "MD_DEBUG_LEVEL DFL"
#undef MD_LEVEL_DEBUG
#undef MD_LEVEL_INFO
#undef MD_LEVEL_WARN
#define MD_LEVEL_ERROR
#define MD_LEVEL_FATAL
#define MD_LEVEL_S "UNSET"
#endif //MD_DEBUG_LEVEL

#ifdef MD_EN_TRACE
/// @def MTRACE(fmt,__VA_ARGS__)
/// @brief trace output (function, line number)
#define MTRACE(fmt, ...) MTPRINT()
#else
#define MTRACE(fmt, ...)
#endif //MD_EN_TRACE

#ifdef MD_LEVEL_DEBUG
#define MDEBUG(fmt, ...) MQPRINT(fmt, ##__VA_ARGS__)
#define MVDEBUG(fmt, ...) MVPRINT(fmt, ##__VA_ARGS__)
#else
/// @def MDEBUG(fmt,__VA_ARGS__)
/// @brief debug output
#define MDEBUG(fmt, ...)
/// @def MVDEBUG(fmt,__VA_ARGS__)
/// @brief variable argument debug output
#define MVDEBUG(fmt, ...)
#endif //MD_LEVEL_DEBUG

#ifdef MD_LEVEL_INFO
/// @def MINFO(fmt,__VA_ARGS__)
/// @brief debug output
#define MINFO(fmt, ...) MQPRINT(fmt, ##__VA_ARGS__)
/// @def MVINFO(fmt,__VA_ARGS__)
/// @brief variable argument debug output
#define MVINFO(fmt, ...) MVPRINT(fmt, ##__VA_ARGS__)
#else
#define MINFO(fmt, ...)
#define MVINFO(fmt, ...)
#endif //MD_LEVEL_INFO

#ifdef MD_LEVEL_WARN
/// @def MWARN(fmt,__VA_ARGS__)
/// @brief debug output
#define MWARN(fmt, ...) MQPRINT(fmt, ##__VA_ARGS__)
/// @def MVWARN(fmt,__VA_ARGS__)
/// @brief variable argument debug output
#define MVWARN(fmt, ...) MVPRINT(fmt, ##__VA_ARGS__)
#else
#define MWARN(fmt, ...)
#define MVWARN(fmt, ...)
#endif //MD_LEVEL_WARN

#ifdef MD_LEVEL_ERROR
/// @def MERROR(fmt,__VA_ARGS__)
/// @brief debug output
#define MERROR(fmt, ...) MQPRINT(fmt, ##__VA_ARGS__)
/// @def MVERROR(fmt,__VA_ARGS__)
/// @brief variable argument debug output
#define MVERROR(fmt, ...) MVPRINT(fmt, ##__VA_ARGS__)
#else
#define MERROR(fmt, ...)
#define MVERROR(fmt, ...)
#endif //MD_LEVEL_ERROR

#ifdef MD_LEVEL_FATAL
/// @def MFATAL(fmt,__VA_ARGS__)
/// @brief debug output
#define MFATAL(fmt, ...) MQPRINT(fmt, ##__VA_ARGS__)
/// @def MVFATAL(fmt,__VA_ARGS__)
/// @brief variable argument debug output
#define MVFATAL(fmt, ...) MVPRINT(fmt, ##__VA_ARGS__)
#else
#define MFATAL(fmt, ...)
#define MVFATAL(fmt, ...)
#endif //MD_LEVEL_FATAL

#ifdef MD_EN_MOD
/// @def MMDEBUG(mod,fmt,__VA_ARGS__)
/// @brief module debug output
#define MMDEBUG(mod,fmt, ...) if( (MD_ALL_LEVEL>=MDL_DEBUG) || (mdb_get(mod,NULL)>=MDL_DEBUG) )\
MQPRINT(fmt, ##__VA_ARGS__)

/// @def MMINFO(mod,fmt,__VA_ARGS__)
/// @brief module debug output
#define MMINFO(mod,fmt, ...) if( (MD_ALL_LEVEL>=MDL_INFO) || (mdb_get(mod,NULL)>=MDL_INFO) )\
MQPRINT(fmt, ##__VA_ARGS__)

/// @def MMWARN(mod,fmt,__VA_ARGS__)
/// @brief module debug output
#define MMWARN(mod,fmt, ...) if( (MD_ALL_LEVEL>=MDL_WARN) || (mdb_get(mod,NULL)>=MDL_WARN) )\
MQPRINT(fmt, ##__VA_ARGS__)

/// @def MMERROR(mod,fmt,__VA_ARGS__)
/// @brief module debug output
#define MMERROR(mod,fmt, ...) if( (MD_ALL_LEVEL>=MDL_ERROR) || (mdb_get(mod,NULL)>=MDL_ERROR) )\
MQPRINT(fmt, ##__VA_ARGS__)

/// @def MMFATAL(mod,fmt,__VA_ARGS__)
/// @brief module debug output
#define MMFATAL(mod,fmt, ...) if( (MD_ALL_LEVEL>=MDL_FATAL) || (mdb_get(mod,NULL)>=MDL_FATAL) )\
MQPRINT(fmt, ##__VA_ARGS__)
#else
#define MMDEBUG(fmt, ...)
#define MMINFO(fmt, ...)
#define MMWARN(fmt, ...)
#define MMERROR(fmt, ...)
#define MMFATAL(fmt, ...)
#endif
/////////////////////////
// Exports
/////////////////////////

/// @fn int mdb_test()
/// @brief test debug macros
/// @return 0 on success, -1 otherwise
int mdb_test();

/// @fn void mdb_initialize()
/// @brief initialize debug structures
/// @return none
void mdb_initialize();

/// @fn int mdb_set(md_module_id_t id, md_level_t level)
/// @brief set debug module (group) debug level
/// @param[in] id module/group ID
/// @param[in] level debug level
/// @return 0 on success, -1 otherwise
int mdb_set(md_module_id_t id, md_level_t level);

/// @fn md_level_t mdb_get(md_module_id_t id, md_level_t *dest)
/// @brief get debug level for specified module (group), optionally store in variable
/// @param[in] id module/group ID
/// @param[in] dest store level in return value (may be NULL)
/// @return debug level for module, or MD_UNSET otherwise
md_level_t mdb_get(md_module_id_t id, md_level_t *dest);

/// @fn int mdb_set_name(md_module_id_t id, const char* name)
/// @brief set name for specified module
/// @param[in] id module/group ID
/// @param[in] name name string
/// @return 0 on success, -1 otherwise
int mdb_set_name(md_module_id_t id, const char* name);

/// @fn const char *mdb_get_name(md_module_id_t id)
/// @brief get name of specified module
/// @param[in] id module/group ID
/// @return module name or NULL
const char *mdb_get_name(md_module_id_t id);

/// @fn const char *mdb_level2str(md_level_t level);
/// @brief get string representation of debug levels
/// @param[in] level debug level
/// @return level name or NULL
const char *mdb_level2str(md_level_t level);

// include guard
#endif