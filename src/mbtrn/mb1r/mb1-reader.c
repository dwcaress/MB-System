///
/// @file mb1-reader.c
/// @authors k. Headley
/// @date 01 jan 2018

/// Reson 7KCenter reader API
/// contains mb1r_reader_t, a component that
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
#include "mb1-reader.h"
#include "mb1_msg.h"
#include "mframe.h"
#include "mlog.h"
#include "msocket.h"
#include "mfile.h"
#include "mstats.h"
#include "mtime.h"
#include "mxdebug.h"
#include "mxd_app.h"
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

static const char *mb1r_event_labels[]={ \
    "frame_valid",
    "frame_invalid",
    "hdr_valid",
    "data_valid",
    "hdr_invalid",
    "data_invalid",
    "data_resync",
    "hdr_resync",
    "hdr_short_read",
    "data_short_read",
    "e_hdr_type",
    "e_hdr_size",
    "e_hdr_time",
    "e_hdr_chksum",
    "e_hdr_read",
    "e_sock",
    "e_data_chk",
    "e_data_sync",
    "e_data_read",
    "e_fc_write"
};

static const char *mb1r_status_labels[]={ \
    "frame_valid_bytes",
    "hdr_valid_bytes",
    "data_valid_bytes",
    "hdr_inval_bytes",
    "data_inval_bytes"
};

static const char *mb1r_metric_labels[]={ \
    //    "mb1r_refill_xt",
    "mb1r_frame_skew"
};

static const char **mb1r_stats_labels[MSLABEL_COUNT]={
    mb1r_event_labels,
    mb1r_status_labels,
    mb1r_metric_labels
};

/////////////////////////
// Function Definitions
/////////////////////////

///// @fn const char *mb1r_get_version()
///// @brief get version string.
///// @return version string
//const char *mb1r_get_version()
//{
//    return MB1R_VERSION_STR;
//}
//// End function mb1r_get_version
//
///// @fn const char *mb1r_get_build()
///// @brief get build string.
///// @return version string
//const char *mb1r_get_build()
//{
//    return MB1R_BUILD;
//}
//// End function mb1r_get_build
//
///// @fn void mb1r_show_app_version(const char *app_version)
///// @brief get version string.
///// @return version string
//void mb1r_show_app_version(const char *app_name, const char *app_version)
//{
//    printf("\n %s built[%s] libmb1rn[v%s / %s] \n\n",app_name, app_version, mb1r_get_version(),mb1r_get_build());
//}
//// End function mb1r_show_app_version

/// @fn int mb1r_reader_connect(*self)
/// @brief connect to 7k center and subscribe to records.
/// @param[in] self reader reference
/// @return 0 on success, -1 otherwise
int mb1r_reader_connect(mb1r_reader_t *self, bool replace_socket)
{
    int retval=-1;
    me_errno = ME_OK;

    if (NULL!=self) {

        char *host = strdup(self->sockif->addr->host);
        int port = self->sockif->addr->port;

        if (replace_socket) {
            MX_MMSG(MB1R_DEBUG,  "destroying socket\n");
            msock_socket_destroy(&self->sockif);

            MX_MMSG(MB1R_DEBUG,  "building socket\n");
            self->sockif = msock_socket_new(host,port,ST_TCP);
        }

        const int optionval = 1;
#if defined(__CYGWIN__)
        msock_set_opt(self->sockif, SO_REUSEADDR, &optionval, sizeof(optionval));
#else
        msock_set_opt(self->sockif, SO_REUSEPORT, &optionval, sizeof(optionval));
#endif
        MX_MPRINT(MB1R_DEBUG, "connecting to stream [%s:%d]\n", self->sockif->addr->host, self->sockif->addr->port);
        if(msock_connect(self->sockif)==0){
            self->state=MB1R_CONNECTED;
            self->sockif->status=SS_CONNECTED;
            retval=0;
        }else{
            MX_MPRINT(MB1R_DEBUG, "connect failed [%s]\n", self->sockif->addr->host);
            me_errno=ME_ECONNECT;
            self->state=MB1R_INITIALIZED;
            mb1r_reader_reset_socket(self);
        }
    }
    return retval;
}
// End function mb1r_reader_connect

/// @fn mb1r_reader_t * mb1r_reader_new(const char * host, int port, uint32_t capacity, uint32_t * slist, uint32_t slist_len)
/// @brief create new reson 7k reader. connect and subscribe to reson data.
/// @param[in] host reson 7k center host IP address
/// @param[in] port reson 7k center host IP port
/// @param[in] capacity input buffer capacity (bytes)
/// @param[in] slist 7k center message subscription list
/// @param[in] slist_len length of subscription list
/// @return new reson reader reference on success, NULL otherwise
mb1r_reader_t *mb1r_reader_new(const char *host, int port, uint32_t capacity)
{
    mb1r_reader_t *self = (mb1r_reader_t *)malloc(sizeof(mb1r_reader_t));
    if (NULL != self) {

        self->state=MB1R_NEW;
        self->fileif=NULL;
        self->sockif = msock_socket_new(host,port,ST_TCP);
        self->log_id=MLOG_ID_INVALID;
        self->logstream=NULL;
        self->state=MB1R_INITIALIZED;

        if (NULL != self->sockif) {
            mb1r_reader_connect(self, false);
            if(me_errno != ME_OK)fprintf(stderr,"connect error (%s)\n",me_strerror(me_errno));
        }else{
            self->state=ME_ECREATE;
        }
        self->stats=mstats_new(MB1R_EV_COUNT, MB1R_STA_COUNT, MB1R_MET_COUNT, mb1r_stats_labels);
    }

    return self;
}
// End function mb1r_reader_create

/// @fn mb1r_reader_t * mb1r_freader_new(mfile_file_t *file, uint32_t capacity, uint32_t * slist, uint32_t slist_len)
/// @brief create new reson 7k reader. connect and subscribe to reson data.
/// @param[in] path input file path
/// @param[in] capacity input buffer capacity (bytes)
/// @param[in] slist 7k center message subscription list
/// @param[in] slist_len length of subscription list
/// @return new reson reader reference on success, NULL otherwise
mb1r_reader_t *mb1r_freader_new(mfile_file_t *file, uint32_t capacity)
{
    mb1r_reader_t *self = (mb1r_reader_t *)malloc(sizeof(mb1r_reader_t));
    if (NULL != self) {

        self->state  = MB1R_NEW;
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

        self->state=MB1R_INITIALIZED;

        self->stats=mstats_new(MB1R_EV_COUNT, MB1R_STA_COUNT, MB1R_MET_COUNT, mb1r_stats_labels);
    }

    return self;
}
// End function mb1r_reader_create

/// @fn void mb1r_reader_destroy(mb1r_reader_t ** pself)
/// @brief release  reader resources.
/// @param[in] pself pointer to instance reference
/// @return none
void mb1r_reader_destroy(mb1r_reader_t **pself)
{
    if (NULL != pself) {
        mb1r_reader_t *self = *pself;
        if (NULL != self) {
            if (self->sockif) {
                msock_socket_destroy(&self->sockif);
            }
            if (self->fileif) {
                mfile_file_destroy(&self->fileif);
            }
            if (self->stats) {
                mstats_destroy(&self->stats);
            }

            free(self);
            *pself = NULL;
        }
    }
}
// End function mb1r_reader_destroy

/// @fn void mb1r_reader_reset_socket(mb1r_reader_t *self)
/// @brief set logger
/// @param[in] self pointer to instance
/// @param[in] log  pointer to log instance
/// @return none
void mb1r_reader_reset_socket(mb1r_reader_t *self)
{
    if (NULL!=self) {
        close(self->sockif->fd);
        self->sockif->fd=-1;
        self->sockif->status=SS_CONFIGURED;
    }
}
// End function mb1r_reader_reset_socket

