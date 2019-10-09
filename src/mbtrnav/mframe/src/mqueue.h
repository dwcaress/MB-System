///
/// @file mqueue.h
/// @authors k. headley
/// @date 06 nov 2012
/// 
/// Wrappers for queue and list functions in sys/queue.h.
/// These are fast, but a little cumbersome to use.
/// For most applications, use mlist and mcbuf

/// Defines a naming convention used to
/// eliminate some of the arguments needed by the queue.h macros:
/// - All list/queue heads are named "queue"
/// - All element lists are named "entries"
/// - Head struct type is <enclosing struct type>_qhead
/// 
/// Typical pattern of use:
/// 
/// ////////////////////
/// A generic list
/// ////////////////////
/// 
/// // a data structure
/// // (we'll want a list of these)
/// struct my_thing{
/// int number;
/// char *name;
/// };
/// 
/// // API for dynamically creating/freeing 
/// // my_thing structs
/// struct my_thing *thing_new(int num, char *name); 
/// void thing_destroy(void *self);
///
/// // create a list, here a cqueue (CIRCLEQ)
/// mq_cqueue_t *thing_list=(mq_cqueue_t *)malloc(sizeof(mq_cqueue_t));
/// // a pointer to cqueue list elements
/// xcq_entry_t *cqe=NULL;
/// 
/// // a pointer to list element type
/// struct my_thing *tptr;
/// 
/// // initialize the list
/// mq_cnqinit(thing_list->head);
/// 
/// // set the free function
/// thing_list->free_fn=thing_destroy;
/// 
/// // dynamically populate the list
/// printf("CQ: adding elements:\n");
/// for (i=0;i<10; i++) {
///  // create the new my_thing
///  tptr=thing_new((i*i+1),"thing");
///  // create/add list entry
///  mq_cnqadd(thing_list->head,tptr);
/// 
///  printf("CQ: adding element = %2d:%s\n",tptr->number,tptr->name);
/// }
///
/// // iterate over the list
/// i=0;
/// printf("CQ: listing elements:\n");
/// mq_cnqiterate(thing_list->head,cqe){
///   tptr=(struct my_thing *)(cqe->item);
///   printf("CQ: item[%d]=%d:%s\n",i++,tptr->number,tptr->name);
/// }
/// 
/// // delete the list and use free_fn to delete all elements
/// mq_cnqdestroy(thing_list);
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

// Always do this
#ifndef MQ_QUEUE_H
#define MQ_QUEUE_H

/////////////////////////
// Includes 
/////////////////////////
#include "mframe.h"

/////////////////////////
// Type Definitions
/////////////////////////
#ifndef MQ_QUEUE_H_SHARED
#define MQ_QUEUE_H_SHARED

/// @typedef struct mq_tqueue_s mq_tqueue_t
/// @brief tail queue type
typedef struct mq_tqueue_s mq_tqueue_t;
/// @typedef struct xtq_entry xtq_entry_t;
/// @brief tail queue ebtry type
typedef struct xtq_entry xtq_entry_t;
/// @typedef struct mq_cqueue_s mq_cqueue_t
/// @brief circle queue type
typedef struct mq_cqueue_s mq_cqueue_t;
/// @typedef struct xcq_entry xcq_entry_t
/// @brief circle queue entry type
typedef struct xcq_entry xcq_entry_t;
#endif

/// @typedef mq_qfree_fn
/// @brief queue free function type
typedef void (* mq_qfree_fn)(void *ptr);

// struct xtqueue definition
TAILQ_HEAD(mq_tqhead,xtq_entry);

/// @struct mq_tqueue_s
/// @brief tail queue
struct mq_tqueue_s{
    /// @var mq_tqueue_s::head
    /// @brief cqueue head
	struct mq_tqhead head;
    /// @var mq_tqueue_s::free_fn
    /// @brief free function
	mq_qfree_fn free_fn;
};

/// @struct xtq_entry
/// @brief tail queue entry
struct xtq_entry {
	TAILQ_ENTRY(xtq_entry)entries;
    /// @var xtq_entry::item
    /// @brief item pointer
	void *item;
};

#if !defined(__CYGWIN__)
// struct xcqueue definition
CIRCLEQ_HEAD(mq_cqhead,xcq_entry);

/// @struct mq_cqueue_s
/// @brief circle queue
struct mq_cqueue_s{
    /// @var mq_cqueue_s::head
    /// @brief cqueue head
	struct mq_cqhead head;
    /// @var mq_cqueue_s::free_fn
    /// @brief free function
	mq_qfree_fn free_fn;
};

/// @struct xcq_entry
/// @brief circle queue entry
struct xcq_entry {
    
