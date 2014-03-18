/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_reson7k.h	3/3/2004
 *	$Id$
 *
 *    Copyright (c) 2004-2014 by
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
 * $Log: mbsys_reson7k.h,v $
 * Revision 5.16  2008/09/27 03:27:10  caress
 * Working towards release 5.1.1beta24
 *
 * Revision 5.15  2008/09/20 00:57:41  caress
 * Release 5.1.1beta23
 *
 * Revision 5.14  2008/05/16 22:56:24  caress
 * Release 5.1.1beta18.
 *
 * Revision 5.13  2008/03/01 09:14:03  caress
 * Some housekeeping changes.
 *
 * Revision 5.12  2007/07/03 17:25:50  caress
 * Changes to handle new time lag value in bluefin nav records.
 *
 * Revision 5.11  2006/11/10 22:36:05  caress
 * Working towards release 5.1.0
 *
 * Revision 5.10  2006/09/11 18:55:53  caress
 * Changes during Western Flyer and Thomas Thompson cruises, August-September
 * 2006.
 *
 * Revision 5.9  2006/04/11 19:14:46  caress
 * Various fixes.
 *
 * Revision 5.8  2005/11/05 00:48:05  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.7  2004/12/02 06:33:29  caress
 * Fixes while supporting Reson 7k data.
 *
 * Revision 5.6  2004/11/08 05:47:20  caress
 * Now gets sidescan from snippet data, maybe even properly...
 *
 * Revision 5.5  2004/11/06 03:55:15  caress
 * Working to support the Reson 7k format.
 *
 * Revision 5.4  2004/09/16 19:02:34  caress
 * Changes to better support segy data.
 *
 * Revision 5.3  2004/07/15 19:25:05  caress
 * Progress in supporting Reson 7k data.
 *
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
 *   5. Navigation data may be found in three different record types.
 *      The bathymetry records (kind = MB_DATA_DATA) hold navigation
 *      and attitude data, but these values are not initially set by
 *      the Reson 6046 datalogger. In MB-System these values get set
 *      by running the program mb7kpreprocess by interpolating the
 *      the values found in either the R7KRECID_Position records
 *      (kind = MB_DATA_NAV1) or the R7KRECID_Bluefin records
 *      (kind = MB_DATA_NAV2). MB-System uses the bathymetry records as
 *      the primary navigation source, so the interpolated values are
 *      accessed by mbnavedit and, by default, mbnavlist. The raw values
 *      of the ancillary navigation records (R7KRECID_Position and
 *      R7KRECID_Bluefin) may be accessed by mbnavlist using the -N1
 *      and -N2 options, respectably.
 *   6. Attitude data may be found in three different record types.
 *      The bathymetry records (kind = MB_DATA_DATA) hold navigation
 *      and attitude data, but these values are not initially set by
 *      the Reson 6046 datalogger. In MB-System these values get set
 *      by running the program mb7kpreprocess by interpolating the
 *      the values found in either the R7KRECID_RollPitchHeave records
 *      (kind = MB_DATA_ATTITUDE) or the R7KRECID_Bluefin records
 *      (kind = MB_DATA_NAV2). MB-System uses the bathymetry records as
 *      the primary attitude source, so the interpolated values are
 *      accessed by mbnavedit and, by default, mbnavlist. The raw values
 *      of the secondary ancillary navigation records (R7KRECID_Bluefin),
 *      including attitude, may be accessed by mbnavlist using the -N2
 *      option.
 *   7. The MB-System code assumes that a Reson 7k data file will include
 *      either R7KRECID_RollPitchHeave and R7KRECID_Position records
 *      or R7KRECID_Bluefin records. Bad things will happen if the
 *      data file contains both the generic records and the bluefin
 *      records.
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
#define R7KRECID_CustomAttitude				1004
#define R7KRECID_Tide					1005
#define R7KRECID_Altitude				1006
#define R7KRECID_MotionOverGround			1007
#define R7KRECID_Depth					1008
#define R7KRECID_SoundVelocityProfile			1009
#define R7KRECID_CTD					1010
#define R7KRECID_Geodesy				1011
#define R7KRECID_RollPitchHeave				1012
#define R7KRECID_Heading				1013
#define R7KRECID_SurveyLine				1014
#define R7KRECID_Navigation				1015
#define R7KRECID_Attitude				1016
#define R7KRECID_Rec1022				1022
#define R7KRECID_GenericSensorCalibration		1050
#define R7KRECID_GenericSidescan			1200

/* 2000-2999 reserved for user defined records */
#define R7KRECID_XYZ					2000

/* 3000-6999 reserved for other vendor records */
#define R7KRECID_FSDWsidescan				3000
#define R7KRECID_FSDWsidescanLo				0
#define R7KRECID_FSDWsidescanHi				1
#define R7KRECID_FSDWsubbottom				3001
#define R7KRECID_Bluefin				3100
#define R7KRECID_BluefinNav				0
#define R7KRECID_BluefinEnvironmental			1
#define R7KRECID_ProcessedSidescan		        3199

/* 7000-7999 reserved for SeaBat 7k records */
#define R7KRECID_7kVolatileSonarSettings		7000
#define R7KRECID_7kConfiguration			7001
#define R7KRECID_7kMatchFilter				7002
#define R7KRECID_7kV2FirmwareHardwareConfiguration	7003
#define R7KRECID_7kBeamGeometry				7004
#define R7KRECID_7kCalibrationData			7005
#define R7KRECID_7kBathymetricData			7006
#define R7KRECID_7kBackscatterImageData			7007
#define R7KRECID_7kBeamData				7008
#define R7KRECID_7kVerticalDepth			7009
#define R7KRECID_7kImageData				7011
#define R7KRECID_7kV2PingMotion				7012
#define R7KRECID_7kV2DetectionSetup			7017
#define R7KRECID_7kV2BeamformedData			7018
#define R7KRECID_7kV2BITEData				7021
#define R7KRECID_7kV27kCenterVersion			7022
#define R7KRECID_7kV28kWetEndVersion			7023
#define R7KRECID_7kV2Detection				7026
#define R7KRECID_7kV2RawDetection			7027
#define R7KRECID_7kV2SnippetData			7028
#define R7KRECID_7kInstallationParameters		7030
#define R7KRECID_7kSystemEvents				7050
#define R7KRECID_7kSystemEventMessage			7051
#define R7KRECID_7kTargetData				7060
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
#define R7KRECID_7kReserved				7504
#define R7KRECID_7kRoll					7600
#define R7KRECID_7kPitch				7601
#define R7KRECID_7kSoundVelocity			7610
#define R7KRECID_7kAbsorptionLoss			7611
#define R7KRECID_7kSpreadingLoss			7612
#define R7KRECID_8100SonarData				8100

/* 11000-11199 reserved for Payload Controller command records */
#define R7KRECID_7kPayloadControllerCommand		11000
#define R7KRECID_7kPayloadControllerCommandAcknowledge	11001
#define R7KRECID_7kPayloadControllerStatus		11002

/* 11200-11999 reserved for Payload Controller sensor QC records */

/*---------------------------------------------------------------*/
/* Record size definitions */
#define	MBSYS_RESON7K_VERSIONSYNCSIZE				64
#define	MBSYS_RESON7K_RECORDHEADER_SIZE				64
#define	MBSYS_RESON7K_RECORDTAIL_SIZE				4

/* 0 means no record at all */
#define	R7KHDRSIZE_None						0

/* 1000-1999 reserved for generic sensor records */
#define R7KHDRSIZE_ReferencePoint				16
#define R7KHDRSIZE_UncalibratedSensorOffset			24
#define R7KHDRSIZE_CalibratedSensorOffset			24
#define R7KHDRSIZE_Position					36
#define R7KHDRSIZE_CustomAttitude				8
#define R7KHDRSIZE_Tide						43
#define R7KHDRSIZE_Altitude					4
#define R7KHDRSIZE_MotionOverGround				8
#define R7KHDRSIZE_Depth					8
#define R7KHDRSIZE_SoundVelocityProfile				24
#define R7KRDTSIZE_SoundVelocityProfile				8
#define R7KHDRSIZE_CTD						36
#define R7KRDTSIZE_CTD						20
#define R7KHDRSIZE_Geodesy					320
#define R7KHDRSIZE_RollPitchHeave				12
#define R7KHDRSIZE_Heading					4
#define R7KHDRSIZE_SurveyLine					16
#define R7KRDTSIZE_SurveyLine					16
#define R7KHDRSIZE_Navigation					41
#define R7KHDRSIZE_Attitude					1
#define R7KRDTSIZE_Attitude					18
#define R7KHDRSIZE_Rec1022					40

/* 2000-2999 reserved for user defined records */

/* 3000-6999 reserved for other vendor records */
#define R7KHDRSIZE_FSDWsidescan					32 /* includes added 12 bytes not in Reson 7k data spec */
#define R7KHDRSIZE_FSDWsubbottom				32 /* includes added 12 bytes not in Reson 7k data spec */
#define R7KHDRSIZE_BluefinDataFrame				32
#define R7KHDRSIZE_FSDWchannelinfo				64
#define R7KHDRSIZE_FSDWssheader					80
#define R7KHDRSIZE_FSDWsbheader					240
#define R7KHDRSIZE_ProcessedSidescan    			48

