///
/// @file mb1rs.c
/// @authors k. Headley
/// @date 01 jan 2018

/// MB1 record server

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
#include <getopt.h>
#include <math.h>
#include "mb1rs.h"
#include "mb1_msg.h"
#include "mthread.h"
#include "msocket.h"
#include "merror.h"
#include "mtime.h"

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

static const char *g_state_str[] = {
    "STOPPED",
    "RUNNING"
};

/////////////////////////
// Function Definitions
/////////////////////////

/// @fn const char *mb1rs_get_version()
/// @brief get version string.
/// @return version string
const char *mb1rs_get_version()
{
    return MB1RS_VERSION_STR;
}

/// @fn const char *mb1rs_get_build()
/// @brief get build string.
/// @return version string
const char *mb1rs_get_build()
{
    return MB1RS_BUILD_STR;
}

/// @fn void mb1r_show_app_version(const char *app_version)
/// @brief get version string.
/// @return version string
void mb1rs_show_app_version(const char *app_name, const char *app_version)
{
    printf("\n %s built[%s] libmb1rs[v%s / %s]\n\n",app_name, app_version, mb1rs_get_version(),mb1rs_get_build());
}

int64_t s_file_frame_fn(mb1rs_ctx_t *ctx, byte *r_buf, uint32_t len)
{
    int64_t retval=-1;

    if(NULL!=ctx && NULL!=r_buf && len>0){

        if(NULL==ctx->rfile){
            ctx->rfile = mfile_file_new(ctx->cfg->ifile);
            mfile_open(ctx->rfile,MFILE_RONLY);
        }

        if(ctx->rfile){

            byte *bp = NULL;
            uint32_t readlen = 1;
            uint32_t record_bytes=0;
            int64_t read_bytes=0;
            uint32_t sync_bytes=0;
            mb1_t *dest = mb1_new(MB1_MAX_BEAMS);

            // sync to start of record
            bp = (byte *)dest;

            while( (read_bytes=mfile_read(ctx->rfile,(byte *)bp,readlen))==readlen){
                if(*bp=='M'){
                    // found sync start
                    record_bytes+=read_bytes;
                    bp++;
                    readlen=MB1_HEADER_BYTES-1;
                    break;
                }else{
                    sync_bytes++;
                }
            }

            // if start of sync found, read header (fixed-length sounding bytes)
            if(record_bytes>0 && (read_bytes=mfile_read(ctx->rfile,(byte *)bp,readlen))==readlen){

                record_bytes+=read_bytes;

                bp=NULL;
                readlen=0;
                retval=record_bytes;

                if(NULL!=dest){
                    if(dest->type==MB1_TYPE_ID){
                        if(dest->nbeams>0 && dest->nbeams<=MB1_MAX_BEAMS){
                            if(mb1_resize(&dest, dest->nbeams, MB1_RS_BEAMS)!=NULL){
                                bp=(byte *)&dest->beams[0];
                                readlen = dest->size-(MB1_HEADER_BYTES+MB1_CHECKSUM_BYTES);
                                }else{
                                    fprintf(stderr,"%s:%d - ERR frame_resize\n",__func__,__LINE__);
                                }
                        }else{
                            // don't resize
                        }
                    }else{
                        fprintf(stderr,"%s:%d - ERR invalid type[%08X/%08X]\n",__func__,__LINE__,dest->type,MB1_TYPE_ID);
                        retval=-1;
                    }
                }else{
                    fprintf(stderr,"%s:%d - ERR dest and/or sounding NULL\n",__func__,__LINE__);
                }

                fprintf(stderr,"%d: sounding->sz[%"PRIu32"] read[%"PRId64"/%"PRIu32"] err[%d/%s]\n",__LINE__,dest->size,read_bytes,readlen,errno,strerror(errno));
                fprintf(stderr,"%d: sounding->type[%08X]\n",__LINE__,dest->type);
                fprintf(stderr,"%d: sounding->checksum[%p]\n",__LINE__,MB1_PCHECKSUM(dest));
                if(readlen>0)
                    fprintf(stderr,"%d: sounding->checksum[%"PRIu32"]\n",__LINE__,MB1_GET_CHECKSUM(dest));

                // if header OK, read sounding data (variable length)
                read_bytes=0;
                if(readlen>0 && (read_bytes=mfile_read(ctx->rfile,(byte *)bp,readlen))==readlen){
                    record_bytes+=read_bytes;

                    bp=(byte *)MB1_PCHECKSUM(dest);
                    readlen=MB1_CHECKSUM_BYTES;

                    // read checksum
                    if( (read_bytes=mfile_read(ctx->rfile,(byte *)bp,readlen))==readlen){
                        record_bytes+=read_bytes;
                        retval=record_bytes;
                        memcpy(r_buf,dest,MB1_SOUNDING_BYTES(dest->nbeams));

                        if(mb1_validate_checksum(dest)!=0){
                            fprintf(stderr,"checksum err (calc/read)[%08X/%08X] failed fp/fsz[%"PRId64"/%"PRId64"]\n",mb1_calc_checksum(dest),MB1_GET_CHECKSUM(dest),mfile_seek(ctx->rfile,0,MFILE_CUR),mfile_fsize(ctx->rfile));
                        }
                    }else{
                        fprintf(stderr,"%d: read failed err[%d/%s] fp/fsz[%"PRId64"/%"PRId64"]\n",__LINE__,errno,strerror(errno),mfile_seek(ctx->rfile,0,MFILE_CUR),mfile_fsize(ctx->rfile));
                    }
                }else{
                    if(readlen>0)
                        fprintf(stderr,"%d: read failed err[%d/%s] readlen[%"PRId32"] read_bytes[%"PRId64"] fp/fsz[%"PRId64"/%"PRId64"]\n",__LINE__,errno,strerror(errno),readlen,read_bytes,mfile_seek(ctx->rfile,0,MFILE_CUR),mfile_fsize(ctx->rfile));
                }
            }else{
                if(mfile_seek(ctx->rfile,0,MFILE_CUR)==mfile_fsize(ctx->rfile)){
                    fprintf(stderr,"%d: read failed end of file reached fp/fsz[%"PRId64"/%"PRId64"] err[%d/%s]\n",__LINE__,mfile_seek(ctx->rfile,0,MFILE_CUR),mfile_fsize(ctx->rfile),errno,strerror(errno));
                }else{
                    fprintf(stderr,"%d: read failed err[%d/%s]\n",__LINE__,errno,strerror(errno));
                }
            }
            mb1_destroy(&dest);
        }
    }
    return retval;
}

