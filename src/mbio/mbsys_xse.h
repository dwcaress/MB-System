/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_xse.h	3/27/2000
 *	$Id$
 *
 *    Copyright (c) 2000-2009 by 
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
 * $Log: mbsys_xse.h,v $
 * Revision 5.12  2007/07/03 17:28:08  caress
 * Fixes to XSE format.
 *
 * Revision 5.11  2007/06/18 01:19:48  caress
 * Changes as of 17 June 2007.
 *
 * Revision 5.10  2006/09/11 18:55:53  caress
 * Changes during Western Flyer and Thomas Thompson cruises, August-September
 * 2006.
 *
 * Revision 5.9  2005/11/05 00:48:03  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
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
 *                           Parameter(8), NavigationAndMotion(9),
 *                           Transducer(10), TransducerExtended(11)
 *        Sidescan        5  General(1), AmplitudeVsTravelTime(2), 
 *                           PhaseVsTravelTime(3), 
 *                           Amplitude(4), Phase(5), Signal(6), PingType(7)
 *                           ComplexSignal(8), Weighting(9)
 *        Multibeam       6  General(1), Beam(2), 
 *                           Traveltime(3), Quality(4), 
 *                           Amplitude(5), Delay(6), Lateral(7), 
 *                           Along(8), Depth(9), Angle(10),  
 *                           Heave(11), Roll(12), Pitch(13),
 *                           Gates(14), Noise(15), EchoLength(16), 
 *                           Hits(17), HeaveReceive(18), Azimuth(19),
 *                           MBsystemNavigation(99)
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
 *   6. The raw XSE format provides asynchronous navigation only; navigation 
 *      is not included in the multibeam or sidescan pings.
 *      MB-System adds MBsystemNavigation(99) groups to the multibeam
 *      frames so that processed navigation can be directly associated
 *      with each survey ping..
 *   7. The comment records are supported by MB-System only and are
 *      not part of the L3 Communications XSE format specification.
 *      The comment frame encloses a single general comment group (id=1).
 *          -------------------------------------------------------
 *          Item     Bytes   Format   Value   Units   Description
 *          -------------------------------------------------------
 *          Start      4     ulong    $HSF            Frame start
 *          Byte Count 4     ulong            bytes   Between byte count
 *                                                      and frame end
 *          Id         4     ulong    99              Frame id - see below
 *          Source     4     ulong    0               Sensor id
 *          Seconds    4     ulong            seconds Seconds since
 *                                                      1/1/1901 00:00:00
 *          Microsec   4     ulong            usec    Microseconds
 *          ...        ...   ...      ...     ...     Frame specific groups
 *          End        4     ulong    #HSF            Frame end
 *          -------------------------------------------------------
 *      The only comment group is 1:
 *          -------------------------------------------------------
 *          Item     Bytes   Format   Value   Units   Description
 *          -------------------------------------------------------
 *          Start      4     ulong    $HSG            Group start
 *          Byte Count 4     ulong            bytes   Between byte count
 *                                                      and group end
 *          Id         4     ulong    1               Group id - see below
 *          N          4     long             bytes   Length of null terminated
 *                                                      comment string, padded 
 *                                                      to a multiple of 4
 *          Comment    N     char             chars   Comment string
 *          End        4     ulong    #HSG            Group end
 *          -------------------------------------------------------
 *
 */

/* maximum number of beams and pixels */
#define	MBSYS_XSE_MAXBEAMS		151
#define	MBSYS_XSE_MAXPIXELS		4096
#define	MBSYS_XSE_MAXSAMPLES		8192
#define	MBSYS_XSE_MAXSVP		200
#define MBSYS_XSE_MAXDRAFT		200
#define	MBSYS_XSE_COMMENT_LENGTH	200
#define	MBSYS_XSE_DESCRIPTION_LENGTH	64
#define	MBSYS_XSE_TIME_OFFSET		2177452800.0
#define	MBSYS_XSE_BUFFER_SIZE		32000
#define	MBSYS_XSE_MAX_SIZE		200
#define	MBSYS_XSE_MAX_SENSORS		16
#define	MBSYS_XSE_MAX_TRANSDUCERS	512

/* frame and group id's */
#define MBSYS_XSE_NONE_FRAME			0

