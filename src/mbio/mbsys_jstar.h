/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_jstar.h	2/21/2005
 *
 *    Copyright (c) 2005-2024 by
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
 * mbsys_jstar.h  defines the data structure used by MBIO functions
 * to store sidescan data read from the MBF_EDGJSTAR format (MBIO id 132).
 *
 * Author:	D. W. Caress
 * Date:	February 21, 2005
 *
 */
/*
 * Notes on the MBSYS_JSTAR data structure:
 *   1. The J-star data format is used to store raw sidescan data from
 *      Edgetech sidescan and subbottom profiler sonars. This format
 *      is a variant of the SEGY format.
 *   2. The J-Star variant eliminates the SEGY EGCDIC and binary reel headers,
 *      and adds a message header to the beginning of each trace header.
 *      A J-Star stander format (JSF) file consists of a collection of trace
 *      records with the following components:
 *            1. A 16-byte message header.
 *            2. A 240 byte trace header.
 *            3. Trace data (2 bytes per sample)
 */

#ifndef MBSYS_JSTAR_H_
#define MBSYS_JSTAR_H_

/* specify the maximum number of sidescan pixels that can be returned
    by mbsys_jstar_extract() */
#define MBSYS_JSTAR_MESSAGE_SIZE 16
#define MBSYS_JSTAR_SBPHEADER_SIZE 240
#define MBSYS_JSTAR_SSHEADER_SIZE 240
#define MBSYS_JSTAR_SSOLDHEADER_SIZE 80
#define MBSYS_JSTAR_PIXELS_MAX 2000
#define MBSYS_JSTAR_SYSINFO_MAX 16384

#define MBSYS_JSTAR_DATA_SONAR                  80
#define MBSYS_JSTAR_DATA_SONAR2                 82
#define MBSYS_JSTAR_DATA_4400SAS                86
#define MBSYS_JSTAR_DATA_SYSINFO                182
#define MBSYS_JSTAR_DATA_FILETIMESTAMP          426     // New 5.5.24
#define MBSYS_JSTAR_DATA_FILEPADDING            428     // New 5.5.24
#define MBSYS_JSTAR_DATA_NMEA                   2002
#define MBSYS_JSTAR_DATA_PITCHROLL              2020
#define MBSYS_JSTAR_DATA_MISCANALOG             2040
#define MBSYS_JSTAR_DATA_PRESSURE               2060
#define MBSYS_JSTAR_DATA_DVL                    2080
#define MBSYS_JSTAR_DATA_SITUATION              2090     // New 5.5.24
#define MBSYS_JSTAR_DATA_SITUATIONV2            2091
#define MBSYS_JSTAR_DATA_CABLECOUNTER           2100
#define MBSYS_JSTAR_DATA_KMPIPEDATA             2101
#define MBSYS_JSTAR_DATA_CONTAINERTIMESTAMP     2111     // New 5.5.24

#define MBSYS_JSTAR_DATA_BATHYMETRICDATA        3000
#define MBSYS_JSTAR_DATA_BATHYMETRICATTITUDE    3001
#define MBSYS_JSTAR_DATA_BATHYMETRICPRESSURE    3002
#define MBSYS_JSTAR_DATA_BATHYMETRICALTITUDE    3003
#define MBSYS_JSTAR_DATA_BATHYMETRICPOSITION    3004

#define MBSYS_JSTAR_DATA_COMMENT                17229

#define MBSYS_JSTAR_SUBSYSTEM_SBP 0
#define MBSYS_JSTAR_SUBSYSTEM_SSLOW 20
#define MBSYS_JSTAR_SUBSYSTEM_SSHIGH 21

/* Edgetech trace data format definitions */
#define MBSYS_JSTAR_TRACEFORMAT_ENVELOPE 0     /* 2 bytes/sample (unsigned) */
#define MBSYS_JSTAR_TRACEFORMAT_ANALYTIC 1     /* 4 bytes/sample (I + Q) */
#define MBSYS_JSTAR_TRACEFORMAT_RAW 2          /* 2 bytes/sample (signed) */
#define MBSYS_JSTAR_TRACEFORMAT_REALANALYTIC 3 /* 2 bytes/sample (signed) */
#define MBSYS_JSTAR_TRACEFORMAT_PIXEL 4        /* 2 bytes/sample (signed) */

struct mbsys_jstar_message_struct {
	/* Message Header */
	unsigned short start_marker; /* bytes 0-1,    Marker for the start of header (0x1601) */
	mb_u_char version;           /* byte  2,      Version of protocol used */
	mb_u_char session;           /* byte  3,      Session identifier */
	unsigned short type;         /* bytes 4-5,    Message type (80 - sonar trace data ) */
	mb_u_char command;           /* bytes 6,      Command type */
	mb_u_char subsystem;         /* bytes 7,      Subsystem:
	                                  0 - subbottom
	                                 20 - 75 or 120 kHz sidescan
	                                 21 - 410 kHz sidescan */
	mb_u_char channel; /* bytes 8,      Channel for multi-channel systems
	                       0 = port
	                       1 = starboard */
	mb_u_char sequence;      /* bytes 9,      Sequence number */
	unsigned short reserved; /* bytes 10-11,  Reserved */
	unsigned int size;       /* bytes 12-15,  Size of following message in bytes */
};

struct mbsys_jstar_comment_struct {
	/* Message Header */
	struct mbsys_jstar_message_struct message;

	/* Comment */
	char comment[MB_COMMENT_MAXLINE];
};

struct mbsys_jstar_sysinfo_struct {
	/* Message Header */
	struct mbsys_jstar_message_struct message;

