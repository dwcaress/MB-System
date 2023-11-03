/*--------------------------------------------------------------------
 *    The MB-system:	mbbs_misc.c	3/3/2014
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
 *	Copyright (c) 1991 by University of Hawaii.
 */

/*
   misc.c --
   Miscellaneous routines for MR1 post-processing software.
*/

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mbbs.h"
#include "mbbs_defines.h"
#include "mbbs_mem.h"

#ifdef WIN32
#define isnan _isnan
#endif

/*
   User-callable routine.

   Returns the size in bytes of the smallest buffer capable of
   holding the sensor, bathymetry, bathymetry flag, sidescan and
   auxiliary beam information data referred to by png into *pngsz.

   Returns BS_SUCCESS or an error code. A reasonable (but possibly
   not airtight and definitely not precise) effort is made to guard
   against pings with negative or overly large sample counts.

   Note that the original design of the BS format implicitly
   limited the maximum ping buffer size to be no greater than the
   number of bytes which could be described by a signed 32-bit
   integer, i.e., BS_MAXSIGNEDINT32 (2147483647). This routine has
   been coded to enforce that limitation, but will attempt if possible
   to determine the actual size of a ping buffer even when it would
   be greater than this limit. In any case where the size of the
   ping is known to be over the limit, or it is believed that it
   might be over the limit, the function will return BS_HUGEPING.
   In such cases the value written to *pngsz will be accurate if and
   only if the host architecture supports 8-byte (or larger) unsigned
   long long integers.
*/
int mbbs_pngdatabufsz(Ping *png, unsigned long long *pngsz) {
	if (pngsz == (unsigned long long *)0)
		return BS_BADARG;

	*pngsz = 0;

	if (sizeof(int) < 4)
		return BS_BADARCH;
	if (png == (Ping *)0)
		return BS_BADARG;

	/* any negative sample count will cause trouble */
	if ((png->png_compass.sns_nsamps < 0) || (png->png_depth.sns_nsamps < 0) || (png->png_pitch.sns_nsamps < 0) ||
	    (png->png_roll.sns_nsamps < 0) || (png->png_sides[ACP_PORT].ps_btycount < 0) ||
	    (png->png_sides[ACP_PORT].ps_btypad < 0) || (png->png_sides[ACP_PORT].ps_sscount < 0) ||
	    (png->png_sides[ACP_PORT].ps_sspad < 0) || (png->png_sides[ACP_STBD].ps_btycount < 0) ||
	    (png->png_sides[ACP_STBD].ps_btypad < 0) || (png->png_sides[ACP_STBD].ps_sscount < 0) ||
	    (png->png_sides[ACP_STBD].ps_sspad < 0))
		return BS_BADDATA;

	/* assume success until we discover otherwise */
	int retval = BS_SUCCESS;

	/* we cannot accurately check for oversized pings
	   on hosts where sizeof(unsigned long long) is
	   less than 8 bytes, so adopt greater restrictions
	   on data sizes as necessary */
	if (sizeof(unsigned long long) < 8) {
		const int npbty = png->png_sides[ACP_PORT].ps_btycount + png->png_sides[ACP_PORT].ps_btypad;
		const int npss = png->png_sides[ACP_PORT].ps_sscount + png->png_sides[ACP_PORT].ps_sspad;
		const int nsbty = png->png_sides[ACP_STBD].ps_btycount + png->png_sides[ACP_STBD].ps_btypad;
		const int nsss = png->png_sides[ACP_STBD].ps_sscount + png->png_sides[ACP_STBD].ps_sspad;

		/* negative sums here probably indicate additive
		   overflow since the input operands were positive */
		if (npbty < 0)
			retval = BS_HUGEPING;
		else if (npss < 0)
			retval = BS_HUGEPING;
		else if (nsbty < 0)
			retval = BS_HUGEPING;
		else if (nsss < 0)
			retval = BS_HUGEPING;

		else if ((png->png_compass.sns_nsamps > BS_MAXATTSAMPS) || (png->png_depth.sns_nsamps > BS_MAXATTSAMPS) ||
		         (png->png_pitch.sns_nsamps > BS_MAXATTSAMPS) || (png->png_roll.sns_nsamps > BS_MAXATTSAMPS) ||
		         (npbty > BS_MAXBTYSAMPS) || (npss > BS_MAXSSSAMPS) || (nsbty > BS_MAXBTYSAMPS) || (nsss > BS_MAXSSSAMPS))
			retval = BS_HUGEPING;

		if (retval != BS_SUCCESS)
			return retval;
	}

	unsigned long long bsi;
	if (png->png_flags & PNG_XYZ)
		bsi = 3;
	else
		bsi = 2;

	/* explicitly cast everything to unsigned long long */
	const unsigned long long ullncompass = (unsigned long long)png->png_compass.sns_nsamps;
	const unsigned long long ullndepth = (unsigned long long)png->png_depth.sns_nsamps;
	const unsigned long long ullnpitch = (unsigned long long)png->png_pitch.sns_nsamps;
	const unsigned long long ullnroll = (unsigned long long)png->png_roll.sns_nsamps;
	const unsigned long long ullpbtyc = (unsigned long long)png->png_sides[ACP_PORT].ps_btycount;
	const unsigned long long ullpbtyp = (unsigned long long)png->png_sides[ACP_PORT].ps_btypad;
	const unsigned long long ullpssc = (unsigned long long)png->png_sides[ACP_PORT].ps_sscount;
	const unsigned long long ullpssp = (unsigned long long)png->png_sides[ACP_PORT].ps_sspad;
	const unsigned long long ullsbtyc = (unsigned long long)png->png_sides[ACP_STBD].ps_btycount;
	const unsigned long long ullsbtyp = (unsigned long long)png->png_sides[ACP_STBD].ps_btypad;
	const unsigned long long ullsssc = (unsigned long long)png->png_sides[ACP_STBD].ps_sscount;
	const unsigned long long ullsssp = (unsigned long long)png->png_sides[ACP_STBD].ps_sspad;

	unsigned long long nbytes = 0;
	unsigned long long nuints;
	unsigned long long nuchars;

	/* sensor section */
	unsigned long long nfloats = ullncompass + ullndepth + ullnpitch + ullnroll;
	nbytes += nfloats * ((unsigned long long)sizeof(float));

	/* port bathymetry/sidescan data and flags */
	nfloats = (bsi * (ullpbtyc + ullpbtyp)) + ullpssc + ullpssp;
	nuints = ullpbtyc + ullpbtyp;
	nuchars = ullpssc + ullpssp;
	nbytes += (nfloats * ((unsigned long long)sizeof(float))) + (nuints * ((unsigned long long)sizeof(unsigned int))) + nuchars;

	/* make sure start of starboard data/flags
	   section is properly byte-aligned */
	{
		const unsigned long long k = nbytes % PNG_BYTEALIGNSZ;
		if (k != 0)
			nbytes += PNG_BYTEALIGNSZ - k;
	}

	/* starboard bathymetry/sidescan data and flags */
	nfloats = (bsi * (ullsbtyc + ullsbtyp)) + ullsssc + ullsssp;
	nuints = ullsbtyc + ullsbtyp;
	nuchars = ullsssc + ullsssp;
	nbytes += (nfloats * ((unsigned long long)sizeof(float))) + (nuints * ((unsigned long long)sizeof(unsigned int))) + nuchars;

	/* make sure start of auxiliary beam
	   information is properly byte-aligned */
	{
		const unsigned long long k = nbytes % PNG_BYTEALIGNSZ;
		if (k != 0)
			nbytes += PNG_BYTEALIGNSZ - k;
	}

	/* auxiliary beam information */
	if (png->png_flags & PNG_ABI) {
		unsigned long long nabis = ullpbtyc + ullpbtyp + ullsbtyc + ullsbtyp;
		nbytes += nabis * ((unsigned long long)sizeof(AuxBeamInfo));
	}

	*pngsz = nbytes;

	if (sizeof(unsigned long long) >= 8) {
		unsigned long long maxsignedint32 = (unsigned long long)BS_MAXSIGNEDINT32;
		if (*pngsz > maxsignedint32)
			retval = BS_HUGEPING;
	}

	return retval;
}