	CIRCLEQ_ENTRY(xcq_entry)entries;
    /// @var xcq_entry::item
    /// @brief item pointer
	void *item;
};
#endif

/////////////////////////
// Macros
/////////////////////////

#if !defined(__CYGWIN__)
/// @def mq_cqinit(parent)
/// @brief initialize queue
/// @param[in] parent TBD
/// @return none
#define mq_cqinit(parent)        CIRCLEQ_INIT(&(parent)->head)
/// @def mq_cqinit(parent)
/// @brief check whether empty
/// @param[in] parent TBD
/// @return true if empty, NULL otherwise
#define mq_cqempty(parent)       CIRCLEQ_EMPTY(&(parent)->head)
/// @def mq_cqremove(parent,var)
/// @brief remove item
/// @param[in] parent TBD
/// @param[in] var TBD
/// @return none
#define mq_cqremove(parent,var)  CIRCLEQ_REMOVE(&(parent)->head,var,entries)
/// @def mq_cqput(parent,var)
/// @brief append item
/// @param[in] parent TBD
/// @param[in] var TBD
/// @return none
#define mq_cqput(parent,var)     CIRCLEQ_INSERT_TAIL(&(parent)->head,var,entries)
/// @def mq_cqget(parent)
/// @brief append item at HEAD
/// @param[in] parent TBD
/// @return none
#define mq_cqget(parent)         CIRCLEQ_FIRST(&(parent)->head)
/// @def mq_cqfirst(parent)
/// @brief get item at HEAD
/// @param[in] parent TBD
/// @return item pointer
#define mq_cqfirst(parent)       CIRCLEQ_FIRST(&(parent)->head)
/// @def mq_cqlast(parent)
/// @brief get item at HEAD
/// @param[in] parent TBD
/// @return item pointer
#define mq_cqlast(parent)        CIRCLEQ_LAST(&(parent)->head)
/// @def mq_cqiterate(parent,var)
/// @brief iterate over list
/// @param[in] parent TBD
/// @param[in] var TBD
/// @return item pointer
#define mq_cqiterate(parent,var) CIRCLEQ_FOREACH(var,&parent->head,entries)

/// @def mq_cnqinit(qname)
/// @brief initialize queue
/// @param[in] qname TBD
/// @return none
#define mq_cnqinit(qname)        CIRCLEQ_INIT(&qname)
/// @def mq_cnqempty(qname)
/// @brief check whether empty
/// @param[in] qname TBD
/// @return true if empty, NULL otherwise
#define mq_cnqempty(qname)       CIRCLEQ_EMPTY(&qname)
/// @def mq_cnqremove(qname,var)
/// @brief remove item
/// @param[in] parent TBD
/// @param[in] var TBD
/// @return none
#define mq_cnqremove(qname,var)  CIRCLEQ_REMOVE(&qname,var,entries)
/// @def mq_cnqput(qname,var)
/// @brief append item
/// @param[in] qname TBD
/// @param[in] var TBD
/// @return none
#define mq_cnqput(qname,var)     CIRCLEQ_INSERT_TAIL(&qname,var,entries)
/// @def mq_cnqget(qname)
/// @brief get item at HEAD
/// @param[in] qname TBD
/// @return item pointer
#define mq_cnqget(qname)         CIRCLEQ_FIRST(&qname)
/// @def mq_cnqfirst(qname)
/// @brief get item at HEAD
/// @param[in] qname TBD
/// @return item pointer
#define mq_cnqfirst(qname)       CIRCLEQ_FIRST(&qname)
/// @def mq_cnqlast(qname)
/// @brief get item at HEAD
/// @param[in] qname TBD
/// @return item pointer
#define mq_cnqlast(qname)        CIRCLEQ_LAST(&qname)
/// @def mq_cnqiterate(qname,var)
/// @brief iterate over list
/// @param[in] qname TBD
/// @param[in] var TBD
/// @return none
#define mq_cnqiterate(qname,var) CIRCLEQ_FOREACH(var,&qname,entries)
/// @def mq_cnqchead(qname,type)
/// @brief return first item (typecast)
/// @param[in] qname TBD
/// @param[in] type TBD
/// @return item pointer
#define mq_cnqchead(qname,type)   ((type *)(mq_cnqfirst(qname))->item)
/// @def mq_cnqchead(qname,type)
/// @brief return last item (typecast)
/// @param[in] qname TBD
/// @param[in] type TBD
/// @return item pointer
#define mq_cnqctail(qname,type)   ((type *)(mq_cnqlast(qname))->item)

