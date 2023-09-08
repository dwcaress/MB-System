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
#include "mxdebug.h"
#include "mxd_app.h"
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

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////

/////////////////////////
// Function Definitions
/////////////////////////

#ifdef WITH_MB1_FRAME
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
#endif // WITH_MB1_FRAME


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
        //    MX_MPRINT(MBIO, "cycles[%d] forever[%s] c||f[%s]\n",cycles,(forever?"Y":"N"),(forever || (cycles>0) ? "Y" :"N"));

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
                MX_MPRINT(MB1IO_DEBUG, "ERR [%d/%s]\n", me_errno, me_strerror(me_errno));
                err++;
                tmout = (me_errno==ME_ETMOUT ? tmout+1 : tmout );
                if (me_errno==ME_ETMOUT || me_errno==ME_EOF || me_errno==ME_ESOCK) {
                    break;
                }
            }else{
                MX_MSG(MB1IO_DEBUG, "read returned 0\n");
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
            MX_MPRINT(MB1IO_DEBUG, "read headers [%"PRId64"/%"PRId64"]\n", nbytes, header_len);
            // get data and checksum
            mb1_header_t *hdr = (mb1_header_t *)(headers);

            // get size of data (beams, checksum)
            uint32_t data_len = MB1_BEAM_ARRAY_BYTES(hdr->nbeams);
            uint32_t read_len = data_len+MB1_CHECKSUM_BYTES;

            MX_MPRINT(MB1IO_DEBUG, "data_len[%u] read_len[%u]\n", data_len, read_len);
            // read rth/rd/od [if any], checksum
            if (read_len>0) {
                // INVALID C. JL
                // valid C99
                byte data[read_len];
                memset(data,0,read_len);
                if ( (nbytes=msock_read_tmout(s,data,read_len,timeout_msec)) == read_len) {
                    total_len+=nbytes;
                   MX_MPRINT(MB1IO_DEBUG, "read data [%"PRId64"/%d] -> %p\n", nbytes, read_len, dest);
                    // TODO: validate content
                    // create message
                    mb1_sounding_t *m = mb1_sounding_new(hdr->nbeams);
                    if (NULL!=m) {
                        memcpy(m,hdr,header_len);
                        byte *pdata = (byte *)m + MB1_HEADER_BYTES;
                        memcpy(pdata,data,nbytes);
//                       MX_MMSG(MB1IO_DEBUG, "dest msg:\n");
//                        mb1_sounding_show(m,true,6);
                        *dest=m;
                        retval=total_len;
                    }else{
                        MX_ERROR_MSG("recv - msg_new failed\n");
                    }
                }else{
                   MX_MPRINT(MB1IO_DEBUG, "recv - incomplete data read nbytes[%"PRId64"] data_len[%u]\n", nbytes, data_len);
                }
            }else{
               MX_MPRINT(MB1IO_DEBUG, "recv - read_len <= 0 nbytes[%"PRId64"] read_len[%u]\n", nbytes, read_len);
            }
        }else{
           MX_MPRINT(MB1IO_DEBUG, "recv - incomplete header read? nbytes[%"PRId64"] header_len[%"PRId64"]\n", nbytes, header_len);
        }
    }else{
        MX_MPRINT(MB1IO_DEBUG, "recv - invalid socket or status s[%p, %d/%d]\n", s,  (s!=NULL?s->status:0), SS_CONNECTED);
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
            //       MX_MMSG(MB1IO_DEBUG, "SEND nf:\n");
            //        mb1_nf_show((mb1_nf_t *)buf,true,4);
            int64_t status=0;

            if( (status=msock_send(s,buf,r_size))>0){
                retval=0;
                MX_MPRINT(MB1IO_DEBUG, "send OK s[%p, %d]\n", s, s->status);
            }else{
                MX_ERROR("send failed [%"PRId64"] [%d/%s]\n", status, errno, strerror(errno));
            }
        }

        free(buf);
    }else{
        MX_MMSG(MB1IO_DEBUG, "invalid socket or message\n");
   }
    return retval;
}
// End function mb1_sounding_send

#endif // WITH_MB1_UTILS
