/********************************************************************
 *
 * Module Name : GSF.H
 *
 * Author/Date : J. S. Byrne / Jan 25,  1994
 *
 * Description : This is the header file for the GSF ToolKit
 *
 * Restrictions/Limitations :
 * 1) This library assumes the host computer uses the ASCII character set.
 * 2) This library assumes that the type short is 16 bits, and that the type
 *    int is 32 bits.
 ******
 * NOTE
 * Not (yet!) supported on a machine with 64 bit architecture.
 ******
 *
 * Change Descriptions :
 * who when      what
 * --- ----      ----
 * jsb 10/17/94  Added support for Reson Seabat data
 * jsb 01/29/95  Added support for em121a nominal depth array
 * jsb 08/14/95  Scale factors and pointers to dynamically allocated memory
 *               now maintained by the library, not the caller.  Consolidated
 *               APIs for sequential and direct access, now both use gsfRead
 *               and gsfWrite. This is version GSF-v01.01.
 * jsb 11/01/95  Completed modifications to indexing to support increase in
 *               gsf file size after initial index file creation.  The size
 *               of the file is now stored in the index file header. Index
 *               files without the expected header are recreated on the first
 *               open. This is still version GSF-v01.01. Also added a unique
 *               sensor specific subrecord for Simrad EM1000.
 * jsb 12/22/95  Added gsfGetMBParams, gsfPutMBParams, gsfIsStarboardPing,
 *               and gsfGetSwathBathyBeamWidths. Also added GSF_APPEND as
 *               a file access mode, and modifed GSF_CREATE access mode so
 *               that files can be updated (read and written). This is gsf
 *               library version GSF-v01.02.
 * hem 08/20/96  Added gsfSingleBeamPing Record structure; added Type III
 *               Seabeam, Echotrac, Bathy200, MGD77, BDB, & NOS HDB subrecord
 *               IDs & subrecords; added gsfStringError.  This is gsf library
 *               version GSF-v1.03.
 * jsb 09/27/96  Added support for SeaBeam with amplitude data.
 * jsb 03/24/97  Added gsfSeaBatIISpecific data structure to replace
 *               the gsfSeaBatSpecific data structure, for the Reson 900x
 *               series sonar systems.  Also added gsfSeaBat8101Specific
 *               data structure for the Reson 8101 series sonar system.
 *               Increased the macro GSF_MAX_RECORD_SIZE from 4k to 32k.
 *               This is GSF library version GSF-v1.04.
 * bac 10/27/97  Added a sensor specific subrecord for the SeaBeam 2112/36.
 * dwc 1/9/98    Added a sensor specific subrecord for the Elac Bottomchart MkII.
 * jsb 9/28/98   Added new navigation error record definition. gsfHVNavigationError.
 *               This record is intended to replace the gsfNavigationError record.
 *               This change addresses CRs: GSF-98-001, and GSF-98-002. Also added
 *               new ping array subrecords: horizontal_error, and vertical_error. This
 *               change address CR: GSF-98-003. These new subrecords are intended to
 *               replace the depth_errror, along_track_error and across_track_error
 *               subrecords. In a future release, new file support for these three
 *               error subrecords will be dropped. This is library version GSF-v1.07.
 * jsb 12/19/98  Added support for em3000. Also increased number of entries available
 *               for processing and sensor parameter records. This is library version
 *               GSF-v1.08.
 * wkm 4/1/99    Added CMP_SASS subrecord for Compressed SASS (BOSDAT) data.  This
 *               subrecord should be used in place of the SASS subrecord (TypeIII).
 *               The original has been left in tactc so as to not break existing code.
 * jsb 07/20/99  Completed work on GSF version 1.08.  Added new functions gsfGetSwathBathyArrayMinMax,
 *               and gsfLoadDepthScaleFactorAutoOffset in support of signed depth.
 *               This release addresses the following CRs: GSF-99-002, GSF-99-006, GSF-99-007,
 *               GSF-99-008, GSF-99-009, GSF-99-010, GSF-99-011, GSF-99-012,
 * wkm 7/30/99   Updated SASS specific data subrecord to include 'lntens' and renamed
 *               surface_velocity to 'lfreq'.  These are the original SASS data filed 
 *               names and were requested by NAVO to remane in tact.  Added commet block 
 *               to document mapping of SASS data fields to GSF.
 *
 * Classification : Unclassified
 *
 * References : DoDBL Generic Sensor Format Sept. 30, 1993
 *
 * Copyright (C) Science Applications International Corp.
 ********************************************************************/

#ifndef  __GSF_H__
#define __GSF_H__

/* Get the required standard C library includes */
#include <stdio.h>
#include <time.h>

/* Get the required system include files */
#include <float.h>
#ifdef __OS2__
#include <types.h>
#include <utils.h>
/* specify OS/2 Optlink linkage in the function prototype */
#ifndef OPTLK
#define OPTLK _Optlink
#endif
#else
#include <sys/types.h>
#define OPTLK
#endif

/* Define this version of the GSF library */
#define GSF_VERSION       "GSF-v01.09"

/* Define largest ever expected record size */
#define GSF_MAX_RECORD_SIZE    32768

/* Define the maximum number of files which may be open at once */
#define GSF_MAX_OPEN_FILES     4

/* Define the GSF data file access flags */
#define GSF_CREATE             1
#define GSF_READONLY           2
#define GSF_UPDATE             3
#define GSF_READONLY_INDEX     4
#define GSF_UPDATE_INDEX       5
#define GSF_APPEND             6

/* Define options for sequential access gsf file pointer manipulation */
#define GSF_REWIND             1
#define GSF_END_OF_FILE        2
#define GSF_PREVIOUS_RECORD    3

/* Typedefs for GSF short and long integers */
typedef unsigned short gsfuShort;      /* an unsigned 16 bit integer */
typedef unsigned int   gsfuLong;       /* an unsigned 32 bit integer */
typedef short          gsfsShort;      /* a signed 16 bit integer */
typedef int            gsfsLong;       /* a signed 32 bit integer */

#define GSF_SHORT_SIZE 2
#define GSF_LONG_SIZE  4

/* Define the gsf Data Identifier structure */
typedef struct t_gsfDataID
{
    int             checksumFlag;       /* boolean */
    int             reserved;           /* up to 9 bits */
    int             recordID;           /* bits 00-11 => data type number */
                                        /* bits 12-22 => registry number */
    int             record_number;      /* specifies the nth occurance of */
                                        /* record type specified by recordID */
                                        /* relavent only for direct access */
                                        /* the record_number counts from 1 */
}
gsfDataID;

/* Specify a key to allow reading the next record, no matter what it is */
#define GSF_NEXT_RECORD 0

/* Specify the GSF record data type numbers, for registry number zero */
#define GSF_RECORD_HEADER                                   (unsigned)1
#define GSF_RECORD_SWATH_BATHYMETRY_PING                    (unsigned)2
#define GSF_RECORD_SOUND_VELOCITY_PROFILE                   (unsigned)3
#define GSF_RECORD_PROCESSING_PARAMETERS                    (unsigned)4
#define GSF_RECORD_SENSOR_PARAMETERS                        (unsigned)5
#define GSF_RECORD_COMMENT                                  (unsigned)6
#define GSF_RECORD_HISTORY                                  (unsigned)7
#define GSF_RECORD_NAVIGATION_ERROR                         (unsigned)8  /* 10/19/98 This record is obsolete */
#define GSF_RECORD_SWATH_BATHY_SUMMARY                      (unsigned)9
#define GSF_RECORD_SINGLE_BEAM_PING                         (unsigned)10
#define GSF_RECORD_HV_NAVIGATION_ERROR                      (unsigned)11 /* This record replaces GSF_RECORD_NAVIGATION_ERROR */

/* Number of currently defined record data types (including 0 which is used
 *  in the indexing for ping records which contain scale factor subrecords).
 */
#define             NUM_REC_TYPES  12

/* Put a ceiling on the maximum number of swath bathymetry ping array
 * subrecords allowed in a gsf file.  This define dimensions the scale
 * factors structure.
 */
#define GSF_MAX_PING_ARRAY_SUBRECORDS 20

/* Specify the GSF swath bathymetry ping subrecord identifiers. The beam
 *  data definitions specify the index into the scale factor table, and
 *  define the subrecord id put down on the disk with the subrecord.
 */
#define GSF_SWATH_BATHY_SUBRECORD_DEPTH_ARRAY                  (unsigned)1
#define GSF_SWATH_BATHY_SUBRECORD_ACROSS_TRACK_ARRAY           (unsigned)2
#define GSF_SWATH_BATHY_SUBRECORD_ALONG_TRACK_ARRAY            (unsigned)3
#define GSF_SWATH_BATHY_SUBRECORD_TRAVEL_TIME_ARRAY            (unsigned)4
#define GSF_SWATH_BATHY_SUBRECORD_BEAM_ANGLE_ARRAY             (unsigned)5
#define GSF_SWATH_BATHY_SUBRECORD_MEAN_CAL_AMPLITUDE_ARRAY     (unsigned)6
#define GSF_SWATH_BATHY_SUBRECORD_MEAN_REL_AMPLITUDE_ARRAY     (unsigned)7
#define GSF_SWATH_BATHY_SUBRECORD_ECHO_WIDTH_ARRAY             (unsigned)8
#define GSF_SWATH_BATHY_SUBRECORD_QUALITY_FACTOR_ARRAY         (unsigned)9
#define GSF_SWATH_BATHY_SUBRECORD_RECEIVE_HEAVE_ARRAY          (unsigned)10
#define GSF_SWATH_BATHY_SUBRECORD_DEPTH_ERROR_ARRAY            (unsigned)11 /* 10/19/98 jsb This ping array subrecord is obsolete */
#define GSF_SWATH_BATHY_SUBRECORD_ACROSS_TRACK_ERROR_ARRAY     (unsigned)12 /* 10/19/98 jsb This ping array subrecord is obsolete */
#define GSF_SWATH_BATHY_SUBRECORD_ALONG_TRACK_ERROR_ARRAY      (unsigned)13 /* 10/19/98 jsb This ping array subrecord is obsolete */
#define GSF_SWATH_BATHY_SUBRECORD_NOMINAL_DEPTH_ARRAY          (unsigned)14
#define GSF_SWATH_BATHY_SUBRECORD_QUALITY_FLAGS_ARRAY          (unsigned)15
#define GSF_SWATH_BATHY_SUBRECORD_BEAM_FLAGS_ARRAY             (unsigned)16
#define GSF_SWATH_BATHY_SUBRECORD_SIGNAL_TO_NOISE_ARRAY        (unsigned)17
#define GSF_SWATH_BATHY_SUBRECORD_BEAM_ANGLE_FORWARD_ARRAY     (unsigned)18
#define GSF_SWATH_BATHY_SUBRECORD_VERTICAL_ERROR_ARRAY         (unsigned)19 /* This record replaces DEPTH_ERROR_ARRAY */
#define GSF_SWATH_BATHY_SUBRECORD_HORIZONTAL_ERROR_ARRAY       (unsigned)20 /* This record replaces ACROSS_TRACK_ERROR_ARRAY and ALONG_TRACK_ERROR_ARRAY */

