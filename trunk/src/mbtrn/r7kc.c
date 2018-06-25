///
/// @file r7kc.c
/// @authors k. Headley
/// @date 01 jan 2018
 
/// Reson 7k Center data structures and protocol API

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
#include <inttypes.h>
#include <errno.h>
#include "r7kc.h"
#include "mconfig.h"
#include "mdebug.h"

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

/// @fn int r7k_subscribe(iow_socket_t * s, uint32_t * records, uint32_t record_count)
/// @brief subscribe to reson 7k center messages.
/// @param[in] s socket reference
/// @param[in] records record subscription list
/// @param[in] record_count sub list length
/// @return 0 on success, -1 otherwise
int r7k_subscribe(iow_socket_t *s, uint32_t *records, uint32_t record_count)
{
    int retval=-1;
    
    if (NULL != s && NULL != records && record_count>0) {
        
        size_t rth_len = sizeof(r7k_rth_7500_rc_t);
        size_t rd_len = sizeof(r7k_sub_rd_t)+record_count*sizeof(uint32_t);
        
        r7k_msg_t *msg = r7k_msg_new(rth_len+rd_len);
        
        if (msg) {
            
            // set NF fields
            msg->nf->tx_id            = r7k_txid();
            msg->nf->protocol_version = R7K_NF_PROTO_VER;
            msg->nf->seq_number      = 0;
            msg->nf->offset          = sizeof(r7k_nf_t);
            msg->nf->packet_size     = msg->msg_len;
            msg->nf->total_size      = msg->msg_len - sizeof(r7k_nf_t);
            msg->nf->dest_dev_id     = 0;
            msg->nf->dest_enumerator = 0;
            msg->nf->src_enumerator  = 0;
            msg->nf->src_dev_id      = 0;
            
            // set DRF fields
            msg->drf->size           = DRF_SIZE(msg);
            msg->drf->record_type_id = R7K_RT_REMCON;
            msg->drf->device_id      = R7K_DEVID_7KCENTER;
            msg->drf->sys_enumerator = R7K_DRF_SYS_ENUM_400KHZ;
            
            
            // set record type header info
            r7k_rth_7500_rc_t *prth =(r7k_rth_7500_rc_t *)(msg->data);
            prth->remcon_id = R7K_RTID_SUB;
            r7k_sub_rd_t *prdata = (r7k_sub_rd_t *)(msg->data+rth_len);
            prdata->record_count = record_count;
            
            // set record data
            memcpy((msg->data+rth_len+sizeof(r7k_sub_rd_t)),records, (record_count*sizeof(uint32_t)));
            
            // set checksum [do last]
            // TODO: optionally do in send()?
            r7k_msg_set_checksum(msg);
            
            // serialize, send
           MMDEBUG(R7K,"sending SUB request:\n");
            if(mdb_get(R7K,NULL)>MD_WARN){
            r7k_msg_show(msg,true,3);
            }
            r7k_msg_send(s, msg);
            
            // get ACK/NAK
            r7k_msg_t *reply = NULL;
            r7k_msg_receive(s, &reply, R7K_SUBSCRIBE_TIMEOUT_MS);
            
           MMDEBUG(R7K,"SUB reply received [%p]:\n",reply);
            if(mdb_get(R7K,NULL)>MD_WARN){
            r7k_msg_show(reply,true,3);
            }
            // validate reply
            
            // release resources
            r7k_msg_destroy(&msg);
            r7k_msg_destroy(&reply);
            retval=0;
        }
    }else{
    	MERROR("ERR - invalid argument\n");
    }
    return retval;
}
// End function r7k_subscribe


/// @fn int r7k_unsubscribe(iow_socket_t * s)
/// @brief unsubscribe from reson 7k records (not implemented).
/// @param[in] s socket reference
/// @return 0 on success, -1 otherwise
int r7k_unsubscribe(iow_socket_t *s)
{
    int retval=-1;
    MERROR("ERR - not implemented\n");
    return retval;
}
// End function r7k_unsubscribe


/// @fn uint16_t r7k_txid()
/// @brief transmission ID (for messages sent to r7kc).
/// @return transmission ID 0-65535
uint16_t r7k_txid()
{
    static uint16_t txid=0;
    return ++txid;
}
// End function r7k_txid


/// @fn uint32_t r7k_checksum(byte * pdata, uint32_t len)
/// @brief return r7k checksum for data.
/// @param[in] pdata data pointer
/// @param[in] len length of data.
/// @return r7k checksum value (sum of bytes).
uint32_t r7k_checksum(byte *pdata, uint32_t len)
{
    uint32_t checksum=0;
    if (NULL!=pdata) {
        byte *bp = pdata;
        for (uint32_t i=0; i<len; i++) {
            checksum+=*(bp+i);
        }
    }
    return checksum;
}
// End function r7k_checksum


/// @fn void r7k_update_time(r7k_time_t * t7k)
/// @brief set the time in r7k time format.
/// @param[in] t7k r7k time structure reference
/// @return none
void r7k_update_time(r7k_time_t *t7k)
{
    if ( (NULL!=t7k) ) {
        // ANSI time (sec resolution)
        // supported on OSX, Windows, *NIX
        struct tm tms={0};
        time_t tt = time(NULL);
        gmtime_r(&tt, &tms);
        t7k->year    = tms.tm_year%100;
        t7k->day     = tms.tm_yday;
        t7k->hours   = tms.tm_hour;
        t7k->minutes = tms.tm_min;
        t7k->seconds = (tms.tm_sec == 60. ? 0. : (float)tms.tm_sec);
        
        // Posix time (nsec resolution)
        // not supported on OSX, Windows
        // struct timespec ts;
        // clock_gettime(CLOCK_REALTIME, &ts);
        // time_t tt = ts.tv_sec;
        // struct tm tms=NULL;
        // gmtime_r(&tt, &tms);
        // drf->_7ktime.year    = tms.tm_year%100;
        // drf->_7ktime.day     = tms.tm_yday;
        // drf->_7ktime.hours   = tms.tm_hour;
        // drf->_7ktime.minutes = tms.tm_min;
        // drf->_7ktime.seconds = tms.tm_sec + ((float)ts.tv_nsec/1000000000.);
        
    }
}
// End function r7k_update_time


/// @fn void r7k_hex_show(byte * data, uint32_t len, uint16_t cols, _Bool show_offsets, uint16_t indent)
/// @brief output data buffer bytes in hex to stderr.
/// @param[in] data buffer pointer
/// @param[in] len number of bytes to display
/// @param[in] cols number of columns to display
/// @param[in] show_offsets show starting offset for each row
/// @param[in] indent output indent spaces
/// @return none
void r7k_hex_show(byte *data, uint32_t len, uint16_t cols, bool show_offsets, uint16_t indent)
{
    if (NULL!=data && len>0 && cols>0) {
        int rows = len/cols;
        int rem = len%cols;
        int i=0,j=0;
        byte *p=data;
        for (i=0; i<rows; i++) {
            if (show_offsets) {
                fprintf(stderr,"%*s%04zd [",indent,(indent>0?" ":""),(p-data));
            }else{
                fprintf(stderr,"%*s[",indent,(indent>0?" ":""));
            }
            for (j=0; j<cols; j++) {
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
                fprintf(stderr,"%*s%04zd [",indent,(indent>0?" ":""),(p-data));
            }else{
                fprintf(stderr,"%*s[",indent,(indent>0?" ":""));
            }
            for (j=0; j<rem; j++) {
                fprintf(stderr," %02x",*p++);
            }
            fprintf(stderr,"%*s ]\n",3*(cols-rem)," ");
        }
        
    }
}
// End function r7k_hex_show


