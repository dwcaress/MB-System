///
/// @file mlist.h
/// @authors k. headley
/// @date 06 nov 2012
 
/// Generic linked list implementation
 
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
#ifndef MLIST_H
/// @def MLIST_H
/// @brief include guard
#define MLIST_H

/////////////////////////
// Includes 
/////////////////////////

#include <stdbool.h>

/////////////////////////
// Type Definitions
/////////////////////////

// compare returns true if a,b are
// in the correct sorting order
/// @typedef _Bool (*)(void *, void *) mlist_cmp_fn
/// @brief compare function pointer type
typedef bool (* mlist_cmp_fn)(void *a, void *b);
// returns true if item i has value v or NULL if not found
/// @typedef _Bool (*)(void *, void *) mlist_ival_fn
/// @brief item value comparison function type
typedef bool (* mlist_ival_fn)(void *i, void *v);

// free function releases all resources
// pointed to by argument
/// @typedef void (*)(void *) mlist_free_fn
/// @brief resource free function type
typedef void (* mlist_free_fn)(void *pself);

/// @struct mlist_s
/// @brief mlist structure decl
struct mlist_s;
/// @typedef struct mlist_s mlist_t
/// @brief mlist structure type
typedef struct mlist_s mlist_t;

/// @struct mlist_item_s
/// @brief mlist item structure decl
struct mlist_item_s;
/// @typedef struct mlist_item_s mlist_item_t
/// @brief mlist item type
typedef struct mlist_item_s mlist_item_t;

/// @struct mlist_item_s
/// @brief mlist item structure definition
struct mlist_item_s
{
    /// @var mlist_item_s::data
    /// @brief item data reference
    void *data;
    /// @var mlist_item_s::free_fn
    /// @brief free function
    mlist_free_fn free_fn;
    /// @var mlist_item_s::next
    /// @brief pointer to next item in list
    mlist_item_t *next;
    /// @var mlist_item_s::fix_malloc
    /// @brief pad structure to fix apparent libc malloc issue
    unsigned char fix_malloc[8];
};

/// @struct mlist_s
/// @brief mlist structure definition
struct mlist_s
{
    /// @var mlist_s::afree_fn
    /// @brief autofree function
    mlist_free_fn afree_fn;
    /// @var mlist_s::head
    /// @brief list head pointer
    mlist_item_t *head;
    /// @var mlist_s::tail
    /// @brief list tail pointer
    mlist_item_t *tail;
    /// @var mlist_s::cursor
    /// @brief list iteration cursor
    mlist_item_t *cursor;
    /// @var mlist_s::size
    /// @brief list size (elements)
    size_t size;
};

/////////////////////////
// Macros
/////////////////////////

/////////////////////////
// Exports
/////////////////////////

mlist_t *mlist_new();
void mlist_destroy(mlist_t **pself);
void mlist_free(void *pself);
void *mlist_head(mlist_t *self);
void *mlist_tail(mlist_t *self);
void *mlist_first(mlist_t *self);
void *mlist_last(mlist_t *self);
void *mlist_next(mlist_t *self);
int mlist_add(mlist_t *self, void *item);
void mlist_remove(mlist_t *self, void *item);
int mlist_push(mlist_t *self, void *item);
void *mlist_pop(mlist_t *self);
void *mlist_item(mlist_t *self, void *item);
void *mlist_vlookup(mlist_t *self, void *value, mlist_ival_fn vcompare);
void mlist_sort(mlist_t *self, mlist_cmp_fn compare);
void mlist_purge(mlist_t *self);
void mlist_autofree(mlist_t *self, mlist_free_fn fn);
void mlist_freefn(mlist_t *self, void *item, mlist_free_fn fn);
size_t mlist_size(mlist_t *self);
int mlist_test();

// include guard
#endif