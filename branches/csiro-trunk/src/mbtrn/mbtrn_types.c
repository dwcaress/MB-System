/****************************************************************************/
/* Copyright (c) 2018 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  : C/C++ functions to work with MB-TRN real-time data. This code */
/*            shall remain system-agnostic (no auv-qnx dependencies).       */
/* Filename : mbtrn_types.cc                                                */
/* Author   : Henthorn                                                      */
/* Project  : Precision Control/Axial Geodesy                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2018                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/

#include <string.h>
#include <stdio.h>
#include "mbtrn_types.h"

/* Checksum function to use for MB1 packets sent/received from RBF.
   checksum is calculated using first vlen bytes.
   Note vlen == vsize - sizeof(unsigned int) where vsize is
   defined as above.
*/ 
static unsigned int mbtrn_checksum(unsigned char vector[], unsigned int vlen);


/* Linearize an mbtrn_mb1 struct into a vector of size vsize. Checksum included.
   If successful, return the number of bytes of the vector (> mbtrn_HEADER_SIZE)
   that was used including the checksum.
   Return 0 on usage errors (e.g., NULL pointer)
   Return 1 if the size of the vector is non sufficient
*/ 
unsigned int mbtrn_deflate_mb1(unsigned char vector[],
			       struct mbtrn_mb1 *mb1, unsigned int vsize)
{
   unsigned int checksum, p, i;
   unsigned int sizeofmb1;

   /* Check usage errors
   */ 
   if (!mb1)
   {
      printf("mbtrn_deflate_mb1 - NULL pointer to mb1\n");
      return 0L;
   }

   if (strncmp("MB1", (char*)&mb1->header.type, 3))
   {
      printf("mbtrn_deflate_mb1 - packet is not of type MB1\n");
      return 1L;
   }


   /* Make sure vector is large enough to hold mb1
   */ 
   sizeofmb1 = 
        MBTRN_BEAMS_OFFSET +         /* rhox,y,z           beam_num   */
        mb1->sounding.nbeams * (3*sizeof(double) + sizeof(unsigned int)) +
        sizeof(checksum);
   if (sizeofmb1 > vsize)
   {
      printf("mbtrn_deflate_mb1 - vector size is less than the calculated of %d for %ld beams\n",
         sizeofmb1, mb1->sounding.nbeams);
      return 1L;
   }

   /* Pack contents of mb1 into vector starting with the header
   */ 
   memset(vector, 0, vsize);
   p = 0;
   memcpy(vector+p, &mb1->header.type, sizeof(mb1->header.type));
   p+=sizeof(mb1->header.type);

   memcpy(vector+p, &mb1->header.size, sizeof(mb1->header.size));
   p+=sizeof(mb1->header.size);

   /* Fixed part of the sounding (vehicle position, etc)
   */ 
   memcpy(vector+p, &mb1->sounding.ts, sizeof(mb1->sounding.ts));
   p+=sizeof(mb1->sounding.ts);

   memcpy(vector+p, &mb1->sounding.lat, sizeof(mb1->sounding.lat));
   p+=sizeof(mb1->sounding.lat);

   memcpy(vector+p, &mb1->sounding.lon, sizeof(mb1->sounding.lon));
   p+=sizeof(mb1->sounding.lon);

   memcpy(vector+p, &mb1->sounding.depth, sizeof(mb1->sounding.depth));
   p+=sizeof(mb1->sounding.depth);

   memcpy(vector+p, &mb1->sounding.hdg, sizeof(mb1->sounding.hdg));
   p+=sizeof(mb1->sounding.hdg);

   memcpy(vector+p, &mb1->sounding.nbeams, sizeof(mb1->sounding.nbeams));
   p+=sizeof(mb1->sounding.nbeams);

   /* Now the beam ranges
   */ 
   for (i = 0; i < mb1->sounding.nbeams; i++)
   {
      memcpy(vector+p, &mb1->sounding.beams[i].beam_num, sizeof(mb1->sounding.beams[i].beam_num));
      p+=sizeof(mb1->sounding.beams[i].beam_num);

      memcpy(vector+p, &mb1->sounding.beams[i].rhox, sizeof(mb1->sounding.beams[i].rhox));
      p+=sizeof(mb1->sounding.beams[i].rhox);

      memcpy(vector+p, &mb1->sounding.beams[i].rhoy, sizeof(mb1->sounding.beams[i].rhoy));
      p+=sizeof(mb1->sounding.beams[i].rhoy);

      memcpy(vector+p, &mb1->sounding.beams[i].rhoz, sizeof(mb1->sounding.beams[i].rhoz));
      p+=sizeof(mb1->sounding.beams[i].rhoz);
   }

   /* And finally the checksum
   */ 
   checksum = mbtrn_checksum(vector, sizeofmb1 - sizeof(checksum));
   memcpy(vector+p, &checksum, sizeof(checksum));
   p+=sizeof(checksum);

   /* Return the number of bytes of vector we used
   */ 
   return p;
}


