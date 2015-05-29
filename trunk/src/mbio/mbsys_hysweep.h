/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_hysweep.h	12/23/2011
 *	$Id$
 *
 *    Copyright (c) 2011-2015 by
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
 * This header file mbsys_hysweep.h defines the data structures used by
 * MBIO functions to store data multibeam sonar systems logged using
 * HYSWEEP software by HYPACK Inc.
 *
 * The data format associated with this representation is:
 *      MBF_HYSWEEP1 : MBIO ID 201
 *
 * Author:	D. W. Caress (MBARI)
 * Date:	December 23, 2011
 *
 *
 */
/*
 * Notes on the MBSYS_HYSWEEP HYPACK and HYSWEEP data:
 *   1. The raw HYPACK and HYSWEEP data formats are associated with HYPACK software
 *      from Hypack Inc. of Middletown, CT, USA, formally known as Coastal
 *      Oceanographics. The raw format produced by the Hypack dataloggers
 *      is constructed of pure ASCII text.
 *
 *      The following information is take more or less verbatim from the
 *	2010 HSX format specification.
 *
 *	Coastal Oceanographics, Inc.
 *	Technical Note:
 *	HSX Format - Hysweep Text (ASCII) Logging
 *	Hysweep survey has a Text logging option (HSX format), allowing raw data
 *	to be stored in a format that can be inspected and modified by most
 *	editing program (Windows Wordpad for example). Easy inspection of files
 *	is the advantage of text logging - the disadvantage is larger files and
 *	slower load time. If file size and load time are important to you, it is
 *	best to choose the Hysweep binary format (HS2).
 *
 *	HSX files are generally compatible with Hypack Survey raw format, allowing
 *	Hypack programs (Hypack Max, Hyplot, etc.) to work with HSX files. The
 *	differences involve logging and processing of multibeam data, which is
 *	by the Hysweep extensions to Hypack.
 *
 *	Each file contains two sections; a header, which is written when data
 *	logging starts, and a data section, which is written as data is collected.
 *	Most records starts with a three character tag.
 *
 *	Header Tags:
 *	DEV	Hypack Device Information
 *	DV2	Hysweep Device Information
 *	EOH	End of Header
 *	EOL	End of Planned Line
 *	FTP	File Type (Hypack File Identifier)
 *	HSP	Hysweep Survey Parameters
 *	HSX	HSX File Identifier
 *	HVF	Hysweep View Filters
 *	INF	General Information
 *	LBP	Planned Line Begin Point
 *	LIN	Planned Line Data follows
 *	LNN	Planned Line Name
 *	MBI	Multibeam / Multiple Transducer Device information
 *	0F2	Hysweep Device Offsets
 *	PRI	Primary Navigation Device
 *	PTS	Planned Line Waypoint
 *	SSI	Sidescan Device Information
 *	SVC	Sound Velocity Correction
 *	TND	Survey Time and Date
 *	PRJ	Projected coordinate system - MB-System extension
 *
 *	Data Tags:
 *	DFT	Dynamic Draft (Squat) Correction
 *	FIX	Fix (Event) Mark
 *	HCP	Heave Compensation
 *	EC1	Echo Sounding (single frequency)
 *	GPS	GPS Measurements
 *	GYR	Gyro Data (Heading)
 *	POS	Position
 *	PSA	Pitch Stabilization Angle.
 *	RMB	Raw Multibeam data
 *	RSS	Raw Sidescan data
 *	SNR	Sonar Runtime Settings
 *	TID	Tide Correction
 *	COM	Comment - MB-System extension
 *	PRJ	Projected coordinate system definition for eastings and
 *	            northings in file - MB-System extension
 *	MSS	Sidescan laid out on seafloor - MB-System extension
 *
 */

/* HYSWEEP record type */
#define MBSYS_HYSWEEP_RECORDTYPE_NONE	0	/* No record */
#define MBSYS_HYSWEEP_RECORDTYPE_DEV	1
#define MBSYS_HYSWEEP_RECORDTYPE_DV2	2
#define MBSYS_HYSWEEP_RECORDTYPE_EOH	3
#define MBSYS_HYSWEEP_RECORDTYPE_EOL	4
#define MBSYS_HYSWEEP_RECORDTYPE_FTP	5
#define MBSYS_HYSWEEP_RECORDTYPE_HSP	6
#define MBSYS_HYSWEEP_RECORDTYPE_HSX	7
#define MBSYS_HYSWEEP_RECORDTYPE_HVF	8
#define MBSYS_HYSWEEP_RECORDTYPE_INF	9
#define MBSYS_HYSWEEP_RECORDTYPE_LBP	11
#define MBSYS_HYSWEEP_RECORDTYPE_LIN	12
#define MBSYS_HYSWEEP_RECORDTYPE_LNN	13
#define MBSYS_HYSWEEP_RECORDTYPE_MBI	14
#define MBSYS_HYSWEEP_RECORDTYPE_OF2	15
#define MBSYS_HYSWEEP_RECORDTYPE_PRI	16
#define MBSYS_HYSWEEP_RECORDTYPE_PTS	17
#define MBSYS_HYSWEEP_RECORDTYPE_SSI	18
#define MBSYS_HYSWEEP_RECORDTYPE_SVC	19
#define MBSYS_HYSWEEP_RECORDTYPE_TND	20
#define MBSYS_HYSWEEP_RECORDTYPE_DFT	21
#define MBSYS_HYSWEEP_RECORDTYPE_FIX	22
#define MBSYS_HYSWEEP_RECORDTYPE_HCP	23
#define MBSYS_HYSWEEP_RECORDTYPE_EC1	24
#define MBSYS_HYSWEEP_RECORDTYPE_GPS	25
#define MBSYS_HYSWEEP_RECORDTYPE_GYR	26
#define MBSYS_HYSWEEP_RECORDTYPE_POS	27
#define MBSYS_HYSWEEP_RECORDTYPE_PSA	28
#define MBSYS_HYSWEEP_RECORDTYPE_RMB	29
#define MBSYS_HYSWEEP_RECORDTYPE_RSS	30
#define MBSYS_HYSWEEP_RECORDTYPE_SNR	31
#define MBSYS_HYSWEEP_RECORDTYPE_TID	32
#define MBSYS_HYSWEEP_RECORDTYPE_VER	33
#define MBSYS_HYSWEEP_RECORDTYPE_COM	101
#define MBSYS_HYSWEEP_RECORDTYPE_PRJ	102
#define MBSYS_HYSWEEP_RECORDTYPE_MSS	103