#define MBSYS_XSE_NAV_FRAME			1
#define MBSYS_XSE_NAV_GROUP_GEN			1
#define MBSYS_XSE_NAV_GROUP_POS			2
#define MBSYS_XSE_NAV_GROUP_ACCURACY		3
#define MBSYS_XSE_NAV_GROUP_MOTIONGT		4
#define MBSYS_XSE_NAV_GROUP_MOTIONTW		5
#define MBSYS_XSE_NAV_GROUP_TRACK		6
#define MBSYS_XSE_NAV_GROUP_HRP			7
#define MBSYS_XSE_NAV_GROUP_HEAVE		8
#define MBSYS_XSE_NAV_GROUP_ROLL		9
#define MBSYS_XSE_NAV_GROUP_PITCH		10
#define MBSYS_XSE_NAV_GROUP_HEADING		11
#define MBSYS_XSE_NAV_GROUP_LOG			12
#define MBSYS_XSE_NAV_GROUP_GPS			13

#define MBSYS_XSE_SVP_FRAME			2
#define MBSYS_XSE_SVP_GROUP_GEN			1
#define MBSYS_XSE_SVP_GROUP_DEPTH		2
#define MBSYS_XSE_SVP_GROUP_VELOCITY		3
#define MBSYS_XSE_SVP_GROUP_CONDUCTIVITY	4
#define MBSYS_XSE_SVP_GROUP_SALINITY		5
#define MBSYS_XSE_SVP_GROUP_TEMP		6
#define MBSYS_XSE_SVP_GROUP_PRESSURE		7
#define MBSYS_XSE_SVP_GROUP_SSV			8
#define MBSYS_XSE_SVP_GROUP_POS			9

#define MBSYS_XSE_TID_FRAME			3
#define MBSYS_XSE_TID_GROUP_GEN			1
#define MBSYS_XSE_TID_GROUP_POS			2
#define MBSYS_XSE_TID_GROUP_TIME		3
#define MBSYS_XSE_TID_GROUP_TIDE		4

#define MBSYS_XSE_SHP_FRAME			4
#define MBSYS_XSE_SHP_GROUP_GEN			1
#define MBSYS_XSE_SHP_GROUP_TIME		2
#define MBSYS_XSE_SHP_GROUP_DRAFT		3
#define MBSYS_XSE_SHP_GROUP_SENSORS		4
#define MBSYS_XSE_SHP_GROUP_MOTION		5
#define MBSYS_XSE_SHP_GROUP_GEOMETRY		6
#define MBSYS_XSE_SHP_GROUP_DESCRIPTION		7
#define MBSYS_XSE_SHP_GROUP_PARAMETER		8
#define MBSYS_XSE_SHP_GROUP_NAVIGATIONANDMOTION	9
#define MBSYS_XSE_SHP_GROUP_TRANSDUCER		10
#define MBSYS_XSE_SHP_GROUP_TRANSDUCEREXTENDED	11

#define MBSYS_XSE_SSN_FRAME			5
#define MBSYS_XSE_SSN_GROUP_GEN			1
#define MBSYS_XSE_SSN_GROUP_AMPVSTT		2
#define MBSYS_XSE_SSN_GROUP_PHASEVSTT		3
#define MBSYS_XSE_SSN_GROUP_AMPVSLAT		4
#define MBSYS_XSE_SSN_GROUP_PHASEVSLAT		5
#define MBSYS_XSE_SSN_GROUP_SIGNAL		6
#define MBSYS_XSE_SSN_GROUP_PINGTYPE		7
#define MBSYS_XSE_SSN_GROUP_COMPLEXSIGNAL	8
#define MBSYS_XSE_SSN_GROUP_WEIGHTING		9

#define MBSYS_XSE_MBM_FRAME			6
#define MBSYS_XSE_MBM_GROUP_GEN			1
#define MBSYS_XSE_MBM_GROUP_BEAM		2
#define MBSYS_XSE_MBM_GROUP_TT			3
#define MBSYS_XSE_MBM_GROUP_QUALITY		4
#define MBSYS_XSE_MBM_GROUP_AMP			5
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
#define MBSYS_XSE_MBM_GROUP_HEAVERECEIVE	18
#define MBSYS_XSE_MBM_GROUP_AZIMUTH		19
#define MBSYS_XSE_MBM_GROUP_MBSYSTEMNAV		99

#define MBSYS_XSE_SNG_FRAME			7
#define MBSYS_XSE_CNT_FRAME			8
#define MBSYS_XSE_BTH_FRAME			9
#define MBSYS_XSE_PRD_FRAME			10
#define MBSYS_XSE_NTV_FRAME			11
#define MBSYS_XSE_GEO_FRAME			12

