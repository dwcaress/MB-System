//--------------------------------------------------------------------
//    The MB-system:  mbsys_reson7k3.h  1/8/2019
//
//    Copyright (c) 2004-2019 by
//    David W. Caress (caress@mbari.org)
//      Monterey Bay Aquarium Research Institute
//      Moss Landing, CA 95039
//    and Dale N. Chayes (dale@ldeo.columbia.edu)
//      Lamont-Doherty Earth Observatory
//      Palisades, NY 10964
//
//    See README file for copying and redistribution conditions.
//----------------------------------------------------------------------
//
// mbsys_reson7k3.h defines the MBIO data structures for handling data from
// Teledyne Reson 7k series, Teledyne Odom MB2, Teledyne BlueView ProScan
// software, Hydrosweep 3rd generation (HS3) sonars and other applications
// and sonars using 7k data record formats:
//      MBF_RESON7K3 : MBIO ID 89 - Teledyne Reson 3rd generation 7K data format
//
// Authors:  C. S. Ferreira & D. W. Caress
// Date:  March 2019
//
// Notes on the mbsys_reson7k3 data structure:
//   1. This format is defined by the 7k Data Format Definition (DFD)
//      document for Teledyne RESON SeaBat 7k Data Format Version 3.10.
//   2. Reson 7k series multibeam sonars output bathymetry, per beam
//      amplitude, sidescan data and water column.
//   3. Reson 7k format is used also to log sidescan and subbottom
//      data from other sonars.
//   4. The 7k record consists of a data record frame (header and checksum),
//      a record  type header, an optional record data field and an optional
//      data field for extra  information. The optional data field typically
//      holds sensor specific data and third party developer
//      embedded data.
//   5. The format specification includes  a number of data record types
//      that have been part of the data format in the past but are now
//      deprecated. Data including deprecated records should be read using
//      the MBF_RESON7KR (88) i/o module. The data supported by this
//      MBF_RESON7K3 (89) i/o module is expected to use the following
//      data records for each ping:
//        Sonar Settings
//          7000 – Sonar Settings
//        Receiver Beam Geometry
//          7004 – Beam Geometry
//        Bathymetry
//          7027 – Raw Detections (+ optional calculated bathymetry)
//          7047 – Segmented Raw Detections (+ optional calculated bathymetry)
//        Sidescan time series
//          7007 – Side-scan data
//        Backscatter samples (snippets)
//          7028 – Snippet
//        Water column beamformed time series
//          7018 – Beamformed data
//          7041 – Compressed beamformed intensity data
//          7042 – Compressed Water Column data
//   6. MB-System stores beamflags used for bathymetry editing in the
//      RawDetection quality values for each beam. The 7k 3.10 DFD
//      defines use of only the first two bits of these 32 bit
//      values. MB-System embeds the 8-bit beamflag values in bits 24-31.
//   7. The maximum dimensions defined below
//          #define MBSYS_RESON7K_MAX_BEAMS 512
//          #define MBSYS_RESON7K_MAX_SOUNDINGS 2560
//          #define MBSYS_RESON7K_MAX_SEGMENTS 3
//          #define MBSYS_RESON7K_MAX_PIXELS 4096
//      reflect the various Teledyne mapping sonars.
//      The Reson SeaBat T50 multibeams have a maximum of 512 formed beams
//      and can produce up to five multi-pick soundings per beam, resulting
//      in the maximum soundings of 2560. These are reported in 7027 Raw Detections
//      records. The Hydrosweep MD/30 multibeam produces 320 formed beams with
//      three soundings per beam produced by split beam processing for 960
//      soundings. These are reported in the 7047 Segmented Raw Detections
//      records.
//   ?. Questions/decisions to be resolved before this i/o module is
//      finalized:
//        - The beamflag can be embedded in either the quality value or the
//          flags value - we've chosen the quality value - is this best?
//        - It's clear that the RawDetection optional data exists when the
//          optional data offset is nonzero in the RawDetection record header
//          is nonzero. How does PDS set the the optional data identifier value?
//          All the DFD states is that this identifier is user defined.
//        - We want to augment the RawDetection optional data space with the
//          usual MB-System laid-out pseudosidescan, but need to use a different
//          optional data identifier than PDS so that the code knows this.
//        - How complete are the attitude time series that can be constructed
//          from the ping motion records? How do these relate to the data stored
//          in the asynchronous RollPitchHeave, Attitude, CustomAttitude, and
//          Heading records?

#ifndef MBSYS_RESON7K3_H_
#define MBSYS_RESON7K3_H_

#include <stdint.h>
#include "mb_define.h"

//-----------------------------------------------------------------
// Record ID definitions

// 0 means no record at all
#define R7KRECID_None 0

// 1000-1999 reserved for generic sensor records
#define R7KRECID_ReferencePoint 1000
#define R7KRECID_UncalibratedSensorOffset 1001
#define R7KRECID_CalibratedSensorOffset 1002
#define R7KRECID_Position 1003                    // MB_DATA_NAV1
#define R7KRECID_CustomAttitude 1004              // MB_DATA_ATTITUDE2
#define R7KRECID_Tide 1005
#define R7KRECID_Altitude 1006                    // MB_DATA_ALTITUDE
#define R7KRECID_MotionOverGround 1007
#define R7KRECID_Depth 1008                       // MB_DATA_SONARDEPTH
#define R7KRECID_SoundVelocityProfile 1009
#define R7KRECID_CTD 1010
#define R7KRECID_Geodesy 1011
#define R7KRECID_RollPitchHeave 1012              // MB_DATA_ATTITUDE1
#define R7KRECID_Heading 1013                     // MB_DATA_HEADING
#define R7KRECID_SurveyLine 1014                  // MB_DATA_SURVEY_LINE
#define R7KRECID_Navigation 1015                  // MB_DATA_NAV
#define R7KRECID_Attitude 1016                    // MB_DATA_ATTITUDE
#define R7KRECID_PanTilt 1017
#define R7KRECID_SonarInstallationIDs 1020
#define R7KRECID_Mystery 1022

// 2000-2999 reserved for user defined records
#define R7KRECID_SonarPipeEnvironment 2004

// 3000-6999 reserved for extra records
#define R7KRECID_ContactOutput 3001
#define R7KRECID_ProcessedSideScan 3199

// 7000-7999 reserved for SeaBat 7k records
#define R7KRECID_SonarSettings 7000
#define R7KRECID_Configuration 7001
#define R7KRECID_MatchFilter 7002
#define R7KRECID_FirmwareHardwareConfiguration 7003
#define R7KRECID_BeamGeometry 7004
#define R7KRECID_Bathymetry 7006
#define R7KRECID_SideScan 7007
#define R7KRECID_WaterColumn 7008
#define R7KRECID_VerticalDepth 7009
#define R7KRECID_TVG 7010
#define R7KRECID_Image 7011
#define R7KRECID_PingMotion 7012
#define R7KRECID_AdaptiveGate 7014
#define R7KRECID_DetectionDataSetup 7017
#define R7KRECID_Beamformed 7018
#define R7KRECID_VernierProcessingDataRaw 7019
#define R7KRECID_BITE 7021
#define R7KRECID_SonarSourceVersion 7022
#define R7KRECID_WetEndVersion8k 7023
#define R7KRECID_RawDetection 7027
#define R7KRECID_Snippet 7028
#define R7KRECID_VernierProcessingDataFiltered 7029
#define R7KRECID_InstallationParameters 7030
#define R7KRECID_BITESummary 7031
#define R7KRECID_CompressedBeamformedMagnitude 7041
#define R7KRECID_CompressedWaterColumn 7042
#define R7KRECID_SegmentedRawDetection 7047
#define R7KRECID_CalibratedBeam 7048
#define R7KRECID_SystemEvents 7050
#define R7KRECID_SystemEventMessage 7051
#define R7KRECID_RDRRecordingStatus 7052
#define R7KRECID_Subscriptions 7053
#define R7KRECID_RDRStorageRecording 7054
#define R7KRECID_CalibrationStatus 7055
#define R7KRECID_CalibratedSideScan 7057
#define R7KRECID_SnippetBackscatteringStrength 7058
#define R7KRECID_MB2Status 7059
#define R7KRECID_FileHeader 7200
#define R7KRECID_FileCatalog 7300
#define R7KRECID_TimeMessage 7400
#define R7KRECID_RemoteControl 7500
#define R7KRECID_RemoteControlAcknowledge 7501
#define R7KRECID_RemoteControlNotAcknowledge 7502
#define R7KRECID_RemoteControlSonarSettings 7503
#define R7KRECID_CommonSystemSettings 7504
#define R7KRECID_SVFiltering 7510
#define R7KRECID_SystemLockStatus 7511
#define R7KRECID_SoundVelocity 7610
#define R7KRECID_AbsorptionLoss 7611
#define R7KRECID_SpreadingLoss 7612

//-----------------------------------------------------------------
// Record size definitions
#define MBSYS_RESON7K_VERSIONSYNCSIZE 64
#define MBSYS_RESON7K_RECORDHEADER_SIZE 64
#define MBSYS_RESON7K_RECORDTAIL_SIZE 4

// 0 means no record at all
#define R7KHDRSIZE_None 0

// 1000-1999 reserved for generic sensor records
#define R7KHDRSIZE_ReferencePoint 16
#define R7KHDRSIZE_UncalibratedSensorOffset 24
#define R7KHDRSIZE_CalibratedSensorOffset 24
#define R7KHDRSIZE_Position 37
#define R7KHDRSIZE_CustomAttitude 8
#define R7KRDTSIZE_CustomAttitude 4
#define R7KHDRSIZE_Tide 43
#define R7KHDRSIZE_Altitude 4
#define R7KHDRSIZE_MotionOverGround 8
#define R7KHDRSIZE_Depth 8
#define R7KHDRSIZE_SoundVelocityProfile 24
#define R7KRDTSIZE_SoundVelocityProfile 8
#define R7KHDRSIZE_CTD 36
#define R7KRDTSIZE_CTD 20
#define R7KHDRSIZE_Geodesy 320
#define R7KHDRSIZE_RollPitchHeave 12
#define R7KHDRSIZE_Heading 4
#define R7KHDRSIZE_SurveyLine 72
#define R7KRDTSIZE_SurveyLine 16
#define R7KHDRSIZE_Navigation 41
#define R7KHDRSIZE_Attitude 1
#define R7KRDTSIZE_Attitude 18
#define R7KHDRSIZE_PanTilt 8
#define R7KHDRSIZE_SonarInstallationIDs 164
#define R7KHDRSIZE_Mystery 40

// 2000-2999 reserved for user defined records
#define R7KHDRSIZE_SonarPipeEnvironment 83
#define R7KRDTSIZE_SonarPipeEnvironment 20

// 3000-6999 reserved for other vendor records
#define R7KHDRSIZE_ContactOutput 450
#define R7KHDRSIZE_ProcessedSideScan 48

// 7000-7999 reserved for SeaBat 7k records
#define R7KHDRSIZE_SonarSettings 156
#define R7KHDRSIZE_Configuration 12
#define R7KHDRSIZE_MatchFilter 88
#define R7KHDRSIZE_FirmwareHardwareConfiguration 8
#define R7KHDRSIZE_BeamGeometry 12
#define R7KHDRSIZE_Bathymetric 24
#define R7KHDRSIZE_SideScan 64
#define R7KHDRSIZE_WaterColumn 30
#define R7KHDRSIZE_VerticalDepth 42
#define R7KHDRSIZE_TVG 50
#define R7KHDRSIZE_Image 56
#define R7KHDRSIZE_PingMotion 44
#define R7KHDRSIZE_AdaptiveGate 22
#define R7KHDRSIZE_DetectionDataSetup 116
#define R7KRDTSIZE_DetectionDataSetup 34
#define R7KHDRSIZE_Beamformed 52
#define R7KHDRSIZE_VernierProcessingDataRaw 92
#define R7KHDRSIZE_BITE 2
#define R7KRDTSIZE_BITERecordData 136
#define R7KRDTSIZE_BITEFieldData 79
#define R7KHDRSIZE_SonarSourceVersion 32
#define R7KHDRSIZE_WetEndVersion8k 32
#define R7KHDRSIZE_RawDetection 99
#define R7KRDTSIZE_RawDetection 34
#define R7KOPTHDRSIZE_RawDetection 45
#define R7KOPTDATSIZE_RawDetection 20
#define R7KHDRSIZE_Snippet 46
#define R7KRDTSIZE_snippetdata 14
#define R7KHDRSIZE_VernierProcessingDataFiltered 26
#define R7KRDTSIZE_VernierProcessingDataFiltered 16
#define R7KHDRSIZE_InstallationParameters 616
#define R7KHDRSIZE_BITESummary 36
#define R7KHDRSIZE_CompressedBeamformedMagnitude 38
#define R7KHDRSIZE_CompressedWaterColumn 44
#define R7KHDRSIZE_SegmentedRawDetection 36
#define R7KRDTSIZE_SegmentedRawDetection 100
#define R7KOPTHDRSIZE_SegmentedRawDetection 45
#define R7KOPTDATSIZE_SegmentedRawDetection 20
#define R7KHDRSIZE_CalibratedBeam 56
#define R7KHDRSIZE_SystemEvents 12
#define R7KHDRSIZE_SystemEventMessage 14
#define R7KHDRSIZE_RDRRecordingStatus 566
#define R7KHDRSIZE_Subscriptions 4
#define R7KRDTSIZE_Subscriptions 780
#define R7KHDRSIZE_RDRStorageRecording 303
#define R7KHDRSIZE_CalibrationStatus 826
#define R7KHDRSIZE_CalibratedSideScan 65
#define R7KHDRSIZE_SnippetBackscatteringStrength 49
#define R7KHDRSIZE_MB2Status 2088
#define R7KHDRSIZE_FileHeader 316
#define R7KRDTSIZE_FileHeader 6
#define R7KHDRSIZE_FileCatalog 14
#define R7KRDTSIZE_FileCatalog 48
#define R7KHDRSIZE_TimeMessage 16
#define R7KHDRSIZE_RemoteControl 24
#define R7KHDRSIZE_RemoteControlAcknowledge 20
#define R7KHDRSIZE_RemoteControlNotAcknowledge 24
#define R7KHDRSIZE_RemoteControlSonarSettings 260
#define R7KHDRSIZE_CommonSystemSettings 543
#define R7KHDRSIZE_SVFiltering 9
#define R7KHDRSIZE_SystemLockStatus 38
#define R7KHDRSIZE_SoundVelocity 4
#define R7KHDRSIZE_AbsorptionLoss 4
#define R7KHDRSIZE_SpreadingLoss 4

//-----------------------------------------------------------------

// Device identifiers
#define R7KDEVID_SeaBatT20 20
#define R7KDEVID_SeaBatT20Dual 22
#define R7KDEVID_SeaBatF30 30
#define R7KDEVID_SeaBatT50 50
#define R7KDEVID_SeaBatT50Dual 52
#define R7KDEVID_GenericPosition 100
#define R7KDEVID_GenericHeading 101
#define R7KDEVID_GenericAttitude 102
#define R7KDEVID_GenericMBES 103
#define R7KDEVID_GenericSideScan 104
#define R7KDEVID_GenericSBP 105
#define R7KDEVID_OdomMB1 1000
#define R7KDEVID_TrueTime 1001
#define R7KDEVID_OdomMB2 1002
#define R7KDEVID_CDCSMCG 2000
#define R7KDEVID_CDCSPG 2001
#define R7KDEVID_EmpireMagnetics 2002
#define R7KDEVID_ResonTC4013 4013
#define R7KDEVID_ResonDiverDat 6000
#define R7KDEVID_Reson7kSonarSource 7000
#define R7KDEVID_Reson7kUserInterface 7001
#define R7KDEVID_ResonPDS 7003
#define R7KDEVID_Reson7kLogger 7004
#define R7KDEVID_BlueViewProScan 7005
#define R7KDEVID_SeaBat7012 7012
#define R7KDEVID_SeaBat7100 7100
#define R7KDEVID_SeaBat7101 7101
#define R7KDEVID_SeaBat7102 7102
#define R7KDEVID_SeaBat7111 7111
#define R7KDEVID_SeaBat7112 7112
#define R7KDEVID_SeaBat7123 7123
#define R7KDEVID_SeaBat7125 7125
#define R7KDEVID_SeaBat7128 7128
#define R7KDEVID_SeaBat7130 7130
#define R7KDEVID_SeaBat7150 7150
#define R7KDEVID_SeaBat7160 7160
#define R7KDEVID_SeaBat8100 8100
#define R7KDEVID_SeaBat8101 8101
#define R7KDEVID_SeaBat8102 8102
#define R7KDEVID_SeaBat8112 8111
#define R7KDEVID_SeaBat8123 8123
#define R7KDEVID_SeaBat8124 8124
#define R7KDEVID_SeaBat8125 8125
#define R7KDEVID_SeaBat8128 8128
#define R7KDEVID_SeaBat8150 8150
#define R7KDEVID_SeaBat8160 8160
#define R7KDEVID_TSSDMS05 10000
#define R7KDEVID_TSS335B 10001
#define R7KDEVID_TSS332B 10002
#define R7KDEVID_SeaBirdSBE37 10010
#define R7KDEVID_Littom200 10200
#define R7KDEVID_EdgetechFSDWSBP 11000
#define R7KDEVID_EdgetechFSDWSSLF 11001
#define R7KDEVID_EdgetechFSDWSSHF 11002
#define R7KDEVID_BlueFin 11100
#define R7KDEVID_IfremerTechsas 11200
#define R7KDEVID_SimradRPT319 12000
#define R7KDEVID_NorbitWBMSFLS400 13002
#define R7KDEVID_NorbitWBMSBathy400 13003
#define R7KDEVID_NorbitiWMBMS 13004
#define R7KDEVID_NorbitBathy400Compact 13005
#define R7KDEVID_NorbitWBMSBathy200 13007
#define R7KDEVID_NorbitBathy400 13008
#define R7KDEVID_NorbitFLSDeepSea400 13009
#define R7KDEVID_NorbitBathyDeepSea400 13010
#define R7KDEVID_NorbitBathyDeepSea200 13011
#define R7KDEVID_NorbitiLidar 13012
#define R7KDEVID_NorbitBathySTX400 13016
#define R7KDEVID_NorbitBathySTX200 13017
#define R7KDEVID_NorbitiWBMSe 13018
#define R7KDEVID_Hydrosweep3DS 14000
#define R7KDEVID_Hydrosweep3MD50 14001
#define R7KDEVID_Hydrosweep3MD30 14002

//-----------------------------------------------------------------

// Structure size definitions
#define MBSYS_RESON7K_BUFFER_STARTSIZE 32768
#define MBSYS_RESON7K_MAX_DEVICE 73
#define MBSYS_RESON7K_MAX_BEAMS 512
#define MBSYS_RESON7K_MAX_SOUNDINGS 2560
#define MBSYS_RESON7K_MAX_SEGMENTS 320
#define MBSYS_RESON7K_MAX_PIXELS 2048
#define MBSYS_RESON7K_MAX_BUFFERED_COMMENTS 100

//-----------------------------------------------------------------

// Data type definitions
typedef char c8;
typedef signed char i8;
typedef unsigned char u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef float f32;
typedef int64_t i64;
typedef uint64_t u64;
typedef double f64;

//-----------------------------------------------------------------

typedef struct s7k3_time_struct {
  u16 Year;    // Year                 0 - 65535
  u16 Day;     // Day                  1 - 366
  f32 Seconds; // Seconds              0.000000 - 59.000000
  u8 Hours;    // Hours                0 - 23
  u8 Minutes;  // Minutes              0 - 59
} s7k3_time;

typedef struct s7k3_header_struct {
  u16 Version;                // Version - Version of this frame (e.g.: 1, 2 etc.)
  u16 Offset;                 // Offset - Offset in bytes from the start of the sync
                              // pattern to the start of the Record Type Header (RTH).
                              // This allows for expansion of the header
                              // whilst maintaining backward compatibility.
  u32 SyncPattern;            // Sync pattern - 0x0000FFFF
  u32 Size;                   // Size - Size in bytes of this record from the start
                              // of the Protocol version field to the end of the
                              // checksum field - including any embedded data.
  u32 OptionalDataOffset;     // Data offset - Offset in bytes to optional data field from
                              // start of record. Zero implies no optional data.
  u32 OptionalDataIdentifier; // User defined.
  s7k3_time s7kTime;           // 7KTIME u8*10 UTC - Time tag indicating when data was
                              // produced.
  u16 RecordVersion;          // Currently 1
  u32 RecordType;             // Record type - Identifier for record type of embedded data.
  u32 DeviceId;               // Device identifier - Identifier of the device to which
                              // this datum pertains.
  u16 Reserved;               // Reserved
  u16 SystemEnumerator;       // System enumerator - The enumerator is used to differentiate
                              // between devices with the same device identifiers in one
                              // installation/system.
  u32 Reserved2;              // Reserved
  u16 Flags;                  // Flags - BIT FIELD:
                                      // Bit 0 - Checksum
                                      //     0 - invalid checksum
                                      //     1 - valid checksum
                                      // Bit 1-14 - Reserved (must be zero)
                                      // Bit 15:
                                      //     0 - Live data
                                      //     1 - Recorded data
  u16 Reserved3;              // Reserved
  u32 Reserved4;              // Reserved
  u32 FragmentedTotal;        // Always zero
  u32 FragmentNumber;         // Always zero
  // Following this header is:
  //  DATA SECTION               Dynamic Record type specific data.
  //  Checksum                   u32 Sum of bytes in data section
                              // (optional, depends on bit 1 of Flags field).
                              // Note: the checksum field  should be computed
                              // as a 64 bit unsigned integer  with the least
                              // significant 32 bits used to populate  this field
                              // thus ensuring a valid checksum and  avoiding
                              // an explicit overflow.
} s7k3_header;

