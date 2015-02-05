/*--------------------------------------------------------------------
 *    The MB-system:	mbbs_io.c	3/3/2014
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
   I/O routines for Hawaii Mapping Research Group BS files.
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "mbbs_defines.h"
#include "mbbs_mem.h"

extern int	mbbs_getpngdataptrs(Ping *, MemType *, PingData *);
extern double	mbbs_nand();
extern float	mbbs_nanf();
extern MemType	*mbbs_pngmemalloc(Ping *);
extern int	mbbs_mr1_xdrpnghdrv1(Ping *, XDR *);
extern int	mbbs_mr1_xdrpnghdrv2(Ping *, XDR *);

unsigned long bs_iobytecnt;
int bs_ionaninit= 0;
float bs_ionanf;
double bs_ionand;

static unsigned char *ssflagbuf= (unsigned char *) 0;
static unsigned int ssflagbufsz= 0;

int
mbbs_rdbsfhdr(BSFile *bsf, XDR *xdrs)
/*
   User-callable routine.
   Gets the next BSFile header from the XDR stream pointed
   to by xdrs and returns this header in the structure bsf.
   xdrs is assumed to be positioned at the next header,
   and does not search.
   Returns BS_SUCCESS, BS_BADARCH or BS_READ.
*/
{
	int mbbs_xdrbsfhdr(BSFile *, XDR *);

	if (sizeof(int) < 4)
		return BS_BADARCH;

	if (mbbs_xdrbsfhdr(bsf, xdrs))
		return BS_SUCCESS;
	else
		return BS_READ;
}

int
mbbs_wrbsfhdr(BSFile *bsf, XDR *xdrs)
/*
   User-callable routine.
   Writes the BSFile header bsf onto the XDR stream pointed to by xdrs.
   Returns BS_SUCCESS, BS_BADARCH or BS_READ.
*/
{
	int mbbs_xdrbsfhdr(BSFile *, XDR *);

	if (sizeof(int) < 4)
		return BS_BADARCH;

	if (mbbs_xdrbsfhdr(bsf, xdrs))
		return BS_SUCCESS;
	else
		return BS_WRITE;
}

int
mbbs_freebsfmem(BSFile *bsf)
{
	if (bsf == (BSFile *) 0)
		return BS_BADARG;

	if (bsf->bsf_srcfilenm != (char *) 0) {
		CFree((MemType *) bsf->bsf_srcfilenm);
		bsf->bsf_srcfilenm= (char *) 0;
	}
	if (bsf->bsf_log != (char *) 0) {
		CFree((MemType *) bsf->bsf_log);
		bsf->bsf_log= (char *) 0;
	}

	return BS_SUCCESS;
}

int
mbbs_rdpnghdr(Ping *png, XDR *xdrs, int version)
/*
   User-callable routine.
   Gets the next Ping header from the XDR stream pointed
   to by xdrs and returns this header in the structure png.
   xdrs is assumed to be positioned at the next header,
   and does not search. version should be the bs_version value
   from the file header which indicates the version of the file.
   Returns BS_SUCCESS, BS_BADARCH, BS_BADARG or BS_READ.
*/
{
	int mbbs_xdrpnghdr(Ping *, XDR *, int);

	if (sizeof(int) < 4)
		return BS_BADARCH;

	switch (version) {
	case MR1_VERSION_1_0:
		if (mbbs_mr1_xdrpnghdrv1(png, xdrs))
			return BS_SUCCESS;
		else
			return BS_READ;
	case MR1_VERSION_2_0:
		if (mbbs_mr1_xdrpnghdrv2(png, xdrs))
			return BS_SUCCESS;
		else
			return BS_READ;
	case BS_VERSION_1_0:
	case BS_VERSION_1_1:
	case BS_VERSION_1_2:
	case BS_VERSION_1_3:
	case BS_VERSION_1_4:
		if (mbbs_xdrpnghdr(png, xdrs, version))
			return BS_SUCCESS;
		else
			return BS_READ;
	default:
		return BS_BADARG;
	}
}

int
mbbs_wrpnghdr(Ping *png, XDR *xdrs)
/*
   User-callable routine.
   Writes the Ping header png onto the XDR stream pointed to by xdrs.
   Returns BS_SUCCESS, BS_BADARCH or BS_WRITE.
*/
{
	int mbbs_xdrpnghdr(Ping *, XDR *, int);

	if (sizeof(int) < 4)
		return BS_BADARCH;

	if (mbbs_xdrpnghdr(png, xdrs, (int) BS_VERSION_CURR))
		return BS_SUCCESS;
	else
		return BS_WRITE;
}

int
mbbs_rdpngdata(Ping *png, MemType *data, XDR *xdrs)
/*
   User-callable routine.
   Reads sample data from the XDR stream pointed to by xdrs into
   the memory pointed to by data argument. It assumes that
   the stream is positioned correctly (i.e., right after the
   header), and does not search.
   Returns BS_SUCCESS, BS_BADARCH or BS_READ.
*/
{
	int mbbs_xdrpngdata(Ping *, MemType *, XDR *);

	if (sizeof(int) < 4)
		return BS_BADARCH;

	if (mbbs_xdrpngdata(png, data, xdrs))
		return BS_SUCCESS;
	else
		return BS_READ;
}

