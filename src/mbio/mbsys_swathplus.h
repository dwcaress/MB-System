/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_swathplus.h	1/28/2014
 *	$Id$
 *
 *    Copyright (c) 2014-2014 by
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
 * mbsys_swathplus.h defines the MBIO data structures for handling data from
 * the following data formats:
 *      MBF_TEMPFORM : MBIO ID 221 - SWATHplus intermediate format
 *      MBF_SWPLSSXP : MBIO ID 222 - SWATHplus processed format
 *
 * Author:	David Finlayson
 * Date:	February 19, 2014
 *
 * HISTORY
 *
 * 2014-04-11 - David Finlayson
 * 	- Reformated comments.
 *	- Added record to hold MB System projection id
 */

/*
 * Notes on the mbsys_swathplus data structure and associated format:
 *
 * In early 2013 SEA sold the SWATHplus system to BathySwath where
 * the system will be rebranded the BathySwath.
 *
 *   1. BathySwath defines three data formats associated with the SWATHplus
 *      interferometric sonar: raw, intermediate, and processed.
 *      MB-System supports the intermediate format as MBIO
 *      format 221 (MBF_SWPLSSXI) and the processed format as MBIO
 *      format 222 (MBF_SWPLSSXP). Only the SXP format is fully tested at this time.
 *
 *   2. Bathyswath is a swath bathymetry sonar system.  It is derived from
 *      the SWATHplus sonar system, and uses the same file formats.  In
 *      turn, SWATHplus was derived from the Submetrix sonars, built by
 *      Submetrix Ltd.
 *
 *   3. The data files are written using a Microsoft Windows operating system
 *      and therefore follow the conventions of that system in terms of file
 *      naming and low-level disk format.  In particular, data structures
 *      often contain padding bytes that must be preserved to maintain
 *      compatibility with the format.
 *
 *   4. All Bathyswath and SWATHplus data is little-endian, i.e. in the
 *      natural 80x86 format with the least significant byte at the lower
 *      address.
 *
 *   5. MBF_SWPLSSXP - Processed data derived from the real-time software.
 *      These files have all corrections applied, including: attitude,
 *      position, tide, speed of sound, includes down-sampled position,
 *      attitude and tide information.  Processed files use a projected
 *      coordinate system (PCS), typically UTM.  The user must supply PRJ
 *      files for each input sxp file identifying the correct PCS.
 *
 *   6. MBF_SWPLSSXI - Raw data, but parsed into a format that is easier for
 *      third-party code to interpret. These files have none of the above
 *      corrections applied.
 *
 *   7. All of the bathyswath files use the same block-oriented data format.
 *      They can be read using the same software code, and the blocks that
 *      they contain may be included in any of the files.  The difference
 *      between the file types is therefore simply the types of data block
 *      that they tend to contain.  Each file contains a file header block,
 *      followed by a series of data blocks.  Every block contains a header
 *      that identifies the block, followed by the length of the block.
 *      Therefore, the reading software can identify the blocks that it
 *      wishes to read and ignore and skip over any block types that it
 *      encounters.  In this way, new blocks can be added to a file without
 *      necessarily having to update the reading software.
 *
 */

/* include mb_define.h */
#ifndef MB_DEFINE_DEF
#include "mb_define.h"
#endif

/*---------------------------------------------------------------
   Record ID definitions (if needed for use in data reading and writing) */

/* Processed (SXP) datagrams */
#define SWPLS_ID_NONE				0x0		/* 0 means no record at all */
#define SWPLS_ID_UNKNOWN				0x1
#define SWPLS_ID_SXP_HEADER_DATA		0x01df01df	/* Processed file header */
#define SWPLS_ID_PROCESSED_PING		0x28		/* processed ping data (prior to
											  January 2010) */
#define SWPLS_ID_PROCESSED_PING2		0x52		/* processed ping data (after
											  January 2010) */
#define SWPLS_ID_SBP_PROJECTION		0x0		/* PLACEHOLDER */

