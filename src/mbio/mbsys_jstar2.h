/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_jstar.h	2/21/2005
 *
 *    Copyright (c) 2005-2020 by
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
 * mbsys_jstar2.h  defines the data structure used by MBIO functions
 * to store bathymetry, amplitude and sidescan data read from the
 * MBF_EDGJSTAR format (MBIO id 134).
 *
 * Author:	D. W. Caress & C. d. S. Ferreira
 * Date:	February 21, 2005 & January 16, 2019
 *
 */
/*
 * Notes on the MBSYS_JSTAR data structure:
 *   1. The J-star data format is used to store raw sidescan data from
 *      Edgetech sidescan and subbottom profiler sonars. This format
 *      is a variant of the SEGY format. More recently the J-star data
 *      format can hold bathymetry data.
 *   2. The J-Star variant eliminates the SEGY EGCDIC and binary reel headers,
 *      and adds a message header to the beginning of each trace header.
 *      A J-Star stander format (JSF) file consists of a collection of trace
 *      records with the following components:
 *            1. A 16-byte message header.
 *            2. A 240 byte trace header.
 *            3. Trace data (2 bytes per sample)
 */

#ifndef MBSYS_JSTAR2_H_
#define MBSYS_JSTAR2_H_

/* specify the maximum number of sidescan pixels that can be returned
    by mbsys_jstar_extract() */
#define MBSYS_JSTAR_MESSAGE_SIZE 16
#define MBSYS_JSTAR_SBPHEADER_SIZE 240
#define MBSYS_JSTAR_SSHEADER_SIZE 240
#define MBSYS_JSTAR_SSOLDHEADER_SIZE 80
#define MBSYS_JSTAR_BATHYHEADER_SIZE 80
#define MBSYS_JSTAR_PIXELS_MAX 2000
#define MBSYS_JSTAR_SYSINFO_MAX 16384

#define MBSYS_JSTAR_DATA_SONAR                  80
#define MBSYS_JSTAR_DATA_SS                 82
#define MBSYS_JSTAR_DATA_SYSINFO                182
#define MBSYS_JSTAR_DATA_FILETIMESTAMP          426
#define MBSYS_JSTAR_DATA_FILEPADDING            428
#define MBSYS_JSTAR_DATA_NMEA                   2002
#define MBSYS_JSTAR_DATA_PITCHROLL              2020
#define MBSYS_JSTAR_DATA_PRESSURE               2060
#define MBSYS_JSTAR_DATA_DVL                    2080
#define MBSYS_JSTAR_DATA_SITUATION              2090
#define MBSYS_JSTAR_DATA_SITUATION2            2091
#define MBSYS_JSTAR_DATA_CABLE           2100
#define MBSYS_JSTAR_DATA_PIPE             2101
#define MBSYS_JSTAR_DATA_CONTAINER     2111

#define MBSYS_JSTAR_DATA_BATHYMETRICDATA        3000
#define MBSYS_JSTAR_DATA_BATHYMETRICATTITUDE    3001
#define MBSYS_JSTAR_DATA_BATHYMETRICPRESSURE    3002
#define MBSYS_JSTAR_DATA_BATHYMETRICALTITUDE    3003
#define MBSYS_JSTAR_DATA_BATHYMETRICPOSITION    3004
#define MBSYS_JSTAR_DATA_BATHYMETRICSTATUS    3005

#define MBSYS_JSTAR_DATA_COMMENT                17229

#define MBSYS_JSTAR_SUBSYSTEM_SBP 0
#define MBSYS_JSTAR_SUBSYSTEM_SSLOW 20
#define MBSYS_JSTAR_SUBSYSTEM_SSHIGH 21
#define MBSYS_JSTAR_SUBSYSTEM_SSVERYHIGH 22
#define MBSYS_JSTAR_SUBSYSTEM_RAW 100
#define MBSYS_JSTAR_SUBSYSTEM_PARSED 101

/* Edgetech trace data format definitions */
#define MBSYS_JSTAR_TRACEFORMAT_ENVELOPE 0     /* 2 bytes/sample (unsigned) */
#define MBSYS_JSTAR_TRACEFORMAT_ANALYTIC 1     /* 4 bytes/sample (I + Q) */
#define MBSYS_JSTAR_TRACEFORMAT_RAW 2          /* 2 bytes/sample (signed) */
#define MBSYS_JSTAR_TRACEFORMAT_REALANALYTIC 3 /* 2 bytes/sample (signed) */
#define MBSYS_JSTAR_TRACEFORMAT_PIXEL 4        /* 2 bytes/sample (signed) */

/* Data type definitions */
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

/* Message Header */
struct mbsys_jstar_header_struct {
	u16 start_marker; /* bytes 0-1,    Marker for the start of header (0x1601) */
	u8 version;           /* byte  2,      Version of protocol used */
	u8 session;           /* byte  3,      Session identifier */
	u16 type;              /* bytes 4-5,    Message type (80 - Acoustic Return Data) */
	u8 command;       /* bytes 6,      Command type
	                                  2 = Normal data source */
	u8 subsystem;      /* bytes 7,      Subsystem:
	                                  0 - subbottom
	                                 20 - 75 or 120 kHz sidescan
	                                 21 - 410 kHz sidescan
	                                 22 - 400 - 1600 kHz sidescan
	                                 100 - Raw Serial/UDP/TCP data
	                                 101 - Parsed Serial/UDP/TCP data */
	u8 channel;          /* bytes 8,      Channel for multi-channel systems
	                                 0 = port
	                                 1 = starboard */
	u8 sequence;       /* bytes 9,      Sequence number */
	u16 reserved;      /* bytes 10-11,  Reserved */
	u32 size;              /* bytes 12-15,  Size of following message in bytes */
};

/* Message Type 80: Sonar Data Message */
struct mbsys_jstar_sonarmessage_struct {
	/* Header Block */
	struct mbsys_jstar_header_struct header;

