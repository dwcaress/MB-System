/*--------------------------------------------------------------------
 *    The MB-system:	mbio_status.h	2/1/93
 *
 *    Copyright (c) 1993-2025 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/**
 * @file
 * @brief Defines version, status and error codes used
 * @details Defines version, status and error codes used
 * by MBIO functions and programs
 *
 * Author:	D. W. Caress
 * Date:	January 19, 1993
 *
 *
 */

#ifndef MB_STATUS_H_
#define MB_STATUS_H_

/* MBIO function yes/no/maybe convention */
#define MB_YES 1
#define MB_NO 0
#define MB_MAYBE -1

/* MBIO topography source types */
#define MB_TOPOGRAPHY_TYPE_UNKNOWN 0
#define MB_TOPOGRAPHY_TYPE_ECHOSOUNDER 1
#define MB_TOPOGRAPHY_TYPE_MULTIBEAM 2
#define MB_TOPOGRAPHY_TYPE_SIDESCAN 3
#define MB_TOPOGRAPHY_TYPE_INTERFEROMETRIC 4
#define MB_TOPOGRAPHY_TYPE_LIDAR 5
#define MB_TOPOGRAPHY_TYPE_CAMERA 6
#define MB_TOPOGRAPHY_TYPE_GRID 7
#define MB_TOPOGRAPHY_TYPE_POINT 8

/* MBIO imagery source types */
#define MB_IMAGERY_TYPE_UNKNOWN 0
#define MB_IMAGERY_TYPE_ECHOSOUNDER 1
#define MB_IMAGERY_TYPE_MULTIBEAM 2
#define MB_IMAGERY_TYPE_SIDESCAN 3
#define MB_IMAGERY_TYPE_INTERFEROMETRIC 4
#define MB_IMAGERY_TYPE_LIDAR 5
#define MB_IMAGERY_TYPE_CAMERA 6
#define MB_IMAGERY_TYPE_GRID 7
#define MB_IMAGERY_TYPE_POINT 8