/* Parsed (SXI) datagrams */
#define SWPLS_ID_SXI_HEADER_DATA		0x521d52d1	/* parsed data file header */
#define SWPLS_ID_PARSED_PING			0x29	/* sonar data in parsed data */
#define SWPLS_ID_PARSED_ATTITUDE		0x2b	/* attitude data in parsed data */
#define SWPLS_ID_PARSED_POSITION_LL	0x2c	/* lat-long position idata in parsed data */
#define SWPLS_ID_PARSED_POSITION_EN	0x2d	/* easting-northing data in parsed data */
#define SWPLS_ID_PARSED_SSV			0x2e	/* speed of sound data in parsed data */
#define SWPLS_ID_PARSED_ECHOSOUNDER	0x2f	/* echosounder data in parsed data */
#define SWPLS_ID_PARSED_TIDE			0x30	/* tide data in parsed data */
#define SWPLS_ID_PARSED_AGDS			0x31	/* AGDS data in parsed data */

/* MB-System Custom datagrams (BathySwath user-reserved blocks) */
#define SWPLS_ID_COMMENT				0x100	/* MB System comment */
#define SWPLS_ID_POS_OFFSET			0x101	/* MB System position offset lever arms */
#define SWPLS_ID_IMU_OFFSET			0x102	/* MB System IMU offset lever arms*/
#define SWPLS_ID_TXER_OFFSET			0x103	/* MB System Transducer offset lever arms */
#define SWPLS_ID_WL_OFFSET			0x104	/* MB System Water Line offset lever arm */
#define SWPLS_ID_PROJECTION			0x105	/* MB System Projection ID */

/*---------------------------------------------------------------
   Record size definitions (if needed for use in data reading and writing) */

#define SWPLS_SIZE_NONE				0	/* 0 means no record at all  */
#define SWPLS_SIZE_BLOCKHEADER		8	/* Block Header (blockid, blocksize) */
#define SWPLS_SIZE_HEADER				8	/* File version block */
#define SWPLS_SIZE_PROCESSED_PING		256	/* Old-style pings (prior to Jan 2010) */
#define SWPLS_SIZE_PROCESSED_PING2	264	/* New-style pings (after Jan 2010) */
#define SWPLS_SIZE_POINT				40	/* Old-style points (prior to Jan 2010)*/
#define SWPLS_SIZE_POINT2				48	/* New-style points (after Jan 2010) */
#define SWPLS_SIZE_PARSED_PING		35
#define SWPLS_SIZE_PARSED_POINT		7
#define SWPLS_SIZE_ATTITUDE			25
#define SWPLS_SIZE_POSITION_LL			25
#define SWPLS_SIZE_POSITION_EN		25
#define SWPLS_SIZE_SSV				13
#define SWPLS_SIZE_ECHOSOUNDER		13	/* assumed value until known */
#define SWPLS_SIZE_TIDE				13	/* assumed value until known */
#define SWPLS_SIZE_AGDS				17	/* assumed value until known */

#define SWPLS_SIZE_PROJECTION			12	/* MB SYSTEM ONLY */
#define SWPLS_SIZE_COMMENT			12	/* MB SYSTEM ONLY */
#define SWPLS_SIZE_POS_OFFSET			25	/* MB SYSTEM ONLY */
#define SWPLS_SIZE_IMU_OFFSET			25	/* MB SYSTEM ONLY */
#define SWPLS_SIZE_TXER_OFFSET		41	/* MB SYSTEM ONLY */
#define SWPLS_SIZE_WL_OFFSET			13	/* MB SYSTEM ONLY */

/*---------------------------------------------------------------
   Array size definitions (if needed for use in data reading and writing) */

#define SWPLS_MAX_BEAMS			2048
#define SWPLS_MAX_PIXELS			2048
#define SWPLS_MAX_TXERS			3
#define SWPLS_MAX_LINENAME		40
#define SWPLS_MAX_TX_INFO			3
#define SWPLS_BUFFER_STARTSIZE	32768

/*---------------------------------------------------------------
   SWATHplus constant definitions */

