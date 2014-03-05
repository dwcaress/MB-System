/*--------------------------------------------------------------------
 *    The MB-system:	misc.c	3/7/2003
 *	$Id$
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
 * $Log: misc.c,v $
 * Revision 5.1  2006/01/11 07:46:15  caress
 * Working towards 5.0.8
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
   misc.c --
   Miscellaneous routines for MR1 post-processing software.
*/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "mr1pr_defines.h"
#include "mem.h"
#ifdef WIN32
#include <float.h>
#define isnan _isnan
#endif

unsigned int
mr1_pngdatabufsz(Ping *png)
/*
   User-callable routine.
   Returns the size in bytes of the smallest buffer
   capable of holding the sensor, bathymetry and sidescan
   data referred to by png.
*/
{
	int nfloats;

	if (png == (Ping *) 0)
		return (MemSizeType) 0;

	nfloats= png->png_compass.sns_nsamps+
		 png->png_depth.sns_nsamps+
		 png->png_pitch.sns_nsamps+
		 png->png_roll.sns_nsamps+
		 png->png_snspad+
		 (2*(png->png_sides[ACP_PORT].ps_btycount+png->png_sides[ACP_PORT].ps_btypad))+
		 png->png_sides[ACP_PORT].ps_sscount+
		 png->png_sides[ACP_PORT].ps_sspad+
		 (2*(png->png_sides[ACP_STBD].ps_btycount+png->png_sides[ACP_STBD].ps_btypad))+
		 png->png_sides[ACP_STBD].ps_sscount+
		 png->png_sides[ACP_STBD].ps_sspad;

	return (unsigned int) (nfloats*sizeof(float));
}

float *
mr1_pngmemalloc(Ping *png)
/*
   User-callable routine.
   Allocates memory for ping data arrays.
   Returns a pointer to the allocated memory.
*/
{
	unsigned int nbytes;

	nbytes= mr1_pngdatabufsz(png);

	return (float *) calloc((MemSizeType) nbytes, (MemSizeType) 1);
}

int
mr1_pngrealloc(Ping *png, float **buf, unsigned int *bufsz)
/*
   User-callable routine.
   Reallocates memory for ping data arrays.
   Returns MR1_SUCCESS or any of various error codes, also
   returns a pointer to the reallocated (if necessary) buffer to *buf and
   the size in bytes of the buffer to bufsz.
*/
{
	unsigned int nbytes;
	int retval;

	nbytes= mr1_pngdatabufsz(png);
	if (png == (Ping *) 0)
		return MR1_BADARG;

	if ((retval= memalloc((MemType **) buf, bufsz, nbytes, (unsigned int) 1)) != MEM_SUCCESS) {
		switch (retval) {
		case MEM_BADARG:
			return MR1_BADARG;
		case MEM_OOB:
			return MR1_BADARG;
		case MEM_CALLOC:
			return MR1_MEMALLOC;
		default:
			return MR1_FAILURE;
		}
	}

	return MR1_SUCCESS;
}

int
mr1_getpngdataptrs(Ping *png, float *databuf, float **cp, float **dp, float **pp, float **rp, float **pbty, float **pss, float **sbty, float **sss)
/*
   User-callable routine.
   Returns pointers to the various sensor, bathymetry and sidescan
   data streams of a ping.
   Returns MR1_SUCCESS or MR1_BADARG.
*/
{
	float *fp;

	if (png == (Ping *) 0)
		return MR1_BADARG;
	if (databuf == (float *) 0)
		return MR1_BADARG;

	fp= databuf;
	if (cp != (float **) 0)
		*cp= fp;

	fp+= png->png_compass.sns_nsamps;
	if (dp != (float **) 0)
		*dp= fp;

	fp+= png->png_depth.sns_nsamps;
	if (pp != (float **) 0)
		*pp= fp;

	fp+= png->png_pitch.sns_nsamps;
	if (rp != (float **) 0)
		*rp= fp;

	fp+= png->png_roll.sns_nsamps;
	fp+= png->png_snspad;
	if (pbty != (float **) 0)
		*pbty= fp;

	fp+= (2*(png->png_sides[ACP_PORT].ps_btycount+png->png_sides[ACP_PORT].ps_btypad));
	if (pss != (float **) 0)
		*pss= fp;

	fp+= png->png_sides[ACP_PORT].ps_sscount;
	fp+= png->png_sides[ACP_PORT].ps_sspad;
	if (sbty != (float **) 0)
		*sbty= fp;

	fp+= (2*(png->png_sides[ACP_STBD].ps_btycount+png->png_sides[ACP_STBD].ps_btypad));
	if (sss != (float **) 0)
		*sss= fp;

	return MR1_SUCCESS;
}

int
mr1_appendstr(char **field, char *str)
/*
   User-callable routine.
   Append a string to the specified string field.
   Returns MR1_SUCCESS, MR1_BADARG or MR1_MEMALLOC.
*/
{
	int len;
	char *newfield;

	if (field == (char **) 0)
		return MR1_BADARG;
	if ((str == (char *) 0) || ((int) strlen(str) == 0))
		return MR1_SUCCESS;

	if (*field != (char *) 0)
		len= (int) strlen(*field);
	else
		len= 0;
	len+= ((int) strlen(str))+1;

	if ((newfield= (char *) calloc((MemSizeType) len, sizeof(char))) == (char *) 0)
		return MR1_MEMALLOC;
	if (*field != (char *) 0)
		(void) strcpy(newfield, *field);
	(void) strcat(newfield, str);
	if (*field != (char *) 0)
		free((MemType *) *field);
	*field= newfield;

	return MR1_SUCCESS;
}

