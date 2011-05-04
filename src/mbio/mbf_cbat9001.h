/*--------------------------------------------------------------------
 *    The MB-system:	mbf_cbat9001.h	8/21/94
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
 * mbf_cbat9001.h defines the data structures used by MBIO functions
 * to store multibeam data read from the MBF_CBAT9001 format (MBIO id 81).  
 *
 * Author:	D. W. Caress
 * Date:	August 21, 1994
 * $Log: mbf_cbat9001.h,v $
 * Revision 5.2  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.1  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.0  2000/12/01 22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.4  2000/09/30  06:29:44  caress
 * Snapshot for Dale.
 *
 * Revision 4.3  1999/01/01  23:41:06  caress
 * MB-System version 4.6beta6
 *
 * Revision 4.2  1998/10/05 17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.1  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.0  1994/10/21  12:35:04  caress
 * Release V4.0
 *
 * Revision 4.0  1994/10/21  12:35:04  caress
 * Release V4.0
 *
 * Revision 1.1  1994/10/21  12:20:01  caress
 * Initial revision
 *
 * Revision 1.1  1994/10/21  12:13:33  caress
 * Initial revision
 *
 *
 *
 */
/*
 * Notes on the MBF_CBAT9001 data format:
 *   1. Reson SeaBat products are high frequency, 
 *      shallow water multibeam sonars.
 *      Reson SeaBat 9001 systems output both bathymetry
 *      and amplitude information for 60 beams.
 *      Reson SeaBat 8101 systems output both bathymetry
 *      and amplitude information for up to 128 beams.
 *      These sonars use fixed, analog beamforming followed
 *      by a combination of amplitude and phase bottom
 *      detection.
 *   2. Reson multibeam systems output raw range and amplitude
 *      data in a binary format. The data acquisition systems 
 *      associated with the sonars calculate bathymetry using 
 *      a water sound velocity, roll, pitch, and heave data.
 *   3. Generally, Reson data acquisition systems record 
 *      navigation asynchronously in the data stream, without
 *      providing speed information. This means that the
 *      navigation must be interpolated on the fly as the
 *      data is read.
 *   4. The navigation is frequently provided in projected
 *      coordinates (eastings and northings) rather than in
 *      longitude and latitude. Since MB-System operates solely
 *      in longitude and latitude, the original navigation must
 *      be unprojected.
 *   5. The Reson data formats supported by MB-System include:
 *        MBF_CBAT9001 - a binary format designed by John Hughes Clarke
 *           of the University of New Brunswick. Parameter and
 *           sound velocity profile records are included.
 *        MBF_CBAT8101 - a clone of the above format supporting
 *           Reson 8101 data.
 *        MBF_HYPC8101 - the ASCII format used by the HYPACK system
 *           of Coastal Oceanographics in conjunction with
 *           Reson 8101 data. This format is supported as read-only 
 *           by MB-System.
 *        MBF_GSFGENMB - the generic sensor format of SAIC which
 *           supports data from a large number of sonars, including
 *           Reson sonars. MB-System handles GSF separately from
 *           other formats.
 *   6. For the UNB-style formats MBF_CBAT9001 and MBF_CBAT8101, 
 *      each data telegram is preceded by a two byte start code and
 *      followed by a three byte end code consisting of 0x03
 *      followed by two bytes representing the checksum for
 *      the data bytes.  MB-System does not calculate checksums
 *      and puts 0's in the checksum bytes.
 *      The relevent telegram start codes, types, and sizes are:
 *         0x0240: Comment***                             200 data bytes
 *         0x0241: Position                                36 data bytes
 *         0x0242: Parameter                               44 data bytes
 *         0x0243: Sound velocity profile                2016 data bytes
 *         0x0244: SeaBat 9001 bathymetry                 752 data bytes
 *         0x0245: Short sound velocity profile           816 data bytes
 *         0x0246: SeaBat 8101 bathymetry***             1244 data bytes
 *         0x0247: Heading***                             752 data bytes
 *         0x0248: Attitude***                            752 data bytes
 *            *** Defined only for MB-System
 *
 */

/* maximum number of beams and pixels */
#define	MBF_CBAT9001_MAXBEAMS	60
#define	MBF_CBAT9001_COMMENT_LENGTH	200

struct mbf_cbat9001_struct
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
	short	transducer_x;	/* reference fore-aft offset (meters) */
	short	transducer_y;	/* reference athwartships offset (meters) */
	short	antenna_x;	/* antenna athwartships offset (meters) */
	short	antenna_y;	/* antenna athwartships offset (meters) */
	short	antenna_z;	/* antenna height (meters) */
	short	motion_sensor_x;/* motion sensor athwartships offset (meters) */
	short	motion_sensor_y;/* motion sensor athwartships offset (meters) */
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

