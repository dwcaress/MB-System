/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_sb2120.h	3/27/2000
 *	$Id: mbsys_sb2120.h,v 5.1 2001-01-22 07:43:34 caress Exp $
 *
 *    Copyright (c) 2000 by 
 *    D. W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbsys_xse.h defines the data structures used by MBIO functions
 * to store swath sonar data in the XSE Data Exchange Format
 * developed by L-3 Communications ELAC Nautik.
 * This format is used for data from ELAC Bottomchart multibeam sonars
 * and SeaBeam 2100 multibeam sonars (made by L-3 Communications
 * SeaBeam Instruments).
 * The data format associated with XSE is:
 *      MBF_ELMK2HYD : MBIO ID 94
 *
 *
 * Author:	P. A. Cohen
 * Date:	March 27, 2000
 * Author:	D. W. Caress
 * Date:	December 7,  2000
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.0  2000/12/10  20:24:25  caress
 * Initial revision.
 *
 *
 */
/*
 * Notes on the XSE data format:
 *   1. ELAC Nautik introduced a new format called XSE in 1999.
 *      This "Data Exchange Format" is associated with the new
 *      version of "Hydrostar ONLINE" and represents the intended
 *      data format for both ELAC Bottomchart Compact MK II sonars
 *      (50 kHz and/or 180 kHz) and SeaBeam 2100 series sonars
 *      (12 kHz, 20 kHz, 36 kHz). This follows the purchase of
 *      SeaBeam Instruments by L3 Communications, the parent 
 *      company of ELAC Nautik.
 *   2. The XSE format implements a well defined binary format
 *      structure in which each data record is represented as
 *      a "frame" with the following structure:
 *          -------------------------------------------------------
 *          Item     Bytes   Format   Value   Units   Description
 *          -------------------------------------------------------
 *          Start      4     ulong    $HSF            Frame start
 *          Byte Count 4     ulong            bytes   Between byte count
 *                                                      and frame end
 *          Id         4     ulong                    Frame id - see below
 *          Source     4     ulong                    Sensor id
 *          Seconds    4     ulong            seconds Seconds since
 *                                                      1/1/1901 00:00:00
 *          Microsec   4     ulong            usec    Microseconds
 *          ...        ...   ...      ...     ...     Frame specific groups
 *          End        4     ulong    #HSF            Frame end
 *          -------------------------------------------------------
 *      Within each frame are "groups", each with the following structure:
 *          -------------------------------------------------------
 *          Item     Bytes   Format   Value   Units   Description
 *          -------------------------------------------------------
 *          Start      4     ulong    $HSG            Group start
 *          Byte Count 4     ulong            bytes   Between byte count
 *                                                      and group end
 *          Id         4     ulong                    Group id - see below
 *          ...        ...   ...      ...     ...     Group specific data
 *          End        4     ulong    #HSG            Group end
 *          -------------------------------------------------------
 *   2. The valid frames include:
 *        Frame Name    Id   Groups w/ group id's in ()
 *        ---------------------------------------------------------
 *        Navigation    1    General(1), Position(2), 
 *                           MotionGroundTruth(4), MotionThroughWater(5), 
 *                           CurrentTrack(6)
 *        Sidescan      5    General(1), Amplitude(4), Phase(5)
 *        Multibeam     6    General(1), Traveltime(3), Quality(4), 
 *                           Amplitude(5), Delay(6), Lateral(7), 
 *                           Along(8), Depth(9), Angle(10), Beam(1)
 *        Comment       99   General(1) **MB-System ONLY!!!!**
 *   3. An additional set of SeaBeam 2100 specific frames are defined, 
 *      but are not supported in this i/o module. Many other frames
 *      are defined, but not supported here. These are read and passed
 *      through MB-System as MB_DATA_OTHER type data records.
 *   4. Elac BottomChart Compact MK II sonar systems output both bathymetry
 *      and amplitude information for up to 126 beams per multibeam frame.
 *      Each ping produces 42 beams.  A wide swath is constructed
 *      by successively pinging in different directions.
 *   5. The XSE format uses asynchronous navigation only; navigation 
 *      is not included in the multibeam or sidescan pings.
 *      MB-System interpolates or extrapolates the available 
 *      navigation as necessary.
 *
 */

/* maximum number of beams and pixels */
#define	MBSYS_SB2120_MAXBEAMS		151
#define	MBSYS_SB2120_MAXPIXELS		2000
#define	MBSYS_SB2120_MAXSVP		200
#define	MBSYS_SB2120_COMMENT_LENGTH	200
#define	MBSYS_SB2120_BUFFER_SIZE	10000
#define	MBSYS_SB2120_DESCRIPTION_LENGTH	64
#define	MBSYS_SB2120_TIME_OFFSET	((unsigned int) 2177452800)
#define	MBSYS_SB2120_BUFFER_SIZE	10000
#define	MBSYS_SB2120_MAX_SIZE		200

