/*--------------------------------------------------------------------
 *    The MB-system:  mbsys_3ddwissl.h  12/19/2017
 *
 *    Copyright (c) 2018-2023 by
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
/*
 * mbsys_3ddwissl.h defines the MBIO data structures for handling data from
 * the 3DatDepth WiSSL (wide swath lidar) submarine lidar:
 *      MBF_3DWISSLR : MBIO ID 232 - 3DatDepth WiSSL vendor format
 *      MBF_3DWISSLP : MBIO ID 233 - 3DatDepth WiSSL processing format
 *
 * Author:  David W. Caress
 * Date:  December 19, 2017
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
 * Modifications made using the WiSSL Wide Swath Subsea LiDAR Software User
 * Manual version 1.4 from May 2019.
 *
 * Two data formats are supported: all variations of the *.raa files logged by
 * the WiSSL are supported as MB-System format 232 (MBF_3DWISSLR), and the
 * processed WiSSL data written by MB-System are supported as format 233 (MBF_3DWISSLP).
 * The logged data have variants 1.1 and 1.2, with the newer variant changing
 * the calibration block size from 450 bytes to 407 bytes, and adding a 1-byte
 * value named diagnostic to each sounding. MB-System has defined an additional
 * variant 1.3 in which the calibration block of 1.2 is lengthened to 450 bytes
 * so that the file header size is the same as 1.1. When a logged *.raa file
 * in the 1.2 variant is read, the data are converted to 1.3. MB-System only
 * writes newer data in the 1.3 variant. Data in the original 1.1 form remain
 * in that form.
 *
 *--------------------------------------------------------------------------------
 * Range Angle Angle data format (binary)
 * 3D at Depth raw WiSSL data format
 * Supported as format 232 in MB-System
 *              Item                                  Value              Bytes
 * ---------------------------------------------------------------------------------------
 * File Header Record (923 bytes for 1.1 and 1.3, 837 bytes for 1.2):
 * ------------------
 *
 * File and version ID (8 bytes)
 *           Record ID – WiSSL                             0x3D47   2 (1 UINT16)
 *           File Magic Number                             0x3D08   2 (1 UINT16)
 *           File version                                  1        2 (1 UINT16)
 *           File sub version                              1 to 3   2 (1 UINT16)
 *
 * Scan Information (15 bytes)
 *           AZ, Cross track angle start, typical (deg)             4 (1 float32)
 *           AZ, Cross track angle end, typical (deg)               4 (1 float32)
 *           Np, Number pulses per cross track scan line            2 (1 UINT16)
 *           Nl, Number LOS (soundings) per pulse                   1 (1 UINT8)
 *           Na, Scan lines per this File, Head A                   2 (1 UINT16)
 *           Nb, Scan lines per this File, Head B                   2 (1 UINT16)
 *
 * Calibration Information (900 bytes for 1.1 & 1.3, 814 bytes for 1.2)
 *           Calibration Structure, Head A                          450 bytes for 1.1 & 1.3, 407 bytes for 1.2
 *           Calibration Structure, Head B                          450 bytes for 1.1 & 1.3, 407 bytes for 1.2
 *
 * ---------------------------------------------------------------------------------------
 * Scan Record (1 to N = Na + Nb scans):
 * -----------
 *
 * Scan Header (49 bytes)
 *           Record ID – Head A or B              0x3D53, 0x3D54    2 (1 UINT16)
 *           Timestamp (year (true year))                           2 (1 UINT16)
 *           Timestamp (month (1-12))                               1 (1 UINT8)
 *           Timestamp (day)                                        1 (1 UINT8)
 *           Timestamp (days since Jan 1)                           2 (1 UINT16)
 *           Timestamp (hours)                                      2 (1 UINT16)
 *           Timestamp (minutes)                                    1 (1 UINT8)
 *           Timestamp (seconds)                                    1 (1 UINT8)
 *           Timestamp (nano seconds)                               4 (1 UINT32)
 *           Gain (laser power)                                     1 (1 UINT8)
 *           Digitizer temperature (degrees C)                      4 (1 float32)
 *           CTD temperature (degrees C)                            4 (1 float32)
 *           CTD salinity (psu)                                     4 (1 float32)
 *           CTD pressure (dbar)                                    4 (1 float32)
 *           Index of refraction                                    4 (1 float32)
 *           Start processing range (meters)                        4 (1 float32)
 *           End processing range  (meters)                         4 (1 float32)
 *           Np, Number of pulses in this scan                      4 (1 UINT32)
 *
 * Laser Pulse Data ( 1 to Np pulses per scan, each pulse has Nl soundings )
 *   Pulse Header (20 bytes)
 *           AZ, Cross track angle (deg)                            4 (1 float32)
 *           EL, Forward track angle (deg)                          4 (1 float32)
 *           AZ, Cross track offset (m)                             4 (1 float32)
 *           EL, Forward track offset (m)                           4 (1 float32)
 *           Pulse time offset (sec)                                4 (1 float32)
 *   Values for LOS (6 bytes per sounding for 1.1, 7 bytes per sounding >= 1.2)
 *           Sounding 1 Range ( from glass front ) meters           4 (1 float32)
 *           ...
 *           Sounding Nl Range ( from glass front ) meters          4 (1 float32)
 *           Sounding 1 Amplitude / peak of signal                  2 (1 UINT16)
 *           ...
 *           Sounding Nl Amplitude / peak of signal                 2 (1 UINT16)
 *           Sounding 1 Diagnostic value (if version >= 1.2):       1 (1 UINT8)
 *           ...
 *           Sounding Nl Diagnostic value (if version >= 1.2):      1 (1 UINT8)
 *
 * Each RAA file begins with a File Header record that includes a “Scan Information”
 * block and two “Calibration Information” blocks, one for each optical head.
 * There may be an arbitrary number of comment records following the File Header,
 * but before the scan data. The scan line data interleaves the Head A and Head B
 * scans with no guarentee that the order is consistent with increasing time stamp.
 * Also, differing scan counts will generally exist for Head A and B.
 * The data for each scan line contains: a Record ID (head designator), a full
 * timestamp, and a” Laser Pulse Data” collection of data.
 * The file suffix of files logged by the WiSSL will be *.raa. Files written by
 * MB-System will typically have a ".mb232" suffix. Either suffix is valid.
 *
 * For example, if the sensor was configured for 250 pulses per scan line and
 * 3 LOS range measurements per pulse, the following data would be present in
 * the RAA file:
 *      File Header Record
 *        Scan Information
 *        Calibration Information Head A (450 bytes for 1.1 & 1.3, 407 bytes for 1.2)
 *        Calibration Information Head B (450 bytes for 1.1 & 1.3, 407 bytes for 1.2)
 *      Comment Record 0
 *      ......
 *      Comment Record Nc - 1
 *      Scan Record 1 (A or B)
 *          Scan Timestamp and characteristics
 *          Pulse count this scan line
 *              (1) Laser Pulse Data:
 *                  AZ angle
 *                  EL angle
 *                  AZ offset
 *                  EL offset
 *                  Pulse time offset
 *                  Range Data:
 *                      Sounding 1 Range
 *                      Sounding 2 Range
 *                      Sounding 3 Range
 *                  Intensity Data:
 *                      Sounding 1 Intensity
 *                      Sounding 2 Intensity
 *                      Sounding 3 Intensity
 *                  Diagnostic Data (if version >= 1.2):
 *                      Sounding 1 Diagnostic
 *                      Sounding 2 Diagnostic
 *                      Sounding 3 Diagnostic
 *                  ...
 *              (250) Laser Pulse Data:
 *                  AZ angle
 *                  EL angle
 *                  AZ offset
 *                  EL offset
 *                  Pulse time offset
 *                  Range Data:
 *                      Sounding 1 Range
 *                      Sounding 2 Range
 *                      Sounding 3 Range
 *                  Intensity Data:
 *                      Sounding 1 Intensity
 *                      Sounding 2 Intensity
 *                      Sounding 3 Intensity
 *                  Diagnostic Data (if version >= 1.2):
 *                      Sounding 1 Diagnostic
 *                      Sounding 2 Diagnostic
 *                      Sounding 3 Diagnostic
 *
 * Note: based on laser head performance, differing counts of data sets may
 * exist for Head A and B. The “.raa” file extension is used for the binary file.
 *
 * ---------------------------------------------------------------------------------------
 * Comment Record:
 * --------------
 *           Record ID – Comment                          0x3D43    2 (1 UINT16)
 *           Comment length (Nc bytes)                              2 (1 UINT16)
 *           Comment (null terminated C string)                    Nc (Nc CHAR)
 *--------------------------------------------------------------------------------
 *
 *--------------------------------------------------------------------------------
 * Processing WiSSL Data Format
 * ----------------------------
 * Supported as format 233 in MB-System
 * The file header and survey data records differ from those of the raw format
 * in several respects:
 *   1) The file magic number is 0x3D09
 *   2) The file header is 923 bytes long, always.
 *   3) The scan record id's are 0x3D73, 0x3D74 rather than 0x3D53, 0x3D54
 *   4) The size in bytes of the scan record, minus 4, is stored as an unsigned
 *      short int immediately following the record id.
 *   5) Only non-null soundings as defined by preprocessing are stored - many
 *      low amplitude picks may be discarded.
 *   7) The valid pulse headers are stored in a sequential list.
 *   8) The valid soundings are stored in a separate sequential list following pulses.
 *   9) The soundings include calculated bathymetry values and the pulse and LOS
 *      pick numbers.
 *--------------------------------------------------------------------------------
 *              Item                                  Value              Bytes
 * ---------------------------------------------------------------------------------------
 * File Header Record (923 bytes):
 * ------------------
 *
 * File and version ID (8 bytes)
 *           Record ID – WiSSL                             0x3D47   2 (1 UINT16)
 *           File Magic Number                             0x3D09   2 (1 UINT16)
 *           File version                                  1        2 (1 UINT16)
 *           File sub version                              1 to 3   2 (1 UINT16)
 *
 * Scan Information (15 bytes)
 *           AZ, Cross track angle start, typical (deg)             4 (1 float32)
 *           AZ, Cross track angle end, typical (deg)               4 (1 float32)
 *           Np, Number pulses per cross track scan line            2 (1 UINT16)
 *           Nl, Number LOS (soundings) per pulse                   1 (1 UINT8)
 *           Na, Scan lines per this File, Head A                   2 (1 UINT16)
 *           Nb, Scan lines per this File, Head B                   2 (1 UINT16)
 *
 * Calibration Information (900 bytes)
 *           Calibration Structure, Head A                          450 bytes for 1.1 & 1.3, 407 bytes for 1.2
 *           Calibration Structure, Head B                          450 bytes for 1.1 & 1.3, 407 bytes for 1.2
 *
 * ---------------------------------------------------------------------------------------
 * Scan Record (0 to N-1 where N = Na + Nb scans):
 * -----------
 *
 * Scan Header (100 bytes)
 *           Record ID – Head A or B              0x3D73, 0x3D74    2 (1 UINT16)
 *           Timestamp (year (true year))                           2 (1 UINT16)
 *           Timestamp (month (1-12))                               1 (1 UINT8)
 *           Timestamp (day)                                        1 (1 UINT8)
 *           Timestamp (days since Jan 1)                           2 (1 UINT16)
 *           Timestamp (hours)                                      2 (1 UINT16)
 *           Timestamp (minutes)                                    1 (1 UINT8)
 *           Timestamp (seconds)                                    1 (1 UINT8)
 *           Timestamp (nano seconds)                               4 (1 UINT32)
 *           Gain (laser power)                                     1 (1 UINT8)
 *           Unused                                                 1 (1 UINT8)
 *           Digitizer temperature (degrees C)                      4 (1 float32)
 *           CTD temperature (degrees C)                            4 (1 float32)
 *           CTD salinity (psu)                                     4 (1 float32)
 *           CTD pressure (dbar)                                    4 (1 float32)
 *           Index of refraction                                    4 (1 float32)
 *           Start processing range (meters)                        4 (1 float32)
 *           End processing range  (meters)                         4 (1 float32)
 *           Np, Number of pulses in this scan                      4 (1 UINT32)
 *           Epoch time (seconds since 1970)                        8 (1 double64)
 *           Scan longitude (degrees)                               8 (1 double64)
 *           Scan latitude (degrees)                                8 (1 double64)
 *           Scan sensordepth (meters)                              8 (1 double64)
 *           Scan speed (m/sec)                                     4 (1 float32)
 *           Scan heading (degrees)                                 4 (1 float32)
 *           Scan roll (degrees)                                    4 (1 float32)
 *           Scan pitch (degrees)                                   4 (1 float32)
 *           Nv, Number of valid pulses                             2 (1 UINT16)
 *           Ns, Number of valid soundings                          2 (1 UINT16)
 *
 * Pulse Data ( 0 to Nv-1 pulses in this scan, 66 bytes per pulse, each valid pulse has at least 1 valid sounding )
 *           Ip, pulse id in scan, counted from 0                   2 (1 UINT16)
 *           AZ, Cross track angle (deg)                            4 (1 float32)
 *           EL, Forward track angle (deg)                          4 (1 float32)
 *           AZ, Cross track offset (m)                             4 (1 float32)
 *           EL, Forward track offset (m)                           4 (1 float32)
 *           Pulse time offset (sec)                                4 (1 float32)
 *           Epoch time (seconds since 1970)                        8 (1 double64)
 *           Navigation Acrosstrack offset (meters)                 8 (1 double64)
 *           Navigation Alongtrack offset (meters)                  8 (1 double64)
 *           Navigation Sensordepth offset (meters)                 8 (1 double64)
 *           Heading offset (degrees)                               4 (1 float32)
 *           Roll offset (degrees)                                  4 (1 float32)
 *           Pitch offset (degrees)                                 4 (1 float32)
 *
 * Sounding Data ( 0 to Ns-1 soundings in this scan, 22 bytes per sounding for 1.1, 23 bytes per sounding >= 1.2)
 *           Ip, pulse id in scan, counted from 0                   2 (1 UINT16)
 *           Is, LOS id in pulse, counted from 0                    1 (1 UINT8)
 *           Range ( from glass front ) meters                      4 (1 float32)
 *           Amplitude / peak of signal                             2 (1 UINT16)
 *           Diagnostic value (if version >= 1.2):                  1 (1 UINT8)
 *           MB-System beamflag                                     1 (1 UINT8)
 *           Acrosstrack (meters)                                   4 (1 float32)
 *           Alongtrack (meters)                                    4 (1 float32)
 *           Sensordepth (meters)                                   4 (1 float32)
 *
 * Each MB-System format 233 file begins with a File Header record that includes
 * a “Scan Information” block and two “Calibration Information” blocks, one for each
 * optical head. There may be an arbitrary number of comment records following the
 * File Header, but before the scan data. The scan line data interleaves the
 * Head A and Head B scans with no guarentee that the order is consistent with
 * increasing time stamp. The data for each scan line contains: a Record ID
 * (head designator), timestamp and navigation, a list of valid pulses (e.g.
 * pulses with at least one valid sounding), and a list of valid soundings.
 * The file suffix will typically be ".mb232".
 *
 * For example, if the sensor was configured for 250 pulses per scan line and
 * 3 LOS range measurements per pulse, and 245 pulses had valid soundings resulting
 * in a list 247 valid soundings, the following data would be present in
 * the format 233 file:
 *      File Header Record
 *        Scan Information
 *        Calibration Information Head A (450 bytes for 1.1 & 1.3, 407 bytes for 1.2)
 *        Calibration Information Head B (450 bytes for 1.1 & 1.3, 407 bytes for 1.2)
 *      Comment Record 0
 *      ......
 *      Comment Record Nc - 1
 *      Scan Record 1 (A or B)
 *        Scan header
 *        Pulse 0
 *        ....
 *        Pulse Nv - 1
 *        Sounding 0
 *        ....
 *        Sounding Ns - 1
 *
 * ---------------------------------------------------------------------------------------
 * Comment Record:
 * --------------
 *           Record ID – Comment                          0x3D43    2 (1 UINT16)
 *           Comment length (Nc bytes)                              2 (1 UINT16)
 *           Comment (null terminated C string)                    Nc (Nc CHAR)
 *--------------------------------------------------------------------------------
 */