int64_t s_auto_frame_fn(mb1rs_ctx_t *ctx, byte *dest, uint32_t len)
{
    int64_t retval=-1;
    static double lat=35.0;
    static double lon=-122.0;
    double stime = mtime_dtime();

    lat+=0.0001;
    lon+=0.0001;
    double RX=10.0*sin(0.1*stime*M_PI/180.);
    double RY=10.0*sin(0.1*stime*M_PI/180.);
    double RZ=-5.0*sin(0.1*stime*M_PI/180.);
    double depth=-1000.0*sin(0.001*stime*M_PI/180.);
    double hdg=360.0*sin(0.01*stime*M_PI/180.);
    uint32_t nbeams_hint = ( ((NULL!=ctx) && (NULL!= ctx->cfg)) ? ctx->cfg->auto_nbeams : 0);
    uint32_t snd_sz =MB1_SOUNDING_BYTES(nbeams_hint);
    if(NULL!=dest && len>=snd_sz){
        static int cx=0;
        mb1_t *snd = (mb1_t *)dest;

        snd->type = MB1_TYPE_ID;
        snd->size = snd_sz;
        snd->lat = lat;
        snd->lon = lon;
        snd->hdg = hdg;
        snd->depth = depth;
        snd->nbeams = nbeams_hint;
        snd->ping_number = cx;
        snd->ts = stime;
        int k=0;
        for(k=0;k<nbeams_hint;k++){
            snd->beams[k].beam_num=k;
            snd->beams[k].rhox = RX-0.02*k*nbeams_hint+0.01*k*k;
            snd->beams[k].rhoy = RY-0.03*k*nbeams_hint+0.01*k*k;
            snd->beams[k].rhoz = RZ-10*sin(k*M_PI/180.);
        }
        mb1_set_checksum(snd);
        cx++;
        retval = snd_sz;
    }
    return retval;
}

