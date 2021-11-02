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

#include <stdint.h>
#include "msocket.h"
#include "mb1_msg.h"

/////////////////////////
// Type Definitions
/////////////////////////


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

/////////////////////////
// Macros
/////////////////////////
/// @def IP_PORT_MB1
/// @brief reson 7k center IP port
#define MB1_IO_PORT               7007

/////////////////////////
// Exports
/////////////////////////
// mmd_module_config_t *mmd_mb1c_config;

// MB1 utility API

/// @fn uint32_t mb1_checksum_u32(byte * pdata, uint32_t len)
/// @brief return 32-bit checksum for data of arbitrary length
/// @param[in] pdata data pointer
/// @param[in] len length of data.
/// @return 32-bit checksum value (sum of bytes).
uint32_t mb1_checksum_u32(byte *pdata, uint32_t len);

/// @fn void mb1_hex_show(byte * data, uint32_t len, uint16_t cols, _Bool show_offsets, uint16_t indent)
/// @brief output data buffer bytes in hex to stderr.
/// @param[in] data buffer pointer
/// @param[in] len number of bytes to display
/// @param[in] cols number of columns to display
/// @param[in] show_offsets show starting offset for each row
/// @param[in] indent output indent spaces
/// @return none
void mb1_hex_show(byte *data, uint32_t len, uint16_t cols, bool show_offsets, uint16_t indent);

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

// mb1_sounding API (used by mbtrn_server)

/// @fn mb1_sounding_t * mb1_sounding_new(uint32_t data_len)
/// @brief create new mb1 protocol message structure.
/// mb1 messages  must be explicitly serialized before sending.
/// @param[in] beams number of bathymetry beams
/// @return new message reference on success, NULL otherwise.
mb1_sounding_t *mb1_sounding_new(uint32_t beams);

/// @fn void mb1_sounding_destroy(mb1_sounding_t ** pself)
/// @brief release message structure resources.
/// @param[in] pself pointer to message reference
/// @return none
void mb1_sounding_destroy(mb1_sounding_t **pself);

/// @fn mb1_sounding_t *mb1_sounding_resize(mb1_sounding_t **pself, uint32_t beams,  int flags)
/// @brief resize an exsiting sounding
/// @param[in] pself pointer to sounding reference
/// @param[in] beams number of bathymetry beams
/// @param[in] flags flags indicating what fields to zero
/// @return new sounding reference on success, NULL otherwise.
mb1_sounding_t *mb1_sounding_resize(mb1_sounding_t **pself, uint32_t beams,  int flags);

/// @fn int mb1_sounding_zero(mb1_sounding_t *self, int flags)
/// @brief clear (set to zero) all/part of a sounding
/// @param[in] self sounding reference
/// @param[in] flags flags indicating what fields to zero
/// @return 0 on success, -1 otherwise.
int mb1_sounding_zero(mb1_sounding_t *self, int flags);

/// @fn void mb1_sounding_show(mb1_sounding_t * self, _Bool verbose, uint16_t indent)
/// @brief output mb1 message parameter summary to stderr.
/// @param[in] self mb1 message reference
/// @param[in] verbose use verbose output
/// @param[in] indent output indent spaces
/// @return none
void mb1_sounding_show(mb1_sounding_t *self, bool verbose, uint16_t indent);

/// @fn uint32_t mb1_calc_checksum(mb1_sounding_t *self)
/// @brief return mb1 checksum for data.
/// @param[in] self mb1 message reference
/// @return mb1 checksum value (sum of bytes).
uint32_t mb1_calc_checksum(mb1_sounding_t *self);

/// @fn uint32_t mb1_sounding_set_checksum(mb1_sounding_t * self)
/// @brief set the checksum for an mb1 message structure.
/// @param[in] self mb1 message reference
/// @return previous checksum value.
uint32_t mb1_sounding_set_checksum(mb1_sounding_t *self);

/// @fn int mb1_sounding_validate_checksum(mb1_sounding_t * self)
/// @brief validate the checksum for an mb1 message structure.
/// @param[in] self mb1 message reference
/// @return 0 if valid, -1 otherwise
int mb1_sounding_validate_checksum(mb1_sounding_t *self);

/// @fn byte * mb1_sounding_serialize(mb1_sounding_t * self)
/// @brief serialize mb1 message into new buffer.
/// Really just validating and copying, since memory is contiguous.
/// caller must release buffer resources using free.
/// @param[in] self mb1 message reference
/// @return new network frame buffer on success, NULL otherwise
byte* mb1_sounding_serialize(mb1_sounding_t *self, size_t *r_size);

// Tests

/// @fn int mb1_test()
/// @brief mb1 unit test(s).
/// currently subscribes to test server (exercising most of the mb1 API).
/// @return 0 on success, -1 otherwise
int mb1_test();

// Network

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


#endif // MB1_IO_H
