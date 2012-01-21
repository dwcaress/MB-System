/* Added HAVE_CONFIG_H for autogen files */
#ifdef HAVE_CONFIG_H
#  include <mbsystem_config.h>
#endif


/*--------------------------------------------------------------------
 *    The MB-system:	swapbytes.c	3/7/2003
 *	$Id: swapbytes.c 1770 2009-10-19 17:16:39Z caress $
 *
 *    Copyright (c) 2003 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/* This source code is part of the MR1PR library used to read and write
 * swath sonar data in the MR1PR format devised and used by the 
 * Hawaii Mapping Research Group of the University of Hawaii.
 * This source code was made available by Roger Davis of the
 * University of Hawaii under the GPL. Minor modifications have
 * been made to the version distributed here as part of MB-System.
 *
 * Author:	Roger Davis (primary author)
 * Author:	D. W. Caress (MB-System revisions)
 * Date:	March 7, 2003 (MB-System revisions)
 * $Log: swapbytes.c,v $
 * Revision 5.0  2003/03/11 19:09:14  caress
 * Initial version.
 *
 *
 *
 *--------------------------------------------------------------------*/
/*
 *	Copyright (c) 1998 by University of Hawaii.
 */

/* Various system dependent defines */
#ifdef SUN
#define Free			(void) free
#define MemType			char
#define MemSizeType		unsigned int
#define MemCopy(m0, m1, n)	bcopy((char *) (m0), (char *) (m1), (int) (n))
#define MemZero(m, n)		bzero((char *) (m), (int) (n))
#else
#define Free			free
#define MemType			void
#define MemSizeType		size_t
#define MemCopy(m0, m1, n)	(void) memmove((void *) (m1), (void *) (m0), (size_t) (n))
#define MemZero(m, n)		(void) memset((void *) (m), (int) 0, (size_t) (n))
#endif

#include "mem.h"

void
swapbytes(MemType *buf, unsigned int bufsz)
{
	unsigned char *a, *b, tmp;
	unsigned int nswap;

	a= (unsigned char *) buf;
	b= a+1;
	for (nswap= bufsz/2; nswap > 0; nswap--, a+= 2, b+= 2) {
		tmp= *a;
		*a= *b;
		*b= tmp;
	}

	return;
}
