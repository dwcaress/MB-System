/*--------------------------------------------------------------------
 *    The MB-system:	mbf_xtfr8101.h	8/24/01
 *	$Id: mbf_xtfr8101.h 1892 2011-05-04 23:54:50Z caress $
 *
 *    Copyright (c) 2001-2011 by
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
 * mbf_xtfr8101.h defines the data structures used by MBIO functions
 * to store multibeam data read from the MBF_XTFR8101 format (MBIO id 83).  
 *
 * Author:	D. W. Caress
 * Date:	August 24, 2001
 *
 * $Log: mbf_xtfr8101.h,v $
 * Revision 5.2  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.1  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.0  2001/09/17 23:24:10  caress
 * Added XTF format.
 *
 *
 *
 *
 */
/*
 * Notes on the MBF_XTFR8101 data format:
 *   1. Reson SeaBat products are high frequency, 
 *
 */

/* maximum number of beams and pixels */
#define	MBF_XTFR8101_MAXBEAMS		240
#define	MBF_XTFR8101_MAXRAWPIXELS	8192
#define	MBF_XTFR8101_COMMENT_LENGTH	200
#define	MBF_XTFR8101_MAXLINE		16384
#define	MBF_XTFR8101_FILEHEADERLEN	1024
#define XTF_MAGIC_NUMBER		0xFACE
#define XTF_DATA_SIDESCAN		0
#define XTF_DATA_ANNOTATION		1
#define XTF_DATA_BATHYMETRY		2
#define XTF_DATA_ATTITUDE		3
#define XTF_DATA_POSITION		100

#define RESON_PACKETID_RT_VERY_OLD	0x11
#define RESON_PACKETID_RIT_VERY_OLD	0x12
#define RESON_PACKETID_RT_OLD		0x13
#define RESON_PACKETID_RIT_OLD		0x14
#define RESON_PACKETID_RT		0x17
#define RESON_PACKETID_RIT		0x18

struct mbf_xtfr8101_xtfchaninfo
	{
	char		TypeOfChannel;
	char		SubChannelNumber;
	unsigned short	CorrectionFlags;
	unsigned short	UniPolar;
	unsigned short	BytesPerSample;
	unsigned int	SamplesPerChannel;
	char		ChannelName[16];
	float		VoltScale;		/* maximum value (V) */
	float		Frequency;		/* Hz */
	float		HorizBeamAngle;		/* degrees */
	float		TiltAngle;
	float		BeamWidth;
	float		OffsetX;
	float		OffsetY;
	float		OffsetZ;
	float		OffsetYaw;
	float		OffsetPitch;
	float		OffsetRoll;
	char		ReservedArea[56];
	};