/* Sonar ping mode settings (sxp_ping->txstat & SWPLS_SONAR_SEL_MASK) */
#define SWPLS_SONAR_SEL_MASK		3
#define SWPLS_SONAR_SEL_OFF		0
#define SWPLS_SONAR_SEL_SINGLE	1
#define SWPLS_SONAR_SEL_ALT		2
#define SWPLS_SONAR_SEL_SIM		3

/* Sample flag definitions */
#define SWPLS_POINT_REJECTED	0
#define SWPLS_POINT_ACCEPTED	1

/* Transducer azimuth beam widths (degrees) */
#define SWPLS_TYPE_L_BEAM_WIDTH	0.85
#define SWPLS_TYPE_M_BEAM_WIDTH	0.55
#define SWPLS_TYPE_H_BEAM_WIDTH	0.55

/* Variations of Pi precomputed */
#define kPi (M_PI)
#define k2Pi (kPi * 2.0)
#define kPiOver2 (kPi / 2.0)
#define k1OverPi (1.0 / kPi)
#define k1Over2Pi (1.0 / k2Pi)

/* Structs needed for doing coordinate transformations
   left-handed coordinate system */

typedef struct swathplus_vector_struct
	{
	double x;			/* positive to right */
	double y;			/* positive up */
	double z;			/* positive forward */
	} swpls_vector;

typedef struct swathplus_euler_angles_struct
	{
	double heading;	/*clockwise rotation of y, +starboard */
	double pitch;		/*clockwise rotation of x, +nose down */
	double bank;		/*clockwise rotation of z, +starboard up */
	} swpls_angles;

typedef struct swathplus_quaternion_struct
	{
	double w;
	double x;
	double y;
	double z;
	} swpls_quaternion;

typedef struct swathplus_matrix_struct
	{
	double m11, m12, m13;
	double m21, m22, m23;
	double m31, m32, m33;
	double tx, ty, tz;
	} swpls_matrix;

/* SWATHplus file header structure (SXI, SXP) */
typedef struct swpls_file_header_struct
	{
	int swver;		/* 3065601 means: Major version 3, Minor version 06, Release 56, Build 01 */
	int fmtver;	/* Obsolete */
	} swpls_header;

/* SWATHplus processed point data. (SXP) */
typedef struct swpls_point_struct
	{
	int sampnum;				/* sample number, rejected may not be present */
	double y;					/* north coordinate (m) */
	double x;					/* east coordinate (m) */
	float z;					/* depth positive down (m) */
	unsigned short int amp;		/* raw amplitude (16-bit) */
	unsigned short int procamp;	/* processed amplitude (16-bit) */
	unsigned char status;		/* 0 (bad) or 1 (good) */
	double tpu;				/* total propagated uncertainty (m) [Version 2]*/
	} swpls_point;

/* SWATHplus processed ping data (SXP)*/
typedef struct swpls_sxpping_struct
	{
	char linename[SWPLS_MAX_LINENAME];	/* apparently not used by SEA software
										  */
	unsigned int pingnumber;	/* ping number */
	double time_d;				/* UNIX time of start of ping */
	int notxers;				/* number of transducers in this record, always 1 */
	double easting;				/* easting coordinate of transducer (m) */
	double northing;			/* northing coordinate of transducer (m) */
	double roll;				/* roll at start of ping (deg) +starboard down*/
	double pitch;				/* pitch at start of ping (deg)  +nose up */
	double heading;			/* heading at start of ping (deg), +clockwise */
	double height;				/* height of crp in survey datum (m) +down */
	double tide;				/* tide at start of ping (m) */
	double sos;				/* speed of sound (mean value) */
	unsigned char txno;			/* transducer identifier */
	unsigned char txstat;		/* transducer status */
	unsigned char txpower;		/* power setting */
	short int analoggain;			/* analog gain setting */
	unsigned char nostaves;		/* number of staves on transducer */
	unsigned char txinfo[SWPLS_MAX_TX_INFO];	/* board type/revision/serial num. */
	unsigned char freq;			/* frequency code */
	double frequency;			/* frequency in hertz */
	short int trnstime;			/* transmit time/number of cycles */
	short int recvtime;			/* receive time/number of samples */
	unsigned char samprate;		/* receive rate micro-sec/sample */
	int nosampsorig;			/* num. samp. read in real time */
	int nosampsfile;			/* num. samp. in the processed file */
	int nosampslots;			/* number of sample slots */
	double txer_e;				/* easting coordinate of transducer (m) */
	double txer_n;				/* northing coordinate of transducer (m) */
	double txer_height;			/* positive down (m) */
	double txer_forward;		/* positive forward (m) */
	double txer_starboard;		/* positive starboard (m) */
	double txer_azimuth;		/* positive clockwise looking down (deg) */
	double txer_elevation;		/* angle of txer plate, positive angles above horizon (deg) */
	double txer_skew;			/* positive clockwise from rear (deg) */
	double txer_time;			/* time offset (sec) */
	double txer_waterdepth;		/* transducer draft (m) */
	double txer_pitch;			/* positive bow up (deg) [Version 2 pings only] */
	size_t points_alloc;			/* MBSYSTEM-ONLY number of allocated points (don't write out) */
	swpls_point *points;			/* array of swpls_points */
	} swpls_sxpping;

