/*--------------------------------------------------------------------
 *    The MB-system:  mb_io.h  1/19/93
 *
 *    Copyright (c) 1993-2020 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/**
@file
 * mb_io.h defines data structures used by MBIO "mb_" functions
 * to store parameters relating to reading data from or writing
 * data to a single multibeam data file.
 *
 * Author:  D. W. Caress
 * Date:  January 19, 1993
 *
 */

#ifndef MB_IO_H_
#define MB_IO_H_

#include "mb_define.h"
#include "mb_status.h"
#include "mb_process.h"

/* ---------------------------------------------------------------------------*/
/* Survey Platform definitions and structures for the
 * mb_platform_*() functions */

/* survey platform type defines */
typedef enum {
    MB_PLATFORM_NONE,
    MB_PLATFORM_SURFACE_VESSEL,
    MB_PLATFORM_TOW_BODY,
    MB_PLATFORM_ROV,
    MB_PLATFORM_AUV,
    MB_PLATFORM_AIRCRAFT,
    MB_PLATFORM_SATELLITE,
    MB_PLATFORM_MOORING,
    MB_PLATFORM_FIXED,
} mb_platform_enum;

#ifdef __cplusplus
extern "C" {
#endif
const char *mb_platform_type(mb_platform_enum platform);
#ifdef __cplusplus
}  /* extern "C" */
#endif

/* survey platform data source sensor defines */
#define MB_PLATFORM_SOURCE_NONE 0
#define MB_PLATFORM_SOURCE_BATHYMETRY 1
#define MB_PLATFORM_SOURCE_BATHYMETRY1 2
#define MB_PLATFORM_SOURCE_BATHYMETRY2 3
#define MB_PLATFORM_SOURCE_BATHYMETRY3 4
#define MB_PLATFORM_SOURCE_BACKSCATTER 5
#define MB_PLATFORM_SOURCE_BACKSCATTER1 6
#define MB_PLATFORM_SOURCE_BACKSCATTER2 7
#define MB_PLATFORM_SOURCE_BACKSCATTER3 8
#define MB_PLATFORM_SOURCE_SUBBOTTOM 9
#define MB_PLATFORM_SOURCE_SUBBOTTOM1 10
#define MB_PLATFORM_SOURCE_SUBBOTTOM2 11
#define MB_PLATFORM_SOURCE_SUBBOTTOM3 12
#define MB_PLATFORM_SOURCE_POSITION 13
#define MB_PLATFORM_SOURCE_POSITION1 14
#define MB_PLATFORM_SOURCE_POSITION2 15
#define MB_PLATFORM_SOURCE_POSITION3 16
#define MB_PLATFORM_SOURCE_DEPTH 17
#define MB_PLATFORM_SOURCE_DEPTH1 18
#define MB_PLATFORM_SOURCE_DEPTH2 19
#define MB_PLATFORM_SOURCE_DEPTH3 20
#define MB_PLATFORM_SOURCE_HEADING 21
#define MB_PLATFORM_SOURCE_HEADING1 22
#define MB_PLATFORM_SOURCE_HEADING2 23
#define MB_PLATFORM_SOURCE_HEADING3 24
#define MB_PLATFORM_SOURCE_ROLLPITCH 25
#define MB_PLATFORM_SOURCE_ROLLPITCH1 26
#define MB_PLATFORM_SOURCE_ROLLPITCH2 27
#define MB_PLATFORM_SOURCE_ROLLPITCH3 28
#define MB_PLATFORM_SOURCE_HEAVE 29
#define MB_PLATFORM_SOURCE_HEAVE1 30
#define MB_PLATFORM_SOURCE_HEAVE2 31
#define MB_PLATFORM_SOURCE_HEAVE3 32

/* survey sensor time latency defines */
#define MB_SENSOR_TIME_LATENCY_NONE 0
#define MB_SENSOR_TIME_LATENCY_STATIC 1
#define MB_SENSOR_TIME_LATENCY_MODEL 2
#define MB_SENSOR_POSITION_OFFSET_NONE 0
#define MB_SENSOR_POSITION_OFFSET_STATIC 1
#define MB_SENSOR_ATTITUDE_OFFSET_NONE 0
#define MB_SENSOR_ATTITUDE_OFFSET_STATIC 1

/* survey platform sensor type defines */
#define NUM_MB_SENSOR_TYPES 22
#define MB_SENSOR_TYPE_NONE 0
#define MB_SENSOR_TYPE_SONAR_ECHOSOUNDER 10
#define MB_SENSOR_TYPE_SONAR_MULTIECHOSOUNDER 11
#define MB_SENSOR_TYPE_SONAR_SIDESCAN 20
#define MB_SENSOR_TYPE_SONAR_INTERFEROMETRY 21
#define MB_SENSOR_TYPE_SONAR_MULTIBEAM 30
#define MB_SENSOR_TYPE_SONAR_MULTIBEAM_TWOHEAD 31
#define MB_SENSOR_TYPE_SONAR_SUBBOTTOM 40
#define MB_SENSOR_TYPE_CAMERA_MONO 50
#define MB_SENSOR_TYPE_CAMERA_STEREO 51
#define MB_SENSOR_TYPE_CAMERA_VIDEO 52
#define MB_SENSOR_TYPE_LIDAR_SCAN 60
#define MB_SENSOR_TYPE_LIDAR_SWATH 61
#define MB_SENSOR_TYPE_POSITION 70
#define MB_SENSOR_TYPE_COMPASS 80
#define MB_SENSOR_TYPE_VRU 90
#define MB_SENSOR_TYPE_IMU 100
#define MB_SENSOR_TYPE_INS 101
#define MB_SENSOR_TYPE_INS_WITH_PRESSURE 102
#define MB_SENSOR_TYPE_CTD 110
#define MB_SENSOR_TYPE_PRESSURE 111
#define MB_SENSOR_TYPE_SOUNDSPEED 120