struct mbf_xtfr8101_xtffileheader
	{
	char		FileFormat;		/* Set to 123 (0x7B */
	char		SystemType;		/* Set to 1 */
	char		RecordingProgramName[8];
	char		RecordingProgramVersion[8];
	char		SonarName[16];
	unsigned short	SonarType;		/* 	0 = Reserved
							1 = JAMSTEC
							2 = Analog_c31
							3 = SIS1000
							4 = Analog_32chan
							5 = Klein2000
							6 = RWS
							7 = DF1000
							8 = SeaBat 9001
							9 = Klein595
							10 = EGG260
							11 = Sonatech_DDS
							12 = Echoscan
							13 = Elac Bottomchart 1180
							14 = Klein 5000	
							15 = Reson SeaBat 8101	
							16 = Imagenex model 858	
							17 = USN SILOS with 3-channel analog	
							18 = Sonatech super high res sidescan sonar	
							19 = Delph AU32 Analog input (2 channel)	
							20 = Generic sonar using the memory-mapped
								file interface	
							21 = Simrad SM2000	
							22 = Standard multibedia audio	
							23 = Edgetech ACI card for 260 sonar throug PC31 card
							24 = Edgetech black box	
							25 = Fugro deeptow	
							26 = C&C Edgetech chirp conversion program	
							27 = DTI SAS synthetic aperture processor (mmap file)	
							*/
	char		NoteString[64];
	char		ThisFileName[64];
	unsigned short	NavUnits;		/* 0 = meters, 3 = degrees */
	unsigned short	NumberOfSonarChannels;	/* if <= 6 use 1024 byte header,
							if > 6 use 2048 byte header */
	unsigned short	NumberOfBathymetryChannels;
	unsigned short	Reserved1;
	unsigned short	Reserved2;
	unsigned short	Reserved3;
	unsigned short	Reserved4;
	unsigned short	Reserved5;
	unsigned short	Reserved6;
	char		ProjectionType[12];	/* not currently used */
	char		SpheroidType[10];	/* not currently used */
	int		NavigationLatency;	/* GPS_time_received - GPS_time_sent (msec) */
	float		OriginY;		/* not currently used */
	float		OriginX; 		/* not currently used */
	float		NavOffsetY;		/* Multibeam nav offset (m) */
	float		NavOffsetX;		/* Multibeam nav offset (m) */
	float		NavOffsetZ; 		/* Multibeam nav z offset (m) */
	float		NavOffsetYaw;		/* Multibeam heading offset (m) */
	float		MRUOffsetY;		/* Multibeam MRU y offset (m) */
	float		MRUOffsetX;		/* Multibeam MRU x offset (m) */
	float		MRUOffsetZ; 		/* Multibeam MRU z offset (m) */
	float		MRUOffsetYaw;		/* Multibeam MRU heading offset (m) */
	float		MRUOffsetPitch; 	/* Multibeam MRU pitch offset (degrees) */
	float		MRUOffsetRoll;		/* Multibeam MRU roll offset (degrees) */
	struct mbf_xtfr8101_xtfchaninfo chaninfo[6];
	
	};

struct mbf_xtfpacketheader
	{
	mb_u_char	MagicNumber[2];		/* 0xFACE */
	mb_u_char	HeaderType;		/* 0 = sonar ping, 1 = annotation, 2 = bathymetry, 3 = attitude */
	mb_u_char	SubChannelNumber;	/* which multibeam head */
	unsigned short	NumChansToFollow;	/* Number of beams to follow */
	unsigned short	Reserved1[2];
	unsigned int	NumBytesThisRecord;	/* total byte count including this header */
	};
	
struct mbf_xtfattitudeheader
	{
	struct mbf_xtfpacketheader packetheader;
	unsigned int	Reserved2[4];
	float		Pitch;
	float		Roll;
	float		Heave;
	float		Yaw;
	unsigned int	TimeTag;		/* time tag (msec) */
	float		Heading;
	char		Reserved3[10];
	};
	