/* Define the additional swath bathymetry subrecords, to which the scale
 * factors do not apply.
 */
#define GSF_SWATH_BATHY_SUBRECORD_UNKNOWN                   (unsigned)  0
#define GSF_SWATH_BATHY_SUBRECORD_SCALE_FACTORS             (unsigned)100
#define GSF_SWATH_BATHY_SUBRECORD_SEABEAM_SPECIFIC          (unsigned)102
#define GSF_SWATH_BATHY_SUBRECORD_EM12_SPECIFIC             (unsigned)103
#define GSF_SWATH_BATHY_SUBRECORD_EM100_SPECIFIC            (unsigned)104
#define GSF_SWATH_BATHY_SUBRECORD_EM950_SPECIFIC            (unsigned)105
#define GSF_SWATH_BATHY_SUBRECORD_EM121A_SPECIFIC           (unsigned)106
#define GSF_SWATH_BATHY_SUBRECORD_EM121_SPECIFIC            (unsigned)107
#define GSF_SWATH_BATHY_SUBRECORD_SASS_SPECIFIC             (unsigned)108  /* 03-30-99 wkm/dbj Typeiii SASS ping array subrecord is obsolete */
#define GSF_SWATH_BATHY_SUBRECORD_SEAMAP_SPECIFIC           (unsigned)109
#define GSF_SWATH_BATHY_SUBRECORD_SEABAT_SPECIFIC           (unsigned)110
#define GSF_SWATH_BATHY_SUBRECORD_EM1000_SPECIFIC           (unsigned)111
#define GSF_SWATH_BATHY_SUBRECORD_TYPEIII_SEABEAM_SPECIFIC  (unsigned)112  /* 03-30-99 wkm/dbj Typeiii Seabeam ping array subrecord is obsolete */
#define GSF_SWATH_BATHY_SUBRECORD_SB_AMP_SPECIFIC           (unsigned)113
#define GSF_SWATH_BATHY_SUBRECORD_SEABAT_II_SPECIFIC        (unsigned)114
#define GSF_SWATH_BATHY_SUBRECORD_SEABAT_8101_SPECIFIC      (unsigned)115
#define GSF_SWATH_BATHY_SUBRECORD_SEABEAM_2112_SPECIFIC     (unsigned)116
#define GSF_SWATH_BATHY_SUBRECORD_ELAC_MKII_SPECIFIC        (unsigned)117
#define GSF_SWATH_BATHY_SUBRECORD_EM3000_SPECIFIC           (unsigned)118
#define GSF_SWATH_BATHY_SUBRECORD_EM1002_SPECIFIC           (unsigned)119
#define GSF_SWATH_BATHY_SUBRECORD_EM300_SPECIFIC            (unsigned)120
#define GSF_SWATH_BATHY_SUBRECORD_CMP_SASS_SPECIFIC         (unsigned)121

#define GSF_SINGLE_BEAM_SUBRECORD_UNKNOWN                   (unsigned)  0
#define GSF_SINGLE_BEAM_SUBRECORD_ECHOTRAC_SPECIFIC         (unsigned)201
#define GSF_SINGLE_BEAM_SUBRECORD_BATHY2000_SPECIFIC        (unsigned)202
#define GSF_SINGLE_BEAM_SUBRECORD_MGD77_SPECIFIC            (unsigned)203
#define GSF_SINGLE_BEAM_SUBRECORD_BDB_SPECIFIC              (unsigned)204
#define GSF_SINGLE_BEAM_SUBRECORD_NOSHDB_SPECIFIC           (unsigned)205

/* Define null values to be used for missing data */
#define GSF_NULL_LATITUDE               91.0
#define GSF_NULL_LONGITUDE             181.0
#define GSF_NULL_HEADING               361.0
#define GSF_NULL_COURSE                361.0
#define GSF_NULL_SPEED                  99.0
#define GSF_NULL_PITCH                  99.0
#define GSF_NULL_ROLL                   99.0
#define GSF_NULL_HEAVE                  99.0
#define GSF_NULL_DRAFT                   0.0
#define GSF_NULL_DEPTH_CORRECTOR        99.99
#define GSF_NULL_TIDE_CORRECTOR         99.99
#define GSF_NULL_SOUND_SPEED_CORRECTION 99.99
#define GSF_NULL_HORIZONTAL_ERROR       -1.00
#define GSF_NULL_VERTICAL_ERROR         -1.00

/* Define null values for the swath bathymetry ping array types. Note that
 * these zero values do not necessarily indicate a non-valid value.  The
 * beam flags array should be used to determine data validity.
 */
#define GSF_NULL_DEPTH                 0.0
#define GSF_NULL_ACROSS_TRACK          0.0
#define GSF_NULL_ALONG_TRACK           0.0
#define GSF_NULL_TRAVEL_TIME           0.0
#define GSF_NULL_BEAM_ANGLE            0.0
#define GSF_NULL_MC_AMPLITUDE          0.0
#define GSF_NULL_MR_AMPLITUDE          0.0
#define GSF_NULL_ECHO_WIDTH            0.0
#define GSF_NULL_QUALITY_FACTOR        0.0
#define GSF_NULL_RECEIVE_HEAVE         0.0
#define GSF_NULL_DEPTH_ERROR           0.0
#define GSF_NULL_ACROSS_TRACK_ERROR    0.0
#define GSF_NULL_ALONG_TRACK_ERROR     0.0
#define GSF_NULL_NAV_POS_ERROR         0.0

/* define Posix.4 proposed structure for internal storage of time */
#if !defined(_STRUCT_TIMESPEC_) && !defined (_TIMESPEC_T) && !defined (_STRUCT_TIMESPEC)        /* SAIC define */
#define _STRUCT_TIMESPEC_
#define _STRUCT_TIMESPEC
#define _TIMESPEC_T
    struct timespec
    {
        time_t          tv_sec;
        long            tv_nsec;
    };
#endif

/* Define a structure for the gsf header record */
#define GSF_VERSION_SIZE 12
typedef struct t_gsfHeader
{
    char            version[GSF_VERSION_SIZE];
}
gsfHeader;

/* Define the data structure for the swath bathymety summary record */
typedef struct t_gsfSwathBathySummary
{
    struct timespec start_time;
    struct timespec end_time;
    double          min_latitude;
    double          min_longitude;
    double          max_latitude;
    double          max_longitude;
    double          min_depth;
    double          max_depth;
}
gsfSwathBathySummary;


/* Define the typeIII specific data structure */
typedef struct t_gsfTypeIIISpecific               /* 03-30-99 wkm/dbj: Obsolete replaced with t_gsfTypeCmpSassSpecific */
{
    unsigned short  leftmost_beam;  /* 0 - leftmost possible beam */
    unsigned short  rightmost_beam;
    unsigned short  total_beams;
    unsigned short  nav_mode;
    unsigned short  ping_number;
    unsigned short  mission_number;
}
gsfTypeIIISpecific;


/* Define the CMP (Compressed) SASS specific data structure (from sass.h) */
typedef struct t_gsfCmpSassSpecific
{
    /************************************************************************************
     *
     *   Mapping from Compressed SASS (BOSDAT) to GSF record
     *
     *    from          to                   comment
     *    ===============================================================================
     *
     *    lntens        ping.heave           mapped only when year is post 1991 or
     *                                       user has elected to force mapping.
     *    lfreq         not-mapped
     *    ldraft        comment              APPLIED_DRAFT comment record
     *    svp.svel      svp.sound_velocity   at <= 1000 ... FATHOMS 
     *                                       at <= 2500 ... METERS
     *                                       otherwise  ... FEET 
     *    svp.deptl     svp.depth            (see sound_velocity)  
     *    lmishn        comment              MISSION_NUMBER comment record
     *    luyr          ping_time            GSF time record from 1960 to 1970 base
     *    pitchl        ping.pitch            
     *    rolll         ping.roll     
     *    lbear         ping.heading         SASS specific (not Seabeam)
     *    pinhd         ping.heading         Seabeam specific (not SASS)
     *    depth         ping.nominal_depth   FATHOMS_TO_METERS_NOMINAL
     *    pslatl        ping.across_track    YARDS_TO_METERS_EXACT
     *    bltime        ping.travel_time
     *    ampl          ping.mr_amplitude
     *    <ftaf file>   ping.beam_flags      HMPS_FLAGS
     *    alpos         ping.along_track     SASS specific YARDS_TO_METERS_EXACT
     *
     ************************************************************************************/

    double lfreq;  /* sea-surface sound velocity in feet/sec from bosdat(lfreq) */
    double lntens; /* post 1992 heave, before 1992 unclear what field contained. */
}
gsfCmpSassSpecific;

