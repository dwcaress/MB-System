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
#define MB1_MAX_BEAMS 512
#define MB1_HEADER_FIELDS   2
#define MB1_BEAM_FIELDS     4
#define MB1_SOUNDING_FIELDS 7
#define MB1_SOUNDING_BEAM_FIELDS (MB1_MAX_BEAMS*MB1_BEAM_FIELDS)
#define MB1_FIXED_FIELDS (MB1_SOUNDING_FIELDS+MB1_HEADER_FIELDS)
#define MB1_FIELDS (MB1_SOUNDING_FIELDS+MB1_SOUNDING_BEAM_FIELDS+MB1_HEADER_FIELDS)
#define MB1_BIN_HEADER_BYTES   56
#define MB1_BIN_SOUNDING_BYTES 28
#define MB1_BIN_CHECKSUM_BYTES 4
#define MB1_BIN_MAX_BYTES (MB1_BIN_HEADER_BYTES+MB1_MAX_BEAMS*MB1_BIN_SOUNDING_BYTES+MB1_BIN_CHECKSUM_BYTES)

/////////////////////////
// Type Definitions
/////////////////////////

#pragma pack(push,1)
typedef struct mb1_beam_s
{
     // beam number (0 is port-most beam)
    unsigned int beam_num;
    // along track position wrt sonar meters
    double rhox;
    // cross track position wrt soanr meters
    double rhoy;
    // vertical position wrt to sonar meters (positive down)
    double rhoz;
}mb1_beam_t;

// MB-TRN sounding data (all beams) with vehicle context
typedef struct mb1_sounding_s
{
    // epoch time of ping
    double ts;
    // epoch time of ping
    double lat;
    // epoch time of ping
    double lon;
    // vehicle position depth meters
    double depth;
    // epoch time of ping
    double hdg;
    // epoch time of ping
    int ping_number;
    // epoch time of ping
    unsigned int nbeams;
    mb1_beam_t beams[MB1_MAX_BEAMS];
}mb1_sounding_t;

// Header for MB-TRN communication packets
typedef struct mb1_header_s
{
    unsigned int type;
    unsigned int size;
}mb1_header_t;


// MB-TRN MB1 communications packet consists of header and sounding data
typedef struct mb1_s
{
    mb1_header_t    header;
    mb1_sounding_t  sounding;
}mb1_t;

//// Header for MB-TRN communication packets
//typedef struct mb1_bin_header_s
//{
//    // 'M''B''1''\0'
//    unsigned int type;
//    // total bytes, including header and checksum
//    unsigned int size;
//    // epoch time of ping
//    double ts;
//    // epoch time of ping
//    double lat;
//    // epoch time of ping
//    double lon;
//    // vehicle position depth meters
//    double depth;
//    // epoch time of ping
//    double hdg;
//    // epoch time of ping
//    int ping_number;
//    // epoch time of ping
//    unsigned int nbeams;
//}mb1_bin_header_t;
//
//typedef struct mb1_bin_s
//{
//    mb1_bin_header_t header;
//    mb1_beam_t beams[MB1_MAX_BEAMS];
//    uint32_t checksum;
//}mb1_bin_t;

#pragma pack(pop)

/////////////////////////
// Exports
/////////////////////////

#ifdef __cplusplus
extern "C" {
#endif
    mb1_t *mb1_new(unsigned int type, int beams);
    void mb1_destroy(mb1_t **pself);
    void mb1_show(mb1_t *self, bool verbose, uint16_t indent);

#ifdef __cplusplus
}
#endif

#endif // THIS_FILE