struct mbf_xtfbathheader
	{
	struct mbf_xtfpacketheader packetheader;
	unsigned short	Year;
	mb_u_char	Month;
	mb_u_char	Day;
	mb_u_char	Hour;
	mb_u_char	Minute;
	mb_u_char	Second;
	mb_u_char	HSeconds;
	unsigned short	JulianDay;
	unsigned short	CurrentLineID;
	unsigned short	EventNumber;
	unsigned int	PingNumber;
	float		SoundVelocity;		/* Half sound speed (e.g. 750 m/sec instead of 1500 m/sec) */
	float		OceanTide;		/* (m) */
	unsigned int	Reserved2;
	float		ConductivityFreq;	/* Raw CTD conductivity frequency (Hz) */
	float		TemperatureFreq;	/* Raw CTD temperature frequency (Hz) */
	float		PressureFreq;		/* Raw CTD pressure frequency (Hz) */
	float		PressureTemp;		/* Raw CTD pressure temperature (deg C) */
	float		Conductivity;		/* computed CTD conductivity (siemens/m) */
	float		WaterTemperature;	/* computed CTD temperature (deg C) */
	float		Pressure;		/* computed CTD water pressure (psia) */
	float		ComputedSoundVelocity;	/* water sound velocity (m/sec) */
	float		MagX;			/* X-axis magnetometer (mGauss) */
	float		MagY;			/* Y-axis magnetometer (mGauss) */
	float		MagZ;			/* Z-axis magnetometer (mGauss) */
	float		AuxVal1;
	float		AuxVal2;
	float		AuxVal3;
	float		AuxVal4;
	float		AuxVal5;
	float		AuxVal6;
	float		SpeedLog;	/* towfish speed (kts) */
	float		Turbidity;	/* turbity (0-5V * 10000) */
	float		ShipSpeed;	/* ship speed (kts) */
	float		ShipGyro;	/* ship heading (deg) */
	double		ShipYcoordinate;/* ship latitude or northing */
	double		ShipXcoordinate;/* ship longitude or easting */
	short		ShipAltitude;	/* ship altitude (decimeters) */
	short		ShipDepth;	/* ship depth (decimeters) */
	mb_u_char	FixTimeHour;	/* last nav fix time (hour) */
	mb_u_char	FixTimeMinute;	/* last nav fix time (min) */
	mb_u_char	FixTimeSecond;	/* last nav fix time (sec) */
	char		Reserved4;
	float		SensorSpeed; 	/* towfish speed (kts) */
	float		KP;		/* kilometers pipe (km) */
	double		SensorYcoordinate; /* towfish latitude or northing */
	double		SensorXcoordinate; /* towfish longitude or easting */
	short		Reserved6;
	short		RangeToSensor;	/* slant range to towfish * 10 */
	short		BearingToSensor;/* bearing to towfish * 100 */
	short		CableOut;	/* cable out (m) */
	float		Layback;	/* distance from ship to sensor (m) */
	float		CableTension;	/* cable tension */
	float		SensorDepth;	/* towfish depth (m) */
	float		SensorPrimaryAltitude; /* towfish altitude (m) */
	float		SensorAuxAltitude; /* towfish altitude (m) */
	float		SensorPitch;	/* sensor pitch (deg) */
	float		SensorRoll;	/* sensor roll (deg) */
	float		SensorHeading;	/* sensor heading (deg) */
	float		Heave;		/* sensor heave (m) */
	float		Yaw;		/* sensor yaw (deg) */
	int		AttitudeTimeTag;/* time tag from MRU */
	float		DOT;		/* distance off track */
	char		ReservedSpace[20];
	};

struct mbf_xtfpingchanheader {

   unsigned short	ChannelNumber;	/* Typically,  */
					/* 0=port (low frequency) */
					/* 1=stbd (low frequency) */
					/* 2=port (high frequency) */
					/* 3=stbd (high frequency) */

   unsigned short	DownsampleMethod;   /* 2=MAX, 4=RMS */
   float		SlantRange;	/* Slant range of the data in meters */
   float		GroundRange;	/* Ground range of the data in meters */
					/*   (SlantRange^2 - Altitude^2) */
   float		TimeDelay;	/* Amount of time (in seconds) to the start of recorded data */
					/*   almost always 0.0 */
   float		TimeDuration;	/* Amount of time (in seconds) recorded */
   float		SecondsPerPing;	/* Amount of time (in seconds) from ping to ping */

   unsigned short	ProcessingFlags;/* 4=TVG, 8=BAC&GAC, 16=Filter, etc... */
					/*   almost always 0 */
   unsigned short	Frequency;	/* Center transmit frequency for this channel. */
					/*   when non-zero, replaces value found in file */
					/*   header CHANINFO struct ChanInfo->SamplesPerChannel. */
					/*   This allows samples per channel to change on the fly. */

   unsigned short	InitialGainCode;/* Settings as transmitted by sonar */
   unsigned short	GainCode;
   unsigned short	BandWidth;

   /* Contact information - updated when contacts are saved through Target.exe */
   unsigned int		ContactNumber;
   unsigned short	ContactClassification;
   mb_u_char		ContactSubNumber;
   mb_u_char		ContactType;


   unsigned int		NumSamples;	/* Number of samples that will follow this structure.  The */
					/* number of bytes will be this value multipied by the */
					/* number of bytes per sample (given in the file header) */

   unsigned short	Reserved;	/* Obsolete. */
   float		ContactTimeOffTrack;	/* Time off track to this contact (stored in milliseconds) */
   mb_u_char	ContactCloseNumber;
   mb_u_char	Reserved2;