// Reference point information (record 1000)
// Note: these offsets should be zero for submersible vehicles
typedef struct s7k3_ReferencePoint_struct {
  s7k3_header header;
  f32 offset_x; // Vehicle's X reference point ot center of gravity (meters)
  f32 offset_y; // Vehicle's Y reference point ot center of gravity (meters)
  f32 offset_z; // Vehicle's Z reference point ot center of gravity (meters)
  f32 water_z;  // Vehicle's water level to center of gravity (meters)
} s7k3_ReferencePoint;

// Sensor uncalibrated offset position information (record 1001)
typedef struct s7k3_UncalibratedSensorOffset_struct {
  s7k3_header header;
  f32 offset_x;     // Sensor X offset from vehicle reference point (meters)
  f32 offset_y;     // Sensor Y offset from vehicle reference point (meters)
  f32 offset_z;     // Sensor Z offset from vehicle reference point (meters)
  f32 offset_roll;  // Sensor roll offset (radians - port up is positive)
  f32 offset_pitch; // Sensor pitch offset (radians - bow up is positive)
  f32 offset_yaw;   // Sensor yaw offset (radians - bow right/starboard is positive)
} s7k3_UncalibratedSensorOffset;

// Sensor calibrated offset position information (record 1002)
typedef struct s7k3_CalibratedSensorOffset_struct {
  s7k3_header header;
  f32 offset_x;     // Sensor X offset from vehicle reference point (meters)
  f32 offset_y;     // Sensor Y offset from vehicle reference point (meters)
  f32 offset_z;     // Sensor Z offset from vehicle reference point (meters)
  f32 offset_roll;  // Sensor roll offset (radians - port up is positive)
  f32 offset_pitch; // Sensor pitch offset (radians - bow up is positive)
  f32 offset_yaw;   // Sensor yaw offset (radians - bow right/starboard is positive)
} s7k3_CalibratedSensorOffset;

// Position (record 1003)
typedef struct s7k3_Position_struct {
  s7k3_header header;
  u32 datum;        // 0=WGS84; others reserved
  f32 latency;      // Position sensor time latency (seconds)
  f64 latitude_northing;     // Latitude (radians) or northing in meters
  f64 longitude_easting;    // Longitude (radians) or easting in meters
  f64 height;       // Height relative to datum (meters)
  u8 type;          // Position type flag:
                    // 0: Geographical coordinates
                    // 1: Grid coordinates
  u8 utm_zone;      // UTM zone
  u8 quality;       // Quality flag
                    // 0: Navigation data
                    // 1: Dead reckoning
  u8 method;        // Positioning method
                    // 0: GPS
                    // 1: DGPS
                    // 2: Start of inertial positioning system from GPS
                    // 3: Start of inertial positioning system from DGPS
                    // 4: Start of inertial positioning system from bottom correlation
                    // 5: Start of inertial positioning system from bottom object
                    // 6: Start of inertial positioning system from inertial positioning
                    // 7: Start of inertial positioning system from optional data
                    // 8: Stop of inertial positioning system from GPS
                    // 9: Stop of inertial positioning system from DGPS
                    // 10: Stop of inertial positioning system from bottom correlation
                    // 11: Stop of inertial positioning system from bottom object
                    // 12: Stop of inertial positioning system from inertial positioning
                    // 13: Stop of inertial positioning system from optional data
                    // 14: User defined
                    // 15: RTK Fixed
                    // 16: RTK Float
  u8 nsat;          // Optional
} s7k3_Position;

// Custom Attitude (record 1004)
typedef struct s7k3_CustomAttitude_struct {
  s7k3_header header;
  u8 fieldmask;  // Boolean bitmask indicating which attitude fields are in data
                //  0: pitch (radians - float)
                //  1: roll (radians - float)
                //  2: heading (radians - float)
                //  3: heave (meters - float)
                //  4: pitch rate (radians per second - float)
                //  5: roll rate (radians per second - float)
                //  6: heading rate (radians per second - float)
                //  7: heave rate (radians per second - float)
  u8 reserved;   // reserved field
  u16 n;         // number of fields
  f32 frequency; // sample rate (samples/second)
  i32 nalloc;    // number of samples allocated
  f32 *pitch;
  f32 *roll;
  f32 *heading;
  f32 *heave;
  f32 *pitchrate;
  f32 *rollrate;
  f32 *headingrate;
  f32 *heaverate;
} s7k3_CustomAttitude;

// Tide (record 1005)
typedef struct s7k3_Tide_struct {
  s7k3_header header;
  f32 tide;               // Height correction above mean sea level (meters)
  u16 source;             // Tide data source: 0 - unspecified; 1 - table; 2 - gauge
  u8 flags;               // Gauge and position validity flags
                          //       Bit 0: 0/1 for gauge id valid/invalid
                          //       Bit 1: 0/1 for position valid/invalid
  u16 gauge;              // User defined
  u32 datum;              // 0 = WGS84; others reserved
  f32 latency;            // Position sensor time latency (seconds)
  f64 latitude_northing;  // Latitude (radians) or northing in meters
  f64 longitude_easting;  // Longitude (radians) or easting in meters
  f64 height;             // Height relative to datum (meters)
  u8 type;                // Position type flag:
                          //       0: Geographical coordinates
                          //       1: Grid coordinates
  u8 utm_zone;            // UTM zone
} s7k3_Tide;

// Altitude (record 1006)
typedef struct s7k3_Altitude_struct {
  s7k3_header header;
  f32 altitude;        // altitude above seafloor (meters)
} s7k3_Altitude;

// Motion over ground (record 1007)
typedef struct s7k3_MotionOverGround_struct {
  s7k3_header header;
  u8 flags;     // Field mask indicating which motion over ground fields are in data
                //  0: X,Y,Z speed (m/s - 3 X float)
                //  1: X,Y,Z acceleration (m/s**2 - 3 X float)
                //  2-7: reserved
  u8 reserved;   // reserved field
  u16 n;         // number of fields
  f32 frequency; // sample rate (samples/second)
  i32 nalloc;    // number of samples allocated
  f32 *x;          // Motion data x
  f32 *y;          // Motion data y
  f32 *z;          // Motion data z
  f32 *xa;          // Motion data x
  f32 *ya;          // Motion data y
  f32 *za;          // Motion data z
} s7k3_MotionOverGround;

// Depth (record 1008)
typedef struct s7k3_Depth_struct {
  s7k3_header header;
  u8 descriptor;  // Depth descriptor:
                //   0 = depth to sensor
                //   1 = water depth
  u8 correction;  // Correction flag:
                //   0 = raw depth as measured
                //   1 = corrected depth (relative to mean sea level)
  u16 reserved;   // Reserved field
  f32 depth;      // The deeper, the bigger (positive) this value becomes (meters)
} s7k3_Depth;

// Sound velocity profile (record 1009)
typedef struct s7k3_SoundVelocityProfile_struct {
  s7k3_header header;
  u8 position_flag;     // Position validity flag:
                        //  0: invalid position fields
                        //  1: valid position field
  u8 reserved1;         // reserved field
  u16 reserved2;        // reserved field
  f64 latitude;         // Latitude (radians)
  f64 longitude;        // Longitude (radians)
  u32 n;                // number of fields
  i32 nalloc;           // number of samples allocated
  f32 *depth;           // depth (meters)
  f32 *sound_velocity;  // sound velocity (meters/second)
} s7k3_SoundVelocityProfile;

// CTD (record 1010)
typedef struct s7k3_CTD_struct {
  s7k3_header header;
  f32 frequency;               // Sample rate
  u8 velocity_source_flag;     // Velocity source flag:
                               //    0: not computed
                               //  1: CTD
                               //  2: user computed
  u8 velocity_algorithm;       // Velocity algorithm flag:
                               //  0: not computed
                               //  1: Chen Millero
                               //  2: Delgrosso
  u8 conductivity_flag;        // Conductivity flag:
                               //  0: conductivity
                               //  1: salinity
  u8 pressure_flag;            // Pressure flag:
                               //  0: pressure
                               //  1: depth
  u8 position_flag;            // Position validity flag:
                               //  0: invalid position fields
                               //  1: valid position field
  u8 validity;                 // Sample content validity:
                               //  Bit 0: conductivity/salinity
                               //  Bit 1: water temperature
                               //  Bit 2: pressure/depth
                               //  Bit 3: sound velocity
                               //  Bit 4: absorption
  u16 reserved;                // Reserved field
  f64 latitude;                // Latitude (radians)
  f64 longitude;               // Longitude (radians)
  f32 sample_rate;             // Sample rate
  u32 n;                       // Number of fields
  i32 nalloc;                  // Number of samples allocated
  f32 *conductivity_salinity;  // Conductivity (s/m) or salinity (ppt)
  f32 *temperature;            // Temperature (degrees celcius)
  f32 *pressure_depth;         // Pressure (pascals) or depth (meters)
  f32 *sound_velocity;         // Sound velocity (meters/second)
  f32 *absorption;             // Sound velocity absorption (dB/second)
} s7k3_CTD;

// Geodesy (record 1011)
typedef struct s7k3_Geodesy_struct {
  s7k3_header header;
  c8 spheroid[32];         // Text description of the spheroid name (e.g. "WGS84")
  f64 semimajoraxis;       // Semi-major axis in meters (e.g. 6378137.0 for WGS84)
  f64 flattening;          // Inverse flattening in meters (e.g. 298.257223563 for WGS84)
  u8 reserved1[16];        // Reserved space
  c8 datum[32];            // Datum name (e.g. "WGS84")
  u32 calculation_method;  // Data calculation method:
                           //  0 - Molodensky
                           //  1 - Bursa / Wolfe
                           //  2 - DMA MRE
                           //  3 - NADCON
                           //  4 - HPGN
                           //  5 - Canadian National Transformation V2
  u8 number_parameters;    // Seven parameter transformation supported
  f64 dx;                  // X shift (meters)
  f64 dy;                  // Y shift (meters)
  f64 dz;                  // Z shift (meters)
  f64 rx;                  // X rotation (radians)
  f64 ry;                  // Y rotation (radians)
  f64 rz;                  // Z rotation (radians)
  f64 scale;               // Scale
  u8 reserved2[35];        // Reserved for implementation of 9 parameter transformation
  c8 grid_name[32];        // Name of grid system in use: (e.g. "UTM")
  u8 distance_units;       // Grid distance units:
                           //    0 - meters
                           //  1 - feet
                           //  2 - yards
                           //  3 - US survey feet
                           //  4 - km
                           //  5 - miles
                           //  6 - US survey miles
                           //  7 - nautical miles
                           //  8 - chains
                                   //  9 - links
  u8 angular_units;        // Grid angulat units:
                           //  0 - radians
                           //  1 - degrees
                           //  2 - degrees, minutes, seconds
                           //  3 - gradians
                           //  4 - arc-seconds
  f64 latitude_origin;     // Latitude of origin
  f64 central_meridian;    // Central meridian
  f64 false_easting;       // False easting (meters)
  f64 false_northing;      // False northing (meters)
  f64 central_scale_factor;  // Central scale factor
  i32 custom_identifier;   // Identifier for optional field definition in 7k record.
                           //  Used to define projection specific parameters.
                           //  -2 - custom
                           //  -1 - not used
  u8 reserved3[50];        // Reserved field
} s7k3_Geodesy;

// Roll pitch heave (record 1012)
typedef struct s7k3_RollPitchHeave_struct {
  s7k3_header header;
  f32 roll;  // Vessel roll (radians)
  f32 pitch; // Vessel pitch (radians)
  f32 heave; // Vessel heave (m)
} s7k3_RollPitchHeave;

// Heading (record 1013)
typedef struct s7k3_Heading_struct {
  s7k3_header header;
  f32 heading; // Vessel heading (radians)
} s7k3_Heading;

// Survey Line (record 1014)
typedef struct s7k3_SurveyLine_struct {
  s7k3_header header;
  u16 n;           // Number of waypoints
  u16 type;        // Position type flag:
                //    0: Geographical coordinates
                //    1: Grid coordinates
  f32 turnradius;  // Turn radius between line segments
                //    (meters, 0 = no curvature in turns)
  c8 name[64];     // Line name
  i32 nalloc;      // Number of points allocated
  f64 *latitude_northing;   // Latitude (radians, -pi/2 to pi/2)
  f64 *longitude_easting;  // Longitude (radians -pi to pi)
} s7k3_SurveyLine;

// Navigation (record 1015)
typedef struct s7k3_Navigation_struct {
  s7k3_header header;
  u8 vertical_reference;      // Vertical reference:
                              //        1 = Ellipsoid
                              //        2 = Geoid
                              //        3 = Chart datum
  f64 latitude;               // Latitude (radians, -pi/2 to pi/2)
  f64 longitude;              // Longitude (radians -pi to pi)
  f32 position_accuracy;      // Horizontal position accuracy (meters)
  f32 height;                 // Height of vessel reference point above
                              //   vertical reference (meters)
  f32 height_accuracy;        // Height accuracy (meters)
  f32 speed;                  // Speed over ground (meters/sec)
  f32 course;                 // Course over ground (radians)
  f32 heading;                // Heading (radians)
} s7k3_Navigation;

// Attitude (record 1016)
typedef struct s7k3_Attitude_struct {
  s7k3_header header;
  u8 n;            // number of datasets
  i32 nalloc;                 // number of samples allocated
  u16 *delta_time; // Time difference with record timestamp (msec)
  f32 *roll;                // Roll (radians)
  f32 *pitch;               // Pitch (radians)
  f32 *heave;               // Heave (m)
  f32 *heading;             // Heading (radians)
} s7k3_Attitude;

// Pan Tilt (record 1017)
typedef struct s7k3_PanTilt_struct {
  s7k3_header header;
  f32 pan;  // Angle (radians)
  f32 tilt; // Angle (radians)
} s7k3_PanTilt;

// Sonar Installation Identifiers (record 1020)
typedef struct s7k3_SonarInstallationIDs_struct {
  s7k3_header header;
  u32 system_id;      // Sonar ID
  u32 tx_id;          // Tx Unid ID
  u32 rx_id;          // Rx Unid ID
  u32 std_id;         // 0=Custom, otherwise all parameters
                      // bellow are ignored
  u32 conf_pars;      // Defines configuration defined parameters
                      // Bit field, 1 = fixed
                      // Bit 0-2: Tx to Rx XYZ Linear Offsets
                      // Bit 3-5: Tx to Reference XYZ Linear Offsets
                      // Bit 6-8: Tx to Rx Angular Offsets
                      // Bit 9-15: Reserved
  f32 tx_length;            // Y measured value of Tx hardware (meters)
  f32 tx_width;             // X measured value of Tx hardware (meters)
  f32 tx_height;            // Z measured value of Tx hardware (meters)
  f32 tx_radius;            // Flat arrays set to 0
  f32 offset_srp2tx_x;      // X linear offset from SRP to center of Tx (meters)
  f32 offset_srp2tx_y;      // Y linear offset from SRP to center of Tx (meters)
  f32 offset_srp2tx_z;      // Z linear offset from SRP to center of Tx (meters)
  f32 offset_tx_roll;       // Angular offsets from array main axis to
  f32 offset_tx_pitch;      //   motion axes in Lagrange coordinates (radians)
  f32 offset_tx_yaw;        //
  f32 rx_length;            // Y measured value of Rx hardware (meters)
  f32 rx_width;             // X measured value of Rx hardware (meters)
  f32 rx_height;            // Z measured value of Rx hardware (meters)
  f32 rx_radius;            // Flat arrays set to 0
  f32 offset_srp2rx_x;      // X linear offset from SRP to center of Rx (meters)
  f32 offset_srp2rx_y;      // Y linear offset from SRP to center of Rx (meters)
  f32 offset_srp2rx_z;      // Z linear offset from SRP to center of Rx (meters)
  f32 offset_rx_roll;       // Angular offsets from array main axis to
  f32 offset_rx_pitch;      //   motion axes in Lagrange coordinates (radians)
  f32 offset_rx_yaw;        //
  f32 frequency;            // System frequency
  f32 offset_vrp2srp_x;     // X linear offset from VRP to SRP (meters)
  f32 offset_vrp2srp_y;     // Y linear offset from VRP to SRP (meters)
  f32 offset_vrp2srp_z;     // Z linear offset from VRP to SRP (meters)
  u32 cable_length;  // Cable length (meters) for DMPA systems, 0 when no set
  u8 reserved[44];          // Reserved field
} s7k3_SonarInstallationIDs;

// Mystery record (record 1022)
typedef struct s7k3_Mystery_struct {
  s7k3_header header;
  mb_u_char data[R7KHDRSIZE_Mystery]; // raw bytes in unknown record
} s7k3_Mystery;

// Sonar Pipe Environment (record 2004)
typedef struct s7k3_SonarPipeEnvironment_struct {
  s7k3_header header;
  u32 pipe_number;              // Pipe identifier
  s7k3_time s7kTime;             // 7KTIME               u8*10   UTC.*/
  u32 ping_number;              // Sequential number
  u32 multiping_number;         // Sub number
  f32 pipe_diameter;            // Diameter of pipe (meters)
  f32 sound_velocity;           // Sound velocity (m/s)
  f32 sample_rate;              // Sonar's sampling frequency (Hertz)
  u8 finished;                  // 0 = Pipe is still growing, otherwise is finished
  u8 points_number;             // Number of point sub records, always 5 (five)
  u8 n;                         // Size of sub record
  u8 reserved[10];              // Reserved field
  i32 nalloc;                   // number of samples allocated
  f32 *x;                       // X coordinate in sonar space (meters)
  f32 *y;                       // Y coordinate in sonar space (meters)
  f32 *z;                       // Z coordinate in sonar space (meters)
  f32 *angle;                   // Point angle (radians)
  f32 *sample_number;           // Sample number
} s7k3_SonarPipeEnvironment;

// Contact Output (record 3001)
typedef struct s7k3_ContactOutput_struct {
  s7k3_header header;
  u32 target_id;             // Contact unique ID
  u32 ping_number;           // Sequential number
  s7k3_time s7kTime;          // 7KTIME               u8*10   UTC.*/
  u8 operator_name[128];     // Optional textual name of the operator
  u32 contact_state;         // 0 = created; 1 = modified; 2 = deleted
  f32 range;                 // Range from sonar to contact (meters)
  f32 bearing;               // Bearing from sonar to contact (meters)
  u32 info_flags;            // Bit field
                               //  Bit 0: Set to 1 if latitude and longitude fields
                               //    contain valid values
                               //  Bit 1: Set to 1 if azimuth field contains a valid
                               //    value
                               //  Bit 2: Set to 1 if contact length field contains
                               //    a valid value
                               //  Bit 3: Set to 1 if latitude and longitude fields
                               //    contain valid values
  f64 latitude;              // Latitude of contact in radians (-pi/2 to pi/2),
                               //  south negative
  f64 longitude;             // Latitude of contact in radians (-pi/2 to pi/2),
                               //  west negative
  f32 azimuth;               // Optional azimuth of contact (radians)
  f32 contact_length;        // Optional length of contact (radians)
  f32 contact_width;         // Optional azimuth of contact (radians)
  u8 classification[128];    // Optional textual classification given by the operator
  u8 description[128];       // Optional textual description given by the operator
} s7k3_ContactOutput;

// Processed sidescan - MB-System extension to s7k3 format (record 3199)
typedef struct s7k3_ProcessedSideScan_struct {
  s7k3_header header;
  mb_u_long serial_number;      // Sonar serial number
  u32 ping_number;              // Sequential number
  u16 multi_ping;               // Flag to indicate multi-ping mode
                                //      0 = no multi-ping
                                //      >0 = sequence number of ping
                                        //  in the multi-ping
                                        //  sequence
  u16 recordversion;            // allows for progression of versions of this data record
                                // version = 1: initial version as of 8 October 2012
  u32 ss_source;                // Source of raw backscatter for this sidescan that has
                                // been laid out on the seafloor:
                                //      ss_source = 0:     None
                                //      ss_source = 1:     Non-Reson sidescan
                                //      ss_source = 7007:  7kBackscatterImageData
                                //      ss_source = 7008:  7kBeamData
                                //      ss_source = 7028:  7kV2SnippetData
                                //      ss_source = 7058:  7kCalibratedSnippetData
  u32 number_pixels;            // Number of sidescan pixels across the entire swath
  u32 ss_type;                  // indicates if sidescan values are logarithmic or linear
                                //      ss_type = 0: logarithmic (dB)
                                //      ss_type = 1: linear (voltage)
  f32 pixelwidth;               // Pixel acrosstrack width in m
                                // Acrosstrack distance of each pixel given by
                                // acrosstrack = (ipixel - number_pixels / 2) * pixelwidth
                                // where i = pixel number and N is the total number
                                // of pixels, counting from port to starboard starting at 0
  f64 sonardepth;                          // Sonar depth in m
  f64 altitude;                            // Sonar nadir altitude in m
  f32 sidescan[MBSYS_RESON7K_MAX_PIXELS];   // Depth releative to chart datum in meters
  f32 alongtrack[MBSYS_RESON7K_MAX_PIXELS]; // Alongtrack distance in meters
} s7k3_ProcessedSideScan;

