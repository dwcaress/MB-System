/*--------------------------------------------------------------------
 *    The MB-system:	mbf_cbat8101.h	8/21/94
 *	$Id: mbf_cbat8101.h,v 5.1 2002-09-18 23:32:59 caress Exp $
 *
 *    Copyright (c) 1998, 2000, 2002 by
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
 * mbf_cbat8101.h defines the data structures used by MBIO functions
 * to store multibeam data read from the MBF_CBAT8101 format (MBIO id 82).  
 *
 * Author:	D. W. Caress
 * Date:	December 10, 1998
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.0  2000/12/01 22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.1  2000/09/30  06:29:44  caress
 * Snapshot for Dale.
 *
 * Revision 4.0  1999/01/01  23:38:01  caress
 * MB-System version 4.6beta6
 *
 *
 */
/*
 * Notes on the MBF_CBAT8101 data format:
 *   1. Reson SeaBat products are high frequency, 
 *      shallow water multibeam sonars.
 *      Reson SeaBat 8101 systems output both bathymetry
 *      and amplitude information for 60 beams.
 *      Reson SeaBat 8101 systems output both bathymetry
 *      and amplitude information for up to 101 beams.
 *      These sonars use fixed, analog beamforming followed
 *      by a combination of amplitude and phase bottom
 *      detection.
 *   2. Reson multibeam systems output raw range and amplitude
 *      data in a binary format. The data acquisition systems 
 *      associated with the sonars calculate bathymetry using 
 *      a water sound velocity, roll, pitch, and heave data.
 *   3. Generally, Reson data acquisition systems record 
 *      navigation asynchronously in the data stream, without
 *      providing speed information. This means that the
 *      navigation must be interpolated on the fly as the
 *      data is read.
 *   4. The navigation is frequently provided in projected
 *      coordinates (eastings and northings) rather than in
 *      longitude and latitude. Since MB-System operates solely
 *      in longitude and latitude, the original navigation must
 *      be unprojected.
 *   5. The Reson data formats supported by MB-System include:
 *        MBF_CBAT8101 - a binary format designed by John Hughes Clarke
 *           of the University of New Brunswick. Parameter and
 *           sound velocity profile records are included.
 *        MBF_CBAT8101 - a clone of the above format supporting
 *           Reson 8101 data.
 *        MBF_HYPC8101 - the ASCII format used by the HYPACK system
 *           of Coastal Oceanographics in conjunction with
 *           Reson 8101 data. This format is supported as read-only 
 *           by MB-System.
 *        MBF_GSFGENMB - the generic sensor format of SAIC which
 *           supports data from a large number of sonars, including
 *           Reson sonars. MB-System handles GSF separately from
 *           other formats.
 *   6. For the UNB-style formats MBF_CBAT8101 and MBF_CBAT8101, 
 *      each data telegram is preceded by a two byte start code and
 *      followed by a three byte end code consisting of 0x03
 *      followed by two bytes representing the checksum for
 *      the data bytes.  MB-System does not calculate checksums
 *      and puts 0's in the checksum bytes.
 *      The relevent telegram start codes, types, and sizes are:
 *         0x0240: Comment***                             200 data bytes
 *         0x0241: Position                                36 data bytes
 *         0x0242: Parameter                               44 data bytes
 *         0x0243: Sound velocity profile                2016 data bytes
 *         0x0244: SeaBat 9001 bathymetry                 752 data bytes
 *         0x0245: Short sound velocity profile           816 data bytes
 *         0x0246: SeaBat 8101 bathymetry***             1244 data bytes
 *         0x0247: Heading***                             752 data bytes
 *         0x0248: Attitude***                            752 data bytes
 *            *** Defined only for MB-System
 *   7. For the ASCII HYPACK format, the following information
 *      is take verbatim from Coastal Oceanographics documentation:
 * 
 *        Coastal Oceanographics, Inc. 
 *        Technical Note: Hypack Raw Data Format
 *        
 *        Data collected by the Hypack Survey program is recorded 
 *        in Raw format, one file per survey line. Raw files are
 *        recorded as text, allowing them to be loaded into any 
 *        text editor that reads large files (Windows Notepad, Write 
 *        and Wordpad for example).
 *        
 *        When inspecting raw files, one of the first things noticed 
 *        is that the format is not tabular. That is, there is not a record for
 *        each sounding containing depth, position, tide corrections, 
 *        etc. Instead, there are separate records for each device
 *        measurement and the correlation between measurements is 
 *        through time tags.
 *        
 *        Every raw file contains two section; a header, which is 
 *        written when data logging starts, and a data section, which is
 *        written as data is collected. Each record starts with a 
 *        three character tag. The tags are:
 *        
 *        Header
 *        ------
 *        DEV - Device Information
 *        EOH - End of Header
 *        EOL - End of Planned Line
 *        FTP - File Type
 *        INF - General Information
 *        LBP - Planned Line Begin Point
 *        LIN - Planned Line Data follows
 *        LNN - Planned Line Name
 *        OFF - Device Offsets
 *        PRD - Private Device Data
 *        PRI - Primary Navigation Device
 *        PTS - Planned Line Waypoint
 *        TND - Survey Time and Date
 *        
 *        Data
 *        ----
 *        FIX - Fix (Event) Mark
 *        HCP - Heave Compensation
 *        EC1 - Echo Sounding (single frequency)
 *        EC2 - Echo Sounding (dual frequency)
 *        ECM - Echo Soundings (multiple transducer system)
 *        GYR - Gyro Data (Heading)
 *        POS - Position
 *        ROX - Roxann data
 *        SB2 - Multibeam data
 *        
 *        --------------
 *        Header Section
 *        --------------
 *        
 *        DEV - Device Information
 *        ------------------------
 *        DEV dn dc "Device Name"
 *        dn: Device Number
 *        dc: Device Capabilities is a bit coded field. The bit definitions are:
 *        Bit Mask Meaning
 *        0 1 Device provides Range/Range positions
 *        1 2 Device provides Range/Azimuth positions
 *        2 4 Device provides Lat/Long (e.g. GPS)
 *        3 8 Device provides grid positions XY
 *        4 16 Device provides echo soundings
 *        5 32 Device provides heading
 *        6 64 Device provides ship speed
 *        7 128 Hypack clock is synched to device clock
 *        8 256 Device provides tides
 *        9 512 Device provides heave, pitch and roll
 *        10 1024 Device is an ROV
 *        11 2048 Device is a Left/Right Indicator
 *        12 4096 Device accepts annotation strings
 *        13 8192 Device accepts output from Hypack
 *        14 16384 xxx
 *        15 32768 Device has extended capabilities
 *        
 *        Example:
 *        DEV 0 100 "GPS"
 *        
 *        INF - General Information
 *        -------------------------
 *        INF "surveyor" "boat" "project" "area" tc dc sv
 *        tc: initial tide correction
 *        dc: initial draft correction
 *        sv: sound velocity
 *        
 *        Example:
 *        INF "steve" "LCH 19" "mcmillen" "617.6 to 618.2" -0.7 0 1500.0
 *        
 *        EOH - End of Header
 *        -------------------
 *        This tag simply indicates end of header and has no data.
 *        
 *        EOL - End of Planned Line
 *        -------------------------
 *        This tag simply indicates end of planned line information no data.
 *        
 *        LBP - Planned Line Begin Point.
 *        -------------------------------
 *        LBP x y
 *        x: x grid position
 *        y: y grid position
 *        
 *        Example:
 *        LBP 5567222.42 3771640.72
 *        
 *        LIN - Planned Line Data follows
 *        -------------------------------
 *        LIN nw
 *        nw: Number of waypoints
 *        
 *        Example:
 *        LIN 5
 *        
 *        LNN - Planned Line Name
 *        -----------------------
 *        LNN text
 *        text: line name or number
 *        
 *        Example:
 *        LNN 14
 *        
 *        OFF - Device Offsets
 *        --------------------
 *        OFF dn n1 n2 n3 n4 n5 n6 n7
 *        dn: device number
 *        n1: starboard, port offset. Positive starboard.
 *        n2: forward, aft offset. Positive forward
 *        n3: height (antenna) or depth (transducer draft) offset. Always positive.
 *        n4: yaw rotation angle. Positive for clockwise rotation.
 *        n5: roll rotation angle. Port side up is positive.
 *        n6: pitch rotation angle. Bow up is positive.
 *        n7: device latency in seconds.
 *        
 *        Example:
 *        OFF 0 0 0 13.35 0 0 0 0.86
 *        
 *        PRD - Private Device Data
 *        -------------------------
 *        PRD - Multiple transducer offset
 *        PRD dn OFF n1 n2 n3
 *        dn: device number
 *        n1: transducer starboard offset
 *        n2: transducer forward offset
 *        n3: transducer depth offset (draft)
 *        
 *        Example:
 *        PRD 1 OFF -25.60 0.00 0.40
 *        
 *        PRD - Odom Echoscan II Multibeam Identifier
 *        -------------------------------------------
 *        PRD dn ECHOSCN2 n1 n2
 *        dn: device number
 *        n1: Not used
 *        n2: Beam width
 *        
 *        Example:
 *        PRD 1 ECHOSCN2 -43.5 3.0
 *        
 *        PRD - Reson Seabat 9001 Multibeam Identifier
 *        --------------------------------------------
 *        PRD dn SEABAT n1 n2
 *        dn: device number
 *        n1: Not used
 *        n2: Beam width
 *        
 *        Example:
 *        PRD 1 SEABAT -44.2 1.5
 *        
 *        PRD - Reson Seabat 9003 Multibeam Identifier
 *        --------------------------------------------
 *        PRD dn SEA9003 n1 n2
 *        dn: device number
 *        n1: Not used
 *        n2: Beam width
 *        
 *        Example:
 *        PRD 1 SEA9003 -60.0 3.0
 *        
 *        PRD - Reson Seabat 8101 Multibeam Identifier
 *        --------------------------------------------
 *        PRD dn SEA8101 n1 n2 n3
 *        dn: device number
 *        n1: Beam 1 angle
 *        n2: Angle increment
 *        n3: Number of beams
 *        
 *        Example:
 *        PRD 1 SEA8101 -75.00 1.50 101
 *        
 *        PRI - Primary Navigation Device
 *        -------------------------------
 *        PRI dn
 *        dn: device number
 *        
 *        Example:
 *        PRI 0
 *        
 *        PTS - Planned Line Waypoint
 *        ---------------------------
 *        PTS x y
 *        x: waypoint easting
 *        y: waypoint northing
 *        
 *        Example:
 *        PTS 5569134.63 3774182.61
 *        
 *        TND - Survey Time and Date
 *        --------------------------
 *        TND t d
 *        t: time string
 *        d: date string
 *        
 *        Example:
 *        TND 15:54:33 08/28/95
 *        
 *        --------------------------
 *        Data Section
 *        --------------------------
 *        
 *        FIX - Fix (Event) Mark
 *        ----------------------
 *        FIX n
 *        n: event number
 *        
 *        Example:
 *        FIX 152
 *        
 *        HCP - Heave Compensation
 *        ------------------------
 *        HCP dn t h r p
 *        dn: device number
 *        t: time tag (seconds past midnight)
 *        h: heave in meters
 *        r: roll in degrees (+ port side up)
 *        p: pitch in degrees (+ bow up)
 *        
 *        Example:
 *        HCP 2 57273.81 0 3.61 0
 *        
 *        EC1 - Echo Sounding (single frequency)
 *        --------------------------------------
 *        EC1 dn t rd
 *        dn: device number
 *        t: time tag (seconds past midnight)
 *        rd: raw depth
 *        
 *        Example:
 *        EC1 0 48077.365 3.20
 *        
 *        EC2 - Echo Sounding (dual frequency)
 *        ------------------------------------
 *        EC2 dn t rd1 rd2
 *        dn: device number
 *        t: time tag (seconds past midnight)
 *        rd1: raw depth 1
 *        rd2: raw depth 2
 *        
 *        Example:
 *        EC2 0 48077.365 3.20 3.15
 *        
 *        ECM - Echo Soundings (multiple transducer system)
 *        -------------------------------------------------
 *        ECM dn t n rd1 rd2 ... rdn
 *        dn: device number
 *        t: time tag (seconds past midnight)
 *        n: number of transducers
 *        rd1: raw depth 1
 *        rd2: raw depth 2
 *        ...
 *        rdn: raw depth, transducer n
 *        
 *        Example:
 *        ECM 1 57274.82 9 11 10.8 10.7 11.4 11.8 13 15.1 15.5 15.6
 *        
 *        GYR - Gyro Data (Heading)
 *        -------------------------
 *        GYR dn t h
 *        dn: device number
 *        t: time tag (seconds past midnight)
 *        h: ship heading angle
 *        
 *        Example:
 *        GYR 0 57274.04 193
 *        
 *        POS - Position
 *        --------------
 *        POS dn t x y
 *        dn: device number
 *        t: time tag (seconds past midnight)
 *        x: easting
 *        y: northing
 *        
 *        Example:
 *        POS 0 57274.04 5569070.02 3774080.46
 *        
 *        ROX - Roxann data
 *        -----------------
 *        ROX dn t n e1 e2
 *        dn: device number
 *        t: time tag (seconds past midnight)
 *        n: number of values to follow (always 2)
 *        e1: roxann e1 measurement
 *        e2: roxann e2 measurement
 *        
 *        Example:
 *        ROX 2 48077.474 2 0.03 0.13
 *        
 *        SB2 - Multibeam data
 *        --------------------
 *        SB2 dn t n sv r1 r2 r3 ... rn q1 q2 ... qn
 *        dn: device number
 *        t: time tag (seconds past midnight)
 *        n: number of values to follow. Depends on device type.
 *        sv: sound velocity from device.
 *        r1-n: ranges in device units.
 *        q1-n: quality codes (0 to 3 range, 0=bad). Packed 4 per number.
 *        
 *        Example (Echoscan II):
 *        SB2 1 48077.474 39 1500.00 19.50 19.31 ...
 *        
 *        Example (Seabat 9001):
 *        SB2 1 48077.474 76 1500.00 19.50 19.31 ...
 *        
 *        Example (Seabat 9003):
 *        SB2 1 48077.474 51 1500.00 19.50 19.31 ...
 *        
 *        Example (Seabat 8101 using 101 beams):
 *        SB2 1 48077.474 51 1500.00 19.50 19.31 ...
 *
 */

