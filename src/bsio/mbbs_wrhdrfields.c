/*--------------------------------------------------------------------
 *    The MB-system:	mbbs_wrhdrfields.c	3/3/2014
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
 *	Copyright (c) 2006 by University of Hawaii.
 */

/*
   wrhdrfields.c --
   Routines to rewrite individual file and/or ping header
   fields of Hawaii Mapping Research Group BS files.
*/

#include <stdio.h>

#include "mbbs.h"
#include "mbbs_defines.h"

extern unsigned long bs_iobytecnt;

/*
   Set the bits from bitmask in the file header flags field.
*/
int mbbs_wrfflagssetbits(FILE *fp, unsigned int bitmask) {
	if (fseek(fp, (long)0, SEEK_SET) != 0)
		return BS_FSEEK;
	XDR xdr;
	xdrstdio_create(&xdr, fp, XDR_DECODE);

	bs_iobytecnt = 0;
	int version;
	if (!xdr_int(&xdr, &version))
		return BS_READ;
	switch (version) {
	case MR1_VERSION_1_0:
	case MR1_VERSION_2_0:
	case BS_VERSION_1_0:
	case BS_VERSION_1_1:
	case BS_VERSION_1_2:
		return BS_BADDATA;
	default:
		break;
	}
	bs_iobytecnt += 4;
	int count;
	if (!xdr_int(&xdr, &count))
		return BS_READ;
	bs_iobytecnt += 4;
	unsigned int flags;
	if (!xdr_u_int(&xdr, &flags))
		return BS_READ;
	bs_iobytecnt += 4;

	xdr_destroy(&xdr);

	flags |= bitmask;

	if (fseek(fp, (long)-4, SEEK_CUR) != 0)
		return BS_FSEEK;
	bs_iobytecnt -= 4;
	xdrstdio_create(&xdr, fp, XDR_ENCODE);
	if (!xdr_u_int(&xdr, &flags))
		return BS_WRITE;
	bs_iobytecnt += 4;
	xdr_destroy(&xdr);
	(void)fflush(fp);

	return BS_SUCCESS;
}

/*
   Clear the bits of bitmask from the file header flags field.
*/
int mbbs_wrfflagsclrbits(FILE *fp, unsigned int bitmask) {
	if (fseek(fp, (long)0, SEEK_SET) != 0)
		return BS_FSEEK;
	XDR xdr;
	xdrstdio_create(&xdr, fp, XDR_DECODE);

	bs_iobytecnt = 0;
	int version;
	if (!xdr_int(&xdr, &version))
		return BS_READ;
	switch (version) {
	case MR1_VERSION_1_0:
	case MR1_VERSION_2_0:
	case BS_VERSION_1_0:
	case BS_VERSION_1_1:
	case BS_VERSION_1_2:
		return BS_BADDATA;
	default:
		break;
	}
	bs_iobytecnt += 4;
	int count;
	if (!xdr_int(&xdr, &count))
		return BS_READ;
	bs_iobytecnt += 4;
	unsigned int flags;
	if (!xdr_u_int(&xdr, &flags))
		return BS_READ;
	bs_iobytecnt += 4;

	xdr_destroy(&xdr);

	flags &= ~bitmask;

	if (fseek(fp, (long)-4, SEEK_CUR) != 0)
		return BS_FSEEK;
	bs_iobytecnt -= 4;
	xdrstdio_create(&xdr, fp, XDR_ENCODE);
	if (!xdr_u_int(&xdr, &flags))
		return BS_WRITE;
	bs_iobytecnt += 4;
	xdr_destroy(&xdr);
	(void)fflush(fp);

	return BS_SUCCESS;
}

/*
   Writes the ping flags field of a ping header
   located at an arbitrary file byte offset.
*/
int mbbs_wrpflags(int version, FILE *fp, long phoffset, unsigned int flags) {
	bs_iobytecnt = 0;

	switch (version) {
	case MR1_VERSION_1_0:
	case MR1_VERSION_2_0:
		return BS_BADDATA;
	case BS_VERSION_1_0:
	case BS_VERSION_1_1:
	case BS_VERSION_1_2:
	case BS_VERSION_1_3:
	case BS_VERSION_1_4:
		break;
	default:
		return BS_BADDATA;
	}

	if (fseek(fp, phoffset, SEEK_SET) != 0)
		return BS_FSEEK;
	XDR xdr;
	xdrstdio_create(&xdr, fp, XDR_ENCODE);
	if (!xdr_u_int(&xdr, &flags))
		return BS_WRITE;
	bs_iobytecnt += 4;
	xdr_destroy(&xdr);
	(void)fflush(fp);

	return BS_SUCCESS;
}

/*
   Set the bits from bitmask in the ping flags field of
   a ping header located at an arbitrary file byte offset.
*/
int mbbs_wrpflagssetbits(int version, FILE *fp, long phoffset, unsigned int bitmask) {
	bs_iobytecnt = 0;

	switch (version) {
	case MR1_VERSION_1_0:
	case MR1_VERSION_2_0:
		return BS_BADDATA;
	case BS_VERSION_1_0:
	case BS_VERSION_1_1:
	case BS_VERSION_1_2:
	case BS_VERSION_1_3:
	case BS_VERSION_1_4:
		break;
	default:
		return BS_BADDATA;
	}

	if (fseek(fp, phoffset, SEEK_SET) != 0)
		return BS_FSEEK;
	XDR xdr;
	xdrstdio_create(&xdr, fp, XDR_DECODE);
	unsigned int flags;
	if (!xdr_u_int(&xdr, &flags))
		return BS_READ;
	bs_iobytecnt += 4;
	xdr_destroy(&xdr);

	flags |= bitmask;

	if (fseek(fp, phoffset, SEEK_SET) != 0)
		return BS_FSEEK;
	bs_iobytecnt -= 4;
	xdrstdio_create(&xdr, fp, XDR_ENCODE);
	if (!xdr_u_int(&xdr, &flags))
		return BS_WRITE;
	bs_iobytecnt += 4;
	xdr_destroy(&xdr);
	(void)fflush(fp);

	return BS_SUCCESS;
}

