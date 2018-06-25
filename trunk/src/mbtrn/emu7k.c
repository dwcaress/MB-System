///
/// @file emu7k.c
/// @authors k. Headley
/// @date 01 jan 2018
 
/// 7k Center emulation
/// Reads MB data from a file and writes
/// it to a socket (e.g. emulates reson 7k center source)

/////////////////////////
// Terms of use 
/////////////////////////
//
// Copyright Information
//
// Copyright 2000-2018 MBARI
// Monterey Bay Aquarium Research Institute, all rights reserved.
// 
// Terms of Use
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version. You can access the GPLv3 license at
// http://www.gnu.org/licenses/gpl-3.0.html
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details
// (http://www.gnu.org/licenses/gpl-3.0.html)
// 
// MBARI provides the documentation and software code "as is", with no warranty,
// express or implied, as to the software, title, non-infringement of third party 
// rights, merchantability, or fitness for any particular purpose, the accuracy of
// the code, or the performance or results which you may obtain from its use. You 
// assume the entire risk associated with use of the code, and you agree to be 
// responsible for the entire cost of repair or servicing of the program with 
// which you are using the code.
// 
// In no event shall MBARI be liable for any damages, whether general, special,
// incidental or consequential damages, arising out of your use of the software, 
// including, but not limited to, the loss or corruption of your data or damages 
// of any kind resulting from use of the software, any prohibited use, or your 
// inability to use the software. You agree to defend, indemnify and hold harmless
// MBARI and its officers, directors, and employees against any claim, loss, 
// liability or expense, including attorneys' fees, resulting from loss of or 
// damage to property or the injury to or death of any person arising out of the 
// use of the software.
// 
// The MBARI software is provided without obligation on the part of the 
// Monterey Bay Aquarium Research Institute to assist in its use, correction, 
// modification, or enhancement.
// 
// MBARI assumes no responsibility or liability for any third party and/or 
// commercial software required for the database or applications. Licensee agrees 
// to obtain and maintain valid licenses for any additional third party software 
// required.

/////////////////////////
// Headers 
/////////////////////////

