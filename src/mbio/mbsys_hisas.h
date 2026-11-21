/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_hisas.h	11/22/2026
 *
 *    Copyright (c) 2026 by
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
/**
 * @file
 * @brief Define the data structures used by MBIO functions
 * to store sidescan data from Kongsberg HISAS synthetic aperture
 * sonars recorded in Kongsberg *.all files.
 * @details The formats associated with this i/o module are:
 *  - MBF_HISASALL (MBIO ID 212)
 */
/*
 * Notes on the MBSYS_HISAS data structure:
 *   1. HISAS sidescan is stored in Kongsberg *.all files: datagram type '"'
 *      holds an XTF file header (channel information), type '%' holds an
 *      XTF sidescan ping record and type '>' holds the vehicle navigation.
 *      The format is read only.
 *   2. The time, position, depth, altitude and attitude fields of the XTF
 *      ping records are zero. The ping time is taken from the Kongsberg
 *      datagram header, and the navigation, attitude, sensor depth and
 *      altitude are interpolated at the ping time from the '>' records.
 *   3. Both sides are stored from near range to far range.
 *   4. One sidescan channel is kept per side (the first channel of that
 *      side in the ping record), as float magnitudes.
 *   5. A single bathymetry value is given per ping: the seafloor depth
 *      below the sonar, i.e. sensor depth plus altitude (positive down).
 */

#ifndef MBSYS_HISAS_H_
#define MBSYS_HISAS_H_

/* sonar models (Kongsberg EM model numbers in the datagram headers) */
#define MBSYS_HISAS_UNKNOWN 0
#define MBSYS_HISAS_HISAS1032 11032
#define MBSYS_HISAS_HISAS1034 11034
#define MBSYS_HISAS_HISAS2040 12040

/* sides */
#define MBSYS_HISAS_PORT 0
#define MBSYS_HISAS_STBD 1

/* maximum number of beams and pixels */
#define MBSYS_HISAS_MAXBEAMS 1
#define MBSYS_HISAS_MAXRAWPIXELS 32768
#define MBSYS_HISAS_MAXPIXELS (2 * MBSYS_HISAS_MAXRAWPIXELS)
#define MBSYS_HISAS_COMMENT_LENGTH 256
#define MBSYS_HISAS_CHANNELNAME_LENGTH 17

/* internal data structure */
struct mbsys_hisas_struct {
	/* type of data record */
	int kind; /* Data kind */

	/* type of sonar */
	int sonar;           /* Kongsberg model number (e.g. 12040 = HISAS 2040) */
	int serial;          /* sonar serial number */
	unsigned int ping_number;

	/* ping time, navigation and attitude (vehicle navigation, interpolated
	    at the ping time from the '>' records) */
	double time_d;      /* epoch seconds */
	double longitude;   /* degrees */
	double latitude;    /* degrees */
	double speed;       /* km/hr */
	double heading;     /* degrees */
	double roll;        /* degrees */
	double pitch;       /* degrees */
	double heave;       /* m, always 0 */
	double sensordepth; /* m, positive down */
	double altitude;    /* m */
	double soundspeed;  /* m/s */

	/* raw sidescan, one channel per side, near range to far range */
	int ss_numsamples[2];                         /* number of samples */
	int ss_channel[2];                            /* XTF channel number */
	float ss_slantrange[2];                       /* slant range (m) */
	float ss_duration[2];                         /* time duration (s) */
	float ss_frequency[2];                        /* Hz (kHz in XTF files) */
	char ss_channelname[2][MBSYS_HISAS_CHANNELNAME_LENGTH];
	float ssraw[2][MBSYS_HISAS_MAXRAWPIXELS];     /* sample magnitudes */

	/* processed bathymetry and sidescan */
	int beams_bath;
	int pixels_ss;
	double pixel_size;
	char beamflag[MBSYS_HISAS_MAXBEAMS];          /* beamflags */
	double bath[MBSYS_HISAS_MAXBEAMS];            /* bathymetry (m) */
	double ss[MBSYS_HISAS_MAXPIXELS];             /* sidescan */
	double ss_acrosstrack[MBSYS_HISAS_MAXPIXELS]; /* acrosstrack distance (m) */
	double ss_alongtrack[MBSYS_HISAS_MAXPIXELS];  /* alongtrack distance (m) */

	/* comment */
	char comment[MBSYS_HISAS_COMMENT_LENGTH];
};

/* system specific function prototypes */
int mbsys_hisas_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_hisas_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_hisas_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss,
                           int *error);
int mbsys_hisas_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                        double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss, char *beamflag,
                        double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss,
                        double *ssacrosstrack, double *ssalongtrack, char *comment, int *error);
int mbsys_hisas_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
                       double navlat, double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath,
                       double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
                       double *ssalongtrack, char *comment, int *error);
int mbsys_hisas_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
                       double *angles_forward, double *angles_null, double *heave, double *alongtrack_offset, double *draft,
                       double *ssv, int *error);
int mbsys_hisas_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                                 double *altitude, int *error);
int mbsys_hisas_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                            double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll,
                            double *pitch, double *heave, int *error);
int mbsys_hisas_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
                           double navlat, double speed, double heading, double draft, double roll, double pitch, double heave,
                           int *error);
int mbsys_hisas_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error);
int mbsys_hisas_makess(int verbose, void *mbio_ptr, void *store_ptr, int pixel_size_set, double *pixel_size,
                       int swath_width_set, double *swath_width, int pixel_int, int *error);

#endif  /* MBSYS_HISAS_H_ */
