///
/// @file mbbuf.c
/// @authors k. Headley
/// @date 06 nov 2012
/// 
/// mframe byte buffer API implementation.
/// Dynamic, reference counted buffers

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

#include "mbbuf.h"
//#include "mmem.h"

/////////////////////////
// Macros
/////////////////////////
// These macros should only be defined for
// application main files rather than general C files
/*
 /// @def PRODUCT
 /// @brief header software product name
 #define PRODUCT "qframe"
 
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

/// @def MB_CONTENT_LEN(buf)
/// @param[in] buf buffer pointer
/// @brief return bytes in buffer
#define MB_CONTENT_LEN(buf) ((off_t)(buf->tail - buf->head))
/// @def MB_AVAILABLE(buf)
/// @brief return avalable space
/// @param[in] buf buffer pointer
#define MB_AVAILABLE(buf) ((off_t)(buf->capacity - (off_t)(buf->tail - buf->head)))
/// @def MB_AVAILABLE_OS(buf,offset)
/// @brief return offset of available space
/// @param[in] buf buffer pointer
/// @param[in] offset
#define MB_AVAILABLE_OS(buf, offset) ((buf->capacity-(off_t)(offset)))
/// @def MB_IS_SPACE(buf,offset,size)
/// @brief return true if buffer is not full
/// @param[in] buf buffer pointer
#define MB_IS_SPACE(buf, offset, size) (((offset+size) <= buf->capacity))
/// @def MB_TOFFSET(buf)
/// @brief get tail offset
/// @param[in] buf buffer pointer
#define MB_TOFFSET(buf) ((off_t)(buf->tail-buf->head))
/// @def MB_ICOFFSET(buf)
/// @brief get input cursor offset
/// @param[in] buf buffer pointer
#define MB_ICOFFSET(buf) ((off_t)(buf->icursor-buf->head))
/// @def MB_OCOFFSET(buf)
/// @brief get output cursor offset
/// @param[in] buf buffer pointer
#define MB_OCOFFSET(buf) ((off_t)(buf->ocursor-buf->head))

/////////////////////////
// Declarations 
/////////////////////////

/// @struct mbbuf_s
/// @brief dynamic buffer structure
struct mbbuf_s{
	/// @var mbbuf_s::capacity
	/// @brief TBD
	off_t capacity;
	/// @var mbbuf_s::head
	/// @brief TBD
	byte *head;
	/// @var mbbuf_s::tail
	/// @brief TBD
	byte *tail;
	/// @var mbbuf_s::icursor
	/// @brief TBD
	byte *icursor;
	/// @var mbbuf_s::ocursor
	/// @brief TBD
	byte *ocursor;
};

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////

/////////////////////////
// Function Definitions
/////////////////////////

/// @fn mbbuf_t *mbb_new(off_t capacity, byte *data, off_t size)
/// @brief allocate new mbbuf_t.
/// @param[in] capacity initial capacity
/// @param[in] data initial contents
/// @param[in] size number of data bytes
/// @return new mbbuf_t reference on success, NULL otherwise
mbbuf_t *mbb_new(off_t capacity, byte *data, off_t size)
{
		if (size>capacity || capacity<0 || size<0) {
		return NULL;
	}
	{
	mbbuf_t *new_obj=(mbbuf_t *)malloc(sizeof(mbbuf_t));
	new_obj->capacity=capacity;
	
	new_obj->head=(byte *)malloc((capacity+1)*sizeof(byte));
	
	if (data!=NULL) {
		memcpy(new_obj->head, data, size);
		new_obj->tail = new_obj->head+size;
		memset(new_obj->tail,'\0', MB_AVAILABLE(new_obj));
	}else {
		memset(new_obj->head,'\0',capacity);
		new_obj->tail = new_obj->head;
	}
	new_obj->icursor = new_obj->head;
	new_obj->ocursor = new_obj->head;
	return new_obj;
    }
}

/// @fn void mbb_destroy(mbbuf_t ** p_self)
/// @brief release buffer resources.
/// @param[in] p_self pointer to buffer reference
/// @return none
void mbb_destroy(mbbuf_t **p_self)
{
	if (p_self) {
		mbbuf_t *self = (*p_self);
		if (self) {
			if (self->head) {
				//printf("%s:%d - got here\n",__FUNCTION__,__LINE__);
				free(self->head);
			}
			//printf("%s:%d - got here\n",__FUNCTION__,__LINE__);
			free(self);
		}
	}
}
// End function mbb_destroy

/// @fn void mbb_free(void * self)
/// @brief release buffer resources.
/// @param[in] self buffer reference
/// @return none
void mbb_free(void *self)
{
	mbb_destroy((mbbuf_t **)&self);
}
// End function mbb_free

/// @fn int grow_buf(mbbuf_t * self, off_t new_size)
/// @brief increase buffer to new size.
/// @param[in] self buffer reference
/// @param[in] new_size new capacity
/// @return 0 on success, -1 otherwise
static int grow_buf(mbbuf_t *self, off_t new_size)
{
	off_t tos = MB_TOFFSET(self);
	off_t ios = MB_ICOFFSET(self);
	off_t oos = MB_OCOFFSET(self);
	off_t capacity=mbb_capacity(self);
	byte *old_head=self->head;

	//printf("%s:%d - head[%p] tail[%p] ios[%ld] oos[%ld] tos[%ld] c[%ld] \n",__FUNCTION__,__LINE__,self->head,self->tail,ios,oos,tos,capacity);

	byte *bp= (byte *)malloc((new_size+1)*sizeof(byte));
	
	if (bp) {
        memset(bp,'\0',new_size+1);
        //printf("%s:%d - bp[%p] head[%p] tail[%p] ios[%ld] oos[%ld] tos[%ld] c[%ld] \n",__FUNCTION__,__LINE__,bp,self->head,self->tail,ios,oos,tos,capacity);
		memcpy(bp,self->head,capacity);
		self->head=bp;
		//printf("%s:%d - bp[%p] head[%p] tail[%p] ios[%ld] oos[%ld] tos[%ld] c[%ld] \n",__FUNCTION__,__LINE__,bp,self->head,self->tail,ios,oos,tos,capacity);

		self->tail=self->head+tos;
		self->icursor=self->head+ios;
		self->ocursor=self->head+oos;
		self->capacity=new_size;
		free(old_head);

		//printf("%s:%d - bp[%p] head[%p] tail[%p] ios[%ld] oos[%ld] tos[%ld] c[%ld] \n",__FUNCTION__,__LINE__,bp,self->head,self->tail,ios,oos,tos,capacity);
		return 0;
	}
	return -1;
}
// End function grow_buf

/// @fn int shrink_buf(mbbuf_t * self, off_t new_size)
/// @brief reduce buffer to new size.
/// @param[in] self buffer reference
/// @param[in] new_size new capacity
/// @return 0 on success, -1 otherwise
static int shrink_buf(mbbuf_t *self, off_t new_size)
{
	off_t tos = MB_TOFFSET(self);
	off_t ios = MB_ICOFFSET(self);
	off_t oos = MB_OCOFFSET(self);
	byte *old_head=self->head;
	byte *bp=(byte *)malloc(((new_size+1)*sizeof(byte)) );
	if (bp) {
        memset(bp,'\0',new_size+1);
		memcpy(bp,self->head,new_size);
		self->head=bp;
		if (tos<new_size) {
			self->tail=self->head+tos;
		}else{
			self->tail=self->head+new_size;
		}
		if (ios<new_size) {
			self->icursor=self->head+ios;
		}else{
			self->icursor=self->head+new_size;
		}
		if (oos<new_size) {
			self->ocursor=self->head+oos;
		}else{
			self->ocursor=self->head+new_size;
		}
		self->capacity=new_size;
		free(old_head);
		return 0;
	}
	return -1;
}
// End function shrink_buf

/// @fn int mbb_set_capacity(mbbuf_t * self, off_t new_size)
/// @brief set buffer size.
/// @param[in] self buffer reference
/// @param[in] new_size new capacity
/// @return 0 on success, -1 otherwise
int mbb_set_capacity(mbbuf_t *self, off_t new_size)
{
	if (self) {
		off_t capacity=mbb_capacity(self);
		
		if (new_size>capacity) {
			if(grow_buf(self,new_size)){
				return -1;
			}
			return 0;
		}
		if (new_size<capacity) {
			if(shrink_buf(self,new_size)){
				return -1;
			}
			return 0;
		}
        // no change
        return 0;
	}
	// error
    return -1;
}
// End function mbb_set_capacity

/// @fn int mbbuf_trim(mbbuf_t * self)
/// @brief trim buffer (remove unused space).
/// @param[in] self buffer reference
/// @return 0 on success, -1 otherwise
int mbbuf_trim(mbbuf_t *self)
{
	return mbb_set_capacity(self, mbb_length(self));
}
// End function mbbuf_trim

/// @fn int s_mbb_adjust_size(mbbuf_t * self, off_t to_add)
/// @brief resize buffer.
/// @param[in] self buffer reference
/// @param[in] offset (unused)
/// @param[in] to_add amount to add
/// @return 0 on success, -1 otherwise
static int s_mbb_adjust_size(mbbuf_t *self, off_t offset, off_t to_add)
{
	off_t available = mbb_available(self);
	
	if (available<to_add) {
		off_t new_size=self->capacity+to_add-available;
		
		if(grow_buf(self,new_size)){
			return -1;
		}
		return 0;
	}
	return 0;
}
// End function s_mbb_adjust_size

/// @fn mbbuf_t * mbb_dup(mbbuf_t * self)
/// @brief copy a buffer.
/// @param[in] self buffer reference
/// @return new mbbuf_t on success, NULL otherwise
mbbuf_t *mbb_dup(mbbuf_t *self)
{
	mbbuf_t *new_obj=NULL;
	if (self) {
		new_obj=mbb_new(self->capacity, self->head, mbb_length(self));
		// set cursor states
		new_obj->tail    = new_obj->head+MB_TOFFSET(self);
		new_obj->icursor = new_obj->head+MB_ICOFFSET(self);
		new_obj->ocursor = new_obj->head+MB_OCOFFSET(self);
	}
	return new_obj;
}
// End function mbb_dup

/// @fn int mbb_pseek(mbbuf_t * self, byte ** cursor, off_t offset)
/// @brief set input or output cursor to specified position.
/// @param[in] self buffer reference
/// @param[in] cursor pointer to cursor
/// @param[in] offset new position
/// @return 0 on success, -1 otherwise
static int mbb_pseek(mbbuf_t *self, byte **cursor, off_t offset)
{
	if (self && (self->capacity>offset) ) {
				
		switch (offset) {
			case MB_SEEK_HEAD:
				*cursor=self->head;
				break;
			case MB_SEEK_TAIL:
				*cursor=self->tail;
				break;
			default:
				*cursor=(self->head+offset);
				break;
		}
		return 0;
	}
	return -1;
}
// End function mbb_pseek


/// @fn int mbb_seek(mbbuf_t * self, off_t offset)
/// @brief set input and output cursors positions.
/// @param[in] self buffer reference
/// @param[in] offset new position
/// @return 0 on success, -1 otherwise
int mbb_seek(mbbuf_t *self, off_t offset)
{
	return ( (mbb_iseek(self,offset)+mbb_oseek(self,offset)) ? -1 :0);
}
// End function mbb_seek

/// @fn int mbb_iseek(mbbuf_t * self, off_t offset)
/// @brief set input (write) cursor position.
/// @param[in] self buffer reference
/// @param[in] offset offset
/// @return input cursor position
int
mbb_iseek(mbbuf_t *self, off_t offset)
{
	return mbb_pseek(self, &self->icursor, offset);
}
// End function mbb_iseek

/// @fn int mbb_oseek(mbbuf_t * self, off_t offset)
/// @brief set output (read) cursor position.
/// @param[in] self buffer reference
/// @param[in] offset offset
/// @return output cursor position
int mbb_oseek(mbbuf_t *self, off_t offset)
{
	return mbb_pseek(self, &self->ocursor, offset);
}
// End function mbb_oseek

/// @fn int mbb_set(mbbuf_t * self, off_t offset, off_t len, byte b)
/// @brief set bytes to value b.
/// @param[in] self buffer reference
/// @param[in] offset start offset
/// @param[in] len number of bytes
/// @param[in] b byte value
/// @return 0 on success, -1 otherwise
int mbb_set(mbbuf_t *self, off_t offset, off_t len, byte b)
{
	if (self) {
		if( (self->capacity<len) && s_mbb_adjust_size(self, offset, len)){
			return -1;
		}
{
		// don't change cursors
		byte *wptr=self->head+offset;

		memset(wptr,b,len);
		// if we've grown beyond the tail
		// move it
		if (self->tail < (self->head+offset+len)) {
			self->tail = self->head+offset+len;
		}
		return 0;
  }
	}

	return -1;
}
// End function mbb_set


/// @fn int mbb_reset(mbbuf_t * self)
/// @brief empty buffer (set all bytes to '\0' and reset cursors.
/// @param[in] self buffer reference
/// @return 0 on success, -1 otherwise
int mbb_reset(mbbuf_t *self)
{
	if (self) {
		int test=mbb_set(self, 0, self->capacity, '\0');
		self->tail=self->head;
		self->icursor=self->head;
		self->ocursor=self->head;
		
		return test;
	}
	return -1;
}
// End function mbb_reset


/// @fn int mbb_append(mbbuf_t * self, byte * data, off_t size)
/// @brief add bytes at end of buffer.
/// @param[in] self buffer reference
/// @param[in] data source data
/// @param[in] size number of bytes to add
/// @return number of bytes added on success, -1 otherwise
int mbb_append(mbbuf_t *self, byte *data, off_t size)
{
	if (self && data && size>0) {
		
		byte *wptr=self->tail;
		if( s_mbb_adjust_size(self, MB_TOFFSET(self), size) ){
			return -1;
		}
		// don't change cursors
		memcpy(wptr, data, size);
		// update pointers
		self->tail+=size;
		// NULL terminate
		*(self->tail)='\0';
		return 0;
	}
	return -1;
}
// End function mbb_append

/// @fn int mbb_push(mbbuf_t * self, byte * data, off_t size)
/// @brief insert bytes at beginning of buffer.
/// @param[in] self buffer reference
/// @param[in] data source data
/// @param[in] size number of bytes
/// @return number of bytes added on success, -1 otherwise
int mbb_push(mbbuf_t *self,byte *data, off_t size)
{
	if (self) {
		// make space as needed
		if( s_mbb_adjust_size(self,MB_TOFFSET(self),size) ){
			return -1;
		}
		// move existing data
		memmove( (self->head+size) ,self->head,size);
		// copy new data to start
		memcpy(self->head, data, size);
		// update pointers
		self->tail+=size;
		self->icursor+=size;
		self->ocursor+=size;
		// NULL terminate
		*(self->tail)='\0';
	}
	return -1;
}
// End function mbb_push

/// @fn byte * mbb_pop(mbbuf_t * self, off_t len)
/// @brief read and remove bytes from beginning of buffer into new byte buffer.
/// (caller must release buffer)
/// @param[in] self buffer reference
/// @param[in] len number of bytes to read
/// @return new byte array on success, NULL otherwise
byte *mbb_pop(mbbuf_t *self, off_t len)
{
	if (self && len<=self->capacity) {

		off_t ios = MB_ICOFFSET(self);
		off_t oos = MB_OCOFFSET(self);

		// allocate memory
		byte *pdat=(byte *)malloc(len*sizeof(byte));
		
		// copy data into it
		memcpy(pdat, self->head, len);
{
		// move remaining data
		off_t rlen=self->tail-(self->head+len);
		memmove( self->head, self->head+len, rlen );
		
		// update pointers
		self->tail=self->head+rlen;
		
		if (MB_ICOFFSET(self) > MB_TOFFSET(self)) {
			self->icursor=self->tail;
		}else{
			self->icursor=self->head+ios;
		}
		if (MB_OCOFFSET(self) > MB_TOFFSET(self)) {
			self->ocursor=self->tail;
		}else{
			self->ocursor=self->head+oos;
		}
		
		// NULL terminate
		*(self->tail)='\0';
		return pdat;
  }
	}
	return NULL;
}
// End function mbb_pop

/// @fn byte * mbb_read(mbbuf_t * self, off_t len)
/// @brief read bytes from buffer into new byte buffer.
/// (Caller must release buffer)
/// @param[in] self buffer reference
/// @param[in] len number of bytes to read
/// @return number of bytes read on success, -1 otherwise
byte * mbb_read(mbbuf_t *self, off_t len)
{
	if (self) {
		// don't read past end of data
		if ( (self->ocursor+len) <= self->tail) {
  {
			// allocate memory
			byte *new_obj=(byte *)malloc((len+1)*sizeof(byte));
			// copy the data
			memcpy(new_obj,self->ocursor,len);
			// null terminate the buffer
			new_obj[len]='\0';

			// adjust the output cursor
			self->ocursor+=len;
			
			return new_obj;
   }
		}
  #ifdef HAS_LONG_LONG
		printf("%s:%d - WARNING: attempt to read past end of data: oc[%p] len[%lld] t[%p]\n",__FUNCTION__, __LINE__,self->ocursor,(long long)len,self->tail);
  #else
        printf("%s:%d - WARNING: attempt to read past end of data: oc[%p] len[%ld] t[%p]\n",__FUNCTION__, __LINE__,self->ocursor,(long)len,self->tail);
  #endif
  
	}
	return NULL;
}
// End function mbb_read

/// @fn int mbb_write(mbbuf_t * self, byte * data, off_t size)
/// @brief write to buffer at input cursor position.
/// @param[in] self buffer reference
/// @param[in] data source buffer
/// @param[in] size number of bytes to write
/// @return number of bytes written on success, -1 otherwise
int mbb_write(mbbuf_t *self, byte *data, off_t size)
{
	if (self) {
		// make space as needed
		if(s_mbb_adjust_size(self,MB_ICOFFSET(self), size)){
			return 0;
		}
		// copy the data
		memcpy(self->icursor, data, size);
		// adjust pointers
		self->icursor+=size;
		if (self->tail<self->icursor) {
			self->tail=self->icursor;
		}
		return size;
	}
    return -1;
}
// End function mbb_write


#if defined(__QNX__)
/// @fn int mbb_printf(mbbuf_t * self, const char * fmt)
/// @brief output buffer contents to console.
/// @param[in] self buffer reference
/// @param[in] fmt printf format
/// @return number of bytes output on success, -1 otherwise
int mbb_printf(mbbuf_t *self, const char *fmt,...)
{
    if (self) {
        va_list va1;
        va_list va2;

        // compute available (relative to icursor)
        off_t available=mbb_iavailable(self);

        int test=-1;
        off_t sz =0;
        off_t adj =0;
        // using NULL/0 for buffer/size arguments
        // returns number of bytes that would be printed (not including NULL).
        // (this vsnprint behavior is C99 and conflicts with other C versions)
        
        // add 1 to size needed for NULL (not counted in vsnprintf return, but we'll need it)
        va_start(va1,fmt);
        sz = vsnprintf(NULL,0,fmt,va1)+1;
        va_end(va1);
        // compute adjustment needed to buffer size
        adj = (sz-available);

        if(adj>0){
            // if more space needed, adjust size
            s_mbb_adjust_size(self,MB_ICOFFSET(self), sz ); //adj
            // recompute available (relative to icursor)
            available=mbb_iavailable(self);
        }

        if(self->icursor > self->head){
            // if not starting from buffer head
            // set the tail pointer...
            self->tail=self->icursor;
            // and back up icursor so that the next input
            // will write over NULL
            self->icursor--;
            // terminate, for good measure
            *(self->tail)='\0';
        }
        // acutally print the data to the buffer
       va_start(va2,fmt);
        test=vsnprintf((char *)self->icursor,available,fmt,va2)+1;
        
        va_end(va2);
        
        // sanity check
        assert((test-1)<=available);

        // adjust pointers
        self->icursor+=test;
        if (self->tail<self->icursor) {
            self->tail=self->icursor;
            *(self->tail)='\0';
        }
        
        return test;
    }
    return -1;
}
// End function mbb_printf
#else
/// @fn int mbb_printf(mbbuf_t * self, const char * fmt)
/// @brief output buffer contents to console.
/// @param[in] self buffer reference
/// @param[in] fmt printf format
/// @return number of bytes output on success, -1 otherwise
int mbb_printf(mbbuf_t *self, const char *fmt,...)
{
    if (self) {
        
        // compute available (relative to icursor)
        off_t available=mbb_iavailable(self);

        int test=-1;
        va_list ap;
        va_start(ap,fmt);
        
        // using NULL/0 for buffer/size arguments
        // returns number of bytes that would be printed (not including NULL).
        // (this vsnprint behavior is C99 and conflicts with other C versions)
        
        // add 1 to size needed for NULL (not counted in vsnprintf return, but we'll need it)
        va_list ap_copy;
        va_copy(ap_copy, ap);
        off_t sz = vsnprintf(NULL,0,fmt,ap_copy)+1;
        // comment out this va_end if it causes an issue
        // (added per cppcheck)
        va_end(ap_copy);
        // compute adjustment needed to buffer size
        off_t adj = (sz-available);

        if(adj>0){
            // if more space needed, adjust size
            s_mbb_adjust_size(self,MB_ICOFFSET(self), sz );
            // recompute available (relative to icursor)
            available=mbb_iavailable(self);
        }

        if(self->icursor > self->head){
            // if not starting from buffer head
            // set the tail pointer...
            self->tail=self->icursor;
            // and back up icursor so that the next input
            // will write over NULL
            self->icursor--;
            // terminate, for good measure
            *(self->tail)='\0';
        }
        // acutally print the data to the buffer
        test=vsnprintf((char *)self->icursor,available,fmt,ap)+1;
        
        va_end(ap);
        
        // sanity check
        assert((test-1)<=available);

        // adjust pointers
        self->icursor+=test;
        if (self->tail<self->icursor) {
            self->tail=self->icursor;
            *(self->tail)='\0';
        }
        
        return test;
    }
    return -1;
}
// End function mbb_printf
#endif //__QNX__

/// @fn off_t mbb_capacity(mbbuf_t * self)
/// @brief get buffer capacity.
/// @param[in] self buffer reference
/// @return capacity on success, -1 otherwise
off_t mbb_capacity(mbbuf_t *self)
{
	if (self) {
		return self->capacity;
	}
	return -1;
}
// End function mbb_capacity

/// @fn off_t mbb_length(mbbuf_t * self)
/// @brief get content length.
/// @param[in] self buffer reference
/// @return content length on success, -1 otherwise
off_t mbb_length(mbbuf_t *self)
{
	if (self) {
		return MB_CONTENT_LEN(self);
	}
	return -1;
}
// End function mbb_length

/// @fn off_t mbb_available(mbbuf_t * self)
/// @brief get available space.
/// @param[in] self buffer reference
/// @return bytes available on success, -1 otherwise
off_t mbb_available(mbbuf_t *self)
{
	if (self) {
		return MB_AVAILABLE(self);
	}
	return -1;
}
// End function mbb_available

/// @fn off_t mbb_iavailable(mbbuf_t * self)
/// @brief get offset of available space.
/// @param[in] self buffer reference
/// @return offset on success, -1 otherwise
off_t mbb_iavailable(mbbuf_t *self)
{
	if (self) {
		return MB_AVAILABLE_OS(self,MB_ICOFFSET(self));
	}
	return -1;
}
// End function mbb_iavailable

/// @fn off_t mbb_icursor(mbbuf_t * self)
/// @brief get input cursor offset.
/// @param[in] self buffer reference
/// @return offset on success, -1 otherwise
off_t mbb_icursor(mbbuf_t *self)
{
	if (self) {
		return MB_ICOFFSET(self);
	}
	return -1;
}
// End function mbb_icursor

/// @fn off_t mbb_ocursor(mbbuf_t * self)
/// @brief get output cursor offset.
/// @param[in] self buffer reference
/// @return offset on success, -1 otherwise
off_t mbb_ocursor(mbbuf_t *self)
{
	if (self) {
		return MB_OCOFFSET(self);
	}
	return -1;
}
// End function mbb_ocursor

/// @fn byte * mbb_head(mbbuf_t * self)
/// @brief get head pointer.
/// @param[in] self buffer reference
/// @return pointer to head on success, NULL otherwise
byte *mbb_head(mbbuf_t *self)
{
	if (self) {
		return self->head;
	}
	return NULL;
}
// End function mbb_head

/// @fn void mbb_buf_show(mbbuf_t * self, int verbose, int indent)
/// @brief output buffer parameter summary to console.
/// @param[in] self buffer reference
/// @param[in] verbose enable verbose output
/// @param[in] indent indent depth
/// @param[in] indent TBD
/// @return none
void mbb_buf_show(mbbuf_t *self, bool verbose, int indent)
{
	printf("%*s[xfbuf: %p]\n",indent,(indent?" ":""), self);
	if (self) {
	#ifdef HAS_LONG_LONG
		printf("%*s[capacity: %lld]\n",indent,(indent?" ":""), (long long)self->capacity);
		printf("%*s[head: %p]\n",indent,(indent?" ":""), self->head);
		printf("%*s[tail: %p/%lld]\n",indent,(indent?" ":""), self->tail,(long long)MB_TOFFSET(self));
		printf("%*s[icursor: %p/%lld]\n",indent,(indent?" ":""), self->icursor,(long long)MB_ICOFFSET(self));
		printf("%*s[ocursor: %p/%lld]\n",indent,(indent?" ":""), self->ocursor,(long long)MB_OCOFFSET(self));
		printf("%*s[len    : %lld]\n",indent,(indent?" ":""), (long long)mbb_length(self));
		printf("%*s[iavail  : %lld]\n",indent,(indent?" ":""), (long long)mbb_iavailable(self));
    #else
        printf("%*s[capacity: %ld]\n",indent,(indent?" ":""), ( long)self->capacity);
        printf("%*s[head: %p]\n",indent,(indent?" ":""), self->head);
        printf("%*s[tail: %p/%ld]\n",indent,(indent?" ":""), self->tail,( long)MB_TOFFSET(self));
        printf("%*s[icursor: %p/%ld]\n",indent,(indent?" ":""), self->icursor,( long)MB_ICOFFSET(self));
        printf("%*s[ocursor: %p/%ld]\n",indent,(indent?" ":""), self->ocursor,( long)MB_OCOFFSET(self));
        printf("%*s[len    : %ld]\n",indent,(indent?" ":""), ( long)mbb_length(self));
        printf("%*s[iavail  : %ld]\n",indent,(indent?" ":""), ( long)mbb_iavailable(self));
#endif
		if (self->capacity>0) {
			if (self->head) {
                char *cp=(char *)self->head;
                const char *fmt=NULL;
				printf("%*s[content:",indent,(indent?" ":""));
				while (cp < ((char *)(self->head+self->capacity)) ) {
					fmt=(isprint((*cp))?"%c":"%02X");
					printf(fmt, *cp++);
				}
				printf("]\n");
			}
		}
	}
}
// End function mbb_buf_show

#ifdef WITH_MBBUF_TEST
/// @fn int mbbuf_test(int verbose)
/// @brief mbbuf unit test.
/// @param[in] argc number or args
/// @param[in] argv array of args
/// @return return 0 on success, -1 otherwise
int mbbuf_test(int argc, char **argv)
{
    // create an empty buffer
	off_t init_sz=64;
	mbbuf_t *buf=mbb_new(init_sz,NULL,0);
	off_t track_len=0;
    char *cp1="0123456790abcdef";
    int len1=0;
    char *read_data=NULL;
    char *cp2="0123456790abcdef";
    int len2=0;
    char *cp3="0123456789012345678901234567890123456789012345678901234567890123";
    int len3=00;
	byte *pop_data=NULL;
    const char *foo="01234567";
	byte *set_data=NULL;
	mbbuf_t *dup=NULL;
	byte *rdata=NULL;
	int test=0;

	printf("%s - new buf\n",__FUNCTION__);
	assert(mbb_capacity(buf)==init_sz);
	assert(mbb_ocursor(buf)==0);
	assert(mbb_icursor(buf)==0);
	assert(mbb_length(buf)==0);
	assert(mbb_available(buf)==init_sz);

	len1=strlen(cp1);
	printf("%s - append\n",__FUNCTION__);

	// add 16 (16/64)
	mbb_append(buf,(byte *)cp1,len1);
	track_len+=len1;
	assert(mbb_length(buf)==track_len);
	assert(mbb_available(buf)==(init_sz-track_len));
	assert(mbb_ocursor(buf)==0);
	assert(mbb_icursor(buf)==0);
	
	printf("%s - seek\n",__FUNCTION__);
	// write:tail read:head
	mbb_iseek(buf, MB_SEEK_TAIL);
	mbb_oseek(buf, MB_SEEK_HEAD);
	assert(mbb_ocursor(buf)==0);
	assert(mbb_icursor(buf)==(len1));
	
	printf("%s - read\n",__FUNCTION__);
	// read 8
	read_data=(char *)mbb_read(buf,8);
	assert(strncmp(read_data,cp1,8)==0);
	assert(mbb_ocursor(buf)==8);
	
	printf("%s - write\n",__FUNCTION__);
	// write 16 (32/64)
	cp2="0123456790abcdef";
	len2=strlen(cp2);
	mbb_write(buf,(byte *)cp2,len2);
	track_len+=len2;
	assert(mbb_length(buf)==(track_len));
	assert(mbb_available(buf)==(init_sz-(len1+len2)));
	
	printf("%s - write (auto-resize)\n",__FUNCTION__);

	cp3="0123456789012345678901234567890123456789012345678901234567890123";
    len3=strlen(cp3);
	mbb_write(buf,(byte *)cp3,len3);
	track_len+=len3;
	//printf("%s:%d -  write %d got here h[%p] t[%p] i[%p] o[%p]\n",__FUNCTION__,__LINE__,len3, buf->head,buf->tail,buf->icursor,buf->ocursor);
	//printf("len %ld capacity %ld avail %ld\n",mbb_length(buf),mbb_capacity(buf),mbb_available(buf));
	assert(mbb_length(buf)==(track_len));
	assert(mbb_available(buf)==(0));
	
	printf("%s - pop\n",__FUNCTION__);
	// pop 32
	pop_data=mbb_pop(buf,32);
	track_len-=32;
	assert(mbb_length(buf)==(track_len));
	assert(mbb_available(buf)==(32));

	printf("%s - push\n",__FUNCTION__);
	// push 8
    foo="01234567";
	mbb_push(buf,(byte *)foo,8);
	track_len+=8;
	assert(mbb_length(buf)==(track_len));
	assert(mbb_available(buf)==(24));
	
	// trim
	printf("%s - trim\n",__FUNCTION__);
	mbbuf_trim(buf);
	assert(mbb_length(buf)==(track_len));
	assert(mbb_available(buf)==(0));
	
	// set
	printf("%s - set\n",__FUNCTION__);
	mbb_set(buf,4,8,'Z');
	mbb_oseek(buf,4);
	set_data=mbb_read(buf,8);
	assert(strcmp((const char *)set_data,"ZZZZZZZZ")==0);
	free(set_data);
	
	printf("%s - dup\n",__FUNCTION__);
	dup=mbb_dup(buf);
	assert(mbb_length(dup)==mbb_length(buf));
	assert(mbb_capacity(dup)==mbb_capacity(buf));
	assert(mbb_available(dup)==mbb_available(buf));
	assert(mbb_icursor(dup)==mbb_icursor(buf));
	assert(mbb_ocursor(dup)==mbb_ocursor(buf));
	
	mbb_seek(buf,0);

    printf("%s - printf [%"PRId64"]\n",__FUNCTION__,OFFT_CAST(mbb_icursor(buf)));

	mbb_printf(buf,"0123456789 %5.2f %d %10s",3.14159,1234,"abc");
	rdata=mbb_read(buf,32);
	assert(strncmp((const char *)rdata,"0123456789  3.14 1234        abc",32)==0);
	free(rdata);
	printf("////////////////\n");
	mbb_set_capacity(buf,16);
	mbb_seek(buf,0);

	printf("%s - printf(0) [%"PRId64"] cap[%"PRId64"] iavail[%"PRId64"]\n",__FUNCTION__,OFFT_CAST(mbb_icursor(buf)),OFFT_CAST(mbb_capacity(buf)),OFFT_CAST(mbb_iavailable(buf)));

	test=mbb_printf(buf,"0123456789ABCDEFGHIJ0123456789AB");
    assert(test==33);
	mbb_buf_show(buf,false,5);
	rdata=mbb_read(buf,16);
	assert(strncmp((const char *)rdata,"0123456789ABCDEF",16)==0);
	free(rdata);
	
	mbb_iseek(buf,MB_SEEK_TAIL);
	printf("%s - printf(1) [%"PRId64"] cap[%"PRId64"] iavail[%"PRId64"]\n",__FUNCTION__,OFFT_CAST(mbb_icursor(buf)),OFFT_CAST(mbb_capacity(buf)),OFFT_CAST(mbb_iavailable(buf)));
	
	mbb_printf(buf,", and a wee bit more");
    mbb_buf_show(buf,false,5);
	mbb_seek(buf,MB_SEEK_HEAD);
	rdata=mbb_read(buf,mbb_length(buf));
	assert(strncmp((const char *)rdata,"0123456789ABCDEFGHIJ0123456789AB, and a wee bit more",strlen((const char *)rdata))==0);
	free(rdata);
	
	mbb_iseek(buf,MB_SEEK_TAIL);

	printf("%s - printf(2)[%"PRId64"] cap[%"PRId64"] iavail[%"PRId64"]\n",__FUNCTION__,OFFT_CAST(mbb_icursor(buf)),OFFT_CAST(mbb_capacity(buf)),OFFT_CAST(mbb_iavailable(buf)));

	mbb_printf(buf,", and then just a wee bit more");
    mbb_buf_show(buf,false,5);
	mbb_seek(buf,MB_SEEK_HEAD);
	rdata=mbb_read(buf,mbb_length(buf));
	assert(strncmp((const char *)rdata,"0123456789ABCDEFGHIJ0123456789AB, and a wee bit more, and then just a wee bit more",strlen((const char *)rdata))==0);
	free(rdata);
	
	mbb_iseek(buf,MB_SEEK_TAIL);

    printf("%s - printf(3)[%"PRId64"] cap[%"PRId64"] iavail[%"PRId64"]\n",__FUNCTION__,OFFT_CAST(mbb_icursor(buf)),OFFT_CAST(mbb_capacity(buf)),OFFT_CAST(mbb_iavailable(buf)));

    mbb_printf(buf,", and howzabout a little bit more?");
    mbb_buf_show(buf,false,5);
	mbb_seek(buf,MB_SEEK_HEAD);
	rdata=mbb_read(buf,mbb_length(buf));
	assert(strncmp((const char *)rdata,"0123456789ABCDEFGHIJ0123456789AB, and a wee bit more, and then just a wee bit more, and howzabout a little bit more?",strlen((const char *)rdata))==0);
	free(rdata);
	
	printf("%s - free\n",__FUNCTION__);
	free(pop_data);
	free(read_data);
	//mbb_destroy(&dup);
	mbb_destroy(&buf);
	mbb_destroy(&dup);

	return 0;
}
// End function mbbuf_test

#endif //WITH_MBBUF_TEST
