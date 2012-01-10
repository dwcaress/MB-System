/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_oic.h	3/1/99
 *	$Id$
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
 * mbsys_oic.h defines the data structure used by MBIO functions
 * to store swath sonar data derived from OIC systems:
 *      MBF_OICGEODA : MBIO ID 141
 *      MBF_OICMBARI : MBIO ID 142
 *
 * Author:	D. W. Caress
 * Date:	March 1, 1999
 *
 * $Log: mbsys_oic.h,v $
 * Revision 5.5  2005/11/05 00:48:05  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.4  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.3  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
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
 * Revision 4.1  2000/09/30  06:31:19  caress
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
 * Notes on the MBSYS_OIC data structure:
 *   1. This data structure is used to store swath sonar data
 *      collected and recorded using the GeoDAS-SV1 system
 *      developed and sold by Ocean Imaging Consultants.
 *      In particular, the Deep Submergence Lab (DSL) of
 *      the Woods Hole Oceanographic Institution (WHOI) now
 *      uses an OIC GeoDAS-SV1 package as the front end for
 *      their deep towed 120 kHz interferometry sonar (AMS-120).
 *   2. Two data formats are supported. The first, MBF_OICGEODA, 
 *      is the vendor format containing unflaggable bathymetry
 *      and raw sidescan. The MBIO i/o module for this format
 *      generates complete bathymetry and sidescan out of the
 *      raw information, but does not support flagging of bad
 *      bathymetry or the storage of corrected sidescan. 
 *      The second,  MBF_OICMBARI, is intended as a processing
 *      and archiving format as it allows the storage of both
 *      the unaltered raw values and editable and correctable
 *      bathymetry and sidescan.
 *   3. In both formats the  data consist of variable length 
 *      binary records.
 *   4. Each data record has three parts.  First there is a 248-byte
 *      header section containing the time stamp, navigation, a 
 *      variety of other values, the numbers and sizes of sonar
 *      data included in the record.  The second, optional, part of
 *      the record is "client specific information" of arbitrary 
 *      length. The third part contains up to eight arrays of
 *      "raw" sonar data followed by, in the MBF_OICMBARI format, 
 *      arrays of processing bathymetry and sidescan.
 *   5. The header begins with a four byte magic number. For the
 *      MBF_OICGEODA format the first three bytes are 'G', 'E', 
 *      and 'O'. For the MBF_OICMBARI format the first three bytes 
 *      are 'G', 'E', and '2'.The fourth byte is the id number for 
 *      the data source.
 *   6. The maximum numbers of beams and pixels defined below are
 *      limitations specific to MBIO - the formats themselves
 *      contain no limits on the amount of data per record.
 *   7. Comment records are encoded in MBIO using the "client
 *      specific information" section of the record and an unused
 *      sonar type value.
 *
 */

/* defines sizes of things */
#define	MBSYS_OIC_MAX_CLIENT		252
#define	MBSYS_OIC_MAX_COMMENT		MBSYS_OIC_MAX_CLIENT
#define	MBSYS_OIC_MAX_CHANNELS		8

/* define maximum number of beams */
#define	MBSYS_OIC_MAX_BEAMS		1024
#define	MBSYS_OIC_MAX_PIXELS		2048

/* define sonar types */
#define	OIC_ID_EGANDG			0
#define	OIC_ID_SEAVIEW			1
#define	OIC_ID_DEEPSCAN			2
#define	OIC_ID_STEST			3
#define	OIC_ID_QMIPS			4
#define	OIC_ID_SEAMARC2			5
#define	OIC_ID_DSLAMS120		22
#define	OIC_ID_DSLAMS200		23
#define	OIC_ID_COMMENT			255

/* define nav types */
#define	OIC_NAV_UTM			0
#define	OIC_NAV_LOCAL			1
#define	OIC_NAV_LONLAT			2

/* define sonar data types */
#define	OIC_TYPE_SIDESCAN		0
#define	OIC_TYPE_ANGLE			1
#define	OIC_TYPE_MULTIBEAM		2

/* define side */
#define	OIC_PORT			0
#define	OIC_STARBOARD			1
#define	OIC_SUBBOTTOM			2

/* define sonar data sizes */
#define	OIC_SIZE_CHAR			0
#define	OIC_SIZE_SHORT			1
#define	OIC_SIZE_INT			2
#define	OIC_SIZE_FLOAT			3
#define	OIC_SIZE_3FLOAT			4

struct mbsys_oic_channel_struct
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

struct mbsys_oic_seaview_struct
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

struct mbsys_oic_struct
	{
	int		kind;
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
	struct mbsys_oic_channel_struct channel[MBSYS_OIC_MAX_CHANNELS];
	char	client[MBSYS_OIC_MAX_CLIENT];
	int		rawsize[MBSYS_OIC_MAX_CHANNELS];
	char		*raw[MBSYS_OIC_MAX_CHANNELS];
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
	
/* system specific function prototypes */
int mbsys_oic_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_oic_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_oic_dimensions(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_oic_extract(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_oic_insert(int verbose, void *mbio_ptr, void *store_ptr, 
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_oic_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles, 
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset, 
			double *draft, double *ssv, int *error);
int mbsys_oic_detects(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams, int *detects, int *error);
int mbsys_oic_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude, 
			int *error);
int mbsys_oic_insert_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			double transducer_depth, double altitude, 
			int *error);
int mbsys_oic_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft, 
			double *roll, double *pitch, double *heave, 
			int *error);
int mbsys_oic_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft, 
			double roll, double pitch, double heave,
			int *error);
int mbsys_oic_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error);

