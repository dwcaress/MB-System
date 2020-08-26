/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_sb.h	2/17/93
 *
 *    Copyright (c) 1993-2020 by
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
 * mbsys_sb.h defines the data structures used by MBIO functions
 * to store data from the 16-beam SeaBeam multibeam sonar systems.
 * The data formats which are commonly used to store SeaBeam
 * data in files include
 *      MBF_SBSIOMRG : MBIO ID 11
 *      MBF_SBSIOCEN : MBIO ID 12
 *      MBF_SBSIOLSI : MBIO ID 13
 *      MBF_SBURICEN : MBIO ID 14
 *
 * Author:	D. W. Caress
 * Date:	February 17, 1993
 *
 */
/*
 * Notes on the MBSYS_SB data structure:
 *   1. SeaBeam multibeam systems output raw data in 16 uncentered
 *      beams.  MBIO and most data formats store the data as 19
 *      centered beams.
 *   5. The kind value in the mbsys_sb_struct indicates whether the
 *      mbsys_sb_data_struct structure holds data from a ping or
 *      data from a comment:
 *        kind = 1 : data from a ping
 *        kind = 2 : comment
 *   6. The data structure defined below includes all of the values
 *      which are passed in SeaBeam records.
 */

#ifndef MBSYS_SB_H_
#define MBSYS_SB_H_

/* maximum line length in characters */
#define MBSYS_SB_MAXLINE 200

/* number of beams for hydrosweep */
#define MBSYS_SB_BEAMS 19

struct mbsys_sb_struct {
	/* type of data record */
	int kind;

	/* position */
	unsigned short lon2u; /* minutes east of prime meridian */
	unsigned short lon2b; /* fraction of minute times 10000 */
	unsigned short lat2u; /* number of minutes north of 90S */
	unsigned short lat2b; /* fraction of minute times 10000 */

	/* time stamp */
	int year; /* year (4 digits) */
	int day;  /* julian day (1-366) */
	int min;  /* minutes from beginning of day (0-1439) */
	int sec;  /* seconds from beginning of minute (0-59) */

	/* depths and distances */
	int dist[MBSYS_SB_BEAMS]; /* 19 depths from SeaBeam in meters
	                assuming 1500 m/s water velocity */
	int deph[MBSYS_SB_BEAMS]; /* 19 cross track distances in
	                meters from port (negative)
	                to starboard (positive) */

	/* additional values */
	unsigned short sbtim; /* SeaBeam computer clock time in 10ths of
	              seconds from start of hour (0-3600) */
	unsigned short sbhdg; /* SeaBeam gyro heading
	              0 = 0 degrees
	              1 = 0.0055 degrees
	              16384 = 90 degrees
	              65535 = 359.9945 degrees
	              0 = 360 degrees */
	short axis;           /* navigation error ellipse major axis angle */
	short major;          /* navigation error ellipse major axis */
	short minor;          /* navigation error ellipse minor axis */

	/* comment */
	char comment[MBSYS_SB_MAXLINE];
};

/* system specific function prototypes */
int mbsys_sb_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_sb_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_sb_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_sb_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                     double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss, char *beamflag,
                     double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss,
                     double *ssacrosstrack, double *ssalongtrack, char *comment, int *error);
int mbsys_sb_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
                    double navlat, double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath,
                    double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
                    double *ssalongtrack, char *comment, int *error);
int mbsys_sb_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error);
int mbsys_sb_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
                    double *angles_forward, double *angles_null, double *heave, double *alongtrack_offset, double *draft,
                    double *ssv, int *error);
int mbsys_sb_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth, double *altitude,
                              int *error);
int mbsys_sb_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                         double *navlat, double *speed, double *heading, double *draft, double *roll, double *pitch,
                         double *heave, int *error);
int mbsys_sb_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon, double navlat,
                        double speed, double heading, double draft, double roll, double pitch, double heave, int *error);
int mbsys_sb_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error);

#endif  /* MBSYS_SB_H_ */
