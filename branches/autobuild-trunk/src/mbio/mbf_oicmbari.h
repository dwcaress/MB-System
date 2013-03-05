/*--------------------------------------------------------------------
 *    The MB-system:	mbf_oicmbari.h	1/8/99
 *	$Id: mbf_oicmbari.h 1917 2012-01-10 19:25:33Z caress $
 *
 *    Copyright (c) 1999-2012 by
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
 * mbf_oicmbari.h defines the data structures used by MBIO functions
 * to store multibeam data read from the  MBF_OICMBARI format (MBIO id 141).  
 *
 * Author:	D. W. Caress
 * Date:	January 8, 1999
 *
 * $Log: mbf_oicmbari.h,v $
 * Revision 5.2  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.1  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.0  2000/12/01 22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.1  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.0  1999/03/31  18:29:20  caress
 * MB-System 4.6beta7
 *
 * Revision 1.1  1999/03/31  18:11:35  caress
 * Initial revision
 *

 *
 */
/*
 * Notes on the MBF_OICMBARI data format:
 *   1. This data format is an extended version of the vendor
 *      format used to store swath sonar data
 *      collected and recorded using the GeoDAS-SV1 system
 *      developed and sold by Ocean Imaging Consultants.
 *      In particular, the Deep Submergence Lab (DSL) of
 *      the Woods Hole Oceanographic Institution (WHOI) now
 *      uses an OIC GeoDAS-SV1 package as the front end for
 *      their deep towed 120 kHz interferometry sonar (AMS-120).
 *      This format stores processed bathymetry and sidescan
 *      data in addition to the "raw" data found in the 
 *      original data files.
 *   2. The data consist of variable length binary records.
 *   3. Each data record has three parts.  First there is a 248-byte
 *      header section containing the time stamp, navigation, a 
 *      variety of other values, the numbers and sizes of sonar
 *      data included in the record.  The second, optional, part of
 *      the record is "client specific information" of arbitrary 
 *      length. The third part contains up to eight arrays of
 *      sonar data.
 *   4. The header begins with a four byte magic number. The first
 *      three bytes are 'G', 'E', and '2'. The fourth byte is
 *      the id number for the data source.
 *   5. The maximum numbers of beams and pixels defined below are
 *      limitations specific to this I/O module - the format itself
 *      contains no limits on the amount of data per record.
 *   6. Comment records are encoded in MBIO using the "client
 *      specific information" section of the record and an unused
 *      sonar type value.
 *
 */

/* defines sizes of things */
#define	MBF_OICMBARI_HEADER_SIZE	276
#define	MBF_OICMBARI_MAX_CLIENT		252
#define	MBF_OICMBARI_MAX_COMMENT	MBF_OICMBARI_MAX_CLIENT
#define	MBF_OICMBARI_MAX_CHANNELS	8

/* define maximum number of beams */
#define	MBF_OICMBARI_MAX_BEAMS		1024
#define	MBF_OICMBARI_MAX_PIXELS		2048

struct mbf_oicmbari_channel_struct
	{
	int		offset;		/* offset in bytes to channel data */
	mb_u_char	type;		/* sonar type:
					    0 = sidescan
					    1 = angle
					    2 = multibeam */
	mb_u_char	side;		/* sonar side:
					    0 = port
					    1 = starboard */
	mb_u_char	size;		/* data sample type and size:
					    0 = 1 byte integer
					    1 = 2 byte integer
					    2 = 4 byte integer
					    3 = 4 byte float
					    4 = 12 byte set of three
						floats - range, theta, amp */
	mb_u_char	empty;		/* spare */
	int		frequency;	/* Hz */
	int		num_samples;	/* number of samples stored for
					    sidescan and angle sonar types, 
					    number of beams for multibeam */
	};

struct mbf_oicmbari_data_struct
	{
	int		rawsize[MBF_OICMBARI_MAX_CHANNELS];
	char		*raw[MBF_OICMBARI_MAX_CHANNELS];
	int		beams_bath_alloc;
	int		beams_amp_alloc;
	int		pixels_ss_alloc;
	char		*beamflag;
	float		*bath;
	float		*amp;
	float		*bathacrosstrack;
	float		*bathalongtrack;
	float		*tt;
	float		*angle;
	float		*ss;
	float		*ssacrosstrack;
	float		*ssalongtrack;
	};
	
struct mbf_oicmbari_seaview_struct
	{
	double		longitude;	/* longitude in degrees */
	double		latitude;	/* latitude in degrees */
	int		x;		/* local x coordinates in yards */
	int		y;		/* local y coordinates in yards */
	int		uncertainty;	/* navigation uncertainty in yards */
	float		speed;		/* speed over ground in knots */
	float		altitude;	/* platform altitude in feet */
	float		depth;		/* platform depth in feet */
	float		sound_velocity;	/* sound velocity in feet/sec */
	char		id[20];		/* client id string */
	};

struct mbf_oicmbari_header_struct
	{
	mb_u_char	type;		/* Magic number:
					    0 - EG&G sonar
					    1 - SEAVIEW sonar
					    2 - DEEPSCAN sonar
					    3 - STEST SEAVIEW test
					    4 - QTEST QMIPs test
					    5 - SM2 SeaMARC2 test
					    22 - WHOI DSL AMS120 */
	int		proc_status;	/* OIC processing status in
					    bit mask form */
	int		data_size;
	mb_u_char	client_size;
	mb_u_char	fish_status;	/* status bit field:
						0:  FocusAutoManual
						1:  FocusManualDisableEnable
						2:  PingRate AutoManual
						3:  TvgAutoManual
						4:  CalibOffOn
						5:  OutputModeProcRaw
						6:  ShadowMask
						7:  QualityBit */
	mb_s_char	nav_used;	
	mb_s_char	nav_type;	/*  0 = UTM coordinates in m
					    1 = Local coordinates in m
					    2 = Latitude and longitude */
	int		utm_zone;
	float		ship_x;		/* meters or degrees  */
	float		ship_y;		/* meters or degrees  */
	float		ship_course;	/* degrees */
	float		ship_speed;	/* m/sec */
	int		sec;
	int		usec;
	float		spare_gain;
	float		fish_heading;	/* degrees */
	float		fish_depth;	/* meters */
	float		fish_range;	/* meters */
	float		fish_pulse_width;   /* msec */
	float		gain_c0;
	float		gain_c1;
	float		gain_c2;
	float		fish_pitch;	/* degrees */
	float		fish_roll;	/* degrees */
	float		fish_yaw;	/* degrees */
	float		fish_x;		/* meters or degrees  */
	float		fish_y;		/* meters or degrees  */
	float		fish_layback;	/* meters */
	float		fish_altitude;	/* meters */
	int		fish_altitude_samples;
	float		fish_ping_period;   /* seconds per ping */
	float		sound_velocity;	/* m/sec */
	int		num_chan;
	int		beams_bath;
	int		beams_amp;
	int		bath_chan_port;
	int		bath_chan_stbd;
	int		pixels_ss;
	int		ss_chan_port;
	int		ss_chan_stbd;
	struct mbf_oicmbari_channel_struct channel[MBF_OICMBARI_MAX_CHANNELS];
	};

struct mbf_oicmbari_struct
	{
	int	kind;
	struct mbf_oicmbari_header_struct header;
	char	client[MBF_OICMBARI_MAX_CLIENT];
	struct mbf_oicmbari_data_struct data;
	};
