///
/// @file r7k-reader.c
/// @authors k. Headley
/// @date 01 jan 2018

/// Reson 7KCenter reader API
/// contains r7kr_reader_t, a component that
/// reads reson 7k center multibeam data for use by MBSystem

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

#include "r7k-reader.h"
#include "mxd_app.h"
#include "mxdebug.h"
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

static const char *r7kr_event_labels[]={ \
    "frame_valid",
    "frame_invalid",
    "nf_valid",
    "drf_valid",
    "nf_invalid",
    "drf_invalid",
    "drf_resync",
    "nf_resync",
    "nf_short_read",
    "drf_short_read",
    "e_drf_proto",
    "e_nf_totalrec",
    "e_nf_packetsz",
    "e_nf_offset",
    "e_nf_ver",
    "e_nf_read",
    "e_sock",
    "e_drf_chk",
    "e_drf_time",
    "e_drf_size",
    "e_drf_sync",
    "e_drf_read",
    "e_fc_write",
    "fc_read",
    "fc_refill"
};

static const char *r7kr_status_labels[]={ \
    "frame_valid_bytes",
    "nf_valid_bytes",
    "drf_valid_bytes",
    "nf_inval_bytes",
    "drf_inval_bytes",
    "sub_frames"
};

static const char *r7kr_metric_labels[]={ \
    //    "r7kr_refill_xt",
    "r7kr_7kframe_skew"
};

static const char **r7kr_stats_labels[MSLABEL_COUNT]={
    r7kr_event_labels,
    r7kr_status_labels,
    r7kr_metric_labels
};

/////////////////////////
// Function Definitions
/////////////////////////

///// @fn const char *r7kr_get_version()
///// @brief get version string.
///// @return version string
//const char *r7kr_get_version()
//{
//    return LIBR7KR_VERSION;
//}
//// End function r7kr_get_version
//
///// @fn const char *r7kr_get_build()
///// @brief get build string.
///// @return version string
//const char *r7kr_get_build()
//{
//    return LIBR7KR_BUILD;
//}
//// End function r7kr_get_build
//
///// @fn void r7kr_show_app_version(const char *app_version)
///// @brief get version string.
///// @return version string
//void r7kr_show_app_version(const char *app_name, const char *app_version)
//{
//    printf("\n %s built[%s] libr7krn[v%s / %s] \n\n",app_name, app_version, r7kr_get_version(),r7kr_get_build());
//}
//// End function r7kr_show_app_version

/// @fn int r7kr_reader_connect(*self)
/// @brief connect to 7k center and subscribe to records.
/// @param[in] self reader reference
/// @return 0 on success, -1 otherwise
int r7kr_reader_connect(r7kr_reader_t *self, bool replace_socket)
{
    int retval=-1;
    me_errno = ME_OK;

    if (NULL!=self) {

        char *host = strdup(self->sockif->addr->host);
        int port = self->sockif->addr->port;

        if (replace_socket) {
//            MX_MMSG(R7KR_DEBUG, "destroying socket\n");
            MX_DMSG(R7KR_DEBUG, "destroying socket\n");
            msock_socket_destroy(&self->sockif);

            MX_DMSG(R7KR_DEBUG, "building socket\n");
            self->sockif = msock_socket_new(host,port,ST_TCP);
        }

        MX_MPRINT(R7KR_DEBUG, "connecting to 7k center [%s]\n",self->sockif->addr->host);
        if(msock_connect(self->sockif)==0){
            self->state=R7KR_CONNECTED;
            self->sockif->status=SS_CONNECTED;

//            if(mmd_channel_isset(MOD_R7KR,MM_DEBUG)){
//            MX_DMSG(R7KR_DEBUG, "requesting 7k device config data"));
//            r7k_req_config(r7kr_reader_sockif(self));
//            }

            MX_MPRINT(R7KR_DEBUG, "subscribing to 7k center [%s]\n",self->sockif->addr->host);
            if (r7k_subscribe(r7kr_reader_sockif(self), self->device, self->sub_list, self->sub_count)==0) {
                self->state=R7KR_SUBSCRIBED;
                retval=0;
            }else{
                MX_MPRINT(R7KR_DEBUG, "subscribe failed [%s]\n",self->sockif->addr->host);
                me_errno=ME_ESUB;
                self->state=R7KR_INITIALIZED;
            }
        }else{
            MX_MPRINT(R7KR_DEBUG, "connect failed [%s]\n",self->sockif->addr->host);
            me_errno=ME_ECONNECT;
            self->state=R7KR_INITIALIZED;
            r7kr_reader_reset_socket(self);
        }
    }
    return retval;
}
// End function r7kr_reader_connect

/// @fn r7kr_reader_t * r7kr_reader_new(const char * host, int port, uint32_t capacity, uint32_t * slist, uint32_t slist_len)
/// @brief create new reson 7k reader. connect and subscribe to reson data.
/// @param[in] host reson 7k center host IP address
/// @param[in] port reson 7k center host IP port
/// @param[in] capacity input buffer capacity (bytes)
/// @param[in] slist 7k center message subscription list
/// @param[in] slist_len length of subscription list
/// @return new reson reader reference on success, NULL otherwise
r7kr_reader_t *r7kr_reader_new(r7k_device_t device, const char *host, int port, uint32_t capacity, uint32_t *slist,  uint32_t slist_len)
{
    r7kr_reader_t *self = (r7kr_reader_t *)malloc(sizeof(r7kr_reader_t));
    if (NULL != self) {

        self->state=R7KR_NEW;
        self->fileif=NULL;
        self->sockif = msock_socket_new(host,port,ST_TCP);
        self->sub_list = (uint32_t *)malloc(slist_len*sizeof(uint32_t));
        memset(self->sub_list,0,slist_len*sizeof(uint32_t));
        self->sub_count = slist_len;
        memcpy(self->sub_list,slist,slist_len*sizeof(uint32_t));
        self->fc = r7k_drfcon_new(capacity);
        self->log_id=MLOG_ID_INVALID;
        self->logstream=NULL;
        self->state=R7KR_INITIALIZED;
        self->device=device;

        if (NULL != self->sockif) {
            r7kr_reader_connect(self, false);
            if(me_errno != ME_OK)fprintf(stderr,"connect error (%s)\n",me_strerror(me_errno));
        }else{
            self->state=ME_ECREATE;
        }
        self->stats=mstats_new(R7KR_EV_COUNT, R7KR_STA_COUNT, R7KR_MET_COUNT, r7kr_stats_labels);
    }

    return self;
}
// End function r7kr_reader_create

/// @fn r7kr_reader_t * r7kr_freader_new(mfile_file_t *file, uint32_t capacity, uint32_t * slist, uint32_t slist_len)
/// @brief create new reson 7k reader. connect and subscribe to reson data.
/// @param[in] path input file path
/// @param[in] capacity input buffer capacity (bytes)
/// @param[in] slist 7k center message subscription list
/// @param[in] slist_len length of subscription list
/// @return new reson reader reference on success, NULL otherwise
r7kr_reader_t *r7kr_freader_new(mfile_file_t *file, uint32_t capacity, uint32_t *slist,  uint32_t slist_len)
{
    r7kr_reader_t *self = (r7kr_reader_t *)malloc(sizeof(r7kr_reader_t));
    if (NULL != self) {

        self->state  = R7KR_NEW;
        self->sockif = NULL;
        self->fileif = file;
        if (NULL!=file) {
            if (mfile_open(self->fileif, MFILE_RONLY)<=0) {
                fprintf(stderr,"ERR - could not open file [%s] [%d/%s]\n",self->fileif->path,errno,strerror(errno));
            }

            fprintf(stderr,"wrapping fd %d for file %s in socket\n",self->fileif->fd,self->fileif->path);
            // wrap file descriptor in socket
            // so it can be passed to read
            self->sockif = msock_wrap_fd(self->fileif->fd);
        }

        self->sub_list = (uint32_t *)malloc(slist_len*sizeof(uint32_t));
        memset(self->sub_list,0,slist_len*sizeof(uint32_t));
        self->sub_count = slist_len;
        memcpy(self->sub_list,slist,slist_len*sizeof(uint32_t));
        self->fc = r7k_drfcon_new(capacity);

        self->state=R7KR_INITIALIZED;

        self->stats=mstats_new(R7KR_EV_COUNT, R7KR_STA_COUNT, R7KR_MET_COUNT, r7kr_stats_labels);
    }

    return self;
}
// End function r7kr_reader_create

/// @fn void r7kr_reader_destroy(r7kr_reader_t ** pself)
/// @brief release reason 7k center reader resources.
/// @param[in] pself pointer to instance reference
/// @return none
void r7kr_reader_destroy(r7kr_reader_t **pself)
{
    if (NULL != pself) {
        r7kr_reader_t *self = *pself;
        if (NULL != self) {
            if (self->sub_list) {
                free(self->sub_list);
            }
            if (self->fc) {
                r7k_drfcon_destroy(&self->fc);
            }
            if (self->stats) {
                mstats_destroy(&self->stats);
            }

            free(self);
            *pself = NULL;
        }
    }
}
// End function r7kr_reader_destroy

/// @fn void r7kr_reader_reset_socket(r7kr_reader_t *self)
/// @brief set logger
/// @param[in] self pointer to instance
/// @param[in] log  pointer to log instance
/// @return none
void r7kr_reader_reset_socket(r7kr_reader_t *self)
{
    if (NULL!=self) {
        close(self->sockif->fd);
        self->sockif->fd=-1;
        self->sockif->status=SS_CONFIGURED;
    }
}
// End function r7kr_reader_reset_socket