	/* Type 80:  Data Format Block */
    i32 unixtime;            /* bytes 0-3, Time since 1970 */
    u32 start_depth;      /* bytes 4-7, Starting Depth (window offset) in Samples */
    u32 ping_number;   /* bytes 8-11, Ping Number (increases with each ping) */
    i16 reserved[2];       /* bytes 12-15, Reserved */
    u16 msb;                 /* bytes 16-17, MSBs - Most Significant Bits - High order bits to extend 16 bits
                                         unsigned short values to 20 bits.
                                         Bits 0 - 3: Start Frequency
                                         Bits 4 - 7: End Frequency
                                         Bits 8 - 11: Samples in this Packet
                                         Bits 12 - 15: Mark Number (added in protocol version 0xA) */
    u16 lsb;                   /* bytes 18-19, LSB - Extended precision - Low order bits for fields requiring greater precision.
                                         Bits 0-7: Sample Interval- - Sample interval fractional component
                                         Bits 8-15: Course- - fractional portion of course (Added in protocol version 0xB) */
    u16 lsb2;                 /* bytes 20-21, LBS2 - Extended precision - Low order bits for fields requiring greater precision.
                                         Bits 0 – 3: Speed - sub fractional speed component (added in protocol version 0xC).
                                         Bits 4 – 13: Sweep Length in Microsecond, from 0 - 999 (added in protocol version 0xD).
                                         Bits 14 – 15: Reserved */
    i16 reserved2[3];     /* bytes 22-27, Reserved */
    i16 id_code;            /* bytes 28-29, ID Code (always 1 = Seismic Data) */
    u16 validity_flag;     /* bytes 30-31, Validity Flag - Validity flags bitmap:
                                         Bit 0: Lat Lon or XY valid
                                         Bit 1: Course valid
                                         Bit 2: Speed valid
                                         Bit 3: Heading valid
                                         Bit 4: Pressure valid
                                         Bit 5: Pitch roll valid
                                         Bit 6: Altitude valid
                                         Bit 7: Reserved
                                         Bit 8: Water temperature valid
                                         Bit 9 : Depth valid
                                         Bit 10: Annotation valid
                                         Bit 11: Cable counter valid
                                         Bit 12: KP valid
                                         Bit 13: Position interpolated
                                         Bit 14: Water sound speed valid */
    u16 reserved3;        /* bytes 32-33, Reserved */
    i16 data_format;      /* bytes 34-35, Data Format
                                         0 = one short per sample - envelope data. The total number of bytes of data to follow is 2 * samples.
                                         1 = two shorts per sample - stored as real (one short),
                                         imaginary (one short). The total number of bytes of data to follow is 4 * samples.
                                         2 =one short per sample - before matched filter. The total number of bytes of data to follow is 2 * samples.
                                         9 = two shorts per sample - stored as real (one short), imaginary (one short), - prior to matched filtering.
                                         This is the code for unmatched filtered analytic data, whereas value 1 is intended for match filtered analytic data.
                                         The total number of bytes of data to follow is 4 * samples. */
    i16 distance_antenna;       /* bytes 36-37, Distance from Antenna to Tow point in Centimeters. Sonar Aft is Positive */
    i16 distance_antenna2;     /* bytes 38-39, Distance from Antenna to Tow Point in Centimeters. Sonar to Starboard is Positive  */
    i16 reserved4[2];               /* bytes 40-43, Reserved */

    /* Type 80:  Navigation Data Block */
    f32 pipe_km;                /* bytes 44-47, Kilometers of Pipe. See Validity Flag (bytes 30 – 31). */
    i16 reserved5[16];        /* bytes 48-79, Reserved */
    i32 longitude;               /* bytes 80-83, Longitude in 10000 * (Minutes of Arc) or X in Millimeters or in Decimeters.
                                            See Validity Flag (bytes 30 – 31) and Coordinate Units (bytes 88 - 89). */
    i32 latitude;                  /* bytes 84-87, Latitude in 10000 * (Minutes of Arc) or Y in Millimeters or in Decimeters.
                                            See Validity Flag (bytes 30 – 31) and Coordinate Units (bytes 88 - 89). */
    i16 coordinates_units; /* bytes 88-89, Coordinate Units
                                            1 = X, Y in millimeters
                                            2 = Latitude, longitude in minutes of arc times 10000
                                            3 = X, Y in decimeters */

    /* Type 80:  Pulse Information Block */
    u8 annotation[24];          /* bytes 90-113, Annotation String (ASCII Data) */
    u16 samples;                 /* bytes 114-115, Samples */
    u32 sampling_internal;  /* bytes 116-119, Sampling Interval in Nanoseconds */
    u16 gain;                       /* bytes 120-121, Gain Factor of ADC */
    i16 transmit_level;         /* bytes 122-123, User Transmit Level Setting (0 – 100%) */
    i16 reserved6;               /* bytes 124-125, Reserved - Do not use */
    u16 start_frequency;     /* bytes 126-127, Transmit Pulse Starting Frequency in daHz (decaHertz, units of 10Hz) */
    u16 end_frequency;      /* bytes 128-129, Transmit Pulse Ending Frequency in daHz (decaHertz, units of 10Hz) */
    u16 sweep_length;       /* bytes 130-131, Sweep Length in Milliseconds */
    i32 pressure;                /* bytes 132-135, Pressure in Milli PSI (1 unit = 1/1000 PSI) */
    i32 depth;                     /* bytes 136-139, Depth in Millimeters (if not = 0) */
    u16 sample_frequency;  /* bytes 140-141, Sample Frequency of the Data in hertz */
    u16 outgoing_pulse;       /* bytes 142-143, Outgoing Pulse Identifier */
    i32 altitude;                     /* bytes 144-147, Altitude in Millimeters */
    f32 soundspeed;             /* bytes 148-151, Sound Speed in Meters per Second */
    f32 mixer_frequency;      /* bytes 152-155, Mixer Frequency in Hertz */

    /* Type 80:  CPU Time Block */
    i16 year;            /* bytes 156-157, Year Data Recorded (CPU time) e.g. 2009 */
    i16 day;             /* bytes 158-159, Day (1 – 366) (should not be used) */
    i16 hour;           /* bytes 160-161, Hour (see Bytes 200-203) (should not be used) */
    i16 minute;       /* bytes 162-163, Minute (should not be used) */
    i16 second;      /* bytes 164-165, Second (should not be used) */
    i16 basis;         /* bytes 166-167, Time Basis (always 3) */

    /* Type 80:  Weighting Factor Block */
    i16 weighting_factor;   /* bytes 168-169, Weighting Factor for Block Floating Point Expansion - defined as 2 to N Volts for LSB */
    i16 pulses_n;               /* bytes 170-171, Number of Pulses in the Water */

    /* Type 80:  Orientation Sensor Data Block */
    u16 compass_heading;    /* bytes 172-173, Compass Heading (0 to 359.99) in units of 1/100 Degree */
    i16 pitch;                           /* bytes 174-175, Pitch [(degrees / 180.0) * 32768.0] maximum resolution */
    i16 roll;                              /* bytes 176-177, Roll [(degrees / 180.0) * 32768.0] maximum resolution */
    i16 reserved6;                   /* bytes 178-179, Reserved */

    /* Type 80:  Trigger Information Block */
    i16 reserved7;              /* bytes 180-181, Reserved */
    i16 trigger_source;       /* bytes 182-183, Trigger Source
                                            0 = Internal
                                            1 = External
                                            2 = Coupled */
    u16 mark_n;                 /* bytes 184-185, Mark Number
                                            0 = No Mark */