	/* System Information */
	int system_type; /* System Type Number and Description:
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
	                                 128     4100, 272 /560A Side Scan */
	int reserved1;
	int version; /* Sonar software version */
	int reserved2;
	int platformserialnumber; /* Serial number of platform */

	/* Sysinfo message */
	int sysinfosize;
	char sysinfo[MBSYS_JSTAR_SYSINFO_MAX];
};

struct mbsys_jstar_filetimestamp_struct {
	/* Message Header */
	struct mbsys_jstar_message_struct message;

    /* time since 1/1/1970 */
    int seconds;
    int milliseconds;
};

struct mbsys_jstar_nmea_struct {
	/* Message Header */
	struct mbsys_jstar_message_struct message;

	/* Time and source */
	int seconds; /* seconds since start of time */
	int milliseconds;    /* milliseconds since start of time */
	char source; /* 1=sonar, 2=discover, 3=ETSI */
	char reserve[3];

	/* NMEA string */
	char nmea[MB_COMMENT_MAXLINE];
};

struct mbsys_jstar_pitchroll_struct {
	/* Message Header */
	struct mbsys_jstar_message_struct message;

	/* Time */
	int seconds; /* seconds since start of time */
	int milliseconds;    /* milliseconds since start of time */
	char reserve1[4];

    /* attitude data */
	short accelerationx;       /* x acceleration: multiply by (20 * 1.5) / (32768) to get G's */
	short accelerationy;       /* y acceleration: multiply by (20 * 1.5) / (32768) to get G's */
	short accelerationz;       /* z acceleration: multiply by (20 * 1.5) / (32768) to get G's */
	short gyroratex;           /* x gyro rate: multiply by (500 * 1.5) / (32768) to get deg/sec */
	short gyroratey;           /* y gyro rate: multiply by (500 * 1.5) / (32768) to get deg/sec */
	short gyroratez;           /* z gyro rate: multiply by (500 * 1.5) / (32768) to get deg/sec */
	short pitch;               /* pitch: multiply by (180.0 / 32768) to get degrees */
	short roll;                /* roll: multiply by (180.0 / 32768) to get degrees */
	short temperature;         /* temperature: 0.1 degree C */
	unsigned short deviceinfo; /* device specific info */
	short heave;               /* heave: 0.001 m */
	unsigned short heading;    /* 0.01 degrees */
	int datavalidflags;        /* data valid flags:
	                       0 - ax
	                       1 - ay
	                       2 - az
	                       3 - rx
	                       4 - ry
	                       5 - rz
	                       6 - pitch
	                       7 - roll
	                       8 - heave
	                       9 - heading
	                       10 - temperature
	                       11 - device info */
	int reserve2;
};

struct mbsys_jstar_pressure_struct {
	/* Message Header */
	struct mbsys_jstar_message_struct message;

	/* Time */
	int seconds; /* seconds since start of time */
	int milliseconds;    /* milliseconds since start of time */
	char reserve1[4];

    /* CTD data */
	int pressure;       /* 0.001 PSI */
	int salinity;       /* ppm */
	int datavalidflags; /* data valid flags:
	                0 - pressure
	                1 - temp
	                2 - salt PPM
	                3 - conductivity
	                4 - sound velocity */
	int conductivity;   /* uSiemens/cm */
	int soundspeed;     /* 0.001 m/sec */
	int reserve2[10];
};

struct mbsys_jstar_dvl_struct {
	/* Message Header */
	struct mbsys_jstar_message_struct message;

	/* Time */
	int seconds; /* seconds since start of time */
	int milliseconds;    /* milliseconds since start of time */
	char reserve1[4];

    /* dvl data */
	unsigned int datavalidflags; /* Bit values indicate which values are present:
	             0: X,Y velocity present
	             1: 1 = velocity in ship coordinates
	                0 = velocity in earth coordinates
	             2: Z (vertical) velocity present
	             3: X, Y water velocity present
	             4: Z (vertical) water velocity present
	             5: Distance to bottom present
	             6: Heading present
	             7: Pitch present
	             8: Roll present
	             9: Temperature present
	             10: Depth present
	             11: Salinity present
	             12: Sound velocity present
	             ---
	             31: Error detected */
	int beam1range;              /* 0.01 m (0 = invalid) */
	int beam2range;              /* 0.01 m (0 = invalid) */
	int beam3range;              /* 0.01 m (0 = invalid) */
	int beam4range;              /* 0.01 m (0 = invalid) */
	short velocitybottomx;       /* x velocity wrt bottom (0.001 m/s, positive to starboard or east) */
	short velocitybottomy;       /* y velocity wrt bottom (0.001 m/s, positive to forward or north) */
	short velocitybottomz;       /* z velocity wrt bottom (0.001 m/s, positive upward) */
	short velocitywaterx;        /* x velocity wrt water (0.001 m/s, positive to starboard or east) */
	short velocitywatery;        /* y velocity wrt water (0.001 m/s, positive to forward or north) */
	short velocitywaterz;        /* z velocity wrt water (0.001 m/s, positive upward) */
	unsigned short depth;        /* depth (0.1 m) */
	short pitch;                 /* pitch (0.01 degree, positive bow up) */
	short roll;                  /* roll (0.01 degree, positive port up) */
	short heading;               /* heading (0.01 degree) */
	short salinity;              /* salinity (ppt (part per thousand)) */
	short temperature;           /* temperature (0.01 degree celcius) */
	short soundspeed;            /* sound speed (m/sec) */
	short reserve2[7];
};

struct mbsys_jstar_situation_struct {
	/* Message Header */
	struct mbsys_jstar_message_struct message;

	/* Time */
	int seconds; /* seconds since start of time */
	int milliseconds;    /* milliseconds since start of time */
	char reserve1[4];