/* Define the 16 Beam SeaBeam specific data structure */
typedef struct t_gsfSeabeamSpecific
{
    unsigned short  EclipseTime; /* In 10ths of seconds */
}
gsfSeaBeamSpecific;

typedef struct t_gsfSBAmpSpecific
{
    unsigned char   hour;
    unsigned char   minute;
    unsigned char   second;
    unsigned char   hundredths;
    unsigned int    block_number;
    short           avg_gate_depth;
}
gsfSBAmpSpecific;

/* Define the Seamap specific data structure */
typedef struct t_gsfSeamapSpecific
{
    double          portTransmitter[2];
    double          stbdTransmitter[2];
    double          portGain;
    double          stbdGain;
    double          portPulseLength;
    double          stbdPulseLength;
    double          pressureDepth;
    double          altitude;
    double          temperature;
}
gsfSeamapSpecific;

/* Define the EM950/EM1000 specific data structure */
typedef struct t_gsfEM950Specific
{
    int             ping_number;
    int             mode;
    int             ping_quality;
    double          ship_pitch;
    double          transducer_pitch;
    double          surface_velocity;
}
gsfEM950Specific;

/* Define the EM100 specific data structure */
typedef struct t_gsfEM100Specific
{
    double          ship_pitch;
    double          transducer_pitch;
    int             mode;
    int             power;
    int             attenuation;
    int             tvg;
    int             pulse_length;
    int             counter;
}
gsfEM100Specific;

/* Define the EM121A specific data structure */
typedef struct t_gsfEM121ASpecific
{
    int             ping_number;
    int             mode;
    int             valid_beams;
    int             pulse_length;
    int             beam_width;
    int             tx_power;
    int             tx_status;
    int             rx_status;
    double          surface_velocity;
}
gsfEM121ASpecific;

/* Define the Reson SeaBat specific data structure */
typedef struct t_gsfSeaBatSpecific
{
    int             ping_number;
    double          surface_velocity;
    int             mode;
    int             sonar_range;
    int             transmit_power;
    int             receive_gain;
}
gsfSeaBatSpecific;

/* The gsfSeaBatIISpecific data structure is intended to replace the
 * gsfSeaBatSpecific data structure as of GSF_1.04.
 */
typedef struct t_gsfSeaBatIISpecific
{
    int             ping_number;            /* 1 - 32767 */
    double          surface_velocity;       /* meters/second */
    int             mode;                   /* bit mapped, see macros below */
    int             sonar_range;            /* meters */
    int             transmit_power;
    int             receive_gain;
    double          fore_aft_bw;            /* fore/aft beam width in degrees */
    double          athwart_bw;             /* athwartships beam width in degrees */
    char            spare[4];               /* Four bytes of spare space, for future use */
}
gsfSeaBatIISpecific;

/* Macro definitions for the SeaBatSpecific and SeaBatIISpecific mode field */
#define GSF_SEABAT_WIDE_MODE         0x01   /* if set 10 deg fore-aft */
#define GSF_SEABAT_9002              0x02   /* if set two sonar heads */
#define GSF_SEABAT_STBD_HEAD         0x04   /* if set starboard ping (seabat head 2) */
#define GSF_SEABAT_9003              0x08   /* if set 9003 series sonar (40 beams) */

/* Define the Reson SeaBat specific data structure */
typedef struct t_gsfSeaBat8101Specific
{
    int             ping_number;            /* 1 - 65535 */
    double          surface_velocity;       /* meters/second */
    int             mode;                   /* bit mapped, see macros below */
    int             range;                  /* meters */
    int             power;                  /* 0-8 + status bits */
    int             gain;                   /* 1-45 + status bits */
    int             pulse_width;            /* in microseconds */
    int             tvg_spreading;          /* tvg spreading coefficient * 4 */
    int             tvg_absorption;         /* tvg absorption coefficient */
    double          fore_aft_bw;            /* fore/aft beam width in degrees */
    double          athwart_bw;             /* athwartships beam width in degrees */
    double          range_filt_min;         /* range filter, minimum value, meters (future use) */
    double          range_filt_max;         /* range filter, maximum value, meters (future use) */
    double          depth_filt_min;         /* depth filter, minimum value, meters (future use) */
    double          depth_filt_max;         /* depth filter, maximum value, meters (future use) */
    int             projector;              /* projector type (future use) */
    char            spare[4];               /* Four bytes of spare space, for future use */
}
gsfSeaBat8101Specific;

/* Macro definitions for the SeaBat8101Specific and SeaBat8101Specific mode field */
#define GSF_8101_WIDE_MODE         0x01   /* set if transmit on receiver */
#define GSF_8101_TWO_HEADS         0x02   /* set if two sonar heads */
#define GSF_8101_STBD_HEAD         0x04   /* set if starboard ping (seabat head 2) */
#define GSF_8101_AMPLITUDE         0x08   /* set if beam amplitude is available (RITHETA packet) */

/* Define the SeaBeam 2112/36 specific data structure */
typedef struct t_gsfSeaBeam2112Specific
{
    int             mode;                       /* bit mapped, see macros below */
    double          surface_velocity;           /* meters/second */
    char            ssv_source;                 /* (V)elocimiter, (M)anual, (T)emperature,
                                                   (E)xternal, or (U)nknown */
    int             ping_gain;                  /* dB */
    int             pulse_width;                /* in milliseconds */
    int             transmitter_attenuation;    /* dB */
    int             number_algorithms;          /* algorithms per beam (1-4) */
    char            algorithm_order[5];         /* null terminated string, each char will be either
                                                   a space, W(MT), or B(DI). if number_algorithms
                                                   equals one, this will be four spaces */
    char            spare[2];                   /* Two bytes of spare space, for future use */
}
gsfSeaBeam2112Specific;

/* Macro definitions for the SeaBeam2112Specific mode field */
#define GSF_2112_SVP_CORRECTION   0x01    /* set if true depth, true position corrections are used */
#define GSF_2112_LOW_FREQUENCY    0x02    /* set if using 12kHz frequecy - 36kHz if not set */
#define GSF_2112_AUTO_DEPTH_GATE  0x04    /* set if depth gate mode is automatic - manual if not set */

/* SeaBeam 2112 specific macro definitions for the quality factor array */
#define GSF_2112_POOR_QUALITY     0x01    /* set if the beam was flagged by the SeaBeam as poor quality */
#define GSF_2112_DATA_SOURCE_WMT  0x10    /* set if the data source is WMT - source is BDI if not set */

/* Define the Elac MkII specific data structure */
typedef struct t_gsfElacMkIISpecific
{
    int             mode;                       /* bit mapped, see macros below */
    int             ping_num;
    int             sound_vel;                  /* 0.1 m/s */
    int             pulse_length;               /* 0.01 ms */
    int             receiver_gain_stbd;         /* db */
    int             receiver_gain_port;         /* db */
    int             reserved;
}
gsfElacMkIISpecific;

/* Macro definitions for the ElacMkIISpecific mode field */
#define GSF_MKII_LOW_FREQUENCY    0x01    /* set if using 12kHz frequecy - 36kHz if not set */
#define GSF_MKII_SOURCE_MODE      0x02    /* set if RDT transmit used, otherwise omni */
#define GSF_MKII_SOURCE_POWER     0x04    /* set if transmit high power - low power if not set */
#define GSF_MKII_STBD_HEAD        0x08    /* set if starboard ping */

/* Define a data structure to hold the Simrad EM3000 series run time parameters. */
typedef struct t_gsfEM3RunTime
{
    int             model_number;               /* from the run-time parameter datagram */
    struct timespec dg_time;                    /* from the run-time parameter datagram */
    int             ping_number;                /* sequential counter 0 - 65535 */
    int             serial_number;              /* The sonar head serial number */
    int             system_status;              /* normally = 0 */
    int             mode;                       /* 0=nearfield, 1=normal, 2=target, 3=deep, 4=very deep */
    int             filter_id;
    double          min_depth;                  /* meters */
    double          max_depth;                  /* meters */
    double          absorption;                 /* dB/km */
    double          pulse_length;               /* micro seconds */
    double          transmit_beam_width;        /* degrees */
    int             power_reduction;            /* dB */
    double          receive_beam_width;         /* degrees */
    int             receive_bandwidth;          /* Hz */
    int             receive_gain;               /* dB */
    int             cross_over_angle;           /* degrees */
    int             ssv_source;                 /* 0=sensor, 1=manual, 2=profile */
    int             swath_width;                /* meters */
    int             beam_spacing;               /* 0=beamwidth, 1=equiangle, 2=equidistant, 3=intermediate */
    int             coverage_sector;            /* degrees */
    int             stabilization;
    int             spare1;                     /* four spare bytes */
    int             spare2;                     /* four more spare bytes */
}
gsfEM3RunTime;

/* Define the Simrad EM3000 series specific data structure */
typedef struct t_gsfEM3Specific
{
    /* The first nine values are updated with each depth datagram */
    int             model_number;               /* ie: 3000, ... */
    int             ping_number;                /* 0 - 65535 */
    int             serial_number;              /* 100 - 65535 */
    double          surface_velocity;           /* in m/s */
    double          transducer_depth;           /* transmit transducer depth in meters */
    int             valid_beams;                /* number of valid beams for this ping */
    int             sample_rate;                /* in Hz */
    double          depth_difference;           /* in meters between sonar heads in em3000d configuration */
    int             offset_multiplier;          /* transducer depth offset multiplier */
    /* The gsfEM3RunTime data structure is updated with each run-time parameter datagram */
    gsfEM3RunTime   run_time[2];                /* A two element array is needed to support em3000d */
}
gsfEM3Specific;

