// ******************************************************************************** 
// ** BitPack
// **  
// ** Takes data and packs it into a byte array.
// **
// ** Copyright (C) 2025 3D at Depth Inc. All Rights Reserved
// ** www.3DatDepth.com  This software can not be copied and/or distributed without 
// ** the express permission of 3D at Depth Inc.
// ********************************************************************************

#include <stdlib.h>
#include "mb_bitpack.h"

/*****************************************************************************
** C wrapper API for class BitPack
*****************************************************************************/

// BitPack C API object wrapper
struct mb_bitpack_s {
    void *obj;
};
typedef struct mb_bitpack_s mb_bitpack_t;

#ifdef __cplusplus
extern "C" {
#endif

// BitPack C API
void *mb_bitpack_new();
void mb_bitpack_delete(void **mbbpptr);
void mb_bitpack_clear(void *mbbpptr);
void mb_bitpack_setbitsize(void *mbbpptr, unsigned int nbits);
bool mb_bitpack_resize(void *mbbpptr, unsigned int arraySize, char **buffer, unsigned int* buffer_size);
int mb_bitpack_getbytestoread(void *mbbpptr);
int mb_bitpack_getbytestowrite(void *mbbpptr);
bool mb_bitpack_readvalue(void *mbbpptr, unsigned int* value);
bool mb_bitpack_writevalue(void *mbbpptr, unsigned int value);

#ifdef __cplusplus
}
#endif

//-----------------------------------------------------------------------------
/* allocate a new instance of a bit packed array */
void *mb_bitpack_new()
{
    mb_bitpack_t *m = (mb_bitpack_t *)malloc(sizeof(*m));

    if (NULL != m) {
        memset(m, 0, sizeof(*m));
        BitPack *obj    = new BitPack();
        m->obj = obj;
    }

	void *mbbpptr = (void *) m;
    return mbbpptr;
}

//-----------------------------------------------------------------------------
/* deallocate a bit packed array */
void mb_bitpack_delete(void **pmbbpptr)
{
	mb_bitpack_t **pself = (mb_bitpack_t **)pmbbpptr;
    if (NULL != pself) {
        mb_bitpack_t *self = (mb_bitpack_t *)(*pself);
        delete static_cast<BitPack *>(self->obj);
        free(self);
        *pmbbpptr = NULL;
    }
}

//-----------------------------------------------------------------------------
/* clear the contents of a bit packed array */
void   mb_bitpack_clear(void *vself)
{
	mb_bitpack_t *self = (mb_bitpack_t *) vself;
    if (NULL != self) {
        BitPack *obj = static_cast<BitPack *>(self->obj);
        if( NULL != obj) {
            obj->Clear();
        }
    }
    return;
}

//-----------------------------------------------------------------------------
/* set the bit packing (number of bits per value) in a bit packed array */
void   mb_bitpack_setbitsize(void *vself, unsigned int nbits)
{
	mb_bitpack_t *self = (mb_bitpack_t *) vself;
	unsigned char WriteSizeBits = (unsigned char) nbits;
    if (NULL != self) {
        BitPack *obj = static_cast<BitPack *>(self->obj);
        if( NULL != obj) {
            obj->SetParameters(WriteSizeBits);
        }
    }
    return;
}

