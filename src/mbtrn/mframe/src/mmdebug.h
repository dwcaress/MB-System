///
/// @file mmdebug.h
/// @authors k. headley
/// @date 06 nov 2012

/// General purpose per-module output macros
/// [run-time configurable, global compilation disable]

/// Compile mmdebug unit test using
/// gcc -DMMD_WITH_TEST -o mmd-test mmdebug-test.c mmdebug.c -L../bin -lmframe
/// use -DWITH_MMT_OPTIONAL, -DWITH_MMT_DEBUG to enable optional groups
/// use -DWITHOUT_MMT_REQUIRED to *disable* the REQUIRED macro group

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
#ifndef MMDEBUG_H
/// @def MMDEBUG_H
/// @brief TBD
#define MMDEBUG_H

/////////////////////////
// Includes
/////////////////////////

#include "mframe.h"

/////////////////////////
// Type Definitions
/////////////////////////

/// @def CHMSK(i)
/// @brief convert channel ID to mask
/// [mask bit <i>]
#define CHMSK(i) (1<<i)

/// @enum mmd_module_ids
/// @brief module IDs reserved for mframe modules
/// typically, applications use module IDs starting at MM_MODULE_COUNT
/// defined in mconfig.h
typedef enum{
    MOD_MFRAME=0,
    MOD_MERR,
    MOD_MBBUF,
    MOD_MCBUF,
    MOD_MFILE,
    MOD_MLIST,
    MOD_MLOG,
    MOD_MMEM,
    MOD_MQUEUE,
    MOD_MSOCK,
    MOD_MSTATS,
    MOD_MTIME,
    MOD_MTHREAD,
    MM_MODULE_COUNT
}mmd_module_ids;

/// @enum mmd_channel_id
/// @brief channel IDs reserved for mframe modules
/// applications may also use these, in which case
/// module-specific channels IDs should start at
/// MM_CHANNEL_COUNT. If applications use these,
/// they must also define corresponding channel names.
typedef enum{
    MMID_TRACE=0,
    MMID_DEBUG,
    MMID_WARN,
    MMID_ERR,
    MM_CHANNEL_COUNT
}mmd_channel_id;

/// @enum mmd_channel_mask
/// @brief channel masks reserved for mframe modules
typedef enum{
	MM_NONE=0x00000000,
    MM_TRACE=(1<<MMID_TRACE),
    MM_DEBUG=(1<<MMID_DEBUG),
    MM_WARN=(1<<MMID_WARN),
    MM_ERR=(1<<MMID_ERR),
    MM_ALL=0xFFFFFFFF
}mmd_channel_mask;

/// define module, channel IDs in a common header (mconfig.h)

/// @typedef mmd_module_id_t
/// @brief module ID
typedef uint16_t mmd_module_id_t;
/// @typedef uint16_t mmd_channel_id_t
/// @brief channel ID
typedef uint16_t mmd_channel_id_t;
/// @typedef uint32_t mmd_en_mask_t
/// @brief enable mask type
typedef uint32_t mmd_en_mask_t;

/// @typedef struct mmd_module_config_s mmd_module_config_t
/// @brief entry defines a module ID and it's debug level
typedef struct mmd_module_config_s
{
    /// @var mmd_module_config_s::module
    /// @brief module ID
    mmd_module_id_t id;
    /// @var mmd_module_config_s::name
    /// @brief module name
    char *name;
    /// @var mmd_module_config_s::channel_count
    /// @brief number of channels
    uint32_t channel_count;
    /// @var mmd_module_config_s::en_mask
    /// @brief channel enable mask
    mmd_en_mask_t en_mask;
    /// @var mmd_module_config_s::channel_names
    /// @brief channel name array
    char **channel_names;
}mmd_module_config_t;

/////////////////////////
// Macros
/////////////////////////

/// @def EOL
/// @brief End of line
#define EOL "\n"

#if defined(WITHOUT_MMDEBUG)
#define PMPRINT(mod,cmsk,x)
#define PMTRACE(mod,cmsk)
#else
#define PMPRINT(mod,cmsk,x) if(mmd_channel_isset(mod,cmsk))fprintf x
#define PMTRACE(mod,cmsk) if( mmd_channel_isset(mod,cmsk))MXTRACE()
#endif
#define PMEPRINT(mod,cmsk,x) if(mmd_channel_isset(mod,cmsk))fprintf x

#if defined(__QNX__)

#define MXTRACE() fprintf(stderr,"%s:%d\n",__FUNCTION__,__LINE__)

#else

