///
/// @file mcbuf.c
/// @authors k. Headley
/// @date 01 jan 2018
 
/// Circular memory buffer implementation

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

#include "mcbuf.h"

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

/// @fn mcbuffer_t * mcbuf_new(uint32_t capacity)
/// @brief return new circular buffer instance reference.
/// caller should release using mcbuf_destroy();
/// @param[in] capacity buffer capacity (bytes)
/// @return instance reference on success, NULL otherwise
mcbuffer_t *mcbuf_new(uint32_t capacity)
{
    mcbuffer_t *self = (mcbuffer_t *)malloc(sizeof(mcbuffer_t));
    if (NULL!=self) {
        self->capacity = capacity;
        self->size=0;
        self->data = (byte *)malloc(capacity*sizeof(byte));
        self->mutex = mthread_mutex_new();
        self->pwrite = self->data;
        self->pread = self->data;
    }else{
        fprintf(stderr,"malloc failed (%p)\n",self);
    }
    return self;
}
// End function mcbuf_new


/// @fn void mcbuf_destroy(mcbuffer_t ** pself)
/// @brief release circular buffer resources.
/// @param[in] pself pointer to instance reference
/// @return none
void mcbuf_destroy(mcbuffer_t **pself)
{
    if (NULL != pself) {
        mcbuffer_t *self = *pself;
        if (NULL != self) {
            if (NULL != self->mutex) {
                free(self->mutex);
            }
            if (NULL != self->data) {
                free(self->data);
            }
            free(self);
            *pself = NULL;
        }
    }
}
// End function mcbuf_destroy


/// @fn void mcbuf_show(mcbuffer_t * self, _Bool verbose, uint16_t indent)
/// @brief output circular buffer parameter summary to stderr.
/// @param[in] self cbuffer reference
/// @param[in] verbose use verbose output
/// @param[in] indent output indentation spaces
/// @return none
void mcbuf_show(mcbuffer_t *self, bool verbose, uint16_t indent)
{
        if (NULL != self) {
            fprintf(stderr,"%*s[self     %10p]\n",indent,(indent>0?" ":""), self);
            fprintf(stderr,"%*s[mutex    %10p]\n",indent,(indent>0?" ":""), self->mutex);
            fprintf(stderr,"%*s[capacity  x%0x/%u]\n",indent,(indent>0?" ":""), self->capacity, self->capacity);
            fprintf(stderr,"%*s[size     %10u]\n",indent,(indent>0?" ":""), self->size);
            fprintf(stderr,"%*s[data     %10p]\n",indent,(indent>0?" ":""), self->data);
            fprintf(stderr,"%*s[pread    %10p]\n",indent,(indent>0?" ":""), self->pread);
            fprintf(stderr,"%*s[pwrite   %10p]\n",indent,(indent>0?" ":""), self->pwrite);
            fprintf(stderr,"%*s[pend     %10p]\n",indent,(indent>0?" ":""), MCBUF_END(self));
            fprintf(stderr,"%*s[avail    %10u]\n",indent,(indent>0?" ":""), mcbuf_available(self));
            fprintf(stderr,"%*s[space    %10u]\n",indent,(indent>0?" ":""), mcbuf_space(self));
        }
}
// End function mcbuf_show


/// @fn int mcbuf_read(mcbuffer_t * self, byte * dest, uint32_t len, mcbuf_flag_t flags, int * status)
/// @brief read from circular buffer into destination buffer.
/// @param[in] self cbuffer reference
/// @param[in] dest destination buffer
/// @param[in] len number of bytes to read
/// @param[in] flags read behavior flags
/// @param[in] status contains exit/error status
/// @return number of bytes read on success, -1 otherwise (status is set)
/// @sa mcbuf_flag_t
int mcbuf_read(mcbuffer_t *self, byte *dest, uint32_t len, mcbuf_flag_t flags, int *status)
{
    int retval = -1;
    
    if (NULL != self && NULL != dest && len>0) {
        
        uint32_t read_len=0;

        if (self->size==0) {
            // buffer is empty
            read_len=0;
            *status = MCB_EMPTY;
        }else{
            if ( self->size >= len) {
                // full read available
                read_len=len;
                *status=MCB_OK;
            }else{
                // partial read available
                if ( (flags&MCB_ALLOW_PARTIAL) != 0 ) {
                    // read partial
                    read_len = self->size;
                    *status=MCB_OK;
                }else{
                    // don't write
                    read_len=0;
                    *status=MCB_UFLOW;
                }
            }
        }

        mthread_mutex_lock(self->mutex);
        if (read_len>0) {
            uint32_t count=0;
            for (count=0; count<read_len; count++) {
                dest[count] = *self->pread;
                *self->pread = 0x00;
                self->size--;
                if (self->pread==MCBUF_END(self)) {
                    self->pread=self->data;
                }else{
                	self->pread++;
                }
            }
            retval=count;
        }
        mthread_mutex_unlock(self->mutex);
        
    }else{
        fprintf(stderr,"invalid argument\n");
    }
    return retval;
}
// End function mcbuf_read



