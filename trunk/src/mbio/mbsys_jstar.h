/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_jstar.h	2/21/2005
 *	$Id: mbsys_jstar.h,v 5.2 2006-11-10 22:36:05 caress Exp $
 *
 *    Copyright (c) 2005 by
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
 * mbsys_jstar.h  defines the data structure used by MBIO functions
 * to store sidescan data read from the MBF_EDGJSTAR format (MBIO id 132).  
 *
 * Author:	D. W. Caress
 * Date:	February 21, 2005
 * $Log: not supported by cvs2svn $
 * Revision 5.1  2005/11/05 00:48:03  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.0  2005/06/04 04:11:35  caress
 * Support for Edgetech Jstar format (id 132 and 133).
 *
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
 
/* specify the maximum number of sidescan pixels that can be returned
	by mbsys_jstar_extract() */
#define	MBSYS_JSTAR_MESSAGE_SIZE	16
#define	MBSYS_JSTAR_SBPHEADER_SIZE	240
#define	MBSYS_JSTAR_SSHEADER_SIZE	240
#define	MBSYS_JSTAR_PIXELS_MAX		2000
#define	MBSYS_JSTAR_SONARDATA		80
#define	MBSYS_JSTAR_COMMENT		17229
#define	MBSYS_JSTAR_SUBSYSTEM_SBP	0
#define	MBSYS_JSTAR_SUBSYSTEM_SSLOW	20
#define	MBSYS_JSTAR_SUBSYSTEM_SSHIGH	21

  /* Edgetech trace data format definitions */
#define	MBSYS_JSTAR_TRACEFORMAT_ENVELOPE		0 	/* 2 bytes/sample (unsigned) */
#define	MBSYS_JSTAR_TRACEFORMAT_ANALYTIC		1 	/* 4 bytes/sample (I + Q) */
#define	MBSYS_JSTAR_TRACEFORMAT_RAW		2 	/* 2 bytes/sample (signed) */
#define	MBSYS_JSTAR_TRACEFORMAT_REALANALYTIC	3 	/* 2 bytes/sample (signed) */
#define	MBSYS_JSTAR_TRACEFORMAT_PIXEL		4 	/* 2 bytes/sample (signed) */

struct mbsys_jstar_message_struct 
	{
	/* Message Header */
	unsigned short	start_marker;	/* bytes 0-1,    Marker for the start of header (0x1601) */
	mb_u_char	version;	/* byte  2,      Version of protocal used */
	mb_u_char	session;	/* byte  3,      Session identifier */
	unsigned short	type;		/* bytes 4-5,    Message type (80 - sonar trace data ) */
	mb_u_char	command;	/* bytes 6,      Command type */
	mb_u_char	subsystem;	/* bytes 7,      Subsystem:
								 0 - subbottom
								20 - 75 or 120 kHz sidescan
								21 - 410 kHz sidescan */
	mb_u_char	channel;	/* bytes 8,      Channel for multi-channel systems
								0 = port
								1 = starboard */
	mb_u_char	sequence;	/* bytes 9,      Sequence number */
	unsigned short	reserved;	/* bytes 10-11,  Reserved */
	unsigned int	size;		/* bytes 12-15,  Size of following message in bytes */
	};
	
struct mbsys_jstar_comment_struct 
	{
	/* Message Header */
	struct mbsys_jstar_message_struct message;

	/* Comment */
	char	comment[MB_COMMENT_MAXLINE];
	};
	
	