/// @def mq_cqadd(parent,var)
/// @brief add item
/// @param[in] parent TBD
/// @param[in] var TBD
/// @return none
#define mq_cqadd(parent,var)   do{ \
xcq_entry_t *cqe=NULL; \
cqe=(xcq_entry_t *)malloc(sizeof(xcq_entry_t)); \
cqe->item = var; \
mq_cqput(parent,cqe); \
}while(0)

/// @def mq_cqnadd(qname,var)
/// @brief add item
/// @param[in] qname TBD
/// @param[in] var TBD
/// @return none
#define mq_cnqadd(qname,var)   do{ \
xcq_entry_t *cqe=NULL; \
cqe=(xcq_entry_t *)malloc(sizeof(xcq_entry_t)); \
cqe->item = var; \
mq_cnqput(qname,cqe); \
}while(0)

/// @def mq_cnqdelete(parent,var)
/// @brief remove item
/// @param[in] qname TBD
/// @param[in] var TBD
/// @return none
#define mq_cnqdelete(parent,var)   do{ \
if(parent!=NULL){ \
mq_cnqremove(parent->head,var); \
if(parent->free_fn !=NULL) \
parent->free_fn(var->item); \
free(var); \
} \
}while(0)

/// @def mq_cnqdestroy(parent)
/// @brief release list resources
/// @param[in] qname TBD
/// @return none
#define mq_cnqdestroy(parent)   do{ \
if(parent!=NULL){ \
xcq_entry_t *cqe=NULL; \
while (!mq_cnqempty(parent->head)) { \
cqe = mq_cnqlast(parent->head); \
mq_cnqdelete(parent,cqe); \
} \
free(parent); \
} \
}while(0)

/// @def mqcq_release(qname)
/// @brief release list resources
/// @param[in] qname TBD
/// @return none
#define mqcq_release(qname) mq_release(qname)

#endif

/// @def mq_tqinit(parent)
/// @brief initialize queue
/// @param[in] parent TBD
/// @return none
#define mq_tqinit(parent)        TAILQ_INIT(&parent->queue)
/// @def mq_tqappend(parent,var)
/// @brief append item
/// @param[in] parent TBD
/// @param[in] var TBD
/// @return none
#define mq_tqappend(parent,var)  TAILQ_INSERT_TAIL(&parent->head,var,entries)
/// @def mq_tqiterate(parent,var)
/// @brief iterate over list
/// @param[in] parent TBD
/// @param[in] var TBD
/// @return item pointer
#define mq_tqiterate(parent,var) TAILQ_FOREACH(var,&parent->head,entries)
/// @def mq_tqriterate(parent,var)
/// @brief iterate over list in reverse
/// @param[in] parent TBD
/// @param[in] var TBD
/// @param[in] htype TBD
/// @return item pointer
#define mq_tqriterate(parent,var,htype) TAILQ_FOREACH_REVERSE(var,&parent->head,htype,entries)
/// @def mq_tqremove(parent,var)
/// @brief remove item
/// @param[in] parent TBD
/// @param[in] var TBD
/// @return item pointer
#define mq_tqremove(parent,var)  TAILQ_REMOVE(&parent->head,var,entries)
/// @def mq_tqfirst(parent)
/// @brief get item at HEAD
/// @param[in] parent TBD
/// @return item pointer
#define mq_tqfirst(parent)       TAILQ_FIRST(&parent->head)
/// @def mq_tqlast(parent)
/// @brief get item at HEAD
/// @param[in] parent TBD
/// @param[in] hname TBD
/// @return item pointer
#define mq_tqlast(parent,hname)  TAILQ_LAST(&parent->head,hname)
/// @def mq_tqempty(parent)
/// @brief check whether empty
/// @param[in] parent TBD
/// @return true if empty, NULL otherwise
#define mq_tqempty(parent)       TAILQ_EMPTY(&parent->head)