/// @fn int mcbuf_write(mcbuffer_t * self, byte * src, uint32_t len, mcbuf_flag_t flags, int * status)
/// @brief write to circular buffer.
/// @param[in] self cbuffer reference
/// @param[in] src source buffer
/// @param[in] len number of bytes to read
/// @param[in] flags write flags
/// @param[in] status contains exit/error status
/// @return number of bytes written on success, -1 otherwise (status is set)
/// @sa mcbuf_flag_t
int mcbuf_write(mcbuffer_t *self, byte *src, uint32_t len, mcbuf_flag_t flags, int *status)
{
    int retval = -1;
    
    if (NULL != self && NULL != src && len>0) {
        
        uint32_t write_len=0;

        if (self->size == self->capacity) {
            // buffer is empty
            write_len=0;
            *status = MCB_FULL;
        }else{
            if ( mcbuf_space(self) >= len) {
                // full read available
                write_len=len;
                *status=MCB_OK;
            }else{
                // partial read available
                if ( (flags&MCB_ALLOW_PARTIAL) != 0 ) {
                    // read partial
                    write_len = mcbuf_space(self);
                    *status=MCB_OK;
                }else{
                    // don't write
                    write_len=0;
                    *status=MCB_OFLOW;
                }
            }
        }
        
        mthread_mutex_lock(self->mutex);
        if (write_len>0) {
            uint32_t count=0;
            for (count=0; count<write_len; count++) {
                *self->pwrite = src[count];
                self->size++;
                if (self->pwrite==MCBUF_END(self)) {
                    self->pwrite=self->data;
                }else{
                    self->pwrite++;
                }
            }
            retval=count;
        }
        mthread_mutex_unlock(self->mutex);
        
    }else{
        fprintf(stderr,"invalid argument\n");
    }
    return retval;
}
// End function mcbuf_write


/// @fn uint32_t mcbuf_available(mcbuffer_t * self)
/// @brief number of unread bytes in buffer.
/// @param[in] self cbuffer reference
/// @return number of bytes available for reading.
uint32_t mcbuf_available(mcbuffer_t *self)
{
    uint32_t retval = 0;
    if (NULL != self && NULL != self->data) {
        retval=self->size;
    }else{
        fprintf(stderr,"invalid argument s[%p] d[%p]\n",self,self->data);
    }
    return retval;
}
// End function mcbuf_available


/// @fn uint32_t mcbuf_space(mcbuffer_t * self)
/// @brief amount of space available for writing.
/// @param[in] self cbuffer reference
/// @return number of bytes that may be written.
uint32_t mcbuf_space(mcbuffer_t *self)
{
    uint32_t retval = 0;
    if (NULL != self && NULL != self->data) {
        retval = self->capacity-self->size;
    }else{
        fprintf(stderr,"invalid argument s[%p] d[%p]\n",self,self->data);
    }
    return retval;
}
// End function mcbuf_space


/// @fn int mcbuf_clear(mcbuffer_t * self)
/// @brief clear circular buffer contents.
/// @param[in] self cbuffer reference
/// @return 0 on success, -1 otherwise
int mcbuf_clear(mcbuffer_t *self)
{
    int retval = -1;
    if (NULL!=self && NULL!=self->data && self->capacity>0) {
        retval=self->size;
        memset(self->data,0,self->capacity);
        self->pread=self->data;
        self->pwrite=self->data;
        self->size=0;
    }else{
        fprintf(stderr,"invalid argument s[%p] d[%p] c[%u]\n",self,self->data,self->capacity);
    }
    return retval;
}
// End function mcbuf_clear