// Reson 7k Sonar Settings (record 7000)
typedef struct s7k3_SonarSettings_struct {
  s7k3_header header;
  u64 serial_number;              // Sonar serial number
  u32 ping_number;                // Ping number
  u16 multi_ping;                 // Flag to indicate multi-ping mode.
                                // 0 = no multi-ping
                                // >0 = sequence number of the ping in the multi-ping sequence.
  f32 frequency;                  // Transmit frequency (Hertz)
  f32 sample_rate;                // Sample rate (Hertz)
  f32 receiver_bandwidth;         // Receiver bandwidth (Hertz)
  f32 tx_pulse_width;             // Transmit pulse length (seconds)
  u32 tx_pulse_type;              // Pulse type identifier:
                                // 0 - CW
                                // 1 - linear chirp
  u32 tx_pulse_envelope;          // Pulse envelope identifier:
                                // 0 - tapered rectangular
                                // 1 - Tukey
                                // 2 - Hamming
                                // 3 - Han
                                // 4 - Rectangular
  f32 tx_pulse_envelope_par;      // Some envelopes don't use this parameter
  u32 tx_pulse_mode;              // 1 - Single ping
                                // 2 - Multi-ping 2
                                // 3 - Multi-ping 3
                                // 4 - Multi-ping 4
  f32 max_ping_rate;              // Maximum ping rate (pings/second)
  f32 ping_period;                // Time since last ping (seconds)
  f32 range_selection;            // Range selection (meters)
  f32 power_selection;            // Power selection (dB/uPa)
  f32 gain_selection;             // Gain selection (dB)
  u32 control_flags;              // Control flags bit field:
                                //    Bit 0-3: Auto range method
                                //    Bit 4-7: Auto bottom detect filter method
                                //    Bit   8: Bottom detection range filter enabled
                                //    Bit   9: Bottom detect depth filter enabled
                                //    Bit  10: Receiver gain method Auto Gain
                                //    Bit  11: Receiver gain method Fixed Gain
                                //    Bit  12: Receiver gain method Reserved
                                //    Bit  13: Reserved
                                //    Bit  14: Trigger out High for entire RX duration
                                        //     0 - disabled
                                        //     1 - enabled
                                //    Bit  15:
                                        //     0 - system inactive
                                        //     1 - active
                                //    Bit16-19: Reserved for bottom detection
                                //    Bit   20: Pipe gating filter
                                        //      0 - Disabled
                                        //      1 - Enabled
                                //    Bit   21: Adaptive gate depth filter fixed
                                        //      0 - Follow seafloor
                                        //      1 - Fix depth
                                //    Bit   22: Adaptive gate
                                        //      0 - Disabled
                                        //      1 - Enabled
                                //    Bit   23: Adaptive gate depth filter
                                        //      0 - Disabled
                                        //      1 - Enabled
                                //    Bit   24: Trigger out
                                        //      0 - Disabled
                                        //      1 - Enabled
                                //    Bit   25: Trigger in edge
                                        //      0 - Positive
                                        //      1 - Negative
                                //    Bit   26: PPS edge
                                        //      0 - Positive
                                        //      1 - Negative
                                //    Bit27-28: Timestamp State
                                        //      0 - Timestamp not applicable
                                        //      1 - Timestamp error/not valid
                                        //      2 - Timestamp warning/use caution
                                        //      3 - Timestamp ok/valid
                                //    Bit   29: Depth filter follows seafloor
                                        //      0 - Fix depth
                                        //      1 - Follow seafloor
                                //    Bit   30: Reduced coverage for constant spacing
                                        //      0 - Always maintain swath coverage
                                        //      1 - Allow swath coverage to be reduced
                                //    Bit   31:
                                        //      0 - 7K
                                        //      1 - Simulator
  u32 projector_id;               // Projector selection
  f32 steering_vertical;          // Projector steering angle vertical (radians)
  f32 steering_horizontal;        // Projector steering angle horizontal (radians)
  f32 beamwidth_vertical;         // Projector -3 dB beamwidth vertical (radians - along track)
  f32 beamwidth_horizontal;       // Projector -3 dB beamwidth horizontal (radians - along track)
  f32 focal_point;                // Projector focal point (meters)
  u32 projector_weighting;        // Projector beam weighting window type:
                                //    0 - rectangular
                                //    1 - Chebyshev
                                //    2 - Gauss
  f32 projector_weighting_par;    // Projector beam weighting window parameter
  u32 transmit_flags;             // Transmit flags bitfield:
                                //    Bit 0-3: Pitch stabilization method
                                //    Bit 4-7: Yaw stabilization method
                                //    Bit8-31: Reserved
  u32 hydrophone_id;         // Hydrophone selection (magic number)
  u32 rx_weighting;          // Receiver beam weighting window type:
                                //    0 - Chebyshev
                                //    1 - Kaiser
  f32 rx_weighting_par;      // Receiver beam weighting window parameter
  u32 rx_flags;              // Receive flags bit field:
                                //    Bit    0: Roll compensation indicator
                                //    Bit    1: Reserved
                                //    Bit    2: Heave compensation indicator
                                //    Bit    3: Reserved
                                //    Bit  4-7: Dynamic focusing method
                                //    Bit 8-11: Doppler compensation method
                                //    Bit12-15: Match filtering method
                                //    Bit16-19: TVG method
                                //    Bit20-23: Multi-ping mode
                                        //      0 - No multi-ping
                                        //      If non-zero, this represents the sequence
                                        //      number of the ping in the multi-ping
                                        //      sequence.
                                //    Bit24-31: Reserved
  f32 rx_width;              // Receive beam width (radians)
  f32 range_minimum;              // Bottom detection minimum range (meters)
  f32 range_maximum;              // Bottom detection maximum range (meters)
  f32 depth_minimum;              // Bottom detection minimum depth (meters)
  f32 depth_maximum;              // Bottom detection maximum depth (meters)
  f32 absorption;                 // Absorption (dB/km)
  f32 sound_velocity;             // Sound velocity (meters/second)
  f32 spreading;                  // Spreading loss (dB)
  u16 reserved;                   // reserved for future pulse shape description
} s7k3_SonarSettings;

// Reson 7k device configuration structure (part of record 7001)*/
typedef struct s7k3_device_struct {
  u32 magic_number;       // Unique identifier number
  u8 description[62];     // Device description string (dimension 60 in the record)
  u32 alphadata_card;     // Data card definition:
                          //   0x0400 - Virtex 2 card
                          //   0x0800 - Virtex 5 card
                          //   0x1000 - Virtex 6 card
  u64 serial_number;      // Device serial number
  u32 info_length;        // Length of device specific data (bytes)
  u32 info_alloc;         // Memory allocated for data (bytes)
  c8 *info;               // Device specific data
} s7k3_device;

// Reson 7k 7kConfiguration (record 7001)
typedef struct s7k3_Configuration_struct {
  s7k3_header header;
  u64 serial_number;      // Sonar serial number
  u64 number_devices;     // Number of devices
  s7k3_device device[MBSYS_RESON7K_MAX_DEVICE]; // Device configuration information
} s7k3_Configuration;

// Reson 7k match filter (record 7002)
typedef struct s7k3_MatchFilter_struct {
  s7k3_header header;
  u64 serial_number;     // Sonar serial number
  u32 ping_number;       // Sequential number
  u32 operation;         // Operation
                        //     0 = off
                        //     1 = on
  f32 start_frequency;   // Start frequency (Hz)
  f32 end_frequency;     // End frequency (Hz)
  u32 window_type;        // Window type
                        //     0 - Rectangular
                        //     1 - Kaiser
                        //     2 - Hamming
                        //     3 - Blackmann
                        //     4 - Triangular
                        //     5 - X (Taylor)
  f32 shading;           // Shading value
  f32 pulse_width;       // Effective pulse width after FM compression
  u32 reserved[13];       // Filled with 0xFB
} s7k3_MatchFilter;

// Reson 7k firmware and hardware configuration (record 7003)
typedef struct s7k3_FirmwareHardwareConfiguration_struct {
  s7k3_header header;
  u32 device_count; // Hardware device count
  u32 info_length;  // Info length (bytes)
  u32 info_alloc;   // Memory allocated for data (bytes)
  c8 *info;                // Device specific data
} s7k3_FirmwareHardwareConfiguration;

// Reson 7k beam geometry (record 7004)
typedef struct s7k3_BeamGeometry_struct {
  s7k3_header header;
  u64 serial_number;                                  // Sonar serial number
  u32 number_beams;                                   // Number of receiver beams
  f32 angle_alongtrack[MBSYS_RESON7K_MAX_BEAMS];      // Receiver beam X direction angle (radians)
  f32 angle_acrosstrack[MBSYS_RESON7K_MAX_BEAMS];     // Receiver beam Y direction angle (radians)
  f32 beamwidth_alongtrack[MBSYS_RESON7K_MAX_BEAMS];  // Receiver beamwidth X (radians)
  f32 beamwidth_acrosstrack[MBSYS_RESON7K_MAX_BEAMS]; // Receiver beamwidth Y (radians)
  f32 tx_delay;                                       // Tx delay for the beam in fractional samples,
                                                      // zero when not applicable.
                                                      // Up to now Tx Delay is only supported for the
                                                      // Hydrosweep sonars.
                                                      // When the sonar does not has Tx Delay the item
                                                      // will not be in the Record Data, check record
                                                      // length in the Data Record Frame.
} s7k3_BeamGeometry;

// Reson 7k bathymetry (record 7006)
typedef struct s7k3_Bathymetry_struct {
  s7k3_header header;
  u64 serial_number;   // Sonar serial number
  u32 ping_number;     // Sequential number
  u16 multi_ping;      // Flag to indicate multi-ping mode
                        //   0 = no multi-ping
                        //  >0 = sequence number of ping in the multi-ping sequence
  u32 number_beams;    // Number of receiver beams
  u8 layer_comp_flag;  // Bit field:
                        //   Bit   0: Layer compensation
                        //   0 = off
                        //   1 = on
                        //   Bit   1: XYZ compensation
                        //   0 = off
                        //   1 = on
                        //   Bit 2-7: Reserved (always 0)
  u8 sound_vel_flag;   // Flag indicating if sound velocity is measured or manually entered
                        //   0 = measured
                        //   1 = manually entered
  f32 sound_velocity;  // Sound veocity at the sonar (m/sec)
  f32 range[MBSYS_RESON7K_MAX_SOUNDINGS];  // Two way travel time (seconds)
  u8 quality[MBSYS_RESON7K_MAX_SOUNDINGS]; // Beam quality bitfield:
                                          // Bit 0: Brightness test
                                          // 1 - Pass
                                          // 0 - Fail
                                          // Bit 1: Colinearity test
                                          // 1 - Pass
                                          // 0 - Fail
                                          // Bit 2: Bottom detection process (magnitude)
                                          // 1 - Used
                                          // 0 - Not used
                                          // Bit 3: Bottom detection process (phase)
                                          // 1 - Used
                                          // 0 - Not used
                                          // Bit 4: Used internally
                                          // Bit 5: PDS nadir filter
                                          // 1 - Fail
                                          // 0 - Pass
                                          // Bit 6-7: Reserved
  f32 intensity[MBSYS_RESON7K_MAX_SOUNDINGS];       // Intensity: Bottom reflectivity. This is a relative
                                          //       value (not calibrated)
  f32 min_depth_gate[MBSYS_RESON7K_MAX_SOUNDINGS];  // Minimum two-way travel time to filter point
                                          //       for each beam (minimum depth gate)
  f32 max_depth_gate[MBSYS_RESON7K_MAX_SOUNDINGS];  // Maximum two-way travel time to filter point
                                          //       for each beam (maximum depth gate)
  // TODO(schwehr): Can optionaldata be a bool?
  u32 optionaldata;                             // Flag indicating if bathymetry calculated and
                                          //       values below filled in
                                                  //  0 = No
                                                  //  1 = Yes
                                          //       This is an internal MB-System flag, not
                                          //       a value in the data format
  f32 frequency;                            // Ping frequency in Hz
  f64 latitude;                             // Latitude of vessel reference point
                                          //     in radians, -pi/2 to +pi/2, south negative
  f64 longitude;                            // Longitude of vessel reference point
                                          //     in radians, -pi to +pi, west negative
  f32 heading;                              // Heading of vessel at transmit time
                                          //     in radians
  u8 height_source;                         // Method used to correct to chart datum.
                                          //     0 = None
                                          //     1 = RTK (implies tide = 0.0)
                                          //     2 = Tide
  f32 tide;                                 // Tide in meters
  f32 roll;                                 // Roll at transmit time
  f32 pitch;                                // Pitch at transmit time
  f32 heave;                                // Heave at transmit time in m*/
  f32 vehicle_depth;                       // Vehicle depth at transmit time in m
  f32 depth[MBSYS_RESON7K_MAX_SOUNDINGS];       // Depth releative to chart datum in meters
  f32 alongtrack[MBSYS_RESON7K_MAX_SOUNDINGS];  // Alongtrack distance in meters
  f32 acrosstrack[MBSYS_RESON7K_MAX_SOUNDINGS];    // Acrosstrack distance in meters
  f32 pointing_angle[MBSYS_RESON7K_MAX_SOUNDINGS]; // Pointing angle from vertical in radians
  f32 azimuth_angle[MBSYS_RESON7K_MAX_SOUNDINGS];  // Azimuth angle in radians
} s7k3_Bathymetry;

// Reson 7k Side Scan Data (record 7007)
typedef struct s7k3_SideScan_struct {
  s7k3_header header;
  u64 serial_number;     // Sonar serial number
  u32 ping_number;       // Sequential number
  u16 multi_ping;        // Flag to indicate multi-ping mode
                        //     0 = no multi-ping
                        //    >0 = sequence number of ping in the multi-ping sequence
  f32 beam_position;     // Beam position forward from position of beam 0 (meters)
  u32 control_flags;     // Control flags bitfield
                        //     Bit 0: Nadir depth record field used
                        //     Bit 1-31: Reserved
  u32 number_samples;    // number of samples
  u32 nadir_depth;       // Nadir depth in samples
  u32 reserved[7];       // Reserved
  u16 number_beams;      // Number of sidescan beams per side (usually only one)
  u16 current_beam;      // Beam number of this record (0 to number_beams - 1)
  u8 sample_size;        // Number of bytes per sample, 1, 2 or 4
  u8 data_type;          // Data type bitfield:
                        //     Bit 0: Reserved (always 0)
                        //     Bit 1-7: Phase
  u32 nalloc;            // Memory allocated in each array (bytes)
  c8 *port_data;         // Magnitude/Phase series Port side. First sample represents
                        //  range 0 meters (total bytes per side
  c8 *stbd_data;         // Magnitude/Phase series Starboard side. First sample represents
                        //  range 0 meters (total bytes per side
  u32 optionaldata;      // Flag indicating if values below filled in
                        //     0 = No
                        //     1 = Yes
                        //     This is an internal MB-System flag, not a value in
                        //     the data format
  f32 frequency;         // Ping frequency in Hz
  f64 latitude;          // Latitude of vessel reference point
                        //     in radians, -pi/2 to +pi/2
  f64 longitude;         // Longitude of vessel reference point
                        //     in radians, -pi to +pi
  f32 heading;           // Heading of vessel at transmit time
                        //     in radians
  f32 altitude;          // Altitude in meters for slant range correction
  f32 depth;             // Nadir depth for slant range correction in meters
} s7k3_SideScan;

// Reson 7k Generic Water Column data (part of record 7008)*/
typedef struct s7k3_wcd_struct {
  u16 beam_number; // Beam or element number
  u32 begin_sample;  // First sample number in beam from transmitter and outward.
  u32 end_sample;    // Last sample number in beam from transmitter and outward.
  u32 nalloc_amp;    // Bytes allocated to hold amplitude time series
  u32 nalloc_phase;  // Bytes allocated to hold phase time series
  void *amplitude;            // Amplitude or I time series as defined by sample_type
  void *phase;                // Phase or Q time series as defined by sample_type
} s7k3_wcd;

// Reson 7k Generic Water Column data (record 7008)
typedef struct s7k3_WaterColumn_struct {
  s7k3_header header;
  u64 serial_number;   // Sonar serial number
  u32 ping_number;     // Sequential number
  u16 multi_ping;      // Flag to indicate multi-ping mode
                        //   0 = no multi-ping
                        //  >0 = sequence number of ping in the multi-ping sequence
  u32 number_beams;    // Number of receiver beams
  u16 reserved;        // Reserved record
  u32 samples;         // Samples per ping
  u8 subset_flag;      // Bit field:
                        //   Bit 0:
                        //   0 - all beams and samples in ping
                        //   1 - beam and/or sample ping subset
                        //   Bit 1:
                        //   0 - aample ping subset
                        //   1 - beam ping subset
  u8 column_flag;      // Bit Field
                        //0 - All samples for a beam, followed by all
                        //samples for the next beam
                        //1 - Sample 1 for all beams, followed by Sample 2
                        //for all beams, etc
  u16 reserved2;       // Reserved record
  u32 sample_type;     // Bit field:
                        //Least significant bit corresponds to Bit 0. Each grouping
                        //of bits is to be treated as an unsigned integer of the
                        //specified width. E.g. magnitude is u4 with possible
                        //values in the range 0 to 16
                        //Bit   0-3: Magnitude
                        //0 - no magnitude
                        //1 - reserved
                        //2 - magnitude (16 bits)
                        //3 - magnitude (32 bits)
                        //Bit   4-7: Phase
                        //0 - no phase
                        //1 - reserved
                        //2 - phase (16 bits)
                        //3 - phase (32 bits)
                        //Bit  8-11: I and Q
                        //0 - no I and Q
                        //1 - signed 16 bit and signed 16 bit Q
                        //2 - signed 32 bit and signed 32 bit Q
                        //Bit 12-14: Beamforming flag
                        //0 - Beam formed data
                        //1 - Element data*/
  s7k3_wcd wcd[MBSYS_RESON7K_MAX_BEAMS]; // Device configuration information
} s7k3_WaterColumn;

// Reson 7k Vertical Depth (record 7009)
typedef struct s7k3_VerticalDepth_struct {
	s7k3_header header;
	f32 frequency;           // Sonar frequency in Hz
	u32 ping_number;  // Sequential number
	u16 multi_ping;   // Flag to indicate multi-ping mode
	                  //    0 = no multi-ping
	                  //    >0 = sequence number of ping
	                  //     in the multi-ping
	                  //     sequence
	f64 latitude;     // Latitude of vessel reference point
	                  //   in radians, -pi/2 to +pi/2
	f64 longitude;    // Longitude of vessel reference point in radians, -pi to +pi
	f32 heading;      // Heading of vessel at transmit time in radians
	f32 alongtrack;   // Sonar alongtrack distance from vessel reference point in meters
	f32 acrosstrack;  // Sonar alongtrack distance from vessel reference point in meters
	f32 vertical_depth;  // Sonar vertical depth with respect
	                     // to chart datum or vessel if
	                     // tide data are unavailable in meters
} s7k3_VerticalDepth;

// Reson 7k TVG data (record 7010)
typedef struct s7k3_TVG_struct {
  s7k3_header header;
  u64 serial_number;     // Sonar serial number
  u32 ping_number;       // Sequential number
  u16 multi_ping;        // Flag to indicate multi-ping mode
                        //  0 = no multi-ping
                        // >0 = sequence number of ping in the multi-ping sequence
  u32 n;                 // number of samples
  u32 reserved[8];       // Reserved records
  u32 nalloc;            // Number of bytes allocated to TVG array
  f32 *tvg;             // Array of TVG data
} s7k3_TVG;

// Reson 7k image data (record 7011)
typedef struct s7k3_Image_struct {
  s7k3_header header;
  u32 ping_number;         // Sequential number
  u16 multi_ping;          // Flag to indicate multi-ping mode
                        //       0 = no multi-ping
                        //      >0 = sequence number of ping in the multi-ping sequence
  u32 width;               // Image width in pixels
  u32 height;              // Image height in pixels
  u16 color_depth;         // Color depth per pixel in bytes
  u16 reserved;            // Reserved record
  u16 compression;         // Reserved for future use
  u32 samples;                   // Original samples prior to compression
  u32 flag;                // Bit field:
                        //    Bit 0: dB visualization
                        //    Bit 1: Un-stabilized beams
  f32 rx_delay;            // Rx delay in fractional samples, zero when not applicable.
  u32 reserved2[6];         // Reserved record
  u32 nalloc;              // Number of bytes allocated to image array
  void *image;             // Array of image data
} s7k3_Image;

// Reson 7k Ping Motion (record 7012)
typedef struct s7k3_PingMotion_struct {
  s7k3_header header;
  u64 serial_number;          // Sonar serial number
  u32 ping_number;            // Sequential number
  u16 multi_ping;             // Flag to indicate multi-ping mode
                                // 0 = no multi-ping
                        //        >0 = sequence number of ping in the multi-ping sequence
  u32 n;                      // number of samples
  u16 flags;                  // Bit field:
                                // Bit 0: Pitch stabilization applied / pitch field present
                                // Bit 1: Roll stabilization applied / roll field present
                                // Bit 2: Yaw stabilization applied / yaw field present
                                // Bit 3: Heave stabilization applied / heave field present
                                // Bit 4-15: Reserved
  u32 error_flags;            // Bit field:
                                // Bit 0: PHINS reference 0 = invalid, 1 = valid
                                // Bit 1-3: Reserved for PHINS
                                // Bit 4: Roll angle > 15 degrees
                                // Bit 5: Pitch angle > 35 degrees
                                // Bit 6: Roll rate > 10 degrees
                                // Bit 7: 1 = External motion data not received (roll
                                // angle and rate are not reported)
                                // Bit 8-15: Reserved
  f32 frequency;              // sampling frequency (Hz)
  u32 nalloc;                 // number of samples allocated
  f32 pitch;                  // Pitch value at the ping time (radians)
  f32 *roll;                  // Roll (radians)
  f32 *heading;               // Heading (radians)
  f32 *heave;                 // Heave (m)
} s7k3_PingMotion;

