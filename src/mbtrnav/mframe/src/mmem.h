///
/// @file mmem.h
/// @authors k. headley
/// @date 06 nov 2012
/// 
/// mframe reference counted memory allocation API.
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
#ifndef MB_MEMORY_H
#define MB_MEMORY_H

/////////////////////////
// Includes 
/////////////////////////
// for size_t
#include "mframe.h"

/////////////////////////
// Type Definitions
/////////////////////////
typedef struct mem_object_s mem_object_t;
typedef struct mem_info_s mem_info_t;

/////////////////////////
// Macros
/////////////////////////
/// @def REFCOUNT_VALID
/// @brief valid refcount signature
#define REFCOUNT_VALID 0x7F7F

/// @def REFCOUNT_INVALID
/// @brief invalid refcount signature
#define REFCOUNT_INVALID 0x5A5A

/////////////////////////
// Exports
/////////////////////////

void *mm_alloc(size_t size);
void *mm_realloc(void *mem, size_t size);
void mm_retain(void *mem);
void mm_release(void *mem);
int mm_refcount(void *ptr);

/// @fn mem_object_t *get_memory_obj(void *mem)
/// @brief tbd
/// @param[in] ptr description
/// @return tbd
mem_object_t *get_memory_obj(void *mem);
void mm_mem_stats(mem_info_t *minfo);
void show_mem_obj(mem_object_t *pmo);
void show_mem_stats();

#endif // THIS_FILE