struct mbsys_jstar_channel_struct 
	{
	/* Message Header */
	struct mbsys_jstar_message_struct message;
	
	/* Trace Header */
	int sequenceNumber; 			/* 0-3 : Trace Sequence Number (always 0) ** */
	unsigned int startDepth;          	/* 4-7 : Starting depth (window offset) in samples. */
	unsigned int pingNum;              	/* 8-11: Ping number (increments with ping) ** */
	unsigned int channelNum;           	/* 12-15 : Channel Number (0 .. n) ** */
	short unused1[6];          		/* 16-27 */

	short traceIDCode;         		/* 28-29 : ID Code (always 1 => seismic data) ** */

	short unused2[2];     			/* 30-33 */
	short dataFormat;			/* 34-35 : DataFormatType */
						/*   0 = 1 short  per sample  - envelope data */
						/*   1 = 2 shorts per sample, - stored as real(1), imag(1), */
						/*   2 = 1 short  per sample  - before matched filter */
						/*   3 = 1 short  per sample  - real part analytic signal */
						/*   4 = 1 short  per sample  - pixel data / ceros data */
	short NMEAantennaeR;			/* 36-37 : Distance from towfish to antennae in cm */
	short NMEAantennaeO;			/* 38-39 : Distance to antennae starboard direction in cm */
	char RS232[32];				/* 40-71 : Reserved for RS232 data - TBD */
	/* -------------------------------------------------------------------- */
	/* Navigation data :                                                    */
	/* If the coorUnits are seconds(2), the x values represent longitude    */
	/* and the y values represent latitude.  A positive value designates    */
	/* the number of seconds east of Greenwich Meridian or north of the     */
	/* equator.                                                             */
	/* -------------------------------------------------------------------- */
	int sourceCoordX;			/* 72-75 : Meters or Seconds of Arc */
	int sourceCoordY;			/* 76-79 : Meters or Seconds of Arc */
	int groupCoordX;			/* 80-83 : mm or 10000 * (Minutes of Arc) */
	int groupCoordY;			/* 84-87 : mm or 10000 * (Minutes of Arc) */
	short coordUnits;			/* 88-89 : Units of coordinates - 1->length (x /y), 2->seconds of arc */
	char annotation[24];			/* 90-113 : Annotation string */
	unsigned short samples;			/* 114-115 : Samples in this packet ** */
						/* Note:  Large sample sizes require multiple packets. */
	unsigned int sampleInterval;		/* 116-119 : Sample interval in ns of stored data ** */
	unsigned short ADCGain;			/* 120-121 : Gain factor of ADC */
	short pulsePower;			/* 122-123 : user pulse power setting (0 - 100) percent */
	short correlated;			/* 124-125 : correlated data 1 - No, 2 - Yes */
	unsigned short startFreq;		/* 126-127 : Starting frequency in 10 * Hz */
	unsigned short endFreq;			/* 128-129 : Ending frequency in 10 * Hz */
	unsigned short sweepLength;		/* 130-131 : Sweep length in ms */
	short unused7[4];			/* 132-139 */
	unsigned short aliasFreq;		/* 140-141 : alias Frequency (sample frequency / 2) */
	unsigned short pulseID;			/* 142-143 : Unique pulse identifier */
	short unused8[6];			/* 144-155 */
	short year;				/* 156-157 : Year data recorded (CPU time) */
	short day;				/* 158-159 : day */
	short hour;				/* 160-161 : hour */
	short minute;				/* 162-163 : minute */
	short second;				/* 164-165 : second */
	short timeBasis;			/* 166-167 : Always 3 (other not specified by standard) */
	short weightingFactor;			/* 168-169 :  weighting factor for block floating point expansion */
						/*            -- defined as 2 -N volts for lsb */
	short unused9;				/* 170-171 : */
	/* -------------------------------------------------------------------- */
	/* From pitch/roll/temp/heading sensor */
	/* -------------------------------------------------------------------- */
	short heading;				/* 172-173 : Compass heading (100 * degrees) -180.00 to 180.00 degrees */
	short pitch;				/* 174-175 : Pitch */
	short roll;				/* 176-177 : Roll */
	short temperature;			/* 178-179 : Temperature (10 * degrees C) */
	/* -------------------------------------------------------------------- */
	/* User defined area from 180-239                                       */
	/* -------------------------------------------------------------------- */
	short heaveCompensation;		/* 180-181 : Heave compensation offset (samples) */
	short trigSource;   			/* 182-183 : TriggerSource (0 = internal, 1 = external) */    
	unsigned short markNumber;		/* 184-185 : Mark Number (0 = no mark) */
	short NMEAHour;				/* 186-187 : Hour */
	short NMEAMinutes;			/* 188-189 : Minutes */
	short NMEASeconds;			/* 190-191 : Seconds */
	short NMEACourse;			/* 192-193 : Course */
	short NMEASpeed;			/* 194-195 : Speed */
	short NMEADay;				/* 196-197 : Day */
	short NMEAYear;				/* 198-199 : Year */
	unsigned int millisecondsToday;		/* 200-203 : Millieconds today */
	unsigned short ADCMax;			/* 204-205 : Maximum absolute value for ADC samples for this packet */
	short calConst;				/* 206-207 : System constant in tenths of a dB */
	short vehicleID;			/* 208-209 : Vehicle ID */
	char softwareVersion[6];		/* 210-215 : Software version number */
	/* Following items are not in X-Star */
	int sphericalCorrection;		/* 216-219 : Initial spherical correction factor (useful for multiping /*/
						/* deep application) * 100 */
	unsigned short packetNum;		/* 220-221 : Packet number (1 - N) (Each ping starts with packet 1) */
	short ADCDecimation;			/* 222-223 : A/D decimation before FFT */
	short decimation;			/* 224-225 : Decimation factor after FFT */
	short unuseda;				/* 226-227 */
	
	/* -------------------------------------------------------------------- */
	/* MB-System-only parameters from 236-239                               */
	/* -------------------------------------------------------------------- */
	int depth;				/* 227-231 : Seafloor depth in 0.001 m */
	int sonardepth;				/* 236-235 : Sonar depth in 0.001 m */
	int sonaraltitude;			/* 236-239 : Sonar altitude in 0.001 m */
	
	/* trace data stored as shorts */
	unsigned int	trace_alloc;
	unsigned short	*trace;
	};
	