int
mbbs_wrpngdata(Ping *png, MemType *data, XDR *xdrs)
/*
   User-callable routine.
   Writes sample data to the XDR stream pointed to by xdrs.
   Returns BS_SUCCESS, BS_BADARCH or BS_WRITE.
*/
{
	int mbbs_xdrpngdata(Ping *, MemType *, XDR *);

	if (sizeof(int) < 4)
		return BS_BADARCH;

	if (mbbs_xdrpngdata(png, data, xdrs))
		return BS_SUCCESS;
	else
		return BS_WRITE;
}

int
mbbs_rdpngpddata(Ping *png, PingData *pddata, XDR *xdrs)
/*
   User-callable routine.
   Reads sample data from the XDR stream pointed to by xdrs.
   Returns BS_SUCCESS, BS_BADARCH or BS_WRITE.
*/
{
	int mbbs_xdrpngpddata(Ping *, PingData *, XDR *);

	if (sizeof(int) < 4)
		return BS_BADARCH;

	if (mbbs_xdrpngpddata(png, pddata, xdrs))
		return BS_SUCCESS;
	else
		return BS_WRITE;
}

int
mbbs_wrpngpddata(Ping *png, PingData *pddata, XDR *xdrs)
/*
   User-callable routine.
   Writes sample data to the XDR stream pointed to by xdrs.
   Returns BS_SUCCESS, BS_BADARCH or BS_WRITE.
*/
{
	int mbbs_xdrpngpddata(Ping *, PingData *, XDR *);

	if (sizeof(int) < 4)
		return BS_BADARCH;

	if (mbbs_xdrpngpddata(png, pddata, xdrs))
		return BS_SUCCESS;
	else
		return BS_WRITE;
}

int
mbbs_rdpng(Ping *png, MemType **data, XDR *xdrs, int version)
/*
   User-callable routine.
   Gets ping header and data from the XDR stream
   pointed to by xdrs. It assumes that the stream is positioned
   at the beginning of the header, and does not search (although
   it does some error checking). Memory for the actual sample
   data is allocated. version should be the bs_version value
   from the file header which indicates the version of the file.
   Returns BS_SUCCESS, BS_BADARCH, BS_BADARG, BS_MEMALLOC or BS_READ.
*/
{
	unsigned long ibcsv;

	if (sizeof(int) < 4)
		return BS_BADARCH;

	bs_iobytecnt= 0;

	switch (version) {
	case MR1_VERSION_1_0:
	case MR1_VERSION_2_0:
	case BS_VERSION_1_0:
	case BS_VERSION_1_1:
	case BS_VERSION_1_2:
	case BS_VERSION_1_3:
	case BS_VERSION_1_4:
		break;
	default:
		return BS_BADARG;
	}

	if (mbbs_rdpnghdr(png, xdrs, version) != BS_SUCCESS)
		return BS_READ;

	if ((*data= mbbs_pngmemalloc(png)) == (MemType *) 0)
		return BS_MEMALLOC;

	ibcsv= bs_iobytecnt;
	if (mbbs_rdpngdata(png, *data, xdrs) < 0)
		return BS_READ;
	bs_iobytecnt+= ibcsv;

	return BS_SUCCESS;
}

int
mbbs_wrpng(Ping *png, MemType *data, XDR *xdrs)
/*
   User-callable routine.
   Writes ping header and data to the
   XDR stream pointed to by xdrs.
   Returns BS_SUCCESS, BS_BADARCH or BS_WRITE.
*/
{
	unsigned long ibcsv;

	if (sizeof(int) < 4)
		return BS_BADARCH;

	bs_iobytecnt= 0;

	if (mbbs_wrpnghdr(png, xdrs) != BS_SUCCESS)
		return BS_WRITE;

	ibcsv= bs_iobytecnt;
	if (mbbs_wrpngdata(png, data, xdrs) != BS_SUCCESS)
		return BS_WRITE;
	bs_iobytecnt+= ibcsv;

	return BS_SUCCESS;
}

