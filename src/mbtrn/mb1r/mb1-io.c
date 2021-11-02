///
/// @file mb1-io.c
/// @authors k. Headley
/// @date 01 jan 2018
 
/// MB1 data structures and protocol API

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
/////////////////////////
// Headers 
/////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include "mb1-io.h"
#include "mb1_msg.h"

#ifdef WITH_MB1_UTILS
#include "msocket.h"
#include "merror.h"
#include "mmdebug.h"
#include "medebug.h"
#include "mconfig.h"
#endif //WITH_MB1_UTILS

/////////////////////////
// Macros
/////////////////////////

// These macros should only be defined for 
// application main files rather than general C files
/*
/// @def PRODUCT
/// @brief header software product name
#define PRODUCT "MBRT"

/// @def COPYRIGHT
/// @brief header software copyright info
#define COPYRIGHT "Copyright 2002-2013 MBARI Monterey Bay Aquarium Research Institute, all rights reserved."
/// @def NOWARRANTY
/// @brief header software terms of use
#define NOWARRANTY  \
"This program is distributed in the hope that it will be useful,\n"\
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"\
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"\
"GNU General Public License for more details (http://www.gnu.org/licenses/gpl-3.0.html)\n"
*/

// string buffer expansion increment
#define MB1_STR_INC 256
#define TRACKING_BYTES 16
/////////////////////////
// Declarations 
/////////////////////////

// define module IDs in mconfig.h

/// @enum mb1_module_ids
/// @brief application module IDs
/// [note : starting above reserved mframe module IDs]
//typedef enum{
//    MOD_MB1=MM_MODULE_COUNT,
//    APP_MODULE_COUNT
//}mb1_module_ids;

///// @enum mb1_channel_id
///// @brief test module channel IDs
///// [note : starting above reserved mframe channel IDs]
//typedef enum{
//    ID_MB1_V1=MM_CHANNEL_COUNT,
//    ID_MB1_V2,
//    ID_MB1_PARSER,
//    ID_MB1_DRFCON,
//    MB1_CH_COUNT
//}mb1_channel_id;
//
///// @enum mb1_channel_mask
///// @brief test module channel masks
//typedef enum{
//    MB1_V1= (1<<ID_MB1_V1),
//    MB1_V2= (1<<ID_MB1_V2),
//    MB1_PARSER= (1<<ID_MB1_PARSER),
//    MB1_DRFCON= (1<<ID_MB1_DRFCON)
//}mb1_channel_mask;
//
///// @var char *mmd_test_m1_ch_names[MB1_CH_COUNT]
///// @brief test module channel names
//char *mb1_ch_names[MB1_CH_COUNT]={
//    "trace.mb1",
//    "debug.mb1",
//    "warn.mb1",
//    "err.mb1",
//    "mb1.1",
//    "mb1.verbose",
//    "mb1.parser"
//};
//
//static mmd_module_config_t mmd_config_default= {MOD_MB1,"MOD_MB1",MB1_CH_COUNT,((MM_ERR|MM_WARN)|MB1_V1),mb1_ch_names};

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////

/////////////////////////
// Function Definitions
/////////////////////////

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

int mb1_test()
{
    int retval=-1;
    fprintf(stderr,"%s not implemented\n",__func__);
    return retval;
}
// End function mb1_test

