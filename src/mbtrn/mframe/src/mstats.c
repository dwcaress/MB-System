///
/// @file mstats.c
/// @authors k. Headley
/// @date 11 aug 2017

/// Utilities for measuring and logging application metrics
/// including timing, event counting and status values

/// @sa doxygen-examples.c for more examples of Doxygen markup


/////////////////////////
// Terms of use
/////////////////////////
/*
 Copyright Information
 
 Copyright 2002-2019 MBARI
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

#include "mstats.h"

/////////////////////////
// Macros
/////////////////////////

// These macros should only be defined for 
// application main files rather than general C files
/*
/// @def PRODUCT
/// @brief header software product name
#define PRODUCT "mframe"

/// @def COPYRIGHT
/// @brief header software copyright info
#define COPYRIGHT "Copyright 2002-2019 MBARI Monterey Bay Aquarium Research Institute, all rights reserved."
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
bool g_mstat_test_quit=false;

/////////////////////////
// Function Definitions
/////////////////////////

/// @fn mstats_t *mstats_stats_new()
/// @brief create new stats structure
/// @return mstats_t on success, NULL otherwise
mstats_t *mstats_new(uint32_t ev_counters, uint32_t st_counters, uint32_t met_channels, const char ***labels)
{
    mstats_t *self = (mstats_t *)malloc(sizeof(mstats_t));
    if (self) {
        memset(self,0,sizeof(mstats_t));
        self->event_n=ev_counters;
        self->status_n=st_counters;
        self->metric_n=met_channels;
        self->per_stats = (mstats_metstats_t *)malloc(met_channels*sizeof(mstats_metstats_t));
        self->agg_stats = (mstats_metstats_t *)malloc(met_channels*sizeof(mstats_metstats_t));
        self->events    = (uint32_t *)malloc(ev_counters*sizeof(double));
        self->status    = (uint32_t *)malloc(st_counters*sizeof(double));
        self->metrics    = (mstats_metric_t *)malloc(met_channels*sizeof(mstats_metric_t));
        
        self->stat_period_start = 0.0;
        self->stat_period_sec   = 0.0;
        self->labels            = labels;
        memset(self->per_stats,    0, met_channels*sizeof(mstats_metstats_t));
        memset(self->agg_stats,    0, met_channels*sizeof(mstats_metstats_t));
        memset(self->events,       0, ev_counters*sizeof(uint32_t));
        memset(self->status,       0, st_counters*sizeof(uint32_t));
        memset(self->metrics, 0, met_channels*sizeof(mstats_metric_t));
    }
    return self;
}
// End function mstats_stats_new

/// @fn void mstats_stats_destroy(mstats_t **pself)
/// @brief release mstats_t resources
/// @return none
void mstats_destroy(mstats_t **pself)
{
    if (NULL!=pself) {
        mstats_t *self=(*pself);
        free(self->events);
        free(self->status);
        free(self->metrics);
        free(self->per_stats);
        free(self->agg_stats);
        free(self);
        *pself=NULL;
    }
}
// End function mstats_stats_destroy

/// @fn mstats_stats_set_period(mstats_t *self, double period_start, double period_sec)
/// @brief set stats period
/// @param[in] log_id log ID
/// @param[in] self mstats_stats reference
/// @param[in] period_start period start time
/// @param[in] period_sec period duration
/// @return none
void mstats_set_period(mstats_t *self, double period_start, double period_sec)
{
    if (NULL!=self) {
        self->stat_period_start=period_start;
        self->stat_period_sec=period_sec;
    }
}
// End function mstats_stats_set_period

/// @fn int mstats_log_timing(mlog_id_t log_id, mstats_metstats_t *stats,  double timestamp, char *type_str, const char **labels, int channels)
/// @brief log timing metrics
/// @param[in] log_id log ID
/// @param[in] stats mstats_stats reference
/// @param[in] timestamp time
/// @param[in] type_str channel type string
/// @param[in] labels pointer to channel labels
/// @param[in] channels pointer to channels
/// @return 0 on success, -1 otherwise
int mstats_log_timing(mlog_id_t log_id, mstats_metstats_t *stats,  double timestamp, char *type_str, const char **labels, int channels)
{
    int retval=-1;
    if (NULL!=stats && NULL!=labels && channels>0) {
   		int i=0;
        for (i=0; i<channels ; i++){
            mlog_tprintf(log_id,"%.3lf,%s,%s,%llu,%1.3g,%1.3g,%1.3g\n",
                         timestamp,
                         type_str,
                         labels[i],
                         stats[i].n,
                         stats[i].min,
                         stats[i].max,
                         stats[i].avg);
        }
        retval=0;
    }
    return retval;
}
// End function mstats_log_timing

/// @fn int mstats_log_counts(mlog_id_t log_id, uint32_t *counts, double timestamp, char *type_str, const char **labels, int channels)
/// @brief log statistics
/// @param[in] log_id log ID
/// @param[in] counts pointer to counts
/// @param[in] timestamp time
/// @param[in] type_str channel type string
/// @param[in] labels pointer to channel labels
/// @param[in] channels pointer to channels
/// @return 0 on success, -1 otherwise
int mstats_log_counts(mlog_id_t log_id, uint32_t *counts, double timestamp, char *type_str, const char **labels, int channels)
{
    int retval=-1;
    if (NULL!=counts && NULL!=labels && channels>0) {
    	int i=0;
        for (i=0; i<channels ; i++){
            mlog_tprintf(log_id,"%.3lf,%s,%s,%"PRIu32"\n",
                         timestamp,
                         type_str,
                         labels[i],
                         counts[i]);
        }
        retval=0;
    }
    return retval;
}
// End function mstats_log_counts

/// @fn int mstats_log_stats(mstats_t *stats, double now, mlog_id_t log_id, mstats_flags flags)
/// @brief log statistics
/// @param[in] stats mstats_stats reference
/// @param[in] now timestamp
/// @param[in] log_id log ID
/// @param[in] flags options
/// @return 0 on success, -1 otherwise
int mstats_log_stats(mstats_t *stats, double now, mlog_id_t log_id, mstats_flags flags)
{
    int retval=-1;
    if (NULL!=stats) {
        
        if (flags&MSF_STATUS) {
            // log status
            mstats_log_counts( log_id, stats->status,   now, "s", stats->labels[MSLABEL_STAT], stats->status_n);
        }
        if (flags&MSF_EVENT) {
            // log events
            mstats_log_counts( log_id, stats->events,   now, "e", stats->labels[MSLABEL_EVENT],  stats->event_n);
        }
        if (flags&MSF_PSTAT) {
            // log period stats
            mstats_log_timing( log_id, stats->per_stats, now, "p", stats->labels[MSLABEL_METRIC],  stats->metric_n);
        }
        if (flags&MSF_ASTAT) {
            // log aggregate statistics
            mstats_log_timing( log_id, stats->agg_stats, now, "a", stats->labels[MSLABEL_METRIC],  stats->metric_n);
        }
        retval=0;
    }
    return retval;
}
// End function mstats_log_stats

/// @fn mstats_reset_pstats(mstats_t *stats, uint32_t channels)
/// @brief reset statistics
/// @param[in] stats mstats_stats reference
/// @param[in] channels number of channels
/// @return none
void mstats_reset_pstats(mstats_t *stats, uint32_t channels)
{
    if (NULL != stats && channels>0) {
        // reset periodic stats
        memset(stats->per_stats,0,(channels*sizeof(mstats_metstats_t)));
    }
}
// End function mstats_reset_pstats

/// @fn int mstats_update_stats(mstats_t *stats, uint32_t channels, mstats_flags flags)
/// @brief update mbtrn statistics
/// @param[in] stats mstats_stats reference
/// @param[in] channels number of channels
/// @param[in] flags processing options
/// @return 0 on success, -1 otherwise
int mstats_update_stats(mstats_t *stats, uint32_t channels, mstats_flags flags)
{
    int retval=-1;
    
    if (NULL!=stats) {
        
        // update all stats channels
        uint32_t i=0;
        for (i=0; i<channels ; i++){
            // update periodic stats
            stats->per_stats[i].n++;
            stats->per_stats[i].sum += stats->metrics[i].value;
            if (stats->per_stats[i].n>1) {
                stats->per_stats[i].min = MST_STATS_SMIN( stats->per_stats[i], stats->metrics[i].value );
                stats->per_stats[i].max = MST_STATS_SMAX( stats->per_stats[i], stats->metrics[i].value );
                stats->per_stats[i].avg = MST_STATS_AVG ( stats->per_stats[i] );
            }else{
                stats->per_stats[i].min = stats->metrics[i].value;
                stats->per_stats[i].max = stats->metrics[i].value;
            }
            
            // update aggregate stats
            stats->agg_stats[i].n++;
            stats->agg_stats[i].sum += stats->metrics[i].value;
            if (stats->agg_stats[i].n>1) {
                stats->agg_stats[i].min = MST_STATS_SMIN( stats->agg_stats[i], stats->metrics[i].value );
                stats->agg_stats[i].max = MST_STATS_SMAX( stats->agg_stats[i], stats->metrics[i].value );
                stats->agg_stats[i].avg = MST_STATS_AVG ( stats->agg_stats[i] );
            }else{
                stats->agg_stats[i].min =stats->metrics[i].value;
                stats->agg_stats[i].max =stats->metrics[i].value;
            }
        }
        
        // reset measurement values
        for (i=0; i<channels ; i++){
            stats->metrics[i].value=0.0;
        }
        retval=0;
        
    } // else invalid arg
    
    return retval;
}
// End function mstats_update_stats

mstats_profile_t *mstats_profile_new(uint32_t ev_counters, uint32_t status_counters, uint32_t tm_channels, const char ***channel_labels, double pstart, double psec)
{
    mstats_profile_t *self =(mstats_profile_t *)malloc(sizeof(mstats_profile_t));
    if (self) {
        self->session_start = mtime_dtime();
        self->uptime = 0.0;
        self->stats = mstats_new(ev_counters, status_counters, tm_channels, channel_labels);
        
        mstats_set_period(self->stats,pstart,psec);
    }
    return self;
}
// End function mstats_profile_new

void mstats_profile_destroy(mstats_profile_t **pself)
{
    if(NULL!=pself){
        mstats_profile_t *self = (mstats_profile_t *)*pself;
        if(NULL!=self){
            if(NULL!=self->stats){
                mstats_destroy(&self->stats);
            }
            free(self);
            *pself=NULL;
        }
    }
}
// End function mstats_profile_destroy

#if defined(WITH_MSTATS_TEST)
/// @typedef enum mstats_event_id mstats_event_id
/// @brief diagnostic event IDs
typedef enum{
    MSAPP_OUTER_LOOP_COUNT=0,
    MSAPP_INNER_LOOP_COUNT,
    MSAPP_EVENT_COUNT
}mstats_event_id;

/// @typedef enum mstats_status_id mstats_status_id
/// @brief diagnostic status (integers) metrics IDs
typedef enum{
    MSAPP_SIN_GT=0,
    MSAPP_SIN_LT,
    MSAPP_SIN_EQ,
    MSAPP_STATUS_COUNT
}mstats_status_id;

/// @typedef enum mstats_metric_id mstats_metric_id
/// @brief diagnostic status (floating point) measurement IDs
typedef enum{
    MSAPP_CYCLE_XT=0,
    MSAPP_OUTER_LOOP_XT,
    MSAPP_INNER_LOOP_XT,
    MSAPP_LOG_XT,
    MSAPP_SLEEPN_XT,
    MSAPP_STATS_XT,
    MSAPP_METRIC_COUNT
}mstats_metric_id;

// define labels for stats channels
// indexed by mstats_x_id
static const char *test_event_labels[MSAPP_EVENT_COUNT]={ \
    "app_outer_n",
    "app_inner_n"
};
static const char *test_status_labels[MSAPP_STATUS_COUNT]={ \
    "sin_gt_0",
    "sin_lt_0",
    "sin_eq_0",
};
static const char *test_metric_labels[MSAPP_METRIC_COUNT]={ \
    "app_cycle_xt",
    "app_outer_xt",
    "app_inner_xt",
    "log_xt",
    "sleep-n_xt",
    "stats_xt"
};
// stats channel label container
// referenced by stats instance
static const char **test_stats_labels[]={
    test_event_labels,
    test_status_labels,
    test_metric_labels
};


#ifdef MST_STATS_EN
#define UPDATE_STATS(pstats,log_id,flags) (s_app_update_stats(pstats,log_id,flags))

static double stats_prev_end=0.0;
static double stats_prev_start=0.0;
static void s_app_update_stats(mstats_t *stats, mlog_id_t log_id, mstats_flags flags)
{
//    fprintf(stderr,"%s:%d\r\n",__FUNCTION__,__LINE__);
    if (NULL!=stats) {
        // get the status update entry time
        double stats_now = mtime_dtime();
        static bool log_clock_res=true;
        
        if (log_clock_res) {
            // log the timing clock resolution (one time)
            struct timespec res;
            clock_getres(CLOCK_MONOTONIC, &res);
            mlog_tprintf(log_id,"%.3lf,i,clkres_mono,s[%ld] ns[%ld]\n",stats_now,res.tv_sec,res.tv_nsec);
            log_clock_res=false;
        }
        
        // measure the time it takes to process stats
        // we can only measure the previous stats cycle...
        if (stats->per_stats[MSAPP_CYCLE_XT].n>0) {
            // get the timing of the last cycle
            MST_METRIC_START(stats->metrics[MSAPP_STATS_XT], stats_prev_start);
            MST_METRIC_LAP(stats->metrics[MSAPP_STATS_XT], stats_prev_end);
        }else{
            // or seed the first cycle
            MST_METRIC_START(stats->metrics[MSAPP_STATS_XT], (stats_now-0.0001));
            MST_METRIC_LAP(stats->metrics[MSAPP_STATS_XT], stats_now);
        }
        
        // end the cycle timer here
        // [start at the end if this function]
        MST_METRIC_LAP(stats->metrics[MSAPP_CYCLE_XT], stats_now);
        
        // update stats
        mstats_update_stats(stats, MSAPP_METRIC_COUNT, flags);

        // Other derivative status calculations here,
        // e.g. throughput

        // check stats periods, process (log) if ready
        if ( (stats->stat_period_sec>0.0) &&
            ((stats_now - stats->stat_period_start) >  stats->stat_period_sec)) {
            
            // start log execution timer
            MST_METRIC_START(stats->metrics[MSAPP_LOG_XT], mtime_dtime());
            
            mstats_log_stats(stats, stats_now, log_id, flags);
            
            // reset period stats
            mstats_reset_pstats(stats, MSAPP_METRIC_COUNT);

            // reset period timer
            stats->stat_period_start = stats_now;
            
            // stop log execution timer
            MST_METRIC_LAP(stats->metrics[MSAPP_LOG_XT], mtime_dtime());
        }
        
        // start cycle timer
       MST_METRIC_START(stats->metrics[MSAPP_CYCLE_XT], mtime_dtime());
        
        // update stats execution time variables
        stats_prev_start=stats_now;
        stats_prev_end=mtime_dtime();

    }
    return;
}
#else
#define UPDATE_STATS(pstats,log_id,flags)
#endif // MST_STATS_EN

int mstats_test()
{
    int retval=-1;
    
    unsigned int stats_period_s = 5;
    unsigned int ncycles = 3;
    int sleep_sec = 1;
    double PI = 3.141592;
    // stats instance
    mstats_t *stats=NULL;
    
    // log instance and configuration
    mlog_id_t MLOG_ID=MLOG_ID_INVALID;
    
    // monolithic log file, ISO1806 timestamp format
    // log to stderr and file; use ML_MONO|ML_DIS to disable file output
    
    // create a configuration
    // Note that mlog uses its own copy of the config, so changes
    // must be made via the mlog API
    
    mlog_config_t mlog_conf_inst={
        ML_NOLIMIT, ML_NOLIMIT, ML_NOLIMIT,
        ML_MONO, ML_SERR|ML_FILE,
        ML_TFMT_ISO1806,ML_DFL_DEL
    }, *mlog_conf=&mlog_conf_inst;
    
    // log file flags: create log if it doesn't exist, append if it does.
    mfile_flags_t log_flags = MFILE_RDWR|MFILE_APPEND|MFILE_CREATE;
    // log mode: r/w user, w group, i.e. 640
    mfile_mode_t log_mode = MFILE_RU|MFILE_WU|MFILE_RG|MFILE_WG;
    
    // create stats instance
    stats=mstats_new(MSAPP_EVENT_COUNT, MSAPP_STATUS_COUNT, MSAPP_METRIC_COUNT, test_stats_labels);
    // log/reset periodic stats every 30 s
    mstats_set_period(stats,mtime_dtime(),stats_period_s);
    
	// get log handle (not an OS file handle)
    MLOG_ID = mlog_get_instance("mstats.log",mlog_conf,"mstats test log");

    // open log file
    mlog_open(MLOG_ID, log_flags, log_mode);
    
    mlog_tprintf(MLOG_ID,"*** mstats-test session start ***\n");
    
    // start the main loop cycle time before we enter
    // [it will be updated and restarted at the end of each cycle]
    // typically, metrics are used to measure execution time
    MST_METRIC_START(stats->metrics[MSAPP_CYCLE_XT], mtime_dtime());
    MST_METRIC_START(stats->metrics[MSAPP_STATS_XT], mtime_dtime());
    while(!g_mstat_test_quit &&
          MST_COUNTER_GET(stats->events[MSAPP_OUTER_LOOP_COUNT])<(ncycles*stats_period_s/sleep_sec)){
        
        // set up two nested loops to measure
        int n=0;
        int m=0;
        // outer loop event counter
    	// typically, event counters count software events,
        // e.g. client connections, number of times a branch of code is entered, etc.
        MST_COUNTER_INC(stats->events[MSAPP_OUTER_LOOP_COUNT]);
        // outer loop timer start
        MST_METRIC_START(stats->metrics[MSAPP_OUTER_LOOP_XT], mtime_dtime());

        for(m=0;m<5;m++){
            // inner loop counter
            MST_COUNTER_INC(stats->events[MSAPP_INNER_LOOP_COUNT]);
            // inner loop timer start
            MST_METRIC_START(stats->metrics[MSAPP_INNER_LOOP_XT], mtime_dtime());
            for(n=0;n<2;n++){
                double k=0.0;
                for(k=0.0;k<2.0*PI;k+=PI/100.0){
                    double j=sin(k);
                    if(j==j){};
#ifdef MST_STATS_EN
                    // some "status" counters: number of times sin()
                    // is gt, lt, or eq zero
                    // typically, status counters are used to accumulate
                    // a quantity, e.g. bytes written/read
                    if(j>0)
                        MST_COUNTER_INC(stats->status[MSAPP_SIN_GT]);
                    else if(j<0)
                        MST_COUNTER_INC(stats->status[MSAPP_SIN_LT]);
                    else
                        MST_COUNTER_INC(stats->status[MSAPP_SIN_EQ]);

#endif
                }
            }
            // inner loop timer stop
            MST_METRIC_LAP(stats->metrics[MSAPP_INNER_LOOP_XT], mtime_dtime());
        }

        // measure sleep time (should be sleep_sec s)
        MST_METRIC_START(stats->metrics[MSAPP_SLEEPN_XT],mtime_dtime());
        sleep(sleep_sec);
        MST_METRIC_LAP(stats->metrics[MSAPP_SLEEPN_XT], mtime_dtime());
        
        // outer loop timer stop
        MST_METRIC_LAP(stats->metrics[MSAPP_OUTER_LOOP_XT], mtime_dtime());

        // update and log stats each time through the main loop.
        // periodic stats are only logged when the period timer expires.
        // A macro is used instead of calling the function directly, so that
        // it (and all mstats code) may be compiled out with a single macro definition
        UPDATE_STATS(stats,MLOG_ID,(MSF_STATUS|MSF_EVENT|MSF_PSTAT|MSF_ASTAT));
        
    }
    
    if(g_mstat_test_quit==false)
        retval=0;
    
    // close log
    mlog_close(MLOG_ID);

    // release log and stats instance resource
    mlog_delete_instance(MLOG_ID);
    mlog_delete_list(true);
    mstats_destroy(&stats);
    
    return retval;
}


#endif //MSTATS_WITH_TEST