    /* Type 80:  NMEA Navigation Data Block */
    i16 position_hour;          /* bytes 186-187, Position Fix Hour (0 – 23) */
    i16 position_minutes;    /* bytes 188-189, Position Fix Minutes (0 – 59) */
    i16 position_seconds;   /* bytes 190-191, Position Fix Seconds (0 – 59) */
    i16 course;                    /* bytes 192-193, Course in Degrees (0 to 359.9) */
    i16 speed;                     /* bytes 194-195, Speed – in Tenths of a Knot */
    i16 position_day;           /* bytes 196-197, Position Fix Day (1 – 366) */
    i16 position_year;          /* bytes 198-199, Position Fix Year */

    /* Type 80:  Miscellaneous Data Block */
    u32 miliseconds_today;          /* bytes 200-203, Milliseconds Today (Since Midnight) */
    u16 max_adc;                         /* bytes 204-205, Maximum Absolute Value of ADC Samples in this Packet */
    i16 reserved8;                         /* bytes 206-207, Reserved */
    i16 reserved9;                         /* bytes 208-209, Reserved */
    i8 sonar_version[6];                /* bytes 210-215, Sonar Software Version Number - ASCII */
    i32 spherical_correction;        /* bytes 219-219, Initial Spherical Correction Factor in Samples times 100 */
    u16 packet_number;              /* bytes 220-221, Packet Number */
    i16 decimation_adc;               /* bytes 222-223, ADC Decimation * 100 times */
    i16 reserved10;                      /* bytes 224-225, Reserved */
    i16 water_temperature;          /* bytes 226-227, Water Temperature in Units of 1/10 Degree C */
    f32 layback;                            /* bytes 228-231, Layback */
    i32 reserved11;                       /* bytes 232-235, Reserved */
    u16 cableout;                          /* bytes 236-237, Cable Out in Decimeters */
    u16 reserved12;                      /* bytes 238-239, Reserved */

	/* -------------------------------------------------------------------- */
	/* MB-System-only parameters from ???-???               */
	/* -------------------------------------------------------------------- */
	//int depth;         /* 227-231 : Seafloor depth in 0.001 m */
	//int sensordepth;    /* 232-235 : Sonar depth in 0.001 m */
	//int sonaraltitude; /* 236-239 : Sonar altitude in 0.001 m */

	/* trace data stored as shorts */
	unsigned int trace_alloc;
	unsigned short *trace;
};

/* Message Type 82: Side Scan Data Message */
struct mbsys_jstar_ssmessage_struct {
	/* Header Block */
	struct mbsys_jstar_header_struct header;

	/* Sidescan Data Block */
    u16 subsystem;               /* bytes 0-1, The subsystem number determines the source of data; common subsystem assignment are:
                                                Sub-Bottom (SB) = 0
                                                Low frequency data of a dual frequency side scan = 20
                                                High frequency data of a dual frequency side scan = 21
                                                Very High frequency data of a tri-frequency side scan = 22
                                                Raw Serial/UDP/TCP data = 100
                                                Parsed Serial/UDP/TCP data = 101 */
    u16 channel;                    /* bytes 2-3, Channel for a Multi-Channel Subsystem
                                                For Side Scan Subsystems:
                                                    0 = Port
                                                    1 = Starboard
                                                For Serial Ports: this is the logical port number, which often differs from physical COM Port in use.
                                                Single Channel Sub-Bottom systems channel is 0. */
    u32 ping_number;           /* bytes 4-7, Ping Number (increments with each ping period) */
    u16 packet_number;       /* bytes 8-9, Packet Number (1..n, each ping starts with packet 1) */
    u16 trigger_source;         /* bytes 10-11, Trigger Source (0 = internal, 1 = external) */
    u32 samples_packet;      /* bytes 12-15, Samples in this Packet */
    u32 sample_interval;       /* bytes 16-19, Sample Interval in Nanoseconds of Stored Data */
    u32 start_depth;              /* bytes 20-23, Starting Depth (window offset) in Samples */
    i16 weighting_factor;       /* bytes 24-25, Weighting Factor (defines 2 to N Volts) */
    u16 gain_adc;                 /* bytes 26-27, Gain Factor of ADC */
    u16 max_adc;                 /* bytes 28-29, Maximum Absolute Value for ADC Samples for this Packet */
    u16 range;                      /* bytes 30-31, Range Setting (in decameters, meters times 10) */
    u16 pulse_id;                  /* bytes 32-33, Unique Pulse Identifier */
    u16 mark_n;                   /* bytes 34-35, Mark Number (0 = no mark) */
    u16 data_format;            /* bytes 36-37, Data Format
                                                0 = one short per sample - envelope data the total number of bytes of data to follow is 2 * samples
                                                1 = two shorts per sample - stored as real (one short), imaginary (one short), the total number of bytes
                                                of data to follow is 4 * samples */
    u8 multiping_n;               /* byte 38, Number of Simultaneous Pulses in the Water */
    u8 reserved;                   /* byte 39, Reserved */

	/* Computer Data / Time Data Block */
	u32 miliseconds_today;     /* bytes 40-43, Milliseconds Today */
	i16 year;                             /* bytes 44-45, Year */
	u16 day_year;                    /* bytes 46-47, Day of year (1 – 366) */
	u16 hour_day;                    /* bytes 48-49, Hour of day (0 – 23) */
	u16 minute;                        /* bytes 50-51, Minute (0 – 59) */
	u16 second;                       /* bytes 52-53, Second (0 – 59) */

	/* Auxiliary Sensor Information Block */
	u16 compass_heading;    /* bytes 54-55, Compass Heading in Minutes (0 – 359.9) x 60 */
    i16 pitch;                           /* bytes 56-57, Pitch (scale by 180 / 32768 to get degrees, bow up is positive) */
    i16 roll;                              /* bytes 58-59, Roll (scale by 180 / 32768 to get degrees, port up is positive) */
    i16 heave;                         /* bytes 60-61, Heave in Centimeters */
    i16 yaw;                            /* bytes 62-63, Yaw in Minutes */
    u32 pressure;                   /* bytes 64-67, Pressure in Units of 1/1000 PSI */
    i16 temperature;               /* bytes 68-69, Temperature in Units of 1/10 of a Degree Celsius */
    i16 reserved2;                  /* bytes 70-71, Reserved */
    i32 altitude;                       /* bytes 72-75, Altitude in Millimeters (or -1 if no valid reading) */
    u8 reserved3[4]                /* bytes 76-79, Reserved */

	/* trace data stored as shorts */
	unsigned int trace_alloc;
	unsigned short *trace;
};

/* Message Type 182: System Information */
struct mbsys_jstar_sysinfo_struct {
	/* Message Header */
	struct mbsys_jstar_header_struct header;