#ifndef MBSYS_3DDWISSL_H_
#define MBSYS_3DDWISSL_H_

#include "mb_define.h"

/* defines */
#define MBF_3DWISSLR_MAGICNUMBER 0x3D08      /* '=', backspace */
#define MBF_3DWISSLP_MAGICNUMBER 0x3D09      /* '=', tab */
#define MBSYS_3DDWISSL_RECORD_FILEHEADER 0x3D47  /* '=''G' */
#define MBSYS_3DDWISSL_RECORD_RAWHEADA 0x3D53  /* '=''S' */
#define MBSYS_3DDWISSL_RECORD_RAWHEADB 0x3D54  /* '=''T' */
#define MBSYS_3DDWISSL_RECORD_COMMENT 0x3D43  /* '=''C' */
#define MBSYS_3DDWISSL_RECORD_PROHEADA 0x3D73  /* '=''s' */
#define MBSYS_3DDWISSL_RECORD_PROHEADB 0x3D74  /* '=''t' */

#define MBSYS_3DDWISSL_V1S1_PARAMETER_SIZE            23
#define MBSYS_3DDWISSL_V1S1_CALIBRATION_SIZE          450
#define MBSYS_3DDWISSL_V1S1_MAX_SOUNDINGS_PER_PULSE   5
#define MBSYS_3DDWISSL_V1S1_RAW_SCAN_HEADER_SIZE      49
#define MBSYS_3DDWISSL_V1S1_RAW_PULSE_HEADER_SIZE     20
#define MBSYS_3DDWISSL_V1S1_RAW_SOUNDING_SIZE         6
#define MBSYS_3DDWISSL_V1S1_PRO_SCAN_HEADER_SIZE      100
#define MBSYS_3DDWISSL_V1S1_PRO_PULSE_HEADER_SIZE     66
#define MBSYS_3DDWISSL_V1S1_PRO_SOUNDING_SIZE         22

