/*--------------------------------------------------------------------
 *    The MB-system:	mbf_mbldeoih.h	1/20/93
 *	$Id: mbf_mbldeoih.h,v 4.3 1998-10-05 17:46:15 caress Exp $
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
 * mbf_mbldeoih.h defines the data structures used by MBIO functions
 * to store multibeam data read from the  MBF_MBLDEOIH format (MBIO id 61).  
 *
 * Author:	D. W. Caress
 * Date:	January 20, 1993
 * $Log: not supported by cvs2svn $
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
 * Revision 4.1  1994/02/21  03:23:28  caress
 * Changed location of amp array pointer in
 * struct mbf_mbldeoih_data_struct .
 *
 * Revision 4.0  1994/02/17  21:11:32  caress
 * First cut at new version.  Recast format to include both
 * beam amplitude and sidescan data.  I hope no one has used
 * the old version of this format, as the files will be
 * orphaned!!!
 *
 * Revision 3.0  1993/05/14  22:52:30  sohara
 * initial version
 *
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
 *      the numbers of depth, beam amplitude, and sidescan values.  The 
 *      second part contains the depth and backscatter values.  The number 
 *      of depth and beam amplitude values is generally different 
 *      from the number of sidescan values.  
 *   4. All data arrays are centered.
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
#define	MBF_MBLDEOIH_MAX_PIXELS	10000

struct mbf_mbldeoih_header_struct
	{
	unsigned short	flag;	/* data ('dd'=25700) or comment ('##'=8995) */
	short	year;		/* year (4 digits) */
	short	day;		/* julian day (1-366) */
	short	min;		/* minutes from beginning of day (0-1439) */
	short	sec;		/* seconds from beginning of minute (0-59) */
	short	msec;		/* milliseconds */
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
	short	beams_bath;	/* number of depth values */
	short	beams_amp;	/* number of amplitude values */
	short	pixels_ss;	/* number of sidescan pixels */
	short	depth_scale;	/* 1000 X scale where depth = bath X scale */
	short	distance_scale;	/* 1000 X scale where distance = dist X scale */
	short	transducer_depth; /* scaled by depth_scale */
	short	altitude;	/* scaled by depth_scale */
	};

struct mbf_mbldeoih_data_struct
	{
	unsigned char	beamflag[MBF_MBLDEOIH_MAX_BEAMS];
	short	bath[MBF_MBLDEOIH_MAX_BEAMS];
	short	bath_acrosstrack[MBF_MBLDEOIH_MAX_BEAMS];
	short	bath_alongtrack[MBF_MBLDEOIH_MAX_BEAMS];
	short	amp[MBF_MBLDEOIH_MAX_BEAMS];
	short	ss[MBF_MBLDEOIH_MAX_PIXELS];
	short	ss_acrosstrack[MBF_MBLDEOIH_MAX_PIXELS];
	short	ss_alongtrack[MBF_MBLDEOIH_MAX_PIXELS];
	};

struct mbf_mbldeoih_struct
	{
	int	kind;
	struct mbf_mbldeoih_header_struct header;
	struct mbf_mbldeoih_data_struct data;
	char	comment[128];
	};
