/*--------------------------------------------------------------------
 *    The MB-system:	mbf_mbldeoih.h	3.00	1/20/93
 *	$Id: mbf_mbldeoih.h,v 3.0 1993-05-14 22:52:30 sohara Exp $
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
 * mbf_mbldeoih.h defines the data structures used by MBIO functions
 * to store multibeam data read from the  MBF_MBLDEOIH format (MBIO id 10).  
 *
 * Author:	D. W. Caress
 * Date:	January 20, 1993
 * $Log: not supported by cvs2svn $
 */
/*
 * Notes on the MBF_MBLDEOIH data format:
 *   1. This data format is used to store multibeam bathymetry
 *      and/or backscatter data with arbitrary numbers of beams.
 *	This format was created by the Lamont-Doherty Earth
 *	Observatory to serve as a general purpose archive format for
 *	processed multibeam data.
 *   2. The data consist of variable length binary records encoded
 *	entirely in 2-byte integers.
 *   3. Each data record has two parts.  First there is a 30-byte
 *      header section containing the time stamp, navigation, and
 *      the numbers of depth and backscatter values.  The second
 *      part contains the depth and backscatter values.  The number 
 *      of depth values may be different from the number of 
 *      backscatter values.  
 *   4. Both the depth and backscatter arrays are centered.
 *   5. Comments can be embedded in the data as 128 byte ascii strings,
 *	where the first two characters of the associated header
 *      must be set to "##" (the other 28 bytes of the header are
 *      unused in a comment record) .
 *
 * The kind value in the mbf_sbsiocen_struct indicates whether the
 * mbf_sbsiocen_data_struct structure holds data (kind = 1) or an
 * ascii comment record (kind = 0).
 *
 * These two structures are direct representations of the binary data 
 * structures used in the MBF_MBLDEOIH format.
 */

/* define maximum number of beams */
#define	MBF_MBLDEOIH_MAX_BEAMS	200

struct mbf_mbldeoih_header_struct
	{
	unsigned short	flag;	/* data ('dd'=25700) or comment ('##'=8995) */
	short	year;		/* year (4 digits) */
	short	day;		/* julian day (1-366) */
	short	min;		/* minutes from beginning of day (0-1439) */
	short	sec;		/* seconds from beginning of minute (0-59) */
	unsigned short	lon2u;	/* minutes east of prime meridian */
	unsigned short	lon2b;	/* fraction of minute times 10000 */
	unsigned short	lat2u;	/* number of minutes north of 90S */
	unsigned short	lat2b;	/* fraction of minute times 10000 */
	unsigned short	heading;/* heading:
					0 = 0 degrees
					1 = 0.0055 degrees
					16384 = 90 degrees
					65535 = 359.9945 degrees
					0 = 360 degrees */
	unsigned short	speed;	/* km/s X100 */
	short	beams_bath;	/* number of depth values in meters/bathscale */
	short	beams_back;	/* number of backscatter values */
	short	bathscale;	/* 1000Xscale where depth=bathXscale */
	short	backscale;	/* 1000Xscale where backscatter=backXscale */
	};

struct mbf_mbldeoih_data_struct
	{
	short	bath[MBF_MBLDEOIH_MAX_BEAMS];
	short	bathdist[MBF_MBLDEOIH_MAX_BEAMS];
	short	back[MBF_MBLDEOIH_MAX_BEAMS];
	short	backdist[MBF_MBLDEOIH_MAX_BEAMS];
	};

struct mbf_mbldeoih_struct
	{
	int	kind;
	struct mbf_mbldeoih_header_struct header;
	struct mbf_mbldeoih_data_struct data;
	char	comment[128];
	};