   float		FixedVSOP;	/* Fixed along-track size of each ping, stored in cm. */
					/*  on multibeam system with zero beam spread, this value */
					/*  needs to be filled in to prevent Isis from calculating */
					/*  along-track ground coverage based on beam spread and  */
					/*  speed over ground.  In order for */
					/* Target to use this number, "223" */
					/* or later must be set in */
					/* XTFFILEHEADER.RecordingProgramVersion */

   mb_u_char	ReservedSpace[6];   /* reserved for future expansion */

};

struct 	RESON8100_RT_VERY_OLD
{
   	char		synch_header[4];		/* synch header {0xff, 0xff, 0x00, 0x00 */
   	char      	packet_type;      		/* identifier for packet type (0x11) */
   	char           	packet_subtype;   		/* identifier for packet subtype */
	unsigned int	Seconds;	/* seconds since 00:00:00, 1 January 1970 */
	unsigned int	Millisecs;	/* milliseconds, LSB = 1 ms */
   	unsigned short 	latency;          		/* time from ping to output (milliseconds) */
   	unsigned short 	velocity;         		/* programmed sound velocity (LSB = 1 m/sec) */
   	unsigned short 	sample_rate;      		/* A/D sample rate (samples per second) */
   	mb_u_char  	pulse_width;      		/* transmit pulse width (microseconds) */
   	unsigned short 	ping_rate;        		/* Ping rate (pings per second * 1000) */
   	unsigned short 	range_set;        		/* range setting for SeaBat (meters ) */
   	unsigned short 	power;            		/* power setting for SeaBat  	 */
						/* bits	0-4 -	power (0 - 8) */
						/* bit	15	(0 = manual, 1 = auto) */
	unsigned short 	gain;             		/* gain setting for SeaBat */
					/* bits	0-6 -	gain (1 - 45) */
					/* bit 	14	(0 = fixed, 1 = tvg) */
					/* bit	15	(0 = manual, 1 = auto) */
	short          	projector;        		/* projector setting */
	mb_u_char  	beam_width;       		/* cross track receive beam width (degrees * 10) */
	short          	beam_count;       		/* number of sets of beam data in packet */
	unsigned short 	range[MBF_XTFR8101_MAXBEAMS]; 		/* range for beam where n = Beam Count */
	mb_u_char  	quality[MBF_XTFR8101_MAXBEAMS/2];   		/* packed quality array (two 4 bit values/char) */
						/* bit 0 - brightness test (0=failed, 1=passed) */
						/* bit 1 - colinearity test (0=failed, 1=passed) */
						/* bit 2 - amplitude bottom detect used */
						/* bit 3 - phase bottom detect used */
						/* bottom detect can be amplitude, phase or both */
	unsigned short 	checksum;         		/* checksum for data packet */
};

struct 	RESON8100_RIT_VERY_OLD
{
	char		synch_header[4];		/* synch header {0xff, 0xff, 0x00, 0x00 */
   	char      	packet_type;      		/* identifier for packet type (0x12) */
   	char           	packet_subtype;   		/* identifier for packet subtype */
	unsigned int	Seconds;	/* seconds since 00:00:00, 1 January 1970 */
	unsigned int	Millisecs;	/* milliseconds, LSB = 1 ms */
   	unsigned short 	latency;          		/* time from ping to output (milliseconds) */
   	unsigned short 	velocity;         		/* programmed sound velocity (LSB = 1 m/sec) */
   	unsigned short 	sample_rate;      		/* A/D sample rate (samples per second) */
   	mb_u_char  	pulse_width;      		/* transmit pulse width (microseconds) */
   	unsigned short 	ping_rate;        		/* Ping rate (pings per second * 1000) */
   	unsigned short 	range_set;        		/* range setting for SeaBat (meters ) */
   	unsigned short 	power;            		/* power setting for SeaBat */
						/* bits	0-4 -	power (0 - 8) */
						/* bit	15	(0 = manual, 1 = auto) */
  	unsigned short 	gain;             		/* gain setting for SeaBat */
					/* bits	0-6 -	gain (1 - 45) */
					/* bit 	14	(0 = fixed, 1 = tvg) */
					/* bit	15	(0 = manual, 1 = auto) */
  	short          	projector;        		/* projector setting */
	mb_u_char  	beam_width;       		/* cross track receive beam width (degrees * 10) */
	short          	beam_count;       		/* number of sets of beam data in packet */
	unsigned short 	range[MBF_XTFR8101_MAXBEAMS]; 		/* range for beam where n = Beam Count */
	mb_u_char  	quality[MBF_XTFR8101_MAXBEAMS/2];   		/* packed quality array (two 4 bit values/char) */
						/* bit 0 - brightness test (0=failed, 1=passed) */
						/* bit 1 - colinearity test (0=failed, 1=passed) */
						/* bit 2 - amplitude bottom detect used */
						/* bit 3 - phase bottom detect used */
						/* bottom detect can be amplitude, phase or both */
	mb_u_char	intensity[MBF_XTFR8101_MAXBEAMS];   		/* intensities at bottom detect */
	unsigned short 	checksum;         		/* checksum for data packet */
};