int
mbbs_seekpng(int count, XDR *xdrs, int version)
/*
   User-callable routine.
   Seeks past the next n pings in the XDR stream pointed to by xdrs.
   version should be the bs_version value from the file header which
   indicates the version of the file.
   Returns BS_SUCCESS, BS_BADARCH, BS_BADARG or BS_READ.
*/
{
	Ping png;
	int i, j, err, bsi, side, n, ii, rem;
	unsigned int ui;
	u_int ui1;
	float f;
	char *cp;
	unsigned long ibcsv;

	if (sizeof(int) < 4)
		return BS_BADARCH;

	bs_iobytecnt= 0;

	switch (version) {
	case MR1_VERSION_1_0:
	case MR1_VERSION_2_0:
	case BS_VERSION_1_0:
	case BS_VERSION_1_1:
	case BS_VERSION_1_2:
	case BS_VERSION_1_3:
	case BS_VERSION_1_4:
		break;
	default:
		return BS_BADARG;
	}

	for (i= 0; i < count; i++) {
		ibcsv= bs_iobytecnt;
		if ((err= mbbs_rdpnghdr(&png, xdrs, version)) != BS_SUCCESS)
			return err;
		bs_iobytecnt+= ibcsv;

		/* sensor data */
		n= png.png_compass.sns_nsamps+
		   png.png_depth.sns_nsamps+
		   png.png_pitch.sns_nsamps+
		   png.png_roll.sns_nsamps;
		for (j= 0; j < n; j++) {
			if (!xdr_float(xdrs, &f))
				return BS_READ;
			bs_iobytecnt+= 4;
		}

		if (png.png_flags & PNG_XYZ)
			bsi= 3;
		else
			bsi= 2;

		for (side= ACP_PORT; side < ACP_NSIDES; side++) {

			/* bathymetry samples */
			n= bsi*png.png_sides[side].ps_btycount;
			for (j= 0; j < n; j++) {
				if (!xdr_float(xdrs, &f))
					return BS_READ;
				bs_iobytecnt+= 4;
			}

			/* bathymetry flags */
			if (!(png.png_flags & PNG_BTYSSFLAGSABSENT)) {
				n= png.png_sides[side].ps_btycount;
				for (j= 0; j < n; j++) {
					if (!xdr_u_int(xdrs, &ui))
						return BS_READ;
					bs_iobytecnt+= 4;
				}
			}

			if ((n= png.png_sides[side].ps_sscount) > 0) {

				/* sidescan samples */
				for (j= 0; j < n; j++) {
					if (!xdr_float(xdrs, &f))
						return BS_READ;
					bs_iobytecnt+= 4;
				}

				/* sidescan flags */
				if (!(png.png_flags & PNG_BTYSSFLAGSABSENT)) {
					if ((err= mbbs_memalloc((MemType **) &ssflagbuf, &ssflagbufsz, (unsigned int) n, (size_t) 1)) != MEM_SUCCESS) {
						switch (err) {
						case MEM_BADARG:
							return BS_BADARG;
						case MEM_OOB:
							return BS_BADARG;
						case MEM_CALLOC:
							return BS_MEMALLOC;
						default:
							return BS_FAILURE;
						}
					}
					cp= (char *) ssflagbuf;
					ui1= (u_int) n;
					if (!xdr_bytes(xdrs, &cp, &ui1, (u_int) n))
						return BS_READ;
					bs_iobytecnt+= 4+n;
					if ((rem= bs_iobytecnt%4) > 0)
						bs_iobytecnt+= 4-rem;
				}
			}
		}

		/* auxiliary beam info */
		if (png.png_flags & PNG_ABI) {
			for (side= ACP_PORT; side < ACP_NSIDES; side++) {
				n= png.png_sides[side].ps_btycount;
				for (j= 0; j < n; j++) {
					if (!xdr_u_int(xdrs, &ui))
						return BS_READ;
					bs_iobytecnt+= 4;
					if (!xdr_int(xdrs, &ii))
						return BS_READ;
					bs_iobytecnt+= 4;
					if (!xdr_float(xdrs, &f))
						return BS_READ;
					bs_iobytecnt+= 4;
					if (!xdr_float(xdrs, &f))
						return BS_READ;
					bs_iobytecnt+= 4;
				}
			}
		}
	}

	return BS_SUCCESS;
}

int
mbbs_seekpngdata(Ping *png, XDR *xdrs)
/*
   User-callable routine.
   Seeks past a single ping data segment
   to the beginning of the next ping header
   in the XDR stream pointed to by xdrs.
   Returns BS_SUCCESS, BS_BADARCH or BS_READ.
*/
{
	int j, bsi, side, err, n, ii, rem;
	unsigned int ui;
	u_int ui1;
	float f;
	char *cp;

	if (sizeof(int) < 4)
		return BS_BADARCH;

	bs_iobytecnt= 0;

	/* sensor data */
	n= png->png_compass.sns_nsamps+
	   png->png_depth.sns_nsamps+
	   png->png_pitch.sns_nsamps+
	   png->png_roll.sns_nsamps;
	for (j= 0; j < n; j++) {
		if (!xdr_float(xdrs, &f))
			return BS_READ;
		bs_iobytecnt+= 4;
	}

	if (png->png_flags & PNG_XYZ)
		bsi= 3;
	else
		bsi= 2;

	for (side= ACP_PORT; side < ACP_NSIDES; side++) {

		/* bathymetry samples */
		n= bsi*png->png_sides[side].ps_btycount;
		for (j= 0; j < n; j++) {
			if (!xdr_float(xdrs, &f))
				return BS_READ;
			bs_iobytecnt+= 4;
		}

		/* bathymetry flags */
		if (!(png->png_flags & PNG_BTYSSFLAGSABSENT)) {
			n= png->png_sides[side].ps_btycount;
			for (j= 0; j < n; j++) {
				if (!xdr_u_int(xdrs, &ui))
					return BS_READ;
				bs_iobytecnt+= 4;
			}
		}

		if ((n= png->png_sides[side].ps_sscount) > 0) {

			/* sidescan samples */
			for (j= 0; j < n; j++) {
				if (!xdr_float(xdrs, &f))
					return BS_READ;
				bs_iobytecnt+= 4;
			}

			/* sidescan flags */
			if (!(png->png_flags & PNG_BTYSSFLAGSABSENT)) {
				if ((err= mbbs_memalloc((MemType **) &ssflagbuf, &ssflagbufsz, (unsigned int) n, (size_t) 1)) != MEM_SUCCESS) {
					switch (err) {
					case MEM_BADARG:
						return BS_BADARG;
					case MEM_OOB:
						return BS_BADARG;
					case MEM_CALLOC:
						return BS_MEMALLOC;
					default:
						return BS_FAILURE;
					}
				}
				cp= (char *) ssflagbuf;
				ui1= (u_int) n;
				if (!xdr_bytes(xdrs, &cp, &ui1, (u_int) n))
					return BS_READ;
				bs_iobytecnt+= 4+n;
				if ((rem= bs_iobytecnt%4) > 0)
					bs_iobytecnt+= 4-rem;
			}
		}
	}

	/* auxiliary beam info */
	if (png->png_flags & PNG_ABI) {
		for (side= ACP_PORT; side < ACP_NSIDES; side++) {
			n= png->png_sides[side].ps_btycount;
			for (j= 0; j < n; j++) {
				if (!xdr_u_int(xdrs, &ui))
					return BS_READ;
				bs_iobytecnt+= 4;
				if (!xdr_int(xdrs, &ii))
					return BS_READ;
				bs_iobytecnt+= 4;
				if (!xdr_float(xdrs, &f))
					return BS_READ;
				bs_iobytecnt+= 4;
				if (!xdr_float(xdrs, &f))
					return BS_READ;
				bs_iobytecnt+= 4;
			}
		}
	}

	return BS_SUCCESS;
}

