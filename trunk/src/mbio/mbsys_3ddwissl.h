/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_3ddwissl.h	12/19/2017
 *	$Id$
 *
 *    Copyright (c) 2018-2018 by
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
 * mbsys_3ddwissl.h defines the MBIO data structures for handling data from
 * the 3DatDepth WiSSL (wide swath lidar) submarine lidar:
 *      MBF_3DDWISSL : MBIO ID 232 - 3DatDepth WiSSL vendor format
 *
 * Author:	David W. Caress
 * Date:	December 19, 2017
 *
 */
/*
 * Notes on the MBSYS_3DATDEPTHWISSL data structure:
 *
 * Vendor format from 3D at Depth for the WiSSL (wide swath lidar) submarine
 * lidar system delivered to MBARI in December 2017.
 *
 * Initial coding done using the WiSSL Wide Swath Subsea LiDAR Software User
 * Manual version 1.2 from December 2017.
 *
 *--------------------------------------------------------------------------------
 * Range Angle Angle data format (binary)
 * 3D at Depth raw WiSSL data format
 *              Item	                                Value	            Bytes
 * ---------------------------------------------------------------------------------------
 * File Header
 *           Record ID – WiSSL                             0x3D47   2 (1 UINT16)
 *           File Magic Number                             0x3D08   2 (1 UINT16)
 *           File version                                  1        2 (1 UINT16)
 *           File sub version                              1        2 (1 UINT16)
 *           
 * Scan Information
 *           AZ, Cross track angle start, typical (deg)             4 (1 float32)
 *           AZ, Cross track angle end, typical (deg)               4 (1 float32)
 *           Pulses per cross track, scan line                      2 (1 UINT16)
 *           Number pulses per LOS                                  1 (1 UINT8)
 *           Scan lines per this File, Head A                       2 (1 UINT16)
 *           Scan lines per this File, Head B                       2 (1 UINT16)
 *           
 * Calibration Information
 *           Calibration Structure, Head A                          Size of calibration structure
 *           Calibration Structure, Head B                          Size of calibration structure
 * 
 * Pulse ID and Timestamp ( 1 to n Scans )
 *           Record ID – Head A or B              0x3D53, 0x3D54    2 (1 UINT16)
 *           Timestamp year (true year)                             2 (1 UINT16)
 *           Timestamp month (1-12)                                 1 (1 UINT8)
 *           Timestamp day                                          1 (1 UINT8)
 *           Timestamp days since Jan 1                             2 (1 UINT16)
 *           Timestamp hour                                         2 (1 UINT16)
 *           Timestamp minutes                                      1 (1 UINT8)
 *           Timestamp seconds                                      1 (1 UINT8)
 *           Timestamp nano seconds                                 4 (1 UINT32)
 *           Gain (laser power)                                     1 (UINT8)
 *           Digitizer temperature C                                4 (float)
 *           CTD temperature C                                      4 (float)
 *           CTD salinity psu                                       4 (float)
 *           CTD pressure dbar                                      4 (float)
 *           Index                                                  4 (float)
 *           Start processing m                                     4 (float)
 *           End processing m                                       4 (float)
 *           Pulse Count this scan line                             4 (1 UINT32)
 *
 * Laser Pulse Data ( 1 to m pulses per scan )
 *           AZ, Cross track angle (deg)                            4 (1 float32)
 *           EL, Forward track angle (deg)                          4 (1 float32)
 *           AZ, Cross track offset (m)                             4 (1 float32)
 *           EL, Forward track offset (m)                           4 (1 float32)
 *           Pulse time offset (sec)                                4 (1 float32)
 *           LOS Range 1 ( from glass front ) meters                4 (1 float32)
 *           ...
 *           LOS Range n ( from glass front ) meters                4 (1 float32) 
 *           Amplitude LOS 1 / peak of signal                       2 (1 UINT16)
 *           ...
 *           Amplitude LOS n / peak of signal                       2 (1 UINT16)
 *
 *
 * Each RAA file begins with a File Header, followed by a “Scan Information”
 * block and a “Calibration Information” block of data. Then, the file contains
 * scan line data. The data for each scan line contains: a Record ID (head
 * designator), a full timestamp, and a” Laser Pulse Data” collection of data.
 * Note, Head A and B scanlines are interleaved in the RAA file per their
 * specific time stamps.
 *
 * For example, if the sensor was configured for 250 pulses per scan line and
 * 3 LOS range measurements per pulse, the following data would be present in
 * the RAA file:
 *      File Header
 *      Scan Information
 *      Calibration Information Head A
 *      Calibration Information Head B
 *          (1) Record ID (A or B)
 *              Scan Timestamp and characteristics
 *              Pulse count this scan line
 *                  (1) Laser Pulse Data:
 *                      AZ angle
 *                      EL angle
 *                      AZ offset
 *                      EL offset
 *                      Pulse time offset
 *                      Range Data:
 *                          LOS Range 1
 *                          LOS Range 2
 *                          LOS Range 3
 *                      Intensity Data:
 *                          Intensity 1
 *                          Intensity 2
 *                          Intensity 3
 *                      ...
 *                  (250) Laser Pulse Data:
 *                      AZ angle
 *                      EL angle
 *                      AZ offset
 *                      EL offset
 *                      Pulse time offset
 *                      Range Data:
 *                          LOS Range 1
 *                          LOS Range 2
 *                          LOS Range 3
 *                      Intensity Data:
 *                          Intensity 1
 *                          Intensity 2
 *                          Intensity 3
 *                      ...
 *
 * Note: based on laser head performance, differing counts of data sets may
 * exist for Head A and B. The “.raa” file extension is used for the binary file.
 *
 *--------------------------------------------------------------------------------
 *
 * Processing WiSSL Data Format
 * ----------------------------
 * The file header, scan information, and calibration information sections are
 * the same as for the raw format. The survey data records differ from those of
 * the raw format in three respects:
 *   1) The size in bytes of the scan record, minus 4, is stored as an unsigned
 *      short int immediately following the record id.
 *   2) Only non-null soundings as defined by preprocessing are stored - many
 *      low amplitude picks may be discarded.
 *   3) The soundings are stored sequentially.
 *   4) The soundings include calculated bathymetry values and the pulse and LOS
 *      pick numbers.
 *
 * ---------------------------------------------------------------------------------------
 * File Header
 *           Record ID – WiSSL                             0x3D47   2 (1 UINT16)
 *           File Magic Number                             0x3D08   2 (1 UINT16)
 *           File version                                  1        2 (1 UINT16)
 *           File sub version                              1        2 (1 UINT16)
 *           
 * Scan Information
 *           AZ, Cross track angle start, typical (deg)             4 (1 float32)
 *           AZ, Cross track angle end, typical (deg)               4 (1 float32)
 *           Pulses per cross track, scan line                      2 (1 UINT16)
 *           Number pulses per LOS                                  1 (1 UINT8)
 *           Scan lines per this File, Head A                       2 (1 UINT16)
 *           Scan lines per this File, Head B                       2 (1 UINT16)
 *           
 * Calibration Information
 *           Calibration Structure, Head A                          450 bytes
 *           Calibration Structure, Head B                          450 bytes
 * 
 * Pulse ID and Timestamp ( 1 to n Scans )
 *           Record ID – Head A or B              0x3D53, 0x3D54    2 (1 UINT16)
 *           Timestamp year (true year)                             2 (1 UINT16)
 *           Timestamp month (1-12)                                 1 (1 UINT8)
 *           Timestamp day                                          1 (1 UINT8)
 *           Timestamp days since Jan 1                             2 (1 UINT16)
 *           Timestamp hour                                         2 (1 UINT16)
 *           Timestamp minutes                                      1 (1 UINT8)
 *           Timestamp seconds                                      1 (1 UINT8)
 *           Timestamp nano seconds                                 4 (1 UINT32)
 *           Gain (laser power)                                     1 (UINT8)
 *           Digitizer temperature C                                4 (float)
 *           CTD temperature C                                      4 (float)
 *           CTD salinity psu                                       4 (float)
 *           CTD pressure dbar                                      4 (float)
 *           Index                                                  4 (float)
 *           Start processing m                                     4 (float)
 *           End processing m                                       4 (float)
 *           Pulse Count this scan line                             4 (1 UINT32)
 *
 * Laser Pulse Data ( 1 to m pulses per scan )
 *           AZ, Cross track angle (deg)                            4 (1 float32)
 *           EL, Forward track angle (deg)                          4 (1 float32)
 *           AZ, Cross track offset (m)                             4 (1 float32)
 *           EL, Forward track offset (m)                           4 (1 float32)
 *           Pulse time offset (sec)                                4 (1 float32)
 *           LOS Range 1 ( from glass front ) meters                4 (1 float32)
 *           ...
 *           LOS Range n ( from glass front ) meters                4 (1 float32) 
 *           Amplitude LOS 1 / peak of signal                       2 (1 UINT16)
 *           ...
 *           Amplitude LOS n / peak of signal                       2 (1 UINT16)
 *
 *
 * Each RAA file begins with a File Header, followed by a “Scan Information”
 * block and a “Calibration Information” block of data. Then, the file contains
 * scan line data. The data for each scan line contains: a Record ID (head
 * designator), a full timestamp, and a” Laser Pulse Data” collection of data.
 * Note, Head A and B scanlines are interleaved in the RAA file per their
 * specific time stamps.
 *
 * For example, if the sensor was configured for 250 pulses per scan line and
 * 3 LOS range measurements per pulse, the following data would be present in
 * the RAA file:
 *      File Header
 *      Scan Information
 *      Calibration Information Head A
 *      Calibration Information Head B
 *          (1) Record ID (A or B)
 *              Scan Timestamp and characteristics
 *              Pulse count this scan line
 *                  (1) Laser Pulse Data:
 *                      AZ angle
 *                      EL angle
 *                      AZ offset
 *                      EL offset
 *                      Pulse time offset
 *                      Range Data:
 *                          LOS Range 1
 *                          LOS Range 2
 *                          LOS Range 3
 *                      Intensity Data:
 *                          Intensity 1
 *                          Intensity 2
 *                          Intensity 3
 *                      ...
 *                  (250) Laser Pulse Data:
 *                      AZ angle
 *                      EL angle
 *                      AZ offset
 *                      EL offset
 *                      Pulse time offset
 *                      Range Data:
 *                          LOS Range 1
 *                          LOS Range 2
 *                          LOS Range 3
 *                      Intensity Data:
 *                          Intensity 1
 *                          Intensity 2
 *                          Intensity 3
 *                      ...
 *
 * Note: based on laser head performance, differing counts of data sets may
 * exist for Head A and B. The “.raa” file extension is used for the binary file.
 *
 *--------------------------------------------------------------------------------
 *--------------------------------------------------------------------------------
 *
 *
 */