/* SWATHplus parsed ping data (SXI) */
typedef struct swpls_sxiping_struct
	{
	int time_d;				/* start of ping, seconds since 1970 */
	int microsec;				/* microseconds since time_d */
	unsigned char channel;		/* identifies the transducer */
	unsigned long pingnumber;	/* simultaneous pings are numbered separately */
	float frequency;				/* frequency of the transducer in Hz */
	float samp_period;			/* time period between sonar data samples, in
								  seconds */
	unsigned short nosamps;		/* number of samples following */
	float sos;					/* speed of sound used to calculate angles, m/s */
	short int txpulse;			/* transmit pulse length, in sonar cycles */
	char data_options;			/* allows options in data encoding */
	unsigned char ping_state;	/* records the status of pinging
								  single/alternating/simultaneous */
	unsigned short max_count;	/* maximum data count before filtering */
	unsigned short reserve1;		/* reserved for other ping information */
	size_t samps_alloc;			/* MB SYSTEM ONLY number of allocated samples
								  for following arrays */
	unsigned short *sampnum;	/* sample numbers */
	short int *angle;			/* Angle coded +15 bits = 180 deg up, -15 bits =
								  180 deg down, relative to the txer pointing
								  angle */
	unsigned short *amplitude;	/* Amplitude scaled so that 16 bits is the full
								  scale of the ADC */
	unsigned char *quality;		/* as set by "data options */
	} swpls_sxiping;

/* SWATHplus parsed attitude data (SXI) */
typedef struct swpls_attitude_struct
	{
	int time_d;				/* start of ping time code. */
	int microsec;				/* microseconds since time_d */
	unsigned char channel;		/* identifies the data source */
	float roll;					/* positive for starboard down */
	float pitch;				/* positive for nose up */
	float heading;				/* positive clockwise looking down */
	float height;				/* positive for down */
	} swpls_attitude;

/* SWATHplus parsed position geographic coordinates (SXI) */
typedef struct swpls_position_ll_struct
	{
	int time_d;				/* start of ping time code. */
	int microsec;				/* microseconds since time_d */
	unsigned char channel;		/* identifies the data source */
	double latitude;			/* degrees (of survey center?) */
	double longitude;			/* degrees (of survey center?) */
	} swpls_posll;

/* SWATHplus parsed position projected coordinates (SXI) */
typedef struct swpls_position_en_struct
	{
	int time_d;				/* start of ping time code. */
	int microsec;				/* microseconds since time_d*/
	unsigned char channel;		/* identifies the data source */
	double easting;				/* easting coordinate (m) */
	double northing;			/* northing coordinate (m) */
	} swpls_posen;

/* SWATHplus parsed speed of sound (SXI) */
typedef struct swpls_ssv_struct
	{
	int time_d;				/* start of ping time code. */
	int microsec;				/* microseconds since time_d */
	unsigned char channel;		/* identifies the data source */
	float ssv;					/* speed of sound (m/s) */
	} swpls_ssv;

