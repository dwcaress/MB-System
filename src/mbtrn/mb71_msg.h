///
/// @file mb71_msg.h
/// @authors k. headley
/// @date 19 aug 2019

/// MB-System record format 71 (FBT)

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
#ifndef MB71_MSG_H
#define MB71_MSG_H

/////////////////////////
// Includes 
/////////////////////////
#include <stdbool.h>
#include <inttypes.h>

/////////////////////////
// Macros
/////////////////////////

/// data sizes : double[8] float[4] int[4] uchar[1] ushort[2] short[2]
/// header size: 5xdouble+9xfloat+4xint+5xuchar+1xushort
/// beams      : beams_bath*(3xshort+1xuchar)
/// amp        : beams_amp*(1xshort)
/// ss         : ss_pix*(3xshort)

/// @def MB71V5_TYPE_ID
/// @brief MB71 record type ID (0x5635:'V''5')
#define MB71V5_TYPE_ID 0x5635
/// @def MB71V5_HEADER_BYTES
/// @brief MB71 record header size (fixed fields, bytes)
#define MB71V5_HEADER_BYTES 98
/// @def MB71V5_BEAM_BYTES
/// @brief MB71 beam data size (bytes)
#define MB71V5_BEAM_BYTES    7
/// @def MB71V5_AMP_BYTES
/// @brief MB71 amplitude data size (bytes)
#define MB71V5_AMP_BYTES     2
/// @def MB71V5_SSPIX_BYTES
/// @brief MB71 sidescan pixel data size (bytes)
#define MB71V5_SSPIX_BYTES   6

/// @def MB71V5_FRAME_BYTES(nbath,namp,sspix)
/// @brief complete MB71 frame size (bytes)
/// @param[in] nbath number of bathymetry beams
/// @param[in] namp  number of amplitude beams
/// @param[in] sspix number of sidescan pixels
#define MB71V5_FRAME_BYTES(nbath,namp,sspix) (MB71V5_HEADER_BYTES+beams*MB71V5_BEAM_BYTES+namp*MB71V5_AMP_BYTES+sspix*MB71V5_SSPIX_BYTES)

/// @def MB71_PBF(pmb71,nbeams)
/// @brief get pointer to beam flags array
/// @param[in] pmb71  mb71_frame_t pointer
/// @param[in] nbeams number of bathymetry beams
#define MB71_PBF(pmb71,nbeams) (NULL!=pmb71 && nbeams>0 ? (unsigned char *)&pmb71->beam_bytes[0*nbeams] : (unsigned char *)NULL)
/// @def MB71_PBZ(pmb71,nbeams)
/// @brief get pointer to beam vertical component array
/// @param[in] pmb71  mb71_frame_t pointer
/// @param[in] nbeams number of bathymetry beams
#define MB71_PBZ(pmb71,nbeams) (NULL!=pmb71 && nbeams>0 ? (short *)&pmb71->beam_bytes[1*nbeams] : (short *)NULL)
/// @def MB71_PBY(pmb71,nbeams)
/// @brief get pointer to beam across-track component array
/// @param[in] pmb71  mb71_frame_t pointer
/// @param[in] nbeams number of bathymetry beams
#define MB71_PBY(pmb71,nbeams) (NULL!=pmb71 && nbeams>0 ? (short *)&pmb71->beam_bytes[3*nbeams] : (short *)NULL)
/// @def MB71_PBX(pmb71,nbeams)
/// @brief get pointer to beam along-track component array
/// @param[in] pmb71  mb71_frame_t pointer
/// @param[in] nbeams number of bathymetry beams
#define MB71_PBX(pmb71,nbeams) (NULL!=pmb71 && nbeams>0 ? (short *)&pmb71->beam_bytes[5*nbeams] : (short *)NULL)

#ifndef MAX
/// @def MAX
/// @brief return larger of a and b
/// @param[in] a number to compare
/// @param[in] b number to compare
#define MAX(a,b) ( (a>b) ? a : b)
#endif

/// @typedef unsigned char byte
/// @brief byte (unsigned char) type
typedef unsigned char byte;

/////////////////////////
// Type Definitions
/////////////////////////
#pragma pack(push,1)