/* include mb_define.h */
#ifndef MB_DEFINE_DEF
#include "mb_define.h"
#endif

/* defines */
#define MBF_3DWISSLR_MAGICNUMBER 0x3D08        /* '=', backspace */
#define MBF_3DWISSLP_MAGICNUMBER 0x3D09        /* '=', tab */
#define MBSYS_3DDWISSL_RECORD_FILEHEADER 0x3D47   /* '=''G' */
#define MBSYS_3DDWISSL_RECORD_RAWHEADA 0x3D53    /* '=''S' */
#define MBSYS_3DDWISSL_RECORD_RAWHEADB 0x3D54    /* '=''T' */
#define MBSYS_3DDWISSL_RECORD_COMMENT 0x3D43     /* '=''C' */
#define MBSYS_3DDWISSL_RECORD_PROHEADA 0x3D73    /* '=''s' */
#define MBSYS_3DDWISSL_RECORD_PROHEADB 0x3D74    /* '=''t' */
#define MBSYS_3DDWISSL_V1S1_PARAMETER_SIZE            23
#define MBSYS_3DDWISSL_V1S1_CALIBRATION_SIZE          450
#define MBSYS_3DDWISSL_V1S1_MAX_SOUNDINGS_PER_PULSE   5
#define MBSYS_3DDWISSL_V1S1_RAW_SCAN_HEADER_SIZE      49
#define MBSYS_3DDWISSL_V1S1_RAW_PULSE_HEADER_SIZE     20
#define MBSYS_3DDWISSL_V1S1_RAW_SOUNDING_SIZE         6
#define MBSYS_3DDWISSL_V1S1_PRO_SCAN_HEADER_SIZE      100
#define MBSYS_3DDWISSL_V1S1_PRO_PULSE_HEADER_SIZE     66
#define MBSYS_3DDWISSL_V1S1_PRO_SOUNDING_SIZE         22
    
    /* Instrument geometry for dual optical heads - the sensor reference point
     * is the midpoint on bottom of the mounting bracketry  as per the WiSSL
     * mechanical ICD. The raw data all reference ranges to the center point of the
     * front of the glass on each optical head. Here are the distance offsets of
     * those points to the sensor reference point:
     * 
     *   Head A (aft mounted, pointed to port):
     *     dx (acrosstrack): -0.48126 inches = -0.012224004 m
     *     dy (alongtrack):  -4.73551 inches = -0.120281954 m
     *     dz (vertical:     -2.44115 inches = -0.062005210 m
     *     droll (in xz plane, + to starboard): +22.08 degrees
     *     dpitch (in yz plane, + forward): -4.68
     *     
     *   Head B (forward mounted, pointed to starboard):
     *     dx (acrosstrack): +0.48126 inches = +0.012224004 m
     *     dy (alongtrack):  +4.73551 inches = +0.120281954 m
     *     dz (vertical:     -2.44115 inches = -0.062005210 m
     *     droll (in xz plane, + to starboard): -22.08 degrees
     *     dpitch (in yz plane, + forward): -5.01
     */