struct 	RESON8100_RT_OLD
{
   	char		synch_header[4];		/* synch header {0xff, 0xff, 0x00, 0x00 */
   	char      	packet_type;      		/* identifier for packet type (0x13) */
   	char           	packet_subtype;   		/* identifier for packet subtype */
	unsigned int	Seconds;	/* seconds since 00:00:00, 1 January 1970 */
	unsigned int	Millisecs;	/* milliseconds, LSB = 1 ms */
   	unsigned short 	latency;          		/* time from ping to output (milliseconds) */
   	unsigned short 	velocity;         		/* programmed sound velocity (LSB = 1 m/sec) */
   	unsigned short 	sample_rate;      		/* A/D sample rate (samples per second) */
   	mb_u_char  	pulse_width;      		/* transmit pulse width (microseconds) */
   	unsigned short 	ping_rate;        		/* Ping rate (pings per second * 1000) */
   	unsigned short 	range_set;        		/* range setting for SeaBat (meters ) */
   	unsigned short 	power;            		/* power setting for SeaBat  	 */
						/* bits	0-4 -	power (0 - 8) */
						/* bit	15	(0 = manual, 1 = auto) */
	unsigned short 	gain;             		/* gain setting for SeaBat */
					/* bits	0-6 -	gain (1 - 45) */
					/* bit 	14	(0 = fixed, 1 = tvg) */
					/* bit	15	(0 = manual, 1 = auto) */
	short          	projector;        		/* projector setting  */
	mb_u_char	tvg_spread;		/* spreading coefficient for tvg * 4  */
						/* valid values = 0 to 240 (0.0 to 60.0 in 0.25 steps) */
	mb_u_char	tvg_absorp;		/* absorption coefficient for tvg */
	mb_u_char  	beam_width;       		/* cross track receive beam width (degrees * 10) */
	short          	beam_count;       		/* number of sets of beam data in packet */
	unsigned short 	range[MBF_XTFR8101_MAXBEAMS]; 		/* range for beam where n = Beam Count */
						/* range units = sample cells * 4 */
	mb_u_char  	quality[MBF_XTFR8101_MAXBEAMS/2];   		/* packed quality array (two 4 bit values/char) */
						/* bit 0 - brightness test (0=failed, 1=passed) */
						/* bit 1 - colinearity test (0=failed, 1=passed) */
						/* bit 2 - amplitude bottom detect used */
						/* bit 3 - phase bottom detect used */
						/* bottom detect can be amplitude, phase or both */
	unsigned short 	checksum;         		/* checksum for data packet */
};

