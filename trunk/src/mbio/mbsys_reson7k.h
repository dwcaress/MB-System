/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_reson7k.h	3/3/2004
 *	$Id: mbsys_reson7k.h,v 5.3 2004-07-15 19:25:05 caress Exp $
 *
 *    Copyright (c) 2004 by
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
 * mbsys_reson7k.h defines the MBIO data structures for handling data from 
 * Reson 7k series sonars:
 *      MBF_RESON7K1 : MBIO ID 191 - Reson 7K Series sonar
 *
 * Author:	D. W. Caress
 * Date:	March 3, 2004
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.2  2004/06/18 05:22:33  caress
 * Working on adding support for segy i/o and for Reson 7k format 88.
 *
 * Revision 5.1  2004/05/21 23:44:50  caress
 * Progress supporting Reson 7k data, including support for extracing subbottom profiler data.
 *
 * Revision 5.0  2004/04/27 01:50:16  caress
 * Adding support for Reson 7k sonar data, including segy extensions.
 *
 *
 */
/*
 * Notes on the mbsys_reson7k data structure:
 *   1. This format is defined by the Interface Control Document
 *      for RESON SeaBat 7k format v0.42.
 *   2. Reson 7k series multibeam sonars output bathymetry, per beam
 *      amplitude, and sidescan data.
 *   3. The Reson 6046 datalogger can also log sidescan and subbottom
 *      data from other sonars.
 *   4. The 7k record consists of a data record frame (header and checksum), 
 *      a record  type header, an optional record data field and an optional 
 *	data field for extra  information. The optional data field typically 
 *	holds non-generic sensor specific data.   *
 */
 
/* include mb_define.h */
#ifndef MB_DEFINE_DEF
#include "mb_define.h"
#endif

/*---------------------------------------------------------------*/
/* Record ID definitions */

/* 0 means no record at all */
#define	R7KRECID_None					0

/* 1000-1999 reserved for generic sensor records */
#define R7KRECID_ReferencePoint				1000
#define R7KRECID_UncalibratedSensorOffset		1001
#define R7KRECID_CalibratedSensorOffset			1002
#define R7KRECID_Position				1003
#define R7KRECID_Attitude				1004
#define R7KRECID_Tide					1005
#define R7KRECID_Altitude				1006
#define R7KRECID_MotionOverGround			1007
#define R7KRECID_Depth					1008
#define R7KRECID_SoundVelocityProfile			1009
#define R7KRECID_CTD					1010
#define R7KRECID_Geodesy				1011

/* 2000-2999 reserved for user defined records */
#define R7KRECID_Survey					2000

/* 3000-6999 reserved for other vendor records */
#define R7KRECID_FSDWsidescan				3000
#define R7KRECID_FSDWsidescanLo				0
#define R7KRECID_FSDWsidescanHi				1
#define R7KRECID_FSDWsubbottom				3001
#define R7KRECID_Bluefin				3100
#define R7KRECID_BluefinNav				0
#define R7KRECID_BluefinEnvironmental			1

/* 7000-7999 reserved for SeaBat 7k records */
#define R7KRECID_7kVolatileSonarSettings		7000
#define R7KRECID_7kConfigurationSettings		7001
#define R7KRECID_7kMatchFilter				7002
#define R7KRECID_7kBeamGeometry				7004
#define R7KRECID_7kCalibrationData			7005
#define R7KRECID_7kBathymetricData			7006
#define R7KRECID_7kBackscatterImageData			7007
#define R7KRECID_7kBeamData				7008
#define R7KRECID_7kImageData				7011
#define R7KRECID_7kSystemEvent				7051
#define R7KRECID_7kDataStorageStatus			7052
#define R7KRECID_7kFileHeader				7200
#define R7KRECID_7kTrigger				7300
#define R7KRECID_7kTriggerSequenceSetup			7301
#define R7KRECID_7kTriggerSequenceDone			7302
#define R7KRECID_7kTimeMessage				7400
#define R7KRECID_7kRemoteControl			7500
#define R7KRECID_7kRemoteControlAcknowledge		7501
#define R7KRECID_7kRemoteControlNotAcknowledge		7502
#define R7KRECID_7kRemoteControlSonarSettings		7503
#define R7KRECID_7kRoll					7600
#define R7KRECID_7kPitch				7601
#define R7KRECID_7kSoundVelocity			7610
#define R7KRECID_7kAbsorptionLoss			7611
#define R7KRECID_7kSpreadingLoss			7612

/* 11000-11199 reserved for Payload Controller command records */
#define R7KRECID_7kPayloadControllerCommand		11000
#define R7KRECID_7kPayloadControllerCommandAcknowledge	11001
#define R7KRECID_7kPayloadControllerStatus		11002

/* 11200-11999 reserved for Payload Controller sensor QC records */

/*---------------------------------------------------------------*/
/* Record size definitions */
#define	MBSYS_RESON7K_RECORDHEADER_SIZE				72
#define	MBSYS_RESON7K_RECORDTAIL_SIZE				4

/* 0 means no record at all */
#define	R7KHDRSIZE_None						0

/* 1000-1999 reserved for generic sensor records */
#define R7KHDRSIZE_ReferencePoint				16
#define R7KHDRSIZE_UncalibratedSensorOffset			24
#define R7KHDRSIZE_CalibratedSensorOffset			24
#define R7KHDRSIZE_Position					28
#define R7KHDRSIZE_Attitude					8
#define R7KHDRSIZE_Tide						8
#define R7KHDRSIZE_Altitude					4
#define R7KHDRSIZE_MotionOverGround				8
#define R7KHDRSIZE_Depth					8
#define R7KHDRSIZE_SoundVelocityProfile				24
#define R7KHDRSIZE_CTD						32
#define R7KHDRSIZE_Geodesy					320

/* 2000-2999 reserved for user defined records */
#define R7KHDRSIZE_Survey					0

/* 3000-6999 reserved for other vendor records */
#define R7KHDRSIZE_FSDWsidescan					20
#define R7KHDRSIZE_FSDWsubbottom				20
#define R7KHDRSIZE_BluefinDataFrame				32
#define R7KHDRSIZE_FSDWchannelinfo				64
#define R7KHDRSIZE_FSDWssheader					80
#define R7KHDRSIZE_FSDWsbheader					240