	/* System Information */
	i32 system_type; /* bytes 0-3, System Type Number and Description:
	                                   1       2xxx Series, Combined Sub-Bottom / Side Scan with SIB Electronics
	                                   2       2xxx Series, Combined Sub-Bottom / Side Scan with FSIC Electronics
	                                   4       4300-MPX (Multi-Ping)
	                                   5       3200-XS, Sub-Bottom Profiler with AIC Electronics
	                                   6       4400-SAS, 12-Channel Side Scan
	                                   7       3200-XS, Sub Bottom Profiler with SIB Electronics
	                                  11      4200 Limited Multipulse Dual Frequency Side Scan
	                                  14      3100-P, Sub Bottom Profiler
	                                  16      2xxx Series, Dual Side Scan with SIB Electronics
	                                  17      4200 Multipulse Dual Frequency Side Scan
	                                  18      4700 Dynamic Focus
	                                  19      4200 Dual Frequency Side Scan
	                                  20      4200 Dual Frequency non Simultaneous Side Scan
	                                  21      2200-MP Combined Sub-Bottom / Dual Frequency Multipulse Side Scan
	                                  23      4600 Bathymetric System
	                                128      4100, 272 /560A Side Scan */
	i32 lowrate_io;     /* bytes 4-7, Low Rate I/O Enabled Option (0 = disabled) */
	i32 version;          /* bytes 8-11, Version Number of Sonar Software used to Generate Data */
	i32 subsystems_n;               /* bytes 12-15, Number of Subsystems Present in this Message */
	i32 serialportdevices_n;       /* bytes 16-19, Number of Serial Port Devices Present in this Message */
	i32 towserialnumber;            /* bytes 20-23, Number of Serial Port Devices Present in this Message */

	/* Sysinfo message */
	int sysinfosize;
	c8 sysinfo[MBSYS_JSTAR_SYSINFO_MAX];  /* bytes 24-End, Reserved */
};

/* Message Type 426: File Timestamp Message */
struct mbsys_jstar_filetimestamp_struct {
	/* Message Header */
	struct mbsys_jstar_header_struct header;

    /* time since 1/1/1970 */
    i32 seconds;            /* bytes 0-3, Time in Seconds since 1/1/1970 */
    i32 milliseconds;      /* bytes 4-7, Milliseconds in the Current Second */
};

/* Message Type 428: File Padding Message */
/* A file padding message is sometimes found at the end of the file. In some implementations files are padded to optimize the write process.
    These messages should be ignored. */

/* Message Type 2002: NMEA String */
struct mbsys_jstar_nmea_struct {
	/* Message Header */
	struct mbsys_jstar_header_struct header;

	/* Time and source */
	i32 seconds;            /* bytes 0-3, Time in Seconds since 1/1/1970 */
	i32 milliseconds;      /* bytes 4-7, Milliseconds in the Current Second */
	u8 source;               /* bytes 8, Source
	                                    1 = Sonar
                                        2 = Discover
                                        3 = ETSI */
	u8 reserved[3];        /* bytes 9-11, Reserved */

	/* NMEA string */
	c8 nmea[MB_COMMENT_MAXLINE];  /* bytes 12-End, NMEA String Data */
};

/* Message Type 2020: Pitch Roll Data */
struct mbsys_jstar_pitchroll_struct {
	/* Message Header */
	struct mbsys_jstar_header_struct header;

	/* Time */
	i32 seconds;          /* bytes 0-3, Time in Seconds since 1/1/1970 */
	i32 milliseconds;    /* bytes 4-7, Milliseconds in the Current Second */
	u8 reserve1[4];       /* bytes 8-11, Reserved */

    /* attitude data */
	i16 accelerationx;       /* bytes 12-13, X acceleration: multiply by (20 * 1.5) / (32768) to get G's */
	i16 accelerationy;       /* bytes 14-15, Y acceleration: multiply by (20 * 1.5) / (32768) to get G's */
	i16 accelerationz;       /* bytes 16-17, Z acceleration: multiply by (20 * 1.5) / (32768) to get G's */
	i16 gyroratex;             /* bytes 18-19, X gyro rate: multiply by (500 * 1.5) / (32768) to get deg/sec */
	i16 gyroratey;             /* bytes 20-21, Y gyro rate: multiply by (500 * 1.5) / (32768) to get deg/sec */
	i16 gyroratez;             /* bytes 22-23, Z gyro rate: multiply by (500 * 1.5) / (32768) to get deg/sec */
	i16 pitch;                    /* bytes 24-25, Pitch: multiply by (180.0 / 32768.0) to get degrees. Bow up is positive */
	i16 roll;                       /* bytes 26-27, Roll: multiply by (180.0 / 32768) to get degrees. Port up is positive */
	i16 temperature;        /* bytes 28-29, Temperature in Units of 1/10 of a Degree Celsius */
	u16 deviceinfo;          /* bytes 30-31, Device specific info. This is device specific info provided for Diagnostic purposes */
	i16 heave;                  /* bytes 32-33, Estimated Heave in Millimeters. Positive is Down */
	u16 heading;              /* bytes 34-35, Heading in units of 0.01 Degrees (0...360) */
	i32 datavalidflags;      /* bytes 36-39, Data Validity Flags
                                            Bit 0: Ax
                                            Bit 1: Ay
                                            Bit 2: Az
                                            Bit 3: Rx
                                            Bit 4: Ry
                                            Bit 5: Rz
                                            Bit 6: Pitch
                                            Bit 7: Roll
                                            Bit 8: Heave
                                            Bit 9: Heading
                                            Bit 10: Temperature
                                            Bit 11: Device Info
                                            Bit 12: Yaw */
	i16 yaw;                    /* bytes 40-41, Yaw in units of 0.01 Degrees (0...360) */
	i16 reserved;            /* bytes 42-42, Reserved */
};

/* Message Type 2060: Pressure Sensor Reading */
struct mbsys_jstar_pressure_struct {
	/* Message Header */
	struct mbsys_jstar_header_struct header;

	/* Time */
	i32 seconds;            /* bytes 0-3, Time in Seconds since 1/1/1970 */
	i32 milliseconds;      /* bytes 4-7, Milliseconds in the Current Second */
	u8 reserve1[4];        /* bytes 8-11, Reserved */

    /* CTD data */
	i32 pressure;           /* bytes 12-15, Pressure in Units of 1/1000th of a PSI */
	i32 temperature;      /* bytes 16-19, Temperature in Units of 1/1000th of Degree Celsius */
	i32 salinity;              /* bytes 20-23, Salinity in Parts Per Million */
	i32 datavalidflags;   /* bytes 24-27, Validity Data Flag:
                                        Bit 0: Pressure
                                        Bit 1: Temperature
                                        Bit 2: Salt PPM
                                        Bit 3: Conductivity
                                        Bit 4: Sound velocity
                                        Bit 5: Depth */
	i32 conductivity;       /* bytes 28-31, Conductivity in Micro-Siemens per Centimeter */
	i32 soundspeed;      /* bytes 32-35, Velocity of Sound in Millimeters per Second */
	i32 depth;                 /* bytes 36-39, Depth in Meters */
	i32 reserve2[9];        /* bytes 40-75, Reserved */
};

