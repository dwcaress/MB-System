/* Added HAVE_CONFIG_H for autogen files */
#ifdef HAVE_CONFIG_H
#  include <mbsystem_config.h>
#endif


/*--------------------------------------------------------------------
 *    The MB-system:	io.c	3/7/2003
 *	$Id: io.c 1770 2009-10-19 17:16:39Z caress $
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
 * $Log: io.c,v $
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
   io.c --
   I/O routines for Hawaii MR1 files.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "mr1pr_defines.h"

extern float *mr1_pngmemalloc(Ping *);
extern int mr1_xdrpnghdrv1(Ping *, XDR *);

int
mr1_rdmrfhdr(MR1File *mrf, XDR *xdrs)
/*
   User-callable routine.
   Gets the next MR1File header from the XDR stream pointed
   to by xdrs and returns this header in the structure mrf.
   xdrs is assumed to be positioned at the next header,
   and does not search.
   Returns MR1_SUCCESS or MR1_READ.
*/
{
	int mr1_xdrmrfhdr(MR1File *, XDR *);

	if (mr1_xdrmrfhdr(mrf, xdrs))
		return MR1_SUCCESS;
	else
		return MR1_READ;
}

int
mr1_wrmrfhdr(MR1File *mrf, XDR *xdrs)
/*
   User-callable routine.
   Writes the MR1File header mrf onto the XDR stream pointed to by xdrs.
   Returns MR1_SUCCESS or MR1_WRHEAD.
*/
{
	int mr1_xdrmrfhdr(MR1File *, XDR *);

	if (mr1_xdrmrfhdr(mrf, xdrs))
		return MR1_SUCCESS;
	else
		return MR1_WRITE;
}

int
mr1_rdpnghdr(Ping *png, XDR *xdrs, int version)
/*
   User-callable routine.
   Gets the next Ping header from the XDR stream pointed
   to by xdrs and returns this header in the structure png.
   xdrs is assumed to be positioned at the next header,
   and does not search. version should be the mf_version value
   from the file header which indicates the version of the file.
   Returns MR1_SUCCESS, MR1_BADARG or MR1_READ.
*/
{
	int mr1_xdrpnghdr(Ping *, XDR *);

	switch (version) {
	case MR1_VERSION_1_0:
		if (mr1_xdrpnghdrv1(png, xdrs))
			return MR1_SUCCESS;
		else
			return MR1_READ;
	case MR1_VERSION_2_0:
		if (mr1_xdrpnghdr(png, xdrs))
			return MR1_SUCCESS;
		else
			return MR1_READ;
	default:
		return MR1_BADARG;
	}
}

int
mr1_wrpnghdr(Ping *png, XDR *xdrs)
/*
   User-callable routine.
   Writes the Ping header png onto the XDR stream pointed to by xdrs.
   Returns MR1_SUCCESS or MR1_WRITE.
*/
{
	int mr1_xdrpnghdr(Ping *, XDR *);

	if (mr1_xdrpnghdr(png, xdrs))
		return MR1_SUCCESS;
	else
		return MR1_WRITE;
}

int
mr1_rdpngdata(Ping *png, float *data, XDR *xdrs)
/*
   User-callable routine.
   Reads sidescan data from the XDR stream pointed to by xdrs into
   the memory pointed to by port and stbd. It assumes that
   the stream is positioned correctly (i.e., right after the
   header), and does not search.
   Returns MR1_SUCCESS or MR1_READ.
*/
{
	int mr1_xdrpngdata(Ping *, float *, XDR *);

	if (mr1_xdrpngdata(png, data, xdrs))
		return MR1_SUCCESS;
	else
		return MR1_READ;
}

int
mr1_wrpngdata(Ping *png, float *data, XDR *xdrs)
/*
   User-callable routine.
   Writes sidescan data to the XDR stream pointed to by xdrs.
   Returns MR1_SUCCESS or MR1_WRITE.
*/
{
	int mr1_xdrpngdata(Ping *, float *, XDR *);

	if (mr1_xdrpngdata(png, data, xdrs))
		return MR1_SUCCESS;
	else
		return MR1_WRITE;
}