// Note: MX* macros use C99/GCC extensions
/// @def MTPRINT
/// @brief TBD
#define MXTRACE() fprintf(stderr,"%s:%d\n",__FUNCTION__,__LINE__)
/// @def MVTPRINT(fmt,__VA_ARGS__)
/// @brief TBD
#define MXVTPRINT(fmt, ...) fprintf(stderr,"%s:%s:%d - "fmt""EOL, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
/// @def MVTWRITE(fmt,__VA_ARGS__)
/// @brief TBD
#define MVTWRITE(fmt, ...) fprintf(stderr,"%s:%s:%d - "fmt, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
/// @def MXMSG(msg,__VA_ARGS__)
/// @brief TBD
#define MXMSG(msg) fprintf(stderr,msg""EOL)
/// @def MXWRITE(fmt,__VA_ARGS__)
/// @brief TBD
#define MXWRITE(fmt, ...) fprintf(stderr,fmt, ##__VA_ARGS__)
/// @def MXPRINT(fmt,__VA_ARGS__)
/// @brief TBD
#define MXPRINT(fmt, ...) fprintf(stderr,fmt""EOL, ##__VA_ARGS__)
/// @def MXWWRITE(fmt,__VA_ARGS__)
/// @brief TBD
#define MXWWRITE(fmt, ...) fprintf(stderr,"WARN - "fmt, ##__VA_ARGS__)
/// @def MXWPRINT(fmt,__VA_ARGS__)
/// @brief TBD
#define MXWPRINT(fmt, ...) fprintf(stderr,"WARN - "fmt""EOL, ##__VA_ARGS__)
/// @def MXEWRITE(fmt,__VA_ARGS__)
/// @brief TBD
#define MXEWRITE(fmt, ...) fprintf(stderr,"ERR - "fmt, ##__VA_ARGS__)
/// @def MXEPRINT(fmt,__VA_ARGS__)
/// @brief TBD
#define MXEPRINT(fmt, ...) fprintf(stderr,"ERR - "fmt""EOL, ##__VA_ARGS__)

#ifndef WITHOUT_MMDEBUG

/// @def MMTRACE(mod,cmsk)
/// @brief trace (func:line EOL)
#define MMTRACE(mod,cmsk) if( mmd_channel_isset(mod,cmsk) ) MXTRACE()

/// @def MMVTWRITE(mod,ch,fmt,__VA_ARGS__)
/// @brief trace write (file,func,line - ...)
#define MMVTWRITE(mod,cmsk,fmt,...) if( mmd_channel_isset(mod,cmsk) ) MXVTWRITE(fmt, ##__VA_ARGS__)

/// @def MMVTPRINT(mod,ch,fmt,__VA_ARGS__)
/// @brief trace print (file,func,line - ...EOL)
#define MMVTPRINT(mod,cmsk,fmt,...) if( mmd_channel_isset(mod,cmsk) ) MXVTPRINT(fmt, ##__VA_ARGS__)

/// @def MMMSG(mod,ch,)
/// @brief messsage (msg EOL)
#define MMMSG(mod,cmsk,msg) if( mmd_channel_isset(mod,cmsk) ) MXMSG(msg)

/// @def MMWRITE(mod,ch,fmt,__VA_ARGS__)
/// @brief output message (w/o EOL) (...)
#define MMWRITE(mod,cmsk,fmt, ...) if( mmd_channel_isset(mod,cmsk) ) MXWRITE(fmt, ##__VA_ARGS__)

/// @def MMPRINT(mod,ch,fmt,__VA_ARGS__)
/// @brief output message (w/ EOL) (...EOL)
#define MMPRINT(mod,cmsk,fmt, ...) if( mmd_channel_isset(mod,cmsk) ) MXPRINT(fmt, ##__VA_ARGS__)

/// @def MWWRITE(mod,ch,fmt,__VA_ARGS__)
/// @brief output warning (w/o EOL) (WARN - ...)
#define MWWRITE(mod,cmsk,fmt, ...) if( mmd_channel_isset(mod,cmsk) ) MXWWRITE(fmt, ##__VA_ARGS__)

/// @def MWPRINT(mod,ch,fmt,__VA_ARGS__)
/// @brief output warning (w/o EOL) (WARN - ... EOL)
#define MWPRINT(mod,cmsk,fmt, ...) if( mmd_channel_isset(mod,cmsk) ) MXWPRINT(fmt, ##__VA_ARGS__)

/// @def MEWRITE(mod,ch,fmt,__VA_ARGS__)
/// @brief output error (w/o EOL) (ERR - ...)
#define MEWRITE(mod,cmsk,fmt, ...) if( mmd_channel_isset(mod,cmsk) ) MXEWRITE(fmt, ##__VA_ARGS__)