    /* navigation and attitude data */
	unsigned int datavalidflags; /* Validity Flags:
                Validity Flags indicate which of the following fields are valid.
                If the corresponding bit is set the field is valid.
                        Bit 0 : microsecondTimestamp
                        Bit 1 : latitude
                        Bit 2 : longitude
                        Bit 3 : depth
                        Bit 4 : heading
                        Bit 5 : pitch
                        Bit 6:roll
                        Bit 7 : XRelativePosition
                        Bit 8 : YRelativePosition
                        Bit 9 : ZRelativePosition
                        Bit 10 : XVelocity
                        Bit 11 : YVelocity
                        Bit 12 : ZVelocity
                        Bit 13 : NorthVelocity
                        Bit 14 : EastVelocity
                        Bit 15 : downVelocity
                        Bit 16 : XAngularRate
                        Bit 17 : YAngularRate
                        Bit 18 : ZAngularRate
                        Bit 19 : XAcceleration
                        Bit 20 : YAcceleration
                        Bit 21 : ZAcceleration
                        Bit 22 : latitudeStandardDeviation
                        Bit 23 : longitudeStandardDeviation Bit
                        24 : depthStandardDeviation
                        Bit 25 : headingStandardDeviation
                        Bit 26 : pitchStandardDeviation
                        Bit 27 : rollStandardDeviation */
	char reserve2[4];
    unsigned long int time_usec; /* Microsecond timestamp, us since 12:00:00 am GMT, January 1, 1970 */
    double latitude; /* Latitude in degrees, north is positive */
    double longitude; /* Longitude in degrees, east is positive */
    double depth; /* Depth in meters */
    double heading; /* Heading in degrees */
    double pitch; /* Pitch in degrees, bow up is positive */
    double roll; /* Roll in degrees, port up is positive */
    double x_forward; /* X, forward, relative position in meters, surge */
    double y_starboard; /* Y, starboard, relative position in meters, sway */
    double z_downward; /* Z downward, relative position in meters, heave */
    double velocity_x_forward; /* X, forward, velocity in meters per second */
    double velocity_y_starboard; /* Y, starboard, velocity in meters per second */
    double velocity_z_downward; /* Z, downward, velocity in meters per second */
    double velocity_north; /* North velocity in meters per second */
    double velocity_east; /* East velocity in meters per second */
    double velocity_down; /* Down velocity in meters per second */
    double angular_rate_x; /* X angular rate in degrees per second, port up is positive */
    double angular_rate_y; /* Y angular rate in degrees per second, bow up is positive */
    double angular_rate_z; /* Z angular rate in degrees per second, starboard is positive */
    double acceleration_x; /* X, forward, acceleration in meters per second per second */
    double acceleration_y; /* Y, starboard, acceleration in meters per second per second */
    double acceleration_z; /* Z, downward, acceleration in meters per second per second */
    double latitude_sigma; /* Latitude standard deviation in meters */
    double longitude_sigma; /* Longitude standard deviation in meters */
    double depth_sigma; /* Depth standard deviation in meters */
    double heading_sigma; /* Heading standard deviation in degrees */
    double pitch_sigma; /* Pitch standard deviation in degrees */
    double roll_sigma; /* Roll standard deviation in degrees */
    unsigned short reserved3[16]; /* Reserved – Do not use */
};


struct mbsys_jstar_situation2_struct {
	/* Message Header */
	struct mbsys_jstar_message_struct message;

	/* Time */
	int seconds; /* seconds since start of time */
	int milliseconds;    /* milliseconds since start of time */
	char reserve1[4];

    /* navigation and attitude data */
	unsigned int datavalidflags; /* Validity Flags:
                Validity Flags indicate which of the following fields are valid.
                If the corresponding bit is set the field is valid.
                        Bit 0 : microsecondTimestamp
                        Bit 1 : latitude
                        Bit 2 : longitude
                        Bit 3 : depth
                        Bit 4 : heading
                        Bit 5 : pitch
                        Bit 6:roll
                        Bit 7 : XRelativePosition
                        Bit 8 : YRelativePosition
                        Bit 9 : ZRelativePosition
                        Bit 10 : XVelocity
                        Bit 11 : YVelocity
                        Bit 12 : ZVelocity
                        Bit 13 : NorthVelocity
                        Bit 14 : EastVelocity
                        Bit 15 : downVelocity
                        Bit 16 : XAngularRate
                        Bit 17 : YAngularRate
                        Bit 18 : ZAngularRate
                        Bit 19 : XAcceleration
                        Bit 20 : YAcceleration
                        Bit 21 : ZAcceleration
                        Bit 22 : latitudeStandardDeviation
                        Bit 23 : longitudeStandardDeviation Bit
                        24 : depthStandardDeviation
                        Bit 25 : headingStandardDeviation
                        Bit 26 : pitchStandardDeviation
                        Bit 27 : rollStandardDeviation */
	char reserve2[4];
    unsigned long int time_usec; /* Microsecond timestamp, us since 12:00:00 am GMT, January 1, 1970 */
    double latitude; /* Latitude in degrees, north is positive */
    double longitude; /* Longitude in degrees, east is positive */
    double depth; /* Depth in meters */
    double heading; /* Heading in degrees */
    double pitch; /* Pitch in degrees, bow up is positive */
    double roll; /* Roll in degrees, port up is positive */
    double x_forward; /* X, forward, relative position in meters, surge */
    double y_starboard; /* Y, starboard, relative position in meters, sway */
    double z_downward; /* Z downward, relative position in meters, heave */
    double velocity_x_forward; /* X, forward, velocity in meters per second */
    double velocity_y_starboard; /* Y, starboard, velocity in meters per second */
    double velocity_z_downward; /* Z, downward, velocity in meters per second */
    double velocity_north; /* North velocity in meters per second */
    double velocity_east; /* East velocity in meters per second */
    double velocity_down; /* Down velocity in meters per second */
    double angular_rate_x; /* X angular rate in degrees per second, port up is positive */
    double angular_rate_y; /* Y angular rate in degrees per second, bow up is positive */
    double angular_rate_z; /* Z angular rate in degrees per second, starboard is positive */
    double acceleration_x; /* X, forward, acceleration in meters per second per second */
    double acceleration_y; /* Y, starboard, acceleration in meters per second per second */
    double acceleration_z; /* Z, downward, acceleration in meters per second per second */
    double latitude_sigma; /* Latitude standard deviation in meters */
    double longitude_sigma; /* Longitude standard deviation in meters */
    double depth_sigma; /* Depth standard deviation in meters */
    double heading_sigma; /* Heading standard deviation in degrees */
    double pitch_sigma; /* Pitch standard deviation in degrees */
    double roll_sigma; /* Roll standard deviation in degrees */
    unsigned short reserved3[16]; /* Reserved – Do not use */
};

