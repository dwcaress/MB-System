///
/// @file mfile.h
/// @authors k. headley
/// @date 06 nov 2012
 
/// mframe cross-platform file IO wrappers
 
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
#ifndef MFLAG_H
/// @def MFLAG_H
/// @brief include guard
#define MFLAG_H

/////////////////////////
// Includes 
/////////////////////////

/////////////////////////
// Macros
/////////////////////////

/////////////////////////
// Type Definitions
/////////////////////////

/// @struct mfile_file_s
/// @brief wrapped file representation (posix implementation)
struct mfile_file_s;

/// @typedef struct mfile_file_s mfile_file_t
/// @brief wrapped file typedef
typedef struct mfile_file_s mfile_file_t;

/// @struct mfile_file_s
/// @brief wrapped file representation (posix implementation)
struct mfile_file_s
{
    /// @var mfile_file_s::path
    /// @brief file path
    char *path;
    /// @var mfile_file_s::fd
    /// @brief file descriptor
    int fd;
    /// @var mfile_file_s::flags
    /// @brief file attribute flags
    int flags;
    /// @var mfile_file_s::mode
    /// @brief file permissions flags
    mode_t mode;
};

/// @typedef enum mfile_flags_t mfile_flags_t
/// @brief file attribute flags
typedef enum {
    MFILE_RONLY=0x1,
    MFILE_WONLY=0x2,
    MFILE_RDWR=0x4,
    MFILE_APPEND=0x8,
    MFILE_CREATE=0x10,
    MFILE_TRUNC=0x20,
    MFILE_NONBLOCK=0x40,
    MFILE_SYNC=0x80,
    MFILE_RSYNC=0x100,
    MFILE_DSYNC=0x200,
    MFILE_ASYNC=0x400,
    MFILE_EXCL=0x400
//    MFILE_DIRECT=0x800
} mfile_flags_t;

/// @typedef enum mfile_mode_t mfile_mode_t
/// @brief file permission flags
typedef enum{
    MFILE_RWXU=0x800,
    MFILE_RU=0x400,
    MFILE_WU=0x200,
    MFILE_XU=0x100,
    MFILE_RWXG=0x80,
    MFILE_RG=0x40,
    MFILE_WG=0x20,
    MFILE_XG=0x10,
    MFILE_RWXO=0x8,
    MFILE_RO=0x4,
    MFILE_WO=0x2,
    MFILE_XO=0x1
} mfile_mode_t;

/// @typedef enum mfile_mode_t mfile_whence_t
/// @brief file seek flags
typedef enum{MFILE_SET=0, MFILE_CUR, MFILE_END} mfile_whence_t;

/////////////////////////
// Exports
/////////////////////////
#ifdef __cplusplus
extern "C" {
#endif

mfile_file_t *mfile_file_new(const char *path);
void mfile_file_destroy(mfile_file_t **pself);
void mfile_file_show(mfile_file_t *self, bool verbose, uint16_t indent);

int mfile_open(mfile_file_t *self,mfile_flags_t flags);
int mfile_close(mfile_file_t *self);
int mfile_mopen(mfile_file_t *self,mfile_flags_t flags,mfile_mode_t mode );
int64_t mfile_seek(mfile_file_t *self, uint32_t ofs, mfile_whence_t whence);
int64_t mfile_read(mfile_file_t *self, byte *dest, uint32_t len);
int64_t mfile_write(mfile_file_t *self, byte *src, uint32_t len);
int mfile_ftruncate(mfile_file_t *self, uint32_t len);
int mfile_fprintf(mfile_file_t *self, const char *fmt, ...);
int mfile_vfprintf(mfile_file_t *self, const char *fmt, va_list args);
int mfile_flush(mfile_file_t *self);
int64_t mfile_fsize(mfile_file_t *self);
time_t mfile_mtime(const char *path);
int mfile_rename(mfile_file_t *self,const char *path);
    
#ifdef __cplusplus
}
#endif

// include guard
#endif