/* sonar types */
#define	MBSYS_HYSWEEP_SONAR_UNKNOWN			0	/* *Not Specified - 0 */
#define	MBSYS_HYSWEEP_SONAR_RESON_SEABAT8101_150	1	/* Reson Seabat 8101 - 150 Deg - 1 */
#define	MBSYS_HYSWEEP_SONAR_ATLAS_FANSWEEP20		2	/* Atlas Fansweep 20 - 2 */
#define	MBSYS_HYSWEEP_SONAR_BENTHOS_C3D			3	/* Benthos C3D - 3 */
#define	MBSYS_HYSWEEP_SONAR_CMAX_CM2			4	/* CMAX CM-2 - 4 */
#define	MBSYS_HYSWEEP_SONAR_EDGETECH_272		5	/* EdgeTech 272 - 5 */
#define	MBSYS_HYSWEEP_SONAR_EDGETECH_4100		6	/* EdgeTech 4100 - 6 */
#define	MBSYS_HYSWEEP_SONAR_EDGETECH_4125		7	/* EdgeTech 4125 - 7 */
#define	MBSYS_HYSWEEP_SONAR_EDGETECH_4150		8	/* EdgeTech 4150 - 8 */
#define	MBSYS_HYSWEEP_SONAR_EDGETECH_4200		9	/* EdgeTech 4200 - 9 */
#define	MBSYS_HYSWEEP_SONAR_EDGETECH_4300		10	/* EdgeTech 4300 - 10 */
#define	MBSYS_HYSWEEP_SONAR_GEOACOUSTICS_GEOSWATH	11	/* GeoAcoustics GeoSwath - 11 */
#define	MBSYS_HYSWEEP_SONAR_IMAGENEX_SPORTSCAN		12	/* Imagenex Sportscan - 12 */
#define	MBSYS_HYSWEEP_SONAR_IMAGENEX_YELLOWFIN		13	/* Imagenex Yellowfin - 13 */
#define	MBSYS_HYSWEEP_SONAR_KLEIN_595			14	/* Klein 595 - 14 */
#define	MBSYS_HYSWEEP_SONAR_KLEIN_2000			15	/* Klein 2000 - 15 */
#define	MBSYS_HYSWEEP_SONAR_KLEIN_3000			16	/* Klein 3000 - 16 */
#define	MBSYS_HYSWEEP_SONAR_KLEIN_3900			17	/* Klein 3900 - 17 */
#define	MBSYS_HYSWEEP_SONAR_KLEIN_5000			18	/* Klein 5000 - 18 */
#define	MBSYS_HYSWEEP_SONAR_ODOM_CV3			19	/* Odom CV3 - 19 */
#define	MBSYS_HYSWEEP_SONAR_ODOM_ECHOSCAN2		20	/* Odom Echoscan 2 - 20 */
#define	MBSYS_HYSWEEP_SONAR_ODOM_ES3			21	/* Odom ES3 - 21 */
#define	MBSYS_HYSWEEP_SONAR_RESON_SEABAT7125		22	/* Reson Seabat 7125 - 22 */
#define	MBSYS_HYSWEEP_SONAR_RESON_SEABAT8111		23	/* Reson Seabat 8111 - 23 */
#define	MBSYS_HYSWEEP_SONAR_RESON_SEABAT8124		24	/* Reson Seabat 8124 - 24 */
#define	MBSYS_HYSWEEP_SONAR_RESON_SEABAT8125		25	/* Reson Seabat 8125 - 25 */
#define	MBSYS_HYSWEEP_SONAR_RESON_SEABAT9001		26	/* Reson Seabat 9001 - 26 */
#define	MBSYS_HYSWEEP_SONAR_RESON_SEABAT9003		27	/* Reson Seabat 9003 - 27 */
#define	MBSYS_HYSWEEP_SONAR_SEA_SWATHPLUS		28	/* SEA Swathplus - 28 */
#define	MBSYS_HYSWEEP_SONAR_SEABEAM_2100		29	/* Seabeam 2100 - 29 */
#define	MBSYS_HYSWEEP_SONAR_SEABEAM_1185		30	/* Seabeam SB1185 - 30 */
#define	MBSYS_HYSWEEP_SONAR_SIMRAD_EA400		31	/* Simrad EA400 - 31 */
#define	MBSYS_HYSWEEP_SONAR_SIMRAD_EM102		32	/* Simrad EM102 - 32 */
#define	MBSYS_HYSWEEP_SONAR_SIMRAD_EM1002		33	/* Simrad EM1002 - 33 */
#define	MBSYS_HYSWEEP_SONAR_SIMRAD_EM2000		34	/* Simrad EM2000 - 34 */
#define	MBSYS_HYSWEEP_SONAR_SIMRAD_EM3000		35	/* Simrad EM3000 - 35 */
#define	MBSYS_HYSWEEP_SONAR_SIMRAD_EM3000D		36	/* Simrad EM3000D - 36 */
#define	MBSYS_HYSWEEP_SONAR_SIMRAD_EM3002		37	/* Simrad EM3002 - 37 */
#define	MBSYS_HYSWEEP_SONAR_SIMRAD_EM3002D		38	/* Simrad EM3002D - 38 */
#define	MBSYS_HYSWEEP_SONAR_RESON_SEABAT8101_210	39	/* Reson Seabat 8101 - 210 Deg - 39 */
#define	MBSYS_HYSWEEP_SONAR_IMAGENEX_DELTAT		40	/* Imagenex Delta T - 40 */
#define	MBSYS_HYSWEEP_SONAR_ATLAS_HYDROSWEEPMD2		41	/* Atlas Hydrosweep MD2 - 41 */
#define	MBSYS_HYSWEEP_SONAR_SIMRAD_SM2000		42	/* Simrad SM2000 - 42 */
#define	MBSYS_HYSWEEP_SONAR_SIMRAD_EM710		43	/* Simrad EM710 - 43 */
#define	MBSYS_HYSWEEP_SONAR_SIMRAD_EM302		44	/* Simrad EM302 - 44 */
#define	MBSYS_HYSWEEP_SONAR_BLUEVIEW_MB1350_45		45	/* Blueview MB1350-45 - 45 */
#define	MBSYS_HYSWEEP_SONAR_BLUEVIEW_MB2250_45		46	/* Blueview MB2250-45 - 46 */
#define	MBSYS_HYSWEEP_SONAR_BLUEVIEW_MB1350_90		47	/* Blueview MB1350-90 - 47 */
#define	MBSYS_HYSWEEP_SONAR_BLUEVIEW_MB2250_90		48	/* Blueview MB2250-90 - 48 */
#define	MBSYS_HYSWEEP_SONAR_GEOACOUSTICS_DSS		49	/* Geoacoustics digital sidescan - 49 */
#define	MBSYS_HYSWEEP_SONAR_BENTHOS_1624		50	/* Benthos 1624 - 50 */
#define	MBSYS_HYSWEEP_SONAR_BENTHOS_1625		51	/* Benthos 1625 - 51 */
#define	MBSYS_HYSWEEP_SONAR_MARINESONIC_SEASCAN		52	/* Marine Sonic Sea Scan - 52 */
#define	MBSYS_HYSWEEP_SONAR_RESON_7101			53	/* Reson Seabat 7101 - 53 */
#define	MBSYS_HYSWEEP_SONAR_FURUNO_HS300F		54	/* Furuno HS-300F - 54 */
#define	MBSYS_HYSWEEP_SONAR_FURUNO_HS600		55	/* Furuno HS-600 - 55 */
#define	MBSYS_HYSWEEP_SONAR_FURUNO_HS600F		56	/* Furuno HS-600F - 56 */
#define	MBSYS_HYSWEEP_SONAR_TRITECH_STARFISH		57	/* Tritech Starfish - 57 */
#define	MBSYS_HYSWEEP_SONAR_RESON_8150			58	/* Reson 8150 - 58 */
#define	MBSYS_HYSWEEP_SONAR_RESON_8160			59	/* Reson 8160 - 59 */
#define	MBSYS_HYSWEEP_SONAR_RESON_7150			60	/* Reson 7150 - 60 */
#define	MBSYS_HYSWEEP_SONAR_EDGETECH_4600		61	/* Edgetech 4600 - 61 */
#define	MBSYS_HYSWEEP_SONAR_RESON_7111			62	/* Reson Seabat 7111 - 62 */
#define	MBSYS_HYSWEEP_SONAR_R2SONIC_SONIC2024		63	/* R2Sonic SONIC 2024 - 63 */
#define	MBSYS_HYSWEEP_SONAR_MDL_DYNASCAN		64	/* MDL Dynascan - 64 */
#define	MBSYS_HYSWEEP_SONAR_WASSP_MULTIBEAM		65	/* WASSP Multibeam - 65 */
#define	MBSYS_HYSWEEP_SONAR_ATLAS_HYDROSWEEPMD50	66	/* Atlas Hydrosweep  MD/50 - 66 */
#define	MBSYS_HYSWEEP_SONAR_ATLAS_HYDROSWEEPMD30	67	/* Atlas Hydrosweep  MD/30 - 67 */
#define	MBSYS_HYSWEEP_SONAR_ATLAS_HYDROSWEEPDS		68	/* Atlas Hydrosweep  DS - 68 */
#define	MBSYS_HYSWEEP_SONAR_INNOMAR_SES			69	/* Innomar SES - 69 */
#define	MBSYS_HYSWEEP_SONAR_SEABEAM_3012		70	/* SeaBeam 3012 - 70 */
#define	MBSYS_HYSWEEP_SONAR_SEABEAM_3020		71	/* SeaBeam 3020 - 71 */
#define	MBSYS_HYSWEEP_SONAR_SEABEAM_3050		72	/* SeaBeam 3050 - 72 */