struct 	RESON8100_RIT_OLD
{
   	char		synch_header[4];		/* synch header {0xff, 0xff, 0x00, 0x00 */
   	char      		packet_type;      		/* identifier for packet type (0x14) */
   	char           	packet_subtype;   	/* identifier for packet subtype */
	unsigned int	Seconds;	/* seconds since 00:00:00, 1 January 1970 */
	unsigned int	Millisecs;	/* milliseconds, LSB = 1 ms */
   	unsigned short 	latency;          		/* time from ping to output (milliseconds) */
   	unsigned short 	velocity;         		/* programmed sound velocity (LSB = 1 m/sec) */
   	unsigned short 	sample_rate;      		/* A/D sample rate (samples per second) */
   	mb_u_char  	pulse_width;      		/* transmit pulse width (microseconds) */
   	unsigned short 	ping_rate;        		/* Ping rate (pings per second * 1000) */
   	unsigned short 	range_set;        		/* range setting for SeaBat (meters ) */
   	unsigned short 	power;            		/* power setting for SeaBat  	 */
						/* bits	0-4 -	power (0 - 8) */
						/* bit	15	(0 = manual, 1 = auto) */
	unsigned short 	gain;             		/* gain setting for SeaBat */
					/* bits	0-6 -	gain (1 - 45) */
					/* bit 	14	(0 = fixed, 1 = tvg) */
					/* bit	15	(0 = manual, 1 = auto) */
	short          	projector;        		/* projector setting  */
	mb_u_char	tvg_spread;		/* spreading coefficient for tvg * 4  */
						/* valid values = 0 to 240 (0.0 to 60.0 in 0.25 steps) */
	mb_u_char	tvg_absorp;		/* absorption coefficient for tvg */
	mb_u_char  	beam_width;       		/* cross track receive beam width (degrees * 10) */
	short          	beam_count;       		/* number of sets of beam data in packet */
	unsigned short 	range[MBF_XTFR8101_MAXBEAMS]; 		/* range for beam where n = Beam Count */
						/* range units = sample cells * 4 */
	mb_u_char  	quality[MBF_XTFR8101_MAXBEAMS];   		/* unpacked quality array (one 8 bit value/char) */
						/* bit 0 - brightness test (0=failed, 1=passed) */
						/* bit 1 - colinearity test (0=failed, 1=passed) */
						/* bit 2 - amplitude bottom detect used */
						/* bit 3 - phase bottom detect used */
						/* bottom detect can be amplitude, phase or both */
	unsigned short	intensity[MBF_XTFR8101_MAXBEAMS];   		/* intensities at bottom detect * 8 */
	unsigned short 	checksum;         		/* checksum for data packet */
};

