/*--------------------------------------------------------------------
 *    The MB-system:	mbbs_swapbytes.c	3/3/2014
 *	$Id$
 *
 *    Copyright (c) 2014-2014 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/* This source code is part of the mbbsio library used to read and write
 * swath sonar data in the bsio format devised and used by the
 * Hawaii Mapping Research Group of the University of Hawaii.
 * This source code was made available by Roger Davis of the
 * University of Hawaii under the GPL. Minor modifications have
 * been made to the version distributed here as part of MB-System.
 *
 * Author:	Roger Davis (primary author)
 * Author:	D. W. Caress (MB-System revisions)
 * Date:	March 3, 2014 (MB-System revisions)
 *
 *--------------------------------------------------------------------*/
/*
 *	Copyright (c) 1998 by University of Hawaii.
 */

#include "mbbs_defines.h"
#include "mbbs_mem.h"

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

void
revbytes(MemType *buf, unsigned int bufsz)
{
	unsigned char *a, *b, tmp;
	unsigned int nrev;

	a= (unsigned char *) buf;
	b= a+bufsz-1;
	for (nrev= bufsz/2; nrev > 0; nrev--, a++, b--) {
		tmp= *a;
		*a= *b;
		*b= tmp;
	}

	return;
}