struct mbsys_jstar_ss_struct 
	{
	/* Message Header */
	struct mbsys_jstar_message_struct message;
	
	/* Trace Header */
	unsigned short subsystem;		/*   0 -   1 : Subsystem (0 .. n) */
	unsigned short channelNum;		/*   2 -   3 : Channel Number (0 .. n) */
	unsigned int pingNum;			/*   4 -   7 : Ping number (increments with ping) */
	unsigned short packetNum;		/*   8 -   9 : Packet number (1..n) Each ping starts with packet 1 */
	unsigned short trigSource;		/*  10 -  11 : TriggerSource (0 = internal, 1 = external) */
	unsigned int samples;			/*  12 -  15 : Samples in this packet */   
	unsigned int sampleInterval;		/*  16 -  19 : Sample interval in ns of stored data */
	unsigned int startDepth;		/*  20 -  23 : starting Depth (window offset) in samples */
	short weightingFactor;			/*  24 -  25 : -- defined as 2 -N volts for lsb */
	unsigned short ADCGain;			/*  26 -  27 : Gain factor of ADC */
	unsigned short ADCMax;			/*  28 -  29 : Maximum absolute value for ADC samples for this packet */
	unsigned short rangeSetting;		/*  30 -  31 : Range Setting (meters X 10) */
	unsigned short pulseID;			/*  32 -  33 : Unique pulse identifier */
	unsigned short markNumber;		/*  34 -  35 : Mark Number (0 = no mark) */
	unsigned short dataFormat;		/*  36 -  37 : Data format */
						/*   0 = 1 short  per sample  - envelope data */
						/*   1 = 2 shorts per sample  - stored as real(1), imag(1), */
						/*   2 = 1 short  per sample  - before matched filter (raw) */
						/*   3 = 1 short  per sample  - real part analytic signal */
						/*   NOTE: For type = 1, the total number of bytes of data to follow is */
						/*   4 * samples.  For all other types the total bytes is 2 * samples */
	unsigned short reserved;		/*  38 -  39 : Reserved field to round up to a 32-bit word boundary */
	/* -------------------------------------------------------------------- */
	/* computer date / time data acquired                                   */
	/* -------------------------------------------------------------------- */
	unsigned int millisecondsToday;		/*  40 -  43 : Millieconds today */
	short year;				/*  44 -  45 : Year */
	unsigned short day;			/*  46 -  47 : Day of year (1 - 366) */
	unsigned short hour;			/*  48 -  49 : Hour of day (0 - 23) */
	unsigned short minute;			/*  50 -  51 : Minute (0 - 59) */
	unsigned short second;			/*  52 -  53 : Second (0 - 59) */
	/* -------------------------------------------------------------------- */
	/* Auxillary sensor information */
	/* -------------------------------------------------------------------- */    
	short heading;				/*  54 -  55 : Compass heading (minutes) */
	short pitch;				/*  56 -  57 : Pitch (minutes) */
	short roll;				/*  58 -  59 : Roll (minutes) */
	short heave;				/*  60 -  61 : Heave (centimeters) */
	short yaw;				/*  62 -  63 : Yaw (minutes) */
	unsigned int depth;			/*  64 -  67 : Vehicle depth (centimeters) */
	short temperature;			/*  68 -  69 : Temperature (degrees Celsius X 10) */
	char reserved2[10];			/*  70 -  79 : Reserved for future use */
	
	/* trace data stored as shorts */
	unsigned int	trace_alloc;
	unsigned short	*trace;
	};
  
struct mbsys_jstar_struct 
	{
	int	kind;			/* MBIO data kind */
	
	/* Ping type */
	mb_u_char	subsystem;	/* bytes 7,      Subsystem:
								 0 - subbottom
								20 - 75 or 120 kHz sidescan
								21 - 410 kHz sidescan */

	/* SBP data */
	struct mbsys_jstar_channel_struct sbp;
	
	/* Sidescan data */
	struct mbsys_jstar_channel_struct ssport;
	struct mbsys_jstar_channel_struct ssstbd;
	
	/* Comment */
	struct mbsys_jstar_comment_struct comment;
	};
	
/* system specific function prototypes */
int mbsys_jstar_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_jstar_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_jstar_dimensions(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_jstar_pingnumber(int verbose, void *mbio_ptr, 
			int *pingnumber, int *error);
int mbsys_jstar_extract(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_jstar_insert(int verbose, void *mbio_ptr, void *store_ptr, 
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_jstar_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles, 
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset, 
			double *draft, double *ssv, int *error);
int mbsys_jstar_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude, 
			int *error);
int mbsys_jstar_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft, 
			double *roll, double *pitch, double *heave, 
			int *error);
int mbsys_jstar_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft, 
			double roll, double pitch, double heave,
			int *error);
int mbsys_jstar_extract_segytraceheader(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind,
			void *segyheader_ptr, 
			int *error);
int mbsys_jstar_extract_segy(int verbose, void *mbio_ptr, void *store_ptr,
			int *sampleformat,
			int *kind,
			void *segyheader_ptr, 
			float *segydata, 
			int *error);
int mbsys_jstar_insert_segy(int verbose, void *mbio_ptr, void *store_ptr,
			int kind,
			void *segyheader_ptr, 
			float *segydata, 
			int *error);
int mbsys_jstar_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error);