struct 	RESON8100_RT
{
   	char		synch_header[4];		/* synch header {0xff, 0xff, 0x00, 0x00 */
   	char      		packet_type;      		/* identifier for packet type (0x17) */
   	char           	packet_subtype;   	/* identifier for packet subtype */
						/* for dual head system, most significant bit (bit 7) */
						/* indicates which sonar head to associate with packet */
						/* 	head 1 - bit 7 set to 0 */
						/* 	head 2 -	bit 7 set to 1 	 */	
	unsigned short 	latency;          		/* time from ping to output (milliseconds) */
	unsigned int	Seconds;	/* seconds since 00:00:00, 1 January 1970 */
	unsigned int	Millisecs;	/* milliseconds, LSB = 1 ms */
	unsigned int	ping_number;		/* sequential ping number from sonar startup/reset */
	unsigned int	sonar_id;		/* least significant four bytes of Ethernet address */
	unsigned short     sonar_model;		/* coded model number of sonar */
	unsigned short	frequency;		/* sonar frequency in KHz */
	unsigned short 	velocity;         		/* programmed sound velocity (LSB = 1 m/sec) */
   	unsigned short 	sample_rate;      		/* A/D sample rate (samples per second) */
	unsigned short 	ping_rate;        		/* Ping rate (pings per second * 1000) */
   	unsigned short 	range_set;        		/* range setting for SeaBat (meters ) */
   	unsigned short 	power;            		/* power setting for SeaBat  	 */
						/* bits	0-4 -	power (0 - 8) */
	unsigned short 	gain;             		/* gain setting for SeaBat */
					/* bits	0-6 -	gain (1 - 45) */
					/* bit 	14	(0 = fixed, 1 = tvg) */
					/* bit	15	(0 = manual, 1 = auto) */
	unsigned short  	pulse_width;      		/* transmit pulse width (microseconds) */
	mb_u_char	tvg_spread;		/* spreading coefficient for tvg * 4  */
						/* valid values = 0 to 240 (0.0 to 60.0 in 0.25 steps) */
	mb_u_char	tvg_absorp;		/* absorption coefficient for tvg */
	mb_u_char     	projector_type;      	/* bits 0-4 = projector type */
						/* 0 = stick projector */
						/* 1 = array face */
						/* 2 = ER projector */
						/* bit 7 - pitch steering (1=enabled, 0=disabled) */
	mb_u_char      projector_beam_width;	/* along track transmit beam width (degrees * 10) */
	unsigned short  	beam_width_num;   	/* cross track receive beam width numerator */
	unsigned short 	beam_width_denom; 	/* cross track receive beam width denominator */
						/* beam width degrees = numerator / denominator */
	short		projector_angle;		/* projector pitch steering angle (degrees * 100) */
	unsigned short	min_range;		/* sonar filter settings */
	unsigned short	max_range;
	unsigned short	min_depth;
	unsigned short	max_depth;
	mb_u_char	filters_active;		/* range/depth filters active  */
						/* bit 0 - range filter (0 = off, 1 = active) */
						/* bit 1 - depth filter (0 = off, 1 = active) */
	mb_u_char	spare[3];			/* spare field for future growth */
	short		temperature;		/* temperature at sonar head (deg C * 10) */
	short          	beam_count;       		/* number of sets of beam data in packet */
	unsigned short 	range[MBF_XTFR8101_MAXBEAMS]; 		/* range for beam where n = Beam Count */
						/* range units = sample cells * 4 */
	mb_u_char  	quality[MBF_XTFR8101_MAXBEAMS/2+1];   		/* packed quality array (two 4 bit values/char) */
						/* cnt = n/2 if beam count even, n/2+1 if odd */
						/* cnt then rounded up to next even number */
						/* e.g. if beam count=101, cnt=52  */
						/* unused trailing quality values set to zero */
						/* bit 0 - brightness test (0=failed, 1=passed) */
						/* bit 1 - colinearity test (0=failed, 1=passed) */
						/* bit 2 - amplitude bottom detect used */
						/* bit 3 - phase bottom detect used */
						/* bottom detect can be amplitude, phase or both */
	unsigned short 	checksum;         		/* checksum for data packet */
};