/* MBIO data type ("kind") convention */
#define MB_DATA_KINDS 74
#define MB_DATA_NONE 0
#define MB_DATA_DATA 1                   /* general survey data */
#define MB_DATA_COMMENT 2                /* general comment */
#define MB_DATA_HEADER 3                 /* general header */
#define MB_DATA_CALIBRATE 4              /* Hydrosweep DS */
#define MB_DATA_MEAN_VELOCITY 5          /* Hydrosweep DS */
#define MB_DATA_VELOCITY_PROFILE 6       /* general */
#define MB_DATA_STANDBY 7                /* Hydrosweep DS */
#define MB_DATA_NAV_SOURCE 8             /* Hydrosweep DS */
#define MB_DATA_PARAMETER 9              /* general */
#define MB_DATA_START 10                 /* Simrad */
#define MB_DATA_STOP 11                  /* Simrad */
#define MB_DATA_NAV 12                   /* Simrad, Reson 7k */
#define MB_DATA_RUN_PARAMETER 13         /* Simrad */
#define MB_DATA_CLOCK 14                 /* Simrad */
#define MB_DATA_TIDE 15                  /* Simrad, Reson 7k */
#define MB_DATA_HEIGHT 16                /* Simrad */
#define MB_DATA_HEADING 17               /* Simrad, Hypack */
#define MB_DATA_ATTITUDE 18              /* Simrad, Hypack, Reson 7k */
#define MB_DATA_SSV 19                   /* Simrad */
#define MB_DATA_ANGLE 20                 /* HSMD */
#define MB_DATA_EVENT 21                 /* HSMD */
#define MB_DATA_HISTORY 22               /* GSF */
#define MB_DATA_SUMMARY 23               /* GSF */
#define MB_DATA_PROCESSING_PARAMETERS 24 /* GSF */
#define MB_DATA_SENSOR_PARAMETERS 25     /* GSF */
#define MB_DATA_NAVIGATION_ERROR 26      /* GSF */
#define MB_DATA_SINGLE_BEAM_PING 27      /* GSF */
#define MB_DATA_RAW_LINE 28              /* uninterpretable line for ascii formats */
#define MB_DATA_NAV1 29                  /* ancillary nav system 1 */
#define MB_DATA_NAV2 30                  /* ancillary nav system 2 */
#define MB_DATA_NAV3 31                  /* ancillary nav system 3 */
#define MB_DATA_TILT 32                  /* Simrad */
#define MB_DATA_MOTION 33                /* Reson 7k */
#define MB_DATA_CTD 34                   /* Reson 7k */
#define MB_DATA_SUBBOTTOM_MCS 35         /* Reson 7k */
#define MB_DATA_SUBBOTTOM_CNTRBEAM 36    /* Simrad */
#define MB_DATA_SUBBOTTOM_SUBBOTTOM 37   /* Reson 7k, XTF */
#define MB_DATA_SIDESCAN2 38             /* Reson 7k, XTF */
#define MB_DATA_SIDESCAN3 39             /* Reson 7k, XTF */
#define MB_DATA_IMAGE 40                 /* Reson 7k */
#define MB_DATA_ROLL 41                  /* Reson 7k */
#define MB_DATA_PITCH 42                 /* Reson 7k */
#define MB_DATA_ABSORPTIONLOSS 43        /* Reson 7k */
#define MB_DATA_SPREADINGLOSS 44         /* Reson 7k */
#define MB_DATA_INSTALLATION 45          /* Reson 7k */
#define MB_DATA_WATER_COLUMN 46          /* Simrad */
#define MB_DATA_STATUS 47                /* Simrad, XTF */
#define MB_DATA_DVL 48                   /* JSTAR */
#define MB_DATA_NMEA_RMC 49              /* NMEA */
#define MB_DATA_NMEA_DBT 50              /* NMEA */
#define MB_DATA_NMEA_DPT 51              /* NMEA */
#define MB_DATA_NMEA_ZDA 52              /* NMEA */
#define MB_DATA_NMEA_GLL 53              /* NMEA */
#define MB_DATA_NMEA_GGA 54              /* NMEA */
#define MB_DATA_SURVEY_LINE 55           /* Reson 7k */
#define MB_DATA_ATTITUDE1 56             /* ancillary attitude system 1 */
#define MB_DATA_ATTITUDE2 57             /* ancillary attitude system 2 */
#define MB_DATA_ATTITUDE3 58             /* ancillary attitude system 3 */
#define MB_DATA_SENSORDEPTH 59            /* HYSWEEP dynamic draft */
#define MB_DATA_ALTITUDE 60              /* HYSWEEP single beam echosounder */
#define MB_DATA_GEN_SENS 61              /* WASSP generic sensor data */
#define MB_DATA_WC_PICKS 62              /* WASSP water column picks */
#define MB_DATA_TIMESTAMP 63             /* JSTAR file timestamp */
#define MB_DATA_HEAVE 64                 /* Kongsberg kmall */
#define MB_DATA_BIST 65                  /* Kongsberg BIST report */
#define MB_DATA_BIST1 66                 /* Kongsberg BIST reply */
#define MB_DATA_BIST2 67                 /* Kongsberg BIST short reply */
#define MB_DATA_MBSYSTEM 68              /* Written by MB-System - extension to Kongsberg kmall */
#define MB_DATA_BSCALIBRATIONFILE 69     /* Kongsberg backscatter calibration file */
#define MB_DATA_SALINITY 70              /* Reson 7k */
#define MB_DATA_TEMPERATURE 71           /* Reson 7k */
#define MB_DATA_PIPE 72                  /* Reson 7k */
#define MB_DATA_CONTACT 73               /* Reson 7k */
#define MB_DATA_GATES 74                 /* Reson 7k */
#define MB_DATA_DATUM 75                 /* Kongsberg SPD Sensor Position Datum */