#define MBSYS_3DDWISSL_V1S2_PARAMETER_SIZE            23
#define MBSYS_3DDWISSL_V1S2_CALIBRATION_SIZE          407
#define MBSYS_3DDWISSL_V1S2_MAX_SOUNDINGS_PER_PULSE   5
#define MBSYS_3DDWISSL_V1S2_RAW_SCAN_HEADER_SIZE      49
#define MBSYS_3DDWISSL_V1S2_RAW_PULSE_HEADER_SIZE     20
#define MBSYS_3DDWISSL_V1S2_RAW_SOUNDING_SIZE         7

#define MBSYS_3DDWISSL_V1S3_PARAMETER_SIZE            23
#define MBSYS_3DDWISSL_V1S3_CALIBRATION_SIZE          450
#define MBSYS_3DDWISSL_V1S3_MAX_SOUNDINGS_PER_PULSE   5
#define MBSYS_3DDWISSL_V1S3_RAW_SCAN_HEADER_SIZE      49
#define MBSYS_3DDWISSL_V1S3_RAW_PULSE_HEADER_SIZE     20
#define MBSYS_3DDWISSL_V1S3_RAW_SOUNDING_SIZE         7
#define MBSYS_3DDWISSL_V1S3_PRO_SCAN_HEADER_SIZE      100
#define MBSYS_3DDWISSL_V1S3_PRO_PULSE_HEADER_SIZE     66
#define MBSYS_3DDWISSL_V1S3_PRO_SOUNDING_SIZE         23