/// @typedef struct beam_dat_s beam_dat_t
/// @brief MB71 beam data format
typedef struct beam_dat_s{
    /// @var mbf71v5_hdr_s::flag
    /// @brief status flag
    unsigned char flag;
    /// @var mbf71v5_hdr_s::beam
    /// @brief beam vertical component
    short beam;
    /// @var mbf71v5_hdr_s::beam_cross
    /// @brief beam cross-track component
    short beam_cross;
    /// @var mbf71v5_hdr_s::
    /// @brief beam along-track component
    short beam_along;
}beam_dat_t;

/// @typedef mbf71v5_s mbf71v5_t
/// @brief MB71 sounding data frame
typedef struct mbf71v5_s{
    /// @var mbf71v5_s::recordtype
    /// @brief record type ID ('V''5')
    unsigned short recordtype;
    /// @var mbf71v5_s::time_d
    /// @brief timestamp (Unix epoch)
    double time_d;
    /// @var mbf71v5_s::longitude
    /// @brief sounding longitude
    double longitude;
    /// @var mbf71v5_s::latitude
    /// @brief sounding latitude
    double latitude;
    /// @var mbf71v5_s::sonardepth
    /// @brief sonar depth
    double sonardepth;
    /// @var mbf71v5_s::altitude
    /// @brief vehicle distance from seafloor
    double altitude;
    /// @var mbf71v5_s::heading
    /// @brief vehicle heading
    float heading;
    /// @var mbf71v5_s::speed
    /// @brief vehicle speed
    float speed;
    /// @var mbf71v5_s::roll
    /// @brief vehicle roll
    float roll;
    /// @var mbf71v5_s::pitch
    /// @brief vehicle pitch
    float pitch;
    /// @var mbf71v5_s::heave
    /// @brief vehicle heave
    float heave;
    /// @var mbf71v5_s::beam_xwidth
    /// @brief cross-track beam width
    float beam_xwidth;
    /// @var mbf71v5_s::
    /// @brief along-track beam width
    float beam_lwidth;
    /// @var mbf71v5_s::beams_bath
    /// @brief number of bathymetry beams
    int beams_bath;
    /// @var mbf71v5_s::beams_amp
    /// @brief number of amplitude beams
    int beams_amp;
    /// @var mbf71v5_s::pixels_ss
    /// @brief number of side scan pixels
    int pixels_ss;
    /// @var mbf71v5_s::spare1spare1
    /// @brief TBD
    int spare1;
    /// @var mbf71v5_s::depth_scale
    /// @brief depth scaling factor
    float depth_scale;
    /// @var mbf71v5_s::distance_scale
    /// @brief distance scaling factor
    float distance_scale;
    /// @var mbf71v5_s::ss_scalepower
    /// @brief sidescan scaling exponent
    unsigned char ss_scalepower;
    /// @var mbf71v5_s::ss_type
    /// @brief side scan type ID
    unsigned char ss_type;
    /// @var mbf71v5_s::imagery_type
    /// @brief imagery type ID
    unsigned char imagery_type;
    /// @var mbf71v5_s::topo_type
    /// @brief topography scheme ID
    unsigned char topo_type;
    /// @var mbf71v5_s::beam_bytes
    /// @brief beam data arrays (bathy, amplitude, sidescan)
    byte beam_bytes[];
}mb71v5_t;
#pragma pack(pop)

/////////////////////////
// Exports
/////////////////////////

#ifdef __cplusplus
extern "C" {
#endif
    /// @fn int mb71v5_bswap(mb71v5_t *dest, mb71v5_t *self)
    /// @brief byte-swap mb71 frame members. Operates on src if dest is NULL.
    /// otherwise, stores result in dest (src does not change)
    /// @param[in] dest output to byte array
    /// @param[in] src mb71 frame data
    int mb71v5_bswap(mb71v5_t *dest, mb71v5_t *src);
     
    /// @fn mb71v5_show(mb71v5_t *self, bool verbose, uint16_t indent)
    /// @brief write summary of mb71_frame_t to console (stderr)
    /// @param[in] self    frame reference
    /// @param[in] verbose indent extra output (if implemented)
    /// @param[in] indent  output indentation (spaces)
    void mb71v5_show(mb71v5_t *self, bool verbose, uint16_t indent);
#ifdef __cplusplus
}
#endif

#endif // THIS_FILE