/* MBIO function status convention */
#define MB_SUCCESS 1
#define MB_FAILURE 0

/* MBIO minimum and maximum error values */
#define MB_ERROR_MIN -25
#define MB_ERROR_MAX 17

/* MBIO function fatal error values */
#define MB_ERROR_NO_ERROR 0
#define MB_ERROR_MEMORY_FAIL 1
#define MB_ERROR_OPEN_FAIL 2
#define MB_ERROR_BAD_FORMAT 3
#define MB_ERROR_EOF 4
#define MB_ERROR_WRITE_FAIL 5
#define MB_ERROR_NONE_IN_BOUNDS 6
#define MB_ERROR_NONE_IN_TIME 7
#define MB_ERROR_BAD_DESCRIPTOR 8
#define MB_ERROR_BAD_USAGE 9
#define MB_ERROR_NO_PINGS_BINNED 10
#define MB_ERROR_BAD_KIND 11
#define MB_ERROR_BAD_PARAMETER 12
#define MB_ERROR_BAD_BUFFER_ID 13
#define MB_ERROR_BAD_SYSTEM 14
#define MB_ERROR_BAD_DATA 15
#define MB_ERROR_MISSING_DATA 16
#define MB_ERROR_BAD_TIME 17

/* MBIO function nonfatal error values */
#define MB_ERROR_TIME_GAP -1
#define MB_ERROR_OUT_BOUNDS -2
#define MB_ERROR_OUT_TIME -3
#define MB_ERROR_SPEED_TOO_SMALL -4
#define MB_ERROR_COMMENT -5
#define MB_ERROR_SUBBOTTOM -6
#define MB_ERROR_WATER_COLUMN -7
#define MB_ERROR_OTHER -8
#define MB_ERROR_UNINTELLIGIBLE -9
#define MB_ERROR_IGNORE -10
#define MB_ERROR_NO_DATA_REQUESTED -11
#define MB_ERROR_BUFFER_FULL -12
#define MB_ERROR_NO_DATA_LOADED -13
#define MB_ERROR_BUFFER_EMPTY -14
#define MB_ERROR_NO_DATA_DUMPED -15
#define MB_ERROR_NO_MORE_DATA -16
#define MB_ERROR_DATA_NOT_INSERTED -17
#define MB_ERROR_BAD_PROJECTION -18
#define MB_ERROR_MISSING_PROJECTIONS -19
#define MB_ERROR_MISSING_NAVATTITUDE -20
#define MB_ERROR_NOT_ENOUGH_DATA -21
#define MB_ERROR_FILE_NOT_FOUND -22
#define MB_ERROR_FILE_LOCKED -23
#define MB_ERROR_INIT_FAIL -24
#define MB_ERROR_SIDESCAN_IGNORED -25

/* MBIO problem values */
#define MB_PROBLEM_MAX 6
#define MB_PROBLEM_NO_DATA 1
#define MB_PROBLEM_ZERO_NAV 2
#define MB_PROBLEM_TOO_FAST 3
#define MB_PROBLEM_AVG_TOO_FAST 4
#define MB_PROBLEM_TOO_DEEP 5
#define MB_PROBLEM_BAD_DATAGRAM 6

/* processing status values returned by mb_datalist_read3() */
#define MB_PROCESSED_NONE 0
#define MB_PROCESSED_EXIST 1
#define MB_PROCESSED_USE 2
#define MB_ALTNAV_NONE 0
#define MB_ALTNAV_USE 1

