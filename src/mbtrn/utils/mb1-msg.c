///
/// @file mb1-msg.c
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
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include "mb1-msg.h"

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

mb1_sounding_t *mb1_sounding_new(uint32_t beams)
{
    mb1_sounding_t *instance =NULL;
    if(beams>=0){
        uint32_t alloc_size = MB1_SOUNDING_BYTES(beams);
        instance = (mb1_sounding_t *)malloc(alloc_size);
        if(NULL!=instance){
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
            mb1_sounding_set_checksum(instance);
        }
    }
    return instance;
}
// End function mb1_sounding_new

void mb1_sounding_destroy(mb1_sounding_t **pself)
{
    if (pself) {
        mb1_sounding_t *self = *pself;
        if (self) {
            free(self);
        }
        *pself=NULL;
    }
}
// End function mb1_sounding_destroy

mb1_sounding_t *mb1_sounding_resize(mb1_sounding_t **pself, uint32_t beams,  int flags)
{
    mb1_sounding_t *instance =NULL;

    if(NULL!=pself && beams>=0){
        uint32_t alloc_size = MB1_SOUNDING_BYTES(beams);
        if(NULL==*pself){
            instance = mb1_sounding_new(beams);
        }else{
            instance = (mb1_sounding_t *)realloc((*pself),alloc_size);
        }
        if(NULL!=instance){
            instance->type = MB1_TYPE_ID;
            instance->size=alloc_size;
            instance->nbeams=beams;
            // always clears checksum (caller must set)
            mb1_sounding_zero(instance,flags);
            *pself = instance;
        }
    }
    return instance;
}
// End function mb1_sounding_new

int mb1_sounding_zero(mb1_sounding_t *self, int flags)
{
    int retval=-1;
    if(NULL!=self){
        uint32_t beams=self->nbeams;
        if(beams>0 && beams<=MB1_MAX_BEAMS){
            uint32_t zlen = MB1_SOUNDING_BYTES(beams);
            if(flags==MB1_RS_ALL){
                memset((byte *)self,0,zlen);
            }else{
                if( (flags&MB1_RS_BEAMS)){
                    mb1_beam_t *pbeams = MB1_PBEAMS(self);
                    memset((byte *)pbeams,0,MB1_BEAM_ARRAY_BYTES(beams));
                }
                if( (flags&MB1_RS_HEADER)){
                    memset((byte *)self,0,MB1_HEADER_BYTES);
                }
                // always clear the checksum
                mb1_checksum_t *pchk = MB1_PCHECKSUM(self);
                memset((byte *)pchk,0,MB1_CHECKSUM_BYTES);
            }
            retval=0;
        }
    }
    return retval;
}
// End function mb1_sounding_zero


void mb1_sounding_show(mb1_sounding_t *self, bool verbose, uint16_t indent)
{
    int wkey=15;
    int wval=15;
    if (self) {
        fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"self",wval,self);
        fprintf(stderr,"%*s%*s %*s%08X\n",indent,(indent>0?" ":""),wkey,"type",wval-8," ",self->type);
        fprintf(stderr,"%*s%*s %*u\n",indent,(indent>0?" ":""),wkey,"size",wval,self->size);
        fprintf(stderr,"%*s%*s %*.3lf\n",indent,(indent>0?" ":""),wkey,"ts",wval,self->ts);
        fprintf(stderr,"%*s%*s %*.3lf\n",indent,(indent>0?" ":""),wkey,"lat",wval,self->lat);
        fprintf(stderr,"%*s%*s %*.3lf\n",indent,(indent>0?" ":""),wkey,"lon",wval,self->lon);
        fprintf(stderr,"%*s%*s %*.3lf\n",indent,(indent>0?" ":""),wkey,"depth",wval,self->depth);
        fprintf(stderr,"%*s%*s %*.3lf\n",indent,(indent>0?" ":""),wkey,"hdg",wval,self->hdg);
        fprintf(stderr,"%*s%*s %*d\n",indent,(indent>0?" ":""),wkey,"ping_number",wval,self->ping_number);
        fprintf(stderr,"%*s%*s %*u\n",indent,(indent>0?" ":""),wkey,"nbeams",wval,self->nbeams);
        uint32_t *pchk = MB1_PCHECKSUM(self);
        fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"&checksum",wval,pchk);
        fprintf(stderr,"%*s%*s %*s%08X\n",indent,(indent>0?" ":""),wkey,"checksum",wval-8," ",*pchk);

        int nbeams = self->nbeams;

        if(verbose && nbeams>0){
            fprintf(stderr,"%*s[ n ] beam     rhox      rhoy       rhoz   \n",indent+3,(indent>0?" ":""));
            mb1_beam_t *pbeams = MB1_PBEAMS(self);
            for(int i=0;i<self->nbeams;i++){
                fprintf(stderr,"%*s[%3d] %03u  %+10.3lf %+10.3lf %+10.3lf\n",indent+3,(indent>0?" ":""), i,
                        pbeams[i].beam_num,
                        pbeams[i].rhox,
                        pbeams[i].rhoy,
                        pbeams[i].rhoz);
            }
        }
    }else{
        fprintf(stderr,"%*s[self %10p (NULL message)]\n",indent,(indent>0?" ":""), self);

    }
}
// End function mb1_sounding_show