/* SWATHplus parsed tide data (SXI) */
typedef struct swpls_tide_struct
	{
	int time_d;				/* start of ping time code. */
	int microsec;				/* microseconds since time_d*/
	unsigned char channel;		/* identifies the data source */
	float tide;					/* speed of sound (m/s) */
	} swpls_tide;

/* SWATHplus parsed echosounder data (SXI) */
typedef struct swpls_echosounder_struct
	{
	int time_d;				/* start of ping time code. */
	int microsec;				/* microseconds since time_d*/
	unsigned char channel;		/* identifies the data source */
	float altitude;				/* height above seabed (m) */
	} swpls_echosounder;

/* SWATHplus parsed Acoustic Ground Discrimination System (SXI) */
typedef struct swpls_agds_struct
	{
	int time_d;				/* start of ping time code. */
	int microsec;				/* microseconds since time_d*/
	unsigned char channel;		/* identifies the data source */
	float hardness;
	float roughness;
	} swpls_agds;

/* MB System comment structure (BathySwath User Reserved Block) */
typedef struct swpls_comment_struct
	{
	int time_d;				/* start of ping time code */
	int microsec;				/* microseconds since time_d*/
	int nchars;				/* number of characters */
	size_t message_alloc;		/* number of characters allocated in storage */
	char *message;			/* characters in the message */
	} swpls_comment;

/* MB System projection definition structure (BathySwath User Reserved Block) */
typedef struct swpls_projection_struct
	{
	int time_d;				/* start of ping time code */
	int microsec;				/* microseconds since time_d*/
	int nchars;				/* number of characters */
	size_t projection_alloc;		/* number of characters allocated in storage */
	char *projection_id;			/* characters in the projection id */
	} swpls_projection;


/* MB System common reference point to position lever arm (BathySwath User
  Reserve Block) */
typedef struct swpls_pos_offset_struct
	{
	int time_d;				/* start of ping time code */
	int microsec;				/* microseconds since time_d*/
	unsigned char channel;		/* identifies the data source */
	float height;				/* +up, meters */
	float forward;				/* +forward, meters */
	float starboard;				/* +starboard, meters */
	float time;					/* +time, seconds */
	} swpls_pos_offset;

/* MB System common reference point to imu position lever arm (BathySwath User
  Reserve Block) */
typedef struct swpls_imu_offset_struct
	{
	int time_d;				/* start of ping time code */
	int microsec;				/* microseconds since time_d */
	unsigned char channel;		/* identifies the data source */
	float height;				/* +up, meters */
	float forward;				/* +forward, meters */
	float starboard;				/* +starboard, meters */
	float time;					/* +time, secodns */
	} swpls_imu_offset;

/* MB System common reference point to transducer position lever arm (BathySwath
  User Reserve Block) */
typedef struct swpls_txer_offset_struct
	{
	int time_d;				/* start of ping time code */
	int microsec;				/* microseconds since time_d */
	unsigned char channel;		/* identifies the transducer */
	float height;				/* +up, meters */
	float forward;				/* +forward, meters */
	float starboard;				/* +starboard, meters */
	float azimuth;				/* +clockwise, degrees */
	float elevation;				/* +above horizon, degrees */
	float pitch;				/* +nose up, degrees */
	float skew;				/* +clockwise, degrees */
	float time;					/* +time, seconds */
	} swpls_txer_offset;

/* MB System common reference point to water line lever arm (BathySwath User
  Reserve Block) */
typedef struct swpls_wl_offset_struct
	{
	int time_d;				/* start of ping time code */
	int microsec;				/* microseconds since time_d*/
	unsigned char channel;		/* identifies the data source */
	float height;				/* +up, meters */
	} swpls_wl_offset;

