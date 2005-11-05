/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_xse.h	3/27/2000
 *	$Id: mbsys_xse.h,v 5.9 2005-11-05 00:48:03 caress Exp $
 *
 *    Copyright (c) 2000, 2001, 2002, 2003 by 
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
 *      MBF_L3XSERAW : MBIO ID 94
 *
 *
 * Author:	D. W. Caress
 * Date:	August 1,  1999
 * Additional Authors:	P. A. Cohen and S. Dzurenko
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.8  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.7  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.6  2001/12/30 20:36:13  caress
 * Fixed array overflows in handling XSE data.
 *
 * Revision 5.5  2001/12/20 20:48:51  caress
 * Release 5.0.beta11
 *
 * Revision 5.4  2001/07/22  21:19:23  caress
 * Removed redundant define.
 *
 * Revision 5.3  2001/07/20 00:32:54  caress
 * Release 5.0.beta03
 *
 * Revision 5.2  2001/04/06  22:05:59  caress
 * Consolidated xse formats into one format.
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
 * Revision 4.0  1999/08/08  04:14:35  caress
 * Initial revision.
 *
 *
 */
/*
 * Notes on the MBSYS_XSE (XSE) data format:
 *   1. L3 Communications introduced a new format called XSE in 1999.
 *      SeaBeam Instruments (maker of SeaBeam multibeam sonars)  
 *      and Elac Nautik (make of Bottomchart multibeam sonars) 
 *      are both divisions of L3 Communications.
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
 *        Navigation      1  General(1), Position(2), Accuracy(3), 
 *                           MotionGroundTruth(4), MotionThroughWater(5), 
 *                           CurrentTrack(6), HeaveRollPitch(7), Heave(8), 
 *			     Roll(9), Pitch(10), Heading(11), Log(12), 
 *                           GPS(13)
 *        Sound Velocity  2  General(1), 
 *                           Depth(2), Velocity(3), 
 *                           Conductivity(4), Salinity (5), Temperature(6), 
 *                           Pressure(7), SSV(8), Position(9)
 *        Tide            3  General(1), Position(2), 
 *                           Time(3), Tide(4)
 *        Ship            4  General(1), Attitude(2), 
 *                           Position(3), Dynamics(4), Motion(5), 
 *                           Geometry(6), Description(7), 
 *                           Parameter(8)
 *        Sidescan        5  General(1), AmplitudeVsTravelTime(2), 
 *                           PhaseVsTravelTime(3), 
 *                           Amplitude(4), Phase(5)
 *        Multibeam       6  General(1), Beam(2), 
 *                           Traveltime(3), Quality(4), 
 *                           Amplitude(5), Delay(6), Lateral(7), 
 *                           Along(8), Depth(9), Angle(10),  
 *                           Heave(11), Roll(12), Pitch(13),
 *                           Gates(14), Noise(15), EchoLength(16), 
 *                           Hits(17)
 *        Single Beam     7  General(1)
 *        Control         8  Request(1), Insert(2), Change(3),  
 *                           Add(4), Delete(5), Action(6), Reply(7)
 *        Bathymetry      9  General(1), Position(2), Depth(3)
 *        Project        10  General(1), Server(2), Status(3), Sources(4)
 *        Native         11  General(1), RWCollectable(2), UNB(3),  
 *                           Raw(4), ELAC(5)
 *        Geodetic       12  General(1), Ellipsoid(2), Datum(3),  
 *                           Projection(4), System(5), Alias(6)
 *        SeaBeam        13  Properties(1), HeaveRollPitch(2), Setup(3),  
 *                           MotionReferenceUnit(4), Settings(5), 
 *                           Beams(6), Gates(7), Raw(8), Center(9), 
 *                           Sidescan, Shutdown(), Ping(), Calibrate(), 
 *                           Collect(), Surface(), Hydrophone(), 
 *                           Projector(17), Calibration(18), Acknowledge(19), 
 *                           Warning(20), Message(21), Error(22)
 *        Comment        99  General(1) **MB-System ONLY!!!!**
 *   3. Not all of these frames are directly supported by this MB-System
 *      i/o module. Unsupported frames are read and passed
 *      through MB-System as MB_DATA_OTHER type data records.
 *   4. SeaBeam Instruments 2120 12 kHz and 20 KHz multibeam sonar 
 *      systems output both bathymetry and amplitude 
 *      information for up to 151 beams per multibeam frame.
 *      Each ping produces a variable number of beams.
 *   5. Elac Bottomchart MkII  50 KHz and 180 kHz sonar systems 
 *      output both bathymetry and amplitude information 
 *      for up to 126 beams per multibeam frame.
 *      Each ping produces a variable number of beams.
 *   6. The XSE format uses asynchronous navigation only; navigation 
 *      is not included in the multibeam or sidescan pings.
 *      MB-System interpolates or extrapolates the available 
 *      navigation as necessary.
 *   7. The comment records are supported by MB-System only and are
 *      not part of the L3 Communications XSE format specification.
 *
 */