int
mr1_appendlog(MR1File *mrf, char **argv)
/*
   User-callable routine.
   Appends the specified argument list to the file log
   with (i) a leading newline (if the current file log is
   non-empty), (ii) separating blank spaces between the
   strings of the argument list and (iii) a trailing semicolon.
   Returns MR1_SUCCESS, MR1_BADARG or MR1_MEMALLOC.
*/
{
	char **av;
	int len, firstarg;
	char *newlog;

	if (mrf == (MR1File *) 0)
		return MR1_BADARG;
	if (argv == (char **) 0)
		return MR1_BADARG;

	for (av= argv, len= 0; *av != (char *) 0; av++) {
		if ((int) strlen(*av) > 0)
			len+= ((int) strlen(*av))+1;
	}
	if (len == 0)
		return MR1_SUCCESS;
	if ((mrf->mf_log != (char *) 0) && ((int) strlen(mrf->mf_log) > 0))
		len+= (int) strlen(mrf->mf_log);
	else
		len-= 1;
	len+= 2;

	if ((newlog= (char *) calloc((MemSizeType) len, sizeof(char))) == (char *) 0)
		return MR1_MEMALLOC;
	if ((mrf->mf_log != (char *) 0) && ((int) strlen(mrf->mf_log) > 0))
		(void) strcpy(newlog, mrf->mf_log);
	else
		(void) strcpy(newlog, "");

	for (av= argv, firstarg= 1; *av != (char *) 0; av++) {
		if ((int) strlen(*av) > 0) {
			if (firstarg) {
				if ((mrf->mf_log != (char *) 0) && ((int) strlen(mrf->mf_log) > 0))
					(void) strcat(newlog, "\n");
				firstarg= 0;
			}
			else
				(void) strcat(newlog, " ");
			(void) strcat(newlog, *av);
		}
	}
	(void) strcat(newlog, ";");

	if (mrf->mf_log != (char *) 0)
		free((MemType *) mrf->mf_log);
	mrf->mf_log= newlog;

	return MR1_SUCCESS;
}

int
mr1_replacestr(char **field, char *str)
/*
   User-callable routine.
   Copy a string to the specified string field.
   Returns MR1_SUCCESS, MR1_BADARG or MR1_MEMALLOC.
*/
{
	int olen, nlen;
	char *newfield;

	if (field == (char **) 0)
		return MR1_BADARG;
	if ((str == (char *) 0) || ((int) strlen(str) == 0)) {
		if (*field != (char *) 0) {
			free((MemType *) *field);
			*field= (char *) 0;
		}
		return MR1_SUCCESS;
	}

	if (*field != (char *) 0)
		olen= ((int) strlen(*field))+1;
	else
		olen= 0;
	nlen= ((int) strlen(str))+1;

	if (nlen != olen) {
		if ((newfield= (char *) calloc((MemSizeType) nlen, sizeof(char))) == (char *) 0)
			return MR1_MEMALLOC;
		(void) strcpy(newfield, str);
		if (*field != (char *) 0)
			free((MemType *) *field);
		*field= newfield;
	}
	else
		(void) strcpy(*field, str);

	return MR1_SUCCESS;
}

void *
mr1_mrkmemalloc(int size)
/*
   User-callable routine.
   Allocates ping mark memory and sets all marks to MR1_NULLMARK (i.e., 0).
*/
{
	unsigned int bufsz;

	bufsz= size/2;
	if (size%2 != 0)
		bufsz++;
	return (void *) calloc((MemSizeType) bufsz, sizeof(char));
}

int
mr1_mrkget(void *mrkbuf, int side, int index)
/*
   User-callable routine.
   Returns the ping mark value of the specified ping index.
*/
{
	char *c;

	c= (char *) mrkbuf;
	c+= index/2;
	return ((*c >> (((index%2)*4)+(side*2))) & 0x3);
}

void
mr1_mrkset(void *mrkbuf, int side, int index, int value)
/*
   User-callable routine.
   Sets the ping mark value of the specified ping index.
*/
{
	char *c;
	unsigned char uv;

	c= (char *) mrkbuf;
	c+= index/2;
	*c&= ~(0x3 << (((index%2)*4)+(side*2)));
	uv= (unsigned char) ((value & 0x3) << (((index%2)*4)+(side*2)));
	*c|= uv;
	return;
}

/*
   The following routines return single- and double-precision NaNs.
   They use specific bit patterns when the size in bytes of the
   floating point storage units and the bit patterns are the same,
   otherwise they fall back to 0./0., which hopefully will work
   in all other cases.
*/

float
mr1_nanf()
{
	unsigned int ui;
	float f;

	/* test removed at suggestion of Joaquim Luis */
	/* if (sizeof(float) != sizeof(unsigned int))
		f= 0./0.;
	else {
		ui= 0xffc00000;
		MemCopy(&ui, &f, sizeof(float));
	} */
	ui= 0xffc00000;
	MemCopy(&ui, &f, sizeof(float));

	return f;
}

double
mr1_nand()
{
	unsigned long long ull;
	double d;

	if (sizeof(double) != sizeof(unsigned long long))
		d= 0./0.;
	else {
		ull= 0xfff8000000000000;
		MemCopy(&ull, &d, sizeof(double));
	}

	return d;
}

int
mr1_isnanf(float f)
{
#if defined(sparc_os5) || defined(IRIX)
        return isnanf(f);
#else
	/* hope this works! */
        return isnan((double) f);
#endif /* defined(sparc_os5) || defined(IRIX) */
}

int
mr1_isnand(double d)
{
#if defined(sparc_os5) || defined(IRIX)
        return isnand(d);
#else
        return isnan(d);
#endif /* defined(sparc_os5) || defined(IRIX) */
}