/* maximum number of beams and pixels */
#define	MBF_CBAT8101_MAXBEAMS	101
#define	MBF_CBAT8101_COMMENT_LENGTH	200

struct mbf_cbat8101_struct
	{
	/* type of data record */
	int	kind;			/* Data vs Comment */

	/* type of sonar */
	int	sonar;			/* Type of Reson sonar */

	/* parameter info (parameter telegrams) */
	int	par_year;
	int	par_month;
	int	par_day;
	int	par_hour;
	int	par_minute;
	int	par_second;
	int	par_hundredth_sec;
	int	par_thousandth_sec;
	short	roll_offset;	/* roll offset (degrees) */
	short	pitch_offset;	/* pitch offset (degrees) */
	short	heading_offset;	/* heading offset (degrees) */
	short	time_delay;	/* positioning system delay (sec) */
	short	transducer_depth;	/* tranducer depth (meters) */
	short	transducer_height;	/* reference height (meters) */
	short	transducer_x;	/* reference fore-aft offset (meters) */
	short	transducer_y;	/* reference athwartships offset (meters) */
	short	antenna_x;	/* antenna athwartships offset (meters) */
	short	antenna_y;	/* antenna athwartships offset (meters) */
	short	antenna_z;	/* antenna height (meters) */
	short	motion_sensor_x;/* motion sensor athwartships offset (meters) */
	short	motion_sensor_y;/* motion sensor athwartships offset (meters) */
	short	motion_sensor_z;/* motion sensor height offset (meters) */
	short	spare;
	short	line_number;
	short	start_or_stop;
	short	transducer_serial_number;

	/* comment */
	char	comment[MBSYS_RESON_COMMENT_LENGTH];

	/* position (position telegrams) */
	int	pos_year;
	int	pos_month;
	int	pos_day;
	int	pos_hour;
	int	pos_minute;
	int	pos_second;
	int	pos_hundredth_sec;
	int	pos_thousandth_sec;
	int	pos_latitude;		/* 180 deg = 2e9 */
	int	pos_longitude;		/* 180 deg = 2e9 */
	int	utm_northing;		/* 0.01 m */
	int	utm_easting;		/* 0.01 m */
	int	utm_zone_lon;		/* 180 deg = 2e9 */
	char	utm_zone;
	char	hemisphere;
	char	ellipsoid;
	char	pos_spare;
	int	semi_major_axis;
	int	other_quality;

	/* sound velocity profile */
	int	svp_year;
	int	svp_month;
	int	svp_day;
	int	svp_hour;
	int	svp_minute;
	int	svp_second;
	int	svp_hundredth_sec;
	int	svp_thousandth_sec;
	int	svp_latitude;		/* 180 deg = 2e9 */
	int	svp_longitude;		/* 180 deg = 2e9 */
	int	svp_num;
	int	svp_depth[500]; /* 0.1 meters */
	int	svp_vel[500];	/* 0.1 meters/sec */

	/* bathymetry */
	int	year;
	int	month;
	int	day;
	int	hour;
	int	minute;
	int	second;
	int	hundredth_sec;
	int	thousandth_sec;
	int	latitude;		/* 180 deg = 2e9 */
	int	longitude;		/* 180 deg = 2e9 */
	int	roll;			/* 0.005 degrees */
	int	pitch;			/* 0.005 degrees */
	int	heading;		/* 0.01 degrees */
	int	heave;			/* 0.001 meters */
	int	ping_number;
	int	sound_vel;	/* 0.1 meters/sec */
	int	mode;		/* unused */
	int	gain1;		/* unused */
	int	gain2;		/* unused */
	int	gain3;		/* unused */
	int	beams_bath;
	short bath[MBSYS_RESON_MAXBEAMS];
				/* depths:  0.01 meters */	
	short int bath_acrosstrack[MBSYS_RESON_MAXBEAMS];
				/* acrosstrack distances: 0.01 meters */
	short int bath_alongtrack[MBSYS_RESON_MAXBEAMS];
				/* alongtrack distances: 0.01 meters */
	short int tt[MBSYS_RESON_MAXBEAMS];
				/* travel times:         0.05 msec */
	short int angle[MBSYS_RESON_MAXBEAMS];		
				/* 0.005 degrees */
	short int quality[MBSYS_RESON_MAXBEAMS];
				/* 0 (bad) to 3 (good) */
	short int amp[MBSYS_RESON_MAXBEAMS];
				/* ??? */

	};