/// @fn void r7kr_reader_setlog(r7kr_reader_t *self, mlog_t *log, mlog_id_t id, const char *desc)
/// @brief set logger
/// @param[in] self pointer to instance
/// @param[in] log  pointer to log instance
/// @return none
void r7kr_reader_set_log(r7kr_reader_t *self, mlog_id_t id)
{
    if (NULL!=self) {
        if (self->log_id!=MLOG_ID_INVALID) {
            mlog_delete_instance(self->log_id);
        }
        self->log_id=id;
     }
    return;
}
// End function r7kr_reader_set_log

/// @fn void r7kr_reader_set_logstream(r7kr_reader_t *self, FILE *log)
/// @brief set logger
/// @param[in] self pointer to instance
/// @param[in] log  pointer to log instance
/// @return none
void r7kr_reader_set_logstream(r7kr_reader_t *self, FILE *log)
{
    if (NULL!=self) {
        if (self->log_id != MLOG_ID_INVALID) {
            fclose(self->logstream);
        }
        self->logstream=log;
    }
    return;
}
// End function r7kr_reader_set_logstream

/// @fn int r7kr_reader_set_file(r7kr_reader_t *self, mfile_file_t *file)
/// @brief set current reader file
/// @param[in] self pointer to instance
/// @param[in] file  pointer to file
/// @return none
int r7kr_reader_set_file(r7kr_reader_t *self, mfile_file_t *file)
{
    int retval=-1;

    if (NULL!=self && NULL!=file) {
        mfile_close(self->fileif);
        msock_socket_destroy(&self->sockif);
        self->sockif=NULL;
        self->fileif=file;
        if (mfile_open(self->fileif, MFILE_RONLY)>0){
            // wrap file descriptor in socket
            // so it can be passed to read
            self->sockif = msock_wrap_fd(self->fileif->fd);
            retval=0;
        }else{
            MX_ERROR("ERR - could not open file [%s] [%d/%s]\n", self->fileif->path, errno, strerror(errno));
        }
    }
    return retval;
}
// End function r7kr_reader_set_file

/// @fn mstats_t *r7kr_reader_get_stats(r7kr_reader_t *self)
/// @brief get statistics reference for r7kr_reader
/// @param[in] self r7kr_reader reference
/// @return mstats_t reference on success, NULL otherwise
mstats_t *r7kr_reader_get_stats(r7kr_reader_t *self)
{
    mstats_t *retval=NULL;
    if (NULL!=self) {
        retval=self->stats;
    }
    return retval;
}
// End function r7kr_reader_get_stats

/// @fn const char ***r7kr_reader_get_statlabels()
/// @brief get statistics labels
/// @return stats label array reference
const char ***r7kr_reader_get_statlabels()
{
    return r7kr_stats_labels;
}
// End function r7kr_reader_get_statlabels


/// @fn msock_socket_t * r7kr_reader_sockif(r7kr_reader_t * self)
/// @brief get reader socket interface.
/// @param[in] self reader reference
/// @return socket reference on success, NULL otherwise.
msock_socket_t *r7kr_reader_sockif(r7kr_reader_t *self)
{
    return (NULL!=self ? self->sockif : NULL);
}
// End function r7kr_reader_sockif

/// @fn mfile_file_t * r7kr_reader_fileif(r7kr_reader_t * self)
/// @brief get reader file interface.
/// @param[in] self reader reference
/// @return socket reference on success, NULL otherwise.
mfile_file_t *r7kr_reader_fileif(r7kr_reader_t *self)
{
    return (NULL!=self ? self->fileif : NULL);
}
// End function r7kr_reader_sockif

/// @fn void r7kr_reader_show(r7kr_reader_t * self, _Bool verbose, uint16_t indent)
/// @brief output reader parameter summary to stderr.
/// @param[in] self reader reference
/// @param[in] verbose provide verbose output
/// @param[in] indent output indentation (spaces)
/// @return none
void r7kr_reader_show(r7kr_reader_t *self, bool verbose, uint16_t indent)
{
    if (NULL != self) {
        fprintf(stderr,"%*s[self      %10p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[sockif    %10p]\n",indent,(indent>0?" ":""), self->sockif);
        fprintf(stderr,"%*s[fileif    %10p]\n",indent,(indent>0?" ":""), self->fileif);
        fprintf(stderr,"%*s[fc        %10p]\n",indent,(indent>0?" ":""), self->fc);
        // TODO : show drfcontainer
        if (verbose) {
            r7k_drfcon_show(self->fc,false,indent+3);
        }
        fprintf(stderr,"%*s[state    %2d/%s]\n",indent,(indent>0?" ":""), self->state, r7kr_strstate(self->state));
        fprintf(stderr,"%*s[sub_count %10u]\n",indent,(indent>0?" ":""), self->sub_count);
        fprintf(stderr,"%*s[sub_list  %10p]\n",indent,(indent>0?" ":""), self->sub_list);
        if (verbose) {
            for (unsigned int i=0; i<self->sub_count; i++) {
                fprintf(stderr,"%*s[sub[%02u]  %10u]\n",indent+3,(indent+3>0?" ":""),i, self->sub_list[i]);
            }
        }
    }
}
// End function r7kr_reader_show


/// @fn const char * r7kr_strstate(r7kr_state_t state)
/// @brief get string for reader state.
/// @param[in] state state ID
/// @return mnemonic string on success, NULL otherwise
const char *r7kr_strstate(r7kr_state_t state)
{
	const char *retval = "UNDEFINED";
    switch (state) {
        case R7KR_NEW:
            retval = "NEW";
            break;
        case R7KR_INITIALIZED:
            retval = "INITIALIZED";
            break;
        case R7KR_CONNECTED:
            retval = "CONNECTED";
            break;
        case R7KR_SUBSCRIBED:
            retval = "SUBSCRIBED";
            break;

        default:
            break;
    }
    return retval;
}
// End function r7kr_strstate

/// @fn void void r7kr_reader_purge(r7kr_reader_t *self)
/// @brief empty reader frame container.
/// @param[in] self reader reference
/// @return none;
void r7kr_reader_purge(r7kr_reader_t *self)
{
    if (NULL!=self) {
        r7k_drfcon_flush(self->fc);
    }
}
// End function r7kr_reader_purge

/// @fn void r7kr_reader_flush(r7kr_reader_t * self, uint32_t len, int32_t retries, uint32_t tmout_ms)
/// @brief flush reader input buffer.
/// @param[in] self reader reference
/// @param[in] len number of bytes of input to read
/// @param[in] retries number of retries
/// @param[in] tmout_ms timeout
/// @return none; attempts to read len characters at a time
/// until a timeout occurs
void r7kr_reader_flush(r7kr_reader_t *self, uint32_t len, int32_t retries, uint32_t tmout_ms)
{
    if (NULL != self) {
        // read until timeout
        byte buf[len];
        int64_t x=0;
        uint32_t n=0;
        bool use_retries = (retries>0 ? true : false);

        do{
           x=msock_read_tmout(r7kr_reader_sockif(self), buf, len, tmout_ms);
           n++;
            if (use_retries) {
                if (retries-- ==0) {
                    break;
                }
            }
        }while ( (x!=-1) && (me_errno!=(int)ME_ETMOUT));
//        MMDEBUG(R7KR,"EXIT - retries[%d/%s] x[%"PRId64"] e[%d/%s] n[%u]\n",retries,(use_retries?"true":"false"),x,
//               me_errno,me_strerror(me_errno),n);
    }
}
// End function r7kr_reader_flush


// return raw (possibly partial) network data frames

/// @fn int64_t r7kr_reader_poll(r7kr_reader_t * self, byte * dest, uint32_t len, uint32_t tmout_ms)
/// @brief read raw data from reson 7k center socket
/// @param[in] self reader reference
/// @param[in] dest data buffer
/// @param[in] len data request length
/// @param[in] tmout_ms timeout
/// @return returns number of bytes read into buffer on success, -1 otherwise
int64_t r7kr_reader_poll(r7kr_reader_t *self, byte *dest, uint32_t len, uint32_t tmout_ms)
{
    int64_t retval=-1;
    me_errno=ME_OK;

    if (NULL!=self && NULL!=dest) {

        int64_t rbytes=0;

		// may want to call r7kr_reader_flush and usleep before
        // calling poll...


        // read up to len bytes or timeout
        // use len=R7KR_TRN_PING_BYTES and R7KR_TRN_PING_MSEC
        // to get a ping
        if ((rbytes=msock_read_tmout(r7kr_reader_sockif(self), dest, len, tmout_ms)) > 0 ) {

            if ( (me_errno==ME_OK || me_errno==ME_ETMOUT) ) {
                retval = rbytes;

//               MX_MPRINT(R7KR_DEBUG, "buf[%p] req[%d] rd[%"PRId64"] to[%u]\n",dest,len,rbytes,tmout_ms);
            }else{
                // error
                MX_MPRINT(R7KR_DEBUG, "read err1 to[%"PRIu32"] merr[%d/%s] rb[%"PRId64"]\n", tmout_ms, me_errno, me_strerror(me_errno), rbytes);
                retval=-1;
            }
        }else{
            // error
            MX_MPRINT(R7KR_DEBUG, "read err2 to[%"PRIu32"] merr[%d/%s] rb[%"PRId64"]\n", tmout_ms, me_errno, me_strerror(me_errno), rbytes);
            retval=-1;
        }
    }else{
//        PEPRINT((stderr,"invalid argument\n"));
        MX_ERROR_MSG("invalid argument\n");
    }

    return retval;
}
// End function r7kr_reader_poll