//int mcbuf_oflush(mcbuffer_t *self);
//int mcbuf_resize(mcbuffer_t *self, uint32_t len);

/// @fn int mcbuf_test()
/// @brief cbuffer unit test(s).
/// @return TBD
int mcbuf_test()
{
#define RWCAP 32
    int retval=0;
    uint32_t cap=16;
    uint32_t rwcap=RWCAP;
    int ret=0;
    int status=MCB_OK;
    mcbuffer_t *b = mcbuf_new(cap);
    uint32_t avail = mcbuf_available(b);
    uint32_t space = mcbuf_space(b);
    byte wdata[RWCAP];
    byte rdata[RWCAP];
   uint32_t i=0;
    fprintf(stderr,"test start:\n");
    mcbuf_show(b,true,5);

    assert(avail==0);
    assert(space==cap);
    assert( (MCBUF_END(b)-b->data+1)==(int32_t)cap);

  
	// init IO buffers
    memset(wdata,0,rwcap);
    memset(rdata,0,rwcap);
 
    for ( i=0;i<rwcap; i++) {
        wdata[i]=i+0x20;
    }
    
    // read empty buffer
    ret = mcbuf_read(b,rdata,5,MCB_NONE,&status);
    assert(ret==-1);
    assert(status==MCB_EMPTY);
    
    // write > capacity
    ret = mcbuf_write(b,wdata,rwcap,MCB_NONE,&status);
    assert(ret==-1);
    assert(status==MCB_OFLOW);

    // write < capacity
    ret = mcbuf_write(b,wdata,10,MCB_NONE,&status);
    assert(ret==10);
    assert(status==MCB_OK);
    assert(mcbuf_available(b)==10);
    assert(mcbuf_space(b)==(cap-10));

    // write > capacity (allow partial)
    ret = mcbuf_write(b,(wdata+10),cap,MCB_ALLOW_PARTIAL,&status);
    assert(ret==(int)(cap-10));
    assert(status==MCB_OK);
    assert(mcbuf_available(b)==cap);
    assert(mcbuf_space(b)==0);

    // write to full buffer (allow partial)
    ret = mcbuf_write(b,(wdata+10),cap,MCB_ALLOW_PARTIAL,&status);
    assert(ret==-1);
    assert(status==MCB_FULL);
    assert(mcbuf_available(b)==cap);
    assert(mcbuf_space(b)==0);

    // read < available
    ret = mcbuf_read(b,rdata,10,MCB_NONE,&status);
    assert(ret==10);
    assert(status==MCB_OK);
    assert(mcbuf_available(b)==(cap-10));
    assert(mcbuf_space(b)==10);

    // read > available (no partial allowed)
    ret = mcbuf_read(b,rdata,cap,MCB_NONE,&status);
    assert(ret==-1);
    assert(status==MCB_UFLOW);
    assert(mcbuf_available(b)==(cap-10));
    assert(mcbuf_space(b)==10);
    
    // read > available (allow partial)
//    MTRACE();
//    mcbuf_show(b,false,5);
    ret = mcbuf_read(b,rdata,cap,MCB_ALLOW_PARTIAL,&status);
//    mcbuf_show(b,false,5);
    assert(ret==(int)(cap-10));
    assert(status==MCB_OK);
    assert(mcbuf_available(b)==0);
    assert(mcbuf_space(b)==cap);
    assert(MCBUF_IS_EMPTY(b));

    // cause pointer wrap
    mcbuf_write(b,wdata,cap,MCB_ALLOW_PARTIAL,&status);
    mcbuf_read(b,rdata,10,MCB_NONE,&status);
    ret = mcbuf_write(b,wdata,cap,MCB_ALLOW_PARTIAL,&status);
    assert(ret==10);
    assert(status==MCB_OK);
    assert(mcbuf_available(b)==cap);
    assert(mcbuf_space(b)==0);
    assert(MCBUF_IS_EMPTY(b)==false);

    // empty it
    ret = mcbuf_clear(b);
    assert(ret==(int)cap);
    assert(mcbuf_available(b)==0);
    assert(mcbuf_space(b)==cap);
    assert(MCBUF_IS_EMPTY(b)==true);

    fprintf(stderr,"test end:\n");
    mcbuf_show(b,true,5);
    mcbuf_destroy(&b);
    
    return retval;
}
// End function mcbuf_test



