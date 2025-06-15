// ******************************************************************************** 
// ** BitPack
// **  
// ** Takes data and packs it into a byte array.
// **
// ** Copyright (C) 2025 3D at Depth Inc. All Rights Reserved
// ** www.3DatDepth.com  This software can not be copied and/or distributed without 
// ** the express permission of 3D at Depth Inc.
// ********************************************************************************
#include <cstring>
   
class BitPack
{
public:
   BitPack();
   ~BitPack();

   int GetBufferSize() { return (m_iBuffSize); }
   unsigned char *GetBuffer() { return (m_iBuff); }
   int GetBytesToWrite() { return (int(m_WriteInBits/8) + 1); }
   bool WriteValue(unsigned int value);
   bool ReadValue(unsigned int* pValue);
   void SetParameters(unsigned char WriteSizeBits);
   void Clear() {m_WriteInBits = 0; m_ReadInBits=0; memset(m_iBuff,0,sizeof(unsigned char)*m_iBuffSize); }
   bool ReSize(unsigned int arraySize);

   unsigned int m_iBuffSizeBits;  // number of bits in m_iBuff. Know if we write off end of array.
   unsigned long long m_WriteInBits;    // where to write next
   unsigned long long m_ReadInBits;     // where to read next
   unsigned char  m_iWriteSizeBits; // number of bits written on each write.
   unsigned char* m_iBuff;          // pointer to the byte buffer array
   unsigned int m_iBuffSize;      // number of bytes in m_iBuff
   // working parameters (might be nice to watch)
   unsigned int m_iByteOffset;
   unsigned int m_iBitOffset;
};