/// @fn void mb1r_reader_setlog(mb1r_reader_t *self, mlog_t *log, mlog_id_t id, const char *desc)
/// @brief set logger
/// @param[in] self pointer to instance
/// @param[in] log  pointer to log instance
/// @return none
void mb1r_reader_set_log(mb1r_reader_t *self, mlog_id_t id)
{
    if (NULL!=self) {
        if (self->log_id!=MLOG_ID_INVALID) {
            mlog_delete_instance(self->log_id);
        }
        self->log_id=id;
     }
    return;
}
// End function mb1r_reader_set_log

/// @fn void mb1r_reader_set_logstream(mb1r_reader_t *self, FILE *log)
/// @brief set logger
/// @param[in] self pointer to instance
/// @param[in] log  pointer to log instance
/// @return none
void mb1r_reader_set_logstream(mb1r_reader_t *self, FILE *log)
{
    if (NULL!=self) {
        if (self->log_id != MLOG_ID_INVALID) {
            fclose(self->logstream);
        }
        self->logstream=log;
    }
    return;
}
// End function mb1r_reader_set_logstream

/// @fn int mb1r_reader_set_file(mb1r_reader_t *self, mfile_file_t *file)
/// @brief set current reader file
/// @param[in] self pointer to instance
/// @param[in] file  pointer to file
/// @return none
int mb1r_reader_set_file(mb1r_reader_t *self, mfile_file_t *file)
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
            MX_MPRINT(MB1R_ERROR, "ERR - could not open file [%s] [%d/%s]\n", self->fileif->path, errno, strerror(errno));
        }
    }
    return retval;
}
// End function mb1r_reader_set_file

/// @fn mstats_t *mb1r_reader_get_stats(mb1r_reader_t *self)
/// @brief get statistics reference for mb1r_reader
/// @param[in] self mb1r_reader reference
/// @return mstats_t reference on success, NULL otherwise
mstats_t *mb1r_reader_get_stats(mb1r_reader_t *self)
{
    mstats_t *retval=NULL;
    if (NULL!=self) {
        retval=self->stats;
    }
    return retval;
}
// End function mb1r_reader_get_stats

/// @fn const char ***mb1r_reader_get_statlabels()
/// @brief get statistics labels
/// @return stats label array reference
const char ***mb1r_reader_get_statlabels()
{
    return mb1r_stats_labels;
}
// End function mb1r_reader_get_statlabels


/// @fn msock_socket_t * mb1r_reader_sockif(mb1r_reader_t * self)
/// @brief get reader socket interface.
/// @param[in] self reader reference
/// @return socket reference on success, NULL otherwise.
msock_socket_t *mb1r_reader_sockif(mb1r_reader_t *self)
{
    return (NULL!=self ? self->sockif : NULL);
}
// End function mb1r_reader_sockif

/// @fn mfile_file_t * mb1r_reader_fileif(mb1r_reader_t * self)
/// @brief get reader file interface.
/// @param[in] self reader reference
/// @return socket reference on success, NULL otherwise.
mfile_file_t *mb1r_reader_fileif(mb1r_reader_t *self)
{
    return (NULL!=self ? self->fileif : NULL);
}
// End function mb1r_reader_sockif

/// @fn void mb1r_reader_show(mb1r_reader_t * self, _Bool verbose, uint16_t indent)
/// @brief output reader parameter summary to stderr.
/// @param[in] self reader reference
/// @param[in] verbose provide verbose output
/// @param[in] indent output indentation (spaces)
/// @return none
void mb1r_reader_show(mb1r_reader_t *self, bool verbose, uint16_t indent)
{
    if (NULL != self) {
        fprintf(stderr,"%*s[self      %10p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[sockif    %10p]\n",indent,(indent>0?" ":""), self->sockif);
        fprintf(stderr,"%*s[fileif    %10p]\n",indent,(indent>0?" ":""), self->fileif);
        fprintf(stderr,"%*s[state    %2d/%s]\n",indent,(indent>0?" ":""), self->state, mb1r_strstate(self->state));
    }
}
// End function mb1r_reader_show


/// @fn void mb1r_reader_show(mb1r_reader_t * self, _Bool verbose, uint16_t indent)
/// @brief output reader parameter summary to stderr.
/// @param[in] self reader reference
/// @param[in] verbose provide verbose output
/// @param[in] indent output indentation (spaces)
/// @return none
void mb1r_ctx_show(mb1r_sm_ctx_t *self, bool verbose, uint16_t indent)
{
    if (NULL != self) {
        int wkey=15;
        int wval=15;
        fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"self",wval,self);
        fprintf(stderr,"%*s%*s %*d/%s\n",indent,(indent>0?" ":""),wkey,"state", wval, self->state, mb1r_ctx_strstate(self->state));
        fprintf(stderr,"%*s%*s %*d/%s\n",indent,(indent>0?" ":""),wkey,"action", wval, self->action, mb1r_ctx_straction(self->action));
        fprintf(stderr,"%*s%*s %*"PRIu32"\n",indent,(indent>0?" ":""),wkey,"timeout_msec",wval,self->timeout_msec);
        fprintf(stderr,"%*s%*s %*.2lf\n",indent,(indent>0?" ":""),wkey,"newer_than",wval,self->newer_than);
        fprintf(stderr,"%*s%*s %*s%08X\n",indent,(indent>0?" ":""),wkey,"flags",wval-8,"",self->flags);
        fprintf(stderr,"%*s%*s %*s%08X\n",indent,(indent>0?" ":""),wkey,"rflags",wval-8,"",self->rflags);
        fprintf(stderr,"%*s%*s %*"PRIu32"\n",indent,(indent>0?" ":""),wkey,"read_len",wval,self->read_len);

        fprintf(stderr,"%*s%*s %*p/%"PRIu32"\n",indent,(indent>0?" ":""),
                wkey,"sync_bytes",wval,self->sync_bytes,(self->sync_bytes?*self->sync_bytes:0));

        fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"dest",wval,self->dest);
        fprintf(stderr,"%*s%*s %*"PRIu32"\n",indent,(indent>0?" ":""),wkey,"len",wval,self->len);
        fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"pbuf",wval,self->pbuf);
        fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"psnd",wval,self->psnd);
        fprintf(stderr,"%*s%*s %*"PRId64"\n",indent,(indent>0?" ":""),wkey,"frame_bytes",wval,self->frame_bytes);
        fprintf(stderr,"%*s%*s %*"PRId64"\n",indent,(indent>0?" ":""),wkey,"lost_bytes",wval,self->lost_bytes);
        fprintf(stderr,"%*s%*s %*"PRIu32"\n",indent,(indent>0?" ":""),wkey,"pending_bytes",wval,self->pending_bytes);
        fprintf(stderr,"%*s%*s %*"PRIu32"\n",indent,(indent>0?" ":""),wkey,"header_bytes",wval,self->header_bytes);
        fprintf(stderr,"%*s%*s %*"PRIu32"\n",indent,(indent>0?" ":""),wkey,"data_bytes",wval,self->data_bytes);
        fprintf(stderr,"%*s%*s %*"PRId64"\n",indent,(indent>0?" ":""),wkey,"read_bytes",wval,self->read_bytes);
    }
}
// End function mb1r_reader_show