int
mr1_rdpng(Ping *png, float **data, XDR *xdrs, int version)
/*
   User-callable routine.
   Gets ping header and data from the XDR stream
   pointed to by xdrs. It assumes that the stream is positioned
   at the beginning of the header, and does not search (although
   it does some error checking). Memory for the actual sidescan
   data is allocated. version should be the mf_version value
   from the file header which indicates the version of the file.
   Returns MR1_SUCCESS, MR1_BADARG, MR1_MEMALLOC or MR1_READ.
*/
{
	switch (version) {
	case MR1_VERSION_1_0:
	case MR1_VERSION_2_0:
		break;
	default:
		return MR1_BADARG;
	}

	if (mr1_rdpnghdr(png, xdrs, version) != MR1_SUCCESS)
		return MR1_READ;

	if ((*data= mr1_pngmemalloc(png)) == (float *) 0)
		return MR1_MEMALLOC;

	if (mr1_rdpngdata(png, *data, xdrs) < 0)
		return MR1_READ;

	return MR1_SUCCESS;
}

int
mr1_wrpng(Ping *png, float *data, XDR *xdrs)
/*
   User-callable routine.
   Writes ping header and data to the
   XDR stream pointed to by xdrs.
   Returns MR1_SUCCESS or MR1_WRITE.
*/
{
	if (mr1_wrpnghdr(png, xdrs) != MR1_SUCCESS)
		return MR1_WRITE;

	if (mr1_wrpngdata(png, data, xdrs) != MR1_SUCCESS)
		return MR1_WRITE;

	return MR1_SUCCESS;
}

int
mr1_seekpng(int count, XDR *xdrs, int version)
/*
   User-callable routine.
   Seeks past the next n pings in the XDR stream pointed to by xdrs.
   version should be the mf_version value from the file header which
   indicates the version of the file.
   Returns MR1_SUCCESS, MR1_BADARG or MR1_READ.
*/
{
	Ping png;
	int i, j, n;
	float f;

	switch (version) {
	case MR1_VERSION_1_0:
	case MR1_VERSION_2_0:
		break;
	default:
		return MR1_BADARG;
	}

	for (i= 0; i < count; i++) {
		if (mr1_rdpnghdr(&png, xdrs, version) == MR1_SUCCESS) {
			n= png.png_compass.sns_nsamps+
			   png.png_depth.sns_nsamps+
			   png.png_pitch.sns_nsamps+
			   png.png_roll.sns_nsamps+
			   (2*png.png_sides[ACP_PORT].ps_btycount)+
			   png.png_sides[ACP_PORT].ps_sscount+
			   (2*png.png_sides[ACP_STBD].ps_btycount)+
			   png.png_sides[ACP_STBD].ps_sscount;
			for (j= 0; j < n; j++) {
				if (!xdr_float(xdrs, &f))
					return MR1_READ;
			}
		}

		else
			return MR1_READ;
	}

	return MR1_SUCCESS;
}

int
mr1_seekpngdata(Ping *png, XDR *xdrs)
/*
   User-callable routine.
   Seeks past a single ping data segment
   to the beginning of the next ping header
   in the XDR stream pointed to by xdrs.
   Returns MR1_SUCCESS or MR1_READ.
*/
{
	int i, n;
	float f;

	n= png->png_compass.sns_nsamps+
	   png->png_depth.sns_nsamps+
	   png->png_pitch.sns_nsamps+
	   png->png_roll.sns_nsamps+
	   (2*png->png_sides[ACP_PORT].ps_btycount)+
	   png->png_sides[ACP_PORT].ps_sscount+
	   (2*png->png_sides[ACP_STBD].ps_btycount)+
	   png->png_sides[ACP_STBD].ps_sscount;
	for (i= 0; i < n; i++) {
		if (!xdr_float(xdrs, &f))
			return MR1_READ;
	}

	return MR1_SUCCESS;
}

