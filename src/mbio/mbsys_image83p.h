/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_image83p.h	5/5/2008
 *
 *    Copyright (c) 2008-2020 by
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
 * mbsys_image83p.h defines the data structures used by MBIO functions
 * to store data from Imagenex DeltaT multibeam sonar systems.
 * The data formats which are commonly used to store Imagenex DeltaT
 * data in files include
 *      MBF_IMAGE83P : MBIO ID 191
 *      MBF_IMAGEMBA : MBIO ID 192
 *
 * Author: Vivek Reddy, Santa Clara University (initial version)
 *         D.W. Caress (many revisions since)
 * Date:  May 5, 2008
 *
 *
 */
/*
 * Notes on the MBSYS_IMAGE83P data structure:
 *   1. Imagenex DeltaT multibeam systems output raw data in a format
 *      combining ascii and binary values.
 *   2. These systems output up to 480 beams of bathymetry
 *   3. The data structure defined below includes all of the values
 *      which are passed in the 83P Imagenex data format records plus many
 *      values calculated from the raw data.
 *   4. The initial 83P format version was labeled 1.xx but is coded as 1.00. The
 *      second format version is 1.10. As of November 2022, versions through 1.10
 *      are supported as format MBF_IMAGE83 (191).
 *   5. Support for comment records is specific to MB-System.
 *   6. The MBF_IMAGE83P format does not support beam flags. Support for beam
 *      flags is specific to the extended MB-System format MBF_IMAGEMBA (id=192).
 *      Format MBF_IMAGEMBA records also include the bathymetry soundings
 *      calculated as arrays of bathymetry values and the acrosstrack and
 *      alongtrack positions of the soundings.
 *   7. Both formats have two spaces for recording heading, roll, and pitch. If the
 *      multibeam has its own attitude sensor then these values are recorded with
 *      0.1 degree precision. There are other spaces in the header for heading,
 *      roll and pitch stored as floats so that there are several digits of
 *      precision available. In some installations the logged files include
 *      attitude data in those secondary fields from an external sensor (and in
 *      that case can also include heave). MB-System uses the float attitude values
 *      in processing. When reading a file, if the internal integer values are
 *      nonzero and the external float values are flagged as undefined, then the
 *      former values (converted to degrees) are copied to the latter.
 *      Subsequently the external float fields are used as the source for heading
 *      and attitude data.
 *   8. The vendor MBF_IMAGE83P format does not include a field for sonar depth, but does
 *      include a field for heave. The extended MBF_IMAGEMBA format includes separate
 *      float fields for both heave and sonar depth - the sonar depth is typically used
 *      either as a static draft on a surface vessel or a pressure depth on a
 *      submerged AUV or ROV platform. Heave is positive up and sonar depth is
 *      positive down. In some cases on submerged platforms the pressure depth is
 *      recorded into the heave field. In that case the --kluge-sensordepth-from-heave
 *      argument to mbpreprocess will cause the heave value to be moved to the
 *      sonar_depth field in the output MBF_IMAGEMBA format files.
 *   9. Comment records are supported for both formats - this is specific to MB-System.
 */

