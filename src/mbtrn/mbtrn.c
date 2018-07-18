///
/// @file mbtrn.c
/// @authors k. Headley
/// @date 01 jan 2018

/// MBSystem Terrain Relative Navigation library implementation

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

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "mbtrn.h"
#include "mconfig.h"
#include "mdebug.h"
#include "merror.h"

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


/// @fn mbtrn_connection_t * mbtrn_con_new()
/// @brief create new connection (to socket or file, not fully implemented).
/// @return new connection reference on success, NULL otherwise
static mbtrn_connection_t *mbtrn_con_new()
{
    mbtrn_connection_t *self=(mbtrn_connection_t *)malloc(sizeof(mbtrn_connection_t));
    if (self) {
        memset(self,0,sizeof(mbtrn_connection_t));
        self->sock_if =  NULL;
        self->file_if = NULL;
        self->state =CS_NEW;
        self->type = CT_NULL;
        self->auto_free = true;
    }
    return self;
}
// End function mbtrn_con_new


/// @fn mbtrn_connection_t * mbtrn_scon_new(iow_socket_t * s)
/// @brief create new socket connection.
/// @param[in] s socket reference
/// @return new connection reference on success, NULL otherwise
mbtrn_connection_t *mbtrn_scon_new(iow_socket_t *s)
{
    mbtrn_connection_t *self = mbtrn_con_new();
    if (self) {
        self->type = CT_SOCKET;
        self->sock_if = s;
    }
    return self;
}
// End function mbtrn_scon_new


/// @fn mbtrn_connection_t * mbtrn_fcon_new(iow_file_t * f)
/// @brief create new file connection.
/// @param[in] f file reference
/// @return new connection reference on success, NULL otherwise
mbtrn_connection_t *mbtrn_fcon_new(iow_file_t *f)
{
    mbtrn_connection_t *self = mbtrn_con_new();
    if (self) {
        self->type = CT_FILE;
        self->file_if = f;
        
    }
    return self;
}
// End function mbtrn_fcon_new


/// @fn void mbtrn_connection_destroy(mbtrn_connection_t ** pself)
/// @brief release connection resources.
/// @param[in] pself pointer to instance reference
/// @return none
void mbtrn_connection_destroy(mbtrn_connection_t **pself)
{
    if (pself) {
        mbtrn_connection_t *self = *pself;
        if (self) {
            if (self->auto_free) {
                iow_socket_destroy(&self->sock_if);
                iow_file_destroy(&self->file_if);
            }
            free(self);
            *pself=NULL;
        }
    }
}
// End function mbtrn_connection_destroy

/// @fn int mbtrn_reader_connect(mbtrn_reader_t *self)
/// @brief connect to 7k center and subscribe to records.
/// @param[in] self reader reference
/// @return 0 on success, -1 otherwise
int mbtrn_reader_connect(mbtrn_reader_t *self)
{
    int retval=-1;
    me_errno = ME_OK;
    
    if (NULL!=self) {
        
        char *host = strdup(self->s->addr->host);
        int port = self->s->addr->port;
        
        MMDEBUG(MBTRN,"destroying socket\n");
        iow_socket_destroy(&self->s);
        
        MMDEBUG(MBTRN,"building socket\n");
        self->s = iow_socket_new(host,port,ST_TCP);
        self->src->sock_if = self->s;

        MMDEBUG(MBTRN,"connecting to 7k center [%s]\n",self->s->addr->host);
        if(iow_connect(self->src->sock_if)==0){
            self->state=MBS_CONNECTED;
            MMDEBUG(MBTRN,"subscribing to 7k center [%s]\n",self->s->addr->host);
            if (r7k_subscribe(mbtrn_reader_sockif(self),self->sub_list,self->sub_count)==0) {
                self->state=MBS_SUBSCRIBED;
                retval=0;
            }else{
                MMDEBUG(MBTRN,"subscribe failed [%s]\n",self->s->addr->host);
//                self->state=ME_ESUB;
                me_errno=ME_ESUB;
                self->state=MBS_INITIALIZED;
            }
        }else{
//            self->state=ME_ECONNECT;
            MMDEBUG(MBTRN,"connect failed [%s]\n",self->s->addr->host);
            me_errno=ME_ECONNECT;
            self->state=MBS_INITIALIZED;
        }
    }
    return retval;
}
// End function mbtrn_reader_connect

