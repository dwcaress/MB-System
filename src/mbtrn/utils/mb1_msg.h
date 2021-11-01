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
#include <stdbool.h>
#include <inttypes.h>

/////////////////////////
// Macros
/////////////////////////

/// @def MB1_MAX_BEAMS
/// @brief maximum number of beams
#define MB1_MAX_BEAMS 512
/// @def MB1_TYPE_ID
/// @brief MB1 record type ID (0x53423100='M''B''1''\0')
#define MB1_TYPE_ID 0x4D423100
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
/// @def MB1_FRAME_BYTES
/// @brief size of complete MB1 data frame
/// @param[in] beams number of beams
#define MB1_FRAME_BYTES(beams) (MB1_HEADER_BYTES+beams*MB1_BEAM_BYTES+MB1_CHECKSUM_BYTES)
/// @def MB1_MAX_FRAME_BYTES
/// @brief max size of MB1 data frame
#define MB1_MAX_FRAME_BYTES MB1_FRAME_BYTES(MB1_MAX_BEAMS)
/// @def MB1_EMPTY_FRAME_BYTES
/// @brief max size of MB1 data frame
#define MB1_EMPTY_FRAME_BYTES MB1_FRAME_BYTES(0)
/// @def MB1_PCHECKSUM_U32(pframe)
/// @brief checksum pointer (unsigned int *)
//#define MB1_PCHECKSUM_U32(pframe) (NULL!=pframe ? (unsigned int *) (((unsigned char *)pframe)+sizeof(mb1_frame_t)+pframe->sounding->size-MB1_CHECKSUM_BYTES) : NULL)
#define MB1_PCHECKSUM_U32(pframe) ((unsigned int *) (((unsigned char *)pframe)+sizeof(mb1_frame_t)+pframe->sounding->size-MB1_CHECKSUM_BYTES))

/// @def MB1_PSNDCHKSUM_U32(psounding)
/// @brief checksum pointer (unsigned int *)
#define MB1_SND_PCHKSUM_U32(psounding) ((mb1_checksum_t *) (((unsigned char *)psounding)+psounding->size-MB1_CHECKSUM_BYTES))

/// @def MB1_PSNDCHKSUM_U32(psounding)
/// @brief checksum pointer (unsigned int *)
#define MB1_SND_GET_CHKSUM(psounding) (mb1_checksum_t)(*MB1_SND_PCHKSUM_U32(psounding))

/// @def MB1_SOUNDING_BYTES(beams)
/// @brief sounding size (bytes)
#define MB1_SOUNDING_BYTES(beams) (MB1_HEADER_BYTES+beams*MB1_BEAM_BYTES+MB1_CHECKSUM_BYTES)

/// @def MB1_PBEAMS(psounding)
/// @brief pointer to start of beam data
#define MB1_PBEAMS(psounding) ((mb1_beam_t *) (((unsigned char *)psounding)+MB1_HEADER_BYTES))

/////////////////////////
// Type Definitions
/////////////////////////

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

#pragma pack(pop)

/////////////////////////
// Exports
/////////////////////////

#ifdef __cplusplus
extern "C" {
#endif
    /// @fn mb1_frame_t *mb1_frame_new(int beams)
    /// @brief allocate new MB1 data frame. Caller must release using mb1_frame_destroy.
    /// @param[in] beams number of beams (>=0)
    /// @return new mb1_frame_t pointer on success, NULL otherwise
//    mb1_frame_t *mb1_frame_new(int beams);
    
    /// @fn mb1_frame_t *mb1_frame_resize(mb1_frame_t **pself, int beams,  int flags)
    /// @brief resize MB1 data frame (e.g. add/remove beam data)
    /// @param[in] pself pointer to mb1_frame_t ref.
    /// @param[in] beams number of beams (>=0)
    /// @param[in] flags indicate what fields to initialize to zero (checksum always cleared)
    /// @return new mb1_frame_t pointer on success, NULL otherwise
//    mb1_frame_t *mb1_frame_resize(mb1_frame_t **pself, int beams,  int flags);

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
#ifdef __cplusplus
}
#endif

#endif // THIS_FILE
