/*--------------------------------------------------------------------
 *    The MB-system:	mbf_sbsiomrg.h	1/20/93
 *
 *    Copyright (c) 1993-2024 by
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
 * mbf_sbsiomrg.h defines the data structure used by MBIO functions
 * to store multibeam data read from the  MBF_SBSIOMRG format (MBIO id 11).
 *
 * Author:	D. W. Caress
 * Date:	January 20, 1993
 *
 */
/*
 * Notes on the MBF_SBSIOMRG data format:
 *   1. This data format is used to store 16 beam Sea Beam bathymetry
 *      data.  This format was created and used by the Scripps
 *      Institution of Oceanography; most data files in this format
 *      consist of Sea Beam data collected on the R/V Thomas Washington.
 *   2. The data consist of 100 byte records with 50 2-byte signed
 *      integer words.
 *   3. The 16 depth values are stored uncentered (the depth values
 *      are centered in most formats).
 *   4. Comments can be embedded in the data as 100 byte ascii strings,
 *	where the first two characters must always be "##" so that
 *      the year value is greater than 7000.
 *   5. Data files created in the early 1980's on an IBM 1800 may have
 *      padding records consisting entirely of zeros; these may be
 *      recognized by the year being 0 and should be ignored.
 *   6. Information on this format was obtained from the Geological
 *      Data Center at the Scripps Institution of Oceanography
 *
 * The kind value in the mbf_sbsiomrg_struct indicates whether the
 * mbf_sbsiomrg_data_struct structure holds data (kind = 1) or an
 * ascii comment record (kind = 2).
 *
 * The mbf_sbsiomrg_data_struct structure is a direct representation
 * of the binary data structure used in the MBF_SBSIOMRG format.
 */

#ifndef MBF_SBSIOMRG_H_
#define MBF_SBSIOMRG_H_

/* size of data records */
#define MBF_SBSIOMRG_RECORD_SIZE 100

/* number of beams in raw and processed pings */
#define MB_BEAMS_RAW_SBSIOMRG 16
#define MB_BEAMS_PROC_SBSIOMRG 19

struct mbf_sbsiomrg_data_struct {
	short year;           /* year (4 digits) */
	short day;            /* julian day (1-366) */
	short min;            /* minutes from beginning of day (0-1439) */
	short sec;            /* seconds from beginning of minute (0-59) */
	short lon2u;          /* minutes east of prime meridian */
	short lon2b;          /* fraction of minute times 10000 */
	short lat2u;          /* number of minutes north of 90S */
	short lat2b;          /* fraction of minute times 10000 */
	short spare1[3];      /* unused */
	unsigned short sbtim; /* Sea Beam computer clock time in 10ths of
	              seconds from start of hour (0-3600) */
	unsigned short sbhdg; /* Sea Beam gyro heading
	              0 = 0 degrees
	              1 = 0.0055 degrees
	              16384 = 90 degrees
	              65535 = 359.9945 degrees
	              0 = 360 degrees */
	short deph[16];       /* 16 depths from Sea Beam in meters
	                  assuming 1500 m/s water velocity */
	short dist[16];       /* 16 cross track distances in meters from port
	                  (negative) to starboard (positive) */
	short spare2[5];      /* unused */
};

struct mbf_sbsiomrg_struct {
	int kind;
	struct mbf_sbsiomrg_data_struct data;
};

#endif  /* MBF_SBSIOMRG_H_ */