/* 7000-7999 reserved for SeaBat 7k records */
#define R7KHDRSIZE_7kVolatileSonarSettings			120
#define R7KHDRSIZE_7kConfigurationSettings			12
#define R7KHDRSIZE_7kMatchFilter				12
#define R7KHDRSIZE_7kBeamGeometry				12
#define R7KHDRSIZE_7kCalibrationData				10
#define R7KHDRSIZE_7kBathymetricData				14
#define R7KHDRSIZE_7kBackscatterImageData			62
#define R7KHDRSIZE_7kBeamData					28
#define R7KHDRSIZE_7kImageData					12
#define R7KHDRSIZE_7kSystemEvent				12
#define R7KHDRSIZE_7kDataStorageStatus				0
#define R7KHDRSIZE_7kFileHeader					44
#define R7KRDTSIZE_7kFileHeader					288
#define R7KHDRSIZE_7kTrigger					2
#define R7KHDRSIZE_7kTriggerSequenceSetup			2
#define R7KHDRSIZE_7kTriggerSequenceDone			2
#define R7KHDRSIZE_7kTimeMessage				16
#define R7KHDRSIZE_7kRemoteControl				8
#define R7KHDRSIZE_7kRemoteControlAcknowledge			4
#define R7KHDRSIZE_7kRemoteControlNotAcknowledge		8
#define R7KHDRSIZE_7kRemoteControlSonarSettings			84
#define R7KHDRSIZE_7kRoll					4
#define R7KHDRSIZE_7kPitch					4
#define R7KHDRSIZE_7kSoundVelocity				4
#define R7KHDRSIZE_7kAbsorptionLoss				4
#define R7KHDRSIZE_7kSpreadingLoss				4

/* 11000-11199 reserved for Payload Controller command records */
#define R7KHDRSIZE_7kPayloadControllerCommand			16
#define R7KHDRSIZE_7kPayloadControllerCommandAcknowledge	12
#define R7KHDRSIZE_7kPayloadControllerStatus			16

/* 11200-11999 reserved for Payload Controller sensor QC records */

/*---------------------------------------------------------------*/

/* Device identifiers */
#define R7KDEVID_SeaBat7001		7001
#define R7KDEVID_SeaBat7012		7012
#define R7KDEVID_SeaBat7100		7100
#define R7KDEVID_SeaBat7101		7101
#define R7KDEVID_SeaBat7102		7102
#define R7KDEVID_SeaBat7112		7112
#define R7KDEVID_SeaBat7123		7123
#define R7KDEVID_SeaBat7125		7125
#define R7KDEVID_SeaBat7128		7128
#define R7KDEVID_SeaBat7150		7150
#define R7KDEVID_SeaBat7160		7160
#define R7KDEVID_TSSDMS05		10000
#define R7KDEVID_TSS335B		10001
#define R7KDEVID_TSS332B		10002
#define R7KDEVID_SeaBirdSBE37		10010
#define R7KDEVID_Littom200		10020
#define R7KDEVID_EdgetechFSDW		11000
#define R7KDEVID_Bluefin		11100
#define R7KDEVID_SimradRPT319		12000

/*---------------------------------------------------------------*/

/* Edgetech trace data format definitions */
#define	EDGETECH_TRACEFORMAT_ENVELOPE		0 	/* 2 bytes/sample (unsigned) */
#define	EDGETECH_TRACEFORMAT_ANALYTIC		1 	/* 4 bytes/sample (I + Q) */
#define	EDGETECH_TRACEFORMAT_RAW		2 	/* 2 bytes/sample (signed) */
#define	EDGETECH_TRACEFORMAT_REALANALYTIC	3 	/* 2 bytes/sample (signed) */
#define	EDGETECH_TRACEFORMAT_PIXEL		4 	/* 2 bytes/sample (signed) */

/*---------------------------------------------------------------*/

/* Bluefin data frame definitions */
#define	BLUEFIN_MAX_FRAMES			25 	/* Maximum number of Bluefin 
								data frames contained
								in a Bluefin data record */

/*---------------------------------------------------------------*/

/* Structure size definitions */
#define	MBSYS_RESON7K_BUFFER_STARTSIZE	32768
#define	MBSYS_RESON7K_MAX_DEVICE	10
#define MBSYS_RESON7K_MAX_RECEIVERS	1024
#define	MBSYS_RESON7K_MAX_BEAMS		240
#define	MBSYS_RESON7K_MAX_PIXELS	4096

typedef struct s7k_time_struct
{
    unsigned short  Year;			/* Year                 u16 0 - 65535 */
    unsigned short  Day;			/* Day                  u16 1 - 366 */

    float           Seconds;			/* Seconds              f32 0.000000 - 59.000000 */

    mb_u_char       Hours;			/* Hours                u8  0 - 23 */
    mb_u_char       Minutes;		/* Minutes              u8  0 - 59 */
}
s7k_time;

typedef struct s7k_header_struct
{
    unsigned short  Version;                    /* Version              u16 Version of this frame (e.g.: 1, 2 etc.) */
    unsigned short  Offset;                     /* Offset               u16 Offset in bytes from the start of the sync 
    												pattern to the start of the DATA SECTION. 
												This allows for expansion of the header 
												whilst maintaining backward compatibility. */

    unsigned int   SyncPattern;                /* Sync pattern         u32 0x0000FFFF */
    unsigned int   Size;			/* Size                 u32 Size in bytes of this record from the start 
    												of the version field to the end of the 
												Checksum. It includes the embedded data size. */
    unsigned int   OffsetToOptionalData;	/* Data offset          u32 Offset in bytes to optional data field from 
    												start of record. Zero implies no optional data. */
    unsigned int   OptionalDataIdentifier;	/* Data idenfitifer     u32 Identifier for optional data field. Zero for 
    	`											no optional field. This identifier is 
												described with each record type. */
    s7k_time 	   s7kTime;				/* 7KTIME               u8*10   UTC.*/
    unsigned short Reserved;			/* Reserved  */
    unsigned int   RecordType;			/* Record type          u32 Unique identifier of indicating the type of 
    												data embedded in this record. */
    unsigned int   DeviceId;			/* Device identifier    u32 Identifier of the device that this data pertains. */
    unsigned int   SubsystemId;		/* Subsystem identifier u32 Identifier for the device subsystem */
    unsigned int   DataSetNumber;		/* Data set             u32 Data set number. */
    unsigned int   RecordNumber;		/* Record count         u32 Sequential record counter. */

    char           PreviousRecord[8];		/* Previous record      i64 Pointer to the previous record of the same type 
    												(in bytes from start of file). This is an 
												optional field for files and shall be -1 
												if not used. */
    char           NextRecord[8];		/* Next record          i64 Pointer to the next record of the same type in 
    												bytes from start of file. This is an optional 
												field for files and shall be -1 if not used. */

    unsigned short Flags;			/* Flags                u16 BIT FIELD: Bit 1 - Valid Checksum */
    unsigned short Reserved2;			/* Reserved  */
                                                                
    /* Following this header is:                                     
    	DATA SECTION							xx  Dynamic Record type specific data.
    	Checksum							u32 Sum of bytes in data section 
										(optional, depends on bit 1 of Flags field).
										Note: the checksum field  should be computed 
										as a 64 bit unsigned integer  with the least 
										significant 32 bits used to populate  this field 
										thus ensuring a valid checksum and  avoiding 
										an explicit overflow.  */
}
s7k_header;