struct mbsys_jstar_channel_struct {
	/* Message Header */
	struct mbsys_jstar_message_struct message;

	/* Trace Header */
	int pingTime;            /* 0-3 : Ping Time in epoch seconds [since (1/1/1970)] (Protocol Version 8 onwards) */
	unsigned int startDepth; /* 4-7 : Starting depth (window offset) in samples. */
	unsigned int pingNum;    /* 8-11: Ping number (increments with ping) ** */
	short reserved1[2];      /* 12-15: Reserved */
	short msb;               /* 16-17: MSBs – Most Significant Bits
                              *   High order bits to extend 16 bits unsigned short
                              *   values to 20 bits. The 4MSB bits become the most
                              *   significant portion of the new 20 bit value.
                              *       Bits 0 – 3: Start Frequency
                              *       Bits 4 – 7: End Frequency
                              *       Bits 8 – 11: Samples in this Packet
                              *       Bits 12 – 15: Mark Number (added in protocol version 0xA)
                              *   The Most Significant Bits fields are used to extend
                              *   16 bit integers to 20 bits. These are added as needed
                              *   when the range of possible values exceeds what can be
                              *   stored in a 16 bit integer. The simplest way to use
                              *   these additional bits is to treat the value as a 32 bit
                              *   integer, the existing value becomes the least
                              *   significant 16 bits, and the MSB field becomes the
                              *   next most significant 4 bits with the most significant
                              *   12 bits set to zeros. */
	short lsb1;              /* 18-19 : LSB – Extended precision
                              *   Low order bits for fields requiring greater precision.
                              *       Bits 0-7: Sample Interval-- Sample interval fractional component
                              *       Bits 8-15: Course- - fractional portion of course
                              *   (Added in protocol version 0xB) */
	short lsb2;              /* 20-21 : Reserved – LBS2 – Extended precision
                              *   Low order bits for fields requiring greater precision.
                              *       Bits 0 – 3: Speed - sub fractional speed component (added in protocol version 0xC).
                              *       Bits 4 – 13: Sweep Length in Microsecond, from 0 - 999 (added in protocol version 0xD).
                              *       Bits 14 – 15: Reserved*/
	short reserved2[3];      /* 22-27 : Reserved – Do not use */

	short traceIDCode;       /* 28-29 : ID Code (always 1 => seismic data) ** */

	unsigned short validityFlag;  /* 30-31 : Validity flags bitmap
                                   *   Bit 0: Lat Lon or XY valid
                                   *   Bit 1: Course valid
                                   *   Bit 2: Speed valid
                                   *   Bit 3: Heading valid
                                   *   Bit 4: Pressure valid
                                   *   Bit 5: Pitch roll valid
                                   *   Bit 6: Altitude valid
                                   *   Bit 7: Reserved
                                   *   Bit 8: Water temperature valid Bit 9: Depth valid
                                   *   Bit 10: Annotation valid
                                   *   Bit 11: Cable counter valid
                                   *   Bit 12: KP valid
                                   *   Bit 13: Position interpolated
                                   *   Bit 14: Water sound speed valid */
    short reserved3;              /* 32-33 : Reserved – Do not use */
	short dataFormat;             /* 34-35 : DataFormatType
                                   *         0 = one short per sample - envelope data. The total number
                                   *             of bytes of data to follow is 2 * samples.
                                   *         1 = two shorts per sample - stored as real (one short),
                                   *             imaginary (one short). The total number of bytes
                                   *             of data to follow is 4 * samples.
                                   *         2 = one short per sample - before matched filter.
                                   *             The total number of bytes of data to follow is 2 * samples.
                                   *         9 = two shorts per sample - stored as real (one short),
                                   *             imaginary (one short), - prior to matched filtering.
                                   *             This is the code for unmatched filtered analytic data,
                                   *             whereas value 1 is intended for match filtered analytic
                                   *             data. The total number of bytes of data to follow is 4 * samples.
                                   *         NOTE: Values greater than 255 indicate that the data to
                                   *         follow is compressed and must be decompressed prior to use.
                                   *         For more detail refer to the JSF Data File Decompression
                                   *         Application Note for more information.
                                   *
                                   *     Old definitions:
                                   *         0 = 1 short  per sample  - envelope data
                                   *         1 = 2 shorts per sample, - stored as real(1), imag(1),
                                   *         2 = 1 short  per sample  - before matched filter
                                   *         3 = 1 short  per sample  - real part analytic signal
                                   *         4 = 1 short  per sample  - pixel data / ceros data */
	short NMEAantennaeR; /* 36-37 : Distance from Antenna to Tow point in Centimeters - Sonar Aft is Positive */
	short NMEAantennaeO; /* 38-39 : Distance from Antenna to Tow Point in Centimeters - Sonar to Starboard is Positive. */
	short reserved4[2];  /* 40-43 : Reserved – Do not use */
	float kmOfPipe;      /* 44-47 : Kilometers of Pipe - See Validity Flag (bytes 30 – 31). */
	short reserved5[16]; /* 48-79 : Reserved – Do not use */