//-----------------------------------------------------------------------------
/* set the size of a bit packed array */
bool   mb_bitpack_resize(void *vself, unsigned int arraySize, char **pbuffer, unsigned int* bufferSize)
{
	mb_bitpack_t *self = (mb_bitpack_t *) vself;
    *pbuffer = NULL;
    *bufferSize = 0;
    if (NULL != self) {
        BitPack *obj = static_cast<BitPack *>(self->obj);
        if( NULL != obj) {
        	bool status = obj->ReSize(arraySize);
        	if (status) {
        		*pbuffer = (char *) obj->GetBuffer();
        		*bufferSize = obj->GetBufferSize();
        	} else {
        		*pbuffer = NULL;
        		*bufferSize = 0;
        	}
            return status;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------
int    mb_bitpack_getbytestoread(void *vself)
{
	mb_bitpack_t *self = (mb_bitpack_t *) vself;
    if (NULL != self) {
        BitPack *obj = static_cast<BitPack *>(self->obj);
        if( NULL != obj) {
            return obj->GetBufferSize();
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------
int    mb_bitpack_getbytestowrite(void *vself)
{
	mb_bitpack_t *self = (mb_bitpack_t *) vself;
    if (NULL != self) {
        BitPack *obj = static_cast<BitPack *>(self->obj);
        if( NULL != obj) {
            return obj->GetBytesToWrite();
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------
/* read the next value from a bit packed array */
bool   mb_bitpack_readvalue(void *vself, unsigned int* pValue)
{
	mb_bitpack_t *self = (mb_bitpack_t *) vself;
    if (NULL != self) {
        BitPack *obj = static_cast<BitPack *>(self->obj);
        if( NULL != obj) {
            return obj->ReadValue(pValue);
        }
    }
    return false;
}

//-----------------------------------------------------------------------------
/* write the next value to a bit packed array */
bool   mb_bitpack_writevalue(void *vself, unsigned int value)
{
	mb_bitpack_t *self = (mb_bitpack_t *) vself;
    if (NULL != self) {
        BitPack *obj = static_cast<BitPack *>(self->obj);
        if( NULL != obj) {
            return obj->WriteValue(value);
        }
    }
    return false;
}

/******************************************************************************
class BitPack constuctor
   
Used by:
	ArchiveCriaat/ArchiveSriat

 *****************************************************************************/
BitPack::BitPack():
  m_iBuffSizeBits  (7936) // 32*31*8
, m_iBuffSize       (992)  // 32*31
, m_WriteInBits     (0)   
, m_iWriteSizeBits  (31) 
{ 
   m_iBuff = new unsigned char[m_iBuffSize]; 
   Clear(); 
}
/*****************************************************************************
** class BitPack destructor
*****************************************************************************/
BitPack::~BitPack()
{
   if (m_iBuff)
   {
      delete[] m_iBuff;
   }
   m_iBuff = 0;
}
/*****************************************************************************
** WriteValue(value)
*****************************************************************************/
bool BitPack::WriteValue(unsigned int value)
{
   m_iByteOffset = (unsigned int)(m_WriteInBits/8);
   m_iBitOffset  = (unsigned int)(m_WriteInBits%8);
   unsigned char bits_write = (8-m_iBitOffset);
   unsigned char lower_mask = 0xFF >> bits_write;
   unsigned char upper_mask = 0xFF << m_iBitOffset;

   unsigned char write_size_bits = m_iWriteSizeBits;
   
   if((unsigned int)m_iByteOffset >= m_iBuffSize) 
      return false; // not enough space.

   while (write_size_bits >= bits_write) // will fill whole space
   {
      m_iBuff[m_iByteOffset] = (m_iBuff[m_iByteOffset] & lower_mask) +
                              ((value << m_iBitOffset)& upper_mask);
      
      write_size_bits -= bits_write;
      m_iByteOffset++;
      m_WriteInBits   += bits_write;
      m_iBitOffset     = 0;
      lower_mask       = 0;
      upper_mask       = 0xff;
      value = value >> bits_write; // value shift down.
      bits_write       = 8; // next cycle can write a whole 8 bits.
      if((unsigned int)m_iByteOffset >= m_iBuffSize) 
         return false; // not enough space.
   }
   if (write_size_bits < bits_write) // will only fill partical.
   {
      upper_mask = ((0xFF >> (8-write_size_bits)) << m_iBitOffset);
      lower_mask = ~upper_mask;

      m_iBuff[m_iByteOffset] = (m_iBuff[m_iByteOffset] & lower_mask) +
                            ((value << m_iBitOffset)& upper_mask);
      m_WriteInBits += write_size_bits;
      //m_iBitOffset   = (unsigned int)(m_WriteInBits%8);
   }

   return (m_iBuffSizeBits >= m_WriteInBits); // bits go off of array?
}
/*****************************************************************************
** ReadValue(*value)
*****************************************************************************/
bool BitPack::ReadValue(unsigned int* pValue)
{
   unsigned int value   = 0;
   unsigned int shifter = 1;

   m_iByteOffset = (unsigned int)(m_ReadInBits/8);
   m_iBitOffset  = (unsigned int)(m_ReadInBits%8);
   unsigned char bits_read = (8-m_iBitOffset);

   unsigned char lower_mask = 0xFF >> bits_read;
   unsigned char upper_mask = 0xFF << m_iBitOffset;

   unsigned char read_size_bits = m_iWriteSizeBits;

   *pValue = 0;
   if(m_iByteOffset >= m_iBuffSize) 
      return false; // off of end of buffer.

   while (read_size_bits >= bits_read) // will read whole space
   {
      value += ((m_iBuff[m_iByteOffset] & upper_mask) >> m_iBitOffset )*shifter;

      shifter *= 1 << bits_read;

      read_size_bits  -= bits_read;
      m_iByteOffset++;
      m_ReadInBits    += bits_read;
      m_iBitOffset     = 0;

      upper_mask       = 0xff;
      bits_read        = 8; // next cycle can write a whole 8 bits.
      if(m_iByteOffset >= m_iBuffSize) 
         return false; // off of end of buffer.
   }
   if (read_size_bits < bits_read) // will only read partical.
   {
      upper_mask = ((0xFF >> (8-read_size_bits)) << m_iBitOffset);

      value += ((m_iBuff[m_iByteOffset] & upper_mask) >> m_iBitOffset )*shifter;

      m_ReadInBits  += read_size_bits;
      //m_iBitOffset   = (unsigned int)(m_ReadInBits%8);
   }
   *pValue = value;
   return (m_iBuffSizeBits >= m_ReadInBits); // bits go off of array?
}
/*****************************************************************************
** SetParameters (write size bits)
*****************************************************************************/
void BitPack::SetParameters(unsigned char WriteSizeBits)
{
   // Limit the write size to 32 - since the ReadValue and WriteValue only can 
   // take in a 32 bit number (could increase if type was changed).
   m_iWriteSizeBits = WriteSizeBits>31? 31:WriteSizeBits;
}
/*****************************************************************************
** ReSize( array size)
*****************************************************************************/
bool BitPack::ReSize(unsigned int arraySize)
{
   int whole = (arraySize*m_iWriteSizeBits)/8;
   //int frac  = (arraySize*m_iWriteSizeBits)%8;
   int byte_size = whole; // + (frac>0? 1:0); we have 2 extra bytes
   byte_size += 5; // spare bytes, one could be taken by last fraction, and 4 zeros to mark end point.
   if (m_iBuff)
      delete[] m_iBuff;
   //Sleep(100);

   m_iBuff = 0;
   m_iBuffSize = byte_size;
   m_iBuffSizeBits = m_iBuffSize*8; // total number of bits.
   m_iBuff     = new unsigned char[m_iBuffSize];
   if (m_iBuff) 
      Clear();
   return (m_iBuff != 0);
}
/*****************************************************************************/