int
mbbs_copypng(int count, XDR *xdris, XDR *xdros, int version)
/*
   User-callable routine.
   Copies the next n pings from the XDR input stream
   pointed to by xdris to the XDR output stream pointed
   to by xdros. version should be the bs_version value
   from the file header which indicates the version of the file.
   Note that this routine will set bs_iobytecnt to the count
   of bytes written, not bytes read!
   Returns BS_SUCCESS, BS_BADARCH, BS_BADARG, BS_READ or BS_WRITE.
*/
{
	Ping png;
	int i, j, bsi, side, n, err, ii, rem;
	unsigned int flags, ui;
	u_int ui1;
	float f;
	char *cp;
	unsigned int ibcsv;

	if (sizeof(int) < 4)
		return BS_BADARCH;

	bs_iobytecnt= 0;

	for (i= 0; i < count; i++) {

		/* note that we want bs_iobytecnt to be a count
		   of the bytes written, not the bytes read! */
		ibcsv= bs_iobytecnt;
		if ((err= mbbs_rdpnghdr(&png, xdris, version)) != BS_SUCCESS)
			return err;
		bs_iobytecnt= ibcsv;

		flags= png.png_flags;
		png.png_flags&= ~PNG_BTYSSFLAGSABSENT;
		ibcsv= bs_iobytecnt;
		if ((err= mbbs_wrpnghdr(&png, xdros)) != BS_SUCCESS)
			return err;
		bs_iobytecnt+= ibcsv;

		/* sensor data */
		n= png.png_compass.sns_nsamps+
		   png.png_depth.sns_nsamps+
		   png.png_pitch.sns_nsamps+
		   png.png_roll.sns_nsamps;
		for (j= 0; j < n; j++) {
			if (!xdr_float(xdris, &f))
				return BS_READ;
			if (!xdr_float(xdros, &f))
				return BS_WRITE;
			bs_iobytecnt+= 4;
		}

		if (png.png_flags & PNG_XYZ)
			bsi= 3;
		else
			bsi= 2;

		for (side= ACP_PORT; side < ACP_NSIDES; side++) {

			/* bathymetry samples */
			n= bsi*png.png_sides[side].ps_btycount;
			for (j= 0; j < n; j++) {
				if (!xdr_float(xdris, &f))
					return BS_READ;
				if (!xdr_float(xdros, &f))
					return BS_WRITE;
				bs_iobytecnt+= 4;
			}

			/* bathymetry flags */
			n= png.png_sides[side].ps_btycount;
			for (j= 0; j < n; j++) {
				if (!(flags & PNG_BTYSSFLAGSABSENT)) {
					if (!xdr_u_int(xdris, &ui))
						return BS_READ;
				}
				else
					ui= BTYD_CLEAR;
				if (!xdr_u_int(xdros, &ui))
					return BS_WRITE;
				bs_iobytecnt+= 4;
			}

			if ((n= png.png_sides[side].ps_sscount) > 0) {

				/* sidescan samples */
				for (j= 0; j < n; j++) {
					if (!xdr_float(xdris, &f))
						return BS_READ;
					if (!xdr_float(xdros, &f))
						return BS_WRITE;
					bs_iobytecnt+= 4;
				}

				/* sidescan flags */
				if ((err= mbbs_memalloc((MemType **) &ssflagbuf, &ssflagbufsz, (unsigned int) n, (size_t) 1)) != MEM_SUCCESS) {
					switch (err) {
					case MEM_BADARG:
						return BS_BADARG;
					case MEM_OOB:
						return BS_BADARG;
					case MEM_CALLOC:
						return BS_MEMALLOC;
					default:
						return BS_FAILURE;
					}
				}
				cp= (char *) ssflagbuf;
				ui1= (u_int) n;
				if (!(flags & PNG_BTYSSFLAGSABSENT)) {
					if (!xdr_bytes(xdris, &cp, &ui1, (u_int) n))
						return BS_READ;
				}
				else {
					for (j= 0; j < n; j++)
						ssflagbuf[j]= SSD_CLEAR;
				}
				if (!xdr_bytes(xdros, &cp, &ui1, (u_int) n))
					return BS_READ;
				bs_iobytecnt+= 4+n;
				if ((rem= bs_iobytecnt%4) > 0)
					bs_iobytecnt+= 4-rem;
			}
		}

		/* auxiliary beam info */
		if (png.png_flags & PNG_ABI) {
			for (side= ACP_PORT; side < ACP_NSIDES; side++) {
				n= png.png_sides[side].ps_btycount;
				for (j= 0; j < n; j++) {
					if (!xdr_u_int(xdris, &ui))
						return BS_READ;
					if (!xdr_u_int(xdros, &ui))
						return BS_WRITE;
					bs_iobytecnt+= 4;
					if (!xdr_int(xdris, &ii))
						return BS_READ;
					if (!xdr_int(xdros, &ii))
						return BS_WRITE;
					bs_iobytecnt+= 4;
					if (!xdr_float(xdris, &f))
						return BS_READ;
					if (!xdr_float(xdros, &f))
						return BS_WRITE;
					bs_iobytecnt+= 4;
					if (!xdr_float(xdris, &f))
						return BS_READ;
					if (!xdr_float(xdros, &f))
						return BS_WRITE;
					bs_iobytecnt+= 4;
				}
			}
		}
	}

	return BS_SUCCESS;
}

