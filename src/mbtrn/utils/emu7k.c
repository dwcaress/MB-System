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
// TODO: clean up server porting
//#if defined(__unix__) || defined(__APPLE__)
//#include <sys/poll.h>
//#include <netinet/in.h>
//#include <string.h>
//#include <errno.h>
//#include <signal.h>
//#endif
//#include <math.h>
//#include <getopt.h>
//#include <stdarg.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <unistd.h>
//#include <inttypes.h>
//#include "emu7k.h"
//#include "mdebug.h"

#include <getopt.h>
#include <stdarg.h>
#include "emu7k.h"
#include "mframe.h"
#include "mtime.h"
#include "merror.h"
#include "mxdebug.h"
#include "mxd_app.h"

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


#define EMU7K_NAME "emu7k"
#ifndef EMU7K_VER
/// @def EMU7K_VER
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DEMU7K_VER=<version>
#define EMU7K_VER (dev)
#endif
#ifndef EMU7K_BUILD
/// @def EMU7K_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMBTRN_BUILD=`date`
#define EMU7K_BUILD VERSION_STRING(EMU7K_VER)" "LIBMFRAME_BUILD
#endif

/// @def CUR_FRAME
/// @brief current frame index
#define CUR_FRAME 0
/// @def NXT_FRAME
/// @brief next frame index
#define NXT_FRAME 1
/// @def TDIFF
/// @brief diff time (b-a)
#define TDIFF(a,b) (b-a)

/////////////////////////
// Declarations
/////////////////////////

///// @enum emu7k_channel_id
///// @brief test module channel IDs
///// [note : starting above reserved mframe channel IDs]
//typedef enum{
//    ID_EMU7K_V1=MM_CHANNEL_COUNT,
//    ID_EMU7K_V2,
//    ID_EMU7K_V3,
//    ID_EMU7K_V4,
//    ID_EMU7K_V5,
//    EMU7K_CH_COUNT
//}emu7k_channel_id;
//
///// @enum emu7k_channel_mask
///// @brief test module channel masks
//typedef enum{
//    EMU7K_V1= (1<<ID_EMU7K_V1),
//    EMU7K_V2= (1<<ID_EMU7K_V2),
//    EMU7K_V3= (1<<ID_EMU7K_V3),
//    EMU7K_V4= (1<<ID_EMU7K_V4),
//    EMU7K_V5= (1<<ID_EMU7K_V5)
//}emu7k_channel_mask;
//
///// @var char *emu7k_ch_names[EMU7K_CH_COUNT]
///// @brief module channel names
//char *emu7k_ch_names[EMU7K_CH_COUNT]={
//    "trace.emu7k",
//    "debug.emu7k",
//    "warn.emu7k",
//    "err.emu7k",
//    "emu7k.v1",
//    "emu7k.v2"
//    "emu7k.v3"
//    "emu7k.v4"
//    "emu7k.v5"
//};
//static mmd_module_config_t mmd_config_default= {MOD_EMU7K,"MOD_EMU7K",EMU7K_CH_COUNT,((MM_ERR|MM_WARN)|EMU7K_1),emu7k_ch_names};

static void s_show_help();
static void s_parse_args(int argc, char **argv, app_cfg_t *cfg);
static int s_server_handle_request(emu7k_t *svr, byte *req, int rlen, int client_fd);
static void *s_server_publish(void *arg);

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////
static bool g_interrupt=false;
static int g_verbose=0;

/////////////////////////
// Function Definitions
/////////////////////////

/// @fn emu7k_client_t * emu7k_client_new(msock_socket_t *s, uint32_t nsubs, int32_t *subs)
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
        self->sub_count=nsubs;
        self->sub_list=NULL;
        if (self->sub_count > 0) {
            self->sub_list = (int32_t *)malloc(self->sub_count * sizeof(int32_t));
            memcpy(self->sub_list,subs,self->sub_count * sizeof(int32_t));
        }
    }
    return self;
}
// End function emu7k_client_new

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
                msock_socket_destroy(&self->sock_if);
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
// End function emu7k_client_destroy

/// @fn emu7k_t * emu7k_new(msock_socket_t * s, mfile_file_t * mb_data, app_cfg_t *cfg)
/// @brief create new emu7k test server - emulate reson 7k center
/// @param[in] s socket reference
/// @param[in] mb_data reson data file
/// @param[in] cfg app configuration
/// @return new server reference
emu7k_t *emu7k_new(msock_socket_t *s, mfile_file_t *mb_data, app_cfg_t *cfg)
{
    emu7k_t *self = (emu7k_t *)malloc(sizeof(emu7k_t));
    if (self) {
        memset((void *)self,0,sizeof(emu7k_t));
        self->auto_free = true;
        self->sock_if=s;
        self->stop=false;
        self->t=mthread_thread_new();
        self->w=mthread_thread_new();
        self->max_clients=16;
        self->client_count=0;
        self->client_list = mlist_new();
        self->cfg = cfg;
        self->reader=r7kr_freader_new(mb_data, 2*MAX_FRAME_BYTES_7K, NULL, 0);
    }
    return self;
}
// End function emu7k_new