/* image status values returned by mb_imagelist_read() */
#define MB_IMAGESTATUS_NONE             0x00
#define MB_IMAGESTATUS_SINGLE           0x01
#define MB_IMAGESTATUS_LEFT             0x01
#define MB_IMAGESTATUS_RIGHT            0x02
#define MB_IMAGESTATUS_STEREO           0x03
#define MB_IMAGESTATUS_IMAGELIST        0x04
#define MB_IMAGESTATUS_PARAMETER        0x09
#define mb_image_check_none(S) ((int)(S == MB_IMAGESTATUS_NONE))
#define mb_image_check_single(S) ((int)(S == MB_IMAGESTATUS_SINGLE))
#define mb_image_check_left(S) ((int)(S & MB_IMAGESTATUS_LEFT))
#define mb_image_check_right(S) ((int)(S & MB_IMAGESTATUS_RIGHT))
#define mb_image_check_stereo(S) ((int)(S == MB_IMAGESTATUS_STEREO))

/* MBIO maximum notice value */
#define MB_NOTICE_MAX (MB_DATA_KINDS - MB_ERROR_MIN + MB_PROBLEM_MAX + 1)

/* MBIO function error messages */
#ifdef DEFINE_MB_MESSAGES
const char *fatal_error_msg[] = {"No error",
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
                                  "This data file is not in the specified format!",
                                  "Required data are missing",
                                  "Bad time value"};
const char *nonfatal_error_msg[] = {
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
    "Initialization failed",
    "Sidescan ignored",
};
const char *unknown_error_msg[] = {"Unknown error identifier"};