//#define MBSYS_3DDWISSL_HEADA_OFFSET_X_M           +0.012224004
//#define MBSYS_3DDWISSL_HEADA_OFFSET_Y_M           -0.120281954
//#define MBSYS_3DDWISSL_HEADA_OFFSET_Z_M           +0.062005210
//#define MBSYS_3DDWISSL_HEADA_OFFSET_HEADING_DEG   -0.0         // ICD value 0.0
//#define MBSYS_3DDWISSL_HEADA_OFFSET_ROLL_DEG      -22.08        // ICD value -22.08
//#define MBSYS_3DDWISSL_HEADA_OFFSET_PITCH_DEG     -4.68         // ICD value -4.68
//#define MBSYS_3DDWISSL_HEADB_OFFSET_X_M           -0.012224004
//#define MBSYS_3DDWISSL_HEADB_OFFSET_Y_M           +0.120281954
//#define MBSYS_3DDWISSL_HEADB_OFFSET_Z_M           +0.062005210
//#define MBSYS_3DDWISSL_HEADB_OFFSET_HEADING_DEG   +0.00         // ICD value 0.0
//#define MBSYS_3DDWISSL_HEADB_OFFSET_ROLL_DEG      +22.08        // ICD value +22.08
//#define MBSYS_3DDWISSL_HEADB_OFFSET_PITCH_DEG     -5.01         // ICD value -5.01

