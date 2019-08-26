///
/// @file mswap.h
/// @authors k. headley
/// @date 20 aug 2019

/// Byte swap macros
///.Not optimized, but more portable than byteswap.h

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
#ifndef MSWAP_H
#define MSWAP_H

/////////////////////////
// Includes 
/////////////////////////
#include <inttypes.h>

/////////////////////////
// Macros
/////////////////////////
/// @def mswap_16(x)
/// @brief swap 16-bit values
/// @param[n] x value to swap
/// @return byte-swapped value of x
#define mswap_16(x) \
(((((uint16_t)x) >> 8) & 0x00FF) | ((((uint16_t)x) << 8) & 0xFF00))

/// @def mswap_32(x)
/// @brief swap 32-bit values
/// @param[n] x value to swap
/// @return byte-swapped value of x
#define mswap_32(x) \
(((((uint32_t)x) >> 24) & 0x000000FF) | ((((uint32_t)x) >>  8) & 0x0000FF00) | \
((((uint32_t)x) <<  8) & 0x00FF0000) | ((((uint32_t)x) << 24) & 0xFF000000))

/// @def mswap_64(x)
/// @brief swap 64-bit values
/// @param[n] x value to swap
/// @return byte-swapped value of x
#define mswap_64(x) \
(((((uint64_t)x) >> 56) & 0x00000000000000ffULL)| ((((uint64_t)x) >> 40) & 0x000000000000ff00ULL) | \
((((uint64_t)x) >> 24) & 0x0000000000ff0000ULL) | ((((uint64_t)x) >>  8) & 0x00000000ff000000ULL) | \
((((uint64_t)x) <<  8) & 0x000000ff00000000ULL) | ((((uint64_t)x) << 24) & 0x0000ff0000000000ULL) | \
((((uint64_t)x) << 40) & 0x00ff000000000000ULL) | ((((uint64_t)x) << 56) & 0xff00000000000000ULL))

/////////////////////////
// Type Definitions
/////////////////////////

/////////////////////////
// Exports
/////////////////////////

#ifdef __cplusplus
extern "C" {
#endif
    /// @fn int mswap_bytes(unsigned char *dest, unsigned char *src, size_t len)
    /// @brief byte-swap arbitrary length byte arrays
    /// @param[n] dest output array (may be NULL)
    /// @param[n] src values to swap
    /// @param[n] len number of bytes (>0, even)
    /// @return 0 on success, -1 otherwise (invalid argument(s)).
    /// swaps src in place if dest is NULL, otherwise, sets dest and src is unchanged
    int  mswap_bytes(void *dest, void *src, size_t len);
#ifdef __cplusplus
}
#endif

#endif // THIS_FILE
