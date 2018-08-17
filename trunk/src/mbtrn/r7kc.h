///
/// @file r7kc.h
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
#ifndef MBTRN_RESON_7K_H
/// @def MBTRN_RESON_7K_H
/// @brief include guard
#define MBTRN_RESON_7K_H

/////////////////////////
// Includes 
/////////////////////////

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "iowrap.h"
// MB System 7k center structure
// definitions and record types
//#include "mbsys_reson7k.h"

/////////////////////////
// Type Definitions
/////////////////////////

/// @typedef struct r7k_empty_rec_s r7k_unsub_t
/// @brief unsubscribe message type
typedef struct r7k_empty_rec_s r7k_unsub_t;
/// @typedef uint32_t r7k_checksum_t
/// @brief r7k data record frame checksum type
typedef uint32_t  r7k_checksum_t;

#pragma pack(push, 1)
/// @typedef struct r7k_time_s r7k_time_t
/// @brief r7k time structure
typedef struct r7k_time_s
{
    /// @var r7k_time_s::year
    /// @brief year
    uint16_t year;
    /// @var r7k_time_s::day
    /// @brief day
    uint16_t day;
    /// @var r7k_time_s::seconds
    /// @brief seconds
    float seconds;
    /// @var r7k_time_s::hours
    /// @brief hours
    uint8_t hours;
    /// @var r7k_time_s::minutes
    /// @brief minutes
    uint8_t minutes;
}r7k_time_t;

/// @typedef struct r7k_nf_s r7k_nf_t
/// @brief 7k center network frame (NF) structure
/// @sa 7k center Data Format Definition document
typedef struct r7k_nf_s
{
    /// @var r7k_nf_s::protocol_version
    /// @brief protocol version
    uint16_t protocol_version;
    /// @var r7k_nf_s::offset
    /// @brief Offset in bytes to the start of data from the start of this packet
    uint16_t offset;
    /// @var r7k_nf_s::total_packets
    /// @brief Number of network packets for set of records transmitted
    uint32_t total_packets;
    /// @var r7k_nf_s::total_records
    /// @brief always 1
    uint16_t total_records;
    /// @var r7k_nf_s::tx_id
    /// @brief Transmission identifier (helper field for packet assembly).
    /// Must be the same number for each network packet in transmission.
    /// Adjacent transmissions in time from one source may not use the same identifier.
    uint16_t tx_id;
    /// @var r7k_nf_s::packet_size
    /// @brief Size in bytes of this packet including the header and appended data
    uint32_t packet_size;
    /// @var r7k_nf_s::total_size
    /// @brief Total size in bytes of all packets in transmission, excluding network frame(s)
    uint32_t total_size;
    /// @var r7k_nf_s::seq_number
    /// @brief Sequential packet number; allows correct ordering during reconstruction. Range = 0 to n-1 packets.
    uint32_t seq_number;
    /// @var r7k_nf_s::dest_dev_id
    /// @brief 0 – Unspecified
    /// 0xFFFFFFFF – Not used
    /// Any other number is a valid address
    uint32_t dest_dev_id;
    /// @var r7k_nf_s::dest_enumerator
    /// @brief Destination enumerator unless destination device identifier is unspecified or not used
    uint16_t dest_enumerator;
    /// @var r7k_nf_s::src_enumerator
    /// @brief Source enumerator unless Source Device Identifier is unspecified or not used
    uint16_t src_enumerator;
    /// @var r7k_nf_s::src_dev_id
    /// @brief 0 – Unspecified
    /// 0xFFFFFFFF – Not used
    /// Any other number is a valid address
    uint32_t src_dev_id;
}r7k_nf_t;