#define MBSYS_3DDWISSL_FILEHEADER                 0
#define MBSYS_3DDWISSL_HEADA                      1
#define MBSYS_3DDWISSL_HEADB                      2
#define MBSYS_3DDWISSL_COMMENT                    3
#define MBSYS_3DDWISSL_HEADA_OFFSET_X_M           +0.012224004
#define MBSYS_3DDWISSL_HEADA_OFFSET_Y_M           -0.120281954
#define MBSYS_3DDWISSL_HEADA_OFFSET_Z_M           +0.062005210
#define MBSYS_3DDWISSL_HEADA_OFFSET_HEADING_DEG   -1.20         // ICD value 0.0
#define MBSYS_3DDWISSL_HEADA_OFFSET_ROLL_DEG      -21.440       // ICD value -22.08
#define MBSYS_3DDWISSL_HEADA_OFFSET_PITCH_DEG     -5.440        // ICD value -4.68
#define MBSYS_3DDWISSL_HEADB_OFFSET_X_M           -0.012224004
#define MBSYS_3DDWISSL_HEADB_OFFSET_Y_M           +0.120281954
#define MBSYS_3DDWISSL_HEADB_OFFSET_Z_M           +0.062005210
#define MBSYS_3DDWISSL_HEADB_OFFSET_HEADING_DEG   +1.20         // ICD value 0.0
#define MBSYS_3DDWISSL_HEADB_OFFSET_ROLL_DEG      +21.440       // ICD value +22.08
#define MBSYS_3DDWISSL_HEADB_OFFSET_PITCH_DEG     -4.230        // ICD value -5.01  pitch too much 1.5 cm = 0.143 deg, roll too much 2.5 cm = 0.239 deg