/* MB System data structure */
struct mbsys_swathplus_struct
	{
	/* Type of most recently read data record */
	int kind;					/* MB-System record ID */
	int type;					/* SWATHplus datagram ID */

	/* MB-System time stamp of most recently read record */
	double time_d;
	int time_i[7];

	/* Projection information */
	int projection_set;
	swpls_projection projection;

	/* Processed (SXP) Records */
	int sxp_header_set;
	swpls_header sxp_header;
	swpls_sxpping sxp_ping;

	/* Parsed (SXI) Records */
	int sxi_header_set;
	swpls_header sxi_header;
	swpls_sxiping sxi_ping;
	swpls_attitude attitude;
	swpls_posll posll;
	swpls_posen posen;
	swpls_ssv ssv;
	swpls_tide tide;
	swpls_echosounder echosounder;
	swpls_agds agds;

	/* MB-System Records */
	swpls_comment comment;
	swpls_pos_offset pos_offset;
	swpls_imu_offset imu_offset;
	swpls_txer_offset txer_offset;
	swpls_wl_offset wl_offset;
	};

/*---------------------------------------------------------------*/

/* System specific function prototypes */

/* Note: this list of functions corresponds to the function pointers
 * that are included in the structure mb_io_struct that is defined
 * in the file mbsystem/src/mbio/mb_io.h
 * Not all of these functions are required - some only make sense to
 * define if the relevant data type is part of the format. For instance,
 * do not define mbsys_swathplus_extract_segy() if there are no subbottom
 * profiler data supported by this data system.
 * The function prototypes that are not required for all data systems
 * are commented out below. When using this example as the basis for
 * for coding a new MB-System I/O module, uncomment any non-required
 * functions that will be useful. */
int mbsys_swathplus_alloc(int verbose, void *mbio_ptr, void **store_ptr,
	int *error);
int mbsys_swathplus_deall(int verbose, void *mbio_ptr, void **store_ptr,
	int *error);
int mbsys_swathplus_dimensions(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbath, int *namp, int *nss,
	int *error);
int mbsys_swathplus_pingnumber(int verbose, void *mbio_ptr, int *pingnumber,
	int *error);
int mbsys_swathplus_sonartype(int verbose, void *mbio_ptr, void *store_ptr,
	int *sonartype, int *error);
int mbsys_swathplus_sidescantype(int verbose, void *mbio_ptr, void *store_ptr,
	int *ss_type, int *error);

/* int mbsys_swathplus_preprocess(int verbose, void *mbio_ptr, void *store_ptr, */
/*                        double time_d, double navlon, double navlat, */
/*                        double speed, double heading, double sonardepth, */
/*                        double roll, double pitch, double heave, */
/*                        int *error); */
int mbsys_swathplus_extract(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int time_i[7], double *time_d,
	double *navlon, double *navlat, double *speed,
	double *heading, int *nbath, int *namp, int *nss,
	char *beamflag, double *bath, double *amp,
	double *bathacrosstrack, double *bathalongtrack,
	double *ss, double *ssacrosstrack,
	double *ssalongtrack, char *comment, int *error);
int mbsys_swathplus_insert(int verbose, void *mbio_ptr, void *store_ptr,
	int kind, int time_i[7], double time_d,
	double navlon, double navlat, double speed,
	double heading, int nbath, int namp, int nss,
	char *beamflag, double *bath, double *amp,
	double *bathacrosstrack, double *bathalongtrack,
	double *ss, double *ssacrosstrack,
	double *ssalongtrack, char *comment, int *error);
int mbsys_swathplus_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int time_i[7], double *time_d,
	double *navlon, double *navlat, double *speed,
	double *heading, double *draft, double *roll,
	double *pitch, double *heave, int *error);

/* int mbsys_swathplus_extract_nnav(int verbose, void *mbio_ptr, void
   *store_ptr, */
/*			int nmax, int *kind, int *n, */
/*			int *time_i, double *time_d, */
/*			double *navlon, double *navlat, */
/*			double *speed, double *heading, double *draft, */
/*			double *roll, double *pitch, double *heave, */
/*			int *error); */
int mbsys_swathplus_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
	int time_i[7], double time_d, double navlon,
	double navlat, double speed, double heading,
	double draft, double roll, double pitch,
	double heave, int *error);
int mbsys_swathplus_extract_altitude(int verbose, void *mbio_ptr,
	void *store_ptr, int *kind,
	double *transducer_depth, double *altitude,
	int *error);

/* int mbsys_swathplus_insert_altitude(int verbose, void *mbio_ptr, void
   *store_ptr, */
