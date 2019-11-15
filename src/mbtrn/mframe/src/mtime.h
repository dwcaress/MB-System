///
/// @file mtime.h
/// @authors k. headley
/// @date 06 nov 2012

/// mframe cross-platorm time wrappers

/// @sa doxygen-examples.c for more examples of Doxygen markup


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

// include guard
#ifndef MTIME_H
/// @def MTIME_H
/// @brief include guard
#define MTIME_H

/////////////////////////
// Includes
/////////////////////////
#include "mframe.h"

/////////////////////////
// Type Definitions
/////////////////////////

/// @typedef struct mtime_stopwatch_s mtime_stopwatch_t
/// @brief structure for measuring time intervals
/// and/or accumulated floating point values
typedef struct mtime_stopwatch_s
{
    /// @var mtime_stopwatch_s::start
    /// @brief measurement start time
    double start;
    /// @var mtime_stopwatch_s::stop
    /// @brief measurement stop time
    double stop;
    /// @var mtime_stopwatch_s::nsplits
    /// @brief number of splits
    unsigned int nsplits;
    /// @var mtime_stopwatch_s::value
    /// @brief measurement value
    double elapsed;
    /// @var mtime_stopwatch_s::res
    /// @brief clock resolution
    struct timespec res;
    /// @var mtime_stopwatch_s::split
    /// @brief measurement value
    double *split;
}mtime_stopwatch_t;

/////////////////////////
// Macros
/////////////////////////
#if defined(__MACH__)
// OSX clocks
// CALENDAR_CLOCK, HIGHRES_CLOCK
#define MTIME_DTIME_CLOCK CALENDAR_CLOCK
#elif defined(__QNX__)
// QNX clocks
// CLOCK_REALTIME
#define MTIME_DTIME_CLOCK CLOCK_REALTIME
#else
// linux clocks
// CLOCK_MONOTONIC, CLOCK_REALTIME, CLOCK_MONOTONIC_RAW
// CLOCK_PROCESS_CPUTIME_ID
#define MTIME_DTIME_CLOCK CLOCK_MONOTONIC
#endif


#ifndef WITHOUT_MTIME_SW
#define MTIME_STOPWATCH_EN 1
#endif

// enable using MTIME_STOPWATCH_EN definition in mconfig.h
#ifdef MTIME_STOPWATCH_EN

// some of these are trivial - using macros
// so they can be compiled out with one #define

/// @def MTIME_SW_GET_INSTANCE
/// @brief diagnostics - allocate a new stopwatch (caller must free with MTIME_SW_RELEASE)
#define MTIME_SW_NEW(ppw,n)  mtime_sw_new(ppw,n)
/// @def MTIME_SW_RELEASE
/// @brief diagnostics - release stopwatch instance
#define MTIME_SW_DESTROY(ppw)         mtime_sw_destroy(ppw)
/// @def MTIME_SW_START
/// @brief diagnostics - stopwatch start
#define MTIME_SW_START(pw,t)           (pw->start=t)
/// @def MTIME_SW_STOP
/// @brief diagnostics - stopwatch stop
#define MTIME_SW_STOP(pw,t)            (pw->stop=t)
/// @def MTIME_SW_ALLOC_SPLITS
/// @brief diagnostics - allocate split memory (n=0 to unallocate)
#define MTIME_SW_ALLOC_SPLITS(pw,n)    mtime_alloc_splits(pw,n)
/// @def MTIME_SW_SET_SPLIT
/// @brief diagnostics - stopwatch split (assign t to split[n])
#define MTIME_SW_SET_SPLIT(pw,n,t)     if(pw->split!=NULL && (unsigned int)n<pw->nsplits) pw->split[(unsigned int)n] = t;
/// @def MTIME_SW_RST_SPLITS
/// @brief diagnostics - clear (zero) stopwatch splits
#define MTIME_SW_RST_SPLITS(pw)        mtime_clr_splits(pw)
/// @def MTIME_SW_GET_SPLIT
/// @brief diagnostics - stopwatch lap (split[b]-split[a])
#define MTIME_SW_GET_SPLIT(pw,a,b)           ( (pw->split!=NULL && (unsigned int)a<pw->nsplits && (unsigned int)b<pw->nsplits) ? (pw->split[b]-pw->split[a]) : 0.0)
/// @def MTIME_SW_ACC
/// @brief diagnostics - stopwatch accumulate (increment elapsed by t-start)
#define MTIME_SW_ACC(pw,t)             (pw->elapsed += (t-pw->start))

