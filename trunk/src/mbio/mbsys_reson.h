/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_reson.h	8/20/94
 *	$Id: mbsys_reson.h,v 4.3 1998-10-05 17:46:15 caress Exp $
 *
 *    Copyright (c) 1994 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbsys_reson.h defines the data structures used by MBIO functions
 * to store data from Reson SeaBat 9001 multibeam sonar systems.
 * The data formats which are commonly used to store Reson 
 * data in files include
 *      MBF_CBAT9001 : MBIO ID 81
 *
 *
 * Author:	D. W. Caress (L-DEO)
 * Date:	August 20, 1994
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.2  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.1  1995/07/13  19:15:09  caress
 * Intermediate check-in during major bug-fixing flail.
 *
 * Revision 4.1  1995/07/13  19:15:09  caress
 * Intermediate check-in during major bug-fixing flail.
 *
 * Revision 4.0  1994/10/21  12:35:08  caress
 * Release V4.0
 *
 * Revision 1.1  1994/10/21  12:20:01  caress
 * Initial revision
 *
 *
 *
 */
/*
 * Notes on the MBSYS_RESON data structure:
 *   1. Reson multibeam systems output binary data telegrams.
 *   2. Reson SeaBat 9001 systems output both bathymetry
 *      and amplitude information for 60 beams.
 *   3. Each telegram is preceded by a two byte start code and
 *      followed by a three byte end code consisting of 0x03
 *      followed by two bytes representing the checksum for
 *      the data bytes.  MB-System does not calculate checksums
 *      and puts 0's in the checksum bytes.
 *   4. The relevent telegram start codes, types, and sizes are:
 *         0x0240: Comment (Defined only for MB-System)   200 data bytes
 *         0x0241: Position                                36 data bytes
 *         0x0242: Parameter                               44 data bytes
 *         0x0243: Sound velocity profile                2016 data bytes
 *         0x0244: SeaBat 9001 bathymetry                 752 data bytes
 *         0x0245: Short sound velocity profile           816 data bytes
 *   5. Reson systems record navigation fixes using the position 
 *      telegram; navigation is not always included in the per ping data.
 *      Since speed is not recorded, it is impossible to extrapolate
 *      position from the last navigation fix when processing the
 *      data serially, as MBIO does.  It may thus be necessary to extract
 *      the navigation from the position telegrams and remerge it with
 *      the ping telegrams using the program mbmerge.
 *
 */

/* sonar types */
#define	MBSYS_RESON_UNKNOWN	0
#define	MBSYS_RESON_SEABAT9001	1

/* maximum number of beams and pixels */
#define	MBSYS_RESON_MAXBEAMS	60
#define	MBSYS_RESON_COMMENT_LENGTH	200

/* telegram types */
#define	RESON_NONE		0
#define	RESON_COMMENT		0x0240
#define	RESON_POS		0x0241
#define	RESON_PARAMETER		0x0242
#define	RESON_SVP		0x0243
#define	RESON_BATH		0x0244
#define	RESON_SHORT_SVP		0x0245

/* telegram sizes */
#define	RESON_COMMENT_SIZE	200
#define	RESON_POS_SIZE		36
#define	RESON_PARAMETER_SIZE	44
#define	RESON_SVP_SIZE		2016
#define	RESON_BATH_SIZE		752
#define	RESON_SHORT_SVP_SIZE	816

/* internal data structure */
struct mbsys_reson_struct
	{
	/* type of data record */
	int	kind;			/* Data vs Comment */

	/* type of sonar */
	int	sonar;			/* Type of Reson sonar */

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
	short	transducer_depth;	/* tranducer depth (meters) */
	short	transducer_height;	/* reference height (meters) */
	short	transducer_x;	/* reference athwartships offset (meters) */
	short	transducer_y;	/* reference fore-aft offset (meters) */
	short	antenna_x;	/* antenna athwartships offset (meters) */
	short	antenna_y;	/* antenna fore-aft offset (meters) */
	short	antenna_z;	/* antenna height (meters) */
	short	motion_sensor_x;/* motion sensor athwartships offset (meters) */
	short	motion_sensor_y;/* motion sensor fore-aft offset (meters) */
	short	motion_sensor_z;/* motion sensor height offset (meters) */
	short	spare;
	short	line_number;
	short	start_or_stop;
	short	transducer_serial_number;

	/* comment */
	char	comment[MBSYS_RESON_COMMENT_LENGTH];

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
	int	svp_depth[500]; /* 0.1 meters */
	int	svp_vel[500];	/* 0.1 meters/sec */

	/* bathymetry */
	int	year;
	int	month;
	int	day;
	int	hour;
	int	minute;
	int	second;
	int	hundredth_sec;
	int	thousandth_sec;
	int	latitude;		/* 180 deg = 2e9 */
	int	longitude;		/* 180 deg = 2e9 */
	int	roll;			/* 0.005 degrees */
	int	pitch;			/* 0.005 degrees */
	int	heading;		/* 0.01 degrees */
	int	heave;			/* 0.001 meters */
	int	ping_number;
	int	sound_vel;	/* 0.1 meters/sec */
	int	mode;		/* unused */
	int	gain1;		/* unused */
	int	gain2;		/* unused */
	int	gain3;		/* unused */
	int	beams_bath;
	short bath[MBSYS_RESON_MAXBEAMS];
				/* depths:  0.01 meters */	
	short int bath_acrosstrack[MBSYS_RESON_MAXBEAMS];
				/* acrosstrack distances: 0.01 meters */
	short int bath_alongtrack[MBSYS_RESON_MAXBEAMS];
				/* alongtrack distances: 0.01 meters */
	short int tt[MBSYS_RESON_MAXBEAMS];
				/* travel times:         0.05 msec */
	short int angle[MBSYS_RESON_MAXBEAMS];		
				/* 0.005 degrees */
	short int quality[MBSYS_RESON_MAXBEAMS];
				/* 0 (bad) to 3 (good) */
	short int amp[MBSYS_RESON_MAXBEAMS];
				/* ??? */

	};