/* 7000-7999 reserved for SeaBat 7k records */
#define R7KHDRSIZE_7kVolatileSonarSettings			156
#define R7KHDRSIZE_7kConfiguration				12
#define R7KHDRSIZE_7kMatchFilter				24
#define R7KHDRSIZE_7kV2FirmwareHardwareConfiguration		8
#define R7KHDRSIZE_7kBeamGeometry				12
#define R7KHDRSIZE_7kCalibrationData				10
#define R7KHDRSIZE_7kBathymetricData_v4				18
#define R7KHDRSIZE_7kBathymetricData				24
#define R7KHDRSIZE_7kBackscatterImageData			64
#define R7KHDRSIZE_7kBeamData					30
#define R7KHDRSIZE_7kVerticalDepth				42
#define R7KHDRSIZE_7kImageData					20
#define R7KHDRSIZE_7kV2PingMotion				28
#define R7KHDRSIZE_7kV2DetectionSetup				116
#define R7KRDTSIZE_7kV2DetectionSetup				30
#define R7KHDRSIZE_7kV2BeamformedData				52
#define R7KHDRSIZE_7kV2BITEData					2
#define R7KRDTSIZE_7kV2BITERecordData				136
#define R7KRDTSIZE_7kV2BITEFieldData				79
#define R7KHDRSIZE_7kV27kCenterVersion				32
#define R7KHDRSIZE_7kV28kWetEndVersion				32
#define R7KHDRSIZE_7kV2Detection				99
#define R7KHDRSIZE_7kV2RawDetection				99
#define R7KHDRSIZE_7kV2SnippetData				46
#define R7KRDTSIZE_7kV2SnippetTimeseries			14
#define R7KHDRSIZE_7kInstallationParameters			616
#define R7KHDRSIZE_7kSystemEvents				22
#define R7KHDRSIZE_7kSystemEventMessage				14
#define R7KHDRSIZE_7kTargetData					121
#define R7KHDRSIZE_7kDataStorageStatus				0
#define R7KHDRSIZE_7kFileHeader					44
#define R7KRDTSIZE_7kFileHeader					272
#define R7KHDRSIZE_7kTrigger					2
#define R7KHDRSIZE_7kTriggerSequenceSetup			2
#define R7KHDRSIZE_7kTriggerSequenceDone			2
#define R7KHDRSIZE_7kTimeMessage				16
#define R7KHDRSIZE_7kRemoteControl				20
#define R7KHDRSIZE_7kRemoteControlAcknowledge			20
#define R7KHDRSIZE_7kRemoteControlNotAcknowledge		24
#define R7KHDRSIZE_7kRemoteControlSonarSettings			260
#define R7KHDRSIZE_7kReserved					543
#define R7KHDRSIZE_7kRoll					4
#define R7KHDRSIZE_7kPitch					4
#define R7KHDRSIZE_7kSoundVelocity				4
#define R7KHDRSIZE_7kAbsorptionLoss				4
#define R7KHDRSIZE_7kSpreadingLoss				4
#define R7KHDRSIZE_8100SonarData				16

/* 11000-11199 reserved for Payload Controller command records */
#define R7KHDRSIZE_7kPayloadControllerCommand			16
#define R7KHDRSIZE_7kPayloadControllerCommandAcknowledge	12
#define R7KHDRSIZE_7kPayloadControllerStatus			16

/* 11200-11999 reserved for Payload Controller sensor QC records */

/*---------------------------------------------------------------*/

/* Device identifiers */
#define R7KDEVID_GenericPosition	100
#define R7KDEVID_GenericHeading		101
#define R7KDEVID_GenericAttitude	102
#define R7KDEVID_GenericMBES		103
#define R7KDEVID_GenericSidescan	104
#define R7KDEVID_GenericSBP		105
#define R7KDEVID_TrueTime		1001
#define R7KDEVID_CDCSMCG		2000
#define R7KDEVID_CDCSPG			2001
#define R7KDEVID_EmpireMagnetics	2002
#define R7KDEVID_ResonTC4013		4013
#define R7KDEVID_ResonDiverDat		6000
#define R7KDEVID_Reson7kCenter		7000
#define R7KDEVID_Reson7kUserInterface	7001
#define R7KDEVID_ResonPDS2000		7003
#define R7KDEVID_SeaBat7012		7012
#define R7KDEVID_SeaBat7100		7100
#define R7KDEVID_SeaBat7101		7101
#define R7KDEVID_SeaBat7102		7102
#define R7KDEVID_SeaBat7111		7111
#define R7KDEVID_SeaBat7112		7112
#define R7KDEVID_SeaBat7123		7123
#define R7KDEVID_SeaBat7125		7125
#define R7KDEVID_SeaBat7128		7128
#define R7KDEVID_SeaBat7150		7150
#define R7KDEVID_SeaBat7160		7160
#define R7KDEVID_SeaBat8100		8100
#define R7KDEVID_SeaBat8101		8101
#define R7KDEVID_SeaBat8102		8102
#define R7KDEVID_SeaBat8112		8111
#define R7KDEVID_SeaBat8123		8123
#define R7KDEVID_SeaBat8124		8124
#define R7KDEVID_SeaBat8125		8125
#define R7KDEVID_SeaBat8128		8128
#define R7KDEVID_SeaBat8150		8150
#define R7KDEVID_SeaBat8160		8160
#define R7KDEVID_TSSDMS05		10000
#define R7KDEVID_TSS335B		10001
#define R7KDEVID_TSS332B		10002
#define R7KDEVID_SeaBirdSBE37		10010
#define R7KDEVID_Littom200		10020
#define R7KDEVID_EdgetechFSDW		11000
#define R7KDEVID_EdgetechFSDWSBP	11000
#define R7KDEVID_EdgetechFSDWSSLF	11001
#define R7KDEVID_EdgetechFSDWSSHF	11002
#define R7KDEVID_Bluefin		11100
#define R7KDEVID_IfremerTechsas		11200
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
#define	MBSYS_RESON7K_MAX_BEAMS		512
#define	MBSYS_RESON7K_MAX_PIXELS	4096

typedef struct s7k_time_struct
{
    unsigned short  Year;			/* Year                 u16 0 - 65535 */
    unsigned short  Day;			/* Day                  u16 1 - 366 */

    float           Seconds;			/* Seconds              f32 0.000000 - 59.000000 */

    mb_u_char       Hours;			/* Hours                u8  0 - 23 */
    mb_u_char       Minutes;			/* Minutes              u8  0 - 59 */
}
s7k_time;

typedef struct s7k_header_struct
{
    unsigned short  Version;                    /* Version              u16 Version of this frame (e.g.: 1, 2 etc.) */
    unsigned short  Offset;                     /* Offset               u16 Offset in bytes from the start of the sync
    												pattern to the start of the DATA SECTION.
												This allows for expansion of the header
												whilst maintaining backward compatibility. */

    unsigned int   SyncPattern;			/* Sync pattern         u32 0x0000FFFF */
    unsigned int   Size;			/* Size                 u32 Size in bytes of this record from the start
    												of the version field to the end of the
												Checksum. It includes the embedded data size. */
    unsigned int   OffsetToOptionalData;	/* Data offset          u32 Offset in bytes to optional data field from
    												start of record. Zero implies no optional data. */
    unsigned int   OptionalDataIdentifier;	/* Data idenfitifer     u32 Identifier for optional data field. Zero for
    	`											no optional field. This identifier is
												described with each record type. */
    s7k_time 	   s7kTime;			/* 7KTIME               u8*10   UTC.*/
    unsigned short Reserved;			/* Reserved  */
    unsigned int   RecordType;			/* Record type          u32 Unique identifier of indicating the type of
    												data embedded in this record. */
    unsigned int   DeviceId;			/* Device identifier    u32 Identifier of the device to which this datum pertains. */
    unsigned short Reserved2;			/* Reserved  */
    unsigned short SystemEnumerator;		/* System enumerator	The enumerator is used to differentiate between devices with the
    									same device identifiers in one installation/system. It is up to
									each application to decide what number to populate this field with.  */
    unsigned int   DataSetNumber;		/* Data set             u32 Data set number - OBSOLETE in version 4 header */
    unsigned int   RecordNumber;		/* Record count         u32 Sequential record counter. */

    char           PreviousRecord[8];		/* Previous record      i64 Pointer to the previous record of the same type
    												(in bytes from start of file). This is an
												optional field for files and shall be -1
												if not used.
												- OBSOLETE in version 4 header */
    char           NextRecord[8];		/* Next record          i64 Pointer to the next record of the same type in
    												bytes from start of file. This is an optional
												field for files and shall be -1 if not used.
												- OBSOLETE in version 4 header */

    unsigned short Flags;			/* Flags                u16 BIT FIELD:
    										Bit 0 - Checksum
											0 - invalid checksum
											1 - valid checksum
										Bit 1 - Reserved
										Bit 2 - Fragmentation
											0 - data unfragmented
											1 - fragmented sequence */
    unsigned short Reserved3;			/* Reserved  */
    unsigned int Reserved4;			/* Reserved  		- NEW in version 4 header */
    unsigned int FragmentedTotal;		/* Total Fragmented	Total records in fragmented data record set (if flag is set) - NEW in version 4 header */
    unsigned int FragmentNumber;		/* Fragment number	Fragment number (if flag is set) - NEW in version 4 header */

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
	unsigned int	datum;			/* 0=WGS84; others not yet defined */
	float		latency;		/* Position sensor time latency (seconds) */
	double		latitude;		/* Latitude (radians) */
	double		longitude;		/* Longitude (radians) */
	double		height;			/* Height relative to datum (meters) */
	mb_u_char	type;			/* Position type flag:
							0: Geographical coordinates
							1: Grid coordinates */
	mb_u_char	utm_zone;		/* UTM zone */
	mb_u_char	quality;		/* Quality flag
							0: Navigation data
							1: Dead reckoning */
	mb_u_char	method;			/* Positioning method
							0: GPS
							1: DGPS
							2: Start of inertial positioning system from GPS
							3: Start of inertial positioning system from DGPS
							4: Start of inertial positioning system from bottom correlation
							5: Start of inertial positioning system from bottom object
							6: Start of inertial positioning system from inertial positioning
							7: Start of inertial positioning system from optional data
							8: Stop of inertial positioning system from GPS
							9: Stop of inertial positioning system from DGPS
							10: Stop of inertial positioning system from bottom correlation
							11: Stop of inertial positioning system from bottom object
							12: Stop of inertial positioning system from inertial positioning
							13: Stop of inertial positioning system from optional data
							14: Optional data
							>14: Reserved */
}
s7kr_position;

/* Custom attitude (record 1004) */
typedef struct s7kr_customattitude_struct
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
	float		*pitchrate;
	float		*rollrate;
	float		*headingrate;
	float		*heaverate;
}
s7kr_customattitude;

