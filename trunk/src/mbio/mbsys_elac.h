/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_elac.h	8/20/94
 *	$Id$
 *
 *    Copyright (c) 1994-2011 by
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
 * mbsys_elac.h defines the data structures used by MBIO functions
 * to store data from Elac SeaBat 9001 multibeam sonar systems.
 * The data formats which are commonly used to store Elac 
 * data in files include
 *      MBF_BCHRTUNB : MBIO ID 91
 *
 *
 * Author:	D. W. Caress (L-DEO)
 * Date:	August 20, 1994
 *
 * $Log: mbsys_elac.h,v $
 * Revision 5.6  2005/11/05 00:48:05  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.5  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.4  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.3  2002/07/20 20:42:40  caress
 * Release 5.0.beta20
 *
 * Revision 5.2  2001/07/20 00:32:54  caress
 * Release 5.0.beta03
 *
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.6  2000/09/30  06:31:19  caress
 * Snapshot for Dale.
 *
 * Revision 4.5  1998/10/05  18:32:27  caress
 * MB-System version 4.6beta
 *
 * Revision 1.1  1998/10/05  17:46:15  caress
 * Initial revision
 *
 * Revision 4.4  1997/09/15  19:06:40  caress
 * Real Version 4.5
 *
 * Revision 4.3  1997/07/25  14:19:53  caress
 * Version 4.5beta2.
 * Much mucking, particularly with Simrad formats.
 *
 * Revision 4.2  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.1  1995/07/13  19:15:09  caress
 * Intermediate check-in during major bug-fixing flail.
 *
 * Revision 4.1  1995/07/13  19:15:09  caress
 * Intermediate check-in during major bug-fixing flail.
 *
 * Revision 4.0  1994/10/21  12:35:07  caress
 * Release V4.0
 *
 * Revision 1.1  1994/10/21  12:20:01  caress
 * Initial revision
 *
 *
 *
 */
/*
 * Notes on the MBSYS_ELAC data structure:
 *   1. Elac multibeam systems output binary data telegrams.
 *   2. Elac BottomChart sonar systems output both bathymetry
 *      and amplitude information for up to 56 beams per telegram.
 *   3. Each ping produces 8 beams.  A wide swath is constructed
 *      by successively pinging in different directions.
 *   4. Each telegram is preceded by a two byte start code and
 *      followed by a three byte end code consisting of 0x03
 *      followed by two bytes representing the checksum for
 *      the data bytes.  MB-System does not calculate checksums
 *      and puts 0's in the checksum bytes.
 *   5. The relevent telegram start codes, types, and sizes are:
 *         0x0250: Comment (Defined only for MB-System)   200 data bytes
 *         0x0251: Position                                36 data bytes
 *         0x0252: Parameter                            56/54 data bytes
 *         0x0253: Sound velocity profile                2016 data bytes
 *         0x0254: BottomChart 56 beam bathymetry         848 data bytes
 *         0x0255: BottomChart 40 beam bathymetry         608 data bytes
 *         0x0256: BottomChart 32 beam bathymetry         488 data bytes
 *   6. Elac systems record navigation fixes using the position 
 *      telegram; navigation is not always included in the per ping data.
 *      Since speed is not recorded, it is impossible to extrapolate
 *      position from the last navigation fix when processing the
 *      data serially, as MBIO does.  It may thus be necessary to extract
 *      the navigation from the position telegrams and remerge it with
 *      the ping telegrams using the program mbmerge.
 *
 */

/* sonar types */
#define	MBSYS_ELAC_UNKNOWN	0
#define	MBSYS_ELAC_BOTTOMCHART	1

/* maximum number of beams and pixels */
#define	MBSYS_ELAC_MAXBEAMS	56
#define	MBSYS_ELAC_MAXSVP	500
#define	MBSYS_ELAC_COMMENT_LENGTH	200

/* telegram types */
#define	ELAC_NONE		0
#define	ELAC_COMMENT		0x0250
#define	ELAC_POS		0x0251
#define	ELAC_PARAMETER		0x0252
#define	ELAC_SVP		0x0253
#define	ELAC_BATH56		0x0254
#define	ELAC_BATH40		0x0255
#define	ELAC_BATH32		0x0256
#define	ELAC_XBATH56		0x0260
#define	ELAC_XBATH40		0x0261
#define	ELAC_XBATH32		0x0262

/* telegram sizes */
#define	ELAC_COMMENT_SIZE	200
#define	ELAC_POS_SIZE		36
#define	ELAC_PARAMETER_SIZE	56
#define	ELAC_SVP_SIZE		2016
#define	ELAC_BATH56_SIZE	848
#define	ELAC_BATH40_SIZE	608
#define	ELAC_BATH32_SIZE	488
#define	ELAC_XPARAMETER_SIZE	54
#define	ELAC_XBATH56_SIZE	1072
#define	ELAC_XBATH40_SIZE	768
#define	ELAC_XBATH32_SIZE	616

/* internal data structure */

struct mbsys_elac_profile_struct
	{
	int	year;
	int	month;
	int	day;
	int	hour;
	int	minute;
	int	second;
	int	hundredth_sec;
	int	thousandth_sec;
	int	latitude;		/* 180 deg = 2*pi*e9 */
	int	longitude;		/* 180 deg = 2*pi*e9 */
	int	roll;			/* 0.005 degrees */
	int	pitch;			/* 0.005 degrees */
	int	heading;		/* PI/180 degrees */
	int	heave;			/* 0.001 meters */
	int	bath[8];		/* depths:  0.01 meters */	
	int	bath_acrosstrack[8];
				/* acrosstrack distances: 0.01 meters */
	short int bath_alongtrack[8];
				/* alongtrack distances: 0.01 meters */
	short int tt[8];	/* travel times:         0.05 msec */
	short int angle[8];	/* 0.005 degrees */
	short int quality[8];	/* 0 (bad) to 3 (good) */
	short int amp[8];	/* ??? */

	};

struct mbsys_elac_struct
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
	short	heave_offset;
	short	line_number;
	short	start_or_stop;
	short	transducer_serial_number;

	/* comment */
	char	comment[MBSYS_ELAC_COMMENT_LENGTH];

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
	int	svp_depth[MBSYS_ELAC_MAXSVP]; /* 0.1 meters */
	int	svp_vel[MBSYS_ELAC_MAXSVP];	/* 0.1 meters/sec */

	/* bathymetry */
	int	ping_num;
	int	sound_vel;
	int	mode;
	int	pulse_length;
	int	source_power;
	int	receiver_gain;
	int	profile_num;	/* number of profiles stored */
	int	beams_bath;	/* number of beams stored */
	struct mbsys_elac_profile_struct profile[7];
	};
	
/* system specific function prototypes */
int mbsys_elac_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_elac_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_elac_dimensions(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_elac_extract(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_elac_insert(int verbose, void *mbio_ptr, void *store_ptr, 
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_elac_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles, 
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset, 
			double *draft, double *ssv, int *error);
int mbsys_elac_detects(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			int *detects, int *error);
int mbsys_elac_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude, 
			int *error);
int mbsys_elac_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft, 
			double *roll, double *pitch, double *heave, 
			int *error);
int mbsys_elac_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft, 
			double roll, double pitch, double heave,
			int *error);
int mbsys_elac_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, 
			int *nsvp, 
			double *depth, double *velocity,
			int *error);
int mbsys_elac_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int nsvp, 
			double *depth, double *velocity,
			int *error);
int mbsys_elac_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error);