/// @fn const char * mb1r_strstate(mb1r_state_t state)
/// @brief get string for reader state.
/// @param[in] state state ID
/// @return mnemonic string on success, NULL otherwise
const char *mb1r_strstate(mb1r_state_t state)
{
	const char *retval = "UNDEFINED";
    switch (state) {
        case MB1R_NEW:
            retval = "NEW";
            break;
        case MB1R_INITIALIZED:
            retval = "INITIALIZED";
            break;
        case MB1R_CONNECTED:
            retval = "CONNECTED";
            break;
        case MB1R_SUBSCRIBED:
            retval = "SUBSCRIBED";
            break;

        default:
            break;
    }
    return retval;
}
// End function mb1r_strstate

/// @fn const char * mb1r_ctx_strstate(mb1r_parse_state_t state)
/// @brief get string for parser state.
/// @param[in] state state ID
/// @return mnemonic string on success, NULL otherwise
const char *mb1r_ctx_straction(mb1r_parse_action_t state)
{
    const char *retval = "UNDEFINED";
    switch (state) {
        case MB1R_ACTION_NOOP:
            retval = "MB1R_ACTION_NOOP";
            break;
        case MB1R_ACTION_READ:
            retval = "MB1R_ACTION_READ";
            break;
        case MB1R_ACTION_VALIDATE_HEADER:
            retval = "MB1R_ACTION_VALIDATE_HEADER";
            break;
        case MB1R_ACTION_VALIDATE_DATA:
            retval = "MB1R_ACTION_VALIDATE_DATA";
            break;
        case MB1R_ACTION_READ_HEADER:
            retval = "MB1R_ACTION_READ_HEADER";
            break;
        case MB1R_ACTION_READ_DATA:
            retval = "MB1R_ACTION_READ_DATA";
            break;
        case MB1R_ACTION_RESYNC:
            retval = "MB1R_ACTION_RESYNC";
            break;
        case MB1R_ACTION_QUIT:
            retval = "MB1R_ACTION_QUIT";
            break;
        default:
            retval = "INVALID_ACTION";
            break;
    }
    return retval;
}

/// @fn const char * mb1r_ctx_strstate(mb1r_parse_state_t state)
/// @brief get string for parser state.
/// @param[in] state state ID
/// @return mnemonic string on success, NULL otherwise
const char *mb1r_ctx_strstate(mb1r_parse_state_t state)
{
    const char *retval = "UNDEFINED";
    switch (state) {
        case MB1R_STATE_START:
            retval = "MB1R_STATE_START";
            break;
        case MB1R_STATE_READ_ERR:
            retval = "MB1R_STATE_READ_ERR";
            break;
        case MB1R_STATE_READ_OK:
            retval = "MB1R_STATE_READ_OK";
            break;
        case MB1R_STATE_READING:
            retval = "MB1R_STATE_READING";
            break;
        case MB1R_STATE_DATA_VALID:
            retval = "MB1R_STATE_DATA_VALID";
            break;
        case MB1R_STATE_DATA_INVALID:
            retval = "MB1R_STATE_DATA_INVALID";
            break;
        case MB1R_STATE_HEADER_INVALID:
            retval = "MB1R_STATE_HEADER_INVALID";
            break;
        case MB1R_STATE_HEADER_VALID:
            retval = "MB1R_STATE_HEADER_VALID";
            break;
        case MB1R_STATE_FRAME_VALID:
            retval = "MB1R_STATE_FRAME_VALID";
            break;
        case MB1R_STATE_FRAME_INVALID:
            retval = "MB1R_STATE_FRAME_INVALID";
            break;
        case MB1R_STATE_COMPLETE:
            retval = "MB1R_STATE_COMPLETE";
            break;
        case MB1R_STATE_DISCONNECTED:
            retval = "MB1R_STATE_DISCONNECTED";
            break;

        default:
            retval = "INVALID_STATE";
            break;
    }
    return retval;
}
// End function mb1r_strstate

/// @fn void mb1r_reader_flush(mb1r_reader_t * self, uint32_t len, int32_t retries, uint32_t tmout_ms)
/// @brief flush reader input buffer.
/// @param[in] self reader reference
/// @param[in] len number of bytes of input to read
/// @param[in] retries number of retries
/// @param[in] tmout_ms timeout
/// @return none; attempts to read len characters at a time
/// until a timeout occurs
void mb1r_reader_flush(mb1r_reader_t *self, uint32_t len, int32_t retries, uint32_t tmout_ms)
{
    if (NULL != self) {
        // read until timeout
        byte buf[len];
        int64_t x=0;
        uint32_t n=0;
        bool use_retries = (retries>0 ? true : false);

        do{
           x=msock_read_tmout(mb1r_reader_sockif(self), buf, len, tmout_ms);
           n++;
            if (use_retries) {
                if (retries-- ==0) {
                    break;
                }
            }
        }while ( (x!=-1) && (me_errno!=(int)ME_ETMOUT));
//        MX_MPRINT(MB1R_DEBUG, "EXIT - retries[%d/%s] x[%"PRId64"] e[%d/%s] n[%u]\n", retries, (use_retries?"true":"false"), x,
//                me_errno, me_strerror(me_errno), n);
    }
}
// End function mb1r_reader_flush

static int s_sm_act_resync(mb1r_reader_t *self, mb1r_sm_ctx_t *pctx)
{
    int retval=-1;
    // input : psync points to start of search
    // input : pbuf points to end of input
    // input : sync_found false
    // input : pframe NULL

    while( pctx->sync_found==false &&
          (pctx->psync < (pctx->pbuf-MB1_HEADER_BYTES))){

        mb1_t *pframe = (mb1_t *)pctx->psync;

        // match only type ID...
        // we'll be back if other parts are invalid, and
        // minimal matching means we can search to the
        // end of the buffer (don't need to access other data
        // members that could be outside the buffer)
        if(pframe->type == MB1_TYPE_ID){
            // found header sync sequence (TYPE_ID)
            // pending bytes: how many bytes left in buffer
            pctx->pending_bytes = (pctx->pbuf - pctx->psync);

            // contents could be partial header and/or data...
            // have read more bytes than a header
            if (pctx->pending_bytes > MB1_HEADER_BYTES) {

                // validate header...
                if(pframe->size==MB1_SOUNDING_BYTES(pframe->nbeams) &&
                   pframe->nbeams<=MB1_MAX_BEAMS){

                    uint32_t completion_bytes = 0;

                    if(pctx->pending_bytes <= pframe->size){
                        // possibly complete data remaining buffer

                        completion_bytes = (pframe->size - pctx->pending_bytes);
                        // move the bytes we have to the
                        // start of the buffer
                        memmove(pctx->dest, pctx->psync, pctx->pending_bytes);

                        // clean up buffer
                        memset((pctx->dest+pctx->pending_bytes), 0, (pctx->len-pctx->pending_bytes));

                        // configure state to continue reading data
                        pctx->pbuf           = pctx->dest + pctx->pending_bytes;
                        pctx->read_len       = completion_bytes;
                        pctx->header_pending = false;
                        pctx->data_pending   = true;
                        pctx->state          = MB1R_STATE_READING;

                        // return to the state machine
                        pctx->sync_found     = true;
                        break;
                    }else{
                        // else the frame found appears to be smaller than
                        // what's already read, i.e. there are leftover bytes
                        // that will cause the next frame to be out of sync.
                        // options:
                        // - discard it all and read until we have a frame with no leftover bytes
                        // - return to the state machine with this frame

                        completion_bytes = (pctx->pending_bytes-pframe->size);
                        // move the bytes we have to the
                        // start of the buffer
                        memmove(pctx->dest, pctx->psync, pframe->size);
                        pframe = (mb1_t *)pctx->dest;

                        // clean up buffer
                        memset((pctx->dest+pframe->size), 0, (pctx->len-pframe->size));

                        // return the frame:
                        pctx->frame_bytes = pframe->size;
                        pctx->lost_bytes += (pctx->pending_bytes-pframe->size);

                        // configure state to resume after data read
                        pctx->pbuf           = pctx->dest+pframe->size;
                        pctx->read_len       = 0;
                        pctx->header_pending = false;
                        pctx->data_pending   = true;
                        pctx->state          = MB1R_STATE_READ_OK;

                        // return to the state machine
                        pctx->sync_found     = true;
                        break;
                    }

                }else{
                    // invalid after all
                    // shift one byte and try again
                    pctx->psync++;
                    pctx->lost_bytes++;
                }

            }else{
                // finish reading in header
                // and continue

                // move the bytes we have to the
                // start of the buffer
                memmove(pctx->dest, pctx->psync, pctx->pending_bytes);

                // clean up buffer
                memset((pctx->dest + pctx->pending_bytes), 0, (pctx->len - pctx->pending_bytes));

                // configure state to resume header read
                pctx->pbuf           = pctx->dest + pctx->pending_bytes;
                pctx->read_bytes     = 0;
                pctx->read_len       = MB1_HEADER_BYTES - pctx->pending_bytes;
                pctx->frame_bytes    = pctx->pending_bytes;
                pctx->header_pending = true;
                pctx->data_pending   = true;
                pctx->state          = MB1R_STATE_READING;

                // return to the state machine
                pctx->sync_found     = true;
                break;
            }

        }else{
            // protocol version not valid
            // shift one byte and try again
            pctx->psync++;
            pctx->lost_bytes++;
        }
    }// while not sync

    if (pctx->sync_found==false) {
        // proto_version not found - restart
        MX_MMSG(MB1R_DEBUG, "INFO - MB1_TYPE_ID not found - restart\n");
        pctx->state=MB1R_STATE_START;
        pctx->lost_bytes+=(pctx->pbuf-pctx->psync);
    }else{
        retval=0;
    }

    return retval;
}

