/*--------------------------------------------------------------------
 *    The MB-system:	mbf_elmk2unb.h	6/6/97
 *	$Id$
 *
 *    Copyright (c) 1997-2014 by
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
 * mbf_elmk2unb.h defines the data structures used by MBIO functions
 * to store multibeam data read from the MBF_ELMK2UNB format (MBIO id 92).
 *
 * Author:	D. W. Caress
 * Date:	June 6, 1997
 *
 * $Log: mbf_elmk2unb.h,v $
 * Revision 5.2  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.1  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.0  2000/12/01 22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.2  2000/09/30  06:29:44  caress
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
 *
 *
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

/* maximum number of beams and pixels */
#define	MBF_ELMK2UNB_MAXBEAMS	126
#define	MBF_ELMK2UNB_COMMENT_LENGTH	200

struct mbf_elmk2unb_beam_struct
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

struct mbf_elmk2unb_struct
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
	char	comment[MBF_ELMK2UNB_COMMENT_LENGTH];

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

	/* general bathymetry */
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
	struct mbf_elmk2unb_beam_struct beams[MBF_ELMK2UNB_MAXBEAMS];
	};
