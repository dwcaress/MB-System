/*--------------------------------------------------------------------
 *    The MB-system:	mbf_hsldedmb.h	1/20/93
 *	$Id: mbf_hsldedmb.h,v 5.0 2000-12-01 22:48:41 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbf_hsldedmb.h defines the data structure used by MBIO functions
 * to store multibeam data read from the  MBF_HSLDEDMB format (MBIO id 22).  
 *
 * Author:	D. W. Caress
 * Date:	January 20, 1993
 * $Log: not supported by cvs2svn $
 * Revision 4.6  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.5  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.4  1997/09/15  19:06:40  caress
 * Real Version 4.5
 *
 * Revision 4.3  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.2  1994/10/21  12:20:01  caress
 * Release V4.0
 *
 * Revision 4.2  1994/10/21  12:20:01  caress
 * Release V4.0
 *
 * Revision 4.1  1994/07/29  18:46:51  caress
 * Changes associated with supporting Lynx OS (byte swapped) and
 * using unix second time base (for time_d values).
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.2  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.1  1994/02/17  21:19:08  caress
 * Updated associated MBIO format id in comments.
 *
 * Revision 4.0  1994/02/17  20:59:54  caress
 * First cut at new version. No changes.
 *
 * Revision 3.0  1993/05/14  22:51:24  sohara
 * initial version
 *
 */
/*
 * Notes on the MBF_HSLDEDMB data format:
 *   1. This data format is used to store 59 beam Hydrosweep DS bathymetry
 *      data.  This format was created by Dale Chayes of L-DEO and Dan
 *      Chayes of NRL for use with an early version of a ping editor. 
 *      Most data files in this format consist of Hydrosweep DS data 
 *      collected on the R/V Maurice Ewing or the R/V Thomas Thompson.
 *   2. The data consist of 328 byte records including 1-byte characters,
 *      2-byte integers, and 8-byte integers.
 *   3. The 59 depth values are stored centered in 59 value arrays.  The
 *      center beam is in word 30 of the depth and distance arrays.
 *   4. There is no provision for embedding comments in the data.
 *   4. Comments can be embedded in the data as 328 byte ascii strings,
 *	where the first four characters must always be "zzzz" so that the
 *      seconds value is 2054847098.  
 *   5. We expect the use of this format to be superceded by format
 *      MBF_MBLDEOIH.
 *
 * The kind value in the mbf_hsldedmb_struct indicates whether the
 * mbf_hsldedmb_data_struct structure holds data (kind = 1) or an
 * ascii comment record (kind = 2).
 *
 * The mbf_hsldedmb_data_struct structure is a direct representation  
 * of the binary data structure used in the MBF_HSLDEDMB format.
 */

struct mbf_hsldedmb_data_struct
	{ 
	unsigned int	seconds;	/* seconds since 1/1/70 00:00:00 */
	unsigned int	microseconds;	/* microseconds */
	unsigned int	alt_seconds;	/* seconds since last survey header */
	unsigned int	alt_microseconds;	/* microseconds */
	int	lat;		/* latitude in degrees times 10000000 */
	int	lon;		/* latitude in degrees times 10000000 */
	short	heading;	/* heading in degrees times 10 */
	short	course;		/* course in degrees times 10 */
	short	speed;		/* speed in m/s times 10 */
	short	pitch;		/* pitch in degrees times 10 */
	short	scale;		/* multiplicative scale times 100 for depth 
					and range values */
	short	depth[59];	/* depths in scaled meters assuming 
					1500 m/s water velocity */
	short	range[59];	/* cross track distances in meters */
	mb_u_char	speed_ref;	/* speed reference 
					("B": bottom track) */
	mb_u_char	quality;	/* quality flag */
	unsigned int	flag[4];
	};

struct mbf_hsldedmb_struct
	{ 
	int	kind;
	struct mbf_hsldedmb_data_struct data;
	};