static int s_sm_act_read_header(mb1r_reader_t *self, mb1r_sm_ctx_t *pctx)
{
    int retval=-1;
    me_errno = ME_EINVAL;

    if (NULL!=self &&
        NULL!= pctx &&
        NULL!=mb1r_reader_sockif(self) &&
        NULL!=pctx->dest ) {

        pctx->sync_found = false;
        pctx->header_pending = true;
        pctx->pending_bytes = 0;

        while (pctx->state != MB1R_STATE_COMPLETE && pctx->state!=MB1R_STATE_READ_ERR) {

            if(errno==EINTR){
                // quit on user interrupt
                break;
            }

            switch (pctx->state) {
                case MB1R_STATE_START:
                    MX_MMSG(MB1R_DEBUG, "read_header MB1R_STATE_START\n");
                    pctx->read_len = MB1_HEADER_BYTES;
                    pctx->pbuf = pctx->dest;
                    pctx->header_pending = true;
                    pctx->frame_bytes = 0;
                    pctx->action = MB1R_ACTION_READ;
                    memset(pctx->dest,0,pctx->len);
                    break;
                case MB1R_STATE_READING:
                    MX_MMSG(MB1R_DEBUG, "read_header MB1R_STATE_READING\n");
                    // still reading
                    // don't touch inputs
                    pctx->action = MB1R_ACTION_READ;
                    break;
                case MB1R_STATE_READ_OK:
                    if (pctx->header_pending) {
                        MX_MMSG(MB1R_DEBUG, "read_header MB1R_STATE_READ_OK (header)\n");
                        pctx->header_pending = false;
                        pctx->action = MB1R_ACTION_VALIDATE_HEADER;
                    }
                    break;
                case MB1R_STATE_HEADER_VALID:
                    pctx->state = MB1R_STATE_HEADER_VALID;
                    pctx->action = MB1R_ACTION_QUIT;
                    retval=0;
                    MX_MMSG(MB1R_DEBUG, "read_header MB1R_STATE_HEADER_VALID\n");
                    break;
                case MB1R_STATE_HEADER_INVALID:
                     MX_LMSG(MB1R, 2, "read_header MB1R_STATE_HEADER_INVALID\n");
                    if ( (pctx->flags&MB1R_RESYNC_HEADER)!=0 ) {
                        // pbuf points to end of input...
                        // psync points 1 byte into invalid frame
                        MX_MMSG(MB1R_DEBUG, "read_header  RESYNC: header buffer:\n");
                        mb1_hex_show(pctx->dest,MB1_HEADER_BYTES,16,true,5);
                        MX_MPRINT(MB1R_DEBUG, "read_header dest[%p] pbuf[%p] ofs[%"PRId32"]\n", pctx->dest, pctx->pbuf,  (int32_t)(pctx->pbuf-pctx->dest));
                        MX_MPRINT(MB1R_DEBUG, "read_header read_len[%"PRIu32"]\n", pctx->read_len);
                        MX_MPRINT(MB1R_DEBUG, "read_header frame_bytes[%"PRId64"]\n", pctx->frame_bytes);
                        MX_MPRINT(MB1R_DEBUG, "read_header lost_bytes[%"PRId64"]\n", pctx->lost_bytes);

                        pctx->psync = pctx->dest+1;
                        pctx->lost_bytes+=1;
                        pctx->sync_found = false;
                        pctx->action = MB1R_ACTION_RESYNC;
                    }else{
                        // don't resync
                        pctx->action = MB1R_ACTION_QUIT;
                    }
                    break;
                default:
                    MX_MPRINT(MB1R_DEBUG, "read_header invalid state [%d]\n", pctx->state);
                    break;
            }// switch

            // actions...
            if(pctx->action == MB1R_ACTION_READ){
                pctx->read_bytes=0;
                while (pctx->read_bytes<pctx->read_len) {

                    MX_MPRINT(MB1R_DEBUG, "reading [%"PRId64"/%"PRIu32"] rto_ms[%"PRIu32"]\n", pctx->read_bytes, pctx->read_len,  pctx->timeout_msec);

                    if ( (pctx->read_bytes=msock_read_tmout(mb1r_reader_sockif(self), pctx->pbuf, pctx->read_len, pctx->timeout_msec)) == pctx->read_len) {

//                        MX_MPRINT(MB1R_DEBUG, "read OK [%"PRId64"]\n", pctx->read_bytes);

//                        pnf = (r7k_nf_t *)dest;
                        pctx->pbuf += pctx->read_bytes;
                        pctx->frame_bytes += pctx->read_bytes;
                        pctx->state=MB1R_STATE_READ_OK;

                    }else{
                        fprintf(stderr,"%s:%d errno[%d/%s] merrno[[%d/%s]\n",__func__,__LINE__,errno,strerror(errno),me_errno,me_strerror(me_errno));
                        MST_COUNTER_INC(self->stats->events[MB1R_EV_HDR_SHORT_READ]);
                        if (pctx->read_bytes>=0) {
                            // short read, keep going
                            pctx->read_len -= pctx->read_bytes;
                            pctx->pbuf += pctx->read_bytes;
                            pctx->frame_bytes += pctx->read_bytes;

                            if (me_errno==ME_ESOCK || me_errno==ME_EOF) {
                                fprintf(stderr,"%s:%d\n",__func__,__LINE__);
                                pctx->state=MB1R_STATE_READ_ERR;
                                MST_COUNTER_INC(self->stats->events[MB1R_EV_ESOCK]);
                                break;
                            }
                        }else{
                            fprintf(stderr,"%s:%d\n",__func__,__LINE__);
                            // error
                            pctx->state=MB1R_STATE_READ_ERR;
                            // let errorno bubble up
                            MST_COUNTER_INC(self->stats->events[MB1R_EV_EHDRREAD]);
                            break;
                        }
                    }// else incomplete read
                    if(errno==EINTR){
                        // quit on user interrupt
                        break;
                    }
                }
            }

            if(pctx->action == MB1R_ACTION_VALIDATE_HEADER){
                pctx->state = MB1R_STATE_HEADER_INVALID;
                pctx->cx=0;

                if(pctx->cx==0 && pctx->psnd->type != MB1_TYPE_ID){
                    MX_MPRINT(MB1R_DEBUG, "INFO - header  type invalid [%04X/%04X]\n", pctx->psnd->type, (uint32_t)MB1_TYPE_ID);
                    MST_COUNTER_INC(self->stats->events[MB1R_EV_EHDRTYPE]);
                    pctx->cx++;
                }

                if(pctx->cx==0 && pctx->psnd->size < MB1_EMPTY_SOUNDING_BYTES){
                    MX_MPRINT(MB1R_DEBUG, "INFO - header  size invalid [%"PRIu32"]\n", pctx->psnd->size);
                    MST_COUNTER_INC(self->stats->events[MB1R_EV_EHDRSZ]);
                    pctx->cx++;
                }

                if(pctx->cx==0 &&
                   (pctx->psnd->size != MB1_SOUNDING_BYTES(pctx->psnd->nbeams))){
                    MX_MPRINT(MB1R_DEBUG, "INFO - header  size invalid [%"PRIu32"/%"PRIu32"]\n", pctx->psnd->size, (uint32_t)MB1_SOUNDING_BYTES(pctx->psnd->nbeams));
                    MST_COUNTER_INC(self->stats->events[MB1R_EV_EHDRSZ]);
                    pctx->cx++;
                }

                if(pctx->cx==0 && pctx->psnd->ts < 0){
                    MX_MPRINT(MB1R_DEBUG, "INFO - header  timestamp invalid [%.2lf]\n", pctx->psnd->ts);
                    MST_COUNTER_INC(self->stats->events[MB1R_EV_EHDRTS]);
                    pctx->cx++;
                }

                if(pctx->cx==0){
                    pctx->state = MB1R_STATE_HEADER_VALID;
                }

            }

            if(pctx->action == MB1R_ACTION_RESYNC){
                if(s_sm_act_resync(self, pctx)==0){
                    MX_MMSG(MB1R_DEBUG, "INFO - header  sync OK\n");
                }else{
                    MX_MMSG(MB1R_DEBUG, "INFO - header  sync ERR\n");
                }
            }

            if(pctx->action == MB1R_ACTION_QUIT){
                pctx->state=MB1R_STATE_COMPLETE;
            }
        }// while
    }
    return retval;
}