struct mbsys_sb2120_beam_struct
	{
	double	    tt;
	double	    delay;
	double	    lateral;
	double	    along;
	double	    depth;
	double	    angle;
	double	    heave;
	double	    roll;
	double	    pitch;
	int	    beam;
	int	    quality;
	int	    amplitude;
	};

struct mbsys_sb2120_struct
	{
	/* type of data record */
	int	kind;			/* Survey, Nav, Comment */
	
	/* parameter (ship frames) */
	int	par_source;		/* sensor id */
	/* Add New ship frames group & fields 3/20/2000 */
	unsigned int    par_length;	/* length of ship name, chars */
	char    par_ship_name[MBSYS_SB2120_DESCRIPTION_LENGTH]; /* Name of Vessel */
	double  par_ship_length;	/* vessel total length, meters */
	double  par_ship_width;		/* vessel total width, meters */
	double  par_ship_draft;		/* vessel maximum draft, meters */
	double  par_ship_height;	/* vessel maximum height, meters */
	double  par_ship_displacement;	/* vessel maximum displacement, cubic meters */
	double  par_ship_weight;	/* vessel maximum weight, kg */
	unsigned int	par_ndraft_time;	/* number of times for each draft  */
	unsigned int	par_draft_time[MBSYS_SB2120_MAX_SIZE]; /* UTC time for each draft value, seconds */
	unsigned int	par_num_drafts;		/* number of draft values */
	double	par_draft_value[MBSYS_SB2120_MAX_SIZE]; /* Array of draft values, meters */
	unsigned int	par_num_sensors;	/* number of external sensors */
	unsigned int	par_sensors_id[MBSYS_SB2120_MAX_SIZE]; /* Array of sensor id's */
	unsigned int	par_num_motion;		/* number of motion sensors */
	unsigned int	par_motion[MBSYS_SB2120_MAX_SIZE]; /* Array of motion values */
	unsigned int    par_num_geometry;	/* number of geometry values */
	unsigned int    par_geometry[MBSYS_SB2120_MAX_SIZE]; /* Array of geometry values */
	unsigned int   par_num_description;	/* length of description string */
	char    par_description[MBSYS_SB2120_MAX_SIZE]; /* Sensor Description string */
	
	unsigned int	par_sec;	/* sec since 1/1/1901 00:00 */
	unsigned int	par_usec;	/* microseconds */
	float	par_roll_bias;		/* radians */
	float	par_pitch_bias;		/* radians */
	float	par_heading_bias;	/* radians */
	float	par_time_delay;		/* nav time lag, seconds */
	float	par_trans_x_port;	/* port transducer x position, meters */
	float	par_trans_y_port;	/* port transducer y position, meters */
	float	par_trans_z_port;	/* port transducer z position, meters */
	float	par_trans_x_stbd;	/* starboard transducer x position, meters */
	float	par_trans_y_stbd;	/* starboard transducer y position, meters */
	float	par_trans_z_stbd;	/* starboard transducer z position, meters */
	float	par_trans_err_port;	/* port transducer rotation in roll direction, radians */
	float	par_trans_err_stbd;	/* starboard transducer rotation in roll direction, radians */
	float	par_nav_x;		/* navigation antenna x position, meters */
	float	par_nav_y;		/* navigation antenna y position, meters */
	float	par_nav_z;		/* navigation antenna z position, meters */
	float	par_hrp_x;		/* motion sensor x position, meters */
	float	par_hrp_y;		/* motion sensor y position, meters */
	float	par_hrp_z;		/* motion sensor z position, meters */
	
	/* svp (sound velocity frames) */
	int	svp_source;		/* sensor id */
	unsigned int	svp_sec;	/* sec since 1/1/1901 00:00 */
	unsigned int	svp_usec;	/* microseconds */
	int	svp_nsvp;		/* number of depth values */
	int	svp_nctd;		/* number of ctd values */
	double	svp_depth[MBSYS_SB2120_MAXSVP];	/* m */
	double	svp_velocity[MBSYS_SB2120_MAXSVP];	/* m/s */
	double	svp_conductivity[MBSYS_SB2120_MAXSVP];	/* mmho/cm */
	double	svp_salinity[MBSYS_SB2120_MAXSVP];	/* o/oo */
	double	svp_temperature[MBSYS_SB2120_MAXSVP];	/* degree celcius */
	double	svp_pressure[MBSYS_SB2120_MAXSVP];	/* bar */
	double	svp_ssv;			/* m/s */

	/* position (navigation frames) */
	int	nav_source;		/* sensor id */
	unsigned int	nav_sec;	/* sec since 1/1/1901 00:00 */
	unsigned int	nav_usec;	/* microseconds */
	int	nav_quality;
	int	nav_status;
	int	nav_description_len;
	char	nav_description[MBSYS_SB2120_DESCRIPTION_LENGTH];
	double	nav_x;			/* eastings (m) or 
					    longitude (radians) */
	double	nav_y;			/* northings (m) or 
					    latitude (radians) */
	double	nav_z;			/* height (m) or 
					    ellipsoidal height (m) */
	double	nav_speed_ground;	/* m/s */
	double	nav_course_ground;	/* radians */
	double	nav_speed_water;	/* m/s */
	double	nav_course_water;	/* radians */
	
	/* survey depth (multibeam frames) */
	int	mul_frame;		/* boolean flag - multibeam frame read */
	int	mul_group_beam;		/* boolean flag - beam group read */
	int	mul_group_tt;		/* boolean flag - tt group read */
	int	mul_group_quality;	/* boolean flag - quality group read */
	int	mul_group_amp;		/* boolean flag - amp group read */
	int	mul_group_delay;	/* boolean flag - delay group read */
	int	mul_group_lateral;	/* boolean flag - lateral group read */
	int	mul_group_along;	/* boolean flag - along group read */
	int	mul_group_depth;	/* boolean flag - depth group read */
	int	mul_group_angle;	/* boolean flag - angle group read */
	int	mul_group_heave;	/* boolean flag - heave group read */
	int	mul_group_roll;		/* boolean flag - roll group read */
	int	mul_group_pitch;	/* boolean flag - pitch group read */
	int	mul_source;		/* sensor id */
	unsigned int	mul_sec;	/* sec since 1/1/1901 00:00 */
	unsigned int	mul_usec;	/* microseconds */
	double	mul_x;			/* interpolated longitude in degrees */
	double	mul_y;			/* interpolated latitude in degrees */
	int	mul_ping;		/* ping number */
	double	mul_frequency;		/* transducer frequency (Hz) */
	double	mul_pulse;		/* transmit pulse length (sec) */
	double	mul_power;		/* transmit power (dB) */
	double	mul_bandwidth;		/* receive bandwidth (Hz) */
	double	mul_sample;		/* receive sample interval (sec) */
	double	mul_swath;		/* swath width (radians) */
	int	mul_num_beams;		/* number of beams */
	struct mbsys_sb2120_beam_struct beams[MBSYS_SB2120_MAXBEAMS];
	
	/* survey sidescan (sidescan frames) */
	int	sid_frame;		/* boolean flag - sidescan frame read */
	int	sid_source;		/* sensor id */
	unsigned int	sid_sec;	/* sec since 1/1/1901 00:00 */
	unsigned int	sid_usec;	/* microseconds */
	int	sid_ping;		/* ping number */
	float	sid_frequency;		/* transducer frequency (Hz) */
	float	sid_pulse;		/* transmit pulse length (sec) */
	float	sid_power;		/* transmit power (dB) */
	float	sid_bandwidth;		/* receive bandwidth (Hz) */
	float	sid_sample;		/* receive sample interval (sec) */
	int	sid_bin_size;		/* bin size in mm */
	int	sid_offset;		/* lateral offset in mm */
	int	sid_num_pixels;		/* number of pixels */
	short	ss[MBSYS_SB2120_MAXPIXELS]; /* sidescan amplitude in dB */

	/* comment */
	int	com_source;		/* sensor id */
	unsigned int	com_sec;	/* sec since 1/1/1901 00:00 */
	unsigned int	com_usec;	/* microseconds */	
	char	comment[MBSYS_SB2120_COMMENT_LENGTH];
	
	/* unsupported frames */
	int	rawsize;		/* size of unknown frame in bytes */
	char	raw[MBSYS_SB2120_BUFFER_SIZE];
	};

	
