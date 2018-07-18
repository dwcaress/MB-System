/****************************************************************************/
/* Copyright (c) 2018 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  : Struct definitions and C/C++ function prototypes to work with */
/*            MB-TRN real-time data. This shall remain system-agnostic (no  */
/*            auv-qnx dependencies).                                        */
/* Filename : mbtrn_types.h                                                 */
/* Author   : Henthorn                                                      */
/* Project  : Precision Control/Axial Geodesy                               */
/* Version  : 1.0                                                           */
/* Created  : 02/05/2018                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/

#ifndef _MBTRN_TYPES_H
#define _MBTRN_TYPES_H

#include <errno.h> 
#include <stdlib.h> 

/* Protocol definitions
*/ 

/* Predfined function return values
*/ 
#define  MBTRN_OK      0
#define  MBTRN_FAIL   -1
#define  MBTRN_TIMEO   EWOULDBLOCK

/* Some protocol stuff
*/ 
#define  MBTRN_CON   "CON"
#define  MBTRN_DCON  "DCN"
#define  MBTRN_ACK   "ACK"
#define  MBTRN_MB1   "MB1"

#define  MAX_NBEAMS  512


/* MB-TRN individual beam data
*/ 
#pragma pack(push,1)
struct mbtrn_beam_data
{
   unsigned int beam_num; /* beam number (0 is port-most beam) */
   double rhox;           /* along track position wrt sonar meters */
   double rhoy;           /* cross track position wrt soanr meters */
   double rhoz;           /* vertical position wrt to sonar meters
                             (positive down) */
};


/* MB-TRN sounding data (all beams) with vehicle context
*/ 
struct mbtrn_sounding 
{
   double ts;             /* epoch time */
   double lat;            /* vehicle position latitude degrees */
   double lon;            /* vehicle position longitude degrees */
   double depth;          /* vehicle position depth meters */
   double hdg;            /* vehicle heading in radians */
   unsigned int nbeams;   /* number of beams in this record */

   struct mbtrn_beam_data beams[MAX_NBEAMS];
};
#pragma pack(pop)

/* Size of non-beam (non-variable) sounding data */
#define MBTRN_FIXED_SIZE   (5*sizeof(double)+sizeof(unsigned int))


/* Header for MB-TRN communication packets
*/ 
struct mbtrn_header 
{
  unsigned int type;
  unsigned int size;
};

/* Linear size of the header */
#define MBTRN_HEADER_SIZE  (2*sizeof(unsigned int))
/* Offset to the beam data */
#define MBTRN_BEAMS_OFFSET (MBTRN_HEADER_SIZE+MBTRN_FIXED_SIZE)


/* MB-TRN MB1 communications packet consists of header and sounding data
*/ 
struct mbtrn_mb1
{
  struct mbtrn_header    header;
  struct mbtrn_sounding  sounding;
};


/* Linearize an mbtrn_mb1 struct into a vector of size vsize, Checksum included.
   If successful, return the number of bytes of the vector (> RBG_HEADER_SIZE)
   that was used including the checksum. The contents of vector is a complete
   and valid MB1 packet.
   Return 0 on usage errors (e.g., NULL pointer)
   Return 1 if the size of the vector is non sufficient
*/ 
unsigned int mbtrn_deflate_mb1(unsigned char vector[], struct mbtrn_mb1 *mb1, unsigned int vsize);


/* Populate an mbtrn_mb1 struct using a vector of size vsize containing a valid
   and complete MB1 packet.
   If successful, return the number of bytes in the vector (> RBG_HEADER_SIZE)
   that was used including the checksum.
   Return 0 on usage errors (e.g., NULL pointer)
   Return 1 if the MB1 packet is incomplete (perhaps the vector too small)
   Return 2 if the checksums do not match.
*/ 
unsigned int mbtrn_inflate_mb1(struct mbtrn_mb1 *mb1, unsigned char vector[], unsigned int vsize);

#endif