#ifdef MB_NEED_SENSOR_TYPE
// TODO(schwehr): Convert these from static header variables to
// an accessor function.
static int mb_sensor_type_id[] = {
    MB_SENSOR_TYPE_NONE,                    // 0
    MB_SENSOR_TYPE_SONAR_ECHOSOUNDER,       // 10
    MB_SENSOR_TYPE_SONAR_MULTIECHOSOUNDER,  // 11
    MB_SENSOR_TYPE_SONAR_SIDESCAN,          // 20
    MB_SENSOR_TYPE_SONAR_INTERFEROMETRY,    // 21
    MB_SENSOR_TYPE_SONAR_MULTIBEAM,         // 30
    MB_SENSOR_TYPE_SONAR_MULTIBEAM_TWOHEAD, // 31
    MB_SENSOR_TYPE_SONAR_SUBBOTTOM,         // 40
    MB_SENSOR_TYPE_CAMERA_MONO,             // 50
    MB_SENSOR_TYPE_CAMERA_STEREO,           // 51
    MB_SENSOR_TYPE_CAMERA_VIDEO,            // 52
    MB_SENSOR_TYPE_LIDAR_SCAN,              // 60
    MB_SENSOR_TYPE_LIDAR_SWATH,             // 61
    MB_SENSOR_TYPE_POSITION,                // 70
    MB_SENSOR_TYPE_COMPASS,                 // 80
    MB_SENSOR_TYPE_VRU,                     // 90
    MB_SENSOR_TYPE_IMU,                     // 100
    MB_SENSOR_TYPE_INS,                     // 101
    MB_SENSOR_TYPE_INS_WITH_PRESSURE,       // 102
    MB_SENSOR_TYPE_CTD,                     // 110
    MB_SENSOR_TYPE_PRESSURE,                // 111
    MB_SENSOR_TYPE_SOUNDSPEED,              // 120
};
static const char *mb_sensor_type_string[] = {"Unknown sensor type",
                                        "Sonar echosounder",
                                        "Sonar multiechosounder",
                                        "Sonar sidescan",
                                        "Sonar interferometry",
                                        "Sonar multibeam",
                                        "Sonar multibeam twohead",
                                        "Sonar subbottom",
                                        "Camera mono",
                                        "Camera stereo",
                                        "Camera video",
                                        "Lidar scan",
                                        "Lidar swath",
                                        "Position",
                                        "Compass",
                                        "VRU",
                                        "IMU",
                                        "INS",
                                        "INS with pressure",
                                        "CTD",
                                        "Pressure",
                                        "Soundspeed"};
#endif  // MB_NEED_SENSOR_TYPE