int
mr1_copypng(int count, XDR *xdris, XDR *xdros, int version)
/*
   User-callable routine.
   Copies the next n pings from the XDR input stream
   pointed to by xdris to the XDR output stream pointed
   to by xdros. version should be the mf_version value
   from the file header which indicates the version of the file.
   Returns MR1_SUCCESS or MR1_READ.
*/
{
	Ping png;
	int i, j, n, err;
	float f;

	for (i= 0; i < count; i++) {
		if (mr1_rdpnghdr(&png, xdris, version) == MR1_SUCCESS) {
			if ((err= mr1_wrpnghdr(&png, xdros)) != MR1_SUCCESS)
				return err;
			n= png.png_compass.sns_nsamps+
			   png.png_depth.sns_nsamps+
			   png.png_pitch.sns_nsamps+
			   png.png_roll.sns_nsamps+
			   (2*png.png_sides[ACP_PORT].ps_btycount)+
			   png.png_sides[ACP_PORT].ps_sscount+
			   (2*png.png_sides[ACP_STBD].ps_btycount)+
			   png.png_sides[ACP_STBD].ps_sscount;
			for (j= 0; j < n; j++) {
				if (!xdr_float(xdris, &f))
					return MR1_READ;
				if (!xdr_float(xdros, &f))
					return MR1_WRITE;
			}
		}

		else
			return MR1_READ;
	}

	return MR1_SUCCESS;
}

int
mr1_xdrmrfhdr(MR1File *mrf, XDR *xdrs)
/*
   Internal routine.
   Does XDR encoding and decoding of an MR1 file header.
   Returns 1 if successful, 0 otherwise.
*/
{
	char **cpp;
	int version;
	int mr1_xdrstring(XDR *, char **);

	switch (xdrs->x_op) {
	case XDR_DECODE:
		if (!xdr_int(xdrs, &mrf->mf_version))
			return 0;
		switch (mrf->mf_version) {
		case MR1_VERSION_1_0:
		case MR1_VERSION_2_0:
			break;
		default:
			return 0;
		}
		break;
	case XDR_ENCODE:
		version= MR1_VERSION_2_0;
		if (!xdr_int(xdrs, &version))
			return 0;
		break;
	default:
		return 0;
	}

	if (!xdr_int(xdrs, &(mrf->mf_count)))
		return 0;

	cpp= &(mrf->mf_log);
	if (!mr1_xdrstring(xdrs, cpp))
		return 0;

	return 1;
}

int
mr1_xdrpnghdr(Ping *png, XDR *xdrs)
/*
   Internal routine.
   Does XDR encoding and decoding of an MR1 ping header.
   Returns 1 if successful, 0 otherwise.
*/
{
	int mr1_xdrside(PingSide *, XDR *);

	if (!xdr_long(xdrs, (long *) &(png->png_tm.tv_sec)))
		return 0;
	if (!xdr_long(xdrs, (long *) &(png->png_tm.tv_usec)))
		return 0;
	if (!xdr_float(xdrs, &(png->png_period)))
		return 0;

	if (!xdr_double(xdrs, &(png->png_slon)))
		return 0;
	if (!xdr_double(xdrs, &(png->png_slat)))
		return 0;
	if (!xdr_float(xdrs, &(png->png_scourse)))
		return 0;
	if (!xdr_float(xdrs, &(png->png_laybackrng)))
		return 0;
	if (!xdr_float(xdrs, &(png->png_laybackbrg)))
		return 0;
	if (!xdr_double(xdrs, &(png->png_tlon)))
		return 0;
	if (!xdr_double(xdrs, &(png->png_tlat)))
		return 0;
	if (!xdr_float(xdrs, &(png->png_tcourse)))
		return 0;

	if (!xdr_float(xdrs, &(png->png_compass.sns_int)))
		return 0;
	if (!xdr_int(xdrs, &(png->png_compass.sns_nsamps)))
		return 0;
	if (!xdr_float(xdrs, &(png->png_compass.sns_repval)))
		return 0;
	if (!xdr_float(xdrs, &(png->png_depth.sns_int)))
		return 0;
	if (!xdr_int(xdrs, &(png->png_depth.sns_nsamps)))
		return 0;
	if (!xdr_float(xdrs, &(png->png_depth.sns_repval)))
		return 0;
	if (!xdr_float(xdrs, &(png->png_pitch.sns_int)))
		return 0;
	if (!xdr_int(xdrs, &(png->png_pitch.sns_nsamps)))
		return 0;
	if (!xdr_float(xdrs, &(png->png_pitch.sns_repval)))
		return 0;
	if (!xdr_float(xdrs, &(png->png_roll.sns_int)))
		return 0;
	if (!xdr_int(xdrs, &(png->png_roll.sns_nsamps)))
		return 0;
	if (!xdr_float(xdrs, &(png->png_roll.sns_repval)))
		return 0;
	if (xdrs->x_op == XDR_DECODE)
		png->png_snspad= 0;

	if (!xdr_float(xdrs, &(png->png_temp)))
		return 0;
	if (!xdr_float(xdrs, &(png->png_atssincr)))
		return 0;
	if (!xdr_float(xdrs, &(png->png_alt)))
		return 0;
	if (!xdr_float(xdrs, &(png->png_magcorr)))
		return 0;
	if (!xdr_float(xdrs, &(png->png_sndvel)))
		return 0;

	if (!mr1_xdrside(&(png->png_sides[ACP_PORT]), xdrs))
		return 0;
	if (!mr1_xdrside(&(png->png_sides[ACP_STBD]), xdrs))
		return 0;

	return 1;
}

