/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_elacmk2.h	6/10/97
 *	$Id: mbsys_elacmk2.h,v 5.3 2001-07-20 00:32:54 caress Exp $
 *
 *    Copyright (c) 1997, 2000 by
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
 * mbsys_elacmk2.h defines the data structures used by MBIO functions
 * to store data from Elac BottomChart Mark II multibeam sonar systems.
 * The data formats which are commonly used to store Elac 
 * data in files include
 *      MBF_ELMK2UNB : MBIO ID 92
 *
 *
 * Author:	D. W. Caress (L-DEO)
 * Date:	August 20, 1994
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.2  2001/06/08  21:44:01  caress
 * Version 5.0.beta01
 *
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.2  2000/09/30  06:31:19  caress
 * Snapshot for Dale.
 *
 * Revision 4.1  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.0  1997/07/25  14:25:40  caress
 * Version 4.5beta2.
 *
 * Revision 1.1  1997/07/25  14:19:53  caress
 * Initial revision
 *
 */
/*
 * Notes on the MBF_ELMK2UNB data format:
 *   1. Elac multibeam systems output binary data telegrams.
 *   2. Elac BottomChart Mark II sonar systems output both bathymetry
 *      and amplitude information for 126 beams per telegram.
 *   3. Each ping produces 42 beams.  A wide swath is constructed
 *      by successively pinging in different directions.
 *   4. Each telegram is preceded by a two byte start code and
 *      followed by a three byte end code consisting of 0x03
 *      followed by two bytes representing the checksum for
 *      the data bytes.  MB-System does not calculate checksums
 *      and puts 0's in the checksum bytes.
 *   5. The relevent telegram start codes, types, and sizes are:
 *         0x0250: Comment (Defined only for MB-System)   200 data bytes
 *         0x0251: Position                                36 data bytes
 *         0x0252: Parameter                               54 data bytes
 *         0x0253: Sound velocity profile                2016 data bytes
 *         0x0258: Mark II general bathymetry wrapper      24 data bytes
 *                 Mark II general bathymetry beam         28 data bytes
 *   6. Elac systems record navigation fixes using the position 
 *      telegram; navigation is not included in the per ping data.
 *      Since speed is not recorded, it is impossible to extrapolate
 *      position from the last navigation fix when processing the
 *      data serially, as MBIO does.  It may thus be necessary to extract
 *      the navigation from the position telegrams and remerge it with
 *      the ping telegrams using the program mbmerge.
 *
 */

/* sonar types */
#define	MBSYS_ELACMK2_UNKNOWN	0
#define	MBSYS_ELACMK2_BOTTOMCHART_MARKII	3

/* maximum number of beams and pixels */
#define	MBSYS_ELACMK2_MAXBEAMS		126
#define	MBSYS_ELACMK2_MAXSVP		500
#define	MBSYS_ELACMK2_COMMENT_LENGTH	200

/* telegram types */
#define	ELACMK2_NONE			0
#define	ELACMK2_COMMENT			0x0250
#define	ELACMK2_POS			0x0251
#define	ELACMK2_PARAMETER		0x0252
#define	ELACMK2_SVP			0x0253
#define	ELACMK2_BATHGEN			0x0258

/* telegram sizes */
#define	ELACMK2_COMMENT_SIZE		200
#define	ELACMK2_POS_SIZE		36
#define	ELACMK2_PARAMETER_SIZE		54
#define	ELACMK2_SVP_SIZE		2016
#define	ELACMK2_BATH56_SIZE		848
#define	ELACMK2_BATH40_SIZE		608
#define	ELACMK2_BATH32_SIZE		488
#define	ELACMK2_BATHGEN_HDR_SIZE	24
#define	ELACMK2_BATHGEN_BEAM_SIZE	28

/* internal data structure */

struct mbsys_elacmk2_beam_struct
	{
	unsigned int	bath;		/* 0.01 m */
	int	bath_acrosstrack;	/* 0.01 m */
	int	bath_alongtrack;	/* 0.01 m */
	unsigned int	tt;		/* 0.05 ms */
	int	quality;		/* 1 (best) to 8 (worst) */
	int	amplitude;		/* dB + 128 */
	unsigned short	time_offset;	/* 0.5 ms */
	short	heave;			/* 0.001 meters */
	short	roll;			/* 0.005 degrees */
	short	pitch;			/* 0.005 degrees */
	short	angle;			/* 0.005 degrees */
	};