/*
   Clear the bits in bitmask from the ping flags field of
   a ping header located at an arbitrary file byte offset.
*/
int mbbs_wrpflagsclrbits(int version, FILE *fp, long phoffset, unsigned int bitmask) {
	bs_iobytecnt = 0;

	switch (version) {
	case MR1_VERSION_1_0:
	case MR1_VERSION_2_0:
		return BS_BADDATA;
	case BS_VERSION_1_0:
	case BS_VERSION_1_1:
	case BS_VERSION_1_2:
	case BS_VERSION_1_3:
	case BS_VERSION_1_4:
		break;
	default:
		return BS_BADDATA;
	}

	if (fseek(fp, phoffset, SEEK_SET) != 0)
		return BS_FSEEK;
	XDR xdr;
	xdrstdio_create(&xdr, fp, XDR_DECODE);
	unsigned int flags;
	if (!xdr_u_int(&xdr, &flags))
		return BS_READ;
	bs_iobytecnt += 4;
	xdr_destroy(&xdr);

	flags &= ~bitmask;

	if (fseek(fp, phoffset, SEEK_SET) != 0)
		return BS_FSEEK;
	bs_iobytecnt -= 4;
	xdrstdio_create(&xdr, fp, XDR_ENCODE);
	if (!xdr_u_int(&xdr, &flags))
		return BS_WRITE;
	bs_iobytecnt += 4;
	xdr_destroy(&xdr);
	(void)fflush(fp);

	return BS_SUCCESS;
}

/*
   Writes ship longitude, latitude and course fields
   of a ping header located at an arbitrary file byte offset.
*/
int mbbs_wrsllc(int version, FILE *fp, long phoffset, double slon, double slat, float scourse) {
	bs_iobytecnt = 0;

	switch (version) {
	case MR1_VERSION_1_0:
		return BS_BADDATA;
	case MR1_VERSION_2_0:
		phoffset += 12;
		break;
	case BS_VERSION_1_0:
	case BS_VERSION_1_1:
	case BS_VERSION_1_2:
	case BS_VERSION_1_3:
	case BS_VERSION_1_4:
		phoffset += 16;
		break;
	default:
		return BS_BADDATA;
	}

	if (fseek(fp, phoffset, SEEK_SET) != 0)
		return BS_FSEEK;
	XDR xdr;
	xdrstdio_create(&xdr, fp, XDR_ENCODE);

	if (!xdr_double(&xdr, &slon))
		return BS_WRITE;
	bs_iobytecnt += 8;
	if (!xdr_double(&xdr, &slat))
		return BS_WRITE;
	bs_iobytecnt += 8;
	if (!xdr_float(&xdr, &scourse))
		return BS_WRITE;
	bs_iobytecnt += 4;

	xdr_destroy(&xdr);
	(void)fflush(fp);

	return BS_SUCCESS;
}

/*
   Writes towfish longitude, latitude and course fields
   of a ping header located at an arbitrary file byte offset.
*/
int mbbs_wrtllc(int version, FILE *fp, long phoffset, double tlon, double tlat, float tcourse) {
	bs_iobytecnt = 0;

	switch (version) {
	case MR1_VERSION_1_0:
		return BS_BADDATA;
	case MR1_VERSION_2_0:
		phoffset += 40;
		break;
	case BS_VERSION_1_0:
	case BS_VERSION_1_1:
	case BS_VERSION_1_2:
	case BS_VERSION_1_3:
	case BS_VERSION_1_4:
		phoffset += 44;
		break;
	default:
		return BS_BADDATA;
	}

	if (fseek(fp, phoffset, SEEK_SET) != 0)
		return BS_FSEEK;
	XDR xdr;
	xdrstdio_create(&xdr, fp, XDR_ENCODE);

	if (!xdr_double(&xdr, &tlon))
		return BS_WRITE;
	bs_iobytecnt += 8;
	if (!xdr_double(&xdr, &tlat))
		return BS_WRITE;
	bs_iobytecnt += 8;
	if (!xdr_float(&xdr, &tcourse))
		return BS_WRITE;
	bs_iobytecnt += 4;

	xdr_destroy(&xdr);
	(void)fflush(fp);

	return BS_SUCCESS;
}

/*
   Writes towfish longitude and latitude fields of a
   ping header located at an arbitrary file byte offset.
*/
int mbbs_wrtll(int version, FILE *fp, long phoffset, double tlon, double tlat) {
	bs_iobytecnt = 0;

	switch (version) {
	case MR1_VERSION_1_0:
		return BS_BADDATA;
	case MR1_VERSION_2_0:
		phoffset += 40;
		break;
	case BS_VERSION_1_0:
	case BS_VERSION_1_1:
	case BS_VERSION_1_2:
	case BS_VERSION_1_3:
	case BS_VERSION_1_4:
		phoffset += 44;
		break;
	default:
		return BS_BADDATA;
	}

	if (fseek(fp, phoffset, SEEK_SET) != 0)
		return BS_FSEEK;
	XDR xdr;
	xdrstdio_create(&xdr, fp, XDR_ENCODE);

	if (!xdr_double(&xdr, &tlon))
		return BS_WRITE;
	bs_iobytecnt += 8;
	if (!xdr_double(&xdr, &tlat))
		return BS_WRITE;
	bs_iobytecnt += 8;

	xdr_destroy(&xdr);
	(void)fflush(fp);

	return BS_SUCCESS;
}