/* Define a union of the known sensor specific ping subrecords */
typedef union t_gsfSensorSpecific
{
    gsfSeaBeamSpecific      gsfSeaBeamSpecific;
    gsfEM100Specific        gsfEM100Specific;
    gsfEM121ASpecific       gsfEM121ASpecific;
    gsfEM121ASpecific       gsfEM121Specific;
    gsfSeaBatSpecific       gsfSeaBatSpecific;
    gsfEM950Specific        gsfEM950Specific;
    gsfEM950Specific        gsfEM1000Specific;
    gsfSeamapSpecific       gsfSeamapSpecific;

    #if 1
    /* 03-30-99 wkm/dbj: Obsolete replaced with gsfCmpSassSpecific */
    gsfTypeIIISpecific      gsfTypeIIISeaBeamSpecific;
    gsfTypeIIISpecific      gsfSASSSpecific;
    #endif

    gsfCmpSassSpecific      gsfCmpSassSpecific;

    gsfSBAmpSpecific        gsfSBAmpSpecific;
    gsfSeaBatIISpecific     gsfSeaBatIISpecific;
    gsfSeaBat8101Specific   gsfSeaBat8101Specific;
    gsfSeaBeam2112Specific  gsfSeaBeam2112Specific;
    gsfElacMkIISpecific     gsfElacMkIISpecific;
    gsfEM3Specific          gsfEM3Specific;
} gsfSensorSpecific;

/* Define the Echotrac Single-Beam sensor specific data structure. */
typedef struct t_gsfEchotracSpecific
{
    int             navigation_error;
    unsigned short  mpp_source; /* Flag To determine if nav was mpp */
    unsigned short  tide_source;
}
gsfEchotracSpecific;

/* Define the MGD77 Single-Beam sensor specific data structure. */
typedef struct t_gsfMGD77Specific
{
    unsigned short  time_zone_corr;
    unsigned short  position_type_code;
    unsigned short  correction_code;
    unsigned short  bathy_type_code;
    unsigned short  quality_code;
    double travel_time;
}
gsfMGD77Specific;

/* Define the BDB sensor specific data structure */
typedef struct t_gsfBDBSpecific
{
    int   doc_no;         /* Document number (5 digits) */
    char  eval;           /* Evaluation (1-best, 4-worst) */
    char  class;          /* Classification ((U)nclass, (C)onfidential,
                                             (S)ecret, (P)roprietary/Unclass,
                                             (Q)Proprietary/Class) */
    char  track_adj_flag; /* Track Adjustment Flag (Y,N) */
    char  source_flag;    /* Source Flag ((S)urvey, (R)andom, (O)cean Survey) */
    char  pt_or_track_ln; /* Discrete Point (D) or Track Line (T) Flag */
    char  datum_flag;     /* Datum Flag ((W)GS84, (D)atumless) */
}
gsfBDBSpecific;

/* Define the NOS HDB sensor specific data structure */
typedef struct t_gsfNOSHDBSpecific
{
   unsigned short  type_code;    /*  Depth type code  */
   unsigned short  carto_code;   /*  Cartographic code  */
}
gsfNOSHDBSpecific;

/* Define a union of the known sensor specific
 * single beam ping subrecords
 */
typedef union t_gsfSBSensorSpecific
{
    gsfEchotracSpecific    gsfEchotracSpecific;
    gsfEchotracSpecific    gsfBathy2000Specific;
    gsfMGD77Specific       gsfMGD77Specific;
    gsfBDBSpecific         gsfBDBSpecific;
    gsfNOSHDBSpecific      gsfNOSHDBSpecific;
} gsfSBSensorSpecific;

/* Define the bit flags for the "ping_flags" field of the swath bathymetry
 *  ping record.
 * GSF_IGNORE_PING may be set to indicate to an application to ignore this ping
 * GSF_PING_USER_FLAGS 01-15 may be set/read by application specific software
 */
#define GSF_IGNORE_PING       (unsigned)0x0001
#define GSF_PING_USER_FLAG_01 (unsigned)0x0002
#define GSF_PING_USER_FLAG_02 (unsigned)0x0004
#define GSF_PING_USER_FLAG_03 (unsigned)0x0008
#define GSF_PING_USER_FLAG_04 (unsigned)0x0010
#define GSF_PING_USER_FLAG_05 (unsigned)0x0020
#define GSF_PING_USER_FLAG_06 (unsigned)0x0040
#define GSF_PING_USER_FLAG_07 (unsigned)0x0080
#define GSF_PING_USER_FLAG_08 (unsigned)0x0100
#define GSF_PING_USER_FLAG_09 (unsigned)0x0200
#define GSF_PING_USER_FLAG_10 (unsigned)0x0400
#define GSF_PING_USER_FLAG_11 (unsigned)0x0800
#define GSF_PING_USER_FLAG_12 (unsigned)0x1000
#define GSF_PING_USER_FLAG_13 (unsigned)0x2000
#define GSF_PING_USER_FLAG_14 (unsigned)0x4000
#define GSF_PING_USER_FLAG_15 (unsigned)0x8000

/* Define a set of macros to set, clear, and test the state of the
 *  ping status flags.
 *  Where:
 *     ping_flags: The ping flags field of the gsfSwathBathyPing structure.
 *     usflag:     The definition of the flag to test, set, or clear.
 */
#define gsfTestPingStatus(ping_flags, usflag)    (((ping_flags) & (usflag)) ? 1 : 0)
#define gsfSetPingStatus(ping_flags, usflag)      ((ping_flags) |= (usflag))
#define gsfClearPingStatus(ping_flags, usflag)    ((ping_flags) &= (~(usflag)))

/* Define the GSF bit flags flags for the beam status array.
 * The GSF_IGNORE_BEAM flag may be set to indicate that this beam should
 *  not be used by any processing/display software.  The flags
 *  GSF_BEAM_USER_FLAG_01-07 may be set/read by application specific software
 */
#define GSF_IGNORE_BEAM       (unsigned)0x01
#define GSF_BEAM_USER_FLAG_01 (unsigned)0x02
#define GSF_BEAM_USER_FLAG_02 (unsigned)0x04
#define GSF_BEAM_USER_FLAG_03 (unsigned)0x08
#define GSF_BEAM_USER_FLAG_04 (unsigned)0x10
#define GSF_BEAM_USER_FLAG_05 (unsigned)0x20
#define GSF_BEAM_USER_FLAG_06 (unsigned)0x40
#define GSF_BEAM_USER_FLAG_07 (unsigned)0x80

/* Define the internal form of the array subrecord scale factor information,
 * which is used to scale the swath bathymetry ping record to/from
 * internal/external form. The subrecord id is specified by the index into
 * the scaleTable array.
 */
typedef struct t_gsfScaleInfo
{
    unsigned char   compressionFlag;    /* flag for applicable compression routine */
    double          multiplier;         /* the scale factor (millionths)for the array */
    double          offset;             /* dc offset to scale data by */
} gsfScaleInfo;
typedef struct t_gsfScaleFactors
{
    int             numArraySubrecords; /* the number of scaling factors we actually have */
    gsfScaleInfo    scaleTable[GSF_MAX_PING_ARRAY_SUBRECORDS];
} gsfScaleFactors;

/* Define the data structure for a ping from a swath bathymetric system */
typedef struct t_gsfSwathBathyPing
{
    struct timespec ping_time;          /* seconds and nanoseconds */
    double          latitude;           /* in degrees */
    double          longitude;          /* in degrees */
    short           number_beams;       /* in this ping */
    short           center_beam;        /* offset into array (0 = portmost outer) */
    unsigned short  ping_flags;         /* flags to mark status of this ping */
    short           reserved;           /* for future use */
    double          tide_corrector;     /* in meters */
    double          depth_corrector;    /* in meters */
    double          heading;            /* in degrees */
    double          pitch;              /* in degrees */
    double          roll;               /* in degrees */
    double          heave;              /* in meters    */
    double          course;             /* in degrees */
    double          speed;              /* in knots */
    gsfScaleFactors scaleFactors;       /* The array scale factors for this data */
    double         *depth;              /* depth array (meters) */
    double         *nominal_depth;      /* Array of depth relative to 1500 m/s */
    double         *across_track;       /* across track array (meters) */
    double         *along_track;        /* along track array (meters) */
    double         *travel_time;        /* roundtrip travel time array (seconds) */
    double         *beam_angle;         /* beam angle array (degrees from vertical) */
    double         *mc_amplitude;       /* mean, calibrated beam amplitude array (dB re 1V/micro pascal at 1 meter) */
    double         *mr_amplitude;       /* mean, relative beam amplitude array (dB re 1V/micro pascal at 1 meter) */
    double         *echo_width;         /* echo width array (seconds) */
    double         *quality_factor;     /* quality factor array (dimensionless) */
    double         *receive_heave;      /* Array of heave data (meters) */
    double         *depth_error;        /* Array of estimated vertical error (meters) */
    double         *across_track_error; /* Array of estimated across track error (meters) */
    double         *along_track_error;  /* Array of estimated along track error (meters) */
    unsigned char  *quality_flags;      /* Two bit beam detection flags provided by Reson sonar */
    unsigned char  *beam_flags;         /* Array of beam status flags */
    double         *signal_to_noise;    /* signal to noise ratio (dB) */
    double         *beam_angle_forward; /* beam angle forward array (degrees counterclockwise from stbd.) */
    double         *vertical_error;     /* Array of estimated vertical error (meters, at 95% confidence) */
    double         *horizontal_error;   /* Array of estimated horizontal error (meters, at 95% confidence) */
    int             sensor_id;          /* a definition which specifies the sensor */
    gsfSensorSpecific sensor_data;      /* union of known sensor specific data */
}
gsfSwathBathyPing;

