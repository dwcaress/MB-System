/****************************************************************************/
/* Copyright (c) 2000 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  :                                                               */
/* Filename : ourTypes.h                                                    */
/* Author   :                                                               */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/07/2000                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#ifndef _OUR_TYPES_H
#define _OUR_TYPES_H

#include <sys/types.h>

#if defined(__QNX__)
typedef unsigned char Boolean;
#define True 1
#define False 0
#else
typedef bool Boolean;
#define True true
#define False false
#endif

#define OT_MAX(a,b)  (((a) > (b)) ? (a) : (b))
#define OT_MIN(a,b)  (((a) < (b)) ? (a) : (b))

#endif // this file