int
mbbs_xdrbsfhdr(BSFile *bsf, XDR *xdrs)
/*
   Internal routine.
   Does XDR encoding and decoding of an BS file header.
   Returns 1 if successful, 0 otherwise.
*/
{
	char **cpp;
	int version;
	unsigned long strbc;
	int mbbs_xdrstring(XDR *, char **, unsigned long *);

	bs_iobytecnt= 0;

	switch (xdrs->x_op) {
	case XDR_DECODE:
		if (!xdr_int(xdrs, &(bsf->bsf_version)))
			return 0;
		bs_iobytecnt+= 4;
		switch (bsf->bsf_version) {
		case MR1_VERSION_1_0:
		case MR1_VERSION_2_0:
		case BS_VERSION_1_0:
		case BS_VERSION_1_1:
		case BS_VERSION_1_2:
		case BS_VERSION_1_3:
		case BS_VERSION_1_4:
			break;
		default:
			return 0;
		}
		break;
	case XDR_ENCODE:
		version= BS_VERSION_CURR;
		if (!xdr_int(xdrs, &version))
			return 0;
		bs_iobytecnt+= 4;
		break;
	default:
		return 0;
	}

	if (!xdr_int(xdrs, &(bsf->bsf_count)))
		return 0;
	bs_iobytecnt+= 4;

	switch (xdrs->x_op) {
	case XDR_DECODE:
		switch (bsf->bsf_version) {
		case MR1_VERSION_1_0:
		case MR1_VERSION_2_0:
		case BS_VERSION_1_0:
		case BS_VERSION_1_1:
		case BS_VERSION_1_2:
			bsf->bsf_flags= BS_CLEAR;
			break;
		default:
			if (!xdr_u_int(xdrs, &(bsf->bsf_flags)))
				return 0;
			bs_iobytecnt+= 4;
		}
		break;
	case XDR_ENCODE:
		if (!xdr_u_int(xdrs, &(bsf->bsf_flags)))
			return 0;
		bs_iobytecnt+= 4;
		break;
	default:	/* shut up compiler warnings */
		break;
	}

	switch (xdrs->x_op) {
	case XDR_DECODE:
		switch (bsf->bsf_version) {
		case MR1_VERSION_1_0:
		case MR1_VERSION_2_0:
			bsf->bsf_inst= BS_INST_UNDEFINED;
			bsf->bsf_srcformat= BS_SFMT_UNDEFINED;
			bsf->bsf_srcfilenm= (char *) 0;
			break;
		default:
			if (!xdr_int(xdrs, &(bsf->bsf_inst)))
				return 0;
			bs_iobytecnt+= 4;
			if (!xdr_int(xdrs, &(bsf->bsf_srcformat)))
				return 0;
			bs_iobytecnt+= 4;
			cpp= &(bsf->bsf_srcfilenm);
			if (!mbbs_xdrstring(xdrs, cpp, &strbc))
				return 0;
			bs_iobytecnt+= strbc;
			break;
		}
		break;
	case XDR_ENCODE:
		if (!xdr_int(xdrs, &(bsf->bsf_inst)))
			return 0;
		bs_iobytecnt+= 4;
		if (!xdr_int(xdrs, &(bsf->bsf_srcformat)))
			return 0;
		bs_iobytecnt+= 4;
		cpp= &(bsf->bsf_srcfilenm);
		if (!mbbs_xdrstring(xdrs, cpp, &strbc))
			return 0;
		bs_iobytecnt+= strbc;
		break;
	default:
		return 0;
	}

	cpp= &(bsf->bsf_log);
	if (!mbbs_xdrstring(xdrs, cpp, &strbc))
		return 0;
	bs_iobytecnt+= strbc;

	return 1;
}

