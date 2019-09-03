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
    /// @var mtime_stopwatch_s::value
    /// @brief measurement value
    double value;
}mtime_stopwatch_t;

/////////////////////////
// Macros
/////////////////////////


// enable using MTIME_STOPWATCH_EN definition in mconfig.h
#ifdef MTIME_STOPWATCH_EN
// some of these are trivial - using macros
// so they can be compiled out with one #define

/// @def MTIME_SW_START
/// @brief diagnostics - stopwatch start
#define MTIME_SW_START(w,t)           (w.start=t)
/// @def MTIME_SW_STOP
/// @brief diagnostics - stopwatch stop
#define MTIME_SW_STOP(w,t)            (w.stop=t)
/// @def MTIME_SW_LAP
/// @brief diagnostics - stopwatch lap (increment value by t-start)
#define MTIME_SW_LAP(w,t)             (w.value += (t-w.start))
/// @def MTIME_SW_REC
/// @brief diagnostics - stopwatch record (assign stop-start to value)
#define MTIME_SW_REC(w)               (w.value = (w.stop-w.start))
/// @def MTIME_SW_DIV
/// @brief diagnostics - stopwatch div (divide and assign)
#define MTIME_SW_DIV(w,n)             (w.value = (w.value/(double)n))
/// @def MTIME_SW_SET
/// @brief diagnostics - stopwatch set (set value=t)
#define MTIME_SW_SET(w,t)             (w.value=t)
/// @def MTIME_SW_RESET
/// @brief diagnostics - stopwatch reset (reset value=0)
#define MTIME_SW_RESET(w)             (w.value=0.0)
/// @def MTIME_SW_ELAPSED
/// @brief diagnostics - stopwatch elapsed (value)
#define MTIME_SW_ELAPSED(w)           (w.value)

#else

#define MTIME_SW_START(w,t)
#define MTIME_SW_STOP(w,t)
#define MTIME_SW_LAP(w,t)
#define MTIME_SW_REC(w)
#define MTIME_SW_DIV(w,n)
#define MTIME_SW_RESET(w)
#define MTIME_SW_ELAPSED(w)          0.0

#endif //MTIME_STOPWATCH_EN

/////////////////////////
// Exports
/////////////////////////
#ifdef __cplusplus
extern "C" {
#endif
    
    // iow time API
    double mtime_dtime();
    double mtime_mdtime(double mod);
    void mtime_delay_ns(uint32_t nsec);
    void mtime_delay_ms(uint32_t msec);
    
#ifdef __cplusplus
}
#endif

// include guard
#endif