/// @typedef struct r7k_drf_s r7k_drf_t
/// @brief Data Record Frame (DRF) header structure.
/// Defined in 7k center Data Format Definition document.
/// Does not contain references to (optional) data or (required) checksum
/// that are part of valid data record frames.
/// @sa 7k center Data Format Definition document
typedef struct r7k_drf_s
{
    /// @var r7k_drf_s::protocol_version
    /// @brief Protocol version of this frame
    uint16_t protocol_version;
    /// @var r7k_drf_s::offset
    /// @brief Offset in bytes from the start of the sync pattern to the start of the Record Type Header (RTH). This allows for expansion of the header whilst maintaining backward compatibility.
    uint16_t offset;
    /// @var r7k_drf_s::sync_pattern
    /// @brief 0x0000FFFF
    uint32_t sync_pattern;
    /// @var r7k_drf_s::size
    /// @brief Size in bytes of this record from the start of the Protocol version field to the end of the checksum field — including any embedded data.
    uint32_t size;
    /// @var r7k_drf_s::opt_data_offset
    /// @brief Offset in bytes to optional data field from start of record. Zero (0) bytes implies no optional data.
    uint32_t opt_data_offset;
    /// @var r7k_drf_s::opt_data_id
    /// @brief User defined
    uint32_t opt_data_id;
    /// @var r7k_drf_s::_7ktime
    /// @brief Time tag indicating when data was produced.
    r7k_time_t _7ktime;
    /// @var r7k_drf_s::record_version
    /// @brief  Currently 1
    uint16_t record_version;
    /// @var r7k_drf_s::record_type_id
    /// @brief Identifier for record type of embedded data
    uint32_t record_type_id;
    /// @var r7k_drf_s::device_id
    /// @brief Identifier of the device to which this data pertains. See appendix B for a overview.
    uint32_t device_id;
    /// @var r7k_drf_s::reserved0
    /// @brief Reserved
    uint16_t reserved0;
    /// @var r7k_drf_s::sys_enumerator
    /// @brief The enumerator is used to differentiate between devices with the same device identifiers in one installation/system. For example, on 7125 200khz/400kHz dual- frequency systems, the enumerator will normally be zero (0) in 200khz mode, and one (1) in 400kHz mode.
    uint16_t sys_enumerator;
    /// @var r7k_drf_s::reserved1
    /// @brief Reserved
    uint32_t reserved1;
    /// @var r7k_drf_s::flags
    /// @brief BIT FIELD:
    ///     Bit 0: Checksum
    ///     0 – Invalid checksum
    ///     1 – Valid checksum
    ///     Bit 1-14: Reserved (must be zero) Bit 15:
    ///     0 – Live data
    ///     1 – Recorded data
    uint16_t flags;
    /// @var r7k_drf_s::reserved2
    /// @brief Reserved
    uint16_t reserved2;
    /// @var r7k_drf_s::reserved3
    /// @brief Reserved
    uint32_t reserved3;
    /// @var r7k_drf_s::total_frag_recs
    /// @brief Always zero
    uint32_t total_frag_recs;
    /// @var r7k_drf_s::frag_number
    /// @brief Always zero
    uint32_t frag_number;
}r7k_drf_t;

/// @typedef struct r7k_drf_container_s r7k_drf_container_t
/// @brief Data Record Frame (DRF) container structure
typedef struct r7k_drf_container_s
{
    /// @var r7k_drf_container_s::size
    /// @brief capacity of data buffer (bytes)
    uint32_t size;
    /// @var r7k_drf_container_s::record_count
    /// @brief number of frames currently in container
    uint32_t record_count;
    /// @var r7k_drf_container_s::data
    /// @brief container data buffer
    byte *data;
    /// @var r7k_drf_container_s::p_read
    /// @brief buffer read/output pointer
    byte *p_read;
    /// @var r7k_drf_container_s::p_write
    /// @brief buffer write/input pointer
    byte *p_write;
    /// @var r7k_drf_container_s::ofs_sz
    /// @brief size of DRF offset array
    uint32_t ofs_sz;
    /// @var r7k_drf_container_s::ofs_count
    /// @brief actual number of DRF offsets
    uint32_t ofs_count;
    /// @var r7k_drf_container_s::ofs_list
    /// @brief DRF offset array
    uint32_t *ofs_list;
    /// @var r7k_drf_container_s::drf_enum
    /// @brief frame enumerator cursor
    uint32_t drf_enum;
    // mutex, if thread-safe required
}r7k_drf_container_t;