/* MBIO function notice messages */
const char *notice_msg[] = {
    "Unknown notice identifier",

    /* notices for data record types */
    "MB_DATA_DATA (ID=1): survey data", "MB_DATA_COMMENT (ID=2): comment", "MB_DATA_HEADER (ID=3): general header",
    "MB_DATA_CALIBRATE (ID=4): Hydrosweep DS calibration ping", "MB_DATA_MEAN_VELOCITY (ID=5): Hydrosweep DS mean sound speed",
    "MB_DATA_VELOCITY_PROFILE (ID=6): SVP", "MB_DATA_STANDBY (ID=7): Hydrosweep DS standby record",
    "MB_DATA_NAV_SOURCE (ID=8): Hydrosweep DS nav source record", "MB_DATA_PARAMETER (ID=9): Parameter record",
    "MB_DATA_START (ID=10): Simrad start datagram", "MB_DATA_STOP (ID=11): Simrad stop datagram",
    "MB_DATA_NAV (ID=12): Navigation record", "MB_DATA_RUN_PARAMETER (ID=13): Simrad runtime parameter datagram",
    "MB_DATA_CLOCK (ID=14): Simrad clock datagram", "MB_DATA_TIDE (ID=15): Tide record",
    "MB_DATA_HEIGHT (ID=16): Simrad height datagram", "MB_DATA_HEADING (ID=17): Heading record",
    "MB_DATA_ATTITUDE (ID=18): Attitude record", "MB_DATA_SSV (ID=19): Surface sound speed record",
    "MB_DATA_ANGLE (ID=20): Beam angle record", "MB_DATA_EVENT (ID=21): Hydrosweep MD event record",
    "MB_DATA_HISTORY (ID=22): GSF history record", "MB_DATA_SUMMARY (ID=23): GSF summary record",
    "MB_DATA_PROCESSING_PARAMETERS (ID=24): GSF processing parameters record",
    "MB_DATA_SENSOR_PARAMETERS (ID=25): GSF sensor parameter record",
    "MB_DATA_NAVIGATION_ERROR (ID=26): GSF navigation error record",
    "MB_DATA_SINGLE_BEAM_PING (ID=27): GSF single beam ping record",
    "MB_DATA_RAW_LINE (ID=28): uninterpretable ASCII line",
    "MB_DATA_NAV1 (ID=29): Auxiliary nav system 1", "MB_DATA_NAV2 (ID=30): Auxiliary nav system 2",
    "MB_DATA_NAV3 (ID=31): Auxiliary nav system 3", "MB_DATA_TILT (ID=32): Mechanical tilt record",
    "MB_DATA_MOTION (ID=33): Motion (DVL) sensor record", "MB_DATA_CTD (ID=34): CTD record",
    "MB_DATA_SUBBOTTOM_MCS (ID=35): MCS subbottom record", "MB_DATA_SUBBOTTOM_CNTRBEAM (ID=36): Centerbeam subbottom record",
    "MB_DATA_SUBBOTTOM_SUBBOTTOM (ID=37): Subbottom record", "MB_DATA_SIDESCAN2 (ID=38): Secondary sidescan record",
    "MB_DATA_SIDESCAN3 (ID=39): Tertiary sidescan record", "MB_DATA_IMAGE (ID=40): Sonar image record",
    "MB_DATA_ROLL (ID=41): Roll record", "MB_DATA_PITCH (ID=42): Pitch record",
    "MB_DATA_ABSORPTIONLOSS (ID=43): Absorption loss record", "MB_DATA_SPREADINGLOSS (ID=44): Spreading loss record",
    "MB_DATA_INSTALLATION (ID=45): Installation parameter record", "MB_DATA_WATER_COLUMN (ID=46): Water column record",
    "MB_DATA_STATUS (ID=47): Status record", "MB_DATA_DVL (ID=48): DVL record", "MB_DATA_NMEA_RMC (ID=49): NMEA RMC record",
    "MB_DATA_NMEA_DBT (ID=50): NMEA DBT record", "MB_DATA_NMEA_DPT (ID=51): NMEA DPT record",
    "MB_DATA_NMEA_ZDA (ID=52): NMEA ZDA record", "MB_DATA_NMEA_GLL (ID=53): NMEA GLL record",
    "MB_DATA_NMEA_GGA (ID=54): NMEA GGA record", "MB_DATA_SURVEY_LINE (ID=55): Survey line record",
    "MB_DATA_ATTITUDE1 (56): ancillary attitude system 1", "MB_DATA_ATTITUDE2 (57): ancillary attitude system 2",
    "MB_DATA_ATTITUDE3 (58): ancillary attitude system 3", "MB_DATA_SENSORDEPTH (59): HYSWEEP dynamic draft",
    "MB_DATA_ALTITUDE (60): HYSWEEP single beam echosounder",
    "MB_DATA_GEN_SENS (61): WASSP generic sensor data",
    "MB_DATA_WC_PICKS (62): WASSP water column picks",
    "MB_DATA_TIMESTAMP (63): JSTAR file timestamp",
    "MB_DATA_HEAVE (64): Kongsberg kmall",
    "MB_DATA_BIST (65): Kongsberg BIST report",
    "MB_DATA_BIST1 (66): Kongsberg BIST reply",
    "MB_DATA_BIST2 (67): Kongsberg BIST short reply",
    "MB_DATA_MBSYSTEM (68): Written by MB-System - extension to Kongsberg kmall",
    "MB_DATA_BSCALIBRATIONFILE (69): Kongsberg backscatter calibration file",
    "MB_DATA_SALINITY (70): Teledyne s7k salinity",
    "MB_DATA_TEMPERATURE (71): Teledyne s7k salinity",
    "MB_DATA_PIPE (72): Teledyne s7k pipe tracking",
    "MB_DATA_CONTACT (73): Teledyne s7k sonar contact",
    "MB_DATA_GATES (74): Teledyne s7k bathymetry picking gates",

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
    "MB_ERROR_FILE_NOT_FOUND (ID=-22): Requested file cannot be found",
    "MB_ERROR_FILE_LOCKED (ID=-23): Requested file locked",
    "MB_ERROR_INIT_FAIL (ID=-24): Initialization failed",
    "MB_ERROR_SIDESCAN_IGNORED (ID=-25): Sidescan data ignored",

    /* problem notices */
    "DATA PROBLEM (ID=1): No survey data found", "DATA PROBLEM (ID=2): Zero longitude or latitude in survey data",
    "DATA PROBLEM (ID=3): Instantaneous speed exceeds 25 km/hr", "DATA PROBLEM (ID=4): Average speed exceeds 25 km/hr",
    "DATA PROBLEM (ID=5): Sounding depth exceeds 11000 m", "DATA PROBLEM (ID=6): Unsupported datagram or record",
};
const char *unknown_notice_msg[] = {"Unknown notice identifier"};
#endif