/* Instrument geometry for dual optical heads - the sensor reference point
 * is the midpoint on bottom of the mounting bracketry  as per the WiSSL
 * mechanical ICD. The raw data all reference ranges to the center point of the
 * front of the glass on each optical head. Here are the distance offsets of
 * those points to the sensor reference point:
 *
 *   Head A (aft mounted, pointed to starboard):
 *     dx (acrosstrack): -0.48126 inches = -0.012224004 m
 *     dy (alongtrack):  -4.73551 inches = -0.120281954 m
 *     dz (vertical:     -2.44115 inches = -0.062005210 m
 *     droll (in xz plane, + to starboard): +22.08 degrees
 *     dpitch (in yz plane, + forward): -4.68
 *
 *   Head B (forward mounted, pointed to port):
 *     dx (acrosstrack): +0.48126 inches = +0.012224004 m
 *     dy (alongtrack):  +4.73551 inches = +0.120281954 m
 *     dz (vertical:     -2.44115 inches = -0.062005210 m
 *     droll (in xz plane, + to starboard): -22.08 degrees
 *     dpitch (in yz plane, + forward): -5.01
 */
#define MBSYS_3DDWISSL_FILEHEADER                 0
#define MBSYS_3DDWISSL_HEADA                      1
#define MBSYS_3DDWISSL_HEADB                      2
#define MBSYS_3DDWISSL_COMMENT                    3
#define MBSYS_3DDWISSL_HEADA_OFFSET_X_M           +0.012224004  /* ICD value +0.012224004 */
#define MBSYS_3DDWISSL_HEADA_OFFSET_Y_M           -0.120281954  /* ICD value -0.120281954 */
#define MBSYS_3DDWISSL_HEADA_OFFSET_Z_M           +0.062005210  /* ICD value +0.062005210 */
#define MBSYS_3DDWISSL_HEADA_OFFSET_HEADING_DEG   -0.0      /* ICD value 0.0 */
#define MBSYS_3DDWISSL_HEADA_OFFSET_ROLL_DEG      -22.08    /* ICD value -22.08 */
#define MBSYS_3DDWISSL_HEADA_OFFSET_PITCH_DEG     -4.68      /* ICD value -4.68 */
#define MBSYS_3DDWISSL_HEADB_OFFSET_X_M           -0.012224004  /* ICD value -0.012224004 */
#define MBSYS_3DDWISSL_HEADB_OFFSET_Y_M           +0.120281954  /* ICD value +0.120281954 */
#define MBSYS_3DDWISSL_HEADB_OFFSET_Z_M           +0.062005210  /* ICD value +0.062005210 */
#define MBSYS_3DDWISSL_HEADB_OFFSET_HEADING_DEG   +0.00      /* ICD value 0.0 */
#define MBSYS_3DDWISSL_HEADB_OFFSET_ROLL_DEG      +22.08    /* ICD value +22.08 */
#define MBSYS_3DDWISSL_HEADB_OFFSET_PITCH_DEG     -5.01      /* ICD value -5.01 */

