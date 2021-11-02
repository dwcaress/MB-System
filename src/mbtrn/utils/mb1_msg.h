///
/// @file mb1_msg.h
/// @authors k. headley
/// @date 09 jul 2019

/// MBSystem MB1 record format

/// @sa doxygen-examples.c for more examples of Doxygen markup


/////////////////////////
// Terms of use
/////////////////////////
/*
 Copyright Information
  
 Copyright 2002-YYYY MBARI
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
#ifndef MB1_MSG_H
#define MB1_MSG_H

/////////////////////////
// Includes 
/////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

/////////////////////////
// Macros
/////////////////////////

/// @def IP_PORT_MB1
/// @brief reson 7k center IP port
#define MB1_IP_PORT_DFL 7007

/// @def MB1_MAX_BEAMS
/// @brief maximum number of beams
#define MB1_MAX_BEAMS 512

/// @def MB1_TYPE_ID
/// @brief MB1 record type ID (0x4D423100='M''B''1''\0')
//#define MB1_TYPE_ID 0x4D423100
#define MB1_TYPE_ID 0x0031424D
/// @def MB1_HEADER_BYTES
/// @brief MB1 header (static field) size (bytes)
#define MB1_HEADER_BYTES   sizeof(mb1_header_t) //56
/// @def MB1_TYPE_BYTES
/// @brief MB1 type size (bytes)
#define MB1_TYPE_BYTES   4
/// @def MB1_SIZE_BYTES
/// @brief MB1 size size (bytes)
#define MB1_SIZE_BYTES   4
/// @def MB1_BEAM_BYTES
/// @brief MB1 beam data size
#define MB1_BEAM_BYTES     28
/// @def MB1_CHECKSUM_BYTES
/// @brief MB1 checksum size
#define MB1_CHECKSUM_BYTES  sizeof(mb1_checksum_t) //4
/// @def MB1_BEAM_ARRAY_BYTES
/// @brief size of beam array
/// @param[in] beams number of beams
#define MB1_BEAM_ARRAY_BYTES(beams) (beams*MB1_BEAM_BYTES)

/// @def MB1_PSNDCHKSUM_U32(psounding)
/// @brief checksum pointer (unsigned int *)
#define MB1_PCHECKSUM(psounding)  ((uint32_t *) (((unsigned char *)psounding)+psounding->size-MB1_CHECKSUM_BYTES))

/// @def MB1_PBEAMS(psounding)
/// @brief pointer to start of beam data
#define MB1_PBEAMS(psounding) ((mb1_beam_t *) (((unsigned char *)psounding)+MB1_HEADER_BYTES))

/// @def MB1_CHECKSUM_LEN_BYTES(psounding)
/// @brief number of bytes over which checksum is calculated (header+beams)
#define MB1_CHECKSUM_LEN_BYTES(psounding) (psounding->size-MB1_CHECKSUM_BYTES)

/// @def MB1_PSNDCHKSUM_U32(psounding)
/// @brief checksum pointer (unsigned int *)
#define MB1_GET_CHECKSUM(psounding) (mb1_checksum_t)(*MB1_PCHECKSUM(psounding))

/// @def MB1_SOUNDING_BYTES(beams)
/// @brief sounding size (bytes)
#define MB1_SOUNDING_BYTES(beams) (MB1_HEADER_BYTES+beams*MB1_BEAM_BYTES+MB1_CHECKSUM_BYTES)

/// @def MB1_MAX_SOUNDING_BYTES
/// @brief sounding size (bytes) for MB1_MAX_BEAMS
#define MB1_MAX_SOUNDING_BYTES MB1_SOUNDING_BYTES(MB1_MAX_BEAMS)

/// @def MB1_EMPTY_SOUNDING_BYTES
/// @brief empty sounding size (bytes) for 0 beams
#define MB1_EMPTY_SOUNDING_BYTES MB1_SOUNDING_BYTES(0)


/////////////////////////
// Type Definitions
/////////////////////////

typedef unsigned char byte;

typedef uint32_t mb1_checksum_t;

/// @typedef enum mb1_resize_flags_t
/// @brief resize flags (indicate which fields to clear)
typedef enum{
    MB1_RS_BEAMS=0x1,
    MB1_RS_HEADER=0x2,
    MB1_RS_CHECKSUM=0x4
} mb1_resize_flags_t;

/// @def MB1_RS_ALL
/// @brief clear whole frame on resize
#define MB1_RS_ALL (MB1_RS_BEAMS|MB1_RS_HEADER|MB1_RS_CHECKSUM)


#pragma pack(push,1)
/// @typedef struct mb1_beam_s mb1_beam_t
/// @brief MB1 beam data structure
typedef struct mb1_beam_s
{
    /// @var mb1_beam_s::beam_num
    /// @brief beam number
     // beam number (0 is port-most beam)
    uint32_t beam_num;
    /// @var mb1_beam_s::rhox
    /// @brief along track position wrt sonar (m)
    double rhox;
    /// @var mb1_beam_s::rhoy
    /// @brief cross track position wrt sonar (m)
    double rhoy;
    /// @var mb1_beam_s::rhoz
    /// @brief vertical position wrt to sonar (m, positive down)
    double rhoz;
}mb1_beam_t;

/// @typedef struct mb1_header_s mb1_header_t
/// @brief static header struct (for convenience)
typedef struct mb1_header_s
{
    /// @var mb1_sounding_s::type
    /// @brief record type ID ('M''B''1''\0')
    uint32_t type;
    /// @var mb1_sounding_s::size
    /// @brief total bytes, including header and checksum
    uint32_t size;
    /// @var mb1_sounding_s::ts
    /// @brief epoch time of ping
    double ts;
    /// @var mb1_sounding_s::lat
    /// @brief latitude
    double lat;
    /// @var mb1_sounding_s::lon
    /// @brief longitude
    double lon;
    /// @var mb1_sounding_s::depth
    /// @brief vehicle position depth meters
    double depth;
    /// @var mb1_sounding_s::hdg
    /// @brief heading
    double hdg;
    /// @var mb1_sounding_s::ping_number
    /// @brief ping number
    int32_t ping_number;
    /// @var mb1_sounding_s::nbeams
    /// @brief number of beams
    uint32_t nbeams;
}mb1_header_t;

/// @typedef struct mb1_sounding_s mb1_sounding_t
/// @brief
typedef struct mb1_sounding_s
{
    /// @var mb1_sounding_s::type
    /// @brief record type ID ('M''B''1''\0')
    uint32_t type;
    /// @var mb1_sounding_s::size
    /// @brief total bytes, including header and checksum
    uint32_t size;
    /// @var mb1_sounding_s::ts
    /// @brief epoch time of ping
    double ts;
    /// @var mb1_sounding_s::lat
    /// @brief latitude
    double lat;
    /// @var mb1_sounding_s::lon
    /// @brief longitude
    double lon;
    /// @var mb1_sounding_s::depth
    /// @brief vehicle position depth meters
    double depth;
    /// @var mb1_sounding_s::hdg
    /// @brief heading
    double hdg;
    /// @var mb1_sounding_s::ping_number
    /// @brief ping number
    int32_t ping_number;
    /// @var mb1_sounding_s::nbeams
    /// @brief number of beams
    uint32_t nbeams;
    /// @var mb1_sounding_s::beams
    /// @brief beam data array
        mb1_beam_t beams[];
    /// 32-bit checksum follows beam array
}mb1_sounding_t;

#pragma pack(pop)

/////////////////////////
// Exports
/////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif // THIS_FILE
