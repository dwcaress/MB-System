///
/// @file mb1_msg.c
/// @authors k. headley
/// @date 11 aug 2019

/// MB1 message API

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

/////////////////////////
// Headers 
/////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// use older ifdef with headers for QNX makedep compatibility
#ifdef __QNX__
#include <ourTypes.h>
#endif
#if defined (__UNIX__) || defined (__unix__) || defined (__APPLE__)
#include <stdbool.h>
#include <inttypes.h>
#endif
#include "mb1_msg.h"

/////////////////////////
// Macros
/////////////////////////

// These macros should only be defined for 
// application main files rather than general C files
/*
 /// @def PRODUCT
 /// @brief header software product name
 #define PRODUCT "TBD_PRODUCT"

 /// @def COPYRIGHT
 /// @brief header software copyright info
 #define COPYRIGHT "Copyright 2002-YYYY MBARI Monterey Bay Aquarium Research Institute, all rights reserved."
 /// @def NOWARRANTY
 /// @brief header software terms of use
 #define NOWARRANTY  \
 "This program is distributed in the hope that it will be useful,\n"\
 "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"\
 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"\
 "GNU General Public License for more details (http://www.gnu.org/licenses/gpl-3.0.html)\n"
 */

/////////////////////////
// Declarations 
/////////////////////////

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////

/////////////////////////
// Function Definitions
/////////////////////////

mb1_t *mb1_new(uint32_t beams)
{
    mb1_t *instance = NULL;

    uint32_t alloc_size = MB1_SOUNDING_BYTES(beams);
    instance = (mb1_t *)malloc(alloc_size);

    if(NULL != instance) {
        memset(instance,0,alloc_size);
        instance->type = MB1_TYPE_ID;
        instance->size=alloc_size;
        instance->ts=0;
        instance->lat=0.;
        instance->lon=0.;
        instance->depth=0.;
        instance->hdg=0.;
        instance->ping_number=0;
        instance->nbeams=beams;
        mb1_set_checksum(instance);
    }

    return instance;
}
// End function mb1_new

void mb1_destroy(mb1_t **pself)
{
    if(pself) {
        mb1_t *self = *pself;
        if(self) {
            free(self);
        }
        *pself=NULL;
    }
}
// End function mb1_destroy

mb1_t *mb1_resize(mb1_t **pself, uint32_t beams,  int flags)
{
    mb1_t *instance =NULL;

    if(NULL != pself) {
        uint32_t alloc_size = MB1_SOUNDING_BYTES(beams);
        if(NULL == *pself) {
            instance = mb1_new(beams);
        } else {
            instance = (mb1_t *)realloc((*pself), alloc_size);
        }
        if(NULL != instance) {
            instance->type = MB1_TYPE_ID;
            instance->size = alloc_size;
            instance->nbeams = beams;
            // always clears checksum (caller must set)
            mb1_zero(instance, flags);
            *pself = instance;
        }
    }
    return instance;
}
// End function mb1_new

int mb1_zero(mb1_t *self, int flags)
{
    int retval = -1;
    if(NULL != self) {

        uint32_t beams = self->nbeams;

        if(beams <= MB1_MAX_BEAMS) {

            if(flags == MB1_RS_ALL) {
                uint32_t zlen = MB1_SOUNDING_BYTES(beams);
                memset((byte *)self,0,zlen);
            } else {
#if defined(__QNX__)
                mb1_checksum_t *pchk = NULL;
#endif
                if(flags & MB1_RS_BEAMS) {
                    mb1_beam_t *pbeams = MB1_PBEAMS(self);
                    memset((byte *)pbeams, 0, MB1_BEAM_ARRAY_BYTES(beams));
                }

                if(flags & MB1_RS_HEADER) {
                    memset((byte *)self, 0, MB1_HEADER_BYTES);
                }
#if defined(__QNX__)
                // always clear the checksum
                pchk = MB1_PCHECKSUM(self);
#else
                // always clear the checksum
                mb1_checksum_t *pchk = MB1_PCHECKSUM(self);
#endif
                memset((byte *)pchk, 0, MB1_CHECKSUM_BYTES);
            }

            retval = 0;
        }
    }
    return retval;
}
// End function mb1_zero

int mb1_zero_len(mb1_t *self, size_t len)
{
    int retval = -1;

    if(NULL != self) {
        memset((byte *)self, 0, len);
        retval = 0;
    }
    return retval;
}
// End function mb1_zero_len