/// @fn int64_t r7kr_read_nf(r7kr_reader_t *self, byte *dest, uint32_t len, r7kr_flags_t flags, double newer_than, uint32_t timeout_msec, uint32_t *sync_bytes)
/// @brief read a network frame from a file or socket.
/// @param[in] self reader reference
/// @param[in] dest data buffer
/// @param[in] flags option flags
/// @param[in] newer_than reject packets older than this (epoch time, decimal seconds)
/// @param[in] timeout_msec read timeout
/// @param[out] sync_bytes number of bytes skipped
/// @return number of bytes returned on success, -1 otherwise (me_errno set)
int64_t r7kr_read_nf(r7kr_reader_t *self, byte *dest, uint32_t len,
                           r7kr_flags_t flags,
                           double newer_than,
                           uint32_t timeout_msec,
                           uint32_t *sync_bytes)
{
    int64_t retval=-1;
    me_errno = ME_EINVAL;

    if (NULL!=self && NULL!=r7kr_reader_sockif(self) && NULL!=dest ) {

        uint32_t read_len   = 0;
        int64_t read_bytes  = 0;
        int64_t frame_bytes = 0;
        int64_t lost_bytes  = 0;
        byte *pbuf          = dest;
        byte *psync         = NULL;
        r7k_nf_t *pnf     = NULL;
        r7k_nf_t *pframe   = NULL;
        bool sync_found     = false;

        r7kr_parse_state_t state = R7KR_STATE_START;
        r7kr_parse_action_t action = R7KR_ACTION_QUIT;
        bool header_pending=true;
        uint32_t pending_bytes = 0;

        // read and validate header
        while (state != R7KR_STATE_NF_VALID ) {

            if(errno==EINTR){
                // quit on user interrupt
                break;
            }

            switch (state) {

                case R7KR_STATE_START:
//                    MX_DMSG(R7KR_DEBUG, "R7KR_STATE_START\n"));
                    read_len       = R7K_NF_BYTES;
                    pbuf           = dest;
                    header_pending = true;
                    frame_bytes    = 0;
                    action         = R7KR_ACTION_READ;
                    memset(dest,0,len);
                    break;

                case R7KR_STATE_READING:
//                   MX_DMSG(R7KR_DEBUG, "R7KR_STATE_READING\n"));
                    // don't touch inputs
                    action   = R7KR_ACTION_READ;
                    break;

                case R7KR_STATE_READ_OK:
                    pnf   = (r7k_nf_t *)dest;
                    if (header_pending) {
//                        MX_MMSG(R7KR_DEBUG, "R7KR_STATE_READ_OK header\n"));
                        header_pending=false;
                        action = R7KR_ACTION_VALIDATE_HEADER;
                    }
                    break;

                case R7KR_STATE_HEADER_VALID:
                    state = R7KR_STATE_NF_VALID;
                    action = R7KR_ACTION_QUIT;
//                    MX_DMSG(R7KR_DEBUG, "R7KR_STATE_HEADER_VALID/NF_VALID\n"));
                    break;

                case R7KR_STATE_NF_INVALID:
                    MX_LMSG(R7KR, 2, "R7KR_STATE_NF_INVALID\n");
                    if ( (flags&R7KR_RESYNC_NF)!=0 ) {
                        // pbuf points to end of input...
                        // psync points 1 byte into invalid frame
                        MX_DMSG(R7KR_DEBUG, ">>>>> RESYNC: NRF buffer:\n");
                        if(mxd_testModule(R7KR_DEBUG, 1)){
                            r7k_hex_show(dest,R7K_NF_BYTES,16,true,5);
                        }
                        MX_MPRINT(R7KR_DEBUG, "dest[%p] pbuf[%p] ofs[%"PRId32"]\n", dest, pbuf, (int32_t)(pbuf-dest));
                        MX_MPRINT(R7KR_DEBUG, "read_len[%"PRIu32"]\n", read_len);
                        MX_MPRINT(R7KR_DEBUG, "frame_bytes[%"PRId64"]\n", frame_bytes);
                        MX_MPRINT(R7KR_DEBUG, "lost_bytes[%"PRId64"]\n", lost_bytes);

                        psync = dest+1;
                        lost_bytes+=1;
                        sync_found = false;
                        pframe = NULL;
                        action = R7KR_ACTION_RESYNC;
                    }else{
                        // don't resync
                        action = R7KR_ACTION_QUIT;
                    }
                    break;

                case R7KR_STATE_READ_ERR:
                    MX_LMSG(R7KR, 2, "R7KR_STATE_READ_ERR\n");
                    if (me_errno==ME_ESOCK) {
                        MX_MMSG(R7KR_DEBUG, "socket disconnected - quitting\n");
                    }else if (me_errno==ME_EOF) {
                        MX_MMSG(R7KR_ERROR, "end of file\n");
                    }else if (me_errno==ME_ENOSPACE) {
                        MX_MPRINT(R7KR_ERROR, "buffer full [%"PRIu32"/%"PRIu32"]\n", (uint32_t)(pbuf+read_len-dest), len);
                    }else{
                        MX_MPRINT(R7KR_DEBUG, "read error [%d/%s]\n", me_errno, me_strerror(me_errno));
                    }
                    action   = R7KR_ACTION_QUIT;
                    break;

                default:
                    MX_MPRINT(R7KR_ERROR, "ERR - unknown state[%d]\n",state);
                    break;
            }

//            MX_MPRINT(R7KR_DEBUG, ">>> ACTION=[%d]\n", action);

            if (action==R7KR_ACTION_READ) {
                // inputs: pbuf, read_len
//                MX_MPRINT(R7KR_DEBUG, "R7KR_ACTION_READ rlen[%"PRIu32"]\n", read_len);
                read_bytes=0;
                while (read_bytes<read_len) {
                    if ( (read_bytes=msock_read_tmout(r7kr_reader_sockif(self), pbuf, read_len, timeout_msec)) == read_len) {
//                        MX_MPRINT(R7KR_DEBUG, "read OK [%"PRId64"]\n", read_bytes));

                        pnf = (r7k_nf_t *)dest;
                        pbuf += read_bytes;
                        frame_bytes += read_bytes;
                        state=R7KR_STATE_READ_OK;

                    }else{
                        MST_COUNTER_INC(self->stats->events[R7KR_EV_NF_SHORT_READ]);
                        if (read_bytes>=0) {
                            // short read, keep going
                            read_len -= read_bytes;
                            pbuf += read_bytes;
                            frame_bytes += read_bytes;

                            if (me_errno==ME_ESOCK || me_errno==ME_EOF) {
                                state=R7KR_STATE_READ_ERR;
                                MST_COUNTER_INC(self->stats->events[R7KR_EV_ESOCK]);
                                break;
                            }
                        }else{
                            // error
                            state=R7KR_STATE_READ_ERR;
                            // let errorno bubble up
                            MST_COUNTER_INC(self->stats->events[R7KR_EV_ENFREAD]);
                            break;
                        }
                   }// else incomplete read
                    if(errno==EINTR){
                        // quit on user interrupt
                        break;
                    }

                    if ( state!=R7KR_STATE_READ_OK && ((pbuf+read_len-dest) > (int32_t)len) ) {
                        state=R7KR_STATE_READ_ERR;
                        me_errno=ME_ENOSPACE;
                        break;
                    }
                }
            }

            if (action==R7KR_ACTION_VALIDATE_HEADER) {
//                MX_MMSG(R7KR_DEBUG, "R7KR_ACTION_VALIDATE_HEADER\n");
                // inputs: pnf
                state=R7KR_STATE_NF_INVALID;
                if (pnf->protocol_version == R7K_NF_PROTO_VER) {
//                   MX_MMSG(R7KR_DEBUG, "nf version valid\n");
                    if (pnf->offset >= (R7K_NF_BYTES)) {
//                        MX_MMSG(R7KR_DEBUG, "nf offset valid\n");
                        if (pnf->packet_size == (pnf->total_size+R7K_NF_BYTES)) {
//                            MX_MPRINT(R7KR_DEBUG, "nf packet_size valid [%"PRIu32"]\n", pnf->packet_size);
                            if (pnf->total_records == 1) {
//                                MX_MMSG(R7KR_DEBUG, "nf total_records valid\n"));
                                state = R7KR_STATE_HEADER_VALID;
                            }else{
                                MX_MPRINT(R7KR_DEBUG, "INFO - nf total_records invalid[%"PRIu16"/%"PRIu16"]\n", pnf->total_records, (uint16_t)1);
                                MST_COUNTER_INC(self->stats->events[R7KR_EV_ENFTOTALREC]);
                            }
                        }else{
                            MX_MPRINT(R7KR_DEBUG, "INFO - nf packet_size invalid[%"PRIu32"/%"PRIu32"]\n", pnf->packet_size, (pnf->total_size+R7K_NF_BYTES));
                            MST_COUNTER_INC(self->stats->events[R7KR_EV_ENFPACKETSZ]);
                        }
                    }else{
                        MX_MPRINT(R7KR_DEBUG, "INFO - nf offset invalid [%"PRIu16"/%"PRIu16"]\n", pnf->offset, (uint16_t)R7K_NF_BYTES);
                        MST_COUNTER_INC(self->stats->events[R7KR_EV_ENFOFFSET]);
                    }
                }else{
                    MX_MPRINT(R7KR_DEBUG, "INFO - nf proto_version invalid [%"PRIu16"/%"PRIu16"]\n", pnf->protocol_version, (uint16_t)R7K_NF_PROTO_VER);
                    MST_COUNTER_INC(self->stats->events[R7KR_EV_ENFVER]);
                }
            }

            if (action==R7KR_ACTION_RESYNC) {
                // input : psync points to start of search
                // input : pbuf points to end of input
                // input : sync_found false
                // input : pframe NULL

                MST_COUNTER_INC(self->stats->events[R7KR_EV_NF_RESYNC]);

                if( psync >= (pbuf-R7K_NF_PROTO_BYTES)){
                    MX_MMSG(R7KR_DEBUG, "WARN - pending bytes > found frame\n");
                }

                // look until end of buffer (or sync found)
                while (  (psync < (pbuf-R7K_NF_PROTO_BYTES)) && (sync_found==false) ) {

                    MX_LPRINT(R7KR, 2, "psync[%p] ofs[%"PRId32"]\n",psync,(int32_t)(psync-dest));
                   pframe = (r7k_nf_t *)psync;
                    // match only proto version...
                    // we'll be back if other parts are invalid, and
                    // minimal matching means we can search to the
                    // end of the buffer (don't need to access other nf
                    // members that could be outside the buffer)
                    if (pframe->protocol_version == R7K_NF_PROTO_VER ) {

                        pending_bytes = (pbuf-psync);

                        // finish reading in header
                        // and continue

                        // move the bytes we have to the
                        // start of the buffer
                        memmove(dest, psync, pending_bytes);

                        // clean up buffer
                        memset((dest+pending_bytes), 0, (len-pending_bytes));

                        // configure state to resume header read
                        pbuf           = dest+pending_bytes;
                        read_bytes     = 0;
                        read_len       = R7K_NF_BYTES-pending_bytes;
                        frame_bytes    = pending_bytes;
                        header_pending = true;
                        state          = R7KR_STATE_READING;

                        // return to the state machine
                        sync_found     = true;
                        MX_LMSG(R7KR, 2, "sync found\n");
                        MX_LPRINT(R7KR, 2, "dest[%p] pbuf[%p] ofs[%"PRId32"]\n",dest,pbuf,(int32_t)(pbuf-dest));
                        MX_LPRINT(R7KR, 2, "psync[%p] ofs[%"PRId32"]\n",psync,(int32_t)(psync-dest));
                        MX_LPRINT(R7KR, 2, "pending_bytes[%"PRIu32"]\n",pending_bytes);
                        MX_LPRINT(R7KR, 2, "read_len[%"PRIu32"]\n",read_len);
                        MX_LPRINT(R7KR, 2, "frame_bytes[%"PRId64"]\n",frame_bytes);
                        MX_LPRINT(R7KR, 2, "frame_bytes+read_len[%"PRId64"]\n",(int64_t)(frame_bytes+read_len));
                        MX_LPRINT(R7KR, 2, "lost_bytes[%"PRId64"]\n",lost_bytes);
                        //r7k_hex_show(dest,len,16,true,5);

                        break;

                    }else{
                        // protocol version not valid
                        // shift one byte and try again
                        psync++;
                        lost_bytes++;
                    }
                }// while

                if (sync_found==false) {
                    // proto_version not found - restart
                    lost_bytes+=(pbuf-psync);
                    MX_LPRINT(R7KR, 2, "nf proto_ver not found - restart lost_bytes[%"PRId64"]\n", lost_bytes);
                    state=R7KR_STATE_START;
                    MX_LPRINT(R7KR, 2, "dest[%p] pbuf[%p] ofs[%"PRId32"]\n", dest, pbuf, (int32_t)(pbuf-dest));
                    MX_LPRINT(R7KR, 2, "psync[%p] ofs[%"PRId32"]\n", psync, (int32_t)(psync-dest));
                    MX_LPRINT(R7KR, 2, "pending_bytes[%"PRIu32"]\n", pending_bytes);
                    MX_LPRINT(R7KR, 2, "read_len[%"PRIu32"]\n", read_len);
                    MX_LPRINT(R7KR, 2, "frame_bytes[%"PRId64"]\n", frame_bytes);
                    MX_LPRINT(R7KR, 2, "frame_bytes+read_len[%"PRId64"]\n", (frame_bytes+read_len));
                    MX_LPRINT(R7KR, 2, "lost_bytes[%"PRId64"]\n", lost_bytes);
                }
            }

            if (action==R7KR_ACTION_QUIT) {

//                MX_MMSG(R7KR_DEBUG, "ACTION_QUIT\n"));
                if (state==R7KR_STATE_NF_VALID) {
                    retval = frame_bytes;
                    MX_LPRINT(R7KR, 2, "NF valid - returning[%"PRId64"] lost[%"PRId64"]\n", retval, lost_bytes);
                    MST_COUNTER_INC(self->stats->events[R7KR_EV_NF_VALID]);
                    MST_COUNTER_ADD(self->stats->status[R7KR_STA_NF_VAL_BYTES],frame_bytes);
                }else{
                    MX_LPRINT(R7KR, 2, "NF invalid - returning[%"PRId64"] lost[%"PRId64"]\n", retval, lost_bytes);
                    MST_COUNTER_INC(self->stats->events[R7KR_EV_NF_INVALID]);
                    MST_COUNTER_ADD(self->stats->status[R7KR_STA_NF_INVAL_BYTES],lost_bytes);
                }
                if (NULL!=sync_bytes) {
                    (*sync_bytes) += lost_bytes;
                }

                // quit and return
                break;
            }
        } // while frame invalid
    }

    return retval;
}// End function r7kr_read_nf