/* Define a single beam record structure. */
typedef struct t_gsfSingleBeamPing
{
    struct timespec ping_time;          /* Time the sounding was made */
    double          latitude;           /* latitude (degrees) of sounding */
    double          longitude;          /* longitude (degrees) of sounding */
    double          tide_corrector;     /* in meters */
    double          depth_corrector;    /* in meters draft corrector for sensor */
    double          heading;            /* in degrees */
    double          pitch;              /* in meters */
    double          roll;               /* in meters */
    double          heave;              /* in meters */
    double          depth;              /* in meters */
    double          sound_speed_correction;  /* in meters */
    unsigned short  positioning_system_type;
    int             sensor_id;
    gsfSBSensorSpecific sensor_data;
}
gsfSingleBeamPing;

/* Define the sound velocity profile structure */
typedef struct t_gsfSVP
{
    struct timespec observation_time;   /* time the SVP measurement was made            */
    struct timespec application_time;   /* time the SVP was used by the sonar           */
    double          latitude;           /* latitude (degrees) of SVP measurement        */
    double          longitude;          /* longitude (degrees) of SVP measurement       */
    int             number_points;      /* number of data points in the profile         */
    double         *depth;              /* array of profile depth values in meters      */
    double         *sound_speed;        /* array of profile sound velocity values in m/s*/
}
gsfSVP;

/* Define the internal record structure for processing parameters */
#define GSF_MAX_PROCESSING_PARAMETERS 128
typedef struct t_gsfProcessingParameters
{
    struct timespec param_time;
    int             number_parameters;
    short           param_size[GSF_MAX_PROCESSING_PARAMETERS];  /* array of sizes of param text */
    char           *param[GSF_MAX_PROCESSING_PARAMETERS];       /* array of parameters: "param_name=param_value" */
}
gsfProcessingParameters;

/* Define the sensor parameters record structure */
#define GSF_MAX_SENSOR_PARAMETERS 128
typedef struct t_gsfSensorParameters
{
    struct timespec param_time;
    int             number_parameters;
    short           param_size[GSF_MAX_SENSOR_PARAMETERS];      /* array of sizes of param text */
    char           *param[GSF_MAX_SENSOR_PARAMETERS];   /* array of parameters: "param_name=param_value" */
}
gsfSensorParameters;

/* Define the comment record structure */
typedef struct t_gsfComment
{
    struct timespec comment_time;
    int             comment_length;
    char           *comment;
}
gsfComment;

/* Define the history record */
#define GSF_OPERATOR_LENGTH  64
#define GSF_HOST_NAME_LENGTH 64
typedef struct t_gsfHistory
{
    struct timespec history_time;
    char            host_name[GSF_HOST_NAME_LENGTH + 1];
    char            operator[GSF_OPERATOR_LENGTH + 1];
    char           *command_line;
    char           *comment;
}
gsfHistory;

/* Define the navigation error record
 * jsb As of GSF v1.07, this record is replaced by gsfHVNavigationError.
 * All newly created files should be written using gsfHVNavigationError,
 * instead of gsfNavigationError.
 */
typedef struct t_gsfNavigationError
{
    struct timespec nav_error_time;
    int             record_id;          /* Containing nav with these errors */
    double          latitude_error;     /* 90% CE in meters */
    double          longitude_error;    /* 90% CE in meters */
}
gsfNavigationError;

/* jsb As of GSF v1.07, This new navigation error record replaces gsfNavigationError.
 *  The definition of gsfNavigationError will remain in the specification for several
 *  release of GSF for backwards compatability. (The HV stands for Horizontal and Vertical)
 */
typedef struct t_gsfHVNavigationError
{
    struct timespec nav_error_time;
    int             record_id;                 /* Containing nav with these errors */
    double          horizontal_error;          /* RMS error in meters */
    double          vertical_error;            /* RMS error in meters */
    char            spare[4];                  /* four bytes of reserved space */
    char           *position_type;             /* A character string code which specifies the type of positioning system */
}
gsfHVNavigationError;

/* Define a set of macros which may to used to set the position_type field */
#define GSF_POS_TYPE_UNKN "UNKN"               /* Unknown positioning system type */
#define GSF_POS_TYPE_GPSU "GPSU"               /* GPS Position, unknown positioning service */
#define GSF_POS_TYPE_PPSD "PPSD"               /* Precise positioning service - differential */
#define GSF_POS_TYPE_PPSK "PPSK"               /* Precise positioning service - kinematic */
#define GSF_POS_TYPE_PPSS "PPSS"               /* Precise positioning service - standalone */
#define GSF_POS_TYPE_SPSD "SPSD"               /* Standard positioning service - differential */
#define GSF_POS_TYPE_SPSK "SPSK"               /* Standard positioning service - kinematic */
#define GSF_POS_TYPE_SPSS "SPSS"               /* Standard positioning service - standalone */

/* Define a structure to encapsulate the known gsf records, this
 * simplifies the number of arguments to gsfRead, and gsfWrite.
 */
typedef struct t_gsfRecords
{
    gsfHeader               header;
    gsfSwathBathySummary    summary;
    gsfSwathBathyPing       mb_ping;
    gsfSingleBeamPing       sb_ping;
    gsfSVP                  svp;
    gsfProcessingParameters process_parameters;
    gsfSensorParameters     sensor_parameters;
    gsfComment              comment;
    gsfHistory              history;
    gsfNavigationError      nav_error;
    gsfHVNavigationError    hv_nav_error;
} gsfRecords;

/* Define a data structure to hold offsets needed to correct multibeam
 * bathymetric data. Currently gsf supports tracking of up to two pairs
 * of each of the relavent offsets.  This is required for systems such as
 * HydroChart II and Reson 9002 which have two pairs of transmit/receive
 * arrays per installation.
 */
#define GSF_MAX_OFFSETS          2
#define GSF_COMPENSATED          1
#define GSF_UNCOMPENSATED        0
#define GSF_TRUE_DEPTHS          1
#define GSF_DEPTHS_RE_1500_MS    2
#define GSF_DEPTH_CALC_UNKNOWN   3
#define GSF_UNKNOWN_PARAM_VALUE  DBL_MIN        /* defined in <float.h> */

typedef struct t_gsfMBOffsets
{
    double           draft[GSF_MAX_OFFSETS];                 /* meters */
    double           roll_bias[GSF_MAX_OFFSETS];             /* degrees */
    double           pitch_bias[GSF_MAX_OFFSETS];            /* degrees */
    double           gyro_bias[GSF_MAX_OFFSETS];             /* degrees */
    double           position_x_offset;                      /* meters */
    double           position_y_offset;                      /* meters */
    double           position_z_offset;                      /* meters */
    double           transducer_x_offset[GSF_MAX_OFFSETS];   /* meters */
    double           transducer_y_offset[GSF_MAX_OFFSETS];   /* meters */
    double           transducer_z_offset[GSF_MAX_OFFSETS];   /* meters */
} gsfMBOffsets;

/* Define a data structure to hold multibeam sonar processing parameters */
typedef struct t_gsfMBParams
{
    /* These parameters define reference points */
    char start_of_epoch[64];
    int horizontal_datum;
    int vertical_datum;

    /* These parameters specify what corrections have been applied to the data */
    int roll_compensated;   /* = GSF_COMPENSATED if the depth data has been corrected for roll */
    int pitch_compensated;  /* = GSF_COMPENSATED if the depth data has been corrected for pitch */
    int heave_compensated;  /* = GSF_COMPENSATED if the depth data has been corrected for heave */
    int tide_compensated;   /* = GSF_COMPENSATED if the depth data has been corrected for tide */
    int ray_tracing;        /* = GSF_COMPENSATED if the travel time/angle pairs are compensated for ray tracing */
    int depth_calculation;  /* = GSF_TRUE_DEPTHS, or GSF_DEPTHS_RE_1500_MS, applicable to the depth field */

    /* These parameters specify known offsets which have NOT been corrected.
     * If each of these values are zero, then all known offsets have been
     * corrected for.
     */
    gsfMBOffsets to_apply;

    /* These parameters specify offsets which have already been corrected. */
    gsfMBOffsets applied;
} gsfMBParams;

/* Macro definitions for approved horizontal datums. Note that as of
 * 12/20/95 only WGS-84 is supported by GSF.
 */
