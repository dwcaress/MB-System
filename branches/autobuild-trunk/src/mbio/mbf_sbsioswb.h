/*--------------------------------------------------------------------
 *    The MB-system:	mbf_sbsioswb.h	9/18/94
 *	$Id: mbf_sbsioswb.h 1917 2012-01-10 19:25:33Z caress $
 *
 *    Copyright (c) 1994-2012 by
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
 * mbf_sbsioswb.h defines the data structure used by MBIO functions
 * to store multibeam data read from the  MBF_SBSIOSWB format (MBIO id 16).  
 *
 * Author:	D. W. Caress
 * Date:	January 20, 1993
 * $Log: mbf_sbsioswb.h,v $
 * Revision 5.3  2009/03/08 09:21:00  caress
 * Fixed problem reading and writing format 16 (MBF_SBSIOSWB) data on little endian systems.
 *
 * Revision 5.2  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.1  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.0  2000/12/01 22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.3  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.2  1998/10/05  18:32:27  caress
 * MB-System version 4.6beta
 *
 * Revision 4.1  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.1  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.0  1994/10/21  12:35:07  caress
 * Release V4.0
 *
 * Revision 4.0  1994/10/21  12:35:07  caress
 * Release V4.0
 *
 * Revision 1.1  1994/10/21  12:20:01  caress
 * Initial revision
 *
 *
 */
/*
 * Notes on the MBF_SBSIOSWB data format:
 *   1. This data format is used to store 16 beam Sea Beam bathymetry
 *      data.  This format was created and used by the Scripps
 *      Institution of Oceanography; most data files in this format
 *      consist of Sea Beam data collected on the R/V Thomas Washington.
 *      This format is one of the "swathbathy" formats created by
 *      Jim Charters of Scripps.
 *   2. The data records consist of three logical records: the header
 *      record, the sensor specific record and the data record.  
 *   3. The header record consists of 36 bytes, including the sizes
 *      of the following sensor specific and data records.
 *   4. The sensor specific records are 4 bytes long.  
 *   5. The data record lengths are variable.
 *   6. Comments are included in text records, which are of variable
 *      length.
 *   7. Information on this format was obtained from the Geological
 *      Data Center and the Shipboard Computer Group at the Scripps 
 *      Institution of Oceanography
 *
 * The kind value in the mbf_sbsioswb_struct indicates whether the
 * mbf_sbsioswb_data_struct structure holds data (kind = 1) or an
 * ascii comment record (kind = 2).
 *
 * The mbf_sbsioswb_data_struct structure is a direct representation 
 * of the binary data structure used in the MBF_SBSIOSWB format.
 */

/* number of beams in pings */
#define	MB_BEAMS_SBSIOSWB	19

/* size in bytes of header records */
#define	MB_SBSIOSWB_HEADER_SIZE	36

struct mbf_sbsioswb_bath_struct
	{
	short	bath;
	short	bath_acrosstrack;
	};

struct mbf_sbsioswb_struct
	{
	int	kind;
	short	year;		/* year (4 digits) */
	short	day;		/* julian day (1-366) */
	short	min;		/* minutes from beginning of day (0-1439) */
	short	sec;		/* seconds from beginning of minute (0-59) */
	int	lat;		/* 1e-7 degrees from equator */
	int	lon;		/* 1e-7 degrees from prime meridian */
	short	heading;	/* heading in 0.1 degrees */
	short	course;		/* course in 0.1 degrees */
	short	speed;		/* fore-aft speed in 0.1 knots */
	short	speed_ps;	/* port-starboard speed in 0.1 knots */
	short	quality;	/* quality value, 0 good, bigger bad */
	short	sensor_size;	/* size of sensor specific record in bytes */
	short	data_size;	/* size of data record in bytes */
	char	speed_ref[2];	/* speed reference */
	char	sensor_type[2];	/* sensor type */
	char	data_type[2];	/* type of data recorded */
	short	eclipse_time;	/* time of day from eclipse computer */
	short	eclipse_heading;	/* heading at time of ping */
	short	beams_bath;	/* number of bathymetry beams */
	short	scale_factor;	/* scale factor */
	struct mbf_sbsioswb_bath_struct bath_struct[MB_BEAMS_SBSIOSWB];
	char	comment[MBSYS_SB_MAXLINE];
	};