/*#define MBSYS_3DDWISSL_FILEHEADER                 0 */
/*#define MBSYS_3DDWISSL_HEADA                      1 */
/*#define MBSYS_3DDWISSL_HEADB                      2 */
/*#define MBSYS_3DDWISSL_COMMENT                    3 */
/*#define MBSYS_3DDWISSL_HEADA_OFFSET_X_M           +0.012224004 */
/*#define MBSYS_3DDWISSL_HEADA_OFFSET_Y_M           -0.120281954 */
/*#define MBSYS_3DDWISSL_HEADA_OFFSET_Z_M           +0.062005210 */
/*#define MBSYS_3DDWISSL_HEADA_OFFSET_HEADING_DEG   -1.20         // ICD value 0.0 */
/*#define MBSYS_3DDWISSL_HEADA_OFFSET_ROLL_DEG      -21.440       // ICD value -22.08 */
/*#define MBSYS_3DDWISSL_HEADA_OFFSET_PITCH_DEG     -5.440        // ICD value -4.68 */
/*#define MBSYS_3DDWISSL_HEADB_OFFSET_X_M           -0.012224004 */
/*#define MBSYS_3DDWISSL_HEADB_OFFSET_Y_M           +0.120281954 */
/*#define MBSYS_3DDWISSL_HEADB_OFFSET_Z_M           +0.062005210 */
/*#define MBSYS_3DDWISSL_HEADB_OFFSET_HEADING_DEG   +1.20         // ICD value 0.0 */
/*#define MBSYS_3DDWISSL_HEADB_OFFSET_ROLL_DEG      +21.440       // ICD value +22.08 */
/*#define MBSYS_3DDWISSL_HEADB_OFFSET_PITCH_DEG     -4.230        // ICD value -5.01  pitch too much   1.5 cm = 0.143 deg, roll too much 2.5 cm = 0.239 deg */

/* Values from Tank Test */
/*#define MBSYS_3DDWISSL_FILEHEADER                 0 */
/*#define MBSYS_3DDWISSL_HEADA                      1 */
/*#define MBSYS_3DDWISSL_HEADB                      2 */
/*#define MBSYS_3DDWISSL_COMMENT                    3 */
/*#define MBSYS_3DDWISSL_HEADA_OFFSET_X_M           -0.031775996 // ICD value +0.012224004    -0.044   ==> -0.031775996 */
/*#define MBSYS_3DDWISSL_HEADA_OFFSET_Y_M           -0.120281954 */
/*#define MBSYS_3DDWISSL_HEADA_OFFSET_Z_M           +0.056005210 // ICD value +0.062005210   -0.012    ==> +0.056005210 */
/*#define MBSYS_3DDWISSL_HEADA_OFFSET_HEADING_DEG   -1.20        // ICD value 0.0 */
/*#define MBSYS_3DDWISSL_HEADA_OFFSET_ROLL_DEG      -22.53       // ICD value -22.08 */
/*#define MBSYS_3DDWISSL_HEADA_OFFSET_PITCH_DEG     -5.590       // ICD value -4.68 */
/*#define MBSYS_3DDWISSL_HEADB_OFFSET_X_M           +0.031775996 // ICD value -0.012224004    +0.044   ==> +0.031775996 */
/*#define MBSYS_3DDWISSL_HEADB_OFFSET_Y_M           +0.120281954 */
/*#define MBSYS_3DDWISSL_HEADB_OFFSET_Z_M           +0.074005210 // ICD value  +0.062005210   +0.012    ==> +0.074005210 */
/*#define MBSYS_3DDWISSL_HEADB_OFFSET_HEADING_DEG   +1.20        // ICD value 0.0 */
/*#define MBSYS_3DDWISSL_HEADB_OFFSET_ROLL_DEG      +22.53       // ICD value +22.08 */
/*#define MBSYS_3DDWISSL_HEADB_OFFSET_PITCH_DEG     -4.080       // ICD value -5.01 */