/* system specific function prototypes */
int mbsys_sb2120_alloc(int verbose, char *mbio_ptr, char **store_ptr, 
			int *error);
int mbsys_sb2120_deall(int verbose, char *mbio_ptr, char **store_ptr, 
			int *error);
int mbsys_sb2120_extract(int verbose, char *mbio_ptr, char *store_ptr, 
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_sb2120_insert(int verbose, char *mbio_ptr, char *store_ptr, 
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_sb2120_ttimes(int verbose, char *mbio_ptr, char *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles, 
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset, 
			double *draft, double *ssv, int *error);
int mbsys_sb2120_extract_altitude(int verbose, char *mbio_ptr, char *store_ptr,
			int *kind, double *transducer_depth, double *altitude, 
			int *error);
int mbsys_sb2120_extract_nav(int verbose, char *mbio_ptr, char *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft, 
			double *roll, double *pitch, double *heave, 
			int *error);
int mbsys_sb2120_insert_nav(int verbose, char *mbio_ptr, char *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft, 
			double roll, double pitch, double heave,
			int *error);
int mbsys_sb2120_extract_svp(int verbose, char *mbio_ptr, char *store_ptr,
			int *kind, 
			int *nsvp, 
			double *depth, double *velocity,
			int *error);
int mbsys_sb2120_insert_svp(int verbose, char *mbio_ptr, char *store_ptr,
			int nsvp, 
			double *depth, double *velocity,
			int *error);
int mbsys_sb2120_copy(int verbose, char *mbio_ptr, 
			char *store_ptr, char *copy_ptr,
			int *error);