/// @fn mbtrn_reader_t * mbtrn_reader_create(const char * host, int port, uint32_t capacity, uint32_t * slist, uint32_t slist_len)
/// @brief create new reson 7k reader. connect and subscribe to reson data.
/// @param[in] host reson 7k center host IP address
/// @param[in] port reson 7k center host IP port
/// @param[in] capacity input buffer capacity (bytes)
/// @param[in] slist 7k center message subscription list
/// @param[in] slist_len length of subscription list
/// @return new reson reader reference on success, NULL otherwise
mbtrn_reader_t *mbtrn_reader_create(const char *host, int port, uint32_t capacity, uint32_t *slist,  uint32_t slist_len)
{
    mbtrn_reader_t *self = (mbtrn_reader_t *)malloc(sizeof(mbtrn_reader_t));
    if (NULL != self) {
        
        self->state=MBS_NEW;
        self->s = iow_socket_new(host,port,ST_TCP);
        self->src = mbtrn_scon_new(self->s);
        self->sub_list = (uint32_t *)malloc(slist_len*sizeof(uint32_t));
        memset(self->sub_list,0,slist_len*sizeof(uint32_t));
        self->sub_count = slist_len;
        memcpy(self->sub_list,slist,slist_len*sizeof(uint32_t));
        self->fc = r7k_drfcon_new(capacity);

        self->state=MBS_INITIALIZED;
        
        if (NULL != self->src) {
            mbtrn_reader_connect(self);
        }else{
            self->state=ME_ECREATE;
        }
    }
    
    return self;
}
// End function mbtrn_reader_create


/// @fn void mbtrn_reader_destroy(mbtrn_reader_t ** pself)
/// @brief release reason 7k center reader resources.
/// @param[in] pself pointer to instance reference
/// @return none
void mbtrn_reader_destroy(mbtrn_reader_t **pself)
{
    if (NULL != pself) {
        mbtrn_reader_t *self = *pself;
        if (NULL != self) {
            if (self->src) {
                mbtrn_connection_destroy(&self->src);
            }
            if (self->sub_list) {
                free(self->sub_list);
            }
            if (self->fc) {
                r7k_drfcon_destroy(&self->fc);
            }
            free(self);
            *pself = NULL;
        }
    }
}
// End function mbtrn_reader_destroy


/// @fn iow_socket_t * mbtrn_reader_sockif(mbtrn_reader_t * self)
/// @brief get reader socket interface.
/// @param[in] self reader reference
/// @return socket reference on success, NULL otherwise.
iow_socket_t *mbtrn_reader_sockif(mbtrn_reader_t *self)
{
    return (NULL!=self ? self->src->sock_if : NULL);
}
// End function mbtrn_reader_sockif



/// @fn void mbtrn_reader_show(mbtrn_reader_t * self, _Bool verbose, uint16_t indent)
/// @brief output reader parameter summary to stderr.
/// @param[in] self reader reference
/// @param[in] verbose provide verbose output
/// @param[in] indent output indentation (spaces)
/// @return none
void mbtrn_reader_show(mbtrn_reader_t *self, bool verbose, uint16_t indent)
{
    if (NULL != self) {
        fprintf(stderr,"%*s[self      %10p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[src       %10p]\n",indent,(indent>0?" ":""), self->src);
        fprintf(stderr,"%*s[fc        %10p]\n",indent,(indent>0?" ":""), self->fc);
        // TODO : show drfcontainer
        if (verbose) {
            r7k_drfcon_show(self->fc,false,indent+3);
        }
        fprintf(stderr,"%*s[state    %2d/%s]\n",indent,(indent>0?" ":""), self->state, mbtrn_strstate(self->state));
        fprintf(stderr,"%*s[sub_count %10u]\n",indent,(indent>0?" ":""), self->sub_count);
        fprintf(stderr,"%*s[sub_list  %10p]\n",indent,(indent>0?" ":""), self->sub_list);
        if (verbose) {
            for (uint32_t i=0; i<self->sub_count; i++) {
                fprintf(stderr,"%*s[sub[%02d]  %10u]\n",indent+3,(indent+3>0?" ":""),i, self->sub_list[i]);
            }
        }
    }
}
// End function mbtrn_reader_show