/// @fn int64_t r7kr_read_drf(r7kr_reader_t *self, byte *dest, uint32_t len, r7kr_flags_t flags, double newer_than, uint32_t timeout_msec, uint32_t *sync_bytes)
/// @brief read a data record frame from a file or socket.
/// @param[in] self reader reference
/// @param[in] dest data buffer
/// @param[in] flags option flags
/// @param[in] newer_than reject packets older than this (decimal seconds)
/// @param[in] timeout_msec read timeout
/// @param[out] sync_bytes number of bytes skipped
/// @return number of bytes returned on success, -1 otherwise (me_errno set)
int64_t r7kr_read_drf(r7kr_reader_t *self, byte *dest, uint32_t len,
                           r7kr_flags_t flags,
                           double newer_than,
                           uint32_t timeout_msec,
                           uint32_t *sync_bytes)
{
    int64_t retval=-1;
    me_errno = ME_EINVAL;

    if (NULL!=self && NULL!=r7kr_reader_sockif(self) && NULL!=dest ) {

        uint32_t read_len   = 0;
        int64_t read_bytes  = 0;
        int64_t frame_bytes = 0;
        int64_t lost_bytes  = 0;
        byte *pbuf          = dest;
        byte *psync         = NULL;
        r7k_drf_t *pdrf     = NULL;
        r7k_drf_t *pframe   = NULL;
        bool sync_found     = false;

        // read and validate header
        r7kr_parse_state_t state = R7KR_STATE_START;
        r7kr_parse_action_t action = R7KR_ACTION_QUIT;
        bool header_pending=true;
        bool data_pending=true;

        while (state != R7KR_STATE_DRF_VALID ) {

            switch (state) {

                case R7KR_STATE_START:
//                    MX_MMSG(R7KR_DEBUG, "R7KR_STATE_START\n");
                   read_len = R7K_DRF_BYTES;
                    pbuf     = dest;
                    memset(dest,0,len);
                    header_pending=true;
                    data_pending=true;
                    frame_bytes=0;
                    action   = R7KR_ACTION_READ;
                    break;

                case R7KR_STATE_READING:
//                   MX_MMSG(R7KR_DEBUG, "R7KR_STATE_READING\n");
                    // don't touch inputs
                    action   = R7KR_ACTION_READ;
                    break;

                case R7KR_STATE_READ_OK:
                    pdrf   = (r7k_drf_t *)dest;
                    if (header_pending) {
//                        MX_MMSG(R7KR_DEBUG, "R7KR_STATE_READ_OK header\n");
                        header_pending=false;
                        action = R7KR_ACTION_VALIDATE_HEADER;
                    }else if (data_pending) {
//                       MX_MMSG(R7KR_DEBUG, "R7KR_STATE_READ_OK data\n");
                        data_pending=false;
                        action = R7KR_ACTION_VALIDATE_CHECKSUM;
                    }
                    break;

                case R7KR_STATE_HEADER_VALID:
                    data_pending=true;
                    read_len = pdrf->size-R7K_DRF_BYTES;
                    action = R7KR_ACTION_READ;
//                    MX_MPRINT(R7KR_DEBUG, "R7KR_STATE_HEADER_VALID data read_len[%"PRIu32"]\n", read_len);
                    break;

                case R7KR_STATE_CHECKSUM_VALID:
//                    MX_MMSG(R7KR_DEBUG, "R7KR_STATE_CHECKSUM_VALID\n");
                    action = R7KR_ACTION_VALIDATE_TIMESTAMP;
                    break;

                case R7KR_STATE_TIMESTAMP_VALID:
//                    MX_MMSG(R7KR_DEBUG, "R7KR_STATE_TIMESTAMP_VALID\n");
                    state = R7KR_STATE_DRF_VALID;
                    action = R7KR_ACTION_QUIT;
                    break;

                case R7KR_STATE_DRF_REJECTED:
                    MX_MMSG(R7KR_DEBUG, "R7KR_STATE_DRF_REJECTED\n");
                    // start over with new frame
                    state = R7KR_STATE_START;
                    break;

                case R7KR_STATE_DRF_INVALID:
                    MX_MMSG(R7KR_DEBUG, "R7KR_STATE_DRF_INVALID\n");
                    if ( (flags&R7KR_RESYNC_DRF)!=0 ) {
                        // pbuf points to end of input...
                        // psync points 1 byte into invalid frame
                        psync = dest+1;
                        lost_bytes+=1;
                        sync_found = false;
                        pframe = NULL;
                        action = R7KR_ACTION_RESYNC;
                    }else{
                    	// don't resync
                        action = R7KR_ACTION_QUIT;
                   }
                    break;

                case R7KR_STATE_READ_ERR:
                    MX_MMSG(R7KR_DEBUG, "R7KR_STATE_READ_ERR\n");
                    if (me_errno==ME_ESOCK) {
                        MX_MMSG(R7KR_DEBUG, "socket disconnected - quitting\n");
                    }else if (me_errno==ME_EOF) {
                        MX_MMSG(R7KR_ERROR, "end of file\n");
                    }else if (me_errno==ME_ENOSPACE) {
                        MX_MPRINT(R7KR_ERROR, "buffer full [%"PRIu32"/%"PRIu32"]\n", (uint32_t)(pbuf+read_len-dest), len);
                    }else{
                        MX_MPRINT(R7KR_DEBUG, "read error [%d/%s]\n", me_errno, me_strerror(me_errno));
                    }
                    action   = R7KR_ACTION_QUIT;
                    break;

                default:
                    MX_MPRINT(R7KR_ERROR, "ERR - unknown state[%d]\n", state);
                    break;
            }

//            MX_MPRINT(R7KR_DEBUG, ">>> ACTION=[%d]\n",action);

            if (action==R7KR_ACTION_READ) {
                // inputs: pbuf, read_len
//                MX_MPRINT(R7KR_DEBUG, "R7KR_ACTION_READ rlen[%"PRIu32"]\n", read_len);
                read_bytes=0;
                while (read_bytes<read_len) {
                    if ( (read_bytes=msock_read_tmout(r7kr_reader_sockif(self), pbuf, read_len, timeout_msec)) == read_len) {
//                        MX_MPRINT(R7KR_DEBUG, "read OK [%"PRId64"]\n",read_bytes);

                        pdrf = (r7k_drf_t *)dest;
                        pbuf += read_bytes;
                        frame_bytes += read_bytes;
                        state=R7KR_STATE_READ_OK;

                    }else{
                        MST_COUNTER_INC(self->stats->events[R7KR_EV_DRF_SHORT_READ]);
                        if (read_bytes>=0) {
                            // short read, keep going
                            read_len -= read_bytes;
                            pbuf += read_bytes;
                            frame_bytes += read_bytes;

                            if (me_errno==ME_ESOCK || me_errno==ME_EOF) {
                                state=R7KR_STATE_READ_ERR;
                                MST_COUNTER_INC(self->stats->events[R7KR_EV_ESOCK]);
                                break;
                            }
                        }else{
                            // error
                            state=R7KR_STATE_READ_ERR;
                            // let errorno bubble up
                            MST_COUNTER_INC(self->stats->events[R7KR_EV_EDRFREAD]);
                            break;
                        }
                    }// else incomplete read

                    if ( state!=R7KR_STATE_READ_OK && (pbuf+read_len-dest) > (int32_t)len ) {
                        state=R7KR_STATE_READ_ERR;
                        me_errno=ME_ENOSPACE;
                        break;
                    }
                }
            }

            if (action==R7KR_ACTION_VALIDATE_HEADER) {
//                MX_MMSG(R7KR_DEBUG, "R7KR_ACTION_VALIDATE_HEADER\n");
                // inputs: pdrf
                state=R7KR_STATE_DRF_INVALID;
                if (pdrf->protocol_version == R7K_DRF_PROTO_VER) {
//                    MX_MPRINT(R7KR_DEBUG, "drf protocol version valid [0x%0X]\n",pdrf->protocol_version);
                    if (pdrf->sync_pattern == R7K_DRF_SYNC_PATTERN) {
//                       MX_MPRINT(R7KR_DEBUG, "drf sync_pattern valid [0x%0X]\n", pdrf->sync_pattern);
                        if (pdrf->size <= R7K_MAX_FRAME_BYTES) {
//                            MX_MPRINT(R7KR_DEBUG, "drf size < max frame valid [0x%"PRIu32"]\n", pdrf->size);

                            // conditionally validate
                            // (pending optional nf size)
                            state=R7KR_STATE_HEADER_VALID;
//
                        }else{
                            MX_MPRINT(R7KR_DEBUG, "INFO - drf size > max frame invalid [%"PRIu32"]\n", pdrf->size);
                            MST_COUNTER_INC(self->stats->events[R7KR_EV_EDRFSIZE]);
                        }
                    }else{
                        MX_MPRINT(R7KR_DEBUG, "INFO - drf sync pattern invalid [0x%0X/0x%0X]\n", pdrf->sync_pattern, R7K_DRF_SYNC_PATTERN);
                        MST_COUNTER_INC(self->stats->events[R7KR_EV_EDRFSYNC]);
                    }
                }else{
                    MX_MPRINT(R7KR_DEBUG, "INFO - drf protocol version invalid [0x%0X/0x%0X]\n", pdrf->protocol_version, R7K_DRF_PROTO_VER);
                    MST_COUNTER_INC(self->stats->events[R7KR_EV_EDRFPROTO]);
                }
            }

            if (action==R7KR_ACTION_VALIDATE_CHECKSUM) {
                // inputs pdrf
                state=R7KR_STATE_DRF_INVALID;

                // validate checksum
                // only if flag set
                if ( (pdrf->flags&0x1)==0 ){
                    // skip checksum constraint
//                    MX_MMSG(R7KR_DEBUG, "drf chksum valid (unchecked)\n");
                    state=R7KR_STATE_CHECKSUM_VALID;
                }else{
                    // buffer/checksum boundary checked on entry
                    byte *pd=(byte *)pdrf;
                    uint32_t vchk = r7k_checksum( pd, (uint32_t)(pdrf->size-R7K_CHECKSUM_BYTES));
                    pd = (byte *)pdrf;
                    pd += ((size_t)pdrf->size-R7K_CHECKSUM_BYTES);
                    uint32_t *pchk = (uint32_t *)pd;

                    if (vchk == (uint32_t)(*pchk) ) {
//                        MX_MPRINT(R7KR_DEBUG, "drf chksum valid [0x%08X]\n", vchk);
                        state=R7KR_STATE_CHECKSUM_VALID;
                    }else{
                        MX_MPRINT(R7KR_DEBUG, "INFO - drf chksum invalid [0x%08X/0x%08X]\n", vchk,  (uint32_t)(*pchk));
                        MST_COUNTER_INC(self->stats->events[R7KR_EV_EDRFCHK]);
                    }
                }
            }

            if (action==R7KR_ACTION_VALIDATE_TIMESTAMP) {
                // inputs pdrf
                state=R7KR_STATE_DRF_REJECTED;
                // validate timestamp
                // optional, per newer_than value
                if (newer_than<=0.0){
                    // skip timestamp constraint
//                    MX_MMSG(R7KR_DEBUG, "drf time valid (unchecked)\n"));
                    state=R7KR_STATE_TIMESTAMP_VALID;
                }else{
#if 0 // TODO: switch after nov2019 cruises
                    struct tm tm7k = {0};
                    tm7k.tm_year = pdrf->_7ktime.year-1900;
                    tm7k.tm_yday = pdrf->_7ktime.day+1;
                    tm7k.tm_hour = pdrf->_7ktime.hours;
                    tm7k.tm_min  = pdrf->_7ktime.minutes;
                    tm7k.tm_sec  = pdrf->_7ktime.seconds;
                    time_t tt7k  = mktime(&tm7k);
                    time_t ttnow = time(NULL);
                    // is this conversion needed if system time is UTC?
                    struct tm tmnow;
                    gmtime_r(&ttnow,&tmnow);
                    ttnow = mktime(&tmnow);

                    if(tt7k>=0 && ( (ttnow-tt7k)<newer_than ) ){
                        //                       MX_MMSG(R7KR_DEBUG, "drf time valid\n");
                        state=R7KR_STATE_TIMESTAMP_VALID;
                    }else{
                        MX_MPRINT(R7KR_DEBUG, "INFO - drf time invalid (stale) [%lu/%lu/%.3lf]\n", (unsigned long)tt7k, (unsigned long)ttnow, newer_than);
                        MST_COUNTER_INC(self->stats->events[R7KR_EV_EDRFTIME]);
                    }
#endif

                    // TODO: this won't compare to epoch time; use struct tm
                    double dtime = 0.0;
                    dtime += pdrf->_7ktime.day*SEC_PER_DAY;
                    dtime += pdrf->_7ktime.hours*SEC_PER_HOUR;
                    dtime += pdrf->_7ktime.minutes*SEC_PER_MIN;
                    dtime += pdrf->_7ktime.seconds;
                    if (dtime>newer_than) {
//                        MX_MMSG(R7KR_DEBUG, "drf time valid\n");
                        state=R7KR_STATE_TIMESTAMP_VALID;
                    }else{
                        MX_MPRINT(R7KR_DEBUG, "INFO - drf time invalid (stale) [%.4lf/%.4lf]\n", dtime, newer_than);
                        MST_COUNTER_INC(self->stats->events[R7KR_EV_EDRFTIME]);
                    }
                }
            }

            if (action==R7KR_ACTION_RESYNC) {
                // input : psync points to start of search
                // input : pbuf points to end of input
                // input : sync_found false
                // input : pframe NULL

                MST_COUNTER_INC(self->stats->events[R7KR_EV_DRF_RESYNC]);

                if( psync >= (pbuf-R7K_DRF_PROTO_BYTES)){
                    MX_MMSG(R7KR_DEBUG, "WARN - pending bytes > found frame\n");
                }

                // look until end of buffer (or sync found)
                while (  (psync < (pbuf-R7K_DRF_PROTO_BYTES)) && (sync_found==false) ) {

                    pframe = (r7k_drf_t *)psync;
                    // match only proto version...
                    // we'll be back if other parts are invalid, and
                    // minimal matching means we can search to the
                    // end of the buffer (don't need to access other drf
                    // members that could be outside the buffer)
                    if (pframe->protocol_version == R7K_DRF_PROTO_VER ) {

                        uint32_t pending_bytes = (pbuf-psync);

                        // have read more bytes than a DRF header...
                        if (pending_bytes>R7K_DRF_BYTES) {

                            // the header should be valid...
                            if (pframe->size<=R7K_MAX_FRAME_BYTES &&
                                pframe->sync_pattern == R7K_DRF_SYNC_PATTERN) {
                                uint32_t completion_bytes =0;
                                if(pending_bytes <= pframe->size){

                                    completion_bytes = (pframe->size - pending_bytes);
                                    // move the bytes we have to the
                                    // start of the buffer
                                    memmove(dest, psync, pending_bytes);

									// clean up buffer
                                    memset((dest+pending_bytes), 0, (len-pending_bytes));

                                    // configure state to continue reading data
                                    pbuf           = dest+pending_bytes;
                                    read_len       = completion_bytes;
                                    header_pending = false;
                                    data_pending   = true;
                                    state          = R7KR_STATE_READING;

                                    // return to the state machine
                                    sync_found     = true;
                                    break;
                                }else{
                                    // else the frame found appears to be smaller than
                                    // what's already read, i.e. there are leftover bytes
                                    // that will cause the next frame to be out of sync.
                                    // options:
                                    // - discard it all and read until we have a frame with no leftover bytes
                                    // - return to the state machine with this frame

                                    completion_bytes = (pending_bytes-pframe->size);
                                    // move the bytes we have to the
                                    // start of the buffer
                                    memmove(dest, psync, pframe->size);
                                    pframe = (r7k_drf_t *)dest;

                                    // clean up buffer
                                    memset((dest+pframe->size), 0, (len-pframe->size));

                                    // return the frame:
                                    frame_bytes = pframe->size;
                                    lost_bytes += (pending_bytes-pframe->size);

                                    // configure state to resume after data read
                                    pbuf           = dest+pframe->size;
                                    read_len       = 0;
                                    header_pending = false;
                                    data_pending   = true;
                                    state          = R7KR_STATE_READ_OK;

                                    // return to the state machine
                                    sync_found     = true;
                                    break;
                                }

                            }else{
                                // invalid after all
                                // shift one byte and try again
                                psync++;
                                lost_bytes++;
                            }

                        }else{
                            // finish reading in header
                            // and continue

                            // move the bytes we have to the
                            // start of the buffer
                            memmove(dest, psync, pending_bytes);

                            // clean up buffer
                            memset((dest+pending_bytes), 0, (len-pending_bytes));

                            // configure state to resume header read
                            pbuf           = dest+pending_bytes;
                            read_bytes     = 0;
                            read_len       = R7K_DRF_BYTES-pending_bytes;
                            frame_bytes    = pending_bytes;
                            header_pending = true;
                            data_pending   = true;
                            state          = R7KR_STATE_READING;

                            // return to the state machine
                            sync_found     = true;
                            break;
                       }
                    }else{
                        // protocol version not valid
                        // shift one byte and try again
                        psync++;
                        lost_bytes++;
                    }
                }// while

                if (sync_found==false) {
                    // proto_version not found - restart
                    MX_MMSG(R7KR_DEBUG, "INFO - drf proto_ver not found - restart\n");
                    state=R7KR_STATE_START;
                    lost_bytes+=(pbuf-psync);
                }
            }

            if (action==R7KR_ACTION_QUIT) {

//                MX_MMSG(R7KR_DEBUG, "ACTION_QUIT\n");
                if (state==R7KR_STATE_DRF_VALID) {
                    retval = frame_bytes;

                    MX_LPRINT(R7KR, 2, "DRF valid - returning[%"PRId64"] lost[%"PRId64"] type[%"PRIu32"]\n", retval, lost_bytes, pdrf->record_type_id);
                    MST_COUNTER_INC(self->stats->events[R7KR_EV_DRF_VALID]);
                    MST_COUNTER_ADD(self->stats->status[R7KR_STA_DRF_VAL_BYTES],frame_bytes);
#ifdef R7KR_TIMING
#if defined(__CYGWIN__)
                    FILETIME ft, epoch_ft;
                    SYSTEMTIME epoch_st = {1970,1, 0, 1, 0, 0, 0, 0};
                    ULARGE_INTEGER a, b;
                    GetSystemTimeAsFileTime(&ft);
                    SystemTimeToFileTime(&epoch_st, &epoch_ft);
                    memcpy(&a, &ft, sizeof(ULARGE_INTEGER));
                    memcpy(&b, &epoch_ft, sizeof(ULARGE_INTEGER));
                    ULARGE_INTEGER c;
                    c.QuadPart = a.QuadPart - b.QuadPart;

                    double stime = (c.QuadPart/(double)(1.e7));

                    r7k_time_t *t7k = &pdrf->_7ktime;
                    double pti=0.0;
                    double ptf=modf(t7k->seconds, &pti);
                    struct tm tms = {0};
                    char tstr[64]={0};
                    sprintf(tstr,"%u %u %02d:%02d:%02.0f",t7k->year,t7k->day,t7k->hours,t7k->minutes,pti);

                    strptime(tstr,"%Y %j %H:%M:%S",&tms);
                    tms.tm_isdst=-1;
                    time_t tt = mktime(&tms);
                    double ptime=(double)tt+ptf;

//                    struct timespec res;
//                    clock_getres(CLOCK_MONOTONIC, &res);
//                    fprintf(stderr,"%11.5lf cskew %+0.4e\n",mtime_dtime(),(double)(stime-ptime));
                    MST_METRIC_SET(self->stats->measurements[R7KR_CH_7KFRAME_SKEW],(stime-ptime));

#else
                    struct timeval stv={0};
                    gettimeofday(&stv,NULL);
                    double stime = (double)stv.tv_sec+((double)stv.tv_usec/1000000.0)+(7*3600);

                    r7k_time_t *t7k = &pdrf->_7ktime;
                    double pti=0.0;
                    double ptf=modf(t7k->seconds, &pti);
                    struct tm tms = {0};
                    char tstr[64]={0};
                    sprintf(tstr,"%u %u %02d:%02d:%02.0f",t7k->year,t7k->day,t7k->hours,t7k->minutes,pti);

                    strptime(tstr,"%Y %j %H:%M:%S",&tms);
                    tms.tm_isdst=-1;
                    time_t tt = mktime(&tms);
                    double ptime=ptf+(double)tt;
                    //                fprintf(stderr,"lrdfr : [%s] ptime[%.3lf] stime[%.3lf] (s-p)[%+6.3lf]\n",tstr,ptime,stime,(double)(stime-ptime));
//                    struct timespec res;
//                    clock_getres(CLOCK_MONOTONIC, &res);
//                    fprintf(stderr,"%11.5lf lskew %+0.4e\n",mtime_dtime(), (double)(stime-ptime));
                    MST_METRIC_SET(self->stats->measurements[R7KR_CH_7KFRAME_SKEW],(stime-ptime));
#endif
#endif


                }else{
                    MX_LPRINT(R7KR, 2, "DRF invalid - returning[%"PRId64"] lost[%"PRId64"]\n", retval, lost_bytes);
                    MST_COUNTER_INC(self->stats->events[R7KR_EV_DRF_INVALID]);
                    MST_COUNTER_ADD(self->stats->status[R7KR_STA_DRF_INVAL_BYTES],lost_bytes);

                }
                if (NULL!=sync_bytes) {
                    (*sync_bytes) += lost_bytes;
                    //                        me_errno = ME_EINVAL;
                }
                // quit and return
                break;
            }

        } // while frame invalid

           // read header
            // if socket error, return esock
            // validate header
            // if invalid and resync, resync
            // if invalid and noresync, return einval
            // if valid continue
            // read payload
            // if socket error, return esock
            // validate checksum (if flag set), timestamp (per newer_than)
            // if invalid and resync, resync
            // if invalid and noresync, return einval
            // else return
    }

    return retval;
}// End function r7kr_read_drf