/* Reference point information (record 1000) */
/*  Note: these offsets should be zero for submersible vehicles */
typedef struct s7kr_reference_struct
{	
	s7k_header	header;
	float		offset_x;		/* Vehicle's X reference point ot center of gravity (meters) */
	float		offset_y;		/* Vehicle's Y reference point ot center of gravity (meters) */
	float		offset_z;		/* Vehicle's Z reference point ot center of gravity (meters) */
	float		water_z;		/* Vehicle's water level to center of gravity (meters) */
}
s7kr_reference;
	
/* Sensor uncalibrated offset position information (record 1001) */
typedef struct s7kr_sensoruncal_struct
{	
	s7k_header	header;
	float		offset_x;		/* Sensor X offset from vehicle reference point (meters) */
	float		offset_y;		/* Sensor Y offset from vehicle reference point (meters) */
	float		offset_z;		/* Sensor Z offset from vehicle reference point (meters) */
	float		offset_roll;		/* Sensor roll offset (radians - port up is positive) */
	float		offset_pitch;		/* Sensor pitch offset (radians - bow up is positive) */
	float		offset_yaw;		/* Sensor yaw offset (radians - bow right is positive) */
}
s7kr_sensoruncal;
	
/* Sensor calibrated offset position information (record 1002) */
typedef struct s7kr_sensorcal_struct
{	
	s7k_header	header;
	float		offset_x;		/* Sensor X offset from vehicle reference point (meters) */
	float		offset_y;		/* Sensor Y offset from vehicle reference point (meters) */
	float		offset_z;		/* Sensor Z offset from vehicle reference point (meters) */
	float		offset_roll;		/* Sensor roll offset (radians - port up is positive) */
	float		offset_pitch;		/* Sensor pitch offset (radians - bow up is positive) */
	float		offset_yaw;		/* Sensor yaw offset (radians - bow right is positive) */
}
s7kr_sensorcal;
	
/* Position (record 1003) */
typedef struct s7kr_position_struct
{	
	s7k_header	header;
	int		datum;			/* 0=WGS84; others not yet defined */
	double		latitude;		/* Latitude (radians) */
	double		longitude;		/* Longitude (radians) */
	double		height;			/* Height relative to datum (meters) */
}
s7kr_position;
	
/* Attitude (record 1004) */
typedef struct s7kr_attitude_struct
{	
	s7k_header	header;
	mb_u_char	bitfield;		/* Boolean bitmask indicating which attitude fields are in data
								0: pitch (radians - float)
								1: roll (radians - float)
								2: heading (radians - float)
								3: heave (meters - float)
								4-7: reserved */
	mb_u_char	reserved;		/* reserved field */
	unsigned short	n;			/* number of fields */
	float		frequency;		/* sample rate (samples/second) */
	int		nalloc;			/* number of samples allocated */
	float		*pitch;
	float		*roll;
	float		*heading;
	float		*heave;
}
s7kr_attitude;
	
/* Tide (record 1005) */
typedef struct s7kr_tide_struct
{	
	s7k_header	header;
	float		tide;			/* height correction above mean sea level (meters) */
	unsigned short	source;			/* tide data source: 0 - table; 1- gauge */
	unsigned short	reserved;		/* reserved field */
}
s7kr_tide;
	
/* Altitude (record 1006) */
typedef struct s7kr_altitude_struct
{	
	s7k_header	header;
	float		altitude;		/* altitude above seafloor (meters) */
}
s7kr_altitude;
	
/* Motion over ground (record 1007) */
typedef struct s7kr_motion_struct
{	
	s7k_header	header;
	mb_u_char	bitfield;		/* Boolean bitmask indicating which motion over ground fields are in data
							0: X,Y,Z speed (m/s - 3 X float)
							1: X,Y,Z acceleration (m/s**2 - 3 X float)
							2-7: reserved */
	mb_u_char	reserved;		/* reserved field */
	unsigned short	n;			/* number of fields */
	float		frequency;		/* sample rate (samples/second) */
	int		nalloc;			/* number of samples allocated */
	float		*x;
	float		*y;
	float		*z;
	float		*xa;
	float		*ya;
	float		*za;
}
s7kr_motion;
	
/* Depth (record 1008) */
typedef struct s7kr_depth_struct
{	
	s7k_header	header;
	mb_u_char	descriptor;		/* Depth descriptor:
							0 = depth to sensor
							1 = water depth */
	mb_u_char	correction;		/* Correction flag: 
							0 = raw depth as measured 
							1 = corrected depth (relative to mean sea level) */
	unsigned short	reserved;		/* reserved field */
	float		depth;			/* depth (meters) */
}
s7kr_depth;
	
/* Sound velocity profile (record 1009) */
typedef struct s7kr_svp_struct
{	
	s7k_header	header;
	mb_u_char	position_flag;		/* Position validity flag:
							0: invalid position fields
							1: valid position field */
	mb_u_char	reserved1;		/* reserved field */
	unsigned short	reserved2;		/* reserved field */
	double		latitude;		/* Latitude (radians) */
	double		longitude;		/* Longitude (radians) */
	unsigned int	n;			/* number of fields */
	int		nalloc;			/* number of samples allocated */
	float		*depth;			/* depth (meters) */
	float		*sound_velocity;	/* sound velocity (meters/second) */
}
s7kr_svp;
	
/* CTD (record 1010) */
typedef struct s7kr_ctd_struct
{	
	s7k_header	header;
	mb_u_char	velocity_source_flag;	/* Velocity source flag:
							0: not computed
							1: CTD
							2: user computed */
	mb_u_char	velocity_algorithm;	/* Velocity algorithm flag:
							0: not computed
							1: Checn Millero
							2: Delgrosso */
	mb_u_char	conductivity_flag;	/* Conductivity flag:
							0: conductivity
							1: salinity */
	mb_u_char	pressure_flag;		/* Pressure flag:
							0: pressure
							1: depth */
	mb_u_char	position_flag;		/* Position validity flag:
							0: invalid position fields
							1: valid position field */
	mb_u_char	reserved1;		/* Reserved field */
	unsigned short	reserved2;		/* Reserved field */
	double		latitude;		/* Latitude (radians) */
	double		longitude;		/* Longitude (radians) */
	float		frequency;		/* Sample rate */
	unsigned int	n;			/* Number of fields */
	int		nalloc;			/* Number of samples allocated */
	float		*conductivity_salinity;	/* Conductivity (s/m) or salinity (ppt) */
	float		*temperature;		/* Temperature (degrees celcius) */
	float		*pressure_depth;	/* Pressure (pascals) or depth (meters) */
	float		*sound_velocity;	/* Sound velocity (meters/second) */
}
s7kr_ctd;
	