/* The following .83P format specification information is taken from a
 *      16 March 2010 document from Imagenex Technology Corporation.
 *
 *  DeltaT - 83P PROFILE POINT OUTPUT
 *  ( 83P UDP/IP Ethernet Datagram, .83P File Format )
 *
 *  For each ping, the following bytes are output during the 83P UDP datagram. If
 *  recording to a .83P file, the following bytes are appended and saved to the file
 *  for each ping. The total number of bytes ‘N’ for each ping will vary depending
 *  on the number of beams selected.
 *
 *      Byte #          Byte Description
 *      ------          ----------------
 *      0-255       File Header (256 bytes)
 *      256-nnn     Profile Ranges for current ping (2 range bytes / beam)
 *                    nnn = 256 + (2*number_of_beams) – 1
 *                  If Intensity Bytes are included (Byte 117 = 1),
 *                    nnn = 256 + (4*number_of_beams) – 1
 *
 *  FILE HEADER (note -this is really a record header present for each survey record)
 *  Bytes 0 through 255 contain the following File Header information:
 *
 *  0       ASCII '8'
 *  1       ASCII '3'
 *  2       ASCII 'P'
 *
 *  3       .83P File Version
 *          0 = v1.00
 *          10 = v1.10
 *
 *  4-5     Total Bytes ‘N’ - number of bytes that are written to the disk for this ping
 *          Byte 117 = 0 (No Intensity) ==> N = 256 + (2*number_of_beams)
 *          Byte 117 = 1 (Intensity)    ==> N = 256 + (4*number_of_beams)
 *
 *  6       Reserved - always 0
 *  7       Reserved - always 0
 *
 *  8-19    Sonar Ping Interrogation Timestamp
 *            Date – system date, null terminated string (12 bytes) "DD-MMM-YYYY"
 *
 *  20-28   Sonar Ping Interrogation Timestamp
 *            Time – system time, null terminated string (9 bytes) "HH:MM:SS"
 *
 *  29-32   Sonar Ping Interrogation Timestamp
 *            Hundredths of Seconds – system time, null terminated string (4 bytes) ".hh"
 *            Note: see Bytes 112-116 for Milliseconds.
 *
 *  33-46   GNSS Ships Position Latitude – text string (14 bytes) “_dd.mm.xxxxx_N”
 *          dd = Degrees
 *          mm = Minutes
 *          xxxxx = Decimal Minutes
 *          _ = Space
 *          N = North or S = South
 *
 *  47-60   GNSS Ships Position Longitude – text string (14 bytes) “ddd.mm.xxxxx_E”
 *          ddd = Degrees
 *          mm = Minutes
 *          xxxxx = Decimal Minutes
 *          _ = Space
 *          E = East or W = West
 *
 *  61 GNSS Ships Speed
 *          Speed = (Byte 61)/10 in knots
 *
 *  62-63 GNSS Ships Course
 *          |        byte 62         |        byte 63         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |           Course * 10 (in degrees)              |
 *
 *  64-65 Pitch Angle (from Internal Sensor)
 *          |        byte 64         |        byte 65         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          | P |       (Pitch Angle*10) + 900                |
 *                If 'P' = 0, Pitch Angle = 0 degrees
 *                If 'P' = 1, Pitch Angle = [[((Byte 64 & 0x7F)<<8) | (Byte 65)]-900]/10
 *
 *  66-67 Roll Angle (from Internal Sensor)
 *          |        byte 66         |        byte 67         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          | R |       (Roll Angle*10) + 900                 |
 *                If 'R' = 0, Roll Angle = 0 degrees
 *                If 'R' = 1, Roll Angle = [[((Byte 66 & 0x7F)<<8) | (Byte 67)]-900]/10
 *
 *  68-69 Heading Angle (from Internal Sensor)
 *          |        byte 68         |        byte 69         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          | H |       Heading Angle*10                      |
 *                If 'H' = 0, Heading Angle = 0 degrees
 *                If 'H' = 1, Heading Angle = [((Byte 68 & 0x7F)<<8) | (Byte 69)]/10
 *
 *  70-71 Beams
 *          |        byte 70         |        byte 71         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |                    Number of Beams              |
 *
 *  72-73 Samples Per Beam
 *          |        byte 72         |        byte 73         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |              Number of Samples Per Beam         |
 *
 *  74-75 Sector Size
 *          |        byte 74         |        byte 75         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |              Sector Size (in degrees)           |
 *
 *  76-77 Start Angle (Beam 0 angle)
 *          |        byte 76         |        byte 77         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |    [Start Angle (in degrees) + 180] * 100       |
 *
 *  78 Angle Increment
 *          Angle spacing per beam = (Byte 78)/100 in degrees
 *
 *  79-80 Acoustic Range
 *          |        byte 79         |        byte 80         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |              Acoustic Range (in meters)         |
 *
 *  81-82 Acoustic Frequency
 *          |        byte 81         |        byte 82         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |             Acoustic Frequency (in kHz)         |
 *
 *  83-84 Sound Velocity
 *          |        byte 83         |        byte 84         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          | V |  Sound Velocity (in meters/second) * 10     |
 *                If 'V' = 0, Sound Velocity = 1500.0 m/s
 *                If 'V' = 1, Sound Velocity = [((Byte 83 & 0x7F)<<8) | (Byte 84)]/10.0
 *
 *  85-86 Range Resolution
 *          |        byte 85         |        byte 86         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |       Range Resolution (in millimeters)         |
 *
 *  87-88 Reserved
 *          Reserved – always 0
 *
 *  89-90 Profile Tilt Angle (mounting offset)
 *          |        byte 89         |        byte 90         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |     Profile Tilt Angle (in degrees) + 180       |
 *
 *  91-92 Repetition Rate – Time between pings
 *          |        byte 91         |        byte 92         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |       Repetition Rate (in milliseconds)         |
 *
 *  93-96 Ping Number – increment for every ping
 *          |  byte 93  |  byte 94   |   byte 93  |  byte 94  |
 *          |                   Ping Number                   |
 *
 *  Version 1.00 - end of nonzero parameters in the header
 *  97-255  Reserved – always 0
 *
 *  Version 1.10 - bytes 100-154 now used for additional parameters
 *
 *  97-98  Reserved – always 0
 *
 *  100-103 Sonar X-Offset – 4-byte single precision floating point number
 *          |  byte 100 |  byte 101  |   byte 102 |  byte 103 |
 *          |            Sonar X-Offset (in meters)           |
 *
 *  104-107 Sonar Y-Offset – 4-byte single precision floating point number
 *          |  byte 100 |  byte 101  |   byte 102 |  byte 103 |
 *          |            Sonar Y-Offset (in meters)           |
 *
 *  108-111 Sonar Z-Offset – 4-byte single precision floating point number
 *          |  byte 100 |  byte 101  |   byte 102 |  byte 103 |
 *          |            Sonar Z-Offset (in meters)           |
 *
 *  112-116 Sonar Ping Interrogation Timestamp
 *          Milliseconds – system time, null terminated string (5 bytes) ".mmm"
 *
 *  117 Intensity Bytes Included
 *          0 = No
 *          1 = Yes
 *
 *  118-119 Ping Latency – Time from sonar ping interrogation to actual ping
 *          |        byte 91         |        byte 92         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |   Ping Latency (in units of 100 microseconds)   |
 *
 *  120-121 Data Latency – Time from sonar ping interrogation to 83P UDP datagram
 *          |        byte 91         |        byte 92         |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |   Data Latency (in units of 100 microseconds)   |
 *          Time Since Ping = Data Latency – Ping Latency
 *          Note: Data Latency is not available during file playback.
 *
 *  122 Sample Rate
 *          0 = Standard Resolution (1 in 500)
 *          1 = High Resolution (1 in 5000)
 *
 *  123 Option Flags
 *          Bit 0 – 1 = data is corrected for roll
 *          Bit 1 – 1 = data is corrected for ray bending
 *          Bit 2 – 1 = sonar is operating in overlapped mode Bit 3 – 0
 *          Bit 4 – 0
 *          Bit 5 – 0
 *          Bit 6 – 0
 *          Bit 7 – 0
 *
 *  124 Reserved - always 0
 *
 *  125 Number of Pings Averaged
 *          0 to 25
 *
 *  126-127 Center Ping Time Offset
 *          The Sonar Ping Interrogation Timestamp (Bytes 8-19, 20-28 and 112-116)
 *          is the timestamp for the current ping. But due to ping averaging,
 *          the ping time of the center ping (of a group of averaged pings)
 *          may be required (i.e. for roll stabilization). The Center Ping Time
 *          Offset is the time difference between the center ping interrogation
 *          and the current ping interrogation.
 *          |        byte 126        |        byte 127        |
 *          | 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 |
 *          |   Center Ping Time Offset (in units of 100 microseconds)   |
 *          Center Ping Time = Sonar Ping Interrogation Timestamp
 *                             – Center Ping Time Offset + Ping Latency
 *          Note: Profile data from the current ping should be used when
 *          subtracting the Center Ping Time Offset.
 *
 *  128-131 Heave (from External Sensor)
 *          4-byte single precision floating point number
 *          |  byte 128 |  byte 129  |   byte 130 |  byte 131 |
 *          |                 Heave (in meters)               |
 *
 *  132     User Defined Byte – this is a copy of the 837 User Defined Byte
           (Byte 45 from the .837 File Header)
 *
 *  133-136 Altitude
 *          4-byte single precision floating point number
 *          |  byte 133 |  byte 134  |   byte 135 |  byte 136 |
 *          |               Altitude (in meters)              |
 *
 *  137     External Sensor Flags
 *          Bit 0 – 1 = external heading angle available
 *          Bit 1 – 1 = external roll angle available
 *          Bit 2 – 1 = external pitch angle available
 *          Bit 3 – 1 = external heave available
 *          Bit 4 – 0
 *          Bit 5 – 0
 *          Bit 6 – 0
 *          Bit 7 – 0
 *
 *  138-141 Pitch Angle (from External Sensor)
 *          4-byte single precision floating point number
 *          |  byte 138 |  byte 139  |   byte 140 |  byte 141 |
 *          |                 Pitch (in degrees)              |
 *
 *  142-145 Roll Angle (from External Sensor)
 *          4-byte single precision floating point number
 *          |  byte 142 |  byte 143  |   byte 144 |  byte 145 |
 *          |                  Roll (in degrees)              |
 *
 *  146-149 Heading Angle (from External Sensor)
 *          4-byte single precision floating point number
 *          |  byte 146 |  byte 147  |   byte 148 |  byte 149 |
 *          |               Heading (in degrees)              |
 *
 *  150     Transmit Scan Flag
 *          0 = manual scan
 *          1 = auto-scan
 *
 *  151-154 Transmit Scan Angle
 *          4-byte single precision floating point number
 *          |  byte 151 |  byte 152  |   byte 153 |  byte 154 |
 *          |        Transmit Scan Angle (in degrees)         |
 *
 *  155-255 Reserved
 *          always 0
 *
 *  BEAM DATA
 *  Version 1.00: Only the beam ranges are provided, as an array of 2-byte
 *  unsigned integers in units of range resolution (bytes 85-86) assuming
 *  a 1500 m/s water sound speed. The number of beams is set in the 2-byte
 *  integer in bytes 70-71.
 *
 *  Version 1.10: If byte 117 is true, then following the array of beam ranges
 *  is an array of beam intensity (per-beam amplitude in MB-System terminology)
 *  as 2-byte unsigned integers described as "normalized amplitude". The number
 *  of beams is set in the 2-byte integer in bytes 70-71.
 */