uint32_t mb1_calc_checksum(mb1_sounding_t *self)
{
    uint32_t retval=0;
    if(NULL!=self){

        retval=mb1_checksum_u32((byte *)self, MB1_CHECKSUM_LEN_BYTES(self));
    }
    return retval;
}
// End function mb1_calc_checksum


uint32_t mb1_sounding_set_checksum(mb1_sounding_t *self)
{
    uint32_t retval=0;
    if (NULL!=self) {
        // compute checksum over
        // sounding and beam data, excluding checksum bytes
        uint32_t *pchk = MB1_PCHECKSUM(self);
        *pchk = mb1_calc_checksum(self);
        //        fprintf(stderr,"%s - snd[%p] pchk[%p/%08X] ofs[%ld]\n",__func__,self,pchk,*pchk,(long)((byte *)pchk-bp));
        retval = *pchk;
    }
    return retval;
}
// End function mb1_sounding_set_checksum

int mb1_sounding_validate_checksum(mb1_sounding_t *self)
{
    int retval=-1;
    if (NULL!=self) {
        // compute checksum over
        // sounding and beam data, excluding checksum bytes
        uint32_t cs_val = MB1_GET_CHECKSUM(self);
        uint32_t cs_calc = mb1_calc_checksum(self);
        retval = ( cs_val==cs_calc ? 0 : -1) ;
    }
    return retval;
}
// End function mb1_sounding_validate_checksum

byte *mb1_sounding_serialize(mb1_sounding_t *self, size_t *r_size)
{
    byte *retval = NULL;
    if ( (NULL!=self)
        &&
        (self->nbeams>0)
        &&
        (self->size == MB1_SOUNDING_BYTES(self->nbeams))) {

        //        fprintf(stderr,"mb1_sounding_serialize - bufsz[%"PRIu32"] msg->data_size[%"PRIu32"]\n",bufsz,self->data_size);
        retval = (byte *)malloc( self->size );

        if (retval) {
            memset(retval,0,self->size);
            memcpy(retval,self, self->size);
            if(NULL!=r_size){
                // return size, if provided
                *r_size = self->size;
            }
        }
    }else{
        fprintf(stderr,"invalid argument\n");
    }

    return retval;
}
// End function mb1_sounding_serialize

uint32_t mb1_checksum_u32(byte *pdata, uint32_t len)
{
    uint32_t checksum=0;
    if (NULL!=pdata) {
        byte *bp = pdata;
        for (uint32_t i=0; i<len; i++) {
            checksum += (byte)(*(bp+i));
        }
    }
    return checksum;
}
// End function mb1_checksum

void mb1_hex_show(byte *data, uint32_t len, uint16_t cols, bool show_offsets, uint16_t indent)
{
    if (NULL!=data && len>0 && cols>0) {
        int rows = len/cols;
        int rem = len%cols;

        byte *p=data;
        for (int i=0; i<rows; i++) {
            if (show_offsets) {
                fprintf(stderr,"%*s%04ld [",indent,(indent>0?" ":""),(long int)(p-data));
            }else{
                fprintf(stderr,"%*s[",indent,(indent>0?" ":""));
            }
            for (int j=0; j<cols; j++) {
                if (p>=data && p<(data+len)) {
                    byte b = (*p);
                    fprintf(stderr," %02x",b);
                    p++;
                }else{
                    fprintf(stderr,"   ");
                }
            }
            fprintf(stderr," ]\n");
        }
        if (rem>0) {
            if (show_offsets) {
                fprintf(stderr,"%*s%04ld [",indent,(indent>0?" ":""),(long int)(p-data));
            }else{
                fprintf(stderr,"%*s[",indent,(indent>0?" ":""));
            }
            for (int j=0; j<rem; j++) {
                fprintf(stderr," %02x",*p++);
            }
            fprintf(stderr,"%*s ]\n",3*(cols-rem)," ");
        }
    }
}
// End function mb1_hex_show

int mb1_test()
{
    int retval=-1;
    fprintf(stderr,"%s not implemented\n",__func__);
    return retval;
}
// End function mb1_test