/// @typedef struct r7k_parse_stat_s r7k_parse_stat_t
/// @brief r7k raw data parser status information
typedef struct r7k_parse_stat_s
{
    /// @var r7k_parse_stat_s::src_bytes
    /// @brief size of source data
    uint32_t src_bytes;
    /// @var r7k_parse_stat_s::sync_bytes
    /// @brief number of bytes skipped due to parsing sync errors
    uint32_t sync_bytes;
    /// @var r7k_parse_stat_s::unread_bytes
    /// @brief number of unread source bytes (remaining bytes < valid record size)
    uint32_t unread_bytes;
    /// @var r7k_parse_stat_s::parsed_records
    /// @brief number of valid data record frames (DRF) parsed
    uint32_t parsed_records;
    /// @var r7k_parse_stat_s::parsed_bytes
    /// @brief total size of parsed frames
    uint32_t parsed_bytes;
    /// @var r7k_parse_stat_s::resync_count
    /// @brief number of sync errors
    uint32_t resync_count;
    /// @var r7k_parse_stat_s::status
    /// @brief exit status: ME_ error or ME_OK
    int status;
}r7k_parse_stat_t;

/// @typedef struct r7k_msg_s r7k_msg_t
/// @brief r7k message structure
typedef struct r7k_msg_s
{
    /// @var r7k_msg_s::msg_len
    /// @brief length of message (bytes)
    uint32_t msg_len;
    /// @var r7k_msg_s::nf
    /// @brief Network Frame (NF) structure reference
    r7k_nf_t *nf;
    /// @var r7k_msg_s::drf
    /// @brief Data Record Frame (DRF) structure reference
    r7k_drf_t *drf;
    /// @var r7k_msg_s::data_size
    /// @brief size of message data (bytes)
    uint32_t data_size;
    /// @var r7k_msg_s::data
    /// @brief message data buffer
    byte *data;
    /// @var r7k_msg_s::checksum
    /// @brief DRF checksum value
    r7k_checksum_t checksum;
}r7k_msg_t;

/// @typedef struct r7k_frame_info_s r7k_frame_info_t
/// @brief r7k message structure
//typedef struct r7k_frame_info_s
//{
//    /// @var r7k_msg_s::msg_len
//    /// @brief length of message (bytes)
//    uint32_t total_len;
//    /// @var r7k_msg_s::data_size
//    /// @brief size of message data (bytes)
//    uint32_t data_len;
//    /// @var r7k_msg_s::timestamp
//    /// @brief frame timestamp
//    double timestamp;
////    /// @var r7k_msg_s::nf
////    /// @brief Network Frame (NF) structure reference
////    byte *nf;
////    /// @var r7k_msg_s::drf
////    /// @brief Data Record Frame (DRF) structure reference
////    byte *drf;
////    /// @var r7k_msg_s::data
////    /// @brief message data buffer
////    byte *data;
////    /// @var r7k_msg_s::checksum
////    /// @brief DRF checksum value
////    byte *checksum;
//    /// @var r7k_msg_s::checksum
//    /// @brief DRF checksum value
//    uint32_t cs_ofs;
//
//}r7k_frame_info_t;

/// @typedef struct r7k_nf_headers_s r7k_nf_headers_t
/// @brief 7k center network frame headers. does not include data or checksum.
typedef struct r7k_nf_headers_s
{
    /// @var r7k_nf_headers_s::nf
    /// @brief Network Frame (NF) header
    r7k_nf_t nf;
    /// @var r7k_nf_headers_s::drf
    /// @brief Data Record Frame (DRF) header
    r7k_drf_t drf;
}r7k_nf_headers_t;

/// @typedef struct r7k_empty_drf_s r7k_empty_drf_t
/// @brief DRF header and checksum (used with data len for computing DRF size)
typedef struct r7k_empty_drf_s
{
    /// @var r7k_empty_drf_s::drf
    /// @brief Data Record Frame (DRF) header
    r7k_drf_t drf;
    /// @var r7k_empty_drf_s::checksum
    /// @brief DRF checksum
    uint32_t checksum;
}r7k_empty_drf_t;