struct mbsys_elacmk2_struct
	{
	/* type of data record */
	int	kind;			/* Data vs Comment */

	/* type of sonar */
	int	sonar;			/* Type of Elac sonar */

	/* parameter info (parameter telegrams) */
	int	par_year;
	int	par_month;
	int	par_day;
	int	par_hour;
	int	par_minute;
	int	par_second;
	int	par_hundredth_sec;
	int	par_thousandth_sec;
	short	roll_offset;	/* roll offset (degrees) */
	short	pitch_offset;	/* pitch offset (degrees) */
	short	heading_offset;	/* heading offset (degrees) */
	short	time_delay;	/* positioning system delay (sec) */
	short	transducer_port_height;
	short	transducer_starboard_height;
	short	transducer_port_depth;
	short	transducer_starboard_depth;
	short	transducer_port_x;
	short	transducer_starboard_x;
	short	transducer_port_y;
	short	transducer_starboard_y;
	short	transducer_port_error;
	short	transducer_starboard_error;
	short	antenna_height;
	short	antenna_x;
	short	antenna_y;
	short	vru_height;
	short	vru_x;
	short	vru_y;
	short	line_number;
	short	start_or_stop;
	short	transducer_serial_number;

	/* comment */
	char	comment[MBSYS_ELACMK2_COMMENT_LENGTH];

	/* position (position telegrams) */
	int	pos_year;
	int	pos_month;
	int	pos_day;
	int	pos_hour;
	int	pos_minute;
	int	pos_second;
	int	pos_hundredth_sec;
	int	pos_thousandth_sec;
	int	pos_latitude;		/* 180 deg = 2e9 */
	int	pos_longitude;		/* 180 deg = 2e9 */
	unsigned int	utm_northing;
	unsigned int	utm_easting;
	int	utm_zone_lon;		/* 180 deg = 2e9 */
	char	utm_zone;
	char	hemisphere;
	char	ellipsoid;
	char	pos_spare;
	int	semi_major_axis;
	int	other_quality;

	/* sound velocity profile */
	int	svp_year;
	int	svp_month;
	int	svp_day;
	int	svp_hour;
	int	svp_minute;
	int	svp_second;
	int	svp_hundredth_sec;
	int	svp_thousandth_sec;
	int	svp_latitude;		/* 180 deg = 2e9 */
	int	svp_longitude;		/* 180 deg = 2e9 */
	int	svp_num;
	int	svp_depth[MBSYS_ELACMK2_MAXSVP]; /* 0.1 meters */
	int	svp_vel[MBSYS_ELACMK2_MAXSVP];	/* 0.1 meters/sec */

	/* general bathymetry */
	int	year;
	int	month;
	int	day;
	int	hour;
	int	minute;
	int	second;
	int	hundredth_sec;
	int	thousandth_sec;
	double	longitude;
	double	latitude;
	double	speed;
	int	ping_num;
	int	sound_vel;		/* 0.1 m/s */
	int	heading;		/* 0.01 deg */
	int	pulse_length;		/* 0.01 ms */
	int	mode;			/* 0: omni, 1: RDT (def) */
	int	source_power;		/* 0: low, 1: high */
	int	receiver_gain_stbd;	/* db */
	int	receiver_gain_port;	/* db */
	int	reserved;
	int	beams_bath;		/* number of beams stored */
	struct mbsys_elacmk2_beam_struct beams[MBSYS_ELACMK2_MAXBEAMS];
	};
	
/* system specific function prototypes */
int mbsys_elacmk2_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_elacmk2_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_elacmk2_extract(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_elacmk2_insert(int verbose, void *mbio_ptr, void *store_ptr, 
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_elacmk2_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles, 
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset, 
			double *draft, double *ssv, int *error);
int mbsys_elacmk2_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude, 
			int *error);
int mbsys_elacmk2_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft, 
			double *roll, double *pitch, double *heave, 
			int *error);
int mbsys_elacmk2_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft, 
			double roll, double pitch, double heave,
			int *error);
int mbsys_elacmk2_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, 
			int *nsvp, 
			double *depth, double *velocity,
			int *error);
int mbsys_elacmk2_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int nsvp, 
			double *depth, double *velocity,
			int *error);
int mbsys_elacmk2_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error);