/// @fn int64_t r7kr_read_stripped_frame(r7kr_reader_t *self, uint32_t nframes, byte *dest, uint32_t len, uint32_t flags, double newer_than, uint32_t timeout_msec )
/// @brief read frame from file or socket, strip network frame headers (return data frame only).
/// @param[in] self reader reference
/// @param[in] dest data buffer
/// @param[in] len buffer size (bytes)
/// @param[in] flags option flags
/// @param[in] newer_than reject packets older than this (epoch time, decimal seconds)
/// @param[in] timeout_msec read timeout
/// @param[out] sync_bytes number of bytes skipped
/// @return number of bytes returned on success, -1 otherwise (me_errno set)
int64_t r7kr_read_stripped_frame(r7kr_reader_t *self, byte *dest,
                             uint32_t len, r7kr_flags_t flags,
                             double newer_than, uint32_t timeout_msec,
                             uint32_t *sync_bytes )
{
    int64_t retval = -1;

    if (flags&R7KR_NET_STREAM) {
        retval = r7kr_read_frame( self, dest,len,  flags,
                             newer_than,  timeout_msec,
                             sync_bytes );

        if (retval>0) {
            r7k_drf_t *pdrf = (r7k_drf_t *)(dest+R7K_NF_BYTES);
            memmove(dest,(dest+R7K_NF_BYTES),pdrf->size);
            retval-=R7K_NF_BYTES;
        }
    }else{
        retval = r7kr_read_frame( self, dest,len,  flags,
                                      newer_than,  timeout_msec,
                                      sync_bytes );
    }

//    fprintf(stderr,"r7kr_read_stripped_frame [%"PRId64"]\n",retval);
//    r7k_drf_t *pd = (r7k_drf_t *)(dest);
//    r7k_drf_show(pd,false,5);
//    r7k_hex_show(dest,retval,16,true,5);
//
//    byte *dp = dest+R7K_DRF_BYTES;
//
//    switch (pd->record_type_id) {
//        case 7027:
//        case 7000:
//            fprintf(stderr,"ping number %u\n",*((uint32_t *)(dp+8)));
//            break;
//
//        default:
//            break;
//    }

    return retval;
}// End function r7kr_read_stripped_frame