/// @fn const char * mbtrn_strstate(mbtrn_state_t state)
/// @brief get string for reader state.
/// @param[in] state state ID
/// @return mnemonic string on success, NULL otherwise
const char *mbtrn_strstate(mbtrn_state_t state)
{
	const char *retval = "UNDEFINED";
    switch (state) {
        case MBS_NEW:
            retval = "NEW";
            break;
        case MBS_INITIALIZED:
            retval = "INITIALIZED";
            break;
        case MBS_CONNECTED:
            retval = "CONNECTED";
            break;
        case MBS_SUBSCRIBED:
            retval = "SUBSCRIBED";
            break;
            
        default:
            break;
    }
    return retval;
}
// End function mbtrn_strstate

/// @fn void void mbtrn_reader_purge(mbtrn_reader_t *self)
/// @brief empty reader frame container.
/// @param[in] self reader reference
/// @return none;
void mbtrn_reader_purge(mbtrn_reader_t *self)
{
    if (NULL!=self) {
        r7k_drfcon_flush(self->fc);
    }
}
// End function mbtrn_reader_purge

/// @fn void mbtrn_reader_flush(mbtrn_reader_t * self, uint32_t len, int32_t retries, uint32_t tmout_ms)
/// @brief flush reader input buffer.
/// @param[in] self reader reference
/// @param[in] len number of bytes of input to read
/// @param[in] retries number of retries
/// @param[in] tmout_ms timeout
/// @return none; attempts to read len characters at a tim
/// until a timeout occurs
void mbtrn_reader_flush(mbtrn_reader_t *self, uint32_t len, int32_t retries, uint32_t tmout_ms)
{
    if (NULL != self) {
        // read until timeout
        byte buf[len];
        int64_t x=0;
        uint32_t n=0;
        bool use_retries = (retries>0 ? true : false);

        do{
           x=iow_read_tmout(mbtrn_reader_sockif(self), buf, len, tmout_ms);
           n++;
            if (use_retries) {
                if (retries-- ==0) {
                    break;
                }
            }
        }while ( (x!=-1) && (me_errno!=(int)ME_ETMOUT));
//        MMDEBUG(MBTRN,"EXIT - retries[%d/%s] x[%lld] e[%d/%s] n[%u]\n",retries,(use_retries?"true":"false"),x,
//               me_errno,me_strerror(me_errno),n);
    }
}
// End function mbtrn_reader_flush


// return raw (possibly partial) network data frames

/// @fn int64_t mbtrn_reader_poll(mbtrn_reader_t * self, byte * dest, uint32_t len, uint32_t tmout_ms)
/// @brief read raw data from reson 7k center socket
/// @param[in] self reader reference
/// @param[in] dest data buffer
/// @param[in] len data request length
/// @param[in] tmout_ms timeout
/// @return returns number of bytes read into buffer on success, -1 otherwise
int64_t mbtrn_reader_poll(mbtrn_reader_t *self, byte *dest, uint32_t len, uint32_t tmout_ms)
{
    int64_t retval=-1;
    me_errno=ME_OK;
  
    if (NULL!=self && NULL!=dest) {
        
        int64_t rbytes=0;
        
		// may want to call mbtrn_reader_flush and usleep before
        // calling poll...
        
        
        // read up to len bytes or timeout
        // use len=MBTRN_TRN_PING_BYTES and MBTRN_TRN_PING_MSEC
        // to get a ping
        if ((rbytes=iow_read_tmout(mbtrn_reader_sockif(self), dest, len, tmout_ms)) > 0 ) {
            
            if ( (me_errno==ME_OK || me_errno==ME_ETMOUT) ) {
                retval = rbytes;
                
//                MMDEBUG(MBTRN,"buf[%p] req[%d] rd[%lld] to[%u]\n",dest,len,rbytes,tmout_ms);
            }else{
                // error
                MMDEBUG(MBTRN,"read err1 to[%"PRIu32"] merr[%d/%s] rb[%"PRId64"]\n",tmout_ms,me_errno,me_strerror(me_errno),rbytes);
                retval=-1;
            }
        }else{
            // error
            MMDEBUG(MBTRN,"read err2 to[%"PRIu32"] merr[%d/%s] rb[%"PRId64"]\n",tmout_ms,me_errno,me_strerror(me_errno),rbytes);
            retval=-1;
        }
    }else{
        MERROR("invalid argument\n");
    }

    return retval;
}
// End function mbtrn_reader_poll


