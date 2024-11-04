///
/// @file mmem.c
/// @authors k. Headley
/// @date 06 nov 2012
/// 
/// mframe reference counting memory allocation API.

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
#include "mmem.h"

/////////////////////////
// Macros
/////////////////////////

/////////////////////////
// Declarations 
/////////////////////////
/// @typedef mem_object_t
/// @brief tbd
/// @struct mem_object_s
/// @brief tbd
struct mem_object_s {
    /// @var mem_object_s::sig
    /// @brief tbd
    uint16_t sig;
    
    /// @var mem_object_s::retain_count
    /// @brief tbd
    uint32_t retain_count;
    
    /// @var mem_object_s::size
    /// @brief tbd
    size_t size;
    
    /// @var mem_object_s::data
    /// @brief tbd
    void * data;
};

struct mem_info_s{
    long unsigned int obj_count;
    long unsigned int ref_count;
    size_t alloc_bytes;
};


/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////
static mem_info_t global_mem_info;
static pthread_mutex_t  mem_info_mutex = PTHREAD_MUTEX_INITIALIZER;

/////////////////////////
// Function Definitions
/////////////////////////

#if defined(__QNX__)
#pragma message("WARNING - MMEM API not implemented for QNX")
void *mm_alloc(size_t size)
{return NULL;}
void *mm_realloc(void *mem, size_t size)
{return NULL;}
void mm_retain(void *mem)
{return;}
void mm_release(void *mem)
{return;}
int mm_refcount(void *ptr)
{return -1;}
/// @fn mem_object_t *get_memory_obj(void *mem)
/// @brief tbd
/// @param[in] ptr description
/// @return tbd
mem_object_t *get_memory_obj(void *mem)
{return NULL;}
void mm_mem_stats(mem_info_t *minfo)
{return;}
void show_mem_obj(mem_object_t *pmo)
{return;}
void show_mem_stats()
{return;}

#else // !__QNX__


void
mm_mem_stats(mem_info_t *minfo)
{
	if (minfo==NULL) {
		return;
	}
	pthread_mutex_lock(&mem_info_mutex);
	memcpy(minfo,&global_mem_info,sizeof (mem_info_t));
	pthread_mutex_unlock(&mem_info_mutex);
	return;
}

/// @fn void print_mem_obj(mem_object_t *pmo)
/// @brief tbd
/// @param[in] pmo description
/// @return tbd
void 
show_mem_obj(mem_object_t *pmo)
{
	//printf("rcount:%d data:0x%08X sz:%u\n",pmo->retain_count,(unsigned int)pmo->data,(unsigned)pmo->size);
	if (pmo!=NULL) {
		//printf("rcount:%d sig:0x%4X data:0x%08X\n",pmo->retain_count,(uint16_t)pmo->sig,(unsigned int)pmo->data );
		printf("rcount:%u sig:0x%4X data:%p\n",pmo->retain_count,(uint16_t)pmo->sig,pmo->data );
	}
}

void 
show_mem_stats()
{
	//printf("rcount:%d data:0x%08X sz:%u\n",pmo->retain_count,(unsigned int)pmo->data,(unsigned)pmo->size);
	fprintf(stdout,"%s: MEMSTAT objects:%lu references:%lu allocated:%zu\n",__FUNCTION__,global_mem_info.obj_count,global_mem_info.ref_count,global_mem_info.alloc_bytes );
}

mem_object_t *
get_memory_obj(void * ptr)
{
	mem_object_t *o=NULL;
	char *cptr=NULL;
	
	if (ptr==NULL) {
		return NULL;
	}
	
	cptr = ( char * )ptr;
	cptr -= sizeof( mem_object_t );
	o = ( mem_object_t * )cptr;
	return o;
}

/// @fn void *alloc( size_t size )
/// @brief tbd
/// @param[in] size description
/// @return tbd
void *
mm_alloc( size_t size )
{
	mem_object_t *o=NULL;
	char *ptr=NULL;
	//show_mem_stats();
	pthread_mutex_lock(&mem_info_mutex);

	size_t alloc_size=sizeof( mem_object_t ) + size;
	o = ( mem_object_t * )calloc( alloc_size, 1 );
	ptr = ( char * )o;
	ptr += sizeof( mem_object_t );
	o->retain_count = 1;
	o->size=size;
	o->sig=REFCOUNT_VALID;
	o->data = ptr;
	memset(ptr,0,size);
	
	global_mem_info.obj_count++;
	global_mem_info.ref_count++;
	global_mem_info.alloc_bytes+=size;
	
	pthread_mutex_unlock(&mem_info_mutex);
	//show_mem_obj(o);
	return ( void * )ptr;
} 