/* maximum number of beams and pixels */
#define	MBSYS_XSE_MAXBEAMS		151
#define	MBSYS_XSE_MAXPIXELS		2000
#define	MBSYS_XSE_MAXSAMPLES		8192
#define	MBSYS_XSE_MAXSVP		200
#define MBSYS_XSE_MAXDRAFT		200
#define	MBSYS_XSE_COMMENT_LENGTH	200
#define	MBSYS_XSE_DESCRIPTION_LENGTH	64
#define	MBSYS_XSE_TIME_OFFSET		2177452800.0
#define	MBSYS_XSE_BUFFER_SIZE		32000
#define	MBSYS_XSE_MAX_SIZE		200

/* frame and group id's */
#define MBSYS_XSE_NONE_FRAME		0

#define MBSYS_XSE_NAV_FRAME			1
#define MBSYS_XSE_NAV_GROUP_GEN		1
#define MBSYS_XSE_NAV_GROUP_POS		2
#define MBSYS_XSE_NAV_GROUP_ACCURACY		3
#define MBSYS_XSE_NAV_GROUP_MOTIONGT		4
#define MBSYS_XSE_NAV_GROUP_MOTIONTW		5
#define MBSYS_XSE_NAV_GROUP_TRACK		6
#define MBSYS_XSE_NAV_GROUP_HRP		7
#define MBSYS_XSE_NAV_GROUP_HEAVE		8
#define MBSYS_XSE_NAV_GROUP_ROLL		9
#define MBSYS_XSE_NAV_GROUP_PITCH		10
#define MBSYS_XSE_NAV_GROUP_HEADING		11
#define MBSYS_XSE_NAV_GROUP_LOG		12
#define MBSYS_XSE_NAV_GROUP_GPS		13

#define MBSYS_XSE_SVP_FRAME			2
#define MBSYS_XSE_SVP_GROUP_GEN		1
#define MBSYS_XSE_SVP_GROUP_DEPTH		2
#define MBSYS_XSE_SVP_GROUP_VELOCITY		3
#define MBSYS_XSE_SVP_GROUP_CONDUCTIVITY	4
#define MBSYS_XSE_SVP_GROUP_SALINITY		5
#define MBSYS_XSE_SVP_GROUP_TEMP		6
#define MBSYS_XSE_SVP_GROUP_PRESSURE		7
#define MBSYS_XSE_SVP_GROUP_SSV		8
#define MBSYS_XSE_SVP_GROUP_POS		9

#define MBSYS_XSE_TID_FRAME			3
#define MBSYS_XSE_TID_GROUP_GEN		1
#define MBSYS_XSE_TID_GROUP_POS		2
#define MBSYS_XSE_TID_GROUP_TIME		3
#define MBSYS_XSE_TID_GROUP_TIDE		4

#define MBSYS_XSE_SHP_FRAME			4
#define MBSYS_XSE_SHP_GROUP_GEN		1
#define MBSYS_XSE_SHP_GROUP_TIME		2
#define MBSYS_XSE_SHP_GROUP_DRAFT		3
#define MBSYS_XSE_SHP_GROUP_SENSORS		4
#define MBSYS_XSE_SHP_GROUP_MOTION		5
#define MBSYS_XSE_SHP_GROUP_GEOMETRY		6
#define MBSYS_XSE_SHP_GROUP_DESCRIPTION	7
#define MBSYS_XSE_SHP_GROUP_PARAMETER	8

#define MBSYS_XSE_SSN_FRAME			5
#define MBSYS_XSE_SSN_GROUP_GEN		1
#define MBSYS_XSE_SSN_GROUP_AMPVSTT		2
#define MBSYS_XSE_SSN_GROUP_PHASEVSTT	3
#define MBSYS_XSE_SSN_GROUP_AMPVSLAT		4
#define MBSYS_XSE_SSN_GROUP_PHASEVSLAT	5

#define MBSYS_XSE_MBM_FRAME			6
#define MBSYS_XSE_MBM_GROUP_GEN		1
#define MBSYS_XSE_MBM_GROUP_BEAM		2
#define MBSYS_XSE_MBM_GROUP_TT		3
#define MBSYS_XSE_MBM_GROUP_QUALITY		4
#define MBSYS_XSE_MBM_GROUP_AMP		5
#define MBSYS_XSE_MBM_GROUP_DELAY		6
#define MBSYS_XSE_MBM_GROUP_LATERAL		7
#define MBSYS_XSE_MBM_GROUP_ALONG		8
#define MBSYS_XSE_MBM_GROUP_DEPTH		9
#define MBSYS_XSE_MBM_GROUP_ANGLE		10
#define MBSYS_XSE_MBM_GROUP_HEAVE		11
#define MBSYS_XSE_MBM_GROUP_ROLL		12
#define MBSYS_XSE_MBM_GROUP_PITCH		13
#define MBSYS_XSE_MBM_GROUP_GATES		14
#define MBSYS_XSE_MBM_GROUP_NOISE		15
#define MBSYS_XSE_MBM_GROUP_LENGTH		16
#define MBSYS_XSE_MBM_GROUP_HITS		17