// Reson 7k Adaptive Gate (record 7014)
typedef struct s7k3_AdaptiveGate_struct {
  s7k3_header header;
  u16 record_size;       // Size of record header in bytes
  u64 serial_number;     // Sonar serial number
  u32 ping_number;       // Sequential number
  u16 multi_ping;        // Flag to indicate multi-ping mode
                        //     0 = no multi-ping
                        //    >0 = sequence number of ping in the multi-ping sequence
  u32 n;                 // Number of gate descriptors
  u16 gate_size;         // Size of gate descriptor information block in bytes
  u32 nalloc;            // Memory allocated for data (bytes)
  f32 *angle;            // Gate angle (radians)
  f32 *min_limit;        // Minimum sample number of gate limit
  f32 *max_limit;        // Maximum sample number of gate limit*/
} s7k3_AdaptiveGate;

// Reson 7k Detection Setup (record 7017)
typedef struct s7k3_DetectionDataSetup_struct {
  s7k3_header header;
  u64 serial_number;       // Sonar serial number
  u32 ping_number;         // Sequential number
  u16 multi_ping;          // Flag to indicate multi-ping mode
                        //       0 = no multi-ping
                        //      >0 = sequence number of ping in the multi-ping sequence
  u32 number_beams;        // Number of detection points
  u32 data_block_size;     // Size of detection information block in bytes
  u8 detection_algorithm;  // Detection algorithm:
                        //       0 = G1_Simple
                        //       1 = G1_BlendFilt
                        //       2 = G2
                        //       3 = G3
                        //       4 = IF1
                        //       5 = PS1 (beam detection)
                        //       6 = HS1 (beam detection)
                        //       7 = HS2 (pseudo beam detection)
                        //       8-255 = Reserved for future use
  u32 detection_flags;     // Bit field:
                        //       Bit 0: 1 = User-defined depth filter enabled
                        //       Bit 1: 1 = User-defined range filter enabled
                        //       Bit 2: 1 = Automatic filter enabled
                        //       Bit 3: 1 = Nadir search limits enabled
                        //       Bit 4: 1 = Automatic window limits enabled
                        //       Bit 5: 1 = Quality filter enabled
                        //       Bit 6: 1 = Multi detection enabled
                        //       Bits 7-31: Reserved for future use
  f32 minimum_depth;               // Minimum depth for user-defined filter (meters)
  f32 maximum_depth;               // Maximum depth for user-defined filter (meters)
  f32 minimum_range;               // Minimum range for user-defined filter (meters)
  f32 maximum_range;               // Maximum range for user-defined filter (meters)
  f32 minimum_nadir_search;        // Minimum depth for automatic filter nadir search (meters)
  f32 maximum_nadir_search;        // Maximum depth for automatic filter nadir search (meters)
  u8 automatic_filter_window; // Automatic filter window size (percent depth)
  f32 applied_roll;                // Roll value (in radians) applied to gates;
                                //      zero if roll stabilization is on
  f32 depth_gate_tilt;             // Angle in radians (positive to starboard)
  f32 nadir_depth;                 // Nadir depth used mb MB2
  u32 reserved[13];                // Reserved for future use
  u16 beam_descriptor[MBSYS_RESON7K_MAX_BEAMS];
                                     // Beam number the detection is taken from
  f32 detection_point[MBSYS_RESON7K_MAX_BEAMS];
                                     // Non-corrected fractional sample number with
                                //      the reference to the receiver's acoustic center
                                //      with the zero sample at the transmit time
  u32 flags[MBSYS_RESON7K_MAX_BEAMS];
                                     // Bit field:
                                //      Bit 0: 1 = automatic limits valid
                                //      Bit 1: 1 = User-defined limits valid
                                //      Bit 2-8: Quality type, defines the type of the quality field
                                //      Bit 9: 1 = Quality passes user-defined criteria
                                //      or no user-defined criteria was specified
                                //      Bit 10-12: Detection type (1 or more of the following)
                                //        Bit 10: Magnitude based detection
                                //        Bit 11: Phase based detection
                                //        Bit 12: Reserved
                                //      Bit 13-15: Reserved for future use
                                //      Bit 16-19: Detection priority number for detections
                                //      within the same beam (Multi-detect only). Value zero is
                                //      highest priority
  f32 auto_limits_min_sample[MBSYS_RESON7K_MAX_BEAMS];
                                        // Minimum sample number for automatic limits
  f32 auto_limits_max_sample[MBSYS_RESON7K_MAX_BEAMS];
                                        // Maximum sample number for automatic limits
  f32 user_limits_min_sample[MBSYS_RESON7K_MAX_BEAMS];
                                        // Minimum sample number for user-defined limits
  f32 user_limits_max_sample[MBSYS_RESON7K_MAX_BEAMS];
                                        // Maximum sample number for user-defined limits
  u32 quality[MBSYS_RESON7K_MAX_BEAMS]; // Bit field:
                                //         Bit 0: 1 = Brightness filter passed
                                //         Bit 1: 1 = Colinearity filter passed
                                //         Bit 2-31: Reserved for future use
  f32 uncertainty[MBSYS_RESON7K_MAX_BEAMS];
                                        // Detection uncertainty represented as an error
                                //         normalized to the detection point
} s7k3_DetectionDataSetup;

// Reson 7k amplitude and phase data (part of record 7018)
typedef struct s7k3_amplitudephase_struct {
  u16 beam_number;   // Beam or element number
  u32 number_samples; // Number of samples
  u32 nalloc;         // Number of samples allocated
  u16 *amplitude;     // Amplitude time series
  i16 *phase;         // Phase time series (radians scaled by 10430)
} s7k3_amplitudephase;

// Reson 7k Beamformed Data (record 7018)
typedef struct s7k3_Beamformed_struct {
  s7k3_header header;
  u64 serial_number;   // Sonar serial number
  u32 ping_number;     // Sequential number
  u16 multi_ping;      // Flag to indicate multi-ping mode
                        //   0 = no multi-ping
                        //  >0 = sequence number of ping in the multi-ping sequence
  u16 number_beams;    // Total number of beams or elements in record
  u32 number_samples;               // Number of samples in each beam in this record
  u32 reserved[8];     // Reserved for future use
  s7k3_amplitudephase amplitudephase[MBSYS_RESON7K_MAX_BEAMS];
                       // amplitude and phase data for each beam
} s7k3_Beamformed;

// Reson 7k angle and magnitude data (part of record 7019)
typedef struct s7k3_anglemagnitude_struct {
  u16 beam_number;    // Beam or element number
  u32 n;              // Number of samples
  u32 nalloc;         // Number of samples allocated
  i16 *angle;         // Vertical angle for samples
  u16 *magnitude;     // Magnitude for samples
  u16 *coherence;      // Coherence data, total size 'decimated samples' times
                      //   'beams' times 2 bytes
  u16 *cross_power;    // Cross Power data, total size 'decimated samples' times
                      //   'beams' times 2 bytes
  u16 *quality_factor; // Coherence data, total size 'decimated samples' times
                      //   'beams' times 2 bytes
  u16 *reserved;       // Coherence data, total size 'decimated samples' times
                      //   'beams' times 2 bytes
} s7k3_anglemagnitude;

// Reson 7k Vernier Processing Data Raw (record 7019)
typedef struct s7k3_VernierProcessingDataRaw_struct {
  s7k3_header header;
  u64 serial_number;       // Sonar serial number
  u32 ping_number;         // Sequential number
  u16 multi_ping;          // Multi-ping sequence number
                        //    0 = single ping
  u8 reference_array;      // Index of reference array
  u8 pair1_array2;         // Index of reference array
  u8 pair2_array2;         // Index of reference array
  u8 decimator;            // Data decimated by this factor
  u16 beam_number;         // Total number of beams or elements in record
  u32 n;                   // Number of samples in each beam in this record
  u32 decimated_samples;   // Number of samples in output angle data after filtering
                        //    and decimation and clipping (where 'First Sample' > 0)
  u32 first_sample;        // Index of first sample (base-0)
  u32 reserved[2];         // Reserved
  u16 smoothing_type;      // Smoothing window type:
                        //     0 - retangular
                        //     2 - hamming
                        //     99 - None
  u16 smoothing_length;    // Smoothing window length [samples]
  u32 reserved2[2];         // Reserved
  f32 magnitude;           // Magnitude threshold for determination of data quality
  f32 min_qf;              // Minimum quality factor (QF), default 0.5
  f32 max_qf;              // Minimum quality factor (QF), default 3.5
  f32 min_angle;           // Lower limit on possible elevation angles,
                        //    normally -45 degrees (in radians)
  f32 max_angle;           // Upper limit on possible elevation angles,
                        //    normally -45 degrees (in radians)
  f32 elevation_coverage;  // Normally 90 degrees (in radians)
  u32 reserved3[4];         // Reserved
  s7k3_anglemagnitude anglemagnitude[MBSYS_RESON7K_MAX_BEAMS];
                           // Angle and magnitude data for each beam plus
                        //    additional records
} s7k3_VernierProcessingDataRaw;

// Reson 7k BITE field (part of record 7021)
typedef struct s7k3_bitefield_struct {
  u16 field;             // Field number
  c8 name[64];           // Name - null terminated ASCII string
  u8 device_type;        // Device type:
                        //    1 = Error count
                        //    2 = FPGA die temperature
                        //    3 = Humidity
                        //    4 = Serial 8-channel ADC
                        //    5 = Firmware version
                        //    6 = Head Temp, 8K WetEnd
                        //    7 = Leak V, 8K WetEnd
                        //    8 = 5 Volt, 8K WetEnd
                        //    9 = 12 Volt, 8K WetEnd
                        //   10 = DipSwitch, 8K WetEnd
                        //   12 = Activity counter. Release an alarm if it increments
                        //        too slowly
                        //   13 = Error counter. Releases an alarm if it increments
                        //        too much, too fast
                        //  100 = Display 'Value' with 0 digits; scale 1
                        //  101 = Display 'Value' with 1 digit; scale 0.1
                        //  102 = Display 'Value' with 2 digits; scale 0.01
                        //  103 = Display 'Value' with 3 digits; scale 0.001
                        //  110 = Display as 4 hex digits
                        //  111 = Display as 8 bit Binary
                        //  112 = Display as Enumeration (literals defined in Bite.html file)
                        //  200 = Display as part number
                        //  201 = Part revision. High order 8 bits is the revision number,
                        //        Low order 8 bit is an ASCII character
                        //  250 = Display as positive number
  f32 minimum;           // Minimum value
  f32 maximum;           // Maximum value
  f32 value;             // Current value
} s7k3_bitefield;

// Reson 7k BITE (part of record 7021)
typedef struct s7k3_bitereport_struct {
  c8 source_name[64];              // source name - null terminated string
  u8 source_address;               // source address
  f32 reserved;                    // reserved
  u16 reserved2;                    // reserved
  s7k3_time downlink_time;          // Downlink time sent
  s7k3_time uplink_time;            // Uplink time received
  s7k3_time bite_time;              // BITE time received
  u8 status;                       // Bit field:
                                //    Bit 0:
                                //    0 = Uplink ok
                                //    1 = Uplink error
                                //    Bit 1:
                                //    0 = Downlink ok
                                //    1 = Downlink error
                                //    Bit 2:
                                //    0 = BITE ok
                                //    1 = BITE error
                                //    Bit 3-4:
                                //    0 = OK
                                //    1 = Warning
                                //    2 = Error
                                //    3 = Fatal
  u16 number_bite;                 // Number of valid BITE fields for this board
  u64 bite_status[4];              // Each bit delineates status of one BITE channel up to 256:
                                //    Bit 0:
                                //    0 = BITE field #0 within range
                                //    1 = BITE field #0 out of range
                                //    Bit 1:
                                //    0 = BITE field #255 within range
                                //    1 = BITE field #255 out of range
  s7k3_bitefield bitefield[256];    // Array of BITE field data
} s7k3_bitereport;

// Reson 7k BITE (record 7021)
typedef struct s7k3_BITE_struct {
  s7k3_header header;
  u16 number_reports;       // Number of Built In Test Environment reports
  u32 nalloc;
  s7k3_bitereport *bitereports;
} s7k3_BITE;

// Reson 7k Sonar Source Version (Record 7022)
typedef struct s7k3_SonarSourceVersion_struct {
  s7k3_header header;
  c8 version[32]; // Null terminated ASCII string
} s7k3_SonarSourceVersion;

// Reson 7k 8k wet end version (Record 7023)
typedef struct s7k3_WetEndVersion8k_struct {
  s7k3_header header;
  c8 version[32]; // Null terminated ASCII string
} s7k3_WetEndVersion8k;

// Reson 7k raw detection data (part of Records 7027 and 7047)
typedef struct s7k3_bathydata_struct {
  f32 depth;                        // Depth relative chart datum (or relative to
                                //     waterline if Height source = 0) (in meters)
  f32 alongtrack;                   // Alongtrack distance in vessel grid in meters
  f32 acrosstrack;                  // Acrosstrack distance in meters
  f32 pointing_angle;               // Pointing angle from vertical in radians
  f32 azimuth_angle;                // Azimuth angle in radians
} s7k3_bathydata;

// Reson 7k raw detection data (part of Record 7027)
typedef struct s7k3_rawdetectiondata_struct {
  u16 beam_descriptor;             // Beam number the detection is taken from
  f32 detection_point;             // Non-corrected fractional sample number with
                                //      the reference to the receiver's
                                //      acoustic center with the zero sample
                                //      at the transmit time
  f32 rx_angle;                    // Beam steering angle with reference to
                                //      receiver's acoustic center in the
                                //      sonar reference frame, at the detection
                                //      point, in radians
  u32 flags;                       // Bit fields:
                                //      Bit 0: 1 = Magnitude based detection
                                //      Bit 1: 1 = Phase based detection
                                //      Bits 2-8: Quality type, defines the type
                                //      of the quality field below
                                //      Bits 9-12: Detection priority number for
                                //      detections within the same beam (Multi-detect
                                //      only). Value zero is highest priority.
                                //      Bit 13: Reserved
                                //      Bit 14: Snippet detection point flag
                                //      0 = Detection used in snippet
                                //      1 = Not used in snippet
                                //      Bits 15-31: Reserved for future use
  u32 quality;                     // Detection quality:
                                //      Bit 0: 1 = Brightness filter passed
                                //      Bit 1: 1 = Co-linearity filter passed
  f32 uncertainty;                 // Detection uncertainty represented as an error
                                //      normalized to the detection point
  f32 signal_strength;             // Signal strength of detection point
  f32 min_limit;                   // Minimum sample number of gate limit
  f32 max_limit;                   // Maximum sample number of gate limit
} s7k3_rawdetectiondata;

// Reson 7k raw detection data (Record 7027)
typedef struct s7k3_RawDetection_struct {
  s7k3_header header;
  u64 serial_number;        // Sonar serial number
  u32 ping_number;          // Sequential number
  u16 multi_ping;           // Flag to indicate multi-ping mode
                        //        0 = no multi-ping
                        //       >0 = sequence number of ping in the multi-ping sequence
  u32 number_beams;         // Number of detection points
  u32 data_field_size;      // Size of detection information block in bytes
  u8 detection_algorithm;   // Detection algorithm:
                        //        0 = G1_Simple
                        //        1 = G1_BlendFilt
                        //        2 = G2
                        //        3 = G3
                        //        4 = IF1
                        //        5 = PS1 (beam detection)
                        //        6 = HS1 (beam detection)
                        //        7 = HS2 (beam detection)
                        //    8-255 = Reserved for future use
  u32 flags;                // Bit field:
                        //     Bit 0-3: Uncertainty method
                        //        0 = Not calculated
                        //        1 = Rob Hare's method
                        //        2 = Ifremer's method
                        //     3-15 = Reserved for future use
                        //     Bit 4: Multi-detection enabled
                        //     Bit 5: Reserved
                        //     Bit 6: Has Snippets detection point flag
                        //     Bit 7-31 = Reserved
  f32 sampling_rate;        // Sonar's sampling frequency in Hz
  f32 tx_angle;             // Applied transmitter steering angle, in radians. The angle is used for
                                //    pitch stabilization. It will be zero if the system doesn't have this feature.
                                //    The value is the same as the Projector Beam Steering Angle of the
                                //    7000 record. They are both filled with the same variable
  f32 applied_roll;         // Roll value (in radians) applied to gates;
                        //     zero if roll stabilization is ON.
  u32 reserved[15];         // Reserved
  s7k3_rawdetectiondata rawdetectiondata[MBSYS_RESON7K_MAX_SOUNDINGS];
  u32 optionaldata;                             // Flag indicating if bathymetry calculated and
                                          //       values below filled in
                                                  //  0 = No
                                                  //  1 = Yes
                                          //       This is an internal MB-System flag, not
                                          //       a value in the data format
  f32 frequency;                            // Ping frequency in Hz
  f64 latitude;                             // Latitude of vessel reference point
                                          //     in radians, -pi/2 to +pi/2, south negative
  f64 longitude;                            // Longitude of vessel reference point
                                          //     in radians, -pi to +pi, west negative
  f32 heading;                              // Heading of vessel at transmit time
                                          //     in radians
  u8 height_source;                         // Method used to correct to chart datum.
                                          //     0 = None
                                          //     1 = RTK (implies tide = 0.0)
                                          //     2 = Tide
  f32 tide;                                 // Tide in meters
  f32 roll;                                 // Roll at transmit time
  f32 pitch;                                // Pitch at transmit time
  f32 heave;                                // Heave at transmit time in m*/
  f32 vehicle_depth;                       // Vehicle depth at transmit time in m
  s7k3_bathydata bathydata[MBSYS_RESON7K_MAX_SOUNDINGS];  // Bathymetry calculated from raw detections
} s7k3_RawDetection;

// Reson 7k snippet data (part of record 7028)
typedef struct s7k3_snippetdata_struct {
  u16 beam_number;   // Beam or element number
  u32 begin_sample;  // First sample included in snippet
  u32 detect_sample; // Detection point
  u32 end_sample;    // Last sample included in snippet
  u32 nalloc;        // Bytes allocated to hold amplitude time series
  u8 *amplitude;     // Pointer to Amplitude time series, which may be 16 bit
                    //    or 32 bit values depending on the flags parameter
} s7k3_snippetdata;

// Reson 7k snippet data (record 7028)
typedef struct s7k3_Snippet_struct {
  s7k3_header header;
  u64 serial_number; // Sonar serial number
  u32 ping_number;   // Sequential number
  u16 multi_ping;    // Flag to indicate multi-ping mode
                        // 0 = no multi-ping
                        //>0 = sequence number of ping in the multi-ping sequence
  u16 number_beams;             // Number of detection points
  u8 error_flag;     // If set, record will not contain any data
                        // Flag itself will indicate an error:
                        // 0 = Ok
                        // 1-5 = Reserved
                        // 6 = Bottom detection failed (R7006)
                        // 7-255 = Reserved
  u8 control_flags;  // Control settings from RC 1118 command:
                        // Bit 0: Automatic snippet window is used
                        // Bit 1: Quality filter enabled
                        // Bit 2: Minimum window size is required
                        // Bit 3: Maximum window size is required
                        // Bit 4-7: Reserved
  u32 flags;          // Bit field:
                        // Bit 0: 0 = 16 bit snippets
                        // 1 = 32 bit snippets
  u32 reserved[6];    // Reserved for future use
  s7k3_snippetdata snippetdata[MBSYS_RESON7K_MAX_BEAMS];
  // Snippet time series for each beam
  u32 optionaldata;    // Optional data
  f32 frequency;       // Ping frequency in Hz
  f64 latitude;        // Latitude of vessel reference point in Radians -pi/2 to pi/2,
                        //south negative
  f64 longitude;       // Longitude of vessel reference point in Radians -pi/2 to pi/2,
                        //west negative
  f32 heading;         // Heading of vessel at transmit time in radians
  f32 beam_alongtrack[MBSYS_RESON7K_MAX_BEAMS];   // Along track distance in vessel grid in meters
  f32 beam_acrosstrack[MBSYS_RESON7K_MAX_BEAMS];  // Across track distance in vessel grid in meters
  u32 center_sample[MBSYS_RESON7K_MAX_BEAMS];     // Sample number at detection point of beam
} s7k3_Snippet;

// Reson 7k vernier Processing Data Filtered (part of record 7029)
typedef struct s7k3_vernierprocessingdatasoundings_struct {
  s7k3_header header;
  f32 beam_angle;     // Sounding horizontal angle (radians)
  u32 sample;         // Sounding sample number (convert to range using sample
                        // rate and sound velocity
  f32 elevation;      // Sounding vertical angle (radians)
  f32 reserved;       // Reserved
} s7k3_vernierprocessingdatasoundings;

// Reson 7k vernier Processing Data Filtered (Record 7029)
typedef struct s7k3_VernierProcessingDataFiltered_struct {
  s7k3_header header;
  u64 serial_number;    // Sonar serial number
  u32 ping_number;      // Sequential number
  u16 multi_ping;       // Flag to indicate multi-ping mode
                        //    0 = no multi-ping
                        //   >0 = sequence number of ping in the multi-ping sequence
  u16 number_soundings; // Number of soundings.
                        // There may be several soundings per beam
  f32 min_angle;        // Minimum elevation angle in all soundings (radians)
  f32 max_angle;        // Maximum elevation angle in all soundings (radians)
  u16 repeat_size;      // Size of sounding repeat blocks following (bytes)
  s7k3_vernierprocessingdatasoundings vernierprocessingdatasoundings[MBSYS_RESON7K_MAX_SOUNDINGS];
} s7k3_VernierProcessingDataFiltered;

