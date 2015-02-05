/*--------------------------------------------------------------------
 *    The MB-system:	mbbs_iomrv2.c	3/3/2014
 *	$Id$
 *
 *    Copyright (c) 2014-2015 by
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
 *	Copyright (c) 2005 by University of Hawaii.
 */

/*
   io.c --
   I/O routines for obsolete Hawaii Mapping Research Group
   MR1 version 2 files.
*/

#include <stdio.h>

#include "mbbs_defines.h"

extern double mbbs_nand();
extern float mbbs_nanf();

extern unsigned long bs_iobytecnt;
extern int bs_ionaninit;
extern float bs_ionanf;
extern double bs_ionand;

int
mbbs_mr1_xdrpnghdrv2(Ping *png, XDR *xdrs)
/*
   Internal routine.
   Does XDR decoding of an obsolete MR1 version 2 ping header.
   Returns 1 if successful, 0 otherwise.
*/
{
	int tvsec, tvusec;
	unsigned long sidebc;
	int mr1_xdrsidev2(PingSide *, XDR *, unsigned long *);

	bs_iobytecnt= 0;

	/* output in obsolete format not allowed! */
	if (xdrs->x_op == XDR_ENCODE)
		return 0;

	if (!bs_ionaninit) {
		bs_ionanf= mbbs_nanf();
		bs_ionand= mbbs_nand();
		bs_ionaninit= 1;
	}

	png->png_flags= PNG_BTYSSFLAGSABSENT;

	/* depending upon the platform, the size of the timeval
	   struct's fields may be 4 or 8 bytes; for backward
	   compatibility with old files that use 4-byte fields,
	   we use 4-byte primitives */
	if (!xdr_int(xdrs, &tvsec))
		return 0;
	bs_iobytecnt+= 4;
	png->png_tm.tv_sec= tvsec;
	if (!xdr_int(xdrs, &tvusec))
		return 0;
	bs_iobytecnt+= 4;
	png->png_tm.tv_usec= tvusec;

	if (!xdr_float(xdrs, &(png->png_period)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_double(xdrs, &(png->png_slon)))
		return 0;
	bs_iobytecnt+= 8;
	if (!xdr_double(xdrs, &(png->png_slat)))
		return 0;
	bs_iobytecnt+= 8;
	if (!xdr_float(xdrs, &(png->png_scourse)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_float(xdrs, &(png->png_laybackrng)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_float(xdrs, &(png->png_laybackbrg)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_double(xdrs, &(png->png_tlon)))
		return 0;
	bs_iobytecnt+= 8;
	if (!xdr_double(xdrs, &(png->png_tlat)))
		return 0;
	bs_iobytecnt+= 8;
	if (!xdr_float(xdrs, &(png->png_tcourse)))
		return 0;
	bs_iobytecnt+= 4;

	if (!xdr_float(xdrs, &(png->png_compass.sns_int)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_int(xdrs, &(png->png_compass.sns_nsamps)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_float(xdrs, &(png->png_compass.sns_repval)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_float(xdrs, &(png->png_depth.sns_int)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_int(xdrs, &(png->png_depth.sns_nsamps)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_float(xdrs, &(png->png_depth.sns_repval)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_float(xdrs, &(png->png_pitch.sns_int)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_int(xdrs, &(png->png_pitch.sns_nsamps)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_float(xdrs, &(png->png_pitch.sns_repval)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_float(xdrs, &(png->png_roll.sns_int)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_int(xdrs, &(png->png_roll.sns_nsamps)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_float(xdrs, &(png->png_roll.sns_repval)))
		return 0;
	bs_iobytecnt+= 4;
	png->png_snspad= 0;

	if (!xdr_float(xdrs, &(png->png_temp)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_float(xdrs, &(png->png_ssincr)))
		return 0;
	bs_iobytecnt+= 4;

	png->png_ssyoffsetmode= PNG_SSYOM_UNKNOWN;

	if (!xdr_float(xdrs, &(png->png_alt)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_float(xdrs, &(png->png_magcorr)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_float(xdrs, &(png->png_sndvel)))
		return 0;
	bs_iobytecnt+= 4;

	png->png_cond= bs_ionanf;
	png->png_magx= bs_ionanf;
	png->png_magy= bs_ionanf;
	png->png_magz= bs_ionanf;

	if (!mr1_xdrsidev2(&(png->png_sides[ACP_PORT]), xdrs, &sidebc))
		return 0;
	bs_iobytecnt+= sidebc;
	png->png_sides[ACP_PORT].ps_ssndrmask= 0.;
	png->png_sides[ACP_PORT].ps_ssyoffset= bs_ionanf;

	if (!mr1_xdrsidev2(&(png->png_sides[ACP_STBD]), xdrs, &sidebc))
		return 0;
	bs_iobytecnt+= sidebc;
	png->png_sides[ACP_STBD].ps_ssndrmask= 0.;
	png->png_sides[ACP_STBD].ps_ssyoffset= bs_ionanf;

	return 1;
}

int
mr1_xdrsidev2(PingSide *ps, XDR *xdrs, unsigned long *bytecnt)
/*
   Internal routine.
   Does XDR decoding of an obsolete MR1 version 2 PingSide header.
   Records the total number of bytes transferred into *bytecnt.
   Returns 1 if successful, 0 otherwise.
*/
{
	*bytecnt= 0;

	/* output in obsolete format not allowed! */
	if (xdrs->x_op == XDR_ENCODE)
		return 0;

	if (!xdr_float(xdrs, &(ps->ps_xmitpwr)))
		return 0;
	*bytecnt+= 4;
	if (!xdr_float(xdrs, &(ps->ps_gain)))
		return 0;
	*bytecnt+= 4;
	if (!xdr_float(xdrs, &(ps->ps_pulse)))
		return 0;
	*bytecnt+= 4;
	if (!xdr_float(xdrs, &(ps->ps_bdrange)))
		return 0;
	*bytecnt+= 4;
	if (!xdr_int(xdrs, &(ps->ps_btycount)))
		return 0;
	*bytecnt+= 4;
	ps->ps_btypad= 0;
	if (!xdr_float(xdrs, &(ps->ps_ssxoffset)))
		return 0;
	*bytecnt+= 4;
	if (!xdr_int(xdrs, &(ps->ps_sscount)))
		return 0;
	*bytecnt+= 4;
	ps->ps_sspad= 0;

	return 1;
}