/* Message Type 2080: Doppler Velocity Log Data */
struct mbsys_jstar_dvl_struct {
	/* Message Header */
	struct mbsys_jstar_header_struct header;

	/* Time */
	i32 seconds;           /* bytes 0-3, Time in Seconds since 1/1/1970 */
	i32 milliseconds;     /* bytes 4-7, Milliseconds in the Current Second */
	u8 reserved1[4];     /* bytes 8-11, Reserved */

    /* dvl data */
	u32 datavalidflags; /* bytes 12-15, Validity Data Flags :
                                        Bit 0: X, Y Velocity Present
                                        Bit 1: 0 = Earth Coordinates, 1= Ship coordinates Bit 2: Z (Vertical Velocity) Present
                                        Bit 3: X, Y Water Velocity Present
                                        Bit 4: Z (Vertical Water Velocity) Present
                                        Bit 5: Distance to Bottom Present
                                        Bit 6: Heading Present
                                        Bit 7: Pitch Present
                                        Bit 8: Roll Present
                                        Bit 9: Temperature Present
                                        Bit 10: Depth Present
                                        Bit 11: Salinity Present
                                        Bit 12: Sound Velocity Present
                                        Bit 31: Error Detected
                                        Rest : Reserved, Presently 0 */
	i32 beam1range;              /* bytes 16-19, distance to bottom in centimeters (0 = invalid or non-existing reading) */
	i32 beam2range;              /* bytes 20-23, distance to bottom in centimeters (0 = invalid or non-existing reading) */
	i32 beam3range;              /* bytes 24-27, distance to bottom in centimeters (0 = invalid or non-existing reading) */
	i32 beam4range;              /* bytes 28-31, distance to bottom in centimeters (0 = invalid or non-existing reading) */
	i16 velocitybottomx;         /* bytes 32-33, X velocity wrt bottom (0.001 m/s, positive to starboard or east) */
	i16 velocitybottomy;         /* bytes 34-35, Y velocity wrt bottom (0.001 m/s, positive to forward or north) */
	i16 velocitybottomz;         /* bytes 36-37, Z velocity wrt bottom (0.001 m/s, positive upward) */
	i16 velocitywaterx;           /* bytes 38-39, X velocity wrt water (0.001 m/s, positive to starboard or east) */
	i16 velocitywatery;           /* bytes 40-41, Y velocity wrt water (0.001 m/s, positive to forward or north) */
	i16 velocitywaterz;           /* bytes 42-43, Z velocity wrt water (0.001 m/s, positive upward) */
	u16 depth;                       /* bytes 44-45, Depth from Depth Sensor in Decimeters */
	i16 pitch;                          /* bytes 46-47, Pitch (0.01 degree (-180 to +180), positive bow up) */
	i16 roll;                             /* bytes 48-49, Roll (0.01 degree (-180 to +180), positive port up) */
	u16 heading;                    /* bytes 50-51, Heading in units of 0.01 of a Degree (0 to 360) */
	u16 salinity;                      /* bytes 52-53, Salinity in 1 Part Per Thousand */
	i16 temperature;               /* bytes 54-55, Temperature in units of 1/100 of a degree Celsius */
	i16 soundspeed;               /* bytes 56-57, Sound Velocity in Meters per Second */
	i16 reserved2[7];               /* bytes 58-71, Reserved */
};

/* Message Type 2090: Situation Message */
struct mbsys_jstar_situation_struct {
	/* Message Header */
	struct mbsys_jstar_header_struct header;

	/* Time */
	i32 seconds;            /* bytes 0-3, Time in Seconds since 1/1/1970 */
	i32 milliseconds;      /* bytes 4-7, Milliseconds in the Current Second */
	u8 reserve1[4];        /* bytes 8-11, Reserved */

    /* navigation and attitude data */
	u32 datavalidflags; /* bytes 12-15, Validity Data Flags:
                                        Bit 0: Microsecond Time stamp
                                        Bit 1: Latitude
                                        Bit 2: Longitude
                                        Bit 3: Depth
                                        Bit 4: Heading
                                        Bit 5: Pitch
                                        Bit 6: Roll
                                        Bit 7: X Relative Position
                                        Bit 8: Y Relative Position
                                        Bit 9: Z Relative Position
                                        Bit 10: X Velocity
                                        Bit 11: Y Velocity
                                        Bit 12: Z Velocity
                                        Bit 13: North Velocity
                                        Bit 14: East Velocity
                                        Bit 15: Down Velocity
                                        Bit 16: X Angular Rate
                                        Bit 17: Y Angular Rate
                                        Bit 18: Z Angular Rate
                                        Bit 19: X Acceleration
                                        Bit 20: Y Acceleration
                                        Bit 21: Z Acceleration
                                        Bit 22: Latitude Standard Deviation
                                        Bit 23: Longitude Standard Deviation
                                        Bit 24: Depth Standard Deviation
                                        Bit 25: Heading Standard Deviation
                                        Bit 26: Pitch Standard Deviation
                                        Bit 27: Roll Standard Deviation */
	u8 reserve2[4];              /* bytes 16-19, Reserved */
    u64 time_usec;             /* bytes 20-27, Microsecond timestamp, us since 12:00:00 am GMT, January 1, 1970 */
    f64 latitude;                   /* bytes 28-35, Latitude in degrees, north is positive */
    f64 longitude;                /* bytes 36-43, Longitude in degrees, east is positive */
    f64 depth;                     /* bytes 44-51, Depth in meters */
    f64 heading;                 /* bytes 52-59, Heading in degrees */
    f64 pitch;                      /* bytes 60-67, Pitch in degrees, bow up is positive */
    f64 roll;                         /* bytes 68-75, Roll in degrees, port up is positive */
    f64 x_forward;              /* bytes 76-83, X, forward, relative position in meters, surge */
    f64 y_starboard;           /* bytes 84-91, Y, starboard, relative position in meters, sway */
    f64 z_downward;          /* bytes 92-99, Z downward, relative position in meters, heave */
    f64 velocity_x_forward;     /* bytes 100-107, X, forward, velocity in meters per second */
    f64 velocity_y_starboard;  /* bytes 108-115, Y, starboard, velocity in meters per second */
    f64 velocity_z_downward; /* bytes 116-123, Z, downward, velocity in meters per second */
    f64 velocity_north;         /* bytes 124-131, North velocity in meters per second */
    f64 velocity_east;          /* bytes 132-139, East velocity in meters per second */
    f64 velocity_down;        /* bytes 140-147, Down velocity in meters per second */
    f64 angular_rate_x;      /* bytes 148-155, X angular rate in degrees per second, port up is positive */
    f64 angular_rate_y;      /* bytes 156-163, Y angular rate in degrees per second, bow up is positive */
    f64 angular_rate_z;      /* bytes 164-171, Z angular rate in degrees per second, starboard is positive */
    f64 acceleration_x;       /* bytes 172-179, X, forward, acceleration in meters per second per second */
    f64 acceleration_y;       /* bytes 180-187, Y, starboard, acceleration in meters per second per second */
    f64 acceleration_z;       /* bytes 188-195, Z, downward, acceleration in meters per second per second */
    f64 latitude_sigma;       /* bytes 196-203, Latitude standard deviation in meters */
    f64 longitude_sigma;    /* bytes 204-211, Longitude standard deviation in meters */
    f64 depth_sigma;         /* bytes 212-219, Depth standard deviation in meters */
    f64 heading_sigma;     /* bytes 220-227, Heading standard deviation in degrees */
    f64 pitch_sigma;          /* bytes 228-235, Pitch standard deviation in degrees */
    f64 roll_sigma;             /* bytes 236-243, Roll standard deviation in degrees */
    u16 reserved3[16];      /* bytes 244-275, Reserved – Do not use */
};

