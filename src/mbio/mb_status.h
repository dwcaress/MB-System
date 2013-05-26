/*--------------------------------------------------------------------
 *    The MB-system:	mbio_status.h	2/1/93
 *    $Id$
 *
 *    Copyright (c) 1993-2013 by
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
 * mbio_status.h defines version, status and error codes used
 * by MBIO functions and programs
 *
 * Author:	D. W. Caress
 * Date:	January 19, 1993
 *
 * $Log: mb_status.h,v $
 *
 *
 */

/* include this code only once */
#ifndef MB_STATUS_DEF
#define MB_STATUS_DEF

/* MB-system version id */
#define	MB_VERSION	"5.4.2083"
#define	MB_BUILD_DATE	"26 May 2013"
#define	MB_SVN		"$Id$"

/* MBIO function boolean convention */
#define	MB_YES	        1
#define	MB_NO	        0
#define	MB_MAYBE	-1

/* MBIO sonar types */
#define	MB_SONARTYPE_UNKNOWN		0
#define	MB_SONARTYPE_ECHOSOUNDER	1
#define	MB_SONARTYPE_MULTIBEAM		2
#define	MB_SONARTYPE_SIDESCAN		3
#define	MB_SONARTYPE_INTERFEROMETRIC	4

/* MBIO data type ("kind") convention */
#define	MB_DATA_KINDS			59
#define	MB_DATA_NONE			0
#define	MB_DATA_DATA			1	/* general survey data */
#define	MB_DATA_COMMENT			2	/* general comment */
#define	MB_DATA_HEADER			3	/* general header */
#define	MB_DATA_CALIBRATE		4	/* Hydrosweep DS */
#define	MB_DATA_MEAN_VELOCITY		5	/* Hydrosweep DS */
#define	MB_DATA_VELOCITY_PROFILE	6	/* general */
#define	MB_DATA_STANDBY			7	/* Hydrosweep DS */
#define	MB_DATA_NAV_SOURCE		8	/* Hydrosweep DS */
#define	MB_DATA_PARAMETER		9	/* general */
#define	MB_DATA_START			10	/* Simrad */
#define	MB_DATA_STOP			11	/* Simrad */
#define	MB_DATA_NAV			12	/* Simrad, Reson 7k */
#define	MB_DATA_RUN_PARAMETER		13	/* Simrad */
#define	MB_DATA_CLOCK			14	/* Simrad */
#define	MB_DATA_TIDE			15	/* Simrad, Reson 7k */
#define	MB_DATA_HEIGHT			16	/* Simrad */
#define	MB_DATA_HEADING			17	/* Simrad, Hypack */
#define	MB_DATA_ATTITUDE		18	/* Simrad, Hypack, Reson 7k */
#define	MB_DATA_SSV			19	/* Simrad */
#define	MB_DATA_ANGLE			20	/* HSMD */
#define	MB_DATA_EVENT			21	/* HSMD */
#define	MB_DATA_HISTORY			22	/* GSF */
#define	MB_DATA_SUMMARY			23	/* GSF */
#define	MB_DATA_PROCESSING_PARAMETERS	24	/* GSF */
#define	MB_DATA_SENSOR_PARAMETERS	25	/* GSF */
#define	MB_DATA_NAVIGATION_ERROR	26	/* GSF */
#define	MB_DATA_RAW_LINE		27	/* uninterpretable line for ascii formats */
#define	MB_DATA_NAV1			28	/* ancillary nav system 1 */
#define	MB_DATA_NAV2			29	/* ancillary nav system 2 */
#define	MB_DATA_NAV3			30	/* ancillary nav system 3 */
#define	MB_DATA_TILT			31	/* Simrad */
#define	MB_DATA_MOTION			32	/* Reson 7k */
#define	MB_DATA_CTD			33	/* Reson 7k */
#define	MB_DATA_SUBBOTTOM_MCS		34	/* Reson 7k */
#define	MB_DATA_SUBBOTTOM_CNTRBEAM	35	/* Simrad */
#define	MB_DATA_SUBBOTTOM_SUBBOTTOM	36	/* Reson 7k, XTF */
#define	MB_DATA_SIDESCAN2		37	/* Reson 7k, XTF */
#define	MB_DATA_SIDESCAN3		38	/* Reson 7k, XTF */
#define	MB_DATA_IMAGE			39	/* Reson 7k */
#define	MB_DATA_ROLL			40	/* Reson 7k */
#define	MB_DATA_PITCH			41	/* Reson 7k */
#define	MB_DATA_ABSORPTIONLOSS		42	/* Reson 7k */
#define	MB_DATA_SPREADINGLOSS		43	/* Reson 7k */
#define	MB_DATA_INSTALLATION		44	/* Reson 7k */
#define	MB_DATA_WATER_COLUMN		45	/* Simrad */
#define	MB_DATA_STATUS			46	/* Simrad, XTF */
#define	MB_DATA_DVL			47	/* JSTAR */
#define	MB_DATA_NMEA_RMC		48	/* NMEA */
#define	MB_DATA_NMEA_DBT		49	/* NMEA */
#define	MB_DATA_NMEA_DPT		50	/* NMEA */
#define	MB_DATA_NMEA_ZDA		51	/* NMEA */
#define	MB_DATA_NMEA_GLL		52	/* NMEA */
#define	MB_DATA_NMEA_GGA		53	/* NMEA */
#define	MB_DATA_SURVEY_LINE		54	/* Reson 7k */
#define	MB_DATA_ATTITUDE1		55	/* ancillary attitude system 1 */
#define	MB_DATA_ATTITUDE2		56      /* ancillary attitude system 2 */
#define	MB_DATA_ATTITUDE3		57	/* ancillary attitude system 3 */
#define	MB_DATA_SONARDEPTH		58	/* HYSWEEP dynamic draft */
#define	MB_DATA_ALTITUDE		59	/* HYSWEEP single beam echosounder */