/// @fn emu7k_t * emu7k_new(msock_socket_t * s, mlist_t *file_list, app_cfg_t *cfg)
/// @brief create new emu7k test server - emulate reson 7k center
/// @param[in] s socket reference
/// @param[in] mb_data reson data file (optional)
/// @return new server reference
emu7k_t *emu7k_lnew(msock_socket_t *s, mlist_t *path_list, app_cfg_t *cfg)
{
    emu7k_t *self = (emu7k_t *)malloc(sizeof(emu7k_t));
    if (self) {
        memset((void *)self,0,sizeof(emu7k_t));
        self->auto_free = true;
        self->sock_if   = s;
        self->stop      = false;
        self->t         = mthread_thread_new();
        self->w         = mthread_thread_new();
        self->max_clients  = 16;
        self->client_count = 0;
        self->client_list  = mlist_new();
        self->cfg       = cfg;
        self->reader    = r7kr_freader_new(NULL, 2*MAX_FRAME_BYTES_7K, NULL, 0);
        self->file_list = NULL;

        if (mlist_size(path_list)>0) {
            self->file_list = mlist_new();
            char *file_path = (char *)mlist_first(path_list);
            while (file_path!=NULL) {
                mfile_file_t *file = mfile_file_new(file_path);
                mlist_add(self->file_list,file);
                file_path = (char *)mlist_next(path_list);
            }
        }else{
            fprintf(stderr,"emu7k_lnew: ERR - no input files\n");
        }
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
                MX_LPRINT(EMU7K, 1, "closing server socket[%s:%d] fd[%d]\n", self->sock_if->addr->host, self->sock_if->addr->port, self->sock_if->fd);
                msock_socket_destroy(&self->sock_if);
                mthread_thread_destroy(&self->t);
                mthread_thread_destroy(&self->w);

                emu7k_client_t *client = (emu7k_client_t *)mlist_first(self->client_list);
                while (NULL!=client) {
                    emu7k_client_destroy(&client);
                    client=NULL;
                    client = (emu7k_client_t *)mlist_next(self->client_list);
                }

                mlist_destroy(&self->client_list);

                mfile_file_t *file = (mfile_file_t *)mlist_first(self->file_list);
                while (NULL!=file) {
                    mfile_file_destroy(&file);
                    file = (mfile_file_t *)mlist_next(self->file_list);
                }
                mlist_destroy(&self->file_list);
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
        fprintf(stderr,"%*s[frame_err  %10"PRId64"]\n",indent,(indent>0?" ":""), self->frame_err);
        fprintf(stderr,"%*s[sync_bytes %10"PRId64"]\n",indent,(indent>0?" ":""), self->sync_bytes);
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
        fprintf(stderr,"%*s[svr thread   %10p]\n",indent,(indent>0?" ":""), self->t);
        fprintf(stderr,"%*s[wrk thread   %10p]\n",indent,(indent>0?" ":""), self->w);
        fprintf(stderr,"%*s[max_clients  %10u]\n",indent,(indent>0?" ":""), self->max_clients);
        fprintf(stderr,"%*s[client_count %10u]\n",indent,(indent>0?" ":""), self->client_count);
        fprintf(stderr,"%*s[client_list  %10p]\n",indent,(indent>0?" ":""), self->client_list);
        fprintf(stderr,"%*s[auto_free    %10c]\n",indent,(indent>0?" ":""), (self->auto_free?'Y':'N'));
        fprintf(stderr,"%*s[stop         %10c]\n",indent,(indent>0?" ":""), (self->stop?'Y':'N'));
        fprintf(stderr,"%*s[stats        %10p]\n",indent,(indent>0?" ":""), &self->stats);
        fprintf(stderr,"%*s[cfg          %10p]\n",indent,(indent>0?" ":""), self->cfg);
        fprintf(stderr,"%*s[file_list    %10p]\n",indent,(indent>0?" ":""), self->file_list);
        if (verbose) {
            mfile_file_t *file = (mfile_file_t *)mlist_first(self->file_list);
            while (NULL!=file) {
                fprintf(stderr,"%*s[file         %s]\n",indent,(indent>0?" ":""), file->path);
                file = (mfile_file_t *)mlist_next(self->file_list);
            }
        }
    }
}
// End function emu7k_show

static int64_t read_s7k_frame(emu7k_t *self, byte *dest, uint32_t len, uint32_t *sync_bytes, emu7k_stat_t *stats)
{
    int64_t retval=-1;

    if (NULL!=self && NULL!=self->reader && NULL!=self->cfg &&
        NULL!=dest) {

        int64_t rbytes=0;
        r7kr_flags_t rflags = ( (self->cfg->netframe_input) ? R7KR_NET_STREAM : R7KR_DRF_STREAM );

        if( (rbytes = r7kr_read_frame(self->reader, dest, len, rflags , 0.0, 20, sync_bytes )) >0 ){
            retval=rbytes;
            MX_LPRINT(EMU7K, 2, "r7kr_read_frame_new returned %s sz[%"PRId64"] sync[%"PRIu32"/x%X]\n", ((self->cfg->netframe_input) ? "NF" : "DRF"), rbytes, *sync_bytes, *sync_bytes);
            //            if (self->cfg->netframe_input) {
            //                r7k_nf_show((r7k_nf_t *)dest,false,5);
            //                r7k_drf_show((r7k_drf_t *)(dest+R7K_NF_BYTES),false,5);
            //                r7k_hex_show(dest,R7K_NF_BYTES,16,true,5);
            //            }else{
            //                r7k_drf_show((r7k_drf_t *)(dest),false,5);
            //            }
        }else{
            if(NULL!=stats){
                stats->frame_err++;
                stats->sync_bytes=*sync_bytes;
            }
        }
    }else{
        MX_ERROR_MSG("invalid argument\n");
    }

    return retval;
}
// End function read_s7k_frame

