/*--------------------------------------------------------------------
 *    The MB-system:	mbf_sbsiocen.h	1/20/93
 *	$Id: mbf_sbsiocen.h,v 4.0 1994-03-06 00:01:56 caress Exp $
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
 * mbf_sbsiocen.h defines the data structure used by MBIO functions
 * to store multibeam data read from the  MBF_SBSIOCEN format (MBIO id 12).  
 *
 * Author:	D. W. Caress
 * Date:	January 20, 1993
 * $Log: not supported by cvs2svn $
 * Revision 4.2  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.1  1994/02/17  21:19:08  caress
 * Updated associated MBIO format id in comments.
 *
 * Revision 4.0  1994/02/17  20:59:54  caress
 * First cut at new version. No changes.
 *
 * Revision 3.0  1993/05/14  22:52:50  sohara
 * initial version
 *
 */
/*
 * Notes on the MBF_SBSIOCEN data format:
 *   1. This data format is used to store 16 beam Sea Beam bathymetry
 *      data.  This format was created and used by the Scripps
 *      Institution of Oceanography; most data files in this format
 *      consist of Sea Beam data collected on the R/V Thomas Washington.
 *   2. The data consist of 112 byte records including a 4-character 
 *      string, 2-byte integers, and 4-byte integers.
 *   3. The 16 depth values are stored centered in 19 value arrays.  The
 *      center beam is in word 10 of the depth and distance arrays.
 *   4. Comments can be embedded in the data as 112 byte ascii strings,
 *	where the first two characters must always be "##" to set
 *      the comment flag.  
 *   5. Information on this format was obtained from the Geological
 *      Data Center at the Scripps Institution of Oceanography
 *
 * The kind value in the mbf_sbsiocen_struct indicates whether the
 * mbf_sbsiocen_data_struct structure holds data (kind = 1) or an
 * ascii comment record (kind = 2).
 *
 * The mbf_sbsiocen_data_struct structure is a direct representation of 
 * the binary data structure used in the MBF_SBSIOCEN format.
 */

struct mbf_sbsiocen_data_struct
	{
	char	flag[4];	/* comment flag (## flags comment record) */
	short	year;		/* year (4 digits) */
	short	day;		/* julian day (1-366) */
	short	min;		/* minutes from beginning of day (0-1439) */
	short	sec;		/* seconds from beginning of minute (0-59) */
	int	major;		/* navigation error ellipse major axis */
	int	minor;		/* navigation error ellipse minor axis */
	short	axis;		/* navigation error ellipse major axis angle */
	short	lat2u;		/* number of minutes north of 90S */
	short	lat2b;		/* fraction of minute times 10000 */
	short	lon2u;		/* minutes east of prime meridian */
	short	lon2b;		/* fraction of minute times 10000 */
	unsigned short	sbtim;	/* Sea Beam computer clock time in 10ths of
					seconds from start of hour (0-3600) */
	unsigned short	sbhdg;	/* Sea Beam gyro heading 
					0 = 0 degrees
					1 = 0.0055 degrees
					16384 = 90 degrees
					65535 = 359.9945 degrees
					0 = 360 degrees */
	short	deph[19];	/* 16 depths from Sea Beam in meters
					assuming 1500 m/s water velocity */
	short	dist[19];	/* 16 cross track distances in meters from port
					(negative) to starboard (positive) */
	short	spare;		/* unused */
	};

struct mbf_sbsiocen_struct
	{
	int	kind;
	struct mbf_sbsiocen_data_struct data;
	};