/* Geodesy (record 1011) */
typedef struct s7kr_geodesy_struct
{	
	s7k_header	header;
	char		spheroid[32];		/* Text description of the spheroid name (e.g. "WGS84") */
	double		semimajoraxis;		/* Semi-major axis in meters (e.g. 6378137.0 for WGS84) */
	double		flattening;		/* Inverse flattening in meters (e.g. 298.257223563 for WGS84) */
	char		reserved1[16];		/* Reserved space */
	char		datum[32];		/* Datum name (e.g. "WGS84") */
	unsigned int	calculation_method;	/* Data calculation method:
							0 - Molodensky
							1 - Bursa / Wolfe
							2 - DMA MRE
							3 - NADCON
							4 - HPGN
							5 - Canadian National Transformation V2 */
	unsigned int	number_parameters;	/* Seven parameter transformation supported */
	double		dx;			/* X shift (meters) */
	double		dy;			/* Y shift (meters) */
	double		dz;			/* Z shift (meters) */
	double		rx;			/* X rotation (degrees) */
	double		ry;			/* Y rotation (degrees) */
	double		rz;			/* Z rotation (degrees) */
	double		scale;			/* Scale */
	char		reserved2[35];		/* Reserved for implementation of 9 parameter transformation */
	char		grid_name[32];		/* Name of grid system in use: (e.g. "UTM") */
	mb_u_char	distance_units;		/* Grid distance units:
							0 - meters
							1 - feet
							2 - yards
							3 - US survey feet
							4 - km 
							5 - miles
							6 - US survey miles
							7 - nautical miles
							8 - chains
							9 - links */
	mb_u_char	angular_units;		/* Grid angulat units:
							0 - radians
							1 - degrees
							2 - degrees, minutes, seconds
							3 - gradians
							4 - arc-seconds */
	double		latitude_origin;	/* Latitude of origin */
	double		central_meriidan;	/* Central meridian */
	double		false_easting;		/* False easting (meters) */
	double		false_northing;		/* False northing */
	double		central_scale_factor;	/* Central scale factor */
	int		custum_identifier;	/* Identifier for optional field definition in 7k record. 
							Used to define projection specific parameters.
							-2 - custom
							-1 - not used */
	char		reserved3[50];		/* Reserved field */
}
s7kr_geodesy;

/* MB-System 7k survey (record 2001) */
typedef struct s7kr_survey_struct
{
	s7k_header	header;
	unsigned short	serial_number;		/* Sonar serial number */
	unsigned short	ping_number;		/* Sequential number */
	unsigned short	number_beams;		/* Number of multibeam beams */
	unsigned short	number_pixels;		/* Number of multibeam pixels */
	unsigned short	number_sslow_pixels;	/* Number of low frequency sidescan pixels */
	unsigned short	number_sshi_pixels;	/* Number of high frequency sidescan pixels */
	double		longitude;		/* Sonar longitude at ping (degrees) */
	double		latitude;		/* Sonar latitude at ping (degrees) */
	float		sonar_depth;		/* Sonar depth at ping (meters) */
	float		sonar_altitude;		/* Sonar altitude at ping (meters) */
	float		heading;		/* Heading at ping (degrees) */
	float		speed;			/* Sonar speed at ping (m/sec) */
	float		beamwidthx;		/* Sonar nadir acrosstrack beamwidth */
	float		beamwidthy;		/* Sonar nadir alongtrack beamwidth */
	float		beam_depth[MBSYS_RESON7K_MAX_BEAMS];		/* Beam depth below sonar */
	float		beam_acrosstrack[MBSYS_RESON7K_MAX_BEAMS];		/* Beam acrosstrack distance (meters) */
	float		beam_alongtrack[MBSYS_RESON7K_MAX_BEAMS];		/* Beam alongtrack distance (meters) */
	mb_u_char	beam_flag[MBSYS_RESON7K_MAX_BEAMS];		/* Beam flags (MB-System conventions) */
	float		beam_amplitude[MBSYS_RESON7K_MAX_BEAMS];		/* Signal strength (dB/uPa) */
	float		ss[MBSYS_RESON7K_MAX_PIXELS];			/* Low frequency sidescan sonar */
	float		ss_acrosstrack[MBSYS_RESON7K_MAX_PIXELS];	/* Low frequency sidescan acrosstrack distance (meters) */
	float		ss_alongtrack[MBSYS_RESON7K_MAX_PIXELS];	/* Low frequency sidescan alongtrack distance (meters) */
	float		sslow[MBSYS_RESON7K_MAX_PIXELS];			/* Low frequency sidescan sonar */
	float		sslow_acrosstrack[MBSYS_RESON7K_MAX_PIXELS];	/* Low frequency sidescan acrosstrack distance (meters) */
	float		sslow_alongtrack[MBSYS_RESON7K_MAX_PIXELS];	/* Low frequency sidescan alongtrack distance (meters) */
	float		sshi[MBSYS_RESON7K_MAX_PIXELS];			/* High frequency sidescan sonar */
	float		sshi_acrosstrack[MBSYS_RESON7K_MAX_PIXELS];	/* High frequency sidescan acrosstrack distance (meters) */
	float		sshi_alongtrack[MBSYS_RESON7K_MAX_PIXELS];		/* High frequency sidescan alongtrack distance (meters) */
}
s7kr_survey;

/* Edgetech sidescan or subbottom channel header data */
typedef struct s7k_fsdwchannel_struct
{
	mb_u_char	number;			/* Channel number (0 to number channels - 1) */
	mb_u_char	type;			/* Channel type:
								0 - port
								1 - starboard */
	mb_u_char	data_type;		/* Channel data type:
								0 - slant range
								1 - ground range */
	mb_u_char	polarity;		/* Channel polarity
								0 - bipolar
								1 - unipolar */
	mb_u_char	bytespersample;		/* Bytes per sample of the imagery */
	char		reserved1[3];		/* Reserved */
	unsigned int	number_samples;		/* Number of samples in this channel */
	unsigned int	start_time;		/* Start of first sample in microseconds relative 
								to the ping time stamp */
	unsigned int	sample_interval;		/* Data sample interval in microseconds */
	float		range;			/* Slant range or ground range in meters and 
								depends on the data type field above */
	float		voltage;			/* Analogue maximum amplitude. Should be -1 if not used */
	char		name[16];		/* Channel name */
	char		reserved2[20];		/* Reserved */
	int		data_alloc;		/* number of bytes allocated for data array */
	char		*data;
}
s7k_fsdwchannel;