/*#define MBSYS_3DDWISSL_FILEHEADER                 0 */
/*#define MBSYS_3DDWISSL_HEADA                      1 */
/*#define MBSYS_3DDWISSL_HEADB                      2 */
/*#define MBSYS_3DDWISSL_COMMENT                    3 */
/*#define MBSYS_3DDWISSL_HEADA_OFFSET_X_M           -0.032775996 // ICD value +0.012224004 */
/*#define MBSYS_3DDWISSL_HEADA_OFFSET_Y_M           -0.102281954 // ICD value -0.120281954 */
/*#define MBSYS_3DDWISSL_HEADA_OFFSET_Z_M           +0.050005210 // ICD value +0.062005210 */
/*#define MBSYS_3DDWISSL_HEADA_OFFSET_HEADING_DEG   -1.20        // ICD value 0.0 */
/*#define MBSYS_3DDWISSL_HEADA_OFFSET_ROLL_DEG      -22.53       // ICD value -22.08 */
/*#define MBSYS_3DDWISSL_HEADA_OFFSET_PITCH_DEG     -5.590       // ICD value -4.68 */
/*#define MBSYS_3DDWISSL_HEADB_OFFSET_X_M           +0.032775996 // ICD value -0.012224004 */
/*#define MBSYS_3DDWISSL_HEADB_OFFSET_Y_M           +0.102281954 // ICD value +0.120281954 */
/*#define MBSYS_3DDWISSL_HEADB_OFFSET_Z_M           +0.074005210 // ICD value  +0.062005210 */
/*#define MBSYS_3DDWISSL_HEADB_OFFSET_HEADING_DEG   +1.20        // ICD value 0.0 */
/*#define MBSYS_3DDWISSL_HEADB_OFFSET_ROLL_DEG      +22.53       // ICD value +22.08 */
/*#define MBSYS_3DDWISSL_HEADB_OFFSET_PITCH_DEG     -4.080       // ICD value -5.01 */

#define MBSYS_3DDWISSL_DEFAULT_AMPLITUDE_THRESHOLD 2000.0
#define MBSYS_3DDWISSL_DEFAULT_TARGET_ALTITUDE        0.0
#define MBSYS_3DDWISSL_LASERPULSERATE               40000.0