/* Populate an mbtrn_mb1 struct using a vector of size vsize containing an MB1 packet.
   If successful, return the number of bytes of the vector (> mbtrn_HEADER_SIZE)
   that was used.
   Return 0 on usage errors (e.g., NULL pointer)
   Return 1 if the MB1 packet is invalid (perhaps the vector too small)
   Return 2 if the checksums do not match.
*/ 
unsigned int mbtrn_inflate_mb1(struct mbtrn_mb1 *mb1,
			       unsigned char vector[], unsigned int vsize)
{
   unsigned int checksum, p, i;
   unsigned int sizeofmb1;

   /* Check usage errors
   */ 
   if (!mb1)
   {
      printf("mbtrn_inflate_mb1 - NULL pointer to mb1\n");
      return 0L;
   }

   if (vsize < MBTRN_BEAMS_OFFSET)
   {
      printf("mbtrn_inflate_mb1 - vector size is less than the minimum of %d\n", MBTRN_BEAMS_OFFSET);
      return 1L;
   }

   /* Get contents of mb1 from vector starting with the header
   */ 
   p = 0;
   memcpy(&mb1->header.type, vector+p, sizeof(mb1->header.type));
   p+=sizeof(mb1->header.type);

   memcpy(&mb1->header.size, vector+p, sizeof(mb1->header.size));
   p+=sizeof(mb1->header.size);

   /* Sanity check the type and size
   */ 
   if (strncmp("MB1", (char*)&mb1->header.type, sizeof("MB1")))
   {
      printf("mbtrn_inflate_mb1 - packet is not of type MB1\n");
      return 1L;
   }

   if (mb1->header.size < MBTRN_FIXED_SIZE)
   {
      printf("mbtrn_inflate_mb1 - packet data size %ld is less than the minimum of %d\n",
         mb1->header.size, MBTRN_FIXED_SIZE);
      return 1L;
   }

   /* Get the fixed-size items from the sounding
   */ 
   memcpy(&mb1->sounding.ts, vector+p, sizeof(mb1->sounding.ts));
   p+=sizeof(mb1->sounding.ts);

   memcpy(&mb1->sounding.lat, vector+p, sizeof(mb1->sounding.lat));
   p+=sizeof(mb1->sounding.lat);

   memcpy(&mb1->sounding.lon, vector+p, sizeof(mb1->sounding.lon));
   p+=sizeof(mb1->sounding.lon);

   memcpy(&mb1->sounding.depth, vector+p, sizeof(mb1->sounding.depth));
   p+=sizeof(mb1->sounding.depth);

   memcpy(&mb1->sounding.hdg, vector+p, sizeof(mb1->sounding.hdg));
   p+=sizeof(mb1->sounding.hdg);

   memcpy(&mb1->sounding.nbeams, vector+p, sizeof(mb1->sounding.nbeams));
   p+=sizeof(mb1->sounding.nbeams);

   /* Make sure vector is large enough to hold mb1
   */ 
   sizeofmb1 = 
        MBTRN_BEAMS_OFFSET +         /* rhox,y,z           beam_num */
        mb1->sounding.nbeams * (3*sizeof(double) + sizeof(unsigned int)) +
        sizeof(checksum);
   if (sizeofmb1 > vsize)
   {
      printf("mbtrn_inflate_mb1 - vector size is less than the calculated of %d for %ld beams\n",
         sizeofmb1, mb1->sounding.nbeams);
      return 1L;
   }

   /* Now the beam ranges
   */ 
   for (i = 0; i < mb1->sounding.nbeams; i++)
   {
      memcpy(&mb1->sounding.beams[i].beam_num, vector+p, sizeof(mb1->sounding.beams[i].beam_num));
      p+=sizeof(mb1->sounding.beams[i].beam_num);

      memcpy(&mb1->sounding.beams[i].rhox, vector+p, sizeof(mb1->sounding.beams[i].rhox));
      p+=sizeof(mb1->sounding.beams[i].rhox);

      memcpy(&mb1->sounding.beams[i].rhoy, vector+p, sizeof(mb1->sounding.beams[i].rhoy));
      p+=sizeof(mb1->sounding.beams[i].rhoy);

      memcpy(&mb1->sounding.beams[i].rhoz, vector+p, sizeof(mb1->sounding.beams[i].rhoz));
      p+=sizeof(mb1->sounding.beams[i].rhoz);
   }

   /* Take a look at the checksum
   */ 
   memcpy(&checksum, vector+sizeofmb1-sizeof(checksum), sizeof(checksum)); p+=sizeof(checksum);
   if (checksum != (i = mbtrn_checksum(vector, vsize-sizeof(unsigned int))))
   {
      printf("mbtrn_inflate_mb1 - vector checksum %d does not match calculated checksum %d\n",
         checksum, i);
      return 2L;
   }

   return p;
}


/* Checksum function to use for MB1 packets received from RBF.
*/
static unsigned int mbtrn_checksum(unsigned char vector[], unsigned int vlen)
{
   unsigned char *buf = vector;
   int i = 0;
   unsigned int result = 0;

   for (i = 0; i < vlen; i++)
      result += (unsigned int)(*buf++);

   return result;
}