/// @fn void r7k_parser_show(r7k_parse_stat_t * self, _Bool verbose, uint16_t indent)
/// @brief output r7k parser statistics to stderr.
/// @param[in] self parser stats structure reference
/// @param[in] verbose use verbose output
/// @param[in] indent output indent spaces
/// @return none
void r7k_parser_show(r7k_parse_stat_t *self, bool verbose, uint16_t indent)
{
    if (NULL!=self) {
        fprintf(stderr,"%*s[self           %10p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[src_bytes      %10u]\n",indent,(indent>0?" ":""), self->src_bytes);
        fprintf(stderr,"%*s[sync_bytes     %10u]\n",indent,(indent>0?" ":""), self->sync_bytes);
        fprintf(stderr,"%*s[unread_bytes   %10u]\n",indent,(indent>0?" ":""), self->unread_bytes);
        fprintf(stderr,"%*s[parsed_records %10u]\n",indent,(indent>0?" ":""), self->parsed_records);
        fprintf(stderr,"%*s[parsed_bytes   %10u]\n",indent,(indent>0?" ":""), self->parsed_bytes);
        fprintf(stderr,"%*s[resync_count   %10u]\n",indent,(indent>0?" ":""), self->resync_count);
        fprintf(stderr,"%*s[status         %10u]\n",indent,(indent>0?" ":""), self->status);
    }
}
// End function r7k_parser_show