/* V1.1 Calibration structure used for data collected in 2018 - 450 bytes */
struct mbsys_3ddwissl_calibration_v1s1_struct
  {
  char cfg_path[ 64 ];
  int laser_head_no;            /* either 1 or 2 */
  int process_for_air;          /* 1 = air, else water */
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

/* Version 1.2 Calibration structure used for data collected in May 2019 */
/* - 407 bytes for logged as 1.2, 450 bytes written for 1.3 with 43 unused bytes */
/*   added so the 450 byte size matches that of version 1.1 */
struct mbsys_3ddwissl_calibration_v1s2_struct
  {
  char cfg_path[ 64 ];
  int laser_head_no;          /* either 1 or 2 */
  int process_for_air;        /* 1 = air, else water */
  mb_u_char temperature_compensation;
  mb_u_char emergency_shutdown;
  float ocb_temperature_limit_c;
  float ocb_humidity_limit;
  float pb_temperature_limit_1_c;
  float pb_temperature_limit_2_c;
  float pb_humidity_limit;
  float dig_temperature_limit_c;
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
  double temp_comp_poly2;
  double temp_comp_poly1;
  double temp_comp_poly;
  float laser_start_time_sec;
  float scanner_shift_cts;
  float factory_scanner_lrg_deg;
  float factory_scanner_med_deg;
  float factory_scanner_sml_deg;
  float factory_dig_cnt_to_volts;
  float el_angle_fixed_deg;
  int zda_to_pps_max_msec;
  int zda_udp_port;
  mb_u_char show_time_sync_errors;
  int min_time_diff_update_msec;
  int ctd_tcp_port;
  double trigger_level_volt;
  int mf_t0_position;
  int mf_start_proc;
  int dig_ref_pos_t0_cnts;
  int dummy;
  int t0_min_height_raw_cts;
  double scanner_neg_polynom_0;
  double scanner_neg_polynom_1;
  double scanner_neg_polynom_2;
  double scanner_neg_polynom_3;
  double scanner_neg_polynom_4;
  double scanner_neg_polynom_5;
  double scanner_pos_polynom_0;
  double scanner_pos_polynom_1;
  double scanner_pos_polynom_2;
  double scanner_pos_polynom_3;
  double scanner_pos_polynom_4;
  double scanner_pos_polynom_5;
  };

/* Version 1.3 Calibration structure used for data collected starting in September 2019 */
/* - 450 byte size matches that of version 1.1 */
struct mbsys_3ddwissl_calibration_v1s3_struct
  {
  char cfg_path[ 64 ];
  int laser_head_no;          /* either 1 or 2 */
  int process_for_air;        /* 1 = air, else water */
  mb_u_char temperature_compensation;
  mb_u_char emergency_shutdown;
  float ocb_temperature_limit_c;
  float ocb_humidity_limit;
  float pb_temperature_limit_1_c;
  float pb_temperature_limit_2_c;
  float pb_humidity_limit;
  float dig_temperature_limit_c;
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
  double temp_comp_poly2;
  double temp_comp_poly1;
  double temp_comp_poly;
  float laser_start_time_sec;
  float scanner_shift_cts;
  float factory_scanner_lrg_deg;
  float factory_scanner_med_deg;
  float factory_scanner_sml_deg;
  float factory_dig_cnt_to_volts;
  float el_angle_fixed_deg;
  int zda_to_pps_max_msec;
  int zda_udp_port;
  mb_u_char show_time_sync_errors;
  int min_time_diff_update_msec;
  int ctd_tcp_port;
  double trigger_level_volt;
  int mf_t0_position;
  int mf_start_proc;
  int dig_ref_pos_t0_cnts;
  int dummy;
  int t0_min_height_raw_cts;
  double scanner_neg_polynom_0;
  double scanner_neg_polynom_1;
  double scanner_neg_polynom_2;
  double scanner_neg_polynom_3;
  double scanner_neg_polynom_4;
  double scanner_neg_polynom_5;
  double scanner_pos_polynom_0;
  double scanner_pos_polynom_1;
  double scanner_pos_polynom_2;
  double scanner_pos_polynom_3;
  double scanner_pos_polynom_4;
  double scanner_pos_polynom_5;

	// New  request as of 8/20/2019
  unsigned short trigger_coupling_type; // 0 = AC, 1 = DC, 2 = GND, 3 = HF_Reject,
                                        // 4 = LF_Reject, 1001 = AC + HF_Reject
  float digitizer_voltage_range_v; // 2.0 nominal
  int prf_tune_wait_ms;
  char unused[33]; // make size match old v1 at 450 bytes
  };

struct mbsys_3ddwissl_sounding_struct
  {
  /* raw information */
  float range;      /* meters from glass front */
  short amplitude;    /* peak of signal - to 1023 */
  mb_u_char diagnostic;  /* diagnostic value - unknown meaning, >= V1.2 only */

  /* processed information incorporating pulse offsets and head offsets */
  mb_u_char beamflag;  /* MB-System beam flag */
  float acrosstrack;  /* acrosstrack distance relative to overall sensor reference point (meters)
              */
  float alongtrack;    /* alongtrack distance relative to overall sensor reference point
                 (meters) */
  float depth;      /* depth relative to overall sensor reference point (meters) */
  };

struct mbsys_3ddwissl_pulse_struct
  {
  /* raw information */
  float angle_az;  /* AZ, Cross track angle, (deg) */
  float angle_el;  /* AZ, Forward track angle, (deg) */
  float offset_az;/* AZ, Cross track offset, (m) */
  float offset_el;/* AZ, Forward track offset, (m) */
  float time_offset;  /* Pulse time offset (sec) */

  /* processed information */
  double time_d;        /* epoch time */
  double acrosstrack_offset;  /* relative to start of scan using position
                * and heading at start of scan */
  double alongtrack_offset;  /* relative to start of scan using position
                * and heading at start of scan */
  double sensordepth_offset;  /* relative to start of scan using position
                * and heading at start of scan */
  float heading_offset;    /* relative to start of scan using position
                * and heading at start of scan */
  float roll_offset;      /* relative to start of scan using position
                * and heading at start of scan */
  float pitch_offset;      /* relative to start of scan using position
                * and heading at start of scan */

  /* soundings */
  int validsounding_count;  /* number of the soundings valid (non-null) for this pulse */
  struct mbsys_3ddwissl_sounding_struct soundings[MBSYS_3DDWISSL_V1S1_MAX_SOUNDINGS_PER_PULSE];
  };

/* 3DatDepth LIDAR data structure */
struct mbsys_3ddwissl_struct
  {
  /* Type of data record */
  int kind;  /* MB-System record ID */

  /* File Header */
  unsigned short parameter_id;  /* 0x3D47 */
  unsigned short magic_number;  /* 0x3D08 */
  unsigned short file_version;  /* 1 */
  unsigned short sub_version;    /* 1 = initial version from 3DatDepth, extended for MB-System */

  /* Scan Information */
  float cross_track_angle_start;  /* AZ, Cross track angle start, typical (deg) */
  float cross_track_angle_end;/* AZ, Cross track angle end, typical (deg) */
  unsigned short pulses_per_scan;  /* Pulses per cross track, scan line */
  unsigned short soundings_per_pulse;  /* soundings per pulse (line of sight, or LOS) */
  unsigned short heada_scans_per_file;/* number of heada scans in this file */
  unsigned short headb_scans_per_file;/* number of headb scans in this file */

  /* WiSSL optical head positional and angular offsets */
  double heada_offset_x_m;          /* head A x offset (m) -0.012224004 */
  double heada_offset_y_m;          /* head A y offset (m) -0.120281954 */
  double heada_offset_z_m;          /* head A z offset (m) -0.062005210 */
  double heada_offset_heading_deg;      /* head A heading offset (degrees) +22.08 */
  double heada_offset_roll_deg;        /* head A roll offset (degrees) +22.08 */
  double heada_offset_pitch_deg;        /* head A pitch offset (degrees) -4.68 */
  double headb_offset_x_m;          /* head B x offset (m) +0.012224004 */
  double headb_offset_y_m;          /* head B y offset (m) +0.120281954 */
  double headb_offset_z_m;          /* head B z offset (m) -0.062005210 */
  double headb_offset_heading_deg;      /* head B heading offset (degrees) -22.08 */
  double headb_offset_roll_deg;        /* head B roll offset (degrees) -22.08 */
  double headb_offset_pitch_deg;        /* head B pitch offset (degrees) -5.01 */

  /* head A calibration */
  struct mbsys_3ddwissl_calibration_v1s1_struct calibration_v1s1_a;
  struct mbsys_3ddwissl_calibration_v1s3_struct calibration_v1s3_a;

  /* head B calibration */
  struct mbsys_3ddwissl_calibration_v1s1_struct calibration_v1s1_b;
  struct mbsys_3ddwissl_calibration_v1s3_struct calibration_v1s3_b;

  /* Scan information from raw records */
  unsigned short record_id;          /* head A (0x3D53 or 0x3D73) or head B (0x3D54 or 0x3D74) */
  unsigned int scan_size;            /* bytes of scan record minus 4 (record_id + scan_size) */
  unsigned short year;
  mb_u_char month;
  mb_u_char day;
  unsigned short jday;
  unsigned short hour;
  mb_u_char minutes;
  mb_u_char seconds;
  unsigned int nanoseconds;
  mb_u_char gain;                /* laser power setting */
  mb_u_char unused;              /* unused */
  float digitizer_temperature;        /* digitizer temperature degrees C */
  float ctd_temperature;            /* ctd temperature degrees C */
  float ctd_salinity;              /* ctd salinity psu */
  float ctd_pressure;              /* ctd pressure dbar */
  float index;                    /* index of refraction */
  float range_start;              /* range start processing meters */
  float range_end;              /* range end processing meters */
  unsigned int pulse_count;          /* pulse count for this scan */

  /* merged navigation and attitude per each scan */
  double time_d;                /* epoch time - not in data file, calculated
                           following reading */
  double navlon;                /* absolute position longitude (degrees) */
  double navlat;                /* absolute position latitude (degrees) */
  double sensordepth;  /* absolute position depth below sea surface (meters), includes any tide
               correction */
  float speed;                /* lidar speed (m/s) */
  float heading;                /* lidar heading (degrees) */
  float roll;                  /* lidar roll (degrees) */
  float pitch;                /* lidar pitch (degrees) */
  unsigned short validpulse_count;      /* number of valid (non-null) pulses stored in this
                           record */
  unsigned short validsounding_count;      /* number of valid (non-null) soundings stored in
                           this record */

  unsigned int scan_count;          /* global scan count */
  unsigned int size_pulse_record_raw;      /* for original logged records
                        * - calculated from file header values */
  unsigned int size_pulse_record_processed;  /* for extended processed records
                        * -  calculated from file header values */
  unsigned int bathymetry_calculated;      /* flag regarding calculation of bathymetry */
  int num_pulses_alloc;            /* array allocated for this number of pulses */
  struct mbsys_3ddwissl_pulse_struct *pulses;

  /* comment */
  unsigned short comment_len;          /* comment length in bytes */
  char comment[MB_COMMENT_MAXLINE];  /* comment string */
  };

/* System specific function prototypes */
int mbsys_3ddwissl_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_3ddwissl_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_3ddwissl_dimensions(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *kind,
  int *nbath,
  int *namp,
  int *nss,
  int *error);
int mbsys_3ddwissl_pingnumber(int verbose, void *mbio_ptr, unsigned int *pingnumber, int *error);
int mbsys_3ddwissl_preprocess(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  void *platform_ptr,
  void *preprocess_pars_ptr,
  int *error);
int mbsys_3ddwissl_sensorhead(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *sensorhead,
  int *error);
int mbsys_3ddwissl_extract(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *kind,
  int time_i[7],
  double *time_d,
  double *navlon,
  double *navlat,
  double *speed,
  double *heading,
  int *nbath,
  int *namp,
  int *nss,
  char *beamflag,
  double *bath,
  double *amp,
  double *bathacrosstrack,
  double *bathalongtrack,
  double *ss,
  double *ssacrosstrack,
  double *ssalongtrack,
  char *comment,
  int *error);
int mbsys_3ddwissl_insert(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int kind,
  int time_i[7],
  double time_d,
  double navlon,
  double navlat,
  double speed,
  double heading,
  int nbath,
  int namp,
  int nss,
  char *beamflag,
  double *bath,
  double *amp,
  double *bathacrosstrack,
  double *bathalongtrack,
  double *ss,
  double *ssacrosstrack,
  double *ssalongtrack,
  char *comment,
  int *error);
int mbsys_3ddwissl_ttimes(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *kind,
  int *nbeams,
  double *ttimes,
  double *angles,
  double *angles_forward,
  double *angles_null,
  double *heave,
  double *alongtrack_offset,
  double *draft,
  double *ssv,
  int *error);
int mbsys_3ddwissl_detects(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *kind,
  int *nbeams,
  int *detects,
  int *error);
int mbsys_3ddwissl_pulses(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *kind,
  int *nbeams,
  int *pulses,
  int *error);
int mbsys_3ddwissl_gains(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *kind,
  double *transmit_gain,
  double *pulse_length,
  double *receive_gain,
  int *error);
int mbsys_3ddwissl_extract_altitude(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *kind,
  double *transducer_depth,
  double *altitude,
  int *error);
int mbsys_3ddwissl_extract_nnav(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int nmax,
  int *kind,
  int *n,
  int *time_i,
  double *time_d,
  double *navlon,
  double *navlat,
  double *speed,
  double *heading,
  double *draft,
  double *roll,
  double *pitch,
  double *heave,
  int *error);
int mbsys_3ddwissl_extract_nav(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *kind,
  int time_i[7],
  double *time_d,
  double *navlon,
  double *navlat,
  double *speed,
  double *heading,
  double *draft,
  double *roll,
  double *pitch,
  double *heave,
  int *error);
int mbsys_3ddwissl_insert_nav(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int time_i[7],
  double time_d,
  double navlon,
  double navlat,
  double speed,
  double heading,
  double draft,
  double roll,
  double pitch,
  double heave,
  int *error);
int mbsys_3ddwissl_extract_svp(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *kind,
  int *nsvp,
  double *depth,
  double *velocity,
  int *error);
int mbsys_3ddwissl_insert_svp(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int nsvp,
  double *depth,
  double *velocity,
  int *error);
int mbsys_3ddwissl_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error);
int mbsys_3ddwissl_print_store(int verbose, void *store_ptr, int *error);
int mbsys_3ddwissl_calculatebathymetry(int verbose,
  void *mbio_ptr,
  void *store_ptr,
  double amplitude_threshold,
  double target_altitude,
  int *error);

/* functions called by mbpreprocess to fix first generation WiSSL timestamp errors */
int mbsys_3ddwissl_indextablefix(int verbose,
  void *mbio_ptr,
  int num_indextable,
  void *indextable_ptr,
  int *error);
int mbsys_3ddwissl_indextableapply(int verbose,
  void *mbio_ptr,
  int num_indextable,
  void *indextable_ptr,
  int n_file,
  int *error);

#endif  /* MBSYS_3DDWISSL_H_ */