#define MBSYS_3DDWISSL_DEFAULT_AMPLITUDE_THRESHOLD 2000.0
#define MBSYS_3DDWISSL_DEFAULT_TARGET_ALTITUDE        0.0
#define MBSYS_3DDWISSL_LASERPULSERATE               40000.0

struct mbsys_3ddwissl_calibration_struct {
    char cfg_path[ 64 ];
    int laser_head_no;      // either 1 or 2
    int process_for_air;        // 1 = air, else water
    mb_u_char temperature_compensation;
    mb_u_char emergency_shutdown;
    float ocb_temperature_limit_c;
    float ocb_humidity_limit;
    float pb_temperature_limit_1_c;
    float pb_temperature_limit_2_c;
    float pb_humidity_limit;
    float dig_temperature_limit_c;
    char l_d_cable_set[ 24 ];
    char ocb_comm_port[ 24 ];
    char ocb_comm_cfg [ 24 ];
    float az_ao_deg_to_volt;
    float az_ai_neg_v_to_deg;
    float az_ai_pos_v_to_deg;
    float t1_air;
    float ff_air;
    float t1_water_g4000;
    float ff_water_g4000;
    float t1_water_g3000;
    float ff_water_g3000;
    float t1_water_g2000;
    float ff_water_g2000;
    float t1_water_g1000;
    float ff_water_g1000;
    float t1_water_g400;
    float ff_water_g400;
    float t1_water_g300;
    float ff_water_g300;
    float t1_water_secondary_g4000;
    float ff_water_secondary_g4000;
    float t1_water_secondary_g3000;
    float ff_water_secondary_g3000;
    float t1_water_secondary_g2000;
    float ff_water_secondary_g2000;
    float t1_water_secondary_g1000;
    float ff_water_secondary_g1000;
    float t1_water_secondary_g400;
    float ff_water_secondary_g400;
    float t1_water_secondary_g300;
    float ff_water_secondary_g300;
    double temp_comp_poly2;
    double temp_comp_poly1;
    double temp_comp_poly;
    float laser_start_time_sec;
    float scanner_shift_cts;
    float factory_scanner_lrg_deg;
    float factory_scanner_med_deg;
    float factory_scanner_sml_deg;
    float el_angle_fixed_deg;
    char unused[116];
};

struct mbsys_3ddwissl_sounding_struct {
    /* raw information */
	float range;                    /* meters from glass front */
	short amplitude;                /* peak of signal - to 1023 */

	/* processed information incorporating pulse offsets and head offsets */
	mb_u_char beamflag; /* MB-System beam flag */
	float acrosstrack;  /* acrosstrack distance relative to overall sensor reference point (meters) */
	float alongtrack;   /* alongtrack distance relative to overall sensor reference point (meters) */
	float depth;        /* depth relative to overall sensor reference point (meters) */
};

struct mbsys_3ddwissl_pulse_struct {
    /* raw information */
    float angle_az; /* AZ, Cross track angle, (deg) */
    float angle_el; /* AZ, Forward track angle, (deg) */
    float offset_az; /* AZ, Cross track offset, (m) */
    float offset_el; /* AZ, Forward track offset, (m) */
    float time_offset; /* Pulse time offset (sec) */

