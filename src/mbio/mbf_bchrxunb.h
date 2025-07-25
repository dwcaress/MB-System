/*--------------------------------------------------------------------
 *    The MB-system:	mbf_bchrxunb.h	8/29/97
 *
 *    Copyright (c) 1997-2025 by
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
 * mbf_bchrxunb.h defines the data structures used by MBIO functions
 * to store multibeam data read from the MBF_BCHRXUNB format (MBIO id 92).
 *
 * Author:	D. W. Caress
 * Date:	August 29, 1997
 *
 */
/*
 * Notes on the MBF_BCHRXUNB data format:
 *   1. Elac multibeam systems output binary data telegrams.
 *      This format differs from the original "UNB" format in
 *      that the depth values are in 4-byte rather than 2-byte
 *      integers. This "extended" format supports deeper depths
 *      than the original format.
 *   2. Elac BottomChart sonar systems output both bathymetry
 *      and amplitude information for up to 56 beams per telegram.
 *   3. Each ping produces 8 beams.  A wide swath is constructed
 *      by successively pinging in different directions.
 *   4. Each telegram is preceded by a two byte start code and
 *      followed by a three byte end code consisting of 0x03
 *      followed by two bytes representing the checksum for
 *      the data bytes.  MB-System does not calculate checksums
 *      and puts 0's in the checksum bytes.
 *   5. The relevant telegram start codes, types, and sizes are:
 *         0x0250: Comment (Defined only for MB-System)   200 data bytes
 *         0x0251: Position                                36 data bytes
 *         0x0252: Parameter                               54 data bytes
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

#ifndef MBF_BCHRXUNB_H_
#define MBF_BCHRXUNB_H_

/* maximum number of beams and pixels */
#define MBF_BCHRXUNB_MAXBEAMS 56
#define MBF_BCHRXUNB_COMMENT_LENGTH 200

struct mbf_bchrxunb_profile_struct {
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int hundredth_sec;
	int thousandth_sec;
	int latitude;  /* 180 deg = 2*pi*e9 */
	int longitude; /* 180 deg = 2*pi*e9 */
	int roll;      /* 0.005 degrees */
	int pitch;     /* 0.005 degrees */
	int heading;   /* PI/180 degrees */
	int heave;     /* 0.001 meters */
	int bath[8];   /* depths:  0.01 meters */
	int bath_acrosstrack[8];
	/* acrosstrack distances: 0.01 meters */
	short int bath_alongtrack[8];
	/* alongtrack distances: 0.01 meters */
	short int tt[8];    /* travel times:         0.05 msec */
	short int angle[8]; /* 0.005 degrees */
	short int quality[8]; /* 1 (good) to 8 (bad)
	              extension:	10: flag by manual edit
	                  20: flag by filter edit */
	short int amp[8]; /* ??? */
};

struct mbf_bchrxunb_struct {
	/* type of data record */
	int kind; /* Data vs Comment */

	/* type of sonar */
	int sonar; /* Type of Elac sonar */

	/* parameter info (parameter telegrams) */
	int par_year;
	int par_month;
	int par_day;
	int par_hour;
	int par_minute;
	int par_second;
	int par_hundredth_sec;
	int par_thousandth_sec;
	short roll_offset;    /* roll offset (degrees) */
	short pitch_offset;   /* pitch offset (degrees) */
	short heading_offset; /* heading offset (degrees) */
	short time_delay;     /* positioning system delay (sec) */
	short transducer_port_height;
	short transducer_starboard_height;
	short transducer_port_depth;
	short transducer_starboard_depth;
	short transducer_port_x;
	short transducer_starboard_x;
	short transducer_port_y;
	short transducer_starboard_y;
	short transducer_port_error;
	short transducer_starboard_error;
	short antenna_height;
	short antenna_x;
	short antenna_y;
	short vru_height;
	short vru_x;
	short vru_y;
	short heave_offset;
	short line_number;
	short start_or_stop;
	short transducer_serial_number;

	/* comment */
	char comment[MBF_BCHRXUNB_COMMENT_LENGTH];

	/* position (position telegrams) */
	int pos_year;
	int pos_month;
	int pos_day;
	int pos_hour;
	int pos_minute;
	int pos_second;
	int pos_hundredth_sec;
	int pos_thousandth_sec;
	int pos_latitude;  /* 180 deg = 2e9 */
	int pos_longitude; /* 180 deg = 2e9 */
	unsigned int utm_northing;
	unsigned int utm_easting;
	int utm_zone_lon; /* 180 deg = 2e9 */
	char utm_zone;
	char hemisphere;
	char ellipsoid;
	char pos_spare;
	int semi_major_axis;
	int other_quality;

	/* sound velocity profile */
	int svp_year;
	int svp_month;
	int svp_day;
	int svp_hour;
	int svp_minute;
	int svp_second;
	int svp_hundredth_sec;
	int svp_thousandth_sec;
	int svp_latitude;  /* 180 deg = 2e9 */
	int svp_longitude; /* 180 deg = 2e9 */
	int svp_num;
	int svp_depth[500]; /* 0.1 meters */
	int svp_vel[500];   /* 0.1 meters/sec */

	/* bathymetry */
	int ping_num;
	int sound_vel;
	int mode;
	int pulse_length;
	int source_power;
	int receiver_gain;
	int profile_num; /* number of profiles stored */
	int beams_bath;  /* number of beams stored */
	struct mbf_bchrxunb_profile_struct profile[7];
};

#endif  /* MBF_BCHRXUNB_H_ */