/* Message Type 2091: Situation Message - Version 2 */
struct mbsys_jstar_situation2_struct {
	/* Message Header */
	struct mbsys_jstar_header_struct header;

	/* Time */
	i32 seconds;          /* bytes 0-3, Time in Seconds since 1/1/1970 */
	i32 milliseconds;    /* bytes 4-7, Milliseconds in the Current Second */
	u8 reserve1[4];      /* bytes 8-11, Reserved */

    /* navigation and attitude data */
	u32 datavalidflags; /* bytes 12-15, Validity Flag:
                                        Bit 0 : Timestamp Provided by the Source Valid
                                        Bit 1: Longitude Valid
                                        Bit 2: Latitude Valid
                                        Bit 3: Depth Valid
                                        Bit 4: Altitude Valid
                                        Bit 5: Heave Valid
                                        Bit 6: Velocity 1 & 2 Valid
                                        Bit 7: Velocity down Valid
                                        Bit 8: Pitch Valid
                                        Bit 9 : Roll Valid
                                        Bit 10: Heading Valid
                                        Bit 11: Sound Speed Valid
                                        Bit 12: Water Temperature Valid
                                        Others: Reserved, Presently 0 */
    u8 velocity12;         /* byte 16, Velocity12 Directions (Velocity1 and Velocity2 Types):
                                        0 = North and East,
                                        1 = Forward and Starboard,
                                        2 = +45 Degrees Rotated from Forward */
	u8 reserve2[3];       /* bytes 17-19, Reserved */
    u64 time_usec;      /* bytes 20-27, Timestamp (0.01 of a microsecond)
                                        Microsecond since 12:00:00AM GST, January 1, 1970. To get seconds since 1970 divide by 1e7 */
    f64 latitude;           /* bytes 28-35, Latitude in degrees, north is positive */
    f64 longitude;        /* bytes 36-43, Longitude in degrees, east is positive */
    f32 depth;              /* bytes 44-47, Depth in Meter  (Below Water Surface) */
    f32 altitude;           /* bytes 48-51, Altitude in Meter (Above Seafloor) */
    f32 heave;             /* bytes 52-55, Heave in Meter (Positive is Down) */
    f32 velocity1;         /* bytes 56-59, Velocity1 in Meters per Second (North Velocity or Forward) */
    f32 velocity2;         /* bytes 60-63, Velocity2 in Meters per Second (East Velocity or Starboard) */
    f32 velocity_down;   /* bytes 64-67, Velocity Down in Meter per Second (Down Velocity) */
    f32 pitch;                  /* bytes 68-71, Pitch in degrees, bow up is positive */
    f32 roll;                     /* bytes 72-75, Roll in degrees, port up is positive */
    f32 heading;             /* bytes 76-79, Heading in degrees (0 to 359.9) */
    f32 soundspeed;      /* bytes 80-83, Sound Speed in Meters per Second */
    f32 water_temperature;      /* bytes 84-87, Water Temperature (in Degrees Celsius) */
    f32 reserved3[3];   /* bytes , Reserved – Do not use */
};

/* Message Type 2100: Cable Counter Data Message */
struct mbsys_jstar_cable_struct {
	/* Message Header */
	struct mbsys_jstar_header_struct header;

    /* Cable Counter Data */
	i32 seconds;                /* bytes 0-3, Time in Seconds since 1/1/1970 */
	i32 milliseconds;          /* bytes 4-7, Milliseconds in the Current Second */
	u8 reserve1[4];            /* bytes 8-11, Reserved – Do Not Use */
	f32 cable_length;        /* bytes 12-15, Cable Length in Meters */
	f32 cable_speed;        /* bytes 16-19, Cable Speed in Meters per Second */
	i16 cable_lengthflag;       /* bytes 20-21, Cable Length Valid Flag (0 – Invalid) */
	i16 cable_speedflag;       /* bytes 22-23, Cable Speed Valid Flag (0 – Invalid) */
	i16 cable_countererror;   /* bytes 24-25, Cable Counter Error (0 – No Error) */
	i16 cable_tensionflag;     /* bytes 26-27, Cable Tension Valid Flag (0 – Invalid) */
	f32 cable_tension;           /* bytes 28-31, Cable Tension in Kilograms */
};

/* Message Type 2101: Kilometer of Pipe Data */
struct mbsys_jstar_pipe_struct {
	/* Message Header */
	struct mbsys_jstar_header_struct header;

    /* Pipe Data */
	i32 seconds;            /* bytes 0-3, Time in Seconds since 1/1/1970 */
	i32 milliseconds;      /* bytes 4-7, Milliseconds in the Current Second */
    u8 source;               /* byte 8, Source
                                        1 = Sonar
                                        2 = DISCOVER
                                        3 = ETSI */
    u8 reserved[3];        /* bytes 9-11, Reserved */
    f32 km_pipe;           /* bytes 12-15, Kilometer of Pipe (KP) */
    i16 kp_value;          /* bytes 16-17, Flag (Valid KP Value) */
    i16 kp_error;           /* bytes 18-19, Flag (KP Report Error) */
};

/* Message Type 2111: Container Timestamp Message */
struct mbsys_jstar_container_struct {
	/* Message Header */
	struct mbsys_jstar_header_struct header;

    /* Container Timestamp Data */
	i32 seconds;            /* bytes 0-3, Time in Seconds since 1/1/1970 */
	i32 milliseconds;      /* bytes 4-7, Milliseconds in the Current Second */
    u8 reserved[4];        /* bytes 8-11, Reserved */
};