/// @fn int64_t r7kr_read_frame(r7kr_reader_t *self, uint32_t nframes, byte *dest, uint32_t len, uint32_t flags, double newer_than, uint32_t timeout_msec )
/// @brief read a frame from a file or socket. Stream may or may not contain
/// network frames (i.e. s7k), as specified using flags)
/// @param[in] self reader reference
/// @param[in] dest data buffer
/// @param[in] len buffer size (bytes)
/// @param[in] flags option flags
/// @param[in] newer_than reject packets older than this (epoch time, decimal seconds)
/// @param[in] timeout_msec read timeout
/// @param[out] sync_bytes number of bytes skipped
/// @return number of bytes returned on success, -1 otherwise (me_errno set)
int64_t r7kr_read_frame(r7kr_reader_t *self, byte *dest,
                         uint32_t len, r7kr_flags_t flags,
                         double newer_than, uint32_t timeout_msec,
                             uint32_t *sync_bytes )
{

    int64_t retval=-1;

    me_errno = ME_OK;
    r7kr_flags_t rflags=0;

    byte *pbuf=dest;

    r7kr_parse_state_t state = R7KR_STATE_START;
    r7kr_parse_action_t action = R7KR_ACTION_QUIT;
    msock_socket_t *sockif =r7kr_reader_sockif(self);
    if (NULL!=self && NULL!=dest &&
        ( (flags&R7KR_NF_STREAM) || (flags&R7KR_DRF_STREAM) || (flags&R7KR_NET_STREAM) ) &&
        NULL!=sockif && sockif->fd>0
        ) {

        uint32_t frame_bytes=0;
        uint32_t nf_bytes=0;
        uint32_t drf_bytes=0;
        int64_t read_bytes=0;
        uint32_t read_len=0;

        while (state != R7KR_STATE_FRAME_VALID ) {

            switch (state) {

                case R7KR_STATE_START:
//                    MX_MPRINT(R7KR_DEBUG, "R7KR_STATE_START\n");
                    if (flags&R7KR_NET_STREAM) {
                        read_len = R7K_NF_BYTES;
                        rflags   = R7KR_RESYNC_NF;
                        action   = R7KR_ACTION_READ_NF;
                    }else if (flags&R7KR_DRF_STREAM) {
                        // drf only (e.g. s7k)
                        rflags   = R7KR_RESYNC_DRF;
                        read_len = len;
                        action   = R7KR_ACTION_READ_DRF;
                    }else {
                        // net frames only
                        rflags   = R7KR_RESYNC_NF;
                        read_len = R7K_NF_BYTES;
                        action   = R7KR_ACTION_READ_NF;
                    }

                    pbuf     = dest;
                    memset(dest,0,len);
                    frame_bytes=0;
                    break;

                case R7KR_STATE_NF_VALID:
//                    MX_MMSG(R7KR_DEBUG, "R7KR_STATE_NF_VALID\n"));
                    if (flags&R7KR_NET_STREAM) {
                        // next, read the DRF
                        read_len = len-R7K_NF_BYTES;
                        // don't resync on failed DRF read
                        rflags   = 0;
                        action   = R7KR_ACTION_READ_DRF;
                    }else if (flags&R7KR_DRF_STREAM) {
                        // drf only (e.g. s7k)
                        // shouldn't be here
                        MX_MMSG(R7KR_DEBUG, "ERR - invalid condition: NF_VALID for DRF stream\n");
                    }else {
                        // net frames only
                        // done
                        state  = R7KR_STATE_FRAME_VALID;
                        action = R7KR_ACTION_QUIT;
                    }
                    break;

                case R7KR_STATE_DRF_VALID:
//                   MX_MMSG(R7KR_DEBUG, "R7KR_STATE_DRF_VALID\n");
                    if (flags&R7KR_NET_STREAM || flags&R7KR_DRF_STREAM) {
                        // done
                        state  = R7KR_STATE_FRAME_VALID;
                        action = R7KR_ACTION_QUIT;
                    }else {
                        // net frames only (unlikely)
                        MX_MMSG(R7KR_DEBUG, "ERR - invalid condition: DRF_VALID for NF stream\n");
                    }
                    break;

                case R7KR_STATE_NF_INVALID:
//                     MX_MMSG(R7KR_DEBUG, "R7KR_STATE_NF_INVALID (retrying)\n");
                    state  = R7KR_STATE_START;
                    action = R7KR_ACTION_NOOP;
                    break;

                case R7KR_STATE_DRF_INVALID:
                    MX_MMSG(R7KR_DEBUG, "R7KR_STATE_DRF_INVALID (retrying)\n");
                    state  = R7KR_STATE_START;
                    action = R7KR_ACTION_NOOP;
                    break;

                case R7KR_STATE_READ_ERR:
                    MX_MMSG(R7KR_DEBUG, "R7KR_STATE_READ_ERR\n");
                    if (me_errno==ME_ESOCK) {
                        MX_MMSG(R7KR_DEBUG, "socket disconnected - quitting\n");
                    }else if (me_errno==ME_EOF) {
                        MX_MMSG(R7KR_ERROR, "end of file\n");
                    }else if (me_errno==ME_ENOSPACE) {
                        MX_MPRINT(R7KR_ERROR, "buffer full [%"PRIu32"/%"PRIu32"]\n", (uint32_t)(pbuf+read_len-dest), len);
                    }else{
                        MX_MPRINT(R7KR_DEBUG, "read error [%d/%s]\n", me_errno, me_strerror(me_errno));
                    }
                    action   = R7KR_ACTION_QUIT;
                    break;

               default:
                    MX_MPRINT(R7KR_ERROR, "ERR - unknown state[%d]\n", state);
                    break;
            }

            if (action==R7KR_ACTION_READ_NF) {
                if ( (read_bytes=r7kr_read_nf(self, pbuf, read_len, rflags, newer_than, timeout_msec, sync_bytes)) == R7K_NF_BYTES) {

//                     MX_MMSG(R7KR_DEBUG, "nf read OK\n");
                    // update pointers
                    pbuf        += read_bytes;
                    nf_bytes    += read_bytes;
                    frame_bytes += read_bytes;

                    state = R7KR_STATE_NF_VALID;
                }else{
                    MX_MPRINT(R7KR_ERROR, "ERR - r7kr_read_nf read_bytes[%"PRId64"] [%d/%s]\n", read_bytes, me_errno, me_strerror(me_errno));
                    state = R7KR_STATE_READ_ERR;
                }
            }

            if (action==R7KR_ACTION_READ_DRF) {
                if ( (read_bytes=r7kr_read_drf(self, pbuf, read_len, rflags, newer_than, timeout_msec, sync_bytes)) > R7K_DRF_BYTES) {

//                    MX_MMSG(R7KR_DEBUG, "drf read OK\n");
                    // update pointers
                    pbuf        += read_bytes;
                    drf_bytes   += read_bytes;
                    frame_bytes += read_bytes;

                    state = R7KR_STATE_DRF_VALID;
                }else{
                    MX_MPRINT(R7KR_ERROR, "ERR - r7kr_read_nf read_bytes[%"PRId64"] [%d/%s]\n", read_bytes, me_errno, me_strerror(me_errno));
                    state = R7KR_STATE_READ_ERR;
                }
            }
            if (action==R7KR_ACTION_QUIT) {

//                MX_MMSG(R7KR_DEBUG, "ACTION_QUIT\n");
                if (state==R7KR_STATE_FRAME_VALID) {
                    retval = frame_bytes;
                    MX_LPRINT(R7KR, 2, "Frame valid - returning[%"PRId64"]\n", retval);
                    MST_COUNTER_ADD(self->stats->status[R7KR_STA_FRAME_VAL_BYTES],frame_bytes);


//                    MST_COUNTER_INC(self->stats->events[R7KR_EV_FRAME_VALID]);
//                    r7k_nf_show((r7k_nf_t *)dest,false,5);
//                    r7k_drf_show((r7k_drf_t *)(dest+R7K_NF_BYTES),false,5);
//                    r7k_hex_show(dest,frame_bytes,16,true,5);
//                    if (self->log_id!=MLOG_ID_INVALIDstream) {
//                       // TODO : replace with plain file
//                        fwrite(dest,frame_bytes,1,self->logstream);
//                    }

                    if (self->log_id!=MLOG_ID_INVALID) {
                        mlog_write(self->log_id,dest,frame_bytes);
                    }

                }else{
                    MX_MMSG(R7KR_DEBUG, "Frame invalid [%d/%s] retval[%"PRId64"]\n", me_errno, me_strerror(me_errno), retval);
                    MST_COUNTER_INC(self->stats->events[R7KR_EV_FRAME_INVALID]);
                }
                // quit and return
                break;
            }
        }// while frame invalid

    }else{
        //        PEPRINT((stderr,"invalid argument\n"));
        MX_ERROR_MSG("invalid argument\n");
    }
    MX_LPRINT(R7KR, 2, "r7kr_read_frame returning [%"PRId64"]\n", retval);
    return retval;
 }
