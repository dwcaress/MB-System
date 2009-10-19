/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_reson.h	8/20/94
 *	$Id: mbsys_reson8k.h,v 5.5 2006/03/06 21:47:48 caress Exp $
 *
 *    Copyright (c) 2001-2009 by
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
 * mbsys_reson.h defines the data structures used by MBIO functions
 * to store data from Reson SeaBat 8101 and other 8K series 
 * multibeam sonar systems.
 * The data formats which are commonly used to store Reson 8K
 * data in files include
 *       MBF_XTFR8101 : MBIO ID 84
 *
 *
 * Author:	D. W. Caress
 * Date:	September 2, 2001
 *
 * $Log: mbsys_reson8k.h,v $
 * Revision 5.5  2006/03/06 21:47:48  caress
 * Implemented changes suggested by Bob Courtney of the Geological Survey of Canada to support translating Reson data to GSF.
 *
 * Revision 5.4  2005/11/05 00:48:05  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.3  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.2  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.1  2001/11/15 22:36:43  caress
 * Working on it.
 *
 * Revision 5.0  2001/09/17  23:24:10  caress
 * Added XTF format.
 *
 *
 *
 */
/*
 * Notes on the MBSYS_RESON8K data:
 *
 */

/* sonar types */
#define	MBSYS_RESON8K_UNKNOWN		0
#define	MBSYS_RESON8K_SEABAT9001	9001
#define	MBSYS_RESON8K_SEABAT9002	9002
#define	MBSYS_RESON8K_SEABAT8101	8101
#define	MBSYS_RESON8K_SEABAT8111	8111
#define	MBSYS_RESON8K_SEABAT8125	8125
#define	MBSYS_RESON8K_MESOTECHSM2000	2000

/* maximum number of beams and pixels */
#define	MBSYS_RESON8K_MAXBEAMS	240
#define	MBSYS_RESON8K_MAXRAWPIXELS	2048
#define	MBSYS_RESON8K_MAXPIXELS	1024
#define	MBSYS_RESON8K_MAXSVP	500
#define	MBSYS_RESON8K_COMMENT_LENGTH	200

#define RESON8K_RT_1		0x11
#define RESON8K_RIT_1		0x12
#define RESON8K_RT_2		0x13
#define RESON8K_RIT_2		0x14
#define RESON8K_RT_3		0x17
#define RESON8K_RIT_3		0x18