/// @fn char * r7k_parser_str(r7k_parse_stat_t * self, char * dest, uint32_t len, _Bool verbose, uint16_t indent)
/// @brief output parser statistics to a string. Caller must free.
/// @param[in] self parser stat struct reference
/// @param[in] dest string output buffer, use NULL to allocate new string
/// @param[in] len buffer length
/// @param[in] verbose use verbose output
/// @param[in] indent output indent spaces
/// @return new string (if dest==NULL) or dest on success, NULL otherwise. May truncate if buffer length is unsufficient.
char *r7k_parser_str(r7k_parse_stat_t *self, char *dest, uint32_t len, bool verbose, uint16_t indent)
{
    char *retval=NULL;
    char *dp=NULL;
    char *wbuf=NULL;
    char *fmt=NULL;
    uint32_t wlen=len;
    uint32_t wb=0;
    uint32_t inc=256;
    uint32_t n=1;
    
    if (NULL!=self) {
        
        wbuf=(char *)malloc(inc*sizeof(char));
        memset(wbuf,0,len);
    	dp=wbuf;

        // TODO: check length/realloc
        if (dp!=NULL) {
            fmt="%*s[self           %10p]\n";
            wb = snprintf(NULL,0,fmt,indent,(indent>0?" ":""), self);
            wlen+=wb;
            if (wlen>inc) {
                n++;
                wbuf=(char *)realloc(wbuf,n*inc*sizeof(char));
                dp = wbuf+strlen(wbuf);
            }
            sprintf(dp,fmt,indent,(indent>0?" ":""), self);
            dp = wbuf+strlen(wbuf);
            
            fmt="%*s[src_bytes      %10u]\n";
            wb = snprintf(NULL,0,fmt,indent,(indent>0?" ":""), self->src_bytes);
            wlen+=wb;
            if (wlen>inc) {
                n++;
                wbuf=(char *)realloc(wbuf,n*inc*sizeof(char));
                dp = wbuf+strlen(wbuf);
            }
            sprintf(dp,fmt,indent,(indent>0?" ":""), self->src_bytes);
            dp = wbuf+strlen(wbuf);
            
            fmt="%*s[sync_bytes     %10u]\n";
            wb = snprintf(NULL,0,fmt,indent,(indent>0?" ":""), self->sync_bytes);
            wlen+=wb;
            if (wlen>inc) {
                n++;
                wbuf=(char *)realloc(wbuf,n*inc*sizeof(char));
                dp = wbuf+strlen(wbuf);
            }
            sprintf(dp,fmt,indent,(indent>0?" ":""), self->sync_bytes);
            dp = wbuf+strlen(wbuf);
            
            fmt="%*s[unread_bytes   %10u]\n";
            wb = snprintf(NULL,0,fmt,indent,(indent>0?" ":""), self->unread_bytes);
            wlen+=wb;
            if (wlen>inc) {
                n++;
                wbuf=(char *)realloc(wbuf,n*inc*sizeof(char));
                dp = wbuf+strlen(wbuf);
            }
            sprintf(dp,fmt,indent,(indent>0?" ":""), self->unread_bytes);
            dp = wbuf+strlen(wbuf);
            
            
            fmt="%*s[parsed_records %10u]\n";
            wb = snprintf(NULL,0,fmt,indent,(indent>0?" ":""), self->parsed_records);
            wlen+=wb;
            if (wlen>inc) {
                n++;
                wbuf=(char *)realloc(wbuf,n*inc*sizeof(char));
                dp = wbuf+strlen(wbuf);
            }
            sprintf(dp,fmt,indent,(indent>0?" ":""), self->parsed_records);
            dp = wbuf+strlen(wbuf);
            
            fmt="%*s[parsed_bytes   %10u]\n";
            wb = snprintf(NULL,0,fmt,indent,(indent>0?" ":""), self->parsed_bytes);
            wlen+=wb;
            if (wlen>inc) {
                n++;
                wbuf=(char *)realloc(wbuf,n*inc*sizeof(char));
                dp = wbuf+strlen(wbuf);
            }
            sprintf(dp,fmt,indent,(indent>0?" ":""), self->parsed_bytes);
            dp = wbuf+strlen(wbuf);
            
            fmt="%*s[resync_count   %10u]\n";
            wb = snprintf(NULL,0,fmt,indent,(indent>0?" ":""), self->resync_count);
            wlen+=wb;
            if (wlen>inc) {
                n++;
                wbuf=(char *)realloc(wbuf,n*inc*sizeof(char));
                dp = wbuf+strlen(wbuf);
            }
            sprintf(dp,fmt,indent,(indent>0?" ":""), self->resync_count);
            dp = wbuf+strlen(wbuf);
            
            fmt="%*s[status         %10u]\n";
            wb = snprintf(NULL,0,fmt,indent,(indent>0?" ":""), self->status);
            wlen+=wb;
            if (wlen>inc) {
                n++;
                wbuf=(char *)realloc(wbuf,n*inc*sizeof(char));
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
// End function r7k_parser_str


// extract valid DRFs from raw network frames
// stops when src parsed or dest full
/// @fn uint32_t r7k_parse(byte * src, uint32_t len, r7k_drf_container_t * dest, r7k_parse_stat_t * status)
/// @brief parse r7k network frames, return buffer with parsed data record frames.
/// does some validation, e.g. length, checksum, rejects invalid frames
/// @param[in] src buffer containing raw r7k data (i.e. network frames)
/// @param[in] len size of src/dest buffer
/// @param[in] dest destination buffer for parsed data (i.e. data record frames
/// @param[in] status parser status structure
/// @return number of bytes returned in destination buffer
uint32_t r7k_parse(byte *src, uint32_t len, r7k_drf_container_t *dest, r7k_parse_stat_t *status)
{
    int retval=-1;
    me_errno=ME_OK;
    
    uint32_t record_count = 0;
    uint32_t sync_bytes = 0;
    r7k_nf_t *pnf=NULL;
    r7k_drf_t *pdrf=NULL;
    r7k_checksum_t *pchk=NULL;
    byte *psrc = NULL;
    
    if (NULL != src && NULL!=dest) {
        psrc=src;
        memset(status,0,sizeof(r7k_parse_stat_t));
        status->src_bytes = len;
        
        bool resync=false;
        
        // move src pointer along, and mark
        // add records to the drf container (expands dynamically)
        // stop when we've found the end of the source buffer
        
        while (psrc<(src+len)) {
            
            pnf = (r7k_nf_t *)psrc;
//            MMDEBUG(R7K,"psrc[%p]\n",psrc);
            // pnf is legit?...
            if ( (pnf->protocol_version == (uint16_t)R7K_NF_PROTO_VER) &&
                (pnf->total_packets > (uint32_t)0) &&
                (pnf->total_size >= (uint32_t)R7K_DRF_BYTES)
                ) {
                
                pdrf=(r7k_drf_t *)(psrc + R7K_NF_BYTES);
                
                // check DRF
                if(pdrf->protocol_version == (uint16_t)R7K_DRF_PROTO_VER &&
                   pdrf->sync_pattern == (uint32_t)R7K_DRF_SYNC_PATTERN &&
                   pdrf->size > (uint32_t)R7K_DRF_BYTES){
                    byte *pw = (byte *)pdrf;
                    // this fixes an uninitialized variable (cs)
                    // warning (in valgrind? compiler?)
                    pw[0]=pw[0];
                    pchk = (r7k_checksum_t *)(pw+pdrf->size-R7K_CHECKSUM_BYTES);
                    r7k_checksum_t cs = 0;
                    cs = r7k_checksum(pw,(pdrf->size-R7K_CHECKSUM_BYTES));
                    
                    // checksum is valid or unused (flags:0 unset)
                    if ( ((pdrf->flags&0x1) == 0) || cs == *pchk ) { // cs UNINIT
                        // found a valid record...
                        
                        // add it to the frame container
                        // also adds frame offset info
                        if(r7k_drfcon_add(dest,(byte *)pdrf,pdrf->size)==0){
//                            byte *prev=psrc;
                            // update src pointer
                            psrc = ((byte *)pchk + R7K_CHECKSUM_BYTES+R7K_CHECKSUM_BYTES);
                            
                            // update record count
                            record_count++;
                            
//                            MMDEBUG(R7K,"adding record prv[%p] nxt[%p]\n",prev,psrc);
                            // set retval to parsed bytes
                            retval = r7k_drfcon_length(dest);
                            
                            resync=false;
                            status->parsed_records++;
                            status->status=ME_OK;
                            
                        }else{
                            MMDEBUG(R7K,"DRF container full\n");
                            status->status = ME_ENOSPACE;
                            me_errno = ME_ENOSPACE;
                            break;
                        }
                        
                    }else{
                        MMDEBUG(R7K,"CHKSUM err: checksum mismatch p/c[%u/%u]\n",*pchk,cs);
                        resync=true;
                    }
                }else{
                    MMDEBUG(R7K,"DRF err\n");
                    //                    r7k_drf_show(pdrf,true,3);
                    //                    r7k_hex_show(psrc,0x50,16,true,5);
                    resync=true;
                }
            }else{
                MMDEBUG(R7K,"NRF err: psrc[%p] ofs[%zd] protov[%hu] totpkt[%u] totsz[%u]\n",
                        psrc,
                        (psrc-src),
                        pnf->protocol_version,
                        pnf->total_packets,
                        pnf->total_size);
                
                resync=true;
            }
            
            if (resync) {
                // search for the next valid network frame
                int64_t x=0;
                // test NF, DRF proto versions and DRF sync pattern
                // to indicate possibly valid frame
                bool sync_found=false;
                size_t hdr_len =(R7K_NF_BYTES+R7K_DRF_BYTES);
                 size_t oofs=(psrc-src);
                
                bool capacity_ok= false;
                size_t space_rem=0;
                // search until sync found or end of buffer
                while ( NULL != psrc && !sync_found){
                    
                    space_rem = (src+len-psrc);
                    // set net frame and data record pointers
                    pnf = (r7k_nf_t *)psrc;
                    pdrf = (r7k_drf_t *)(psrc+R7K_NF_BYTES);
                    
                    capacity_ok= (space_rem >= hdr_len);
                    bool hdr_valid = (( pnf->protocol_version == (uint16_t)R7K_NF_PROTO_VER ) &&
                                      ( pnf->total_packets > (uint32_t)0 ) &&
                                      ( pnf->total_size >= (uint32_t)R7K_DRF_BYTES) &&
                                      ( pdrf->protocol_version  == (uint16_t)R7K_DRF_PROTO_VER ) &&
                                      ( pdrf->sync_pattern  == (uint32_t)R7K_DRF_SYNC_PATTERN) );
                    
                    // stop if we find a valid record
                    // or there isn't remaining space for one
                    if(  capacity_ok && hdr_valid ){
                        sync_found=true;
                        break;
                    }
                    if (!capacity_ok) {
                        break;
                    }
                    // advance source pointer
                    psrc++;
                    x++;
                    sync_bytes++;
                    status->sync_bytes++;
                }

                
                if (sync_found){
                    MMDEBUG(R7K,"skipped %"PRId64" bytes oofs[%zd] new_ofs[%zd]\n",x,oofs,(psrc-src));
//                    MMDEBUG(R7K,"nrf ofs[%zd] protov[%hu] totpkt[%u] totsz[%u]\n",
//                            (psrc-src),
//                            pnf->protocol_version,
//                            pnf->total_packets,
//                            pnf->total_size);
                }else{
                    MMDEBUG(R7K,"ERR - resync failed: spc[%zd] hdr_len[%zu] skipped [%"PRId64"]\n",space_rem,hdr_len,x);
                    if (!capacity_ok) {
                        MMDEBUG(R7K,"DRF container full\n");
                        status->status = ME_ENOSPACE;
                        me_errno = ME_ENOSPACE;
                        break;
                    }
                }
                status->resync_count++;
                resync=false;
            }
        }
        status->unread_bytes = ((src+len)-psrc);
        status->parsed_bytes = r7k_drfcon_length(dest);
        if (mdb_get(RPARSER,NULL)==MDL_DEBUG) {
            r7k_parser_show(status,true,5);
        }
        MMDEBUG(RPARSER,"valid[%d] resyn[%d] sync[%d] rv[%d]\n",status->parsed_records,status->resync_count,status->sync_bytes,retval);
    }
    return retval;
}
// End function r7k_parse


/// @fn int r7k_stream_show(iow_socket_t * s, int sz, uint32_t tmout_ms, int cycles)
/// @brief output raw r7k stream to stderr as formatted ASCII hex.
/// @param[in] s r7k host socket
/// @param[in] sz read buffer size (read sz bytes at a time)
/// @param[in] tmout_ms read timeout
/// @param[in] cycles number of cycles to read (<=0 read forever)
/// @return 0 on success, -1 otherwise
int r7k_stream_show(iow_socket_t *s, int sz, uint32_t tmout_ms, int cycles)
{
    int retval=-1;
    int x=(sz<=0?16:sz);
    byte buf[x];
    int64_t test=0;
    int good=0,err=0,zero=0,tmout=0;
    int status = 0;
    bool forever=true;
    int count=0;

    if (cycles>0) {
        forever=false;
    }
    //    MERROR("cycles[%d] forever[%s] c||f[%s]\n",cycles,(forever?"Y":"N"),(forever || (cycles>0) ? "Y" :"N"));

    // read cycles or forever (cycles<=0)
    while ( forever || (count++ < cycles)) {
        memset(buf,0,x);
        test = iow_read_tmout(s, buf, x, tmout_ms);
        if(test>0){
            good++;
            r7k_hex_show(buf, test, 16, true, 3);
            fprintf(stderr,"c[%d/%d] ret[%"PRId64"/%u] stat[%d] good/zero/tmout/err [%d/%d/%d/%d]\n",count,cycles,test,sz,status,good,zero,tmout,err);
            retval=0;
        }else if(test<0){
           MMDEBUG(R7K,"ERR [%d/%s]\n",me_errno,me_strerror(me_errno));
            err = (me_errno==ME_ETMOUT ? err : err+1 );
            tmout = (me_errno==ME_ETMOUT ? tmout+1 : tmout );
        }else{
           MMDEBUG(R7K,"read returned 0\n");
            zero++;
        }
    }
    return retval;
}
// End function r7k_stream_show


/// @fn r7k_nf_t * r7k_nf_new()
/// @brief create new r7k network frame structure. used mostly by components.
/// @return reference to new instance on success, NULL otherwise
r7k_nf_t *r7k_nf_new()
{
    r7k_nf_t *self = (r7k_nf_t *)malloc( sizeof(r7k_nf_t) );
    if (NULL!=self) {
        r7k_nf_init(&self,true);
    }
    return self;
}
// End function r7k_nf_new


/// @fn void r7k_nf_destroy(r7k_nf_t ** pself)
/// @brief release network frame structure resources.
/// @param[in] pself pointer to instance reference
/// @return none
void r7k_nf_destroy(r7k_nf_t **pself)
{
    if (pself) {
        r7k_nf_t *self = *pself;
        if (self) {
            free(self);
        }
        *pself=NULL;
    }
	
}
// End function r7k_nf_destroy


/// @fn r7k_nf_t * r7k_nf_init(r7k_nf_t ** pnf, _Bool erase)
/// @brief initialize network frame with common defaults. create new
/// network frame if pnf argument is NULL.
/// @param[in] pnf network frame to initialize (or NULL to create new)
/// @param[in] erase zero network frame before initializing
/// @return network frame reference on success, NULL otherwise
r7k_nf_t * r7k_nf_init(r7k_nf_t **pnf,bool erase)
{
    r7k_nf_t *nf=NULL;
    
    if (NULL == pnf) {
        nf = (r7k_nf_t *)malloc(sizeof(r7k_nf_t));
        memset(nf,0,sizeof(r7k_nf_t));
        *pnf = nf;
    }else{
        nf = *pnf;
    }
    if (NULL!=nf) {
        if (erase) {
            memset(nf,0,sizeof(r7k_nf_t));
        }
        // caller must set:
        // total_size
        // packet_size
        
        // caller may optionally set
        // total packets
        // trans_id
        // seq_number
        
        nf->protocol_version = R7K_NF_PROTO_VER;
        nf->offset           = sizeof(r7k_nf_headers_t);
        nf->total_packets    = 1;
        nf->total_records    = 1;
        nf->tx_id            = 0;
        
        nf->seq_number       = 0;
        nf->dest_dev_id      = R7K_DEVID_7KCENTER;
        nf->dest_enumerator  = 0;
        nf->src_enumerator   = 0;
        nf->src_dev_id       = R7K_NF_DEVID_UNUSED;
    }
    
    
    return nf;
}
// End function r7k_nf_init


/// @fn void r7k_nf_show(r7k_nf_t * self, _Bool verbose, uint16_t indent)
/// @brief output network frame structure parameter summary to stderr.
/// @param[in] self network fram struct reference
/// @param[in] verbose use verbose output
/// @param[in] indent output indent spaces
/// @return none
void r7k_nf_show(r7k_nf_t *self, bool verbose, uint16_t indent)
{
    if (self) {
        fprintf(stderr,"%*s[self             %10p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[protocol_version %10hu]\n",indent,(indent>0?" ":""), self->protocol_version);
        fprintf(stderr,"%*s[offset           %10hu]\n",indent,(indent>0?" ":""), self->offset);
        fprintf(stderr,"%*s[total_packets    %10u]\n",indent,(indent>0?" ":""), self->total_packets);
        fprintf(stderr,"%*s[total_records    %10hu]\n",indent,(indent>0?" ":""), self->total_records);
        fprintf(stderr,"%*s[tx_id            %10hu]\n",indent,(indent>0?" ":""), self->tx_id);
        fprintf(stderr,"%*s[packet_size      %10u]\n",indent,(indent>0?" ":""), self->packet_size);
        fprintf(stderr,"%*s[total_size       %10u]\n",indent,(indent>0?" ":""), self->total_size);
        fprintf(stderr,"%*s[seq_number       %10u]\n",indent,(indent>0?" ":""), self->seq_number);
        fprintf(stderr,"%*s[dest_dev_id      %10u]\n",indent,(indent>0?" ":""), self->dest_dev_id);
        fprintf(stderr,"%*s[dest_enumerator  %10hu]\n",indent,(indent>0?" ":""), self->dest_enumerator);
        fprintf(stderr,"%*s[src_enumerator   %10hu]\n",indent,(indent>0?" ":""), self->src_enumerator);
        fprintf(stderr,"%*s[src_dev_id       %10u]\n",indent,(indent>0?" ":""), self->src_dev_id);
    }
    
}
// End function r7k_nf_show


/// @fn r7k_drf_t * r7k_drf_new()
/// @brief TBD.
/// @return TBD
r7k_drf_t *r7k_drf_new()
{
    r7k_drf_t *self = (r7k_drf_t *)malloc( sizeof(r7k_drf_t) );
    if (NULL!=self) {
        r7k_drf_init(self,true);
    }
    return self;
}
// End function r7k_drf_new

/// @fn void r7k_drf_destroy(r7k_drf_t ** pself)
/// @brief TBD.
/// @param[in] pself TBD
/// @return TBD
void r7k_drf_destroy(r7k_drf_t **pself)
{
    if (pself) {
        r7k_drf_t *self = *pself;
        if (self) {
            free(self);
        }
        *pself=NULL;
    }
}
// End function r7k_drf_destroy


/// @fn void r7k_drf_show(r7k_drf_t * self, _Bool verbose, uint16_t indent)
/// @brief TBD.
/// @param[in] self TBD
/// @param[in] verbose TBD
/// @param[in] indent TBD
/// @return TBD
void r7k_drf_show(r7k_drf_t *self, bool verbose, uint16_t indent)
{
    if (self) {
        fprintf(stderr,"%*s[self            %15p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[protocol_ver    %15hu]\n",indent,(indent>0?" ":""), self->protocol_version);
        fprintf(stderr,"%*s[offset          %15hu]\n",indent,(indent>0?" ":""), self->offset);
        fprintf(stderr,"%*s[sync_pattern    %*c0x%04x]\n",indent,(indent>0?" ":""),9,' ',self->sync_pattern);
        fprintf(stderr,"%*s[size            %15u]\n",indent,(indent>0?" ":""), self->size);
        fprintf(stderr,"%*s[opt_data_offset %15u]\n",indent,(indent>0?" ":""), self->opt_data_offset);
        fprintf(stderr,"%*s[opt_data_id     %15u]\n",indent,(indent>0?" ":""), self->opt_data_id);
        fprintf(stderr,"%*s[_7ktime   %02hu %03hu %02hhu:%02hhu:%2.3f]\n", \
                indent,(indent>0?" ":""), self->_7ktime.year, self->_7ktime.day, \
                self->_7ktime.hours, self->_7ktime.minutes, self->_7ktime.seconds);
        fprintf(stderr,"%*s[record_version  %15hu]\n",indent,(indent>0?" ":""), self->record_version);
        fprintf(stderr,"%*s[record_type_id  %15u]\n",indent,(indent>0?" ":""), self->record_type_id);
        fprintf(stderr,"%*s[device_id       %15u]\n",indent,(indent>0?" ":""), self->device_id);
        fprintf(stderr,"%*s[reserved0       %15hu]\n",indent,(indent>0?" ":""), self->reserved0);
        fprintf(stderr,"%*s[sys_enumerator  %15hu]\n",indent,(indent>0?" ":""), self->sys_enumerator);
        fprintf(stderr,"%*s[reserved1       %15u]\n",indent,(indent>0?" ":""), self->reserved1);
        fprintf(stderr,"%*s[flags           %15hu]\n",indent,(indent>0?" ":""), self->flags);
        fprintf(stderr,"%*s[reserved2       %15hu]\n",indent,(indent>0?" ":""), self->reserved2);
        fprintf(stderr,"%*s[reserved3       %15u]\n",indent,(indent>0?" ":""), self->reserved3);
        fprintf(stderr,"%*s[total_frag_recs %15u]\n",indent,(indent>0?" ":""), self->total_frag_recs);
        fprintf(stderr,"%*s[frag_number     %15u]\n",indent,(indent>0?" ":""), self->frag_number);
        // this won't work here, since drf has no data, but could be used elsewhere
        // for parsed records
//        if (verbose) {
//            byte *pdata = ((byte *)&self->sync_pattern)+self->offset;
//            uint32_t data_sz = self->size-R7K_DRF_BYTES-R7K_CHECKSUM_BYTES;
//            fprintf(stderr,"%*s[data:            %10p]\n",indent,(indent>0?" ":""),pdata);
//            fprintf(stderr,"%*s[data bytes:      %10u]\n",indent,(indent>0?" ":""),data_sz);
//            
//            r7k_hex_show(pdata,data_sz,16,true,indent+3);
//            
//            fprintf(stderr,"%*s[checksum          x%08x]\n",indent,(indent>0?" ":""), r7k_drf_get_checksum(self));
//        }
    }
}
// End function r7k_drf_show


/// @fn r7k_checksum_t r7k_drf_get_checksum(r7k_drf_t * self)
/// @brief return checksum of parsed data record frame (DRF).
/// @param[in] self drf reference
/// @return checksum on success, 0 otherwise
r7k_checksum_t r7k_drf_get_checksum(r7k_drf_t *self)
{
    r7k_checksum_t retval=0;
    
    if (self) {
        byte *bp = (byte *)self;
        r7k_checksum_t *pchk = (r7k_checksum_t *)(bp+self->size-R7K_CHECKSUM_BYTES);
        retval = *pchk;
    }
    return retval;
}
// End function r7k_drf_get_checksum


/// @fn void r7k_drf_init(r7k_drf_t * drf, _Bool erase)
/// @brief initialize a data record frame structure.
/// @param[in] drf data record frame reference
/// @param[in] erase zero DRF structure before initialization
/// @return none
void r7k_drf_init(r7k_drf_t *drf, bool erase)
{
    if (NULL == drf) {
        drf = (r7k_drf_t *)malloc(sizeof(r7k_drf_t));
        memset(drf,0,sizeof(r7k_drf_t));
    }
    if (NULL!=drf) {
        if (erase) {
            memset(drf,0,sizeof(r7k_drf_t));
        }
        // caller must set:
        // size
        // _7ktime
        // record_type_id
        
        // and optionally set
        // device_id
        // opt_data_offset
        // opt_data_id

        drf->protocol_version = R7K_DRF_PROTO_VER;
        drf->offset           = sizeof(r7k_drf_t)-2*sizeof(uint16_t);
        drf->sync_pattern     = R7K_DRF_SYNC_PATTERN;
        //drf->size;
        drf->opt_data_offset  = 0;
        drf->opt_data_id      = 0;
        //drf->_7ktime;
        drf->record_version   = R7K_DRF_RECORD_VER;
        //drf->record_type_id;
        drf->device_id        = R7K_DEVID_7KCENTER;
        drf->reserved0        = 0;
        drf->sys_enumerator   = R7K_DRF_SYS_ENUM_400KHZ;
        drf->reserved1        = 0;
        drf->flags            = 0x1;
        drf->reserved2        = 0;
        drf->reserved3        = 0;
        drf->total_frag_recs  = 0;
        drf->frag_number      = 0;
    }
}
// End function r7k_drf_init


// DRF container API
/// @fn r7k_drf_container_t * r7k_drfcon_new(uint32_t size)
/// @brief create new data record frame (DRF) container. DRF container
/// may contain multiple frames, and has an API for enumeration, as well as
/// for reading like a file.
/// @param[in] size buffer size
/// @return new drf container reference on success, NULL otherwise
r7k_drf_container_t *r7k_drfcon_new(uint32_t size)
{
    r7k_drf_container_t *self = (r7k_drf_container_t *)malloc(sizeof(r7k_drf_container_t));
    if (self) {
        memset(self,0,sizeof(r7k_drf_container_t));
        self->size=size;
        self->record_count=0;
        self->drf_enum=0;
        self->ofs_sz=R7K_DRFC_RECORD_INC;
        self->ofs_count=0;
        self->ofs_list = (uint32_t *)malloc(R7K_DRFC_RECORD_INC*sizeof(uint32_t));
        memset( self->ofs_list,0,R7K_DRFC_RECORD_INC);

        self->data = (byte *)malloc(size*sizeof(byte));
        memset(self->data,0,size);
        if (NULL != self->data) {
            self->p_write = self->data;
            self->p_read = self->data;
        }else{
            free(self);
            self=NULL;
        }
    }
    return self;
}
// End function r7k_drfcon_new


/// @fn void r7k_drfcon_destroy(r7k_drf_container_t ** pself)
/// @brief release data record frame container resources.
/// @param[in] pself pointer to instance reference
/// @return none
void r7k_drfcon_destroy(r7k_drf_container_t **pself)
{
    if (pself) {
        r7k_drf_container_t *self = *pself;
        if(self){
            if(NULL != self->data){
                free(self->data);
            }
            if (NULL != self->ofs_list) {
                free(self->ofs_list);
            }
            free(self);
            *pself=NULL;
        }
    }
}
// End function r7k_drfcon_destroy


/// @fn void r7k_drfcon_show(r7k_drf_container_t * self, _Bool verbose, uint16_t indent)
/// @brief output data record frame (DRF)  container parameter summary to stderr.
/// @param[in] self DRF container reference
/// @param[in] verbose use verbose output
/// @param[in] indent output indent spaces
/// @return none
void r7k_drfcon_show(r7k_drf_container_t *self, bool verbose, uint16_t indent)
{
    if (NULL != self) {
        fprintf(stderr,"%*s[self         %10p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[size         %10u]\n",indent,(indent>0?" ":""), self->size);
        fprintf(stderr,"%*s[record_count %10u]\n",indent,(indent>0?" ":""), self->record_count);
        fprintf(stderr,"%*s[data         %10p]\n",indent,(indent>0?" ":""), self->data);
        fprintf(stderr,"%*s[p_read       %10p]\n",indent,(indent>0?" ":""), self->p_read);
        fprintf(stderr,"%*s[p_write      %10p]\n",indent,(indent>0?" ":""), self->p_write);
        fprintf(stderr,"%*s[ofs_list     %10p]\n",indent,(indent>0?" ":""), self->ofs_list);
        fprintf(stderr,"%*s[ofs_sz       %10u]\n",indent,(indent>0?" ":""), self->ofs_sz);
        fprintf(stderr,"%*s[ofs_count    %10u]\n",indent,(indent>0?" ":""), self->ofs_count);
        fprintf(stderr,"%*s[drf_enum     %10u]\n",indent,(indent>0?" ":""), self->drf_enum);
        if (verbose) {
            for (uint32_t i=0; i<self->ofs_count; i++) {
                fprintf(stderr,"%*s[ofs[%02d]  %10u]\n",indent+3,(indent+3>0?" ":""),i, self->ofs_list[i]);
            }
        }
    }
}
// End function r7k_drfcon_show


// add one data record frame
/// @fn int r7k_drfcon_resize(r7k_drf_container_t * self, uint32_t new_size)
/// @brief resize a data record frame (DRF) container buffer.
/// @param[in] self DRF container reference
/// @param[in] new_size new size of buffer (bytes)
/// @return 0 on success, -1 otherwise
int r7k_drfcon_resize(r7k_drf_container_t *self, uint32_t new_size)
{
    int retval=-1;
    if (NULL!=self && new_size>0) {
        
        if (new_size > self->size) {
            // save read/write pointer offsets
            off_t rofs  = self->p_read-self->data;
            off_t wofs  = self->p_write-self->data;
            off_t owe   = self->data + self->size - self->p_write;
//            byte *odata    = self->data;
//            uint32_t osize = self->size;
            // increase size
            self->size    += (uint32_t)R7K_DRFC_SIZE_INC;
            self->data     = (byte *)realloc(self->data,(new_size*sizeof(byte)));
            
            // update read/write pointers
            self->p_read    = self->data+rofs;
            self->p_write   = self->data+wofs;
            
            // clear new memory
            memset(self->p_write+owe,0,R7K_DRFC_SIZE_INC);
            retval=0;
        }else{
            MERROR("shrink not implemented\n");
        }

    }else{
        MERROR("invalid argument\n");
    }
    return retval;
}
// End function r7k_drfcon_resize

// add one data record frame
/// @fn int r7k_drfcon_add(r7k_drf_container_t * self, byte * src, uint32_t len)
/// @brief add a new data record frame (DRF) to a DRF container.
/// @param[in] self DRF container reference
/// @param[in] src source data (containing parsed DRF)
/// @param[in] len size of new record to add
/// @return ME_OK on success, -1 otherwise and me_errno is set
int r7k_drfcon_add(r7k_drf_container_t *self, byte *src, uint32_t len)
{
    me_errno=ME_OK;
    
    int retval=-1;
    if (NULL!=self && NULL!=src) {

        if ( len <= r7k_drfcon_space(self) ) {
            
            // save record offset
            uint32_t record_ofs = self->p_write-self->data;
            
            // add to offset table if needed
            if ( (self->ofs_count!=0) && (self->ofs_count%self->ofs_sz) == 0) {
                uint32_t *mp = NULL;
                if( (mp = (uint32_t *)realloc(self->ofs_list, (size_t)(R7K_DRFC_RECORD_INC+self->ofs_sz)*sizeof(uint32_t) ))!=NULL){
                    self->ofs_list = mp;
                    self->ofs_sz+=R7K_DRFC_RECORD_INC;
                    memset( &self->ofs_list[self->ofs_count],0,R7K_DRFC_RECORD_INC*sizeof(uint32_t));
                }else{
                    MERROR("record offset realloc failed\n");
                    me_errno = ME_ENOMEM;
                }
            }
            
            if (me_errno==ME_OK) {
                // add record
                memcpy(self->p_write,src,len);
                
                // update count, pointers
                self->p_write += len;
                
                // add record offset entry
                self->ofs_list[self->ofs_count] = record_ofs;
                self->ofs_count++;
                self->record_count++;
                
                retval=0;
            }
        }else{
            MMDEBUG(R7K,"no space in container[%u/%u]\n",len,r7k_drfcon_space(self));
            me_errno = ME_ENOSPACE;
        }
    }else{
        MERROR("invalid argument\n");
    }
    return retval;
}
// End function r7k_drfcon_add


/// @fn int r7k_drfcon_flush(r7k_drf_container_t * self)
/// @brief clear data record frame (DRF) container buffer.
/// @param[in] self DRF reference
/// @return 0 on success, -1 otherwise
int r7k_drfcon_flush(r7k_drf_container_t *self)
{
    int retval=-1;
    if (NULL!=self && NULL!=self->data ) {
        if (r7k_drfcon_length(self)>0) {
            memset(self->data,0,self->size);
            self->p_read=self->data;
            self->p_write=self->data;
            if (NULL!=self->ofs_list && self->ofs_count>0) {
                memset(self->ofs_list,0,self->ofs_sz*sizeof(uint32_t));
                self->ofs_count=0;
            }
            self->record_count=0;
        }
        retval=0;
    }else{
        MERROR("invalid argument]\n");
    }
    return retval;
}
// End function r7k_drfcon_flush


/// @fn int r7k_drfcon_seek(r7k_drf_container_t * self, uint32_t ofs)
/// @brief set data record frame (DRF) container output (read) pointer offset.
/// @param[in] self DRF container reference
/// @param[in] ofs new offset
/// @return new offset on success (>=0), -1 otherwise
int r7k_drfcon_seek(r7k_drf_container_t *self, uint32_t ofs)
{
    int retval=-1;
    if (NULL!=self && NULL!=self->data && ofs<self->size && (self->data+ofs)<=self->p_write) {
       MMDEBUG(R7K,"sz[%u] ofs[%u]\n",self->size,ofs);
        self->p_read = self->data+ofs;
        retval = 0;
    }
    return retval;
}
// End function r7k_drfcon_seek


/// @fn uint32_t r7k_drfcon_tell(r7k_drf_container_t * self)
/// @brief return current output (read) pointer offset.
/// @param[in] self DRF container reference
/// @return output pointer offset (>=0) on success, -1 otherwise
uint32_t r7k_drfcon_tell(r7k_drf_container_t *self)
{
    uint32_t retval=0;
    if (NULL!=self && NULL!=self->data) {
        retval = self->p_read - self->data;
    }
    return retval;
}
// End function r7k_drfcon_tell


/// @fn uint32_t r7k_drfcon_read(r7k_drf_container_t * self, byte * dest, uint32_t len)
/// @brief read bytes from the data record frame (DRF) container.
/// @param[in] self DRF container reference
/// @param[in] dest buffer to read data into
/// @param[in] len number of bytes to read
/// @return number of bytes read (>=0) on success, -1 otherwise
uint32_t r7k_drfcon_read(r7k_drf_container_t *self, byte *dest, uint32_t len)
{
    uint32_t retval=0;
    
    if (NULL!=self && NULL!=self->data && len!=0 ) {
        uint32_t read_len = 0;
        
        if (len<r7k_drfcon_pending(self)) {
            read_len=len;
        }else{
            read_len = r7k_drfcon_pending(self);
        }
        if (read_len>0) {
            memcpy(dest,self->p_read,read_len);
            self->p_read += read_len;
            if (self->p_read > self->p_write) {
                MMDEBUG(DRFCON,"pread>pwrite\n");
                self->p_read = self->p_write;
            }else if (self->p_read > (self->data+self->size)) {
                MMDEBUG(DRFCON,"pread>data+size\n");
                self->p_read = self->data+self->size;
            }
            retval = read_len;
        }else{
//           MMDEBUG(R7K,"read_len <= 0 [%u]\n",read_len);
//            r7k_drfcon_show(self,false,5);
        }
    }else{
        MERROR("invalid argument\n");
    }
    return retval;
}
// End function r7k_drfcon_read


/// @fn uint32_t r7k_drfcon_size(r7k_drf_container_t * self)
/// @brief return total capacity (bytes) of data record frame (DRF) container.
/// @param[in] self DRF container reference
/// @return number of bytes (>=0) on success, -1 otherwise
uint32_t r7k_drfcon_size(r7k_drf_container_t *self)
{
    uint32_t retval=-1;
    if (NULL!=self){
        retval = self->size;
    }
    return retval;
}
// End function r7k_drfcon_size


/// @fn uint32_t r7k_drfcon_length(r7k_drf_container_t * self)
/// @brief total number of bytes currently in data record frame (DRF) container.
/// @param[in] self DRF container reference
/// @return number of bytes (>=0) on success, -1 otherwise
uint32_t r7k_drfcon_length(r7k_drf_container_t *self)
{
    uint32_t retval=-1;
    if (NULL!=self){
        retval = self->p_write - self->data;
    }
    return retval;
}
// End function r7k_drfcon_length


/// @fn uint32_t r7k_drfcon_pending(r7k_drf_container_t * self)
/// @brief return number of unread bytes in data record frame (DRF) container.
/// @param[in] self DRF container reference
/// @return number of bytes (>=0) on success, -1 otherwise
uint32_t r7k_drfcon_pending(r7k_drf_container_t *self)
{
    uint32_t retval=0;
    if (NULL!=self){
        retval = self->p_write - self->p_read;
    }
   return retval;
}
// End function r7k_drfcon_pending


/// @fn uint32_t r7k_drfcon_space(r7k_drf_container_t * self)
/// @brief return amount of space available (bytes) in data record frame (DRF) container.
/// @param[in] self DRF container reference
/// @return number of bytes (>=0) on success, -1 otherwise
uint32_t r7k_drfcon_space(r7k_drf_container_t *self)
{
    uint32_t retval=-1;
    if (NULL!=self){
        retval = self->data + self->size - self->p_write;
    }
    return retval;
}
// End function r7k_drfcon_space


/// @fn uint32_t r7k_drfcon_frames(r7k_drf_container_t * self)
/// @brief number of data record frames (DRFs) in container.
/// @param[in] self DRF container reference
/// @return number of frames (>=0) on success, -1 otherwise
uint32_t r7k_drfcon_frames(r7k_drf_container_t *self)
{
    uint32_t retval=-1;
    if (NULL!=self){
        retval = self->record_count;
    }
    return retval;
}
// End function r7k_drfcon_frames


/// @fn int r7k_drfcon_bytes(r7k_drf_container_t * self, uint32_t ofs, byte * dest, uint32_t len)
/// @brief copy bytes from a data record frame (DRF) container to a specified buffer.
/// @param[in] self DRF container reference
/// @param[in] ofs starting DRF offset
/// @param[in] dest destination data buffer
/// @param[in] len number of bytes to copy
/// @return TBD
int r7k_drfcon_bytes(r7k_drf_container_t *self, uint32_t ofs, byte *dest, uint32_t len)
{
    int retval=-1;
    if (NULL!=self &&
        NULL!=self->data &&
        len < (self->size - ofs) &&
        ofs < self->size ){
        
        memcpy(dest,self->data+ofs,len);
        retval = 0;
    }
    return retval;
}
// End function r7k_drfcon_bytes


/// @fn r7k_drf_t * r7k_drfcon_enumerate(r7k_drf_container_t * self)
/// @brief return first data record frame (DRF) in container. subsequent calls
/// to r7k_drfcon_next() will return the next frame(s) in the container.
/// @param[in] self DRF container reference
/// @return first DRF reference on success, NULL when the last frame is reached
r7k_drf_t* r7k_drfcon_enumerate(r7k_drf_container_t *self)
{
    r7k_drf_t *retval = NULL;
    if (NULL!=self && self->record_count>0){
        self->drf_enum = 0;
        retval = (r7k_drf_t *)(self->data+self->ofs_list[self->drf_enum]);
        self->drf_enum++;
    }
    return retval;
}
// End function r7k_drfcon_enumerate


/// @fn r7k_drf_t * r7k_drfcon_next(r7k_drf_container_t * self)
/// @brief return next first data record frame (DRF) in container.
/// To begin enumeration, first call r7k_drfcon_enumerate();
/// @param[in] self DRF container reference
/// @return DRF reference on success, NULL when no frames remain
r7k_drf_t* r7k_drfcon_next(r7k_drf_container_t *self)
{
    r7k_drf_t *retval = NULL;
    if (NULL!=self){
        
        if (self->drf_enum < self->ofs_count) {
            if (self->ofs_list[self->drf_enum] < self->size) {
                retval = (r7k_drf_t *)(self->data+self->ofs_list[self->drf_enum]);
                self->drf_enum++;
            }
        }
    }
    return retval;
}
// End function r7k_drfcon_next


/// @fn r7k_msg_t * r7k_msg_new(uint32_t data_len)
/// @brief create new r7k protocol message structure.
/// r7k messages have DRF and NF elements, but must be explicitely
/// serialized before sending to 7k center.
/// currently only used in mbtrn-server.
/// @param[in] data_len number of message data bytes
/// @return new message reference on success, NULL otherwise.
r7k_msg_t *r7k_msg_new(uint32_t data_len)
{

    r7k_msg_t *self = (r7k_msg_t *)malloc(sizeof(r7k_msg_t));
    if (NULL != self) {
        memset(self,0,sizeof(r7k_msg_t));
        self->nf = r7k_nf_new();
        self->drf = r7k_drf_new();
        self->msg_len = sizeof(r7k_nf_headers_t);
        self->msg_len += data_len;
        self->msg_len += sizeof(r7k_checksum_t);
        
        self->data_size = data_len;
        if (data_len>0) {
            self->data=(byte *)malloc(data_len*sizeof(byte));
            memset(self->data,0,data_len*sizeof(byte));
        }else{
            self->data=NULL;
        }
        r7k_update_time(&self->drf->_7ktime);
    }
    return self;
}
// End function r7k_msg_new


/// @fn void r7k_msg_destroy(r7k_msg_t ** pself)
/// @brief release message structure resources.
/// @param[in] pself pointer to message reference
/// @return none
void r7k_msg_destroy(r7k_msg_t **pself)
{
    if (pself) {
        r7k_msg_t *self = *pself;
        if (self) {
            if (NULL != self->nf) {
                free(self->nf);
            }
            if (NULL != self->drf) {
                free(self->drf);
            }
            if (NULL != self->data) {
                free(self->data);
            }
            free(self);
        }
        *pself=NULL;
    }
}
// End function r7k_msg_destroy



/// @fn void r7k_msg_show(r7k_msg_t * self, _Bool verbose, uint16_t indent)
/// @brief output r7k message parameter summary to stderr.
/// @param[in] self r7k message reference
/// @param[in] verbose use verbose output
/// @param[in] indent output indent spaces
/// @return none
void r7k_msg_show(r7k_msg_t *self, bool verbose, uint16_t indent)
{
    if (self) {
        fprintf(stderr,"%*s[self     %10p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[msg_len %10u]\n",indent,(indent>0?" ":""), self->msg_len);
        fprintf(stderr,"%*s[nf       %10p]\n",indent,(indent>0?" ":""), self->nf);
        if (verbose) {
            r7k_nf_show(self->nf,verbose,indent+3);
        }
        fprintf(stderr,"%*s[drf      %10p]\n",indent,(indent>0?" ":""), self->drf);
        if (verbose) {
            r7k_drf_show(self->drf,verbose,indent+3);
        }
        fprintf(stderr,"%*s[data_size %10u]\n",indent,(indent>0?" ":""), self->data_size);
        fprintf(stderr,"%*s[data       %10p]\n",indent,(indent>0?" ":""), self->data);
        if (verbose) {
            r7k_hex_show(self->data,self->data_size,16,true,indent+3);
        }

        fprintf(stderr,"%*s[checksum 0x%08u]\n",indent,(indent>0?" ":""), self->checksum);
    }
}
// End function r7k_msg_show


/// @fn uint32_t r7k_msg_set_checksum(r7k_msg_t * self)
/// @brief set the checksum for an r7k message structure.
/// @param[in] self r7k message reference
/// @return previous checksum value.
uint32_t r7k_msg_set_checksum(r7k_msg_t *self)
{
    uint32_t cs_save=0;
    if (NULL!=self) {
        cs_save = self->checksum;
        self->checksum=0;
        // compute checksum over
        // DRF, RTH, record data, optional data
        byte *bp = (byte *)self->drf;
        for (uint32_t i=0; i<(sizeof(r7k_drf_t)); i++) {
            self->checksum += *(bp+i);
        }
        if (self->data_size>0) {
            bp = (byte *)self->data;
            for (uint32_t i=0; i<self->data_size; i++) {
                self->checksum += *(bp+i);
            }
        }
    }
    return cs_save;
}
// End function r7k_msg_set_checksum


/// @fn byte * r7k_msg_serialize(r7k_msg_t * self)
/// @brief serialize r7k message into new network frame buffer.
/// caller must release frame buffer resources.
/// @param[in] self r7k message reference
/// @return new network frame buffer on success, NULL otherwise
byte *r7k_msg_serialize(r7k_msg_t *self)
{
    byte *buf = NULL;
    if ( (NULL!=self)
        &&
        NULL != self->nf
        &&
        NULL != self->drf
        &&
        NULL != self->data
        &&
        self->data_size>0
        &&
        self->msg_len>=(sizeof(r7k_empty_nf_t)+self->data_size)) {

        buf = (byte *)malloc(self->msg_len);
        
        if (buf) {
            byte *pnf  = buf;
            byte *pdrf = pnf+sizeof(r7k_nf_t);
            byte *pdata = pdrf+sizeof(r7k_drf_t);
            byte *pchk = pdata+self->data_size;
            
            memcpy(pnf,self->nf,sizeof(r7k_nf_t));
            memcpy(pdrf,self->drf,sizeof(r7k_drf_t));
            memcpy(pdata,self->data,self->data_size);
            memcpy(pchk,&self->checksum,sizeof(r7k_checksum_t));
        }
    }else{
        MERROR("invalid argument\n");
    }
                   return buf;
}
// End function r7k_msg_serialize


/// @fn int r7k_msg_receive(iow_socket_t * s, r7k_msg_t ** dest, uint32_t timeout_msec)
/// @brief receive network frame from 7k center into r7k message structure.
/// @param[in] s socket reference
/// @param[in] dest pointer to r7k message reference to hold message
/// @param[in] timeout_msec read timeout
/// @return number of bytes received on success, -1 otherwise.
int r7k_msg_receive(iow_socket_t *s, r7k_msg_t **dest, uint32_t timeout_msec)
{
    int retval=-1;
    int64_t nbytes=0;
    if (NULL != s && s->status==SS_CONNECTED) {
        
        // read nf, drf headers
        int64_t header_len = sizeof(r7k_nf_headers_t);
        byte headers[header_len];
        int64_t total_len=0;
        
        memset(headers,0,header_len);
        if ( (nbytes=iow_read_tmout(s,headers,header_len,timeout_msec)) == header_len) {
            total_len+=nbytes;
           MMDEBUG(R7K,"read headers [%"PRId64"/%"PRId64"]\n",nbytes,header_len);
            // get frame content
            r7k_nf_t *nf = (r7k_nf_t *)(headers);
            r7k_drf_t *drf = (r7k_drf_t *)(headers+sizeof(r7k_nf_t));
			// get size of data (RTH, RD, OD, checksum)
            uint32_t read_len = drf->size - sizeof(r7k_drf_t);
            uint32_t data_len = read_len-sizeof(r7k_checksum_t);
//           MMDEBUG(R7K,"ACK nf:\n");
//            r7k_nf_show(nf,true,5);
//           MMDEBUG(R7K,"ACK drf:\n");
//            r7k_drf_show(drf,true,5);
           MMDEBUG(R7K,"data_len[%u] read_len[%u]\n",data_len,read_len);
            // read rth/rd/od [if any], checksum
            if (read_len>0) {
                byte data[read_len];
                memset(data,0,read_len);
                if ( (nbytes=iow_read_tmout(s,data,read_len,timeout_msec)) == read_len) {
                    total_len+=nbytes;
                   MMDEBUG(R7K,"read data [%"PRId64"/%d] -> %p\n",nbytes,read_len,dest);
                    // TODO: validate content
                    // create message
                    r7k_msg_t *m = r7k_msg_new(data_len);
                    if (NULL!=m) {
                        memcpy(m->nf,nf,sizeof(r7k_nf_t));
                        memcpy(m->drf,drf,sizeof(r7k_drf_t));
                        memcpy(m->data,data,data_len);
                        memcpy(&m->checksum,(data+data_len),sizeof(r7k_checksum_t));
//                       MMDEBUG(R7K,"dest msg:\n");
//                        r7k_msg_show(m,true,6);
                        *dest=m;
                    }else{
                        MERROR("msg_new failed\n");
                    }
                }else{
                   MMDEBUG(R7K,"incomplete data read nbytes[%"PRId64"] data_len[%u]\n",nbytes,data_len);
                }
            }else{
               MMDEBUG(R7K,"read_len <= 0 nbytes[%"PRId64"] read_len[%u]\n",nbytes,read_len);
            }
        }else{
           MMDEBUG(R7K,"incomplete header read? nbytes[%"PRId64"] header_len[%"PRId64"]\n",nbytes,header_len);
        }
    }else{
        MINFO("invalid socket or status s[%p]\n",s);
    }
        
    return retval;
}
// End function r7k_msg_receive



/// @fn int r7k_msg_send(iow_socket_t * s, r7k_msg_t * self)
/// @brief serialize and send an r7k message to 7k center.
/// @param[in] s socket reference
/// @param[in] self r7k message structure
/// @return number of bytes sent (>=0) on success, -1 otherwise
int r7k_msg_send(iow_socket_t *s, r7k_msg_t *self)
{
    int retval=-1;
    if ( (NULL != self) && (NULL!=s)) {
        
        byte *buf = r7k_msg_serialize(self);
//       MMDEBUG(R7K,"SEND nf:\n");
//        r7k_nf_show((r7k_nf_t *)buf,true,4);
        int64_t status=0;
        if( (status=iow_send(s,buf,self->msg_len))>0){
            retval=0;
        }else{
            MERROR("send failed [%"PRId64"] [%d/%s]\n",status,errno,strerror(errno));
        }

        free(buf);
    }else{
        MINFO("invalid socket or message\n");
   }
    return retval;
}
// End function r7k_msg_send



/// @fn int r7k_test()
/// @brief r7k unit test(s).
/// currently subscribes to test server (exercising most of the r7k API).
/// @return 0 on success, -1 otherwise
int r7k_test()
{
    int retval=-1;
   MMDEBUG(R7K,"entering...\n");
    uint32_t sub_recs[2]={1000,2000};
   MMDEBUG(R7K,"create/connect socket...\n");
    iow_socket_t *s = iow_socket_new("localhost",R7K_7KCENTER_PORT,ST_TCP);
    iow_connect(s);
   MMDEBUG(R7K,"subscribing...\n");
    retval = r7k_subscribe(s,sub_recs,2);
   MMDEBUG(R7K,"releasing resources...\n");
    iow_socket_destroy(&s);
    
//    byte random[50];
//    for (int i=0; i<50; i++) {
//        random[i]=i%256+20;
//    }
//   MMDEBUG(R7K,"hex_show 30/9/f/5\n");
//    r7k_hex_show(random,30,9,false,5);
//   MMDEBUG(R7K,"hex_show 30/7/t/5\n");
//    r7k_hex_show(random,30,7,true,5);
//   MMDEBUG(R7K,"hex_show 30/10/t,5\n");
//    r7k_hex_show(random,30,10,true,5);
    return retval;
}
// End function r7k_test


