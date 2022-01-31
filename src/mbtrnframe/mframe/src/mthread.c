///
/// @file mthread.c
/// @authors k. Headley
/// @date 01 jan 2018
 
/// mframe cross-platform thread wrappers implementation
/// for *nix/Cygwin

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
#include "mthread.h"
#include "mtime.h"

/////////////////////////
// Macros
/////////////////////////

// These macros should only be defined for 
// application main files rather than general C files
/*
/// @def PRODUCT
/// @brief header software product name
#define PRODUCT "MFRAME"

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

/////////////////////////
// Function Definitions
/////////////////////////


#if defined (__QNX__)
#pragma message("WARNING - MTHREAD API not implemented for QNX")

mthread_thread_t *mthread_thread_new()
{return NULL;}

void mthread_thread_destroy(mthread_thread_t **pself)
{return;}


int mthread_thread_start(mthread_thread_t *thread, mthread_thread_fn func, void *arg)
{return -1;}

int mthread_thread_join(mthread_thread_t *thread)
{return -1;}


// mfile mutex API
mthread_mutex_t *mthread_mutex_new()
{return NULL;}

void mthread_mutex_destroy(mthread_mutex_t **pself)
{return;}

int mthread_mutex_lock(mthread_mutex_t *self)
{return -1;}

int mthread_mutex_unlock(mthread_mutex_t *self)
{return -1;}

#else
/// @fn mthread_thread_t * mthread_thread_new()
/// @brief create new thread.
/// @return pointer to thread
mthread_thread_t *mthread_thread_new()
{
    mthread_thread_t *self = (mthread_thread_t *)malloc( sizeof(mthread_thread_t) );
    if (self) {
        self->status=NULL;
    }
    return self;
}
// End function mthread_thread_new

/// @fn void mthread_thread_destroy(mthread_thread_t ** pself)
/// @brief release thread resources.
/// @param[in] pself pointer to instance pointer (created with mthread_thread_new)
/// @return none
void mthread_thread_destroy(mthread_thread_t **pself)
{
    if (pself) {
        mthread_thread_t *self = *pself;
        if (self) {
            free(self);
            *pself=NULL;
        }
    }
}
// End function mthread_thread_destroy

/// @fn int mthread_thread_start(mthread_thread_t * thread, mthread_thread_fn func, void * arg)
/// @brief start thread.
/// @param[in] thread thread instance
/// @param[in] func thread entry point function
/// @param[in] arg pointer to thread arguments
/// @return 0 on success, -1 otherwise
int mthread_thread_start(mthread_thread_t *thread, mthread_thread_fn func, void *arg)
{
    int retval=0;
    
    pthread_attr_init(&thread->attr);
    pthread_attr_setdetachstate(&thread->attr, PTHREAD_CREATE_JOINABLE);
    
    if ( pthread_create(&thread->t, &thread->attr, func, arg) != 0)
    {
        fprintf(stderr,"error creating thread.");
        retval=-1;
    }
    // release attributes, no longer needed
    pthread_attr_destroy(&thread->attr);
    return retval;
}
// End function mthread_thread_start

/// @fn int mthread_thread_join(mthread_thread_t * thread)
/// @brief wait for thread to complete.
/// @param[in] thread thread instance
/// @return 0 on success, -1 otherwise
int mthread_thread_join(mthread_thread_t *thread){
    int retval=0;
    if ( thread && thread->t && pthread_join ( thread->t, (void **)&thread->status ) ) {
        fprintf(stderr,"error joining thread.");
        retval=-1;
    }
    return retval;
}
// End function mthread_thread_join

// mthread mutex API
/// @fn mthread_mutex_t * mthread_mutex_new()
/// @brief create and initialize new mutex.
/// @return mutex reference on success, or NULL otherwise
mthread_mutex_t *mthread_mutex_new()
{
    mthread_mutex_t *self = (mthread_mutex_t *)malloc(sizeof(mthread_mutex_t));
    if (self) {
        //self->m = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_init(&self->m, NULL);
    }
    return self;
}
// End function mthread_mutex_new

/// @fn void mthread_mutex_destroy(mthread_mutex_t ** pself)
/// @brief release mutex resources.
/// @param[in] pself pointer to mutex reference
/// @return none
void mthread_mutex_destroy(mthread_mutex_t **pself)
{
    if (pself) {
        mthread_mutex_t *self = *pself;
        if (self) {
            pthread_mutex_destroy(&self->m);
            free(self);
            *pself=NULL;
        }
    }
}
// End function mthread_mutex_destroy

/// @fn int mthread_mutex_lock(mthread_mutex_t * self)
/// @brief lock a mutex.
/// @param[in] self mutex reference
/// @return 0 on success, -1 otherwise
int mthread_mutex_lock(mthread_mutex_t *self)
{
    int retval=-1;
    if (self) {
        retval=pthread_mutex_lock(&self->m);
    }
    return retval;
}
// End function mthread_mutex_lock

/// @fn int mthread_mutex_unlock(mthread_mutex_t * self)
/// @brief unlock mutex.
/// @param[in] self mutex reference
/// @return 0 on success, -1 otherwise
int mthread_mutex_unlock(mthread_mutex_t *self)
{
	int retval=-1;
	if (self) {
 	   retval=pthread_mutex_unlock(&self->m);
    }
	return retval;
}
// End function mthread_mutex_unlock

#endif