/// @typedef struct r7k_empty_nf_s r7k_empty_nf_t
/// @brief 7k center network frame headers. does not include data or checksum.
/// @sa Reson Data Record Format documentation
typedef struct r7k_empty_nf_s
{
    /// @var r7k_empty_nf_s::nf
    /// @brief Network Frame (NF) header
    r7k_nf_t nf;
    /// @var r7k_empty_nf_s::drf
    /// @brief Data Record Frame (DRF) header
    r7k_empty_drf_t drf;
}r7k_empty_nf_t;


/// @typedef struct r7k_rth_7500_rc_s r7k_rth_7500_rc_t
/// @brief 7k center record type header : remote control message
/// @sa Reson Data Record Format documentation
typedef struct r7k_rth_7500_rc_s{
    /// @var r7k_rth_7500_rc_s::remcon_id
    /// @brief See separate remote control table for details. See section 10 Record Type Definitions.
    /// @sa Reson Data Record Format documentation
    uint32_t remcon_id;
    /// @var r7k_rth_7500_rc_s::ticket
    /// @brief Ticket number. Set by client for control packet matching ACK or NAK packets.
    uint32_t ticket;
    /// @var r7k_rth_7500_rc_s::tracking_number
    /// @brief Unique number. Set by client for packet tracking.
    byte tracking_number[16];
}r7k_rth_7500_rc_t;

/// @typedef struct r7k_rth_7501_ack_s r7k_rth_7501_ack_t
/// @brief 7k center record type header : 7501 message ACK.
/// @sa Reson Data Record Format documentation
typedef struct r7k_rth_7501_ack_s{
    /// @var r7k_rth_7501_ack_s::ticket
    /// @brief Ticket number.
    uint32_t ticket;
    /// @var r7k_rth_7501_ack_s::tracking_number
    /// @brief Unique number in record 7500.
    byte tracking_number[16];
}r7k_rth_7501_ack_t;

/// @typedef struct r7k_sub_rd_s r7k_sub_rd_t
/// @brief reson 7k center record data : subscribe message data.
/// @sa Reson Data Record Format documentation
typedef struct r7k_sub_rd_s
{
    /// @var r7k_sub_rd_s::record_count
    /// @brief number of records requested.
    /// message record data also includes array of uint32 (message IDs).
    /// i.e. uint32_t records[record_count];
    uint32_t record_count;
    // followed by
    // uint32_t records[record_count];
}r7k_sub_rd_t;

/// @typedef struct r7k_rth_rcack_s r7k_rth_rcack_t
/// @brief 7k center record type header : 7501 remote control ACK.
/// @sa Reson Data Record Format documentation
typedef struct r7k_rth_rcack_s
{
    /// @var r7k_rth_rcack_s::ticket
    /// @brief ticket number.
    uint32_t ticket;
    /// @var r7k_rth_rcack_s::tracking_number.
    /// @brief Unique number in record 7500.
    byte tracking_number[16];
}r7k_rth_rcack_t;

/// @typedef struct r7k_rd_rcnak_s r7k_rd_rcnak_t
/// @brief 7k center record type header : 7501 remote control NACK.
/// @sa Reson Data Record Format documentation
typedef struct r7k_rd_rcnak_s
{
    /// @var r7k_rd_rcnak_s::ticket
    /// @brief TBD
    uint32_t ticket;
    /// @var r7k_rd_rcnak_s::tracking_number
    /// @brief ticket number
    byte tracking_number[16];
    /// @var r7k_rd_rcnak_s::error_code
    /// @brief error code
    uint32_t error_code;
}r7k_rd_rcnak_t;

#pragma pack(pop)

/////////////////////////
// Macros
/////////////////////////