/* internal data structure */
struct mbsys_reson8k_struct
	{
	/* type of data record */
	int	kind;			/* Data kind */

	/* type of sonar */
	int	sonar;			/* Type of Reson sonar */

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
	char	comment[MBSYS_RESON8K_COMMENT_LENGTH];

	/* sound velocity profile */
	double	svp_time_d;
	int	svp_num;
	float	svp_depth[MBSYS_RESON8K_MAXSVP]; /* meters */
	float	svp_vel[MBSYS_RESON8K_MAXSVP];	/* meters/sec */

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


   	char      	packet_type;      		/* identifier for packet type (0x18) */
	char           	packet_subtype;   	/* identifier for packet subtype */
						/* for dual head system, most significant bit (bit 7) */
						/* indicates which sonar head to associate with packet */
						/* 	head 1 - bit 7 set to 0 */
						/* 	head 2 -	bit 7 set to 1 		 */
	unsigned short 	latency;          		/* time from ping to output (milliseconds) */
	unsigned int	Seconds;	/* seconds since 00:00:00, 1 January 1970 */
	unsigned int	Millisecs;	/* milliseconds, LSB = 1 ms */
	unsigned int	ping_number;		/* sequential ping number from sonar startup/reset */
	unsigned int	sonar_id;		/* least significant four bytes of Ethernet address */
	unsigned short	sonar_model;		/* coded model number of sonar */
	unsigned short	frequency;		/* sonar frequency in KHz */
	unsigned short 	velocity;         		/* programmed sound velocity (LSB = 1 m/sec) */
   	unsigned short 	sample_rate;      		/* A/D sample rate (samples per second) */
	unsigned short 	ping_rate;        		/* Ping rate (pings per second * 1000) */
   	unsigned short 	range_set;        		/* range setting for SeaBat (meters ) */
   	unsigned short 	power;            		/* power setting for SeaBat  	 */
						/* bits	0-4 -	power (0 - 8) */
	unsigned short 	gain;             		/* gain setting for SeaBat */
					/* bits	0-6 -	gain (1 - 45) */
					/* bit 	14	(0 = fixed, 1 = tvg) */
					/* bit	15	(0 = manual, 1 = auto) */
	unsigned short  	pulse_width;      		/* transmit pulse width (microseconds) */
	mb_u_char	tvg_spread;		/* spreading coefficient for tvg * 4  */
						/* valid values = 0 to 240 (0.0 to 60.0 in 0.25 steps) */
	mb_u_char	tvg_absorp;		/* absorption coefficient for tvg */
	mb_u_char     	projector_type;      	/* bits 0-4 = projector type */
						/* 0 = stick projector */
						/* 1 = array face */
						/* 2 = ER projector */
						/* bit 7 - pitch steering (1=enabled, 0=disabled) */
	mb_u_char      projector_beam_width;	/* along track transmit beam width (degrees * 10) */
	unsigned short  	beam_width_num;   	/* cross track receive beam width numerator */
	unsigned short 	beam_width_denom; 	/* cross track receive beam width denominator */
						/* beam width degrees = numerator / denominator */
	short		projector_angle;		/* projector pitch steering angle (degrees * 100) */
	unsigned short	min_range;		/* sonar filter settings */
	unsigned short	max_range;
	unsigned short	min_depth;
	unsigned short	max_depth;
	mb_u_char	filters_active;		/* range/depth filters active  */
						/* bit 0 - range filter (0 = off, 1 = active) */
						/* bit 1 - depth filter (0 = off, 1 = active) */
	mb_u_char	spare[3];			/* spare field for future growth */
	short		temperature;		/* temperature at sonar head (deg C * 10) */
	short          	beam_count;       		/* number of sets of beam data in packet */
	unsigned short 	range[MBSYS_RESON8K_MAXBEAMS]; 		/* range for beam where n = Beam Count */
						/* range units = sample cells * 4 */
	mb_u_char  	quality[MBSYS_RESON8K_MAXBEAMS/2+1];   		/* packed quality array (two 4 bit values/char) */
						/* cnt = n/2 if beam count even, n/2+1 if odd */
						/* cnt then rounded up to next even number */
						/* e.g. if beam count=101, cnt=52  */
						/* unused trailing quality values set to zero */
						/* bit 0 - brightness test (0=failed, 1=passed) */
						/* bit 1 - colinearity test (0=failed, 1=passed) */
						/* bit 2 - amplitude bottom detect used */
						/* bit 3 - phase bottom detect used */
						/* bottom detect can be amplitude, phase or both */
	unsigned short	intensity[MBSYS_RESON8K_MAXBEAMS];   		/* intensities at bottom detect  */

	float ssrawtimedelay;				/* raw sidescan delay (sec) */
	float ssrawtimeduration;			/* raw sidescan duration (sec) */
	float ssrawbottompick;				/* bottom pick time (sec) */
	unsigned short ssrawportsamples;		/* number of port raw sidescan samples */
	unsigned short ssrawstbdsamples;		/* number of stbd raw sidescan samples */
	unsigned short ssrawport[MBSYS_RESON8K_MAXRAWPIXELS];		/* raw port sidescan */
	unsigned short ssrawstbd[MBSYS_RESON8K_MAXRAWPIXELS];		/* raw starboard sidescan */

	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	double	pixel_size;
	char 	beamflag[MBSYS_RESON8K_MAXBEAMS]; 		/* beamflags */
	double 	bath[MBSYS_RESON8K_MAXBEAMS];			/* bathymetry (m) */
	double 	amp[MBSYS_RESON8K_MAXBEAMS];			/* bathymetry (m) */	
	double 	bath_acrosstrack[MBSYS_RESON8K_MAXBEAMS];	/* acrosstrack distance (m) */
	double 	bath_alongtrack[MBSYS_RESON8K_MAXBEAMS];	/* alongtrack distance (m) */
	double	ss[MBSYS_RESON8K_MAXPIXELS];			/* sidescan */
	double 	ss_acrosstrack[MBSYS_RESON8K_MAXPIXELS];	/* acrosstrack distance (m) */
	double 	ss_alongtrack[MBSYS_RESON8K_MAXPIXELS];		/* alongtrack distance (m) */
	};
	
/* system specific function prototypes */
int mbsys_reson8k_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_reson8k_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_reson8k_dimensions(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_reson8k_extract(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_reson8k_insert(int verbose, void *mbio_ptr, void *store_ptr, 
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_reson8k_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles, 
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset, 
			double *draft, double *ssv, int *error);
int mbsys_reson8k_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude, 
			int *error);
int mbsys_reson8k_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft, 
			double *roll, double *pitch, double *heave, 
			int *error);
int mbsys_reson8k_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft, 
			double roll, double pitch, double heave,
			int *error);
int mbsys_reson8k_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, 
			int *nsvp, 
			double *depth, double *velocity,
			int *error);
int mbsys_reson8k_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int nsvp, 
			double *depth, double *velocity,
			int *error);
int mbsys_reson8k_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error);
int mbsys_reson8k_makess(int verbose, void *mbio_ptr, void *store_ptr,
			int pixel_size_set, double *pixel_size, 
			int swath_width_set, double *swath_width, 
			int *error);

