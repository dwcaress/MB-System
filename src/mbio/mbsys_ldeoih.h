/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_hsds.h	3.00	3/2/93
 *	$Id: mbsys_ldeoih.h,v 3.0 1993-05-14 23:04:50 sohara Exp $
 *
 *    Copyright (c) 1993 by 
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
	short	beams_bath;	/* number of depth values in meters/bathscale */
	short	beams_back;	/* number of backscatter values */
	short	bathscale;	/* 1000Xscale where depth=bathXscale */
	short	backscale;	/* 1000Xscale where backscatter=backXscale */

	/* pointers to arrays */
	short	*bath;
	short	*bathdist;
	short	*back;
	short	*backdist;

	/* comment */
	char	comment[MBSYS_LDEOIH_MAXLINE];
	};
