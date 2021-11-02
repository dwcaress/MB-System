///
/// @file mb1-io.h
/// @authors k. headley
/// @date 06 nov 2012

/// Reson 7k Center data structures and protocol API

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
#ifndef MB1_IO_H
/// @def MB1_DATA_H
/// @brief include guard
#define MB1_IO_H

/////////////////////////
// Includes
/////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include "mb1_msg.h"
#ifdef WITH_MB1_UTILS
#include "msocket.h"
#endif //WITH_MB1_UTILS

/////////////////////////
// Type Definitions
/////////////////////////


#ifdef WITH_MB1_PARSE_STAT
#pragma pack(push, 1)
/// @typedef struct mb1_parse_stat_s mb1_parse_stat_t
/// @brief mb1 raw data parser status information
typedef struct mb1_parse_stat_s
{
    /// @var mb1_parse_stat_s::src_bytes
    /// @brief size of source data
    uint32_t src_bytes;
    /// @var mb1_parse_stat_s::sync_bytes
    /// @brief number of bytes skipped due to parsing sync errors
    uint32_t sync_bytes;
    /// @var mb1_parse_stat_s::unread_bytes
    /// @brief number of unread source bytes (remaining bytes < valid record size)
    uint32_t unread_bytes;
    /// @var mb1_parse_stat_s::parsed_records
    /// @brief number of valid data record frames (DRF) parsed
    uint32_t parsed_records;
    /// @var mb1_parse_stat_s::parsed_bytes
    /// @brief total size of parsed frames
    uint32_t parsed_bytes;
    /// @var mb1_parse_stat_s::resync_count
    /// @brief number of sync errors
    uint32_t resync_count;
    /// @var mb1_parse_stat_s::status
    /// @brief exit status: ME_ error or ME_OK
    int status;
}mb1_parse_stat_t;
#pragma pack(pop)
#endif // WITH_MB1_PARSE_STAT

#ifdef WITH_MB1_FRAME

/// @typedef struct mb1_frame_s mb1_frame_t
/// @brief MB1 data frame.
/// Structure is variable length; sounding and checksum
/// pointers are at the beginning, followed by sounding
/// data (variable length) and checksum, which must be
/// contiguous.
typedef struct mb1_frame_s
{
    /// @var mb1_frame_s::sounding
    /// @brief sounding data
    mb1_sounding_t *sounding;
    /// @var mb1_frame_s::checksum
    /// @brief 32-bit checksum (byte sum over header and beam data)
    uint32_t *checksum;
    /// sounding data (variable length, contiguous) follows checksum pointer
    /// sounding points to start of sounding header
    /// checksum points to checksum (after variable length beam data array)
}mb1_frame_t;
#endif // WITH_MB1_FRAME


/////////////////////////
// Macros
/////////////////////////
#ifdef WITH_MB1_FRAME
/// @def MB1_FRAME_BYTES
/// @brief size of complete MB1 data frame
/// @param[in] beams number of beams
//#define MB1_FRAME_BYTES(beams) (MB1_HEADER_BYTES+beams*MB1_BEAM_BYTES+MB1_CHECKSUM_BYTES)
/// @def MB1_MAX_FRAME_BYTES
/// @brief max size of MB1 data frame
//#define MB1_MAX_FRAME_BYTES MB1_FRAME_BYTES(MB1_MAX_BEAMS)
/// @def MB1_EMPTY_FRAME_BYTES
/// @brief max size of MB1 data frame
//#define MB1_EMPTY_FRAME_BYTES MB1_FRAME_BYTES(0)
#endif

/////////////////////////
// Exports
/////////////////////////
// mmd_module_config_t *mmd_mb1c_config;

#ifdef WITH_MB1_FRAME
/// @fn mb1_frame_t *mb1_frame_new(int beams)
/// @brief allocate new MB1 data frame. Caller must release using mb1_frame_destroy.
/// @param[in] beams number of beams (>=0)
/// @return new mb1_frame_t pointer on success, NULL otherwise
mb1_frame_t *mb1_frame_new(int beams);

/// @fn mb1_frame_t *mb1_frame_resize(mb1_frame_t **pself, int beams,  int flags)
/// @brief resize MB1 data frame (e.g. add/remove beam data)
/// @param[in] pself pointer to mb1_frame_t ref.
/// @param[in] beams number of beams (>=0)
/// @param[in] flags indicate what fields to initialize to zero (checksum always cleared)
/// @return new mb1_frame_t pointer on success, NULL otherwise
mb1_frame_t *mb1_frame_resize(mb1_frame_t **pself, int beams,  int flags);

