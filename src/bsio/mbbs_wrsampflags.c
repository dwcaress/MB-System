/*--------------------------------------------------------------------
 *    The MB-system:	mbbs_wrsampflags.c	3/3/2014
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
 *	Copyright (c) 2011 by University of Hawaii.
 */

/*
   wrsampflags.c --
   Routines to rewrite ping sample flags within
   Hawaii Mapping Research Group BS files.
*/

#include <stdio.h>

#include "mbbs.h"
#include "mbbs_defines.h"

extern unsigned long bs_iobytecnt;

static MemType *bswsf_databuf = (MemType *)0;
static unsigned int bswsf_databufsz = 0;

/*
   Flags all samples of the specified datatype on the named side at
   across-track distances greater than swradius with {BTYD,SSD}_SWEDGE for
   the ping whose header is located at an arbitrary file byte offset.
*/
int mbbs_setswradius(int version, FILE *fp, long phoffset, int side, unsigned int dtmask, float swradius) {
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

	if (fp == (FILE *)0)
		return BS_BADARG;

	switch (side) {
	case ACP_PORT:
	case ACP_STBD:
		break;
	default:
		return BS_BADARG;
	}

	if (!(dtmask & BS_DTM_BATHYMETRY) && !(dtmask & BS_DTM_SIDESCAN))
		return BS_BADARG;

	if (swradius < 0.)
		return BS_BADARG;

	if (fseek(fp, phoffset, SEEK_SET) != 0)
		return BS_FSEEK;
	XDR xdr;
	xdrstdio_create(&xdr, fp, XDR_DECODE);

	Ping png;
	if (!mbbs_xdrpnghdr(&png, &xdr, version)) {
		xdr_destroy(&xdr);
		return BS_READ;
	}

	int trimbty = 0;
	int trimss = 0;
	int nbtyvals = 0;
	if (dtmask & BS_DTM_BATHYMETRY) {
		if ((nbtyvals = png.png_sides[side].ps_btycount) > 0)
			trimbty = 1;
	}
	int ssstart = 0;
	int nssvals = 0;
	if (dtmask & BS_DTM_SIDESCAN) {
		float sscutoff;
		if ((sscutoff = swradius - png.png_sides[side].ps_ssxoffset) < 0.)
			sscutoff = 0.;
		ssstart = sscutoff / png.png_ssincr;
		if ((nssvals = png.png_sides[side].ps_sscount) > ssstart)
			trimss = 1;
	}
	if (!trimbty && !trimss) {
		xdr_destroy(&xdr);
		return BS_SUCCESS;
	}

	/* remember current file offset at end of ping header */
	const long pdoffset = phoffset + bs_iobytecnt;

	int err;
	if ((err = mbbs_pngrealloc(&png, &bswsf_databuf, &bswsf_databufsz)) != BS_SUCCESS) {
		xdr_destroy(&xdr);
		return err;
	}
	if ((err = mbbs_rdpngdata(&png, bswsf_databuf, &xdr)) != BS_SUCCESS) {
		xdr_destroy(&xdr);
		return err;
	}
	const unsigned long datasz = bs_iobytecnt;
	xdr_destroy(&xdr);

	PingData pngdata;
	if ((err = mbbs_getpngdataptrs(&png, bswsf_databuf, &pngdata)) != BS_SUCCESS)
		return err;

	/* set {BTYD,SSD}_SWEDGE on all desired samples */
	if (dtmask & BS_DTM_BATHYMETRY) {
		int bsi;
		if (png.png_flags & PNG_XYZ)
			bsi = 3;
		else
			bsi = 2;
		float *bty;
		if ((bty = pngdata.pd_bty[side]) == (float *)0)
			return BS_BADDATA;
		unsigned int *btyflags;
		if ((btyflags = pngdata.pd_btyflags[side]) == (unsigned int *)0)
			return BS_BADDATA;
		for (int i = 0; i < nbtyvals; i++) {
			if (bty[bsi * i] > swradius)
				btyflags[i] |= BTYD_SWEDGE;
		}
	}
	if (dtmask & BS_DTM_SIDESCAN) {
		unsigned char *ssflags;
		if ((ssflags = pngdata.pd_ssflags[side]) == (unsigned char *)0)
			return BS_BADDATA;
		for (int i = ssstart; i < nssvals; i++)
			ssflags[i] |= SSD_SWEDGE;
	}

	/* seek to beginning of ping data region and rewrite it */
	if (fseek(fp, pdoffset, SEEK_SET) != 0)
		return BS_FSEEK;
	xdrstdio_create(&xdr, fp, XDR_ENCODE);
	if ((err = mbbs_wrpngdata(&png, bswsf_databuf, &xdr)) != BS_SUCCESS) {
		xdr_destroy(&xdr);
		return err;
	}

	bs_iobytecnt = datasz;
	xdr_destroy(&xdr);
	(void)fflush(fp);

	return BS_SUCCESS;
}