/// @fn int64_t mbtrn_reader_parse(mbtrn_reader_t * self, byte * src, uint32_t len, r7k_drf_container_t * dest)
/// @brief parse raw 7k center data, return Data Record Frames (w/o Network Frames).
/// @param[in] self reader reference
/// @param[in] src raw data buffer
/// @param[in] len raw data bytes
/// @param[in] dest destination buffer (should be at least len bytes)
/// @return parsed data length in bytes on success, -1 otherwise
int64_t mbtrn_reader_parse(mbtrn_reader_t *self, byte *src, uint32_t len, r7k_drf_container_t *dest)
{
    int64_t retval=-1;

    int64_t parsed_bytes=0;
    
    //uint32_t status=0;
    r7k_parse_stat_t stats = {0},*status = &stats;
    
    if (NULL!=self && NULL!=src &&
        ( NULL!=dest || NULL!=self->fc) &&
        len>=(uint32_t)R7K_EMPTY_FRAME_BYTES ) {

        if ( (parsed_bytes = r7k_parse(src, len, (dest==NULL ? self->fc : dest), status)) > 0) {
            retval=status->parsed_records;
        }else{
            MMDEBUG(MBTRN,"parse_raw err [%d]\n",status->status);
        }
    }
    else{
        MERROR("invalid argument\n");
    }
    
    MMDEBUG(MBTRN,"returning [%0x]\n",status->status);
    return retval;
}
// End function mbtrn_reader_parse