#define GSF_H_DATUM_ADI   1   /* Adinan */
#define GSF_H_DATUM_ARF   2   /* Arc 1950 */
#define GSF_H_DATUM_ARS   3   /* Arc 1960 */
#define GSF_H_DATUM_AUA   4   /* Australian Geodetic */
#define GSF_H_DATUM_BAT   5   /* Djakarta (Batavia) */
#define GSF_H_DATUM_BID   6   /* Bissau Base Northwest End Pillar, Portugese Guinea */
#define GSF_H_DATUM_BUR   7   /* Bukit Rimpah */
#define GSF_H_DATUM_CAI   8   /* Campo Inchauspe */
#define GSF_H_DATUM_CAM   9   /* Camacupa Base SW End */
#define GSF_H_DATUM_CAP  10   /* Cape, South Africa */
#define GSF_H_DATUM_CAA  11   /* Campo Area Astro */
#define GSF_H_DATUM_CHO  12   /* Chatham Island Astro */
#define GSF_H_DATUM_CHU  13   /* Chua Astro */
#define GSF_H_DATUM_COA  14   /* Corrego Alegre */
#define GSF_H_DATUM_ENB  15   /* European 79 */
#define GSF_H_DATUM_EUR  16   /* European */
#define GSF_H_DATUM_GDA  17   /* German */
#define GSF_H_DATUM_GEO  18   /* Geodetic Datum 1949 */
#define GSF_H_DATUM_GHA  19   /* Ghana */
#define GSF_H_DATUM_GSB  20   /* G. Segara */
#define GSF_H_DATUM_GSF  21   /* G. Serindung */
#define GSF_H_DATUM_GUA  22   /* Guam 1963 */
#define GSF_H_DATUM_HEN  23   /* Herat North */
#define GSF_H_DATUM_HER  24   /* Hermannskogel */
#define GSF_H_DATUM_HJO  25   /* Hjorsey 1955 */
#define GSF_H_DATUM_HTN  26   /* Hu-Tu-Shan */
#define GSF_H_DATUM_IDA  27   /* Italian */
#define GSF_H_DATUM_IND  28   /* Indian */
#define GSF_H_DATUM_IRE  29   /* Ireland 1965 */
#define GSF_H_DATUM_KEA  30   /* Kertau */
#define GSF_H_DATUM_LIB  31   /* Liberia 1964 (Robertsfield Astro) */
#define GSF_H_DATUM_LOC  32   /* Local Astro */
#define GSF_H_DATUM_LUZ  33   /* Luzon */
#define GSF_H_DATUM_MER  34   /* Merchich */
#define GSF_H_DATUM_MET  35   /* Mercury */
#define GSF_H_DATUM_MOL  36   /* Montjong Lowe */
#define GSF_H_DATUM_NAN  37   /* Nanking 1960 */
#define GSF_H_DATUM_NAR  38   /* North American 1983 */
#define GSF_H_DATUM_NAS  39   /* North American 1927 */
#define GSF_H_DATUM_NIG  40   /* Nigeria */
#define GSF_H_DATUM_OGB  41   /* Ordnance Survey of Great Britain 1936 */
#define GSF_H_DATUM_OHA  42   /* Old Hawaiian */
#define GSF_H_DATUM_OSI  43   /* Ordnance Survey of Ireland */
#define GSF_H_DATUM_PLN  44   /* Pico de las Nieves, Gran Canaria, Canary Islands */
#define GSF_H_DATUM_PRP  45   /* Provisional South American 1956 */
#define GSF_H_DATUM_QUO  46   /* Qornoq */
#define GSF_H_DATUM_SIB  47   /* Sierra leone 1960 */
#define GSF_H_DATUM_TAN  48   /* Tananarive Obsv 1925 */
#define GSF_H_DATUM_TIL  49   /* Timbalai */
#define GSF_H_DATUM_TOK  50   /* Tokyo */
#define GSF_H_DATUM_UND  51   /* Undetermined */
#define GSF_H_DATUM_VOI  52   /* Voirol */
#define GSF_H_DATUM_WGA  53   /* World Geodetic System 1960 */
#define GSF_H_DATUM_WGB  54   /* World Geodetic System 1966 */
#define GSF_H_DATUM_WGC  55   /* World Geodetic System 1972 */
#define GSF_H_DATUM_WGD  56   /* World Geodetic System 1980 */
#define GSF_H_DATUM_WGE  57   /* World Geodetic System 1984 */
#define GSF_H_DATUM_WGS  58   /* World Geodetic System (year unknown) */
#define GSF_H_DATUM_XXX  59   /* Multiple datums */
#define GSF_H_DATUM_YAC  60   /* Yacare */

/* Macro definitions for supported vertical datums */
#define GSF_V_DATUM_UNKNOWN  1  /* Unknown vertical datum */
#define GSF_V_DATUM_MLLW     2  /* Mean lower low water */
#define GSF_V_DATUM_MLW      3  /* Mean low water */

/* Define the error codes which gsfError may be set to */
#define GSF_NORMAL                                 0
#define GSF_FOPEN_ERROR                           -1
#define GSF_UNRECOGNIZED_FILE                     -2
#define GSF_BAD_ACCESS_MODE                       -3
#define GSF_READ_ERROR                            -4
#define GSF_WRITE_ERROR                           -5
#define GSF_INSUFFICIENT_SIZE                     -6
#define GSF_RECORD_SIZE_ERROR                     -7
#define GSF_CHECKSUM_FAILURE                      -8
#define GSF_FILE_CLOSE_ERROR                      -9
#define GSF_TOO_MANY_ARRAY_SUBRECORDS            -10
#define GSF_TOO_MANY_OPEN_FILES                  -11
#define GSF_MEMORY_ALLOCATION_FAILED             -12
#define GSF_UNRECOGNIZED_RECORD_ID               -13
#define GSF_STREAM_DECODE_FAILURE                -14
#define GSF_BAD_SEEK_OPTION                      -15
#define GSF_FILE_SEEK_ERROR                      -16
#define GSF_UNRECOGNIZED_SENSOR_ID               -17
#define GSF_UNRECOGNIZED_DATA_RECORD             -18
#define GSF_UNRECOGNIZED_ARRAY_SUBRECORD_ID      -19
#define GSF_UNRECOGNIZED_SUBRECORD_ID            -20
#define GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER      -21
#define GSF_CANNOT_REPRESENT_PRECISION           -22
#define GSF_READ_TO_END_OF_FILE                  -23
#define GSF_BAD_FILE_HANDLE                      -24
#define GSF_HEADER_RECORD_DECODE_FAILED          -25
#define GSF_MB_PING_RECORD_DECODE_FAILED         -26
#define GSF_SVP_RECORD_DECODE_FAILED             -27
#define GSF_PROCESS_PARAM_RECORD_DECODE_FAILED   -28
#define GSF_SENSOR_PARAM_RECORD_DECODE_FAILED    -29
#define GSF_COMMENT_RECORD_DECODE_FAILED         -30
#define GSF_HISTORY_RECORD_DECODE_FAILED         -31
#define GSF_NAV_ERROR_RECORD_DECODE_FAILED       -32
#define GSF_HEADER_RECORD_ENCODE_FAILED          -25
#define GSF_MB_PING_RECORD_ENCODE_FAILED         -26
#define GSF_SVP_RECORD_ENCODE_FAILED             -27
#define GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED   -28
#define GSF_SENSOR_PARAM_RECORD_ENCODE_FAILED    -29
#define GSF_COMMENT_RECORD_ENCODE_FAILED         -30
#define GSF_HISTORY_RECORD_ENCODE_FAILED         -31
#define GSF_NAV_ERROR_RECORD_ENCODE_FAILED       -32
#define GSF_SETVBUF_ERROR                        -33
#define GSF_FLUSH_ERROR                          -34
#define GSF_FILE_TELL_ERROR                      -35
#define GSF_INDEX_FILE_OPEN_ERROR                -36
#define GSF_CORRUPT_INDEX_FILE_ERROR             -37
#define GSF_SCALE_INDEX_CALLOC_ERROR             -38
#define GSF_RECORD_TYPE_NOT_AVAILABLE            -39
#define GSF_SUMMARY_RECORD_DECODE_FAILED         -40
#define GSF_SUMMARY_RECORD_ENCODE_FAILED         -41
#define GSF_INVALID_NUM_BEAMS                    -42
#define GSF_INVALID_RECORD_NUMBER                -43
#define GSF_INDEX_FILE_READ_ERROR                -44
#define GSF_PARAM_SIZE_FIXED                     -45
#define GSF_SINGLE_BEAM_ENCODE_FAILED            -46
#define GSF_HV_NAV_ERROR_RECORD_ENCODE_FAILED    -47
#define GSF_HV_NAV_ERROR_RECORD_DECODE_FAILED    -48


/* The following are the function protoytpes for all functions intended
 * to be exported by the library.
 */

int OPTLK       gsfOpen(const char *filename, const int mode, int *handle);
/*
 * Description : This function attempts to open a gsf data file.  If the
 *  file exits and is opened readonly or update the gsf header is read
 *  to confirm that this is a gsf data file.  If the file is opened create,
 *  the GSF header containing the version number of the software library is
 *  written into the header.  This function passes an integer handle back to
 *  the calling application.  The handle is used for all further access to the
 *  file. gsfOpen explicitly sets stream bufferring to the value specified
 *  by GSF_STREAM_BUF_SIZE.  The internal file table is searched for an
 *  available entry whose name matches that specified in the argument list, if
 *  no match is found, then the first available entry is used.  Up to
 *  GSF_MAX_OPEN_FILES files may be open by an application at a time.
 *
 * Inputs :
 *  filename = a fully qualified path to the gsf file to open
 *  mode may have the following values:
 *     GSF_READONLY = open an existing file for read only access
 *     GSF_UPDATE   = open an existing file for reading an writing
 *     GSF_CREATE   = create a new gsf file
 *     GSF_READONLY_INDEX = open an existing file for read only access with index
 *     GSF_UPDATE_INDEX   = open an existing file for reading an writing with index
 *  handle = a pointer to an integer to be assigned a handle which will be
 *     reference for all future file access.
 *
 * Returns :
 *  This funciton returns zero if successful, or -1 if an error occured.
 *
 * Error Conditions :
 *     GSF_BAD_ACCESS_MODE
 *     GSF_TOO_MANY_OPEN_FILES
 *     GSF_FOPEN_ERROR
 *     GSF_SETVBUF_ERROR
 *     GSF_UNRECOGNIZED_FILE
 */

int OPTLK       gsfOpenBuffered(const char *filename, const int mode, int *handle, int buf_size);
/*
 * Description : This function attempts to open a gsf data file.  If the
 *  file exits and is opened readonly or update the gsf header is read
 *  to confirm that this is a gsf data file.  If the file is opened create,
 *  the GSF header containing the version number of the software library is
 *  written into the header.  This function passes an integer handle back to
 *  the calling application.  The handle is used for all further access to the
 *  file. gsfOpenBufferd explicitly sets stream bufferring to the value
 *  specified by the buf_size argument. The internal file table is searched
 *  for an available entry whose name matches that specified in the argument
 *  list, if no match is found, then the first available entry is used.  Up
 *  to GSF_MAX_OPEN_FILES files may be open by an application at a time.
 *  gsfOpenBuffered performs identical processing to gsfOpen, except here,
 *  the caller is allowed to explicitly set the standard system library level
 *  I/O buffer size.
 *
 * Inputs :
 *  filename = a fully qualified path to the gsf file to open
 *  mode may have the following values:
 *     GSF_READONLY = open an existing file for read only access
 *     GSF_UPDATE   = open an existing file for reading an writing
 *     GSF_CREATE   = create a new gsf file
 *     GSF_READONLY_INDEX = open an existing file for read only access with index
 *     GSF_UPDATE_INDEX   = open an existing file for reading an writing with index
 *  handle = a pointer to an integer to be assigned a handle which will be
 *     reference for all future file access.
 *  buf_size = an integer buffer size in bytes.
 *
 * Returns :
 *  This funciton returns zero if successful, or -1 if an error occured.
 *
 * Error Conditions :
 *     GSF_BAD_ACCESS_MODE
 *     GSF_TOO_MANY_OPEN_FILES
 *     GSF_FOPEN_ERROR
 *     GSF_SETVBUF_ERROR
 *     GSF_UNRECOGNIZED_FILE
 */