/* MBIO function status convention */
#define	MB_SUCCESS			1
#define	MB_FAILURE			0

/* MBIO minimum and maximum error values */
#define	MB_ERROR_MIN			-23
#define	MB_ERROR_MAX			15

/* MBIO function fatal error values */
#define	MB_ERROR_NO_ERROR		0
#define	MB_ERROR_MEMORY_FAIL		1
#define	MB_ERROR_OPEN_FAIL		2
#define	MB_ERROR_BAD_FORMAT		3
#define	MB_ERROR_EOF			4
#define	MB_ERROR_WRITE_FAIL		5
#define	MB_ERROR_NONE_IN_BOUNDS		6
#define	MB_ERROR_NONE_IN_TIME		7
#define	MB_ERROR_BAD_DESCRIPTOR		8
#define	MB_ERROR_BAD_USAGE		9
#define	MB_ERROR_NO_PINGS_BINNED	10
#define	MB_ERROR_BAD_KIND		11
#define MB_ERROR_BAD_PARAMETER		12
#define	MB_ERROR_BAD_BUFFER_ID		13
#define	MB_ERROR_BAD_SYSTEM		14
#define	MB_ERROR_BAD_DATA		15

/* MBIO function nonfatal error values */
#define	MB_ERROR_TIME_GAP		-1
#define	MB_ERROR_OUT_BOUNDS		-2
#define	MB_ERROR_OUT_TIME		-3
#define	MB_ERROR_SPEED_TOO_SMALL	-4
#define	MB_ERROR_COMMENT		-5
#define	MB_ERROR_SUBBOTTOM		-6
#define	MB_ERROR_WATER_COLUMN		-7
#define	MB_ERROR_OTHER			-8
#define	MB_ERROR_UNINTELLIGIBLE		-9
#define	MB_ERROR_IGNORE			-10
#define	MB_ERROR_NO_DATA_REQUESTED	-11
#define	MB_ERROR_BUFFER_FULL		-12
#define	MB_ERROR_NO_DATA_LOADED		-13
#define	MB_ERROR_BUFFER_EMPTY		-14
#define	MB_ERROR_NO_DATA_DUMPED		-15
#define	MB_ERROR_NO_MORE_DATA		-16
#define	MB_ERROR_DATA_NOT_INSERTED	-17
#define	MB_ERROR_BAD_PROJECTION		-18
#define	MB_ERROR_MISSING_PROJECTIONS	-19
#define	MB_ERROR_MISSING_NAVATTITUDE	-20
#define	MB_ERROR_NOT_ENOUGH_DATA	-21
#define	MB_ERROR_FILE_NOT_FOUND		-22
#define	MB_ERROR_FILE_LOCKED		-23