#ifdef WITH_MB1_PARSE_STAT
    void mb1_parser_show(mb1_parse_stat_t *self, bool verbose, uint16_t indent)
    {
        if (NULL!=self) {
            fprintf(stderr,"%*s[self           %10p]\n",indent,(indent>0?" ":""), self);
            fprintf(stderr,"%*s[src_bytes      %10u]\n",indent,(indent>0?" ":""), self->src_bytes);
            fprintf(stderr,"%*s[sync_bytes     %10u]\n",indent,(indent>0?" ":""), self->sync_bytes);
            fprintf(stderr,"%*s[unread_bytes   %10u]\n",indent,(indent>0?" ":""), self->unread_bytes);
            fprintf(stderr,"%*s[parsed_records %10u]\n",indent,(indent>0?" ":""), self->parsed_records);
            fprintf(stderr,"%*s[parsed_bytes   %10u]\n",indent,(indent>0?" ":""), self->parsed_bytes);
            fprintf(stderr,"%*s[resync_count   %10u]\n",indent,(indent>0?" ":""), self->resync_count);
            fprintf(stderr,"%*s[status         %10d]\n",indent,(indent>0?" ":""), self->status);
        }
    }
    // End function mb1_parser_show

    char *mb1_parser_str(mb1_parse_stat_t *self, char *dest, uint32_t len, bool verbose, uint16_t indent)
    {
        char *retval=NULL;

        //    uint32_t inc=256;

        if (NULL!=self) {

            char *wbuf=(char *)malloc(MB1_STR_INC*sizeof(char));

            // TODO: check length/realloc
            if (wbuf!=NULL) {
                memset(wbuf,0,len);
                char *dp=wbuf;

                uint32_t n=1;
                char *fmt="%*s[self           %10p]\n";
                uint32_t wlen=len;
                uint32_t wb = snprintf(NULL,0,fmt,indent,(indent>0?" ":""), self);
                wlen+=wb;
                if (wlen>MB1_STR_INC) {
                    n++;
                    wbuf=(char *)realloc(wbuf,n*MB1_STR_INC*sizeof(char));
                    dp = wbuf+strlen(wbuf);
                }
                sprintf(dp,fmt,indent,(indent>0?" ":""), self);
                dp = wbuf+strlen(wbuf);

                fmt="%*s[src_bytes      %10u]\n";
                wb = snprintf(NULL,0,fmt,indent,(indent>0?" ":""), self->src_bytes);
                wlen+=wb;
                if (wlen>MB1_STR_INC) {
                    n++;
                    wbuf=(char *)realloc(wbuf,n*MB1_STR_INC*sizeof(char));
                    dp = wbuf+strlen(wbuf);
                }
                sprintf(dp,fmt,indent,(indent>0?" ":""), self->src_bytes);
                dp = wbuf+strlen(wbuf);

                fmt="%*s[sync_bytes     %10u]\n";
                wb = snprintf(NULL,0,fmt,indent,(indent>0?" ":""), self->sync_bytes);
                wlen+=wb;
                if (wlen>MB1_STR_INC) {
                    n++;
                    wbuf=(char *)realloc(wbuf,n*MB1_STR_INC*sizeof(char));
                    dp = wbuf+strlen(wbuf);
                }
                sprintf(dp,fmt,indent,(indent>0?" ":""), self->sync_bytes);
                dp = wbuf+strlen(wbuf);

                fmt="%*s[unread_bytes   %10u]\n";
                wb = snprintf(NULL,0,fmt,indent,(indent>0?" ":""), self->unread_bytes);
                wlen+=wb;
                if (wlen>MB1_STR_INC) {
                    n++;
                    wbuf=(char *)realloc(wbuf,n*MB1_STR_INC*sizeof(char));
                    dp = wbuf+strlen(wbuf);
                }
                sprintf(dp,fmt,indent,(indent>0?" ":""), self->unread_bytes);
                dp = wbuf+strlen(wbuf);


                fmt="%*s[parsed_records %10u]\n";
                wb = snprintf(NULL,0,fmt,indent,(indent>0?" ":""), self->parsed_records);
                wlen+=wb;
                if (wlen>MB1_STR_INC) {
                    n++;
                    wbuf=(char *)realloc(wbuf,n*MB1_STR_INC*sizeof(char));
                    dp = wbuf+strlen(wbuf);
                }
                sprintf(dp,fmt,indent,(indent>0?" ":""), self->parsed_records);
                dp = wbuf+strlen(wbuf);

                fmt="%*s[parsed_bytes   %10u]\n";
                wb = snprintf(NULL,0,fmt,indent,(indent>0?" ":""), self->parsed_bytes);
                wlen+=wb;
                if (wlen>MB1_STR_INC) {
                    n++;
                    wbuf=(char *)realloc(wbuf,n*MB1_STR_INC*sizeof(char));
                    dp = wbuf+strlen(wbuf);
                }
                sprintf(dp,fmt,indent,(indent>0?" ":""), self->parsed_bytes);
                dp = wbuf+strlen(wbuf);

                fmt="%*s[resync_count   %10u]\n";
                wb = snprintf(NULL,0,fmt,indent,(indent>0?" ":""), self->resync_count);
                wlen+=wb;
                if (wlen>MB1_STR_INC) {
                    n++;
                    wbuf=(char *)realloc(wbuf,n*MB1_STR_INC*sizeof(char));
                    dp = wbuf+strlen(wbuf);
                }
                sprintf(dp,fmt,indent,(indent>0?" ":""), self->resync_count);
                dp = wbuf+strlen(wbuf);

                fmt="%*s[status         %10d]\n";
                wb = snprintf(NULL,0,fmt,indent,(indent>0?" ":""), self->status);
                wlen+=wb;
                if (wlen>MB1_STR_INC) {
                    n++;
                    wbuf=(char *)realloc(wbuf,n*MB1_STR_INC*sizeof(char));
                    dp = wbuf+strlen(wbuf);
                }
                sprintf(dp,fmt,indent,(indent>0?" ":""), self->status);

                if (NULL!=dest) {
                    snprintf(dest,(len<wlen?len:wlen),"%s",wbuf);
                    free(wbuf);
                    retval=dest;
                }else{
                    retval=wbuf;
                }
            }
        }
        return retval;
    }
    // End function mb1_parser_str
