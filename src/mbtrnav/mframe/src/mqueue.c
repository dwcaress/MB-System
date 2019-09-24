///
/// @file mqueue.c
/// @authors k. Headley
/// @date 06 nov 2012
/// 
/// mframe queue (linked list) macro API
/// These are fast, but a little cumbersome to use.
/// For most applications, use mlist and mcbuf

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

/////////////////////////
// Headers 
/////////////////////////

#include "mqueue.h"
#include "mmem.h"

/////////////////////////
// Macros
/////////////////////////

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

/// @fn mq_tqueue_t *mqtq_new()
/// @brief read create new tail queue
/// @return new tail queue on success, NULL otherwise
mq_tqueue_t *mqtq_new()
{
	mq_tqueue_t *newq=(mq_tqueue_t *)malloc(sizeof(mq_tqueue_t));
	newq->free_fn=NULL;
	mq_tnqinit(newq->head);
	return newq;
}

/// @fn mq_tqueue_t *mqtq_xnew()
/// @brief read create new reference-counted tail queue
/// @return new tail queue on success, NULL otherwise
mq_tqueue_t *mqtq_xnew()
{
	mq_tqueue_t *newq=(mq_tqueue_t *)mm_alloc(sizeof(mq_tqueue_t));
	newq->free_fn=NULL;
	mq_tnqinit(newq->head);
	return newq;
}

#if !defined(__CYGWIN__)
/// @fn mq_cqueue_t *mqcq_new()
/// @brief read create new circular queue
/// @return new circular queue on success, NULL otherwise
mq_cqueue_t *mqcq_new()
{
	mq_cqueue_t *newq=(mq_cqueue_t *)malloc(sizeof(mq_cqueue_t));
	newq->free_fn=NULL;
	mq_cnqinit(newq->head);
	return newq;
}

/// @fn mq_cqueue_t *mqcq_xnew()
/// @brief read create new reference-counted circular queue
/// @return new circular queue on success, NULL otherwise
mq_cqueue_t *mqcq_xnew()
{
	mq_cqueue_t *newq=(mq_cqueue_t *)mm_alloc(sizeof(mq_cqueue_t));
	newq->free_fn=NULL;
	mq_cnqinit(newq->head);
	return newq;
}
#endif