/* MBIO problem values */
#define	MB_PROBLEM_MAX			6
#define	MB_PROBLEM_NO_DATA		1
#define	MB_PROBLEM_ZERO_NAV		2
#define	MB_PROBLEM_TOO_FAST		3
#define	MB_PROBLEM_AVG_TOO_FAST		4
#define	MB_PROBLEM_TOO_DEEP		5
#define	MB_PROBLEM_BAD_DATAGRAM		6

/* processing status values returned by mb_datalist_read2() */
#define MB_PROCESSED_NONE	0
#define MB_PROCESSED_EXIST	1
#define MB_PROCESSED_USE	2

/* MBIO maximum notice value */
#define	MB_NOTICE_MAX	(MB_DATA_KINDS - MB_ERROR_MIN + MB_PROBLEM_MAX + 1)

/* MBIO function error messages */
#ifdef DEFINE_MB_MESSAGES
static char *fatal_error_msg[] =
	{
	"No error",
	"Unable to allocate memory, initialization failed",
	"Unable to open file, initialization failed",
	"Illegal format identifier, initialization failed",
	"Read error, probably end-of-file",
	"Write error",
	"No data in specified location bounds",
	"No data in specified time interval",
	"Invalid mbio i/o descriptor",
	"Inconsistent usage of mbio i/o descriptor",
	"No pings binned but no fatal error - this should not happen!",
	"Invalid data record type specified for writing",
	"Invalid control parameter specified by user",
	"Invalid buffer id",
	"Invalid system id - this should not happen!",
	"This data file is not in the specified format!"
	};
static char *nonfatal_error_msg[] =
	{
	"No error",
	"Time gap in data",
	"Data outside specified location bounds",
	"Data outside specified time interval",
	"Ship speed too small",
	"Comment record",
	"Subbottom record",
	"Water column record",
	"Neither a data record nor a comment record",
	"Unintelligible data record",
	"Ignore these data",
	"No data requested for buffer load",
	"Data buffer is full",
	"No data was loaded into the buffer",
	"Data buffer is empty",
	"No data was dumped from the buffer",
	"No more survey data records in buffer",
	"Data inconsistencies prevented inserting data into storage structure",
	"UTM projection initialization failed",
	"Projection database cannot be read",
	"Missing navigation and/or attitude data",
	"Not enough data available to perform operation",
	"Requested file not found",
	"Requested file locked",
	};
static char *unknown_error_msg[] =
	{
	"Unknown error identifier"
	};

