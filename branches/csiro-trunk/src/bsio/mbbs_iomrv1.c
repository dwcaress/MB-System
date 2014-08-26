/*--------------------------------------------------------------------
 *    The MB-system:	mbbs_iomrv1.c	3/3/2014
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
 *	Copyright (c) 1991 by University of Hawaii.
 */

/*
   iomrv1.c --
   I/O routines for obsolete Hawaii Mapping Research Group
   MR1 version 1 files.
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
mbbs_mr1_xdrpnghdrv1(Ping *png, XDR *xdrs)
/*
   Internal routine.
   Does XDR decoding of an MR1 version 1 ping header.
   Returns 1 if successful, 0 otherwise.
*/
{
	int tvsec, tvusec;
	unsigned long sidebc;
	int mr1_xdrsidev1(PingSide *, XDR *, unsigned long *);

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

	png->png_period= bs_ionanf;
	png->png_slon= bs_ionand;
	png->png_slat= bs_ionand;
	png->png_scourse= bs_ionanf;
	png->png_laybackrng= bs_ionanf;
	png->png_laybackbrg= bs_ionanf;

	if (!xdr_double(xdrs, &(png->png_tlon)))
		return 0;
	bs_iobytecnt+= 8;
	if (!xdr_double(xdrs, &(png->png_tlat)))
		return 0;
	bs_iobytecnt+= 8;
	if (!xdr_float(xdrs, &(png->png_tcourse)))
		return 0;
	bs_iobytecnt+= 4;

	png->png_compass.sns_int= bs_ionanf;
	png->png_compass.sns_nsamps= 0;
	if (!xdr_float(xdrs, &(png->png_compass.sns_repval)))
		return 0;
	bs_iobytecnt+= 4;

	png->png_depth.sns_int= bs_ionanf;
	png->png_depth.sns_nsamps= 0;
	if (!xdr_float(xdrs, &(png->png_depth.sns_repval)))
		return 0;
	bs_iobytecnt+= 4;

	if (!xdr_float(xdrs, &(png->png_alt)))
		return 0;
	bs_iobytecnt+= 4;

	png->png_pitch.sns_int= bs_ionanf;
	png->png_pitch.sns_nsamps= 0;
	if (!xdr_float(xdrs, &(png->png_pitch.sns_repval)))
		return 0;
	bs_iobytecnt+= 4;

	png->png_roll.sns_int= bs_ionanf;
	png->png_roll.sns_nsamps= 0;
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
	png->png_magcorr= bs_ionanf;
	png->png_sndvel= bs_ionanf;
	png->png_cond= bs_ionanf;
	png->png_magx= bs_ionanf;
	png->png_magy= bs_ionanf;
	png->png_magz= bs_ionanf;

	if (!mr1_xdrsidev1(&(png->png_sides[ACP_PORT]), xdrs, &sidebc))
		return 0;
	bs_iobytecnt+= sidebc;
	png->png_sides[ACP_PORT].ps_bdrange= png->png_alt;
	png->png_sides[ACP_PORT].ps_ssndrmask= 0.;
	png->png_sides[ACP_PORT].ps_ssyoffset= bs_ionanf;

	if (!mr1_xdrsidev1(&(png->png_sides[ACP_STBD]), xdrs, &sidebc))
		return 0;
	bs_iobytecnt+= sidebc;
	png->png_sides[ACP_STBD].ps_bdrange= png->png_alt;
	png->png_sides[ACP_STBD].ps_ssndrmask= 0.;
	png->png_sides[ACP_STBD].ps_ssyoffset= bs_ionanf;

	return 1;
}

int
mr1_xdrsidev1(PingSide *ps, XDR *xdrs, unsigned long *bytecnt)
/*
   Internal routine.
   Does XDR decoding of an obsolete MR1 version 1 PingSide header.
   Records the total number of bytes transferred into *bytecnt.
   Returns 1 if successful, 0 otherwise.
*/
{
	*bytecnt= 0;

	/* output in obsolete format not allowed! */
	if (xdrs->x_op == XDR_ENCODE)
		return 0;

	if (!bs_ionaninit) {
		bs_ionanf= mbbs_nanf();
		bs_ionand= mbbs_nand();
		bs_ionaninit= 1;
	}

	/* HMRG code never archived anything to the old ps_trans[]
	   fields, so their contents are meaningless -- this code
	   reads those meaningless values and then stores a NaN
	   to the new ps_xmitpwr field */
	if (!xdr_float(xdrs, &(ps->ps_xmitpwr)))
		return 0;
	*bytecnt+= 4;
	if (!xdr_float(xdrs, &(ps->ps_xmitpwr)))
		return 0;
	*bytecnt+= 4;
	ps->ps_xmitpwr= bs_ionanf;

	/* HMRG code never archived anything to the ps_gain field
	   prior to the format MR1 version 2 changeover -- this code
	   reads that meaningless value and then stores a NaN
	   to the ps_gain field */
	if (!xdr_float(xdrs, &(ps->ps_gain)))
		return 0;
	*bytecnt+= 4;
	ps->ps_gain= bs_ionanf;

	/* HMRG code never archived anything to the ps_pulse field
	   prior to the format MR1 version 2 changeover with the exception
	   of one format conversion program (sb4b2mr) -- this code
	   reads that value and then replaces it with a NaN unless
	   it is non-zero */
	if (!xdr_float(xdrs, &(ps->ps_pulse)))
		return 0;
	*bytecnt+= 4;
	if (ps->ps_pulse == 0.)
		ps->ps_pulse= bs_ionanf;

	if (!xdr_int(xdrs, &(ps->ps_btycount)))
		return 0;
	*bytecnt+= 4;
	if (xdrs->x_op == XDR_DECODE)
		ps->ps_btypad= 0;
	if (!xdr_float(xdrs, &(ps->ps_ssxoffset)))
		return 0;
	*bytecnt+= 4;
	if (!xdr_int(xdrs, &(ps->ps_sscount)))
		return 0;
	*bytecnt+= 4;
	if (xdrs->x_op == XDR_DECODE)
		ps->ps_sspad= 0;

	return 1;
}