/* Message Type 3000: Header Description */
struct mbsys_jstar_headerbathy_struct {
    u32 seconds;                        /* bytes 0-3, Time Since 1/1/1970 in seconds */
    u32 nanoseconds;                /* bytes 4-7, Nanosecond Supplement to Time */
    u32 ping_number;                /* bytes 8-11, Ping Number */
    u16 bathysamples;               /* bytes 12-13, Number of BathymetricSampleType Entries */
    u8 channel;                           /* byte 14, Channel (0 – port, 1 – starboard) */
    u8 algorithm_type;                /* byte 15, Algorithm Type */
    u8 pulse_number;                 /* byte 16, Number of Pulses */
    u8 pulse_phase;                   /* byte 17, Pulse Phase */
    u16 pulse_length;                 /* bytes 18-19, Pulse Length in milliseconds */
    f32 pulse_amplitude;            /* bytes 20-23, Transmit Pulse Amplitude (0 to 1) */
    f32 chirp_startfrequency;     /* bytes 24-27, Chirp Start Frequency in Hertz */
    f32 chirp_endfrequency;      /* bytes 28-31, Chirp End Frequency in Hertz */
    f32 mixer_frequency;           /* bytes 32-35, Mixer Frequency in Hertz */
    f32 sample_rate;                  /* bytes 36-39, Sample Rate in Hertz */
    u32 offset_sample;              /* bytes 40-43, Offset to First Sample in Nanoseconds */
    f32 timedelay_uncertainty;  /* bytes 44-47, Time Delay Uncertainty in Seconds */
    f32 timescale_factor;           /* bytes 48-51, Time Scale Factor in Seconds */
    f32 timescale_accuracy;      /* bytes 52-55, Time Scale Accuracy in percentage */
    u32 anglescale_factor;        /* bytes 56-59, Angle Scale Factor in Degrees */
    u32 reserved;                      /* bytes 60-63, Reserved */
    u32 time_bottom;                /* bytes 64-67, Time to First Bottom Return in Nanoseconds */
    u8 format_revision;             /* byte 68, Format Revision Level (0 to 4) */
    u8 binning_flag;                  /* byte 69, Binning Flag (0 to 2) */
    u8 tvg;                                 /* byte 70, TVG db/100m */
    u8 reserved;                        /* byte 71, Reserved */
    f32 span;                             /* bytes 72-75, Span in Meter or Degrees */
    u32 reserved2;                   /* bytes 76-79, Reserved */

    u16 time_delay;                 /* bytes ,  */
    i16 angle;
    u8 amplitude;
    u8 angle_uncertainty;
    u8 flag;
    u32 snr;
    u32 quality;
};

/* Message Type 3001: Attitude Message Type */
struct mbsys_jstar_attitudebathy_struct {
	/* Bathy Message Header */
	struct mbsys_jstar_headerbathy_struct headerbathy;

    /* Attitude Data */
	u32 seconds;                /* bytes 0-3, Time Since 1/1/1970 in Seconds */
	u32 nanoseconds;        /* bytes 4-7, Nanosecond Supplement to Time in Nanoseconds */
    u32 valid_flag;              /* bytes 8-11, Data Valid Flag: 0 - clear, 1 - set
                                                    Bit 0: Heading
                                                    Bit 1: Heave
                                                    Bit 2: Pitch
                                                    Bit 3: Roll
                                                    Bit 4: Yaw  */
    f32 heading;                 /* bytes 12-15, Heading (0 to 359.9) */
    f32 heave;                    /* bytes 16-19, Heave in Meters */
    f32 pitch;                      /* bytes 20-23, Pitch in Degrees */
    f32 roll;                         /* bytes 24-27, Roll in Degrees */
    f32 yaw;                       /* bytes 28-31, Yaw in Degrees */
};

/* Message Type 3002: Pressure Message Type */
struct mbsys_jstar_pressurebathy_struct {
	/* Bathy Message Header */
	struct mbsys_jstar_headerbathy_struct headerbathy;

    /* Pressure Data */
	u32 seconds;                    /* bytes 0-3, Time Since 1/1/1970 in Seconds */
	u32 nanoseconds;            /* bytes 4-7, Nanosecond Supplement to Time in Nanoseconds */
    u32 valid_flag;                  /* bytes 8-11, Data Valid Flag: 0 - clear, 1 - set
                                                    Bit 0: Pressure
                                                    Bit 1: Water Temperature
                                                    Bit 2: Salinity
                                                    Bit 3: Conductivity
                                                    Bit 4: Sound Velocity
                                                    Bit 5: Depth */
    f32 absolute_pressure;     /* bytes 12-15, Absolute Pressure in PSI */
    f32 water_temperature;    /* bytes 16-19, Water Temperature in Degrees */
    f32 salinity;                       /* bytes 20-23, Salinity in PPM */
    f32 conductivity;               /* bytes 24-27, Conductivity in Degrees */
    f32 sound_velocity;          /* bytes 28-31, Sound Velocity in Meters per Second */
    f32 depth;                         /* bytes 32-35, Depth in Meters */
};

/* Message Type 3003: Altitude Message Type */
struct mbsys_jstar_altitudebathy_struct {
	/* Bathy Message Header */
	struct mbsys_jstar_headerbathy_struct headerbathy;

    /* Altitude Data */
	u32 seconds;                /* bytes 0-3, Time Since 1/1/1970 in Seconds */
	u32 nanoseconds;        /* bytes 4-7, Nanosecond Supplement to Time in Nanoseconds */
    u32 valid_flag;              /* bytes 8-11, Data Valid Flag: 0 - clear, 1 - set
                                                    Bit 0: Altitude
                                                    Bit 1: Speed
                                                    Bit 2: Heading */
    f32 altitude;                   /* bytes 12-15, Altitude in Meters */
    f32 speed;                     /* bytes 16-19, Speed in Knots */
    f32 heading;                  /* bytes 20-23, Heading (0 to 359.9) in Degrees */
};

/* Message Type 3004: Position Message Type */
struct mbsys_jstar_positionbathy_struct {
	/* Bathy Message Header */
	struct mbsys_jstar_headerbathy_struct headerbathy;