// Reson 7k sonar installation parameters (record 7030)
typedef struct s7k3_InstallationParameters_struct {
  s7k3_header header;
  f32 frequency;                     // Sonar frequency (Hz)
  u16 firmware_version_len;          // Length of firmware version info in bytes
  u8 firmware_version[128];          // Firmware version info
  u16 software_version_len;          // Length of software version info in bytes
  u8 software_version[128];          // Software version info
  u16 s7k3_version_len;               // Length of 7k software version info in bytes
  u8 s7k3_version[128];               // 7k software version info
  u16 protocal_version_len;          // Length of protocol version info in bytes
  u8 protocal_version[128];          // Protocol version info
  f32 transmit_x;                    // Sonar transmit array X offset (m)
  f32 transmit_y;                    // Sonar transmit array Y offset (m)
  f32 transmit_z;                    // Sonar transmit array Z offset (m)
  f32 transmit_roll;                 // Sonar transmit array roll offset radiansm)
  f32 transmit_pitch;                // Sonar transmit array pitch offset (radians)
  f32 transmit_heading;              // Sonar transmit array heading offset (radians)
  f32 receive_x;                     // Sonar receive array X offset (m)
  f32 receive_y;                     // Sonar receive array Y offset (m)
  f32 receive_z;                     // Sonar receive array Z offset (m)
  f32 receive_roll;                  // Sonar receive array roll offset (radians)
  f32 receive_pitch;                 // Sonar receive array pitch offset (radians)
  f32 receive_heading;               // Sonar receive array heading offset (radians)
  f32 motion_x;                      // Motion sensor X offset (m)
  f32 motion_y;                      // Motion sensor Y offset (m)
  f32 motion_z;                      // Motion sensor Z offset (m)
  f32 motion_roll;                   // Motion sensor roll offset (radians)
  f32 motion_pitch;                  // Motion sensor pitch offset (radians)
  f32 motion_heading;                // Motion sensor heading offset (radians)
  u16 motion_time_delay;             // Motion sensor time delay (msec)
  f32 position_x;                    // Position sensor X offset (m)
  f32 position_y;                    // Position sensor Y offset (m)
  f32 position_z;                    // Position sensor Z offset (m)
  u16 position_time_delay;           // Position sensor time delay (msec)
  f32 waterline_z;                   // Vertical offset from reference
                                //        point to waterline (m)
} s7k3_InstallationParameters;

// Reson 7k BITE summary (Record 7031)
typedef struct s7k3_BITESummary_struct {
  s7k3_header header;
  u16 total_items;  // Total of all warnings, error or fatal level BITE status
                    //   items. If this field is not zero. then the following
                    //   fields can be examined to determine severity and source
                    //   of BITE alerts
  u16 warnings[4];  // Index 0 = Overall number of warnings
                        //Index 1 = Receiver related warning
                        //Index 2 = Transmitter related warnings
                        //Index 3 = Other (system) related warnings
  u16 errors[4];    // Index 0 = Overall number of errors
                        //Index 1 = Receiver related errors
                        //Index 2 = Transmitter related errors
                        //Index 3 = Other (system) related errors
  u16 fatals[4];    // Index 0 = Overall number of fatal status items
                        //Index 1 = Receiver related fatal status items
                        //Index 2 = Transmitter related fatal status items
                        //Index 3 = Other (system) related fatal status items
  u32 reserved[2];  // Reserved
} s7k3_BITESummary;

// Reson 7k Compressed Beamformed Magnitude Data (part of Record 7041)
typedef struct s7k3_beamformedmagnitude_struct {
  s7k3_header header;
  u32 beam;          // Identification for the beam
  u32 samples;       // Total number of samples recorded ofr this beam
  u32 nalloc;        // Bytes allocated to hold amplitude time series
  u32 *data;         // Data series for each sample
} s7k3_beamformedmagnitude;

// Reson 7k Compressed Beamformed Magnitude Data (Record 7041)
typedef struct s7k3_CompressedBeamformedMagnitude_struct {
  s7k3_header header;
  u64 serial_number; // Sonar serial number
  u32 ping_number;   // Sequential number
  u16 multi_ping;    // Flag to indicate multi-ping mode
                        // 0 = no multi-ping
                        //>0 = sequence number of ping in the multi-ping sequence
  u16 number_beams;  // Total number of beams in ping record
  u16 flags;         // Bit Field:
                        // Bit 0-1: Reserved
                        // Always 1 for backward compatibility with 7111 systems
                        // Bit 2-4: Down-sampling method
                        // 0 = no down sampling
                        // 1 = nearest neighbor
                        // 2 = linear approximation
                        // 3-7 = reserved
                        // Bit 5-7: Filtering method
                        // 0 = no filtering
                        // 1-7 = reserved
                        // Bit 8: Beam identification method
                        // 0 = beam number (u16)
                        // 1 = beam angle (f32, in radians)
                        // Bit 9-15: Reserved
  f32 sample_rate;   // Sampling rate for the data
  u32 reserved;      // Reserved
  s7k3_beamformedmagnitude beamformedmagnitude[MBSYS_RESON7K_MAX_BEAMS];
} s7k3_CompressedBeamformedMagnitude;

// Reson 7k Compressed Water Column Data (part of Record 7042)
typedef struct s7k3_compressedwatercolumndata_struct {
  u16 beam_number;    // Beam Number for this data.
  u8 segment_number;  // Segment number for this beam. Optional field, see ‘Bit 14’ of Flags.
  u32 samples;        // Number of samples included for this beam.
  u32 nalloc;      // Bytes allocated to hold the time series
  u8 *data;         // Pointer to time series. Each “Sample” may be one of the
                        // following, depending on the Flags bits:
                        //  A) 16 bit Mag & 16bit Phase (32 bits total)
                        //  B) 16 bit Mag (16 bits total, no phase)
                        //  C) 8 bit Mag & 8 bit Phase (16 bits total)
                        //  D) 8 bit Mag (8 bits total, no phase)
                        //  E) 32 bit Mag & 8 bit Phase(40 bits total)
                        //  F) 32 bit Mag(32 bits total, no phase)
} s7k3_compressedwatercolumndata;

// Reson 7k Compressed Water Column Data (Record 7042)
typedef struct s7k3_CompressedWaterColumn_struct {
  s7k3_header header;
  u64 serial_number;       // Sonar serial number
  u32 ping_number;         // Sequential number
  u16 multi_ping;          // Flag to indicate multi-ping mode
                        //    0 = no multi-ping
                        //   >0 = sequence number of ping in the multi-ping sequence
  u16 number_beams;        // Total number of beams in ping record
  u32 samples;             //
  u32 compressed_samples;  // Number of samples (maximum over all beams if Flags bit 0 set
                        //    [samples per beam varies]. Otherwise same as Samples(N) )
                        //    When all beams come with the same number of samples
                        //    'Compressed Samples' is the same as 'Samples(N)' for each
                        //    beam in the data section of the record. But if bit 0
                        //    is set in the 'Flags' the beams are individually cut based
                        //    on bottom detection and thus have all different length.
                        //    'Compressed Samples' then gives you the maximum number of
                        //    samples of the beam with the longest range.
                        //    Same as the largest value of 'Samples(N)' in the data
                        //    section.
  u32 flags;               // Bit field:
                        //    Bit 0 : Use maximum bottom detection point in each beam to
                        //    limit data. Data is included up to the bottom detection
                        //    point + 10%. This flag has no effect on systems which
                        //    do not perform bottom detection.
                        //    Bit 1 : Include magnitude data only (strip phase)
                        //    Bit 2 : Convert mag to dB, then compress from 16 bit to
                        //    8 bit by truncation of 8 lower bits. Phase compression simply
                        //    truncates lower (least significant) byte of phase data.
                        //    Bit 3 : Reserved.
                        //    Bit 4-7 : Downsampling divisor. Value = (BITS >> 4). Only
                        //    values 2-16 are valid. This field is ignored if downsampling
                        //    is not enabled (type = “none”).
                        //    Bit 8-11 : Downsampling type:
                        //      0x000 = None
                        //      0x100 = Middle value
                        //      0x200 = Peak value
                        //      0x300 = Average value
                        //    Bit 12 : 32 Bits data
                        //    Bit 13 : Compression factor available
                        //    Bit 14 : Segment numbers available
                        //    Bit 15 : First sample contains RxDelay value.
  u32 first_sample;        // First sample included for each beam. Normally zero,
                        //    unless power saving mode “Range Blank” or absolute gate
                        //    (bit 3) is in effect. See RC 1046 for details. Thus,
                        //    the samples in each beam data section will run from F
                        //    to F+N-1. Construction of a correct water column image
                        //    must take this into account.
  f32 sample_rate;         // Effective sample rate after downsampling, if specified.
  f32 compression_factor;  // Factor used in magnitude compression.
  u32 reserved;            // Zero. Reserved for future use.
  size_t magsamplesize;    // calculated bytes per sample for magnitude (not stored in file)
  size_t phasesamplesize;  // calculated bytes per sample for phase (not stored in file)
  s7k3_compressedwatercolumndata compressedwatercolumndata[MBSYS_RESON7K_MAX_BEAMS];
} s7k3_CompressedWaterColumn;

// Reson 7k Segmented Raw Detection Data (part of Record 7047)
typedef struct s7k3_segmentedrawdetectionrxdata_struct {
  u16 beam_number;      // Beam number the detection is taken from
  u16 used_segment;     // Number of Segment descriptor.
  f32 detection_point;  // Non-corrected fractional sample number with reference
                        // to receiver’s acoustic center with the zero sample at the
                        // transmit time
  f32 rx_angle_cross;   // Beam steering angle with reference to receiver’s
                        // acoustic center in the sonar reference frame,
                        // at the detection point; in radians
  u32 flags2;            // BIT FIELD:
                        // Bit 0:
                        // 1 – Magnitude based detection
                        // Bit 1:
                        // 1 – Phase based detection
                        // Bit 2-8:
                        // Quality type, defines the type of the quality field below
                        // Bits 9-12:
                        // Detection priority number for detections within the same beam
                        // (Multi-detect only). Value zero is highest priority.
                        // Bit 13:
                        // Interferometry between beam point.
                        // Bit 14:
                        // Snippet detection point flag
                        // 0 – Detection used in snippets
                        // 1 – Not used in snippets
                        // Bits 15-31: Reserved for future use
  u32 quality;          // 0 - Quality is not available / Not used
                        // 1 - Bit field:
                        // Bit 0: 1 = Brightness filter passed
                        // Bit 1: 1 = Co-linearity filter passed
                        // 3-31 - Reserved for future use
  f32 uncertainty;      // Detection uncertainty represented as an error normalized
                        // to the detection point
  f32 signal_strength;  // Signal strength of detection point
  f32 sn_ratio;         // S/N ratio in dB
} s7k3_segmentedrawdetectionrxdata;

// Reson 7k Segmented Raw Detection Data (part of Record 7047)
typedef struct s7k3_segmentedrawdetectiontxdata_struct {
  u16 segment_number;   // Number of the Segment descriptor
  f32 tx_angle_along;   // Applied transmitter along steering angle, in radians
  f32 tx_angle_across;  // Applied transmitter across steering angle, in radians
  f32 tx_delay;         // Transmit delay in seconds
  f32 frequency;        // Hz
  u32 pulse_type;       // BIT FIELD
                        // Bit 0-7: 0=CW, 1=FM(chirp), 2-10=reserved 11=Barker11,
                        // 12=Barer11r, 13=Barker13, 14=Barker13r
                        // Bit8-9: 0=linear, 1=Parametric
                        // Bit 10: Pilot Pulse
  f32 pulse_bandwidth;  // “+”=up Chirp; “-“=down Chirp in Hz
                        // ChirpStartFrequency=Frequency-ChirpHeave/2
  f32 tx_pulse_width;   // In seconds
  f32 tx_pulse_width_across;    // Tx -3dB beam width cross , in radians
  f32 tx_pulse_width_along;     // Tx -3dB beam width along , in radians
  u32 tx_pulse_envelope;        // 0 = rectangular
                                // 1 = Tukey (rectangular->Hann, variable)
                                // 2 = Hamming
                                // 3 = Deconv
  f32 tx_pulse_envelope_parameter;  // eg: Tukey.Alpha value.
  f32 tx_relative_src_level;    // Tx relative Src Level In %.
  f32 rx_beam_width;        // Rx -3dB beam width
  u8 detection_algorithm;       // 0 – G1_Simple
                                // 1 – G1_BlendFilt
                                // 2 – G2
                                // 3 – G3
                                // 4 – IF1
                                // 5 – PS1 (beam detection)
                                // 6 – HS1 (beam detection)
                                // 7 – HS2 (pseudo beam detection)
                                // 8-255 – Reserved for future use
  u32 flags;            // BIT FIELD:
                        // Bit 0-3:
                        // Uncertainty method
                        // 0 – Not calculated
                        // 1 – Rob Hare’s method
                        // 2 – Ifremer’s method
                        // 3 – Reserved for future use
                        // Bit 4:
                        // Multi-detection enabled
                        // Bit 5:
                        // Beam data is pulse correlated
                        // Bit 6:
                        // Has Snippets detection point flag. Indicates that Snippet
                        // detection point flags will be polulated. (Rx flag bit 14)
                        // Bit 7-31: Reserved for future use
  f32 sampling_rate;    // Sonar’s sampling frequency in Hz
  u8 tvg;               // Applied TVG value
  f32 rx_bandwidth;     // In Hz
} s7k3_segmentedrawdetectiontxdata;

// Reson 7k Segmented Raw Detection Data (Record 7047)
typedef struct s7k3_SegmentedRawDetection_struct {
  s7k3_header header;
  u16 record_header_size;  // Size of record header in bytes
  u32 n_segments;          // Number of Segment descriptors
  u16 segment_field_size;  // Size of transmitter descriptor block in bytes
  u32 n_rx;                // Number of Rx detection points
  u16 rx_field_size;       // Size of detection information block in bytes
  u64 serial_number;       // Sonar serial number
  u32 ping_number;         // Sequential number
  u16 multi_ping;          // Flag to indicate multi-ping mode
                        //    0 = no multi-ping
                        //   >0 = sequence number of ping in the multi-ping sequence
  f32 sound_velocity;      // Sound velocity at the transducer in meters/second
  f32 rx_delay;            // Delay between start of first Tx pulse and start of sample
                        //    data recoding in fractional samples.
  s7k3_segmentedrawdetectiontxdata segmentedrawdetectiontxdata[MBSYS_RESON7K_MAX_SEGMENTS];
  s7k3_segmentedrawdetectionrxdata segmentedrawdetectionrxdata[MBSYS_RESON7K_MAX_SOUNDINGS];
  u32 optionaldata;                             // Flag indicating if bathymetry calculated and
                                          //       values below filled in
                                                  //  0 = No
                                                  //  1 = Yes
                                          //       This is an internal MB-System flag, not
                                          //       a value in the data format
  f32 frequency;                            // Ping frequency in Hz
  f64 latitude;                             // Latitude of vessel reference point
                                          //     in radians, -pi/2 to +pi/2, south negative
  f64 longitude;                            // Longitude of vessel reference point
                                          //     in radians, -pi to +pi, west negative
  f32 heading;                              // Heading of vessel at transmit time
                                          //     in radians
  u8 height_source;                         // Method used to correct to chart datum.
                                          //     0 = None
                                          //     1 = RTK (implies tide = 0.0)
                                          //     2 = Tide
  f32 tide;                                 // Tide in meters
  f32 roll;                                 // Roll at transmit time
  f32 pitch;                                // Pitch at transmit time
  f32 heave;                                // Heave at transmit time in m*/
  f32 vehicle_depth;                       // Vehicle depth at transmit time in m
  s7k3_bathydata bathydata[MBSYS_RESON7K_MAX_SOUNDINGS];  // Bathymetry calculated from raw detections
} s7k3_SegmentedRawDetection;

// Reson 7k Calibrated Beam Data (Record 7048)
typedef struct s7k3_CalibratedBeam_struct {
  s7k3_header header;
  u64 serial_number;        // Sonar serial number
  u32 ping_number;          // Sequential number
  u16 multi_ping;           // Flag to indicate multi-ping mode
                        //     0 = no multi-ping
                        //    >0 = sequence number of ping in the multi-ping sequence
  u16 first_beam;           // Total number of beams in ping record
  u16 total_beams;          // Total number of beams in ping record
  u32 total_samples;        // Total number of samples in ping record
  u8 foward_looking_sonar;  // FLS flag
  u8 error_flag;            // If set, record contains original non-calibrated beamformed
                        //     data. Flag itself will indicate an error.
                        //     0 - OK
                        //     1 - No calibration
                        //     2 - TVG read error (R7010)
                        //     3 - CTD not available (R1010)
                        //     4 - Invalid or not available geometry (R7004)
                        //     5 - Invalid sonar specifications (XML)
                        //     6 - Bottom detection failed
                        //     7 - No power (Power is set to zero)
                        //     8 - No gain (Gain is too low)
                        //     128-254 - Reserved for internal errors
                        //     255 - System cannot be calibrated (c7k file missing)
  u32 reserved[8];          // Reserved for future use
  f32 *sample;              // Amplitude series for each beam. First sample represents
                        //     range 0 meters
} s7k3_CalibratedBeam;

// Reson 7k Reserved (Record 7049)

// Reson 7k System Events (part of Record 7050)
typedef struct s7k3_systemeventsdata_struct {
  s7k3_header header;
  u16 event_type;
  u16 event_id;
  u32 device_id;
  u16 system_enum;
  u16 event_message_length;
  s7k3_time s7kTime;
  u8 *event_message;
} s7k3_systemeventsdata;

// Reson 7k System Events (Record 7050)
typedef struct s7k3_SystemEvents_struct {
  s7k3_header header;
  u64 serial_number;
  u32 number_events;
  s7k3_systemeventsdata systemeventsdata;
} s7k3_SystemEvents;

// Reson 7k System Event Message (record 7051)
typedef struct s7k3_SystemEventMessage_struct {
  s7k3_header header;
  u64 serial_number;    // Sonar serial number
  u16 event_id;         // Event id:
                        //    0: success
                        //    1: information (used for MB-System comment record)
                        //    2: warning
                        //    3: error
                        //    4: fatal
  u16 message_length;   // Message length in bytes
  u16 event_identifier; // Undefined
  u32 message_alloc;    // Number of bytes allocated for message
  c8 *message;          // Message string (null terminated)
} s7k3_SystemEventMessage;

// Reson 7k RDR Recording Status (part of Record 7052)
typedef struct s7k3_rdrrecordingstatusdata_struct {
  s7k3_header header;
  u32 threshold_length;
  u32 *threshold_value_array;
  u32 included_records;
  u32 *included_records_array;
  u32 excluded_records;
  u32 *excluded_records_array;
  u32 included_devices;
  u32 *included_devices_array;
  u32 excluded_devices;
  u32 *excluded_devices_array;
} s7k3_rdrrecordingstatusdata;

// Reson 7k RDR Recording Status (Record 7052)
typedef struct s7k3_RDRRecordingStatus_struct {
  s7k3_header header;
  u32 position;            // Seconds since start of recording
  u8 disk_free;            // Percentage of disk space free (0 - 100)
  u8 mode;                 // Bit field:
                        //    Bit 0-5:
                        //    0 - Stopped
                        //    1 - Recording
                        //    2 - Playing
                        //    3 - Deleting
                        //    4 - Stopping
                        //    5+ - Reserved
  u32 filerecords;         // Total number of records in file at the time the request
                        //    is processed
  u64 filesize;            // File size in bytes
  u8 first_time[10];     // Time tag first record time
  u8 last_time[10];      // Time tag last record time
  u32 totaltime;           // Time span between first and last record (in seconds)
  c8 directory_name[256];  // Current directory name. Null-terminated ASCII string
  c8 filename[256];        // Current file name. Null-terminated ASCII string
  u32 error;               // Error code
  u32 flags;               // Bit 0: External logger supported
                        //    Bit 1: External logger attached
                        //    Bit 2: External logger confirmed
                        //    Bit 3: Custom logger supported
  u32 logger_address;      // IP address of stand alone 7K when connected (little
                        //    endian data order)
  u8 file_number;          // Zero = write logfiles of multiple 1GB files
                        //    Non-zero = write single 7K logfile
  u8 ping_data;            // Zero = no lead-in ping data
                        //    Non-zero = write 10 sec of lead-in ping data
  u16 reserved;            // Reserved
  u32 reserved2[4];         // Reserved
  s7k3_rdrrecordingstatusdata rdrrecordingstatusdata;
} s7k3_RDRRecordingStatus;

// Reson 7k Subscriptions (part of Record 7053)
typedef struct s7k3_subscriptionsdata_struct {
  s7k3_header header;
  u32 address;           // IP Address (little endian data order)
  u16 port;              // Port number
  u16 type;              // 0 - UPD; 1 - TCP
  u32 records_number;    // Number of records
  u32 record_list[64];   // Array of records ID; N - # of valid records
  u32 reserved[128];     // Reserved
} s7k3_subscriptionsdata;

// Reson 7k Subscriptions (Record 7053)
typedef struct s7k3_Subscriptions_struct {
  s7k3_header header;
  i32 n_subscriptions; // Number of subscriptions
  s7k3_subscriptionsdata *subscriptionsdata;
} s7k3_Subscriptions;

