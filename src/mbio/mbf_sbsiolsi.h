/*--------------------------------------------------------------------
 *    The MB-system:	mbf_sbsiolsi.h	1/20/93
 *
 *    Copyright (c) 1993-2025 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbf_sbsiolsi.h defines the data structure used by MBIO functions
 * to store multibeam data read from the  MBF_SBSIOLSI format (MBIO id 13).
 *
 * Author:	D. W. Caress
 * Date:	January 20, 1993
 *
 */
/*
 * Notes on the MBF_SBSIOLSI data format:
 *   1. This data format is used to store 16 beam Sea Beam bathymetry
 *      data.  This format was created and used by the Scripps
 *      Institution of Oceanography; most data files in this format
 *      consist of Sea Beam data collected on the R/V Thomas Washington.
 *   2. This data format is no longer in use and is supported only to
 *      provide a means of dealing with a few old data tapes.
 *   3. The data consist of 100 byte records consisting entirely of
 *      2-byte integers.
 *   4. The 16 depth values are stored centered in 19 value arrays.  The
 *      center beam is in word 10 of the depth and distance arrays.
 *   5. Comments can be embedded in the data as 100 byte ascii strings,
 *	where the first two characters must always be "cc" so that the
 *      first depth value is 25443.
 *   6. Information on this format was obtained by deciphering some
 *      old data tapes; there could be more to know about this format,
 *      but its probably not worth worrying about.
 *
 * The kind value in the mbf_sbsiolsi_struct indicates whether the
 * mbf_sbsiolsi_data_struct structure holds data (kind = 1) or an
 * ascii comment record (kind = 2).
 *
 * The mbf_sbsiolsi_data_struct structure is a direct representation
 * of the binary data structure used in the MBF_SBSIOLSI format.
 */

#ifndef MBF_SBSIOLSI_H_
#define MBF_SBSIOLSI_H_

struct mbf_sbsiolsi_data_struct {
	short deph[19];       /* 16 depths from Sea Beam in meters
	                  assuming 1500 m/s water velocity */
	short dist[19];       /* 16 cross track distances in meters from port
	                  (negative) to starboard (positive) */
	short axis;           /* navigation error ellipse major axis angle */
	short major;          /* navigation error ellipse major axis */
	short minor;          /* navigation error ellipse minor axis */
	unsigned short sbhdg; /* Sea Beam gyro heading
	              0 = 0 degrees
	              1 = 0.0055 degrees
	              16384 = 90 degrees
	              65535 = 359.9945 degrees
	              0 = 360 degrees */
	short lat2b;          /* fraction of minute times 10000 */
	short lat2u;          /* number of minutes north of 90S */
	short lon2b;          /* fraction of minute times 10000 */
	short lon2u;          /* minutes east of prime meridian */
	short sec;            /* seconds from beginning of minute (0-59) */
	short min;            /* minutes from beginning of day (0-1439) */
	short day;            /* julian day (1-366) */
	short year;           /* year (4 digits) */
};

struct mbf_sbsiolsi_struct {
	int kind;
	struct mbf_sbsiolsi_data_struct data;
};

#endif  /* MBF_SBSIOLSI_H_ */