	/* processed information */
	double time_d;              /* epoch time */
	double acrosstrack_offset;  /* relative to start of scan using position
                                 * and heading at start of scan */
	double alongtrack_offset;   /* relative to start of scan using position
                                 * and heading at start of scan */
	double sensordepth_offset;  /* relative to start of scan using position
                                 * and heading at start of scan */
	float heading_offset;       /* relative to start of scan using position
                                 * and heading at start of scan */
	float roll_offset;          /* relative to start of scan using position
                                 * and heading at start of scan */
	float pitch_offset;         /* relative to start of scan using position
                                 * and heading at start of scan */

    /* soundings */
    int validsounding_count;    /* number of the soundings valid (non-null) for this pulse */
    struct mbsys_3ddwissl_sounding_struct soundings[MBSYS_3DDWISSL_V1S1_MAX_SOUNDINGS_PER_PULSE];
};

/* 3DatDepth LIDAR data structure */
struct mbsys_3ddwissl_struct {
    
	/* Type of data record */
	int kind; /* MB-System record ID */

	/* File Header */
    unsigned short parameter_id;    /* 0x3D47 */
    unsigned short magic_number;    /* 0x3D08 */
	unsigned short file_version;    /* 1 */
	unsigned short sub_version;     /* 1 = initial version from 3DatDepth, extended for MB-System */
    
    /* Scan Information */
    float cross_track_angle_start; /* AZ, Cross track angle start, typical (deg) */
    float cross_track_angle_end; /* AZ, Cross track angle end, typical (deg) */
    unsigned short pulses_per_scan; /* Pulses per cross track, scan line */
    unsigned short soundings_per_pulse; /* soundings per pulse (line of sight, or LOS) */
	unsigned short heada_scans_per_file; /* number of heada scans in this file */
	unsigned short headb_scans_per_file; /* number of headb scans in this file */

    /* WiSSL optical head positional and angular offsets */
    double heada_offset_x_m;                    /* head A x offset (m) -0.012224004 */
    double heada_offset_y_m;                    /* head A y offset (m) -0.120281954 */
    double heada_offset_z_m;                    /* head A z offset (m) -0.062005210 */
    double heada_offset_heading_deg;            /* head A heading offset (degrees) +22.08 */
    double heada_offset_roll_deg;               /* head A roll offset (degrees) +22.08 */
    double heada_offset_pitch_deg;              /* head A pitch offset (degrees) -4.68 */
    double headb_offset_x_m;                    /* head B x offset (m) +0.012224004 */
    double headb_offset_y_m;                    /* head B y offset (m) +0.120281954 */
    double headb_offset_z_m;                    /* head B z offset (m) -0.062005210 */
    double headb_offset_heading_deg;            /* head B heading offset (degrees) -22.08 */
    double headb_offset_roll_deg;               /* head B roll offset (degrees) -22.08 */
    double headb_offset_pitch_deg;              /* head B pitch offset (degrees) -5.01 */
    
    /* head A calibration */
    struct mbsys_3ddwissl_calibration_struct calibration_a;

    /* head B calibration */
    struct mbsys_3ddwissl_calibration_struct calibration_b;
    
	/* Scan information from raw records */
    unsigned short record_id;       /* head A (0x3D53 or 0x3D73) or head B (0x3D54 or 0x3D74) */
    unsigned int scan_size;         /* bytes of scan record minus 4 (record_id + scan_size) */
    unsigned short year;
    mb_u_char month;
    mb_u_char day;
    unsigned short jday;
    unsigned short hour;
    mb_u_char minutes;
    mb_u_char seconds;
    unsigned int nanoseconds;
    mb_u_char gain;                 /* laser power setting */
    mb_u_char unused;               /* unused */
    float digitizer_temperature;    /* digitizer temperature degrees C */
    float ctd_temperature;          /* ctd temperature degrees C */
    float ctd_salinity;             /* ctd salinity psu */
    float ctd_pressure;             /* ctd pressure dbar */
    float index;
    float range_start;              /* range start processing meters */
    float range_end;                /* range end processing meters */
    unsigned int pulse_count;       /* pulse count for this scan */