#endif // WITH_MB1_PARSE_STAT

#ifdef WITH_MB1_UTILS
int mb1_stream_show(msock_socket_t *s, int sz, uint32_t tmout_ms, int cycles, bool *interrupt)
{
    int retval=-1;
    int x=(sz<=0?16:sz);
    byte *buf=(byte *)malloc(x);

    if(NULL!=buf){
        int good=0,err=0,zero=0,tmout=0;
        int status = 0;
        bool forever=true;
        int count=0;

        if (cycles>0) {
            forever=false;
        }
        //    PEPRINT((stderr,"cycles[%d] forever[%s] c||f[%s]\n",cycles,(forever?"Y":"N"),(forever || (cycles>0) ? "Y" :"N")));

        // read cycles or forever (cycles<=0)
        while ( (forever || (count++ < cycles)) &&
               (NULL!=interrupt && !(*interrupt)) ) {
            memset(buf,0,x);
            int64_t test = msock_read_tmout(s, buf, x, tmout_ms);
            if(test>0){
                good++;
                mb1_hex_show(buf, test, 16, true, 3);
                fprintf(stderr,"c[%d/%d] ret[%"PRId64"/%u] stat[%d] good/zero/tmout/err [%d/%d/%d/%d]\n",count,cycles,test,sz,status,good,zero,tmout,err);
                retval=0;
            }else if(test<0){
                PMPRINT(MOD_MB1,MM_DEBUG,(stderr,"ERR [%d/%s]\n",me_errno,me_strerror(me_errno)));
                err++;
                tmout = (me_errno==ME_ETMOUT ? tmout+1 : tmout );
                if (me_errno==ME_ETMOUT || me_errno==ME_EOF || me_errno==ME_ESOCK) {
                    break;
                }
            }else{
                PMPRINT(MOD_MB1,MM_DEBUG,(stderr,"read returned 0\n"));
                zero++;
                if (me_errno==ME_ESOCK || me_errno==ME_EOF) {
                    break;
                }
            }
        }
        free(buf);
    }
    return retval;
}
// End function mb1_stream_show