/* MBIO function notice messages */
static char *notice_msg[] =
	{
	"Unknown notice identifier junk",

	/* notices for data record types */
	"MB_DATA_DATA (ID=1): survey data",
	"MB_DATA_COMMENT (ID=2): comment",
	"MB_DATA_HEADER (ID=3): general header",
	"MB_DATA_CALIBRATE (ID=4): Hydrosweep DS calibration ping",
	"MB_DATA_MEAN_VELOCITY (ID=5): Hydrosweep DS mean sound speed",
	"MB_DATA_VELOCITY_PROFILE (ID=6): SVP",
	"MB_DATA_STANDBY (ID=7): Hydrosweep DS standby record",
	"MB_DATA_NAV_SOURCE (ID=8): Hydrosweep DS nav source record",
	"MB_DATA_PARAMETER (ID=9): Parameter record",
	"MB_DATA_START (ID=10): Simrad start datagram",
	"MB_DATA_STOP (ID=11): Simrad stop datagram",
	"MB_DATA_NAV (ID=12): Navigation record",
	"MB_DATA_RUN_PARAMETER (ID=13): Simrad runtime parameter datagram",
	"MB_DATA_CLOCK (ID=14): Simrad clock datagram",
	"MB_DATA_TIDE (ID=15): Tide record",
	"MB_DATA_HEIGHT (ID=16): Simrad height datagram",
	"MB_DATA_HEADING (ID=17): Heading record",
	"MB_DATA_ATTITUDE (ID=18): Attitude record",
	"MB_DATA_SSV (ID=19): Surface sound speed record",
	"MB_DATA_ANGLE (ID=20): Beam angle record",
	"MB_DATA_EVENT (ID=21): Hydrosweep MD event record",
	"MB_DATA_HISTORY (ID=22): GSF history record",
	"MB_DATA_SUMMARY (ID=23): GSF summary record",
	"MB_DATA_PROCESSING_PARAMETERS (ID=24): GSF processing parameters record",
	"MB_DATA_SENSOR_PARAMETERS (ID=25): GSF sensor parameter record",
	"MB_DATA_NAVIGATION_ERROR (ID=26): GSF navigation error record",
	"MB_DATA_RAW_LINE (ID=27): uninterpretable ASCII line",
	"MB_DATA_NAV1 (ID=28): Auxilliary nav system 1",
	"MB_DATA_NAV2 (ID=29): Auxilliary nav system 2",
	"MB_DATA_NAV3 (ID=30): Auxilliary nav system 3",
	"MB_DATA_TILT (ID=31): Mechanical tilt record",
	"MB_DATA_MOTION (ID=32): Motion (DVL) sensor record",
	"MB_DATA_CTD (ID=33): CTD record",
	"MB_DATA_SUBBOTTOM_MCS (ID=34): MCS subbottom record",
	"MB_DATA_SUBBOTTOM_CNTRBEAM (ID=35): Centerbeam subbottom record",
	"MB_DATA_SUBBOTTOM_SUBBOTTOM (ID=36): Subbottom record",
	"MB_DATA_SIDESCAN2 (ID=37): Secondary sidescan record",
	"MB_DATA_SIDESCAN3 (ID=38): Tertiary sidescan record",
	"MB_DATA_IMAGE (ID=39): Sonar image record",
	"MB_DATA_ROLL (ID=40): Roll record",
	"MB_DATA_PITCH (ID=41): Pitch record",
	"MB_DATA_ABSORPTIONLOSS (ID=42): Absorption loss record",
	"MB_DATA_SPREADINGLOSS (ID=43): Spreading loss record",
	"MB_DATA_INSTALLATION (ID=44): Installation parameter record",
	"MB_DATA_WATER_COLUMN (ID=45): Water column record",
	"MB_DATA_STATUS (ID=46): Status record",
	"MB_DATA_DVL (ID=47): DVL record",
	"MB_DATA_NMEA_RMC (ID=48): NMEA RMC record",
	"MB_DATA_NMEA_DBT (ID=49): NMEA DBT record",
	"MB_DATA_NMEA_DPT (ID=50): NMEA DPT record",
	"MB_DATA_NMEA_ZDA (ID=51): NMEA ZDA record",
	"MB_DATA_NMEA_GLL (ID=52): NMEA GLL record",
	"MB_DATA_NMEA_GGA (ID=53): NMEA GGA record",
	"MB_DATA_SURVEY_LINE (ID=54): Survey line record",
	"MB_DATA_ATTITUDE1 (55): ancillary attitude system 1",
	"MB_DATA_ATTITUDE2 (56): ancillary attitude system 2",
	"MB_DATA_ATTITUDE3 (57): ancillary attitude system 3",
	"MB_DATA_SONARDEPTH (58): HYSWEEP dynamic draft",
	"MB_DATA_ALTITUDE (59): HYSWEEP single beam echosounder",

	/* notices for nonfatal error messages */
	"MB_ERROR_TIME_GAP (ID=-1): Time gap in data",
	"MB_ERROR_OUT_BOUNDS (ID=-2): Data outside specified location bounds",
	"MB_ERROR_OUT_TIME (ID=-3): Data outside specified time interval",
	"MB_ERROR_SPEED_TOO_SMALL (ID=-4): Ship speed too small",
	"MB_ERROR_COMMENT (ID=-5): Comment record",
	"MB_ERROR_SUBBOTTOM (ID=-6): Subbottom record",
	"MB_ERROR_WATER_COLUMN (ID=-7): Water column record",
	"MB_ERROR_OTHER (ID=-8): Neither a data record nor a comment record",
	"MB_ERROR_UNINTELLIGIBLE (ID=-9): Unintelligible data record",
	"MB_ERROR_IGNORE (ID=-10): Ignore these data",
	"MB_ERROR_NO_DATA_REQUESTED (ID=-11): No data requested for buffer load",
	"MB_ERROR_BUFFER_FULL (ID=-12): Data buffer is full",
	"MB_ERROR_NO_DATA_LOADED (ID=-13): No data was loaded into the buffer",
	"MB_ERROR_BUFFER_EMPTY (ID=-14): Data buffer is empty",
	"MB_ERROR_NO_DATA_DUMPED (ID=-15): No data was dumped from the buffer",
	"MB_ERROR_NO_MORE_DATA (ID=-16): No more survey data records in buffer",
	"MB_ERROR_DATA_NOT_INSERTED (ID=-17): Data inconsistencies prevented inserting data into storage structure",
	"MB_ERROR_BAD_PROJECTION (ID=-18): UTM projection initialization failed",
	"MB_ERROR_MISSING_PROJECTIONS (ID=-19): Projection database cannot be read",
	"MB_ERROR_MISSING_NAVATTITUDE (ID=-20): Attitude data are missing for this ping",
	"MB_ERROR_NOT_ENOUGH_DATA (ID=-21): Not enough data to perform spline interpolation",
	"MB_ERROR_FILE_NOT_FOUND (ID=-22): Required file cannot be found",
	"MB_ERROR_FILE_LOCKED (ID=-23): Required file locked",

	/* problem notices */
	"DATA PROBLEM (ID=1): No survey data found",
	"DATA PROBLEM (ID=2): Zero longitude or latitude in survey data",
	"DATA PROBLEM (ID=3): Instantaneous speed exceeds 25 km/hr",
	"DATA PROBLEM (ID=4): Average speed exceeds 25 km/hr",
	"DATA PROBLEM (ID=5): Sounding depth exceeds 11000 m",
	"DATA PROBLEM (ID=6): Unsupported datagram or record",
	};
