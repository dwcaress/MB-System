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
#ifndef _ALTEXTYPES_H
#define _ALTEXTYPES_H

#include <sys/types.h>
//#include </usr/include/syslog.h>  // QNX/Watcom bug; include full path

#define True 1
#define False 0
#define On True
#define Off False

#define OK 0
#define ERROR -1

#define MaxSharedObjNameLen 256

typedef unsigned char Boolean;

#define MaxLong 2147483648

#define callMemberFunction(objPtr, memberPtr) ((objPtr)->*(memberPtr))

#define max(a,b)  (((a) > (b)) ? (a) : (b))
#define min(a,b)  (((a) < (b)) ? (a) : (b))


#endif
