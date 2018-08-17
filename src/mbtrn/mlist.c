///
/// @file mlist.c
/// @authors k. Headley
/// @date 01 jan 2018
 
/// Generic linked list implementation

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

#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>
#include <memory.h>
// assert only used in test
#include <assert.h>
// string only used in test
#include <string.h>

#include "mlist.h"
#include "mdebug.h"

/////////////////////////
// Macros
/////////////////////////

// These macros should only be defined for 
// application main files rather than general C files
/*
/// @def PRODUCT
/// @brief header software product name
#define PRODUCT "MBRT"

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

/// @fn mlist_item_t * s_mlist_partition(mlist_item_t * head, mlist_item_t * end, mlist_item_t ** newHead, mlist_item_t ** newEnd, mlist_cmp_fn compare)
/// @brief mlist qsort partioning.
/// @param[in] head list head
/// @param[in] end list tail
/// @param[in] newHead pointer to new list head reference
/// @param[in] newEnd pointer to new list head reference
/// @param[in] compare list item comparison function
/// @return new list head
static mlist_item_t *s_mlist_partition(mlist_item_t *head, mlist_item_t *end,
                                       mlist_item_t **newHead, mlist_item_t **newEnd, mlist_cmp_fn compare)
{
    mlist_item_t *pivot = end;
    mlist_item_t *prev = NULL, *cur = head, *tail = pivot;
    
    // During partition, both the head and end of the list might change
    // which is updated in the newHead and newEnd variables
    while (cur != pivot){
        if (compare(cur->data,pivot->data)){
            // First node that has a value less than the pivot - becomes
            // the new head
            if ((*newHead) == NULL)
                (*newHead) = cur;
            
            prev = cur;
            cur = cur->next;
        }else{ // If cur node is greater than pivot
            
            // Move cur node to next of tail, and change tail
            if (prev)
                prev->next = cur->next;
            mlist_item_t *tmp = cur->next;
            cur->next = NULL;
            tail->next = cur;
            tail = cur;
            cur = tmp;
        }
    }
    
    // If the pivot data is the smallest element in the current list,
    // pivot becomes the head
    if ((*newHead) == NULL)
        (*newHead) = pivot;
    
    // Update newEnd to the current last node
    (*newEnd) = tail;
    
    // Return the pivot node
    return pivot;
}
// End function s_mlist_partition


/// @fn mlist_item_t * s_mlist_rsort(mlist_item_t * head, mlist_item_t * tail, mlist_cmp_fn compare)
/// @brief mlist recursive quick-sort function.
/// @param[in] head list head
/// @param[in] tail list tail
/// @param[in] compare list item comparison function
/// @return sorted list
static mlist_item_t *s_mlist_rsort(mlist_item_t *head, mlist_item_t *tail, mlist_cmp_fn compare)
{
    mlist_item_t *retval=head;
    if (NULL!=head && NULL!=compare) {
        // base condition
        if (NULL==head || head == tail)
            return head;
        
        mlist_item_t *newHead = NULL, *newTail = NULL;
        
        // Partition the list, newHead and newEnd will be updated
        // by the partition function
        mlist_item_t *pivot = s_mlist_partition(head, tail, &newHead, &newTail,compare);
        
        // If pivot is the smallest element - no need to recur for
        // the left part.
        if (newHead != pivot)
        {
            // Set the node before the pivot node as NULL
            mlist_item_t *tmp = newHead;
            while (tmp->next != pivot)
                tmp = tmp->next;
            tmp->next = NULL;
            
            // Recur for the list before pivot
            newHead = s_mlist_rsort(newHead, tmp, compare);
            
            // Change next of last node of the left half to pivot
            mlist_item_t *cur=newHead;
            while (cur != NULL && cur->next != NULL){
                cur = cur->next;
            }
            tmp = cur;
            tmp->next =  pivot;
        }
        
        // Recur for the list after the pivot element
        pivot->next = s_mlist_rsort(pivot->next, newTail,compare);
        
        retval = newHead;
    }
    return retval;
}
// End function s_mlist_rsort


/// @fn mlist_item_t * mlist_item_new(void * item)
/// @brief create new mlist item.
/// @param[in] item list item data
/// @return new list item reference on success, NULL otherwise
mlist_item_t *mlist_item_new(void *item)
{
    // TODO: this malloc crashes in mbtrnpreprocess
    // on Ubuntu 16.04, gcc
    // adding a few extra bytes to the struct seems to fix it
    // valgrind does not indicate any errors
    // is sizeof() getting this wrong?
    mlist_item_t *self = (mlist_item_t *)malloc(sizeof(mlist_item_t));
    if (NULL!=self) {
        memset(self,0,sizeof(mlist_item_t));
        self->data    = item;
        self->free_fn = NULL;
        self->next    = NULL;
    }else{
        MERROR("malloc failed\n");
    }
    return self;
}
// End function mlist_item_new


/// @fn void mlist_item_destroy(mlist_item_t ** pself)
/// @brief release list item resources.
/// @param[in] pself pointer to instance reference
/// @return none
void mlist_item_destroy(mlist_item_t **pself)
{
    if (NULL!=pself) {
        mlist_item_t *self = *pself;
        if (self) {
            free(self);
            *pself=NULL;
        }
    }

}
// End function mlist_item_destroy


/// @fn mlist_t * mlist_new()
/// @brief create new mlist.
/// @return new mlist reference on success, NULL otherwise
mlist_t *mlist_new()
{
    mlist_t *self = (mlist_t *)malloc(sizeof(mlist_t));
    if (self) {
        memset(self,0,sizeof(mlist_t));
        self->head     = NULL;
        self->tail     = NULL;
        self->afree_fn = NULL;
        self->cursor   = NULL;
    }
    return self;
}
// End function mlist_new


/// @fn void mlist_destroy(mlist_t ** pself)
/// @brief release mlist resources. optionally release item resources
/// using global autofree function or per-item free function(s).
/// @param[in] pself pointer to instance reference
/// @return none
void mlist_destroy(mlist_t **pself)
{
    if (NULL!=pself) {
        mlist_t *self = *pself;
        if (NULL!=self) {
            
            // release item data
            if (self->size>0) {
                // release list items
                self->cursor = self->head;
                if (NULL!=self->cursor) {
                    mlist_item_t *pitem=NULL;
                    mlist_item_t *pnext=NULL;
                    while (self->size>0) {
                        if (NULL != self->cursor->free_fn) {
                            // use per item free if set
                             self->cursor->free_fn(self->cursor->data);
                        }else if(NULL!=self->afree_fn){
                            // else use autofree if set
                            self->afree_fn(self->cursor->data);
                        }
                        // update pointers and release
                        // list item resources
                        pnext=self->cursor->next;
                        pitem=self->cursor;
                        self->cursor=pnext;
                        free(pitem);
                        self->size--;
                    }
                }
            }
            // free list resources
            free(self);
            *pself=NULL;
        }
    }
}
// End function mlist_destroy


/// @fn void mlist_free(void * pself)
/// @brief release mlist resources (using free semantics).
/// @param[in] pself void mlist reference
/// @return none
void mlist_free(void *pself)
{
    mlist_t *self = (mlist_t *)pself;
    if (NULL!=self) {
        
        // release item data
        if (self->size>0) {
            // release list items
            self->cursor = self->head;
            if (NULL!=self->cursor) {
                mlist_item_t *pitem=NULL;
                mlist_item_t *pnext=NULL;
                while (self->size>0) {
                    if (NULL != self->cursor->free_fn) {
                        // use per item free if set
                        self->cursor->free_fn(self->cursor->data);
                    }else if(NULL!=self->afree_fn){
                        // else use autofree if set
                        self->afree_fn(self->cursor->data);
                    }
                    // update pointers and release
                    // list item resources
                    pnext=self->cursor->next;
                    pitem=self->cursor;
                    self->cursor=pnext;
                    free(pitem);
                    self->size--;
                }
            }
        }
        // free list resources
        free(self);
        pself=NULL;
    }
}
// End function mlist_free



/// @fn void * mlist_head(mlist_t * self)
/// @brief return void reference to first list item data.
/// @param[in] self mlist reference
/// @return list item data pointer on success, NULL otherwise
void *mlist_head(mlist_t *self)
{
    void *retval=NULL;
    if(NULL!=self && NULL!=self->head){
        retval=self->head->data;
    }
    return retval;
}
// End function mlist_head


/// @fn void * mlist_tail(mlist_t * self)
/// @brief return void reference to last list item data.
/// @param[in] self mlist reference
/// @return list item data pointer on success, NULL otherwise
void *mlist_tail(mlist_t *self)
{
    void *retval=NULL;
    if(NULL!=self && NULL!=self->tail){
        retval=self->tail->data;
    }
    return retval;
}
// End function mlist_tail


/// @fn void * mlist_first(mlist_t * self)
/// @brief return void reference to first list item data. also sets list
/// iterator cursor -> head.
/// @param[in] self mlist reference
/// @return list item data pointer on success, NULL otherwise
void *mlist_first(mlist_t *self)
{
    void *retval=NULL;
    if(NULL!=self && NULL!=self->head){
        self->cursor=self->head;
        retval=self->cursor->data;
    }
    return retval;
}
// End function mlist_first


/// @fn void * mlist_last(mlist_t * self)
/// @brief return void reference to last list item data. also sets list
/// iterator cursor -> tail.
/// @param[in] self mlist reference
/// @return list item data pointer on success, NULL otherwise
void *mlist_last(mlist_t *self)
{
    void *retval=NULL;
    if (NULL!=self && NULL!=self->tail) {
        self->cursor = self->tail;
        retval = self->cursor->data;
    }
    return retval;
}
// End function mlist_last


/// @fn void * mlist_next(mlist_t * self)
/// @brief get next list item data. initialize to head using mlist_first.
/// @param[in] self mlist reference
/// @return list item data pointer on success, NULL otherwise
void *mlist_next(mlist_t *self)
{
    void *retval=NULL;
    if (NULL!=self && NULL!=self->cursor) {
        self->cursor = (self->cursor!=NULL?self->cursor->next : NULL);
        retval = (self->cursor!=NULL ? self->cursor->data : NULL);
    }
    return retval;
}
// End function mlist_next


/// @fn void mlist_add(mlist_t * self, void * item)
/// @brief add an item to mlist.
/// @param[in] self mlist reference
/// @param[in] item pointer to item data
/// @return 0 on success, -1 otherwise
int mlist_add(mlist_t *self, void *item)
{
    int retval=-1;
    if (NULL!=self && NULL!=item) {
        mlist_item_t *new_item = mlist_item_new(item);
        if (NULL!=new_item) {
            new_item->next=NULL;
            if (NULL==self->tail) {
                // list empty
                self->head   = new_item;
                self->tail   = new_item;
                self->cursor = new_item;
            }else{
                // list populated
                // add to end
                self->cursor=(self->cursor==self->tail?new_item:self->cursor);
                self->tail->next = new_item;
                self->tail=new_item;
            }
            // adjust size
            self->size++;
            retval=0;
        }else{
            MERROR("mlist_item_new failed\n");
        }
    }
    return retval;
}
// End function mlist_add


/// @fn void mlist_remove(mlist_t * self, void * item)
/// @brief remove item from mlist.
/// @param[in] self mlist reference
/// @param[in] item item reference
/// @return removes item (destroys item resources if autofree or per-item free
/// function are set)
void mlist_remove(mlist_t *self, void *item)
{
    if (NULL!=self && NULL!=item) {
        // check whether list contains item
        if (mlist_size(self)>0) {
            mlist_item_t *plist = self->head;
            mlist_item_t *pb    = NULL;
            mlist_item_t *pa    = (plist!=NULL ? plist->next : NULL);

            while (plist!=NULL) {
                if (plist->data == item) {
                    if (NULL==pb) {
                        // is head...
                        if (pa==NULL) {
                            // is tail (i.e. single item)
                            self->head=NULL;
                            self->tail=NULL;
                            self->cursor=NULL;
                        }else{
                        	// not tail
                            self->head=pa;
                            self->cursor=(self->cursor==plist? pa : self->cursor);
                        }
                    }else{
                        // not head...
                        if (pa==NULL) {
                            // is tail
                            self->tail=pb;
                            self->tail->next=NULL;
                            self->cursor=(self->cursor==plist? NULL : self->cursor);
                        }else{
                            // not tail
                            pb->next=pa;
                            // move cursor to next item
                            self->cursor=(self->cursor==plist? pa : self->cursor);
                        }
                    }

                    //MDEBUG("freeing item[%p] plist[%p] w/ destroy\n",plist->data,plist);
                    // free item, list entry
                    if (plist->free_fn!=NULL) {
                        plist->free_fn(plist->data);
                    }else if(self->afree_fn!=NULL){
                        self->afree_fn(plist->data);
                    }

                    mlist_item_destroy(&plist);
                    // adjust size
                    self->size--;
                    break;
                }
                
                pb=plist;
                plist=plist->next;
                pa=(plist!=NULL ? plist->next : NULL);
            }
        }
    }
}
// End function mlist_remove


/// @fn void mlist_push(mlist_t * self, void * item)
/// @brief push item onto list (insert at head).
/// @param[in] self mlist reference
/// @param[in] item item reference
/// @return 0 on success, -1 otherwise
int mlist_push(mlist_t *self, void *item)
{
    int retval=-1;
    
    if (NULL!=self && NULL!=item) {
        mlist_item_t *new_item = mlist_item_new(item);
        if (new_item!=NULL) {
            if (self->cursor==self->head || self->cursor==NULL) {
                self->cursor=new_item;
            }
            new_item->next=(self->head==NULL?NULL:self->head);
            self->head=new_item;
            self->tail=(self->tail==NULL?self->head:NULL);
            self->size++;
            retval=0;
        }
    }
    return retval;
}
// End function mlist_push


/// @fn void * mlist_pop(mlist_t * self)
/// @brief pop item of list (return first item, and remove it from the list).
/// @param[in] self mlist reference
/// @return first item data on success, NULL otherwise
void *mlist_pop(mlist_t *self)
{
    void *retval=NULL;
    if (NULL!=self && NULL!=self->head && self->size>0) {
        
        retval = self->head->data;
        
        if (self->head==self->tail) {
            // deleting only item
            self->tail=NULL;
            self->cursor=NULL;
            free(self->head);
            self->head=NULL;
        }else{
            self->cursor = (self->cursor==self->head?self->head->next:self->cursor);
            mlist_item_t *pdel = self->head;
        	self->head = self->head->next;
            free(pdel);
        }
        
        self->size--;
    }
    return retval;
}
// End function mlist_pop


/// @fn void * mlist_item(mlist_t * self, void * item)
/// @brief return mlist item data (by reference).
/// @param[in] self mlist reference
/// @param[in] item item reference
/// @return item reference data on success, NULL otherwise
void *mlist_item(mlist_t *self, void *item)
{
    void *retval=NULL;
    if (NULL!=self && NULL!=item) {
        // check whether list contains item
        mlist_item_t *plist=self->head;
        while (plist!=NULL) {
            if (plist->data == item) {
                retval=plist->data;
                break;
            }
            plist=plist->next;
        }
    }
    return retval;
}
// End function mlist_item


/// @fn void * mlist_vlookup(mlist_t * self, void * value, mlist_ival_fn vcompare)
/// @brief return first list item with specified value.
/// @param[in] self mlist reference
/// @param[in] value pointer to value (void)
/// @param[in] vcompare comparison function (application-specific)
/// @return reference to item data with matching value on success, NULL otherwise
void *mlist_vlookup(mlist_t *self, void *value, mlist_ival_fn vcompare)
{
    void *retval=NULL;
    if (NULL!=self && NULL!=vcompare) {
        // check whether list contains item
        mlist_item_t *plist=self->head;
//        MDEBUG("plist[%p] self[%p] head[%p] vc[%p] v[%p]\n",plist,self,(self!=NULL?self->head:NULL),vcompare,value);
        while (NULL!=plist) {
            if (vcompare(plist->data,value)) {
                retval=plist->data;
                break;
            }
            plist=plist->next;
        }
    }
    return retval;
}
// End function mlist_vlookup


/// @fn void mlist_sort(mlist_t * self, mlist_cmp_fn compare)
/// @brief sort mlist. Uses quick sort implementation - may not be stable.
/// @param[in] self mlist reference
/// @param[in] compare compare function (returns true if items a,b are in desired sort order)
/// @return none
void mlist_sort(mlist_t *self, mlist_cmp_fn compare)
{
    if (NULL!=self && NULL!=compare && self->size>1) {
        self->head = s_mlist_rsort(self->head,self->tail,compare);
        self->tail=self->head;
        while (self->tail->next != NULL)
            self->tail = self->tail->next;
        self->cursor=self->head;
    }
}
// End function mlist_sort


/// @fn void mlist_purge(mlist_t * self)
/// @brief remove all items from list. optionally release item resources
/// if autofree or per-item free functions are set.
/// @param[in] self mlist reference
/// @return none
void mlist_purge(mlist_t *self)
{
    if (NULL!=self) {
        // check whether list contains item
        mlist_item_t *plist=self->head;
        while (plist!=NULL) {
            if (plist->free_fn!=NULL) {
                plist->free_fn(plist->data);
            }else if(self->afree_fn!=NULL){
                self->afree_fn(plist->data);
            }
            self->tail = plist;
            plist=plist->next;
            free(self->tail);
        }
        self->head=NULL;
        self->tail=NULL;
        self->cursor=NULL;
    }
}
// End function mlist_purge


/// @fn void mlist_autofree(mlist_t * self, mlist_free_fn fn)
/// @brief set mlist autofree function. If set, will be used to release
/// item resources that do not have a per-item free function set.
/// @param[in] self mlist reference
/// @param[in] fn free function
/// @return none
void mlist_autofree(mlist_t *self, mlist_free_fn fn)
{
    if (NULL!=self) {
        self->afree_fn = fn;
    }
}
// End function mlist_autofree


/// @fn void mlist_freefn(mlist_t * self, void * item, mlist_free_fn fn)
/// @brief set per-item free function function. If set, will be used to release
/// item resources. Overrides autofree function.
/// @param[in] self mlist reference
/// @param[in] fn free function
/// @return none
void mlist_freefn(mlist_t *self, void *item, mlist_free_fn fn)
{
    if (NULL!=self && NULL!=item) {
        // check whether list contains item
        mlist_item_t *plist=self->head;
        while (plist!=NULL) {
            if (plist->data == item) {
                plist->free_fn=fn;
                break;
            }
            plist=plist->next;
        }
    }
}
// End function mlist_freefn


/// @fn size_t mlist_size(mlist_t * self)
/// @brief return number of list items.
/// @param[in] self mlist reference
/// @return number of list items
size_t mlist_size(mlist_t *self)
{
    size_t retval=0;
    if (NULL!=self) {
        retval=self->size;
    }
    return retval;
}
// End function mlist_size


/// @fn _Bool s_testcmp(void * a, void * b)
/// @brief unit test compare function (for sorting).
/// @param[in] a element a reference (string)
/// @param[in] b element b reference (string)
/// @return true if a, b are in correct sort order
static bool s_testcmp(void *a, void *b)
{
    char *sa=(char *)a;
    char *sb=(char *)b;
    int cmp=strcmp(sa,sb);
//    MDEBUG("a[%s] vs b[%s] -> [%d]\n",sa,sb,cmp);
    return (cmp<0?true:false);
}
// End function s_testcmp


/// @fn int mlist_test()
/// @brief mlist unit test(s).
/// @return 0 on success, -1 otherwise
int mlist_test()
{
    int retval=-1;
    char *dp=NULL;

    // test new
    mlist_t *list = mlist_new();
    assert(list!=NULL);
    MDEBUG("test new          OK\n");

    // test add
    mlist_add(list,"wine");
    mlist_add(list,"cheese");
    mlist_add(list,"bread");
    MDEBUG("test add          OK\n");

    // test size
    assert(mlist_size(list)==3);

    // test head, tail, last, first
    dp = (char *)mlist_head(list);
    assert(strcmp(dp,"wine")==0);
    dp = (char *)mlist_tail(list);
    assert(strcmp(dp,"bread")==0);
    MDEBUG("test head/tail    OK\n");

    dp = (char *)mlist_first(list);
    assert(strcmp(dp,"wine")==0);
    dp = (char *)mlist_last(list);
    assert(strcmp(dp,"bread")==0);
    MDEBUG("test first/last   OK\n");
    
    
    char *xp=NULL;
    
    // test sort
    mlist_sort(list,s_testcmp);
    MDEBUG("test sort         OK\n");
    
    // test iterator
    xp = (char *)mlist_first(list);
    int i=0;
    while (xp!=NULL) {
        switch (i) {
            case 0:
                assert(strcmp(xp,"bread")==0);
                break;
            case 1:
                assert(strcmp(xp,"cheese")==0);
                break;
            case 2:
                assert(strcmp(xp,"wine")==0);
                break;
            default:
                assert(false);
                break;
        }
        xp=(char *)mlist_next(list);
        i++;
    }
    MDEBUG("test next         OK\n");
    
    // test pop
    xp=(char *)mlist_pop(list);
//    MDEBUG("pop :%s\n",xp);
    assert(strcmp(xp,"bread")==0);
    xp=(char *)mlist_pop(list);
//    MDEBUG("pop :%s\n",xp);
    assert(strcmp(xp,"cheese")==0);
    xp=(char *)mlist_pop(list);
//    MDEBUG("pop :%s\n",xp);
    assert(strcmp(xp,"wine")==0);
    assert(mlist_size(list)==0);
    MDEBUG("test pop          OK\n");
    // test push
    // be careful about using constants
    // to look them up, the address pointers must match
    // and they may only be valid within their scope
    mlist_push(list,"this");
    assert(mlist_size(list)==1);
    mlist_push(list,"that");
    assert(mlist_size(list)==2);
    assert(strcmp("this",mlist_item(list,"this"))==0);
    assert(strcmp("that",mlist_item(list,"that"))==0);
    MDEBUG("test push         OK\n");
    
    // this item is dynamically allocated
    // and a reference is retained.
    // it may be looked up, a free fn may be set
    // and it would be valid outside function scope.
    dp = strdup("other");
    mlist_push(list,dp);
    
    // test freefn
    mlist_freefn(list,dp, free);
    assert(mlist_size(list)==3);
    MDEBUG("test freefn       OK\n");
   
    // test item lookup
    assert(strcmp((char *)mlist_item(list,dp),dp)==0);
    MDEBUG("test item         OK\n");

    // release resources
    mlist_destroy(&list);

    retval = 0;
    return retval;
}
// End function mlist_test