static int s_sm_act_read_data(mb1r_reader_t *self, mb1r_sm_ctx_t *pctx)
{
    int retval=-1;
    me_errno = ME_EINVAL;

    if (NULL!=self &&
        NULL!= pctx &&
        NULL!=mb1r_reader_sockif(self) &&
        NULL!=pctx->dest ) {

        pctx->sync_found = false;
        pctx->pending_bytes = 0;

        while (pctx->state != MB1R_STATE_COMPLETE && pctx->state!=MB1R_STATE_READ_ERR) {

            if(errno==EINTR){
                // quit on user interrupt
                break;
            }

            switch (pctx->state) {
                case MB1R_STATE_START:
                    MX_MMSG(MB1R_DEBUG, "read_data MB1R_STATE_START\n");
                    pctx->read_len = MB1_BEAM_ARRAY_BYTES(pctx->psnd->nbeams)+MB1_CHECKSUM_BYTES;
                    pctx->pbuf = (byte *)MB1_PBEAMS(pctx->psnd);
                    MX_MPRINT(MB1R_DEBUG, "read_data pbuf[%p] rlen[%"PRIu32"]\n", pctx->pbuf, pctx->read_len);
                    pctx->action = MB1R_ACTION_READ;
                    break;
                case MB1R_STATE_READING:
                    MX_MMSG(MB1R_DEBUG, "read_data MB1R_STATE_READING\n");
                    // still reading
                    // don't touch inputs
                    pctx->action = MB1R_ACTION_READ;
                    break;
                case MB1R_STATE_READ_OK:
                    MX_MMSG(MB1R_DEBUG, "read_data MB1R_STATE_READ_OK\n");
                        pctx->action = MB1R_ACTION_VALIDATE_DATA;
                    break;
                case MB1R_STATE_DATA_VALID:
                    pctx->state = MB1R_STATE_DATA_VALID;
                    pctx->action = MB1R_ACTION_QUIT;
                    retval=0;
                    MX_MMSG(MB1R_DEBUG, "read_data MB1R_STATE_DATA_VALID\n");
                    break;
                case MB1R_STATE_DATA_INVALID:
                    MX_LMSG(MB1R, 2, "read_data MB1R_STATE_DATA_INVALID\n");
                    if ( (pctx->flags&MB1R_RESYNC_HEADER)!=0 ) {
                        // pbuf points to end of input...
                        // psync points 1 byte into invalid frame
                        MX_MMSG(MB1R_DEBUG, "read_data  RESYNC: header buffer:\n");
                        mb1_hex_show(pctx->dest,MB1_HEADER_BYTES,16,true,5);
                        MX_MPRINT(MB1R_DEBUG, "read_data dest[%p] pbuf[%p] ofs[%"PRId32"]\n", pctx->dest, pctx->pbuf, (int32_t)(pctx->pbuf-pctx->dest));
                        MX_MPRINT(MB1R_DEBUG, "read_data read_len[%"PRIu32"]\n", pctx->read_len);
                        MX_MPRINT(MB1R_DEBUG, "read_data frame_bytes[%"PRId64"]\n", pctx->frame_bytes);
                        MX_MPRINT(MB1R_DEBUG, "read_data lost_bytes[%"PRId64"]\n", pctx->lost_bytes);

                        pctx->psync = pctx->dest+1;
                        pctx->lost_bytes+=1;
                        pctx->sync_found = false;
                        pctx->action = MB1R_ACTION_RESYNC;
                    }else{
                        // don't resync
                        pctx->action = MB1R_ACTION_QUIT;
                    }
                    break;
                default:
                    MX_MPRINT(MB1R_DEBUG, "read_data invalid state [%d]\n", pctx->state);
                    sleep(1);
                    break;
            }// switch

            // actions...
            if(pctx->action == MB1R_ACTION_READ){
                pctx->read_bytes=0;
                while (pctx->read_bytes<pctx->read_len) {
                    if ( (pctx->read_bytes=msock_read_tmout(mb1r_reader_sockif(self), pctx->pbuf, pctx->read_len, pctx->timeout_msec)) == pctx->read_len) {
                        MX_MPRINT(MB1R_DEBUG, "read_data OK [%"PRId64"]\n", pctx->read_bytes);

                        pctx->pbuf += pctx->read_bytes;
                        pctx->frame_bytes += pctx->read_bytes;
                        pctx->state=MB1R_STATE_READ_OK;

                    }else{
                        fprintf(stderr,"%s:%d errno[%d/%s] merrno[[%d/%s]\n",__func__,__LINE__,errno,strerror(errno),me_errno,me_strerror(me_errno));

                        MX_MPRINT(MB1R_DEBUG, "read_data SHORT READ [%"PRId64"]\n", pctx->read_bytes);
                        MST_COUNTER_INC(self->stats->events[MB1R_EV_DATA_SHORT_READ]);
                        if (pctx->read_bytes>=0) {
                            // short read, keep going
                            pctx->read_len -= pctx->read_bytes;
                            pctx->pbuf += pctx->read_bytes;
                            pctx->frame_bytes += pctx->read_bytes;

                            if (me_errno==ME_ESOCK || me_errno==ME_EOF) {
                                fprintf(stderr,"%s:%d\n",__func__,__LINE__);
                                pctx->state=MB1R_STATE_READ_ERR;
                                MST_COUNTER_INC(self->stats->events[MB1R_EV_ESOCK]);
                                break;
                            }
                        }else{
                            fprintf(stderr,"%s:%d\n",__func__,__LINE__);
                            // error
                            pctx->state=MB1R_STATE_READ_ERR;
                            // let errorno bubble up
                            MST_COUNTER_INC(self->stats->events[MB1R_EV_EDATAREAD]);
                            break;
                        }
                    }// else incomplete read
                    if(errno==EINTR){
                        // quit on user interrupt
                        break;
                    }
                }
            }

            if(pctx->action == MB1R_ACTION_VALIDATE_DATA){
                pctx->state = MB1R_STATE_DATA_INVALID;
                pctx->cx=0;

                if(pctx->cx==0 && mb1_validate_checksum(pctx->psnd)!=0 ){
                    MX_MPRINT(MB1R_DEBUG, "INFO - read_data checksum invalid [%08X/%08X]\n", MB1_GET_CHECKSUM(pctx->psnd), mb1_calc_checksum(pctx->psnd));
                    MST_COUNTER_INC(self->stats->events[MB1R_EV_ECHKSUM]);
                    pctx->cx++;
                }

                if(pctx->cx==0){
                    pctx->state = MB1R_STATE_DATA_VALID;
                }

            }

            if(pctx->action == MB1R_ACTION_RESYNC){
                if(s_sm_act_resync(self, pctx)==0){
                    MX_MMSG(MB1R_DEBUG, "INFO - read_data  sync OK\n");
                }else{
                    MX_MMSG(MB1R_DEBUG, "INFO - read_data  sync ERR\n");
                }
            }

            if(pctx->action == MB1R_ACTION_QUIT){
                pctx->state=MB1R_STATE_COMPLETE;
            }
        }// while

    }
    return retval;
}