struct 	RESON8100_RIT
{
	char		synch_header[4];		/* synch header {0xff, 0xff, 0x00, 0x00 */
   	char      	packet_type;      		/* identifier for packet type (0x18) */
	char           	packet_subtype;   	/* identifier for packet subtype */
						/* for dual head system, most significant bit (bit 7) */
						/* indicates which sonar head to associate with packet */
						/* 	head 1 - bit 7 set to 0 */
						/* 	head 2 -	bit 7 set to 1 		 */
	unsigned short 	latency;          		/* time from ping to output (milliseconds) */
	unsigned int	Seconds;	/* seconds since 00:00:00, 1 January 1970 */
	unsigned int	Millisecs;	/* milliseconds, LSB = 1 ms */
	unsigned int	ping_number;		/* sequential ping number from sonar startup/reset */
	unsigned int	sonar_id;		/* least significant four bytes of Ethernet address */
	unsigned short     sonar_model;		/* coded model number of sonar */
	unsigned short	frequency;		/* sonar frequency in KHz */
	unsigned short 	velocity;         		/* programmed sound velocity (LSB = 1 m/sec) */
   	unsigned short 	sample_rate;      		/* A/D sample rate (samples per second) */
	unsigned short 	ping_rate;        		/* Ping rate (pings per second * 1000) */
   	unsigned short 	range_set;        		/* range setting for SeaBat (meters ) */
   	unsigned short 	power;            		/* power setting for SeaBat  	 */
						/* bits	0-4 -	power (0 - 8) */
	unsigned short 	gain;             		/* gain setting for SeaBat */
					/* bits	0-6 -	gain (1 - 45) */
					/* bit 	14	(0 = fixed, 1 = tvg) */
					/* bit	15	(0 = manual, 1 = auto) */
	unsigned short  	pulse_width;      		/* transmit pulse width (microseconds) */
	mb_u_char	tvg_spread;		/* spreading coefficient for tvg * 4  */
						/* valid values = 0 to 240 (0.0 to 60.0 in 0.25 steps) */
	mb_u_char	tvg_absorp;		/* absorption coefficient for tvg */
	mb_u_char     	projector_type;      	/* bits 0-4 = projector type */
						/* 0 = stick projector */
						/* 1 = array face */
						/* 2 = ER projector */
						/* bit 7 - pitch steering (1=enabled, 0=disabled) */
	mb_u_char      projector_beam_width;	/* along track transmit beam width (degrees * 10) */
	unsigned short  	beam_width_num;   	/* cross track receive beam width numerator */
	unsigned short 	beam_width_denom; 	/* cross track receive beam width denominator */
						/* beam width degrees = numerator / denominator */
	short		projector_angle;		/* projector pitch steering angle (degrees * 100) */
	unsigned short	min_range;		/* sonar filter settings */
	unsigned short	max_range;
	unsigned short	min_depth;
	unsigned short	max_depth;
	mb_u_char	filters_active;		/* range/depth filters active  */
						/* bit 0 - range filter (0 = off, 1 = active) */
						/* bit 1 - depth filter (0 = off, 1 = active) */
	mb_u_char	spare[3];			/* spare field for future growth */
	short		temperature;		/* temperature at sonar head (deg C * 10) */
	short          	beam_count;       		/* number of sets of beam data in packet */
	unsigned short 	range[MBF_XTFR8101_MAXBEAMS]; 		/* range for beam where n = Beam Count */
						/* range units = sample cells * 4 */
	mb_u_char  	quality[MBF_XTFR8101_MAXBEAMS/2+1];   		/* packed quality array (two 4 bit values/char) */
						/* cnt = n/2 if beam count even, n/2+1 if odd */
						/* cnt then rounded up to next even number */
						/* e.g. if beam count=101, cnt=52  */
						/* unused trailing quality values set to zero */
						/* bit 0 - brightness test (0=failed, 1=passed) */
						/* bit 1 - colinearity test (0=failed, 1=passed) */
						/* bit 2 - amplitude bottom detect used */
						/* bit 3 - phase bottom detect used */
						/* bottom detect can be amplitude, phase or both */
	unsigned short	intensity[MBF_XTFR8101_MAXBEAMS];   		/* intensities at bottom detect * 8 */
	unsigned short 	checksum;         		/* checksum for data packet */
};

struct	RESON_STATUS
{
	char			Synch[4];	/* synch header {0xff, 0xff, 0x00, 0x00 } */
	char			PacketType;	/* identifier for packet type (0x70) */
	unsigned short		ErrorCode;	/* bit mapped error codes */
						/* bit 0	=	leak sensor */
						/* bit 1	=	power supply fault */
	unsigned int	Seconds;	/* seconds since 00:00:00, 1 January 1970 */
	unsigned int	Millisecs;	/* milliseconds, LSB = 1 ms */
	unsigned short		Temperature;	/* temperature reported by SeaBat (deg C) */
	char			Message[40];	/* ASCII message describing fault */
	char			Spare[40];	/* spare fields */
	unsigned short		Checksum;	/* checksum for data packet */
};

struct mbf_xtfr8101_struct
	{
	/* type of data record */
	int	kind;			/* Data vs Comment */

	/* type of sonar */
	int	sonar;			/* Type of Reson sonar */
	
	/* xtf file header */
	struct mbf_xtfr8101_xtffileheader fileheader;

	/* xtf attitude record */
	struct mbf_xtfattitudeheader attitudeheader;
	
	/* xtf Reson multibeam bathymetry record */
	struct mbf_xtfbathheader bathheader;
	struct RESON8100_RIT reson8100rit;
	struct mbf_xtfbathheader sidescanheader;
	struct mbf_xtfpingchanheader pingchanportheader;
	unsigned short ssrawport[MBF_XTFR8101_MAXRAWPIXELS];
	struct mbf_xtfpingchanheader pingchanstbdheader;
	unsigned short ssrawstbd[MBF_XTFR8101_MAXRAWPIXELS];

	/* comment */
	char comment[MBF_XTFR8101_COMMENT_LENGTH];
	};

