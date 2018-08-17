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
#if defined(__CYGWIN__)
//#define _XOPEN_SOURCE
//#define __USE_XOPEN
//#define _GNU_SOURCE
#include <Windows.h>
#endif


#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <errno.h>

#include "mbtrn.h"
#include "iowrap.h"
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
const char *mbtr_stevent_labels[]={ \
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
const char *mbtr_ststatus_labels[]={ \
    "frame_valid_bytes",
    "nf_valid_bytes",
    "drf_valid_bytes",
    "nf_inval_bytes",
    "drf_inval_bytes",
    "sub_frames"
};
const char *mbtr_stchan_labels[]={ \
    "mbtrn_refill_xt"
};

const char **mbtr_stats_labels[MBTR_LABEL_COUNT]={
    mbtr_stevent_labels,
    mbtr_ststatus_labels,
    mbtr_stchan_labels
};

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////


/////////////////////////
// Function Definitions
/////////////////////////

/// @fn const char *mbtrn_get_version()
/// @brief get version string.
/// @return version string
const char *mbtrn_get_version()
{
    return LIBMBTRN_VERSION;
}
/// @fn const char *mbtrn_get_build()
/// @brief get build string.
/// @return version string
const char *mbtrn_get_build()
{
    return LIBMBTRN_BUILD;
}
/// @fn void mbtrn_show_app_version(const char *app_version)
/// @brief get version string.
/// @return version string
void mbtrn_show_app_version(const char *app_name, const char *app_version)
{
    printf("\n %s built[%s] libmbtrn[v%s / %s] \n\n",app_name, app_version, mbtrn_get_version(),mbtrn_get_build());
}

/// @fn int mbtrn_reader_connect(*self)
/// @brief connect to 7k center and subscribe to records.
/// @param[in] self reader reference
/// @return 0 on success, -1 otherwise
int mbtrn_reader_connect(mbtrn_reader_t *self, bool replace_socket)
{
    int retval=-1;
    me_errno = ME_OK;
    
    if (NULL!=self) {
        
        char *host = strdup(self->sockif->addr->host);
        int port = self->sockif->addr->port;

        if (replace_socket) {
            MMDEBUG(MBTRN,"destroying socket\n");
            iow_socket_destroy(&self->sockif);
            
            MMDEBUG(MBTRN,"building socket\n");
            self->sockif = iow_socket_new(host,port,ST_TCP);
        }

        MMDEBUG(MBTRN,"connecting to 7k center [%s]\n",self->sockif->addr->host);
        if(iow_connect(self->sockif)==0){
            self->state=MBS_CONNECTED;
            MMDEBUG(MBTRN,"subscribing to 7k center [%s]\n",self->sockif->addr->host);
            if (r7k_subscribe(mbtrn_reader_sockif(self),self->sub_list,self->sub_count)==0) {
                self->state=MBS_SUBSCRIBED;
                retval=0;
            }else{
                MMDEBUG(MBTRN,"subscribe failed [%s]\n",self->sockif->addr->host);
                me_errno=ME_ESUB;
                self->state=MBS_INITIALIZED;
            }
        }else{
            MMDEBUG(MBTRN,"connect failed [%s]\n",self->sockif->addr->host);
            me_errno=ME_ECONNECT;
            self->state=MBS_INITIALIZED;
            mbtrn_reader_reset_socket(self);
        }
    }
    return retval;
}
// End function mbtrn_reader_connect

/// @fn mbtrn_reader_t * mbtrn_reader_new(const char * host, int port, uint32_t capacity, uint32_t * slist, uint32_t slist_len)
/// @brief create new reson 7k reader. connect and subscribe to reson data.
/// @param[in] host reson 7k center host IP address
/// @param[in] port reson 7k center host IP port
/// @param[in] capacity input buffer capacity (bytes)
/// @param[in] slist 7k center message subscription list
/// @param[in] slist_len length of subscription list
/// @return new reson reader reference on success, NULL otherwise
mbtrn_reader_t *mbtrn_reader_new(const char *host, int port, uint32_t capacity, uint32_t *slist,  uint32_t slist_len)
{
    mbtrn_reader_t *self = (mbtrn_reader_t *)malloc(sizeof(mbtrn_reader_t));
    if (NULL != self) {
        
        self->state=MBS_NEW;
        self->fileif=NULL;
        self->sockif = iow_socket_new(host,port,ST_TCP);
        self->sub_list = (uint32_t *)malloc(slist_len*sizeof(uint32_t));
        memset(self->sub_list,0,slist_len*sizeof(uint32_t));
        self->sub_count = slist_len;
        memcpy(self->sub_list,slist,slist_len*sizeof(uint32_t));
        self->fc = r7k_drfcon_new(capacity);
        self->log=NULL;
        self->logstream=NULL;
        self->state=MBS_INITIALIZED;
        
        if (NULL != self->sockif) {
            mbtrn_reader_connect(self, false);
        }else{
            self->state=ME_ECREATE;
        }
        self->stats=mbtrn_stats_new(MBTR_EV_COUNT, MBTR_STA_COUNT, MBTR_CH_COUNT, mbtr_stats_labels);
    }
    
    return self;
}
// End function mbtrn_reader_create

/// @fn mbtrn_reader_t * mbtrn_freader_new(iow_file_t *file, uint32_t capacity, uint32_t * slist, uint32_t slist_len)
/// @brief create new reson 7k reader. connect and subscribe to reson data.
/// @param[in] path input file path
/// @param[in] capacity input buffer capacity (bytes)
/// @param[in] slist 7k center message subscription list
/// @param[in] slist_len length of subscription list
/// @return new reson reader reference on success, NULL otherwise
mbtrn_reader_t *mbtrn_freader_new(iow_file_t *file, uint32_t capacity, uint32_t *slist,  uint32_t slist_len)
{
    mbtrn_reader_t *self = (mbtrn_reader_t *)malloc(sizeof(mbtrn_reader_t));
    if (NULL != self) {
        
        self->state  = MBS_NEW;
        self->sockif = NULL;
        self->fileif = file;
        if (NULL!=file) {
            if (iow_open(self->fileif, IOW_RONLY)<=0) {
                fprintf(stderr,"ERR - could not open file [%s] [%d/%s]\n",self->fileif->path,errno,strerror(errno));
            }
            
            fprintf(stderr,"wrapping fd %d for file %s in socket\n",self->fileif->fd,self->fileif->path);
            // wrap file descriptor in socket
            // so it can be passed to read
            self->sockif = iow_wrap_fd(self->fileif->fd);
        }
        
        self->sub_list = (uint32_t *)malloc(slist_len*sizeof(uint32_t));
        memset(self->sub_list,0,slist_len*sizeof(uint32_t));
        self->sub_count = slist_len;
        memcpy(self->sub_list,slist,slist_len*sizeof(uint32_t));
        self->fc = r7k_drfcon_new(capacity);
        
        self->state=MBS_INITIALIZED;
        
        self->stats=mbtrn_stats_new(MBTR_EV_COUNT, MBTR_STA_COUNT, MBTR_CH_COUNT, mbtr_stats_labels);
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
            if (self->sub_list) {
                free(self->sub_list);
            }
            if (self->fc) {
                r7k_drfcon_destroy(&self->fc);
            }
            if (self->stats) {
                mbtrn_stats_destroy(&self->stats);
            }
            
            free(self);
            *pself = NULL;
        }
    }
}
// End function mbtrn_reader_destroy

void mbtrn_reader_reset_socket(mbtrn_reader_t *self)
{
    if (NULL!=self) {
        close(self->sockif->fd);
        self->sockif->fd=-1;
        self->sockif->status=SS_CONFIGURED;
    }
}

/// @fn void mbtrn_reader_setlog(mbtrn_reader_t *self, mlog_t *log, mlog_id_t id, const char *desc)
/// @brief set logger
/// @param[in] self pointer to instance
/// @param[in] log  pointer to log instance
/// @return none
void mbtrn_reader_set_log(mbtrn_reader_t *self, mlog_t *log, mlog_id_t id, char *desc)
{
    if (NULL!=self) {
        if (self->log!=NULL) {
            mlog_delete(self->log_id);
            mlog_destroy(&self->log);
            self->log=NULL;
        }
        self->log=log;
        self->log_id=id;
        mlog_add(self->log,id,desc);
    }
    return;
}

/// @fn void mbtrn_reader_set_logstream(mbtrn_reader_t *self, FILE *log)
/// @brief set logger
/// @param[in] self pointer to instance
/// @param[in] log  pointer to log instance
/// @return none
void mbtrn_reader_set_logstream(mbtrn_reader_t *self, FILE *log)
{
    if (NULL!=self) {
        if (self->log!=NULL) {
            fclose(self->logstream);
        }
        self->logstream=log;
    }
    return;
}

/// @fn int mbtrn_reader_set_file(mbtrn_reader_t *self, iow_file_t *file)
/// @brief set current reader file
/// @param[in] self pointer to instance
/// @param[in] file  pointer to file
/// @return none
int mbtrn_reader_set_file(mbtrn_reader_t *self, iow_file_t *file)
{
    int retval=-1;

    if (NULL!=self && NULL!=file) {
        iow_close(self->fileif);
        iow_socket_destroy(&self->sockif);
        self->sockif=NULL;
        self->fileif=file;
        if (iow_open(self->fileif, IOW_RONLY)>0){
            // wrap file descriptor in socket
            // so it can be passed to read
            self->sockif = iow_wrap_fd(self->fileif->fd);
            retval=0;
        }else{
            MMERROR(MBTRN,"ERR - could not open file [%s] [%d/%s]\n",self->fileif->path,errno,strerror(errno));
        }
    }
    return retval;
}

/// @fn  mbtrn_stats_t *mbtrn_stats_new()
/// @brief create new stats structure
/// @return mbtrn_stats_t
mbtr_stats_t *mbtrn_stats_new(uint32_t ev_counters, uint32_t st_counters, uint32_t tm_channels, const char ***labels)
{
    mbtr_stats_t *self = (mbtr_stats_t *)malloc(sizeof(mbtr_stats_t));
    if (self) {
        memset(self,0,sizeof(mbtr_stats_t));
        self->ev_n=ev_counters;
        self->st_n=st_counters;
        self->tm_n=tm_channels;
        self->per_stats = (mbtrn_stat_chan_t *)malloc(tm_channels*sizeof(mbtrn_stat_chan_t));
        self->agg_stats = (mbtrn_stat_chan_t *)malloc(tm_channels*sizeof(mbtrn_stat_chan_t));
        self->events    = (uint32_t *)malloc(ev_counters*sizeof(double));
        self->status    = (uint32_t *)malloc(st_counters*sizeof(double));
        self->measurements    = (mbtrn_cmeas_t *)malloc(tm_channels*sizeof(mbtrn_cmeas_t));
        
        self->stat_period_start = 0.0;
        self->stat_period_sec   = 0.0;
        self->labels            = labels;
        memset(self->per_stats,    0, tm_channels*sizeof(mbtrn_stat_chan_t));
        memset(self->agg_stats,    0, tm_channels*sizeof(mbtrn_stat_chan_t));
        memset(self->events,       0, ev_counters*sizeof(uint32_t));
        memset(self->status,       0, st_counters*sizeof(uint32_t));
        memset(self->measurements, 0, tm_channels*sizeof(mbtrn_cmeas_t));
    }
    return self;
}