/* Edgetech sidescan header data */
typedef struct s7k_fsdwssheader_struct
{
unsigned short subsystem;		/*   0 -   1 : Subsystem (0 .. n) */
unsigned short channelNum;		/*   2 -   3 : Channel Number (0 .. n) */
unsigned int pingNum;			/*   4 -   7 : Ping number (increments with ping) */
unsigned short packetNum;		/*   8 -   9 : Packet number (1..n) Each ping starts with packet 1 */
unsigned short trigSource;		/*  10 -  11 : TriggerSource (0 = internal, 1 = external) */
unsigned int samples;			/*  12 -  15 : Samples in this packet */   
unsigned int sampleInterval;		/*  16 -  19 : Sample interval in ns of stored data */
unsigned int startDepth;		/*  20 -  23 : starting Depth (window offset) in samples */
short weightingFactor;			/*  24 -  25 : -- defined as 2 -N volts for lsb */
unsigned short ADCGain;			/*  26 -  27 : Gain factor of ADC */
unsigned short ADCMax;			/*  28 -  29 : Maximum absolute value for ADC samples for this packet */
unsigned short rangeSetting;		/*  30 -  31 : Range Setting (meters X 10) */
unsigned short pulseID;			/*  32 -  33 : Unique pulse identifier */
unsigned short markNumber;		/*  34 -  35 : Mark Number (0 = no mark) */
unsigned short dataFormat;		/*  36 -  37 : Data format */
					/*   0 = 1 short  per sample  - envelope data */
					/*   1 = 2 shorts per sample  - stored as real(1), imag(1), */
					/*   2 = 1 short  per sample  - before matched filter (raw) */
					/*   3 = 1 short  per sample  - real part analytic signal */
					/*   NOTE: For type = 1, the total number of bytes of data to follow is */
					/*   4 * samples.  For all other types the total bytes is 2 * samples */
unsigned short reserved;		/*  38 -  39 : Reserved field to round up to a 32-bit word boundary */
/* -------------------------------------------------------------------- */
/* computer date / time data acquired                                   */
/* -------------------------------------------------------------------- */
unsigned int millisecondsToday;		/*  40 -  43 : Millieconds today */
short year;				/*  44 -  45 : Year */
unsigned short day;			/*  46 -  47 : Day of year (1 - 366) */
unsigned short hour;			/*  48 -  49 : Hour of day (0 - 23) */
unsigned short minute;			/*  50 -  51 : Minute (0 - 59) */
unsigned short second;			/*  52 -  53 : Second (0 - 59) */
/* -------------------------------------------------------------------- */
/* Auxillary sensor information */
/* -------------------------------------------------------------------- */    
short heading;				/*  54 -  55 : Compass heading (minutes) */
short pitch;				/*  56 -  57 : Pitch (minutes) */
short roll;				/*  58 -  59 : Roll (minutes) */
short heave;				/*  60 -  61 : Heave (centimeters) */
short yaw;				/*  62 -  63 : Yaw (minutes) */
unsigned int depth;			/*  64 -  67 : Vehicle depth (centimeters) */
short temperature;			/*  68 -  69 : Temperature (degrees Celsius X 10) */
char reserved2[10];			/*  70 -  79 : Reserved for future use */
}
s7k_fsdwssheader;

/* Edgetech segy header data */
typedef struct s7k_fsdwsegyheader_struct
{
int sequenceNumber; 			/* 0-3 : Trace Sequence Number (always 0) ** */
unsigned int startDepth;          	/* 4-7 : Starting depth (window offset) in samples. */
unsigned int pingNum;              	/* 8-11: Ping number (increments with ping) ** */
unsigned int channelNum;           	/* 12-15 : Channel Number (0 .. n) ** */
short unused1[6];          		/* 16-27 */

short traceIDCode;         		/* 28-29 : ID Code (always 1 => seismic data) ** */

short unused2[2];     			/* 30-33 */
short dataFormat;			/* 34-35 : DataFormatType */
					/*   0 = 1 short  per sample  - envelope data */
					/*   1 = 2 shorts per sample, - stored as real(1), imag(1), */
					/*   2 = 1 short  per sample  - before matched filter */
					/*   3 = 1 short  per sample  - real part analytic signal */
					/*   4 = 1 short  per sample  - pixel data / ceros data */
short NMEAantennaeR;			/* 36-37 : Distance from towfish to antennae in cm */
short NMEAantennaeO;			/* 38-39 : Distance to antennae starboard direction in cm */
char RS232[32];				/* 40-71 : Reserved for RS232 data - TBD */
/* -------------------------------------------------------------------- */
/* Navigation data :                                                    */
/* If the coorUnits are seconds(2), the x values represent longitude    */
/* and the y values represent latitude.  A positive value designates    */
/* the number of seconds east of Greenwich Meridian or north of the     */
/* equator.                                                             */
/* -------------------------------------------------------------------- */
int sourceCoordX;			/* 72-75 : Meters or Seconds of Arc */
int sourceCoordY;			/* 76-79 : Meters or Seconds of Arc */
int groupCoordX;			/* 80-83 : mm or 10000 * (Minutes of Arc) */
int groupCoordY;			/* 84-87 : mm or 10000 * (Minutes of Arc) */
short coordUnits;			/* 88-89 : Units of coordinates - 1->length (x /y), 2->seconds of arc */
char annotation[24];			/* 90-113 : Annotation string */
unsigned short samples;			/* 114-115 : Samples in this packet ** */
					/* Note:  Large sample sizes require multiple packets. */
unsigned int sampleInterval;		/* 116-119 : Sample interval in ns of stored data ** */
unsigned short ADCGain;			/* 120-121 : Gain factor of ADC */
short pulsePower;			/* 122-123 : user pulse power setting (0 - 100) percent */
short correlated;			/* 124-125 : correlated data 1 - No, 2 - Yes */
unsigned short startFreq;		/* 126-127 : Starting frequency in 10 * Hz */
unsigned short endFreq;			/* 128-129 : Ending frequency in 10 * Hz */
unsigned short sweepLength;		/* 130-131 : Sweep length in ms */
short unused7[4];			/* 132-139 */
unsigned short aliasFreq;		/* 140-141 : alias Frequency (sample frequency / 2) */
unsigned short pulseID;			/* 142-143 : Unique pulse identifier */
short unused8[6];			/* 144-155 */
short year;				/* 156-157 : Year data recorded (CPU time) */
short day;				/* 158-159 : day */
short hour;				/* 160-161 : hour */
short minute;				/* 162-163 : minute */
short second;				/* 164-165 : second */
short timeBasis;			/* 166-167 : Always 3 (other not specified by standard) */
short weightingFactor;			/* 168-169 :  weighting factor for block floating point expansion */
					/*            -- defined as 2 -N volts for lsb */
short unused9;				/* 170-171 : */
/* -------------------------------------------------------------------- */
/* From pitch/roll/temp/heading sensor */
/* -------------------------------------------------------------------- */
short heading;				/* 172-173 : Compass heading (100 * degrees) -180.00 to 180.00 degrees */
short pitch;				/* 174-175 : Pitch */
short roll;				/* 176-177 : Roll */
short temperature;			/* 178-179 : Temperature (10 * degrees C) */
/* -------------------------------------------------------------------- */
/* User defined area from 180-239                                       */
/* -------------------------------------------------------------------- */
short heaveCompensation;		/* 180-181 : Heave compensation offset (samples) */
short trigSource;   			/* 182-183 : TriggerSource (0 = internal, 1 = external) */    
unsigned short markNumber;		/* 184-185 : Mark Number (0 = no mark) */
short NMEAHour;				/* 186-187 : Hour */
short NMEAMinutes;			/* 188-189 : Minutes */
short NMEASeconds;			/* 190-191 : Seconds */
short NMEACourse;			/* 192-193 : Course */
short NMEASpeed;			/* 194-195 : Speed */
short NMEADay;				/* 196-197 : Day */
short NMEAYear;				/* 198-199 : Year */
unsigned int millisecondsToday;		/* 200-203 : Millieconds today */
unsigned short ADCMax;			/* 204-205 : Maximum absolute value for ADC samples for this packet */
short calConst;				/* 206-207 : System constant in tenths of a dB */
short vehicleID;			/* 208-209 : Vehicle ID */
char softwareVersion[6];		/* 210-215 : Software version number */
/* Following items are not in X-Star */
int sphericalCorrection;		/* 216-219 : Initial spherical correction factor (useful for multiping /*/
					/* deep application) * 100 */
unsigned short packetNum;		/* 220-221 : Packet number (1 - N) (Each ping starts with packet 1) */
short ADCDecimation;			/* 222-223 : A/D decimation before FFT */
short decimation;			/* 224-225 : Decimation factor after FFT */
short unuseda[7];			/* 226-239 */
}
s7k_fsdwsegyheader;
	
