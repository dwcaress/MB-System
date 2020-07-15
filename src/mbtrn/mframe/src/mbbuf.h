///
/// @file mbbuf.h
/// @authors k. headley
/// @date 06 nov 2012
/// 
/// mframe byte buffer API.
/// Dynamic, reference counted buffers. 
/// Automatically resize when writing.
/// 
/// @sa doxygen-examples.c for more examples of Doxygen markup

/////////////////////////
// Terms of use 
/////////////////////////

/*
 Copyright Information
 
 Copyright 2002-2013 MBARI
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

// Always do this.
#ifndef MBBUF_H
#define MBBUF_H

/////////////////////////
// Includes 
/////////////////////////

// for byte
#include "mframe.h"

/////////////////////////
// Type Definitions
/////////////////////////
typedef struct mbbuf_s mbbuf_t;

/////////////////////////
// Macros
/////////////////////////
/// @def MB_SEEK_HEAD
/// @brief seek start of buffer
#define MB_SEEK_HEAD -1
/// @def MB_SEEK_TAIL
/// @brief seek end of buffer
#define MB_SEEK_TAIL -2

/////////////////////////
// Exports
/////////////////////////

mbbuf_t *mbb_new(off_t capacity, byte *data, off_t size);
void mbb_destroy(mbbuf_t **p_self);
void mbb_free(void *self);

mbbuf_t *mbb_dup(mbbuf_t *self);
int mbb_set_capacity(mbbuf_t *self, off_t size);

int mbb_seek(mbbuf_t *self, off_t offset);
int mbb_iseek(mbbuf_t *self, off_t offset);
int mbb_oseek(mbbuf_t *self, off_t offset);

int mbb_set(mbbuf_t *self, off_t offset, off_t len, byte b);
int mbb_reset(mbbuf_t *self);
int mbbuf_trim(mbbuf_t *self);
int mbb_append(mbbuf_t *self, byte *data, off_t size);
int mbb_push(mbbuf_t *self,byte *data, off_t size);
byte *mbb_pop(mbbuf_t *self, off_t len);
byte *mbb_read(mbbuf_t *self, off_t len);
int mbb_write(mbbuf_t *self, byte *data, off_t size);
int mbb_printf(mbbuf_t *self, const char *fmt,...);
byte *mbb_head(mbbuf_t *self);

off_t mbb_capacity(mbbuf_t *self);
off_t mbb_length(mbbuf_t *self);
off_t mbb_available(mbbuf_t *self);
off_t mbb_iavailable(mbbuf_t *self);
off_t mbb_icursor(mbbuf_t *self);
off_t mbb_ocursor(mbbuf_t *self);
void mbb_buf_show(mbbuf_t *self, bool verbose, int indent);
void mbb_dump(mbbuf_t *self);

#ifdef WITH_MBBUF_TEST
int mbbuf_test(int verbose);
#endif //WITH_MBBUF_TEST
#endif  // THIS_FILE
