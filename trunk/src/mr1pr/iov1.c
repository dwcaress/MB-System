/*--------------------------------------------------------------------
 *    The MB-system:	iov1.c	3/7/2003
 *	$Id: iov1.c,v 5.1 2006/01/24 19:24:04 caress Exp $
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
 * $Log: iov1.c,v $
 * Revision 5.1  2006/01/24 19:24:04  caress
 * Version 5.0.8 beta.
 *
 * Revision 5.0  2003/03/11 19:09:14  caress
 * Initial version.
 *
 *
 *
 *--------------------------------------------------------------------*/
/*
 *	Copyright (c) 1991 by University of Hawaii.
 */

/*
   iov1.c --
   I/O routines for obsolete Hawaii MR1 version 1 files.
*/

#include <stdio.h>

#include "mr1pr_defines.h"

extern double mr1_nand();
extern float mr1_nanf();

static int nan_init= 0;
static float nan_f;
static double nan_d;

int
mr1_xdrpnghdrv1(Ping *png, XDR *xdrs)
/*
   Internal routine.
   Does XDR decoding of an MR1 version 1 ping header.
   Returns 1 if successful, 0 otherwise.
*/
{
	int mr1_xdrsidev1(PingSide *, XDR *);

	/* output in obsolete version 1 format not allowed! */
	if (xdrs->x_op == XDR_ENCODE)
		return 0;

	if (!nan_init) {
		nan_f= mr1_nanf();
		nan_d= mr1_nand();
		nan_init= 1;
	}

	if (!xdr_long(xdrs, (long *) &(png->png_tm.tv_sec)))
		return 0;
	if (!xdr_long(xdrs, (long *) &(png->png_tm.tv_usec)))
		return 0;
	png->png_period= nan_f;

	png->png_slon= nan_d;
	png->png_slat= nan_d;
	png->png_scourse= nan_f;
	png->png_laybackrng= nan_f;
	png->png_laybackbrg= nan_f;

	if (!xdr_double(xdrs, &(png->png_tlon)))
		return 0;
	if (!xdr_double(xdrs, &(png->png_tlat)))
		return 0;
	if (!xdr_float(xdrs, &(png->png_tcourse)))
		return 0;

	png->png_compass.sns_int= nan_f;
	png->png_compass.sns_nsamps= 0;
	if (!xdr_float(xdrs, &(png->png_compass.sns_repval)))
		return 0;

	png->png_depth.sns_int= nan_f;
	png->png_depth.sns_nsamps= 0;
	if (!xdr_float(xdrs, &(png->png_depth.sns_repval)))
		return 0;

	if (!xdr_float(xdrs, &(png->png_alt)))
		return 0;

	png->png_pitch.sns_int= nan_f;
	png->png_pitch.sns_nsamps= 0;
	if (!xdr_float(xdrs, &(png->png_pitch.sns_repval)))
		return 0;

	png->png_roll.sns_int= nan_f;
	png->png_roll.sns_nsamps= 0;
	if (!xdr_float(xdrs, &(png->png_roll.sns_repval)))
		return 0;

	png->png_snspad= 0;

	if (!xdr_float(xdrs, &(png->png_temp)))
		return 0;
	if (!xdr_float(xdrs, &(png->png_atssincr)))
		return 0;

	png->png_magcorr= nan_f;
	png->png_sndvel= nan_f;

	if (!mr1_xdrsidev1(&(png->png_sides[ACP_PORT]), xdrs))
		return 0;
	png->png_sides[ACP_PORT].ps_bdrange= png->png_alt;

	if (!mr1_xdrsidev1(&(png->png_sides[ACP_STBD]), xdrs))
		return 0;
	png->png_sides[ACP_STBD].ps_bdrange= png->png_alt;

	return 1;
}

int
mr1_xdrsidev1(PingSide *ps, XDR *xdrs)
/*
   Internal routine.
   Does XDR decoding of an obsolete MR1 version 1 PingSide header.
   Returns 1 if successful, 0 otherwise.
*/
{
	/* output in obsolete version 1 format not allowed! */
	if (xdrs->x_op == XDR_ENCODE)
		return 0;

	if (!nan_init) {
		nan_f= mr1_nanf();
		nan_d= mr1_nand();
		nan_init= 1;
	}

	/* HMRG code never archived anything to the old ps_trans[]
	   fields, so their contents are meaningless -- this code
	   reads those meaningless values and then stores a NaN
	   to the new ps_xmitpwr field */
	if (!xdr_float(xdrs, &(ps->ps_xmitpwr)))
		return 0;
	if (!xdr_float(xdrs, &(ps->ps_xmitpwr)))
		return 0;
	ps->ps_xmitpwr= nan_f;

	/* HMRG code never archived anything to the ps_gain field
	   prior to the format version 2 changeover -- this code
	   reads that meaningless value and then stores a NaN
	   to the ps_gain field */
	if (!xdr_float(xdrs, &(ps->ps_gain)))
		return 0;
	ps->ps_gain= nan_f;

	/* HMRG code never archived anything to the ps_pulse field
	   prior to the format version 2 changeover with the exception
	   of one format conversion program (sb4b2mr) -- this code
	   reads that value and then replaces it with a NaN unless
	   it is non-zero */
	if (!xdr_float(xdrs, &(ps->ps_pulse)))
		return 0;
	if (ps->ps_pulse == 0.)
		ps->ps_pulse= nan_f;

	if (!xdr_int(xdrs, &(ps->ps_btycount)))
		return 0;
	if (xdrs->x_op == XDR_DECODE)
		ps->ps_btypad= 0;
	if (!xdr_float(xdrs, &(ps->ps_ssoffset)))
		return 0;
	if (!xdr_int(xdrs, &(ps->ps_sscount)))
		return 0;
	if (xdrs->x_op == XDR_DECODE)
		ps->ps_sspad= 0;

	return 1;
}
