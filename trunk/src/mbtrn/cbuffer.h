///
/// @file cbuffer.h
/// @authors k. headley
/// @date 06 nov 2012
 
/// Circular memory buffer implementation

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
#ifndef CBUFFER_H
/// @def CBUFFER_H
/// @brief TBD
#define CBUFFER_H

/////////////////////////
// Includes 
/////////////////////////

#include <stdint.h>
#include <stdbool.h>
#include "iowrap.h"

/////////////////////////
// Type Definitions
/////////////////////////

/// @typedef uint32_t cbuf_flags_t
/// @brief cbuffer behavior flags
/// CB_NONE          no flags (0)
/// CB_ALLOW_PARTIAL allow partial reads (otherwise, return error if
///                  request exceeds available data
typedef uint32_t cbuf_flags_t;
/// @typedef enum cbuf_flag_t cbuf_flag_t
/// @brief cbuffer flag values
typedef enum {CB_NONE=0,CB_ALLOW_PARTIAL=0x1} cbuf_flag_t;
/// @typedef enum cbuf_status_t cbuf_status_t
/// @brief cbuffer status/error values
/// CB_OK    success
/// CB_UFLOW underflow (e.g. read request exceeds available)
/// CB_EMPTY buffer empty
/// CB_OFLOW overflow  (e.g. write request exceeds available space)
typedef enum {CB_OK=0,CB_UFLOW, CB_EMPTY, CB_FULL, CB_OFLOW} cbuf_status_t;

/// @typedef struct cbuffer_s cbuffer_t
/// @brief circular buffer structure. cbuffer is thread safe.
typedef struct cbuffer_s{
    /// @var cbuffer_s::capacity
    /// @brief buffer capacity
    uint32_t capacity;
    /// @var cbuffer_s::size
    /// @brief number of bytes currently in buffer
    uint32_t size;
    /// @var cbuffer_s::mutex
    /// @brief mutex
    iow_mutex_t *mutex;
    /// @var cbuffer_s::pwrite
    /// @brief write (input) pointer
    byte *pwrite;
    /// @var cbuffer_s::pread
    /// @brief read (output) pointer
    byte *pread;
    /// @var cbuffer_s::data
    /// @brief data buffer memory
    byte *data;
}cbuffer_t;

/////////////////////////
// Macros
/////////////////////////

/// @def CB_OFLAG_RETURN_AVAIL
/// @brief unused
#define CB_OFLAG_RETURN_AVAIL 0x10
/// @def CB_OFLAG_BLOCK
/// @brief unused
#define CB_OFLAG_BLOCK        0x20
/// @def CBUF_END(c)
/// @brief return byte pointer to end of buffer
/// @param[in] c cbuffer reference
#define CBUF_END(c)        ( (byte *)(c->data+c->capacity-1))
/// @def CBUF_IS_EMPTY(c)
/// @brief returns true if buffer is empty
/// @param[in] c cbuffer reference
#define CBUF_IS_EMPTY(c)   (c->size==0 ? true : false)
//#define CBUF_RWRAP(c)      ( ((c->data+c->capacity) - c->pread) + (self->pwrite - self->data) )
//#define CBUF_WWRAP(c)      ( ((c->data+c->capacity) - c->pwrite) + (self->pread - self->data) )
//#define CBUF_I2O(c)        (c->pwrite - c->pread)
//#define CBUF_O2I(c)        (c->pread - c->pwrite)
//#define CBUF_I2E(c)        ( c->data + c->capacity - c->pwrite)
//#define CBUF_O2E(c)        ( c->data + c->capacity - c->pread)
//#define CBUF_B2I(c)        ( c->pwrite - c->data )
//#define CBUF_B2O(c)        ( c->pread - c->data )
//#define CBUF_IS_WRAPPED(c) (c->pread > c->pwrite ? true : false)

/////////////////////////
// Exports
/////////////////////////

// Circular Buffer API
cbuffer_t *cbuf_new(uint32_t capacity);
void cbuf_destroy(cbuffer_t **pself);
void cbuf_show(cbuffer_t *self, bool verbose, uint16_t indent);

int cbuf_read(cbuffer_t *self, byte *dest, uint32_t len, cbuf_flag_t flags, int *status);
int cbuf_write(cbuffer_t *self, byte *src, uint32_t len, cbuf_flag_t flags, int *status);
uint32_t cbuf_available(cbuffer_t *self);
uint32_t cbuf_space(cbuffer_t *self);
int cbuf_clear(cbuffer_t *self);

// cbuf_set( from, to, value)
// cbuf_clearr( from, to, value)
//int cbuf_dup(cbuffer_t *self, cbuffer_t *dest);

//int cbuf_resize(cbuffer_t *self, uint32_t len);
//int cbuf_oflush(cbuffer_t *self);
int cbuf_test();

// include guard
#endif