/// @def R7K_7KCENTER_PORT
/// @brief 7K Center IP port.
#define R7K_7KCENTER_PORT       7000
/// @def R7K_RT_REMCON
/// @brief 7K Center message type ID remote control.
#define R7K_RT_REMCON           7500
/// @def R7K_RT_REMCON_ACK
/// @brief 7K Center message type ID remote control ACK.
#define R7K_RT_REMCON_ACK       7501
/// @def R7K_RT_REMCON_NACK
/// @brief 7K Center message type ID remote control NACK.
#define R7K_RT_REMCON_NACK      7502
/// @def R7K_RT_CONFIG_DATA
/// @brief 7K Center message type config data.
#define R7K_RT_CONFIG_DATA      7001
/// @def R7K_RT_RC_SONAR
/// @brief 7K Center message type remote control sonar.
#define R7K_RT_RC_SONAR         7503
/// @def R7K_RT_SYSTEM_STATE
/// @brief 7K Center message type system state.
#define R7K_RT_SYSTEM_STATE     7503
// max frame bytes is defined in Reson DRF.
/// @def R7K_MAX_FRAME_BYTES
/// @brief max 7k center frame size (bytes).
#define R7K_MAX_FRAME_BYTES    600000 //60000
// max frames/record is a guess
/// @def R7K_MAX_RECORD_FRAMES
/// @brief max record frames per ping (empirical/estimate).
#define R7K_MAX_RECORD_FRAMES     16
// max record bytes depends on
// number of frames in a record
/// @def R7K_MAX_RECORD_BYTES
/// @brief approximate maximum record size (bytes).
#define R7K_MAX_RECORD_BYTES    ((uint32_t)(R7K_MAX_RECORD_FRAMES*R7K_MAX_FRAME_BYTES))
// max bytes per ping depends on
// number/type of messages, frames
// per record, etc.
/// @def R7K_MAX_PING_RECORDS
/// @brief guess number of records per ping. Depends on
/// number/type of messages, frames
/// per record, etc.
#define R7K_MAX_PING_RECORDS 32


/// @def R7K_RTID_SUB
/// @brief 7K Center message type subscribe to message streams.
#define R7K_RTID_SUB            1051
/// @def R7K_RTID_UNSUB
/// @brief 7K Center message type unsubscribe from message streams.
#define R7K_RTID_UNSUB          1052

/// @def R7K_NF_PROTO_VER
/// @brief Network Frame protocol version.
#define R7K_NF_PROTO_VER        5
/// @def R7K_NF_DEVID_UNUSED
/// @brief Network Frame device ID unused value.
#define R7K_NF_DEVID_UNUSED     0xFFFFFFFF

/// @def R7K_DEVID_7KCENTER
/// @brief 7K Center device ID : 7K Center.
#define R7K_DEVID_7KCENTER      7000
/// @def R7K_DEVID_7KCENTER_UI
/// @brief 7K Center device ID : 7K Center User Interface.
#define R7K_DEVID_7KCENTER_UI   7001
/// @def R7K_DEVID_7KLOGGER
/// @brief 7K Center device ID : 7K Center logger.
#define R7K_DEVID_7KLOGGER      7004

/// @def R7K_DRF_PROTO_VER
/// @brief Data Record Frame protocol version.
#define R7K_DRF_PROTO_VER       5
/// @def R7K_DRF_SYS_ENUM_200KHZ
/// @brief Data Record Frame system enumerator 200khz.
#define R7K_DRF_SYS_ENUM_200KHZ 0
/// @def R7K_DRF_SYS_ENUM_400KHZ
/// @brief Data Record Frame system enumerator 400khz.
#define R7K_DRF_SYS_ENUM_400KHZ 1
/// @def R7K_DRF_RECORD_VER
/// @brief Data Record Frame record version.
#define R7K_DRF_RECORD_VER      1
/// @def R7K_DRF_SYNC_PATTERN
/// @brief TBD
#define R7K_DRF_SYNC_PATTERN    0x0000FFFF
/// @def R7K_DRF_SYNC_OFFSET
/// @brief Data Record Frame offset of sync pattern.
/// (in bytes, relative to start of DRF)
#define R7K_DRF_SYNC_OFFSET     (2*sizeof(uint16_t))

/// @def R7K_EMPTY_FRAME_BYTES
/// @brief size of empty network frame headers (nf, drf, w/o data, checksum).
#define R7K_EMPTY_FRAME_BYTES (sizeof(r7k_empty_nf_t))
/// @def R7K_FRAME_HEADER_BYTES
/// @brief size of empty network frame headers (nf, drf only).
#define R7K_FRAME_HEADER_BYTES (sizeof(r7k_nf_headers_t))
/// @def R7K_NF_BYTES
/// @brief size of NF header
/// (sizeof(r7k_nf_t))
#define R7K_NF_BYTES 36