/* Edgetech FS-DW sidescan (record 3000) */
typedef struct s7kr_fsdwss_struct
{
	s7k_header	header;
	int		msec_timestamp;		/* Relative millisecond timer value */
	int		ping_number;		/* Ping number as received from the Edgetech subsystem */
	int		number_channels;	/* Number of imagery channels to follow (typically 2) */
	int		total_bytes;		/* Total bytes of channel data (and headers) to follow 
							RTH (record type header) (including optional data) */
	int		data_format;		/* Data format:
							0 - envelope
;							1 - I and Q (complex) */
	s7k_fsdwchannel	channel[2];		/* Channel header and data */
	s7k_fsdwssheader ssheader[2];	/* Edgetech sidescan header */
}
s7kr_fsdwss;
	
/* Edgetech FS-DW subbottom (record 3001) */
typedef struct s7kr_fsdwsb_struct
{
	s7k_header	header;
	int		msec_timestamp;		/* Relative millisecond timer value */
	int		ping_number;		/* Ping number as received from the Edgetech subsystem */
	int		number_channels;	/* Number of imagery channels to follow (typically 2) */
	int		total_bytes;		/* Total bytes of channel data (and headers) to follow 
							RTH (record type header) (including optional data) */
	int		data_format;		/* Data format:
							0 - envelope
							1 - I and Q (complex) */
	s7k_fsdwchannel	channel;		/* Channel header and data */
	s7k_fsdwsegyheader segyheader;		/* Segy header for subbottom trace */
}
s7kr_fsdwsb;
	
/* Bluefin Navigation Data Frame (can be included in record 3100) */
typedef struct s7k_bluefin_nav_struct
{
	int		packet_size;		/* size in bytes of this packet including the
							header and appended data */
	unsigned short	version;		/* Version of this frame */
	unsigned short	offset;			/* Offset in bytes to the start of data from the
							start of this packet */
	int		data_type;		/* Data type identifier 
							0 - Navigation data
							1 - Environment data */
	int		data_size;		/* Size of data in bytes */
    	s7k_time 	s7kTime;		/* 7KTIME               u8*10   UTC.*/
	unsigned int	checksum;		/* Checksum for all bytes in record */
	short		reserved;
	unsigned int	quality;
	double		latitude;		/* Latitude (radians) */
	double		longitude;		/* Longitude (radians) */
	float		speed;			/* Speed (m/sec) */
	double		depth;			/* Vehicle depth (m) */
	double		altitude;		/* Vehicle altitude (m) */
	float		roll;			/* Vehicle roll (radians) */
	float		pitch;			/* Vehicle pitch (radians) */
	float		yaw;			/* Vehicle yaw (radians) */
	float		northing_rate;		/* Vehicle northing rate (m/sec) */
	float		easting_rate;		/* Vehicle easting rate (m/sec) */
	float		depth_rate;		/* Vehicle depth rate (m/sec) */
	float		altitude_rate;		/* Vehicle altitude rate (m/sec) */
	float		roll_rate;		/* Vehicle roll rate (radians/sec) */
	float		pitch_rate;		/* Vehicle pitch rate (radians/sec) */
	float		yaw_rate;		/* Vehicle yaw rate (radians/sec) */
	double		position_time;		/* Vehicle position time (unix sec) */
	double		altitude_time;		/* Vehicle altitude time (unix sec) */
}
s7k_bluefin_nav;
	
/* Bluefin Environmental Data Frame (can be included in record 3100) */
typedef struct s7k_bluefin_environmental_struct
{
	int		packet_size;		/* size in bytes of this packet including the
							header and appended data */
	unsigned short	version;		/* Version of this frame */
	unsigned short	offset;			/* Offset in bytes to the start of data from the
							start of this packet */
	int		data_type;		/* Data type identifier 
							0 - Navigation data
							1 - Environment data */
	int		data_size;		/* Size of data in bytes */
    	s7k_time	s7kTime;		/* 7KTIME               u8*10   UTC.*/
	unsigned int	checksum;		/* Checksum for all bytes in record */
	short		reserved1;
	unsigned int	quality;
	float		sound_speed;		/* Sound speed (m/sec) */
	float		conductivity;		/* Conductivity (S/m) */
	float		temperature;		/* Temperature (deg C) */
	float		pressure;		/* Pressure (?) */
	float		salinity;		/* Salinity (?) */
	double		ctd_time;		/* CTD sample time (unix sec) */
	double		temperature_time;	/* Temperature sample time (unix sec) */
	char		reserved2[56];
}
s7k_bluefin_environmental;
	
/* Bluefin Data Frame (record 3100) */
typedef struct s7kr_bluefin_struct
{
	s7k_header	header;
	int		msec_timestamp;		/* Relative millisecond timer value */
	int		number_frames;		/* Number of frames embedded in this record */
	int		frame_size;		/* Embedded frame size in bytes */
	int		data_format;		/* Data type identifier 
							0 - Navigation data
							1 - Environment data */
	char		reserved[16];		/* Reserved */
	s7k_bluefin_nav	nav[BLUEFIN_MAX_FRAMES];			/* Bluefin navigation frames */
	s7k_bluefin_environmental environmental[BLUEFIN_MAX_FRAMES];	/* Bluefin environmental frames */
}
s7kr_bluefin;
	