/*                        double transducer_depth, double altitude, */
/*                        int *error); */
int mbsys_swathplus_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nsvp, double *depth,
	double *velocity, int *error);
int mbsys_swathplus_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
	int nsvp, double *depth, double *velocity,
	int *error);
int mbsys_swathplus_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams, double *ttimes,
	double *angles, double *angles_forward,
	double *angles_null, double *heave,
	double *alongtrack_offset, double *draft,
	double *ssv, int *error);
int mbsys_swathplus_detects(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams, int *detects, int *error);

/* int mbsys_swathplus_pulses(int verbose, void *mbio_ptr, void *store_ptr, */
/*                        int *kind, int *nbeams, int *pulses, int *error); */
int mbsys_swathplus_gains(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, double *transmit_gain,
	double *pulse_length, double *receive_gain,
	int *error);

/* int mbsys_swathplus_extract_rawss(int verbose, void *mbio_ptr, void
   *store_ptr, */
/*			int *kind, int *nrawss, */
/*			double *rawss, */
/*			double *rawssacrosstrack, */
/*			double *rawssalongtrack, */
/*			int *error); */
/* int mbsys_swathplus_insert_rawss(int verbose, void *mbio_ptr, void
   *store_ptr, */
/*			int nrawss, */
/*			double *rawss, */
/*			double *rawssacrosstrack, */
/*			double *rawssalongtrack, */
/*			int *error); */
/* int mbsys_swathplus_extract_segytraceheader(int verbose, void *mbio_ptr, void
   *store_ptr, */
/*			int *kind, */
/*			void *segytraceheader_ptr, */
/*			int *error); */
/* int mbsys_swathplus_extract_segy(int verbose, void *mbio_ptr, void
   *store_ptr, */
/*			int *sampleformat, */
/*			int *kind, */
/*			void *segytraceheader_ptr, */
/*			float *segydata, */
/*			int *error); */
/* int mbsys_swathplus_insert_segy(int verbose, void *mbio_ptr, void *store_ptr, */
/*			int kind, */
/*			void *segytraceheader_ptr, */
/*			float *segydata, */
/*			int *error); */
/* int mbsys_swathplus_ctd(int verbose, void *mbio_ptr, void *store_ptr, */
/*			int *kind, int *nctd, double *time_d, */
/*			double *conductivity, double *temperature, */
/*			double *depth, double *salinity, double *soundspeed, int *error); */
/* int mbsys_swathplus_ancilliarysensor(int verbose, void *mbio_ptr, void
   *store_ptr, */
/*			int *kind, int *nsensor, double *time_d, */
/*			double *sensor1, double *sensor2, double *sensor3, */
/*			double *sensor4, double *sensor5, double *sensor6, */
/*			double *sensor7, double *sensor8, int *error); */
int mbsys_swathplus_copy(int verbose, void *mbio_ptr, void *store_ptr,
	void *copy_ptr, int *error);

/* Custom swathplus functions */
int mbsys_swathplus_print_store(int verbose,
	struct mbsys_swathplus_struct *store,
	int *error);
int mbsys_swathplus_blockheader_read(int verbose, void *mb_io_ptr,
	int *recordid, int *size, int *error);

int swpls_init_transform(int verbose, swpls_matrix *m, int *error);
int swpls_concat_translate(int verbose, swpls_matrix *m, double dx, double dy,
	double dz, int *error);
int swpls_concat_rotate_x(int verbose, swpls_matrix *m, double pitch,
	int *error);
int swpls_concat_rotate_y(int verbose, swpls_matrix *m, double heading,
	int *error);
int swpls_concat_rotate_z(int verbose, swpls_matrix *m, double bank,
	int *error);
int swpls_transform(int verbose, const swpls_matrix *m, swpls_vector *p,
	int *error);

/*---------------------------------------------------------------*/
int swpls_chk_header(int verbose, void *mbio_ptr, char *buffer, int *recordid,
	int *size, int *error);
int swpls_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error);

int swpls_rd_sxpheader(int verbose, char *buffer, void *store_ptr, int *error);
int swpls_rd_sxpping(int verbose, char *buffer, void *store_ptr, int pingtype,
	int *error);
