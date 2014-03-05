/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_benthos.h	3/29/2011
 *	$Id$
 *
 *    Copyright (c) 2012-2013 by
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
 * mbsys_benthos.h defines the data structures used by MBIO functions
 * to store data from Benthos SIS1624 Sidescan sonar systems.
 * The formats associated with this i/o module are:
 *      MBF_XTFB1624 : MBIO ID 211
 *
 * Author:	Jens Renken (MARUM/University of Bremen)
 * Date:	March 29, 2011
 *
 * Author:	D. W. Caress
 * Date:	2 May 2012 (when the code was brought into the MB-System archive as a read-only i/o module)
 *
 * $Log: $
 *
 */
/*
 * Notes on the MBSYS_BENTHOS data:
 *
 */

/* sonar types */
#define	MBSYS_BENTHOS_UNKNOWN		0
#define	MBSYS_BENTHOS_SIS1624		1624

/* maximum number of beams and pixels */
#define	MBSYS_BENTHOS_MAXBEAMS	1
#define	MBSYS_BENTHOS_MAXRAWPIXELS	15360
#define	MBSYS_BENTHOS_MAXPIXELS	(15360 * 2)
#define	MBSYS_BENTHOS_MAXSVP	0
#define	MBSYS_BENTHOS_COMMENT_LENGTH	200

/* internal data structure */
struct mbsys_benthos_struct
{
	/* type of data record */
	int	kind;			/* Data kind */

	/* type of sonar */
	int	sonar;			/* Type of Benthos sonar */

	/* parameter info */
	float		MBOffsetX;
	float		MBOffsetY;
	float		MBOffsetZ;
	float		NavLatency;		/* GPS_time_received - GPS_time_sent (sec) */
	float		NavOffsetY;		/* Nav offset (m) */
	float		NavOffsetX;		/* Nav offset (m) */
	float		NavOffsetZ; 		/* Nav z offset (m) */
	float		NavOffsetYaw;		/* Heading offset (m) */
	float		MRUOffsetY;		/* Multibeam MRU y offset (m) */
	float		MRUOffsetX;		/* Multibeam MRU x offset (m) */
	float		MRUOffsetZ; 		/* Multibeam MRU z offset (m) */
	float		MRUOffsetPitch; 	/* Multibeam MRU pitch offset (degrees) */
	float		MRUOffsetRoll;		/* Multibeam MRU roll offset (degrees) */

	/* nav data */
	double	nav_time_d;
	double	nav_longitude;
	double	nav_latitude;
	float	nav_heading;

	/* attitude data */
	double	att_timetag;
	float	att_heading;
	float	att_heave;
	float	att_roll;
	float	att_pitch;

	/* comment */
	char	comment[MBSYS_BENTHOS_COMMENT_LENGTH];
	/* sound velocity profile -> discarded */

	/* survey data */
	double	png_time_d;
	double	png_latency;
	double	png_latitude;
	double	png_longitude;
	double	png_speed;			/* km/hr */
	double	png_roll;
	double	png_pitch;
	double	png_heading;
	double	png_heave;


	float	png_rtsv;		/* round trip soundvelocity */
	float   png_computedsv;	/* computed sound velocity	*/
	float	png_pressure;	/* pressure in psi	*/
	float	png_depth;		/* depth in meter	*/


	unsigned int	Seconds;	/* seconds since 00:00:00, 1 January 1970 */
	unsigned int	Millisecs;	/* milliseconds, LSB = 1 ms */
	unsigned int	ping_number;		/* sequential ping number from sonar startup/reset */



	float ssrawtimedelay;				/* raw sidescan delay (sec) */
	float ssrawtimeduration;			/* raw sidescan duration (sec) */
	float ssrawbottompick;				/* bottom pick time (sec) 	*/

	float ssrawslantrange;				/* slant range (m) */
	float ssrawgroundrange;				/* ground range (m) */

	unsigned short ssfrequency;

	unsigned short ssportinitgain;
	unsigned short ssstbdinitgain;
	unsigned short ssportgain;
	unsigned short ssstbdgain;

	unsigned short ssrawportsamples;		/* number of port raw sidescan samples */
	unsigned short ssrawstbdsamples;		/* number of stbd raw sidescan samples */
	unsigned short ssrawport[MBSYS_BENTHOS_MAXRAWPIXELS];		/* raw port sidescan */
	unsigned short ssrawstbd[MBSYS_BENTHOS_MAXRAWPIXELS];		/* raw starboard sidescan */

	int	beams_bath;
	int	pixels_ss;
	double	pixel_size;
	char 	beamflag[MBSYS_BENTHOS_MAXBEAMS]; 		/* beamflags */
	double 	bath[MBSYS_BENTHOS_MAXBEAMS];			/* bathymetry (m) */
	double	ss[MBSYS_BENTHOS_MAXPIXELS];			/* sidescan */
	double 	ss_acrosstrack[MBSYS_BENTHOS_MAXPIXELS];	/* acrosstrack distance (m) */
	double 	ss_alongtrack[MBSYS_BENTHOS_MAXPIXELS];		/* alongtrack distance (m) */

};

/* system specific function prototypes */
int mbsys_benthos_alloc(int verbose, void *mbio_ptr, void **store_ptr,
			int *error);
int mbsys_benthos_deall(int verbose, void *mbio_ptr, void **store_ptr,
			int *error);
int mbsys_benthos_dimensions(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_benthos_extract(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp,
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_benthos_insert(int verbose, void *mbio_ptr, void *store_ptr,
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp,
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_benthos_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles,
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset,
			double *draft, double *ssv, int *error);
int mbsys_benthos_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude,
			int *error);
int mbsys_benthos_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft,
			double *roll, double *pitch, double *heave,
			int *error);
int mbsys_benthos_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft,
			double roll, double pitch, double heave,
			int *error);
int mbsys_benthos_copy(int verbose, void *mbio_ptr,
			void *store_ptr, void *copy_ptr,
			int *error);
int mbsys_benthos_makess(int verbose, void *mbio_ptr, void *store_ptr,
			int pixel_size_set, double *pixel_size,
			int swath_width_set, double *swath_width,
			int *error);