/* survey platform sensor capability bitmask defines */
#define MB_SENSOR_CAPABILITY1_NONE 0x00000000          // All bits = 0
#define MB_SENSOR_CAPABILITY1_POSITION 0x00000001      // Bit 0 = 1
#define MB_SENSOR_CAPABILITY1_DEPTH 0x00000002         // Bit 1 = 2
#define MB_SENSOR_CAPABILITY1_ALTITUDE 0x00000004      // Bit 2 = 4
#define MB_SENSOR_CAPABILITY1_VELOCITY 0x00000008      // Bit 3 = 8
#define MB_SENSOR_CAPABILITY1_ACCELERATION 0x00000010  // Bit 4 = 16
#define MB_SENSOR_CAPABILITY1_PRESSURE 0x00000020      // Bit 5 = 32
#define MB_SENSOR_CAPABILITY1_ROLLPITCH 0x00000040     // Bit 6 = 64
#define MB_SENSOR_CAPABILITY1_HEADING 0x00000080       // Bit 7 = 128
#define MB_SENSOR_CAPABILITY1_HEAVE 0x00000100         // Bit 8 = 256
#define MB_SENSOR_CAPABILITY1_UNUSED09 0x00000200      // Bit 9 = 512
#define MB_SENSOR_CAPABILITY1_UNUSED10 0x00000400      // Bit 10 = 1024
#define MB_SENSOR_CAPABILITY1_UNUSED11 0x00000800      // Bit 11 = 2048
#define MB_SENSOR_CAPABILITY1_UNUSED12 0x00001000      // Bit 12 = 4096
#define MB_SENSOR_CAPABILITY1_TEMPERATURE 0x00002000   // Bit 13 = 8192
#define MB_SENSOR_CAPABILITY1_CONDUCTIVITY 0x00004000  // Bit 14 = 16384
#define MB_SENSOR_CAPABILITY1_SALINITY 0x00008000      // Bit 15 = 32768
#define MB_SENSOR_CAPABILITY1_SOUNDSPEED 0x00010000    // Bit 16 = 65536
#define MB_SENSOR_CAPABILITY1_UNUSED17 0x00020000      // Bit 17= 131072
#define MB_SENSOR_CAPABILITY1_UNUSED18 0x00040000      // Bit 18 = 262144
#define MB_SENSOR_CAPABILITY1_UNUSED19 0x00080000      // Bit 19 = 524288
#define MB_SENSOR_CAPABILITY1_GRAVITY 0x00100000       // Bit 20 = 1048576
#define MB_SENSOR_CAPABILITY1_UNUSED21 0x00200000      // Bit 21 = 2097152
#define MB_SENSOR_CAPABILITY1_UNUSED22 0x00400000      // Bit 22 = 4194304
#define MB_SENSOR_CAPABILITY1_UNUSED23 0x00800000      // Bit 23 = 8388608
#define MB_SENSOR_CAPABILITY1_MAGNETICFIELD 0x01000000 // Bit 24 = 16777216
#define MB_SENSOR_CAPABILITY1_UNUSED25 0x02000000      // Bit 25 = 33554432
#define MB_SENSOR_CAPABILITY1_UNUSED26 0x04000000      // Bit 26 = 67108864
#define MB_SENSOR_CAPABILITY1_UNUSED27 0x08000000      // Bit 27 = 134217728
#define MB_SENSOR_CAPABILITY1_UNUSED28 0x10000000      // Bit 28 = 268435456
#define MB_SENSOR_CAPABILITY1_UNUSED29 0x20000000      // Bit 29 = 536870912
#define MB_SENSOR_CAPABILITY1_UNUSED30 0x40000000      // Bit 30 = 1073741824
#define MB_SENSOR_CAPABILITY1_UNUSED31 0x80000000      // Bit 31 = 2147483648
#define mb_check_sensor_capability1_position(F) ((int)(F & MB_SENSOR_CAPABILITY1_POSITION))
#define mb_check_sensor_capability1_depth(F) ((int)(F & MB_SENSOR_CAPABILITY1_DEPTH))
#define mb_check_sensor_capability1_altitude(F) ((int)(F & MB_SENSOR_CAPABILITY1_ALTITUDE))
#define mb_check_sensor_capability1_velocity(F) ((int)(F & MB_SENSOR_CAPABILITY1_VELOCITY))
#define mb_check_sensor_capability1_acceleration(F) ((int)(F & MB_SENSOR_CAPABILITY1_ACCELERATION))
#define mb_check_sensor_capability1_pressure(F) ((int)(F & MB_SENSOR_CAPABILITY1_PRESSURE))
#define mb_check_sensor_capability1_rollpitch(F) ((int)(F & MB_SENSOR_CAPABILITY1_ROLLPITCH))
#define mb_check_sensor_capability1_heading(F) ((int)(F & MB_SENSOR_CAPABILITY1_HEADING))
#define mb_check_sensor_capability1_heading(F) ((int)(F & MB_SENSOR_CAPABILITY1_HEADING))
#define mb_check_sensor_capability1_unused09(F) ((int)(F & MB_SENSOR_CAPABILITY1_UNUSED09))
#define mb_check_sensor_capability1_unused10(F) ((int)(F & MB_SENSOR_CAPABILITY1_UNUSED10))
#define mb_check_sensor_capability1_unused11(F) ((int)(F & MB_SENSOR_CAPABILITY1_UNUSED11))
#define mb_check_sensor_capability1_unused12(F) ((int)(F & MB_SENSOR_CAPABILITY1_UNUSED12))
#define mb_check_sensor_capability1_temperature(F) ((int)(F & MB_SENSOR_CAPABILITY1_TEMPERATURE))
#define mb_check_sensor_capability1_conductivity(F) ((int)(F & MB_SENSOR_CAPABILITY1_CONDUCTIVITY))
#define mb_check_sensor_capability1_salinity(F) ((int)(F & MB_SENSOR_CAPABILITY1_SALINITY))
#define mb_check_sensor_capability1_soundspeed(F) ((int)(F & MB_SENSOR_CAPABILITY1_SOUNDSPEED))
#define mb_check_sensor_capability1_unused17(F) ((int)(F & MB_SENSOR_CAPABILITY1_UNUSED17))
#define mb_check_sensor_capability1_unused18(F) ((int)(F & MB_SENSOR_CAPABILITY1_UNUSED18))
#define mb_check_sensor_capability1_unused19(F) ((int)(F & MB_SENSOR_CAPABILITY1_UNUSED19))
#define mb_check_sensor_capability1_gravity(F) ((int)(F & MB_SENSOR_CAPABILITY1_GRAVITY))
#define mb_check_sensor_capability1_unused21(F) ((int)(F & MB_SENSOR_CAPABILITY1_UNUSED21))
#define mb_check_sensor_capability1_unused22(F) ((int)(F & MB_SENSOR_CAPABILITY1_UNUSED22))
#define mb_check_sensor_capability1_unused23(F) ((int)(F & MB_SENSOR_CAPABILITY1_UNUSED23))
#define mb_check_sensor_capability1_magneticfield(F) ((int)(F & MB_SENSOR_CAPABILITY1_MAGNETICFIELD))
#define mb_check_sensor_capability1_unused25(F) ((int)(F & MB_SENSOR_CAPABILITY1_UNUSED25))
#define mb_check_sensor_capability1_unused26(F) ((int)(F & MB_SENSOR_CAPABILITY1_UNUSED26))
#define mb_check_sensor_capability1_unused27(F) ((int)(F & MB_SENSOR_CAPABILITY1_UNUSED27))
#define mb_check_sensor_capability1_unused28(F) ((int)(F & MB_SENSOR_CAPABILITY1_UNUSED28))
#define mb_check_sensor_capability1_unused29(F) ((int)(F & MB_SENSOR_CAPABILITY1_UNUSED29))
#define mb_check_sensor_capability1_unused30(F) ((int)(F & MB_SENSOR_CAPABILITY1_UNUSED30))
#define mb_check_sensor_capability1_unused31(F) ((int)(F & MB_SENSOR_CAPABILITY1_UNUSED31))