#define	MBSYS_HYSWEEP_DEVICE_NUM_MAX	12
#define	MBSYS_HYSWEEP_OFFSET_NUM_MAX	12

#define MBSYS_HYSWEEP_MSS_NUM_PIXELS    1024
#define	MBSYS_HYSWEEP_MAXLINE	        32768

/* HYSWEEP device offset structure */
struct mbsys_hysweep_device_offset_struct
	{
	/* OF2: Hysweep device offsets
		OF2 dn on n1 n2 n3 n4 n5 n6 n7 */
	int	OF2_device_number;	/* device number */
	int	OF2_offset_type;	/* offset type
						0 - position antenna offsets
						1 - gyro heading offset
						2 - MRU device offsets
						3 - sonar head 1 / transducer 1 offsets
						4 - sonar head 2 / transducer 2 offsets
						........
						131 - transducer 128 offsets */
	double	OF2_offset_starboard;	/* starboard/port mounting offset, positive to starboard */
	double	OF2_offset_forward;	/* forward/aft mounting offset, positive to forward */
	double	OF2_offset_vertical;	/* vertical mounting offset, positive downward from waterline */
	double	OF2_offset_yaw;		/* yaw rotation angle, positive for clockwise rotation */
	double	OF2_offset_roll;	/* roll rotation angle, port side up is positive */
	double	OF2_offset_pitch;	/* pitch rotation angle, bow up is positive */
	double	OF2_offset_time;	/* time latency in seconds */
	};