/// @def MMEPRINT(mod,ch,fmt,__VA_ARGS__)
/// @brief output error (w/o EOL) (ERR - ... EOL)
#define MEPRINT(mod,cmsk,fmt, ...) if( mmd_channel_isset(mod,cmsk) ) MXEPRINT(fmt, ##__VA_ARGS__)
#else
#define MMTRACE(mod,cmsk)
#define MMVTWRITE(mod,cmsk,fmt,...)
#define MMVTPRINT(mod,cmsk,fmt,...)
#define MMMSG(mod,cmsk,msg)
#define MMWRITE(mod,cmsk,fmt, ...)
#define MMPRINT(mod,cmsk,fmt, ...)
#define MWWRITE(mod,cmsk,fmt, ...)
#define MWPRINT(mod,cmsk,fmt, ...)
#define MEWRITE(mod,cmsk,fmt, ...)
#define MEPRINT(mod,cmsk,fmt, ...)
#endif //WITHOUT_MMDEBUG

#endif // ! __QNX__

/////////////////////////
// Exports
/////////////////////////

/// @fn int mmd_test()
/// @brief test debug macros
/// @return 0 on success, -1 otherwise
int mmd_test();

/// @fn void mmd_initialize()
/// @brief initialize debug structures
/// @return none
void mmd_initialize();

/// @fn void mmd_release()
/// @brief release resources
/// @return none
void mmd_release();

/// @fn int mmd_module_configure(mmd_module_config_t *config)
/// @brief configure module
/// @param[in] config module configuration
//// @return 0 on success, -1 otherwise
int mmd_module_configure(mmd_module_config_t *config);

/// @fn int mmd_channel_configure(mmd_module_id_t id, char *name, uint32_t channel, bool enable);
/// @brief configure module output channels
/// @param[in] id module/group ID
/// @param[in] ch_id channel ID
/// @param[in] name channel name
/// @param[in] enable 0:disable 1:enable -1:N/C
/// @return 0 on success, -1 otherwise
//int mmd_channel_configure(mmd_module_id_t id, mmd_channel_id_t ch_id, char *name,  int enable);

/// @fn int mmd_channel_set(mmd_module_id_t id, uint32_t mask)
/// @brief set module output channels
/// @param[in] id module/group ID
/// @param[in] mask output channel mask
/// @return 0 on success, -1 otherwise
int mmd_channel_set(mmd_module_id_t id, mmd_en_mask_t mask);

/// @fn int mmd_channel_en(mmd_module_id_t id, uint32_t mask)
/// @brief enable module output channels
/// @param[in] id module/group ID
/// @param[in] mask output channel mask
/// @return 0 on success, -1 otherwise
int mmd_channel_en(mmd_module_id_t id, mmd_en_mask_t mask);

/// @fn int mmd_channel_en(mmd_module_id_t id, uint32_t mask)
/// @brief disable module output channels
/// @param[in] id module/group ID
/// @param[in] mask output channel mask
/// @return 0 on success, -1 otherwise
int mmd_channel_dis(mmd_module_id_t id, mmd_en_mask_t mask);

/// @fn bool mmd_channel_isset(mmd_module_id_t id, uint32_t mask);
/// @brief return true if any of the specified module channels are enabled
/// @param[in] id module/group ID
/// @param[in] ch_mask channel mask
/// @return debug level for module, or MD_UNSET otherwise
bool mmd_channel_isset(mmd_module_id_t id, mmd_en_mask_t mask);

/// @fn md_level_t mmd_get_enmask(mmd_module_id_t id, md_level_t *dest)
/// @brief get current output channel mask (optionally store in destination)
/// @param[in] id module/group ID
/// @param[out] dest return value (pass NULL to ignore)
/// @return debug level for module, or MD_UNSET otherwise
mmd_en_mask_t mmd_get_enmask(mmd_module_id_t id, mmd_en_mask_t *dest);

/// @fn char *mmd_module_name(mmd_module_id_t id)
/// @brief get module name
/// @param[in] id module/group ID
/// @return module name on success, NULL otherwise
char *mmd_module_name(mmd_module_id_t id);

/// @fn char *mmd_channel_name(mmd_module_id_t id, mmd_channel_id_t ch_id)
/// @brief get channel name
/// @param[in] id module/group ID
/// @param[in] ch_id channel ID
/// @return channel name on success, NULL otherwise
char *mmd_channel_name(mmd_module_id_t id, mmd_channel_id_t ch_id);

#ifdef WITH_MMDEBUG_TEST
/// @fn int mmd_test()
/// @brief mmdebug unit test
/// @return 0 on success, -1 otherwise
int mmd_test();
#endif //WITH_MMDEBUG_TEST

// include guard
#endif