static char *unknown_notice_msg[] =
	{
	"Unknown notice identifier detritus"
	};
#endif

/* MBIO sidescan types
	- sidescan values can be logarithmic (dB) or linear (usually voltage) */
#define MB_SIDESCAN_LOGARITHMIC 0
#define MB_SIDESCAN_LINEAR	1

/* MBIO null sidescan:
	- value used to flag sidescan values as undefined */
#define MB_SIDESCAN_NULL -1000000000.0

/* MBIO unknown time flag:
	- time_d value used to flag unknown time tag
	- e.g. for xyz soundings */
#define MB_TIME_D_UNKNOWN -2209075200.000000

/*
 * The following defines the values used to flag or
 * select individual bathymetry values (soundings). This scheme
 * is very similar to the convention used in the HMPS
 * hydrographic data processing package and the SAIC Hydrobat
 * package. The values passed in MBIO functions are single
 * byte characters.
 *
 * Macros used to identify the flags are also defined here.
 *
 * The flagging scheme is as follows:
 *
 * Beams cannot be both flagged and selected. However, more than
 * one "reason bit" can be set for either flagging or selection.
 *
 * The flag and select bits:
 *   xxxxxx00 => This beam is neither flagged nor selected.
 *   xxxxxx01 => This beam is flagged as bad and should be ignored.
 *   xxxxxx10 => This beam has been selected.
 *
 * Flagging modes:
 *   00000001 => Flagged because no detection was made by the sonar.
 *   xxxxx101 => Flagged by manual editing.
 *   xxxx1x01 => Flagged by automatic filter.
 *   xxx1xx01 => Flagged because uncertainty exceeds 1 X IHO standard.
 *   xx1xxx01 => Flagged because uncertainty exceeds 2 X IHO standard.
 *   x1xxxx01 => Flagged because footprint is too large
 *   1xxxxx01 => Flagged by sonar as unreliable.
 *
 * Selection modes:
 *   00000010 => Selected, no reason specified.
 *   xxxxx110 => Selected as least depth.
 *   xxxx1x10 => Selected as average depth.
 *   xxx1xx10 => Selected as maximum depth.
 *   xx1xxx10 => Selected as location of sidescan contact.
 *   x1xxxx10 => Selected, spare.
 *   1xxxxx10 => Selected, spare.
 *
 */

/* Definitions for FLAG category */
#define MB_FLAG_NONE			0x00
#define MB_FLAG_FLAG			0x01
#define MB_FLAG_NULL			0x01
#define MB_FLAG_MANUAL			0x04
#define MB_FLAG_FILTER			0x08
#define MB_FLAG_FILTER2			0x10
#define MB_FLAG_GT_1X_IHO		0x10
#define MB_FLAG_GT_2X_IHO		0x20
#define MB_FLAG_FOOTPRINT       	0x40
#define MB_FLAG_SONAR			0x80