/// @fn void * s_server_publish(void * arg)
/// @brief test server publishing thread function.
/// @param[in] arg void pointer to server reference
/// @return 0 on success, -1 otherwise
static void *s_server_publish(void *arg)
{
    emu7k_t *svr = (emu7k_t *)arg;

    if (NULL!=svr) {
        msock_socket_t  *s = svr->sock_if;
        bool stop_req    = true;

#if defined (__APPLE__)
        // OSX segfaults when putting large arrays on stack
        byte *cur_frame=NULL;
        byte *nxt_frame=NULL;
        cur_frame=(byte *)malloc(R7K_MAX_FRAME_BYTES*sizeof(byte));
        nxt_frame=(byte *)malloc(R7K_MAX_FRAME_BYTES*sizeof(byte));
#else
        byte cur_frame[R7K_MAX_FRAME_BYTES];
        byte nxt_frame[R7K_MAX_FRAME_BYTES];
#endif

        // iterate over the file list
        mfile_file_t *source_file = (mfile_file_t *)mlist_first(svr->file_list);
        uint32_t start_offset = svr->cfg->start_offset;

        while (NULL!=source_file && !svr->stop) {
            double min_delay = ((double)svr->cfg->min_delay)/1000.0;
            double max_delay = ((double)svr->cfg->max_delay)/1000.0;//MAX_DELAY_DFL_SEC;

            MX_LPRINT(EMU7K, 1, "running file[%s]\n", source_file->path);
            MX_LPRINT(EMU7K, 1, "min_delay[%.3lf] max_delay[%.3lf]\n", min_delay, max_delay);
            if (r7kr_reader_set_file(svr->reader,source_file)!=0) {
                MX_ERROR_MSG("r7kr_reader_set_file failed\n");
                source_file=NULL;
            }

            if (NULL != source_file) {
                double sys_start = 0.0;
                double str_start = 0.0;
                double pkt_time  = 0.0;
                double sys_now   = 0.0;

                r7k_drf_t *pdrf[2] = {0};
                r7k_nf_t *pnf[2] = {0};
                int64_t rbytes=0;
                uint32_t sync_bytes=0;
                byte *pframe = NULL;

                struct timespec delay;
                struct timespec rem;
                int seq_number=0;
                bool delete_client=false;
                emu7k_stat_t *stats = &svr->stats;

                off_t file_end = mfile_seek(source_file,0,MFILE_END);
                if ((off_t)start_offset>=file_end) {
                    mfile_seek(source_file,file_end,MFILE_SET);
                    start_offset-=file_end;
                }else{
                    mfile_seek(source_file,start_offset,MFILE_SET);
                    start_offset=0;
                }
                //                mfile_seek(source_file,0,MFILE_SET);
                off_t file_cur = mfile_seek(source_file,0,MFILE_CUR);

                memset((void *)cur_frame,0,R7K_MAX_FRAME_BYTES*sizeof(byte));
                memset((void *)nxt_frame,0,R7K_MAX_FRAME_BYTES*sizeof(byte));

                pnf[CUR_FRAME] = (r7k_nf_t *)(cur_frame);
                pdrf[CUR_FRAME] = (r7k_drf_t *)(cur_frame+R7K_NF_BYTES);
                pnf[NXT_FRAME] = (r7k_nf_t *)(nxt_frame);
                pdrf[NXT_FRAME] = (r7k_drf_t *)(nxt_frame+R7K_NF_BYTES);

                // seed the frame buffer
                // [need to look ahead for timing]
                pframe = ( svr->cfg->netframe_input ? cur_frame : (cur_frame+R7K_NF_BYTES));
                if ( (rbytes=read_s7k_frame(svr, pframe, R7K_MAX_FRAME_BYTES, &sync_bytes,stats)) > 0) {

                    sync_bytes=0;
                    pframe = ( svr->cfg->netframe_input ? nxt_frame : (nxt_frame+R7K_NF_BYTES));
                    if ( (rbytes=read_s7k_frame(svr, pframe, R7K_MAX_FRAME_BYTES, &sync_bytes,stats)) > 0) {
                        stop_req=false;

                    }else{
                        MX_ERROR("ERR - init next frame failed ret[%"PRId64"] [%d/%s]\n", rbytes, me_errno, me_strerror(me_errno));
                        pdrf[NXT_FRAME] = NULL;
                        pnf[NXT_FRAME] = NULL;
                    }
                }else{
                    MX_ERROR("ERR - init current frame failed ret[%"PRId64"] [%d/%s]\n", rbytes, me_errno, me_strerror(me_errno));
                    pdrf[CUR_FRAME] = NULL;
                    pnf[CUR_FRAME] = NULL;
                }

                // mark the start of the system and stream times
                str_start = r7k_7ktime2d(&pdrf[CUR_FRAME]->_7ktime);
                sys_start = mtime_dtime();

                while (!stop_req && !svr->stop) {

                    if (mlist_size(svr->client_list)>0) {

                        emu7k_client_t *client = (emu7k_client_t *)mlist_first(svr->client_list);

                        if (svr->cfg->netframe_input == false) {
                            memset((void *)cur_frame,0,R7K_NF_BYTES);

                            pnf[CUR_FRAME]->protocol_version = R7K_NF_PROTO_VER;
                            pnf[CUR_FRAME]->tx_id       = r7k_txid();
                            pnf[CUR_FRAME]->seq_number  = seq_number++;
                            pnf[CUR_FRAME]->offset      = R7K_NF_BYTES;
                            pnf[CUR_FRAME]->packet_size = R7K_NF_BYTES+pdrf[CUR_FRAME]->size;
                            pnf[CUR_FRAME]->total_size  = pdrf[CUR_FRAME]->size;
                            pnf[CUR_FRAME]->total_records  = 1;
                        }

                        if (svr->cfg->verbose>=3) {
                            fprintf(stderr,"CUR_FRAME cf[%p] pnf[%p] pdrf[%p]\n",cur_frame, (byte *)pnf[CUR_FRAME],(byte *)pdrf[CUR_FRAME]);
                            if (svr->cfg->netframe_input) {
                                r7k_nf_show(pnf[CUR_FRAME],false,5);
                            }
                            r7k_drf_show(pdrf[CUR_FRAME],false,5);
                            r7k_hex_show(cur_frame,(R7K_NF_BYTES+R7K_DRF_BYTES),16,true,5);
                        }

                        // iterate over client list
                        while (NULL != client && NULL!=pdrf[CUR_FRAME] ) {

                            delete_client=false;

                            // check client's subscription list
                            for (uint32_t i=0; i<client->sub_count; i++) {
                                // send frame if client is subscribed
                                if ( pdrf[CUR_FRAME]->record_type_id == (uint32_t)client->sub_list[i] ) {

                                    if (min_delay>=0.0) {
                                        // get current time
                                        sys_now  = mtime_dtime();
                                        pkt_time = r7k_7ktime2d(&pdrf[CUR_FRAME]->_7ktime);
                                        // compare packet time and real time
                                        // relative to start times
                                        double sys_diff = sys_now-sys_start;
                                        double str_diff = pkt_time-str_start;
                                        double twait = 0.0;
                                        // if packet is behind, send it
                                        // if packet is ahead delay
                                        if (str_diff>0.0) {
                                            if (str_diff>sys_diff) {
                                                twait = str_diff-sys_diff;
                                                // Since the stream jumped ahead,
                                                // delay, then advance the
                                                // elapsed system time by moving
                                                // the start back, rather than
                                                // advancing the system clock.
                                                sys_start -= twait;
                                            }else{
                                                twait = 0.0;
                                            }
                                        }else{
                                            twait = 0.0;
                                        }
                                        MX_LMSG(EMU7K, 1, "\n");
                                        MX_LPRINT(EMU7K, 1, "sys_start[%14.3lf] sys_now [%14.3lf] sys_dif[%14.3lf]\n", sys_start, sys_now, sys_diff);
                                        MX_LPRINT(EMU7K, 1, "str_start[%14.3lf] pkt_time[%14.3lf] str_dif[%14.3lf]\n", str_start, pkt_time, str_diff);
                                        MX_LPRINT(EMU7K, 1, "twait[%7.3lf]\n", twait);
                                        // adjust delay max/min constraints
                                        if (min_delay==0.0 && twait>max_delay) {
                                            // max_delay only applied to
                                            // minimize long stream delays.
                                            // min_delay overrides max_delay
                                            // if it is larger
                                            twait = max_delay;

                                            if( svr->cfg->verbose>=2) {
                                                char isostr[64]={0};
                                                time_t tt_pkt=(time_t)0+pkt_time;
                                                struct tm tm_pkt;
                                                // should already be UTC; use localtime not gmtime
                                                localtime_r(&tt_pkt,&tm_pkt);

                                                strftime(isostr,64,"%FT%H:%M:%S",&tm_pkt);
                                                MX_LPRINT(EMU7K, 1, "WARN: possible data gap twait[%lf] ending @ %0.3lf [%s]\n", twait, pkt_time, isostr);
                                            }
                                        }
                                        if(twait<min_delay){
                                            twait = min_delay;
                                        }

                                        if (twait>0.0) {
                                            double dsec = 0;
                                            double dnsec = modf(twait,&dsec);
                                            time_t lsec = (time_t)dsec;
                                            long lnsec = (dnsec*1.0e9);
                                            delay.tv_sec=lsec;
                                            delay.tv_nsec=lnsec;
                                            MX_LPRINT(EMU7K, 1, "twait[%.3lf] ds[%.3lf/%.3lf] ls[%ld/%ld] min/max[%.3lf/%.3lf]\n", twait, dsec, dnsec, (long)lsec, lnsec, min_delay, max_delay);
                                            MX_LPRINT(EMU7K, 1, "delaying %.3lf sec:nsec[%ld:%ld]\n", twait, delay.tv_sec, delay.tv_nsec);
                                            while (nanosleep(&delay,&rem)<0) {
                                                MX_LMSG(EMU7K, 5, "sleep interrupted\n");
                                                delay.tv_sec=rem.tv_sec;
                                                delay.tv_nsec=rem.tv_nsec;
                                            }
                                        }
                                    }// else no delay

                                    MX_LPRINT(EMU7K, 1, ">>>> sending frame ofs[%"PRId32"] len[%6"PRIu32"] txid[%5"PRIu16"] seq[%"PRIu32"] type[%"PRIu32"] ts[%.3lf]\n", (int32_t)file_cur, pnf[CUR_FRAME]->packet_size, pnf[CUR_FRAME]->tx_id, pnf[CUR_FRAME]->seq_number, pdrf[CUR_FRAME]->record_type_id, pkt_time);

                                    if( svr->cfg->verbose>=3) {
                                        r7k_nf_show(pnf[CUR_FRAME],false,5);
                                        r7k_drf_show(pdrf[CUR_FRAME],false,5);
                                        r7k_hex_show(cur_frame,pnf[CUR_FRAME]->packet_size,16,true,5);
                                    }

                                    int64_t status=-1;
                                    if( (status=msock_send(client->sock_if, cur_frame, pnf[CUR_FRAME]->packet_size))<=0){
                                        MX_ERROR("send failed [%"PRId64"] [%d/%s]\n", status, errno, strerror(errno));
                                        if ( errno==EPIPE || errno==ECONNRESET || errno==EBADF) {
                                            delete_client=true;
                                        }
                                    }

                                    stats->pub_total++;
                                    stats->pub_cycle++;

                                    if (delete_client) {
                                        MX_LPRINT(EMU7K, 1, "connection broken, deleting client %p fd[%d]\n", client, client->fd);
                                        mlist_remove(svr->client_list,client);
                                        emu7k_client_destroy(&client);
                                        MX_LPRINT(EMU7K, 1, "clients remaining[%"PRIu32"]\n", (uint32_t)mlist_size(svr->client_list));
                                        client=NULL;
                                        stats->con_active--;
                                    }

                                    // test feature: delay xds sec every xdt sec
                                    // (stop xmission w/o disconnecting socket, to test mbtrnpp response)
                                    if(svr->cfg->xds>1){
                                        time_t xdnow=time(NULL);
                                        if ( (xdnow-svr->cfg->xdstart) >= svr->cfg->xdt) {
                                            fprintf(stderr,"xdelay[%ld][%d]\n",svr->cfg->xdt, svr->cfg->xds);
                                            sleep(svr->cfg->xds);
                                            svr->cfg->xdstart=xdnow;
                                        }
                                    }

                                    break;
                                }else{
                                    if (NULL!=pdrf[CUR_FRAME]) {
                                        MX_LPRINT(EMU7K, 5, "client[%d] record[%d] not type[%d]\n", client->sock_if->fd, pdrf[CUR_FRAME]->record_type_id, client->sub_list[i]);
                                    }
                                }
                                // if client subscribed
                            }// for client
                            // get next client
                            client=(emu7k_client_t *)mlist_next(svr->client_list);
                        }// while clients

                        // check for end of input file
                        file_cur = mfile_seek(source_file,0,MFILE_CUR);
                        if (file_cur >= file_end) {
                            stats->cyc_total++;
                            stats->rec_cycle=0;
                            stats->pub_cycle=0;

                            MX_LPRINT(EMU7K, 2, "reached end of file eof[%"PRId32"] cur[%"PRIu32"]\n", (int32_t)file_end, (uint32_t)file_cur);

                            MX_LMSG(EMU7K, 2, "setting stop_req\n");
                            stop_req=true;
                        }

                        if (!stop_req) {
                            // clear current frame
                            memset((void *)cur_frame, 0, R7K_MAX_FRAME_BYTES);
                            // move next to current
                            memcpy((void *)cur_frame, nxt_frame, R7K_MAX_FRAME_BYTES);

                            // clear next frame data
                            memset((void *)nxt_frame, 0, R7K_MAX_FRAME_BYTES);

                            // read next record
                            sync_bytes=0;
                            pframe = ( svr->cfg->netframe_input ? nxt_frame : (nxt_frame+R7K_NF_BYTES));
                            if ( (rbytes=read_s7k_frame(svr, pframe, R7K_MAX_FRAME_BYTES, &sync_bytes,stats)) > 0) {
                                stop_req=false;
                            }else{
                                MX_ERROR_MSG("ERR - read next returned[%"PRId64"] syncbytes[%"PRIu32"] [%d/%s]\n", rbytes, sync_bytes, me_errno, me_strerror(me_errno));
                                MX_LMSG(EMU7K, 2, "setting stop_req\n");
                                stop_req=true;
                            }

                            MX_LPRINT(EMU7K, 2, "read frame at ofs[%"PRId32"/x%08X] rbytes[%"PRId64"] sbytes[%"PRIu32"]\n", (int32_t)file_cur, (unsigned int)file_cur, rbytes, sync_bytes);

                            if( svr->cfg->verbose>=3) {
                                if (svr->cfg->netframe_input) {
                                    r7k_nf_show(pnf[NXT_FRAME],false,5);
                                }
                                r7k_drf_show(pdrf[NXT_FRAME],false,5);
                                r7k_hex_show(cur_frame,pdrf[NXT_FRAME]->size,16,true,5);
                            }

                            stats->rec_cycle++;
                            stats->rec_total++;
                        }

                        if ( (svr->cfg->verbose>=2) &&
                            (svr->cfg->statn>0) &&
                            (stats->rec_total%svr->cfg->statn == 0) ) {
                            MX_LMSG(EMU7K, 2, "stats\n");
                            emu7k_stat_show(stats,false,7);
                        }

                    }else{
                        //                        MX_LMSG(EMU7K, 1, "no clients\n");
                        sleep(1);
                    }
                }// while !stop

                if(svr->cfg->verbose>=1){
                    MX_LMSG(EMU7K, 1, "stopped - stats\n");
                    emu7k_stat_show(stats,false,7);
                }

            }else{
                MX_ERROR_MSG("NULL source file\n");
                s->status=-1;
            }

            // get next file from list
            source_file = (mfile_file_t *)mlist_next(svr->file_list);

            // if file is NULL (end of list) and restart set
            // start at beginning of list
            if (NULL==source_file && svr->cfg->restart) {
                MX_LMSG(EMU7K, 2, "restarting at beginning of file list\n");
                source_file = (mfile_file_t *)mlist_first(svr->file_list);
            }

        }// while source_file


        MX_LPRINT(EMU7K, 2, "publisher exiting sreq[%c] stop[%c]\n", (stop_req?'Y':'N'), (svr->stop?'Y':'N'));

#if defined (__APPLE__)
        free(cur_frame);
        free(nxt_frame);
#endif
        g_interrupt = true;
        pthread_exit((void *)&s->status);

        return (void *)(&s->status);
    }else{
        MX_ERROR_MSG("NULL server\n");
    }
    pthread_exit((void *)NULL);

    return (void *)NULL;


}
// End function s_server_publish

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
            MX_LMSG(EMU7K, 1, "STOP received\n");
            send(client_fd,"ACK",strlen("ACK"),0);
            svr->stop=true;
        }else if(strncmp((const char *)req,"REQ",3)==0){
            MX_LMSG(EMU7K, 1, "REQ received\n");
            send(client_fd,"ACK",strlen("ACK"),0);
        }else if((unsigned long)rlen>=hdr_len){
            MX_LPRINT(EMU7K, 1, "proto ver      [%d]\n", nf->protocol_version);
            MX_LPRINT(EMU7K, 1, "record_type_id [%"PRIu32"]\n", drf->record_type_id);

            if(nf->protocol_version == R7K_NF_PROTO_VER &&
               drf->record_type_id == R7K_RT_REMCON &&
               rth->remcon_id == R7K_RTID_SUB){

                // got 7k center subscription record
                MX_LMSG(EMU7K, 1, "7K SUB request received\n");
                // create, send SUB ACK message
                r7k_msg_t *msg = r7k_msg_new(sizeof(r7k_rth_7501_ack_t));
                r7k_rth_7501_ack_t *prth = (r7k_rth_7501_ack_t *)(msg->data);
                prth->ticket = 1;
                memcpy(prth->tracking_number, "ABCDEF0123456789",strlen("ABCDEF0123456789"));
                msg->drf->size           = R7K_MSG_DRF_SIZE(msg);
                msg->drf->record_type_id = R7K_RT_REMCON_ACK;
                msg->drf->device_id      = R7K_DEVID_7KCENTER;
                msg->nf->tx_id       = r7k_txid();
                msg->nf->seq_number  = 0;
                msg->nf->packet_size = R7K_MSG_NF_PACKET_SIZE(msg);
                msg->nf->total_size  = R7K_MSG_NF_TOTAL_SIZE(msg);
                r7k_msg_set_checksum(msg);

                MX_LMSG(EMU7K, 1, "sending SUB ACK:\n");
                if(svr->cfg->verbose>=1){
                    r7k_msg_show(msg,true,3);
                }
                msock_socket_t *s = msock_wrap_fd(client_fd);
                r7k_msg_send(s,msg);
                r7k_msg_destroy(&msg);

                // get sub request data
                byte *pdata = (req + hdr_len);
                uint32_t nsubs = *((uint32_t *)pdata);
                int32_t *subs = (int32_t *)(pdata+sizeof(uint32_t));

                // create client, add to list
                emu7k_client_t *cli = emu7k_client_new(client_fd, nsubs, subs);
                MX_LPRINT(EMU7K, 1, "adding client fd[%d] to list\n", client_fd);
                mlist_add(svr->client_list, cli);
                cli->sock_if=s;
            }
        }else{
            MX_ERROR_MSG("ERR - unsupported request\n");
            retval=-1;
        }
    }else{
        MX_ERROR_MSG("ERR - invalid/NULL request\n");
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

    int *retval=NULL;

    if ( (NULL!=svr) ) {
        struct timeval tv;
        fd_set master;
        fd_set read_fds;
        byte iobuf[256]; // buffer for client data
        struct sockaddr_storage client_addr={0};
        socklen_t addr_size=0;
        bool stop_req=false;
        emu7k_stat_t *stats = &svr->stats;

        stats->start_time=time(NULL);

        msock_socket_t  *s = svr->sock_if;
        msock_set_blocking(s,true);
        MX_LMSG(EMU7K, 4, "starting worker thread\n");
        if(mthread_thread_start(svr->w, s_server_publish, (void *)svr)!=0){
            MX_ERROR_MSG("worker thread start failed\n");
            stop_req=true;
        }else{

            char buf[ADDRSTR_BYTES]={0};
            int fdmax;

            const int optionval = 1;
#if !defined(__CYGWIN__)
            msock_set_opt(s, SO_REUSEPORT, &optionval, sizeof(optionval));
#endif
            msock_set_opt(s, SO_REUSEADDR, &optionval, sizeof(optionval));

            msock_bind(s);
            MX_LPRINT(EMU7K, 2, "server [%s] - starting\n", msock_addr2str(s, buf, ADDRSTR_BYTES));
            msock_listen(s,1);

            tv.tv_sec = 3;
            tv.tv_usec = 0;

            FD_ZERO(&read_fds);
            FD_ZERO(&master);
            FD_SET(s->fd,&master);
            fdmax = s->fd;

            while (!svr->stop && !stop_req) {
                read_fds = master;
                int stat=0;
                // MMINFO(APP1,"pending on select\n");
                if( (stat=select(fdmax+1, &read_fds, NULL, NULL, &tv)) != -1){
                    int newfd=-1;
                    for (int i=s->fd; i<=fdmax; i++) {

                        if (FD_ISSET(i, &read_fds)){
                            // MMINFO(APP1,"readfs [%d/%d] selected\n",i,fdmax);
                            if (i==s->fd) {
                                MX_LPRINT(EMU7K, 4, "server main listener [%d] got request\n", i);

                                newfd = accept(s->fd, (struct sockaddr *)&client_addr, &addr_size);
                                if (newfd != -1) {
                                    MX_LPRINT(EMU7K, 4, "client connected on socket [%d]\n", newfd);
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
                                int nbytes=0;
                                bool do_close=false;
                                MX_LPRINT(EMU7K, 4, "server waiting for client data fd[%d]\n", i);
                                if (( nbytes = recv(i, iobuf, sizeof iobuf, 0)) <= 0) {
                                    MX_LPRINT(EMU7K, 4, "ERR - recv failed fd[%d] nbytes[%d] [%d/%s]\n", i, nbytes, errno, strerror(errno));
                                    // got error or connection closed by client
                                    if (nbytes == 0) {
                                        // connection closed
                                        fprintf(stderr,"ERR - socket %d hung up\n", i);
                                    } else if(nbytes<0) {
                                        fprintf(stderr,"ERR - recv failed socket[%d] [%d/%s]\n",i,errno,strerror(errno));
                                    }
                                    do_close=true;
                                }else{
                                    MX_LPRINT(EMU7K, 4, "server received request on socket [%d] len[%d]\n", i, nbytes);
                                    s_server_handle_request(svr,iobuf,nbytes,i);
                                }
                                if (do_close) {
                                    fprintf(stderr,"ERR - closing fd[%d]\n", i);
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
                MX_LMSG(EMU7K, 1, "stats\n");
                emu7k_stat_show(stats,false,7);
            }
        }
        if (stop_req) {
            MX_LMSG(EMU7K, 3, "Test server - interrupted - stop flag set\n");
            if (NULL!=s) {
                s->status=1;
            }
        }else{
            MX_LMSG(EMU7K, 3, "Test server - normal exit\n");
            if (NULL!=s) {
                s->status=0;
            }
        }

        retval=(NULL!=s?&s->status:NULL);
    }


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
        if(mthread_thread_start(self->t, s_server_main, (void *)self)!=0){
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
        MX_LMSG(EMU7K, 2, "stopping server thread\n");
        self->stop=true;

        while( mthread_thread_join(self->t)!=0){

        }
        retval=0;
    }
    return retval;
}
// End function emu7k_stop

/// @fn void s_show_help()
/// @brief output user help message to stdout.
/// @return none
static void s_show_help()
{
    char help_message[] = "\n Emulate 7k Center using .s7k file data or network frame logs\n";
    char usage_message[] = "\n emu7k [options] file [file...]\n"
    "\n Options:\n"
    "  --verbose=n    : verbose output level\n"
    "  --version      : print version info\n"
    "  --host=s       : host IP address or name\n"
    "  --port=n       : TCP/IP port\n"
    "  --min-delay=n  : minimum packet processing delay (msec)\n"
    "  --max-delay=n  : maximum packet processing delay (msec)\n"
    "  --restart      : restart data when end of file is reached\n"
    "  --no-restart   : stop when end of file is reached\n"
    "  --statn=n      : output stats every n records\n"
    "  --xdelay=n/s   : [test feature] wait s seconds every n messages\n"
    "  --nf           : input includes network frames\n"
    "  --offset=n     : start offset\n"
    "\n";
    //    "--file=s       : data file path (.s7k)\n"
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
    bool version=false;

    static struct option options[] = {
        {"verbose", required_argument, NULL, 0},
        {"help", no_argument, NULL, 0},
        {"version", no_argument, NULL, 0},
        {"host", required_argument, NULL, 0},
        {"port", required_argument, NULL, 0},
        {"file", required_argument, NULL, 0},
        {"min-delay", required_argument, NULL, 0},
        {"max-delay", required_argument, NULL, 0},
        {"statn", required_argument, NULL, 0},
        {"restart", no_argument, NULL, 0},
        {"no-restart", no_argument, NULL, 0},
        {"xdelay", required_argument, NULL, 0},
        {"nf", no_argument, NULL, 0},
        {"offset", required_argument, NULL, 0},
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
                // version
                if (strcmp("version", options[option_index].name) == 0) {
                    version=true;
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
                    sscanf(optarg,"%d",&cfg->min_delay);
                }
                // max-delay
                else if (strcmp("max-delay", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->max_delay);
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
                // xdelay
                else if (strcmp("xdelay", options[option_index].name) == 0) {
                    sscanf(optarg,"%ld/%d",&cfg->xdt,&cfg->xds);
                }
                // nf
                else if (strcmp("nf", options[option_index].name) == 0) {
                    cfg->netframe_input=true;
                }
                // offset
                else if (strcmp("offset", options[option_index].name) == 0) {
                    sscanf(optarg,"%"PRIu32"",&cfg->start_offset);
                }
                break;
            default:
                help=true;
                break;
        }
        if (version) {
            MFRAME_SHOW_VERSION(EMU7K_NAME, EMU7K_BUILD);
            //            r7kr_show_app_version(EMU7K_NAME, EMU7K_BUILD);
            exit(0);
        }
        if (help) {
            MFRAME_SHOW_VERSION(EMU7K_NAME, EMU7K_BUILD);
            //            r7kr_show_app_version(EMU7K_NAME, EMU7K_BUILD);
            s_show_help();
            exit(0);
        }
    }// while

    for (int i=optind; i<argc; i++) {
        mlist_add(cfg->file_paths,strdup(argv[i]));
    }

    if (cfg->verbose>0) {
        fprintf(stderr,"verbose   [%d]\n",cfg->verbose);
        fprintf(stderr,"host      [%s]\n",cfg->host);
        fprintf(stderr,"port      [%d]\n",cfg->port);
        fprintf(stderr,"file      [%s]\n",cfg->file_path);
        fprintf(stderr,"restart   [%c]\n",(cfg->restart?'Y':'N'));
        fprintf(stderr,"statn     [%u]\n",cfg->statn);
        fprintf(stderr,"min-delay [%d]\n",cfg->min_delay);
        fprintf(stderr,"max-delay [%d]\n",cfg->max_delay);
        fprintf(stderr,"nf        [%c]\n",(cfg->netframe_input?'Y':'N'));
        fprintf(stderr,"offset    [%"PRIu32"]\n",cfg->start_offset);
        fprintf(stderr,"xds       [%d]\n",cfg->xds);
        fprintf(stderr,"paths     [%p]\n",cfg->file_paths);
        fprintf(stderr,"files:\n");
        char *path=(char *)mlist_first(cfg->file_paths);
        while (NULL!=path) {
            fprintf(stderr,"path      [%s]\n",path);
            path = (char *)mlist_next(cfg->file_paths);
        }
    }

    g_verbose = cfg->verbose;

    mxd_setModule(MXDEBUG, 0, true, NULL);
    mxd_setModule(MXERROR, 5, false, NULL);
    mxd_setModule(EMU7K, 1, false, "emu7k");
    mxd_setModule(EMU7K_ERROR, 1, true, "emu7k.error");
    mxd_setModule(EMU7K_DEBUG, 1, true, "emu7k.debug");
    mxd_setModule(MXMSOCK, 1, true, "msock");
    mxd_setModule(R7KC, 1, true, "r7kc");
    mxd_setModule(R7KC_DEBUG, 1, true, "r7kc.debug");
    mxd_setModule(R7KC_ERROR, 1, true, "r7kc.error");
    mxd_setModule(R7KR, 1, true, "r7kr");
    mxd_setModule(R7KR_ERROR, 1, true, "r7kr.error");
    mxd_setModule(R7KR_DEBUG, 1, true, "r7kr.debug");

    switch (cfg->verbose) {
        case 0:
            break;
        case 1:
            mxd_setModule(MXDEBUG, 0, true, NULL);
            mxd_setModule(MXERROR, 5, false, NULL);
            mxd_setModule(EMU7K, 1, false, "emu7k");
            break;
        case 2:
            mxd_setModule(MXDEBUG, 5, true, NULL);
            mxd_setModule(MXERROR, 5, false, NULL);
            mxd_setModule(EMU7K, 2, false, "emu7k");
            break;
        case 3:
            mxd_setModule(MXDEBUG, 5, true, NULL);
            mxd_setModule(MXERROR, 5, false, NULL);
            mxd_setModule(EMU7K, 3, false, "emu7k");
            break;
        case 4:
            mxd_setModule(MXDEBUG, 5, true, NULL);
            mxd_setModule(MXERROR, 5, false, NULL);
            mxd_setModule(EMU7K, 4, false, "emu7k");
            break;
        case 5:
            mxd_setModule(MXDEBUG, 5, false, NULL);
            mxd_setModule(MXERROR, 5, false, NULL);
            mxd_setModule(EMU7K, 5, false, "emu7k");
            mxd_setModule(EMU7K_ERROR, 5, false, "emu7k.error");
            mxd_setModule(EMU7K_DEBUG, 5, false, "emu7k.debug");
            mxd_setModule(MXMSOCK, 5, false, "msock");
            mxd_setModule(R7KC, 5, false, "r7kc");
            mxd_setModule(R7KC_DEBUG, 5, false, "r7kc.debug");
            mxd_setModule(R7KC_ERROR, 5, false, "r7kc.error");
            mxd_setModule(R7KR, 5, false, "r7kr");
            mxd_setModule(R7KR_ERROR, 5, false, "r7kr.error");
            mxd_setModule(R7KR_DEBUG, 5, false, "r7kr.debug");
            break;
        default:
            break;
    }

    if(cfg->verbose != 0) {
        mxd_show();
    }

}
// End function parse_args

/// @fn void termination_handler (int signum)
/// @brief termination signal handler.
/// @param[in] signum signal number
/// @return none
static void s_termination_handler (int signum)
{
    switch (signum) {
        case SIGINT:
        case SIGHUP:
        case SIGTERM:
            MX_LPRINT(EMU7K, 2, "received sig[%d]\n",signum);
            g_interrupt=true;
            break;
        default:
            MX_ERROR_MSG("not handled[%d]\n",signum);
            break;
    }
}
// End function termination_handler

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
    saStruct.sa_handler = s_termination_handler;
    sigaction(SIGINT, &saStruct, NULL);

    // configure application
    mlist_t *file_paths=mlist_new();
    mlist_autofree(file_paths,free);

    app_cfg_t cfg = {
        VERBOSE_OUTPUT_DFL,
        NULL,
        strdup(EMU_HOST_DFL),
        EMU_PORT_DFL,
        MIN_DELAY_DFL_MSEC,
        MAX_DELAY_DFL_MSEC,
        RESTART_DFL,
        STATN_DFL_REC,
        0,0,0,
        false,
        file_paths,
        0
    };

    // parse command line args
    s_parse_args(argc, argv, &cfg);

    // create/configure server socket
    msock_socket_t *svr_socket = msock_socket_new(cfg.host, cfg.port, ST_TCP);

    // create/configure server
    // emu7k_t *server = emu7k_new(svr_socket, svr_data, &cfg);
    emu7k_t *server = emu7k_lnew(svr_socket, file_paths, &cfg);

    // start server thread
    emu7k_start(server);

    // wait for input signal (SIGINT)
    while (!server->stop && !g_interrupt) {
        sleep(2);
    }
    // stop the server
    MX_LMSG(EMU7K, 1, "stopping server...\n");
    emu7k_stop(server);
    MX_LPRINT(EMU7K, 4, "socket status [%d]\n", svr_socket->status);

    // release resources
    MX_LMSG(EMU7K, 4, "releasing resources...\n");
    // server will release socket, file resources
    emu7k_destroy(&server);
    mlist_destroy(&file_paths);
    free(cfg.host);
    free(cfg.file_path);

    return 0;
}
// End function main