/// @def mq_tnqinit(qname)
/// @brief initialize queue
/// @param[in] qname TBD
/// @return none
#define mq_tnqinit(qname)         TAILQ_INIT(&qname)
/// @def mq_tnqappend(qname,var)
/// @brief append item
/// @param[in] qname TBD
/// @param[in] var TBD
/// @return none
#define mq_tnqappend(qname,var)   TAILQ_INSERT_TAIL(&qname,var,entries)
/// @def mq_tnqinshead(qname,var)
/// @brief insert item at HEAD
/// @param[in] qname TBD
/// @param[in] var TBD
/// @return none
#define mq_tnqinshead(qname,var)  TAILQ_INSERT_HEAD(&qname,var,entries)
/// @def mq_tnqiterate(qname,var)
/// @brief iterate over list
/// @param[in] qname TBD
/// @param[in] var TBD
/// @return none
#define mq_tnqiterate(qname,var)  TAILQ_FOREACH(var,&qname,entries)
/// @def mq_tnqriterate(qname,var)
/// @brief iterate over list
/// @param[in] qname TBD
/// @param[in] var TBD
/// @param[in] htype TBD
/// @return none
#define mq_tnqriterate(qname,var,htype) TAILQ_FOREACH_REVERSE(var,&qname,htype,entries)
/// @def mq_tnqremove(qname,var)
/// @brief remove item
/// @param[in] qname TBD
/// @param[in] var TBD
/// @return none
#define mq_tnqremove(qname,var)   TAILQ_REMOVE(&qname,var,entries)
/// @def mq_tnqfirst(qname)
/// @brief get item at HEAD
/// @param[in] qname TBD
/// @return item pointer
#define mq_tnqfirst(qname)        TAILQ_FIRST(&qname)
/// @def mq_tnqlast(qname)
/// @brief get item at HEAD
/// @param[in] qname TBD
/// @return item pointer
#define mq_tnqlast(qname,hname)   TAILQ_LAST(&qname,hname)
/// @def mq_tnqempty(qname)
/// @brief check whether empty
/// @param[in] qname TBD
/// @return true if empty, NULL otherwise
#define mq_tnqempty(qname)        TAILQ_EMPTY(&qname)
/// @def mq_tnqchead(qname,type)
/// @brief return first item (typecast)
/// @param[in] parent TBD
/// @param[in] type TBD
/// @return item pointer
#define mq_tnqchead(qname,type)   ((type *)(mq_tnqfirst(qname))->item)
/// @def mq_tnqctail(qname,type)
/// @brief return last item (typecast)
/// @param[in] parent TBD
/// @param[in] type TBD
/// @return item pointer
#define mq_tnqctail(qname,type)   ((type *)(mq_tnqlast(qname))->item)
/// @def mq_tqadd(parent,var)
/// @brief add item
/// @param[in] parent TBD
/// @param[in] var TBD
/// @return none
#define mq_tqadd(parent,var)   do{ \
xtq_entry_t *tqe=NULL; \
tqe=(xtq_entry_t *)malloc(sizeof(xtq_entry_t)); \
tqe->item = var; \
mq_tqappend(parent,tqe); \
}while(0)

/// @def mq_tnqadd(qname,var)
/// @brief add item
/// @param[in] qname TBD
/// @param[in] var TBD
/// @return none
#define mq_tnqadd(qname,var)   do{ \
xtq_entry_t *tqe=NULL; \
tqe=(xtq_entry_t *)malloc(sizeof(xtq_entry_t)); \
tqe->item = var; \
mq_tnqappend(qname,tqe); \
}while(0)

/// @def mq_tnqpush(qname,var)
/// @brief add item at HEAD
/// @param[in] qname TBD
/// @param[in] var TBD
/// @return none
#define mq_tnqpush(qname,var)   do{ \
xtq_entry_t *tqe=NULL; \
tqe=(xtq_entry_t *)malloc(sizeof(xtq_entry_t)); \
tqe->item = var; \
mq_tnqinshead(qname,tqe); \
}while(0)

/// @def mq_tnqdelete(parent,var)
/// @brief remove item
/// @param[in] parent TBD
/// @param[in] var TBD
/// @return none
#define mq_tnqdelete(parent,var)   do{ \
if(parent!=NULL){ \
mq_tnqremove(parent->head,var); \
if(parent->free_fn !=NULL){ \
parent->free_fn(var->item); \
} \
free(var); \
} \
}while(0)

/// @def mq_tnqdestroy(parent)
/// @brief release list resources
/// @param[in] parent TBD
/// @return none
#define mq_tnqdestroy(parent)   do{ \
if(parent!=NULL){ \
	xtq_entry_t *tqe=NULL; \
	while (!mq_tnqempty(parent->head)) { \
		tqe = mq_tnqlast(parent->head,mq_tqhead); \
        mq_tnqdelete(parent,tqe); \
	} \
	free(parent); \
} \
}while(0)


/// @def mqtq_release(qname)
/// @brief release list resources
/// @param[in] qname TBD
/// @return none
#define mqtq_release(qname) mq_release(qname)

/////////////////////////
// Exports
/////////////////////////

mq_tqueue_t *mqtq_new();	
mq_tqueue_t *mqtq_xnew();
#if !defined(__CYGWIN__)
mq_cqueue_t *mqcq_new();
mq_cqueue_t *mqcq_xnew();
#endif

#endif // MQ_QUEUE_H