/// @fn int64_t mbtrn_reader_xread(mbtrn_reader_t * self, byte * dest, uint32_t len, uint32_t tmout_ms, mbtrn_flags_t flags)
/// @brief combined poll and parse operation. used by MB System to
/// present a file-like view of reson 7k center data in real time.
/// automatically detects empty buffer and refills.
/// @param[in] self reader reference
/// @param[in] dest data buffer for parsed data record frames
/// @param[in] len data buffer length in bytes
/// @param[in] tmout_ms poll timeout
/// @param[in] flags behavior flags. e.g. force to flush/poll,
/// enable/disable incomplete reads
/// @return number of bytes read on success, -1 otherwise
int64_t mbtrn_reader_xread(mbtrn_reader_t *self, byte *dest, uint32_t len, uint32_t tmout_ms, mbtrn_flags_t flags)
{
    me_errno=ME_OK;
    int64_t retval=-1;
    int64_t read_len=0;
    int64_t read_bytes=0;
    int64_t poll_bytes=0;
    int64_t parse_bytes=0;
    r7k_parse_stat_t stats={0};
    r7k_parse_stat_t *parstat=&stats;
    int32_t pstat=0;

    memset(dest,0,len);
    if (NULL!=self && NULL!=dest && NULL!=self->fc) {
        byte buf[MBTRN_TRN_PING_BYTES]={0};
 
        // if force flag set, flush and poll first
        if (flags&MBR_FORCE) {
            if (flags&MBR_FLUSH || flags&MBR_IFLUSH) {
                // flush socket
                // then wait for for some bytes to arrive
                mbtrn_reader_flush(self,MBTRN_TRN_PING_BYTES,MBTRN_FLUSH_RETRIES, 500);
                // need this?
                usleep(MBTRN_PING_INTERVAL_USEC);
            }
            
            if (flags&MBR_FLUSH || flags&MBR_OFLUSH) {
                // flush contents of frame buffer
                r7k_drfcon_flush(self->fc);
            }
            memset(buf,0,MBTRN_TRN_PING_BYTES);
            if( (read_len=mbtrn_reader_poll(self,buf,MBTRN_TRN_PING_BYTES,3*MBTRN_PING_INTERVAL_MSEC))>0){
//                uint32_t stat=0;
                r7k_parse_stat_t stats2 = {0}, *stat = &stats2;
                
                if(r7k_parse(buf,read_len,self->fc,stat)==0){
                    MERROR("r7k_parse failed [%0x]\n",stat->status);
                    me_errno=ME_EPARSE;
                }
            }else{
                me_errno=ME_EPOLL;
            }
        }
        
        if ( r7k_drfcon_pending(self->fc) >= len ) {
            // if it's available, read it and return
            retval = r7k_drfcon_read(self->fc,dest,len);
//            MMDEBUG(MBTRN,"read OK [%lld]\n",retval);
            
        }else{
            if ( (flags&MBR_ALLOW_PARTIAL) ) {
                
                // read what's available, if any

                if( (read_bytes = r7k_drfcon_read(self->fc,dest,len)) >= 0){
                    MMDEBUG(MBTRN,"drfcon read OK [%"PRId64"]\n",read_bytes);
                    retval = read_bytes;
                }else{
                    MMDEBUG(MBTRN,"drfcon read failed\n");
                    retval = -1;
                    me_errno=ME_EREAD;
                }

                // if everything is read, flush the container
                // [to make room for more]
                if (r7k_drfcon_pending(self->fc)==0) {
                    if (r7k_drfcon_flush(self->fc)==0) {
                        MMDEBUG(MBTRN,"fc flush OK - size/length/pending %u/%u/%u\n",r7k_drfcon_size(self->fc),r7k_drfcon_length(self->fc),r7k_drfcon_pending(self->fc));
                    }else{
                        MMDEBUG(MBTRN,"fc flush failed\n");
                    }
                }
                
                // refill the buffer
                memset(buf,0,MBTRN_TRN_PING_BYTES);
                if( (poll_bytes=mbtrn_reader_poll(self,buf,MBTRN_TRN_PING_BYTES,3*MBTRN_PING_INTERVAL_MSEC) ) > 0){
                    
                    MMDEBUG(MBTRN,"poll OK [%"PRId64"/%u]\n",poll_bytes,(uint32_t)MBTRN_TRN_PING_BYTES);
                    if( (parse_bytes=r7k_parse(buf,poll_bytes,self->fc,&stats))>0){
                    	// success
                        MMDEBUG(MBTRN,"parse OK[%"PRId64"/%"PRId64"]\n",parse_bytes,poll_bytes);
                        
                        // re-read if partial read or error before
                        if (read_bytes<len) {
                            
                            byte *pdest = dest;
                            
                            if (read_bytes<=0) {
                                read_len = len;
                            }else if (read_bytes>0) {
                             	// we got a partial read before
                                read_len = len-read_bytes;
                                pdest = dest+read_bytes;
                            }
                            MMDEBUG(MBTRN,"re-read retval[%"PRId64"] read_bytes[%"PRId64"] req(read_len)[%"PRId64"]\n",retval,read_bytes,read_len);
//                            MMDEBUG(MBTRN,"retry read\n");
                            if( (read_bytes = r7k_drfcon_read(self->fc,pdest,read_len)) >= 0){
                                retval+=read_bytes;
                                MMDEBUG(MBTRN,"re-read OK [%"PRId64"/%"PRId64"]\n",retval,read_len);
                            }else{
                                MMDEBUG(MBTRN,"re-=read failed\n");
                                retval = -1;
                                me_errno=ME_EREAD;
                            }
                        }
                    }else{
                        MERROR("parse failed [%0x]\n",parstat->status);
                        me_errno=ME_EPARSE;
                    }
                    
                }else{
                    MERROR("poll failed [%0x] [%d/%s]\n",pstat,me_errno,me_strerror(me_errno));
                    if (me_errno==ME_ESOCK) {
                        self->state=MBS_INITIALIZED;
                        self->src->state=CS_INITIALIZED;
                        self->s->fd=-1;
                    }

                    //me_errno=ME_EPOLL;
                }
                
            }else{
                MMDEBUG(MBTRN,"full read only - skipping\n");
            }
        }
    }else{
        MERROR("invalid argument\n");
        retval=-1;
        me_errno = ME_EINVAL;
    }
//    MMDEBUG(MBTRN,"retval[%lld] merr[%d/%s]\n",retval,me_errno,me_strerror(me_errno));
    return retval;
}
// End function mbtrn_reader_xread


/// @fn int64_t mbtrn_reader_read(mbtrn_reader_t * self, byte * dest, uint32_t len)
/// @brief read from reson 7k center socket.
/// @param[in] self reader reference
/// @param[in] dest data buffer
/// @param[in] len data buffer (read) length
/// @return number of bytes read on success, -1 otherwise
int64_t mbtrn_reader_read(mbtrn_reader_t *self, byte *dest, uint32_t len)
{
    int64_t retval=0;
    if (NULL != self && NULL != dest) {
        return r7k_drfcon_read(self->fc,dest,len);
    }else{
        MERROR("invalid argument\n");
        retval=-1;
        me_errno = ME_EINVAL;
    }
    return retval;
}
// End function mbtrn_reader_read