int OPTLK       gsfClose(const int handle);
/*
 * Description : This function closes a gsf file previously openned
 *  using gsfOpen.
 *
 * Inputs :
 *  handle = the handle of the gsf file to be closed.
 *
 * Returns :
 *  This function returns zero if successful, or -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_BAD_FILE_HANDLE
 *    GSF_FILE_CLOSE_ERROR
 */

int OPTLK       gsfSeek(int handle, int option);
/*
 * Description : This function may be used to move the file pointer
 *  for a previously openned gsf file.
 *
 * Inputs :
 *  handle = the integer handle returned from gsf Open
 *  option = the desired action for moving the file pointer, where:
 *    GSF_REWIND, move pointer to first record in the file.
 *    GSF_END_OF_FILE, move pointer to the end of the file.
 *    GSF_PREVIOUS_RECORD, backup to the beginning of the record just
 *     written or just read.
 *
 * Returns :
 *  This function returns zero if successful, or -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_BAD_FILE_HANDLE
 *    GSF_FILE_SEEK_ERROR
 *    GSF_BAD_SEEK_OPTION
 */

int OPTLK       gsfRead(int handle, int desiredRecord, gsfDataID * dataID, gsfRecords * rec, unsigned char *stream, int max_size);
/* Description : gsfRead supports both direct and sequential access. If the
 *  file is opened for sequential access, this function reads the desired
 *  record from the gsf data file specified by handle.  The "desiredRecord"
 *  argument may be set to GSF_NEXT_RECORD to read the next record in the
 *  data file, or "desiredRecord" record may be set to the id of the record
 *  of interest, in which case the file will be read, skipping past
 *  intermediary records until the desired record is found.  When the desired
 *  record is found, it is read and then decoded from external to internal
 *  form. If the optional checksum is found with the data it will be verified.
 *  All of the fields of the gsfDataID structure, with the exception of the
 *  record_number field will be loaded with the values contained in the GSF
 *  record byte stream.  The record_number field will be undefined.  The
 *  stream and max_size arguments are normally set to NULL, unless the
 *  calling application is interested in a copy of the GSF byte stream.
 *
 *  If the file is opened for direct access, then the combination of the
 *  recordID and the record_number fields of the dataID structure are used
 *  to uniquely identify the record of interest.  The address for this record
 *  is retrieved from the index file, which was created on a previous call
 *  to gsfOpen or gsfOpenBuffered.  If the record of interest is a ping record
 *  for which we need to retrieve new scale factors, then the ping record
 *  containing the scale factors needed is read first, and then the ping
 *  record of interest is read.  Direct access applications should set the
 *  desiredRecord argument equal to the recordID field in the gsfDataID
 *  structure.
 *
 * Inputs :
 *    handle = the handle to the file as provided by gsfOpen
 *    desiredRecord = the desired record or GSF_NEXT_RECORD
 *    dataID = a pointer to a gsfDataID structure to be populated for the
 *             input record.
 *    rptr = a pointer to a gsfRecords structure to be populated with the
 *           data from the input record in internal form.
 *    stream = an optional pointer to caller memory to be populated with a copy
 *          of the gsf byte stream for this record.
 *    max_size = an optional maximum size to copy into buf
 *
 * Returns :
 *  This function returns the number of bytes read if successful,
 *  or -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_BAD_FILE_HANDLE
 *    GSF_FILE_SEEK_ERROR
 *    GSF_FLUSH_ERROR
 *    GSF_READ_TO_END_OF_FILE
 *    GSF_READ_ERROR
 *    GSF_RECORD_SIZE_ERROR
 *    GSF_INSUFFICIENT_SIZE
 *    GSF_CHECKSUM_FAILURE
 *    GSF_UNRECOGNIZED_RECORD_ID
 *    GSF_HEADER_RECORD_DECODE_FAILED
 *    GSF_SVP_RECORD_DECODE_FAILED
 *    GSF_PROCESS_PARAM_RECORD_DECODE_FAILED
 *    GSF_SENSOR_PARAM_RECORD_DECODE_FAILED
 *    GSF_COMMENT_RECORD_DECODE_FAILED
 *    GSF_HISTORY_RECORD_DECODE_FAILED
 *    GSF_NAV_ERROR_RECORD_DECODE_FAILED
 */

int OPTLK       gsfWrite(int handle, gsfDataID * id, gsfRecords * record);
/* Description : gsfWrite encodes the data from internal to external form,
 *  and then writes the requested record into the file specified by handle,
 *  where handle is the value retured by gsfOpen.  The record is written to
 *  the current file pointer for handle.  An optional checksum may be computed
 *  and encoded with the data.
 *
 *  If the file is opened for sequential access (GSF_CREATE, or GSF_UPDATE)
 *  then the recordID field of the gsfDataID structure is used to specify
 *  the record to be written.  The record is written at the current location
 *  in the file.
 *
 *  If the file is opened for direct access (GSF_UPDATE_INDEX), then the
 *  combination of the recordID and the record_number fields of the gsfDataID
 *  structure are used to uniquely identify the record to be written.  The
 *  address of the record of interest is read from the index file and the file
 *  pointer is moved to this offset before the record is encoded and written
 *  to disk.
 *
 * Inputs :
 *  handle = the handle for this file as returned by gsfOpen
 *  id = a pointer to a gsfDataID containing the record id information for
 *       the record to write.
 *  rptr = a pointer to a gsfRecords structure from which to get the internal
 *         form of the record to be written to the file.
 *
 * Returns :
 *  This function returns the number of bytes written if successful, or
 *  -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_BAD_FILE_HANDLE
 *    GSF_UNRECOGNIZED_RECORD_ID
 *    GSF_FILE_SEEK_ERROR
 *    GSF_WRITE_ERROR
 *    GSF_HEADER_RECORD_ENCODE_FAILED
 *    GSF_SVP_RECORD_ENCODE_FAILED
 *    GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED
 *    GSF_SENSOR_PARAM_RECORD_ENCODE_FAILED
 *    GSF_COMMENT_RECORD_ENCODE_FAILED
 *    GSF_HISTORY_RECORD_ENCODE_FAILED
 *    GSF_NAV_ERROR_RECORD_ENCODE_FAILED
 *    GSF_FLUSH_ERROR
 *    GSF_SINGLE_BEAM_ENCODE_FAILED
 */

int OPTLK       gsfLoadScaleFactor(gsfScaleFactors *sf, int subrecordID, char c_flag, double precision, int offset);
/*
 * Description : gsfLoadScaleFactors should be used to load the swath
 *  bathymetry ping record scale factor structure.  This function assures
 *  that the multiplier and offset fields of the scale factor structure
 *  have a precision equal to that which will be stored in the gsf data file.
 *  This function should be called once for each beam array data type
 *  contained in your data.
 *
 * Inputs :
 *  sf = a pointer to the gsfScaleFactors structure to be loaded
 *  subrecordID = the subrecord id for the beam array data
 *  c_flag = the compression flag for the beam array
 *  precision = the presision to which the beam array data are to be stored
 *              (a value of 0.1 would indicate decimeter precision for depth)
 *  offset = the "DC" offset to scale the data by.
 *
 * Returns :
 *  This function returns zero if successful, or -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_TOO_MANY_ARRAY_SUBRECORDS
 */

int OPTLK gsfGetScaleFactor(int handle, int subrecordID, unsigned char *c_flag, double *multiplier, double *offset);
/*
 * Description : gsfGetScaleFactor may be used to obtain the multiplier
 *  and DC offset values by which each swath bathymetry ping array subrecord
 *  is be scaled. gsfGetScalesFactor must be called once for each array
 *  subrecord of interest.  At leat one swath bathymetry ping record
 *  must have been read from, or written to the file specified by handle.
 *
 * Inputs :
 *  handle = the integer value set by a call to gsfOpen.
 *  subrecordID = an integer value containing the subrecord id of the requested scale factors
 *  c_flag = the address of an unsigned character to contain the the compression flag
 *  multiplier = the address of a double to contain the scaling multiplier
 *  offset = the address of a double to contain the scaling DC offset.
 *
 * Returns :
 *  This function returns zero if successful, or -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_TOO_MANY_ARRAY_SUBRECORDS
 *    GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER
 */

void OPTLK      gsfFree(gsfRecords *rec);
/*
 * Description : This function frees all dynamically allocated memory
 *    from a gsfRecords data structure, and it then clears all the
 *    data elements in the structure.
 *
 * Inputs :
 *    gsfRecords *rec = a pointer to ta gsfRecords data structure
 *
 * Returns : none
 *
 * Error Conditions : none
 */

void OPTLK      gsfPrintError(FILE * fp);
/* Description : This function is used to print a short message describing
 *  the most recent error encountered.  This function need only be called if
 *  a -1 is returned from one of the gsf functions.
 *
 * Inputs :
 *  fp = a pointer to a FILE to which to write the message.
 *
 * Returns : none
 *
 * Error Conditions : none
 */

char *gsfStringError(void);
/* Description : This function is used to return a short message describing
 *  the most recent error encountered.  This function need only be called if
 *  a -1 is returned from one of the gsf functions.
 *
 * Inputs : none
 *
 * Returns : none
 *
 * Error Conditions : none
 */