#ifndef MBSYS_IMAGE83P_H_
#define MBSYS_IMAGE83P_H_

/* number of beams for imagex multibeam */
#define MBSYS_IMAGE83P_BEAMS 480
#define MBSYS_IMAGE83P_COMMENTLEN 248

#define MBSYS_IMAGE83P_HEADERLEN 256

struct mbsys_image83p_struct {
	/* type of data record */
	int kind;

  /* file version - 0 = version 1.0, 10 = version 1.1 */
  int version;

	/* time stamp (all records but comment ) */
	int time_i[7];
	double time_d;

	/* navigation attitude and sonar settings  */
	double nav_lat;
	double nav_long;
	int nav_speed;   /* 0.1 knots */
	int course;      /* 0.1 degrees */
	int pitch;       /* degrees / 10 - 900 */
	int roll;        /* degrees / 10 - 900 */
	int heading      /* degrees / 10 */;
	int num_beams;
	int samples_per_beam;
	int sector_size;        /* degrees */
	int start_angle;        /* 0.01 degrees + 180.0 */
	int angle_increment;    /* 0.01 degrees */
	int acoustic_range;     /* meters */
	int acoustic_frequency; /* kHz */
	int sound_velocity;     /* 0.1 m/sec */
	int range_resolution;   /* 0.001 meters */
	int pulse_length;       /* usec */
	int profile_tilt_angle; /* degrees + 180.0 */
	int rep_rate;           /* msec */
	int ping_number;