/* HYSWEEP device structure */
struct mbsys_hysweep_device_struct
	{
	/* DEV: first line of device declaration
		DEV dn dc "name"
		DEV 0 544 "IXSEA OCTANS Serial" */
	int	DEV_device_number;
	int	DEV_device_capability;	/* Hypack device capabilities (bit code)
						1 - device provides range/range position
						2 - device provides range/azimuth position
						4 - device provides lat/long (e.g. GPS)
						8 - device provides grid positions XY
						16 - device provides echo soundings
						32 - device provides heading
						64 - device provides ship speed
						128 - Hypack clock is synched to device clock
						256 - device provides tide
						512 - device provides heave, pitch, and roll
						1024 - device is a ROV
						2048 - device is a left/right indicator
						4096 - device accepts annotations strings
						8192 - device accepts output from Hypack
						16384 - xxx
						32768 - device has extended capability
					NOTE: an MB-System extension to allow */
	mb_path	DEV_device_name;		/* e.g. "GPS" */

	/* DV2: second line of device declaration
		DV2 dn dc tf en
		DV2 0 220 0 1 */
	int	DV2_device_capability;	/* Hysweep device capabilities (bit coded hexadecimal)
						0001 - multibeam sonar
						0002 - multiple transducer sonar
						0004 - GPS (boat position)
						0008 - sidescan sonar
						0010 - single beam echosounder
						0020 - gyro (boat heading)
						0040 - tide
						0200 - MRU (heave, pitch, and roll compensation) */
	int	DV2_towfish;		/* 1 if device is mountedc on a tow fish */
	int	DV2_enabled;		/* 1 if device is enabled */

	/* OF2: Hysweep device offsets  */
	int	num_offsets;		/* number of offsets identified for this device */
	struct mbsys_hysweep_device_offset_struct offsets[MBSYS_HYSWEEP_OFFSET_NUM_MAX];

	/* PRI: device set as primary navigational device */
	int	PRI_primary_nav_device;	/* 1 if device is primary navigational device */

	/* MBI: multibeam / multiple transducer device information
		MBI dn st sf db n1 n2 fa ai */
	int	MBI_sonar_id;		/* sonar id from table above, not part of MBI record
						but instead inferred from device name in the
						corresponding DEV record */
	int	MBI_sonar_receive_shape;/* sonar receive head shape, not part of MBI record
						but instead inferred from device name in the
						corresponding DEV record
						0 - flat
						1 - circular */
	int	MBI_sonar_type;		/* sonar type code:
						0 - invalid
						1 - fixed beam roll angles (e.g. Reson Seabat )
						2 - variable beam roll angles (e.g. Seabeam SB1185)
						3 - beam info in spherical coordinates (e.g. Simrad EM3000)
						4 - multiple transducer (e.g. Odom Miniscan) */
	int	MBI_sonar_flags;	/* sonar flags (bit coded hexadecimal)
						0x0001 - roll corrected by sonar
						0x0002 - pitch corrected by sonar
						0x0004 - dual head
						0x0008 - heading corrected by sonar (ver 1)
						0x0010 - medium depth: slant ranges recorded to 1 dm
								resolution (version 2)
						0x0020 - deep water: slant ranges divided by 1 m
								resolution (ver 2)
						0x0040 - SVP corrected by sonar (ver 5)
						0x0080 - topographic device, upgoing beams accepted (ver 6) */
	int	MBI_beam_data_available;/* beam data (bit coded hexadecimal)
						0x0001 - beam ranges are available (survey units)
						0x0002 - sounding point casting available (survey units)
						0x0004 - point northing available (survey units)
						0x0008 - point corrected depth available (survey units)
						0x0010 - along track distance available (survey units)
						0x0020 - across track distance available (survey units)
						0x0040 - beam pitch angles available (degrees, TSS convention)
						0x0080 - beam roll angles available (degrees, TSS convention)
						0x0100 - beam takeoff angles available (degrees from vertical)
						0x0200 - beam direction angles available (degrees from forward)
						0x0400 - ping delay times included (milliseconds)
						0x0800 - beam intensity data available
						0x1000 - beam quality codes (from sonar unit) available
						0x2000 - sounding flags included
						0x4000 - spare
						0x8000 - spare */
	int	MBI_num_beams_1;	/* number of beams, head 1 (multibeam) or number of transducers
						(multitransducer) */
	int	MBI_num_beams_2;	/* number of beams, head 2 (multibeam) */
	double	MBI_first_beam_angle;	/* first beam angle is for sonar type = fixed angle
						(degrees, TSS convention) */
	double	MBI_angle_increment;	/* angle increment is for sonare type = fixed angle
						(degrees, TSS convention) */

	/* SSI: sidescan device information */
	int	SSI_sonar_flags;	/* sonar flags (bit coded hexadecimal)
						0x0100 - amplitude is bit-shifted into byte storage */
	int	SSI_port_num_samples;	/* number of samples per ping, port transducer */
	int	SSI_starboard_num_samples;	/* number of samples per ping, starboard transducer */

	};