int
mbbs_xdrpnghdr(Ping *png, XDR *xdrs, int version)
/*
   Internal routine.
   Does XDR encoding and decoding of a BS ping header.
   Returns 1 if successful, 0 otherwise.
*/
{
	unsigned int flags;
	int tvsec, tvusec;
	unsigned long sidebc;
	int mbbs_xdrside(PingSide *, XDR *, int, unsigned long *);

	switch (version) {
	case BS_VERSION_1_0:
	case BS_VERSION_1_1:
	case BS_VERSION_1_2:
	case BS_VERSION_1_3:

		/* output in obsolete format not allowed! */
		if (xdrs->x_op == XDR_ENCODE)
			return 0;

		break;
	case BS_VERSION_1_4:
		break;
	default:
		return 0;
	}

	bs_iobytecnt= 0;

	if (!bs_ionaninit) {
		bs_ionanf= mbbs_nanf();
		bs_ionand= mbbs_nand();
		bs_ionaninit= 1;
	}

	/* always clear the PNG_BTYSSFLAGSABSENT bit when writing
	   since all current format output files are guaranteed to
	   include bathymetry and sidescan flags -- this bit should
	   normally be set only by I/O functions from this library
	   when reading flagless older format version files */	
	if (xdrs->x_op == XDR_ENCODE) {
		flags= png->png_flags;
		flags&= ~PNG_BTYSSFLAGSABSENT;
	}
	if (!xdr_u_int(xdrs, &flags))
		return 0;
	bs_iobytecnt+= 4;
	if (xdrs->x_op == XDR_DECODE)
		png->png_flags= flags;

	/* depending upon the platform, the size of the timeval
	   struct's fields may be 4 or 8 bytes; for backward
	   compatibility with old files that use 4-byte fields,
	   we force these quantities into 4-byte primitives when
	   doing output, returning an error when overflow would result */
	if (xdrs->x_op == XDR_ENCODE) {
		if (((long) png->png_tm.tv_sec < INT32_MIN) ||
		    ((long) png->png_tm.tv_sec > INT32_MAX))
			return 0;

		/* on some platforms where tv_usec is 4 bytes (e.g., mac_x8664
		   under MacOS 10.7) the following may generate a compiler
		   warning about an invalid comparison -- it's probably best to
		   leave this as is rather than code around this unusual size
		   and risk failure in the future if it's ever increased */
		if ((sizeof png->png_tm.tv_usec > 4) &&
		    (((long) png->png_tm.tv_usec < INT32_MIN) ||
		     ((long) png->png_tm.tv_usec > INT32_MAX)))
			return 0;

		tvsec= (int) png->png_tm.tv_sec;
		tvusec= (int) png->png_tm.tv_usec;
	}
	if (!xdr_int(xdrs, &tvsec))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_int(xdrs, &tvusec))
		return 0;
	bs_iobytecnt+= 4;
	if (xdrs->x_op == XDR_DECODE) {
		png->png_tm.tv_sec= tvsec;
		png->png_tm.tv_usec= tvusec;
	}

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
	if (png->png_compass.sns_nsamps < 0)
		return 0;
	if (!xdr_float(xdrs, &(png->png_compass.sns_repval)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_float(xdrs, &(png->png_depth.sns_int)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_int(xdrs, &(png->png_depth.sns_nsamps)))
		return 0;
	bs_iobytecnt+= 4;
	if (png->png_depth.sns_nsamps < 0)
		return 0;
	if (!xdr_float(xdrs, &(png->png_depth.sns_repval)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_float(xdrs, &(png->png_pitch.sns_int)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_int(xdrs, &(png->png_pitch.sns_nsamps)))
		return 0;
	bs_iobytecnt+= 4;
	if (png->png_pitch.sns_nsamps < 0)
		return 0;
	if (!xdr_float(xdrs, &(png->png_pitch.sns_repval)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_float(xdrs, &(png->png_roll.sns_int)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_int(xdrs, &(png->png_roll.sns_nsamps)))
		return 0;
	bs_iobytecnt+= 4;
	if (png->png_roll.sns_nsamps < 0)
		return 0;
	if (!xdr_float(xdrs, &(png->png_roll.sns_repval)))
		return 0;
	bs_iobytecnt+= 4;
	if (xdrs->x_op == XDR_DECODE)
		png->png_snspad= 0;

	if (!xdr_float(xdrs, &(png->png_temp)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_float(xdrs, &(png->png_ssincr)))
		return 0;
	bs_iobytecnt+= 4;
	if (version >= BS_VERSION_1_4) {
		if (!xdr_int(xdrs, &(png->png_ssyoffsetmode)))
			return 0;
		bs_iobytecnt+= 4;
	}
	else {
		if (xdrs->x_op == XDR_DECODE)
			png->png_ssyoffsetmode= PNG_SSYOM_UNKNOWN;
	}
	if (!xdr_float(xdrs, &(png->png_alt)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_float(xdrs, &(png->png_magcorr)))
		return 0;
	bs_iobytecnt+= 4;
	if (!xdr_float(xdrs, &(png->png_sndvel)))
		return 0;
	bs_iobytecnt+= 4;
	if (version >= BS_VERSION_1_1) {
		if (!xdr_float(xdrs, &(png->png_cond)))
			return 0;
		bs_iobytecnt+= 4;
		if (!xdr_float(xdrs, &(png->png_magx)))
			return 0;
		bs_iobytecnt+= 4;
		if (!xdr_float(xdrs, &(png->png_magy)))
			return 0;
		bs_iobytecnt+= 4;
		if (!xdr_float(xdrs, &(png->png_magz)))
			return 0;
		bs_iobytecnt+= 4;
	}
	else {
		if (xdrs->x_op == XDR_DECODE) {
			png->png_cond= bs_ionanf;
			png->png_magx= bs_ionanf;
			png->png_magy= bs_ionanf;
			png->png_magz= bs_ionanf;
		}
	}

	if (!mbbs_xdrside(&(png->png_sides[ACP_PORT]), xdrs, version, &sidebc))
		return 0;
	bs_iobytecnt+= sidebc;
	if (!mbbs_xdrside(&(png->png_sides[ACP_STBD]), xdrs, version, &sidebc))
		return 0;
	bs_iobytecnt+= sidebc;

	return 1;
}

int
mbbs_xdrside(PingSide *ps, XDR *xdrs, int version, unsigned long *bytecnt)
/*
   Internal routine.
   Does XDR encoding and decoding of a PingSide header.
   Records the total number of bytes transferred into *bytecnt.
   Returns 1 if successful, 0 otherwise.
*/
{
	*bytecnt= 0;

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
	if (ps->ps_btycount < 0)
		return 0;
	if (xdrs->x_op == XDR_DECODE)
		ps->ps_btypad= 0;
	if (!xdr_float(xdrs, &(ps->ps_ssxoffset)))
		return 0;
	*bytecnt+= 4;
	if (!xdr_int(xdrs, &(ps->ps_sscount)))
		return 0;
	*bytecnt+= 4;
	if (ps->ps_sscount < 0)
		return 0;
	if (xdrs->x_op == XDR_DECODE)
		ps->ps_sspad= 0;
	if (version >= BS_VERSION_1_2) {
		if (!xdr_float(xdrs, &(ps->ps_ssndrmask)))
			return 0;
		*bytecnt+= 4;
	}
	else {
		if (xdrs->x_op == XDR_DECODE)
			ps->ps_ssndrmask= 0.;
	}
	if (version >= BS_VERSION_1_4) {
		if (!xdr_float(xdrs, &(ps->ps_ssyoffset)))
			return 0;
		bs_iobytecnt+= 4;
	}
	else {
		if (xdrs->x_op == XDR_DECODE)
			ps->ps_ssyoffset= bs_ionanf;
	}

	return 1;
}

int
mbbs_xdrpngdata(Ping *png, MemType *data, XDR *xdrs)
/*
   Internal routine.
*/
{
	PingData pd;
	int mbbs_xdrpngpddata(Ping *, PingData *, XDR *);

	if (mbbs_getpngdataptrs(png, data, &pd) != BS_SUCCESS)
		return 0;

	return mbbs_xdrpngpddata(png, &pd, xdrs);
}

int
mbbs_xdrpngpddata(Ping *png, PingData *pddata, XDR *xdrs)
/*
   Internal routine.
*/
{
	float *fp;
	int i, side, n, bsi, rem;
	unsigned int *uip;
	unsigned char *ucp;
	char *cp;
	u_int ui1;
	AuxBeamInfo *abi;

	bs_iobytecnt= 0;

	/* compass */
	n= png->png_compass.sns_nsamps;
	fp= pddata->pd_compass;
	if ((n > 0) && (fp == (float *) 0))
		return 0;
	for (i= 0; i < n; i++) {
		if (!xdr_float(xdrs, fp++))
			return 0;
		bs_iobytecnt+= 4;
	}

	/* depth */
	n= png->png_depth.sns_nsamps;
	fp= pddata->pd_depth;
	if ((n > 0) && (fp == (float *) 0))
		return 0;
	for (i= 0; i < n; i++) {
		if (!xdr_float(xdrs, fp++))
			return 0;
		bs_iobytecnt+= 4;
	}

	/* pitch */
	n= png->png_pitch.sns_nsamps;
	fp= pddata->pd_pitch;
	if ((n > 0) && (fp == (float *) 0))
		return 0;
	for (i= 0; i < n; i++) {
		if (!xdr_float(xdrs, fp++))
			return 0;
		bs_iobytecnt+= 4;
	}

	/* roll */
	n= png->png_roll.sns_nsamps;
	fp= pddata->pd_roll;
	if ((n > 0) && (fp == (float *) 0))
		return 0;
	for (i= 0; i < n; i++) {
		if (!xdr_float(xdrs, fp++))
			return 0;
		bs_iobytecnt+= 4;
	}

	if (png->png_flags & PNG_XYZ)
		bsi= 3;
	else
		bsi= 2;
	for (side= ACP_PORT; side < ACP_NSIDES; side++) {

		/* bathymetry */
		n= bsi*png->png_sides[side].ps_btycount;
		/* watch for overflow! */
		if (n < 0)
			return 0;
		fp= pddata->pd_bty[side];
		if ((n > 0) && (fp == (float *) 0))
			return 0;
		for (i= 0; i < n; i++) {
			if (!xdr_float(xdrs, fp++))
				return 0;
			bs_iobytecnt+= 4;
		}

		/* bathymetry flags */
		n= png->png_sides[side].ps_btycount;
		uip= pddata->pd_btyflags[side];
		if ((n > 0) && (uip == (unsigned int *) 0))
			return 0;
		for (i= 0; i < n; i++) {
			if (xdrs->x_op == XDR_DECODE) {
				if (!(png->png_flags & PNG_BTYSSFLAGSABSENT)) {
					if (!xdr_u_int(xdrs, uip++))
						return 0;
					bs_iobytecnt+= 4;
				}
				else
					*(uip++)= BTYD_CLEAR;
			}
			else {
				if (!xdr_u_int(xdrs, uip++))
					return 0;
				bs_iobytecnt+= 4;
			}
		}

		/* sidescan */
		n= png->png_sides[side].ps_sscount;
		fp= pddata->pd_ss[side];
		if ((n > 0) && (fp == (float *) 0))
			return 0;
		for (i= 0; i < n; i++) {
			if (!xdr_float(xdrs, fp++))
				return 0;
			bs_iobytecnt+= 4;
		}

		/* sidescan flags */
		n= png->png_sides[side].ps_sscount;
		ucp= pddata->pd_ssflags[side];
		if ((n > 0) && (ucp == (unsigned char *) 0))
			return 0;
		if (n > 0) {
			cp= (char *) ucp;
			ui1= (u_int) n;
			if (xdrs->x_op == XDR_DECODE) {
				if (!(png->png_flags & PNG_BTYSSFLAGSABSENT)) {
					if (!xdr_bytes(xdrs, &cp, &ui1, (u_int) n))
						return 0;
					bs_iobytecnt+= 4+n;
					if ((rem= bs_iobytecnt%4) > 0)
						bs_iobytecnt+= 4-rem;
				}
				else {
					for (i= 0; i < n; i++)
						ucp[i]= SSD_CLEAR;
				}
			}
			else {
				if (!xdr_bytes(xdrs, &cp, &ui1, (u_int) n))
					return 0;
				bs_iobytecnt+= 4+n;
				if ((rem= bs_iobytecnt%4) > 0)
					bs_iobytecnt+= 4-rem;
			}
		}
	}

	/* auxiliary beam information */
	if (png->png_flags & PNG_ABI) {
		for (side= ACP_PORT; side < ACP_NSIDES; side++) {
			n= png->png_sides[side].ps_btycount;
			abi= pddata->pd_abi[side];
			if ((n > 0) && (abi == (AuxBeamInfo *) 0))
				return 0;
			for (i= 0; i < n; i++, abi++) {
				if (!xdr_u_int(xdrs, &(abi->abi_flags)))
					return 0;
				bs_iobytecnt+= 4;
				if (!xdr_int(xdrs, &(abi->abi_id)))
					return 0;
				bs_iobytecnt+= 4;
				if (!xdr_float(xdrs, &(abi->abi_ssat0)))
					return 0;
				bs_iobytecnt+= 4;
				if (!xdr_float(xdrs, &(abi->abi_ssat1)))
					return 0;
				bs_iobytecnt+= 4;
			}
		}
	}

	return 1;
}

int
mbbs_xdrstring(XDR *xdrs, char **cpp, unsigned long *bytecnt)
/*
   User-callable routine.
   Does XDR encoding and decoding of character strings.
   These are stored as an integer (the string length) followed
   by the bytes of the string (if the length is greater than 0).
   Records the total number of bytes transferred (including
   the leading integer) into *bytecnt.
   Returns 1 if successful, 0 otherwise.
*/
{
	int len, rem;
	u_int ul;

	*bytecnt= 0;

	switch (xdrs->x_op) {
	case XDR_ENCODE:
		if ((*cpp == (char *) 0) || (strlen(*cpp) == (StrSizeType) 0)) {
			len= 0;
			if (!xdr_int(xdrs, &len))
				return 0;
			*bytecnt+= 4;
		}
		else {
			len= (int) strlen(*cpp);
			if (!xdr_int(xdrs, &len))
				return 0;
			*bytecnt+= 4;
			ul= (u_int) len;
			if (!xdr_bytes(xdrs, cpp, &ul, (u_int) len))
				return 0;
			*bytecnt+= 4+len;
			if ((rem= (*bytecnt)%4) > 0)
				*bytecnt+= 4-rem;
		}
		break;
	case XDR_DECODE:
		if (!xdr_int(xdrs, &len))
			return 0;
		*bytecnt+= 4;
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
			(*cpp)[len]= '\0';
			*bytecnt+= 4+len;
			if ((rem= (*bytecnt)%4) > 0)
				*bytecnt+= 4-rem;
		}
		break;
	default:	/* shut up compiler warnings */
		break;
	}

	return 1;
}

int
mbbs_rdversion(FILE *fp, int *version)
{
	XDR xdrs;

	if (sizeof(int) < 4)
		return BS_BADARCH;

	bs_iobytecnt= 0;

	xdrstdio_create(&xdrs, fp, XDR_DECODE);

	if (!xdr_int(&xdrs, version))
		return BS_FAILURE;
	bs_iobytecnt+= 4;
	switch (*version) {
	case MR1_VERSION_1_0:
	case MR1_VERSION_2_0:
	case BS_VERSION_1_0:
	case BS_VERSION_1_1:
	case BS_VERSION_1_2:
	case BS_VERSION_1_3:
	case BS_VERSION_1_4:
		break;
	default:
		return BS_BADDATA;
	}

	xdr_destroy(&xdrs);

	return BS_SUCCESS;
}