#define MB_SENSOR_CAPABILITY2_NONE 0x00000000                           // All bits = 0
#define MB_SENSOR_CAPABILITY2_TOPOGRAPHY_ECHOSOUNDER 0x00000001         // Bit 0 = 1
#define MB_SENSOR_CAPABILITY2_TOPOGRAPHY_INTERFEROMETRY 0x00000002      // Bit 1 = 2
#define MB_SENSOR_CAPABILITY2_TOPOGRAPHY_SASS 0x00000004                // Bit 2 = 4
#define MB_SENSOR_CAPABILITY2_TOPOGRAPHY_MULTIBEAM 0x00000008           // Bit 3 = 8
#define MB_SENSOR_CAPABILITY2_TOPOGRAPHY_PHOTOGRAMMETRY 0x00000010      // Bit 4 = 16
#define MB_SENSOR_CAPABILITY2_TOPOGRAPHY_STRUCTUREFROMMOTION 0x00000020 // Bit 5 = 32
#define MB_SENSOR_CAPABILITY2_TOPOGRAPHY_LIDAR 0x00000040               // Bit 6 = 64
#define MB_SENSOR_CAPABILITY2_TOPOGRAPHY_STRUCTUREDLIGHT 0x00000080     // Bit 7 = 128
#define MB_SENSOR_CAPABILITY2_TOPOGRAPHY_LASERSCANNER 0x00000100        // Bit 8 = 256
#define MB_SENSOR_CAPABILITY2_UNUSED09 0x00000200                       // Bit 9 = 512
#define MB_SENSOR_CAPABILITY2_UNUSED10 0x00000400                       // Bit 10 = 1024
#define MB_SENSOR_CAPABILITY2_UNUSED11 0x00000800                       // Bit 11 = 2048
#define MB_SENSOR_CAPABILITY2_BACKSCATTER_ECHOSOUNDER 0x00001000        // Bit 12 = 4096
#define MB_SENSOR_CAPABILITY2_BACKSCATTER_SIDESCAN 0x00002000           // Bit 13 = 8192
#define MB_SENSOR_CAPABILITY2_BACKSCATTER_INTERFEROMETRY 0x00004000     // Bit 14 = 16384
#define MB_SENSOR_CAPABILITY2_BACKSCATTER_SASS 0x00008000               // Bit 15 = 32768
#define MB_SENSOR_CAPABILITY2_BACKSCATTER_MULTIBEAM 0x00010000          // Bit 16 = 65536
#define MB_SENSOR_CAPABILITY2_BACKSCATTER_LIDAR 0x00020000              // Bit 17= 131072
#define MB_SENSOR_CAPABILITY2_BACKSCATTER_STRUCTUREDLIGHT 0x00040000    // Bit 18 = 262144
#define MB_SENSOR_CAPABILITY2_BACKSCATTER_LASERSCANNER 0x00080000       // Bit 19 = 524288
#define MB_SENSOR_CAPABILITY2_UNUSED20 0x00100000                       // Bit 20 = 1048576
#define MB_SENSOR_CAPABILITY2_SUBBOTTOM_ECHOSOUNDER 0x00200000          // Bit 21 = 2097152
#define MB_SENSOR_CAPABILITY2_SUBBOTTOM_CHIRP 0x00400000                // Bit 22 = 4194304
#define MB_SENSOR_CAPABILITY2_UNUSED23 0x00800000                       // Bit 23 = 8388608
#define MB_SENSOR_CAPABILITY2_PHOTOGRAPHY 0x01000000                    // Bit 24 = 16777216
#define MB_SENSOR_CAPABILITY2_STEREOPHOTOGRAPHY 0x02000000              // Bit 25 = 33554432
#define MB_SENSOR_CAPABILITY2_VIDEO 0x04000000                          // Bit 26 = 67108864
#define MB_SENSOR_CAPABILITY2_STEREOVIDEO 0x08000000                    // Bit 27 = 134217728
#define MB_SENSOR_CAPABILITY2_UNUSED28 0x10000000                       // Bit 28 = 268435456
#define MB_SENSOR_CAPABILITY2_UNUSED29 0x20000000                       // Bit 29 = 536870912
#define MB_SENSOR_CAPABILITY2_UNUSED30 0x40000000                       // Bit 30 = 1073741824
#define MB_SENSOR_CAPABILITY2_UNUSED31 0x80000000                       // Bit 31 = 2147483648
#define mb_check_sensor_capability2_topography_echosounder(F) ((int)(F & MB_SENSOR_CAPABILITY2_TOPOGRAPHY_ECHOSOUNDER))
#define mb_check_sensor_capability2_topography_interferometry(F) ((int)(F & MB_SENSOR_CAPABILITY2_TOPOGRAPHY_INTERFEROMETRY))
#define mb_check_sensor_capability2_topography_sass(F) ((int)(F & MB_SENSOR_CAPABILITY2_TOPOGRAPHY_SASS))
#define mb_check_sensor_capability2_topography_multibeam(F) ((int)(F & MB_SENSOR_CAPABILITY2_TOPOGRAPHY_MULTIBEAM))
#define mb_check_sensor_capability2_topography_photogrammetry(F) ((int)(F & MB_SENSOR_CAPABILITY2_TOPOGRAPHY_PHOTOGRAMMETRY))
#define mb_check_sensor_capability2_topography_structurefrommotion(F)                                                            \
  ((int)(F & MB_SENSOR_CAPABILITY2_TOPOGRAPHY_STRUCTUREFROMMOTION))
