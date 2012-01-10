/*--------------------------------------------------------------------
 *    The MB-system:	mbf_sb2120xs.h	3/20/2000
 *	$Id$
 *
 *    Copyright (c) 2000-2012 by 
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
 * mbf_sb2120xs.h defines the data structures used by MBIO functions
 * to store multibeam data read from the MBF_SB2120XSE format (MBIO id 44).  
 *
 * Author:	P. A. Cohen
 * Date:	March 20, 2000
 * Author:	D. W. Caress
 * Date:	December 8,  2000
 *
 * $Log: mbf_sb2120xs.h,v $
 * Revision 5.0  2000/12/10 20:24:25  caress
 * Initial revision.
 *
 *
 *
 */
/*
 * Notes on the MBF_SB2120XS (XSE) data format:
 *   1. SeaBeam Instruments introduced a new format called XSE in 1999.
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
 *                           CurrentTrack(6), HeaveRollPitch (7), Heave(8), 
 *			     Roll(9), Pitch(10), Heading(11), Log(12)
 *        Sidescan      5    General(1), Amplitude(4), Phase(5)
 *        Multibeam     6    General(1), Traveltime(3), Quality(4), 
 *                           Amplitude(5), Delay(6), Lateral(7), 
 *                           Along(8), Depth(9), Angle(10), Beam(1)
 *        Comment       99   General(1) **MB-System ONLY!!!!**
 *   3. An additional set of SeaBeam 2100 specific frames are defined, 
 *      but are not supported in this i/o module. Many other frames
 *      are defined, but not supported here. These are read and passed
 *      through MB-System as MB_DATA_OTHER type data records.
 *   4. SeaBeam Instruments 2120 20KHz sonar systems output both bathymetry
 *      and amplitude information for up to 151 beams per multibeam frame.
 *      Each ping produces a variable number of beams.
 *   5. The XSE format uses asynchronous navigation only; navigation 
 *      is not included in the multibeam or sidescan pings.
 *      MB-System interpolates or extrapolates the available 
 *      navigation as necessary.
 *
 */

/* maximum number of beams and pixels */
#define	MBF_SB2120XS_MAXBEAMS	151
#define	MBF_SB2120XS_MAXPIXELS	2000
#define	MBF_SB2120XS_MAXSVP	200
#define MBF_SB2120XS_MAXDRAFT	200
#define	MBF_SB2120XS_COMMENT_LENGTH	200
#define	MBF_SB2120XS_BUFFER_SIZE		32000
#define	MBF_SB2120XS_DESCRIPTION_LENGTH	64

/* frame and group id's */
#define MBF_SB2120XS_NONE_FRAME		0

#define MBF_SB2120XS_NAV_FRAME			1
#define MBF_SB2120XS_NAV_GROUP_GEN		1
#define MBF_SB2120XS_NAV_GROUP_POS		2
#define MBF_SB2120XS_NAV_GROUP_ACCURACY		3
#define MBF_SB2120XS_NAV_GROUP_MOTIONGT		4
#define MBF_SB2120XS_NAV_GROUP_MOTIONTW		5
#define MBF_SB2120XS_NAV_GROUP_TRACK		6
#define MBF_SB2120XS_NAV_GROUP_HRP		7
#define MBF_SB2120XS_NAV_GROUP_HEAVE		8
#define MBF_SB2120XS_NAV_GROUP_ROLL		9
#define MBF_SB2120XS_NAV_GROUP_PITCH		10
#define MBF_SB2120XS_NAV_GROUP_HEADING		11
#define MBF_SB2120XS_NAV_GROUP_LOG		12
#define MBF_SB2120XS_NAV_GROUP_GPS		13

#define MBF_SB2120XS_SVP_FRAME			2
#define MBF_SB2120XS_SVP_GROUP_GEN		1
#define MBF_SB2120XS_SVP_GROUP_DEPTH		2
#define MBF_SB2120XS_SVP_GROUP_VELOCITY		3
#define MBF_SB2120XS_SVP_GROUP_CONDUCTIVITY	4
#define MBF_SB2120XS_SVP_GROUP_SALINITY		5
#define MBF_SB2120XS_SVP_GROUP_TEMP		6
#define MBF_SB2120XS_SVP_GROUP_PRESSURE		7
#define MBF_SB2120XS_SVP_GROUP_SSV		8
#define MBF_SB2120XS_SVP_GROUP_POS		9

#define MBF_SB2120XS_TID_FRAME			3
#define MBF_SB2120XS_TID_GROUP_GEN		1
#define MBF_SB2120XS_TID_GROUP_POS		2
#define MBF_SB2120XS_TID_GROUP_TIME		3
#define MBF_SB2120XS_TID_GROUP_TIDE		4