/// @fn int64_t mbtrn_reader_seek(mbtrn_reader_t * self, uint32_t ofs)
/// @brief set output buffer (read) pointer.
/// @param[in] self reader reference
/// @param[in] ofs buffer offset
/// @return new file position on success, -1 otherwise
int64_t mbtrn_reader_seek(mbtrn_reader_t *self, uint32_t ofs)
{
    int64_t retval=-1;
    if (NULL != self) {
        return r7k_drfcon_seek(self->fc, ofs);
    }else{
        MERROR("invalid argument\n");
        retval=-1;
        me_errno = ME_EINVAL;
    }
    return retval;
}
// End function mbtrn_reader_seek


/// @fn int64_t mbtrn_reader_tell(mbtrn_reader_t * self)
/// @brief return current output buffer (read) pointer position.
/// @param[in] self reader reference
/// @return read pointer offset on success, -1 otherwise
int64_t mbtrn_reader_tell(mbtrn_reader_t *self)
{
    int64_t retval=-1;
    if (NULL != self) {
        return r7k_drfcon_tell(self->fc);
    }else{
        MERROR("invalid argument\n");
        retval=-1;
        me_errno = ME_EINVAL;
    }
    return retval;
}
// End function mbtrn_reader_tell


/// @fn uint32_t mbtrn_reader_frames(mbtrn_reader_t * self)
/// @brief return number of data record frames currently in buffer.
/// @param[in] self reader reference
/// @return number of data record frames on success, -1 otherwise.
uint32_t mbtrn_reader_frames(mbtrn_reader_t *self)
{
    uint32_t retval=-1;
    if (NULL != self) {
        return r7k_drfcon_frames(self->fc);
    }else{
        MERROR("invalid argument\n");
        retval=-1;
        me_errno = ME_EINVAL;
    }
    return retval;
}
// End function mbtrn_reader_frames


/// @fn r7k_drf_t * mbtrn_reader_enumerate(mbtrn_reader_t * self)
/// @brief set buffer pointer to first frame and return a pointer to it.
/// @param[in] self reader reference
/// @return pointer first data record frame in reader buffer on success, NULL otherwise
r7k_drf_t *mbtrn_reader_enumerate(mbtrn_reader_t *self)
{
    r7k_drf_t *retval=NULL;
    if (NULL != self) {
        return r7k_drfcon_enumerate(self->fc);
    }else{
        MERROR("invalid argument\n");
    }
    return retval;
}
// End function mbtrn_reader_enumerate


/// @fn r7k_drf_t * mbtrn_reader_next(mbtrn_reader_t * self)
/// @brief return reference to next frame in reader buffer.
/// should call mbtrn_reader_enumerate to start enumeration
/// @param[in] self reader reference
/// @return pointer to next DRF in reader buffer
r7k_drf_t    *mbtrn_reader_next(mbtrn_reader_t *self)
{
    r7k_drf_t *retval=NULL;
    if (NULL != self) {
        return r7k_drfcon_next(self->fc);
    }else{
        MERROR("invalid argument\n");
    }
    return retval;
}
// End function mbtrn_reader_next



/// @fn _Bool mbtrn_peer_cmp(void * a, void * b)
/// @brief compare iow_peer_t IDs. Used by mlist.
/// @param[in] a void pointer to iow_peer_t
/// @param[in] b void pointer to iow_peer_t
/// @return true if peer id's are equal, false otherwise
bool mbtrn_peer_cmp(void *a, void *b)
{
    bool retval=false;
    if (a!=NULL && b!=NULL) {
        iow_peer_t *pa = (iow_peer_t *)a;
        iow_peer_t *pb = (iow_peer_t *)b;
        retval = (pa->id == pb->id);
    }
    return retval;
}
// End function mbtrn_peer_cmp


/// @fn _Bool mbtrn_peer_vcmp(void * item, void * value)
/// @brief compare iow_peer_t ID to specified value. Used by mlist.
/// @param[in] item void pointer to iow_peer_t
/// @param[in] value void pointer to integer (id value)
/// @return returns true if the specified peer has the specified ID. false otherwise
bool mbtrn_peer_vcmp(void *item, void *value)
{
    bool retval=false;
    if (NULL!=item && NULL!=value) {
        iow_peer_t *peer = (iow_peer_t *)item;
        int svc = *((int *)value);
//        MMDEBUG(MBTRN,"peer[%p] id[%d] svc[%d]\n",peer,peer->id,svc);
        retval = (peer->id == svc);
    }
    return retval;
}
// End function mbtrn_peer_vcmp


