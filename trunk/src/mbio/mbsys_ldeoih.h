/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_hsds.h	3/2/93
 *	$Id: mbsys_ldeoih.h,v 4.0 1994-03-06 00:01:56 caress Exp $
 *
 *    Copyright (c) 1993, 1994 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbsys_hsds.h defines the data structure used by MBIO functions
 * to store multibeam data in a general purpose archive format:
 *      MBF_HSLDEOIH : MBIO ID 9
 *
 * Author:	D. W. Caress
 * Date:	March 2, 1993
 * $Log: not supported by cvs2svn $
 * Revision 4.1  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.0  1994/02/17  20:42:20  caress
 * First cut at new version.  Recast format to include both
 * beam amplitude and sidescan data.  I hope noone has used
 * the old version of this format, as the files will be
 * orphaned!!!
 *
 * Revision 3.0  1993/05/14  23:04:50  sohara
 * initial version
 *
 */
/*
 * Notes on the MBSYS_LDEOIH data structure:
 *   1. This data structure is used to store multibeam bathymetry
 *      and/or backscatter data with arbitrary numbers of beams.
 *	This format was created by the Lamont-Doherty Earth
 *	Observatory to serve as a general purpose archive format for
 *	processed multibeam data.
 *   2. The data consist of variable length binary records encoded
 *	entirely in 2-byte integers.
 *   3. Both the depth and backscatter arrays are centered.
 *   4. The kind value in the mbf_sbsiocen_struct indicates whether the
 *      mbf_sbsiocen_data_struct structure holds data (kind = 1) or an
 *      ascii comment record (kind = 0).
 */

/* maximum line length in characters */
#define MBSYS_LDEOIH_MAXLINE 200

struct mbsys_ldeoih_struct
	{
	/* type of data record */
	int	kind;

	/* position */
	unsigned short	lon2u;	/* minutes east of prime meridian */
	unsigned short	lon2b;	/* fraction of minute times 10000 */
	unsigned short	lat2u;	/* number of minutes north of 90S */
	unsigned short	lat2b;	/* fraction of minute times 10000 */

	/* time stamp */
	short	year;		/* year (4 digits) */
	short	day;		/* julian day (1-366) */
	short	min;		/* minutes from beginning of day (0-1439) */
	short	sec;		/* seconds from beginning of minute (0-59) */

	/* heading and speed */
	unsigned short	heading;/* heading:
					0 = 0 degrees
					1 = 0.0055 degrees
					16384 = 90 degrees
					65535 = 359.9945 degrees
					0 = 360 degrees */
	unsigned short	speed;	/* km/s X100 */

	/* numbers of beams */
	short	beams_bath;	/* number of depth values */
	short	beams_amp;	/* number of amplitude values */
	short	pixels_ss;	/* number of sidescan pixels */
	short	bath_scale;	/* 1000Xscale where depth=bathXscale */
	short	amp_scale;	/* 1000Xscale where amplitude=ampXscale */
	short	ss_scale;	/* 1000Xscale where sidescan=ssXscale */

	/* pointers to arrays */
	short	*bath;
	short	*amp;
	short	*bath_acrosstrack;
	short	*bath_alongtrack;
	short	*ss;
	short	*ss_acrosstrack;
	short	*ss_alongtrack;

	/* comment */
	char	comment[MBSYS_LDEOIH_MAXLINE];
	};