    /* Position Data */
	u32 seconds;                    /* bytes 0-3, Time Since 1/1/1970 in Seconds */
	u32 nanoseconds;            /* bytes 4-7, Nanosecond Supplement to Time in Nanoseconds */
    u16 valid_flag;                  /* bytes 8-9, Data Valid Flag: 0 - clear, 1 - set
                                                    Bit 0: UTM Zone
                                                    Bit 1: Easting
                                                    Bit 2: Northing
                                                    Bit 3: Latitude
                                                    Bit 4: Longitude
                                                    Bit 5: Speed
                                                    Bit 6: Heading
                                                    Bit 7: Antenna Height */
    u16 utm_zone;                 /* bytes 10-11, UTM Zone */
    f64 easing;                       /* bytes 12-19, Easting in Meters */
    f64 northing;                     /* bytes 20-27, Northing in Meters */
    f64 latitude;                      /* bytes 28-35, Latitude in Degrees (North is positive) */
    f64 longitude;                   /* bytes 36-43, Longitude in Degrees (East is positive) */
    f32 speed;                        /* bytes 44-47, Speed in Knots */
    f32 heading;                     /* bytes 48-51, Heading (0 to 359.9, is always positive) */
    f32 antenna_height;         /* bytes 52-55, Antenna Height in Meters (positive up) */
};

/* Message Type 3005: Status Message Type */
struct mbsys_jstar_statusbathy_struct {
	/* Bathy Message Header */
	struct mbsys_jstar_headerbathy_struct headerbathy;

    /* Position Data */
	u32 seconds;                    /* bytes 0-3, Time Since 1/1/1970 in Seconds */
	u32 nanoseconds;            /* bytes 4-7, Nanosecond Supplement to Time in Nanoseconds */
    u16 valid_flag;                  /* bytes 8-9, Data Valid Flag: 0 - clear, 1 - set
                                                    Bit 0: GGA Status
                                                    Bit 1: GGK Status
                                                    Bit 2: Number of Satellites
                                                    Bit 3: Dilution of Precision */
    u8 version;                       /* byte 10, Version */
    u8 gga_status;                 /* byte 11, GGA Status */
    u8 ggk_status;                 /* byte 12, GGK Status */
    u8 satellites_n;                /* byte 13, Number of Satellites */
    u8 reserved;                    /* bytes 14-15, Reserved */
    f32 precision_dilution;     /* bytes 16-19, Dilution of Precision */
    u32 reserved2[11];          /* bytes 20-63, Reserved */
};

struct mbsys_jstar_struct {
	int kind; /* MBIO data kind */

	/* Ping type */
	u8 subsystem; /* bytes 7,      Subsystem:
	                          0 - subbottom
	                         20 - 75 or 120 kHz sidescan
	                         21 - 410 kHz sidescan */

	/* SBP data */
	struct mbsys_jstar_channel_struct sbp;

	/* Sidescan data */
	struct mbsys_jstar_channel_struct ssport;
	struct mbsys_jstar_channel_struct ssstbd;

	/* Bathymetry data */
	struct mbsys_jstar_channel_struct bathy;

	/* System Information data */
	struct mbsys_jstar_sysinfo_struct sysinfo;

    /* File timestamp data */
    struct mbsys_jstar_filetimestamp_struct filetimestamp;

	/* NMEA */
	struct mbsys_jstar_nmea_struct nmea;

	/* Comment */
	struct mbsys_jstar_comment_struct comment;

	/* Pitch Roll data */
	struct mbsys_jstar_pitchroll_struct pitchroll;

	/* Pressure data */
	struct mbsys_jstar_pressure_struct pressure;

	/* DVL data */
	struct mbsys_jstar_dvl_struct dvl;

	/* Situation data */
	struct mbsys_jstar_situation_struct situation;

	/* Situation data V2 */
	struct mbsys_jstar_situation2_struct situation2;

    /* Cable Counter data */
    struct mbsys_jstar_cable_struct cable;

    /* Kilometer of Pipe data */
    struct mbsys_jstar_pipe_struct pipe;

    /* Container Timestamp */
    struct mbsys_jstar_container_struct container;

    /* Attitude data */
    struct mbsys_jstar_attitudebathy_struct attitudebathy;

    /* Pressure data */
    struct mbsys_jstar_pressurebathy_struct pressurebathy;

    /* Altitude data */
    struct mbsys_jstar_altitudebathy_struct altitude;

    /* Position data */
    struct mbsys_jstar_positionbathy_struct positionbathy;

    /* Status data */
    struct mbsys_jstar_statusbathy_struct statusbathy;
};

/* system specific function prototypes */
int mbsys_jstar_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_jstar_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_jstar_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_jstar_pingnumber(int verbose, void *mbio_ptr, unsigned int *pingnumber, int *error);
int mbsys_jstar_preprocess(int verbose, void *mbio_ptr, void *store_ptr, void *platform_ptr, void *preprocess_pars_ptr,
                           int *error);
int mbsys_jstar_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                        double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss, char *beamflag,
                        double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss,
                        double *ssacrosstrack, double *ssalongtrack, char *comment, int *error);
int mbsys_jstar_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
                       double navlat, double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath,
                       double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
                       double *ssalongtrack, char *comment, int *error);
int mbsys_jstar_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
                       double *angles_forward, double *angles_null, double *heave, double *alongtrack_offset, double *draft,
                       double *ssv, int *error);
int mbsys_jstar_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error);
int mbsys_jstar_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                                 double *altitude, int *error);
int mbsys_jstar_insert_altitude(int verbose, void *mbio_ptr, void *store_ptr, double transducer_depth, double altitude,
                                int *error);
int mbsys_jstar_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                            double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll,
                            double *pitch, double *heave, int *error);
int mbsys_jstar_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
                           double navlat, double speed, double heading, double draft, double roll, double pitch, double heave,
                           int *error);
int mbsys_jstar_extract_rawssdimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *sample_interval,
                                        int *num_samples_port, int *num_samples_stbd, int *error);
int mbsys_jstar_extract_rawss(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *sidescan_type,
                              double *sample_interval, double *beamwidth_xtrack, double *beamwidth_ltrack, int *num_samples_port,
                              double *rawss_port, int *num_samples_stbd, double *rawss_stbd, int *error);
int mbsys_jstar_insert_rawss(int verbose, void *mbio_ptr, void *store_ptr, int kind, int sidescan_type, double sample_interval,
                             double beamwidth_xtrack, double beamwidth_ltrack, int num_samples_port, double *rawss_port,
                             int num_samples_stbd, double *rawss_stbd, int *error);
int mbsys_jstar_extract_segytraceheader(int verbose, void *mbio_ptr, void *store_ptr, int *kind, void *segyheader_ptr,
                                        int *error);
int mbsys_jstar_extract_segy(int verbose, void *mbio_ptr, void *store_ptr, int *sampleformat, int *kind, void *segyheader_ptr,
                             float *segydata, int *error);
int mbsys_jstar_insert_segy(int verbose, void *mbio_ptr, void *store_ptr, int kind, void *segyheader_ptr, float *segydata,
                            int *error);
int mbsys_jstar_ctd(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nctd, double *time_d, double *conductivity,
                    double *temperature, double *depth, double *salinity, double *soundspeed, int *error);
int mbsys_jstar_copyrecord(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error);

#endif  /* MBSYS_JSTAR2_H_ */
