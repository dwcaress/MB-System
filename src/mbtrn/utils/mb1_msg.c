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
#include <memory.h>
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

mb1_frame_t *mb1_frame_new(int beams)
{
    mb1_frame_t *instance =NULL;
    if(beams>=0){
        uint32_t frame_size = MB1_FRAME_BYTES(beams);
        uint32_t alloc_size = frame_size+sizeof(mb1_frame_t);
        instance = (mb1_frame_t *)malloc(alloc_size);
        if(NULL!=instance){
            memset(instance,0,alloc_size);
            instance->sounding = (mb1_sounding_t *)((unsigned char *)instance+sizeof(mb1_frame_t));
            instance->checksum = MB1_PCHECKSUM_U32(instance);
            instance->sounding->type = MB1_TYPE_ID;
            instance->sounding->size=frame_size;
            instance->sounding->nbeams=beams;
        }
    }
    return instance;
}

void mb1_frame_destroy(mb1_frame_t **pself)
{
    if(NULL!=pself){
        mb1_frame_t *self = *(pself);
        free(self);
        *pself=NULL;
    }
}

mb1_frame_t *mb1_frame_resize(mb1_frame_t **pself, int beams, int flags)
{
    mb1_frame_t *instance =NULL;
    
    if(NULL!=pself && beams>=0){
        uint32_t frame_size = MB1_FRAME_BYTES(beams);
        uint32_t alloc_size = frame_size+sizeof(mb1_frame_t);
        if(NULL==*pself){
            instance = mb1_frame_new(beams);
        }else{
            instance = (mb1_frame_t *)realloc((*pself),alloc_size);
        }
        if(NULL!=instance){
            instance->sounding = (mb1_sounding_t *)((unsigned char *)instance+sizeof(mb1_frame_t));
            instance->checksum = MB1_PCHECKSUM_U32(instance);
            instance->sounding->type = MB1_TYPE_ID;
            instance->sounding->size=frame_size;
            instance->sounding->nbeams=beams;
            mb1_frame_zero(instance,flags);
            *pself = instance;
        }
    }
    return instance;
}

int mb1_frame_zero(mb1_frame_t *self, int flags)
{
    int retval=-1;
    if(NULL!=self){
        uint32_t beams=self->sounding->nbeams;
        if(beams>0 && beams<=MB1_MAX_BEAMS){
            uint32_t frame_size = MB1_FRAME_BYTES(beams);
            if(flags==MB1_RS_ALL){
                memset(self->sounding,0,frame_size);
            }else{
                if( (flags&MB1_RS_BEAMS)){
                    memset(&self->sounding->beams[0],0,MB1_BEAM_ARRAY_BYTES(beams));
                }
                if( (flags&MB1_RS_HEADER)){
                    memset(self->sounding,0,MB1_HEADER_BYTES);
                }
                // always clear the checksum
                memset(self->checksum,0,MB1_CHECKSUM_BYTES);
            }
            retval=0;
        }
    }
    return retval;
}

unsigned int mb1_frame_calc_checksum(mb1_frame_t *self)
{
    unsigned int retval = 0xFFFFFFFF;
    if(NULL!=self){
        unsigned char *cp = (unsigned char *)self->sounding;
        int chk_len =self->sounding->size-MB1_CHECKSUM_BYTES;
        if(chk_len>0 && chk_len<=MB1_MAX_FRAME_BYTES){
            unsigned int checksum=0x00000000;
            int i=0;
            for (i = 0; i < chk_len; i++) {
                checksum += (unsigned int) (*cp++);
            }
            retval=checksum;
        }
    }
    return retval;
}

void mb1_frame_show(mb1_frame_t *self, bool verbose, uint16_t indent)
{
    if (NULL != self) {
        int wkey=15;
        int wval=15;
        fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"self",wval,self);
        fprintf(stderr,"%*s%*s %*s%08X\n",indent,(indent>0?" ":""),wkey,"type",wval-8," ",self->sounding->type);
        fprintf(stderr,"%*s%*s %*u\n",indent,(indent>0?" ":""),wkey,"size",wval,self->sounding->size);
        fprintf(stderr,"%*s%*s %*.3lf\n",indent,(indent>0?" ":""),wkey,"ts",wval,self->sounding->ts);
        fprintf(stderr,"%*s%*s %*.3lf\n",indent,(indent>0?" ":""),wkey,"lat",wval,self->sounding->lat);
        fprintf(stderr,"%*s%*s %*.3lf\n",indent,(indent>0?" ":""),wkey,"lon",wval,self->sounding->lon);
        fprintf(stderr,"%*s%*s %*.3lf\n",indent,(indent>0?" ":""),wkey,"depth",wval,self->sounding->depth);
        fprintf(stderr,"%*s%*s %*.3lf\n",indent,(indent>0?" ":""),wkey,"hdg",wval,self->sounding->hdg);
        fprintf(stderr,"%*s%*s %*d\n",indent,(indent>0?" ":""),wkey,"ping_number",wval,self->sounding->ping_number);
        fprintf(stderr,"%*s%*s %*u\n",indent,(indent>0?" ":""),wkey,"nbeams",wval,self->sounding->nbeams);
        fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"&checksum",wval,self->checksum);
        fprintf(stderr,"%*s%*s %*s%08X\n",indent,(indent>0?" ":""),wkey,"checksum",wval-8," ",*self->checksum);

        int nbeams = self->sounding->nbeams;

        if(nbeams>0){
        fprintf(stderr,"%*s[ n ] beam     rhox      rhoy       rhoz   \n",indent+3,(indent>0?" ":""));
        for(int i=0;i<nbeams;i++){
            fprintf(stderr,"%*s[%3d] %03u  %+10.3lf %+10.3lf %+10.3lf\n",indent+3,(indent>0?" ":""), i,
                    self->sounding->beams[i].beam_num,
                    self->sounding->beams[i].rhox,
                    self->sounding->beams[i].rhoy,
                    self->sounding->beams[i].rhoz);
        }
        }
    }
}