#define MBSYS_XSE_SNG_FRAME			7
#define MBSYS_XSE_CNT_FRAME			8
#define MBSYS_XSE_BTH_FRAME			9
#define MBSYS_XSE_PRD_FRAME			10
#define MBSYS_XSE_NTV_FRAME			11
#define MBSYS_XSE_GEO_FRAME			12

#define MBSYS_XSE_SBM_FRAME			13
#define MBSYS_XSE_SBM_GROUP_PROPERTIES	1
#define MBSYS_XSE_SBM_GROUP_HRP		2
#define MBSYS_XSE_SBM_GROUP_SETUP		3
#define MBSYS_XSE_SBM_GROUP_MRU		4
#define MBSYS_XSE_SBM_GROUP_SETTINGS		5
#define MBSYS_XSE_SBM_GROUP_BEAMS		6
#define MBSYS_XSE_SBM_GROUP_GATES		7
#define MBSYS_XSE_SBM_GROUP_RAW		8
#define MBSYS_XSE_SBM_GROUP_CENTER		9
#define MBSYS_XSE_SBM_GROUP_MESSAGE		21

#define MBSYS_XSE_MSG_FRAME			14
#define MBSYS_XSE_ATT_FRAME			15

#define MBSYS_XSE_COM_FRAME			99
#define MBSYS_XSE_COM_GROUP_GEN		1

struct mbsys_xse_beam_struct
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
	short	    beam;
	char	    quality;
	short	    amplitude;
	double	    gate_angle;
	double	    gate_start;
	double	    gate_stop;
	float	    noise;
	float	    length;
	unsigned int hits;
	};