// Reson 7k RDR Storage Recording (Record 7054)
typedef struct s7k3_RDRStorageRecording_struct {
  s7k3_header header;
  u16 diskfree_percentage; // Percentage of free disk space
  u32 number_records;      // Number of records logged to record file
  u64 size;                // Size of recording file
  u32 reserved[4];         // Reserved
  u8 mode;                 // RDR mode
  c8 file_name[256];       // The name of the recording file
  u32 RDR_error;           // Current RDR error code
  u64 data_rate;           // Bytes written per second
  u32 minutes_left;        // Available time left to log in minutes (max 24 hours)
} s7k3_RDRStorageRecording;

// Reson 7k Calibration Status (Record 7055)
typedef struct s7k3_CalibrationStatus_struct {
  s7k3_header header;
  u64 serial_number;       // Sonar serial number
  u16 calibration_status;  // 0 - Calibration is not available
                        //    1 - Calibration was not done
                        //    8 - Calibration is in progress
                        //    16 - Calibration completed
                        //    >127 - Calibration failed
  u16 percent_complete;    // If status is 8 (in progress) this field indicates percentage
                        //    completed. If calibration status is 16 (completed) this
                        //    field indicates the following:
                        //    0 - Results of previous calibration used without validation
                        //    1-99 - Results of previous calibration validated and used
                        //    100 - Full calibration performed
  u8 calibration_time[10]; // Completion time of most recent calibration (zero if none).
                        //    TIME_7K format (UTC). If calibration status is 1 (not
                        //    done), calibration time other than zero indicates that
                        //    previous calibration results are available but not
                        //    validated.
  c8 status_message[800];  // Status message text string (null terminated)
  u32 sub_status;          // Status details
                        //    0 - Ok
                        //    1 - No license file
                        //    2 - License file corrupt
                        //    3 - Invalid version
                        //    10 - Failed - noise
                        //    11 - Failed - ceramics bad
                        //    12 - Failed - magnitude tolerance
                        //    13 - Failed - phase tolerance
  u32 optionaldata;        // Flag indicating if optional data are filled in
                        //    0 = No
                        //    1 = Yes
                        //    This is an internal MB-System flag,
                        //    not a value in the data format
  u8 system_calibration;   // Bitfield indicating which system(s) are being calibrated
                        //    Bit 0: enum
                        //    Bit 1: enum
                        //    Etc
  u8 done_calibration;     // Bitfield indicating which ones are already done
  u8 current_calibration;  // Enum of system being calibrated
  u8 startup_calibration;  // Non zero if start-up calibration is in progress
  u16 status[8];           // Final status of each system calibrated
  u32 reserved[2];         // Reserved
} s7k3_CalibrationStatus;

// Reson 7k Calibrated Sidescan Data (part of record 7057)
typedef struct s7k3_calibratedsidescanseries_struct {
  u32 nalloc;            // Bytes allocated to hold the time series
  u64 *portbeams;        // Magnitude/Phase series. First sample represents range
                        //  0 meters (total bytes per side)
  u64 *starboardbeams;   // Magnitude/Phase series. First sample represents range
                        //  0 meters (total bytes per side)
  u64 *portnumber_beams;        // Magnitude/Phase series. First sample represents range
                        //  0 meters (total bytes per side)
  u64 *starboardnumber_beams;   // Magnitude/Phase series. First sample represents range
                        //  0 meters (total bytes per side)
} s7k3_calibratedsidescanseries;

// Reson 7k Calibrated Sidescan Data (record 7057)
typedef struct s7k3_CalibratedSideScan_struct {
  s7k3_header header;
  u64 serial_number;  // Sonar serial number
  u32 ping_number;    // Sequential number
  u16 multi_ping;     // Flag to indicate multi-ping sequence. Always 0 (zero) if not
                      //   in multi-ping mode; otherwise this represents the sequence
                      //   number of the ping in the multi-ping sequence
  f32 beam_position;  // Meters forward from position of beam 0
  u32 reserved;       // Controls Bit field:
                      //   Bit 0-31: Reserved
  u32 samples;        // Samples per side (port/starboard)
  f32 reserved2;      // Reserved
  u16 beams;          // Number of beams per side
  u16 current_beam;   // Beam number of this record's data (0 to N-1)
  u8 bytes_persample; // Number of bytes per sample
                      //   4 - Single precision (u32)
  u8 data_types;      // Bit field:
                      //   Bit 0: Reserved (always 0)
                      //   Bit 1-7: Reserved
  u8 error_flag;      // If set, record contains original non-calibrated beamformed
                        // data. Flag itself will indicate an error.
                        //  0 = Ok
                        //  1 = No calibration
                        //  2 = TVG read error (R7010)
                        //  3 = CTD not available (R1010)
                        //  4 = Invalid or not available geometry (R7004)
                        //  5 = Invalid sonar specifications (XML)
                        //  6 = Bottom detection failed (R7006)
                        //  7 = No power (Power is set to zero)
                        //  8 = No gain (Gain is too low)
                        //  128-254 = Reserved for internal errors
                        //  255 = System cannot be calibrated (c7k file missing)
  s7k3_calibratedsidescanseries calibratedsidescanseries;
  u32 optionaldata;    // Optional data
  f32 frequency;       // Ping frequency in Hz
  f64 latitude;        // Latitude of vessel reference point in Radians -pi/2 to pi/2,
                        //south negative
  f64 longitude;       // Longitude of vessel reference point in Radians -pi/2 to pi/2,
                        //west negative
  f32 heading;         // Heading of vessel at transmit time in radians
  f32 depth;           // Depth for slant range correction in meters
} s7k3_CalibratedSideScan;

// Reson 7k Snippet Backscattering Strength (part of Record 7058)
typedef struct s7k3_snippetbackscatteringstrengthdata_struct {
  s7k3_header header;
  u16 beam_number;    // Beam or element number
  u32 begin_sample;   // First sample number in beam from transmitter and outward
  u32 bottom_sample;  // Bottom detection point in beam from transmitter and outward
  u32 end_sample;     // Last sample number in beam from transmitter and outward
  u32 nalloc;         // Bytes allocated to hold the time series
  f32 *bs;            // Backscattering Strength (BS) for each sample. BS = 10 log10(sigma),
                      //   where 'sigma' is the backscattering cross section. The snippet
                      //   vector of each beam is ordered in samples of increasing range
                      //   from the transmitter.
  f32 *footprints;    // Footprint area series for each sample in square meters. Only
                      //   available when control flag bit 6 is set
} s7k3_snippetbackscatteringstrengthdata;

// Reson 7k Snippet Backscattering Strength (Record 7058)
typedef struct s7k3_SnippetBackscatteringStrength_struct {
  s7k3_header header;
  u64 serial_number;  // Sonar serial number
  u32 ping_number;    // Sequential number
  u16 multi_ping;     // Flag to indicate multi-ping sequence. Always 0 (zero) if not
                      //   in multi-ping mode; otherwise this represents the sequence
                      //   number of the ping in the multi-ping sequence
  u16 number_beams;   // Number of detection points
  u8 error_flag;      // If set, record contains original non-calibrated beamformed
                        //  data. Flag itself will indicate an error.
                        //  0 = Ok
                        //  1 = No calibration
                        //  2 = TVG read error (R7010)
                        //  3 = CTD not available (R1010)
                        //  4 = Invalid or not available geometry (R7004)
                        //  5 = Invalid sonar specifications (XML)
                        //  6 = Bottom detection failed (R7006)
                        //  7 = No power (Power is set to zero)
                        //  8 = No gain (Gain is too low)
                        //  128-254 = Reserved for internal errors
                        //  255 = System cannot be calibrated (c7k file missing)
  u32 control_flags;  // Control settings from RC 1113 command:
                        //  Bit 0: Brightness is required to pass
                        //  Bit 1: Colinearity is required to pass
                        //  Bit 2: Bottom detection results are used for snippet
                        //  Bit 3: Snippets display min requirements are used
                        //  Bit 4: Minimum window size is required
                        //  Bit 5: Maximum window size is required
                        //  Bit 6-31: reserved
  f32 absorption;     // Absorption value in dB/km. Only valid when
                      //   control flag bit 8 is set
  u32 reserved[6];  // Reserved for future use
  s7k3_snippetbackscatteringstrengthdata
                       snippetbackscatteringstrengthdata[MBSYS_RESON7K_MAX_BEAMS];
  // Snippet time series for each beam
  u32 optionaldata;    // Optional data
  f32 frequency;       // Ping frequency in Hz
  f64 latitude;        // Latitude of vessel reference point in Radians -pi/2 to pi/2,
                        //south negative
  f64 longitude;       // Longitude of vessel reference point in Radians -pi/2 to pi/2,
                        //west negative
  f32 heading;         // Heading of vessel at transmit time in radians
  f32 beam_alongtrack[MBSYS_RESON7K_MAX_BEAMS];   // Along track distance in vessel grid in meters
  f32 beam_acrosstrack[MBSYS_RESON7K_MAX_BEAMS];  // Across track distance in vessel grid in meters
  u32 center_sample[MBSYS_RESON7K_MAX_BEAMS];     // Sample number at detection point of beam
} s7k3_SnippetBackscatteringStrength;

// Reson 7k MB2 Specific Status (Record 7059)
typedef struct s7k3_MB2Status_struct {
  s7k3_header header;
  c8 directory[256];      // Null terminated ASCII string. Maximum of 256 char + null
  c8 header_name[256];    // Null terminated ASCII string. Maximum of 256 char + null
  c8 trailer_name[256];   // Null terminated ASCII string. Maximum of 256 char + null
  u8 prepend_header;      // When not zero; Prepend the file specified by the trailer
                        //   name for each record
  u8 append_trailer;      // When not zero; Append the file specified by the trailer
                        //   name for each record
  u8 storage;             // When not zero; Enable custom recording
  c8 playback_path[256];  // Full directory path name.
                        //   Null terminated ASCII string. Maximum of 256 char + null
  c8 playback_file[256];  // Null terminated ASCII string. Maximum of 256 char + null
  u32 playback_loopmode;  // 0 - Play file once
                        //   1 - Loop the file
                        //   2 - Advanced to next file
  u8 playback;            // When not zero; Enable custom playback
  c8 rrio_address1[256];  // RRIO IP address port, ASCII string, max length 255 char + null
  c8 rrio_address2[256];  // RRIO IP address port, ASCII string, max length 255 char + null
  c8 rrio_address3[256];  // RRIO IP address port, ASCII string, max length 255 char + null
  u8 build_hpr;           // 0 - Use HPR sensor connected to RTA
                        //   1 - Use attached HPR sensor
  u8 attached_hpr;        // 0 - Use SVP sensor connected to RTA
                        //   1 - Use attached SVP sensor
  u8 stacking;            // 0 - Disable stacking mode
                        //   1 - Enable staking mode
  u8 stacking_value;      // Number of results to stack min = 1, max = 9
  u8 zda_baudrate;        // 0 - 4800
                        //   1 - 9600
                        //   2 - 14400
                        //   3 - 19200
                        //   4 - 28800
                        //   5 - 38400
                        //   6 - 56000
                        //   7 - 57600
                        //   8 - 115200
                        //  9+ - Reserved
  u8 zda_parity;          // 0 - None
                        //   1 - Even
                        //   2 - Odd
                        //   3 - Space
                        //   4 - Mark
                        //  5+ - Reserved
  u8 zda_databits;        // 0 - 5 bits
                        //   1 - 6 bits
                        //   2 - 7 bits
                        //   3 - 8 bits
  u8 zda_stopbits;        // 0 - 1 bit
                        //   1 - 2 bits
  u8 gga_baudrate;        // 0 - 4800
                        //   1 - 9600
                        //   2 - 14400
                        //   3 - 19200
                        //   4 - 28800
                        //   5 - 38400
                        //   6 - 56000
                        //   7 - 57600
                        //   8 - 115200
                        //  9+ - Reserved
  u8 gga_parity;          // 0 - None
                        //   1 - Even
                        //   2 - Odd
                        //   3 - Space
                        //   4 - Mark
                        //  5+ - Reserved
  u8 gga_databits;        // 0 - 5 bits
                        //   1 - 6 bits
                        //   2 - 7 bits
                        //   3 - 8 bits
  u8 gga_stopbits;        // 0 - 1 bit
                        //   1 - 2 bits
  u8 svp_baudrate;        // 0 - 4800
                        //   1 - 9600
                        //   2 - 14400
                        //   3 - 19200
                        //   4 - 28800
                        //   5 - 38400
                        //   6 - 56000
                        //   7 - 57600
                        //   8 - 115200
                        //  9+ - Reserved
  u8 svp_parity;          // 0 - None
                        //   1 - Even
                        //   2 - Odd
                        //   3 - Space
                        //   4 - Mark
                        //  5+ - Reserved
  u8 svp_databits;        // 0 - 5 bits
                        //   1 - 6 bits
                        //   2 - 7 bits
                        //   3 - 8 bits
  u8 svp_stopbits;        // 0 - 1 bit
                        //   1 - 2 bits
  u8 hpr_baudrate;        // 0 - 4800
                        //   1 - 9600
                        //   2 - 14400
                        //   3 - 19200
                        //   4 - 28800
                        //   5 - 38400
                        //   6 - 56000
                        //   7 - 57600
                        //   8 - 115200
                        //  9+ - Reserved
  u8 hpr_parity;          // 0 - None
                        //   1 - Even
                        //   2 - Odd
                        //   3 - Space
                        //   4 - Mark
                        //  5+ - Reserved
  u8 hpr_databits;        // 0 - 5 bits
                        //   1 - 6 bits
                        //   2 - 7 bits
                        //   3 - 8 bits
  u8 hpr_stopbits;        // 0 - 1 bit
                        //   1 - 2 bits
  u8 hdt_baudrate;        // 0 - 4800
                        //   1 - 9600
                        //   2 - 14400
                        //   3 - 19200
                        //   4 - 28800
                        //   5 - 38400
                        //   6 - 56000
                        //   7 - 57600
                        //   8 - 115200
                        //  9+ - Reserved
  u8 hdt_parity;          // 0 - None
                        //   1 - Even
                        //   2 - Odd
                        //   3 - Space
                        //   4 - Mark
                        //  5+ - Reserved
  u8 hdt_databits;        // 0 - 5 bits
                        //   1 - 6 bits
                        //   2 - 7 bits
                        //   3 - 8 bits
  u8 hdt_stopbits;        // 0 - 1 bit
                        //   1 - 2 bits
  u16 rrio;                // RRIO port used by SUI
  u8 playback_timestamps; // 0 - Set new timestamps
                        //   1 - Keep original timestamps
  u8 reserved;            // Reserved
  u32 reserved2;          // Reserved
} s7k3_MB2Status;

// Reson 7k subsystem structure (part of Record 7200)
typedef struct s7k3_subsystem_struct {
  u32 device_identifier;   // Identifier for record type of embedded data
  u16 system_enumerator; // Identifier for the device subsystem
} s7k3_subsystem;

// Reson 7k file header (record 7200)
typedef struct s7k3_FileHeader_struct {
  s7k3_header header;
  u64 file_identifier[2];    // File identifier: 0xF3302F43CFB04D6FA93E2AEC33DF577D
  u16 version;               // File format version number
  u16 reserved;              // Reserved
  u64 session_identifier[2]; // User defined session identifier. Used to associate
                        //      multiple files for a given session
  u32 record_data_size;      // Size of record data - 0 if not set
  u32 number_devices;        // Number of devices - N >= 0
  c8 recording_name[64];     // Recording program name - null terminated string
  c8 recording_version[16];  // Recording program version number - null terminated string
  c8 user_defined_name[64];  // User defined name - null terminated string
  c8 notes[128];             // Notes - null terminated string
  s7k3_subsystem subsystem[MBSYS_RESON7K_MAX_DEVICE];
  u32 optionaldata;          // Optional data
  u32 file_catalog_size;     // File catalog record size in bytes
  u64 file_catalog_offset;   // File catalog record offset in bytes from the file beginning
} s7k3_FileHeader;

// Reson 7k File Catalog (part of Record 7300)
typedef struct s7k3_filecatalogdata_struct {
  int sequence;
  double time_d;              // Epoch time used for sorting
  int    pingrecord;          // MB_YES if record has a ping number
  u32 size;                   // Record size in bytes
  u64 offset;                 // File offset
  u16 record_type;            // Record type identifier
  u16 device_id;              // Device identifier
  u16 system_enumerator;      // System enumerator
  s7k3_time s7kTime;          // 7KTIME u8*10 UTC - Time tag indicating when
                              // data was produced.
  u32 record_count;           // Total records in fragmented data record set
  u16 reserved[8];            // Reserved
} s7k3_filecatalogdata;

// Reson 7k File Catalog (Record 7300)
typedef struct s7k3_FileCatalog_struct {
  s7k3_header header;
  u32 size;                   // Size of this record type header
  u16 version;                // 1
  u32 n;                      // Number of records in the file
  u32 nalloc;
  u32 reserved;               // Reserved
  s7k3_filecatalogdata *filecatalogdata;
} s7k3_FileCatalog;

// Reson 7k Time Message (Record 7400)
typedef struct s7k3_TimeMessage_struct {
  s7k3_header header;
  i8 second_offset;   // -1, 0, +1 second for midnight 31 Dec
  u8 pulse_flag;      // 0 - Message is not associated with hardware pulse
                      //  1 - Message preceding hardware pulse
                      //  2 - Message following hardware pulse
  u16 port_id;        // Port number identifier for pulse
  u32 reserved;       // Reserved
  u64 reserved2;      // Reserved
  u32 optionaldata;   // Optional data
  f64 utctime;        // Time since midnight in HHMMSS.SS format
  f64 external_time;  // UTC Time in milliseconds since 1 Jan 1970
  f64 t0;             // T Null Time in milliseconds since 1 Jan 1970
  f64 t1;             // T One Time in milliseconds since 1 Jan 1970
  f64 pulse_length;   // Pulse length in milliseconds
  f64 difference;     // Difference between computer clock and External time in milliseconds
  u16 io_status;      // IO Module synchronization status
} s7k3_TimeMessage;

// Reson 7k Remote Control (Record 7500)
typedef struct s7k3_RemoteControl_struct {
  s7k3_header header;
  u32 remote_id;      // Remote control ID
  u32 ticket;         // Ticket number. Set by client for control packet matching
                      //   ACK or NAK packets
  u64 tracking_n[2];  // Unique number. Set by client for packet tracking
} s7k3_RemoteControl;

// Reson 7k Remote Control Acknowledge (Record 7501)
typedef struct s7k3_RemoteControlAcknowledge_struct {
  s7k3_header header;
  u32 ticket;         // Ticket number in record 7500
  u64 tracking_n[2];  // Unique number in record 7500
} s7k3_RemoteControlAcknowledge;

// Reson 7k Remote Control Not Acknowledge (Record 7502)
typedef struct s7k3_RemoteControlNotAcknowledge_struct {
  s7k3_header header;
  u32 ticket;         // Ticket number in record 7500
  u64 tracking_n[2];  // Unique number in record 7500
  u32 error_code;     // Error code
} s7k3_RemoteControlNotAcknowledge;

