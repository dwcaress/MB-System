///
/// @file mstats.h
/// @authors k. headley
/// @date 11 aug 2017

/// Utilities for measuring and logging application metrics
/// including timing, event counting and status values

/// mstats consists of a set of data structures and macros
/// used to count events and quantities of interest, and to
/// measure time intervals (profiling). mstats tracks min, max
/// and averages for time measurements over a specified period
/// and/or aggregate (cumulative).
/// The mstat API is wrapped by macros, enabling all of the code to
/// be compiled in/out with a single macro definition (MST_STATS_EN)
/// Optionally, applications may use mlog to direct output to a file, stderr, and/or stdout.

/// mstats_t, mlog_t, and mlog_config_t are the primary data structures used.
/// mstats_t     : has separate integer counters for events and status quantities
/// mlog_t       : log instance (optional)
/// mlog_config_t: log settings (optional)

/// mstats.c includes a test function, mstats_test(), that demonstrates how the module is typically used.
/// The mframe library includes a C application wrapper (test-mstats.c) that calls this function. Compile
/// with WITH_

/// The basic steps to using mstats include:
/// - define channel IDs for event counters, status counters and metrics (time measurement) channels.
/// - define channel name labels (strings) for the channels
/// - typically, an update function and macro wrapper (to allow it to be compiled out) are defined
///   to update periodic stats, and direct output
/// - in application code, use the macros to gather statistics, and call the update function(s)

/// @sa doxygen-examples.c for more examples of Doxygen markup
/// @sa mlog, mfile, mthread, mtime, mdebug


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

// Always do this
#ifndef MSTATS_H
#define MSTATS_H

/////////////////////////
// Includes 
/////////////////////////
#include "mframe.h"
#include "mlog.h"
#include "mtime.h"

/////////////////////////
// Macros
/////////////////////////

// enable using MST_STATS_EN definition in mconfig.h
// or compile using -DWITH_MS_STATS
#ifdef WITH_MST_STATS
#ifndef MST_STATS_EN
//#define MST_STATS_EN
#endif
#endif

#ifdef MST_STATS_EN


// some of these are trivial - using macros
// so they can be compiled out with one #define

/// @def MST_METRIC_START
/// @brief diagnostics - stopwatch start
#define MST_METRIC_START(w,t)           (w.start=t)
/// @def MST_METRIC_STOP
/// @brief diagnostics - stopwatch stop
#define MST_METRIC_STOP(w,t)            (w.stop=t)
/// @def MST_METRIC_LAP
/// @brief diagnostics - stopwatch lap (increment value by t-start)
#define MST_METRIC_LAP(w,t)             (w.value += (t-w.start))
/// @def MST_METRIC_REC
/// @brief diagnostics - stopwatch record (assign stop-start to value)
#define MST_METRIC_REC(w)               (w.value = (w.stop-w.start))
/// @def MST_METRIC_DIV
/// @brief diagnostics - stopwatch div (divide and assign)
#define MST_METRIC_DIV(w,n)             (w.value = (w.value/(double)n))
/// @def MST_METRIC_SET
/// @brief diagnostics - stopwatch set (set value=t)
#define MST_METRIC_SET(w,t)             (w.value=t)
/// @def MST_METRIC_RESET
/// @brief diagnostics - stopwatch reset (reset value=0)
#define MST_METRIC_RESET(w)             (w.value=0.0)
/// @def MST_METRIC_ELAPSED
/// @brief diagnostics - stopwatch elapsed (value)
#define MST_METRIC_ELAPSED(w)           (w.value)

/// @def MST_COUNTER_INC
/// @brief diagnostics - counter increment
#define MST_COUNTER_INC(v)         (v++)
/// @def MST_COUNTER_DEC
/// @brief diagnostics - counter decrement
#define MST_COUNTER_DEC(v)         (v--)
/// @def MST_COUNTER_ADD
/// @brief diagnostics - counter add value
#define MST_COUNTER_ADD(v,n)       ((v) += (n))
/// @def MST_COUNTER_ADIF
/// @brief diagnostics - counter add (a-v)
#define MST_COUNTER_ADIF(v,a,b)    (v += (a-b))
/// @def MST_COUNTER_SET
/// @brief diagnostics - counter set value
#define MST_COUNTER_SET(v,n)       (v = n)
/// @def MST_COUNTER_GET
/// @brief diagnostics - get counter value
#define MST_COUNTER_GET(v)         (v)

/// @def MST_STATS_SMAX
/// @brief stats - greater of v,a
#define MST_STATS_SMAX(v,a)        ( (a > v.max) ? a : v.max)
/// @def MST_STATS_SMIN
/// @brief stats - less of v,a
#define MST_STATS_SMIN(v,a)        ( (a < v.min) ? a : v.min)
/// @def MST_STATS_AVG
/// @brief stats - average (or DBL_MAX if N<=0)
#define MST_STATS_AVG(v)           ( (v.n>0 ? (double)(v.sum)/(v.n) : DBL_MAX) )