/* Reson 7k volatile sonar settings (record 7000) */
typedef struct s7kr_volatilesettings_struct
{
	s7k_header	header;
	unsigned int	serial_number;		/* Sonar serial number */
	unsigned int	ping_number;		/* Ping number */
	float		frequency;		/* Transmit frequency (Hertz) */
	float		sample_rate;		/* Sample rate (Hertz) */
	float		receiver_bandwidth;	/* Receiver bandwidth (Hertz) */
	float		pulse_width;		/* Transmit pulse length (seconds) */
	unsigned int	pulse_type;		/* Pulse type:
							0 - rectangular */
	unsigned int	pulse_reserved;		/* Reserved pulse information */
	float		ping_period;		/* Time since last ping (seconds) */
	float		range_selection;	/* Range selection (meters) */
	float		power_selection;	/* Power selection (dB/uPa) */
	float		gain_selection;		/* Gain selection (dB) */
	float		steering_x;		/* Projector steering angle X (radians) */
	float		steering_y;		/* Projector steering angle Y (radians) */
	float		beamwidth_x;		/* Projector -3 dB beamwidth X (radians) */
	float		beamwidth_y;		/* Projector -3 dB beamwidth Y (radians) */
	float		focal_point;		/* Projector focal point (meters) */
	unsigned int	control_flags;		/* Control flags bitfield:
							3-0: auto range method
							7-4: auto bottom detect filter method
							8: bottom detect range filter
							9: bottom detect depth filter
							14-10: auto receiver gain method
							31-15: reserved	*/
	unsigned int	projector_selection;	/* Projector selection (magic number) */
	unsigned int	transmit_flags;		/* Transmit flags bitfield:
							3-0: pitch stabilization method
							7-4: yaw stabilization method
							31-8: reserved */
	unsigned int	hydrophone_selection;	/* Hydrophone selection (magic number) */
	unsigned int	receive_flags;		/* Receive flags bitfield:
							3-0: roll stabilization method
							7-4: dynamic focusing method
							11-8: doppler compensation method
							15-12: match filtering method
							19-16: TVG method
							31-20: reserved */
	float		range_minimum;		/* Bottom detection minimum range (meters) */
	float		range_maximum;		/* Bottom detection maximum range (meters) */
	float		depth_minimum;		/* Bottom detection minimum depth (meters) */
	float		depth_maximum;		/* Bottom detection maximum depth (meters) */
	float		absorption;		/* Absorption (dB/km) */
	float		sound_velocity;		/* Sound velocity (meters/second) */
	float		spreading;		/* Spreading loss (dB) */
}
s7kr_volatilesettings;
	
/* Reson 7k device configuration structure */
typedef struct s7k_device_struct
{
	unsigned int	magic_number;		/* Unique identifier number */
	char		description[64];	/* Device description string */
	unsigned long	serial_number;		/* Device serial number */
	unsigned int	info_length;		/* Length of device specific data (bytes) */
	unsigned int	info_alloc;		/* Memory allocated for data (bytes) */
	char		*info;			/* Device specific data */
}
s7k_device;

/* Reson 7k configuration (record 7001) */
typedef struct s7kr_configuration_struct
{
	s7k_header	header;
	unsigned long	serial_number;		/* Sonar serial number */
	unsigned int	number_devices;		/* Number of devices */
	s7k_device	device[MBSYS_RESON7K_MAX_DEVICE];	/* Device configuration information */
}
s7kr_configuration;

/* Reson 7k beam geometry (record 7004) */
typedef struct s7kr_beamgeometry_struct
{
	s7k_header	header;
	unsigned long	serial_number;		/* Sonar serial number */
	unsigned int	number_beams;		/* Number of receiver beams */
	float		angle_x[MBSYS_RESON7K_MAX_BEAMS];		/* Receiver beam X direction angle (radians) */
	float		angle_y[MBSYS_RESON7K_MAX_BEAMS];		/* Receiver beam Y direction angle (radians) */
	float		beamwidth_x[MBSYS_RESON7K_MAX_BEAMS];	/* Receiver beamwidth X (radians) */
	float		beamwidth_y[MBSYS_RESON7K_MAX_BEAMS];	/* Receiver beamwidth Y (radians) */
}
s7kr_beamgeometry;

/* Reson 7k calibration data (record 7005) */
typedef struct s7kr_calibration_struct
{
	s7k_header	header;
	unsigned long	serial_number;		/* Sonar serial number */
	unsigned short	number_channels;	/* Number of hydrophone receiver channels */
	float		gain[MBSYS_RESON7K_MAX_RECEIVERS];	/* Receiver gain relative to a nominal gain of 1.0 */
	float		phase[MBSYS_RESON7K_MAX_RECEIVERS];/* Receiver phase relative to a 
							nominal phase of 0.0 radians */
}
s7kr_calibration;

/* Reson 7k bathymetry (record 7006) */
typedef struct s7kr_bathymetry_struct
{
	s7k_header	header;
	unsigned long	serial_number;		/* Sonar serial number */
	unsigned int	ping_number;		/* Sequential number */
	unsigned short	number_beams;		/* Number of receiver beams */
	float		range[MBSYS_RESON7K_MAX_BEAMS];	/* Two way travel time (seconds) */
	mb_u_char	quality[MBSYS_RESON7K_MAX_BEAMS];	/* Beam quality bitfield:
							3-0: quality value 
								0 = bad
								15 = best
							7-4: reserved */
	float		intensity[MBSYS_RESON7K_MAX_BEAMS];	/* Signal strength (dB/uPa) */
}
s7kr_bathymetry;

/* Reson 7k backscatter imagery data (record 7007) */
typedef struct s7kr_backscatter_struct
{
	s7k_header	header;
	unsigned long	serial_number;		/* Sonar serial number */
	unsigned int	ping_number;		/* Sequential number */
	float		beam_position;		/* Beam position forward from
							position of beam 0 (meters) */
	unsigned int	control_flags;		/* Control flags bitfield
							3-0: yaw stabilization method
							7-4: beamforming method
							31-8: reserved */
	unsigned int	number_samples;		/* number of samples */
	float		port_beamwidth_x;	/* Port -3 dB X beamwidth 
							(radians - typically a large angle) */
	float		port_beamwidth_y;	/* Port -3 dB Y beamwidth 
							(radians - typically a small angle) */
	float		stbd_beamwidth_x;	/* Starboard -3 dB X beamwidth 
							(radians - typically a large angle) */
	float		stbd_beamwidth_y;	/* Starboard -3 dB Y beamwidth 
							(radians - typically a small angle) */
	float		port_steering_x;	/* Port -3 dB X steering angle
							(radians - typically slightly positive) */
	float		port_steering_y;	/* Port -3 dB Y steering angle 
							(radians - typically pi */
	float		stbd_steering_x;	/* Starboard -3 dB X steering angle 
							(radians - typically slightly positive) */
	float		stbd_steering_y;	/* Starboard -3 dB Y steering angle 
							(radians - typically zero) */
	unsigned short	number_beams;		/* Number of sidescan beams per side (usually only one) */
	unsigned short	current_beam;		/* Beam number of this record (0 to number_beams - 1) */
	mb_u_char	sample_size;		/* Number of bytes per sample */
	mb_u_char	data_type;		/* Data type bitfield:
							0: Amplitude
							1: Phase */
	unsigned int	nalloc;			/* Memory allocated in each array (bytes) */
	char		*port_data;
	char		*stbd_data;
}
s7kr_backscatter;