// Reson 7k Remote Control Sonar Settings (record 7503)
typedef struct s7k3_RemoteControlSonarSettings_struct {
  s7k3_header header;
  u64 serial_number;            // Sonar serial number
  u32 ping_number;              // Ping number
  f32 frequency;                // Center transmit frequency (in Hertz)
  f32 sample_rate;              // Sample rate (in Hertz)
  f32 receiver_bandwidth;       // Receiver bandwidth (in Hertz)
  f32 tx_pulse_width;           // Transmit pulse length (seconds)
  u32 tx_pulse_type;            // Pulse type identifier:
                                // 0 - CW
                                // 1 - linear chirp
  u32 tx_pulse_envelope;        // Pulse envelope identifier:
                                // 0 - tapered rectangular
                                // 1 - Tukey
  f32 tx_pulse_envelope_par;    // Pulse envelope shading. Some envelopes don't use
                                // this parameter
  u16 tx_pulse_mode;            // 1 - Single ping
                                // 2 - Multi-ping 2
                                // 3 - Multi-ping 3
                                // 4 - Multi-ping 4
  u16 tx_pulse_reserved;        // Reserved
  f32 max_ping_rate;            // Maximum ping rate (pings/second)
  f32 ping_period;              // Time since last ping (seconds)
  f32 range_selection;          // Range selection (meters)
  f32 power_selection;          // Power selection (dB/uPa)
  f32 gain_selection;           // Gain selection (dB)
  u32 control_flags;            // Control flags bitfield:
                                //    Bit 0-3: Auto range method
                                //    Bit 4-7: Auto bottom detect filter method
                                //    Bit 8: Bottom detect range filter enabled
                                //    Bit 9: Bottom detect depth filter enabled
                                //    Bit 10: Receiver gain method Auto Gain
                                //    Bit 11: Receiver gain method Fixed Gain
                                //    Bit 12: Receiver gain method Reserved
                                //    Bit 13: Reserved
                                //    Bit 14: Trigger out HIGH for entire Rx duration
                                //       0 - Disabled
                                //       1 - Enabled
                                //    Bit 15:
                                //       0 - System inactive
                                //       1 - Active
                                //    Bit 16-18: Reserved for bottom detection
                                //    Bit 19: To indicate the adaptive search window
                                //    is active or in passive mode
                                //       0 - Filter active
                                //       1 - Filter passive
                                //    Bit 20: Pipe gating filter
                                //       0 - Disabled
                                //       1 - Enabled
                                //    Bit 21: Adaptive gate depth filter fixed
                                //       0 - Follow seafloor
                                //       1 - Fix depth
                                //    Bit 22: Adaptive gate
                                //       0 - Disabled
                                //       1 - Enabled
                                //    Bit 23: Adaptive gate depth filter
                                //       0 - Disabled
                                //       1 - Enabled
                                //    Bit 24: Trigger Out
                                //       0 - Disabled
                                //       1 - Enabled
                                //    Bit 25: Trigger In Edge
                                //       0 - Positive
                                //       1 - Negative
                                //    Bit 26: PPS Edge
                                //       0 - Positive
                                //       1 - Negative
                                //    Bit 27-28: Timestamp State
                                //       0 - Timestamp not applicable
                                //       1 - Timestamp error / not valid
                                //       2 - Timestamp warning / use caution
                                //       3 - Timestamp ok / valid
                                //    Bit 29: Depth filter follows seaflooor
                                //       0 - Fix depth
                                //       1 - Follow seafloor
                                //    Bit 30: Reduced coverage for constant spacing
                                //       0 - Always maintain swath coverage
                                //       1 - Allow swath coverage to be reduced
                                //    Bit 31:
                                //       0 - 7K
                                //       1 - Simulator
  u32 projector_id;             // Projector selection (identifier)
  f32 steering_vertical;        // Projector steering angle vertical (radians)
  f32 steering_horizontal;      // Projector steering angle horizontal (radians)
  f32 beamwidth_vertical;       // Projector -3 dB beamwidth vertical (radians)
  f32 beamwidth_horizontal;     // Projector -3 dB beamwidth horizontal (radians)
  f32 focal_point;              // Projector focal point (meters)
  u32 projector_weighting;      // Projector beam weighting window type:
                                //    0 - rectangular
                                //    1 - Chebyshev
  f32 projector_weighting_par;  // Projector beam weighting window parameter
  u32 transmit_flags;           // Transmit flags bitfield:
                                //    0-3: pitch stabilization method
                                //    4-7: yaw stabilization method
                                //    8-31: reserved
  u32 hydrophone_id;            // Hydrophone selection (identifier)
  u32 rx_weighting;             // Receiver beam weighting window type:
                                //    0 - Chebyshev
                                //    1 - Kaiser
  f32 rx_weighting_par;         // Receiver beam weighting window parameter
  u32 rx_flags;                 // Receive flags bitfield:
                                //    Bit 0: Roll compensation indicator
                                //    Bit 1: Reserved
                                //    Bit 2: Heave compensation indicator
                                //    Bit 3: Reserved
                                //    Bit 4-7: Dynamic Focusing method
                                //    Bit 8-11: Doppler compensation method
                                //    Bit 12-15: Match filtering method
                                //    Bit 16-19: TVG method
                                //    Bit 20-23: Multi-ping mode
                                //       0 = no multi-ping
                                //      >0 = sequence number of the ping in the multi-ping
                                //       sequence.
                                //    Bit 24-31: Reserved
  f32 range_minimum;           // Bottom detection minimum range (meters) - if range filter active
  f32 range_maximum;           // Bottom detection maximum range (meters) - if range filter active
  f32 depth_minimum;           // Bottom detection minimum depth (meters) - if range filter active
  f32 depth_maximum;           // Bottom detection maximum depth (meters) - if range filter active
  f32 absorption;              // Absorption (in dB/km)
  f32 sound_velocity;          // Sound velocity (meters/second)
  f32 spreading;               // Spreading loss (dB)
  u8 vernier_operation_mode;   // 0 – Vernier
                        //        3 – Triple array Mag & Phase
                                //    (3 sets of records generated for each ping
                                //    – one for each stave)
  u8 autofilter_window;        // Automatic filter window size in percent of the depth
  f32 tx_offset_x;             // Offset of the transducer array in m, relative
                        //        to the receiver array on the x axis, positive
                        //        value is to the right, if the receiver faces
                        //        forward.
  f32 tx_offset_y;             // Offset of the transducer array in m, relative
                        //        to the receiver array on the y axis, positive
                        //        value is forward, if the receiver faces
                        //        forward.
  f32 tx_offset_z;             // Offset of the transducer array in m, relative
                        //        to the receiver array on the z axis, positive
                        //        value is up, if the receiver faces forward.
  f32 head_tilt_x;             // Head tilt x (radians)
  f32 head_tilt_y;             // Head tilt y (radians)
  f32 head_tilt_z;             // Head tilt z (radians)
  u32 ping_state;              // Ping on/off state:
                                //   0 = pinging disabled
                                //   1 = pinging enabled
                                //   2 = External trigger
  u16 beam_angle_mode;         // Beam angle spacing mode:
                                //   1: Equiangle
                                //   2: Eqidistant
                                //   3: Flex
                                //   4: Intermediate
  u16 s7kcenter_mode;          // 7kCenter mode:
                                //   0: Normal
                                //   1: Autopilot
                                //   2: Calibration (IQ)
                                //  3+: Reserved
  f32 gate_depth_min;          // Adaptive gate minimum depth (if filter is active)
  f32 gate_depth_max;          // Adaptive gate maximum depth (if filter is active)
  f64 trigger_width;           // Valid if control bit 24 is set
  f64 trigger_offset;          // Valid if control bit 27 is set
  u16 projector_selection;     // For 81xx series
                                //   0 - Stick
                                //   1 - Main Array
                                //   2 - Extended Range
                                //   3+ - Reserved
  u32 reserved2[2];            // Reserved
  f32 alternate_gain;          // Gain in dB for Method not selected in Control flags
                        //        bits 10 and 11
  u8 vernier_filter;           // Vernier filter settings
  u8 reserved3;                // Reserved
  u16 custom_beams;            // Custom number of beams
  f32 coverage_angle;          // Coverage angle in radians
  u8 coverage_mode;            // 0 = Reduce Spacing
                        //        1 = Reduce Beams
  u8 quality_filter;           // Bit field flags:
                        //        Bit 0:
                                //   0 - quality filter disabled
                                //   1 - quality filter enabled
                        //        Bit 1-7: Reserved, must be zero
  f32 received_steering;       // Horizontal receiver beam steering angle in radians
                        //        (positive to starboard)
  f32 flexmode_coverage;       // Flexmode sector coverage in radians
  f32 flexmode_steering;       // Flexmode steering angle in radians
                        //        (positive to starboard)
  f32 constant_spacing;        // Constant beam spacing on the seafloor in meters
  u16 beam_mode;               // Zero based index number corresponding with the
                        //        available beam modes in the sonar XML
  f32 depth_gate_tilt;         // Angle in radians (positive to starboard)
  f32 applied_frequency;       // Transmit frequency for UI slider. Will be different
                        //        from center frequency in full-rate dual-head
  u32 element_number;               // Selected receive element number for 7049 record
} s7k3_RemoteControlSonarSettings;

// Reson 7k Common System Settings (Record 7504)
typedef struct s7k3_CommonSystemSettings_struct {
  s7k3_header header;
  u64 serial_number;            // Sonar serial number
  u32 ping_number;              // Sequential number
  f32 sound_velocity;           // Sound velocity in m/s
  f32 absorption;               // Absorption in dB/km
  f32 spreading_loss;           // Spreading loss in dB
  u32 sequencer_control;        // 0 - Off
                                // 1 - On
  u8 mru_format;                // 0 - TSS1
                                // 1 - SIMRAD EM1000
                                // 2 - SIMRAD EM3000
                                // 3 - NMEA $PASHR
                                // 4 - OCTANS TAH
                                // 5+ - Reserved
  u8 mru_baudrate;              // 0 - 4800
                                // 1 - 9600
                                // 2 - 14400
                                // 3 - 19200
                                // 4 - 28800
                                // 5 - 38400
                                // 6 - 56000
                                // 7 - 57600
                                // 8 - 115200
                                // 9+ - Reserved
  u8 mru_parity;                // 0 - None
                                // 1 - Even
                                // 2 - Odd
                                // 3 - Space
                                // 4 - Mark
                                // 5+ - Reserved
  u8 mru_databits;              // 0 - 5 bits
                                // 1 - 6 bits
                                // 2 - 7 bits
                                // 3 - 8 bits
                                // 4+ - Reserved
  u8 mru_stopbits;              // 0 - 1 bit
                                // 1 - 2 bits
                                // 2+ - Reserved
  u8 orientation;               // 0 - Port Up
                                // 1 - Port Down
  u8 record_version;            // Record revision number
  f32 motion_latency;           // Motion sensor latency in seconds. Valid range 0 - 0.050
  u8 svp_filter;                // SVP Filter type
                                // 1 - No Filter
                                // 2 - Light Filter
                                // 3 - Normal Filter
                                // 4 - SVP70 Wizard
  u8 sv_override;               // Deprecated, use Sensor Manual Override flags field
  u16 activeenum;               // Enumerator of pinging system
  u32 active_id;                // Device IS of pinging system
  u32 system_mode;              // 0 - Manual (normal) mode
                                // 1 - AutoPilot mode
                                // 2 - I&Q (normalization) mode
                                // 3 - Playback mode
                                // 4+ - Reserved
  u32 masterslave_mode;         // 0 - Normal
                                // 1 - Master full (full range)
                                // 2 - Slave
                                // 3 - Master half. Separate enumeration for 1/2 rate
                                // ("pinp-pong") mode vs. full rate.
  u32 tracker_flags;            // Bit 0: Enable range control
                                // Bit 1: Enable power & gain control
                                // Bit 2: Enable pulse length control
                                // Bit 3: Enable coverage angle control
                                // Bit 4: Use fixed swath width
                                // Bit 5: Set maximum coverage angle
  f32 tracker_swathwidth;       // Tracker swath width in meters
  u16 multidetect_enable;       // Zero - Multi-detect OFF.
                                // Non-Zero = Multi-detect ON.
  u16 multidetect_obsize;       // Range of 1 to 100.
                                // Controls the sensitivity of the algorithm of
                                // Object Size. This is a unit-less quantity.
                                // Note: Increasing this parameter results in more
                                // detections on smaller objects. Decreasing this
                                // parameter results in fewer detections and only on
                                // larger objects.
  u16 multidetect_sensitivity;  // Range of 1 to 100.
                                // Controls the sensitivity of the algorithm to
                                // Amplitude. This is a unit-less quantity.
                                // Note: Increasing this parameter causes more objects
                                // to be detected.
  u16 multidetect_detections;   // Range of 1 to 5.
                                // Limits the number of detections produced per beam.
                                // The maximum number of detections per beam is five.
                                // Note: If there are fewer clusters which are over
                                // the sensitivity threshold than the selected number
                                // of detections, only the number of valid detections
                                // shall be produced, i.e. detections will not be
                                // generated from clusters below the sensitivity
                                // threshold.
  u16 multidetect_reserved[2];  // Reserved. Set to zero.
  u8 slave_ip[4];               // Slave IP V4 Address (big endian data order).
                                // Only valid for record revision number 1 or greater.
  u32 snippet_controlflags;     // Bit Field: (1 - Enabled)
                                // Bit 0: Use automatic snippet window.
                                // Bit 1: Include at least samples around bottom
                                // detection (min. window size is valid).
                                // Bit 2: Include at most samples around bottom
                                // detection (max. window size is valid).
                                // Bit 3-31: Reserved.
  u32 snippet_minwindow;        // Used as Minimum Window Size when bit 0 is set
                                // (automatic window) AND bit 1 is set.
                                // Used as Fixed Window Size when flags bit 0 is NOT
                                // set (fixed window).
  u32 snippet_maxwindow;        // Max snippet window. Used when flags bit 2 is set.
  u32 fullrange_dualhead;       // 1 - Full-rate dual head enabled.
  f32 delay_multiplier;         // Master delay multiplier.
  u8 powersaving_mode;          // 0: None. No power saving enabled at all. Can be used
                                // as reference.
                                // 1: Normal. Components save power when possible.
                                // No effect on operation.
                                // 2: Range Blank. Normal saving + "real" samples are
                                // not output from RX controller until X range is
                                // reached. X is controlled by the Flags and Range
                                // Blank Control fields.
                                // 3: Sleep. Components will be put in sleep mode.
                                // Sleep mode has 0 to 50ms recovery time. All settings
                                // will be retained. NO DATA is produced in sleep mode.
                                // 4: Hibernate. Components will be put in hibernate
                                // mode. Hibernate mode has 0 to 10 second recovery
                                // time. All settings will be retained. NO DATA is
                                // produced in hibernate mode.
  u8 flags;                     // Bits 0-7: Reserved. Zero.
  u16 range_blank;              // Used only for power saving mode 2 ("Range Blank"
                                // mode). Controls size of range blanking interval.
                                // Ping data during this time interval will be zero.
                                // Only affects receiver component. This value gives
                                // range blanking interval as a percent of range
                                // scale (0 - 100). The number of samples blanked
                                // will change with range settings in this case.
  u8 startup_normalization;     // Non-zero to enable normalization at startup.
  u8 restore_pingrate;          // Non-zero to restore ping rate to previous setting
                                // (which was in effect when system was last shut down.
  u8 restore_power;             // Non-zero to restore power to previous setting.
                                // Otherwise system starts with power OFF
  u8 sv_interlock;              // Non-zero to enable Sound Velocity Interlock safety
                                // feature (for system with integrated SV probe only.
  u8 ignorepps_errors;          // Non-zero to suppress error messages due to PPS signal
                                // errors. Proper functioning of the PPS signal is
                                // normally required for accurate data time-stamping
                                // in bathymetry systems.
  u8 reserved1[15];             // Reserved. Zero
  u32 compressed_wcflags;       // Bit field:
                                // Bit 0: Use maximum bottom detection point in each
                                // beam to limit data. Data is included up to the
                                // bottom detection point + 10%. This flag has no effect
                                // on systems which do not perform bottom detection.
                                // Bit 1: Include magnitude data only (strip phase).
                                // Bit 2: Convert mag to dB, then compress from 16 to
                                // 8 bit. Phase compression simply truncates lower
                                // (least significant) byte of phase data.
                                // Bit 3-31: Reserved.
  u8 deckmode;                  // Deck mode. Non-zero: Sonar is in deck mode
  u8 reserved2;                 // Reserved. Filled with 0xFB
  u8 powermode_flags;           // Bit field:
                                // Bit 0: Power mode supported by Control Center
                                // computer
                                // Bit 1: Power mode status CPU throttled
                                // Bit 2: Power mode status AC
  u8 powermode_max;             // Percentage on which the CPU is throttled between
                                // 0 and 100%
  f32 water_temperature;        // Water temperature (in Celsius)
  u8 sensor_override;           // Bit field:
                                // Bit 0: Manual override of sound velocity in effect
                                // Bit 1: Manual override of temperature in effect
  u8 sensor_dataflags;          // Bit field:
                                // Bit 0: Sound velocity sensor data stream detected
                                // Bit 1: Temperature sensor data stream detected
                                // (Persistent - bit set if sensor data stream was
                                // ever seen)
  u8 sensor_active;             // Bit field:
                                // Bit 0: Sound velocity sensor data stream active
                                // Bit 1: Temperature sensor data stream active
                                // (Volatile - bit set if sensor input is active;
                                // timeout period = ≈15 sec.
  u8 reserved3;                 // Reserved. Filled with 0xFB
  f32 tracker_maxcoverage;      // In radians
  u16 dutycycle_mode;           // 0 - Ping rate
                                // 1 - Power level
                                // Bits 3-15 - Reserved
  u16 reserved4;                // Reserved. Filled with 0xFB
  u32 reserved5[99];            // Reserved. Filled with 0xFB
} s7k3_CommonSystemSettings;

// Reson 7k SV Filtering (record 7510)
typedef struct s7k3_SVFiltering_struct {
  s7k3_header header;
  f32 sensor_sv;    // Surface sound velocity reported by sensor. If 'Filter' value is 0
                    //   (no value) or 128 (manual), Sensor SV is set to 0 (zero)
  f32 filtered_sv;  // Filtered sound velocity value used
  u8 filter;        // 0 - No value; no sound velocity value was received by the 7k sonar source
                    //   1 - Transparent; no filtering
                    //   2 - Light filter
                    //   3 - Normal filter
                    //   4 - SVP70 Wizard
                    //   5-217 - Reserved
                    //   128 - Manual input
} s7k3_SVFiltering;

// Reson 7k System Lock Status (record 7511)
typedef struct s7k3_SystemLockStatus_struct {
  s7k3_header header;
  u16 systemlock;
  u32 client_ip;     // IP address of the client that has exclusive control of the system.
                    //  127.0.0.1 (little endian) is reported for local clients (those
                    //  that are running on the same host as 7k sonar source) regardless
                    //  of the type on the connection (TCP or Shared Memory). This field
                    //  is not valid if system is not locked.
  u32 reserved[8];   // Reserved
} s7k3_SystemLockStatus;

// Reson 7k Sound Velocity (record 7610)
typedef struct s7k3_SoundVelocity_struct {
  s7k3_header header;
  f32 soundvelocity; // Water sound speed (m/s)
  u32 optionaldata;  // Flag indicating if optional data are filled in
                    //  // 0 = No
                    //  // 1 = Yes
                    //  This is an internal MB-System flag, not a value in the data format
  f32 temperature;   // Kelvin (optional)
  f32 pressure;      // Pascal (optional)
} s7k3_SoundVelocity;

// Reson 7k Absorption Loss (record 7611)
typedef struct s7k3_AbsorptionLoss_struct {
  s7k3_header header;
  f32 absorptionloss; // Absorption loss (dB/km)
} s7k3_AbsorptionLoss;

// Reson 7k Spreading Loss (record 7612)
typedef struct s7k3_SpreadingLoss_struct {
  s7k3_header header;
  f32 spreadingloss; // dB (0 - 60)
} s7k3_SpreadingLoss;

// internal data structure
struct mbsys_reson7k3_struct {
  // Type of data record
  int kind;   // MB-System record ID
  int type;   // Reson record ID

  // ping record read flags
  int read_ProcessedSideScan;
  int read_SonarSettings;
  int read_MatchFilter;
  int read_BeamGeometry;
  int read_Bathymetry;
  int read_SideScan;
  int read_WaterColumn;
  int read_VerticalDepth;
  int read_TVG;
  int read_Image;
  int read_PingMotion;
  int read_DetectionDataSetup;
  int read_Beamformed;
  int read_VernierProcessingDataRaw;
  int read_RawDetection;
  int read_Snippet;
  int read_VernierProcessingDataFiltered;
  int read_CompressedBeamformedMagnitude;
  int read_CompressedWaterColumn;
  int read_SegmentedRawDetection;
  int read_CalibratedBeam;
  int read_CalibratedSideScan;
  int read_SnippetBackscatteringStrength;
  int read_RemoteControlSonarSettings;

  // MB-System time stamp
  f64 time_d;
  int time_i[7];

  // Reference point information (record 1000)
  //  Note: these offsets should be zero for submersible vehicles
  s7k3_ReferencePoint ReferencePoint;

  // Sensor uncalibrated offset position information (record 1001)
  s7k3_UncalibratedSensorOffset UncalibratedSensorOffset;

  // Sensor calibrated offset position information (record 1002)
  s7k3_CalibratedSensorOffset CalibratedSensorOffset;

  // Position (record 1003)
  s7k3_Position Position;

  // Custom attitude (record 1004)
  s7k3_CustomAttitude CustomAttitude;

  // Tide (record 1005)
  s7k3_Tide Tide;

  // Altitude (record 1006)
  s7k3_Altitude Altitude;

  // Motion over ground (record 1007)
  s7k3_MotionOverGround MotionOverGround;

  // Depth (record 1008)
  s7k3_Depth Depth;

  // Sound velocity profile (record 1009)
  s7k3_SoundVelocityProfile SoundVelocityProfile;

  // CTD (record 1010)
  s7k3_CTD CTD;

  // Geodesy (record 1011)
  s7k3_Geodesy Geodesy;

  // Roll pitch heave (record 1012)
  s7k3_RollPitchHeave RollPitchHeave;

  // Heading (record 1013)
  s7k3_Heading Heading;

  // Survey line (record 1014)
  s7k3_SurveyLine SurveyLine;

  // Navigation (record 1015)
  s7k3_Navigation Navigation;

  // Attitude (record 1016)
  s7k3_Attitude Attitude;

  // Pan Tilt (record 1017)
  s7k3_PanTilt PanTilt;

  // Sonar Installation Identifiers (record 1020)
  s7k3_SonarInstallationIDs SonarInstallationIDs;

  // Mystery (record 1022)
  s7k3_Mystery Mystery;

  // Sonar Pipe Environment (record 2004)
  s7k3_SonarPipeEnvironment SonarPipeEnvironment;

  // Contact Output (record 3001)
  s7k3_ContactOutput ContactOutput;

  // Processed sidescan - MB-System extension to s7k3 format (record 3199)
  s7k3_ProcessedSideScan ProcessedSideScan;

  // Reson 7k sonar settings (record 7000)
  s7k3_SonarSettings SonarSettings;

  // Reson 7k configuration (record 7001)
  s7k3_Configuration Configuration;

  // Reson 7k match filter (record 7002)
  s7k3_MatchFilter MatchFilter;

  // Reson 7k firmware and hardware configuration (record 7003)
  s7k3_FirmwareHardwareConfiguration FirmwareHardwareConfiguration;

  // Reson 7k beam geometry (record 7004)
  s7k3_BeamGeometry BeamGeometry;

  // Reson 7k bathymetry (record 7006)
  s7k3_Bathymetry Bathymetry;

  // Reson 7k Side Scan Data (record 7007)
  s7k3_SideScan SideScan;

  // Reson 7k Generic Water Column data (record 7008)
  s7k3_WaterColumn WaterColumn;