static int s_sm_update(mb1r_reader_t *self, mb1r_sm_ctx_t *pctx)
{

    int retval = 0;
    switch (pctx->state) {

        case MB1R_STATE_START:
            MX_MMSG(MB1R_DEBUG, "update MB1R_STATE_START\n");
            pctx->read_len = MB1_HEADER_BYTES;
            // resync on failed header read(?)
            pctx->rflags = MB1R_RESYNC_HEADER;
            pctx->action = MB1R_ACTION_READ_HEADER;

            pctx->pbuf = pctx->dest;
            pctx->psnd = (mb1_t *)pctx->dest;
            memset(pctx->dest,0,pctx->len);
            pctx->frame_bytes=0;
            break;

        case MB1R_STATE_HEADER_VALID:
            MX_MMSG(MB1R_DEBUG, "update MB1R_STATE_HEADER_VALID\n");
            // next, read the data and checksum
            pctx->read_len = MB1_BEAM_ARRAY_BYTES(pctx->psnd->nbeams)+MB1_CHECKSUM_BYTES;
            // don't resync on failed data read
            pctx->rflags = 0;
            pctx->state = MB1R_STATE_START;
            pctx->action = MB1R_ACTION_READ_DATA;

            break;

        case MB1R_STATE_DATA_VALID:
            MX_MMSG(MB1R_DEBUG, "update MB1R_STATE_DATA_VALID\n");
            // done
            pctx->action = MB1R_ACTION_QUIT;
            pctx->state = MB1R_STATE_FRAME_VALID;
            break;

        case MB1R_STATE_HEADER_INVALID:
            // MX_MMSG(MB1R_DEBUG, "MB1R_STATE_HEADER_INVALID (retrying)\n");
            pctx->state  = MB1R_STATE_START;
            pctx->action = MB1R_ACTION_NOOP;
            break;

        case MB1R_STATE_DATA_INVALID:
            MX_MMSG(MB1R_DEBUG, "MB1R_STATE_DATA_INVALID (retrying)\n");
            pctx->state  = MB1R_STATE_START;
            pctx->action = MB1R_ACTION_NOOP;
            break;

        case MB1R_STATE_READ_ERR:
            MX_MMSG(MB1R_DEBUG, "MB1R_STATE_READ_ERR\n");
            if (pctx->merrno==ME_ESOCK) {
                MX_MMSG(MB1R_DEBUG, "socket disconnected - quitting\n");
            }else if (pctx->merrno==ME_EOF) {
                MX_MMSG(MB1R_ERROR, "end of file\n");
            }else if (pctx->merrno==ME_ENOSPACE) {
                MX_MPRINT(MB1R_ERROR, "buffer full [%"PRIu32"/%"PRIu32"]\n", (uint32_t)(pctx->pbuf+pctx->read_len-pctx->dest),pctx->len);
            }else{
                MX_MPRINT(MB1R_ERROR, "read error [%d/%s]\n", pctx->merrno, me_strerror(pctx->merrno));
            }
            pctx->action   = MB1R_ACTION_QUIT;
            break;

        default:
            MX_MPRINT(MB1R_ERROR, "ERR - unknown state[%d]\n", pctx->state);
            retval=-1;
            break;
    }
    return retval;
}

