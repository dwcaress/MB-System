/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_mr1.h	7/19/94
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
 * mbsys_mr1.h defines the data structures used by MBIO functions
 * to store data from the MR1 towed sonar.
 * The data formats which are commonly used to store MR1
 * data in files include
 *      MBF_MR1PRHIG : MBIO ID 61
 *
 * Author:	D. W. Caress
 * Date:	July 19, 1994
 *
 */
/*
 * Notes on the MBSYS_MR1 data structure:
 *   1. The MR1 post processing format uses the xdr external
 *      data representation for portability.
 *   2. The data stream consists of a file header followed
 *      by individual pings.
 *   3. The file header contains a comment string and the
 *      number of pings. The comment string is broken up
 *      into multiple comments by MBIO on reading; the comments
 *      are concatenated into a single string on writing.
 *   4. The pings each contain a header plus the bathymetry and/or
 *      sidescan data.
 *   6. The data structure defined below includes all of the values
 *      which are passed in the MR1 post processing format.
 *   7. The data structure defined below also includes travel
 *      time values for each bathymetry beam - this is an
 *      addition to the HIG MR1 post processing format.
 */

#ifndef MBSYS_MR1_H_
#define MBSYS_MR1_H_

/* maximum number of bathymetry beams per side for MR1 */
#define MBSYS_MR1_BEAMS_SIDE 1500

/* maximum number of sidescan pixels per side for MR1 */
#define MBSYS_MR1_PIXELS_SIDE 3500

/* maximum number of bathymetry beams for MR1 */
#define MBSYS_MR1_BEAMS (2 * MBSYS_MR1_BEAMS_SIDE + 3)

/* maximum number of sidescan pixels output for MR1 */
#define MBSYS_MR1_PIXELS (2 * MBSYS_MR1_PIXELS_SIDE + 3)

/* maximum length of comment */
#define MBSYS_MR1_MAXLINE 200

/* angle from vertical of MR1 transducers */
#define MBSYS_MR1_XDUCER_ANGLE 50.0

struct mbsys_mr1_struct {
	/* type of data record */
	int kind;

	/* file header info */
	int mf_magic; /* magic cookie */
	int mf_count; /* number of objects */
	char *mf_log; /* processing log */

	/* ping header */
	int sec;            /* timestamp */
	int usec;           /* timestamp */
	double png_lon;     /* longitude (deg) */
	double png_lat;     /* latitude (deg) */
	float png_course;   /* course determined from nav (deg) */
	float png_compass;  /* compass heading of vehicle
	                0=N,90=E, etc. (deg) */
	float png_prdepth;  /* pressure depth (m) */
	float png_alt;      /* altitude of vehicle (m) */
	float png_pitch;    /* vehicle pitch (deg) */
	float png_roll;     /* vehicle roll (deg) */
	float png_temp;     /* water temperature (deg) */
	float png_atssincr; /* across-track sidescan increment (m) */
	float png_tt;       /* nadir travel time (s) */

	/* port settings */
	float port_trans[2]; /* transmitter settings (units?) */
	float port_gain;     /* gain setting (units?) */
	float port_pulse;    /* pulse length (units?) */
	int port_btycount;   /* number of valid bathymetry samples */
	int port_btypad;     /* number of invalid trailing pad samples */
	float port_ssoffset; /* across-track distance to
	             first sidescan sample */
	int port_sscount;    /* number of valid sidescan samples */
	int port_sspad;      /* number of invalid trailing pad samples */

	/* starboard settings */
	float stbd_trans[2]; /* transmitter settings (units?) */
	float stbd_gain;     /* gain setting (units?) */
	float stbd_pulse;    /* pulse length (units?) */
	int stbd_btycount;   /* number of valid bathymetry samples */
	int stbd_btypad;     /* number of invalid trailing pad samples */
	float stbd_ssoffset; /* across-track distance to
	             first sidescan sample */
	int stbd_sscount;    /* number of valid sidescan samples */
	int stbd_sspad;      /* number of invalid trailing pad samples */

	/* bathymetry */
	float bath_acrosstrack_port[MBSYS_MR1_BEAMS_SIDE];
	float bath_port[MBSYS_MR1_BEAMS_SIDE];
	float tt_port[MBSYS_MR1_BEAMS_SIDE];
	float angle_port[MBSYS_MR1_BEAMS_SIDE];
	float bath_acrosstrack_stbd[MBSYS_MR1_BEAMS_SIDE];
	float bath_stbd[MBSYS_MR1_BEAMS_SIDE];
	float tt_stbd[MBSYS_MR1_BEAMS_SIDE];
	float angle_stbd[MBSYS_MR1_BEAMS_SIDE];

	/* sidescan */
	float ss_port[MBSYS_MR1_PIXELS_SIDE];
	float ss_stbd[MBSYS_MR1_PIXELS_SIDE];

	/* comment */
	char comment[MBSYS_MR1_MAXLINE];
};

/* system specific function prototypes */
int mbsys_mr1_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_mr1_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_mr1_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_mr1_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                      double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss, char *beamflag,
                      double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss,
                      double *ssacrosstrack, double *ssalongtrack, char *comment, int *error);
int mbsys_mr1_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
                     double navlat, double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath,
                     double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
                     double *ssalongtrack, char *comment, int *error);
int mbsys_mr1_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
                     double *angles_forward, double *angles_null, double *heave, double *alongtrack_offset, double *draft,
                     double *ssv, int *error);
int mbsys_mr1_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error);
int mbsys_mr1_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                               double *altitude, int *error);
int mbsys_mr1_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                          double *navlat, double *speed, double *heading, double *draft, double *roll, double *pitch,
                          double *heave, int *error);
int mbsys_mr1_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon, double navlat,
                         double speed, double heading, double draft, double roll, double pitch, double heave, int *error);
int mbsys_mr1_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error);

#endif  /* MBSYS_MR1_H_ */