/// @def R7K_DRF_BYTES
/// @brief size of DRF header (not including data or checksum).
/// (sizeof(r7k_drf_t))
#define R7K_DRF_BYTES 64

/// @def R7K_CHECKSUM_BYTES
/// @brief size of DRF checksum
/// (sizeof(r7k_checksum_t))
#define R7K_CHECKSUM_BYTES 4

/// @def R7K_NF_PROTO_BYTES
/// @brief size of NF protocol version (bytes)
#define R7K_NF_PROTO_BYTES (sizeof(uint16_t))

/// @def R7K_DRF_PROTO_BYTES
/// @brief size of DRF protocol version (bytes)
#define R7K_DRF_PROTO_BYTES (sizeof(uint16_t))

/// @def R7K_NF_OFS2PTR(bp,ofs)
/// @brief cast byte pointer, offset to network frame pointer.
/// @param[in] bp byte pointer
/// @param[in] ofs offset
/// @return network frame pointer
#define R7K_NF_OFS2PTR(bp,ofs) ((r7k_nf_t *)(bp+ofs))
/// @def R7K_DRF_OFS2PTR(bp,ofs)
/// @brief cast byte pointer, offset to data record frame pointer.
/// @param[in] bp byte pointer
/// @param[in] ofs offset
/// @return data record frame pointer
#define R7K_DRF_OFS2PTR(bp,ofs) ((r7k_drf_t *)(bp+ofs))
/// @def R7K_CHK_OFS2PTR(bp,ofs)
/// @brief cast byte pointer, offset to checksum pointer
/// @param[in] bp byte pointer
/// @param[in] ofs offset
/// @return checksum pointer
#define R7K_CHK_OFS2PTR(bp,ofs) ((r7k_checksum_t *)(bp+ofs))
/// @def R7K_MSG_DRF_SIZE(m)
/// @brief size of data record frame (including data, checksum).
/// @param[in] m message pointer
/// @return data record frame size (bytes)
#define R7K_MSG_DRF_SIZE(m) (R7K_DRF_BYTES+m->data_size+R7K_CHECKSUM_BYTES)

/// @def R7K_MSG_NF_PACKET_SIZE(m)
/// @brief network frame packet size value
/// @param[in] m message pointer
/// @return network frame packet size (bytes)
//#define R7K_MSG_NF_PACKET_SIZE(m) (m->msg_len-R7K_CHECKSUM_BYTES)
#define R7K_MSG_NF_PACKET_SIZE(m) (R7K_DRF_BYTES+m->data_size+R7K_CHECKSUM_BYTES+R7K_NF_BYTES)

/// @def R7K_MSG_NF_TOTAL_SIZE(m)
/// @brief network frame total size value
/// @param[in] m message pointer
/// @return network frame total size (bytes)
#define R7K_MSG_NF_TOTAL_SIZE(m) (R7K_DRF_BYTES+m->data_size+R7K_CHECKSUM_BYTES)

/// @def R7K_MSG_NF_OFFSET(m)
/// @brief network frame offset value
/// @param[in] m message pointer
/// @return network frame offset (bytes)
#define R7K_MSG_NF_OFFSET(m) (R7K_NF_BYTES)

/// @def R7K_DRFC_SIZE_INC
/// @brief default data record frame container buffer size (bytes).
#define R7K_DRFC_SIZE_INC 10240
/// @def R7K_DRFC_RECORD_INC
/// @brief data record frame container offset array allocation.
/// used to increment size of record offset array as records are added.
/// i.e. when it fills, add space for 16 offsets
#define R7K_DRFC_RECORD_INC 16

/// @def R7K_SUBSCRIBE_TIMEOUT_MS
/// @brief timeout for socket IO during subscription transaction.
#define R7K_SUBSCRIBE_TIMEOUT_MS 5000