int OPTLK       gsfIndexTime(int, int, int, time_t *, long *);
/* Description : This function returns the time (Posix.4) associated with
 *  a specified record number and type.  It also returns the record number
 *  that was read.
 *
 * Inputs :
 *  handle = gsf file handle assigned by gsfOpen or gsfOpenBuffered
 *  record_type = record type to be retrieved
 *  record_number = record number to be retrieved (-1 will get the time
 *                  and record number of the last record of this type)
 *  sec = Posix.4 seconds
 *  nsec = Posix.4 nanoseconds
 *
 * Returns :
 *  This function returns the record number if successful, or -1 if an
 *  error occured.
 *
 * Error Conditions :
 *    GSF_RECORD_TYPE_NOT_AVAILABLE
 */

int OPTLK       gsfPercent (int handle);
/* Description : This function returns an integer value representing
 *  the location of the file pointer as a percentage of the total file
 *  size.  It may be used to obtain an indication of how far along a
 *  program is in reading a gsf data file.  The file size is obtained
 *  when the file is opened.
 *
 * Inputs :
 *  handle = gsf file handle assigned by gsfOpen or gsfOpenBuffered
 *
 * Returns :
 *  This function returns the current file position as a percentage of
 *  the file size, or -1 if an error occurred. gsfError will be set to
 *  indicate the error.
 *
 * Error Conditions :
 *  GSF_BAD_FILE_HANDLE
 *  GSF_FILE_TELL_ERROR
 */

int OPTLK       gsfGetNumberRecords (int handle, int desiredRecord);
/* Description : This function will return the number of records of a
 *  given type to the caller. The number of records is retreived from
 *  the index file, so the file must have been opened for direct
 *  access (GSF_READONLY_INDEX, or GSF_UPDATE_INDEX).
 *
 * Inputs :
 *    handle = the handle to the file as provided by gsfOpen
 *    desiredRecord = the desired record or GSF_NEXT_RECORD
 *
 * Returns :
 *  This function returns the number of records of type desiredRecord
 *  contained in the GSF file designated by handle, or -1 if an error
 *  occured.
 *
 * Error Conditions :
 *  GSF_BAD_FILE_HANDLE
 *  GSF_BAD_ACCESS_MODE
 */

int OPTLK       gsfCopyRecords (gsfRecords *target, gsfRecords *source);
/* Description : This function will copy all of the data contained in the
 *  source gsfRecords data structure to the target gsfRecords data
 *  structure. The target MUST be memset to zero before the first call to
 *  gsfCopyRecords.  This function allocates dynmanic memory which is NOT
 *  maintained by the library.  It is up to the calling application to
 *  release the memory allocated.  This may be done by maintaining the
 *  target data structure as static data, or by using gsfFree to release
 *  the memory.
 *
 * Inputs :
 *  target = a pointer to a gsfRecords data structure allocated by the
 *      calling application, into which the source data is to be copied.
 *  source = a pointer to a gsfRecords data structure allocated by the
 *      calling application, from which data is to be copied.
 *
 * Returns : This function returns zero if successful, or -1 if an error
 *  occurs.
 *
 * Error Conditions :
 *  GSF_MEMORY_ALLOCATION_FAILED
 */

int OPTLK       gsfPutMBParams(gsfMBParams *p, gsfRecords *rec, int handle, int numArrays);
/* Description : This function moves swath bathymetry sonar processing
 *    parameters from internal form to "KEYWORD=VALUE" form.  The internal
 *    form parameters are read from an MB_PARAMETERS data structure maintained
 *    by the caller.  The "KEYWORD=VALUE" form parameters are written into the
 *    processing_parameters structure of the gsfRecords data structure
 *    maitained by the caller. Parameters for up to two pairs of
 *    transmit/receive arrays are supported, for systems such as Reson SeaBat
 *    9002.
 *
 * Inputs :
 *     p = a pointer to the gsfMBParams data structure which contains
 *         the parameters in internal form.
 *     rec = a pointer to the gsfRecords data structure into which the
 *         parameters are to be written in the "KEYWORK=VALUE" form.
 *     handle = the integer handle to the file set by gsfOpen.
 *     numArrays = the integer value specifying the number of pairs of
 *         arrays which need to have seperate parameters tracked.
 *
 * Returns : This function returns zero if successful, or -1 if an error
 *  occurs.
 *
 * Error Conditions :
 *     GSF_MEMORY_ALLOCATION_FAILED
 *     GSF_PARAM_SIZE_FIXED
 */

int OPTLK       gsfGetMBParams(gsfRecords *rec, gsfMBParams *p, int *numArrays);
/* Description : This function moves swath bathymetry sonar processing
 *    parameters from external, form to internal form.  The external
 *    "KEYWORD=VALUE" format parameters are read from a processing_params
 *    structure of a gsfRecords data structure maintained by the caller.
 *    The internal form parameters are written into a gsfMBParams data
 *    structure maintained by the caller. Parameters for up to two pairs of
 *    transmit/receive arrays are supported, for systems such as Reson SeaBat
 *    9002.
 *
 * Inputs :
 *     rec = a pointer to the gsfRecords data structure from which the
 *         parameters in "KEYWORK=VALUE" form are to be read.
 *     p = a pointer to the gsfMBParams data structure which will be populated.
 *     numArrays = the integer value specifying the number of pairs of
 *         arrays which need to have seperate parameters tracked.
 *
 * Returns : This function returns zero if successful, or -1 if an error
 *  occurs.
 *
 * Error Conditions :
 *  none.
 */

int OPTLK       gsfGetSwathBathyBeamWidths(gsfRecords *data, double *fore_aft, double *athwartship);
/* Description : This function returns to the caller the fore-aft and
 *    the port-starboard beam widths in degrees for a swath bathymetry
 *    multibeam sonar, given a gsfRecords data structure which contains
 *    a populated gsfSwathBathyPing structure.
 *
 * Inputs :
 *     data = The address of a gsfRecords data structure maintained by the
 *         caller which contains a populated gsfSwathBathyPing substructure.
 *     fore_aft = The address of a double allocated by the caller which will
 *         be loaded with the sonar's fore/aft beam width in degrees.
 *     athwartship = The address of a double allocated by the caller which will
 *         be loaded with the sonar's athwartship beam width in degrees.
 *
 * Returns : This function returns zero if successful, or -1 if an error
 *     occured.
 *
 * Error Conditions : unrecognized sonar id or mode.
 */

int OPTLK gsfIsStarboardPing(gsfRecords *data);
/* Description : This function uses the sonar specific data union
 *     of a gsfSwathBathymetry ping structure to determine if the ping
 *     is from the starboard arrays of a multibeam installation with
 *     dual transmit receive sonar arrays.
 *
 * Inputs :
 *     data = The address of a gsfRecords data structure maintained by the
 *         caller which contains a populated gsfSwathBathyPing substructure.
 *
 * Returns : This function returns non-zero if the ping contained in the
 *     passed data represents a starboard looking ping from a dual headed
 *     sonar installation. Otherwise, zero is returned.
 *
 * Error Conditions : unrecognized sonar id or mode.
 */

int OPTLK gsfLoadDepthScaleFactorAutoOffset(gsfSwathBathyPing *ping, int subrecordID, int reset, double min_depth, double max_depth, double *last_corrector, char c_flag, double precision);
/* Description : gsfLoadDepthScaleFactorAutoOffset should be used to load
 *  the scale factors for the depth subrecords of the swath bathymetry ping
 *  record scale factor structure. The approach uses the tide and depth
 *  correction fields to help establish the offset component of the scale
 *  factor such that negative depth values may be supported.  Negative
 *  depth values may be encountered when surveying above the tidal datum.
 *  In addition, systems mounted on subsea platforms may support their
 *  native precision even in deep water.
 *
 * Inputs :
 *  ping = A pointer to the gsfSwathBathyPing which contains the depht
 *      and tide correction values, and the scale factors data structure.
 *  subrecordID = the subrecord id for the beam array data.  This must be
 *      either GSF_SWATH_BATHY_SUBRECORD_DEPTH_ARRAY, or
 *      GSF_SWATH_BATHY_SUBRECORD_NOMINAL_DEPTH_ARRAY
 *  c_flag = The compression flag for the beam array
 *  precision = The presision to which the beam array data are to be stored
 *      (a value of 0.1 would indicate decimeter precision for depth)
 *  reset = An integer value which will cause the internal logic to be
 *      refreshed when the value is non-zero.  The first call to this function
 *      should use a non-zero reset, from then on, this value may be passed
 *      as zero.
 *  last_corrector = The address of a double value stored as permanent memory.
 *      Successive calls to this function must pass the same address for this
 *      argument.
 *
 * Returns :
 *  This function returns zero if successful, or -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_TOO_MANY_ARRAY_SUBRECORDS
 *
 */

int OPTLK gsfGetSwathBathyArrayMinMax(gsfSwathBathyPing *ping, int subrecordID, double *min_value, double *max_value);
/* Description : This function may be used to obtain the minimum and maximum
 *  supportable values for each of the swath bathymetry arrays.  The minimum
 *  and maximum values are determined based on the scale factors and the array
 *  type.
 *
 * Inputs :
 *  ping = A pointer to the gsfSwathBathyPing which contains the depht
 *      and tide correction values, and the scale factors data structure.
 *  subrecordID = The subrecord id for the beam array data.  This must be
 *      either GSF_SWATH_BATHY_SUBRECORD_DEPTH_ARRAY, or
 *      GSF_SWATH_BATHY_SUBRECORD_NOMINAL_DEPTH_ARRAY
 *  min_value = The address of a double value allocated by the caller into
 *      which will be placed the minimum value which may be represented for
 *      this array type.
 *  max_value = The address of a double value allocated by the caller into
 *      which will be placed the maximum value which may be represented for
 *      this array type.
 *
 * Returns :
 *  This function returns zero if successful, or -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_UNRECOGNIZED_ARRAY_SUBRECORD_ID
 *    GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER
 */

#endif /* __GSF_H__ */
