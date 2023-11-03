/*--------------------------------------------------------------------
 *    The MB-system:	mbbs_swapbytes.c	3/3/2014
 *
 *    Copyright (c) 2014-2023 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
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

void swapbytes(MemType *buf, unsigned int bufsz) {
	unsigned char *a = (unsigned char *)buf;
	unsigned char *b = a + 1;
	for (unsigned int nswap = bufsz / 2; nswap > 0; nswap--, a += 2, b += 2) {
		const unsigned char tmp = *a;
		*a = *b;
		*b = tmp;
	}
}

void revbytes(MemType *buf, unsigned int bufsz) {
	unsigned char *a = (unsigned char *)buf;
	unsigned char *b = a + bufsz - 1;
	for (unsigned int nrev = bufsz / 2; nrev > 0; nrev--, a++, b--) {
		const unsigned char tmp = *a;
		*a = *b;
		*b = tmp;
	}
}