/* Tide (record 1005) */
typedef struct s7kr_tide_struct
{
	s7k_header	header;
	float		tide;			/* height correction above mean sea level (meters) */
	unsigned short	source;			/* tide data source: 0 - table; 1- gauge */
	mb_u_char	flags;			/* Gauge and position validity flags
							Bit 0: 0/1 for gauge id valid/invalid
							Bit 1: 0/1 for position valid/invalid */
	unsigned short	gauge;			/* Optional field to permit discrimination
							between different devices */
	unsigned int	datum;			/* 0=WGS84; others not yet defined */
	float		latency;		/* Position sensor time latency (seconds) */
	double		latitude;		/* Latitude (radians) */
	double		longitude;		/* Longitude (radians) */
	double		height;			/* Height relative to datum (meters) */
	mb_u_char	type;			/* Position type flag:
							0: Geographical coordinates
							1: Grid coordinates */
	mb_u_char	utm_zone;		/* UTM zone */
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
	float		frequency;		/* Sample rate */
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
	mb_u_char	validity;		/* Sample content validity:
							Bit 0: conductivity/salinity
							Bit 1: water temperature
							Bit 2: pressure/depth
							Bit 3: sound velocity
							Bit 4: absorption */
	unsigned short	reserved;		/* Reserved field */
	double		latitude;		/* Latitude (radians) */
	double		longitude;		/* Longitude (radians) */
	float		sample_rate;		/* Sample rate */
	unsigned int	n;			/* Number of fields */
	int		nalloc;			/* Number of samples allocated */
	float		*conductivity_salinity;	/* Conductivity (s/m) or salinity (ppt) */
	float		*temperature;		/* Temperature (degrees celcius) */
	float		*pressure_depth;	/* Pressure (pascals) or depth (meters) */
	float		*sound_velocity;	/* Sound velocity (meters/second) */
	float		*absorption;		/* Sound velocity absorption (dB/second) */
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

/* Roll pitch heave (record 1012) */
typedef struct s7kr_rollpitchheave_struct
{
	s7k_header	header;
	float		roll;			/* Roll (radians) */
	float		pitch;			/* Pitch (radians) */
	float		heave;			/* Heave (m) */
}
s7kr_rollpitchheave;

/* Heading (record 1013) */
typedef struct s7kr_heading_struct
{
	s7k_header	header;
	float		heading;		/* Heading (radians) */
}
s7kr_heading;

/* Survey Line (record 1014) */
typedef struct s7kr_surveyline_struct
{
	s7k_header	header;
	unsigned short	n;			/* Number of points */
	unsigned short	type;			/* Position type flag:
							0: Geographical coordinates
							1: Grid coordinates */
	float		turnradius;		/* Turn radius between line segments
							(meters, 0 = no curvature in turns) */
	char		name[64];		/* Line name */
	int		nalloc;			/* Number of points allocated */
	double		*latitude;		/* Latitude (radians, -pi/2 to pi/2) */
	double		*longitude;		/* Longitude (radians -pi to pi) */
}
s7kr_surveyline;

/* Navigation (record 1015) */
typedef struct s7kr_navigation_struct
{
	s7k_header	header;
	mb_u_char	vertical_reference;	/* Vertical reference:
							1 = Ellipsoid
							2 = Geoid
							3 = Chart datum */
	double		latitude;		/* Latitude (radians, -pi/2 to pi/2) */
	double		longitude;		/* Longitude (radians -pi to pi) */
	float		position_accuracy;	/* Horizontal position accuracy (meters) */
	float		height;			/* Height of vessel reference point above
							vertical reference (meters) */
	float		height_accuracy;	/* Height accuracy (meters) */
	float		speed;			/* Speed over ground (meters/sec) */
	float		course;			/* Course over ground (radians) */
	float		heading;		/* Heading (radians) */
}
s7kr_navigation;

/* Attitude (record 1016) */
typedef struct s7kr_attitude_struct
{
	s7k_header	header;
	mb_u_char	n;			/* number of datasets */
	int		nalloc;			/* number of samples allocated */
	unsigned short	*delta_time;		/* Time difference with record timestamp (msec) */
	float		*roll;			/* Roll (radians) */
	float		*pitch;			/* Pitch (radians) */
	float		*heave;			/* Heave (m) */
	float		*heading;		/* Heading (radians) */
}
s7kr_attitude;

/* Unknown record 1022 (record 1022) */
typedef struct s7kr_rec1022_struct
{
	s7k_header	header;
	mb_u_char	data[R7KHDRSIZE_Rec1022];/* raw bytes in unknown record */
}
s7kr_rec1022;

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
char reserved2[2];			/*  70 -  71 : Reserved for future use */
int longitude;				/*  72 -  75 : 0.01 Longitude (arc sec) - Reserved for future use by Edgetech */
int latitude;				/*  76 -  79 : 0.01 Latitude (arc sec) - Reserved for future use by Edgetech */
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
int sourceCoordX;			/* 72-75 : 0.01 arc seconds - original Meters or Seconds of Arc */
int sourceCoordY;			/* 76-79 : 0.01 arc seconds - original Meters or Seconds of Arc */
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
	short		timedelay;		/* Delay of position and altitude time values
							compared to Reson 7k time values (msec)
							- add this value to the position and
							altitude time values to get the
							times synced to the 7k multibeam data */
	unsigned int	quality;		/* Kearfott INS quality and mode info */
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
	double		depth_time;		/* Vehicle depth time (unix sec) */
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
	float		pressure;		/* Pressure (dBar) */
	float		salinity;		/* Salinity (psu) */
	double		ctd_time;		/* CTD sample time (unix sec) */
	double		temperature_time;	/* Temperature sample time (unix sec) */
	double		surface_pressure;	/* dBar */
	int		temperature_counts;	/* thermister A/D counts */
	float		conductivity_frequency;	/* Hz */
	int		pressure_counts;	/* strain gauge pressure sensor A/D counts */
	float		pressure_comp_voltage;	/* Volts */
						/* 5/10/2009 R/V Thompson TN134 Lau Basin
							- added support for five channels
							  of data from analog sensors integrated
							  with the MBARI Mapping AUV
							- Each channel is stored as unsigned 16 bit
							  integers representing -5V to +5V
							- initial use is for PMEL eH and optical
							  backscatter sensors */
	int		sensor_time_sec;	/* Ancilliary sensor time (unix seconds) */
	int		sensor_time_nsec;	/* Ancilliary sensor time (nanno seconds) */
	unsigned short	sensor1;		/* voltage: 0 = -5.00V, 65535 = +5.00V */
	unsigned short	sensor2;		/* voltage: 0 = -5.00V, 65535 = +5.00V */
	unsigned short	sensor3;		/* voltage: 0 = -5.00V, 65535 = +5.00V */
	unsigned short	sensor4;		/* voltage: 0 = -5.00V, 65535 = +5.00V */
	unsigned short	sensor5;		/* voltage: 0 = -5.00V, 65535 = +5.00V */
	unsigned short	sensor6;		/* voltage: 0 = -5.00V, 65535 = +5.00V */
	unsigned short	sensor7;		/* voltage: 0 = -5.00V, 65535 = +5.00V */
	unsigned short	sensor8;		/* voltage: 0 = -5.00V, 65535 = +5.00V */
	char		reserved2[8];
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

/* Processed sidescan - MB-System extension to 7k format (record 3199) */
typedef struct s7kr_processedsidescan_struct
{
	s7k_header	header;
	mb_u_long	serial_number;		/* Sonar serial number */
	unsigned int	ping_number;		/* Sequential number */
	unsigned short	multi_ping;		/* Flag to indicate multi-ping mode
							0 = no multi-ping
							>0 = sequence number of ping
								in the multi-ping
								sequence */
        unsigned short  recordversion;          /* allows for progression of versions of this data record
                                                    version = 1: initial version as of 8 October 2012 */
        unsigned int    ss_source;              /* Source of raw backscatter for this sidescan that has
                                                    been laid out on the seafloor:
                                                        ss_source = 0:     None
                                                        ss_source = 1:     Non-Reson sidescan
                                                        ss_source = 7007:  7kBackscatterImageData
                                                        ss_source = 7008:  7kBeamData
                                                        ss_source = 7028:  7kV2SnippetData */
	unsigned int	number_pixels;		/* Number of sidescan pixels across the entire swath */
	unsigned int	ss_type;		/* indicates if sidescan values are logarithmic or linear
                                                    ss_type = 0: logarithmic (dB)
                                                    ss_type = 1: linear (voltage) */
	float		pixelwidth;		/* Pixel acrosstrack width in m
                                                    Acrosstrack distance of each pixel given by
                                                        acrosstrack = (ipixel - number_pixels / 2) * pixelwidth
                                                    where i = pixel number and N is the total number
                                                    of pixels, counting from port to starboard starting at 0 */
	double		sonardepth;		/* Sonar depth in m */
	double		altitude;		/* Sonar nadir altitude in m */
	float		sidescan[MBSYS_RESON7K_MAX_PIXELS];		/* Depth releative to chart datum in meters */
	float		alongtrack[MBSYS_RESON7K_MAX_PIXELS];	/* Alongtrack distance in meters */
}
s7kr_processedsidescan;

/* Reson 7k volatile sonar settings (record 7000) */
typedef struct s7kr_volatilesettings_struct
{
	s7k_header	header;
	mb_u_long	serial_number;		/* Sonar serial number */
	unsigned int	ping_number;		/* Ping number */
	unsigned short	multi_ping;		/* Flag to indicate multi-ping mode.
							0 = no multi-ping
							>0 = sequence number of the ping
								in the multi-ping
								sequence. */
	float		frequency;		/* Transmit frequency (Hertz) */
	float		sample_rate;		/* Sample rate (Hertz) */
	float		receiver_bandwidth;	/* Receiver bandwidth (Hertz) */
	float		pulse_width;		/* Transmit pulse length (seconds) */
	unsigned int	pulse_type;		/* Pulse type identifier:
							0 - CW
							1 - linear chirp */
	unsigned int	pulse_envelope;		/* Pulse envelope identifier:
							0 - tapered rectangular
							1 - Tukey */
	float		pulse_envelope_par;	/* Pulse envelope parameter */
	unsigned int	pulse_reserved;		/* Reserved pulse information */
	float		max_ping_rate;		/* Maximum ping rate (pings/second) */
	float		ping_period;		/* Time since last ping (seconds) */
	float		range_selection;	/* Range selection (meters) */
	float		power_selection;	/* Power selection (dB/uPa) */
	float		gain_selection;		/* Gain selection (dB) */
	unsigned int	control_flags;		/* Control flags bitfield:
							0-3: auto range method
							4-7: auto bottom detect filter method
							8: bottom detect range filter
							9: bottom detect depth filter
							10-14: auto receiver gain method
							15-31: reserved	*/
	unsigned int	projector_magic_no;	/* Projector selection */
	float		steering_vertical;	/* Projector steering angle vertical (radians) */
	float		steering_horizontal;	/* Projector steering angle horizontal (radians) */
	float		beamwidth_vertical;	/* Projector -3 dB beamwidth vertical (radians) */
	float		beamwidth_horizontal;	/* Projector -3 dB beamwidth horizontal (radians) */
	float		focal_point;		/* Projector focal point (meters) */
	unsigned int	projector_weighting;	/* Projector beam weighting window type:
							0 - rectangular
							1 - Chebyshev */
	float		projector_weighting_par;/* Projector beam weighting window parameter */
	unsigned int	transmit_flags;		/* Transmit flags bitfield:
							0-3: pitch stabilization method
							4-7: yaw stabilization method
							8-31: reserved */
	unsigned int	hydrophone_magic_no;	/* Hydrophone selection (magic number) */
	unsigned int	receive_weighting;	/* Receiver beam weighting window type:
							0 - Chebyshev
							1 - Kaiser */
	float		receive_weighting_par;/* Receiver beam weighting window parameter */
	unsigned int	receive_flags;		/* Receive flags bitfield:
							0-3: roll stabilization method
							4-7: dynamic focusing method
							8-11: doppler compensation method
							12-15: match filtering method
							16-19: TVG method
							20-23: Multi-ping mode
								0 = no multi-ping
								>0 = sequence number of the ping
									in the multi-ping
									sequence.
							24-31: Reserved */
	float		receive_width;		/* Receive beam width (radians) */
	float		range_minimum;		/* Bottom detection minimum range (meters) */
	float		range_maximum;		/* Bottom detection maximum range (meters) */
	float		depth_minimum;		/* Bottom detection minimum depth (meters) */
	float		depth_maximum;		/* Bottom detection maximum depth (meters) */
	float		absorption;		/* Absorption (dB/km) */
	float		sound_velocity;		/* Sound velocity (meters/second) */
	float		spreading;		/* Spreading loss (dB) */
	unsigned short	reserved;		/* reserved for future pulse shape description */
}
s7kr_volatilesettings;

/* Reson 7k device configuration structure */
typedef struct s7k_device_struct
{
	unsigned int	magic_number;		/* Unique identifier number */
	char		description[64];	/* Device description string */
	mb_u_long	serial_number;		/* Device serial number */
	unsigned int	info_length;		/* Length of device specific data (bytes) */
	unsigned int	info_alloc;		/* Memory allocated for data (bytes) */
	char		*info;			/* Device specific data */
}
s7k_device;

/* Reson 7k configuration (record 7001) */
typedef struct s7kr_configuration_struct
{
	s7k_header	header;
	mb_u_long	serial_number;		/* Sonar serial number */
	unsigned int	number_devices;		/* Number of devices */
	s7k_device	device[MBSYS_RESON7K_MAX_DEVICE];	/* Device configuration information */
}
s7kr_configuration;

/* Reson 7k match filter (record 7002) */
typedef struct s7kr_matchfilter_struct
{
	s7k_header	header;
	mb_u_long	serial_number;		/* Sonar serial number */
	unsigned int	ping_number;		/* Sequential number */
	unsigned int	operation;		/* Operation
							0 = off
							1 = on */
	float		start_frequency;	/* Start frequency (Hz) */
	float		end_frequency;		/* End frequency (Hz) */
}
s7kr_matchfilter;

/* Reson 7k firmware and hardware configuration (record 7003) */
typedef struct s7kr_v2firmwarehardwareconfiguration_struct
{
	s7k_header	header;
	unsigned int	device_count;		/* Hardware device count */
	unsigned int	info_length;		/* Info length (bytes) */
	unsigned int	info_alloc;		/* Memory allocated for data (bytes) */
	char		*info;			/* Device specific data */
}
s7kr_v2firmwarehardwareconfiguration;

/* Reson 7k beam geometry (record 7004) */
typedef struct s7kr_beamgeometry_struct
{
	s7k_header	header;
	mb_u_long	serial_number;		/* Sonar serial number */
	unsigned int	number_beams;		/* Number of receiver beams */
	float		angle_alongtrack[MBSYS_RESON7K_MAX_BEAMS];		/* Receiver beam X direction angle (radians) */
	float		angle_acrosstrack[MBSYS_RESON7K_MAX_BEAMS];		/* Receiver beam Y direction angle (radians) */
	float		beamwidth_alongtrack[MBSYS_RESON7K_MAX_BEAMS];	/* Receiver beamwidth X (radians) */
	float		beamwidth_acrosstrack[MBSYS_RESON7K_MAX_BEAMS];	/* Receiver beamwidth Y (radians) */
}
s7kr_beamgeometry;

/* Reson 7k calibration data (record 7005) */
typedef struct s7kr_calibration_struct
{
	s7k_header	header;
	mb_u_long	serial_number;		/* Sonar serial number */
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
	mb_u_long	serial_number;		/* Sonar serial number */
	unsigned int	ping_number;		/* Sequential number */
	unsigned short	multi_ping;		/* Flag to indicate multi-ping mode
							0 = no multi-ping
							>0 = sequence number of ping
								in the multi-ping
								sequence */
	unsigned int	number_beams;		/* Number of receiver beams */
	mb_u_char	layer_comp_flag;	/* Flag indicating if layer compensation is on
							0 = off
							1 = on
							****Not present prior to Version 5!!! */
	mb_u_char	sound_vel_flag;		/* Flag indicating if sound velocity is measured
							or manually entered
							0 = measured
							1 = manually entered
							****Not present prior to Version 5!!! */
	float		sound_velocity;		/* Sound veocity at the sonar (m/sec)
							****Not present prior to Version 5!!! */
	float		range[MBSYS_RESON7K_MAX_BEAMS];	/* Two way travel time (seconds) */
	mb_u_char	quality[MBSYS_RESON7K_MAX_BEAMS];
                                                /* Beam quality bitfield:
							0-3: Reson quality flags use bits 0-3
								bit 0: brightness test
								bit 1: colinearity test
								bit 2: amplitude pick
								bit 3: phase pick
							4-7: MB beam flagging - use bits 4-7
								- bits 0-3 left in original values
								- beam valid if bit 4 or 5 are set
								- beam flagged if bit 6 or 7 set
								bit 4: on = amplitude
								bit 5: on = phase
								bit 6: on = auto flag
								bit 7: on = manual flag */
	float		intensity[MBSYS_RESON7K_MAX_BEAMS];	/* Signal strength (dB/uPa) */
        float           min_depth_gate[MBSYS_RESON7K_MAX_BEAMS];
                                                /* Minimum two-way travel time to filter point
                                                   for each beam (minimum depth gate) */
        float           max_depth_gate[MBSYS_RESON7K_MAX_BEAMS];
                                                /* Maximum two-way travel time to filter point
                                                   for each beam (maximum depth gate) */
	unsigned int	optionaldata;		/* Flag indicating if bathymetry calculated and
							values below filled in
								0 = No
								1 = Yes
							This is an internal MB-System flag, not
							a value in the data format */
	float		frequency;		/* Ping frequency in Hz */
	double		latitude;		/* Latitude of vessel reference point
							in radians, -pi/2 to +pi/2 */
	double		longitude;		/* Longitude of vessel reference point
							in radians, -pi to +pi */
	float		heading;		/* Heading of vessel at transmit time
							in radians */
	unsigned char	height_source;		/* Method used to correct to chart datum.
							0 = None
							1 = RTK (implies tide = 0.0)
							2 = Tide */
	float		tide;			/* Tide in meters */
	float		roll;			/* Roll at transmit time */
	float		pitch;			/* Pitch at transmit time */
	float		heave;			/* Heave at transmit time in m*/
	float		vehicle_height;		/* Vehicle height at transmit time in m */
	float		depth[MBSYS_RESON7K_MAX_BEAMS];		/* Depth releative to chart datum in meters */
	float		alongtrack[MBSYS_RESON7K_MAX_BEAMS];	/* Alongtrack distance in meters */
	float		acrosstrack[MBSYS_RESON7K_MAX_BEAMS];	/* Acrosstrack distance in meters */
	float		pointing_angle[MBSYS_RESON7K_MAX_BEAMS];/* Pointing angle from vertical in radians */
	float		azimuth_angle[MBSYS_RESON7K_MAX_BEAMS];	/* Azimuth angle in radians */

	int             acrossalongerror;           /* MB-System flipped the order of the alongtrack
                                                     * and acrosstrack distance values through 4.3.2003
                                                     * - if acrossalongerror == MB_MAYBE check max values of
                                                     *       the acrosstrack and alongtrack arrays and treat the larger
                                                     *       as acrosstrack - if this is found to be the case ten
                                                     *       times then set acrossalongerro = MB_YES and always flip
                                                     *       the values - if this is found to not be the case ten
                                                     *       times then set acrossalongerror = MB_NO and never flip
                                                     *       the values. */
	int             nacrossalongerroryes;       /* counter for times acrosstrack and alongtrack values flipped */
	int             nacrossalongerrorno;        /* counter for times acrosstrack and alongtrack values not flipped */
}
s7kr_bathymetry;

/* Reson 7k backscatter imagery data (record 7007) */
typedef struct s7kr_backscatter_struct
{
	s7k_header	header;
	mb_u_long	serial_number;		/* Sonar serial number */
	unsigned int	ping_number;		/* Sequential number */
	unsigned short	multi_ping;		/* Flag to indicate multi-ping mode
							0 = no multi-ping
							>0 = sequence number of ping
								in the multi-ping
								sequence */
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
	unsigned int	optionaldata;		/* Flag indicating if values below filled in
								0 = No
								1 = Yes
							This is an internal MB-System flag, not
							a value in the data format */
	float		frequency;		/* Ping frequency in Hz */
	double		latitude;		/* Latitude of vessel reference point
							in radians, -pi/2 to +pi/2 */
	double		longitude;		/* Longitude of vessel reference point
							in radians, -pi to +pi */
	float		heading;		/* Heading of vessel at transmit time
							in radians */
	float		altitude;		/* Altitude in meters for slant range correction */
}
s7kr_backscatter;

/* Reson 7k snippet data (part of record 7008) */
typedef struct s7kr_snippet_struct
{
	unsigned short	beam_number;		/* Beam or element number */
	unsigned int	begin_sample;		/* First sample number in beam from transmitter and outward. */
	unsigned int	end_sample;		/* Last sample number in beam from transmitter and outward. */
	unsigned int	nalloc_amp;		/* Bytes allocated to hold amplitude time series */
	unsigned int	nalloc_phase;		/* Bytes allocated to hold phase time series */
	void		*amplitude;		/* Amplitude or I time series as defined by sample_type */
	void		*phase;			/* Phase or Q time series as defined by sample_type */
}
s7kr_snippet;

/* Reson 7k beam data (record 7008) */
typedef struct s7kr_beam_struct
{
	s7k_header	header;
	mb_u_long	serial_number;		/* Sonar serial number */
	unsigned int	ping_number;		/* Sequential number */
	unsigned short	multi_ping;		/* Flag to indicate multi-ping mode
							0 = no multi-ping
							>0 = sequence number of ping
								in the multi-ping
								sequence */
	unsigned short	number_beams;		/* Total number of beams or elements in record */
	unsigned short	reserved;		/* Reserved */
	unsigned int	number_samples;		/* Number of samples in ping. Only valid if all
							beams and samples are in record. */
	mb_u_char	record_subset_flag;	/* Record subset flag:
							0 = All beams and samples in ping
							1 = Beam and/or sample ping subset */
	mb_u_char	row_column_flag;	/* Row column flag:
							0 = Beam followed by samples
							1 = Sample follows by beams */
	unsigned short	sample_header_id;	/* Sample header identifier
							0 = no sample header */
	unsigned int	sample_type;		/* Data sample type
							0-3: Amplitude:
								0 = No amplitude
								1 = Amplitude (8 bits)
								2 = Amplitude (16 bits)
								3 = Amplitude (32 bits)
							4-7: Phase:
								0 = No phase
								1 = Phase (8 bits)
								2 = Phase (16 bits)
								3 = Phase (32 bits)
							8-11: I and Q
								0 = No I and Q
								1 = Signed 16 bit I and signed 16 bit Q
								2 = Signed 32 bit I and signed 32 bit Q
							12-14: Beam forming flag:
								0 = Beam formed data
								1 = Element data */
	s7kr_snippet	snippets[MBSYS_RESON7K_MAX_RECEIVERS];
}
s7kr_beam;

/* Reson 7k vertical depth (record 7009) */
typedef struct s7kr_verticaldepth_struct
{
	s7k_header	header;
	float		frequency;		/* Sonar frequency in Hz */
	unsigned int	ping_number;		/* Sequential number */
	unsigned short	multi_ping;		/* Flag to indicate multi-ping mode
							0 = no multi-ping
							>0 = sequence number of ping
								in the multi-ping
								sequence */
	double		latitude;		/* Latitude of vessel reference point
							in radians, -pi/2 to +pi/2 */
	double		longitude;		/* Longitude of vessel reference point
							in radians, -pi to +pi */
	float		heading;		/* Heading of vessel at transmit time
							in radians */
	float		alongtrack;		/* Sonar alongtrack distance from
							vessel reference point in meters */
	float		acrosstrack;		/* Sonar alongtrack distance from
							vessel reference point in meters */
	float		vertical_depth;		/* Sonar vertical depth with respect
							to chart datum or vessel if
							tide data are unavailable in meters */
}
s7kr_verticaldepth;

/* Reson 7k image data (record 7011) */
typedef struct s7kr_image_struct
{
	s7k_header	header;
	unsigned int	ping_number;		/* Sequential number */
	unsigned short	multi_ping;		/* Flag to indicate multi-ping mode
							0 = no multi-ping
							>0 = sequence number of ping
								in the multi-ping
								sequence */
	unsigned int	width;			/* Image width in pixels */
	unsigned int	height;			/* Image height in pixels */
	unsigned short	color_depth;			/* Color depth per pixel in bytes */
	unsigned short	width_height_flag;	/* Image data width-height flag:
							0 = Width followed by height
							1 = Height followed by width */
	unsigned short	compression;		/* Compression algorithm
							0 = no compression */
	unsigned int	nalloc;			/* Number of bytes allocated to image array */
	void		*image;			/* Array of image data */
}
s7kr_image;

/* Reson 7k ping motion (record 7012) */
typedef struct s7kr_v2pingmotion_struct
{
	s7k_header	header;
	mb_u_long	serial_number;		/* Sonar serial number */
	unsigned int	ping_number;		/* Sequential number */
	unsigned short	multi_ping;		/* Flag to indicate multi-ping mode
							0 = no multi-ping
							>0 = sequence number of ping
								in the multi-ping
								sequence */
	unsigned int	n;			/* number of samples */
	unsigned short	flags;			/* Bit field:
							Bit 0:
								1 = Pitch stabilization applied
									/ pitch field present
							Bit 1:
								1 = Roll stabilization applied
									/ roll field present
							Bit 2: Yaw stabilization applied
									/ yaw field present
							Bit 3: Heave stabilization applied
									/ heave field present
							Bit 4-15: Reserved */
	unsigned int	error_flags;		/* Bit field:
							Bit 0:
								PHINS referecne 0 = invalid, 1 = valid
							Bit 1-3:
								Reserved for PHINS
							Bit 4: Roll angle > 15 degrees
							Bit 5: Pitch angle > 35 degrees
							Bit 6: Roll rate > 10 degrees
							Bit 7-15: Reserved */
	float		frequency;		/* sampling frequency (Hz) */
	float		pitch;			/* Pitch value at the ping time (radians) */
	int		nalloc;			/* number of samples allocated */
	float		*roll;			/* Roll (radians) */
	float		*heading;		/* Heading (radians) */
	float		*heave;			/* Heave (m) */
}
s7kr_v2pingmotion;

/* Reson 7k detection setup (record 7017) */
typedef struct s7kr_v2detectionsetup_struct
{
	s7k_header	header;
	mb_u_long	serial_number;		/* Sonar serial number */
	unsigned int	ping_number;		/* Sequential number */
	unsigned short	multi_ping;		/* Flag to indicate multi-ping mode
							0 = no multi-ping
							>0 = sequence number of ping
								in the multi-ping
								sequence */
	unsigned int	number_beams;		/* Number of detection points */
	unsigned int	data_field_size;	/* Size of detection information block in bytes */
	mb_u_char	detection_algorithm;	/* Detection algorithm:
							0 = G1_Simple
							1 = G1_BlendFilt
							2 = G2
							3-255: Reserved for future use */
	unsigned int	detection_flags;		/* Bit field:
							Bit 0: 1 = User-defined depth filter enabled
							Bit 1: 1 = User-defined range filter enabled
							Bit 2: 1 = Automatic filter enabled
							Bit 3: 1 = Nadir search limits enabled
							Bit 4: 1 = Automatic window limits enabled
							Bits 5-31: Reserved for future use */
	float		minimum_depth;		/* Minimum depth for user-defined filter (meters) */
	float		maximum_depth;		/* Maximum depth for user-defined filter (meters) */
	float		minimum_range;		/* Minimum range for user-defined filter (meters) */
	float		maximum_range;		/* Maximum range for user-defined filter (meters) */
	float		minimum_nadir_search;	/* Minimum depth for automatic filter nadir search (meters) */
	float		maximum_nadir_search;	/* Maximum depth for automatic filter nadir search (meters) */
	mb_u_char	automatic_filter_window;/* Automatic filter window size (percent altitude) */
        float           applied_roll;           /* Roll value (in radians) applied to gates; zero if roll stabilization is on */
        float           depth_gate_tilt;        /* Angle in radians (positive to starboard) */
	float   	reserved[14];		/* Reserved for future use */
	unsigned short	beam_descriptor[MBSYS_RESON7K_MAX_BEAMS];
						/* Beam number the detection is taken from */
	float		detection_point[MBSYS_RESON7K_MAX_BEAMS];
						/* Non-corrected fractional sample number with
							the reference to the receiver's
							acoustic center with the zero sample
							at the transmit time */
	unsigned int	flags[MBSYS_RESON7K_MAX_BEAMS];
						/* Bit field:
							Bit 0: 1 = automatic limits valid
							Bit 1: 1 = User-defined limits valid
							Bit 2-8: Quality type, defines the type of the
									quality field
								0: Quality not available / not used
								1: Quality used
							Bit 9: 1 = Quality passes user-defined criteria
									or no user-defined criteria was
									specified
							Bit 10: 1 = Magnitude based detection
							Bit 11: 1 = Phase based detection
							Bit 12: 1 = Other detection 1
							Bit 13-31: Reserved for future use
						    Note that bits 1-12 are not mutually exclusive. For example,
							bits 10 & 11 will both be set when the current "blend" of
							magnitude and phase detection is used. */
	unsigned int	auto_limits_min_sample[MBSYS_RESON7K_MAX_BEAMS];
						/* Minimum sample number for automatic limits */
	unsigned int	auto_limits_max_sample[MBSYS_RESON7K_MAX_BEAMS];
						/* Maximum sample number for automatic limits */
	unsigned int	user_limits_min_sample[MBSYS_RESON7K_MAX_BEAMS];
						/* Minimum sample number for user-defined limits */
	unsigned int	user_limits_max_sample[MBSYS_RESON7K_MAX_BEAMS];
						/* Maximum sample number for user-defined limits */
	unsigned int	quality[MBSYS_RESON7K_MAX_BEAMS];		/* Bit field:
							Bit 0: 1 = Brightness filter passed
							Bit 1: 1 = Colinearity filter passed
							Bit 2-31: Reserved for future use */
	float           uncertainty[MBSYS_RESON7K_MAX_BEAMS];   /* Detection uncertainty represented as an error normalized to the detection point */
}
s7kr_v2detectionsetup;

/* Reson 7k amplitude and phase data (part of record 7018) */
typedef struct s7kr_v2amplitudephase_struct
{
	unsigned short	beam_number;		/* Beam or element number */
	unsigned int	number_samples;		/* Number of samples */
	unsigned int	nalloc;			/* Number of samples allocated */
	unsigned short 	*amplitude;		/* Amplitude time series  */
	short		*phase;			/* Phase time series (radians scaled by 10430) */
}
s7kr_v2amplitudephase;

/* Reson 7k beamformed magnitude and phase data (record 7018) */
typedef struct s7kr_v2beamformed_struct
{
	s7k_header	header;
	mb_u_long	serial_number;		/* Sonar serial number */
	unsigned int	ping_number;		/* Sequential number */
	unsigned short	multi_ping;		/* Flag to indicate multi-ping mode
							0 = no multi-ping
							>0 = sequence number of ping
								in the multi-ping
								sequence */
	unsigned short	number_beams;		/* Total number of beams or elements in record */
	unsigned int	number_samples;		/* Number of samples in each beam in this record */
	mb_u_char	reserved[32];		/* Reserved for future use */
	s7kr_v2amplitudephase	amplitudephase[MBSYS_RESON7K_MAX_BEAMS];
						/* amplitude and phase data for each beam */
}
s7kr_v2beamformed;

/* Reson 7k BITE field (part of record 7021) */
typedef struct s7kr_v2bitefield_struct
{
	unsigned short	reserved;		/* Reserved */
	char		name[64];		/* Name - null terminated ASCII string */
	mb_u_char	device_type;		/* Device type:
							1 = Error count
							2 = FPGA die temperature
							3 = Humidity
							4 = Serial 8-channel ADC
							5 = Firmware version
							6 = Head Temp,_8K WetEnd
							7 = Leak V,_8K WetEnd
							8 = 5 Volt,_8K WetEnd
							9 = 12 Volt,_8K WetEnd
							10 = DipSwitch,_8K WetEnd */
	float		minimum;		/* Minimum value */
	float		maximum;		/* Maximum value */
	float		value;			/* Current value */
}
s7kr_v2bitefield;

/* Reson 7k BITE (part of record 7021) */
typedef struct s7kr_v2bitereport_struct
{
	char		source_name[64];	/* source name - null terminated string */
	mb_u_char	source_address;		/* source address */
	float		frequency;		/* frequency for transmitter or 0 */
	unsigned short	enumerator;		/* Enumerator for transmitter or 0 */
	s7k_time	downlink_time;		/* Downlink time sent */
	s7k_time	uplink_time;		/* Uplink time received */
	s7k_time	bite_time;		/* BITE time received */
	mb_u_char	status;			/* Bit field:
							Bit 0:
								0 = Uplink ok
								1 = Uplink error
							Bit 1:
								0 = Downlink ok
								1 = Downlink error
							Bit 2:
								0 = BITE ok
								1 = BITE error */
	unsigned short	number_bite;		/* Number of valid BITE fields for this board */
	mb_u_char	bite_status[32];		/* Each bit delineates status of one BITE channel up to 256:
							0 = BITE field within range
							1 = BITE field out of range */
	s7kr_v2bitefield	bitefield[256];	/* Array of BITE field data */
}
s7kr_v2bitereport;

/* Reson 7k BITE (record 7021) */
typedef struct s7kr_v2bite_struct
{
	s7k_header	header;
	unsigned short	number_reports;		/* Number of Built In Test Environment reports */
	unsigned int	nalloc;
	s7kr_v2bitereport *reports;
}
s7kr_v2bite;

/* Reson 7k center version (record 7022) */
typedef struct s7kr_v27kcenterversion_struct
{
	s7k_header	header;
	char		version[32];		/* Null terminated ASCII string */
}
s7kr_v27kcenterversion;

/* Reson 7k 8k wet end version (record 7023) */
typedef struct s7kr_v28kwetendversion_struct
{
	s7k_header	header;
	char		version[32];		/* Null terminated ASCII string */
}
s7kr_v28kwetendversion;

/* Reson 7k version 2 detection (record 7026) */
typedef struct s7kr_v2detection_struct
{
	s7k_header	header;
	mb_u_long	serial_number;		/* Sonar serial number */
	unsigned int	ping_number;		/* Sequential number */
	unsigned short	multi_ping;		/* Flag to indicate multi-ping mode
							0 = no multi-ping
							>0 = sequence number of ping
								in the multi-ping
								sequence */
	unsigned int	number_beams;		/* Number of detection points */
	unsigned int	data_field_size;	/* Size of detection information block in bytes */
	mb_u_long 	corrections;		/* Corrections/Methods bit field:
							Bit 0:
								1 = Geometrical corrections for
									cylindrical arrays applied
							Bit 1-2:
								0 = Manually entered surface sound velocity used
								1 = Measured surface sound velocity used
								2,3 = reserved for future use
							Bit 3:
								1 = Roll stabilization applied
							Bit 4:
								1 = Pitch stablization applied
							Bits 5-63:
								Reserved for future use */
	mb_u_char	detection_algorithm;	/* Detection algorithm:
							0 = G1_Simple
							1 = G1_BlendFilt
							2 = G2
							3-255: Reserved for future use */
	unsigned int	flags;			/* Bit field:
							Bit 0:
								1 = Quality filter applied. Only detections
									that pass user-defined criteria
									are generated.
							Bit 1:
								1 = Motion error(s) detected. Data may not
									be accurate
							Bit 2-3: Reference frame
								0 = Sonar
								1 = Vessel
								2-3 = Reserved */
	mb_u_char	reserved[64];		/* Reserved for future use */

	float		range[MBSYS_RESON7K_MAX_BEAMS];
						/* Two-way travel time to the bottom/target (seconds) */
	float		angle_x[MBSYS_RESON7K_MAX_BEAMS];
						/* Across-track angle to detection point (radians) */
	float		angle_y[MBSYS_RESON7K_MAX_BEAMS];
						/* Along-track angle to detection point (radians) */
	float		range_error[MBSYS_RESON7K_MAX_BEAMS];
						/* Measurement error (seconds) */
	float		angle_x_error[MBSYS_RESON7K_MAX_BEAMS];
						/* Measurement error (radians) */
	float		angle_y_error[MBSYS_RESON7K_MAX_BEAMS];
						/* Measurement error (radians) */
}
s7kr_v2detection;

/* Reson 7k version 2 raw detection (record 7027) */
typedef struct s7kr_v2rawdetection_struct
{
	s7k_header	header;
	mb_u_long	serial_number;		/* Sonar serial number */
	unsigned int	ping_number;		/* Sequential number */
	unsigned short	multi_ping;		/* Flag to indicate multi-ping mode
							0 = no multi-ping
							>0 = sequence number of ping
								in the multi-ping
								sequence */
	unsigned int	number_beams;		/* Number of detection points */
	unsigned int	data_field_size;	/* Size of detection information block in bytes */
	mb_u_char	detection_algorithm;	/* Detection algorithm:
							0 = G1_Simple
							1 = G1_BlendFilt
							2 = G2
							3-255: Reserved for future use */
	unsigned int	detection_flags;	/* Bit field: Bits 0-31: Reserved for future use */
	float		sampling_rate;		/* Sonar's sampling frequency in Hz */
	float		tx_angle;		/* Applied transmitter steering angle, in radians */
	mb_u_char	reserved[64];		/* Reserved for future use */

	unsigned short	beam_descriptor[MBSYS_RESON7K_MAX_BEAMS];
						/* Beam number the detection is taken from */
	float		detection_point[MBSYS_RESON7K_MAX_BEAMS];
						/* Non-corrected fractional sample number with
							the reference to the receiver's
							acoustic center with the zero sample
							at the transmit time */
	float		rx_angle[MBSYS_RESON7K_MAX_BEAMS];
						/* Beam steering angle with reference to
							receiver's acoustic center in the
							sonar reference frame, at the detection
							point, in radians */
	unsigned int	flags[MBSYS_RESON7K_MAX_BEAMS];
						/* Bit fields:
							Bit 0: 1 = Magnitude based detection
							Bit 1: 1 = Phase based detection
							Bits 2-8: Quality type, defines the type
								of the quality field below
								0: Quality not available / not used
								1: Quality available
								2-31: Reserved for future use
							Bit 9: Uncertainty information is available
							Bits 10-31: Reserved for future use */
	unsigned int	quality[MBSYS_RESON7K_MAX_BEAMS];
						/* Detection quality:
							Bit 0: 1 = Brightness filter passed
							Bit 1: 1 = Co-linearity filter passed */
	float		uncertainty[MBSYS_RESON7K_MAX_BEAMS];
						/* Detection uncertainty represented as an error
							normalized to the detection point */
}
s7kr_v2rawdetection;

/* Reson 7k version 2 snippet data (part of record 7028) */
typedef struct s7kr_v2snippettimeseries_struct
{
	unsigned short	beam_number;		/* Beam or element number */
	unsigned int	begin_sample;		/* First sample included in snippet */
	unsigned int	detect_sample;		/* Detection point */
	unsigned int	end_sample;		/* Last sample included in snippet */
	unsigned int	nalloc;			/* Bytes allocated to hold amplitude time series */
	unsigned short	*amplitude;		/* Amplitude time series */
}
s7kr_v2snippettimeseries;

/* Reson 7k version 2 snippet (record 7028) */
typedef struct s7kr_v2snippet_struct
{
	s7k_header	header;
	mb_u_long	serial_number;		/* Sonar serial number */
	unsigned int	ping_number;		/* Sequential number */
	unsigned short	multi_ping;		/* Flag to indicate multi-ping mode
							0 = no multi-ping
							>0 = sequence number of ping
								in the multi-ping
								sequence */
	unsigned short	number_beams;		/* Number of detection points */
	mb_u_char	error_flag;		/* If set, record will not contain any data
							Flag itself will indicate an error:
							0 = Ok
							6 = Bottom detection failed
							Other = reserved */
	mb_u_char	control_flags;		/* Control settings from RC 1118 command:
							Bit 0: Automatic snippet window is used
							Bit 1: Quality filter enabled
							Bit 2: Minimum window size is required
							Bit 3: Maximum window size is required
							Bit 4-7: Reserved */
	mb_u_char	reserved[28];		/* Reserved for future use */
	s7kr_v2snippettimeseries	snippettimeseries[MBSYS_RESON7K_MAX_BEAMS];
						/* Snippet time series for each beam */
}
s7kr_v2snippet;

/* Reson 7k sonar installation parameters (record 7030) */
typedef struct s7kr_installation_struct
{
	s7k_header	header;
	float		frequency;		/* Sonar frequency (Hz) */
	unsigned short	firmware_version_len;	/* Length of firmware version info in bytes */
	char		firmware_version[128];	/* Firmware version info */
	unsigned short	software_version_len;	/* Length of software version info in bytes */
	char		software_version[128];	/* Software version info */
	unsigned short	s7k_version_len;	/* Length of 7k software version info in bytes */
	char		s7k_version[128];	/* 7k software version info */
	unsigned short	protocal_version_len;	/* Length of protocal version info in bytes */
	char		protocal_version[128];	/* Protocal version info */
	float		transmit_x;		/* Sonar transmit array X offset (m) */
	float		transmit_y;		/* Sonar transmit array Y offset (m) */
	float		transmit_z;		/* Sonar transmit array Z offset (m) */
	float		transmit_roll;		/* Sonar transmit array roll offset radiansm) */
	float		transmit_pitch;		/* Sonar transmit array pitch offset (radians) */
	float		transmit_heading;	/* Sonar transmit array heading offset (radians) */
	float		receive_x;		/* Sonar receive array X offset (m) */
	float		receive_y;		/* Sonar receive array Y offset (m) */
	float		receive_z;		/* Sonar receive array Z offset (m) */
	float		receive_roll;		/* Sonar receive array roll offset (radians) */
	float		receive_pitch;		/* Sonar receive array pitch offset (radians) */
	float		receive_heading;	/* Sonar receive array heading offset (radians) */
	float		motion_x;		/* Motion sensor X offset (m) */
	float		motion_y;		/* Motion sensor Y offset (m) */
	float		motion_z;		/* Motion sensor Z offset (m) */
	float		motion_roll;		/* Motion sensor roll offset (radians) */
	float		motion_pitch;		/* Motion sensor pitch offset (radians) */
	float		motion_heading;		/* Motion sensor heading offset (radians) */
	unsigned short	motion_time_delay;	/* Motion sensor time delay (msec) */
	float		position_x;		/* Position sensor X offset (m) */
	float		position_y;		/* Position sensor Y offset (m) */
	float		position_z;		/* Position sensor Z offset (m) */
	unsigned short	position_time_delay;	/* Position sensor time delay (msec) */
	float		waterline_z;		/* Vertical offset from reference
							point to waterline (m) */
}
s7kr_installation;

/* Reson 7k system event (record 7051) */
typedef struct s7kr_systemeventmessage_struct
{
	s7k_header	header;
	mb_u_long	serial_number;		/* Sonar serial number */
	unsigned short	event_id;		/* Event id:
							0: success
							1: information (used for MB-System comment record)
							2: warning
							3: error */
	unsigned short	message_length;		/* Message length in bytes */
	unsigned short	event_identifier;	/* Undefined */
	unsigned int	message_alloc;		/* Number of bytes allocated for message */
	char		*message;		/* Message string (null terminated) */
}
s7kr_systemeventmessage;

/* Reson 7k subsystem structure */
typedef struct s7kr_subsystem_struct
{
	unsigned int	device_identifier;	/* Identifier for record type of embedded data */
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

/* Reson 7k remote control sonar settings (record 7503) */
typedef struct s7kr_remotecontrolsettings_struct
{
	s7k_header	header;
	mb_u_long	serial_number;		/* Sonar serial number */
	unsigned int	ping_number;		/* Ping number */
	float		frequency;		/* Transmit frequency (Hertz) */
	float		sample_rate;		/* Sample rate (Hertz) */
	float		receiver_bandwidth;	/* Receiver bandwidth (Hertz) */
	float		pulse_width;		/* Transmit pulse length (seconds) */
	unsigned int	pulse_type;		/* Pulse type identifier:
							0 - CW
							1 - linear chirp */
	unsigned int	pulse_envelope;		/* Pulse envelope identifier:
							0 - tapered rectangular
							1 - Tukey */
	float		pulse_envelope_par;	/* Pulse envelope parameter */
	unsigned int	pulse_reserved;		/* Reserved pulse information */
	float		max_ping_rate;		/* Maximum ping rate (pings/second) */
	float		ping_period;		/* Time since last ping (seconds) */
	float		range_selection;	/* Range selection (meters) */
	float		power_selection;	/* Power selection (dB/uPa) */
	float		gain_selection;		/* Gain selection (dB) */
	unsigned int	control_flags;		/* Control flags bitfield:
							0-3: auto range method
							4-7: auto bottom detect filter method
							8: bottom detect range filter
							9: bottom detect depth filter
							10-14: auto receiver gain method
							15-31: reserved	*/
	unsigned int	projector_magic_no;	/* Projector selection */
	float		steering_vertical;	/* Projector steering angle vertical (radians) */
	float		steering_horizontal;	/* Projector steering angle horizontal (radians) */
	float		beamwidth_vertical;	/* Projector -3 dB beamwidth vertical (radians) */
	float		beamwidth_horizontal;	/* Projector -3 dB beamwidth horizontal (radians) */
	float		focal_point;		/* Projector focal point (meters) */
	unsigned int	projector_weighting;	/* Projector beam weighting window type:
							0 - rectangular
							1 - Chebyshev */
	float		projector_weighting_par;/* Projector beam weighting window parameter */
	unsigned int	transmit_flags;		/* Transmit flags bitfield:
							0-3: pitch stabilization method
							4-7: yaw stabilization method
							8-31: reserved */
	unsigned int	hydrophone_magic_no;	/* Hydrophone selection (magic number) */
	unsigned int	receive_weighting;	/* Receiver beam weighting window type:
							0 - Chebyshev
							1 - Kaiser */
	float		receive_weighting_par;/* Receiver beam weighting window parameter */
	unsigned int	receive_flags;		/* Receive flags bitfield:
							0-3: roll stabilization method
							4-7: dynamic focusing method
							8-11: doppler compensation method
							12-15: match filtering method
							16-19: TVG method
							20-23: Multi-ping mode
								0 = no multi-ping
								>0 = sequence number of the ping
									in the multi-ping
									sequence.
							24-31: Reserved */
	float		range_minimum;		/* Bottom detection minimum range (meters) */
	float		range_maximum;		/* Bottom detection maximum range (meters) */
	float		depth_minimum;		/* Bottom detection minimum depth (meters) */
	float		depth_maximum;		/* Bottom detection maximum depth (meters) */
	float		absorption;		/* Absorption (dB/km) */
	float		sound_velocity;		/* Sound velocity (meters/second) */
	float		spreading;		/* Spreading loss (dB) */
	unsigned short	reserved;		/* reserved for future pulse shape description */

	/* parameters added by version 1.0 */
	float		tx_offset_x;		/* Offset of the transducer array in m, relative
							to the receiver array on the x axis, positive
							value is to the right, if the receiver faces
							forward. */
	float		tx_offset_y;		/* Offset of the transducer array in m, relative
							to the receiver array on the y axis, positive
							value is forward, if the receiver faces
							forward. */
	float		tx_offset_z;		/* Offset of the transducer array in m, relative
							to the receiver array on the z axis, positive
							value is up, if the receiver faces forward. */
	float		head_tilt_x;		/* Head tilt x (radians) */
	float		head_tilt_y;		/* Head tilt y (radians) */
	float		head_tilt_z;		/* Head tilt z (radians) */
	unsigned short	ping_on_off;		/* Ping on/off state:
							0 = pinging disabled
							1 = pinging enabled */
	mb_u_char	data_sample_types;	/* */
	mb_u_char	projector_orientation;	/* Projector orientation:
							0: down
							1: up */
	unsigned short	beam_angle_mode;	/* Beam angle spacing mode:
							1: equiangle
							2: eqidistant */
	unsigned short	r7kcenter_mode;		/* 7kCenter mode:
							0: normal
							1: autopilot
							2: calibration (IQ)
							3+: reserved */
	float		gate_depth_min;		/* Adaptive gate minimum depth */
	float		gate_depth_max;		/* Adaptive gate maximum depth */
	unsigned short	reserved2[35];
}
s7kr_remotecontrolsettings;

/* Reson 7k Reserved (well, unknown really...) (record 7504) */
typedef struct s7kr_reserved_struct
{
	s7k_header	header;
	mb_u_char	reserved[R7KHDRSIZE_7kReserved];	/* raw bytes of unknown record */
}
s7kr_reserved;

/* Reson 7k Roll (record 7600) */
typedef struct s7kr_roll_struct
{
	s7k_header	header;
	float		roll;			/* Roll (radians) */
}
s7kr_roll;

/* Reson 7k Pitch (record 7601) */
typedef struct s7kr_pitch_struct
{
	s7k_header	header;
	float		pitch;			/* Pitch (radians) */
}
s7kr_pitch;

/* Reson 7k Sound Velocity (record 7610) */
typedef struct s7kr_soundvelocity_struct
{
	s7k_header	header;
	float		soundvelocity;		/* Water sound speed (m/s) */
}
s7kr_soundvelocity;

/* Reson 7k Absorption Loss (record 7611) */
typedef struct s7kr_absorptionloss_struct
{
	s7k_header	header;
	float		absorptionloss;		/* Absorption loss (dB/km) */
}
s7kr_absorptionloss;

/* Reson 7k Spreading Loss (record 7612) */
typedef struct s7kr_spreadingloss_struct
{
	s7k_header	header;
	float		spreadingloss;		/* dB (0 - 60) */
}
s7kr_spreadingloss;

/* internal data structure */
struct mbsys_reson7k_struct
	{
	/* Type of data record */
	int		kind;			/* MB-System record ID */
	int		type;			/* Reson record ID */
	int		sstype;			/* If type == R7KRECID_FSDWsidescan
							sstype: 0 = low frequency sidescan
							 	1 = high frequency sidescan */

	/* ping record id's */
	int		current_ping_number;
	int		read_volatilesettings;
	int		read_matchfilter;
	int		read_beamgeometry;
	int		read_remotecontrolsettings;
	int		read_bathymetry;
	int		read_backscatter;
	int		read_beam;
	int		read_verticaldepth;
	int		read_image;
	int		read_v2pingmotion;
	int		read_v2detectionsetup;
	int		read_v2beamformed;
	int		read_v2detection;
	int		read_v2rawdetection;
	int		read_v2snippet;
	int		read_processedsidescan;

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

	/* Custom attitude (record 1004) */
	s7kr_customattitude	customattitude;

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

	/* Roll pitch heave (record 1012) */
	s7kr_rollpitchheave	rollpitchheave;

	/* Heading (record 1013) */
	s7kr_heading	heading;

	/* Survey line (record 1014) */
	s7kr_surveyline	surveyline;

	/* Navigation (record 1015) */
	s7kr_navigation	navigation;

	/* Attitude (record 1016) */
	s7kr_attitude	attitude;

	/* Unknown record 1022 (record 1022) */
	s7kr_rec1022	rec1022;

	/* Edgetech FS-DW low frequency sidescan (record 3000) */
	s7kr_fsdwss	fsdwsslo;

	/* Edgetech FS-DW high frequency sidescan (record 3000) */
	s7kr_fsdwss	fsdwsshi;

	/* Edgetech FS-DW subbottom (record 3001) */
	s7kr_fsdwsb	fsdwsb;

	/* Bluefin data frames (record 3100) */
	s7kr_bluefin	bluefin;

        /* Processed sidescan - MB-System extension to 7k format (record 3199) */
	s7kr_processedsidescan   processedsidescan;

	/* Reson 7k volatile sonar settings (record 7000) */
	s7kr_volatilesettings	volatilesettings;

	/* Reson 7k configuration (record 7001) */
	s7kr_configuration	configuration;

	/* Reson 7k match filter (record 7002) */
	s7kr_matchfilter	matchfilter;

	/* Reson 7k firmware and hardware configuration (record 7003) */
	s7kr_v2firmwarehardwareconfiguration	v2firmwarehardwareconfiguration;

	/* Reson 7k beam geometry (record 7004) */
	s7kr_beamgeometry	beamgeometry;

	/* Reson 7k calibration (record 7005) */
	s7kr_calibration	calibration;

	/* Reson 7k bathymetry (record 7006) */
	s7kr_bathymetry		bathymetry;

	/* Reson 7k backscatter imagery data (record 7007) */
	s7kr_backscatter	backscatter;

	/* Reson 7k beam data (record 7008) */
	s7kr_beam		beam;

	/* Reson 7k vertical depth (record 7009) */
	s7kr_verticaldepth	verticaldepth;

	/* Reson 7k image data (record 7011) */
	s7kr_image		image;

	/* Ping motion (record 7012) */
	s7kr_v2pingmotion	v2pingmotion;

	/* Detection setup (record 7017) */
	s7kr_v2detectionsetup	v2detectionsetup;

	/* Reson 7k beamformed magnitude and phase data (record 7018) */
	s7kr_v2beamformed	v2beamformed;

	/* Reson 7k BITE (record 7021) */
	s7kr_v2bite	v2bite;

	/* Reson 7k center version (record 7022) */
	s7kr_v27kcenterversion	v27kcenterversion;

	/* Reson 7k 8k wet end version (record 7023) */
	s7kr_v28kwetendversion	v28kwetendversion;

	/* Reson 7k version 2 detection (record 7026) */
	s7kr_v2detection	v2detection;

	/* Reson 7k version 2 raw detection (record 7027) */
	s7kr_v2rawdetection	v2rawdetection;

	/* Reson 7k version 2 snippet (record 7028) */
	s7kr_v2snippet	v2snippet;

	/* Reson 7k sonar installation parameters (record 7030) */
	s7kr_installation	installation;

	/* Reson 7k system event (record 7051) */
	s7kr_systemeventmessage	systemeventmessage;

	/* Reson 7k file header (record 7200) */
	s7kr_fileheader		fileheader;

	/* Reson 7k remote control sonar settings (record 7503) */
	s7kr_remotecontrolsettings	remotecontrolsettings;

	/* Reson 7k Reserved (well, unknown really...) (record 7504) */
	s7kr_reserved		reserved;

	/* Reson 7k Roll (record 7600) */
	s7kr_roll		roll;

	/* Reson 7k Pitch (record 7601) */
	s7kr_pitch		pitch;

	/* Reson 7k Sound Velocity (record 7610) */
	s7kr_soundvelocity	soundvelocity;

	/* Reson 7k Absorption Loss (record 7611) */
	s7kr_absorptionloss	absorptionloss;

	/* Reson 7k Spreading Loss (record 7612) */
	s7kr_spreadingloss	spreadingloss;

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
int mbsys_reson7k_dimensions(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_reson7k_pingnumber(int verbose, void *mbio_ptr,
			int *pingnumber, int *error);
int mbsys_reson7k_sonartype(int verbose, void *mbio_ptr, void *store_ptr,
		int *sonartype, int *error);
int mbsys_reson7k_sidescantype(int verbose, void *mbio_ptr, void *store_ptr,
		int *ss_type, int *error);
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
int mbsys_reson7k_gains(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transmit_gain, double *pulse_length,
			double *receive_gain, int *error);
int mbsys_reson7k_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude,
			int *error);
int mbsys_reson7k_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft,
			double *roll, double *pitch, double *heave,
			int *error);
int mbsys_reson7k_extract_nnav(int verbose, void *mbio_ptr, void *store_ptr,
			int nmax, int *kind, int *n,
			int *time_i, double *time_d,
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
			int *sampleformat,
			int *kind,
			void *segyheader_ptr,
			float *segydata,
			int *error);
int mbsys_reson7k_insert_segy(int verbose, void *mbio_ptr, void *store_ptr,
			int kind,
			void *segyheader_ptr,
			float *segydata,
			int *error);
int mbsys_reson7k_ctd(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nctd, double *time_d,
			double *conductivity, double *temperature,
			double *depth, double *salinity, double *soundspeed, int *error);
int mbsys_reson7k_ancilliarysensor(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nsamples, double *time_d,
			double *sensor1, double *sensor2, double *sensor3,
			double *sensor4, double *sensor5, double *sensor6,
			double *sensor7, double *sensor8, int *error);
int mbsys_reson7k_copy(int verbose, void *mbio_ptr,
			void *store_ptr, void *copy_ptr,
			int *error);
int mbsys_reson7k_checkheader(s7k_header header);
int mbsys_reson7k_makess(int verbose, void *mbio_ptr, void *store_ptr,
			int source, int pixel_size_set, double *pixel_size,
			int swath_width_set, double *swath_width,
			int pixel_int, int *error);
int mbsys_reson7k_print_header(int verbose,
			s7k_header *header,
			int *error);
int mbsys_reson7k_print_reference(int verbose,
			s7kr_reference *reference,
			int *error);
int mbsys_reson7k_print_sensoruncal(int verbose,
			s7kr_sensoruncal *sensoruncal,
			int *error);
int mbsys_reson7k_print_sensorcal(int verbose,
			s7kr_sensorcal *sensorcal,
			int *error);
int mbsys_reson7k_print_position(int verbose,
			s7kr_position *position,
			int *error);
int mbsys_reson7k_print_customattitude(int verbose,
			s7kr_customattitude *customattitude,
			int *error);
int mbsys_reson7k_print_tide(int verbose,
			s7kr_tide *tide,
			int *error);
int mbsys_reson7k_print_altitude(int verbose,
			s7kr_altitude *altitude,
			int *error);
int mbsys_reson7k_print_motion(int verbose,
			s7kr_motion *motion,
			int *error);
int mbsys_reson7k_print_depth(int verbose,
			s7kr_depth *depth,
			int *error);
int mbsys_reson7k_print_svp(int verbose,
			s7kr_svp *svp,
			int *error);
int mbsys_reson7k_print_ctd(int verbose,
			s7kr_ctd *ctd,
			int *error);
int mbsys_reson7k_print_geodesy(int verbose,
			s7kr_geodesy *geodesy,
			int *error);
int mbsys_reson7k_print_rollpitchheave(int verbose,
			s7kr_rollpitchheave *rollpitchheave,
			int *error);
int mbsys_reson7k_print_heading(int verbose,
			s7kr_heading *heading,
			int *error);
int mbsys_reson7k_print_surveyline(int verbose,
			s7kr_surveyline *surveyline,
			int *error);
int mbsys_reson7k_print_navigation(int verbose,
			s7kr_navigation *navigation,
			int *error);
int mbsys_reson7k_print_attitude(int verbose,
			s7kr_attitude *attitude,
			int *error);
int mbsys_reson7k_print_rec1022(int verbose,
			s7kr_rec1022 *rec1022,
			int *error);
int mbsys_reson7k_print_fsdwchannel(int verbose, int data_format,
			s7k_fsdwchannel *fsdwchannel,
			int *error);
int mbsys_reson7k_print_fsdwssheader(int verbose,
			s7k_fsdwssheader *fsdwssheader,
			int *error);
int mbsys_reson7k_print_fsdwsegyheader(int verbose,
			s7k_fsdwsegyheader *fsdwsegyheader,
			int *error);
int mbsys_reson7k_print_fsdwss(int verbose,
			s7kr_fsdwss *fsdwss,
			int *error);
int mbsys_reson7k_print_fsdwsb(int verbose,
			s7kr_fsdwsb *fsdwsb,
			int *error);
int mbsys_reson7k_print_bluefin(int verbose,
			s7kr_bluefin *bluefin,
			int *error);
int mbsys_reson7k_print_processedsidescan(int verbose,
			s7kr_processedsidescan *processedsidescan,
			int *error);
int mbsys_reson7k_print_volatilesettings(int verbose,
			s7kr_volatilesettings *volatilesettings,
			int *error);
int mbsys_reson7k_print_device(int verbose,
			s7k_device *device,
			int *error);
int mbsys_reson7k_print_configuration(int verbose,
			s7kr_configuration *configuration,
			int *error);
int mbsys_reson7k_print_matchfilter(int verbose,
			s7kr_matchfilter *matchfilter,
			int *error);
int mbsys_reson7k_print_v2firmwarehardwareconfiguration(int verbose,
			s7kr_v2firmwarehardwareconfiguration *v2firmwarehardwareconfiguration,
			int *error);
int mbsys_reson7k_print_beamgeometry(int verbose,
			s7kr_beamgeometry *beamgeometry,
			int *error);
int mbsys_reson7k_print_calibration(int verbose,
			s7kr_calibration *calibration,
			int *error);
int mbsys_reson7k_print_bathymetry(int verbose,
			s7kr_bathymetry *bathymetry,
			int *error);
int mbsys_reson7k_print_backscatter(int verbose,
			s7kr_backscatter *backscatter,
			int *error);
int mbsys_reson7k_print_beam(int verbose,
			s7kr_beam *beam,
			int *error);
int mbsys_reson7k_print_verticaldepth(int verbose,
			s7kr_verticaldepth *verticaldepth,
			int *error);
int mbsys_reson7k_print_image(int verbose,
			s7kr_image *image,
			int *error);
int mbsys_reson7k_print_v2pingmotion(int verbose,
			s7kr_v2pingmotion *v2pingmotion,
			int *error);
int mbsys_reson7k_print_v2detectionsetup(int verbose,
			s7kr_v2detectionsetup *v2detectionsetup,
			int *error);
int mbsys_reson7k_print_v2beamformed(int verbose,
			s7kr_v2beamformed *v2beamformed,
			int *error);
int mbsys_reson7k_print_v2bite(int verbose,
			s7kr_v2bite *v2bite,
			int *error);
int mbsys_reson7k_print_v27kcenterversion(int verbose,
			s7kr_v27kcenterversion *v27kcenterversion,
			int *error);
int mbsys_reson7k_print_v28kwetendversion(int verbose,
			s7kr_v28kwetendversion *v28kwetendversion,
			int *error);
int mbsys_reson7k_print_v2detection(int verbose,
			s7kr_v2detection *v2detection,
			int *error);
int mbsys_reson7k_print_v2rawdetection(int verbose,
			s7kr_v2rawdetection *v2rawdetection,
			int *error);
int mbsys_reson7k_print_v2snippet(int verbose,
			s7kr_v2snippet *v2snippet,
			int *error);
int mbsys_reson7k_print_installation(int verbose,
			s7kr_installation *installation,
			int *error);
int mbsys_reson7k_print_systemeventmessage(int verbose,
			s7kr_systemeventmessage *systemeventmessage,
			int *error);
int mbsys_reson7k_print_subsystem(int verbose,
			s7kr_subsystem *subsystem,
			int *error);
int mbsys_reson7k_print_fileheader(int verbose,
			s7kr_fileheader *fileheader,
			int *error);
int mbsys_reson7k_print_remotecontrolsettings(int verbose,
			s7kr_remotecontrolsettings *remotecontrolsettings,
			int *error);
int mbsys_reson7k_print_reserved(int verbose,
			s7kr_reserved *reserved,
			int *error);
int mbsys_reson7k_print_roll(int verbose,
			s7kr_roll *roll,
			int *error);
int mbsys_reson7k_print_pitch(int verbose,
			s7kr_pitch *pitch,
			int *error);
int mbsys_reson7k_print_soundvelocity(int verbose,
			s7kr_soundvelocity *soundvelocity,
			int *error);
int mbsys_reson7k_print_absorptionloss(int verbose,
			s7kr_absorptionloss *absorptionloss,
			int *error);
int mbsys_reson7k_print_spreadingloss(int verbose,
			s7kr_spreadingloss *spreadingloss,
			int *error);