/////////////////////////
// Macros
/////////////////////////
/*
// These macros should only be defined for 
// application main files rather than general C files
//
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

// TODO: clean up server porting
#if defined(__unix__) || defined(__APPLE__)
#include <sys/poll.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#endif
#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include "emu7k.h"
#include "mdebug.h"


/////////////////////////
// Declarations 
/////////////////////////

static bool g_interrupt=false;
static int g_verbose=0;

static void s_show_help();
static void s_parse_args(int argc, char **argv, app_cfg_t *cfg);
static int s_server_handle_request(emu7k_t *svr, byte *req, int rlen, int client_fd);
static void *s_server_publish(void *arg);


#define S_PER_M ((double)60.0)
#define S_PER_H ((double)S_PER_M*60.0)
#define S_PER_D ((double)S_PER_H*24.0)
#define S_PER_Y ((double)S_PER_D*365.0)

#define R7KTIME2D(r7kt) ((double)r7kt.seconds+r7kt.year*S_PER_Y + r7kt.day*S_PER_D + r7kt.hours*S_PER_H + r7kt.minutes*S_PER_M)

#define TDIFF(a,b) (b-a)

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////

/////////////////////////
// Function Definitions
/////////////////////////

/// @fn emu7k_client_t * emu7k_client_new(iow_socket_t *s, uint32_t nsubs, int32_t *subs)
/// @brief create new emu7k test client
/// @param[in] s socket reference
/// @param[in] nsubs number of subscriptions
/// @param[in] subs subscription list (array)
/// @return new client reference
emu7k_client_t *emu7k_client_new(int fd, uint32_t nsubs, int32_t *subs)
{
    emu7k_client_t *self = (emu7k_client_t *)malloc(sizeof(emu7k_client_t));
    if (self) {
        self->fd=fd;
        // self->sub_count=(nsubs<0?0:nsubs); //nsubs always >= 0
        self->sub_list=NULL;
        if (self->sub_count > 0) {
            self->sub_list = (int32_t *)malloc(self->sub_count * sizeof(int32_t));
            memcpy(self->sub_list,subs,self->sub_count * sizeof(int32_t));
        }
    }
    return self;
}

/// @fn void emu7k_client_destroy(emu7k_client_t **pself)
/// @brief release client resources.
/// @param[in] pself pointer to instance reference
/// @return none
void emu7k_client_destroy(emu7k_client_t **pself)
{
    if (pself) {
        emu7k_client_t *self = *(pself);
        if (self) {
            if (NULL != self->sock_if) {
                iow_socket_destroy(&self->sock_if);
            }
            if (NULL != self->sub_list) {
                free(self->sub_list);
            }
            free(self);
        }
        *pself =  NULL;
    }
   return;
}

/// @fn emu7k_t * emu7k_new(iow_socket_t * s, iow_file_t * mb_data)
/// @brief create new emu7k test server - emulate reson 7k center (not fully implemented).
/// @param[in] s socket reference
/// @param[in] mb_data reson data file (optional)
/// @return new server reference
emu7k_t *emu7k_new(iow_socket_t *s, iow_file_t *mb_data)
{
    emu7k_t *self = (emu7k_t *)malloc(sizeof(emu7k_t));
    if (self) {
        memset(self,0,sizeof(emu7k_t));
        self->in_file = mb_data;
        self->auto_free = true;
        self->sock_if=s;
        self->stop=false;
        self->t=iow_thread_new();
        self->w=iow_thread_new();
        self->max_clients=16;
        self->client_count=0;
        self->client_list = mlist_new();
    }
    return self;
}
// End function emu7k_new


/// @fn void emu7k_destroy(emu7k_t ** pself)
/// @brief release server resources.
/// @param[in] pself pointer to instance reference
/// @return none
void emu7k_destroy(emu7k_t **pself)
{
    if (pself) {
        emu7k_t *self = *(pself);
        if (self) {
            if (self->auto_free) {
                iow_socket_destroy(&self->sock_if);
                iow_file_destroy(&self->in_file);
                iow_thread_destroy(&self->t);
                iow_thread_destroy(&self->w);
                
                emu7k_client_t *client = (emu7k_client_t *)mlist_first(self->client_list);
                while (NULL!=client) {
                    emu7k_client_destroy(&client);
                    client=NULL;
                    client = (emu7k_client_t *)mlist_next(self->client_list);
                }

                mlist_destroy(&self->client_list);
            }
            free(self);
        }
        *pself =  NULL;
    }
    return;
}
// End function emu7k_destroy


/// @fn void emu7k_rec_show(emu7k_t *self, bool verbose, uint16_t indent)
/// @brief emu7k statistics parameter summary to stderr.
/// @param[in] self emu7k stats reference
/// @param[in] verbose use verbose output
/// @param[in] indent output indent spaces
/// @return none
void emu7k_rec_show(emu7k_record_t *self, bool verbose, uint16_t indent)
{
    if (self) {
        if(verbose){
            fprintf(stderr,"%*s[self     %15p]\n",indent,(indent>0?" ":""), self);
            fprintf(stderr,"%*s[header   %15p]\n",indent,(indent>0?" ":""), self->header);
            fprintf(stderr,"%*s[data     %15p]\n",indent,(indent>0?" ":""), self->data);
            fprintf(stderr,"%*s[data_len %15"PRId64"]\n",indent,(indent>0?" ":""), self->data_len);
        }

        fprintf(stderr,"%*s[rtype    %15d]\n",indent,(indent>0?" ":""), self->rtype);
        fprintf(stderr,"%*s[time     %15.3lf]\n",indent,(indent>0?" ":""), self->time);
        fprintf(stderr,"%*s[size     %15"PRId64"]\n",indent,(indent>0?" ":""), (self->tail-self->head));
        fprintf(stderr,"%*s[head     %15"PRId64"]\n",indent,(indent>0?" ":""), self->head);
        fprintf(stderr,"%*s[tail     %15"PRId64"]\n",indent,(indent>0?" ":""), self->tail);
    }
}
// End function emu7k_record_t

/// @fn void emu7k_stat_show(emu7k_t *self, bool verbose, uint16_t indent)
/// @brief emu7k statistics parameter summary to stderr.
/// @param[in] self emu7k stats reference
/// @param[in] verbose use verbose output
/// @param[in] indent output indent spaces
/// @return none
void emu7k_stat_show(emu7k_stat_t *self, bool verbose, uint16_t indent)
{
    if (self) {
        fprintf(stderr,"%*s[self       %10p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[uptime     %10"PRId64"]\n",indent,(indent>0?" ":""), (int64_t)(time(NULL)-self->start_time));
        fprintf(stderr,"%*s[con_total  %10"PRId64"]\n",indent,(indent>0?" ":""), self->con_total);
        fprintf(stderr,"%*s[con_active %10"PRId64"]\n",indent,(indent>0?" ":""), self->con_active);
        fprintf(stderr,"%*s[cyc_total  %10"PRId64"]\n",indent,(indent>0?" ":""), self->cyc_total);
        fprintf(stderr,"%*s[rec_total  %10"PRId64"]\n",indent,(indent>0?" ":""), self->rec_total);
        fprintf(stderr,"%*s[pub_total  %10"PRId64"]\n",indent,(indent>0?" ":""), self->pub_total);
        fprintf(stderr,"%*s[rec_cycle  %10"PRId64"]\n",indent,(indent>0?" ":""), self->rec_cycle);
        fprintf(stderr,"%*s[pub_cycle  %10"PRId64"]\n",indent,(indent>0?" ":""), self->pub_cycle);
    }
}
// End function emu7k_stat_show

/// @fn void emu7k_show(emu7k_t *self, bool verbose, uint16_t indent)
/// @brief emu7k parameter summary to stderr.
/// @param[in] self emu7k reference
/// @param[in] verbose use verbose output
/// @param[in] indent output indent spaces
/// @return none
void emu7k_show(emu7k_t *self, bool verbose, uint16_t indent)
{
    if (self) {
        fprintf(stderr,"%*s[self         %10p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[sock_if      %10p]\n",indent,(indent>0?" ":""), self->sock_if);
        fprintf(stderr,"%*s[in_file      %10p]\n",indent,(indent>0?" ":""), self->in_file);
        fprintf(stderr,"%*s[svr thread   %10p]\n",indent,(indent>0?" ":""), self->t);
        fprintf(stderr,"%*s[wrk thread   %10p]\n",indent,(indent>0?" ":""), self->w);
        fprintf(stderr,"%*s[max_clients  %10u]\n",indent,(indent>0?" ":""), self->max_clients);
        fprintf(stderr,"%*s[client_count %10u]\n",indent,(indent>0?" ":""), self->client_count);
        fprintf(stderr,"%*s[client_list  %10p]\n",indent,(indent>0?" ":""), self->client_list);
        fprintf(stderr,"%*s[auto_free    %10c]\n",indent,(indent>0?" ":""), (self->auto_free?'Y':'N'));
        fprintf(stderr,"%*s[stop         %10c]\n",indent,(indent>0?" ":""), (self->stop?'Y':'N'));
        fprintf(stderr,"%*s[stats        %10p]\n",indent,(indent>0?" ":""), &self->stats);
        fprintf(stderr,"%*s[cfg          %10p]\n",indent,(indent>0?" ":""), self->cfg);
    }
}
// End function emu7k_show


static int64_t read_s7k_rec(emu7k_record_t *dest, iow_file_t *src, int64_t ofs)
{
    int64_t retval=-1;
    int64_t rbytes=0;
    uint32_t req_len=R7K_DRF_BYTES;
    off_t file_end = iow_seek(src,0,IOW_END);
    
    if (NULL!=dest && NULL!=src && ofs<file_end && ofs>=0) {
        
        if (NULL == dest->header) {
            dest->header=(byte *)malloc(R7K_DRF_BYTES);
        }
        
        if(iow_seek(src,ofs,IOW_SET)>=0){
            
            off_t frame_head = iow_seek(src,0,IOW_CUR);

            if( (rbytes=iow_read(src,dest->header,req_len))==req_len){
             
                r7k_drf_t *drf = (r7k_drf_t *)dest->header;

                if (drf->protocol_version == R7K_DRF_PROTO_VER &&
                    drf->offset == R7K_DRF_BYTES-2*sizeof(uint16_t) &&
                    drf->size > R7K_DRF_BYTES &&
                    drf->sync_pattern == R7K_DRF_SYNC_PATTERN
                    ) {
                    
                    dest->header = (byte *)realloc(dest->header, drf->size);
                    drf = (r7k_drf_t *)dest->header;

                    byte *pdata = dest->header+R7K_DRF_BYTES;
                    req_len = (drf->size-R7K_DRF_BYTES);
                    
                    if ((rbytes=iow_read(src,pdata,req_len))==req_len) {
                        dest->head     = frame_head;
                        dest->tail     = iow_seek(src,0,IOW_CUR);
                        dest->time     = R7KTIME2D(drf->_7ktime);
                        dest->data     = pdata;
                        dest->data_len = req_len;
                        dest->rtype    = drf->record_type_id;
                        retval         = drf->size;
                        
                        if(g_verbose>=4){
	                        emu7k_rec_show(dest,false,5);
//                        MMDEBUG(APP1,"type[%d]\n",dest->rtype);
//                        MMDEBUG(APP1,"time[%.3lf]\n",dest->time);
//                        MMDEBUG(APP1,"head[%"PRId64"]\n",dest->head);
//                        MMDEBUG(APP1,"tail[%"PRId64"]\n",dest->tail);
//                        MMDEBUG(APP1,"size[%"PRId64"/%"PRIu32"]\n",(dest->tail-dest->head),drf->size);
//                        r7k_drf_show(drf,false,5);
                        }
                    }else{
                        MERROR("data read failed read[%"PRId64"] req[%"PRId32"] [%d/%s]\n",rbytes,req_len,errno,strerror(errno));
                    }
                    
                }else{
                    MERROR("invalid header frame_head[0x%0X] nf_sz[%lu] drf_sz[%lu]\n",(unsigned int)frame_head,R7K_NF_BYTES,R7K_DRF_BYTES);
                    if(g_verbose>=1){
	                    r7k_drf_show(drf,false,5);
                    }
                }
            }else{
                MERROR("data read failed read[%"PRId64"] req[%"PRId32"] [%d/%s]\n",rbytes,req_len,errno,strerror(errno));
            }
        }else{
            MERROR("seek failed [%d/%s]\n",errno,strerror(errno));
        }
    }else{
        MERROR("invalid argument\n");
    }
    return retval;
}

/// @fn void * s_server_publish(void * arg)
/// @brief test server publishing thread function.
/// @param[in] arg void pointer to server reference
/// @return 0 on success, -1 otherwise
static void *s_server_publish(void *arg)
{
    emu7k_t *svr = (emu7k_t *)arg;
    
    if (NULL!=svr) {
        iow_socket_t  *s = svr->sock_if;
        bool stop_req = true;

        if (NULL != svr->in_file) {
            emu7k_record_t cur={0};
            emu7k_record_t nxt={0};
            
            if (read_s7k_rec(&cur,svr->in_file,0) > 0) {
                if (read_s7k_rec(&nxt,svr->in_file,cur.tail) > 0) {
                    stop_req=false;
                }else{
                    MERROR("init next failed\n");
                }
            }else{
                MERROR("init current failed\n");
            }
            
            struct timespec delay;
            struct timespec rem;
            int seq_number=0;
            bool delete_client=false;
            emu7k_stat_t *stats = &svr->stats;
            
            while (!stop_req && !svr->stop) {
                //MMDEBUG(APP1,"publisher running...clients[%"PRId32"]\n",mlist_size(svr->client_list));
                
                if (mlist_size(svr->client_list)>0) {
                    
                    emu7k_client_t *client = (emu7k_client_t *)mlist_first(svr->client_list);
                    while (NULL != client) {
                        delete_client=false;
                        for (int i=0; i<client->sub_count; i++) {
                            //MMDEBUG(APP1,"client sub type[%d] rtype[%d]\n",client->sub_list[i],cur.rtype);
                            if (cur.rtype == client->sub_list[i]) {

                                //MMDEBUG(APP1,"client has sub to type[%d]\n",cur.rtype);
                                
                                r7k_msg_t *msg = r7k_msg_new(cur.data_len);
                                memcpy(msg->drf,cur.header,R7K_DRF_BYTES);
                                memcpy(msg->data,cur.data,cur.data_len);
                                
                                msg->nf->tx_id       = r7k_txid();
                                msg->nf->seq_number  = seq_number++;
                                msg->nf->packet_size = msg->msg_len;
                                msg->nf->total_size  = msg->msg_len - sizeof(r7k_nf_t);
                                r7k_msg_set_checksum(msg);
                                
//                                MMDEBUG(APP1,"publishing rtype[%d] @ofs[%"PRId64"] to client[%d]:\n",cur.rtype,cur.head,client->fd);
                                
                                MMDEBUG(APP3," cli[%d]\n",client->fd);
                                if(g_verbose>=3){
                                    emu7k_rec_show(&cur,false,5);
//                                MMDEBUG(APP1,"type[%d]\n",cur.rtype);
//                                MMDEBUG(APP1,"time[%.3lf]\n",cur.time);
//                                MMDEBUG(APP1,"head[%"PRId64"]\n",cur.head);
//                                MMDEBUG(APP1,"tail[%"PRId64"]\n",cur.tail);
//                                MMDEBUG(APP1,"size[%"PRId64"/%"PRIu32"]\n",(cur.tail-cur.head),msg->drf->size);
                                    r7k_drf_show(msg->drf,false,5);
                                }

                                if(r7k_msg_send(client->sock_if,msg)==-1 && errno==EPIPE){
                                    delete_client=true;
                                }
                                stats->pub_total++;
                                stats->pub_cycle++;
                                r7k_msg_destroy(&msg);
                                if (delete_client) {
                                    MMDEBUG(APP1,"connection broken, deleting client %p fd[%d]\n",client,client->fd);
                                    mlist_remove(svr->client_list,client);
                                    emu7k_client_destroy(&client);
                                    MMDEBUG(APP3,"clients remaining[%"PRId32"]\n",(int)mlist_size(svr->client_list));
                                    client=NULL;
                                    stats->con_active--;
                                }
                                break;
                            }
                        }
                        client=(emu7k_client_t *)mlist_next(svr->client_list);
                    }
                    
                    double twait = TDIFF(cur.time,nxt.time);
                    twait = (twait<=0 ? ((double)svr->cfg->min_delay/1000.0) : twait);
                    MMDEBUG(APP4,"twait[%.3lf]\n",twait);
                    if (twait>0 && twait < 3) {
                        time_t lsec = (time_t)twait;
                        long lnsec = (1000000L*(twait-lsec));
                        delay.tv_sec=lsec;
                        delay.tv_nsec=lnsec;
                        MMDEBUG(APP5,"delaying %.3lf sec:nsec[%ld:%ld]\n",twait,delay.tv_sec,delay.tv_nsec);
                        while (nanosleep(&delay,&rem)<0) {
                            MMDEBUG(APP5,"sleep interrupted\n");
                            delay.tv_sec=rem.tv_sec;
                            delay.tv_nsec=rem.tv_nsec;
                        }
                    }
                    // release current record data
                    free(cur.header);
                    // move next to current
                    memcpy(&cur,&nxt,sizeof(emu7k_record_t));
                    // make next data NULL so it will be allocated
                    nxt.header=NULL;
                    off_t file_end = iow_seek(svr->in_file,0,IOW_END);
                    if (cur.tail>=file_end) {
                        stats->cyc_total++;
                        stats->rec_cycle=0;
                        stats->pub_cycle=0;
                        
                        MMDEBUG(APP2,"reached end of file fs[%lld] ofs[%"PRId64"]\n",file_end,cur.tail);
                        if (svr->cfg->restart) {
                            MMDEBUG(APP2,"restarting\n");
                            cur.tail=0;
                        }else{
                            MMDEBUG(APP2,"exiting\n");
                            stop_req=true;
                        }
                    }
                    // read next record
                    read_s7k_rec(&nxt,svr->in_file,cur.tail);
                    stats->rec_cycle++;
                    stats->rec_total++;
                    
                    if ( (svr->cfg->verbose>=2) &&
                        (svr->cfg->statn>0) &&
                        (stats->rec_total%svr->cfg->statn == 0) ) {
                        MMDEBUG(APP2,"stats\n");
                        emu7k_stat_show(stats,false,7);
                    }
                }else{
                    //MMDEBUG(APP1,"no clients\n");
                    sleep(1);
                }
            }
            if(svr->cfg->verbose>=1){
                MMDEBUG(APP1,"stats\n");
                emu7k_stat_show(stats,false,7);
            }

            // free pending records
            free(cur.header);
            free(nxt.header);
        }else{
            MERROR("NULL file[]\n");
            s->status=-1;
        }

        MMDEBUG(APP2,"publisher exiting sreq[%c] stop[%c]\n",(stop_req?'Y':'N'),(svr->stop?'Y':'N'));
        pthread_exit((void *)&s->status);
        
        return (void *)(&s->status);
    }else{
        MERROR("NULL server\n");
    }
    pthread_exit((void *)NULL);
    
    return (void *)NULL;
 
	
}
// End function s_server_main

/// @fn int s_server_handle_request(emu7k_t * svr, char * req, int client_fd)
/// @brief handle client request.
/// @param[in] svr server reference
/// @param[in] req request string
/// @param[in] client_fd client file descriptor/handle
/// @return 0 on success, -1 otherwise
static int s_server_handle_request(emu7k_t *svr, byte *req, int rlen, int client_fd)
{
    int retval=0;
	
    if (NULL!=req) {
        
        r7k_nf_headers_t *fh=(r7k_nf_headers_t *)req;
        r7k_nf_t *nf = &(fh->nf);
        r7k_drf_t *drf = &(fh->drf);
        r7k_rth_7500_rc_t *rth = (r7k_rth_7500_rc_t *)((byte *)drf+sizeof(r7k_drf_t));
        size_t hdr_len = sizeof(r7k_nf_headers_t)+sizeof(r7k_rth_7500_rc_t);

        if (strncmp((const char *)req,"STOP",4)==0) {
            MMDEBUG(APP1,"STOP received\n");
            send(client_fd,"ACK",strlen("ACK"),0);
            svr->stop=true;
        }else if(strncmp((const char *)req,"REQ",3)==0){
            MMDEBUG(APP1,"REQ received\n");
            send(client_fd,"ACK",strlen("ACK"),0);
        }else if(rlen>=hdr_len){
            MMDEBUG(APP1,"proto ver      [%d]\n",nf->protocol_version);
            MMDEBUG(APP1,"record_type_id [%u]\n",drf->record_type_id);

            if(nf->protocol_version == R7K_NF_PROTO_VER &&
               drf->record_type_id == R7K_RT_REMCON &&
               rth->remcon_id == R7K_RTID_SUB){
                
                // got 7k center subscription record
	            MMDEBUG(APP1,"7K SUB request received\n");
                // create, send SUB ACK message
                r7k_msg_t *msg = r7k_msg_new(sizeof(r7k_rth_7501_ack_t));
                r7k_rth_7501_ack_t *prth = (r7k_rth_7501_ack_t *)(msg->data);
                prth->ticket = 1;
                memcpy(prth->tracking_number, "ABCDEF0123456789",strlen("ABCDEF0123456789"));
                msg->drf->size           = DRF_SIZE(msg);
                msg->drf->record_type_id = R7K_RT_REMCON_ACK;
                msg->drf->device_id      = R7K_DEVID_7KCENTER;
                msg->nf->tx_id       = r7k_txid();
                msg->nf->seq_number  = 0;
                msg->nf->packet_size = msg->msg_len;
                msg->nf->total_size  = msg->msg_len - sizeof(r7k_nf_t);
                r7k_msg_set_checksum(msg);
                
                MMDEBUG(APP1,"sending SUB ACK:\n");
                if(svr->cfg->verbose>=1){
                    r7k_msg_show(msg,true,3);
                }
                iow_socket_t *s = iow_wrap_fd(client_fd);
                r7k_msg_send(s,msg);
                r7k_msg_destroy(&msg);
                
                // get sub request data
                byte *pdata = (req + hdr_len);
                uint32_t nsubs = *((uint32_t *)pdata);
                int32_t *subs = (int32_t *)(pdata+sizeof(uint32_t));

                // create client, add to list
                emu7k_client_t *cli = emu7k_client_new(client_fd, nsubs, subs);
                MMDEBUG(APP1,"adding client fd[%d] to list\n",client_fd);
                mlist_add(svr->client_list, cli);
                cli->sock_if=s;
            }
        }else{
            MERROR("ERR - unsupported request\n");
            retval=-1;
        }
    }else{
        MERROR("ERR - invalid/NULL request\n");
        retval=-1;
    }
    return retval;
}
// End function s_server_handle_request

/// @fn void * s_server_main(void * arg)
/// @brief test server thread function.
/// @param[in] arg void pointer to server reference
/// @return 0 on success, -1 otherwise
void *s_server_main(void *arg)
{
    emu7k_t *svr = (emu7k_t *)arg;
    
    char buf[ADDRSTR_BYTES]={0};
    struct timeval tv;
    fd_set master;
    fd_set read_fds;
    int fdmax;
    int stat=0;
    byte iobuf[256]; // buffer for client data
    int nbytes;
    struct sockaddr_storage client_addr={0};
    socklen_t addr_size=0;
    bool stop_req=false;
    emu7k_stat_t *stats = &svr->stats;
    int *retval=NULL;

    stats->start_time=time(NULL);
    
    int dat_fd=iow_open(svr->in_file,IOW_RONLY);
    if (dat_fd<=0) {
        MERROR("file open failed for %s [%d/%s]\n",svr->in_file->path,errno,strerror(errno));
        stop_req=true;
        g_interrupt=true;
    }
    iow_socket_t  *s = svr->sock_if;
    
    if ( (NULL!=svr) && (NULL!=s) && dat_fd>0) {
       	MMDEBUG(APP4,"starting worker thread\n");
        if(iow_thread_start(svr->w, s_server_publish, (void *)svr)!=0){
            MERROR("worker thread start failed\n");
            stop_req=true;
        }else{

            MMDEBUG(APP2,"server [%s] - starting\n",iow_addr2str(s,buf,ADDRSTR_BYTES));
            iow_listen(s);
            
            tv.tv_sec = 3;
            tv.tv_usec = 0;
            
            FD_ZERO(&read_fds);
            FD_ZERO(&master);
            FD_SET(s->fd,&master);
            fdmax = s->fd;
            
            bool do_close=true;
            while (!svr->stop && !stop_req) {
                read_fds = master;
                // MMINFO(APP1,"pending on select\n");
                if( (stat=select(fdmax+1, &read_fds, NULL, NULL, &tv)) != -1){
                    int newfd=-1;
                    for (int i=s->fd; i<=fdmax; i++) {
                        
                        if (FD_ISSET(i, &read_fds)){
                            // MMINFO(APP1,"readfs [%d/%d] selected\n",i,fdmax);
                            do_close=true;
                            if (i==s->fd) {
                                MMDEBUG(APP4,"server main listener [%d] got request\n",i);
                                
                                newfd = accept(s->fd, (struct sockaddr *)&client_addr, &addr_size);
                                if (newfd != -1) {
                                    MMDEBUG(APP4,"client connected on socket [%d]\n",newfd);
                                    FD_SET(newfd,&read_fds);
                                    if (newfd>fdmax) {
                                        fdmax=newfd;
                                    }
                                    stats->con_total++;
                                    stats->con_active++;

                                }else{
                                    // accept failed
                                    // MMINFO(APP1,"accept failed [%d/%s]\n",errno,strerror(errno));
                                }
                            }else{
                                do_close=false;
                                MMDEBUG(APP4,"server waiting for client data fd[%d]\n",i);
                                if (( nbytes = recv(i, iobuf, sizeof iobuf, 0)) <= 0) {
                                    MMDEBUG(APP4,"handle client data fd[%d] nbytes[%d]\n",i,nbytes);
                                    // got error or connection closed by client
                                    if (nbytes == 0) {
                                        // connection closed
                                        fprintf(stderr,"ERR - socket %d hung up\n", i);
                                    } else if(nbytes<0) {
                                        fprintf(stderr,"ERR - recv failed socket[%d] [%d/%s]\n",i,errno,strerror(errno));
                                    }
                                }else{
                                    MMDEBUG(APP4,"server received request on socket [%d] len[%d]\n",i,nbytes);
                                    s_server_handle_request(svr,iobuf,nbytes,i);
                                }
                                if (do_close) {
                                    close(i); // bye!
                                }
                                FD_CLR(i, &master); // remove from master set
                            }
                        }else{
                            //                       MMINFO(APP1,"readfs fd[%d/%d] ISSET:%s\n",i,fdmax,(FD_ISSET(i,&read_fds)?"TRUE":"FALSE"));
                        }
                    }
                }else{
                    // select failed
//                    MMINFO(APP1,"select failed stat[%d] [%d/%s]\n",stat,errno,strerror(errno));
                    tv.tv_sec = 3;
                    tv.tv_usec = 0;
                }
            }

            if(svr->cfg->verbose>=1){
                MMDEBUG(APP1,"stats\n");
                emu7k_stat_show(stats,false,7);
            }
        }
    }
    
    if (stop_req) {
        MMDEBUG(APP3,"Test server - interrupted - stop flag set\n");
        if (NULL!=s) {
            s->status=1;
        }
    }else{
        MMDEBUG(APP3,"Test server - normal exit\n");
        if (NULL!=s) {
        	s->status=0;
        }
    }
    
    retval=(NULL!=s?&s->status:NULL);
    
    pthread_exit((void *)retval);

    return (void *)retval;
}
// End function s_server_main


/// @fn int emu7k_start(emu7k_t * self)
/// @brief start test server in a thread.
/// @param[in] self server reference
/// @return 0 on success, -1 otherwise
int emu7k_start(emu7k_t *self)
{
    int retval=0;
    if (self) {
        self->stop=false;
        if(iow_thread_start(self->t, s_server_main, (void *)self)!=0){
            retval=-1;
        }else{
            sleep(1);
        }
    }
    return retval;
}
// End function emu7k_start


/// @fn int emu7k_stop(emu7k_t * self)
/// @brief stop server thread.
/// @param[in] self server reference
/// @return 0 on success, -1 otherwise
int emu7k_stop(emu7k_t *self)
{
    int retval=-1;
    if (NULL!=self) {
        MMDEBUG(APP2,"stopping server thread\n");
        self->stop=true;
        iow_thread_join(self->t);
        retval=0;
    }
    return retval;
}
// End function emu7k_stop

/// @fn void s_show_help()Safety Committee -- agenda items
/// @brief output user help message to stdout.
/// @return none
static void s_show_help()
{
    char help_message[] = "\nStream raw reson bytes to console\n";
    char usage_message[] = "\nstream7k [options]\n"
    "--verbose n   : verbose output level (n>=0)\n"
    "--host s      : host IP address or name\n"
    "--port n      : TCP/IP port\n"
    "--file s      : data file path (.s7k)\n"
    "--min-delay n : minimum publish delay (msec)\n"
    "--restart     : restart data when end of file is reached\n"
    "--no-restart  : stop when end of file is reached\n"
    "--statn n     : stop when end of file is reached\n"
    "\n";
    printf("%s",help_message);
    printf("%s",usage_message);
}
// End function s_show_help


/// @fn void parse_args(int argc, char ** argv, app_cfg_t * cfg)
/// @brief parse command line args, set application configuration.
/// @param[in] argc number of arguments
/// @param[in] argv array of command line arguments (strings)
/// @param[in] cfg application config structure
/// @return none
static void s_parse_args(int argc, char **argv, app_cfg_t *cfg)
{
    extern char WIN_DECLSPEC *optarg;
    int option_index;
    int c;
    bool help=false;
    
    static struct option options[] = {
        {"verbose", required_argument, NULL, 0},
        {"help", no_argument, NULL, 0},
        {"host", required_argument, NULL, 0},
        {"port", required_argument, NULL, 0},
        {"file", required_argument, NULL, 0},
        {"min-delay", required_argument, NULL, 0},
        {"statn", required_argument, NULL, 0},
        {"restart", no_argument, NULL, 0},
        {"no-restart", no_argument, NULL, 0},
        {NULL, 0, NULL, 0}};
    
    // process argument list
    while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1){
        switch (c) {
                // long options all return c=0
            case 0:
                // verbose 
                if (strcmp("verbose", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->verbose);
                }
                
                // help 
                else if (strcmp("help", options[option_index].name) == 0) {
                    help = true;
                }
                // file
                else if (strcmp("file", options[option_index].name) == 0) {
                    if (cfg->file_path) {
                        free(cfg->file_path);
                    }
                    cfg->file_path=strdup(optarg);
                }
                // host
                else if (strcmp("host", options[option_index].name) == 0) {
                    if (NULL!=cfg->host) {
                        free(cfg->host);
                    }
                    cfg->host=strdup(optarg);
                }
                // port
                else if (strcmp("port", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->port);
                }
                // min-delay
                else if (strcmp("min-delay", options[option_index].name) == 0) {
                    sscanf(optarg,"%u",&cfg->min_delay);
                }
                // statn
                else if (strcmp("statn", options[option_index].name) == 0) {
                    sscanf(optarg,"%u",&cfg->statn);
                }
                // restart
                else if (strcmp("restart", options[option_index].name) == 0) {
                    cfg->restart=true;
                }
                // no-restart
                else if (strcmp("no-restart", options[option_index].name) == 0) {
                    cfg->restart=false;
                }
                break;
            default:
                help=true;
                break;
        }
        if (help) {
            s_show_help();
            exit(0);
        }
    }// while
    
    if (cfg->verbose>0) {
        MDEBUG("verbose   [%d]\n",cfg->verbose);
        MDEBUG("host      [%s]\n",cfg->host);
        MDEBUG("port      [%d]\n",cfg->port);
        MDEBUG("file      [%s]\n",cfg->file_path);
        MDEBUG("restart   [%c]\n",(cfg->restart?'Y':'N'));
        MDEBUG("statn     [%d]\n",cfg->statn);
        MDEBUG("min-delay [%u]\n",cfg->min_delay);
    }
    
    g_verbose = cfg->verbose;
    // use MDI_ALL to globally set module debug output
    // may also set per module basis using module IDs
    // defined in mconfig.h:
    // APP1, APP2...APP5
    // valid level values are
    // MDL_UNSET,MDL_NONE
    // MDL_FATAL, MDL_ERROR, MDL_WARN
    // MDL_INFO, MDL_DEBUG
    mcfg_configure(NULL,0);
    mdb_set(MDI_ALL,MD_UNSET);
    mdb_set(IOW,MD_ERROR);
    switch (cfg->verbose) {
        case 0:
            mdb_set(APP1,MDL_NONE);
            mdb_set(APP2,MDL_NONE);
            mdb_set(APP3,MDL_NONE);
            mdb_set(APP4,MDL_NONE);
            mdb_set(APP5,MDL_NONE);
            break;
        case 1:
            mdb_set(APP1,MDL_DEBUG);
            break;
        case 2:
            mdb_set(APP1,MDL_DEBUG);
            mdb_set(APP2,MDL_DEBUG);
            break;
        case 3:
            mdb_set(APP1,MDL_DEBUG);
            mdb_set(APP2,MDL_DEBUG);
            mdb_set(APP3,MDL_DEBUG);
            break;
        case 4:
            mdb_set(APP1,MDL_DEBUG);
            mdb_set(APP2,MDL_DEBUG);
            mdb_set(APP3,MDL_DEBUG);
            mdb_set(APP4,MDL_DEBUG);
            break;
        case 5:
            mdb_set(APP1,MDL_DEBUG);
            mdb_set(APP2,MDL_DEBUG);
            mdb_set(APP3,MDL_DEBUG);
            mdb_set(APP4,MDL_DEBUG);
            mdb_set(APP5,MDL_DEBUG);
            break;
        default:
            mdb_set(APP1,MDL_ERROR);
            mdb_set(APP2,MDL_ERROR);
            mdb_set(APP3,MDL_ERROR);
            mdb_set(APP4,MDL_ERROR);
            mdb_set(APP5,MDL_ERROR);
            break;
    }
}
// End function parse_args

/// @fn void termination_handler (int signum)
/// @brief termination signal handler.
/// @param[in] signum signal number
/// @return none
void termination_handler (int signum)
{
    switch (signum) {
        case SIGINT:
        case SIGHUP:
        case SIGTERM:
            MMDEBUG(APP2,"received sig[%d]\n",signum);
            g_interrupt=true;
            break;
        default:
            MERROR("not handled[%d]\n",signum);
            break;
    }
}
// End function parse_args

/// @fn int main(int argc, char ** argv)
/// @brief frames7k main entry point.
/// subscribe to reson 7k center data streams, and output
/// parsed data record frames to stderr.
/// Use argument --cycles=x, x<=0  to stream indefinitely
/// @param[in] argc number of command line arguments
/// @param[in] argv array of command line arguments (strings)
/// @return 0 on success, -1 otherwise
int main(int argc, char **argv)
{
    // configure signal handling
    // for main thread
    struct sigaction saStruct;
    sigemptyset(&saStruct.sa_mask);
    saStruct.sa_flags = 0;
    saStruct.sa_handler = termination_handler;
    sigaction(SIGINT, &saStruct, NULL);
  
    // configure application
    app_cfg_t cfg = {
        VERBOSE_OUTPUT_DFL,
        NULL,
        strdup(EMU_HOST_DFL),
        EMU_PORT_DFL,
        MIN_DELAY_DFL_MSEC,
        RESTART_DFL,
        STATN_DFL_REC};

    // parse command line args
    s_parse_args(argc, argv, &cfg);

    // create/configure server socket
    iow_socket_t *svr_socket = iow_socket_new(cfg.host, R7K_7KCENTER_PORT, ST_TCP);
    // create/configure input file
    iow_file_t *svr_data = iow_file_new(cfg.file_path);

    // create/configure server
    emu7k_t *server = emu7k_new(svr_socket, svr_data);
    server->cfg = &cfg;
    
    // start server thread
    emu7k_start(server);
    
    // wait for input signal (SIGINT)
    while (!server->stop && !g_interrupt) {
        sleep(2);
    }
    // stop the server
    MMDEBUG(APP1,"stopping server...\n");
    emu7k_stop(server);
    MMDEBUG(APP4,"socket status [%d]\n",svr_socket->status);

    // release resources
    MMDEBUG(APP4,"releasing resources...\n");
    // server will release socket, file resources
    emu7k_destroy(&server);
    free(cfg.host);
    free(cfg.file_path);

    return 0;
}
// End function main