/// @fn  void mbtrn_stats_destroy(mbtrn_stats_t **pself)
/// @brief release mbtrn_stats_t resources
/// @return none
void mbtrn_stats_destroy(mbtr_stats_t **pself)
{
    if (NULL!=pself) {
        mbtr_stats_t *self=(*pself);
        free(self->events);
        free(self->status);
        free(self->measurements);
        free(self->per_stats);
        free(self->agg_stats);
        free(self);
        *pself=NULL;
    }
}

void mbtrn_stats_set_period(mbtr_stats_t *self, double period_start, double period_sec)
{
    if (NULL!=self) {
        self->stat_period_start=period_start;
        self->stat_period_sec=period_sec;
    }
}

int mbtrn_log_timing(mlog_id_t log_id, mbtrn_stat_chan_t *stats,  double timestamp, char *type_str, const char **labels, int channels)
{
    if (NULL!=stats && NULL!=labels && channels>0) {
        for (int i=0; i<channels ; i++){
            mlog_tprintf(log_id,"%.3lf,%s,%s,%llu,%1.3g,%1.3g,%1.3g\n",
                         timestamp,
                         type_str,
                         labels[i],
                         stats[i].n,
                         stats[i].min,
                         stats[i].max,
                         stats[i].avg);
        }
    }
    return 0;
}

int mbtrn_log_counts(mlog_id_t log_id, uint32_t *counts, double timestamp, char *type_str, const char **labels, int channels)
{
    if (NULL!=counts && NULL!=labels && channels>0) {
        for (int i=0; i<channels ; i++){
            mlog_tprintf(log_id,"%.3lf,%s,%s,%"PRIu32"\n",
                         timestamp,
                         type_str,
                         labels[i],
                         counts[i]);
        }
    }
    return 0;
}

int mbtrn_log_stats(mbtr_stats_t *stats,
                    double now,
                    mlog_id_t log_id,
                    mbtr_stat_flags flags)
{
    int retval=-1;
    if (NULL!=stats) {

        if (flags&MBTF_STATUS) {
            // log status
            mbtrn_log_counts( log_id, stats->status,   now, "s", stats->labels[MBTR_LABEL_ST], stats->st_n);
        }
        if (flags&MBTF_EVENT) {
            // log events
            mbtrn_log_counts( log_id, stats->events,   now, "e", stats->labels[MBTR_LABEL_EV],  stats->ev_n);
        }
        if (flags&MBTF_PSTAT) {
            // log period stats
            mbtrn_log_timing( log_id, stats->per_stats, now, "p", stats->labels[MBTR_LABEL_ME],  stats->tm_n);
        }
        if (flags&MBTF_ASTAT) {
            // log aggregate statistics
            mbtrn_log_timing( log_id, stats->agg_stats, now, "a", stats->labels[MBTR_LABEL_ME],  stats->tm_n);
        }
        retval=0;
    }
    return retval;
}

mbtr_stats_t *mbtrn_reader_get_stats(mbtrn_reader_t *self)
{
    mbtr_stats_t *retval=NULL;
    if (NULL!=self) {
        retval=self->stats;
    }
    return retval;
}

const char ***mbtrn_reader_get_statlabels()
{
    return mbtr_stats_labels;
}
void mbtrn_reset_pstats(mbtr_stats_t *stats, uint32_t channels)
{
    if (NULL != stats && channels>0) {
        // reset periodic stats
        memset(stats->per_stats,0,(channels*sizeof(mbtrn_stat_chan_t)));
    }
}


int mbtrn_update_stats(mbtr_stats_t *stats, uint32_t channels, mbtr_stat_flags flags)
{
    if (NULL!=stats) {

        
        // update all stats channels
        for (uint32_t i=0; i<channels ; i++){
            // update periodic stats
            stats->per_stats[i].n++;
            stats->per_stats[i].sum += stats->measurements[i].value;
            if (stats->per_stats[i].n>1) {
                stats->per_stats[i].min = MBTR_STATS_SMIN( stats->per_stats[i], stats->measurements[i].value );
                stats->per_stats[i].max = MBTR_STATS_SMAX( stats->per_stats[i], stats->measurements[i].value );
                stats->per_stats[i].avg = MBTR_STATS_AVG ( stats->per_stats[i] );
            }else{
                stats->per_stats[i].min = stats->measurements[i].value;
                stats->per_stats[i].max = stats->measurements[i].value;
            }
            
            // update aggregate stats
            stats->agg_stats[i].n++;
            stats->agg_stats[i].sum += stats->measurements[i].value;
            if (stats->agg_stats[i].n>1) {
                stats->agg_stats[i].min = MBTR_STATS_SMIN( stats->agg_stats[i], stats->measurements[i].value );
                stats->agg_stats[i].max = MBTR_STATS_SMAX( stats->agg_stats[i], stats->measurements[i].value );
                stats->agg_stats[i].avg = MBTR_STATS_AVG ( stats->agg_stats[i] );
            }else{
                stats->agg_stats[i].min =stats->measurements[i].value;
                stats->agg_stats[i].max =stats->measurements[i].value;
            }
        }
        
        // reset measurement values
        for (uint32_t i=0; i<channels ; i++){
            stats->measurements[i].value=0.0;
        }
    }

    return 0;
}

/// @fn iow_socket_t * mbtrn_reader_sockif(mbtrn_reader_t * self)
/// @brief get reader socket interface.
/// @param[in] self reader reference
/// @return socket reference on success, NULL otherwise.
iow_socket_t *mbtrn_reader_sockif(mbtrn_reader_t *self)
{
    return (NULL!=self ? self->sockif : NULL);
}
// End function mbtrn_reader_sockif