/* Reson 7k system event (record 7051) */
typedef struct s7kr_systemevent_struct
{
	s7k_header	header;
	unsigned int	serial_number;		/* Sonar serial number */
	unsigned short	event_id;		/* Event id:
							0: success
							1: information (used for MB-System comment record)
							2: warning
							3: error */
	unsigned short	message_length;		/* Message length in bytes */
	unsigned int	message_alloc;		/* Number of bytes allocated for message */
	char		*message;		/* Message string (null terminated) */
}
s7kr_systemevent;

/* Reson 7k subsystem structure */
typedef struct s7kr_subsystem_struct
{
	unsigned int	device_identifier;	/* Identifier for record type of embedded data */
	unsigned short	subsystem_identifier;	/* identifier of the device that this data pertains */
	unsigned short	system_enumerator;	/* Identifier for the device subsystem */
}
s7kr_subsystem;

/* Reson 7k file header (record 7200) */
typedef struct s7kr_fileheader_struct
{
	s7k_header	header;
	char		file_identifier[16];	/* File identifier:
							0xF3302F43CFB04D6FA93E2AEC33DF577D */
	unsigned short	version;		/* File format version number */
	unsigned short	reserved;		/* Reserved */
	char		session_identifier[16];	/* Session identifier - used to associate multiple
							files for a given session */
	unsigned int	record_data_size;	/* Size of record data - 0 if not set */
	unsigned int	number_subsystems;	/* Number of subsystems - 0 if not set */
	char		recording_name[64];	/* Recording program name - null terminated string */
	char		recording_version[16];	/* Recording program version number - null terminated string */
	char		user_defined_name[64];	/* User defined name - null terminated string */
	char		notes[128];		/* Notes - null terminated string */
	s7kr_subsystem	subsystem[MBSYS_RESON7K_MAX_DEVICE];
}
s7kr_fileheader;

/* internal data structure */
struct mbsys_reson7k_struct
	{
	/* Type of data record */
	int		kind;			/* MB-System record ID */
	int		type;			/* Reson record ID */
	int		sstype;			/* If type == R7KRECID_FSDWsidescan
							sstype: 0 = low frequency sidescan 
							 	1 = high frequency sidescan */
	
	/* MB-System time stamp */
	double		time_d;	
	int		time_i[7];
	
	/* Reference point information (record 1000) */
	/*  Note: these offsets should be zero for submersible vehicles */
	s7kr_reference	reference;
	
	/* Sensor uncalibrated offset position information (record 1001) */
	s7kr_sensoruncal	sensoruncal;
	
	/* Sensor calibrated offset position information (record 1002) */
	s7kr_sensorcal	sensorcal;
	
	/* Position (record 1003) */
	s7kr_position	position;
	
	/* Attitude (record 1004) */
	s7kr_attitude	attitude;
	
	/* Tide (record 1005) */
	s7kr_tide	tide;
	
	/* Altitude (record 1006) */
	s7kr_altitude	altitude;
	
	/* Motion over ground (record 1007) */
	s7kr_motion	motion;
	
	/* Depth (record 1008) */
	s7kr_depth	depth;
	
	/* Sound velocity profile (record 1009) */
	s7kr_svp	svp;
	
	/* CTD (record 1010) */
	s7kr_ctd	ctd;
	
	/* Geodesy (record 1011) */
	s7kr_geodesy	geodesy;
	
	/* MB-System 7k survey (record 2000) */
	s7kr_survey	survey;
	
	/* Edgetech FS-DW low frequency sidescan (record 3000) */
	s7kr_fsdwss	fsdwsslo;
	
	/* Edgetech FS-DW high frequency sidescan (record 3000) */
	s7kr_fsdwss	fsdwsshi;

	/* Edgetech FS-DW subbottom (record 3001) */
	s7kr_fsdwsb	fsdwsb;

	/* Bluefin data frames (record 3100) */
	s7kr_bluefin	bluefin;

	/* Reson 7k volatile sonar settings (record 7000) */
	s7kr_volatilesettings	volatilesettings;

	/* Reson 7k configuration (record 7001) */
	s7kr_configuration	configuration;

	/* Reson 7k beam geometry (record 7004) */
	s7kr_beamgeometry	beamgeometry;

	/* Reson 7k calibration (record 7005) */
	s7kr_calibration	calibration;

	/* Reson 7k bathymetry (record 7006) */
	s7kr_bathymetry		bathymetry;

	/* Reson 7k backscatter imagery data (record 7007) */
	s7kr_backscatter	backscatter;

	/* Reson 7k system event (record 7051) */
	s7kr_systemevent	systemevent;

	/* Reson 7k file header (record 7200) */
	s7kr_fileheader		fileheader;
	
	};



/* 7K Macros */
int mbsys_reson7k_checkheader(s7k_header header);
	
/* system specific function prototypes */
int mbsys_reson7k_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_reson7k_survey_alloc(int verbose, 
			void *mbio_ptr, void *store_ptr, 
			int *error);
int mbsys_reson7k_attitude_alloc(int verbose, 
			void *mbio_ptr, void *store_ptr, 
			int *error);
int mbsys_reson7k_heading_alloc(int verbose, 
			void *mbio_ptr, void *store_ptr, 
			int *error);
int mbsys_reson7k_ssv_alloc(int verbose, 
			void *mbio_ptr, void *store_ptr, 
			int *error);
int mbsys_reson7k_tlt_alloc(int verbose, 
			void *mbio_ptr, void *store_ptr, 
			int *error);
int mbsys_reson7k_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_reson7k_zero_ss(int verbose, void *store_ptr, int *error);
int mbsys_reson7k_extract(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_reson7k_insert(int verbose, void *mbio_ptr, void *store_ptr, 
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_reson7k_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles, 
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset, 
			double *draft, double *ssv, int *error);
int mbsys_reson7k_detects(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams, int *detects, int *error);
int mbsys_reson7k_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude, 
			int *error);
int mbsys_reson7k_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft, 
			double *roll, double *pitch, double *heave, 
			int *error);
int mbsys_reson7k_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft, 
			double roll, double pitch, double heave,
			int *error);
int mbsys_reson7k_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, 
			int *nsvp, 
			double *depth, double *velocity,
			int *error);
int mbsys_reson7k_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int nsvp, 
			double *depth, double *velocity,
			int *error);
int mbsys_reson7k_extract_segytraceheader(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind,
		void *segyheader_ptr, 
		int *error);
int mbsys_reson7k_extract_segy(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind,
		void *segyheader_ptr, 
		float *segydata, 
		int *error);
int mbsys_reson7k_insert_segy(int verbose, void *mbio_ptr, void *store_ptr,
		int kind,
		void *segyheader_ptr, 
		float *segydata, 
		int *error);
int mbsys_reson7k_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error);