#define mb_check_sensor_capability2_topography_lidar(F) ((int)(F & MB_SENSOR_CAPABILITY2_TOPOGRAPHY_LIDAR))
#define mb_check_sensor_capability2_topography_structuredlight(F) ((int)(F & MB_SENSOR_CAPABILITY2_TOPOGRAPHY_STRUCTUREDLIGHT))
#define mb_check_sensor_capability2_topography_laserscanner(F) ((int)(F & MB_SENSOR_CAPABILITY2_TOPOGRAPHY_LASERSCANNER))
#define mb_check_sensor_capability2_unused09(F) ((int)(F & MB_SENSOR_CAPABILITY2_UNUSED09))
#define mb_check_sensor_capability2_unused10(F) ((int)(F & MB_SENSOR_CAPABILITY2_UNUSED10))
#define mb_check_sensor_capability2_unused11(F) ((int)(F & MB_SENSOR_CAPABILITY2_UNUSED11))
#define mb_check_sensor_capability2_backscatter_echosounder(F) ((int)(F & MB_SENSOR_CAPABILITY2_BACKSCATTER_ECHOSOUNDER))
#define mb_check_sensor_capability2_backscatter_sidescan(F) ((int)(F & MB_SENSOR_CAPABILITY2_BACKSCATTER_SIDESCAN))
#define mb_check_sensor_capability2_backscatter_interferometry(F) ((int)(F & MB_SENSOR_CAPABILITY2_BACKSCATTER_INTERFEROMETRY))
#define mb_check_sensor_capability2_backscatter_sass(F) ((int)(F & MB_SENSOR_CAPABILITY2_BACKSCATTER_SASS))
#define mb_check_sensor_capability2_backscatter_multibeam(F) ((int)(F & MB_SENSOR_CAPABILITY2_BACKSCATTER_MULTIBEAM))
#define mb_check_sensor_capability2_backscatter_lidar(F) ((int)(F & MB_SENSOR_CAPABILITY2_BACKSCATTER_LIDAR))
#define mb_check_sensor_capability2_backscatter_structuredlight(F) ((int)(F & MB_SENSOR_CAPABILITY2_BACKSCATTER_STRUCTUREDLIGHT))
#define mb_check_sensor_capability2_backscatter_laserscanner(F) ((int)(F & MB_SENSOR_CAPABILITY2_BACKSCATTER_LASERSCANNER))
#define mb_check_sensor_capability2_unused20(F) ((int)(F & MB_SENSOR_CAPABILITY2_UNUSED20))
#define mb_check_sensor_capability2_subbottom_echosounder(F) ((int)(F & MB_SENSOR_CAPABILITY2_SUBBOTTOM_ECHOSOUNDER))
#define mb_check_sensor_capability2_subbottom_chirp(F) ((int)(F & MB_SENSOR_CAPABILITY2_SUBBOTTOM_CHIRP))
#define mb_check_sensor_capability2_unused23(F) ((int)(F & MB_SENSOR_CAPABILITY2_UNUSED23))
#define mb_check_sensor_capability2_photography(F) ((int)(F & MB_SENSOR_CAPABILITY2_PHOTOGRAPHY))
#define mb_check_sensor_capability2_stereophotography(F) ((int)(F & MB_SENSOR_CAPABILITY2_STEREOPHOTOGRAPHY))
#define mb_check_sensor_capability2_video(F) ((int)(F & MB_SENSOR_CAPABILITY2_VIDEO))
#define mb_check_sensor_capability2_stereovideo(F) ((int)(F & MB_SENSOR_CAPABILITY2_STEREOVIDEO))
#define mb_check_sensor_capability2_unused28(F) ((int)(F & MB_SENSOR_CAPABILITY2_UNUSED28))
#define mb_check_sensor_capability2_unused29(F) ((int)(F & MB_SENSOR_CAPABILITY2_UNUSED29))
#define mb_check_sensor_capability2_unused30(F) ((int)(F & MB_SENSOR_CAPABILITY2_UNUSED30))
#define mb_check_sensor_capability2_unused31(F) ((int)(F & MB_SENSOR_CAPABILITY2_UNUSED31))

/* survey platform definition structures */
struct mb_sensor_offset_struct {
  int position_offset_mode;
  double position_offset_x;
  double position_offset_y;
  double position_offset_z;

  int attitude_offset_mode;
  double attitude_offset_heading;
  double attitude_offset_roll;
  double attitude_offset_pitch;

  FILE *ofp; /* file pointer for integrated nav output by mbpreprocess */
};

/** Describes individual sensor */
struct mb_sensor_struct {
  int type;
  mb_longname model;
  mb_longname manufacturer;
  mb_longname serialnumber;
  int capability1; ///< bitmask indicating position and attitude capabilities
  int capability2; ///< bitmask indicating mapping and imaging capabilities
  int num_offsets; /**< most sensors have one set of offsets, multibeam sonars
                    have two sets of offsets, one for the transmit and
                    one for the receive array */
  int num_offsets_alloc;
  struct mb_sensor_offset_struct *offsets;

  int time_latency_mode;
  double time_latency_static;
  int num_time_latency;
  int num_time_latency_alloc;
  double *time_latency_time_d;
  double *time_latency_value;
};

struct mb_platform_struct {
  int type;
  mb_longname name;
  mb_longname organization;
  mb_longname documentation_url;
  double start_time_d;
  double end_time_d;
  int start_time_i[7];
  int end_time_i[7];

  int source_bathymetry;
  int source_bathymetry1;
  int source_bathymetry2;
  int source_bathymetry3;
  int source_backscatter;
  int source_backscatter1;
  int source_backscatter2;
  int source_backscatter3;
  int source_subbottom;
  int source_subbottom1;
  int source_subbottom2;
  int source_subbottom3;
  int source_position;
  int source_position1;
  int source_position2;
  int source_position3;
  int source_depth;
  int source_depth1;
  int source_depth2;
  int source_depth3;
  int source_heading;
  int source_heading1;
  int source_heading2;
  int source_heading3;
  int source_rollpitch;
  int source_rollpitch1;
  int source_rollpitch2;
  int source_rollpitch3;
  int source_heave;
  int source_heave1;
  int source_heave2;
  int source_heave3;

  int num_sensors;
  int num_sensors_alloc;
  struct mb_sensor_struct *sensors;
};

/* ---------------------------------------------------------------------------*/
/* MBIO data storage and control structures */

/** MBIO file index storage structure */
struct mb_io_indextable_struct {
    int file_index;
    int total_index_org;
    int total_index_sorted;
    int subsensor;
    int subsensor_index;
    double time_d_org;
    double time_d_corrected;
    long offset;
    size_t size;
    mb_u_char kind;
    mb_u_char read;
};