/// @fn iow_file_t * mbtrn_reader_fileif(mbtrn_reader_t * self)
/// @brief get reader file interface.
/// @param[in] self reader reference
/// @return socket reference on success, NULL otherwise.
iow_file_t *mbtrn_reader_fileif(mbtrn_reader_t *self)
{
    return (NULL!=self ? self->fileif : NULL);
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
        fprintf(stderr,"%*s[sockif    %10p]\n",indent,(indent>0?" ":""), self->sockif);
        fprintf(stderr,"%*s[fileif    %10p]\n",indent,(indent>0?" ":""), self->fileif);
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


/// @fn int64_t mbtrn_read_nf(mbtrn_reader_t *self, byte *dest, uint32_t len, mbtrn_flags_t flags, double newer_than, uint32_t timeout_msec, uint32_t *sync_bytes)
/// @brief read a network frame from a file or socket.
/// @param[in] self reader reference
/// @param[in] dest data buffer
/// @param[in] flags option flags
/// @param[in] newer_than reject packets older than this (epoch time, decimal seconds)
/// @param[in] timeout_msec read timeout
/// @param[out] sync_bytes number of bytes skipped
/// @return number of bytes returned on success, -1 otherwise (me_errno set)
int64_t mbtrn_read_nf(mbtrn_reader_t *self, byte *dest, uint32_t len,
                           mbtrn_flags_t flags,
                           double newer_than,
                           uint32_t timeout_msec,
                           uint32_t *sync_bytes)
{
    int64_t retval=-1;
    me_errno = ME_EINVAL;
    
    if (NULL!=self && NULL!=mbtrn_reader_sockif(self) && NULL!=dest ) {
        
        uint32_t read_len   = 0;
        int64_t read_bytes  = 0;
        int64_t frame_bytes = 0;
        int64_t lost_bytes  = 0;
        byte *pbuf          = dest;
        byte *psync         = NULL;
        r7k_nf_t *pnf     = NULL;
        r7k_nf_t *pframe   = NULL;
        bool sync_found     = false;
        
        mbtrn_parse_state_t state = MBR_STATE_START;
        mbtrn_parse_action_t action = MBR_ACTION_QUIT;
        bool header_pending=true;
        uint32_t pending_bytes = 0;
        
        // read and validate header
        while (state != MBR_STATE_NF_VALID ) {
            
            switch (state) {
                    
                case MBR_STATE_START:
//                    MMDEBUG(MBTRN,"MBR_STATE_START\n");
                    read_len       = R7K_NF_BYTES;
                    pbuf           = dest;
                    header_pending = true;
                    frame_bytes    = 0;
                    action         = MBR_ACTION_READ;
                    memset(dest,0,len);
                    break;
                    
                case MBR_STATE_READING:
//                    MMDEBUG(MBTRN,"MBR_STATE_READING\n");
                    // don't touch inputs
                    action   = MBR_ACTION_READ;
                    break;
                    
                case MBR_STATE_READ_OK:
                    pnf   = (r7k_nf_t *)dest;
                    if (header_pending) {
//                        MMDEBUG(MBTRN,"MBR_STATE_READ_OK header\n");
                        header_pending=false;
                        action = MBR_ACTION_VALIDATE_HEADER;
                    }
                    break;
                    
                case MBR_STATE_HEADER_VALID:
                    state = MBR_STATE_NF_VALID;
                    action = MBR_ACTION_QUIT;
//                    MMDEBUG(MBTRN,"MBR_STATE_HEADER_VALID/NF_VALID\n");
                    break;
                    
                case MBR_STATE_NF_INVALID:
                    MMDEBUG(MBTRNV,"MBR_STATE_NF_INVALID\n");
                    if ( (flags&MBR_RESYNC_NF)!=0 ) {
                        // pbuf points to end of input...
                        // psync points 1 byte into invalid frame
                        MMDEBUG(MBTRN,">>>>> RESYNC: NRF buffer:\n");
                        r7k_hex_show(dest,R7K_NF_BYTES,16,true,5);
                        MMDEBUG(MBTRN,"dest[%p] pbuf[%p] ofs[%"PRId32"]\n",dest,pbuf,(int32_t)(pbuf-dest));
                        MMDEBUG(MBTRN,"read_len[%"PRIu32"]\n",read_len);
                        MMDEBUG(MBTRN,"frame_bytes[%"PRId64"]\n",frame_bytes);
                        MMDEBUG(MBTRN,"lost_bytes[%"PRId64"]\n",lost_bytes);

                        psync = dest+1;
                        lost_bytes+=1;
                        sync_found = false;
                        pframe = NULL;
                        action = MBR_ACTION_RESYNC;
                    }else{
                        // don't resync
                        action = MBR_ACTION_QUIT;
                    }
                    break;
                    
                case MBR_STATE_READ_ERR:
                    MMDEBUG(MBTRNV,"MBR_STATE_READ_ERR\n");
                    if (me_errno==ME_ESOCK) {
                        MMDEBUG(MBTRN,"socket disconnected - quitting\n");
                    }else if (me_errno==ME_EOF) {
                        MMERROR(MBTRN,"end of file\n");
                    }else if (me_errno==ME_ENOSPACE) {
                        MMERROR(MBTRN,"buffer full [%"PRIu32"/%"PRIu32"]\n",(uint32_t)(pbuf+read_len-dest),len);
                    }else{
                        MMDEBUG(MBTRN,"read error [%d/%s]\n",me_errno,me_strerror(me_errno));
                    }
                    action   = MBR_ACTION_QUIT;
                    break;
                    
                default:
                    MMERROR(MBTRN,"ERR - unknown state[%d]\n",state);
                    break;
            }
            
//            MMDEBUG(MBTRN,">>> ACTION=[%d]\n",action);
            
            if (action==MBR_ACTION_READ) {
                // inputs: pbuf, read_len
//                MMDEBUG(MBTRN,"MBR_ACTION_READ rlen[%"PRIu32"]\n",read_len);
                read_bytes=0;
                while (read_bytes<read_len) {
                    if ( (read_bytes=iow_read_tmout(mbtrn_reader_sockif(self), pbuf, read_len, timeout_msec)) == read_len) {
//                        MMDEBUG(MBTRN,"read OK [%"PRId64"]\n",read_bytes);
                        
                        pnf = (r7k_nf_t *)dest;
                        pbuf += read_bytes;
                        frame_bytes += read_bytes;
                        state=MBR_STATE_READ_OK;

                    }else{
                        MBTR_COUNTER_INC(self->stats->events[MBTR_EV_NF_SHORT_READ]);
                        if (read_bytes>=0) {
                            // short read, keep going
                            read_len -= read_bytes;
                            pbuf += read_bytes;
                            frame_bytes += read_bytes;
                            
                            if (me_errno==ME_ESOCK || me_errno==ME_EOF) {
                                state=MBR_STATE_READ_ERR;
                                MBTR_COUNTER_INC(self->stats->events[MBTR_EV_ESOCK]);
                                break;
                            }
                        }else{
                            // error
                            state=MBR_STATE_READ_ERR;
                            // let errorno bubble up
                            MBTR_COUNTER_INC(self->stats->events[MBTR_EV_ENFREAD]);
                            break;
                        }
                    }// else incomplete read
                    
                    if ( state!=MBR_STATE_READ_OK && ((pbuf+read_len-dest) > (int32_t)len) ) {
                        state=MBR_STATE_READ_ERR;
                        me_errno=ME_ENOSPACE;
                        break;
                    }
                }
            }
            
            if (action==MBR_ACTION_VALIDATE_HEADER) {
//                MMDEBUG(MBTRN,"MBR_ACTION_VALIDATE_HEADER\n");
                // inputs: pnf
                state=MBR_STATE_NF_INVALID;
                if (pnf->protocol_version == R7K_NF_PROTO_VER) {
//                    MMDEBUG(MBTRN,"nf version valid\n");
                    if (pnf->offset >= (R7K_NF_BYTES)) {
//                        MMDEBUG(MBTRN,"nf offset valid\n");
                        if (pnf->packet_size == (pnf->total_size+R7K_NF_BYTES)) {
//                            MMDEBUG(MBTRN,"nf packet_size valid [%"PRIu32"]\n",pnf->packet_size);
                            if (pnf->total_records == 1) {
//                                MMDEBUG(MBTRN,"nf total_records valid\n");
                                state = MBR_STATE_HEADER_VALID;
                            }else{
                                MMDEBUG(MBTRN,"INFO - nf total_records invalid[%"PRIu16"/%"PRIu16"]\n",pnf->total_records,(uint16_t)1);
                                MBTR_COUNTER_INC(self->stats->events[MBTR_EV_ENFTOTALREC]);
                            }
                        }else{
                            MMDEBUG(MBTRN,"INFO - nf packet_size invalid[%"PRIu32"/%"PRIu32"]\n",pnf->packet_size,(pnf->total_size+R7K_NF_BYTES));
                            MBTR_COUNTER_INC(self->stats->events[MBTR_EV_ENFPACKETSZ]);
                        }
                    }else{
                        MMDEBUG(MBTRN,"INFO - nf offset invalid [%"PRIu16"/%"PRIu16"]\n",pnf->offset,(uint16_t)R7K_NF_BYTES);
                        MBTR_COUNTER_INC(self->stats->events[MBTR_EV_ENFOFFSET]);
                    }
                }else{
                    MMDEBUG(MBTRN,"INFO - nf proto_version invalid [%"PRIu16"/%"PRIu16"]\n",pnf->protocol_version,(uint16_t)R7K_NF_PROTO_VER);
                    MBTR_COUNTER_INC(self->stats->events[MBTR_EV_ENFVER]);
                }
            }
            
            if (action==MBR_ACTION_RESYNC) {
                // input : psync points to start of search
                // input : pbuf points to end of input
                // input : sync_found false
                // input : pframe NULL
                
                MBTR_COUNTER_INC(self->stats->events[MBTR_EV_NF_RESYNC]);
                
                if( psync >= (pbuf-R7K_NF_PROTO_BYTES)){
                    MMDEBUG(MBTRN,"WARN - pending bytes > found frame\n");
                }
                
                // look until end of buffer (or sync found)
                while (  (psync < (pbuf-R7K_NF_PROTO_BYTES)) && (sync_found==false) ) {
                    
                    MMDEBUG(MBTRNV,"psync[%p] ofs[%"PRId32"]\n",psync,(int32_t)(psync-dest));
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
                        state          = MBR_STATE_READING;
                        
                        // return to the state machine
                        sync_found     = true;
                        MMDEBUG(MBTRNV,"sync found\n");
                        MMDEBUG(MBTRNV,"dest[%p] pbuf[%p] ofs[%"PRId32"]\n",dest,pbuf,(int32_t)(pbuf-dest));
                        MMDEBUG(MBTRNV,"psync[%p] ofs[%"PRId32"]\n",psync,(int32_t)(psync-dest));
                        MMDEBUG(MBTRNV,"pending_bytes[%"PRIu32"]\n",pending_bytes);
                        MMDEBUG(MBTRNV,"read_len[%"PRIu32"]\n",read_len);
                        MMDEBUG(MBTRNV,"frame_bytes[%"PRId64"]\n",frame_bytes);
                        MMDEBUG(MBTRNV,"frame_bytes+read_len[%"PRId64"]\n",(int64_t)(frame_bytes+read_len));
                        MMDEBUG(MBTRNV,"lost_bytes[%"PRId64"]\n",lost_bytes);
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
                    MMDEBUG(MBTRNV,"nf proto_ver not found - restart lost_bytes[%"PRId64"]\n",lost_bytes);
                    state=MBR_STATE_START;
                    MMDEBUG(MBTRNV,"dest[%p] pbuf[%p] ofs[%"PRId32"]\n",dest,pbuf,(int32_t)(pbuf-dest));
                    MMDEBUG(MBTRNV,"psync[%p] ofs[%"PRId32"]\n",psync,(int32_t)(psync-dest));
                    MMDEBUG(MBTRNV,"pending_bytes[%"PRIu32"]\n",pending_bytes);
                    MMDEBUG(MBTRNV,"read_len[%"PRIu32"]\n",read_len);
                    MMDEBUG(MBTRNV,"frame_bytes[%"PRId64"]\n",frame_bytes);
                    MMDEBUG(MBTRNV,"frame_bytes+read_len[%"PRId64"]\n",(frame_bytes+read_len));
                    MMDEBUG(MBTRNV,"lost_bytes[%"PRId64"]\n",lost_bytes);
                }
            }
            
            if (action==MBR_ACTION_QUIT) {
                
//                MMDEBUG(MBTRN,"ACTION_QUIT\n");
                if (state==MBR_STATE_NF_VALID) {
                    retval = frame_bytes;
                    MMDEBUG(MBTRNV,"NF valid - returning[%"PRId64"] lost[%"PRId64"]\n",retval,lost_bytes);
                    MBTR_COUNTER_INC(self->stats->events[MBTR_EV_NF_VALID]);
                    MBTR_COUNTER_ADD(self->stats->status[MBTR_STA_NF_VAL_BYTES],frame_bytes);
                }else{
                    MMDEBUG(MBTRNV,"NF invalid - returning[%"PRId64"] lost[%"PRId64"]\n",retval,lost_bytes);
                    MBTR_COUNTER_INC(self->stats->events[MBTR_EV_NF_INVALID]);
                    MBTR_COUNTER_ADD(self->stats->status[MBTR_STA_NF_INVAL_BYTES],lost_bytes);
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
}// End function mbtrn_read_nf


/// @fn int64_t mbtrn_read_drf(mbtrn_reader_t *self, byte *dest, uint32_t len, mbtrn_flags_t flags, double newer_than, uint32_t timeout_msec, uint32_t *sync_bytes)
/// @brief read a data record frame from a file or socket.
/// @param[in] self reader reference
/// @param[in] dest data buffer
/// @param[in] flags option flags
/// @param[in] newer_than reject packets older than this (epoch time, decimal seconds)
/// @param[in] timeout_msec read timeout
/// @param[out] sync_bytes number of bytes skipped
/// @return number of bytes returned on success, -1 otherwise (me_errno set)
int64_t mbtrn_read_drf(mbtrn_reader_t *self, byte *dest, uint32_t len,
                           mbtrn_flags_t flags,
                           double newer_than,
                           uint32_t timeout_msec,
                           uint32_t *sync_bytes)
{
    int64_t retval=-1;
    me_errno = ME_EINVAL;
    
    if (NULL!=self && NULL!=mbtrn_reader_sockif(self) && NULL!=dest ) {
        
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
        mbtrn_parse_state_t state = MBR_STATE_START;
        mbtrn_parse_action_t action = MBR_ACTION_QUIT;
        bool header_pending=true;
        bool data_pending=true;
        
        while (state != MBR_STATE_DRF_VALID ) {
            
            switch (state) {
                    
                case MBR_STATE_START:
//                    MMDEBUG(MBTRN,"MBR_STATE_START\n");
                   read_len = R7K_DRF_BYTES;
                    pbuf     = dest;
                    memset(dest,0,len);
                    header_pending=true;
                    data_pending=true;
                    frame_bytes=0;
                    action   = MBR_ACTION_READ;
                    break;
                    
                case MBR_STATE_READING:
//                    MMDEBUG(MBTRN,"MBR_STATE_READING\n");
                    // don't touch inputs
                    action   = MBR_ACTION_READ;
                    break;
                   
                case MBR_STATE_READ_OK:
                    pdrf   = (r7k_drf_t *)dest;
                    if (header_pending) {
//                        MMDEBUG(MBTRN,"MBR_STATE_READ_OK header\n");
                        header_pending=false;
                        action = MBR_ACTION_VALIDATE_HEADER;
                    }else if (data_pending) {
//                        MMDEBUG(MBTRN,"MBR_STATE_READ_OK data\n");
                        data_pending=false;
                        action = MBR_ACTION_VALIDATE_CHECKSUM;
                    }
                    break;
                    
                case MBR_STATE_HEADER_VALID:
                    data_pending=true;
                    read_len = pdrf->size-R7K_DRF_BYTES;
                    action = MBR_ACTION_READ;
//                    MMDEBUG(MBTRN,"MBR_STATE_HEADER_VALID data read_len[%"PRIu32"]\n",read_len);
                    break;
                    
                case MBR_STATE_CHECKSUM_VALID:
//                    MMDEBUG(MBTRN,"MBR_STATE_CHECKSUM_VALID\n");
                    action = MBR_ACTION_VALIDATE_TIMESTAMP;
                    break;
                    
                case MBR_STATE_TIMESTAMP_VALID:
//                    MMDEBUG(MBTRN,"MBR_STATE_TIMESTAMP_VALID\n");
                    state = MBR_STATE_DRF_VALID;
                    action = MBR_ACTION_QUIT;
                    break;
                    
                case MBR_STATE_DRF_REJECTED:
                    MMDEBUG(MBTRN,"MBR_STATE_DRF_REJECTED\n");
                    // start over with new frame
                    state = MBR_STATE_START;
                    break;
                    
                case MBR_STATE_DRF_INVALID:
                    MMDEBUG(MBTRN,"MBR_STATE_DRF_INVALID\n");
                    if ( (flags&MBR_RESYNC_DRF)!=0 ) {
                        // pbuf points to end of input...
                        // psync points 1 byte into invalid frame
                        psync = dest+1;
                        lost_bytes+=1;
                        sync_found = false;
                        pframe = NULL;
                        action = MBR_ACTION_RESYNC;
                    }else{
                    	// don't resync
                        action = MBR_ACTION_QUIT;
                   }
                    break;
                    
                case MBR_STATE_READ_ERR:
                    MMDEBUG(MBTRN,"MBR_STATE_READ_ERR\n");
                    if (me_errno==ME_ESOCK) {
                        MMDEBUG(MBTRN,"socket disconnected - quitting\n");
                    }else if (me_errno==ME_EOF) {
                        MMERROR(MBTRN,"end of file\n");
                    }else if (me_errno==ME_ENOSPACE) {
                        MMERROR(MBTRN,"buffer full [%"PRIu32"/%"PRIu32"]\n",(uint32_t)(pbuf+read_len-dest),len);
                    }else{
                            MMDEBUG(MBTRN,"read error [%d/%s]\n",me_errno,me_strerror(me_errno));
                    }
                    action   = MBR_ACTION_QUIT;
                    break;
                    
                default:
                    MMERROR(MBTRN,"ERR - unknown state[%d]\n",state);
                    break;
            }

//            MMDEBUG(MBTRN,">>> ACTION=[%d]\n",action);
          
            if (action==MBR_ACTION_READ) {
                // inputs: pbuf, read_len
//                MMDEBUG(MBTRN,"MBR_ACTION_READ rlen[%"PRIu32"]\n",read_len);
                read_bytes=0;
                while (read_bytes<read_len) {
                    if ( (read_bytes=iow_read_tmout(mbtrn_reader_sockif(self), pbuf, read_len, timeout_msec)) == read_len) {
//                        MMDEBUG(MBTRN,"read OK [%"PRId64"]\n",read_bytes);
                        
                        pdrf = (r7k_drf_t *)dest;
                        pbuf += read_bytes;
                        frame_bytes += read_bytes;
                        state=MBR_STATE_READ_OK;
                        
                    }else{
                        MBTR_COUNTER_INC(self->stats->events[MBTR_EV_DRF_SHORT_READ]);
                        if (read_bytes>=0) {
                            // short read, keep going
                            read_len -= read_bytes;
                            pbuf += read_bytes;
                            frame_bytes += read_bytes;
                            
                            if (me_errno==ME_ESOCK || me_errno==ME_EOF) {
                                state=MBR_STATE_READ_ERR;
                                MBTR_COUNTER_INC(self->stats->events[MBTR_EV_ESOCK]);
                                break;
                            }
                        }else{
                            // error
                            state=MBR_STATE_READ_ERR;
                            // let errorno bubble up
                            MBTR_COUNTER_INC(self->stats->events[MBTR_EV_EDRFREAD]);
                            break;
                        }
                    }// else incomplete read
                    
                    if ( state!=MBR_STATE_READ_OK && (pbuf+read_len-dest) > (int32_t)len ) {
                        state=MBR_STATE_READ_ERR;
                        me_errno=ME_ENOSPACE;
                        break;
                    }
                }
            }
            
            if (action==MBR_ACTION_VALIDATE_HEADER) {
//                MMDEBUG(MBTRN,"MBR_ACTION_VALIDATE_HEADER\n");
                // inputs: pdrf
                state=MBR_STATE_DRF_INVALID;
                if (pdrf->protocol_version == R7K_DRF_PROTO_VER) {
//                    MMDEBUG(MBTRN,"drf protocol version valid [0x%0X]\n",pdrf->protocol_version);
                    if (pdrf->sync_pattern == R7K_DRF_SYNC_PATTERN) {
//                        MMDEBUG(MBTRN,"drf sync_pattern valid [0x%0X]\n",pdrf->sync_pattern);
                        if (pdrf->size <= R7K_MAX_FRAME_BYTES) {
//                            MMDEBUG(MBTRN,"drf size < max frame valid [0x%"PRIu32"]\n",pdrf->size);
                            
                            // conditionally validate
                            // (pending optional nf size)
                            state=MBR_STATE_HEADER_VALID;
//                            
                        }else{
                            MMDEBUG(MBTRN,"INFO - drf size<max frame invalid [%"PRIu32"]\n",pdrf->size);
                            MBTR_COUNTER_INC(self->stats->events[MBTR_EV_EDRFSIZE]);
                        }
                    }else{
                        MMDEBUG(MBTRN,"INFO - drf sync pattern invalid [0x%0X/0x%0X]\n",pdrf->sync_pattern, R7K_DRF_SYNC_PATTERN);
                        MBTR_COUNTER_INC(self->stats->events[MBTR_EV_EDRFSYNC]);
                    }
                }else{
                    MMDEBUG(MBTRN,"INFO - drf protocol version invalid [0x%0X/0x%0X]\n",pdrf->protocol_version, R7K_DRF_PROTO_VER);
                    MBTR_COUNTER_INC(self->stats->events[MBTR_EV_EDRFPROTO]);
                }
            }
            
            if (action==MBR_ACTION_VALIDATE_CHECKSUM) {
                // inputs pdrf
                state=MBR_STATE_DRF_INVALID;
                
                // validate checksum
                // only if flag set
                if ( (pdrf->flags&0x1)==0 ){
                    // skip checksum constraint
//                    MMDEBUG(MBTRN,"drf chksum valid (unchecked)\n");
                    state=MBR_STATE_CHECKSUM_VALID;
                }else{
                    // buffer/checksum boundary checked on entry
                    byte *pd=(byte *)pdrf;
                    uint32_t vchk = r7k_checksum( pd, (uint32_t)(pdrf->size-R7K_CHECKSUM_BYTES));
                    pd = (byte *)pdrf;
                    pd += ((size_t)pdrf->size-R7K_CHECKSUM_BYTES);
                    uint32_t *pchk = (uint32_t *)pd;
                    
                    if (vchk == (uint32_t)(*pchk) ) {
//                        MMDEBUG(MBTRN,"drf chksum valid [0x%08X]\n",vchk);
                        state=MBR_STATE_CHECKSUM_VALID;
                    }else{
                        MMDEBUG(MBTRN,"INFO - drf chksum invalid [0x%08X/0x%08X]\n",vchk, (uint32_t)(*pchk));
                        MBTR_COUNTER_INC(self->stats->events[MBTR_EV_EDRFCHK]);
                    }
                }
            }
            
            if (action==MBR_ACTION_VALIDATE_TIMESTAMP) {
                // inputs pdrf
                state=MBR_STATE_DRF_REJECTED;
                // validate timestamp
                // optional, per newer_than value
                if (newer_than<=0.0){
                    // skip timestamp constraint
//                    MMDEBUG(MBTRN,"drf time valid (unchecked)\n");
                    state=MBR_STATE_TIMESTAMP_VALID;
                }else{
                    double dtime = 0.0;
                    dtime += pdrf->_7ktime.day*SEC_PER_DAY;
                    dtime += pdrf->_7ktime.hours*SEC_PER_HOUR;
                    dtime += pdrf->_7ktime.minutes*SEC_PER_MIN;
                    dtime += pdrf->_7ktime.seconds;
                    if (dtime>newer_than) {
//                        MMDEBUG(MBTRN,"drf time valid\n");
                        state=MBR_STATE_TIMESTAMP_VALID;
                    }else{
                        MMDEBUG(MBTRN,"INFO - drf time invalid [%.4lf/%.4lf]\n",dtime,newer_than);
                        MBTR_COUNTER_INC(self->stats->events[MBTR_EV_EDRFTIME]);
                    }
                }
            }
            
            if (action==MBR_ACTION_RESYNC) {
                // input : psync points to start of search
                // input : pbuf points to end of input
                // input : sync_found false
                // input : pframe NULL
                
                MBTR_COUNTER_INC(self->stats->events[MBTR_EV_DRF_RESYNC]);
	
                if( psync >= (pbuf-R7K_DRF_PROTO_BYTES)){
                    MMDEBUG(MBTRN,"WARN - pending bytes > found frame\n");
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
                        uint32_t completion_bytes = 0;
                        
                        // have read more bytes than a DRF header...
                        if (pending_bytes>R7K_DRF_BYTES) {
                            
                            // the header should be valid...
                            if (pframe->size<=R7K_MAX_FRAME_BYTES &&
                                pframe->sync_pattern == R7K_DRF_SYNC_PATTERN) {
                                
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
                                    state          = MBR_STATE_READING;

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
                                    state          = MBR_STATE_READ_OK;
                                    
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
                            state          = MBR_STATE_READING;
                            
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
                    MMDEBUG(MBTRN,"INFO - drf proto_ver not found - restart\n");
                    state=MBR_STATE_START;
                    lost_bytes+=(pbuf-psync);
                }
            }
            
            if (action==MBR_ACTION_QUIT) {
                
//                MMDEBUG(MBTRN,"ACTION_QUIT\n");
                if (state==MBR_STATE_DRF_VALID) {
                    retval = frame_bytes;
                    
                    MMDEBUG(MBTRNV,"DRF valid - returning[%"PRId64"] lost[%"PRId64"] type[%"PRIu32"]\n",retval,lost_bytes,pdrf->record_type_id);
                    MBTR_COUNTER_INC(self->stats->events[MBTR_EV_DRF_VALID]);
                    MBTR_COUNTER_ADD(self->stats->status[MBTR_STA_DRF_VAL_BYTES],frame_bytes);
#ifdef MBTRN_TIMING
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
                    
                    struct timespec res;
                    clock_getres(CLOCK_MONOTONIC, &res);
                    fprintf(stderr,"%11.5lf cskew %+0.4e\n",iow_dtime(),(double)(stime-ptime));
                    
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
                    struct timespec res;
                    clock_getres(CLOCK_MONOTONIC, &res);
                    fprintf(stderr,"%11.5lf lskew %+0.4e\n",iow_dtime(), (double)(stime-ptime));
#endif
#endif
                    

                }else{
                    MMDEBUG(MBTRNV,"DRF invalid - returning[%"PRId64"] lost[%"PRId64"]\n",retval,lost_bytes);
                    MBTR_COUNTER_INC(self->stats->events[MBTR_EV_DRF_INVALID]);
                    MBTR_COUNTER_ADD(self->stats->status[MBTR_STA_DRF_INVAL_BYTES],lost_bytes);

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
}// End function mbtrn_read_drf


/// @fn int64_t mbtrn_read_stripped_frame(mbtrn_reader_t *self, uint32_t nframes, byte *dest, uint32_t len, uint32_t flags, double newer_than, uint32_t timeout_msec )
/// @brief read frame from file or socket, strip network frame headers (return data frame only).
/// @param[in] self reader reference
/// @param[in] dest data buffer
/// @param[in] len buffer size (bytes)
/// @param[in] flags option flags
/// @param[in] newer_than reject packets older than this (epoch time, decimal seconds)
/// @param[in] timeout_msec read timeout
/// @param[out] sync_bytes number of bytes skipped
/// @return number of bytes returned on success, -1 otherwise (me_errno set)
int64_t mbtrn_read_stripped_frame(mbtrn_reader_t *self, byte *dest,
                             uint32_t len, mbtrn_flags_t flags,
                             double newer_than, uint32_t timeout_msec,
                             uint32_t *sync_bytes )
{
    int64_t retval = -1;
    
    if (flags&MBR_NET_STREAM) {
        retval = mbtrn_read_frame( self, dest,len,  flags,
                             newer_than,  timeout_msec,
                             sync_bytes );
        
        if (retval>0) {
            r7k_drf_t *pdrf = (r7k_drf_t *)(dest+R7K_NF_BYTES);
            memmove(dest,(dest+R7K_NF_BYTES),pdrf->size);
            retval-=R7K_NF_BYTES;
        }
    }else{
        retval = mbtrn_read_frame( self, dest,len,  flags,
                                      newer_than,  timeout_msec,
                                      sync_bytes );
    }

//    fprintf(stderr,"mbtrn_read_stripped_frame [%"PRId64"]\n",retval);
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
}// End function mbtrn_read_stripped_frame

/// @fn int64_t mbtrn_read_frame(mbtrn_reader_t *self, uint32_t nframes, byte *dest, uint32_t len, uint32_t flags, double newer_than, uint32_t timeout_msec )
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
int64_t mbtrn_read_frame(mbtrn_reader_t *self, byte *dest,
                         uint32_t len, mbtrn_flags_t flags,
                         double newer_than, uint32_t timeout_msec,
                             uint32_t *sync_bytes )
{
    
    int64_t retval=-1;
    me_errno = ME_OK;
    uint32_t frame_bytes=0;
    uint32_t nf_bytes=0;
    uint32_t drf_bytes=0;
    mbtrn_flags_t rflags=0;
    
    byte *pbuf=dest;
    int64_t read_bytes=0;
    uint32_t read_len=0;

    mbtrn_parse_state_t state = MBR_STATE_START;
    mbtrn_parse_action_t action = MBR_ACTION_QUIT;
    iow_socket_t *sockif =mbtrn_reader_sockif(self);
    if (NULL!=self && NULL!=dest &&
        ( (flags&MBR_NF_STREAM) || (flags&MBR_DRF_STREAM) || (flags&MBR_NET_STREAM) ) &&
        NULL!=sockif && sockif->fd>0
        ) {

        while (state != MBR_STATE_FRAME_VALID ) {
            
            switch (state) {
                    
                case MBR_STATE_START:
//                    MMDEBUG(MBTRN,"MBR_STATE_START\n");
                    if (flags&MBR_NET_STREAM) {
                        read_len = R7K_NF_BYTES;
                        rflags   = MBR_RESYNC_NF;
                        action   = MBR_ACTION_READ_NF;
                    }else if (flags&MBR_DRF_STREAM) {
                        // drf only (e.g. s7k)
                        rflags   = MBR_RESYNC_DRF;
                        read_len = len;
                        action   = MBR_ACTION_READ_DRF;
                    }else {
                        // net frames only
                        rflags   = MBR_RESYNC_NF;
                        read_len = R7K_NF_BYTES;
                        action   = MBR_ACTION_READ_NF;
                    }
                    
                    pbuf     = dest;
                    memset(dest,0,len);
                    frame_bytes=0;
                    break;
                    
                case MBR_STATE_NF_VALID:
//                    MMDEBUG(MBTRN,"MBR_STATE_NF_VALID\n");
                    if (flags&MBR_NET_STREAM) {
                        // next, read the DRF
                        read_len = len-R7K_NF_BYTES;
                        // don't resync on failed DRF read
                        rflags   = 0;
                        action   = MBR_ACTION_READ_DRF;
                    }else if (flags&MBR_DRF_STREAM) {
                        // drf only (e.g. s7k)
                        // shouldn't be here
                        MMDEBUG(MBTRN,"ERR - invalid condition: NF_VALID for DRF stream\n");
                    }else {
                        // net frames only
                        // done
                        state  = MBR_STATE_FRAME_VALID;
                        action = MBR_ACTION_QUIT;
                    }
                    break;
                    
                case MBR_STATE_DRF_VALID:
//                    MMDEBUG(MBTRN,"MBR_STATE_DRF_VALID\n");
                    if (flags&MBR_NET_STREAM || flags&MBR_DRF_STREAM) {
                        // done
                        state  = MBR_STATE_FRAME_VALID;
                        action = MBR_ACTION_QUIT;
                    }else {
                        // net frames only (unlikely)
                        MMDEBUG(MBTRN,"ERR - invalid condition: DRF_VALID for NF stream\n");
                    }
                    break;
                  
                case MBR_STATE_NF_INVALID:
//                    MMDEBUG(MBTRN,"MBR_STATE_NF_INVALID (retrying)\n");
                    state  = MBR_STATE_START;
                    action = MBR_ACTION_NOOP;
                    break;

                case MBR_STATE_DRF_INVALID:
                    MMDEBUG(MBTRN,"MBR_STATE_DRF_INVALID (retrying)\n");
                    state  = MBR_STATE_START;
                    action = MBR_ACTION_NOOP;
                    break;

                case MBR_STATE_READ_ERR:
                    MMDEBUG(MBTRN,"MBR_STATE_READ_ERR\n");
                    if (me_errno==ME_ESOCK) {
                        MMDEBUG(MBTRN,"socket disconnected - quitting\n");
                    }else if (me_errno==ME_EOF) {
                        MMERROR(MBTRN,"end of file\n");
                    }else if (me_errno==ME_ENOSPACE) {
                            MMERROR(MBTRN,"buffer full [%"PRIu32"/%"PRIu32"]\n",(uint32_t)(pbuf+read_len-dest),len);
                    }else{
                        MMDEBUG(MBTRN,"read error [%d/%s]\n",me_errno,me_strerror(me_errno));
                    }
                    action   = MBR_ACTION_QUIT;
                    break;

               default:
                    MMERROR(MBTRN,"ERR - unknown state[%d]\n",state);
                    break;
            }

            if (action==MBR_ACTION_READ_NF) {
                if ( (read_bytes=mbtrn_read_nf(self, pbuf, read_len, rflags, newer_than, timeout_msec, sync_bytes)) == R7K_NF_BYTES) {
                    
//                    MMDEBUG(MBTRN,"nf read OK\n");
                    // update pointers
                    pbuf        += read_bytes;
                    nf_bytes    += read_bytes;
                    frame_bytes += read_bytes;
                    
                    state = MBR_STATE_NF_VALID;
                }else{
                    MMERROR(MBTRN,"ERR - mbtrn_read_nf read_bytes[%"PRId64"] [%d/%s]\n",read_bytes,me_errno,me_strerror(me_errno));
                    state = MBR_STATE_READ_ERR;
                }
            }

            if (action==MBR_ACTION_READ_DRF) {
                if ( (read_bytes=mbtrn_read_drf(self, pbuf, read_len, rflags, newer_than, timeout_msec, sync_bytes)) > R7K_DRF_BYTES) {
                    
//                    MMDEBUG(MBTRN,"drf read OK\n");
                    // update pointers
                    pbuf        += read_bytes;
                    drf_bytes   += read_bytes;
                    frame_bytes += read_bytes;
                    
                    state = MBR_STATE_DRF_VALID;
                }else{
                    MMERROR(MBTRN,"ERR - mbtrn_read_nf read_bytes[%"PRId64"] [%d/%s]\n",read_bytes,me_errno,me_strerror(me_errno));
                    state = MBR_STATE_READ_ERR;
                }
            }
            if (action==MBR_ACTION_QUIT) {
                
//                MMDEBUG(MBTRN,"ACTION_QUIT\n");
                if (state==MBR_STATE_FRAME_VALID) {
                    retval = frame_bytes;
                    MMDEBUG(MBTRNV,"Frame valid - returning[%"PRId64"]\n",retval);
                    MBTR_COUNTER_ADD(self->stats->status[MBTR_STA_FRAME_VAL_BYTES],frame_bytes);
                    

//                    MBTR_COUNTER_INC(self->stats->events[MBTR_EV_FRAME_VALID]);
//                    r7k_nf_show((r7k_nf_t *)dest,false,5);
//                    r7k_drf_show((r7k_drf_t *)(dest+R7K_NF_BYTES),false,5);
//                    r7k_hex_show(dest,frame_bytes,16,true,5);
//                    if (NULL!=self->logstream) {
//                       // TODO : replace with plain file
//                        fwrite(dest,frame_bytes,1,self->logstream);
//                    }
                    
                    if (NULL!=self->log) {
                        mlog_write(self->log_id,dest,frame_bytes);
                    }

                }else{
                    MMDEBUG(MBTRN,"Frame invalid [%d/%s] retval[%"PRId64"]\n",me_errno,me_strerror(me_errno),retval);
                    MBTR_COUNTER_INC(self->stats->events[MBTR_EV_FRAME_INVALID]);
                }
                // quit and return
                break;
            }
        }// while frame invalid
        
    }else{
        MERROR("invalid argument\n");
    }
    MMDEBUG(MBTRNV,"returning [%lld]\n",retval);
    return retval;
 }// End function mbtrn_read_frame

/// @fn iint64_t mbtrn_read_nf_dep (mbtrn_reader_t *self, byte *dest, uint32_t *sync_bytes, uint32_t timeout_msec)
/// @brief read network frame (deprecated).
/// @param[in] self reader reference
/// @param[in] dest destination buffer
/// @param[out] sync_bytes number of bytes skipped
/// @param[in] timeout_msec timeout to use for reads
/// @return number of bytes returned in dest on success, -1 otherwise
int mbtrn_read_nf_dep(mbtrn_reader_t *self, byte *dest, uint32_t *sync_bytes, uint32_t timeout_msec)
{
    int retval=-1;
    me_errno = ME_OK;
    byte wbuf[R7K_MAX_FRAME_BYTES]={0};
    bool nf_valid=false;
    
    if (NULL!=self && NULL!=mbtrn_reader_sockif(self) && NULL!=dest) {
        byte *pbuf=NULL;
        r7k_nf_t *pnf=NULL;
        int64_t read_bytes=0;
        uint32_t read_len=0;
        
        bool do_resync=true;
        
        // initialize state
        pbuf=wbuf;
        pnf=(r7k_nf_t *)pbuf;
        read_len=R7K_NF_BYTES;
        
        // read until valid net frame header
        while (nf_valid==false) {
            
            do_resync=true;
            //            MMDEBUG(MBTRN,"reading read_len[%"PRIu32"] to pbuf[%p]\n",read_len,pbuf);
            if ( (read_bytes=iow_read_tmout(mbtrn_reader_sockif(self), pbuf, read_len, timeout_msec)) == read_len) {
                
                pbuf=wbuf;
                
                pnf=(r7k_nf_t *)wbuf;
                
                if (pnf->protocol_version == R7K_NF_PROTO_VER) {
                    MMDEBUG(MBTRN,"nf version valid\n");
                    if (pnf->offset >= (R7K_NF_BYTES)) {
                        MMDEBUG(MBTRN,"nf offset valid\n");
                        if (pnf->packet_size == (pnf->total_size+R7K_NF_BYTES)) {
                            MMDEBUG(MBTRN,"nf packet_size valid [%"PRIu32"]\n",pnf->packet_size);
                            if (pnf->total_records == 1) {
                                MMDEBUG(MBTRN,"nf total_records valid\n");
                                MMDEBUG(MBTRN,">>>>> NF VALID >>>>>\n");
                                nf_valid=true;
                                do_resync=false;
                                pbuf = wbuf+R7K_NF_BYTES;
                                memcpy(dest,wbuf,R7K_NF_BYTES);
                                retval = 0;
                                break;
                            }else{
                                MMDEBUG(MBTRN,"err - nf total_records invalid[%"PRIu16"/%"PRIu16"]\n",pnf->total_records,(uint16_t)1);
                                MBTR_COUNTER_INC(self->stats->events[MBTR_EV_ENFTOTALREC]);
                            }
                        }else{
                            MMDEBUG(MBTRN,"err - nf packet_size invalid[%"PRIu32"/%"PRIu32"]\n",pnf->packet_size,(pnf->total_size+R7K_NF_BYTES));
                            MBTR_COUNTER_INC(self->stats->events[MBTR_EV_ENFPACKETSZ]);
                        }
                    }else{
                        MMDEBUG(MBTRN,"err - nf offset invalid [%"PRIu16"/%"PRIu16"]\n",pnf->offset,(uint16_t)R7K_NF_BYTES);
                        MBTR_COUNTER_INC(self->stats->events[MBTR_EV_ENFOFFSET]);
                    }
                }else{
                    MMDEBUG(MBTRN,"err - nf proto_version invalid [%"PRIu16"/%"PRIu16"]\n",pnf->protocol_version,(uint16_t)R7K_NF_PROTO_VER);
                    MBTR_COUNTER_INC(self->stats->events[MBTR_EV_ENFVER]);
                }
            }else{
                
                //                MMDEBUG(MBTRN,"err - nf read read_bytes[%"PRId64"] [%d/%s]\n",read_bytes,me_errno,me_strerror(me_errno));
                
                MBTR_COUNTER_INC(self->stats->events[MBTR_EV_NF_SHORT_READ]);
                if (read_bytes>=0) {
                    // short read -
                    // try again, don't resync
                    read_len -= read_bytes;
                    pbuf += read_bytes;
                    do_resync=false;
                    //                    MMDEBUG(MBTRN,"retrying read [%"PRIu32"]\n",read_len);
                    
                    if (me_errno==ME_ESOCK) {
                        MMDEBUG(MBTRN,"socket closed: setting ESOCK and returning\n");
                        MBTR_COUNTER_INC(self->stats->events[MBTR_EV_ESOCK]);
                        break;
                    }
                    
                    // 13jul18T1420 : orig code did NOT have the read_bytes==0
                    // check, and so would delay 3s ea 8 reads.
                    // This block was commented out by hand for the 13jul18T1420 mission
                    // should leave out, since socket select/read timeout
                    // should cover it anyway.
                    //                    if (read_bytes==0 && read_retries-- <= 0 ) {
                    //                        // don't allow loop to spin
                    //                        // hard (though really,
                    //                        // the select timeout in read_tmout
                    //                        // should not let that happen
                    //                        sleep(MBTRN_RETRY_DELAY_SEC);
                    //                        read_retries=MBTRN_READ_RETRIES;
                    //                    }
                    
                }else{
                    // error
                    MMERROR(MBTRN,"read error [%d/%s]\n",me_errno,me_strerror(me_errno));
                    // let errorno bubble up
                    MBTR_COUNTER_INC(self->stats->events[MBTR_EV_ENFREAD]);
                    break;
                }
            }
            
            if (do_resync==true) {
                MBTR_COUNTER_INC(self->stats->events[MBTR_EV_NF_RESYNC]);
                //                MMDEBUG(MBTRN,"start nf resync\n");
                // find a possibly valid proto version
                byte *pb = wbuf+1;
                r7k_nf_t *pn = NULL;
                bool sync_found=false;
//                r7k_nf_show(pnf, true, 5);
//                r7k_hex_show(wbuf, R7K_NF_BYTES, 12, true, 5);
                
                while ( (pb<=(wbuf+R7K_NF_BYTES-R7K_NF_PROTO_BYTES)) && (sync_found==false) ) {
                    pn = (r7k_nf_t *)pb;
                    
                    if (pn->protocol_version == R7K_NF_PROTO_VER) {
                        uint32_t move_bytes = (R7K_NF_BYTES-(pb-wbuf));
                        uint32_t fill_bytes = (R7K_NF_BYTES-move_bytes);
                        // move bytes to start of buffer
                        memmove(wbuf, pb, move_bytes);
                        // updates pointers to fill in frame
                        pbuf       = wbuf+move_bytes;
                        memset(pbuf,0,fill_bytes);
                        read_len   = fill_bytes;
                        sync_found = true;
                        MMDEBUG(MBTRN,"nf proto_ver found wb[%p] pb[%p] rlen[%"PRIu32"]\n",wbuf,pb,read_len);
                        MMDEBUG(MBTRN,"mb[%"PRIu32"] fb[%"PRIu32"]\n",move_bytes,fill_bytes);
                        break;
                    }
                    pb++;
                    if (NULL != sync_bytes) {
                        (*sync_bytes)++;
                    }
                }// while
                
                if (sync_found==false) {
                    // nf proto_version found
                    // start over
                    MMDEBUG(MBTRN,"nf proto_ver not found - restart\n");
                    pbuf = wbuf;
                    read_len=R7K_NF_BYTES;
                    memset(wbuf,0,R7K_NF_BYTES);
                }
            }
        }// while !nf_valid
        
    }else{
        MERROR("invalid argument\n");
        me_errno = ME_EINVAL;
    }
    
    if(nf_valid){
        MBTR_COUNTER_INC(self->stats->events[MBTR_EV_NF_VALID]);
        MBTR_COUNTER_ADD(self->stats->status[MBTR_STA_NF_VAL_BYTES],retval);
    }else{
        MBTR_COUNTER_INC(self->stats->events[MBTR_EV_NF_INVALID]);
        MBTR_COUNTER_ADD(self->stats->status[MBTR_STA_NF_INVAL_BYTES],*(sync_bytes));
        if (NULL != sync_bytes) {
        }
    }
    return retval;
}// End function mbtrn_read_nf_dep

/// @fn iint64_t mbtrn_read_drf_dep (mbtrn_reader_t *self, byte *dest, uint32_t len, r7k_nf_t *nf, double newer_than, uint32_t *sync_bytes, uint32_t timeout_msec)
/// @brief read data record frame (deprecated).
/// @param[in] self reader reference
/// @param[in] dest destination buffer
/// @param[in] len  max destination buffer size
/// @param[in] nf   network frame reference
/// @param[in] newer_than filter packets by time (reject if older than newer_than)
/// @param[out] sync_bytes number of bytes skipped
/// @param[in] timeout_msec timeout to use for reads
/// @return number of bytes returned in dest on success, -1 otherwise
int64_t mbtrn_read_drf_dep(mbtrn_reader_t *self, byte *dest, uint32_t len, r7k_nf_t *nf, double newer_than, uint32_t *sync_bytes, uint32_t timeout_msec)
{
    int64_t retval=-1;
    me_errno = ME_EINVAL;
    
    if (NULL!=self && NULL!=mbtrn_reader_sockif(self) &&
        NULL!=dest && NULL!=nf &&
        nf->packet_size>0 && len>=(nf->packet_size-R7K_NF_BYTES)) {
        
        byte *pbuf = dest;
        uint32_t read_len = (nf->packet_size-R7K_NF_BYTES);
        int64_t read_bytes=0;
        r7k_drf_t *pdrf =NULL;
        bool drf_valid=false;
        
        while (read_bytes<read_len) {
            
            if ( (read_bytes=iow_read_tmout(mbtrn_reader_sockif(self), pbuf, read_len, timeout_msec)) == read_len) {
                MMDEBUG(MBTRN,"drf read OK [%"PRId64"]\n",read_bytes);
                pdrf = (r7k_drf_t *)pbuf;
                if (pdrf->sync_pattern == R7K_DRF_SYNC_PATTERN) {
                    MMDEBUG(MBTRN,"drf sync_pattern valid [0x%0X]\n",pdrf->sync_pattern);
                    if (pdrf->size == (nf->total_size)) {
                        MMDEBUG(MBTRN,"drf size valid [%"PRIu32"]\n",pdrf->size);
                        
                        // conditionally validate
                        // (pending optional time, checksum)
                        drf_valid=true;
                        
                        // validate checksum if flag set
                        if ( (pdrf->flags&0x1)!=0 ) {
                            // buffer/checksum boundary checked on entry
                            byte *pd=(byte *)pdrf;
                            uint32_t vchk = r7k_checksum( pd, (uint32_t)(pdrf->size-R7K_CHECKSUM_BYTES));
                            pd = (byte *)pdrf;
                            pd += ((size_t)pdrf->size-R7K_CHECKSUM_BYTES);
                            uint32_t *pchk = (uint32_t *)pd;
                            
                            if (vchk == (uint32_t)(*pchk) ) {
                                MMDEBUG(MBTRN,"drf chksum valid [0x%08X]\n",vchk);
                            }else{
                                MMDEBUG(MBTRN," err - drf chksum invalid [0x%08X/0x%08X]\n",vchk, (uint32_t)(*pchk));
                                drf_valid=false;
                                MBTR_COUNTER_INC(self->stats->events[MBTR_EV_EDRFCHK]);
                            }
                        }else{
                            // skip checksum constraint
                            MMDEBUG(MBTRN,"drf chksum valid (unchecked)\n");
                        }
                        
                        // validate timestamp
                        // optional, per newer_than value
                        if (newer_than>0) {
                            double dtime = 0.0;
                            dtime += pdrf->_7ktime.day*SEC_PER_DAY;
                            dtime += pdrf->_7ktime.hours*SEC_PER_HOUR;
                            dtime += pdrf->_7ktime.minutes*SEC_PER_MIN;
                            dtime += pdrf->_7ktime.seconds;
                            if (dtime>newer_than) {
                                MMDEBUG(MBTRN,"drf time valid\n");
                            }else{
                                MMDEBUG(MBTRN," err - drf time invalid [%.4lf/%.4lf]\n",dtime,newer_than);
                                drf_valid=false;
                                MBTR_COUNTER_INC(self->stats->events[MBTR_EV_EDRFTIME]);
                            }
                        }else{
                            // skip timestamp constraint
                            MMDEBUG(MBTRN,"drf time valid (unchecked)\n");
                        }
                        if (drf_valid) {
                            MMDEBUG(MBTRN,">>>>> DRF VALID >>>>>\n");
                        }
                        
                    }else{
                        MMDEBUG(MBTRN," err - drf size invalid [%"PRIu32"/%"PRIu32"]\n",pdrf->size , (nf->packet_size-R7K_NF_BYTES));
                        MBTR_COUNTER_INC(self->stats->events[MBTR_EV_EDRFSIZE]);
                    }
                }else{
                    MMDEBUG(MBTRN," err - drf sync pattern invalid [0x%0X/0x%0X]\n",pdrf->sync_pattern, R7K_DRF_SYNC_PATTERN);
                    MBTR_COUNTER_INC(self->stats->events[MBTR_EV_EDRFSYNC]);
                }
            }else{
                MBTR_COUNTER_INC(self->stats->events[MBTR_EV_DRF_SHORT_READ]);
                if (read_bytes>=0) {
                    // short read - try again, don't resync
                    read_len -= read_bytes;
                    pbuf += read_bytes;
                    
                    if (me_errno==ME_ESOCK) {
                        MMDEBUG(MBTRN,"socket closed: setting ESOCK and returning\n");
                        MBTR_COUNTER_INC(self->stats->events[MBTR_EV_ESOCK]);
                        break;
                    }
                }else{
                    // error
                    MMERROR(MBTRN,"read error [%d/%s]\n",me_errno,me_strerror(me_errno));
                    // let errorno bubble up
                    MBTR_COUNTER_INC(self->stats->events[MBTR_EV_EDRFREAD]);
                    break;
                }
            }// else incomplete read
        }// while
        
        if (drf_valid) {
            retval=read_bytes;
            MBTR_COUNTER_INC(self->stats->events[MBTR_EV_DRF_VALID]);
            MBTR_COUNTER_ADD(self->stats->status[MBTR_STA_DRF_VAL_BYTES],read_bytes);
        }else{
            MBTR_COUNTER_INC(self->stats->events[MBTR_EV_DRF_INVALID]);
            if (NULL!=sync_bytes) {
                (*sync_bytes)+=read_bytes;
                MBTR_COUNTER_ADD(self->stats->status[MBTR_STA_DRF_INVAL_BYTES],read_bytes);
            }
        }
    }else{
        MERROR("invalid argument\n");
    }
    
    
    return retval;
} // End function mbtrn_read_drf_dep

/// @fn int64_t mbtrn_read_frame_dep(mbtrn_reader_t *self, uint32_t nframes, byte *dest, uint32_t len, uint32_t flags, double newer_than, uint32_t timeout_msec )
/// @brief add data record frames to reader container w/ age new than specified time.
/// @param[in] self reader reference
/// @param[in] dest destination buffer
/// @param[in] len  max destination buffer size
/// @param[in] nf   network frame reference
/// @param[in] newer_than filter packets by time (reject if older than newer_than)
/// @param[in] timeout_msec timeout to use for reads
/// @return number of bytes returned in dest on success, -1 otherwise
int64_t mbtrn_read_frame_dep(mbtrn_reader_t *self, byte *dest,
                         uint32_t len, mbtrn_flags_t flags,
                         double newer_than, uint32_t timeout_msec )
{
    int64_t retval=-1;
    me_errno = ME_OK;
    uint32_t sync_bytes=0;
    uint32_t total_bytes=0;
    uint32_t nf_bytes=0;
    uint32_t drf_bytes=0;
    uint32_t min_len = 0;
    
    if ( (flags&MBR_NET_STREAM) != 0) {
        min_len=(R7K_NF_BYTES+R7K_DRF_BYTES+R7K_CHECKSUM_BYTES);
    }else{
        min_len=R7K_DRF_BYTES;
    }
    
    if (NULL!=self && NULL!=dest && len>=min_len &&
        NULL!=mbtrn_reader_sockif(self) && NULL!=self->fc) {
        
        byte buf[R7K_MAX_FRAME_BYTES]={0};
        byte *pdest=dest;
        byte *psrc=buf;
        r7k_nf_t *pnf=NULL;
        r7k_drf_t *pdrf=NULL;
        byte *pbuf=buf;
        int64_t read_bytes=0;
    
        if (flags&MBR_NET_STREAM) {
            MMDEBUG(MBTRN,"reading NF flags[%d]\n",flags);
            if (mbtrn_read_nf_dep(self, pbuf, &sync_bytes, timeout_msec) == 0) {
                
                MMDEBUG(MBTRN,"nf read OK\n");
                // update pointers
                pbuf = buf+R7K_NF_BYTES;
                pnf = (r7k_nf_t *)buf;
                nf_bytes+=R7K_NF_BYTES;
                total_bytes+=R7K_NF_BYTES;
            }else{
                MMERROR(MBTRN,"err - mbtrn_read_nf [%d/%s]\n",me_errno,me_strerror(me_errno));
                //me_errno=ME_EREAD;
            }
        }else{
            MMDEBUG(MBTRN,"skipping NF flags[%d]\n",flags);
        }
        
        if (me_errno==ME_OK) {
            if ((read_bytes=mbtrn_read_drf_dep(self, pbuf, (R7K_MAX_FRAME_BYTES-R7K_NF_BYTES), pnf, newer_than, &sync_bytes, timeout_msec)) > 0) {
  
                MMDEBUG(MBTRN,"drf read OK [%"PRId64"]\n",read_bytes);
                pdrf = (r7k_drf_t *)(buf+R7K_NF_BYTES);
                
                drf_bytes+=read_bytes;
                total_bytes+=read_bytes;

#ifdef MBTRN_TIMING
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
                
                double stime = (c.QuadPart/(double)(1.e7))+(7.*3600.);
                
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

                struct timespec res;
                clock_getres(CLOCK_MONOTONIC, &res);
                fprintf(stderr,"%11.5lf cskew %+0.4e\n",iow_dtime(),(double)(stime-ptime));

#elif defined(__linux__)
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
                struct timespec res;
                clock_getres(CLOCK_MONOTONIC, &res);
                fprintf(stderr,"%11.5lf lskew %+0.4e\n",iow_dtime(), (double)(stime-ptime));
#else
                fprintf(stderr,"MBTRN_TIMING - skew not implemented\n");
               
#endif
#endif
                


            }else{
                MMERROR(MBTRN,"err - mbtrn_read_drf [%"PRId64"] [%d/%s]\n",read_bytes,me_errno,me_strerror(me_errno));
                me_errno=ME_EREAD;
           }
       }
        
       if (me_errno==ME_OK) {
            // check destination buffer space
            if ((len-nf_bytes) >= drf_bytes) {
                retval=0;
                MMDEBUG(MBTRN,"buf len OK\n");
                
                if (flags&MBR_NET_STREAM) {
                    MMDEBUG(MBTRN,"copying NF\n");
                    // if full frame, write NF header
                    memcpy(pdest,psrc,nf_bytes);
                    pdest+=nf_bytes;
                    psrc+=nf_bytes;
                    retval+=nf_bytes;
                }
                MMDEBUG(MBTRN,"copying DRF\n");
                // write DRF header
                memcpy(pdest,psrc,drf_bytes);
                retval += drf_bytes;
                
                // fill out info
                // [all pointers are byte pointers]
//                if (NULL != info) {
//                    MMDEBUG(MBTRN,"filling INFO\n");
//                   info->total_len = total_bytes;
//                    info->data_len  = pdrf->size - (R7K_DRF_BYTES + R7K_CHECKSUM_BYTES);
//                    if (flags&MBR_NET_STREAM) {
//                        info->nf   = dest;
//                        info->drf = (dest+R7K_NF_BYTES);
//                    }else{
//                        info->nf   = NULL;
//                        info->drf = dest;
//                    }
//                    info->data = ( info->drf+R7K_DRF_BYTES);
//                    info->checksum = (info->data + pdrf->size);
//                    info->timestamp = R7K_R7KTIME2D(pdrf->_7ktime);
//                    MMDEBUG(MBTRN,"nf[%p] drf[%p] dest[%p]\n",info->nf,info->drf,dest);
//               }
            }else{
                MMERROR(MBTRN,"err - insufficient buffer len [%"PRIu32"/%"PRIu32"]\n",(len-nf_bytes),drf_bytes);
                me_errno=ME_EWRITE;
            }
        }

        if (me_errno==ME_ESOCK || me_errno==ME_ERCV ) {
            MMERROR(MBTRN,"socket closed\n");
        }
        memset(buf,0,R7K_MAX_FRAME_BYTES);
        pbuf=buf;
        pnf=NULL;
        pdrf=NULL;
        
    }else{
        MERROR("invalid argument\n");
    }
    MMDEBUG(MBTRN,"returning [%lld]\n",retval);
    return retval;
}

/// @fn int64_t mbtrn_read_frames(mbtrn_reader_t *self, byte *dest, uint32_t len, uint32_t nframes, uint32_t max_age_ms )
/// @brief add data record frames to reader container w/ age new than specified time.
/// @param[in] self reader reference
/// @param[in] dest destination buffer
/// @param[in] nframes number of frames to read
/// @param[in] newer_than filter packets by time (reject if older than newer_than)
/// @param[in] timeout_msec timeout to use for reads
/// @return number of bytes returned in dest on success, -1 otherwise
int64_t mbtrn_read_frames_dep(mbtrn_reader_t *self, uint32_t nframes, double newer_than, uint32_t timeout_msec )
{
    int64_t retval=-1;
    me_errno = ME_OK;
    uint32_t frame_count=0;
    uint32_t sync_bytes=0;
    uint32_t total_bytes=0;
    uint32_t nf_bytes=0;
    uint32_t drf_bytes=0;
 
    if (NULL!=self && NULL!=mbtrn_reader_sockif(self) && NULL!=self->fc) {

        byte buf[R7K_MAX_FRAME_BYTES]={0};
        r7k_nf_t *pnf=NULL;
        r7k_drf_t *pdrf=NULL;
        byte *pbuf=buf;
        int64_t read_bytes=0;
        
        while ( ((nframes>0) && (frame_count<nframes)) ||
                ((nframes<=0) && (r7k_drfcon_space(self->fc) > R7K_MAX_FRAME_BYTES))
               ) {

            if (mbtrn_read_nf_dep(self, pbuf, &sync_bytes, timeout_msec) == 0) {
                
                MMDEBUG(MBTRN,"nf read OK\n");
                // update pointers
                pbuf = buf+R7K_NF_BYTES;
                pnf = (r7k_nf_t *)buf;
                nf_bytes+=R7K_NF_BYTES;
                total_bytes+=R7K_NF_BYTES;
                
                if ( (read_bytes=mbtrn_read_drf_dep(self, pbuf, (R7K_MAX_FRAME_BYTES-R7K_NF_BYTES), pnf, newer_than, &sync_bytes, timeout_msec)) > 0) {
                    
                    MMDEBUG(MBTRN,"drf read OK [%"PRId64"]\n",read_bytes);
                    pdrf = (r7k_drf_t *)(buf+R7K_NF_BYTES);
                    if(r7k_drfcon_add(self->fc,(byte *)pdrf, (uint32_t)read_bytes)==0){
                        frame_count++;
                        drf_bytes+=read_bytes;
                        total_bytes+=read_bytes;
                        MMDEBUG(MBTRN,">>>>> DRF ADD [%"PRIu32"/%"PRIu32"/%"PRId64"] >>>>>\n",frame_count, nframes,read_bytes);
                        MBTR_COUNTER_INC(self->stats->events[MBTR_EV_FRAME_VALID]);
                        MBTR_COUNTER_ADD(self->stats->status[MBTR_STA_NF_VAL_BYTES],nf_bytes);
                        MBTR_COUNTER_ADD(self->stats->status[MBTR_STA_DRF_VAL_BYTES],drf_bytes);
                        MBTR_COUNTER_ADD(self->stats->status[MBTR_STA_FRAME_VAL_BYTES],total_bytes);
                        if (mbtrn_reader_issub(self,pdrf->record_type_id)) {
                            MBTR_COUNTER_INC(self->stats->status[MBTR_STA_SUB_FRAMES]);
                        }
                        
                        if (NULL!=self->log) {
                            mlog_write(self->log_id,buf,total_bytes);
                        }
//                        r7k_nf_show(pnf,true,5);
//                        r7k_drf_show(pdrf,true,5);
//                        struct timeval stv={0};
//                        gettimeofday(&stv,NULL);
//                        double stime = (double)stv.tv_sec+((double)stv.tv_usec/1000000.0);
//                        
//                        r7k_time_t *t7k = &pdrf->_7ktime;
//                        double pti=0.0;
//                        double ptf=modf(t7k->seconds, &pti);
//                        struct tm tms = {0};
//                        char tstr[64]={0};
//                        sprintf(tstr,"%u %u %02d:%02d:%02.0f",t7k->year,t7k->day-1,t7k->hours+17,t7k->minutes,pti);
//
//                        strptime(tstr,"%Y %j %H:%M:%S",&tms);
//                        tms.tm_isdst=-1;
//                        time_t tt = mktime(&tms);
//                        double ptime=ptf+(double)tt;
////                        fprintf(stderr,"read : ptime[%.3lf] stime[%.3lf] (s-p)[%+6.3lf]\n",ptime,stime,(double)(stime-ptime));
//                        fprintf(stderr,"read : (s-p)[%+6.3lf]\n",(double)(stime-ptime));

                        retval=total_bytes;
                    }else{
                        MMERROR(MBTRN,"err - r7k_drfcon_add [%d/%s]\n",me_errno,me_strerror(me_errno));
                        me_errno=ME_EWRITE;
                        MBTR_COUNTER_INC(self->stats->events[MBTR_EV_EFCWR]);
                    }
                }else{
                    MMERROR(MBTRN,"err - mbtrn_read_drf [%"PRId64"] [%d/%s]\n",read_bytes,me_errno,me_strerror(me_errno));
                }
            }else{
                MMERROR(MBTRN,"err - mbtrn_read_nf [%d/%s]\n",me_errno,me_strerror(me_errno));
            }
            if (me_errno==ME_ESOCK || me_errno==ME_ERCV ) {
                MMERROR(MBTRN,"socket closed\n");
                break;
            }
            memset(buf,0,R7K_MAX_FRAME_BYTES);
            pbuf=buf;
            pnf=NULL;
            pdrf=NULL;
        }// while
    }else{
        MERROR("invalid argument\n");
        me_errno = ME_EINVAL;
    }
    MMDEBUG(MBTRN,"nframes[%"PRIu32"] frame_count[%"PRIu32"] space[%"PRIu32"]\n",nframes,frame_count,r7k_drfcon_space(self->fc));
    MMDEBUG(MBTRN,"sync_bytes[%"PRIu32"] total_bytes[%"PRIu32"]\n",sync_bytes,total_bytes);
    MMDEBUG(MBTRN,"nf_bytes[%"PRIu32"] drf_bytes[%"PRIu32"]\n",nf_bytes,drf_bytes);

    return retval;
}

int64_t mbtrn_refill(mbtrn_reader_t *self, uint32_t max_age_ms, uint32_t timeout_msec )
{
    int64_t retval=-1;
    me_errno = ME_OK;
 
    if (NULL!=self) {
        MBTR_COUNTER_INC(self->stats->events[MBTR_EV_FC_REFILL]);
       int64_t read_bytes=0;
        // empty drf container
        r7k_drfcon_flush(self->fc);
        
        double gmdtime=0.0;
        
        // get time, calculate greatest
        // acceptable packet age
        time_t tt_now=0;
        struct tm tm_now={0};
        
        time(&tt_now);
        gmtime_r(&tt_now, &tm_now);
        
        // note struct tm uses yday 0-365,
        // 7ktime uses yday 1-366
        gmdtime  = (tm_now.tm_yday+1)*SEC_PER_DAY;
        gmdtime += tm_now.tm_hour*SEC_PER_HOUR;
        gmdtime += tm_now.tm_min*SEC_PER_MIN;
        gmdtime += tm_now.tm_sec;

        // new_than 0:ignore age, >0: now-max_ave_ms
        double newer_than = ( (max_age_ms > 0) ? (gmdtime-((double)max_age_ms/1000.0)) : 0.0 );

        retval=0;
        if( (read_bytes=mbtrn_read_frames_dep(self, MBTRN_REFILL_FRAMES, newer_than, timeout_msec)) >= 0){
            MMDEBUG(MBTRN,"fill drfcon - frames[%"PRIu32"] read_bytes[%"PRId64"]\n",r7k_drfcon_frames(self->fc),read_bytes);
            retval=read_bytes;


        }else{
            
            MBTR_COUNTER_INC(self->stats->events[MBTR_EV_FRAME_INVALID]);

            MMERROR(MBTRN,"read_frames returned error [%d/%s]\n",me_errno,me_strerror(me_errno));
            if (me_errno==ME_ESOCK|| me_errno==ME_ERCV) {
                MMERROR(MBTRN,"socket closed\n");
                retval=-1;
            }
        }

//        while (r7k_drfcon_space(self->fc) > R7K_MAX_FRAME_BYTES) {
//            // use nframes<=0 to fill container
//            // use newer_than <=0 to ignore age
//            if( (read_bytes=mbtrn_read_frames(self,0,newer_than)) >= 0){
//                MMDEBUG(MBTRN,"fill drfcon - frames[%"PRIu32"] read_bytes[%"PRId64"]\n",r7k_drfcon_frames(self->fc),read_bytes);
//                retval+=read_bytes;
//            }else{
//                MMERROR(MBTRN,"read_frames returned error [%d/%s]\n",me_errno,me_strerror(me_errno));
//                if (me_errno==ME_ESOCK|| me_errno==ME_ERCV) {
//                    MMERROR(MBTRN,"socket closed\n");
//                    retval=-1;
//                    break;
//                }
//            }
//        }
    }else{
        MERROR("invalid argument\n");
        me_errno = ME_EINVAL;
    }
    return retval;
}

/// @fn int64_t mbtrn_reader_xread(mbtrn_reader_t * self, byte * dest, uint32_t len, uint32_t tmout_ms, mbtrn_flags_t flags)
/// @brief combined poll and parse operation. used by MB System to
/// present a file-like view of reson 7k center data in real time.
/// automatically detects empty buffer and refills.
/// @param[in] self reader reference
/// @param[in] dest data buffer for parsed data record frames
/// @param[in] len data buffer length in bytes
/// @param[in] tmout_ms poll timeout
/// @param[in] flags behavior flags. e.g. force to flush/poll,
/// @param[in] max_age_ms skip packets older than max_age_ms
/// enable/disable incomplete reads
/// @return number of bytes read on success, -1 otherwise (sets me_errno)
int64_t mbtrn_reader_xread(mbtrn_reader_t *self, byte *dest, uint32_t len, uint32_t tmout_ms,  mbtrn_flags_t flags, uint32_t max_age_ms)
{
    me_errno=ME_OK;
    int64_t retval=-1;
    int64_t read_bytes=0;
    int64_t total_bytes=0;
    
    memset(dest,0,len);
    if (NULL!=self && NULL!=dest && NULL!=self->fc) {
        
        uint32_t pending = 0;
		
        // if container is empty, refill
        // (even if non-blocking)
        if ( (pending=r7k_drfcon_pending(self->fc))==0) {
            mbtrn_refill(self, max_age_ms, tmout_ms);
        }
        
        // if non-blocking...
        // read what's available and return
        if ( ( (flags&MBR_NONBLOCK) > 0) ){
            
            if( (read_bytes = r7k_drfcon_read(self->fc,dest,len)) >= 0){
                MBTR_COUNTER_INC(self->stats->events[MBTR_EV_FC_READ]);
               MMDEBUG(MBTRN,"drfcon req<pend OK [%"PRId64"]\n",read_bytes);
                retval = read_bytes;
            }else{
                MMDEBUG(MBTRN,"drfcon read failed\n");
                me_errno=ME_EREAD;
            }
            
            // flush container if empty
            // so it will be filled on the next read
            if (r7k_drfcon_pending(self->fc)==0) {
                r7k_drfcon_flush(self->fc);
            }
            
        }else{
            // if blocking...
            // blocking for the frame buffer indicates that it should
            // refill the buffer if it's empty before enough bytes are available
            // [the socket it non-blocking, and uses the supplied timeout value]
            int64_t rem_bytes = len;
            // read pending frame bytes from container
            while (rem_bytes>0) {
                
                if( (read_bytes = r7k_drfcon_read(self->fc,dest,rem_bytes)) > 0){
                    MBTR_COUNTER_INC(self->stats->events[MBTR_EV_FC_READ]);
                    total_bytes += read_bytes;
                    // update remaining
                    rem_bytes -= read_bytes;
                    MMDEBUG(MBTRN,"read from drfcon - read_bytes[%"PRId64"] rem[%"PRId64"]\n",read_bytes,(rem_bytes));
                }else{
                    MMDEBUG(MBTRN,"drfcon read failed\n");
                    retval = -1;
                    me_errno=ME_EREAD;
                }
                
                // flush and refill container when empty
                if ( (pending=r7k_drfcon_pending(self->fc))==0) {
                    int rstat=0;
					
                    MMDEBUG(MBTRN,"refilling drfcon \n");

                    MBTR_SW_START(self->stats->measurements[MBTR_CH_MBTRN_REFILL_XT],iow_dtime());

                   if( (rstat=mbtrn_refill(self, max_age_ms, tmout_ms ))<=0){
                        // quit if socket closed, else keep trying
                        if (rstat<0) {
                            MMERROR(MBTRN,"refill error [%d/%s]\n",me_errno,me_strerror(me_errno));
                        }
                        if (me_errno==ME_ESOCK|| me_errno==ME_ERCV) {
                            MMERROR(MBTRN,"socket closed - quitting\n");
                            break;
                        }
                    }
                    MBTR_SW_LAP(self->stats->measurements[MBTR_CH_MBTRN_REFILL_XT],iow_dtime());
                }
          
                // break if done
                if (rem_bytes==0) {
                    MMDEBUG(MBTRN,"drfcon read complete\n");
                    retval=total_bytes;
                    break;
                }else if(rem_bytes<0) {
                    // this should never happen
                    MMERROR(MBTRN,"rem_bytes<0 [%"PRId64"]\n",rem_bytes);
                    me_errno=ME_EREAD;
                    break;
                }
            }
        }
        
    }else{
        MERROR("invalid argument\n");
        retval=-1;
        me_errno = ME_EINVAL;
    }
    return retval;
}

/// @fn int64_t mbtrn_reader_xread_frmae(mbtrn_reader_t * self, byte * dest, uint32_t len)
/// @brief combined poll and parse operation. used by MB System to
/// present a file-like view of reson 7k center data in real time.
/// automatically detects empty buffer and refills.
/// @param[in] self reader reference
/// @param[in] dest data buffer for parsed data record frames
/// @param[in] len data buffer length in bytes
/// @return number of bytes read on success, -1 otherwise (sets me_errno)
int64_t mbtrn_reader_xread_frame(mbtrn_reader_t *self, byte *dest, uint32_t len)
{
    int64_t retval=-1;
    if (NULL != self) {
        retval=0;
    }// else invalid arg
    return retval;
}

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
int64_t mbtrn_reader_xread_orig(mbtrn_reader_t *self, byte *dest, uint32_t len, uint32_t tmout_ms, mbtrn_flags_t flags)
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
                r7k_parse_stat_t stats_s = {0}, *stat = &stats_s;
                
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
                        MMERROR(MBTRN,"fc flush failed\n");
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
                        self->sockif->fd=-1;
                    }
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

/// @fn mbtrn_reader_issub(mbtrn_reader_t *self, uint32_t record_type)
/// @brief return true if record type is on subscription list.
/// @param[in] self reader reference
/// @param[in] record_type packet type to check
/// @return true if packet type is on subscription list, false otherwise
bool mbtrn_reader_issub(mbtrn_reader_t *self, uint32_t record_type)
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
// End function mbtrn_reader_issub


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