int swpls_wr_sxpheader(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error);
int swpls_wr_sxpping(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error);
int swpls_pr_sxpheader(int verbose, FILE *fout, swpls_header *header,
	int *error);
int swpls_pr_sxpping(int verbose, FILE *fout, swpls_sxpping *ping, int *error);

int swpls_rd_sxiheader(int verbose, char *buffer, void *store_ptr, int *error);
int swpls_rd_sxiping(int verbose, char *buffer, void *store_ptr, int *error);
int swpls_rd_attitude(int verbose, char *buffer, void *store_ptr, int *error);
int swpls_rd_posll(int verbose, char *buffer, void *store_ptr, int *error);
int swpls_rd_posen(int verbose, char *buffer, void *store_ptr, int *error);
int swpls_rd_ssv(int verbose, char *buffer, void *store_ptr, int *error);
int swpls_rd_echosounder(int verbose, char *buffer, void *store_ptr,
	int *error);
int swpls_rd_tide(int verbose, char *buffer, void *store_ptr, int *error);
int swpls_rd_agds(int verbose, char *buffer, void *store_ptr, int *error);
int swpls_wr_sxiheader(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error);
int swpls_wr_sxiping(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error);
int swpls_wr_attitude(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error);
int swpls_wr_posll(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error);
int swpls_wr_posen(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error);
int swpls_wr_ssv(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error);
int swpls_wr_echosounder(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error);
int swpls_wr_tide(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error);
int swpls_wr_agds(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error);
int swpls_pr_sxiheader(int verbose, FILE *fout, swpls_header *header,
	int *error);
int swpls_pr_sxiping(int verbose, FILE *fout, swpls_sxiping *ping, int *error);
int swpls_pr_attitude(int verbose, FILE *fout, swpls_attitude *attitude,
	int *error);
int swpls_pr_posll(int verbose, FILE *fout, swpls_posll *posll, int *error);
int swpls_pr_posen(int verbose, FILE *fout, swpls_posen *posen, int *error);
int swpls_pr_ssv(int verbose, FILE *fout, swpls_ssv *ssv, int *error);
int swpls_pr_echosounder(int verbose, FILE *fout, swpls_echosounder *sounder,
	int *error);
int swpls_pr_tide(int verbose, FILE *fout, swpls_tide *tide, int *error);
int swpls_pr_agds(int verbose, FILE *fout, swpls_agds *agds, int *error);

int swpls_rd_projection(int verbose, char *buffer, void *store_ptr, int *error);
int swpls_rd_comment(int verbose, char *buffer, void *store_ptr, int *error);
int swpls_rd_pos_offset(int verbose, char *buffer, void *store_ptr, int *error);
int swpls_rd_imu_offset(int verbose, char *buffer, void *store_ptr, int *error);
int swpls_rd_txer_offset(int verbose, char *buffer, void *store_ptr,
	int *error);
int swpls_rd_wl_offset(int verbose, char *buffer, void *store_ptr, int *error);
int swpls_wr_projection(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error);
int swpls_wr_comment(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error);
int swpls_wr_pos_offset(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error);
int swpls_wr_imu_offset(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error);
int swpls_wr_txer_offset(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error);
int swpls_wr_wl_offset(int verbose, int *bufferalloc, char **bufferptr,
	void *store_ptr, int *size, int *error);
int swpls_pr_projection(int verbose, FILE *fout, swpls_projection *projection,
	int *error);
int swpls_pr_comment(int verbose, FILE *fout, swpls_comment *comment,
	int *error);
int swpls_pr_pos_offset(int verbose, FILE *fout, swpls_pos_offset *pos_offset,
	int *error);
int swpls_pr_imu_offset(int verbose, FILE *fout, swpls_imu_offset *imu_offset,
	int *error);
int swpls_pr_txer_offset(int verbose, FILE *fout,
	swpls_txer_offset *txer_offset, int *error);
int swpls_pr_wl_offset(int verbose, FILE *fout, swpls_wl_offset *wl_offset,
	int *error);