// server (thread worker)
static void *s_server_function(void *pargs)
{
    mb1rs_ctx_t *ctx = (mb1rs_ctx_t *)pargs;

    if(NULL!=ctx){
        mb1rs_cfg_t *cfg = ctx->cfg;

        struct timeval tv;
        fd_set active_set;
        fd_set read_fds;
        fd_set write_fds;
        fd_set err_fds;
        // buffer for client data
        byte iobuf[MB1_MAX_SOUNDING_BYTES];
        struct sockaddr_storage client_addr={0};
        socklen_t addr_size=0;

        msock_socket_t *s = msock_socket_new(cfg->host, cfg->port, ST_TCP);
        msock_set_blocking(s,true);
        int fdmax=0;
        const int optionval = 1;

#if !defined(__CYGWIN__)
        msock_set_opt(s, SO_REUSEPORT, &optionval, sizeof(optionval));
#endif
        msock_set_opt(s, SO_REUSEADDR, &optionval, sizeof(optionval));
        msock_bind(s);
        msock_listen(s,1);

        tv.tv_sec = cfg->rto_ms/1000;
        tv.tv_usec = ((cfg->rto_ms%1000)*1000L);

        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        FD_ZERO(&err_fds);
        FD_ZERO(&active_set);
        // add server socket to active_set set
        FD_SET(s->fd,&active_set);
        fdmax = s->fd;
        int cx = 0;
        ctx->state = MB1RS_ST_RUNNING;
        while(ctx->stop_req==0){
            read_fds = active_set;
            write_fds = active_set;
            err_fds = active_set;

            int stat=0;
            fprintf(stderr,"server pending on select fd[%d]\n",s->fd);
            if( (stat=select(fdmax+1, &read_fds, &write_fds, &err_fds, &tv)) != -1){
                int newfd=-1;
                for (int i=s->fd; i<=fdmax; i++) {
                    bool do_close=false;
                    if (FD_ISSET(i, &read_fds)){
                        // MMINFO(APP1,"readfs [%d/%d] selected\n",i,fdmax);
                        if (i==s->fd) {
                            fprintf(stderr,"server ready to read\n");

                            newfd = accept(s->fd, (struct sockaddr *)&client_addr, &addr_size);
                            if (newfd != -1) {
                                fprintf(stderr,"client connected on socket fd[%d]\n",newfd);
                                // add client to active list
                                FD_SET(newfd,&active_set);

                                struct timeval rto={1,0};
                                int test = setsockopt(newfd, SOL_SOCKET, SO_RCVTIMEO,
                                                      &rto, sizeof(rto));
                                if(test!=0)fprintf(stderr,"setsockopt [%d] failed[%d/%s]\n",newfd,errno,strerror(errno));
                                if (newfd>fdmax) {
                                    fdmax=newfd;
                                }
                            }else{
                                // accept failed
                                // MMINFO(APP1,"accept failed [%d/%s]\n",errno,strerror(errno));
                            }
                        }else{
                            // client ready to read
                            fprintf(stderr,"server client ready to read fd[%d]\n",i);
                            int nbytes=0;
                            if (( nbytes = recv(i, iobuf, sizeof iobuf, 0)) > 0) {
                                fprintf(stderr,"server received msg on socket [%d] len[%d]\n",i,nbytes);
                                // test proto doesn't expect anything from clients
                            }else{
                                fprintf(stderr,"ERR - recv failed fd[%d] nbytes[%d] [%d/%s]\n",i,nbytes,errno, strerror(errno));
                                // got error or connection closed by client
                                if (nbytes == 0) {
                                    // connection closed
                                    fprintf(stderr,"ERR - socket %d hung up\n", i);
                                    do_close=true;
                                    ctx->err_count++;
                                } else if(nbytes<0) {
                                    fprintf(stderr,"ERR - recv failed socket[%d] [%d/%s]\n",i,errno,strerror(errno));
                                    if(errno!=EAGAIN){
                                        ctx->err_count++;
                                        do_close=true;
                                    }
                                }
                            }
                        }
                    }else{
                        // fd[i] not ready to read
                        // MMINFO(APP1,"readfs fd[%d/%d] ISSET:%s\n",i,fdmax,(FD_ISSET(i,&read_fds)?"TRUE":"FALSE"));
                    }

                    if (FD_ISSET(i, &err_fds)){
                        ctx->err_count++;
                        if (i==s->fd) {
                            fprintf(stderr,"server socket err fd[%d]--stopping\n",i);
                            ctx->stop_req = 1;
                        }else{
                            fprintf(stderr,"client socket err fd[%d] err[%d/%s]\n",i,errno,strerror(errno));
                            do_close=true;
                        }
                    }

                    if (FD_ISSET(i, &write_fds)){
                        if (i==s->fd) {
                            fprintf(stderr,"server socket ready to write fd[%d]\n",i);
                        }else{
                            fprintf(stderr,"client socket ready to write fd[%d]\n",i);

                            if(ctx->frame_func(ctx, iobuf, MB1_MAX_SOUNDING_BYTES) > 0){

                                mb1_t *snd = (mb1_t *)iobuf;

                                if(cfg->err_mod>0 && ++cx%cfg->err_mod==0){
                                    snd->ts+=1;
                                    fprintf(stderr,"!!! server generating invalid frame !!!\n");
                                }

                                int64_t nbytes=send(i,iobuf,MB1_SOUNDING_BYTES(snd->nbeams),0);
                                if( nbytes > 0){
                                    ctx->tx_count++;
                                    ctx->tx_bytes+=nbytes;
                                }

                                fprintf(stderr,"server sent frame len[%"PRId64"]:\n",nbytes);
                                mb1_show(snd,true,5);
                                fprintf(stderr,"\n");
                                mb1_hex_show((byte *)snd,snd->size,16,true,5);
                            }
                        }
                    }

                    if (do_close) {
                        fprintf(stderr,"ERR - closing fd[%d]\n", i);
                        // remove from active_set
                        FD_CLR(i, &active_set);
                        close(i);
                    }
                }// for client
            }else{
                // select failed
                // MMINFO(APP1,"select failed stat[%d] [%d/%s]\n",stat,errno,strerror(errno));
                tv.tv_sec = cfg->rto_ms/1000;
                tv.tv_usec = ((cfg->rto_ms%1000)*1000L);
            }
            if(cfg->del_ms>0){
                struct timespec ts={0};
                struct timespec ts_rem={0};
                ts.tv_sec=cfg->del_ms/1000;
                ts.tv_nsec=(cfg->del_ms%1000)*1000000L;
                nanosleep(&ts,&ts_rem);
            }
            ctx->cyc_count++;
        }//while
        fprintf(stderr,"server stop_req set--exiting\n");
        close(s->fd);
        msock_socket_destroy(&s);
        ctx->state=MB1RS_ST_STOPPED;
    }
    pthread_exit((void *)NULL);
    return (void *)NULL;
}