#define MBF_SB2120XS_SHP_FRAME			4
#define MBF_SB2120XS_SHP_GROUP_GEN		1
#define MBF_SB2120XS_SHP_GROUP_ATTITUDE		2
#define MBF_SB2120XS_SHP_GROUP_POS		3
#define MBF_SB2120XS_SHP_GROUP_DYNAMICS		4
#define MBF_SB2120XS_SHP_GROUP_MOTION		5
#define MBF_SB2120XS_SHP_GROUP_GEOMETRY		6
#define MBF_SB2120XS_SHP_GROUP_DESCRIPTION	7
#define MBF_SB2120XS_SHP_GROUP_PARAMETER		8

#define MBF_SB2120XS_SSN_FRAME			5
#define MBF_SB2120XS_SSN_GROUP_GEN		1
#define MBF_SB2120XS_SSN_GROUP_AMPVSTT		2
#define MBF_SB2120XS_SSN_GROUP_PHASEVSTT		3
#define MBF_SB2120XS_SSN_GROUP_AMPVSLAT		4
#define MBF_SB2120XS_SSN_GROUP_PHASEVSLAT		5

#define MBF_SB2120XS_MBM_FRAME			6
#define MBF_SB2120XS_MBM_GROUP_GEN		1
#define MBF_SB2120XS_MBM_GROUP_BEAM		2
#define MBF_SB2120XS_MBM_GROUP_TT			3
#define MBF_SB2120XS_MBM_GROUP_QUALITY		4
#define MBF_SB2120XS_MBM_GROUP_AMP		5
#define MBF_SB2120XS_MBM_GROUP_DELAY		6
#define MBF_SB2120XS_MBM_GROUP_LATERAL		7
#define MBF_SB2120XS_MBM_GROUP_ALONG		8
#define MBF_SB2120XS_MBM_GROUP_DEPTH		9
#define MBF_SB2120XS_MBM_GROUP_ANGLE		10
#define MBF_SB2120XS_MBM_GROUP_HEAVE		11
#define MBF_SB2120XS_MBM_GROUP_ROLL		12
#define MBF_SB2120XS_MBM_GROUP_PITCH		13
#define MBF_SB2120XS_MBM_GROUP_GATES		14
#define MBF_SB2120XS_MBM_GROUP_NOISE		15
#define MBF_SB2120XS_MBM_GROUP_LENGTH		16
#define MBF_SB2120XS_MBM_GROUP_HITS		17

#define MBF_SB2120XS_COM_FRAME			99
#define MBF_SB2120XS_COM_GROUP_GEN		1

struct mbf_sb2120xs_beam_struct
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
	};

struct mbf_sb2120xs_struct
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
	
	/* svp (sound velocity frames) */
	int	svp_source;		/* sensor id */
	unsigned int	svp_sec;	/* sec since 1/1/1901 00:00 */
	unsigned int	svp_usec;	/* microseconds */
	int	svp_nsvp;		/* number of depth values */
	int	svp_nctd;		/* number of ctd values */
	double	svp_depth[MBF_SB2120XS_MAXSVP];		/* m */
	double	svp_velocity[MBF_SB2120XS_MAXSVP];	/* m/s */
	double	svp_conductivity[MBF_SB2120XS_MAXSVP];	/* mmho/cm */
	double	svp_salinity[MBF_SB2120XS_MAXSVP];	/* o/oo */
	double	svp_temperature[MBF_SB2120XS_MAXSVP];	/* degree celcius */
	double	svp_pressure[MBF_SB2120XS_MAXSVP];	/* bar */
	double	svp_ssv;		/* m/s */

	/* position (navigation frames) */
	int	nav_source;		/* sensor id */
	unsigned int	nav_sec;	/* sec since 1/1/1901 00:00 */
	unsigned int	nav_usec;	/* microseconds */
	int	nav_quality;
	int	nav_status;
	int	nav_description_len;
	char	nav_description[MBF_SB2120XS_DESCRIPTION_LENGTH];
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
	struct mbf_sb2120xs_beam_struct beams[MBF_SB2120XS_MAXBEAMS];
	
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
	short	ss[MBF_SB2120XS_MAXPIXELS]; /* sidescan amplitude in dB */

	/* comment */
	int	com_source;		/* sensor id */
	unsigned int	com_sec;	/* sec since 1/1/1901 00:00 */
	unsigned int	com_usec;	/* microseconds */	
	char	comment[MBF_SB2120XS_COMMENT_LENGTH];
	
	/* unsupported frames */
	int	rawsize;		/* size of unknown frame in bytes */
	char	raw[MBF_SB2120XS_BUFFER_SIZE];
	};