/* MBIO sidescan types
    - sidescan values can be logarithmic (dB) or linear (usually voltage) */
#define MB_SIDESCAN_LOGARITHMIC 0
#define MB_SIDESCAN_LINEAR 1

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
 *   xxx1xx01 => Flagged by automatic filter in the current program
 *   xx1xxx01 => Flagged because this is a secondary bottom pick.
 *   x1xxxx01 => Flagged because this is an interpolated rather than observed sounding
 *   1xxxxx01 => Flagged as unreliable using original quality values from sonar.
 *
 *   xxx1xx01 => Flagged because uncertainty exceeds 1 X IHO standard. (original meaning, deprecated)
 *   xx1xxx01 => Flagged because uncertainty exceeds 2 X IHO standard. (original meaning, deprecated)
 *   x1xxxx01 => Flagged because footprint is too large (original meaning, deprecated)
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
#define MB_FLAG_NONE        0x00  // =   0
#define MB_FLAG_FLAG        0x01  // =   1
#define MB_FLAG_NULL        0x01  // =   1
#define MB_FLAG_MANUAL      0x04  // =   4
#define MB_FLAG_FILTER      0x08  // =   8
#define MB_FLAG_FILTER2     0x10  // =  16
#define MB_FLAG_MULTIPICK   0x20  // =  32
#define MB_FLAG_INTERPOLATE 0x40  // =  64
#define MB_FLAG_SONAR       0x80  // = 128
//#define MB_FLAG_GT_1X_IHO 0x10 // original meaning, deprecated
//#define MB_FLAG_GT_2X_IHO 0x20 // original meaning, deprecated

/* Definitions for the SELECT category */
#define MB_SELECT_SELECT 0x02
#define MB_SELECT_LEAST 0x04
#define MB_SELECT_MAXIMUM 0x08
#define MB_SELECT_AVERAGE 0x10
#define MB_SELECT_CONTACT 0x20
#define MB_SELECT_SPARE_1 0x40
#define MB_SELECT_SPARE_2 0x80