mb1rs_cfg_t *mb1rs_cfg_new()
{
    mb1rs_cfg_t *instance = (mb1rs_cfg_t *)malloc(sizeof(mb1rs_cfg_t));

    if(NULL!=instance){
        memset(instance,0,sizeof(mb1rs_cfg_t));
        instance->host=strdup(MB1RS_HOST_DFL);
        instance->port=MB1RS_IP_PORT_DFL;
        instance->ifile=NULL;
        instance->lim_cyc=0;
        instance->lim_ret=0;
        instance->lim_sec=0.0;
        instance->err_mod=0;
        instance->auto_nbeams=0;
        instance->verbose=0;
        instance->flags=MB1RS_MODE_AUTO;
        instance->rto_ms=MB1RS_RTO_MS_DFL;
        instance->del_ms=MB1RS_DEL_MS_DFL;
    }
    return instance;
}

void mb1rs_cfg_destroy(mb1rs_cfg_t **pself)
{
    if(NULL!=pself){
        mb1rs_cfg_t *self = (mb1rs_cfg_t *)(*pself);
        if(NULL!=self){
            if(NULL!=self->ifile)
                free(self->ifile);
            if(NULL!=self->host)
                free(self->host);
            free(self);
            *pself=NULL;
        }
    }
}

void mb1rs_cfg_show(mb1rs_cfg_t *self, bool verbose, uint16_t indent)
{
    if (self) {
        int wkey=15;
        int wval=15;
        fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"self",wval,self);
    }
}

mb1rs_ctx_t *mb1rs_dfl_new()
{
    mb1rs_ctx_t *instance = (mb1rs_ctx_t *)malloc(sizeof(mb1rs_ctx_t));

    if(NULL!=instance){
        memset(instance,0,sizeof(mb1rs_ctx_t));
        instance->cfg = mb1rs_cfg_new();
        instance->frame_func = s_auto_frame_fn;
        instance->state=MB1RS_ST_STOPPED;
        instance->stop_req=0;
        instance->cyc_count=0;
        instance->ret_count=0;
        instance->err_count=0;
        instance->tx_count=0;
        instance->tx_bytes=0;
    }
    return instance;
}