/// @def MTIME_SW_EL_NOW
/// @brief diagnostics - current dtime-start
#define MTIME_SW_EL_NOW(pw)            (mtime_dtime()-pw->start))
/// @def MTIME_SW_EL_SAVE
/// @brief diagnostics - stopwatch save elapsed (assign stop-start to elapsed)
#define MTIME_SW_EL_SAVE(pw)           (pw->elapsed = (pw->stop-pw->start))
/// @def MTIME_SW_EL_DIV
/// @brief diagnostics - stopwatch div (divide and assign)
#define MTIME_SW_EL_DIV(pw,n)          (pw->elapsed = (pw->elapsed/(double)n))
/// @def MTIME_SW_EL_SET
/// @brief diagnostics - stopwatch set (set value=t)
#define MTIME_SW_EL_SET(pw,t)          (pw->elapsed=t)
/// @def MTIME_SW_EL_RST
/// @brief diagnostics - stopwatch reset (reset value=0)
#define MTIME_SW_EL_RST(pw)            (pw->elapsed=0.0)
/// @def MTIME_SW_ELAPSED
/// @brief diagnostics - stopwatch elapsed (value)
#define MTIME_SW_ELAPSED(pw)           (pw->elapsed)
/// @def MTIME_SW_GET_RES
/// @brief diagnostics - get clock resolution
#define MTIME_SW_GETRES(pw)           mtime_clock_getres(MTIME_DTIME_CLOCK,&pw->res)
/// @def MTIME_SW_SETRES
/// @brief diagnostics - get clock resolution
#define MTIME_SW_SET_RES(pw,n)        mtime_clock_setres(MTIME_DTIME_CLOCK,n,&pw->res)
/// @def MTIME_SW_RES
/// @brief diagnostics - get clock resolution
#define MTIME_SW_RES(pw)              (pw->res.tv_nsec)
/// @def MTIME_SW_GET_DTIME
/// @brief diagnostics - store dtime in pd
#define MTIME_SW_GET_DTIME(pd)           (*pd = mtime_dtime())

#else

#define MTIME_SW_NEW(ppw,n)
#define MTIME_SW_DESTROY(ppw)
#define MTIME_SW_START(pw,t)
#define MTIME_SW_STOP(pw,t)
#define MTIME_SW_ALLOC_SPLITS(pw,n)
#define MTIME_SW_SET_SPLIT(pw,n,t)
#define MTIME_SW_RST_SPLITS(pw,n)
#define MTIME_SW_GET_SPLIT(pw,a,b)   0.0
#define MTIME_SW_ACC(pw,t)           0.0
#define MTIME_SW_EL_NOW(pw)          0.0
#define MTIME_SW_EL_SAVE(pw)         0.0
#define MTIME_SW_EL_DIV(pw,n)        0.0
#define MTIME_SW_EL_SET(pw,t)        0.0
#define MTIME_SW_EL_RST(pw)
#define MTIME_SW_ELAPSED(pw)         0.0
#define MTIME_SW_GETRES(pw)         -1
#define MTIME_SW_SETRES(pw,n,pout)
#define MTIME_SW_RES(pw)             0L
#define MTIME_SW_GET_DTIME(pd)

#endif //MTIME_STOPWATCH_EN

/////////////////////////
// Exports
/////////////////////////
#ifdef __cplusplus
extern "C" {
#endif
    
    // mtime API
    
    /// @fn double mtime_dtime()
    /// @brief get system time as a double
    /// @return system time as double, with usec precision
    ///  if supported by platform
	double mtime_dtime();
    /// @fn double mtime_mdtime(double mod)
    /// @brief get system time as a double
    /// @return system time as double, with usec precision
    ///  if supported by platform
    double mtime_mdtime(double mod);
    /// @fn void mtime_delay_ns(uint32_t nsec)
    /// @brief delay for specied period
    /// @param[in] nsec delay period (nsec)
    /// @return none
    void mtime_delay_ns(uint32_t nsec);
    /// @fn void mtime_delay_ms(uint32_t msec)
    /// @brief delay for specied period
    /// @param[in] msec delay period (msec)
    /// @return none
    void mtime_delay_ms(uint32_t msec);
    /// @fn void mtime_alloc_splits(mtime_stopwatch_t *self, unsigned int n)
    /// @brief (re)allocate split times array
    /// @param[in] n number of split times
    /// @return none
    void mtime_alloc_splits(mtime_stopwatch_t *self, unsigned int n);
    /// @fn void mtime_clr_splits(mtime_stopwatch_t *self)
    /// @brief clear (set to zero) split times
    /// @return none
    void mtime_clr_splits(mtime_stopwatch_t *self);
    /// @fn mtime_sw_new(mtime_stopwatch_t **self, unsigned int splits)
    /// @brief allocate a new stopwatch.
    /// Caller owns: must free using mtime_sw_destroy
    /// @param[in] pself pointer to stopwatch instance (pointer)
    /// @param[in] splits initial size of splits array
    /// @return *pself points to new stopwatch on success, NULL otherwise
    void mtime_sw_new(mtime_stopwatch_t **pself, unsigned int splits);
    /// @fn mtime_sw_destroy(mtime_stopwatch_t **pself)
    /// @brief release stopwatch resources
    /// @param[in] pself pointer to stopwatch instance (pointer)
    /// @return *pself is set to NULL
    void mtime_sw_destroy(mtime_stopwatch_t **pself);
    /// @fn int mtime_clock_getres(int clock_id, struct timespec *res);
    /// @brief get clock resolution
    /// @param[in] clock_id clock ID
    /// @param[out] res  result stored in res->tv_nsec
    /// @return 0 on success, -1 otherwise; res->tv_nsec is set
    int mtime_clock_getres(int clock_id, struct timespec *res);
    /// @fn int mtime_clock_setres(int clock_id, struct timespec *res);
    /// @brief set clock resolution (not supported on all platforms)
    /// @param[in] clock_id clock ID
    /// @param[in] res  resolution in res->tv_nsec
    /// @return 0 on success, -1 otherwise; res->tv_nsec is set
    int mtime_clock_setres(int clock_id, struct timespec *res);
#ifdef WITH_MTIME_TEST
    /// @fn int mtime_test()
    /// @brief unit test
    /// @return 0 on success, -1 otherwise
    int mtime_test(int argc, char **argv);
#endif
    
#ifdef __cplusplus
}
#endif

// include guard
#endif