	/* -------------------------------------------------------------------- */
	/* Navigation data :                                                    */
	/* If the coorUnits are seconds(2), the x values represent longitude    */
	/* and the y values represent latitude.  A positive value designates    */
	/* the number of seconds east of Greenwich Meridian or north of the     */
	/* equator.                                                             */
	/* -------------------------------------------------------------------- */
	int coordX;                  /* 80-83 : longitude or easting  */
	int coordY;                  /* 84-87 : latitude or northing */
	short coordUnits;            /* 88-89 : Units of coordinates -
                                  *         1 = X,Y in millimeters
                                  *         2 = X,Y in iminutes of arc times 10000
                                  *         3 = X,Y in decimeters */
	char annotation[24];         /* 90-113 : Annotation string */
	unsigned short samples;      /* 114-115 : Samples in this packet
	                              *           Large sample sizes require multiple packets.
	                              *           For protocol versions 0xA and above, the
	                              *           MSB1 field should include the MSBs
	                              *           (Most Significant Bits) needed to
	                              *           determine the number of samples.
	                              *           See bits 8-11 in bytes 16-17. Field MSB1
	                              *           for MSBs for large sample sizes. */
	unsigned int sampleInterval; /* 116-119 : Sampling Interval in Nanoseconds
                                  *           NOTE: For protocol versions 0xB and
                                  *           above, see the LSBs field should
                                  *           include the fractional component
                                  *           needed to determine the sample interval.
                                  *           See bits 0-7 in bytes 18-19. Field LSB1
                                  *           for LSBs for increased precision. */
	unsigned short ADCGain;      /* 120-121 : Gain factor of ADC */
	short pulsePower;            /* 122-123 : User Transmit Level Setting (0 – 100%). */
	short reserved6;             /* 124-125 : Reserved */
	unsigned short startFreq;    /* 126-127 : Transmit Pulse Starting Frequency in
                                  *           daHz (decaHertz, units of 10Hz).
                                  *           NOTE: For protocol versions 0xA and above,
                                  *           the MSB1 field should include the MSBs
                                  *           (Most Significant Bits) needed to
                                  *           determine the starting frequency
                                  *           of transmit pulse.
                                  *           See Bits 0-3 in byte 16-17. Field MSB1
                                  *           for MSBs for large transmit pulse. */
	unsigned short endFreq;      /* 128-129 : Transmit Pulse Ending Frequency in
                                  *           daHz (decaHertz, units of 10Hz).
                                  *           NOTE: For protocol versions 0xA and above,
                                  *           the MSB1 field should include the MSBs
                                  *           (Most Significant Bits) needed to
                                  *           determine the starting frequency of
                                  *           transmit pulse.
                                  *           See bits 4-7 in byte 16-17. Field MSB1
                                  *           for MSBs for large transmit pulse. */
	unsigned short sweepLength;  /* 130-131 : Sweep Length in Milliseconds.
                                  *           See bytes 18-19 for LSBs (Least Significant Bits),
                                  *           LSB2 bits 4 - 13 contain the microsecond
                                  *           portion (0 - 999). LSB2 part was added
                                  *           in protocol version 0xD, and was previously 0.
                                  *           */
	int pressure;                /* 132-135 : Pressure in Milli PSI (1 unit = 1/1000 PSI)
                                  *           See Validity Flag (bytes 30-31) */
	int sonarDepth;              /* 136-139 : Depth in Millimeters (if not = 0)
                                  *           See Validity Flag (bytes 30-31). */
	unsigned short sampleFreq;   /* 140-141 : Sample Frequency of the Data in hertz
                                  *           NOTE: For all data types EXCEPT RAW
                                  *           (Data Format = 2) this is the sampling
                                  *           frequency of the data. For RAW data,
                                  *           this is one- half the sample frequency
                                  *           of the data (FS/2). All values are
                                  *           modulo 65536. Use this in conjunction
                                  *           with the Sample Interval (bytes 114-115)
                                  *           to calculate correct sample rate. */
	unsigned short pulseID;      /* 142-143 : Outgoing Pulse Identifier */
	int sonarAltitude;           /* 144-147 : Altitude in Millimeters
                                  *           A zero implies not filled. See Validity Flag (bytes 30-31) */
	float soundspeed;            /* 148-151 : Sound Speed in Meters per Second.
                                  *           See Validity Flag (bytes 30-31). */
	float mixerFrequency;        /* 152-155 : Mixer Frequency in Hertz
                                  *           For single pulses systems this should
                                  *           be close to the center frequency.
                                  *           For multi pulse systems this should
                                  *           be the approximate center frequency
                                  *           of the span of all the pulses. */
	short year;                  /* 156-157 : Year Data Recorded (CPU time) e.g. 2009.
                                  *           The Ping Time can also be determined
                                  *           from the Year, Day, Hour, Minute and
                                  *           Seconds as per bytes 156 to 165.
                                  *           Provides 1 second level accuracy and
                                  *           resolution.
                                  *           See Bytes 0-3 these 2 time stamps
                                  *           are equivalent and identical. For
                                  *           most purposes this should not be used.
                                  *           For higher resolution (milliseconds)
                                  *           use the Year, and Day values of bytes
                                  *           156 to 159, and then use the milliSecondsToday
                                  *           value of bytes 200-203 to complete the
                                  *           timestamp. System time is set to UTC,
                                  *           regardless of time zone. This time
                                  *           format is backwards compatible with
                                  *           all older Protocol Revisions */
	short day;                   /* 158-159 : Day (1 – 366) (should not be used) */
	short hour;                  /* 160-161 : Hour (see Bytes 200-203) (should not be used) */
	short minute;                /* 162-163 : Minute (should not be used) */
	short second;                /* 164-165 : Second (should not be used) */
	short timeBasis;             /* 166-167 : Time Basis (always 3) */
	short weightingFactor;       /* 168-169 :  Weighting Factor for Block Floating
                                  *            Point Expansion
                                  *            -- defined as 2 to N Volts for LSB.
                                  *            All data MUST be scaled by 2-N,
                                  *            where N is the Weighting Factor.
                                  *            (See Equation 2-1, on page 2-8) */
	short numberPulses;          /* 170-171 : Number of Pulses in the Water */
	/* -------------------------------------------------------------------- */
	/* From pitch/roll/temp/heading sensor */
	/* -------------------------------------------------------------------- */
	short heading;               /* 172-173 : Compass Heading (0 to 359.99) in units of 1/100 Degree.
                                  *           See Validity Flag (bytes 30-31).
                                  *           The Compass heading is the magnetic heading
                                  *           of the towfish. If a Gyro sensor is properly
                                  *           interfaced to the DISCOVER Topside Acquisition Unit
                                  *           with a valid NMEA HDT message, this field will
                                  *           contain the Gyro heading, relative to True North. */
	short pitch;                 /* 174-175 : Pitch [(degrees / 180.0) * 32768.0] maximum resolution.
                                  *           Positive values indicate bow up.
                                  *           See Validity Flag (bytes 30-31).*/
	short roll;                  /* 176-177 : Roll [(degrees / 180.0) * 32768.0] maximum resolution.
                                  *           Positive values indicate port up.
                                  *           See Validity Flag (bytes 30-31).*/
	short reserved8;             /* 178-179 : Reserved */
	/* -------------------------------------------------------------------- */
	/* Trigger source from 180-185                                          */
	/* -------------------------------------------------------------------- */
	short reserved9;             /* 180-181 : Reserved */
	short triggerSource;         /* 182-183 : Trigger Source
                                  *               0 = Internal
                                  *               1 = External
                                  *               2 = Coupled */
	unsigned short markNumber;   /* 184-185 : Mark Number
                                  *               0 = No Mark
                                  *           See bytes 16 –17 fields MSB1 for MSBs
                                  *           (Most Significant Bits) for large
                                  *           values (> 655350). */
	/* -------------------------------------------------------------------- */
	/* Position fix time                                                    */
	/* -------------------------------------------------------------------- */
	short NMEAHour;                 /* 186-187 : Position Fix Hour (0 – 23)
                                     *           NOTE: the NAV time is the time
                                     *           of the latitude and longitude fix.*/
	short NMEAMinutes;              /* 188-189 : Position Fix Minutes (0 – 59)
                                     *           NOTE: the NAV time is the time
                                     *           of the latitude and longitude fix.*/
	short NMEASeconds;              /* 190-191 : Position Fix Seconds (0 – 59)
                                     *           NOTE: the NAV time is the time
                                     *           of the latitude and longitude fix.*/
	short NMEACourse;               /* 192-193 : Course in Degrees (0 to 359.9)
                                     *           Starting with protocol version 0x0C
                                     *           two digits of fractional degrees
                                     *           are stored in LSB1. Fractional
                                     *           portion in LSBs (Least Significant Bits).
                                     *           See bytes 18 – 19.*/
	short NMEASpeed;                /* 194-195 : Speed – in Tenths of a Knot
                                     *           Starting with protocol version 0x0C
                                     *           one additional digit of fractional
                                     *           knot (1/100) is stored in LSB2.
                                     *           For an additional fractional digit,
                                     *           see LSB2 (bytes 20 -21).*/
	short NMEADay;                  /* 196-197 : Position Fix Day (1 – 366) */
	short NMEAYear;                 /* 198-199 : Position Fix Year */