mb1rs_ctx_t *mb1rs_new(mb1rs_cfg_t *cfg)
{
    mb1rs_ctx_t *instance= mb1rs_dfl_new();

    if(NULL!=instance){
        if(NULL!=cfg){
            mb1rs_cfg_destroy(&instance->cfg);
            instance->cfg=cfg;
        }
        if(MB1RS_GET_MSK(&instance->cfg->flags,MB1RS_MODE_AUTO)){
            instance->frame_func = s_auto_frame_fn;
        }else{
            instance->frame_func = s_file_frame_fn;
        }
    }
    return instance;
}

void mb1rs_destroy(mb1rs_ctx_t **pself)
{
    if(NULL!=pself){
        mb1rs_ctx_t *self = (mb1rs_ctx_t *)*pself;
        if(NULL!=self){
            if(self->state == MB1RS_ST_RUNNING){
                mb1rs_stop(self);
            }
            if(NULL!=self->worker)
                mthread_thread_destroy(&self->worker);
            mb1rs_cfg_destroy(&self->cfg);
            mfile_file_destroy(&self->rfile);
            mb1rs_cfg_destroy(&self->cfg);
            free(self);
            *pself = NULL;
        }
    }
}


const char *mb1rs_statestr(mb1rs_state_t state)
{
    const char *retval=NULL;
    switch (state) {
        case MB1RS_ST_STOPPED:
        case MB1RS_ST_RUNNING:
            retval=g_state_str[state];
            break;

        default:
            break;
    }
    return retval;
}

void mb1rs_show(mb1rs_ctx_t *self, bool verbose, uint16_t indent)
{
    if (self) {
        int wkey=15;
        int wval=15;
        fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"self",wval,self);
        fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"cfg",wval,self->cfg);
        fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"frame_func",wval,self->frame_func);
        fprintf(stderr,"%*s%*s %*d\n",indent,(indent>0?" ":""),wkey,"stop_req",wval,self->stop_req);
        fprintf(stderr,"%*s%*s %*d/%s\n",indent,(indent>0?" ":""),wkey,"state",wval,self->state,mb1rs_statestr(self->state));
        fprintf(stderr,"%*s%*s %*"PRIu32"\n",indent,(indent>0?" ":""),wkey,"cyc_count",wval,self->cyc_count);
        fprintf(stderr,"%*s%*s %*"PRIu32"\n",indent,(indent>0?" ":""),wkey,"ret_count",wval,self->ret_count);
        fprintf(stderr,"%*s%*s %*"PRIu32"\n",indent,(indent>0?" ":""),wkey,"err_count",wval,self->err_count);
        fprintf(stderr,"%*s%*s %*"PRIu32"\n",indent,(indent>0?" ":""),wkey,"tx_count",wval,self->tx_count);

        if(verbose){
            mb1rs_cfg_show(self->cfg, verbose, indent);
        }
    }else{
        fprintf(stderr,"%*s[self %10p (NULL message)]\n",indent,(indent>0?" ":""), self);

    }
}

int mb1rs_start(mb1rs_ctx_t *self)
{
    int retval=-1;
    if(NULL!=self && self->state!=MB1RS_ST_RUNNING){
        self->worker = mthread_thread_new();
        if(mthread_thread_start(self->worker, s_server_function, (void *)self)==0){
            int i=5;
            while( (self->state != MB1RS_ST_RUNNING) &&
                  (i-- > 0)){
                sleep(1);
            }
            if(self->state == MB1RS_ST_RUNNING){
                retval=0;
            }else{
                fprintf(stderr,"ERR - thread start timed out\n");
            }
        }else{
            fprintf(stderr,"ERR - mthread_thread_start\n");
        }
    }
    return retval;
}

int mb1rs_stop(mb1rs_ctx_t *self)
{
    int retval=-1;
    self->stop_req=1;
    fprintf(stderr,"joining worker\n");
    mthread_thread_join(self->worker);
    int i=5;
    while( (self->state != MB1RS_ST_STOPPED) &&
          (i-- <= 0)){
        sleep(1);
    }
    if(self->state == MB1RS_ST_STOPPED){
        mthread_thread_destroy(&self->worker);
        self->worker=NULL;
        retval=0;
    }else{
        fprintf(stderr,"ERR - thread stop timed out\n");
    }
    return retval;
}