/** MBIO ping storage structure */
struct mb_io_ping_struct {
  double time_d;
  double navlon;
  double navlat;
  double speed;
  double heading;
  double distance;
  double altitude;
  double sonardepth;
  int nbath;
  int namp;
  int nss;
  char *beamflag;
  double *bath;
  double *bathlon;
  double *bathlat;
  double *amp;
  double *ss;
  double *sslon;
  double *sslat;
};

/** MBIO input/output control structure */
struct mb_io_struct {
  /* system byte swapping */
  int byteswapped; /**< 0 = unswapped, 1 = swapped (Intel byte order) */

  /* format parameters */
  int format;           /**< data format id */
  int system;           /**< sonar system id */
  int beams_bath_max;   /**< maximum number of bathymetry beams */

  /** maximum number of amplitude beams - either 0 or = beams_bath */
  int beams_amp_max; 
                   
  int pixels_ss_max;    /**< maximum number of sidescan pixels */
  int beams_bath_alloc; /**< allocated number of bathymetry beams */
  int beams_amp_alloc;  /**< allocated number of amplitude beams */
  int pixels_ss_alloc;  /**< allocated number of sidescan pixels */
  char format_name[MB_NAME_LENGTH];
  char system_name[MB_NAME_LENGTH];
  char format_description[MB_DESCRIPTION_LENGTH];
  int numfile;             /**< the number of parallel files required for i/o */
  int filetype;            /**< type of files used (normal, single normal, xdr, or gsf) */
  mb_filemode_enum filemode; /**< file mode (read or write) */
  // TODO(schwehr): Bool
  int variable_beams;      /**< if true then number of beams variable */
  int traveltime;          /**< if true then traveltime and angle data supported */
  int beam_flagging;       /**< if true then beam flagging supported */
  int platform_source;     /**< data record type containing sensor offsets */
  int nav_source;          /**< data record type containing the primary navigation */
  int sensordepth_source;  /**< data record type containing the primary sensordepth */
  int heading_source;      /**< data record type containing the primary heading */
  int attitude_source;     /**< data record type containing the primary attitude */
  int svp_source;          /**< data record type containing the primary svp */
  double beamwidth_xtrack; /**< nominal acrosstrack beamwidth */
  double beamwidth_ltrack; /**< nominal alongtrack beamwidth */

  /* control parameters - see mbio manual pages for explanation */
  int pings;        /**< controls ping averaging */
  int lonflip;      /**< controls longitude range */
  double bounds[4]; /**< locations bounds of acceptable data */
  int btime_i[7];   /**< beginning time of acceptable data */
  int etime_i[7];   /**< ending time of acceptable data */
  double btime_d;   /**< beginning time of acceptable data
                in "_d" format (unix seconds) */
  double etime_d;   /**< ending time of acceptable data
                in "_d" format (unix seconds) */
  double speedmin;  /**< minimum ship speed of acceptable data
                in km/hr */
  double timegap;   /**< maximum time between pings without
                a data gap */

  /** application defined i/o accessed through mb_input_init()
  rather than mb_read_init(), usually socket based */
  void *mbsp;

  /* file descriptor, file name, and usage flag */
  FILE *mbfp;                  /**< file descriptor */
  mb_path file;                /**< file name */
  long file_pos;               /**< file position at start of last record read */
  long file_bytes;             /**< number of bytes read from file */
  char *file_iobuffer;         /**< file i/o buffer for fread() and fwrite() calls */
  FILE *mbfp2;                 /**< file descriptor #2 */
  char file2[MB_PATH_MAXLINE]; /**< file name #2 */
  long file2_pos;              /**< file position #2 at start of last record read */
  long file2_bytes;            /**< number of bytes read from file */
  FILE *mbfp3;                 /**< file descriptor #3 */
  char file3[MB_PATH_MAXLINE]; /**< file name #3 */
  long file3_pos;              /**< file position #3 at start of last record read */
  long file3_bytes;            /**< number of bytes read from file */
  int ncid;                    /**< netCDF datastream ID */
  int gsfid;                   /**< GSF datastream ID */
  void *xdrs;                  /**< XDR stream handle */
  void *xdrs2;                 /**< XDR stream handle #2 */
  void *xdrs3;                 /**< XDR stream handle #2 */


  /** file indexing (used by some formats) */
    int num_indextable;
    int num_indextable_alloc;
    struct mb_io_indextable_struct *indextable;

  /* read or write history */
  int fileheader;       /**< indicates whether file header has
                        been read or written */
  int hdr_comment_size; /**< number of characters in
                   header_comment string */
  int hdr_comment_loc;  /**< number of characters already extracted
                    from header_comment string */
  char *hdr_comment; /**< placeholder for long comment strings
             for formats using a single
             comment string in a file header */
  int irecord_count; /**< counting variable used for VMS derived
                 data formats to remove extra
                 bytes (e.g. sburivax format) */
  int orecord_count; /**< counting variable used for VMS derived
                 data formats to insert extra
                 bytes (e.g. sburivax format) */

  /* pointer to structure containing raw data (could be any format) */
  int structure_size;
  int data_structure_size;
  int header_structure_size;
  void *raw_data;
  void *store_data;

  /* working variables */
  int ping_count;    /**< number of pings read or written so far */
  int nav_count;     /**< number of nav records read or written so far */
  int comment_count; /**< number of comments read or written so far */
  int pings_avg;     /**< number of pings currently averaged */
  int pings_read;    /**< number of pings read this binning cycle */
  int error_save;    /**< saves time gap error to end of binning */
  double last_time_d;
  double last_lon;
  double last_lat;
  double old_time_d;
  double old_lon;
  double old_lat;
  double old_ntime_d;
  double old_nlon;
  double old_nlat;