#else
// disable (compile out) stats macros
#define MST_METRIC_START(w,t)
#define MST_METRIC_STOP(w,t)
#define MST_METRIC_LAP(w,t)
#define MST_METRIC_REC(w)
#define MST_METRIC_DIV(w,n)
#define MST_METRIC_RESET(w)
#define MST_METRIC_ELAPSED(w)          0.0

#define MST_COUNTER_INC(v)
#define MST_COUNTER_DEC(v)
#define MST_COUNTER_ADD(v,n)
#define MST_COUNTER_ADIF(v,a,b)
#define MST_COUNTER_SET(v,n)
#define MST_COUNTER_GET(v)         0.0
#define MST_STATS_SMAX(v,a)        0.0
#define MST_STATS_SMIN(v,a)        0.0
#define MST_STATS_AVG(v)           0.0
#endif //MST_STATS_EN

/////////////////////////
// Type Definitions
/////////////////////////


/// @typedef struct mstats_metric_stats_s mstats_metstats_t
/// @brief metric statistics
typedef struct mstats_metric_stats_s
{
    /// @var mstats_metric_stats_s::n
    /// @brief average N
    uint64_t n;
    /// @var mstats_metric_stats_s::sum
    /// @brief average sum
    double sum;
    /// @var mstats_metric_stats_s::min
    /// @brief min value
    double min;
    /// @var mstats_metric_stats_s::max
    /// @brief max value
    double max;
    /// @var mstats_metric_stats_s::avg
    /// @brief average value
    double avg;
}mstats_metstats_t;

/// @typedef struct mstats_metric_s mstats_metric_t
/// @brief structure for measuring continuous quantities
/// and intervals (e.g. floating point)
typedef struct mstats_metric_s
{
    /// @var mstats_metric_s::start
    /// @brief measurement start time
    double start;
    /// @var mstats_metric_s::stop
    /// @brief measurement stop time
    double stop;
    /// @var mstats_metric_s::value
    /// @brief measurement value
    double value;
}mstats_metric_t;

/// @typedef enum uint32_t mstats_counter_t
/// @brief diagnostic integer status type
typedef uint32_t mstats_counter_t;

/// @typedef enum mstats_flags mstats_flags
/// @brief diagnostic category types
typedef enum {MSF_STATUS=0x1, MSF_EVENT=0x2, MSF_PSTAT=0x4, MSF_ASTAT=0x8, MSF_READER=0x10}mstats_flags;

/// @typedef enum mstats_label_id mstats_label_id
/// @brief diagnostic label category types. Used to index label sets in mstats_t.labels
typedef enum {MSLABEL_EVENT=0, MSLABEL_STAT, MSLABEL_METRIC, MSLABEL_COUNT}mstats_label_id;

/// @typedef struct mstats_s mstats_t
/// @brief structure for diagnostic measurement channels
typedef struct mstats_s{
    /// @var mstats_s::stat_period_start
    /// @brief statics period start time (decimal seconds)
    double stat_period_start;
    /// @var mstats_s::stat_period_sec
    /// @brief statics period duration (s)
    double stat_period_sec;
    /// @var mstats_s::event_n
    /// @brief number of event channels
    uint32_t event_n;
    /// @var mstats_s::status_n
    /// @brief number of status channels
    uint32_t status_n;
    /// @var mstats_s::metric_n
    /// @brief number of timing/measurement channels
    uint32_t metric_n;
    /// @var mstats_s::events
    /// @brief integer event channels
    mstats_counter_t *events;
    /// @var mstats_s:: status
    /// @brief integer status channels
    mstats_counter_t *status;
    /// @var mstats_s::metrics
    /// @brief floating point measurement channels
    mstats_metric_t *metrics;
    /// @var mstats_s::per_stats
    /// @brief periodic stats
    mstats_metstats_t *per_stats;
    /// @var mstats_s::agg_stats
    /// @brief aggregate (cumulative) stats
    mstats_metstats_t *agg_stats;
    /// @var mstats_s::labels
    /// @brief metric channel labels
    const char ***labels;
}mstats_t;

/////////////////////////
// Exports
/////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

	extern bool g_mstat_test_quit;
    // mbtrn stats API
    mstats_t *mstats_new(uint32_t ev_counters, uint32_t status_counters, uint32_t tm_channels, const char ***labels);
    void mstats_destroy(mstats_t **pself);
    int mstats_update_stats(mstats_t *stats, uint32_t channels, mstats_flags flags);
    void mstats_reset_pstats(mstats_t *stats, uint32_t channels);
    void mstats_set_period(mstats_t *self, double period_start, double period_sec);
    int mstats_log_stats(mstats_t *stats, double now, mlog_id_t log_id, mstats_flags flags);
    int mstats_log_timing(mlog_id_t log_id, mstats_metstats_t *stats,  double timestamp, char *type_str, const char **labels, int channels);
    int mstats_log_counts(mlog_id_t log_id, uint32_t *counts, double timestamp, char *type_str, const char **labels, int channels);
    double mstats_dtime();
    
#if defined(WITH_MSTATS_TEST)
    int mstats_test();
#endif

#ifdef __cplusplus
}
#endif

#endif // THIS_FILE
