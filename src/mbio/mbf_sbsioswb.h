/*--------------------------------------------------------------------
 *    The MB-system:	mbf_sbsioswb.h	9/18/94
 *
 *    Copyright (c) 1994-2025 by
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
 * mbf_sbsioswb.h defines the data structure used by MBIO functions
 * to store multibeam data read from the  MBF_SBSIOSWB format (MBIO id 16).
 *
 * Author:	D. W. Caress
 * Date:	January 20, 1993
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

#ifndef MBF_SBSIOSWB_H_
#define MBF_SBSIOSWB_H_

/* number of beams in pings */
#define MB_BEAMS_SBSIOSWB 19

/* size in bytes of header records */
#define MB_SBSIOSWB_HEADER_SIZE 36

struct mbf_sbsioswb_bath_struct {
	short bath;
	short bath_acrosstrack;
};

struct mbf_sbsioswb_struct {
	int kind;
	short year;            /* year (4 digits) */
	short day;             /* julian day (1-366) */
	short min;             /* minutes from beginning of day (0-1439) */
	short sec;             /* seconds from beginning of minute (0-59) */
	int lat;               /* 1e-7 degrees from equator */
	int lon;               /* 1e-7 degrees from prime meridian */
	short heading;         /* heading in 0.1 degrees */
	short course;          /* course in 0.1 degrees */
	short speed;           /* fore-aft speed in 0.1 knots */
	short speed_ps;        /* port-starboard speed in 0.1 knots */
	short quality;         /* quality value, 0 good, bigger bad */
	short sensor_size;     /* size of sensor specific record in bytes */
	short data_size;       /* size of data record in bytes */
	char speed_ref[2];     /* speed reference */
	char sensor_type[2];   /* sensor type */
	char data_type[2];     /* type of data recorded */
	short eclipse_time;    /* time of day from eclipse computer */
	short eclipse_heading; /* heading at time of ping */
	short beams_bath;      /* number of bathymetry beams */
	short scale_factor;    /* scale factor */
	struct mbf_sbsioswb_bath_struct bath_struct[MB_BEAMS_SBSIOSWB];
	char comment[MBSYS_SB_MAXLINE];
};

#endif  /* MBF_SBSIOSWB_H_ */
