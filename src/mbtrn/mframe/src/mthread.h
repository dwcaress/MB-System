///
/// @file mthread.h
/// @authors k. headley
/// @date 06 nov 2012
 
/// mframe cross-platform thread wrappers
 
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
#ifndef MTHREAD_H
/// @def MTHREAD_H
/// @brief include guard
#define MTHREAD_H

/////////////////////////
// Includes 
/////////////////////////
#include "mframe.h"

/////////////////////////
// Macros
/////////////////////////

/////////////////////////
// Type Definitions
/////////////////////////

/// @struct mthread_thread_s
/// @brief wrapped thread representation (posix implementation)
struct mthread_thread_s;
/// @typedef struct mthread_thread_s mthread_thread_t
/// @brief wrapped thread typedef
typedef struct mthread_thread_s mthread_thread_t;

/// @struct mthread_mutex_s
/// @brief wrapped mutex representation (posix implementation)
struct mthread_mutex_s;
/// @typedef struct mthread_mutex_s mthread_mutex_t
/// @brief wrapped mutex typedef
typedef struct mthread_mutex_s mthread_mutex_t;

// @typedef void *(*)(void *) mbtrn_thread_fn
/// @brief thread function
/// @param[in] arg data to pass into thread function
typedef void *(* mthread_thread_fn)(void *arg);

/// @typedef struct mthread_thread_s mthread_thread_t
/// @brief wrapped thread representation (posix implementation)
struct mthread_thread_s
{
    /// @var mthread_thread_s::t
    /// @brief posix thread
    pthread_t t;
    /// @var mthread_thread_s::attr
    /// @brief thread attributes
    pthread_attr_t attr;
    /// @var mthread_thread_s::status
    /// @brief thread exit status
    void **status;
};

/// @typedef struct mthread_mutex_s mthread_mutex_t
/// @brief wrapped mutex representation (posix implementation)
struct mthread_mutex_s
{
    /// @var mthread_mutex_s::m
    /// @brief posix mutex
    pthread_mutex_t m;
};

/////////////////////////
// Exports
/////////////////////////
#ifdef __cplusplus
extern "C" {
#endif

    // mfile thread API (not implemented for QNX)
mthread_thread_t *mthread_thread_new();
void mthread_thread_destroy(mthread_thread_t **pself);

int mthread_thread_start(mthread_thread_t *thread, mthread_thread_fn func, void *arg);
int mthread_thread_join(mthread_thread_t *thread);

// mfile mutex API
mthread_mutex_t *mthread_mutex_new();
void mthread_mutex_destroy(mthread_mutex_t **pself);
int mthread_mutex_lock(mthread_mutex_t *self);
int mthread_mutex_unlock(mthread_mutex_t *self);
    
#ifdef __cplusplus
}
#endif


// include guard
#endif
