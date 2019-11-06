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
#define MB1_TYPE_ID 0x53423100
/// @def MB1_HEADER_BYTES
/// @brief MB1 header (static field) size (bytes)
#define MB1_HEADER_BYTES   56
/// @def MB1_BEAM_BYTES
/// @brief MB1 beam data size
#define MB1_BEAM_BYTES     28
/// @def MB1_CHECKSUM_BYTES
/// @brief MB1 checksum size
#define MB1_CHECKSUM_BYTES  4
/// @def MB1_BEAM_ARRAY_BYTES
/// @brief size of beam array
/// @param[in] beams number of beams
#define MB1_BEAM_ARRAY_BYTES(beams) (beams*MB1_BEAM_BYTES)
/// @def MB1_FRAME_BYTES
/// @brief size of complete MB1 data frame
/// @param[in] beams number of beams
#define MB1_FRAME_BYTES(beams) (MB1_HEADER_BYTES+beams*MB1_BEAM_BYTES+MB1_CHECKSUM_BYTES)

/////////////////////////
// Type Definitions
/////////////////////////

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
    unsigned int beam_num;
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

/// @typedef struct mb1_sounding_s mb1_sounding_t
/// @brief
typedef struct mb1_sounding_s
{
    /// @var mb1_sounding_s::type
    /// @brief record type ID ('M''B''1''\0')
    unsigned int type;
    /// @var mb1_sounding_s::size
    /// @brief total bytes, including header and checksum
    unsigned int size;
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
    int ping_number;
    /// @var mb1_sounding_s::nbeams
    /// @brief number of beams
    unsigned int nbeams;
    /// @var mb1_sounding_s::beams
    /// @brief beam data array
    mb1_beam_t beams[];
}mb1_sounding_t;

/// @typedef struct mb1_frame_s mb1_frame_t
/// @brief MB1 data frame
typedef struct mb1_frame_s
{
    /// @var mb1_frame_s::sounding
    /// @brief sounding data
    mb1_sounding_t sounding;
    /// @var mb1_frame_s::checksum
    /// @brief 32-bit checksum (byte sum over header and beam data)
    uint32_t checksum;
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
    mb1_frame_t *mb1_frame_new(int beams);
    
    /// @fn mb1_frame_t *mb1_frame_resize(mb1_frame_t **pself, int beams,  int flags)
    /// @brief resize MB1 data frame (e.g. add/remove beam data)
    /// @param[in] pself pointer to mb1_frame_t ref.
    /// @param[in] beams number of beams (>=0)
    /// @param[in] flags indicate what fields to initialize to zero (checksum always cleared)
    mb1_frame_t *mb1_frame_resize(mb1_frame_t **pself, int beams,  int flags);
    
    /// @fn void mb1_frame_destroy(mb1_frame_t **pself);
    /// @brief release resources for mb1_frame_t.
    /// @param[in] pself pointer to mb1_frame_t ref.
    void mb1_frame_destroy(mb1_frame_t **pself);
    
    /// @fn void mb1_frame_show(mb1_frame_t *self, bool verbose, uint16_t indent)
    /// @brief write frame summary to console (stderr)
    /// @param[in] self    frame reference
    /// @param[in] verbose indent extra output (if implemented)
    /// @param[in] indent  output indentation (spaces)
    void mb1_frame_show(mb1_frame_t *self, bool verbose, uint16_t indent);
#ifdef __cplusplus
}
#endif

#endif // THIS_FILE