  /* parameters added in version 1.1 */
  float sonar_x_offset;   /* meters */
  float sonar_y_offset;   /* meters */
  float sonar_z_offset;   /* meters */
  bool has_intensity;     /* indicates if amplitude data are included */
  int ping_latency;       /* Time from sonar ping interrogation to actual ping (100 microseconds) */
  int data_latency;       /* Time from sonar ping interrogation to 83P UDP datagram (100 microseconds) */
  int sample_rate;        /* 0 = standard resolution (1 in 500), 1 = high resolution (1 in 5000) */
  mb_u_char option_flags;      /* Bit 0: if set data are correct4ed for roll */
                          /* Bit 1: if set data are correct4ed for ray bending */
                          /* Bit 2: if set sonar is operating in overlapped mode */
                          /* Bits 3-7 not used */
  int number_averaged;    /* Number of pings averaged (0 to 25) */
  unsigned short center_time_offset;
                          /* The Sonar Ping Interrogation Timestamp
                            (Bytes 8-19, 20-28 and 112-116) is the timestamp for
                            the current ping. But due to ping averaging, the ping
                            time of the center ping (of a group of averaged pings)
                            may be required (i.e. for roll stabilization).
                            The Center Ping Time Offset is the time difference
                            between the center ping interrogation and the current
                            ping interrogation. */
  float heave_external;  /* heave from external sensor (meters) */
  mb_u_char user_defined_byte;
                          /* User Defined Byte – this is a copy of the 837 User
                            Defined Byte (Byte 45 from the .837 File Header) */
  float altitude;         /* altitude (meters) */
  mb_u_char external_sensor_flags;
                          /* External Sensor Flags
                              Bit 0 – 1 = external heading angle available
                              Bit 1 – 1 = external roll angle available
                              Bit 2 – 1 = external pitch angle available
                              Bit 3 – 1 = external heave available
                              Bit 4 – 0
                              Bit 5 – 0
                              Bit 6 – 0
                              Bit 7 – 0 */
  float pitch_external;   /* Pitch from external sensor (degrees) */
  float roll_external;    /* Roll from external sensor (degrees) */
  float heading_external; /* Heading from external sensor (degrees) */
  mb_u_char transmit_scan_flag;   /* Transmit scan flag: 0=manual scan, 1=auto scan */
  float transmit_scan_angle;      /* Transmit scan angle (degrees) */