/* internal data structure */
struct mbsys_hysweep_struct
	{
	/* MBIO kind of current data record */
	int	kind;

	/* MBIO time stamp of current data record */
	int	time_i[7];
	double	time_d;

        /* Char array used to read data */
        char    readline[MBSYS_HYSWEEP_MAXLINE];
        char    writeline[MBSYS_HYSWEEP_MAXLINE];

	/* HYSWEEP type of current data record */
	int	type;			/* HYSWEEP current data record type */

	/* HYSWEEP file header records */

	/* FTP - first record of header - file type identifier
		Always:
		FTP NEW 2 */
	mb_name	FTP_record;		/* FTP NEW 2: HYSWEEP file type */

	/* HSX - HSX format version
		HSX vr
			vr: version id
		Example:
		HSX 7 */
	int	HSX_record;		/* HSX: HYSWEEP format version number
						0: HYPACK Max 0.4 29-Mar-2000
						1: HYPACK Max 0.5 11-Sep-2000
						2: HYPACK Max 0.5b 18-Jun-2001
						3: HYPACK Max 2.12A 05-Jun-2003 */

	/* VER - HYSWEEP version
		VER vr
			vr: version string
		Example:
		VER 10.0.7.0 */
	mb_name	VER_version;		/* VER: HYSWEEP distribution version */

	/* TND - Survey time and date
		TND t d
			t: time string
			d: date string
		Example:
		TND 22:21:31 11/27/2010 */
	int	TND_survey_time_i[7];	/* TND: Survey time and date (system startup)
						hh:mm:ss mm/dd/yy */
	double	TND_survey_time_d;	/* TND: Survey time */

	/* INF - General information
		INF "surveyor" "boat" "project" "area" tc dc sv
			tc: initial tide correction
			dc: initial draft correction
			sv: sound velocity
		Example:
		INF "" "" "" "" 0.00 0.00 1500.00 */
	mb_name	INF_surveyor;		/* INF: surveyor name */
	mb_name	INF_boat;		/* INF: boat name */
	mb_name	INF_project;		/* INF: project name */
	mb_name	INF_area;		/* INF: area name */
	double	INF_tide_correction;	/* INF: initial tide correction */
	double	INF_draft_correction;	/* INF: initial draft correction */
	double	INF_sound_velocity;	/* INF: initial sound velocity */

	/* PRJ - Projected coordinate system definition - MB-System extension
		PRJ pr
			pr: projection system definition string
				- either a PROJ4 command string or an MB-System
				projection ID enclosed in <>. For instance, the
				UTM projection for zone 10 north can be defined using
				any of the following examples:
		Example:
		PRJ <UTM10N>
		PRJ <epsg32610>
		PRJ +proj=utm +zone=10 +ellps=WGS84 +datum=WGS84 +units=m +no_defs */
	mb_path	PRJ_proj4_command;	/* PRJ: projection in use defined as either
						a PROJ4 command string or as an
						EPSG identifier */

	/* EOH - end file header records */

	/* HSP - Hysweep survey parameters
		HSP p1 p2 p3 p4 p5 p6 p7 p8 p9 p10 p11 p12
			p1: minimum depth in work units
			p2: maximum depth in work units
			p3: port side offset limit
			p4: starboard side offset limit
			p5: port side angle limit in degrees
			p6: starboard side angle limit in degrees
			p7: high beam quality; codes >= this are good
			p8: low beam quality; codes < this are bad
			p9: sonar range setting in work units
			p10: towfish layback in work units
			p11: work units:
						0: = meters
						1 = US foot
						2 = international footin work units
			p12: sonar id for advanced processing (see defines above)
		Example:
		HSP 0.00 95.00 35.62 35.62 75 75 1 1 0.50 0.00 0 0 */
	double	HSP_minimum_depth;	/* HSP: minimum depth in work units */
	double	HSP_maximum_depth;	/* HSP: maximum depth in work units */
	double	HSP_port_offset_limit;	/* HSP: port side offset limit in work units */
	double	HSP_stbd_offset_limit;	/* HSP: starboard side offset limit in work units */
	double	HSP_port_angle_limit;	/* HSP: port side angle limit in degrees */
	double	HSP_stbd_angle_limit;	/* HSP: starboard side angle limit in degrees */
	int	HSP_high_beam_quality;	/* HSP: high beam quality; codes >= this are good */
	int	HSP_low_beam_quality;	/* HSP: low beam quality; codes < this are bad */
	double	HSP_sonar_range;	/* HSP: sonar range setting in work units */
	double	HSP_towfish_layback;	/* HSP: towfish layback in work units */
	int	HSP_units;		/* HSP: work units:
						0: = meters
						1 = US foot
						2 = international foot */
	int	HSP_sonar_id;		/* HSP: sonar id for advanced processing (see defines above) */

	/* HYSWEEP devices */
	int	num_devices;		/* number of devices defined */
	struct mbsys_hysweep_device_struct devices[MBSYS_HYSWEEP_DEVICE_NUM_MAX];
	int	primary_nav_device;	/* device number of primary navigational device */

	/* HYSWEEP HVF - Hysweep view filters - always first record after end of file header
		HVF dn tt p1 p2 p3 p4 p5 p6
			dn: dummy device number, always = 99
			tt: time tag this filter set became active (in seconds past midnight)
			p1: minimum depth in work units
			p2: maximum depth in work units
			p3: port side offset limit in work units
			p4: starboard side offset limit in work units
			p5: minimum beam angle limit, 0 to -90 degrees
			p6: maximum beam angle limit, 0 to 90 degrees
		Example:
		HVF 99 80491.965 0.0 95.0 35.6 35.6 -75.0 75.0 */
	double	HVF_time_after_midnight;	/* time this filter set became active
							seconds after midnight */
	double	HVF_minimum_depth;	/* minimum depth in work units */
	double	HVF_maximum_depth;	/* maximum depth in work units */
	double	HVF_port_offset_limit;	/* port side offset limit in work units */
	double	HVF_starboard_offset_limit;	/* starboard side offset limit in work units */
	double	HVF_minimum_angle_limit;	/* minimum beam angle limit, 0 to -90 degrees */
	double	HVF_maximum_angle_limit;	/* maximum beam angle limit, 0 to 90 degrees */

	/* HYSWEEP FIX - fix (event) mark - always second record after end of file header
		HVF dn t n */
	int	FIX_device_number;		/* device number */
	double	FIX_time_after_midnight;	/* time in seconds after midnight */
	int	FIX_event_number;		/* FIX event number */

	/* HYSWEEP RMB - raw multibeam data
		RMB dn t st sf bd n sv pn psa
			dn: device number 
			t: time tag (seconds past midnight) 
			st: sonar type code (see MBI above) 
			sf: sonar flags (see MBI above) 
			bd: available beam data (see MBI above) 
			n: number of beams to follow 
			sv: sound velocity in m/sec 
			pn: ping number (or 0 if not tracked)

		Immediately following the RMB record is a record containing slant ranges (multibeam) 
		or raw depths (multiple transducer). Following the ranges are 0 to n additional records 
		depending on the bd (beam data) field.

		Example (Seabat 9001 storing slant ranges and quality codes): 
			RMB 1 27244.135 1 0 1001 1500.00 0 60 
			19.50 19.31 18.60 1.66 18.47 ... (60 slant ranges in work units) 
			3 3 3 0 3 ... (60 quality codes)

		Example (multiple transducer storing 8 raw depths): 
			RMB 1 27244.135 4 0 1 1500.00 0 60 
			31.44 33.01 32.83 32.80 ... (8 raw depths in work units)

		Example (Dual-head Seabeam SB1185 storing range, beam pitch and roll angles, 
			ping delay times, beam quality code and sounding flags): 
			RMB 1 27244.135 2 5 1481 1500.00 0 
			108 93.18 88.30 84.74 80.46 ... (108 slant ranges in working units)
			-69.72 -68.53 -67.36 -66.15 ... (108 beam roll angles in degrees) 
			0 0 0 67 ... (108 ping delay times in msecs) 
			7 7 7 7 ... (108 beam quality codes) */
	int	RMB_device_number;	/* device number */
	double	RMB_time;		/* time tag (seconds past midnight) */
	int	RMB_sonar_type;		/* sonar type code:
						0 - invalid
						1 - fixed beam roll angles (e.g. Reson Seabat )
						2 - variable beam roll angles (e.g. Seabeam SB1185)
						3 - beam info in spherical coordinates (e.g. Simrad EM3000)
						4 - multiple transducer (e.g. Odom Miniscan) */
	int	RMB_sonar_flags;	/* sonar flags (bit coded hexadecimal)
						0x0001 - roll corrected by sonar
						0x0002 - pitch corrected by sonar
						0x0004 - dual head
						0x0008 - heading corrected by sonar (ver 1)
						0x0010 - medium depth: slant ranges recorded to 1 dm
								resolution (version 2)
						0x0020 - deep water: slant ranges divided by 1 m
								resolution (ver 2)
						0x0040 - SVP corrected by sonar (ver 5)
						0x0080 - topographic device, upgoing beams accepted (ver 6) */
	int	RMB_beam_data_available;/* beam data (bit coded hexadecimal)
						0x0001 - beam ranges are available (survey units)
						0x0002 - sounding point casting available (survey units)
						0x0004 - point northing available (survey units)
						0x0008 - point corrected depth available (survey units)
						0x0010 - along track distance available (survey units)
						0x0020 - across track distance available (survey units)
						0x0040 - beam pitch angles available (degrees, TSS convention)
						0x0080 - beam roll angles available (degrees, TSS convention)
						0x0100 - beam takeoff angles available (degrees from vertical)
						0x0200 - beam direction angles available (degrees from forward)
						0x0400 - ping delay times included (milliseconds)
						0x0800 - beam intensity data available
						0x1000 - beam quality codes (from sonar unit) available
						0x2000 - sounding flags included
						0x4000 - spare
						0x8000 - spare */
	int	RMB_num_beams;		/* number of beams to follow */
	int	RMB_num_beams_alloc;	/* number of beams allocated to non-null arrays */
	double	RMB_sound_velocity;	/* sound velocity in m/sec */
	int	RMB_ping_number;	/* ping number (or 0 if not tracked) */
	double	*RMB_beam_ranges;	/* beam ranges (survey units) */
	double	*RMB_multi_ranges;	/* sounding point casting (survey units) (multi-transducer rangers) */
	double	*RMB_sounding_eastings;	/* easting positions of soundings */
	double	*RMB_sounding_northings;/* northing positions of soundings */
	double	*RMB_sounding_depths;	/* corrected depths of soundings */
	double	*RMB_sounding_across;	/* acrosstrack positions of soundings */
	double	*RMB_sounding_along;	/* alongtrack positions of soundings */
	double	*RMB_sounding_pitchangles;	/* beam pitch angles of soundings (degrees, TSS convention) */
	double	*RMB_sounding_rollangles;	/* beam roll angles of soundings (degrees, TSS convention) */
	double	*RMB_sounding_takeoffangles;	/* beam takeoff angles of soundings (degrees from vertical) */
	double	*RMB_sounding_azimuthalangles;	/* beam azimuthal angles of soundings (degrees from forward) */
	int	*RMB_sounding_timedelays;	/* beam delay times (milliseconds) */
	int	*RMB_sounding_intensities;	/* beam intensities */
	int	*RMB_sounding_quality;		/* beam quality codes (from sonar unit) */
	int	*RMB_sounding_flags;		/* beam edit flags */

	/* Interpolated position and attitude data for multibeam ping
		In the case of data previously handled by MB-System this
		will derive from HCP, GYR, POS, DFT records with the same time
		stamp as the multibeam RMB record that will be placed
		immediately before the RMB record in the file.
		In the case of data not previously handled by MB-System these
		values will be extracted by interpolation at the time the
		RMB record is read. Subsequent data writing will have the
		interpolated records placed before the RMB records, and will
		have the interpolated HCP, GYR, POS, and DFT records set
		as "enabled". Any original asynchronous HCP, GYR, POS, and
		DFT records will have the "enabled" flag unset. Four new
		devices will be declared as well. */
	double	RMBint_heave;		/* heave (meters) */
	double	RMBint_roll;		/* roll (+ port side up) */
	double	RMBint_pitch;		/* pitch (+ bow up) */
	double	RMBint_heading;		/* heading (degrees) */
	double	RMBint_x;		/* easting */
	double	RMBint_y;		/* northing */
	double	RMBint_lon;		/* longitude (degrees) */
	double	RMBint_lat;		/* latitude (degrees) */
	double	RMBint_draft;		/* draft correction */
	double	RMBint_tide;		/* tide correction */

	/* RSS - Raw Sidescan
		RSS dn t sf np ns sv pn alt sr amin amax bs freq
			dn: device number
			t: time tag (seconds past midnight)
			sf: sonar flags (bit coded hexadecimal)
				0100 - amplitude is bit-shifted into byte storage
			np: number of samples, port transducer (down-sampled to 4096 max)
			ns: number of samples, starboard transducer (down-sampled to 4096 max)
			sv: sound velocity in m/sec
			pn: ping number (or 0 if not tracked)
			alt: altitude in work units
			sr: sample rate (samples per second after down-sample)
			amin: amplitude minimum
			amax: amplitude maximum
			bs: bit shift for byte recording
			freq: frequency 0 or 1 for simultaneous dual frequency operation

		Immediately following the RSS record are two records containing port and
		starboard amplitude samples.

		Example:
		RSS 3 61323.082 100 341 341 1460.00 0 10.75 4983.47 0 4096 4 0
		109 97 84 95 120 111 ... (341 port samples)
		106 93 163 106 114 127 ... (341 starboard samples) */
	int	RSS_device_number;	/* device number */
	double	RSS_time;		/* time tag (seconds past midnight) */
	int	RSS_sonar_flags;	/* sonar flags:
						0100 - amplitude is bit-shifted into byte storage */
	int	RSS_port_num_samples; 	/* number of samples, port transducer (down-sampled to 4096 max) */
	int	RSS_port_num_samples_alloc; 	/* number of samples, port transducer (down-sampled to 4096 max) */
	int	RSS_starboard_num_samples; 	/* number of samples, starboard transducer
						(down-sampled to 4096 max) */
	int	RSS_starboard_num_samples_alloc; 	/* number of samples, starboard transducer
						(down-sampled to 4096 max) */
	double	RSS_sound_velocity;	/* sound velocity in m/sec  */
	int	RSS_ping_number;	/* ping number (or 0 if not tracked) */
	double	RSS_altitude;		/* altitude in work units  */
	double	RSS_sample_rate;	/* sample rate (samples per second after down-sample)  */
	int	RSS_minimum_amplitude;	/* amplitude minimum  */
	int	RSS_maximum_amplitude;	/* amplitude maximum  */
	int	RSS_bit_shift;		/* bit shift for byte recording  */
	int	RSS_frequency;		/* frequency 0 or 1 for simultaneous dual frequency operation */
	int	*RSS_port;		/* port sidescan amplitude samples */
	int	*RSS_starboard;		/* starboard sidescan amplitude samples */

        /* MSS - MB-System Sidescan
                MSS dn t pn n ps
			dn: device number
			t: time tag (seconds past midnight)
			pn: ping number (or 0 if not tracked)
			n: number of pixels
			ps: pixel size in meters

		Immediately following the MSS record is one record containing
		the amplitude samples. Zero samples are null values
		indicating no data.

		Example:
		MSS 3 61323.082 1500.00 1024 0.340 27535
		0.00 0.00 109.25 97.13 .... 95.34 120.76 111.26 0.00 (1024 samples) */
	int	MSS_device_number;	/* device number */
	double	MSS_time;		/* time tag (seconds past midnight) */
	double	MSS_sound_velocity;	/* sound velocity in m/sec  */
	int	MSS_num_pixels;	        /* number of pixels (typically 1024) */
	double	MSS_pixel_size;	        /* pixel size (meters) */
	int	MSS_ping_number;	/* ping number (or 0 if not tracked) */
        double  MSS_ss[MBSYS_HYSWEEP_MSS_NUM_PIXELS];           /* processed sidescan */
        int     MSS_ss_cnt[MBSYS_HYSWEEP_MSS_NUM_PIXELS];           /* processed sidescan */
        double  MSS_ss_across[MBSYS_HYSWEEP_MSS_NUM_PIXELS];    /* processed sidescan acrosstrack (meters) */
        double  MSS_ss_along[MBSYS_HYSWEEP_MSS_NUM_PIXELS];     /* processed sidescan alongtrack (meters) */
        int     MSS_table_num_alloc;        /* sidescan working array allocated dimensions */
        double  *MSS_table_altitude_sort;   /* sidescan working array - altitude = depth - draft + heave */
        double  *MSS_table_range;           /* sidescan working array - range twtt (seconds) */
        double  *MSS_table_acrosstrack;     /* sidescan working array - acrosstrack (meters) */
        double  *MSS_table_alongtrack;      /* sidescan working array - alongtrack (meters) */

	/* SNR - dynamic sonar settings
		up to 12 fields depending on sonar type
		SNR dn t pn sonar ns s0 --- s11
			dn: device number 
			t: time tag (seconds past midnight) 
			pn: ping number (or 0 if not tracked)
			sonar: sonar ID code (see defines above) 
			ns: number of settings to follow
			s: up to 12 settings

		Up to 12 fields are included in SNR records, providing sonar runtime settings.
		Not available for all systems. Defined differently depending on sonar model
		and manufacturer.

		For Seabat 81XX Serial and 81XX Network Drivers:
			Sonar id: 1, 23, 24, 25, 39
			P0: Sonar range setting in meters.
			P1: power setting, 0 - 8
			P2: gain setting, 1 Ð 45
			P3: gain modes: bit 0 = TVG on/off, bit 1 = auto gain on/off.

		For Seabat 7K drivers (7125, 7101, 7150, 7111)
			Sonar id: 22, 53, 60, 62
			P0: Sonar range selection in meters.
			P1: Transmit power selection in dBs relative to 1 uPa.
			P2: Receiver gain selection in 0.1 dBs.
			P3: Transmitter frequency in KHz.
			P4: Transmit pulse width in microseconds.

		For EdgeTech 4200 Driver
			Sonar id: 7-10
			P0: Pulse power setting, 0 to 100 percent.
			P1: ADC Gain factor.
			P2: Start Frequency in 10 * Hz.
			P3: End Frequency in 10 * Hz.
			P4: Sweep length in milliseconds. */
	int	SNR_device_number;	/* device number */
	double	SNR_time;		/* time tag (seconds past midnight) */
	int	SNR_ping_number;	/* ping number (or 0 if not tracked) */
	int	SNR_sonar_id;		/* sonar ID (see defines above) */
	int	SNR_num_settings;	/* number of settings to follow */
	double	SNR_settings[12];	/* sonar settings */

	/* PSA - pitch stabilization angle
		PSA dn t pn a0 a1
			dn: device number 
			t: time tag (seconds past midnight) 
			pn: ping number (or 0 if not tracked)
			a0: projector (head 0) pitch angle
			a1: projector (head 1) pitch angle

		Note: PSA records are recorded only when pitch stabilization
		is active. They immediately proceed corresponding RMB records. */
	int	PSA_device_number;	/* device number */
	double	PSA_time;		/* time tag (seconds past midnight) */
	int	PSA_ping_number;	/* ping number (or 0 if not tracked) */
	double	PSA_a0;			/* projector (head 0) pitch angle */
	double	PSA_a1;			/* projector (head 1) pitch angle */

	/* HCP - heave compensation
		HCP dn t h r p
			dn: device number
			t: time tag (seconds past midnight)
			h: heave in meters
			r: roll in degrees (+ port side up)
			p: pitch in degrees (+ bow up)
		Example:
		HCP 0 80492.021 -0.65 -0.51 -1.73 */
	int	HCP_device_number;	/* device number */
	double	HCP_time;		/* time tag (seconds past midnight) */
	double	HCP_heave;		/* heave (meters) */
	double	HCP_roll;		/* roll (+ port side up) */
	double	HCP_pitch;		/* pitch (+ bow up) */

	/* EC1 - echo sounding (single frequency)
		EC1 dn t rd
			dn: device number
			t: time tag (seconds past midnight)
			rd: raw depth
		Example:
		EC1 1 80491.897 99.02 */
	int	EC1_device_number;	/* device number */
	double	EC1_time;		/* time tag (seconds past midnight) */
	double	EC1_rawdepth;		/* raw depth */

	/* GPS - GPS measurements
		GPS dn t cog sog hdop mode nsats
			dn: device number
			t: time tag (seconds past midnight)
			cog: course over ground (degrees)
			sog: speed over ground (knots)
			hdop: GPS hdop
			mode: GPS mode
				0 - unknown
				1 - stand alone
				2 - differential
				3 - rtk
			nsats: number of satellites
		Example:
		GPS 1 80491.897 178.16 0.23 1.2 2 8 */
	int	GPS_device_number;	/* device number */
	double	GPS_time;		/* time tag (seconds past midnight) */
	double	GPS_cog;		/* course over ground (degrees) */
	double	GPS_sog;		/* speed over ground (knots) */
	double	GPS_hdop;		/* GPS hdop */
	int	GPS_mode;		/* GPS mode
						0 - unknown
						1 - stand alone
						2 - differential
						3 - rtk */
	int	GPS_nsats;		/* number of satellites */

	/* GYR - gyro data (heading)
		GYR dn t h
			dn: device number
			t: time tag (seconds past midnight)
			h: heading (degrees)
		Example:
		GYR 1 80491.897 178.16 */
	int	GYR_device_number;	/* device number */
	double	GYR_time;		/* time tag (seconds past midnight) */
	double	GYR_heading;		/* heading (degrees) */

	/* POS - position
		POS dn t x y
			dn: device number
			t: time tag (seconds past midnight)
			x: easting
			y: northing
		Example:
		POS 1 80491.897 308214.82 1414714.97 */
	int	POS_device_number;	/* device number */
	double	POS_time;		/* time tag (seconds past midnight) */
	double	POS_x;			/* easting */
	double	POS_y;			/* northing */

	/* DFT - dynamic draft (squat) correction
		DFT dn t dc
			dn: device number
			t: time tag (seconds past midnight)
			dc: draft correction
		Example:
		DFT 1 80491.897 1453.44 */
	int	DFT_device_number;	/* device number */
	double	DFT_time;		/* time tag (seconds past midnight) */
	double	DFT_draft;		/* draft correction */

	/* TID - tide correction
		TID dn t tc
			dn: device number
			t: time tag (seconds past midnight)
			tc: tide correction
		Example:
		TID 1 80491.897 0.00 */
	int	TID_device_number;	/* device number */
	double	TID_time;		/* time tag (seconds past midnight) */
	double	TID_tide;		/* tide correction */

	/* COM - comment record - MB-System extension
		COM c
			c: comment string
		Example:
		COM everything after the first four characters is part of the comment */
	char	COM_comment[MB_COMMENT_MAXLINE];	/* comment string */
	};