// End function r7kr_read_frame

/// @fn int64_t r7kr_reader_read(r7kr_reader_t * self, byte * dest, uint32_t len)
/// @brief read from reson 7k center socket.
/// @param[in] self reader reference
/// @param[in] dest data buffer
/// @param[in] len data buffer (read) length
/// @return number of bytes read on success, -1 otherwise
int64_t r7kr_reader_read(r7kr_reader_t *self, byte *dest, uint32_t len)
{
    int64_t retval=0;
    if (NULL != self && NULL != dest) {
        return r7k_drfcon_read(self->fc,dest,len);
    }else{
        //        PEPRINT((stderr,"invalid argument\n"));
        MX_ERROR_MSG("invalid argument\n");
        retval=-1;
        me_errno = ME_EINVAL;
    }
    return retval;
}
// End function r7kr_reader_read

/// @fn int64_t r7kr_reader_seek(r7kr_reader_t * self, uint32_t ofs)
/// @brief set output buffer (read) pointer.
/// @param[in] self reader reference
/// @param[in] ofs buffer offset
/// @return new file position on success, -1 otherwise
int64_t r7kr_reader_seek(r7kr_reader_t *self, uint32_t ofs)
{
    int64_t retval=-1;
    if (NULL != self) {
        return r7k_drfcon_seek(self->fc, ofs);
    }else{
        //        PEPRINT((stderr,"invalid argument\n"));
        MX_ERROR_MSG("invalid argument\n");
        retval=-1;
        me_errno = ME_EINVAL;
    }
    return retval;
}
// End function r7kr_reader_seek