void mb1_show(mb1_t *self, bool verbose, uint16_t indent)
{
    int wkey = 15;
    int wval = 15;

    if(self) {
#if defined(__QNX__)
        uint32_t *pchk = MB1_PCHECKSUM(self);
        int nbeams = self->nbeams;
        mb1_beam_t *pbeams = NULL;
#endif
        fprintf(stderr, "%*s%*s %*p\n", indent, (indent>0?" ":""), wkey, "self", wval, self);
        fprintf(stderr, "%*s%*s %*s%08X\n", indent, (indent>0?" ":""), wkey, "type", wval-8, " ", self->type);
        fprintf(stderr, "%*s%*s %*u\n", indent, (indent>0?" ":""), wkey, "size", wval, self->size);
        fprintf(stderr, "%*s%*s %*.3lf\n", indent, (indent>0?" ":""), wkey, "ts", wval, self->ts);
        fprintf(stderr, "%*s%*s %*.3lf\n", indent, (indent>0?" ":""), wkey, "lat", wval, self->lat);
        fprintf(stderr, "%*s%*s %*.3lf\n", indent, (indent>0?" ":""), wkey, "lon", wval, self->lon);
        fprintf(stderr, "%*s%*s %*.3lf\n", indent, (indent>0?" ":""), wkey, "depth", wval, self->depth);
        fprintf(stderr, "%*s%*s %*.3lf\n", indent, (indent>0?" ":""), wkey, "hdg", wval, self->hdg);
        fprintf(stderr, "%*s%*s %*d\n", indent, (indent>0?" ":""), wkey, "ping_number", wval, self->ping_number);
        fprintf(stderr, "%*s%*s %*u\n", indent, (indent>0?" ":""), wkey, "nbeams", wval, self->nbeams);
#if !defined(__QNX__)
        uint32_t *pchk = MB1_PCHECKSUM(self);
#endif
        fprintf(stderr, "%*s%*s %*p\n", indent, (indent>0?" ":""), wkey, "&checksum", wval, pchk);
        fprintf(stderr, "%*s%*s %*s%08X\n", indent, (indent>0?" ":""), wkey, "checksum", wval-8, " ", *pchk);

#if !defined(__QNX__)
        int nbeams = self->nbeams;
#endif

        if(verbose && nbeams > 0) {
            fprintf(stderr, "%*s[ n ] beam     rhox      rhoy       rhoz   \n", indent+3, (indent>0?" ":""));
#if !defined(__QNX__)
            mb1_beam_t *pbeams = MB1_PBEAMS(self);
#endif
            pbeams = MB1_PBEAMS(self);
            {int i=0; for(i=0;i<self->nbeams;i++) {
                fprintf(stderr, "%*s[%3d] %03u  %+10.3lf %+10.3lf %+10.3lf\n", indent+3, (indent>0?" ":""), i,
                        pbeams[i].beam_num,
                        pbeams[i].rhox,
                        pbeams[i].rhoy,
                        pbeams[i].rhoz);
            }
            }}
    } else {
        fprintf(stderr, "%*s%*s %*p\n", indent, (indent>0?" ":""), wkey, "self", wval, self);
    }
}
// End function mb1_show

uint32_t mb1_calc_checksum(mb1_t *self)
{
    uint32_t retval = 0;

    if(NULL != self) {

        retval = mb1_checksum_u32((byte *)self, MB1_CHECKSUM_LEN_BYTES(self));
    }
    return retval;
}
// End function mb1_calc_checksum


uint32_t mb1_set_checksum(mb1_t *self)
{
    uint32_t retval = 0;

    if(NULL != self) {
        // compute checksum over
        // sounding and beam data, excluding checksum bytes
        uint32_t *pchk = MB1_PCHECKSUM(self);
        *pchk = mb1_calc_checksum(self);
//        fprintf(stderr,"%s - snd[%p] pchk[%p/%08X] ofs[%ld]\n",__func__,self,pchk,*pchk,(long)((byte *)pchk-bp));
        retval = *pchk;
    }
    return retval;
}
// End function mb1_set_checksum

int mb1_validate_checksum(mb1_t *self)
{
    int retval = -1;

    if(NULL != self) {
        // compute checksum over
        // sounding and beam data, excluding checksum bytes
        uint32_t cs_val = MB1_GET_CHECKSUM(self);
        uint32_t cs_calc = mb1_calc_checksum(self);
        retval = ( cs_val == cs_calc ? 0 : -1) ;
    }
    return retval;
}
// End function mb1_validate_checksum