	/* -------------------------------------------------------------------- */
	/* Miscellaneous data                                                   */
	/* -------------------------------------------------------------------- */
	unsigned int millisecondsToday; /* 200-203 : Milliseconds Today (Since Midnight)
                                     *           Use with seconds since 1970 to
                                     *           get time to the milliseconds (time of Ping).*/
	unsigned short ADCMax;          /* 204-205 : Maximum Absolute Value of ADC Samples
                                     *           in this Packet */
	short reserved10;               /* 206-207 : Reserved */
	short reserved11;               /* 208-209 : Reserved */
	char softwareVersion[6];        /* 210-215 : Sonar Software Version Number - ASCII */
	int sphericalCorrection;  /* 216-219 : Initial Spherical Correction Factor in
                               *           Samples times 100.
                               *            A value of -1 indicates that the spherical
                               *            spreading is disabled. /*/
	unsigned short packetNum; /* 220-221 : Packet number (1 - N) (Each ping starts with packet 1) */
	short ADCDecimation;      /* 222-223 : ADC Decimation * 100 times */
	short reserved12;         /* 224-225 : Reserved */
	short temperature;        /* 226-227 : Water Temperature in Units of 1/10 Degree C.
                               *           See Validity Flag (bytes 30-31).*/
	float layback;            /* 227-231 : Layback
                               *           Distance to the sonar in meters. */
	int reserved13;           /* 232-235 : Reserved */
	short cableOut;           /* 236-239 : Cable Out in Decimeters
                               *           See Validity Flag (bytes 30-31). */
	short reserved14;         /* 236-239 : Reserved */