/*
   User-callable routine.
   Allocates memory for ping data arrays.
   Returns a pointer to the allocated memory.
*/
MemType *mbbs_pngmemalloc(Ping *png) {
	if (sizeof(int) < 4)
		return (MemType *)0;

	unsigned long long ullnbytes;
	const int err = mbbs_pngdatabufsz(png, &ullnbytes);
	if (err != BS_SUCCESS)
		return (MemType *)0;

	/* this should be safe to do if the
	   preceding call has not returned
	   BS_HUGEPING (or any other error) */
	unsigned int nbytes = (unsigned int)ullnbytes;

	return (float *)calloc((MemSizeType)nbytes, (MemSizeType)1);
}

/*
   User-callable routine.

   Reallocates memory for ping data arrays.

   Returns BS_SUCCESS or any of various error codes, also
   returns a pointer to the reallocated (if necessary) buffer
   into *buf and the size in bytes of the buffer into bufsz.
*/
int mbbs_pngrealloc(Ping *png, MemType **buf, unsigned int *bufsz) {
	if (sizeof(int) < 4)
		return BS_BADARCH;

	if (png == (Ping *)0)
		return BS_BADARG;

	unsigned long long ullnbytes;
	int err = mbbs_pngdatabufsz(png, &ullnbytes);
	if (err != BS_SUCCESS)
		return err;

	/* this should be safe to do if the
	   preceding call has not returned
	   BS_HUGEPING (or any other error) */
	const unsigned int nbytes = (unsigned int)ullnbytes;

	int retval = mbbs_memalloc(buf, bufsz, nbytes, (size_t)1);
	if (retval != MEM_SUCCESS) {
		switch (retval) {
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

	return BS_SUCCESS;
}

/*
   User-callable routine.

   Returns pointers to the various sections (e.g., sensor, bathymetry,
   etc.) of a ping's data sample buffer into the fields of pd.

   Returns BS_SUCCESS or BS_BADARG.
*/
int mbbs_getpngdataptrs(Ping *png, MemType *data, PingData *pd) {
	if (sizeof(int) < 4)
		return BS_BADARCH;

	if (png == (Ping *)0)
		return BS_BADARG;

	unsigned long long ullnbytes;
	const int err = mbbs_pngdatabufsz(png, &ullnbytes);
	if (err != BS_SUCCESS)
		return err;

	/* this may or may not be the right thing to do,
	   but not doing it will break almost any HMRG program
	   which encounters a ping with no data */
	if (ullnbytes == 0)
		return BS_SUCCESS;

	if (data == (MemType *)0)
		return BS_BADARG;
	if (pd == (PingData *)0)
		return BS_BADARG;

	float *fp = (float *)data;
	pd->pd_compass = fp;
	fp += png->png_compass.sns_nsamps;
	pd->pd_depth = fp;
	fp += png->png_depth.sns_nsamps;
	pd->pd_pitch = fp;
	fp += png->png_pitch.sns_nsamps;
	pd->pd_roll = fp;
	fp += png->png_roll.sns_nsamps;
	fp += png->png_snspad;
	unsigned char *ucp1 = (unsigned char *)fp;

	int bsi;
	if (png->png_flags & PNG_XYZ)
		bsi = 3;
	else
		bsi = 2;

	for (int side = ACP_PORT; side < ACP_NSIDES; side++) {
		fp = (float *)ucp1;

		pd->pd_bty[side] = fp;
		fp += (bsi * (png->png_sides[side].ps_btycount + png->png_sides[side].ps_btypad));

		unsigned int *uip = (unsigned int *)fp;
		pd->pd_btyflags[side] = uip;
		uip += png->png_sides[side].ps_btycount + png->png_sides[side].ps_btypad;

		fp = (float *)uip;
		pd->pd_ss[side] = fp;
		fp += png->png_sides[side].ps_sscount + png->png_sides[side].ps_sspad;

		ucp1 = (unsigned char *)fp;
		pd->pd_ssflags[side] = ucp1;
		ucp1 += png->png_sides[side].ps_sscount + png->png_sides[side].ps_sspad;

		/* make sure start of next data
		   section is properly byte-aligned */
		const unsigned char *ucp0 = (unsigned char *)data;
		const unsigned int nbytes = (unsigned int)(ucp1 - ucp0);
		const int k = nbytes % PNG_BYTEALIGNSZ;
		if (k != 0)
			ucp1 += PNG_BYTEALIGNSZ - k;
	}

	if (png->png_flags & PNG_ABI) {
		pd->pd_abi[ACP_PORT] = (AuxBeamInfo *)ucp1;
		pd->pd_abi[ACP_STBD] = pd->pd_abi[ACP_PORT] + png->png_sides[ACP_PORT].ps_btycount + png->png_sides[ACP_PORT].ps_btypad;
	}
	else
		pd->pd_abi[ACP_PORT] = pd->pd_abi[ACP_STBD] = (AuxBeamInfo *)0;

	return BS_SUCCESS;
}

/*
   User-callable routine.
   Append a string to the specified string field.
   Returns BS_SUCCESS, BS_BADARG or BS_MEMALLOC.
*/
int mbbs_appendstr(char **field, char *str) {
	if (field == (char **)0)
		return BS_BADARG;
	if ((str == (char *)0) || (strlen(str) == 0))
		return BS_SUCCESS;

	StrSizeType len;
	if (*field != (char *)0)
		len = strlen(*field);
	else
		len = (StrSizeType)0;
	len += strlen(str) + 1;

	char *newfield = (char *)calloc((MemSizeType)len, sizeof(char));
	if (newfield == NULL)
		return BS_MEMALLOC;
	if (*field != (char *)0)
		(void)strcpy(newfield, *field);
	(void)strcat(newfield, str);
	if (*field != (char *)0)
		CFree((MemType *)*field);
	*field = newfield;

	return BS_SUCCESS;
}

/*
   User-callable routine.
   Appends the specified argument list to the file log
   with (i) a leading newline (if the current file log is
   non-empty), (ii) separating blank spaces between the
   strings of the argument list and (iii) a trailing semicolon.
   Returns BS_SUCCESS, BS_BADARCH, BS_BADARG or BS_MEMALLOC.
*/
int mbbs_appendlog(BSFile *bsf, char **argv) {
	if (sizeof(int) < 4)
		return BS_BADARCH;

	if (bsf == (BSFile *)0)
		return BS_BADARG;
	if (argv == (char **)0)
		return BS_BADARG;

	StrSizeType len = 0;
	for (char **av = argv; *av != (char *)0; av++) {
		if (strlen(*av) > 0)
			len += strlen(*av) + 1;
	}
	if (len == (StrSizeType)0)
		return BS_SUCCESS;
	if ((bsf->bsf_log != (char *)0) && (strlen(bsf->bsf_log) > 0))
		len += strlen(bsf->bsf_log);
	else
		len -= 1;
	len += 2;

	char *newlog = (char *)calloc((MemSizeType)len, sizeof(char));
	if (newlog == NULL)
		return BS_MEMALLOC;
	if ((bsf->bsf_log != (char *)0) && (strlen(bsf->bsf_log) > 0))
		(void)strcpy(newlog, bsf->bsf_log);
	else
		(void)strcpy(newlog, "");

	int firstarg = 1;
	for (char **av = argv; *av != (char *)0; av++) {
		if (strlen(*av) > 0) {
			if (firstarg) {
				if ((bsf->bsf_log != (char *)0) && (strlen(bsf->bsf_log) > 0))
					(void)strcat(newlog, "\n");
				firstarg = 0;
			}
			else
				(void)strcat(newlog, " ");
			(void)strcat(newlog, *av);
		}
	}
	(void)strcat(newlog, ";");

	if (bsf->bsf_log != (char *)0)
		CFree((MemType *)bsf->bsf_log);
	bsf->bsf_log = newlog;

	return BS_SUCCESS;
}

/*
   User-callable routine.
   Copy a string to the specified string field.
   Returns BS_SUCCESS, BS_BADARG or BS_MEMALLOC.
*/
int mbbs_replacestr(char **field, char *str) {
	if (field == (char **)0)
		return BS_BADARG;
	if ((str == (char *)0) || (strlen(str) == 0)) {
		if (*field != (char *)0) {
			CFree((MemType *)*field);
			*field = (char *)0;
		}
		return BS_SUCCESS;
	}

	StrSizeType olen;
	if (*field != (char *)0)
		olen = strlen(*field) + 1;
	else
		olen = 0;
	const StrSizeType nlen = strlen(str) + 1;

	if (nlen != olen) {
		char *newfield = (char *)calloc((MemSizeType)nlen, sizeof(char));
		if (newfield == NULL)
			return BS_MEMALLOC;
		(void)strcpy(newfield, str);
		if (*field != (char *)0)
			CFree((MemType *)*field);
		*field = newfield;
	}
	else
		(void)strcpy(*field, str);

	return BS_SUCCESS;
}

/*
   User-callable routine.
   Strips all consecutive instances of c from the end of str.
   Returns BS_SUCCESS or BS_BADARG.
*/
int mbbs_striptail(char *str, char c) {
	if (str == NULL)
		return BS_BADARG;
	const StrSizeType len = strlen(str);
	if (len == 0)
		return BS_BADARG;
	if (c == '\0')
		return BS_BADARG;

	for (char *cp = str + len - 1; cp >= str; cp--) {
		if (*cp == c)
			*cp = '\0';
		else
			break;
	}

	return BS_SUCCESS;
}

/*
   User-callable routine.
   Allocates ping mark memory and sets all marks to BS_NULLMARK (i.e., 0).
*/
void *mbbs_mrkmemalloc(int size) {
	unsigned int bufsz = size / 2;
	if (size % 2 != 0)
		bufsz++;
	return (void *)calloc((MemSizeType)bufsz, sizeof(char));
}

/*
   User-callable routine.
   Returns the ping mark value of the specified ping index.
*/
int mbbs_mrkget(void *mrkbuf, int side, int index) {
	char *c = (char *)mrkbuf;
	c += index / 2;
	return ((*c >> (((index % 2) * 4) + (side * 2))) & 0x3);
}

/*
   User-callable routine.
   Sets the ping mark value of the specified ping index.
*/
void mbbs_mrkset(void *mrkbuf, int side, int index, int value) {
	char *c = (char *)mrkbuf;
	c += index / 2;
	*c &= ~(0x3 << (((index % 2) * 4) + (side * 2)));
	unsigned char uv = (unsigned char)((value & 0x3) << (((index % 2) * 4) + (side * 2)));
	*c |= uv;
	return;
}

/*
   The following routines return single- and double-precision NaNs.
   They use specific bit patterns when the size in bytes of the
   floating point storage units and the bit patterns are the same,
   otherwise they fall back to 0./0., which hopefully will work
   in all other cases.
*/
float mbbs_nanf() {
	unsigned int ui = 0xffc00000;
	float f;
	MemCopy(&ui, &f, sizeof(float));

	return f;
}

double mbbs_nand() {
	unsigned long long ull = 0xfff8000000000000ULL;
	double d;
	MemCopy(&ull, &d, sizeof(double));

	return d;
}

int mbbs_isnanf(float f) { return isnan((double)f); }

int mbbs_isnand(double d) { return isnan(d); }
