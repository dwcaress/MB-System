/*--------------------------------------------------------------------
 *    The MB-system:	mbf_sbifremr.h	3/29/96
 *	$Id$
 *
 *    Copyright (c) 1996-2011 by
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
 * mbf_sbifremr.h defines the data structure used by MBIO functions
 * to store multibeam data read from the  MBF_SBIFREMR format (MBIO id 17).  
 *
 * Author:	D. W. Caress
 * Date:	March 29, 1996
 * Location:	152 39.061W; 34 09.150S on R/V Ewing
 *
 * $Log: mbf_sbifremr.h,v $
 * Revision 5.2  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.1  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.0  2000/12/01 22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.2  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.1  1998/10/05  18:32:27  caress
 * MB-System version 4.6beta
 *
 * Revision 4.0  1996/04/22  10:59:51  caress
 * Initial version.
 *
 * Revision 4.0  1996/04/22  10:59:51  caress
 * Initial version.
 *
 *
 */
/*
 * Notes on the MBF_SBIFREMR data format:
 *   1. This data format is used to store 16 beam Sea Beam bathymetry
 *      data.  This format was created and used by IFREMER in Brest, 
 *      France. IFREMER archives SeaBeam "Classic" data from the
 *      R/V Jean Charcot and the R/V Sonne (and probably other
 *      vessels) in this format.
 *   2. The data consist of ASCII text. The data is stored in a
 *      sounding oriented rather than swath oriented fashion, with
 *      a separate record for each beam value.
 *   3. Each 108 character line contains the beam position, depth, 
 *      ping number, beam number, sounding number, and ping time.
 *      The ship's position can be obtained as that of the
 *      the center beam (beam 10 out of beams 1-19) and the heading
 *      can be calculated from the orientation of the starboard
 *      and port outer beams. When the center beam and its location
 *      are missing, the ping is ignored. Thus, DATA CAN BE LOST
 *      when this format is read with MB-System programs.
 *   4. Comments can be embedded in the data as lines beginning
 *      with "##".  
 *   5. The depth values are stored as negative numbers (topography
 *      rather than bathymetry). In order to accomodate flagging
 *      of suspect depths, the flagged depths are stored as
 *      positive numbers. This does not affect the internal
 *      MB-System convention of flagging depths with negative
 *      numbers.
 *   6. Information on this format was obtained from IFREMER.
 *
 * The kind value in the mbf_sbifremr_struct indicates whether the
 * mbf_sbifremr_data_struct structure holds data (kind = 1) or an
 * ascii comment record (kind = 2).
 */

/* maximum comment length in characters */
#define MBF_SBIFREMR_MAXLINE 200

/* number of beams in format */
#define MBF_SBIFREMR_NUM_BEAMS	19

/* angle spacing for SeaBeam Classic */
#define MBF_SBIFREMR_ANGLE_SPACING	2.62

struct mbf_sbifremr_struct
	{
	int	kind;		/* comment flag (comment if != 1) */
	short	year;		/* year (4 digits) */
	short	day;		/* julian day (1-366) */
	short	min;		/* minutes from beginning of day (0-1439) */
	short	sec;		/* seconds from beginning of minute (0-59) */
	short	lat2u;		/* number of minutes north of 90S */
	short	lat2b;		/* fraction of minute times 10000 */
	short	lon2u;		/* minutes east of prime meridian */
	short	lon2b;		/* fraction of minute times 10000 */
	unsigned short	sbhdg;	/* SeaBeam gyro heading 
					0 = 0 degrees
					1 = 0.0055 degrees
					16384 = 90 degrees
					65535 = 359.9945 degrees
					0 = 360 degrees */
	short	deph[MBF_SBIFREMR_NUM_BEAMS];	
				/* 16 depths from Sea Beam in meters
					assuming 1500 m/s water velocity */
	short	dist[MBF_SBIFREMR_NUM_BEAMS];	
				/* 16 cross track distances in meters from port
					(negative) to starboard (positive) */
	double	lon[MBF_SBIFREMR_NUM_BEAMS];	
				/* longitudes of beam values */
	double	lat[MBF_SBIFREMR_NUM_BEAMS];	
				/* latitudes of beam values */
	char	comment[MBF_SBIFREMR_MAXLINE];
	};