  /* data binning variables */
  int pings_binned;
  double time_d;
  double lon;
  double lat;
  double speed;
  double heading;
  char *beamflag;
  double *bath;
  double *amp;
  double *bath_acrosstrack;
  double *bath_alongtrack;
  int *bath_num;
  int *amp_num;
  double *ss;
  double *ss_acrosstrack;
  double *ss_alongtrack;
  int *ss_num;

  /** @name current ping variables */
  ///@{
  int need_new_ping;
  int new_kind;
  int new_error;
  char new_comment[MB_COMMENT_MAXLINE];
  int new_time_i[7];
  double new_time_d;
  double new_lon;
  double new_lat;
  double new_speed;
  double new_heading;
  int new_beams_bath; /**< number of bathymetry beams */
  int new_beams_amp;  /**< number of amplitude beams
                  - either 0 or = beams_bath */
  int new_pixels_ss;  /**< number of sidescan pixels */
  char *new_beamflag;
  double *new_bath;
  double *new_amp;
  double *new_bath_acrosstrack;
  double *new_bath_alongtrack;
  double *new_ss;
  double *new_ss_acrosstrack;
  double *new_ss_alongtrack;
  ///@}
  
  /* variables for projections to and from projected coordinates */
  int projection_initialized;  // TODO(schwehr): bool
  char projection_id[MB_NAME_LENGTH];
  void *pjptr;

  /* variables for interpolating/extrapolating navigation
      for formats containing nav as asynchronous
      position records separate from ping data */
  int nfix;
  double fix_time_d[MB_ASYNCH_SAVE_MAX];
  double fix_lon[MB_ASYNCH_SAVE_MAX];
  double fix_lat[MB_ASYNCH_SAVE_MAX];

  /* variables for interpolating/extrapolating attitude
      for formats containing attitude as asynchronous
      data records separate from ping data */
  int nattitude;
  double attitude_time_d[MB_ASYNCH_SAVE_MAX];
  double attitude_heave[MB_ASYNCH_SAVE_MAX];
  double attitude_roll[MB_ASYNCH_SAVE_MAX];
  double attitude_pitch[MB_ASYNCH_SAVE_MAX];

  /* variables for interpolating/extrapolating heading
      for formats containing heading as asynchronous
      data records separate from ping data */
  int nheading;
  double heading_time_d[MB_ASYNCH_SAVE_MAX];
  double heading_heading[MB_ASYNCH_SAVE_MAX];

  /* variables for interpolating/extrapolating sonar depth
      for formats containing sonar depth as asynchronous
      data records separate from ping data */
  int nsonardepth;
  double sonardepth_time_d[MB_ASYNCH_SAVE_MAX];
  double sonardepth_sonardepth[MB_ASYNCH_SAVE_MAX];

  /* variables for interpolating/extrapolating altitude
      for formats containing altitude as asynchronous
      data records separate from ping data */
  int naltitude;
  double altitude_time_d[MB_ASYNCH_SAVE_MAX];
  double altitude_altitude[MB_ASYNCH_SAVE_MAX];

  /* preprocessing parameter structure used by some formats */
  struct mb_preprocess_struct preprocess_pars;

  /* variables for accumulating MBIO notices */
  int notice_list[MB_NOTICE_MAX];

  /* variable for registering and maintaining application i/o arrays */
  int bath_arrays_reallocated;
  int amp_arrays_reallocated;
  int ss_arrays_reallocated;
  int n_regarray;
  int n_regarray_alloc;
  void **regarray_handle;
  void **regarray_ptr;
  void **regarray_oldptr;
  int *regarray_type;
  size_t *regarray_size;

  /* variables for saving information */
  char save_label[12];
  int save_label_flag;
  int save_flag;
  int save1;
  int save2;
  int save3;
  int save4;
  int save5;
  int save6;
  int save7;
  int save8;
  int save9;
  int save10;
  int save11;
  int save12;
  int save13;
  int save14;
  int save15;
  int save16;
  int save17;
  int save18;
  int save19;
  int save20;
  int save21;
  int save22;
  double saved1;
  double saved2;
  double saved3;
  double saved4;
  double saved5;
  void *saveptr1;
  void *saveptr2;
  void *saveptr3;

  /* function pointers for allocating and deallocating format
      specific structures */
  int (*mb_io_format_alloc)(int verbose, void *mbio_ptr, int *error);
  int (*mb_io_format_free)(int verbose, void *mbio_ptr, int *error);
  int (*mb_io_store_alloc)(int verbose, void *mbio_ptr, void **store_ptr, int *error);
  int (*mb_io_store_free)(int verbose, void *mbio_ptr, void **store_ptr, int *error);