/// @fn int mb1_frame_zero(mb1_frame_t **pself, int flags)
/// @brief zero MB1 data frame
/// @param[in] pself pointer to mb1_frame_t ref.
/// @param[in] flags indicate what fields to initialize to zero (checksum always cleared)
/// @return 0 on success, -1 otherwise
int mb1_frame_zero(mb1_frame_t *self, int flags);

/// @fn void mb1_frame_destroy(mb1_frame_t **pself);
/// @brief release resources for mb1_frame_t.
/// @param[in] pself pointer to mb1_frame_t ref.
/// @return none
void mb1_frame_destroy(mb1_frame_t **pself);

/// @fn unsigned int mb1_frame_calc_checksum(mb1_frame_t *self);
/// @brief calculate 32-bit checksum.
/// @param[in] self pointer to mb1_frame_t ref.
/// @return 32-bit (unsigned int) checksum) on success, 0xFFFF otherwise
unsigned int mb1_frame_calc_checksum(mb1_frame_t *self);

/// @fn void mb1_frame_show(mb1_frame_t *self, bool verbose, uint16_t indent)
/// @brief write frame summary to console (stderr)
/// @param[in] self    frame reference
/// @param[in] verbose indent extra output (if implemented)
/// @param[in] indent  output indentation (spaces)
/// @return none
void mb1_frame_show(mb1_frame_t *self, bool verbose, uint16_t indent);
#endif //WITH_MB1_FRAME

#ifdef WITH_MB1_PARSE_STAT
/// @fn void mb1_parser_show(mb1_parse_stat_t * self, _Bool verbose, uint16_t indent)
/// @brief output mb1 parser statistics to stderr.
/// @param[in] self parser stats structure reference
/// @param[in] verbose use verbose output
/// @param[in] indent output indent spaces
/// @return none
void mb1_parser_show(mb1_parse_stat_t *self, bool verbose, uint16_t indent);

/// @fn char * mb1_parser_str(mb1_parse_stat_t * self, char * dest, uint32_t len, _Bool verbose, uint16_t indent)
/// @brief output parser statistics to a string. Caller must free.
/// @param[in] self parser stat struct reference
/// @param[in] dest string output buffer, use NULL to allocate new string
/// @param[in] len buffer length
/// @param[in] verbose use verbose output
/// @param[in] indent output indent spaces
/// @return new string (if dest==NULL) or dest on success, NULL otherwise. May truncate if buffer length is insufficient.
char *mb1_parser_str(mb1_parse_stat_t *self, char *dest, uint32_t len, bool verbose, uint16_t indent);
#endif // WITH_MB1_PARSE_STAT

// Network
#ifdef WITH_MB1_UTILS

/// @fn int mb1_stream_show(msock_socket_t * s, int sz, uint32_t tmout_ms, int cycles)
/// @brief output raw mb1 stream to stderr as formatted ASCII hex.
/// @param[in] s mb1 host socket
/// @param[in] sz read buffer size (read sz bytes at a time)
/// @param[in] tmout_ms read timeout
/// @param[in] cycles number of cycles to read (<=0 read forever)
/// @return 0 on success, -1 otherwise
int mb1_stream_show(msock_socket_t *s, int sz, uint32_t tmout_ms, int cycles, bool *interrupt);


/// @fn int mb1_sounding_send(msock_socket_t * s, mb1_sounding_t * self)
/// @brief serialize and send an mb1 message
/// @param[in] s socket reference
/// @param[in] self mb1 message structure
/// @return number of bytes sent (>=0) on success, -1 otherwise
int mb1_sounding_send(msock_socket_t *s, mb1_sounding_t *self);

/// @fn int mb1_sounding_receive(msock_socket_t * s, mb1_sounding_t ** dest, uint32_t timeout_msec)
/// @brief receive sounding into message structure.
/// @param[in] s socket reference
/// @param[in] dest pointer to mb1 message reference to hold message
/// @param[in] timeout_msec read timeout
/// @return number of bytes received on success, -1 otherwise.
int mb1_sounding_receive(msock_socket_t *s, mb1_sounding_t **dest, uint32_t timeout_msec);
#endif //WITH_MB1_UTILS


#endif // MB1_IO_H