#define MBSYS_XSE_SBM_FRAME			13
#define MBSYS_XSE_SBM_GROUP_PROPERTIES		1
#define MBSYS_XSE_SBM_GROUP_HRP			2
#define MBSYS_XSE_SBM_GROUP_SETUP		3
#define MBSYS_XSE_SBM_GROUP_MRU			4
#define MBSYS_XSE_SBM_GROUP_SETTINGS		5
#define MBSYS_XSE_SBM_GROUP_BEAMS		6
#define MBSYS_XSE_SBM_GROUP_GATES		7
#define MBSYS_XSE_SBM_GROUP_RAW			8
#define MBSYS_XSE_SBM_GROUP_CENTER		9
#define MBSYS_XSE_SBM_GROUP_MESSAGE		21

#define MBSYS_XSE_MSG_FRAME			14
#define MBSYS_XSE_ATT_FRAME			15

#define MBSYS_XSE_COM_FRAME			99
#define MBSYS_XSE_COM_GROUP_GEN			1

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
	double      heavereceive;
	double      azimuth;
	};

struct mbsys_xse_struct
	{
	/* type of data record */
	int	kind;			/* Survey, nav, Comment */
	
	/* parameter (ship frames) */
	int	par_source;		/* sensor id */
	unsigned int	par_sec;	/* sec since 1/1/1901 00:00 */
	unsigned int	par_usec;	/* microseconds */
	unsigned int    par_ship_name_length;	/* length of ship name, chars */
	char    par_ship_name[MBSYS_XSE_DESCRIPTION_LENGTH]; /* Name of Vessel */
	double  par_ship_length;	/* vessel total length, meters */
	double  par_ship_beam;		/* vessel total width, meters */
	double  par_ship_draft;		/* vessel maximum draft, meters */
	double  par_ship_height;	/* vessel maximum height, meters */
	double  par_ship_displacement;	/* vessel maximum displacement, cubic meters */
	double  par_ship_weight;	/* vessel maximum weight, kg */
	
	int	par_ship_nsensor;					/* number of sensors */
	int	par_ship_sensor_id[MBSYS_XSE_MAX_SENSORS];		/* sensor id array */
	int	par_ship_sensor_type[MBSYS_XSE_MAX_SENSORS];		/* sensor type array 
										1000 : SeaBeam 1000
										2000 : SeaBeam 2100
										2001 : SeaBeam 2100 V-shaped
										3000 : SeaBeam 3000 
										4000 : single beam
										8000 : Edgetech sidescan
										9000 : 
										9001 : SSV */
	int	par_ship_sensor_frequency[MBSYS_XSE_MAX_SENSORS];	/* sensor frequency array (kHz) */
	
	int	par_parameter;		/* boolean flag for parameter group */
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
	
	int	par_navigationandmotion;/* boolean flag for navigationandmotion group */
	double	par_nam_roll_bias;	/* roll bias, radians */
	double	par_nam_pitch_bias;	/* pitch bias, radians */
	double	par_nam_heave_bias;	/* heave bias, meters */
	double	par_nam_heading_bias;	/* heading/gyro bias, radians */
	double	par_nam_time_delay;	/* nav time lag, seconds */
	double	par_nam_nav_x;		/* navigation antenna x position, meters */
	double	par_nam_nav_y;		/* navigation antenna y position, meters */
	double	par_nam_nav_z;		/* navigation antenna z position, meters */
	double	par_nam_hrp_x;		/* motion sensor x position, meters */
	double	par_nam_hrp_y;		/* motion sensor y position, meters */
	double	par_nam_hrp_z;		/* motion sensor z position, meters */

	int	par_xdr_num_transducer; /* number of transducers */
	int	par_xdr_sensorid[MBSYS_XSE_MAX_TRANSDUCERS]; 		/* sensor ids */
	char 	par_xdr_transducer[MBSYS_XSE_MAX_TRANSDUCERS]; 		/* transducer type:
										0: hydrophone
										1: projector
										2: transducer */
	unsigned int par_xdr_frequency[MBSYS_XSE_MAX_TRANSDUCERS]; 	/* frequency (Hz) */
	char 	par_xdr_side[MBSYS_XSE_MAX_TRANSDUCERS]; 		/* transducer side:
										0: undefined
										1: port
										2: starboard
										3: midship
										4: system defined */
	double	par_xdr_mountingroll[MBSYS_XSE_MAX_TRANSDUCERS];	/* array mounting angle roll (radians) */
	double	par_xdr_mountingpitch[MBSYS_XSE_MAX_TRANSDUCERS];	/* array mounting angle roll (radians) */
	double	par_xdr_mountingazimuth[MBSYS_XSE_MAX_TRANSDUCERS];	/* array mounting angle roll (radians) */
	double	par_xdr_mountingdistance[MBSYS_XSE_MAX_TRANSDUCERS];	/* horizontal distance between
										innermost elements of the
										transducer arrays to the
										ship center line in a
										V-shaped configuration (m) */
	double	par_xdr_x[MBSYS_XSE_MAX_TRANSDUCERS];	/* transducer center across track offset (m) */
	double	par_xdr_y[MBSYS_XSE_MAX_TRANSDUCERS];	/* transducer center along track offset (m) */
	double	par_xdr_z[MBSYS_XSE_MAX_TRANSDUCERS];	/* transducer center vertical offset (m) */
	double	par_xdr_roll[MBSYS_XSE_MAX_TRANSDUCERS];	/* beamforming roll bias (radians - port up positive) */
	double	par_xdr_pitch[MBSYS_XSE_MAX_TRANSDUCERS];	/* beamforming pitch bias (radians - bow up positive) */
	double	par_xdr_azimuth[MBSYS_XSE_MAX_TRANSDUCERS];	/* beamforming azimuth bias (radians 
									- projector axis clockwise with
									respect to compass positive) */
	int	par_xdx_num_transducer; 			/* number of transducers */
	char 	par_xdx_roll[MBSYS_XSE_MAX_TRANSDUCERS]; 	/* mounting mode roll (0: auto, 1: manual) */
	char	par_xdx_pitch[MBSYS_XSE_MAX_TRANSDUCERS]; 	/* mounting mode pitch (0: auto, 1: manual) */
	char	par_xdx_azimuth[MBSYS_XSE_MAX_TRANSDUCERS]; 	/* mounting mode azimuth (0: auto, 1: manual) */

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
	int	nav_group_general;	/* boolean flag */
	int	nav_group_position;	/* boolean flag */
	int	nav_group_accuracy;	/* boolean flag */
	int	nav_group_motiongt;	/* boolean flag */
	int	nav_group_motiontw;	/* boolean flag */
	int	nav_group_track;	/* boolean flag */
	int	nav_group_hrp;		/* boolean flag */
	int	nav_group_heave;	/* boolean flag */
	int	nav_group_roll;		/* boolean flag */
	int	nav_group_pitch;	/* boolean flag */
	int	nav_group_heading;	/* boolean flag */
	int	nav_group_log;		/* boolean flag */
	int	nav_group_gps;		/* boolean flag */	
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
	short	nav_acc_quality;	/* GPS quality:
						0: invalid
						1: SPS
						2: SPS differential
						3: PPS
						4. RTK
						5: Float RTK
						6: Estimated
						7: Manual
						8: Simulator */
	char	nav_acc_numsatellites;	/* number of satellites */
	float	nav_acc_horizdilution;	/* horizontal dilution of precision */
	float	nav_acc_diffage;	/* age of differential data (sec since last update) */
	unsigned int nav_acc_diffref;	/* differential reference station */
	double	nav_speed_ground;	/* m/s */
	double	nav_course_ground;	/* radians */
	double	nav_speed_water;	/* m/s */
	double	nav_course_water;	/* radians */
	double	nav_trk_offset_track;	/* offset track (m) */
	double	nav_trk_offset_sol;	/* offset SOL (m) */
	double	nav_trk_offset_eol;	/* offset EOL (m) */
	double	nav_trk_distance_sol;	/* distance SOL (m) */
	double	nav_trk_azimuth_sol;	/* azimuth SOL (radians) */
	double	nav_trk_distance_eol;	/* distance EOL (m) */
	double	nav_trk_azimuth_eol;	/* azimuth EOL (radians) */
	double	nav_hrp_heave;		/* heave (m) */
	double	nav_hrp_roll;		/* roll (radians) */
	double	nav_hrp_pitch;		/* pitch (radians) */
	double	nav_hea_heave;		/* heave (m) */
	double	nav_rol_roll;		/* roll (radians) */
	double	nav_pit_pitch;		/* pitch (radians) */
	double	nav_hdg_heading;	/* heading (radians) */
	double	nav_log_speed;		/* speed (m/s) */
	float	nav_gps_altitude;	/* altitude with respect to geoid */
	float	nav_gps_geoidalseparation;	/* difference between WGS84 ellipsoid and geoid (m)
							(positive means sea level geoid is above
							ellipsoid) */
	
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
	int	mul_group_heavereceive;	/* boolean flag - heavereceive group read */
	int	mul_group_azimuth;	/* boolean flag - azimuth group read */
	int	mul_group_mbsystemnav;	/* boolean flag - mbsystemnav group read */
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
	double	mul_lon;		/* longitude (radians) */
	double	mul_lat;		/* latitude (radians) */
	double	mul_heading;		/* heading (radians) */
	double	mul_speed;		/* speed (m/s) */
	struct mbsys_xse_beam_struct beams[MBSYS_XSE_MAXBEAMS];
	
	/* survey sidescan (sidescan frames) */
	int	sid_frame;		/* boolean flag - sidescan frame read */
	int	sid_group_avt;		/* boolean flag - amp vs time group read */
	int	sid_group_pvt;		/* boolean flag - phase vs time group read */
	int	sid_group_avl;		/* boolean flag - amp vs lateral group read */
	int	sid_group_pvl;		/* boolean flag - phase vs lateral group read */
	int	sid_group_signal;	/* boolean flag - signal group read */
	int	sid_group_ping;		/* boolean flag - ping group read */
	int	sid_group_complex;	/* boolean flag - complex group read */
	int	sid_group_weighting;	/* boolean flag - weighting group read */
	int	sid_source;		/* sensor id */
	unsigned int	sid_sec;	/* sec since 1/1/1901 00:00 */
	unsigned int	sid_usec;	/* microseconds */
	int	sid_ping;		/* ping number */
	float	sid_frequency;		/* transducer frequency (Hz) */
	float	sid_pulse;		/* transmit pulse length (sec) */
	float	sid_power;		/* transmit power (dB) */
	float	sid_bandwidth;		/* receive bandwidth (Hz) */
	float	sid_sample;		/* receive sample interval (sec) */
	int	sid_avt_sampleus;	/* sample interval (usec) */
	int	sid_avt_offset;		/* time offset (usec) */
	int	sid_avt_num_samples;	/* number of samples */
	short	sid_avt_amp[MBSYS_XSE_MAXPIXELS]; /* sidescan amplitude (dB) */
	int	sid_pvt_sampleus;	/* sample interval (usec) */
	int	sid_pvt_offset;		/* time offset (usec) */
	int	sid_pvt_num_samples;	/* number of samples */
	short	sid_pvt_phase[MBSYS_XSE_MAXPIXELS]; /* sidescan phase (radians) */
	int	sid_avl_binsize;	/* bin size (mm) */
	int	sid_avl_offset;		/* lateral offset (mm) */
	int	sid_avl_num_samples;	/* number of samples */
	short	sid_avl_amp[MBSYS_XSE_MAXPIXELS]; /* sidescan amplitude (dB) */
	int	sid_pvl_binsize;	/* bin size (mm) */
	int	sid_pvl_offset;		/* lateral offset (mm) */
	int	sid_pvl_num_samples;	/* number of samples */
	short	sid_pvl_phase[MBSYS_XSE_MAXPIXELS]; /* sidescan phase (radians) */
	int	sid_sig_ping;		/* ping number */
	int	sid_sig_channel;	/* channel number */
	double	sid_sig_offset;		/* start offset */
	double	sid_sig_sample;		/* bin size / sample interval */
	int	sid_sig_num_samples;	/* number of samples */
	short	sid_sig_phase[MBSYS_XSE_MAXPIXELS]; /* sidescan phase in radians */
	unsigned int	sid_png_pulse;	/* pulse type (0=constant, 1=linear sweep) */
	double	sid_png_startfrequency;	/* start frequency (Hz) */
	double	sid_png_endfrequency;	/* end frequency (Hz) */
	double	sid_png_duration;	/* pulse duration (msec) */
	int	sid_png_mancode;	/* manufacturer code (1=Edgetech, 2=Elac) */
	unsigned int	sid_png_pulseid;/* pulse identifier */
	char	sid_png_pulsename[MBSYS_XSE_DESCRIPTION_LENGTH];	/* pulse name */
	int	sid_cmp_ping;		/* ping number */
	int	sid_cmp_channel;	/* channel number */
	double	sid_cmp_offset;		/* start offset (usec) */
	double	sid_cmp_sample;		/* bin size / sample interval (usec) */
	int	sid_cmp_num_samples;	/* number of samples */
	short	sid_cmp_real[MBSYS_XSE_MAXPIXELS]; /* real sidescan signal */
	short	sid_cmp_imaginary[MBSYS_XSE_MAXPIXELS]; /* imaginary sidescan signal */
	short	sid_wgt_factorleft;		/* weighting factor for block floating 
						point expansion  -- 
						defined as 2^(-N) volts for lsb */
	unsigned int sid_wgt_samplesleft;	/* number of left samples */
	short	sid_wgt_factorright;		/* weighting factor for block floating 
						point expansion  -- 
						defined as 2^(-N) volts for lsb */
	unsigned int sid_wgt_samplesright;	/* number of right samples */
	
	
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