/// @fn int64_t mb1r_read_frame(mb1r_reader_t *self, uint32_t nframes, byte *dest, uint32_t len, uint32_t flags, double newer_than, uint32_t timeout_msec )
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
int64_t mb1r_read_frame(mb1r_reader_t *self,
                        byte *dest,
                        uint32_t len,
                        mb1r_flags_t flags,
                        double newer_than,
                        uint32_t timeout_msec,
                        uint32_t *sync_bytes )
{

    int64_t retval=-1;
    me_errno = ME_OK;

    mb1r_sm_ctx_t ctx_s = {0}, *pctx = &ctx_s;
    pctx->state = MB1R_STATE_START;
    pctx->action = MB1R_ACTION_QUIT;
    pctx->dest = dest;
    pctx->len = len;
    pctx->flags = 0;
    pctx->newer_than = newer_than;
    pctx->timeout_msec = timeout_msec;
    pctx->sync_bytes = sync_bytes;
    pctx->rflags = 0;
    pctx->pbuf = dest;
    pctx->psnd = (mb1_t *)dest;
    pctx->psync = dest;
    pctx->frame_bytes = 0;
    pctx->lost_bytes = 0;
    pctx->pending_bytes = 0;
    pctx->header_bytes = 0;
    pctx->data_bytes = 0;
    pctx->read_bytes = 0;
    pctx->read_len = 0;

    msock_socket_t *sockif = mb1r_reader_sockif(self);

    if (NULL!=self && NULL!=dest &&
        NULL!=sockif && sockif->fd>0
        ) {

        while (pctx->state != MB1R_STATE_FRAME_VALID ) {

            s_sm_update(self, pctx);


            if (pctx->action==MB1R_ACTION_READ_HEADER) {

                if ( s_sm_act_read_header(self,  pctx) == 0) {
                    MX_MMSG(MB1R_DEBUG, "read_frame HEADER read OK\n");
                    pctx->state = MB1R_STATE_HEADER_VALID;
                    pctx->action = MB1R_ACTION_NOOP;
                }else{
                    MX_MPRINT(MB1R_ERROR, "read_frame ERR - mb1r_read_hdr read_bytes[%"PRId64"] [%d/%s]\n", pctx->read_bytes, me_errno, me_strerror(me_errno));
                    mb1r_ctx_show(pctx, true, 5);
                    pctx->merrno = me_errno;
                    pctx->state = MB1R_STATE_READ_ERR;
                }
            }

            if (pctx->action==MB1R_ACTION_READ_DATA) {
                if ( s_sm_act_read_data(self, pctx) == 0) {
                    MX_MMSG(MB1R_DEBUG, "read_frame DATA read OK\n");
                    pctx->state = MB1R_STATE_DATA_VALID;
                    pctx->action = MB1R_ACTION_NOOP;
                }else{
                    MX_MPRINT(MB1R_ERROR, "read_frame ERR - mb1r_read_data read_bytes[%"PRId64"] [%d/%s]\n",pctx->read_bytes,me_errno,me_strerror(me_errno));
                    mb1r_ctx_show(pctx, true, 5);
                    pctx->merrno = me_errno;
                    pctx->state = MB1R_STATE_READ_ERR;
                }
            }

            if (pctx->action==MB1R_ACTION_QUIT) {

                // MX_MMSG(MB1R_DEBUG, "ACTION_QUIT\n");
                if (pctx->state==MB1R_STATE_FRAME_VALID) {
                    retval = pctx->frame_bytes;
                    MX_LPRINT(MB1R, 2, "read_frame Frame valid - returning[%"PRId64"]\n", retval);
                    MST_COUNTER_ADD(self->stats->status[MB1R_STA_FRAME_VAL_BYTES],pctx->frame_bytes);

                    //                    MST_COUNTER_INC(self->stats->events[MB1R_EV_FRAME_VALID]);
                    //                    mb1_nf_show((mb1_nf_t *)dest,false,5);
                    //                    mb1_drf_show((mb1_drf_t *)(dest+MB1_NF_BYTES),false,5);
                    //                    mb1_hex_show(dest,frame_bytes,16,true,5);
                    //                    if (self->log_id!=MLOG_ID_INVALIDstream) {
                    //                       // TODO : replace with plain file
                    //                        fwrite(dest,frame_bytes,1,self->logstream);
                    //                    }

                    if (self->log_id!=MLOG_ID_INVALID) {
                        mlog_write(self->log_id,pctx->dest,pctx->frame_bytes);
                    }

                }else{
                    MX_MPRINT(MB1R_DEBUG, "read_frame Frame invalid [%d/%s] retval[%"PRId64"]\n", me_errno, me_strerror(me_errno), retval);
                    MST_COUNTER_INC(self->stats->events[MB1R_EV_FRAME_INVALID]);
                }
                // quit and return
                break;
            }

        }// while frame invalid

    }else{
        MX_ERROR_MSG("read_frame invalid argument\n");
    }
    MX_LPRINT(MB1R, 2, "mb1r_read_frame returning [%"PRId64"]\n", retval);
    return retval;
 }
// End function mb1r_read_frame

/// @fn void void mb1r_reader_purge(mb1r_reader_t *self)
/// @brief empty reader frame container.
/// @param[in] self reader reference
/// @return none;
void mb1r_reader_purge(mb1r_reader_t *self)
{
    // do nothing
    fprintf(stderr,"%s - WARN not implemented\n",__func__);
}
// End function mb1r_reader_purge

#ifdef WITH_MB1R_PEER_CMP

/// @fn _Bool mb1r_peer_cmp(void * a, void * b)
/// @brief compare msock_connection_t IDs. Used by mlist.
/// @param[in] a void pointer to msock_connection_t
/// @param[in] b void pointer to msock_connection_t
/// @return true if peer id's are equal, false otherwise
bool mb1r_peer_cmp(void *a, void *b)
{
    bool retval=false;
    if (a!=NULL && b!=NULL) {
        msock_connection_t *pa = (msock_connection_t *)a;
        msock_connection_t *pb = (msock_connection_t *)b;
        retval = (pa->id == pb->id);
    }
    return retval;
}
// End function mb1r_peer_cmp


/// @fn _Bool mb1r_peer_vcmp(void * item, void * value)
/// @brief compare msock_connection_t ID to specified value. Used by mlist.
/// @param[in] item void pointer to msock_connection_t
/// @param[in] value void pointer to integer (id value)
/// @return returns true if the specified peer has the specified ID. false otherwise
bool mb1r_peer_vcmp(void *item, void *value)
{
    bool retval=false;
    if (NULL!=item && NULL!=value) {
        msock_connection_t *peer = (msock_connection_t *)item;
        int svc = *((int *)value);
//        MX_MPRINT(MB1R_DEBUG, "peer[%p] id[%d] svc[%d]\n", peer, peer->id, svc);
        retval = (peer->id == svc);
    }
    return retval;
}
// End function mb1r_peer_vcmp
#endif // WITH_MB1R_PEER_CMP

#ifdef WITH_MB1R_TEST
#include <getopt.h>
#include <mthread.h>

typedef struct mb1_test_cfg_s{
    char *host;
    int port;
    int cycles;
    int retries;
    int err_mod;
    uint32_t test_beams;
    int verbose;
    int stop_req;
}mb1_test_cfg_t;

// test frame generator (thread worker)
static void *s_test_worker(void *pargs)
{
    mb1_test_cfg_t *cfg = (mb1_test_cfg_t *)pargs;
    if(NULL!=cfg){

        struct timeval tv;
        fd_set active_set;
        fd_set read_fds;
        fd_set write_fds;
        fd_set err_fds;
        byte iobuf[MB1_MAX_SOUNDING_BYTES]; // buffer for client data
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

        tv.tv_sec = 3;
        tv.tv_usec = 0;

        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        FD_ZERO(&err_fds);
        FD_ZERO(&active_set);
        // add server socket to active_set set
        FD_SET(s->fd,&active_set);
        fdmax = s->fd;
        int cx = 0;
        while(cfg->stop_req==0){
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
                        // MX_INFO("readfs [%d/%d] selected\n", i, fdmax);
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
                                // MX_INFO("accept failed [%d/%s]\n", errno, strerror(errno));
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
                                } else if(nbytes<0) {
                                    fprintf(stderr,"ERR - recv failed socket[%d] [%d/%s]\n",i,errno,strerror(errno));
                                    if(errno!=EAGAIN){
                                        do_close=true;
                                    }
                                }
                            }
                        }
                    }else{
                        // fd[i] not ready to read
//                        MX_INFO("readfs fd[%d/%d] ISSET:%s\n", i, fdmax, (FD_ISSET(i,&read_fds)?"TRUE":"FALSE"));
                    }

                    if (FD_ISSET(i, &err_fds)){
                        if (i==s->fd) {
                            fprintf(stderr,"server socket err fd[%d]--stopping\n",i);
                            cfg->stop_req = 1;
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
                            mb1_t *snd = (mb1_t *)iobuf;
                            uint32_t test_beams = cfg->test_beams;
                            snd->type = MB1_TYPE_ID;
                            snd->size = MB1_SOUNDING_BYTES(test_beams);
                            snd->nbeams = test_beams;
                            snd->ping_number = cx;
                            snd->ts = mtime_dtime();
                            unsigned int k=0;
                            for(k=0;k<test_beams;k++){
                                snd->beams[k].beam_num=k;
                                snd->beams[k].rhox = cx*1.;
                                snd->beams[k].rhoy = cx*2.;
                                snd->beams[k].rhoz = cx*4.;
                            }

                            mb1_set_checksum(snd);

                            if(cfg->err_mod>0 && ++cx%cfg->err_mod==0){
                                snd->ts+=1;
                                fprintf(stderr,"!!! server generating invalid frame !!!\n");
                            }
                            int nbytes = send(i,iobuf,MB1_SOUNDING_BYTES(test_beams),0);

                            fprintf(stderr,"server sent frame len[%d]:\n",nbytes);
                            mb1_show(snd,true,5);
                            fprintf(stderr,"\n");
                            mb1_hex_show((byte *)snd,snd->size,16,true,5);
                        }
                    }

                    if (do_close) {
                        fprintf(stderr,"ERR - closing fd[%d]\n", i);
                        FD_CLR(i, &active_set); // remove from active_set
                        close(i); // bye!
                    }
                }// for client

            }else{
                // select failed
                //                    MX_INFO("select failed stat[%d] [%d/%s]\n", stat, errno, strerror(errno));
                tv.tv_sec = 3;
                tv.tv_usec = 0;
            }
            sleep(1);
        }//while
        fprintf(stderr,"server stop_req set--exiting\n");
        close(s->fd);
    }
    pthread_exit((void *)NULL);
    return (void *)NULL;
}