int mb1_sounding_receive(msock_socket_t *s, mb1_sounding_t **dest, uint32_t timeout_msec)
{
    int retval=-1;
    if (NULL != s && s->status==SS_CONNECTED) {
        
       int64_t nbytes=0;
        // read nf, drf headers
        int64_t header_len = MB1_HEADER_BYTES;

        // Invalid C.    JL
        // Valid since C99; faster on stack, OK (better) since small, no free required. klh
        byte headers[header_len];
        memset(headers,0,header_len);

        if ( (nbytes=msock_read_tmout(s,headers,header_len,timeout_msec)) == header_len) {
            int64_t total_len=nbytes;
            PMPRINT(MOD_MB1,MM_DEBUG,(stderr,"read headers [%"PRId64"/%"PRId64"]\n",nbytes,header_len));
            // get data and checksum
            mb1_header_t *hdr = (mb1_header_t *)(headers);

            // get size of data (beams, checksum)
            uint32_t data_len = MB1_BEAM_ARRAY_BYTES(hdr->nbeams);
            uint32_t read_len = data_len+MB1_CHECKSUM_BYTES;

            PMPRINT(MOD_MB1,MM_DEBUG,(stderr,"data_len[%u] read_len[%u]\n",data_len,read_len));
            // read rth/rd/od [if any], checksum
            if (read_len>0) {
                // INVALID C. JL
                // valid C99
                byte data[read_len];
                memset(data,0,read_len);
                if ( (nbytes=msock_read_tmout(s,data,read_len,timeout_msec)) == read_len) {
                    total_len+=nbytes;
                   PMPRINT(MOD_MB1,MM_DEBUG,(stderr,"read data [%"PRId64"/%d] -> %p\n",nbytes,read_len,dest));
                    // TODO: validate content
                    // create message
                    mb1_sounding_t *m = mb1_sounding_new(hdr->nbeams);
                    if (NULL!=m) {
                        memcpy(m,hdr,header_len);
                        byte *pdata = (byte *)m + MB1_HEADER_BYTES;
                        memcpy(pdata,data,nbytes);
//                       PMPRINT(MOD_MB1,MM_DEBUG,(stderr,"dest msg:\n"));
//                        mb1_sounding_show(m,true,6);
                        *dest=m;
                        retval=total_len;
                    }else{
                        PEPRINT((stderr,"recv - msg_new failed\n"));
                    }
                }else{
                   PMPRINT(MOD_MB1,MM_DEBUG,(stderr,"recv - incomplete data read nbytes[%"PRId64"] data_len[%u]\n",nbytes,data_len));
                }
            }else{
               PMPRINT(MOD_MB1,MM_DEBUG,(stderr,"recv - read_len <= 0 nbytes[%"PRId64"] read_len[%u]\n",nbytes,read_len));
            }
        }else{
           PMPRINT(MOD_MB1,MM_DEBUG,(stderr,"recv - incomplete header read? nbytes[%"PRId64"] header_len[%"PRId64"]\n",nbytes,header_len));
        }
    }else{
        PMPRINT(MOD_MB1,MM_DEBUG,(stderr,"recv - invalid socket or status s[%p, %d/%d]\n",s, (s!=NULL?s->status:0),SS_CONNECTED));
    }
        
    return retval;
}
// End function mb1_sounding_receive

int mb1_sounding_send(msock_socket_t *s, mb1_sounding_t *self)
{
    int retval=-1;
    if ( (NULL != self) && (NULL!=s)) {
        size_t r_size = 0;
        byte *buf = mb1_sounding_serialize(self, &r_size);

        if( NULL!=buf && r_size>0){
            //       PMPRINT(MOD_MB1,MM_DEBUG,(stderr,"SEND nf:\n"));
            //        mb1_nf_show((mb1_nf_t *)buf,true,4);
            int64_t status=0;

            if( (status=msock_send(s,buf,r_size))>0){
                retval=0;
                PMPRINT(MOD_MB1,MM_DEBUG,(stderr,"send OK s[%p, %d]\n",s, s->status));
            }else{
                PEPRINT((stderr,"send failed [%"PRId64"] [%d/%s]\n",status,errno,strerror(errno)));
            }
        }

        free(buf);
    }else{
        PMPRINT(MOD_MB1,MM_DEBUG,(stderr,"invalid socket or message\n"));
   }
    return retval;
}
// End function mb1_sounding_send

#endif // WITH_MB1_UTILS