  /* beam values - amplitude added in version 1.1, and amplitude only present if
     the has_intensity flag (byte 117 in record header) is set true */
	int range[MBSYS_IMAGE83P_BEAMS];
  int intensity[MBSYS_IMAGE83P_BEAMS];

	/* important values not in vendor format */
	float sonar_depth; /* meters */
	int num_proc_beams;
	double beamrange[MBSYS_IMAGE83P_BEAMS];
	double angles[MBSYS_IMAGE83P_BEAMS];
	double angles_forward[MBSYS_IMAGE83P_BEAMS];
	float bath[MBSYS_IMAGE83P_BEAMS];
	float bathacrosstrack[MBSYS_IMAGE83P_BEAMS];
	float bathalongtrack[MBSYS_IMAGE83P_BEAMS];
  float amp[MBSYS_IMAGE83P_BEAMS];
	char beamflag[MBSYS_IMAGE83P_BEAMS];

	/* comment */
	char comment[MBSYS_IMAGE83P_COMMENTLEN];
};

/* system specific function prototypes */
int mbsys_image83p_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_image83p_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_image83p_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss,
                              int *error);
int mbsys_image83p_pingnumber(int verbose, void *mbio_ptr, unsigned int *pingnumber, int *error);
int mbsys_image83p_sonartype(int verbose, void *mbio_ptr, void *store_ptr, int *sonartype, int *error);
int mbsys_image83p_preprocess(int verbose,
                              void *mbio_ptr,
                              void *store_ptr,
                              void *platform_ptr, void *preprocess_pars_ptr, int *error);
int mbsys_image83p_extract_platform(int verbose, void *mbio_ptr, void *store_ptr,
                              int *kind, void **platform_ptr, int *error);
int mbsys_image83p_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                              double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss, char *beamflag,
                              double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss,
                              double *ssacrosstrack, double *ssalongtrack, char *comment, int *error);
int mbsys_image83p_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
                              double navlat, double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath,
                              double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
                              double *ssalongtrack, char *comment, int *error);
int mbsys_image83p_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
                              double *angles_forward, double *angles_null, double *heave, double *alongtrack_offset, double *draft,
                              double *ssv, int *error);
int mbsys_image83p_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error);
int mbsys_image83p_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                              double *altitudev, int *error);
int mbsys_image83p_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                              double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll,
                              double *pitch, double *heave, int *error);
int mbsys_image83p_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
                              double navlat, double speed, double heading, double draft, double roll, double pitch, double heave,
                              int *error);
int mbsys_image83p_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error);

#endif  /* MBSYS_IMAGE83P_H_ */