int mb1r_test(int argc, char **argv)
{
    int retval=-1;

    int i=0;
    mb1_test_cfg_t cfg_s = {
        "localhost",
        MB1_IP_PORT_DFL,
        3,// cycles
        5,// retries
        3,// err_mod
        4,// test_beams
        1,// verbose
        0 // stop_req
    }, *cfg = &cfg_s;

    int errors=0;
    for(i=1;i<argc;i++){
        char *ap=strstr(argv[i],"--host=");
        if(ap!=NULL){
            cfg->host=ap+strlen("--host=");
            continue;
        }
        ap=strstr(argv[i],"--port=");
        if(ap!=NULL){
            sscanf(ap+strlen("--port="),"%d",&cfg->port);
            continue;
        }
        ap=strstr(argv[i],"--verbose=");
        if(ap!=NULL){
            sscanf(ap+strlen("--verbose="),"%d",&cfg->verbose);
            continue;
        }
        ap=strstr(argv[i],"--retries=");
        if(ap!=NULL){
            sscanf(ap+strlen("--retries="),"%d",&cfg->retries);
            continue;
        }
        ap=strstr(argv[i],"--cycles=");
        if(ap!=NULL){
            sscanf(ap+strlen("--cycles="),"%d",&cfg->cycles);
            continue;
        }
        ap=strstr(argv[i],"--emod=");
        if(ap!=NULL){
            sscanf(ap+strlen("--emod="),"%d",&cfg->err_mod);
            continue;
        }
        ap=strstr(argv[i],"--beams=");
        if(ap!=NULL){
            sscanf(ap+strlen("--beams="),"%"PRIu32"",&cfg->test_beams);
            continue;
        }
        fprintf(stderr,"  Options : \n");
        fprintf(stderr,"   --verbose=n      : output level (n>=0)\n");
        fprintf(stderr,"   --host=<ip_addr> : TRN host IP address\n");
        fprintf(stderr,"   --port=<op_port> : TRN host IP address\n");
        fprintf(stderr,"   --cycles=n       : number of frames to read\n");
        fprintf(stderr,"   --retries=n      : reconnection retries\n");
        fprintf(stderr,"   --emod=n         : error every n frames (<=0 to disable)\n");
        fprintf(stderr,"   --beams=u        : test frame beams\n");
        fprintf(stderr,"   --help=n         : show use info\n\n");
        exit(0);

    }

    if(cfg->verbose>1){
        fprintf(stderr,"host    : [%s]\n",cfg->host);
        fprintf(stderr,"port    : [%d]\n",cfg->port);
        fprintf(stderr,"cycles  : [%d]\n",cfg->cycles);
        fprintf(stderr,"retries : [%d]\n",cfg->retries);
        fprintf(stderr,"err_mod : [%d]\n",cfg->err_mod);
        fprintf(stderr,"verbose : [%d]\n",cfg->verbose);
    }

    mxd_setModule(MB1R, 1, false, "MB1R");
    mxd_setModule(MB1R_ERROR, 1, false, "MB1R_ERR");
    mxd_setModule(MB1R_DEBUG, 1, false, "MB1R_DEBUG");
    mxd_setModule(MB1R, 1, false, "MB1R");

    mthread_thread_t *worker = mthread_thread_new();
    if(mthread_thread_start(worker, s_test_worker, (void *)cfg)!=0){
        fprintf(stderr,"ERR - mthread_thread_start\n");
    }
    sleep(1);

    // initialize reader
    // create and open socket connection
    mb1r_reader_t *reader = mb1r_reader_new(cfg->host, cfg->port, MB1_MAX_SOUNDING_BYTES);

    // show reader config
    if(cfg->verbose>1)
    mb1r_reader_show(reader,true, 5);

    uint32_t lost_bytes=0;
    // test mb1r_read_frame
    byte frame_buf[MB1_MAX_SOUNDING_BYTES]={0};
    int frames_read=0;

    if(cfg->verbose>1)
        fprintf(stderr,"connecting reader [%s/%d]\n",cfg->host,cfg->port);

    int retries=cfg->retries;
    while ( (frames_read<cfg->cycles) && retries>0 ) {
         // clear frame buffer
        memset(frame_buf,0,MB1_MAX_SOUNDING_BYTES);
        // read frame
        int istat=0;
        fprintf(stderr,"reading sounding ret[%d]\n",retries);
        if( (istat = mb1r_read_frame(reader, frame_buf, MB1_MAX_SOUNDING_BYTES, MB1R_NOFLAGS, 0.0, MB1R_READ_TMOUT_MSEC, &lost_bytes )) > 0){

            frames_read++;

            if(cfg->verbose>0)
                fprintf(stderr,"mb1r_read_frame cycle[%d/%d] lost[%u] ret[%d]\n", frames_read,
                        cfg->cycles,lost_bytes,istat);
            // show contents
            if (cfg->verbose>=1) {
                MX_LMSG(MB1R, 1, "MB1:\n");
//                mb1_t *psnd = (mb1_t *)(frame_buf);
//                mb1_show(psnd,false,5);

                if(cfg->verbose>1){
                    MX_LMSG(MB1R, 1, "data:\n");
                    mb1_hex_show(frame_buf,istat,16,true,5);
                }
            }
        }else{
            retries--;
            errors++;
            // read error
            fprintf(stderr,"ERR - mb1r_read_frame - cycle[%d/%d] ret[%d] lost[%u] err[%d/%s]\n",frames_read+1,cfg->cycles,istat,lost_bytes,errno,strerror(errno));
            if (errno==ECONNREFUSED || me_errno==ME_ESOCK || me_errno==ME_EOF || me_errno==ME_ERECV) {

                fprintf(stderr,"socket closed - reconnecting in 5 sec\n");
                sleep(5);
                mb1r_reader_connect(reader,true);
            }
        }
    }

    cfg->stop_req=1;
    fprintf(stderr,"joining worker\n");
    mthread_thread_join(worker);
    fprintf(stderr,"releasing worker\n");
    mthread_thread_destroy(&worker);

    fprintf(stderr,"releasing reader\n");
    mb1r_reader_destroy(&reader);

    if(frames_read==cfg->cycles)
        retval=0;

    if(cfg->verbose>0)
    fprintf(stderr,"frames[%d/%d]  retries[%d] lost[%u] errors[%d]\n",
            frames_read, cfg->cycles, cfg->retries-retries, lost_bytes, errors);
    return retval;
}
#endif // WITH_MB1R_TEST