/* Definitions for the SELECT category */
#define MB_SELECT_SELECT		0x02
#define MB_SELECT_LEAST			0x04
#define MB_SELECT_MAXIMUM		0x08
#define MB_SELECT_AVERAGE		0x10
#define MB_SELECT_CONTACT		0x20
#define MB_SELECT_SPARE_1		0x40
#define MB_SELECT_SPARE_2		0x80

/* Definitions for macros applying and testing flags */
#define mb_beam_ok(F)				((int)(!(F & MB_FLAG_FLAG)))
#define mb_beam_check_flag(F)			((int)(F & MB_FLAG_FLAG))
#define mb_beam_check_flag_null(F)		((int)(F == MB_FLAG_NULL))
#define mb_beam_check_flag_manual(F)		((int)((F & MB_FLAG_MANUAL   ) && (F & MB_FLAG_FLAG)))
#define mb_beam_check_flag_filter(F)		((int)((F & MB_FLAG_FILTER   ) && (F & MB_FLAG_FLAG)))
#define mb_beam_check_flag_filter2(F)		((int)((F & MB_FLAG_FILTER2   ) && (F & MB_FLAG_FLAG)))
#define mb_beam_check_flag_gt_1x_iho(F)		((int)((F & MB_FLAG_GT_1X_IHO) && (F & MB_FLAG_FLAG)))
#define mb_beam_check_flag_gt_2x_iho(F)		((int)((F & MB_FLAG_GT_2X_IHO) && (F & MB_FLAG_FLAG)))
#define mb_beam_check_flag_footprint(F)		((int)((F & MB_FLAG_FOOTPRINT) && (F & MB_FLAG_FLAG)))
#define mb_beam_check_flag_sonar(F)		((int)((F & MB_FLAG_SONAR    ) && (F & MB_FLAG_FLAG)))
#define mb_beam_set_flag_null(F)		(MB_FLAG_NULL)
#define mb_beam_set_flag_manual(F)		(F | 0x05)
#define mb_beam_set_flag_filter(F)		(F | 0x09)
#define mb_beam_set_flag_filter2(F)		(F | 0x11)
#define mb_beam_set_flag_gt_1x_iho(F)		(F | 0x11)
#define mb_beam_set_flag_gt_2x_iho(F)		(F | 0x21)
#define mb_beam_set_flag_footprint(F)		(F | 0x41)
#define mb_beam_check_select(F)			((int)(F & MB_SELECT_SELECT))
#define mb_beam_check_select_least(F)		((int)((F & MB_SELECT_LEAST  ) && (F & MB_SELECT_SELECT)))
#define mb_beam_check_select_maximum(F)		((int)((F & MB_SELECT_MAXIMUM) && (F & MB_SELECT_SELECT)))
#define mb_beam_check_select_average(F)		((int)((F & MB_SELECT_AVERAGE) && (F & MB_SELECT_SELECT)))
#define mb_beam_check_select_contact(F)		((int)((F & MB_SELECT_CONTACT) && (F & MB_SELECT_SELECT)))
#define mb_beam_check_select_spare_1(F)		((int)((F & MB_SELECT_SPARE_1) && (F & MB_SELECT_SELECT)))
#define mb_beam_check_select_spare_2(F)		((int)((F & MB_SELECT_SPARE_2) && (F & MB_SELECT_SELECT)))
#define mb_beam_set_select(F)			(F | 0x02)
#define mb_beam_set_select_least(F)		(F | 0x06)
#define mb_beam_set_select_maximum(F)		(F | 0x0a)
#define mb_beam_set_select_average(F)		(F | 0x12)
#define mb_beam_set_select_contact(F)		(F | 0x22)
#define mb_beam_set_select_spare_1(F)		(F | 0x42)
#define mb_beam_set_select_spare_2(F)		(F | 0x82)

/* Bottom detect flags */
#define MB_DETECT_UNKNOWN	0
#define MB_DETECT_AMPLITUDE	1
#define MB_DETECT_PHASE		2

/* Source pulse type flags */
#define MB_PULSE_UNKNOWN	0
#define MB_PULSE_CW		1
#define MB_PULSE_UPCHIRP	2
#define MB_PULSE_DOWNCHIRP	3

/* end conditional include */
#endif