#define SEC_PER_MIN (60)
#define SEC_PER_HOUR (SEC_PER_MIN*60)
#define SEC_PER_DAY (SEC_PER_HOUR*24)

#define S_PER_M ((double)60.0)
#define S_PER_H ((double)S_PER_M*60.0)
#define S_PER_D ((double)S_PER_H*24.0)
#define S_PER_Y ((double)S_PER_D*365.0)

/////////////////////////
// Exports
/////////////////////////

// R7K utility API

int  r7k_subscribe(iow_socket_t *s, uint32_t *records, uint32_t len);
int  r7k_unsubscribe(iow_socket_t *s);

uint16_t r7k_txid();
uint32_t r7k_checksum(byte *pdata, uint32_t len);
void r7k_update_time(r7k_time_t *t7k);
void r7k_hex_show(byte *data, uint32_t len, uint16_t cols, bool show_offsets, uint16_t indent);
int r7k_stream_show(iow_socket_t *s, int sz, uint32_t tmout_ms, int cycles);

// R7K packet frame (DRF/NF) API
double r7k_7ktime2d(r7k_time_t *r7kt);

r7k_nf_t  *r7k_nf_new();
void r7k_nf_destroy(r7k_nf_t **pself);
void r7k_drf_init(r7k_drf_t *drf, bool erase);
void r7k_drf_show(r7k_drf_t *self, bool verbose, uint16_t indent);
r7k_checksum_t r7k_drf_get_checksum(r7k_drf_t *self);

r7k_drf_t *r7k_drf_new();
void r7k_drf_destroy(r7k_drf_t **pself);
r7k_nf_t * r7k_nf_init(r7k_nf_t **pnf, bool erase);
void r7k_nf_show(r7k_nf_t *self, bool verbose, uint16_t indent);

// DRF container API

r7k_drf_container_t *r7k_drfcon_new(uint32_t size);
void r7k_drfcon_destroy(r7k_drf_container_t **pself);
void r7k_drfcon_show(r7k_drf_container_t *self, bool verbose, uint16_t indent);
uint32_t r7k_parse(byte *src, uint32_t len, r7k_drf_container_t *dest, r7k_parse_stat_t *status);

int r7k_drfcon_add(r7k_drf_container_t *self, byte *src, uint32_t len);
uint32_t r7k_drfcon_read(r7k_drf_container_t *self, byte *dest, uint32_t len);
int r7k_drfcon_seek(r7k_drf_container_t *self, uint32_t ofs);
uint32_t r7k_drfcon_tell(r7k_drf_container_t *self);
int r7k_drfcon_flush(r7k_drf_container_t *self);
int r7k_drfcon_trim(r7k_drf_container_t *self);
int r7k_drfcon_resize(r7k_drf_container_t *self, uint32_t new_size);
uint32_t r7k_drfcon_frames(r7k_drf_container_t *self);
uint32_t r7k_drfcon_size(r7k_drf_container_t *self);
uint32_t r7k_drfcon_pending(r7k_drf_container_t *self);
uint32_t r7k_drfcon_space(r7k_drf_container_t *self);
uint32_t r7k_drfcon_length(r7k_drf_container_t *self);

int r7k_drfcon_bytes(r7k_drf_container_t *self, uint32_t ofs, byte *dest, uint32_t len);
r7k_drf_t* r7k_drfcon_enumerate(r7k_drf_container_t *self);
r7k_drf_t* r7k_drfcon_next(r7k_drf_container_t *self);

// r7k_msg API (used by mbtrn_server)

r7k_msg_t *r7k_msg_new(uint32_t data_len);
void r7k_msg_destroy(r7k_msg_t **pself);
void r7k_msg_show(r7k_msg_t *self, bool verbose, uint16_t indent);
uint32_t r7k_msg_size(r7k_msg_t *self);
uint32_t r7k_msg_set_checksum(r7k_msg_t *self);
byte* r7k_msg_serialize(r7k_msg_t *self);
int r7k_msg_send(iow_socket_t *s, r7k_msg_t *self);
int r7k_msg_receive(iow_socket_t *s, r7k_msg_t **dest, uint32_t timeout_msec);

// unit tests

int r7k_test();


// include guard
#endif