struct mbsys_xse_struct
	{
	/* type of data record */
	int	kind;			/* Survey, nav, Comment */
	
	/* parameter (ship frames) */
	int	par_source;		/* sensor id */
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
	unsigned int    par_length;	/* length of ship name, chars */
	char    par_ship_name[MBSYS_XSE_DESCRIPTION_LENGTH]; /* Name of Vessel */
	double  par_ship_length;	/* vessel total length, meters */
	double  par_ship_width;		/* vessel total width, meters */
	double  par_ship_draft;		/* vessel maximum draft, meters */
	double  par_ship_height;	/* vessel maximum height, meters */
	double  par_ship_displacement;	/* vessel maximum displacement, cubic meters */
	double  par_ship_weight;	/* vessel maximum weight, kg */
	unsigned int	par_ndraft_time;	/* number of times for each draft  */
	unsigned int	par_draft_time[MBSYS_XSE_MAX_SIZE]; /* UTC time for each draft value, seconds */
	unsigned int	par_num_drafts;		/* number of draft values */
	double	par_draft_value[MBSYS_XSE_MAX_SIZE]; /* Array of draft values, meters */
	unsigned int	par_num_sensors;	/* number of external sensors */
	unsigned int	par_sensors_id[MBSYS_XSE_MAX_SIZE]; /* Array of sensor id's */
	unsigned int	par_num_motion;		/* number of motion sensors */
	unsigned int	par_motion[MBSYS_XSE_MAX_SIZE]; /* Array of motion values */
	unsigned int    par_num_geometry;	/* number of geometry values */
	unsigned int    par_geometry[MBSYS_XSE_MAX_SIZE]; /* Array of geometry values */
	unsigned int   par_num_description;	/* length of description string */
	char    par_description[MBSYS_XSE_MAX_SIZE]; /* Sensor Description string */
	
	/* svp (sound velocity frames) */
	int	svp_source;		/* sensor id */
	unsigned int	svp_sec;	/* sec since 1/1/1901 00:00 */
	unsigned int	svp_usec;	/* microseconds */
	int	svp_nsvp;		/* number of depth values */
	int	svp_nctd;		/* number of ctd values */
	double	svp_depth[MBSYS_XSE_MAXSVP];		/* m */
	double	svp_velocity[MBSYS_XSE_MAXSVP];	/* m/s */
	double	svp_conductivity[MBSYS_XSE_MAXSVP];	/* mmho/cm */
	double	svp_salinity[MBSYS_XSE_MAXSVP];	/* o/oo */
	double	svp_temperature[MBSYS_XSE_MAXSVP];	/* degree celcius */
	double	svp_pressure[MBSYS_XSE_MAXSVP];	/* bar */
	double	svp_ssv;		/* m/s */

	/* position (navigation frames) */
	int	nav_source;		/* sensor id */
	unsigned int	nav_sec;	/* sec since 1/1/1901 00:00 */
	unsigned int	nav_usec;	/* microseconds */
	int	nav_quality;
	int	nav_status;
	int	nav_description_len;
	char	nav_description[MBSYS_XSE_DESCRIPTION_LENGTH];
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
	int	mul_group_gates;	/* boolean flag - gates group read */
	int	mul_group_noise;	/* boolean flag - noise group read */
	int	mul_group_length;	/* boolean flag - length group read */
	int	mul_group_hits;		/* boolean flag - hits group read */
	int	mul_source;		/* sensor id */
	unsigned int	mul_sec;	/* sec since 1/1/1901 00:00 */
	unsigned int	mul_usec;	/* microseconds */
	int	mul_ping;		/* ping number */
	float	mul_frequency;		/* transducer frequency (Hz) */
	float	mul_pulse;		/* transmit pulse length (sec) */
	float	mul_power;		/* transmit power (dB) */
	float	mul_bandwidth;		/* receive bandwidth (Hz) */
	float	mul_sample;		/* receive sample interval (sec) */
	float	mul_swath;		/* swath width (radians) */
	int	mul_num_beams;		/* number of beams */
	double	mul_x;			/* longitude in degrees */
	double	mul_y;			/* latitude in degrees */
	struct mbsys_xse_beam_struct beams[MBSYS_XSE_MAXBEAMS];
	
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
	short	ss[MBSYS_XSE_MAXPIXELS]; /* sidescan amplitude in dB */
	
	/* seabeam (seabeam frames) */
	int	sbm_properties;		/* boolean flag - sbm properties group read */
	int	sbm_hrp;		/* boolean flag - sbm hrp group read */
	int	sbm_center;		/* boolean flag - sbm center group read */
	int	sbm_source;		/* sensor id */
	int	sbm_message;		/* sensor id */
	unsigned int	sbm_sec;	/* sec since 1/1/1901 00:00 */
	unsigned int	sbm_usec;	/* microseconds */
	int	sbm_ping;		/* ping number */
	float	sbm_ping_gain;		/* ping gain (dB) */
	float	sbm_pulse_width;	/* pulse width (s) */
	float	sbm_transmit_power;	/* transmit power (dB) */
	float	sbm_pixel_width;	/* pixel width (m) */
	float	sbm_swath_width;	/* swath width (radians) */
	float	sbm_time_slice;		/* time slice (s) */
	int	sbm_depth_mode;		/* depth mode (1=shallow, 2=deep) */
	int	sbm_beam_mode;		/* focused beam mode (0=off, 1=on) */
	float	sbm_ssv;		/* surface sound velocity (m/s) */
	float	sbm_frequency;		/* sonar frequency (kHz) */
	float	sbm_bandwidth;		/* receiver bandwidth (kHz) */
	double	sbm_heave;		/* heave (m) */
	double	sbm_roll;		/* roll (radians) */
	double	sbm_pitch;		/* pitch (radians) */
	int	sbm_center_beam;	/* beam number for center beam profile */
	int	sbm_center_count;	/* number of samples in center beam profile */
	float	sbm_center_amp[MBSYS_XSE_MAXSAMPLES];	/* center beam profile values */
	int	sbm_message_id;		/* seabeam message id */
	int	sbm_message_len;	/* seabeam message length */
	char	sbm_message_txt[MBSYS_XSE_COMMENT_LENGTH]; /* seabeam message */

	/* comment */
	int	com_source;		/* sensor id */
	unsigned int	com_sec;	/* sec since 1/1/1901 00:00 */
	unsigned int	com_usec;	/* microseconds */	
	char	comment[MBSYS_XSE_COMMENT_LENGTH];
	
	/* unsupported frames */
	int	rawsize;		/* size of unknown frame in bytes */
	char	raw[MBSYS_XSE_BUFFER_SIZE];
	};

	
/* system specific function prototypes */
int mbsys_xse_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_xse_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_xse_dimensions(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_xse_extract(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_xse_insert(int verbose, void *mbio_ptr, void *store_ptr, 
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_xse_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles, 
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset, 
			double *draft, double *ssv, int *error);
int mbsys_xse_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude, 
			int *error);
int mbsys_xse_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft, 
			double *roll, double *pitch, double *heave, 
			int *error);
int mbsys_xse_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft, 
			double roll, double pitch, double heave,
			int *error);
int mbsys_xse_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, 
			int *nsvp, 
			double *depth, double *velocity,
			int *error);
int mbsys_xse_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int nsvp, 
			double *depth, double *velocity,
			int *error);
int mbsys_xse_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error);