/// @fn int64_t r7kr_reader_tell(r7kr_reader_t * self)
/// @brief return current output buffer (read) pointer position.
/// @param[in] self reader reference
/// @return read pointer offset on success, -1 otherwise
int64_t r7kr_reader_tell(r7kr_reader_t *self)
{
    int64_t retval=-1;
    if (NULL != self) {
        return r7k_drfcon_tell(self->fc);
    }else{
        //        PEPRINT((stderr,"invalid argument\n"));
        MX_ERROR_MSG("invalid argument\n");
        retval=-1;
        me_errno = ME_EINVAL;
    }
    return retval;
}
// End function r7kr_reader_tell

/// @fn uint32_t r7kr_reader_frames(r7kr_reader_t * self)
/// @brief return number of data record frames currently in buffer.
/// @param[in] self reader reference
/// @return number of data record frames on success, -1 otherwise.
uint32_t r7kr_reader_frames(r7kr_reader_t *self)
{
    uint32_t retval=-1;
    if (NULL != self) {
        return r7k_drfcon_frames(self->fc);
    }else{
        //        PEPRINT((stderr,"invalid argument\n"));
        MX_ERROR_MSG("invalid argument\n");
        retval=-1;
        me_errno = ME_EINVAL;
    }
    return retval;
}
// End function r7kr_reader_frames

/// @fn r7k_drf_t * r7kr_reader_enumerate(r7kr_reader_t * self)
/// @brief set buffer pointer to first frame and return a pointer to it.
/// @param[in] self reader reference
/// @return pointer first data record frame in reader buffer on success, NULL otherwise
r7k_drf_t *r7kr_reader_enumerate(r7kr_reader_t *self)
{
    r7k_drf_t *retval=NULL;
    if (NULL != self) {
        return r7k_drfcon_enumerate(self->fc);
    }else{
        //        PEPRINT((stderr,"invalid argument\n"));
        MX_ERROR_MSG("invalid argument\n");
    }
    return retval;
}
// End function r7kr_reader_enumerate

/// @fn r7k_drf_t * r7kr_reader_next(r7kr_reader_t * self)
/// @brief return reference to next frame in reader buffer.
/// should call r7kr_reader_enumerate to start enumeration
/// @param[in] self reader reference
/// @return pointer to next DRF in reader buffer
r7k_drf_t    *r7kr_reader_next(r7kr_reader_t *self)
{
    r7k_drf_t *retval=NULL;
    if (NULL != self) {
        return r7k_drfcon_next(self->fc);
    }else{
        //        PEPRINT((stderr,"invalid argument\n"));
        MX_ERROR_MSG("invalid argument\n");
    }
    return retval;
}
// End function r7kr_reader_next

/// @fn _Bool r7kr_reader_issub(r7kr_reader_t *self, uint32_t record_type)
/// @brief return true if record type is on subscription list.
/// @param[in] self reader reference
/// @param[in] record_type packet type to check
/// @return true if packet type is on subscription list, false otherwise
bool r7kr_reader_issub(r7kr_reader_t *self, uint32_t record_type)
{
    if (NULL!=self && NULL!=self->sub_list && self->sub_count>0) {
        for (uint32_t i=0; i<self->sub_count; i++) {
            if (self->sub_list[i]==record_type) {
                return true;
            }
        }
    }
    return false;
}
// End function r7kr_reader_issub


/// @fn _Bool r7kr_peer_cmp(void * a, void * b)
/// @brief compare msock_connection_t IDs. Used by mlist.
/// @param[in] a void pointer to msock_connection_t
/// @param[in] b void pointer to msock_connection_t
/// @return true if peer id's are equal, false otherwise
bool r7kr_peer_cmp(void *a, void *b)
{
    bool retval=false;
    if (a!=NULL && b!=NULL) {
        msock_connection_t *pa = (msock_connection_t *)a;
        msock_connection_t *pb = (msock_connection_t *)b;
        retval = (pa->id == pb->id);
    }
    return retval;
}
// End function r7kr_peer_cmp


/// @fn _Bool r7kr_peer_vcmp(void * item, void * value)
/// @brief compare msock_connection_t ID to specified value. Used by mlist.
/// @param[in] item void pointer to msock_connection_t
/// @param[in] value void pointer to integer (id value)
/// @return returns true if the specified peer has the specified ID. false otherwise
bool r7kr_peer_vcmp(void *item, void *value)
{
    bool retval=false;
    if (NULL!=item && NULL!=value) {
        msock_connection_t *peer = (msock_connection_t *)item;
        int svc = *((int *)value);
//        MX_MPRINT(R7KR_DEBUG, "peer[%p] id[%d] svc[%d]\n", peer, peer->id, svc);
        retval = (peer->id == svc);
    }
    return retval;
}
// End function r7kr_peer_vcmp

#ifdef WITH_R7KR_TEST
int r7kr_test(int argc, char **argv)
{
    int retval=-1;

    int i=0;
    char *host="localhost";
    int port=R7K_7KCENTER_PORT;
    int cycles=3;
    int retries=10;
    int errors=0;

    int verbose=1;
    for(i=1;i<argc;i++){
        char *ap=strstr(argv[i],"--host=");
        if(ap!=NULL){
            host=ap+strlen("--host=");
            continue;
        }
        ap=strstr(argv[i],"--port=");
        if(ap!=NULL){
            sscanf(ap+strlen("--port="),"%d",&port);
            continue;
        }
        ap=strstr(argv[i],"--verbose=");
        if(ap!=NULL){
            sscanf(ap+strlen("--verbose="),"%d",&verbose);
            continue;
        }
        ap=strstr(argv[i],"--retries=");
        if(ap!=NULL){
            sscanf(ap+strlen("--retries="),"%d",&retries);
            continue;
        }
        ap=strstr(argv[i],"--cycles=");
        if(ap!=NULL){
            sscanf(ap+strlen("--cycles="),"%d",&cycles);
            continue;
        }
        fprintf(stderr,"  Options : \n");
        fprintf(stderr,"   --verbose=n      : output level (n>=0)\n");
        fprintf(stderr,"   --host=<ip_addr> : TRN host IP address\n");
        fprintf(stderr,"   --port=<op_port> : TRN host IP address\n");
        fprintf(stderr,"   --cycles=n       : number of frames to read\n");
        fprintf(stderr,"   --retries=n      : reconnection retries\n");
        fprintf(stderr,"   --help=n         : show use info\n\n");
        exit(0);

    }

    if(verbose>1){
        fprintf(stderr,"host    : [%s]\n",host);
        fprintf(stderr,"port    : [%d]\n",port);
        fprintf(stderr,"cycles  : [%d]\n",cycles);
        fprintf(stderr,"retries : [%d]\n",retries);
        fprintf(stderr,"verbose : [%d]\n",verbose);
    }

    uint32_t nsubs=11;
    uint32_t subs[]={1003, 1006, 1008, 1010, 1012, 1013, 1015,
        1016, 7000, 7004, 7027};

    // initialize reader
    // create and open socket connection

    r7kr_reader_t *reader = r7kr_reader_new(R7KC_DEV_7125_400KHZ, host,port,MAX_FRAME_BYTES_7K, subs, nsubs);

    // show reader config
    if(verbose>1)
    r7kr_reader_show(reader,true, 5);

    uint32_t lost_bytes=0;
    // test r7kr_read_frame
    byte frame_buf[MAX_FRAME_BYTES_7K]={0};
    int frames_read=0;

    if(verbose>1)
        fprintf(stderr,"connecting reader [%s/%d]\n",host,R7K_7KCENTER_PORT);

    while ( (frames_read<cycles) && retries>0 ) {
         // clear frame buffer
        memset(frame_buf,0,MAX_FRAME_BYTES_7K);
        // read frame
        int istat=0;
        if( (istat = r7kr_read_frame(reader, frame_buf, MAX_FRAME_BYTES_7K, R7KR_NET_STREAM, 0.0, R7KR_READ_TMOUT_MSEC,&lost_bytes )) > 0){

            frames_read++;

            if(verbose>0)
                fprintf(stderr,"r7kr_read_frame cycle[%d/%d] lost[%u] ret[%d]\n",frames_read,cycles,lost_bytes,istat);
            // show contents
            if (verbose>=1) {
                r7k_nf_t *nf = (r7k_nf_t *)(frame_buf);
                r7k_drf_t *drf = (r7k_drf_t *)(frame_buf+R7K_NF_BYTES);

                MX_LMSG(R7KR, 1, "NF:\n");
                r7k_nf_show(nf,false,5);
                MX_LMSG(R7KR, 1, "DRF:\n");
                r7k_drf_show(drf,false,5);
                if(verbose>1){
                    MX_LMSG(R7KR, 1, "data:\n");
                    r7k_hex_show(frame_buf,istat,16,true,5);
                }
            }
        }else{
            retries--;
            errors++;
            // read error
            fprintf(stderr,"ERR - r7kr_read_frame - cycle[%d/%d] ret[%d] lost[%u] err[%d/%s]\n",frames_read+1,cycles,istat,lost_bytes,errno,strerror(errno));
            if (errno==ECONNREFUSED || me_errno==ME_ESOCK || me_errno==ME_EOF || me_errno==ME_ERECV) {

                fprintf(stderr,"socket closed - reconnecting in 5 sec\n");
                sleep(5);
                r7kr_reader_connect(reader,true);
            }
        }
    }

    r7kr_reader_destroy(&reader);
    if(frames_read==cycles)retval=0;

    if(verbose>0)
    fprintf(stderr,"frames[%d/%d]  retries[%d] lost[%u] errors[%d]\n",frames_read,cycles,retries,lost_bytes,errors);

    return retval;
}
#endif // WITH_R7KR_TEST