/* Definitions for macros applying and testing flags */
#define mb_beam_ok(F) ((int)(!(F & MB_FLAG_FLAG)))
#define mb_beam_check_flag(F) ((int)(F & MB_FLAG_FLAG))
#define mb_beam_check_flag_null(F) ((int)(F == MB_FLAG_NULL))
#define mb_beam_check_flag_flagged(F) ((int)((F & MB_FLAG_FLAG) && (F & 0xFC)))
#define mb_beam_check_flag_manual(F) ((int)((F & MB_FLAG_MANUAL) && (F & MB_FLAG_FLAG)))
#define mb_beam_check_flag_filter(F) ((int)((F & MB_FLAG_FILTER) && (F & MB_FLAG_FLAG)))
#define mb_beam_check_flag_filter2(F) ((int)((F & MB_FLAG_FILTER2) && (F & MB_FLAG_FLAG)))
#define mb_beam_check_flag_multipick(F) ((int)((F & MB_FLAG_MULTIPICK) && (F & MB_FLAG_FLAG)))
#define mb_beam_check_flag_interpolate(F) ((int)((F & MB_FLAG_INTERPOLATE) && (F & MB_FLAG_FLAG)))
#define mb_beam_check_flag_sonar(F) ((int)((F & MB_FLAG_SONAR) && (F & MB_FLAG_FLAG)))
//#define mb_beam_check_flag_gt_1x_iho(F) ((int)((F & MB_FLAG_GT_1X_IHO) && (F & MB_FLAG_FLAG)))
//#define mb_beam_check_flag_gt_2x_iho(F) ((int)((F & MB_FLAG_GT_2X_IHO) && (F & MB_FLAG_FLAG)))
#define mb_beam_check_flag_usable(F) ((int)((F != MB_FLAG_NULL) && !((F & MB_FLAG_FLAG) && ((F & MB_FLAG_INTERPOLATE)))))
#define mb_beam_check_flag_unusable(F) ((int)((F == MB_FLAG_NULL) || ((F & MB_FLAG_FLAG) && ((F & MB_FLAG_INTERPOLATE)))))
#define mb_beam_check_flag_usable2(F) ((int)((F != MB_FLAG_NULL) && !((F & MB_FLAG_FLAG) && ((F & MB_FLAG_INTERPOLATE) || (F & MB_FLAG_MULTIPICK)))))
#define mb_beam_check_flag_unusable2(F) ((int)((F == MB_FLAG_NULL) || ((F & MB_FLAG_FLAG) && ((F & MB_FLAG_INTERPOLATE) || (F & MB_FLAG_MULTIPICK)))))
#define mb_beam_set_flag_null(F) (0x01)
#define mb_beam_set_flag_none(F) (0x00)
#define mb_beam_set_flag_manual(F) (F | 0x05)
#define mb_beam_set_flag_filter(F) (F | 0x09)
#define mb_beam_set_flag_filter2(F) (F | 0x11)
#define mb_beam_set_flag_multipick(F) (F | 0x21)
#define mb_beam_set_flag_interpolate(F) (F | 0x41)
#define mb_beam_set_flag_sonar(F) (F | 0x81)
//#define mb_beam_set_flag_filter2(F) (F | 0x11)
//#define mb_beam_set_flag_gt_1x_iho(F) (F | 0x11)
//#define mb_beam_set_flag_gt_2x_iho(F) (F | 0x21)
#define mb_beam_check_select(F) ((int)(F & MB_SELECT_SELECT))
#define mb_beam_check_select_least(F) ((int)((F & MB_SELECT_LEAST) && (F & MB_SELECT_SELECT)))
#define mb_beam_check_select_maximum(F) ((int)((F & MB_SELECT_MAXIMUM) && (F & MB_SELECT_SELECT)))
#define mb_beam_check_select_average(F) ((int)((F & MB_SELECT_AVERAGE) && (F & MB_SELECT_SELECT)))
#define mb_beam_check_select_contact(F) ((int)((F & MB_SELECT_CONTACT) && (F & MB_SELECT_SELECT)))
#define mb_beam_check_select_spare_1(F) ((int)((F & MB_SELECT_SPARE_1) && (F & MB_SELECT_SELECT)))
#define mb_beam_check_select_spare_2(F) ((int)((F & MB_SELECT_SPARE_2) && (F & MB_SELECT_SELECT)))
#define mb_beam_set_select(F) (F | 0x02)
#define mb_beam_set_select_least(F) (F | 0x06)
#define mb_beam_set_select_maximum(F) (F | 0x0a)
#define mb_beam_set_select_average(F) (F | 0x12)
#define mb_beam_set_select_contact(F) (F | 0x22)
#define mb_beam_set_select_spare_1(F) (F | 0x42)
#define mb_beam_set_select_spare_2(F) (F | 0x82)

/* Bottom detect flags */
#define MB_DETECT_TYPE_NUM 5
#define MB_DETECT_UNKNOWN 0
#define MB_DETECT_AMPLITUDE 1
#define MB_DETECT_PHASE 2
#define MB_DETECT_LIDAR 3
#define MB_DETECT_PHOTOGRAMMETRY 4

/* Source pulse type flags */
#define MB_PULSE_TYPE_NUM 5
#define MB_PULSE_UNKNOWN 0
#define MB_PULSE_CW 1
#define MB_PULSE_UPCHIRP 2
#define MB_PULSE_DOWNCHIRP 3
#define MB_PULSE_LIDAR 4

#endif  /* MB_STATUS_H_ */