  /** @name function pointers for reading and writing records */
  ///@{
  int (*mb_io_read_ping)(int verbose, void *mbio_ptr, void *store_ptr, int *error);
  int (*mb_io_write_ping)(int verbose, void *mbio_ptr, void *store_ptr, int *error);
  ///@}

  
  /** @name function pointers for extracting and inserting data */
  ///@{
  int (*mb_io_dimensions)(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss, int *error);
  int (*mb_io_pingnumber)(int verbose, void *mbio_ptr, unsigned int *pingnumber, int *error);
  int (*mb_io_segynumber)(int verbose, void *mbio_ptr, unsigned int *line, unsigned int *shot, unsigned int *cdp, int *error);
  int (*mb_io_sonartype)(int verbose, void *mbio_ptr, void *store_ptr, int *sonartype, int *error);
  int (*mb_io_sidescantype)(int verbose, void *mbio_ptr, void *store_ptr, int *ss_type, int *error);
  int (*mb_io_preprocess)(int verbose, void *mbio_ptr, void *store_ptr, void *platform_ptr, void *preprocess_pars, int *error);
  int (*mb_io_extract_platform)(int verbose, void *mbio_ptr, void *store_ptr, int *kind, void **platform_ptr, int *error);
  int (*mb_io_sensorhead)(int verbose, void *mbio_ptr, void *store_ptr, int *sensorhead, int *error);
  int (*mb_io_extract)(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                       double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss, char *beamflag,
                       double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss,
                       double *ssacrosstrack, double *ssalongtrack, char *comment, int *error);
  int (*mb_io_insert)(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
                      double navlat, double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath,
                      double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
                      double *ssalongtrack, char *comment, int *error);
  int (*mb_io_extract_nav)(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                           double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll,
                           double *pitch, double *heave, int *error);
  int (*mb_io_extract_nnav)(int verbose, void *mbio_ptr, void *store_ptr, int nmax, int *kind, int *n, int *time_i,
                            double *time_d, double *navlon, double *navlat, double *speed, double *heading, double *draft,
                            double *roll, double *pitch, double *heave, int *error);
  int (*mb_io_insert_nav)(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
                          double navlat, double speed, double heading, double draft, double roll, double pitch, double heave,
                          int *error);
  int (*mb_io_extract_altitude)(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                                double *altitude, int *error);
  int (*mb_io_insert_altitude)(int verbose, void *mbio_ptr, void *store_ptr, double transducer_depth, double altitude,
                               int *error);
  int (*mb_io_extract_svp)(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsvp, double *depth, double *velocity,
                           int *error);
  int (*mb_io_insert_svp)(int verbose, void *mbio_ptr, void *store_ptr, int nsvp, double *depth, double *velocity, int *error);
  int (*mb_io_ttimes)(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
                      double *angles_forward, double *angles_null, double *heave, double *alongtrack_offset, double *draft,
                      double *ssv, int *error);
  int (*mb_io_detects)(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error);
  int (*mb_io_pulses)(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *pulses, int *error);
  int (*mb_io_gains)(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transmit_gain, double *pulse_length,
                     double *receive_gain, int *error);
  int (*mb_io_extract_rawssdimensions)(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *sample_interval,
                                       int *num_samples_port, int *num_samples_stbd, int *error);
  int (*mb_io_extract_rawss)(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *sidescan_type,
                             double *sample_interval, double *beamwidth_xtrack, double *beamwidth_ltrack, int *num_samples_port,
                             double *rawss_port, int *num_samples_stbd, double *rawss_stbd, int *error);
  int (*mb_io_insert_rawss)(int verbose, void *mbio_ptr, void *store_ptr, int kind, int sidescan_type, double sample_interval,
                            double beamwidth_xtrack, double beamwidth_ltrack, int num_samples_port, double *rawss_port,
                            int num_samples_stbd, double *rawss_stbd, int *error);
  int (*mb_io_makess)(int verbose, void *mbio_ptr, void *store_ptr, int pixel_size_set, double *pixel_size,
                         int swath_width_set, double *swath_width, int pixel_int, int *error);
  int (*mb_io_extract_segytraceheader)(int verbose, void *mbio_ptr, void *store_ptr, int *kind, void *segytraceheader_ptr,
                                       int *error);
  int (*mb_io_extract_segy)(int verbose, void *mbio_ptr, void *store_ptr, int *sampleformat, int *kind,
                            void *segytraceheader_ptr, float *segydata, int *error);
  int (*mb_io_insert_segy)(int verbose, void *mbio_ptr, void *store_ptr, int kind, void *segytraceheader_ptr, float *segydata,
                           int *error);
  int (*mb_io_ctd)(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nctd, double *time_d, double *conductivity,
                   double *temperature, double *depth, double *salinity, double *soundspeed, int *error);
  int (*mb_io_ancilliarysensor)(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsensor, double *time_d,
                                double *sensor1, double *sensor2, double *sensor3, double *sensor4, double *sensor5,
                                double *sensor6, double *sensor7, double *sensor8, int *error);
  int (*mb_io_copyrecord)(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error);
  ///@}
  
  /** @name function pointers used by mbpreprocess to fix timestamps */
  ///@{
  int (*mb_io_indextablefix)(int verbose, void *mbio_ptr, int num_indextable, void *indextable_ptr, int *error);
    int (*mb_io_indextableapply)(int verbose, void *mbio_ptr, int num_indextable, void *indextable_ptr, int n_file, int *error);
  ///@}
  
  /** @name function pointers for reading from application defined input */
  ///@{
  int (*mb_io_input_open)(int verbose, void *mbio_ptr, char *definition, int *error);
  int (*mb_io_input_read)(int verbose, void *mbio_ptr, size_t *size, char *buffer, int *error);
  int (*mb_io_input_close)(int verbose, void *mbio_ptr, int *error);
  ///@}
};

/** MBIO buffer control structure */
struct mb_buffer_struct {
  void *buffer[MB_BUFFER_MAX];
  int buffer_kind[MB_BUFFER_MAX];
  int nbuffer;
};

#define MB_DATALIST_RECURSION_MAX 25

/** MBIO datalist control structure */
struct mb_datalist_struct {
  bool open;
  int recursion;
  int look_processed;
  bool local_weight;
  bool weight_set;
  double weight;
  FILE *fp;
  char path[MB_PATH_MAXLINE];
  int printed;
  struct mb_datalist_struct *datalist;
};

#define MB_IMAGELIST_RECURSION_MAX 25

/** MBIO imagelist control structure */
struct mb_imagelist_struct {
  int open;
  int recursion;
    int leftrightstereo;
    int printed;
   char path[MB_PATH_MAXLINE];
  FILE *fp;
  struct mb_imagelist_struct *imagelist;
};

#endif  /* MB_IO_H_ */