/* system specific function prototypes */
int mbsys_hysweep_alloc(int verbose, void *mbio_ptr, void **store_ptr,
			int *error);
int mbsys_hysweep_deall(int verbose, void *mbio_ptr, void **store_ptr,
			int *error);
int mbsys_hysweep_dimensions(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_hysweep_sonartype(int verbose, void *mbio_ptr, void *store_ptr,
                        int *sonartype, int *error);
int mbsys_hysweep_sidescantype(int verbose, void *mbio_ptr, void *store_ptr,
                        int *ss_type, int *error);
int mbsys_hysweep_pingnumber(int verbose, void *mbio_ptr,
		int *pingnumber, int *error);
int mbsys_hysweep_extract(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp,
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_hysweep_insert(int verbose, void *mbio_ptr, void *store_ptr,
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp,
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_hysweep_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles,
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset,
			double *draft, double *ssv, int *error);
int mbsys_hysweep_detects(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams, int *detects, int *error);
int mbsys_hysweep_gains(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transmit_gain, double *pulse_length,
			double *receive_gain, int *error);
int mbsys_hysweep_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude,
			int *error);
int mbsys_hysweep_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft,
			double *roll, double *pitch, double *heave,
			int *error);
int mbsys_hysweep_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft,
			double roll, double pitch, double heave,
			int *error);
int mbsys_hysweep_copy(int verbose, void *mbio_ptr,
			void *store_ptr, void *copy_ptr,
			int *error);
int mbsys_hysweep_makess(int verbose, void *mbio_ptr, void *store_ptr,
                        int pixel_size_set, double *pixel_size,
                        int swath_width_set, double *swath_width,
                        int pixel_int, int *error);