	/* -------------------------------------------------------------------- */
	/* MB-System-only parameters from 236-239                               */
	/* -------------------------------------------------------------------- */
	//int depth;         /* 227-231 : Seafloor depth in 0.001 m */
	//int sensordepth;   /* 232-235 : Sonar depth in 0.001 m */
	//int sonaraltitude; /* 236-239 : Sonar altitude in 0.001 m */

	/* trace data stored as shorts */
	unsigned int trace_alloc;
	unsigned short *trace;
};

struct mbsys_jstar_channel_old_struct {
	/* Message Header */
	struct mbsys_jstar_message_struct message;

	/* Trace Header */
	int sequenceNumber;      /* 0-3 : Trace Sequence Number (always 0) ** */
	unsigned int startDepth; /* 4-7 : Starting depth (window offset) in samples. */
	unsigned int pingNum;    /* 8-11: Ping number (increments with ping) ** */
	unsigned int channelNum; /* 12-15 : Channel Number (0 .. n) ** */
	short unused1[6];        /* 16-27 */

	short traceIDCode; /* 28-29 : ID Code (always 1 => seismic data) ** */

	short unused2[2];    /* 30-33 */
	short dataFormat;    /* 34-35 : DataFormatType */
	                     /*   0 = 1 short  per sample  - envelope data */
	                     /*   1 = 2 shorts per sample, - stored as real(1), imag(1), */
	                     /*   2 = 1 short  per sample  - before matched filter */
	                     /*   3 = 1 short  per sample  - real part analytic signal */
	                     /*   4 = 1 short  per sample  - pixel data / ceros data */
	short NMEAantennaeR; /* 36-37 : Distance from towfish to antennae in cm */
	short NMEAantennaeO; /* 38-39 : Distance to antennae starboard direction in cm */
	char RS232[32];      /* 40-71 : Reserved for RS232 data - TBD */
	/* -------------------------------------------------------------------- */
	/* Navigation data :                                                    */
	/* If the coorUnits are seconds(2), the x values represent longitude    */
	/* and the y values represent latitude.  A positive value designates    */
	/* the number of seconds east of Greenwich Meridian or north of the     */
	/* equator.                                                             */
	/* -------------------------------------------------------------------- */
	int sourceCoordX;            /* 72-75 : Meters or Seconds of Arc */
	int sourceCoordY;            /* 76-79 : Meters or Seconds of Arc */
	int groupCoordX;             /* 80-83 : mm or 10000 * (Minutes of Arc) */
	int groupCoordY;             /* 84-87 : mm or 10000 * (Minutes of Arc) */
	short coordUnits;            /* 88-89 : Units of coordinates - 1->length (x /y), 2->seconds of arc */
	char annotation[24];         /* 90-113 : Annotation string */
	unsigned short samples;      /* 114-115 : Samples in this packet ** */
	                             /* Note:  Large sample sizes require multiple packets. */
	unsigned int sampleInterval; /* 116-119 : Sample interval in ns of stored data ** */
	unsigned short ADCGain;      /* 120-121 : Gain factor of ADC */
	short pulsePower;            /* 122-123 : user pulse power setting (0 - 100) percent */
	short correlated;            /* 124-125 : correlated data 1 - No, 2 - Yes */
	unsigned short startFreq;    /* 126-127 : Starting frequency in 10 * Hz */
	unsigned short endFreq;      /* 128-129 : Ending frequency in 10 * Hz */
	unsigned short sweepLength;  /* 130-131 : Sweep length in ms */
	short unused7[4];            /* 132-139 */
	unsigned short aliasFreq;    /* 140-141 : alias Frequency (sample frequency / 2) */
	unsigned short pulseID;      /* 142-143 : Unique pulse identifier */
	short unused8[6];            /* 144-155 */
	short year;                  /* 156-157 : Year data recorded (CPU time) */
	short day;                   /* 158-159 : day */
	short hour;                  /* 160-161 : hour */
	short minute;                /* 162-163 : minute */
	short second;                /* 164-165 : second */
	short timeBasis;             /* 166-167 : Always 3 (other not specified by standard) */
	short weightingFactor;       /* 168-169 :  weighting factor for block floating point expansion */
	                             /*            -- defined as 2 -N volts for lsb */
	short unused9;               /* 170-171 : */
	/* -------------------------------------------------------------------- */
	/* From pitch/roll/temp/heading sensor */
	/* -------------------------------------------------------------------- */
	short heading;     /* 172-173 : Compass heading (100 * degrees) -180.00 to 180.00 degrees */
	short pitch;       /* 174-175 : Pitch */
	short roll;        /* 176-177 : Roll */
	short temperature; /* 178-179 : Temperature (10 * degrees C) */
	/* -------------------------------------------------------------------- */
	/* User defined area from 180-239                                       */
	/* -------------------------------------------------------------------- */
	short heaveCompensation;        /* 180-181 : Heave compensation offset (samples) */
	short trigSource;               /* 182-183 : TriggerSource (0 = internal, 1 = external) */
	unsigned short markNumber;      /* 184-185 : Mark Number (0 = no mark) */
	short NMEAHour;                 /* 186-187 : Hour */
	short NMEAMinutes;              /* 188-189 : Minutes */
	short NMEASeconds;              /* 190-191 : Seconds */
	short NMEACourse;               /* 192-193 : Course */
	short NMEASpeed;                /* 194-195 : Speed */
	short NMEADay;                  /* 196-197 : Day */
	short NMEAYear;                 /* 198-199 : Year */
	unsigned int millisecondsToday; /* 200-203 : Millieconds today */
	unsigned short ADCMax;          /* 204-205 : Maximum absolute value for ADC samples for this packet */
	short calConst;                 /* 206-207 : System constant in tenths of a dB */
	short vehicleID;                /* 208-209 : Vehicle ID */
	char softwareVersion[6];        /* 210-215 : Software version number */
	/* Following items are not in X-Star */
	int sphericalCorrection;  /* 216-219 : Initial spherical correction factor (useful for multiping /*/
	                          /* deep application) * 100 */
	unsigned short packetNum; /* 220-221 : Packet number (1 - N) (Each ping starts with packet 1) */
	short ADCDecimation;      /* 222-223 : A/D decimation before FFT */
	short decimation;         /* 224-225 : Decimation factor after FFT */
	short unuseda;            /* 226-227 */