int
mr1_xdrside(PingSide *ps, XDR *xdrs)
/*
   Internal routine.
   Does XDR encoding and decoding of a PingSide header.
   Returns 1 if successful, 0 otherwise.
*/
{
	if (!xdr_float(xdrs, &(ps->ps_xmitpwr)))
		return 0;
	if (!xdr_float(xdrs, &(ps->ps_gain)))
		return 0;
	if (!xdr_float(xdrs, &(ps->ps_pulse)))
		return 0;
	if (!xdr_float(xdrs, &(ps->ps_bdrange)))
		return 0;
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

int
mr1_xdrpngdata(Ping *png, float *data, XDR *xdrs)
{
	float *f;
	int i, n;

	f= data;
	n= png->png_compass.sns_nsamps;
	for (i= 0; i < n; i++) {
		if (!xdr_float(xdrs, f++))
			return 0;
	}
	n= png->png_depth.sns_nsamps;
	for (i= 0; i < n; i++) {
		if (!xdr_float(xdrs, f++))
			return 0;
	}
	n= png->png_pitch.sns_nsamps;
	for (i= 0; i < n; i++) {
		if (!xdr_float(xdrs, f++))
			return 0;
	}
	n= png->png_roll.sns_nsamps;
	for (i= 0; i < n; i++) {
		if (!xdr_float(xdrs, f++))
			return 0;
	}

	f+= png->png_snspad;
	n= 2*png->png_sides[ACP_PORT].ps_btycount;
	for (i= 0; i < n; i++) {
		if (!xdr_float(xdrs, f++))
			return 0;
	}

	f+= 2*png->png_sides[ACP_PORT].ps_btypad;
	n= png->png_sides[ACP_PORT].ps_sscount;
	for (i= 0; i < n; i++) {
		if (!xdr_float(xdrs, f++))
			return 0;
	}

	f+= png->png_sides[ACP_PORT].ps_sspad;
	n= 2*png->png_sides[ACP_STBD].ps_btycount;
	for (i= 0; i < n; i++) {
		if (!xdr_float(xdrs, f++))
			return 0;
	}

	f+= 2*png->png_sides[ACP_STBD].ps_btypad;
	n= png->png_sides[ACP_STBD].ps_sscount;
	for (i= 0; i < n; i++) {
		if (!xdr_float(xdrs, f++))
			return 0;
	}

	return 1;
}

int
mr1_xdrstring(XDR *xdrs, char **cpp)
/*
   Internal routine.
   Does XDR encoding and decoding of character strings.
   These are stored as an integer (the string length) followed
   by the bytes of the string (if the length is greater than 0).
   Returns 1 if successful, 0 otherwise.
*/
{
	int len;
	u_int ul;

	switch (xdrs->x_op) {
	case XDR_ENCODE:
		if ((*cpp == (char *) 0) || ((int) strlen(*cpp) == 0)) {
			len= 0;
			if (!xdr_int(xdrs, &len))
				return 0;
		}
		else {
			len= (int) strlen(*cpp);
			if (!xdr_int(xdrs, &len))
				return 0;
			ul= (u_int) len;
			if (!xdr_bytes(xdrs, cpp, &ul, (u_int) len))
				return 0;
		}
		break;
	case XDR_DECODE:
		if (!xdr_int(xdrs, &len))
			return 0;
		if (len == 0)
			*cpp= (char *) 0;
		else if (len < 0)
			return 0;
		else {
			if ((*cpp= (char *) calloc((MemSizeType) (len+1), sizeof(char))) == (char *) 0)
				return 0;
			ul= (u_int) len;
			if (!xdr_bytes(xdrs, cpp, &ul, (u_int) len))
				return 0;
			if (ul != (u_int) len)
				return 0;
		}
		break;
	}

	return 1;
}
