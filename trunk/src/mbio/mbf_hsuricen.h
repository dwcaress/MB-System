/*--------------------------------------------------------------------
 *    The MB-system:	mbf_hsuricen.h	3.00	1/20/93
 *	$Id: mbf_hsuricen.h,v 3.0 1993-05-14 22:52:10 sohara Exp $
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
 * mbf_hsuricen.h defines the data structure used by MBIO functions
 * to store multibeam data read from the  MBF_HSURICEN format (MBIO id 6).  
 *
 * Author:	D. W. Caress
 * Date:	January 20, 1993
 * $Log: not supported by cvs2svn $
 */
/*
 * Notes on the MBF_HSURICEN data format:
 *   1. This data format is used to store 59 beam Hydrosweep DS bathymetry
 *      data.  This format was created and used by the Ocean Mapping
 *      Development Center at the Graduate School of Oceanography of
 *      the University of Rhode Islande; most data files in this format
 *      consist of Hydrosweep DS data collected on the R/V Maurice Ewing.
 *   2. The data consist of 328 byte records including 1-byte characters,
 *      2-byte integers, and 8-byte integers.
 *   3. The 59 depth values are stored centered in 59 value arrays.  The
 *      center beam is in word 30 of the depth and distance arrays.
 *   4. Comments can be embedded in the data as 100 byte ascii strings,
 *	where the first two characters must always be "cc" so that the
 *      sec value is 25443.  
 *   5. This format was deciphered from examples of data.
 *
 * The kind value in the mbf_hsuricen_struct indicates whether the
 * mbf_hsuricen_data_struct structure holds data (kind = 1) or an
 * ascii comment record (kind = 2).
 *
 * The mbf_hsuricen_data_struct structure is a direct representation 
 * of the binary data structure used in the MBF_HSURICEN format.
 */

struct mbf_hsuricen_data_struct
	{
	short	sec;		/* seconds x 100 */
	short	min;		/* minute of the day */
	short	day;		/* day of the year */
	short	year;		/* 4-digit year */
	long	lat;		/* latitude in degrees times 10^7 */
	long	lon;		/* longitude in degrees times 10^7 */
	short	hdg;		/* heading in degrees x 10 */
	short	course;		/* course in degrees x 10 */
	short	speed;		/* speed in knots x 100 */
	short	pitch;		/* pitch in degrees x 10 */
	short	scale;		/* scale factor x 100 */
	unsigned char	speed_ref;	/* 'B' or 'W' */
	unsigned char	quality;	
	short	deph[59];	/* scaled depths in meters
					portmost to stbdmost */
	short	dist[59];	/* scaled cross track distances,
					port ranges are negative */
	};	

struct mbf_hsuricen_struct
	{
	int	kind;
	struct mbf_hsuricen_data_struct data;
	};
