/*--------------------------------------------------------------------
 *    The MB-system:	mbf_sburicen.h	1/20/93
 *	$Id$
 *
 *    Copyright (c) 1993-2014 by
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
 * mbf_sburicen.h defines the data structure used by MBIO functions
 * to store multibeam data read from the  MBF_SBURICEN format (MBIO id 14).
 *
 * Author:	D. W. Caress
 * Date:	January 20, 1993
 * $Log: mbf_sburicen.h,v $
 * Revision 5.2  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.1  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.0  2000/12/01 22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.4  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.3  1998/10/05  18:32:27  caress
 * MB-System version 4.6beta
 *
 * Revision 4.2  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.2  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.1  1994/10/21  12:20:01  caress
 * Release V4.0
 *
 * Revision 4.1  1994/10/21  12:20:01  caress
 * Release V4.0
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
 * Revision 3.0  1993/05/14  22:54:13  sohara
 * initial version
 *
 */
/*
 * Notes on the MBF_SBURICEN data format:
 *   1. This data format is used to store 16 beam Sea Beam bathymetry
 *      data.  This format was created and used by the Ocean Mapping
 *      Development Center at the Graduate School of Oceanography of
 *      the University of Rhode Islande; most data files in this format
 *      consist of Sea Beam data collected on the R/V Robert Conrad or
 *      the R/V Atlantis II.
 *   2. The data consist of 102 byte records consisting entirely of
 *      2-byte integers.
 *   3. The 16 depth values are stored centered in 19 value arrays.  The
 *      center beam is in word 10 of the depth and distance arrays.
 *   4. Comments can be embedded in the data as 100 byte ascii strings,
 *	where the first two characters must always be "cc" so that the
 *      first depth value is 25443.
 *   5. Information on this format was obtained from the Geological
 *      Data Center at the Scripps Institution of Oceanography
 *
 * The kind value in the mbf_sburicen_struct indicates whether the
 * mbf_sburicen_data_struct structure holds data (kind = 1) or an
 * ascii comment record (kind = 2).
 *
 * The mbf_sburicen_data_struct structure is a direct representation
 * of the binary data structure used in the MBF_SBURICEN format.
 */

struct mbf_sburicen_data_struct
	{
	short	deph[19];	/* 16 depths from Sea Beam in meters
					assuming 1500 m/s water velocity */
	short	dist[19];	/* 16 cross track distances in meters from port
					(negative) to starboard (positive) */
	short	axis;		/* navigation error ellipse major axis angle */
	short	major;		/* navigation error ellipse major axis */
	short	minor;		/* navigation error ellipse minor axis */
	unsigned short	sbhdg;	/* Sea Beam gyro heading
					0 = 0 degrees
					1 = 0.0055 degrees
					16384 = 90 degrees
					65535 = 359.9945 degrees
					0 = 360 degrees */
	short	lat2b;		/* fraction of minute times 10000 */
	short	lat2u;		/* number of minutes north of 90S */
	short	lon2b;		/* fraction of minute times 10000 */
	short	lon2u;		/* minutes east of prime meridian */
	short	sec;		/* seconds from beginning of minute (0-59) */
	short	min;		/* minutes from beginning of day (0-1439) */
	short	day;		/* julian day (1-366) */
	short	year;		/* year (4 digits) */
	unsigned short	sbtim;	/* Sea Beam computer clock time in 10ths of
					seconds from start of hour (0-3600) */
	};

struct mbf_sburicen_struct
	{
	int	kind;
	struct mbf_sburicen_data_struct data;
	};