byte *mb1_serialize(mb1_t *self, size_t *r_size)
{
    byte *retval = NULL;

    if( (NULL != self) &&
        (self->size == MB1_SOUNDING_BYTES(self->nbeams))) {

        retval = (byte *)malloc(self->size);

        if(retval) {
            memset(retval, 0, self->size);
            memcpy(retval, self, self->size);

            if(NULL != r_size) {
                // return size, if provided
                *r_size = self->size;
            }
        }
    } else {
        fprintf(stderr,"invalid argument\n");
    }

    return retval;
}
// End function mb1_serialize

uint32_t mb1_checksum_u32(byte *pdata, uint32_t len)
{
    uint32_t checksum = 0;

    if(NULL != pdata) {
        byte *bp = pdata;
        // for ANSI C/QNX compatibility
        uint32_t i = 0;
        for(i = 0; i < len; i++) {
            checksum += (byte)(*(bp + i));
        }
    }
    return checksum;
}
// End function mb1_checksum

void mb1_hex_show(byte *data, uint32_t len, uint16_t cols, bool show_offsets, uint16_t indent)
{
    if(NULL != data && len > 0 && cols > 0) {
        int rows = len / cols;
        int rem = len % cols;

        byte *p = data;
        // for ANSI C/QNX compatibility
        int i = 0;
        for(i = 0; i < rows; i++) {
            // for ANSI C/QNX compatibility
            int j = 0;
            if(show_offsets) {
                fprintf(stderr, "%*s%04ld [", indent, (indent > 0 ? " " : ""), (long int)(p - data));
            } else {
                fprintf(stderr, "%*s[", indent, (indent > 0 ? " " : ""));
            }

            for(j = 0; j < cols; j++) {
                if(p >= data && p < (data + len)) {
                    byte b = (*p);
                    fprintf(stderr, " %02x", b);
                    p++;
                } else {
                    fprintf(stderr, "   ");
                }
            }
            fprintf(stderr, " ]\n");
        }
        if(rem > 0) {
            // for ANSI C/QNX compatibility
            int j = 0;
            if(show_offsets) {
                fprintf(stderr, "%*s%04ld [", indent, (indent > 0 ? " " : ""), (long int)(p - data));
            } else {
                fprintf(stderr, "%*s[", indent, (indent > 0 ? " " : ""));
            }
            
            for(j = 0; j < rem; j++) {
                fprintf(stderr, " %02x", *p++);
            }
            fprintf(stderr, "%*s ]\n", 3*(cols-rem), " ");
        }
    }
}
// End function mb1_hex_show

int mb1_test(int verbose)
{
    int retval = 0;

    uint32_t beams = 0;
    mb1_t *m = mb1_new(beams);
#if defined(__QNX__)
    size_t sz = 0;
    byte *b = NULL;
#endif

    if(m == NULL || m->size != MB1_SOUNDING_BYTES(beams))
        retval = -1;

    if(verbose) {
        fprintf(stderr, "%u beams size[%"PRIu32"]\n", beams, m->size);
        mb1_show(m, true, 5);
        mb1_hex_show((byte *)m, m->size, 16, 1, 5);
        fprintf(stderr, "\n");
    }
#if defined(__QNX__)
    b = mb1_serialize(m, &sz);
#else
    size_t sz = 0;
    byte *b = mb1_serialize(m, &sz);
#endif

    if(verbose) {
        fprintf(stderr, "%u beams serialized len[%"PRIu32"]\n", beams, (uint32_t)sz);
        mb1_hex_show((byte *)m, m->size, 16, 1, 5);
        fprintf(stderr, "\n");
    }
    free(b);
    b = NULL;

    if(sz != MB1_SOUNDING_BYTES(beams))
        retval = -1;

    beams = 10;
    mb1_resize(&m, beams, MB1_RS_BEAMS);

    if(verbose)
        fprintf(stderr, "%u beams size[%"PRIu32"]\n", beams, m->size);

    mb1_set_checksum(m);

    if(verbose) {
        mb1_show(m, true, 5);
        mb1_hex_show((byte *)m, m->size, 16, 1, 5);
        fprintf(stderr, "\n");
    }

    if(m == NULL || m->size != MB1_SOUNDING_BYTES(beams))
        retval = -1;

    sz = 0;
    b = mb1_serialize(m, &sz);
    if(verbose) {
        fprintf(stderr, "%u beams serialized len[%"PRIu32"]\n", beams, (uint32_t)sz);
        mb1_hex_show((byte *)m, m->size, 16, 1, 5);
        fprintf(stderr, "\n");
    }
    free(b);
    b = NULL;

    if(sz != MB1_SOUNDING_BYTES(beams))
        retval = -1;

    mb1_destroy(&m);

    if(verbose)
        fprintf(stderr, "returning %d/%s\n\n", retval, (retval==0 ? "OK" : "ERR"));

    return retval;
}
// End function mb1_test