    /* merged navigation and attitude per each scan */
    double time_d;      /* epoch time - not in data file, calculated following reading */
	double navlon;      /* absolute position longitude (degrees) */
	double navlat;      /* absolute position latitude (degrees) */
	double sensordepth; /* absolute position depth below sea surface (meters), includes any tide correction */
	float speed;        /* lidar speed (m/s) */
	float heading;      /* lidar heading (degrees) */
	float roll;         /* lidar roll (degrees) */
	float pitch;        /* lidar pitch (degrees) */
    unsigned short validpulse_count;              /* number of valid (non-null) pulses stored in this record */
    unsigned short validsounding_count;           /* number of valid (non-null) soundings stored in this record */

	unsigned int scan_count;                    /* global scan count */
    unsigned int size_pulse_record_raw;         /* for original logged records
                                                 * - calculated from file header values */
    unsigned int size_pulse_record_processed;   /* for extended processed records
                                                 * -  calculated from file header values */
    unsigned int bathymetry_calculated;         /* flag regarding calculation of bathymetry */
	int num_pulses_alloc;      /* array allocated for this number of pulses */
    struct mbsys_3ddwissl_pulse_struct *pulses;

	/* comment */
	unsigned short comment_len;       /* comment length in bytes */
	char comment[MB_COMMENT_MAXLINE]; /* comment string */
};

/* System specific function prototypes */
int mbsys_3ddwissl_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_3ddwissl_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_3ddwissl_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss,
                                    int *error);
int mbsys_3ddwissl_pingnumber(int verbose, void *mbio_ptr, int *pingnumber, int *error);
int mbsys_3ddwissl_preprocess(int verbose, void *mbio_ptr, void *store_ptr, void *platform_ptr, void *preprocess_pars_ptr,
                                    int *error);
int mbsys_3ddwissl_sensorhead(int verbose, void *mbio_ptr, void *store_ptr, int *sensorhead, int *error);
int mbsys_3ddwissl_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                                 double *navlon, double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss,
                                 char *beamflag, double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack,
                                 double *ss, double *ssacrosstrack, double *ssalongtrack, char *comment, int *error);
int mbsys_3ddwissl_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d,
                                double navlon, double navlat, double speed, double heading, int nbath, int namp, int nss,
                                char *beamflag, double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack,
                                double *ss, double *ssacrosstrack, double *ssalongtrack, char *comment, int *error);
int mbsys_3ddwissl_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes,
                                double *angles, double *angles_forward, double *angles_null, double *heave,
                                double *alongtrack_offset, double *draft, double *ssv, int *error);
int mbsys_3ddwissl_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error);
int mbsys_3ddwissl_pulses(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *pulses, int *error);
int mbsys_3ddwissl_gains(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transmit_gain,
                               double *pulse_length, double *receive_gain, int *error);
int mbsys_3ddwissl_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                                          double *altitude, int *error);
int mbsys_3ddwissl_extract_nnav(int verbose, void *mbio_ptr, void *store_ptr, int nmax, int *kind, int *n, int *time_i,
                                      double *time_d, double *navlon, double *navlat, double *speed, double *heading,
                                      double *draft, double *roll, double *pitch, double *heave, int *error);
int mbsys_3ddwissl_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                                     double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll,
                                     double *pitch, double *heave, int *error);
int mbsys_3ddwissl_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
                                    double navlat, double speed, double heading, double draft, double roll, double pitch,
                                    double heave, int *error);
int mbsys_3ddwissl_extract_svp(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsvp, double *depth,
                                     double *velocity, int *error);
int mbsys_3ddwissl_insert_svp(int verbose, void *mbio_ptr, void *store_ptr, int nsvp, double *depth, double *velocity,
                                int *error);
int mbsys_3ddwissl_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error);
int mbsys_3ddwissl_print_store(int verbose, void *store_ptr, int *error);
int mbsys_3ddwissl_calculatebathymetry(int verbose, void *mbio_ptr, void *store_ptr,
                                double amplitude_threshold, double target_altitude, int *error);

/* functions called by mbpreprocess to fix first generation WiSSL timestamp errors */
int mbsys_3ddwissl_indextablefix(int verbose, void *mbio_ptr, int num_indextable,
                                 void *indextable_ptr, int *error);
int mbsys_3ddwissl_indextableapply(int verbose, void *mbio_ptr, int num_indextable,
                                   void *indextable_ptr, int n_file, int *error);