void *
mm_realloc(void *mem, size_t size)
{
	mem_object_t *o=NULL,*old=NULL,*p=NULL;
	char *ptr=NULL;
	pthread_mutex_lock(&mem_info_mutex);
	
	p=( mem_object_t * )((mem_object_t *)mem-sizeof(mem_object_t));
	old=p;
	
	size_t alloc_size = sizeof( mem_object_t ) + size;
	
	o = ( mem_object_t * )realloc( p, alloc_size);
	ptr = ( char * )o;
	ptr += sizeof( mem_object_t );
	
	// no need to change refcount, signature
	//o->retain_count = 1;
	//o->sig=REFCOUNT_VALID;
	
	// update size and pointer
	o->size=size;
	o->data = ptr;
	// initialize new memory to zero
	// if larger than before
	if (size > old->size) {
		memset((ptr+(old->size)), 0, (size - old->size));
	}
	
	//global_mem_info.obj_count++;
	//global_mem_info.ref_count++;
	global_mem_info.alloc_bytes += (size - old->size);
	
	pthread_mutex_unlock(&mem_info_mutex);
	return (void *)ptr;
}

/// @fn void retain( void * ptr )
/// @brief tbd
/// @param[in] ptr description
/// @return tbd
void 
mm_retain( void * ptr )
{
	mem_object_t *o=NULL;
	//fprintf(stderr,"%s:%d - retain called [%p]\n",__FUNCTION__,__LINE__,ptr);
	if (ptr!=NULL) {
		pthread_mutex_lock(&mem_info_mutex);
		// point to reference count info
		o=get_memory_obj(ptr);
		if (o->sig == REFCOUNT_VALID) {
			// increment the retain count
			o->retain_count++;
			
			global_mem_info.ref_count++;
		}else {
			fprintf(stderr,"%s:%d - warning: attempting to retain unmanaged memory [%p]\n",__FUNCTION__,__LINE__,ptr);
		}
		pthread_mutex_unlock(&mem_info_mutex);
	}else {
		fprintf(stderr,"%s:%d - warning: pointer is NULL\n",__FUNCTION__,__LINE__);
	}

}

/// @fn void release( void * ptr )
/// @brief tbd
/// @param[in] ptr description
/// @return tbd
void 
mm_release( void * ptr )
{
	mem_object_t *o=NULL;
	//fprintf(stderr,"%s:%d - release called [%p]\n",__FUNCTION__,__LINE__,ptr);
	if (ptr!=NULL) {
		pthread_mutex_lock(&mem_info_mutex);
		
		// point to reference count info
		o=get_memory_obj(ptr);

		// only decrement reference count
		// if it is greater than zero
		if (o->sig != REFCOUNT_VALID) {
			fprintf(stderr,"%s:%d - warning: attempting to release unmanaged memory [%p]\n",__FUNCTION__,__LINE__,ptr);
		}else if (o->retain_count > 0) {
			o->retain_count--;
			global_mem_info.ref_count--;
			// only free once when 
			// reference count is zero
			if( o->retain_count == 0 ){
				// mark invalid before freeing, so 
				// nothing still pointing to it can
				// change it
				o->sig=REFCOUNT_INVALID;
				global_mem_info.alloc_bytes-=o->size;			
				free( o );
				global_mem_info.obj_count--;
			}
		}else{
			fprintf(stderr,"%s:%d - warning: attempting redundant memory release [%p]\n",__FUNCTION__,__LINE__,ptr);
		}
		
		pthread_mutex_unlock(&mem_info_mutex);
	}else {
		fprintf(stderr,"%s:%d - warning: pointer is NULL\n",__FUNCTION__,__LINE__);
	}
} 

int
mm_refcount(void *ptr)
{
	int rcount=-1;
	mem_object_t *o=NULL;
	pthread_mutex_lock(&mem_info_mutex);
	if (ptr != NULL) {
		o=get_memory_obj(ptr);

		//if ( (o->sig == REFCOUNT_VALID) && 
		//	(o->retain_count > 0) ) {
		if ( o->sig == REFCOUNT_VALID ) {
			rcount = o->retain_count;
		}else{
			fprintf(stderr,"%s:%d - warning: obj marked invalid [%p]\n",__FUNCTION__,__LINE__,ptr);
			rcount = -1;
		}
	}else {
		fprintf(stderr,"%s:%d - warning: pointer is NULL\n",__FUNCTION__,__LINE__);
		rcount = -1;
	}
	pthread_mutex_unlock(&mem_info_mutex);		

	return rcount;
}
#endif //!__QNX__