	// Reson 7k Vertical Depth (record 7009)
	s7k3_VerticalDepth VerticalDepth;

  // Reson 7k TVG data (record 7010)
  s7k3_TVG TVG;

  // Reson 7k image data (record 7011)
  s7k3_Image Image;

  // Ping motion (record 7012)
  s7k3_PingMotion PingMotion;

  // Reson 7k Adaptive Gate (record 7014)
  s7k3_AdaptiveGate AdaptiveGate;

  // Detection setup (record 7017)
  s7k3_DetectionDataSetup DetectionDataSetup;

  // Reson 7k Beamformed Data (record 7018)
  s7k3_Beamformed Beamformed;

  // Reson 7k Vernier Processing Data Raw (record 7019)
  s7k3_VernierProcessingDataRaw VernierProcessingDataRaw;

  // Reson 7k BITE (record 7021)
  s7k3_BITE BITE;

  // Reson 7k sonar source version (record 7022)
  s7k3_SonarSourceVersion SonarSourceVersion;

  // Reson 7k 8k wet end version (record 7023)
  s7k3_WetEndVersion8k WetEndVersion8k;

  // Reson 7k raw detection (record 7027)
  s7k3_RawDetection RawDetection;

  // Reson 7k snippet (record 7028)
  s7k3_Snippet Snippet;

  // Reson 7k vernier Processing Data Filtered (Record 7029)
  s7k3_VernierProcessingDataFiltered VernierProcessingDataFiltered;

  // Reson 7k sonar installation parameters (record 7030)
  s7k3_InstallationParameters InstallationParameters;

  // Reson 7k BITE summary (Record 7031)
  s7k3_BITESummary BITESummary;

  // Reson 7k Compressed Beamformed Magnitude Data (Record 7041)
  s7k3_CompressedBeamformedMagnitude CompressedBeamformedMagnitude;

  // Reson 7k Compressed Water Column Data (Record 7042)
  s7k3_CompressedWaterColumn CompressedWaterColumn;

  // Reson 7k Segmented Raw Detection Data (Record 7047)
  s7k3_SegmentedRawDetection SegmentedRawDetection;

  // Reson 7k Calibrated Beam Data (Record 7048)
  s7k3_CalibratedBeam CalibratedBeam;

  // Reson 7k System Events (part of Record 7050)
  s7k3_SystemEvents SystemEvents;

  // Reson 7k system event (record 7051)
  s7k3_SystemEventMessage SystemEventMessage;

  // Reson 7k RDR Recording Status (Record 7052)
  s7k3_RDRRecordingStatus RDRRecordingStatus;

  // Reson 7k Subscriptions (part of Record 7053)
  s7k3_Subscriptions Subscriptions;

  // Reson 7k System Events (Record 7054)
  s7k3_RDRStorageRecording RDRStorageRecording;

  // Reson 7k Calibration Status (Record 7055)
  s7k3_CalibrationStatus CalibrationStatus;

  // Reson 7k Calibrated Sidescan Data (record 7057)
  s7k3_CalibratedSideScan CalibratedSideScan;

  // Reson 7k Snippet Backscattering Strength (Record 7058)
  s7k3_SnippetBackscatteringStrength SnippetBackscatteringStrength;

  // Reson 7k MB2 Specific Status (Record 7059)
  s7k3_MB2Status MB2Status;

  // Reson 7k file header (record 7200)
  s7k3_FileHeader FileHeader;

  // Reson 7k File Catalog (Record 7300)
  s7k3_FileCatalog FileCatalog_read;
  s7k3_FileCatalog FileCatalog_write;

  // Reson 7k Time Message (Record 7400)
  s7k3_TimeMessage TimeMessage;

  // Reson 7k Remote Control (Record 7500)
  s7k3_RemoteControl RemoteControl;

  // Reson 7k Remote Control Acknowledge (Record 7501)
  s7k3_RemoteControlAcknowledge RemoteControlAcknowledge;

  // Reson 7k Remote Control Not Acknowledge (Record 7502)
  s7k3_RemoteControlNotAcknowledge RemoteControlNotAcknowledge;

  // Reson 7k remote control sonar settings (record 7503)
  s7k3_RemoteControlSonarSettings RemoteControlSonarSettings;

  // Reson 7k Common System Settings (Record 7504)
  s7k3_CommonSystemSettings CommonSystemSettings;

  // Reson 7k SV Filtering (record 7510)
  s7k3_SVFiltering SVFiltering;

  // Reson 7k System Lock Status (record 7511)
  s7k3_SystemLockStatus SystemLockStatus;

  // Reson 7k Sound Velocity (record 7610)
  s7k3_SoundVelocity SoundVelocity;

  // Reson 7k Absorption Loss (record 7611)
  s7k3_AbsorptionLoss AbsorptionLoss;

  // Reson 7k Spreading Loss (record 7612)
  s7k3_SpreadingLoss SpreadingLoss;

  // Buffered comments - many MB-System programs output comments before
  // the first data records are passed through to be written
  // - but when writing a file the comments have to be
  // saved until a FileHeader record is written because the size of
  // the FileHeader is unpredictable but it must be the first record
  // in the file.
  int n_saved_comments;
  mb_path comments[MBSYS_RESON7K_MAX_BUFFERED_COMMENTS];

  // record counting variables
  int nrec_read;
  int nrec_write;
  int nrec_ReferencePoint;
  int nrec_UncalibratedSensorOffset;
  int nrec_CalibratedSensorOffset;
  int nrec_Position;
  int nrec_CustomAttitude;
  int nrec_Tide;
  int nrec_Altitude;
  int nrec_MotionOverGround;
  int nrec_Depth;
  int nrec_SoundVelocityProfile;
  int nrec_CTD;
  int nrec_Geodesy;
  int nrec_RollPitchHeave;
  int nrec_Heading;
  int nrec_SurveyLine;
  int nrec_Navigation;
  int nrec_Attitude;
  int nrec_PanTilt;
  int nrec_SonarInstallationIDs;
  int nrec_Mystery;
  int nrec_SonarPipeEnvironment;
  int nrec_ContactOutput;
  int nrec_ProcessedSideScan;
  int nrec_SonarSettings;
  int nrec_Configuration;
  int nrec_MatchFilter;
  int nrec_FirmwareHardwareConfiguration;
  int nrec_BeamGeometry;
  int nrec_Bathymetry;
  int nrec_SideScan;
  int nrec_WaterColumn;
  int nrec_VerticalDepth;
  int nrec_TVG;
  int nrec_Image;
  int nrec_PingMotion;
  int nrec_AdaptiveGate;
  int nrec_DetectionDataSetup;
  int nrec_Beamformed;
  int nrec_VernierProcessingDataRaw;
  int nrec_BITE;
  int nrec_SonarSourceVersion;
  int nrec_WetEndVersion8k;
  int nrec_RawDetection;
  int nrec_Snippet;
  int nrec_VernierProcessingDataFiltered;
  int nrec_InstallationParameters;
  int nrec_BITESummary;
  int nrec_CompressedBeamformedMagnitude;
  int nrec_CompressedWaterColumn;
  int nrec_SegmentedRawDetection;
  int nrec_CalibratedBeam;
  int nrec_SystemEvents;
  int nrec_SystemEventMessage;
  int nrec_RDRRecordingStatus;
  int nrec_Subscriptions;
  int nrec_RDRStorageRecording;
  int nrec_CalibrationStatus;
  int nrec_CalibratedSideScan;
  int nrec_SnippetBackscatteringStrength;
  int nrec_MB2Status;
  int nrec_FileHeader;
  int nrec_FileCatalog;
  int nrec_TimeMessage;
  int nrec_RemoteControl;
  int nrec_RemoteControlAcknowledge;
  int nrec_RemoteControlNotAcknowledge;
  int nrec_RemoteControlSonarSettings;
  int nrec_CommonSystemSettings;
  int nrec_SVFiltering;
  int nrec_SystemLockStatus;
  int nrec_SoundVelocity;
  int nrec_AbsorptionLoss;
  int nrec_SpreadingLoss;
};

// 7K Macros
int mbsys_reson7k3_checkheader(s7k3_header header);

// system specific function prototypes
int mbsys_reson7k3_zero7kheader(int verbose, s7k3_header *header, int *error);
int mbsys_reson7k3_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_reson7k3_survey_alloc(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbsys_reson7k3_attitude_alloc(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbsys_reson7k3_heading_alloc(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbsys_reson7k3_ssv_alloc(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbsys_reson7k3_tlt_alloc(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbsys_reson7k3_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_reson7k3_zero_ss(int verbose, void *store_ptr, int *error);
int mbsys_reson7k3_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_reson7k3_pingnumber(int verbose, void *mbio_ptr, unsigned int *pingnumber, int *error);
int mbsys_reson7k3_sonartype(int verbose, void *mbio_ptr, void *store_ptr, int *sonartype, int *error);
int mbsys_reson7k3_sidescantype(int verbose, void *mbio_ptr, void *store_ptr, int *ss_type, int *error);
int mbsys_reson7k3_preprocess(int verbose, void *mbio_ptr, void *store_ptr, void *platform_ptr, void *preprocess_pars_ptr, int *error);
int mbsys_reson7k3_extract_platform(int verbose, void *mbio_ptr, void *store_ptr, int *kind, void **platform_ptr, int *error);
int mbsys_reson7k3_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                          double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss, char *beamflag,
                          double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss,
                          double *ssacrosstrack, double *ssalongtrack, char *comment, int *error);
int mbsys_reson7k3_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
                         double navlat, double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath,
                         double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
                         double *ssalongtrack, char *comment, int *error);
int mbsys_reson7k3_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
                         double *angles_forward, double *angles_null, double *heave, double *alongtrack_offset, double *draft,
                         double *ssv, int *error);
int mbsys_reson7k3_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error);
int mbsys_reson7k3_gains(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transmit_gain, double *pulse_length,
                        double *receive_gain, int *error);
int mbsys_reson7k3_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                                   double *altitude, int *error);
int mbsys_reson7k3_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                              double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll,
                              double *pitch, double *heave, int *error);
int mbsys_reson7k3_extract_nnav(int verbose, void *mbio_ptr, void *store_ptr, int nmax, int *kind, int *n, int *time_i,
                               double *time_d, double *navlon, double *navlat, double *speed, double *heading, double *draft,
                               double *roll, double *pitch, double *heave, int *error);
int mbsys_reson7k3_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
                             double navlat, double speed, double heading, double draft, double roll, double pitch, double heave,
                             int *error);
int mbsys_reson7k3_extract_svp(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsvp, double *depth, double *velocity,
                              int *error);
int mbsys_reson7k3_insert_svp(int verbose, void *mbio_ptr, void *store_ptr, int nsvp, double *depth, double *velocity, int *error);
int mbsys_reson7k3_extract_segytraceheader(int verbose, void *mbio_ptr, void *store_ptr, int *kind, void *segyheader_ptr,
                                          int *error);
int mbsys_reson7k3_extract_segy(int verbose, void *mbio_ptr, void *store_ptr, int *sampleformat, int *kind, void *segyheader_ptr,
                               float *segydata, int *error);
int mbsys_reson7k3_insert_segy(int verbose, void *mbio_ptr, void *store_ptr, int kind, void *segyheader_ptr, float *segydata,
                              int *error);
int mbsys_reson7k3_ctd(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nctd, double *time_d, double *conductivity,
                      double *temperature, double *depth, double *salinity, double *soundspeed, int *error);
int mbsys_reson7k3_ancilliarysensor(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsamples, double *time_d,
                                   double *sensor1, double *sensor2, double *sensor3, double *sensor4, double *sensor5,
                                   double *sensor6, double *sensor7, double *sensor8, int *error);
int mbsys_reson7k3_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error);
int mbsys_reson7k3_checkheader(s7k3_header header);
int mbsys_reson7k3_makess(int verbose, void *mbio_ptr, void *store_ptr, int source, int pixel_size_set, double *pixel_size,
                         int swath_width_set, double *swath_width, int pixel_int, int *error);

int mbsys_reson7k3_print_header(int verbose, s7k3_header *header, int *error);
int mbsys_reson7k3_print_ReferencePoint(int verbose, s7k3_ReferencePoint *ReferencePoint, int *error);
int mbsys_reson7k3_print_UncalibratedSensorOffset(int verbose, s7k3_UncalibratedSensorOffset *UncalibratedSensorOffset, int *error);
int mbsys_reson7k3_print_CalibratedSensorOffset(int verbose, s7k3_CalibratedSensorOffset *CalibratedSensorOffset, int *error);
int mbsys_reson7k3_print_Position(int verbose, s7k3_Position *Position, int *error);
int mbsys_reson7k3_print_CustomAttitude(int verbose, s7k3_CustomAttitude *CustomAttitude, int *error);
int mbsys_reson7k3_print_Tide(int verbose, s7k3_Tide *Tide, int *error);
int mbsys_reson7k3_print_Altitude(int verbose, s7k3_Altitude *Altitude, int *error);
int mbsys_reson7k3_print_MotionOverGround(int verbose, s7k3_MotionOverGround *MotionOverGround, int *error);
int mbsys_reson7k3_print_Depth(int verbose, s7k3_Depth *Depth, int *error);
int mbsys_reson7k3_print_SoundVelocityProfile(int verbose, s7k3_SoundVelocityProfile *SoundVelocityProfile, int *error);
int mbsys_reson7k3_print_CTD(int verbose, s7k3_CTD *CTD, int *error);
int mbsys_reson7k3_print_Geodesy(int verbose, s7k3_Geodesy *Geodesy, int *error);
int mbsys_reson7k3_print_RollPitchHeave(int verbose, s7k3_RollPitchHeave *RollPitchHeave, int *error);
int mbsys_reson7k3_print_Heading(int verbose, s7k3_Heading *Heading, int *error);
int mbsys_reson7k3_print_SurveyLine(int verbose, s7k3_SurveyLine *SurveyLine, int *error);
int mbsys_reson7k3_print_Navigation(int verbose, s7k3_Navigation *Navigation, int *error);
int mbsys_reson7k3_print_Attitude(int verbose, s7k3_Attitude *Attitude, int *error);
int mbsys_reson7k3_print_PanTilt(int verbose, s7k3_PanTilt *PanTilt, int *error);
int mbsys_reson7k3_print_SonarInstallationIDs(int verbose, s7k3_SonarInstallationIDs *SonarInstallationIDs, int *error);
int mbsys_reson7k3_print_Mystery(int verbose, s7k3_Mystery *Mystery, int *error);
int mbsys_reson7k3_print_SonarPipeEnvironment(int verbose, s7k3_SonarPipeEnvironment *SonarPipeEnvironment, int *error);
int mbsys_reson7k3_print_ContactOutput(int verbose, s7k3_ContactOutput *ContactOutput, int *error);
int mbsys_reson7k3_print_ProcessedSideScan(int verbose, s7k3_ProcessedSideScan *ProcessedSideScan, int *error);
int mbsys_reson7k3_print_SonarSettings(int verbose, s7k3_SonarSettings *SonarSettings, int *error);
int mbsys_reson7k3_print_Configuration(int verbose, s7k3_Configuration *Configuration, int *error);
int mbsys_reson7k3_print_MatchFilter(int verbose, s7k3_MatchFilter *MatchFilter, int *error);
int mbsys_reson7k3_print_FirmwareHardwareConfiguration(int verbose, s7k3_FirmwareHardwareConfiguration *FirmwareHardwareConfiguration, int *error);
int mbsys_reson7k3_print_BeamGeometry(int verbose, s7k3_BeamGeometry *BeamGeometry, int *error);
int mbsys_reson7k3_print_Bathymetry(int verbose, s7k3_Bathymetry *Bathymetry, int *error);
int mbsys_reson7k3_print_SideScan(int verbose, s7k3_SideScan *SideScan, int *error);
int mbsys_reson7k3_print_WaterColumn(int verbose, s7k3_WaterColumn *WaterColumn, int *error);
int mbsys_reson7k3_print_VerticalDepth(int verbose, s7k3_VerticalDepth *VerticalDepth, int *error);
int mbsys_reson7k3_print_TVG(int verbose, s7k3_TVG *TVG, int *error);
int mbsys_reson7k3_print_Image(int verbose, s7k3_Image *Image, int *error);
int mbsys_reson7k3_print_PingMotion(int verbose, s7k3_PingMotion *PingMotion, int *error);
int mbsys_reson7k3_print_AdaptiveGate(int verbose, s7k3_AdaptiveGate *AdaptiveGate, int *error);
int mbsys_reson7k3_print_DetectionDataSetup(int verbose, s7k3_DetectionDataSetup *DetectionDataSetup, int *error);
int mbsys_reson7k3_print_Beamformed(int verbose, s7k3_Beamformed *Beamformed, int *error);
int mbsys_reson7k3_print_VernierProcessingDataRaw(int verbose, s7k3_VernierProcessingDataRaw *VernierProcessingDataRaw, int *error);
int mbsys_reson7k3_print_BITE(int verbose, s7k3_BITE *BITE, int *error);
int mbsys_reson7k3_print_SonarSourceVersion(int verbose, s7k3_SonarSourceVersion *SonarSourceVersion, int *error);
int mbsys_reson7k3_print_WetEndVersion8k(int verbose, s7k3_WetEndVersion8k *WetEndVersion8k, int *error);
int mbsys_reson7k3_print_RawDetection(int verbose, s7k3_RawDetection *RawDetection, int *error);
int mbsys_reson7k3_print_Snippet(int verbose, s7k3_Snippet *Snippet, int *error);
int mbsys_reson7k3_print_VernierProcessingDataFiltered(int verbose, s7k3_VernierProcessingDataFiltered *VernierProcessingDataFiltered, int *error);
int mbsys_reson7k3_print_InstallationParameters(int verbose, s7k3_InstallationParameters *InstallationParameters, int *error);
int mbsys_reson7k3_print_BITESummary(int verbose, s7k3_BITESummary *BITESummary, int *error);
int mbsys_reson7k3_print_CompressedBeamformedMagnitude(int verbose, s7k3_CompressedBeamformedMagnitude *CompressedBeamformedMagnitude, int *error);
int mbsys_reson7k3_print_CompressedWaterColumn(int verbose, s7k3_CompressedWaterColumn *CompressedWaterColumn, int *error);
int mbsys_reson7k3_print_SegmentedRawDetection(int verbose, s7k3_SegmentedRawDetection *SegmentedRawDetection, int *error);
int mbsys_reson7k3_print_CalibratedBeam(int verbose, s7k3_CalibratedBeam *CalibratedBeam, int *error);
int mbsys_reson7k3_print_SystemEvents(int verbose, s7k3_SystemEvents *SystemEvents, int *error);
int mbsys_reson7k3_print_SystemEventMessage(int verbose, s7k3_SystemEventMessage *SystemEventMessage, int *error);
int mbsys_reson7k3_print_RDRRecordingStatus(int verbose, s7k3_RDRRecordingStatus *RDRRecordingStatus, int *error);
int mbsys_reson7k3_print_Subscriptions(int verbose, s7k3_Subscriptions *Subscriptions, int *error);
int mbsys_reson7k3_print_RDRStorageRecording(int verbose, s7k3_RDRStorageRecording *RDRStorageRecording, int *error);
int mbsys_reson7k3_print_CalibrationStatus(int verbose, s7k3_CalibrationStatus *CalibrationStatus, int *error);
int mbsys_reson7k3_print_CalibratedSideScan(int verbose, s7k3_CalibratedSideScan *CalibratedSideScan, int *error);
int mbsys_reson7k3_print_SnippetBackscatteringStrength(int verbose, s7k3_SnippetBackscatteringStrength *SnippetBackscatteringStrength, int *error);
int mbsys_reson7k3_print_MB2Status(int verbose, s7k3_MB2Status *MB2Status, int *error);
int mbsys_reson7k3_print_FileHeader(int verbose, s7k3_FileHeader *FileHeader, int *error);
int mbsys_reson7k3_print_FileCatalog(int verbose, s7k3_FileCatalog *FileCatalog, int *error);
int mbsys_reson7k3_print_TimeMessage(int verbose, s7k3_TimeMessage *TimeMessage, int *error);
int mbsys_reson7k3_print_RemoteControl(int verbose, s7k3_RemoteControl *RemoteControl, int *error);
int mbsys_reson7k3_print_RemoteControlAcknowledge(int verbose, s7k3_RemoteControlAcknowledge *RemoteControlAcknowledge, int *error);
int mbsys_reson7k3_print_RemoteControlNotAcknowledge(int verbose, s7k3_RemoteControlNotAcknowledge *RemoteControlNotAcknowledge, int *error);
int mbsys_reson7k3_print_RemoteControlSonarSettings(int verbose, s7k3_RemoteControlSonarSettings *RemoteControlSonarSettings, int *error);
int mbsys_reson7k3_print_CommonSystemSettings(int verbose, s7k3_CommonSystemSettings *CommonSystemSettings, int *error);
int mbsys_reson7k3_print_SVFiltering(int verbose, s7k3_SVFiltering *SVFiltering, int *error);
int mbsys_reson7k3_print_SystemLockStatus(int verbose, s7k3_SystemLockStatus *SystemLockStatus, int *error);
int mbsys_reson7k3_print_SoundVelocity(int verbose, s7k3_SoundVelocity *SoundVelocity, int *error);
int mbsys_reson7k3_print_AbsorptionLoss(int verbose, s7k3_AbsorptionLoss *AbsorptionLoss, int *error);
int mbsys_reson7k3_print_SpreadingLoss(int verbose, s7k3_SpreadingLoss *SpreadingLoss, int *error);

#endif  //  MBSYS_RESON7K3_H_
