/*--------------------------------------------------------------------
 *    The MB-system:	mbf_mstiffss.h	10/14/94
 *	$Id: mbf_mstiffss.h,v 5.1 2002-09-18 23:32:59 caress Exp $
 *
 *    Copyright (c) 1998, 2000, 2002 by
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
 * mbf_mstiffss.h defines the data structure used by MBIO functions
 * to store sidescan data read from the MBF_MSTIFFSS format (MBIO id 131).  
 *
 * Author:	D. W. Caress
 * Date:	April 7,  1998
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.0  2000/12/01 22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.1  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.0  1998/10/05  18:30:03  caress
 * MB-System version 4.6beta
 *
 * Revision 1.1  1998/10/05  18:22:40  caress
 * Initial revision
 *
 * Revision 1.1  1998/10/05  17:46:15  caress
 * Initial revision
 *
 *
 *
 */
/*
 * Notes on the MBF_MSTIFFSS data format:
 *   1. The MSTIFF data format is used to store raw sidescan data from 
 *      Sea Scan sidescan sonars. This format is a variant of
 *      the TIFF image format with navigation and other information
 *      embedded within the file.
 *   2. The file structure consists of a bunch of pointers to
 *      data objects at various arbitrary locations within the
 *      file. The header contains a pointer to the location of
 *      the "image file directory", which in turn contains
 *      pointers to the locations of data arrays within the file.
 *   3. As far as MB-System is concerned,  this is a read-only
 *      data format.
 *   4. The raw sidescan data in the file consists of 1000 pings.
 *      Each ping produces two 512 sample arrays - one for
 *      each side (port and starboard).
 *   5. The sidescan data is not slant range corrected - the 
 *      bottom detect and slant range correction is done on
 *      input by MBIO. The data stored internally by MBIO
 *      is slant range corrected.
 *   6. The MSTIFF files contain lots of information not used by
 *      MBIO,  including images of the data derived from a
 *      realtime display.
 *   7. Comments are not supported in this format.
 */
 
/* number of sidescan pixels for Sea Scan sidescan sonars */
#define MBF_MSTIFFSS_PIXELS	1024

/* threshold of sidescan values for detecting bottom return */
#define MBF_MSTIFF_TRANSMIT_BINS	10
#define MBF_MSTIFF_BOTTOM_THRESHOLD	10

struct mbf_mstiffss_struct
	{
	/* time stamp */
	double	time_d;		/* unix time */

	/* position */
	double	lat;		/* latitude in degrees */
	double	lon;		/* longitude in degrees */

	/* other values */
	double	heading;	    /* heading in degrees */
	double	speed;		    /* fore-aft speed in km/hr */
	double	altitude;	    /* altitude in m */
	double	slant_range_max;    /* seconds */
	double	range_delay;	    /* seconds */
	double	sample_interval;    /* seconds */

	/* sidescan data */
	int	pixels_ss;	/* number of pixels */
	unsigned char	ss[MBF_MSTIFFSS_PIXELS];
	double	ssacrosstrack[MBF_MSTIFFSS_PIXELS];
	};
