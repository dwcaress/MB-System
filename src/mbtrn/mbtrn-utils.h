///
/// @file mbtrn-utils.h
/// @authors k. headley
/// @date 06 nov 2012
 
/// MBTRN utility common definitions
 
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
#ifndef MBTRN_UTILS_H
#define MBTRN_UTILS_H

/////////////////////////
// Includes 
/////////////////////////
#include "mbtrn_types.h"

/////////////////////////
// Type Definitions
/////////////////////////

/// @typedef struct mbtrn_beam_data mbtrn_beam_data_t
/// @brief TRN beam data type
typedef struct mbtrn_beam_data mbtrn_beam_data_t;
/// @typedef struct mbtrn_sounding mbtrn_sounding_t
/// @brief TRN sounding data type
typedef struct mbtrn_sounding mbtrn_sounding_t;
/// @typedef struct mbtrn_header mbtrn_header_t
/// @brief TRN header type
typedef struct mbtrn_header mbtrn_header_t;
/// @typedef struct mbtrn_mb1 mbtrn_mb1_t
/// @brief TRN message type
typedef struct mbtrn_mb1 mbtrn_mb1_t;
#pragma pack(push, 1)
/// @typedef struct trn_message_s trn_message_t
/// @brief trn binary message structure
typedef struct trn_message_s{
    /// @var trn_record_s::data
    /// @brief TBD
    mbtrn_mb1_t data;
    /// @var trn_record_s::chksum
    /// @brief TBD
    uint32_t chksum;
}trn_message_t;
#pragma pack(pop)

/////////////////////////
// Macros
/////////////////////////
/// @def MBTRN_HEADER_TYPE_BYTES
/// @brief TRN message header length
#define MBTRN_HEADER_TYPE_BYTES 4
/// @def MBTRN_CHKSUM_BYTES
/// @brief TRN message checksum length
#define MBTRN_CHKSUM_BYTES (sizeof(uint32_t))
/// @def MBTRN_BEAM_BYTES
/// @brief TRN beam data length
#define MBTRN_BEAM_BYTES (sizeof(struct mbtrn_beam_data))
/// @def MBTRN_MSG_BYTES
/// @brief TRN message length
#define MBTRN_MAX_MSG_BYTES (sizeof(trn_message_t))
/// @def MBTRN_MSGTYPE_ACK
/// @brief TRN message type ACK
#define MBTRN_MSGTYPE_ACK (0x004B4341)
/// @def MBTRN_MSGTYPE_MB1
/// @brief TRN message type MB1
#define MBTRN_MSGTYPE_MB1 (0x0031424D)

/////////////////////////
// Exports
/////////////////////////

// include guard
#endif