	/* -------------------------------------------------------------------- */
	/* MB-System-only parameters from 236-239                               */
	/* -------------------------------------------------------------------- */
	int depth;         /* 227-231 : Seafloor depth in 0.001 m */
	int sensordepth;   /* 236-235 : Sonar depth in 0.001 m */
	int sonaraltitude; /* 236-239 : Sonar altitude in 0.001 m */

	/* trace data stored as shorts */
	unsigned int trace_alloc;
	unsigned short *trace;
};

struct mbsys_jstar_ssold_struct {
	/* Message Header */
	struct mbsys_jstar_message_struct message;

	/* Trace Header */
	unsigned short subsystem;    /*   0 -   1 : Subsystem (0 .. n) */
	unsigned short channelNum;   /*   2 -   3 : Channel Number (0 .. n) */
	unsigned int pingNum;        /*   4 -   7 : Ping number (increments with ping) */
	unsigned short packetNum;    /*   8 -   9 : Packet number (1..n) Each ping starts with packet 1 */
	unsigned short trigSource;   /*  10 -  11 : TriggerSource (0 = internal, 1 = external) */
	unsigned int samples;        /*  12 -  15 : Samples in this packet */
	unsigned int sampleInterval; /*  16 -  19 : Sample interval in ns of stored data */
	unsigned int startDepth;     /*  20 -  23 : starting Depth (window offset) in samples */
	short weightingFactor;       /*  24 -  25 : -- defined as 2 -N volts for lsb */
	unsigned short ADCGain;      /*  26 -  27 : Gain factor of ADC */
	unsigned short ADCMax;       /*  28 -  29 : Maximum absolute value for ADC samples for this packet */
	unsigned short rangeSetting; /*  30 -  31 : Range Setting (meters X 10) */
	unsigned short pulseID;      /*  32 -  33 : Unique pulse identifier */
	unsigned short markNumber;   /*  34 -  35 : Mark Number (0 = no mark) */
	unsigned short dataFormat;   /*  36 -  37 : Data format */
	                             /*   0 = 1 short  per sample  - envelope data */
	                             /*   1 = 2 shorts per sample  - stored as real(1), imag(1), */
	                             /*   2 = 1 short  per sample  - before matched filter (raw) */
	                             /*   3 = 1 short  per sample  - real part analytic signal */
	                             /*   NOTE: For type = 1, the total number of bytes of data to follow is */
	                             /*   4 * samples.  For all other types the total bytes is 2 * samples */
	unsigned short reserved;     /*  38 -  39 : Reserved field to round up to a 32-bit word boundary */
	/* -------------------------------------------------------------------- */
	/* computer date / time data acquired                                   */
	/* -------------------------------------------------------------------- */
	unsigned int millisecondsToday; /*  40 -  43 : Millieconds today */
	short year;                     /*  44 -  45 : Year */
	unsigned short day;             /*  46 -  47 : Day of year (1 - 366) */
	unsigned short hour;            /*  48 -  49 : Hour of day (0 - 23) */
	unsigned short minute;          /*  50 -  51 : Minute (0 - 59) */
	unsigned short second;          /*  52 -  53 : Second (0 - 59) */
	/* -------------------------------------------------------------------- */
	/* Auxiliary sensor information */
	/* -------------------------------------------------------------------- */
	short heading;      /*  54 -  55 : Compass heading (minutes) */
	short pitch;        /*  56 -  57 : Pitch (minutes) */
	short roll;         /*  58 -  59 : Roll (minutes) */
	short heave;        /*  60 -  61 : Heave (centimeters) */
	short yaw;          /*  62 -  63 : Yaw (minutes) */
	unsigned int depth; /*  64 -  67 : Vehicle depth (centimeters) */
	short temperature;  /*  68 -  69 : Temperature (degrees Celsius X 10) */
	char reserved2[10]; /*  70 -  79 : Reserved for future use */

	/* trace data stored as shorts */
	unsigned int trace_alloc;
	unsigned short *trace;
};

struct mbsys_jstar_struct {
	int kind; /* MBIO data kind */

	/* Ping type */
	mb_u_char subsystem; /* bytes 7,      Subsystem:
	                          0 - subbottom
	                         20 - 75 or 120 kHz sidescan
	                         21 - 410 kHz sidescan */

	/* SBP data */
	struct mbsys_jstar_channel_struct sbp;

	/* Sidescan data */
	struct mbsys_jstar_channel_struct ssport;
	struct mbsys_jstar_channel_struct ssstbd;

	/* System Information data */
	struct mbsys_jstar_sysinfo_struct sysinfo;

	/* Pitch Roll data */
	struct mbsys_jstar_pitchroll_struct pitchroll;

	/* NMEA */
	struct mbsys_jstar_nmea_struct nmea;

	/* DVL data */
	struct mbsys_jstar_dvl_struct dvl;

	/* Pressure data */
	struct mbsys_jstar_pressure_struct pressure;

	/* Situation data */
	struct mbsys_jstar_situation_struct situation;

    /* File timestamp data */
    struct mbsys_jstar_filetimestamp_struct filetimestamp;

	/* Comment */
	struct mbsys_jstar_comment_struct comment;
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

#endif  /* MBSYS_JSTAR_H_ */
